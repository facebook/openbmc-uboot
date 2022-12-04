// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright ASPEED Technology Inc.
 * Copyright 2021 IBM Corp.
 */
#include <common.h>
#include <clk.h>

#include <log.h>
#include <asm/io.h>
#include <malloc.h>
#include <hash.h>

#include <dm/device.h>
#include <dm/fdtaddr.h>

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/iopoll.h>

#define ASPEED_HACE_STS			0x1C
#define  HACE_RSA_ISR			BIT(13)
#define  HACE_CRYPTO_ISR		BIT(12)
#define  HACE_HASH_ISR			BIT(9)
#define  HACE_RSA_BUSY			BIT(2)
#define  HACE_CRYPTO_BUSY		BIT(1)
#define  HACE_HASH_BUSY			BIT(0)
#define ASPEED_HACE_HASH_SRC		0x20
#define ASPEED_HACE_HASH_DIGEST_BUFF	0x24
#define ASPEED_HACE_HASH_KEY_BUFF	0x28
#define ASPEED_HACE_HASH_DATA_LEN	0x2C
#define  HACE_SG_LAST			BIT(31)
#define ASPEED_HACE_HASH_CMD		0x30
#define  HACE_SHA_BE_EN			BIT(3)
#define  HACE_MD5_LE_EN			BIT(2)
#define  HACE_ALGO_MD5			0
#define  HACE_ALGO_SHA1			BIT(5)
#define  HACE_ALGO_SHA224		BIT(6)
#define  HACE_ALGO_SHA256		(BIT(4) | BIT(6))
#define  HACE_ALGO_SHA512		(BIT(5) | BIT(6))
#define  HACE_ALGO_SHA384		(BIT(5) | BIT(6) | BIT(10))
#define  HASH_CMD_ACC_MODE		(0x2 << 7)
#define  HACE_SG_EN			BIT(18)

#define ASPEED_SHA1_DIGEST_SIZE		20
#define ASPEED_SHA256_DIGEST_SIZE	32
#define ASPEED_SHA384_DIGEST_SIZE	64
#define ASPEED_SHA512_DIGEST_SIZE	64

#define ASPEED_SHA_TYPE_SHA1		1
#define ASPEED_SHA_TYPE_SHA256		2
#define ASPEED_SHA_TYPE_SHA384		3
#define ASPEED_SHA_TYPE_SHA512		4

struct aspeed_sg {
	u32 len;
	u32 addr;
};

struct aspeed_hash_ctx {
	struct aspeed_sg sg[2]; /* Must be 8 byte aligned */
	u8 digest[64]; /* Must be 8 byte aligned */
	u32 method;
	u32 digest_size;
	u32 block_size;
	u64 digcnt[2]; /* total length */
	u32 bufcnt;
	u8 buffer[256];
};

struct aspeed_hace {
	struct clk clk;
};

static const u32 sha1_iv[8] = {
	0x01234567UL, 0x89abcdefUL, 0xfedcba98UL, 0x76543210UL,
	0xf0e1d2c3UL, 0, 0, 0
};

static const u32 sha256_iv[8] = {
	0x67e6096aUL, 0x85ae67bbUL, 0x72f36e3cUL, 0x3af54fa5UL,
	0x7f520e51UL, 0x8c68059bUL, 0xabd9831fUL, 0x19cde05bUL
};

static const u32 sha384_iv[16] = {
	0x5d9dbbcbUL, 0xd89e05c1UL, 0x2a299a62UL, 0x07d57c36UL,
	0x5a015991UL, 0x17dd7030UL, 0xd8ec2f15UL, 0x39590ef7UL,
	0x67263367UL, 0x310bc0ffUL, 0x874ab48eUL, 0x11155868UL,
	0x0d2e0cdbUL, 0xa78ff964UL, 0x1d48b547UL, 0xa44ffabeUL
};

static const u32 sha512_iv[16] = {
	0x67e6096aUL, 0x08c9bcf3UL, 0x85ae67bbUL, 0x3ba7ca84UL,
	0x72f36e3cUL, 0x2bf894feUL, 0x3af54fa5UL, 0xf1361d5fUL,
	0x7f520e51UL, 0xd182e6adUL, 0x8c68059bUL, 0x1f6c3e2bUL,
	0xabd9831fUL, 0x6bbd41fbUL, 0x19cde05bUL, 0x79217e13UL
};

static phys_addr_t base;

static int aspeed_hace_wait_completion(u32 reg, u32 flag, int timeout_us)
{
	u32 val;

	return readl_poll_timeout(reg, val, (val & flag) == flag, timeout_us);
}

static void aspeed_ahash_fill_padding(struct aspeed_hash_ctx *ctx, unsigned int remainder)
{
	unsigned int index, padlen;
	u64 bits[2];

	if (ctx->block_size == 64) {
		bits[0] = cpu_to_be64(ctx->digcnt[0] << 3);
		index = (ctx->bufcnt + remainder) & 0x3f;
		padlen = (index < 56) ? (56 - index) : ((64 + 56) - index);
		*(ctx->buffer + ctx->bufcnt) = 0x80;
		memset(ctx->buffer + ctx->bufcnt + 1, 0, padlen - 1);
		memcpy(ctx->buffer + ctx->bufcnt + padlen, bits, 8);
		ctx->bufcnt += padlen + 8;
	} else {
		bits[1] = cpu_to_be64(ctx->digcnt[0] << 3);
		bits[0] = cpu_to_be64(ctx->digcnt[1] << 3 | ctx->digcnt[0] >> 61);
		index = (ctx->bufcnt + remainder) & 0x7f;
		padlen = (index < 112) ? (112 - index) : ((128 + 112) - index);
		*(ctx->buffer + ctx->bufcnt) = 0x80;
		memset(ctx->buffer + ctx->bufcnt + 1, 0, padlen - 1);
		memcpy(ctx->buffer + ctx->bufcnt + padlen, bits, 16);
		ctx->bufcnt += padlen + 16;
	}
}

static int hash_trigger(struct aspeed_hash_ctx *ctx, int hash_len)
{
	if (readl(base + ASPEED_HACE_STS) & HACE_HASH_BUSY) {
		debug("HACE error: engine busy\n");
		return -EBUSY;
	}
	/* Clear pending completion status */
	writel(HACE_HASH_ISR, base + ASPEED_HACE_STS);

	writel((u32)ctx->sg, base + ASPEED_HACE_HASH_SRC);
	writel((u32)ctx->digest, base + ASPEED_HACE_HASH_DIGEST_BUFF);
	writel((u32)ctx->digest, base + ASPEED_HACE_HASH_KEY_BUFF);
	writel(hash_len, base + ASPEED_HACE_HASH_DATA_LEN);
	writel(ctx->method, base + ASPEED_HACE_HASH_CMD);

	/* SHA512 hashing appears to have a througput of about 12MB/s */
	return aspeed_hace_wait_completion(base + ASPEED_HACE_STS,
					   HACE_HASH_ISR,
					   1000 + (hash_len >> 3));
}

#if IS_ENABLED(CONFIG_SHA_PROG_HW_ACCEL)
int hw_sha_init(struct hash_algo *algo, void **ctxp)
{
	struct aspeed_hash_ctx *ctx;
	u32 method;
	u32 block_size;

	ctx = memalign(8, sizeof(struct aspeed_hash_ctx));
	memset(ctx, '\0', sizeof(struct aspeed_hash_ctx));

	method = HASH_CMD_ACC_MODE | HACE_SHA_BE_EN | HACE_SG_EN;
	if (!strcmp(algo->name, "sha1")) {
		method |= HACE_ALGO_SHA1;
		block_size = 64;
		memcpy(ctx->digest, sha1_iv, 32);
	} else if (!strcmp(algo->name, "sha256")) {
		method |= HACE_ALGO_SHA256;
		block_size = 64;
		memcpy(ctx->digest, sha256_iv, 32);
	} else if (!strcmp(algo->name, "sha384")) {
		method |= HACE_ALGO_SHA384;
		block_size = 128;
		memcpy(ctx->digest, sha384_iv, 64);
	} else if (!strcmp(algo->name, "sha512")) {
		method |= HACE_ALGO_SHA512;
		block_size = 128;
		memcpy(ctx->digest, sha512_iv, 64);
	} else {
		return -ENOTSUPP;
	}

	if (!ctx) {
		debug("HACE error: Cannot allocate memory for context\n");
		return -ENOMEM;
	}

	ctx->method = method;
	ctx->block_size = block_size;
	ctx->digest_size = algo->digest_size;
	ctx->bufcnt = 0;
	ctx->digcnt[0] = 0;
	ctx->digcnt[1] = 0;
	*ctxp = ctx;

	return 0;
}

int hw_sha_update(struct hash_algo *algo, void *hash_ctx, const void *buf,
		  unsigned int size, int is_last)
{
	struct aspeed_hash_ctx *ctx = hash_ctx;
	struct aspeed_sg *sg = ctx->sg;
	int rc;
	int remainder;
	int total_len;
	int i;

	ctx->digcnt[0] += size;
	if (ctx->digcnt[0] < size)
		ctx->digcnt[1]++;

	if (ctx->bufcnt + size < ctx->block_size) {
		memcpy(ctx->buffer + ctx->bufcnt, buf, size);
		ctx->bufcnt += size;
		return 0;
	}
	remainder = (size + ctx->bufcnt) % ctx->block_size;
	total_len = size + ctx->bufcnt - remainder;
	i = 0;
	if (ctx->bufcnt != 0) {
		sg[0].addr = (u32)ctx->buffer;
		sg[0].len = ctx->bufcnt;
		if (total_len == ctx->bufcnt)
			sg[0].len |= HACE_SG_LAST;
		i++;
	}

	if (total_len != ctx->bufcnt) {
		sg[i].addr = (u32)buf;
		sg[i].len = (total_len - ctx->bufcnt) | HACE_SG_LAST;
	}

	rc = hash_trigger(ctx, total_len);
	if (remainder != 0) {
		memcpy(ctx->buffer, buf + (total_len - ctx->bufcnt), remainder);
		ctx->bufcnt = remainder;
	} else {
		ctx->bufcnt = 0;
	}

	return rc;
}

int hw_sha_finish(struct hash_algo *algo, void *hash_ctx, void *dest_buf, int size)
{
	struct aspeed_hash_ctx *ctx = hash_ctx;
	struct aspeed_sg *sg = ctx->sg;
	int rc;

	if (size < ctx->digest_size) {
		debug("HACE error: insufficient size on destination buffer\n");
		free(ctx);
		return -EINVAL;
	}
	aspeed_ahash_fill_padding(ctx, 0);

	sg[0].addr = (u32)ctx->buffer;
	sg[0].len = ctx->bufcnt | HACE_SG_LAST;

	rc = hash_trigger(ctx, ctx->bufcnt);
	memcpy(dest_buf, ctx->digest, ctx->digest_size);

	free(ctx);

	return rc;
}
#endif

static int sha_digest(const void *src, unsigned int length, void *digest,
		      u32 sha_type)
{
	struct aspeed_hash_ctx *ctx;
	int ret;

	if (!((u32)src & BIT(31))) {
		debug("HACE src out of bounds: can only copy from SDRAM\n");
		return -EINVAL;
	}

	if (readl(base + ASPEED_HACE_STS) & HACE_HASH_BUSY) {
		debug("HACE error: engine busy\n");
		return -EBUSY;
	}

	ctx = memalign(8, sizeof(struct aspeed_hash_ctx));
	memset(ctx, '\0', sizeof(struct aspeed_hash_ctx));

	if (!ctx) {
		debug("HACE error: Cannot allocate memory for context\n");
		return -ENOMEM;
	}
	ctx->method = HASH_CMD_ACC_MODE | HACE_SHA_BE_EN | HACE_SG_EN;

	switch (sha_type) {
	case ASPEED_SHA_TYPE_SHA1:
		ctx->block_size = 64;
		ctx->digest_size = 20;
		ctx->method |= HACE_ALGO_SHA1;
		memcpy(ctx->digest, sha1_iv, 32);
		break;
	case ASPEED_SHA_TYPE_SHA256:
		ctx->block_size = 64;
		ctx->digest_size = 32;
		ctx->method |= HACE_ALGO_SHA256;
		memcpy(ctx->digest, sha256_iv, 32);
		break;
	case ASPEED_SHA_TYPE_SHA384:
		ctx->block_size = 128;
		ctx->digest_size = 64;
		ctx->method |= HACE_ALGO_SHA384;
		memcpy(ctx->digest, sha384_iv, 64);
		break;
	case ASPEED_SHA_TYPE_SHA512:
		ctx->block_size = 128;
		ctx->digest_size = 64;
		ctx->method |= HACE_ALGO_SHA512;
		memcpy(ctx->digest, sha512_iv, 64);
		break;
	default:
		return -ENOTSUPP;
	}

	ctx->digcnt[0] = length;
	ctx->digcnt[1] = 0;

	aspeed_ahash_fill_padding(ctx, length);

	if (length != 0) {
		ctx->sg[0].addr = (u32)src;
		ctx->sg[0].len = length;
		ctx->sg[1].addr = (u32)ctx->buffer;
		ctx->sg[1].len = ctx->bufcnt | HACE_SG_LAST;
	} else {
		ctx->sg[0].addr = (u32)ctx->buffer;
		ctx->sg[0].len = ctx->bufcnt | HACE_SG_LAST;
	}

	ret = hash_trigger(ctx, length + ctx->bufcnt);
	memcpy(digest, ctx->digest, ctx->digest_size);
	free(ctx);

	return ret;
}

void hw_sha1(const unsigned char *pbuf, unsigned int buf_len,
	     unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = sha_digest(pbuf, buf_len, pout, ASPEED_SHA_TYPE_SHA1);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha256(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = sha_digest(pbuf, buf_len, pout, ASPEED_SHA_TYPE_SHA256);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha384(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = sha_digest(pbuf, buf_len, pout, ASPEED_SHA_TYPE_SHA384);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha512(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = sha_digest(pbuf, buf_len, pout, ASPEED_SHA_TYPE_SHA512);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

static int aspeed_hace_probe(struct udevice *dev)
{
	struct aspeed_hace *hace = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_index(dev, 0, &hace->clk);
	if (ret < 0) {
		debug("Can't get clock for %s: %d\n", dev->name, ret);
		return ret;
	}

	ret = clk_enable(&hace->clk);
	if (ret) {
		debug("Failed to enable fsi clock (%d)\n", ret);
		return ret;
	}

	/* As the crypto code does not pass us any driver state */
	base = devfdt_get_addr(dev);

	return ret;
}

static int aspeed_hace_remove(struct udevice *dev)
{
	struct aspeed_hace *hace = dev_get_priv(dev);

	clk_disable(&hace->clk);

	return 0;
}

static const struct udevice_id aspeed_hace_ids[] = {
	{ .compatible = "aspeed,ast2600-hace" },
	{ }
};

U_BOOT_DRIVER(aspeed_hace) = {
	.name		= "aspeed_hace",
	.id		= UCLASS_MISC,
	.of_match	= aspeed_hace_ids,
	.probe		= aspeed_hace_probe,
	.remove		= aspeed_hace_remove,
	.priv_auto_alloc_size = sizeof(struct aspeed_hace),
	.flags  = DM_FLAG_PRE_RELOC,
};
