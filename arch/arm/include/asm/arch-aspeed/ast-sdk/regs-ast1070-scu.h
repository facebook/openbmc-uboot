/* arch/arm/mach-aspeed/include/mach/regs-ast1070-scu.h
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *   History      : 
 *    1. 2013/05/15 Ryan Chen Create
 * 
********************************************************************************/
#ifndef __AST1070_SCU_H
#define __AST1070_SCU_H                     1

/*
 *  Register for SCU
 *  */
#define AST1070_SCU_PROTECT			0x00		/*	protection key register	*/
#define AST1070_SCU_RESET			0x04		/*	system reset control register */
#define AST1070_SCU_MISC_CTRL		0x08		/*	misc control register	*/
#define AST1070_SCU_UART_MUX		0x0C		/*	UART Mux control register	*/
#define AST1070_SCU_SPI_USB_MUX		0x10		/*	SPI/USB Mux control register	*/
#define AST1070_SCU_IO_DRIVING		0x14		/*	I/O Driving Strength control register	*/
#define AST1070_SCU_IO_PULL			0x18		/*	I/O Internal Pull control register	*/
#define AST1070_SCU_IO_SLEW			0x1C		/*	I/O Slew Rate control register	*/
#define AST1070_SCU_IO_SCHMITT		0x20		/*	I/O Schmitt Trigger Control register	*/
#define AST1070_SCU_IO_SELECT		0x24		/*	I/O Port Selection register	*/
#define AST1070_SCU_TRAP			0x30		/*	HW TRAPPING register	*/
#define AST1070_SCU_CHIP_ID			0x34		/*	CHIP ID register	*/


/*	AST1070_SCU_PROTECT: 0x00  - protection key register */
#define AST1070_SCU_PROTECT_UNLOCK			0x16881A78

/*	AST1070_SCU_RESET :0x04	 - system reset control register */
#define SCU_RESET_DMA				(0x1 << 11)
#define SCU_RESET_SPI_M				(0x1 << 10)
#define SCU_RESET_SPI_S				(0x1 << 9)
#define SCU_RESET_N4_LPC			(0x1 << 8)
#define SCU_RESET_N3_LPC			(0x1 << 7)
#define SCU_RESET_N2_LPC			(0x1 << 6)
#define SCU_RESET_N1_LPC			(0x1 << 5)
#define SCU_RESET_I2C				(0x1 << 4)
#define SCU_RESET_N4_UART			(0x1 << 3)
#define SCU_RESET_N3_UART			(0x1 << 2)
#define SCU_RESET_N2_UART			(0x1 << 1)
#define SCU_RESET_N1_UART			(0x1 << 0)

/*	AST1070_SCU_MISC_CTRL		0x08		misc control register	*/
#define SCU_DMA_M_S_MASK			(0x3 << 9)

#define SCU_DMA_SLAVE_EN			(0x1 << 10)
#define SCU_DMA_MASTER_EN			(0x1 << 9)


#endif

