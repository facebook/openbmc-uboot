// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <pci.h>

#include <asm/arch/h2x_ast2600.h>

/* reg 0x24 */
#define PCIE_TX_IDLE			BIT(31)

#define PCIE_STATUS_OF_TX		GENMASK(25, 24)
#define	PCIE_RC_TX_COMPLETE		0
#define	PCIE_RC_L_TX_COMPLETE	BIT(24)
#define	PCIE_RC_H_TX_COMPLETE	BIT(25)

#define PCIE_TRIGGER_TX			BIT(0)

/* reg 0x80, 0xC0 */
#define PCIE_RX_TAG_MASK		GENMASK(23, 16)
#define PCIE_RX_LINEAR			BIT(8)
#define PCIE_RX_MSI_SEL			BIT(7)
#define PCIE_RX_MSI_EN			BIT(6)
#define PCIE_1M_ADDRESS_EN		BIT(5)
#define PCIE_UNLOCK_RX_BUFF		BIT(4)
#define PCIE_RX_TLP_TAG_MATCH	BIT(3)
#define PCIE_Wait_RX_TLP_CLR	BIT(2)
#define PCIE_RC_RX_ENABLE		BIT(1)
#define PCIE_RC_ENABLE			BIT(0)

/* reg 0x88, 0xC8 : RC ISR */
#define PCIE_RC_CPLCA_ISR		BIT(6)
#define PCIE_RC_CPLUR_ISR		BIT(5)
#define PCIE_RC_RX_DONE_ISR		BIT(4)

#define PCIE_RC_INTD_ISR		BIT(3)
#define PCIE_RC_INTC_ISR		BIT(2)
#define PCIE_RC_INTB_ISR		BIT(1)
#define PCIE_RC_INTA_ISR		BIT(0)

struct aspeed_h2x_priv {
	struct aspeed_h2x_reg *h2x;
};

struct aspeed_h2x_priv *h2x_priv;

static u8 txTag = 0;

extern void aspeed_pcie_workaround(void *rc_offset) 
{
	u32 timeout = 0;
	u32 rc_base = (u32)h2x_priv->h2x + (u32) rc_offset;

	writel(BIT(4) | readl(rc_base), rc_base);
	writel(0x74000001, &h2x_priv->h2x->h2x_tx_desc3);
	writel(0x00400050, &h2x_priv->h2x->h2x_tx_desc2);
	writel(0x0, &h2x_priv->h2x->h2x_tx_desc1);
	writel(0x0, &h2x_priv->h2x->h2x_tx_desc0);

	writel(0x1a, &h2x_priv->h2x->h2x_tx_data);

	//trigger tx
	writel(PCIE_TRIGGER_TX, &h2x_priv->h2x->h2x_reg24);

	//wait tx idle
	while(!(readl(&h2x_priv->h2x->h2x_reg24) & BIT(31))) {
		timeout++;
		if(timeout > 1000) {
			return;
		}
	};

	//write clr tx idle
	writel(1, &h2x_priv->h2x->h2x_reg08);
	timeout = 0;

	//check tx status and clr rx done int
	while(!(readl(rc_base + 0x08) & PCIE_RC_RX_DONE_ISR)) {
		timeout++;
		if(timeout > 10) {
			break;
		}
		mdelay(1);
	}
	writel(PCIE_RC_RX_DONE_ISR, rc_base + 0x08);

}

extern void aspeed_pcie_cfg_read(void *rc_offset, pci_dev_t bdf, uint offset, ulong *valuep)
{
	u32 timeout = 0;
	u32 bdf_offset;
	u32 type = 0;
	int rx_done_fail = 0;
	u32 cfg_base = (u32)h2x_priv->h2x + (u32) rc_offset;
//	int i = 0;
	//H2X80[4] (unlock) is write-only.
	//Driver may set H2X80/H2XC0[4]=1 before triggering next TX config.
	writel(BIT(4) | readl(cfg_base), cfg_base);

	if(PCI_BUS(bdf) == 0)
		type = 0;
	else
		type = 1;

	bdf_offset = (PCI_BUS(bdf) << 24) |
					(PCI_DEV(bdf) << 19) |
					(PCI_FUNC(bdf) << 16) |
					(offset & ~3);

	txTag %= 0x7;

	writel(0x04000001 | (type << 24), &h2x_priv->h2x->h2x_tx_desc3);
	writel(0x0000200f | (txTag << 8), &h2x_priv->h2x->h2x_tx_desc2);
	writel(bdf_offset, &h2x_priv->h2x->h2x_tx_desc1);
	writel(0x00000000, &h2x_priv->h2x->h2x_tx_desc0);

	//trigger tx
	writel(PCIE_TRIGGER_TX, &h2x_priv->h2x->h2x_reg24);	

	//wait tx idle
	while(!(readl(&h2x_priv->h2x->h2x_reg24) & PCIE_TX_IDLE)) {
		timeout++;
		if(timeout > 1000) {
//			printf("time out b : %d, d : %d, f: %d \n", PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
			*valuep = 0xffffffff;
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x_priv->h2x->h2x_reg08);

	timeout = 0;
	//check tx status 
	switch(readl(&h2x_priv->h2x->h2x_reg24) & PCIE_STATUS_OF_TX) {
		case PCIE_RC_L_TX_COMPLETE:
			while(!(readl(&h2x_priv->h2x->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR)) {
				timeout++;
				if(timeout > 10) {
					rx_done_fail = 1;
					*valuep = 0xffffffff;
					break;
				}
				mdelay(1);
			}
			if(!rx_done_fail) {
				if(readl(&h2x_priv->h2x->h2x_reg94) & BIT(13)) {
					*valuep = 0xffffffff;
				} else
					*valuep = readl(&h2x_priv->h2x->h2x_rc_l_rdata);
			}
			writel(BIT(4) | readl(&h2x_priv->h2x->h2x_reg80), &h2x_priv->h2x->h2x_reg80);
			writel(readl(&h2x_priv->h2x->h2x_rc_l_isr), &h2x_priv->h2x->h2x_rc_l_isr);
			break;
		case PCIE_RC_H_TX_COMPLETE:
			while(!(readl(&h2x_priv->h2x->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR)) {
				timeout++;
				if(timeout > 10) {
					rx_done_fail = 1;
					*valuep = 0xffffffff;
					break;
				}
				mdelay(1);
			}
			if(!rx_done_fail) {
				if(readl(&h2x_priv->h2x->h2x_regD4) & BIT(13)) {
					*valuep = 0xffffffff;
				} else			
					*valuep = readl(&h2x_priv->h2x->h2x_rc_h_rdata);
			}
			writel(BIT(4) | readl(&h2x_priv->h2x->h2x_regC0), &h2x_priv->h2x->h2x_regC0);
			writel(readl(&h2x_priv->h2x->h2x_rc_h_isr), &h2x_priv->h2x->h2x_rc_h_isr);
			break;
		default:	//read rc data
			*valuep = readl(&h2x_priv->h2x->h2x_rdata);
			break;
	}

out:
	txTag++;
}

extern void aspeed_pcie_cfg_write(void *rc_offset, pci_dev_t bdf, uint offset, ulong value, enum pci_size_t size)
{
	u32 timeout = 0;
	u32 type = 0;
	u32 bdf_offset;
	u8 byte_en = 0;
	u32 cfg_addr = (u32)h2x_priv->h2x + (u32)rc_offset;
	
	writel(BIT(4) | readl(cfg_addr), cfg_addr);

	switch (size) {
	case PCI_SIZE_8:
		switch(offset % 4) {
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
		switch((offset >> 1) % 2 ) {
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

	if(PCI_BUS(bdf) == 0)
		type = 0;
	else
		type = 1;

	bdf_offset = (PCI_BUS(bdf) << 24) |
					(PCI_DEV(bdf) << 19) |
					(PCI_FUNC(bdf) << 16) |
					(offset & ~3);

	txTag %= 0x7;

	writel(0x44000001 | (type << 24), &h2x_priv->h2x->h2x_tx_desc3);
	writel(0x00002000 | (txTag << 8) | byte_en, &h2x_priv->h2x->h2x_tx_desc2);
	writel(bdf_offset, &h2x_priv->h2x->h2x_tx_desc1);
	writel(0x00000000, &h2x_priv->h2x->h2x_tx_desc0);

	value = pci_conv_size_to_32(0x0, value, offset, size);

	writel(value, &h2x_priv->h2x->h2x_tx_data);

	//trigger tx
	writel(1, &h2x_priv->h2x->h2x_reg24);	

	//wait tx idle
	while(!(readl(&h2x_priv->h2x->h2x_reg24) & BIT(31))) {
		timeout++;
		if(timeout > 1000) {
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x_priv->h2x->h2x_reg08);

	timeout = 0;
	//check tx status and clr rx done int
	switch(readl(&h2x_priv->h2x->h2x_reg24) & PCIE_STATUS_OF_TX) {
		case PCIE_RC_L_TX_COMPLETE:
			while(!(readl(&h2x_priv->h2x->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR)) {
				timeout++;
				if(timeout > 10) {
					break;
				}
				mdelay(1);
			}
			writel(PCIE_RC_RX_DONE_ISR, &h2x_priv->h2x->h2x_rc_l_isr);
			break;
		case PCIE_RC_H_TX_COMPLETE:
			while(!(readl(&h2x_priv->h2x->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR)) {
				timeout++;
				if(timeout > 10) {
					break;
				}
				mdelay(1);
			}
			writel(PCIE_RC_RX_DONE_ISR, &h2x_priv->h2x->h2x_rc_h_isr);
			break;
	}

out:
	txTag++;
}

extern void aspeed_h2x_rc_enable(void *rc_offset)
{
	u32 rc_addr = (u32)h2x_priv->h2x + (u32)rc_offset;

	//rc_l
	writel( PCIE_RX_LINEAR | PCIE_RX_MSI_EN |
			PCIE_Wait_RX_TLP_CLR | PCIE_RC_RX_ENABLE | PCIE_RC_ENABLE, rc_addr);
	//assign debug tx tag
	writel((u32)rc_offset, rc_addr + 0x3C);
}

static int aspeed_h2x_probe(struct udevice *dev)
{
	struct reset_ctl reset_ctl;
	h2x_priv = dev_get_priv(dev);
	int ret = 0;

	debug("%s(dev=%p) \n", __func__, dev);

	ret = reset_get_by_index(dev, 0, &reset_ctl);

	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	reset_assert(&reset_ctl);
	reset_deassert(&reset_ctl);

	h2x_priv->h2x = devfdt_get_addr_ptr(dev);
	if (IS_ERR(h2x_priv->h2x))
		return PTR_ERR(h2x_priv->h2x);

	//init
	writel(0x1, &h2x_priv->h2x->h2x_reg00);

	//ahb to pcie rc 
	writel(0xe0006000, &h2x_priv->h2x->h2x_reg60);
	writel(0x0, &h2x_priv->h2x->h2x_reg64);
	writel(0xFFFFFFFF, &h2x_priv->h2x->h2x_reg68);

	return 0;
}

static const struct udevice_id aspeed_h2x_ids[] = {
	{ .compatible = "aspeed,ast2600-h2x" },
	{ }
};

U_BOOT_DRIVER(aspeed_h2x) = {
	.name		= "aspeed_h2x",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_h2x_ids,
	.probe		= aspeed_h2x_probe,
	.priv_auto_alloc_size = sizeof(struct aspeed_h2x_priv),
};
