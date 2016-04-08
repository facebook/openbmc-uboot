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
#ifndef _TYPE_H_
#define _TYPE_H_

	typedef unsigned char               BOOL;
	typedef unsigned char               UINT8;
	typedef unsigned short              UINT16;
	typedef unsigned int                UINT32;

	#define  FLONG                    unsigned long
	#define  BYTE                     unsigned char
	#define  INT                      int
	#define  VOID                     void
	#define  BOOLEAN                  unsigned short
	#define  ULONG                    unsigned long
	#define  USHORT                   unsigned short
	#define  UCHAR                    unsigned char
	#define  CHAR                     char
	#define  LONG                     long
	#define  PUCHAR                   UCHAR *
	#define  PULONG                   ULONG *

	#define  FAIL                     1

	#define  intfunc      int386

	#define  outdwport         outpd
	#define  indwport          inpd
	#define  outport           outp
	#define  inport            inp

	//#define     NULL    ((void *)0)
	#define     FALSE   0
	#define     TRUE    1

	#define ReadMemoryBYTE(baseaddress,offset)        *(BYTE *)((ULONG)(baseaddress)+(ULONG)(offset))
	#define ReadMemoryLong(baseaddress,offset)        *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))
	#define ReadMemoryShort(baseaddress,offset)       *(USHORT *)((ULONG)(baseaddress)+(ULONG)(offset))
	#define WriteMemoryBYTE(baseaddress,offset,data)  *(BYTE *)((ULONG)(baseaddress)+(ULONG)(offset)) = (BYTE)(data)
	#define WriteMemoryLong(baseaddress,offset,data)  *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))=(ULONG)(data)
	#define WriteMemoryShort(baseaddress,offset,data) *(USHORT *)((ULONG)(baseaddress)+(ULONG)(offset))=(USHORT)(data)
	#define WriteMemoryLongWithANDData(baseaddress, offset, anddata, data)  *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) = *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) & (ULONG)(anddata) | (ULONG)(data)

	#define WriteMemoryLongWithMASK(baseaddress, offset, data, mask) \
	*(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) = *(volatile ULONG *)(((ULONG)(baseaddress)+(ULONG)(offset)) & (ULONG)(~(mask))) | ((ULONG)(data) & (ULONG)(mask))

	#define ReadMemoryLongHost(baseaddress,offset)        *(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))
	#define WriteMemoryLongHost(baseaddress,offset,data)  *(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))=(ULONG)(data)
	#define WriteMemoryBYTEHost(baseaddress,offset,data)  *(volatile BYTE *)((ULONG)(baseaddress)+(ULONG)(offset)) = (BYTE)(data)    
#define WriteMemoryLongWithMASKHost(baseaddress, offset, data, mask)  *(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) = (((*(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)))&(~mask)) | (ULONG)((data)&(mask)))
    
	#define ReadMemoryLongClient(baseaddress,offset)        *(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))
	#define WriteMemoryLongClient(baseaddress,offset,data)  *(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))=(ULONG)(data)
	#define WriteMemoryBYTEClient(baseaddress,offset,data)  *(volatile BYTE *)((ULONG)(baseaddress)+(ULONG)(offset)) = (BYTE)(data)    
#define WriteMemoryLongWithMASKClient(baseaddress, offset, data, mask)  *(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) = (((*(volatile ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)))&(~mask)) | (ULONG)((data)&(mask)))

#ifdef BUF_GLOBALS
#define BUF_EXT
#else
#define BUF_EXT extern
#endif

BUF_EXT ULONG    g_CAPTURE_VIDEO1_BUF1_ADDR; /* VIDEO1_BUF_1_ADDR*/ 
BUF_EXT ULONG    g_CAPTURE_VIDEO1_BUF2_ADDR; /* VIDEO1_BUF_2_ADDR*/
BUF_EXT ULONG    g_VIDEO1_COMPRESS_BUF_ADDR; /* Encode destination address */
BUF_EXT ULONG    g_VIDEO1_CRC_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO1_FLAG_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO1_RC4_BUF_ADDR;


BUF_EXT ULONG    g_CAPTURE_VIDEO2_BUF1_ADDR; 
BUF_EXT ULONG    g_CAPTURE_VIDEO2_BUF2_ADDR; 
BUF_EXT ULONG    g_VIDEO2_COMPRESS_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO2_CRC_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO2_FLAG_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO2_RC4_BUF_ADDR;

BUF_EXT ULONG    g_VIDEO1_DECODE_BUF_1_ADDR;
BUF_EXT ULONG    g_VIDEO1_DECODE_BUF_2_ADDR;
BUF_EXT ULONG    g_VIDEO1_DECOMPRESS_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO1_DECODE_RC4_BUF_ADDR;

BUF_EXT ULONG    g_VIDEO2_DECODE_BUF_1_ADDR;
BUF_EXT ULONG    g_VIDEO2_DECODE_BUF_2_ADDR;
BUF_EXT ULONG    g_VIDEO2_DECOMPRESS_BUF_ADDR;
BUF_EXT ULONG    g_VIDEO2_DECODE_RC4_BUF_ADDR;

BUF_EXT ULONG    g_CAPTURE_VIDEOM_BUF1_ADDR; 
BUF_EXT ULONG    g_CAPTURE_VIDEOM_BUF2_ADDR; 
BUF_EXT ULONG    g_VIDEOM_COMPRESS_BUF_ADDR;
BUF_EXT ULONG    g_VIDEOM_FLAG_BUF_ADDR;
BUF_EXT ULONG    g_VIDEOM_RC4_BUF_ADDR;

BUF_EXT ULONG    g_VIDEOM_DECODE_BUF_1_ADDR;
BUF_EXT ULONG    g_VIDEOM_DECODE_BUF_2_ADDR;
BUF_EXT ULONG    g_VIDEOM_DECOMPRESS_BUF_ADDR;
BUF_EXT ULONG    g_VIDEOM_DECODE_RC4_BUF_ADDR;

#ifdef WIN_GLOBALS
#define WIN_EXT
#else
#define WIN_EXT extern
#endif

WIN_EXT USHORT	g_DefWidth, g_DefHeight;
   
#endif
