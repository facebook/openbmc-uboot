// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <asm/arch/pinctrl.h>
#include <asm/arch/scu_ast2600.h>
#include <dm/pinctrl.h>
#include "pinctrl-aspeed.h"
/*
 * This driver works with very simple configuration that has the same name
 * for group and function. This way it is compatible with the Linux Kernel
 * driver.
 */

struct ast2600_pinctrl_priv {
	struct ast2600_scu *scu;
};

static int ast2600_pinctrl_probe(struct udevice *dev)
{
	struct ast2600_pinctrl_priv *priv = dev_get_priv(dev);
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

static struct aspeed_sig_desc i2c1_link[] = {
	{ 0x418, GENMASK(9, 8), 1 },
	{ 0x4B8, GENMASK(9, 8), 0 },
};

static struct aspeed_sig_desc i2c2_link[] = {
	{ 0x418, GENMASK(11, 10), 1 },
	{ 0x4B8, GENMASK(11, 10), 0 },
};

static struct aspeed_sig_desc i2c3_link[] = {
	{ 0x418, GENMASK(13, 12), 1 },
	{ 0x4B8, GENMASK(13, 12), 0 },
};

static struct aspeed_sig_desc i2c4_link[] = {
	{ 0x418, GENMASK(15, 14), 1 },
	{ 0x4B8, GENMASK(15, 14), 0 },
};

static struct aspeed_sig_desc i2c5_link[] = {
	{ 0x418, GENMASK(17, 16), 0 },
};

static struct aspeed_sig_desc i2c6_link[] = {
	{ 0x418, GENMASK(19, 18), 0 },
};

static struct aspeed_sig_desc i2c7_link[] = {
	{ 0x418, GENMASK(21, 20), 0 },
};

static struct aspeed_sig_desc i2c8_link[] = {
	{ 0x418, GENMASK(23, 22), 0 },
};

static struct aspeed_sig_desc i2c9_link[] = {
	{ 0x418, GENMASK(25, 24), 0 },
};

static struct aspeed_sig_desc i2c10_link[] = {
	{ 0x418, GENMASK(27, 26), 0 },
};
	
static struct aspeed_sig_desc i2c11_link[] = {
	{ 0x410, GENMASK(1, 0), 1 },
	{ 0x4B0, GENMASK(1, 0), 0 },
};

static struct aspeed_sig_desc i2c12_link[] = {
	{ 0x410, GENMASK(3, 2), 1 },
	{ 0x4B0, GENMASK(3, 2), 0 },
};

static struct aspeed_sig_desc i2c13_link[] = {
	{ 0x410, GENMASK(5, 4), 1 },
	{ 0x4B0, GENMASK(5, 4), 0 },
};

static struct aspeed_sig_desc i2c14_link[] = {
	{ 0x410, GENMASK(7, 6), 1 },
	{ 0x4B0, GENMASK(7, 6), 0 },
};

static struct aspeed_sig_desc i2c15_link[] = {
	{ 0x414, GENMASK(29, 28), 1 },
	{ 0x4B4, GENMASK(29, 28), 0 },
};

static struct aspeed_sig_desc i2c16_link[] = {
	{ 0x414, GENMASK(31, 30), 1 },
	{ 0x4B4, GENMASK(31, 30), 0 },
};


static struct aspeed_sig_desc mac1_link[] = {
	{ 0x410, BIT(4), 0 },
#ifndef CONFIG_FPGA_ASPEED
	{ 0x470, BIT(4), 1 },
#endif
};

static struct aspeed_sig_desc mac2_link[] = {
	{ 0x410, BIT(5), 0	},
	{ 0x470, BIT(5), 1 },
};

static struct aspeed_sig_desc mac3_link[] = {
	{ 0x410, BIT(6), 0		},
	{ 0x470, BIT(6), 1      	},
};

static struct aspeed_sig_desc mac4_link[] = {
	{ 0x410, BIT(7), 0		},
	{ 0x470, BIT(7), 1		},
};


static struct aspeed_sig_desc rgmii1[] = {
#ifndef CONFIG_FPGA_ASPEED
	{ 0x500, BIT(6), 0         	},
	{ 0x400, GENMASK(11, 0), 0 	},
#endif
};

static struct aspeed_sig_desc rgmii2[] = {
	{ 0x500, BIT(7), 0		},
	{ 0x400, GENMASK(23, 12), 0 	},
};

static struct aspeed_sig_desc rgmii3[] = {	
	{ 0x510, BIT(0), 0         },
	{ 0x410, GENMASK(27, 16), 0	},
};

static struct aspeed_sig_desc rgmii4[] = {
	{ 0x510, BIT(1), 0         },
	{ 0x410, GENMASK(31, 28), 1	},
	{ 0x4b0, GENMASK(31, 28), 0	},
	{ 0x474, GENMASK(7, 0), 1	},
	{ 0x414, GENMASK(7, 0), 1	},
	{ 0x4b4, GENMASK(7, 0), 0	},
};

static struct aspeed_sig_desc rmii1[] = {
	{ 0x504, BIT(6), 0         	},
	{ 0x400, GENMASK(3, 0), 0	},
	{ 0x400, GENMASK(11, 6), 0	},
};

static struct aspeed_sig_desc rmii2[] = {
	{ 0x504, BIT(7), 0         	},
	{ 0x400, GENMASK(15, 12), 0	},
	{ 0x400, GENMASK(23, 18), 0	},
};

static struct aspeed_sig_desc rmii3[] = {
	{ 0x514, BIT(0), 0         	},
	{ 0x410, GENMASK(27, 22), 0	},
	{ 0x410, GENMASK(19, 16), 0	},
};

static struct aspeed_sig_desc rmii4[] = {
	{ 0x514, BIT(1), 0         	},
	{ 0x410, GENMASK(7, 2), 1	},
	{ 0x410, GENMASK(31, 28), 1	},
	{ 0x414, GENMASK(7, 2), 1	},
	{ 0x4B0, GENMASK(31, 28), 0	},
	{ 0x4B4, GENMASK(7, 2), 0	},
};

static struct aspeed_sig_desc rmii1_rclk_oe[] = {
	{ 0x340, BIT(29), 0         	},
};
static struct aspeed_sig_desc rmii2_rclk_oe[] = {
	{ 0x340, BIT(30), 0         	},
};

static struct aspeed_sig_desc rmii3_rclk_oe[] = {
	{ 0x350, BIT(29), 0         	},
};
static struct aspeed_sig_desc rmii4_rclk_oe[] = {
	{ 0x350, BIT(30), 0         	},
};

static struct aspeed_sig_desc mdio1_link[] = {
	{ 0x430, BIT(17) | BIT(16), 0	},
};

static struct aspeed_sig_desc mdio2_link[] = {
	{ 0x470, BIT(13) | BIT(12), 1	},
	{ 0x410, BIT(13) | BIT(12), 0	},
};

static struct aspeed_sig_desc mdio3_link[] = {
	{ 0x470, BIT(1) | BIT(0), 1	},
	{ 0x410, BIT(1) | BIT(0), 0	},
};

static struct aspeed_sig_desc mdio4_link[] = {
	{ 0x470, BIT(3) | BIT(2), 1	},
	{ 0x410, BIT(3) | BIT(2), 0	},
};

static struct aspeed_sig_desc sdio2_link[] = {
	{ 0x414, GENMASK(23, 16), 1	},
	{ 0x4B4, GENMASK(23, 16), 0	},
	{ 0x450, BIT(1), 0		},
};

static struct aspeed_sig_desc sdio1_link[] = {
	{ 0x414, GENMASK(15, 8), 0	},
};	

//when sdio1 8bits, sdio2 can't use
static struct aspeed_sig_desc sdio1_8bit_link[] = {
	{ 0x414, GENMASK(15, 8), 0	},
	{ 0x4b4, GENMASK(21, 18), 0	},
	{ 0x450, BIT(3), 0	},
	{ 0x450, BIT(1), 1	},
};	

static struct aspeed_sig_desc emmc_link[] = {
	{ 0x400, GENMASK(31, 24), 0 },
};

static struct aspeed_sig_desc emmcg8_link[] = {
	{ 0x400, GENMASK(31, 24), 0 },
	{ 0x404, GENMASK(3, 0), 0 },
//because it is strap use 0x4 to clear
	{ 0x504, BIT(3), 0 },
	{ 0x504, BIT(5), 0 },
};

static struct aspeed_sig_desc fmcquad_link[] = {
	{ 0x438, GENMASK(5, 4), 0 },
};

static struct aspeed_sig_desc spi1_link[] = {
	{ 0x438, GENMASK(13, 11), 0 },
};

static struct aspeed_sig_desc spi1abr_link[] = {
	{ 0x438, BIT(9), 0 },
};

static struct aspeed_sig_desc spi1cs1_link[] = {
	{ 0x438, BIT(8), 0 },
};

static struct aspeed_sig_desc spi1wp_link[] = {
	{ 0x438, BIT(10), 0 },
};
	
static struct aspeed_sig_desc spi1quad_link[] = {
	{ 0x438, GENMASK(15, 14), 0 },
};

static struct aspeed_sig_desc spi2_link[] = {
	{ 0x434, GENMASK(29, 27) | BIT(24), 0 },
};

static struct aspeed_sig_desc spi2cs1_link[] = {
	{ 0x434, BIT(25), 0 },
};

static struct aspeed_sig_desc spi2cs2_link[] = {
	{ 0x434, BIT(26), 0 },
};

static struct aspeed_sig_desc spi2quad_link[] = {
	{ 0x434, GENMASK(31, 30), 0 },
};

static struct aspeed_sig_desc fsi1[] = {
	{ 0xd48, GENMASK(21, 20), 0 },
};

static struct aspeed_sig_desc fsi2[] = {
	{ 0xd48, GENMASK(23, 22), 0 },
};

static struct aspeed_sig_desc usb2ah_link[] = {
	{ 0x440, BIT(24), 1 },
	{ 0x440, BIT(25), 0 },
};

static struct aspeed_sig_desc usb2bh_link[] = {
	{ 0x440, BIT(28), 1 },
	{ 0x440, BIT(29), 0 },
};

static struct aspeed_sig_desc pcie0rc_link[] = {
	{ 0x40, BIT(21), 0 },	
};

static struct aspeed_sig_desc pcie1rc_link[] = {
	{ 0x40, BIT(19), 0 },	//SSPRST# output enable 
	{ 0x500, BIT(24), 0 },	//dedicate rc reset
};

static const struct aspeed_group_config ast2600_groups[] = {
	{ "MAC1LINK", ARRAY_SIZE(mac1_link), mac1_link },
	{ "MAC2LINK", ARRAY_SIZE(mac2_link), mac2_link },
	{ "MAC3LINK", ARRAY_SIZE(mac3_link), mac3_link },
	{ "MAC4LINK", ARRAY_SIZE(mac4_link), mac4_link },
	{ "RGMII1", ARRAY_SIZE(rgmii1), rgmii1 },
	{ "RGMII2", ARRAY_SIZE(rgmii2), rgmii2 },
	{ "RGMII3", ARRAY_SIZE(rgmii3), rgmii3 },
	{ "RGMII4", ARRAY_SIZE(rgmii4), rgmii4 },
	{ "RMII1", ARRAY_SIZE(rmii1), rmii1 },
	{ "RMII2", ARRAY_SIZE(rmii2), rmii2 },
	{ "RMII3", ARRAY_SIZE(rmii3), rmii3 },
	{ "RMII4", ARRAY_SIZE(rmii4), rmii4 },
	{ "RMII1RCLK", ARRAY_SIZE(rmii1_rclk_oe), rmii1_rclk_oe },
	{ "RMII2RCLK", ARRAY_SIZE(rmii2_rclk_oe), rmii2_rclk_oe },
	{ "RMII3RCLK", ARRAY_SIZE(rmii3_rclk_oe), rmii3_rclk_oe },
	{ "RMII4RCLK", ARRAY_SIZE(rmii4_rclk_oe), rmii4_rclk_oe },
	{ "MDIO1", ARRAY_SIZE(mdio1_link), mdio1_link },
	{ "MDIO2", ARRAY_SIZE(mdio2_link), mdio2_link },
	{ "MDIO3", ARRAY_SIZE(mdio3_link), mdio3_link },
	{ "MDIO4", ARRAY_SIZE(mdio4_link), mdio4_link },
	{ "SD1", ARRAY_SIZE(sdio1_link), sdio1_link },
	{ "SD1_8bits", ARRAY_SIZE(sdio1_8bit_link), sdio1_8bit_link },
	{ "SD2", ARRAY_SIZE(sdio2_link), sdio2_link },
	{ "EMMC", ARRAY_SIZE(emmc_link), emmc_link },
	{ "EMMCG8", ARRAY_SIZE(emmcg8_link), emmcg8_link },
	{ "FMCQUAD", ARRAY_SIZE(fmcquad_link), fmcquad_link },
	{ "SPI1", ARRAY_SIZE(spi1_link), spi1_link },
	{ "SPI1ABR", ARRAY_SIZE(spi1abr_link), spi1abr_link },
	{ "SPI1CS1", ARRAY_SIZE(spi1cs1_link), spi1cs1_link },
	{ "SPI1WP", ARRAY_SIZE(spi1wp_link), spi1wp_link },
	{ "SPI1QUAD", ARRAY_SIZE(spi1quad_link), spi1quad_link },
	{ "SPI2", ARRAY_SIZE(spi2_link), spi2_link },
	{ "SPI2CS1", ARRAY_SIZE(spi2cs1_link), spi2cs1_link },
	{ "SPI2CS2", ARRAY_SIZE(spi2cs2_link), spi2cs2_link },
	{ "SPI2QUAD", ARRAY_SIZE(spi2quad_link), spi2quad_link },
	{ "I2C1", ARRAY_SIZE(i2c1_link), i2c1_link },
	{ "I2C2", ARRAY_SIZE(i2c2_link), i2c2_link },
	{ "I2C3", ARRAY_SIZE(i2c3_link), i2c3_link },
	{ "I2C4", ARRAY_SIZE(i2c4_link), i2c4_link },
	{ "I2C5", ARRAY_SIZE(i2c5_link), i2c5_link },
	{ "I2C6", ARRAY_SIZE(i2c6_link), i2c6_link },
	{ "I2C7", ARRAY_SIZE(i2c7_link), i2c7_link },
	{ "I2C8", ARRAY_SIZE(i2c8_link), i2c8_link },
	{ "I2C9", ARRAY_SIZE(i2c9_link), i2c9_link },
	{ "I2C10", ARRAY_SIZE(i2c10_link), i2c10_link },
	{ "I2C11", ARRAY_SIZE(i2c11_link), i2c11_link },
	{ "I2C12", ARRAY_SIZE(i2c12_link), i2c12_link },
	{ "I2C13", ARRAY_SIZE(i2c13_link), i2c13_link },
	{ "I2C14", ARRAY_SIZE(i2c14_link), i2c14_link },
	{ "I2C15", ARRAY_SIZE(i2c15_link), i2c15_link },
	{ "I2C16", ARRAY_SIZE(i2c16_link), i2c16_link },
	{ "FSI1", ARRAY_SIZE(fsi1), fsi1 },
	{ "FSI2", ARRAY_SIZE(fsi2), fsi2 },
	{ "USB2AH", ARRAY_SIZE(usb2ah_link), usb2ah_link },
	{ "USB2BH", ARRAY_SIZE(usb2bh_link), usb2bh_link },
	{ "PCIE0RC", ARRAY_SIZE(pcie0rc_link), pcie0rc_link },
	{ "PCIE1RC", ARRAY_SIZE(pcie1rc_link), pcie1rc_link },	
};

static int ast2600_pinctrl_get_groups_count(struct udevice *dev)
{
	debug("PINCTRL: get_(functions/groups)_count\n");

	return ARRAY_SIZE(ast2600_groups);
}

static const char *ast2600_pinctrl_get_group_name(struct udevice *dev,
						  unsigned selector)
{
	debug("PINCTRL: get_(function/group)_name %u\n", selector);

	return ast2600_groups[selector].group_name;
}

static int ast2600_pinctrl_group_set(struct udevice *dev, unsigned selector,
				     unsigned func_selector)
{
	struct ast2600_pinctrl_priv *priv = dev_get_priv(dev);
	const struct aspeed_group_config *config;
	const struct aspeed_sig_desc *descs;
	u32 ctrl_reg = (u32)priv->scu;
	u32 i;

	debug("PINCTRL: group_set <%u, %u>\n", selector, func_selector);
	if (selector >= ARRAY_SIZE(ast2600_groups))
		return -EINVAL;

	config = &ast2600_groups[selector];
	for (i = 0; i < config->ndescs; i++) {
		descs = &config->descs[i];
		if (descs->clr) {
			clrbits_le32((u32)ctrl_reg + descs->offset,
				     descs->reg_set);
		} else {
			setbits_le32((u32)ctrl_reg + descs->offset,
				     descs->reg_set);
		}
	}

	return 0;
}

static int
ast2600_pinctrl_restore_to_gpio(struct udevice *dev, int gpio_offset)
{
	struct ast2600_pinctrl_priv *priv = dev_get_priv(dev);
	u32 ctrl_reg = (u32)priv->scu;
	u32 reg_off, reg_bit;
	bool bit_set = false;

	switch(gpio_offset) {
		case 184: reg_off = 0x434; reg_bit = BIT(24); break; /* GPIOX0 */
		case 187: reg_off = 0x434; reg_bit = BIT(27); break; /* GPIOX3 */
		case 188: reg_off = 0x434; reg_bit = BIT(28); break; /* GPIOX4 */
		case 189: reg_off = 0x434; reg_bit = BIT(29); break; /* GPIOX5 */
		case 200: reg_off = 0x438; reg_bit = BIT(8);  break; /* GPIOZ0 */
		case 203: reg_off = 0x438; reg_bit = BIT(11); break; /* GPIOZ3 */
		case 204: reg_off = 0x438; reg_bit = BIT(12); break; /* GPIOZ4 */
		case 205: reg_off = 0x438; reg_bit = BIT(13); break; /* GPIOZ5 */
		default:
			dev_warn(dev, "TODO: restore pin-%u to GPIO%c%u\n",
				gpio_offset, 'A' + (gpio_offset >> 3) % 26,
				gpio_offset & 0x7);
			return -EINVAL;
	}
	dev_info(dev, "restore pin-%u to GPIO%c%u\n", gpio_offset,
		'A' + (gpio_offset >> 3) % 26, gpio_offset & 0x7);
	if (bit_set)
		setbits_le32(ctrl_reg + reg_off, reg_bit);
	else
		clrbits_le32(ctrl_reg + reg_off, reg_bit);
	return 0;
}

static int
ast2600_pinctrl_request(struct udevice *dev, int func, int flags)
{
	if (!flags) {
		return ast2600_pinctrl_restore_to_gpio(dev, func);
	}
	dev_err(dev, "unknown flags %d", flags);
	return -EINVAL;
}

static struct pinctrl_ops ast2600_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_groups_count = ast2600_pinctrl_get_groups_count,
	.get_group_name = ast2600_pinctrl_get_group_name,
	.get_functions_count = ast2600_pinctrl_get_groups_count,
	.get_function_name = ast2600_pinctrl_get_group_name,
	.pinmux_group_set = ast2600_pinctrl_group_set,
	.request = ast2600_pinctrl_request,
};

static const struct udevice_id ast2600_pinctrl_ids[] = {
	{ .compatible = "aspeed,g6-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_aspeed) = {
	.name = "aspeed_ast2600_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = ast2600_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct ast2600_pinctrl_priv),
	.ops = &ast2600_pinctrl_ops,
	.probe = ast2600_pinctrl_probe,
};
