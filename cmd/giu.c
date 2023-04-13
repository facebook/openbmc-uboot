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

static int vboot_upgrade_vrom(void *fdt)
{
	return 0;
}

static int ack_giu_upgrade_recvimg(void)
{
	volatile char *giu_flag = (volatile char *)VBOOT_OP_CERT_ADDR;
	if (giu_flag[0] == 'G' && giu_flag[1] == 'I' && giu_flag[2] == 'U' &&
	    giu_flag[3] == '\0') {
		giu_flag[0] = 'A', giu_flag[1] = 'C', giu_flag[2] = 'K',
		giu_flag[3] = '\0';
		printf("Success\n");
		return CMD_RET_SUCCESS;
	} else {
		printf("Failed\n");
		return CMD_RET_FAILURE;
	}
}

static int do_giu(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	volatile struct vbs *vbs = (volatile struct vbs *)AST_SRAM_VBS_BASE;
	void *fdt = (void *)vbs->op_cert;

	printf("giu_mode = %d, cert = %p, cert_size = %d\n", vbs->giu_mode, fdt,
	       vbs->op_cert_size);

	switch (vbs->giu_mode) {
	case GIU_NONE:
		printf("GIU_NONE: No golden image upgrading\n");
		return CMD_RET_SUCCESS;
	case GIU_VROM:
		printf("GIU_VROM: Upgrade vboot rom\n");
		return vboot_upgrade_vrom(fdt);
	case GIU_RECV:
		printf("GIU_RECV: Confirming giu light mark request...");
		return ack_giu_upgrade_recvimg();
	case GIU_OPEN:
		printf("GIU_OPEN: Keep GIU latch Open\n");
		return CMD_RET_SUCCESS;
	default:
		printf("Unknown giu_mode %d\n", vbs->giu_mode);
		return CMD_RET_FAILURE;
	}
}

U_BOOT_CMD(giu, 1, 0, do_giu, "Execute golden image upgrading",
	   "giu command will execute the golden image upgrading according to\n"
	   "upgrading mode and information specified in certificate deployed "
	   "in SRAM");
