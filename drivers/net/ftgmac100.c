/*
 * Faraday FTGMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * (C) Copyright 2010 Andes Technology
 * Macpaul Lin <macpaul@andestech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <linux/mii.h>

#include <i2c.h>

#include <asm/arch/ast_scu.h>
#include <asm/arch/aspeed.h>

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#include <miiphy.h>
#endif


#include "ftgmac100.h"

#define ETH_ZLEN	60

/* RBSR - hw default init value is also 0x640 */
#define RBSR_DEFAULT_VALUE	0x640

/* PKTBUFSTX/PKTBUFSRX must both be power of 2 */
#define PKTBUFSTX	4	/* must be power of 2 */

struct ftgmac100_data {
	struct ftgmac100_txdes txdes[PKTBUFSTX];
	struct ftgmac100_rxdes rxdes[PKTBUFSRX];
	int tx_index;
	int rx_index;
	int phy_addr;
};

/*
 * struct mii_bus functions
 */
static int ftgmac100_mdiobus_read(struct eth_device *dev, int phy_addr,
	int regnum)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	int fear0;
	int phycr;
	int i;

#ifdef AST_MAC_OWN
	if (__raw_readl(&ftgmac100->physts) & FTGMAC100_PHY_STS_UNVALID) {
		printf("owner \n");
	} else {
		printf("not owner \n");
		__raw_writel(__raw_readl(&ftgmac100->physts) | FTGMAC100_PHY_REQ_EN, &ftgmac100->physts);
		while(!(__raw_readl(&ftgmac100->isr) & FTGMAC100_INT_FLAG_ACK)) {
			mdelay(10);
			i++;
			if(i > 1000) {
				printf("time out \n");
				break;
			}
		}
		__raw_writel(__raw_readl(&ftgmac100->physts) | FTGMAC100_PHY_POLL, &ftgmac100->physts);
		__raw_writel(__raw_readl(&ftgmac100->physts) & ~FTGMAC100_PHY_REQ_EN, &ftgmac100->physts);
		__raw_writel(FTGMAC100_INT_FLAG_ACK, &ftgmac100->isr);

	}
#endif

	fear0 = __raw_readl(&ftgmac100->fear0);
	if(fear0 & (1 << 31)) {	//New MDC/MDIO
		phycr = FTGMAC100_PHYCR_NEW_FIRE | FTGMAC100_PHYCR_ST_22 | FTGMAC100_PHYCR_NEW_READ |
				FTGMAC100_PHYCR_NEW_PHYAD(phy_addr) | // 20141114
				FTGMAC100_PHYCR_NEW_REGAD(regnum); // 20141114

		__raw_writel(phycr, &ftgmac100->phycr);

		for (i = 0; i < 10; i++) {
			phycr = __raw_readl(&ftgmac100->phycr);

			if ((phycr & FTGMAC100_PHYCR_NEW_FIRE) == 0) {
				int data;

				data = __raw_readl(&ftgmac100->phydata);
				return FTGMAC100_PHYDATA_NEW_MIIWDATA(data);
			}

			mdelay(10);
		}

		debug("mdio read timed out\n");
		return -1;

	} else {
		phycr = __raw_readl(&ftgmac100->phycr);

		/* preserve MDC cycle threshold */
	//	phycr &= FTGMAC100_PHYCR_MDC_CYCTHR_MASK;

		phycr = FTGMAC100_PHYCR_PHYAD(phy_addr)
		      |  FTGMAC100_PHYCR_REGAD(regnum)
		      |  FTGMAC100_PHYCR_MIIRD | 0x34;

		__raw_writel(phycr, &ftgmac100->phycr);

		for (i = 0; i < 10; i++) {
			phycr = __raw_readl(&ftgmac100->phycr);

			if ((phycr & FTGMAC100_PHYCR_MIIRD) == 0) {
				int data;

				data = __raw_readl(&ftgmac100->phydata);
				return FTGMAC100_PHYDATA_MIIRDATA(data);
			}

			mdelay(10);
		}

		debug("mdio read timed out\n");
		return -1;
	}
}

static int ftgmac100_mdiobus_write(struct eth_device *dev, int phy_addr,
	int regnum, u16 value)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	int fear0;
	int phycr;
	int data;
	int i;

#ifdef AST_MAC_OWN
	if (__raw_readl(&ftgmac100->physts) & FTGMAC100_PHY_STS_UNVALID) {
		printf("owner \n");
	} else {
		printf("not owner \n");
		__raw_writel(__raw_readl(&ftgmac100->physts) | FTGMAC100_PHY_REQ_EN, &ftgmac100->physts);
		while(!(__raw_readl(&ftgmac100->isr) & FTGMAC100_INT_FLAG_ACK)) {
			mdelay(10);
			i++;
			if(i > 1000) {
				printf("time out \n");
				break;
			}
		}
		__raw_writel(__raw_readl(&ftgmac100->physts) | FTGMAC100_PHY_POLL, &ftgmac100->physts);
		__raw_writel(__raw_readl(&ftgmac100->physts) & ~FTGMAC100_PHY_REQ_EN, &ftgmac100->physts);
		__raw_writel(FTGMAC100_INT_FLAG_ACK, &ftgmac100->isr);

	}
#endif

	fear0 = __raw_readl(&ftgmac100->fear0);
	if(fear0 & (1 << 31)) {	//New MDC/MDIO
		phycr = (value << 16) |
				FTGMAC100_PHYCR_NEW_FIRE | FTGMAC100_PHYCR_ST_22 |
				FTGMAC100_PHYCR_NEW_WRITE |
				FTGMAC100_PHYCR_NEW_PHYAD(phy_addr) | // 20141114
				FTGMAC100_PHYCR_NEW_REGAD(regnum); // 20141114

		__raw_writel(phycr, &ftgmac100->phycr);

		for (i = 0; i < 10; i++) {
			phycr = __raw_readl(&ftgmac100->phycr);

			if ((phycr & FTGMAC100_PHYCR_NEW_FIRE) == 0) {
				debug("(phycr & FTGMAC100_PHYCR_MIIWR) == 0: " \
					"phy_addr: %x\n", phy_addr);
				return 0;
			}

			mdelay(10);
		}

		debug("mdio write timed out\n");
		return -1;

	} else {

		phycr = __raw_readl(&ftgmac100->phycr);

		/* preserve MDC cycle threshold */
	//	phycr &= FTGMAC100_PHYCR_MDC_CYCTHR_MASK;

		phycr = FTGMAC100_PHYCR_PHYAD(phy_addr)
		      |  FTGMAC100_PHYCR_REGAD(regnum)
		      |  FTGMAC100_PHYCR_MIIWR | 0x34;

		data = FTGMAC100_PHYDATA_MIIWDATA(value);

		__raw_writel(data, &ftgmac100->phydata);
		__raw_writel(phycr, &ftgmac100->phycr);

		for (i = 0; i < 10; i++) {
			phycr = __raw_readl(&ftgmac100->phycr);

			if ((phycr & FTGMAC100_PHYCR_MIIWR) == 0) {
				debug("(phycr & FTGMAC100_PHYCR_MIIWR) == 0: " \
					"phy_addr: %x\n", phy_addr);
				return 0;
			}

			mdelay(1);
		}

		debug("mdio write timed out\n");
		return -1;
	}
}

int ftgmac100_phy_read(struct eth_device *dev, int addr, int reg, u16 *value)
{
	*value = ftgmac100_mdiobus_read(dev , addr, reg);

	if (*value == -1)
		return -1;

	return 0;
}

int  ftgmac100_phy_write(struct eth_device *dev, int addr, int reg, u16 value)
{
	if (ftgmac100_mdiobus_write(dev, addr, reg, value) == -1)
		return -1;

	return 0;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int ftgmac100_reg_read(const char *devname, u8 phy_addr, u8 phy_reg,
			u16 *value)
{
	struct eth_device *dev = eth_get_dev_by_name(devname);
	*value = ftgmac100_mdiobus_read(dev , phy_addr, phy_reg);

	if (*value == -1)
		return -1;

	return 0;

}

static int ftgmac100_reg_write(const char *devname, u8 phy_addr, u8 phy_reg, u16 value)
{
	struct eth_device *dev = eth_get_dev_by_name(devname);
	if (ftgmac100_mdiobus_write(dev, phy_addr, phy_reg, value) == -1)
		return -1;

	return 0;
}
#endif

static int ftgmac100_phy_reset(struct eth_device *dev)
{
	struct ftgmac100_data *priv = dev->priv;
	int i;
	u16 status, adv;

	adv = ADVERTISE_CSMA | ADVERTISE_ALL;

	ftgmac100_phy_write(dev, priv->phy_addr, MII_ADVERTISE, adv);

	printf("%s: Starting autonegotiation...\n", dev->name);

	ftgmac100_phy_write(dev, priv->phy_addr,
		MII_BMCR, (BMCR_ANENABLE | BMCR_ANRESTART));

	for (i = 0; i < 100000 / 100; i++) {
		ftgmac100_phy_read(dev, priv->phy_addr, MII_BMSR, &status);

		if (status & BMSR_ANEGCOMPLETE)
			break;
		mdelay(1);
	}

	if (status & BMSR_ANEGCOMPLETE) {
		printf("%s: Autonegotiation complete\n", dev->name);
	} else {
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       dev->name, status);
		return 0;
	}

	return 1;
}

static int ftgmac100_phy_init(struct eth_device *dev)
{
	struct ftgmac100_data *priv = dev->priv;

	int phy_addr;
//	u32 phy_id;
	u16 phy_id1, status, adv, lpa, stat_ge;
	u16 phy_id2;
	int media, speed, duplex;
	int i;

	/* Check if the PHY is up to snuff... */
	for (phy_addr = 0; phy_addr < CONFIG_PHY_MAX_ADDR; phy_addr++) {

		ftgmac100_phy_read(dev, phy_addr, MII_PHYSID1, &phy_id1);
		ftgmac100_phy_read(dev, phy_addr, MII_PHYSID2, &phy_id2);

		/*
		 * When it is unable to found PHY,
		 * the interface usually return 0xffff or 0x0000
		 */
		if (phy_id1 != 0xffff && phy_id1 != 0x0) {
//			printf("%s: found PHY at 0x%02x\n",
//				dev->name, phy_addr);
			priv->phy_addr = phy_addr;
			break;
		}
	}

	if (phy_id1 == 0xffff || phy_id1 == 0x0) {
		printf("%s: no PHY present\n", dev->name);
		return 0;
	}

	ftgmac100_phy_read(dev, priv->phy_addr, MII_BMSR, &status);

//	printf("phy_id : %x , status %x \n",((phy_id1 << 16) | phy_id2), status );

	if (!(status & BMSR_LSTATUS)) {
//		printf("! status \n");
		/* Try to re-negotiate if we don't have link already. */
		ftgmac100_phy_reset(dev);

		for (i = 0; i < 100000 / 100; i++) {
			ftgmac100_phy_read(dev, priv->phy_addr,
				MII_BMSR, &status);
			if (status & BMSR_LSTATUS)
				break;
			udelay(100);
		}
	}

	if (!(status & BMSR_LSTATUS)) {
		printf("%s: link down\n", dev->name);
		return 0;
	}

#ifdef CONFIG_FTGMAC100_EGIGA
	/* 1000 Base-T Status Register */
	ftgmac100_phy_read(dev, priv->phy_addr,
		MII_STAT1000, &stat_ge);

	speed = (stat_ge & (LPA_1000FULL | LPA_1000HALF)
		 ? 1 : 0);

	duplex = ((stat_ge & LPA_1000FULL)
		 ? 1 : 0);

	if (speed) { /* Speed is 1000 */
		printf("%s: link up, 1000bps %s-duplex\n",
			dev->name, duplex ? "full" : "half");
		return 0;
	}
#endif

	ftgmac100_phy_read(dev, priv->phy_addr, MII_ADVERTISE, &adv);
	ftgmac100_phy_read(dev, priv->phy_addr, MII_LPA, &lpa);

	media = mii_nway_result(lpa & adv);
	speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF) ? 1 : 0);
	duplex = (media & ADVERTISE_FULL) ? 1 : 0;

	printf("%s: link up, %sMbps %s-duplex\n",
	       dev->name, speed ? "100" : "10", duplex ? "full" : "half");

	return 1;
}

static int ftgmac100_update_link_speed(struct eth_device *dev)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	struct ftgmac100_data *priv = dev->priv;

	unsigned short stat_fe;
	unsigned short stat_ge;
	unsigned int maccr;

#ifdef CONFIG_FTGMAC100_EGIGA
	/* 1000 Base-T Status Register */
	ftgmac100_phy_read(dev, priv->phy_addr, MII_STAT1000, &stat_ge);
#endif

	ftgmac100_phy_read(dev, priv->phy_addr, MII_BMSR, &stat_fe);

	if (!(stat_fe & BMSR_LSTATUS))	/* link status up? */
		return 0;

	/* read MAC control register and clear related bits */
	maccr = __raw_readl(&ftgmac100->maccr) &
		~(FTGMAC100_MACCR_GIGA_MODE |
		  FTGMAC100_MACCR_FAST_MODE |
		  FTGMAC100_MACCR_FULLDUP);

#ifdef CONFIG_FTGMAC100_EGIGA
	if (stat_ge & LPA_1000FULL) {
		/* set gmac for 1000BaseTX and Full Duplex */
		maccr |= FTGMAC100_MACCR_GIGA_MODE | FTGMAC100_MACCR_FULLDUP;
	}

	if (stat_ge & LPA_1000HALF) {
		/* set gmac for 1000BaseTX and Half Duplex */
		maccr |= FTGMAC100_MACCR_GIGA_MODE;
	}
#endif

	if (stat_fe & BMSR_100FULL) {
		/* set MII for 100BaseTX and Full Duplex */
		maccr |= FTGMAC100_MACCR_FAST_MODE | FTGMAC100_MACCR_FULLDUP;
	}

	if (stat_fe & BMSR_10FULL) {
		/* set MII for 10BaseT and Full Duplex */
		maccr |= FTGMAC100_MACCR_FULLDUP;
	}

	if (stat_fe & BMSR_100HALF) {
		/* set MII for 100BaseTX and Half Duplex */
		maccr |= FTGMAC100_MACCR_FAST_MODE;
	}

	if (stat_fe & BMSR_10HALF) {
		/* set MII for 10BaseT and Half Duplex */
		/* we have already clear these bits, do nothing */
		;
	}

	/* update MII config into maccr */
	__raw_writel(maccr, &ftgmac100->maccr);

	return 1;
}

/*
 * Reset MAC
 */
static void ftgmac100_reset(struct eth_device *dev)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;

	debug("%s()\n", __func__);

	//Ryan modify
	__raw_writel(__raw_readl(&ftgmac100->maccr) | FTGMAC100_MACCR_SW_RST, &ftgmac100->maccr);

	while (__raw_readl(&ftgmac100->maccr) & FTGMAC100_MACCR_SW_RST);

	//Use New MDC and MDIO interface
	__raw_writel(__raw_readl(&ftgmac100->fear0) | 0x80000000, &ftgmac100->fear0);
}

/*
 * Set MAC address
 */
static void ftgmac100_set_mac(struct eth_device *dev,
	const unsigned char *mac)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	debug("%s(%x %x)\n", __func__, maddr, laddr);

	__raw_writel(maddr, &ftgmac100->mac_madr);
	__raw_writel(laddr, &ftgmac100->mac_ladr);
}

static void ftgmac100_set_mac_from_env(struct eth_device *dev)
{
#ifdef CONFIG_SYS_I2C_MAC_OFFSET

	char *s;
    int i, env;                // env variable 0: eeprom, 1: environment parameters

	s = getenv ("eeprom");
	env = (s && (*s == 'y')) ? 1 : 0;

	if (env) {
		printf("TODO ... eerprom --> \n");
	    eeprom_init();
		i2c_set_bus_num(3);
	    eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, CONFIG_SYS_I2C_MAC_OFFSET, dev->enetaddr, 6);

		for (i = 0; i < 6; i++) {
		    if (dev->enetaddr[i] != 0xFF) {
		        env = 0;	//Suppose not all 0xFF is valid
		    }
		}
	}

	if(env)
		eth_getenv_enetaddr_by_index("eth", dev->index, dev->enetaddr);
//		eth_setenv_enetaddr("ethaddr", dev->enetaddr);
	else
		eth_getenv_enetaddr_by_index("eth", dev->index, dev->enetaddr);
//		eth_getenv_enetaddr("ethaddr", dev->enetaddr);

	ftgmac100_set_mac(dev, dev->enetaddr);
#else
	eth_getenv_enetaddr_by_index("eth", dev->index, dev->enetaddr);
//	eth_getenv_enetaddr("ethaddr", dev->enetaddr);
	ftgmac100_set_mac(dev, dev->enetaddr);
#endif

}

/*
 * disable transmitter, receiver
 */
static void ftgmac100_halt(struct eth_device *dev)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;

	debug("%s()\n", __func__);

//	__raw_writel(0, &ftgmac100->maccr);
	//ryan modify
	__raw_writel(__raw_readl(&ftgmac100->maccr) & ~(FTGMAC100_MACCR_TXDMA_EN | FTGMAC100_MACCR_RXDMA_EN | FTGMAC100_MACCR_TXMAC_EN | FTGMAC100_MACCR_RXMAC_EN), &ftgmac100->maccr);

}

static int ftgmac100_init(struct eth_device *dev, bd_t *bd)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	struct ftgmac100_data *priv = dev->priv;
	struct ftgmac100_txdes *txdes = priv->txdes;
	struct ftgmac100_rxdes *rxdes = priv->rxdes;
	unsigned int maccr;
	int i;

	debug("%s()\n", __func__);

	//Ryan Add
	ftgmac100_reset(dev);

	/* set the ethernet address */
	ftgmac100_set_mac_from_env(dev);

	/* disable all interrupts */
	__raw_writel(0, &ftgmac100->ier);

	/* initialize descriptors */
	priv->tx_index = 0;
	priv->rx_index = 0;

	txdes[PKTBUFSTX - 1].txdes0	= cpu_to_le32(FTGMAC100_TXDES0_EDOTR);
	rxdes[PKTBUFSRX - 1].rxdes0	= cpu_to_le32(FTGMAC100_RXDES0_EDORR);

	for (i = 0; i < PKTBUFSTX; i++) {
		/* TXBUF_BADR */
		txdes[i].txdes3 = 0;
		txdes[i].txdes1 = 0;
	}

	for (i = 0; i < PKTBUFSRX; i++) {
		/* RXBUF_BADR */
		rxdes[i].rxdes3 = cpu_to_le32((unsigned int)net_rx_packets[i]);

//		rxdes[i].rxdes0 &= ~FTGMAC100_RXDES0_RXPKT_RDY;
		rxdes[i].rxdes0 = cpu_to_le32(le32_to_cpu(rxdes[i].rxdes0) & ~FTGMAC100_RXDES0_RXPKT_RDY);
	}

	/* transmit ring */
	__raw_writel((unsigned int)txdes, &ftgmac100->txr_badr);

	/* receive ring */
	__raw_writel((unsigned int)rxdes, &ftgmac100->rxr_badr);

	/* poll receive descriptor automatically */
	__raw_writel(FTGMAC100_APTC_RXPOLL_CNT(1), &ftgmac100->aptc);

	/* config receive buffer size register */
	__raw_writel(FTGMAC100_RBSR_SIZE(RBSR_DEFAULT_VALUE), &ftgmac100->rbsr);

	/* enable transmitter, receiver */
	maccr = FTGMAC100_MACCR_TXMAC_EN |
		FTGMAC100_MACCR_RXMAC_EN |
		FTGMAC100_MACCR_TXDMA_EN |
		FTGMAC100_MACCR_RXDMA_EN |
		FTGMAC100_MACCR_CRC_APD |
		FTGMAC100_MACCR_FULLDUP |
		FTGMAC100_MACCR_RX_RUNT |
		FTGMAC100_MACCR_RX_BROADPKT;

	__raw_writel(maccr, &ftgmac100->maccr);

//Ryan modify
#if 1
	ftgmac100_phy_init(dev);
	if (!ftgmac100_update_link_speed(dev))
		return -1;
#else
	if (!ftgmac100_phy_init(dev)) {
		if (!ftgmac100_update_link_speed(dev))
			return -1;
	}
#endif

/////////////////
#if 0 //(defined(CONFIG_MII) || defined(CONFIG_CMD_MII))
	/* Search phy address from range 0-31 */
	phy_adr = ethernet_phy_detect(dev);
	if (phy_adr < 0) {
		printf("FTGMAC100 : PHY not detected at address range 0-31\n");
		return -1;
	} else {
		debug("FTGMAC100 : PHY detected at addr %d\n", phy_adr);
		miiphy_write(dev->name, PHY_ADR_REQ, PHY_ADR_REQ, phy_adr);
	}
#endif

//////////////////
	return 0;
}

/*
 * Get a data block via Ethernet
 */
static int ftgmac100_recv(struct eth_device *dev)
{
	struct ftgmac100_data *priv = dev->priv;
	struct ftgmac100_rxdes *curr_des;
	unsigned short rxlen;

	curr_des = &priv->rxdes[priv->rx_index];

	invalidate_dcache_range((u32) curr_des, (u32) curr_des + sizeof(struct ftgmac100_rxdes));

	if (!(le32_to_cpu(curr_des->rxdes0) & FTGMAC100_RXDES0_RXPKT_RDY))
		return -1;

	if (le32_to_cpu(curr_des->rxdes0) & (FTGMAC100_RXDES0_RX_ERR |
				FTGMAC100_RXDES0_CRC_ERR |
				FTGMAC100_RXDES0_FTL |
				FTGMAC100_RXDES0_RUNT |
				FTGMAC100_RXDES0_RX_ODD_NB)) {
		return -1;
	}

	rxlen = FTGMAC100_RXDES0_VDBC(le32_to_cpu(curr_des->rxdes0));

	debug("%s(): RX buffer %d, %x received\n",
	       __func__, priv->rx_index, rxlen);

	invalidate_dcache_range((u32) (le32_to_cpu(curr_des->rxdes3)), (u32) (le32_to_cpu(curr_des->rxdes3)) + rxlen);

	/* pass the packet up to the protocol layers. */
	net_process_received_packet((void *)le32_to_cpu(curr_des->rxdes3), rxlen);

	/* release buffer to DMA */
	//curr_des->rxdes0 &= ~FTGMAC100_RXDES0_RXPKT_RDY;

	curr_des->rxdes0 = cpu_to_le32(le32_to_cpu(curr_des->rxdes0) & ~FTGMAC100_RXDES0_RXPKT_RDY);

	priv->rx_index = (priv->rx_index + 1) % PKTBUFSRX;

	return 0;
}

/*
 * Send a data block via Ethernet
 */
static int ftgmac100_send(struct eth_device *dev, void *packet, int length)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	struct ftgmac100_data *priv = dev->priv;
	struct ftgmac100_txdes *curr_des = &priv->txdes[priv->tx_index];
	unsigned long start;
    unsigned long now;
    unsigned long diff_time;

	if (curr_des->txdes0 & FTGMAC100_TXDES0_TXDMA_OWN) {
		debug("%s(): no TX descriptor available\n", __func__);
		return -1;
	}

	debug("%s(%x, %x)\n", __func__, (int)packet, length);

	length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

	/* initiate a transmit sequence */
	curr_des->txdes3 = cpu_to_le32((unsigned int)packet);	/* TXBUF_BADR */

	/* only one descriptor on TXBUF */
	/*
	curr_des->txdes0 &= FTGMAC100_TXDES0_EDOTR;

	curr_des->txdes0 |= FTGMAC100_TXDES0_FTS |
			    FTGMAC100_TXDES0_LTS |
			    FTGMAC100_TXDES0_TXBUF_SIZE(length) |
			    FTGMAC100_TXDES0_TXDMA_OWN ;
	*/
	curr_des->txdes0 = cpu_to_le32(le32_to_cpu(curr_des->txdes0) & FTGMAC100_TXDES0_EDOTR);

	curr_des->txdes0 = cpu_to_le32(le32_to_cpu(curr_des->txdes0) | (FTGMAC100_TXDES0_FTS |
			    FTGMAC100_TXDES0_LTS |
			    FTGMAC100_TXDES0_TXBUF_SIZE(length) |
			    FTGMAC100_TXDES0_TXDMA_OWN));

	/* start transmit */
	__raw_writel(1, &ftgmac100->txpd);
	invalidate_dcache_range((u32) curr_des, (u32) curr_des + sizeof(struct ftgmac100_rxdes));

	/* wait for transfer to succeed */
	start = get_timer(0);

	while (le32_to_cpu(curr_des->txdes0) & FTGMAC100_TXDES0_TXDMA_OWN) {
//		invalidate_dcache_range((u32) curr_des, (u32) curr_des + sizeof(struct ftgmac100_rxdes));
        now = get_timer(0);
        if ( now >= start )
            diff_time = now - start;
        else
            diff_time = now + 0xFFFFFFFF - start;
        if (diff_time >= 5000) /* 5ms */
        {
			debug("%s(): timed out\n", __func__);
			return -1;
		}
	}

	debug("%s(): packet sent\n", __func__);

	priv->tx_index = (priv->tx_index + 1) % PKTBUFSTX;

	return 0;
}

int ftgmac100_initialize(bd_t *bd)
{
	struct eth_device *dev;
	struct ftgmac100_data *priv;

#if 1   //Ryan Chen for more mac use
	int i, card_number = 0, mac_no;
	unsigned int			iobase[CONFIG_MAC_NUM];

	mac_no = CONFIG_MAC_NUM;
	iobase[0] = AST_MAC0_BASE;

#ifdef AST_MAC1_BASE
	iobase[1] = AST_MAC1_BASE;
#endif
	for (i = 0; i < mac_no; i++)
	{
		ast_scu_multi_func_eth(i);
		ast_scu_init_eth(i);

		debug ("FTGMAC100: Device @0x%x\n", iobase[i]);

		dev = malloc(sizeof *dev);

		/* Transmit and receive descriptors should align to 16 bytes */
		priv = memalign(16, sizeof(struct ftgmac100_data));
		if (!priv) {
			printf("%s(): failed to allocate priv\n", __func__);
			goto free_dev;
		}

		memset(dev, 0, sizeof(*dev));
		memset(priv, 0, sizeof(*priv));

		sprintf(dev->name, "FTGMAC100#%d", card_number);

		dev->iobase = iobase[i];
		dev->init	= ftgmac100_init;
		dev->halt	= ftgmac100_halt;
		dev->send	= ftgmac100_send;
		dev->recv	= ftgmac100_recv;
		dev->priv	= priv;
		dev->write_hwaddr = NULL;		//20130209, ryan chen add

		eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
		miiphy_register(dev->name, ftgmac100_reg_read, ftgmac100_reg_write);
#endif

		/* set the ethernet address */
		ftgmac100_set_mac_from_env(dev);

		ftgmac100_reset(dev);

		card_number++;
	}
	return card_number;

#else
	dev = malloc(sizeof *dev);
	if (!dev) {
		printf("%s(): failed to allocate dev\n", __func__);
		goto out;
	}

	/* Transmit and receive descriptors should align to 16 bytes */
	priv = memalign(16, sizeof(struct ftgmac100_data));
	if (!priv) {
		printf("%s(): failed to allocate priv\n", __func__);
		goto free_dev;
	}

	memset(dev, 0, sizeof(*dev));
	memset(priv, 0, sizeof(*priv));

	sprintf(dev->name, "FTGMAC100#0");
	dev->iobase	= AST_MAC0_BASE;
	dev->init	= ftgmac100_init;
	dev->halt	= ftgmac100_halt;
	dev->send	= ftgmac100_send;
	dev->recv	= ftgmac100_recv;
	dev->priv	= priv;

	eth_register(dev);
//////
	ast_scu_multi_func_eth(0);
	ast_scu_init_eth(0);
/////
	ftgmac100_reset(dev);

	return 1;
#endif

free_dev:
	free(dev);
//out:
	return 0;

}
