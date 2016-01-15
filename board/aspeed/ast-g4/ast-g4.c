/*
 * (C) Copyright 2002
 * Ryan Chen 
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/ast_scu.h>
#include <asm/arch/watchdog.h>
#include <asm/gpio.h>

#ifdef CONFIG_GENERIC_MMC
#include <sdhci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

int board_init (void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gd->flags = 0;
	return 0;
}

int wait_calibration_done()
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned char data;
	unsigned long reg, count = 0;

	do {
		udelay(1000);
		count++;
		if (count >= 1000) {
		
			return 1;
		}
	} while ((*(volatile ulong*) 0x1e6ec000) & 0xf00);

//	printf ("count = %d\n", count);

	return 0;
}

/* AST1070 Calibration
Program 0x101 to 0x1e6ec000
Wait till 1e6ec000 [8] = 0
Check 0x1e6ec004 = 0x5a5a5a5a
*/
int ast1070_calibration()
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned char data;
	unsigned long reg, i, j;

	//only for 2 chip
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 4; j++) {
//			printf ("chip = %d, delay = %d\n", i, j);
			*((volatile ulong*) 0x1e6ec000) = (j << (12 + i * 2)) + (1 << (8 + i)) + 0x01;
//			printf ("1e6ec000 = %x\n", *(volatile ulong*)0x1e6ec000);
			if (!wait_calibration_done()) {
				if ((*(volatile ulong*) 0x1e6ec004) == 0x5a5a5a5a) {
//					printf ("calibration result: chip %d pass, timing = %d\n", i, j);
					break;
				}
				else {
//					printf ("calibration result: chip %d fail, timing = %d\n", i, j);
				}
			}
		}
	}
   
	return 0;
}

// Should init some scratch register .. depend on different chip model  
int misc_init_r (void)
{
	unsigned long reset, reg, lpc_plus;

	//init PLL , SCU ,	Multi-pin share
	/* AHB Controller */
	*((volatile ulong*) 0x1E600000)  = 0xAEED1A03;	/* unlock AHB controller */ 
	*((volatile ulong*) 0x1E60008C) |= 0x01;		/* map DRAM to 0x00000000 */
#ifdef CONFIG_PCI
	*((volatile ulong*) 0x1E60008C) |= 0x30;		/* map PCI */
#endif

	/* SCU */
	*((volatile ulong*) 0x1e6e2000) = 0x1688A8A8;	/* unlock SCU */
	reg = *((volatile ulong*) 0x1e6e2008);		/* LHCLK = HPLL/8 */
	reg &= 0x1c0fffff;									/* PCLK  = HPLL/8 */
	reg |= 0x61800000;					/* BHCLK = HPLL/8 */

#ifdef CONFIG_ARCH_AST1070
	//check lpc or lpc+ mode
	//Reset AST1070 and AST2400 engine [bit 23:15]
	reset = *((volatile ulong*) 0x1e7890a0);
	reset &= ~0x808000;
	*((volatile ulong*) 0x1e7890a0) = reset;

	udelay(5000);	
	
	//in reset state
	lpc_plus = gpio_get_value(PIN_GPIOI1);

	if(lpc_plus)
		reg |= 0x100000;				/* LHCLK = HPLL/4 */
	else
		reg |= 0x300000;				/* LHCLK = HPLL/8 */

	reg |= 0x80000; 				/* enable LPC Host Clock */

	*((volatile ulong*) 0x1e6e2008) = reg;	   

	reg = *((volatile ulong*) 0x1e6e200c);		/* enable LPC clock */
	*((volatile ulong*) 0x1e6e200c) &= ~(1 << 28);

	if(lpc_plus) {
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
	
	reg = *((volatile ulong*) 0x1e6e200c);		/* enable 2D Clk */
	*((volatile ulong*) 0x1e6e200c) &= 0xFFFFFFFD;
/* enable wide screen. If your video driver does not support wide screen, don't
enable this bit 0x1e6e2040 D[0]*/
	reg = *((volatile ulong*) 0x1e6e2040);
	*((volatile ulong*) 0x1e6e2040) |= 0x01;	

#ifdef CONFIG_ARCH_AST1070

	if(lpc_plus) {
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


	reg = *((volatile ulong*) 0x1e7890a0);
	reg |= 0x800000;	
	*((volatile ulong*) 0x1e7890a0) = reg;
	
	udelay(1000);
	
	reg = *((volatile ulong*) 0x1e7890a0);
	reg |= 0x008000;	
	*((volatile ulong*) 0x1e7890a0) = reg;		


	if(lpc_plus) {
		*((volatile ulong*) 0x1E60008C) |= 0x011;		/* map DRAM to 0x00000000 and LPC+ 0x70000000*/
		
		//SCU multi-Function pin
		reg = *((volatile ulong*) 0x1e6e2090); 
		reg |= (1 << 30);
		*((volatile ulong*) 0x1e6e2090) = reg;		
		//LPC+ Engine Enable
		reg = *((volatile ulong*) 0x1e6ec000);
		reg |= 1;								
		*((volatile ulong*) 0x1e6ec000) = reg;		
	
		ast1070_calibration();

		ast1070_scu_init(AST_LPC_PLUS_BRIDGE);
		printf("C/C:   LPC+ :");
		
	} else {
		// enable AST1050's LPC master
		reg = *((volatile ulong*) 0x1e7890a0);
		*((volatile ulong*) 0x1e7890a0) |= 0x11;
		ast1070_scu_init(AST_LPC_BRIDGE);		
		printf("C/C:   LPC :");
	
	}

	printf("AST1070 ID [%x]",ast1070_scu_revision_id(0));

	if(gpio_get_value(PIN_GPIOI2)) {
		printf(", 2nd : AST1070 ID [%x]",ast1070_scu_revision_id(1));
	}	
	printf("\n");
		
#endif

#ifdef CONFIG_CPU1

	//uart 3/4 shar pin
	*((volatile ulong*) 0x1e6e2080) = 0xffff0000;	

	//mapping table
	*((volatile ulong*) 0x1e6e2104) = CONFIG_CPU1_MAP_FLASH;		
	//Sram
	*((volatile ulong*) 0x1e6e210c) = CONFIG_CPU1_MAP_SRAM;	

	*((volatile ulong*) 0x1e6e2114) = CONFIG_CPU1_MAP_DRAM;	

	//Enable coldfire V1 clock
	*((volatile ulong*) 0x1e6e2100) = 0x01;	

	printf("Coldfire V1 : UART3 \n");
#endif

	watchdog_init();

	return 0;

}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = (dram - vga);

	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bd)
{
	return aspeednic_initialize(bd);
}
#endif

#ifdef CONFIG_GENERIC_MMC

#define CONFIG_SYS_MMC_NUM		2
#define CONFIG_SYS_MMC_BASE		{AST_SDHC_BASE + 0x100, AST_SDHC_BASE + 0x200}

int board_mmc_init(bd_t *bis)
{
	ulong mmc_base_address[CONFIG_SYS_MMC_NUM] = CONFIG_SYS_MMC_BASE;
	u8 i, data;

	ast_scu_init_sdhci();
	//multipin. Remind: AST2300FPGA only supports one port at a time

	for (i = 0; i < CONFIG_SYS_MMC_NUM; i++) {
			ast_scu_multi_func_sdhc_slot1(i);
		if (ast_sdhi_init(mmc_base_address[i], ast_get_sd_clock_src(), 100000,
				SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE |
		SDHCI_QUIRK_BROKEN_R1B | SDHCI_QUIRK_32BIT_DMA_ADDR |
		SDHCI_QUIRK_WAIT_SEND_CMD))
			return 1;
	}

	return 0;
}
#endif
