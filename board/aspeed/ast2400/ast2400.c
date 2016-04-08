/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <pci.h>

int board_init (void)
{
    DECLARE_GLOBAL_DATA_PTR;
	unsigned char data;
	unsigned long gpio;
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
#ifdef CONFIG_AST1070
	//check lpc or lpc+ mode
////////////////////////////////////////////////////////////////////////
	gpio = *((volatile ulong*) 0x1e780070);		/* mode check */
	if(gpio & 0x2)
		reg |= 0x100000;				/* LHCLK = HPLL/4 */
	else
		reg |= 0x300000;				/* LHCLK = HPLL/8 */

	reg |= 0x80000; 				/* enable LPC Host Clock */

    *((volatile ulong*) 0x1e6e2008) = reg;

	reg = *((volatile ulong*) 0x1e6e200c);		/* enable LPC clock */
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

int dram_init (void)
{
    DECLARE_GLOBAL_DATA_PTR;

    /* dram_init must store complete ramsize in gd->ram_size */
    gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);

    return 0;
}

/*
SCU7C: Silicon Revision ID Register
D[31:24]: Chip ID
0: AST2050/AST2100/AST2150/AST2200/AST3000
1: AST2300

D[23:16] Silicon revision ID for AST2300 generation and later
0: A0
1: A1
2: A2
.
.
.
FPGA revision starts from 0x80


D[11:8] Bounding option

D[7:0] Silicon revision ID for AST2050/AST2100 generation (for software compatible)
0: A0
1: A1
2: A2
3: A3
.
.
FPGA revision starts from 0x08, 8~10 means A0, 11+ means A1, AST2300 should be assigned to 3
*/
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

static void watchdog_init()
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
#define AST_WDT1_BASE 0x1e785000
#define AST_WDT2_BASE 0x1e785020
#define AST_WDT_CLK (1*1000*1000) /* 1M clock source */
  u32 reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
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
  __raw_writel(reload, AST_WDT1_BASE + 0x04);
  /* magic word to reload */
  __raw_writel(0x4755, AST_WDT1_BASE + 0x08);
  /* start the watchdog with 1M clk src and reset whole chip */
  __raw_writel(0x33, AST_WDT1_BASE + 0x0c);
  printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}

int misc_init_r(void)
{
    unsigned int reg, reg1, revision, chip_id, lpc_plus;

#ifdef CONFIG_AST1070
	//Reset AST1070 and AST2400 engine [bit 23:15]
	reg = *((volatile ulong*) 0x1e7890a0);
	reg &= ~0x808000;
	*((volatile ulong*) 0x1e7890a0) = reg;

	udelay(5000);

	lpc_plus = (*((volatile ulong*) 0x1e780070)) & 0x2;

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

	} else {
		// enable AST1050's LPC master
		reg = *((volatile ulong*) 0x1e7890a0);
		*((volatile ulong*) 0x1e7890a0) |= 0x11;

	}

#endif
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

#ifdef CONFIG_AST1070
	if(lpc_plus) {
		puts ("C/C:   LPC+ :");
		revision = (unsigned int) (*((ulong*) 0x70002034));
		printf("AST1070 ID [%08x] ", revision);

		if((*((volatile ulong*) 0x1e780070)) & 0x4) {
			if((unsigned int) (*((ulong*) 0x70012034)) == 0x10700001)
				printf(", 2nd : AST1070 ID [%08x] \n", (unsigned int) (*((ulong*) 0x70012034)));
			else
				printf("\n");
		} else {
			printf("\n");
		}
	} else {
		puts ("C/C:   LPC  :");
		revision = (unsigned int) (*((ulong*) 0x60002034));
		printf("LPC : AST1070 ID [%08x] \n", revision);

	}
#endif

#ifdef	CONFIG_PCI
    pci_init ();
#endif

    if (getenv ("verify") == NULL) {
	setenv ("verify", "n");
    }
    if (getenv ("eeprom") == NULL) {
	setenv ("eeprom", "y");
    }

    watchdog_init();
}

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void aspeed_init_pci (struct pci_controller *hose);

void pci_init_board(void)
{
    aspeed_init_pci(&hose);
}
#endif

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
