// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
#include <asm/arch/scu_ast2500.h>
#endif

#define WDT_AST2600	2600
#define WDT_AST2500	2500
#define WDT_AST2400	2400

struct ast_wdt_priv {
	struct ast_wdt *regs;
};

#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
static int ast_wdt_setup_rst_pulse(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	// TODO: get pulse reset information from device tree
	u32 reset_pulse = SET_WDT_RST_PULSE_POLARITY_HIGH | 0xFF;
	writel(reset_pulse, &priv->regs->reset_width);
	return 0;
}
#endif

static int ast_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	ulong driver_data = dev_get_driver_data(dev);
	u32 reset_mode = ast_reset_mode_from_flags(flags);
	/* watchdog timer clock is fixed at 1MHz */
	u32 timeout_us = (u32)timeout_ms * 1000;

	dev_info(dev, "wdt%u set timeout after %uus\n", dev->seq, timeout_us);

#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
	int ret;
	dev_info(dev, "Enable WDT trigger external reset\n");
	ret = ast_scu_enable_wdtrst1();
	if (ret)
		return ret;
	ret = ast_wdt_setup_rst_pulse(dev);
	if (ret)
		return ret;
#endif
	clrsetbits_le32(&priv->regs->ctrl,
			WDT_CTRL_RESET_MASK << WDT_CTRL_RESET_MODE_SHIFT,
			reset_mode << WDT_CTRL_RESET_MODE_SHIFT);

	if (driver_data == WDT_AST2500 && reset_mode == WDT_CTRL_RESET_SOC)
		writel(ast_reset_mask_from_flags(flags),
		       &priv->regs->reset_mask);

	writel(timeout_us, &priv->regs->counter_reload_val);
	writel(WDT_COUNTER_RESTART_VAL, &priv->regs->counter_restart);
	/*
	 * Setting CLK1MHZ bit is just for compatibility with ast2400 part.
	 * On ast2500 watchdog timer clock is fixed at 1MHz and the bit is
	 * read-only
	 * Based on configuration to enable 2ND_BOOT or TRIGGER WDTRST1
	 */
	clrbits_le32(&priv->regs->ctrl, WDT_CTRL_2ND_BOOT | WDT_CTRL_EXT);
	setbits_le32(&priv->regs->ctrl,
		#ifdef CONFIG_ASPEED_WATCHDOG_2ND_BOOT
		        WDT_CTRL_2ND_BOOT |
		#endif
		#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
			WDT_CTRL_EXT |
		#endif
		     WDT_CTRL_EN | WDT_CTRL_RESET | WDT_CTRL_CLK1MHZ);

	return 0;
}

static int ast_wdt_stop(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	ulong driver_data = dev_get_driver_data(dev);

	dev_info(dev, "Watch Dog stopped.\n");
	clrbits_le32(&priv->regs->ctrl, WDT_CTRL_EN);

        if(driver_data == WDT_AST2600) {
		writel(0x030f1ff1, &priv->regs->reset_mask);
		writel(0x3fffff1, &priv->regs->reset_mask2);
	} else {
		writel(WDT_RESET_DEFAULT, &priv->regs->reset_mask);
	}

	writel(WDT_RESET_DEFAULT, &priv->regs->reset_mask);
	return 0;
}

static int ast_wdt_reset(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

	dev_info(dev, "Watch Dog reload.\n");
	writel(WDT_COUNTER_RESTART_VAL, &priv->regs->counter_restart);

	return 0;
}

static int ast_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ast_wdt_start(dev, 1, flags);
	if (ret)
		return ret;

	while (readl(&priv->regs->ctrl) & WDT_CTRL_EN)
		;

	return ast_wdt_stop(dev);
}

static int ast_wdt_ofdata_to_platdata(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	return 0;
}

static const struct wdt_ops ast_wdt_ops = {
	.start = ast_wdt_start,
	.reset = ast_wdt_reset,
	.stop = ast_wdt_stop,
	.expire_now = ast_wdt_expire_now,
};

static int ast_wdt_probe(struct udevice *dev)
{
	dev_dbg(dev, "%s() wdt%u\n", __func__, dev->seq);
	ast_wdt_stop(dev);

	return 0;
}

static const struct udevice_id ast_wdt_ids[] = {
	{ .compatible = "aspeed,wdt", .data = WDT_AST2500 },
	{ .compatible = "aspeed,ast2600-wdt", .data = WDT_AST2600 },
	{ .compatible = "aspeed,ast2500-wdt", .data = WDT_AST2500 },
	{ .compatible = "aspeed,ast2400-wdt", .data = WDT_AST2400 },
	{}
};

U_BOOT_DRIVER(ast_wdt) = {
	.name = "ast_wdt",
	.id = UCLASS_WDT,
	.of_match = ast_wdt_ids,
	.probe = ast_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct ast_wdt_priv),
	.ofdata_to_platdata = ast_wdt_ofdata_to_platdata,
	.ops = &ast_wdt_ops,
};
