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

	return 0;
}

int misc_init_r (void)
{
	unsigned long reset, reg, lpc_plus;

	//init PLL , SCU ,	Multi-pin share
	/* AHB Controller */
	*((volatile ulong*) 0x1E600000)  = 0xAEED1A03;	/* unlock AHB controller */ 
	*((volatile ulong*) 0x1E60008C) |= 0x01;		/* map DRAM to 0x00000000 */

	/* SCU */
	*((volatile ulong*) 0x1e6e2000) = 0x1688A8A8;	/* unlock SCU */
	reg = *((volatile ulong*) 0x1e6e2008);		/* LHCLK = HPLL/8 */
	reg &= 0x1c0fffff;									/* PCLK  = HPLL/8 */
	reg |= 0x61800000;					/* BHCLK = HPLL/8 */

	*((volatile ulong*) 0x1e6e2008) = reg;

	reg = *((volatile ulong*) 0x1e6e200c);		/* enable 2D Clk */
	*((volatile ulong*) 0x1e6e200c) &= 0xFFFFFFFD;
	/* enable wide screen. If your video driver does not support wide screen, don't
	   enable this bit 0x1e6e2040 D[0]*/
	reg = *((volatile ulong*) 0x1e6e2040);
	*((volatile ulong*) 0x1e6e2040) |= 0x01;	

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
	return ftgmac100_initialize(bd);
}
#endif
