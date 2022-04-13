// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Facebook Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/fmc_dual_boot_ast2600.h>

int board_init(void)
{
#if CONFIG_IS_ENABLED(FMC_DUAL_BOOT)
	fmc_enable_dual_boot();
#endif

	return 0;
}
