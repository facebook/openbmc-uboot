/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 */

#ifndef _ASPEED_PLATFORM_H_
#define _ASPEED_PLATFORM_H_

#define AST_PLL_25MHZ			25000000
#define AST_PLL_24MHZ			24000000
#define AST_PLL_12MHZ			12000000

/*********************************************************************************/
#if defined(CONFIG_ASPEED_AST2400)
#define ASPEED_MAC_COUNT	2
#define ASPEED_HW_STRAP1	0x1e6e2070
#define ASPEED_REVISION_ID	0x1e6e207C
#define ASPEED_SYS_RESET_CTRL	0x1e6e203C
#define ASPEED_VGA_HANDSHAKE0	0x1e6e2040	/*	VGA fuction handshake register */
#define ASPEED_DRAM_BASE	0x40000000
#define ASPEED_SRAM_BASE	0x1E720000
#define ASPEED_SRAM_SIZE	0x9000
#define ASPEED_FMC_CS0_BASE	0x20000000
#elif defined(CONFIG_ASPEED_AST2500)
#define ASPEED_MAC_COUNT	2
#define ASPEED_HW_STRAP1	0x1e6e2070
#define ASPEED_HW_STRAP2	0x1e6e20D0
#define ASPEED_REVISION_ID	0x1e6e207C
#define ASPEED_SYS_RESET_CTRL	0x1e6e203C
#define ASPEED_VGA_HANDSHAKE0	0x1e6e2040	/*	VGA fuction handshake register */
#define ASPEED_MAC_COUNT	2
#define ASPEED_DRAM_BASE	0x80000000
#define ASPEED_SRAM_BASE	0x1E720000
#define ASPEED_SRAM_SIZE	0x9000
#define ASPEED_FMC_CS0_BASE	0x20000000
#elif defined(CONFIG_ASPEED_AST2600)
#define ASPEED_HW_STRAP1	0x1e6e2500
#define ASPEED_HW_STRAP2	0x1e6e2510
#define ASPEED_REVISION_ID	0x1e6e2004
#define ASPEED_SYS_RESET_CTRL	0x1e6e2064
#define ASPEED_SYS_RESET_CTRL3	0x1e6e206c
#define ASPEED_VGA_HANDSHAKE0	0x1e6e2100	/*	VGA fuction handshake register */
#define ASPEED_MAC_COUNT	4
#define ASPEED_DRAM_BASE	0x80000000
#define ASPEED_SRAM_BASE	0x10000000
#define ASPEED_SRAM_SIZE	0x10000
#define ASPEED_FMC_CS0_BASE	0x20000000
#else
#err "No define for platform.h"
#endif
 
#endif
