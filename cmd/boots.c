// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <bootm.h>
#include <command.h>
#include <environment.h>
#include <errno.h>
#include <image.h>
#include <malloc.h>
#include <nand.h>
#include <asm/byteorder.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <u-boot/zlib.h>
#include <asm/arch/aspeed_verify.h>

DECLARE_GLOBAL_DATA_PTR;

int do_boots(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char boot_arg[11];
	char *boot_argv[1];
	struct aspeed_secboot_header *sb_header = (struct aspeed_secboot_header *)CONFIG_ASPEED_KERNEL_FIT_DRAM_BASE;
	struct aspeed_secboot_header *cur_header = (struct aspeed_secboot_header *)CONFIG_SYS_TEXT_BASE - 1;
	struct aspeed_secboot_header *load_hdr;

	if (argc == 1)
		load_hdr = sb_header;
	else
		load_hdr = (struct aspeed_secboot_header *)simple_strtoul(argv[1], NULL, 16);

	if (strcmp((char *)load_hdr->sbh_magic, ASPEED_SECBOOT_MAGIC_STR)) {
		printf("secure_boot: cannot find secure boot header\n");
		return -EPERM;
	}
	if (sb_header != load_hdr)
		memcpy(sb_header, load_hdr, load_hdr->sbh_img_size + 512);

	if (aspeed_verify_boot(cur_header, sb_header) != 0)
		return -EPERM;

	sprintf(boot_arg, "%x", CONFIG_ASPEED_KERNEL_FIT_DRAM_BASE + sizeof(*sb_header));
	boot_argv[0] = boot_arg;

	return do_bootm_states(cmdtp, flag, 1, (char *const *)boot_argv, BOOTM_STATE_START |
			       BOOTM_STATE_FINDOS | BOOTM_STATE_FINDOTHER |
			       BOOTM_STATE_LOADOS |
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
			       BOOTM_STATE_RAMDISK |
#endif
			       BOOTM_STATE_OS_PREP | BOOTM_STATE_OS_FAKE_GO |
			       BOOTM_STATE_OS_GO, &images, 1);
}

U_BOOT_CMD(
	boots, 2, 0, do_boots,
	"Aspeed secure boot with in-memory image",
	""
);
