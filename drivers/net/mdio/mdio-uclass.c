// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Author: Ken Ma<make@marvell.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <miiphy.h>
#include <net/mdio.h>

DECLARE_GLOBAL_DATA_PTR;

int mdio_mii_bus_get(struct udevice *mdio_dev, struct mii_dev **busp)
{
       *busp = (struct mii_dev *)dev_get_uclass_platdata(mdio_dev);

       return 0;
}

int mdio_device_get_from_phy(ofnode phy_node, struct udevice **devp)
{
       ofnode mdio_node;

       mdio_node = ofnode_get_parent(phy_node);
       return uclass_get_device_by_ofnode(UCLASS_MDIO, mdio_node, devp);
}

int mdio_mii_bus_get_from_phy(ofnode phy_node, struct mii_dev **busp)
{
       struct udevice *mdio_dev;
       int ret;

       ret = mdio_device_get_from_phy(phy_node, &mdio_dev);
       if (ret)
               return ret;

       *busp = (struct mii_dev *)dev_get_uclass_platdata(mdio_dev);

       return 0;
}

int mdio_device_get_from_eth(struct udevice *eth, struct udevice **devp)
{
       struct ofnode_phandle_args phy_args;
       int ret;

       ret = dev_read_phandle_with_args(eth, "phy", NULL, 0, 0, &phy_args);
       if (!ret)
               return mdio_device_get_from_phy(phy_args.node, devp);

       /*
        * If there is no phy reference under the ethernet fdt node,
        * it is not an error since the ethernet device may do not use
        * mode; so in this case, the output mdio device pointer is set
        * as NULL.
        */
       *devp = NULL;
       return 0;
}

int mdio_mii_bus_get_from_eth(struct udevice *eth, struct mii_dev **busp)
{
       struct udevice *mdio_dev;
       int ret;

       ret = mdio_device_get_from_eth(eth, &mdio_dev);
       if (ret)
               return ret;

       if (mdio_dev)
               *busp = (struct mii_dev *)dev_get_uclass_platdata(mdio_dev);
       else
               *busp = NULL;

       return 0;
}

static int mdio_uclass_pre_probe(struct udevice *dev)
{
       struct mii_dev *bus = (struct mii_dev *)dev_get_uclass_platdata(dev);
       const char *name;

       /* initialize mii_dev struct fields,  implement mdio_alloc() setup */
       INIT_LIST_HEAD(&bus->link);

       name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
                          "mdio-name", NULL);
       if (name)
               strncpy(bus->name, name, MDIO_NAME_LEN);

       return 0;
}

static int mdio_uclass_post_probe(struct udevice *dev)
{
       struct mii_dev *bus = (struct mii_dev *)dev_get_uclass_platdata(dev);

       return mdio_register(bus);
}

UCLASS_DRIVER(mdio) = {
       .id             = UCLASS_MDIO,
       .name           = "mdio",
       .pre_probe      = mdio_uclass_pre_probe,
       .post_probe     = mdio_uclass_post_probe,
       .per_device_platdata_auto_alloc_size = sizeof(struct mii_dev),
};
