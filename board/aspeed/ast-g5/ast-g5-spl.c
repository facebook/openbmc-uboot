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
#include <asm/io.h>

#include "flash-spl.h"

/* Location in SRAM used for verified boot content/flags. */
#define AST_SRAM_VBS_BASE   0x1E720100
#define AST_SRAM_VBS_FLAGS  (AST_SRAM_VBS_BASE + 0x04)

/* Size of RW environment parsable by ROM. */
#define AST_ROM_ENV_MAX     0x200

DECLARE_GLOBAL_DATA_PTR;

static ulong fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
  const u32 *cell;
  int len;

  cell = fdt_getprop(fdt, node, prop, &len);
  if (len != sizeof(*cell)) {
    return -1U;
  }
  return fdt32_to_cpu(*cell);
}

void __noreturn jump(u32 to)
{
  debug("Blindly jumping to 0x%08x\n", to);
  typedef void __noreturn (*image_entry_noargs_t)(void);

  image_entry_noargs_t image_entry =
    (image_entry_noargs_t)(unsigned long)to;
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
  writeb(1, AST_SRAM_VBS_FLAGS);

  /* Jump to the well-known location of the Recovery U-Boot. */
  printf("Booting recovery U-Boot\n");
  jump(CONFIG_SYS_RECOVERY_BASE);
}

int spl_getenv_yesno(const char* var) {
  const char* n = 0;
  const char* v = 0;
  int size = strlen(var);

  /* Use a RW environment. */
  env_t *env = (env_t*)CONFIG_ENV_ADDR;

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

int verify_required(int hw_verify, int sw_verify) {
  if (hw_verify == 1) {
    return 1;
  } else if (sw_verify == 1) {
    return 1;
  }

  debug("OpenBMC verified-boot is not enforcing in hardware or software.\n");
  debug("Allowing execution to proceed into an unverified state.\n");
  return 0;
}

void load_fit(u32 from) {
  struct image_header *header;

  /* Reset the target RW flash chip. */
  if (!ast_fmc_spi_cs1_reset()) {
    debug("%s: Cannot reset FMC/CS1 (SPI0.1) state", __func__);
    spl_recovery();
  }

  header = (struct image_header*)(from);
  if (image_get_magic(header) != FDT_MAGIC) {
    /* FIT loading is not available or this U-Boot is not a FIT. */
    /* This will bypass signature checking */
    debug("%s: Cannot find FIT image header: 0x%08x\n", __func__, from);
    spl_recovery();
  }

  void *fit = header;
  u32 size = fdt_totalsize(fit);
  size = (size + 3) & ~3;
  u32 base_offset = (size + 3) & ~3;
  debug("%s: Parsed FIT: size=%x base_offset=%x\n", __func__,
        size, base_offset);

  /* Node path to images */
  int images = fdt_path_offset(fit, FIT_IMAGES_PATH);
  if (images < 0) {
    debug("%s: Cannot find /images node: %d\n", __func__, images);
    spl_recovery();
  }

  /* Be simple, select the first image. */
  int node = fdt_first_subnode(fit, images);
  if (node < 0) {
    debug("%s: Cannot find first image node: %d\n", __func__, node);
    spl_recovery();
  }

  /* Get its information and set up the spl_image structure */
  int data_position = fdt_getprop_u32(fit, node, "data-position");
  int data_size = fdt_getprop_u32(fit, node, "data-size");

  u32 load = from;
  if (data_position > 0) {
    /* Position will include the relative offset from the base of U-Boot's FIT */
    load += data_position;
  }

  int hw_verify = 0;
  int sw_verify = spl_getenv_yesno("verify");
  int force_recovery = spl_getenv_yesno("force_recovery");
  printf("OpenBMC verified-boot enforcing [hw:%d sw:%d recovery: %d]\n",
    hw_verify, sw_verify, force_recovery);
  debug("%s: U-Boot: data_position=0x%08x, load=0x%08x hw/sw verify=%d/%d\n",
        __func__, data_position, load, hw_verify, sw_verify);
  if (force_recovery == 1) {
    spl_recovery();
  }

  /*
   * It is possible to disabled verified boot at build-time.
   * Later it will be possible to disable using a config similar to the U-Boot
   * verified boot bypass: verify=no
   */
  void *signature_store = (void*)gd_fdt_blob();
  if (signature_store == 0x0) {
    /* It is possible the spl_init method did not find a fdt. */
    printf("No signature store was included in the SPL.\n");
    if (verify_required(hw_verify, sw_verify)) {
      spl_recovery();
    }
  }

#ifdef CONFIG_SPL_FIT_SUBORDINATE_KEYS
  /* Node path to subordinate keys. */
  int keys = fdt_path_offset(fit, FIT_KEYS_PATH);
  if (keys < 0) {
    debug("%s: Cannot find /keys node: %d\n", __func__, keys);
    if (verify_required(hw_verify, sw_verify)) {
      spl_recovery();
    }
  }

  int subordinate_verified = 0;
  /* Be simple, select the first key. */
  int key_node = fdt_first_subnode(fit, keys);
  subordinate_verified = fit_config_verify(fit, key_node);
  debug("%s: Subordinate keys verified %d\n", __func__, subordinate_verified);
#endif

  /*
   * Check that at least 1 image was verified.
   * This is an interesting error state communication, but it is the API
   * given, so let's make it as clear as possible.
   */
  int verified = 0;

  /*
   * The load and offset *should* be the same, we'll need to fix that
   * in the FIT generation + external movement.
   *
   * For now we can set this to the data.
   */
  void *data = (void*)load;

  /* Verify all required signatures, keys must be marked required. */
  if (fit_image_verify_required_sigs(fit, node, data, data_size,
             signature_store, &verified)) {
    printf("Unable to verify required signature.\n");
    if (verify_required(hw_verify, sw_verify)) {
      spl_recovery();
    }
  }

  if (verified != 0) {
    /* When verified is 0, then an image was verified. */
    printf("U-Boot was not verified.\n");
    debug("Check that the 'required' field for each key- is set to 'image'.\n");
    debug("Check the board configuration for supported hash algorithms.\n");
    if (verify_required(hw_verify, sw_verify)) {
      spl_recovery();
    }
  } else {
    printf("U-Boot verified.\n");
  }
  jump(load);
}

void board_init_f(ulong bootflag)
{
  /* Must set up console for printing/logging. */
  preloader_console_init();
  /* Must set up global data pointers for local device tree. */
  spl_init();

  /*
   * We are not relocated, use the simple malloc with the relocated
   * malloc start and size configuration options.
   *
   * Malloc will forever count forward, there is no free, as state
   * tracking does not mean anything.
   */
  /* gd->malloc_base = CONFIG_SYS_SPL_MALLOC_START; */
  /* gd->malloc_limit = CONFIG_SYS_SPL_MALLOC_SIZE; */

  /*
   * This will never be relocated, so jump directly to the U-boot.
   */
  load_fit(CONFIG_SYS_SPL_FIT_BASE);
  hang();
}

