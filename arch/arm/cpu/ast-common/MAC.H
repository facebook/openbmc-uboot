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

#ifndef MAC_H
#define MAC_H

#ifdef SPI_BUS
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>
  #define SPI_CS            1
#endif
// ( USE_P2A | USE_LPC )

#if defined(LinuxAP)
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <stdarg.h>
  #include <unistd.h>
  #include <string.h>
  #include <fcntl.h>
  #include <pthread.h>
  #include <sys/mman.h>
  #if defined(__i386__) || defined(__amd64__)
    #include <sys/io.h>
  #endif
#endif
#ifdef SLT_UBOOT
  #include <common.h>
  #include <command.h>
#endif
#ifdef DOS_ALONE
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>
  #include <conio.h>
  #include <dos.h>
  #include <mem.h>
#endif

#include "NCSI.H"
#include "IO.H"

// --------------------------------------------------------------
// Define
// --------------------------------------------------------------
//#define Force_Enable_NewMDIO           //[off][SLT:off] (Force enable new MDC/MDIO)
#define Enable_RecovSCU                //[ON]
//#define Enable_Fast_SCU                //[off]
#define ENABLE_DASA                    //[ON]
//#define Enable_Int125MHz               //[off] only for AST2300/AST2500
//#define ENABLE_ARP_2_WOL               //[off]
//#define Enable_MAC_SWRst               //[off]
//#define Enable_MAC_ExtLoop             //[off]
//#define   Enable_MAC_ExtLoop_PakcegMode//[off]
#define   Enable_No_IOBoundary         //[ON]

//#define Enable_Jumbo
//#define Enable_BufMerge
//#define Disable_VGA
//#define Enable_ShowBW

#define AST2500_IOStageStep                      1
#define AST2500_IOStageShiftBit_In               2 //0~2
#define AST2500_IOStageShiftBit_Out              2 //0~2

//#define SelectSimpleBoundary                                    //[off] Using in debug
//#define SelectSimpleData                                        //[off] Using in debug
//#define SelectSimpleLength                       1514           //[off] 60(0x3c) ~ 1514(0x5ea); 1512(0x5e8)
//#define SelectDesNumber                          8              //[off] 1 ~
//#define SelectSimpleDA                                          //[off] Using in debug
//#define SelectSimpleDes                                         //[off]
//#define SelectLengthInc                                         //[off] Using in debug

#define   SimpleData_Fix                                        //[ON] Using in debug
#define     SimpleData_FixNum                    12
#define     SimpleData_FixVal00                  0x00000000     //[0]no SelectSimpleDA: (60: 0412 8908)(1512: e20d e9da)
#define     SimpleData_FixVal01                  0xffffffff     //[0]no SelectSimpleDA: (60: f48c f14d)(1512: af05 260c)
#define     SimpleData_FixVal02                  0x55555555     //[0]no SelectSimpleDA: (60: 5467 5ecb)(1512: d90a 5368)
#define     SimpleData_FixVal03                  0xaaaaaaaa     //[0]no SelectSimpleDA: (60: a4f9 268e)(1512: 9402 9cbe)
#define     SimpleData_FixVal04                  0x5a5a5a5a     //[1]no SelectSimpleDA: (60: 7f01 e22d)(1512: 4fd3 8012)
#define     SimpleData_FixVal05                  0xc3c3c3c3     //[1]no SelectSimpleDA: (60: 5916 02d5)(1512: 99f1 6127)
#define     SimpleData_FixVal06                  0x96969696     //[1]no SelectSimpleDA: (60: 0963 d516)(1512: a2f6 db95)
#define     SimpleData_FixVal07                  0xf0f0f0f0     //[1]no SelectSimpleDA: (60: dfea 4dab)(1512: 39dc f576)
#define     SimpleData_FixVal08                  0x5555aaaa     //[2]no SelectSimpleDA: (60: b61b 5777)(1512: 4652 ddb0)
#define     SimpleData_FixVal09                  0xffff0000     //[2]no SelectSimpleDA: (60: 16f0 f8f1)(1512: 305d a8d4)
#define     SimpleData_FixVal10                  0x5a5aa5a5     //[2]no SelectSimpleDA: (60: 9d7d eb91)(1512: d08b 0eca)
#define     SimpleData_FixVal11                  0xc3c33c3c     //[2]no SelectSimpleDA: (60: bb6a 0b69)(1512: 06a9 efff)

#define   SimpleData_XORVal                      0x00000000
//#define   SimpleData_XORVal                    0xffffffff

#define   SelectSimpleDA_Dat0                    0x67052301
#define   SelectSimpleDA_Dat1                    0xe0cda089
#define   SelectSimpleDA_Dat2                    0x98badcfe

#define   SelectWOLDA_DatH                       0x206a
#define   SelectWOLDA_DatL                       0x8a374d9b

#ifdef Enable_MAC_ExtLoop_PakcegMode
  #define MOVE_DATA_MB_SEC                       80             // MByte per second to move data
#else
  #define MOVE_DATA_MB_SEC                       800            // MByte per second to move data
#endif

//---------------------------------------------------------
// Frame size
//---------------------------------------------------------
#define ENABLE_RAND_SIZE                         0
#define   RAND_SIZE_SED                          0xffccd
#define   RAND_SIZE_SIMPLE                       0
#define   RAND_SIZE_MIN                          60
#define   RAND_SIZE_MAX                          1514

#define FRAME_SELH_PERD                          7
#ifdef Enable_Jumbo
    #define FRAME_LENH                           9592           //max:9592
    #define FRAME_LENL                           9592           //max:9592
//    #define FRAME_LENH                           9212           //max:9212
//    #define FRAME_LENL                           9211           //max:9212
//    #define FRAME_LENH                           9212           //max:9212
//    #define FRAME_LENL                           9212           //max:9212
//    #define FRAME_LENH                           2048
//    #define FRAME_LENL                           2048
#else
  #ifdef SelectSimpleLength
//    #define FRAME_LENH                           ( SelectSimpleLength + 1 )
//    #define FRAME_LENL                           ( SelectSimpleLength     )
    #define FRAME_LENH                           SelectSimpleLength
    #define FRAME_LENL                           SelectSimpleLength
  #else
//    #define FRAME_LENH                           1514           //max:1514
//    #define FRAME_LENL                           1513           //max:1514
    #define FRAME_LENH                           1514           //max:1514
    #define FRAME_LENL                           1514           //max:1514
  #endif
#endif

#endif // MAC_H
