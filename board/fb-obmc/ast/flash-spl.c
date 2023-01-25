/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#define DEBUG
#include <common.h>
#include <malloc.h>
#include <timer.h>
#include <stdio.h>
#include <asm/io.h>

#include <asm/arch/platform.h>
#include "vboot-op-cert.h"
#include "flash-spl.h"

/* defines migrate to upsteam/new-sdk */
#define ASPEED_FMC_CS1_BASE (ASPEED_FMC_CS0_BASE + 0x8000000)

#define AST_FMC_WRITE_ENABLE 0x800f0000

/* ===================== AST2600 FMC_CEn_CTRL Default ========================*/
#if defined(CONFIG_ASPEED_AST2600)
/*
 *  set to power on default value
 */
#define AST_FMC_STATUS_RESET 0x00000400
/* ===================== AST2500 FMC_CEn_CTRL Default ========================*/
#elif defined(CONFIG_FBAL) || defined(CONFIG_FBEP) || defined(CONFIG_FBY3) ||  \
	defined(CONFIG_FBCC)
/* Workaround slow down SPI clk to 40 Mhz */
#define AST_FMC_STATUS_RESET 0x000b0d41
#elif defined(CONFIG_FBSP)
/* Workaround slow down SPI clk to 12 Mhz */
#define AST_FMC_STATUS_RESET 0x000b0041
#else
#define AST_FMC_STATUS_RESET 0x000b0641
#endif

#define AST_FMC_CE1_CONTROL 0x14
#define AST_FMC_CE0_CONTROL 0x10
#define AST_FMC_CE_CONTROL 0x04

#define SPI_CMD_ID 0x9f
#define SPI_CMD_3B 0xe9
#define SPI_CMD_4B 0xb7
#define SPI_CMD_WE 0x06
#define SPI_CMD_RS 0x05
#define SPI_CMD_WS 0x01
#define SPI_CMD_RC 0x15

#define SPI_SRWD (0x1 << 7)
#define SPI_BP3 (0x1 << 5)
#define SPI_BP2 (0x1 << 4)
#define SPI_BP1 (0x1 << 3)
#define SPI_BP0 (0x1 << 2)
#define SPI_WEL (0x1 << 1)
#define SPI_WIP (0x1 << 0)

/* Status register Top/Bottom bit select */
#define SPI_TB (0x1 << 6)

/* Micron Tech */
#define SPI_BP3_MT (0x1 << 6)
#define SPI_TB_MT (0x1 << 5)
// clang-format off
#if CONFIG_FBVBOOT_GOLDEN_IMAGE_SIZE_MB == 64
/* Lock top 64MB of CS0 */
#  define SPI_CS0_HW_PROTECTIONS (SPI_BP3 | SPI_BP1 | SPI_BP0)
#  define SPI_CS0_HW_PROTECTIONS_MT (SPI_BP3_MT | SPI_BP1 | SPI_BP0)
#elif CONFIG_FBVBOOT_GOLDEN_IMAGE_SIZE_MB == 32
/* Lock top 32MB of CS0 */
#  define SPI_CS0_HW_PROTECTIONS (SPI_BP3 | SPI_BP1)
#  define SPI_CS0_HW_PROTECTIONS_MT (SPI_BP3_MT | SPI_BP1)
#else
#  error "Invalid CONFIG_FBVBOOT_GOLDEN_IMAGE_SIZE_MB, only support 32 or 64"
#endif
// clang-format on
/* Lock top 64KB of CS1 */
#define SPI_CS1_HW_PROTECTIONS (SPI_BP0)

/* Lock top 256KB (SPL partition) of CS0 */
#define SPI_CS0_LOCK_SPL (SPI_BP0 | SPI_BP1)

#define WRITEREG(r, v) *(volatile u32 *)(r) = v
#define WRITEB(r, b) *(volatile uchar *)(r) = (uchar)b
#define READREG(r) *(volatile u32 *)(r)
#define READB(r) *(volatile uchar *)(r)

typedef int (*heaptimer_t)(unsigned ulong);
typedef int (*heapstatus_t)(heaptimer_t, uchar, bool, int);

#pragma GCC push_options
#pragma GCC optimize("O0")

inline void fmc_enable_write(void)
{
	/* If FMC00:{16, 17, 18} is 0 then it needs to be enabled with FMCA4. */
	/* Set FMCA4 to |= 2AA */
	WRITEREG(ASPEED_FMC_BASE,
		 READREG(ASPEED_FMC_BASE) | AST_FMC_WRITE_ENABLE);
}

inline void fmc_reset(u32 ctrl)
{
	WRITEREG(ASPEED_FMC_BASE + ctrl, AST_FMC_STATUS_RESET);
}

#if !defined(CONFIG_ASPEED_AST2600) /* 4B mode enable setup in board_init*/
inline void fmc_enable4b(uchar cs)
{
	WRITEREG(ASPEED_SCU_BASE + 0x70,
		 READREG(ASPEED_SCU_BASE + 0x70) | 0x10);
	WRITEREG(ASPEED_FMC_BASE + AST_FMC_CE_CONTROL,
		 READREG(ASPEED_FMC_BASE + AST_FMC_CE_CONTROL) | (0x01 << cs));
}
#else
#define fmc_enable4b(cs)
#endif

#if !defined(CONFIG_ASPEED_AST2600) /* AST2600 did not multi-func FMC-CS pins*/
#define AST_SCU_FUN_PIN_CTRL3 0x88
#define SCU_FUN_PIN_ROMCS(x) (0x1 << (23 + x))
inline void fmc_romcs(uchar cs)
{
	u32 function_pin = READREG(ASPEED_SCU_BASE + AST_SCU_FUN_PIN_CTRL3);
	function_pin |= SCU_FUN_PIN_ROMCS(cs);
	WRITEREG(ASPEED_SCU_BASE, SCU_PROTECT_UNLOCK);
	WRITEREG(ASPEED_SCU_BASE + AST_SCU_FUN_PIN_CTRL3, function_pin);
}
#else
#define fmc_romcs(cs)
#endif

inline void spi_write_enable(heaptimer_t timer, u32 base, u32 ctrl)
{
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_WE);
	timer(10);
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
}

inline uchar spi_status(heaptimer_t timer, u32 base, u32 ctrl, bool wel)
{
	uchar r1;
	u32 timeout;

	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_RS);
	timeout = 1600;
	do {
		if (timeout == 0) {
			r1 = 0xFF;
			break;
		}
		--timeout;
		timer(10);
		r1 = READB(base);
	} while ((wel && !(r1 & SPI_WEL)) || (!wel && (r1 & SPI_WIP)));
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
	return r1;
}

inline void spi_write_config(heaptimer_t timer, u32 base, u32 ctrl, uchar p)
{
	uchar r1;

	/* The 'configuration' register on MXIC chips is the second status byte. */
	r1 = spi_status(timer, base, ctrl, false);

	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_WS);
	timer(10);
	/* Must write the status register first. */
	WRITEB(base, r1);
	WRITEB(base, p);
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
}

inline uchar spi_config(heaptimer_t timer, u32 base, u32 ctrl)
{
	uchar r1;

	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_RC);
	timer(10);
	r1 = READB(base);
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
	return r1;
}

inline void spi_write_status(heaptimer_t timer, u32 base, u32 ctrl, uchar p)
{
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_WS);
	timer(10);
	WRITEB(base, p);
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
}

inline void spi_enable4b(heaptimer_t timer, u32 base, u32 ctrl)
{
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_4B);
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
}

inline void spi_id(heaptimer_t timer, u32 base, u32 ctrl, uchar *ch)
{
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x03);
	timer(200);
	WRITEB(base, SPI_CMD_ID);
	timer(10);
	ch[0] = READB(base);
	timer(10);
	ch[1] = READB(base);
	timer(10);
	ch[2] = READB(base);
	WRITEREG(ASPEED_FMC_BASE + ctrl, 0x07);
	timer(200);
}

inline void set_topbottom_mxic(heaptimer_t timer, u32 base, u32 ctrl)
{
	uchar r1;

	r1 = spi_config(timer, base, ctrl);
	spi_write_config(timer, base, ctrl, r1 | (0x1 << 3));

	/* Wait for the WIP/Busy to clear */
	(void)spi_status(timer, base, ctrl, false);
}

inline u32 giu_mode_2_cs0_bp_bits(int giu_mode, bool is_mt)
{
	/*
	* GIU_CERT to execute golden image upgrade we only lock SPL 256KB of CS0
	*/
	if (giu_mode == GIU_CERT) {
		return SPI_CS0_LOCK_SPL;
	}
	if (is_mt) {
		/* MT BP3 bit at different location */
		return SPI_CS0_HW_PROTECTIONS_MT;
	}
	return SPI_CS0_HW_PROTECTIONS;
}

#define ASPEED_TIMER1_STS_REG (ASPEED_TIMER_BASE)
int heaptimer(unsigned long usec)
{
	ulong last;
	ulong clks;
	ulong elapsed;
	ulong now;

	/* translate usec to clocks */
	clks = (usec / 1000) * (CONFIG_ASPEED_TIMER_CLK / CONFIG_SYS_HZ);
	clks += (usec % 1000) * (CONFIG_ASPEED_TIMER_CLK / CONFIG_SYS_HZ) /
		1000;

	elapsed = 0;
	now = 0;
	last = READREG(ASPEED_TIMER1_STS_REG);
	while (clks > elapsed) {
		now = READREG(ASPEED_TIMER1_STS_REG);
		/* counting down timer, reload value set to 0xFFFFFFFF
		 * so simply: last modulo_minus now
		 */
		elapsed += last - now;
		last = now;
	}
	return 1;
}

int heaptimer_end(void)
{
	return 0;
}

int doheap(heaptimer_t timer, uchar cs, bool should_lock, int giu_mode)
{
	uchar status_set, status_check;
	uchar id[3] = { 0 };
	u32 base;
	u32 ctrl;
	u32 prot;

	if (cs == 0) {
		base = ASPEED_FMC_CS0_BASE;
		ctrl = AST_FMC_CE0_CONTROL;
		prot = giu_mode_2_cs0_bp_bits(giu_mode, false);
	} else {
		base = ASPEED_FMC_CS1_BASE;
		ctrl = AST_FMC_CE1_CONTROL;
		prot = SPI_CS1_HW_PROTECTIONS;
	}

	fmc_romcs(cs);

	/* Set the T/B bit based on the chip vendor. */
	spi_id(timer, base, ctrl, id);

	if (id[0] == 0xC2) {
		/* This is MX */
		spi_write_enable(timer, base, ctrl);
		spi_status(timer, base, ctrl, true);
		set_topbottom_mxic(timer, base, ctrl);
	} else if (id[0] == 0xEF || id[0] == 0xC8) {
		/* WB or GD */
		prot |= SPI_TB;
	} else if (id[0] == 0x20) {
		/* MT */
		if (cs == 0) {
			prot = giu_mode_2_cs0_bp_bits(giu_mode, true);
		}
		prot |= SPI_TB_MT;
	} else {
		return AST_FMC_ERROR;
	}

	/* Set the status register write disable.
	 * Only effective if WP# is low.
	 * GIU_OPEN speical mode will override the hwlock
	 */
	if (should_lock && (giu_mode != GIU_OPEN)) {
		prot |= SPI_SRWD;
	}

	/* Try write protect CSn */
	spi_write_enable(timer, base, ctrl);
	spi_status(timer, base, ctrl, true);

	spi_write_status(timer, base, ctrl, prot);
	status_set = spi_status(timer, base, ctrl, false);

	/* Disable write protection for CSn */
	spi_write_enable(timer, base, ctrl);
	spi_status(timer, base, ctrl, true);
	spi_write_status(timer, base, ctrl, 0x0);
	status_check = spi_status(timer, base, ctrl, false);

	/* Set the SPI into 32bit addressing mode. */
	fmc_enable4b(cs);
	timer(200);

	spi_enable4b(timer, base, ctrl);

	/* Reset the FMC state. */
	fmc_reset(ctrl);
	timer(200);

	if (status_set == 0xFF || status_check == 0xFF) {
		return AST_FMC_ERROR;
	} else if ((status_set & prot) != prot) {
		return AST_FMC_ERROR;
	} else if ((status_check & prot) != prot) {
		return AST_FMC_WP_OFF;
	}
	return AST_FMC_WP_ON;
}

int doheap_end(void)
{
	return 0;
}

#if defined(CONFIG_ASPEED_AST2600)
#define ASPEED_TIMER1_RELOAD_VAL 0xFFFFFFFF
#define ASPEED_TIMER1_RELOAD_REG (ASPEED_TIMER_BASE + 0x04)
#define ASPEED_TIMER_CTRL_REG (ASPEED_TIMER_BASE + 0x30)
#define ASPEED_TIMER_CLER_REG (ASPEED_TIMER_BASE + 0x3C)
#define ASPEED_TIMER1_EN (1 << 0)
#define ASPEED_TIMER1_1MHZ (1 << 1)

static int ast2600_start_timer1(void)
{
	writel(ASPEED_TIMER1_RELOAD_VAL, ASPEED_TIMER1_RELOAD_REG);

	/*
	 * Stop the timer. This will also load reload_val into
	 * the status register.
	 */
	setbits_le32(ASPEED_TIMER_CLER_REG, ASPEED_TIMER1_EN);
	/* Start the timer from the fixed 1MHz clock. */
	setbits_le32(ASPEED_TIMER_CTRL_REG,
		     (ASPEED_TIMER1_EN | ASPEED_TIMER1_1MHZ));

	return 0;
}
#endif

int ast_fmc_spi_check(bool should_lock, int giu_mode, void **pp_timer,
		      void **pp_spi_check)
{
	u32 function_size;
	uchar *buffer;
	int cs0_status, cs1_status;
	int ret;

	heaptimer_t timer_fp;
	heapstatus_t spi_check;

	u32 ce0_ctrl = readl(ASPEED_FMC_BASE + AST_FMC_CE0_CONTROL);
	u32 ce1_ctrl = readl(ASPEED_FMC_BASE + AST_FMC_CE1_CONTROL);
	debug("should_lock=%d, giu_mode=0x%X\n", should_lock, giu_mode);
	debug("Before: CE0_CTRL=0x%08X, CE1_CTRL=0x%08X\n", ce0_ctrl, ce1_ctrl);
	if (*pp_timer || *pp_spi_check) {
		printf("Not first time execute spi_check, skip timer init.\n");
		ret = 0;
	} else {
#if defined(CONFIG_ASPEED_AST2600)
		ret = ast2600_start_timer1();
#else
		ret = dm_timer_init();
#endif
	}
	if (ret) {
		debug("timer init failed (%d)\n", ret);
	}
	fmc_enable_write();

	/* Place a timer function into SRAM */
	if (*pp_timer) {
		printf("timer_fp already in SRAM 0x%p\n", *pp_timer);
		timer_fp = (heaptimer_t)(*pp_timer);
	} else {
		function_size =
			(u32)((uchar *)heaptimer_end - (uchar *)heaptimer);
		buffer = (uchar *)malloc(function_size);
		if (!buffer) {
			debug("malloc buffer for timer_fp failed\n");
			return AST_FMC_ERROR;
		}
		memcpy(buffer, (uchar *)heaptimer, function_size);
		timer_fp = (heaptimer_t)buffer;
		*pp_timer = timer_fp;
	}
	/* Place our SPI inspections into SRAM */
	if (*pp_spi_check) {
		printf("spi_check already in SRAM 0x%p\n", *pp_spi_check);
		spi_check = (heapstatus_t)(*pp_spi_check);
	} else {
		function_size = (u32)((uchar *)doheap_end - (uchar *)doheap);
		buffer = (uchar *)malloc(function_size);
		if (!buffer) {
			debug("malloc buffer for spi_check failed\n");
			return AST_FMC_ERROR;
		}
		memcpy(buffer, (uchar *)doheap, function_size);
		spi_check = (heapstatus_t)buffer;
		*pp_spi_check = spi_check;
	}
	/* Protect and unprotect CS1 (always check this first) */
	cs1_status = spi_check(timer_fp, 1, should_lock, giu_mode);

	/* Protect and detect the hardware protection for CS0 */
	cs0_status = spi_check(timer_fp, 0, should_lock, giu_mode);

	debug("cs0_status = %d, cs1_status = %d\n", cs0_status, cs1_status);
	/* Return an ERROR indicating PROM status issues. */
	ce0_ctrl = readl(ASPEED_FMC_BASE + AST_FMC_CE0_CONTROL);
	ce1_ctrl = readl(ASPEED_FMC_BASE + AST_FMC_CE1_CONTROL);
	debug("After: CE0_CTRL=0x%08X, CE1_CTRL=0x%08X\n", ce0_ctrl, ce1_ctrl);
	return (cs1_status == AST_FMC_ERROR) ? AST_FMC_ERROR : cs0_status;
}
