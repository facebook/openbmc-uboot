/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#define DEBUG
#include <common.h>
#include <spl.h>
#include <asm/spl.h>
#include <malloc.h>
#include <u-boot/crc.h>

#include <environment.h>

#include <image.h>
#include <linux/libfdt.h>

#include <asm/arch/platform.h>
#include <asm/arch/vbs.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm.h>

#include "flash-spl.h"
#include "tpm-spl.h"
#include "util.h"
#include "tpm-event.h"
#include "vboot-op-cert.h"

/* Size of RW environment parsable by ROM. */
#define AST_ROM_ENV_MAX 0x200

/* Max size of U-Boot FIT */
#define AST_MAX_UBOOT_FIT 0x4000

#define vboot_recovery(v, t, c)                                                \
	do {                                                                   \
		debug("%s %d\n", __func__, __LINE__);                          \
		real_vboot_recovery((v), (t), (c));                            \
	} while (0)

#define vboot_enforce(v, t, c)                                                 \
	do {                                                                   \
		debug("%s %d\n", __func__, __LINE__);                          \
		real_vboot_enforce((v), (t), (c));                             \
	} while (0)

DECLARE_GLOBAL_DATA_PTR;

/**
 * vboot_getenv_yesno() - Turn an environment variable into a boolean
 *
 * @param var the environment variable name
 * @return 0 for false, 1 for true
 */
static u8 vboot_getenv_yesno(const char *var)
{
	const char *n = 0;
	const char *v = 0;
	int size = strlen(var);

	/* Use a RW environment. */
	volatile env_t *env = (volatile env_t *)CONFIG_ENV_ADDR;

	int offset;
	char last = 0;
	char current = env->data[0];
	for (offset = 1; offset < AST_ROM_ENV_MAX; ++offset) {
		last = current;
		current = env->data[offset];
		if (last == 0 && current == 0) {
			/* This is the end of the environment. */
			return 0;
		}

		if (n == 0) {
			/* This is the first variable. */
			n = (const char *)&env->data[offset - 1];
		} else if (current == 0) {
			/* This is the end of a variable. */
			v = 0;
			n = (const char *)&env->data[offset + 1];
		} else if (v == 0 && current == '=') {
			v = (const char *)&env->data[offset + 1];
			if (strncmp(n, var, size) == 0) {
				if (*v == '1' || *v == 'y' || *v == 't' ||
				    *v == 'T' || *v == 'Y') {
					return 1;
				}
			}
		}
	}
	return 0;
}

/**
 * vboot_store() - Store the VBS state into SRAM
 *
 * @param pointer to vbs state
 */
void vboot_store(struct vbs *vbs)
{
	u32 rom_handoff;

	rom_handoff = vbs->rom_handoff;
	vbs->crc = 0;
	vbs->rom_handoff = 0x0;
	/* backward compatible with vboot-util, keep crc covering unchanged */
	vbs->crc = crc16_ccitt(0, (uchar *)vbs, offsetof(struct vbs, vbs_ver));
	vbs->rom_handoff = rom_handoff;
	memcpy((void *)AST_SRAM_VBS_BASE, vbs, sizeof(struct vbs));
}

/**
 * vboot_jump() - Jump, or set the PC, to an address.
 *
 * @param to an address
 */
void __noreturn vboot_jump(volatile void *to, struct vbs *vbs)
{
	vboot_store(vbs);

	if (to == 0x0) {
		debug("Resetting CPU0\n");
		reset_cpu(0);
	}

	debug("Blindly jumping to 0x%08x\n", (u32)to);
	typedef void __noreturn (*image_entry_noargs_t)(void);
	image_entry_noargs_t image_entry = (image_entry_noargs_t)to;
	image_entry();
}

/**
 * vboot_status() - Set the vboot status on the first error.
 *
 * @param t the error type (category)
 * @param c the error code
 */
static void vboot_status(struct vbs *vbs, u8 t, u8 c)
{
	/* Store an error condition in the VBS structure for reporting/testing. */
	if (t != VBS_SUCCESS && vbs->error_code == VBS_SUCCESS) {
		/* Only store the initial error code and type. */
		vbs->error_code = c;
		vbs->error_type = t;
	}
}

#ifdef CONFIG_ASPEED_TPM
/**
 * vboot_spl_do_measures() - Measure SPL, key-store, rec-uboot uboot-env.
 */
static void vboot_spl_do_measures(u8 *uboot, uint32_t uboot_size)
{
	union tpm_event_index eventidx;
	printf("\n");

	printf("measure SPL...");
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_SPL, 0, measure_spl);
	ast_tpm_extend(eventidx.index, (unsigned char *)0x0,
		       CONFIG_SPL_MAX_FOOTPRINT);
	printf("done\n");

	printf("measure key-store...");
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_FIT, 0, measure_keystore);
	ast_tpm_extend(eventidx.index, (unsigned char *)CONFIG_SYS_SPL_FIT_BASE,
		       AST_MAX_UBOOT_FIT);
	printf("done\n");

	if (uboot_size) {
		printf("measure U-Boot...");
		SET_EVENT_IDX(eventidx, AST_TPM_PCR_UBOOT, 0, measure_uboot);
		ast_tpm_extend(eventidx.index, uboot, uboot_size);
		printf("done\n");
	} else {
		printf("measure recovery U-Boot...");
		SET_EVENT_IDX(eventidx, AST_TPM_PCR_UBOOT, 0,
			      measure_recv_uboot);
		ast_tpm_extend(eventidx.index,
			       (unsigned char *)CONFIG_SYS_RECOVERY_BASE,
			       CONFIG_RECOVERY_UBOOT_SIZE);
		printf("done\n");
	}

	printf("measure U-Boot environment...");
	SET_EVENT_IDX(eventidx, AST_TPM_PCR_ENV, 0, measure_uboot_env);
	ast_tpm_extend(eventidx.index, (unsigned char *)CONFIG_ENV_ADDR,
		       CONFIG_ENV_SIZE);
	printf("done\n");
}
#endif
/**
 * vboot_recovery() - Boot the Recovery U-Boot.
 */
static void real_vboot_recovery(struct vbs *vbs, u8 t, u8 c)
{
#ifdef CONFIG_ASPEED_TPM
	int tpm_state = ast_tpm_get_state();
	/* make sure TPM is provisioned */
	if (AST_TPM_STATE_INIT == tpm_state) {
		tpm_state = AST_TPM_STATE_GOOD;
		if (ast_tpm_provision(vbs))
			tpm_state = AST_TPM_STATE_FAIL;
	}
	/* make sure all PCRs used by SPL closed if TPM_STATE is good*/
	if (AST_TPM_STATE_GOOD == tpm_state) {
		vboot_spl_do_measures(0, 0);
	}
#endif /* ASPEED_TPM */

	vboot_status(vbs, t, c);
	/* Overwrite all of the verified boot flags we've defined. */
	vbs->recovery_boot = 1;
	vbs->recovery_retries += 1;
	vbs->rom_handoff = 0x0;

	/* Jump to the well-known location of the Recovery U-Boot. */
	printf("Booting recovery U-Boot.\n");
	vboot_jump((volatile void *)CONFIG_SYS_RECOVERY_BASE, vbs);
}

/**
 * vboot_getenv_yesno() - Turn an environment variable into a boolean
 *
 * @param var the environment variable name
 * @return 0 for false, 1 for true
 */
static void real_vboot_enforce(struct vbs *vbs, u8 t, u8 c)
{
	vboot_status(vbs, t, c);

	if (vbs->hardware_enforce == 1 || vbs->software_enforce == 1) {
		vboot_recovery(vbs, 0, 0);
	}
}

void vboot_check_fit(void *fit, struct vbs *vbs, int *uboot, int *config,
		     u32 *uboot_position, u32 *uboot_size)
{
	u32 fit_size;
	int images_path;
	int configs_path;

	if (fdt_magic(fit) != FDT_MAGIC) {
		/* FIT loading is not available or this U-Boot is not a FIT. */
		/* This will bypass signature checking */
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_MAGIC);
	}

	fit_size = fdt_totalsize(fit);
	fit_size = (fit_size + 3) & ~3;
	if (fit_size > AST_MAX_UBOOT_FIT) {
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA,
			       VBS_ERROR_INVALID_SIZE);
	}

	/* Node path to images */
	images_path = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_path < 0) {
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_IMAGES);
	}

	/* Be simple, select the first image. */
	*uboot = fdt_first_subnode(fit, images_path);
	if (*uboot < 0) {
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_FW);
	}

	/* Node path to configurations */
	configs_path = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (configs_path < 0) {
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_CONFIG);
	}

	/* We only support a single configuration for U-Boot. */
	*config = fdt_first_subnode(fit, configs_path);
	if (*config < 0) {
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_CONFIG);
	}

	/* Get its information and set up the spl_image structure */
	*uboot_position = fdt_getprop_u32(fit, *uboot, "data-position");
	*uboot_size = fdt_getprop_u32(fit, *uboot, "data-size");

	/*
   	 * Check the sanity of U-Boot position and size.
   	 * If there is a problem force recovery.
   	 */
	if (*uboot_size == 0 || *uboot_position == 0 || *uboot_size > 0xE0000 ||
	    *uboot_position > 0xE0000) {
		printf("Cannot find U-Boot firmware.\n");
		vboot_recovery(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_FW);
	}
}

void vboot_rollback_protection(const void *fit, uint8_t image, struct vbs *vbs)
{
	/* Only attempt to update timestamps if the TPM was provisioned. */
	int root = fdt_path_offset(fit, "/");
	int timestamp = fdt_getprop_u32(fit, root, "timestamp");
	bool no_fallback =
		(fdt_getprop(fit, root, "no-fallback", NULL) != NULL);
	if (root < 0 || timestamp <= 0) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_RB,
			      VBS_ERROR_ROLLBACK_MISSING);
	}

	int tpm_status =
		ast_tpm_try_version(vbs, image, timestamp, no_fallback);
	if (tpm_status != VBS_SUCCESS) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_RB, tpm_status);
	}
}

void vboot_verify_subordinate(void *fit, struct vbs *vbs)
{
	/* Node path to keys */
	int keys_path = fdt_path_offset(fit, VBS_KEYS_PATH);
	if (keys_path < 0) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEYS);
		return;
	}

	/* After the first image (uboot) expect to find the subordinate store. */
	int subordinate = fdt_first_subnode(fit, keys_path);
	if (subordinate < 0) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEYS);
		return;
	}

	/* Access the data and data-size to call image verify directly. */
	int subordinate_size = 0;
	const char *data =
		fdt_getprop(fit, subordinate, "data", &subordinate_size);
	if (data != 0 && subordinate_size > 0) {
		/* This can return success if none of the keys were attempted. */
		int verified = 0;
		if (fit_image_verify_required_sigs(
			    fit, subordinate, data, subordinate_size,
			    (const char *)vbs->rom_keys, &verified)) {
			printf("Unable to verify required subordinate certificate store.\n");
			vboot_enforce(vbs, VBS_ERROR_TYPE_FW,
				      VBS_ERROR_KEYS_INVALID);
		}

		/* Check that at least 1 subordinate store was verified. */
		if (verified == 0) {
#ifdef CONFIG_ASPEED_TPM
			vboot_rollback_protection(
				data, AST_TPM_ROLLBACK_SUBORDINATE, vbs);
#endif

			/*
	 * Change the certificate store to the subordinate after it is verified.
         * This means the first image, 'firmware' is signed with a key that is NOT
         * in ROM but rather signed by the verified subordinate key.
         */
			vbs->subordinate_keys = (u32)data;
		} else {
			printf("Subordinate certificate store was not verified.\n");
			vboot_enforce(vbs, VBS_ERROR_TYPE_FW,
				      VBS_ERROR_KEYS_UNVERIFIED);
		}
	} else {
		vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_BAD_KEYS);
	}
}

void vboot_verify_uboot(void *fit, struct vbs *vbs, void *load, int config,
			int uboot, int uboot_size, uint8_t *uboot_hash)
{
	int verified = 0;
	const char *sig_store = (const char *)vbs->rom_keys;
	if (vbs->subordinate_keys != 0x0) {
		// The subordinate keys values will be non-0 when verified.
		sig_store = (const char *)vbs->subordinate_keys;
	}

	if (fit_config_verify_required_sigs(fit, config, sig_store,
					    &verified)) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_FW_INVALID);
	}

	if (verified != 0) {
		/* When verified is 0, then an image was verified. */
		printf("U-Boot configuration was not verified.\n");
		debug("Check that the 'required' field for each key- is set to 'conf'.\n");
		debug("Check the board configuration for supported hash algorithms.\n");
		vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_FW_UNVERIFIED);
		return;
	}

	/* Now verify the hash of the first image. */
	char *error;
	int hash = fdt_subnode_offset(fit, uboot, FIT_HASH_NODENAME);
	if (fit_image_check_hash(fit, hash, load, uboot_size, uboot_hash,
				 &error)) {
		printf("\nU-Boot was not verified.\n");
		vboot_enforce(vbs, VBS_ERROR_TYPE_FW, VBS_ERROR_FW_UNVERIFIED);
		return;
	}

	printf("\nU-Boot verified.\n");
}

static void set_and_lock_hwstrap(bool should_lock)
{
/* currently only apply to AST2600 verified boot platform */
#if defined(CONFIG_ASPEED_AST2600)

/* SCU500, SCU504 and SCU508 */
#define AST2600_HWSTRAP_SET_REG1 0x1E6E2500
#define AST2600_HWSTRAP_CLEAR_REG1 0x1E6E2504
#define AST2600_HWSTRAP_LOCK_REG1 0x1E6E2508
#define AST2600_HWSTRAP_REG1_BIT_BOOT_EMMC (1UL << 2)
#define AST2600_HWSTRAP_REG1_BIT_BOOT_DBUG_SPI (1UL << 3)
#define AST2600_HWSTRAP_REG1_BIT_DISABLE_DBG_PATH_SET1 (1UL << 19)

/* SCU510, SCU514 and SCU518 */
#define AST2600_HWSTRAP_SET_REG2 0x1E6E2510
#define AST2600_HWSTRAP_CLEAR_REG2 0x1E6E2514
#define AST2600_HWSTRAP_LOCK_REG2 0x1E6E2518
#define AST2600_HWSTRAP_REG2_BIT_DISABLE_DBG_PATH_SET2 (1UL << 4)
#define AST2600_HWSTRAP_REG2_BIT_BOOT_UART (1UL << 8)
#define AST2600_HWSTRAP_REG2_BIT_ABR_MODE_SINGLE_FLASH (1UL << 12)

	u32 hwstrap_lock_reg1;
	u32 hwstrap_lock_reg2;
	if (should_lock) {
		printf("runtime clear hwstraps:\n");
		printf(" boot from emmc,debug-spi,uart and middle of flash0\n");
		setbits_le32(AST2600_HWSTRAP_CLEAR_REG1,
			     (AST2600_HWSTRAP_REG1_BIT_BOOT_EMMC |
			      AST2600_HWSTRAP_REG1_BIT_BOOT_DBUG_SPI));
		setbits_le32(AST2600_HWSTRAP_CLEAR_REG2,
			     (AST2600_HWSTRAP_REG2_BIT_BOOT_UART |
			      AST2600_HWSTRAP_REG2_BIT_ABR_MODE_SINGLE_FLASH));
		printf("runtime set hwstraps: disable debug path\n");
		setbits_le32(AST2600_HWSTRAP_SET_REG1,
			     AST2600_HWSTRAP_REG1_BIT_DISABLE_DBG_PATH_SET1);
		setbits_le32(AST2600_HWSTRAP_SET_REG2,
			     AST2600_HWSTRAP_REG2_BIT_DISABLE_DBG_PATH_SET2);
		printf("runtime protect all these hwstraps from changing\n");
		setbits_le32(AST2600_HWSTRAP_LOCK_REG1,
			     (AST2600_HWSTRAP_REG1_BIT_BOOT_EMMC |
			      AST2600_HWSTRAP_REG1_BIT_BOOT_DBUG_SPI |
			      AST2600_HWSTRAP_REG1_BIT_DISABLE_DBG_PATH_SET1));
		setbits_le32(AST2600_HWSTRAP_LOCK_REG2,
			     (AST2600_HWSTRAP_REG2_BIT_DISABLE_DBG_PATH_SET2 |
			      AST2600_HWSTRAP_REG2_BIT_BOOT_UART |
			      AST2600_HWSTRAP_REG2_BIT_ABR_MODE_SINGLE_FLASH));
	}
	hwstrap_lock_reg1 = readl(AST2600_HWSTRAP_LOCK_REG1);
	hwstrap_lock_reg2 = readl(AST2600_HWSTRAP_LOCK_REG2);
	printf("hwstrap write protect SCU508=0x%08x, SCU518=0x%08x\n",
	       hwstrap_lock_reg1, hwstrap_lock_reg2);

#endif
}

static int vboot_setup_wp_latch(struct vbs *vbs, void **pp_timer,
				void **pp_spi_check)
{
#ifdef CONFIG_GIU_HW_SUPPORT
	int ret, node;
	struct gpio_desc latch_ctrl;
	struct gpio_desc latch_status;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "bsm_wp_latch");
	if (node < 0) {
		printf("Cannot get bsm_wp_latch fdt node (%d)\n", node);
		return AST_FMC_ERROR;
	}
	ret = gpio_request_by_name_nodev(offset_to_ofnode(node),
					 "latch_ctrl-gpios", 0, &latch_ctrl,
					 GPIOD_IS_OUT);
	if (ret < 0) {
		printf("Request latch_ctrl-gpios failed (%d)\n", ret);
		return AST_FMC_ERROR;
	}
	ret = gpio_request_by_name_nodev(offset_to_ofnode(node),
					 "latch_status-gpios", 0, &latch_status,
					 GPIOD_IS_IN);
	if (ret < 0) {
		printf("Request latch_status-gpios failed (%d)\n", ret);
		return AST_FMC_ERROR;
	}
	ret = dm_gpio_get_value(&latch_status);
	if (ret < 0) {
		printf("Read BSM lock status failed (%d)\n", ret);
		return AST_FMC_ERROR;
	}

	if (ret && (vbs->rom_handoff == 0)) {
		printf("reboot to open latch\n");
		/* deactive latch, notice dm_gpio_set_value will set
		* physical gpio value based on GPIOD_ACTIVE_LOW or HIGH
		* of the latch_ctrl gpio configuration
		*/
		dm_gpio_set_value(&latch_ctrl, false);
		vbs->rom_handoff = VBS_HANDOFF_OPEN_LATCH;
		vboot_jump(0, vbs);
		hang();
	}
	if (vbs->rom_handoff == VBS_HANDOFF_OPEN_LATCH) {
		printf("clean up VBS_HANDOFF_OPEN_LATCH rom_handoff\n");
		vbs->rom_handoff = 0;
	}
	if (ret) {
		printf("WARNING! bsm latch cannot open\n");
		return AST_FMC_ERROR;
	}
	/* reuse fmc_spi_check to clear SPI SR to 0 when latch open */
	printf("clear flash device SR to 0 and confirm latch open\n");
	ret = ast_fmc_spi_check(true, GIU_NONE, pp_timer, pp_spi_check);
	if (ret != AST_FMC_WP_OFF) {
		printf("WARNING! flash SR clear failed. spi_check=%s\n",
		       (ret == AST_FMC_WP_ON) ? "ON" : "ERROR");
		return AST_FMC_ERROR;
	}

	if (IS_ENABLED(CONFIG_GIU_HW_OPEN)) {
		printf("leave latch open\n");
		return AST_FMC_WP_OFF;
	}

	printf("close latch before setup flash SR based on giu_mode\n");
	dm_gpio_set_value(&latch_ctrl, true);
	/* spin loop could took around 1ms */
	int latch_active_puls_delay = 1000;
	while (latch_active_puls_delay--)
		;
	printf("then keep latch ctrl lift\n");
	dm_gpio_set_value(&latch_ctrl, false);

	return AST_FMC_WP_ON;
#else
	printf("Not support Golden Image Upgrade.\n");
	return AST_FMC_ERROR;
#endif
}

#ifdef CONFIG_GIU_HW_SUPPORT
static int vboot_get_giu_mode_from_cert(struct vbs *vbs)
{
	return GIU_NONE;
}

static void vboot_verify_op_cert(struct vbs *vbs)
{
	/* default init output */
	void *cert_fit = (void*) VBOOT_OP_CERT_ADDR;
	vbs->op_cert = 0;
	vbs->op_cert_size = 0;
	/* check vboot operation certficate is wellformed fit */
	if (fdt_magic(cert_fit) != FDT_MAGIC) {
		printf("No vboot operation certificate file deployed\n");
		return;
	}
	size_t cert_fit_size = fdt_totalsize(cert_fit);
	cert_fit_size = (cert_fit_size + 3) & ~3;
	if (cert_fit_size > VBOOT_OP_CERT_MAX_SIZE) {
		printf("vboot operation certificate file %d is too big\n",
		       cert_fit_size);
		return;
	}
	/* Node path to certificate */
	int cert_path = fdt_path_offset(cert_fit, "/images/fdt@1");
	if (cert_path < 0) {
		printf("vboot operation certificate is malformed\n");
		return;
	}
	/* get the cert and cert_size to call image verify directly. */
	int cert_size = 0;
	const char *cert =
		fdt_getprop(cert_fit, cert_path, "data", &cert_size);
	if (cert == 0 || cert_size <= 0) {
		return;
	}
	/* verify the signature of the certificate */
	int not_required = 0;
	if (fit_image_verify_required_sigs(cert_fit, cert_path, cert, cert_size,
					   (const char *)vbs->rom_keys,
					   &not_required) ||
	    not_required) {
		printf("verify vboot operation certificate fail\n");
		return;
	}
	vbs->op_cert = (u32)cert;
	vbs->op_cert_size = (u32)cert_size;
}
#endif

static int vboot_get_giu_mode(struct vbs *vbs, int bsm_latched)
{
#ifdef CONFIG_GIU_HW_SUPPORT
	switch (bsm_latched) {
	case AST_FMC_ERROR:
		printf("BSM latch circuit error, booting with GIU_NONE mode\n");
		return GIU_NONE;
	case AST_FMC_WP_ON:
	case AST_FMC_WP_OFF:
		/* continue golden image upgrading mode select logic */
		break;
	default:
		printf("Unknown bsm_latched state %d, booting with GIU_NONE\n",
		       bsm_latched);
		return GIU_NONE;
	}

	/* verify vboot operation certificate if exists */
	vboot_verify_op_cert(vbs);
	if (vbs->op_cert == 0 || vbs->op_cert_size == 0)
		return GIU_NONE;

	/* get giu_mode from verified operation certificate */
	return vboot_get_giu_mode_from_cert(vbs);
#else
	return GIU_NONE;
#endif
}

void vboot_reset(struct vbs *vbs)
{
	void *fp_timer = 0;
	void *fp_spi_check = 0;
	volatile struct vbs *current = (volatile struct vbs *)AST_SRAM_VBS_BASE;

	memset((void *)vbs, 0, sizeof(struct vbs));
	vbs->rom_exec_address = current->uboot_exec_address;
	vbs->recovery_retries = current->recovery_retries;
	vbs->rom_handoff = current->rom_handoff;
	if (vbs->recovery_retries >= 25) {
		/* Retries has hit an impossible upper bound, reset to 0. */
		vbs->recovery_retries = 0;
	}

	if (vbs->rom_handoff == VBS_HANDOFF) {
		printf("U-Boot failed to execute.\n");
		vboot_recovery(vbs, VBS_ERROR_TYPE_SPI,
			       VBS_ERROR_EXECUTE_FAILURE);
	}

	/* Keep track of requested and valid H/W enforce enable scenario. */
	bool should_lock = false;
	/* Verified boot is not possible if the SPL does not include a KEK. */
	const void *rom_fdt = (const void *)gd_fdt_blob();
	vbs->rom_keys = (u32)rom_fdt;
	if (rom_fdt == 0x0) {
		/* It is possible the spl_init method did not find a fdt. */
		printf("No signature store (KEK) was included in the SPL.\n");
		vboot_enforce(vbs, VBS_ERROR_TYPE_DATA, VBS_ERROR_NO_KEK);
	} else {
		/* Check if there is a signature subnode and the /hwlock path exists. */
		if (fdt_subnode_offset(rom_fdt, 0, FIT_SIG_NODENAME) >= 0 &&
		    fdt_getprop_u32(rom_fdt, 0, "hwlock") == 1) {
			should_lock = true;
		}
	}
	/* setup golden image WP# latch */
	int bsm_latched = vboot_setup_wp_latch(vbs, &fp_timer, &fp_spi_check);
	/* get golden image upgrading mode */
	int giu_mode = vboot_get_giu_mode(vbs, bsm_latched);
	/* check and lock hwstrap for AST2600 platform */
	set_and_lock_hwstrap(should_lock);
	/* Reset FMC SPI PROMs and check WP# for FMC SPI CS0. */
	int spi_status = ast_fmc_spi_check(should_lock, giu_mode, &fp_timer,
					   &fp_spi_check);
	/* The presence of WP# on FMC SPI CS0 determines hardware enforcement. */
	vbs->hardware_enforce = (spi_status == AST_FMC_WP_ON) ? 1 : 0;
	if (spi_status == AST_FMC_ERROR) {
		/* The QEMU models will always return SPI PROM errors. */
#ifndef CONFIG_DEBUG_QEMU
		vboot_recovery(vbs, VBS_ERROR_TYPE_SPI, VBS_ERROR_SPI_PROM);
#endif
	}

	/* set rom_handoff only if no valid handoff been set yet
	 *  which means this is a clean (re)boot:
	 *   a) AC-Power on
	 *   b) reboot from OpenBMC
	 */
	if ((vbs->rom_handoff != VBS_HANDOFF) &&
	    (vbs->rom_handoff != VBS_HANDOFF_TPM_SETUP) &&
	    (vbs->rom_handoff != VBS_HANDOFF_TPM_RST) &&
	    (vbs->rom_handoff != VBS_HANDOFF_SWAP)) {
		/* Set a handoff and expect U-Boot to clear indicating a clean boot. */
		vbs->recovery_retries = 0;
		vbs->rom_handoff = VBS_HANDOFF;
		vboot_store(vbs);
	}

#ifdef CONFIG_TEST_ASPEED_WATCHDOG_SPL
	printf("Testing SPL hang recovery ...\n");
	hang();
#endif

#ifdef CONFIG_ASPEED_TPM
	int tpm_status = ast_tpm_provision(vbs);
	if (tpm_status == VBS_ERROR_TPM_SETUP) {
		/* The TPM was not reset correctly */
		debug("TPM was not reset correctly, cur->rom_handoff=%d\n",
		      current->rom_handoff);
		if (current->rom_handoff != VBS_HANDOFF_TPM_SETUP) {
			vbs->rom_handoff = VBS_HANDOFF_TPM_SETUP;
			vboot_jump(0x0, vbs);
		}
	}

	if (tpm_status == VBS_ERROR_TPM_RESET_NEEDED) {
		if (current->rom_handoff == VBS_HANDOFF_TPM_RST) {
			/* The TPM needed a reset before, and needs another, this is a problem. */
			printf("TPM was deactivated and remains so after a reset.\n");
			vboot_enforce(vbs, VBS_ERROR_TYPE_TPM,
				      VBS_ERROR_TPM_RESET_NEEDED);
		} else {
			printf("TPM was deactivated and needs a reset.\n");
			vbs->rom_handoff = VBS_HANDOFF_TPM_RST;
			vboot_jump(0x0, vbs);
		}
	} else if (tpm_status != VBS_SUCCESS) {
		/* The TPM could not be provisioned. */
		vboot_enforce(vbs, VBS_ERROR_TYPE_TPM, tpm_status);
		return;
	}

	tpm_status = ast_tpm_owner_provision(vbs);
	if (tpm_status != VBS_SUCCESS) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_TPM, tpm_status);
		return;
	}

	/* Only attempt to provision the NV space if the TPM was provisioned. */
	tpm_status = ast_tpm_nv_provision(vbs);
	if (tpm_status != VBS_SUCCESS) {
		vboot_enforce(vbs, VBS_ERROR_TYPE_NV, tpm_status);
		return;
	}
#endif
}

void vboot_load_fit(volatile void *from)
{
	void *fit = (void *)from;

	/* Set the VBS structure to the expected location in SRAM. */
	struct vbs *vbs = (struct vbs *)malloc(sizeof(struct vbs));

	/* The AST comes out of reset so we check the previous state and SPI PROMs. */
	vboot_reset(vbs);

	/* The offset into the FIT containing signed configuration. */
	int config;
	/* The offset into the FIT containing U-Boot information. */
	int uboot;
	/* The offset after the FIT containing U-Boot. */
	u32 uboot_position;
	/* The size of U-Boot content following the FIT */
	u32 uboot_size;
	/* uboot hash extracted fro FIT */
	uint8_t uboot_hash[FIT_MAX_HASH_LEN];

	/* Check the sanity of the FIT. */
	vboot_check_fit(fit, vbs, &uboot, &config, &uboot_position,
			&uboot_size);

	/* Check for software enforcement and forced recovery */
	vbs->software_enforce = vboot_getenv_yesno("verify");
	vbs->force_recovery = vboot_getenv_yesno("force_recovery");
	if (vbs->force_recovery == 1) {
		vboot_recovery(vbs, VBS_ERROR_TYPE_FORCE,
			       VBS_ERROR_FORCE_RECOVERY);
	}

	if (fdt_subnode_offset((const void *)vbs->rom_keys, 0,
			       FIT_SIG_NODENAME) >= 0) {
		/* If the SPL contains a KEK then verification is enforced. */
		vbs->software_enforce = 1;
	}

	/* Verify subordinate keys kept in the FIT */
	vboot_verify_subordinate(fit, vbs);

	/* If verified boot is successful the next load is U-Boot. */
	void *load = (void *)((u32)from + uboot_position);

	/* Finally verify U-Boot using the subordinate store if verified. */
	vboot_verify_uboot(fit, vbs, load, config, uboot, uboot_size,
			   uboot_hash);

#ifdef CONFIG_ASPEED_TPM
	vboot_rollback_protection(fit, AST_TPM_ROLLBACK_UBOOT, vbs);
	if (vbs->hardware_enforce || vbs->software_enforce) {
		vboot_spl_do_measures(uboot_hash, FIT_MAX_HASH_LEN);
	}
#endif

	vboot_jump((volatile void *)load, vbs);
}

void board_init_f(ulong bootflag)
{
#if defined(CONFIG_ASPEED_AST2600)
	u32 fmc_ce_ctrl;
	/* stop the FMCWDT2 as common lowlevel init code leave FMCWDT2 handling
	 * to application. verified boot don't use FMCWDT2
	 */
	writel(0, ASPEED_FMC_WDT2);
	spl_early_init();
	timer_init();
	preloader_console_init();
	debug("SYS_INIT_RAM_END=%x \n", SYS_INIT_RAM_END);
	debug("CONFIG_SYS_INIT_SP_ADDR=%x\n", CONFIG_SYS_INIT_SP_ADDR);
	debug("CONFIG_MALLOC_F_ADDR=%x\n", CONFIG_MALLOC_F_ADDR);
	debug("gd = sp = %p\n", gd);
	debug("fdt=%p\n", gd->fdt_blob);
	debug("Setup flash: write enable, addr4B, CE1 AHB 64MB window\n");
	setbits_le32(ASPEED_FMC_BASE, (7 << 16));
	udelay(1000);
	if (IS_ENABLED(CONFIG_QEMU_BUILD)) {
		/* QEMU SPI flash always 3B before probing */
		clrbits_le32(ASPEED_FMC_BASE + 0x4, 0x33);
		udelay(1000);
		fmc_ce_ctrl = readl(ASPEED_FMC_BASE + 0x4);
		printf("!!!QEMU!!! FMC_CE_CTRL = 0x%08X\n", fmc_ce_ctrl);
	} else {
		/* H/W SPI flash can always use 4B before probing
		* Winbond: manufacturing set 4B to NV-status register
		* Macronix: accept 4B addr cmd w/o need explict EN4B
		*/
		setbits_le32(ASPEED_FMC_BASE + 0x4, 0x33);
		udelay(1000);
		fmc_ce_ctrl = readl(ASPEED_FMC_BASE + 0x4);
		debug("Setup FMC_CE_CTRL = 0x%08X\n", fmc_ce_ctrl);
	}
	writel(0x2BF02800, ASPEED_FMC_BASE + 0x34);
	udelay(1000);
#else /* CONFIG_ASPEED_AST2600 */
	/* Must set up console for printing/logging. */
	preloader_console_init();
	/* Must set up global data pointers for local device tree. */
	spl_init();

	void *sp;
	asm("mov %0, sp" : "=r"(sp) :);
	debug("gd = %p, sp = %p\n", gd, sp);
	debug("malloc_base = %lx, malloc_limit = %lx\n", gd->malloc_base,
	      gd->malloc_limit);
#endif
	watchdog_init(CONFIG_ASPEED_WATCHDOG_SPL_TIMEOUT);

	if (!pfr_checkpoint(0x01)) { // CHKPT_START
		printf("PFR: UFM provisioned\n");
		typedef void __noreturn (*image_entry_noargs_t)(void);
		image_entry_noargs_t image_entry =
			(image_entry_noargs_t)CONFIG_SYS_RECOVERY_BASE;
		image_entry();
	}
	vboot_load_fit((volatile void *)CONFIG_SYS_SPL_FIT_BASE);
	hang();
}

u32 spl_boot_device()
{
	/* Include this NOP symbol to use the SPL_FRAMEWORK APIs. */
	return BOOT_DEVICE_NONE;
}

void spl_display_print()
{
	/* Nothing */
}
