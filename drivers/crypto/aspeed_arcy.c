// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright ASPEED Technology Inc.
 */
#include <common.h>
#include <clk.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>

struct aspeed_arcy {
	struct clk clk;
};

static int aspeed_arcy_probe(struct udevice *dev)
{
	struct aspeed_arcy *arcy = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_index(dev, 0, &arcy->clk);
	if (ret < 0) {
		debug("Can't get clock for %s: %d\n", dev->name, ret);
		return ret;
	}

	ret = clk_enable(&arcy->clk);
	if (ret) {
		debug("Failed to enable arcy clock (%d)\n", ret);
		return ret;
	}

	return ret;
}

static int aspeed_arcy_remove(struct udevice *dev)
{
	struct aspeed_arcy *arcy = dev_get_priv(dev);

	clk_disable(&arcy->clk);

	return 0;
}

static const struct udevice_id aspeed_arcy_ids[] = {
	{ .compatible = "aspeed,ast2600-arcy" },
	{ }
};

U_BOOT_DRIVER(aspeed_arcy) = {
	.name = "aspeed_arcy",
	.id = UCLASS_MISC,
	.of_match = aspeed_arcy_ids,
	.probe = aspeed_arcy_probe,
	.remove = aspeed_arcy_remove,
	.priv_auto_alloc_size = sizeof(struct aspeed_arcy),
	.flags = DM_FLAG_PRE_RELOC,
};
