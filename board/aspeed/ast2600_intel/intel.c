// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */
#include <common.h>
#include <asm/io.h>

/* SCU registers */
#define SCU_BASE	0x1e6e2000
#define SCU_PINMUX4	(SCU_BASE + 0x410)
#define   SCU_PINMUX4_RGMII3TXD1	BIT(19)
#define SCU_PINMUX5	(SCU_BASE + 0x414)
#define   SCU_PINMUX5_SGPMI		BIT(27)
#define   SCU_PINMUX5_SGPMO		BIT(26)
#define   SCU_PINMUX5_SGPMLD		BIT(25)
#define   SCU_PINMUX5_SGPMCK		BIT(24)
#define SCU_GPIO_PD0	(SCU_BASE + 0x610)
#define SCU_GPIO_PD0_B6			BIT(14)
#define SCU_PINMUX27	(SCU_BASE + 0x69c)
#define   SCU_PINMUX27_HBLED_EN		BIT(31)

/* eSPI registers */
#define ESPI_BASE		0x1e6ee000
#define ESPI_CTRL		(ESPI_BASE + 0x0)
#define ESPI_INT_EN		(ESPI_BASE + 0xc)
#define ESPI_CTRL2		(ESPI_BASE + 0x80)
#define ESPI_SYSEVT_INT_EN	(ESPI_BASE + 0x94)
#define ESPI_SYSEVT1_INT_EN	(ESPI_BASE + 0x100)
#define ESPI_SYSEVT_INT_T0	(ESPI_BASE + 0x110)
#define ESPI_SYSEVT_INT_T1	(ESPI_BASE + 0x114)
#define ESPI_SYSEVT1_INT_T0	(ESPI_BASE + 0x120)

/* LPC registers */
#define LPC_BASE	0x1e789000
#define LPC_HICR5	(LPC_BASE + 0x80)
#define   LPC_HICR5_SIO80HGPIO_EN	BIT(31)
#define   LPC_HICR5_80HGPIO_EN		BIT(30)
#define   LPC_HICR5_80HGPIO_SEL_MASK	GENMASK(28, 24)
#define   LPC_HICR5_80HGPIO_SEL_SHIFT	24
#define   LPC_HICR5_SNP0_INT_EN		BIT(1)
#define   LPC_HICR5_SNP0_EN		BIT(0)
#define LPC_HICR6	(LPC_BASE + 0x84)
#define   LPC_HICR6_STS_SNP1		BIT(1)
#define   LPC_HICR6_STS_SNP0		BIT(0)
#define LPC_SNPWADR	(LPC_BASE + 0x90)
#define   LPC_SNPWADR_SNP0_MASK		GENMASK(15, 0)
#define   LPC_SNPWADR_SNP0_SHIFT	0
#define LPC_HICRB	(LPC_BASE + 0x100)
#define   LPC_HICRB_80HSGPIO_EN		BIT(13)

/* GPIO/SGPIO registers */
#define GPIO_BASE	0x1e780000
#define GPIO_ABCD_VAL	(GPIO_BASE + 0x0)
#define GPIO_ABCD_VAL_D4		BIT(28)
#define GPIO_ABCD_VAL_C5		BIT(21)
#define GPIO_ABCD_VAL_C3		BIT(19)
#define GPIO_ABCD_DIR	(GPIO_BASE + 0x4)
#define GPIO_ABCD_DIR_D4		BIT(28)
#define GPIO_ABCD_DIR_C5		BIT(21)
#define GPIO_ABCD_DIR_C3		BIT(19)
#define GPIO_EFGH_DIR	(GPIO_BASE + 0x24)
#define GPIO_EFGH_DIR_G6		BIT(22)
#define SGPIO_M1_CONF	(GPIO_BASE + 0x554)
#define   SGPIO_M1_CONF_CLKDIV_MASK	GENMASK(31, 16)
#define   SGPIO_M1_CONF_CLKDIV_SHIFT	16
#define   SGPIO_M1_PINS_MASK		GENMASK(10, 6)
#define   SGPIO_M1_PINS_SHIFT		6
#define   SPGIO_M1_EN			BIT(0)

#define LPC_HICR5_UNKVAL_MASK		0x1FFF0000	/* bits with unknown values on reset */

static void snoop_init(void)
{
	u32 val;

	/* set lpc snoop #0 to port 0x80 */
	val = readl(LPC_SNPWADR) & 0xffff0000;
	val |= ((0x80 << LPC_SNPWADR_SNP0_SHIFT) &
		LPC_SNPWADR_SNP0_MASK);
	writel(val, LPC_SNPWADR);

	/* clear interrupt status */
	val = readl(LPC_HICR6);
	val |= (LPC_HICR6_STS_SNP0 |
		LPC_HICR6_STS_SNP1);
	writel(val, LPC_HICR6);

	/* enable lpc snoop #0 and SIOGIO */
	val = readl(LPC_HICR5);
	val |= (LPC_HICR5_SIO80HGPIO_EN |
		LPC_HICR5_SNP0_EN);
	writel(val, LPC_HICR5);

	/* enable port80h snoop on SGPIO */
	val = readl(LPC_HICRB);
	val |= LPC_HICRB_80HSGPIO_EN;
	writel(val, LPC_HICRB);
}

static void sgpio_init(void)
{
#define SGPIO_CLK_DIV(N)	((N) << 16)
#define SGPIO_BYTES(N)		((N) << 6)
#define SGPIO_ENABLE		1
#define SCU_414_SGPM_MASK	GENMASK(27, 24)

	/* set the sgpio clock to pclk/(2*(5+1)) or ~2 MHz */
	u32 val;

	val = ((256 << SGPIO_M1_CONF_CLKDIV_SHIFT) & SGPIO_M1_CONF_CLKDIV_MASK) |
	      ((10 << SGPIO_M1_PINS_SHIFT) & SGPIO_M1_PINS_MASK) |
	      SPGIO_M1_EN;
	writel(val, SGPIO_M1_CONF);

	val = readl(SCU_PINMUX5);
	val |= (SCU_PINMUX5_SGPMI  |
		SCU_PINMUX5_SGPMO  |
		SCU_PINMUX5_SGPMLD |
		SCU_PINMUX5_SGPMCK);
	writel(val, SCU_PINMUX5);
}

static void gpio_init(void)
{
	/* Default setting of Y23 pad in AST2600 A1 is HBLED so disable it. */
	writel(readl(SCU_PINMUX27) & ~SCU_PINMUX27_HBLED_EN,
	       SCU_PINMUX27);

	/*
	 * Set GPIOC3 as an output with value high explicitly since it doesn't
	 * have an external pull up. It uses direct register access because
	 * it's called from board_early_init_f().
	 */
	writel(readl(SCU_PINMUX4) & ~SCU_PINMUX4_RGMII3TXD1,
	       SCU_PINMUX4);
	writel(readl(GPIO_ABCD_DIR) | GPIO_ABCD_DIR_C3,
	       GPIO_ABCD_DIR);
	writel(readl(GPIO_ABCD_VAL) | GPIO_ABCD_VAL_C3,
	       GPIO_ABCD_VAL);

	writel(readl(SCU_GPIO_PD0) | SCU_GPIO_PD0_B6, SCU_GPIO_PD0);

	/*
	 * GPIO C5 has a connection between BMC(3.3v) and CPU(1.0v) so if we
	 * set it as an logic high output, it will be clipped by a protection
	 * circuit in the CPU and eventually the signal will be detected as
	 * logic low. So we leave this GPIO as an input so that the signal
	 * can be pulled up by a CPU internal resister. The signal will be
	 * 1.0v logic high resultingy.
	 */
	writel(readl(GPIO_ABCD_DIR) & ~GPIO_ABCD_DIR_C5,
	       GPIO_ABCD_DIR);

	/*
	 * Set GPIOD4 as an output with value low explicitly to set the
	 * default SPD mux path to CPU and DIMMs.
	 */
	writel(readl(GPIO_ABCD_DIR) | GPIO_ABCD_DIR_D4,
	       GPIO_ABCD_DIR);
	writel(readl(GPIO_ABCD_VAL) & ~GPIO_ABCD_VAL_D4,
	       GPIO_ABCD_VAL);

	/* GPIO G6 is also an open-drain output so set it as an input. */
	writel(readl(GPIO_EFGH_DIR) & ~GPIO_EFGH_DIR_G6,
	       GPIO_EFGH_DIR);
}

static void espi_init(void)
{
	u32 reg;

	/*
	 * Aspeed STRONGLY NOT recommend to use eSPI early init.
	 *
	 * This eSPI early init sequence merely set OOB_FREE. It
	 * is NOT able to actually handle OOB requests from PCH.
	 *
	 * During the power on stage, PCH keep waiting OOB_FREE
	 * to continue its booting. In general, OOB_FREE is set
	 * when BMC firmware is ready. That is, the eSPI kernel
	 * driver is mounted and ready to serve eSPI. However,
	 * it means that PCH must wait until BMC kernel ready.
	 *
	 * For customers that request PCH booting as soon as
	 * possible. You may use this early init to set OOB_FREE
	 * to prevent PCH from blocking by OOB_FREE before BMC
	 * kernel ready.
	 *
	 * If you are not sure what you are doing, DO NOT use it.
	 */
	reg = readl(ESPI_CTRL);
	reg |= 0xef;
	writel(reg, ESPI_CTRL);

	writel(0x0, ESPI_SYSEVT_INT_T0);
	writel(0x0, ESPI_SYSEVT_INT_T1);

	reg = readl(ESPI_INT_EN);
	reg |= 0x80000000;
	writel(reg, ESPI_INT_EN);

	writel(0xffffffff, ESPI_SYSEVT_INT_EN);
	writel(0x1, ESPI_SYSEVT1_INT_EN);
	writel(0x1, ESPI_SYSEVT1_INT_T0);

	reg = readl(ESPI_CTRL2);
	reg |= 0x50;
	writel(reg, ESPI_CTRL2);

	reg = readl(ESPI_CTRL);
	reg |= 0x10;
	writel(reg, ESPI_CTRL);
}

int board_early_init_f(void)
{
	snoop_init();
	gpio_init();
	sgpio_init();
	espi_init();
	return 0;
}
