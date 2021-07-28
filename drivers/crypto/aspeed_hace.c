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
#define  HACE_SG_EN			BIT(18)

#define ASPEED_MAX_SG			32

struct aspeed_sg {
	u32 len;
	u32 addr;
};

struct aspeed_hash_ctx {
	u32 method;
	u32 digest_size;
	u32 len;
	u32 count;
	struct aspeed_sg list[ASPEED_MAX_SG]; /* Must be 8 byte aligned */
};

struct aspeed_hace {
	struct clk clk;
};

static phys_addr_t base;

static int aspeed_hace_wait_completion(u32 reg, u32 flag, int timeout_us)
{
	u32 val;

	return readl_poll_timeout(reg, val, (val & flag) == flag, timeout_us);
}

static int digest_object(const void *src, unsigned int length, void *digest,
			 u32 method)
{
	if (!((u32)src & BIT(31))) {
		debug("HACE src out of bounds: can only copy from SDRAM\n");
		return -EINVAL;
	}

	if ((u32)digest & 0x7) {
		debug("HACE dest alignment incorrect: %p\n", digest);
		return -EINVAL;
	}

	if (readl(base + ASPEED_HACE_STS) & HACE_HASH_BUSY) {
		debug("HACE error: engine busy\n");
		return -EBUSY;
	}

	/* Clear pending completion status */
	writel(HACE_HASH_ISR, base + ASPEED_HACE_STS);

	writel((u32)src, base + ASPEED_HACE_HASH_SRC);
	writel((u32)digest, base + ASPEED_HACE_HASH_DIGEST_BUFF);
	writel(length, base + ASPEED_HACE_HASH_DATA_LEN);
	writel(HACE_SHA_BE_EN | method, base + ASPEED_HACE_HASH_CMD);

	/* SHA512 hashing appears to have a througput of about 12MB/s */
	return aspeed_hace_wait_completion(base + ASPEED_HACE_STS,
			HACE_HASH_ISR,
			1000 + (length >> 3));
}

void hw_sha1(const unsigned char *pbuf, unsigned int buf_len,
	     unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA1);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha256(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA256);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha512(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA512);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

#if IS_ENABLED(CONFIG_SHA_PROG_HW_ACCEL)
int hw_sha_init(struct hash_algo *algo, void **ctxp)
{
	struct aspeed_hash_ctx *ctx;
	u32 method;

	if (!strcmp(algo->name, "sha1"))
		method = HACE_ALGO_SHA1;
	else if (!strcmp(algo->name, "sha256"))
		method = HACE_ALGO_SHA256;
	else if (!strcmp(algo->name, "sha512"))
		method = HACE_ALGO_SHA512;
	else
		return -ENOTSUPP;

	ctx = memalign(8, sizeof(*ctx));
	memset(ctx, '\0', sizeof(*ctx));

	if (!ctx) {
		debug("HACE error: Cannot allocate memory for context\n");
		return -ENOMEM;
	}

	if (((uintptr_t)ctx->list & 0x3) != 0) {
		printf("HACE error: Invalid alignment for input data\n");
		return -EINVAL;
	}

	ctx->method = method | HACE_SG_EN;
	ctx->digest_size = algo->digest_size;
	*ctxp = ctx;

	return 0;
}

int hw_sha_update(struct hash_algo *algo, void *hash_ctx, const void *buf,
		  unsigned int size, int is_last)
{
	struct aspeed_hash_ctx *ctx = hash_ctx;
	struct aspeed_sg *sg = &ctx->list[ctx->count];

	if (ctx->count >= ARRAY_SIZE(ctx->list)) {
		debug("HACE error: Reached maximum number of hash segments\n");
		free(ctx);
		return -EINVAL;
	}

	sg->addr = (u32)buf;
	sg->len = size;
	if (is_last)
		sg->len |= HACE_SG_LAST;

	ctx->count++;
	ctx->len += size;

	return 0;
}

int hw_sha_finish(struct hash_algo *algo, void *hash_ctx, void *dest_buf, int size)
{
	struct aspeed_hash_ctx *ctx = hash_ctx;
	int rc;

	if (size < ctx->digest_size) {
		debug("HACE error: insufficient size on destination buffer\n");
		free(ctx);
		return -EINVAL;
	}

	rc = digest_object(ctx->list, ctx->len, dest_buf, ctx->method);
	if (rc)
		debug("HACE Scatter-Gather failure\n");

	free(ctx);

	return rc;
}
#endif

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
