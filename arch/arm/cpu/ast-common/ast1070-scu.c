/********************************************************************************
* File Name     : arch/arm/mach-aspeed/ast1070-scu.c 
* Author         : Ryan Chen
* Description   : AST1070 SCU 
* 
* Copyright (C) 2012-2020  ASPEED Technology Inc.
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by the Free Software Foundation; 
* either version 2 of the License, or (at your option) any later version. 
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
* You should have received a copy of the GNU General Public License 
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

*   History      : 
*    1. 2013/05/15 Ryan Chen Create
* 
********************************************************************************/
#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch/regs-ast1070-scu.h>
#include <asm/gpio.h>

#define CONFIG_AST1070_SCU_LOCK
//#define AST1070_SCU_DEBUG

#ifdef AST1070_SCU_DEBUG
#define SCUDBUG(fmt, args...) printf("%s() " fmt, __FUNCTION__, ## args)
#else
#define SCUDBUG(fmt, args...)
#endif

#define SCUMSG(fmt, args...) printf(fmt, ## args)

static u32 ast1070_scu_base = 0;

static inline u32 
ast1070_scu_read(u8 node, u32 reg)
{
	u32 val;

	val = readl(ast1070_scu_base + (node * 0x10000) + reg);
	SCUDBUG("ast1070_scu_read : reg = 0x%08x, val = 0x%08x\n", reg, val);
	
	return val;
}

static inline void
ast1070_scu_write(u8 node, u32 val, u32 reg) 
{
	SCUDBUG("ast1070_scu_write : reg = 0x%08x, val = 0x%08x\n", reg, val);
#ifdef CONFIG_AST1070_SCU_LOCK
	//unlock 
	writel(AST1070_SCU_PROTECT_UNLOCK, ast1070_scu_base  + (node * 0x10000));
	writel(val, ast1070_scu_base + (node * 0x10000) + reg);
	//lock
//	writel(0xaa,ast1070_scu_base + (node * 0x10000));	
#else
	writel(val, ast1070_scu_base  + (node * 0x10000) + reg);
#endif
}

extern void
ast1070_scu_init_i2c(u8 node)
{
	//SCU I2C Reset 
	ast1070_scu_write(node, ast1070_scu_read(node, AST1070_SCU_RESET) & ~SCU_RESET_I2C, AST1070_SCU_RESET);
}

EXPORT_SYMBOL(ast1070_scu_init_i2c);

extern void
ast1070_dma_init(u8 node)
{
	u32 val =0;

	//let the uart_dma engine leave the reset state
	ast1070_scu_write(node, ast1070_scu_read(node, AST1070_SCU_RESET) & ~SCU_RESET_DMA, AST1070_SCU_RESET);

	val = ast1070_scu_read(node, AST1070_SCU_MISC_CTRL) & ~SCU_DMA_M_S_MASK;

	//Enable DMA master
	if(node == 0)
		ast1070_scu_write(node, val | SCU_DMA_MASTER_EN, AST1070_SCU_MISC_CTRL);
	else
		ast1070_scu_write(node, val | SCU_DMA_SLAVE_EN, AST1070_SCU_MISC_CTRL);
}
EXPORT_SYMBOL(ast1070_dma_init);


extern void
ast1070_scu_init_lpc(void)
{

}

EXPORT_SYMBOL(ast1070_scu_init_lpc);

//***********************************CLK control***********************************


//***********************************CLK Information***********************************
extern u32
ast1070_get_clk_source(void)
{

}
EXPORT_SYMBOL(ast1070_get_clk_source);

//***********************************Information ***********************************
extern u32
ast1070_scu_revision_id(u8 node)
{
	u32 rev_id;
	rev_id = ast1070_scu_read(node, AST1070_SCU_CHIP_ID);
	return rev_id;
	
}	

EXPORT_SYMBOL(ast1070_scu_revision_id);

extern void
ast1070_scu_init(u32 lpc_base)
{
	ast1070_scu_base = AST1070_SCU_BASE(lpc_base);
}	
EXPORT_SYMBOL(ast1070_scu_init);
