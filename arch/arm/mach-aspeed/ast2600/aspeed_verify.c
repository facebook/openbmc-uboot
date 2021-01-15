/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <asm/arch/aspeed_verify.h>

static int aspeed_digest_verify(struct aspeed_verify_info *info)
{
	int digest_length = 64;
	int ret = 0;
	u8 *digest_result;

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

	enable_crypto();
	digest_result = memalign(8, digest_length);
	ret = digest_object(info->image, info->image_size, digest_result, info->sha_mode);
	if (ret)
		goto err;


	ret = memcmp(info->digest, digest_result, digest_length);
err:
	return ret;
}

static int aspeed_signature_verify(struct aspeed_verify_info *info)
{
	int digest_length;
	int ret = 0;
	int m_bits;
	u8 *digest_result;
	u8 *rsa_result;
	u8 *contex_buf;
	u8 *rsa_m;
	u8 *rsa_e;

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
	default:
		digest_length = 0;
	}

	digest_result = memalign(8, digest_length);
	enable_crypto();
	ret = digest_object(info->image, info->image_size, digest_result, info->sha_mode);
	if (ret)
		goto err;

	contex_buf = malloc(0x600);
	memset(contex_buf, 0, 0x600);

	rsa_result = malloc(0x200);
	rsa_m = info->rsa_key;
	rsa_e = NULL;

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
	default:
		m_bits = 0;
		rsa_e = NULL;
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

int _aspeed_verify_boot(u32 cot_alg, u8 *cot_info, void *verify_image)
{
	struct aspeed_verify_info info;
	struct aspeed_secboot_header *sbh;
	int ret = 0;

	sbh = (struct aspeed_secboot_header *)verify_image;

	if (strcmp((char *)sbh->sbh_magic, ASPEED_SECBOOT_MAGIC_STR)) {
		debug("secure_boot: cannot find secure boot header\n");
		return -EPERM;
	}

	info.rsa_mode = 0;
	info.signature = NULL;
	info.image = verify_image;
	info.image_size = sbh->sbh_img_size;
	info.sha_mode = ASPEED_VERIFY_SHA(cot_alg);
	info.verify_mode = ASPEED_VERIFY_MODE(cot_alg);
	info.digest = NULL;
	info.rsa_key = NULL;

	printf("## Starting verify image.\n");
	switch (info.verify_mode) {
	case DIGEST_MODE:
		printf("   Verifing Hash Integrity ... ");
		info.digest = cot_info;
		ret = aspeed_digest_verify(&info);
		break;
	case SIGN_MODE:
		printf("   Verifing Signature ... ");
		info.rsa_mode = ASPEED_VERIFY_RSA(cot_alg);
		info.e_len = ASPEED_VERIFY_E_LEN(cot_alg);
		info.rsa_key = cot_info;
		info.signature = verify_image + sbh->sbh_sig_off;
		ret = aspeed_signature_verify(&info);
		break;
	}
	if (ret != 0) {
		printf("Failed to verify.\n");
	} else {
		printf("OK.\n");
	}

	print_verification_data(&info, "     ");
	return ret;
}

int aspeed_verify_boot(void *cur_image, void *next_image)
{
	struct aspeed_secboot_header *cur_sbh;
	u32 cot_alg;
	u8 *cot_info;

	cur_sbh = (struct aspeed_secboot_header *)cur_image;
	cot_alg = cur_sbh->sbh_cot_alg;
	cot_info = cur_image + cur_sbh->sbh_cot_info_off;


	return _aspeed_verify_boot(cot_alg, cot_info, next_image);
}

/**
 * Aspeed verified boot
 *
 * \param bl2_image	u-boot image offset
 * \param bl1_image	spl image offset
 */
int aspeed_bl2_verify(void *bl2_image, void *bl1_image)
{
	u32 cot_alg = * (u32 *)(bl1_image + ASPEED_VERIFY_HEADER);
	u8 *cot_info = (u8 *) * (u32 *)(bl1_image + ASPEED_VERIFY_INFO_OFFSET);

	return _aspeed_verify_boot(cot_alg, cot_info, bl2_image);
}