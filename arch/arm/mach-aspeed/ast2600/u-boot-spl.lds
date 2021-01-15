/*
 * (C) Copyright 2002
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

MEMORY
{
	flash : ORIGIN = CONFIG_SPL_TEXT_BASE,
		LENGTH = CONFIG_SPL_MAX_SIZE
	sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR,
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.text :
	{
		__image_copy_start = .;
		*(.vectors)
		CPUDIR/start.o (.text*)
		*(.text*)
	} > flash

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } > flash

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) } > flash

	. = ALIGN(4);

	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} > flash

	. = ALIGN(4);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

#ifndef CONFIG_SPL_SKIP_RELOCATE
	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	} > flash

	.rel.dyn : {
		*(.rel*)
	} > flash

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	} > flash
#endif

	. = ALIGN(4);

	.end_align : { *(.end_align*) } > flash
	__spl_flash_end = .;

	_image_binary_end = .;

	.bss : {
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} > sdram
}
