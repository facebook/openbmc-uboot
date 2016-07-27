/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBCMM_CONFIG_H
#define __FBCMM_CONFIG_H

#define CONFIG_BOOTARGS   "debug console=ttyS1,9600n8 root=/dev/ram rw"
#define CONFIG_UPDATE   "tftp 80800000 ast2500.scr; so 80800000'"
#define CONFIG_BOOTCOMMAND  "bootm 20480000"  /* Location of FIT */
#define CONFIG_BOOTFILE   "flash-wedge"

/*
 * Memory Configuration
 * Deprecating with v2016.03: CONFIG_ASPEED_WRITE_DEFAULT_ENV
 */
#define PHYS_SDRAM_1_SIZE 0x10000000  /* 256 MB */

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_COM1  AST_UART1_BASE
#define CONFIG_CONS_INDEX 1
#define CONFIG_BAUDRATE   9600
#define CONFIG_ASPEED_COM AST_UART1_BASE  /* COM1(UART1) */

/*
 * NIC configuration
 */
#define CONFIG_MAC2_ENABLE
#define CONFIG_MAC1_PHY_SETTING     0
#define CONFIG_MAC2_PHY_SETTING     0
#define CONFIG_ASPEED_MAC_NUMBER  2
#define CONFIG_ASPEED_MAC_CONFIG  2

#include "facebook_common.h"
#include "ast2500_common.h"

#endif  /* __FBCMM_CONFIG_H */
