// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Chia-Wei Wang <chiawei_wang@aspeedtech.com>
 */

#include <common.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

void enable_caches(void)
{
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	enum dcache_option opt = DCACHE_WRITETHROUGH;
#else
	enum dcache_option opt = DCACHE_WRITEBACK;
#endif
	/* enable D-cache as well as MMU */
	dcache_enable();

	/* setup cache attribute for DRAM region */
	mmu_set_region_dcache_behaviour(ASPEED_DRAM_BASE,
				       gd->ram_size,
				       opt);
}
