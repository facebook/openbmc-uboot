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
#ifndef _VFUN_H_
#define _VFUN_H_

//#define vBufAlign(x)		    ((x + 0x0000007F) & 0xFFFFFF80) //128 byte alignment
#define vBufAlign(x)		    ((x + 0x000003FF) & 0xFFFFFC00) //128 byte alignment
#define vBufAlign2(x)		    ((x + 0x0000FFFF) & 0xFFFF0000) //128 byte alignment
#define v16byteAlign(x)		    ((x + 0x0000000F) & 0xFFFFFFF0)
#define vBuf_ALIGNMENT			128

#define HOST_TOTAL_SIZE         0x8000000   /* 128M */
#define STATION_TOTAL_SIZE      0xF800000   /* 120M */

#define VIDEO_SOURCE_SIZE       0x200000    /* 800X600X4 = 0x1D4C00 */
#define VIDEO_MAX_STREAM_SIZE   0x400000    /* 32X128K = 0x400000 */
#define VIDEO_FLAG_SIZE         0x5000      /* 1920X1200/128 = 0x4650*/ 
#define VIDEO_CRC_SIZE          0x50000     /* 1920/64X1200X8 = 0x46500*/

#define VIDEO1_EN_TOTAL_SIZE    (VIDEO_SOURCE_SIZE*2+VIDEO_MAX_STREAM_SIZE+VIDEO_FLAG_SIZE+VIDEO_CRC_SIZE) /* 0x1655000 =  about 23M*/
#define VIDEO2_EN_TOTAL_SIZE    VIDEO1_EN_TOTAL_SIZE
//#define VIDEOM_EN_TOTAL_SIZE    (VIDEO_SOURCE_SIZE*2+VIDEO_MAX_STREAM_SIZE+VIDEO_FLAG_SIZE) /* 0x1605000 = about 22.7M */
//#define VIDEO_HOST_SIZE         (VIDEO1_EN_TOTAL_SIZE + VIDEO2_EN_TOTAL_SIZE + VIDEOM_EN_TOTAL_SIZE) /* 0x69922816 = about 70M */
#define VIDEO_HOST_SIZE         (VIDEO1_EN_TOTAL_SIZE + VIDEO2_EN_TOTAL_SIZE) /* NOT NEED VIDEOM */

#define VIDEO1_EN_BASE          0x100000
#define VIDEO2_EN_BASE          (VIDEO1_EN_BASE + VIDEO1_EN_TOTAL_SIZE)
#define VIDEOM_EN_BASE          (VIDEO2_EN_BASE + VIDEO2_EN_TOTAL_SIZE)

#define VIDEO1_DE_TOTAL_SIZE    (VIDEO_MAX_STREAM_SIZE + VIDEO_SOURCE_SIZE) /* 0xD00000 = 13M*/
#define VIDEO2_DE_TOTAL_SIZE    (VIDEO1_DE_TOTAL_SIZE)
#define VIDEO_STATION_SIZE      (VIDEO1_DE_TOTAL_SIZE + VIDEO2_DE_TOTAL_SIZE) /* 26M */

#define VIDEO1_DE_BASE          VIDEO_HOST_SIZE
#define VIDEO2_DE_BASE          (VIDEO1_DE_BASE + VIDEO1_DE_TOTAL_SIZE)
#define VIDEO_ALL_SIZE          (VIDEO_HOST_SIZE + VIDEO_STATION_SIZE) //Host and Station

#define OutdwmBankModeHost(offset,data)         WriteMemoryLongHost(DRAM_BASE,offset,data)
#define IndwmBankModeHost(offset)               ReadMemoryLongHost(DRAM_BASE,offset)      

ULONG UnlockVideoRegHost(ULONG   MMIOBase, ULONG Key);
BOOL CheckOnStartHost(void);
BOOL CheckOnStartClient(void);
void StartVideoCaptureTriggerHost(ULONG   MMIOBase, ULONG offset);
void StartVideoCaptureTriggerHost(ULONG   MMIOBase, ULONG offset);
void StartVideoCodecTriggerHost(ULONG   MMIOBase, ULONG offset);
ULONG UnlockSCURegHost(ULONG   MMIOBase, ULONG Key);
ULONG UnlockSCURegHost(ULONG   MMIOBase, ULONG Key);
void StartModeDetectionTriggerHost(ULONG   MMIOBase, ULONG offset);
void ClearVideoInterruptHost(ULONG   MMIOBase, ULONG value);
BOOL ReadVideoInterruptHost(ULONG   MMIOBase, ULONG value);
void StopModeDetectionTriggerHost(ULONG   MMIOBase, ULONG offset);
void ResetVideoHost(void);
ULONG  InitializeVideoEngineHost (ULONG                MMIOBase,
                               int                  nVideo,
							   BOOL					HorPolarity,
				  BOOL					VerPolarity);
ULONG  InitializeVideoEngineClient (ULONG                MMIOBase,
				    int                  nVideo);
BYTE  GetI2CRegClient(ULONG  MMIOBase, 
				 BYTE DeviceSelect, 
				 BYTE DeviceAddress, 
				 BYTE RegisterIndex);

ULONG  SetI2CRegClient(ULONG  MMIOBase, 
                 BYTE   DeviceSelect, 
                 BYTE   DeviceAddress, 
                 BYTE   RegisterIndex, 
				 BYTE   RegisterValue);
#endif //_VFUN_H_

