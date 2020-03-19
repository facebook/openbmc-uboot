/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <common.h>
#include <spl.h>
#include <asm/spl.h>
#include <malloc.h>
#include <u-boot/crc.h>

#include <environment.h>

#include <image.h>
#include <linux/libfdt.h>

#include <asm/arch/ast-sdk/aspeed.h>
#include <asm/arch/ast-sdk/ast_scu.h>
#include <asm/arch/ast-sdk/vbs.h>
#include <asm/io.h>

#include "flash-spl.h"
#include "tpm-spl.h"
#include "util.h"

/* Size of RW environment parsable by ROM. */
#define AST_ROM_ENV_MAX     0x200

/* Max size of U-Boot FIT */
#define AST_MAX_UBOOT_FIT   0x4000

#define vboot_recovery(v, t , c) do {			\
	printf("%s %d\n", __func__, __LINE__);		\
	real_vboot_recovery((v), (t), (c));		\
} while(0)

#define vboot_enforce(v, t , c) do {			\
	printf("%s %d\n", __func__, __LINE__);		\
	real_vboot_enforce((v), (t), (c));		\
} while(0)


DECLARE_GLOBAL_DATA_PTR;

/**
 * vboot_getenv_yesno() - Turn an environment variable into a boolean
 *
 * @param var the environment variable name
 * @return 0 for false, 1 for true
 */
static u8 vboot_getenv_yesno(const char* var) {
  const char* n = 0;
  const char* v = 0;
  int size = strlen(var);

  /* Use a RW environment. */
  volatile env_t *env = (volatile env_t*)CONFIG_ENV_ADDR;

  int offset;
  char last = 0;
  char current = env->data[0];
  for (offset = 1; offset < AST_ROM_ENV_MAX; ++offset) {
    last = current;
    current = env->data[offset];
    if (last == 0 && current == 0) {
      /* This is the end of the environment. */
      return 0;
    }

    if (n == 0) {
      /* This is the first variable. */
      n = (const char*)&env->data[offset - 1];
    } else if (current == 0) {
      /* This is the end of a variable. */
      v = 0;
      n = (const char*)&env->data[offset + 1];
    } else if (v == 0 && current == '=') {
      v = (const char*)&env->data[offset + 1];
      if (strncmp(n, var, size) == 0) {
        if (*v == '1' || *v == 'y' || *v == 't' || *v == 'T' || *v == 'Y') {
          return 1;
        }
      }
    }
  }
  return 0;
}

/**
 * vboot_store() - Store the VBS state into SRAM
 *
 * @param pointer to vbs state
 */
void vboot_store(struct vbs *vbs)
{
  u32 rom_handoff;

  rom_handoff = vbs->rom_handoff;
  vbs->crc = 0;
  vbs->rom_handoff = 0x0;
  vbs->crc = crc16_ccitt(0, (uchar*)vbs, sizeof(struct vbs));
  vbs->rom_handoff = rom_handoff;
  memcpy((void*)AST_SRAM_VBS_BASE, vbs, sizeof(struct vbs));
}

/**
 * vboot_jump() - Jump, or set the PC, to an address.
 *
 * @param to an address
 */
void __noreturn vboot_jump(volatile void* to, struct vbs* vbs)
{
  vboot_store(vbs);

  if (to == 0x0) {
    debug("Resetting CPU0\n");
    reset_cpu(0);
  }

  debug("Blindly jumping to 0x%08x\n", (u32)to);
  typedef void __noreturn (*image_entry_noargs_t)(void);
  image_entry_noargs_t image_entry = (image_entry_noargs_t)to;
  image_entry();
}

/**
 * vboot_status() - Set the vboot status on the first error.
 *
 * @param t the error type (category)
 * @param c the error code
 */
static void vboot_status(struct vbs *vbs, u8 t, u8 c) {
  /* Store an error condition in the VBS structure for reporting/testing. */
  if (t != VBS_SUCCESS && vbs->error_code == VBS_SUCCESS) {
    /* Only store the initial error code and type. */
    vbs->error_code = c;
    vbs->error_type = t;
  }
}

/**
 * vboot_recovery() - Boot the Recovery U-Boot.
 */
static void real_vboot_recovery(struct vbs *vbs, u8 t, u8 c) {
  vboot_status(vbs, t, c);

  /* Overwrite all of the verified boot flags we've defined. */
  vbs->recovery_boot = 1;
  vbs->recovery_retries += 1;
  vbs->rom_handoff = 0x0;

  /* Jump to the well-known location of the Recovery U-Boot. */
  printf("Booting recovery U-Boot.\n");
  vboot_jump((volatile void*)CONFIG_SYS_RECOVERY_BASE, vbs);
}

/**
 * vboot_getenv_yesno() - Turn an environment variable into a boolean
 *
 * @param var the environment variable name
 * @return 0 for false, 1 for true
 */
static void real_vboot_enforce(struct vbs *vbs, u8 t, u8 c) {
  vboot_status(vbs, t, c);

  if (vbs->hardware_enforce == 1 || vbs->software_enforce == 1) {
    vboot_recovery(vbs, 0, 0);
  }
}

void vboot_check_fit(void* fit, struct vbs *vbs,
                     int* uboot, int* config,
                     u32* uboot_position, u32* uboot_size) {
  u32 fit_size;
  int images_path;
  int configs_path;

  if (fdt_magic(fit) != FDT_MAGIC) {
    /* FIT loading is not available or this U-Boot is not a FIT. */
    /* This will bypass signature checking */
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_MAGIC);
  }

  fit_size = fdt_totalsize(fit);
  fit_size = (fit_size + 3) & ~3;
  if (fit_size > AST_MAX_UBOOT_FIT) {
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_INVALID_SIZE);
  }

  /* Node path to images */
  images_path = fdt_path_offset(fit, FIT_IMAGES_PATH);
  if (images_path < 0) {
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_IMAGES);
  }

  /* Be simple, select the first image. */
  *uboot = fdt_first_subnode(fit, images_path);
  if (*uboot < 0) {
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_FW);
  }

  /* Node path to configurations */
  configs_path = fdt_path_offset(fit, FIT_CONFS_PATH);
  if (configs_path < 0) {
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_CONFIG);
  }

  /* We only support a single configuration for U-Boot. */
  *config = fdt_first_subnode(fit, configs_path);
  if (*config < 0) {
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_CONFIG);
  }

  /* Get its information and set up the spl_image structure */
  *uboot_position = fdt_getprop_u32(fit, *uboot, "data-position");
  *uboot_size = fdt_getprop_u32(fit, *uboot, "data-size");

  /*
   * Check the sanity of U-Boot position and size.
   * If there is a problem force recovery.
   */
  if (*uboot_size == 0 || *uboot_position == 0 ||
      *uboot_size > 0xE0000 || *uboot_position > 0xE0000) {
    printf("Cannot find U-Boot firmware.\n");
    vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_FW);
  }

#ifdef CONFIG_ASPEED_TPM
  /* Measure the U-Boot FIT into PCR1 */
  ast_tpm_extend(AST_TPM_PCR_FIT, (unsigned char*)fit, AST_MAX_UBOOT_FIT);
#endif
}

void vboot_rollback_protection(const void* fit, uint8_t image, struct vbs *vbs) {
  /* Only attempt to update timestamps if the TPM was provisioned. */
  int root = fdt_path_offset(fit, "/");
  int timestamp = fdt_getprop_u32(fit, root, "timestamp");
  bool no_fallback = (fdt_getprop(fit, root, "no-fallback", NULL) != NULL);
  if (root < 0 || timestamp <= 0) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_RB, VBS_ERROR_ROLLBACK_MISSING);
  }

  int tpm_status = ast_tpm_try_version(vbs, image, timestamp, no_fallback);
  if (tpm_status != VBS_SUCCESS) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_RB, tpm_status);
  }
}

void vboot_verify_subordinate(void* fit, struct vbs *vbs) {
  /* Node path to keys */
  int keys_path = fdt_path_offset(fit, VBS_KEYS_PATH);
  if (keys_path < 0) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEYS);
    return;
  }

  /* After the first image (uboot) expect to find the subordinate store. */
  int subordinate = fdt_first_subnode(fit, keys_path);
  if (subordinate < 0) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEYS);
    return;
  }

  /* Access the data and data-size to call image verify directly. */
  int subordinate_size = 0;
  const char* data = fdt_getprop(fit, subordinate, "data", &subordinate_size);
  if (data != 0 && subordinate_size > 0) {
    /* This can return success if none of the keys were attempted. */
    int verified = 0;
    if (fit_image_verify_required_sigs(fit, subordinate, data,
               subordinate_size, (const char*)vbs->rom_keys, &verified)) {
      printf("Unable to verify required subordinate certificate store.\n");
      vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_KEYS_INVALID);
    }

    /* Check that at least 1 subordinate store was verified. */
    if (verified == 0) {
#ifdef CONFIG_ASPEED_TPM
      vboot_rollback_protection(data, AST_TPM_ROLLBACK_SUBORDINATE, vbs);
#endif

      /*
       * Change the certificate store to the subordinate after it is verified.
       * This means the first image, 'firmware' is signed with a key that is NOT
       * in ROM but rather signed by the verified subordinate key.
       */
      vbs->subordinate_keys = (u32)data;
    } else {
      printf("Subordinate certificate store was not verified.\n");
      vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_KEYS_UNVERIFIED);
    }
  } else {
    vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_KEYS);
  }
}

void vboot_verify_uboot(void* fit, struct vbs *vbs, void* load,
                        int config, int uboot, int uboot_size) {
  int verified = 0;
  const char* sig_store = (const char*)vbs->rom_keys;
  if (vbs->subordinate_keys != 0x0) {
    // The subordinate keys values will be non-0 when verified.
    sig_store = (const char*)vbs->subordinate_keys;
  }

  if (fit_config_verify_required_sigs(fit, config, sig_store, &verified)) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_FW_INVALID);
  }

  if (verified != 0) {
    /* When verified is 0, then an image was verified. */
    printf("U-Boot configuration was not verified.\n");
    debug("Check that the 'required' field for each key- is set to 'conf'.\n");
    debug("Check the board configuration for supported hash algorithms.\n");
    vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_FW_UNVERIFIED);
    return;
  }

  /* Now verify the hash of the first image. */
  char *error;
  uint8_t hash_value[FIT_MAX_HASH_LEN];
  int hash = fdt_subnode_offset(fit, uboot, FIT_HASH_NODENAME);
  if (fit_image_check_hash(fit, hash, load, uboot_size, hash_value, &error)) {
    printf("\nU-Boot was not verified.\n");
    vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_FW_UNVERIFIED);
    return;
  }

  printf("\nU-Boot verified.\n");
#ifdef CONFIG_ASPEED_TPM
  /* Hash the SHA256 hash of U-boot firmware, avoid recalculating the hash. */
  ast_tpm_extend(AST_TPM_PCR_UBOOT, hash_value, FIT_MAX_HASH_LEN);
  /* Hash the contents of the environment before passing execution to U-Boot. */
  ast_tpm_extend(AST_TPM_PCR_ENV, (unsigned char*)CONFIG_ENV_ADDR,
      CONFIG_ENV_SIZE);
#endif
}

void vboot_reset(struct vbs *vbs) {
  volatile struct vbs* current = (volatile struct vbs*)AST_SRAM_VBS_BASE;

  memset((void*)vbs, 0, sizeof(struct vbs));
  vbs->rom_exec_address = current->uboot_exec_address;
  vbs->recovery_retries = current->recovery_retries;
  vbs->rom_handoff = current->rom_handoff;
  if (vbs->recovery_retries >= 25) {
    /* Retries has hit an impossible upper bound, reset to 0. */
    vbs->recovery_retries = 0;
  }

  if (vbs->rom_handoff == VBS_HANDOFF) {
    printf("U-Boot failed to execute.\n");
    vboot_recovery(vbs, VBS_ERROR_TYPE_SPI, VBS_ERROR_EXECUTE_FAILURE);
  }

  /* Keep track of requested and valid H/W enforce enable scenario. */
  bool should_lock = false;
  /* Verified boot is not possible if the SPL does not include a KEK. */
  const void *rom_fdt = (const void*)gd_fdt_blob();
  vbs->rom_keys = (u32)rom_fdt;
  if (rom_fdt == 0x0) {
    /* It is possible the spl_init method did not find a fdt. */
    printf("No signature store (KEK) was included in the SPL.\n");
    vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEK);
  } else {
    /* Check if there is a signature subnode and the /hwlock path exists. */
    if (fdt_subnode_offset(rom_fdt, 0, FIT_SIG_NODENAME) >= 0 &&
        fdt_getprop_u32(rom_fdt, 0, "hwlock") == 1) {
      should_lock = true;
    }
  }

  /* Reset FMC SPI PROMs and check WP# for FMC SPI CS0. */
  int spi_status = ast_fmc_spi_check(should_lock);
  /* The presence of WP# on FMC SPI CS0 determines hardware enforcement. */
  vbs->hardware_enforce = (spi_status == AST_FMC_WP_ON) ? 1 : 0;
  if (spi_status == AST_FMC_ERROR) {
    /* The QEMU models will always return SPI PROM errors. */
#ifndef CONFIG_DEBUG_QEMU
    vboot_recovery(vbs, VBS_ERROR_TYPE_SPI, VBS_ERROR_SPI_PROM);
#endif
  }

  /* Set a handoff and expect U-Boot to clear indicating a clean boot. */
  vbs->recovery_retries = 0;
  vbs->rom_handoff = VBS_HANDOFF;
  /* Store to SRAM in case watchdog kicks before we jump to u-boot */
  vboot_store(vbs);

#ifdef CONFIG_ASPEED_TPM
  int tpm_status = ast_tpm_provision(vbs);
  if (tpm_status == VBS_ERROR_TPM_SETUP) {
    /* The TPM was not reset correctly */
    debug("TPM was not reset correctly, cur->rom_handoff=%d\n", current->rom_handoff);
    if (current->rom_handoff != VBS_HANDOFF_TPM_SETUP) {
      vbs->rom_handoff = VBS_HANDOFF_TPM_SETUP;
      vboot_jump(0x0, vbs);
    }
  }

  if (tpm_status == VBS_ERROR_TPM_RESET_NEEDED) {
    if (current->rom_handoff == VBS_HANDOFF_TPM_RST) {
      /* The TPM needed a reset before, and needs another, this is a problem. */
      printf("TPM was deactivated and remains so after a reset.\n");
      vboot_enforce(vbs, VBS_ERROR_TYPE_TPM, VBS_ERROR_TPM_RESET_NEEDED);
    } else {
      printf("TPM was deactivated and needs a reset.\n");
      vbs->rom_handoff = VBS_HANDOFF_TPM_RST;
      vboot_jump(0x0, vbs);
    }
  } else if (tpm_status != VBS_SUCCESS) {
    /* The TPM could not be provisioned. */
    vboot_enforce(vbs, VBS_ERROR_TYPE_TPM, tpm_status);
    return;
  }

  tpm_status = ast_tpm_owner_provision(vbs);
  if (tpm_status != VBS_SUCCESS) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_TPM, tpm_status);
    return;
  }

  /* Only attempt to provision the NV space if the TPM was provisioned. */
  tpm_status = ast_tpm_nv_provision(vbs);
  if (tpm_status != VBS_SUCCESS) {
    vboot_enforce(vbs, VBS_ERROR_TYPE_NV, tpm_status);
    return;
  }

  /* Measure the SPL into PCR0 */
  ast_tpm_extend(AST_TPM_PCR_SPL, (unsigned char*)0x0,
      CONFIG_SPL_MAX_FOOTPRINT);
#endif
}

void vboot_load_fit(volatile void* from) {
  void* fit = (void*)from;

  /* Set the VBS structure to the expected location in SRAM. */
  struct vbs *vbs = (struct vbs*)malloc(sizeof(struct vbs));

  /* The AST comes out of reset so we check the previous state and SPI PROMs. */
  vboot_reset(vbs);

  /* The offset into the FIT containing signed configuration. */
  int config;
  /* The offset into the FIT containing U-Boot information. */
  int uboot;
  /* The offset after the FIT containing U-Boot. */
  u32 uboot_position;
  /* The size of U-Boot content following the FIT */
  u32 uboot_size;

  /* Check the sanity of the FIT. */
  vboot_check_fit(fit, vbs, &uboot, &config, &uboot_position, &uboot_size);

  /* Check for software enforcement and forced recovery */
  vbs->software_enforce = vboot_getenv_yesno("verify");
  vbs->force_recovery = vboot_getenv_yesno("force_recovery");
  if (vbs->force_recovery == 1) {
    vboot_recovery(vbs, VBS_ERROR_TYPE_FORCE, VBS_ERROR_FORCE_RECOVERY);
  }

  if (fdt_subnode_offset((const void*)vbs->rom_keys, 0, FIT_SIG_NODENAME) >= 0) {
    /* If the SPL contains a KEK then verification is enforced. */
    vbs->software_enforce = 1;
  }

  /* Verify subordinate keys kept in the FIT */
  vboot_verify_subordinate(fit, vbs);

  /* If verified boot is successful the next load is U-Boot. */
  void* load = (void*)((u32)from + uboot_position);

  /* Finally verify U-Boot using the subordinate store if verified. */
  vboot_verify_uboot(fit, vbs, load, config, uboot, uboot_size);

#ifdef CONFIG_ASPEED_TPM
  vboot_rollback_protection(fit, AST_TPM_ROLLBACK_UBOOT, vbs);
#endif

  vboot_jump((volatile void*)load, vbs);
}

void board_init_f(ulong bootflag)
{
  /* Must set up console for printing/logging. */
  preloader_console_init();
  /* Must set up global data pointers for local device tree. */
  spl_init();

  gd->malloc_base = CONFIG_SYS_SPL_MALLOC_START;
  gd->malloc_limit = CONFIG_SYS_SPL_MALLOC_SIZE;

//#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
//  ast_wdt_reset(120 * AST_WDT_CLK, 0x3 | 0x08);
//#endif
	watchdog_init(120);
  /*
   * This will never be relocated, so jump directly to the U-boot.
   */
  vboot_load_fit((volatile void*)CONFIG_SYS_SPL_FIT_BASE);
  hang();
}

u32 spl_boot_device() {
  /* Include this NOP symbol to use the SPL_FRAMEWORK APIs. */
  return BOOT_DEVICE_NONE;
}

void spl_display_print() {
  /* Nothing */
}
