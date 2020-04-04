// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>

#include "ehci.h"

struct ehci_aspeed_priv {
	struct ehci_ctrl ehci;
};

static void aspeed_ehci_powerup_fixup(struct ehci_ctrl *ctrl,
				     uint32_t *status_reg, uint32_t *reg)
{
	mdelay(50);
	/* This is to avoid PORT_ENABLE bit to be cleared in "ehci-hcd.c". */
	*reg |= EHCI_PS_PE;

	/* For EHCI_PS_CSC to be cleared in ehci_hcd.c */
	if (ehci_readl(status_reg) & EHCI_PS_CSC)
		*reg |= EHCI_PS_CSC;
}

static const struct ehci_ops aspeed_ehci_ops = {
	.powerup_fixup		= aspeed_ehci_powerup_fixup,
};

static int ehci_aspeed_probe(struct udevice *dev)
{
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	fdt_addr_t hcd_base;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	/*
	* Get the base address for EHCI controller from the device node
	*/
	hcd_base = devfdt_get_addr(dev);
	if (hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the EHCI register base address\n");
		return -ENXIO;
	}

	hccr = (struct ehci_hccr *)hcd_base;
	hcor = (struct ehci_hcor *)
	((u32)hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	debug("echi-aspeed: init hccr %x and hcor %x hc_length %d\n",
	(u32)hccr, (u32)hcor,
	(u32)HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, &aspeed_ehci_ops, 0, USB_INIT_HOST);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "aspeed,aspeed-ehci", },
	{ }
};

U_BOOT_DRIVER(ehci_aspeed) = {
	.name           = "ehci_aspeed",
	.id             = UCLASS_USB,
	.of_match       = ehci_usb_ids,
	.probe          = ehci_aspeed_probe,
	.remove         = ehci_deregister,
	.ops            = &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_aspeed_priv),
	.flags          = DM_FLAG_ALLOC_PRIV_DMA,
};
