/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>

#include <asm/arch/vbs.h>

#include "tpm-spl.h"

static inline void set_tpm_error(struct vbs *vbs, uint32_t r) {
  vbs->error_tpm = (r > 0xFF) ? 0xFF : r;
}

int ast_tpm_provision(struct vbs *vbs) {
  uint32_t result;
  struct tpm_permanent_flags pflags;
  struct tpm_volatile_flags vflags;

  /* The TPM lib may need to retry commands. */
  timer_init();

  /* The Infineon TPM needs 30ms */
  udelay(35 * 1000);

  /* The SPL should init (software-only setup), startup-clear, and test. */
  result = tpm_init();
  if (result) {
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_NOT_ENABLED;
  }

  result = tpm_startup(TPM_ST_CLEAR);
  if (result) {
    /* If this returns invalid postinit (38) then the TPM was not reset. */
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_SETUP;
  }

  tpm_self_test_full();

  /* Inspect the result of the TPM self-test. */
  uint32_t retries;
  for (retries = 0; retries < 10; retries++) {
    result = tpm_continue_self_test();
    if (result == TPM_SUCCESS) {
      break;
    }
    /* The TPM could have asked for a retry or is still self-testing. */
    if (result < TPM_NON_FATAL) {
      set_tpm_error(vbs, result);
      return VBS_ERROR_TPM_SETUP;
    }
  }

  /* Inspect the permanent and volatile flags. */
  result = tpm_get_permanent_flags(&pflags);
  result = result | tpm_get_volatile_flags(&vflags);
  if (result) {
    set_tpm_error(vbs, result);
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
      set_tpm_error(vbs, result);
      return VBS_ERROR_TPM_INVALID_PP;
    }

#ifdef CONFIG_ASPEED_TPM_LOCK
    /* After configuring, lock the physical presence mode. */
    result = tpm_tsc_physical_presence(TPM_PHYSICAL_PRESENCE_LIFETIME_LOCK);
    if (result) {
      set_tpm_error(vbs, result);
      return VBS_ERROR_TPM_NO_PPLL;
    }
#endif
  }

  /* Assert physical presence. */
  if (!vflags.physical_presence) {
    result = tpm_tsc_physical_presence(TPM_PHYSICAL_PRESENCE_PRESENT);
    if (result) {
      set_tpm_error(vbs, result);
      return VBS_ERROR_TPM_PP_FAILED;
    }
  }

  /* Set the permanent enabled flag (need physical presence). */
  if (pflags.disable) {
    result = tpm_physical_enable();
    if (result) {
      set_tpm_error(vbs, result);
      return VBS_ERROR_TPM_NOT_ENABLED;
    }
  }

  /* Set the persistent disabled flag to false. */
  if (pflags.deactivated) {
    result = tpm_physical_set_deactivated(0x0);
    if (result) {
      set_tpm_error(vbs, result);
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

int ast_tpm_owner_provision(struct vbs *vbs) {
  uint32_t result;

  /* Allow the TPM to be owned. */
  result = tpm_set_owner_install();
  if (result) {
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_FAILURE;
  }

  return VBS_SUCCESS;
}

static uint32_t ast_tpm_write(uint32_t index, const void *data, uint32_t length) {
  uint32_t result;

  result = tpm_nv_write_value(index, data, length);
  if (result == TPM_MAXNVWRITES) {
    tpm_force_clear();
    result = tpm_nv_write_value(index, data, length);
  }
  return result;
}

int ast_tpm_nv_provision(struct vbs *vbs) {
  uint32_t result;
  struct tpm_permanent_flags pflags;
  struct tpm_volatile_flags vflags;

  result = tpm_get_permanent_flags(&pflags);
  if (result) {
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_FAILURE;
  }

#ifdef CONFIG_ASPEED_TPM_LOCK
  /* Lock the NV storage, request that ACLs are applied. */
  if (!pflags.nv_locked) {
    result = tpm_nv_define_space(TPM_NV_INDEX_LOCK, 0, 0);
    if (result) {
      set_tpm_error(vbs, result);
      return VBS_ERROR_TPM_NV_LOCK_FAILED;
    }
  }
#endif

  /* Request the flag values again. */
  result = tpm_get_permanent_flags(&pflags);
  result = result | tpm_get_volatile_flags(&vflags);
  if (result) {
    set_tpm_error(vbs, result);
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

  /* ACLs will be applied to our probe index and fallback index. */
  u32 acls = TPM_NV_PER_GLOBALLOCK | TPM_NV_PER_PPWRITE;

  char blank[VBS_TPM_ROLLBACK_SIZE];
  result = tpm_nv_read_value(VBS_TPM_ROLLBACK_INDEX, &blank, sizeof(blank));
  if (result != TPM_BADINDEX && result != TPM_NOSPACE) {
    /* The index already exists. */
    return VBS_SUCCESS;
  }

  /**
   * Define area for U-Boot version and Kernel version.
   * Set that area's ACLs to TPM_NV_PER_GLOBALLOCK | TPM_NV_PER_PPWRITE.
   * Set that area's size to 32 * 4.
   */
  result = tpm_nv_define_space(VBS_TPM_ROLLBACK_INDEX, acls,
      VBS_TPM_ROLLBACK_SIZE);
  if (result == TPM_MAXNVWRITES) {
    tpm_force_clear();
    result = tpm_nv_define_space(VBS_TPM_ROLLBACK_INDEX, acls,
        VBS_TPM_ROLLBACK_SIZE);
  }
  if (result) {
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_NV_SPACE;
  }

  memset(blank, 0x0, VBS_TPM_ROLLBACK_SIZE);
  result = ast_tpm_write(VBS_TPM_ROLLBACK_INDEX, blank, sizeof(blank));

  if (result) {
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_NV_BLANK;
  }

  return VBS_SUCCESS;
}

int ast_tpm_extend(uint32_t index, unsigned char* data, uint32_t data_len) {
  unsigned char value[20];
  sha1_csum_wd(data, data_len, value, CHUNKSZ_SHA1);
  return tpm_extend(index, value, value);
}

static void ast_tpm_update_vbs_times(struct tpm_rollback_t *rb,
    struct vbs *vbs) {
  /* This copies the TPM NV data into the verified-boot status structure. */
  vbs->subordinate_last = rb->fallback_subordinate;
  vbs->subordinate_current = rb->subordinate;
  vbs->uboot_last = rb->fallback_uboot;
  vbs->uboot_current = rb->uboot;
  vbs->kernel_last = rb->fallback_kernel;
  vbs->kernel_current = rb->kernel;
}

int ast_tpm_try_version(struct vbs *vbs, uint8_t image, uint32_t version,
    bool no_fallback) {
  uint32_t result;
  uint32_t *rb_target;
  uint32_t *rb_fallback_target;
  struct tpm_rollback_t rb;

  /* Need to load the last-executed version of U-Boot. */
  rb.subordinate = -1;
  rb.uboot = -1;
  rb.kernel = -1;
  rb.fallback_subordinate = -1;
  rb.fallback_uboot = -1;
  rb.fallback_kernel = -1;
  if (image == AST_TPM_ROLLBACK_UBOOT) {
    rb_target = &rb.uboot;
    rb_fallback_target = &rb.fallback_uboot;
  } else if (image == AST_TPM_ROLLBACK_SUBORDINATE) {
    rb_target = &rb.subordinate;
    rb_fallback_target = &rb.fallback_subordinate;
  } else {
    rb_target = &rb.kernel;
    rb_fallback_target = &rb.fallback_kernel;
  }

  result = tpm_nv_read_value(VBS_TPM_ROLLBACK_INDEX, &rb, sizeof(rb));
  if (result) {
    set_tpm_error(vbs, result);
    return VBS_ERROR_TPM_NV_READ_FAILED;
  }

  ast_tpm_update_vbs_times(&rb, vbs);
  if (*rb_target == -1) {
    /**
     * Content is still -1.
     * Alternatively someone had booted a payload signed at time UINT_MAX.
     * This is huge issue and will brick the system from future updates.
     * To save the system and put security/safety pressure on the signer, this
     * causes an intentional wrap-around.
     */
    *rb_target = 0x0;
  }

  if (*rb_target > version) {
    /* This seems to be attempting a rollback. */
    if (no_fallback || *rb_fallback_target != version) {
      return VBS_ERROR_ROLLBACK_FAILED;
    }
  }

  if (*rb_target != 0 &&
      version > *rb_target + (86400 * 365 * AST_TPM_MAX_YEARS)) {
    /* Do not allow fast-forwarding beyond 5 years. */
    return VBS_ERROR_ROLLBACK_HUGE;
  }

  if (version != *rb_target) {
    /* Only update the NV space if this is a new version or fallback 'promo'. */
    if (no_fallback) {
      /* Fallback is disabled, the fallback is always the current. */
      *rb_fallback_target = version;
    } else if (version != *rb_fallback_target) {
      /* This is not fallback 'promotion', previous version is now fallback. */
      *rb_fallback_target = *rb_target;
    }

    /* Current is now the promoted fallback or a later version. */
    *rb_target = version;
    if (vbs->error_code == VBS_SUCCESS) {
      /* Only update times if verification has not yet failed. */
      result = ast_tpm_write(VBS_TPM_ROLLBACK_INDEX, &rb, sizeof(rb));
      if (result) {
        set_tpm_error(vbs, result);
        return VBS_ERROR_TPM_NV_WRITE_FAILED;
      }
    }

    /* The times have changed. */
    ast_tpm_update_vbs_times(&rb, vbs);
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
