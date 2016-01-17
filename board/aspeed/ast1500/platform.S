/*
 * Board specific setup info
 *
 */

#include <config.h>
#include <version.h>

/* PATTERN_TABLE, wait_engine_write_idle, wait_engine_read_idel,
   set_dram_engine, load_pattern
   load_dram_parameter and set_result are for DRAM calibration */

PATTERN_TABLE:
    .word    0xff00ff00
    .word    0xcc33cc33
    .word    0xaa55aa55
    .word    0xfffe0001
    .word    0x683501fe        @ 5
    .word    0x0f1929b0
    .word    0x2d4600fd
    .word    0x2d0b4346
    .word    0x60767f02
    .word    0x6fbe36a6        @ 10
    .word    0x3a253035
    .word    0x3019686d
    .word    0x41c6167e
    .word    0x620152bf
    .word    0x00000000        @ 15


    .macro    wait_engine_write_idle
    ldr        r0, =0x1e6e0070
wait_write_idle:
    ldr        r2, [r0]
    tst        r2, #0x40                    @ D[6] = idle bit
    beq        wait_write_idle
    .endm

    .macro    wait_engine_read_idle
    ldr        r0, =0x1e6e0070
wait_read_idle:
    ldr        r2, [r0]
    tst        r2, #0x40                    @ D[6] = idle bit
    beq        wait_read_idle
    .endm

    .macro    set_dram_engine
    ldr        r0, =0x1e6e0070
    ldr        r3, =0x00000000
    str        r3, [r0]

    mov        r3, r1, lsl #3
    orr        r3, r3, r2
    str        r3, [r0]
    .endm

    .macro    load_pattern
    ldr        r1, [r5]
    ldr        r0, =0x1e6e007c
    str        r1, [r0]
    cmp        r1, #0x00
    bne        load_end
    cmp        r6, #0x00                    @ last pattern. we will record it if
                                            @ it is the first passed parameter
    bne        load_end
    and        r8, r7, #0xff
    sub        r8, r8, #0x01                @ we add one after loop check so we
                                            @ need to decrease 1
    mov        r6, #0x01                    @ set found parameter
load_end:
    .endm

    .macro    load_dram_parameter
    ldr        r0, =0x1e6e0068
    str        r7, [r0]
    /* Delay about 400us */
    ldr r2, =0x00007000
delay3:
    nop
    nop
    subs r2, r2, #1                         /* until source end addreee [r2]    */
    bne delay3
    .endm

/* r7 = DLL max, r8 = DLL min */
    .macro    set_result
/* Below are for record min and max only, your code can skip this part */
    ldr r0, =0x1e6e0014
    mov r2, r7, lsl #8
    orr r2, r2, r8                          @ record
    str r2, [r0]
/* record min and max end */
    cmp r8, #0x00
    beq DLL_MIN_0
    sub r7, r7, r8                          @ DLL minimum != 0
    ldr r2, =0x46
    mul r1, r7, r2
    mov r1, r1, asr #7                      @ (max - min) * 70 / 128 ~=
                                            @ (max - min) * 0.546
    add r7, r1, r8
    b   set_register

DLL_MIN_0:                                  @ DLL minimum == 0
    mov r7, r7, asr #1                      @ then DLL = DLL maximum / 2
    b   set_register

set_register:
    mov r1, r7, lsl #0x08
    add r7, r7, r1                          @ D[7:0] --> D[15:8]
    mov r1, r7, lsl #0x10
    add r7, r7, r1                          @ D[15:0] --> D[31:16]
    ldr    r0, =0x1e6e0068
    str    r7, [r0]
    ldr    r0, =0x1e6e0070
    mov    r7, #0x00
    str    r7, [r0]
    .endm

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

    /* MCLK Setting for AST1500*/
    ldr r0, =0x1e6e2004
    ldr r1, [r0]
    orr r1, r1, #0x01A0
    str r1, [r0]

    ldr r0, =0x1e6e2008
    ldr r1, [r0]
    orr r1, r1, #0x0D
    str r1, [r0]

    ldr r0, =0x1e6e200C
    ldr r1, [r0]
    orr r1, r1, #0x0130
    str r1, [r0]

    /* Enable 12MHz clock output for audio, client side only */
    ldr r0, =0x1e6e2010
    ldr r1, =0x00000010
    str r1, [r0]

    ldr r0, =0x1e6e2020
#ifdef CONFIG_AST1500_CLIENT
    ldr r1, =0x00034120     //12MHz   //Host: 0x00014120
#else
    ldr r1, =0x00014120     //12MHz
#endif
    str r1, [r0]

#if CONFIG_AST1500_CLIENT //overclock
    //Set CPU clock
    ldr r0, =0x1e6e2024
    //ldr r1, =0x000441F0    //200MHZ
    //ldr r1, =0x00044120    //266MHZ
    //ldr r1, =0x000442A1    //275MHZ
    ldr r1, =0x00044140    //288MHZ
    //ldr r1, =0x000442E1    //300MHZ
    //ldr r1, =0x00044160    //312MHZ
    //ldr r1, =0x00044321    //324MHZ
    //ldr r1, =0x00044180    //336MHZ
    //ldr r1, =0x00044361    //350MHZ
    str r1, [r0]
#else
    //AHB/3
    //ldr r0, =0x1e6e2070
    //ldr r1, =0x00013002
    //str r1, [r0]

    //Set CPU clock
    ldr r0, =0x1e6e2024
    //ldr r1, =0x000441F0    //200MHZ
    //ldr r1, =0x00044120    //266MHZ
    //ldr r1, =0x000442A1    //275MHZ
    ldr r1, =0x00044140    //288MHZ
    //ldr r1, =0x000442E1    //300MHZ
    //ldr r1, =0x00044160    //312MHZ
    //ldr r1, =0x00044321    //324MHZ
    //ldr r1, =0x00044180    //336MHZ
    //ldr r1, =0x00044361    //350MHZ
    str r1, [r0]
#endif

    ldr r0, =0x1e6e202C
#ifdef CONFIG_AST1500_CLIENT
    ldr r1, =0x00000012 //For C-Media OSCCLK
#else
    ldr r1, =0x00000010 //For GPIOD7
#endif
    str r1, [r0]

    //This defualt value should be fine for PTV platform
    ldr r0, =0x1e6e2074
    ldr r1, =0x0
    str r1, [r0]

    /* DRAM Setting */
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e006C
    ldr r1, =0x00848484
    str r1, [r0]

    ldr r0, =0x1e6e0064
#ifdef CONFIG_AST1500_X32_RAM
    ldr r1, =0x00070000
#else //#ifdef CONFIG_AST1500_X32_RAM
    ldr r1, =0x00050000
#endif //#ifdef CONFIG_AST1500_X32_RAM
    str r1, [r0]

    ldr r0, =0x1e6e0004
#ifdef CONFIG_AST1500_X32_RAM
    #ifdef CONFIG_AST1500_RAMSIZE_1Gb
        ldr r1, =0x00000C8D                           // 1Gb   X32 RAM
    #elif defined (CONFIG_AST1500_RAMSIZE_512Mb)
        ldr r1, =0x00000489                           // 512Mb X32 RAM
    #elif defined (CONFIG_AST1500_RAMSIZE_256Mb)
        ldr r1, =0x00000484                           // 256Mb X32  RAM
    #endif
#else //#ifdef CONFIG_AST1500_X32_RAM
    #ifdef CONFIG_AST1500_RAMSIZE_1Gb
        ldr r1, =0x00000D89                           // 1Gb   X16 RAM
    #elif defined (CONFIG_AST1500_RAMSIZE_512Mb)
        ldr r1, =0x00000585                           // 512Mb X16 RAM
    #elif defined (CONFIG_AST1500_RAMSIZE_256Mb)
        ldr r1, =0x00000580                           // 256Mb X16 RAM
    #endif
#endif //#ifdef CONFIG_AST1500_X32_RAM
    str r1, [r0]

    ldr r0, =0x1e6e0008
    ldr r1, =0x00000000		                          /* CRT */
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x33302A27                               /* For 312MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x246E031F                               /* For 312MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00D22222                               /* For 312MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
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
#ifdef CONFIG_AST1500_X32_RAM
    ldr r1, =0x0F2AA02A
#else
    ldr r1, =0x032AA02A
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0064
#ifdef CONFIG_AST1500_X32_RAM
    ldr r1, =0x003F2005
#else
    ldr r1, =0x002D2005
#endif
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
    ldr r2, =0x00007000
delay0:
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000952
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
    ldr r1, =0x00000852                               /* For 312MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x000003C0
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
    ldr r1, =0x00005D01
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00007C03
    str r1, [r0]

/******************************************************************************
 Calibration Code Start
 ******************************************************************************/
    /* Delay about 1s */
    ldr r2, =0x04000000
delay4:
    nop
    nop
    subs r2, r2, #1                                   /* until source end addreee [r2] */
    bne delay4

/* Set Test DRAM Size and Base. Default Size = 1KB - 1 and Base = 0MB. */
    ldr r0, =0x1e6e0074
    ldr r1, =0x000003FF
    str r1, [r0]

/* Start
  r5 = pattern table index
  r6 = found passed parameter or not. 0 = not yet, 1 = found
  r7 = dram parameter index (0x1e6e0068), max is 0x40
  r8 = first passed parameter (DLL minimum), initialize to 0xff
  r9 = fail counter, initialize to 10 (fail 10 times)
  r10 = pattern fail counter, initialize to 10 (fail 10 times)
*/
retry:
    mov r6, #0x00
    mov r7, #0x00
    mov r8, #0xff
    mov r9, #10
    mov r10, #10
next_dram_parameter:
    load_dram_parameter
    ldr r2, =0x40404040                         @ parameter's max is to 0x40404040
    cmp r7, r2
    beq test_end
    ldr r2, =0x01010101
    add r7, r7, r2

fail_retry:
    adr r5, PATTERN_TABLE
next_pattern:
pattern_fail_retry:
    load_pattern
    cmp r1, #0x00                               @ the last data in pattern is 0x00
    beq next_dram_parameter                     @ pattern end so check next parameter
    mov r1, #0x00                               @ initialize loop1 index, r1 is
                                                @ loop1's index
loop1:
    mov r2, #0x01                               @ write
    set_dram_engine
    wait_engine_write_idle
    mov r2, #0x03                               @ read
    set_dram_engine
    wait_engine_read_idle

    ldr r0, =0x1e6e0070
    ldr r2, [r0]
    tst r2, #0x80                               @ check pass/fail
    beq next_test_mode                          @ pass

    subs r10, r10, #1
    bne pattern_fail_retry
    mov r10, #10

    subs r9, r9, #1
    beq fail_10_times
    b   fail_retry

fail_10_times:
    cmp r6, #0x01
    beq test_end                                @ fail and already store passed
                                                @ parameter in r8 then test end
    mov r9, #10
    b   next_dram_parameter                     @ fail but did not have any passed
                                                @ parameter so check next one

next_test_mode:
    add r1, r1, #1                              @ increase the test mode index
    cmp r1, #0x08                               @ there are 8 modes
    bne loop1

    add r5, r5, #0x04                           @ increase the test pattern index
    b   next_pattern

test_end:
    and r7, r7, #0xff
    sub r7, r7, #0x02                           @ we add one after loop check and the
                                                @ last one test is failed also
    cmp r7, r8                                  @ check DLL minimum(r8) and DLL maximum (r7)
    blt down_frequency                          @ if r7 < r8, it means that there's no
                                                @ passed parameter, we forced it to
                                                @ down_frequency.
    cmp r8, #0
    beq DLL_0
    sub r1, r7, r8
    cmp r1, #20                                 @ if (DLLmin != 0) && ((DLL maximum -
                                                @ DLL minimum) < 20), there's
                                                @ something wrong on the board
    blt down_frequency                          @ jump to down_frequency
DLL_0:
#if 0
    cmp r7, #10                                 @ if (DLLmin == 0) && ((DLL maximum -
                                                @ DLL minimum) < 10), there's something
                                                @ wrong on the board
#else
    cmp r7, #6                                  @ if (DLLmin == 0) && ((DLL maximum -
                                                @ DLL minimum) < 6), there's something
                                                @ wrong on the board
#endif
    blt down_frequency                          @ jump to down_frequency

    set_result
    b   set_scratch
/* End codes for DRAM calibration */

down_frequency:
    ldr r0, =0x1e6e0034
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000000
    str r1, [r0]

    /* AST1500 platform uses H-PLL as Memory clock. */
    //Set H-PLL(CPU) clock
    ldr r0, =0x1e6e2024
    ldr r1, =0x00044120                         //266MHZ
    str r1, [r0]

    ldr r0, =0x1e6e0064
    ldr r1, =0x00073005
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x22201825
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x254A0120
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00CE2222
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00007000
delay1:
    nop
    nop
    subs r2, r2, #1                                   /* until source end addreee [r2]    */
    bne delay1

    ldr r0, =0x1e6e0064
    ldr r1, =0x003F3005
    str r1, [r0]

    ldr r0, =0x1e6e0068
    ldr r1, =0x02020202
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00007000
delay2:
    nop
    nop
    subs r2, r2, #1                                   /* until source end addreee [r2]    */
    bne delay2

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000742
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
    ldr r1, =0x00000642
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x000003C0
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
    ldr r1, =0x00005D01
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00007C03
    str r1, [r0]

    ldr r0, =0x1e6e0120            /* for AST2000 compatible */
    ldr r1, =0x00004C81            /* 200MHz */
    str r1, [r0]

set_scratch:
    /*Set Scratch register Bit 6 after ddr initial finished */
/*
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]
*/

platform_exit:
    mov lr, r4		/* restore lr */

    /* back to arch calling code */
    mov pc, lr

