/*
 * Board specific setup info
 *
 */

#include <config.h>
#include <version.h>

.globl lowlevel_init
lowlevel_init:

init_dram:
    mov r4, lr		/* save lr */

    /*Set Scratch register Bit 7 before initialize*/
    /* *(scu1+ 0) = 0xa8 */
    ldr r0, =0x1e6e2000
    ldr r1, =0x1688a8a8
    str r1, [r0]

    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

    /*Check Stratch Register Bit 6*/
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

    /* DRAM Setting */
#ifdef 	CONFIG_AST2200_FPGA
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000484
    str r1, [r0]

    ldr r0, =0x1e6e0008
    ldr r1, =0x0011030f		/* VGA */
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x01001726
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x05220103
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00aa0000
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0x01001726
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, =0x05220103
    str r1, [r0]

    ldr r0, =0x1e6e0024
    ldr r1, =0x00aa0000
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0xffffff02
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0x66666666
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x66666666
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x66666660
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x66666666
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0058
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e005c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0060
    ldr r1, =0x0f000000
    str r1, [r0]

    ldr r0, =0x1e6e0064
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0068
    ldr r1, =0x00000000
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

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400
delay0:
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000532
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x0000b408
    str r1, [r0]

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000432
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000384
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x0000a008
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000003
    str r1, [r0]
#else		//Real Chip
    ldr r0, =0x1e6e2020
    ldr r1, =0x00004120		//266MHz
    str r1, [r0]

    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e006C
    ldr r1, =0x00909090
    str r1, [r0]

    ldr r0, =0x1e6e0064
    ldr r1, =0x00070000
    str r1, [r0]

    ldr r0, =0x1e6e2070		//Set VGA size trapping to SDC
    ldr r1, [r0]
    ldr r2, =0x0000000C
    and	r1, r1, r2
    mov	r2, r1, lsl #2

#ifdef CONFIG_DDRII1G_266
    ldr r0, =0x1e6e0004
    ldr r1, =0x00000c8d
    orr r1, r1, r2
    str r1, [r0]
#else  /* DDR2 512M 266 */
    ldr r0, =0x1e6e0004
    ldr r1, =0x00000489
    orr r1, r1, r2
    str r1, [r0]
#endif

    ldr r0, =0x1e6e0008
#ifdef	CONFIG_AST2200DFB
    ldr r1, =0x00000000		/* CRT */
#else
    ldr r1, =0x0011030F		/* VGA */
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x32302926
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x274c0122
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00ce2222
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0x01001523
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, =0x1024010d
    str r1, [r0]

    ldr r0, =0x1e6e0024
    ldr r1, =0x00cb2522
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0xffffff82
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0x00F00000
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0058
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e005c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0060
    ldr r1, =0x0F2AA02A
    str r1, [r0]

    ldr r0, =0x1e6e0064
    ldr r1, =0x003F3005
    str r1, [r0]

    ldr r0, =0x1e6e0068
    ldr r1, =0x02020202
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

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400
delay0:
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000942
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005A08
    str r1, [r0]

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000842
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x000003c0
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005A21
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00007c03
    str r1, [r0]

    ldr r0, =0x1e6e0120			/* for AST2000 compatible */
    ldr r1, =0x00005061			/* 266MHz */
    str r1, [r0]

/******************************************************************************
 Calibration Code Start, Search the DLLI margin
 Release date: 2011.03.01
  r0 = free
  r1 = free
  r2 = free
  r3 = free
  r4 = record the return pc value, do not use
* r5 = pattern table index
* r6 = pass count
* r7 = dram DLL parameter index (0x1e6e0068), max is 0x4C
  r8 = DLL pass range
  r9 = calibration result accumulator
* r10 = pattern fail counter, initialize to 5 (fail 5 times)
  r11 = free
 *****************************************************************************/
    b	CBR_START
.LTORG

PATTERN_TABLE:
    .word   0xFF00FF00
    .word   0xCC33CC33
    .word   0xAA55AA55
    .word   0xFFFE0001
    .word   0x683501FE       @ 5
    .word   0x0F1929B0
    .word   0x2D0B4346
    .word   0x60767F02
    .word   0x00000000       @ End

CBR_START:
    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 16KB
    ldr   r1, =0x00003FFF
    str   r1, [r0]

    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL parameter index
    mov   r8, #0xFF                              @ init DLL pass range MAX:MIN

/****************************
 DLLI delay margin test loop
 ***************************/
cbr_next_dll_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL parameter
    str   r7, [r0]
    ldr   r2, =0x4C4C4C4C                        @ parameter's max is to 0x4C4C4C4C
    cmp   r7, r2
    beq   CBR_END
    ldr   r2, =0x01010101
    add   r7, r7, r2

/* CBRScan() start */
    mov   r9, #0x01                              @ init test status
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr_next_test_pattern:
    mov   r10, #5                                @ set the retry loop of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr_test_single

cbr_test_pattern_end:
    cmp   r9, #0x00
    bne   cbr_test_pass
    cmp   r6, #10
    bge   CBR_END
    b     cbr_next_dll_parameter                 @ CBRScan() end and test result fail, go to next step

cbr_test_pass:
    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add one after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count
    bic   r2, r8, #0xFFFFFF00
    cmp   r2, r3                                 @ record min
    bicgt r8, r8, #0xFFFF00FF
    orrgt r8, r8, r3
    bic   r2, r8, #0xFFFF00FF
    cmp   r3, r2, lsr #8                         @ record max
    bicgt r8, r8, #0xFFFFFF00
    orrgt r8, r8, r3, lsl #8
    b     cbr_next_dll_parameter

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
cbr_wait_engine_idle_0:
    ldr   r2, [r0]
    tst   r2, #0x40                              @ D[6] = idle bit
    beq   cbr_wait_engine_idle_0

    ldr   r2, [r0]
    tst   r2, #0x80                              @ D[7] = fail bit
    beq   cbr_test_burst
    b     cbr_test_fail

cbr_test_burst:
    mov   r1, #0x00                              @ initialize loop index, r1 is loop's index
cbr_test_burst_loop:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, r1, lsl #3
    orr   r2, r2, #0x01                          @ write command = 0x01 | (datagen << 3)
    str   r2, [r0]

cbr_wait_engine_idle_1:
    ldr   r2, [r0]
    tst   r2, #0x40                              @ D[6] = idle bit
    beq   cbr_wait_engine_idle_1

    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, r1, lsl #3
    orr   r2, r2, #0x03                          @ read command = 0x03 | (datagen << 3)
    str   r2, [r0]

cbr_wait_engine_idle_2:
    ldr   r2, [r0]
    tst   r2, #0x40                              @ D[6] = idle bit
    beq   cbr_wait_engine_idle_2

    ldr   r2, [r0]
    tst   r2, #0x80                              @ D[7] = fail bit
    beq   cbr_next_test_burst_mode
/* CBRTest() end */

cbr_test_fail:
    subs  r10, r10, #1
    bne   cbr_pattern_fail_retry
    mov   r9, #0x00
    b     cbr_test_pattern_end                   @ CBRScan() return(0)

cbr_next_test_burst_mode:
    add   r1, r1, #1                             @ increase the test mode index
    cmp   r1, #0x08                              @ there are 8 modes
    bne   cbr_test_burst_loop

    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr_next_test_pattern

CBR_END:
    ldr   r0, =0x1e6e0014
    str   r8, [r0]
    mov   r2, r8, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r8, #0xFF                          @ get dllmin
    subs  r5, r2, r1                             @ dllmax - dllmin
    bmi   CBR_START                              @ no valid margin found, retry again
    cmp   r5, #10                                @ (dllmax - dllmin) < 10
    blt   CBR_START                              @ no enough margin found, retry again
    mov   r2, #7
    mul   r2, r5, r2
    add   r1, r1, r2, lsr #4                     @ dllmin[1] + ((dllmax[1] - dllmin[1]) * 7) >> 4
    mov   r2, r1, lsl #8
    orr   r1, r1, r2
    mov   r2, r1, lsl #16
    orr   r1, r1, r2
    ldr   r0, =0x1e6e0068                        @ save the result dll value
    str   r1, [r0]

    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
/******************************************************************************
 Calibration Code End
 ******************************************************************************/
#endif

    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]

platform_exit:
    mov lr, r4		/* restore lr */

    /* back to arch calling code */
    mov pc, lr

