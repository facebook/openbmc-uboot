/********************************************************************************
* File Name     : arch/arm/mach-aspeed/ast-mctp.c 
* Author         : Ryan Chen
* Description   : AST MCTP Ctrl
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
*    1. 2013/03/15 Ryan Chen Create
* 
********************************************************************************/
#include <common.h>
//#include <pci.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch/regs-p2x.h>
#include <asm/arch/ast_p2x.h>

DECLARE_GLOBAL_DATA_PTR;

//#define AST_P2X_DEBUG 

#ifdef AST_P2X_DEBUG
#define MCTPDBUG(fmt, args...) printf("%s() " fmt, __FUNCTION__, ## args)
#else
#define MCTPDBUG(fmt, args...)
#endif

static u32 ast_p2x_base = AST_P2X_BASE;
static u8 txTag = 0;
static inline u32 
ast_p2x_read(u32 reg)
{
	u32 val;
		
	val = readl(ast_p2x_base + reg);
	
	MCTPDBUG("reg = 0x%08x, val = 0x%08x\n", reg, val);
	
	return val;
}

static inline void
ast_p2x_write(u32 val, u32 reg) 
{
	MCTPDBUG("reg = 0x%08x, val = 0x%08x\n", reg, val);

	writel(val, ast_p2x_base + reg);
}

//***********************************Information ***********************************

extern void ast_pcie_cfg_read(u8 type, u32 bdf_offset, u32 *value)
{
	u32 timeout =0;
	u32 desc3,desc2;
	txTag %= 0x7;
//	printf("type = %d, busfunc = %x \n",type, bdf);
	if((ast_p2x_read(AST_P2X_INT) & P2X_RX_COMPLETE) != 0)
		printf("EEEEEEEE  \n");
	
	ast_p2x_write(0x4000001 | (type << 24), AST_P2X_TX_DESC3);	
	ast_p2x_write(0x200f | (txTag << 8), AST_P2X_TX_DESC2);
	ast_p2x_write(bdf_offset, AST_P2X_TX_DESC1);
	ast_p2x_write(0, AST_P2X_TX_DESC0);
//	ast_p2x_write(0, AST_P2X_TX_DATA);

	//trigger
	ast_p2x_write(7, AST_P2X_CTRL);	
	//wait 
//	printf("trigger \n");
	while(!(ast_p2x_read(AST_P2X_INT) & P2X_RX_COMPLETE)) {
		timeout++;
		if(timeout > 10000) {
			printf("time out \n");
			*value = 0xffffffff;
			goto out;
		}
	};

	//read 
	desc3 = ast_p2x_read(AST_P2X_RX_DESC3);	
	desc2 = ast_p2x_read(AST_P2X_RX_DESC2);
	ast_p2x_read(AST_P2X_RX_DESC1);	

	if( ((desc3 >> 24) == 0x4A) && 
		((desc3 & 0xfff) == 0x1) && 
		((desc2 & 0xe000) == 0)) {
		*value = ast_p2x_read(AST_P2X_RX_DATA);

	} else {
		*value = 0xffffffff;		
		
	}

out:
	txTag++;
	ast_p2x_write(0x15, AST_P2X_CTRL);
	ast_p2x_write(0x3, AST_P2X_INT);	
	//wait 
	while(ast_p2x_read(AST_P2X_INT) & P2X_RX_COMPLETE);
	
}

extern void ast_pcie_cfg_write(u8 type, u8 byte_en, u32 bdf_offset, u32 data)
{
	txTag %= 0x7;

	//printf("byte_en : %x, offset: %x, value = %x \n",byte_en , bdf_offset, data);

	ast_p2x_write(0x44000001 | (type << 24), AST_P2X_TX_DESC3);	
	ast_p2x_write(0x2000 | (txTag << 8) | byte_en, AST_P2X_TX_DESC2);
	ast_p2x_write(bdf_offset, AST_P2X_TX_DESC1);
	ast_p2x_write(0, AST_P2X_TX_DESC0);
	ast_p2x_write(data, AST_P2X_TX_DATA);	

	//trigger
	ast_p2x_write(7, AST_P2X_CTRL);	
//	printf("trigger \n");	
	//wait 
	while(!(ast_p2x_read(AST_P2X_INT) & P2X_RX_COMPLETE));

	//read TODO Check TAG 
	ast_p2x_read(AST_P2X_RX_DESC3);	
	ast_p2x_read(AST_P2X_RX_DESC2);
	ast_p2x_read(AST_P2X_RX_DESC1);	
//	while(header && tag )
	txTag++;	
	ast_p2x_write(0x15, AST_P2X_CTRL);
	ast_p2x_write(0x3, AST_P2X_INT);	
	//wait 
	while(ast_p2x_read(AST_P2X_INT) & P2X_RX_COMPLETE);

}

extern void ast_p2x_addr_map(u32 mask, u32 addr)
{
	//Address mapping
	ast_p2x_write(addr, AST_P2X_DEC_ADDR);
	ast_p2x_write(mask, AST_P2X_DEC_MASK);
	ast_p2x_write(0x00000028, AST_P2X_DEC_TAG);
}
