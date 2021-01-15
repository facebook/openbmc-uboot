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
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>

#include <inttypes.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compiler.h>

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <asm/arch/scu_ast2600.h>
#include <dt-bindings/clock/ast2600-clock.h>

#include <asm/arch/scu_ast2600.h>
#include <dt-bindings/clock/ast2600-clock.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
unsigned long get_ast2600_pll_rate(struct udevice *scu_dev, unsigned int pll_idx)
{
	struct clk clk;
	unsigned long ret = 0;

	clk.id = pll_idx;
	ret = clk_request(scu_dev, &clk);
	if (ret < 0) {
		return ret;
	}
	
	ret = clk_get_rate(&clk);
	
	clk_free(&clk);

	return ret;
}

static int cal_ast2600_28nm_pll_rate(unsigned long pll_rate, unsigned int pll_idx)
{
	u32 ulCounter, ulLowLimit, ulHighLimit;
    u32 ulData;	
	u32 ulErrRate = 2;
	int i = 0;

//	printf("pll rate : %ld \n", pll_rate);

	ulCounter = (pll_rate/1000) * 512 / 25000 - 1;
    ulLowLimit = ulCounter * (100 - ulErrRate) / 100;
    ulHighLimit = ulCounter * (100 + ulErrRate) / 100;
	writel((ulLowLimit << 16) | ulHighLimit, 0x1e6e2324);
//	printf("ulCounter %lx , ulLowLimit %lx , ulHighLimit  %lx \n", ulCounter, ulLowLimit, ulHighLimit);

	//1. Set SCU320 = 0x24 use hclk reset freq measurement counter
	writel(0x24, 0x1e6e2320);
	//2. Wait until SCU320[29:16] = 0
    do {
        ulData = readl(0x1e6e2320);
        mdelay(1);
    } while ((ulData & 0x3fff0000) && (i++<1000));	 //wait until SCU320[29:16]=0

    if (i>= 1000)
    {
        printf("Fre Count Eng No idle %x\n", ulData);
        return 1;
    }
	//3. Set SCU320[0] = 1 and SCU320[5:2] = clock for measurement
	writel((pll_idx << 2) | 0x1, 0x1e6e2320);
	//4. Delay 1ms
	mdelay(1);
	//5. Set SCU320[1] = 1
	writel((pll_idx << 2) | 0x3, 0x1e6e2320);
	
	//6. Wait until SCU320[6] = 1
    i = 0;
    do {
        ulData = readl(0x1e6e2320);
        mdelay(1);
    } while ((!(ulData & 0x40)) && (i++<1000));

    if (i>= 1000)
    {
        printf("PLL Detec No idle %x\n", ulData);
        return 1;
    }

	//7. Read SCU320[29:16] and calculate the result frequency using following equation
//	printf("ulCounter val : %lx \n", (readl(0x1e6e2320) & GENMASK(29, 16)) >> 16);
	//When the reference clock CLK25M count from 0 to 512, measure the OSCCLK counting value, then
	//OSCCLK frequency = CLK25M / 512 * (SCU320[29:16] + 1)
//	printf("rate %ld \n", (25000000 / 512) * (((readl(0x1e6e2320) & GENMASK(29, 16)) >> 16) + 1));

	ulData = readl(0x1e6e2320);

	writel(0x2C, 0x1e6e2320); //disable ring	

	if(ulData & 0x80) 
		return 0;
	else
		return 1;
}

static int cal_ast2600_13nm_pll_rate(unsigned long pll_rate, unsigned int pll_idx)
{
    u32 ulData;	
	u32 ulCounter, ulLowLimit, ulHighLimit;
	u32 ulErrRate = 2;
	int i = 0;

//	printf("pll rate : %ld \n", pll_rate);

	ulCounter = (pll_rate/1000) * 512 / 25000 - 1;
    ulLowLimit = ulCounter * (100 - ulErrRate) / 100;
    ulHighLimit = ulCounter * (100 + ulErrRate) / 100;
	writel((ulHighLimit << 16) | ulLowLimit, 0x1e6e2334);
//	printf("ulCounter %lx , ulLowLimit %lx , ulHighLimit  %lx \n", ulCounter, ulLowLimit, ulHighLimit);
		
	//1. Set SCU320 = 0x30
	writel(0x30, 0x1e6e2330);
	//2. Wait until SCU320[29:16] = 0
    do {
        ulData = readl(0x1e6e2330);
        mdelay(1);
    } while ((ulData & 0x3fff0000) && (i++<1000));	 //wait until SCU320[29:16]=0

    if (i>= 1000)
    {
        printf("Fre Count Eng No idle %x\n", ulData);
        return 1;
    }
	//3. Set SCU320[0] = 1 and SCU320[5:2] = clock for measurement
	writel((pll_idx << 2) | 0x1, 0x1e6e2330);
	//4. Delay 1ms
	mdelay(1);
	//5. Set SCU320[1] = 1
	writel((pll_idx << 2) | 0x3, 0x1e6e2330);
	
	//6. Wait until SCU320[6] = 1
    i = 0;
    do {
        ulData = readl(0x1e6e2330);
        mdelay(1);
    } while ((!(ulData & 0x40)) && (i++<1000));

    if (i>= 1000)
    {
        printf("PLL Detec No idle %x\n", ulData);
        return 1;
    }	

	//7. Read SCU320[29:16] and calculate the result frequency using following equation
//	printf("ulCounter val : %lx \n", (readl(0x1e6e2330) & GENMASK(29, 16)) >> 16);
	//When the reference clock CLK25M count from 0 to 512, measure the OSCCLK counting value, then
	//OSCCLK frequency = CLK25M / 512 * (SCU320[29:16] + 1)
//	printf("rate %ld \n", (25000000 / 512) * (((readl(0x1e6e2330) & GENMASK(29, 16)) >> 16) + 1));

	ulData = readl(0x1e6e2330);

    writel(0x2C, 0x1e6e2330); //disable ring	

	if(ulData & 0x80) 
		return 0;
	else
		return 1;
}


int do_ast2600_pll_test(struct udevice *scu_dev, unsigned int pll_idx)
{
	int ret = 0;
	unsigned long pll_rate = 0;

	switch(pll_idx) {
		case 0:	//delay cell
			printf("Delay Cell : BYPASS \n");
			break;
		case 1: //nand gate
			printf("NAND Gate : BYPASS \n");
			break;
		case 2:
			printf("DLY 16 : BYPASS \n");
			break;
		case 3:
			printf("DLY 32 : BYPASS \n");
			break;
		case 4:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_DPLL);
			if(cal_ast2600_28nm_pll_rate(pll_rate/2, 4)) {
				printf("D-PLL : Fail \n");
				ret = 1;
			} else
				printf("D-PLL : PASS \n");
			break;
		case 5:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_EPLL);
			if(cal_ast2600_28nm_pll_rate(pll_rate/4, 5))
				printf("E-PLL : Fail \n");
			else
				printf("E-PLL : PASS \n");
			break;
		case 6:
			if(readl(0x1e6ed0c0) & BIT(5)) {
				switch((readl(0x1e6ed0d0) >> 16) & 0x3) {
					case 1: //2.5G 125Mhz
						pll_rate = 125000000;
						break;
					case 2:
						pll_rate = 250000000;
						break;
				}
				if(cal_ast2600_28nm_pll_rate(pll_rate, 6))
					printf("XPCLK-EP : Fail \n");
				else
					printf("XPCLK-EP : PASS \n");
			} else
				printf("XPCLK-EP : Fail 0 \n");
			break;
		case 8:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_MPLL);
			if(cal_ast2600_28nm_pll_rate(pll_rate/2, 8)) {
				printf("MCLK : Fail \n");
				ret = 1;
			} else
				printf("MCLK : PASS \n");
			break;
		case 9:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_AHB);
			if(cal_ast2600_28nm_pll_rate(pll_rate, 9)) {
				printf("HCLK-28nm : Fail \n");
				ret = 1;
			} else
				printf("HCLK-28nm : PASS \n");
			break;
		case 16:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_APLL);
			if(cal_ast2600_13nm_pll_rate(pll_rate/8, 0)) {
				printf("APLL : Fail \n");
				ret = 1;
			} else
				printf("APLL : PASS \n");
			break;
		case 17:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_AHB);
			if(cal_ast2600_13nm_pll_rate(pll_rate, 1)) {
				printf("HCLK-13nm : Fail \n");
				ret = 1;
			} else
				printf("HCLK-13nm : PASS \n");
			break;
		case 18:
			pll_rate = get_ast2600_pll_rate(scu_dev, ASPEED_CLK_APB2);
			if(cal_ast2600_13nm_pll_rate(pll_rate, 2))
				printf("PCLK : Fail \n");
			else
				printf("PCLK : PASS \n");
			break;
	}

	return ret;
}

int do_aspeed_full_pll_test(struct udevice *scu_dev)
{
	int i = 0;
	int ret = 0;
	for(i = 0; i < 32; i++) {
		ret += do_ast2600_pll_test(scu_dev, i);
	}

	printf("**************************************************** \n");
	if(ret)
		printf("PLL Test : Fail \n");
	else
		printf("PLL Test : Pass \n");
	printf("**************************************************** \n");
	
	return 1;
}

static int 
do_aspeed_plltest(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]) 
{
	struct udevice *scu_dev;
	char *pll_cmd;
	unsigned long pll_idx;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_GET_DRIVER(aspeed_scu), &scu_dev);
	if (ret)
		return ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	pll_cmd = argv[1];

	if (!strcmp(pll_cmd, "full"))
		ret = do_aspeed_full_pll_test(scu_dev);
	else {
		pll_idx = strtoul(argv[1], 0, 0);
		ret = do_ast2600_pll_test(scu_dev, pll_idx);
	}

	return ret;
}

U_BOOT_CMD(
	plltest,   3, 0,  do_aspeed_plltest,
	"ASPEED PLL test",
	"ping d2 - Ping to check if the device at the targeted address can respond\n"
	""
);
