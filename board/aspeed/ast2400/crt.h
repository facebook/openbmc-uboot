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
#ifndef _CRT_H_
#define _CRT_H_

#ifdef Watcom
#define		CRT_REMAP_OFFSET	0x10000
#else
#define		CRT_REMAP_OFFSET	0x0
#endif

/********************************************************/
/*    CRT register                                      */
/********************************************************/
#define		CRT_BASE_OFFSET						0x6000+CRT_REMAP_OFFSET

#define		CRT1_CONTROL_REG					0x00 + CRT_BASE_OFFSET
	#define		GRAPH_DISPLAY_BIT				0
	#define     GRAPH_DISPLAY_MASK				(1<<0)
		#define		GRAPH_DISPLAY_ON			1
		#define		GRAPH_DISPLAY_OFF			0
    #define     FORMAT_SELECT_BIT               8
    #define     FORMAT_SELECT_MASK              (3<<8)   
	#define		HOR_SYNC_SELECT_BIT				16
    #define		HOR_SYNC_SELECT_MASK			(1<<16)	
		#define		HOR_NEGATIVE				1
		#define		HOR_POSITIVE				0
	#define		VER_SYNC_SELECT_BIT				17
    #define		VER_SYNC_SELECT_MASK			(1<<17)	
		#define		VER_NEGATIVE				1
		#define		VER_POSITIVE				0

#define		CRT1_CONTROL2_REG					0x04 + CRT_BASE_OFFSET
        
#define     CRT1_VIDEO_PLL_REG                  0x0C + CRT_BASE_OFFSET
    #define     POST_DIV_BIT                    18
    #define     POST_DIV_MASK                   3<<18
        #define     DIV_1_1                     0
        //#define     DIV_1_2                     1
        #define     DIV_1_2                     2
        #define     DIV_1_4                     3
        
#define     CRT1_HOR_TOTAL_END_REG              0x10 + CRT_BASE_OFFSET
    #define     HOR_TOTAL_BIT                   0
    #define     HOR_ENABLE_END_BIT              16

#define     CRT1_HOR_RETRACE_REG                0x14 + CRT_BASE_OFFSET
    #define     HOR_RETRACE_START_BIT           0
    #define     HOR_RETRACE_END_BIT             16

#define     CRT1_VER_TOTAL_END_REG              0x18 + CRT_BASE_OFFSET
    #define     VER_TOTAL_BIT                   0
    #define     VER_ENABLE_END_BIT              16

#define     CRT1_VER_RETRACE_REG                0x1C + CRT_BASE_OFFSET
    #define     VER_RETRACE_START_BIT           0
    #define     VER_RETRACE_END_BIT             16

#define     CRT1_DISPLAY_ADDRESS				0x20 + CRT_BASE_OFFSET
	#define		DISPLAY_ADDRESS_MASK			0x0FFFFFFF

#define     CRT1_DISPLAY_OFFSET                 0x24 + CRT_BASE_OFFSET
    #define     DISPLAY_OFFSET_ALIGN            7 /* 8 byte alignment*/
    #define     TERMINAL_COUNT_BIT              16    

#define     CRT1_THRESHOLD_REG                  0x28 + CRT_BASE_OFFSET
    #define     THRES_LOW_BIT                   0
    #define     THRES_HIGHT_BIT                 8

#define    CURSOR_POSITION                  0x30 + OFFSET
#define    CURSOR_OFFSET                    0x34 + OFFSET
#define    CURSOR_PATTERN                   0x38 + OFFSET
#define    OSD_HORIZONTAL                   0x40 + OFFSET
#define    OSD_VERTICAL                     0x44 + OFFSET
#define    OSD_PATTERN                      0x48 + OFFSET
#define    OSD_OFFSET                       0x4C + OFFSET
#define    OSD_THRESHOLD                    0x50 + OFFSET

//Ch7301c
#define		DEVICE_ADDRESS_CH7301_CRT1	0xEA
#define		DEVICE_ADDRESS_CH7301_CRT2	0xEC


#define		DEVICE_SELECT_CH7301		0x3

/* CH7301 Register Definition */
#define     CH7301_CD_REG				0x20
	#define     CD_DACT					0x0E
	#define		CD_DVIT					1 << 5
#define     CH7301_DC_REG				0x21
#define     CH7301_PM_REG				0x49

BOOL CheckHotPlug(int nCRTIndex);
BOOL CheckDAC(int nCRTIndex);

BOOL  ASTSetModeV (ULONG MMIOBase, 
                int nCRTIndex, 
                ULONG VGABaseAddr, 
                USHORT Horizontal, 
                USHORT Vertical, 
                BYTE ColorFormat, 
                BYTE CenterMode);
                
BOOL  SelCRTClock(ULONG MMIOBase,
				  int	nCRTIndex,
				  USHORT Horizontal, 
				  USHORT Vertical);

void DisableCRT(ULONG MMIOBase, int nCRTIndex);
void ClearCRTWithBlack(ULONG ulCRTAddr, int iWidth, int iHeight);

#endif /* _CRT_H_ */

