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

//#define PCIE_RC_L

struct aspeed_h2x_priv {
	struct aspeed_h2x_reg *h2x;
};

static u8 txTag = 0;

extern void aspeed_pcie_cfg_read(struct aspeed_h2x_reg *h2x, pci_dev_t bdf, uint offset, ulong *valuep)
{
	u32 timeout = 0;
	u32 bdf_offset;
	u32 type = 0;
//	int i = 0;

	//H2X80[4] (unlock) is write-only.
	//Driver may set H2X80/H2XC0[4]=1 before triggering next TX config.
#ifdef PCIE_RC_L
	writel(BIT(4) | readl(&h2x->h2x_reg80), &h2x->h2x_reg80);
#else
	writel(BIT(4) | readl(&h2x->h2x_regC0), &h2x->h2x_regC0);
#endif

	if(PCI_BUS(bdf) == 0)
		type = 0;
	else
		type = 1;

	bdf_offset = (PCI_BUS(bdf) << 24) |
					(PCI_DEV(bdf) << 19) |
					(PCI_FUNC(bdf) << 16) |
					(offset & ~3);

	txTag %= 0x7;

	writel(0x04000001 | (type << 24), &h2x->h2x_tx_desc3);
	writel(0x0000200f | (txTag << 8), &h2x->h2x_tx_desc2);
	writel(bdf_offset, &h2x->h2x_tx_desc1);
	writel(0x00000000, &h2x->h2x_tx_desc0);

	//trigger tx
	writel(PCIE_TRIGGER_TX, &h2x->h2x_reg24);	

	//wait tx idle
	while(!(readl(&h2x->h2x_reg24) & PCIE_TX_IDLE)) {
		timeout++;
		if(timeout > 10000) {
			printf("time out b : %d, d : %d, f: %d \n", PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
			*valuep = 0xffffffff;

#if 0
			printf("=======0x1e770000========\n");
			for (i = 0; i < 0x40; i++) {
				if((i % 4) == 0)
					printf("\n %2x : ", (i * 4));

				printf("%08x ", readl(0x1e770000 + (i * 4)));
			}
			printf("\n =======0x1e6ed000========\n");
			for (i = 0; i < 0x40; i++) {
				if((i % 4) == 0)
					printf("\n %2x : ", (i * 4));
				printf(" %08x ", readl(0x1e6ed000 + (i * 4)));
			}
			printf("===============\n");			
#endif
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x->h2x_reg08);

	//check tx status 
	switch(readl(&h2x->h2x_reg24) & PCIE_STATUS_OF_TX) {
		case PCIE_RC_L_TX_COMPLETE:
			while(!(readl(&h2x->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR));
#if 0
			if(readl(&h2x->h2x_rc_l_isr) & (PCIE_RC_CPLCA_ISR | PCIE_RC_CPLUR_ISR)) {
				printf("return ffffffff \n");
				*valuep = 0xffffffff;
			} else
				*valuep = readl(&h2x->h2x_rc_l_rdata);
#else
			*valuep = readl(&h2x->h2x_rc_l_rdata);
#endif
			writel(readl(&h2x->h2x_rc_l_isr), &h2x->h2x_rc_l_isr);
			break;
		case PCIE_RC_H_TX_COMPLETE:
			while(!(readl(&h2x->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR));
#if 0
			if(readl(&h2x->h2x_rc_h_isr) & (PCIE_RC_CPLCA_ISR | PCIE_RC_CPLUR_ISR))
				*valuep = 0xffffffff;
			else
				*valuep = readl(&h2x->h2x_rc_h_rdata);
#else
			*valuep = readl(&h2x->h2x_rc_h_rdata);
#endif
			writel(readl(&h2x->h2x_rc_h_isr), &h2x->h2x_rc_h_isr);
			break;
		default:	//read rc data
			*valuep = readl(&h2x->h2x_rdata);
			break;
	}

out:
	txTag++;

}

extern void aspeed_pcie_cfg_write(struct aspeed_h2x_reg *h2x, pci_dev_t bdf, uint offset, ulong value, enum pci_size_t size)
{
	u32 timeout = 0;
	u32 type = 0;
	u32 bdf_offset;
	u8 byte_en = 0;

#ifdef PCIE_RC_L
	writel(BIT(4) | readl(&h2x->h2x_reg80), &h2x->h2x_reg80);
#else
	writel(BIT(4) | readl(&h2x->h2x_regC0), &h2x->h2x_regC0);
#endif

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

	writel(0x44000001 | (type << 24), &h2x->h2x_tx_desc3);
	writel(0x00002000 | (txTag << 8) | byte_en, &h2x->h2x_tx_desc2);
	writel(bdf_offset, &h2x->h2x_tx_desc1);
	writel(0x00000000, &h2x->h2x_tx_desc0);

	value = pci_conv_size_to_32(0x0, value, offset, size);

	writel(value, &h2x->h2x_tx_data);

	//trigger tx
	writel(1, &h2x->h2x_reg24);	

	//wait tx idle
	while(!(readl(&h2x->h2x_reg24) & BIT(31))) {
		timeout++;
		if(timeout > 10000) {
			printf("time out \n");
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x->h2x_reg08);

	//check tx status and clr rx done int
	switch(readl(&h2x->h2x_reg24) & PCIE_STATUS_OF_TX) {
		case PCIE_RC_L_TX_COMPLETE:
			while(!(readl(&h2x->h2x_rc_l_isr) & PCIE_RC_RX_DONE_ISR));
			writel(PCIE_RC_RX_DONE_ISR, &h2x->h2x_rc_l_isr);
			break;
		case PCIE_RC_H_TX_COMPLETE:
			while(!(readl(&h2x->h2x_rc_h_isr) & PCIE_RC_RX_DONE_ISR));
			writel(PCIE_RC_RX_DONE_ISR, &h2x->h2x_rc_h_isr);
			break;
	}

out:
	txTag++;

}

static int aspeed_h2x_probe(struct udevice *dev)
{
	struct reset_ctl reset_ctl;
	struct aspeed_h2x_priv *priv = dev_get_priv(dev);
	int ret = 0;

	debug("%s(dev=%p) \n", __func__, dev);

	ret = reset_get_by_index(dev, 0, &reset_ctl);

	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	reset_assert(&reset_ctl);
	reset_deassert(&reset_ctl);

	priv->h2x = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->h2x))
		return PTR_ERR(priv->h2x);

	//init
	writel(0x1, &priv->h2x->h2x_reg00);

	//ahb to pcie rc 
	writel(0xe0006000, &priv->h2x->h2x_reg60);
	writel(0x0, &priv->h2x->h2x_reg64);
	writel(0xFFFFFFFF, &priv->h2x->h2x_reg68);

#ifdef PCIE_RC_L
	//rc_l
	writel( PCIE_RX_LINEAR | PCIE_RX_MSI_EN |
			PCIE_Wait_RX_TLP_CLR | PCIE_RC_RX_ENABLE | PCIE_RC_ENABLE,
	&priv->h2x->h2x_reg80);
	//assign debug tx tag
	writel(0x28, &priv->h2x->h2x_regBC);
#else
	//rc_h
	writel( PCIE_RX_LINEAR | PCIE_RX_MSI_SEL | PCIE_RX_MSI_EN |
			PCIE_Wait_RX_TLP_CLR | PCIE_RC_RX_ENABLE | PCIE_RC_ENABLE,
	&priv->h2x->h2x_regC0);

	//assign debug tx tag
	writel(0x28, &priv->h2x->h2x_regFC);
#endif

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
