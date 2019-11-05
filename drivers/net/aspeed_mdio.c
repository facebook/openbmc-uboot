/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <reset.h>
#include <linux/errno.h>

/*
 * PHY control register
 */
#define FTGMAC100_PHYCR_MDC_CYCTHR_MASK	0x3f
#define FTGMAC100_PHYCR_MDC_CYCTHR(x)	((x) & 0x3f)
#define FTGMAC100_PHYCR_PHYAD(x)	(((x) & 0x1f) << 16)
#define FTGMAC100_PHYCR_REGAD(x)	(((x) & 0x1f) << 21)
#define FTGMAC100_PHYCR_MIIRD		(1 << 26)
#define FTGMAC100_PHYCR_MIIWR		(1 << 27)

#ifdef CONFIG_ASPEED_AST2600
//G6 MDC/MDIO 
#define FTGMAC100_PHYCR_NEW_FIRE		BIT(31)
#define FTGMAC100_PHYCR_ST_22			BIT(28)
#define FTGMAC100_PHYCR_NEW_WRITE		BIT(26)
#define FTGMAC100_PHYCR_NEW_READ		BIT(27)
#define FTGMAC100_PHYCR_NEW_WDATA(x)	(x & 0xffff)
#define FTGMAC100_PHYCR_NEW_PHYAD(x)	(((x) & 0x1f) << 21)
#define FTGMAC100_PHYCR_NEW_REGAD(x)	(((x) & 0x1f) << 16)
#else
//New MDC/MDIO 
#define FTGMAC100_PHYCR_NEW_FIRE		BIT(15)
#define FTGMAC100_PHYCR_ST_22			BIT(12)
#define FTGMAC100_PHYCR_NEW_WRITE		BIT(10)
#define FTGMAC100_PHYCR_NEW_READ		BIT(11)
#define FTGMAC100_PHYCR_NEW_WDATA(x)	((x & 0xffff) << 16)
#define FTGMAC100_PHYCR_NEW_PHYAD(x)	(((x) & 0x1f) << 5)
#define FTGMAC100_PHYCR_NEW_REGAD(x)	((x) & 0x1f)
#endif

/*
 * PHY data register
 */
#define FTGMAC100_PHYDATA_MIIWDATA(x)		((x) & 0xffff)
#define FTGMAC100_PHYDATA_MIIRDATA(phydata)	(((phydata) >> 16) & 0xffff)

#define FTGMAC100_PHYDATA_NEW_MIIWDATA(x)		((x) & 0xffff)

struct aspeed_mdio_regs {
	unsigned int	phycr;
	unsigned int	phydata;
};

extern int aspeed_mdio_read(struct mii_dev *bus, int phy_addr, int dev_addr, 
					int reg_addr)
{
	struct aspeed_mdio_regs __iomem *mdio_regs = (struct aspeed_mdio_regs __iomem *)bus->priv;
	u32 phycr;
	int i;

#if 1
	phycr = FTGMAC100_PHYCR_NEW_FIRE | FTGMAC100_PHYCR_ST_22 | FTGMAC100_PHYCR_NEW_READ |
			FTGMAC100_PHYCR_NEW_PHYAD(phy_addr) | 
			FTGMAC100_PHYCR_NEW_REGAD(reg_addr);

	writel(phycr, &mdio_regs->phycr);
	mb();

	for (i = 0; i < 10; i++) {
		phycr = readl(&mdio_regs->phycr);

		if ((phycr & FTGMAC100_PHYCR_NEW_FIRE) == 0) {
			u32 data;

			data = readl(&mdio_regs->phydata);
			return FTGMAC100_PHYDATA_NEW_MIIWDATA(data);
		}

		mdelay(10);
	}
#else
	phycr = readl(&mdio_regs->phycr);

	/* preserve MDC cycle threshold */
//	phycr &= FTGMAC100_PHYCR_MDC_CYCTHR_MASK;

	phycr = FTGMAC100_PHYCR_PHYAD(addr)
		  |  FTGMAC100_PHYCR_REGAD(reg)
		  |  FTGMAC100_PHYCR_MIIRD | 0x34;

	writel(phycr, &mdio_regs->phycr);

	for (i = 0; i < 10; i++) {
		phycr = readl(&mdio_regs->phycr);

		if ((phycr & FTGMAC100_PHYCR_MIIRD) == 0) {
			int data;

			data = readl(&mdio_regs->phydata);
			return FTGMAC100_PHYDATA_MIIRDATA(data);
		}

		mdelay(10);
	}
#endif
	debug("mdio read timed out\n");
	return -1;

}

extern int aspeed_mdio_write(struct mii_dev *bus, int phy_addr, int dev_addr,
				int reg_addr, u16 value)
{
	struct aspeed_mdio_regs __iomem *mdio_regs = (struct aspeed_mdio_regs __iomem *)bus->priv;
	int phycr;
	int i;
	
#ifdef CONFIG_MACH_ASPEED_G4
	int data;

	phycr = readl(&mdio_regs->phycr);

	/* preserve MDC cycle threshold */
//	phycr &= FTGMAC100_PHYCR_MDC_CYCTHR_MASK;

	phycr = FTGMAC100_PHYCR_PHYAD(phy_addr)
		  |  FTGMAC100_PHYCR_REGAD(reg_addr)
		  |  FTGMAC100_PHYCR_MIIWR | 0x34;

	data = FTGMAC100_PHYDATA_MIIWDATA(value);

	writel(data, &mdio_regs->phydata);
	writel(phycr, &mdio_regs->phycr);

	for (i = 0; i < 10; i++) {
		phycr = readl(&mdio_regs->phycr);

		if ((phycr & FTGMAC100_PHYCR_MIIWR) == 0) {
			debug("(phycr & FTGMAC100_PHYCR_MIIWR) == 0: " \
				"phy_addr: %x\n", phy_addr);
			return 0;
		}

		mdelay(1);
	}
#else

	phycr = FTGMAC100_PHYCR_NEW_WDATA(value) |
			FTGMAC100_PHYCR_NEW_FIRE | FTGMAC100_PHYCR_ST_22 |
			FTGMAC100_PHYCR_NEW_WRITE |
			FTGMAC100_PHYCR_NEW_PHYAD(phy_addr) |
			FTGMAC100_PHYCR_NEW_REGAD(reg_addr);

	writel(phycr, &mdio_regs->phycr);
	mb();

	for (i = 0; i < 10; i++) {
		phycr = readl(&mdio_regs->phycr);

		if ((phycr & FTGMAC100_PHYCR_NEW_FIRE) == 0) {
			debug("(phycr & FTGMAC100_PHYCR_MIIWR) == 0: " \
				"phy_addr: %x\n", phy_addr);
			return 0;
		}

		mdelay(10);
	}
#endif
	debug("mdio write timed out\n");

	return -1;
}

static int aspeed_mdio_probe(struct udevice *dev)
{
//	struct mii_dev *bus = (struct mii_dev *)dev_get_uclass_platdata(dev);
	struct reset_ctl reset_ctl;
	int ret = 0;

	debug("%s(dev=%p) \n", __func__, dev);

	ret = reset_get_by_index(dev, 0, &reset_ctl);

	if (ret) {
		printf("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	reset_assert(&reset_ctl);
	reset_deassert(&reset_ctl);

	return 0;
}

static const struct udevice_id aspeed_mdio_ids[] = {
	{ .compatible = "aspeed,aspeed-mdio" },
	{ }
};

U_BOOT_DRIVER(aspeed_mdio) = {
	.name		= "aspeed_mdio",
	.id			= UCLASS_MDIO,
	.of_match	= aspeed_mdio_ids,
	.probe		= aspeed_mdio_probe,
	.priv_auto_alloc_size = sizeof(struct mii_dev),
};


