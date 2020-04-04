// SPDX-License-Identifier: GPL-2.0+
/*
 * ASPEED AST2500 FMC/SPI Controller driver
 *
 * Copyright (c) 2015-2018, IBM Corporation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>
#include <linux/ioport.h>

#define ASPEED_SPI_MAX_CS		3

struct aspeed_spi_regs {
	u32 conf;			/* 0x00 CE Type Setting */
	u32 ctrl;			/* 0x04 Control */
	u32 intr_ctrl;			/* 0x08 Interrupt Control and Status */
	u32 cmd_ctrl;			/* 0x0c Command Control */
	u32 ce_ctrl[ASPEED_SPI_MAX_CS];	/* 0x10 .. 0x18 CEx Control */
	u32 _reserved0[5];		/* .. */
	u32 segment_addr[ASPEED_SPI_MAX_CS];
					/* 0x30 .. 0x38 Segment Address */
	u32 _reserved1[5];		/* .. */
	u32 soft_rst_cmd_ctrl;	/* 0x50 Auto Soft-Reset Command Control */
	u32 _reserved2[11];		/* .. */
	u32 dma_ctrl;			/* 0x80 DMA Control/Status */
	u32 dma_flash_addr;		/* 0x84 DMA Flash Side Address */
	u32 dma_dram_addr;		/* 0x88 DMA DRAM Side Address */
	u32 dma_len;			/* 0x8c DMA Length Register */
	u32 dma_checksum;		/* 0x90 Checksum Calculation Result */
	u32 timings;			/* 0x94 Read Timing Compensation */

	/* not used */
	u32 soft_strap_status;		/* 0x9c Software Strap Status */
	u32 write_cmd_filter_ctrl;	/* 0xa0 Write Command Filter Control */
	u32 write_addr_filter_ctrl;	/* 0xa4 Write Address Filter Control */
	u32 lock_ctrl_reset;		/* 0xa8 Lock Control (SRST#) */
	u32 lock_ctrl_wdt;		/* 0xac Lock Control (Watchdog) */
	u32 write_addr_filter[5];	/* 0xb0 Write Address Filter */
};

/* CE Type Setting Register */
#define CONF_ENABLE_W2			BIT(18)
#define CONF_ENABLE_W1			BIT(17)
#define CONF_ENABLE_W0			BIT(16)
#define CONF_FLASH_TYPE2		4
#define CONF_FLASH_TYPE1		2	/* Hardwired to SPI */
#define CONF_FLASH_TYPE0		0	/* Hardwired to SPI */
#define	  CONF_FLASH_TYPE_NOR		0x0
#define	  CONF_FLASH_TYPE_SPI		0x2

/* CE Control Register */
#define CTRL_EXTENDED2			BIT(2)	/* 32 bit addressing for SPI */
#define CTRL_EXTENDED1			BIT(1)	/* 32 bit addressing for SPI */
#define CTRL_EXTENDED0			BIT(0)	/* 32 bit addressing for SPI */

/* Interrupt Control and Status Register */
#define INTR_CTRL_DMA_STATUS		BIT(11)
#define INTR_CTRL_CMD_ABORT_STATUS	BIT(10)
#define INTR_CTRL_WRITE_PROTECT_STATUS	BIT(9)
#define INTR_CTRL_DMA_EN		BIT(3)
#define INTR_CTRL_CMD_ABORT_EN		BIT(2)
#define INTR_CTRL_WRITE_PROTECT_EN	BIT(1)

/* CEx Control Register */
#define CE_CTRL_IO_MODE_MASK		GENMASK(31, 28)
#define CE_CTRL_IO_QPI_DATA			BIT(31)
#define CE_CTRL_IO_DUAL_DATA		BIT(29)
#define CE_CTRL_IO_DUAL_ADDR_DATA	(BIT(29) | BIT(28))
#define CE_CTRL_IO_QUAD_DATA		BIT(30)
#define CE_CTRL_IO_QUAD_ADDR_DATA	(BIT(30) | BIT(28))
#define CE_CTRL_CMD_SHIFT		16
#define CE_CTRL_CMD_MASK		0xff
#define CE_CTRL_CMD(cmd)					\
	(((cmd) & CE_CTRL_CMD_MASK) << CE_CTRL_CMD_SHIFT)
#define CE_CTRL_DUMMY_HIGH_SHIFT	14
#define CE_CTRL_DUMMY_HIGH_MASK		0x1
#define CE_CTRL_CLOCK_FREQ_SHIFT	8
#define CE_CTRL_CLOCK_FREQ_MASK		0xf
#define CE_CTRL_CLOCK_FREQ(div)						\
	(((div) & CE_CTRL_CLOCK_FREQ_MASK) << CE_CTRL_CLOCK_FREQ_SHIFT)
#define CE_G6_CTRL_CLOCK_FREQ(div)						\
	((((div) & CE_CTRL_CLOCK_FREQ_MASK) << CE_CTRL_CLOCK_FREQ_SHIFT) | (((div) & 0xf0) << 20))
#define CE_CTRL_DUMMY_LOW_SHIFT		6 /* 2 bits [7:6] */
#define CE_CTRL_DUMMY_LOW_MASK		0x3
#define CE_CTRL_DUMMY(dummy)						\
	(((((dummy) >> 2) & CE_CTRL_DUMMY_HIGH_MASK)			\
	  << CE_CTRL_DUMMY_HIGH_SHIFT) |				\
	 (((dummy) & CE_CTRL_DUMMY_LOW_MASK) << CE_CTRL_DUMMY_LOW_SHIFT))
#define CE_CTRL_STOP_ACTIVE		BIT(2)
#define CE_CTRL_MODE_MASK		0x3
#define	  CE_CTRL_READMODE		0x0
#define	  CE_CTRL_FREADMODE		0x1
#define	  CE_CTRL_WRITEMODE		0x2
#define	  CE_CTRL_USERMODE		0x3

/* Auto Soft-Reset Command Control */
#define SOFT_RST_CMD_EN     GENMASK(1, 0)

/*
 * The Segment Register uses a 8MB unit to encode the start address
 * and the end address of the AHB window of a SPI flash device.
 * Default segment addresses are :
 *
 *   CE0  0x20000000 - 0x2fffffff  128MB
 *   CE1  0x28000000 - 0x29ffffff   32MB
 *   CE2  0x2a000000 - 0x2bffffff   32MB
 *
 * The full address space of the AHB window of the controller is
 * covered and CE0 start address and CE2 end addresses are read-only.
 */
#define SEGMENT_ADDR_START(reg)		((((reg) >> 16) & 0xff) << 23)
#define SEGMENT_ADDR_END(reg)		((((reg) >> 24) & 0xff) << 23)
#define SEGMENT_ADDR_VALUE(start, end)					\
	(((((start) >> 23) & 0xff) << 16) | ((((end) >> 23) & 0xff) << 24))

#define G6_SEGMENT_ADDR_START(reg)		(reg & 0xffff)
#define G6_SEGMENT_ADDR_END(reg)		((reg >> 16) & 0xffff)
#define G6_SEGMENT_ADDR_VALUE(start, end)					\
	((((start) >> 16) & 0xffff) | (((end) - 0x100000) & 0xffff0000))

/* DMA Control/Status Register */
#define DMA_CTRL_DELAY_SHIFT		8
#define DMA_CTRL_DELAY_MASK		0xf
#define DMA_CTRL_FREQ_SHIFT		4
#define G6_DMA_CTRL_FREQ_SHIFT		16

#define DMA_CTRL_FREQ_MASK		0xf
#define TIMING_MASK(div, delay)					   \
	(((delay & DMA_CTRL_DELAY_MASK) << DMA_CTRL_DELAY_SHIFT) | \
	 ((div & DMA_CTRL_FREQ_MASK) << DMA_CTRL_FREQ_SHIFT))
#define G6_TIMING_MASK(div, delay)					   \
	(((delay & DMA_CTRL_DELAY_MASK) << DMA_CTRL_DELAY_SHIFT) | \
	 ((div & DMA_CTRL_FREQ_MASK) << G6_DMA_CTRL_FREQ_SHIFT))
#define DMA_CTRL_CALIB			BIT(3)
#define DMA_CTRL_CKSUM			BIT(2)
#define DMA_CTRL_WRITE			BIT(1)
#define DMA_CTRL_ENABLE			BIT(0)

/* for ast2600 setting */
#define SPI_3B_AUTO_CLR_REG   0x1e6e2510
#define SPI_3B_AUTO_CLR       BIT(9)


/*
 * flash related info
 */
struct aspeed_spi_flash {
	u8		cs;
	bool		init;		/* Initialized when the SPI bus is
					 * first claimed
					 */
	void __iomem	*ahb_base;	/* AHB Window for this device */
	u32		ahb_size;	/* AHB Window segment size */
	u32		ce_ctrl_user;	/* CE Control Register for USER mode */
	u32		ce_ctrl_fread;	/* CE Control Register for FREAD mode */
	u32		iomode;

	struct spi_flash *spi;		/* Associated SPI Flash device */
};

struct aspeed_spi_priv {
	struct aspeed_spi_regs	*regs;
	void __iomem	*ahb_base;	/* AHB Window for all flash devices */
	int new_ver;
	u32		ahb_size;	/* AHB Window segments size */

	ulong		hclk_rate;	/* AHB clock rate */
	u32		max_hz;
	u8		num_cs;
	bool		is_fmc;

	struct aspeed_spi_flash flashes[ASPEED_SPI_MAX_CS];
	u32		flash_count;

	u8		cmd_buf[16];	/* SPI command in progress */
	size_t		cmd_len;
};

static struct aspeed_spi_flash *aspeed_spi_get_flash(struct udevice *dev)
{
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	struct aspeed_spi_priv *priv = dev_get_priv(dev->parent);
	u8 cs = slave_plat->cs;

	if (cs >= priv->flash_count) {
		pr_err("invalid CS %u\n", cs);
		return NULL;
	}

	return &priv->flashes[cs];
}

static u32 aspeed_g6_spi_hclk_divisor(struct aspeed_spi_priv *priv, u32 max_hz)
{
	u32 hclk_rate = priv->hclk_rate;
	/* HCLK/1 ..	HCLK/16 */
	const u8 hclk_masks[] = {
		15, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0
	};
	u8 base_div = 0;
	int done = 0;
	u32 i, j = 0;
	u32 hclk_div_setting = 0;

	for (j = 0; j < 0xf; i++) {
		for (i = 0; i < ARRAY_SIZE(hclk_masks); i++) {
			base_div = j * 16;
			if (max_hz >= (hclk_rate / ((i + 1) + base_div))) {
				
				done = 1;
				break;
			}
		}
			if (done)
				break;
	}

	debug("hclk=%d required=%d h_div %d, divisor is %d (mask %x) speed=%d\n",
		  hclk_rate, max_hz, j, i + 1, hclk_masks[i], hclk_rate / (i + 1 + base_div));

	hclk_div_setting = ((j << 4) | hclk_masks[i]);

	return hclk_div_setting;

}

static u32 aspeed_spi_hclk_divisor(struct aspeed_spi_priv *priv, u32 max_hz)
{
	u32 hclk_rate = priv->hclk_rate;
	/* HCLK/1 ..	HCLK/16 */
	const u8 hclk_masks[] = {
		15, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0
	};
	u32 i;
	u32 hclk_div_setting = 0;

	for (i = 0; i < ARRAY_SIZE(hclk_masks); i++) {
		if (max_hz >= (hclk_rate / (i + 1)))
			break;
	}
	debug("hclk=%d required=%d divisor is %d (mask %x) speed=%d\n",
	      hclk_rate, max_hz, i + 1, hclk_masks[i], hclk_rate / (i + 1));

	hclk_div_setting = hclk_masks[i];

	return hclk_div_setting;
}

/*
 * Use some address/size under the first flash device CE0
 */
static u32 aspeed_spi_fmc_checksum(struct aspeed_spi_priv *priv, u8 div,
				   u8 delay)
{
	u32 flash_addr = (u32)priv->ahb_base + 0x10000;
	u32 flash_len = 0x200;
	u32 dma_ctrl;
	u32 checksum;

	writel(flash_addr, &priv->regs->dma_flash_addr);
	writel(flash_len,  &priv->regs->dma_len);

	/*
	 * When doing calibration, the SPI clock rate in the CE0
	 * Control Register and the data input delay cycles in the
	 * Read Timing Compensation Register are replaced by bit[11:4].
	 */
	if(priv->new_ver)
		dma_ctrl = DMA_CTRL_ENABLE | DMA_CTRL_CKSUM | DMA_CTRL_CALIB |
			G6_TIMING_MASK(div, delay);
	else		
		dma_ctrl = DMA_CTRL_ENABLE | DMA_CTRL_CKSUM | DMA_CTRL_CALIB |
			TIMING_MASK(div, delay);
	writel(dma_ctrl, &priv->regs->dma_ctrl);

	while (!(readl(&priv->regs->intr_ctrl) & INTR_CTRL_DMA_STATUS))
		;

	writel(0x0, &priv->regs->intr_ctrl);

	checksum = readl(&priv->regs->dma_checksum);

	writel(0x0, &priv->regs->dma_ctrl);

	return checksum;
}

static u32 aspeed_spi_read_checksum(struct aspeed_spi_priv *priv, u8 div,
				    u8 delay)
{
	/* TODO(clg@kaod.org): the SPI controllers do not have the DMA
	 * registers. The algorithm is the same.
	 */
	if (!priv->is_fmc) {
		pr_warn("No timing calibration support for SPI controllers");
		return 0xbadc0de;
	}

	return aspeed_spi_fmc_checksum(priv, div, delay);
}

#define TIMING_DELAY_DI_4NS         BIT(3)
#define TIMING_DELAY_HCYCLE_MAX     5

static int aspeed_spi_timing_calibration(struct aspeed_spi_priv *priv)
{
	/* HCLK/5 .. HCLK/1 */
	const u8 hclk_masks[] = { 13, 6, 14, 7, 15 };
	u32 timing_reg = 0;
	u32 checksum, gold_checksum;
	int i, hcycle, delay_ns;

	debug("Read timing calibration :\n");

	/* Compute reference checksum at lowest freq HCLK/16 */
	gold_checksum = aspeed_spi_read_checksum(priv, 0, 0);

	/*
	 * Set CE0 Control Register to FAST READ command mode. The
	 * HCLK divisor will be set through the DMA Control Register.
	 */
	writel(CE_CTRL_CMD(0xb) | CE_CTRL_DUMMY(1) | CE_CTRL_FREADMODE,
	       &priv->regs->ce_ctrl[0]);

	/* Increase HCLK freq */
	if (priv->new_ver) {
		for (i = 0; i < ARRAY_SIZE(hclk_masks) - 1; i++) {
			u32 hdiv = 5 - i;
			u32 hshift = (hdiv - 2) * 8;
			bool pass = false;
			u8 delay;
			u16 first_delay = 0;
			u16 end_delay = 0;
			u32 cal_tmp;
			debug(" hdiv %d, hshift %d \n", hdiv, hshift);
			if (priv->hclk_rate / hdiv > priv->max_hz) {
				debug("skipping freq %ld\n", priv->hclk_rate / hdiv);
				continue;
			}

			/* Try without the 4ns DI delay */
			hcycle = delay = 0;
			debug("** Dealy Disable ** \n");
			checksum = aspeed_spi_read_checksum(priv, hclk_masks[i], delay);
			pass = (checksum == gold_checksum);
			debug(" HCLK/%d,  no DI delay, %d HCLK cycle : %s\n",
				  hdiv, hcycle, pass ? "PASS" : "FAIL");

			/* All good for this freq  */
			if (pass)
				goto next_div;

			/* Increase HCLK cycles until read succeeds */
			for (hcycle = 0; hcycle <= TIMING_DELAY_HCYCLE_MAX; hcycle++) {
				/* Try first with a 4ns DI delay */
				delay = TIMING_DELAY_DI_4NS | hcycle;
				debug("** Delay Enable : hcycle %x ** \n", hcycle);
				for (delay_ns = 0; delay_ns < 0xf; delay_ns++) {
					delay |= (delay_ns << 4);
					checksum = aspeed_spi_read_checksum(priv, hclk_masks[i],
									    delay);
					pass = (checksum == gold_checksum);
					debug(" HCLK/%d, 4ns DI delay, %d HCLK cycle, %d delay_ns : %s\n",
					      hdiv, hcycle, delay_ns, pass ? "PASS" : "FAIL");

					/* Try again with more HCLK cycles */
					if (!pass) {
						if (!first_delay)
							continue;
						else {
							end_delay = (hcycle << 4) | (delay_ns);
							end_delay = end_delay - 1;
							pass = 1;
							debug("find end_delay %x %d %d\n", end_delay, hcycle, delay_ns);
							break;
						}
					} else {
						if (!first_delay) {
							first_delay = (hcycle << 4) | delay_ns;
							debug("find first_delay %x %d %d\n", first_delay, hcycle, delay_ns);
						}
						if (!end_delay)
							pass = 0;
					}
				}
				
				if (pass) {
					cal_tmp = (first_delay + end_delay) / 2;
					delay = TIMING_DELAY_DI_4NS | ((cal_tmp & 0xf) << 4) | (cal_tmp >> 4);
					break;
				}
			}
next_div:
			if (pass) {
				timing_reg &= ~(0xfu << hshift);
				timing_reg |= delay << hshift;
				debug("timing_reg %x, delay %x, hshift bit %d\n",timing_reg, delay, hshift);
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(hclk_masks); i++) {
			u32 hdiv = 5 - i;
			u32 hshift = (hdiv - 1) << 2;
			bool pass = false;
			u8 delay;

			if (priv->hclk_rate / hdiv > priv->max_hz) {
				debug("skipping freq %ld\n", priv->hclk_rate / hdiv);
				continue;
			}

			/* Increase HCLK cycles until read succeeds */
			for (hcycle = 0; hcycle <= TIMING_DELAY_HCYCLE_MAX; hcycle++) {
				/* Try first with a 4ns DI delay */
				delay = TIMING_DELAY_DI_4NS | hcycle;
				checksum = aspeed_spi_read_checksum(priv, hclk_masks[i],
								    delay);
				pass = (checksum == gold_checksum);
				debug(" HCLK/%d, 4ns DI delay, %d HCLK cycle : %s\n",
				      hdiv, hcycle, pass ? "PASS" : "FAIL");

				/* Try again with more HCLK cycles */
				if (!pass)
					continue;

				/* Try without the 4ns DI delay */
				delay = hcycle;
				checksum = aspeed_spi_read_checksum(priv, hclk_masks[i],
								    delay);
				pass = (checksum == gold_checksum);
				debug(" HCLK/%d,  no DI delay, %d HCLK cycle : %s\n",
				      hdiv, hcycle, pass ? "PASS" : "FAIL");

				/* All good for this freq  */
				if (pass)
					break;
			}

			if (pass) {
				timing_reg &= ~(0xfu << hshift);
				timing_reg |= delay << hshift;
			}
		}
	}
	debug("Read Timing Compensation set to 0x%08x\n", timing_reg);
	writel(timing_reg, &priv->regs->timings);

	/* Reset CE0 Control Register */
	writel(0x0, &priv->regs->ce_ctrl[0]);

	return 0;
}

static int aspeed_spi_controller_init(struct aspeed_spi_priv *priv)
{
	int cs, ret;

	/*
	 * Enable write on all flash devices as USER command mode
	 * requires it.
	 */
	setbits_le32(&priv->regs->conf,
		     CONF_ENABLE_W2 | CONF_ENABLE_W1 | CONF_ENABLE_W0);

	/*
	 * Set the Read Timing Compensation Register. This setting
	 * applies to all devices.
	 */
	ret = aspeed_spi_timing_calibration(priv);
	if (ret)
		return ret;

	/*
	 * Set safe default settings for each device. These will be
	 * tuned after the SPI flash devices are probed.
	 */
	if (priv->new_ver) {
		for (cs = 0; cs < priv->flash_count; cs++) {
			struct aspeed_spi_flash *flash = &priv->flashes[cs];
			u32 seg_addr = readl(&priv->regs->segment_addr[cs]);
			u32 addr_config = 0;
			switch(cs) {
				case 0:
					flash->ahb_base = cs ? (void *)G6_SEGMENT_ADDR_START(seg_addr) :
						priv->ahb_base;
					debug("cs0 mem-map : %x \n", (u32)flash->ahb_base);
					break;
				case 1:
					flash->ahb_base = priv->flashes[0].ahb_base + 0x8000000;	//cs0 + 128Mb : use 64MB
					debug("cs1 mem-map : %x end %x \n", (u32)flash->ahb_base, (u32)flash->ahb_base + 0x4000000);
					addr_config = G6_SEGMENT_ADDR_VALUE((u32)flash->ahb_base, (u32)flash->ahb_base + 0x4000000); //add 128Mb
					writel(addr_config, &priv->regs->segment_addr[cs]);
					break;
				case 2:
					flash->ahb_base = priv->flashes[0].ahb_base + 0xc000000;	//cs0 + 192Mb : use 64MB
					debug("cs2 mem-map : %x end %x \n", (u32)flash->ahb_base, (u32)flash->ahb_base + 0x4000000);
					addr_config = G6_SEGMENT_ADDR_VALUE((u32)flash->ahb_base, (u32)flash->ahb_base + 0x4000000); //add 128Mb
					writel(addr_config, &priv->regs->segment_addr[cs]);
					break;
			}
			flash->cs = cs;
			flash->ce_ctrl_user = CE_CTRL_USERMODE;
			flash->ce_ctrl_fread = CE_CTRL_READMODE;
		}
	} else {
		for (cs = 0; cs < priv->flash_count; cs++) {
			struct aspeed_spi_flash *flash = &priv->flashes[cs];
			u32 seg_addr = readl(&priv->regs->segment_addr[cs]);
			/*
			 * The start address of the AHB window of CE0 is
			 * read-only and is the same as the address of the
			 * overall AHB window of the controller for all flash
			 * devices.
			 */
			flash->ahb_base = cs ? (void *)SEGMENT_ADDR_START(seg_addr) :
				priv->ahb_base;

			flash->cs = cs;
			flash->ce_ctrl_user = CE_CTRL_USERMODE;
			flash->ce_ctrl_fread = CE_CTRL_READMODE;
		}
	}
	return 0;
}

static int aspeed_spi_read_from_ahb(void __iomem *ahb_base, void *buf,
				    size_t len)
{
	size_t offset = 0;

	if (!((uintptr_t)buf % 4)) {
		readsl(ahb_base, buf, len >> 2);
		offset = len & ~0x3;
		len -= offset;
	}
	readsb(ahb_base, (u8 *)buf + offset, len);

	return 0;
}

static int aspeed_spi_write_to_ahb(void __iomem *ahb_base, const void *buf,
				   size_t len)
{
	size_t offset = 0;

	if (!((uintptr_t)buf % 4)) {
		writesl(ahb_base, buf, len >> 2);
		offset = len & ~0x3;
		len -= offset;
	}
	writesb(ahb_base, (u8 *)buf + offset, len);

	return 0;
}

static void aspeed_spi_start_user(struct aspeed_spi_priv *priv,
				  struct aspeed_spi_flash *flash)
{
	u32 ctrl_reg = flash->ce_ctrl_user | CE_CTRL_STOP_ACTIVE;

	/* Deselect CS and set USER command mode */
	writel(ctrl_reg, &priv->regs->ce_ctrl[flash->cs]);

	/* Select CS */
	clrbits_le32(&priv->regs->ce_ctrl[flash->cs], CE_CTRL_STOP_ACTIVE);
}

static void aspeed_spi_stop_user(struct aspeed_spi_priv *priv,
				 struct aspeed_spi_flash *flash)
{
	/* Deselect CS first */
	setbits_le32(&priv->regs->ce_ctrl[flash->cs], CE_CTRL_STOP_ACTIVE);

	/* Restore default command mode */
	writel(flash->ce_ctrl_fread, &priv->regs->ce_ctrl[flash->cs]);
}

static int aspeed_spi_read_reg(struct aspeed_spi_priv *priv,
			       struct aspeed_spi_flash *flash,
			       u8 opcode, u8 *read_buf, int len)
{
	aspeed_spi_start_user(priv, flash);
	aspeed_spi_write_to_ahb(flash->ahb_base, &opcode, 1);
	aspeed_spi_read_from_ahb(flash->ahb_base, read_buf, len);
	aspeed_spi_stop_user(priv, flash);

	return 0;
}

static int aspeed_spi_write_reg(struct aspeed_spi_priv *priv,
				struct aspeed_spi_flash *flash,
				u8 opcode, const u8 *write_buf, int len)
{
	aspeed_spi_start_user(priv, flash);
	aspeed_spi_write_to_ahb(flash->ahb_base, &opcode, 1);
	aspeed_spi_write_to_ahb(flash->ahb_base, write_buf, len);
	aspeed_spi_stop_user(priv, flash);

	debug("=== write opcode [%x] ==== \n", opcode);
	switch(opcode) {
		case SPINOR_OP_EN4B:
			/* For ast2600, if 2 chips ABR mode is enabled,
			 * turn on 3B mode auto clear in order to avoid
			 * the scenario where spi controller is at 4B mode
			 * and flash site is at 3B mode after 3rd switch.
			 */
			if (priv->new_ver == 1 && (readl(SPI_3B_AUTO_CLR_REG) & SPI_3B_AUTO_CLR))
				writel(readl(&priv->regs->soft_rst_cmd_ctrl) | SOFT_RST_CMD_EN,
						&priv->regs->soft_rst_cmd_ctrl);

			writel(readl(&priv->regs->ctrl) | BIT(flash->cs), &priv->regs->ctrl);
			break;
		case SPINOR_OP_EX4B:
			writel(readl(&priv->regs->ctrl) & ~BIT(flash->cs), &priv->regs->ctrl);
			break;
	}
	return 0;
}

static void aspeed_spi_send_cmd_addr(struct aspeed_spi_priv *priv,
				     struct aspeed_spi_flash *flash,
				     const u8 *cmdbuf, unsigned int cmdlen)
{
	int i;
	u8 byte0 = 0x0;
	u8 addrlen = cmdlen - 1;

	/* First, send the opcode */
	aspeed_spi_write_to_ahb(flash->ahb_base, &cmdbuf[0], 1);

	if(flash->iomode == CE_CTRL_IO_QUAD_ADDR_DATA)
		writel(flash->ce_ctrl_user | flash->iomode, &priv->regs->ce_ctrl[flash->cs]);

	/*
	 * The controller is configured for 4BYTE address mode. Fix
	 * the address width and send an extra byte if the SPI Flash
	 * layer uses 3 bytes addresses.
	 */
	if (addrlen == 3 && readl(&priv->regs->ctrl) & BIT(flash->cs))
		aspeed_spi_write_to_ahb(flash->ahb_base, &byte0, 1);

	/* Then the address */
	for (i = 1 ; i < cmdlen; i++)
		aspeed_spi_write_to_ahb(flash->ahb_base, &cmdbuf[i], 1);
}

static ssize_t aspeed_spi_read_user(struct aspeed_spi_priv *priv,
				    struct aspeed_spi_flash *flash,
				    unsigned int cmdlen, const u8 *cmdbuf,
				    unsigned int len, u8 *read_buf)
{
	u8 dummy = 0xff;
	int i;

	aspeed_spi_start_user(priv, flash);

	/* cmd buffer = cmd + addr + dummies */
	aspeed_spi_send_cmd_addr(priv, flash, cmdbuf,
				 cmdlen - (flash->spi->read_dummy/8));

	for (i = 0 ; i < (flash->spi->read_dummy/8); i++)
		aspeed_spi_write_to_ahb(flash->ahb_base, &dummy, 1);

	if (flash->iomode) {
		clrbits_le32(&priv->regs->ce_ctrl[flash->cs],
			     CE_CTRL_IO_MODE_MASK);
		setbits_le32(&priv->regs->ce_ctrl[flash->cs], flash->iomode);
	}

	aspeed_spi_read_from_ahb(flash->ahb_base, read_buf, len);
	aspeed_spi_stop_user(priv, flash);

	return 0;
}

static ssize_t aspeed_spi_write_user(struct aspeed_spi_priv *priv,
				     struct aspeed_spi_flash *flash,
				     unsigned int cmdlen, const u8 *cmdbuf,
				     unsigned int len,	const u8 *write_buf)
{
	aspeed_spi_start_user(priv, flash);

	/* cmd buffer = cmd + addr : normally cmd is use signle mode*/
	aspeed_spi_send_cmd_addr(priv, flash, cmdbuf, cmdlen);

	/* data will use io mode */
	if(flash->iomode == CE_CTRL_IO_QUAD_DATA)
		writel(flash->ce_ctrl_user | flash->iomode, &priv->regs->ce_ctrl[flash->cs]);

	aspeed_spi_write_to_ahb(flash->ahb_base, write_buf, len);

	aspeed_spi_stop_user(priv, flash);

	return 0;
}

static u32 aspeed_spi_flash_to_addr(struct aspeed_spi_flash *flash,
				    const u8 *cmdbuf, unsigned int cmdlen)
{
	u8 addrlen = cmdlen - 1;
	u32 addr = (cmdbuf[1] << 16) | (cmdbuf[2] << 8) | cmdbuf[3];

	/*
	 * U-Boot SPI Flash layer uses 3 bytes addresses, but it might
	 * change one day
	 */
	if (addrlen == 4)
		addr = (addr << 8) | cmdbuf[4];

	return addr;
}

/* TODO(clg@kaod.org): add support for XFER_MMAP instead ? */
static ssize_t aspeed_spi_read(struct aspeed_spi_priv *priv,
			       struct aspeed_spi_flash *flash,
			       unsigned int cmdlen, const u8 *cmdbuf,
			       unsigned int len, u8 *read_buf)
{
	/* cmd buffer = cmd + addr + dummies */
	u32 offset = aspeed_spi_flash_to_addr(flash, cmdbuf,
					      cmdlen - (flash->spi->read_dummy/8));

	/*
	 * Switch to USER command mode if the AHB window configured
	 * for the device is too small for the read operation
	 */
	if (offset + len >= flash->ahb_size) {
		return aspeed_spi_read_user(priv, flash, cmdlen, cmdbuf,
					    len, read_buf);
	}

	memcpy_fromio(read_buf, flash->ahb_base + offset, len);

	return 0;
}

static int aspeed_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct aspeed_spi_flash *flash;
	u8 *cmd_buf = priv->cmd_buf;
	size_t data_bytes;
	int err = 0;

	flash = aspeed_spi_get_flash(dev);
	if (!flash)
		return -ENXIO;

	if (flags & SPI_XFER_BEGIN) {
		/* save command in progress */
		priv->cmd_len = bitlen / 8;
		memcpy(cmd_buf, dout, priv->cmd_len);
	}

	if (flags == (SPI_XFER_BEGIN | SPI_XFER_END)) {
		/* if start and end bit are set, the data bytes is 0. */
		data_bytes = 0;
	} else {
		data_bytes = bitlen / 8;
	}

	debug("CS%u: %s cmd %zu bytes data %zu bytes\n", flash->cs,
	      din ? "read" : "write", priv->cmd_len, data_bytes);

	if ((flags & SPI_XFER_END) || flags == 0) {
		if (priv->cmd_len == 0) {
			pr_err("No command is progress !\n");
			return -1;
		}

		if (din && data_bytes) {
			if (priv->cmd_len == 1)
				err = aspeed_spi_read_reg(priv, flash,
							  cmd_buf[0],
							  din, data_bytes);
			else
				err = aspeed_spi_read(priv, flash,
						      priv->cmd_len,
						      cmd_buf, data_bytes,
						      din);
		} else if (dout) {
			if (priv->cmd_len == 1)
				err = aspeed_spi_write_reg(priv, flash,
							   cmd_buf[0],
							   dout, data_bytes);
			else
				err = aspeed_spi_write_user(priv, flash,
							    priv->cmd_len,
							    cmd_buf, data_bytes,
							    dout);
		}

		if (flags & SPI_XFER_END) {
			/* clear command */
			memset(cmd_buf, 0, sizeof(priv->cmd_buf));
			priv->cmd_len = 0;
		}
	}

	return err;
}

static int aspeed_spi_child_pre_probe(struct udevice *dev)
{
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

	debug("pre_probe slave device on CS%u, max_hz %u, mode 0x%x.\n",
	      slave_plat->cs, slave_plat->max_hz, slave_plat->mode);

	if (!aspeed_spi_get_flash(dev))
		return -ENXIO;

	return 0;
}

/*
 * It is possible to automatically define a contiguous address space
 * on top of all CEs in the AHB window of the controller but it would
 * require much more work. Let's start with a simple mapping scheme
 * which should work fine for a single flash device.
 *
 * More complex schemes should probably be defined with the device
 * tree.
 */
static int aspeed_spi_flash_set_segment(struct aspeed_spi_priv *priv,
					struct aspeed_spi_flash *flash)
{
	u32 seg_addr;

	/* could be configured through the device tree */
	flash->ahb_size = flash->spi->size;

	if (priv->new_ver) {
		seg_addr = G6_SEGMENT_ADDR_VALUE((u32)flash->ahb_base,
					      (u32)flash->ahb_base + flash->ahb_size);
	} else {
		seg_addr = SEGMENT_ADDR_VALUE((u32)flash->ahb_base,
						  (u32)flash->ahb_base + flash->ahb_size);
	}
	writel(seg_addr, &priv->regs->segment_addr[flash->cs]);

	return 0;
}

static int aspeed_spi_flash_init(struct aspeed_spi_priv *priv,
				 struct aspeed_spi_flash *flash,
				 struct udevice *dev)
{
	struct spi_flash *spi_flash = dev_get_uclass_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	u32 read_hclk;

	/*
	 * The SPI flash device slave should not change, so initialize
	 * it only once.
	 */
	if (flash->init)
		return 0;

	/*
	 * The flash device has not been probed yet. Initial transfers
	 * to read the JEDEC of the device will use the initial
	 * default settings of the registers.
	 */
	if (!spi_flash->name)
		return 0;

	debug("CS%u: init %s flags:%x size:%d page:%d sector:%d erase:%d "
	      "cmds [ erase:%x read=%x write:%x ] dummy:%d\n",
	      flash->cs,
	      spi_flash->name, spi_flash->flags, spi_flash->size,
	      spi_flash->page_size, spi_flash->sector_size,
	      spi_flash->erase_size, spi_flash->erase_opcode,
	      spi_flash->read_opcode, spi_flash->program_opcode,
	      spi_flash->read_dummy);

	flash->spi = spi_flash;

	flash->ce_ctrl_user = CE_CTRL_USERMODE;

	if(priv->new_ver)
		read_hclk = aspeed_g6_spi_hclk_divisor(priv, slave->speed);
	else
		read_hclk = aspeed_spi_hclk_divisor(priv, slave->speed);

	switch(flash->spi->read_opcode) {
		case SPINOR_OP_READ_1_1_2:
		case SPINOR_OP_READ_1_1_2_4B:
			flash->iomode = CE_CTRL_IO_DUAL_DATA;
			break;
		case SPINOR_OP_READ_1_1_4:
		case SPINOR_OP_READ_1_1_4_4B:
			flash->iomode = CE_CTRL_IO_QUAD_DATA;
			break;
		case SPINOR_OP_READ_1_4_4:
		case SPINOR_OP_READ_1_4_4_4B:
			flash->iomode = CE_CTRL_IO_QUAD_ADDR_DATA;
			printf("need modify dummy for 3 bytes");
			break;
	}

	if(priv->new_ver) {
		flash->ce_ctrl_fread = CE_G6_CTRL_CLOCK_FREQ(read_hclk) |
			flash->iomode |
			CE_CTRL_CMD(flash->spi->read_opcode) |
			CE_CTRL_DUMMY((flash->spi->read_dummy/8)) |
			CE_CTRL_FREADMODE;
	} else {
		flash->ce_ctrl_fread = CE_CTRL_CLOCK_FREQ(read_hclk) |
			flash->iomode |
			CE_CTRL_CMD(flash->spi->read_opcode) |
			CE_CTRL_DUMMY((flash->spi->read_dummy/8)) |
			CE_CTRL_FREADMODE;
	}

	debug("CS%u: USER mode 0x%08x FREAD mode 0x%08x\n", flash->cs,
	      flash->ce_ctrl_user, flash->ce_ctrl_fread);

	/* Set the CE Control Register default (FAST READ) */
	writel(flash->ce_ctrl_fread, &priv->regs->ce_ctrl[flash->cs]);

	/* Set Address Segment Register for direct AHB accesses */
	aspeed_spi_flash_set_segment(priv, flash);

	/* All done */
	flash->init = true;

	return 0;
}

static int aspeed_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	struct aspeed_spi_flash *flash;

	debug("%s: claim bus CS%u\n", bus->name, slave_plat->cs);

	flash = aspeed_spi_get_flash(dev);
	if (!flash)
		return -ENODEV;

	return aspeed_spi_flash_init(priv, flash, dev);
}

static int aspeed_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

	debug("%s: release bus CS%u\n", bus->name, slave_plat->cs);

	if (!aspeed_spi_get_flash(dev))
		return -ENODEV;

	return 0;
}

static int aspeed_spi_set_mode(struct udevice *bus, uint mode)
{
	debug("%s: setting mode to %x\n", bus->name, mode);

	if (mode & (SPI_RX_QUAD | SPI_TX_QUAD)) {
#ifndef CONFIG_ASPEED_AST2600
		pr_err("%s invalid QUAD IO mode\n", bus->name);
		return -EINVAL;
#endif
	}

	/* The CE Control Register is set in claim_bus() */
	return 0;
}

static int aspeed_spi_set_speed(struct udevice *bus, uint hz)
{
	debug("%s: setting speed to %u\n", bus->name, hz);

	/* The CE Control Register is set in claim_bus() */
	return 0;
}

static int aspeed_spi_count_flash_devices(struct udevice *bus)
{
	ofnode node;
	int count = 0;

	dev_for_each_subnode(node, bus) {
		if (ofnode_is_available(node) &&
		    ofnode_device_is_compatible(node, "spi-flash"))
			count++;
	}

	return count;
}

static int aspeed_spi_bind(struct udevice *bus)
{
	debug("%s assigned req_seq=%d seq=%d\n", bus->name, bus->req_seq,
	      bus->seq);

	return 0;
}

static int aspeed_spi_probe(struct udevice *bus)
{
	struct resource res_regs, res_ahb;
	struct aspeed_spi_priv *priv = dev_get_priv(bus);
	struct clk hclk;
	int ret;

	ret = dev_read_resource(bus, 0, &res_regs);
	if (ret < 0)
		return ret;

	priv->regs = (void __iomem *)res_regs.start;

	ret = dev_read_resource(bus, 1, &res_ahb);
	if (ret < 0)
		return ret;

	priv->ahb_base = (void __iomem *)res_ahb.start;
	priv->ahb_size = res_ahb.end - res_ahb.start;

	ret = clk_get_by_index(bus, 0, &hclk);
	if (ret < 0) {
		pr_err("%s could not get clock: %d\n", bus->name, ret);
		return ret;
	}

	priv->hclk_rate = clk_get_rate(&hclk);
	clk_free(&hclk);

	priv->max_hz = dev_read_u32_default(bus, "spi-max-frequency",
					    100000000);

	priv->num_cs = dev_read_u32_default(bus, "num-cs", ASPEED_SPI_MAX_CS);

	priv->flash_count = aspeed_spi_count_flash_devices(bus);
	if (priv->flash_count > priv->num_cs) {
		pr_err("%s has too many flash devices: %d\n", bus->name,
		       priv->flash_count);
		return -EINVAL;
	}

	if (!priv->flash_count) {
		pr_err("%s has no flash devices ?!\n", bus->name);
		return -ENODEV;
	}

	if (device_is_compatible(bus, "aspeed,ast2600-fmc") || 
			device_is_compatible(bus, "aspeed,ast2600-spi")) {
		priv->new_ver = 1;
	}

	/*
	 * There are some slight differences between the FMC and the
	 * SPI controllers
	 */
	priv->is_fmc = dev_get_driver_data(bus);

	ret = aspeed_spi_controller_init(priv);
	if (ret)
		return ret;

	debug("%s probed regs=%p ahb_base=%p max-hz=%d cs=%d seq=%d\n",
	      bus->name, priv->regs, priv->ahb_base, priv->max_hz,
	      priv->flash_count, bus->seq);

	return 0;
}

static const struct dm_spi_ops aspeed_spi_ops = {
	.claim_bus	= aspeed_spi_claim_bus,
	.release_bus	= aspeed_spi_release_bus,
	.set_mode	= aspeed_spi_set_mode,
	.set_speed	= aspeed_spi_set_speed,
	.xfer		= aspeed_spi_xfer,
};

static const struct udevice_id aspeed_spi_ids[] = {
	{ .compatible = "aspeed,ast2600-fmc", .data = 1 },
	{ .compatible = "aspeed,ast2600-spi", .data = 0 },
	{ .compatible = "aspeed,ast2500-fmc", .data = 1 },
	{ .compatible = "aspeed,ast2500-spi", .data = 0 },
	{ }
};

U_BOOT_DRIVER(aspeed_spi) = {
	.name = "aspeed_spi",
	.id = UCLASS_SPI,
	.of_match = aspeed_spi_ids,
	.ops = &aspeed_spi_ops,
	.priv_auto_alloc_size = sizeof(struct aspeed_spi_priv),
	.child_pre_probe = aspeed_spi_child_pre_probe,
	.bind  = aspeed_spi_bind,
	.probe = aspeed_spi_probe,
};
