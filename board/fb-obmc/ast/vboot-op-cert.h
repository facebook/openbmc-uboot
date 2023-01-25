/*
 * (C) Copyright 2023-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H
#define _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H

#ifndef OP_CERT_DTS

#  define VBOOT_OP_CERT_ADDR (0x10014000)
#  ifdef VBOOT_OP_CERT_SRAM_SIZE
#    define VBOOT_OP_CERT_MAX_SIZE VBOOT_OP_CERT_SRAM_SIZE
#  else
#    define VBOOT_OP_CERT_MAX_SIZE 0
#  endif
#  define VBOOT_OP_CERT_POISON (0xDEADBEEF)
#  define CERT_IAMGE_PATH ("/images/fdt@1")
#  define VBOOT_OP_CERT_PROP(pn) #pn

#  ifdef CONFIG_GIU_HW_SUPPORT
struct vbs;
void __noreturn vboot_abort_giu_reset(struct vbs *vbs, const u8 *bound_hash,
				      u32 bound_hash_len);
#  else /* define dummy functions for platform not support GIU*/
#    define vboot_setup_wp_latch(vbs, pp_timer, pp_spi_check) \
	({ printf("Not support Golden Image Upgrade.\n"); AST_FMC_ERROR;})
#    define vboot_abort_giu_reset(vbs, uboot_hash, uboot_hash_len)
#    define vboot_verify_giu_uboot_bound(vbs, uboot_hash)
#    define vboot_get_giu_mode(vbs, bsm_latched) \
	({bsm_latched = bsm_latched; GIU_NONE;})
#  endif

#else /* ======= following expose to vboot op-cert.dts only ======== */

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
#define PROP_UBOOT_HASH_LEN VBOOT_OP_CERT_PROP(UBOOT_HASH_LEN)
#endif
