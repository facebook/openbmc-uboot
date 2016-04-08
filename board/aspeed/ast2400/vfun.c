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
#define BUF_GLOBALS
#include "type.h"
#include "vdef.h"
#include "vreg.h"
#include "crt.h"
#include "vfun.h"

ULONG UnlockSCURegHost(ULONG   MMIOBase, ULONG Key)
{
    WriteMemoryLongHost(SCU_BASE, SCU_PROTECT_REG, Key);
    return ReadMemoryLongHost(SCU_BASE,SCU_PROTECT_REG);
}

void ResetVideoHost(void)
{
	WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CONTROL_REG, VIDEO_RESET_EN << VIDEO_ENGINE_RESET_BIT, VIDEO_ENGINE_RESET_MASK);
	WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CONTROL_REG, VIDEO_RESET_OFF << VIDEO_ENGINE_RESET_BIT, VIDEO_ENGINE_RESET_MASK);
}

void StartModeDetectionTriggerHost(ULONG   MMIOBase, ULONG offset)
{
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, 0, MODE_DETECTION_TRIGGER);
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, MODE_DETECTION_TRIGGER, MODE_DETECTION_TRIGGER);
}

BOOL ReadVideoInterruptHost(ULONG   MMIOBase, ULONG value)
{
    return ((ReadMemoryLongHost(VIDEO_REG_BASE, VIDEO_INT_CONTROL_READ_REG) & value) ? TRUE : FALSE);
}

ULONG UnlockVideoRegHost(ULONG   MMIOBase, ULONG Key)
{
    WriteMemoryLongHost(VIDEO_REG_BASE, KEY_CONTROL_REG, Key);
    return ReadMemoryLongHost(VIDEO_REG_BASE,KEY_CONTROL_REG);
}

void StartVideoCaptureTriggerHost(ULONG   MMIOBase, ULONG offset)
{
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, 0, VIDEO_CAPTURE_TRIGGER);
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, VIDEO_CAPTURE_TRIGGER, VIDEO_CAPTURE_TRIGGER);
}

void StartVideoCodecTriggerHost(ULONG   MMIOBase, ULONG offset)
{
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, 0, VIDEO_CODEC_TRIGGER);
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, VIDEO_CODEC_TRIGGER, VIDEO_CODEC_TRIGGER);
}

void StopModeDetectionTriggerHost(ULONG   MMIOBase, ULONG offset)
{
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, offset, 0, MODE_DETECTION_TRIGGER);
}

void ClearVideoInterruptHost(ULONG   MMIOBase, ULONG value)
{
    //WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO_INT_CONTROL_CLEAR_REG, value, value);
	WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_INT_CONTROL_CLEAR_REG, value);
}

/* UnLock SCU Host and Reset Engine */
BOOL CheckOnStartHost(void)
{
    int i=0, dwValue=0;
    
    do
    {
	dwValue = UnlockSCURegHost(0, SCU_UNLOCK_KEY);
	i++;
    }
    while ((SCU_WRITE_ENABLE != dwValue) && (i<10));

    //Clear SCU Reset Register
    WriteMemoryLongHost(SCU_BASE, SCU_CONTROL_REG, 0);

    WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_STOP_REG, (EN_ECLK | EN_V1CLK | EN_V2CLK), (STOP_ECLK_MASK | STOP_V1CLK_MASK | STOP_V2CLK_MASK));

#if	defined(CONFIG_AST2300)
    WriteMemoryLongWithMASKHost(SCU_BASE, (0x90 + SCU_OFFSET), 0x00000020, 0x00000030);	//enable 24bits
    WriteMemoryLongWithMASKHost(SCU_BASE, (0x88 + SCU_OFFSET), 0x000fff00, 0x000fff00);	//enable video multi-pins    
#else	//AST2100 
    //WriteMemoryLongWithMASKHost(SCU_BASE, SCU_PIN_CTRL1_REG, (VIDEO_PORTA_EN | VIDEO_PORTB_EN | VIDEO_VP1_EN | VIDEO_VP2_EN) , 
    //							(VIDEO_PORTA_MASK | VIDEO_PORTB_MASK | VIDEO_VP1_MASK | VIDEO_VP2_MASK));
    WriteMemoryLongWithMASKHost(SCU_BASE, SCU_PIN_CTRL2_REG, (VIDEO_PORTA_SINGLE_EDGE | VIDEO_PORTB_SINGLE_EDGE) , 
	      						     (VIDEO_PORTA_SINGLE_EDGE_MASK | VIDEO_PORTB_SINGLE_EDGE_MASK));
#endif	      						     

    ResetVideoHost();

    return TRUE;
}

BOOL CheckOnStartClient(void)
{
	int i=0, dwValue=0;
    
	do
	{
		dwValue = UnlockSCURegHost(0, SCU_UNLOCK_KEY);
		i++;
	}
	while ((SCU_WRITE_ENABLE != dwValue) && (i<10));

	//Clear SCU Reset Register
	WriteMemoryLongClient(SCU_BASE, SCU_CONTROL_REG, 0);

	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_CLOCK_STOP_REG, (EN_ECLK | EN_V1CLK | EN_D1CLK | EN_D2CLK | EN_V2CLK), 
								  (STOP_ECLK_MASK | STOP_D1CLK_MASK | STOP_D2CLK_MASK | STOP_V1CLK_MASK | STOP_V2CLK_MASK));

	//WriteMemoryLongWithMASKClient(SCU_BASE, SCU_CLOCK_SELECTION_REG, PORTB_FROM_D2CLK | PORTB_CLOCK_INV_DELAY_3NS | PORTA_CLOCK_INV_DELAY_3NS, PORTB_CLOCK_SEL | PORTB_CLOCK_DELAY_MASK | PORTA_CLOCK_DELAY_MASK);
	//A1EVA
	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_CLOCK_SELECTION_REG, (PORTB_FROM_D2CLK | PORTB_CLOCK_INV_DELAY_1NS | PORTA_CLOCK_INV_DELAY_1NS), (PORTB_CLOCK_SEL | PORTB_CLOCK_DELAY_MASK | PORTA_CLOCK_DELAY_MASK));
	WriteMemoryLongWithMASKClient(SCU_BASE, 0x202C, (0x03<<9), (0x03<<9));

	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_PIN_CTRL1_REG, (VIDEO_PORTA_EN | VIDEO_PORTB_EN | VIDEO_VP1_EN | VIDEO_VP2_EN), 
								  (VIDEO_PORTA_MASK | VIDEO_PORTB_MASK | VIDEO_VP1_MASK | VIDEO_VP2_MASK));

#if CONFIG_AST3000
	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_PIN_CTRL2_REG, (VIDEO_PORTA_DUAL_EDGE | VIDEO_PORTB_DUAL_EDGE), 
								  (VIDEO_PORTA_SINGLE_EDGE_MASK | VIDEO_PORTB_SINGLE_EDGE_MASK));
#else
	//2100 is single edge
	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_PIN_CTRL2_REG, (VIDEO_PORTA_SINGLE_EDGE | VIDEO_PORTB_SINGLE_EDGE), 
								  (VIDEO_PORTA_SINGLE_EDGE_MASK | VIDEO_PORTB_SINGLE_EDGE_MASK));
#endif

	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_CLOCK_STOP_REG, (EN_D1CLK | EN_D2CLK), (STOP_D1CLK_MASK | STOP_D2CLK_MASK));
	WriteMemoryLongWithMASKClient(SCU_BASE, SCU_PIN_CTRL1_REG, VGA_PIN_OFF, VGA_PIN_MASK);

    //ResetVideoHost();

	return TRUE;
}

ULONG  InitializeVideoEngineHost (ULONG                MMIOBase,
                               int                  nVideo,
							   BOOL					HorPolarity,
							   BOOL					VerPolarity)
{
    //ULONG   temp, temp1, temp2; 
    ULONG   dwRegOffset = nVideo * 0x100;
    ULONG   dwValue;
    int     i;


    /* General Video Control */
    //LineBufEn 0
    //dwValue = (COMPRESS_MODE << CODEC_DECOMPRESS_MODE_BIT) | DELAY_VSYNC_EN;
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_CONTROL_REG, dwValue);
    //Video Data Truncation Register
	WriteMemoryLongHost(VIDEO_REG_BASE, 0x328, 0);

	//D2CLK clock must config according to video's line buffer
	if (VIDEO1 == nVideo)
	    dwValue = LINE_BUFFER_VIDEO1;
    else
        dwValue = LINE_BUFFER_VIDEO2;
    
	//D2CLK clock must config according to video's line buffer
	switch (dwValue)
	{
	case LINE_BUFFER_OFF:
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_SELECTION_REG, NORMAL_CRT1, D2CLK_CLOCK_SELECTION_MASK);
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_STOP_REG, STOP_D2CLK, STOP_D2CLK_MASK);
		break;
	case LINE_BUFFER_VIDEO1:
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_SELECTION_REG, V1CLK_VIDEO1 << D2CLK_CLOCK_SELECTION_BIT, D2CLK_CLOCK_SELECTION_MASK);
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_STOP_REG, EN_D2CLK, STOP_D2CLK_MASK);
		break;
	case LINE_BUFFER_VIDEO2:
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_SELECTION_REG, V1CLK_VIDEO2 << D2CLK_CLOCK_SELECTION_BIT, D2CLK_CLOCK_SELECTION_MASK);
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_STOP_REG, EN_D2CLK, STOP_D2CLK_MASK);
		break;
	case LINE_BUFFER_VIDEOM:
		//If select this option, it will config at videoM INIT
		break;
	default:
		break;
	}

    dwValue = 0;
    //VR30 now is capture window in the compression
    dwValue = g_DefHeight << CAPTURE_VER_LINE_BIT |
    g_DefWidth << CAPTURE_HOR_PIXEL_BIT;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_CAPTURE_WINDOWS_REG + dwRegOffset, dwValue);        

    dwValue = 0;
    //VR34 now is destionation window in the compression
	dwValue = g_DefHeight << COMPRESS_VER_LINE_BIT |
	g_DefWidth << COMPRESS_HOR_PIXEL_BIT;

    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_WINDOWS_REG + dwRegOffset, dwValue);        

    //BitCOUNT according compress data format
    dwValue = YUV444_MODE;
	if (YUV444_MODE == dwValue)
		WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_LINE_OFFSET_REG + dwRegOffset, g_DefWidth * INPUT_BITCOUNT_YUV444, BUF_LINE_OFFSET_MASK);
	else
		WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_LINE_OFFSET_REG + dwRegOffset, g_DefWidth * INPUT_BITCOUNT_YUV420, BUF_LINE_OFFSET_MASK);
              
    //  CRC
    //Disable
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_CRC_PRIMARY_REG, 0x0);    
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_CRC_SECOND_REG, 0x0);
    
    /* Sequence Control register */
	//Oonly Encoder need to set
	/* Engine Sequence Contol Register */
	dwValue = (WATCH_DOG_EN << WATCH_DOG_ENABLE_BIT) |
	            VIDEO_CAPTURE_AUTO_MODE |
	            VIDEO_CODEC_AUTO_MODE;
    
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG + dwRegOffset, dwValue);
    
    /* Control register */
	dwValue = (HOR_NEGATIVE == HorPolarity) ? NO_INVERSE_POL : INVERSE_POL;
	dwValue = (((VER_NEGATIVE == VerPolarity) ? NO_INVERSE_POL : INVERSE_POL) << VIDEO_VSYNC_POLARITY_BIT) | dwValue;
    
	/* HW Recommand*/
	//dwValue = (TILE_MODE << 9) | dwValue;
	dwValue = (EXTERNAL_VGA_SOURCE << EXTERNAL_SOURCE_BIT) | dwValue;
    
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_CONTROL_REG + dwRegOffset, dwValue);
        
    /* BCD register */
    //NO BCD
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_BCD_CONTROL_REG + dwRegOffset, dwValue);

    /* Stream Buffer Size register */
    dwValue = (YUV_TEST << SKIP_TEST_MODE_BIT) |
                (PACKET_SIZE_32KB << STREAM_PACKET_SIZE_BIT) |
                (PACKETS_8 << RING_BUF_PACKET_NUM_BIT);
    /* the same with Video1, Video2, and VideoM*/
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_STREAM_BUF_SIZE, dwValue);

    /* Comression control register */
    dwValue = (USE_UV_CIR656 << UV_CIR656_FORMAT_BIT)|
                (JPEG_MIX_MODE << JPEG_ONLY_BIT)|
                (VQ_4_COLOR_MODE << VQ_4_COLOR_BIT)|
                (QUANTI_CODEC_MODE << QUALITY_CODEC_SETTING_BIT)|
                (7 << NORMAL_QUANTI_CHROMI_TABLE_BIT) |
                (23 << NORMAL_QUANTI_LUMI_TABLE_BIT);
   
    //Video2 have same value as video1
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_CONTROL_REG, dwValue);

    /* JPEG Quantization Table register */
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_QUANTI_TABLE_LOW_REG, dwValue);
    
    /* Quantization value register */
    //Video2 have same value as video1
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_QUANTI_VALUE_REG, dwValue);

    //Video BSD Parameter Register
    //Video2 have same value as video1
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_BSD_PARA_REG, dwValue);

    //no scale
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_SCALE_FACTOR_REG,  0x10001000);
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_SCALE_FACTOR_PARAMETER0_REG, 0x00200000);
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_SCALE_FACTOR_PARAMETER1_REG, 0x00200000);
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_SCALE_FACTOR_PARAMETER2_REG, 0x00200000);
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_SCALE_FACTOR_PARAMETER3_REG, 0x00200000);
    return TRUE;
}

ULONG  InitializeVideoEngineClient (ULONG                MMIOBase,
                               int                  nVideo)
{
    //ULONG   temp, temp1, temp2; 
    ULONG   dwRegOffset = nVideo * 0x100;
    ULONG   dwValue;
    int     i;


    /* General Video Control */
    //LineBufEn 0
    dwValue = (DECOMPRESS_MODE << CODEC_DECOMPRESS_MODE_BIT);
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_CONTROL_REG, dwValue);
    //Video Data Truncation Register
	WriteMemoryLongHost(VIDEO_REG_BASE, 0x328, 0);

    //VR30 now is capture window in the compression
    dwValue = g_DefHeight << CAPTURE_VER_LINE_BIT |
    g_DefWidth << CAPTURE_HOR_PIXEL_BIT;
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_CAPTURE_WINDOWS_REG + dwRegOffset, dwValue, CAPTURE_VER_LINE_MASK | CAPTURE_HOR_PIXEL_MASK);        

    //VR34 now is destionation window in the compression
	dwValue = g_DefHeight << COMPRESS_VER_LINE_BIT |
	g_DefWidth << COMPRESS_HOR_PIXEL_BIT;

    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_WINDOWS_REG + dwRegOffset, dwValue, COMPRESS_VER_LINE_MASK | COMPRESS_HOR_PIXEL_MASK);        

    //BitCOUNT according compress data format
    dwValue = YUV444_MODE;
	if (YUV444_MODE == dwValue)
		WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_LINE_OFFSET_REG + dwRegOffset, g_DefWidth * INPUT_BITCOUNT_YUV444, BUF_LINE_OFFSET_MASK);
	else
		WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_LINE_OFFSET_REG + dwRegOffset, g_DefWidth * INPUT_BITCOUNT_YUV420, BUF_LINE_OFFSET_MASK);
              
    //  CRC
    //Disable
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_CRC_PRIMARY_REG, 0x0);    
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO_CRC_SECOND_REG, 0x0);
    
    /* Sequence Control register */
	//Oonly Encoder need to set
	/* Engine Sequence Contol Register */
    dwValue = VIDEO_CAPTURE_AUTO_MODE |
        VIDEO_CODEC_AUTO_MODE;
    
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG + dwRegOffset, dwValue);
    
    /* Control register */
	/* HW Recommand*/
	dwValue = (TILE_MODE << 9); 	
    
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_CONTROL_REG + dwRegOffset, dwValue);
        
    /* BCD register */
    //NO BCD
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_BCD_CONTROL_REG + dwRegOffset, dwValue);

    /* Stream Buffer Size register */
    dwValue = (YUV_TEST << SKIP_TEST_MODE_BIT) |
                (PACKET_SIZE_32KB << STREAM_PACKET_SIZE_BIT) |
                (PACKETS_8 << RING_BUF_PACKET_NUM_BIT);
    /* the same with Video1, Video2, and VideoM*/
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_STREAM_BUF_SIZE, dwValue);
        

    /* Comression control register */
    dwValue = (USE_UV_CIR656 << UV_CIR656_FORMAT_BIT)|
                (JPEG_MIX_MODE << JPEG_ONLY_BIT)|
                (VQ_4_COLOR_MODE << VQ_4_COLOR_BIT)|
                (QUANTI_CODEC_MODE << QUALITY_CODEC_SETTING_BIT)|
                (7 << NORMAL_QUANTI_CHROMI_TABLE_BIT) |
                (23 << NORMAL_QUANTI_LUMI_TABLE_BIT);
   
    //Video2 have same value as video1
    if (VIDEO1 == nVideo)
    {
        WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_CONTROL_REG, dwValue);
    }
    else
    {
        WriteMemoryLongHost(VIDEO_REG_BASE, VIDEOM_COMPRESS_CONTROL_REG, dwValue);
    }

    /* JPEG Quantization Table register */
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_QUANTI_TABLE_LOW_REG, dwValue);
    
    /* Quantization value register */
    //Video2 have same value as video1
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_QUANTI_VALUE_REG, dwValue);

    //Video BSD Parameter Register
    //Video2 have same value as video1
    dwValue = 0;
    WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_BSD_PARA_REG, dwValue);

    return TRUE;
}

BYTE  GetI2CRegClient(ULONG  MMIOBase, 
				 BYTE DeviceSelect, 
				 BYTE DeviceAddress, 
				 BYTE RegisterIndex)
{
    BYTE    Data;
    ULONG   Status;

//  Reset
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 0);
//  Set AC Timing and Speed
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, AC_TIMING);
//  Lower Speed
//    WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0, 0x33317805);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x08, 0);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable Master Mode
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 1);
//  Enable Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0xAF);
//  BYTE I2C Mode
//  Start and Send Device Address
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
//  Wait TX ACK
    do {
        Status = ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
    } while (Status != 1);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Index
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Status = ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
    } while (Status != 1);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Start, Send Device Address + 1(Read Mode), Receive Data
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress + 1);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x1B);
//  Wait Rx Done
    do {
        Status = (ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x04) >> 2;
    } while (Status != 1);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

//  Enable STOP Interrupt
    WriteMemoryLongWithMASKClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
//  Issue STOP Command
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
//  Wait STOP
    do {
        Status = (ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x10) >> 4;
    } while (Status != 1);
//  Disable STOP Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Read Received Data
    Data = (BYTE)((ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20) & 0xFF00) >> 8);

    return    Data;
}

ULONG  SetI2CRegClient(ULONG  MMIOBase, 
                 BYTE   DeviceSelect, 
                 BYTE   DeviceAddress, 
                 BYTE   RegisterIndex, 
                 BYTE   RegisterValue)
{
    ULONG   Status; 
	ULONG   Count = 0;

//  Reset
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 0);
//  Set Speed
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, AC_TIMING);
//  Lower Speed
//    WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0, 0x33317805);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x08, 0);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable Master Mode
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 1);
//  Enable Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0xAF);
//  BYTE I2C Mode
//  Start and Send Device Address
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

		if (2 == Status)
		{
			//Clear Interrupt
			WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
			//Re-Send Start and Send Device Address while NACK return
			WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
			WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
		}
		//else
		{
			if (Count > LOOP_COUNT) {
				return CAN_NOT_FIND_DEVICE;
			}
		}
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Index
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
        if (Count > LOOP_COUNT) {
            return CAN_NOT_FIND_DEVICE;
        }
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Value and Stop
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterValue);
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
        if (Count > LOOP_COUNT) {
            return CAN_NOT_FIND_DEVICE;
        }
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable STOP Interrupt
    WriteMemoryLongWithMASKClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
//  Issue STOP Command
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
//  Wait STOP
    do {
        Count++;
        Status = (ReadMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x10) >> 4;
        if (Count > LOOP_COUNT) {
            return CAN_NOT_FIND_DEVICE;
        }
    } while (Status != 1);
//  Disable STOP Interrupt
    WriteMemoryLongWithMASKClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0, 0x10);
//  Clear Interrupt
    WriteMemoryLongClient(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

    return SET_I2C_DONE;
}
