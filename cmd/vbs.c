/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <u-boot/crc.h>
#if CONFIG_IS_ENABLED(TPM_V1)
#include <tpm-v1.h>
#endif
#if CONFIG_IS_ENABLED(TPM_V2)
#include <tpm-v2.h>
#endif

#include <asm/io.h>
#include <asm/arch/vbs.h>
#include <asm/arch/giu_def.h>

extern int get_tpm(struct udevice **devp);

static void put_vbs(volatile struct vbs *vbs)
{
	uint32_t uboot_exec_address = vbs->uboot_exec_address;
	uint32_t rom_handoff = vbs->rom_handoff;
	/* These are not part of CRC, set them to 0 */
	vbs->rom_handoff = 0;
	vbs->uboot_exec_address = 0;
	vbs->crc = 0;
	vbs->crc = crc16_ccitt(0, (uchar *)vbs, offsetof(struct vbs, vbs_ver));
	vbs->uboot_exec_address = uboot_exec_address;
	vbs->rom_handoff = rom_handoff;
	/* no changes in crc2 protected area now, so no need update crc2 */
}

const char *giu_mode_name(uint8_t giu_mode)
{
	switch (giu_mode) {
	case GIU_NONE:
		return "NONE";
	case GIU_VROM:
		return "VROM";
	case GIU_RECV:
		return "RECV";
	case GIU_OPEN:
		return "OPEN";
	default:
		return "NA";
	}
}

static int do_vbs(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	volatile struct vbs *vbs = (volatile struct vbs *)AST_SRAM_VBS_BASE;
#ifdef CONFIG_ASPEED_TPM
	struct udevice *dev;
	int err;

	err = get_tpm(&dev);
	if (err) {
		return err;
	}
#endif

	if (argc == 3) {
		ulong t = simple_strtoul(argv[1], NULL, 10);
		ulong c = simple_strtoul(argv[2], NULL, 10);
		if (t != VBS_SUCCESS &&
		    (vbs->software_enforce || vbs->hardware_enforce)) {
			vbs->recovery_boot = 1;
			vbs->recovery_retries += 1;
		}
		vbs->error_type = t;
		vbs->error_code = c;
		put_vbs(vbs);
		return 0;
	}

	if (argc == 2) {
		if (strncmp(argv[1], "interrupt", sizeof("interrupt")) == 0) {
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
			/* This will disable the WTD1. */
			printf("Disable WDT\n");
			writel(readl(ASPEED_WDT_BASE + 0x0C) & ~1,
			       ASPEED_WDT_BASE + 0x0C);
#endif
			vbs->rom_handoff = 0x0;
			return 0;
		} else if (strncmp(argv[1], "clear", sizeof("clear")) == 0) {
#ifdef CONFIG_ASPEED_TPM
			char blank[VBS_TPM_ROLLBACK_SIZE];
			memset(blank, 0x0, sizeof(blank));

#if CONFIG_IS_ENABLED(TPM_V1)
			tpm_nv_define_space(
				dev, VBS_TPM_ROLLBACK_INDEX,
				TPM_NV_PER_GLOBALLOCK | TPM_NV_PER_PPWRITE, 0);
			return tpm_nv_write_value(dev, VBS_TPM_ROLLBACK_INDEX,
						  blank, sizeof(blank));
#endif

#if CONFIG_IS_ENABLED(TPM_V2)
			printf("Zero-out Rollback NV-index 0x%08x\n",
			       VBS_TPM_ROLLBACK_INDEX);
			return tpm2_nv_write(dev, NULL, 0, TPM2_RH_PLATFORM,
					     (TPM_HT_NV_INDEX << 24) |
						     (VBS_TPM_ROLLBACK_INDEX &
						      0xFFFFF),
					     (u8 *)blank, sizeof(blank), 0);
#endif

#endif
		} else if (strncmp(argv[1], "oscheck", sizeof("oscheck")) ==
			   0) {
			if (vbs->error_type == VBS_SUCCESS) {
				/**
				 * Verified-boot will enforce from here on. If
				 * it was previously enforcing then it would not
				 * reach this point.
				 */
				env_set("verify", "yes");
			}
		} else {
			printf("Unknown vbs command\n");
		}
		return 0;
	}

	uint16_t crc = vbs->crc;
	uint32_t handoff = vbs->rom_handoff;
	bool crc_valid = false;
	uint16_t crc2 = vbs->crc2;
	bool crc2_valid = false;

	/* Check CRC value */
	vbs->crc = 0;
	vbs->rom_handoff = 0x0;
	if (crc ==
	    crc16_ccitt(0, (uchar *)vbs, offsetof(struct vbs, vbs_ver))) {
		crc_valid = true;
	}
	vbs->crc = crc;
	vbs->rom_handoff = handoff;
	/* Check CRC2 value */
	vbs->crc2 = 0;
	if (crc2 ==
	    crc16_ccitt(0, (uchar *)&vbs->vbs_ver,
			sizeof(struct vbs) - offsetof(struct vbs, vbs_ver))) {
		crc2_valid = true;
	}
	vbs->crc2 = crc2;

	printf("ROM executed from:       0x%08x\n", vbs->rom_exec_address);
	printf("ROM KEK certificates:    0x%08x\n", vbs->rom_keys);
	printf("ROM handoff marker:      0x%08x\n", vbs->rom_handoff);
	printf("U-Boot executed from:    0x%08x\n", vbs->uboot_exec_address);
	printf("U-Boot certificates:     0x%08x\n", vbs->subordinate_keys);
	printf("\n");
	printf("Certificates fallback:   %u\n", vbs->subordinate_last);
	printf("Certificates time:       %u\n", vbs->subordinate_current);
	printf("U-Boot fallback:         %u\n", vbs->uboot_last);
	printf("U-Boot time:             %u\n", vbs->uboot_current);
	printf("Kernel fallback:         %u\n", vbs->kernel_last);
	printf("Kernel time:             %u\n", vbs->kernel_current);
	printf("\n");
	printf("Flags force_recovery:    %d\n", (vbs->force_recovery) ? 1 : 0);
	printf("Flags hardware_enforce:  %d\n",
	       (vbs->hardware_enforce) ? 1 : 0);
	printf("Flags software_enforce:  %d\n",
	       (vbs->software_enforce) ? 1 : 0);
	printf("Flags recovery_boot:     %d\n", (vbs->recovery_boot) ? 1 : 0);
	printf("Flags recovery_retries:  %u\n", vbs->recovery_retries);
	printf("\n");
#if CONFIG_IS_ENABLED(TPM_V1)
	printf("TPM status:  %u\n", vbs->error_tpm);
#endif
#if CONFIG_IS_ENABLED(TPM_V2)
	printf("TPM2 status:  %u\n", vbs->error_tpm2);
#endif
	printf("CRC valid:   %d (%hu)\n", (crc_valid) ? 1 : 0, crc);
	printf("Status: type (%d) code (%d)\n", vbs->error_type,
	       vbs->error_code);

	if (!crc2_valid || !vbs->vbs_ver) {
		printf("vbs_ver: Legacy\n");
		return 0;
	}
	printf("VBS version:              %d\n", vbs->vbs_ver);
	printf("GIU Mode:                 %4s(0x%02x)\n",
	       giu_mode_name(vbs->giu_mode), vbs->giu_mode);
	printf("GIU certificate:          %p\n", (void *)vbs->op_cert);
	printf("GIU certificate size:     %u\n", vbs->op_cert);

	return 0;
}

U_BOOT_CMD(
	vbs, 3, 1, do_vbs, "print verified-boot status",
	"type code - set the vbs error type and code\n"
	"interrupt - disable the watchdog timer and clear ROM handoff check\n"
	"clear - remove all fallback timestamps\n"
	"oscheck - check the vboot status and soft-enable verification on success");
