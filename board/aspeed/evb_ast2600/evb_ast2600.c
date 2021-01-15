// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */
#include <common.h>
#include <asm/io.h>

#define SCU_BASE	0x1e6e2000

#define AST_LPC_BASE	0x1e789000
#define HICR5		0x80
#define HICR6		0x84
#define SNPWADR		0x90
#define HICRB		0x100
#define LPC_SNOOP_ADDR	0x80

#define AST_GPIO_BASE	0x1e780000

/* HICR5 Bits */
#define HICR5_EN_SIOGIO (1 << 31)		/* Enable SIOGIO */
#define HICR5_EN80HGIO (1 << 30)		/* Enable 80hGIO */
#define HICR5_SEL80HGIO (0x1f << 24)		/* Select 80hGIO */
#define SET_SEL80HGIO(x) ((x & 0x1f) << 24)	/* Select 80hGIO Offset */
#define HICR5_UNKVAL_MASK 0x1FFF0000		/* Bits with unknown values on reset */
#define HICR5_ENINT_SNP0W (1 << 1)		/* Enable Snooping address 0 */
#define HICR5_EN_SNP0W (1 << 0)			/* Enable Snooping address 0 */

/* HRCR6 Bits */
#define HICR6_STR_SNP0W (1 << 0)	/* Interrupt Status Snoop address 0 */
#define HICR6_STR_SNP1W (1 << 1)	/* Interrupt Status Snoop address 1 */

/* HICRB Bits */
#define HICRB_EN80HSGIO (1 << 13)	/* Enable 80hSGIO */

static void __maybe_unused port80h_snoop_init(void)
{
	uint32_t value;
	/* enable port80h snoop and sgpio */
	/* set lpc snoop #0 to port 0x80 */
	value = readl(AST_LPC_BASE + SNPWADR) & 0xffff0000;
	writel(value | LPC_SNOOP_ADDR, AST_LPC_BASE + SNPWADR);

	/* clear interrupt status */
	value = readl(AST_LPC_BASE + HICR6);
	value |= HICR6_STR_SNP0W | HICR6_STR_SNP1W;
	writel(value, AST_LPC_BASE + HICR6);

	/* enable lpc snoop #0 and SIOGIO */
	value = readl(AST_LPC_BASE + HICR5) & ~(HICR5_UNKVAL_MASK);
	value |= HICR5_EN_SIOGIO | HICR5_EN_SNP0W;
	writel(value, AST_LPC_BASE + HICR5);

	/* enable port80h snoop on SGPIO */
	value = readl(AST_LPC_BASE + HICRB) | HICRB_EN80HSGIO;
	writel(value, AST_LPC_BASE + HICRB);
}

static void __maybe_unused sgpio_init(void)
{
#define SGPIO_CLK_DIV(N)	((N) << 16)
#define SGPIO_BYTES(N)		((N) << 6)
#define SGPIO_ENABLE		1
#define GPIO554			0x554
#define SCU_414			0x414 /* Multi-function Pin Control #5 */
#define SCU_414_SGPM_MASK	GENMASK(27, 24)

	uint32_t value;
	/* set the sgpio clock to pclk/(2*(5+1)) or ~2 MHz */
	value = SGPIO_CLK_DIV(256) | SGPIO_BYTES(10) | SGPIO_ENABLE;
	writel(value, AST_GPIO_BASE + GPIO554);
	writel(readl(SCU_BASE | SCU_414) | SCU_414_SGPM_MASK,
		SCU_BASE | SCU_414);
}

int board_early_init_f(void)
{
#if 0
	port80h_snoop_init();
	sgpio_init();
#endif
	return 0;
}
