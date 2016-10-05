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
#define CONFIG_AST_FPGA_VER 4 /* for arm1176 */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_MISC_INIT_R		1
#define CONFIG_MACH_TYPE				MACH_TYPE_ASPEED
#ifdef CONFIG_ARCH_AST1070
#define CONFIG_AST_GPIO
#endif


#include <asm/arch/platform.h>

/* Misc CPU related */
//#define CONFIG_ARCH_CPU_INIT
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_CMDLINE_EDITING		1	/* command line history */
//#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser		*/

/* ------------------------------------------------------------------------- */

//Enable cache controller
#define CONFIG_SYS_DCACHE_OFF	1

/* ------------------------------------------------------------------------- */
/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE			(AST_DRAM_BASE)
#define CONFIG_SYS_INIT_RAM_ADDR		CONFIG_SYS_SDRAM_BASE /*(AST_SRAM_BASE)*/
#define CONFIG_SYS_INIT_RAM_SIZE		(32*1024)
#define CONFIG_SYS_INIT_RAM_END     (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR 	(CONFIG_SYS_INIT_RAM_END - GENERATED_GBL_DATA_SIZE)

#define CONFIG_NR_DRAM_BANKS	1
/////////////////////////////////////////////////////////////////
#define CONFIG_SYS_MEMTEST_START		CONFIG_SYS_SDRAM_BASE + 0x300000
#define CONFIG_SYS_MEMTEST_END			(CONFIG_SYS_MEMTEST_START + (80*1024*1024))
/*-----------------------------------------------------------------------*/

#define CONFIG_SYS_TEXT_BASE            0

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN   	(0x1000 + 4*1024*1024) /* malloc() len */


/*
 * Timer Set
 */
#define CONFIG_SYS_HZ				(1*1000*1000)	/* use external clk (1M) */

/*
 * Hardware drivers
 */
//#define CONFIG_FARADAYNIC
//#define CONFIG_DRIVER_ASPEED_I2C

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE		-4
#define CONFIG_SYS_NS16550_CLK			24000000
#define CONFIG_SYS_NS16550_COM1			AST_UART0_BASE
#define CONFIG_SYS_LOADS_BAUD_CHANGE
#define CONFIG_SERIAL1					1
#define CONFIG_CONS_INDEX				1
#define CONFIG_BAUDRATE					115200
//#define CONFIG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#include <config_cmd_default.h>

//
//#define CONFIG_SYS_MAX_FLASH_BANKS      1
//#define CONFIG_SYS_MAX_FLASH_SECT       (256)           /* max number of sectors on one chip */
//
/*
 * Command line configuration.
 */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_NETTEST

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * Environment Config
 */
#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTFILE		"all.bin"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
/* Monitor Command Prompt	 */
#define CONFIG_SYS_PROMPT		"ast# "

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_LOAD_ADDR		0x83000000	/* default load address */

#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 ramdisk_size=16384 root=/dev/ram rw init=/linuxrc mem=80M"

/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
#define CONFIG_AST_SPI_NOR    /* AST SPI NOR Flash */

#ifdef CONFIG_AST_SPI_NOR

#define CONFIG_FMC_CS			1
/*#define CONFIG_SPI0_CS		1 */

/*#define CONFIG_FLASH_DMA */
#define CONFIG_SYS_MAX_FLASH_BANKS 	(CONFIG_FMC_CS)
/*#define CONFIG_SYS_MAX_FLASH_SECT	(1024)	*/	/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	(8192)		/* max number of sectors on one chip */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR				(AST_FMC_CS0_BASE + 0x60000)

#endif

/* ------------------------------------------------------------------------- */
#define CONFIG_ENV_OFFSET				0x60000	/* environment starts here  */
#define CONFIG_ENV_SIZE					0x20000		/* Total Size of Environment Sector */

#define CONFIG_BOOTCOMMAND	"bootm 20080000 20300000"
#define CONFIG_ENV_OVERWRITE

#define AST2500_ENV_SETTINGS					\
	"verify=yes\0"	\
	"spi_dma=yes\0" \
	"update=tftp 80800000 ast2500.scr; so 80800000\0" \
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

/*-----------------------------------------------------------------------
 * WDT
 *-----------------------------------------------------------------------
 */
#define CONFIG_AST_WATCHDOG
//#define CONFIG_AST_WATCHDOG_ENABLE
#define CONFIG_AST_WATCHDOG_TIMEOUT		(1*1000*1000 *30)

#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

#define CONFIG_SYS_PCIE1_BASE		0x8c000000
#define CONFIG_SYS_PCIE1_CFG_BASE	0x8c000000
#define CONFIG_SYS_PCIE1_CFG_SIZE	0x01000000
#define CONFIG_SYS_PCIE1_MEM_BASE	0x8d000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x8d000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x01000000
#define CONFIG_SYS_PCIE1_IO_BASE	0x8e000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0x8e000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00100000

#define CONFIG_SYS_PCIE2_BASE		0x8f000000
#define CONFIG_SYS_PCIE2_CFG_BASE	0x8f000000
#define CONFIG_SYS_PCIE2_CFG_SIZE	0x01000000
#define CONFIG_SYS_PCIE2_MEM_BASE	0x90000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0x90000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x00100000
#define CONFIG_SYS_PCIE2_IO_BASE	0x91000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0x91000000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00100000
#endif

/* ------------------------------------------------------------------------- */
//1. DRAM Speed
//#define CONFIG_DRAM_1333       //
//#define CONFIG_DRAM_1600       // (default)
//#define CONFIG_DRAM_1866       //
// 2. UART5 message output	//
//#define	 CONFIG_DRAM_UART_38400
//3. DRAM Type
//#define CONFIG_DDR3_8GSTACK    // DDR3 8Gbit Stack die
//4. ECC Function enable
#define CONFIG_DRAM_ECC
#define	CONFIG_DRAM_ECC_SIZE	0x10000000

#endif	/* __CONFIG_H */
