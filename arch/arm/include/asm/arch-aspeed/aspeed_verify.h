/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASPEED_VERIFY_H_
#define _ASPEED_VERIFY_H_

#include <asm/arch/crypto.h>

/* In SPL */
#define ASPEED_VERIFY_HEADER		0x40

#define ASPEED_VERIFY_MODE(x)		(x & 0x1)
#define  DIGEST_MODE			0x0
#define  SIGN_MODE			0x1
#define ASPEED_VERIFY_SHA(x)		((x >> 1) & 0x3)
#define ASPEED_VERIFY_RSA(x)		((x >> 3) & 0x3)
#define ASPEED_VERIFY_E_LEN(x)		((x >> 20) & 0xfff)

#define ASPEED_VERIFY_INFO_OFFSET	0x44

/* In u-boot */
#define ASPEED_VERIFY_SIZE		0x20
#define ASPEED_VERIFY_SIGN_OFFSET	0x24

#define ASPEED_SECBOOT_MAGIC_STR	"SOCSEC"

struct aspeed_secboot_header {
	u8 sbh_magic[16];
	u32 sbh_cot_alg;
	u32 sbh_img_size;
	u32 sbh_sig_off;
	u32 sbh_cot_info_off;
	u64 sbh_rollback_idx;
	u8 reserved[472];
} __packed;

struct aspeed_secboot_info {
	struct aspeed_secboot_header sb_hdr;
	u8 signature[512];
};

struct aspeed_verify_info {
	u8 rsa_mode;
	u8 sha_mode;
	u8 verify_mode;
	u32 e_len;
	u32 image_size;
	u8 *image;
	u8 *verify_dram_offset;
	u8 *signature;
	u8 *digest;
	u8 *rsa_key;
	struct aspeed_secboot_header *sb_header;
};

extern int aspeed_bl2_verify(void *bl2_image, void *bl1_image);
extern int aspeed_verify_boot(void *cur_image, void *next_image);

#endif /* #ifndef _ASPEED_VERIFY_H_ */
