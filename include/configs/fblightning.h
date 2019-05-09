/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBLIGHTNING_CONFIG_H
#define __FBLIGHTNING_CONFIG_H

#define CONFIG_FBLIGHTNING 1

#define CONFIG_BOOTARGS		"debug console=ttyS0,57600n8 root=/dev/ram rw printk.time=1 dual_flash=1"
#define CONFIG_UPDATE		"tftp 40800000 ast2400.scr; so 40800000'"
#define CONFIG_BOOTFILE		"all.bin"

#define CONFIG_CMD_CS1TEST
#define CONFIG_CS1TEST_CS0PATTERN_ADDR 0x48000000

/*
 * Memory Configuration
 * Deprecating with v2016.03: CONFIG_ASPEED_WRITE_DEFAULT_ENV
*/
#define PHYS_SDRAM_1_SIZE	0x8000000	/* 128 MB */

/*
 * Serial configuration
 */
#define CONFIG_SYS_LOADS_BAUD_CHANGE
#define CONFIG_BAUDRATE		57600
#define CONFIG_ASPEED_COM	0x1e784000	/* COM2(UART5) */

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
#define CONFIG_MAC1_ENABLE
#define CONFIG_MAC1_PHY_SETTING		2
#define CONFIG_MAC2_PHY_SETTING		0
#define CONFIG_ASPEED_MAC_NUMBER	1
#define CONFIG_ASPEED_MAC_CONFIG	1

#define CONFIG_MAC1_PHY_LINK_INTERRUPT
#define CONFIG_MAC2_PHY_LINK_INTERRUPT
#define CONFIG_ARCH_AST2400

#define CONFIG_ASPEED_ENABLE_WATCHDOG
#define CONFIG_ASPEED_ENABLE_DUAL_BOOT_WATCHDOG
#define CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT \
  (CONFIG_ASPEED_WATCHDOG_TIMEOUT - 5)

/*
 * SPI flash configuration
 */
#define CONFIG_2SPIFLASH

/*
 * Enable DRAM ECC, going to lose 1/8 of memory
 */
#define CONFIG_DRAM_ECC

#include "facebook_common.h"
#include "ast2400_common.h"


#endif  /* __FBLIGHTNING_CONFIG_H */
