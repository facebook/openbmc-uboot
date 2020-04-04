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
	.word	0x0	/* Key location */
	.word	0x0	/* start address of image */
	.word	0X0	/* image size */
	.word	0x0	/* signature address */
	.word	0x0	/* header revision ID low */
	.word	0x0	/* header revision ID high */
	.word	0x0	/* reserved */
	.word	0x0	/* checksum */
	.word	0x0	/* BL2 secure header */
	.word	0x0	/* public key or digest offset for BL2 */
#else /* BL2 */
	.word	0x0	/* image size */
	.word	0x0	/* signature location */
#endif

#endif /* __BOOT0_H */