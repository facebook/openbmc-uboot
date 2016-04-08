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
#define MACTEST_C
static const char ThisFile[] = "mactest.c";

#include "swfunc.h"

#ifdef SLT_UBOOT
  #include <common.h>
  #include <command.h>
  #include <post.h>
  #include <malloc.h>
  #include <net.h>
  #include "comminf.h"
  #include "stduboot.h"
  #include "io.h"
#else
  #include <stdlib.h>
  #include <string.h>
  #include "lib.h"
  #include "comminf.h"
  #include "io.h"
#endif

const  BYTE        Val_Array[16]    = {0,1, 2,3, 4,5, 6,7, 8,9, 10,11, 12,13, 14,15}; // AST2300-A1
const  BYTE        Val_Array_A0[16] = {8,1, 10,3, 12,5, 14,7, 0,9, 2,11, 4,13, 6,15}; // AST2300-A0

#ifdef SLT_UBOOT
int main_function(int argc, char *argv[])
#endif
#ifdef SLT_DOS
int main(int argc, char *argv[])
#endif
{
    CHAR    MAC2_Valid;
    CHAR    MAC_1GEn;
    CHAR    MAC1_RMII;
    CHAR    Enable_IntLoopPHY;
    CHAR    Disable_RecovPHY;
    CHAR    Force1G;
    CHAR    Force10M;
    CHAR    Force100M;
    CHAR    *stop_at;
    ULONG   IOStr_val;
    ULONG   IOStr_max;
    ULONG   IOStr_shf;
    ULONG   IOdly_val;
    ULONG   Err_Flag_allapeed;    
    int     DES_LowNumber;
    int     index;
    int     i;
    int     j;
    #ifdef Enable_NCSI_LOOP_INFINI
        BYTE    GSpeed_org[3];
    #endif

#ifdef SPI_BUS
    VIDEO_ENGINE_INFO VideoEngineInfo;
#else
    // ( USE_P2A | USE_LPC )
    UCHAR   *ulMMIOLinearBaseAddress;
#endif

    #ifdef SLT_UBOOT
    #else
        time(&timestart);
    #endif

    // For DOS system
    #if defined(PHY_NCSI)
        // For DOS compiler OPEN WATCOM
        ModeSwitch = MODE_NSCI;
    #else
        #ifdef SLT_DOS
        ModeSwitch = MODE_DEDICATED;
        #endif
    #endif
    
//------------------------------------------------------------
// Argument Initial
//------------------------------------------------------------
    Err_Flag_allapeed = 0;
    Err_Flag          = 0;
    Err_Flag_PrintEn  = 1;
    Loop_rl[0]        = 0;
    Loop_rl[1]        = 0;
    Loop_rl[2]        = 0;

//------------------------------------------------------------
// Bus Initial
//------------------------------------------------------------
#if defined(LinuxAP)
#else   
  //DOS system
  #ifdef SPI_BUS
  #endif
  #ifdef USE_LPC

        if ( findlpcport( 0x0d ) == 0) {
            printf("Failed to find proper LPC port \n");
            
            return(1);
        }
        open_aspeed_sio_password();
        enable_aspeed_LDU( 0x0d );
  #endif
  #ifdef USE_P2A
        // PCI bus
        #ifdef DOS_PMODEW
        if (CheckDOS()) return 1;
        #endif

        #ifdef  DbgPrn_FuncHeader
            printf ("Initial-MMIO\n"); 
            Debug_delay();
        #endif
        ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x2000, ACTIVE);
        if ( ulPCIBaseAddress == 0 ) 
            ulPCIBaseAddress = FindPCIDevice (0x1688, 0x2000, ACTIVE);
        if ( ulPCIBaseAddress == 0 ) 
            ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x0200, ACTIVE);
        if ( ulPCIBaseAddress == 0 ) 
            ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x3000, ACTIVE);
        if ( ulPCIBaseAddress == 0 ) 
            ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x2010, ACTIVE);
        if ( ulPCIBaseAddress == 0 ) { 
            printf ("Can't find device\n"); 
            
            return(1);
        }

        WritePCIReg (ulPCIBaseAddress, 0x04, 0xFFFFFFFc, 0x3);
        ulMMIOBaseAddress       = ReadPCIReg (ulPCIBaseAddress, 0x14, 0xFFFF0000);
        ulMMIOLinearBaseAddress = (UCHAR *)MapPhysicalToLinear (ulMMIOBaseAddress, 64 * 1024 * 1024);
  #endif // #ifdef USE_P2A
#endif // End defined(LinuxAP)

#ifdef SPI_BUS
    GetDevicePCIInfo (&VideoEngineInfo);
    mmiobase = VideoEngineInfo.VGAPCIInfo.ulMMIOBaseAddress;
    spim_init(SPI_CS);
#else    
    // ( USE_P2A | USE_LPC )
    mmiobase = ulMMIOLinearBaseAddress;
#endif

//------------------------------------------------------------
// Check Chip Feature
//------------------------------------------------------------
    read_scu();

    if (RUN_STEP >= 1) {
        switch (SCU_7ch_old) {
//          case 0x02000003 : sprintf(ASTChipName, "[ ]AST3200-FPGA"                  ); ASTChipType = 6; AST1100 = 0; break;
            
            case 0x03020003 : sprintf(ASTChipName, "[ ]AST1010-A2"                    ); ASTChipType = 5; AST1100 = 0; break;
            case 0x03010003 : sprintf(ASTChipName, "[ ]AST1010-A1"                    ); ASTChipType = 5; AST1100 = 0; break;
            case 0x03000003 : sprintf(ASTChipName, "[*]AST1010-A0"                    ); ASTChipType = 5; AST1100 = 0; break;
            
            case 0x02010303 : sprintf(ASTChipName, "[*]AST2400-A1"                    ); ASTChipType = 4; AST1100 = 0; break;//AST2400-A1
            case 0x02000303 : sprintf(ASTChipName, "[ ]AST2400-A0"                    ); ASTChipType = 4; AST1100 = 0; break;//AST2400-A0
            case 0x02010103 : sprintf(ASTChipName, "[*]AST1400-A1"                    ); ASTChipType = 4; AST1100 = 0; break;//AST1400-A1
            case 0x02000003 : sprintf(ASTChipName, "[ ]AST1400-A0"                    ); ASTChipType = 4; AST1100 = 0; break;//AST1400-A0
            
            case 0x01010303 : sprintf(ASTChipName, "[*]AST2300-A1"                    ); ASTChipType = 3; AST1100 = 0; break;//AST2300-A1
            case 0x01010203 : sprintf(ASTChipName, "[*]AST1050-A1"                    ); ASTChipType = 3; AST1100 = 0; break;//AST1050-A1
            case 0x01010003 : sprintf(ASTChipName, "[*]AST1300-A1"                    ); ASTChipType = 3; AST1100 = 0; break;//AST1300-A1
            case 0x01000003 : sprintf(ASTChipName, "[ ]AST2300-A0"                    ); ASTChipType = 3; AST1100 = 0; break;//AST2300-A0
//          case 0x01860003 : sprintf(ASTChipName, "[ ]AST2300-FPGA"                  ); ASTChipType = 3; AST1100 = 0; break;
            
            case 0x00000102 : sprintf(ASTChipName, "[*]AST2200-A1"                    ); ASTChipType = 2; AST1100 = 0; break;//AST2200-A1/A0
            
            case 0x00000302 : sprintf(ASTChipName, "[*]AST2100-A3"                    ); ASTChipType = 1; AST1100 = 0; break;//AST2100-A3/A2
            case 0x00000301 : sprintf(ASTChipName, "[ ]AST2100-A1"                    ); ASTChipType = 1; AST1100 = 0; break;//AST2100-A1
            case 0x00000300 : sprintf(ASTChipName, "[ ]AST2100-A0"                    ); ASTChipType = 1; AST1100 = 0; break;//AST2100-A0
            case 0x00000202 : sprintf(ASTChipName, "[*]AST2050/AST1100-A3, AST2150-A1"); ASTChipType = 1; AST1100 = 1; break;//AST2050/AST1100-A3/A2 AST2150-A1/A0
            case 0x00000201 : sprintf(ASTChipName, "[ ]AST2050/AST1100-A1"            ); ASTChipType = 1; AST1100 = 1; break;//AST2050/AST1100-A1
            case 0x00000200 : sprintf(ASTChipName, "[ ]AST2050/AST1100-A0"            ); ASTChipType = 1; AST1100 = 1; break;//AST2050/AST1100-A0
            
            default     : 
                printf ("Error Silicon Revision ID(SCU7C) %08lx!!!\n", SCU_7ch_old); 
                return(1);
        } // End switch (SCU_7ch_old)
        
        switch (ASTChipType) {
            case 6  : AST2300 = 1; AST2400 = 1; AST1010 = 0; AST3200 = 1; break;
            case 5  : AST2300 = 1; AST2400 = 1; AST1010 = 1; AST3200 = 0; break;
            case 4  : AST2300 = 1; AST2400 = 1; AST1010 = 0; AST3200 = 0; break;
            case 3  : AST2300 = 1; AST2400 = 0; AST1010 = 0; AST3200 = 0; break;
            default : AST2300 = 0; AST2400 = 0; AST1010 = 0; AST3200 = 0; break;
        } // End switch (ASTChipType)

        if (ASTChipType == 3) {
#ifdef Force_Enable_MAC34
            WriteSOC_DD( SCU_BASE + 0xf0, 0xAEED0001 ); //enable mac34
            Enable_MAC34 = 1;
#else
            if (SCU_f0h_old & 0x00000001) 
                Enable_MAC34 = 1;
            else                          
                Enable_MAC34 = 0;
#endif
        } 
        else {
            Enable_MAC34 = 0;
        } // End if (ASTChipType == 3)

    Setting_scu();

//------------------------------------------------------------
// Argument Input
//------------------------------------------------------------
        // Load default value
        UserDVal         = DEF_USER_DEF_PACKET_VAL;
        IOTimingBund_arg = DEF_IOTIMINGBUND;
        PHY_ADR_arg      = DEF_PHY_ADR;
        TestMode         = DEF_TESTMODE;
        LOOP_INFINI      = 0;
        LOOP_MAX_arg     = 0;
        GCtrl            = ( DEF_MAC_LOOP_BACK << 6 ) | ( DEF_SKIP_CHECK_PHY << 5 ) | ( DEF_INIT_PHY << 3 );
        GSpeed           = DEF_SPEED;
        
        // Get setting information by user
        GRun_Mode        = (BYTE)atoi(argv[1]);

        if ( ModeSwitch == MODE_NSCI ) {
            ARPNumCnt        = DEF_ARPNUMCNT;
            ChannelTolNum    = DEF_CHANNEL2NUM;
            PackageTolNum    = DEF_PACKAGE2NUM;
            GSpeed           = SET_100MBPS;        // In NCSI mode, we set to 100M bps
        }
        
        // Setting user's configuration
        if (argc > 1) {
            if ( ModeSwitch == MODE_NSCI )
                switch (argc) {
                    case 7: ARPNumCnt        = (ULONG)atoi(argv[6]);
                    case 6: IOTimingBund_arg = (BYTE)atoi(argv[5]);
                    case 5: TestMode         = (BYTE)atoi(argv[4]);
                    case 4: ChannelTolNum    = (BYTE)atoi(argv[3]);
                    case 3: PackageTolNum    = (BYTE)atoi(argv[2]);
                    default: break;
                }
            else
                switch (argc) {
                    case 9: UserDVal         = strtoul (argv[8], &stop_at, 16);
                    case 8: IOTimingBund_arg = (BYTE)atoi(argv[7]);
                    case 7: PHY_ADR_arg      = (BYTE)atoi(argv[6]);
                    case 6: TestMode         = (BYTE)atoi(argv[5]);
                    case 5: strcpy(LOOP_Str, argv[4]);
                        if (!strcmp(LOOP_Str, "#")) LOOP_INFINI = 1;
                        else                        LOOP_MAX_arg = (ULONG)atoi(LOOP_Str);
                    case 4: GCtrl            = (BYTE)atoi(argv[3]);
                    case 3: GSpeed           = (BYTE)atoi(argv[2]);
                    default: break;
                }

            IOTimingBund = IOTimingBund_arg;
            PHY_ADR      = PHY_ADR_arg;
        } 
        else {
            // Wrong parameter
            if ( ModeSwitch == MODE_NSCI ) {
                if (AST2300) 
                    printf ("\nNCSITEST.exe  run_mode  <package_num>  <channel_num>  <test_mode>  <IO margin>\n\n");
                else         
                    printf ("\nNCSITEST.exe  run_mode  <package_num>  <channel_num>  <test_mode>\n\n");
                PrintMode ();
                PrintPakNUm();
                PrintChlNUm();
                PrintTest ();
                if (AST2300) 
                    PrintIOTimingBund ();
            } 
            else {
                if (AST2300) 
                    printf ("\nMACTEST.exe  run_mode  <speed>  <ctrl>  <loop_max>  <test_mode>  <phy_adr>  <IO margin>\n\n");
                else         
                    printf ("\nMACTEST.exe  run_mode  <speed>  <ctrl>  <loop_max>  <test_mode>  <phy_adr>\n\n");
                PrintMode ();
                PrintSpeed ();
                PrintCtrl ();
                PrintLoop ();
                PrintTest ();
                PrintPHYAdr ();
                if (AST2300) 
                    PrintIOTimingBund ();
            }
            Finish_Close();
            
            return(1);
        } // End if (argc > 1) 

//------------------------------------------------------------
// Check Argument
//------------------------------------------------------------
        switch ( GRun_Mode ) {
            case 0:                    printf ("\n[MAC1]\n"); SelectMAC = 0; H_MAC_BASE = MAC_BASE1; break;
            case 1:                    printf ("\n[MAC2]\n"); SelectMAC = 1; H_MAC_BASE = MAC_BASE2; break;
            case 2: if (Enable_MAC34) {printf ("\n[MAC3]\n"); SelectMAC = 2; H_MAC_BASE = MAC_BASE3; break;} 
                    else 
                        goto Error_MAC_Mode;
            case 3: if (Enable_MAC34) {printf ("\n[MAC4]\n"); SelectMAC = 3; H_MAC_BASE = MAC_BASE4; break;} 
                    else 
                        goto Error_MAC_Mode;
            default:
Error_MAC_Mode:     
                    printf ("Error run_mode!!!\n");
                    PrintMode ();
                    
                    return(1);
        } // End switch ( GRun_Mode )
        
        H_TDES_BASE   = TDES_BASE1;
        H_RDES_BASE   = RDES_BASE1;
        MAC_PHYBASE   = H_MAC_BASE;
        
        Force1G       = 0;
        Force10M      = 0;
        Force100M     = 0;
        GSpeed_sel[0] = 0;//1G
        GSpeed_sel[1] = 0;//100M
        GSpeed_sel[2] = 0;//10M
        
        switch ( GSpeed ) {
            case SET_1GBPS          : Force1G   = 1; GSpeed_sel[0] = 1; break;
            case SET_100MBPS        : Force100M = 1; GSpeed_sel[1] = 1; break;
            case SET_10MBPS         : Force10M  = 1; GSpeed_sel[2] = 1; break;
            case SET_1G_100M_10MBPS :                                   break;
            default: printf ("Error speed!!!\n");
                PrintSpeed ();
                return(1);
        } // End switch ( GSpeed )

        if ( ModeSwitch == MODE_NSCI ) {
            Enable_MACLoopback = 0; // For mactest function
            Enable_SkipChkPHY  = 0; // For mactest function
            Enable_IntLoopPHY  = 0; // For mactest function
            Enable_InitPHY     = 0; // For mactest function
            Disable_RecovPHY   = 0; // For mactest function
            BurstEnable        = 0; // For mactest function

            PrintNCSIEn = (ARPNumCnt & 0x1);
            ARPNumCnt   = ARPNumCnt & 0xfffffffe;
    
            // Check parameter
            if ((PackageTolNum < 1) || (PackageTolNum >  8)) { 
                PrintPakNUm();
                return(1);
            }
//            if ((ChannelTolNum < 0) || (ChannelTolNum > 32)) { 
            if (ChannelTolNum > 32) { 
                PrintChlNUm(); 
                return(1);
            }

            switch (TestMode) {
                case 0 :               NCSI_DiSChannel = 1; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 1; break;
                case 1 :               NCSI_DiSChannel = 0; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 1; break;
                case 6 : if (AST2300) {NCSI_DiSChannel = 1; IOTiming = 1; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 1; break;} 
                         else 
                            goto Error_Test_Mode_NCSI;
                case 7 : if (AST2300) {NCSI_DiSChannel = 1; IOTiming = 1; IOStrength = 1; TxDataEnable = 1; RxDataEnable = 1; break;} 
                         else 
                            goto Error_Test_Mode_NCSI;
                default:
                    // Wrong parameter
Error_Test_Mode_NCSI:       
                    printf ("Error test_mode!!!\n");
                    PrintTest ();
                    return(1);
            } // End switch (TestMode)
        }
        else {
            if ( GCtrl & 0xffffff83 ) {
                printf ("Error ctrl!!!\n");
                PrintCtrl ();
                return(1);
            } 
            else {
                Enable_MACLoopback = ( GCtrl >> 6 ) & 0x1; // ??
                Enable_SkipChkPHY  = ( GCtrl >> 5 ) & 0x1; // ??
                Enable_IntLoopPHY  = ( GCtrl >> 4 ) & 0x1;
                Enable_InitPHY     = ( GCtrl >> 3 ) & 0x1;
                Disable_RecovPHY   = ( GCtrl >> 2 ) & 0x1; // ??
            
                if (!AST2400 && Enable_MACLoopback) {
                    printf ("Error ctrl!!!\n");
                    PrintCtrl ();
                    return(1);
                }
            } // End if ( GCtrl & 0xffffff83 )
            
            if (!LOOP_MAX_arg) {
                switch (GSpeed) {
                    case SET_1GBPS         : LOOP_MAX_arg  = DEF_LOOP_MAX * 200; break; // 20140325
                    case SET_100MBPS       : LOOP_MAX_arg  = DEF_LOOP_MAX * 20 ; break; // 20140325
                    case SET_10MBPS        : LOOP_MAX_arg  = DEF_LOOP_MAX * 10 ; break; // 20140325
                    case SET_1G_100M_10MBPS: LOOP_MAX_arg  = DEF_LOOP_MAX * 10 ; break; // 20140325
                }
            } // End if (!LOOP_MAX_arg)
            
            LOOP_MAX = LOOP_MAX_arg * 10; // 20140325
            Calculate_LOOP_CheckNum();

            switch (TestMode) {
                case  0 :               BurstEnable = 0; IEEETesting = 0; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 1; DataDelay = 0; break;
                case  1 :               BurstEnable = 1; IEEETesting = 1; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 0; DataDelay = 0; break;
                case  2 :               BurstEnable = 1; IEEETesting = 1; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 0; DataDelay = 0; break;
                case  3 :               BurstEnable = 1; IEEETesting = 1; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 0; DataDelay = 0; break;
                case  4 :               BurstEnable = 1; IEEETesting = 0; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 0; DataDelay = 0; break; // ??
                case  5 :               BurstEnable = 1; IEEETesting = 1; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 0; DataDelay = 0; break; // ??
                case  6 : if (AST2300) {BurstEnable = 0; IEEETesting = 0; IOTiming = 1; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 1; DataDelay = 0; break;} 
                          else 
                            goto Error_Test_Mode;
                case  7 : if (AST2300) {BurstEnable = 0; IEEETesting = 0; IOTiming = 1; IOStrength = 1; TxDataEnable = 1; RxDataEnable = 1; DataDelay = 0; break;} 
                          else 
                            goto Error_Test_Mode;
                case  8 :               BurstEnable = 0; IEEETesting = 0; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 0; DataDelay = 0; break; // ??
                case  9 :               BurstEnable = 0; IEEETesting = 0; IOTiming = 0; IOStrength = 0; TxDataEnable = 0; RxDataEnable = 1; DataDelay = 0; break; // ??
                case 10 :               BurstEnable = 0; IEEETesting = 0; IOTiming = 0; IOStrength = 0; TxDataEnable = 1; RxDataEnable = 1; DataDelay = 1; break; // ??
                default:
Error_Test_Mode:        
                    printf ("Error test_mode!!!\n");
                    PrintTest ();
                    return(1);
            } // End switch (TestMode)

            if ( PHY_ADR > 31 ) {
                printf ("Error phy_adr!!!\n");
                PrintPHYAdr ();
                return(1);
            } // End if (PHY_ADR > 31)
        } // End if ( ModeSwitch == MODE_NSCI )

        if ( BurstEnable ) {
            IOTimingBund = 0;
        } 
        else {
            if ( ~DataDelay && AST2300 ) {
                if ( !( ( (7 >= IOTimingBund) && (IOTimingBund & 0x1) ) || 
                        ( IOTimingBund == 0 )                              ) ) {
                    printf ("Error IO margin!!!\n");
                    PrintIOTimingBund ();
                    return(1);
                }
            } 
            else {
                IOTimingBund = 0;
            } // End if ( ~DataDelay && AST2300 )
            
            // Define Output file name
            if ( ModeSwitch == MODE_NSCI )
                sprintf(FileNameMain, "%d", SelectMAC+1);
            else {
                if (Enable_IntLoopPHY) 
                    sprintf(FileNameMain, "%dI", SelectMAC+1);
                else                   
                    sprintf(FileNameMain, "%dE", SelectMAC+1);
            }
        
            #ifndef SLT_UBOOT
            if ( IOTiming ) {
                if ( IOStrength ) 
                    sprintf(FileName, "MIOD%sS.log", FileNameMain);
                else            
                    sprintf(FileName, "MIOD%s.log", FileNameMain);

                fp_log = fopen(FileName,"w");
        
                if ( IOStrength ) 
                    sprintf(FileName, "MIO%sS.log", FileNameMain);
                else            
                    sprintf(FileName, "MIO%s.log", FileNameMain);

                fp_io  = fopen(FileName,"w");
            } 
            else {
                sprintf(FileName, "MAC%s.log", FileNameMain);

                fp_log = fopen(FileName,"w");
            }
            #endif
        } // End if (BurstEnable)

//------------------------------------------------------------
// Check Definition
//------------------------------------------------------------
        for (i = 0; i < 16; i++) 
            valary[i] = Val_Array[i];
        
        if ( AST3200 ) {
            MAC_Mode   = (SCU_70h_old >> 6) & 0x1;
            MAC1_1GEn  = (MAC_Mode & 0x1) ? 1 : 0;//1:RGMII, 0:RMII
            MAC2_1GEn  = 0;
        
            MAC1_RMII  = !MAC1_1GEn;
            MAC2_RMII  = 0;
            MAC2_Valid = 0;
        } 
        else if ( AST1010 ) {
            MAC_Mode   = 0;
            MAC1_1GEn  = 0;
            MAC2_1GEn  = 0;
        
            MAC1_RMII  = 1;
            MAC2_RMII  = 0;
            MAC2_Valid = 0;
        } 
        else if ( AST2300 ) {
            if (SCU_7ch_old == 0x01000003) {
                //AST2300-A0
                for (i = 0; i < 16; i++) {
                    valary[i] = Val_Array_A0[i];
                }
            }
        
            MAC_Mode   = (SCU_70h_old >> 6) & 0x3;
            MAC1_1GEn  = (MAC_Mode & 0x1) ? 1 : 0;//1:RGMII, 0:RMII
            MAC2_1GEn  = (MAC_Mode & 0x2) ? 1 : 0;//1:RGMII, 0:RMII
        
            MAC1_RMII  = !MAC1_1GEn;
            MAC2_RMII  = !MAC2_1GEn;
            MAC2_Valid = 1;
        } 
        else {
            MAC_Mode   = (SCU_70h_old >> 6) & 0x7;
            MAC1_1GEn  = (MAC_Mode == 0x0) ? 1 : 0;
            MAC2_1GEn  = 0;
        
            switch ( MAC_Mode ) {
                case 0 : MAC1_RMII = 0; MAC2_RMII = 0; MAC2_Valid = 0; break; //000: Select GMII(MAC#1) only
                case 1 : MAC1_RMII = 0; MAC2_RMII = 0; MAC2_Valid = 1; break; //001: Select MII (MAC#1) and MII(MAC#2)
                case 2 : MAC1_RMII = 1; MAC2_RMII = 0; MAC2_Valid = 1; break; //010: Select RMII(MAC#1) and MII(MAC#2)
                case 3 : MAC1_RMII = 0; MAC2_RMII = 0; MAC2_Valid = 0; break; //011: Select MII (MAC#1) only
                case 4 : MAC1_RMII = 1; MAC2_RMII = 0; MAC2_Valid = 0; break; //100: Select RMII(MAC#1) only
//              case 5 : MAC1_RMII = 0; MAC2_RMII = 0; MAC2_Valid = 0; break; //101: Reserved
                case 6 : MAC1_RMII = 1; MAC2_RMII = 1; MAC2_Valid = 1; break; //110: Select RMII(MAC#1) and RMII(MAC#2)
//              case 7 : MAC1_RMII = 0; MAC2_RMII = 0; MAC2_Valid = 0; break; //111: Disable dual MAC
                default: return(Finish_Check(Err_MACMode));
            }
        } // End if ( AST3200 )
        
        if ( SelectMAC == 0 ) {
            Enable_RMII = MAC1_RMII;
            MAC_1GEn    = MAC1_1GEn;
        
            if ( Force1G & !MAC1_1GEn ) {
                printf ("\nMAC1 don't support 1Gbps !!!\n"); 
                return( Finish_Check(Err_MACMode) );
            }
        } else if (SelectMAC == 1) {
            Enable_RMII = MAC2_RMII;
            MAC_1GEn    = MAC2_1GEn;
        
            if ( Force1G & !MAC2_1GEn ) {
                printf ("\nMAC2 don't support 1Gbps !!!\n"); 
                return(Finish_Check(Err_MACMode));
            }
            if ( !MAC2_Valid ) {
                printf ("\nMAC2 not valid !!!\n"); 
                return(Finish_Check(Err_MACMode));
            }
        } 
        else {
            Enable_RMII = 1;
            MAC_1GEn = 0;
        
            if (Force1G) {
                printf ("\nMAC3/MAC4 don't support 1Gbps !!!\n"); 
                return(Finish_Check(Err_MACMode));
            }
        } // End if ( SelectMAC == 0 )

        if ( ModeSwitch == MODE_NSCI ) {
            if (!Enable_RMII) {
                printf ("\nNCSI must be RMII interface !!!\n");
                return(Finish_Check(Err_MACMode));
            }
        }

        if ( GSpeed == SET_1G_100M_10MBPS ) {
            GSpeed_sel[0] = MAC_1GEn;
            GSpeed_sel[1] = 1;
            GSpeed_sel[2] = 1;
        }

        if ( AST1010 ) {
            // Check bit 13:12
            Dat_ULONG = SCU_08h_old & 0x00003000;
            if (Dat_ULONG != 0x00000000)
                return(Finish_Check(Err_MHCLK_Ratio));
        } 
        else if ( AST2300 ) {
            Dat_ULONG = (SCU_08h_old >> 16) & 0x7;
            if (MAC1_1GEn | MAC2_1GEn) {
                if ( (Dat_ULONG == 0) || (Dat_ULONG > 2) )
                    return(Finish_Check(Err_MHCLK_Ratio));
            } 
            else {
                if (Dat_ULONG != 4)
                    return(Finish_Check(Err_MHCLK_Ratio));
            }
        } // End if (AST1010)

        //MAC
        MAC_08h_old = ReadSOC_DD( H_MAC_BASE + 0x08 );
        MAC_0ch_old = ReadSOC_DD( H_MAC_BASE + 0x0c );
        MAC_40h_old = ReadSOC_DD( H_MAC_BASE + 0x40 );
        
        if (  ((MAC_08h_old == 0x0000) && (MAC_0ch_old == 0x00000000))
           || ((MAC_08h_old == 0xffff) && (MAC_0ch_old == 0xffffffff))
//         || (MAC_0ch_old & 0x1)
//         || (MAC_0ch_old & 0x2)
           ) 
        {
            // Load default for MAC address
            SA[0] = 0x00;
            SA[1] = 0x57;
            SA[2] = 0x89;
            SA[3] = 0x56;
            SA[4] = 0x88;
            SA[5] = 0x38;
        } 
        else {
            SA[0] = (MAC_08h_old >>  8) & 0xff;
            SA[1] = (MAC_08h_old      ) & 0xff;
            SA[2] = (MAC_0ch_old >> 24) & 0xff;
            SA[3] = (MAC_0ch_old >> 16) & 0xff;
            SA[4] = (MAC_0ch_old >>  8) & 0xff;
            SA[5] = (MAC_0ch_old      ) & 0xff;
        }
        // printf ("%08x %08x: %02x %02x %02x %02x %02x %02x\n", MAC_08h_old, MAC_0ch_old, SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]);        

        if ( AST2300 ) {
#ifdef Force_Enable_NewMDIO
            AST2300_NewMDIO = 1;
            WriteSOC_DD(H_MAC_BASE+0x40, MAC_40h_old | 0x80000000)
#else
            AST2300_NewMDIO = (MAC_40h_old & 0x80000000) ? 1 : 0;
#endif
        } 
        else {
            AST2300_NewMDIO = 0;
        } // End if (AST2300)

//------------------------------------------------------------
// Parameter Initial
//------------------------------------------------------------
        if ( AST3200 ) {
            SCU_04h = 0x0c000800; //Reset Engine
        } 
        else if (AST1010) {
            SCU_04h = 0x00000010; //Reset Engine
        } 
        else if (AST2300) {
            SCU_04h = 0x0c001800; //Reset Engine
        } 
        else {
            SCU_04h = 0x00001800; //Reset Engine
        } // End if ( AST3200 )
    
        if ( ModeSwitch == MODE_NSCI )
            // Set to 100Mbps and Enable RX broabcast packets and CRC_APD and Full duplex
            MAC_50h = 0x000a0500;
        else {
            // RX_ALLADR and CRC_APD and Full duplex
            MAC_50h = 0x00004500;
            
            #ifdef Enable_Runt
            MAC_50h = MAC_50h | 0x00001000;
            #endif
            
            #ifdef Enable_Jumbo
            MAC_50h = MAC_50h | 0x00002000;
            #endif
        } // End if ( ModeSwitch == MODE_NSCI )

//------------------------------------------------------------
// Descriptor Number
//------------------------------------------------------------
        if ( ModeSwitch == MODE_DEDICATED ) {

            #ifdef Enable_Jumbo
            DES_LowNumber = 1;
            #else
            DES_LowNumber = IOTiming;
            #endif
            if ( Enable_SkipChkPHY && ( TestMode == 0 ) ) {
                DES_NUMBER = 114;//for SMSC's LAN9303 issue
            } 
            else {
                if ( AST1010 | AST3200 ) {
                    DES_NUMBER = (IOTimingBund) ? 100 : 256;
                } 
                else {
                    switch ( GSpeed ) {
                        case SET_1GBPS          : DES_NUMBER = (IOTimingBund) ? 10 : (DES_LowNumber) ? 50 : 400; break; // 20140325
                        case SET_100MBPS        : DES_NUMBER = (IOTimingBund) ? 10 : (DES_LowNumber) ? 50 : 400; break; // 20140325
                        case SET_10MBPS         : DES_NUMBER = (IOTimingBund) ? 10 : (DES_LowNumber) ? 10 :  80; break; // 20140325
                        case SET_1G_100M_10MBPS : DES_NUMBER = (IOTimingBund) ? 10 : (DES_LowNumber) ? 10 :  80; break; // 20140325
                    }
                } // End if ( Enable_SkipChkPHY && ( TestMode == 0 ) )
            } 

            #ifdef SelectDesNumber
            DES_NUMBER = SelectDesNumber;
            #endif
            
            #ifdef USE_LPC
                DES_NUMBER /= 8;
            #endif
        
            #ifdef ENABLE_ARP_2_WOL
            if ( TestMode == 4 ) {
                DES_NUMBER = 1;
            }
            #endif
            
            DES_NUMBER_Org = DES_NUMBER;
            
            if ( DbgPrn_Info ) {
                printf ("CheckBuf_MBSize : %ld\n", CheckBuf_MBSize);
                printf ("LOOP_CheckNum   : %ld\n", LOOP_CheckNum);
                printf ("DES_NUMBER      : %ld\n", DES_NUMBER);
                printf ("DMA_BufSize     : %ld bytes\n", DMA_BufSize);
                printf ("DMA_BufNum      : %d\n", DMA_BufNum);
                printf ("\n");
            }
//          if (3 > DMA_BufNum) 
//                return( Finish_Check(Err_DMABufNum) );
            
            if (2 > DMA_BufNum) 
                return( Finish_Check(Err_DMABufNum) );
        } // End if ( ModeSwitch == MODE_DEDICATED )
    } // End if (RUN_STEP >= 1)

//------------------------------------------------------------
// SCU Initial
//------------------------------------------------------------
    if ( RUN_STEP >= 2 ) {
        init_scu1();
    }
    
    if ( RUN_STEP >= 3 ) {
        init_scu_macrst();
    }

//------------------------------------------------------------
// Data Initial
//------------------------------------------------------------
    if (RUN_STEP >= 4) {
        setup_arp();
        if ( ModeSwitch ==  MODE_DEDICATED ) {

            FRAME_LEN = (ULONG *)malloc(DES_NUMBER * sizeof( ULONG ));
            wp_lst    = (ULONG *)malloc(DES_NUMBER * sizeof( ULONG ));
        
            if ( !FRAME_LEN ) 
                return( Finish_Check( Err_MALLOC_FrmSize ) );
                
            if ( !wp_lst ) 
                return( Finish_Check( Err_MALLOC_LastWP ));
                
            // Setup data and length
            TestingSetup();
        } // End if ( ModeSwitch ==  MODE_DEDICATED )
        
        // Get bit (shift) of IO driving strength register
        if ( IOStrength ) {
            if (AST1010) {
                IOStr_max = 1;//0~1
            } 
            else if (AST2400) {
                IOStr_max = 1;//0~1
                switch (SelectMAC) {
                    case 0  : IOStr_shf =  9; break;
                    case 1  : IOStr_shf = 11; break;
                }
            } 
            else {
                IOStr_max = 3;//0~3
                switch (SelectMAC) {
                    case 0  : IOStr_shf =  8; break;
                    case 1  : IOStr_shf = 10; break;
                    case 2  : IOStr_shf = 12; break;
                    case 3  : IOStr_shf = 14; break;
                }
            }
        } 
        else {
            IOStr_max = 0;
            IOStr_shf = 0;
        } // End if (IOStrength)

        // Get current clock delay value of TX(out) and RX(in) in the SCU48 register
        // and setting test range
        if ( Enable_RMII ) {
            switch (GRun_Mode) {
                case 0  : IOdly_out_shf = 24; IOdly_in_shf =  8; break;
                case 1  : IOdly_out_shf = 25; IOdly_in_shf = 12; break;
                case 2  : IOdly_out_shf = 26; IOdly_in_shf = 16; break;
                case 3  : IOdly_out_shf = 27; IOdly_in_shf = 20; break;
            }
            IOdly_in_reg  = (SCU_48h_old >> IOdly_in_shf ) & 0xf;
            IOdly_out_reg = (SCU_48h_old >> IOdly_out_shf) & 0x1;
        } 
        else {
            switch (GRun_Mode) {
                case 0  : IOdly_out_shf =  0; IOdly_in_shf  =  8; break;
                case 1  : IOdly_out_shf =  4; IOdly_in_shf  = 12; break;
            }
            IOdly_in_reg  = (SCU_48h_old >> IOdly_in_shf ) & 0xf;
            IOdly_out_reg = (SCU_48h_old >> IOdly_out_shf) & 0xf;
        } // End if ( Enable_RMII )
        
        // Find the coordinate in X-Y axis
        for ( index = 0; index <= 15; index++ ) 
            if ( IOdly_in_reg == valary[index] ) {
                IOdly_in_reg_idx = index; 
                break;
            }
        for ( index = 0; index <= 15; index++ ) 
            if ( IOdly_out_reg == valary[index] ) {
                IOdly_out_reg_idx = index; 
                break;
            }
        
        // Get the range for testmargin block
        if ( IOTiming ) {
            if ( Enable_RMII ) {
                IOdly_incval  = 1;
                IOdly_in_str  = 0;
                IOdly_in_end  = 15;
                IOdly_out_str = 0;
                IOdly_out_end = 1;
            } 
            else {
                IOdly_incval  = 1;
                IOdly_in_str  = 0;
                IOdly_in_end  = 15;
                IOdly_out_str = 0;
                IOdly_out_end = 15;
            }
        } 
        else if ( IOTimingBund ) {
            if ( Enable_RMII ) {
                IOdly_incval  = 1;
                IOdly_in_str  = IOdly_in_reg_idx  - ( IOTimingBund >> 1 );
                IOdly_in_end  = IOdly_in_reg_idx  + ( IOTimingBund >> 1 );
                IOdly_out_str = IOdly_out_reg_idx;
                IOdly_out_end = IOdly_out_reg_idx;
            } 
            else {
                IOdly_incval  = 1;
                IOdly_in_str  = IOdly_in_reg_idx  - ( IOTimingBund >> 1 );
                IOdly_in_end  = IOdly_in_reg_idx  + ( IOTimingBund >> 1 );
                IOdly_out_str = IOdly_out_reg_idx - ( IOTimingBund >> 1 );
                IOdly_out_end = IOdly_out_reg_idx + ( IOTimingBund >> 1 );
            }
            if ((IOdly_in_str  < 0) || (IOdly_in_end  > 15)) 
                return( Finish_Check( Err_IOMarginOUF ) );
                
            if ((IOdly_out_str < 0) || (IOdly_out_end > 15)) 
                return( Finish_Check( Err_IOMarginOUF ) );

//          if (IOdly_in_str  <  0) IOdly_in_str  =  0;
//          if (IOdly_in_end  > 15) IOdly_in_end  = 15;
//          if (IOdly_out_str <  0) IOdly_out_str =  0;
//          if (IOdly_out_end > 15) IOdly_out_end = 15;
        } 
        else {
            IOdly_incval  = 1;
            IOdly_in_str  = 0;
            IOdly_in_end  = 0;
            IOdly_out_str = 0;
            IOdly_out_end = 0;
        } // End if (IOTiming)
    } // End if (RUN_STEP >= 4)

//------------------------------------------------------------
// main
//------------------------------------------------------------
    if (RUN_STEP >= 5) {
        #ifdef  DbgPrn_FuncHeader
            printf ("GSpeed_sel: %d %d %d\n", GSpeed_sel[0], GSpeed_sel[1], GSpeed_sel[2]); 
            Debug_delay();
        #endif

    if ( ModeSwitch == MODE_NSCI ) {
        #ifdef Enable_NCSI_LOOP_INFINI
        for ( GSpeed_idx = 0; GSpeed_idx < 3; GSpeed_idx++ ) {
            GSpeed_org[GSpeed_idx] = GSpeed_sel[GSpeed_idx];
        }
NCSI_LOOP_INFINI:;
        for ( GSpeed_idx = 0; GSpeed_idx < 3; GSpeed_idx++ ) {
            GSpeed_sel[GSpeed_idx] = GSpeed_org[GSpeed_idx];
        }
        #endif
    } // End if ( ModeSwitch == MODE_NSCI )

        for (GSpeed_idx = 0; GSpeed_idx < 3; GSpeed_idx++) {
            Err_Flag_PrintEn = 1;
            if ( GSpeed_sel[GSpeed_idx] ) {
                // Setting the LAN speed
                if ( ModeSwitch ==  MODE_DEDICATED ) {


                    // Test three speed of LAN, we will modify loop number
                    if (GSpeed == SET_1G_100M_10MBPS) {
                        if      (GSpeed_sel[0]) LOOP_MAX = LOOP_MAX_arg;
                        else if (GSpeed_sel[1]) LOOP_MAX = LOOP_MAX_arg / 10;
                        else                    LOOP_MAX = LOOP_MAX_arg / 100;
                            
                        if ( !LOOP_MAX ) 
                            LOOP_MAX = 1;
                        
                        Calculate_LOOP_CheckNum();
                    }
                    
                    // Setting speed of LAN
                    if      (GSpeed_sel[0]) MAC_50h_Speed = 0x00000200;
                    else if (GSpeed_sel[1]) MAC_50h_Speed = 0x00080000;
                    else                    MAC_50h_Speed = 0x00000000;
                        
                    //------------------------------------------------------------
                    // PHY Initial
                    //------------------------------------------------------------
                    if ( AST1100 ) 
                        init_scu2();
                        
                    if ( Enable_InitPHY ) {
#ifdef SUPPORT_PHY_LAN9303
                        LAN9303(LAN9303_I2C_BUSNUM, PHY_ADR_arg, GSpeed_idx, Enable_IntLoopPHY | (BurstEnable<<1) | IEEETesting);
#else
                        init_phy( Enable_IntLoopPHY );
#endif
                        DELAY( Delay_PHYRst * 10 );
                    } // End if (Enable_InitPHY)

                    if ( AST1100 ) 
                        init_scu3();

                    if ( Err_Flag ) 
                        return( Finish_Check( 0 ) );
                } // End if ( ModeSwitch ==  MODE_DEDICATED )

                //------------------------------------------------------------
                // Start
                //------------------------------------------------------------
                
                // The loop is for different IO strength
                for ( IOStr_i = 0; IOStr_i <= IOStr_max; IOStr_i++ ) {
                    
                    // Print Header of report to monitor and log file
                    if ( IOTiming || IOTimingBund ) {
                        if ( IOStrength ) {
                            if ( AST1010 ) {
                                IOStr_val = (SCU_ach_old & 0xfff0ffff) | ((IOStr_i) ? 0xf0000 : 0x0);
                            } 
                            else {
                                IOStr_val = (SCU_90h_old & 0xffff00ff) | (IOStr_i << IOStr_shf);
                            }
                            //printf("\nIOStrength_val= %08x, ", IOStr_val);
                            //printf("SCU90h: %08x ->", ReadSOC_DD(SCU_BASE+0x90));
                            WriteSOC_DD( SCU_BASE + 0x90, IOStr_val );
                            //printf(" %08x\n", ReadSOC_DD(SCU_BASE+0x90));
                            
                            #ifndef SLT_UBOOT
                            if      (GSpeed_sel[0]) fprintf(fp_log, "[Strength %d][1G  ]========================================\n", IOStr_i);
                            else if (GSpeed_sel[1]) fprintf(fp_log, "[Strength %d][100M]========================================\n", IOStr_i);
                            else                    fprintf(fp_log, "[Strength %d][10M ]========================================\n", IOStr_i);
                            #endif
                        } 
                        else {
                            #ifndef SLT_UBOOT
                            if      (GSpeed_sel[0]) fprintf(fp_log, "[1G  ]========================================\n");
                            else if (GSpeed_sel[1]) fprintf(fp_log, "[100M]========================================\n");
                            else                    fprintf(fp_log, "[10M ]========================================\n");
                            #endif
                        }

                        if ( IOTimingBund ) 
                            PrintIO_Header(FP_LOG);
                        if ( IOTiming     ) 
                            PrintIO_Header(FP_IO);

                        PrintIO_Header(STD_OUT);
                        
                    } 
                    else {
                        if ( ModeSwitch == MODE_DEDICATED ) {

                            if (!BurstEnable) 
                                Print_Header(FP_LOG);

                            Print_Header(STD_OUT);
                        }
                    } // End if (IOTiming || IOTimingBund)
                    
#ifdef Enable_Old_Style
                    for (IOdly_i = IOdly_in_str; IOdly_i <= IOdly_in_end; IOdly_i+=IOdly_incval) {
                        IOdly_in  = valary[IOdly_i];
#else
                    for (IOdly_j = IOdly_out_str; IOdly_j <= IOdly_out_end; IOdly_j+=IOdly_incval) {
                        IOdly_out = valary[IOdly_j];
#endif

                        if (IOTiming || IOTimingBund) {
#ifdef Enable_Fast_SCU
  #ifdef Enable_Old_Style
                            WriteSOC_DD(SCU_BASE + 0x48, SCU_48h_mix | (IOdly_in << IOdly_in_shf));
  #else
                            WriteSOC_DD(SCU_BASE + 0x48, SCU_48h_mix | (IOdly_out << IOdly_out_shf));
  #endif
#endif

                            if ( IOTimingBund ) 
                                PrintIO_LineS(FP_LOG);
                            if ( IOTiming     ) 
                                PrintIO_LineS(FP_IO);

                            PrintIO_LineS(STD_OUT);
                        } // End if (IOTiming || IOTimingBund)
                        
                        //------------------------------------------------------------
                        // SCU Initial
                        //------------------------------------------------------------
#ifdef Enable_Fast_SCU
                        init_scu_macrst();
#endif
#ifdef Enable_Old_Style
                        for (IOdly_j = IOdly_out_str; IOdly_j <= IOdly_out_end; IOdly_j+=IOdly_incval) {
                            IOdly_out = valary[IOdly_j];
#else
                        for (IOdly_i = IOdly_in_str; IOdly_i <= IOdly_in_end; IOdly_i+=IOdly_incval) {
                            IOdly_in  = valary[IOdly_i];
#endif
                            if ( IOTiming || IOTimingBund ) {
                                IOdly_val = (IOdly_in  << IOdly_in_shf) | (IOdly_out << IOdly_out_shf);

//printf("\nIOdly_val= %08x, ", IOdly_val);
//printf("SCU48h: %08x ->", ReadSOC_DD( SCU_BASE + 0x48 ) );
                            WriteSOC_DD( SCU_BASE + 0x48, SCU_48h_mix | IOdly_val );
//printf(" %08x\n", ReadSOC_DD(SCU_BASE+0x48));
                            } // End if (IOTiming || IOTimingBund)
                        
                            //------------------------------------------------------------
                            // SCU Initial
                            //------------------------------------------------------------
#ifdef Enable_Fast_SCU
#else
                            init_scu_macrst();
#endif

                            //------------------------------------------------------------
                            // MAC Initial
                            //------------------------------------------------------------
                            init_mac(H_MAC_BASE, H_TDES_BASE, H_RDES_BASE);
                            if ( Err_Flag ) 
                                return( Finish_Check(0) );

                            // Testing 
                            if ( ModeSwitch == MODE_NSCI )
                                dlymap[IOdly_i][IOdly_j] = phy_ncsi();
                            else
                                dlymap[IOdly_i][IOdly_j] = TestingLoop(LOOP_CheckNum);
                            
                            
                            // Display to Log file and monitor
                            if ( IOTiming || IOTimingBund ) {

                                if ( IOTimingBund ) 
                                    PrintIO_Line(FP_LOG);
                                    
                                if ( IOTiming     ) 
                                    PrintIO_Line(FP_IO);

                                PrintIO_Line(STD_OUT);

                                // Find the range of current setting 
                                if ( ( IOdly_in_reg == IOdly_in ) && ( IOdly_out_reg == IOdly_out ) ) {
                                    IOdly_i_min = IOdly_i - ( IOTimingBund >> 1 );
                                    IOdly_i_max = IOdly_i + ( IOTimingBund >> 1 );
                                
                                    if ( Enable_RMII ) {
                                        IOdly_j_min = IOdly_j;
                                        IOdly_j_max = IOdly_j;
                                    } 
                                    else {
                                        IOdly_j_min = IOdly_j - (IOTimingBund >> 1 );
                                        IOdly_j_max = IOdly_j + (IOTimingBund >> 1 );
                                    }
                                }
                                
                                PrintIO_Line_LOG();
                                FPri_ErrFlag(FP_LOG);

//                              Err_Flag_allapeed = Err_Flag_allapeed | Err_Flag;
                                Err_Flag = 0;
                            }
                        }// End for (IOdly_j = IOdly_out_str; IOdly_j <= IOdly_out_end; IOdly_j+=IOdly_incval)
#ifndef SLT_UBOOT                        
                        if ( IOTiming || IOTimingBund ) {
                            if ( IOTimingBund ) 
                                fprintf(fp_log, "\n");
                            if (IOTiming    ) 
                                fprintf(fp_io, "\n");
                        }
#endif
                        printf("\n");
                    } // End for (IOdly_j = IOdly_out_str; IOdly_j <= IOdly_out_end; IOdly_j+=IOdly_incval)
                    
                    //------------------------------------------------------------
                    // End
                    //------------------------------------------------------------

                    if ( IOTiming || IOTimingBund ) {
                        if ( ( IOdly_i_min < 0 ) || ( IOdly_i_max > 15 ) ) 
                            FindErr(Err_IOMarginOUF);
                        if ( ( IOdly_j_min < 0 ) || ( IOdly_j_max > 15 ) ) 
                            FindErr(Err_IOMarginOUF);

                        if ( IOdly_i_min <  0 ) IOdly_i_min =  0;
                        if ( IOdly_i_max > 15 ) IOdly_i_max = 15;
                        if ( IOdly_j_min <  0 ) IOdly_j_min =  0;
                        if ( IOdly_j_max > 15 ) IOdly_j_max = 15;

#ifdef Enable_Old_Style
                        for (IOdly_i = IOdly_i_min; IOdly_i <= IOdly_i_max; IOdly_i++)
                            for (IOdly_j = IOdly_j_min; IOdly_j <= IOdly_j_max; IOdly_j++)
#else
                        for (IOdly_j = IOdly_j_min; IOdly_j <= IOdly_j_max; IOdly_j++)
                            for (IOdly_i = IOdly_i_min; IOdly_i <= IOdly_i_max; IOdly_i++)
#endif
                            {             
                                if ( dlymap[IOdly_i][IOdly_j] ) {
#ifdef SLT_DOS                   
                                    if ( IOTiming ) {
#ifdef Enable_Old_Style
                                        for (i = IOdly_i_min; i <= IOdly_i_max; i++)
#else
                                        for (j = IOdly_j_min; j <= IOdly_j_max; j++)
#endif
                                        {
#ifdef Enable_Old_Style
                                            for (j = IOdly_j_min; j <= IOdly_j_max; j++)
#else
                                            for (i = IOdly_i_min; i <= IOdly_i_max; i++)
#endif
                                            {    
                                                if (dlymap[i][j]) fprintf(fp_io, "x ");
                                                else              fprintf(fp_io, "o ");
                                            }
                                            fprintf(fp_io, "\n");
                                        }
                                    } // End if ( IOTiming )
#endif // End SLT_DOS  
                                    FindErr(Err_IOMargin);
                                    goto Find_Err_IOMargin;
                                } // End if ( dlymap[IOdly_i][IOdly_j] )
                            }
                    } // End if ( IOTiming || IOTimingBund )

Find_Err_IOMargin:;
                    if ( !BurstEnable ) 
                        FPri_ErrFlag(FP_LOG);
                    if ( IOTiming ) 
                        FPri_ErrFlag(FP_IO);

                    FPri_ErrFlag(STD_OUT);
                    
                    Err_Flag_allapeed = Err_Flag_allapeed | Err_Flag;
                    Err_Flag = 0;
                } // End for (IOStr_i = 0; IOStr_i <= IOStr_max; IOStr_i++)
                
                if ( ModeSwitch == MODE_DEDICATED ) {
                    if ( Enable_InitPHY & !Disable_RecovPHY ) 
                        recov_phy(Enable_IntLoopPHY);
                }
                    
                GSpeed_sel[GSpeed_idx] = 0;
            } // End if (GSpeed_sel[GSpeed_idx])
            
            Err_Flag_PrintEn = 0;
        } // End for (GSpeed_idx = 0; GSpeed_idx < 3; GSpeed_idx++)
        
        Err_Flag = Err_Flag_allapeed;

        if ( ModeSwitch == MODE_NSCI ) {
            #ifdef Enable_NCSI_LOOP_INFINI
            if (Err_Flag == 0) {
                if (fp_log) {
                    fclose(fp_log);
                    fp_log = fopen(FileName,"w");
                }
                goto NCSI_LOOP_INFINI;
            }
            #endif
        }
        
    } // End if (RUN_STEP >= 5)

    return(Finish_Check(0));
   
}
