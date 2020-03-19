// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Facebook, Inc
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2500.h>

#define AST_I2C_BASE			0x1E78A000 /* I2C */

static int ast_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_GET_DRIVER(aspeed_scu), devp);
}

void *ast_get_scu(void)
{
	struct ast2500_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = ast_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->scu;
}

void ast_scu_unlock(struct ast2500_scu *scu)
{
	writel(SCU_UNLOCK_VALUE, &scu->protection_key);
	while (!readl(&scu->protection_key))
		;
}

void ast_scu_lock(struct ast2500_scu *scu)
{
#ifdef CONFIG_AST_SCU_LOCK
	writel(0,  &scu->protection_key);
	while (readl(&scu->protection_key))
		;
#endif
}

/*
 * Don't want to bring in whole PINCTRL for just enable I2C and WDTREST1
 */

#if CONFIG_IS_ENABLED(PINCTRL)
int ast_scu_enable_i2c_dev(struct udevice *dev)
{
	/* PINCTRL shall handle this */
	;
}

int ast_scu_enable_wdtrst1(void)
{
	/* PINCTRL shall handle this */
	;
}

#else /* !CONFIG_IS_ENABLED(PINCTRL) */

struct scu_i2c_pinctrl {
	/* Control register number (1-10) */
	unsigned reg_num;
	/* The mask of control bits in the register */
	u32 ctrl_bit_mask;
};

static const struct scu_i2c_pinctrl scu_i2c_pinctrls[] = {
	{ /*0: I2C1*/  8, (1 << 13) | (1 << 12) },
	{ /*1: I2C2*/  8, (1 << 15) | (1 << 14) },
	{ /*2: I2C3*/  8, (1 << 16) },
	{ /*3: I2C4*/  5, (1 << 17) },
	{ /*4: I2C5*/  5, (1 << 18) },
	{ /*5: I2C6*/  5, (1 << 19) },
	{ /*6: I2C7*/  5, (1 << 20) },
	{ /*7: I2C8*/  5, (1 << 21) },
	{ /*8: I2C9*/  5, (1 << 22) },
	{ /*9: I2C10*/ 5, (1 << 23) },
	{ /*10:I2C11*/ 5, (1 << 24) },
	{ /*11:I2C12*/ 5, (1 << 25) },
	{ /*12:I2C13*/ 5, (1 << 26) },
	{ /*13:I2C14*/ 5, (1 << 27) },
};

static int get_i2c_bus_idx(struct udevice *dev)
{
	int bus_num;

	fdt_addr_t bus_addr = devfdt_get_addr(dev);
	if (FDT_ADDR_T_NONE == bus_addr) {
		debug("Invalid I2C Device\n");
		return -EINVAL;
	}

	/* Regardless of labels, enable the correct AST bus. */
	bus_num = (bus_addr - AST_I2C_BASE) / 0x40;
	// The register of I2C device is not continuous.
	// I2C device 8-14 (1-base) need to subtract 4;
	if (bus_num > 7) {
		bus_num = bus_num - 4;
	}

	// bus_idx is 0-based
	debug("label(i2c%u) => idx(i2c%d)\n", dev->seq, (bus_num - 1));
	return (bus_num - 1);
}

int ast_scu_enable_i2c_dev(struct udevice *dev)
{
	struct ast2500_scu *scu;
	const struct scu_i2c_pinctrl *pinctrl;
	u32 *ctrl_reg;
	int bus_idx = get_i2c_bus_idx(dev);

	if (bus_idx < 0) {
		return bus_idx;
	}
	debug("SCU enable i2c%d \n", bus_idx);

	scu = ast_get_scu();
	if(IS_ERR_OR_NULL(scu)) {
		debug("Get SCU failed ret(%ld)\n", PTR_ERR(scu));
		return scu ? PTR_ERR(scu) : -ENODEV;
	}
	/* bus_idx 0 based */
	if (bus_idx >= ARRAY_SIZE(scu_i2c_pinctrls)) {
		debug("bus_idx %d is out of range, must be [%d - %d]\n",
		      bus_idx, 0, ARRAY_SIZE(scu_i2c_pinctrls)-1);
		return -EINVAL;
	}
	pinctrl = &scu_i2c_pinctrls[bus_idx];
	if (pinctrl->reg_num > 6)
		ctrl_reg = &scu->pinmux_ctrl1[pinctrl->reg_num - 7];
	else
		ctrl_reg = &scu->pinmux_ctrl[pinctrl->reg_num - 1];

	ast_scu_unlock(scu);
	setbits_le32(ctrl_reg, pinctrl->ctrl_bit_mask);
	ast_scu_lock(scu);

	return 0;
}

int ast_scu_enable_wdtrst1(void)
{
	struct ast2500_scu *scu;
	u32 *ctrl_reg;

	scu = ast_get_scu();
	if(IS_ERR_OR_NULL(scu)) {
		debug("Get SCU failed ret(%ld)\n", PTR_ERR(scu));
		return scu ? PTR_ERR(scu) : -ENODEV;
	}

	/* SCU_A8: pinmux_ctrl1[2], pin-2 */
	ctrl_reg = &scu->pinmux_ctrl1[2];

	ast_scu_unlock(scu);
	setbits_le32(ctrl_reg, (1<<2));
	ast_scu_lock(scu);

	return 0;
}

#endif
