// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Chia-Wei Wang <chiawei_wang@aspeedtech.com>
 */

#include <config.h>
#include <asm/armv7.h>
#include <linux/linkage.h>

#define AST_SCU_BASE		(0x1E6E2000)
#define AST_SCU_HW_STRAP1	(AST_SCU_BASE + 0x500)
#define AST_SCU_HW_STRAP2	(AST_SCU_BASE + 0x510)


/*
 * void aspeed_bootmode(void)
 *
 * return the boot mode according to the HW strap information
 */
ENTRY(aspeed_bootmode)
	ldr	r1, =AST_SCU_HW_STRAP2
	ldr	r0, [r1]
	tst	r0, #0x100
	movne	r0, #0x2	@; AST_BOOTMODE_UART
	bne	0f

	ldr	r1, =AST_SCU_HW_STRAP1
	ldr	r0, [r1]
	tst	r0, #0x4
	moveq	r0, #0x0	@; AST_BOOTMODE_SPI
	movne	r0, #0x1	@; AST_BOOTMODE_EMMC
0:
	mov	pc, lr
ENDPROC(aspeed_bootmode)


