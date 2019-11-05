#ifndef __AHBC_ASPEED_H_INCLUDED
#define __AHBC_ASPEED_H_INCLUDED

struct aspeed_ahbc_reg {
	u32 protection_key;	//0x00
	u32 reserved0[15];
	u32 cmd_recod;		//0x40
	u32 log_buff;		//0x44
	u32 polling_addr;	//0x48
	u32 hw_fifo_sts;	//0x4C
	u32 reserved1[3];
	u32 hw_fifo_merge;	//0x5C
	u32 hw_fifo_stage0;	//0x60
	u32 hw_fifo_stage1;	//0x64
	u32 hw_fifo_stage2;	//0x68
	u32 hw_fifo_stage3;	//0x6C
	u32 hw_fifo_stage4;	//0x70
	u32 hw_fifo_stage5;	//0x74
	u32 hw_fifo_stage6;	//0x78
	u32 hw_fifo_stage7;	//0x7C
	u32 priority_ctrl;	//0x80
	u32 ier;			//0x84
	u32 target_disable_ctrl;	//0x88
	u32 addr_remap;		//0x8C
	u32 wdt_count_sts;	//0x90
	u32 wdt_count_reload;	//0x94
};

extern void aspeed_ahbc_remap_enable(struct aspeed_ahbc_reg *ahbc);

#endif
