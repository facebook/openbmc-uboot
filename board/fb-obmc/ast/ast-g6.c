// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Facebook Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <dm/uclass.h>
#include "util.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FBGC
static void system_status_led_init(void)
{
	/* GPIO088: GPIO_U/V/W/X Data Value Register
	 * 31:24 Port GPIOX[7:0] data register
	 * 23:16 Port GPIOW[7:0] data register
	 * 15:8 Port GPIOV[7:0] data register
	 * 7:0 Port GPIOU[7:0] data register
	 */
	u32 value = readl(0x1e780088);
	/* GPIO08C: GPIO_U/V/W/X Direction Value Register
	 * 31:24 Port GPIOX[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 23:16 Port GPIOW[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 15:8 Port GPIOV[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 7:0 Reserved
	 * GPIOU is input only
	 */
	u32 direction = readl(0x1e78008C);

	/* GPIOV4: BMC_LED_STATUS_BLUE_EN_R
	 * 0: LED OFF 1:LED ON BLUE
	 * GPIOV5: BMC_LED_STATUS_YELLOW_EN_R
	 * 0: LED ON YELLOW 1:LED OFF
	 */
	direction |= 0x3000; // set GPIOV4 & GPIOV5 direction output
	value &= 0xFFFFCFFF; // set GPIOV4 & GPIOV5 value 0
	writel(direction, 0x1e78008C);
	writel(value, 0x1e780088);
}

#endif  /* CONFIG_FBGC */

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

#if CONFIG_IS_ENABLED(ASPEED_ENABLE_DUAL_BOOT_WATCHDOG)
	dual_boot_watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#else
	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
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

#ifdef CONFIG_FBGC
	system_status_led_init();
#endif /* CONFIG_FBGC */

	return 0;
}
