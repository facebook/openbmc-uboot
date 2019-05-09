/*******************************************************************************
 * File Name     : arch/arm/mach-aspeed/ast-ahbc.c
 * Author         : Ryan Chen
 * Description   : AST AHB Ctrl
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 *   History      :
 *    1. 2014/03/15 Ryan Chen Create
 *
 ******************************************************************************/
#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch/regs-ahbc.h>
#include <asm/arch/ast-ahbc.h>
#include <asm/arch/aspeed.h>

static inline u32 ast_ahbc_read(u32 reg)
{
	u32 val = readl(AST_AHBC_BASE + reg);

	debug("reg = 0x%08x, val = 0x%08x\n", reg, val);
	return val;
}

static inline void ast_ahbc_write(u32 val, u32 reg)
{
	debug("reg = 0x%08x, val = 0x%08x\n", reg, val);

#ifdef CONFIG_AST_AHBC_LOCK
	//unlock
	writel(AHBC_PROTECT_UNLOCK, AST_AHBC_BASE);
	writel(val, AST_AHBC_BASE + reg);
	//lock
	writel(0xaa,AST_AHBC_BASE);
#else
	writel(AHBC_PROTECT_UNLOCK, AST_AHBC_BASE);
	writel(val, AST_AHBC_BASE + reg);
#endif

}

void ast_ahbc_boot_remap(void)
{
#if ! defined(AST_SOC_G5)
	ast_ahbc_write(ast_ahbc_read(AST_AHBC_ADDR_REMAP) |
		       AHBC_BOOT_REMAP, AST_AHBC_ADDR_REMAP);
#endif
}

#ifdef AST_SOC_G5
void ast_ahbc_peie_mapping(u8 enable)
{
	if (enable)
		ast_ahbc_write(ast_ahbc_read(AST_AHBC_ADDR_REMAP) |
			       AHBC_PCIE_MAP, AST_AHBC_ADDR_REMAP);
	else
		ast_ahbc_write(ast_ahbc_read(AST_AHBC_ADDR_REMAP) &
			       ~AHBC_PCIE_MAP, AST_AHBC_ADDR_REMAP);
}

void ast_ahbc_lpc_plus_mapping(u8 enable)
{
	if(enable)
		ast_ahbc_write(ast_ahbc_read(AST_AHBC_ADDR_REMAP) |
			       AHBC_LPC_PLUS_MAP, AST_AHBC_ADDR_REMAP);
	else
		ast_ahbc_write(ast_ahbc_read(AST_AHBC_ADDR_REMAP) &
			       ~AHBC_LPC_PLUS_MAP, AST_AHBC_ADDR_REMAP);
}
#endif
