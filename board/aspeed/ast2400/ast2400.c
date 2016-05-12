/*
 * (C) Copyright 2004-Present
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#define AHC_CONTROLLER_BASE	0x1E600000
#define DRAM_SETTINGS_BASE	0x1E60008C
#define SCU_BASE		0x1e6e2000
#define HW_VERSION_BASE		0x1e6e207c
#define VIDEO_BASE		0x1e6e2040

#define AHC_MAGIC		0xAEED1A03
#define SCU_MAGIC		0x1688A8A8

#ifdef CONFIG_FLASH_AST2300
#define FLASH_CONTROLLER_BASE	0x1e620000
#define FLASH_WRITE_ENABLE	0x800f0000
#else
#define FLASH_CONTROLLER_BASE	0x16000000
#define FLASH_WRITE_ENABLE	0x00001c00
#endif

#ifdef CONFIG_ASPEEDNIC
int aspeednic_initialize(bd_t *bis);
#endif

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	u32 flags;

	/* AHB Controller */
	/* Unlock AHB controller */
	writel(AHC_MAGIC, AHC_CONTROLLER_BASE);
	flags = readl(DRAM_SETTINGS_BASE);
	flags |= 0x01;
	/* Map DRAM to 0x00000000 */
	writel(flags, DRAM_SETTINGS_BASE);

	/* Flash Controller */
	flags = readl(FLASH_CONTROLLER_BASE);
	flags |= FLASH_WRITE_ENABLE;
	/* Enable flash write */
	writel(flags, FLASH_CONTROLLER_BASE);

	/* SCU */
	/* unlock SCU */
	writel(SCU_MAGIC, SCU_BASE);

	/* PCLK  = HPLL/8 */
	flags = readl(SCU_BASE + 0x08);
	flags &= 0x1c0fffff;
	flags |= 0x61800000;
	writel(flags, SCU_BASE + 0x08);

	/* enable 2D Clk */
	flags = readl(SCU_BASE + 0x0c);
	flags &= 0xFFFFFFFD;
	writel(flags, SCU_BASE + 0x0c);

	/*
	 * Enable wide screen.
	 * If your video driver does not support wide screen, don't enable this
	 * bit 0x1e6e2040 D[0]
	 */
	flags = readl(VIDEO_BASE);
	flags |= 0x01;
	writel(flags, VIDEO_BASE);

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x40000100;

	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);

	return 0;
}

void watchdog_init(void)
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
	u32 reload;
#ifdef CONFIG_ASPEED_ENABLE_DUAL_BOOT_WATCHDOG
	/* dual boot watchdog is enabled */
	/* set the reload value */
	reload = CONFIG_AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT;
	/* set the reload value */
	writel(reload, CONFIG_AST_WDT2_BASE + 0x04);
	/* magic word to reload */
	writel(0x4755, CONFIG_AST_WDT2_BASE + 0x08);
	printf("Dual boot watchdog: %us\n",
		CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT);
#endif
	reload = CONFIG_AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
	/* set the reload value */
	writel(reload, CONFIG_AST_WDT_BASE + 0x04);
	/* magic word to reload */
	writel(0x4755, CONFIG_AST_WDT_BASE + 0x08);
	/* start the watchdog with 1M clk src and reset whole chip */
	writel(0x33, CONFIG_AST_WDT_BASE + 0x0c);
	printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}

int misc_init_r(void)
{
	u32 reg, revision, chip_id;

	/* Show H/W Version */
	reg = readl(HW_VERSION_BASE);

	chip_id = (reg & 0xff000000) >> 24;
	revision = (reg & 0xff0000) >> 16;

	puts ("H/W:   ");
	if (chip_id == 1) {
		if (revision >= 0x80)
			printf("AST2300 series FPGA Rev. %02x \n", revision);
		else
			printf("AST2300 series chip Rev. %02x \n", revision);
	} else if (chip_id == 2)
		printf("AST2400 series chip Rev. %02x \n", revision);
	else if (chip_id == 0)
		printf("AST2050/AST2150 series chip\n");

	if (getenv("verify") == NULL)
		setenv("verify", "n");
	if (getenv("eeprom") == NULL)
		setenv("eeprom", "y");

	watchdog_init();
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -1;
#if defined(CONFIG_ASPEEDNIC)
	ret = aspeednic_initialize(bis);
#else
	printf("No ETH, ");
#endif
	return ret;
}
