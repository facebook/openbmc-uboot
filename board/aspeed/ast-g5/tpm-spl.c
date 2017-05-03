/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "tpm-spl.h"

#include <asm/arch/vbs.h>

int ast_tpm_provision(void) {
  uint32_t result;
  struct tpm_permanent_flags pflags;
  struct tpm_volatile_flags vflags;

  /* The SPL should init (software-only setup), startup-clear, and test. */
  tpm_init();
  tpm_startup(TPM_ST_CLEAR);
  tpm_self_test_full();

  /* Inspect the result of the TPM self-test. */
  result = tpm_continue_self_test();
  if (result) {
    return VBS_ERROR_TPM_SETUP;
  }

  /* Inspect the permanent and volatile flags. */
  result = tpm_get_permanent_flags(&pflags);
  result = result | tpm_get_volatile_flags(&vflags);
  if (result) {
    return VBS_ERROR_TPM_FAILURE;
  }

  if (!pflags.physical_presence_cmd_enable &&
      pflags.physical_presence_lifetime_lock) {
    /* We cannot set physical presence so the board must be doing it. */
    if (vflags.physical_presence != 0x0) {
      /* We do not have physical presence, so cannot configure anything. */
      return VBS_ERROR_TPM_NO_PP;
    }
  }

  if (!pflags.physical_presence_lifetime_lock) {
    /* We have not locked PP yet, configure for command only. */
    result = tpm_tsc_physical_presence(
        TPM_PHYSICAL_PRESENCE_HW_DISABLE | TPM_PHYSICAL_PRESENCE_CMD_ENABLE);
    if (result) {
      return VBS_ERROR_TPM_INVALID_PP;
    }

#ifdef CONFIG_ASPEED_TPM_LOCK
    /* After configuring, lock the physical presence mode. */
    result = tpm_tsc_physical_presence(TPM_PHYSICAL_PRESENCE_LIFETIME_LOCK);
    if (result) {
      return VBS_ERROR_TPM_NO_PPLL;
    }
#endif
  }

  /* Assert physical presence. */
  result = tpm_tsc_physical_presence(TPM_PHYSICAL_PRESENCE_PRESENT);
  if (result) {
    return VBS_ERROR_TPM_PP_FAILED;
  }

  /* Set the permanent enabled flag (need physical presence). */
  if (pflags.disable) {
    result = tpm_physical_enable();
    if (result) {
      return VBS_ERROR_TPM_NOT_ENABLED;
    }
  }

  /* Set the persistent disabled flag to false. */
  if (pflags.deactivated) {
    result = tpm_physical_set_deactivated(0x0);
    if (result) {
      return VBS_ERROR_TPM_ACTIVATE_FAILED;
    }

    /**
     * If the TPM is persistent deactivated it must be enabled and reset.
     * This provisioning has enabled the TPM, allow the caller to reset.
     */
    return VBS_ERROR_TPM_RESET_NEEDED;
  }

  return VBS_SUCCESS;
}

int ast_tpm_owner_provision(void) {
  uint32_t result;

  /* Allow the TPM to be owned. */
  result = tpm_set_owner_install();
  if (result) {
    return VBS_ERROR_TPM_FAILURE;
  }

  return VBS_SUCCESS;
}

int ast_tpm_nv_provision(void) {
  uint32_t result;
  struct tpm_permanent_flags pflags;
  struct tpm_volatile_flags vflags;

#ifdef CONFIG_ASPEED_TPM_LOCK
  /* Lock the NV storage, request that ACLs are applied. */
  result = tpm_nv_define_space(TPM_NV_INDEX_LOCK, 0, 0);
  if (result) {
    return VBS_ERROR_TPM_NV_LOCK_FAILED;
  }
#endif

  /* Request the flag values again. */
  result = tpm_get_permanent_flags(&pflags);
  result = result | tpm_get_volatile_flags(&vflags);
  if (result) {
    return VBS_ERROR_TPM_FAILURE;
  }

#ifdef CONFIG_ASPEED_TPM_LOCK
  if (!pflags.nv_locked) {
    return VBS_ERROR_TPM_NV_NOT_LOCKED;
  }
#endif

  if (vflags.deactivated) {
    return VBS_ERROR_TPM_NOT_ACTIVATED;
  }

  /* Probe is used to test the index existence. */
  u32 probe = 1;
  /* ACLs will be applied to our probe index and fallback index. */
  u32 acls = TPM_NV_PER_GLOBALLOCK | TPM_NV_PER_PPWRITE;

  /* Define a probe index to check for max write errors. */
  result = tpm_nv_define_space(AST_TPM_PROBE_INDEX, acls, sizeof(probe));
  /* If there was an error and it was not max writes (no-owner), fail. */
  if (result && result != TPM_MAXNVWRITES) {
    return VBS_ERROR_TPM_NV_SPACE;
  }

  /* Attempt a define-space and write-value and catch a max writes error. */
  result = tpm_nv_write_value(AST_TPM_PROBE_INDEX, &probe, sizeof(probe));
  if (result == TPM_MAXNVWRITES) {
    /* The TPM is not owned and we have exhausted the write attempts. */
    tpm_force_clear();
    result = tpm_nv_write_value(AST_TPM_PROBE_INDEX, &probe, sizeof(probe));
  }

  /* If the write-value continues to fail. */
  if (result) {
    return VBS_ERROR_TPM_NV_SPACE;
  }

  result = tpm_nv_read_value(AST_TPM_ROLLBACK_INDEX, &probe, sizeof(probe));
  if (result != TPM_BADINDEX) {
    /* The index already exists. */
    return VBS_SUCCESS;
  }

  /**
   * Define area for U-Boot version and Kernel version.
   * Set that area's ACLs to TPM_NV_PER_GLOBALLOCK | TPM_NV_PER_PPWRITE.
   * Set that area's size to 32 * 4.
   */
  result = tpm_nv_define_space(AST_TPM_ROLLBACK_INDEX, acls,
      AST_TPM_ROLLBACK_SIZE);
  if (result) {
    return VBS_ERROR_TPM_NV_SPACE;
  }

  char blank[AST_TPM_ROLLBACK_SIZE];
  memset(blank, 0x0, AST_TPM_ROLLBACK_SIZE);
  result = tpm_nv_write_value(AST_TPM_ROLLBACK_INDEX, blank, sizeof(blank));

  if (result) {
    return VBS_ERROR_TPM_NV_BLANK;
  }

  return VBS_SUCCESS;
}

int ast_tpm_try_version(uint8_t image, uint32_t version) {
  uint32_t result;
  uint32_t *last_executed_target;
  struct tpm_rollback_t last_executed;

  /* Need to load the last-executed version of U-Boot. */
  last_executed.uboot = -1;
  last_executed.kernel = -1;
  if (image == AST_TPM_ROLLBACK_UBOOT) {
    last_executed_target = &last_executed.uboot;
  } else {
    last_executed_target = &last_executed.kernel;
  }

  result = tpm_nv_read_value(AST_TPM_ROLLBACK_INDEX,
      &last_executed, sizeof(last_executed));
  if (result) {
    return VBS_ERROR_TPM_NV_READ_FAILED;
  }

  if (*last_executed_target == -1) {
    /* Content is still -1. */
    return VBS_ERROR_TPM_NV_NOTSET;
  }

  if (*last_executed_target > version) {
    /* This seems to be attempting a rollback. */
    return VBS_ERROR_ROLLBACK_FAILED;
  }

  if (*last_executed_target != 0 &&
      version > *last_executed_target + (86400 * 365 * AST_TPM_MAX_YEARS)) {
    /* Do not allow fast-forwarding beyond 10 years. */
    return VBS_ERROR_ROLLBACK_HUGE;
  }

  if (version > *last_executed_target) {
    /* Only update the NV space if this is a new version. */
    *last_executed_target = version;
    result = tpm_nv_write_value(AST_TPM_ROLLBACK_INDEX,
        &last_executed, sizeof(last_executed));
    if (result) {
      return VBS_ERROR_TPM_NV_WRITE_FAILED;
    }
  }

  return VBS_SUCCESS;
}

int ast_tpm_finish(void) {
  uint32_t result;

  result = tpm_tsc_physical_presence(
      TPM_PHYSICAL_PRESENCE_NOTPRESENT | TPM_PHYSICAL_PRESENCE_LOCK);
  if (result) {
    return VBS_ERROR_ROLLBACK_FINISH;
  }

  return VBS_SUCCESS;
}
