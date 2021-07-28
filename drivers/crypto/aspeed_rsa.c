// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 ASPEED Technology Inc.
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <asm/types.h>
#include <asm/io.h>
#include <malloc.h>
#include <u-boot/rsa-mod-exp.h>

/* ARCY data memory (internal SRAM) */
#define ARCY_DMEM_BASE		0x1e710000
#define ARCY_DMEM_SIZE		0x10000

/* ARCY registers */
#define ARCY_BASE			0x1e6fa000
#define ARCY_CTRL1			(ARCY_BASE + 0x00)
#define   ARCY_CTRL1_RSA_DMA		BIT(1)
#define   ARCY_CTRL1_RSA_START		BIT(0)
#define ARCY_CTRL2			(ARCY_BASE + 0x44)
#define ARCY_CTRL3			(ARCY_BASE + 0x48)
#define   ARCY_CTRL3_SRAM_AHB_ACCESS	BIT(8)
#define   ARCY_CTRL3_ECC_RSA_MODE_MASK	GENMASK(5, 4)
#define   ARCY_CTRL3_ECC_RSA_MODE_SHIFT	4
#define ARCY_DMA_DRAM_SADDR		(ARCY_BASE + 0x4c)
#define ARCY_DMA_DMEM_TADDR		(ARCY_BASE + 0x50)
#define   ARCY_DMA_DMEM_TADDR_LEN_MASK	GENMASK(15, 0)
#define   ARCY_DMA_DMEM_TADDR_LEN_SHIFT	0
#define ARCY_RSA_PARAM			(ARCY_BASE + 0x58)
#define   ARCY_RSA_PARAM_EXP_MASK	GENMASK(31, 16)
#define   ARCY_RSA_PARAM_EXP_SHIFT	16
#define   ARCY_RSA_PARAM_MOD_MASK	GENMASK(15, 0)
#define   ARCY_RSA_PARAM_MOD_SHIFT	0
#define ARCY_RSA_INT_EN			(ARCY_BASE + 0x3f8)
#define   ARCY_RSA_INT_EN_RSA_READY	BIT(2)
#define   ARCY_RSA_INT_EN_RSA_CMPLT	BIT(1)
#define ARCY_RSA_INT_STS		(ARCY_BASE + 0x3fc)
#define   ARCY_RSA_INT_STS_RSA_READY	BIT(2)
#define   ARCY_RSA_INT_STS_RSA_CMPLT	BIT(1)

/* misc. constant */
#define ARCY_ECC_MODE	2
#define ARCY_RSA_MODE	3
#define ARCY_CTX_BUFSZ	0x600

static int aspeed_mod_exp(struct udevice *dev, const uint8_t *sig, uint32_t sig_len,
			  struct key_prop *prop, uint8_t *out)
{
	int i, j;
	u8 *ctx;
	u8 *ptr;
	u32 reg;

	ctx = memalign(16, ARCY_CTX_BUFSZ);
	if (!ctx)
		return -ENOMEM;

	memset(ctx, 0, ARCY_CTX_BUFSZ);

	ptr = (u8 *)prop->public_exponent;
	for (i = prop->exp_len - 1, j = 0; i >= 0; --i) {
		ctx[j] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	ptr = (u8 *)prop->modulus;
	for (i = (prop->num_bits >> 3) - 1, j = 0; i >= 0; --i) {
		ctx[j + 16] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	ptr = (u8 *)sig;
	for (i = sig_len - 1, j = 0; i >= 0; --i) {
		ctx[j + 32] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	writel((u32)ctx, ARCY_DMA_DRAM_SADDR);

	reg = (((prop->exp_len << 3) << ARCY_RSA_PARAM_EXP_SHIFT) & ARCY_RSA_PARAM_EXP_MASK) |
		  ((prop->num_bits << ARCY_RSA_PARAM_MOD_SHIFT) & ARCY_RSA_PARAM_MOD_MASK);
	writel(reg, ARCY_RSA_PARAM);

	reg = (ARCY_CTX_BUFSZ << ARCY_DMA_DMEM_TADDR_LEN_SHIFT) & ARCY_DMA_DMEM_TADDR_LEN_MASK;
	writel(reg, ARCY_DMA_DMEM_TADDR);

	reg = (ARCY_RSA_MODE << ARCY_CTRL3_ECC_RSA_MODE_SHIFT) & ARCY_CTRL3_ECC_RSA_MODE_MASK;
	writel(reg, ARCY_CTRL3);

	writel(ARCY_CTRL1_RSA_DMA | ARCY_CTRL1_RSA_START, ARCY_CTRL1);

	/* polling RSA status */
	while (1) {
		reg = readl(ARCY_RSA_INT_STS);
		if ((reg & ARCY_RSA_INT_STS_RSA_READY) && (reg & ARCY_RSA_INT_STS_RSA_CMPLT))
			break;
		udelay(20);
	}

	writel(0x0, ARCY_CTRL1);
	writel(ARCY_CTRL3_SRAM_AHB_ACCESS, ARCY_CTRL3);
	udelay(20);

	ptr = (u8 *)ARCY_DMEM_BASE;
	for (i = (prop->num_bits / 8) - 1, j = 0; i >= 0; --i) {
		out[i] = ptr[j + 32];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	return 0;
}

static const struct mod_exp_ops aspeed_arcy_ops = {
	.mod_exp	= aspeed_mod_exp,
};

U_BOOT_DRIVER(aspeed_rsa_mod_exp) = {
	.name = "aspeed_rsa_mod_exp",
	.id = UCLASS_MOD_EXP,
	.ops = &aspeed_arcy_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DEVICE(aspeed_rsa) = {
	.name = "aspeed_rsa_mod_exp",
};
