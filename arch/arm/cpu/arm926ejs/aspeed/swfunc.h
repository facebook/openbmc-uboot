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


//---------------------------------------------------------
// Program information
//---------------------------------------------------------
#define    VER_NAME    "Ver 0.34 version @2014/03/25 0932"

/* == Step 1: ====== Support OS system =================== */
// LinuxAP
// #define Windows
#define SLT_UBOOT
//#define SLT_DOS

/* == Step 2:======== Support interface ================== */
/* Choose One */
//#define SPI_BUS
//#define USE_LPC
//#define USE_P2A     // PCI or PCIe bus to AHB bus

/* == Step 3:==========  Support Chip   ================== */
//#define AST1010_CHIP
//#define AST3200_IOMAP
//#define FPGA

#ifdef AST1010_CHIP
    #ifdef SLT_UBOOT
        #define AST1010_IOMAP   1
    #endif
    #ifdef SLT_DOS
        #define AST1010_IOMAP   2
        
        // AST1010 only has LPC interface 
        #undef USE_P2A
        #undef SPI_BUS
        #define USE_LPC
    #endif
#endif   

/* == Step 4:==========   Select PHY    ================== */
//#define SUPPORT_PHY_LAN9303        // Initial PHY via I2C bus
#define LAN9303_I2C_BUSNUM      6  // 1-based
#define LAN9303_I2C_ADR         0x14

/* ====================== Program ======================== */
// The "PHY_NCSI" option is only for DOS compiler
#if defined (PHY_NCSI)
    #ifdef SLT_UBOOT
        #error Wrong setting......
    #endif
#endif

#if defined (PHY_NCSI)
    #ifdef SUPPORT_PHY_LAN9303
        #error Wrong setting (Can't support LAN9303)......
    #endif
#endif

/* =================  Check setting  ===================== */
#ifdef SLT_UBOOT
    #ifdef SLT_DOS
        #error Can NOT support two OS 
    #endif
#endif
#ifdef SLT_DOS
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
#define RUN_STEP                     5
// 0: read_scu
// 1: parameter setup
// 2: init_scu1,
// 3: init_scu_macrst
// 4: Data Initial
// 5: ALL

/* ====================== Switch print debug message ====================== */
#define   DbgPrn_Enable_Debug_delay  0
//#define   DbgPrn_FuncHeader          0 //1
#define   DbgPrn_ErrFlg              0
#define   DbgPrn_BufAdr              0 //1
#define   DbgPrn_Bufdat              0
#define   DbgPrn_BufdatDetail        0
#define   DbgPrn_PHYRW               0
#define   DbgPrn_PHYInit             0
#define   DbgPrn_PHYName             0
#define   DbgPrn_DumpMACCnt          0
#define   DbgPrn_Info                0 //1
#define   DbgPrn_FRAME_LEN           0


/* ============ Enable or Disable Check item of the descriptor ============ */
#define   CheckRxOwn
#define   CheckRxErr
//#define   CheckOddNibble
#define   CheckCRC
#define   CheckRxFIFOFull
#define   CheckRxLen
//#define   CheckDataEveryTime

//#define   CheckRxbufUNAVA
#define   CheckRPktLost
//#define   CheckNPTxbufUNAVA
#define   CheckTPktLost
#define   CheckRxBuf

#endif // SWFUNC_H
