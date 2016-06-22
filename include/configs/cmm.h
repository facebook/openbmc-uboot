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

#ifndef __CONFIG_H
#define __CONFIG_H

/* Uncommit the following line to enable JTAG in u-boot */
//#define CONFIG_ASPEED_ENABLE_JTAG

#define CONFIG_AST_FPGA_VER 4 /* for arm1176 */
#define CONFIG_ARCH_ASPEED
#define CONFIG_ARCH_AST2500
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_MACH_TYPE            MACH_TYPE_ASPEED
#define CONFIG_MISC_INIT_R          1
#define __LITTLE_ENDIAN             1

#define CONFIG_SYS_HZ               1000
#define CONFIG_ASPEED_TIMER_CLK     (1*1000*1000) /* use external clk (1M) */
#define CONFIG_SYS_TIMERBASE        AST_TIMER_BASE /* use timer 1 */

#include <asm/arch/platform.h>
#include <asm/arch/aspeed.h>

// Command line editor
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER  /* Use the HUSH parser */
#define CONFIG_SYS_PROMPT           "boot# " /* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE           256 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */

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
//2. DRAM Speed
//3. VGA Mode
//4. ECC Function enable
//5. UART Debug Message
#define CONFIG_DRAM_UART_OUT

#define CONFIG_NR_DRAM_BANKS    1 /* we have 1 bank of DRAM */
#define CONFIG_DRAM_ECC_SIZE    0x10000000

//#define CONFIG_ASPEED_NO_VIDEO

#define CONFIG_SYS_SDRAM_BASE   (AST_DRAM_BASE)
#define CONFIG_SYS_INIT_RAM_ADDR CONFIG_SYS_SDRAM_BASE /*(AST_SRAM_BASE)*/
#define CONFIG_SYS_INIT_RAM_SIZE (32*1024)
#define CONFIG_SYS_INIT_RAM_END  (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_TEXT_BASE    0x0 // SPI flash is mapped to 0x0 initially
#define CONFIG_SYS_LOAD_ADDR    0x83000000  /* default load address */

// Enable cache controller
#define CONFIG_SYS_DCACHE_OFF   1

#define CONFIG_MONITOR_BASE     TEXT_BASE
#define CONFIG_MONITOR_LEN      (192 << 10)

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN       (CONFIG_ENV_SIZE + 1*1024*1024)
#define CONFIG_SYS_GBL_DATA_SIZE    128 /* size in bytes reserved for initial data */

/*
 * Stack sizes,  The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE            (128*1024) /* regular stack */
#define CONFIG_STACKSIZE_IRQ        (4*1024)   /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ        (4*1024)   /* FIQ stack */


/*
 * FLASH Configuration
 */
#define CONFIG_AST_SPI_NOR          /* AST SPI NOR Flash */

#define PHYS_FLASH_1                0x20000000 /* Flash Bank #1 */
#define CONFIG_SYS_FLASH_BASE       PHYS_FLASH_1
#define CONFIG_FLASH_BANKS_LIST     { PHYS_FLASH_1 }

#define CONFIG_FMC_CS               1

#define CONFIG_SYS_MAX_FLASH_BANKS  1
#define CONFIG_SYS_MAX_FLASH_SECT   (8192) /* max # of sectors on one chip */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT (20*CONFIG_SYS_HZ)  /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT (20*CONFIG_SYS_HZ)  /* Timeout for Flash Write */


/*
 * Environment Config
 */
#define CONFIG_CMDLINE_TAG       1 /* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG        1
#define CONFIG_BOOTARGS          "debug console=ttyS1,9600n8 root=/dev/ram rw"
#define CONFIG_UPDATE            "tftp 80800000 ast2500.scr; so 80800000'"
#define CONFIG_BOOTCOMMAND       "bootm 20080000 20480000"
#define CONFIG_BOOTFILE          "flash-wedge"
#define CONFIG_ENV_IS_IN_FLASH   1
#define CONFIG_ENV_OFFSET        0x60000 /* environment starts here  */
#define CONFIG_ENV_SIZE          0x20000 /* # of bytes of env, 128k */
#define CONFIG_ENV_ADDR          (AST_FMC_CS0_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_EXTRA_ENV_SETTINGS                       \
    "verify=yes\0"                                      \
    "spi_dma=yes\0"                                     \
    ""
#define CONFIG_ASPEED_WRITE_DEFAULT_ENV
#define CONFIG_ENV_OVERWRITE     /* allow overwrite */
#define CONFIG_SYS_MAXARGS       16 /* max number of command args   */
#define CONFIG_SYS_BARGSIZE      CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */


/*
 * Boot delay
 */
#define CONFIG_BOOTDELAY         3 /* autoboot after 3 seconds */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT      \
    "autoboot in %d seconds (stop with 'Delete' key)...\n", bootdelay
#define CONFIG_AUTOBOOT_STOP_STR    "\x1b\x5b\x33\x7e" /* 'Delete', ESC[3~ */
#define CONFIG_ZERO_BOOTDELAY_CHECK


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DFL
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
//#define CONFIG_CMD_I2C /* i2c is not supported in the latest SDK */
//#define CONFIG_CMD_EEPROM /* EEPROM needs i2c */


/*
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE     -4
#define CONFIG_SYS_NS16550_CLK          24000000
#define CONFIG_SYS_NS16550_COM1         AST_UART1_BASE
#define CONFIG_CONS_INDEX               1
#define CONFIG_BAUDRATE                 9600
#define CONFIG_SYS_BAUDRATE_TABLE       { 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_LOADS_BAUD_CHANGE
#define CONFIG_ASPEED_COM               AST_UART1_BASE // UART1
#define CONFIG_ASPEED_COM_IER           (CONFIG_ASPEED_COM + 0x4)
#define CONFIG_ASPEED_COM_IIR           (CONFIG_ASPEED_COM + 0x8)
#define CONFIG_ASPEED_COM_LCR           (CONFIG_ASPEED_COM + 0xc)
#define CONFIG_ASPEED_COM_LSR           (CONFIG_ASPEED_COM + 0x14)


/*
 * I2C configuration
 */
/*
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED        100000
#define CONFIG_SYS_I2C_SLAVE        1
#define CONFIG_DRIVER_ASPEED_I2C
*/


/*
* EEPROM configuration
*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN  2
#define CONFIG_SYS_I2C_EEPROM_ADDR  0xa0

#define __BYTE_ORDER __LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD


/*
 * Network configuration
 */
#ifdef CONFIG_CMD_MII
#define CONFIG_MII          1
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_ADDR         0
#define CONFIG_PHY_REALTEK
#endif

#ifdef CONFIG_CMD_NET
#define CONFIG_ASPEEDNIC
#define CONFIG_NET_MULTI
#define CONFIG_MAC1_PHY_LINK_INTERRUPT
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
#define CONFIG_MAC1_PHY_SETTING     0
#define CONFIG_MAC2_PHY_SETTING     0
#define CONFIG_ASPEED_MAC_NUMBER  2
#define CONFIG_ASPEED_MAC_CONFIG  2 // config MAC2
#define _PHY_SETTING_CONCAT(mac) CONFIG_MAC##mac##_PHY_SETTING
#define _GET_MAC_PHY_SETTING(mac) _PHY_SETTING_CONCAT(mac)
#define CONFIG_ASPEED_MAC_PHY_SETTING \
  _GET_MAC_PHY_SETTING(CONFIG_ASPEED_MAC_CONFIG)
#define CONFIG_MAC_INTERFACE_CLOCK_DELAY    0x2255
#define CONFIG_RANDOM_MACADDR
//#define CONFIG_GATEWAYIP 192.168.0.1
//#define CONFIG_NETMASK   255.255.255.0
//#define CONFIG_IPADDR    192.168.0.45
//#define CONFIG_SERVERIP  192.168.0.81
#endif


/*
 * Watchdog
 */
#define CONFIG_ASPEED_ENABLE_WATCHDOG
#define CONFIG_ASPEED_WATCHDOG_TIMEOUT (5*60) // 5m

#endif  /* __CONFIG_H */
