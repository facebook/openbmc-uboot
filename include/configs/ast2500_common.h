/*
 * (C) Copyright 2004-Present
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __AST2500_CONFIG_H
#define __AST2500_CONFIG_H

#define MACH_TYPE_ASPEED  8888
#define CONFIG_MACH_TYPE  MACH_TYPE_ASPEED

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_AST_FPGA_VER 4
/*
 * TODO:
 * 1. define CONFIG_ARCH_ASPEED in defconfig
 * 2. replace CONFIG_ARCH_AST2500 with CONFOG_ASPEED_AST2500 in defconfig
 * 3. define CONFIG_SYS_I2C_ASPEED in defconfig
 * and move it to Kconfig
 */
#define CONFIG_SYS_I2C_ASPEED

#define CONFIG_ARCH_CPU_INIT


#include <asm/arch/ast-sdk/platform.h>

/* new */
#define CONFIG_SYS_DCACHE_OFF 1

/*
 * Flash type (mutex):
 *   CONFIG_SYS_FLASH_CFI
 *   CONFIG_FLASH_SPI
 *
 * There is potential to port the SPI driver to u-boot.
 * For now it is not used.
 *   CONFIG_SYS_ASPEED_FLASH_SPI
 *
 * Flash driver choices:
 *   CONFIG_FLASH_AST2300
 *
 * Flash configuration choices:
 *   CONFIG_2SPIFLASH
 *   CONFIG_FLASH_AST2300_DMA
 *   CONFIG_FLASH_SPIx2_Dummy
 *   CONFIG_FLASH_SPIx4_Dummy
 *
 * If using CONFIG_2SPIFLASH:
 *   PHYS_FLASH_2
 *   PHYS_FLASH_2_BASE
 *
 */

#define PHYS_FLASH_1                0x20000000 /* Flash Bank #1 */
#define PHYS_FLASH_2                0x28000000 /* Flash Bank #2 */
#define PHYS_FLASH_2_BASE           0x28000000 /* Base of Flash 1 */

#ifndef CONFIG_SPL_BUILD
#define CONFIG_2SPIFLASH
#define CONFIG_FLASH_BANKS_LIST     { PHYS_FLASH_1, PHYS_FLASH_2 }
#define CONFIG_FMC_CS               2
#define CONFIG_SYS_MAX_FLASH_BANKS  2
#else
#define CONFIG_FLASH_BANKS_LIST     { PHYS_FLASH_1 }
#define CONFIG_FMC_CS               1
#define CONFIG_SYS_MAX_FLASH_BANKS  1
#endif

#define CONFIG_SYS_FLASH_BASE       PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT   (8192) /* max # of sectors on one chip */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT (20*CONFIG_SYS_HZ)  /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT (20*CONFIG_SYS_HZ)  /* Timeout for Flash Write */

/*
 * DRAM Config
 */
#ifndef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS  1
#endif

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE     (AST_DRAM_BASE) /* used to be 0x40000000 */
#ifndef CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR    CONFIG_SYS_SDRAM_BASE /*(AST_SRAM_BASE)*/
#endif

#ifndef CONFIG_SYS_INIT_RAM_SIZE
#define CONFIG_SYS_INIT_RAM_SIZE    (32*1024)
#endif

#ifndef CONFIG_SYS_INIT_RAM_END
#define CONFIG_SYS_INIT_RAM_END     (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#endif

/*
 * SRAM configuration
 */
#define CONFIG_SYS_SRAM_BASE        AST_SRAM_BASE
#define CONFIG_SYS_SRAM_TOP         (CONFIG_SYS_SDRAM_BASE + 0x8000)
#define CONFIG_SYS_MEMTEST_START    CONFIG_SYS_SDRAM_BASE + 0x300000
#define CONFIG_SYS_MEMTEST_END      (CONFIG_SYS_MEMTEST_START + (80*1024*1024))

/*
 * U-Boot Entry Point (EP) and load location configuration.
 *
 * A board may set this specifically, for example a verified boot will change
 * the EP for U-Boot to become the location of MMIO/strapped SPI1.
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE    0x00000000
#endif
#ifndef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)
#endif

#define CONFIG_SYS_UBOOT_BASE   CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_LOAD_ADDR    0x83000000  /* default load address */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE   256   /* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS    16    /* max number of command args */
#define CONFIG_SYS_BARGSIZE   CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size  */

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN       (CONFIG_ENV_SIZE + 1*1024*1024)
#define CONFIG_SYS_GBL_DATA_SIZE    128 /* size in bytes reserved for initial data */

/*
 * Stack sizes
 */
#define CONFIG_STACKSIZE            (128*1024) /* regular stack */
#define CONFIG_STACKSIZE_IRQ        (4*1024)   /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ        (4*1024)   /* FIQ stack */

/*
 * Environment Config
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#endif

/*
 * Serial configuration
 * The board configuration must set:
 *   CONFIG_SYS_NS16550_COM1
 *   CONFIG_CONS_INDEX
 *   CONFIG_BAUDRATE
 *   CONFIG_ASPEED_COM
 *
 * The board may optionally configure:
 *   CONFIG_SYS_NS16550_MEM32
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE -4
#define CONFIG_SYS_NS16550_CLK    24000000
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_ASPEED_COM_IER (CONFIG_ASPEED_COM + 0x4)
#define CONFIG_ASPEED_COM_IIR (CONFIG_ASPEED_COM + 0x8)
#define CONFIG_ASPEED_COM_LCR (CONFIG_ASPEED_COM + 0xc)
#define CONFIG_ASPEED_COM_LSR (CONFIG_ASPEED_COM + 0x14)

/*
 * NIC configuration
 */
#define CONFIG_NET_MULTI

/*
 * Timer
 */
#define CONFIG_ASPEED_TIMER_CLK   (1*1000*1000) /* use external clk (1M) */

/*
 * NOTICE: MAC1 and MAC2 now have their own separate PHY configuration.
 * We use 2 bits for each MAC in the scratch register(D[15:11] in 0x1E6E2040) to
 * inform kernel driver.
 * The meanings of the 2 bits are:
 * 00(0): Dedicated PHY
 * 01(1): ASPEED's EVA + INTEL's NC-SI PHY chip EVA
 * 10(2): ASPEED's MAC is connected to NC-SI PHY chip directly
 * 11: Reserved
 *
 * We use CONFIG_MAC1_PHY_SETTING and CONFIG_MAC2_PHY_SETTING in U-Boot
 * 0: Dedicated PHY
 * 1: ASPEED's EVA + INTEL's NC-SI PHY chip EVA
 * 2: ASPEED's MAC is connected to NC-SI PHY chip directly
 * 3: Reserved
 *
 * Configuration must define:
 *   CONFIG_MAC1_PHY_SETTING
 *   CONFIG_MAC2_PHY_SETTING
 */
#define CONFIG_MAC_INTERFACE_CLOCK_DELAY  0x2255
#define _PHY_SETTING_CONCAT(mac) CONFIG_MAC##mac##_PHY_SETTING
#define _GET_MAC_PHY_SETTING(mac) _PHY_SETTING_CONCAT(mac)
#define CONFIG_ASPEED_MAC_PHY_SETTING _GET_MAC_PHY_SETTING(CONFIG_ASPEED_MAC_CONFIG)

#endif
