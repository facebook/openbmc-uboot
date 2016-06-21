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
static const char ThisFile[] = "MACTEST.c";

#include "SWFUNC.H"

#if defined(SLT_UBOOT)
  #include <common.h>
  #include <command.h>
  #include <post.h>
  #include <malloc.h>
  #include <net.h>
  #include "COMMINF.H"
  #include "STDUBOOT.H"
  #include "IO.H"
#else
  #include <stdlib.h>
  #include <string.h>
  #include "LIB.H"
  #include "COMMINF.H"
  #include "IO.H"
#endif

#if	defined(SLT_NEW_ARCH)
  #include "mactest.h"
#endif

#if	defined(LinuxAP) || defined(SLT_NEW_ARCH)
int MACTest(_MACInfo *MACInfo)
#endif
#if defined(SLT_UBOOT)
int main_function(int argc, char *argv[], char mode)
#endif
#if defined(DOS_ALONE)
int main(int argc, char *argv[])
#endif
{
#if defined(SPI_BUS)
	VIDEO_ENGINE_INFO    VideoEngineInfo;
#endif
#if defined(USE_P2A)
	UCHAR                *ulMMIOLinearBaseAddress;
#endif

#if defined(SLT_UBOOT)
#elif defined(LinuxAP) || defined(SLT_NEW_ARCH)
	int                  argc;
	char                 **argv;
#else
#endif
	MAC_ENGINE           MACENG;
	MAC_ENGINE           *eng;
	PHY_ENGINE           PHYENG;
	PHY_ENGINE           *phyeng;
#if defined(ENABLE_LOG_FILE)
	CHAR                 FileNameMain[256];
	CHAR                 FileName[256];
#endif
	int                  DES_LowNumber;

	CHAR                 *stop_at;
	ULONG                Wrn_Flag_allapeed;
	ULONG                Err_Flag_allapeed;
	ULONG                Des_Flag_allapeed;
	ULONG                NCSI_Flag_allapeed;

	int                  i;
	int                  j;
	ULONG                temp;

//------------------------------------------------------------
// main Start
//------------------------------------------------------------
	eng    = &MACENG;
	phyeng = &PHYENG;
	phyeng->fp_set = 0;
	phyeng->fp_clr = 0;
#if defined(PHY_SPECIAL)
	special_PHY_init( eng );
#endif

#if defined(SLT_UBOOT)
	eng->ModeSwitch = mode;
#endif

#if defined(DOS_ALONE)
  #if defined(PHY_NCSI)
	// For DOS compiler OPEN WATCOM
	eng->ModeSwitch = MODE_NSCI;
  #else
	eng->ModeSwitch = MODE_DEDICATED;
  #endif
#endif

#if defined(DOS_ALONE) || defined(SLT_NEW_ARCH)
	// DOS system
	time( &(eng->timestart) );
#endif

//------------------------------------------------------------
// Bus Initial
//------------------------------------------------------------

#if defined(LinuxAP) || defined(SLT_NEW_ARCH)
	if ( MACInfo->MACInterface == MDC_INTERFACE )
		eng->ModeSwitch = MODE_DEDICATED;
	else
		eng->ModeSwitch = MODE_NSCI;

  #if defined(USE_P2A)
	ulMMIOBaseAddress = MACInfo->ulMMIOBaseAddress;
	ulMMIOLinearBaseAddress = (UCHAR *) (MACInfo->ulMMIOLinear);
  #endif
  #if defined(USE_LPC)
	// LPC interface
	SetLPCport( MACInfo->LPC_port );
  #endif

	argc = (int)MACInfo->argc;
	argv = MACInfo->argv;
#endif // End ( defined(LinuxAP) || defined(SLT_NEW_ARCH) )

#ifdef DOS_ALONE
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
	if (CheckDOS()) return (1);
    #endif

    #ifdef  DbgPrn_FuncHeader
	printf("Initial-MMIO\n");
	Debug_delay();
    #endif
	ulPCIBaseAddress = FindPCIDevice ( 0x1A03, 0x2000, ACTIVE );
	if ( ulPCIBaseAddress == 0 )
		ulPCIBaseAddress = FindPCIDevice ( 0x1688, 0x2000, ACTIVE );
	if ( ulPCIBaseAddress == 0 )
		ulPCIBaseAddress = FindPCIDevice ( 0x1A03, 0x0200, ACTIVE );
	if ( ulPCIBaseAddress == 0 )
		ulPCIBaseAddress = FindPCIDevice ( 0x1A03, 0x3000, ACTIVE );
	if ( ulPCIBaseAddress == 0 )
		ulPCIBaseAddress = FindPCIDevice ( 0x1A03, 0x2010, ACTIVE );
	if ( ulPCIBaseAddress == 0 ) {
		printf("Can't find device\n");

		return(1);
	}

	WritePCIReg (ulPCIBaseAddress, 0x04, 0xFFFFFFFc, 0x3);
	ulMMIOBaseAddress       = ReadPCIReg ( ulPCIBaseAddress, 0x14, 0xFFFF0000 );
	ulMMIOLinearBaseAddress = (UCHAR *)MapPhysicalToLinear ( ulMMIOBaseAddress, 64 * 1024 * 1024 );
  #endif // #ifdef USE_P2A
#endif // End DOS_ALONE

#ifdef SPI_BUS
	GetDevicePCIInfo ( &VideoEngineInfo );
	mmiobase = VideoEngineInfo.VGAPCIInfo.ulMMIOBaseAddress;
	spim_init( SPI_CS );
#endif
#if defined(USE_P2A)
	mmiobase = ulMMIOLinearBaseAddress;
#endif

#if defined(DOS_ALONE) || defined(SLT_NEW_ARCH) || defined(LinuxAP)
	init_hwtimer();
#endif

//------------------------------------------------------------
// Get Chip Feature
//------------------------------------------------------------
	Wrn_Flag_allapeed  = 0;
	Err_Flag_allapeed  = 0;
	Des_Flag_allapeed  = 0;
	NCSI_Flag_allapeed = 0;
	eng->flg.Wrn_Flag              = 0;
	eng->flg.Err_Flag              = 0;
	eng->flg.Des_Flag              = 0;
	eng->flg.NCSI_Flag             = 0;
	eng->flg.Flag_PrintEn          = 1;
	eng->run.TIME_OUT_Des_PHYRatio = 1;
	eng->run.Loop_ofcnt            = 0;
	eng->run.Loop                  = 0;
	eng->run.Loop_rl[0]            = 0;
	eng->run.Loop_rl[1]            = 0;
	eng->run.Loop_rl[2]            = 0;
	eng->dat.FRAME_LEN             = 0;
	eng->dat.wp_lst                = 0;
	eng->io.init_done              = 0;
	eng->env.VGAModeVld            = 0;
	eng->reg.SCU_oldvld            = 0;
	eng->phy.Adr                   = 0;
	eng->phy.loop_phy              = 0;
	eng->phy.PHY_ID2               = 0;
	eng->phy.PHY_ID3               = 0;
	eng->phy.PHYName[0]            = 0;
	eng->ncsi_cap.PCI_DID_VID      = 0;
	eng->ncsi_cap.ManufacturerID   = 0;
	read_scu( eng );

	if ( RUN_STEP >= 1 ) {
		//------------------------------
		// [Reg]check SCU_07c
		// [Env]setup ASTChipName
		// [Env]setup ASTChipType
		//------------------------------
		switch ( eng->reg.SCU_07c ) {
#ifdef AST2500_IOMAP
#elif defined(AST1010_CHIP)
			case 0x03010003 : sprintf( eng->env.ASTChipName, "[*]AST1010-A2/AST1010-A1"         ); eng->env.ASTChipType = 6; break;//AST1010-A2/A1
			case 0x03000003 : sprintf( eng->env.ASTChipName, "[ ]AST1010-A0"                    ); eng->env.ASTChipType = 6; break;//AST1010-A0
#else
			case 0x02010303 : sprintf( eng->env.ASTChipName, "[*]AST2400-A1"                    ); eng->env.ASTChipType = 5; break;//AST2400-A1
			case 0x02000303 : sprintf( eng->env.ASTChipName, "[ ]AST2400-A0"                    ); eng->env.ASTChipType = 5; break;//AST2400-A0
			case 0x02010103 : sprintf( eng->env.ASTChipName, "[*]AST1400-A1"                    ); eng->env.ASTChipType = 5; break;//AST1400-A1
			case 0x02000003 : sprintf( eng->env.ASTChipName, "[ ]AST1400-A0"                    ); eng->env.ASTChipType = 5; break;//AST1400-A0

			case 0x01010303 : sprintf( eng->env.ASTChipName, "[*]AST2300-A1"                    ); eng->env.ASTChipType = 4; break;//AST2300-A1
			case 0x01010203 : sprintf( eng->env.ASTChipName, "[*]AST1050-A1"                    ); eng->env.ASTChipType = 4; break;//AST1050-A1
			case 0x01010003 : sprintf( eng->env.ASTChipName, "[*]AST1300-A1"                    ); eng->env.ASTChipType = 4; break;//AST1300-A1
			case 0x01000003 : sprintf( eng->env.ASTChipName, "[ ]AST2300-A0"                    ); eng->env.ASTChipType = 4; break;//AST2300-A0

			case 0x00000102 : sprintf( eng->env.ASTChipName, "[*]AST2200-A1"                    ); eng->env.ASTChipType = 3; break;//AST2200-A1/A0

			case 0x00000302 : sprintf( eng->env.ASTChipName, "[*]AST2100-A3"                    ); eng->env.ASTChipType = 2; break;//AST2100-A3/A2
			case 0x00000301 : sprintf( eng->env.ASTChipName, "[ ]AST2100-A1"                    ); eng->env.ASTChipType = 2; break;//AST2100-A1
			case 0x00000300 : sprintf( eng->env.ASTChipName, "[ ]AST2100-A0"                    ); eng->env.ASTChipType = 2; break;//AST2100-A0
			case 0x00000202 : sprintf( eng->env.ASTChipName, "[*]AST2050/AST1100-A3, AST2150-A1"); eng->env.ASTChipType = 1; break;//AST2050/AST1100-A3/A2 AST2150-A1/A0
			case 0x00000201 : sprintf( eng->env.ASTChipName, "[ ]AST2050/AST1100-A1"            ); eng->env.ASTChipType = 1; break;//AST2050/AST1100-A1
			case 0x00000200 : sprintf( eng->env.ASTChipName, "[ ]AST2050/AST1100-A0"            ); eng->env.ASTChipType = 1; break;//AST2050/AST1100-A0
#endif
			default:
				sprintf( eng->env.ASTChipName, "[ ]" );
				temp = ( eng->reg.SCU_07c ) & 0xff000000;
				switch ( temp ) {
#ifdef AST2500_IOMAP
					case 0x04000000 : eng->env.ASTChipType = 8; goto PASS_CHIP_ID;
#elif defined(AST1010_CHIP)
					case 0x03000000 : eng->env.ASTChipType = 6; goto PASS_CHIP_ID;
#else
					case 0x02000000 : eng->env.ASTChipType = 5; goto PASS_CHIP_ID;
					case 0x01000000 : eng->env.ASTChipType = 4; goto PASS_CHIP_ID;
#endif
					default:
						printf("Error Silicon Revision ID(SCU7C) %08lx [%08lx]!!!\n", eng->reg.SCU_07c, temp);
#if defined(AST1010_CHIP)
						printf("Only support AST1010\n");
#endif
				}
				return(1);
		} // End switch ( eng->reg.SCU_07c )
PASS_CHIP_ID:
		//------------------------------
		// [Env]check ASTChipType
		// [Env]setup AST1100
		// [Env]setup AST2300
		// [Env]setup AST2400
		// [Env]setup AST1010
		// [Env]setup AST2500
		// [Env]setup AST2500A1
		//------------------------------
		eng->env.AST1100   = 0;
		eng->env.AST2300   = 0;
		eng->env.AST2400   = 0;
		eng->env.AST1010   = 0;
		eng->env.AST2500   = 0;
		eng->env.AST2500A1 = 0;
		switch ( eng->env.ASTChipType ) {
			case 8  : eng->env.AST2500A1 = 1;
			case 7  : eng->env.AST2500   = 1;
			case 5  : eng->env.AST2400   = 1;
			case 4  : eng->env.AST2300   = 1; break;

			case 6  : eng->env.AST2300 = 1; eng->env.AST2400 = 1; eng->env.AST1010 = 1; break;
			case 1  : eng->env.AST1100 = 1; break;
			default : break;
		} // End switch ( eng->env.ASTChipType )

		//------------------------------
		// [Env]check ASTChipType
		// [Reg]check SCU_0f0
		// [Env]setup MAC34_vld
		//------------------------------
		if ( ( eng->env.ASTChipType == 4 ) && ( eng->reg.SCU_0f0 & 0x00000001 ) )//only AST2300
			eng->env.MAC34_vld = 1;
		else
			eng->env.MAC34_vld = 0;

//------------------------------------------------------------
// Get Argument Input
//------------------------------------------------------------
		//------------------------------
		// Load default value
		//------------------------------
		if ( eng->ModeSwitch == MODE_NSCI ) {
			eng->arg.GARPNumCnt     = DEF_GARPNUMCNT;
			eng->arg.GChannelTolNum = DEF_GCHANNEL2NUM;
			eng->arg.GPackageTolNum = DEF_GPACKAGE2NUM;
			eng->arg.GCtrl          = 0;
			eng->arg.GSpeed         = SET_100MBPS;        // In NCSI mode, we set to 100M bps
		}
		else {
			eng->arg.GUserDVal    = DEF_GUSER_DEF_PACKET_VAL;
			eng->arg.GPHYADR      = DEF_GPHY_ADR;
			eng->arg.GLOOP_INFINI = 0;
			eng->arg.GLOOP_MAX    = 0;
			eng->arg.GCtrl        = DEF_GCTRL;
			eng->arg.GSpeed       = DEF_GSPEED;
		}
		eng->arg.GChk_TimingBund = DEF_GIOTIMINGBUND;
		eng->arg.GTestMode       = DEF_GTESTMODE;

		//------------------------------
		// Get setting information by user
		//------------------------------
		eng->arg.GRun_Mode = (BYTE)atoi(argv[1]);
		if (argc > 1) {
			if ( eng->ModeSwitch == MODE_NSCI ) {
				switch ( argc ) {
					case 7: eng->arg.GARPNumCnt      = (ULONG)atoi(argv[6]);
					case 6: eng->arg.GChk_TimingBund = (BYTE)atoi(argv[5]);
					case 5: eng->arg.GTestMode       = (BYTE)atoi(argv[4]);
					case 4: eng->arg.GChannelTolNum  = (BYTE)atoi(argv[3]);
					case 3: eng->arg.GPackageTolNum  = (BYTE)atoi(argv[2]);
					default: break;
				}
			}
			else {
				switch ( argc ) {
					case 9: eng->arg.GUserDVal       = strtoul (argv[8], &stop_at, 16);
					case 8: eng->arg.GChk_TimingBund = (BYTE)atoi(argv[7]);
					case 7: eng->arg.GPHYADR         = (BYTE)atoi(argv[6]);
					case 6: eng->arg.GTestMode       = (BYTE)atoi(argv[5]);
					case 5: strcpy( eng->arg.GLOOP_Str, argv[4] );
						if ( !strcmp( eng->arg.GLOOP_Str, "#" ) )
							eng->arg.GLOOP_INFINI = 1;
						else
							eng->arg.GLOOP_MAX    = (ULONG)atoi( eng->arg.GLOOP_Str );
					case 4: eng->arg.GCtrl           = (BYTE)atoi(argv[3]);
					case 3: eng->arg.GSpeed          = (BYTE)atoi(argv[2]);
					default: break;
				}
			} // End if ( eng->ModeSwitch == MODE_NSCI )
		}
		else {
			// Wrong parameter
			if ( eng->ModeSwitch == MODE_NSCI ) {
				if ( eng->env.AST2300 )
					printf("\nNCSITEST.exe  run_mode  <package_num>  <channel_num>  <test_mode>  <IO margin>\n\n");
				else
					printf("\nNCSITEST.exe  run_mode  <package_num>  <channel_num>  <test_mode>\n\n");
				PrintMode         ( eng );
				PrintPakNUm       ( eng );
				PrintChlNUm       ( eng );
				PrintTest         ( eng );
				PrintIOTimingBund ( eng );
			}
			else {
#ifdef Enable_MAC_ExtLoop
				printf("\nMACLOOP.exe  run_mode  <speed>\n\n");
#else
				if ( eng->env.AST2300 )
					printf("\nMACTEST.exe  run_mode  <speed>  <ctrl>  <loop_max>  <test_mode>  <phy_adr>  <IO margin>\n\n");
				else
					printf("\nMACTEST.exe  run_mode  <speed>  <ctrl>  <loop_max>  <test_mode>  <phy_adr>\n\n");
#endif
				PrintMode         ( eng );
				PrintSpeed        ( eng );
#ifndef Enable_MAC_ExtLoop
				PrintCtrl         ( eng );
				PrintLoop         ( eng );
				PrintTest         ( eng );
				PrintPHYAdr       ( eng );
				PrintIOTimingBund ( eng );
#endif
			} // End if ( eng->ModeSwitch == MODE_NSCI )
			Finish_Close( eng );

			return(1);
		} // End if (argc > 1)

#ifdef Enable_MAC_ExtLoop
		eng->arg.GChk_TimingBund = 0;
		eng->arg.GTestMode       = 0;
		eng->arg.GLOOP_INFINI    = 1;
		eng->arg.GCtrl           = 0;
#endif

//------------------------------------------------------------
// Check Argument & Setup
//------------------------------------------------------------
		//------------------------------
		// [Env]check MAC34_vld
		// [Arg]check GRun_Mode
		// [Run]setup MAC_idx
		// [Run]setup MAC_BASE
		//------------------------------
		switch ( eng->arg.GRun_Mode ) {
			case 0:                            printf("\n[MAC1]\n"); eng->run.MAC_idx = 0; eng->run.MAC_BASE = MAC_BASE1; break;
			case 1:                            printf("\n[MAC2]\n"); eng->run.MAC_idx = 1; eng->run.MAC_BASE = MAC_BASE2; break;
			case 2: if ( eng->env.MAC34_vld ) {printf("\n[MAC3]\n"); eng->run.MAC_idx = 2; eng->run.MAC_BASE = MAC_BASE3; break;}
			        else
			            goto Error_GRun_Mode;
			case 3: if ( eng->env.MAC34_vld ) {printf("\n[MAC4]\n"); eng->run.MAC_idx = 3; eng->run.MAC_BASE = MAC_BASE4; break;}
			        else
			            goto Error_GRun_Mode;
			default:
Error_GRun_Mode:
				printf("Error run_mode!!!\n");
				PrintMode ( eng );
				return(1);
		} // End switch ( eng->arg.GRun_Mode )

		//------------------------------
		// [Arg]check GSpeed
		// [Run]setup Speed_1G
		// [Run]setup Speed_org
		//------------------------------
		switch ( eng->arg.GSpeed ) {
			case SET_1GBPS          : eng->run.Speed_1G = 1; eng->run.Speed_org[ 0 ] = 1; eng->run.Speed_org[ 1 ] = 0; eng->run.Speed_org[ 2 ] = 0; break;
			case SET_100MBPS        : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 0; eng->run.Speed_org[ 1 ] = 1; eng->run.Speed_org[ 2 ] = 0; break;
			case SET_10MBPS         : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 0; eng->run.Speed_org[ 1 ] = 0; eng->run.Speed_org[ 2 ] = 1; break;
#ifndef Enable_MAC_ExtLoop
			case SET_1G_100M_10MBPS : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 1; eng->run.Speed_org[ 1 ] = 1; eng->run.Speed_org[ 2 ] = 1; break;
			case SET_100M_10MBPS    : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 0; eng->run.Speed_org[ 1 ] = 1; eng->run.Speed_org[ 2 ] = 1; break;
#endif
			default:
				printf("Error speed!!!\n");
				PrintSpeed ( eng );
				return(1);
		} // End switch ( eng->arg.GSpeed )

		//------------------------------
		// [Arg]check GCtrl
		// [Arg]setup GEn_MACLoopback
		// [Arg]setup GEn_SkipChkPHY
		// [Arg]setup GEn_IntLoopPHY
		// [Arg]setup GEn_InitPHY
		// [Arg]setup GDis_RecovPHY
		// [Arg]setup GEn_PHYAdrInv
		// [Arg]setup GEn_SinglePacket
		//------------------------------
		if ( eng->arg.GCtrl & 0xffffff80 ) {
			printf("Error ctrl!!!\n");
			PrintCtrl ( eng );
			return(1);
		}
		else {
			eng->arg.GEn_MACLoopback  = ( eng->arg.GCtrl >> 6 ) & 0x1;
			eng->arg.GEn_SkipChkPHY   = ( eng->arg.GCtrl >> 5 ) & 0x1;
			eng->arg.GEn_IntLoopPHY   = ( eng->arg.GCtrl >> 4 ) & 0x1;

			eng->arg.GEn_InitPHY      = ( eng->arg.GCtrl >> 3 ) & 0x1;
			eng->arg.GDis_RecovPHY    = ( eng->arg.GCtrl >> 2 ) & 0x1;
			eng->arg.GEn_PHYAdrInv    = ( eng->arg.GCtrl >> 1 ) & 0x1;
			eng->arg.GEn_SinglePacket = ( eng->arg.GCtrl      ) & 0x1;

			if ( !eng->env.AST2400 && eng->arg.GEn_MACLoopback ) {
				printf("Error ctrl!!!\n");
				PrintCtrl ( eng );
				return(1);
			}
		} // End if ( eng->arg.GCtrl & 0xffffff83 )

		if ( eng->ModeSwitch == MODE_NSCI ) {
			//------------------------------
			// [Arg]check GPackageTolNum
			// [Arg]check GChannelTolNum
			//------------------------------
			if (( eng->arg.GPackageTolNum < 1 ) || ( eng->arg.GPackageTolNum >  8 )) {
				PrintPakNUm ( eng );
				return(1);
			}
			if (( eng->arg.GChannelTolNum < 1 ) || ( eng->arg.GChannelTolNum > 32 )) {
				PrintChlNUm ( eng );
				return(1);
			}
			//------------------------------
			// [Arg]check GARPNumCnt
			// [Arg]setup GEn_PrintNCSI
			// [Arg]setup GARPNumCnt
			//------------------------------
			eng->arg.GEn_PrintNCSI = (eng->arg.GARPNumCnt & 0x1);
			eng->arg.GARPNumCnt    = eng->arg.GARPNumCnt & 0xfffffffe;
		}
		else {
			//------------------------------
			// [Arg]check GPHYADR
			//------------------------------
			if ( eng->arg.GPHYADR > 31 ) {
				printf("Error phy_adr!!!\n");
				PrintPHYAdr ( eng );
				return(1);
			} // End if ( eng->arg.GPHYADR > 31)
			//------------------------------
			// [Arg]check GLOOP_MAX
			// [Arg]check GSpeed
			// [Arg]setup GLOOP_MAX
			//------------------------------
			if ( !eng->arg.GLOOP_MAX )
				switch ( eng->arg.GSpeed ) {
					case SET_1GBPS         : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 20; break;
					case SET_100MBPS       : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 2 ; break;
					case SET_10MBPS        : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX     ; break;
					case SET_1G_100M_10MBPS: eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 20; break;
					case SET_100M_10MBPS   : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 2 ; break;
				}
		} // End if ( eng->ModeSwitch == MODE_NSCI )

//------------------------------------------------------------
// Check Argument & Setup Running Parameter
//------------------------------------------------------------
		//------------------------------
		// [Env]check AST2300
		// [Arg]check GTestMode
		// [Run]setup TM_IOTiming
		// [Run]setup TM_IOStrength
		// [Run]setup TM_TxDataEn
		// [Run]setup TM_RxDataEn
		// [Run]setup TM_NCSI_DiSChannel
		// [Run]setup TM_Burst
		// [Run]setup TM_IEEE
		// [Run]setup TM_WaitStart
		//------------------------------
		eng->run.TM_IOTiming        = 0;
		eng->run.TM_IOStrength      = 0;
		eng->run.TM_TxDataEn        = 1;
		eng->run.TM_RxDataEn        = 1;
		eng->run.TM_NCSI_DiSChannel = 1; // For ncsitest function
		eng->run.TM_Burst           = 0; // For mactest function
		eng->run.TM_IEEE            = 0; // For mactest function
		eng->run.TM_WaitStart       = 0; // For mactest function
		if ( eng->ModeSwitch == MODE_NSCI ) {
			switch ( eng->arg.GTestMode ) {
				case 0 :     break;
				case 1 :     eng->run.TM_NCSI_DiSChannel = 0; break;
				case 6 : if ( eng->env.AST2300 ) {
					     eng->run.TM_IOTiming = 1; break;}
					 else
					     goto Error_GTestMode_NCSI;
				case 7 : if ( eng->env.AST2300 ) {
					     eng->run.TM_IOTiming = 1; eng->run.TM_IOStrength = 1; break;}
					 else
					     goto Error_GTestMode_NCSI;
				default:
Error_GTestMode_NCSI:
					printf("Error test_mode!!!\n");
					PrintTest ( eng );
					return(1);
			} // End switch ( eng->arg.GTestMode )
		}
		else {
			switch ( eng->arg.GTestMode ) {
				case  0 :     break;
				case  1 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  2 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  3 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  4 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 0; break;
				case  5 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  6 : if ( eng->env.AST2300 ) {
				              eng->run.TM_IOTiming = 1; break;}
				          else
				              goto Error_GTestMode;
				case  7 : if ( eng->env.AST2300 ) {
				              eng->run.TM_IOTiming = 1; eng->run.TM_IOStrength = 1; break;}
				          else
				              goto Error_GTestMode;
				case  8 :     eng->run.TM_RxDataEn = 0; break;
				case  9 :     eng->run.TM_TxDataEn = 0; break;
				case 10 :     eng->run.TM_WaitStart = 1; break;
				default:
Error_GTestMode:
					printf("Error test_mode!!!\n");
					PrintTest ( eng );
					return(1);
			} // End switch ( eng->arg.GTestMode )
		} // End if ( eng->ModeSwitch == MODE_NSCI )

		//------------------------------
		// [Env]check AST2300
		// [Arg]check GChk_TimingBund
		// [Run]check TM_Burst
		// [Arg]setup GIEEE_sel
		// [Run]setup IO_Bund
		//------------------------------
		if ( eng->run.TM_Burst ) {
			eng->arg.GIEEE_sel = eng->arg.GChk_TimingBund;
			eng->run.IO_Bund = 0;
		}
		else {
			eng->arg.GIEEE_sel = 0;
			if ( eng->env.AST2300 ) {
				eng->run.IO_Bund = eng->arg.GChk_TimingBund;

				if ( !( ( ( 7 >= eng->run.IO_Bund ) && ( eng->run.IO_Bund & 0x1 ) ) ||
				        ( eng->run.IO_Bund == 0                                   )
				       ) ) {
					printf("Error IO margin!!!\n");
					PrintIOTimingBund ( eng );
					return(1);
				}
			}
			else {
				eng->run.IO_Bund = 0;
			}
		} // End if ( eng->run.TM_Burst )

//------------------------------------------------------------
// File Name
//------------------------------------------------------------
#if defined(ENABLE_LOG_FILE)
		//------------------------------
		// [Arg]check GEn_IntLoopPHY
		// [Run]check TM_Burst
		// [Run]check TM_IOTiming
		// [Run]check TM_IOStrength
		//------------------------------
		if ( !eng->run.TM_Burst ) {
			// Define Output file name
			if ( eng->ModeSwitch == MODE_NSCI )
				sprintf( FileNameMain, "%d",  eng->run.MAC_idx + 1 );
			else {
				if ( eng->arg.GEn_IntLoopPHY )
					sprintf( FileNameMain, "%dI",  eng->run.MAC_idx + 1 );
				else
					sprintf( FileNameMain, "%dE",  eng->run.MAC_idx + 1 );
			}

			if ( eng->run.TM_IOTiming ) {
				if ( eng->run.TM_IOStrength )
					sprintf( FileName, "MIOD%sS.log", FileNameMain );
				else
					sprintf( FileName, "MIOD%s.log", FileNameMain );

				eng->fp_log = fopen( FileName,"w" );

				if ( eng->run.TM_IOStrength )
					sprintf( FileName, "MIO%sS.log", FileNameMain );
				else
					sprintf( FileName, "MIO%s.log", FileNameMain );

				eng->fp_io  = fopen( FileName,"w" );
			}
			else {
				sprintf( FileName, "MAC%s.log", FileNameMain );

				eng->fp_log = fopen( FileName,"w" );
			}
		} // End if ( !eng->run.TM_Burst )
#endif // End defined(ENABLE_LOG_FILE)

//------------------------------------------------------------
// Setup Environment
//------------------------------------------------------------
		//------------------------------
		// [Env]check AST1010
		// [Env]check AST2300
		// [Reg]check SCU_070
		// [Env]setup MAC_Mode
		// [Env]setup MAC1_1Gvld
		// [Env]setup MAC2_1Gvld
		// [Env]setup MAC1_RMII
		// [Env]setup MAC2_RMII
		// [Env]setup MAC2_vld
		//------------------------------
#ifdef AST1010_CHIP
		eng->env.MAC_Mode   = 0;
		eng->env.MAC1_1Gvld = 0;
		eng->env.MAC2_1Gvld = 0;

		eng->env.MAC1_RMII  = 1;
		eng->env.MAC2_RMII  = 0;
		eng->env.MAC2_vld   = 0;
#else
		if ( eng->env.AST2300 ) {
			eng->env.MAC_Mode   = ( eng->reg.SCU_070 >> 6 ) & 0x3;
			eng->env.MAC1_1Gvld = ( eng->env.MAC_Mode & 0x1 ) ? 1 : 0;//1:RGMII, 0:RMII
			eng->env.MAC2_1Gvld = ( eng->env.MAC_Mode & 0x2 ) ? 1 : 0;//1:RGMII, 0:RMII

			eng->env.MAC1_RMII  = !eng->env.MAC1_1Gvld;
			eng->env.MAC2_RMII  = !eng->env.MAC2_1Gvld;
			eng->env.MAC2_vld   = 1;
		}
		else {
			eng->env.MAC_Mode   = ( eng->reg.SCU_070 >> 6 ) & 0x7;
			eng->env.MAC1_1Gvld = ( eng->env.MAC_Mode == 0x0 ) ? 1 : 0;
			eng->env.MAC2_1Gvld = 0;

			switch ( eng->env.MAC_Mode ) {
				case 0 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //000: Select GMII(MAC#1) only
				case 1 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 1; break; //001: Select MII (MAC#1) and MII(MAC#2)
				case 2 : eng->env.MAC1_RMII = 1; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 1; break; //010: Select RMII(MAC#1) and MII(MAC#2)
				case 3 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //011: Select MII (MAC#1) only
				case 4 : eng->env.MAC1_RMII = 1; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //100: Select RMII(MAC#1) only
//				case 5 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //101: Reserved
				case 6 : eng->env.MAC1_RMII = 1; eng->env.MAC2_RMII = 1; eng->env.MAC2_vld = 1; break; //110: Select RMII(MAC#1) and RMII(MAC#2)
//				case 7 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //111: Disable dual MAC
				default:
					return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		} // End if ( eng->env.AST2300 )
#endif

		eng->env.MAC_atlast_1Gvld = eng->env.MAC1_1Gvld | eng->env.MAC2_1Gvld;

//------------------------------------------------------------
// Check & Setup Environment
//------------------------------------------------------------
		//------------------------------
		// [Phy]setup PHY_BASE
		// [Env]setup MAC_1Gvld
		// [Env]setup MAC_RMII
		//------------------------------
		if ( eng->run.MAC_idx == 0 ) {
			if ( eng->arg.GEn_PHYAdrInv )
				eng->phy.PHY_BASE = MAC_BASE2;
			else
				eng->phy.PHY_BASE = MAC_BASE1;
			eng->env.MAC_1Gvld = eng->env.MAC1_1Gvld;
			eng->env.MAC_RMII  = eng->env.MAC1_RMII;

			if ( eng->run.Speed_1G & !eng->env.MAC1_1Gvld ) {
				printf("\nMAC1 don't support 1Gbps !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		}
		else if ( eng->run.MAC_idx == 1 ) {
			if ( eng->arg.GEn_PHYAdrInv )
				eng->phy.PHY_BASE = MAC_BASE1;
			else
				eng->phy.PHY_BASE = MAC_BASE2;
			eng->env.MAC_1Gvld = eng->env.MAC2_1Gvld;
			eng->env.MAC_RMII  = eng->env.MAC2_RMII;

			if ( eng->run.Speed_1G & !eng->env.MAC2_1Gvld ) {
				printf("\nMAC2 don't support 1Gbps !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
			if ( !eng->env.MAC2_vld ) {
				printf("\nMAC2 not valid !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		}
		else {
			if ( eng->run.MAC_idx == 2 )
				if ( eng->arg.GEn_PHYAdrInv )
					eng->phy.PHY_BASE = MAC_BASE4;
				else
					eng->phy.PHY_BASE = MAC_BASE3;
			else
				if ( eng->arg.GEn_PHYAdrInv )
					eng->phy.PHY_BASE = MAC_BASE3;
				else
					eng->phy.PHY_BASE = MAC_BASE4;

			eng->env.MAC_1Gvld = 0;
			eng->env.MAC_RMII  = 1;

			if ( eng->run.Speed_1G ) {
				printf("\nMAC3/MAC4 don't support 1Gbps !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		} // End if ( eng->run.MAC_idx == 0 )

		if ( !eng->env.MAC_1Gvld )
			eng->run.Speed_org[ 0 ] = 0;

		if ( ( eng->ModeSwitch == MODE_NSCI ) && ( !eng->env.MAC_RMII ) ) {
			printf("\nNCSI must be RMII interface !!!\n");
			return( Finish_Check( eng, Err_Flag_MACMode ) );
		}

		//------------------------------
		// [Env]setup MHCLK_Ratio
		//------------------------------
#ifdef AST1010_CHIP
		// Check bit 13:12
		// The STA of the AST1010 is MHCLK 100 MHz
		eng->env.MHCLK_Ratio = ( eng->reg.SCU_008 >> 12 ) & 0x3;
		if ( eng->env.MHCLK_Ratio != 0x0 ) {
			FindErr( eng, Err_Flag_MHCLK_Ratio );
//			return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
		}
#elif defined(AST2500_IOMAP)
		eng->env.MHCLK_Ratio = ( eng->reg.SCU_008 >> 16 ) & 0x7;
		if ( eng->env.MAC_atlast_1Gvld ) {
			if ( eng->env.MHCLK_Ratio != 2 ) {
				FindErr( eng, Err_Flag_MHCLK_Ratio );
//				return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
			}
		}
		else {
			if ( eng->env.MHCLK_Ratio != 4 ) {
				FindErr( eng, Err_Flag_MHCLK_Ratio );
//				return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
			}
		}
#else
		if ( eng->env.AST2300 ) {
			eng->env.MHCLK_Ratio = ( eng->reg.SCU_008 >> 16 ) & 0x7;
			if ( eng->env.MAC_atlast_1Gvld ) {
				if ( ( eng->env.MHCLK_Ratio == 0 ) || ( eng->env.MHCLK_Ratio > 2 ) ) {
					FindErr( eng, Err_Flag_MHCLK_Ratio );
//					return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
				}
			}
			else {
				if ( eng->env.MHCLK_Ratio != 4 ) {
					FindErr( eng, Err_Flag_MHCLK_Ratio );
//					return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
				}
			}
		} // End if ( eng->env.AST2300 )
#endif

//------------------------------------------------------------
// Parameter Initial
//------------------------------------------------------------
		//------------------------------
		// [Reg]setup SCU_004_rstbit
		// [Reg]setup SCU_004_mix
		// [Reg]setup SCU_004_dis
		// [Reg]setup SCU_004_en
		//------------------------------
#ifdef AST1010_CHIP
		eng->reg.SCU_004_rstbit = 0x00000010; //Reset Engine
#elif defined(AST2500_IOMAP)
		if ( eng->arg.GEn_PHYAdrInv ) {
			eng->reg.SCU_004_rstbit = 0x00001800; //Reset Engine
		}
		else {
			if ( eng->run.MAC_idx == 1 )
				eng->reg.SCU_004_rstbit = 0x00001000; //Reset Engine
			else
				eng->reg.SCU_004_rstbit = 0x00000800; //Reset Engine
		}
#else
			if ( eng->env.AST2300 )
				eng->reg.SCU_004_rstbit = 0x0c001800; //Reset Engine
			else
				eng->reg.SCU_004_rstbit = 0x00001800; //Reset Engine
#endif
		eng->reg.SCU_004_mix = eng->reg.SCU_004;
		eng->reg.SCU_004_en  = eng->reg.SCU_004_mix & (~eng->reg.SCU_004_rstbit);
		eng->reg.SCU_004_dis = eng->reg.SCU_004_mix |   eng->reg.SCU_004_rstbit;

		//------------------------------
		// [Reg]setup MAC_050
		//------------------------------
		if ( eng->ModeSwitch == MODE_NSCI )
			// Set to 100Mbps and Enable RX broabcast packets and CRC_APD and Full duplex
			eng->reg.MAC_050 = 0x000a0500;// [100Mbps] RX_BROADPKT_EN & CRC_APD & Full duplex
//			eng->reg.MAC_050 = 0x000a4500;// [100Mbps] RX_BROADPKT_EN & RX_ALLADR & CRC_APD & Full duplex
		else {
#ifdef Enable_MAC_ExtLoop
			eng->reg.MAC_050 = 0x00004100;// RX_ALLADR & Full duplex
#else
			eng->reg.MAC_050 = 0x00004500;// RX_ALLADR & CRC_APD & Full duplex
#endif
#ifdef Enable_Runt
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00001000;
#endif
#if defined(PHY_SPECIAL)
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00002000;
#elif defined(Enable_Jumbo)
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00002000;
#endif
		} // End if ( eng->ModeSwitch == MODE_NSCI )

//------------------------------------------------------------
// Descriptor Number
//------------------------------------------------------------
		//------------------------------
		// [Dat]setup Des_Num
		// [Dat]setup DMABuf_Size
		// [Dat]setup DMABuf_Num
		//------------------------------
		if ( eng->ModeSwitch == MODE_DEDICATED ) {
#ifdef Enable_Jumbo
			DES_LowNumber = 1;
#else
			DES_LowNumber = eng->run.TM_IOTiming;
#endif
			if ( eng->arg.GEn_SkipChkPHY && ( eng->arg.GTestMode == 0 ) )
				eng->dat.Des_Num = 114;//for SMSC's LAN9303 issue
			else {
#ifdef AST1010_CHIP
				eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : 256;
#else
				switch ( eng->arg.GSpeed ) {
					case SET_1GBPS          : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 512 : 4096; break;
					case SET_100MBPS        : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 512 : 4096; break;
					case SET_10MBPS         : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
					case SET_1G_100M_10MBPS : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
					case SET_100M_10MBPS    : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
				}
#endif
			} // End if ( eng->arg.GEn_SkipChkPHY && ( eng->arg.GTestMode == 0 ) )
#ifdef SelectDesNumber
			eng->dat.Des_Num = SelectDesNumber;
#endif
#ifdef USE_LPC
			eng->dat.Des_Num /= 8;
#endif
#ifdef ENABLE_ARP_2_WOL
			if ( eng->arg.GTestMode == 4 )
				eng->dat.Des_Num = 1;
#endif
			eng->dat.Des_Num_Org = eng->dat.Des_Num;
			eng->dat.DMABuf_Size = DMA_BufSize; //keep in order: Des_Num --> DMABuf_Size --> DMABuf_Num
			eng->dat.DMABuf_Num  = DMA_BufNum;  //keep in order: Des_Num --> DMABuf_Size --> DMABuf_Num

			if ( DbgPrn_Info ) {
				printf("CheckBuf_MBSize : %ld\n",       eng->run.CheckBuf_MBSize);
				printf("LOOP_CheckNum   : %ld\n",       eng->run.LOOP_CheckNum);
				printf("Des_Num         : %ld\n",       eng->dat.Des_Num);
				printf("DMA_BufSize     : %ld bytes\n", eng->dat.DMABuf_Size);
				printf("DMA_BufNum      : %ld\n",       eng->dat.DMABuf_Num);
				printf("DMA_PakSize     : %d\n",        DMA_PakSize);
				printf("\n");
			}
			if ( 2 > eng->dat.DMABuf_Num )
				return( Finish_Check( eng, Err_Flag_DMABufNum ) );
		} // End if ( eng->ModeSwitch == MODE_DEDICATED )

//------------------------------------------------------------
// Setup Running Parameter
//------------------------------------------------------------
#ifdef Enable_MAC_ExtLoop
		eng->run.TDES_BASE = RDES_BASE1;
		eng->run.RDES_BASE = RDES_BASE1;
#else
		eng->run.TDES_BASE = TDES_BASE1;
		eng->run.RDES_BASE = RDES_BASE1;
#endif

		if ( eng->run.TM_IOTiming || eng->run.IO_Bund )
			eng->run.IO_MrgChk = 1;
		else
			eng->run.IO_MrgChk = 0;

		eng->phy.Adr      = eng->arg.GPHYADR;
		eng->phy.loop_phy = eng->arg.GEn_IntLoopPHY;

		eng->run.LOOP_MAX = eng->arg.GLOOP_MAX;
		Calculate_LOOP_CheckNum( eng );

	} // End if (RUN_STEP >= 1)

//------------------------------------------------------------
// SCU Initial
//------------------------------------------------------------
	if ( RUN_STEP >= 2 ) {
		get_mac_info( eng );
		Setting_scu( eng );
		init_scu1( eng );
	}

	if ( RUN_STEP >= 3 ) {
		init_scu_macrst( eng );
		if ( eng->ModeSwitch ==  MODE_DEDICATED ) {
			eng->phy.PHYAdrValid = find_phyadr( eng );
			if ( eng->phy.PHYAdrValid == TRUE )
				phy_sel( eng, phyeng );
		}
	}

//------------------------------------------------------------
// Data Initial
//------------------------------------------------------------
	if (RUN_STEP >= 4) {
#if defined(PHY_SPECIAL)
		special_PHY_buf_init( eng );
#endif
		setup_arp ( eng );
		if ( eng->ModeSwitch ==  MODE_DEDICATED ) {

			eng->dat.FRAME_LEN = (ULONG *)malloc( eng->dat.Des_Num    * sizeof( ULONG ) );
			eng->dat.wp_lst    = (ULONG *)malloc( eng->dat.Des_Num    * sizeof( ULONG ) );

			if ( !eng->dat.FRAME_LEN )
				return( Finish_Check( eng, Err_Flag_MALLOC_FrmSize ) );
			if ( !eng->dat.wp_lst )
				return( Finish_Check( eng, Err_Flag_MALLOC_LastWP ) );

			// Setup data and length
			TestingSetup ( eng );
		} // End if ( eng->ModeSwitch ==  MODE_DEDICATED )

		init_iodelay( eng );
		eng->run.Speed_idx = 0;
		if ( !eng->io.Dly_3Regiser )
			if ( get_iodelay( eng ) )
				return( Finish_Check( eng, 0 ) );

	} // End if (RUN_STEP >= 4)

//------------------------------------------------------------
// main
//------------------------------------------------------------
	if (RUN_STEP >= 5) {
#ifdef  DbgPrn_FuncHeader
		printf("Speed_org: %d %d %d\n", eng->run.Speed_org[ 0 ], eng->run.Speed_org[ 1 ], eng->run.Speed_org[ 2 ]);
		Debug_delay();
#endif

#ifdef Enable_LOOP_INFINI
LOOP_INFINI:;
#endif
		for ( eng->run.Speed_idx = 0; eng->run.Speed_idx < 3; eng->run.Speed_idx++ )
			eng->run.Speed_sel[ (int)eng->run.Speed_idx ] = eng->run.Speed_org[ (int)eng->run.Speed_idx ];

		//------------------------------
		// [Start] The loop of different speed
		//------------------------------
		for ( eng->run.Speed_idx = 0; eng->run.Speed_idx < 3; eng->run.Speed_idx++ ) {
			eng->flg.Flag_PrintEn = 1;
			if ( eng->run.Speed_sel[ (int)eng->run.Speed_idx ] ) {
				// Setting speed of LAN
				if      ( eng->run.Speed_sel[ 0 ] ) eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0000020f;
				else if ( eng->run.Speed_sel[ 1 ] ) eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0008000f;
				else                                eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0000000f;
#ifdef Enable_CLK_Stable
				Write_Reg_SCU_DD( 0x0c, ( eng->reg.SCU_00c |   eng->reg.SCU_00c_clkbit  ) );//Clock Stop Control
				Read_Reg_SCU_DD( 0x0c );
				Write_Reg_MAC_DD( eng, 0x50, eng->reg.MAC_050_Speed & 0xfffffff0 );
				Write_Reg_SCU_DD( 0x0c, ( eng->reg.SCU_00c & (~eng->reg.SCU_00c_clkbit) ) );//Clock Stop Control
#endif

				// Setting check owner time out
				if      ( eng->run.Speed_sel[ 0 ] ) eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_1G;
				else if ( eng->run.Speed_sel[ 1 ] ) eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_100M;
				else                                eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_10M;

				if ( eng->run.TM_WaitStart )
					eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des * 10000;

				// Setting the LAN speed
				if ( eng->ModeSwitch ==  MODE_DEDICATED ) {
					// Test three speed of LAN, we will modify loop number
					if ( ( eng->arg.GSpeed == SET_1G_100M_10MBPS ) || ( eng->arg.GSpeed == SET_100M_10MBPS ) ) {
						if      ( eng->run.Speed_sel[ 0 ] ) eng->run.LOOP_MAX = eng->arg.GLOOP_MAX;
						else if ( eng->run.Speed_sel[ 1 ] ) eng->run.LOOP_MAX = eng->arg.GLOOP_MAX / 100;
						else                                eng->run.LOOP_MAX = eng->arg.GLOOP_MAX / 1000;

						if ( !eng->run.LOOP_MAX )
							eng->run.LOOP_MAX = 1;

						Calculate_LOOP_CheckNum( eng );
					}

					//------------------------------
					// PHY Initial
					//------------------------------
					if ( eng->env.AST1100 )
						init_scu2 ( eng );

#ifdef SUPPORT_PHY_LAN9303
					if ( eng->arg.GEn_InitPHY )
						LAN9303( LAN9303_I2C_BUSNUM, eng->arg.GPHYADR, eng->run.Speed_idx, eng->arg.GEn_IntLoopPHY | (eng->run.TM_Burst<<1) | eng->run.TM_IEEE );
#elif defined(PHY_SPECIAL)
					special_PHY_reg_init( eng );
#else
					if ( phyeng->fp_set != 0 ) {
						init_phy( eng, phyeng );
  #ifdef Delay_PHYRst
//						DELAY( Delay_PHYRst * 10 );
  #endif
					}
#endif

					if ( eng->env.AST1100 )
						init_scu3 ( eng );

					if ( eng->flg.Err_Flag )
						return( Finish_Check( eng, 0 ) );
				} // End if ( eng->ModeSwitch ==  MODE_DEDICATED )

				//------------------------------
				// [Start] The loop of different IO strength
				//------------------------------
				for ( eng->io.Str_i = 0; eng->io.Str_i <= eng->io.Str_max; eng->io.Str_i++ ) {
					//------------------------------
					// Print Header of report to monitor and log file
					//------------------------------
					if ( eng->io.Dly_3Regiser )
						if ( get_iodelay( eng ) )
							return( Finish_Check( eng, 0 ) );

					if ( eng->run.IO_MrgChk ) {
						if ( eng->run.TM_IOStrength ) {
#ifdef AST1010_CHIP
							eng->io.Str_val = eng->io.Str_reg_mask | ( ( eng->io.Str_i ) ? 0xc000 : 0x0 );
#else
							eng->io.Str_val = eng->io.Str_reg_mask | ( eng->io.Str_i << eng->io.Str_shf );
#endif
//printf("\nIOStrength_val= %08lx, ", eng->io.Str_val);
//printf("SCU90h: %08lx ->", Read_Reg_SCU_DD( 0x90 ));
							Write_Reg_SCU_DD( eng->io.Str_reg_idx, eng->io.Str_val );
//printf(" %08lx\n", Read_Reg_SCU_DD( 0x90 ));
						}

						if ( eng->run.IO_Bund )
							PrintIO_Header( eng, FP_LOG );
						if ( eng->run.TM_IOTiming )
							PrintIO_Header( eng, FP_IO );
						PrintIO_Header( eng, STD_OUT );
					}
					else {
						if ( eng->ModeSwitch == MODE_DEDICATED ) {
							if ( !eng->run.TM_Burst )
								Print_Header( eng, FP_LOG );
							Print_Header( eng, STD_OUT );
						}
					} // End if ( eng->run.IO_MrgChk )

					//------------------------------
					// [Start] The loop of different IO out delay
					//------------------------------
					for ( eng->io.Dly_out = eng->io.Dly_out_str; eng->io.Dly_out <= eng->io.Dly_out_end; eng->io.Dly_out+=eng->io.Dly_out_cval ) {
						if ( eng->run.IO_MrgChk ) {
							eng->io.Dly_out_selval  = eng->io.value_ary[ eng->io.Dly_out ];
							eng->io.Dly_out_reg_hit = ( eng->io.Dly_out_reg == eng->io.Dly_out_selval ) ? 1 : 0;
#ifdef Enable_Fast_SCU
							Write_Reg_SCU_DD( eng->io.Dly_reg_idx, eng->reg.SCU_048_mix | ( eng->io.Dly_out_selval << eng->io.Dly_out_shf ) );
#endif
							if ( eng->run.TM_IOTiming )
								PrintIO_LineS( eng, FP_IO );
							PrintIO_LineS( eng, STD_OUT );
						} // End if ( eng->run.IO_MrgChk )

#ifdef Enable_Fast_SCU
						//------------------------------
						// SCU Initial
						//------------------------------
						init_scu_macrst( eng );

						//------------------------------
						// MAC Initial
						//------------------------------
						init_mac( eng );
						if ( eng->flg.Err_Flag )
							return( Finish_Check( eng, 0 ) );
#endif
						//------------------------------
						// [Start] The loop of different IO in delay
						//------------------------------
						for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
							if ( eng->run.IO_MrgChk ) {
								eng->io.Dly_in_selval  = eng->io.value_ary[ eng->io.Dly_in ];
								eng->io.Dly_val = ( eng->io.Dly_in_selval  << eng->io.Dly_in_shf  )
								                | ( eng->io.Dly_out_selval << eng->io.Dly_out_shf );

//printf("\nDly_val= %08lx, ", eng->io.Dly_val);
//printf("SCU%02lxh: %08lx ->", eng->io.Dly_reg_idx, Read_Reg_SCU_DD( eng->io.Dly_reg_idx ) );
								Write_Reg_SCU_DD( eng->io.Dly_reg_idx, eng->reg.SCU_048_mix | eng->io.Dly_val );
//printf(" %08lx\n", Read_Reg_SCU_DD( eng->io.Dly_reg_idx ) );
							} // End if ( eng->run.IO_MrgChk )
#ifdef Enable_Fast_SCU
#else
							//------------------------------
							// SCU Initial
							//------------------------------
							init_scu_macrst( eng );

							//------------------------------
							// MAC Initial
							//------------------------------
							init_mac( eng );
							if ( eng->flg.Err_Flag )
								return( Finish_Check( eng, 0 ) );
#endif
							// Testing
							if ( eng->ModeSwitch == MODE_NSCI )
								eng->io.Dly_result = phy_ncsi( eng );
							else
								eng->io.Dly_result = TestingLoop( eng, eng->run.LOOP_CheckNum );
							eng->io.dlymap[ eng->io.Dly_in ][ eng->io.Dly_out ] = eng->io.Dly_result;

							// Display to Log file and monitor
							if ( eng->run.IO_MrgChk ) {
								if ( eng->run.TM_IOTiming )
									PrintIO_Line( eng, FP_IO );
								PrintIO_Line( eng, STD_OUT );

								FPri_ErrFlag( eng, FP_LOG );
								PrintIO_Line_LOG( eng );

								eng->flg.Wrn_Flag  = 0;
								eng->flg.Err_Flag  = 0;
								eng->flg.Des_Flag  = 0;
								eng->flg.NCSI_Flag = 0;
							} //End if ( eng->run.IO_MrgChk )
						} // End for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval )


						if ( eng->run.IO_MrgChk ) {
							if ( eng->run.TM_IOTiming ) {
								PRINTF( FP_IO, "\n" );
							}
							printf("\n");
						}
					} // End for ( eng->io.Dly_out = eng->io.Dly_out_str; eng->io.Dly_out <= eng->io.Dly_out_end; eng->io.Dly_out+=eng->io.Dly_out_cval )


					//------------------------------
					// End
					//------------------------------
					if ( eng->run.IO_MrgChk ) {
						for ( eng->io.Dly_out = eng->io.Dly_out_min; eng->io.Dly_out <= eng->io.Dly_out_max; eng->io.Dly_out++ )
							for ( eng->io.Dly_in = eng->io.Dly_in_min; eng->io.Dly_in <= eng->io.Dly_in_max; eng->io.Dly_in++ )
								if ( eng->io.dlymap[ eng->io.Dly_in ][ eng->io.Dly_out ] ) {
									if ( eng->run.TM_IOTiming ) {
										for ( j = eng->io.Dly_out_min; j <= eng->io.Dly_out_max; j++ ) {
											for ( i = eng->io.Dly_in_min; i <= eng->io.Dly_in_max; i++ )
												if ( eng->io.dlymap[i][j] )
													{ PRINTF( FP_IO, "x " ); }
												else
													{ PRINTF( FP_IO, "o " ); }
											PRINTF( FP_IO, "\n" );
										}
									} // End if ( eng->run.TM_IOTiming )

									FindErr( eng, Err_Flag_IOMargin );
									goto Find_Err_Flag_IOMargin;
								} // End if ( eng->io.dlymap[ eng->io.Dly_in ][ eng->io.Dly_out ] )
					} // End if ( eng->run.IO_MrgChk )

Find_Err_Flag_IOMargin:
					if ( !eng->run.TM_Burst )
						FPri_ErrFlag( eng, FP_LOG );
					if ( eng->run.TM_IOTiming )
						FPri_ErrFlag( eng, FP_IO );

					FPri_ErrFlag( eng, STD_OUT );

					Wrn_Flag_allapeed  = Wrn_Flag_allapeed  | eng->flg.Wrn_Flag;
					Err_Flag_allapeed  = Err_Flag_allapeed  | eng->flg.Err_Flag;
					Des_Flag_allapeed  = Des_Flag_allapeed  | eng->flg.Err_Flag;
					NCSI_Flag_allapeed = NCSI_Flag_allapeed | eng->flg.Err_Flag;
					eng->flg.Wrn_Flag  = 0;
					eng->flg.Err_Flag  = 0;
					eng->flg.Des_Flag  = 0;
					eng->flg.NCSI_Flag = 0;
				} // End for ( eng->io.Str_i = 0; eng->io.Str_i <= eng->io.Str_max; eng->io.Str_i++ ) {

				if ( eng->ModeSwitch == MODE_DEDICATED ) {
#ifdef PHY_SPECIAL
					if ( eng->arg.GEn_InitPHY )
						special_PHY_recov( eng );
#else
					if ( phyeng->fp_clr != 0 )
						recov_phy( eng, phyeng );
#endif
				}

				eng->run.Speed_sel[ (int)eng->run.Speed_idx ] = 0;
			} // End if ( eng->run.Speed_sel[ eng->run.Speed_idx ] )

			eng->flg.Flag_PrintEn = 0;
		} // End for ( eng->run.Speed_idx = 0; eng->run.Speed_idx < 3; eng->run.Speed_idx++ )

		eng->flg.Wrn_Flag  = Wrn_Flag_allapeed;
		eng->flg.Err_Flag  = Err_Flag_allapeed;
		eng->flg.Des_Flag  = Des_Flag_allapeed;
		eng->flg.NCSI_Flag = NCSI_Flag_allapeed;

#ifdef Enable_LOOP_INFINI
		if ( eng->flg.Err_Flag == 0 ) {
  #if defined(ENABLE_LOG_FILE)
			if ( eng->fp_log ) {
				fclose( eng->fp_log );
				eng->fp_log = fopen(FileName,"w");
			}
  #endif
			goto LOOP_INFINI;
		}
#endif
	} // End if (RUN_STEP >= 5)

	return( Finish_Check( eng, 0 ) );
}
