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
/* Macro */
#define m08byteAlignment(x)		((x + 0x00000007) & 0xFFFFFFF8)
#define m16byteAlignment(x)		((x + 0x0000000F) & 0xFFFFFFF0)
#define m64byteAlignment(x)		((x + 0x0000003F) & 0xFFFFFFC0)

/* Options */
#define MIC_TEST_PAGE		32
#define DRAMSIZE		(MIC_TEST_PAGE * 0x1000)
#define MIC_MAX_CTRL		(MIC_TEST_PAGE / 4 + 16)
#define MIC_MAX_CHKSUM		(MIC_TEST_PAGE * 4 + 16)

/* Default Setting */
#define DEFAULT_RATE		0x00000000
#define DEFAULT_CTRL		0xFF
#define DEFAULT_CHKSUM		0x00000000
#define DEFAULT_WRITEBACK	0x08880000

/* Reg. Definition */
#define DRAM_BASE		0x40000000
#define MIC_BASE		0x1e640000
#define MIC_CTRLBUFF_REG	0x00
#define MIC_CHKSUMBUF_REG	0x04
#define MIC_RATECTRL_REG	0x08
#define MIC_ENGINECTRL_REG	0x0C
#define MIC_STOPPAGE_REG	0x10
#define MIC_STATUS_REG		0x14
#define MIC_STATUS1_REG		0x18
#define MIC_STATUS2_REG		0x1C

#define MIC_RESET_MIC		0x00000000
#define MIC_ENABLE_MIC		0x10000000
#define MIC_MAXPAGE_MASK	0x0FFFF000
#define MIC_WRITEBACK_MASK	0xFFFF0000
#define MIC_STOPPAGE_MASK	0x0000FFFF
#define MIC_PAGEERROR		0x40000000
#define MIC_PAGE1ERROR		0x10000000
#define MIC_PAGE2ERROR		0x20000000
#define MIC_INTMASK		0x00060000
#define MIC_ERRPAGENO_MASK	0x0000FFFF

#define MIC_CTRL_MASK		0x03
#define MIC_CTRL_SKIP		0x00
#define MIC_CTRL_CHK1		0x01
#define MIC_CTRL_CHK2		0x02
#define MIC_CTRL_CHK3		0x03
