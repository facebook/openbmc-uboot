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

#include "swfunc.h"

#if defined(LinuxAP)
#endif
#ifdef SLT_UBOOT
#endif
#ifdef SLT_DOS
  #include <stdio.h>
  #include <time.h>
  #include <dos.h>    // For delay()
#endif

#include "typedef.h"
#include "lib.h"

//---------------------------------------------------------
// Print Message 
//---------------------------------------------------------
// for function
#define FP_LOG    0
#define FP_IO     1    
#define STD_OUT   2    
    
#ifdef SLT_UBOOT
    #define PRINT     printf
    #define OUT_OBJ   
    #define FILE_VAR  

    #define GET_OBJ( i )    \
    do {                    \
        if ( i != STD_OUT ) \
            return;         \
    } while ( 0 );
            
#else
    #define PRINT     fprintf
    #define OUT_OBJ   fp,    
    #define FILE_VAR  FILE *fp;
    
    #define GET_OBJ( i )     \
        switch( i ) {        \
            case FP_LOG:     \
                fp = fp_log; \
                break;       \
            case FP_IO:      \
                fp = fp_io;  \
                break;       \
            case STD_OUT:    \
                fp = stdout; \
                break;       \
            default : break; \
        }                
#endif

//---------------------------------------------------------
// Function
//---------------------------------------------------------
#ifdef SLT_UBOOT
    #define DELAY( x )  udelay( x * 1000 ) // For Uboot, the unit of udelay() is us.
    #define GET_CAHR    getc
#endif
#ifdef SLT_DOS
    #define DELAY( x )  delay( x )         // For DOS, the unit of delay() is ms.
    #define GET_CAHR    getchar
#endif

//---------------------------------------------------------
// Default argument
//---------------------------------------------------------
#define  DEF_USER_DEF_PACKET_VAL      0x66666666     //0xff00ff00, 0xf0f0f0f0, 0xcccccccc, 0x55aa55aa, 0x5a5a5a5a, 0x66666666
#define  DEF_IOTIMINGBUND             5              //0/1/3/5/7
#define  DEF_PHY_ADR                  0
#define  DEF_TESTMODE                 0              //[0]0: no burst mode, 1: 0xff, 2: 0x55, 3: random, 4: ARP, 5: ARP, 6: IO timing, 7: IO timing+IO Strength
#define  DEF_LOOP_MAX                 1
#define  DEF_MAC_LOOP_BACK            0              //GCtrl bit6
#define  DEF_SKIP_CHECK_PHY           0              //GCtrl bit4
#define  DEF_INIT_PHY                 1              //GCtrl bit3

#define  SET_1GBPS                    0              // 1G bps
#define  SET_100MBPS                  1              // 100M bps
#define  SET_10MBPS                   2              // 10M bps
#define  SET_1G_100M_10MBPS           3              // 1G and 100M and 10M bps
#define  DEF_SPEED                    SET_1G_100M_10MBPS 
#define  DEF_ARPNUMCNT                0

//---------------------------------------------------------
// MAC information
//---------------------------------------------------------
#if ( AST1010_IOMAP == 1 )
  // AST1010 only has a MAC
  #define MAC_BASE1               AST_MAC1_BASE
  #define MAC_BASE2               AST_MAC1_BASE
  #define MAC_BASE3               AST_MAC1_BASE
  #define MAC_BASE4               AST_MAC1_BASE
#endif

#if ( AST1010_IOMAP == 2 )
  // AST1010 only has a MAC
  #define MAC_BASE1               0x0830000
  #define MAC_BASE2               0x0830000
  #define MAC_BASE3               0x0830000
  #define MAC_BASE4               0x0830000
#endif

#ifndef AST1010_IOMAP
  #define MAC_BASE1               0x1e660000
  #define MAC_BASE2               0x1e680000
  #define MAC_BASE3               0x1e670000
  #define MAC_BASE4               0x1e690000
#endif

#define MDC_Thres                 0x3f
#define MAC_PHYWr                 0x08000000
#define MAC_PHYRd                 0x04000000

#define MAC_PHYWr_New             0x00009400
#define MAC_PHYRd_New             0x00009800
#define MAC_PHYBusy_New           0x00008000

//#define MAC_30h                   0x00001010
//#define MAC_34h                   0x00000000
//#define MAC_38h                   0x00d22f00 //default 0x22f00
//#define MAC_38h                   0x00022f00 //default 0x22f00

#define MAC_40h                   0x40000000

#ifdef Enable_BufMerge
    #define MAC_48h               0x007702F1 //default 0xf1
#else
  #ifdef AST1010_IOMAP
    #define MAC_48h               0x000002F1 //default 0xf1
  #else
    #define MAC_48h               0x000001F1 //default 0xf1
  #endif
#endif

//---------------------------------------------------------
// Data information
//---------------------------------------------------------
#ifdef SelectSimpleBoundary
  #define ZeroCopy_OFFSET       0
#else
  #define ZeroCopy_OFFSET       ( (BurstEnable) ? 0 : 2 )
#endif

//      --------------------------------- DRAM_MapAdr            = TDES_BASE1
//              | TX descriptor ring #1 |
//              ------------------------- DRAM_MapAdr + 0x040000 = RDES_BASE1
//              | RX descriptor ring #1 |
//              ------------------------- DRAM_MapAdr + 0x080000 = TDES_BASE2
//              | TX descriptor ring #2 |
//              ------------------------- DRAM_MapAdr + 0x0C0000 = RDES_BASE2
//              | RX descriptor ring #2 |
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
    #define DRAM_MapAdr                 ( CONFIG_DRAM_SWAP_BASE + 0x00200000 ) // We use 0xA00000 to 0xEFFFFF
    #define CPU_BUS_ADDR_SDRAM_OFFSET   0x01000000                             // In ReMapping function, MAC engine need Bus address
                                                                               // But Coldfire need CPU address, so need to do offset
#endif

#if ( AST1010_IOMAP == 2 )
    #define DRAM_MapAdr                 0x0A00000                              // We use 0xA00000 to 0xEFFFFF
    #define CPU_BUS_ADDR_SDRAM_OFFSET   0
#endif
                                                                               
#ifndef AST1010_IOMAP
  #ifdef AST3200_IOMAP
    #define DRAM_MapAdr           0x80000000
  #else
    #define DRAM_MapAdr           0x44000000   
  #endif
  
  #define   CPU_BUS_ADDR_SDRAM_OFFSET   0
#endif  

  #define TDES_BASE1              ( 0x00000000 + DRAM_MapAdr   )
  #define RDES_BASE1              ( 0x00040000 + DRAM_MapAdr   )
  #define TDES_BASE2              ( 0x00080000 + DRAM_MapAdr   )
  #define RDES_BASE2              ( 0x000C0000 + DRAM_MapAdr   )
                                               
  #define TDES_IniVal             ( 0xb0000000 + FRAME_LEN_Cur )
  #define RDES_IniVal             ( 0x00000fff )
  #define EOR_IniVal              ( 0x40008000 )
  #define HWOwnTx(dat)            (   (dat) & 0x80000000  )
  #define HWOwnRx(dat)            ( !((dat) & 0x80000000) )
  #define HWEOR(dat)              (    dat  & 0x40000000  )

//---------------------------------------------------------
// Error Flag Bits
//---------------------------------------------------------
#define Err_MACMode                            ( 1 <<  0 )   // MAC interface mode mismatch
#define Err_PHY_Type                           ( 1 <<  1 )   // Unidentifiable PHY
#define Err_MALLOC_FrmSize                     ( 1 <<  2 )   // Malloc fail at frame size buffer
#define Err_MALLOC_LastWP                      ( 1 <<  3 )   // Malloc fail at last WP buffer
#define Err_Check_Buf_Data                     ( 1 <<  4 )   // Received data mismatch
#define Err_Check_Des                          ( 1 <<  5 )   // Descriptor error
#define Err_NCSI_LinkFail                      ( 1 <<  6 )   // NCSI packet retry number over flows
#define Err_NCSI_Check_TxOwnTimeOut            ( 1 <<  7 )   // Time out of checking Tx owner bit in NCSI packet
#define Err_NCSI_Check_RxOwnTimeOut            ( 1 <<  8 )   // Time out of checking Rx owner bit in NCSI packet
#define Err_NCSI_Check_ARPOwnTimeOut           ( 1 <<  9 )   // Time out of checking ARP owner bit in NCSI packet
#define Err_NCSI_No_PHY                        ( 1 << 10 )   // Can not find NCSI PHY
#define Err_NCSI_Channel_Num                   ( 1 << 11 )   // NCSI Channel Number Mismatch
#define Err_NCSI_Package_Num                   ( 1 << 12 )   // NCSI Package Number Mismatch
#define Err_PHY_TimeOut                        ( 1 << 13 )   // Time out of read/write/reset PHY register
#define Err_RXBUF_UNAVA                        ( 1 << 14 )   // MAC00h[2]:Receiving buffer unavailable
#define Err_RPKT_LOST                          ( 1 << 15 )   // MAC00h[3]:Received packet lost due to RX FIFO full
#define Err_NPTXBUF_UNAVA                      ( 1 << 16 )   // MAC00h[6]:Normal priority transmit buffer unavailable
#define Err_TPKT_LOST                          ( 1 << 17 )   // MAC00h[7]:Packets transmitted to Ethernet lost
#define Err_DMABufNum                          ( 1 << 18 )   // DMA Buffer is not enough
#define Err_IOMargin                           ( 1 << 19 )   // IO timing margin is not enough
#define Err_IOMarginOUF                        ( 1 << 20 )   // IO timing testing out of boundary
#define Err_MHCLK_Ratio                        ( 1 << 21 )   // Error setting of MAC AHB bus clock (SCU08[18:16])
                                                                
#define Check_Des_TxOwnTimeOut                 ( 1 <<  0 )   // Time out of checking Tx owner bit
#define Check_Des_RxOwnTimeOut                 ( 1 <<  1 )   // Time out of checking Rx owner bit
#define Check_Des_RxErr                        ( 1 <<  2 )   // Input signal RxErr
#define Check_Des_OddNibble                    ( 1 <<  3 )   // Nibble bit happen
#define Check_Des_CRC                          ( 1 <<  4 )   // CRC error of frame
#define Check_Des_RxFIFOFull                   ( 1 <<  5 )   // Rx FIFO full
#define Check_Des_FrameLen                     ( 1 <<  6 )   // Frame length mismatch

#define NCSI_LinkFail_Get_Version_ID           ( 1 <<  0 )   // Time out when Get Version ID
#define NCSI_LinkFail_Get_Capabilities         ( 1 <<  1 )   // Time out when Get Capabilities
#define NCSI_LinkFail_Select_Active_Package    ( 1 <<  2 )   // Time out when Select Active Package
#define NCSI_LinkFail_Enable_Set_MAC_Address   ( 1 <<  3 )   // Time out when Enable Set MAC Address
#define NCSI_LinkFail_Enable_Broadcast_Filter  ( 1 <<  4 )   // Time out when Enable Broadcast Filter
#define NCSI_LinkFail_Enable_Network_TX        ( 1 <<  5 )   // Time out when Enable Network TX
#define NCSI_LinkFail_Enable_Channel           ( 1 <<  6 )   // Time out when Enable Channel
#define NCSI_LinkFail_Disable_Network_TX       ( 1 <<  7 )   // Time out when Disable Network TX
#define NCSI_LinkFail_Disable_Channel          ( 1 <<  8 )   // Time out when Disable Channel

//---------------------------------------------------------
// SCU information
//---------------------------------------------------------
#if ( AST1010_IOMAP == 1 )
    #define SCU_BASE              AST_SCU_BASE
#endif
#if ( AST1010_IOMAP == 2 )
    #define SCU_BASE              0x0841000
#endif

#ifndef AST1010_IOMAP                         
  #define SCU_BASE                0x1e6e2000
#endif                            

#define SCU_48h_AST1010           0x00000200
#define SCU_48h_AST2300           0x00222255

//#ifdef SLT_DOS
//  #define SCU_80h               0x00000000
//  #define SCU_88h               0x00000000
//  #define SCU_90h               0x00000000
//  #define SCU_74h               0x00000000
//#else
//  #define SCU_80h               0x0000000f     //AST2300[3:0]MAC1~4 PHYLINK
//  #define SCU_88h               0xc0000000     //AST2300[31]MAC1 MDIO, [30]MAC1 MDC
//  #define SCU_90h               0x00000004     //AST2300[2 ]MAC2 MDC/MDIO
//  #define SCU_74h               0x06300000     //AST3000[20]MAC2 MDC/MDIO, [21]MAC2 MII, [25]MAC1 PHYLINK, [26]MAC2 PHYLINK
//#endif

//---------------------------------------------------------
// DMA Buffer information
//---------------------------------------------------------
#ifdef FPGA
        #define DRAM_KByteSize    ( 56 * 1024 )
#else
  #ifdef AST1010_IOMAP
        #define DRAM_KByteSize    ( 3 * 1024 )      // DATA buffer only use 0xB00000 to 0xE00000
  #else
      #define DRAM_KByteSize      ( 18 * 1024 )
  #endif
#endif

#define DMA_BASE                ( 0x00100000 + DRAM_MapAdr )

#ifdef Enable_Jumbo
  #define DMA_PakSize           ( 10 * 1024 )
#else
  #define DMA_PakSize           ( 2 * 1024 ) // The size of one LAN packet
#endif

#ifdef SelectSimpleBoundary
  #define DMA_BufSize           (    ( ( ( ( DES_NUMBER + 15 ) * DMA_PakSize ) >> 2 ) << 2 ) ) //vary by DES_NUMBER
#else                                                                                        
  #define DMA_BufSize           (4 + ( ( ( ( DES_NUMBER + 15 ) * DMA_PakSize ) >> 2 ) << 2 ) ) //vary by DES_NUMBER
#endif

#define DMA_BufNum              ( ( DRAM_KByteSize * 1024 ) / ( DMA_BufSize ) )                //vary by DES_NUMBER
#define GET_DMA_BASE_SETUP      ( DMA_BASE )
#define GET_DMA_BASE(x)         ( DMA_BASE + ( ( ( ( x ) % DMA_BufNum ) + 1 ) * DMA_BufSize ) )//vary by DES_NUMBER

#define SEED_START              8
#define DATA_SEED(seed)         ( ( seed ) | (( seed + 1 ) << 16 ) )
#define DATA_IncVal             0x00020001
//#define DATA_IncVal             0x01000001     //fail
//#define DATA_IncVal             0x10000001     //fail
//#define DATA_IncVal             0x10000000     //fail
//#define DATA_IncVal             0x80000000     //fail
//#define DATA_IncVal             0x00000001     //ok
//#define DATA_IncVal             0x01000100     //ok
//#define DATA_IncVal             0x01010000     //ok
//#define DATA_IncVal             0x01010101     //ok
//#define DATA_IncVal             0x00000101     //ok
//#define DATA_IncVal             0x00001111     //fail
//#define DATA_IncVal             0x00000011     //fail
//#define DATA_IncVal             0x10100101     //fail
//#define DATA_IncVal             0xfeff0201
//#define DATA_IncVal             0x00010001
#define PktByteSize              ( ( ( ( ZeroCopy_OFFSET + FRAME_LEN_Cur - 1 ) >> 2 ) + 1) << 2 )

//---------------------------------------------------------
// Delay (ms)
//---------------------------------------------------------
//#define Delay_DesGap              1000        //off
//#define Delay_DesGap              700         //off

//#define Delay_ChkRxOwn            10
//#define Delay_ChkTxOwn            10
#define Delay_CntMax              100000000
//#define Delay_CntMax              1000
//#define Delay_CntMax              8465
//#define Delay_CntMaxIncVal        50000
#define Delay_CntMaxIncVal        47500

#define Delay_PHYRst              100
#define Delay_PHYRd               5

#define Delay_SCU                 11
#define Delay_MACRst              1
#define Delay_MACDump             100

//#define Delay_DES                 1

//---------------------------------------------------------
// Time Out
//---------------------------------------------------------
#define TIME_OUT_Des              10000
#define TIME_OUT_PHY_RW           10000
#define TIME_OUT_PHY_Rst          10000

//#define TIME_OUT_NCSI             300000
#define TIME_OUT_NCSI             30000



//---------------------------------------------------------
// Chip memory MAP
//---------------------------------------------------------
#define LITTLE_ENDIAN_ADDRESS   0
#define BIG_ENDIAN_ADDRESS      1

typedef struct {
    ULONG StartAddr;
    ULONG EndAddr;
}  LittleEndian_Area;

#if ( AST1010_IOMAP == 1 )
  static const LittleEndian_Area LittleEndianArea[] = { 
            { AST_IO_BASE, (AST_IO_BASE + 0x000FFFFF) },
            { 0xFFFFFFFF, 0xFFFFFFFF }  // End
            };
#else
  static const LittleEndian_Area LittleEndianArea[] = { 
            { 0xFFFFFFFF, 0xFFFFFFFF }  // End
            };            
#endif

// ========================================================
// For ncsi.c 

#define DEF_PACKAGE2NUM                         1      // Default value
#define DEF_CHANNEL2NUM                         2      // Default value

typedef struct {
    unsigned char Package_ID;
    unsigned char Channel_ID;
    unsigned long Capabilities_Flags;
    unsigned long Broadcast_Packet_Filter_Capabilities;
    unsigned long Multicast_Packet_Filter_Capabilities;
    unsigned long Buffering_Capabilities;
    unsigned long AEN_Control_Support;
    unsigned long PCI_DID_VID;
    unsigned long ManufacturerID;
} NCSI_Capability;

#undef GLOBAL
#ifdef NCSI_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

GLOBAL  NCSI_Capability     NCSI_Cap_SLT;
GLOBAL  BYTE                number_chl;

GLOBAL  char phy_ncsi (void);
           
// ========================================================
// For mactest 

#undef GLOBAL
#ifdef MACTEST_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

GLOBAL  ULONG    NCSI_DiSChannel;

// 
#ifdef SLT_UBOOT
#else 
// SLT_DOS
GLOBAL  FILE        *fp_log;
GLOBAL  FILE        *fp_io;
#endif

GLOBAL  CHAR        dlymap[16][16];
GLOBAL  CHAR        PrintNCSIEn;
GLOBAL  ULONG       ARPNumCnt;
GLOBAL  CHAR        FileNameMain[256];
GLOBAL  CHAR        FileName[256];
GLOBAL  CHAR        ASTChipName[256];
GLOBAL  CHAR        LOOP_Str[256];
GLOBAL  BYTE        IOTimingBund;
GLOBAL  BYTE        ChannelTolNum;
GLOBAL  BYTE        PackageTolNum;
GLOBAL  ULONG       IOdly_out_reg;
GLOBAL  BYTE        IOdly_out_reg_idx;
GLOBAL  ULONG       Dat_ULONG;
GLOBAL  ULONG       IOdly_incval;
GLOBAL  ULONG       IOdly_in_reg;
GLOBAL  BYTE        IOdly_in_reg_idx;
GLOBAL  ULONG       *wp_lst;
GLOBAL  ULONG       *FRAME_LEN;
GLOBAL  ULONG       DES_NUMBER;
GLOBAL  ULONG       DES_NUMBER_Org;
GLOBAL  int         LOOP_MAX;
GLOBAL  ULONG       LOOP_CheckNum;
GLOBAL  int         Loop;
GLOBAL  ULONG       CheckBuf_MBSize;
GLOBAL  ULONG       Err_Flag;
GLOBAL  ULONG       SCU_f0h_old;
#ifdef AST1010_IOMAP
  GLOBAL  ULONG       SCU_11Ch_old;
#endif
GLOBAL  ULONG       SCU_04h;
GLOBAL  ULONG       SCU_90h_old;
GLOBAL  ULONG       SCU_7ch_old;
GLOBAL  ULONG       MAC_50h;
GLOBAL  ULONG       SCU_ach_old;
GLOBAL  ULONG       SCU_70h_old;
GLOBAL  ULONG       MAC_50h_Speed;
GLOBAL  ULONG       SCU_48h_old;
GLOBAL  ULONG       SCU_48h_mix;
GLOBAL  ULONG       MAC_08h_old;
GLOBAL  ULONG       MAC_0ch_old;
GLOBAL  ULONG       MAC_40h_old;
GLOBAL  ULONG       SCU_08h_old;
GLOBAL  ULONG       MAC_PHYBASE;
GLOBAL  ULONG       LOOP_MAX_arg;
GLOBAL  BYTE        GRun_Mode;
GLOBAL  ULONG       GSpeed_idx;
GLOBAL  CHAR        GSpeed_sel[3];
GLOBAL  CHAR        PHY_ADR;
GLOBAL  CHAR        PHY_ADR_arg;
GLOBAL  CHAR        PHYName[256];
GLOBAL  ULONG       PHY_ID3;
GLOBAL  ULONG       PHY_ID2;
GLOBAL  BYTE        number_pak;
GLOBAL  BYTE        TestMode;
GLOBAL  ULONG       IOStr_i;
GLOBAL  BYTE        IOTimingBund_arg;
GLOBAL  BYTE        GSpeed;
GLOBAL  BYTE        GCtrl;
GLOBAL  ULONG       UserDVal;
GLOBAL  ULONG       H_MAC_BASE;
GLOBAL  ULONG       H_TDES_BASE;
GLOBAL  ULONG       H_RDES_BASE;
GLOBAL  CHAR         Loop_rl[3];
GLOBAL  CHAR        IOTiming;
GLOBAL  CHAR        LOOP_INFINI;
GLOBAL  CHAR        SelectMAC;
GLOBAL  CHAR        Enable_SkipChkPHY;
GLOBAL  CHAR        Enable_MAC34;
GLOBAL  CHAR        IOStrength;
GLOBAL  CHAR        DataDelay;
GLOBAL  CHAR        SA[6];
GLOBAL  CHAR        RxDataEnable;
GLOBAL  CHAR        IEEETesting;
GLOBAL  CHAR        BurstEnable;
GLOBAL  CHAR        MAC_Mode;
GLOBAL  CHAR        Enable_MACLoopback;
GLOBAL  CHAR        Enable_InitPHY;
GLOBAL  CHAR        MAC1_1GEn;
GLOBAL  CHAR        MAC2_RMII;
GLOBAL  CHAR        Enable_RMII;
GLOBAL  CHAR        MAC2_1GEn;
GLOBAL  CHAR        TxDataEnable;
GLOBAL  CHAR        AST2300_NewMDIO;
GLOBAL  CHAR        ASTChipType;
GLOBAL  CHAR        Err_Flag_PrintEn;
GLOBAL  CHAR        AST2400;
GLOBAL  CHAR        AST2300;
GLOBAL  CHAR        AST1100;//Different in phy & dram initiation & dram size & RMII
GLOBAL  CHAR        AST3200;
GLOBAL  CHAR        AST1010;
GLOBAL  SCHAR       IOdly_i_min;
GLOBAL  SCHAR       IOdly_j_min;
GLOBAL  SCHAR       IOdly_i_max;
GLOBAL  SCHAR       IOdly_j_max;
GLOBAL  BYTE        IOdly_i;
GLOBAL  BYTE        IOdly_j;
GLOBAL  BYTE        IOdly_in;
GLOBAL  BYTE        IOdly_out;
GLOBAL  SCHAR       IOdly_in_str;
GLOBAL  BYTE        IOdly_in_end;
GLOBAL  BYTE        IOdly_out_end;
GLOBAL  BYTE        IOdly_out_shf;
GLOBAL  BYTE        IOdly_in_shf;
GLOBAL  SCHAR       IOdly_out_str;
GLOBAL  BYTE        valary[16];

#define MODE_DEDICATED      0x01
#define MODE_NSCI           0x02
GLOBAL  CHAR        ModeSwitch;

#ifdef SLT_UBOOT
#else
    GLOBAL  time_t      timestart;
#endif

#ifdef SPI_BUS
    GLOBAL ULONG   mmiobase;
#else    
    // ( USE_P2A | USE_LPC )
    GLOBAL  UCHAR       *mmiobase;
    GLOBAL  ULONG       ulPCIBaseAddress;
    GLOBAL  ULONG       ulMMIOBaseAddress;
#endif

   
// ========================================================
// For mac.c 
#undef GLOBAL
#ifdef MAC_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif
    
GLOBAL  ULONG   ARP_data[16];
GLOBAL  ULONG   NCSI_LinkFail_Val;
static  const  char version_name[] = VER_NAME;

GLOBAL void    Debug_delay (void);
GLOBAL void    read_scu (void);
GLOBAL void    Setting_scu (void);
GLOBAL void    PrintMode (void);
GLOBAL void    PrintPakNUm (void);
GLOBAL void    PrintChlNUm (void);
GLOBAL void    PrintTest (void);
GLOBAL void    PrintIOTimingBund (void);
GLOBAL void    PrintSpeed (void);
GLOBAL void    PrintCtrl (void);
GLOBAL void    PrintLoop (void);
GLOBAL void    PrintPHYAdr (void);
GLOBAL void    Finish_Close (void);
GLOBAL void    Calculate_LOOP_CheckNum (void);
GLOBAL char    Finish_Check (int value);
GLOBAL void    init_scu1 (void);
GLOBAL void    init_scu_macrst (void);
GLOBAL void    setup_arp (void);
GLOBAL void    TestingSetup (void);
GLOBAL void    init_scu2 (void);
GLOBAL void    init_scu3 (void);
GLOBAL void    init_mac (ULONG base, ULONG tdexbase, ULONG rdexbase);
GLOBAL char    TestingLoop (ULONG loop_checknum);
GLOBAL void    PrintIO_Line_LOG (void);
GLOBAL void    init_phy (int loop_phy);
GLOBAL void    recov_phy (int loop_phy);
GLOBAL int     FindErr (int value);
GLOBAL int     FindErr_Des (int value);
GLOBAL void    PrintIO_Header (BYTE option);
GLOBAL void    Print_Header (BYTE option);
GLOBAL void    PrintIO_LineS (BYTE option);
GLOBAL void    PrintIO_Line (BYTE option);
GLOBAL void    FPri_ErrFlag (BYTE option);

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

