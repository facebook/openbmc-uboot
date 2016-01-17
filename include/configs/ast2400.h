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

//#define DEBUG	1
//#define CONFIG_SKIP_LOWLEVEL_INIT
//#define CONFIG_ASPEED_FPGA

/* High Level Configuration Options (easy to change) */
#define CONFIG_ARM926EJS				1				/* This is an arm926ejs CPU core */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_MISC_INIT_R				1
#define CONFIG_MACH_TYPE				MACH_TYPE_ASPEED
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_ARCH_AST1070
#define CONFIG_AST_GPIO
#endif
/* ------------------------------------------------------------------------- */
//Enable cache controller
#define CONFIG_SYS_DCACHE_OFF			1

#define CONFIG_SYS_TEXT_BASE            0x0

/*use char ('a', 'b') or ASCII code (0x41 for 'A') to press stop boot*/
//#define CONFIG_BOOTLOADER_PRESSKEY 	'a'		

#include <asm/arch/platform.h>
#include <asm/arch/aspeed.h>

/* Misc CPU related */
//#define CONFIG_ARCH_CPU_INIT

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_CMDLINE_EDITING		1	/* command line history */
//#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser		*/

/* ------------------------------------------------------------------------- */



/* ------------------------------------------------------------------------- */
/* additions for new relocation code, must added to all boards */
#define CONFIG_NR_DRAM_BANKS				1
#define CONFIG_SYS_SDRAM_BASE				(AST_DRAM_BASE)
#define CONFIG_SYS_INIT_RAM_ADDR			CONFIG_SYS_SDRAM_BASE /* (AST_SRAM_BASE)*/
#define CONFIG_SYS_INIT_RAM_SIZE			(32*1024)
#define CONFIG_SYS_INIT_RAM_END     		(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR 			(CONFIG_SYS_INIT_RAM_END - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN   			(0x1000 + 4*1024*1024) /* malloc() len */
/////////////////////////////////////////////////////////////////
#define CONFIG_SYS_MEMTEST_START			CONFIG_SYS_SDRAM_BASE + 0x300000
#define CONFIG_SYS_MEMTEST_END			(CONFIG_SYS_MEMTEST_START + (80*1024*1024))
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Timer Set */
#define CONFIG_SYS_HZ				(1*1000*1000)	/* use external clk (1M) */

/* device drivers  */

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE		(-4)
#define CONFIG_SYS_NS16550_CLK			24000000
#define CONFIG_SYS_NS16550_COM1			AST_UART0_BASE
#define CONFIG_SYS_LOADS_BAUD_CHANGE
#define CONFIG_SERIAL1					1
#define CONFIG_CONS_INDEX				1
#define CONFIG_BAUDRATE					115200

/* Command line configuration */
#include <config_cmd_default.h>
/* Command line configuration. */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NETTEST

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/* Environment Config */
#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTFILE		"all.bin"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
/* Monitor Command Prompt	 */
#define CONFIG_SYS_PROMPT		"ast# " 

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_LOAD_ADDR				0x43000000	/* default load address */

#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 ramdisk_size=16384 root=/dev/ram rw init=/linuxrc mem=80M"

/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
#define CONFIG_AST_SPI_NOR    /* AST SPI NOR Flash */

#ifdef CONFIG_AST_SPI_NOR

#define CONFIG_FMC_CS			1

/*#define CONFIG_FLASH_DMA */
#define CONFIG_SYS_MAX_FLASH_BANKS 	(CONFIG_FMC_CS)
/*#define CONFIG_SYS_MAX_FLASH_SECT	(1024)	*/	/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	(8192)		/* max number of sectors on one chip */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR				(AST_FMC_CS0_BASE + 0x60000)
#endif

/* ------------------------------------------------------------------------- */
/* Environment Config */

#define CONFIG_ENV_OFFSET					0x60000	/* environment starts here  */
#define CONFIG_ENV_SIZE					0x20000		/* Total Size of Environment Sector */

#define CONFIG_BOOTCOMMAND	"bootm 20080000 20300000"
#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"verify=yes\0"	\
	"spi_dma=yes\0" \
	"update=tftp 40800000 ast2400.scr; so 40800000\0" \
	""

/* ------------------------------------------------------------------------- */

/* Ethernet */
#ifdef CONFIG_CMD_MII
#define CONFIG_MII			1
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_ADDR			0
#define CONFIG_PHY_REALTEK
#endif
//
#ifdef CONFIG_CMD_NET
//#define CONFIG_MAC_NUM	2
#define CONFIG_FTGMAC100
#define CONFIG_PHY_MAX_ADDR	32	/* this comes from <linux/phy.h> */
//#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_FTGMAC100_EGIGA

#define CONFIG_GATEWAYIP		192.168.0.1
#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_IPADDR			192.168.0.45
#define CONFIG_SERVERIP			192.168.0.81
#define CONFIG_ETHADDR			00:C0:A8:12:34:56
#define CONFIG_ETH1ADDR			00:C0:A8:12:34:57

#endif


/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
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

//#define CONFIG_CPU_384 1
//#define CONFIG_CPU_408 1
#define CONFIG_CPU_420 1
//#define CONFIG_CPU_432 1
//#define CONFIG_CPU_456 1
//#define CONFIG_DRAM_336 1
//#define CONFIG_DRAM_408 1
//#define CONFIG_DRAM_504 1
#define CONFIG_DRAM_528 1
//#define CONFIG_DRAM_552 1
//#define CONFIG_DRAM_576 1
//#define CONFIG_DRAM_624 1
#define CONFIG_CRT_DISPLAY	1		/* undef if not support CRT */
//#define CONFIG_FLASH_AST2300      //Enable new 2300 flash interface
//#define CONFIG_FLASH_AST2300_DMA
//#define CONFIG_FLASH_SPIx2_Dummy
//#define CONFIG_FLASH_SPIx4_Dummy

#endif	/* __CONFIG_H */ 
