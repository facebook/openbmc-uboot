/*
 * (C) Copyright 2019-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/arch/wdt.h>

int watchdog_init(u32 timeout_sec)
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
	struct udevice *wdt;
	int ret;
	u32 timeout_ms = timeout_sec * 1000;

	ret = uclass_first_device_err(UCLASS_WDT, &wdt);
	if (ret) {
		printf("No WDT device: %d\n", ret);
		return ret;
	}

	ret = wdt_start(wdt, timeout_ms, WDT_CTRL_RESET_CHIP);
	if (ret) {
		printf("Start WDT%u %us failed: %d\n",
		       wdt->seq, timeout_sec, ret);
		return ret;
	}

	printf("Watchdog: %us\n", timeout_sec);
#endif
	return 0;
}



