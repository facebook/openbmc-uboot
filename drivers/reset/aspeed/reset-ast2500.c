// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2500.h>

struct ast2500_reset_priv {
	/* WDT used to perform resets. */
	struct udevice *wdt;
	struct ast2500_scu *scu;
};

static int ast2500_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct ast2500_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2500_scu *scu = priv->scu;
	int ret = 0;

	debug("ast2500_reset_deassert reset_ctl->id %ld \n", reset_ctl->id);

	if(reset_ctl->id >= 32)
		clrbits_le32(&scu->sysreset_ctrl2 , BIT(reset_ctl->id - 32));
	else
		clrbits_le32(&scu->sysreset_ctrl1 , BIT(reset_ctl->id));

	return ret;
}

static int ast2500_reset_assert(struct reset_ctl *reset_ctl)
{
	struct ast2500_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2500_scu *scu = priv->scu;
//	u32 reset_mode, reset_mask;
//	bool reset_sdram;
	int ret = 0;

	debug("ast2500_reset_assert reset_ctl->id %ld \n", reset_ctl->id);
	/*
	 * To reset SDRAM, a specifal flag in SYSRESET register
	 * needs to be enabled first
	 */
#if 0	 
	reset_mode = ast_reset_mode_from_flags(reset_ctl->id);
	reset_mask = ast_reset_mask_from_flags(reset_ctl->id);
	reset_sdram = reset_mode == WDT_CTRL_RESET_SOC &&
		(reset_mask & WDT_RESET_SDRAM);

	if (reset_sdram) {
		ast_scu_unlock(priv->scu);
		setbits_le32(&priv->scu->sysreset_ctrl1,
			     SCU_SYSRESET_SDRAM_WDT);
		ret = wdt_expire_now(priv->wdt, reset_ctl->id);
		clrbits_le32(&priv->scu->sysreset_ctrl1,
			     SCU_SYSRESET_SDRAM_WDT);
		ast_scu_lock(priv->scu);
	} else {
		ret = wdt_expire_now(priv->wdt, reset_ctl->id);
	}
#endif
	if(reset_ctl->id >= 32)
		setbits_le32(&scu->sysreset_ctrl2 , BIT(reset_ctl->id - 32));
	else
		setbits_le32(&scu->sysreset_ctrl1 , BIT(reset_ctl->id));

	return ret;
}


static int ast2500_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int ast2500_reset_probe(struct udevice *dev)
{
	struct ast2500_reset_priv *priv = dev_get_priv(dev);
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

static int ast2500_ofdata_to_platdata(struct udevice *dev)
{
	struct ast2500_reset_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_WDT, dev, "aspeed,wdt",
					   &priv->wdt);
	if (ret) {
		debug("%s: can't find WDT for reset controller", __func__);
		return ret;
	}

	return 0;
}

static const struct udevice_id aspeed_reset_ids[] = {
	{ .compatible = "aspeed,ast2500-reset" },
	{ }
};

struct reset_ops aspeed_reset_ops = {
	.rst_assert = ast2500_reset_assert,
	.rst_deassert = ast2500_reset_deassert,
	.request = ast2500_reset_request,
};

U_BOOT_DRIVER(aspeed_reset) = {
	.name		= "aspeed_reset",
	.id		= UCLASS_RESET,
	.of_match = aspeed_reset_ids,
	.probe = ast2500_reset_probe,
	.ops = &aspeed_reset_ops,
	.ofdata_to_platdata = ast2500_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct ast2500_reset_priv),
};
