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
#include <linux/io.h>
#include <linux/ioport.h>

#define TIMING_PHASE_OFFSET 0xf4

struct aspeed_sdhci_general_reg {
	u32 genreal_info;
	u32 debounce_setting;
	u32 bus_setting;
};

struct aspeed_sdhci_general_data {
	struct aspeed_sdhci_general_reg *regs;
	struct clk_bulk clks;
};

static int aspeed_sdhci_irq_ofdata_to_platdata(struct udevice *dev)
{
	struct aspeed_sdhci_general_data *priv = dev_get_priv(dev);

	return clk_get_bulk(dev, &priv->clks);
}

static int aspeed_sdhci_irq_probe(struct udevice *dev)
{
	struct aspeed_sdhci_general_data *priv = dev_get_priv(dev);
	int ret = 0;
	struct resource regs;
	void __iomem  *sdhci_ctrl_base;
	u32 timing_phase;

	debug("%s(dev=%p) \n", __func__, dev);

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		pr_debug("fail enable sdhci clk \n");
		return ret;
	}

	ret = dev_read_resource(dev, 0, &regs);
	if (ret < 0)
		return ret;

	sdhci_ctrl_base = (void __iomem  *)regs.start;

	timing_phase = dev_read_u32_default(dev, "timing-phase", 0);
	writel(timing_phase, sdhci_ctrl_base + TIMING_PHASE_OFFSET);

	return 0;
}

static const struct udevice_id aspeed_sdhci_irq_ids[] = {
	{ .compatible = "aspeed,aspeed-sdhci-irq" },
	{ .compatible = "aspeed,aspeed-emmc-irq" },
	{ }
};

U_BOOT_DRIVER(aspeed_sdhci_ic) = {
	.name		= "aspeed_sdhci_ic",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_sdhci_irq_ids,
	.probe		= aspeed_sdhci_irq_probe,
	.ofdata_to_platdata = aspeed_sdhci_irq_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct aspeed_sdhci_general_data),
};
