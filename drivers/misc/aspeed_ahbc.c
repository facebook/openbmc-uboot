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
#include <asm/arch/ahbc_aspeed.h>
#include <linux/types.h>

#define AHBC_UNLOCK	0xaeed1a03
struct aspeed_ahbc_priv {
	struct aspeed_ahbc_reg *ahbc;
};

extern void aspeed_ahbc_remap_enable(struct aspeed_ahbc_reg *ahbc)
{
	uint32_t tmp_val;

	tmp_val = readl(&ahbc->addr_remap);
	tmp_val |= 0x20;
	writel(AHBC_UNLOCK, &ahbc->protection_key);
	writel(tmp_val, &ahbc->addr_remap);
	writel(0x1, &ahbc->protection_key);
}

static int aspeed_ahbc_probe(struct udevice *dev)
{
	struct aspeed_ahbc_priv *priv = dev_get_priv(dev);

	debug("%s(dev=%p) \n", __func__, dev);

	priv->ahbc = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->ahbc))
		return PTR_ERR(priv->ahbc);

	return 0;
}

static const struct udevice_id aspeed_ahbc_ids[] = {
	{ .compatible = "aspeed,aspeed-ahbc" },
	{ }
};

U_BOOT_DRIVER(aspeed_ahbc) = {
	.name		= "aspeed_ahbc",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_ahbc_ids,
	.probe		= aspeed_ahbc_probe,
	.priv_auto_alloc_size = sizeof(struct aspeed_ahbc_priv),
};
