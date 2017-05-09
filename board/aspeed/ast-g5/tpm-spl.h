/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>

#include <tpm.h>

#include <asm/arch/vbs.h>

#define AST_TPM_ROLLBACK_INDEX 0x100
#define AST_TPM_PROBE_INDEX    0x101

/* Size of the rollback index space, allow for some reserved bytes. */
#define AST_TPM_ROLLBACK_SIZE  48

/* Years the next upgrade must occur, protect against timestamp DoS */
#define AST_TPM_MAX_YEARS 1

#define AST_TPM_ROLLBACK_UBOOT 1
#define AST_TPM_ROLLBACK_KERNEL 2

struct tpm_rollback_t {
  uint32_t uboot;
  uint32_t kernel;
};

/**
 * int ast_tpm_provision() - Perform a 1-time provision of the TPM.
 *
 * @return a status for vboot_status
 */
int ast_tpm_provision(struct vbs *vbs);

/**
 * int ast_tpm_nv_provision() - Preform a 1-time NV provision.
 *
 * @return a status for vboot_status
 */
int ast_tpm_nv_provision(struct vbs *vbs);

/**
 * int ast_tpm_owner_provision() - Preform a 1-time owner provision.
 *
 * @return a status for vboot_status
 */
int ast_tpm_owner_provision(struct vbs *vbs);

/**
 * int ast_tpm_try_version() - Try to verify a version is not rolling back.
 *
 * @param image set this to either ROLLBACK_UBOOT or ROLLBACK_KERNEL (see above)
 * @param version this is the version you are attempting to boot.
 * @return a status for vboot_status
 */
int ast_tpm_try_version(struct vbs *vbs, uint8_t image, uint32_t version);

/**
 * int ast_tpm_finish() - Lock the physical presence.
 *
 * @return a status for vboot_status
 */
int ast_tpm_finish(void);
