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

DECLARE_GLOBAL_DATA_PTR;

#define DRAM_BASE	0x80000000
#define TIMEOUT_DRAM	5000000

#ifdef CONFIG_ASPEED_AST2600
#define BLK_SIZE_MB			64
#define N_BLK				32
#define MCR74_BLK_OFFSET		26
#define MCR74_LEN_OFFSET		4
#define MCR74_BLK_LEN_MASK		GENMASK(30, 4)
#else
#define BLK_SIZE_MB			16
#define N_BLK				64
#define MCR74_BLK_OFFSET		24
#define MCR74_LEN_OFFSET		4
#define MCR74_BLK_LEN_MASK		GENMASK(29, 4)
#endif
#define BLK_SIZE			(BLK_SIZE_MB * 1024 * 1024)
#define N_16B_IN_BLK			(BLK_SIZE / 16)

static int MMCTestBurst(u32 datagen)
{
	u32 data;
	int timeout = 0;

	writel(0x00000000, 0x1e6e0070);
	writel((0x000000c1 | (datagen << 3)), 0x1e6e0070);

	do {
		data = readl(0x1e6e0070) & 0x3000;

		if (data & 0x2000)
			return 0;

		if (++timeout > TIMEOUT_DRAM) {
			printf("Timeout!!\n");
			writel(0x00000000, 0x1e6e0070);
			return 0;
		}
	} while (!data);

	writel(0x00000000, 0x1e6e0070);

	return 1;
}

static int MMCTestSingle(u32 datagen)
{
	u32 data;
	int timeout = 0;

	writel(0x00000000, 0x1e6e0070);
	writel((0x00000085 | (datagen << 3)), 0x1e6e0070);

	do {
		data = readl(0x1e6e0070) & 0x3000;

		if (data & 0x2000)
			return 0;

		if (++timeout > TIMEOUT_DRAM) {
			printf("Timeout!!\n");
			writel(0x00000000, 0x1e6e0070);

			return 0;
		}
	} while (!data);

	writel(0x00000000, 0x1e6e0070);

	return 1;
}

static int MMCTest(void)
{
	unsigned int pattern;

	pattern = rand();
	writel(pattern, 0x1e6e007c);
	printf("Pattern = %08X : ", pattern);

	if(!MMCTestBurst(0))    return(0);
	if(!MMCTestBurst(1))    return(0);
	if(!MMCTestBurst(2))    return(0);
	if(!MMCTestBurst(3))    return(0);
	if(!MMCTestBurst(4))    return(0);
	if(!MMCTestBurst(5))    return(0);
	if(!MMCTestBurst(6))    return(0);
	if(!MMCTestBurst(7))    return(0);
	if(!MMCTestSingle(0))   return(0);
	if(!MMCTestSingle(1))   return(0);
	if(!MMCTestSingle(2))   return(0);
	if(!MMCTestSingle(3))   return(0);
	if(!MMCTestSingle(4))   return(0);
	if(!MMCTestSingle(5))   return(0);
	if(!MMCTestSingle(6))   return(0);
	if(!MMCTestSingle(7))   return(0);

	return 1;
}

static void print_usage(void)
{
	printf("\nASPEED DRAM BIST\n\n");
	printf("Usage: dramtest <count> <block> <length>\n\n");
	printf("count:  how many iterations to run (mandatory, in decimal)\n");
	printf("        0: infinite loop.\n");
	printf("block:  index of the address block to test "
			"(optional, in decimal, default: 0)\n");
	printf("        0: [%08x, %08x)\n", DRAM_BASE + BLK_SIZE * 0, DRAM_BASE + BLK_SIZE * 1);
	printf("        1: [%08x, %08x)\n", DRAM_BASE + BLK_SIZE * 1, DRAM_BASE + BLK_SIZE * 2);
	printf("        2: [%08x, %08x)\n", DRAM_BASE + BLK_SIZE * 2, DRAM_BASE + BLK_SIZE * 3);
	printf("        n: [80000000 + n*%dMB, 80000000 + (n+1)*%dMB)\n", BLK_SIZE_MB, BLK_SIZE_MB);
	printf("           where n = [0, %d]\n", N_BLK - 1);
	printf("length: size to test (optional, in hex, default: 0x10000)\n");
	printf("        0x0: test the whole memory block %dMB\n", BLK_SIZE_MB);
	printf("        0x1: test the first 16 Bytes of the memory block\n");
	printf("        0x2: test the first 2*16 Bytes of the memory block\n");
	printf("        n  : test the first n*16 Bytes of the memory block\n");
	printf("             where n = [0x00000001, 0x%08x]\n", N_16B_IN_BLK);
	printf("\n\n");
}

static int do_ast_dramtest(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	u32 PassCnt = 0;
	unsigned long Testcounter = 0;
	unsigned long block = 0;
	unsigned long length = 0x10000;
	int ret;

	if (argc < 2) {
		ret = 0;
		goto cmd_err;
	}

	ret = CMD_RET_USAGE;
	switch (argc) {
	case 4:
		if (strict_strtoul(argv[3], 16, &length) < 0)
			goto cmd_err;
	case 3:
		if (strict_strtoul(argv[2], 10, &block) < 0)
			goto cmd_err;
	case 2:
		if (strict_strtoul(argv[1], 10, &Testcounter) < 0)
			goto cmd_err;
		break;
	}

	if (length == 0)
		length = N_16B_IN_BLK;

	printf("Test range: 0x%08lx - 0x%08lx\n", DRAM_BASE + (block << MCR74_BLK_OFFSET),
	       DRAM_BASE | (block << MCR74_BLK_OFFSET) | ((length - 1) << MCR74_LEN_OFFSET) | 0xf);

	ret = 1;
	writel(0xfc600309, 0x1e6e0000);
	while ((Testcounter > PassCnt) || (Testcounter == 0)) {
		clrsetbits_le32(0x1e6e0074, MCR74_BLK_LEN_MASK,
				(block << MCR74_BLK_OFFSET) | ((length - 1) << MCR74_LEN_OFFSET));

		if (!MMCTest()) {
			printf("FAIL %d/%ld (fail DQ 0x%08x)\n", PassCnt, Testcounter,
			       readl(0x1e6e0078));
			ret = 0;
			break;
		} else {
			PassCnt++;
			printf("Pass %d/%ld\n", PassCnt, Testcounter);
		}

		if (ctrlc())
			break;
	}

	return ret;

cmd_err:	
	print_usage();
	return ret;
}

U_BOOT_CMD(
	dramtest,   5, 0,  do_ast_dramtest,
	"ASPEED DRAM BIST",
	""
);
