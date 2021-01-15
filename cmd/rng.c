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
#include <stdlib.h>
#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <inttypes.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#define AST_RNG_TYPE	0x1e6e2520
#define AST_RNG		0x1e6e2524

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(int numerator, int denominator, char *format, ...)
{
	int val = numerator * 100 / denominator;
	int lpad = numerator * PBWIDTH / denominator;
	int rpad = PBWIDTH - lpad;
	char buffer[256];
	va_list aptr;

	va_start(aptr, format);
	vsprintf(buffer, format, aptr);
	va_end(aptr);

	printf("\r%3d%% [%.*s%*s] %s", val, lpad, PBSTR, rpad, "", buffer);
	if (numerator == denominator)
		printf("\n");
}

void wait_new_rng(void)
{

	int i;
	// unsigned long long start, end;
	// long long time;

	// start = get_ticks();

	// take 3456 kicks time, about 2.88us
	i = 200;
	while (i--) {
		asm("");
	}

	// end = get_ticks();
	// time = end - start;
	// printf("%llu %llu %lld\n", start, end, time);
}

static int do_ast_rng(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int i, j;
	int type;
	int len;
	phys_addr_t addr;
	char *buf;

	if (argc != 4)
		return CMD_RET_USAGE;
	type = simple_strtoul(argv[1], NULL, 16);
	addr = simple_strtoul(argv[2], NULL, 16);
	len = simple_strtoul(argv[3], NULL, 16);
	buf = map_physmem(addr, len, MAP_WRBACK);

	if (type > 7)
		return CMD_RET_USAGE;
	writel((type & 0x7) << 1, AST_RNG_TYPE);

	wait_new_rng();

	for (i = 0; i < len; i++) {
		if (i % 1000 == 0)
			printProgress(i, len, "");
		buf[i] = 0;
		for (j = 0; j < 8; j++) {
			buf[i] |= (readl(AST_RNG) & 0x1) << j;
			wait_new_rng();
		}
	}

	printProgress(len, len, "");

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	rng, 4, 0,  do_ast_rng,
	"ASPEED true random number generator",
	"rng <type> <addr> <len>\n"
	"  type: 0~7\n"
);
