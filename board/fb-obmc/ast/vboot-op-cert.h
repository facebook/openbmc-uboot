/*
 * (C) Copyright 2023-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H
#define _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H

#ifndef OP_CERT_DTS

#  ifdef CONFIG_GIU_HW_SUPPORT
#    define VBOOT_OP_CERT_ADDR (0x10014000)
#  endif
#  define CERT_IAMGE_PATH ("/images/fdt@1")
#  define VBOOT_OP_CERT_PROP(pn) #pn

#else
/* ======= following expose to vboot op-cert.dts only ======== */

#  define VBOOT_OP_CERT_PROP(pn) pn

#endif /* end of VBOOT_OP_CERT*/

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

#endif
