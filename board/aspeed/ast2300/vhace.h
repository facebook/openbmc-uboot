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
#ifndef _VHACE_H_
#define _VHACE_H_

#define VHASHMODE_MD5			0x00
#define VHASHMODE_SHA1			0x01
#define VHASHMODE_SHA256			0x02
#define VHASHMODE_SHA224			0x03

#define VHASH_ALG_SELECT_MASK		0x70
#define VHASH_ALG_SELECT_MD5			0x00
#define VHASH_ALG_SELECT_SHA1		0x20
#define VHASH_ALG_SELECT_SHA224		0x40
#define VHASH_ALG_SELECT_SHA256		0x50

#define VHASH_BUSY			0x01

#define VHAC_REG_BASE 			0x1e6e0000
#define	VHAC_REG_OFFSET			0x3000

#define VREG_HASH_SRC_BASE_OFFSET	(0x20+VHAC_REG_OFFSET)
#define VREG_HASH_DST_BASE_OFFSET	(0x24+VHAC_REG_OFFSET)
#define VREG_HASH_KEY_BASE_OFFSET	(0x28+VHAC_REG_OFFSET)
#define VREG_HASH_LEN_OFFSET			(0x2C+VHAC_REG_OFFSET)
#define VREG_HASH_CMD_OFFSET			(0x30+VHAC_REG_OFFSET)

#define VREG_HASH_STATUS_OFFSET		(0x1C+VHAC_REG_OFFSET)

typedef struct
{
	int HashMode;
	int DigestLen;
} HASH_METHOD;


#ifdef HASH_GLOBALS
#define HASH_EXT
#else
#define HASH_EXT extern
#endif

HASH_EXT HASH_METHOD	g_HashMethod;
HASH_EXT BYTE			g_DigestBuf[32];
HASH_EXT ULONG			g_HashSrcBuffer;
HASH_EXT ULONG			g_HashDstBuffer;

void HashAst3000(ULONG ulLength, ULONG *output, ULONG ulHashMode);
#endif

