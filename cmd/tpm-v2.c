// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include "tpm-user-utils.h"

static int do_tpm2_startup(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	enum tpm2_startup_types mode;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;
	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcasecmp("TPM2_SU_CLEAR", argv[1])) {
		mode = TPM2_SU_CLEAR;
	} else if (!strcasecmp("TPM2_SU_STATE", argv[1])) {
		mode = TPM2_SU_STATE;
	} else {
		printf("Couldn't recognize mode string: %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return report_return_code(tpm2_startup(dev, mode));
}

static int do_tpm2_self_test(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	enum tpm2_yes_no full_test;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;
	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcasecmp("full", argv[1])) {
		full_test = TPMI_YES;
	} else if (!strcasecmp("continue", argv[1])) {
		full_test = TPMI_NO;
	} else {
		printf("Couldn't recognize test mode: %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return report_return_code(tpm2_self_test(dev, full_test));
}

static int do_tpm2_clear(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	u32 handle = 0;
	const char *pw = (argc < 3) ? NULL : argv[2];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	if (pw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	if (!strcasecmp("TPM2_RH_LOCKOUT", argv[1]))
		handle = TPM2_RH_LOCKOUT;
	else if (!strcasecmp("TPM2_RH_PLATFORM", argv[1]))
		handle = TPM2_RH_PLATFORM;
	else
		return CMD_RET_USAGE;

	return report_return_code(tpm2_clear(dev, handle, pw, pw_sz));
}

static int do_tpm2_pcr_extend(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	struct udevice *dev;
	struct tpm_chip_priv *priv;
	u32 index = simple_strtoul(argv[1], NULL, 0);
	void *digest = map_sysmem(simple_strtoul(argv[2], NULL, 0), 0);
	int ret;
	u32 rc;

	if (argc != 3)
		return CMD_RET_USAGE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -EINVAL;

	if (index >= priv->pcr_count)
		return -EINVAL;

	rc = tpm2_pcr_extend(dev, index, digest);

	unmap_sysmem(digest);

	return report_return_code(rc);
}

static int do_tpm_pcr_read(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	struct udevice *dev;
	struct tpm_chip_priv *priv;
	u32 index, rc;
	unsigned int updates;
	void *data;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -EINVAL;

	index = simple_strtoul(argv[1], NULL, 0);
	if (index >= priv->pcr_count)
		return -EINVAL;

	data = map_sysmem(simple_strtoul(argv[2], NULL, 0), 0);

	rc = tpm2_pcr_read(dev, index, priv->pcr_select_min, data, &updates);
	if (!rc) {
		printf("PCR #%u content (%u known updates):\n", index, updates);
		print_byte_string(data, TPM2_DIGEST_LEN);
	}

	unmap_sysmem(data);

	return report_return_code(rc);
}

static int do_tpm_get_capability(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	u32 capability, property, rc;
	u8 *data;
	size_t count;
	int i, j;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc != 5)
		return CMD_RET_USAGE;

	capability = simple_strtoul(argv[1], NULL, 0);
	property = simple_strtoul(argv[2], NULL, 0);
	data = map_sysmem(simple_strtoul(argv[3], NULL, 0), 0);
	count = simple_strtoul(argv[4], NULL, 0);

	rc = tpm2_get_capability(dev, capability, property, data, count);
	if (rc)
		goto unmap_data;

	printf("Capabilities read from TPM:\n");
	for (i = 0; i < count; i++) {
		printf("Property 0x");
		for (j = 0; j < 4; j++)
			printf("%02x", data[(i * 8) + j]);
		printf(": 0x");
		for (j = 4; j < 8; j++)
			printf("%02x", data[(i * 8) + j]);
		printf("\n");
	}

unmap_data:
	unmap_sysmem(data);

	return report_return_code(rc);
}

static int do_tpm_dam_reset(cmd_tbl_t *cmdtp, int flag, int argc,
			    char *const argv[])
{
	const char *pw = (argc < 2) ? NULL : argv[1];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (pw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	return report_return_code(tpm2_dam_reset(dev, pw, pw_sz));
}

static int do_tpm_dam_parameters(cmd_tbl_t *cmdtp, int flag, int argc,
				 char *const argv[])
{
	const char *pw = (argc < 5) ? NULL : argv[4];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	/*
	 * No Dictionary Attack Mitigation (DAM) means:
	 * maxtries = 0xFFFFFFFF, recovery_time = 1, lockout_recovery = 0
	 */
	unsigned long int max_tries;
	unsigned long int recovery_time;
	unsigned long int lockout_recovery;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 4 || argc > 5)
		return CMD_RET_USAGE;

	if (pw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	if (strict_strtoul(argv[1], 0, &max_tries))
		return CMD_RET_USAGE;

	if (strict_strtoul(argv[2], 0, &recovery_time))
		return CMD_RET_USAGE;

	if (strict_strtoul(argv[3], 0, &lockout_recovery))
		return CMD_RET_USAGE;

	log(LOGC_NONE, LOGL_INFO, "Changing dictionary attack parameters:\n");
	log(LOGC_NONE, LOGL_INFO, "- maxTries: %lu", max_tries);
	log(LOGC_NONE, LOGL_INFO, "- recoveryTime: %lu\n", recovery_time);
	log(LOGC_NONE, LOGL_INFO, "- lockoutRecovery: %lu\n", lockout_recovery);

	return report_return_code(tpm2_dam_parameters(dev, pw, pw_sz, max_tries,
						      recovery_time,
						      lockout_recovery));
}

static int do_tpm_change_auth(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	u32 handle;
	const char *newpw = argv[2];
	const char *oldpw = (argc == 3) ? NULL : argv[3];
	const ssize_t newpw_sz = strlen(newpw);
	const ssize_t oldpw_sz = oldpw ? strlen(oldpw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	if (newpw_sz > TPM2_DIGEST_LEN || oldpw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	if (!strcasecmp("TPM2_RH_LOCKOUT", argv[1]))
		handle = TPM2_RH_LOCKOUT;
	else if (!strcasecmp("TPM2_RH_ENDORSEMENT", argv[1]))
		handle = TPM2_RH_ENDORSEMENT;
	else if (!strcasecmp("TPM2_RH_OWNER", argv[1]))
		handle = TPM2_RH_OWNER;
	else if (!strcasecmp("TPM2_RH_PLATFORM", argv[1]))
		handle = TPM2_RH_PLATFORM;
	else
		return CMD_RET_USAGE;

	return report_return_code(tpm2_change_auth(dev, handle, newpw, newpw_sz,
						   oldpw, oldpw_sz));
}

static int do_tpm_pcr_setauthpolicy(cmd_tbl_t *cmdtp, int flag, int argc,
				    char * const argv[])
{
	u32 index = simple_strtoul(argv[1], NULL, 0);
	char *key = argv[2];
	const char *pw = (argc < 4) ? NULL : argv[3];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (strlen(key) != TPM2_DIGEST_LEN)
		return -EINVAL;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	return report_return_code(tpm2_pcr_setauthpolicy(dev, pw, pw_sz, index,
							 key));
}

static int do_tpm_pcr_setauthvalue(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	u32 index = simple_strtoul(argv[1], NULL, 0);
	char *key = argv[2];
	const ssize_t key_sz = strlen(key);
	const char *pw = (argc < 4) ? NULL : argv[3];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (strlen(key) != TPM2_DIGEST_LEN)
		return -EINVAL;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	return report_return_code(tpm2_pcr_setauthvalue(dev, pw, pw_sz, index,
							key, key_sz));
}

static int do_tpm2_nv_define(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	u32 nv_index = simple_strtoul(argv[1], NULL, 0);
	u32 data_sz = simple_strtoul(argv[2], NULL, 0);
	char* pw = NULL;
	u32 pw_sz = 0;
	struct udevice *dev;
	int ret;

	u32 attri = TPMA_NV_PPREAD | TPMA_NV_PPWRITE | TPMA_NV_PLATFORMCREATE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	if (argc == 4)
	{
		pw = argv[3];
		pw_sz = strlen(pw);
	}

	nv_index |= (TPM_HT_NV_INDEX<<24);
	return report_return_code(tpm2_nv_definespace(dev, (u8*)pw, pw_sz,
		TPM2_RH_PLATFORM, nv_index, TPM2_ALG_SHA256, attri, data_sz));
}

static int do_tpm2_nv_write(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	u32 nv_index, offset, length;
	char* data = NULL;
	char* pw = NULL;
	u32 pw_sz = 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 4 || argc > 5)
		return CMD_RET_USAGE;

	nv_index = simple_strtoul(argv[1], NULL, 0);
	offset = simple_strtoul(argv[2], NULL, 0);
	data = argv[3];
	length = strlen(data);

	if (argc == 5)
	{
		pw = argv[4];
		pw_sz = strlen(pw);
	}

	nv_index |= (TPM_HT_NV_INDEX<<24);
	return report_return_code(tpm2_nv_write(dev, (u8*)pw, pw_sz,
		TPM2_RH_PLATFORM, nv_index, (u8*)data, length, offset));
}

static int do_tpm2_nv_read(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	u32 nv_index, offset, length;
	char data[256] = {0};
	char* pw = NULL;
	u32 pw_sz = 0;
	struct udevice *dev;
	int ret;
	u32 rc;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 4 || argc > 5)
		return CMD_RET_USAGE;

	nv_index = simple_strtoul(argv[1], NULL, 0);
	offset = simple_strtoul(argv[2], NULL, 0);
	length = simple_strtoul(argv[3], NULL, 0);

	if (argc == 5)
	{
		pw = argv[4];
		pw_sz = strlen(pw);
	}

	nv_index |= (TPM_HT_NV_INDEX<<24);
	rc = tpm2_nv_read(dev, (u8*)pw, pw_sz, TPM2_RH_PLATFORM,
				nv_index, length, offset, (u8*)data);

	if (rc == TPM2_RC_SUCCESS)
		printf("[0x%08x]: %s\n", nv_index, data);

	return report_return_code(rc);
}

static int do_tpm2_hierarchy_control(cmd_tbl_t *cmdtp, int flag,
					int argc, char * const argv[])
{
	char* pw = NULL;
	u32 pw_sz = 0;
	u32 res_handle;
	bool set = false;
	struct udevice *dev;
	int ret;
	u32 rc;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	if (argc == 4)
	{
		pw = argv[4];
		pw_sz = strlen(pw);
	}

	if (!strcasecmp("TPM2_RH_OWNER", argv[1]))
		res_handle = TPM2_RH_OWNER;
	else if (!strcasecmp("TPM2_RH_ENDORSEMENT", argv[1]))
		res_handle = TPM2_RH_ENDORSEMENT;
	else if (!strcasecmp("TPM2_RH_PLATFORM", argv[1]))
		res_handle = TPM2_RH_PLATFORM;
	else if (!strcasecmp("TPM2_RH_PLATFORM_NV", argv[1]))
		res_handle = TPM2_RH_PLATFORM_NV;
	else
		return CMD_RET_USAGE;

	if (!strcasecmp("YES", argv[2]) || !strcasecmp("Y", argv[2]) ||
		!strcasecmp("SET", argv[2]) || !strcasecmp("S", argv[2]) ||
		!strcasecmp("TRUE", argv[2]) || !strcasecmp("T", argv[2]))
		set = true;
	else if (!strcasecmp("NO", argv[2]) || !strcasecmp("N", argv[2]) ||
		!strcasecmp("CLEAR", argv[2]) || !strcasecmp("C", argv[2]) ||
		!strcasecmp("FALSE", argv[2]) || !strcasecmp("F", argv[2]))
		set = false;
	else
		return CMD_RET_USAGE;

	rc = tpm2_hierarchy_control(dev, (u8*)pw, pw_sz, TPM2_RH_PLATFORM,
				res_handle, set);

	return report_return_code(rc);
}

static int do_tpm2_nv_readpublic(cmd_tbl_t *cmdtp, int flag,
					int argc, char * const argv[])
{
	u32 nv_index = 0;
	u16 hash_alg, policy_sz, data_sz, nv_name_sz;
	u32 attributes;
	u8 policy[TPM2_DIGEST_LEN] = {0};
	u8 nv_name[TPM2_DIGEST_LEN] = {0};

	struct udevice *dev;
	int ret;
	u32 rc;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 1 || argc > 2)
		return CMD_RET_USAGE;

	if (argc == 1)
	{
		printf("read all not support yet\n");
		return ret;
	}
	else if (argc == 2)
	{
		nv_index = simple_strtoul(argv[1], NULL, 0);
		nv_index |= (TPM_HT_NV_INDEX<<24);
	}

	rc = tpm2_nv_readpublic(dev, nv_index,
					&hash_alg, &attributes, &policy_sz, policy,
					&data_sz, &nv_name_sz, nv_name);

	if (rc == TPM2_RC_SUCCESS)
	{
		printf("0x%08X:\n", nv_index);
		printf("name: \n");
		print_byte_string(nv_name, nv_name_sz);

		printf("hash algorithm: 0x%04X ", hash_alg);
		if (hash_alg == TPM2_ALG_SHA256)
			printf("(SHA256)\n");
		else
			printf("\n");

		printf("attributes: 0x%08X\n", (unsigned int)attributes);

		printf("policy: \n");
		print_byte_string(policy, policy_sz);

		printf("size: %u\n", data_sz);
	}

	return report_return_code(rc);
}

static int do_tpm2_nv_undefinespace(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	u32 nv_index = simple_strtoul(argv[1], NULL, 0);
	char* pw = NULL;
	u32 pw_sz = 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	if (argc == 3)
	{
		pw = argv[2];
		pw_sz = strlen(pw);
	}

	nv_index |= (TPM_HT_NV_INDEX<<24);
	return report_return_code(tpm2_nv_undefinespace(dev, (u8*)pw, pw_sz,
		TPM2_RH_PLATFORM, nv_index));
}

static cmd_tbl_t tpm2_commands[] = {
	U_BOOT_CMD_MKENT(info, 0, 1, do_tpm_info, "", ""),
	U_BOOT_CMD_MKENT(init, 0, 1, do_tpm_init, "", ""),
	U_BOOT_CMD_MKENT(startup, 0, 1, do_tpm2_startup, "", ""),
	U_BOOT_CMD_MKENT(self_test, 0, 1, do_tpm2_self_test, "", ""),
	U_BOOT_CMD_MKENT(clear, 0, 1, do_tpm2_clear, "", ""),
	U_BOOT_CMD_MKENT(pcr_extend, 0, 1, do_tpm2_pcr_extend, "", ""),
	U_BOOT_CMD_MKENT(pcr_read, 0, 1, do_tpm_pcr_read, "", ""),
	U_BOOT_CMD_MKENT(get_capability, 0, 1, do_tpm_get_capability, "", ""),
	U_BOOT_CMD_MKENT(dam_reset, 0, 1, do_tpm_dam_reset, "", ""),
	U_BOOT_CMD_MKENT(dam_parameters, 0, 1, do_tpm_dam_parameters, "", ""),
	U_BOOT_CMD_MKENT(change_auth, 0, 1, do_tpm_change_auth, "", ""),
	U_BOOT_CMD_MKENT(pcr_setauthpolicy, 0, 1,
			 do_tpm_pcr_setauthpolicy, "", ""),
	U_BOOT_CMD_MKENT(pcr_setauthvalue, 0, 1,
			 do_tpm_pcr_setauthvalue, "", ""),
	U_BOOT_CMD_MKENT(nv_define, 0, 1, do_tpm2_nv_define, "", ""),
	U_BOOT_CMD_MKENT(nv_write, 0, 1, do_tpm2_nv_write, "", ""),
	U_BOOT_CMD_MKENT(nv_read, 0, 1, do_tpm2_nv_read, "", ""),
	U_BOOT_CMD_MKENT(hier_ctrl, 0, 1, do_tpm2_hierarchy_control, "", ""),
	U_BOOT_CMD_MKENT(nv_readpublic, 0, 1, do_tpm2_nv_readpublic, "", ""),
	U_BOOT_CMD_MKENT(nv_undefine, 0, 1, do_tpm2_nv_undefinespace, "", ""),
};

cmd_tbl_t *get_tpm2_commands(unsigned int *size)
{
	*size = ARRAY_SIZE(tpm2_commands);

	return tpm2_commands;
}

U_BOOT_CMD(tpm2, CONFIG_SYS_MAXARGS, 1, do_tpm, "Issue a TPMv2.x command",
"<command> [<arguments>]\n"
"\n"
"info\n"
"    Show information about the TPM.\n"
"init\n"
"    Initialize the software stack. Always the first command to issue.\n"
"startup <mode>\n"
"    Issue a TPM2_Startup command.\n"
"    <mode> is one of:\n"
"        * TPM2_SU_CLEAR (reset state)\n"
"        * TPM2_SU_STATE (preserved state)\n"
"self_test <type>\n"
"    Test the TPM capabilities.\n"
"    <type> is one of:\n"
"        * full (perform all tests)\n"
"        * continue (only check untested tests)\n"
"clear <hierarchy>\n"
"    Issue a TPM2_Clear command.\n"
"    <hierarchy> is one of:\n"
"        * TPM2_RH_LOCKOUT\n"
"        * TPM2_RH_PLATFORM\n"
"pcr_extend <pcr> <digest_addr>\n"
"    Extend PCR #<pcr> with digest at <digest_addr>.\n"
"    <pcr>: index of the PCR\n"
"    <digest_addr>: address of a 32-byte SHA256 digest\n"
"pcr_read <pcr> <digest_addr>\n"
"    Read PCR #<pcr> to memory address <digest_addr>.\n"
"    <pcr>: index of the PCR\n"
"    <digest_addr>: address to store the a 32-byte SHA256 digest\n"
"get_capability <capability> <property> <addr> <count>\n"
"    Read and display <count> entries indexed by <capability>/<property>.\n"
"    Values are 4 bytes long and are written at <addr>.\n"
"    <capability>: capability\n"
"    <property>: property\n"
"    <addr>: address to store <count> entries of 4 bytes\n"
"    <count>: number of entries to retrieve\n"
"dam_reset [<password>]\n"
"    If the TPM is not in a LOCKOUT state, reset the internal error counter.\n"
"    <password>: optional password\n"
"dam_parameters <max_tries> <recovery_time> <lockout_recovery> [<password>]\n"
"    If the TPM is not in a LOCKOUT state, set the DAM parameters\n"
"    <maxTries>: maximum number of failures before lockout,\n"
"                0 means always locking\n"
"    <recoveryTime>: time before decrement of the error counter,\n"
"                    0 means no lockout\n"
"    <lockoutRecovery>: time of a lockout (before the next try),\n"
"                       0 means a reboot is needed\n"
"    <password>: optional password of the LOCKOUT hierarchy\n"
"change_auth <hierarchy> <new_pw> [<old_pw>]\n"
"    <hierarchy>: the hierarchy\n"
"    <new_pw>: new password for <hierarchy>\n"
"    <old_pw>: optional previous password of <hierarchy>\n"
"pcr_setauthpolicy|pcr_setauthvalue <pcr> <key> [<password>]\n"
"    Change the <key> to access PCR #<pcr>.\n"
"    hierarchy and may be empty.\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <pcr>: index of the PCR\n"
"    <key>: secret to protect the access of PCR #<pcr>\n"
"    <password>: optional password of the PLATFORM hierarchy\n"
"nv_define <nv_index> <data_size> [<password>]\n"
"    Experimental: Implementation of TPM2_NV_DefineSpace command\n"
"    hierarchy: platform\n"
"    password: 	null password\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <nv_index>: index of the NVRAM\n"
"    <data_size>: required data size\n"
"    <password>: optional password of the PLATFORM hierarchy\n"
"nv_write <nv_index> <offset> <data_str> [<passward>]\n"
"    Experimental: Implementation of TPM2_NV_Write command\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <nv_index>: index of the NVRAM\n"
"    <offset>: offset into the area"
"    <data_str>: data string \n"
"    <password>: optional password of the PLATFORM hierarchy\n"
"nv_read <nv_index> <offset> <length> [<password>]\n"
"    Experimental: Implementation of TPM2_NV_Read command\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <nv_index>: index of the NVRAM\n"
"    <offset>: offset into the area"
"    <length>: number of reading bytes\n"
"    <password>: optional password of the PLATFORM hierarchy\n"
"nv_readpublic <nv_index>\n"
"    Experimental: Implementation of TPM2_NV_ReadPublic command\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <nv_index>: index of the NVRAM\n"
"nv_undefine <nv_index> [<password>]\n"
"    Experimental: Implementation of TPM2_NV_UndefineSpace command\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <nv_index>: index of the NVRAM\n"
"    <password>: optional password of the PLATFORM hierarchy\n"
"hier_ctrl <res_handle> <set_or_clear> [<password>]\n"
"    Experimental: Implementation of TPM_HierarchyControl command\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <res_handle>: resource handle (case insensitive)\n"
"        * TPM2_RH_OWNER\n"
"        * TPM2_RH_ENDORSEMENT\n"
"        * TPM2_RH_PLATFORM\n"
"        * TPM2_RH_PLATFORM_NV\n"
"    <set_or_clear>: set or clear resource handle (case insensitive)\n"
"        * set: Y|YES|T|TRUE|S|SET\n"
"        * clear: N|NO|F|FALSE|C|CLEAR\n"
);
