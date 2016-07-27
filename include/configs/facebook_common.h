/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FACEBOOK_CONFIG_H
#define __FACEBOOK_CONFIG_H

/*
 * Requirements:
 * Before including this common configuration, the board must include
 * the CPU/arch platform configuration.
 */

/*
 * Basic boot command configuration based on flash
 */
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds (stop with 'Delete' key)...\n"
#define CONFIG_AUTOBOOT_STOP_STR	"\x1b\x5b\x33\x7e"	/* 'Delete', ESC[3~ */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_ZERO_BOOTDELAY_CHECK

/*
 * Environment configuration
 * This used to have:
 *   CONFIG_ENV_IS_IN_FLASH
 *   CONFIG_ENV_IS_IN_SPI_FLASH
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_OFFSET	0x60000		/* environment starts here  */
#define CONFIG_ENV_SIZE		0x20000		/* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE

/*
 * Flash configuration
 * It is possible to run using the SMC and not enable flash
 *   CONFIG_CMD_FLASH
 */
#define CONFIG_SYS_NO_FLASH

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE -4

/*
 * Watchdog timer configuration
 */
#define CONFIG_ASPEED_ENABLE_WATCHDOG
#define CONFIG_ASPEED_WATCHDOG_TIMEOUT	(5*60) /* 5 minutes */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory   */
#define CONFIG_SYS_TIMERBASE	AST_TIMER_BASE 	/* use timer 1 */
#define CONFIG_SYS_HZ 		1000

/*
 * NIC configuration
 */
#define CONFIG_NET_RANDOM_ETHADDR
#define CONFIG_LIB_RAND

/*
 * Command configuration
 */
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_CMD_SDRAM

/*
 * Additional command configuration
 *   CONFIG_CMD_I2C
 *   CONFIG_CMD_EEPROM
 */

#endif /* __FACEBOOK_CONFIG_H */
