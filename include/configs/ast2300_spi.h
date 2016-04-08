/*
 * (C) Copyright 2004
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
//#define CONFIG_INIT_CRITICAL			/* define for U-BOOT 1.1.1 */
#undef  CONFIG_INIT_CRITICAL			/* undef for  U-BOOT 1.1.4 */ 
#define CONFIG_ARM926EJS	1		/* This is an arm926ejs CPU */
#define	CONFIG_ASPEED		1
#define CONFIG_AST2300		1
//#define CONFIG_SYS_FLASH_CFI			/* CONFIG_FLASH_CFI, CONFIG_FLASH_SPI is exclusive*/
#define CONFIG_FLASH_SPI
//#define CONFIG_2SPIFLASH			/* Boot SPI: CS2, 2nd SPI: CS0 */
#undef CONFIG_2SPIFLASH
#undef CONFIG_ASPEED_SLT
#define CONFIG_FLASH_AST2300
#define CONFIG_FLASH_AST2300_DMA
//#define CONFIG_FLASH_SPIx2_Dummy
//#define CONFIG_FLASH_SPIx4_Dummy
#define CONFIG_CRT_DISPLAY	1		/* undef if not support CRT */

//#define CONFIG_USE_IRQ				/* we don't need IRQ/FIQ stuff */
#define CONFIG_MISC_INIT_R 

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
 
//1. DRAM Size
//#define    CONFIG_DRAM_512MBIT
#define    CONFIG_DRAM_1GBIT
//#define    CONFIG_DRAM_2GBIT
//#define    CONFIG_DRAM_4GBIT
//2. DRAM Speed
//#define    CONFIG_DRAM_336
#define    CONFIG_DRAM_408
//3. VGA Mode
//#define    CONFIG_CRT_DISPLAY
//4. ECC Function enable
//#define    CONFIG_DRAM_ECC
//5. UART Debug Message
#define    CONFIG_DRAM_UART_OUT
//#define    CONFIG_DRAM_UART_38400



/* 
 * Environment Config
 */
#define CONFIG_CMDLINE_TAG	 1		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1
#define	CONFIG_BOOTARGS 	"debug console=ttyS0,115200n8 ramdisk_size=16384 root=/dev/ram rw init=/linuxrc mem=80M"
#define CONFIG_BOOTDELAY	3		/* autoboot after 3 seconds	*/
#ifdef CONFIG_FLASH_AST2300
#define CONFIG_BOOTCOMMAND	"bootm 20080000 20300000"
#else
#ifdef	CONFIG_SYS_FLASH_CFI 
#define CONFIG_BOOTCOMMAND	"bootm 10080000 10300000"
#else
#define CONFIG_BOOTCOMMAND	"bootm 14080000 14300000"
#endif
#endif
#define CONFIG_BOOTFILE		"all.bin"
#define CONFIG_ENV_OVERWRITE

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DFL
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM

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
 */
#define CONFIG_NR_DRAM_BANKS	1	   	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x40000000 	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x4000000 	/* 64 MB */

#define CONFIG_SYS_SDRAM_BASE	0x40000000

/*
 * FLASH Configuration
 */
#ifdef CONFIG_SYS_FLASH_CFI				/* NOR Flash */

#ifdef CONFIG_FLASH_AST2300
#define PHYS_FLASH_1			0x20000000 	/* Flash Bank #1 */
#else
#define PHYS_FLASH_1			0x10000000 	/* Flash Bank #1 */
#endif

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_FLASH_BANKS_LIST 	{ PHYS_FLASH_1 }

#define CONFIG_SYS_MAX_FLASH_BANKS 	1
#define CONFIG_SYS_MAX_FLASH_SECT	(256)		/* max number of sectors on one chip */

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET			0x60000 	/* environment starts here  */
#define CONFIG_ENV_SIZE			0x20000 	/* Total Size of Environment Sector */

#define CONFIG_SYS_FLASH_CFI_AMD_RESET
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

#else						/* SPI Flash */

#ifdef CONFIG_FLASH_AST2300
#define PHYS_FLASH_1		0x20000000 	/* Flash Bank #1 */
#else
#define PHYS_FLASH_1		0x14000000 	/* Flash Bank #1 */
#define PHYS_FLASH_2		0x14800000 	/* Flash Bank #2 */
#define PHYS_FLASH_2_BASE	0x10000000
#endif

#ifdef CONFIG_2SPIFLASH
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_2_BASE
#define CONFIG_FLASH_BANKS_LIST 	{ PHYS_FLASH_1, PHYS_FLASH_2 }
#define CONFIG_SYS_MAX_FLASH_BANKS 	2
#define CONFIG_SYS_MAX_FLASH_SECT	(1024)		/* max number of sectors on one chip */

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_OFFSET		0x7F0000 	/* environment starts here  */
#define CONFIG_ENV_SIZE			0x010000 	/* Total Size of Environment Sector */
#else
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_FLASH_BANKS_LIST 	{ PHYS_FLASH_1 }
#define CONFIG_SYS_MAX_FLASH_BANKS 	1
#define CONFIG_SYS_MAX_FLASH_SECT	(1024)		/* max number of sectors on one chip */

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_OFFSET		0x7F0000 	/* environment starts here  */
#define CONFIG_ENV_SIZE			0x010000 	/* Total Size of Environment Sector */
#endif

#endif

#define __LITTLE_ENDIAN

#define CONFIG_MONITOR_BASE		TEXT_BASE
#define CONFIG_MONITOR_LEN		(192 << 10)

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(20*CONFIG_SYS_HZ) 	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(20*CONFIG_SYS_HZ) 	/* Timeout for Flash Write */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/

#define CONFIG_SYS_PROMPT		"boot# " 	/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x40000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x44FFFFFF	/* 256 MB in DRAM	*/

#define	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#undef	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CONFIG_SYS_LOAD_ADDR		0x43000000	/* default load address */

#define CONFIG_SYS_TIMERBASE		0x1E782000	/* use timer 1 */
#define CONFIG_SYS_HZ			(1*1000*1000)	/* use external clk (1M) */

/*
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	4
#define CONFIG_SYS_NS16550_CLK		24000000
#define CONFIG_SYS_NS16550_COM1		0x1e783000
#define CONFIG_SYS_NS16550_COM2		0x1e784000
#define	CONFIG_SYS_LOADS_BAUD_CHANGE
#define CONFIG_SERIAL1			1
#define CONFIG_CONS_INDEX		2
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * USB device configuration
 */
/*
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1

#define CONFIG_USBD_VENDORID		0x1234
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Siemens"
#define CONFIG_USBD_PRODUCT_NAME	"SX1"
*/

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_DRIVER_ASPEED_I2C

/*
* EEPROM configuration
*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 	2
#define CONFIG_SYS_I2C_EEPROM_ADDR 	0xa0

/*
 * NIC configuration
 */
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD
#define CONFIG_MAC_PARTITION
#define CONFIG_ASPEEDNIC
//#define CONFIG_MAC1_PHY_LINK_INTERRUPT
#define CONFIG_MAC2_ENABLE
#define CONFIG_MAC2_PHY_LINK_INTERRUPT
/*
*-------------------------------------------------------------------------------
* NOTICE: MAC1 and MAC2 now have their own seperate PHY configuration.
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
*-------------------------------------------------------------------------------
*/
#define CONFIG_MAC1_PHY_SETTING		0
#define CONFIG_MAC2_PHY_SETTING		0
#define CONFIG_MAC_INTERFACE_CLOCK_DELAY	0x2255
#define CONFIG_NET_MULTI
#define CONFIG_ETHACT			aspeednic#0
#define CONFIG_GATEWAYIP		192.168.0.1
#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_IPADDR			192.168.0.188
#define CONFIG_SERVERIP			192.168.0.106
#define CONFIG_ETHADDR			00:C0:A8:12:34:56
#define CONFIG_ETH1ADDR			00:C0:A8:12:34:57

/*
 * SLT
 */
/*
#define CONFIG_SLT
#define CFG_CMD_SLT		(CFG_CMD_VIDEOTEST | CFG_CMD_MACTEST | CFG_CMD_HACTEST | CFG_CMD_MICTEST)
*/

#endif	/* __CONFIG_H */
