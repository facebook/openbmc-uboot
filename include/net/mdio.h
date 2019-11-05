/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Author: Ken Ma<make@marvell.com>
 */

#ifndef _MDIO_H_
#define _MDIO_H_

#include <dm.h>
#include <phy.h>

/**
 * mdio_mii_bus_get() - Get mii bus from mdio udevice
 *
 * @mdio_dev:  mdio udevice
 * @busp:      returns mii bus
 * @returns 0 on success, error code otherwise.
 */
int mdio_mii_bus_get(struct udevice *mdio_dev, struct mii_dev **busp);

/**
 * mdio_device_get_from_phy() - Get the mdio udevice which the phy belongs to
 *
 * @phy_node:  phy node offset
 * @devp:      returns mdio udevice
 * @returns 0 on success, error code otherwise.
 */
int mdio_device_get_from_phy(ofnode phy_node, struct udevice **devp);

/**
 * mdio_mii_bus_get_from_phy() - Get the mii bus which the phy belongs to
 *
 * @phy_node:  phy node offset
 * @busp:      returns mii bus
 * @returns 0 on success, error code otherwise.
 */
int mdio_mii_bus_get_from_phy(ofnode phy_node, struct mii_dev **busp);

/**
 * mdio_device_get_from_eth() - When there is a phy reference of "phy = <&...>"
 *                      under an ethernet udevice fdt node, this function can
 *                      get the mdio udevice which the phy belongs to
 *
 * @dev:       the ethernet udevice which contains the phy reference
 * @devp:      returns mdio udevice
 * @returns 0 on success, error code otherwise.
 */
int mdio_device_get_from_eth(struct udevice *eth, struct udevice **devp);

/**
 * mdio_mii_bus_get_from_eth() - When there is a phy reference of
 *                      "phy = <&...>" under an ethernet udevice fdt node, this
 *                      function can get the mii bus which the phy belongs to
 *
 * @eth:       the ethernet udevice which contains the phy reference
 * @busp:      returns mii bus
 * @returns 0 on success, error code otherwise.
 */
int mdio_mii_bus_get_from_eth(struct udevice *eth, struct mii_dev **busp);

#endif /* _MDIO_H_ */


