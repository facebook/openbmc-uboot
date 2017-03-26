/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/spl.h>
#include <malloc.h>

#include <environment.h>

#include <image.h>
#include <libfdt.h>

#include <asm/arch/aspeed.h>
#include <asm/arch/ast_scu.h>
#include <asm/arch/vbs.h>
#include <asm/io.h>

#include "flash-spl.h"

/* Size of RW environment parsable by ROM. */
#define AST_ROM_ENV_MAX     0x200

/* Max size of U-Boot FIT */
#define AST_MAX_UBOOT_FIT   0x4000

DECLARE_GLOBAL_DATA_PTR;

static void vbs_status(u8 t, u8 c) {
  /* Store an error condition in the VBS structure for reporting/testing. */
  struct vbs *vbs = (struct vbs*)AST_SRAM_VBS_BASE;
  if (vbs->error_code == VBS_SUCCESS) {
    /* Only store the initial error code and type. */
    vbs->error_code = c;
    vbs->error_type = t;
  }
}

static u32 fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
  const u32 *cell;
  int len;

  cell = fdt_getprop(fdt, node, prop, &len);
  if (len != sizeof(*cell)) {
    return -1U;
  }
  return fdt32_to_cpu(*cell);
}

void __noreturn jump(volatile void* to)
{
  debug("Blindly jumping to 0x%08x\n", (u32)to);
  typedef void __noreturn (*image_entry_noargs_t)(void);

  image_entry_noargs_t image_entry = (image_entry_noargs_t)to;
  image_entry();
}

u32 spl_boot_device() {
  /* Include this NOP symbol to use the SPL_FRAMEWORK APIs. */
  return BOOT_DEVICE_NONE;
}

void spl_display_print() {
  /* Nothing */
}

void spl_recovery(void) {
  /* Overwirte all of the verified boot flags we've defined. */
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  vbs->recovery_boot = 1;
  vbs->recovery_retries += 1;
  vbs->rom_handoff = 0x0;

  /* Jump to the well-known location of the Recovery U-Boot. */
  printf("Booting recovery U-Boot.\n");
  jump((volatile void*)CONFIG_SYS_RECOVERY_BASE);
}

u8 spl_getenv_yesno(const char* var) {
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

u8 verify_required(u8* notified) {
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  if (vbs->hardware_enforce == 1) {
    return 1;
  } else if (vbs->software_enforce == 1) {
    return 1;
  }

  if (*notified == 0) {
    *notified = 1;
    debug("OpenBMC verified-boot is not enforcing in hardware or software.\n");
    debug("Allowing execution to proceed into an unverified state.\n");
  }
  return 0;
}

#define CHECK_AND_RECOVER(n) \
  if (verify_required(n)) { spl_recovery(); }

void load_fit(volatile void* from) {
  /* Set the VBS structure to the expected location in SRAM */
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  u32 rom_address = vbs->uboot_exec_address;
  u32 rom_handoff = vbs->rom_handoff;
  u8 recovery_retries = vbs->recovery_retries;
  if (recovery_retries >= 25) {
    /* Retries has hit an impossible upper bound, reset to 0. */
    recovery_retries = 0;
  }

  /* Reset all data, then restore selected variables. */
  memset((void*)vbs, 0, sizeof(struct vbs));
  vbs->rom_exec_address = rom_address;
  vbs->recovery_retries = recovery_retries;
  vbs->hardware_enforce = 0;

  if (rom_handoff == 0xADEFAD8B) {
    printf("U-Boot failed to execute.\n");
    vbs_status(VBS_ERROR_TYPE_SPI, VBS_ERROR_EXECUTE_FAILURE);
    spl_recovery();
  }

  /* Reset the target RW flash chip. */
  if (!ast_fmc_spi_cs1_reset()) {
    debug("%s: Cannot reset FMC/CS1 (SPI0.1) state", __func__);
    vbs_status(VBS_ERROR_TYPE_SPI, VBS_ERROR_SPI_PROM);
    spl_recovery();
  }

  void* fit = (void*)from;
  if (fdt_magic(fit) != FDT_MAGIC) {
    /* FIT loading is not available or this U-Boot is not a FIT. */
    /* This will bypass signature checking */
    debug("%s: Cannot find FIT image header: 0x%08x\n", __func__, (u32)from);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_MAGIC);
    spl_recovery();
  }

  u32 size = fdt_totalsize(fit);
  size = (size + 3) & ~3;
  u32 base_offset = (size + 3) & ~3;
  debug("%s: Parsed FIT: size=%x base_offset=%x\n", __func__,
        size, base_offset);
  if (size > AST_MAX_UBOOT_FIT) {
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_INVALID_SIZE);
    spl_recovery();
  }

  /* Node path to images */
  int images = fdt_path_offset(fit, FIT_IMAGES_PATH);
  if (images < 0) {
    debug("%s: Cannot find /images node: %d\n", __func__, images);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_IMAGES);
    spl_recovery();
  }

  /* Be simple, select the first image. */
  int uboot_node = fdt_first_subnode(fit, images);
  if (uboot_node < 0) {
    debug("%s: Cannot find first image (u-boot) node: %d\n", __func__,
      uboot_node);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_FW);
    spl_recovery();
  }

  /* Node path to configurations */
  int configs = fdt_path_offset(fit, FIT_CONFS_PATH);
  if (configs < 0) {
    debug("%s: Cannot find /configurations node: %d\n", __func__, configs);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_CONFIG);
    spl_recovery();
  }

  /* We only support a single configuration for U-Boot. */
  int config_node = fdt_first_subnode(fit, configs);
  if (config_node < 0) {
    debug("%s: Cannot find first configuration (u-boot) node: %d\n", __func__,
      config_node);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_CONFIG);
    spl_recovery();
  }

  /* Get its information and set up the spl_image structure */
  u32 uboot_position = fdt_getprop_u32(fit, uboot_node, "data-position");
  u32 uboot_size = fdt_getprop_u32(fit, uboot_node, "data-size");

  u32 load = (u32)from;
  if (uboot_position > 0) {
    /* Position will include the relative offset from the base of U-Boot's FIT */
    load += uboot_position;
  }

  vbs->software_enforce = spl_getenv_yesno("verify");
  vbs->force_recovery = spl_getenv_yesno("force_recovery");
  printf("OpenBMC verified-boot enforcing [hw:%d sw:%d recovery:%d]\n",
    vbs->hardware_enforce, vbs->software_enforce, vbs->force_recovery);
  debug("%s: U-Boot: data_position=0x%08x, load=0x%08x hw/sw verify=%d/%d\n",
        __func__, uboot_position, load,
        vbs->hardware_enforce, vbs->software_enforce);
  if (vbs->force_recovery == 1) {
    vbs_status(VBS_ERROR_TYPE_FORCE, VBS_ERROR_FORCE_RECOVERY);
    spl_recovery();
  }

  /* Be kind and do not print warnings more than once. */
  u8 notified = 0;

  /*
   * It is possible to disable verified boot at build-time.
   * Later it will be possible to disable using a config similar to the U-Boot
   * verified boot bypass: verify=no
   */
  const void *signature_store = (const void*)gd_fdt_blob();
  vbs->rom_keys = (u32)signature_store;
  if (signature_store == 0x0) {
    /* It is possible the spl_init method did not find a fdt. */
    printf("No signature store was included in the SPL.\n");
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEK);
    CHECK_AND_RECOVER(&notified);
  }

  /* Node path to keys */
  int keys = fdt_path_offset(fit, VBS_KEYS_PATH);
  if (keys < 0) {
    debug("%s: Cannot find /keys node: %d\n", __func__, keys);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEYS);
    CHECK_AND_RECOVER(&notified);
  }

  /* After the first image (uboot) expect to find the subordinate store. */
  int subordinate_node = fdt_first_subnode(fit, keys);
  if (subordinate_node < 0) {
    debug("%s: Cannot find first keys (fdt) node: %d\n", __func__,
        subordinate_node);
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEYS);
    CHECK_AND_RECOVER(&notified);
  }

  /* Access the data and data-size to call image verify directly. */
  int subordinate_size = 0;
  const char* subordinate_data = fdt_getprop(fit, subordinate_node, "data",
    &subordinate_size);
  if (subordinate_data == 0 || subordinate_size <= 0) {
    debug("Cannot find subordinate certificate store.\n");
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_KEYS);
    CHECK_AND_RECOVER(&notified);
  }

  /* This can return success if none of the keys were attempted. */
  int subordinate_verified = 0;
  if (fit_image_verify_required_sigs(fit, subordinate_node, subordinate_data,
             subordinate_size, signature_store, &subordinate_verified)) {
    printf("Unable to verify required subordinate certificate store.\n");
    debug("Check that a '/keys' node was included.\n");
    vbs_status(VBS_ERROR_TYPE_FW, VBS_ERROR_KEYS_INVALID);
    CHECK_AND_RECOVER(&notified);
  }

  /* Check that at least 1 subordinate store was verified. */
  if (subordinate_verified != 0) {
    printf("Subordinate certificate store was not verified.\n");
    vbs_status(VBS_ERROR_TYPE_FW, VBS_ERROR_KEYS_UNVERIFIED);
    CHECK_AND_RECOVER(&notified);
  }

  /*
   * Change the certificate store to the subordinate after it is verified.
   * This means the first image, 'firmware' is signed with a key that is NOT
   * in ROM but rather signed by the verified subordinate key.
   */
  signature_store = subordinate_data;
  vbs->subordainte_keys = (u32)signature_store;

  /*
   * Check the sanity of U-Boot position and size.
   * If there is a problem force recovery.
   */
  if (uboot_size == 0 || uboot_position == 0 || uboot_position == 0xFFFFFFFF) {
    printf("Cannot find U-Boot firmware.\n");
    vbs_status(VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_FW);
    spl_recovery();
  }

  /*
   * Check that the first configuration was verified.
   * This is an interesting error state communication, but it is the API
   * given, so let's make it as clear as possible.
   */
  int uboot_verified = 0;
  if (fit_config_verify_required_sigs(fit, config_node, signature_store,
             &uboot_verified)) {
    printf("Unable to verify required signature of U-Boot configuration.\n");
    vbs_status(VBS_ERROR_TYPE_FW, VBS_ERROR_FW_INVALID);
    CHECK_AND_RECOVER(&notified);
  }

  if (uboot_verified != 0) {
    /* When verified is 0, then an image was verified. */
    printf("U-Boot was not verified.\n");
    debug("Check that the 'required' field for each key- is set to 'conf'.\n");
    debug("Check the board configuration for supported hash algorithms.\n");
    vbs_status(VBS_ERROR_TYPE_FW, VBS_ERROR_FW_UNVERIFIED);
    CHECK_AND_RECOVER(&notified);
  } else {
    printf("U-Boot verified.\n");
  }

  /* Set a handoff and expect U-Boot to clear indicating a clean boot. */
  vbs->recovery_retries = 0;
  vbs->rom_handoff = 0xADEFAD8B;
  jump((volatile void*)load);
}

void board_init_f(ulong bootflag)
{
  /* Must set up console for printing/logging. */
  preloader_console_init();
  /* Must set up global data pointers for local device tree. */
  spl_init();

#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
  ast_wdt_reset(60 * AST_WDT_CLK, 0x33 | 0x08);
#endif

  /*
   * This will never be relocated, so jump directly to the U-boot.
   */
  load_fit((volatile void*)CONFIG_SYS_SPL_FIT_BASE);
  hang();
}
