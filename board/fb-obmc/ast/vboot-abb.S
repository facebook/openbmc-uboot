/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Verified boot alter boot blocker.
 *
 * Copyright (c) 2021 Facebook, Inc
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/arch/platform.h>

#if defined(CONFIG_ASPEED_AST2600)

/* refer to vbs.h for vbs structure and location */
.macro save_start_addr_to_vbs
	ldr	r1, =_start
	ldr	r0, =0x10015800 @;vbs.uboot_exec_address
	str	r1, [r0]
.endm

/* verified boot altert boot blocker */
.macro alter_boot_blocker
	/* check the FMCWDT2 boot source */
	ldr	r0, =ASPEED_FMC_WDT2
	ldr	r1, [r0] @; FMCWDT2 bit4
	tst	r1, #0x10
	beq	abb_out

abb_clear:
	/* schedule WDT1 reset in 100 ms */
	ldr	r0, =ASPEED_WDT_BASE
	ldr	r1, =0
	str	r1, [r0, #0x0C]   @; stop WDT1
	ldr	r1, =100000
	str	r1, [r0, #0x04]   @; stage 100ms counter
	ldr	r1, =0x4755
	str	r1, [r0, #0x08]   @; load counter
	ldr	r1, =0x13
	str	r1, [r0, #0x0C]   @; start WDT1
	/* clear boot from alter source */
	ldr	r0, =ASPEED_FMC_WDT2
	ldr	r1, =0x00EA0000
	str	r1, [r0] @; clear FMCWDT2 bit4
	/* here SPI mapping switched immediately
	 * NOTICE: if flash0 and flash1 code is different
	 * system will run in unexpected state for <100 ms and reset
	 */
wait_abb_clear_reset:
	b	wait_abb_clear_reset

abb_out:
	/* boot from primary soruce */
.endm

.section .text.vboot_abb
ENTRY(save_boot_params)
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_QEMU_BUILD)
	alter_boot_blocker
#endif
	save_start_addr_to_vbs
	b	save_boot_params_ret
ENDPROC(save_boot_params)

#endif
