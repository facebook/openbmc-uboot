/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBYAMP_CONFIG_H
#define __FBYAMP_CONFIG_H

#define CONFIG_FBYAMP 1

#ifndef CONFIG_SYS_LONGHELP
#define CONFIG_SYS_LONGHELP
#endif

#define CONFIG_BOOTARGS   "console=ttyS0,9600n8 root=/dev/ram rw"
#define CONFIG_BOOTFILE   "flash-yamp"

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_COM1 AST_UART0_BASE
#define CONFIG_CONS_INDEX       1
#define CONFIG_ASPEED_COM       AST_UART0_BASE

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
 * Enable DRAM ECC, going to lose 1/8 of memory
 */
#define CONFIG_DRAM_ECC

#include "facebook_common.h"
#include "ast2500_common.h"

#endif  /* __FBYAMP_CONFIG_H */
