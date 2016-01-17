/*
 * Board specific setup info
 *
 ******************************************************************************
 * ASPEED Technology Inc.
 * AST2500 FPGA DDR3 SDRAM controller initialization and calibration sequence
 *
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * Release date:
 *
 * Optional define variable
 ******************************************************************************
 */

#include <config.h>
#include <version.h>

/******************************************************************************
 Calibration Macro Start
 Usable registers:
  r0, r1, r2, r3, r5, r6, r7, r8, r9, r10, r11
 ******************************************************************************/
/* PATTERN_TABLE,
   init_delay_timer,
   check_delay_timer,
   clear_delay_timer,
   record_pll_pass_range,
   record_pll_pass_range_h,
   are for DRAM calibration */

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
    mov r2, #7
    mov r1, r2, lsl #8
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

    .macro record_pll_pass_range
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

    .macro record_pll_pass_range_h
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

/******************************************************************************
 Calibration Macro End
 ******************************************************************************/

.globl lowlevel_init
lowlevel_init:

    /* save lr */
    mov   r4, lr

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

#if (CONFIG_AST_FPGA_VER == 4)
init_arm11:
    /* Start of ES40004A PLL init */
    /* Step 1. Program PLL_config and keep power down */
    ldr   r0, =0x33000000
    ldr   r1, =0x01000000
    str   r1, [r0]
    ldr   r1, =0x0102001A                        @ 324 MHz
    str   r1, [r0]

    /* Step 2. Wait 1us for PLL initialization */
    ldr   r2, =0x00000100
delay_ES40004A_pll_init:
    subs  r2, r2, #1
    bne   delay_ES40004A_pll_init

    /* Step 3. Program PLL_config to exit Power down */
    ldr   r1, =0x0002001A
    str   r1, [r0]

    /* Step 4. Check pll_ld = 1?. Read PLL_config, check bit 27. */
    ldr   r2, =0x08000000                        @ bit[27] PLL lock detection
check_pll_ld:
    ldr   r1, [r0]
    tst   r1, r2
    beq   check_pll_ld

    /* Step 5. Program aclk_div */
    ldr   r0, =0x33000004
    ldr   r1, =0x00000007                        @ CPU/AXI = 8/1
    str   r1, [r0]

    /* Step 6. Program set_pll */
    ldr   r1, =0x00010007
    str   r1, [r0]
    /* End of ES40004A PLL init */
#endif

    /* Check Scratch Register Bit 6 */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

/* Debug - UART console message */
    ldr   r0, =0x1e78400c
    mov   r1, #0x83
    str   r1, [r0]

    ldr   r0, =0x1e6e202c
    ldr   r2, [r0]
    mov   r2, r2, lsr #12
    tst   r2, #0x01
    ldr   r0, =0x1e784000
    moveq r1, #0x0D                              @ Baudrate 115200
    movne r1, #0x01                              @ Baudrate 115200, div13
#if defined(CONFIG_DRAM_UART_38400)
    moveq r1, #0x27                              @ Baudrate 38400
    movne r1, #0x03                              @ Baudrate 38400 , div13
#endif
    str   r1, [r0]

    ldr   r0, =0x1e784004
    mov   r1, #0x00
    str   r1, [r0]

    ldr   r0, =0x1e78400c
    mov   r1, #0x03
    str   r1, [r0]

    ldr   r0, =0x1e784008
    mov   r1, #0x07
    str   r1, [r0]

    ldr   r0, =0x1e784000
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x52                              @ 'R'
    str   r1, [r0]
    mov   r1, #0x41                              @ 'A'
    str   r1, [r0]
    mov   r1, #0x4D                              @ 'M'
    str   r1, [r0]
    mov   r1, #0x20                              @ ' '
    str   r1, [r0]
    mov   r1, #0x49                              @ 'I'
    str   r1, [r0]
    mov   r1, #0x6E                              @ 'n'
    str   r1, [r0]
    mov   r1, #0x69                              @ 'i'
    str   r1, [r0]
    mov   r1, #0x74                              @ 't'
    str   r1, [r0]
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x52                              @ 'R'
    str   r1, [r0]
/* Debug - UART console message */

    /* Enable Timer separate clear mode */
    ldr   r0, =0x1e782038
    mov   r1, #0xAE
    str   r1, [r0]
    clear_delay_timer

    /* Delay about 100us */
    ldr   r2, =0x00000064                        @ Set Timer3 Reload = 100 us
    init_delay_timer
delay_0:
    check_delay_timer
    bne   delay_0
    clear_delay_timer
    /* end delay 100us */
/**************************************************************************************************/
/**************************************************************************************************/
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x33                              @ '3'
    str   r1, [r0]
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    mov   r1, #0x46                              @ 'F'
    str   r1, [r0]
    mov   r1, #0x50                              @ 'P'
    str   r1, [r0]
    mov   r1, #0x47                              @ 'G'
    str   r1, [r0]
    mov   r1, #0x41                              @ 'A'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e0000
    ldr   r1, =0xfc600309
    str   r1, [r0]

    /* Reset MMC */
    ldr   r1, =0x00000000
    ldr   r0, =0x1e6e0004
    ldr   r2, =0x1e6e00A0
reset_mmc:
    str   r1, [r0]
    add   r0, r0, #4
    cmp   r2, r0
    bge   reset_mmc

    ldr   r0, =0x1e6e0034                        @ disable SDRAM reset
    ldr   r1, =0x00000080
    str   r1, [r0]

    ldr   r0, =0x1e6e0004
    ldr   r1, =0x00000107
    str   r1, [r0]

    ldr   r0, =0x1e6e0008
    ldr   r1, =0x2001000F
    str   r1, [r0]

    ldr   r0, =0x1e6e0010
    ldr   r1, =0x12102725
    str   r1, [r0]

    ldr   r0, =0x1e6e0014
    ldr   r1, =0x52344420
    str   r1, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, =0x00010000
    str   r1, [r0]

    ldr   r0, =0x1e6e0038
    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e6e003C
    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0040
    ldr   r1, =0x88888888
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x00000081
    str   r1, [r0]

    ldr   r0, =0x1e6e000C
    ldr   r1, =0x00000040
    str   r1, [r0]

    /* Delay about 400us */
    ldr   r2, =0x00000190                        @ Set Timer3 Reload = 400 us
    init_delay_timer
delay_1:
    check_delay_timer
    bne   delay_1
    clear_delay_timer
    /* end delay 400us */

    ldr   r0, =0x1e6e002C
    ldr   r1, =0x04001320
    str   r1, [r0]

    ldr   r0, =0x1e6e0030
    ldr   r1, =0x00000041
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000005
    str   r1, [r0]
    ldr   r1, =0x00000007
    str   r1, [r0]
    ldr   r1, =0x00000003
    str   r1, [r0]
    ldr   r1, =0x00000001
    str   r1, [r0]

    ldr   r0, =0x1e6e002C
    ldr   r1, =0x04001220
    str   r1, [r0]

    ldr   r0, =0x1e6e000C
    ldr   r1, =0x00005C48
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000001
    str   r1, [r0]

    ldr   r0, =0x1e6e000C
    ldr   r1, =0x00005C01
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x00008081
    str   r1, [r0]

/******************************************************************************
 CBR Start
 *****************************************************************************/
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x43                                @ 'C'
    str r1, [r0]
    mov r1, #0x42                                @ 'B'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
/* Debug - UART console message */

CBR_START:
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x23                                @ '#'
    str r1, [r0]
/* Debug - UART console message */
    /* Calibrate PLL Read clock phase */
    /* Reset PLL */
    ldr   r0, =0x1e6e2050
    ldr   r1, =0x00000026
    str   r1, [r0]
    mov   r1, #0x00
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    ldr   r1, =0xAD000000
    add   r0, r0, #0x04
    str   r1, [r0]

    ldr   r0, =0x1e6e204C
check_pll_reset_done_1:
    ldr   r1, [r0]
    mov   r1, r1, lsr #24
    and   r1, r1, #0x0F
    cmp   r1, #0x00
    beq   check_pll_reset_done_1

    ldr   r0, =0x1e6e206C
    ldr   r1, =0xDA000000
    str   r1, [r0]

    ldr   r0, =0x1e6e204C
check_pll_reset_clear_1:
    ldr   r1, [r0]
    mov   r1, r1, lsr #24
    and   r1, r1, #0x0F
    cmp   r1, #0x00
    bne   check_pll_reset_clear_1

    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x01                              @ init PLL parameter index
    ldr   r1, =0x000000ff
    ldr   r0, =0x1e6e0088                        @ init dllmax,dllmin
    str   r1, [r0]

/****************************
 PLL delay margin test loop
 ***************************/
cbr_next_pll_parameter:
    ldr   r2, =0xFF                              @ parameter's max is to 0xFF
    cmp   r7, r2
    bge   CBR_END
    add   r7, r7, #0x01

    /* Phase shift */
    ldr   r0, =0x1e6e2050
    ldr   r1, =0x00000016
    str   r1, [r0]

    ldr   r0, =0x1e6e206C
    ldr   r1, =0xAD000000
    str   r1, [r0]

    ldr   r0, =0x1e6e204C
check_pll_phase_done_1:
    ldr   r1, [r0]
    mov   r1, r1, lsr #24
    and   r1, r1, #0x0F
    cmp   r1, #0x00
    beq   check_pll_phase_done_1

    ldr   r0, =0x1e6e206C
    ldr   r1, =0xDA000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 16KB
    ldr   r1, =0x00003FFF
    str   r1, [r0]

/* CBRScan() start */
    mov   r9, #0x01                              @ init test status
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr_next_test_pattern:
    mov   r10, #3                                @ set the retry loop of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr_test_single

cbr_test_pattern_end:
    cmp   r9, #0x00
    bne   cbr_test_pass_dql
    cmp   r6, #10
    bge   CBR_END
    b     cbr_next_pll_parameter                 @ CBRScan() end and test result fail, go to next step

cbr_test_pass_dql:
    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add one after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count

    ldr   r0, =0x1e6e0088
    record_pll_pass_range
    b     cbr_next_pll_parameter

/****************************
 Test fail retry loop
 ***************************/
cbr_pattern_fail_retry:

/* CBRTest() start */
cbr_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000005
    str   r1, [r0]
    ldr   r3, =0x1000
    ldr   r8, =0x10000
cbr_wait_engine_idle_0:
    subs  r8, r8, #1
    beq   cbr_test_single_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr_wait_engine_idle_0

cbr_test_single_end:
    ldr   r0, =0x1e6e0070                        @ read fail bit status
    ldr   r3, =0x2000
    ldr   r2, [r0]
    tst   r2, r3
    bne   cbr_test_fail

cbr_test_burst:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, #0xC1
    str   r2, [r0]
    ldr   r3, =0x1000
    ldr   r8, =0x10000
cbr_wait_engine_idle_1:
    subs  r8, r8, #1
    beq   cbr_test_burst_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr_wait_engine_idle_1

cbr_test_burst_end:
    ldr   r0, =0x1e6e0070                        @ read fail bit status
    ldr   r3, =0x2000
    ldr   r2, [r0]
    tst   r2, r3
    bne   cbr_test_fail
    b     cbr_next_test_burst_mode
/* CBRTest() end */

cbr_test_fail:
    subs  r10, r10, #1
    bne   cbr_pattern_fail_retry
    mov   r9, #0x00
    b     cbr_test_pattern_end                   @ CBRScan() return(0)

cbr_next_test_burst_mode:
    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr_next_test_pattern

CBR_END:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0088                        @ check PLL margin
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r5, r2, r1                             @ dllmax - dllmin
    bmi   CBR_START                              @ no valid margin found, retry again
    cmp   r5, #10                                @ (dllmax - dllmin) < 10
    blt   CBR_START                              @ no enough margin found, retry again
    add   r2, r1, r2                             @ (dllmin[1] + dllmax[1] + 1) >> 1
    add   r2, r2, #0x01
    mov   r5, r2, lsr #1
    ldr   r0, =0x1e6e008c
    str   r5, [r0]                               @ store the dll search result

    /* Reset PLL */
    ldr   r0, =0x1e6e0034
    ldr   r1, =0x00008080
    str   r1, [r0]

    ldr   r0, =0x1e6e2050
    ldr   r1, =0x00000026
    str   r1, [r0]
    ldr   r0, =0x1e6e206C
    ldr   r1, =0xAD000000
    str   r1, [r0]

    ldr   r0, =0x1e6e204C
check_pll_reset_done_2:
    ldr   r1, [r0]
    mov   r1, r1, lsr #24
    and   r1, r1, #0x0F
    cmp   r1, #0x00
    beq   check_pll_reset_done_2

    ldr   r0, =0x1e6e206C
    ldr   r1, =0xDA000000
    str   r1, [r0]

    ldr   r0, =0x1e6e204C
check_pll_reset_clear_2:
    ldr   r1, [r0]
    mov   r1, r1, lsr #24
    and   r1, r1, #0x0F
    cmp   r1, #0x00
    bne   check_pll_reset_clear_2

    /* Phase shift */
    ldr   r0, =0x1e6e2050
    ldr   r1, =0x00000016
    orr   r1, r1, r5, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e206C
    ldr   r1, =0xAD000000
    str   r1, [r0]

    ldr   r0, =0x1e6e204C
check_pll_phase_done_2:
    ldr   r1, [r0]
    mov   r1, r1, lsr #24
    and   r1, r1, #0x0F
    cmp   r1, #0x00
    beq   check_pll_phase_done_2

    ldr   r0, =0x1e6e206C
    ldr   r1, =0xDA000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x00008083
    str   r1, [r0]

/******************************************************************************
 CBR Finish
 *****************************************************************************/
/******************************************************************************
 Check DRAM Size
 1Gb : 0x80000000 ~ 0x87FFFFFF
 2Gb : 0x80000000 ~ 0x8FFFFFFF
 4Gb : 0x80000000 ~ 0x9FFFFFFF
 8Gb : 0x80000000 ~ 0xBFFFFFFF
 *****************************************************************************/
    ldr   r0, =0x1e6e0004
    ldr   r5, [r0]
    bic   r5, r5, #0x00000003                    @ record MCR04
    orr   r1, r5, #0x23
    str   r1, [r0]                               @ set to 8Gbit
    ldr   r6, =0x0020100B

check_dram_size:
    ldr   r0, =0xA0100000
    ldr   r1, =0x41424344
    str   r1, [r0]
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    str   r1, [r0]
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    str   r1, [r0]
    ldr   r0, =0x80100000
    ldr   r1, =0x1D1E1F10
    str   r1, [r0]
    ldr   r0, =0xA0100000
    ldr   r1, =0x41424344
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 8Gbit
    orreq r5, r5, #0x23
    moveq r6, r6, lsr #16
    beq   check_dram_size_end
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 4Gbit
    orreq r5, r5, #0x02
    moveq r6, r6, lsr #16
    beq   check_dram_size_end
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 2Gbit
    orreq r5, r5, #0x01
    moveq r6, r6, lsr #8
    beq   check_dram_size_end

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
    str   r1, [r0]

    /* Enable DRAM Cache */
    ldr   r0, =0x1e6e0004
    ldr   r1, [r0]
    mov   r2, #1
    orr   r2, r1, r2, lsl #12
    ldr   r3, =0x00080000
    str   r2, [r0]
dram_cache_init:
    ldr   r2, [r0]
    tst   r2, r3
    beq   dram_cache_init
    mov   r2, #1
    orr   r1, r1, r2, lsl #10
    str   r1, [r0]

    /* Enable DRAM ECC */
    orr   r1, r1, r2, lsl #7
    str   r1, [r0]
    ldr   r0, =0x1e6e0050
    ldr   r1, =0x80000000
    str   r1, [r0]
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e6e0054
    ldr   r1, =0x05FFFFFF                        @ set ECC size = 96MB
    str   r1, [r0]

    ldr   r0, =0x1e6e0074
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e6e007C
    str   r1, [r0]
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000221
    ldr   r2, =0x00001000
    str   r1, [r0]
dram_ecc_init:
    ldr   r1, [r0]
    tst   r1, r2
    beq   dram_ecc_init
    ldr   r1, =0x00000400
    str   r1, [r0]

/******************************************************************************
 Version Number
 *****************************************************************************/
    ldr   r0, =0x1e6e0004
    ldr   r1, [r0]
    mov   r2, #0x01
    orr   r1, r1, r2, lsl #20
    str   r1, [r0]

/******************************************************************************
 Calibration Code End
 ******************************************************************************/
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x6F                              @ 'o'
    str   r1, [r0]
    mov   r1, #0x6E                              @ 'n'
    str   r1, [r0]
    mov   r1, #0x65                              @ 'e'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
/* Debug - UART console message */

platform_exit:

    ldr   r0, =0x1e6e2004                        @ enable CPUI DRAM request for ARM9
    ldr   r1, [r0]
    bic   r1, r1, #0x00020000
    str   r1, [r0]

    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0xC0
    str r1, [r0]

    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    orr r1, r1, #0x20
    str r1, [r0]

    /* restore lr */
    mov   lr, r4

    /* back to arch calling code */
    mov   pc, lr

