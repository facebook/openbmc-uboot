/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBWEDGE_CONFIG_H
#define __FBWEDGE_CONFIG_H

#define CONFIG_BOOTARGS		"debug console=ttyS0,9600n8 root=/dev/ram rw"
#define CONFIG_UPDATE		"tftp 40800000 ast2400.scr; so 40800000'"
#define CONFIG_BOOTCOMMAND 	"bootm 20080000"	/* Location of FIT */
#define CONFIG_BOOTFILE		"all.bin"

/*
 * Memory Configuration
 * Deprecating with v2016.03: CONFIG_ASPEED_WRITE_DEFAULT_ENV
 */
#define PHYS_SDRAM_1_SIZE	0x10000000 	/* 256 MB */

/*
 * Serial configuration
 */
#define CONFIG_CONS_INDEX	3
#define CONFIG_BAUDRATE		9600
#define CONFIG_ASPEED_COM	0x1e78e000	/* COM3 */
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/*
 * NIC configuration
 */
#define CONFIG_MAC2_ENABLE
#define CONFIG_MAC1_PHY_SETTING		0
#define CONFIG_MAC2_PHY_SETTING		0
#define CONFIG_ASPEED_MAC_NUMBER	2
#define CONFIG_ASPEED_MAC_CONFIG	2

#define CONFIG_MAC1_PHY_LINK_INTERRUPT
#define CONFIG_MAC2_PHY_LINK_INTERRUPT

#include "facebook_common.h"
#include "ast2400_common.h"

#endif  /* __FBWEDGE_CONFIG_H */
