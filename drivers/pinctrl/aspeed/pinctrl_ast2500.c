// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/pinctrl.h>
#include <asm/arch/scu_ast2500.h>
#include <dm/pinctrl.h>
#include "pinctrl-aspeed.h"
/*
 * This driver works with very simple configuration that has the same name
 * for group and function. This way it is compatible with the Linux Kernel
 * driver.
 */

struct ast2500_pinctrl_priv {
	struct ast2500_scu *scu;
};

static int ast2500_pinctrl_probe(struct udevice *dev)
{
	struct ast2500_pinctrl_priv *priv = dev_get_priv(dev);
	struct udevice *clk_dev;
	int ret = 0;

	/* find SCU base address from clock device */
	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_GET_DRIVER(aspeed_scu),
                                          &clk_dev);
        if (ret) {
		debug("clock device not found\n");
		return ret;
        }

	priv->scu = devfdt_get_addr_ptr(clk_dev);
	if (IS_ERR(priv->scu)) {
		debug("%s(): can't get SCU\n", __func__);
		return PTR_ERR(priv->scu);
	}

	return 0;
}

static struct aspeed_sig_desc mac1_link[] = {
	{ 0x80, BIT(0), 0	},
};

static struct aspeed_sig_desc mac2_link[] = {
	{ 0x80, BIT(1), 0	},
};

static struct aspeed_sig_desc mdio1_link[] = {
	{ 0x88, BIT(31) | BIT(30), 0	},
};

static struct aspeed_sig_desc mdio2_link[] = {
	{ 0x90, BIT(2), 0	},
};

static struct aspeed_sig_desc i2c3_link[] = {
	{ 0x90, BIT(16), 0	},
};

static struct aspeed_sig_desc i2c4_link[] = {
	{ 0x90, BIT(17), 0	},
};

static struct aspeed_sig_desc i2c5_link[] = {
	{ 0x90, BIT(18), 0	},
};

static struct aspeed_sig_desc i2c6_link[] = {
	{ 0x90, BIT(19), 0	},
};

static struct aspeed_sig_desc i2c7_link[] = {
	{ 0x90, BIT(20), 0	},
};

static struct aspeed_sig_desc i2c8_link[] = {
	{ 0x90, BIT(21), 0	},
};

static struct aspeed_sig_desc i2c9_link[] = {
	{ 0x90, BIT(22), 0	},
	{ 0x90, BIT(6), 1	},
};

static struct aspeed_sig_desc i2c10_link[] = {
	{ 0x90, BIT(23), 0	},
	{ 0x90, BIT(0), 1	},
};

static struct aspeed_sig_desc i2c11_link[] = {
	{ 0x90, BIT(24), 0	},
	{ 0x90, BIT(0), 1	},
};

static struct aspeed_sig_desc i2c12_link[] = {
	{ 0x90, BIT(25), 0	},
	{ 0x90, BIT(0), 1	},
};

static struct aspeed_sig_desc i2c13_link[] = {
	{ 0x90, BIT(26), 0	},
	{ 0x90, BIT(0), 1	},
};

static struct aspeed_sig_desc i2c14_link[] = {
	{ 0x90, BIT(27), 0	},
};

static struct aspeed_sig_desc sdio1_link[] = {
	{ 0x90, BIT(0), 0	},
};

static struct aspeed_sig_desc sdio2_link[] = {
	{ 0x90, BIT(1), 0	},
};

static const struct aspeed_group_config ast2500_groups[] = {
	{ "MAC1LINK", 1, mac1_link },
	{ "MAC2LINK", 1, mac2_link },
	{ "MDIO1", 1, mdio1_link },
	{ "MDIO2", 1, mdio2_link },
	{ "I2C3", 1, i2c3_link },
	{ "I2C4", 1, i2c4_link },
	{ "I2C5", 1, i2c5_link },
	{ "I2C6", 1, i2c6_link },
	{ "I2C7", 1, i2c7_link },
	{ "I2C8", 1, i2c8_link },
	{ "I2C9", 2, i2c9_link },
	{ "I2C10", 2, i2c10_link },
	{ "I2C11", 2, i2c11_link },
	{ "I2C12", 2, i2c12_link },
	{ "I2C13", 2, i2c13_link },
	{ "I2C14", 1, i2c14_link },
	{ "SD2", 1, sdio2_link },
	{ "SD1", 1, sdio1_link },
};

static struct aspeed_sig_desc gpior2_link[] = {
	{ 0x88, BIT(26), 1	},
	{ 0x94, BIT(0), 1	},
	{ 0x94, BIT(1), 1	},
};

static struct aspeed_sig_desc gpior3_link[] = {
	{ 0x88, BIT(27), 1	},
	{ 0x94, BIT(0), 1	},
	{ 0x94, BIT(1), 1	},
};

static struct aspeed_sig_desc gpior4_link[] = {
	{ 0x88, BIT(28), 1	},
	{ 0x94, BIT(0), 1	},
	{ 0x94, BIT(1), 1	},
};

static struct aspeed_sig_desc gpior5_link[] = {
	{ 0x88, BIT(29), 1	},
	{ 0x94, BIT(0), 1	},
	{ 0x94, BIT(1), 1	},
};

static const struct aspeed_group_config ast2500_gpio_funcs[] = {
	{ "GPIOR2", 3, gpior2_link },
	{ "GPIOR3", 3, gpior3_link },
	{ "GPIOR4", 3, gpior4_link },
	{ "GPIOR5", 3, gpior5_link },
};

static int ast2500_pinctrl_get_groups_count(struct udevice *dev)
{
	debug("PINCTRL: get_(functions/groups)_count\n");

	return ARRAY_SIZE(ast2500_groups);
}

static const char *ast2500_pinctrl_get_group_name(struct udevice *dev,
						  unsigned selector)
{
	debug("PINCTRL: get_(function/group)_name %u\n", selector);

	return ast2500_groups[selector].group_name;
}

static int
convert_gpio_offset_to_label(unsigned offset, char* label, unsigned maxlen)
{
	unsigned bank_id, pin_id;
	int len;

	bank_id = offset >> 3;
	pin_id = offset & 0x7;
	for ( len = 0; len < maxlen && len < bank_id / 26; len++ ) {
		label[len] = 'A';
	}
	if (len < maxlen) {
		label[len++] = 'A' + bank_id % 26;
	} else {
		return -EINVAL;
	}
	if (len < maxlen) {
		label[len++] = '0' + pin_id;
	} else {
		return -EINVAL;
	}

	return len;
}

static char ast2500_pin_name[8] = {'G','P','I','O',};
static const int pin_name_prefix_len = 4;
static const int pin_label_max_len = 3;

static const char*
ast2500_pinctrl_get_pin_name(struct udevice *dev, unsigned selector)
{
	char* label;
	int label_len;

	label = &ast2500_pin_name[pin_name_prefix_len];
	label_len = convert_gpio_offset_to_label(selector, label,
						pin_label_max_len);
	if (label_len <= 0) {
		dev_warn(dev, "unknown pin(%u)\n", selector);
		label[0] = label[1] = label[2] = '?';
		label_len = pin_label_max_len;
	}
	ast2500_pin_name[pin_name_prefix_len + label_len] = '\0';
	return ast2500_pin_name;
}

static void
do_pinctrl_config(struct udevice* dev, const struct aspeed_group_config *config)
{
	struct ast2500_pinctrl_priv *priv = dev_get_priv(dev);
	const struct aspeed_sig_desc *descs;
	u32 *ctrl_reg = (u32*)priv->scu;
	int i;

	for( i = 0; i < config->ndescs; i++) {
		descs = &config->descs[i];
		if(descs->clr) {
			clrbits_le32((u32)ctrl_reg + descs->offset, descs->reg_set);
		} else {
			setbits_le32((u32)ctrl_reg + descs->offset, descs->reg_set);
		}
	}
}

static int
ast2500_pinctrl_restore_to_gpio(struct udevice *dev, int gpio_offset)
{
	const char* pin_name;
	const struct aspeed_group_config *config;
	int func;

	pin_name = ast2500_pinctrl_get_pin_name(dev, gpio_offset);
	/* find the gpio func to be restored in gpio_mux_func table */
	for (func = 0;
	     func < ARRAY_SIZE(ast2500_gpio_funcs) &&
	     strcmp(ast2500_gpio_funcs[func].group_name, pin_name);
	     ++func ) {
		/* nothing */
	}
	if (func >= ARRAY_SIZE(ast2500_gpio_funcs)) {
		dev_err(dev, "TODO: restore pin-%u to %s\n",
			gpio_offset, pin_name);
		return -EINVAL;
	}

	dev_info(dev, "Restore pin-%u to %s\n", gpio_offset, pin_name);
	config = &ast2500_gpio_funcs[func];
	do_pinctrl_config(dev, config);

	return 0;
}

static int ast2500_pinctrl_group_set(struct udevice *dev, unsigned selector,
				     unsigned func_selector)
{
	const struct aspeed_group_config *config;

	debug("PINCTRL: group_set <%u, %u> \n", selector, func_selector);
	if (selector >= ARRAY_SIZE(ast2500_groups))
		return -EINVAL;

	config = &ast2500_groups[selector];
	do_pinctrl_config(dev, config);

	return 0;
}

static int
ast2500_pinctrl_request(struct udevice *dev, int func, int flags)
{
	if (!flags) {
		return ast2500_pinctrl_restore_to_gpio(dev, func);
	}
	dev_err(dev, "unknown flags %d", flags);
	return -EINVAL;
}

static struct pinctrl_ops ast2500_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_groups_count = ast2500_pinctrl_get_groups_count,
	.get_group_name = ast2500_pinctrl_get_group_name,
	.get_functions_count = ast2500_pinctrl_get_groups_count,
	.get_function_name = ast2500_pinctrl_get_group_name,
	.pinmux_group_set = ast2500_pinctrl_group_set,
	.get_pin_name = ast2500_pinctrl_get_pin_name,
	.request = ast2500_pinctrl_request,
};

static const struct udevice_id ast2500_pinctrl_ids[] = {
	{ .compatible = "aspeed,g5-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_aspeed) = {
	.name = "aspeed_ast2500_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = ast2500_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct ast2500_pinctrl_priv),
	.ops = &ast2500_pinctrl_ops,
	.probe = ast2500_pinctrl_probe,
};
