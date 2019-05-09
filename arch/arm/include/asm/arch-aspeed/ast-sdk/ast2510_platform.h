/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _AST2510_PLATFORM_H_
#define _AST2510_PLATFORM_H_                 1

#define IO_ADDRESS(x)				(x)

#define PHYS_SDRAM_8M			0x800000 /* 8 MB */

#define AST_PLL_CLOCK			AST_PLL_24MHZ

#define MAP_AREA0_BASE			0x80000000	// Map area 0, flash
#define MAP_AREA1_BASE			0x00800000	// map area 1, map to I/O space
#define MAP_AREA2_BASE			0x80900000	// map area 2, map to internal SRAM
#define MAP_AREA3_BASE			0x81400000	// map area 3, map to DRAM memory, big endian
#define MAP_AREA4_BASE			0x81500000	// map area 4, map to DRAM memory, big endian
#define MAP_AREA5_BASE			0x81600000	// map area 5, map to DRAM memory, big endian
#define MAP_AREA6_BASE			0x81700000	// map area 6, map to DRAM memory, big endian
#define MAP_AREA7_BASE			0x81800000	// map area 7, map to DRAM memory, big endian

#define REMAP_AREA0_BASE			0x00000000	// Map to flash
#define REMAP_AREA1_BASE			0x00200000	// Remap area 1, flash
#define REMAP_AREA2_BASE			0x00300000	// Remap area 2, flash
#define REMAP_AREA3_BASE			0x00400000	// Remap area 3, map to DRAM memory
#define REMAP_AREA4_BASE			0x00500000	// Remap area 4, map to DRAM memory
#define REMAP_AREA5_BASE			0x00600000	// Remap area 5, map to DRAM memory
#define REMAP_AREA6_BASE			0x00700000	// Remap area 6, map to DRAM memory


#define FLASH_BASE_ADDR			REMAP_AREA0_BASE
#define IO_BASE0_ADDR				0x00200000
#define IO_BASE1_ADDR				0x00300000

#define AST_DRAM_BASE				0x00800000

#define AST_SPI0_MEM				REMAP_AREA0_BASE
#define AST_SPI1_MEM				(REMAP_AREA2_BASE + 0x00018000)

#define AST_FMC_BASE				(IO_BASE0_ADDR + 0x00020000)	/* CPU SPI Memory Controller, FMC */
#define AST_SPI0_BASE				(IO_BASE0_ADDR + 0x00030000)	/* SPI2 Memory Controller */
#define AST_SPI1_BASE				(IO_BASE0_ADDR + 0x00031000)	/* SPI3 Memory Controller */
#define AST_SDMC_BASE				(IO_BASE0_ADDR + 0x000e0000)	/* SDMC Controller */
#define AST_SCU_BASE				(IO_BASE0_ADDR + 0x000e2000)	/* SCU Controller */

#define AST_SRAM_BASE				(IO_BASE1_ADDR + 0x00020000)	/* SRAM */
#define AST_UART1_BASE				(IO_BASE1_ADDR + 0x00083000)	/* UART1 */
#define AST_UART0_BASE				(IO_BASE1_ADDR + 0x00084000)	/* UART5 system debug console */
#define AST_WDT_BASE				(IO_BASE1_ADDR + 0x00085000)	/* WDT */
#define AST_UART2_BASE				(IO_BASE1_ADDR + 0x0008D000)	/* UART2 */
#define AST_TIMER_BASE				(IO_BASE1_ADDR + 0x00082000)	/* Timer */

#endif
