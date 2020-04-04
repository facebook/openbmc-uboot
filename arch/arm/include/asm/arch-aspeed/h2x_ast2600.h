#ifndef __H2X_ASPEED_H_INCLUDED
#define __H2X_ASPEED_H_INCLUDED
#include <pci.h>

struct aspeed_h2x_reg {
	u32 h2x_reg00;
	u32 h2x_reg04;
	u32 h2x_reg08;
	u32 h2x_rdata;		//0x0c
	u32 h2x_tx_desc3;	//0x10
	u32 h2x_tx_desc2;	//0x14
	u32 h2x_tx_desc1;	//0x18
	u32 h2x_tx_desc0;	//0x1c
	u32 h2x_tx_data;	//0x20
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
	u32 h2x_reg80;
	u32 h2x_reg84;
	u32 h2x_rc_l_isr;
	u32 h2x_rc_l_rdata;
	u32 h2x_reg90;
	u32 h2x_reg94;
	u32 h2x_reg98;
	u32 h2x_reg9C;
	u32 h2x_regA0;
	u32 h2x_regA4;
	u32 h2x_regA8;
	u32 h2x_regAC;
	u32 h2x_regB0;
	u32 h2x_regB4;
	u32 h2x_regB8;
	u32 h2x_regBC;
	u32 h2x_regC0;
	u32 h2x_regC4;
	u32 h2x_rc_h_isr;
	u32 h2x_rc_h_rdata;
	u32 h2x_regD0;
	u32 h2x_regD4;
	u32 h2x_regD8;
	u32 h2x_regDC;
	u32 h2x_regE0;
	u32 h2x_regE4;
	u32 h2x_regE8;
	u32 h2x_regEC;
	u32 h2x_regF0;
	u32 h2x_regF4;
	u32 h2x_regF8;
	u32 h2x_regFC;
};

extern void aspeed_pcie_cfg_read(void *rc_offset, pci_dev_t bdf, uint offset, ulong *valuep);
extern void aspeed_pcie_cfg_write(void *rc_offset, pci_dev_t bdf, uint offset, ulong value, enum pci_size_t size);
extern void aspeed_h2x_rc_enable(void *rc_offset);
extern void aspeed_pcie_workaround(void *rc_offset);

#endif
