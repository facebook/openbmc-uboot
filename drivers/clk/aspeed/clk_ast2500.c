// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2016 Google, Inc
 *
 * Copyright (C) ASPEED Technology Inc.
 *
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <asm/arch/scu_ast2500.h>
#include <dt-bindings/clock/ast2500-clock.h>
#include <dt-bindings/reset/ast2500-reset.h>

/*
 * MAC Clock Delay settings, taken from Aspeed SDK
 */
#define RGMII_TXCLK_ODLY		8
#define RMII_RXCLK_IDLY		2

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
struct ast2500_div_config {
	unsigned int num;
	unsigned int denum;
	unsigned int post_div;
};

extern u32 ast2500_get_clkin(struct ast2500_scu *scu)
{
	return readl(&scu->hwstrap) & SCU_HWSTRAP_CLKIN_25MHZ
			? 25 * 1000 * 1000 : 24 * 1000 * 1000;
}

/*
 * Get the rate of the M-PLL clock from input clock frequency and
 * the value of the M-PLL Parameter Register.
 */
extern u32 ast2500_get_mpll_rate(struct ast2500_scu *scu)
{
	u32 clkin = ast2500_get_clkin(scu);
	u32 mpll_reg = readl(&scu->m_pll_param);

	const ulong num = (mpll_reg & SCU_MPLL_NUM_MASK) >> SCU_MPLL_NUM_SHIFT;
	const ulong denum = (mpll_reg & SCU_MPLL_DENUM_MASK)
			>> SCU_MPLL_DENUM_SHIFT;
	const ulong post_div = (mpll_reg & SCU_MPLL_POST_MASK)
			>> SCU_MPLL_POST_SHIFT;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}

/*
 * Get the rate of the H-PLL clock from input clock frequency and
 * the value of the H-PLL Parameter Register.
 */
extern u32 ast2500_get_hpll_rate(struct ast2500_scu *scu)
{
	u32 clkin = ast2500_get_clkin(scu);
	u32 hpll_reg = readl(&scu->h_pll_param);
	
	/* F = clkin * [(M+1) / (N+1)] / (P + 1) */
	const ulong num = (hpll_reg & SCU_HPLL_NUM_MASK) >> SCU_HPLL_NUM_SHIFT;
	const ulong denum = (hpll_reg & SCU_HPLL_DENUM_MASK)
			>> SCU_HPLL_DENUM_SHIFT;
	const ulong post_div = (hpll_reg & SCU_HPLL_POST_MASK)
			>> SCU_HPLL_POST_SHIFT;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}

/*
 * Get the rate of the D-PLL clock from input clock frequency and
 * the value of the D-PLL Parameter Register.
 */
extern u32 ast2500_get_dpll_rate(struct ast2500_scu *scu)
{
	u32 clkin = ast2500_get_clkin(scu);
	u32 dpll_reg = readl(&scu->d_pll_param);

	/* F = clkin * [(M+1) / (N+1)] / (P + 1)/ (od + 1) */
	const ulong num = (dpll_reg & 0xff);
	const ulong denum = (dpll_reg >> 8) & 0x1f;
	const ulong post_div = (dpll_reg >> 13) & 0x3f;
	const ulong od_div = (dpll_reg >> 19) & 0x7;
	return (((clkin * ((num + 1) / (denum + 1))) / (post_div + 1))/ (od_div + 1));
}

/*
 * Get the rate of the D2-PLL clock from input clock frequency and
 * the value of the D2-PLL Parameter Register.
 */
extern u32 ast2500_get_d2pll_rate(struct ast2500_scu *scu)
{
	u32 clkin = ast2500_get_clkin(scu);
	u32 d2pll_reg = readl(&scu->d2_pll_param);

	/* F = clkin * [(M+1) / (N+1)] / (P + 1)/ (od + 1) */
	const ulong num = (d2pll_reg & 0xff);
	const ulong denum = (d2pll_reg >> 8) & 0x1f;
	const ulong post_div = (d2pll_reg >> 13) & 0x3f;
	const ulong od_div = (d2pll_reg >> 19) & 0x7 ;

	return (((clkin * ((num + 1) / (denum + 1))) / (post_div + 1))/ (od_div + 1));
}

#define SCU_HWSTRAP_AXIAHB_DIV_SHIFT    9
#define SCU_HWSTRAP_AXIAHB_DIV_MASK     (0x7 << SCU_HWSTRAP_AXIAHB_DIV_SHIFT)

static u32 ast2500_get_hclk(struct ast2500_scu *scu)
{
	ulong ahb_div = 1 + ((readl(&scu->hwstrap)
			      & SCU_HWSTRAP_AXIAHB_DIV_MASK)
			     >> SCU_HWSTRAP_AXIAHB_DIV_SHIFT);
	
	ulong axi_div = 2;
	u32 rate = 0;

	rate = ast2500_get_hpll_rate(scu);
	return (rate / axi_div / ahb_div);
}

static u32 ast2500_get_pclk(struct ast2500_scu *scu)
{
	u32 rate = 0;
	ulong apb_div = 4 + 4 * ((readl(&scu->clk_sel1)
				  & SCU_PCLK_DIV_MASK)
				 >> SCU_PCLK_DIV_SHIFT);
	rate = ast2500_get_hpll_rate(scu);

	return (rate / apb_div);
}

static u32 ast2500_get_sdio_clk_rate(struct ast2500_scu *scu)
{
	u32 clkin = ast2500_get_hpll_rate(scu);
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 div = (clk_sel >> 12) & 0x7;
	
	div = (div + 1) << 2;

	return (clkin / div);
}

static u32 ast2500_get_uart_clk_rate(struct ast2500_scu *scu, int uart_idx)
{
	/*
	 * ast2500 datasheet is very confusing when it comes to UART clocks,
	 * especially when CLKIN = 25 MHz. The settings are in
	 * different registers and it is unclear how they interact.
	 *
	 * This has only been tested with default settings and CLKIN = 24 MHz.
	 */
	u32 uart_clkin;

	if (readl(&scu->misc_ctrl2) &
	    (1 << (uart_idx - 1 + SCU_MISC2_UARTCLK_SHIFT)))
		uart_clkin = 192 * 1000 * 1000;
	else
		uart_clkin = 24 * 1000 * 1000;

	if (readl(&scu->misc_ctrl1) & SCU_MISC_UARTCLK_DIV13)
		uart_clkin /= 13;

	return uart_clkin;
}

static ulong ast2500_clk_get_rate(struct clk *clk)
{
	struct ast2500_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate;

	switch (clk->id) {
	case ASPEED_CLK_HPLL:
		rate = ast2500_get_hpll_rate(priv->scu);
		break;
	case ASPEED_CLK_MPLL:
		rate = ast2500_get_mpll_rate(priv->scu);
		break;
	case ASPEED_CLK_DPLL:
		rate = ast2500_get_dpll_rate(priv->scu);
		break;	
	case ASPEED_CLK_D2PLL:
		rate = ast2500_get_d2pll_rate(priv->scu);
		break;
	case ASPEED_CLK_AHB:
		rate = ast2500_get_hclk(priv->scu);
		break;
	case ASPEED_CLK_APB:
		rate = ast2500_get_pclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_UART1CLK:
		rate = ast2500_get_uart_clk_rate(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_UART2CLK:
		rate = ast2500_get_uart_clk_rate(priv->scu, 2);
		break;
	case ASPEED_CLK_GATE_UART3CLK:
		rate = ast2500_get_uart_clk_rate(priv->scu, 3);
		break;
	case ASPEED_CLK_GATE_UART4CLK:
		rate = ast2500_get_uart_clk_rate(priv->scu, 4);
		break;
	case ASPEED_CLK_GATE_UART5CLK:
		rate = ast2500_get_uart_clk_rate(priv->scu, 5);
		break;
	case ASPEED_CLK_SDIO:
		rate = ast2500_get_sdio_clk_rate(priv->scu);
		break;
	default:
		pr_debug("can't get clk rate \n");
		return -ENOENT;
		break;
	}

	return rate;
}

struct ast2500_clock_config {
	ulong input_rate;
	ulong rate;
	struct ast2500_div_config cfg;
};

static const struct ast2500_clock_config ast2500_clock_config_defaults[] = {
	{ 24000000, 250000000, { .num = 124, .denum = 1, .post_div = 5 } },
};

static bool ast2500_get_clock_config_default(ulong input_rate,
					     ulong requested_rate,
					     struct ast2500_div_config *cfg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ast2500_clock_config_defaults); i++) {
		const struct ast2500_clock_config *default_cfg =
			&ast2500_clock_config_defaults[i];
		if (default_cfg->input_rate == input_rate &&
		    default_cfg->rate == requested_rate) {
			*cfg = default_cfg->cfg;
			return true;
		}
	}

	return false;
}

/*
 * @input_rate - the rate of input clock in Hz
 * @requested_rate - desired output rate in Hz
 * @div - this is an IN/OUT parameter, at input all fields of the config
 * need to be set to their maximum allowed values.
 * The result (the best config we could find), would also be returned
 * in this structure.
 *
 * @return The clock rate, when the resulting div_config is used.
 */
static ulong ast2500_calc_clock_config(ulong input_rate, ulong requested_rate,
				       struct ast2500_div_config *cfg)
{
	/*
	 * The assumption is that kHz precision is good enough and
	 * also enough to avoid overflow when multiplying.
	 */
	const ulong input_rate_khz = input_rate / 1000;
	const ulong rate_khz = requested_rate / 1000;
	const struct ast2500_div_config max_vals = *cfg;
	struct ast2500_div_config it = { 0, 0, 0 };
	ulong delta = rate_khz;
	ulong new_rate_khz = 0;

	/*
	 * Look for a well known frequency first.
	 */
	if (ast2500_get_clock_config_default(input_rate, requested_rate, cfg))
		return requested_rate;

	for (; it.denum <= max_vals.denum; ++it.denum) {
		for (it.post_div = 0; it.post_div <= max_vals.post_div;
		     ++it.post_div) {
			it.num = (rate_khz * (it.post_div + 1) / input_rate_khz)
			    * (it.denum + 1);
			if (it.num > max_vals.num)
				continue;

			new_rate_khz = (input_rate_khz
					* ((it.num + 1) / (it.denum + 1)))
			    / (it.post_div + 1);

			/* Keep the rate below requested one. */
			if (new_rate_khz > rate_khz)
				continue;

			if (new_rate_khz - rate_khz < delta) {
				delta = new_rate_khz - rate_khz;
				*cfg = it;
				if (delta == 0)
					return new_rate_khz * 1000;
			}
		}
	}

	return new_rate_khz * 1000;
}

static ulong ast2500_configure_ddr(struct ast2500_scu *scu, ulong rate)
{
	ulong clkin = ast2500_get_clkin(scu);
	u32 mpll_reg;
	struct ast2500_div_config div_cfg = {
		.num = (SCU_MPLL_NUM_MASK >> SCU_MPLL_NUM_SHIFT),
		.denum = (SCU_MPLL_DENUM_MASK >> SCU_MPLL_DENUM_SHIFT),
		.post_div = (SCU_MPLL_POST_MASK >> SCU_MPLL_POST_SHIFT),
	};

	ast2500_calc_clock_config(clkin, rate, &div_cfg);

	mpll_reg = readl(&scu->m_pll_param);
	mpll_reg &= ~(SCU_MPLL_POST_MASK | SCU_MPLL_NUM_MASK
		      | SCU_MPLL_DENUM_MASK);
	mpll_reg |= (div_cfg.post_div << SCU_MPLL_POST_SHIFT)
	    | (div_cfg.num << SCU_MPLL_NUM_SHIFT)
	    | (div_cfg.denum << SCU_MPLL_DENUM_SHIFT);

	writel(mpll_reg, &scu->m_pll_param);

	return ast2500_get_mpll_rate(scu);
}

static ulong ast2500_configure_d2pll(struct ast2500_scu *scu, ulong rate)
{
	/*
	 * The values and the meaning of the next three
	 * parameters are undocumented. Taken from Aspeed SDK.
	 *
	 * TODO(clg@kaod.org): the SIP and SIC values depend on the
	 * Numerator value
	 */
	const u32 d2_pll_ext_param = 0x2c;
	const u32 d2_pll_sip = 0x11;
	const u32 d2_pll_sic = 0x18;
	struct ast2500_div_config div_cfg = {
		.num = SCU_D2PLL_NUM_MASK >> SCU_D2PLL_NUM_SHIFT,
		.denum = SCU_D2PLL_DENUM_MASK >> SCU_D2PLL_DENUM_SHIFT,
		.post_div = SCU_D2PLL_POST_MASK >> SCU_D2PLL_POST_SHIFT,
	};
	ulong clkin = ast2500_get_clkin(scu);
	ulong new_rate;

	writel((d2_pll_ext_param << SCU_D2PLL_EXT1_PARAM_SHIFT)
	       | SCU_D2PLL_EXT1_OFF
	       | SCU_D2PLL_EXT1_RESET, &scu->d2_pll_ext_param[0]);

	/*
	 * Select USB2.0 port1 PHY clock as a clock source for GCRT.
	 * This would disconnect it from D2-PLL.
	 */
	clrsetbits_le32(&scu->misc_ctrl1, SCU_MISC_D2PLL_OFF,
			SCU_MISC_GCRT_USB20CLK);

	new_rate = ast2500_calc_clock_config(clkin, rate, &div_cfg);
	writel((d2_pll_sip << SCU_D2PLL_SIP_SHIFT)
	       | (d2_pll_sic << SCU_D2PLL_SIC_SHIFT)
	       | (div_cfg.num << SCU_D2PLL_NUM_SHIFT)
	       | (div_cfg.denum << SCU_D2PLL_DENUM_SHIFT)
	       | (div_cfg.post_div << SCU_D2PLL_POST_SHIFT),
	       &scu->d2_pll_param);

	clrbits_le32(&scu->d2_pll_ext_param[0],
		     SCU_D2PLL_EXT1_OFF | SCU_D2PLL_EXT1_RESET);

	clrsetbits_le32(&scu->misc_ctrl2,
			SCU_MISC2_RGMII_HPLL | SCU_MISC2_RMII_MPLL
			| SCU_MISC2_RGMII_CLKDIV_MASK |
			SCU_MISC2_RMII_CLKDIV_MASK,
			(4 << SCU_MISC2_RMII_CLKDIV_SHIFT));

	return new_rate;
}

static unsigned long ast2500_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2500_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
	//mpll
	case ASPEED_CLK_MPLL:
		new_rate = ast2500_configure_ddr(priv->scu, rate);
//		printf("ast2500_clk_set_rate mpll %ld \n", new_rate);
		break;
	case ASPEED_CLK_D2PLL:
		new_rate = ast2500_configure_d2pll(priv->scu, rate);
//		printf("ast2500_clk_set_rate d2pll ==== %ld \n", new_rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

#define SCU_CLKSTOP_MAC1		(20)
#define SCU_CLKSTOP_MAC2		(21)

static ulong ast2500_configure_mac(struct ast2500_scu *scu, int index)
{
	ulong hpll_rate = ast2500_get_hpll_rate(scu);
	ulong required_rate;
	u32 hwstrap;
	u32 divisor;
	u32 reset_bit;
	u32 clkstop_bit;
	u32 clk_delay_settings =
		(RMII_RXCLK_IDLY << SCU_MICDS_MAC1RMII_RDLY_SHIFT)
		| (RMII_RXCLK_IDLY << SCU_MICDS_MAC2RMII_RDLY_SHIFT)
		| (RGMII_TXCLK_ODLY << SCU_MICDS_MAC1RGMII_TXDLY_SHIFT)
		| (RGMII_TXCLK_ODLY << SCU_MICDS_MAC2RGMII_TXDLY_SHIFT);
	
	/*
	 * According to data sheet, for 10/100 mode the MAC clock frequency
	 * should be at least 25MHz and for 1000 mode at least 100MHz
	 */
	hwstrap = readl(&scu->hwstrap);
	if (hwstrap & (SCU_HWSTRAP_MAC1_RGMII | SCU_HWSTRAP_MAC2_RGMII))
		required_rate = 100 * 1000 * 1000;
	else
		required_rate = 25 * 1000 * 1000;

	divisor = hpll_rate / required_rate;

	if (divisor < 4) {
		/* Clock can't run fast enough, but let's try anyway */
		debug("MAC clock too slow\n");
		divisor = 4;
	} else if (divisor > 16) {
		/* Can't slow down the clock enough, but let's try anyway */
		debug("MAC clock too fast\n");
		divisor = 16;
	}

	switch (index) {
	case 1:
		reset_bit = BIT(ASPEED_RESET_MAC1);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC1);
		break;
	case 2:
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC2);
		break;
	default:
		return -EINVAL;
	}

	clrsetbits_le32(&scu->clk_sel1, SCU_MACCLK_MASK,
			((divisor - 2) / 2) << SCU_MACCLK_SHIFT);

	/*
	 * Disable MAC, start its clock and re-enable it.
	 * The procedure and the delays (100us & 10ms) are
	 * specified in the datasheet.
	 */
	setbits_le32(&scu->sysreset_ctrl1, reset_bit);
	udelay(100);
	clrbits_le32(&scu->clk_stop_ctrl1, clkstop_bit);
	mdelay(10);
	clrbits_le32(&scu->sysreset_ctrl1, reset_bit);

	writel((RGMII2_TXCK_DUTY << SCU_CLKDUTY_RGMII2TXCK_SHIFT)
	       | (RGMII1_TXCK_DUTY << SCU_CLKDUTY_RGMII1TXCK_SHIFT),
	       &scu->clk_duty_sel);

	writel(clk_delay_settings | SCU_MICDS_RGMIIPLL, &scu->mac_clk_delay);
	writel(clk_delay_settings, &scu->mac_clk_delay_100M);
	writel(clk_delay_settings, &scu->mac_clk_delay_10M);

	return required_rate;
}

#define SCU_CLKSTOP_SDIO 27
static ulong ast2500_enable_sdclk(struct ast2500_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASEPPD_RESET_SDIO);
	clkstop_bit = BIT(SCU_CLKSTOP_SDIO);

	setbits_le32(&scu->sysreset_ctrl1, reset_bit);
	udelay(100);
	//enable clk 
	clrbits_le32(&scu->clk_stop_ctrl1, clkstop_bit);
	mdelay(10);
	clrbits_le32(&scu->sysreset_ctrl1, reset_bit);

	return 0;
}

#define SCU_CLKSTOP_EXTSD 15
#define SCU_CLK_SD_MASK				(0x7 << 12)
#define SCU_CLK_SD_DIV(x)			(x << 12)

static ulong ast2500_enable_extsdclk(struct ast2500_scu *scu)
{
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 enableclk_bit;

	enableclk_bit = BIT(SCU_CLKSTOP_EXTSD);

	// SDCLK = G4  H-PLL / 4, G5 = H-PLL /8
	clk_sel &= ~SCU_CLK_SD_MASK;
	clk_sel |= SCU_CLK_SD_DIV(1);
	writel(clk_sel, &scu->clk_sel1);
	
	//enable clk 
	setbits_le32(&scu->clk_sel1, enableclk_bit);
	
	return 0;
}

static int ast2500_clk_enable(struct clk *clk)
{
	struct ast2500_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	/*
	 * For MAC clocks the clock rate is
	 * configured based on whether RGMII or RMII mode has been selected
	 * through hardware strapping.
	 */
	case ASPEED_CLK_GATE_MAC1CLK:
		ast2500_configure_mac(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_MAC2CLK:
		ast2500_configure_mac(priv->scu, 2);
		break;
	case ASPEED_CLK_D2PLL:
		ast2500_configure_d2pll(priv->scu, D2PLL_DEFAULT_RATE);
		break;
	case ASPEED_CLK_GATE_SDCLK:
		ast2500_enable_sdclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_SDEXTCLK:
		ast2500_enable_extsdclk(priv->scu);
		break;
	default:
		pr_debug("can't enable clk \n");
		return -ENOENT;
		break;
	}

	return 0;
}

struct clk_ops ast2500_clk_ops = {
	.get_rate = ast2500_clk_get_rate,
	.set_rate = ast2500_clk_set_rate,
	.enable = ast2500_clk_enable,
};

static int ast2500_clk_probe(struct udevice *dev)
{
	struct ast2500_clk_priv *priv = dev_get_priv(dev);

	priv->scu = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	return 0;
}

static int ast2500_clk_bind(struct udevice *dev)
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
	{ ASPEED_CLK_DPLL, "dpll" },
	{ ASPEED_CLK_D2PLL, "d2pll" },
	{ ASPEED_CLK_AHB, "hclk" },
	{ ASPEED_CLK_APB, "pclk" },
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

static const struct udevice_id ast2500_clk_ids[] = {
	{ .compatible = "aspeed,ast2500-scu" },
	{ }
};

U_BOOT_DRIVER(aspeed_scu) = {
	.name		= "aspeed_scu",
	.id		= UCLASS_CLK,
	.of_match	= ast2500_clk_ids,
	.priv_auto_alloc_size = sizeof(struct ast2500_clk_priv),
	.ops		= &ast2500_clk_ops,
	.bind		= ast2500_clk_bind,
	.probe		= ast2500_clk_probe,
};
