/*
 * (C) Copyright 2023-Present, META Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_ASPEED_GIU_DEF_H_
#define _ASM_ARCH_ASPEED_GIU_DEF_H_

#define VBOOT_OP_CERT_ADDR (0x10014000)
#ifdef VBOOT_OP_CERT_SRAM_SIZE
#  define VBOOT_OP_CERT_MAX_SIZE VBOOT_OP_CERT_SRAM_SIZE
#else
#  define VBOOT_OP_CERT_MAX_SIZE 0
#endif
#define VBOOT_OP_CERT_POISON (0xDEADBEEF)
#define CERT_IAMGE_PATH ("/images/fdt@1")

/* Redfine VBOOT_OP_CERT_PROP for DTS and uboot */
#ifdef OP_CERT_DTS
#   define VBOOT_OP_CERT_PROP(pn) pn
#else
#   define VBOOT_OP_CERT_PROP(pn) #pn
#endif

/* op-cert version */
#define PROP_CERT_VER VBOOT_OP_CERT_PROP(CERT_VER)
#define VBOOT_OP_CERT_VER 1
#define VBOOT_OP_CERT_UNSUPPORT_VER (VBOOT_OP_CERT_VER + 1)

/* golden image upgrade mode */
#define PROP_GIU_MODE VBOOT_OP_CERT_PROP(GIU_MODE)
#define GIU_NONE 0
#define GIU_CERT 1
#define GIU_OPEN 0xEA

/* bounding to u-boot's hash256 */
#define PROP_UBOOT_HASH VBOOT_OP_CERT_PROP(UBOOT_HASH)
#define PROP_UBOOT_HASH_LEN VBOOT_OP_CERT_PROP(UBOOT_HASH_LEN)

#endif /* _ASM_ARCH_ASPEED_GIU_DEF_H_ */
