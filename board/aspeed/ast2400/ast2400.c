/*
 * (C) Copyright 2004-Present
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#define MACH_TYPE_ASPEED 8888

#define AST_WDT_BASE 0x1e785000
#define AST_WDT2_BASE 0x1e785020
#define AST_WDT_CLK (1*1000*1000) /* 1M clock source */

#ifdef CONFIG_ASPEEDNIC
int aspeednic_initialize(bd_t *bis);
#endif

int board_init(void)
{
    DECLARE_GLOBAL_DATA_PTR;
	unsigned long reg;

    /* AHB Controller */
    *((volatile ulong*) 0x1E600000)  = 0xAEED1A03;	/* unlock AHB controller */
    *((volatile ulong*) 0x1E60008C) |= 0x01;		/* map DRAM to 0x00000000 */

    /* Flash Controller */
#ifdef	CONFIG_FLASH_AST2300
    *((volatile ulong*) 0x1e620000) |= 0x800f0000;	/* enable Flash Write */
#else
    *((volatile ulong*) 0x16000000) |= 0x00001c00;	/* enable Flash Write */
#endif

    /* SCU */
    *((volatile ulong*) 0x1e6e2000) = 0x1688A8A8;	/* unlock SCU */
	reg = *((volatile ulong*) 0x1e6e2008);
	reg &= 0x1c0fffff;
	reg |= 0x61800000;				/* PCLK  = HPLL/8 */

	*((volatile ulong*) 0x1e6e2008) = reg;
    reg = *((volatile ulong*) 0x1e6e200c);		/* enable 2D Clk */
    *((volatile ulong*) 0x1e6e200c) &= 0xFFFFFFFD;
/* enable wide screen. If your video driver does not support wide screen, don't
enable this bit 0x1e6e2040 D[0]*/
    reg = *((volatile ulong*) 0x1e6e2040);
    *((volatile ulong*) 0x1e6e2040) |= 0x01;

    /* arch number */
    gd->bd->bi_arch_number = MACH_TYPE_ASPEED;

    /* adress of boot parameters */
    gd->bd->bi_boot_params = 0x40000100;

    return 0;
}

int dram_init(void)
{
    DECLARE_GLOBAL_DATA_PTR;

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
  reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT;
  /* set the reload value */
  __raw_writel(reload, AST_WDT2_BASE + 0x04);
  /* magic word to reload */
  __raw_writel(0x4755, AST_WDT2_BASE + 0x08);
  printf("Dual boot watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT);
#endif
  reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
  /* set the reload value */
  __raw_writel(reload, AST_WDT_BASE + 0x04);
  /* magic word to reload */
  __raw_writel(0x4755, AST_WDT_BASE + 0x08);
  /* start the watchdog with 1M clk src and reset whole chip */
  __raw_writel(0x33, AST_WDT_BASE + 0x0c);
  printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}

int misc_init_r(void)
{
    unsigned int reg1, revision, chip_id;

    /* Show H/W Version */
    reg1 = (unsigned int) (*((ulong*) 0x1e6e207c));
    chip_id = (reg1 & 0xff000000) >> 24;
    revision = (reg1 & 0xff0000) >> 16;

    puts ("H/W:   ");
    if (chip_id == 1) {
	if (revision >= 0x80) {
		printf("AST2300 series FPGA Rev. %02x \n", revision);
	}
	else {
		printf("AST2300 series chip Rev. %02x \n", revision);
	}
    }
    else if (chip_id == 2) {
	printf("AST2400 series chip Rev. %02x \n", revision);
    }
    else if (chip_id == 0) {
		printf("AST2050/AST2150 series chip\n");
    }

    if (getenv ("verify") == NULL) {
	setenv ("verify", "n");
    }
    if (getenv ("eeprom") == NULL) {
	setenv ("eeprom", "y");
    }

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
