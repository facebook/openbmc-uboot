// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#ifndef _ASM_ARCH_SCU_AST2600_H
#define _ASM_ARCH_SCU_AST2600_H

#define AST2600_CLK_IN	25000000

struct ast2600_clk_priv {
	struct ast2600_scu *scu;
	int dly32_lookup[2][32];
};

struct hw_strap {
	u32 hwstrap;			/* 0x500 */
	u32 hwstrap_clr;		/* 0x504 */
	u32 hwstrap_protect;		/* 0x508 */
};

struct ast2600_scu {
	u32 protection_key;		/* 0x000 */
	u32 chip_id0;			/* 0x004 */
	u32 reserve_0x08;		/* 0x008 */
	u32 reserve_0x0C;		/* 0x00C */
	u32 reserve_0x10;		/* 0x010 */
	u32 chip_id1;			/* 0x014 */
	u32 reserve_0x18;		/* 0x018 */
	u32 reserve_0x1C;		/* 0x01C */
	u32 reserve_0x20;		/* 0x020 */
	u32 reserve_0x24;		/* 0x024 */
	u32 reserve_0x28;		/* 0x028 */
	u32 reserve_0x2c;		/* 0x02C */
	u32 reserve_0x30;		/* 0x030 */
	u32 reserve_0x34;		/* 0x034 */
	u32 reserve_0x38;		/* 0x038 */
	u32 reserve_0x3C;		/* 0x03C */
	u32 sysreset_ctrl1;		/* 0x040 */
	u32 sysreset_clr_ctrl1;		/* 0x044 */
	u32 reserve_0x48;		/* 0x048 */
	u32 reserve_0x4C;		/* 0x04C */
	u32 sysreset_ctrl2;		/* 0x050 */
	u32 sysreset_clr_ctrl2;		/* 0x054 */
	u32 reserve_0x58;		/* 0x058 */
	u32 reserve_0x5C;		/* 0x05C */
	u32 extrst_sel1;		/* 0x060 */
	u32 sysrst_evet_log1_1;		/* 0x064 */
	u32 sysrst_evet_log1_2;		/* 0x068 */
	u32 reserve_0x6C;		/* 0x06C */
	u32 extrst_sel2;		/* 0x070 */
	u32 sysrst_evet_log2_1;		/* 0x074 */
	u32 sysrst_evet_log2_2;		/* 0x078 */
	u32 reserve_0x7C;		/* 0x07C */
	u32 clk_stop_ctrl1;		/* 0x080 */
	u32 clk_stop_clr_ctrl1;		/* 0x084 */
	u32 reserve_0x88;		/* 0x088 */
	u32 reserve_0x8C;		/* 0x08C */
	u32 clk_stop_ctrl2;		/* 0x090 */
	u32 clk_stop_clr_ctrl2;		/* 0x094 */
	u32 reserve_0x98;		/* 0x098 */
	u32 reserve_0x9C;		/* 0x09C */
	u32 reserve_0xA0;		/* 0x0A0 */
	u32 reserve_0xA4;		/* 0x0A4 */
	u32 reserve_0xA8;		/* 0x0A8 */
	u32 reserve_0xAC;		/* 0x0AC */
	u32 reserve_0xB0;		/* 0x0B0 */
	u32 reserve_0xB4;		/* 0x0B4 */
	u32 reserve_0xB8;		/* 0x0B8 */
	u32 reserve_0xBC;		/* 0x0BC */
	u32 misc_ctrl1;			/* 0x0C0 */
	u32 misc_ctrl2;			/* 0x0C4 */
	u32 backdoor_ctrl;		/* 0x0C8 */
	u32 reserve_0xCC;		/* 0x0CC */
	u32 misc_ctrl3;			/* 0x0D0 */
	u32 misc_ctrl4;			/* 0x0D4 */
	u32 reserve_0xD8;		/* 0x0D8 */
	u32 reserve_0xDC;		/* 0x0DC */
	u32 reserve_0xE0;		/* 0x0E0 */
	u32 reserve_0xE4;		/* 0x0E4 */
	u32 reserve_0xE8;		/* 0x0E8 */
	u32 reserve_0xEC;		/* 0x0EC */
	u32 reserve_0xF0;		/* 0x0F0 */
	u32 reserve_0xF4;		/* 0x0F4 */
	u32 reserve_0xF8;		/* 0x0F8 */
	u32 reserve_0xFC;		/* 0x0FC */
	u32 soc_scratch[4];		/* 0x100 */
	u32 reserve_0x110;		/* 0x110 */
	u32 reserve_0x114;		/* 0x114 */
	u32 reserve_0x118;		/* 0x118 */
	u32 reserve_0x11C;		/* 0x11C */
	u32 cpu_scratch_wp;		/* 0x120 */
	u32 reserve_0x124[23];		/* 0x124 */
	u32 cpu_scratch[32];		/* 0x180 */
	u32 h_pll_param;		/* 0x200 */
	u32 h_pll_ext_param;		/* 0x204 */
	u32 reserve_0x208;		/* 0x208 */
	u32 reserve_0x20C;		/* 0x20C */
	u32 a_pll_param;		/* 0x210 */
	u32 a_pll_ext_param;		/* 0x214 */
	u32 reserve_0x218;		/* 0x218 */
	u32 reserve_0x21C;		/* 0x21C */
	u32 m_pll_param;		/* 0x220 */
	u32 m_pll_ext_param;		/* 0x224 */
	u32 reserve_0x228;		/* 0x228 */
	u32 reserve_0x22C;		/* 0x22C */
	u32 reserve_0x230[4];		/* 0x230 */
	u32 e_pll_param;		/* 0x240 */
	u32 e_pll_ext_param;		/* 0x244 */
	u32 reserve_0x248;		/* 0x248 */
	u32 reserve_0x24C;		/* 0x24C */
	u32 reserve_0x250[4];		/* 0x250 */
	u32 d_pll_param;		/* 0x260 */
	u32 d_pll_ext_param;		/* 0x264 */
	u32 reserve_0x268;		/* 0x268 */
	u32 reserve_0x26C;		/* 0x26C */
	u32 reserve_0x270[36];		/* 0x270 */
	u32 clk_sel1;			/* 0x300 */
	u32 clk_sel2;			/* 0x304 */
	u32 clk_sel3;			/* 0x308 */
	u32 reserve_0x30C;		/* 0x30C */
	u32 clk_sel4;			/* 0x310 */
	u32 clk_sel5;			/* 0x314 */
	u32 reserve_0x318;		/* 0x318 */
	u32 reserve_0x31C;		/* 0x31C */
	u32 freq_counter_ctrl1;		/* 0x320 */
	u32 freq_counter_cmp1;		/* 0x324 */
	u32 reserve_0x328;		/* 0x328 */
	u32 uart_24m_ref_hpll;		/* 0x32C */
	u32 freq_counter_ctrl2;		/* 0x330 */
	u32 freq_counter_cmp2;		/* 0x334 */
	u32 uart_24m_ref_uxclk;		/* 0x338 */
	u32 uart_24m_ref_huxclk;	/* 0x33C */
	u32 mac12_clk_delay;		/* 0x340 */
	u32 reserve_0x344;		/* 0x344 */
	u32 mac12_clk_delay_100M;	/* 0x348 */
	u32 mac12_clk_delay_10M;	/* 0x34c */
	u32 mac34_clk_delay;		/* 0x350 */
	u32 reserve_0x354;		/* 0x354 */
	u32 mac34_clk_delay_100M;	/* 0x358 */
	u32 mac34_clk_delay_10M;	/* 0x35c */
	u32 clk_duty_meas_ctrl;		/* 0x360 */
	u32 clk_duty_sel0;		/* 0x364 */
	u32 clk_duty_sel1;		/* 0x368 */
	u32 clk_duty_meas_res;		/* 0x36C */
	u32 clk_duty_meas_ctr2;		/* 0x370 */
	u32 clk_duty_sel2;		/* 0x374 */
	u32 reserve_0x378[34];		/* 0x378 ~ 0x3fc */
	u32 pinmux_ctrl1;		/* 0x400 */
	u32 pinmux_ctrl2;		/* 0x404 */
	u32 reserve_0x408;		/* 0x408 */
	u32 pinmux_ctrl3;		/* 0x40C */
	u32 pinmux_ctrl4;		/* 0x410 */
	u32 pinmux_ctrl5;		/* 0x414 */
	u32 pinmux_ctrl6;		/* 0x418 */
	u32 pinmux_ctrl7;		/* 0x41C */
	u32 reserve_0x420[4];		/* 0x420 ~ 0x42c */
	u32 pinmux_ctrl8;		/* 0x430 */
	u32 pinmux_ctrl9;		/* 0x434 */
	u32 pinmux_ctrl10;		/* 0x438 */
	u32 reserve_0x43c;		/* 0x43c */
	u32 pinmux_ctrl12;		/* 0x440 */
	u32 pinmux_ctrl13;		/* 0x444 */
	u32 reserv_0x448[2];		/* 0x448 ~ 0x44c */
	u32 pinmux_ctrl14;		/* 0x450 */
	u32 pinmux_ctrl15;		/* 0x454 */
	u32 pinmux_ctrl16;		/* 0x458 */
	u32 reserv_0x45c[21];		/* 0x45c ~ 0x4ac */
	u32 pinmux_ctrl17;		/* 0x4b0 */
	u32 pinmux_ctrl18;		/* 0x4b4 */
	u32 pinmux_ctrl19;		/* 0x4b8 */
	u32 pinmux_ctrl20;		/* 0x4bc */
	u32 reserv_0x4c0[5];		/* 0x4c0 ~ 0x4d0 */
	u32 pinmux_ctrl22;		/* 0x4d4 */
	u32 pinmux_ctrl23;		/* 0x4d8 */
	u32 reserv_0x4dc[9];		/* 0x4dc ~ 0x4fc */
	struct hw_strap hwstrap1;	/* 0x500 */
	u32 reserve_0x50C;		/* 0x50C */
	struct hw_strap hwstrap2;	/* 0x510 */
	u32 reserve_0x51C;		/* 0x51C */
	u32 rng_ctrl;			/* 0x520 */
	u32 rng_data;			/* 0x524 */
	u32 reserve_0x528[6];		/* 0x528 */
	u32 pwr_save_wakeup_en1;	/* 0x540 */
	u32 pwr_save_wakeup_ctrl1;	/* 0x544 */
	u32 reserve_0x548[2];		/* 0x548 */
	u32 pwr_save_wakeup_en2;	/* 0x550 */
	u32 pwr_save_wakeup_ctrl2;	/* 0x554 */
	u32 reserve_0x558[2];		/* 0x558 */
	u32 intr1_ctrl_sts;		/* 0x560 */
	u32 reserve_0x564[3];		/* 0x564 */
	u32 intr2_ctrl_sts;		/* 0x570 */
	u32 reserve_0x574[3];		/* 0x574 */
	u32 reserve_0x580[4];		/* 0x580 */
	u32 opt_ctrl;			/* 0x590 */
	u32 hw_config;			/* 0x594 */
	u32 reserve_0x598[6];		/* 0x598 */
	u32 chip_unique_id[8];		/* 0x5B0 */
	u32 reserve_0x5E0[8];		/* 0x5E0 */
	u32 disgpio_in_pull_down0;	/* 0x610 */
	u32 disgpio_in_pull_down1;	/* 0x614 */
	u32 disgpio_in_pull_down2;	/* 0x618 */
	u32 disgpio_in_pull_down3;	/* 0x61C */
	u32 reserve_0x620[4];		/* 0x620 */
	u32 disgpio_in_pull_down4;	/* 0x630 */
	u32 disgpio_in_pull_down5;	/* 0x634 */
	u32 disgpio_in_pull_down6;	/* 0x638 */
	u32 reserve_0x63C[5];		/* 0x63Cs */
	u32 sli_driving_strength;	/* 0x650 */
	u32 reserve_0x654[235];		/* 0x654 */
	u32 cm3_ctrl;			/* 0xA00 */
	u32 cm3_base;			/* 0xA04 */
	u32 cm3_instr_mem_addr;		/* 0xA08 */
	u32 cm3_data_mem_addr;		/* 0xA0C */
	u32 reserve_0xA10[12];		/* 0xA10 */
	u32 cm3_cache_area;		/* 0xA40 */
	u32 cm3_cache_invalid_ctrl;	/* 0xA44 */
	u32 cm3_cache_fun_ctrl;		/* 0xA48 */
	u32 reserve_0xA4C[108];		/* 0xA4C */
	u32 pci_config[3];		/* 0xC00 */
	u32 reserve_0xC0C[5];		/* 0xC0C */
	u32 pcie_config;		/* 0xC20 */
	u32 mmio_decode;		/* 0xC24 */
	u32 reloc_ctrl_decode[2];	/* 0xC28 */
	u32 reserve_0xC30[4];		/* 0xC30 */
	u32 mailbox_addr;		/* 0xC40 */
	u32 shared_sram_decode[2];	/* 0xC44 */
	u32 bmc_rev_id;			/* 0xC4C */
	u32 reserve_0xC50[5];		/* 0xC50 */
	u32 bmc_device_id;		/* 0xC64 */
	u32 reserve_0xC68[102];		/* 0xC68 */
	u32 vga_scratch[8];		/* 0xE00 */
};

#endif  /* _ASM_ARCH_SCU_AST2600_H */
