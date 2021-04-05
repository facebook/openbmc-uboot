// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Facebook Inc.
 */

#include <common.h>
#include <asm/arch/vbs.h>
#include "tpm-spl.h"

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
