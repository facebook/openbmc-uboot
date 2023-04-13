/*
 * (C) Copyright 2023-Present, META Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H
#define _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H

#include <asm/arch/giu_def.h>

#ifdef CONFIG_GIU_HW_SUPPORT
    struct vbs;
    void __noreturn vboot_abort_giu_reset(struct vbs *vbs, const u8 *bound_hash,
					  u32 bound_hash_len);
#else /* define dummy functions for platform not support GIU */
#   define vboot_setup_wp_latch(vbs, pp_timer, pp_spi_check) \
	({ printf("Not support Golden Image Upgrade.\n"); AST_FMC_ERROR;})
#   define vboot_abort_giu_reset(vbs, uboot_hash, uboot_hash_len)
#   define vboot_verify_giu_uboot_bound(vbs, uboot_hash)
#   define vboot_get_giu_mode(vbs, bsm_latched) \
	({bsm_latched = bsm_latched; GIU_NONE;})
#endif /* CONFIG_GIU_HW_SUPPORT */

#endif
