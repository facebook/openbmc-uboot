// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <dm.h>
#include <reset.h>
#include <fdtdec.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/h2x_ast2600.h>
#include <asm/arch/ahbc_aspeed.h>

DECLARE_GLOBAL_DATA_PTR;

/* PCI Host Controller registers */

#define ASPEED_PCIE_CLASS_CODE		0x04
#define ASPEED_PCIE_GLOBAL			0x30
#define ASPEED_PCIE_CFG_DIN			0x50
#define ASPEED_PCIE_CFG3			0x58
#define ASPEED_PCIE_LOCK			0x7C
#define ASPEED_PCIE_LINK			0xC0
#define ASPEED_PCIE_INT				0xC4

/* 	AST_PCIE_CFG2			0x04		*/
#define PCIE_CFG_CLASS_CODE(x)	(x << 8)
#define PCIE_CFG_REV_ID(x)		(x)

/* 	AST_PCIE_GLOBAL			0x30 	*/
#define ROOT_COMPLEX_ID(x)		(x << 4)

/* 	AST_PCIE_LOCK			0x7C	*/
#define PCIE_UNLOCK				0xa8

/*	AST_PCIE_LINK			0xC0	*/
#define PCIE_LINK_STS			BIT(5)

struct pcie_aspeed {
	void *ctrl_base;
	void *h2x_pt;
	void *cfg_base;
	fdt_size_t cfg_size;
	int link_sts;
	int first_busno;
};

static int pcie_aspeed_read_config(struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong *valuep,
				     enum pci_size_t size)
{
	struct pcie_aspeed *pcie = dev_get_priv(bus);

	debug("PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) \n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	/* Only allow one other device besides the local one on the local bus */
	if (PCI_BUS(bdf) == 1 && PCI_DEV(bdf) != 0) {
			debug("- out of range\n");
			/*
			 * If local dev is 0, the first other dev can
			 * only be 1
			 */
			*valuep = pci_get_ff(size);
			return 0;
	}

	if (PCI_BUS(bdf) == 1 && (!pcie->link_sts)) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	aspeed_pcie_cfg_read(pcie->h2x_pt, bdf, offset, valuep);

	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, *valuep);
	*valuep = pci_conv_32_to_size(*valuep, offset, size);

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

	aspeed_pcie_cfg_write(pcie->h2x_pt, bdf, offset, value, size);

	return 0;
}

static int pcie_aspeed_probe(struct udevice *dev)
{
	void *fdt = (void *)gd->fdt_blob;

	struct reset_ctl reset_ctl0, reset_ctl1;
	struct pcie_aspeed *pcie = dev_get_priv(dev);
//	struct udevice *ctrl = pci_get_controller(dev);
//	struct pci_controller *host = dev_get_uclass_priv(ctrl);
	struct udevice *ahbc_dev;
	int h2x_of_handle;	
	int ret = 0;

	ret = reset_get_by_index(dev, 0, &reset_ctl0);
	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &reset_ctl1);
	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	reset_assert(&reset_ctl0);
	reset_assert(&reset_ctl1);
	mdelay(100);
	reset_deassert(&reset_ctl0);

	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_GET_DRIVER(aspeed_ahbc),
										  &ahbc_dev);
	if (ret) {
		debug("ahbc device not defined\n");
		return ret;
	}

	h2x_of_handle = fdtdec_lookup_phandle(fdt, dev_of_offset(dev), "cfg-handle");
	if (h2x_of_handle > 0) {
		pcie->h2x_pt = (void *)fdtdec_get_addr(fdt, h2x_of_handle, "reg");
		debug("h2x cfg addr %x \n", (u32)pcie->h2x_pt);
	} else {
		debug("h2x device not defined\n");
		return h2x_of_handle;
	}

	aspeed_ahbc_remap_enable(devfdt_get_addr_ptr(ahbc_dev));

	//plda enable 
	writel(PCIE_UNLOCK, pcie->ctrl_base + ASPEED_PCIE_LOCK);
	writel(ROOT_COMPLEX_ID(0x3), pcie->ctrl_base + ASPEED_PCIE_GLOBAL);

	pcie->first_busno = dev->seq;
	mdelay(100);

	/* Don't register host if link is down */
	if (readl(pcie->ctrl_base + ASPEED_PCIE_LINK) & PCIE_LINK_STS) {
		printf("PCIE-%d: Link up\n", dev->seq);
		pcie->link_sts = 1;
	} else {
		printf("PCIE-%d: Link down\n", dev->seq);
		pcie->link_sts = 0;
	}
	aspeed_h2x_rc_enable(pcie->h2x_pt);
	if(pcie->link_sts)
		aspeed_pcie_workaround(pcie->h2x_pt);

	return 0;
}

static int pcie_aspeed_ofdata_to_platdata(struct udevice *dev)
{
	struct pcie_aspeed *pcie = dev_get_priv(dev);

	/* Get the controller base address */
	pcie->ctrl_base = (void *)devfdt_get_addr_index(dev, 0);

	/* Get the config space base address and size */
	pcie->cfg_base = (void *)devfdt_get_addr_size_index(dev, 1,
							 &pcie->cfg_size);

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
