/*
 * (C) Copyright 2023-Present, META Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <u-boot/crc.h>
#include <asm/arch/vbs.h>
#include <asm/arch/giu_def.h>

static int upgrade_recv_uboot(void *fdt)
{
	// get the recovery uboot upgrading information

	return 0;
}

static int do_giu(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	volatile struct vbs *vbs = (volatile struct vbs *)AST_SRAM_VBS_BASE;
	void *fdt = (void *)vbs->op_cert;

	int ret;

	printf("giu_mode = %d, cert = %p, cert_size = %d\n", vbs->giu_mode, fdt,
	       vbs->op_cert_size);

	if (vbs->giu_mode == GIU_NONE) {
		printf("GIU_NONE do nothing.\b");
		return 0;
	}

	if (vbs->giu_mode == GIU_CERT) {
		ret = upgrade_recv_uboot(fdt);
	}

	return ret;
}

U_BOOT_CMD(giu, 1, 0, do_giu, "Execute golden image upgrading",
	   "giu command will execute the golden image upgrading according to\n"
	   "upgrading mode and information specified in certificate deployed "
	   "in SRAM");
