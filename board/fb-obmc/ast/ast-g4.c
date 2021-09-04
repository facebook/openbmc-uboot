/*
 * (C) Copyright 2002 Ryan Chen
 * Copyright 2016 IBM Corporation
 * Copyright 2021 Facebook Corportation
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

#include <asm/arch/platform.h>
#include <asm/io.h>
#include "util.h"

#define HW_VERSION_BASE 0x1e6e207c

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	unsigned long reg;

	/* Flash Controller */
#ifdef  CONFIG_FLASH_AST2300
	*((volatile ulong*) 0x1e620000) |= 0x800f0000;      /* enable Flash Write */
#else
	*((volatile ulong*) 0x16000000) |= 0x00001c00;      /* enable Flash Write */
#endif

	/* SCU */
	*((volatile ulong *)0x1e6e2000) = 0x1688A8A8; /* unlock SCU */
	reg = *((volatile ulong *)0x1e6e2008);
	reg &= 0x1c0fffff;
	reg |= 0x61800000; /* PCLK  = HPLL/8 */
	*((volatile ulong *)0x1e6e2008) = reg;

	/* arch number */
	gd->bd->bi_arch_number = MACH_TYPE_ASPEED;

	/* set the clock source of WDT2 for 1MHz */
	*((volatile ulong *)0x1e78502C) |= 0x10;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#if CONFIG_IS_ENABLED(ASPEED_ENABLE_DUAL_BOOT_WATCHDOG)
	dual_boot_watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#else
	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif

	return 0;
}

int misc_init_r(void)
{
	u32 reg, revision, chip_id;

	/* Show H/W Version */
	reg = readl(HW_VERSION_BASE);

	chip_id = (reg & 0xff000000) >> 24;
	revision = (reg & 0xff0000) >> 16;

	puts("H/W:   ");
	if (chip_id == 1) {
		if (revision >= 0x80)
			printf("AST2300 series FPGA Rev. %02x \n", revision);
		else
			printf("AST2300 series chip Rev. %02x \n", revision);
	} else if (chip_id == 2)
		printf("AST2400 series chip Rev. %02x \n", revision);
	else if (chip_id == 0)
		printf("AST2050/AST2150 series chip\n");

	if (env_get("verify") == NULL)
		env_set("verify", "n");
	if (env_get("eeprom") == NULL)
		env_set("eeprom", "y");

	return 0;
}

