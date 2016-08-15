/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBCMM_CONFIG_H
#define __FBCMM_CONFIG_H

#define CONFIG_IDENT_STRING " fbcmm-v0.0"
#define CONFIG_FBCMM 1

#define CONFIG_SYS_LONGHELP     /* undef to save memory   */
#define CONFIG_SYS_HUSH_PARSER  /* Use the HUSH parser */
#define CONFIG_BOOTARGS   "debug console=ttyS1,9600n8 root=/dev/ram rw"
#define CONFIG_UPDATE   "tftp 80800000 ast2500.scr; so 80800000'"
#define CONFIG_BOOTCOMMAND  "bootm 20080000"  /* Location of FIT */
#define CONFIG_BOOTFILE   "flash-wedge"

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_COM1  AST_UART1_BASE
#define CONFIG_CONS_INDEX 1
#define CONFIG_BAUDRATE   9600

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
