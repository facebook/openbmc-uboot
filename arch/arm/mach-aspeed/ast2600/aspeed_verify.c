/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <asm/arch/aspeed_verify.h>

static int aspeed_digest_verify(struct aspeed_verify_info *info)
{
	int digest_length = 64;
	int ret = 0;
	u8 *digest_result = info->verify_dram_offset;

	enable_crypto();
	ret = digest_object(info->image, info->image_size, digest_result, info->sha_mode);
	if (ret)
		goto err;

	switch (info->sha_mode) {
	case ASPEED_SHA224:
		digest_length = 28;
		break;
	case ASPEED_SHA256:
		digest_length = 32;
		break;
	case ASPEED_SHA384:
		digest_length = 48;
		break;
	case ASPEED_SHA512:
		digest_length = 64;
		break;
	}

	ret = memcmp(info->digest, info->verify_dram_offset, digest_length);
err:
	return ret;
}

static int aspeed_signature_verify(struct aspeed_verify_info *info)
{
	int digest_length = 64;
	int ret = 0;
	int m_bits;
	u8 *digest_result = info->verify_dram_offset;
	u8 *rsa_result = digest_result + 0x40;
	u8 *contex_buf = rsa_result + 0x200;
	u8 *rsa_m;
	u8 *rsa_e;

	enable_crypto();
	ret = digest_object(info->image, info->image_size, digest_result, info->sha_mode);
	if (ret)
		goto err;

	switch (info->sha_mode) {
	case ASPEED_SHA224:
		digest_length = 28;
		break;
	case ASPEED_SHA256:
		digest_length = 32;
		break;
	case ASPEED_SHA384:
		digest_length = 48;
		break;
	case ASPEED_SHA512:
		digest_length = 64;
		break;
	}

	memset(contex_buf, 0, 0x600);

	rsa_m = info->rsa_key;

	switch (info->rsa_mode) {
	case ASPEED_RSA1024:
		m_bits = 1024;
		rsa_e = info->rsa_key + 0x80;
		break;
	case ASPEED_RSA2048:
		m_bits = 2048;
		rsa_e = info->rsa_key + 0x100;
		break;
	case ASPEED_RSA3072:
		m_bits = 3072;
		rsa_e = info->rsa_key + 0x180;
		break;
	case ASPEED_RSA4096:
		m_bits = 4096;
		rsa_e = info->rsa_key + 0x200;
		break;
	}

	ret = rsa_alg(info->signature, m_bits / 8, rsa_m, m_bits, rsa_e, info->e_len, rsa_result, contex_buf);
	if (ret)
		goto err;

	ret = memcmp(digest_result, rsa_result, digest_length);

err:
	return ret;
}

static void print_hash_data(const struct aspeed_verify_info *info,
			    const char *p)
{
	int digest_length = 64;
	char hash_algo[10];
	int i;

	switch (info->sha_mode) {
	case ASPEED_SHA224:
		digest_length = 28;
		strcpy(hash_algo, "sha224");
		break;
	case ASPEED_SHA256:
		digest_length = 32;
		strcpy(hash_algo, "sha256");
		break;
	case ASPEED_SHA384:
		digest_length = 48;
		strcpy(hash_algo, "sha384");
		break;
	case ASPEED_SHA512:
		digest_length = 64;
		strcpy(hash_algo, "sha512");
		break;
	}
	printf("%s Hash algo:    %s\n", p, hash_algo);
	printf("%s Hash value:   ", p);
	for (i = 0; i < digest_length; i++) {
		printf("%02x", info->digest[i]);
		if ((i + 1) % 32 == 0)
			printf("\n%s                ", p);
	}
	printf("\n");
}

static void print_sign_data(const struct aspeed_verify_info *info,
			    const char *p)
{
	int sign_length = 512;
	char hash_algo[10];
	char rsa_algo[10];
	int i;

	switch (info->sha_mode) {
	case ASPEED_SHA224:
		strcpy(hash_algo, "sha224");
		break;
	case ASPEED_SHA256:
		strcpy(hash_algo, "sha256");
		break;
	case ASPEED_SHA384:
		strcpy(hash_algo, "sha384");
		break;
	case ASPEED_SHA512:
		strcpy(hash_algo, "sha512");
		break;
	}
	switch (info->rsa_mode) {
	case ASPEED_RSA1024:
		sign_length = 128;
		strcpy(rsa_algo, "rsa1024");
		break;
	case ASPEED_RSA2048:
		sign_length = 256;
		strcpy(rsa_algo, "rsa2048");
		break;
	case ASPEED_RSA3072:
		sign_length = 384;
		strcpy(rsa_algo, "rsa3072");
		break;
	case ASPEED_RSA4096:
		sign_length = 512;
		strcpy(rsa_algo, "rsa4096");
		break;
	}
	printf("%s Sign algo:    %s,%s\n", p, hash_algo, rsa_algo);
	printf("%s Sign value:   ", p);
	for (i = 0; i < sign_length; i++) {
		printf("%02x", info->signature[i]);
		if ((i + 1) % 32 == 0)
			printf("\n%s               ", p);
	}
	printf("\n");
}

static void print_verification_data(const struct aspeed_verify_info *info,
				    const char *p)
{
	switch (info->verify_mode) {
	case DIGEST_MODE:
		print_hash_data(info, p);
		break;
	case SIGN_MODE:
		print_sign_data(info, p);
		break;
	}
}

/**
 * Aspeed verified boot
 *
 * \param bl2_image	u-boot image offset
 * \param bl1_image	spl image offset
 */

int aspeed_bl2_verify(void *bl2_image, void *bl1_image)
{
	struct aspeed_verify_info info;
	u8 *bl1_info = (u8 *) * (u32 *)(bl1_image + ASPEED_VERIFY_INFO_OFFSET);
	u32 bl1_header = * (u32 *)(bl1_image + ASPEED_VERIFY_HEADER);
	u32 sing_offset;
	int ret = 0;

	info.image = bl2_image;
	info.verify_dram_offset = (u8 *)ALIGN((u32) CONFIG_SYS_SDRAM_BASE, 16);
	info.image_size = *(u32 *)(bl2_image + ASPEED_VERIFY_SIZE);
	info.sha_mode = ASPEED_VERIFY_SHA(bl1_header);
	info.verify_mode = ASPEED_VERIFY_MODE(bl1_header);
	printf("## Starting verify image.\n");
	switch (info.verify_mode) {
	case DIGEST_MODE:
		printf("   Verifing Hash Integrity ... ");
		info.digest = bl1_info;
		ret = aspeed_digest_verify(&info);
		break;
	case SIGN_MODE:
		printf("   Verifing Signature ... ");
		info.rsa_mode = ASPEED_VERIFY_RSA(bl1_header);
		info.e_len = ASPEED_VERIFY_E_LEN(bl1_header);
		info.rsa_key = bl1_info;
		sing_offset = * (u32 *)(bl2_image + ASPEED_VERIFY_SIGN_OFFSET);
		info.signature = (u8 *)(sing_offset + (u32)bl2_image);
		ret = aspeed_signature_verify(&info);
		break;
	}
	if (ret != 0) {
		printf("Failed to verify.\n");
	} else {
		printf("OK.\n");
		print_verification_data(&info, "     ");
	}
	return ret;
}
