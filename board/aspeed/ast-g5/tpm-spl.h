/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>

#include <tpm.h>

#define AST_TPM_ROLLBACK_INDEX 0x100
#define AST_TPM_ROLLBACK_SIZE  48

#define AST_TPM_ROLLBACK_UBOOT 1
#define AST_TPM_ROLLBACK_KERNEL 2

struct tpm_rollback_t {
  uint32_t uboot;
  uint32_t kernel;
};

/**
 * int ast_tpm_provision() - Perform a 1-time provision of the TPM.
 */
int ast_tpm_provision(void);

/**
 * int ast_tpm_nv_provision() - Preform a 1-time NV provision of the TPM.
 */
int ast_tpm_nv_provision(void);

/**
 * int ast_tpm_try_version() - Try to verify a version is not rolling back.
 *
 * image - Set this to either ROLLBACK_UBOOT or ROLLBACK_KERNEL (see above)
 * version - This is the version you are attempting to boot.
 */
int ast_tpm_try_version(uint8_t image, uint32_t version);

/**
 * int ast_tpm_finish() - Lock the physical presence.
 */
int ast_tpm_finish(void);
