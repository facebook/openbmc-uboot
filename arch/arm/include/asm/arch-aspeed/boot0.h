/*
 * Specialty padding for the ASPEED image
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BOOT0_H
#define __BOOT0_H

_start:
	ARM_VECTORS

#ifdef CONFIG_SPL_BUILD
	.word	0x0;	/* Key location */		
	.word	0x0;	/* start address of image */	
	.word	0x0;	/* image size */		
	.word	0x0;	/* signature address */ 	
	.word	0x0;	/* head revision ID low */ 	
	.word	0x0;	/* head revision ID high */ 	
	.word	0x0;	/* reserved */ 	                
	.word	0x0;	/* checksum */			
#endif

#endif /* __BOOT0_H */