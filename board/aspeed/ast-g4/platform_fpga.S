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

    /* Check DRAM Type by H/W Trapping */
    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0x7FFFFFFF
    mov r2, r1, lsr #24
    cmp r2, #0x80
    beq ddr2_init

/* DDR3 Init*/    
ddr3_init:
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400 
delay2:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay2

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000032
    str r1, [r0]
      
    ldr r0, =0x1e6e0008
#ifdef 	CONFIG_CRT_DISPLAY
    ldr r1, =0x0090040f		/* CRT */       
#else    
    ldr r1, =0x0090040f		/* VGA */
#endif    
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x23202826
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0xA6338115
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x00000070
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0x00000000
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

    ldr r0, =0x1e6e0060
    ldr r1, =0x00000000
    str r1, [r0]
                 
    ldr r0, =0x1e6e0064
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
   
    ldr r0, =0x1e6e000c
    ldr r1, =0x00000040
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400 
delay0:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0	 
	
    ldr r0, =0x1e6e002c
    ldr r1, =0x00001520
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

    ldr r0, =0x1e6e002c
    ldr r1, =0x00001220
    str r1, [r0]
  
    ldr r0, =0x1e6e000c
    ldr r1, =0x00005a48
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x42aa5a21
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400 
delay1:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay1	 

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000003
    str r1, [r0]
    
    b	platform_exit2
/* ~DDR3 Init*/    

/* DDR2 Init*/        
ddr2_init:
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e2020
    ldr r1, =0x00000000
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400 
delay2_2:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay2_2

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000022
    str r1, [r0]
      
    ldr r0, =0x1e6e0008
    ldr r1, =0x0090040f		/* VGA */
    str r1, [r0]

    ldr r0, =0x1e6e0010
    ldr r1, =0x12101412
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0x32004008
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x00000030
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0020
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0x00000000
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

    ldr r0, =0x1e6e0060
    ldr r1, =0x00000000
    str r1, [r0]
                 
    ldr r0, =0x1e6e0064
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
delay0_2:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0_2
	
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
    ldr r1, =0x00005a08
    str r1, [r0]

    ldr r0, =0x1e6e002c
    ldr r1, =0x00000432
    str r1, [r0]
  
    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000380
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005a21
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000400 
delay1_2:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay1_2

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000003
    str r1, [r0]

    b   platform_exit2
/* ~DDR2 Init*/    
    
platform_exit2:
    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]

platform_exit:
    mov lr, r4		/* restore lr */
 
    /* back to arch calling code */    
    mov pc, lr

