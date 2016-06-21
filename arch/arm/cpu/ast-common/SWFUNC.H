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

#ifndef SWFUNC_H
#define SWFUNC_H

#if !defined(DOS_PCIE) && !defined(DOS_LPC) && !defined(NewSLT) && !defined(LinuxAP) && !defined(WINSLT)
  #include <config.h> // for uboot system
#endif

//---------------------------------------------------------
// Program information
//---------------------------------------------------------
#ifdef PHY_SPECIAL
#define    VER_NAME    "@2015/12/10 Verification PHY_SPECIAL"
#else
#define    VER_NAME    "Ver 0.67 version @2019/02/22 1419 for BCM54210E"
#endif

/* == Auto setting ==========================================  */
/* ====  Support OS system                                     */
/*        "LinuxAP"                                            */
/*        "Windows"                                            */
/*        "SLT_UBOOT"                                          */
/*        "DOS_ALONE" for independent DOS program              */
/*        "SLT_NEW_ARCH" for new arch. SLT tool                */
/* ====  Support interface                                     */
/*        "SPI_BUS" or "USE_LPC" or "USE_P2A"                  */
/* ====  Support Chip                                          */
/*        "AST1010_CHIP" or "AST2500_IOMAP" or "FPGA"          */
#if defined(CONFIG_ARCH_AST3200)
  #define SLT_UBOOT
  #define AST2500_IOMAP
#endif

#if defined(CONFIG_ARCH_AST2500)
  #define SLT_UBOOT
  #define AST2500_IOMAP
#endif

#if defined(CONFIG_AST1500)
  #define SLT_UBOOT
  #define ISUBOOT_1_1_4
  #define AST2500_IOMAP
#endif

#if defined(CONFIG_ARCH_AST2400)
  #define SLT_UBOOT
#endif

#if defined(CONFIG_ARCH_AST2300)
  #define SLT_UBOOT
#endif

#if defined(CONFIG_AST1010)
  #define SLT_UBOOT
  #define AST1010_CHIP
#endif

#if defined(DOS_PCIE)
  #define DOS_ALONE
  #define USE_P2A
  #if defined(AST25XX)
    #define AST2500_IOMAP
  #endif
#endif

#if defined(DOS_LPC)
  #define DOS_ALONE
  #define USE_LPC
  #if defined(AST25XX)
    #define AST2500_IOMAP
  #endif
  #if defined(AST10XX)
    #define AST1010_CHIP
  #endif
#endif

#if defined(NewSLT) || defined(LinuxAP) || defined(WINSLT)
  #ifndef  LinuxAP
  #define SLT_NEW_ARCH
  #endif
  #if defined(SLT2500) || defined(SLT2510) || defined(SLT2520) || defined(SLT2530) || defined(SLT1520) || defined(SLT1525) || defined(SLT3200)
    #define AST2500_IOMAP
  #endif
#endif

/* ========================================================== */
#if defined(SLT_UBOOT) && defined(AST1010_CHIP)
    #define ENABLE_BIG_ENDIAN
#endif

/* ========================================================== */
#if !defined (SLT_UBOOT)
  #define ENABLE_LOG_FILE
#endif

/* ========================================================== */
#ifdef AST1010_CHIP
  #ifdef SLT_UBOOT
        #define AST1010_IOMAP                    1
  #endif
  #ifdef DOS_ALONE
        #define AST1010_IOMAP                    2

        // AST1010 only has LPC interface
        #undef USE_P2A
        #undef SPI_BUS
        #define USE_LPC
  #endif
#endif

#ifdef SLT_UBOOT
  #ifdef ISUBOOT_1_1_4
    #define NETESTCMD_MAX_ARGS CFG_MAXARGS
  #else
    #define NETESTCMD_MAX_ARGS CONFIG_SYS_MAXARGS
  #endif
#endif

/* == Step 4:==========   Select PHY    ================== */
//#define SUPPORT_PHY_LAN9303                                   // Initial PHY via I2C bus
#define LAN9303_I2C_BUSNUM                       6              // 1-based
#define LAN9303_I2C_ADR                          0x14

/* ====================== Program ======================== */
// The "PHY_NCSI" option is only for DOS compiler
#if defined (PHY_NCSI)
  #ifdef SLT_UBOOT
        #error Wrong setting......
  #endif
#endif

#if defined (PHY_NCSI)
  #ifdef SUPPORT_PHY_LAN9303
        #error Wrong setting (Can not support LAN9303)......
  #endif
#endif

/* =================  Check setting  ===================== */
#ifdef SLT_UBOOT
  #ifdef DOS_ALONE
        #error Can NOT support two OS
  #endif
#endif
#ifdef DOS_ALONE
  #ifdef SLT_UBOOT
        #error Can NOT support two OS
  #endif
#endif

#ifdef USE_P2A
  #ifdef SLT_UBOOT
        #error Can NOT be set PCI bus in Uboot
  #endif
#endif
#ifdef USE_LPC
  #ifdef SLT_UBOOT
        #error Can NOT be set LPC bus in Uboot
  #endif
#endif
#ifdef SPI_BUS
  #ifdef SLT_UBOOT
        #error Can NOT be set SPI bus in Uboot
  #endif
#endif

/* ======================== Program flow control ======================== */
#define RUN_STEP                                 5
// 0: read_scu
// 1: parameter setup
// 2: init_scu1,
// 3: init_scu_macrst
// 4: Data Initial
// 5: ALL

/* ====================== Switch print debug message ====================== */
#define   DbgPrn_ErrFlg                          0
#define   DbgPrn_BufAdr                          0 //1
#define   DbgPrn_Bufdat                          0
#define   DbgPrn_BufdatDetail                    0
#define   DbgPrn_PHYRW                           0
#define   DbgPrn_PHYInit                         0
#define   DbgPrn_PHYName                         0
#define   DbgPrn_DumpMACCnt                      0
#define   DbgPrn_Info                            0 //1
#define   DbgPrn_FRAME_LEN                       0


/* ============ Enable or Disable Check item of the descriptor ============ */
#ifdef Enable_MAC_ExtLoop
#else
    #define CheckRxOwn
    #define CheckRxLen
    #define CheckRxErr
    #define CheckCRC
    #define CheckFTL
    #define CheckRunt
//    #define CheckOddNibble
    #define CheckRxFIFOFull
  #ifdef Enable_MAC_ExtLoop_PakcegMode
    #define CheckDataEveryTime
  #else
//    #define CheckDataEveryTime
  #endif

//    #define CheckRxbufUNAVA
    #define CheckRPktLost
//    #define CheckNPTxbufUNAVA
    #define CheckTPktLost
    #define CheckRxBuf
#endif

#ifdef CheckRxErr
  #define Check_ErrMask_RxErr                    0x00040000 //bit18
#else
  #define Check_ErrMask_RxErr                    0x00000000
#endif
#ifdef CheckCRC
  #define Check_ErrMask_CRC                      0x00080000 //bit19
#else
  #define Check_ErrMask_CRC                      0x00000000
#endif
#ifdef CheckFTL
  #define Check_ErrMask_FTL                      0x00100000 //bit20
#else
  #define Check_ErrMask_FTL                      0x00000000
#endif
#ifdef CheckRunt
  #define Check_ErrMask_Runt                     0x00200000 //bit21
#else
  #define Check_ErrMask_Runt                     0x00000000
#endif
#ifdef CheckOddNibble
  #define Check_ErrMask_OddNibble                0x00400000 //bit22
#else
  #define Check_ErrMask_OddNibble                0x00000000
#endif
#ifdef CheckRxFIFOFull
  #define Check_ErrMask_RxFIFOFull               0x00800000 //bit23
#else
  #define Check_ErrMask_RxFIFOFull               0x00000000
#endif
#define Check_ErrMask_ALL                        ( Check_ErrMask_RxErr | Check_ErrMask_CRC | Check_ErrMask_FTL | Check_ErrMask_Runt | Check_ErrMask_OddNibble | Check_ErrMask_RxFIFOFull )


#endif // SWFUNC_H
