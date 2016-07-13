/*******************************************************************************
 * File Name     : arch/arm/mach-aspeed/include/plat/ast-scu.h
 * Author        : Ryan Chen
 * Description   : AST SCU Service Header
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
 *      1. 2012/08/03 Ryan Chen create this file
 *
 ******************************************************************************/

#ifndef __AST_SCU_H
#define __AST_SCU_H

extern void ast_scu_show_system_info (void);
extern void ast_scu_sys_rest_info(void);
extern void ast_scu_security_info(void);
extern u32 ast_scu_revision_id(void);
extern u32 ast_scu_get_vga_memsize(void);
extern void ast_scu_get_who_init_dram(void);

extern u32 ast_get_clk_source(void);
extern u32 ast_get_h_pll_clk(void);
extern u32 ast_get_ahbclk(void);

extern u32 ast_scu_get_vga_memsize(void);

extern void ast_scu_init_eth(u8 num);
extern void ast_scu_multi_func_eth(u8 num);
extern void ast_scu_multi_func_romcs(u8 num);

#endif
