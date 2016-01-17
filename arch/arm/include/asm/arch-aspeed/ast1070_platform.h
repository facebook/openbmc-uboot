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

#ifndef _AST1070_PLATFORM_H_
#define _AST1070_PLATFORM_H_                 1

#define AST1070_UART0_BASE(lpc)						(lpc + 0x000)	/* Companion UART1 */
#define AST1070_UART1_BASE(lpc)						(lpc + 0x400)	/* Companion UART2 */
#define AST1070_UART2_BASE(lpc)						(lpc + 0x800)	/* Companion UART3 */
#define AST1070_UART3_BASE(lpc)						(lpc + 0xc00)	/* Companion UART4 */
#define AST1070_LPC0_BASE(lpc)						(lpc + 0x1000)	/* Companion LPC1 */
#define AST1070_LPC1_BASE(lpc)						(lpc + 0x1400)	/* Companion LPC2 */
#define AST1070_LPC2_BASE(lpc)						(lpc + 0x1800)	/* Companion LPC3 */
#define AST1070_LPC3_BASE(lpc)						(lpc + 0x1c00)	/* Companion LPC4 */
#define AST1070_SCU_BASE(lpc)						(lpc + 0x2000)	/* Companion SCU */
#define AST1070_VIC_BASE(lpc)						(lpc + 0x2400)	/* Companion VIC */
#define AST1070_LPC_SLAVE_BASE(lpc)					(lpc + 0x2c00)	/* Companion LPC SlLAVE */
#define AST1070_I2C_BASE(lpc)						(lpc + 0x3000)	/* Companion I2C */
#define AST1070_SPI_BASE(lpc)						(lpc + 0x4000)	/* Companion SPI */
#define AST1070_LPC_SPI_BASE(lpc)					(lpc + 0x4400)	/* Companion LPC SPI */
#define AST1070_UART_DMA_BASE(lpc)					(lpc + 0x4800)	/* Companion UART DMA */
#define AST1070_SPI_CONTROL_BASE(lpc)				(lpc + 0x4c00)	/* Companion SPI CONTROL */
#define AST1070_SPI_SHADOW_SRAM_BASE(lpc)			(lpc + 0x5000)	/* Companion SPI SHADOW SRAM */

#endif
