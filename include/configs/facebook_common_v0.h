/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FACEBOOK_CONFIG_V0_H
#define __FACEBOOK_CONFIG_V0_H

/*
 * Verified boot options.
 *
 * These are general feature flags related to verified boot.
 *   CONFIG_SPL: Use a dual SPI for booting U-Boot.
 *   CONFIG_SPL_FIT_SIGNATURE: Enforce verified boot code in ROM.
 *   CONFIG_SPL_BUILD: Defined when the compilation is for the SPL
 *
 *   CONFIG_SYS_REMAP_BASE: Location U-Boot expects to execute from.
 *   CONFIG_SYS_UBOOT_START: Similar to CONFIG_SYS_REMAP_BASE
 *   CONFIG_SYS_ENV_BASE: Flash base address for RW environment data.
 *
 * Custom build options:
 *   CONFIG_SYS_SPL_FIT_BASE: The known location of FIT containing U-Boot.
 *   CONFIG_SYS_RECOVERY_BASE: The known location of the Recovery U-Boot.
 *   CONFIG_ASPEED_RECOVERY_BUILD: Defined when the compilation is for the Recovery.
 */
#define CONFIG_CS0_SPL_KERNEL_LOAD    "200E0000"
#define CONFIG_CS1_SPL_KERNEL_LOAD    "280E0000"

#ifdef CONFIG_SPL
#ifdef CONFIG_ASPEED_RECOVERY_BUILD
#define CONFIG_SYS_REMAP_BASE     0x20015000
#define CONFIG_SYS_UBOOT_START    0x20015000 /* Must be defined as-is */
#define CONFIG_KERNEL_LOAD        CONFIG_CS0_SPL_KERNEL_LOAD
#else
#define CONFIG_SYS_REMAP_BASE     0x28084000
#define CONFIG_SYS_UBOOT_START    0x28084000 /* Must be defined as-is */
#define CONFIG_KERNEL_LOAD        CONFIG_CS1_SPL_KERNEL_LOAD
#endif
#define CONFIG_SYS_SPL_FIT_BASE   0x28080000
#define CONFIG_SYS_RECOVERY_BASE  0x20015000
#define CONFIG_SYS_ENV_BASE       0x28000000
#define CONFIG_ENV_SPI_BUS 0
#define CONFIG_ENV_SPI_CS 1
#else
/* Legacy non-Verified boot configuration. */
#define CONFIG_SYS_REMAP_BASE     0x00000000
#define CONFIG_SYS_UBOOT_START    0x00000000
#define CONFIG_SYS_ENV_BASE       0x20000000
#ifdef CONFIG_FIT
#define CONFIG_KERNEL_LOAD        "20080000"
#else
#define CONFIG_KERNEL_LOAD        "20080000 20480000"
#endif
#endif

/*
 * Requirements:
 * Before including this common configuration, the board must include
 * the CPU/arch platform configuration.
 */

/*
 * Environment configuration
 * This used to have:
 *   CONFIG_ENV_IS_IN_FLASH
 *   CONFIG_ENV_IS_IN_SPI_FLASH
 *   CONFIG_ENV_IS_NOWHERE
 */
#if defined(CONFIG_ASPEED_RECOVERY_BUILD) || defined(CONFIG_SPL_BUILD)
/* Prevent the Recovery build from using the RW environment. */
#ifndef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_NOWHERE
#endif
#else
#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OVERWRITE
#endif
#define CONFIG_ENV_OFFSET        0x60000 /* environment starts here  */
#define CONFIG_ENV_ADDR          (CONFIG_SYS_ENV_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE          0x20000 /* # of bytes of env, 128k */
#define CONFIG_ENV_SECT_SIZE	 (64 << 10) /* 64 KiB */
#define ENV_INITRD_HIGH "initrd_high=a0000000\0"
#define CONFIG_EXTRA_ENV_SETTINGS                       \
    "verify=no\0"                                       \
    "spi_dma=no\0"                                      \
    "updatefile=" CONFIG_BOOTFILE ".fit\0"              \
    ENV_INITRD_HIGH                                     \
    ""

/*
 * Flash configuration
 * It is possible to run using the SMC and not enable flash
 *   CONFIG_CMD_FLASH
 *   CONFIG_SYS_NO_FLASH
 */

/*
 * Watchdog timer configuration
 */
#define CONFIG_ASPEED_ENABLE_WATCHDOG
#define CONFIG_ASPEED_WATCHDOG_TIMEOUT	(5*60) /* 5 minutes */
/* #define CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HZ     1000
#define CONFIG_SYS_TIMERBASE	AST_TIMER_BASE 	/* use timer 1 */

/*
 * NIC configuration
 */
#define CONFIG_NET_RANDOM_ETHADDR
#define CONFIG_LIB_RAND

/*
 * Memory Test configuration
 */
#define CONFIG_SYS_MEMTEST_ITERATION 10

/*
 * Command configuration
 */
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_CMD_MEMTEST2
#define CONFIG_CMD_TFTPPUT
#ifndef CONFIG_CMD_FLASH
#define CONFIG_CMD_FLASH
#endif
#ifdef CONFIG_SPL
#define CONFIG_CMD_VBS
#endif
/*
 * Additional command configuration
 *   CONFIG_CMD_I2C
 *   CONFIG_CMD_EEPROM
 */

/*
 * Additional features configuration
 */
#ifndef CONFIG_SHA256
#define CONFIG_SHA256
#endif

/* Configure the rare, boot-fail aftermath. */
#ifdef CONFIG_CMD_VBS
/* If this runs then verified-boot failed */
#define CONFIG_POSTBOOT "vbs 6 60; bootm " CONFIG_CS0_SPL_KERNEL_LOAD "; "
#else
#define CONFIG_POSTBOOT " "
#endif

/*
 * Recovery boot flow
 */
#ifdef CONFIG_ASPEED_RECOVERY_BUILD
#define CONFIG_PREBOOT "setenv verify no; "
#else
#ifdef CONFIG_CMD_VBS
#define CONFIG_PREBOOT "vbs oscheck; "
#else
#define CONFIG_PREBOOT " "
#endif
#endif

/*
 * Basic boot command configuration based on flash
 */
#define CONFIG_BOOTCOMMAND                                \
  "bootm " CONFIG_KERNEL_LOAD "; " /* Location of FIT */  \
  CONFIG_POSTBOOT

/*
 * Command to run in if CLI is used.
 */
#if defined(CONFIG_CMD_VBS) && defined(CONFIG_SPL)
#define CONFIG_PRECLICOMMAND "vbs interrupt; "
#endif

/*
 * Lock the BMC TPM during provisioning (perform 1-time operations)
 */
#define CONFIG_ASPEED_TPM_LOCK

#ifdef CONFIG_SPL
#ifdef CONFIG_SPL_BUILD
/* This is an SPL build */

#ifndef DEBUG
/* The SPL has size constraints, the debug build may overflow. */
/* The Tiny-printf works in theory, but has side effects in the SPL. */
/* #define CONFIG_USE_TINY_PRINTF */
#else
#define CONFIG_SPL_DISPLAY_PRINT
#endif

//#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_MAX_FOOTPRINT  0x15000

/* During an SPL build the base is 0x0. */
// 2019.04 or 07 version this will go to Kconfig
#define CONFIG_SPL_TEXT_BASE     0x00000000

/* Grow the stack down from 0x6000 to an expected max of 12kB. */
#define CONFIG_SPL_STACK          (CONFIG_SYS_SRAM_BASE + 0x6000)
#define CONFIG_SYS_INIT_SP_ADDR   CONFIG_SPL_STACK

/* Establish an 'arena' for heap from 0x1000 - 0x3000, 8k */
#define CONFIG_SYS_SPL_MALLOC_START (CONFIG_SYS_SRAM_BASE + 0x1000)
#define CONFIG_SYS_SPL_MALLOC_SIZE  0x2000

/* General SPL build feature includes. */
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT

/* Verified boot required features. */
#define CONFIG_SPL_CRYPTO_SUPPORT
#define CONFIG_SPL_HASH_SUPPORT
#define CONFIG_SPL_SHA256_SUPPORT
#define CONFIG_SPL_SHA256
#define CONFIG_SPL_SHA1
#define CONFIG_SPL_TPM

#else
/* This is a U-Boot build */

/* During the U-Boot build the base address is the SPL FIT start address. */
//#undef  CONFIG_SYS_TEXT_BASE
//#define CONFIG_SYS_TEXT_BASE    CONFIG_SYS_UBOOT_START
#endif
#endif

/*
 * Console UART configuration
 */
#ifndef CONFIG_ASPEED_COM
#define CONFIG_ASPEED_COM               AST_UART0_BASE // UART5
#endif

/*
 * Autoboot configuration
 */
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds (stop with 'Delete' key)...\n"
#define CONFIG_AUTOBOOT_STOP_STR	"\x1b\x5b\x33\x7e"	/* 'Delete', ESC[3~ */
#define CONFIG_AUTOBOOT_KEYED

#define CONFIG_USE_BOOTCOMMAND
#define CONFIG_USE_BOOTARGS
#endif /* __FACEBOOK_CONFIG_V0_H */
