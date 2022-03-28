// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Facebook Inc.
 */

#include <common.h>
#include <asm/arch/vbs.h>
#include "tpm-spl.h"
#include "tpm-event.h"


#ifdef CONFIG_ASPEED_TPM
static int vboot_get_image_hash_from_fit(uint8_t *fit, int image_offset,
				uint8_t **valuep, int *value_lenp)
{
	int hash_offset;
	uint8_t *value = 0;
	int value_len = 0;
	int ret = -1;

	log_debug("Get hash from fit 0x%p(%d)\n", fit, image_offset);
	hash_offset = fdt_subnode_offset(fit, image_offset, FIT_HASH_NODENAME);
	if (hash_offset < 0) {
		log_err("No hash subnode in fit = %p, image = %d\n",
			fit, image_offset);
		return ret;
	}

	ret = fit_image_hash_get_value(fit, hash_offset, &value, &value_len);
	if (ret)
		return ret;

	log_debug("hash[%d]:\n", value_len);
	log_debug_buffer( value, value_len );
	if (valuep) *valuep = value;
	if (value_lenp) *value_lenp = value_len;

	return ret;
}

static void vboot_uboot_do_measures(volatile struct vbs *vbs)
{
	uint8_t *value;
	int value_len;
	union tpm_event_index eventidx;

	printf("\n");
	printf("measure vbs...");
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_VBS, 0, measure_vbs);
	ast_tpm_extend(eventidx.index, (uint8_t *)vbs, sizeof(struct vbs));
	printf("done\n");

	printf("measure OS-Kernel...");
	value = 0; value_len = 0;
	vboot_get_image_hash_from_fit(images.fit_hdr_os, images.fit_noffset_os,
		&value, &value_len);
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_OS, 0,
		(vbs->recovery_boot ? measure_recv_os_kernel : measure_os_kernel));
	ast_tpm_extend(eventidx.index, value, value_len);
	printf("done\n");

	printf("measure Ramdisk...");
	value = 0; value_len = 0;
	vboot_get_image_hash_from_fit(images.fit_hdr_rd, images.fit_noffset_rd,
		&value, &value_len);
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_OS, 1,
		(vbs->recovery_boot ? measure_recv_os_rootfs : measure_os_rootfs));
	ast_tpm_extend(eventidx.index, value, value_len);
	printf("done\n");

	printf("measure Fdt...");
	value = 0; value_len = 0;
	vboot_get_image_hash_from_fit(images.fit_hdr_fdt, images.fit_noffset_fdt,
		&value, &value_len);
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_OS, 2,
		(vbs->recovery_boot ? measure_recv_os_dtb : measure_os_dtb));
	ast_tpm_extend(eventidx.index, value, value_len);
	printf("done\n");
}
#endif

static void vboot_finish(void)
{
	/* Clean the handoff marker from ROM. */
	volatile struct vbs *vbs = (volatile struct vbs *)AST_SRAM_VBS_BASE;

#if 	defined(CONFIG_TEST_ASPEED_WATCHDOG_UBOOT) && \
	!defined(CONFIG_ASPEED_RECOVERY_BUILD)
	printf("Testing U-Boot hang recovery ...");
	hang();
#endif

	vbs->rom_handoff = 0x0;

#ifdef CONFIG_ASPEED_TPM
	if (vbs->hardware_enforce || vbs->software_enforce ||
	    vbs->recovery_boot) {
		/* Only do measure when verifiy boot is enabled */
		vboot_uboot_do_measures(vbs);
	}
	ast_tpm_finish();
#endif

}

char *fit_cert_store(void)
{
	volatile struct vbs *vbs = (volatile struct vbs *)AST_SRAM_VBS_BASE;
	return (char *)(vbs->subordinate_keys);
}

void arch_preboot_os(void)
{
	vboot_finish();
}

void vboot_check_enforce(void)
{
	/* Clean the handoff marker from ROM. */
	volatile struct vbs *vbs = (volatile struct vbs *)AST_SRAM_VBS_BASE;
	if (vbs->hardware_enforce) {
		/* If we are hardware-enforcing then this U-Boot is verified. */
		env_set("verify", "yes");
	}
}

