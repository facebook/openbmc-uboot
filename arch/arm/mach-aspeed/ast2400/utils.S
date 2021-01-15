// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Chia-Wei Wang <chiawei_wang@aspeedtech.com>
 */

#include <config.h>
#include <asm/armv7.h>
#include <linux/linkage.h>

#define AST_FMC_BASE		(0x1E620000)
#define AST_FMC_INT_CTRL_STAT	(AST_FMC_BASE + 0x008)
#define AST_FMC_DMA_CTRL	(AST_FMC_BASE + 0x080)
#define AST_FMC_DMA_FLASH_ADDR	(AST_FMC_BASE + 0x084)
#define AST_FMC_DMA_DRAM_ADDR	(AST_FMC_BASE + 0x088)
#define AST_FMC_DMA_LENGTH	(AST_FMC_BASE + 0x08C)

/*
 * void aspeed_spi_fastcpy(u32 mem_addr, u32 spi_addr, u32 count)
 *
 * perform FMC SPI DMA to speed up flash copy.
 * @dst: destination memory address
 * @src: source SPI address
 * @count: number of bytes to be copied, 4-byte aligned
 *
 * NOTE that the caller must ensure the validity of parameters.
 */
ENTRY(aspeed_spi_fastcpy)
	ldr	r3, =AST_FMC_DMA_DRAM_ADDR
	str	r0, [r3]

	ldr	r3, =AST_FMC_DMA_FLASH_ADDR
	str	r1, [r3]

	ldr	r3, =AST_FMC_DMA_LENGTH
	str	r2, [r3]

	ldr	r0, =AST_FMC_DMA_CTRL
	mov	r1, #1
	str	r1, [r0]

	ldr	r0, =AST_FMC_INT_CTRL_STAT
polling:
	ldr	r1, [r0]
	tst	r1, #(1 << 11)
	beq	polling

	ldr	r0, =AST_FMC_DMA_CTRL
	mov	r1, #0
	str	r1, [r0]

	mov	pc, lr
ENDPROC(aspeed_spi_fastcpy)
