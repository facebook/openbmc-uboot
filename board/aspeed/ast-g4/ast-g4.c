/*
 * (C) Copyright 2002 Ryan Chen
 * Copyright 2016 IBM Corporation
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

#include <asm/arch/platform.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/arch/ast_scu.h>
#include <asm/arch/regs-ahbc.h>
#include <asm/arch/regs-scu.h>
#include <asm/io.h>

#define HW_VERSION_BASE               0x1e6e207c

DECLARE_GLOBAL_DATA_PTR;

// Add for watchdog init
void watchdog_init(void)
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
  	#define AST_WDT1_BASE 0x1e785000
  	#define AST_WDT2_BASE 0x1e785020

  	u32 reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
#ifdef CONFIG_ASPEED_ENABLE_DUAL_BOOT_WATCHDOG
  	/* dual boot watchdog is enabled */
  	/* set the reload value */
  	reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT;
  	/* set the reload value */
  	__raw_writel(reload, AST_WDT2_BASE + 0x04);
  	/* magic word to reload */
  	__raw_writel(0x4755, AST_WDT2_BASE + 0x08);
        /* init the WDT2 clock source to 1MHz.
           set WDT2 reset type to SOC reset.
           enable 2nd flash boot whenever WDT2 reset.*/
        __raw_writel(0x93, AST_WDT2_BASE + 0x0c);
  	printf("Dual boot watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_DUAL_BOOT_TIMEOUT);
#else
  	/* disable WDT2 */
  	*((volatile ulong*) 0x1e78502C) &= 0xFFFFFFFE;
#endif
  	reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
  	/* set the reload value */
  	__raw_writel(reload, AST_WDT1_BASE + 0x04);
  	/* magic word to reload */
  	__raw_writel(0x4755, AST_WDT1_BASE + 0x08);
  	/* start the watchdog with 1M clk src and reset whole chip */
  	__raw_writel(0x33, AST_WDT1_BASE + 0x0c);
  	printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}

int board_init(void)
{
    unsigned long reg;

    /* AHB Controller */
    *((volatile ulong*) 0x1E600000)  = 0xAEED1A03;      /* unlock AHB controller */
    *((volatile ulong*) 0x1E60008C) |= 0x01;            /* map DRAM to 0x00000000 */

    /* Flash Controller */
#ifdef  CONFIG_FLASH_AST2300
    *((volatile ulong*) 0x1e620000) |= 0x800f0000;      /* enable Flash Write */
#else
    *((volatile ulong*) 0x16000000) |= 0x00001c00;      /* enable Flash Write */
#endif

    /* SCU */
    *((volatile ulong*) 0x1e6e2000) = 0x1688A8A8;       /* unlock SCU */
        reg = *((volatile ulong*) 0x1e6e2008);
        reg &= 0x1c0fffff;
        reg |= 0x61800000;                              /* PCLK  = HPLL/8 */
#ifdef CONFIG_AST1070
        unsigned long gpio;

        //check lpc or lpc+ mode
        gpio = *((volatile ulong*) 0x1e780070);         /* mode check */
        if(gpio & 0x2)
                reg |= 0x100000;                                /* LHCLK = HPLL/4 */
        else
                reg |= 0x300000;                                /* LHCLK = HPLL/8 */

        reg |= 0x80000;                                 /* enable LPC Host Clock */

    *((volatile ulong*) 0x1e6e2008) = reg;

        reg = *((volatile ulong*) 0x1e6e200c);          /* enable LPC clock */
        *((volatile ulong*) 0x1e6e200c) &= ~(1 << 28);

        if(gpio & 0x2) {

                //use LPC+ for sys clk
                // set OSCCLK = VPLL1
                *((volatile ulong*) 0x1e6e2010) = 0x18;

                // enable OSCCLK
                reg = *((volatile ulong*) 0x1e6e202c);
                reg |= 0x00000002;
                *((volatile ulong*) 0x1e6e202c) = reg;
        } else {
                // USE LPC use D2 clk
                /*set VPPL1 */
            *((volatile ulong*) 0x1e6e201c) = 0x6420;

                // set d2-pll & enable d2-pll D[21:20], D[4]
            reg = *((volatile ulong*) 0x1e6e202c);
            reg &= 0xffcfffef;
            reg |= 0x00200010;
            *((volatile ulong*) 0x1e6e202c) = reg;

                // set OSCCLK = VPLL1
            *((volatile ulong*) 0x1e6e2010) = 0x8;

                // enable OSCCLK
            reg = *((volatile ulong*) 0x1e6e202c);
            reg &= 0xfffffffd;
            reg |= 0x00000002;
            *((volatile ulong*) 0x1e6e202c) = reg;
        }
#else
        *((volatile ulong*) 0x1e6e2008) = reg;
#endif
    reg = *((volatile ulong*) 0x1e6e200c);              /* enable 2D Clk */
    *((volatile ulong*) 0x1e6e200c) &= 0xFFFFFFFD;
    /* enable wide screen. If your video driver does not support wide screen, don't
       enable this bit 0x1e6e2040 D[0]*/
    reg = *((volatile ulong*) 0x1e6e2040);
    *((volatile ulong*) 0x1e6e2040) |= 0x01;

    /* arch number */
    gd->bd->bi_arch_number = MACH_TYPE_ASPEED;

    /* set the clock source of WDT2 for 1MHz */
    *((volatile ulong*) 0x1e78502C) |= 0x10;

    /* adress of boot parameters */
    gd->bd->bi_boot_params = 0x40000100;

    watchdog_init();

    return 0;

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

        return 0;
}

int dram_init(void)
{
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = dram - vga;
#ifdef CONFIG_DRAM_ECC
        gd->ram_size -= gd->ram_size >> 3; /* need 1/8 for ECC */
#endif
	return 0;
}

#ifdef CONFIG_FTGMAC100
int board_eth_init(bd_t *bd)
{
	return ftgmac100_initialize(bd);
}
#endif

#ifdef CONFIG_ASPEEDNIC
int board_eth_init(bd_t *bd)
{
	return aspeednic_initialize(bd);
}
#endif
