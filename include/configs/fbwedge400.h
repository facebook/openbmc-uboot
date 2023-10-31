/*
 * (C) Copyright 2019-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBWEDGE400_CONFIG_H
#define __FBWEDGE400_CONFIG_H

#define CONFIG_FBWEDGE400 1

#ifndef CONFIG_SYS_LONGHELP
#define CONFIG_SYS_LONGHELP
#endif

#define CONFIG_BOOTARGS   "debug console=ttyS0,9600n8 root=/dev/ram rw"
#define CONFIG_BOOTFILE   "flash-wedge400"

/*
 * Enable DRAM ECC, going to lose 1/8 of memory
 */
#define CONFIG_DRAM_ECC

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE     -4
#define CONFIG_SYS_NS16550_COM1 AST_UART1_BASE
#define CONFIG_CONS_INDEX       1
#define CONFIG_ASPEED_COM       AST_UART1_BASE

/*
 * UART configurtion
 */
#define CONFIG_ASPEED_UART1_ENABLE
#define CONFIG_ASPEED_UART1_BMC // by default, src is LPC for UART1
#define CONFIG_ASPEED_UART2_ENABLE
#define CONFIG_ASPEED_UART2_BMC // by default, src is LPC for UART2
#define CONFIG_ASPEED_UART3_ENABLE
#define CONFIG_ASPEED_UART4_ENABLE
#define CONFIG_ASPEED_UART5_MAP_IO6

/*
 * NIC configuration
 */
#define CONFIG_PHY_BROADCOM_MODE1000X
/*
#define CONFIG_MAC2_ENABLE
#define CONFIG_MAC1_PHY_SETTING     0
#define CONFIG_MAC2_PHY_SETTING     0
#define CONFIG_MAC2_PHY_LINK_INTERRUPT
#define CONFIG_ASPEED_MAC_NUMBER    2
#define CONFIG_ASPEED_MAC_CONFIG    2
*/

#include "facebook_common.h"
#include "ast2500_common.h"

/*
 * Extend dual_boot watchdog (WDT2)'s timeout to 10 minutes to allow time
 * to format a 64MB data0 partition
 */
#undef CONFIG_ASPEED_WATCHDOG_TIMEOUT
#define CONFIG_ASPEED_WATCHDOG_TIMEOUT (10 * 60)

#endif  /* __FBWEDGE400_CONFIG_H */
