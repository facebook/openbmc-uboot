/*
 * (C) Copyright 2004-Present
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __AST2400_CONFIG_H
#define __AST2400_CONFIG_H

#define MACH_TYPE_ASPEED	8888
#define CONFIG_MACH_TYPE	MACH_TYPE_ASPEED

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM926EJS
#define	CONFIG_ASPEED
#define CONFIG_AST2400

#define CONFIG_ARCH_ASPEED
#define CONFIG_ARCH_AST2400

#define CONFIG_MISC_INIT_R

/*
 * Must disable dcache as the aspeednic.c does not flush and invalidate caches
 * correctly during DMA.
 */
#define CONFIG_SYS_DCACHE_OFF

#include <asm/arch/platform.h>
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
 * It may also be necessary to configure the set of flash banks.
 * When using the AST2300, the following flash configuration will be available:
 *  CE0 = 0x20000000 - 0x23FFFFFF (64MB)
 *  CE1 = 0x24000000 - 0x25FFFFFF (32MB)
 *  CE2 = 0x26000000 - 0x27FFFFFF (32MB)
 *  CE3 = 0x28000000 - 0x29FFFFFF (32MB)
 ONFIG_SYS_TEXT_BASEㄥ
 */
#define CONFIG_FLASH_AST2300

#define CONFIG_AST_SPI_NOR
#define PHYS_FLASH_1                0x20000000 /* Flash Bank #1 */
#define PHYS_FLASH_2                0x24000000 /* Flash Bank #2 */
#define PHYS_FLASH_2_BASE           0x24000000 /* Base of Flash 1 */

/*
 * DRAM Config
 *
 * 1. DRAM Size              //
 *    CONFIG_DRAM_512MBIT    // 512M bit
 *    CONFIG_DRAM_1GBIT      // 1G   bit (default)
 *    CONFIG_DRAM_2GBIT      // 2G   bit
 *    CONFIG_DRAM_4GBIT      // 4G   bit
 * 2. DRAM Speed             //
 *    CONFIG_DRAM_336        // 336MHz (DDR-667)
 *    CONFIG_DRAM_408        // 408MHz (DDR-800) (default)
 * 3. VGA Mode
 *    CONFIG_CRT_DISPLAY     // define to disable VGA function
 * 4. ECC Function enable
 *    CONFIG_DRAM_ECC        // define to enable ECC function
 * 5. UART Debug Message
 *    CONFIG_DRAM_UART_OUT   // enable output message at UART5
 *    CONFIG_DRAM_UART_38400 // set the UART baud rate to 38400, default is 115200
 */
#define CONFIG_DRAM_1GBIT
#define CONFIG_DRAM_408
#define CONFIG_DRAM_UART_OUT

/*
 * Environment Config
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * CPU Setting
 */
#define CPU_CLOCK_RATE		18000000	/* 16.5 MHz clock for the ARM core */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 768*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Stack sizes,  The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */

/*
 * Memory Configuration
 * The board configuration must set: PHYS_SDRAM_1_SIZE
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x40000000	/* SDRAM Bank #1 */
#define CONFIG_SYS_SDRAM_BASE	0x40000000
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

/*
 * Flash Configuration
 */
#ifdef CONFIG_SYS_FLASH_CFI
#ifdef CONFIG_FLASH_AST2300
#define PHYS_FLASH_1			0x20000000  /* Flash Bank #1 */
#else
#define PHYS_FLASH_1			0x10000000  /* Flash Bank #1 */
#endif
#define CONFIG_SYS_MAX_FLASH_SECT	(256)
#define CONFIG_SYS_FLASH_CFI_AMD_RESET
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#else	/* SPI Flash */
#ifdef CONFIG_FLASH_AST2300
#define PHYS_FLASH_1			0x20000000 	/* Flash Bank #1 */
#else
#define PHYS_FLASH_1			0x14000000 	/* Flash Bank #1 */
#endif
#define CONFIG_SYS_MAX_FLASH_SECT	(1024)
#endif

/*
 * Additional flash configuration
 */
#define CONFIG_SYS_TEXT_BASE		0x00000000
#ifdef CONFIG_2SPIFLASH
#define CONFIG_SYS_FLASH_BASE           PHYS_FLASH_1
#define CONFIG_FLASH_BANKS_LIST         { PHYS_FLASH_1, PHYS_FLASH_2 }
#define CONFIG_FMC_CS                   2
#define CONFIG_SYS_MAX_FLASH_BANKS      2
#else
#define CONFIG_SYS_FLASH_BASE           PHYS_FLASH_1
#define CONFIG_FLASH_BANKS_LIST         { PHYS_FLASH_1 }
#define CONFIG_FMC_CS                   1
#define CONFIG_SYS_MAX_FLASH_BANKS      1
#endif

#define CONFIG_SYS_MONITOR_LEN		(192 << 10)
#define CONFIG_SYS_FLASH_ERASE_TOUT	(20*CONFIG_SYS_HZ) 	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(20*CONFIG_SYS_HZ) 	/* Timeout for Flash Write */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE

#define __LITTLE_ENDIAN			1
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_MEMTEST_START	0x40000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x44FFFFFF	/* 256 MB in DRAM	*/
#define CONFIG_SYS_LOAD_ADDR		0x43000000	/* default load address */

/*
 * Timer configuration
 */
#define CONFIG_ASPEED_TIMER_CLK		(1*1000*1000)	/* use external clk (1M) */

/*
 * Serial configuration
 * The board configuration must set:
 *   CONFIG_SYS_NS16550_REG_SIZE
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
#define CONFIG_SYS_NS16550_CLK		24000000
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_ASPEED_COM_IER (CONFIG_ASPEED_COM + 0x4)
#define CONFIG_ASPEED_COM_IIR (CONFIG_ASPEED_COM + 0x8)
#define CONFIG_ASPEED_COM_LCR (CONFIG_ASPEED_COM + 0xc)
#define CONFIG_ASPEED_COM_LSR (CONFIG_ASPEED_COM + 0x14)

/*
 * USB device configuration
 * To configure/enable USB:
 *   CONFIG_USB_DEVICE
 *   CONFIG_USB_TTY
 *   CONFIG_USBD_VENDORID 0x1234
 *   CONFIG_USBD_PRODUCTID  0x5678
 *   CONFIG_USBD_MANUFACTURER "Siemens"
 *   CONFIG_USBD_PRODUCT_NAME "SX1"
 */

/*
 * I2C configuration
 */

/*
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_ASPEED
*/

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_I2C_EEPROM_ADDR	0xa0


/*
 * NIC configuration
 */
#define CONFIG_ASPEEDNIC
#define CONFIG_NET_MULTI

/*
 * Watchdog configuration
 */
#define CONFIG_AST_WDT_BASE	0x1e785000
#define CONFIG_AST_WDT2_BASE	0x1e785020
#define CONFIG_AST_WDT_CLK	(1 * 1000 * 1000)

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
#define CONFIG_MAC_INTERFACE_CLOCK_DELAY	0x2255
#define _PHY_SETTING_CONCAT(mac) CONFIG_MAC##mac##_PHY_SETTING
#define _GET_MAC_PHY_SETTING(mac) _PHY_SETTING_CONCAT(mac)
#define CONFIG_ASPEED_MAC_PHY_SETTING _GET_MAC_PHY_SETTING(CONFIG_ASPEED_MAC_CONFIG)

#endif	/* __AST2400_CONFIG_H */
