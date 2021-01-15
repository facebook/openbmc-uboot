// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Fuzhou Rockchip Electronics Co., Ltd
 *
 * Rockchip SD Host Controller Interface
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <mapmem.h>
#include <sdhci.h>
#include <clk.h>

/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000

struct aspeed_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	unsigned int f_max;
};

struct aspeed_sdhci_priv {
	struct sdhci_host *host;
	struct clk clk;	
};

static int aspeed_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct aspeed_sdhci_plat *plat = dev_get_platdata(dev);
	struct aspeed_sdhci_priv *prv = dev_get_priv(dev);
	struct sdhci_host *host = prv->host;
	unsigned long clock;
	struct clk clk;
	int ret;
#ifndef CONFIG_SPL_BUILD
	int node = dev_of_offset(dev);
#endif

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		pr_debug("%s: Can't get clock for %s: %d\n", __func__, dev->name,
		      ret);
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	debug("%s: CLK %ld\n", __func__, clock);

#ifndef CONFIG_SPL_BUILD
	//1: sd card pwr, 0: no pwr
	gpio_request_by_name_nodev(offset_to_ofnode(node), "pwr-gpios", 0,
				   &host->pwr_gpio, GPIOD_IS_OUT);
	if (dm_gpio_is_valid(&host->pwr_gpio)) {
		printf("\n");
		dm_gpio_set_value(&host->pwr_gpio, 1);
		if (ret) {
			debug("MMC not configured\n");
			return ret;
		}
	}

	//1: 3.3v, 0: 1.8v
	gpio_request_by_name_nodev(offset_to_ofnode(node), "pwr-sw-gpios", 0,
				   &host->pwr_sw_gpio, GPIOD_IS_OUT);

	if (dm_gpio_is_valid(&host->pwr_sw_gpio)) {
		dm_gpio_set_value(&host->pwr_sw_gpio, 1);
		if (ret) {
			debug("MMC not configured\n");
			return ret;
		}
	}
#endif
//	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;
	host->max_clk = clock;

	host->bus_width = dev_read_u32_default(dev, "bus-width", 4);

	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;

	ret = sdhci_setup_cfg(&plat->cfg, host, host->max_clk, EMMC_MIN_FREQ);

	host->mmc = &plat->mmc;
	if (ret)
		return ret;

	host->mmc->drv_type = dev_read_u32_default(dev, "sdhci-drive-type", 0);
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int aspeed_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct aspeed_sdhci_priv *priv = dev_get_priv(dev);

	priv->host = calloc(1, sizeof(struct sdhci_host));
	if (!priv->host)
			return -1;

	priv->host->name = dev->name;
	priv->host->ioaddr = (void *)dev_read_addr(dev);

	return 0;
}

static int aspeed_sdhci_bind(struct udevice *dev)
{
	struct aspeed_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id aspeed_sdhci_ids[] = {
	{ .compatible = "aspeed,sdhci-ast2500" },
	{ .compatible = "aspeed,sdhci-ast2600" },
	{ .compatible = "aspeed,emmc-ast2600" },
	{ }
};

U_BOOT_DRIVER(aspeed_sdhci_drv) = {
	.name		= "aspeed_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= aspeed_sdhci_ids,
	.ofdata_to_platdata = aspeed_sdhci_ofdata_to_platdata,
	.ops		= &sdhci_ops,
	.bind		= aspeed_sdhci_bind,
	.probe		= aspeed_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct aspeed_sdhci_priv),
	.platdata_auto_alloc_size = sizeof(struct aspeed_sdhci_plat),
};
