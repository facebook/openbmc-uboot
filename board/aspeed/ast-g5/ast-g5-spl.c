/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/spl.h>
#include <malloc.h>

#include <image.h>
#include <libfdt.h>

#include <asm/arch/aspeed.h>

#include "flash-spl.h"

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

void load_fit(u32 from) {
  struct image_header *header;

  /* Reset the target RW flash chip. */
  ast_fmc_spi_cs1_reset();

  header = (struct image_header*)(from);
  if (!IS_ENABLED(CONFIG_SPL_LOAD_FIT) ||
      image_get_magic(header) != FDT_MAGIC) {
    /* FIT loading is not available or this U-Boot is not a FIT. */
    /* This will bypass signature checking */
    debug("%s: Cannot find FIT image header: 0x%08x\n", __func__, from);
    jump(from);
  }

  void *fit = header;
  u32 size = fdt_totalsize(fit);
  size = (size + 3) & ~3;
  u32 base_offset = (size + 3) & ~3;
  debug("size=%x base_offset=%x\n", size, base_offset);

  /* Node path to images */
  int images = fdt_path_offset(fit, FIT_IMAGES_PATH);
  if (images < 0) {
    debug("%s: Cannot find /images node: %d\n", __func__, images);
    hang();
  }

  /* Be simple, select the first image. */
  int node = fdt_first_subnode(fit, images);
  if (node < 0) {
    debug("%s: Cannot find first image node: %d\n", __func__, node);
    hang();
  }

  /* Get its information and set up the spl_image structure */
  int data_offset = fdt_getprop_u32(fit, node, "data-offset");
  int data_size = fdt_getprop_u32(fit, node, "data-size");
  u32 load = fdt_getprop_u32(fit, node, "load");
  debug("%s: data_offset=%x, data_size=%x load=%x\n",
    __func__, data_offset, data_size, load);

  /*
   * It is possible to disabled verified boot at build-time.
   * Later it will be possible to disable using a config similar to the U-Boot
   * verified boot bypass: verify=no
   */
#ifdef CONFIG_SPL_FIT_SIGNATURE

  void *signature_store = (void*)gd_fdt_blob();
  if (signature_store == 0x0) {
    /* It is possible the spl_init method did not find a fdt. */
    printf("No signature store was included in the SPL.\n");
    hang();
  }

  /* Node path to subordinate keys. */
  int keys = fdt_path_offset(fit, FIT_KEYS_PATH);
  if (keys < 0) {
    debug("%s: Cannot find /keys node: %d\n", __func__, keys);
    hang();
  }

  int subordinate_verified = 0;
  /* Be simple, select the first key. */
  int key_node = fdt_first_subnode(fit, keys);
  subordinate_verified = fit_config_verify(fit, key_node);
  debug("%s: Subordinate keys verified %d\n", __func__, subordinate_verified);

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
    hang();
  }

  if (verified != 0) {
    /* When verified is 0, then an image was verified. */
    printf("No images were verified.\n");
    hang();
  }

  printf("Images verified\n");
#endif
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

