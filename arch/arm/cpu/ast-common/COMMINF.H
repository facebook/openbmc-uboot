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

#ifndef COMMINF_H
#define COMMINF_H

#include "SWFUNC.H"

#if defined(LinuxAP)
  #include <stdio.h>
#endif
#if defined(SLT_UBOOT)
    #ifdef AST1010_CHIP
        #include <aspeed/ast_addr_remap.h>
        #include <aspeed/ast_addr_controller_base.h>
    #endif
#endif
#if defined(DOS_ALONE) || defined(SLT_NEW_ARCH)
  #include <stdio.h>
  #include <time.h>
  #include <dos.h>    // For delay()
#endif

#include "TYPEDEF.H"
#include "LIB.H"
#include "MAC.H"
#include "PHYSPECIAL.H"

//---------------------------------------------------------
// Print Message
//---------------------------------------------------------
// for function
#define FP_LOG                                   0
#define FP_IO                                    1
#define STD_OUT                                  2

#if defined(ENABLE_LOG_FILE)
  #define PRINTF(i, ...)                                   \
      switch( i ) {                                        \
          case FP_LOG:                                     \
              fprintf(eng->fp_log, __VA_ARGS__);           \
              break;                                       \
          case FP_IO:                                      \
              fprintf(eng->fp_io, __VA_ARGS__);            \
              break;                                       \
          case STD_OUT:                                    \
              fprintf(stdout, __VA_ARGS__);                \
              break;                                       \
          default : break; }
#elif defined (SLT_UBOOT)
  #define PRINTF(i, ...)                                          \
     do {                                                         \
         if (i == STD_OUT) {                                      \
             fprintf(stdout, __VA_ARGS__);                        \
             break;                                               \
         }                                                        \
         if ( (display_lantest_log_msg != 0) && (i == FP_LOG) ) { \
             fprintf(stdout, "[Log]:   ");                        \
             fprintf(stdout, __VA_ARGS__);                        \
         }                                                        \
     } while ( 0 );
#else
  #define PRINTF(i, ...) do {} while ( 0 );
#endif

//---------------------------------------------------------
// Function
//---------------------------------------------------------
  #define SWAP_4B( x )                                                             \
                                                 ( ( ( ( x ) & 0xff000000 ) >> 24) \
                                                 | ( ( ( x ) & 0x00ff0000 ) >>  8) \
                                                 | ( ( ( x ) & 0x0000ff00 ) <<  8) \
                                                 | ( ( ( x ) & 0x000000ff ) << 24) \
                                                 )
  #define SWAP_2B( x )                                                             \
                                                 ( ( ( ( x ) & 0xff00     ) >>  8) \
                                                 | ( ( ( x ) & 0x00ff     ) <<  8) \
                                                 )
#if defined(ENABLE_BIG_ENDIAN)
  #define SWAP_2B_BEDN( x )                      ( x )
  #define SWAP_2B_LEDN( x )                      ( SWAP_2B ( x ) )
  #define SWAP_4B_BEDN( x )                      ( x )
  #define SWAP_4B_LEDN( x )                      ( SWAP_4B ( x ) )
#else
  #define SWAP_2B_BEDN( x )                      ( SWAP_2B ( x ) )
  #define SWAP_2B_LEDN( x )                      ( x )
  #define SWAP_4B_BEDN( x )                      ( SWAP_4B ( x ) )
  #define SWAP_4B_LEDN( x )                      ( x )
#endif

#if defined(ENABLE_BIG_ENDIAN)
  #define SWAP_4B_BEDN_NCSI( x )                 ( x )
  #define SWAP_4B_LEDN_NCSI( x )                 ( SWAP_4B( x ) )
#else
  #define SWAP_4B_BEDN_NCSI( x )                 ( SWAP_4B( x ) )
  #define SWAP_4B_LEDN_NCSI( x )                 ( x )
#endif

#if defined(ENABLE_BIG_ENDIAN_MEM)
  #define SWAP_4B_LEDN_MEM( x )                  ( SWAP_4B( x ) )
#else
  #define SWAP_4B_LEDN_MEM( x )                  ( x )
#endif
#if defined(ENABLE_BIG_ENDIAN_REG)
  #define SWAP_4B_LEDN_REG( x )                  ( SWAP_4B( x ) )
#else
  #define SWAP_4B_LEDN_REG( x )                  ( x )
#endif

#if defined(DOS_ALONE) || defined(SLT_NEW_ARCH) || defined(LinuxAP)
//    #define DELAY( x )                           delay( x )         // For DOS, the unit of delay() is m
    #define DELAY( x )                           delay_hwtimer( x )   // For hardware timer #2, the unit of delay() is ms.s.
    #define GET_CAHR                             getchar
#elif defined(SLT_UBOOT)
    #define DELAY( x )                           udelay( ( x ) * 1000 ) // For Uboot, the unit of udelay() is us.
    #define GET_CAHR                             getc
#else
    #define DELAY(val)                           usleep( val * 1000 )
    #define GET_CAHR                             getchar
#endif

//---------------------------------------------------------
// Default argument
//---------------------------------------------------------
#define  DEF_GUSER_DEF_PACKET_VAL                0x66666666     //0xff00ff00, 0xf0f0f0f0, 0xcccccccc, 0x55aa55aa, 0x5a5a5a5a, 0x66666666
#define  DEF_GIOTIMINGBUND                       5              //0/1/3/5/7
#define  DEF_GPHY_ADR                            0
#define  DEF_GTESTMODE                           0              //[0]0: no burst mode, 1: 0xff, 2: 0x55, 3: random, 4: ARP, 5: ARP, 6: IO timing, 7: IO timing+IO Strength
#define  DEF_GLOOP_MAX                           1
#define  DEF_MAC_LOOP_BACK                       0              //GCtrl bit6
#define  DEF_SKIP_CHECK_PHY                      0              //GCtrl bit5
#define  DEF_INTERNAL_LOOP_PHY                   0              //GCtrl bit4
#define  DEF_INIT_PHY                            1              //GCtrl bit3
#define  DEF_DIS_RECOVPHY                        0              //GCtrl bit2
#define  DEF_GCTRL                               (( DEF_MAC_LOOP_BACK << 6 ) | ( DEF_SKIP_CHECK_PHY << 5 ) | ( DEF_INTERNAL_LOOP_PHY << 4 ) | ( DEF_INIT_PHY << 3 ) | ( DEF_DIS_RECOVPHY << 2 ))

#define  SET_1GBPS                               0              // 1G bps
#define  SET_100MBPS                             1              // 100M bps
#define  SET_10MBPS                              2              // 10M bps
#define  SET_1G_100M_10MBPS                      3              // 1G and 100M and 10M bps
#define  SET_100M_10MBPS                         4              // 100M and 10M bps
#ifdef Enable_MAC_ExtLoop
#define  DEF_GSPEED                              SET_1GBPS
#else
#define  DEF_GSPEED                              SET_1G_100M_10MBPS
#endif
#define  DEF_GARPNUMCNT                          0

//---------------------------------------------------------
// MAC information
//---------------------------------------------------------
#if ( AST1010_IOMAP == 1 )
  // AST1010 only has a MAC
  #define MAC_BASE1                              AST_MAC1_BASE
  #define MAC_BASE2                              AST_MAC1_BASE
  #define MAC_BASE3                              AST_MAC1_BASE
  #define MAC_BASE4                              AST_MAC1_BASE
#endif

#if ( AST1010_IOMAP == 2 )
  // AST1010 only has a MAC
  #define MAC_BASE1                              0x00830000
  #define MAC_BASE2                              0x00830000
  #define MAC_BASE3                              0x00830000
  #define MAC_BASE4                              0x00830000
#endif

#ifndef AST1010_IOMAP
  #define MAC_BASE1                              0x1e660000
  #define MAC_BASE2                              0x1e680000
  #define MAC_BASE3                              0x1e670000
  #define MAC_BASE4                              0x1e690000
#endif

#define MDC_Thres                                0x3f
#define MAC_PHYWr                                0x08000000
#define MAC_PHYRd                                0x04000000

#define MAC_PHYWr_New                            0x00009400
#define MAC_PHYRd_New                            0x00009800
#define MAC_PHYBusy_New                          0x00008000

//#define MAC_030_def                              0x00001010
//#define MAC_034_def                              0x00000000
//#define MAC_038_def                              0x00d22f00 //default 0x22f00
//#define MAC_038_def                              0x00022f00 //default 0x22f00
//#define MAC_038_def                              0x00022400 //default 0x22500 (AST2500)
//#define MAC_040_def                              0x00000000

#ifdef Enable_BufMerge
    #define MAC_048_def                          0x007702F1 //default 0xf1
#else
  #ifdef AST2500_IOMAP
    #define MAC_048_def                          0x000002F1 //default 0xf1
  #elif defined(AST1010_IOMAP)
    #define MAC_048_def                          0x000002F1 //default 0xf1
  #else
    #define MAC_048_def                          0x000002F1 //default 0xf1
  #endif
#endif
//#define MAC_058_def                              0x00000040 //0x000001c0

//---------------------------------------------------------
// Data information
//---------------------------------------------------------
#if defined(Enable_MAC_ExtLoop) || defined(Enable_MAC_ExtLoop_PakcegMode) || defined(SelectSimpleBoundary) || defined(PHY_SPECIAL)
      #define ZeroCopy_OFFSET                    0
#else
      #define ZeroCopy_OFFSET                    (( eng->run.TM_Burst ) ? 0 : 2)
#endif

//      --------------------------------- DRAM_MapAdr            = TDES_BASE
//              | TX descriptor ring    |
//              ------------------------- DRAM_MapAdr + 0x040000 = RDES_BASE
//              | RX descriptor ring    |
//              -------------------------
//              | Reserved              |
//              -------------------------
//              | Reserved              |
//      --------------------------------- DRAM_MapAdr + 0x100000 = DMA_BASE    -------------------------
//              |   #1                  |  \                                   |     #1     Tx         |
//  DMA buffer  |                       |   DMA_BufSize                        |      LOOP = 0         |
// ( Tx/Rx )    -------------------------  /                                   --------------------------------------------------
//              |   #2                  |                                      |     #2     Rx         |  #2     Tx             |
//              |                       |                                      |      LOOP = 0         |   LOOP = 1             |
//              -------------------------                                      --------------------------------------------------
//              |   #3                  |                                                              |  #3     Rx             |
//              |                       |                                                              |   LOOP = 1             |
//              -------------------------                                                              -------------------------
//              |   #4                  |                                                                                     ..........
//              |                       |
//              -------------------------
//              |   #5                  |
//              |                       |
//              -------------------------
//              |   #6                  |
//              |                       |
//              -------------------------
//                           .
//                           .
//              -------------------------
//              |   #n, n = DMA_BufNum  |
//              |                       |
//      ---------------------------------

#if ( AST1010_IOMAP == 1 )
    #define DRAM_OFS_BUF                         0x01a00000
    #define DRAM_OFS_WINDOW                      0x00800000
//    #define DRAM_OFS_REMAP                      (CONFIG_DRAM_SWAP_BASE & 0x7fffffff)
    #define DRAM_OFS_REMAP                       0x01800000
#endif
#if ( AST1010_IOMAP == 2 )
    #define DRAM_OFS_BUF                         0x00a00000
    #define DRAM_OFS_WINDOW                      0x01000000
    #define DRAM_OFS_REMAP                       0x00000000
#endif
#ifndef AST1010_IOMAP
  #ifdef Enable_MAC_ExtLoop
    #define DRAM_OFS_BUF                         0x00000000
  #else
    #define DRAM_OFS_BUF                         0x06000000 // 0x04000000 for SLT
  #endif
  #ifdef AST2500_IOMAP
    #define DRAM_OFS_WINDOW                      0x80000000
  #else
    #define DRAM_OFS_WINDOW                      0x40000000
  #endif
    #define DRAM_OFS_REMAP                       0x00000000
#endif

  #define TDES_BASE1                             ( 0x00000000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )
#ifdef Enable_MAC_ExtLoop
  #define RDES_BASE1                             ( 0x00080000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )
#else
  #define RDES_BASE1                             ( 0x00040000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )
#endif
  #define DMA_BASE                               ( 0x00100000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )

  #define TDES_IniVal                            ( 0xb0000000 + eng->dat.FRAME_LEN_Cur )
  #define RDES_IniVal                            ( 0x00000fff )
  #define EOR_IniVal                             ( 0x40008000 )
  #define HWOwnTx(dat)                           ( dat & 0x80000000      )
  #define HWOwnRx(dat)                           ((dat & 0x80000000) == 0)
  #define HWEOR(dat)                             ( dat & 0x40000000      )

  #define AT_MEMRW_BUF( x )                      ( ( x ) + DRAM_OFS_REMAP - DRAM_OFS_WINDOW )
  #define AT_BUF_MEMRW( x )                      ( ( x ) - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )

//---------------------------------------------------------
// Error Flag Bits
//---------------------------------------------------------
#define Err_Flag_MACMode                              ( 1 <<  0 )   // MAC interface mode mismatch
#define Err_Flag_PHY_Type                             ( 1 <<  1 )   // Unidentifiable PHY
#define Err_Flag_MALLOC_FrmSize                       ( 1 <<  2 )   // Malloc fail at frame size buffer
#define Err_Flag_MALLOC_LastWP                        ( 1 <<  3 )   // Malloc fail at last WP buffer
#define Err_Flag_Check_Buf_Data                       ( 1 <<  4 )   // Received data mismatch
#define Err_Flag_Check_Des                            ( 1 <<  5 )   // Descriptor error
#define Err_Flag_NCSI_LinkFail                        ( 1 <<  6 )   // NCSI packet retry number over flows
#define Err_Flag_NCSI_Check_TxOwnTimeOut              ( 1 <<  7 )   // Time out of checking Tx owner bit in NCSI packet
#define Err_Flag_NCSI_Check_RxOwnTimeOut              ( 1 <<  8 )   // Time out of checking Rx owner bit in NCSI packet
#define Err_Flag_NCSI_Check_ARPOwnTimeOut             ( 1 <<  9 )   // Time out of checking ARP owner bit in NCSI packet
#define Err_Flag_NCSI_No_PHY                          ( 1 << 10 )   // Can not find NCSI PHY
#define Err_Flag_NCSI_Channel_Num                     ( 1 << 11 )   // NCSI Channel Number Mismatch
#define Err_Flag_NCSI_Package_Num                     ( 1 << 12 )   // NCSI Package Number Mismatch
#define Err_Flag_PHY_TimeOut_RW                       ( 1 << 13 )   // Time out of read/write PHY register
#define Err_Flag_PHY_TimeOut_Rst                      ( 1 << 14 )   // Time out of reset PHY register
#define Err_Flag_RXBUF_UNAVA                          ( 1 << 15 )   // MAC00h[2]:Receiving buffer unavailable
#define Err_Flag_RPKT_LOST                            ( 1 << 16 )   // MAC00h[3]:Received packet lost due to RX FIFO full
#define Err_Flag_NPTXBUF_UNAVA                        ( 1 << 17 )   // MAC00h[6]:Normal priority transmit buffer unavailable
#define Err_Flag_TPKT_LOST                            ( 1 << 18 )   // MAC00h[7]:Packets transmitted to Ethernet lost
#define Err_Flag_DMABufNum                            ( 1 << 19 )   // DMA Buffer is not enough
#define Err_Flag_IOMargin                             ( 1 << 20 )   // IO timing margin is not enough
#define Err_Flag_IOMarginOUF                          ( 1 << 21 )   // IO timing testing out of boundary
#define Err_Flag_MHCLK_Ratio                          ( 1 << 22 )   // Error setting of MAC AHB bus clock (SCU08[18:16])

#define Wrn_Flag_IOMarginOUF                          ( 1 <<  0 )   // IO timing testing out of boundary

#define Des_Flag_TxOwnTimeOut                         ( 1 <<  0 )   // Time out of checking Tx owner bit
#define Des_Flag_RxOwnTimeOut                         ( 1 <<  1 )   // Time out of checking Rx owner bit
#define Des_Flag_FrameLen                             ( 1 <<  2 )   // Frame length mismatch
#define Des_Flag_RxErr                                ( 1 <<  3 )   // Input signal RxErr
#define Des_Flag_CRC                                  ( 1 <<  4 )   // CRC error of frame
#define Des_Flag_FTL                                  ( 1 <<  5 )   // Frame too long
#define Des_Flag_Runt                                 ( 1 <<  6 )   // Runt packet
#define Des_Flag_OddNibble                            ( 1 <<  7 )   // Nibble bit happen
#define Des_Flag_RxFIFOFull                           ( 1 <<  8 )   // Rx FIFO full

#define NCSI_Flag_Get_Version_ID                      ( 1 <<  0 )   // Time out when Get Version ID
#define NCSI_Flag_Get_Capabilities                    ( 1 <<  1 )   // Time out when Get Capabilities
#define NCSI_Flag_Select_Active_Package               ( 1 <<  2 )   // Time out when Select Active Package
#define NCSI_Flag_Enable_Set_MAC_Address              ( 1 <<  3 )   // Time out when Enable Set MAC Address
#define NCSI_Flag_Enable_Broadcast_Filter             ( 1 <<  4 )   // Time out when Enable Broadcast Filter
#define NCSI_Flag_Enable_Network_TX                   ( 1 <<  5 )   // Time out when Enable Network TX
#define NCSI_Flag_Enable_Channel                      ( 1 <<  6 )   // Time out when Enable Channel
#define NCSI_Flag_Disable_Network_TX                  ( 1 <<  7 )   // Time out when Disable Network TX
#define NCSI_Flag_Disable_Channel                     ( 1 <<  8 )   // Time out when Disable Channel
#define NCSI_Flag_Select_Package                      ( 1 <<  9 )   // Time out when Select Package
#define NCSI_Flag_Deselect_Package                    ( 1 << 10 )   // Time out when Deselect Package
#define NCSI_Flag_Set_Link                            ( 1 << 11 )   // Time out when Set Link
#define NCSI_Flag_Get_Controller_Packet_Statistics    ( 1 << 12 )   // Time out when Get Controller Packet Statistics

//---------------------------------------------------------
// SCU information
//---------------------------------------------------------
#if ( AST1010_IOMAP == 1 )
  #define SMB_BASE                               0x00300000
  #define SCU_BASE                               0x00241000
  #define SDR_BASE                               0x00240000
  #define WDT_BASE                               0x00243000
  #define TIMER_BASE                             0x00242000
  #define GPIO_BASE                              0x0024b000
#endif
#if ( AST1010_IOMAP == 2 )
  #define SMB_BASE                               0x00900000
  #define SCU_BASE                               0x00841000
  #define SDR_BASE                               0x00840000
  #define WDT_BASE                               0x00843000
  #define TIMER_BASE                             0x00842000
  #define GPIO_BASE                              0x0084b000
#endif

#ifndef AST1010_IOMAP
  #define SMB_BASE                               0x1e720000
  #define SCU_BASE                               0x1e6e2000
  #define SDR_BASE                               0x1e6e0000
  #define WDT_BASE                               0x1e785000
  #define TIMER_BASE                             0x1e782000
  #define GPIO_BASE                              0x1e780000
#endif

#define SCU_48h_AST1010                          0x00000200
#define SCU_48h_AST2300                          0x00222255
#define SCU_48h_AST2500                          0x00082208
#define SCU_B8h_AST2500                          0x00082208
#define SCU_BCh_AST2500                          0x00082208

//#define SCU_80h                                  0x0000000f     //AST2300[3:0]MAC1~4 PHYLINK
//#define SCU_88h                                  0xc0000000     //AST2300[31]MAC1 MDIO, [30]MAC1 MDC
//#define SCU_90h                                  0x00000004     //AST2300[2 ]MAC2 MDC/MDIO
//#define SCU_74h                                  0x06300000     //AST3000[20]MAC2 MDC/MDIO, [21]MAC2 MII, [25]MAC1 PHYLINK, [26]MAC2 PHYLINK

//---------------------------------------------------------
// DMA Buffer information
//---------------------------------------------------------
#ifdef FPGA
        #define DRAM_KByteSize                   ( 56 * 1024 )
#else
  #ifdef AST1010_IOMAP
        #define DRAM_KByteSize                   ( 3 * 1024 )      // DATA buffer only use 0xB00000 to 0xE00000
  #else
      #define DRAM_KByteSize                     ( 18 * 1024 )
  #endif
#endif


#ifdef Enable_Jumbo
  #define DMA_PakSize                            ( 10 * 1024 )
#else
  #define DMA_PakSize                            ( 2 * 1024 ) // The size of one LAN packet
#endif

#ifdef SelectSimpleBoundary
  #define DMA_BufSize                            (     ( ( ( ( eng->dat.Des_Num + 15 ) * DMA_PakSize ) >> 2 ) << 2 ) ) //vary by Des_Num
#else
  #define DMA_BufSize                            ( 4 + ( ( ( ( eng->dat.Des_Num + 15 ) * DMA_PakSize ) >> 2 ) << 2 ) ) //vary by Des_Num
#endif

#define DMA_BufNum                               ( ( DRAM_KByteSize * 1024 ) / ( eng->dat.DMABuf_Size ) )                //vary by eng->dat.Des_Num
#define GET_DMA_BASE_SETUP                       ( DMA_BASE )
//#define GET_DMA_BASE(x)                          ( DMA_BASE + ( ( ( ( x ) % eng->dat.DMABuf_Num ) + 1 ) * eng->dat.DMABuf_Size ) )//vary by eng->dat.Des_Num
#define GET_DMA_BASE(x)                          ( DMA_BASE + ( ( ( ( x ) % eng->dat.DMABuf_Num ) + 1 ) * eng->dat.DMABuf_Size ) + ( ( ( x ) % 7 ) * DMA_PakSize ) )//vary by eng->dat.Des_Num

#define SEED_START                               8
#define DATA_SEED(seed)                          ( ( seed ) | (( seed + 1 ) << 16 ) )
#define DATA_IncVal                              0x00020001
//#define DATA_IncVal                              0x01000001     //fail
//#define DATA_IncVal                              0x10000001     //fail
//#define DATA_IncVal                              0x10000000     //fail
//#define DATA_IncVal                              0x80000000     //fail
//#define DATA_IncVal                              0x00000001     //ok
//#define DATA_IncVal                              0x01000100     //ok
//#define DATA_IncVal                              0x01010000     //ok
//#define DATA_IncVal                              0x01010101     //ok
//#define DATA_IncVal                              0x00000101     //ok
//#define DATA_IncVal                              0x00001111     //fail
//#define DATA_IncVal                              0x00000011     //fail
//#define DATA_IncVal                              0x10100101     //fail
//#define DATA_IncVal                              0xfeff0201
//#define DATA_IncVal                              0x00010001
#define PktByteSize                              ( ( ( ( ZeroCopy_OFFSET + eng->dat.FRAME_LEN_Cur - 1 ) >> 2 ) + 1) << 2 )

//---------------------------------------------------------
// Delay (ms)
//---------------------------------------------------------
//#define Delay_DesGap                             1    //off
//#define Delay_CntMax                             40
//#define Delay_CntMax                             1000
//#define Delay_CntMax                             8465
//#define Delay_CntMaxIncVal                       50000
#define Delay_CntMaxIncVal                       47500

#define Delay_PHYRst                             100
//#define Delay_PHYRd                              5
#define Delay_PHYRd                              1         //20150423

//#define Delay_SCU                                11
#define Delay_SCU                                1         //20150423
#define Delay_MACRst                             1
#define Delay_MACDump                            1

//#define Delay_DES                                1
#ifdef Enable_MAC_ExtLoop_PakcegMode
  #define Delay_CheckData                        100
  #define Delay_CheckData_LoopNum                100
#else
//  #define Delay_CheckData                        100
//  #define Delay_CheckData_LoopNum                100
#endif

//---------------------------------------------------------
// Time Out
//---------------------------------------------------------
#ifdef Enable_MAC_ExtLoop_PakcegMode
    #define TIME_OUT_Des_1G                      0
    #define TIME_OUT_Des_100M                    0
    #define TIME_OUT_Des_10M                     0
    #define TIME_OUT_NCSI                        0
    #define TIME_OUT_PHY_RW                      10000
    #define TIME_OUT_PHY_Rst                     10000
#else
  #ifdef DOS_LPC
    #define TIME_OUT_Des_1G                      4         //20150529 (16x slower then DOS_PCIE)
    #define TIME_OUT_Des_100M                    40
    #define TIME_OUT_Des_10M                     200
    #define TIME_OUT_NCSI                        400
    #define TIME_OUT_PHY_RW                      1000
    #define TIME_OUT_PHY_Rst                     10
  #elif defined(DOS_PCIE)
    #define TIME_OUT_Des_1G                      1000       //20150423
    #define TIME_OUT_Des_100M                    2000
    #define TIME_OUT_Des_10M                     5000
    #define TIME_OUT_NCSI                        10000
    #define TIME_OUT_PHY_RW                      200000
    #define TIME_OUT_PHY_Rst                     2000
  #elif ( AST1010_IOMAP == 1 ) /* Coldfire and uboot */
    #define TIME_OUT_Des_1G                      40
    #define TIME_OUT_Des_100M                    400
    #define TIME_OUT_Des_10M                     2000
    #define TIME_OUT_NCSI                        4000
    #define TIME_OUT_PHY_RW                      10000
    #define TIME_OUT_PHY_Rst                     100
  #else
    #define TIME_OUT_Des_1G                      10000     //400
    #define TIME_OUT_Des_100M                    20000     //4000
    #define TIME_OUT_Des_10M                     50000     //20000
    #define TIME_OUT_NCSI                        100000    //40000
    #define TIME_OUT_PHY_RW                      2000000   //100000
    #define TIME_OUT_PHY_Rst                     20000     //1000
  #endif
#endif
//#define TIME_OUT_PHY_RW                          10000
//#define TIME_OUT_PHY_Rst                         10000

////#define TIME_OUT_NCSI                            300000
//#define TIME_OUT_NCSI                            30000     //20150423

//---------------------------------------------------------
// Others
//---------------------------------------------------------
#define Loop_OverFlow                            0x7fffffff

//---------------------------------------------------------
// Chip memory MAP
//---------------------------------------------------------
#define LITTLE_ENDIAN_ADDRESS                    0
#define BIG_ENDIAN_ADDRESS                       1

typedef struct {
    ULONG StartAddr;
    ULONG EndAddr;
}  LittleEndian_Area;

#if ( AST1010_IOMAP == 1 )
  static const LittleEndian_Area LittleEndianArea[] = {
            { AST_IO_BASE, (AST_IO_BASE + 0x000FFFFF) },
            { 0xFFFFFFFF, 0xFFFFFFFF } // End
            };
#else
  static const LittleEndian_Area LittleEndianArea[] = {
            { 0xFFFFFFFF, 0xFFFFFFFF } // End
            };
#endif

// ========================================================
// For ncsi.c

#define DEF_GPACKAGE2NUM                         1         // Default value
#define DEF_GCHANNEL2NUM                         2         // Default value

//---------------------------------------------------------
// Variable
//---------------------------------------------------------
//NC-SI Command Packet
typedef struct {
//Ethernet Header
	unsigned char        DA[6];                        // Destination Address
	unsigned char        SA[6];                        // Source Address
	unsigned short       EtherType;                    // DMTF NC-SI, it should be 0x88F8
//NC-SI Control Packet
	unsigned char        MC_ID;                        // Management Controller should set this field to 0x00
	unsigned char        Header_Revision;              // For NC-SI 1.0 spec, this field has to set 0x01
	unsigned char        Reserved_1;                   // Reserved has to set to 0x00
	unsigned char        IID;                          // Instance ID
	unsigned char        Command;
//	unsigned char        Channel_ID;
	unsigned char        ChID;
	unsigned short       Payload_Length;               // Payload Length = 12 bits, 4 bits are reserved
	unsigned long        Reserved_2;
	unsigned long        Reserved_3;

	unsigned short       Reserved_4;
	unsigned short       Reserved_5;
	unsigned short       Response_Code;
	unsigned short       Reason_Code;
	unsigned char        Payload_Data[64];
}  NCSI_Command_Packet;
//NC-SI Response Packet
typedef struct {
	unsigned char        DA[6];
	unsigned char        SA[6];
	unsigned short       EtherType;                    //DMTF NC-SI
//NC-SI Control Packet
	unsigned char        MC_ID;                        //Management Controller should set this field to 0x00
	unsigned char        Header_Revision;              //For NC-SI 1.0 spec, this field has to set 0x01
	unsigned char        Reserved_1;                   //Reserved has to set to 0x00
	unsigned char        IID;                          //Instance ID
	unsigned char        Command;
	unsigned char        ChID;
	unsigned short       Payload_Length;               //Payload Length = 12 bits, 4 bits are reserved
	unsigned short       Reserved_2;
	unsigned short       Reserved_3;
	unsigned short       Reserved_4;
	unsigned short       Reserved_5;

	unsigned short       Response_Code;
	unsigned short       Reason_Code;
	unsigned char        Payload_Data[64];
}  NCSI_Response_Packet;
typedef struct {
	unsigned char        All_ID                                   ;//__attribute__ ((aligned (4)));
	unsigned char        Package_ID                               ;//__attribute__ ((aligned (4)));
	unsigned char        Channel_ID                               ;//__attribute__ ((aligned (4)));
	unsigned long        Capabilities_Flags                       ;//__attribute__ ((aligned (4)));
	unsigned long        Broadcast_Packet_Filter_Capabilities     ;//__attribute__ ((aligned (4)));
	unsigned long        Multicast_Packet_Filter_Capabilities     ;//__attribute__ ((aligned (4)));
	unsigned long        Buffering_Capabilities                   ;//__attribute__ ((aligned (4)));
	unsigned long        AEN_Control_Support                      ;//__attribute__ ((aligned (4)));
	unsigned char        VLAN_Filter_Count                        ;//__attribute__ ((aligned (4)));
	unsigned char        Mixed_Filter_Count                       ;//__attribute__ ((aligned (4)));
	unsigned char        Multicast_Filter_Count                   ;//__attribute__ ((aligned (4)));
	unsigned char        Unicast_Filter_Count                     ;//__attribute__ ((aligned (4)));
	unsigned char        VLAN_Mode_Support                        ;//__attribute__ ((aligned (4)));
	unsigned char        Channel_Count                            ;//__attribute__ ((aligned (4)));
	unsigned long        PCI_DID_VID                              ;//__attribute__ ((aligned (4)));
	unsigned long        ManufacturerID                           ;//__attribute__ ((aligned (4)));
} NCSI_Capability;
typedef struct {
	ULONG                MAC_000                       ;//__attribute__ ((aligned (4)));
	ULONG                MAC_008                       ;//__attribute__ ((aligned (4)));
	ULONG                MAC_00c                       ;//__attribute__ ((aligned (4)));
	ULONG                MAC_040                       ;//__attribute__ ((aligned (4)));
	ULONG                MAC_040_new                   ;//__attribute__ ((aligned (4)));
	ULONG                MAC_050                       ;//__attribute__ ((aligned (4)));
	ULONG                MAC_050_Speed                 ;//__attribute__ ((aligned (4)));
	ULONG                SCU_004                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_004_mix                   ;//__attribute__ ((aligned (4)));
	ULONG                SCU_004_rstbit                ;//__attribute__ ((aligned (4)));
	ULONG                SCU_004_dis                   ;//__attribute__ ((aligned (4)));
	ULONG                SCU_004_en                    ;//__attribute__ ((aligned (4)));
	ULONG                SCU_008                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_00c                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_00c_clkbit                ;//__attribute__ ((aligned (4)));
	ULONG                SCU_048                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_048_mix                   ;//__attribute__ ((aligned (4)));
	ULONG                SCU_048_default               ;//__attribute__ ((aligned (4)));
	ULONG                SCU_048_check                 ;//__attribute__ ((aligned (4)));
	ULONG                SCU_070                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_074                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_07c                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_080                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_088                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_090                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_09c                       ;//__attribute__ ((aligned (4)));
#ifdef AST1010_CHIP
	ULONG                SCU_0ac                       ;//__attribute__ ((aligned (4)));
#endif
#ifdef AST2500_IOMAP
	ULONG                SCU_0b8                       ;//__attribute__ ((aligned (4)));
	ULONG                SCU_0bc                       ;//__attribute__ ((aligned (4)));
#endif
	ULONG                SCU_0f0                       ;//__attribute__ ((aligned (4)));
	ULONG                WDT_00c                       ;//__attribute__ ((aligned (4)));
	ULONG                WDT_02c                       ;//__attribute__ ((aligned (4)));
#ifdef AST2500_IOMAP
	ULONG                WDT_04c                       ;//__attribute__ ((aligned (4)));
#endif

	CHAR                 SCU_oldvld                    ;//__attribute__ ((aligned (4)));
} MAC_Register;
typedef struct {
	CHAR                 ASTChipType                   ;//__attribute__ ((aligned (4)));
	CHAR                 ASTChipName[64]               ;//__attribute__ ((aligned (4)));
	CHAR                 AST1100                       ;//__attribute__ ((aligned (4)));//Different in phy & dram initiation & dram size & RMII
	CHAR                 AST2300                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST2400                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST1010                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST2500                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST2500A1                     ;//__attribute__ ((aligned (4)));

	CHAR                 MAC_Mode                      ;//__attribute__ ((aligned (4)));
	CHAR                 MAC1_1Gvld                    ;//__attribute__ ((aligned (4)));
	CHAR                 MAC2_1Gvld                    ;//__attribute__ ((aligned (4)));
	CHAR                 MAC1_RMII                     ;//__attribute__ ((aligned (4)));
	CHAR                 MAC2_RMII                     ;//__attribute__ ((aligned (4)));
	CHAR                 MAC_atlast_1Gvld              ;//__attribute__ ((aligned (4)));
	CHAR                 MAC2_vld                      ;//__attribute__ ((aligned (4)));
	CHAR                 MAC34_vld                     ;//__attribute__ ((aligned (4)));

	CHAR                 MAC_1Gvld                     ;//__attribute__ ((aligned (4)));
	CHAR                 MAC_RMII                      ;//__attribute__ ((aligned (4)));

	CHAR                 MHCLK_Ratio                   ;//__attribute__ ((aligned (4)));

	ULONG                VGAMode                       ;//__attribute__ ((aligned (4)));
	char                 VGAModeVld                    ;//__attribute__ ((aligned (4)));
} MAC_Environment;
typedef struct {
	ULONG                GARPNumCnt                    ;//__attribute__ ((aligned (4)));//argv     [6]
	ULONG                GUserDVal                     ;//__attribute__ ((aligned (4)));//argv[8]
	BYTE                 GChk_TimingBund               ;//__attribute__ ((aligned (4)));//argv[7]  [5]
	CHAR                 GPHYADR                       ;//__attribute__ ((aligned (4)));//argv[6]
	BYTE                 GTestMode                     ;//__attribute__ ((aligned (4)));//argv[5]  [4]
	CHAR                 GLOOP_Str[32]                 ;//__attribute__ ((aligned (4)));//argv[4]
	BYTE                 GCtrl                         ;//__attribute__ ((aligned (4)));//argv[3]
	BYTE                 GSpeed                        ;//__attribute__ ((aligned (4)));//argv[2]
	BYTE                 GChannelTolNum                ;//__attribute__ ((aligned (4)));//argv     [3]
	BYTE                 GPackageTolNum                ;//__attribute__ ((aligned (4)));//argv     [2]
	BYTE                 GRun_Mode                     ;//__attribute__ ((aligned (4)));//argv[1]  [1]

	CHAR                 GIEEE_sel                     ;//__attribute__ ((aligned (4)));//argv[7]
	CHAR                 GLOOP_INFINI                  ;//__attribute__ ((aligned (4)));//argv[4]
	ULONG                GLOOP_MAX                     ;//__attribute__ ((aligned (4)));//argv[4]

	CHAR                 GEn_PrintNCSI                 ;//__attribute__ ((aligned (4)));//GARPNumCnt[0]

	CHAR                 GEn_MACLoopback               ;//__attribute__ ((aligned (4)));//GCtrl[6]
	CHAR                 GEn_SkipChkPHY                ;//__attribute__ ((aligned (4)));//GCtrl[5]
	CHAR                 GEn_IntLoopPHY                ;//__attribute__ ((aligned (4)));//GCtrl[4]
	CHAR                 GEn_InitPHY                   ;//__attribute__ ((aligned (4)));//GCtrl[3]
	CHAR                 GDis_RecovPHY                 ;//__attribute__ ((aligned (4)));//GCtrl[2]
	CHAR                 GEn_PHYAdrInv                 ;//__attribute__ ((aligned (4)));//GCtrl[1]
	CHAR                 GEn_SinglePacket              ;//__attribute__ ((aligned (4)));//GCtrl[0]
} MAC_Argument;
typedef struct {
	CHAR                 MAC_idx                       ;//__attribute__ ((aligned (4)));//GRun_Mode
	ULONG                MAC_BASE                      ;//__attribute__ ((aligned (4)));//GRun_Mode

	CHAR                 Speed_1G                      ;//__attribute__ ((aligned (4)));//GSpeed
	CHAR                 Speed_org[3]                  ;//__attribute__ ((aligned (4)));//GSpeed
	CHAR                 Speed_sel[3]                  ;//__attribute__ ((aligned (4)));
	CHAR                 Speed_idx                     ;//__attribute__ ((aligned (4)));

	CHAR                 TM_Burst                      ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_IEEE                       ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_IOTiming                   ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_IOStrength                 ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_TxDataEn                   ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_RxDataEn                   ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_WaitStart                  ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_NCSI_DiSChannel            ;//__attribute__ ((aligned (4)));//GTestMode

	BYTE                 IO_Bund                       ;//__attribute__ ((aligned (4)));
	CHAR                 IO_MrgChk                     ;//__attribute__ ((aligned (4)));

	ULONG                TDES_BASE                     ;//__attribute__ ((aligned (4)));
	ULONG                RDES_BASE                     ;//__attribute__ ((aligned (4)));

	ULONG                NCSI_TxDesBase                ;//__attribute__ ((aligned (4)));
	ULONG                NCSI_RxDesBase                ;//__attribute__ ((aligned (4)));
	int                  NCSI_RxTimeOutScale           ;//__attribute__ ((aligned (4)));

	int                  LOOP_MAX                      ;//__attribute__ ((aligned (4)));
	ULONG                LOOP_CheckNum                 ;//__attribute__ ((aligned (4)));
	ULONG                CheckBuf_MBSize               ;//__attribute__ ((aligned (4)));
	ULONG                TIME_OUT_Des                  ;//__attribute__ ((aligned (4)));
	ULONG                TIME_OUT_Des_PHYRatio         ;//__attribute__ ((aligned (4)));

	int                  Loop_ofcnt                    ;//__attribute__ ((aligned (4)));
	int                  Loop                          ;//__attribute__ ((aligned (4)));
	int                  Loop_rl[3]                    ;//__attribute__ ((aligned (4)));
} MAC_Running;
typedef struct {
	CHAR                 SA[6]                         ;//__attribute__ ((aligned (4)));
	CHAR                 NewMDIO                       ;//__attribute__ ((aligned (4))); //start from AST2300
} MAC_Information;
typedef struct {
	ULONG                PHY_BASE                      ;//__attribute__ ((aligned (4)));
	int                  loop_phy                      ;//__attribute__ ((aligned (4)));
	CHAR                 Adr                           ;//__attribute__ ((aligned (4)));

	CHAR                 PHYName[64]                   ;//__attribute__ ((aligned (4)));
	ULONG                PHY_ID3                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_ID2                       ;//__attribute__ ((aligned (4)));

	ULONG                PHY_00h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_06h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_09h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_0eh                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_10h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_11h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_12h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_14h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_15h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_18h                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_1ch                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_1eh                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_1fh                       ;//__attribute__ ((aligned (4)));
	ULONG                PHY_06hA[7]                   ;//__attribute__ ((aligned (4)));
	BOOLEAN              PHYAdrValid                   ;//__attribute__ ((aligned (4)));
} MAC_PHY;
typedef struct {
	CHAR                 init_done                     ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_MrgEn                     ;//__attribute__ ((aligned (4)));
	CHAR                 Dly_3Regiser                  ;//__attribute__ ((aligned (4)));

	ULONG                Str_reg_idx                   ;//__attribute__ ((aligned (4)));
	BYTE                 Str_reg_Lbit                  ;//__attribute__ ((aligned (4)));
	BYTE                 Str_reg_Hbit                  ;//__attribute__ ((aligned (4)));
	ULONG                Str_reg_value                 ;//__attribute__ ((aligned (4)));
	ULONG                Str_reg_mask                  ;//__attribute__ ((aligned (4)));
	BYTE                 Str_max                       ;//__attribute__ ((aligned (4)));
	BYTE                 Str_shf                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stagebit                  ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage                     ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage_in                  ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage_out                 ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_step                      ;//__attribute__ ((aligned (4)));
	ULONG                Dly_mask                      ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_shf                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_shf                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_shf_regH               ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_shf_regH              ;//__attribute__ ((aligned (4)));
	BYTE                 value_ary[64]                 ;//__attribute__ ((aligned (4)));

	ULONG                Dly_reg_idx                   ;//__attribute__ ((aligned (4)));
	ULONG                Dly_reg_value                 ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_tx[32]           ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_rx[32]           ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_tx_new[32]       ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_rx_new[32]       ;//__attribute__ ((aligned (4)));
	ULONG                Dly_in_reg                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_reg_idx                ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_in_min                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_max                    ;//__attribute__ ((aligned (4)));
	ULONG                Dly_out_reg                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_reg_idx               ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_out_min                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_max                   ;//__attribute__ ((aligned (4)));
	ULONG                Dly_in_cval                   ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_in_str                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_end                    ;//__attribute__ ((aligned (4)));
	ULONG                Dly_out_cval                  ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_out_str                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_end                   ;//__attribute__ ((aligned (4)));

	BYTE                 Str_i                         ;//__attribute__ ((aligned (4)));
	ULONG                Str_val                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in                        ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_selval                 ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_selval                ;//__attribute__ ((aligned (4)));
	ULONG                Dly_val                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_reg_hit               ;//__attribute__ ((aligned (4)));
	CHAR                 Dly_result                    ;//__attribute__ ((aligned (4)));
	CHAR                 dlymap[64][64]                ;//__attribute__ ((aligned (4)));
} MAC_IO;
typedef struct {
#ifdef Enable_ShowBW
	double               Total_frame_len               ;//__attribute__ ((aligned (8)));
#endif
	ULONG                Des_Num                       ;//__attribute__ ((aligned (4)));
	ULONG                Des_Num_Org                   ;//__attribute__ ((aligned (4)));
	ULONG                DMABuf_Size                   ;//__attribute__ ((aligned (4)));
	ULONG                DMABuf_Num                    ;//__attribute__ ((aligned (4)));

	ULONG                *FRAME_LEN                    ;//__attribute__ ((aligned (4)));
	ULONG                FRAME_LEN_Cur                 ;//__attribute__ ((aligned (4)));
	ULONG                *wp_lst                       ;//__attribute__ ((aligned (4)));
	ULONG                wp_fir                        ;//__attribute__ ((aligned (4)));

	ULONG                DMA_Base_Setup                 ;//__attribute__ ((aligned (4)));
	ULONG                DMA_Base_Tx                  ;//__attribute__ ((aligned (4)));
	ULONG                DMA_Base_Rx                   ;//__attribute__ ((aligned (4)));

	ULONG                ARP_data[16]                  ;//__attribute__ ((aligned (4)));
	ULONG                TxDes0DW                      ;//__attribute__ ((aligned (4)));
	ULONG                RxDes0DW                      ;//__attribute__ ((aligned (4)));
	ULONG                RxDes3DW                      ;//__attribute__ ((aligned (4)));

	BYTE                 number_chl                    ;//__attribute__ ((aligned (4)));
	BYTE                 number_pak                    ;//__attribute__ ((aligned (4)));
	ULONG                NCSI_TxDWBUF[512]             ;//__attribute__ ((aligned (4)));
	ULONG                NCSI_RxDWBUF[512]             ;//__attribute__ ((aligned (4)));
	char                 NCSI_CommandStr[512]          ;//__attribute__ ((aligned (4)));
	unsigned char        *NCSI_TxByteBUF               ;//__attribute__ ((aligned (4)));
	unsigned char        *NCSI_RxByteBUF               ;//__attribute__ ((aligned (4)));
	unsigned char        NCSI_Payload_Data[16]         ;//__attribute__ ((aligned (4)));
	unsigned long        Payload_Checksum_NCSI         ;//__attribute__ ((aligned (4)));
} MAC_Data;
typedef struct {
	ULONG                Wrn_Flag                      ;//__attribute__ ((aligned (4)));
	ULONG                Err_Flag                      ;//__attribute__ ((aligned (4)));
	ULONG                Des_Flag                      ;//__attribute__ ((aligned (4)));
	ULONG                NCSI_Flag                     ;//__attribute__ ((aligned (4)));
	ULONG                Bak_Err_Flag                  ;//__attribute__ ((aligned (4)));
	ULONG                Bak_NCSI_Flag                 ;//__attribute__ ((aligned (4)));
	CHAR                 Flag_PrintEn                  ;//__attribute__ ((aligned (4)));
	ULONG                CheckDesFail_DesNum           ;//__attribute__ ((aligned (4)));
} MAC_Flag;
typedef struct {
	MAC_Register         reg;
	MAC_Environment      env;
	MAC_Argument         arg;
	MAC_Running          run;
	MAC_Information      inf;
	MAC_PHY              phy;
	MAC_IO               io;
	MAC_Data             dat;
	MAC_Flag             flg;
	NCSI_Command_Packet  ncsi_req;
	NCSI_Response_Packet ncsi_rsp;
	NCSI_Capability      ncsi_cap;

#if defined(PHY_SPECIAL)
	PHY_BCM5389          BCM5389;
#endif
#if defined(ENABLE_LOG_FILE)
	FILE                 *fp_log;
	FILE                 *fp_io;
#endif
	CHAR                 ModeSwitch;

#ifdef Enable_ShowBW
  #ifdef SLT_UBOOT
  #else
	double               timeused;//__attribute__ ((aligned (8)));
  #endif
#endif
#ifdef SLT_UBOOT
#elif defined(LinuxAP)
#else
	time_t               timestart;
#endif
} MAC_ENGINE;
typedef void (* PHY_SETTING) (MAC_ENGINE *);
typedef struct {
	PHY_SETTING          fp_set;
	PHY_SETTING          fp_clr;
} PHY_ENGINE;

#undef GLOBAL
#ifdef NCSI_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

GLOBAL  char phy_ncsi (MAC_ENGINE *eng);

// ========================================================
// For mactest

#undef GLOBAL
#ifdef MACTEST_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

#define MODE_DEDICATED                           0x01
#define MODE_NSCI                                0x02

#ifdef SPI_BUS
    GLOBAL ULONG             mmiobase;
#else
    // ( USE_P2A | USE_LPC )
    GLOBAL  UCHAR            *mmiobase;
    GLOBAL  ULONG            ulPCIBaseAddress;
    GLOBAL  ULONG            ulMMIOBaseAddress;
#endif

#ifdef SLT_UBOOT
    GLOBAL  BYTE             display_lantest_log_msg;
#endif

// ========================================================
// For mac.c
#undef GLOBAL
#ifdef MAC_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

#if defined(MAC_C)
static  const  char version_name[] = VER_NAME;
static  const  BYTE IOValue_Array_A0[16] = {8,1, 10,3, 12,5, 14,7, 0,9, 2,11, 4,13, 6,15}; // AST2300-A0
#endif

GLOBAL void    Debug_delay (void);
GLOBAL ULONG   Read_Mem_Dat_NCSI_DD (ULONG addr);
GLOBAL ULONG   Read_Mem_Des_NCSI_DD (ULONG addr);
GLOBAL ULONG   Read_Mem_Dat_DD (ULONG addr);
GLOBAL ULONG   Read_Mem_Des_DD (ULONG addr);
GLOBAL ULONG   Read_Reg_MAC_DD (MAC_ENGINE *eng, ULONG addr);
GLOBAL ULONG   Read_Reg_PHY_DD (MAC_ENGINE *eng, ULONG addr);
GLOBAL ULONG   Read_Reg_SCU_DD (ULONG addr);
GLOBAL ULONG   Read_Reg_WDT_DD (ULONG addr);
GLOBAL ULONG   Read_Reg_SDR_DD (ULONG addr);
GLOBAL ULONG   Read_Reg_SMB_DD (ULONG addr);
GLOBAL ULONG   Read_Reg_TIMER_DD (ULONG addr);
GLOBAL ULONG   Read_Reg_GPIO_DD (ULONG addr);
GLOBAL void    Write_Mem_Dat_NCSI_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Mem_Des_NCSI_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Mem_Dat_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Mem_Des_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Reg_MAC_DD (MAC_ENGINE *eng, ULONG addr, ULONG data);
GLOBAL void    Write_Reg_PHY_DD (MAC_ENGINE *eng, ULONG addr, ULONG data);
GLOBAL void    Write_Reg_SCU_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Reg_WDT_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Reg_TIMER_DD (ULONG addr, ULONG data);
GLOBAL void    Write_Reg_GPIO_DD (ULONG addr, ULONG data);
GLOBAL void    init_iodelay (MAC_ENGINE *eng);
GLOBAL int     get_iodelay (MAC_ENGINE *eng);
GLOBAL void    read_scu (MAC_ENGINE *eng);
GLOBAL void    Setting_scu (MAC_ENGINE *eng);
GLOBAL void    PrintMode (MAC_ENGINE *eng);
GLOBAL void    PrintPakNUm (MAC_ENGINE *eng);
GLOBAL void    PrintChlNUm (MAC_ENGINE *eng);
GLOBAL void    PrintTest (MAC_ENGINE *eng);
GLOBAL void    PrintIOTimingBund (MAC_ENGINE *eng);
GLOBAL void    PrintSpeed (MAC_ENGINE *eng);
GLOBAL void    PrintCtrl (MAC_ENGINE *eng);
GLOBAL void    PrintLoop (MAC_ENGINE *eng);
GLOBAL void    PrintPHYAdr (MAC_ENGINE *eng);
GLOBAL void    Finish_Close (MAC_ENGINE *eng);
GLOBAL void    Calculate_LOOP_CheckNum (MAC_ENGINE *eng);
GLOBAL char    Finish_Check (MAC_ENGINE *eng, int value);
GLOBAL void    init_scu1 (MAC_ENGINE *eng);
GLOBAL void    init_scu_macrst (MAC_ENGINE *eng);
GLOBAL void    setup_arp (MAC_ENGINE *eng);
GLOBAL void    TestingSetup (MAC_ENGINE *eng);
GLOBAL void    init_scu2 (MAC_ENGINE *eng);
GLOBAL void    init_scu3 (MAC_ENGINE *eng);
GLOBAL void    get_mac_info (MAC_ENGINE *eng);
GLOBAL void    init_mac (MAC_ENGINE *eng);
GLOBAL char    TestingLoop (MAC_ENGINE *eng, ULONG loop_checknum);
GLOBAL void    PrintIO_Line_LOG (MAC_ENGINE *eng);
GLOBAL void    init_phy (MAC_ENGINE *eng, PHY_ENGINE *phyeng);
GLOBAL BOOLEAN find_phyadr (MAC_ENGINE *eng);
GLOBAL void    phy_write (MAC_ENGINE *eng, int adr, ULONG data);
GLOBAL ULONG   phy_read (MAC_ENGINE *eng, int adr);
GLOBAL void    phy_sel (MAC_ENGINE *eng, PHY_ENGINE *phyeng);
GLOBAL void    recov_phy (MAC_ENGINE *eng, PHY_ENGINE *phyeng);
#ifdef PHY_SPECIAL
GLOBAL void    special_PHY_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_buf_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_recov (MAC_ENGINE *eng);
GLOBAL void    special_PHY_reg_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_debug (MAC_ENGINE *eng);
GLOBAL ULONG   special_PHY_FRAME_LEN (MAC_ENGINE *eng);
GLOBAL ULONG  *special_PHY_txpkt_ptr (MAC_ENGINE *eng);
GLOBAL ULONG  *special_PHY_rxpkt_ptr (MAC_ENGINE *eng);
#endif
GLOBAL int     FindErr (MAC_ENGINE *eng, int value);
GLOBAL int     FindErr_Des (MAC_ENGINE *eng, int value);
GLOBAL void    PrintIO_Header (MAC_ENGINE *eng, BYTE option);
GLOBAL void    Print_Header (MAC_ENGINE *eng, BYTE option);
GLOBAL void    PrintIO_LineS (MAC_ENGINE *eng, BYTE option);
GLOBAL void    PrintIO_Line (MAC_ENGINE *eng, BYTE option);
GLOBAL void    FPri_ErrFlag (MAC_ENGINE *eng, BYTE option);

GLOBAL void init_hwtimer( void );
GLOBAL void delay_hwtimer( USHORT msec );

#ifdef SUPPORT_PHY_LAN9303
// ========================================================
// For LAN9303.c
#undef GLOBAL
#ifdef LAN9303_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

GLOBAL void LAN9303(int num, int phy_adr, int speed, int int_loopback);
#endif // SUPPORT_PHY_LAN9303
#endif // End COMMINF_H

