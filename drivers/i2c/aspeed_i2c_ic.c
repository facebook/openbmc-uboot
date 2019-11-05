// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <reset.h>
#include <asm/arch/scu_ast2500.h>

static int aspeed_i2c_global_probe(struct udevice *dev)
{
	struct reset_ctl reset_ctl;
	int ret = 0;

	debug("%s(dev=%p) \n", __func__, dev);

	ret = reset_get_by_index(dev, 0, &reset_ctl);

	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	reset_assert(&reset_ctl);
	reset_deassert(&reset_ctl);

	return 0;
}

static const struct udevice_id aspeed_i2c_global_ids[] = {
	{ .compatible = "aspeed,ast2500-i2c-ic" },
	{ .compatible = "aspeed,ast2600-i2c-global" },
	{ }
};

U_BOOT_DRIVER(aspeed_i2c_ic) = {
	.name		= "aspeed_i2c_ic",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_i2c_global_ids,
	.probe		= aspeed_i2c_global_probe,
};
