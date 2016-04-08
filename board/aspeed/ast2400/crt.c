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

#include "type.h"
#include "vesa.h"
#include "vdef.h"
#include "vfun.h"
#include "vreg.h"
#include "crt.h"

ULONG AST3000DCLKTableV [] = {
    0x00046515,     /* 00: VCLK25_175 */  
    0x00047255,     /* 01: VCLK28_322 */
    0x0004682a,     /* 02: VCLK31_5         */
    0x0004672a,     /* 03: VCLK36          */
    0x00046c50,     /* 04: VCLK40           */
    0x00046842,     /* 05: VCLK49_5         */
    0x00006c32,     /* 06: VCLK50           */
    0x00006a2f,     /* 07: VCLK56_25        */
    0x00006c41,     /* 08: VCLK65  */
    0x00006832,     /* 09: VCLK75         */
    0x0000672e,     /* 0A: VCLK78_75        */
    0x0000683f,     /* 0B: VCLK94_5         */
    0x00004824,     /* 0C: VCLK108          */
	0x00004723,		/* 0D: VCLK119			*/
    0x0000482d,     /* 0E: VCLK135          */
	0x00004B37,		/* 0F: VCLK146_25		*/
    0x0000472e,     /* 10: VCLK157_5        */
    0x00004836,     /* 11: VCLK162          */

};

BOOL CheckDAC(int nCRTIndex)
{
	BYTE	btValue;	
	BOOL	bValue;

	BYTE btDeviceSelect;

	switch (nCRTIndex)
	{
	case CRT_1:
		btDeviceSelect = DEVICE_ADDRESS_CH7301_CRT1;
		break;
	case CRT_2:
		btDeviceSelect = DEVICE_ADDRESS_CH7301_CRT2;
		break;
	default:
		printf("CRTIndex is not 1 or 2");
		return FALSE;
		break;
	}

	//Enable all DAC's and set register 21h[0] = '0'
	//DVIP and DVIL disable for DAC
	SetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_PM_REG, 0x00);

	btValue = GetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_DC_REG);
	btValue = btValue & 0xFE;
	SetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_DC_REG, btValue);

	//Set SENSE bit to 1
	btValue = GetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG);
	btValue = btValue | 0x01;
	SetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG, btValue);

	//Reset SENSE bit to 0
	btValue = GetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG);
	btValue = btValue & 0xFE;
	SetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG, btValue);

	bValue = (GetI2CRegClient(0, DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG) & CD_DACT) ? TRUE : FALSE;

	return bValue;
}

VOID  SetCH7301C(ULONG	MMIOBase,
				 int	nCRTIndex,
                 int	inFreqRange,
				 int	inOperating)
{
	BYTE btDeviceSelect;
	BYTE btValue;

//#ifdef EVB_CLIENT
	//output RGB doesn't need to set CH7301
	//if (1 == inOperating)
	//	return;
//#endif

	switch (nCRTIndex)
	{
	case CRT_1:
		btDeviceSelect = 0xEA;

		break;
	case CRT_2:
		btDeviceSelect = 0xEC;

		break;
	default:
		printf("CRTIndex is not 1 or 2");
		return;
		break;
	}

	if (inFreqRange <= VCLK65)
	{
	  printf("ch7301: low f \n");
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x33, 0x08);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x34, 0x16);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x36, 0x60);
	}
	else
	{
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x33, 0x06);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x34, 0x26);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x36, 0xA0);
	}

	switch (inOperating)
	{
	case 0:
		//DVI is normal function
		 SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x49, 0xC0);
		 SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x1D, 0x47);
		break;
	case 1:
		//RGB
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x48, 0x18);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x49, 0x0);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x56, 0x0);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x21, 0x9);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x1D, 0x48);
		SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x1C, 0x00);
		break;
	default:
		break;
	};
}

void SetASTModeTiming (ULONG MMIOBase, int nCRTIndex, BYTE ModeIndex, BYTE ColorDepth)
{
    ULONG    temp, RetraceStart, RetraceEnd, DisplayOffset, TerminalCount, bpp;

//  Access CRT Engine
	//  SetPolarity
    WriteMemoryLongWithMASKClient(SCU_BASE, CRT1_CONTROL_REG + nCRTIndex*0x60, ((vModeTable[ModeIndex].HorPolarity << HOR_SYNC_SELECT_BIT) | (vModeTable[ModeIndex].VerPolarity << VER_SYNC_SELECT_BIT)), (HOR_SYNC_SELECT_MASK|VER_SYNC_SELECT_MASK));

#if CONFIG_AST3000
	WriteMemoryLongClient(SCU_BASE, CRT1_CONTROL2_REG + nCRTIndex*0x60, 0xc0);
#else
    //2100 is single edge
	WriteMemoryLongClient(SCU_BASE, CRT1_CONTROL2_REG + nCRTIndex*0x60, 0x80);
#endif    
	//  Horizontal Timing
	temp = 0;
    temp = ((vModeTable[ModeIndex].HorizontalActive - 1) << HOR_ENABLE_END_BIT) | ((vModeTable[ModeIndex].HorizontalTotal - 1) << HOR_TOTAL_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_HOR_TOTAL_END_REG  + nCRTIndex*0x60, temp);

    RetraceStart = vModeTable[ModeIndex].HorizontalTotal - vModeTable[ModeIndex].HBackPorch - vModeTable[ModeIndex].HSyncTime - vModeTable[ModeIndex].HLeftBorder - 1;
    RetraceEnd = (RetraceStart + vModeTable[ModeIndex].HSyncTime);
	temp = 0;
    temp = (RetraceEnd << HOR_RETRACE_END_BIT) | (RetraceStart << HOR_RETRACE_START_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_HOR_RETRACE_REG  + nCRTIndex*0x60, temp);

	//  Vertical Timing
	temp = 0;
    temp = ((vModeTable[ModeIndex].VerticalActive - 1) << VER_ENABLE_END_BIT) | ((vModeTable[ModeIndex].VerticalTotal - 1) << VER_TOTAL_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_VER_TOTAL_END_REG  + nCRTIndex*0x60, temp);

	temp = 0;
    RetraceStart = vModeTable[ModeIndex].VerticalTotal - vModeTable[ModeIndex].VBackPorch - vModeTable[ModeIndex].VSyncTime - vModeTable[ModeIndex].VTopBorder - 1;
    RetraceEnd = (RetraceStart + vModeTable[ModeIndex].VSyncTime);
    temp = (RetraceEnd << VER_RETRACE_END_BIT) | (RetraceStart << VER_RETRACE_START_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_VER_RETRACE_REG  + nCRTIndex*0x60, temp);

	//  Set CRT Display Offset and Terminal Count
    if (ColorDepth == RGB_565) {
        bpp = 16;
    }
    else {
        bpp = 32;
    }

    DisplayOffset = vModeTable[ModeIndex].HorizontalActive * bpp / 8;
    TerminalCount = vModeTable[ModeIndex].HorizontalActive * bpp / 64;
    if (ColorDepth == YUV_444) {
        TerminalCount = TerminalCount * 3 / 4;
    }
    if (((vModeTable[ModeIndex].HorizontalActive * bpp) % 64) != 0) {
        TerminalCount++;
    }

    WriteMemoryLongClient(SCU_BASE, CRT1_DISPLAY_OFFSET + nCRTIndex*0x60, ((TerminalCount << TERMINAL_COUNT_BIT) | DisplayOffset));

	//  Set Color Format
    WriteMemoryLongWithMASKClient(SCU_BASE, CRT1_CONTROL_REG + nCRTIndex*0x60, (ColorDepth << FORMAT_SELECT_BIT), FORMAT_SELECT_MASK);

	//  Set Threshold
	temp = 0;
    temp = (CRT_HIGH_THRESHOLD_VALUE << THRES_HIGHT_BIT) | (CRT_LOW_THRESHOLD_VALUE << THRES_LOW_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_THRESHOLD_REG + nCRTIndex*0x60, temp);

	WriteMemoryLongClient(SCU_BASE, CRT1_VIDEO_PLL_REG + nCRTIndex*0x60, AST3000DCLKTableV[vModeTable[ModeIndex].PixelClock]);
}

void SetASTCenter1024ModeTiming (ULONG MMIOBase, int nCRTIndex, BYTE ModeIndex, BYTE ColorDepth)
{
    ULONG    temp, RetraceStart, RetraceEnd, DisplayOffset, TerminalCount, bpp;

	//  Access CRT Engine
	//  SetPolarity
    WriteMemoryLongWithMASKClient(SCU_BASE, CRT1_CONTROL_REG + nCRTIndex*0x60, (HOR_NEGATIVE << HOR_SYNC_SELECT_BIT) | (VER_NEGATIVE << VER_SYNC_SELECT_BIT), HOR_SYNC_SELECT_MASK|VER_SYNC_SELECT_MASK);

	WriteMemoryLongClient(SCU_BASE, CRT1_CONTROL2_REG + nCRTIndex*0x60, 0xC0);

	//  Horizontal Timing
	temp = 0;
    temp = ((vModeTable[ModeIndex].HorizontalActive - 1) << HOR_ENABLE_END_BIT) | ((vModeTable[10].HorizontalTotal - 1) << HOR_TOTAL_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_HOR_TOTAL_END_REG + nCRTIndex*0x60, temp);

    RetraceStart = vModeTable[10].HorizontalTotal - vModeTable[10].HBackPorch - vModeTable[10].HSyncTime - vModeTable[10].HLeftBorder - 1;
    RetraceStart = RetraceStart - (vModeTable[10].HorizontalActive - vModeTable[ModeIndex].HorizontalActive) / 2 - 1;
    RetraceEnd = (RetraceStart + vModeTable[10].HSyncTime);
	temp = 0;
    temp = (RetraceEnd << HOR_RETRACE_END_BIT) | (RetraceStart << HOR_RETRACE_START_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_HOR_RETRACE_REG + nCRTIndex*0x60, temp);

	//  Vertical Timing
	temp = 0;
    temp = ((vModeTable[ModeIndex].VerticalActive - 1) << VER_ENABLE_END_BIT) | ((vModeTable[10].VerticalTotal - 1) << VER_TOTAL_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_VER_TOTAL_END_REG + nCRTIndex*0x60, temp);

    RetraceStart = vModeTable[10].VerticalTotal - vModeTable[10].VBackPorch - vModeTable[10].VSyncTime - vModeTable[10].VTopBorder - 1;
    RetraceStart = RetraceStart - (vModeTable[10].VerticalActive - vModeTable[ModeIndex].VerticalActive) / 2 - 1;
    RetraceEnd = (RetraceStart + vModeTable[10].VSyncTime);
    temp = (RetraceEnd << VER_RETRACE_END_BIT) | (RetraceStart << VER_RETRACE_START_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_VER_RETRACE_REG + nCRTIndex*0x60, temp);

	//  Set CRT Display Offset and Terminal Count
    if (ColorDepth == RGB_565) {
        bpp = 16;
    }
    else {
        bpp = 32;
    }
    DisplayOffset = vModeTable[ModeIndex].HorizontalActive * bpp / 8;
    TerminalCount = vModeTable[ModeIndex].HorizontalActive * bpp / 64;
    if (ColorDepth == YUV_444) {
        TerminalCount = TerminalCount * 3 / 4;
    }
    if (((vModeTable[ModeIndex].HorizontalActive * bpp) % 64) != 0) {
        TerminalCount++;
    }

    WriteMemoryLongClient(SCU_BASE, CRT1_DISPLAY_OFFSET + nCRTIndex*0x60, (TerminalCount << TERMINAL_COUNT_BIT) | DisplayOffset);

	//  Set Color Format
    WriteMemoryLongWithMASKClient(SCU_BASE, CRT1_CONTROL_REG + nCRTIndex*0x60, (ColorDepth << FORMAT_SELECT_BIT), FORMAT_SELECT_MASK);

	//  Set Threshold
	temp = 0;
    temp = (CRT_HIGH_THRESHOLD_VALUE << THRES_HIGHT_BIT) | (CRT_LOW_THRESHOLD_VALUE << THRES_LOW_BIT);
    WriteMemoryLongClient(SCU_BASE, CRT1_THRESHOLD_REG + nCRTIndex*0x60, temp);
    
	// Set DCLK
	WriteMemoryLongClient(SCU_BASE, CRT1_VIDEO_PLL_REG + nCRTIndex*0x60, AST3000DCLKTableV[vModeTable[ModeIndex].PixelClock]);

}

BOOL  ASTSetModeV (ULONG MMIOBase, int nCRTIndex, ULONG VGABaseAddr, USHORT Horizontal, USHORT Vertical, BYTE ColorFormat, BYTE CenterMode)
{
    BYTE    i, ModeIndex;
    BOOL	bDAC;	
    ULONG   ulTemp;

    //  Access CRT Engine
    //Enable CRT1 graph
    WriteMemoryLongWithMASKClient(SCU_BASE, CRT1_CONTROL_REG + 0x60*nCRTIndex, GRAPH_DISPLAY_ON, GRAPH_DISPLAY_MASK);
    
    //  Set CRT Display Start Address
    WriteMemoryLongWithMASKClient(SCU_BASE, CRT1_DISPLAY_ADDRESS + 0x60*nCRTIndex, VGABaseAddr, DISPLAY_ADDRESS_MASK);

    for (i = 0; i < Mode60HZCount; i++) {
        if ((vModeTable[i].HorizontalActive == Horizontal) && (vModeTable[i].VerticalActive == Vertical)) {

	    ModeIndex = i;

            if (CenterMode != 1) {
                SetASTModeTiming(MMIOBase, nCRTIndex, i, ColorFormat);
            }
            else {
                SetASTCenter1024ModeTiming (MMIOBase, nCRTIndex, i, ColorFormat);
            }

            //use internal video out sigal and don't need use 7301
            /*
			bDAC = CheckDAC(nCRTIndex);
            
			SetCH7301C(0,
				nCRTIndex,
				vModeTable[ModeIndex].PixelClock,
				bDAC);		//For RGB
			*/	
			return TRUE;
        }
    }

    return FALSE;
}

