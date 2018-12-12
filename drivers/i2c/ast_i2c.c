/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Copyright 2016 IBM Corporation
 * Copyright 2016 Google, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>

#include <asm/arch/ast_scu.h>
#include <asm/arch/regs-scu.h>
#include <asm/io.h>

#include "ast_i2c.h"

#define I2C_TIMEOUT_US (100000)
#define I2C_SLEEP_STEP (20)
#define EI2C_TIMEOUT (1001)

DECLARE_GLOBAL_DATA_PTR;

struct ast_i2c {
	u32 id;
	struct ast_i2c_regs *regs;
	int speed;
};

static u32 get_clk_reg_val(u32 divider_ratio)
{
	unsigned int inc = 0, div;
	u32 scl_low, scl_high, data;

	for (div = 0; divider_ratio >= 16; div++) {
		inc |= (divider_ratio & 1);
		divider_ratio >>= 1;
	}
	divider_ratio += inc;
	scl_low = (divider_ratio >> 1) - 1;
	scl_high = divider_ratio - scl_low - 2;
	data = 0x77700300 | (scl_high << 16) | (scl_low << 12) | div;
	return data;
}

static inline void ast_i2c_clear_interrupts(struct ast_i2c_regs *i2c_base)
{
	writel(~0, &i2c_base->isr);
}

static void ast_i2c_init_bus(struct ast_i2c *i2c_bus)
{
	/* Reset device */
	writel(0, &i2c_bus->regs->fcr);
	/* Enable Master Mode. Assuming single-master. */
	debug("Enable Master for %p\n", i2c_bus->regs);
	writel(AST_I2CD_MASTER_EN
	       | AST_I2CD_M_SDA_LOCK_EN
	       | AST_I2CD_MULTI_MASTER_DIS | AST_I2CD_M_SCL_DRIVE_EN,
	       &i2c_bus->regs->fcr);
	debug("FCR: %p\n", &i2c_bus->regs->fcr);
	/* Enable Interrupts */
	writel(AST_I2CD_INTR_TX_ACK
	       | AST_I2CD_INTR_TX_NAK
	       | AST_I2CD_INTR_RX_DONE
	       | AST_I2CD_INTR_BUS_RECOVER_DONE
	       | AST_I2CD_INTR_NORMAL_STOP
	       | AST_I2CD_INTR_ABNORMAL, &i2c_bus->regs->icr);
}

static int ast_i2c_probe(struct udevice *dev)
{
	u8 bus_num;
	u32 bus_addr;

	struct ast_i2c *i2c_bus = dev_get_priv(dev);
	bus_addr = dev_get_addr(dev);

	/* Regardless of labels, enable the correct AST bus. */
	bus_num = (bus_addr - AST_I2C_BASE) / 0x40;
	// The register of I2C device is not continuous.
	// I2C device 8-14 (1-base) need to subtract 4;
	if (bus_num > 7) {
		bus_num = bus_num - 4;
	}

	debug("Enabling I2C (bus %u) %u\n", bus_num, dev->seq);
	ast_scu_enable_i2c(bus_num);

	i2c_bus->id = dev->seq;
	struct ast_i2c_regs *i2c_base =
	    (struct ast_i2c_regs *)dev_get_addr(dev);
	i2c_bus->regs = i2c_base;

	ast_i2c_init_bus(i2c_bus);
	return 0;
}

static inline int ast_i2c_wait_isr(struct ast_i2c_regs *i2c_base, u32 flag)
{
	int timeout = I2C_TIMEOUT_US;
	while (!(readl(&i2c_base->isr) & flag) && timeout > 0) {
		udelay(I2C_SLEEP_STEP);
		timeout -= I2C_SLEEP_STEP;
	}

	ast_i2c_clear_interrupts(i2c_base);
	if (timeout <= 0)
		return -EI2C_TIMEOUT;
	return 0;
}

static inline int ast_i2c_send_stop(struct ast_i2c_regs *i2c_base)
{
	writel(AST_I2CD_M_STOP_CMD, &i2c_base->csr);
	return ast_i2c_wait_isr(i2c_base, AST_I2CD_INTR_NORMAL_STOP);
}

static inline int ast_i2c_wait_tx(struct ast_i2c_regs *i2c_base)
{
	int timeout = I2C_TIMEOUT_US;
	u32 flag = AST_I2CD_INTR_TX_ACK | AST_I2CD_INTR_TX_NAK;
	u32 status = readl(&i2c_base->isr) & flag;
	while (!status && timeout > 0) {
		udelay(I2C_SLEEP_STEP);
		status = readl(&i2c_base->isr) & flag;
		timeout -= I2C_SLEEP_STEP;
	}

	int ret = 0;
	if (status == AST_I2CD_INTR_TX_NAK)
		ret = -EREMOTEIO;

	if (timeout <= 0)
		ret = -EI2C_TIMEOUT;

	ast_i2c_clear_interrupts(i2c_base);
	return ret;
}

static inline int ast_i2c_start_txn(struct ast_i2c_regs *i2c_base, u8 devaddr)
{
	/* Start and Send Device Address */
	writel(devaddr, &i2c_base->trbbr);
	writel(AST_I2CD_M_START_CMD | AST_I2CD_M_TX_CMD, &i2c_base->csr);
	return ast_i2c_wait_tx(i2c_base);
}

static int ast_i2c_read_data(struct ast_i2c *i2c_bus, u8 chip_addr, u8 *buffer,
			     size_t len, bool send_stop)
{
	struct ast_i2c_regs *i2c_base = i2c_bus->regs;

	int i2c_error =
	    ast_i2c_start_txn(i2c_base, (chip_addr << 1) | I2C_M_RD);
	if (i2c_error < 0)
		return i2c_error;

	u32 i2c_cmd = AST_I2CD_M_RX_CMD;
	for (; len > 0; len--, buffer++) {
		if (len == 1)
			i2c_cmd |= AST_I2CD_M_S_RX_CMD_LAST;
		writel(i2c_cmd, &i2c_base->csr);
		i2c_error = ast_i2c_wait_isr(i2c_base, AST_I2CD_INTR_RX_DONE);
		if (i2c_error < 0)
			return i2c_error;
		*buffer = AST_I2CD_RX_DATA_BUF_GET(readl(&i2c_base->trbbr));
	}
	ast_i2c_clear_interrupts(i2c_base);

	if (send_stop)
		return ast_i2c_send_stop(i2c_base);

	return 0;
}

static int ast_i2c_write_data(struct ast_i2c *i2c_bus, u8 chip_addr, u8
			      *buffer, size_t len, bool send_stop)
{
	struct ast_i2c_regs *i2c_base = i2c_bus->regs;

	int i2c_error = ast_i2c_start_txn(i2c_base, (chip_addr << 1));
	if (i2c_error < 0)
		return i2c_error;

	for (; len > 0; len--, buffer++) {
		writel(*buffer, &i2c_base->trbbr);
		writel(AST_I2CD_M_TX_CMD, &i2c_base->csr);
		i2c_error = ast_i2c_wait_tx(i2c_base);
		if (i2c_error < 0)
			return i2c_error;
	}

	if (send_stop)
		return ast_i2c_send_stop(i2c_base);

	return 0;
}

static int ast_i2c_deblock(struct udevice *dev)
{
	struct ast_i2c *i2c_bus = dev_get_priv(dev);
	struct ast_i2c_regs *i2c_base = i2c_bus->regs;

	u32 csr = readl(&i2c_base->csr);

	int deblock_error = 0;
	bool sda_high = csr & AST_I2CD_SDA_LINE_STS;
	bool scl_high = csr & AST_I2CD_SCL_LINE_STS;
	if (sda_high && scl_high) {
		/* Bus is idle, no deblocking needed. */
		return 0;
	} else if (sda_high) {
		/* Send stop command */
		debug("Unterminated TXN in (%x), sending stop\n", csr);
		deblock_error = ast_i2c_send_stop(i2c_base);
	} else if (scl_high) {
		/* Possibly stuck slave */
		debug("Bus stuck (%x), attempting recovery\n", csr);
		writel(AST_I2CD_BUS_RECOVER_CMD, &i2c_base->csr);
		deblock_error = ast_i2c_wait_isr(
			i2c_base, AST_I2CD_INTR_BUS_RECOVER_DONE);
	} else {
		/* Just try to reinit the device. */
		ast_i2c_init_bus(i2c_bus);
	}

	return deblock_error;
}

static int ast_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct ast_i2c *i2c_bus = dev_get_priv(dev);
	int ret;

	(void)ast_i2c_deblock(dev);
	debug("i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD) {
			debug("i2c_read: chip=0x%x, len=0x%x, flags=0x%x\n",
			      msg->addr, msg->len, msg->flags);
			ret =
			    ast_i2c_read_data(i2c_bus, msg->addr, msg->buf,
						msg->len, (nmsgs == 1));
		} else {
			debug("i2c_write: chip=0x%x, len=0x%x, flags=0x%x\n",
			      msg->addr, msg->len, msg->flags);
			ret =
			    ast_i2c_write_data(i2c_bus, msg->addr, msg->buf,
						 msg->len, (nmsgs == 1));
		}
		if (ret) {
			debug("%s: error (%d)\n", __func__, ret);
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int ast_i2c_set_speed(struct udevice *dev, unsigned int speed)
{
	debug("Setting speed ofr I2C%d to <%u>\n", dev->seq, speed);
	if (!speed) {
		debug("No valid speed specified.\n");
		return -EINVAL;
	}
	struct ast_i2c *i2c_bus = dev_get_priv(dev);

	i2c_bus->speed = speed;
	/* TODO: get this from device tree */
	u32 pclk = ast_get_apbclk();
	u32 divider = pclk / speed;

	struct ast_i2c_regs *i2c_base = i2c_bus->regs;
	if (speed > 400000) {
		debug("Enabling High Speed\n");
		setbits_le32(&i2c_base->fcr, AST_I2CD_M_HIGH_SPEED_EN
			     | AST_I2CD_M_SDA_DRIVE_1T_EN
			     | AST_I2CD_SDA_DRIVE_1T_EN);
		writel(0x3, &i2c_base->cactcr2);
		writel(get_clk_reg_val(divider), &i2c_base->cactcr1);
	} else {
		debug("Enabling Normal Speed\n");
		writel(get_clk_reg_val(divider), &i2c_base->cactcr1);
		writel(AST_NO_TIMEOUT_CTRL, &i2c_base->cactcr2);
	}

	ast_i2c_clear_interrupts(i2c_base);
	return 0;
}

static const struct dm_i2c_ops ast_i2c_ops = {
	.xfer = ast_i2c_xfer,
	.set_bus_speed = ast_i2c_set_speed,
	.deblock = ast_i2c_deblock,
};

static const struct udevice_id ast_i2c_ids[] = {
	{.compatible = "aspeed,ast2400-i2c-controller",},
	{.compatible = "aspeed,ast2400-i2c-bus",},
	{},
};

/* Tell GNU Indent to keep this as is: */
/* *INDENT-OFF* */
U_BOOT_DRIVER(i2c_aspeed) = {
	.name = "i2c_aspeed",
	.id = UCLASS_I2C,
	.of_match = ast_i2c_ids,
	.probe = ast_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct ast_i2c),
	.ops = &ast_i2c_ops,
};
/* *INDENT-ON* */
