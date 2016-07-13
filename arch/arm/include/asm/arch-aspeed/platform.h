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

#ifndef _AST_PLATFORM_H
#define _AST_PLATFORM_H

#include <asm/arch/aspeed.h>

#define AST_PLL_25MHZ			25000000
#define AST_PLL_24MHZ			24000000
#define AST_PLL_12MHZ			12000000

#if defined(CONFIG_ARCH_AST2400)
#include <asm/arch/ast2400_platform.h>
#elif defined(AST_SOC_G5)
#include <asm/arch/ast_g5_platform.h>
#else
#err "No define for platform.h"
#endif

#endif
