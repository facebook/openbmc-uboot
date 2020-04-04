// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <asm/arch/scu_ast2600.h>
#include <dt-bindings/clock/ast2600-clock.h>
#include <dt-bindings/reset/ast2600-reset.h>

/*
 * MAC Clock Delay settings, taken from Aspeed SDK
 */
#define RGMII_TXCLK_ODLY	8
#define RMII_RXCLK_IDLY		2

#define MAC_DEF_DELAY_1G	0x0041b75d
#define MAC_DEF_DELAY_100M	0x00417410
#define MAC_DEF_DELAY_10M	0x00417410

#define MAC34_DEF_DELAY_1G	0x0010438a
#define MAC34_DEF_DELAY_100M	0x00104208
#define MAC34_DEF_DELAY_10M	0x00104208

/*
 * TGMII Clock Duty constants, taken from Aspeed SDK
 */
#define RGMII2_TXCK_DUTY	0x66
#define RGMII1_TXCK_DUTY	0x64

#define D2PLL_DEFAULT_RATE	(250 * 1000 * 1000)

DECLARE_GLOBAL_DATA_PTR;

/*
 * Clock divider/multiplier configuration struct.
 * For H-PLL and M-PLL the formula is
 * (Output Frequency) = CLKIN * ((M + 1) / (N + 1)) / (P + 1)
 * M - Numerator
 * N - Denumerator
 * P - Post Divider
 * They have the same layout in their control register.
 *
 * D-PLL and D2-PLL have extra divider (OD + 1), which is not
 * yet needed and ignored by clock configurations.
 */
union ast2600_pll_reg {
	unsigned int w;
	struct {
		unsigned int m : 13;		/* bit[12:0]	*/
		unsigned int n : 6;		/* bit[18:13]	*/
		unsigned int p : 4;		/* bit[22:19]	*/
		unsigned int off : 1;		/* bit[23]	*/
		unsigned int bypass : 1;	/* bit[24]	*/
		unsigned int reset : 1;		/* bit[25]	*/
		unsigned int reserved : 6;	/* bit[31:26]	*/
	} b;
};

struct ast2600_pll_cfg {
	union ast2600_pll_reg reg;
	unsigned int ext_reg;
};

struct ast2600_pll_desc {
	u32 in;
	u32 out;
	struct ast2600_pll_cfg cfg;
};

static const struct ast2600_pll_desc ast2600_pll_lookup[] = {
    {.in = AST2600_CLK_IN, .out = 400000000,
    .cfg.reg.b.m = 95, .cfg.reg.b.n = 2, .cfg.reg.b.p = 1,
    .cfg.ext_reg = 0x31,
    },
    {.in = AST2600_CLK_IN, .out = 200000000,
    .cfg.reg.b.m = 127, .cfg.reg.b.n = 0, .cfg.reg.b.p = 15,
    .cfg.ext_reg = 0x3f
    },
    {.in = AST2600_CLK_IN, .out = 334000000,
    .cfg.reg.b.m = 667, .cfg.reg.b.n = 4, .cfg.reg.b.p = 9,
    .cfg.ext_reg = 0x14d
    },

    {.in = AST2600_CLK_IN, .out = 1000000000,
    .cfg.reg.b.m = 119, .cfg.reg.b.n = 2, .cfg.reg.b.p = 0,
    .cfg.ext_reg = 0x3d
    },

    {.in = AST2600_CLK_IN, .out = 50000000,
    .cfg.reg.b.m = 95, .cfg.reg.b.n = 2, .cfg.reg.b.p = 15,
    .cfg.ext_reg = 0x31
    },
};

extern u32 ast2600_get_pll_rate(struct ast2600_scu *scu, int pll_idx)
{
	u32 clkin = AST2600_CLK_IN;
	u32 pll_reg = 0;
	unsigned int mult, div = 1;

	switch(pll_idx) {
		case ASPEED_CLK_HPLL:	
			pll_reg = readl(&scu->h_pll_param);
			break;
		case ASPEED_CLK_MPLL:	
			pll_reg = readl(&scu->m_pll_param);
			break;
		case ASPEED_CLK_DPLL:	
			pll_reg = readl(&scu->d_pll_param);
			break;		
		case ASPEED_CLK_EPLL:	
			pll_reg = readl(&scu->e_pll_param);
			break;

	}
	if (pll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1) */
		union ast2600_pll_reg reg;
		reg.w = pll_reg;
		mult = (reg.b.m + 1) / (reg.b.n + 1);
		div = (reg.b.p + 1);
	}
	return ((clkin * mult)/div);
	
}

extern u32 ast2600_get_apll_rate(struct ast2600_scu *scu)
{
	u32 clkin = AST2600_CLK_IN;
	u32 apll_reg = readl(&scu->a_pll_param);
	unsigned int mult, div = 1;

	if (apll_reg & BIT(20)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * (2-od) * [(m + 2) / (n + 1)] */
		u32 m = (apll_reg >> 5) & 0x3f;
		u32 od = (apll_reg >> 4) & 0x1;
		u32 n = apll_reg & 0xf;

		mult = (2 - od) * (m + 2);
		div = n + 1;
	}
	return ((clkin * mult)/div);
}

static u32 ast2600_a0_axi_ahb_div_table[] = {
	2, 2, 3, 4,
};

static u32 ast2600_a1_axi_ahb_div0_table[] = {
	3, 2, 3, 4,
};

static u32 ast2600_a1_axi_ahb_div1_table[] = {
	3, 4, 6, 8,
};

static u32 ast2600_a1_axi_ahb_default_table[] = {
	3, 4, 3, 4, 2, 2, 2, 2,
};

static u32 ast2600_get_hclk(struct ast2600_scu *scu)
{
	u32 hw_rev = readl(&scu->chip_id0);
	u32 hwstrap1 = readl(&scu->hwstrap1.hwstrap);
	u32 axi_div = 1;
	u32 ahb_div = 0;
	u32 rate = 0;

	if (hw_rev & BIT(16)) {
		if(hwstrap1 & BIT(16)) {
			ast2600_a1_axi_ahb_div1_table[0] = ast2600_a1_axi_ahb_default_table[(hwstrap1 >> 8) & 0x3];
			axi_div = 1;
			ahb_div = ast2600_a1_axi_ahb_div1_table[(hwstrap1 >> 11) & 0x3];
		} else {
			ast2600_a1_axi_ahb_div0_table[0] = ast2600_a1_axi_ahb_default_table[(hwstrap1 >> 8) & 0x3];
			axi_div = 2;
			ahb_div = ast2600_a1_axi_ahb_div0_table[(hwstrap1 >> 11) & 0x3];
		}
	} else {
		//a0 : fix axi = hpll / 2		
		axi_div = 2;
		ahb_div = ast2600_a0_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
	}

	rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	return (rate / axi_div / ahb_div);
}

static u32 ast2600_hpll_pclk1_div_table[] = {
	4, 8, 12, 16, 20, 24, 28, 32,
};

static u32 ast2600_hpll_pclk2_div_table[] = {
	2, 4, 6, 8, 10, 12, 14, 16,
};

static u32 ast2600_get_pclk1(struct ast2600_scu *scu)
{
	u32 clk_sel1 = readl(&scu->clk_sel1);
	u32 apb_div = ast2600_hpll_pclk1_div_table[((clk_sel1 >> 23) & 0x7)];
	u32 rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);

	return (rate / apb_div);
}

static u32 ast2600_get_pclk2(struct ast2600_scu *scu)
{
	u32 clk_sel4 = readl(&scu->clk_sel4);
	u32 apb_div = ast2600_hpll_pclk2_div_table[((clk_sel4 >> 9) & 0x7)];
	u32 rate = ast2600_get_hclk(scu);

	return (rate / apb_div);
}

static u32 ast2600_get_uxclk_in_rate(struct ast2600_scu *scu)
{
	u32 clk_in = 0;
	u32 uxclk_sel = readl(&scu->clk_sel5);

	uxclk_sel &= 0x3;
	switch(uxclk_sel) {
		case 0:
			clk_in = ast2600_get_apll_rate(scu) / 4;
			break;
		case 1:
			clk_in = ast2600_get_apll_rate(scu) / 2;
			break;
		case 2:
			clk_in = ast2600_get_apll_rate(scu);
			break;
		case 3:
			clk_in = ast2600_get_hclk(scu);
			break;
	}

	return clk_in;
}

static u32 ast2600_get_huxclk_in_rate(struct ast2600_scu *scu)
{
	u32 clk_in = 0;
	u32 huclk_sel = readl(&scu->clk_sel5);

	huclk_sel = ((huclk_sel >> 3) & 0x3);
	switch(huclk_sel) {
		case 0:
			clk_in = ast2600_get_apll_rate(scu) / 4;
			break;
		case 1:
			clk_in = ast2600_get_apll_rate(scu) / 2;
			break;
		case 2:
			clk_in = ast2600_get_apll_rate(scu);
			break;
		case 3:
			clk_in = ast2600_get_hclk(scu);
			break;
	}

	return clk_in;
}

static u32 ast2600_get_uart_uxclk_rate(struct ast2600_scu *scu)
{
	u32 clk_in = ast2600_get_uxclk_in_rate(scu);
	u32 div_reg = readl(&scu->uart_24m_ref_uxclk);
	unsigned int mult, div;

	u32 n = (div_reg >> 8) & 0x3ff;
	u32 r = div_reg & 0xff;

	mult = r;
	div = (n * 2);
	return (clk_in * mult)/div;
}

static u32 ast2600_get_uart_huxclk_rate(struct ast2600_scu *scu)
{
	u32 clk_in = ast2600_get_huxclk_in_rate(scu);
	u32 div_reg = readl(&scu->uart_24m_ref_huxclk);

	unsigned int mult, div;

	u32 n = (div_reg >> 8) & 0x3ff;
	u32 r = div_reg & 0xff;
	
	mult = r;
	div = (n * 2);
	return (clk_in * mult)/div;
}

static u32 ast2600_get_sdio_clk_rate(struct ast2600_scu *scu)
{
	u32 clkin = 0;
	u32 clk_sel = readl(&scu->clk_sel4);
	u32 div = (clk_sel >> 28) & 0x7;

	if(clk_sel & BIT(8)) {
		clkin = ast2600_get_apll_rate(scu);
	} else {
		clkin = ast2600_get_hclk(scu);
	}
	div = (div + 1) << 1;

	return (clkin / div);
}

static u32 ast2600_get_emmc_clk_rate(struct ast2600_scu *scu)
{
	u32 clkin = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 div = (clk_sel >> 12) & 0x7;
	
	div = (div + 1) << 2;

	return (clkin / div);
}

static u32 ast2600_get_uart_clk_rate(struct ast2600_scu *scu, int uart_idx)
{
	u32 uart_sel = readl(&scu->clk_sel4);
	u32 uart_sel5 = readl(&scu->clk_sel5);	
	ulong uart_clk = 0;

	switch(uart_idx) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
			if(uart_sel & BIT(uart_idx - 1))
				uart_clk = ast2600_get_uart_huxclk_rate(scu) ;
			else
				uart_clk = ast2600_get_uart_uxclk_rate(scu) ;
			break;
		case 5: //24mhz is come form usb phy 48Mhz
			{
			u8 uart5_clk_sel = 0;
			//high bit
			if (readl(&scu->misc_ctrl1) & BIT(12))
				uart5_clk_sel = 0x2;
			else
				uart5_clk_sel = 0x0;

			if (readl(&scu->clk_sel2) & BIT(14))
				uart5_clk_sel |= 0x1;
			
			switch(uart5_clk_sel) {
				case 0:
					uart_clk = 24000000;
					break;
				case 1:
					uart_clk = 192000000;
					break;
				case 2:
					uart_clk = 24000000/13;
					break;
				case 3:
					uart_clk = 192000000/13;
					break;
			}
			}
			break;
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:			
			if(uart_sel5 & BIT(uart_idx - 1))
				uart_clk = ast2600_get_uart_huxclk_rate(scu);
			else
				uart_clk = ast2600_get_uart_uxclk_rate(scu);
			break;
	}

	return uart_clk;
}

static ulong ast2600_clk_get_rate(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	switch (clk->id) {
	case ASPEED_CLK_HPLL:
	case ASPEED_CLK_EPLL:
	case ASPEED_CLK_DPLL:
	case ASPEED_CLK_MPLL:
		rate = ast2600_get_pll_rate(priv->scu, clk->id);
		break;
	case ASPEED_CLK_AHB:
		rate = ast2600_get_hclk(priv->scu);
		break;
	case ASPEED_CLK_APB1:
		rate = ast2600_get_pclk1(priv->scu);
		break;
	case ASPEED_CLK_APB2:
		rate = ast2600_get_pclk2(priv->scu);
		break;	
	case ASPEED_CLK_APLL:
		rate = ast2600_get_apll_rate(priv->scu);
		break;
	case ASPEED_CLK_GATE_UART1CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_UART2CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 2);
		break;
	case ASPEED_CLK_GATE_UART3CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 3);
		break;
	case ASPEED_CLK_GATE_UART4CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 4);
		break;
	case ASPEED_CLK_GATE_UART5CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 5);
		break;
	case ASPEED_CLK_SDIO:
		rate = ast2600_get_sdio_clk_rate(priv->scu);
		break;
	case ASPEED_CLK_EMMC:
		rate = ast2600_get_emmc_clk_rate(priv->scu);
		break;
	case ASPEED_CLK_UARTX:
		rate = ast2600_get_uart_uxclk_rate(priv->scu);
		break;
	case ASPEED_CLK_HUARTX:
		rate = ast2600_get_uart_huxclk_rate(priv->scu);
		break;
	default:
		pr_debug("can't get clk rate \n");
		return -ENOENT;
		break;
	}

	return rate;
}

/**
 * @brief	lookup PLL divider config by input/output rate
 * @param[in]	*pll - PLL descriptor
 * @return	true - if PLL divider config is found, false - else
 * 
 * The function caller shall fill "pll->in" and "pll->out", then this function
 * will search the lookup table to find a valid PLL divider configuration.
 */
static bool ast2600_search_clock_config(struct ast2600_pll_desc *pll)
{
	u32 i;
	bool is_found = false;

	for (i = 0; i < ARRAY_SIZE(ast2600_pll_lookup); i++) {
		const struct ast2600_pll_desc *def_cfg = &ast2600_pll_lookup[i];
		if ((def_cfg->in == pll->in) && (def_cfg->out == pll->out)) {
			is_found = true;
			pll->cfg.reg.w = def_cfg->cfg.reg.w;
			pll->cfg.ext_reg = def_cfg->cfg.ext_reg;
			break;
		}
	}
	return is_found;
}
static u32 ast2600_configure_pll(struct ast2600_scu *scu,
				 struct ast2600_pll_cfg *p_cfg, int pll_idx)
{
	u32 addr, addr_ext;
	u32 reg;

	switch (pll_idx) {
	case ASPEED_CLK_HPLL:
		addr = (u32)(&scu->h_pll_param);
		addr_ext = (u32)(&scu->h_pll_ext_param);
		break;
	case ASPEED_CLK_MPLL:
		addr = (u32)(&scu->m_pll_param);
		addr_ext = (u32)(&scu->m_pll_ext_param);
		break;
	case ASPEED_CLK_DPLL:
		addr = (u32)(&scu->d_pll_param);
		addr_ext = (u32)(&scu->d_pll_ext_param);
		break;
	case ASPEED_CLK_EPLL:
		addr = (u32)(&scu->e_pll_param);
		addr_ext = (u32)(&scu->e_pll_ext_param);
		break;
	default:
		debug("unknown PLL index\n");
		return 1;
	}

	p_cfg->reg.b.bypass = 0;
	p_cfg->reg.b.off = 1;
	p_cfg->reg.b.reset = 1;

	reg = readl(addr);
	reg &= ~GENMASK(25, 0);
	reg |= p_cfg->reg.w;
	writel(reg, addr);
	
	/* write extend parameter */
	writel(p_cfg->ext_reg, addr_ext);
	udelay(100);
	p_cfg->reg.b.off = 0;
	p_cfg->reg.b.reset = 0;
	reg &= ~GENMASK(25, 0);
	reg |= p_cfg->reg.w;
	writel(reg, addr);

	/* polling PLL lock status */
	while(0 == (readl(addr_ext) & BIT(31)));

	return 0;
}
static u32 ast2600_configure_ddr(struct ast2600_scu *scu, ulong rate)
{
	struct ast2600_pll_desc mpll;

	mpll.in = AST2600_CLK_IN;
	mpll.out = rate;
	if (false == ast2600_search_clock_config(&mpll)) {
		printf("error!! unable to find valid DDR clock setting\n");
		return 0;
	}
	ast2600_configure_pll(scu, &(mpll.cfg), ASPEED_CLK_MPLL);

	return ast2600_get_pll_rate(scu, ASPEED_CLK_MPLL);
}

static ulong ast2600_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
	case ASPEED_CLK_MPLL:
		new_rate = ast2600_configure_ddr(priv->scu, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

#define SCU_CLKSTOP_MAC1		(20)
#define SCU_CLKSTOP_MAC2		(21)
#define SCU_CLKSTOP_MAC3		(20)
#define SCU_CLKSTOP_MAC4		(21)

static u32 ast2600_configure_mac12_clk(struct ast2600_scu *scu)
{
	/* scu340[25:0]: 1G default delay */
	clrsetbits_le32(&scu->mac12_clk_delay, GENMASK(25, 0),
			MAC_DEF_DELAY_1G);

	/* set 100M/10M default delay */
	writel(MAC_DEF_DELAY_100M, &scu->mac12_clk_delay_100M);
	writel(MAC_DEF_DELAY_10M, &scu->mac12_clk_delay_10M);

	/* MAC AHB = HPLL / 6 */
	clrsetbits_le32(&scu->clk_sel1, GENMASK(18, 16), (0x2 << 16));

	return 0;
}

static u32 ast2600_configure_mac34_clk(struct ast2600_scu *scu)
{

	/*
	 * scu350[31]   RGMII 125M source: 0 = from IO pin
	 * scu350[25:0] MAC 1G delay
	 */
	clrsetbits_le32(&scu->mac34_clk_delay, (BIT(31) | GENMASK(25, 0)),
			MAC34_DEF_DELAY_1G);
	writel(MAC34_DEF_DELAY_100M, &scu->mac34_clk_delay_100M);
	writel(MAC34_DEF_DELAY_10M, &scu->mac34_clk_delay_10M);

	/*
	 * clock source seletion and divider
	 * scu310[26:24] : MAC AHB bus clock = HCLK / 2
	 * scu310[18:16] : RMII 50M = HCLK_200M / 4
	 */
	clrsetbits_le32(&scu->clk_sel4, (GENMASK(26, 24) | GENMASK(18, 16)),
			((0x0 << 24) | (0x3 << 16)));

	/*
	 * set driving strength
	 * scu458[3:2] : MAC4 driving strength
	 * scu458[1:0] : MAC3 driving strength
	 */
	clrsetbits_le32(&scu->pinmux_ctrl16, GENMASK(3, 0),
			(0x3 << 2) | (0x3 << 0));	

	return 0;
}

/**
 * ast2600 RGMII clock source tree
 * 
 *    125M from external PAD -------->|\
 *    HPLL -->|\                      | |---->RGMII 125M for MAC#1 & MAC#2
 *            | |---->| divider |---->|/                             +
 *    EPLL -->|/                                                     |
 *                                                                   |
 *    +---------<-----------|RGMIICK PAD output enable|<-------------+
 *    |
 *    +--------------------------->|\
 *                                 | |----> RGMII 125M for MAC#3 & MAC#4
 *    HCLK 200M ---->|divider|---->|/
 * 
 * To simplify the control flow:
 * 	1. RGMII 1/2 always use EPLL as the internal clock source
 * 	2. RGMII 3/4 always use RGMIICK pad as the RGMII 125M source
 * 
 *    125M from external PAD -------->|\
 *                                    | |---->RGMII 125M for MAC#1 & MAC#2
 *            EPLL---->| divider |--->|/                             + 
 *                                                                   |
 *    +<--------------------|RGMIICK PAD output enable|<-------------+
 *    |
 *    +--------------------------->RGMII 125M for MAC#3 & MAC#4
*/
#define RGMIICK_SRC_PAD			0
#define RGMIICK_SRC_EPLL		1	/* recommended */
#define RGMIICK_SRC_HPLL		2

#define RGMIICK_DIV2			1
#define RGMIICK_DIV3			2
#define RGMIICK_DIV4			3
#define RGMIICK_DIV5			4
#define RGMIICK_DIV6			5
#define RGMIICK_DIV7			6
#define RGMIICK_DIV8			7	/* recommended */

#define RMIICK_DIV4			0
#define RMIICK_DIV8			1
#define RMIICK_DIV12			2
#define RMIICK_DIV16			3
#define RMIICK_DIV20			4	/* recommended */
#define RMIICK_DIV24			5
#define RMIICK_DIV28			6
#define RMIICK_DIV32			7

struct ast2600_mac_clk_div {
	u32 src;	/* 0=external PAD, 1=internal PLL */
	u32 fin;	/* divider input speed */
	u32 n;		/* 0=div2, 1=div2, 2=div3, 3=div4,...,7=div8 */
	u32 fout;	/* fout = fin / n */
};

struct ast2600_mac_clk_div rgmii_clk_defconfig = {
	.src = ASPEED_CLK_EPLL,
	.fin = 1000000000,
	.n = RGMIICK_DIV8,
	.fout = 125000000,
};

struct ast2600_mac_clk_div rmii_clk_defconfig = {
	.src = ASPEED_CLK_EPLL,
	.fin = 1000000000,
	.n = RMIICK_DIV20,
	.fout = 50000000,
};
static void ast2600_init_mac_pll(struct ast2600_scu *p_scu,
				 struct ast2600_mac_clk_div *p_cfg) 
{
	struct ast2600_pll_desc pll;

	pll.in = AST2600_CLK_IN;
	pll.out = p_cfg->fin;
	if (false == ast2600_search_clock_config(&pll)) {
		printf("error!! unable to find valid ETHNET MAC clock "
		       "setting\n");
		debug("%s: pll cfg = 0x%08x 0x%08x\n", __func__, pll.cfg.reg.w,
		      pll.cfg.ext_reg);
		debug("%s: pll cfg = %02x %02x %02x\n", __func__,
		      pll.cfg.reg.b.m, pll.cfg.reg.b.n, pll.cfg.reg.b.p);
		return;
	}
	ast2600_configure_pll(p_scu, &(pll.cfg), p_cfg->src);
}

static void ast2600_init_rgmii_clk(struct ast2600_scu *p_scu,
				   struct ast2600_mac_clk_div *p_cfg) 
{
	u32 reg_304 = readl(&p_scu->clk_sel2);
	u32 reg_340 = readl(&p_scu->mac12_clk_delay);
	u32 reg_350 = readl(&p_scu->mac34_clk_delay);

	reg_340 &= ~GENMASK(31, 29);
	/* scu340[28]: RGMIICK PAD output enable (to MAC 3/4) */
	reg_340 |= BIT(28);
	if ((p_cfg->src == ASPEED_CLK_EPLL) ||
	    (p_cfg->src == ASPEED_CLK_HPLL)) {
		/*
		 * re-init PLL if the current PLL output frequency doesn't match
		 * the divider setting
		 */
		if (p_cfg->fin != ast2600_get_pll_rate(p_scu, p_cfg->src)) {
			ast2600_init_mac_pll(p_scu, p_cfg);
		}
		/* scu340[31]: select RGMII 125M from internal source */
		reg_340 |= BIT(31);
	}

	reg_304 &= ~GENMASK(23, 20);
	
	/* set clock divider */
	reg_304 |= (p_cfg->n & 0x7) << 20;
	
	/* select internal clock source */
	if (ASPEED_CLK_HPLL == p_cfg->src) {
		reg_304 |= BIT(23);
	}

	/* RGMII 3/4 clock source select */
	reg_350 &= ~BIT(31);
#if 0
	if (RGMII_3_4_CLK_SRC_HCLK == p_cfg->rgmii_3_4_clk_src) {
		reg_350 |= BIT(31);
	}

	/* set clock divider */
	reg_310 &= ~GENMASK(22, 20);
	reg_310 |= (p_cfg->hclk_clk_div & 0x7) << 20;
#endif

	writel(reg_304, &p_scu->clk_sel2);
	writel(reg_340, &p_scu->mac12_clk_delay);
	writel(reg_350, &p_scu->mac34_clk_delay);
}

/**
 * ast2600 RMII/NCSI clock source tree
 * 
 *    HPLL -->|\                      
 *            | |---->| divider |----> RMII 50M for MAC#1 & MAC#2
 *    EPLL -->|/ 
 * 
 *    HCLK(SCLICLK)---->| divider |----> RMII 50M for MAC#3 & MAC#4
*/
static void ast2600_init_rmii_clk(struct ast2600_scu *p_scu,
				  struct ast2600_mac_clk_div *p_cfg) 
{
	u32 reg_304;
	u32 reg_310;

	if ((p_cfg->src == ASPEED_CLK_EPLL) ||
	    (p_cfg->src == ASPEED_CLK_HPLL)) {
		/*
		 * re-init PLL if the current PLL output frequency doesn't match
		 * the divider setting
		 */
		if (p_cfg->fin != ast2600_get_pll_rate(p_scu, p_cfg->src)) {
			ast2600_init_mac_pll(p_scu, p_cfg);
		}
	}

	reg_304 = readl(&p_scu->clk_sel2);
	reg_310 = readl(&p_scu->clk_sel4);

	reg_304 &= ~GENMASK(19, 16);

	/* set RMII 1/2 clock divider */
	reg_304 |= (p_cfg->n & 0x7) << 16;

	/* RMII clock source selection */
	if (ASPEED_CLK_HPLL == p_cfg->src) {
		reg_304 |= BIT(19);
	}	

	/* set RMII 3/4 clock divider */
	reg_310 &= ~GENMASK(18, 16);
	reg_310 |= (0x3 << 16);

	writel(reg_304, &p_scu->clk_sel2);
	writel(reg_310, &p_scu->clk_sel4);
}

static u32 ast2600_configure_mac(struct ast2600_scu *scu, int index)
{
	u32 reset_bit;
	u32 clkstop_bit;

	switch (index) {
	case 1:
		reset_bit = BIT(ASPEED_RESET_MAC1);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC1);
		writel(reset_bit, &scu->sysreset_ctrl1);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl1);

		break;
	case 2:
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC2);
		writel(reset_bit, &scu->sysreset_ctrl1);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl1);
		break;
	case 3:
		reset_bit = BIT(ASPEED_RESET_MAC3 - 32);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC3);
		writel(reset_bit, &scu->sysreset_ctrl2);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl2);
		break;
	case 4:
		reset_bit = BIT(ASPEED_RESET_MAC4 - 32);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC4);
		writel(reset_bit, &scu->sysreset_ctrl2);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl2);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#define SCU_CLKSTOP_SDIO 4
static ulong ast2600_enable_sdclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_SD - 32);
	clkstop_bit = BIT(SCU_CLKSTOP_SDIO);

	writel(reset_bit, &scu->sysreset_ctrl2);

	udelay(100);
	//enable clk 
	writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
	mdelay(10);
	writel(reset_bit, &scu->sysreset_clr_ctrl2);

	return 0;
}

#define SCU_CLKSTOP_EXTSD 31
#define SCU_CLK_SD_MASK				(0x7 << 28)
#define SCU_CLK_SD_DIV(x)			(x << 28)

static ulong ast2600_enable_extsdclk(struct ast2600_scu *scu)
{
	u32 clk_sel = readl(&scu->clk_sel4);
	u32 enableclk_bit;

	enableclk_bit = BIT(SCU_CLKSTOP_EXTSD);

	//default use apll for clock source 800/4 = 200 : controller max is 200mhz
	clk_sel &= ~SCU_CLK_SD_MASK;
	clk_sel |= SCU_CLK_SD_DIV(1) | BIT(8);
	writel(clk_sel, &scu->clk_sel4);
	
	//enable clk 
	setbits_le32(&scu->clk_sel4, enableclk_bit);
	
	return 0;
}

#define SCU_CLKSTOP_EMMC 27
static ulong ast2600_enable_emmcclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_EMMC);
	clkstop_bit = BIT(SCU_CLKSTOP_EMMC);

	writel(reset_bit, &scu->sysreset_ctrl1);
	udelay(100);
	//enable clk 
	writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
	mdelay(10);
	writel(reset_bit, &scu->sysreset_clr_ctrl1);

	return 0;
}

#define SCU_CLKSTOP_EXTEMMC 15
#define SCU_CLK_EMMC_MASK			(0x7 << 12)
#define SCU_CLK_EMMC_DIV(x)			(x << 12)

static ulong ast2600_enable_extemmcclk(struct ast2600_scu *scu)
{
	u32 revision_id = readl(&scu->chip_id0);
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 enableclk_bit;
	u32 rate = 0;
	u32 div = 0;
	int i = 0;

	enableclk_bit = BIT(SCU_CLKSTOP_EXTEMMC);

	if(((revision_id & GENMASK(23, 16)) >> 16) == 0x1) {
		//use mpll to be clk source
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_MPLL);
		for(i = 0; i < 8; i++) {
			div = (i + 1) * 2;
			if ((rate / div) <= 200000000)
				break;
		}
		
		clk_sel &= ~SCU_CLK_EMMC_MASK;
		clk_sel |= SCU_CLK_EMMC_DIV(i) | BIT(11);
		writel(clk_sel, &scu->clk_sel1);	

	} else {
	 	//use hpll to be clk source

		//ast2600 eMMC controller max clk is 200Mhz
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);

		for(i = 0; i < 8; i++) {
			div = (i + 1) * 4;
			if ((rate / div) <= 200000000)
				break;
		}

		clk_sel &= ~SCU_CLK_EMMC_MASK;
		clk_sel |= SCU_CLK_EMMC_DIV(i);
		writel(clk_sel, &scu->clk_sel1);
	}

	//enable clk 
	setbits_le32(&scu->clk_sel1, enableclk_bit);

	return 0;
}

#define SCU_CLKSTOP_FSICLK 30

static ulong ast2600_enable_fsiclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_FSI % 32);
	clkstop_bit = BIT(SCU_CLKSTOP_FSICLK);

	/* The FSI clock is shared between masters. If it's already on
	 * don't touch it, as that will reset the existing master. */
	if (!(readl(&scu->clk_stop_ctrl2) & clkstop_bit)) {
		debug("%s: already running, not touching it\n", __func__);
		return 0;
	}

	writel(reset_bit, &scu->sysreset_ctrl2);
	udelay(100);
	//enable clk
	writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
	mdelay(10);
	writel(reset_bit, &scu->sysreset_clr_ctrl2);

	return 0;
}

static ulong ast2600_enable_usbahclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_EHCI_P1);
	clkstop_bit = BIT(14);

	writel(reset_bit, &scu->sysreset_ctrl1);
		udelay(100);
	//enable phy clk
	writel(clkstop_bit, &scu->clk_stop_ctrl1);
	mdelay(20);
	writel(reset_bit, &scu->sysreset_clr_ctrl1);

	return 0;
}

static ulong ast2600_enable_usbbhclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_EHCI_P2);
	clkstop_bit = BIT(7);

	writel(reset_bit, &scu->sysreset_ctrl1);
			udelay(100);
	//enable phy clk
	writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
	mdelay(20);

	writel(reset_bit, &scu->sysreset_clr_ctrl1);

	return 0;
}

static int ast2600_clk_enable(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
		case ASPEED_CLK_GATE_MAC1CLK:
			ast2600_configure_mac(priv->scu, 1);
			break;
		case ASPEED_CLK_GATE_MAC2CLK:
			ast2600_configure_mac(priv->scu, 2);
			break;
		case ASPEED_CLK_GATE_MAC3CLK:
			ast2600_configure_mac(priv->scu, 3);
			break;
		case ASPEED_CLK_GATE_MAC4CLK:
			ast2600_configure_mac(priv->scu, 4);
			break;
		case ASPEED_CLK_GATE_SDCLK:
			ast2600_enable_sdclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_SDEXTCLK:
			ast2600_enable_extsdclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_EMMCCLK:
			ast2600_enable_emmcclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_EMMCEXTCLK:
			ast2600_enable_extemmcclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_FSICLK:
			ast2600_enable_fsiclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_USBPORT1CLK:
			ast2600_enable_usbahclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_USBPORT2CLK:
			ast2600_enable_usbbhclk(priv->scu);
			break;
		default:
			pr_debug("can't enable clk \n");
			return -ENOENT;
			break;
	}

	return 0;
}

struct clk_ops ast2600_clk_ops = {
	.get_rate = ast2600_clk_get_rate,
	.set_rate = ast2600_clk_set_rate,
	.enable = ast2600_clk_enable,
};

static int ast2600_clk_probe(struct udevice *dev)
{
	struct ast2600_clk_priv *priv = dev_get_priv(dev);
	u32 uart_clk_source;

	priv->scu = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	uart_clk_source = dev_read_u32_default(dev, "uart-clk-source",
					    0x0);

	if(uart_clk_source) {
		if(uart_clk_source & GENMASK(5, 0))
			setbits_le32(&priv->scu->clk_sel4, uart_clk_source & GENMASK(5, 0));
		if(uart_clk_source & GENMASK(12, 6))
			setbits_le32(&priv->scu->clk_sel5, uart_clk_source & GENMASK(12, 6));
	}

	
	ast2600_init_rgmii_clk(priv->scu, &rgmii_clk_defconfig);	
	ast2600_init_rmii_clk(priv->scu, &rmii_clk_defconfig);
	ast2600_configure_mac12_clk(priv->scu);
	ast2600_configure_mac34_clk(priv->scu);

	return 0;
}

static int ast2600_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "ast_sysreset", "reset", &dev);
	if (ret)
		debug("Warning: No reset driver: ret=%d\n", ret);

	return 0;
}

#if CONFIG_IS_ENABLED(CMD_CLK)
struct aspeed_clks {
	ulong id;
	const char *name;
};

static struct aspeed_clks aspeed_clk_names[] = {
	{ ASPEED_CLK_HPLL, "hpll" },
	{ ASPEED_CLK_MPLL, "mpll" },
	{ ASPEED_CLK_APLL, "apll" },
	{ ASPEED_CLK_EPLL, "epll" },
	{ ASPEED_CLK_DPLL, "dpll" },
	{ ASPEED_CLK_AHB, "hclk" },
	{ ASPEED_CLK_APB1, "pclk1" },
	{ ASPEED_CLK_APB2, "pclk2" },
	{ ASPEED_CLK_UARTX, "uxclk" },
	{ ASPEED_CLK_HUARTX, "huxclk" },
};

int soc_clk_dump(void)
{
	struct udevice *dev;
	struct clk clk;
	unsigned long rate;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_GET_DRIVER(aspeed_scu), &dev);
	if (ret)
		return ret;

	printf("Clk\t\tHz\n");

	for (i = 0; i < ARRAY_SIZE(aspeed_clk_names); i++) {
		clk.id = aspeed_clk_names[i].id;
		ret = clk_request(dev, &clk);
		if (ret < 0) {
			debug("%s clk_request() failed: %d\n", __func__, ret);
			continue;
		}

		ret = clk_get_rate(&clk);
		rate = ret;

		clk_free(&clk);

		if (ret == -ENOTSUPP) {
			printf("clk ID %lu not supported yet\n",
			       aspeed_clk_names[i].id);
			continue;
		}
		if (ret < 0) {
			printf("%s %lu: get_rate err: %d\n",
			       __func__, aspeed_clk_names[i].id, ret);
			continue;
		}

		printf("%s(%3lu):\t%lu\n",
		       aspeed_clk_names[i].name, aspeed_clk_names[i].id, rate);
	}

	return 0;
}
#endif

static const struct udevice_id ast2600_clk_ids[] = {
	{ .compatible = "aspeed,ast2600-scu", },
	{ }
};

U_BOOT_DRIVER(aspeed_scu) = {
	.name		= "aspeed_scu",
	.id		= UCLASS_CLK,
	.of_match	= ast2600_clk_ids,
	.priv_auto_alloc_size = sizeof(struct ast2600_clk_priv),
	.ops		= &ast2600_clk_ops,
	.bind		= ast2600_clk_bind,
	.probe		= ast2600_clk_probe,
};
