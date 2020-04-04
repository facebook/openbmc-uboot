/*
 * (C) Copyright ASPEED Technology Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#define ASPEED_SHA224		0x0
#define ASPEED_SHA256		0x1
#define ASPEED_SHA384		0x2
#define ASPEED_SHA512		0x3

#define ASPEED_RSA1024		0x0
#define ASPEED_RSA2048		0x1
#define ASPEED_RSA3072		0x2
#define ASPEED_RSA4096		0x3

#define ASPEED_HACE_BASE		(0x1E6D0000)
#define ASPEED_HACE_SRC			0x00
#define ASPEED_HACE_DEST		0x04
#define ASPEED_HACE_CONTEXT		0x08	/* 8 byte aligned*/
#define ASPEED_HACE_DATA_LEN		0x0C
#define ASPEED_HACE_CMD			0x10
#define ASPEED_HACE_STS			(ASPEED_HACE_BASE + 0x1C)
#define  HACE_RSA_ISR			BIT(13)
#define  HACE_CRYPTO_ISR		BIT(12)
#define  HACE_HASH_ISR			BIT(9)
#define  HACE_RSA_BUSY			BIT(2)
#define  HACE_CRYPTO_BUSY		BIT(1)
#define  HACE_HASH_BUSY			BIT(0)
#define ASPEED_HACE_HASH_SRC		(ASPEED_HACE_BASE + 0x20)
#define ASPEED_HACE_HASH_DIGEST_BUFF	(ASPEED_HACE_BASE + 0x24)
#define ASPEED_HACE_HASH_KEY_BUFF	(ASPEED_HACE_BASE + 0x28)	// 64 byte aligned,g6 16 byte aligned
#define ASPEED_HACE_HASH_DATA_LEN	(ASPEED_HACE_BASE + 0x2C)
#define ASPEED_HACE_HASH_CMD		(ASPEED_HACE_BASE + 0x30)

extern int digest_object(u8 *src, u32 length, u8 *digest, u32 method);
extern int aes256ctr_decrypt_object(u8 *src, u8 *dst, u32 length, u8 *context);
extern int rsa_alg(u8 *data, int data_bytes, u8 *m, int m_bits, u8 *e, int e_bits, u8 *dst, void *contex_buf);
extern void enable_crypto(void);

#endif /* #ifndef _CRYPTO_H_ */
