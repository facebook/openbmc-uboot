// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Aspeed Technology Inc.
 */

#include <stdlib.h>
#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <inttypes.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <u-boot/sha256.h>
#include "otp_info.h"

DECLARE_GLOBAL_DATA_PTR;

#define OTP_VER				"1.1.0"

#define OTP_PASSWD			0x349fe38a
#define RETRY				20
#define OTP_REGION_STRAP		BIT(0)
#define OTP_REGION_CONF			BIT(1)
#define OTP_REGION_DATA			BIT(2)

#define OTP_USAGE			-1
#define OTP_FAILURE			-2
#define OTP_SUCCESS			0

#define OTP_PROG_SKIP			1

#define OTP_KEY_TYPE_RSA_PUB		1
#define OTP_KEY_TYPE_RSA_PRIV		2
#define OTP_KEY_TYPE_AES		3
#define OTP_KEY_TYPE_VAULT		4
#define OTP_KEY_TYPE_HMAC		5

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

#define OTP_BASE		0x1e6f2000
#define OTP_PROTECT_KEY		OTP_BASE
#define OTP_COMMAND		OTP_BASE + 0x4
#define OTP_TIMING		OTP_BASE + 0x8
#define OTP_ADDR		OTP_BASE + 0x10
#define OTP_STATUS		OTP_BASE + 0x14
#define OTP_COMPARE_1		OTP_BASE + 0x20
#define OTP_COMPARE_2		OTP_BASE + 0x24
#define OTP_COMPARE_3		OTP_BASE + 0x28
#define OTP_COMPARE_4		OTP_BASE + 0x2c

#define OTP_MAGIC		"SOCOTP"
#define CHECKSUM_LEN		32
#define OTP_INC_DATA		BIT(31)
#define OTP_INC_CONFIG		BIT(30)
#define OTP_INC_STRAP		BIT(29)
#define OTP_ECC_EN		BIT(28)
#define OTP_REGION_SIZE(info)	((info >> 16) & 0xffff)
#define OTP_REGION_OFFSET(info)	(info & 0xffff)
#define OTP_IMAGE_SIZE(info)	(info & 0xffff)

#define OTP_A0		0
#define OTP_A1		1
#define OTP_A2		2
#define OTP_A3		3

#define ID0_AST2600A0	0x05000303
#define ID1_AST2600A0	0x05000303
#define ID0_AST2600A1	0x05010303
#define ID1_AST2600A1	0x05010203
#define ID0_AST2600A2	0x05010303
#define ID1_AST2600A2	0x05020303
#define ID0_AST2600A3	0x05030303
#define ID1_AST2600A3	0x05030303
#define ID0_AST2620A1	0x05010203
#define ID1_AST2620A1	0x05010203
#define ID0_AST2620A2	0x05010203
#define ID1_AST2620A2	0x05020203
#define ID0_AST2620A3	0x05030203
#define ID1_AST2620A3	0x05030203
#define ID0_AST2620A3	0x05030203
#define ID1_AST2620A3	0x05030203
#define ID0_AST2605A2	0x05010103
#define ID1_AST2605A2	0x05020103
#define ID0_AST2605A3	0x05030103
#define ID1_AST2605A3	0x05030103
#define ID0_AST2625A3	0x05030403
#define ID1_AST2625A3	0x05030403

struct otp_header {
	u8	otp_magic[8];
	u8	otp_version[8];
	u32	image_info;
	u32	data_info;
	u32	config_info;
	u32	strap_info;
	u32	checksum_offset;
} __packed;

struct otpstrap_status {
	int value;
	int option_array[7];
	int remain_times;
	int writeable_option;
	int reg_protected;
	int protected;
};

struct otpconf_parse {
	int dw_offset;
	int bit;
	int length;
	int value;
	int ignore;
	char status[80];
};

struct otpkey_type {
	int value;
	int key_type;
	int need_id;
	char information[110];
};

struct otp_info_cb {
	int version;
	char ver_name[3];
	const struct otpstrap_info *strap_info;
	int strap_info_len;
	const struct otpconf_info *conf_info;
	int conf_info_len;
	const struct otpkey_type *key_info;
	int key_info_len;
};

struct otp_image_layout {
	int data_length;
	int conf_length;
	int strap_length;
	u8 *data;
	u8 *data_ignore;
	u8 *conf;
	u8 *conf_ignore;
	u8 *strap;
	u8 *strap_reg_pro;
	u8 *strap_pro;
	u8 *strap_ignore;
};

static struct otp_info_cb info_cb;

static const struct otpkey_type a0_key_type[] = {
	{0, OTP_KEY_TYPE_AES,   0, "AES-256 as OEM platform key for image encryption/decryption"},
	{1, OTP_KEY_TYPE_VAULT, 0, "AES-256 as secret vault key"},
	{4, OTP_KEY_TYPE_HMAC,  1, "HMAC as encrypted OEM HMAC keys in Mode 1"},
	{8, OTP_KEY_TYPE_RSA_PUB,   1, "RSA-public as OEM DSS public keys in Mode 2"},
	{9, OTP_KEY_TYPE_RSA_PUB,   0, "RSA-public as SOC public key"},
	{10, OTP_KEY_TYPE_RSA_PUB,  0, "RSA-public as AES key decryption key"},
	{13, OTP_KEY_TYPE_RSA_PRIV,  0, "RSA-private as SOC private key"},
	{14, OTP_KEY_TYPE_RSA_PRIV,  0, "RSA-private as AES key decryption key"},
};

static const struct otpkey_type a1_key_type[] = {
	{1, OTP_KEY_TYPE_VAULT, 0, "AES-256 as secret vault key"},
	{2, OTP_KEY_TYPE_AES,   1, "AES-256 as OEM platform key for image encryption/decryption in Mode 2 or AES-256 as OEM DSS keys for Mode GCM"},
	{8, OTP_KEY_TYPE_RSA_PUB,   1, "RSA-public as OEM DSS public keys in Mode 2"},
	{10, OTP_KEY_TYPE_RSA_PUB,  0, "RSA-public as AES key decryption key"},
	{14, OTP_KEY_TYPE_RSA_PRIV,  0, "RSA-private as AES key decryption key"},
};

static const struct otpkey_type a2_key_type[] = {
	{1, OTP_KEY_TYPE_VAULT, 0, "AES-256 as secret vault key"},
	{2, OTP_KEY_TYPE_AES,   1, "AES-256 as OEM platform key for image encryption/decryption in Mode 2 or AES-256 as OEM DSS keys for Mode GCM"},
	{8, OTP_KEY_TYPE_RSA_PUB,   1, "RSA-public as OEM DSS public keys in Mode 2"},
	{10, OTP_KEY_TYPE_RSA_PUB,  0, "RSA-public as AES key decryption key"},
	{14, OTP_KEY_TYPE_RSA_PRIV,  0, "RSA-private as AES key decryption key"},
};

static const struct otpkey_type a3_key_type[] = {
	{1, OTP_KEY_TYPE_VAULT, 0, "AES-256 as secret vault key"},
	{2, OTP_KEY_TYPE_AES,   1, "AES-256 as OEM platform key for image encryption/decryption in Mode 2 or AES-256 as OEM DSS keys for Mode GCM"},
	{8, OTP_KEY_TYPE_RSA_PUB,   1, "RSA-public as OEM DSS public keys in Mode 2"},
	{9, OTP_KEY_TYPE_RSA_PUB,   1, "RSA-public as OEM DSS public keys in Mode 2(big endian)"},
	{10, OTP_KEY_TYPE_RSA_PUB,  0, "RSA-public as AES key decryption key"},
	{11, OTP_KEY_TYPE_RSA_PUB,  0, "RSA-public as AES key decryption key(big endian)"},
	{12, OTP_KEY_TYPE_RSA_PRIV,  0, "RSA-private as AES key decryption key"},
	{13, OTP_KEY_TYPE_RSA_PRIV,  0, "RSA-private as AES key decryption key(big endian)"},
};

static void buf_print(u8 *buf, int len)
{
	int i;

	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	for (i = 0; i < len; i++) {
		if (i % 16 == 0)
			printf("%04X: ", i);
		printf("%02X ", buf[i]);
		if ((i + 1) % 16 == 0)
			printf("\n");
	}
}

static int get_dw_bit(u32 *rid, int offset)
{
	int bit_offset;
	int i;

	if (offset < 32) {
		i = 0;
		bit_offset = offset;
	} else {
		i = 1;
		bit_offset = offset - 32;
	}
	if ((rid[i] >> bit_offset) & 0x1)
		return 1;
	else
		return 0;
}

static int get_rid_num(u32 *rid)
{
	int i;
	int fz = 0;
	int rid_num = 0;
	int ret = 0;

	for (i = 0; i < 64; i++) {
		if (get_dw_bit(rid, i) == 0) {
			if (!fz)
				fz = 1;

		} else {
			rid_num++;
			if (fz)
				ret = OTP_FAILURE;
		}
	}
	if (ret)
		return ret;

	return rid_num;
}

static u32 chip_version(void)
{
	u32 revid0, revid1;

	revid0 = readl(ASPEED_REVISION_ID0);
	revid1 = readl(ASPEED_REVISION_ID1);

	if (revid0 == ID0_AST2600A0 && revid1 == ID1_AST2600A0) {
		/* AST2600-A0 */
		return OTP_A0;
	} else if (revid0 == ID0_AST2600A1 && revid1 == ID1_AST2600A1) {
		/* AST2600-A1 */
		return OTP_A1;
	} else if (revid0 == ID0_AST2600A2 && revid1 == ID1_AST2600A2) {
		/* AST2600-A2 */
		return OTP_A2;
	} else if (revid0 == ID0_AST2600A3 && revid1 == ID1_AST2600A3) {
		/* AST2600-A3 */
		return OTP_A3;
	} else if (revid0 == ID0_AST2620A1 && revid1 == ID1_AST2620A1) {
		/* AST2620-A1 */
		return OTP_A1;
	} else if (revid0 == ID0_AST2620A2 && revid1 == ID1_AST2620A2) {
		/* AST2620-A2 */
		return OTP_A2;
	} else if (revid0 == ID0_AST2620A3 && revid1 == ID1_AST2620A3) {
		/* AST2620-A3 */
		return OTP_A3;
	} else if (revid0 == ID0_AST2605A2 && revid1 == ID1_AST2605A2) {
		/* AST2605-A2 */
		return OTP_A2;
	} else if (revid0 == ID0_AST2605A3 && revid1 == ID1_AST2605A3) {
		/* AST2605-A3 */
		return OTP_A3;
	} else if (revid0 == ID0_AST2625A3 && revid1 == ID1_AST2625A3) {
		/* AST2605-A3 */
		return OTP_A3;
	}
	return OTP_FAILURE;
}

static void wait_complete(void)
{
	int reg;

	do {
		reg = readl(OTP_STATUS);
	} while ((reg & 0x6) != 0x6);
}

static void otp_write(u32 otp_addr, u32 data)
{
	writel(otp_addr, OTP_ADDR); //write address
	writel(data, OTP_COMPARE_1); //write data
	writel(0x23b1e362, OTP_COMMAND); //write command
	wait_complete();
}

static void otp_soak(int soak)
{
	if (info_cb.version == OTP_A2 || info_cb.version == OTP_A3) {
		switch (soak) {
		case 0: //default
			otp_write(0x3000, 0x0210); // Write MRA
			otp_write(0x5000, 0x2000); // Write MRB
			otp_write(0x1000, 0x0); // Write MR
			break;
		case 1: //normal program
			otp_write(0x3000, 0x1200); // Write MRA
			otp_write(0x5000, 0x107F); // Write MRB
			otp_write(0x1000, 0x1024); // Write MR
			writel(0x04191388, OTP_TIMING); // 200us
			break;
		case 2: //soak program
			otp_write(0x3000, 0x1220); // Write MRA
			otp_write(0x5000, 0x2074); // Write MRB
			otp_write(0x1000, 0x08a4); // Write MR
			writel(0x04193a98, OTP_TIMING); // 600us
			break;
		}
	} else {
		switch (soak) {
		case 0: //default
			otp_write(0x3000, 0x0); // Write MRA
			otp_write(0x5000, 0x0); // Write MRB
			otp_write(0x1000, 0x0); // Write MR
			break;
		case 1: //normal program
			otp_write(0x3000, 0x4021); // Write MRA
			otp_write(0x5000, 0x302f); // Write MRB
			otp_write(0x1000, 0x4020); // Write MR
			writel(0x04190760, OTP_TIMING); // 75us
			break;
		case 2: //soak program
			otp_write(0x3000, 0x4021); // Write MRA
			otp_write(0x5000, 0x1027); // Write MRB
			otp_write(0x1000, 0x4820); // Write MR
			writel(0x041930d4, OTP_TIMING); // 500us
			break;
		}
	}

	wait_complete();
}

static void otp_read_data(u32 offset, u32 *data)
{
	writel(offset, OTP_ADDR); //Read address
	writel(0x23b1e361, OTP_COMMAND); //trigger read
	wait_complete();
	data[0] = readl(OTP_COMPARE_1);
	data[1] = readl(OTP_COMPARE_2);
}

static void otp_read_conf(u32 offset, u32 *data)
{
	int config_offset;

	config_offset = 0x800;
	config_offset |= (offset / 8) * 0x200;
	config_offset |= (offset % 8) * 0x2;

	writel(config_offset, OTP_ADDR);  //Read address
	writel(0x23b1e361, OTP_COMMAND); //trigger read
	wait_complete();
	data[0] = readl(OTP_COMPARE_1);
}

static int otp_compare(u32 otp_addr, u32 addr)
{
	u32 ret;
	u32 *buf;

	buf = map_physmem(addr, 16, MAP_WRBACK);
	printf("%08X\n", buf[0]);
	printf("%08X\n", buf[1]);
	printf("%08X\n", buf[2]);
	printf("%08X\n", buf[3]);
	writel(otp_addr, OTP_ADDR); //Compare address
	writel(buf[0], OTP_COMPARE_1); //Compare data 1
	writel(buf[1], OTP_COMPARE_2); //Compare data 2
	writel(buf[2], OTP_COMPARE_3); //Compare data 3
	writel(buf[3], OTP_COMPARE_4); //Compare data 4
	writel(0x23b1e363, OTP_COMMAND); //Compare command
	wait_complete();
	ret = readl(OTP_STATUS); //Compare command
	if (ret & 0x1)
		return OTP_SUCCESS;
	else
		return OTP_FAILURE;
}

static int verify_bit(u32 otp_addr, int bit_offset, int value)
{
	u32 ret[2];

	if (otp_addr % 2 == 0)
		writel(otp_addr, OTP_ADDR); //Read address
	else
		writel(otp_addr - 1, OTP_ADDR); //Read address

	writel(0x23b1e361, OTP_COMMAND); //trigger read
	wait_complete();
	ret[0] = readl(OTP_COMPARE_1);
	ret[1] = readl(OTP_COMPARE_2);

	if (otp_addr % 2 == 0) {
		if (((ret[0] >> bit_offset) & 1) == value)
			return OTP_SUCCESS;
		else
			return OTP_FAILURE;
	} else {
		if (((ret[1] >> bit_offset) & 1) == value)
			return OTP_SUCCESS;
		else
			return OTP_FAILURE;
	}
}

static u32 verify_dw(u32 otp_addr, u32 *value, u32 *ignore, u32 *compare, int size)
{
	u32 ret[2];

	otp_addr &= ~(1 << 15);

	if (otp_addr % 2 == 0)
		writel(otp_addr, OTP_ADDR); //Read address
	else
		writel(otp_addr - 1, OTP_ADDR); //Read address
	writel(0x23b1e361, OTP_COMMAND); //trigger read
	wait_complete();
	ret[0] = readl(OTP_COMPARE_1);
	ret[1] = readl(OTP_COMPARE_2);
	if (size == 1) {
		if (otp_addr % 2 == 0) {
			// printf("check %x : %x = %x\n", otp_addr, ret[0], value[0]);
			if ((value[0] & ~ignore[0]) == (ret[0] & ~ignore[0])) {
				compare[0] = 0;
				return OTP_SUCCESS;
			}
			compare[0] = value[0] ^ ret[0];
			return OTP_FAILURE;

		} else {
			// printf("check %x : %x = %x\n", otp_addr, ret[1], value[0]);
			if ((value[0] & ~ignore[0]) == (ret[1] & ~ignore[0])) {
				compare[0] = ~0;
				return OTP_SUCCESS;
			}
			compare[0] = ~(value[0] ^ ret[1]);
			return OTP_FAILURE;
		}
	} else if (size == 2) {
		// otp_addr should be even
		if ((value[0] & ~ignore[0]) == (ret[0] & ~ignore[0]) && (value[1] & ~ignore[1]) == (ret[1] & ~ignore[1])) {
			// printf("check[0] %x : %x = %x\n", otp_addr, ret[0], value[0]);
			// printf("check[1] %x : %x = %x\n", otp_addr, ret[1], value[1]);
			compare[0] = 0;
			compare[1] = ~0;
			return OTP_SUCCESS;
		}
		// printf("check[0] %x : %x = %x\n", otp_addr, ret[0], value[0]);
		// printf("check[1] %x : %x = %x\n", otp_addr, ret[1], value[1]);
		compare[0] = value[0] ^ ret[0];
		compare[1] = ~(value[1] ^ ret[1]);
		return OTP_FAILURE;
	} else {
		return OTP_FAILURE;
	}
}

static void otp_prog(u32 otp_addr, u32 prog_bit)
{
	otp_write(0x0, prog_bit);
	writel(otp_addr, OTP_ADDR); //write address
	writel(prog_bit, OTP_COMPARE_1); //write data
	writel(0x23b1e364, OTP_COMMAND); //write command
	wait_complete();
}

static void _otp_prog_bit(u32 value, u32 prog_address, u32 bit_offset)
{
	int prog_bit;

	if (prog_address % 2 == 0) {
		if (value)
			prog_bit = ~(0x1 << bit_offset);
		else
			return;
	} else {
		if (info_cb.version != OTP_A3)
			prog_address |= 1 << 15;
		if (!value)
			prog_bit = 0x1 << bit_offset;
		else
			return;
	}
	otp_prog(prog_address, prog_bit);
}

static int otp_prog_dc_b(u32 value, u32 prog_address, u32 bit_offset)
{
	int pass;
	int i;

	otp_soak(1);
	_otp_prog_bit(value, prog_address, bit_offset);
	pass = 0;

	for (i = 0; i < RETRY; i++) {
		if (verify_bit(prog_address, bit_offset, value) != 0) {
			otp_soak(2);
			_otp_prog_bit(value, prog_address, bit_offset);
			if (verify_bit(prog_address, bit_offset, value) != 0) {
				otp_soak(1);
			} else {
				pass = 1;
				break;
			}
		} else {
			pass = 1;
			break;
		}
	}
	if (pass)
		return OTP_SUCCESS;

	return OTP_FAILURE;
}

static void otp_prog_dw(u32 value, u32 ignore, u32 prog_address)
{
	int j, bit_value, prog_bit;

	for (j = 0; j < 32; j++) {
		if ((ignore >> j) & 0x1)
			continue;
		bit_value = (value >> j) & 0x1;
		if (prog_address % 2 == 0) {
			if (bit_value)
				prog_bit = ~(0x1 << j);
			else
				continue;
		} else {
			if (info_cb.version != OTP_A3)
				prog_address |= 1 << 15;
			if (bit_value)
				continue;
			else
				prog_bit = 0x1 << j;
		}
		otp_prog(prog_address, prog_bit);
	}
}

static int otp_prog_verify_2dw(u32 *data, u32 *buf, u32 *ignore_mask, u32 prog_address)
{
	int pass;
	int i;
	u32 data0_masked;
	u32 data1_masked;
	u32 buf0_masked;
	u32 buf1_masked;
	u32 compare[2];

	data0_masked = data[0]  & ~ignore_mask[0];
	buf0_masked  = buf[0] & ~ignore_mask[0];
	data1_masked = data[1]  & ~ignore_mask[1];
	buf1_masked  = buf[1] & ~ignore_mask[1];
	if (data0_masked == buf0_masked && data1_masked == buf1_masked)
		return OTP_SUCCESS;

	otp_soak(1);
	if (data0_masked != buf0_masked)
		otp_prog_dw(buf[0], ignore_mask[0], prog_address);
	if (data1_masked != buf1_masked)
		otp_prog_dw(buf[1], ignore_mask[1], prog_address + 1);

	pass = 0;
	for (i = 0; i < RETRY; i++) {
		if (verify_dw(prog_address, buf, ignore_mask, compare, 2) != 0) {
			otp_soak(2);
			if (compare[0] != 0)
				otp_prog_dw(compare[0], ignore_mask[0], prog_address);
			if (compare[1] != ~0)
				otp_prog_dw(compare[1], ignore_mask[1], prog_address + 1);
			if (verify_dw(prog_address, buf, ignore_mask, compare, 2) != 0) {
				otp_soak(1);
			} else {
				pass = 1;
				break;
			}
		} else {
			pass = 1;
			break;
		}
	}

	if (!pass) {
		otp_soak(0);
		return OTP_FAILURE;
	}
	return OTP_SUCCESS;
}

static void otp_strap_status(struct otpstrap_status *otpstrap)
{
	u32 OTPSTRAP_RAW[2];
	int strap_end;
	int i, j;

	if (info_cb.version == OTP_A0) {
		for (j = 0; j < 64; j++) {
			otpstrap[j].value = 0;
			otpstrap[j].remain_times = 7;
			otpstrap[j].writeable_option = -1;
			otpstrap[j].protected = 0;
		}
		strap_end = 30;
	} else {
		for (j = 0; j < 64; j++) {
			otpstrap[j].value = 0;
			otpstrap[j].remain_times = 6;
			otpstrap[j].writeable_option = -1;
			otpstrap[j].reg_protected = 0;
			otpstrap[j].protected = 0;
		}
		strap_end = 28;
	}

	otp_soak(0);
	for (i = 16; i < strap_end; i += 2) {
		int option = (i - 16) / 2;

		otp_read_conf(i, &OTPSTRAP_RAW[0]);
		otp_read_conf(i + 1, &OTPSTRAP_RAW[1]);
		for (j = 0; j < 32; j++) {
			char bit_value = ((OTPSTRAP_RAW[0] >> j) & 0x1);

			if (bit_value == 0 && otpstrap[j].writeable_option == -1)
				otpstrap[j].writeable_option = option;
			if (bit_value == 1)
				otpstrap[j].remain_times--;
			otpstrap[j].value ^= bit_value;
			otpstrap[j].option_array[option] = bit_value;
		}
		for (j = 32; j < 64; j++) {
			char bit_value = ((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1);

			if (bit_value == 0 && otpstrap[j].writeable_option == -1)
				otpstrap[j].writeable_option = option;
			if (bit_value == 1)
				otpstrap[j].remain_times--;
			otpstrap[j].value ^= bit_value;
			otpstrap[j].option_array[option] = bit_value;
		}
	}

	if (info_cb.version != OTP_A0) {
		otp_read_conf(28, &OTPSTRAP_RAW[0]);
		otp_read_conf(29, &OTPSTRAP_RAW[1]);
		for (j = 0; j < 32; j++) {
			if (((OTPSTRAP_RAW[0] >> j) & 0x1) == 1)
				otpstrap[j].reg_protected = 1;
		}
		for (j = 32; j < 64; j++) {
			if (((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1) == 1)
				otpstrap[j].reg_protected = 1;
		}
	}

	otp_read_conf(30, &OTPSTRAP_RAW[0]);
	otp_read_conf(31, &OTPSTRAP_RAW[1]);
	for (j = 0; j < 32; j++) {
		if (((OTPSTRAP_RAW[0] >> j) & 0x1) == 1)
			otpstrap[j].protected = 1;
	}
	for (j = 32; j < 64; j++) {
		if (((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1) == 1)
			otpstrap[j].protected = 1;
	}
}

static int otp_strap_bit_confirm(struct otpstrap_status *otpstrap, int offset, int ibit, int bit, int pbit, int rpbit)
{
	int prog_flag = 0;

	// ignore this bit
	if (ibit == 1)
		return OTP_SUCCESS;
	printf("OTPSTRAP[%X]:\n", offset);

	if (bit == otpstrap->value) {
		if (!pbit && !rpbit) {
			printf("    The value is same as before, skip it.\n");
			return OTP_PROG_SKIP;
		}
		printf("    The value is same as before.\n");
	} else {
		prog_flag = 1;
	}
	if (otpstrap->protected == 1 && prog_flag) {
		printf("    This bit is protected and is not writable\n");
		return OTP_FAILURE;
	}
	if (otpstrap->remain_times == 0 && prog_flag) {
		printf("    This bit is no remaining times to write.\n");
		return OTP_FAILURE;
	}
	if (pbit == 1)
		printf("    This bit will be protected and become non-writable.\n");
	if (rpbit == 1 && info_cb.version != OTP_A0)
		printf("    The relative register will be protected.\n");
	if (prog_flag)
		printf("    Write 1 to OTPSTRAP[%X] OPTION[%X], that value becomes from %d to %d.\n", offset, otpstrap->writeable_option + 1, otpstrap->value, otpstrap->value ^ 1);

	return OTP_SUCCESS;
}

static int otp_prog_strap_b(int bit_offset, int value)
{
	struct otpstrap_status otpstrap[64];
	u32 prog_address;
	int offset;
	int ret;

	otp_strap_status(otpstrap);

	ret = otp_strap_bit_confirm(&otpstrap[bit_offset], bit_offset, 0, value, 0, 0);

	if (ret != OTP_SUCCESS)
		return ret;

	prog_address = 0x800;
	if (bit_offset < 32) {
		offset = bit_offset;
		prog_address |= ((otpstrap[bit_offset].writeable_option * 2 + 16) / 8) * 0x200;
		prog_address |= ((otpstrap[bit_offset].writeable_option * 2 + 16) % 8) * 0x2;

	} else {
		offset = (bit_offset - 32);
		prog_address |= ((otpstrap[bit_offset].writeable_option * 2 + 17) / 8) * 0x200;
		prog_address |= ((otpstrap[bit_offset].writeable_option * 2 + 17) % 8) * 0x2;
	}

	return otp_prog_dc_b(1, prog_address, offset);
}

static int otp_print_conf(u32 offset, int dw_count)
{
	int i;
	u32 ret[1];

	if (offset + dw_count > 32)
		return OTP_USAGE;
	otp_soak(0);
	for (i = offset; i < offset + dw_count; i++) {
		otp_read_conf(i, ret);
		printf("OTPCFG%X: %08X\n", i, ret[0]);
	}
	printf("\n");
	return OTP_SUCCESS;
}

static int otp_print_data(u32 offset, int dw_count)
{
	int i;
	u32 ret[2];

	if (offset + dw_count > 2048 || offset % 4 != 0)
		return OTP_USAGE;
	otp_soak(0);
	for (i = offset; i < offset + dw_count; i += 2) {
		otp_read_data(i, ret);
		if (i % 4 == 0)
			printf("%03X: %08X %08X ", i * 4, ret[0], ret[1]);
		else
			printf("%08X %08X\n", ret[0], ret[1]);
	}
	printf("\n");
	return OTP_SUCCESS;
}

static int otp_print_strap(int start, int count)
{
	int i, j;
	int remains;
	struct otpstrap_status otpstrap[64];

	if (start < 0 || start > 64)
		return OTP_USAGE;

	if ((start + count) < 0 || (start + count) > 64)
		return OTP_USAGE;

	otp_strap_status(otpstrap);

	if (info_cb.version == OTP_A0) {
		remains = 7;
		printf("BIT(hex)  Value  Option           Status\n");
	} else {
		remains = 6;
		printf("BIT(hex)  Value  Option         Reg_Protect Status\n");
	}
	printf("______________________________________________________________________________\n");

	for (i = start; i < start + count; i++) {
		printf("0x%-8X", i);
		printf("%-7d", otpstrap[i].value);
		for (j = 0; j < remains; j++)
			printf("%d ", otpstrap[i].option_array[j]);
		printf("   ");
		if (info_cb.version != OTP_A0)
			printf("%d           ", otpstrap[i].reg_protected);
		if (otpstrap[i].protected == 1) {
			printf("protected and not writable");
		} else {
			printf("not protected ");
			if (otpstrap[i].remain_times == 0)
				printf("and no remaining times to write.");
			else
				printf("and still can write %d times", otpstrap[i].remain_times);
		}
		printf("\n");
	}

	return OTP_SUCCESS;
}

static void otp_print_revid(u32 *rid)
{
	int bit_offset;
	int i, j;

	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("___________________________________________________\n");
	for (i = 0; i < 64; i++) {
		if (i < 32) {
			j = 0;
			bit_offset = i;
		} else {
			j = 1;
			bit_offset = i - 32;
		}
		if (i % 16 == 0)
			printf("%2x | ", i);
		printf("%d  ", (rid[j] >> bit_offset) & 0x1);
		if ((i + 1) % 16 == 0)
			printf("\n");
	}
}

static int otp_print_conf_image(struct otp_image_layout *image_layout)
{
	const struct otpconf_info *conf_info = info_cb.conf_info;
	u32 *OTPCFG = (u32 *)image_layout->conf;
	u32 *OTPCFG_IGNORE = (u32 *)image_layout->conf_ignore;
	u32 mask;
	u32 dw_offset;
	u32 bit_offset;
	u32 otp_value;
	u32 otp_ignore;
	int fail = 0;
	int rid_num = 0;
	char valid_bit[20];
	int fz;
	int i;
	int j;

	printf("DW    BIT        Value       Description\n");
	printf("__________________________________________________________________________\n");
	for (i = 0; i < info_cb.conf_info_len; i++) {
		dw_offset = conf_info[i].dw_offset;
		bit_offset = conf_info[i].bit_offset;
		mask = BIT(conf_info[i].length) - 1;
		otp_value = (OTPCFG[dw_offset] >> bit_offset) & mask;
		otp_ignore = (OTPCFG_IGNORE[dw_offset] >> bit_offset) & mask;

		if (otp_ignore == mask)
			continue;
		else if (otp_ignore != 0)
			fail = 1;

		if (otp_value != conf_info[i].value &&
		    conf_info[i].value != OTP_REG_RESERVED &&
		    conf_info[i].value != OTP_REG_VALUE &&
		    conf_info[i].value != OTP_REG_VALID_BIT)
			continue;
		printf("0x%-4X", dw_offset);

		if (conf_info[i].length == 1) {
			printf("0x%-9X", conf_info[i].bit_offset);
		} else {
			printf("0x%-2X:0x%-4X",
			       conf_info[i].bit_offset + conf_info[i].length - 1,
			       conf_info[i].bit_offset);
		}
		printf("0x%-10x", otp_value);

		if (fail) {
			printf("Ignore mask error\n");
			continue;
		}
		if (conf_info[i].value == OTP_REG_RESERVED) {
			printf("Reserved\n");
		} else if (conf_info[i].value == OTP_REG_VALUE) {
			printf(conf_info[i].information, otp_value);
			printf("\n");
		} else if (conf_info[i].value == OTP_REG_VALID_BIT) {
			if (otp_value != 0) {
				for (j = 0; j < 7; j++) {
					if (otp_value == (1 << j))
						valid_bit[j * 2] = '1';
					else
						valid_bit[j * 2] = '0';
					valid_bit[j * 2 + 1] = ' ';
				}
				valid_bit[15] = 0;
			} else {
				strcpy(valid_bit, "0 0 0 0 0 0 0 0\0");
			}
			printf(conf_info[i].information, valid_bit);
			printf("\n");
		} else {
			printf("%s\n", conf_info[i].information);
		}
	}

	if (OTPCFG[0xa] != 0 || OTPCFG[0xb] != 0) {
		if (OTPCFG_IGNORE[0xa] != 0 && OTPCFG_IGNORE[0xb] != 0) {
			printf("OTP revision ID is invalid.\n");
			fail = 1;
		} else {
			fz = 0;
			for (i = 0; i < 64; i++) {
				if (get_dw_bit(&OTPCFG[0xa], i) == 0) {
					if (!fz)
						fz = 1;
				} else {
					rid_num++;
					if (fz) {
						printf("OTP revision ID is invalid.\n");
						fail = 1;
						break;
					}
				}
			}
		}
		if (fail)
			printf("OTP revision ID\n");
		else
			printf("OTP revision ID: 0x%x\n", rid_num);
		otp_print_revid(&OTPCFG[0xa]);
	}

	if (fail)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static int otp_print_conf_info(int input_offset)
{
	const struct otpconf_info *conf_info = info_cb.conf_info;
	u32 OTPCFG[16];
	u32 mask;
	u32 dw_offset;
	u32 bit_offset;
	u32 otp_value;
	char valid_bit[20];
	int i;
	int j;

	otp_soak(0);
	for (i = 0; i < 16; i++)
		otp_read_conf(i, &OTPCFG[i]);

	printf("DW    BIT        Value       Description\n");
	printf("__________________________________________________________________________\n");
	for (i = 0; i < info_cb.conf_info_len; i++) {
		if (input_offset != -1 && input_offset != conf_info[i].dw_offset)
			continue;
		dw_offset = conf_info[i].dw_offset;
		bit_offset = conf_info[i].bit_offset;
		mask = BIT(conf_info[i].length) - 1;
		otp_value = (OTPCFG[dw_offset] >> bit_offset) & mask;

		if (otp_value != conf_info[i].value &&
		    conf_info[i].value != OTP_REG_RESERVED &&
		    conf_info[i].value != OTP_REG_VALUE &&
		    conf_info[i].value != OTP_REG_VALID_BIT)
			continue;
		printf("0x%-4X", dw_offset);

		if (conf_info[i].length == 1) {
			printf("0x%-9X", conf_info[i].bit_offset);
		} else {
			printf("0x%-2X:0x%-4X",
			       conf_info[i].bit_offset + conf_info[i].length - 1,
			       conf_info[i].bit_offset);
		}
		printf("0x%-10x", otp_value);

		if (conf_info[i].value == OTP_REG_RESERVED) {
			printf("Reserved\n");
		} else if (conf_info[i].value == OTP_REG_VALUE) {
			printf(conf_info[i].information, otp_value);
			printf("\n");
		} else if (conf_info[i].value == OTP_REG_VALID_BIT) {
			if (otp_value != 0) {
				for (j = 0; j < 7; j++) {
					if (otp_value == (1 << j))
						valid_bit[j * 2] = '1';
					else
						valid_bit[j * 2] = '0';
					valid_bit[j * 2 + 1] = ' ';
				}
				valid_bit[15] = 0;
			} else {
				strcpy(valid_bit, "0 0 0 0 0 0 0 0\0");
			}
			printf(conf_info[i].information, valid_bit);
			printf("\n");
		} else {
			printf("%s\n", conf_info[i].information);
		}
	}
	return OTP_SUCCESS;
}

static int otp_print_strap_image(struct otp_image_layout *image_layout)
{
	const struct otpstrap_info *strap_info = info_cb.strap_info;
	u32 *OTPSTRAP;
	u32 *OTPSTRAP_REG_PRO;
	u32 *OTPSTRAP_PRO;
	u32 *OTPSTRAP_IGNORE;
	int i;
	int fail = 0;
	u32 bit_offset;
	u32 dw_offset;
	u32 mask;
	u32 otp_value;
	u32 otp_reg_protect;
	u32 otp_protect;
	u32 otp_ignore;

	OTPSTRAP = (u32 *)image_layout->strap;
	OTPSTRAP_PRO = (u32 *)image_layout->strap_pro;
	OTPSTRAP_IGNORE = (u32 *)image_layout->strap_ignore;
	if (info_cb.version == OTP_A0) {
		OTPSTRAP_REG_PRO = NULL;
		printf("BIT(hex)   Value       Protect     Description\n");
	} else {
		OTPSTRAP_REG_PRO = (u32 *)image_layout->strap_reg_pro;
		printf("BIT(hex)   Value       Reg_Protect Protect     Description\n");
	}
	printf("__________________________________________________________________________________________\n");

	for (i = 0; i < info_cb.strap_info_len; i++) {
		if (strap_info[i].bit_offset > 31) {
			dw_offset = 1;
			bit_offset = strap_info[i].bit_offset - 32;
		} else {
			dw_offset = 0;
			bit_offset = strap_info[i].bit_offset;
		}

		mask = BIT(strap_info[i].length) - 1;
		otp_value = (OTPSTRAP[dw_offset] >> bit_offset) & mask;
		otp_protect = (OTPSTRAP_PRO[dw_offset] >> bit_offset) & mask;
		otp_ignore = (OTPSTRAP_IGNORE[dw_offset] >> bit_offset) & mask;

		if (info_cb.version != OTP_A0)
			otp_reg_protect = (OTPSTRAP_REG_PRO[dw_offset] >> bit_offset) & mask;
		else
			otp_reg_protect = 0;

		if (otp_ignore == mask)
			continue;
		else if (otp_ignore != 0)
			fail = 1;

		if (otp_value != strap_info[i].value &&
		    strap_info[i].value != OTP_REG_RESERVED)
			continue;

		if (strap_info[i].length == 1) {
			printf("0x%-9X", strap_info[i].bit_offset);
		} else {
			printf("0x%-2X:0x%-4X",
			       strap_info[i].bit_offset + strap_info[i].length - 1,
			       strap_info[i].bit_offset);
		}
		printf("0x%-10x", otp_value);
		if (info_cb.version != OTP_A0)
			printf("0x%-10x", otp_reg_protect);
		printf("0x%-10x", otp_protect);

		if (fail) {
			printf("Ignore mask error\n");
		} else {
			if (strap_info[i].value != OTP_REG_RESERVED)
				printf("%s\n", strap_info[i].information);
			else
				printf("Reserved\n");
		}
	}

	if (fail)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static int otp_print_strap_info(int view)
{
	const struct otpstrap_info *strap_info = info_cb.strap_info;
	struct otpstrap_status strap_status[64];
	int i, j;
	int fail = 0;
	u32 bit_offset;
	u32 length;
	u32 otp_value;
	u32 otp_protect;

	otp_strap_status(strap_status);

	if (view) {
		if (info_cb.version == OTP_A0)
			printf("BIT(hex) Value  Remains  Protect   Description\n");
		else
			printf("BIT(hex) Value  Remains  Reg_Protect Protect   Description\n");
		printf("___________________________________________________________________________________________________\n");
	} else {
		printf("BIT(hex)   Value       Description\n");
		printf("________________________________________________________________________________\n");
	}
	for (i = 0; i < info_cb.strap_info_len; i++) {
		otp_value = 0;
		bit_offset = strap_info[i].bit_offset;
		length = strap_info[i].length;
		for (j = 0; j < length; j++) {
			otp_value |= strap_status[bit_offset + j].value << j;
			otp_protect |= strap_status[bit_offset + j].protected << j;
		}
		if (otp_value != strap_info[i].value &&
		    strap_info[i].value != OTP_REG_RESERVED)
			continue;
		if (view) {
			for (j = 0; j < length; j++) {
				printf("0x%-7X", strap_info[i].bit_offset + j);
				printf("0x%-5X", strap_status[bit_offset + j].value);
				printf("%-9d", strap_status[bit_offset + j].remain_times);
				if (info_cb.version != OTP_A0)
					printf("0x%-10X", strap_status[bit_offset + j].reg_protected);
				printf("0x%-7X", strap_status[bit_offset + j].protected);
				if (strap_info[i].value == OTP_REG_RESERVED) {
					printf(" Reserved\n");
					continue;
				}
				if (length == 1) {
					printf(" %s\n", strap_info[i].information);
					continue;
				}

				if (j == 0)
					printf("/%s\n", strap_info[i].information);
				else if (j == length - 1)
					printf("\\ \"\n");
				else
					printf("| \"\n");
			}
		} else {
			if (length == 1) {
				printf("0x%-9X", strap_info[i].bit_offset);
			} else {
				printf("0x%-2X:0x%-4X",
				       bit_offset + length - 1, bit_offset);
			}

			printf("0x%-10X", otp_value);

			if (strap_info[i].value != OTP_REG_RESERVED)
				printf("%s\n", strap_info[i].information);
			else
				printf("Reserved\n");
		}
	}

	if (fail)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static int otp_print_data_image(struct otp_image_layout *image_layout)
{
	int key_id, key_offset, last, key_type, key_length, exp_length;
	const struct otpkey_type *key_info_array = info_cb.key_info;
	struct otpkey_type key_info;
	u32 *buf;
	u8 *byte_buf;
	char empty = 1;
	int i = 0, len = 0;
	int j;

	byte_buf = image_layout->data;
	buf = (u32 *)byte_buf;

	for (i = 0; i < 16; i++) {
		if (buf[i] != 0)
			empty = 0;
	}
	if (empty)
		return OTP_SUCCESS;

	i = 0;
	while (1) {
		key_id = buf[i] & 0x7;
		key_offset = buf[i] & 0x1ff8;
		last = (buf[i] >> 13) & 1;
		key_type = (buf[i] >> 14) & 0xf;
		key_length = (buf[i] >> 18) & 0x3;
		exp_length = (buf[i] >> 20) & 0xfff;

		for (j = 0; j < info_cb.key_info_len; j++) {
			if (key_type == key_info_array[j].value) {
				key_info = key_info_array[j];
				break;
			}
		}

		printf("\nKey[%d]:\n", i);
		printf("Key Type: ");
		printf("%s\n", key_info.information);

		if (key_info.key_type == OTP_KEY_TYPE_HMAC) {
			printf("HMAC SHA Type: ");
			switch (key_length) {
			case 0:
				printf("HMAC(SHA224)\n");
				break;
			case 1:
				printf("HMAC(SHA256)\n");
				break;
			case 2:
				printf("HMAC(SHA384)\n");
				break;
			case 3:
				printf("HMAC(SHA512)\n");
				break;
			}
		} else if (key_info.key_type == OTP_KEY_TYPE_RSA_PRIV ||
			   key_info.key_type == OTP_KEY_TYPE_RSA_PUB) {
			printf("RSA SHA Type: ");
			switch (key_length) {
			case 0:
				printf("RSA1024\n");
				len = 0x100;
				break;
			case 1:
				printf("RSA2048\n");
				len = 0x200;
				break;
			case 2:
				printf("RSA3072\n");
				len = 0x300;
				break;
			case 3:
				printf("RSA4096\n");
				len = 0x400;
				break;
			}
			printf("RSA exponent bit length: %d\n", exp_length);
		}
		if (key_info.need_id)
			printf("Key Number ID: %d\n", key_id);
		printf("Key Value:\n");
		if (key_info.key_type == OTP_KEY_TYPE_HMAC) {
			buf_print(&byte_buf[key_offset], 0x40);
		} else if (key_info.key_type == OTP_KEY_TYPE_AES) {
			printf("AES Key:\n");
			buf_print(&byte_buf[key_offset], 0x20);
			if (info_cb.version == OTP_A0) {
				printf("AES IV:\n");
				buf_print(&byte_buf[key_offset + 0x20], 0x10);
			}

		} else if (key_info.key_type == OTP_KEY_TYPE_VAULT) {
			if (info_cb.version == OTP_A0) {
				printf("AES Key:\n");
				buf_print(&byte_buf[key_offset], 0x20);
				printf("AES IV:\n");
				buf_print(&byte_buf[key_offset + 0x20], 0x10);
			} else {
				printf("AES Key 1:\n");
				buf_print(&byte_buf[key_offset], 0x20);
				printf("AES Key 2:\n");
				buf_print(&byte_buf[key_offset + 0x20], 0x20);
			}
		} else if (key_info.key_type == OTP_KEY_TYPE_RSA_PRIV) {
			printf("RSA mod:\n");
			buf_print(&byte_buf[key_offset], len / 2);
			printf("RSA exp:\n");
			buf_print(&byte_buf[key_offset + (len / 2)], len / 2);
		} else if (key_info.key_type == OTP_KEY_TYPE_RSA_PUB) {
			printf("RSA mod:\n");
			buf_print(&byte_buf[key_offset], len / 2);
			printf("RSA exp:\n");
			buf_print((u8 *)"\x01\x00\x01", 3);
		}
		if (last)
			break;
		i++;
	}
	return OTP_SUCCESS;
}

static int otp_strap_image_confirm(struct otp_image_layout *image_layout)
{
	int i;
	u32 *strap;
	u32 *strap_ignore;
	u32 *strap_reg_protect;
	u32 *strap_pro;
	int bit, pbit, ibit, rpbit;
	int fail = 0;
	int ret;
	struct otpstrap_status otpstrap[64];

	strap = (u32 *)image_layout->strap;
	strap_pro = (u32 *)image_layout->strap_pro;
	strap_ignore = (u32 *)image_layout->strap_ignore;
	strap_reg_protect = (u32 *)image_layout->strap_reg_pro;

	otp_strap_status(otpstrap);
	for (i = 0; i < 64; i++) {
		if (i < 32) {
			bit = (strap[0] >> i) & 0x1;
			ibit = (strap_ignore[0] >> i) & 0x1;
			pbit = (strap_pro[0] >> i) & 0x1;
		} else {
			bit = (strap[1] >> (i - 32)) & 0x1;
			ibit = (strap_ignore[1] >> (i - 32)) & 0x1;
			pbit = (strap_pro[1] >> (i - 32)) & 0x1;
		}

		if (info_cb.version != OTP_A0) {
			if (i < 32)
				rpbit = (strap_reg_protect[0] >> i) & 0x1;
			else
				rpbit = (strap_reg_protect[1] >> (i - 32)) & 0x1;
		} else {
			rpbit = 0;
		}
		ret = otp_strap_bit_confirm(&otpstrap[i], i, ibit, bit, pbit, rpbit);

		if (ret == OTP_FAILURE)
			fail = 1;
	}
	if (fail == 1)
		return OTP_FAILURE;
	else
		return OTP_SUCCESS;
}

static int otp_prog_data(struct otp_image_layout *image_layout)
{
	int i;
	int ret;
	int data_dw;
	u32 data[2048];
	u32 *buf;
	u32 *buf_ignore;
	u32 data_masked;
	u32 buf_masked;

	buf = (u32 *)image_layout->data;
	buf_ignore = (u32 *)image_layout->data_ignore;

	data_dw = image_layout->data_length / 4;

	printf("Read OTP Data:\n");

	for (i = 0; i < data_dw - 2 ; i += 2)
		otp_read_data(i, &data[i]);

	printf("Check writable...\n");
	// ignore last two dw, the last two dw is used for slt otp write check.
	for (i = 0; i < data_dw - 2; i++) {
		data_masked = data[i]  & ~buf_ignore[i];
		buf_masked  = buf[i] & ~buf_ignore[i];
		if (data_masked == buf_masked)
			continue;
		if (i % 2 == 0) {
			if ((data_masked | buf_masked) == buf_masked) {
				continue;
			} else {
				printf("Input image can't program into OTP, please check.\n");
				printf("OTP_ADDR[%x] = %x\n", i, data[i]);
				printf("Input   [%x] = %x\n", i, buf[i]);
				printf("Mask    [%x] = %x\n", i, ~buf_ignore[i]);
				return OTP_FAILURE;
			}
		} else {
			if ((data_masked & buf_masked) == buf_masked) {
				continue;
			} else {
				printf("Input image can't program into OTP, please check.\n");
				printf("OTP_ADDR[%x] = %x\n", i, data[i]);
				printf("Input   [%x] = %x\n", i, buf[i]);
				printf("Mask    [%x] = %x\n", i, ~buf_ignore[i]);
				return OTP_FAILURE;
			}
		}
	}

	printf("Start Programing...\n");

	// programing ecc region first
	for (i = 1792; i < 2046; i += 2) {
		ret = otp_prog_verify_2dw(&data[i], &buf[i], &buf_ignore[i], i);
		if (ret != OTP_SUCCESS) {
			printf("address: %08x, data: %08x %08x, buffer: %08x %08x, mask: %08x %08x\n",
			       i, data[i], data[i + 1], buf[i], buf[i + 1], buf_ignore[i], buf_ignore[i + 1]);
			return ret;
		}
	}

	for (i = 0; i < 1792; i += 2) {
		ret = otp_prog_verify_2dw(&data[i], &buf[i], &buf_ignore[i], i);
		if (ret != OTP_SUCCESS) {
			printf("address: %08x, data: %08x %08x, buffer: %08x %08x, mask: %08x %08x\n",
			       i, data[i], data[i + 1], buf[i], buf[i + 1], buf_ignore[i], buf_ignore[i + 1]);
			return ret;
		}
	}
	otp_soak(0);
	return OTP_SUCCESS;
}

static int otp_prog_strap(struct otp_image_layout *image_layout)
{
	u32 *strap;
	u32 *strap_ignore;
	u32 *strap_pro;
	u32 *strap_reg_protect;
	u32 prog_address;
	int i;
	int bit, pbit, ibit, offset, rpbit;
	int fail = 0;
	int ret;
	int prog_flag = 0;
	struct otpstrap_status otpstrap[64];

	strap = (u32 *)image_layout->strap;
	strap_pro = (u32 *)image_layout->strap_pro;
	strap_ignore = (u32 *)image_layout->strap_ignore;
	strap_reg_protect = (u32 *)image_layout->strap_reg_pro;

	printf("Read OTP Strap Region:\n");
	otp_strap_status(otpstrap);

	printf("Check writable...\n");
	if (otp_strap_image_confirm(image_layout) == OTP_FAILURE) {
		printf("Input image can't program into OTP, please check.\n");
		return OTP_FAILURE;
	}

	for (i = 0; i < 64; i++) {
		prog_address = 0x800;
		if (i < 32) {
			offset = i;
			bit = (strap[0] >> offset) & 0x1;
			ibit = (strap_ignore[0] >> offset) & 0x1;
			pbit = (strap_pro[0] >> offset) & 0x1;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 16) / 8) * 0x200;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 16) % 8) * 0x2;

		} else {
			offset = (i - 32);
			bit = (strap[1] >> offset) & 0x1;
			ibit = (strap_ignore[1] >> offset) & 0x1;
			pbit = (strap_pro[1] >> offset) & 0x1;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 17) / 8) * 0x200;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 17) % 8) * 0x2;
		}
		if (info_cb.version != OTP_A0) {
			if (i < 32)
				rpbit = (strap_reg_protect[0] >> i) & 0x1;
			else
				rpbit = (strap_reg_protect[1] >> (i - 32)) & 0x1;
		} else {
			rpbit = 0;
		}

		if (ibit == 1)
			continue;
		if (bit == otpstrap[i].value)
			prog_flag = 0;
		else
			prog_flag = 1;

		if (otpstrap[i].protected == 1 && prog_flag) {
			fail = 1;
			continue;
		}
		if (otpstrap[i].remain_times == 0 && prog_flag) {
			fail = 1;
			continue;
		}

		if (prog_flag) {
			ret = otp_prog_dc_b(1, prog_address, offset);
			if (ret)
				return OTP_FAILURE;
		}

		if (rpbit == 1 && info_cb.version != OTP_A0) {
			prog_address = 0x800;
			if (i < 32)
				prog_address |= 0x608;
			else
				prog_address |= 0x60a;

			ret = otp_prog_dc_b(1, prog_address, offset);
			if (ret)
				return OTP_FAILURE;
		}

		if (pbit != 0) {
			prog_address = 0x800;
			if (i < 32)
				prog_address |= 0x60c;
			else
				prog_address |= 0x60e;

			ret = otp_prog_dc_b(1, prog_address, offset);
			if (ret)
				return OTP_FAILURE;
		}
	}
	otp_soak(0);
	if (fail == 1)
		return OTP_FAILURE;
	return OTP_SUCCESS;
}

static int otp_prog_conf(struct otp_image_layout *image_layout)
{
	int i, k;
	int pass = 0;
	u32 prog_address;
	u32 data[16];
	u32 compare[2];
	u32 *conf = (u32 *)image_layout->conf;
	u32 *conf_ignore = (u32 *)image_layout->conf_ignore;
	u32 data_masked;
	u32 buf_masked;

	printf("Read OTP Config Region:\n");

	for (i = 0; i < 16 ; i++) {
		prog_address = 0x800;
		prog_address |= (i / 8) * 0x200;
		prog_address |= (i % 8) * 0x2;
		otp_read_data(prog_address, &data[i]);
	}

	printf("Check writable...\n");
	for (i = 0; i < 16; i++) {
		data_masked = data[i]  & ~conf_ignore[i];
		buf_masked  = conf[i] & ~conf_ignore[i];
		if (data_masked == buf_masked)
			continue;
		if ((data_masked | buf_masked) == buf_masked) {
			continue;
		} else {
			printf("Input image can't program into OTP, please check.\n");
			printf("OTPCFG[%X] = %x\n", i, data[i]);
			printf("Input [%X] = %x\n", i, conf[i]);
			printf("Mask  [%X] = %x\n", i, ~conf_ignore[i]);
			return OTP_FAILURE;
		}
	}

	printf("Start Programing...\n");
	otp_soak(0);
	for (i = 0; i < 16; i++) {
		data_masked = data[i]  & ~conf_ignore[i];
		buf_masked  = conf[i] & ~conf_ignore[i];
		prog_address = 0x800;
		prog_address |= (i / 8) * 0x200;
		prog_address |= (i % 8) * 0x2;
		if (data_masked == buf_masked) {
			pass = 1;
			continue;
		}

		otp_soak(1);
		otp_prog_dw(conf[i], conf_ignore[i], prog_address);

		pass = 0;
		for (k = 0; k < RETRY; k++) {
			if (verify_dw(prog_address, &conf[i], &conf_ignore[i], compare, 1) != 0) {
				otp_soak(2);
				otp_prog_dw(compare[0], conf_ignore[i], prog_address);
				if (verify_dw(prog_address, &conf[i], &conf_ignore[i], compare, 1) != 0) {
					otp_soak(1);
				} else {
					pass = 1;
					break;
				}
			} else {
				pass = 1;
				break;
			}
		}
		if (pass == 0) {
			printf("address: %08x, data: %08x, buffer: %08x, mask: %08x\n",
			       i, data[i], conf[i], conf_ignore[i]);
			break;
		}
	}

	otp_soak(0);
	if (!pass)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static int otp_verify_image(u8 *src_buf, u32 length, u8 *digest_buf)
{
	sha256_context ctx;
	u8 digest_ret[CHECKSUM_LEN];

	sha256_starts(&ctx);
	sha256_update(&ctx, src_buf, length);
	sha256_finish(&ctx, digest_ret);

	if (!memcmp(digest_buf, digest_ret, CHECKSUM_LEN))
		return OTP_SUCCESS;
	return OTP_FAILURE;
}

static int otp_prog_image(int addr, int nconfirm)
{
	int ret;
	int image_version = 0;
	struct otp_header *otp_header;
	struct otp_image_layout image_layout;
	int image_size;
	u8 *buf;
	u8 *checksum;

	otp_header = map_physmem(addr, sizeof(struct otp_header), MAP_WRBACK);
	if (!otp_header) {
		puts("Failed to map physical memory\n");
		return OTP_FAILURE;
	}

	image_size = OTP_IMAGE_SIZE(otp_header->image_info);
	unmap_physmem(otp_header, MAP_WRBACK);

	buf = map_physmem(addr, image_size + CHECKSUM_LEN, MAP_WRBACK);

	if (!buf) {
		puts("Failed to map physical memory\n");
		return OTP_FAILURE;
	}
	otp_header = (struct otp_header *)buf;
	checksum = buf + otp_header->checksum_offset;

	if (strcmp(OTP_MAGIC, (char *)otp_header->otp_magic) != 0) {
		puts("Image is invalid\n");
		return OTP_FAILURE;
	}

	image_layout.data_length = (int)(OTP_REGION_SIZE(otp_header->data_info) / 2);
	image_layout.data = buf + OTP_REGION_OFFSET(otp_header->data_info);
	image_layout.data_ignore = image_layout.data + image_layout.data_length;

	image_layout.conf_length = (int)(OTP_REGION_SIZE(otp_header->config_info) / 2);
	image_layout.conf = buf + OTP_REGION_OFFSET(otp_header->config_info);
	image_layout.conf_ignore = image_layout.conf + image_layout.conf_length;

	image_layout.strap = buf + OTP_REGION_OFFSET(otp_header->strap_info);

	if (!strcmp("A0", (char *)otp_header->otp_version)) {
		image_version = OTP_A0;
		image_layout.strap_length = (int)(OTP_REGION_SIZE(otp_header->strap_info) / 3);
		image_layout.strap_pro = image_layout.strap + image_layout.strap_length;
		image_layout.strap_ignore = image_layout.strap + 2 * image_layout.strap_length;
	} else if (!strcmp("A1", (char *)otp_header->otp_version)) {
		image_version = OTP_A1;
		image_layout.strap_length = (int)(OTP_REGION_SIZE(otp_header->strap_info) / 4);
		image_layout.strap_reg_pro = image_layout.strap + image_layout.strap_length;
		image_layout.strap_pro = image_layout.strap + 2 * image_layout.strap_length;
		image_layout.strap_ignore = image_layout.strap + 3 * image_layout.strap_length;
	} else if (!strcmp("A2", (char *)otp_header->otp_version)) {
		image_version = OTP_A2;
		image_layout.strap_length = (int)(OTP_REGION_SIZE(otp_header->strap_info) / 4);
		image_layout.strap_reg_pro = image_layout.strap + image_layout.strap_length;
		image_layout.strap_pro = image_layout.strap + 2 * image_layout.strap_length;
		image_layout.strap_ignore = image_layout.strap + 3 * image_layout.strap_length;
	} else if (!strcmp("A3", (char *)otp_header->otp_version)) {
		image_version = OTP_A3;
		image_layout.strap_length = (int)(OTP_REGION_SIZE(otp_header->strap_info) / 4);
		image_layout.strap_reg_pro = image_layout.strap + image_layout.strap_length;
		image_layout.strap_pro = image_layout.strap + 2 * image_layout.strap_length;
		image_layout.strap_ignore = image_layout.strap + 3 * image_layout.strap_length;
	} else {
		puts("Version is not supported\n");
		return OTP_FAILURE;
	}

	if (image_version != info_cb.version) {
		puts("Version is not match\n");
		return OTP_FAILURE;
	}

	if (otp_verify_image(buf, image_size, checksum)) {
		puts("checksum is invalid\n");
		return OTP_FAILURE;
	}

	if (!nconfirm) {
		if (otp_header->image_info & OTP_INC_DATA) {
			printf("\nOTP data region :\n");
			if (otp_print_data_image(&image_layout) < 0) {
				printf("OTP data error, please check.\n");
				return OTP_FAILURE;
			}
		}
		if (otp_header->image_info & OTP_INC_STRAP) {
			printf("\nOTP strap region :\n");
			if (otp_print_strap_image(&image_layout) < 0) {
				printf("OTP strap error, please check.\n");
				return OTP_FAILURE;
			}
		}
		if (otp_header->image_info & OTP_INC_CONFIG) {
			printf("\nOTP configuration region :\n");
			if (otp_print_conf_image(&image_layout) < 0) {
				printf("OTP config error, please check.\n");
				return OTP_FAILURE;
			}
		}

		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}

	if (otp_header->image_info & OTP_INC_DATA) {
		printf("programing data region ...\n");
		ret = otp_prog_data(&image_layout);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		}
		printf("Done\n");
	}
	if (otp_header->image_info & OTP_INC_STRAP) {
		printf("programing strap region ...\n");
		ret = otp_prog_strap(&image_layout);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		}
		printf("Done\n");
	}
	if (otp_header->image_info & OTP_INC_CONFIG) {
		printf("programing configuration region ...\n");
		ret = otp_prog_conf(&image_layout);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		}
		printf("Done\n");
	}

	return OTP_SUCCESS;
}

static int otp_prog_bit(int mode, int otp_dw_offset, int bit_offset, int value, int nconfirm)
{
	u32 read[2];
	u32 prog_address = 0;
	struct otpstrap_status otpstrap[64];
	int otp_bit;
	int ret = 0;

	otp_soak(0);
	switch (mode) {
	case OTP_REGION_CONF:
		otp_read_conf(otp_dw_offset, read);
		prog_address = 0x800;
		prog_address |= (otp_dw_offset / 8) * 0x200;
		prog_address |= (otp_dw_offset % 8) * 0x2;
		otp_bit = (read[0] >> bit_offset) & 0x1;
		if (otp_bit == value) {
			printf("OTPCFG%X[%X] = %d\n", otp_dw_offset, bit_offset, value);
			printf("No need to program\n");
			return OTP_SUCCESS;
		}
		if (otp_bit == 1 && value == 0) {
			printf("OTPCFG%X[%X] = 1\n", otp_dw_offset, bit_offset);
			printf("OTP is programed, which can't be clean\n");
			return OTP_FAILURE;
		}
		printf("Program OTPCFG%X[%X] to 1\n", otp_dw_offset, bit_offset);
		break;
	case OTP_REGION_DATA:
		prog_address = otp_dw_offset;

		if (otp_dw_offset % 2 == 0) {
			otp_read_data(otp_dw_offset, read);
			otp_bit = (read[0] >> bit_offset) & 0x1;

			if (otp_bit == 1 && value == 0) {
				printf("OTPDATA%X[%X] = 1\n", otp_dw_offset, bit_offset);
				printf("OTP is programed, which can't be cleaned\n");
				return OTP_FAILURE;
			}
		} else {
			otp_read_data(otp_dw_offset - 1, read);
			otp_bit = (read[1] >> bit_offset) & 0x1;

			if (otp_bit == 0 && value == 1) {
				printf("OTPDATA%X[%X] = 1\n", otp_dw_offset, bit_offset);
				printf("OTP is programed, which can't be writen\n");
				return OTP_FAILURE;
			}
		}
		if (otp_bit == value) {
			printf("OTPDATA%X[%X] = %d\n", otp_dw_offset, bit_offset, value);
			printf("No need to program\n");
			return OTP_SUCCESS;
		}

		printf("Program OTPDATA%X[%X] to 1\n", otp_dw_offset, bit_offset);
		break;
	case OTP_REGION_STRAP:
		otp_strap_status(otpstrap);
		otp_print_strap(bit_offset, 1);
		ret = otp_strap_bit_confirm(&otpstrap[bit_offset], bit_offset, 0, value, 0, 0);
		if (ret == OTP_FAILURE)
			return OTP_FAILURE;
		else if (ret == OTP_PROG_SKIP)
			return OTP_SUCCESS;

		break;
	}

	if (!nconfirm) {
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}

	switch (mode) {
	case OTP_REGION_STRAP:
		ret =  otp_prog_strap_b(bit_offset, value);
		break;
	case OTP_REGION_CONF:
	case OTP_REGION_DATA:
		ret = otp_prog_dc_b(value, prog_address, bit_offset);
		break;
	}
	otp_soak(0);
	if (ret) {
		printf("OTP cannot be programed\n");
		printf("FAILURE\n");
		return OTP_FAILURE;
	}

	printf("SUCCESS\n");
	return OTP_SUCCESS;
}

static int otp_update_rid(u32 update_num, int force)
{
	u32 otp_rid[2];
	int rid_num = 0;
	int bit_offset;
	int dw_offset;
	int i;
	int ret;

	otp_read_conf(10, &otp_rid[0]);
	otp_read_conf(11, &otp_rid[1]);

	rid_num = get_rid_num(otp_rid);

	if (rid_num < 0) {
		printf("Currennt OTP revision ID cannot handle by this command,\n"
		       "plase use 'otp pb' command to update it manually\n");
		otp_print_revid(otp_rid);
		return OTP_FAILURE;
	}

	printf("current OTP revision ID: 0x%x\n", rid_num);
	otp_print_revid(otp_rid);
	printf("input update number: 0x%x\n", update_num);

	if (rid_num >= update_num) {
		printf("OTP rev_id is bigger than 0x%x\n", update_num);
		printf("Skip\n");
		return OTP_FAILURE;
	}

	for (i = rid_num; i < update_num; i++) {
		if (i < 32) {
			dw_offset = 0xa;
			bit_offset = i;
		} else {
			dw_offset = 0xb;
			bit_offset = i - 32;
		}
		printf("OTPCFG%X[%d]", dw_offset, bit_offset);
		if (i + 1 != update_num)
			printf(", ");
	}

	printf(" will be programmed\n");
	if (force == 0) {
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}

	ret = 0;
	for (i = rid_num; i < update_num; i++) {
		if (i < 32) {
			dw_offset = 0xa04;
			bit_offset = i;
		} else {
			dw_offset = 0xa06;
			bit_offset = i - 32;
		}
		if (otp_prog_dc_b(1, dw_offset, bit_offset)) {
			printf("OTPCFG%X[%d] programming failed\n", dw_offset, bit_offset);
			ret = OTP_FAILURE;
			break;
		}
	}

	otp_read_conf(10, &otp_rid[0]);
	otp_read_conf(11, &otp_rid[1]);
	rid_num = get_rid_num(otp_rid);
	if (rid_num >= 0)
		printf("OTP revision ID: 0x%x\n", rid_num);
	else
		printf("OTP revision ID\n");
	otp_print_revid(otp_rid);
	if (!ret)
		printf("SUCCESS\n");
	else
		printf("FAILED\n");
	return ret;
}

static int do_otpread(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 offset, count;
	int ret;

	if (argc == 4) {
		offset = simple_strtoul(argv[2], NULL, 16);
		count = simple_strtoul(argv[3], NULL, 16);
	} else if (argc == 3) {
		offset = simple_strtoul(argv[2], NULL, 16);
		count = 1;
	} else {
		return CMD_RET_USAGE;
	}

	if (!strcmp(argv[1], "conf")) {
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		ret = otp_print_conf(offset, count);
	} else if (!strcmp(argv[1], "data")) {
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		ret = otp_print_data(offset, count);
	} else if (!strcmp(argv[1], "strap")) {
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		ret = otp_print_strap(offset, count);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	return CMD_RET_USAGE;
}

static int do_otpprog(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t addr;
	int ret;

	if (argc == 3) {
		if (strcmp(argv[1], "o"))
			return CMD_RET_USAGE;
		addr = simple_strtoul(argv[2], NULL, 16);
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		ret = otp_prog_image(addr, 1);
	} else if (argc == 2) {
		addr = simple_strtoul(argv[1], NULL, 16);
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		ret = otp_prog_image(addr, 0);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else if (ret == OTP_FAILURE)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_USAGE;
}

static int do_otppb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int mode = 0;
	int nconfirm = 0;
	int otp_addr = 0;
	int bit_offset;
	int value;
	int ret;

	if (argc != 4 && argc != 5 && argc != 6)
		return CMD_RET_USAGE;

	/* Drop the pb cmd */
	argc--;
	argv++;

	if (!strcmp(argv[0], "conf"))
		mode = OTP_REGION_CONF;
	else if (!strcmp(argv[0], "strap"))
		mode = OTP_REGION_STRAP;
	else if (!strcmp(argv[0], "data"))
		mode = OTP_REGION_DATA;
	else
		return CMD_RET_USAGE;

	/* Drop the region cmd */
	argc--;
	argv++;

	if (!strcmp(argv[0], "o")) {
		nconfirm = 1;
		/* Drop the force option */
		argc--;
		argv++;
	}

	if (mode == OTP_REGION_STRAP) {
		bit_offset = simple_strtoul(argv[0], NULL, 16);
		value = simple_strtoul(argv[1], NULL, 16);
		if (bit_offset >= 64 || (value != 0 && value != 1))
			return CMD_RET_USAGE;
	} else {
		otp_addr = simple_strtoul(argv[0], NULL, 16);
		bit_offset = simple_strtoul(argv[1], NULL, 16);
		value = simple_strtoul(argv[2], NULL, 16);
		if (bit_offset >= 32 || (value != 0 && value != 1))
			return CMD_RET_USAGE;
		if (mode == OTP_REGION_DATA) {
			if (otp_addr >= 0x800)
				return CMD_RET_USAGE;
		} else {
			if (otp_addr >= 0x20)
				return CMD_RET_USAGE;
		}
	}
	if (value != 0 && value != 1)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
	ret = otp_prog_bit(mode, otp_addr, bit_offset, value, nconfirm);

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else if (ret == OTP_FAILURE)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_USAGE;
}

static int do_otpcmp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t addr;
	int otp_addr = 0;

	if (argc != 3)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
	addr = simple_strtoul(argv[1], NULL, 16);
	otp_addr = simple_strtoul(argv[2], NULL, 16);
	if (otp_compare(otp_addr, addr) == 0) {
		printf("Compare pass\n");
		return CMD_RET_SUCCESS;
	}
	printf("Compare fail\n");
	return CMD_RET_FAILURE;
}

static int do_otpinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int view = 0;
	int input;

	if (argc != 2 && argc != 3)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "conf")) {
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		if (argc == 3) {
			input = simple_strtoul(argv[2], NULL, 16);
			otp_print_conf_info(input);
		} else {
			otp_print_conf_info(-1);
		}
	} else if (!strcmp(argv[1], "strap")) {
		if (!strcmp(argv[2], "v")) {
			view = 1;
			/* Drop the view option */
			argc--;
			argv++;
		}
		writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
		otp_print_strap_info(view);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int _do_otpprotect(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[], int preg)
{
	int input;
	int bit_offset;
	u32 prog_address;
	int ret;
	char info[10];

	if (preg) {
		sprintf(info, "register ");
		prog_address = 0xe08;
	} else {
		info[0] = 0;
		prog_address = 0xe0c;
	}

	if (argc != 3 && argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "o")) {
		input = simple_strtoul(argv[2], NULL, 16);
	} else {
		input = simple_strtoul(argv[1], NULL, 16);
		printf("OTPSTRAP[%d] %swill be protected\n", input, info);
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return CMD_RET_FAILURE;
		}
	}

	if (input < 32) {
		bit_offset = input;
	} else if (input < 64) {
		bit_offset = input - 32;
		prog_address += 2;
	} else {
		return CMD_RET_USAGE;
	}

	writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
	if (verify_bit(prog_address, bit_offset, 1) == 0) {
		printf("OTPSTRAP[%d] %salready protected\n", input, info);
		return CMD_RET_SUCCESS;
	}

	ret = otp_prog_dc_b(1, prog_address, bit_offset);
	otp_soak(0);

	if (ret) {
		printf("Protect OTPSTRAP[%d] %sfail\n", input, info);
		return CMD_RET_FAILURE;
	}

	printf("OTPSTRAP[%d] %sis protected\n", input, info);
	return CMD_RET_SUCCESS;
}

static int do_otpprotect(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return _do_otpprotect(cmdtp, flag, argc, argv, 0);
}

static int do_otprprotect(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return _do_otpprotect(cmdtp, flag, argc, argv, 1);
}

static int do_otpver(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printf("SOC OTP version: %s\n", info_cb.ver_name);
	printf("OTP tool version: %s\n", OTP_VER);
	printf("OTP info version: %s\n", OTP_INFO_VER);

	return CMD_RET_SUCCESS;
}

static int do_otpupdate(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 update_num;
	int force = 0;
	int ret;

	if (argc == 3) {
		if (strcmp(argv[1], "o"))
			return CMD_RET_USAGE;
		force = 1;
		update_num = simple_strtoul(argv[2], NULL, 16);
	} else if (argc == 2) {
		update_num = simple_strtoul(argv[1], NULL, 16);
	} else {
		return CMD_RET_USAGE;
	}

	if (update_num > 64)
		return CMD_RET_USAGE;
	writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
	ret = otp_update_rid(update_num, force);
	if (ret)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}

static int do_otprid(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 otp_rid[2];
	int rid_num = 0;
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, OTP_PROTECT_KEY); //password
	otp_read_conf(10, &otp_rid[0]);
	otp_read_conf(11, &otp_rid[1]);

	rid_num = get_rid_num(otp_rid);

	if (rid_num >= 0) {
		printf("current OTP revision ID: 0x%x\n", rid_num);
		ret = CMD_RET_SUCCESS;
	} else {
		printf("Currennt OTP revision ID cannot handle by 'otp update',\n"
		       "plase use 'otp pb' command to update it manually\n"
		       "current OTP revision ID\n");
		ret = CMD_RET_FAILURE;
	}
	otp_print_revid(otp_rid);

	return ret;
}

static cmd_tbl_t cmd_otp[] = {
	U_BOOT_CMD_MKENT(version, 1, 0, do_otpver, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 0, do_otpread, "", ""),
	U_BOOT_CMD_MKENT(info, 3, 0, do_otpinfo, "", ""),
	U_BOOT_CMD_MKENT(prog, 3, 0, do_otpprog, "", ""),
	U_BOOT_CMD_MKENT(pb, 6, 0, do_otppb, "", ""),
	U_BOOT_CMD_MKENT(protect, 3, 0, do_otpprotect, "", ""),
	U_BOOT_CMD_MKENT(rprotect, 3, 0, do_otprprotect, "", ""),
	U_BOOT_CMD_MKENT(cmp, 3, 0, do_otpcmp, "", ""),
	U_BOOT_CMD_MKENT(update, 3, 0, do_otpupdate, "", ""),
	U_BOOT_CMD_MKENT(rid, 1, 0, do_otprid, "", ""),
};

static int do_ast_otp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *cp;
	u32 ver;
	int ret;

	cp = find_cmd_tbl(argv[1], cmd_otp, ARRAY_SIZE(cmd_otp));

	/* Drop the otp command */
	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	ver = chip_version();
	switch (ver) {
	case OTP_A0:
		info_cb.version = OTP_A0;
		info_cb.conf_info = a0_conf_info;
		info_cb.conf_info_len = ARRAY_SIZE(a0_conf_info);
		info_cb.strap_info = a0_strap_info;
		info_cb.strap_info_len = ARRAY_SIZE(a0_strap_info);
		info_cb.key_info = a0_key_type;
		info_cb.key_info_len = ARRAY_SIZE(a0_key_type);
		sprintf(info_cb.ver_name, "A0");
		break;
	case OTP_A1:
		info_cb.version = OTP_A1;
		info_cb.conf_info = a1_conf_info;
		info_cb.conf_info_len = ARRAY_SIZE(a1_conf_info);
		info_cb.strap_info = a1_strap_info;
		info_cb.strap_info_len = ARRAY_SIZE(a1_strap_info);
		info_cb.key_info = a1_key_type;
		info_cb.key_info_len = ARRAY_SIZE(a1_key_type);
		sprintf(info_cb.ver_name, "A1");
		break;
	case OTP_A2:
		info_cb.version = OTP_A2;
		info_cb.conf_info = a2_conf_info;
		info_cb.conf_info_len = ARRAY_SIZE(a2_conf_info);
		info_cb.strap_info = a2_strap_info;
		info_cb.strap_info_len = ARRAY_SIZE(a2_strap_info);
		info_cb.key_info = a2_key_type;
		info_cb.key_info_len = ARRAY_SIZE(a2_key_type);
		sprintf(info_cb.ver_name, "A2");
		break;
	case OTP_A3:
		info_cb.version = OTP_A3;
		info_cb.conf_info = a2_conf_info;
		info_cb.conf_info_len = ARRAY_SIZE(a2_conf_info);
		info_cb.strap_info = a2_strap_info;
		info_cb.strap_info_len = ARRAY_SIZE(a2_strap_info);
		info_cb.key_info = a3_key_type;
		info_cb.key_info_len = ARRAY_SIZE(a3_key_type);
		sprintf(info_cb.ver_name, "A3");
		break;
	default:
		printf("SOC is not supported\n");
		return CMD_RET_FAILURE;
	}

	ret = cp->cmd(cmdtp, flag, argc, argv);
	writel(1, OTP_PROTECT_KEY); //password

	return ret;
}

U_BOOT_CMD(otp, 7, 0,  do_ast_otp,
	   "ASPEED One-Time-Programmable sub-system",
	   "version\n"
	   "otp read conf|data <otp_dw_offset> <dw_count>\n"
	   "otp read strap <strap_bit_offset> <bit_count>\n"
	   "otp info strap [v]\n"
	   "otp info conf [otp_dw_offset]\n"
	   "otp prog [o] <addr>\n"
	   "otp pb conf|data [o] <otp_dw_offset> <bit_offset> <value>\n"
	   "otp pb strap [o] <bit_offset> <value>\n"
	   "otp protect [o] <bit_offset>\n"
	   "otp rprotect [o] <bit_offset>\n"
	   "otp update [o] <revision_id>\n"
	   "otp rid\n"
	  );
