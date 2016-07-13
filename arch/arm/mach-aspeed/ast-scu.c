/*******************************************************************************
 * File Name     : arch/arm/cpu/ast-common/ast-scu.c
 * Author         : Ryan Chen
 * Description   : AST SCU
 *
 * Copyright (C) 2012-2020 ASPEED Technology Inc.  This program is free
 * software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * CLK24M
 *  |
 *  |--> H-PLL -->HCLK
 *  |
 *  |--> M-PLL -xx->MCLK
 *  |
 *  |--> V-PLL1 -xx->DCLK
 *  |
 *  |--> V-PLL2 -xx->D2CLK
 *  |
 *  |--> USB2PHY -->UTMICLK
 *
 *   History      :
 *    1. 2012/08/15 Ryan Chen Create
 *
 ******************************************************************************/
#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch/regs-scu.h>
#include <asm/arch/ast_scu.h>
#include <asm/arch/platform.h>
#include <asm/arch/aspeed.h>

/* #define ASPEED_SCU_LOCK */

static inline u32 ast_scu_read(u32 reg)
{
	u32 val = readl(AST_SCU_BASE + reg);

	debug("ast_scu_read : reg = 0x%08x, val = 0x%08x\n", reg, val);
	return val;
}

static inline void ast_scu_write(u32 val, u32 reg)
{
	debug("ast_scu_write : reg = 0x%08x, val = 0x%08x\n", reg, val);

	writel(SCU_PROTECT_UNLOCK, AST_SCU_BASE);
	writel(val, AST_SCU_BASE + reg);
#ifdef CONFIG_AST_SCU_LOCK
	writel(0xaa, AST_SCU_BASE);
#endif
}

/* SoC mapping Table */
struct soc_id {
	const char *name;
	u32	   rev_id;
};

#define SOC_ID(str, rev) { .name = str, .rev_id = rev, }

static struct soc_id soc_map_table[] = {
	SOC_ID("AST1100/AST2050-A0", 0x00000200),
	SOC_ID("AST1100/AST2050-A1", 0x00000201),
	SOC_ID("AST1100/AST2050-A2,3/AST2150-A0,1", 0x00000202),
	SOC_ID("AST1510/AST2100-A0", 0x00000300),
	SOC_ID("AST1510/AST2100-A1", 0x00000301),
	SOC_ID("AST1510/AST2100-A2,3", 0x00000302),
	SOC_ID("AST2200-A0,1", 0x00000102),
	SOC_ID("AST2300-A0", 0x01000003),
	SOC_ID("AST2300-A1", 0x01010303),
	SOC_ID("AST1300-A1", 0x01010003),
	SOC_ID("AST1050-A1", 0x01010203),
	SOC_ID("AST2400-A0", 0x02000303),
	SOC_ID("AST2400-A1", 0x02010303),
	SOC_ID("AST1010-A0", 0x03000003),
	SOC_ID("AST1010-A1", 0x03010003),
	SOC_ID("AST1520-A0", 0x03000203),
	SOC_ID("AST3200-A0", 0x03000303),
	SOC_ID("AST2500-A0", 0x04000303),
	SOC_ID("AST2510-A0", 0x04000103),
	SOC_ID("AST2520-A0", 0x04000203),
	SOC_ID("AST2530-A0", 0x04000403),
	SOC_ID("AST1520-A1", 0x03010203),
	SOC_ID("AST3200-A1", 0x03010303),
	SOC_ID("AST2500-A1", 0x04010303),
	SOC_ID("AST2510-A1", 0x04010103),
	SOC_ID("AST2520-A1", 0x04010203),
	SOC_ID("AST2530-A1", 0x04010403),
};

void ast_scu_init_eth(u8 num)
{
/* Set MAC delay Timing */
#ifndef AST_SOC_G5
	/* AST2300 max clk to 125Mhz, AST2400 max clk to 198Mhz */

	/* RGMII --> H-PLL/6 */
	if (ast_scu_read(AST_SCU_HW_STRAP1) &
	   (SCU_HW_STRAP_MAC1_RGMII | SCU_HW_STRAP_MAC0_RGMII))
		ast_scu_write((ast_scu_read(AST_SCU_CLK_SEL) &
			       ~SCU_CLK_MAC_MASK) | SCU_CLK_MAC_DIV(2),
			      AST_SCU_CLK_SEL);
	else /* RMII --> H-PLL/10 */
		ast_scu_write((ast_scu_read(AST_SCU_CLK_SEL) &
			       ~SCU_CLK_MAC_MASK) | SCU_CLK_MAC_DIV(4),
			      AST_SCU_CLK_SEL);

	ast_scu_write(0x2255, AST_SCU_MAC_CLK);
#endif

	switch (num) {
	case 0:
		ast_scu_write(ast_scu_read(AST_SCU_RESET) | SCU_RESET_MAC0,
			      AST_SCU_RESET);
		udelay(100);
		ast_scu_write(ast_scu_read(AST_SCU_CLK_STOP) &
			      ~SCU_MAC0CLK_STOP_EN, AST_SCU_CLK_STOP);
		udelay(1000);
		ast_scu_write(ast_scu_read(AST_SCU_RESET) & ~SCU_RESET_MAC0,
			      AST_SCU_RESET);

		break;
#if defined(AST_MAC1_BASE)
	case 1:
		ast_scu_write(ast_scu_read(AST_SCU_RESET) | SCU_RESET_MAC1,
			      AST_SCU_RESET);
		udelay(100);
		ast_scu_write(ast_scu_read(AST_SCU_CLK_STOP) &
			      ~SCU_MAC1CLK_STOP_EN, AST_SCU_CLK_STOP);
		udelay(1000);
		ast_scu_write(ast_scu_read(AST_SCU_RESET) & ~SCU_RESET_MAC1,
			      AST_SCU_RESET);
		break;
#endif
	}
}

/* 0: disable spi
 * 1: enable spi master
 * 2: enable spi master and spi slave to ahb
 * 3: enable spi pass-through
 */
void ast_scu_spi_master(u8 mode)
{
#ifdef AST_SOC_G5
	switch (mode) {
	case 0:
		ast_scu_write(SCU_HW_STRAP_SPI_MODE_MASK, AST_SCU_REVISION_ID);
		break;
	case 1:
		ast_scu_write(SCU_HW_STRAP_SPI_MODE_MASK, AST_SCU_REVISION_ID);
		ast_scu_write(SCU_HW_STRAP_SPI_MASTER, AST_SCU_HW_STRAP1);
		break;
	case 2:
		ast_scu_write(SCU_HW_STRAP_SPI_MODE_MASK, AST_SCU_REVISION_ID);
		ast_scu_write(SCU_HW_STRAP_SPI_M_S_EN, AST_SCU_HW_STRAP1);
		break;
	case 3:
		ast_scu_write(SCU_HW_STRAP_SPI_MODE_MASK, AST_SCU_REVISION_ID);
		ast_scu_write(SCU_HW_STRAP_SPI_PASS_THROUGH, AST_SCU_HW_STRAP1);
		break;
	}
#else
	switch (mode) {
	case 0:
		ast_scu_write(ast_scu_read(AST_SCU_HW_STRAP1) &
			      ~SCU_HW_STRAP_SPI_MODE_MASK, AST_SCU_HW_STRAP1);
		break;
	case 1:
		ast_scu_write((ast_scu_read(AST_SCU_HW_STRAP1) &
			       ~SCU_HW_STRAP_SPI_MODE_MASK) |
			      SCU_HW_STRAP_SPI_MASTER, AST_SCU_HW_STRAP1);
		break;
	case 2:
		ast_scu_write((ast_scu_read(AST_SCU_HW_STRAP1) &
			       ~SCU_HW_STRAP_SPI_MODE_MASK) |
			      SCU_HW_STRAP_SPI_MASTER, AST_SCU_HW_STRAP1);
		break;
	case 3:
		ast_scu_write((ast_scu_read(AST_SCU_HW_STRAP1) &
			       ~SCU_HW_STRAP_SPI_MODE_MASK) |
			      SCU_HW_STRAP_SPI_PASS_THROUGH, AST_SCU_HW_STRAP1);
		break;
	}
#endif
}

u32 ast_get_clk_source(void)
{
	if (ast_scu_read(AST_SCU_HW_STRAP1) & CLK_25M_IN)
		return AST_PLL_25MHZ;
	else
		return AST_PLL_24MHZ;
}

#if defined(AST_SOC_G5)

u32 ast_get_h_pll_clk(void)
{
	u32 clk = 0;
	u32 h_pll_set = ast_scu_read(AST_SCU_H_PLL);

	if (h_pll_set & SCU_H_PLL_OFF)
		return 0;

	/* Programming */
	clk = ast_get_clk_source();
	if (h_pll_set & SCU_H_PLL_BYPASS_EN) {
		return clk;
	} else {
		/* P = SCU24[18:13]
		 * M = SCU24[12:5]
		 * N = SCU24[4:0]
		 * hpll = 24MHz * [(M+1) /(N+1)] / (P+1)
		 */
		clk = ((clk * (SCU_H_PLL_GET_MNUM(h_pll_set) + 1)) /
		       (SCU_H_PLL_GET_NNUM(h_pll_set) + 1)) /
			(SCU_H_PLL_GET_PNUM(h_pll_set) + 1);
	}
	debug("h_pll = %d\n", clk);
	return clk;
}

u32 ast_get_ahbclk(void)
{
	unsigned int axi_div, ahb_div, hpll;

	hpll = ast_get_h_pll_clk();
	/* AST2500 A1 fix */
	axi_div = 2;
	ahb_div = (SCU_HW_STRAP_GET_AXI_AHB_RATIO(
			   ast_scu_read(AST_SCU_HW_STRAP1)) + 1);

	debug("HPLL=%d, AXI_Div=%d, AHB_DIV = %d, AHB CLK=%d\n", hpll, axi_div,
	      ahb_div, (hpll / axi_div) / ahb_div);
	return ((hpll / axi_div) / ahb_div);
}

#else /* ! AST_SOC_G5 */

u32 ast_get_h_pll_clk(void)
{
	u32 speed, clk = 0;
	u32 h_pll_set = ast_scu_read(AST_SCU_H_PLL);

	if (h_pll_set & SCU_H_PLL_OFF)
		return 0;

	if (h_pll_set & SCU_H_PLL_PARAMETER) {
		/* Programming */
		clk = ast_get_clk_source();
		if (h_pll_set & SCU_H_PLL_BYPASS_EN) {
			return clk;
		} else {
		/* OD == SCU24[4]
		 * OD = SCU_H_PLL_GET_DIV(h_pll_set);
		 * Numerator == SCU24[10:5]
		 * num = SCU_H_PLL_GET_NUM(h_pll_set);
		 * Denumerator == SCU24[3:0]
		 * denum = SCU_H_PLL_GET_DENUM(h_pll_set);
		 * hpll = 24MHz * (2-OD) * ((Numerator+2)/(Denumerator+1))
		 */
			clk = ((clk * (2 - SCU_H_PLL_GET_DIV(h_pll_set)) *
				(SCU_H_PLL_GET_NUM(h_pll_set) + 2)) /
			       (SCU_H_PLL_GET_DENUM(h_pll_set) + 1));
		}
	} else {
		/* HW Trap */
		speed = SCU_HW_STRAP_GET_H_PLL_CLK(
			ast_scu_read(AST_SCU_HW_STRAP1));
		switch (speed) {
		case 0:
			clk = 384000000;
			break;
		case 1:
			clk = 360000000;
			break;
		case 2:
			clk = 336000000;
			break;
		case 3:
			clk = 408000000;
			break;
		default:
			BUG();
			break;
		}
	}
	debug("h_pll = %d\n", clk);
	return clk;
}

u32 ast_get_ahbclk(void)
{
	unsigned int div, hpll;

	hpll = ast_get_h_pll_clk();
	div = SCU_HW_STRAP_GET_CPU_AHB_RATIO(ast_scu_read(AST_SCU_HW_STRAP1));
	div += 1;

	debug("HPLL=%d, Div=%d, AHB CLK=%d\n", hpll, div, hpll / div);
	return (hpll / div);
}

#endif /* AST_SOC_G5 */

void ast_scu_show_system_info(void)
{

#ifdef AST_SOC_G5
	unsigned int axi_div, ahb_div, h_pll;

	h_pll = ast_get_h_pll_clk();

	/* AST2500 A1 fix */
	axi_div = 2;
	ahb_div = (SCU_HW_STRAP_GET_AXI_AHB_RATIO(
			   ast_scu_read(AST_SCU_HW_STRAP1)) + 1);

	printf("CPU = %d MHz , AXI = %d MHz, AHB = %d MHz (%d:%d:1)\n",
	       h_pll / 1000000,
	       h_pll / axi_div / 1000000,
	       h_pll / axi_div / ahb_div / 1000000, axi_div, ahb_div);

#else
	u32 h_pll, div;

	h_pll = ast_get_h_pll_clk();

	div = SCU_HW_STRAP_GET_CPU_AHB_RATIO(ast_scu_read(AST_SCU_HW_STRAP1));
	div += 1;

	printf("CPU = %d MHz ,AHB = %d MHz (%d:1)\n", h_pll / 1000000,
	       h_pll / div / 1000000, div);
#endif
	return;
}

void ast_scu_multi_func_eth(u8 num)
{
	switch (num) {
	case 0:
		if (ast_scu_read(AST_SCU_HW_STRAP1) & SCU_HW_STRAP_MAC0_RGMII) {
			printf("MAC0 : RGMII\n");
			ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL1) |
				      SCU_FUN_PIN_MAC0_PHY_LINK,
				      AST_SCU_FUN_PIN_CTRL1);
		} else {
			printf("MAC0 : RMII/NCSI\n");
			ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL1) &
				      ~SCU_FUN_PIN_MAC0_PHY_LINK,
				      AST_SCU_FUN_PIN_CTRL1);
		}

#ifdef AST_SOC_G5
		ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL1) |
			      SCU_FUN_PIN_MAC0_PHY_LINK, AST_SCU_FUN_PIN_CTRL1);

#endif
		ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL3) |
			      SCU_FUN_PIN_MAC0_MDIO | SCU_FUN_PIN_MAC0_MDC,
			      AST_SCU_FUN_PIN_CTRL3);

		break;
	case 1:
		if (ast_scu_read(AST_SCU_HW_STRAP1) & SCU_HW_STRAP_MAC1_RGMII) {
			printf("MAC1 : RGMII\n");
			ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL1) |
				      SCU_FUN_PIN_MAC1_PHY_LINK,
				      AST_SCU_FUN_PIN_CTRL1);
		} else {
			printf("MAC1 : RMII/NCSI\n");
			ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL1) &
				      ~SCU_FUN_PIN_MAC1_PHY_LINK,
				      AST_SCU_FUN_PIN_CTRL1);
		}

		ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL1) |
			      SCU_FUN_PIN_MAC1_PHY_LINK,
			      AST_SCU_FUN_PIN_CTRL1);

		ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL5) |
			      SCU_FUC_PIN_MAC1_MDIO,
			      AST_SCU_FUN_PIN_CTRL5);

		break;
	}
}

void ast_scu_multi_func_romcs(u8 num)
{
	ast_scu_write(ast_scu_read(AST_SCU_FUN_PIN_CTRL3) |
		      SCU_FUN_PIN_ROMCS(num), AST_SCU_FUN_PIN_CTRL3);
}

u32 ast_scu_revision_id(void)
{
	int i;
	u32 rev_id = ast_scu_read(AST_SCU_REVISION_ID);

	for (i = 0; i < ARRAY_SIZE(soc_map_table); i++) {
		if (rev_id == soc_map_table[i].rev_id)
			break;
	}

	if (i == ARRAY_SIZE(soc_map_table))
		printf("UnKnown SOC : %x\n", rev_id);
	else
		printf("SOC : %4s\n", soc_map_table[i].name);

	return rev_id;
}

void ast_scu_security_info(void)
{
	switch ((ast_scu_read(AST_SCU_HW_STRAP2) >> 18) & 0x3) {
	case 1:
		printf("SEC : DSS Mode\n");
		break;
	case 2:
		printf("SEC : UnKnown\n");
		break;
	case 3:
		printf("SEC : SPI2 Mode\n");
		break;
	}
}

void ast_scu_sys_rest_info(void)
{
	u32 rest = ast_scu_read(AST_SCU_SYS_CTRL);

	if (rest & SCU_SYS_EXT_RESET_FLAG) {
		printf("RST : External\n");
		ast_scu_write(SCU_SYS_EXT_RESET_FLAG, AST_SCU_SYS_CTRL);
	} else if (rest & SCU_SYS_WDT_RESET_FLAG) {
		printf("RST : Watchdog\n");
		ast_scu_write(SCU_SYS_WDT_RESET_FLAG, AST_SCU_SYS_CTRL);
	} else if (rest & SCU_SYS_PWR_RESET_FLAG) {
		printf("RST : Power On\n");
		ast_scu_write(SCU_SYS_PWR_RESET_FLAG, AST_SCU_SYS_CTRL);
	} else {
		printf("RST : CLK en\n");
	}
}

u32 ast_scu_get_vga_memsize(void)
{
	u32 size = 0;

	switch (SCU_HW_STRAP_VGA_SIZE_GET(ast_scu_read(AST_SCU_HW_STRAP1))) {
	case VGA_8M_DRAM:
		size = 8 * 1024 * 1024;
		break;
	case VGA_16M_DRAM:
		size = 16 * 1024 * 1024;
		break;
	case VGA_32M_DRAM:
		size = 32 * 1024 * 1024;
		break;
	case VGA_64M_DRAM:
		size = 64 * 1024 * 1024;
		break;
	default:
		printf("error vga size\n");
		break;
	}
	return size;
}

void ast_scu_get_who_init_dram(void)
{
	switch (SCU_VGA_DRAM_INIT_MASK(ast_scu_read(AST_SCU_VGA0))) {
	case 0:
		printf("DRAM :   init by VBIOS\n");
		break;
	case 1:
		printf("DRAM :   init by SOC\n");
		break;
	default:
		printf("error vga size\n");
		break;
	}
}
