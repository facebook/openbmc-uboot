/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASPEED_MDIO_H__
#define __ASPEED_MDIO_H__

#include <net.h>
#include <miiphy.h>

extern int aspeed_mdio_read(struct mii_dev *bus, int phy_addr, int dev_addr, 
					int reg_addr);
extern int aspeed_mdio_write(struct mii_dev *bus, int phy_addr, int dev_addr,
				int reg_addr, u16 value);


#endif /* __ASPEED_MDIO_H__ */
