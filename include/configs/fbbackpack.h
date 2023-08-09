/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBBACKPACK_CONFIG_H
#define __FBBACKPACK_CONFIG_H

#define CONFIG_FBBACKPACK 1


#define CONFIG_BOOTARGS   "console=ttyS0,9600n8 root=/dev/ram rw"
#define CONFIG_BOOTFILE   "flash-backpack100_fc_lc"

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_COM1 AST_UART0_BASE
#define CONFIG_CONS_INDEX       1
#define CONFIG_ASPEED_COM       AST_UART0_BASE
#define CONFIG_BAUDRATE         9600

/*
 * UART configurtion
 */
#define CONFIG_ASPEED_UART1_ENABLE
#define CONFIG_ASPEED_UART1_BMC // by default, src is LPC for UART1
#define CONFIG_ASPEED_UART2_ENABLE
#define CONFIG_ASPEED_UART2_BMC // by default, src is LPC for UART2
#define CONFIG_ASPEED_UART3_ENABLE
#define CONFIG_ASPEED_UART4_ENABLE

/*
 * BROADCOM PHY hack
 */
#define CONFIG_PHY_BROADCOM_MODE1000X
/*
 * SPI flash configuration
 */
#define CONFIG_2SPIFLASH

#include "facebook_common.h"
#include "ast2400_common.h"

#endif  /* __FBBACKPACK_CONFIG_H */
