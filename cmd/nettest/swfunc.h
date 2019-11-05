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

#include <config.h> // for uboot system

//---------------------------------------------------------
// Program information
//---------------------------------------------------------
/* PHY in Normal mode */
#define VER_NAME "Ver 0.77 version @2017/12/20 1310"

/* ========================================================== */
#define NETESTCMD_MAX_ARGS CONFIG_SYS_MAXARGS

/* == Step 4:==========   Select PHY    ================== */

/* ======================== Program flow control ======================== */
#define RUN_STEP                                 5
// 1: parameter setup
// 2: mdc/mdio pinmux,
// 4: Data Initial
// 5: ALL

/* ====================== Switch print debug message ====================== */
//#define   DbgPrn_Enable_Debug_pause                //[off]
//#define DBG_LOG_FUNC_NAME
#define   DbgPrn_ErrFlg                          0
#define   DbgPrn_BufAdr                          0
#define   DbgPrn_Bufdat                          0
#define   DbgPrn_BufdatDetail                    0
#define   DbgPrn_PHYRW                           0
#define   DbgPrn_PHYInit                         0
#define   DbgPrn_PHYName                         0
#define   DbgPrn_DumpMACCnt                      0
#define   DbgPrn_Info                            1
#define   DbgPrn_FRAME_LEN                       0

#ifdef DBG_LOG_FUNC_NAME
#define nt_log_func_name()				\
			do{printf("%s\n", __func__); debug_pause();}while(0)
#else
#define nt_log_func_name(...)
#endif
/* ============ Enable or Disable Check item of the descriptor ============ */
    #define CheckRxOwn
    #define CheckRxLen
    #define CheckRxErr
    #define CheckCRC
    #define CheckFTL
    #define CheckRunt
//    #define CheckOddNibble
    #define CheckRxFIFOFull
  

//    #define CheckRxbufUNAVA
    #define CheckRPktLost
//    #define CheckNPTxbufUNAVA
    #define CheckTPktLost
    #define CheckRxBuf

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
