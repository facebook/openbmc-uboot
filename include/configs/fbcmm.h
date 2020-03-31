/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBCMM_CONFIG_H
#define __FBCMM_CONFIG_H

#define CONFIG_FBCMM 1

#ifndef CONFIG_SYS_LONGHELP
#define CONFIG_SYS_LONGHELP
#endif

#define CONFIG_BOOTARGS   "debug console=ttyS1,9600n8 root=/dev/ram rw"
#define CONFIG_BOOTFILE   "flash-cmm"

/*
 * Serial configuration
 */
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
#define CONFIG_ASPEED_MAC_NUMBER  2
#define CONFIG_ASPEED_MAC_CONFIG  2
*/

/*
 * Enable DRAM ECC, going to lose 1/8 of memory
 */
#define CONFIG_DRAM_ECC

#include "facebook_common.h"
#include "ast2500_common.h"

#endif  /* __FBCMM_CONFIG_H */
