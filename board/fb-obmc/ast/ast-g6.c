// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Facebook Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <asm/arch/fmc_dual_boot_ast2600.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <dm/uclass.h>
#include "util.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	struct udevice *dev;
	int i;
	int ret;
	u64 rev_id;
	u32 tmp_val;

	/* disable address remapping for A1 to prevent secure boot reboot failure */
	rev_id = readl(ASPEED_REVISION_ID0);
	rev_id = ((u64)readl(ASPEED_REVISION_ID1) << 32) | rev_id;

	if (rev_id == 0x0501030305010303 || rev_id == 0x0501020305010203) {
		if ((readl(ASPEED_SB_STS) & BIT(6))) {
			tmp_val = readl(0x1e60008c) & (~BIT(0));
			writel(0xaeed1a03, 0x1e600000);
			writel(tmp_val, 0x1e60008c);
			writel(0x1, 0x1e600000);
		}
	}

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);

#if CONFIG_IS_ENABLED(FMC_DUAL_BOOT)
	fmc_enable_dual_boot();
#endif

	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	i = 0;
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}


	vboot_check_enforce();

	return 0;
}
