/*
 * AST2400 DDR Calibration tests
 *
 * (C) Copyright 2004, ASPEED Technology Inc.
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

/******************************************************************************
 Calibration Macro Start
 Usable registers:
  r0, r1, r2, r3, r5, r6, r7, r8, r9, r10, r11
 ******************************************************************************/
PATTERN_TABLE:
    .word   0xff00ff00
    .word   0xcc33cc33
    .word   0xaa55aa55
    .word   0x88778877
    .word   0x92cc4d6e       @ 5
    .word   0x543d3cde
    .word   0xf1e843c7
    .word   0x7c61d253
    .word   0x00000000       @ 8

.macro init_delay_timer
    ldr r0, =0x1e782024                          @ Set Timer3 Reload
    str r2, [r0]

    ldr r0, =0x1e6c0038                          @ Clear Timer3 ISR
    ldr r1, =0x00040000
    str r1, [r0]

    ldr r0, =0x1e782030                          @ Enable Timer3
    ldr r1, [r0]
    mov r2, #7
    orr r1, r1, r2, lsl #8
    str r1, [r0]

    ldr r0, =0x1e6c0090                          @ Check ISR for Timer3 timeout
.endm

.macro check_delay_timer
    ldr r1, [r0]
    bic r1, r1, #0xFFFBFFFF
    mov r2, r1, lsr #18
    cmp r2, #0x01
.endm

.macro clear_delay_timer
    ldr r0, =0x1e782030                          @ Disable Timer3
    ldr r1, [r0]
    bic r1, r1, #0x00000F00
    str r1, [r0]

    ldr r0, =0x1e6c0038                          @ Clear Timer3 ISR
    ldr r1, =0x00040000
    str r1, [r0]
.endm

.macro record_dll2_pass_range
    ldr                 r1, [r0]
    bic                 r2, r1, #0xFFFFFF00
    cmp                 r2, r3                   @ record min
    bicgt               r1, r1, #0x000000FF
    orrgt               r1, r1, r3
    bic                 r2, r1, #0xFFFF00FF
    cmp                 r3, r2, lsr #8           @ record max
    bicgt               r1, r1, #0x0000FF00
    orrgt               r1, r1, r3, lsl #8
    str                 r1, [r0]
.endm

.macro record_dll2_pass_range_h
    ldr                 r1, [r0]
    bic                 r2, r1, #0xFF00FFFF
    mov                 r2, r2, lsr #16
    cmp                 r2, r3                   @ record min
    bicgt               r1, r1, #0x00FF0000
    orrgt               r1, r1, r3, lsl #16
    bic                 r2, r1, #0x00FFFFFF
    cmp                 r3, r2, lsr #24          @ record max
    bicgt               r1, r1, #0xFF000000
    orrgt               r1, r1, r3, lsl #24
    str                 r1, [r0]
.endm

.macro init_spi_checksum
    ldr r0, =0x1e620084
    ldr r1, =0x20010000
    str r1, [r0]
    ldr r0, =0x1e62008C
    ldr r1, =0x20000200
    str r1, [r0]
    ldr r0, =0x1e620080
    ldr r1, =0x0000000D
    orr r2, r2, r7
    orr r1, r1, r2, lsl #8
    and r2, r6, #0xF
    orr r1, r1, r2, lsl #4
    str r1, [r0]
    ldr r0, =0x1e620008
    ldr r2, =0x00000800
.endm

#ifdef CONFIG_YOSEMITE
    /* Use GPIOE[0-3] to select debug console */
.macro console_bmc
    ldr r0, =0x1e780024
    ldr r1, [r0]
    orr r1, r1, #0xF
    str r1, [r0]

    ldr r0, =0x1e780020
    ldr r1, [r0]
    and r1, r1, #0xFFFFFFF0
    orr r1, r1, #0xC
    str r1, [r0]
.endm

.macro console_slot1
    ldr r0, =0x1e780024
    ldr r1, [r0]
    orr r1, r1, #0xF
    str r1, [r0]

    ldr r0, =0x1e780020
    ldr r1, [r0]
    and r1, r1, #0xFFFFFFF0
    orr r1, r1, #0x1
    str r1, [r0]
.endm

.macro console_slot2
    ldr r0, =0x1e780024
    ldr r1, [r0]
    orr r1, r1, #0xF
    str r1, [r0]

    ldr r0, =0x1e780020
    ldr r1, [r0]
    and r1, r1, #0xFFFFFFF0
    orr r1, r1, #0x0
    str r1, [r0]
.endm

.macro console_slot3
    ldr r0, =0x1e780024
    ldr r1, [r0]
    orr r1, r1, #0xF
    str r1, [r0]

    ldr r0, =0x1e780020
    ldr r1, [r0]
    and r1, r1, #0xFFFFFFF0
    orr r1, r1, #0x3
    str r1, [r0]
.endm

.macro console_slot4
    ldr r0, =0x1e780024
    ldr r1, [r0]
    orr r1, r1, #0xF
    str r1, [r0]

    ldr r0, =0x1e780020
    ldr r1, [r0]
    and r1, r1, #0xFFFFFFF0
    orr r1, r1, #0x2
    str r1, [r0]
.endm

.macro console_sel
    /* Disable SoL UARTs[1-4] */
    ldr r0, =0x1e6e2080
    ldr r1, [r0]
    ldr r2, =0xBFBFFFFF
    and r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e2084
    ldr r1, [r0]
    and r1, r1, r2
    str r1, [r0]

    /* Read key position */
    ldr r2, =0x1e780080
    ldr r0, [r2]
    bic r0, r0, #0xFFFFC3FF
    /* Test for position#1 */
    ldr r1, =0x0000
    cmp r0, r1
    bne case_pos2\@
    console_slot1
    b case_end\@
case_pos2\@:
    /* Test for position#2 */
    ldr r1, =0x0400
    cmp r0, r1
    bne case_pos3\@
    console_slot2
    b case_end\@
case_pos3\@:
    /* Test for position#3 */
    ldr r1, =0x0800
    cmp r0, r1
    bne case_pos4\@
    console_slot3
    b case_end\@
case_pos4\@:
    /* Test for position#4 */
    ldr r1, =0x0c00
    cmp r0, r1
    bne case_pos5\@
    console_slot4
    b case_end\@
case_pos5\@:
    /* Test for position#5 */
    ldr r1, =0x1000
    cmp r0, r1
    bne case_pos6\@
    console_bmc
    b case_end\@
case_pos6\@:
    /* Test for position#6 */
    ldr r1, =0x1400
    cmp r0, r1
    bne case_pos7\@
    console_slot1
    b case_end\@
case_pos7\@:
    /* Test for position#7 */
    ldr r1, =0x1800
    cmp r0, r1
    bne case_pos8\@
    console_slot2
  b case_end\@
case_pos8\@:
    /* Test for position#8 */
    ldr r1, =0x1c00
    cmp r0, r1
    bne case_pos9\@
    console_slot3
    b case_end\@
case_pos9\@:
    /* Test for position#9 */
    ldr r1, =0x2000
    cmp r0, r1
    bne case_pos10\@
    console_slot4
    b case_end\@
case_pos10\@:
    /* Test for position#10 */
    ldr r1, =0x2400
    cmp r0, r1
    bne case_end\@
    console_bmc
    b case_end\@
case_end\@:
.endm
#endif
