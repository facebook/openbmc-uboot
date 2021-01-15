// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <asm/arch/scu_ast2400.h>
#include <dt-bindings/clock/ast2400-clock.h>
#include <dt-bindings/reset/ast2400-reset.h>

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
struct ast2400_div_config {
	unsigned int num;
	unsigned int denum;
	unsigned int post_div;
};

#define CLKIN_25MHZ_EN BIT(23)
#define AST2400_CLK_SOURCE_SEL BIT(18)
extern u32 ast2400_get_clkin(struct ast2400_scu *scu)
{
	u32 clkin;
	u32 strap = readl(&scu->hwstrap);

	if (strap & CLKIN_25MHZ_EN) {
		clkin = 25 * 1000 * 1000;
	} else {
		if (strap & AST2400_CLK_SOURCE_SEL)
			clkin = 48 * 1000 * 1000;
		else
			clkin = 24 * 1000 * 1000;
	}

	return clkin;
}

#define AST2400_MPLL_BYPASS_EN BIT(17)
#define AST2400_MPLL_OFF BIT(16)

/*
 * Get the rate of the M-PLL clock from input clock frequency and
 * the value of the M-PLL Parameter Register.
 */
extern u32 ast2400_get_mpll_rate(struct ast2400_scu *scu)
{
	unsigned int mult, div;
	u32 clkin = ast2400_get_clkin(scu);
	u32 mpll_reg = readl(&scu->m_pll_param);

	if (mpll_reg & AST2400_MPLL_OFF)
		return 0;

	if (mpll_reg & AST2400_MPLL_BYPASS_EN)
		return clkin;
	else {
		u32 od = (mpll_reg >> 4) & 0x1;
		u32 n = (mpll_reg >> 5) & 0x3f;
		u32 d = mpll_reg & 0xf;

		//mpll = 24MHz * (2-OD) * ((Numerator+2)/(Denumerator+1))
		mult = (2 - od) * (n + 2);
		div = (d + 1);
	}
	
	return (clkin * mult / div);
}

#define AST2400_HPLL_PROGRAMMED BIT(18)
#define AST2400_HPLL_BYPASS_EN BIT(17)
/*
 * Get the rate of the H-PLL clock from input clock frequency and
 * the value of the H-PLL Parameter Register.
 */
extern u32 ast2400_get_hpll_rate(struct ast2400_scu *scu)
{
	unsigned int mult, div;
	u32 clkin = ast2400_get_clkin(scu);
	u32 hpll_reg = readl(&scu->h_pll_param);
	
	const u16 hpll_rates[][4] = {
	    {384, 360, 336, 408},
	    {400, 375, 350, 425},
	};

	if (hpll_reg & AST2400_HPLL_PROGRAMMED) {
		if (hpll_reg & AST2400_HPLL_BYPASS_EN) {
			/* Pass through mode */
			mult = div = 1;
		} else {
			/* F = 24Mhz * (2-OD) * [(N + 2) / (D + 1)] */
			u32 n = (hpll_reg >> 5) & 0x3f;
			u32 od = (hpll_reg >> 4) & 0x1;
			u32 d = hpll_reg & 0xf;

			mult = (2 - od) * (n + 2);
			div = d + 1;
		}
	} else {
		//fix
		u32 strap = readl(ASPEED_HW_STRAP1);
		u16 rate = (hpll_reg >> 8) & 3;
		if (strap & CLKIN_25MHZ_EN)
			clkin = hpll_rates[1][rate];
		else {
			if (strap & AST2400_CLK_SOURCE_SEL)
				clkin = hpll_rates[0][rate];
			else
				clkin = hpll_rates[0][rate];
		}
		clkin *= 1000000;
		mult = 1;
		div = 1;
	}

	return (clkin * mult / div);
}

#define AST2400_D2PLL_OFF BIT(17)
#define AST2400_D2PLL_BYPASS_EN BIT(18)

/*
 * Get the rate of the D2-PLL clock from input clock frequency and
 * the value of the D2-PLL Parameter Register.
 */
extern u32 ast2400_get_d2pll_rate(struct ast2400_scu *scu)
{
	unsigned int mult, div;
	u32 clkin = ast2400_get_clkin(scu);
	u32 d2pll_reg = readl(&scu->d2_pll_param);

	/* F = clkin * [(M+1) / (N+1)] / (P + 1)/ (od + 1) */
	if (d2pll_reg & AST2400_D2PLL_OFF)
		return 0;

	// Programming
	if (d2pll_reg & AST2400_D2PLL_BYPASS_EN)
		return clkin;
	else {
		u32 n = (d2pll_reg & 0xff);
		u32 d = (d2pll_reg >> 8) & 0x1f;
		u32 o = (d2pll_reg >> 13) & 0x3;
		o = (1 << (o - 1));
		u32 p = (d2pll_reg >> 15) & 0x3;
		if (p == 2)
			p = 2;
		else
			p = (0x1 << p);
		u32 p2 = (d2pll_reg >> 19) & 0x7;
		p2 += 1;
		//FOUT (Output frequency) = 24MHz * (Num * 2) / (Denum * OD * PD * PD2)
		mult = (n * 2);
		div = (d * o * p * p2);
	}
	return (clkin * mult / div);
}

#define SCU_HWSTRAP_AXIAHB_DIV_SHIFT    9
#define SCU_HWSTRAP_AXIAHB_DIV_MASK     (0x7 << SCU_HWSTRAP_AXIAHB_DIV_SHIFT)

static u32 ast2400_get_hclk(struct ast2400_scu *scu)
{
	u32 ahb_div;
	u32 strap = readl(&scu->hwstrap);
	u32 rate = ast2400_get_hpll_rate(scu);

	ahb_div = ((strap >> 10) & 0x3) + 1;

	return (rate / ahb_div);
}

static u32 ast2400_get_pclk(struct ast2400_scu *scu)
{
	u32 rate = 0;
	rate = ast2400_get_hpll_rate(scu);
	u32 apb_div = (readl(&scu->clk_sel1) >> 23) & 0x7;

	apb_div = (apb_div + 1) << 1;

	return (rate / apb_div);
}

static u32 ast2400_get_sdio_clk_rate(struct ast2400_scu *scu)
{
	u32 clkin = ast2400_get_hpll_rate(scu);
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 div = (clk_sel >> 12) & 0x7;
	
	div = (div + 1) << 1;

	return (clkin / div);
}

static u32 ast2400_get_uart_clk_rate(struct ast2400_scu *scu, int uart_idx)
{
	u32	uart_clkin = 24 * 1000 * 1000;

	if (readl(&scu->misc_ctrl1) & SCU_MISC_UARTCLK_DIV13)
		uart_clkin /= 13;

	return uart_clkin;
}

static ulong ast2400_clk_get_rate(struct clk *clk)
{
	struct ast2400_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate;

	switch (clk->id) {
	case ASPEED_CLK_HPLL:
		rate = ast2400_get_hpll_rate(priv->scu);
		break;
	case ASPEED_CLK_MPLL:
		rate = ast2400_get_mpll_rate(priv->scu);
		break;
	case ASPEED_CLK_D2PLL:
		rate = ast2400_get_d2pll_rate(priv->scu);
		break;
	case ASPEED_CLK_AHB:
		rate = ast2400_get_hclk(priv->scu);
		break;
	case ASPEED_CLK_APB:
		rate = ast2400_get_pclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_UART1CLK:
		rate = ast2400_get_uart_clk_rate(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_UART2CLK:
		rate = ast2400_get_uart_clk_rate(priv->scu, 2);
		break;
	case ASPEED_CLK_GATE_UART3CLK:
		rate = ast2400_get_uart_clk_rate(priv->scu, 3);
		break;
	case ASPEED_CLK_GATE_UART4CLK:
		rate = ast2400_get_uart_clk_rate(priv->scu, 4);
		break;
	case ASPEED_CLK_GATE_UART5CLK:
		rate = ast2400_get_uart_clk_rate(priv->scu, 5);
		break;
	case ASPEED_CLK_SDIO:
		rate = ast2400_get_sdio_clk_rate(priv->scu);
		break;
	default:
		pr_debug("can't get clk rate \n");
		return -ENOENT;
		break;
	}

	return rate;
}

struct ast2400_clock_config {
	ulong input_rate;
	ulong rate;
	struct ast2400_div_config cfg;
};

static const struct ast2400_clock_config ast2400_clock_config_defaults[] = {
	{ 24000000, 250000000, { .num = 124, .denum = 1, .post_div = 5 } },
};

static bool ast2400_get_clock_config_default(ulong input_rate,
					     ulong requested_rate,
					     struct ast2400_div_config *cfg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ast2400_clock_config_defaults); i++) {
		const struct ast2400_clock_config *default_cfg =
			&ast2400_clock_config_defaults[i];
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
static ulong ast2400_calc_clock_config(ulong input_rate, ulong requested_rate,
				       struct ast2400_div_config *cfg)
{
	/*
	 * The assumption is that kHz precision is good enough and
	 * also enough to avoid overflow when multiplying.
	 */
	const ulong input_rate_khz = input_rate / 1000;
	const ulong rate_khz = requested_rate / 1000;
	const struct ast2400_div_config max_vals = *cfg;
	struct ast2400_div_config it = { 0, 0, 0 };
	ulong delta = rate_khz;
	ulong new_rate_khz = 0;

	/*
	 * Look for a well known frequency first.
	 */
	if (ast2400_get_clock_config_default(input_rate, requested_rate, cfg))
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

static ulong ast2400_configure_ddr(struct ast2400_scu *scu, ulong rate)
{
	ulong clkin = ast2400_get_clkin(scu);
	u32 mpll_reg;
	struct ast2400_div_config div_cfg = {
		.num = (SCU_MPLL_NUM_MASK >> SCU_MPLL_NUM_SHIFT),
		.denum = (SCU_MPLL_DENUM_MASK >> SCU_MPLL_DENUM_SHIFT),
		.post_div = (SCU_MPLL_POST_MASK >> SCU_MPLL_POST_SHIFT),
	};

	ast2400_calc_clock_config(clkin, rate, &div_cfg);

	mpll_reg = readl(&scu->m_pll_param);
	mpll_reg &= ~(SCU_MPLL_POST_MASK | SCU_MPLL_NUM_MASK
		      | SCU_MPLL_DENUM_MASK);
	mpll_reg |= (div_cfg.post_div << SCU_MPLL_POST_SHIFT)
	    | (div_cfg.num << SCU_MPLL_NUM_SHIFT)
	    | (div_cfg.denum << SCU_MPLL_DENUM_SHIFT);

	writel(mpll_reg, &scu->m_pll_param);

	return ast2400_get_mpll_rate(scu);
}

static unsigned long ast2400_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2400_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
		//mpll
		case ASPEED_CLK_MPLL:
			new_rate = ast2400_configure_ddr(priv->scu, rate);
			break;
		default:
			return -ENOENT;
	}

	return new_rate;
}

#define SCU_CLKSTOP_MAC1		(20)
#define SCU_CLKSTOP_MAC2		(21)

static ulong ast2400_configure_mac(struct ast2400_scu *scu, int index)
{
	u32 reset_bit;
	u32 clkstop_bit;

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

	return 0;
}

#define SCU_CLKSTOP_SDIO 27
static ulong ast2400_enable_sdclk(struct ast2400_scu *scu)
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

static ulong ast2400_enable_extsdclk(struct ast2400_scu *scu)
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

static int ast2400_clk_enable(struct clk *clk)
{
	struct ast2400_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
		case ASPEED_CLK_GATE_MAC1CLK:
			ast2400_configure_mac(priv->scu, 1);
			break;
		case ASPEED_CLK_GATE_MAC2CLK:
			ast2400_configure_mac(priv->scu, 2);
			break;
		case ASPEED_CLK_GATE_SDCLK:
			ast2400_enable_sdclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_SDEXTCLK:
			ast2400_enable_extsdclk(priv->scu);
			break;
		default:
			pr_debug("can't enable clk \n");
			return -ENOENT;
			break;
	}

	return 0;
}

struct clk_ops ast2400_clk_ops = {
	.get_rate = ast2400_clk_get_rate,
	.set_rate = ast2400_clk_set_rate,
	.enable = ast2400_clk_enable,
};

static int ast2400_clk_probe(struct udevice *dev)
{
	struct ast2400_clk_priv *priv = dev_get_priv(dev);

	priv->scu = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	return 0;
}

static int ast2400_clk_bind(struct udevice *dev)
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
	{ ASPEED_CLK_UART, "uart" },
	{ ASPEED_CLK_HPLL, "hpll" },
	{ ASPEED_CLK_MPLL, "mpll" },
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

static const struct udevice_id ast2400_clk_ids[] = {
	{ .compatible = "aspeed,ast2400-scu" },
	{ }
};

U_BOOT_DRIVER(aspeed_scu) = {
	.name		= "aspeed_scu",
	.id		= UCLASS_CLK,
	.of_match	= ast2400_clk_ids,
	.priv_auto_alloc_size = sizeof(struct ast2400_clk_priv),
	.ops		= &ast2400_clk_ops,
	.bind		= ast2400_clk_bind,
	.probe		= ast2400_clk_probe,
};
