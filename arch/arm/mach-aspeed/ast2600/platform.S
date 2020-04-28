// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Chia-Wei Wang <chiawei_wang@aspeedtech.com>
 */

#include <config.h>
#include <version.h>
#include <asm/secure.h>
#include <asm/armv7.h>
#include <linux/linkage.h>

/*
 *       SMP mailbox
 * +----------------------+
 * |                      |
 * | mailbox insn. for    |
 * | cpuN polling SMP go  |
 * |                      |
 * +----------------------+ 0xC
 * | mailbox ready signal |
 * +----------------------+ 0x8
 * | cpuN GO signal       |
 * +----------------------+ 0x4
 * | cpuN entrypoint      |
 * +----------------------+ AST_SMP_MAILBOX_BASE
 */

#define AST_SMP_MAILBOX_BASE		(0x1E6E2180)
#define AST_SMP_MBOX_FIELD_ENTRY	(AST_SMP_MAILBOX_BASE + 0x0)
#define AST_SMP_MBOX_FIELD_GOSIGN	(AST_SMP_MAILBOX_BASE + 0x4)
#define AST_SMP_MBOX_FIELD_READY	(AST_SMP_MAILBOX_BASE + 0x8)
#define AST_SMP_MBOX_FIELD_POLLINSN	(AST_SMP_MAILBOX_BASE + 0xc)

/* AST2600 HW registers */
#define AST_SCU_BASE			(0x1E6E2000)
#define AST_SCU_PROT_KEY1		(AST_SCU_BASE)
#define AST_SCU_PROT_KEY2		(AST_SCU_BASE + 0x010)
#define AST_SCU_REV_ID			(AST_SCU_BASE + 0x014)
#define AST_SCU_SYSRST_CTRL		(AST_SCU_BASE + 0x040)
#define AST_SCU_SYSRST_CTRL_CLR		(AST_SCU_BASE + 0x044)
#define AST_SCU_DEBUG_CTRL              (AST_SCU_BASE + 0x0C8)
#define AST_SCU_DEBUG_CTRL2             (AST_SCU_BASE + 0x0D8)
#define AST_SCU_HPLL_PARAM		(AST_SCU_BASE + 0x200)
#define AST_SCU_HPLL_PARAM_EXT		(AST_SCU_BASE + 0x204)
#define AST_SCU_MFP_CTRL7		(AST_SCU_BASE + 0x41C)
#define AST_SCU_MFP_CTRL15		(AST_SCU_BASE + 0x454)
#define AST_SCU_HW_STRAP1		(AST_SCU_BASE + 0x500)
#define AST_SCU_HW_STRAP2		(AST_SCU_BASE + 0x510)
#define AST_SCU_CA7_CTRL		(AST_SCU_BASE + 0x800)
#define AST_SCU_CA7_AXI_PREFETCH_START	(AST_SCU_BASE + 0x808)
#define AST_SCU_CA7_AXI_PREFETCH_END	(AST_SCU_BASE + 0x80C)
#define AST_SCU_CA7_PARITY_CHK		(AST_SCU_BASE + 0x820)
#define AST_SCU_CA7_PARITY_CLR		(AST_SCU_BASE + 0x824)
#define AST_SCU_MMIO_DEC_SET		(AST_SCU_BASE + 0xC24)

#define AST_FMC_BASE		(0x1E620000)
#define AST_FMC_CE0_CTRL	(AST_FMC_BASE + 0x010)
#define AST_FMC_WDT1_CTRL_MODE	(AST_FMC_BASE + 0x060)
#define AST_FMC_WDT2_CTRL_MODE	(AST_FMC_BASE + 0x064)

#define AST_GPIO_BASE		(0x1E780000)
#define AST_GPIOYZ_DATA_VALUE	(AST_GPIO_BASE + 0x1E0)
#define AST_HI_CTRL9            (AST_GPIO_BASE + 0x9098)

/* Revision ID */
#define REV_ID_AST2600A0	0x05000303

.macro scu_unlock
	movw	r0, #0xA8A8
	movt	r0, #0x1688	@; magic key to unlock SCU

	ldr	r1, =AST_SCU_PROT_KEY1
	str	r0, [r1]
	ldr	r1, =AST_SCU_PROT_KEY2
	str	r0, [r1]
.endm

.macro timer_init
#ifdef CONFIG_FPGA_ASPEED
	movw	r0, #0xF080
	movt	r0, #0x2FA
#else
	ldr	r0, =AST_SCU_REV_ID
	ldr	r0, [r0]

	ldr	r1, =REV_ID_AST2600A0
	cmp	r0, r1

	movweq 	r0, #0x32C0
	movteq	r0, #0x4013
	movwne	r0, #0x8C00
	movtne	r0, #0x4786
#endif
	mcr	p15, 0, r0, c14, c0, 0	@; update CNTFRQ
.endm


.globl lowlevel_init

lowlevel_init:
#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
	mov	pc, lr
#else
	/* setup ARM arch timer frequency */
	timer_init

	/* reset SMP mailbox as early as possible */
	mov	r0, #0x0
	ldr	r1, =AST_SMP_MBOX_FIELD_READY
	str	r0, [r1]

	/* set ACTLR.SMP to enable cache use */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1

	/*
	 * we treat cpu0 as the primary core and
	 * put secondary core (cpuN) to sleep
	 */
	mrc   p15, 0, r0, c0, c0, 5	@; Read CPU ID register
	ands  r0, #0xFF			@; Mask off, leaving the CPU ID field
	movw  r2, #0xAB00
	movt  r2, #0xABBA
	orr   r2, r0

	beq   do_primary_core_setup

	/* hold cpuN until mailbox is ready */
poll_mailbox_ready:
	wfe
	ldr	r0, =AST_SMP_MBOX_FIELD_READY
	ldr	r0, [r0]
	movw	r1, #0xCAFE
	movt	r1, #0xBABE
	cmp	r1, r0
	bne	poll_mailbox_ready

	/* parameters for relocated SMP go polling insn. */
	ldr	r0, =AST_SMP_MBOX_FIELD_GOSIGN
	ldr	r1, =AST_SMP_MBOX_FIELD_ENTRY

	/* no return */
	ldr	pc, =AST_SMP_MBOX_FIELD_POLLINSN

do_primary_core_setup:
	/* unlock system control unit */
	scu_unlock

	/* identify AST2600 A0/A1 */
	ldr	r0, =AST_SCU_REV_ID
	ldr	r0, [r0]

	ldr	r1, =REV_ID_AST2600A0
	cmp	r0, r1

	bne	0f

	/* tune up CPU clocks (A0 only) */
	ldr	r0, =AST_SCU_HW_STRAP1
	ldr	r1, [r0]
	bic	r1, #0x1800
	orr	r1, #0x1000
	str	r1, [r0]

	ldr	r0, =AST_SCU_HPLL_PARAM
	movw	r1, #0x4080
	movt	r1, #0x1000
	str	r1, [r0]

	ldr	r0, =AST_SCU_HPLL_PARAM_EXT
	mov	r1, #0x47
	str	r1, [r0]

wait_lock:
	ldr	r1, [r0]
	tst	r1, #0x80000000
	beq	wait_lock

	/* skip A1 only area */
	b 1f

0:
	/* enable AXI prefetch (A1 only) */
	ldr	r0, =AST_SCU_CA7_AXI_PREFETCH_START
	ldr r1, =ASPEED_DRAM_BASE
	str r1, [r0]

	ldr	r0, =AST_SCU_CA7_AXI_PREFETCH_END
	ldr	r1, =0xFFFFFFFF
	str r1, [r0]

	ldr	r0, =AST_SCU_CA7_CTRL
	ldr	r1, [r0]
	orr	r1, #0x8000
	str	r1, [r0]

	/* LPC/eSPI mode selection (A1 only) */
	ldr	r0, =AST_GPIOYZ_DATA_VALUE
	ldr	r0, [r0]
	tst	r0, #0x1000
	beq	1f

	/* switch to LPC mode if GPIOZ[4]=1 */
	ldr	r0, =AST_SCU_HW_STRAP2
	ldr	r1, [r0]
	orr	r1, #0x40
	str	r1, [r0]

1:
	/* release display port reset */
	ldr	r0, =AST_SCU_SYSRST_CTRL_CLR
	movw	r1, #0x0000
	movt	r1, #0x3000
	str	r1, [r0]

	/* MMIO decode setting */
	ldr	r0, =AST_SCU_MMIO_DEC_SET
	mov	r1, #0x2000
	str	r1, [r0]

	/* enable cache & SRAM parity check */
	mov	r0, #0
	ldr	r1, =AST_SCU_CA7_PARITY_CLR
	str	r0, [r1]

	/* disable SRAM parity check to prevent AST2620 A1 hang */
	mov	r0, #0x01
	ldr	r1, =AST_SCU_CA7_PARITY_CHK
	str	r0, [r1]

	/* disable FMC WDT for SPI address mode detection */
	mov	r0, #0
	ldr	r1, =AST_FMC_WDT1_CTRL_MODE
	str	r0, [r1]
#if 0
	ldr	r1, =AST_FMC_WDT2_CTRL_MODE
	str	r0, [r1]
#endif
	/* tune up SPI clock */
	movw	r0, #0x0641
	movt	r0, #0x203B
	ldr	r1, =AST_FMC_CE0_CTRL
	str	r0, [r1]

#if 0
	/* disable UART-based SoC Debug Interface UART5 and P2A bridge*/
	ldr     r0, =AST_SCU_DEBUG_CTRL
	ldr     r1, [r0]
	orr     r1, #0x03
	str     r1, [r0]
	
	/* disable UART-based SoC Debug Interface UART1 and LPC2AHB bridge */
	ldr     r0, =AST_SCU_DEBUG_CTRL2
	ldr     r1, [r0]
	orr     r1, #0x0A
	str     r1, [r0]
#endif

	/* Enable UART1: Set Multi-function Pin for UART1 */
	ldr     r0, =AST_SCU_MFP_CTRL7
	ldr     r1, [r0]
	orr     r1, #0x0F
	str     r1, [r0]

	/* Disable UART1 reset source from LPC */
	ldr   r0, =AST_HI_CTRL9
	ldr   r1, =0x20
	str   r1, [r0]

	/* Strongest LADESPID Driving Strngth */
	ldr	r0, =AST_SCU_MFP_CTRL15
	ldr	r1, [r0]
	mov r2, #0xff00
	orr r1, r2, lsl #16
	str	r1, [r0]

	/* relocate mailbox insn. for cpuN polling SMP go signal */
	adrl	r0, mailbox_insn
	adrl	r1, mailbox_insn_end

	ldr	r2, =#AST_SMP_MBOX_FIELD_POLLINSN

relocate_mailbox_insn:
	ldr	r3, [r0], #0x4
	str	r3, [r2], #0x4
	cmp	r0, r1
	bne	relocate_mailbox_insn

	/* reset SMP go sign */
	mov	r0, #0
	ldr	r1, =AST_SMP_MBOX_FIELD_GOSIGN
	str	r0, [r1]

	/* notify cpuN mailbox is ready */
	movw	r0, #0xCAFE
	movt	r0, #0xBABE
	ldr	r1, =AST_SMP_MBOX_FIELD_READY
	str	r0, [r1]
	sev

	/* back to arch calling code */
	mov	pc, lr

/*
 * insn. inside mailbox to poll SMP go signal.
 *
 * Note that as this code will be relocated, any
 * pc-relative assembly should NOT be used.
 */
mailbox_insn:
	/*
	 * r0 ~ r3 are parameters:
	 *  r0 = AST_SMP_MBOX_FIELD_GOSIGN
	 *  r1 = AST_SMP_MBOX_FIELD_ENTRY
	 *  r2 = per-cpu go sign value
	 *  r3 = no used now
	 */
poll_mailbox_smp_go:
	wfe
	ldr	r4, [r0]
	cmp	r2, r4
	bne	poll_mailbox_smp_go

	/* SMP GO signal confirmed, release cpuN */
	ldr	pc, [r1]

mailbox_insn_end:
	/* should never reach */
	b	.

#endif
