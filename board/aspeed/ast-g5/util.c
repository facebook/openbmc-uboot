/*
 * (C) Copyright 2019-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <wdt.h>
#include <asm/arch/wdt.h>

#define PFR_PROV_STS  0x0A
#define PFR_BMC_CHKPT 0x0F

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

int pfr_checkpoint(uint cmd) {
  int ret = -1;
#ifdef CONFIG_PFR_BUS
  int retry = 2;
  struct udevice *bus, *dev;

  do {
    ret = uclass_get_device_by_name(UCLASS_I2C, CONFIG_PFR_BUS, &bus);
    if (ret)
      break;

    ret = i2c_get_chip(bus, CONFIG_PFR_ADDR, 1, &dev);
    if (ret)
      break;

    do {
      ret = dm_i2c_reg_read(dev, PFR_PROV_STS);
      if ((ret < 0) && retry)
        udelay(10000);
    } while ((ret < 0) && (retry-- > 0));

    if ((ret <= 0) || !(ret & 0x20)) {
      ret = -1;
      break;
    }

    retry = 2;
    do {
      ret = dm_i2c_reg_write(dev, PFR_BMC_CHKPT, cmd);
      if (ret && retry)
        udelay(10000);
    } while (ret && (retry-- > 0));
  } while (0);
#endif
  if (ret)
    return ret;

  debug("pfr_checkpoint: 0x%02x\n", cmd);
  return ret;
}
