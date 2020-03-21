/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __FBY2_CONFIG_H
#define __FBY2_CONFIG_H

#define CONFIG_FBY2 1

#define CONFIG_ASPEED_UART1_ENABLE
#define CONFIG_ASPEED_UART2_ENABLE
#define CONFIG_ASPEED_UART3_ENABLE
#define CONFIG_ASPEED_UART4_ENABLE

#ifdef CONFIG_SPL
#define CONFIG_BOOTARGS          "debug console=ttyS0,57600n8 root=/dev/ram rw printk.time=1 dual_flash=2"
#else
#define CONFIG_BOOTARGS          "debug console=ttyS0,57600n8 root=/dev/ram rw printk.time=1"
#endif
#define CONFIG_UPDATE            "tftp 80800000 ast2500.scr; so 80800000'"
#define CONFIG_BOOTFILE          "flash-fby2"
#ifndef CONFIG_SPL_BUILD
#ifndef CONFIG_SYS_LONGHELP
#define CONFIG_SYS_LONGHELP     /* undef to save memory   */
#endif
#define CONFIG_SYS_HUSH_PARSER  /* Use the HUSH parser */
#endif

#define MAX_NODES 4
/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE     -4
#define CONFIG_SYS_NS16550_COM1         AST_UART0_BASE
#define CONFIG_CONS_INDEX               1
#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE                 57600
#endif
/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN  2
#define CONFIG_SYS_I2C_EEPROM_ADDR  0xa0

/*
 * NIC configuration
 */
#define CONFIG_MAC1_PHY_SETTING     2
#define CONFIG_MAC2_PHY_SETTING     0
#define CONFIG_ASPEED_MAC_NUMBER  1
#define CONFIG_ASPEED_MAC_CONFIG  1 /* config MAC1 */

/*
 * Enable DRAM ECC, going to lose 1/8 of memory
 */
#define CONFIG_DRAM_ECC

/*
 * Watchdog configuration, needed for TPM hardware reset
 * This will trigger GPIOAB2 output high when the CPU is reset
 */
#define CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO

#include "fby2_ext.h"
#include "facebook_common.h"
#include "ast2500_common.h"

#endif  /* __FBY2_CONFIG_H */
