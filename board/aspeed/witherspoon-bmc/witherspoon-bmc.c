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

#include <asm/arch/ast-sdmc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gd->flags = 0;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = ast_sdmc_get_mem_size();

	return 0;
}

int board_eth_init(bd_t *bd)
{
	return aspeednic_initialize(bd);
}
