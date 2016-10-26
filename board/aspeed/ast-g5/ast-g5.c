/*
 * Copyright 2016 IBM Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <common.h>
#include <netdev.h>

#include <asm/arch/ast_scu.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

void watchdog_init()
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
#define AST_WDT_CLK (1*1000*1000) /* 1M clock source */
    u32 reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
    /* set the reload value */
    __raw_writel(reload, AST_WDT_BASE + 0x04);
    /* magic word to reload */
    __raw_writel(0x4755, AST_WDT_BASE + 0x08);
    /* start the watchdog with 1M clk src and reset whole chip */
    __raw_writel(0x33, AST_WDT_BASE + 0x0c);
    printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}

int board_init(void)
{

	watchdog_init();

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gd->flags = 0;

	return 0;
}

int dram_init(void)
{
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = dram - vga - (64*1024*1024); // ECC size = dram/8

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
