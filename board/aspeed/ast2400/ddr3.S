/*
 * AST2400 DDR Calibration tests
 *
 * (C) Copyright 2004, ASPEED Technology Inc.
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include "macros.h"

.globl ddr3_init

/******************************************************************************
 DDR3 Init

 tRCD   = 15     ns
 tRAS   = 37.5   ns
 tRRD   = max(4 CK,10 ns)
 tRP    = 15     ns
 tRFC   = 110ns/1Gbit, 160ns/2Gbit, 300ns/4Gbit
 tRTP   = max(4 CK,7.5 ns)
 tWR    = 15     ns
 tXSNR  = max(10 CK,200 ns)
 tWTR   = max(4 CK,7.5 ns)
 tFAW   = 50     ns
 tMRD   = max(15 CK,20 ns)
 ******************************************************************************/
ddr3_init:
#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x33                                @ '3'
    str r1, [r0]
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    /* End Debug - UART console message */
#endif

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000531                          @ Default set to 1Gbit
    str r1, [r0]

    ldr r0, =0x1e6e0010                          @ REG_AC1
    ldr r1, =0x33302825                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x22202725
#endif
    str r1, [r0]

    /* Check DRAM CL Timing by H/W Trapping */
    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xF9FFFFFF
    mov r2, r1, lsr #9                           @ Set CL
    ldr r1, =0x00020000
    add r2, r2, r1
    ldr r1, [r0]
    bic r1, r1, #0xFBFFFFFF
    mov r1, r1, lsr #6                           @ Set CWL
    orr r2, r2, r1
    ldr r1, =0x00300000
    add r2, r2, r1

    ldr r0, =0x1e6e0014                          @ REG_AC2
    ldr r1, =0xCC00963F                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0xAA007636
#endif
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e0004                          @ check 2400 mode
    ldr r2, [r0]
    mov r2, r2, lsr #10

    ldr r0, =0x1e6e006c                          @ REG_IOZ
    ldr r1, =0x00002312                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x00002312
#endif
#ifdef CONFIG_TARGET_FBYOSEMITE
    ldr r1, =0x00002310                          @ Crossing Voltage Criterion
#endif
    tst r2, #0x01
    moveq r1, r1, lsr #8
    str r1, [r0]

    ldr r0, =0x1e6e0120
    mov r1, #0
    str r1, [r0]
    tst r2, #0x01                                @ check AST2300
    beq CBRDLL1_2300_Start
    ldr r0, =0x1e6e207c                          @ check AST2400 revision A0
    ldr r1, [r0]
    mov r1, r1, lsr #16
    and r1, r1, #0xFF
    cmp r1, #0x0
    beq CBRDLL1_2300_Start
    b   CBRDLL1_2400_Start

MCLK2X_Phase_CBR_Done_DDR3:
    ldr r0, =0x1e6e0018
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000040
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000190                          @ Set Timer3 Reload = 400 us
    init_delay_timer
delay3_4:
    check_delay_timer
    bne delay3_4
    clear_delay_timer
    /* End delay 400us */

    /* Check DRAM CL Timing by H/W Trapping */
    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xF9FFFFFF
    mov r2, r1, lsr #21                          @ Set CL
    ldr r1, =0x00000010
    add r2, r2, r1
    ldr r1, [r0]
    bic r1, r1, #0xFBFFFFFF
    mov r1, r1, lsr #7                           @ Set CWL
    orr r2, r2, r1

    ldr r0, =0x1e6e002c                          @ REG_MRS
    ldr r1, =0x04001700                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x04001500
#endif
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e0030                          @ REG_EMRS
    ldr r1, =0x00000000                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x00000000
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS2
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS3
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set MRS
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e002c                          @ REG_MRS
    ldr r1, =0x04001600                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x04001400
#endif
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e000c                          @ Refresh 8 times
    ldr r1, =0x00005C48
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set MRS
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c                          @ Set refresh cycle
    ldr r1, =0x00002001
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, [r0]
    bic r1, r1, #0xFFF9FFFF
    mov r2, r1, lsr #3                           @ get CL

    ldr r0, =0x1e6e0034                          @ REG_PWC
    ldr r1, =0x00000303                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x00000303
#endif
    orr r1, r1, r2
    str r1, [r0]

    b   Calibration_Start
.LTORG
/******************************************************************************
 End DDR3 Init
 ******************************************************************************/

/******************************************************************************
 DDR CK duty finetune program
    SRAM buffer definition
    0x1E720204 : gdll     golden DLL1 record
    0x1E720208 : gduty    golden duty setting record
    0x1E72020C : gdutysum golden duty data record
    0x1E720210 : duty record of delay 0  invert
    0x1E720214 : duty record of delay 1  invert
    ....
    0x1E72024C : duty record of delay 15 invert
    0x1E720250 : duty record of delay 0
    0x1E720254 : duty record of delay 1
    ....
    0x1E72028C : duty record of delay 15

    Register usage
    r0 - r3 = free
    r4  = record the return pc value, do not use
    r5  = free
    r6  = free
    r7  = duty count
    r8  = gdll
    r9  = gduty
    r10 = gdutysum
 ******************************************************************************/
CBRDLL1_2400_Start:
    ldr   r0, =0x1e6e0120
    ldr   r1, [r0]
    orr   r1, r1, #0x02
    str   r1, [r0]

    ldr   r1, =0x00000000
    ldr   r0, =0x1e720204
    ldr   r2, =0x1e7202a0

init_sram_start0:
    str   r1, [r0]
    add   r0, r0, #4
    cmp   r0, r2
    blt   init_sram_start0

    ldr   r0, =0x1e6e0034
    mov   r1, #0x20
    str   r1, [r0]

    ldr   r0, =0x1e6e0060
    ldr   r1, [r0]
    mov   r2, #0x01
    orr   r1, r1, r2, lsl #13
    str   r1, [r0]

    mov   r7, #0x0                               @ init duty count
    mov   r8, #0x0                               @ init gdll
    mov   r9, #0x0                               @ init gduty
    mov   r10, #0x0                              @ init gdutysum

cbrdll1_duty_start:
    cmp   r7, #32
    bge   cbrdll1_duty_end

    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00008120
    str   r1, [r0]

    ldr   r0, =0x1e6e0060
    ldr   r1, [r0]
    bic   r1, r1, #0x00001F00
    orr   r1, r1, r7, lsl #8
    mov   r2, #0x10
    eor   r1, r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e0000                        @ dummy read
    ldr   r1, [r0]

    b     CBRDLL1_2300_Start

CBRDLL1_2400_Call:
    mov   r5, #0x01                              @ init dqidly count
    mov   r6, #0x00                              @ init duty sum
cbrdll1_duty_cal_start:
    cmp   r5, #0x05
    bge   cbrdll1_duty_cal_end

    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00200120
    orr   r1, r1, r5, lsl #16
    str   r1, [r0]

    ldr   r0, =0x1e6e0000
    ldr   r1, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    mov   r2, #0x10
    orr   r1, r1, r2, lsl #24
    str   r1, [r0]

    ldr   r0, =0x1e6e0080
    ldr   r1, =0x80000000                        @ init duty cal waiting

cbrdll1_duty_cal_wait:
    ldr   r2, [r0]
    tst   r2, r1
    beq   cbrdll1_duty_cal_wait

    ldr   r0, =0x1e6e008c
    ldr   r2, [r0]

    ldr   r0, =0x1e720210
    add   r0, r0, r7, lsl #2
    str   r2, [r0]

    ldr   r1, =0xFFFF
    and   r3, r1, r2
    cmp   r3, r1
    moveq r2, r2, lsr #16
    and   r3, r1, r2
    add   r6, r6, r3
    ldr   r1, =0xF000
    cmp   r3, r1
    blt   cbrdll1_duty_cal_end
    add   r5, r5, #0x01
    b     cbrdll1_duty_cal_start

cbrdll1_duty_cal_end:
    mov   r6, r6, lsr #2                         @ get dutysum
    cmp   r6, r10                                @ check dutysum > gdutysum
    ble   cbrdll1_duty_next
    ldr   r0, =0x1e6e0068
    ldr   r8, [r0]
    eor   r9, r7, #0x10
    mov   r10, r6

cbrdll1_duty_next:
    add   r7, r7, #0x01
    cmp   r7, #16                                @ check duty >= 15
    blt   cbrdll1_duty_start
    ldr   r0, =0xFA00                            @ check gdutysum > 0xFA00
    cmp   r10, r0
    blt   cbrdll1_duty_start

cbrdll1_duty_end:
    ldr   r0, =0x1e6e0060
    ldr   r1, [r0]
    bic   r1, r1, #0x00001F00
    orr   r1, r1, r9, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e0068
    bic   r8, r8, #0xFF000000
    bic   r8, r8, #0x00FF0000
    str   r8, [r0]

    ldr   r0, =0x1e720204                        @ record result
    str   r8, [r0]
    add   r0, r0, #0x04
    str   r9, [r0]
    add   r0, r0, #0x04
    str   r10, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00008120
    str   r1, [r0]
    ldr   r0, =0x1e6e0000                        @ dummy read
    ldr   r1, [r0]
    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00000120
    str   r1, [r0]

    ldr   r0, =0x1e6e0120
    ldr   r1, [r0]
    cmp   r1, #0x3
    beq   platform_unsupported_exit              @ DDR2 not supported
    b     MCLK2X_Phase_CBR_Done_DDR3

/******************************************************************************
 MCLK2X lock to MCLK program
  r0 - r3 = free
  r5 = madjmax
  r6 = dllend
  0x1E720200 = 0x96cnt:failcnt:dllmax:dllmin
 ******************************************************************************/
CBRDLL1_2300_Start:
    ldr   r0, =0x1e6e0064
    ldr   r5, [r0]
    and   r5, r5, #0xFF                          @ init madjmax
    mov   r6, r5                                 @ init dllend

    ldr   r1, =0x000000ff
    ldr   r0, =0x1e720200
    str   r1, [r0]                               @ init dllcnt2:dllmax:dllmin

    mov   r3, #0x0                               @ init loop count

cbrdll1_scan_start:
    cmp   r3, r6
    bge   cbrdll1_scan_end

    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00008120
    str   r1, [r0]

    ldr   r0, =0x1e6e0068
    mov   r1, r3
    cmp   r1, r5
    subge r1, r1, r5
    str   r1, [r0]

    ldr   r0, =0x1e6e0000                        @ dummy read
    ldr   r1, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00000120
    str   r1, [r0]

    ldr   r0, =0x1e6e0000                        @ dummy read
    ldr   r1, [r0]
    ldr   r0, =0x1e6e0000                        @ dummy read
    ldr   r1, [r0]

    ldr   r0, =0x1e6e001c
    ldr   r1, [r0]
    mov   r1, r1, lsr #16
    and   r1, r1, #0xFF

    and   r2, r1, #0x96
    cmp   r2, #0x96
    beq   cbrdll1_scan_pass                      @ if (mclk2x_phase & 0x96) == 0x96
    ldr   r0, =0x1e720200
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    ands  r2, r2, #0xFF                          @ get dllmax
    beq   cbrdll1_scan_next                      @ if dllmax == 0
    mov   r2, r1, lsr #16
    and   r2, r2, #0xFF
    add   r2, r2, #0x01
    cmp   r2, #0x02
    movge r6, r3
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r2, lsl #16
    str   r1, [r0]
    b     cbrdll1_scan_next

cbrdll1_scan_pass:
    cmp   r3, #0x0                               @ if dll = 0
    moveq r3, #0x0F
    addeq r6, r6, #0x10
    beq   cbrdll1_scan_next
    ldr   r0, =0x1e720200
    ldr   r2, [r0]
    cmp   r1, #0x96
    bne   cbrdll1_scan_pass2
    mov   r1, r2, lsr #24
    add   r1, r1, #0x01
    bic   r2, r2, #0xFF000000
    orr   r2, r2, r1, lsl #24
    cmp   r1, #0x03                              @ check (phase == 0x96) count == 3
    bicge r2, r2, #0x0000FF00
    bicge r2, r2, #0x000000FF
    orrge r2, r2, r3, lsl #8
    orrge r2, r2, r3
    str   r2, [r0]
    bge   cbrdll1_scan_end

cbrdll1_scan_pass2:
    and   r1, r2, #0xFF                          @ if(dllmin > dll)
    cmp   r1, r3
    bicgt r2, r2, #0x000000FF
    orrgt r2, r2, r3

    mov   r1, r2, lsr #8                         @ if(dllmax < dll)
    and   r1, r1, #0xFF
    cmp   r1, r3
    biclt r2, r2, #0x0000FF00
    orrlt r2, r2, r3, lsl #8

    bic   r2, r2, #0x00FF0000
    str   r2, [r0]

cbrdll1_scan_next:
    add   r3, r3, #0x01
    b     cbrdll1_scan_start

cbrdll1_scan_end:
    ldr   r0, =0x1e720200
    ldr   r1, [r0]
    mov   r2, r1, lsr #8                         @ get dllmax
    ands  r2, r2, #0xFF
    bne   cbrdll1_scan_done                      @ if(dllmax != 0)
    ldr   r0, =0x1e6e0064
    ldr   r3, [r0]
    bic   r1, r3, #0x000C0000
    str   r1, [r0]
    add   r0, r0, #0x04
    mov   r1, #0x0
    str   r1, [r0]

    /* Delay about 10us */
    ldr r2, =0x0000000A                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay0_1:
    check_delay_timer
    bne delay0_1
    clear_delay_timer
    /* End delay 10us */

    ldr   r0, =0x1e6e0064
    str   r3, [r0]

    /* Delay about 10us */
    ldr r2, =0x0000000A                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay0_2:
    check_delay_timer
    bne delay0_2
    clear_delay_timer
    /* End delay 10us */

    b     CBRDLL1_2300_Start

cbrdll1_scan_done:
    and   r1, r1, #0xFF
    add   r1, r1, r2
    mov   r6, r1, lsr #1                         @ dll1.0 = (dllmin + dllmax) >> 1
    cmp   r6, r5
    subge r6, r6, r5
    add   r3, r6, r5, lsr #2                     @ dll1.1 = dll1.0 + (MADJ >> 2)

    ldr   r0, =0x1e6e0004
    ldr   r1, [r0]
    mov   r1, r1, lsr #10
    tst   r1, #0x1
    bne   cbrdll1_scan_set_2400
    cmp   r3, r5
    subge r3, r3, r5
    mov   r2, #0x0
    tst   r3, #0x08
    beq   cbrdll1_scan_set_2300_2                @ if !(dll & 8)

cbrdll1_scan_set_2300_1:                         @ if  (dll & 8)
    mov   r1, #0x0
    tst   r3, #0x08
    addeq r1, r1, #0x01
    cmp   r2, #0x05
    addge r1, r1, #0x01
    cmp   r1, #0x02
    beq   cbrdll1_scan_set
    add   r2, r2, #0x01
    add   r3, r3, #0x01
    cmp   r3, r5
    subge r3, r3, r5
    b     cbrdll1_scan_set_2300_1

cbrdll1_scan_set_2300_2:
    and   r1, r3, #0x07
    cmp   r1, #0x07
    beq   cbrdll1_scan_set
    cmp   r2, #0x05
    bge   cbrdll1_scan_set
    add   r2, r2, #0x01
    add   r3, r3, #0x01
    cmp   r3, r5
    subge r3, r3, r5
    b     cbrdll1_scan_set_2300_2

cbrdll1_scan_set_2400:
    add   r3, r3, #0x05                          @ dll1.1 = dll1.0 + (MADJ >> 2) + 5
    cmp   r3, r5
    subge r3, r3, r5

cbrdll1_scan_set:
    orr   r1, r6, r3, lsl #8
    ldr   r0, =0x1e6e0068
    str   r1, [r0]

    ldr   r0, =0x1e6e0120
    ldr   r1, [r0]
    cmp   r1, #0x0
    beq   MCLK2X_Phase_CBR_Done_DDR3
    cmp   r1, #0x1
    beq   platform_unsupported_exit              @ DDR2 not supported
    b     CBRDLL1_2400_Call
.LTORG

/******************************************************************************
 Calibration Code Start
    SRAM buffer definition
    0x1E720000 : Pass 1, DLLI MIN value range
    0x1E720008 : DQS0 DLL valid range, 2nd time CBR
    0x1E72000C : DQS1 DLL valid range, 2nd time CBR
    0x1E720010 : DQ0  DLL valid range, Pass 1
    0x1E720014 : DQ1  DLL valid range, Pass 1
    ....
    0x1E720048 : DQ14 DLL valid range, Pass 1
    0x1E72004C : DQ15 DLL valid range, Pass 1
    0x1E720090 : DLL1 SAdj record
    0x1E720094 : DQL  Pass1 finetune result
    0x1E720098 : DQH  Pass1 finetune result
    0x1E72009C : DRAM initial time, (us)
    0x1E7200A0 : CBR3 retry counter
    0x1E7200A4 : DRAM initial time, (us)
    0x1E7200A8 : Released date
    0x1E7200AC : Released SDK version
    0x1E7200B0 : DQS input mask window for MCR18[4] = 0
    0x1E7200B4 : DQS input mask window for MCR18[4] = 1
    0x1E720100 : DQIDLY=00, DLL valid range
    0x1E720104 : DQIDLY=01, DLL valid range
    ....
    0x1E720178 : DQIDLY=30, DLL valid range
    0x1E72017C : DQIDLY=31, DLL valid range
    0x1E720180 : DQSI-MCLK2X P-phase pass record DLL2= 0-31
    0x1E720184 : DQSI-MCLK2X P-phase pass record DLL2=32-63
    0x1E720188 : DQSI-MCLK2X N-phase pass record DLL2= 0-31
    0x1E72018C : DQSI-MCLK2X N-phase pass record DLL2=32-63
 ******************************************************************************/
Calibration_Start_pre:                           @ Toggle DQSI mask delay
    ldr r0, =0x1e6e0018
    ldr r1, [r0]
    eor r1, r1, #0x10
    str r1, [r0]

Calibration_Start:
    /* Init SRAM buffer */
    ldr r1, =0x000000ff
    ldr r0, =0x1e720000
    ldr r2, =0x1e720100
init_sram_start:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start

    ldr r1, =0x00ff00ff
    ldr r0, =0x1e720100
    ldr r2, =0x1e720180
init_sram_start2:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start2

    ldr r1, =0x00000000
    ldr r0, =0x1e720180
    ldr r2, =0x1e720200
init_sram_start3:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start3

    ldr r0, =0x1e6e0068                          @ save the DLL1 SAdj initial value
    ldr r1, [r0]
    ldr r0, =0x1e720090
    str r1, [r0]

/******************************************************************************
 Start
  r0 = free
  r1 = free
  r2 = free
  r3 = free
  r4 = record the return pc value, do not use
  r5 = pattern table index
  r6 = pass count
  r7 = dram DLL2 parameter index (0x1e6e0068), max is 0x4C

 Fine DQI delay and DQSI-MCLK phase
  r8  = DQIDLY count
  r9  = DQSI-MCLK2X phase count
  r10 = pattern fail retry counter, initialize to 2 (fail 2 times)
  r11 = passcnt accumulator for each DQIDLY
 *****************************************************************************/
CBR0_START:
#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x43                                @ 'C'
    str r1, [r0]
    mov r1, #0x42                                @ 'B'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x30                                @ '0'
    str r1, [r0]
    mov r1, #0x2D                                @ '-'
    str r1, [r0]
    /* End Debug - UART console message */
#endif

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0xFF000000
    bic   r1, r1, #0x00FF0000
    str   r1, [r0]

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 1KB
    ldr   r1, =0x000003FF
    str   r1, [r0]

    mov   r8, #0x00                              @ init DQIDLY
    mov   r9, #0x00                              @ init DQSI-MCLK2X phase
    mov   r11, #0x01                             @ init passcnt accumulator

cbr0_next_dqidly:
    cmp   r9, #0x00
    bne   cbr0_next_dqsiphase
    cmp   r11, #0x00
    addeq r8, r8, #0x01                          @ jump 1 stage if no pass at previous stage
    mov   r11, #0x00
    add   r8, r8, #0x01
    cmp   r8, #0x1F                              @ max DQIDLY = 31
    bgt   CBR0_END

#ifdef DEBUG
    /* Debug - UART console message */
    ldr   r0, =CONFIG_ASPEED_COM
    and   r1, r8, #0x07
    add   r1, r1, #0x30                          @ '0-7'
    str   r1, [r0]
    /* Debug - UART console message */
#endif

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r8, lsl #16
    str   r1, [r0]
    mov   r9, #0x01                              @ '1':p_phase, '0':n_phase

    /* Delay about 3us */                        @ wait DQIDLY load
    ldr r2, =0x00000003                          @ Set Timer4 Reload = 3 us
    init_delay_timer
delay_4:
    check_delay_timer
    bne delay_4
    clear_delay_timer
    /* End delay 3us */

    b     cbr0_dll2_scan_start

cbr0_next_dqsiphase:
    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    orr   r1, r1, r9, lsl #23                    @ set DQSI-MCLK2X phase
    str   r1, [r0]
    mov   r9, #0x00

cbr0_dll2_scan_start:
    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL2 parameter index

/****************************
 DLL2 delay margin test loop
 ***************************/
cbr0_next_dll2_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL2 parameter
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    bic   r1, r1, #0xFF000000
    orr   r1, r1, r7, lsl #16
    str   r1, [r0]
    ldr   r2, =0x40404040                        @ DLL2 max is 0x40404040
    cmp   r7, r2
    bge   cbr0_next_dqidly
    ldr   r2, =0x01010101
    add   r7, r7, r2

/* CBRScan3() start */
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr0_next_test_pattern:
    mov   r10, #2                                @ set the retry loop = 2 of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr0_test_burst

    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add 1 after loop check so we need to decrease 1
    cmp   r3, #0x00
    beq   cbr0_next_dqidly                       @ pass at dlli = 0, invalid
    add   r6, r6, #0x01                          @ increment pass count
    add   r11, r11, #0x01                        @ increment pass count

    ldr   r0, =0x1e720180                        @ record DLL2 pass window
    cmp   r9, #0x00                              @ DQSI-MCLK2X phase check
    addeq r0, r0, #0x08
    cmp   r3, #32
    addge r0, r0, #0x4
    and   r1, r3, #0x1F
    mov   r2, #0x1
    mov   r2, r2, lsl r1
    ldr   r1, [r0]
    orr   r1, r1, r2
    str   r1, [r0]

    ldr   r0, =0x1e720100                        @ record DLL2 min:max value for each DQIDLY
    add   r0, r0, r8, lsl #2
    cmp   r9, #0x00                              @ DQSI-MCLK2X phase check
    beq   cbr0_test_pass_dqsin
    record_dll2_pass_range
    b     cbr0_next_dll2_parameter

cbr0_test_pass_dqsin:
    record_dll2_pass_range_h
    b     cbr0_next_dll2_parameter

cbr0_test_pattern_fail:
    cmp   r6, #5                                 @ passcnt >= 5
    bge   cbr0_next_dqidly
    ldr   r0, =0x1e720100                        @ reset DLL2 min:max value
    add   r0, r0, r8, lsl #2
    ldr   r1, [r0]
    ldr   r2, =0xFFFF0000
    ldr   r3, =0x000000FF
    cmp   r9, #0x00
    moveq r2, r2, lsr #16
    moveq r3, r3, lsl #16
    and   r1, r1, r2
    orr   r1, r1, r3
    str   r1, [r0]
    b     cbr0_next_dll2_parameter               @ CBRScan3() end and test result fail, go to next step

/****************************
 Test fail retry loop
 ***************************/
cbr0_pattern_fail_retry:

    /* CBRTest3() start */
cbr0_test_burst:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x000000C1
    str   r1, [r0]
    ldr   r3, =0x3000
cbr0_wait_engine_idle_0:
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr0_wait_engine_idle_0

    ldr   r2, [r0]                               @ read fail bit status
    mov   r1, #0x0
    str   r1, [r0]
    mov   r2, r2, lsr #13                        @ D[13] = fail bit
    cmp   r2, #0x00
    bne   cbr0_test_fail

cbr0_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000085
    str   r1, [r0]
    ldr   r3, =0x3000
cbr0_wait_engine_idle_1:
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr0_wait_engine_idle_1

    ldr   r2, [r0]                               @ read fail bit status
    mov   r1, #0x0
    str   r1, [r0]
    mov   r2, r2, lsr #13                        @ D[13] = fail bit
    cmp   r2, #0x00
    beq   cbr0_test_pass
    /* CBRTest3() end */

cbr0_test_fail:
    subs  r10, r10, #1
    bne   cbr0_pattern_fail_retry
    b     cbr0_test_pattern_fail                 @ CBRScan3() return(0)

cbr0_test_pass:
    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr0_next_test_pattern

CBR0_END:
    mov   r5, #0x0                               @ init DQIDLY search count
    mov   r6, #0x0                               @ init max_margin:g_margin
    mov   r8, #0x0                               @ init g_side
    mov   r7, #0x0                               @ init maximum margin DQIDLY,DQSI-MCLK2X phase
cbr0_search_dll_margin_s:
    ldr   r0, =0x1e720100
    add   r0, r0, r5, lsl #2
    ldr   r1, [r0]
    and   r2, r1, #0xFF                          @ get dllmin_p
    mov   r1, r1, lsr #8
    and   r3, r1, #0xFF                          @ get dllmax_p
    subs  r2, r3, r2                             @ get margin-P
    movmi r2, #0x0
    mov   r1, r1, lsr #8
    and   r3, r1, #0xFF                          @ get dllmin_n
    mov   r1, r1, lsr #8
    and   r1, r1, #0xFF                          @ get dllmax_n
    subs  r3, r1, r3                             @ get margin-N
    movmi r3, #0x0
    add   r1, r2, r3
    cmp   r1, #0x0
    beq   cbr0_search_dll_margin_e               @ if margin-P = 0 && margin-N = 0

    ldr   r9, [r0]
    ldr   r0, =0x1e720180
    cmp   r2, r3
    orrlt r5, r5, #0x80                          @ margin-N > margin-P
    addlt r0, r0, #0x08
    movlt r9, r9, lsr #16
    movge r3, r2                                 @ max(margin-P/N)
    add   r2, r3, #0x2                           @ define +/- 2 steps of variation
    mov   r1, r6, lsr #16
    cmp   r2, r1
    blt   cbr0_search_dll_margin_e               @ if max(margin-P/N) + 2 < max_margin

    and   r1, r9, #0xFF                          @ r1 = dlli counter
    cmp   r1, #32
    ldrge r2, [r0, #0x4]                         @ load pass window
    ldrlt r2, [r0]
    and   r1, r1, #0x1F
    mov   r10, #0x1                              @ init test bit mask
    mov   r10, r10, lsl r1
    and   r1, r9, #0xFF

cbr0_search_dllmin_margin_s:
    tst   r2, r10
    beq   cbr0_search_dllmin_margin_e
    mov   r10, r10, lsr #1
    cmp   r1, #32
    ldreq r2, [r0]
    ldreq r10, =0x80000000
    subs  r1, r1, #0x1
    bne   cbr0_search_dllmin_margin_s

cbr0_search_dllmin_margin_e:
    and   r2, r9, #0xFF
    sub   r11, r2, r1                            @ get dllmin side margin

    mov   r9, r9, lsr #8
    and   r1, r9, #0xFF                          @ r1 = dlli counter
    cmp   r1, #32
    ldrge r2, [r0, #0x4]                         @ load pass window
    ldrlt r2, [r0]
    and   r1, r1, #0x1F
    mov   r10, #0x1                              @ init test bit mask
    mov   r10, r10, lsl r1
    and   r1, r9, #0xFF

cbr0_search_dllmax_margin_s:
    tst   r2, r10
    beq   cbr0_search_dllmax_margin_e
    mov   r10, r10, lsl #1
    cmp   r1, #31
    ldreq r2, [r0, #0x4]
    ldreq r10, =0x00000001
    add   r1, r1, #0x1
    cmp   r1, #64
    bne   cbr0_search_dllmax_margin_s

cbr0_search_dllmax_margin_e:
    and   r2, r9, #0xFF
    sub   r1, r1, r2                             @ get dllmax side margin
    cmp   r1, r11
    movlt r11, r1                                @ get side_margin

cbr0_check_dll_margin:                           @ if max(margin-P/N) > g_margin && side_margin >= g_side && dqidly <= 20
    cmp   r5, #20
    bgt   cbr0_check_dll_margin2
    and   r1, r6, #0xFF
    cmp   r3, r1
    ble   cbr0_check_dll_margin3
    cmp   r11, r8
    bge   cbr0_set_dll_margin

cbr0_check_dll_margin2:                          @ if max(margin-P/N) > g_margin+1 && side_margin >= g_side)
    and   r1, r6, #0xFF
    add   r2, r1, #0x1
    cmp   r3, r2
    ble   cbr0_check_dll_margin3
    cmp   r11, r8
    bge   cbr0_set_dll_margin

cbr0_check_dll_margin3:                          @ if side_margin > g_side && g_side < 8
    cmp   r8, #8
    bge   cbr0_search_dll_margin_e
    cmp   r11, r8
    ble   cbr0_search_dll_margin_e

cbr0_set_dll_margin:
    mov   r1, r6, lsr #16
    cmp   r3, r1
    bicgt r6, r6, #0x00FF0000
    orrgt r6, r6, r3, lsl #16
    bic   r6, r6, #0x000000FF
    orr   r6, r6, r3
    mov   r7, r5
    mov   r8, r11

cbr0_search_dll_margin_e:
    and   r5, r5, #0x7F
    add   r5, r5, #0x01
    cmp   r5, #0x20                              @ last DQIDLY
    blt   cbr0_search_dll_margin_s

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r7, lsl #16
    str   r1, [r0]

    ldr   r0, =0x1e6e0068
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    bic   r1, r1, #0xFF000000
    str   r1, [r0]

    /* Delay about 5us */
    ldr r2, =0x00000005                          @ Set Timer5 Reload = 5 us
    init_delay_timer
delay_5:
    check_delay_timer
    bne delay_5
    clear_delay_timer
    /* End delay 5us */

    ldr r0, =0x1e6e000c                          @ Set refresh cycle
    ldr r1, =0x00005C01
    str r1, [r0]

/******************************************************************************
 Fine tune per bit DQ input delay -- Pass 1, left edge align
  r8  = free
  r9  = DQ fail bit accumulator
  r10 = pattern fail counter, initialize to 5 (fail 5 times)
  r11 = free
 *****************************************************************************/
CBR1_START:
#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    mov r1, #0x43                                @ 'C'
    str r1, [r0]
    mov r1, #0x42                                @ 'B'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x31                                @ '1'
    str r1, [r0]
    /* Debug - UART console message */
#endif

    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL2 parameter index

/****************************
 DLL2 delay margin test loop
 ***************************/
cbr1_next_dll2_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL2 parameter
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    bic   r1, r1, #0xFF000000
    orr   r1, r1, r7, lsl #16
    str   r1, [r0]
    ldr   r2, =0x40404040                        @ parameters max is to 0x40404040
    cmp   r7, r2
    bge   CBR1_END
    ldr   r2, =0x01010101
    add   r7, r7, r2

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 4KB
    ldr   r1, =0x00000FFF
    str   r1, [r0]

/* CBRScan2() start */
    ldr   r9, =0xFFFF                            @ init test status
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr1_next_test_pattern:
    mov   r10, #5                                @ set the retry loop of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr1_test_single

cbr1_test_pattern_end:
    cmp   r9, #0x00
    bne   cbr1_test_pass_dqi
    cmp   r6, #10
    bge   CBR1_END
    b     cbr1_next_dll2_parameter               @ CBRScan2() end and test result fail, go to next step

cbr1_test_pass_dqi:
    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add 1 after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count
    ldr   r0, =0x1e720010
    mov   r8, #0x01
cbr1_test_pass_dqi_loop_s:
    tst   r9, r8
    beq   cbr1_test_pass_dqi_loop_e
    record_dll2_pass_range

cbr1_test_pass_dqi_loop_e:
    add   r0, r0, #0x04
    mov   r8, r8, lsl #1
    ldr   r1, =0xFFFF
    tst   r8, r1
    bne   cbr1_test_pass_dqi_loop_s
    b     cbr1_next_dll2_parameter

/****************************
 Test fail retry loop
 ***************************/
cbr1_pattern_fail_retry:

    /* CBRTest2() start */
cbr1_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000005
    str   r1, [r0]
    ldr   r3, =0x1000
    ldr   r1, =0x1000
cbr1_wait_engine_idle_0:
    subs  r1, r1, #1
    beq   cbr1_test_single_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr1_wait_engine_idle_0

cbr1_test_single_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r11, [r0]
    orr   r11, r11, r11, lsr #16
    bic   r11, r11, #0xFF000000
    bic   r11, r11, #0x00FF0000

    ldr   r1, =0xFFFF
    cmp   r11, r1
    beq   cbr1_test_fail

cbr1_test_burst:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    ldr   r2, =0x00000041
    str   r2, [r0]
    ldr   r3, =0x1000
    ldr   r1, =0x1000
cbr1_wait_engine_idle_1:
    subs  r1, r1, #1
    beq   cbr1_test_burst_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr1_wait_engine_idle_1

cbr1_test_burst_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r2, [r0]
    orr   r2, r2, r2, lsr #16
    bic   r2, r2, #0xFF000000
    bic   r2, r2, #0x00FF0000
    orr   r11, r11, r2

    ldr   r2, =0xFFFF
    cmp   r11, r2
    bne   cbr1_test_pass
    /* CBRTest2() end */

cbr1_test_fail:
    subs  r10, r10, #1
    bne   cbr1_pattern_fail_retry
    mov   r9, #0x00
    b     cbr1_test_pattern_end                  @ CBRScan2() return(0)

cbr1_test_pass:
    ldr   r1, =0xFFFF                            @ record the pass bit
    eor   r11, r11, r1
    and   r9, r9, r11                            @ DQ pass bit
    cmp   r9, #0x00
    beq   cbr1_test_pattern_end                  @ CBRScan2() return(0)

    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr1_next_test_pattern

CBR1_END:
    mov   r5, #0x0                               @ init DQ DLL_min sum
    mov   r6, #0x0                               @ init DQ DLL_min valid count
    ldr   r0, =0x1e72000c
    ldr   r3, =0x1e720050
cbr1_search_dllmin_s:
    add   r0, r0, #0x04
    cmp   r0, r3
    beq   cbr1_search_dllmin_e
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r2, r2, r1                             @ dllmax - dllmin
    bmi   cbr1_search_dllmin_s                   @ no valid margin found, bypass fine tune
    cmp   r2, #10                                @ (dllmax - dllmin) < 10
    blt   cbr1_search_dllmin_s                   @ no enough margin found, bypass fine tune
    add   r5, r5, r1
    add   r6, r6, #1
    b     cbr1_search_dllmin_s

cbr1_search_dllmin_e:
    cmp   r6, #16
    bne   Calibration_Start_pre                  @ not all bits valid, retry again

    mov   r5, r5, lsr #4
    ldr   r0, =0x1e720000
    str   r5, [r0]

    mov   r6, #0x00                              @ init DQL CBR value
    ldr   r0, =0x1e720030
    ldr   r7, =0x1e72000c

cbr1_set_result_dql:
    sub   r0, r0, #4
    cmp   r0, r7
    beq   cbr1_set_result_next
    mov   r6, r6, lsl #3
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    mov   r3, r1                                 @ dll = dllmin
    cmp   r5, r3
    blt   cbr1_set_result_dql_neg
    sub   r1, r5, r3
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    cmp   r1, #2                                 @ dqi_tune max = 2
    movgt r1, #2
    orr   r6, r6, r1
    b     cbr1_set_result_dql

cbr1_set_result_dql_neg:
    sub   r1, r3, r5
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    cmp   r1, #2                                 @ dqi_tune max = -2
    movgt r1, #2
    mov   r2, #8
    sub   r1, r2, r1
    and   r1, r1, #7
    orr   r6, r6, r1
    b     cbr1_set_result_dql

cbr1_set_result_next:
    ldr   r0, =0x1e6e0080                        @ save DQL fine tune result
    str   r6, [r0]
    ldr   r0, =0x1e720094
    str   r6, [r0]

    mov   r6, #0x00                              @ init DQH CBR value
    ldr   r0, =0x1e720050
    ldr   r7, =0x1e72002c

cbr1_set_result_dqh:
    sub   r0, r0, #4
    cmp   r0, r7
    beq   cbr1_set_result_end
    mov   r6, r6, lsl #3
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    mov   r3, r1                                 @ dll = dllmin
    cmp   r5, r3
    blt   cbr1_set_result_dqh_neg
    sub   r1, r5, r3
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    cmp   r1, #3                                 @ dqi_tune max = 2
    movgt r1, #3
    subs  r1, r1, #1
    movmi r1, #7
    orr   r6, r6, r1
    b     cbr1_set_result_dqh

cbr1_set_result_dqh_neg:
    sub   r1, r3, r5
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    add   r1, r1, #1
    cmp   r1, #2                                 @ dqi_tune max = -2
    movgt r1, #2
    mov   r2, #8
    sub   r1, r2, r1
    and   r1, r1, #7
    orr   r6, r6, r1
    b     cbr1_set_result_dqh

cbr1_set_result_end:
    ldr   r0, =0x1e6e0084                        @ save DQH fine tune result
    str   r6, [r0]
    ldr   r0, =0x1e720098
    str   r6, [r0]

/******************************************************************************
 Search the DLL2 detail margin
 *****************************************************************************/
    ldr   r0, =0x1e7200a0
    mov   r1, #0
    str   r1, [r0]

CBR3_START:
#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x33                                @ '3'
    str r1, [r0]
    /* Debug - UART console message */
#endif

    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL2 parameter index
    ldr   r1, =0x000000ff
    ldr   r0, =0x1e720008                        @ init DQL dllmax,dllmin
    str   r1, [r0]
    ldr   r0, =0x1e72000c                        @ init DQH dllmax,dllmin
    str   r1, [r0]

    ldr   r0, =0x1e7200a0                        @ CBR3 iteration counter
    ldr   r1, [r0]
    add   r1, r1, #1
    str   r1, [r0]

/****************************
 DLL2 delay margin test loop
 ***************************/
cbr3_next_dll2_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL2 parameter
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    bic   r1, r1, #0xFF000000
    orr   r1, r1, r7, lsl #16
    str   r1, [r0]
    ldr   r2, =0x40404040                        @ parameters max is to 0x40404040
    cmp   r7, r2
    bge   CBR3_END
    ldr   r2, =0x01010101
    add   r7, r7, r2

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 64KB
    ldr   r1, =0x0000FFFF
    str   r1, [r0]

    /* CBRScan() start */
    mov   r9, #0x03                              @ init test status
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr3_next_test_pattern:
    mov   r10, #5                                @ set the retry loop of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr3_test_single

cbr3_test_pattern_end:
    cmp   r9, #0x00
    bne   cbr3_test_pass_dql
    cmp   r6, #10
    bge   CBR3_END
    b     cbr3_next_dll2_parameter               @ CBRScan() end and test result fail, go to next step

cbr3_test_pass_dql:
    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add one after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count
    tst   r9, #0x01
    beq   cbr3_test_pass_dqh

    ldr   r0, =0x1E720008
    record_dll2_pass_range

cbr3_test_pass_dqh:
    tst   r9, #0x02
    beq   cbr3_next_dll2_parameter
    ldr   r0, =0x1E72000c
    record_dll2_pass_range
    b     cbr3_next_dll2_parameter

/****************************
 Test fail retry loop
 ***************************/
cbr3_pattern_fail_retry:

    /* CBRTest() start */
cbr3_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000005
    str   r1, [r0]
    ldr   r3, =0x1000
    ldr   r8, =0x10000
cbr3_wait_engine_idle_0:
    subs  r8, r8, #1
    beq   cbr3_test_single_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr3_wait_engine_idle_0

cbr3_test_single_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r11, [r0]
    orr   r11, r11, r11, lsr #16
    bic   r11, r11, #0xFF000000
    bic   r11, r11, #0x00FF0000

    ldr   r1, =0xFF
    tst   r11, r1
    beq   cbr3_test_burst
    tst   r11, r1, lsl #8
    bne   cbr3_test_fail

cbr3_test_burst:
    mov   r1, #0x00                              @ initialize loop index, r1 is loop's index
cbr3_test_burst_loop:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, r1, lsl #3
    orr   r2, r2, #0x41                          @ test command = 0x41 | (datagen << 3)
    str   r2, [r0]
    ldr   r3, =0x1000
    ldr   r8, =0x10000
cbr3_wait_engine_idle_1:
    subs  r8, r8, #1
    beq   cbr3_test_burst_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr3_wait_engine_idle_1

cbr3_test_burst_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r2, [r0]
    orr   r2, r2, r2, lsr #16
    bic   r2, r2, #0xFF000000
    bic   r2, r2, #0x00FF0000
    orr   r11, r11, r2

    ldr   r2, =0xFF
    tst   r11, r2
    beq   cbr3_next_test_burst_mode
    tst   r11, r2, lsl #8
    beq   cbr3_next_test_burst_mode
    /* CBRTest() end */

cbr3_test_fail:
    subs  r10, r10, #1
    bne   cbr3_pattern_fail_retry
    mov   r9, #0x00
    b     cbr3_test_pattern_end                  @ CBRScan() return(0)

cbr3_next_test_burst_mode:
    add   r1, r1, #1                             @ increase the test mode index
    cmp   r1, #0x08                              @ there are 8 modes
    bne   cbr3_test_burst_loop

    ldr   r1, =0xFF                              @ record the pass byte
    tst   r11, r1
    andne r9, r9, #0x02                          @ DQL fail
    tst   r11, r1, lsl #8
    andne r9, r9, #0x01                          @ DQH fail
    cmp   r9, #0x00
    beq   cbr3_test_pattern_end                  @ CBRScan() return(0)

    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr3_next_test_pattern

CBR3_END:
    ldr   r0, =0x1e72000c                        @ check DQH margin
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r5, r2, r1                             @ dllmax - dllmin
    bmi   CBR3_START                             @ no valid margin found, retry again
    cmp   r5, #10                                @ (dllmax - dllmin) < 10
    blt   CBR3_START                             @ no enough margin found, retry again
    add   r2, r1, r2                             @ (dllmin[1] + dllmax[1] + 1) >> 1
    add   r2, r2, #0x01
    mov   r1, r2, lsr #1
    mov   r3, r1, lsl #8
    ldr   r1, [r0]                               @ store the dll search result
    bic   r1, r1, #0xFF000000
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r3, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e720008                        @ check DQL margin
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r5, r2, r1                             @ dllmax - dllmin
    bmi   CBR3_START                             @ no valid margin found, retry again
    cmp   r5, #10                                @ (dllmax - dllmin) < 10
    blt   CBR3_START                             @ no enough margin found, retry again
    add   r2, r1, r2                             @ (dllmin[0] + dllmax[0] + 1) >> 1
    add   r2, r2, #0x01
    mov   r1, r2, lsr #1
    ldr   r2, [r0]                               @ store the dll search result
    bic   r2, r2, #0xFF000000
    bic   r2, r2, #0x00FF0000
    orr   r2, r2, r1, lsl #16
    str   r2, [r0]
    orr   r3, r3, r1

    ldr   r0, =0x1e6e0068                        @ save the result dll value
    ldr   r1, [r0]
    bic   r1, r1, #0xFF000000
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r3, lsl #16
    str   r1, [r0]
    b     CBR4_START
.LTORG

/******************************************************************************
 Search the DQS input mask margin
 *****************************************************************************/
CBR4_START:
#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x34                                @ '4'
    str r1, [r0]
    /* Debug - UART console message */
#endif
    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 4KB
    ldr   r1, =0x00000FFF
    str   r1, [r0]

    mov   r8, #0x00                              @ init MCR18[4]
    ldr   r1, =0x000000ff
    ldr   r0, =0x1e7200b0                        @ init MCR18[4]=0 max,min
    str   r1, [r0]
    ldr   r0, =0x1e7200b4                        @ init MCR18[4]=1 max,min
    str   r1, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0x0000001F
    str   r1, [r0]

    b     cbr4_scan_start

cbr4_next_maskdly:
    add   r8, r8, #0x01
    and   r2, r8, #0x01
    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0x0000001F
    orr   r1, r1, r2, lsl #4
    str   r1, [r0]
    cmp   r8, #0x02
    bge   CBR4_END

cbr4_scan_start:
    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init mask delay

/****************************
 DQS Mask delay margin test loop
 ***************************/
cbr4_next_parameter:
    cmp   r7, #0x10                              @ max delay = 0xF
    bge   cbr4_next_maskdly
    ldr   r0, =0x1e6e0018                        @ load MCR18 parameter
    ldr   r1, [r0]
    bic   r1, r1, #0x0000000F
    orr   r1, r1, r7
    str   r1, [r0]
    add   r7, r7, #0x01

    /* CBRScan3() start */
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr4_next_test_pattern:
    mov   r10, #2                                @ set the retry loop = 2 of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr4_test_burst

    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add 1 after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count

    ldr   r0, =0x1e7200b0                        @ record pass window
    add   r0, r0, r8, lsl #2
    record_dll2_pass_range
    mov   r2, #0x01
    add   r1, r1, r2, lsl #16
    str   r1, [r0]
    b     cbr4_next_parameter

cbr4_test_pattern_fail:
    cmp   r6, #5                                 @ passcnt >= 5
    bge   cbr4_next_maskdly
    b     cbr4_next_parameter

/****************************
 Test fail retry loop
 ***************************/
cbr4_pattern_fail_retry:
    /* CBRTest3() start */
cbr4_test_burst:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x000000C1
    str   r1, [r0]
    ldr   r3, =0x3000
cbr4_wait_engine_idle_0:
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr4_wait_engine_idle_0

    ldr   r2, [r0]                               @ read fail bit status
    mov   r1, #0x0
    str   r1, [r0]
    mov   r2, r2, lsr #13                        @ D[13] = fail bit
    cmp   r2, #0x00
    bne   cbr4_test_fail

cbr4_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000085
    str   r1, [r0]
    ldr   r3, =0x3000
cbr4_wait_engine_idle_1:
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr4_wait_engine_idle_1

    ldr   r2, [r0]                               @ read fail bit status
    mov   r1, #0x0
    str   r1, [r0]
    mov   r2, r2, lsr #13                        @ D[13] = fail bit
    cmp   r2, #0x00
    beq   cbr4_test_pass
    /* CBRTest3() end */

cbr4_test_fail:
    subs  r10, r10, #1
    bne   cbr4_pattern_fail_retry
    b     cbr4_test_pattern_fail                 @ CBRScan3() return(0)

cbr4_test_pass:
    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr4_next_test_pattern

CBR4_END:
    ldr   r0, =0x1e7200b0                        @ check mask margin
    ldr   r1, [r0]
    add   r0, r0, #0x04
    ldr   r2, [r0]
    ands  r6, r2, #0xFF                          @ get min of MCR18[4] = 1
    bne   cbr4_noset_delay
    ands  r5, r1, #0xFF                          @ get min of MCR18[4] = 0
    bne   cbr4_set_delay
    mov   r1, r1, lsr #8                         @ get max of MCR18[4] = 0
    and   r1, r1, #0xFF
    mov   r2, r2, lsr #8                         @ get max of MCR18[4] = 1
    and   r2, r2, #0xFF
    sub   r1, r1, r5
    sub   r2, r2, r6
    cmp   r1, r2
    bge   cbr4_noset_delay

cbr4_set_delay:
    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    orr   r1, r1, #0x10
    str   r1, [r0]

cbr4_noset_delay:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
/******************************************************************************
 CBR Finish
 *****************************************************************************/

/******************************************************************************
 Check DRAM Size
 *****************************************************************************/
    ldr   r0, =0x1e6e2070
    ldr   r1, [r0]
    bic   r1, r1, #0xFEFFFFFF                    @ bit[24]=1 => DDR2
    mov   r2, r1, lsr #24
    cmp   r2, #0x01
    beq   platform_unsupported_exit              @ DDR2 not supported

    ldr   r0, =0x1e6e0004
    ldr   r5, [r0]
    bic   r5, r5, #0x00000003                    @ record MCR04
    orr   r1, r5, #0x3
    str   r1, [r0]                               @ set to 4Gbit
    ldr   r6, =0x003F2217
#if defined(CONFIG_DRAM_336)
    ldr   r6, =0x00361C13
#endif
    b     check_dram_size

check_dram_size:
    ldr   r0, =0x50100000
    ldr   r1, =0x41424344
    str   r1, [r0]
    ldr   r0, =0x48100000
    ldr   r1, =0x25262728
    str   r1, [r0]
    ldr   r0, =0x40100000
    ldr   r1, =0x191A1B1C
    str   r1, [r0]
    ldr   r0, =0x50100000
    ldr   r1, =0x41424344
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 4Gbit
    orreq r5, r5, #0x03
    moveq r6, r6, lsr #16
    beq   check_dram_size_end
    ldr   r0, =0x48100000
    ldr   r1, =0x25262728
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 2Gbit
    orreq r5, r5, #0x02
    moveq r6, r6, lsr #8
    beq   check_dram_size_end
    orr   r5, r5, #0x01                          @ == 1Gbit

check_dram_size_end:
    ldr   r0, =0x1e6e0004
    str   r5, [r0]
    ldr   r0, =0x1e6e0014
    ldr   r1, [r0]
    bic   r1, r1, #0x000000FF
    and   r6, r6, #0xFF
    orr   r1, r1, r6
    str   r1, [r0]

    ldr   r0, =0x1e6e0120                        @ VGA Compatible Mode
    ldr   r1, =0x000050C0                        @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr   r1, =0x00004DC0
#endif
    str   r1, [r0]
/******************************************************************************
 Calibration Code End
 ******************************************************************************/

/******************************************************************************
 Version Number
 *****************************************************************************/
    ldr   r0, =0x1e7200a8
    ldr   r1, =0x20141229                        @ released date
    str   r1, [r0]

    add   r0, r0, #4
    ldr   r1, =0x00000060                        @ released SDK version
    str   r1, [r0]

set_scratch:
    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]
#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x6F                                @ 'o'
    str r1, [r0]
    mov r1, #0x6E                                @ 'n'
    str r1, [r0]
    mov r1, #0x65                                @ 'e'
    str r1, [r0]
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    /* Debug - UART console message */
#endif

/******************************************************************************
 Solve PCIe ASPM issue, only applied to AST2300 series
 ******************************************************************************/
    ldr r0, =0x1e6e207c                          @ Check bounding for AST1150 existence
    ldr r1, [r0]
    mov r2, r1, lsr #24
    cmp r2, #0x01
    bne platform_exit                            @ not match AST2300
    bic r1, r1, #0xFFFFFCFF
    mov r1, r1, lsr #8
    cmp r1, #0x02
    beq platform_exit                            @ match AST1050

    ldr r0, =0x1e6e2004                          @ Disable I2C controller reset
    ldr r1, [r0]
    orr r1, r1, #0x04
    str r1, [r0]
    bic r1, r1, #0x04
    str r1, [r0]

    ldr r0, =0x1e78a054                          @ Check I2C bus state, if busy then quit
    ldr r1, [r0]
    mov r1, r1, lsr #17
    and r1, r1, #0x03
    cmp r1, #0x03
    bne platform_exit

    ldr r0, =0x1e78a040                          @ Init I2C1 controller
    mov r1, #0x01
    orr r1, r1, r1, lsl #16
    str r1, [r0]

    ldr r0, =0x1e78a044
    ldr r1, =0x77776704
    str r1, [r0]

    mov r1, #0x0
    ldr r0, =0x1e78a048
    str r1, [r0]
    ldr r0, =0x1e78a04c
    str r1, [r0]

    ldr r0, =0x1e78a050
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

    ldr r0, =0x1e78a200                          @ Set AST1150 I2C password
    ldr r1, =0x00A88FA8
    str r1, [r0]

    ldr r0, =0x1e78a05c
    ldr r1, =0x00000200                          @ Enable buffer mode transfering 3 bytes
    str r1, [r0]

    ldr r0, =0x1e78a054
    ldr r1, =0x00000063                          @ Fire commmand
    str r1, [r0]

    ldr r0, =0x1e78a050
i2c_wait_cmddone_1:
    ldr r1, [r0]
    tst r1, #0x38
    beq i2c_wait_cmddone_1
    tst r1, #0x2A                                @ transmit error
    bne platform_exit2
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

    ldr r0, =0x1e78a200                          @ Disable ASPM capability
    ldr r1, =0x04005DA8
    str r1, [r0]

    ldr r0, =0x1e78a204
    ldr r1, =0x00000024
    str r1, [r0]

    ldr r0, =0x1e78a05c
    ldr r1, =0x00000200                          @ Enable buffer mode transfering 3 bytes
    str r1, [r0]

    ldr r0, =0x1e78a054
    ldr r1, =0x00000063                          @ Fire commmand
    str r1, [r0]

    ldr r0, =0x1e78a050
i2c_wait_cmddone_2:
    ldr r1, [r0]
    tst r1, #0x38
    beq i2c_wait_cmddone_2
    tst r1, #0x2A                                @ transmit error
    bne platform_exit2
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

platform_exit2:
    ldr r0, =0x1e78a040                          @ Disable I2C1 controller
    mov r1, #0x00
    str r1, [r0]

    b   platform_exit
.LTORG
