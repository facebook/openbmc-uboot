/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/spl.h>

DECLARE_GLOBAL_DATA_PTR;

#define AST_BOOTMODE_SPI  0
#define AST_BOOTMODE_EMMC 1

u32 ast_bootmode(void);

void board_init_f(ulong dummy)
{
#ifndef CONFIG_SPL_TINY
	spl_early_init();
	timer_init();
	preloader_console_init();
	dram_init();
#endif
}

u32 spl_boot_device(void)
{
	switch(ast_bootmode()) {
#ifdef CONFIG_SPL_MMC_SUPPORT
		case AST_BOOTMODE_EMMC:
			return BOOT_DEVICE_MMC1;
#endif
		case AST_BOOTMODE_SPI:
			return BOOT_DEVICE_RAM;
		default:
			break;
	}
	return BOOT_DEVICE_NONE;
 }

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
    return (struct image_header *)(CONFIG_SYS_TEXT_BASE);
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* boot linux */
	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
