// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 *
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <regmap.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/arch/sdram_ast2600.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <dt-bindings/clock/ast2600-clock.h>
#include "sdram_phy_ast2600.h"

/* in order to speed up DRAM init time, write pre-defined values to registers
 * directly */
#define AST2600_SDRAMMC_MANUAL_CLK

/* register offset */
#define AST_SCU_FPGA_STATUS	0x004
#define AST_SCU_HANDSHAKE	0x100
#define AST_SCU_MPLL		0x220
#define AST_SCU_MPLL_EXT	0x224
#define AST_SCU_FPGA_PLL	0x400
#define AST_SCU_HW_STRAP	0x500


/* bit-field of AST_SCU_HW_STRAP */
#define SCU_HWSTRAP_VGAMEM_SHIFT	13
#define SCU_HWSTRAP_VGAMEM_MASK		GENMASK(14, 13)


/* bit-field of AST_SCU_HANDSHAKE */
#define SCU_SDRAM_INIT_READY_MASK	BIT(6)
#define SCU_SDRAM_INIT_BY_SOC_MASK	BIT(7)

/* bit-field of AST_SCU_MPLL */
#define SCU_MPLL_RESET			BIT(25)
#define SCU_MPLL_BYPASS			BIT(24)
#define SCU_MPLL_TURN_OFF		BIT(23)
#define SCU_MPLL_FREQ_MASK		GENMASK(22, 0)

#define SCU_MPLL_FREQ_400M		0x0008405F
#define SCU_MPLL_EXT_400M		0x00000031
//#define SCU_MPLL_FREQ_400M		0x0038007F
//#define SCU_MPLL_EXT_400M		0x0000003F
#define SCU_MPLL_FREQ_333M		0x00488299
#define SCU_MPLL_EXT_333M		0x0000014C
#define SCU_MPLL_FREQ_200M		0x0078007F
#define SCU_MPLL_EXT_200M		0x0000003F
#define SCU_MPLL_FREQ_100M		0x0078003F
#define SCU_MPLL_EXT_100M		0x0000001F
/* MPLL configuration */
#if defined(CONFIG_ASPEED_DDR4_1600)
#define SCU_MPLL_FREQ_CFG		SCU_MPLL_FREQ_400M
#define SCU_MPLL_EXT_CFG		SCU_MPLL_EXT_400M
#elif defined(CONFIG_ASPEED_DDR4_1333)
#define SCU_MPLL_FREQ_CFG		SCU_MPLL_FREQ_333M
#define SCU_MPLL_EXT_CFG		SCU_MPLL_EXT_333M
#elif defined(CONFIG_ASPEED_DDR4_800)
#define SCU_MPLL_FREQ_CFG		SCU_MPLL_FREQ_200M
#define SCU_MPLL_EXT_CFG		SCU_MPLL_EXT_200M
#elif defined(CONFIG_ASPEED_DDR4_400)
#define SCU_MPLL_FREQ_CFG		SCU_MPLL_FREQ_100M
#define SCU_MPLL_EXT_CFG		SCU_MPLL_EXT_100M
#else
#error "undefined DDR4 target rate\n"
#endif

/* AC timing and SDRAM mode registers */
#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)
/* mode register settings for FPGA are fixed */
#define DDR4_MR01_MODE		0x03010100
#define DDR4_MR23_MODE		0x00000000
#define DDR4_MR45_MODE		0x04C00000
#define DDR4_MR6_MODE		0x00000050
#define DDR4_TRFC_FPGA		0x17263434

/* FPGA need for an additional initialization procedure: search read window */
#define SEARCH_RDWIN_ANCHOR_0   (CONFIG_SYS_SDRAM_BASE + 0x0000)
#define SEARCH_RDWIN_ANCHOR_1   (CONFIG_SYS_SDRAM_BASE + 0x0004)
#define SEARCH_RDWIN_PTRN_0     0x12345678
#define SEARCH_RDWIN_PTRN_1     0xaabbccdd
#define SEARCH_RDWIN_PTRN_SUM   0xbcf02355
#else
/* mode register setting for real chip are derived from the model GDDR4-1600 */
#define DDR4_MR01_MODE		0x03010510
#define DDR4_MR23_MODE		0x00000000
#define DDR4_MR45_MODE		0x04000000
#define DDR4_MR6_MODE           0x00000400
#define DDR4_TRFC_1600		0x467299f1
#define DDR4_TRFC_1333		0x3a5f80c9
#define DDR4_TRFC_800		0x23394c78
#define DDR4_TRFC_400		0x111c263c
#endif /* end of "#if defined(CONFIG_FPGA_ASPEED) ||                           \
	  defined(CONFIG_ASPEED_PALLADIUM)" */

#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)
#define DDR4_TRFC			DDR4_TRFC_FPGA
#else
/* real chip setting */
#if defined(CONFIG_ASPEED_DDR4_1600)
#define DDR4_TRFC			DDR4_TRFC_1600
#define DDR4_PHY_TRAIN_TRFC		0xc30
#elif defined(CONFIG_ASPEED_DDR4_1333)
#define DDR4_TRFC			DDR4_TRFC_1333
#define DDR4_PHY_TRAIN_TRFC		0xa25
#elif defined(CONFIG_ASPEED_DDR4_800)
#define DDR4_TRFC			DDR4_TRFC_800
#define DDR4_PHY_TRAIN_TRFC		0x618
#elif defined(CONFIG_ASPEED_DDR4_400)
#define DDR4_TRFC			DDR4_TRFC_400
#define DDR4_PHY_TRAIN_TRFC		0x30c
#else
#error "undefined tRFC setting"
#endif	/* end of "#if (SCU_MPLL_FREQ_CFG == SCU_MPLL_FREQ_400M)" */
#endif  /* end of "#if defined(CONFIG_FPGA_ASPEED) ||                          \
	   defined(CONFIG_ASPEED_PALLADIUM)" */

/* supported SDRAM size */
#define SDRAM_SIZE_1KB		(1024U)
#define SDRAM_SIZE_1MB		(SDRAM_SIZE_1KB * SDRAM_SIZE_1KB)
#define SDRAM_MIN_SIZE		(256 * SDRAM_SIZE_1MB)
#define SDRAM_MAX_SIZE		(2048 * SDRAM_SIZE_1MB)


DECLARE_GLOBAL_DATA_PTR;

/*
 * Bandwidth configuration parameters for different SDRAM requests.
 * These are hardcoded settings taken from Aspeed SDK.
 */
#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)
static const u32 ddr4_ac_timing[4] = {0x030C0207, 0x04451133, 0x0E010200,
                                      0x00000140};

static const u32 ddr_max_grant_params[4] = {0x88888888, 0x88888888, 0x88888888,
                                            0x88888888};
#else
static const u32 ddr4_ac_timing[4] = {0x040e0307, 0x0f4711f1, 0x0e060304,
                                      0x00001240};

static const u32 ddr_max_grant_params[4] = {0x44444444, 0x44444444, 0x44444444,
                                            0x44444444};
#endif  

struct dram_info {
	struct ram_info info;
	struct clk ddr_clk;
	struct ast2600_sdrammc_regs *regs;
	void __iomem *scu;
	struct ast2600_ddr_phy *phy;
	void __iomem *phy_setting;
	void __iomem *phy_status;
	ulong clock_rate;
};

static void ast2600_sdramphy_kick_training(struct dram_info *info)
{
#if !defined(CONFIG_FPGA_ASPEED) && !defined(CONFIG_ASPEED_PALLADIUM)
	struct ast2600_sdrammc_regs *regs = info->regs;
	u32 volatile data;

	writel(SDRAM_PHYCTRL0_NRST, &regs->phy_ctrl[0]);
	udelay(5);
	writel(SDRAM_PHYCTRL0_NRST | SDRAM_PHYCTRL0_INIT, &regs->phy_ctrl[0]);
	udelay(1000);

	while (1) {
		data = readl(&regs->phy_ctrl[0]) & SDRAM_PHYCTRL0_INIT;
		if (~data) {			
			break;
		}
	}

#if 0
	while (1) {
		data = readl(0x1e6e0400) & BIT(1);
		if (data) {			
			break;
		}
	}
#endif
#endif
}

/**
 * @brief	load DDR-PHY configurations table to the PHY registers
 * @param[in]	p_tbl - pointer to the configuration table
 * @param[in]	info - pointer to the DRAM info struct
 * 
 * There are two sets of MRS (Mode Registers) configuration in ast2600 memory 
 * system: one is in the SDRAM MC (memory controller) which is used in run 
 * time, and the other is in the DDR-PHY IP which is used during DDR-PHY 
 * training.
*/
static void ast2600_sdramphy_init(u32 *p_tbl, struct dram_info *info)
{
#if !defined(CONFIG_FPGA_ASPEED) && !defined(CONFIG_ASPEED_PALLADIUM)
	u32 reg_base = (u32)info->phy_setting;
	u32 addr = p_tbl[0];
        u32 data;
        int i = 1;

	writel(0, &info->regs->phy_ctrl[0]);
	udelay(10);
	//writel(SDRAM_PHYCTRL0_NRST, &regs->phy_ctrl[0]);

	debug("%s:reg base = 0x%08x, 1st addr = 0x%08x\n", __func__, reg_base,
               addr);

        /* load PHY configuration table into PHY-setting registers */
        while (1) {
                if (addr < reg_base) {
                        debug("invalid DDR-PHY addr: 0x%08x\n", addr);
                        break;
                }
                data = p_tbl[i++];

                if (DDR_PHY_TBL_END == data) {
                        break;
                } else if (DDR_PHY_TBL_CHG_ADDR == data) {
                        addr = p_tbl[i++];
                } else {
                        writel(data, addr);
                        addr += 4;
                }
        }

	data = readl(info->phy_setting + 0x84) & ~GENMASK(16, 0);
	data |= DDR4_PHY_TRAIN_TRFC;
	writel(data, info->phy_setting + 0x84);
#endif        
}

static int ast2600_sdramphy_check_status(struct dram_info *info)
{
#if !defined(CONFIG_FPGA_ASPEED) && !defined(CONFIG_ASPEED_PALLADIUM)
        u32 value, tmp;
        u32 reg_base = (u32)info->phy_status;
	int need_retrain = 0;
	
	debug("\nSDRAM PHY training report:\n");
	/* training status */
        value = readl(reg_base + 0x00);
	debug("rO_DDRPHY_reg offset 0x00 = 0x%08x\n", value);
        if (value & BIT(3)) {
                debug("\tinitial PVT calibration fail\n");
        }
        if (value & BIT(5)) {
                debug("\truntime calibration fail\n");
        }

	/* PU & PD */
	value = readl(reg_base + 0x30);	
	debug("rO_DDRPHY_reg offset 0x30 = 0x%08x\n", value);
        debug("  PU = 0x%02x\n", value & 0xff);
        debug("  PD = 0x%02x\n", (value >> 16) & 0xff);

	/* read eye window */
        value = readl(reg_base + 0x68);
	if (0 == (value & GENMASK(7, 0))) {
		need_retrain = 1;
	}

	debug("rO_DDRPHY_reg offset 0x68 = 0x%08x\n", value);
	debug("  rising edge of read data eye training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 255;
        debug("    B1:%d%%\n", tmp);	

	value = readl(reg_base + 0xC8);
	debug("rO_DDRPHY_reg offset 0xC8 = 0x%08x\n", value);
	debug("  falling edge of read data eye training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 255;
        debug("    B1:%d%%\n", tmp);

        /* write eye window */
        value = readl(reg_base + 0x7c);
	if (0 == (value & GENMASK(7, 0))) {
		need_retrain = 1;
	}

	debug("rO_DDRPHY_reg offset 0x7C = 0x%08x\n", value);
	debug("  rising edge of write data eye training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 255;
        debug("    B1:%d%%\n", tmp);

	/* read Vref training result */
        value = readl(reg_base + 0x88);
	debug("rO_DDRPHY_reg offset 0x88 = 0x%08x\n", value);
        debug("  read Vref training result\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 127;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 127;
        debug("    B1:%d%%\n", tmp);

        /* write Vref training result */
        value = readl(reg_base + 0x90);
	debug("rO_DDRPHY_reg offset 0x90 = 0x%08x\n", value);
#if 0
	tmp = (((value & GENMASK(5, 0)) >> 0) * 100) / 127;
        debug("  write Vref training result = %d%%\n", tmp);
#endif	

        /* gate train */
	value = readl(reg_base + 0x50);
	if ((0 == (value & GENMASK(15, 0))) ||
	    (0 == (value & GENMASK(31, 16)))) {
		need_retrain = 1;
	}
#if 0		
	if (((value >> 24) & 0xff) < 3)
		need_retrain = 1;
	else
		need_retrain = 0;		
#endif
	debug("rO_DDRPHY_reg offset 0x50 = 0x%08x\n", value);
#if 0	
	debug("  gate training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    module 0: %d.%03d\n", (value >> 8) & 0xff, tmp);        
        tmp = (((value & GENMASK(23, 16)) >> 0) * 100) / 255;
	debug("    module 1: %d.%03d\n", (value >> 24) & 0xff, tmp);
#endif	

	return need_retrain;
#else
	return 0;	
#endif              
}
#ifndef CONFIG_ASPEED_BYPASS_SELFTEST
#define MC_TEST_PATTERN_N 8
static u32 as2600_sdrammc_test_pattern[MC_TEST_PATTERN_N] = {
    0xcc33cc33, 0xff00ff00, 0xaa55aa55, 0x88778877,
    0x92cc4d6e, 0x543d3cde, 0xf1e843c7, 0x7c61d253};

#define TIMEOUT_DRAM	5000000
int ast2600_sdrammc_dg_test(struct dram_info *info, unsigned int datagen, u32 mode)
{
	unsigned int data;
	unsigned int timeout = 0;
	struct ast2600_sdrammc_regs *regs = info->regs;

	writel(0, &regs->ecc_test_ctrl);
	if (mode == 0) {
		writel(0x00000085 | (datagen << 3), &regs->ecc_test_ctrl);
	} else {
		writel(0x000000C1 | (datagen << 3), &regs->ecc_test_ctrl);
	}

	do {
		data = readl(&regs->ecc_test_ctrl) & GENMASK(13, 12);

		if (data & BIT(13))
			return (0);

		if (++timeout > TIMEOUT_DRAM) {
			printf("Timeout!!\n");
			writel(0, &regs->ecc_test_ctrl);

			return (0);
		}
	} while (!data);

	writel(0, &regs->ecc_test_ctrl);

	return (1);
}

int ast2600_sdrammc_cbr_test(struct dram_info *info)
{
	struct ast2600_sdrammc_regs *regs = info->regs;
	u32 i;

	clrsetbits_le32(&regs->test_addr, GENMASK(30, 4), 0x7ffff0);

	/* single */
	for (i=0; i<8; i++) {
  		if(!ast2600_sdrammc_dg_test(info, i, 0))   return(0);
	}
	
	/* burst */
	for (i=0; i<8; i++) {
  		if(!ast2600_sdrammc_dg_test(info, i, i))   return(0);
	}

	return(1);
}

static int ast2600_sdrammc_test(struct dram_info *info) 
{
	struct ast2600_sdrammc_regs *regs = info->regs;

	u32 pass_cnt = 0;
	u32 fail_cnt = 0;
	u32 target_cnt = 2;
	u32 test_cnt = 0;
	u32 pattern;
	u32 i = 0;
	bool finish = false;

	debug("sdram mc test:\n");
	while (finish == false) {
		pattern = as2600_sdrammc_test_pattern[i++];
		i = i % MC_TEST_PATTERN_N;
		debug("  pattern = %08X : ",pattern);
		writel(pattern, regs->test_init_val);

		if (!ast2600_sdrammc_cbr_test(info)) {
			debug("fail\n");
			fail_cnt++;
		} else {
			debug("pass\n");
			pass_cnt++;
		}

		if (++test_cnt == target_cnt) {
			finish = true;
		}
	}
	debug("statistics: pass/fail/total:%d/%d/%d\n", pass_cnt, fail_cnt,
	       target_cnt);
	return fail_cnt;
}
#endif
/**
 * scu500[14:13]
 * 	2b'00: VGA memory size = 16MB
 * 	2b'01: VGA memory size = 16MB
 * 	2b'10: VGA memory size = 32MB
 * 	2b'11: VGA memory size = 64MB
 *
 * mcr04[3:2]
 * 	2b'00: VGA memory size = 8MB
 * 	2b'01: VGA memory size = 16MB
 * 	2b'10: VGA memory size = 32MB
 * 	2b'11: VGA memory size = 64MB
*/
static size_t ast2600_sdrammc_get_vga_mem_size(struct dram_info *info)
{
        u32 vga_hwconf;
        size_t vga_mem_size_base = 8 * 1024 * 1024;

	vga_hwconf =
	    (readl(info->scu + AST_SCU_HW_STRAP) & SCU_HWSTRAP_VGAMEM_MASK) >>
	    SCU_HWSTRAP_VGAMEM_SHIFT;	

	vga_hwconf = (vga_hwconf == 0) ? 1 : vga_hwconf;

	clrsetbits_le32(&info->regs->config, SDRAM_CONF_VGA_SIZE_MASK,
			((vga_hwconf << SDRAM_CONF_VGA_SIZE_SHIFT) &
			 SDRAM_CONF_VGA_SIZE_MASK));	    

	return vga_mem_size_base << vga_hwconf;
}
#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)
static void ast2600_sdrammc_fpga_set_pll(struct dram_info *info)
{
        u32 data;
        u32 scu_base = (u32)info->scu;

        writel(0x00000303, scu_base + AST_SCU_FPGA_PLL);                                
        
        do {
                data = readl(scu_base + AST_SCU_FPGA_STATUS);
        } while (!(data & 0x100));
        
        writel(0x00000103, scu_base + AST_SCU_FPGA_PLL);
}

static int ast2600_sdrammc_search_read_window(struct dram_info *info)
{
        u32 pll, pll_min, pll_max, dat1, offset;
        u32 win = 0x03, gwin = 0, gwinsize = 0;
        u32 phy_setting = (u32)info->phy_setting;                

#ifdef CONFIG_ASPEED_PALLADIUM
	writel(0xc, phy_setting + 0x0000);
	return (1);
#endif
        writel(SEARCH_RDWIN_PTRN_0, SEARCH_RDWIN_ANCHOR_0);
        writel(SEARCH_RDWIN_PTRN_1, SEARCH_RDWIN_ANCHOR_1);

        while (gwin == 0) {
                while (!(win & 0x80)) {
                        debug("Window = 0x%X\n", win);
                        writel(win, phy_setting + 0x0000);

                        dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
                        dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
                        while (dat1 == SEARCH_RDWIN_PTRN_SUM) {
                                ast2600_sdrammc_fpga_set_pll(info);
                                dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
                                dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
                        }

                        pll_min = 0xfff;
                        pll_max = 0x0;
                        pll = 0;
                        while (pll_max > 0 || pll < 256) {
                                ast2600_sdrammc_fpga_set_pll(info);
                                dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
                                dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
                                if (dat1 == SEARCH_RDWIN_PTRN_SUM) {
                                        if (pll_min > pll) {
                                                pll_min = pll;
                                        }
                                        if (pll_max < pll) {
                                                pll_max = pll;
                                        }
                                        debug("%3d_(%3d:%3d)\n", pll, pll_min,
                                               pll_max);
                                } else if (pll_max > 0) {
                                        pll_min = pll_max - pll_min;
                                        if (gwinsize < pll_min) {
                                                gwin = win;
                                                gwinsize = pll_min;
                                        }
                                        break;
                                }
                                pll += 1;
                        }

                        if (gwin != 0 && pll_max == 0) {
                                break;
                        }
                        win = win << 1;
                }
                if (gwin == 0) {
                        win = 0x7;
                }
        }
        debug("Set PLL Read Gating Window = %x\n", gwin);
        writel(gwin, phy_setting + 0x0000);

        debug("PLL Read Window training\n");
        pll_min = 0xfff;
        pll_max = 0x0;

        debug("Search Window Start\n");
        dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
        dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
        while (dat1 == SEARCH_RDWIN_PTRN_SUM) {
                ast2600_sdrammc_fpga_set_pll(info);
                dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
                dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
        }

        debug("Search Window Margin\n");
        pll = 0;
        while (pll_max > 0 || pll < 256) {
                ast2600_sdrammc_fpga_set_pll(info);
                dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
                dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
                if (dat1 == SEARCH_RDWIN_PTRN_SUM) {
                        if (pll_min > pll) {
                                pll_min = pll;
                        }
                        if (pll_max < pll) {
                                pll_max = pll;
                        }
                        debug("%3d_(%3d:%3d)\n", pll, pll_min, pll_max);
                } else if (pll_max > 0 && (pll_max - pll_min) > 20) {
                        break;
                } else if (pll_max > 0) {
                        pll_min = 0xfff;
                        pll_max = 0x0;
                }
                pll += 1;
        }
        if (pll_min < pll_max) {
                debug("PLL Read window = %d\n", (pll_max - pll_min));
                offset = (pll_max - pll_min) >> 1;
                pll_min = 0xfff;
                pll = 0;
                while (pll < (pll_min + offset)) {
                        ast2600_sdrammc_fpga_set_pll(info);
                        dat1 = readl(SEARCH_RDWIN_ANCHOR_0);
                        dat1 += readl(SEARCH_RDWIN_ANCHOR_1);
                        if (dat1 == SEARCH_RDWIN_PTRN_SUM) {
                                if (pll_min > pll) {
                                        pll_min = pll;
                                }
                                debug("%d\n", pll);
                        } else {
                                pll_min = 0xfff;
                                pll_max = 0x0;
                        }
                        pll += 1;
                }
                return (1);
        } else {
                debug("PLL Read window training fail\n");
                return (0);
        }
}
#endif /* end of "#if defined(CONFIG_FPGA_ASPEED) ||                           \
	  defined(CONFIG_ASPEED_PALLADIUM)" */

/*
 * Find out RAM size and save it in dram_info
 *
 * The procedure is taken from Aspeed SDK
 */
static void ast2600_sdrammc_calc_size(struct dram_info *info)
{
	/* The controller supports 256/512/1024/2048 MB ram */
	size_t ram_size = SDRAM_MIN_SIZE;
	const int write_test_offset = 0x100000;
	u32 test_pattern = 0xdeadbeef;
	u32 cap_param = SDRAM_CONF_CAP_2048M;
	u32 refresh_timing_param = DDR4_TRFC;
	const u32 write_addr_base = CONFIG_SYS_SDRAM_BASE + write_test_offset;

	for (ram_size = SDRAM_MAX_SIZE; ram_size > SDRAM_MIN_SIZE;
	     ram_size >>= 1) {
		writel(test_pattern, write_addr_base + (ram_size >> 1));
		test_pattern = (test_pattern >> 4) | (test_pattern << 28);
	}

	/* One last write to overwrite all wrapped values */
	writel(test_pattern, write_addr_base);

	/* Reset the pattern and see which value was really written */
	test_pattern = 0xdeadbeef;
	for (ram_size = SDRAM_MAX_SIZE; ram_size > SDRAM_MIN_SIZE;
	     ram_size >>= 1) {
		if (readl(write_addr_base + (ram_size >> 1)) == test_pattern)
			break;

		--cap_param;
		refresh_timing_param >>= 8;
		test_pattern = (test_pattern >> 4) | (test_pattern << 28);
	}

	clrsetbits_le32(&info->regs->ac_timing[1],
			(SDRAM_AC_TRFC_MASK << SDRAM_AC_TRFC_SHIFT),
			((refresh_timing_param & SDRAM_AC_TRFC_MASK)
			 << SDRAM_AC_TRFC_SHIFT));

	info->info.base = CONFIG_SYS_SDRAM_BASE;
	info->info.size = ram_size - ast2600_sdrammc_get_vga_mem_size(info);
	clrsetbits_le32(
	    &info->regs->config, SDRAM_CONF_CAP_MASK,
	    ((cap_param << SDRAM_CONF_CAP_SHIFT) & SDRAM_CONF_CAP_MASK));
}

static int ast2600_sdrammc_init_ddr4(struct dram_info *info)
{
        const u32 power_ctrl = MCR34_CKE_EN | MCR34_AUTOPWRDN_EN |
                               MCR34_MREQ_BYPASS_DIS | MCR34_RESETN_DIS |
                               MCR34_ODT_EN | MCR34_ODT_AUTO_ON |
                               (0x1 << MCR34_ODT_EXT_SHIFT);

        /* init SDRAM-PHY only on real chip */
	ast2600_sdramphy_init(ast2600_sdramphy_config, info);
        writel((MCR34_CKE_EN | MCR34_MREQI_DIS | MCR34_RESETN_DIS),
               &info->regs->power_ctrl);
	udelay(5);	       
	ast2600_sdramphy_kick_training(info);
	udelay(500);
        writel(SDRAM_RESET_DLL_ZQCL_EN, &info->regs->refresh_timing);

        writel(MCR30_SET_MR(3), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(6), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(5), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(4), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(2), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(1), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(0) | MCR30_RESET_DLL_DELAY_EN,
               &info->regs->mode_setting_control);

#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)

        writel(SDRAM_REFRESH_EN | SDRAM_RESET_DLL_ZQCL_EN |
                   (0x5d << SDRAM_REFRESH_PERIOD_SHIFT),
               &info->regs->refresh_timing);
#else
        writel(SDRAM_REFRESH_EN | SDRAM_RESET_DLL_ZQCL_EN |
                   (0x5f << SDRAM_REFRESH_PERIOD_SHIFT),
               &info->regs->refresh_timing);
#endif

        /* wait self-refresh idle */
        while (readl(&info->regs->power_ctrl) & MCR34_SELF_REFRESH_STATUS_MASK)
                ;

#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)
        writel(SDRAM_REFRESH_EN | SDRAM_LOW_PRI_REFRESH_EN |
                   SDRAM_REFRESH_ZQCS_EN |
                   (0x5d << SDRAM_REFRESH_PERIOD_SHIFT) |
                   (0x4000 << SDRAM_REFRESH_PERIOD_ZQCS_SHIFT),
               &info->regs->refresh_timing);
#else
        writel(SDRAM_REFRESH_EN | SDRAM_LOW_PRI_REFRESH_EN |
                   SDRAM_REFRESH_ZQCS_EN |
                   (0x5f << SDRAM_REFRESH_PERIOD_SHIFT) |
                   (0x42aa << SDRAM_REFRESH_PERIOD_ZQCS_SHIFT),
               &info->regs->refresh_timing);
#endif

        writel(power_ctrl, &info->regs->power_ctrl);
	udelay(500);

#if defined(CONFIG_FPGA_ASPEED)
        /* toggle Vref training */
        setbits_le32(&info->regs->mr6_mode_setting, 0x80);
        writel(MCR30_RESET_DLL_DELAY_EN | MCR30_SET_MR(6),
               &info->regs->mode_setting_control);
        clrbits_le32(&info->regs->mr6_mode_setting, 0x80);
        writel(MCR30_RESET_DLL_DELAY_EN | MCR30_SET_MR(6),
               &info->regs->mode_setting_control);
#endif
	return 0;
}

static void ast2600_sdrammc_unlock(struct dram_info *info)
{
	writel(SDRAM_UNLOCK_KEY, &info->regs->protection_key);
	while (!readl(&info->regs->protection_key))
		;
}

static void ast2600_sdrammc_lock(struct dram_info *info)
{
	writel(~SDRAM_UNLOCK_KEY, &info->regs->protection_key);
	while (readl(&info->regs->protection_key))
		;
}

static void ast2600_sdrammc_common_init(struct ast2600_sdrammc_regs *regs)
{
	int i;

        writel(MCR34_MREQI_DIS | MCR34_RESETN_DIS, &regs->power_ctrl);
        writel(SDRAM_VIDEO_UNLOCK_KEY, &regs->gm_protection_key);
        writel(0x10 << MCR38_RW_MAX_GRANT_CNT_RQ_SHIFT,
               &regs->arbitration_ctrl);
        writel(0xFFBBFFF4, &regs->req_limit_mask);

	for (i = 0; i < ARRAY_SIZE(ddr_max_grant_params); ++i)
                writel(ddr_max_grant_params[i], &regs->max_grant_len[i]);	

        writel(MCR50_RESET_ALL_INTR, &regs->intr_ctrl);

        /* FIXME: the sample code does NOT match the datasheet */
        writel(0x07FFFFFF, &regs->ecc_range_ctrl);

        writel(0, &regs->ecc_test_ctrl);
        writel(0x80000001, &regs->test_addr);
        writel(0, &regs->test_fail_dq_bit);
        writel(0, &regs->test_init_val);

        writel(0xFFFFFFFF, &regs->req_input_ctrl);
        writel(0, &regs->req_high_pri_ctrl);

        udelay(600);

#ifdef CONFIG_ASPEED_DDR4_DUALX8
	writel(0x37, &regs->config);
#else
	writel(0x17, &regs->config);
#endif

	/* load controller setting */
	for (i = 0; i < ARRAY_SIZE(ddr4_ac_timing); ++i)
		writel(ddr4_ac_timing[i], &regs->ac_timing[i]);

	writel(DDR4_MR01_MODE, &regs->mr01_mode_setting);
	writel(DDR4_MR23_MODE, &regs->mr23_mode_setting);
	writel(DDR4_MR45_MODE, &regs->mr45_mode_setting);
	writel(DDR4_MR6_MODE, &regs->mr6_mode_setting);
}

#ifdef CONFIG_ASPEED_ECC
static void ast2600_sdrammc_ecc_enable(struct dram_info *info)
{
	struct ast2600_sdrammc_regs *regs = info->regs;
	size_t conf_size;
	u32 reg;
	
	conf_size = CONFIG_ASPEED_ECC_SIZE * SDRAM_SIZE_1MB;
	if (conf_size > info->info.size) {
		printf("warning: ECC configured %dMB but actual size is %dMB\n",
		       CONFIG_ASPEED_ECC_SIZE,
		       info->info.size / SDRAM_SIZE_1MB);
		conf_size = info->info.size;
	} else if (conf_size == 0) {
		conf_size = info->info.size;
	}

	info->info.size = (((conf_size / 9) * 8) >> 20) << 20;
	writel(((info->info.size >> 20) - 1) << 20, &regs->ecc_range_ctrl);
	reg = readl(&regs->config) |
	      (SDRAM_CONF_ECC_EN | SDRAM_CONF_ECC_AUTO_SCRUBBING);
	writel(reg, &regs->config);

	writel(0, &regs->test_init_val);
	writel(0x80000001, &regs->test_addr);
	writel(0x221, &regs->ecc_test_ctrl);
	while (0 == (readl(&regs->ecc_test_ctrl) & BIT(12)))
		;
	writel(0, &regs->ecc_test_ctrl);
	writel(BIT(31), &regs->intr_ctrl);
	writel(0, &regs->intr_ctrl);
	printf("ECC enable, ");
}
#endif

static int ast2600_sdrammc_probe(struct udevice *dev)
{
	struct dram_info *priv = (struct dram_info *)dev_get_priv(dev);
	struct ast2600_sdrammc_regs *regs = priv->regs;
	struct udevice *clk_dev;
	int ret;
	volatile uint32_t reg;

	/* find SCU base address from clock device */
	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_GET_DRIVER(aspeed_scu),
                                          &clk_dev);
	if (ret) {
		debug("clock device not defined\n");
		return ret;
	}

	priv->scu = devfdt_get_addr_ptr(clk_dev);
	if (IS_ERR(priv->scu)) {
		debug("%s(): can't get SCU\n", __func__);
		return PTR_ERR(priv->scu);
	}

	if (readl(priv->scu + AST_SCU_HANDSHAKE) & SCU_SDRAM_INIT_READY_MASK) {
		printf("already initialized, ");
		ast2600_sdrammc_calc_size(priv);
#ifdef CONFIG_ASPEED_ECC		
		ast2600_sdrammc_ecc_enable(priv);
#endif
		return 0;
	}

#ifdef AST2600_SDRAMMC_MANUAL_CLK
	reg = readl(priv->scu + AST_SCU_MPLL);
	reg &= ~(BIT(24) | GENMASK(22, 0));
	reg |= (BIT(25) | BIT(23) | SCU_MPLL_FREQ_CFG);
	writel(reg, priv->scu + AST_SCU_MPLL);
        writel(SCU_MPLL_EXT_CFG, priv->scu + AST_SCU_MPLL_EXT);
	udelay(100);
	reg &= ~(BIT(25) | BIT(23));
	writel(reg, priv->scu + AST_SCU_MPLL);
	while(0 == (readl(priv->scu + AST_SCU_MPLL_EXT) & BIT(31)));
#else
	ret = clk_get_by_index(dev, 0, &priv->ddr_clk);
	if (ret) {
		debug("DDR:No CLK\n");
		return ret;
	}
	clk_set_rate(&priv->ddr_clk, priv->clock_rate);
#endif

#if 0
	/* FIXME: enable the following code if reset-driver is ready */
	struct reset_ctl reset_ctl;
	ret = reset_get_by_index(dev, 0, &reset_ctl);
	if (ret) {
		debug("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	ret = reset_assert(&reset_ctl);
	if (ret) {
		debug("%s(): SDRAM reset failed: %u\n", __func__, ret);
		return ret;
	}
#endif

	ast2600_sdrammc_unlock(priv);
	ast2600_sdrammc_common_init(regs);
L_ast2600_sdramphy_train:		
	ast2600_sdrammc_init_ddr4(priv);

#if defined(CONFIG_FPGA_ASPEED) || defined(CONFIG_ASPEED_PALLADIUM)
        ast2600_sdrammc_search_read_window(priv);
#else	
	/* make sure DDR-PHY is ready before access */
	do {
		reg = readl(priv->phy_status) & BIT(1);
	} while(reg == 0);
#endif

	if (0 != ast2600_sdramphy_check_status(priv)) {
		printf("DDR4 PHY training fail, retrain\n");
		goto L_ast2600_sdramphy_train;
	}

	ast2600_sdrammc_calc_size(priv);

#ifndef CONFIG_ASPEED_BYPASS_SELFTEST
        if (0 != ast2600_sdrammc_test(priv)) {
		printf("%s: DDR4 init fail\n", __func__);
		return -EINVAL;
	}
#endif	

#ifdef CONFIG_ASPEED_ECC
	ast2600_sdrammc_ecc_enable(priv);
#endif	

	writel(readl(priv->scu + AST_SCU_HANDSHAKE) | SCU_SDRAM_INIT_READY_MASK,
	       priv->scu + AST_SCU_HANDSHAKE);

	clrbits_le32(&regs->intr_ctrl, MCR50_RESET_ALL_INTR);
	ast2600_sdrammc_lock(priv);
	return 0;
}

static int ast2600_sdrammc_ofdata_to_platdata(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	priv->regs = (void *)(uintptr_t)devfdt_get_addr_index(dev, 0);
	priv->phy_setting = (void *)(uintptr_t)devfdt_get_addr_index(dev, 1);
	priv->phy_status = (void *)(uintptr_t)devfdt_get_addr_index(dev, 2);

	priv->clock_rate = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					  "clock-frequency", 0);
	if (!priv->clock_rate) {
		debug("DDR Clock Rate not defined\n");
		return -EINVAL;
	}

	return 0;
}

static int ast2600_sdrammc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops ast2600_sdrammc_ops = {
	.get_info = ast2600_sdrammc_get_info,
};

static const struct udevice_id ast2600_sdrammc_ids[] = {
	{ .compatible = "aspeed,ast2600-sdrammc" },
	{ }
};

U_BOOT_DRIVER(sdrammc_ast2600) = {
	.name = "aspeed_ast2600_sdrammc",
	.id = UCLASS_RAM,
	.of_match = ast2600_sdrammc_ids,
	.ops = &ast2600_sdrammc_ops,
	.ofdata_to_platdata = ast2600_sdrammc_ofdata_to_platdata,
	.probe = ast2600_sdrammc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
};
