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

#ifndef __FBEP_CONFIG_H
#define __FBEP_CONFIG_H

#define CONFIG_FBEP 1

#define CONFIG_BOOTARGS          "debug console=ttyS0,57600n8 root=/dev/ram rw printk.time=1 dual_flash=2"
#define CONFIG_BOOTFILE          "flash-emeraldpools"
/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE     -4
#define CONFIG_SYS_NS16550_COM1         AST_UART0_BASE
#define CONFIG_CONS_INDEX               1
#define CONFIG_BAUDRATE                 57600

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

/*
 * Enable 2x I/O read command
 */
#define CONFIG_FLASH_SPIx2_Dummy

#define CONFIG_PFR_BUS "i2c-bus@140"
#define CONFIG_PFR_ADDR 0x58

#include "facebook_common.h"
#include "ast2500_common.h"

#endif  /* __FBEP_CONFIG_H */
