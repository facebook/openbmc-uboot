/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

#include <asm/arch/vbs.h>

static int do_vbs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  struct vbs *vbs = (struct vbs*)AST_SRAM_VBS_BASE;
  printf("ROM executed from:       0x%08x\n", vbs->rom_exec_address);
  printf("U-Boot executed from:    0x%08x\n", vbs->uboot_exec_address);
  printf("ROM KEK certificates:    0x%08x\n", vbs->rom_keys);
  printf("U-Boot certificates:     0x%08x\n", vbs->subordainte_keys);
  printf("\n");
  printf("Flags force_recovery:    %d\n", (vbs->force_recovery) ? 1 : 0);
  printf("Flags hardware_enforce:  %d\n", (vbs->hardware_enforce) ? 1 : 0);
  printf("Flags software_enforce:  %d\n", (vbs->software_enforce) ? 1 : 0);
  printf("Flags recovery_boot:     %d\n", (vbs->recovery_boot) ? 1 : 0);
  printf("\n");
  printf("Status: type (%d) code (%d)\n", vbs->error_type, vbs->error_code);

	return 0;
}

U_BOOT_CMD(
	vbs,	1,	1,	do_vbs,
	"print verified-boot status",
	""
);
