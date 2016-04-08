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
#define HASH_GLOBALS
#include "type.h"
#include "vdef.h"
#include "vhace.h"
#include "vfun.h"

void HashAst3000(ULONG ulLength, ULONG *output, ULONG ulHashMode)
{
	ULONG i, ulTemp, ulCommand, ulDigestLength;
	ULONG ulValue;
   
	/* Get Info */
	switch (ulHashMode)
	{
	case VHASHMODE_MD5:
		ulCommand = VHASH_ALG_SELECT_MD5;
		ulDigestLength = 16;
		break;
	case VHASHMODE_SHA1:
		ulCommand = VHASH_ALG_SELECT_SHA1;
		ulDigestLength = 20;
		break;
	case VHASHMODE_SHA256:
		ulCommand = VHASH_ALG_SELECT_SHA256;
		ulDigestLength = 32;
		break;
	case VHASHMODE_SHA224:
		ulCommand = VHASH_ALG_SELECT_SHA224;
		ulDigestLength = 28;
		break;       	
	}
   	
	/* Init. HW */
	WriteMemoryLongHost(VHAC_REG_BASE, VREG_HASH_SRC_BASE_OFFSET, g_HashSrcBuffer);
	WriteMemoryLongHost(VHAC_REG_BASE, VREG_HASH_DST_BASE_OFFSET, g_HashDstBuffer);
	WriteMemoryLongHost(VHAC_REG_BASE, VREG_HASH_LEN_OFFSET, ulLength);
        
    /* write src */
	//already fill in g_VIDEO1_COMPRESS_BUF_ADDR

    /* fire cmd */
	WriteMemoryLongHost(VHAC_REG_BASE, VREG_HASH_CMD_OFFSET, ulCommand);
    
    /* get digest */
    do {
    	ulTemp = ReadMemoryLongHost(VHAC_REG_BASE, VREG_HASH_STATUS_OFFSET);
    } while (ulTemp & VHASH_BUSY);	
    	
    for (i=0; i<ulDigestLength/4; i++)
    {
                ulValue = *(((ULONG *)g_HashDstBuffer) + i); 

		//output is ULONG pointer
		*(output + i) = ulValue;    
    }        
}
