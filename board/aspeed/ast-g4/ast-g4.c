/*
 * (C) Copyright 2002 Ryan Chen
 * Copyright 2016 IBM Corporation
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

#include <asm/arch/platform.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/arch/ast_scu.h>
#include <asm/arch/regs-ahbc.h>
#include <asm/arch/regs-scu.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gd->flags = 0;
	return 0;
}

int misc_init_r(void)
{
	u32 reg;

	/* Unlock AHB controller */
	writel(AHBC_PROTECT_UNLOCK, AST_AHBC_BASE);

	/* Map DRAM to 0x00000000 */
	reg = readl(AST_AHBC_BASE + AST_AHBC_ADDR_REMAP);
	writel(reg | BIT(0), AST_AHBC_BASE + AST_AHBC_ADDR_REMAP);

	/* Unlock SCU */
	writel(SCU_PROTECT_UNLOCK, AST_SCU_BASE);

	/*
	 * The original file contained these comments.
	 * TODO: verify the register write does what it claims
	 *
	 * LHCLK = HPLL/8
	 * PCLK  = HPLL/8
	 * BHCLK = HPLL/8
	 */
	reg = readl(AST_SCU_BASE + AST_SCU_CLK_SEL);
	reg &= 0x1c0fffff;
	reg |= 0x61800000;
	writel(reg, AST_SCU_BASE + AST_SCU_CLK_SEL);

	return 0;
}

int dram_init(void)
{
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = dram - vga;

	return 0;
}

#ifdef CONFIG_FTGMAC100
int board_eth_init(bd_t *bd)
{
	return ftgmac100_initialize(bd);
}
#endif

#ifdef CONFIG_ASPEEDNIC
int board_eth_init(bd_t *bd)
{
	return aspeednic_initialize(bd);
}
#endif
