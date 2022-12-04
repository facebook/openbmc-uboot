/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __PCIE_ASPEED_H_INCLUDED
#define __PCIE_ASPEED_H_INCLUDED
#include <linux/bitops.h>
#include <dm/device.h>

/* reg 0x24 */
#define PCIE_TX_IDLE			BIT(31)

#define PCIE_STATUS_OF_TX		GENMASK(25, 24)
#define	PCIE_RC_TX_COMPLETE		0
#define	PCIE_RC_L_TX_COMPLETE	BIT(24)
#define	PCIE_RC_H_TX_COMPLETE	BIT(25)

#define PCIE_TRIGGER_TX			BIT(0)

#define PCIE_RC_L				0x80
#define PCIE_RC_H				0xC0
/* reg 0x80, 0xC0 */
#define PCIE_RX_TAG_MASK		GENMASK(23, 16)
#define PCIE_RX_LINEAR			BIT(8)
#define PCIE_RX_MSI_SEL			BIT(7)
#define PCIE_RX_MSI_EN			BIT(6)
#define PCIE_1M_ADDRESS_EN		BIT(5)
#define PCIE_UNLOCK_RX_BUFF		BIT(4)
#define PCIE_RX_TLP_TAG_MATCH	BIT(3)
#define PCIE_WAIT_RX_TLP_CLR	BIT(2)
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

struct aspeed_h2x_reg {
	u32 h2x_reg00;
	u32 h2x_reg04;
	u32 h2x_reg08;
	u32 h2x_rdata;			//0x0c
	u32 h2x_tx_desc3;		//0x10
	u32 h2x_tx_desc2;		//0x14
	u32 h2x_tx_desc1;		//0x18
	u32 h2x_tx_desc0;		//0x1c
	u32 h2x_tx_data;		//0x20
	u32 h2x_reg24;
	u32 h2x_reg28;
	u32 h2x_reg2C;
	u32 h2x_reg30;
	u32 h2x_reg34;
	u32 h2x_reg38;
	u32 h2x_reg3C;
	u32 h2x_reg40;
	u32 h2x_reg44;
	u32 h2x_reg48;
	u32 h2x_reg4C;
	u32 h2x_reg50;
	u32 h2x_reg54;
	u32 h2x_reg58;
	u32 h2x_reg5C;
	u32 h2x_reg60;
	u32 h2x_reg64;
	u32 h2x_reg68;
	u32 h2x_reg6C;
	u32 h2x_reg70;
	u32 h2x_reg74;
	u32 h2x_reg78;
	u32 h2x_reg7C;
	u32 h2x_rc_l_ctrl;		//0x80
	u32 h2x_rc_l_ier;		//0x84
	u32 h2x_rc_l_isr;		//0x88
	u32 h2x_rc_l_rdata;		//0x8C
	u32 h2x_rc_l_rxdesc3;	//0x90
	u32 h2x_rc_l_rxdesc2;	//0x94
	u32 h2x_rc_l_rxdesc1;	//0x98
	u32 h2x_rc_l_rxdesc0;	//0x9C
	u32 h2x_rc_l_msi1_ier;	//0xA0
	u32 h2x_rc_l_msi0_ier;	//0xA4
	u32 h2x_rc_l_msi1_isr;	//0xA8
	u32 h2x_rc_l_msi0_isr;	//0xAC
	u32 h2x_regb0;
	u32 h2x_regb4;
	u32 h2x_regb8;
	u32 h2x_rc_l_tx_tag;	//0xBC
	u32 h2x_rc_h_ctrl;		//0xC0
	u32 h2x_rc_h_ier;		//0xC4
	u32 h2x_rc_h_isr;		//0xC8
	u32 h2x_rc_h_rdata;		//0xCC
	u32 h2x_rc_h_rxdesc3;	//0xD0
	u32 h2x_rc_h_rxdesc2;	//0xD4
	u32 h2x_rc_h_rxdesc1;	//0xD8
	u32 h2x_rc_h_rxdesc0;	//0xDC
	u32 h2x_rc_h_msi1_ier;	//0xE0
	u32 h2x_rc_h_msi0_ier;	//0xE4
	u32 h2x_rc_h_msi1_isr;	//0xE8
	u32 h2x_rc_h_msi0_isr;	//0xEC
	u32 h2x_regf0;
	u32 h2x_regf4;
	u32 h2x_regf8;
	u32 h2x_rc_h_tx_tag;	//0xFC
};

struct aspeed_rc_bridge {
	void *reg;
};

int aspeed_pcie_phy_link_status(struct udevice *dev);

#endif
