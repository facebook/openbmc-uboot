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
#ifndef _VDEF_H_
#define _VDEF_H_

#define     VIDEO1                              0
#define     VIDEO1_ON                           0x01
#define     VIDEO2                              1
#define     VIDEO2_ON                           0x02

#define		VIDEOM_ON							0x04
#define     VIDEOM                              2

#define		CRT_1								0
#define		CRT_1_ON							0x01
#define		CRT_2								1
#define		CRT_2_ON							0x02

#define		SINGLE_CODEC_SINGLE_CAPTURE			0
#define     AUTO_CODEC_SINGLE_CAPTURE			2
#define		AUTO_CODEC_AUTO_CAPTURE				3

#define		MAC1_BASE							0x1E660000
#define     APB_BRIDGE_1_BASE                   0x1E6E0000
#define     VIDEO_REG_BASE                      0x1E700000
#define     APB_BRIDGE_2_BASE                   0x1E780000

#define		DRAM_INIT_BASE						0x1E6E0000

#define		SDRAM_PROTECT_REG					0x00
    #define     SDRAM_WRITE_DISABLE             0
    #define     SDRAM_WRITE_ENABLE              1

#define		SCU_BASE							0x1E6E0000
#define		SCU_OFFSET							0x2000

#define		VIC_BASE							0x1E6C0000
	#define		VIDEO_INT_BIT				7

#define		IRQ_STATUS_REG						0x00
#define		RAW_INT_STATUS_REG					0x08
#define		INT_SEL_REG							0x0C
	#define		FIQ_INT							1
	#define		IRQ_INT							0	
#define		INT_EN_REG							0x10
#define		INT_EN_CLEAR_REG					0x14
#define		INT_SOFT_REG						0x18
#define		INT_SOFT_CLEAR_REG					0x1C
#define		INT_SENSE_REG						0x24
	#define		LEVEL_SENSE						1
	#define		EDGE_SENSE						0
#define		INT_EVENT_REG						0x2C
	#define		HIGH_LEVEL_SENSE				1
	#define		LOW_LEVEL_SENSE					0

#define		SCU_HW_TRAPPING_REG					0x70 + SCU_OFFSET
	#define		CLIENT_MODE_EN_BIT				18
	#define		CLIENT_MODE_EN_MASK				0x00040000
		#define		BE_HOST_CHIP				0
		#define		BE_CLIENT_CHIP				1

#define     SCU_ULOCK_KEY                       0x1688A8A8
    
#define     SCU_PROTECT_REG                     0x00 + SCU_OFFSET
    #define     SCU_WRITE_DISABLE               0
    #define     SCU_WRITE_ENABLE                1

#define     SCU_CONTROL_REG                     0x04 + SCU_OFFSET    
	#define     VIDEO_ENGINE_RESET				0x00000040
		#define     VIDEO_ENGINE_RESET_BIT      6
		#define     VIDEO_ENGINE_RESET_MASK     0x00000040
        #define     VIDEO_RESET_EN              1
        #define     VIDEO_RESET_OFF             0

#define     SCU_CLOCK_SELECTION_REG             0x08 + SCU_OFFSET
	#define		PORTA_CLOCK_DELAY_MASK			7 << 8	//Video port A output clcok selection
		#define	PORTA_CLOCK_INV_DELAY_1NS		5 << 8	//Clock inversed and delay ~ 2ns
		#define	PORTA_CLOCK_INV_DELAY_2NS		6 << 8	//Clock inversed and delay ~ 3ns
	#define		PORTB_CLOCK_DELAY_MASK			7 << 12	//Video port B output clock delay
		#define	PORTB_CLOCK_INV_DELAY_1NS		5 << 12	//Clock inversed and delay ~ 3ns
		#define	PORTB_CLOCK_INV_DELAY_2NS		6 << 12	//Clock inversed and delay ~ 3ns
	#define		PORTB_CLOCK_SEL					1 << 15 //Video port B clock selection
		#define	PORTB_FROM_D1CLK				0 << 15
		#define PORTB_FROM_D2CLK				1 << 15
	#define		ECLK_CLK_SEL_MASK				(3 << 2)
		#define	ECLK_FROM_HPLL					(1 << 2)

	#define     D2CLK_CLOCK_SELECTION			0x00020000
		#define     D2CLK_CLOCK_SELECTION_BIT   17
		#define     D2CLK_CLOCK_SELECTION_MASK  0x00060000
		#define		NORMAL_CRT1					0
        #define     V1CLK_VIDEO1				2
        #define     V1CLK_VIDEO2				3

#define     SCU_CLOCK_STOP_REG					0x0C + SCU_OFFSET    
	#define		EN_ECLK							0 << 0	//Enable ECLK (For Video Engine)
	#define		STOP_ECLK_BIT					0
	#define		STOP_ECLK_MASK					1 << 0
	#define		EN_V1CLK						0 << 3  //Enable V1CLK (For Video Capture #1)
	#define		STOP_V1CLK_BIT					3
	#define		STOP_V1CLK_MASK					1 << 3
	#define		EN_D1CLK						0 << 10 //Enable D1CLK (For CRT1)
	#define		STOP_D1CLK_BIT					10
	#define		STOP_D1CLK_MASK					1 << 10
	#define		EN_D2CLK						0 << 11 //Stop D2CLK (For CRT2)
	#define		STOP_D2CLK						(1 << 11)
	#define		STOP_D2CLK_BIT					11
	#define		STOP_D2CLK_MASK					1 << 11
	#define		EN_V2CLK						0 << 12 //Stop V2CLK (For Video Capture #2)
	#define		STOP_V2CLK_BIT					12
	#define		STOP_V2CLK_MASK					1 << 12
	#define		STOP_HACE_BIT					13
	#define		EN_HACE							(0 << 13)
	#define		STOP_HACE_MASK					(1 << 13)
	#define     EN_I2SCLK						0 << 18
	#define     STOP_I2SCLK_MASK				1 << 18

#define     SCU_PIN_CTRL1_REG					0x74 + SCU_OFFSET
	#define		I2C_5_PIN_EN					1 << 12	//Enable I2C #5 PIN
	#define		I2C_5_PIN_OFF					0 << 12	//Disable I2C #5 PIN
	#define		I2C_5_PIN_MASK					1 << 12
	#define		VGA_PIN_OFF						0 << 15 //Enable VGA pins
	#define		VGA_PIN_MASK					1 << 15
	#define		VIDEO_PORTA_EN					1 << 16 //Enable Video port A control pins
	#define		VIDEO_PORTA_MASK				1 << 16
	#define		VIDEO_PORTB_EN					1 << 17 //Enable Video port B control pins
	#define		VIDEO_PORTB_MASK				1 << 17
	#define		VIDEO_VP1_EN					1 << 22 //Enable VP[11:0]
	#define		VIDEO_VP1_MASK					1 << 22 
	#define		VIDEO_VP2_EN					1 << 23 //Enable VP[23:12]
	#define		VIDEO_VP2_MASK					1 << 23
	#define		I2S_PIN_EN						1 << 29 //Enable I2S function pins
	#define		I2S_PIN_MASK					1 << 29

#define     SCU_PIN_CTRL2_REG					0x78 + SCU_OFFSET
	#define		VIDEO_PORTA_SINGLE_EDGE_MASK	1 << 0
		#define		VIDEO_PORTA_SINGLE_EDGE		1 << 0	//Enable Video port A single mode
		#define		VIDEO_PORTA_DUAL_EDGE		0 << 0
	#define		VIDEO_PORTB_SINGLE_EDGE_MASK	1 << 1
		#define		VIDEO_PORTB_DUAL_EDGE		0 << 1
		#define		VIDEO_PORTB_SINGLE_EDGE		1 << 1	//Enable Video port B single mode

#define		SCU_M_PLL_PARAM_REG					0x20 + SCU_OFFSET

#define		DRAM_BASE							0x40000000

#define     INPUT_BITCOUNT_YUV444				4
#define		INPUT_BITCOUNT_YUV420				2

/* HW comment value */
//PASSWORD
#define     VIDEO_UNLOCK_KEY                    0x1A038AA8
#define		SCU_UNLOCK_KEY						0x1688A8A8
#define		SDRAM_UNLOCK_KEY					0xFC600309


//#define     SAMPLE_RATE                                 12000000.0
#ifdef OSC_NEW
    #define     SAMPLE_RATE                                 24576000.0
#else
    #define     SAMPLE_RATE                                 24000000.0
#endif

#define     MODEDETECTION_VERTICAL_STABLE_MAXIMUM       0x4
#define     MODEDETECTION_HORIZONTAL_STABLE_MAXIMUM     0x4
#define     MODEDETECTION_VERTICAL_STABLE_THRESHOLD     0x4
#define     MODEDETECTION_HORIZONTAL_STABLE_THRESHOLD   0x8

#define     MODEDETECTION_EDGE_PIXEL_THRES_DIGITAL      2
#define     MODEDETECTION_EDGE_PIXEL_THRES_ANALOGE      0x0A

#define		MODEDETECTION_OK							0
#define		MODEDETECTION_ERROR							1
#define		JUDGE_MODE_ERROR							2

//I2C Loop Count
#define    LOOP_COUNT                                  1000
#define    CAN_NOT_FIND_DEVICE                         1
#define    SET_I2C_DONE                                0
#define	   I2C_BASE									   0xA000
#define    AC_TIMING                                   0x77743355

//I2C channel and Devices
#define     I2C_VIDEO1_EEPROM                           0x2
#define     I2C_VIDEO2_EEPROM                           0x5
#define     I2C_VIDEO2_9883                             0x4
/*
ULONG    CAPTURE1_ADDRESS =                     0x1000000;
ULONG    CAPTURE2_ADDRESS =                     0x3000000;
ULONG    PASS1_ENCODE_SOURCE_ADDRESS =          0x1000000;
ULONG    PASS1_ENCODE_DESTINATION_ADDRESS =     0x2000000;
ULONG    Buffer1_DECODE_SOURCE_ADDRESS =        0x1000000;
ULONG    Buffer2_DECODE_SOURCE_ADDRESS =        0x1400000;
ULONG    PASS1_DECODE_DESTINATION_ADDRESS =     0x600000;
ULONG    CAPTURE_2ND_ADDRESS =                  0x1800000;
ULONG    PASS1_2ND_ENCODE_SOURCE_ADDRESS =      0x1800000;
ULONG    PASS1_2ND_ENCODE_DESTINATION_ADDRESS = 0x2800000;
ULONG    PASS1_2ND_DECODE_SOURCE_ADDRESS =      0x1000000;
ULONG    PASS1_2ND_DECODE_DESTINATION_ADDRESS = 0x600000;
ULONG    PASS2_ENCODE_SOURCE_ADDRESS =          0x000000;
ULONG    PASS2_ENCODE_DESTINATION_ADDRESS =     0xC00000;
ULONG    PASS2_DECODE_SOURCE_ADDRESS =          0xC00000;
ULONG    PASS2_DECODE_DESTINATION_ADDRESS =     0x600000;
ULNG    PASS2_DECODE_REFERENCE_ADDRESS =       0x600000;
*/

typedef struct _CTL_REG_G {
    ULONG   CompressMode:1;
    ULONG   SkipEmptyFrame:1;
    ULONG   MemBurstLen:2;
    ULONG   LineBufEn:2;
    ULONG   Unused:26;
} CTL_REG_G;


typedef union _U_CTL_G {
    ULONG       Value;
    CTL_REG_G   CtlReg;
} U_CTL_G;

typedef struct _MODE_DETECTION_PARAM_REG {
    ULONG   Unused1:8;
    ULONG   EdgePixelThres:8;
    ULONG   VerStableMax:4;
    ULONG   HorStableMax:4;
    ULONG   VerDiffMax:4;
    ULONG   HorDiffMax:4;
} MODE_DETECTION_PARAM_REG;

typedef struct _CRC_PRI_PARAM_REG {
    ULONG   Enable:1;
    ULONG   HighBitOnly:1;
    ULONG   SkipCountMax:6;
    ULONG   PolyLow:8;
    ULONG   PolyHigh:16;
} CRC_PRI_PARAM_REG;

typedef union _U_CRC_PRI_PARAM {
    ULONG               Value;
    CRC_PRI_PARAM_REG   CRCPriParam;
} U_CRC_PRI_PARAM;

typedef struct _CRC_SEC_PARAM_REG {
    ULONG   Unused1:8;
    ULONG   PolyLow:8;
    ULONG   PolyHigh:16;
} CRC_SEC_PARAM_REG;

typedef union _U_CRC_SEC_PARAM {
    ULONG               Value;
    CRC_SEC_PARAM_REG   CRCSecParam;
} U_CRC_SEC_PARAM;

typedef struct _GENERAL_INFO {
    BYTE                EnableVideoM;
    BYTE                CenterMode;
	BYTE				RC4NoResetFrame;
	BYTE				RC4TestMode;
    U_CTL_G             uCtlReg;
    U_CRC_PRI_PARAM     uCRCPriParam;
    U_CRC_SEC_PARAM     uCRCSecParam;
} GENERAL_INFO, *PGENERAL_INFO;

typedef struct _SEQ_CTL_REG {
    ULONG   Unused1:1;
    ULONG   Unused2:1;
    ULONG   Unused3:1;
    ULONG   CaptureAutoMode:1;
    ULONG   Unused4:1;
    ULONG   CodecAutoMode:1;
    ULONG   Unused5:1;
    ULONG   WatchDog:1;
    ULONG   CRTSel:1;
    ULONG   AntiTearing:1;
    ULONG   DataType:2;
    ULONG   Unused6:20;
} SEQ_CTL_REG;

typedef union _U_SEQ_CTL {
    ULONG               Value;
    SEQ_CTL_REG   SeqCtlReg;
} U_SEQ_CTL;

typedef struct _CTL_REG {
    ULONG   SrcHsync:1;
    ULONG   SrcVsync:1;
    ULONG   ExtSrc:1;
    ULONG   AnalongExtSrc:1;
    ULONG   IntTimingGen:1;
    ULONG   IntDataFrom:1;
    ULONG   WriteFmt:2;
    ULONG   VGACursor:1;
    ULONG   LinearMode:1;
    ULONG   ClockDelay:2;
    ULONG   CCIR656Src:1;
    ULONG   PortClock:1;
    ULONG   ExtPort:1;        
    ULONG   Unused1:1;
    ULONG   FrameRate:8;
    ULONG   Unused2:8;
} CTL_REG;

typedef union _U_CTL {
    ULONG       Value;
    CTL_REG     CtlReg;
} U_CTL_REG;

typedef struct _TIMING_GEN_SETTING_H {
    ULONG   HDEEnd:13;
    ULONG   Unused1:3;
    ULONG   HDEStart:13;
    ULONG   Unused2:3;
} TIMING_GEN_SETTING_H;

typedef struct _TIMING_GEN_SETTING_V {
    ULONG   VDEEnd:13;
    ULONG   Unused1:3;
    ULONG   VDEStart:13;
    ULONG   Unused2:3;
} TIMING_GEN_SETTING_V;

typedef struct _BCD_CTL_REG {
    ULONG   Enable:1;
    ULONG   Unused1:15;
    ULONG   Tolerance:8;
    ULONG   Unused2:8;
} BCD_CTL_REG;

typedef union _U_BCD_CTL {
    ULONG           Value;
    BCD_CTL_REG     BCDCtlReg;
} U_BCD_CTL;

typedef struct _COMPRESS_WINDOW_REG {
    ULONG   VerLine:13;
    ULONG   Unused1:3;
    ULONG   HorPixel:13;
    ULONG   Unused2:3;
} COMPRESS_WINDOW_REG;

typedef struct _STREAM_BUF_SIZE {
    ULONG   PacketSize:3;
    ULONG   RingBufNum:2;
    ULONG   Unused1:11;
    ULONG   SkipHighMBThres:7;
    ULONG   SkipTestMode:2;
    ULONG   Unused2:7;
} STREAM_BUF_SIZE;

typedef union _U_STREAM_BUF {
    ULONG               Value;
    STREAM_BUF_SIZE     StreamBufSize;
} U_STREAM_BUF;


typedef struct _COMPRESS_CTL_REG {
    ULONG   JPEGOnly:1; /* True: Jpeg Only mode(Disable VQ), False:Jpeg and VQ mix mode */
    ULONG   En4VQ:1; /* True: 1, 2, 4 color mode, False: 1,2 color mode */
    ULONG   CodecMode:1; /* High and best Quantization encoding/decoding setting*/
    ULONG   DualQuality:1;
    ULONG   EnBest:1;
    ULONG   EnRC4:1;
    ULONG   NorChromaDCTTable:5;
    ULONG   NorLumaDCTTable:5;
    ULONG   EnHigh:1;
    ULONG   TestCtl:2;
    ULONG   UVFmt:1;
    ULONG   HufTable:2;
    ULONG   AlterValue1:5;
    ULONG   AlterValue2:5;
} COMPRESS_CTL_REG;

typedef union _U_COMPRESS_CTL {
    ULONG               Value;
    COMPRESS_CTL_REG    CompressCtlReg;
} U_COMPRESS_CTL;

typedef struct _QUANTI_TABLE_LOW_REG {
    ULONG   ChromaTable:5;
    ULONG   LumaTable:5;
    ULONG   Unused1:22;
} QUANTI_TABLE_LOW_REG;

typedef union _U_CQUANTI_TABLE_LOW {
    ULONG                   Value;
    QUANTI_TABLE_LOW_REG    QTableLowReg;
} U_QUANTI_TABLE_LOW;

typedef struct _QUANTI_VALUE_REG {
    ULONG   High:15;
	ULONG	Unused1:1;
    ULONG   Best:15;
    ULONG   Unused2:1;
} QUANTI_VALUE_REG;

typedef union _U_QUANTI_VALUE {
    ULONG               Value;
    QUANTI_VALUE_REG    QValueReg;
} U_QUANTI_VALUE;

typedef struct _BSD_PARAM_REG {
    ULONG   HighThres:8;
    ULONG   LowThres:8;
    ULONG   HighCount:6;
    ULONG   Unused1:2;
    ULONG   LowCount:6;
    ULONG   Unused2:2;
} BSD_PARAM_REG;

typedef union _U_BSD_PARAM {
    ULONG           Value;
    BSD_PARAM_REG   BSDParamReg;
} U_BSD_PARAM;

typedef struct _VIDEO_INFO {
    BYTE      ExtADCAct;    /* read from modection register */
	BYTE	  EnableRC4;
    BYTE      DownScalingMethod;
    USHORT    AnalogDifferentialThreshold; /* BCD tolerance */
    USHORT    DigitalDifferentialThreshold; /* BCD tolerance */
    USHORT    DstWidth;
    USHORT    DstHeight;
    USHORT    SrcWidth;
    USHORT    SrcHeight;
    BYTE      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
    BYTE      HighChromaTable;
    BYTE      HighDeQuantiValue;
    BYTE      BestDequantiValue;
    U_SEQ_CTL               uSeqCtlReg;
    U_CTL_REG               uCtlReg;
    U_BCD_CTL               uBCDCtlReg;
    U_STREAM_BUF            uStreamBufSize;
    U_COMPRESS_CTL          uCompressCtlReg;
    U_QUANTI_TABLE_LOW      uQTableLowReg;
    U_QUANTI_VALUE          uQValueReg;
    U_BSD_PARAM             uBSDParamReg;
} VIDEO_INFO, *PVIDEO_INFO ;

typedef struct _VIDEOM_SEQ_CTL_REG {
    ULONG   Unused1:1;  //Bit 0
    ULONG   Unused2:1;	//Bit 1
    ULONG   Unused3:1;		//Bit 2
    ULONG   StreamMode:1;	//Bit 3
    ULONG   Unused4:1;	//Bit 4
    ULONG   CodecAutoMode:1;	//Bit 5
	ULONG	Unused6:1;	//Bit 6
    ULONG   Unused7:1;	//Bit 7
    ULONG   SrcSel:1;	//Bit 8
    ULONG   Unused9:1;	//Bit 9
    ULONG   DataType:2;  //Bit[11:10]
    ULONG   Unused12:20;
} VIDEOM_SEQ_CTL_REG;

typedef union _U_VIDEOM_SEQ_CTL {
    ULONG					Value;
    VIDEOM_SEQ_CTL_REG		SeqCtlReg;
} U_VIDEOM_SEQ_CTL;

typedef struct _VIDEOM_INFO {
    BYTE      DownScalingMethod;
    USHORT    AnalogDifferentialThreshold; /* BCD tolerance */
    USHORT    DigitalDifferentialThreshold; /* BCD tolerance */
    USHORT    DstWidth;
    USHORT    DstHeight;
    USHORT    SrcWidth;
    USHORT    SrcHeight;
    BYTE      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
    BYTE      HighChromaTable;
    BYTE      HighDeQuantiValue;
    BYTE      BestDequantiValue;
    BYTE      PacketSize;   //the same as video1 & video2
    BYTE      RingBufNum;
	BYTE	  EnableRC4;
    U_VIDEOM_SEQ_CTL        uSeqCtlReg;
    U_BCD_CTL               uBCDCtlReg;
    U_COMPRESS_CTL          uCompressCtlReg;
    U_QUANTI_TABLE_LOW      uQTableLowReg;
    U_QUANTI_VALUE          uQValueReg;
    U_BSD_PARAM             uBSDParamReg;
} VIDEOM_INFO, *PVIDEOM_INFO ;

typedef struct _VIDEO_MODE_INFO
{
    USHORT    X;
    USHORT    Y;
    USHORT    ColorDepth;
    USHORT    RefreshRate;
    BYTE      ModeIndex;
} VIDEO_MODE_INFO, *PVIDEO_MODE_INFO;

#endif

