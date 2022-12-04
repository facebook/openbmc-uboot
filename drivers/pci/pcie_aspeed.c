// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <dm.h>
#include <reset.h>
#include <fdtdec.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/ahbc_aspeed.h>
#include "pcie_aspeed.h"

DECLARE_GLOBAL_DATA_PTR;

struct pcie_aspeed {
	struct aspeed_h2x_reg *h2x_reg;
};

static u8 txTag;

void aspeed_pcie_set_slot_power_limit(struct pcie_aspeed *pcie, int slot)
{
	u32 timeout = 0;
	struct aspeed_h2x_reg *h2x_reg = pcie->h2x_reg;

	//optional : set_slot_power_limit
	switch (slot) {
	case 0:
		writel(BIT(4) | readl(&h2x_reg->h2x_rc_l_ctrl),
		       &h2x_reg->h2x_rc_l_ctrl);
		break;
	case 1:
		writel(BIT(4) | readl(&h2x_reg->h2x_rc_h_ctrl),
		       &h2x_reg->h2x_rc_h_ctrl);
		break;
	}

	txTag %= 0x7;

	writel(0x74000001, &h2x_reg->h2x_tx_desc3);
	switch (slot) {
	case 0:	//write for 0.8.0
		writel(0x00400050 | (txTag << 8), &h2x_reg->h2x_tx_desc2);
		break;
	case 1:	//write for 0.4.0
		writel(0x00200050 | (txTag << 8), &h2x_reg->h2x_tx_desc2);
		break;
	}
	writel(0x0, &h2x_reg->h2x_tx_desc1);
	writel(0x0, &h2x_reg->h2x_tx_desc0);

	writel(0x1a, &h2x_reg->h2x_tx_data);

	//trigger tx
	writel(PCIE_TRIGGER_TX, &h2x_reg->h2x_reg24);

	//wait tx idle
	while (!(readl(&h2x_reg->h2x_reg24) & BIT(31))) {
		timeout++;
		if (timeout > 1000)
			goto out;
	};

	//write clr tx idle
	writel(1, &h2x_reg->h2x_reg08);
	timeout = 0;

	switch (slot) {
	case 0:
		//check tx status and clr rx done int
		while (!(readl(&h2x_reg->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR)) {
			timeout++;
			if (timeout > 10)
				break;
			mdelay(1);
		}
		writel(PCIE_RC_RX_DONE_ISR, &h2x_reg->h2x_rc_l_isr);
		break;
	case 1:
		//check tx status and clr rx done int
		while (!(readl(&h2x_reg->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR)) {
			timeout++;
			if (timeout > 10)
				break;
			mdelay(1);
		}
		writel(PCIE_RC_RX_DONE_ISR, &h2x_reg->h2x_rc_h_isr);
		break;
	}
out:
	txTag++;
}

static void aspeed_pcie_cfg_read(struct pcie_aspeed *pcie, pci_dev_t bdf,
				 uint offset, ulong *valuep)
{
	struct aspeed_h2x_reg *h2x_reg = pcie->h2x_reg;
	u32 timeout = 0;
	u32 bdf_offset;
	u32 type = 0;
	int rx_done_fail = 0;

	//H2X80[4] (unlock) is write-only.
	//Driver may set H2X80/H2XC0[4]=1 before triggering next TX config.
	writel(BIT(4) | readl(&h2x_reg->h2x_rc_l_ctrl),
	       &h2x_reg->h2x_rc_l_ctrl);
	writel(BIT(4) | readl(&h2x_reg->h2x_rc_h_ctrl),
	       &h2x_reg->h2x_rc_h_ctrl);

	if (PCI_BUS(bdf) == 0)
		type = 0;
	else
		type = 1;

	bdf_offset = (PCI_BUS(bdf) << 24) |
					(PCI_DEV(bdf) << 19) |
					(PCI_FUNC(bdf) << 16) |
					(offset & ~3);

	txTag %= 0x7;

	writel(0x04000001 | (type << 24), &h2x_reg->h2x_tx_desc3);
	writel(0x0000200f | (txTag << 8), &h2x_reg->h2x_tx_desc2);
	writel(bdf_offset, &h2x_reg->h2x_tx_desc1);
	writel(0x00000000, &h2x_reg->h2x_tx_desc0);

	//trigger tx
	writel(PCIE_TRIGGER_TX, &h2x_reg->h2x_reg24);

	//wait tx idle
	while (!(readl(&h2x_reg->h2x_reg24) & PCIE_TX_IDLE)) {
		timeout++;
		if (timeout > 1000) {
			*valuep = 0xffffffff;
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x_reg->h2x_reg08);

	timeout = 0;
	//check tx status
	switch (readl(&h2x_reg->h2x_reg24) & PCIE_STATUS_OF_TX) {
	case PCIE_RC_L_TX_COMPLETE:
		while (!(readl(&h2x_reg->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR)) {
			timeout++;
			if (timeout > 10) {
				rx_done_fail = 1;
				*valuep = 0xffffffff;
				break;
			}
			mdelay(1);
		}
		if (!rx_done_fail) {
			if (readl(&h2x_reg->h2x_rc_l_rxdesc2) & BIT(13))
				*valuep = 0xffffffff;
			else
				*valuep = readl(&h2x_reg->h2x_rc_l_rdata);
		}
		writel(BIT(4) | readl(&h2x_reg->h2x_rc_l_ctrl),
		       &h2x_reg->h2x_rc_l_ctrl);
		writel(readl(&h2x_reg->h2x_rc_l_isr),
		       &h2x_reg->h2x_rc_l_isr);
		break;
	case PCIE_RC_H_TX_COMPLETE:
		while (!(readl(&h2x_reg->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR)) {
			timeout++;
			if (timeout > 10) {
				rx_done_fail = 1;
				*valuep = 0xffffffff;
				break;
			}
			mdelay(1);
		}
		if (!rx_done_fail) {
			if (readl(&h2x_reg->h2x_rc_h_rxdesc2) & BIT(13))
				*valuep = 0xffffffff;
			else
				*valuep = readl(&h2x_reg->h2x_rc_h_rdata);
		}
		writel(BIT(4) | readl(&h2x_reg->h2x_rc_h_ctrl),
		       &h2x_reg->h2x_rc_h_ctrl);
		writel(readl(&h2x_reg->h2x_rc_h_isr), &h2x_reg->h2x_rc_h_isr);
		break;
	default:	//read rc data
		*valuep = readl(&h2x_reg->h2x_rdata);
		break;
	}

out:
	txTag++;
}

static void aspeed_pcie_cfg_write(struct pcie_aspeed *pcie, pci_dev_t bdf,
				  uint offset, ulong value,
				  enum pci_size_t size)
{
	struct aspeed_h2x_reg *h2x_reg = pcie->h2x_reg;
	u32 timeout = 0;
	u32 type = 0;
	u32 bdf_offset;
	u8 byte_en = 0;

	writel(BIT(4) | readl(&h2x_reg->h2x_rc_l_ctrl),
	       &h2x_reg->h2x_rc_l_ctrl);
	writel(BIT(4) | readl(&h2x_reg->h2x_rc_h_ctrl),
	       &h2x_reg->h2x_rc_h_ctrl);

	switch (size) {
	case PCI_SIZE_8:
		switch (offset % 4) {
		case 0:
			byte_en = 0x1;
			break;
		case 1:
			byte_en = 0x2;
			break;
		case 2:
			byte_en = 0x4;
			break;
		case 3:
			byte_en = 0x8;
			break;
		}
		break;
	case PCI_SIZE_16:
		switch ((offset >> 1) % 2) {
		case 0:
			byte_en = 0x3;
			break;
		case 1:
			byte_en = 0xc;
			break;
		}
		break;
	default:
		byte_en = 0xf;
		break;
	}

	if (PCI_BUS(bdf) == 0)
		type = 0;
	else
		type = 1;

	bdf_offset = (PCI_BUS(bdf) << 24) |
					(PCI_DEV(bdf) << 19) |
					(PCI_FUNC(bdf) << 16) |
					(offset & ~3);

	txTag %= 0x7;

	writel(0x44000001 | (type << 24), &h2x_reg->h2x_tx_desc3);
	writel(0x00002000 | (txTag << 8) | byte_en, &h2x_reg->h2x_tx_desc2);
	writel(bdf_offset, &h2x_reg->h2x_tx_desc1);
	writel(0x00000000, &h2x_reg->h2x_tx_desc0);

	value = pci_conv_size_to_32(0x0, value, offset, size);

	writel(value, &h2x_reg->h2x_tx_data);

	//trigger tx
	writel(1, &h2x_reg->h2x_reg24);

	//wait tx idle
	while (!(readl(&h2x_reg->h2x_reg24) & BIT(31))) {
		timeout++;
		if (timeout > 1000)
			goto out;
	};

	//write clr tx idle
	writel(1, &h2x_reg->h2x_reg08);

	timeout = 0;
	//check tx status and clr rx done int
	switch (readl(&h2x_reg->h2x_reg24) & PCIE_STATUS_OF_TX) {
	case PCIE_RC_L_TX_COMPLETE:
		while (!(readl(&h2x_reg->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR)) {
			timeout++;
			if (timeout > 10)
				break;
			mdelay(1);
		}
		writel(PCIE_RC_RX_DONE_ISR, &h2x_reg->h2x_rc_l_isr);
		break;
	case PCIE_RC_H_TX_COMPLETE:
		while (!(readl(&h2x_reg->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR)) {
			timeout++;
			if (timeout > 10)
				break;
			mdelay(1);
		}
		writel(PCIE_RC_RX_DONE_ISR, &h2x_reg->h2x_rc_h_isr);
		break;
	}

out:
	txTag++;
}

static int pcie_aspeed_read_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	struct pcie_aspeed *pcie = dev_get_priv(bus);

	debug("PCIE CFG read: (b,d,f)=(%2d,%2d,%2d)\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	/* Only allow one other device besides the local one on the local bus */
	if (PCI_BUS(bdf) == 1 && PCI_DEV(bdf) > 0) {
		debug("- out of range\n");
		/*
		 * If local dev is 0, the first other dev can
		 * only be 1
		 */
		*valuep = pci_get_ff(size);
		return 0;
	}

	if (PCI_BUS(bdf) == 2 && PCI_DEV(bdf) > 0) {
		debug("- out of range\n");
		/*
		 * If local dev is 0, the first other dev can
		 * only be 1
		 */
		*valuep = pci_get_ff(size);
		return 0;
	}

	aspeed_pcie_cfg_read(pcie, bdf, offset, valuep);

	*valuep = pci_conv_32_to_size(*valuep, offset, size);
	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, *valuep);

	return 0;
}

static int pcie_aspeed_write_config(struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	struct pcie_aspeed *pcie = dev_get_priv(bus);

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);

	aspeed_pcie_cfg_write(pcie, bdf, offset, value, size);

	return 0;
}

void aspeed_pcie_rc_slot_enable(struct pcie_aspeed *pcie, int slot)

{
	struct aspeed_h2x_reg *h2x_reg = pcie->h2x_reg;

	switch (slot) {
	case 0:
		//rc_l
		writel(PCIE_RX_LINEAR | PCIE_RX_MSI_EN |
				PCIE_WAIT_RX_TLP_CLR |
				PCIE_RC_RX_ENABLE | PCIE_RC_ENABLE,
				&h2x_reg->h2x_rc_l_ctrl);
		//assign debug tx tag
		writel((u32)&h2x_reg->h2x_rc_l_ctrl, &h2x_reg->h2x_rc_l_tx_tag);
		break;
	case 1:
		//rc_h
		writel(PCIE_RX_LINEAR | PCIE_RX_MSI_EN |
				PCIE_WAIT_RX_TLP_CLR |
				PCIE_RC_RX_ENABLE | PCIE_RC_ENABLE,
				&h2x_reg->h2x_rc_h_ctrl);
		//assign debug tx tag
		writel((u32)&h2x_reg->h2x_rc_h_ctrl, &h2x_reg->h2x_rc_h_tx_tag);
		break;
	}
}

static int pcie_aspeed_probe(struct udevice *dev)
{
	void *fdt = (void *)gd->fdt_blob;
	struct reset_ctl reset_ctl, rc0_reset_ctl, rc1_reset_ctl;
	struct pcie_aspeed *pcie = (struct pcie_aspeed *)dev_get_priv(dev);
	struct aspeed_h2x_reg *h2x_reg = pcie->h2x_reg;
	struct udevice *ahbc_dev, *slot0_dev, *slot1_dev;
	int slot0_of_handle, slot1_of_handle;
	int ret = 0;

	txTag = 0;
	ret = reset_get_by_index(dev, 0, &reset_ctl);
	if (ret) {
		printf("%s(): Failed to get pcie reset signal\n", __func__);
		return ret;
	}

	reset_assert(&reset_ctl);
	mdelay(1);
	reset_deassert(&reset_ctl);

	//workaround : Send vender define message for avoid when PCIE RESET send unknown message out
	writel(0x34000000, &h2x_reg->h2x_tx_desc3);
	writel(0x0000007f, &h2x_reg->h2x_tx_desc2);
	writel(0x00001a03, &h2x_reg->h2x_tx_desc1);
	writel(0x00000000, &h2x_reg->h2x_tx_desc0);

	ret = uclass_get_device_by_driver
			(UCLASS_MISC, DM_GET_DRIVER(aspeed_ahbc), &ahbc_dev);
	if (ret) {
		debug("ahbc device not defined\n");
		return ret;
	}
	aspeed_ahbc_remap_enable(devfdt_get_addr_ptr(ahbc_dev));

	//ahb to pcie rc
	writel(0xe0006000, &h2x_reg->h2x_reg60);
	writel(0x0, &h2x_reg->h2x_reg64);
	writel(0xFFFFFFFF, &h2x_reg->h2x_reg68);

	//PCIe Host Enable
	writel(BIT(0), &h2x_reg->h2x_reg00);

	slot0_of_handle =
		fdtdec_lookup_phandle(fdt, dev_of_offset(dev), "slot0-handle");
	if (slot0_of_handle) {
		ret = reset_get_by_index(dev, 1, &rc0_reset_ctl);
		if (ret) {
			printf("%s(): Failed to get rc low reset signal\n", __func__);
			return ret;
		}
		aspeed_pcie_rc_slot_enable(pcie, 0);
		reset_deassert(&rc0_reset_ctl);
		mdelay(50);
		if (uclass_get_device_by_of_offset
				(UCLASS_MISC, slot0_of_handle, &slot0_dev))
			goto slot1;
		if (aspeed_pcie_phy_link_status(slot0_dev))
			aspeed_pcie_set_slot_power_limit(pcie, 0);
	}

slot1:
	slot1_of_handle =
		fdtdec_lookup_phandle(fdt, dev_of_offset(dev), "slot1-handle");
	if (slot1_of_handle) {
		ret = reset_get_by_index(dev, 2, &rc1_reset_ctl);
		if (ret) {
			printf("%s(): Failed to get rc high reset signal\n", __func__);
			return ret;
		}
		aspeed_pcie_rc_slot_enable(pcie, 1);
		reset_deassert(&rc1_reset_ctl);
		mdelay(50);
		if (uclass_get_device_by_of_offset
				(UCLASS_MISC, slot1_of_handle, &slot1_dev))
			goto end;
		if (aspeed_pcie_phy_link_status(slot1_dev))
			aspeed_pcie_set_slot_power_limit(pcie, 1);
	}
end:
	return 0;
}

static int pcie_aspeed_ofdata_to_platdata(struct udevice *dev)
{
	struct pcie_aspeed *pcie = dev_get_priv(dev);

	/* Get the controller base address */
	pcie->h2x_reg = (void *)devfdt_get_addr_index(dev, 0);

	return 0;
}

static const struct dm_pci_ops pcie_aspeed_ops = {
	.read_config	= pcie_aspeed_read_config,
	.write_config	= pcie_aspeed_write_config,
};

static const struct udevice_id pcie_aspeed_ids[] = {
	{ .compatible = "aspeed,ast2600-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_aspeed) = {
	.name			= "pcie_aspeed",
	.id				= UCLASS_PCI,
	.of_match		= pcie_aspeed_ids,
	.ops			= &pcie_aspeed_ops,
	.ofdata_to_platdata	= pcie_aspeed_ofdata_to_platdata,
	.probe			= pcie_aspeed_probe,
	.priv_auto_alloc_size	= sizeof(struct pcie_aspeed),
};
