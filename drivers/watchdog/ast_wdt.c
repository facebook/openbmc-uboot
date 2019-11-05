// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <asm/io.h>

/*
 * Special value that needs to be written to counter_restart register to
 * (re)start the timer
 */
#define WDT_COUNTER_RESTART_VAL		0x4755

/* Control register */
#define WDT_CTRL_RESET_MODE_SHIFT	5
#define WDT_CTRL_RESET_MODE_MASK	3

#define WDT_CTRL_EN			(1 << 0)
#define WDT_CTRL_RESET			(1 << 1)
#define WDT_CTRL_CLK1MHZ		(1 << 4)
#define WDT_CTRL_2ND_BOOT		(1 << 7)

/* Values for Reset Mode */
#define WDT_CTRL_RESET_SOC		0
#define WDT_CTRL_RESET_CHIP		1
#define WDT_CTRL_RESET_CPU		2
#define WDT_CTRL_RESET_MASK		3

/* Reset Mask register */
#define WDT_RESET_ARM			(1 << 0)
#define WDT_RESET_COPROC		(1 << 1)
#define WDT_RESET_SDRAM			(1 << 2)
#define WDT_RESET_AHB			(1 << 3)
#define WDT_RESET_I2C			(1 << 4)
#define WDT_RESET_MAC1			(1 << 5)
#define WDT_RESET_MAC2			(1 << 6)
#define WDT_RESET_GCRT			(1 << 7)
#define WDT_RESET_USB20			(1 << 8)
#define WDT_RESET_USB11_HOST		(1 << 9)
#define WDT_RESET_USB11_EHCI2		(1 << 10)
#define WDT_RESET_VIDEO			(1 << 11)
#define WDT_RESET_HAC			(1 << 12)
#define WDT_RESET_LPC			(1 << 13)
#define WDT_RESET_SDSDIO		(1 << 14)
#define WDT_RESET_MIC			(1 << 15)
#define WDT_RESET_CRT2C			(1 << 16)
#define WDT_RESET_PWM			(1 << 17)
#define WDT_RESET_PECI			(1 << 18)
#define WDT_RESET_JTAG			(1 << 19)
#define WDT_RESET_ADC			(1 << 20)
#define WDT_RESET_GPIO			(1 << 21)
#define WDT_RESET_MCTP			(1 << 22)
#define WDT_RESET_XDMA			(1 << 23)
#define WDT_RESET_SPI			(1 << 24)
#define WDT_RESET_MISC			(1 << 25)

#define WDT_RESET_DEFAULT						\
	(WDT_RESET_ARM | WDT_RESET_COPROC | WDT_RESET_I2C |		\
	 WDT_RESET_MAC1 | WDT_RESET_MAC2 | WDT_RESET_GCRT |		\
	 WDT_RESET_USB20 | WDT_RESET_USB11_HOST | WDT_RESET_USB11_EHCI2 | \
	 WDT_RESET_VIDEO | WDT_RESET_HAC | WDT_RESET_LPC |		\
	 WDT_RESET_SDSDIO | WDT_RESET_MIC | WDT_RESET_CRT2C |		\
	 WDT_RESET_PWM | WDT_RESET_PECI | WDT_RESET_JTAG |		\
	 WDT_RESET_ADC | WDT_RESET_GPIO | WDT_RESET_MISC)

enum aspeed_wdt_model {
	WDT_AST2400,
	WDT_AST2500,
	WDT_AST2600,
};

struct ast_wdt {
	u32 counter_status;
	u32 counter_reload_val;
	u32 counter_restart;
	u32 ctrl;
	u32 timeout_status;
	u32 clr_timeout_status;
	u32 reset_width;
	/* On pre-ast2500 SoCs this register is reserved. */
	u32 reset_mask1;
	u32 reset_mask2;	//ast2600 support
	u32 sw_ctrl;	//ast2600 support
	u32 sw_reset_mask1;	//ast2600 support
	u32 sw_reset_mask2;	//ast2600 support
	u32 sw_fun_disable;	//ast2600 support
	
};

struct ast_wdt_priv {
	struct ast_wdt *regs;
};

static int ast_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

	writel((u32) timeout, &priv->regs->counter_reload_val);
	
	writel(WDT_COUNTER_RESTART_VAL, &priv->regs->counter_restart);

	writel(WDT_CTRL_EN | WDT_CTRL_RESET, &priv->regs->ctrl);

	return 0;
}

static int ast_wdt_stop(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	ulong driver_data = dev_get_driver_data(dev);

	clrbits_le32(&priv->regs->ctrl, WDT_CTRL_EN);

	if(driver_data == WDT_AST2600) {
		writel(0x030f1ff1, &priv->regs->reset_mask1);
		writel(0x3fffff1, &priv->regs->reset_mask1);		
	} else 
		writel(WDT_RESET_DEFAULT, &priv->regs->reset_mask1);
	
	return 0;
}

static int ast_wdt_reset(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

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

	while (readl(&priv->regs->ctrl) & WDT_CTRL_EN);

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
	debug("%s() wdt%u\n", __func__, dev->seq);
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
