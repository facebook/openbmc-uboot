/*
 * (C) Copyright 2020-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBELBERT_CONFIG_H
#define __FBELBERT_CONFIG_H

#define CONFIG_FBELBERT 1

#define CONFIG_SYS_LONGHELP

#define CONFIG_BOOTARGS   "debug console=ttyS0,9600n8 root=/dev/ram rw"
#define CONFIG_BOOTFILE   "flash-elbert"

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
#define CONFIG_ASPEED_UART5_MAP_IO6

/*
 * NIC configuration
 */
#define CONFIG_MAC1_PHY_SETTING     0
#define CONFIG_MAC1_RGMII_MODE
#define CONFIG_ASPEED_MAC_NUMBER  1
#define CONFIG_ASPEED_MAC_CONFIG  1

#define CONFIG_MII_
#define CONFIG_CMD_MII

/*
 * Enable DRAM ECC, going to lose 1/8 of memory
 */
#define CONFIG_DRAM_ECC

/*
 * Watchdog configuration, needed for TPM hardware reset
 * This will trigger GPIOAB2 output high when the CPU is reset
 */
#define CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO

#include "facebook_common.h"
#include "ast2500_common.h"

#endif  /* __FBELBERT_CONFIG_H */
