/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ******************************************************************************
 * ARM11 Simulation
 *
 ******************************************************************************
 */

#include <config.h>
#include <version.h>
/******************************************************************************
 Calibration Macro Start
 Usable registers:
  r0, r1, r2, r3, r5, r6, r7, r8, r9, r10, r11
 ******************************************************************************/
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
    ldr r0, =0x1e78203C                          @ Disable Timer3
    mov r2, #0xF
    mov r1, r2, lsl #8
    str r1, [r0]

    ldr r0, =0x1e6c0038                          @ Clear Timer3 ISR
    ldr r1, =0x00040000
    str r1, [r0]
    .endm

/******************************************************************************
 Calibration Macro End
 ******************************************************************************/
.globl lowlevel_init
lowlevel_init:

init_dram:
    /* save lr */
    mov r4, lr

    /*Set Scratch register Bit 7 before initialize*/
    ldr r0, =0x1e6e2000
    ldr r1, =0x1688a8a8
    str r1, [r0]

    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

/******************************************************************************
 Disable WDT3 for SPI Address mode detection function
 ******************************************************************************/
    ldr   r0, =0x1e78504c
    mov   r1, #0
    str   r1, [r0]
/******************************************************************************
 Disable WDT2 for 2nd boot function
 ******************************************************************************/
    ldr   r0, =0x1e78502c
    mov   r1, #0
    str   r1, [r0]

    /* Check Scratch Register Bit 6 */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

set_MPLL:
    ldr r0, =0x1e6e2020                          @ M-PLL (DDR SDRAM) Frequency
    ldr r1, =0x63002400
    str r1, [r0]

    ldr r0, =0x1e780000
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e780004
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

    /* Enable Timer separate clear mode */
    ldr   r0, =0x1e782038
    mov   r1, #0xAE
    str   r1, [r0]
    clear_delay_timer

    /* Delay about 100us */
    ldr   r2, =0x0000000A                        @ Set Timer3 Reload = 10 us
    init_delay_timer
delay_0:
    check_delay_timer
    bne   delay_0
    clear_delay_timer
    /* end delay 10us */

    /* Enable AXI_P */
    ldr   r0, =0x00000016
    mrc   p15, 0, r1, c15, c2, 4
    mcr   p15, 0, r0, c15, c2, 4

/******************************************************************************
 Init DRAM common registers
 ******************************************************************************/
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0024
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e002c
    ldr r1, =0x00181D60
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0208
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0218
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0220
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0228
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0230
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e02a8
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e02b0
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0200
    ldr r1, =0x02492AAE
    str r1, [r0]

    ldr r0, =0x1e6e0204
    ldr r1, =0x00000801
    str r1, [r0]

    ldr r0, =0x1e6e020c
    ldr r1, =0x55E00A0A
    str r1, [r0]

    ldr r0, =0x1e6e0210
    ldr r1, =0x20000000
    str r1, [r0]

    ldr r0, =0x1e6e0214
    ldr r1, =0x00000024
    str r1, [r0]

    ldr r0, =0x1e6e0240
    ldr r1, =0x86000000
    str r1, [r0]

    ldr r0, =0x1e6e0244
    ldr r1, =0x00008600
    str r1, [r0]

    ldr r0, =0x1e6e0248
    ldr r1, =0x80000000
    str r1, [r0]

    ldr r0, =0x1e6e024c
    ldr r1, =0x80808080
    str r1, [r0]

    ldr r0, =0x1e6e02e0
    ldr r1, =0x01000600
    str r1, [r0]

    ldr r0, =0x1e6e02e4
    ldr r1, =0x05000091
    str r1, [r0]

    ldr r0, =0x1e6e02e8
    ldr r1, =0x000F800C
    str r1, [r0]

    ldr r0, =0x1e6e02ec
    ldr r1, =0x00258105
    str r1, [r0]

    ldr r0, =0x1e6e02f0
    ldr r1, =0x03040403
    str r1, [r0]

    ldr r0, =0x1e6e02f4
    ldr r1, =0x8D000700
    str r1, [r0]

    ldr r0, =0x1e6e02f8
    ldr r1, =0x05400A00
    str r1, [r0]

    ldr r0, =0x1e6e0290
    ldr r1, =0x00100018
    str r1, [r0]

    ldr r0, =0x1e6e02c0
    ldr r1, =0x00000006
    str r1, [r0]

    ldr r0, =0x1e6e0060
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x43403C38
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0x0568E32B
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x00000200
    str r1, [r0]

    ldr r0, =0x1e6e0008
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0xFFFFFFF0
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0x88888888
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x88888888
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x88888888
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x88888888
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000400
    str r1, [r0]

    ldr r0, =0x1e6e0074
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0078
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e007c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000081
    str r1, [r0]

    ldr r0, =0x1e6e0004
    ldr r1, =0x00001000
    ldr r2, [r0]
    orr r1, r1, r2
    str r1, [r0]

    ldr r1, =0x00080000
wait_cache_init:
    ldr r2, [r0]
    tst r1, r2
    beq wait_cache_init

    ldr r0, =0x1e780000
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0060
wait_ddrphy_init:
    ldr r1, [r0]
    tst r1, #0x01
    bne wait_ddrphy_init
    ldr r1, =0x00000006
    str r1, [r0]

    ldr r0, =0x1e780000
    ldr r1, =0x00000002
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000091
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000040
    str r1, [r0]

    /* Delay about 400us */
    ldr   r2, =0x00000002                        @ Set Timer3 Reload = 400 us
    init_delay_timer
delay_1:
    check_delay_timer
    bne   delay_1
    clear_delay_timer
    /* end delay 400us */

    ldr r0, =0x1e6e002c
    ldr r1, =0x00181D60
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000005
    str r1, [r0]
    ldr r1, =0x00000007
    str r1, [r0]
    ldr r1, =0x00000003
    str r1, [r0]
    ldr r1, =0x00000011
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005A01
    str r1, [r0]

    /* Delay about 1us */
    ldr   r2, =0x00000001                        @ Set Timer3 Reload = 1 us
    init_delay_timer
delay_2:
    check_delay_timer
    bne   delay_2
    clear_delay_timer
    /* end delay 1us */

    ldr r0, =0x1e6e000c
    ldr r1, =0x00002048
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x70000000
wait_ddrdll_reset:
    ldr r2, [r0]
    tst r1, r2
    bne wait_ddrdll_reset

    ldr r0, =0x1e6e002c
    ldr r1, =0x00181C60
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005C01
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x0001AF93
    str r1, [r0]

    ldr r0, =0x1e780000
    ldr r1, =0x00000004
    str r1, [r0]

platform_exit:

    /* restore lr */
    mov   lr, r4

    /* back to arch calling code */
    mov   pc, lr

