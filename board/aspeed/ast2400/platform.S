/*
 * AST2400 DDR Calibration tests
 *
 * (C) Copyright 2004, ASPEED Technology Inc.
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

/* 
 * Board specific setup info
 *
 * AST2300/AST2400 DDR2/DDR3 SDRAM initialization and calibration
 *
 * Optional configurations:
 * 1. DRAM Speed             //
 *    CONFIG_DRAM_336        // 336MHz (DDR-667)
 *    CONFIG_DRAM_408        // 408MHz (DDR-800) (default)
 * 2. ECC Function enable
 *    CONFIG_DRAM_ECC        // define to enable ECC function
 *                           // must define the ECC protected memory size
 *                           // at 0x1e6e0054
 * 3. UART5 message output   //
 *    CONFIG_DRAM_UART_38400 // set the UART baud to 38400, default is 115200
 */

#include <config.h>
#include <version.h>
#include "macros.h"

.globl lowlevel_init
.globl platform_exit
.globl platform_unsupported_exit

lowlevel_init:

init_dram:
    /* Save lr for platform exit */
    mov r4, lr
    /* Test - DRAM initial time */
    ldr r0, =0x1e782044
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

    ldr r0, =0x1e782030
    ldr r1, [r0]
    bic r1, r1, #0x0000F000
    str r1, [r0]
    mov r2, #3
    orr r1, r1, r2, lsl #12
    str r1, [r0]
    /* End Test - DRAM initial time */

    /*Set Scratch register Bit 7 before initialize*/
    ldr r0, =0x1e6e2000
    ldr r1, =0x1688a8a8
    str r1, [r0]

    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

    /* Fix LPC lock issue for AST2300 */
    ldr r0, =0x1e6e207c                          @ Check AST2300
    ldr r1, [r0]
    mov r1, r1, lsr #24
    cmp r1, #0x01
    beq platform_unsupported_exit                @ Matches AST2300
    ldr r0, =0x1e7890a0                          @ set LHCR0[31:0]=0xFFFFFF00
    ldr r1, =0xFFFFFF00
    str r1, [r0]
    /* End Fix LPC lock issue for AST2300 */

    /* Check Scratch Register Bit 6 */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

    ldr r2, =0x033103F1                          @ load PLL parameter for 24Mhz CLKIN (396:324)
    ldr r0, =0x1e6e207c                          @ Check Revision ID
    ldr r1, [r0]
    mov r1, r1, lsr #24
    cmp r1, #0x02
    bne set_MPLL                                 @ not match AST2400

    ldr r0, =0x1e6e2070                          @ Check CLKIN freq
    ldr r1, [r0]
    mov r1, r1, lsr #23
    tst r1, #0x01
    ldrne r2, =0x017001D0                        @ load PLL parameter for 25Mhz CLKIN (400:325)

set_MPLL:
    ldr r0, =0x1e6e2020                          @ M-PLL (DDR SDRAM) Frequency
    ldr r1, =0xFFFF
#if defined(CONFIG_DRAM_336)
    mov r2, r2, lsr #16
#endif
    and r1, r2, r1
    str r1, [r0]

  /* Yosemite Platform specific Initialization */
#ifdef CONFIG_TARGET_FBYOSEMITE
  /*
   * Disable WDT2 before the 2nd SPI flash is tried
   * TODO: Need to detect booting from 2nd flash and recover
   */
    ldr r0, =0x1e78502c
    ldr r1, [r0]
    ldr r2, =0xFFFFFFFE
    and r1, r1, r2
    str r1, [r0]

    console_sel

  /* Enable UART3/4 only for non-yosemite platform */
    ldr r0, =0x1e6e2080
    ldr r1, =0xFFFF0000                          @ enable UART3 and UART4
    str r1, [r0]
#endif

    ldr r0, =CONFIG_ASPEED_COM_LCR
    mov r1, #0x83
    str r1, [r0]

    ldr r0, =0x1e6e202c
    ldr r2, [r0]
    mov r2, r2, lsr #12
    tst r2, #0x01
    ldr r0, =CONFIG_ASPEED_COM
#if CONFIG_BAUDRATE == 115200
    moveq r1, #0x0D                              @ Baudrate 115200
    movne r1, #0x01                              @ Baudrate 115200, div13
#endif
#if CONFIG_BAUDRATE == 57600
    moveq r1, #0x1A                              @ Baudrate 57600
    movne r1, #0x02                              @ Baudrate 57600, div13
#endif
#if CONFIG_BAUDRATE == 38400
    moveq r1, #0x27                              @ Baudrate 38400
    movne r1, #0x03                              @ Baudrate 38400 , div13
#endif
#if CONFIG_BAUDRATE == 9600
    moveq r1, #0x9c                              @ Baudrate 9600
    movne r1, #0x0C                              @ Baudrate 9600 , div13
#endif
    str r1, [r0]

    ldr r0, =CONFIG_ASPEED_COM_IER
    mov r1, #0x00
    str r1, [r0]

    ldr r0, =CONFIG_ASPEED_COM_LCR
    mov r1, #0x03
    str r1, [r0]

    ldr r0, =CONFIG_ASPEED_COM_IIR
    mov r1, #0x07
    str r1, [r0]

#ifdef DEBUG
    /* Debug - UART console message */
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x41                                @ 'A'
    str r1, [r0]
    mov r1, #0x4D                                @ 'M'
    str r1, [r0]
    mov r1, #0x20                                @ ' '
    str r1, [r0]
    mov r1, #0x49                                @ 'I'
    str r1, [r0]
    mov r1, #0x6E                                @ 'n'
    str r1, [r0]
    mov r1, #0x69                                @ 'i'
    str r1, [r0]
    mov r1, #0x74                                @ 't'
    str r1, [r0]
    mov r1, #0x2D                                @ '-'
    str r1, [r0]
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    /* End Debug - UART console message */
#endif

    /* Delay about 100us */
    ldr r0, =0x1e782030                          @ Init Timer3 Control
    ldr r1, [r0]
    bic r1, r1, #0x00000F00
    str r1, [r0]

    ldr r2, =0x00000064                          @ Set Timer3 Reload = 100 us
    init_delay_timer
delay_0:
    check_delay_timer
    bne delay_0
    clear_delay_timer
    /* End delay 100us */

/******************************************************************************
 Init DRAM common registers
 ******************************************************************************/
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    /* Reset MMC */
    ldr r1, =0x00000000
    ldr r0, =0x1e6e0034
    str r1, [r0]
    ldr r0, =0x1e6e0018
    str r1, [r0]
    ldr r0, =0x1e6e0024
    str r1, [r0]
    ldr r0, =0x1e6e0064                          @ REG_MADJ, power down DLL
    str r1, [r0]

    ldr r1, =0x00034C4C                          @ REG_MADJ, reset DLL
    str r1, [r0]

    ldr r0, =0x1e6e0068                          @ REG_SADJ
    ldr r1, =0x00001800
    str r1, [r0]

    /* Delay about 10us */
    ldr r2, =0x0000000B                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay_1:
    check_delay_timer
    bne delay_1
    clear_delay_timer
    /* End delay 10us */

    ldr r0, =0x1e6e0064                          @ REG_MADJ | 0xC0000, enable DLL
    ldr r1, [r0]
    ldr r2, =0xC0000
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e0008
    ldr r1, =0x0090040f                          /* VGA */
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x4000A120
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x00000120
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0xFF444444
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x22222222
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x22222222
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x22222222
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x80000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0060                          @ REG_DRV
    ldr r1, =0x000000FA                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x000000FA
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
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

    ldr r0, =0x1e6e0080
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0084
    ldr r1, =0x00FFFFFF
    str r1, [r0]

    ldr r0, =0x1e6e0088                          @ REG_DQIDLY
    ldr r1, =0x00000089                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x00000074
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0020                          @ REG_DQSIC
    ldr r1, =0x000000E2                          @ 408 MHz
#if defined(CONFIG_DRAM_336)
    ldr r1, =0x000000BA
#endif
    str r1, [r0]

    /* Delay about 10us */
    ldr r2, =0x0000000B                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay_2:
    check_delay_timer
    bne delay_2
    clear_delay_timer
    /* End delay 10us */

    /* Check DRAM Type by H/W Trapping */
    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xFEFFFFFF                      @ bit[24]=1 => DDR2
    mov r2, r1, lsr #24
    cmp r2, #0x01
    beq platform_unsupported_exit                @ DDR2 not supported on AST2400+
    b   ddr3_init
.LTORG

platform_unsupported_exit:
    ldr r0, =CONFIG_ASPEED_COM
    mov r1, #0x55                                @ 'U'
    str r1, [r0]
    mov r1, #0x6E                                @ 'n'
    str r1, [r0]
    mov r1, #0x73                                @ 's'
    str r1, [r0]
    mov r1, #0x75                                @ 'u'
    str r1, [r0]
    mov r1, #0x70                                @ 'p'
    str r1, [r0]
    mov r1, #0x70                                @ 'p'
    str r1, [r0]
    mov r1, #0x6F                                @ 'o'
    str r1, [r0]
    mov r1, #0x72                                @ 'r'
    str r1, [r0]
    mov r1, #0x74                                @ 't'
    str r1, [r0]
    mov r1, #0x65                                @ 'e'
    str r1, [r0]
    mov r1, #0x64                                @ 'd'
    str r1, [r0]
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]

platform_exit:
    /*
     * Fix for enabling BMC reset without hang using external reset pulse
     * Need to disable WDT2 without which the reset will hang
     */
#ifdef CONFIG_TARGET_FBYOSEMITE
    ldr r0, =0x1e78502c
    ldr r1, [r0]
    ldr r2, =0xFFFFFFFE
    and r1, r1, r2
    str r1, [r0]

    console_sel
#endif
#ifdef CONFIG_DRAM_ECC
    ldr r0, =0x1e6e0004
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x05000000                          /* ECC protected memory size, default set at 80M   */
    str r1, [r0]

    ldr r0, =0x1e6e007C
    ldr r1, =0x00000000
    str r1, [r0]
    ldr r0, =0x1e6e0074
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000221
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r2, =0x00001000
ECC_Init_Flag:
    ldr r1, [r0]
    tst r1, r2                                   @ D[12] = 1, Done
    beq ECC_Init_Flag

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x80000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000400
    str r1, [r0]
#endif
    ldr r0, =0x1e6e2008                          @ Set Video ECLK phase
    ldr r1, [r0]
    ldr r2, =0xfffffff3
    and r1, r1, r2
    orr r1, r1, #0x08
    str r1, [r0]

#ifndef CONFIG_ASPEED_ENABLE_JTAG
    ldr r0, =0x1e6e2004
    ldr r1, [r0]
    ldr r2, =0xFFBFFFFF                          @ Enable JTAG Master, solve ARM stucked by JTAG issue
    and r1, r1, r2
    str r1, [r0]
#endif

    ldr r0, =0x1e6e2048                          @ Set MAC interface delay timing
    ldr r1, =0x2255
    str r1, [r0]

    ldr r0, =0x1e6e2070                          @ Set MAC AHB bus clock
    ldr r1, [r0]
    mov r2, #0x04                                @ Default RMII, set MHCLK = HPLL/10
    tst r1, #0xC0
    movne r2, #0x02                              @ if RGMII,     set MHCLK = HPLL/6
    ldr r0, =0x1e6e2008
    ldr r1, [r0]
    bic r1, r1, #0x00070000
    orr r1, r1, r2, lsl #16
    str r1, [r0]

    /* Test - DRAM initial time */
    ldr r0, =0x1e782040
    ldr r1, [r0]
    ldr r0, =0xFFFFFFFF
    sub r1, r0, r1
    ldr r0, =0x1e72009c
    str r1, [r0]
    ldr r0, =0x1e7200a4
    str r1, [r0]
    ldr r0, =0x1e782030
    ldr r1, [r0]
    bic r1, r1, #0x0000F000
    str r1, [r0]
    /* End Test - DRAM initial time */

/******************************************************************************
 Reset GPIO registers when watchdog reset
 ******************************************************************************/
    ldr r0, =0x1e6e207c                          @ Check Revision ID
    ldr r1, [r0]
    mov r1, r1, lsr #24
    cmp r1, #0x02
    bne platform_exit3                           @ not match AST2400

    ldr r0, =0x1e6e203c                          @ Check watchdog reset event
    ldr r1, [r0]
    and r1, r1, #0x06
    cmp r1, #0x0
    beq platform_exit3                           @ no watchdog reset event

    ldr r0, =0x1e6e209c                          @ Check watchdog GPIO selection
    ldr r1, [r0]
    mov r1, r1, lsr #21
    tst r1, #0x01
    beq platform_exit3                           @ no watchdog reset selection

    ldr r1, =0x00000000                          @ clear GPIO register reset by PRST_N
    ldr r2, =0xFFFFFFFF
    ldr r0, =0x1e780008
    str r1, [r0]
    ldr r0, =0x1e78000c
    str r1, [r0]
    ldr r0, =0x1e780010
    str r1, [r0]
    ldr r0, =0x1e780014
    str r1, [r0]
    ldr r0, =0x1e780018
    str r2, [r0]
    ldr r0, =0x1e780028
    str r1, [r0]
    ldr r0, =0x1e78002c
    str r1, [r0]
    ldr r0, =0x1e780030
    str r1, [r0]
    ldr r0, =0x1e780034
    str r1, [r0]
    ldr r0, =0x1e780038
    str r2, [r0]
    ldr r0, =0x1e780040
    str r1, [r0]
    ldr r0, =0x1e780044
    str r1, [r0]
    ldr r0, =0x1e780048
    str r1, [r0]
    ldr r0, =0x1e78004c
    str r1, [r0]
    ldr r0, =0x1e780050
    str r1, [r0]
    ldr r0, =0x1e780054
    str r1, [r0]
    ldr r0, =0x1e780058
    str r1, [r0]
    ldr r0, =0x1e780060
    str r1, [r0]
    ldr r0, =0x1e780064
    str r1, [r0]
    ldr r0, =0x1e780068
    str r1, [r0]
    ldr r0, =0x1e78006c
    str r1, [r0]
    ldr r0, =0x1e780090
    str r1, [r0]
    ldr r0, =0x1e780094
    str r1, [r0]
    ldr r0, =0x1e780098
    str r1, [r0]
    ldr r0, =0x1e78009c
    str r1, [r0]
    ldr r0, =0x1e7800a0
    str r1, [r0]
    ldr r0, =0x1e7800a4
    str r1, [r0]
    ldr r0, =0x1e7800a8
    str r2, [r0]
    ldr r0, =0x1e7800b0
    str r1, [r0]
    ldr r0, =0x1e7800b4
    str r1, [r0]
    ldr r0, =0x1e7800b8
    str r1, [r0]
    ldr r0, =0x1e7800e0
    str r1, [r0]
    ldr r0, =0x1e7800e4
    str r1, [r0]
    ldr r0, =0x1e7800e8
    str r1, [r0]
    ldr r0, =0x1e7800ec
    str r1, [r0]
    ldr r0, =0x1e7800f0
    str r1, [r0]
    ldr r0, =0x1e7800f4
    str r1, [r0]
    ldr r0, =0x1e7800f8
    str r2, [r0]
    ldr r0, =0x1e780100
    str r1, [r0]
    ldr r0, =0x1e780104
    str r1, [r0]
    ldr r0, =0x1e780108
    str r1, [r0]
    ldr r0, =0x1e780110
    str r1, [r0]
    ldr r0, =0x1e780114
    str r1, [r0]
    ldr r0, =0x1e780118
    str r1, [r0]
    ldr r0, =0x1e78011c
    str r1, [r0]
    ldr r0, =0x1e780120
    str r1, [r0]
    ldr r0, =0x1e780124
    str r1, [r0]
    ldr r0, =0x1e780128
    str r2, [r0]
    ldr r0, =0x1e780130
    str r1, [r0]
    ldr r0, =0x1e780134
    str r1, [r0]
    ldr r0, =0x1e780138
    str r1, [r0]
    ldr r0, =0x1e780140
    str r1, [r0]
    ldr r0, =0x1e780144
    str r1, [r0]
    ldr r0, =0x1e780148
    str r1, [r0]
    ldr r0, =0x1e78014c
    str r1, [r0]
    ldr r0, =0x1e780150
    str r1, [r0]
    ldr r0, =0x1e780154
    str r1, [r0]
    ldr r0, =0x1e780158
    str r2, [r0]
    ldr r0, =0x1e780160
    str r1, [r0]
    ldr r0, =0x1e780164
    str r1, [r0]
    ldr r0, =0x1e780168
    str r1, [r0]
    ldr r0, =0x1e780170
    str r1, [r0]
    ldr r0, =0x1e780174
    str r1, [r0]
    ldr r0, =0x1e780178
    str r1, [r0]
    ldr r0, =0x1e78017c
    str r1, [r0]
    ldr r0, =0x1e780180
    str r1, [r0]
    ldr r0, =0x1e780184
    str r1, [r0]
    ldr r0, =0x1e780188
    str r2, [r0]
    ldr r0, =0x1e780190
    str r1, [r0]
    ldr r0, =0x1e780194
    str r1, [r0]
    ldr r0, =0x1e780198
    str r1, [r0]
    ldr r0, =0x1e7801d0
    str r1, [r0]
    ldr r0, =0x1e7801d4
    str r1, [r0]

    ldr r0, =0x1e780204                          @ clear SGPIOM register reset by PRST_N
    str r1, [r0]
    ldr r0, =0x1e780208
    str r1, [r0]
    ldr r0, =0x1e78020c
    str r1, [r0]
    ldr r0, =0x1e780210
    str r1, [r0]
    ldr r0, =0x1e780214
    str r2, [r0]
    ldr r0, =0x1e780220
    str r1, [r0]
    ldr r0, =0x1e780224
    str r1, [r0]
    ldr r0, =0x1e780228
    str r1, [r0]
    ldr r0, =0x1e78022c
    str r1, [r0]
    ldr r0, =0x1e780230
    str r2, [r0]
    ldr r0, =0x1e78023c
    str r1, [r0]
    ldr r0, =0x1e780240
    str r1, [r0]
    ldr r0, =0x1e780244
    str r1, [r0]
    ldr r0, =0x1e780248
    str r1, [r0]
    ldr r0, =0x1e78024c
    str r2, [r0]
    ldr r0, =0x1e780254
    ldr r3, =0x01000040
    str r3, [r0]
    ldr r0, =0x1e780258
    str r1, [r0]
    ldr r0, =0x1e78025c
    str r1, [r0]
    ldr r0, =0x1e780260
    str r1, [r0]

    ldr r0, =0x1e780300                          @ clear SGPIOS register reset by PRST_N
    str r1, [r0]
    ldr r0, =0x1e780304
    str r1, [r0]
    ldr r0, =0x1e780308
    str r1, [r0]
    ldr r0, =0x1e78030c
    str r1, [r0]
    ldr r0, =0x1e780310
    str r1, [r0]
    ldr r0, =0x1e780314
    str r1, [r0]
    ldr r0, =0x1e780318
    str r2, [r0]
    ldr r0, =0x1e78031c
    str r2, [r0]
    ldr r0, =0x1e780320
    str r2, [r0]

platform_exit3:
/******************************************************************************
 SPI Timing Calibration, not applicable to AST2300 series
 ******************************************************************************/
    ldr r0, =0x1e6e207c                          @ Check Revision ID
    ldr r1, [r0]
    mov r1, r1, lsr #24
    cmp r1, #0x02
    blt platform_exit4                           @ not match AST2400 or later

    ldr r0, =0x1e6e2070                          @ Check SPI flash
    ldr r1, [r0]
    and r1, r1, #0x03
    cmp r1, #0x02
    bne platform_exit4

    mov r2, #0x0
    mov r6, #0x0
    mov r7, #0x0
    init_spi_checksum

spi_checksum_wait_0:
    ldr r1, [r0]
    tst r1, r2
    beq spi_checksum_wait_0
    ldr r0, =0x1e620090
    ldr r5, [r0]                                 @ record golden checksum
    ldr r0, =0x1e620080
    mov r1, #0x0
    str r1, [r0]

    ldr r0, =0x1e620010                          @ set to fast read mode
    ldr r1, =0x000B0041
    str r1, [r0]

    ldr r6, =0x00F7E6D0                          @ Init spiclk loop
    mov r8, #0x0                                 @ Init delay record

spi_cbr_next_clkrate:
    mov r6, r6, lsr #0x4
    cmp r6, #0x0
    beq spi_cbr_end

    mov r7, #0x0                                 @ Init delay loop
    mov r8, r8, lsl #4

spi_cbr_next_delay_s:
    mov r2, #0x8
    init_spi_checksum
spi_checksum_wait_1:
    ldr r1, [r0]
    tst r1, r2
    beq spi_checksum_wait_1
    ldr r0, =0x1e620090
    ldr r2, [r0]                                 @ read checksum
    ldr r0, =0x1e620080
    mov r1, #0x0
    str r1, [r0]
    cmp r2, r5
    bne spi_cbr_next_delay_e

    mov r2, #0x0
    init_spi_checksum
spi_checksum_wait_2:
    ldr r1, [r0]
    tst r1, r2
    beq spi_checksum_wait_2
    ldr r0, =0x1e620090
    ldr r2, [r0]                                 @ read checksum
    ldr r0, =0x1e620080
    mov r1, #0x0
    str r1, [r0]
    cmp r2, r5
    bne spi_cbr_next_delay_e
    orr r8, r8, r7                               @ record passed delay
    b   spi_cbr_next_clkrate

spi_cbr_next_delay_e:
    add r7, r7, #0x1
    cmp r7, #0x6
    blt spi_cbr_next_delay_s
    b   spi_cbr_next_clkrate

spi_cbr_end:
    ldr r0, =0x1e620094
    str r8, [r0]
    ldr r0, =0x1e620010
    mov r1, #0x0
    str r1, [r0]

platform_exit4:
    /* Restore lr and return. */
    mov lr, r4
    mov pc, lr
