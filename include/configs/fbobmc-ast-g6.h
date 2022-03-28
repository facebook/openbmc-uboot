/* SPDX-License-Identifier: GPL-2.0+ */
/*
  * Copyright (c) 2021 Facebook Inc.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*============= Copy from aspeed_common.h ==========================*/
#include <asm/arch/platform.h>

#define CONFIG_STANDALONE_LOAD_ADDR 0x83000000

/* Misc CPU related */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG



#define CONFIG_SYS_BOOTMAPSZ		(256 * 1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(32 << 20)

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_BOOTM_LEN 		(0x800000 * 2)

/*
 * Ethernet related
 */
#define PHY_ANEG_TIMEOUT		800

/* Uboot size */
#define CONFIG_SYS_MONITOR_LEN (1024 * 1024)

#define CONFIG_ENV_OVERWRITE

#define CONFIG_ASPEED_TIMER_CLK (1*1000*1000) /* 1MHz timer */

/*============ Memory Maps For SPL ================================*/
/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		(ASPEED_DRAM_BASE + CONFIG_ASPEED_SSP_RERV_MEM)

/* Flash */
#define CONFIG_SPL_TEXT_BASE     	0x00000000 // FMC MMIO mapped
#define CONFIG_SPL_MAX_FOOTPRINT  0x40000   // 256KB

/* SRAM */
#define CONFIG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE)
#define CONFIG_SYS_INIT_RAM_SIZE	(ASPEED_SRAM_SIZE)
#define SYS_INIT_RAM_END \
  ( CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)

/* The Top 2KB is reserved for VBS and reboot flags */
#define VBOOT_RESERVE_SZ   (0x800) // 2KB
/* The next 2KB reserved for TPM event log */
#define TPM_EVENT_LOG_SRAM_SIZE (0x800) // 2KB
/*
 * common/board_init.c::board_init_f_alloc_reserve will allocate
 * (SPL)_SYS_MALLOC_F_LEN = 12K for malloc and GD from
 * CONFIG_SYS_INIT_SP_ADDR, and let c-runtime stack growing down
 * from base of GD. sp only set to CONFIG_SYS_INIT_SP_ADDR
 * before c-runtime (refer to crt0.S)
 * To reserve space for VBS,
 * CONFIG_SYS_INIT_SP_ADDR=SYS_INIT_RAM_END - VBOOT_RESERVE_SZ
 * CONFIG_SPL_SYS_MALLOC_F_LEN by default equal SYS_MALLOC_F_LEN
 * which must define in kconfig, while CONFIG_MALLOC_F_ADDR still
 * need define in config header file
 */
#define CONFIG_SYS_INIT_SP_ADDR \
	(SYS_INIT_RAM_END - VBOOT_RESERVE_SZ - TPM_EVENT_LOG_SRAM_SIZE)

#define SRAM_HEAP_MINI_SIZE     0x3000 // 12KB
#ifdef CONFIG_SPL_SYS_MALLOC_F_LEN
# if (CONFIG_SPL_SYS_MALLOC_F_LEN < SRAM_HEAP_MINI_SIZE )
#   error "CONFIG_SPL_SYS_MALLOC_F_LEN is too small"
# endif
#else
# define CONFIG_SPL_SYS_MALLOC_F_LEN SRAM_HEAP_MINI_SIZE
#endif
#define CONFIG_MALLOC_F_ADDR (CONFIG_SYS_INIT_SP_ADDR - CONFIG_SPL_SYS_MALLOC_F_LEN)

/*============= UBoot Common setup ===============*/

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x300000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x5000000)

#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* Memory Info */
#define CONFIG_SYS_LOAD_ADDR		0x83000000

/*============= Watch dog configuration ===============*/
#define CONFIG_ASPEED_ENABLE_WATCHDOG

#if defined(CONFIG_TEST_ASPEED_WATCHDOG_SPL)
# define CONFIG_ASPEED_WATCHDOG_SPL_TIMEOUT	(50)    /* 50 seconds */
#else
# define CONFIG_ASPEED_WATCHDOG_SPL_TIMEOUT	(5*60)    /* 5 minutes */
#endif

#if defined(CONFIG_TEST_ASPEED_WATCHDOG_UBOOT) && \
    !defined(CONFIG_ASPEED_RECOVERY_BUILD)
# define CONFIG_ASPEED_WATCHDOG_TIMEOUT	(50)    /* 50 seconds */
#else
# define CONFIG_ASPEED_WATCHDOG_TIMEOUT	(5*60) /* 5 minutes */
#endif

/* u-boot reset command and reset() is implemented via sysreset
 * which will trigger the WDT to do full-chip reset by default.
 * But WDT Full-chip reset cannot successfully drive the WDTRST_N pin,
 * so define AST_SYSRESET_WITH_SOC to make sysreset use SOC reset,
 * and use AST_SYS_RESET_WITH_SOC as SOC reset mask #1.
 * Notice: For system which loop back the WDTRST_N pin to BMC SRST pin.
 * the value of AST_SYSRESET_WITH_SOC does not matter, because
 * no matter how the BMC will get full reset by SRST pin.
 */
#define AST_SYSRESET_WITH_SOC 0x030f1ff1


/*====== Verified boot configuration ==========*/
#ifdef CONFIG_SPL
#define CONFIG_CMD_VBS
#endif

#define CONFIG_CS0_SPL_KERNEL_LOAD    "201A0000"
#define CONFIG_CS1_SPL_KERNEL_LOAD    "281A0000"

#ifdef CONFIG_SPL
#	ifdef CONFIG_ASPEED_RECOVERY_BUILD
#		define CONFIG_KERNEL_LOAD        CONFIG_CS0_SPL_KERNEL_LOAD
#	else
#		define CONFIG_KERNEL_LOAD        CONFIG_CS1_SPL_KERNEL_LOAD
#	endif
#	define CONFIG_SYS_REMAP_BASE     CONFIG_SYS_TEXT_BASE
#	define CONFIG_SYS_UBOOT_START    CONFIG_SYS_TEXT_BASE
#	define CONFIG_SYS_SPL_FIT_BASE   (CONFIG_SYS_TEXT_BASE - 0x4000)
# define CONFIG_SYS_RECOVERY_BASE  (CONFIG_SYS_TEXT_BASE - 0x8104000 + 0x40000)
#	define CONFIG_RECOVERY_UBOOT_SIZE 0xA0000
  /* ENV setup */
#	define CONFIG_SYS_ENV_BASE       0x28000000
# define CONFIG_USE_ENV_SPI_BUS
#	define CONFIG_ENV_SPI_BUS 0
# define CONFIG_USE_ENV_SPI_CS
#	define CONFIG_ENV_SPI_CS 1
  /* Prevent the Recovery build from using the RW environment. */
# if defined(CONFIG_ASPEED_RECOVERY_BUILD) || defined(CONFIG_SPL_BUILD)
#   ifndef CONFIG_ENV_IS_NOWHERE
#     define CONFIG_ENV_IS_NOWHERE
#   endif
# else
#   undef CONFIG_ENV_IS_NOWHERE
#	  define CONFIG_ENV_IS_IN_SPI_FLASH
# endif
  /* define commands */
# undef CONFIG_PREBOOT
# define CONFIG_POSTBOOT "vbs 6 60; bootm " CONFIG_CS0_SPL_KERNEL_LOAD "; "
#if CONFIG_IS_ENABLED(ASPEED_ENABLE_DUAL_BOOT_WATCHDOG) /* stop fmcwdt2 */
# define CONFIG_PRECLICOMMAND "echo stop fmcwdt2; mw 1e620064 0; "
#else /* stop wdt1 */
# if defined(CONFIG_CMD_VBS)
#   define CONFIG_PRECLICOMMAND "vbs interrupt; "
# else
#   define CONFIG_PRECLICOMMAND "echo stop wdt1; mw 1e78500c 0; "
# endif
#endif /* ASPEED_ENABLE_DUAL_BOOT_WATCHDOG */

/*====== non-vboot configuration ========*/
#else
#	define CONFIG_SYS_REMAP_BASE     0x00000000
#	define CONFIG_SYS_UBOOT_START    0x00000000
#	define CONFIG_SYS_ENV_BASE       0x20000000
#	define CONFIG_KERNEL_LOAD	    "20100000"
# define CONFIG_ENV_IS_IN_FLASH
# define CONFIG_POSTBOOT  ""
#endif

/*======= vboot and non-vboot common configuration ========*/
#ifdef CONFIG_QEMU_BUILD
# define QEMU_FMC_WORKAROUND "sf probe 0:0; "
#else
# define QEMU_FMC_WORKAROUND ""
#endif

#define CONFIG_USE_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND                                \
  QEMU_FMC_WORKAROUND                                     \
  "bootm " CONFIG_KERNEL_LOAD "; " /* Location of FIT */  \
  CONFIG_POSTBOOT

#define CONFIG_ENV_ADDR          (CONFIG_SYS_ENV_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_OFFSET		0xE0000
#define CONFIG_ENV_SECT_SIZE	 (64 << 10) /* 64 KiB */

#ifndef PAL_EXTRA_ENV
#define PAL_EXTRA_ENV ""
#endif

#define CONFIG_EXTRA_ENV_SETTINGS                       \
    "verify=no\0"                                       \
    "spi_dma=no\0"                                      \
    "initrd_high=a0000000\0"                            \
    PAL_EXTRA_ENV                                       \
    ""


#endif	/* __CONFIG_H */
