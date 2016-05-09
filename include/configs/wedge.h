/*
 * Configuration settings for the ASPEED AST2400.
 *
 * Copyright 2004 Peter Chen <peterc@socle-tech.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __WEDGE_CONFIG_H
#define __WEDGE_CONFIG_H

#define CONFIG_FLASH_SPI
#define CONFIG_FLASH_AST2300

#define CONFIG_DRAM_1GBIT
#define CONFIG_DRAM_408
#define CONFIG_DRAM_UART_OUT

#define CONFIG_BOOTARGS         "debug console=ttyS2,9600n8 root=/dev/ram rw"
#define CONFIG_UPDATE           "tftp 40800000 ast2400.scr; so 40800000'"

#define CONFIG_CMD_MII
#define CONFIG_CMD_NETTEST
#define CONFIG_CMD_SLT

/*
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550_REG_SIZE 4
#define CONFIG_SYS_NS16550_COM3		0x1e78e000
#define CONFIG_CONS_INDEX		3
#define CONFIG_BAUDRATE			9600
#define CONFIG_ASPEED_COM		0x1e78e000 // COM3

/*
 * NIC configuration
 */
#define CONFIG_MAC1_ENABLE
#define CONFIG_MAC1_PHY_LINK_INTERRUPT
#define CONFIG_MAC1_PHY_SETTING     0

#define CONFIG_MAC2_ENABLE
#define CONFIG_MAC2_PHY_LINK_INTERRUPT
#define CONFIG_MAC2_PHY_SETTING     0

#define CONFIG_ASPEED_MAC_NUMBER  2
#define CONFIG_ASPEED_MAC_CONFIG  2 // config MAC2

#include "ast2400_common.h"

#endif	/* __WEDGE_CONFIG_H */
