/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */


#include <common.h>
#include <malloc.h>
#include <timer.h>
#include <stdio.h>

#include <asm/arch/ast-sdk/ast_g5_platform.h>
#include <asm/arch/ast-sdk/ast_scu.h>
#include <asm/arch/ast-sdk/regs-scu.h>

#include "flash-spl.h"
#define AST_FMC_WRITE_ENABLE 0x800f0000

#if defined(CONFIG_FBAL) || defined(CONFIG_FBSP)
/* Workaround slow down SPI clk to 12Mhz */
#define AST_FMC_STATUS_RESET 0x000b0041
#elif defined(CONFIG_FBY3)
/* Workaround slow down SPI clk to 40Mhz */
#define AST_FMC_STATUS_RESET 0x000B0D41
#else
#define AST_FMC_STATUS_RESET 0x000b0641
#endif

#define AST_FMC_CE1_CONTROL  0x14
#define AST_FMC_CE0_CONTROL  0x10
#define AST_FMC_CE_CONTROL   0x04

#define SPI_CMD_ID 0x9f
#define SPI_CMD_3B 0xe9
#define SPI_CMD_4B 0xb7
#define SPI_CMD_WE 0x06
#define SPI_CMD_RS 0x05
#define SPI_CMD_WS 0x01
#define SPI_CMD_RC 0x15

#define SPI_SRWD  (0x1 << 7)
#define SPI_BP3   (0x1 << 5)
#define SPI_BP2   (0x1 << 4)
#define SPI_BP1   (0x1 << 3)
#define SPI_BP0   (0x1 << 2)
#define SPI_WEL   (0x1 << 1)
#define SPI_WIP   (0x1 << 0)

/* Status register Top/Bottom bit select */
#define SPI_TB (0x1 << 6)

#define SPI_CS0_HW_PROTECTIONS (SPI_BP0 | SPI_BP1 | SPI_BP2 | SPI_BP3)
#define SPI_CS1_HW_PROTECTIONS (SPI_BP0)

#define WRITEREG(r, v) *(volatile u32*)(r) = v
#define WRITEB(r, b) *(volatile uchar*)(r) = (uchar)b
#define READREG(r) *(volatile u32*)(r)
#define READB(r) *(volatile uchar*)(r)

typedef int (*heaptimer_t)(unsigned ulong);
typedef int (*heapstatus_t)(heaptimer_t, uchar, bool);

#pragma GCC push_options
#pragma GCC optimize ("O0")

inline void fmc_enable_write(void) {
  /* If FMC00:{16, 17, 18} is 0 then it needs to be enabled with FMCA4. */
  /* Set FMCA4 to |= 2AA */
  WRITEREG(AST_FMC_BASE, READREG(AST_FMC_BASE) | AST_FMC_WRITE_ENABLE);
}

inline void fmc_reset(u32 ctrl) {
  WRITEREG(AST_FMC_BASE + ctrl, AST_FMC_STATUS_RESET);
}

inline void fmc_enable4b(uchar cs) {
  WRITEREG(AST_SCU_BASE + 0x70, READREG(AST_SCU_BASE + 0x70) | 0x10);
  WRITEREG(AST_FMC_BASE + AST_FMC_CE_CONTROL,
    READREG(AST_FMC_BASE + AST_FMC_CE_CONTROL) | (0x01 << cs));
}

inline void fmc_romcs(uchar cs) {
  u32 function_pin = READREG(AST_SCU_BASE + AST_SCU_FUN_PIN_CTRL3);
  function_pin |= SCU_FUN_PIN_ROMCS(cs);
  WRITEREG(AST_SCU_BASE, SCU_PROTECT_UNLOCK);
  WRITEREG(AST_SCU_BASE + AST_SCU_FUN_PIN_CTRL3, function_pin);
}

inline void spi_write_enable(heaptimer_t timer, u32 base, u32 ctrl) {
  WRITEREG(AST_FMC_BASE + ctrl, 0x603);
  timer(200);
  WRITEB(base, SPI_CMD_WE);
  timer(10);
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
}

inline uchar spi_status(heaptimer_t timer, u32 base, u32 ctrl, bool wel) {
  uchar r1;
  u32 timeout;

  WRITEREG(AST_FMC_BASE + ctrl, 0x03);
  timer(200);
  WRITEB(base, SPI_CMD_RS);
  timer(10);
  timeout = 8000;
  do {
    if (--timeout == 0) {
      r1 = 0xFF;
      break;
    }
    r1 = READB(base);
  } while ((wel && !(r1 & SPI_WEL)) || (!wel && (r1 & SPI_WIP)));
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
  return r1;
}

inline void spi_write_config(heaptimer_t timer, u32 base, u32 ctrl, uchar p) {
  uchar r1;

  /* The 'configuration' register on MXIC chips is the second status byte. */
  r1 = spi_status(timer, base, ctrl, false);

  WRITEREG(AST_FMC_BASE + ctrl, 0x603);
  timer(200);
  WRITEB(base, SPI_CMD_WS);
  timer(10);
  /* Must write the status register first. */
  WRITEB(base, r1);
  WRITEB(base, p);
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
}

inline uchar spi_config(heaptimer_t timer, u32 base, u32 ctrl) {
  uchar r1;

  WRITEREG(AST_FMC_BASE + ctrl, 0x03);
  timer(200);
  WRITEB(base, SPI_CMD_RC);
  timer(10);
  r1 = READB(base);
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
  return r1;
}

inline void spi_write_status(heaptimer_t timer, u32 base, u32 ctrl, uchar p) {
  WRITEREG(AST_FMC_BASE + ctrl, 0x603);
  timer(200);
  WRITEB(base, SPI_CMD_WS);
  timer(10);
  WRITEB(base, p);
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
}

inline void spi_enable4b(heaptimer_t timer, u32 base, u32 ctrl) {
  WRITEREG(AST_FMC_BASE + ctrl, 0x603);
  timer(200);
  WRITEB(base, SPI_CMD_4B);
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
}

inline void spi_id(heaptimer_t timer, u32 base, u32 ctrl, uchar* ch) {
  WRITEREG(AST_FMC_BASE + ctrl, 0x03);
  timer(200);
  WRITEB(base, SPI_CMD_ID);
  timer(10);
  ch[0] = READB(base);
  timer(10);
  ch[1] = READB(base);
  timer(10);
  ch[2] = READB(base);
  WRITEREG(AST_FMC_BASE + ctrl, 0x07);
  timer(200);
}

inline void set_topbottom_mxic(heaptimer_t timer, u32 base, u32 ctrl) {
  uchar r1;

  r1 = spi_config(timer, base, ctrl);
  spi_write_config(timer, base, ctrl, r1 | (0x1 << 3));

  /* Wait for the WIP/Busy to clear */
  (void)spi_status(timer, base, ctrl, false);
}
#if defined(CONFIG_FBAL)
//Workaround Add signal strength-->
inline void set_ods(heaptimer_t timer, u32 base, u32 ctrl) {
  uchar r1;

  r1 = spi_config(timer, base, ctrl);
  r1 |= 0x06;
  r1 &= 0xFE;
  spi_write_config(timer, base, ctrl, r1);
  /* Wait for the WIP/Busy to clear */
  (void)spi_status(timer, base, ctrl, false);
}
//<--end
#endif
int heaptimer(unsigned long usec) {
  ulong last;
  ulong clks;
  ulong elapsed;
  ulong now;

  /* translate usec to clocks */
  clks = (usec / 1000) * (CONFIG_ASPEED_TIMER_CLK / CONFIG_SYS_HZ);
  clks += (usec % 1000) * (CONFIG_ASPEED_TIMER_CLK / CONFIG_SYS_HZ) / 1000;

  elapsed = 0;
  now = 0;
  last = READREG(AST_TIMER_BASE);
  while (clks > elapsed) {
    now = READREG(AST_TIMER_BASE);
    if (now <= last) {
      elapsed += last - now;
    } else {
      elapsed += 0xffffffff - (now - last);
    }
    last = now;
  }
  return 1;
}

int heaptimer_end(void) {
  return 0;
}

int doheap(heaptimer_t timer, uchar cs, bool should_lock) {
  uchar status_set, status_check;
  u32 base;
  u32 ctrl;
  u32 prot;

  if (cs == 0) {
    base = AST_FMC_CS0_BASE;
    ctrl = AST_FMC_CE0_CONTROL;
    prot = SPI_CS0_HW_PROTECTIONS;
  } else {
    base = AST_FMC_CS1_BASE;
    ctrl = AST_FMC_CE1_CONTROL;
    prot = SPI_CS1_HW_PROTECTIONS;
  }

  /* Set the status register write disable. Only effective if WP# is low. */
  if (should_lock) {
    prot |= SPI_SRWD;
  }

  fmc_romcs(cs);

  if (cs == 1) {
    /* Set the T/B bit based on the chip vendor. */
    uchar id[3];
    spi_id(timer, base, ctrl, id);

    if (id[0] == 0xC2) {
      /* This is MX */
      spi_write_enable(timer, base, ctrl);
      spi_status(timer, base, ctrl, true);
      set_topbottom_mxic(timer, base, ctrl);
    } else if (id[0] == 0xEF || id[0] == 0xC8) {
      /* WB or GD */
      prot |= SPI_TB;
    } else {
      return AST_FMC_ERROR;
    }
  }
#if defined(CONFIG_FBAL)
//Workaround Add signal strength-->
  spi_write_enable(timer, base, ctrl);
  spi_status(timer, base, ctrl, true);
  spi_config(timer, base, ctrl);
  set_ods(timer, base, ctrl);
  spi_config(timer, base, ctrl);
//<--end
#endif
  /* Write enable for CSn */
  spi_write_enable(timer, base, ctrl);
  spi_status(timer, base, ctrl, true);

  spi_write_status(timer, base, ctrl, prot);
  status_set = spi_status(timer, base, ctrl, false);

  /* Write enable for CSn */
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

int doheap_end(void) {
  return 0;
}

int ast_fmc_spi_check(bool should_lock) {
  u32 function_size;
  uchar *buffer;
  int cs0_status, cs1_status;
  int ret;

  heaptimer_t timer_fp;
  heapstatus_t spi_check;

  ret = dm_timer_init();
  if (ret) {
     debug("timer init failed (%d)\n", ret);
  }
  fmc_enable_write();

  /* Place a timer function into SRAM */
  function_size = (u32) ((uchar*) heaptimer_end - (uchar*) heaptimer);
  buffer = (uchar*) malloc(function_size);
  if (!buffer) {
	  debug("malloc buffer failed\n");
	  return AST_FMC_ERROR;
  }
  memcpy(buffer, (uchar*)heaptimer, function_size);
  timer_fp = (heaptimer_t)buffer;

  /* Place our SPI inspections into SRAM */
  function_size = (u32) ((uchar*) doheap_end - (uchar*) doheap);
  buffer = (uchar*) malloc(function_size);
  memcpy(buffer, (uchar*)doheap, function_size);
  spi_check = (heapstatus_t)buffer;

  /* Protect and unprotect CS1 (always check this first) */
  cs1_status = spi_check(timer_fp, 1, should_lock);

  /* Protect and detect the hardware protection for CS0 */
  cs0_status = spi_check(timer_fp, 0, should_lock);

  /* Return an ERROR indicating PROM status issues. */
  return (cs1_status == AST_FMC_ERROR) ? AST_FMC_ERROR : cs0_status;
}
