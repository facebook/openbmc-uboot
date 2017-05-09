/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <crc.h>

#include <asm/arch/vbs.h>

static int do_vbs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  volatile struct vbs *vbs = (volatile struct vbs*)AST_SRAM_VBS_BASE;
  uint16_t crc = vbs->crc;
  uint32_t handoff = vbs->rom_handoff;
  bool crc_valid = false;

  /* Check CRC value */
  vbs->crc = 0;
  vbs->rom_handoff = 0x0;
  if (crc == crc16_ccitt(0, (uchar*)vbs, sizeof(struct vbs))) {
    crc_valid = true;
  }
  vbs->crc = crc;
  vbs->rom_handoff = handoff;

  printf("ROM executed from:       0x%08x\n", vbs->rom_exec_address);
  printf("ROM KEK certificates:    0x%08x\n", vbs->rom_keys);
  printf("ROM handoff marker:      0x%08x\n", vbs->rom_handoff);
  printf("U-Boot executed from:    0x%08x\n", vbs->uboot_exec_address);
  printf("U-Boot certificates:     0x%08x\n", vbs->subordinate_keys);
  printf("\n");
  printf("Flags force_recovery:    %d\n", (vbs->force_recovery) ? 1 : 0);
  printf("Flags hardware_enforce:  %d\n", (vbs->hardware_enforce) ? 1 : 0);
  printf("Flags software_enforce:  %d\n", (vbs->software_enforce) ? 1 : 0);
  printf("Flags recovery_boot:     %d\n", (vbs->recovery_boot) ? 1 : 0);
  printf("Flags recovery_retries:  %u\n", vbs->recovery_retries);
  printf("\n");
  printf("TPM status:  %u\n", vbs->error_tpm);
  printf("CRC valid:   %d (%hu)\n", (crc_valid) ? 1 : 0, crc);
  printf("Status: type (%d) code (%d)\n", vbs->error_type, vbs->error_code);

	return 0;
}

U_BOOT_CMD(
	vbs,	1,	1,	do_vbs,
	"print verified-boot status",
	""
);
