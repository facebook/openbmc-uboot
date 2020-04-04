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
#include <asm/arch/aspeed_verify.h>

DECLARE_GLOBAL_DATA_PTR;

#define AST_BOOTMODE_SPI	0
#define AST_BOOTMODE_EMMC	1
#define AST_BOOTMODE_UART	2

u32 aspeed_bootmode(void);

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
	switch(aspeed_bootmode()) {
		case AST_BOOTMODE_EMMC:
			return BOOT_DEVICE_MMC1;
		case AST_BOOTMODE_SPI:
			return BOOT_DEVICE_RAM;
		case AST_BOOTMODE_UART:
			return BOOT_DEVICE_UART;
		default:
			break;
	}
	return BOOT_DEVICE_NONE;
}

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
#ifdef CONFIG_SECURE_BOOT
	void *dst = (void*)CONFIG_SYS_UBOOT_START;
	void *src = (void*)CONFIG_SYS_TEXT_BASE;
	u32 count = CONFIG_SYS_MONITOR_LEN;
	memmove(dst, src, count);
#endif
    return (struct image_header *)(CONFIG_SYS_TEXT_BASE);
}

#ifdef CONFIG_SECURE_BOOT
void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;
	if (aspeed_bl2_verify((void*)spl_image->entry_point, CONFIG_SPL_TEXT_BASE) != 0)
		hang();
	image_entry();
}
#endif

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
