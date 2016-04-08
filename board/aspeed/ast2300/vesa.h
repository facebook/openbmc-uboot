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
#ifndef _VESA_H_
#define _VESA_H_

typedef enum 
{
	VCLK25_175	= 0x00,
	VCLK28_322	= 0x01,
	VCLK31_5	= 0x02,
	VCLK31_574	= 0x03,
	VCLK32_76	= 0x04,
	VCLK33_154	= 0x05,
	VCLK36		= 0x06,
	VCLK40		= 0x07,
	VCLK45_978	= 0x08,
	VCLK49_5	= 0x09,
	VCLK50		= 0x0A,
	VCLK52_95	= 0x0B,
	VCLK56_25	= 0x0C,
	VCLK65		= 0x0D,
	VCLK74_48	= 0x0E,
	VCLK75		= 0x0F,
	VCLK78_75	= 0x10,
	VCLK79_373	= 0x11,
	VCLK81_624	= 0x12,
	VCLK83_462	= 0x13,
	VCLK84_715	= 0x14,
	VCLK94_5	= 0x15,
	VCLK106_5	= 0x16,
	VCLK108		= 0x17,
	VCLK119		= 0x18,
	VCLK135		= 0x19,
	VCLK136_358 = 0x1A,
	VCLK146_25	= 0x1B,
	VCLK154		= 0x1C,
	VCLK157_5	= 0x1D,
	VCLK162		= 0x1E
} ePIXEL_CLOCK;

typedef struct {
    USHORT    HorizontalTotal;
    USHORT    VerticalTotal;
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    BYTE      RefreshRate;
    double    HorizontalFrequency;
    USHORT    HSyncTime;
    USHORT    HBackPorch;
    USHORT    VSyncTime;
    USHORT    VBackPorch;
    USHORT    HLeftBorder;
    USHORT    HRightBorder;
    USHORT    VBottomBorder;
    USHORT	  VTopBorder;
	USHORT	  PixelClock;
	BOOL	  HorPolarity;
	BOOL	  VerPolarity;
    BYTE      ADCIndex1;
    BYTE      ADCIndex2;
    BYTE      ADCIndex3;
    BYTE      ADCIndex5;
    BYTE      ADCIndex6;
    BYTE      ADCIndex7;
    BYTE      ADCIndex8;
    BYTE      ADCIndex9;
    BYTE      ADCIndexA;
    BYTE      ADCIndexF;
    BYTE      ADCIndex15;
    int       HorizontalShift;
    int       VerticalShift;
} VESA_MODE;

#define HOR_POSITIVE 0
#define HOR_NEGATIVE 1
#define VER_POSITIVE 0
#define VER_NEGATIVE 1

#ifdef VESA_GLOBALS

//  Note: Modified for modes which have border issue
VESA_MODE vModeTable[] = {
////////////////////////// 60Hz mode
// 720x480 done
  {1056, 497,  720,  480,  60, 29.900, 88,  104, 3, 13, 0, 0, 0, 0,	VCLK31_574,  HOR_NEGATIVE, VER_NEGATIVE, 0x41,	0xF0, 0x48, 0x05, 0x20, 0x58, 0x60, 0x60, 0x60,	0x5E, 0xFE,  6,	2},
// 848x480 done                                                                               
  {1064, 517,  848,  480,  60, 31.160, 88,  91,  3, 26, 0, 0, 0, 0,	VCLK33_154,  HOR_NEGATIVE, VER_NEGATIVE, 0x42,	0x70, 0x48, 0x05, 0x20, 0x58, 0x60, 0x60, 0x60,	0x5E, 0xFE,  6,	2},
  {800,  525,  640,  480,  60, 31.469, 96,  40,  2, 25, 1, 1, 8, 8,	VCLK25_175,  HOR_NEGATIVE, VER_NEGATIVE, 0x31,	0xF0, 0x48, 0x05, 0x20, 0x60, 0x60, 0x60, 0x60,	0x5E, 0xFE,  6,	2},
// 720x576                                                                                    
  {912,  597,  720,  576,  60, 35.920, 72,  88,  3, 17, 0, 0, 0, 0,	VCLK32_76,   HOR_NEGATIVE, VER_NEGATIVE, 0x38,	0xF0, 0x48, 0x05, 0x20, 0x48, 0x60, 0x60, 0x60,	0x5E, 0xFE,  6,	2},
// 960x600 GTF done                                                                           
  {1232, 622,  960,  600,  60, 37.320, 96,  136, 3, 18, 0, 0, 0, 0,	VCLK45_978,  HOR_NEGATIVE, VER_NEGATIVE, 0x4C,	0xF0, 0x60, 0x05, 0x20, 0x60, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1056, 628,  800,  600,  60, 37.879, 128, 88,  4, 23, 0, 0, 0, 0,	VCLK40,	     HOR_POSITIVE, VER_POSITIVE, 0x41,	0xF0, 0x60, 0x05, 0x20, 0x80, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
// 1088x612 GTF done                                                                          
  {1392, 634,  1088, 612,  60, 38.04,  112, 152, 3, 18, 0, 0, 0, 0,	VCLK52_95,   HOR_NEGATIVE, VER_NEGATIVE, 0x56,	0xF0, 0x60, 0x05, 0x20, 0x70, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
// 1280x720 GTF done                                                                          
  {1664, 746,  1280, 720,  60, 44.760, 136, 192, 3, 22, 0, 0, 0, 0,	VCLK74_48,   HOR_NEGATIVE, VER_NEGATIVE, 0x67,	0xF0, 0xA8, 0x05, 0x20, 0x88, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
// 1360x768 GTF done                                                                          
  {1776, 795,  1360, 768,  60, 47.700, 144, 208, 3, 23, 0, 0, 0, 0,	VCLK84_715,  HOR_NEGATIVE, VER_NEGATIVE, 0x6E,	0xF0, 0xA8, 0x05, 0x20, 0x90, 0x60, 0x60, 0x60, 0x5E, 0xFE,  7, 1},
// 1280x768 done                                                                              
  {1664, 798,  1280, 768,  60, 47.700, 128, 184, 7, 20, 0, 0, 0, 0,	VCLK79_373,  HOR_NEGATIVE, VER_NEGATIVE, 0x67,	0xF0, 0xA8, 0x05, 0x20, 0x80, 0x60, 0x60, 0x60, 0x5E, 0xFE,  7, 1},
  {1344, 806,  1024, 768,  60, 48.363, 136, 160, 6, 29, 0, 0, 0, 0,	VCLK65,	     HOR_NEGATIVE, VER_NEGATIVE, 0x53,	0xF0, 0xA8, 0x05, 0x20, 0x88, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	7},
// 1280x800 GTF done                                                                          
  {1680, 828,  1280, 800,  60, 49.680, 136, 200, 3, 24, 0, 0, 0, 0,	VCLK83_462,  HOR_NEGATIVE, VER_NEGATIVE, 0x68,	0xF0, 0xA8, 0x05, 0x20, 0x88, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
// 1152x864 GTF done                                                                          
  {1520, 895,  1152, 864,  60, 53.700, 120, 184, 3, 27, 0, 0, 0, 0,	VCLK81_624,  HOR_NEGATIVE, VER_NEGATIVE, 0x5E,	0xF0, 0xA8, 0x05, 0x20, 0x78, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
// 1600x900 GTF done                                                                          
  {2128, 932,  1600, 900,  60, 55.920, 168, 264, 3, 28, 0, 0, 0, 0,	VCLK119,	 HOR_NEGATIVE, VER_NEGATIVE, 0x84,	0xF0, 0xA8, 0x05, 0x20, 0xA8, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
// 1440X900 CVT done                                                                          
  {1904, 933,  1440, 900,  60, 55.935, 152, 232, 6,	25,	0, 0, 0, 0,	VCLK106_5,   HOR_NEGATIVE, VER_POSITIVE, 0x76,	0xF0, 0xA8, 0x05, 0x20, 0x96, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
  {1800, 1000, 1280, 960,  60, 60.000, 112, 312, 3, 36, 0, 0, 0, 0,	VCLK108,	 HOR_POSITIVE, VER_POSITIVE, 0x70,	0x70, 0xA8, 0x05, 0x20, 0x70, 0x60, 0x60, 0x60, 0x5E, 0xFE, -1, 0},
// 1600x1024 GTF done                                                                            
  {2144, 1060, 1600, 1024, 60, 63.600, 168, 272, 3, 32, 0, 0, 0, 0,	VCLK136_358, HOR_NEGATIVE, VER_NEGATIVE, 0x85,	0xF0, 0xE8, 0x05, 0x20, 0xA8, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1688, 1066, 1280, 1024, 60, 63.981, 112, 248, 3, 38, 0, 0, 0, 0,	VCLK108,	 HOR_POSITIVE, VER_POSITIVE, 0x69,	0x70, 0xA8, 0x05, 0x20, 0x70, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
// 1680X1050 CVT done Reduced Blanking                                                          
  {1840, 1080, 1680, 1050, 60, 64.674, 32,  80,	 6,	21,	0, 0, 0, 0,	VCLK119,	 HOR_POSITIVE, VER_NEGATIVE, 0x72,	0xF0, 0xA8, 0x05, 0x20, 0x20, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
// 1920X1200 CVT done Reduced Blanking                           
  {2080, 1235, 1920, 1200, 60, 74.038, 32,  80,  6, 26, 0, 0, 0, 0, VCLK154,     HOR_POSITIVE, VER_NEGATIVE, 0x81,	0xF0, 0xA8, 0x05, 0x20, 0x20, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  //{2160,           1250,           1600,           1200, 60,75.000,           192,        304,         3,          46,          0,            0,             0,              0,			VCLK162,		HOR_POSITIVE,	VER_POSITIVE},
  {2160, 1248, 1600, 1200, 60, 75.000, 192, 304, 3, 46, 0, 0, 0, 0,	VCLK162,	 HOR_POSITIVE, VER_POSITIVE, 0x86,	0xF0, 0xE8, 0x05, 0x20, 0xC0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
                                                              
//////////////////////  Not 60Hz mode                                                                                                                  
  {900,  449,  720,  400,  70, 31.469, 108, 45,  2, 25, 1, 1, 8, 8,	0,			 HOR_NEGATIVE, VER_NEGATIVE, 0x38,	0x30, 0x48, 0x05, 0x20, 0x6C, 0x60, 0x60, 0x60,	0x5E, 0xFE,  6,	1},
  {832,  520,  640,  480,  72, 37.861, 40,  120, 3, 20, 1, 1, 8, 8,	0,			 HOR_NEGATIVE, VER_NEGATIVE, 0x33,	0xF0, 0x48, 0x05, 0x20, 0x28, 0x60, 0x60, 0x60,	0x5E, 0xFE,  6,	3},
  {840,  500,  640,  480,  75, 37.500, 64,  120, 3, 16, 0, 0, 0, 0,	0,			 HOR_NEGATIVE, VER_NEGATIVE, 0x34,	0x70, 0x48, 0x05, 0x20, 0x40, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	3},                                         
  {832,  509,  640,  480,  85, 43.269, 56,  80,  3, 25, 0, 0, 0, 0,	0,			 HOR_NEGATIVE, VER_NEGATIVE, 0x33,	0xF0, 0x48, 0x05, 0x20, 0x38, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	3},
  {1024, 625,  800,  600,  56, 35.156, 72,  128, 2, 22, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x3F,	0xF0, 0x60, 0x05, 0x20, 0x48, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1040, 666,  800,  600,  72, 48.077, 120, 64,  6, 23, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x40,	0xF0, 0x60, 0x05, 0x20, 0x78, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1056, 625,  800,  600,  75, 46.875, 80,  160, 3, 21, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x41,	0xF0, 0x60, 0x05, 0x20, 0x50, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1048, 631,  800,  600,  85, 53.674, 64,  152, 3, 27, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x41,	0x70, 0x60, 0x05, 0x20, 0x40, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1328, 806,  1024, 768,  70, 56.476, 136, 144, 6, 29, 0, 0, 0, 0,	0,			 HOR_NEGATIVE, VER_NEGATIVE, 0x52,	0xF0, 0xA8, 0x05, 0x20, 0x88, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	7},
  {1312, 800,  1024, 768,  75, 60.023, 96,  176, 3, 28, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x51,	0xF0, 0xA8, 0x05, 0x20, 0x60, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	1},
  {1376, 808,  1024, 768,  85, 68.677, 96,  208, 3, 36, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x55,	0xF0, 0xA8, 0x05, 0x20, 0x60, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	1},
  {1600, 900,  1152, 864,  75, 67.500, 128, 256, 3, 32, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x63,	0xF0, 0xA8, 0x05, 0x20, 0x80, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1728, 1011, 1280, 960,  85, 85.938, 160, 224, 3, 47, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x6B,	0xF0, 0xA8, 0x05, 0x20, 0xA0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1688, 1066, 1280, 1024, 75, 79.976, 144, 248, 3, 38, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x69,	0x70, 0xE8, 0x05, 0x20, 0x90, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {1728, 1072, 1280, 1024, 85, 91.146, 160, 224, 3, 44, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x6B,	0xF0, 0xA8, 0x05, 0x20, 0xA0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {2160, 1250, 1600, 1200, 65, 81.250, 192, 304, 3, 46, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x86,	0xF0, 0xA8, 0x05, 0x20, 0xC0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {2160, 1250, 1600, 1200, 70, 87.500, 192, 304, 3, 46, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x86,	0xF0, 0xA8, 0x05, 0x20, 0xC0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {2160, 1250, 1600, 1200, 75, 93.750, 192, 304, 3, 46, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x86,	0xF0, 0xA8, 0x05, 0x20, 0xC0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0},
  {2160, 1250, 1600, 1200, 85, 106.250,192, 304, 3, 46, 0, 0, 0, 0,	0,			 HOR_POSITIVE, VER_POSITIVE, 0x86,	0xF0, 0xA8, 0x05, 0x20, 0xC0, 0x60, 0x60, 0x60,	0x5E, 0xFE, -1,	0}
};

USHORT  ModeNumberCount = sizeof (vModeTable) / sizeof (VESA_MODE);
USHORT  Mode60HZCount = 21;

#else  /* NOT VESA_GLOBALS */
extern VESA_MODE vModeTable[];
extern USHORT  ModeNumberCount;
extern USHORT  Mode60HZCount;
#endif

#endif /* _VESA_H_ */


