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

#define AST_WDT_BASE 0x1e785000
void reset_cpu(ulong addr)
{
	__raw_writel(0x10 , AST_WDT_BASE+0x04);
	__raw_writel(0x4755, AST_WDT_BASE+0x08);
	__raw_writel(0x23, AST_WDT_BASE+0x0c); /* reset the full chip */

	while (1)
	/*nothing*/;
}
