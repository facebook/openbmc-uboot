/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _ASM_ARCH_SCU_AST2400_H
#define _ASM_ARCH_SCU_AST2400_H

#define SCU_UNLOCK_VALUE		0x1688a8a8

#define SCU_HWSTRAP_VGAMEM_SHIFT	2
#define SCU_HWSTRAP_VGAMEM_MASK		(3 << SCU_HWSTRAP_VGAMEM_SHIFT)
#define SCU_HWSTRAP_MAC1_RGMII		(1 << 6)
#define SCU_HWSTRAP_MAC2_RGMII		(1 << 7)
#define SCU_HWSTRAP_DDR4		(1 << 24)
#define SCU_HWSTRAP_CLKIN_25MHZ		(1 << 23)

#define SCU_MPLL_DENUM_SHIFT		0
#define SCU_MPLL_DENUM_MASK		0x1f
#define SCU_MPLL_NUM_SHIFT		5
#define SCU_MPLL_NUM_MASK		(0xff << SCU_MPLL_NUM_SHIFT)
#define SCU_MPLL_POST_SHIFT		13
#define SCU_MPLL_POST_MASK		(0x3f << SCU_MPLL_POST_SHIFT)
#define SCU_PCLK_DIV_SHIFT		23
#define SCU_PCLK_DIV_MASK		(7 << SCU_PCLK_DIV_SHIFT)
#define SCU_HPLL_DENUM_SHIFT		0
#define SCU_HPLL_DENUM_MASK		0x1f
#define SCU_HPLL_NUM_SHIFT		5
#define SCU_HPLL_NUM_MASK		(0xff << SCU_HPLL_NUM_SHIFT)
#define SCU_HPLL_POST_SHIFT		13
#define SCU_HPLL_POST_MASK		(0x3f << SCU_HPLL_POST_SHIFT)

#define SCU_MACCLK_SHIFT		16
#define SCU_MACCLK_MASK			(7 << SCU_MACCLK_SHIFT)

#define SCU_MISC2_RGMII_HPLL		(1 << 23)
#define SCU_MISC2_RGMII_CLKDIV_SHIFT	20
#define SCU_MISC2_RGMII_CLKDIV_MASK	(3 << SCU_MISC2_RGMII_CLKDIV_SHIFT)
#define SCU_MISC2_RMII_MPLL		(1 << 19)
#define SCU_MISC2_RMII_CLKDIV_SHIFT	16
#define SCU_MISC2_RMII_CLKDIV_MASK	(3 << SCU_MISC2_RMII_CLKDIV_SHIFT)
#define SCU_MISC2_UARTCLK_SHIFT		24

#define SCU_MISC_D2PLL_OFF		(1 << 4)
#define SCU_MISC_UARTCLK_DIV13		(1 << 12)
#define SCU_MISC_GCRT_USB20CLK		(1 << 21)

#define SCU_MICDS_MAC1RGMII_TXDLY_SHIFT	0
#define SCU_MICDS_MAC1RGMII_TXDLY_MASK	(0x3f\
					 << SCU_MICDS_MAC1RGMII_TXDLY_SHIFT)
#define SCU_MICDS_MAC2RGMII_TXDLY_SHIFT	6
#define SCU_MICDS_MAC2RGMII_TXDLY_MASK	(0x3f\
					 << SCU_MICDS_MAC2RGMII_TXDLY_SHIFT)
#define SCU_MICDS_MAC1RMII_RDLY_SHIFT	12
#define SCU_MICDS_MAC1RMII_RDLY_MASK	(0x3f << SCU_MICDS_MAC1RMII_RDLY_SHIFT)
#define SCU_MICDS_MAC2RMII_RDLY_SHIFT	18
#define SCU_MICDS_MAC2RMII_RDLY_MASK	(0x3f << SCU_MICDS_MAC2RMII_RDLY_SHIFT)
#define SCU_MICDS_MAC1RMII_TXFALL	(1 << 24)
#define SCU_MICDS_MAC2RMII_TXFALL	(1 << 25)
#define SCU_MICDS_RMII1_RCLKEN		(1 << 29)
#define SCU_MICDS_RMII2_RCLKEN		(1 << 30)
#define SCU_MICDS_RGMIIPLL		(1 << 31)



/* Bits 16-27 in the register control pin functions for I2C devices 3-14 */
#define SCU_PINMUX_CTRL5_I2C		(1 << 16)

/*
 * The values are grouped by function, not by register.
 * They are actually scattered across multiple loosely related registers.
 */
#define SCU_PIN_FUN_MAC1_MDC		(1 << 30)
#define SCU_PIN_FUN_MAC1_MDIO		(1 << 31)
#define SCU_PIN_FUN_MAC1_PHY_LINK	(1 << 0)
#define SCU_PIN_FUN_MAC2_MDIO		(1 << 2)
#define SCU_PIN_FUN_MAC2_PHY_LINK	(1 << 1)
#define SCU_PIN_FUN_SCL1		(1 << 12)
#define SCU_PIN_FUN_SCL2		(1 << 14)
#define SCU_PIN_FUN_SDA1		(1 << 13)
#define SCU_PIN_FUN_SDA2		(1 << 15)

#define SCU_D2PLL_EXT1_OFF		(1 << 0)
#define SCU_D2PLL_EXT1_BYPASS		(1 << 1)
#define SCU_D2PLL_EXT1_RESET		(1 << 2)
#define SCU_D2PLL_EXT1_MODE_SHIFT	3
#define SCU_D2PLL_EXT1_MODE_MASK	(3 << SCU_D2PLL_EXT1_MODE_SHIFT)
#define SCU_D2PLL_EXT1_PARAM_SHIFT	5
#define SCU_D2PLL_EXT1_PARAM_MASK	(0x1ff << SCU_D2PLL_EXT1_PARAM_SHIFT)

#define SCU_D2PLL_NUM_SHIFT		0
#define SCU_D2PLL_NUM_MASK		(0xff << SCU_D2PLL_NUM_SHIFT)
#define SCU_D2PLL_DENUM_SHIFT		8
#define SCU_D2PLL_DENUM_MASK		(0x1f << SCU_D2PLL_DENUM_SHIFT)
#define SCU_D2PLL_POST_SHIFT		13
#define SCU_D2PLL_POST_MASK		(0x3f << SCU_D2PLL_POST_SHIFT)
#define SCU_D2PLL_ODIV_SHIFT		19
#define SCU_D2PLL_ODIV_MASK		(7 << SCU_D2PLL_ODIV_SHIFT)
#define SCU_D2PLL_SIC_SHIFT		22
#define SCU_D2PLL_SIC_MASK		(0x1f << SCU_D2PLL_SIC_SHIFT)
#define SCU_D2PLL_SIP_SHIFT		27
#define SCU_D2PLL_SIP_MASK		(0x1f << SCU_D2PLL_SIP_SHIFT)

#define SCU_CLKDUTY_DCLK_SHIFT		0
#define SCU_CLKDUTY_DCLK_MASK		(0x3f << SCU_CLKDUTY_DCLK_SHIFT)
#define SCU_CLKDUTY_RGMII1TXCK_SHIFT	8
#define SCU_CLKDUTY_RGMII1TXCK_MASK	(0x7f << SCU_CLKDUTY_RGMII1TXCK_SHIFT)
#define SCU_CLKDUTY_RGMII2TXCK_SHIFT	16
#define SCU_CLKDUTY_RGMII2TXCK_MASK	(0x7f << SCU_CLKDUTY_RGMII2TXCK_SHIFT)

struct ast2400_clk_priv {
	struct ast2400_scu *scu;
};

struct ast2400_scu {
	u32 protection_key;			/* 0x00 */
	u32 sysreset_ctrl1;			/* 0x04 */
	u32 clk_sel1;				/* 0x08 */
	u32 clk_stop_ctrl1;			/* 0x0C */
	u32 freq_counter_ctrl;		/* 0x10 */
	u32 freq_counter_measure;	/* 0x14 */
	u32 intr_ctrl;				/* 0x18 */
	u32 d2_pll_param;			/* 0x1C */
	u32 m_pll_param;			/* 0x20 */
	u32 h_pll_param;			/* 0x24 */
	u32 freq_counter_cmp;		/* 0x28 */
	u32 misc_ctrl1;				/* 0x2C */
	u32 pci_config[3];			/* 0x30 */
	u32 sysreset_status;		/* 0x3C */
	u32 vga_handshake[2];		/* 0x40 */
	u32 mac_clk_delay;			/* 0x48 */
	u32 misc_ctrl2;				/* 0x4C */
	u32 vga_scratch[8];			/* 0x50 */
	u32 hwstrap;				/* 0x70 */	
	u32 rng_ctrl;				/* 0x74 */
	u32 rng_data;				/* 0x78 */
	u32 rev_id;					/* 0x7C */
	u32 pinmux_ctrl[6];			/* 0x80 */
	u32 reserved0;				/* 0x98 */
	u32 wdt_rst_sel;			/* 0x9C */
	u32 pinmux_ctrl1[3];		/* 0xA0 */
	u32 reserved1[5];			/* 0xAC */
	u32 wakeup_enable;			/* 0xC0 */
	u32 wakeup_control;			/* 0xC4 */
	u32 reserved2[2];			/* 0xC8 */
	u32 hwstrap2;				/* 0xD0 */
	u32 reserved3[3];			/* 0xD4 */
	u32 freerun_counter;		/* 0xE0 */
	u32 freerun_counter_ext;	/* 0xE4 */
	//E8/EC
	//F0/F4/F8/FC
	u32 reserved4[6];			/* 0xE8 */
	/* The next registers are not key-protected */
	struct ast2400_cpu2 {		/* 0x100 */
		u32 ctrl;
		u32 base_addr[5];
		u32 cache_ctrl;
	} cpu2;
	u32 reserved5[17];			/* 0x11C */
	//11C/
	//120/124/128/12c
	//130/134/138/13c
	//140/144/148/14c
	//150/154/158/15c
	u32 uart_clk_ctrl;			/* 0x160 */
	u32 reserved7[7];
	u32 pcie_config;			/* 0x180 */
	u32 mmio_decode;
	u32 reloc_ctrl_decode[2];
	u32 mailbox_addr;
	u32 shared_sram_decode[2];
	u32 bmc_rev_id;
	u32 reserved8;
	u32 bmc_device_id;
	u32 reserved9[13];
	u32 clk_duty_sel;
};

#endif  /* _ASM_ARCH_SCU_AST2400_H */
