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
#include <command.h>

extern int pll_function(int argc, char *argv[]);
extern int trap_function(int argc, char *argv[]);
extern int dram_stress_function(int argc, char *argv[]);

int do_plltest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    return pll_function( argc, argv);
}

U_BOOT_CMD(
    plltest,   CONFIG_SYS_MAXARGS, 0,  do_plltest,
    "plltest - PLLTest [pll mode] [err rate] \n",
    NULL
);

int do_traptest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    return trap_function( argc, argv);
}

U_BOOT_CMD(
    traptest,   CONFIG_SYS_MAXARGS, 0,  do_traptest,
    "traptest- Check hardware trap for CPU clock and CPU\\AHB ratio.\n",
    NULL
);

int do_dramtest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    return dram_stress_function( argc, argv);
}

U_BOOT_CMD(
    dramtest,   CONFIG_SYS_MAXARGS, 0,  do_dramtest,
    "dramtest- Stress DRAM.\n",
    NULL
);
