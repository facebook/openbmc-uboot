// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <asm/arch/fmc_dual_boot_ast2600.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * RMII daughtercard workaround
 */
//#define ASPEED_RMII_DAUGHTER_CARD

#ifdef ASPEED_RMII_DAUGHTER_CARD
/**
 * @brief	workaround for RMII daughtercard, reset PHY manually
 *
 * workaround for Aspeed RMII daughtercard, reset Eth PHY by GPO F0 and F2
 * Where GPO F0 controls the reset signal of RMII PHY 1 and 2.
 * Where GPO F2 controls the reset signal of RMII PHY 3 and 4.
*/
void reset_eth_phy(void)
{
#define GRP_F		8
#define PHY_RESET_MASK  (BIT(GRP_F + 0) | BIT(GRP_F + 2))

	u32 value = readl(0x1e780020);
	u32 direction = readl(0x1e780024);

	debug("RMII workaround: reset PHY manually\n");

	direction |= PHY_RESET_MASK;
	value &= ~PHY_RESET_MASK;
	writel(direction, 0x1e780024);
	writel(value, 0x1e780020);
	while((readl(0x1e780020) & PHY_RESET_MASK) != 0);

	udelay(1000);

	value |= PHY_RESET_MASK;
	writel(value, 0x1e780020);
	while((readl(0x1e780020) & PHY_RESET_MASK) != PHY_RESET_MASK);
}
#endif

__weak int board_init(void)
{
	struct udevice *dev;
	int i;
	int ret;
	u64 rev_id;
	u32 tmp_val;

	/* disable address remapping for A1 to prevent secure boot reboot failure */
	rev_id = readl(ASPEED_REVISION_ID0);
	rev_id = ((u64)readl(ASPEED_REVISION_ID1) << 32) | rev_id;

	if (rev_id == 0x0501030305010303 || rev_id == 0x0501020305010203) {
		if ((readl(ASPEED_SB_STS) & BIT(6))) {
			tmp_val = readl(0x1e60008c) & (~BIT(0));
			writel(0xaeed1a03, 0x1e600000);
			writel(tmp_val, 0x1e60008c);
			writel(0x1, 0x1e600000);
		}
	}

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#if CONFIG_IS_ENABLED(FMC_DUAL_BOOT)
	fmc_enable_dual_boot();
#endif

#ifdef ASPEED_RMII_DAUGHTER_CARD
	reset_eth_phy();
#endif
	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	i = 0;
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

	return 0;
}

__weak int dram_init(void)
{
	struct udevice *dev;
	struct ram_info ram;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM FAIL1\r\n");
		return ret;
	}

	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("DRAM FAIL2\r\n");
		return ret;
	}

	gd->ram_size = ram.size;
	return 0;
}

int arch_early_init_r(void)
{
#ifdef CONFIG_DM_PCI
	/* Trigger PCIe devices detection */
	pci_init();
#endif

	return 0;
}

void board_add_ram_info(int use_default)
{
#define MMC_BASE 0x1e6e0000
#define SCU_BASE 0x1e6e2000
	uint32_t act_size = 256 << (readl(MMC_BASE + 0x04) & 0x3);
	uint32_t vga_rsvd = 8 << ((readl(MMC_BASE + 0x04) >> 2) & 0x3);
	uint8_t ecc = (readl(MMC_BASE + 0x04) >> 7) & 0x1;

	/* no VGA reservation if efuse VGA disable bit is set */
	if (readl(SCU_BASE + 0x594) & BIT(14))
		vga_rsvd = 0;

	printf(" (capacity:%d MiB, VGA:%d MiB, ECC:%s", act_size,
	       vga_rsvd, ecc == 1 ? "on" : "off");

	if (ecc)
		printf(", ECC size:%d MiB", (readl(MMC_BASE + 0x54) >> 20) + 1);

	printf(")");
}

union ast2600_pll_reg {
	unsigned int w;
	struct {
		unsigned int m : 13;		/* bit[12:0]	*/
		unsigned int n : 6;		/* bit[18:13]	*/
		unsigned int p : 4;		/* bit[22:19]	*/
		unsigned int off : 1;		/* bit[23]	*/
		unsigned int bypass : 1;	/* bit[24]	*/
		unsigned int reset : 1;		/* bit[25]	*/
		unsigned int reserved : 6;	/* bit[31:26]	*/
	} b;
};

void aspeed_mmc_init(void)
{
	u32 reset_bit;
	u32 clkstop_bit;
	u32 clkin = 25000000;
	u32 pll_reg = 0;
	u32 enableclk_bit;
	u32 rate = 0;
	u32 div = 0;
	u32 i = 0;
	u32 mult;
	u32 clk_sel = readl(0x1e6e2300);

	/* check whether boot from eMMC is enabled */
	if ((readl(0x1e6e2500) & 0x4) == 0)
		return;

	/*
	 * disable fmc wdt since it will be triggered
	 * when flash memory is touched
	 */
	writel(readl(0x1e620064) & 0xfffffffe, 0x1e620064);

	/* disable eMMC boot controller engine */
	*(volatile int *)0x1e6f500C &= ~0x90000000;
	/* set pinctrl for eMMC */
	*(volatile int *)0x1e6e2400 |= 0xff000000;

	/* clock setting for eMMC */
	enableclk_bit = BIT(15);

	reset_bit = BIT(16);
	clkstop_bit = BIT(27);
	writel(reset_bit, 0x1e6e2040);
	udelay(100);
	writel(clkstop_bit, 0x1e6e2084);
	mdelay(10);
	writel(reset_bit, 0x1e6e2044);

	pll_reg = readl(0x1e6e2220);
	if (pll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1) */
		union ast2600_pll_reg reg;
		reg.w = pll_reg;
		mult = (reg.b.m + 1) / (reg.b.n + 1);
		div = (reg.b.p + 1);
	}
	rate = ((clkin * mult)/div);

	for(i = 0; i < 8; i++) {
		div = (i + 1) * 2;
		if ((rate / div) <= 200000000)
			break;
	}

	clk_sel &= ~(0x7 << 12);
	clk_sel |= (i << 12) | BIT(11);
	writel(clk_sel, 0x1e6e2300);

	setbits_le32(0x1e6e2300, enableclk_bit);

	return;

}
