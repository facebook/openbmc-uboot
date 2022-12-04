// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <linux/bitfield.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <fdtdec.h>
#include <asm/io.h>
#include "dp_mcu_firmware.h"

#define MCU_CTRL                        0x180100e0
#define  MCU_CTRL_AHBS_IMEM_EN          BIT(0)
#define  MCU_CTRL_AHBS_SW_RST           BIT(4)
#define  MCU_CTRL_AHBM_SW_RST           BIT(8)
#define  MCU_CTRL_CORE_SW_RST           BIT(12)
#define  MCU_CTRL_DMEM_SHUT_DOWN        BIT(16)
#define  MCU_CTRL_DMEM_SLEEP            BIT(17)
#define  MCU_CTRL_DMEM_CLK_OFF          BIT(18)
#define  MCU_CTRL_IMEM_SHUT_DOWN        BIT(20)
#define  MCU_CTRL_IMEM_SLEEP            BIT(21)
#define  MCU_CTRL_IMEM_CLK_OFF          BIT(22)
#define  MCU_CTRL_IMEM_SEL              BIT(24)
#define  MCU_CTRL_CONFIG                BIT(28)

#define MCU_INTR_CTRL                   0x180100e8
#define  MCU_INTR_CTRL_CLR              GENMASK(7, 0)
#define  MCU_INTR_CTRL_MASK             GENMASK(15, 8)
#define  MCU_INTR_CTRL_EN               GENMASK(23, 16)

#define MCU_IMEM_START                  0x18020000

struct aspeed_dp_priv {
	void *ctrl_base;
};

static int aspeed_dp_probe(struct udevice *dev)
{
	struct aspeed_dp_priv *dp = dev_get_priv(dev);
	struct reset_ctl dp_reset_ctl, dpmcu_reset_ctrl;
	int i, ret = 0, len;
	const u32 *cell;
	u32 tmp, mcu_ctrl;

	/* Get the controller base address */
	dp->ctrl_base = (void *)devfdt_get_addr_index(dev, 0);

	debug("%s(dev=%p) \n", __func__, dev);

	ret = reset_get_by_index(dev, 0, &dp_reset_ctl);
	if (ret) {
		printf("%s(): Failed to get dp reset signal\n", __func__);
		return ret;
	}

	ret = reset_get_by_index(dev, 1, &dpmcu_reset_ctrl);
	if (ret) {
		printf("%s(): Failed to get dp mcu reset signal\n", __func__);
		return ret;
	}

	/* release reset for DPTX and DPMCU */
	reset_assert(&dp_reset_ctl);
	reset_assert(&dpmcu_reset_ctrl);
	reset_deassert(&dp_reset_ctl);
	reset_deassert(&dpmcu_reset_ctrl);

	/* select HOST or BMC as display control master
	enable or disable sending EDID to Host	*/
	writel(readl(dp->ctrl_base + 0xB8) & ~(BIT(24) | BIT(28)), dp->ctrl_base + 0xB8);

	/* DPMCU */
	/* clear display format and enable region */
	writel(0, 0x18000de0);
	
	/* load DPMCU firmware to internal instruction memory */
	mcu_ctrl = MCU_CTRL_CONFIG | MCU_CTRL_IMEM_CLK_OFF | MCU_CTRL_IMEM_SHUT_DOWN |
	      MCU_CTRL_DMEM_CLK_OFF | MCU_CTRL_DMEM_SHUT_DOWN | MCU_CTRL_AHBS_SW_RST;
	writel(mcu_ctrl, MCU_CTRL);

	mcu_ctrl &= ~(MCU_CTRL_IMEM_SHUT_DOWN | MCU_CTRL_DMEM_SHUT_DOWN);
	writel(mcu_ctrl, MCU_CTRL);

	mcu_ctrl &= ~(MCU_CTRL_IMEM_CLK_OFF | MCU_CTRL_DMEM_CLK_OFF);
	writel(mcu_ctrl, MCU_CTRL);

	mcu_ctrl |= MCU_CTRL_AHBS_IMEM_EN;
	writel(mcu_ctrl, MCU_CTRL);

	for (i = 0; i < ARRAY_SIZE(firmware_ast2600_dp); i++)
		writel(firmware_ast2600_dp[i], MCU_IMEM_START + (i * 4));

	// update configs to dmem for re-driver
	writel(0x0000dead, 0x18000e00);	// mark re-driver cfg not ready
	cell = dev_read_prop(dev, "eq-table", &len);
	if (cell) {
		for (i = 0; i < len / sizeof(u32); ++i)
			writel(fdt32_to_cpu(cell[i]), 0x18000e04 + i * 4);
	} else {
		debug("%s(): Failed to get eq-table for re-driver\n", __func__);
		goto ERR_DTS;
	}

	tmp = dev_read_s32_default(dev, "i2c-base-addr", -1);
	if (tmp == -1) {
		debug("%s(): Failed to get i2c port's base address\n", __func__);
		goto ERR_DTS;
	}
	writel(tmp, 0x18000e28);

	tmp = dev_read_s32_default(dev, "i2c-buf-addr", -1);
	if (tmp == -1) {
		debug("%s(): Failed to get i2c port's buf address\n", __func__);
		goto ERR_DTS;
	}
	writel(tmp, 0x18000e2c);

	tmp = dev_read_s32_default(dev, "dev-addr", -1);
	if (tmp == -1)
		tmp = 0x70;
	writel(tmp, 0x18000e30);
	writel(0x0000cafe, 0x18000e00);	// mark re-driver cfg ready

ERR_DTS:
	/* release DPMCU internal reset */
	mcu_ctrl &= ~MCU_CTRL_AHBS_IMEM_EN;
	writel(mcu_ctrl, MCU_CTRL);
	mcu_ctrl |= MCU_CTRL_CORE_SW_RST | MCU_CTRL_AHBM_SW_RST;
	writel(mcu_ctrl, MCU_CTRL);
	//disable dp interrupt
	writel(FIELD_PREP(MCU_INTR_CTRL_EN, 0xff), MCU_INTR_CTRL);
	//set vga ASTDP with DPMCU FW handling scratch
	writel(readl(0x1e6e2100) | (0x7 << 9), 0x1e6e2100);	

	return 0;
}

static int dp_aspeed_ofdata_to_platdata(struct udevice *dev)
{
	struct aspeed_dp_priv *dp = dev_get_priv(dev);

	/* Get the controller base address */
	dp->ctrl_base = (void *)devfdt_get_addr_index(dev, 0);

	return 0;
}

static const struct udevice_id aspeed_dp_ids[] = {
	{ .compatible = "aspeed,ast2600-displayport" },
	{ }
};

U_BOOT_DRIVER(aspeed_dp) = {
	.name		= "aspeed_dp",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_dp_ids,
	.probe		= aspeed_dp_probe,
	.ofdata_to_platdata   = dp_aspeed_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct aspeed_dp_priv),
};
