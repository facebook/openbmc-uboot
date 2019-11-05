// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>

struct ast2600_reset_priv {
	/* WDT used to perform resets. */
	struct udevice *wdt;
	struct ast2600_scu *scu;
};

static int ast2600_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;

	debug("ast2600_reset_assert reset_ctl->id %ld \n", reset_ctl->id);

	if(reset_ctl->id >= 32)
		writel(BIT(reset_ctl->id - 32), &scu->sysreset_clr_ctrl2);
	else
		writel(BIT(reset_ctl->id), &scu->sysreset_clr_ctrl1);

	return 0;
}

static int ast2600_reset_assert(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;	

	debug("ast2600_reset_assert reset_ctl->id %ld \n", reset_ctl->id);

	if(reset_ctl->id >= 32)
		writel(BIT(reset_ctl->id - 32), &scu->sysreset_ctrl2);
	else
		writel(BIT(reset_ctl->id), &scu->sysreset_ctrl1);

	return 0;
}

static int ast2600_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int ast2600_reset_probe(struct udevice *dev)
{
	struct ast2600_reset_priv *priv = dev_get_priv(dev);
	struct udevice *clk_dev;	
	int ret = 0;

	/* find SCU base address from clock device */
	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_GET_DRIVER(aspeed_scu),
                                          &clk_dev);
    if (ret) {
            debug("clock device not found\n");
            return ret;
    }

	priv->scu = devfdt_get_addr_ptr(clk_dev);
	if (IS_ERR(priv->scu)) {
	        debug("%s(): can't get SCU\n", __func__);
	        return PTR_ERR(priv->scu);
	}

	return 0;
}

static int ast2600_ofdata_to_platdata(struct udevice *dev)
{
//	struct ast2600_reset_priv *priv = dev_get_priv(dev);
//	int ret;

#if 0
	ret = uclass_get_device_by_phandle(UCLASS_WDT, dev, "aspeed,ast2600-wdt",
					   &priv->wdt);
	if (ret) {
		printf("%s: can't find WDT for reset controller", __func__);
		return ret;
	}
#endif	

	return 0;
}

static const struct udevice_id aspeed_reset_ids[] = {
	{ .compatible = "aspeed,ast2600-reset" },
	{ }
};

struct reset_ops aspeed_reset_ops = {
	.rst_assert = ast2600_reset_assert,
	.rst_deassert = ast2600_reset_deassert,
	.request = ast2600_reset_request,
};

U_BOOT_DRIVER(aspeed_reset) = {
	.name		= "aspeed_reset",
	.id		= UCLASS_RESET,
	.of_match = aspeed_reset_ids,
	.probe = ast2600_reset_probe,
	.ops = &aspeed_reset_ops,
	.ofdata_to_platdata = ast2600_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct ast2600_reset_priv),
};
