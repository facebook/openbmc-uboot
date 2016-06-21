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

#define PHY_C
static const char ThisFile[] = "PHY.c";
//#define PHY_debug
//#define PHY_debug_set_clr
//#define Realtek_debug

#ifdef Realtek_debug
int	GPIO_20h_Value;
int	GPIO_24h_Value;
#endif

#include "SWFUNC.H"
#include "COMMINF.H"

#if defined(SLT_UBOOT)
  #include <common.h>
  #include <command.h>
  #include "STDUBOOT.H"
#endif
#if defined(DOS_ALONE)
  #include <stdio.h>
  #include <stdlib.h>
  #include <conio.h>
  #include <string.h>
#endif

#include "PHY.H"
#include "TYPEDEF.H"
#include "IO.H"

//------------------------------------------------------------
// PHY R/W basic
//------------------------------------------------------------
void phy_write (MAC_ENGINE *eng, int adr, ULONG data) {
	int        timeout = 0;

	if ( eng->inf.NewMDIO ) {
		Write_Reg_PHY_DD( eng, 0x60, ( data << 16 ) | MAC_PHYWr_New | (eng->phy.Adr<<5) | (adr & 0x1f) );

		while ( Read_Reg_PHY_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !eng->run.TM_Burst )
					PRINTF( FP_LOG, "[PHY-Write] Time out: %08lx\n", Read_Reg_PHY_DD( eng, 0x60 ) );

				FindErr( eng, Err_Flag_PHY_TimeOut_RW );
				break;
			}
		}
	}
	else {
		Write_Reg_PHY_DD( eng, 0x64, data );

		Write_Reg_PHY_DD( eng, 0x60, MDC_Thres | MAC_PHYWr | (eng->phy.Adr<<16) | ((adr & 0x1f) << 21) );

		while ( Read_Reg_PHY_DD( eng, 0x60 ) & MAC_PHYWr ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !eng->run.TM_Burst )
					PRINTF( FP_LOG, "[PHY-Write] Time out: %08lx\n", Read_Reg_PHY_DD( eng, 0x60 ) );

				FindErr( eng, Err_Flag_PHY_TimeOut_RW );
				break;
			}
		}
	} // End if ( eng->inf.NewMDIO )

#ifdef PHY_debug
	if ( 1 ) {
#else
	if ( DbgPrn_PHYRW ) {
#endif
		printf("[Wr ]%02d: 0x%04lx (%02d:%08lx)\n", adr, data, eng->phy.Adr, eng->phy.PHY_BASE );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "[Wr ]%02d: 0x%04lx (%02d:%08lx)\n", adr, data, eng->phy.Adr, eng->phy.PHY_BASE );
	}

} // End void phy_write (int adr, ULONG data)

//------------------------------------------------------------
ULONG phy_read (MAC_ENGINE *eng, int adr) {
	int        timeout = 0;
	ULONG      read_value;

	if ( eng->inf.NewMDIO ) {
		Write_Reg_PHY_DD( eng, 0x60, MAC_PHYRd_New | (eng->phy.Adr << 5) | ( adr & 0x1f ) );

		while ( Read_Reg_PHY_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !eng->run.TM_Burst )
					PRINTF( FP_LOG, "[PHY-Read] Time out: %08lx\n", Read_Reg_PHY_DD( eng, 0x60 ) );

				FindErr( eng, Err_Flag_PHY_TimeOut_RW );
				break;
			}
		}

#ifdef Delay_PHYRd
		DELAY( Delay_PHYRd );
#endif
		read_value = Read_Reg_PHY_DD( eng, 0x64 ) & 0xffff;
	}
	else {
		Write_Reg_PHY_DD( eng, 0x60, MDC_Thres | MAC_PHYRd | (eng->phy.Adr << 16) | ((adr & 0x1f) << 21) );

		while ( Read_Reg_PHY_DD( eng, 0x60 ) & MAC_PHYRd ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !eng->run.TM_Burst )
					PRINTF( FP_LOG, "[PHY-Read] Time out: %08lx\n", Read_Reg_PHY_DD( eng, 0x60 ) );

				FindErr( eng, Err_Flag_PHY_TimeOut_RW );
				break;
			}
		}

#ifdef Delay_PHYRd
		DELAY( Delay_PHYRd );
#endif
		read_value = Read_Reg_PHY_DD( eng, 0x64 ) >> 16;
	}

#ifdef PHY_debug
	if ( 1 ) {
#else
	if ( DbgPrn_PHYRW ) {
#endif
		printf("[Rd ]%02d: 0x%04lx (%02d:%08lx)\n", adr, read_value, eng->phy.Adr, eng->phy.PHY_BASE );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "[Rd ]%02d: 0x%04lx (%02d:%08lx)\n", adr, read_value, eng->phy.Adr, eng->phy.PHY_BASE );
	}

	return( read_value );
} // End ULONG phy_read (MAC_ENGINE *eng, int adr)

//------------------------------------------------------------
void phy_Read_Write (MAC_ENGINE *eng, int adr, ULONG clr_mask, ULONG set_mask) {
#ifdef PHY_debug
	if ( 1 ) {
#else
	if ( DbgPrn_PHYRW ) {
#endif
		printf("[RW ]%02d: clr:0x%04lx: set:0x%04lx (%02d:%08lx)\n", adr, clr_mask, set_mask, eng->phy.Adr, eng->phy.PHY_BASE);
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "[RW ]%02d: clr:0x%04lx: set:0x%04lx (%02d:%08lx)\n", adr, clr_mask, set_mask, eng->phy.Adr, eng->phy.PHY_BASE);
	}
	phy_write( eng, adr, ((phy_read( eng, adr ) & (~clr_mask)) | set_mask) );
}

//------------------------------------------------------------
void phy_out (MAC_ENGINE *eng, int adr) {
	printf("%02d: %04lx\n", adr, phy_read( eng, adr ));
}

//------------------------------------------------------------
//void phy_outchg (MAC_ENGINE *eng,  int adr) {
//	ULONG	PHY_valold = 0;
//	ULONG	PHY_val;
//
//	while (1) {
//		PHY_val = phy_read( eng, adr );
//		if (PHY_valold != PHY_val) {
//			printf("%02ld: %04lx\n", adr, PHY_val);
//			PHY_valold = PHY_val;
//		}
//	}
//}

//------------------------------------------------------------
void phy_dump (MAC_ENGINE *eng) {
	int        index;

	printf("[PHY%d][%d]----------------\n", eng->run.MAC_idx + 1, eng->phy.Adr);
	for (index = 0; index < 32; index++) {
		printf("%02d: %04lx ", index, phy_read( eng, index ));

		if ((index % 8) == 7)
			printf("\n");
	}
}

//------------------------------------------------------------
void phy_id (MAC_ENGINE *eng, BYTE option) {

	ULONG      reg_adr;
	CHAR       PHY_ADR_org;

	PHY_ADR_org = eng->phy.Adr;
	for ( eng->phy.Adr = 0; eng->phy.Adr < 32; eng->phy.Adr++ ) {

		PRINTF(option, "[%02d] ", eng->phy.Adr);

		for ( reg_adr = 2; reg_adr <= 3; reg_adr++ ) {
			PRINTF(option, "%ld:%04lx ", reg_adr, phy_read( eng, reg_adr ));
		}

		if ( ( eng->phy.Adr % 4 ) == 3 ) {
			PRINTF(option, "\n");
		}
	}
	eng->phy.Adr = PHY_ADR_org;
}

//------------------------------------------------------------
void phy_delay (int dt) {
#ifdef Realtek_debug
Write_Reg_GPIO_DD( 0x20, GPIO_20h_Value & 0xffbfffff);
//	delay_hwtimer( dt );
#endif
#ifdef PHY_debug
	printf("delay %d ms\n", dt);
#endif
	DELAY( dt );

#ifdef Realtek_debug
Write_Reg_GPIO_DD( 0x20, GPIO_20h_Value );
#endif
}

//------------------------------------------------------------
// PHY IC basic
//------------------------------------------------------------
void phy_basic_setting (MAC_ENGINE *eng) {
	phy_Read_Write( eng,  0, 0x7140, eng->phy.PHY_00h ); //clr set
#ifdef PHY_debug_set_clr
	if ( 1 ) {
#else
	if ( DbgPrn_PHYRW ) {
#endif
		printf("[Set]00: 0x%04lx (%02d:%08lx)\n", phy_read( eng, PHY_REG_BMCR ), eng->phy.Adr, eng->phy.PHY_BASE );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "[Set]00: 0x%04lx (%02d:%08lx)\n", phy_read( eng, PHY_REG_BMCR ), eng->phy.Adr, eng->phy.PHY_BASE );
	}
}

//------------------------------------------------------------
void phy_Wait_Reset_Done (MAC_ENGINE *eng) {
	int        timeout = 0;

	while (  phy_read( eng, PHY_REG_BMCR ) & 0x8000 ) {
		if (++timeout > TIME_OUT_PHY_Rst) {
			if ( !eng->run.TM_Burst )
				PRINTF( FP_LOG, "[PHY-Reset] Time out: %08lx\n", Read_Reg_PHY_DD( eng, 0x60 ) );

			FindErr( eng, Err_Flag_PHY_TimeOut_Rst );
			break;
		}
	}//wait Rst Done

#ifdef PHY_debug_set_clr
	if ( 1 ) {
#else
	if ( DbgPrn_PHYRW ) {
#endif
		printf("[Clr]00: 0x%04lx (%02d:%08lx)\n", phy_read( eng, PHY_REG_BMCR ), eng->phy.Adr, eng->phy.PHY_BASE );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "[Clr]00: 0x%04lx (%02d:%08lx)\n", phy_read( eng, PHY_REG_BMCR ), eng->phy.Adr, eng->phy.PHY_BASE );
	}
#ifdef Delay_PHYRst
	DELAY( Delay_PHYRst );
#endif
}

//------------------------------------------------------------
void phy_Reset (MAC_ENGINE *eng) {
	phy_basic_setting( eng );

//	phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
	phy_Read_Write( eng,  0, 0x7140, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
	phy_Wait_Reset_Done( eng );

	phy_basic_setting( eng );
#ifdef Delay_PHYRst
	DELAY( Delay_PHYRst );
#endif
}

//------------------------------------------------------------
void phy_check_register (MAC_ENGINE *eng, ULONG adr, ULONG check_mask, ULONG check_value, ULONG hit_number, char *runname) {
	USHORT     wait_phy_ready = 0;
	USHORT     hit_count = 0;

	while ( wait_phy_ready < 1000 ) {
		if ( (phy_read( eng, adr ) & check_mask) == check_value ) {
			if ( ++hit_count >= hit_number ) {
				break;
			}
			else {
				phy_delay(1);
			}
		} else {
			hit_count = 0;
			wait_phy_ready++;
			phy_delay(10);
		}
	}
	if ( hit_count < hit_number ) {
		printf("Timeout: %s\n", runname);
		PRINTF( FP_LOG, "Timeout: %s\n", runname);
	}
}

//------------------------------------------------------------
// PHY IC
//------------------------------------------------------------
void recov_phy_marvell (MAC_ENGINE *eng) {//88E1111
	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng,  9, eng->phy.PHY_09h );

			phy_Reset( eng );

			phy_write( eng, 29, 0x0007 );
			phy_Read_Write( eng, 30, 0x0008, 0x0000 );//clr set
			phy_write( eng, 29, 0x0010 );
			phy_Read_Write( eng, 30, 0x0002, 0x0000 );//clr set
			phy_write( eng, 29, 0x0012 );
			phy_Read_Write( eng, 30, 0x0001, 0x0000 );//clr set

			phy_write( eng, 18, eng->phy.PHY_12h );
		}
	}
}

//------------------------------------------------------------
void phy_marvell (MAC_ENGINE *eng) {//88E1111
//	int        Retry;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			eng->phy.PHY_09h = phy_read( eng, PHY_GBCR );
			eng->phy.PHY_12h = phy_read( eng, PHY_INER );
			phy_write( eng, 18, 0x0000 );
			phy_Read_Write( eng,  9, 0x0000, 0x1800 );//clr set
		}

		phy_Reset( eng );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 29, 0x0007 );
			phy_Read_Write( eng, 30, 0x0000, 0x0008 );//clr set
			phy_write( eng, 29, 0x0010 );
			phy_Read_Write( eng, 30, 0x0000, 0x0002 );//clr set
			phy_write( eng, 29, 0x0012 );
			phy_Read_Write( eng, 30, 0x0000, 0x0001 );//clr set
		}
	}

	if ( !eng->phy.loop_phy )
		phy_check_register ( eng, 17, 0x0400, 0x0400, 1, "wait 88E1111 link-up");
//	Retry = 0;
//	do {
//		eng->phy.PHY_11h = phy_read( eng, PHY_SR );
//	} while ( !( ( eng->phy.PHY_11h & 0x0400 ) | eng->phy.loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void recov_phy_marvell0 (MAC_ENGINE *eng) {//88E1310
	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 16, 0x0020, 0x0000 );//clr set
			phy_write( eng, 22, 0x0000 );
		}
	}
}

//------------------------------------------------------------
void phy_marvell0 (MAC_ENGINE *eng) {//88E1310
//	int        Retry;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_write( eng, 22, 0x0002 );

	eng->phy.PHY_15h = phy_read( eng, 21 );
	if ( eng->phy.PHY_15h & 0x0030 ) {
		printf("\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", eng->phy.PHY_15h);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", eng->phy.PHY_15h );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", eng->phy.PHY_15h );

		phy_write( eng, 21, eng->phy.PHY_15h & 0xffcf ); // Set [5]Rx Dly, [4]Tx Dly to 0
	}
	phy_write( eng, 22, 0x0000 );

	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng, 22, 0x0002 );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_Read_Write( eng, 21, 0x6040, 0x0040 );//clr set
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			phy_Read_Write( eng, 21, 0x6040, 0x2000 );//clr set
		}
		else {
			phy_Read_Write( eng, 21, 0x6040, 0x0000 );//clr set
		}
		phy_write( eng, 22, 0x0000 );
		phy_Reset(  eng  );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 16, 0x0000, 0x0020 );//clr set
			phy_write( eng, 22, 0x0000 );
		}

		phy_Reset( eng );
	}

	if ( !eng->phy.loop_phy )
		phy_check_register ( eng, 17, 0x0400, 0x0400, 1, "wait 88E1310 link-up");
//	Retry = 0;
//	do {
//		eng->phy.PHY_11h = phy_read( eng, PHY_SR );
//	} while ( !( ( eng->phy.PHY_11h & 0x0400 ) | eng->phy.loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void recov_phy_marvell1 (MAC_ENGINE *eng) {//88E6176
	CHAR       PHY_ADR_org;

	PHY_ADR_org = eng->phy.Adr;
	for ( eng->phy.Adr = 16; eng->phy.Adr <= 22; eng->phy.Adr++ ) {
		if ( eng->run.TM_Burst ) {
		}
		else {
			phy_write( eng,  6, eng->phy.PHY_06hA[eng->phy.Adr-16] );//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	for ( eng->phy.Adr = 21; eng->phy.Adr <= 22; eng->phy.Adr++ ) {
		phy_write( eng,  1, 0x0003 ); //01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
	}
	eng->phy.Adr = PHY_ADR_org;
}

//------------------------------------------------------------
void phy_marvell1 (MAC_ENGINE *eng) {//88E6176
//	ULONG      PHY_01h;
	CHAR       PHY_ADR_org;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		printf("This mode doesn't support in 88E6176.\n");
	} else {
		//The 88E6176 is switch with 7 Port(P0~P6) and the PHYAdr will be fixed at 0x10~0x16, and only P5/P6 can be connected to the MAC.
		//Therefor, the 88E6176 only can run the internal loopback.
		PHY_ADR_org = eng->phy.Adr;
		for ( eng->phy.Adr = 16; eng->phy.Adr <= 20; eng->phy.Adr++ ) {
			eng->phy.PHY_06hA[eng->phy.Adr-16] = phy_read( eng, PHY_ANER );
			phy_write( eng,  6, 0x0000 );//06h[5]P5 loopback, 06h[6]P6 loopback
		}

		for ( eng->phy.Adr = 21; eng->phy.Adr <= 22; eng->phy.Adr++ ) {
//			PHY_01h = phy_read( eng, PHY_REG_BMSR );
//			if      ( eng->run.Speed_sel[ 0 ] ) phy_write( eng,  1, (PHY_01h & 0xfffc) | 0x0002 );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
//			else if ( eng->run.Speed_sel[ 1 ] ) phy_write( eng,  1, (PHY_01h & 0xfffc) | 0x0001 );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
//			else                              phy_write( eng,  1, (PHY_01h & 0xfffc)          );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
			if      ( eng->run.Speed_sel[ 0 ] ) phy_write( eng,  1, 0x0002 );//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
			else if ( eng->run.Speed_sel[ 1 ] ) phy_write( eng,  1, 0x0001 );//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
			else                                phy_write( eng,  1, 0x0000 );//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.

			eng->phy.PHY_06hA[eng->phy.Adr-16] = phy_read( eng, PHY_ANER );
			if ( eng->phy.Adr == 21 ) phy_write( eng,  6, 0x0020 );//06h[5]P5 loopback, 06h[6]P6 loopback
			else                      phy_write( eng,  6, 0x0040 );//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	eng->phy.Adr = PHY_ADR_org;
}

//------------------------------------------------------------
void recov_phy_marvell2 (MAC_ENGINE *eng) {//88E1512//88E15 10/12/14/18
	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			// Enable Stub Test
			// switch page 6
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 18, 0x0008, 0x0000 );//clr set
			phy_write( eng, 22, 0x0000 );
		}
	}
}

//------------------------------------------------------------
void phy_marvell2 (MAC_ENGINE *eng) {//88E1512//88E15 10/12/14/18
//	int        Retry = 0;
//	ULONG      temp_reg;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

//	eng->run.TIME_OUT_Des_PHYRatio = 10;

	// switch page 2
	phy_write( eng, 22, 0x0002 );
	eng->phy.PHY_15h = phy_read( eng, 21 );
	if ( eng->phy.PHY_15h & 0x0030 ) {
		printf("\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", eng->phy.PHY_15h);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", eng->phy.PHY_15h );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", eng->phy.PHY_15h );

		phy_write( eng, 21, eng->phy.PHY_15h & 0xffcf );
	}
	phy_write( eng, 22, 0x0000 );


	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		// Internal loopback funciton only support in copper mode
		// switch page 18
		phy_write( eng, 22, 0x0012 );
		eng->phy.PHY_14h = phy_read( eng, 20 );
		// Change mode to Copper mode
//		if ( eng->phy.PHY_14h & 0x0020 ) {
		if ( ( eng->phy.PHY_14h & 0x003f ) != 0x0010 ) {
			printf("\n\n[Warning] Internal loopback funciton only support in copper mode[%04lx]\n\n", eng->phy.PHY_14h);
			if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Internal loopback funciton only support in copper mode[%04lx]\n\n", eng->phy.PHY_14h);
			if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Internal loopback funciton only support in copper mode[%04lx]\n\n", eng->phy.PHY_14h);

			phy_write( eng, 20, ( eng->phy.PHY_14h & 0xffc0 ) | 0x8010 );
			// do software reset
			phy_check_register ( eng, 20, 0x8000, 0x0000, 1, "wait 88E15 10/12/14/18 mode reset");
//			do {
//				temp_reg = phy_read( eng, 20 );
//			} while ( ( (temp_reg & 0x8000) == 0x8000 ) & (Retry++ < 20) );
		}

		// switch page 2
		phy_write( eng, 22, 0x0002 );
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_Read_Write( eng, 21, 0x2040, 0x0040 );//clr set
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			phy_Read_Write( eng, 21, 0x2040, 0x2000 );//clr set
		}
		else {
			phy_Read_Write( eng, 21, 0x2040, 0x0000 );//clr set
		}
		phy_write( eng, 22, 0x0000 );

		phy_Reset( eng );

		//Internal loopback at 100Mbps need delay 400~500 ms
//		DELAY( 400 );//Still fail at 100Mbps
//		DELAY( 500 );//All Pass
		if ( !eng->run.Speed_sel[ 0 ] ) {
			phy_check_register ( eng, 17, 0x0040, 0x0040, 10, "wait 88E15 10/12/14/18 link-up");
			phy_check_register ( eng, 17, 0x0040, 0x0000, 10, "wait 88E15 10/12/14/18 link-up");
			phy_check_register ( eng, 17, 0x0040, 0x0040, 10, "wait 88E15 10/12/14/18 link-up");
		}
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			// Enable Stub Test
			// switch page 6
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 18, 0x0000, 0x0008 );//clr set
			phy_write( eng, 22, 0x0000 );
		}

		phy_Reset( eng );
		phy_check_register ( eng, 17, 0x0400, 0x0400, 10, "wait 88E15 10/12/14/18 link-up");
	}

//	if ( !eng->phy.loop_phy )
////	if ( !eng->run.TM_Burst )
//		phy_check_register ( eng, 17, 0x0400, 0x0400, 10, "wait 88E15 10/12/14/18 link-up");
////	Retry = 0;
////	do {
////		eng->phy.PHY_11h = phy_read( eng, PHY_SR );
////	} while ( !( ( eng->phy.PHY_11h & 0x0400 ) | eng->phy.loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void phy_marvell3 (MAC_ENGINE *eng) {//88E3019
#ifdef PHY_debug
	if ( 1 ) {
#else
	if ( DbgPrn_PHYName ) {
#endif
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);
	}

	//Reg1ch[11:10]: MAC Interface Mode
	// 00 => RGMII where receive clock trnasitions when data transitions
	// 01 => RGMII where receive clock trnasitions when data is stable
	// 10 => RMII
	// 11 => MII
	eng->phy.PHY_1ch = phy_read( eng, 28 );
	if ( eng->env.MAC_RMII ) {
		if ( ( eng->phy.PHY_1ch & 0x0c00 ) != 0x0800 ) {
			printf("\n\n[Warning] Register 28, bit 10~11 must be 2 (RMII Mode)[Reg1ch:%04lx]\n\n", eng->phy.PHY_1ch);
			eng->phy.PHY_1ch = ( eng->phy.PHY_1ch & 0xf3ff ) | 0x0800;
			phy_write( eng, 28, eng->phy.PHY_1ch );
//			phy_write( eng,  0, phy_read( eng,  0 ) | 0x8000 );
//			phy_Wait_Reset_Done( eng );
		}
	} else {
		if ( ( eng->phy.PHY_1ch & 0x0c00 ) != 0x0000 ) {
			printf("\n\n[Warning] Register 28, bit 10~11 must be 0 (RGMIIRX Edge-align Mode)[Reg1ch:%04lx]\n\n", eng->phy.PHY_1ch);
			eng->phy.PHY_1ch = ( eng->phy.PHY_1ch & 0xf3ff ) | 0x0000;
			phy_write( eng, 28, eng->phy.PHY_1ch );
//			phy_write( eng,  0, phy_read( eng,  0 ) | 0x8000 );
//			phy_Wait_Reset_Done( eng );
		}
	}

	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		phy_Reset( eng );
	}

	phy_check_register ( eng, 17, 0x0400, 0x0400, 1, "wait 88E3019 link-up");
}

//------------------------------------------------------------
void phy_broadcom (MAC_ENGINE *eng) {//BCM5221
    ULONG      reg;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Broadcom] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );

	if ( eng->run.TM_IEEE ) {
		if ( eng->arg.GIEEE_sel == 0 ) {
			phy_write( eng, 25, 0x1f01 );//Force MDI  //Measuring from channel A
		}
		else {
			phy_Read_Write( eng, 24, 0x0000, 0x4000 );//clr set//Force Link
//			phy_write( eng,  0, eng->phy.PHY_00h );
//			phy_write( eng, 30, 0x1000 );
		}
	}
	else
	{
		// we can check link status from register 0x18
		if ( eng->run.Speed_sel[ 1 ] ) {
		    do {
			reg = phy_read( eng, 0x18 ) & 0xF;
		    } while ( reg != 0x7 );
		}
		else {
		    do {
			reg = phy_read( eng, 0x18 ) & 0xF;
		    } while ( reg != 0x1 );
		}
	}
}

//------------------------------------------------------------
void recov_phy_broadcom0 (MAC_ENGINE *eng) {//BCM54612
	phy_write( eng,  0, eng->phy.PHY_00h );
	phy_write( eng,  9, eng->phy.PHY_09h );
//	phy_write( eng, 24, eng->phy.PHY_18h | 0xf007 );//write reg 18h, shadow value 111
//	phy_write( eng, 28, eng->phy.PHY_1ch | 0x8c00 );//write reg 1Ch, shadow value 00011

	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng,  0, eng->phy.PHY_00h );
	}
	else {
	}
}

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: Don't support(?)
//internal loop 10M : Don't support(?)
void phy_broadcom0 (MAC_ENGINE *eng) {//BCM54612
	ULONG      PHY_new;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Broadcom] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	eng->phy.PHY_00h = phy_read( eng, PHY_REG_BMCR );
	eng->phy.PHY_09h = phy_read( eng, PHY_GBCR );
	phy_write( eng, 24, 0x7007 );//read reg 18h, shadow value 111
	eng->phy.PHY_18h = phy_read( eng, 24 );
	phy_write( eng, 28, 0x0c00 );//read reg 1Ch, shadow value 00011
	eng->phy.PHY_1ch = phy_read( eng, 28 );

	if ( eng->phy.PHY_18h & 0x0100 ) {
		PHY_new = ( eng->phy.PHY_18h & 0x0af0 ) | 0xf007;
		printf("\n\n[Warning] Shadow value 111, Register 24, bit 8 must be 0 [Reg18h_7:%04lx->%04lx]\n\n", eng->phy.PHY_18h, PHY_new);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Shadow value 111, Register 24, bit 8 must be 0 [Reg18h_7:%04lx->%04lx]\n\n", eng->phy.PHY_18h, PHY_new );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Shadow value 111, Register 24, bit 8 must be 0 [Reg18h_7:%04lx->%04lx]\n\n", eng->phy.PHY_18h, PHY_new );

		phy_write( eng, 24, PHY_new ); // Disable RGMII RXD to RXC Skew
	}
	if ( eng->phy.PHY_1ch & 0x0200 ) {
		PHY_new = ( eng->phy.PHY_1ch & 0x0000 ) | 0x8c00;
		printf("\n\n[Warning] Shadow value 00011, Register 28, bit 9 must be 0 [Reg1ch_3:%04lx->%04lx]\n\n", eng->phy.PHY_1ch, PHY_new);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Shadow value 00011, Register 28, bit 9 must be 0 [Reg1ch_3:%04lx->%04lx]\n\n", eng->phy.PHY_1ch, PHY_new );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Shadow value 00011, Register 28, bit 9 must be 0 [Reg1ch_3:%04lx->%04lx]\n\n", eng->phy.PHY_1ch, PHY_new );

		phy_write( eng, 28, PHY_new );// Disable GTXCLK Clock Delay Enable
	}

	if ( eng->run.TM_Burst ) {
		phy_basic_setting( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_basic_setting( eng );

		// Enable Internal Loopback mode
		// Page 58, BCM54612EB1KMLG_Spec.pdf
		phy_write( eng,  0, 0x5140 );
#ifdef Delay_PHYRst
		phy_delay( Delay_PHYRst );
#endif
		/* Only 1G Test is PASS, 100M and 10M is false @20130619 */

// Waiting for BCM FAE's response
//		if ( eng->run.Speed_sel[ 0 ] ) {
//			// Speed 1G
//			// Enable Internal Loopback mode
//			// Page 58, BCM54612EB1KMLG_Spec.pdf
//			phy_write( eng,  0, 0x5140 );
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {
//			// Speed 100M
//			// Enable Internal Loopback mode
//			// Page 58, BCM54612EB1KMLG_Spec.pdf
//			phy_write( eng,  0, 0x7100 );
//			phy_write( eng, 30, 0x1000 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {
//			// Speed 10M
//			// Enable Internal Loopback mode
//			// Page 58, BCM54612EB1KMLG_Spec.pdf
//			phy_write( eng,  0, 0x5100 );
//			phy_write( eng, 30, 0x1000 );
//		}
//
#ifdef Delay_PHYRst
//		phy_delay( Delay_PHYRst );
#endif
	}
	else {

		if ( eng->run.Speed_sel[ 0 ] ) {
			// Page 60, BCM54612EB1KMLG_Spec.pdf
			// need to insert loopback plug
			phy_write( eng,  9, 0x1800 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 24, 0x8400 ); // Enable Transmit test mode
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			// Page 60, BCM54612EB1KMLG_Spec.pdf
			// need to insert loopback plug
			phy_write( eng,  0, 0x2100 );
			phy_write( eng, 24, 0x8400 ); // Enable Transmit test mode
		}
		else {
			// Page 60, BCM54612EB1KMLG_Spec.pdf
			// need to insert loopback plug
			phy_write( eng,  0, 0x0100 );
			phy_write( eng, 24, 0x8400 ); // Enable Transmit test mode
		}
	}
}

//------------------------------------------------------------
void phy_realtek (MAC_ENGINE *eng) {//RTL8201N
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );
}

//------------------------------------------------------------
//internal loop 100M: Don't support
//internal loop 10M : no  loopback stub
void phy_realtek0 (MAC_ENGINE *eng) {//RTL8201E
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );

//	phy_Read_Write( eng, 25, 0x2800, 0x0000 );//clr set
//	printf("Enable phy output RMII clock\n");
	if ( eng->run.TM_IEEE ) {
		phy_write( eng, 31, 0x0001 );
		if ( eng->arg.GIEEE_sel == 0 ) {
			phy_write( eng, 25, 0x1f01 );//Force MDI  //Measuring from channel A
		}
		else {
			phy_write( eng, 25, 0x1f00 );//Force MDIX //Measuring from channel B
		}
		phy_write( eng, 31, 0x0000 );
	}
}

//------------------------------------------------------------
void recov_phy_realtek1 (MAC_ENGINE *eng) {//RTL8211D
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {//Test Mode 1
					//Rev 1.2
					phy_write( eng, 31, 0x0002 );
					phy_write( eng,  2, 0xc203 );
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x0000 );
				}
				else {//Test Mode 4
					//Rev 1.2
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x0000 );
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				//Rev 1.2
				phy_write( eng, 23, 0x2100 );
				phy_write( eng, 16, 0x016e );
			}
			else {
				//Rev 1.2
				phy_write( eng, 31, 0x0006 );
				phy_write( eng,  0, 0x5a00 );
				phy_write( eng, 31, 0x0000 );
			}
		} else {
			phy_Reset( eng );
		} // End if ( eng->run.TM_IEEE )
	}
	else if ( eng->phy.loop_phy ) {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0000 ); // new in Rev. 1.6
			phy_write( eng,  0, 0x1140 ); // new in Rev. 1.6
			phy_write( eng, 20, 0x8040 ); // new in Rev. 1.6
		}
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  3, 0xdf41 );
			phy_write( eng,  2, 0xdf20 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 26, 0x0020 ); // Rev. 1.2
			phy_write( eng, 26, 0x0040 ); // new in Rev. 1.6
			phy_write( eng,  0, 0x1140 );
//			phy_write( eng, 21, 0x0006 ); // Rev. 1.2
			phy_write( eng, 21, 0x1006 ); // new in Rev. 1.6
			phy_write( eng, 23, 0x2100 );
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  0, 0x1200 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
		}
		phy_Reset( eng );
		phy_delay(2000);
	} // End if ( eng->run.TM_Burst )
} // End void recov_phy_realtek1 (MAC_ENGINE *eng)

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek1 (MAC_ENGINE *eng) {//RTL8211D
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {//Test Mode 1
					//Rev 1.2
					phy_write( eng, 31, 0x0002 );
					phy_write( eng,  2, 0xc22b );
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x2000 );
				}
				else {//Test Mode 4
					//Rev 1.2
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x8000 );
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {//From Channel A
					//Rev 1.2
					phy_write( eng, 23, 0xa102 );
					phy_write( eng, 16, 0x01ae );//MDI
				}
				else {//From Channel B
					//Rev 1.2
					phy_Read_Write( eng, 17, 0x0008, 0x0000 ); // clr set
					phy_write( eng, 23, 0xa102 );         // MDI
					phy_write( eng, 16, 0x010e );
				}
			}
			else {
				if ( eng->arg.GIEEE_sel == 0 ) {//Diff. Voltage/TP-IDL/Jitter: Pseudo-random pattern
					phy_write( eng, 31, 0x0006 );
					phy_write( eng,  0, 0x5a21 );
					phy_write( eng, 31, 0x0000 );
				}
				else if ( eng->arg.GIEEE_sel == 1 ) {//Harmonic: pattern
					phy_write( eng, 31, 0x0006 );
					phy_write( eng,  2, 0x05ee );
					phy_write( eng,  0, 0xff21 );
					phy_write( eng, 31, 0x0000 );
				}
				else {//Harmonic: pattern
					phy_write( eng, 31, 0x0006 );
					phy_write( eng,  2, 0x05ee );
					phy_write( eng,  0, 0x0021 );
					phy_write( eng, 31, 0x0000 );
				}
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 20, 0x0042 );//new in Rev. 1.6
		}
	}
	else {
        // refer to RTL8211D Register for Manufacture Test_V1.6.pdf
        // MDI loop back
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  3, 0xff41 );
			phy_write( eng,  2, 0xd720 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0040 );
			phy_write( eng, 24, 0x0008 );

			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  9, 0x0300 );
			phy_write( eng, 26, 0x0020 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 23, 0xa101 );
			phy_write( eng, 21, 0x0200 );
			phy_write( eng, 23, 0xa121 );
			phy_write( eng, 23, 0xa161 );
			phy_write( eng,  0, 0x8000 );
			phy_Wait_Reset_Done( eng );

//			phy_delay(200); // new in Rev. 1.6
			phy_delay(5000); // 20150504
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x0061 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(5000);
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(5000);
		}
		else {
			phy_Reset( eng );
		}
	}
} // End void phy_realtek1 (MAC_ENGINE *eng)

//------------------------------------------------------------
void recov_phy_realtek2 (MAC_ENGINE *eng) {//RTL8211E
#ifdef Realtek_debug
printf ("\nClear RTL8211E [Start] =====>\n");
#endif
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				//Rev 1.2
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  9, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				//Rev 1.2
				phy_write( eng, 31, 0x0007 );
				phy_write( eng, 30, 0x002f );
				phy_write( eng, 23, 0xd88f );
				phy_write( eng, 30, 0x002d );
				phy_write( eng, 24, 0xf050 );
				phy_write( eng, 31, 0x0000 );
				phy_write( eng, 16, 0x006e );
			}
			else {
				//Rev 1.2
				phy_write( eng, 31, 0x0006 );
				phy_write( eng,  0, 0x5a00 );
				phy_write( eng, 31, 0x0000 );
			}
			//Rev 1.2
			phy_write( eng, 31, 0x0005 );
			phy_write( eng,  5, 0x8b86 );
			phy_write( eng,  6, 0xe201 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0020 );
			phy_write( eng, 21, 0x1108 );
			phy_write( eng, 31, 0x0000 );
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			//Rev 1.5  //not stable
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x8000 );
//			phy_Wait_Reset_Done( eng );
//			phy_delay(30);
//			phy_write( eng, 23, 0x2160 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0040 );
//			phy_write( eng, 24, 0x0004 );
//			phy_write( eng, 24, 0x1a24 );
//			phy_write( eng, 25, 0xfd00 );
//			phy_write( eng, 24, 0x0000 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x1140 );
//			phy_write( eng, 26, 0x0040 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x002f );
//			phy_write( eng, 23, 0xd88f );
//			phy_write( eng, 30, 0x0023 );
//			phy_write( eng, 22, 0x0300 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 21, 0x1006 );
//			phy_write( eng, 23, 0x2100 );

			//Rev 1.6
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x8000 );
#ifdef Realtek_debug
#else
			phy_Wait_Reset_Done( eng );
			phy_delay(30);
#endif

			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0042 );
			phy_write( eng, 21, 0x0500 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x1140 );
			phy_write( eng, 26, 0x0040 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x002f );
			phy_write( eng, 23, 0xd88f );
			phy_write( eng, 30, 0x0023 );
			phy_write( eng, 22, 0x0300 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 21, 0x1006 );
			phy_write( eng, 23, 0x2100 );
		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  0, 0x1200 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
//		}
		else {
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x1140 );
		}
#ifdef Realtek_debug
#else
		// Check register 0x11 bit10 Link OK or not OK
		phy_check_register ( eng, 17, 0x0c02, 0x0000, 10, "clear RTL8211E");
#endif
	}
#ifdef Realtek_debug
printf ("\nClear RTL8211E [End] =====>\n");
#endif
} // End void recov_phy_realtek2 (MAC_ENGINE *eng)

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek2 (MAC_ENGINE *eng) {//RTL8211E
	USHORT     check_value;
#ifdef Realtek_debug
printf ("\nSet RTL8211E [Start] =====>\n");
GPIO_20h_Value = Read_Reg_GPIO_DD( 0x20 );
GPIO_24h_Value = Read_Reg_GPIO_DD( 0x24 ) | 0x00400000;

Write_Reg_GPIO_DD( 0x24, GPIO_24h_Value );
#endif
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

#ifdef Realtek_debug
#else
	phy_write( eng, 31, 0x0000 );
	phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h ); // clr set // Rst PHY
	phy_Wait_Reset_Done( eng );
	phy_delay(30);
#endif

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			//Rev 1.2
			phy_write( eng, 31, 0x0005 );
			phy_write( eng,  5, 0x8b86 );
			phy_write( eng,  6, 0xe200 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0020 );
			phy_write( eng, 21, 0x0108 );
			phy_write( eng, 31, 0x0000 );

			if ( eng->run.Speed_sel[ 0 ] ) {
				//Rev 1.2
				phy_write( eng, 31, 0x0000 );

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng,  9, 0x2000 );//Test Mode 1
				}
				else {
					phy_write( eng,  9, 0x8000 );//Test Mode 4
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				//Rev 1.2
				phy_write( eng, 31, 0x0007 );
				phy_write( eng, 30, 0x002f );
				phy_write( eng, 23, 0xd818 );
				phy_write( eng, 30, 0x002d );
				phy_write( eng, 24, 0xf060 );
				phy_write( eng, 31, 0x0000 );

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng, 16, 0x00ae );//From Channel A
				}
				else {
					phy_write( eng, 16, 0x008e );//From Channel B
				}
			}
			else {
				//Rev 1.2
				phy_write( eng, 31, 0x0006 );
				if ( eng->arg.GIEEE_sel == 0 ) {//Diff. Voltage/TP-IDL/Jitter
					phy_write( eng,  0, 0x5a21 );
				}
				else if ( eng->arg.GIEEE_sel == 1 ) {//Harmonic: pattern
					phy_write( eng,  2, 0x05ee );
					phy_write( eng,  0, 0xff21 );
				}
				else {//Harmonic: pattern
					phy_write( eng,  2, 0x05ee );
					phy_write( eng,  0, 0x0021 );
				}
				phy_write( eng, 31, 0x0000 );
			}
		}
		else {
			phy_basic_setting( eng );
			phy_delay(30);
		}
	}
	else if ( eng->phy.loop_phy ) {
#ifdef Realtek_debug
		phy_write( eng,  0, 0x0000 );
		phy_write( eng,  0, 0x8000 );
		phy_delay(60);
		phy_write( eng,  0, eng->phy.PHY_00h );
		phy_delay(60);
#else
		phy_basic_setting( eng );

		phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
		phy_Wait_Reset_Done( eng );
		phy_delay(30);

		phy_basic_setting( eng );
		phy_delay(30);
#endif
	}
	else {
#ifdef Enable_Dual_Mode
		if ( eng->run.Speed_sel[ 0 ] ) {
			check_value = 0x0c02 | 0xa000;
			//set GPIO
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			check_value = 0x0c02 | 0x6000;
			//set GPIO
		}
		else if ( eng->run.Speed_sel[ 2 ] ) {
			check_value = 0x0c02 | 0x2000;
			//set GPIO
		}
#else
		if ( eng->run.Speed_sel[ 0 ] ) {
			check_value = 0x0c02 | 0xa000;
			//Rev 1.5  //not stable
//			phy_write( eng, 23, 0x2160 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0040 );
//			phy_write( eng, 24, 0x0004 );
//			phy_write( eng, 24, 0x1a24 );
//			phy_write( eng, 25, 0x7d00 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 23, 0x2100 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0040 );
//			phy_write( eng, 24, 0x0000 );
//			phy_write( eng, 30, 0x0023 );
//			phy_write( eng, 22, 0x0006 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x0140 );
//			phy_write( eng, 26, 0x0060 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x002f );
//			phy_write( eng, 23, 0xd820 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 21, 0x0206 );
//			phy_write( eng, 23, 0x2120 );
//			phy_write( eng, 23, 0x2160 );

			//Rev 1.6
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x8000 );
//			phy_Wait_Reset_Done( eng );
//			phy_delay(30);
  #ifdef Realtek_debug
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x8000 );
			phy_delay(60);
  #endif

			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0042 );
			phy_write( eng, 21, 0x2500 );
			phy_write( eng, 30, 0x0023 );
			phy_write( eng, 22, 0x0006 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 26, 0x0060 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x002f );
			phy_write( eng, 23, 0xd820 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 21, 0x0206 );
			phy_write( eng, 23, 0x2120 );
			phy_write( eng, 23, 0x2160 );
  #ifdef Realtek_debug
			phy_delay(600);
  #else
			phy_delay(300);
  #endif
		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			check_value = 0x0c02 | 0x6000;
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(6000);
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			check_value = 0x0c02 | 0x2000;
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x0061 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(6000);
//		}
		else {
			if ( eng->run.Speed_sel[ 1 ] )
				check_value = 0x0c02 | 0x6000;
			else
				check_value = 0x0c02 | 0x2000;
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, eng->phy.PHY_00h );
  #ifdef Realtek_debug
			phy_delay(300);
  #else
			phy_delay(150);
  #endif
		}
#endif
#ifdef Realtek_debug
#else
		// Check register 0x11 bit10 Link OK or not OK
		phy_check_register ( eng, 17, 0x0c02 | 0xe000, check_value, 10, "set RTL8211E");
#endif
	}
#ifdef Realtek_debug
printf ("\nSet RTL8211E [End] =====>\n");
#endif
} // End void phy_realtek2 (MAC_ENGINE *eng)

//------------------------------------------------------------
void recov_phy_realtek3 (MAC_ENGINE *eng) {//RTL8211C
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				phy_write( eng,  9, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng, 17, eng->phy.PHY_11h );
				phy_write( eng, 14, 0x0000 );
				phy_write( eng, 16, 0x00a0 );
			}
			else {
//				phy_write( eng, 31, 0x0006 );
//				phy_write( eng,  0, 0x5a00 );
//				phy_write( eng, 31, 0x0000 );
			}
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 11, 0x0000 );
		}
		phy_write( eng, 12, 0x1006 );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  4, 0xb000 );
			phy_write( eng,  3, 0xff41 );
			phy_write( eng,  2, 0xdf20 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 25, 0x8c00 );
			phy_write( eng, 26, 0x0040 );
			phy_write( eng,  0, 0x1140 );
			phy_write( eng, 14, 0x0000 );
			phy_write( eng, 12, 0x1006 );
			phy_write( eng, 23, 0x2109 );
		}
	}
}

//------------------------------------------------------------
void phy_realtek3 (MAC_ENGINE *eng) {//RTL8211C
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {   //Test Mode 1
					phy_write( eng,  9, 0x2000 );
				}
				else if ( eng->arg.GIEEE_sel == 1 ) {//Test Mode 2
					phy_write( eng,  9, 0x4000 );
				}
				else if ( eng->arg.GIEEE_sel == 2 ) {//Test Mode 3
					phy_write( eng,  9, 0x6000 );
				}
				else {                           //Test Mode 4
					phy_write( eng,  9, 0x8000 );
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				eng->phy.PHY_11h = phy_read( eng, PHY_SR );
				phy_write( eng, 17, eng->phy.PHY_11h & 0xfff7 );
				phy_write( eng, 14, 0x0660 );

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng, 16, 0x00a0 );//MDI  //From Channel A
				}
				else {
					phy_write( eng, 16, 0x0080 );//MDIX //From Channel B
				}
			}
			else {
//				if ( eng->arg.GIEEE_sel == 0 ) {//Pseudo-random pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  0, 0x5a21 );
//					phy_write( eng, 31, 0x0000 );
//				}
//				else if ( eng->arg.GIEEE_sel == 1 ) {//pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  2, 0x05ee );
//					phy_write( eng,  0, 0xff21 );
//					phy_write( eng, 31, 0x0000 );
//				}
//				else {//pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  2, 0x05ee );
//					phy_write( eng,  0, 0x0021 );
//					phy_write( eng, 31, 0x0000 );
//				}
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng,  0, 0x9200 );
		phy_Wait_Reset_Done( eng );
		phy_delay(30);

		phy_write( eng, 17, 0x401c );
		phy_write( eng, 12, 0x0006 );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 11, 0x0002 );
		}
		else {
			phy_basic_setting( eng );
		}
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  4, 0xb000 );
			phy_write( eng,  3, 0xff41 );
			phy_write( eng,  2, 0xd720 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 25, 0x8400 );
			phy_write( eng, 26, 0x0020 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 14, 0x0210 );
			phy_write( eng, 12, 0x0200 );
			phy_write( eng, 23, 0x2109 );
			phy_write( eng, 23, 0x2139 );
		}
		else {
			phy_Reset( eng );
		}
	}
} // End void phy_realtek3 (MAC_ENGINE *eng)

//------------------------------------------------------------
//external loop 100M: OK
//external loop 10M : OK
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek4 (MAC_ENGINE *eng) {//RTL8201F
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_write( eng, 31, 0x0007 );
	eng->phy.PHY_10h = phy_read( eng, 16 );
	if ( ( eng->phy.PHY_10h & 0x0008 ) == 0x0 ) {
		phy_write( eng, 16, eng->phy.PHY_10h | 0x0008 );
		printf("\n\n[Warning] Page 7 Register 16, bit 3 must be 1 [Reg10h_7:%04lx]\n\n", eng->phy.PHY_10h);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Page 7 Register 16, bit 3 must be 1 [Reg10h_7:%04lx]\n\n", eng->phy.PHY_10h );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Page 7 Register 16, bit 3 must be 1 [Reg10h_7:%04lx]\n\n", eng->phy.PHY_10h );
	}
	phy_write( eng, 31, 0x0000 );

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			//Rev 1.0
			phy_write( eng, 31, 0x0004 );
			phy_write( eng, 16, 0x4077 );
			phy_write( eng, 21, 0xc5a0 );
			phy_write( eng, 31, 0x0000 );

			if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng,  0, 0x8000 ); // Reset PHY
				phy_Wait_Reset_Done( eng );
				phy_write( eng, 24, 0x0310 ); // Disable ALDPS

				if ( eng->arg.GIEEE_sel == 0 ) {//From Channel A (RJ45 pair 1, 2)
					phy_write( eng, 28, 0x40c2 ); //Force MDI
				}
				else {//From Channel B (RJ45 pair 3, 6)
					phy_write( eng, 28, 0x40c0 ); //Force MDIX
				}
				phy_write( eng,  0, 0x2100 );       //Force 100M/Full Duplex)
			} else {
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		// Internal loopback
		if ( eng->run.Speed_sel[ 1 ] ) {
			// Enable 100M PCS loop back; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x6100 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 16, 0x1FF8 );
			phy_write( eng, 16, 0x0FF8 );
			phy_write( eng, 31, 0x0000 );
			phy_delay(20);
		} else if ( eng->run.Speed_sel[ 2 ] ) {
			// Enable 10M PCS loop back; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x4100 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 16, 0x1FF8 );
			phy_write( eng, 16, 0x0FF8 );
			phy_write( eng, 31, 0x0000 );
			phy_delay(20);
		}
	}
	else {
		// External loopback
		if ( eng->run.Speed_sel[ 1 ] ) {
			// Enable 100M MDI loop back Nway option; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  4, 0x01E1 );
			phy_write( eng,  0, 0x1200 );
		} else if ( eng->run.Speed_sel[ 2 ] ) {
			// Enable 10M MDI loop back Nway option; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  4, 0x0061 );
			phy_write( eng,  0, 0x1200 );
		}
//		phy_write( eng,  0, 0x8000 );
//		while ( phy_read( eng, 0 ) != 0x3100 ) {}
//		while ( phy_read( eng, 0 ) != 0x3100 ) {}
//		phy_write( eng,  0, eng->phy.PHY_00h );
////		phy_delay(100);
//		phy_delay(400);

		// Check register 0x1 bit2 Link OK or not OK
		phy_check_register ( eng, 1, 0x0004, 0x0004, 10, "set RTL8201F");
		phy_delay(300);
	}
}

//------------------------------------------------------------
void recov_phy_realtek5 (MAC_ENGINE *eng) {//RTL8211F
#ifdef Realtek_debug
printf ("\nClear RTL8211F [Start] =====>\n");
#endif
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				//Rev 1.0
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  9, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				//Rev 1.0
				phy_write( eng, 31, 0x0000 );
				phy_write( eng, 24, 0x2118 );//RGMII
				phy_write( eng,  9, 0x0200 );
				phy_write( eng,  0, 0x9200 );
				phy_Wait_Reset_Done( eng );
			}
			else {
				//Rev 1.0
				phy_write( eng, 31, 0x0c80 );
				phy_write( eng, 16, 0x5a00 );
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  4, 0x01e1 );
				phy_write( eng,  9, 0x0200 );
				phy_write( eng,  0, 0x9200 );
				phy_Wait_Reset_Done( eng );
			}
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			//Rev 1.1
			phy_write( eng, 31, 0x0a43 );
			phy_write( eng, 24, 0x2118 );
			phy_write( eng,  0, 0x1040 );
		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  0, 0x1200 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  4, 0x01e1 );
//			phy_write( eng,  0, 0x1200 );
//		}
		else {
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x1040 );
		}

#ifdef Realtek_debug
#else
		// Check register 0x1A bit2 Link OK or not OK
		phy_write( eng, 31, 0x0a43 );
		phy_check_register ( eng, 26, 0x0004, 0x0000, 10, "clear RTL8211F");
		phy_write( eng, 31, 0x0000 );
#endif
	}
#ifdef Realtek_debug
printf ("\nClear RTL8211F [End] =====>\n");
#endif
}

//------------------------------------------------------------
void phy_realtek5 (MAC_ENGINE *eng) {//RTL8211F
	USHORT     check_value;
#ifdef Realtek_debug
printf ("\nSet RTL8211F [Start] =====>\n");
#endif
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				//Rev 1.0
				phy_write( eng, 31, 0x0000 );
				if ( eng->arg.GIEEE_sel == 0 ) {//Test Mode 1
					phy_write( eng,  9, 0x0200 );
				}
				else if ( eng->arg.GIEEE_sel == 1 ) {//Test Mode 2
					phy_write( eng,  9, 0x0400 );
				}
				else {//Test Mode 4
					phy_write( eng,  9, 0x0800 );
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {//option
				//Rev 1.0
				phy_write( eng, 31, 0x0000 );
				if ( eng->arg.GIEEE_sel == 0 ) {//Output MLT-3 from Channel A
					phy_write( eng, 24, 0x2318 );
				}
				else {//Output MLT-3 from Channel B
					phy_write( eng, 24, 0x2218 );
				}
				phy_write( eng,  9, 0x0000 );
				phy_write( eng,  0, 0x2100 );
			}
			else {
				//Rev 1.0
				//0: For Diff. Voltage/TP-IDL/Jitter with EEE
				//1: For Diff. Voltage/TP-IDL/Jitter without EEE
				//2: For Harmonic (all "1" patten) with EEE
				//3: For Harmonic (all "1" patten) without EEE
				//4: For Harmonic (all "0" patten) with EEE
				//5: For Harmonic (all "0" patten) without EEE
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  9, 0x0000 );
				phy_write( eng,  4, 0x0061 );
				if ( (eng->arg.GIEEE_sel & 0x1) == 0 ) {//with EEE
					phy_write( eng, 25, 0x0853 );
				}
				else {//without EEE
					phy_write( eng, 25, 0x0843 );
				}
				phy_write( eng,  0, 0x9200 );
				phy_Wait_Reset_Done( eng );

				if ( (eng->arg.GIEEE_sel & 0x6) == 0 ) {//For Diff. Voltage/TP-IDL/Jitter
					phy_write( eng, 31, 0x0c80 );
					phy_write( eng, 18, 0x0115 );
					phy_write( eng, 16, 0x5a21 );
				}
				else if ( (eng->arg.GIEEE_sel & 0x6) == 0x2 ) {//For Harmonic (all "1" patten)
					phy_write( eng, 31, 0x0c80 );
					phy_write( eng, 18, 0x0015 );
					phy_write( eng, 16, 0xff21 );
				}
				else {//For Harmonic (all "0" patten)
					phy_write( eng, 31, 0x0c80 );
					phy_write( eng, 18, 0x0015 );
					phy_write( eng, 16, 0x0021 );
				}
				phy_write( eng, 31, 0x0000 );
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			check_value = 0x0004 | 0x0028;
			//Rev 1.1
			phy_write( eng, 31, 0x0a43 );
			phy_write( eng,  0, 0x8000 );
#ifdef Realtek_debug
			phy_delay(60);
#else
			phy_Wait_Reset_Done( eng );
			phy_delay(30);
#endif

			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 24, 0x2d18 );
#ifdef Realtek_debug
			phy_delay(600);
#else
			phy_delay(300);
#endif
		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			check_value = 0x0004 | 0x0018;
//			phy_write( eng, 31, 0x0a43 );
//			phy_write( eng,  0, 0x8000 );
//			phy_Wait_Reset_Done( eng );
//			phy_delay(30);
//
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x01e1 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(6000);
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			check_value = 0x0004 | 0x0008;
//			phy_write( eng, 31, 0x0a43 );
//			phy_write( eng,  0, 0x8000 );
//			phy_Wait_Reset_Done( eng );
//			phy_delay(30);
//
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x0061 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(6000);
//		}
		else {
			if ( eng->run.Speed_sel[ 1 ] )
				check_value = 0x0004 | 0x0018;
			else
				check_value = 0x0004 | 0x0008;
#ifdef Realtek_debug
#else
			phy_write( eng, 31, 0x0a43 );
			phy_write( eng,  0, 0x8000 );
			phy_Wait_Reset_Done( eng );
			phy_delay(30);
#endif

			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, eng->phy.PHY_00h );
#ifdef Realtek_debug
			phy_delay(300);
#else
			phy_delay(150);
#endif
		}

#ifdef Realtek_debug
#else
		// Check register 0x1A bit2 Link OK or not OK
		phy_write( eng, 31, 0x0a43 );
		phy_check_register ( eng, 26, 0x0004 | 0x0038, check_value, 10, "set RTL8211F");
		phy_write( eng, 31, 0x0000 );
#endif
	}
#ifdef Realtek_debug
printf ("\nSet RTL8211F [End] =====>\n");
#endif
}

//------------------------------------------------------------
//It is a LAN Switch, only support 1G internal loopback test.
void phy_realtek6 (MAC_ENGINE *eng) {//RTL8363S
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		printf("This mode doesn't support in RTL8363S.\n");
	}
	else if ( eng->phy.loop_phy ) {

		// RXDLY2 and TXDLY2 of RTL8363S should set to LOW
		phy_basic_setting( eng );

		phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
		phy_Wait_Reset_Done( eng );
		phy_delay(30);

		phy_basic_setting( eng );
		phy_delay(30);
	}
	else {
		printf("This mode doesn't support in RTL8363S\n");
	}
} // End void phy_realtek6 (MAC_ENGINE *eng)

//------------------------------------------------------------
void phy_smsc (MAC_ENGINE *eng) {//LAN8700
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[SMSC] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );
}

//------------------------------------------------------------
void phy_micrel (MAC_ENGINE *eng) {//KSZ8041
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Micrel] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );

//	phy_write( eng, 24, 0x0600 );
}

//------------------------------------------------------------
void phy_micrel0 (MAC_ENGINE *eng) {//KSZ8031/KSZ8051
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Micrel] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	//For KSZ8051RNL only
	//Reg1Fh[7] = 0(default): 25MHz Mode, XI, XO(pin 9, 8) is 25MHz(crystal/oscilator).
	//Reg1Fh[7] = 1         : 50MHz Mode, XI(pin 9) is 50MHz(oscilator).
	eng->phy.PHY_1fh = phy_read( eng, 31 );
	if ( eng->phy.PHY_1fh & 0x0080 ) sprintf(eng->phy.PHYName, "%s-50MHz Mode", eng->phy.PHYName);
	else                             sprintf(eng->phy.PHYName, "%s-25MHz Mode", eng->phy.PHYName);

	if ( eng->run.TM_IEEE ) {
		phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
		phy_Wait_Reset_Done( eng );

		phy_Read_Write( eng, 31, 0x0000, 0x2000 );//clr set//1Fh[13] = 1: Disable auto MDI/MDI-X
		phy_basic_setting( eng );
		phy_Read_Write( eng, 31, 0x0000, 0x0800 );//clr set//1Fh[11] = 1: Force link pass

//		phy_delay(2500);//2.5 sec
	}
	else {
		phy_Reset( eng );

		//Reg16h[6] = 1         : RMII B-to-B override
		//Reg16h[1] = 1(default): RMII override
		phy_Read_Write( eng, 22, 0x0000, 0x0042 );//clr set
	}

	if ( eng->phy.PHY_1fh & 0x0080 )
		phy_Read_Write( eng, 31, 0x0000, 0x0080 );//clr set//Reset PHY will clear Reg1Fh[7]
}

//------------------------------------------------------------
//external loop 1G  : NOT Support
//external loop 100M: OK
//external loop 10M : OK
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_micrel1 (MAC_ENGINE *eng) {//KSZ9031
//	int        temp;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Micrel] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

/*
	phy_write( eng, 13, 0x0002 );
	phy_write( eng, 14, 0x0004 );
	phy_write( eng, 13, 0x4002 );
	temp = phy_read( eng, 14 );
	//Reg2.4[ 7: 4]: RXDV Pad Skew
	phy_write( eng, 14, temp & 0xff0f | 0x0000 );
//	phy_write( eng, 14, temp & 0xff0f | 0x00f0 );
printf("Reg2.4 = %04x -> %04x\n", temp, phy_read( eng, 14 ));

	phy_write( eng, 13, 0x0002 );
	phy_write( eng, 14, 0x0005 );
	phy_write( eng, 13, 0x4002 );
	temp = phy_read( eng, 14 );
	//Reg2.5[15:12]: RXD3 Pad Skew
	//Reg2.5[11: 8]: RXD2 Pad Skew
	//Reg2.5[ 7: 4]: RXD1 Pad Skew
	//Reg2.5[ 3: 0]: RXD0 Pad Skew
	phy_write( eng, 14, 0x0000 );
//	phy_write( eng, 14, 0xffff );
printf("Reg2.5 = %04x -> %04x\n", temp, phy_read( eng, 14 ));

	phy_write( eng, 13, 0x0002 );
	phy_write( eng, 14, 0x0008 );
	phy_write( eng, 13, 0x4002 );
	temp = phy_read( eng, 14 );
	//Reg2.8[9:5]: GTX_CLK Pad Skew
	//Reg2.8[4:0]: RX_CLK Pad Skew
//	phy_write( eng, 14, temp & 0xffe0 | 0x0000 );
	phy_write( eng, 14, temp & 0xffe0 | 0x001f );
printf("Reg2.8 = %04x -> %04x\n", temp, phy_read( eng, 14 ));
*/

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_Reset( eng );
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_Reset( eng );//DON'T support for 1G external loopback testing
		}
		else {
			phy_Reset( eng );
		}
	}
}

//------------------------------------------------------------
//external loop 100M: OK
//external loop 10M : OK
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_micrel2 (MAC_ENGINE *eng) {//KSZ8081
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Micrel] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_Reset( eng );
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 1 ] )
			phy_Reset( eng );
		else
			phy_Reset( eng );
	}
}

//------------------------------------------------------------
void recov_phy_vitesse (MAC_ENGINE *eng) {//VSC8601
	if ( eng->run.TM_Burst ) {
//		if ( eng->run.TM_IEEE ) {
//		}
//		else {
//		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 24, eng->phy.PHY_18h );
			phy_write( eng, 18, eng->phy.PHY_12h );
		}
	}
}

//------------------------------------------------------------
void phy_vitesse (MAC_ENGINE *eng) {//VSC8601
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[VITESSE] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_Reset( eng );
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			eng->phy.PHY_18h = phy_read( eng, 24 );
			eng->phy.PHY_12h = phy_read( eng, PHY_INER );

			phy_Reset( eng );

			phy_write( eng, 24, eng->phy.PHY_18h | 0x0001 );
			phy_write( eng, 18, eng->phy.PHY_12h | 0x0020 );
		}
		else {
			phy_Reset( eng );
		}
	}
}

//------------------------------------------------------------
void recov_phy_atheros (MAC_ENGINE *eng) {//AR8035
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		phy_Read_Write( eng, 11, 0x0000, 0x8000 );//clr set//Disable hibernate: Reg0Bh[15] = 0
		phy_Read_Write( eng, 17, 0x0001, 0x0000 );//clr set//Enable external loopback: Reg11h[0] = 1
	}
}

//------------------------------------------------------------
void phy_atheros (MAC_ENGINE *eng) {//AR8035
#ifdef PHY_debug
	if ( 1 ) {
#else
	if ( DbgPrn_PHYName ) {
#endif
		printf("--->(%04lx %04lx)[ATHEROS] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "--->(%04lx %04lx)[ATHEROS] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);
	}

	//Reg0b[15]: Power saving
	phy_write( eng, 29, 0x000b );
	eng->phy.PHY_1eh = phy_read( eng, 30 );
	if ( eng->phy.PHY_1eh & 0x8000 ) {
		printf("\n\n[Warning] Debug register offset = 11, bit 15 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Debug register offset = 11, bit 15 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Debug register offset = 11, bit 15 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);

		phy_write( eng, 30, eng->phy.PHY_1eh & 0x7fff );
	}
//	phy_write( eng, 30, (eng->phy.PHY_1eh & 0x7fff) | 0x8000 );

	//Check RGMIIRXCK delay (Sel_clk125m_dsp)
	phy_write( eng, 29, 0x0000 );
	eng->phy.PHY_1eh = phy_read( eng, 30 );
	if ( eng->phy.PHY_1eh & 0x8000 ) {
		printf("\n\n[Warning] Debug register offset = 0, bit 15 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Debug register offset = 0, bit 15 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Debug register offset = 0, bit 15 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);

		phy_write( eng, 30, eng->phy.PHY_1eh & 0x7fff );
	}
//	phy_write( eng, 30, (eng->phy.PHY_1eh & 0x7fff) | 0x8000 );

	//Check RGMIITXCK delay (rgmii_tx_clk_dly)
	phy_write( eng, 29, 0x0005 );
	eng->phy.PHY_1eh = phy_read( eng, 30 );
	if ( eng->phy.PHY_1eh & 0x0100 ) {
		printf("\n\n[Warning] Debug register offset = 5, bit 8 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Debug register offset = 5, bit 8 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Debug register offset = 5, bit 8 must be 0 [%04lx]\n\n", eng->phy.PHY_1eh);

		phy_write( eng, 30, eng->phy.PHY_1eh & 0xfeff );
	}
//	phy_write( eng, 30, (eng->phy.PHY_1eh & 0xfeff) | 0x0100 );

	//Check CLK_25M output (Select_clk125m)
	phy_write( eng, 13, 0x0007 );
	phy_write( eng, 14, 0x8016 );
	phy_write( eng, 13, 0x4007 );
	eng->phy.PHY_0eh = phy_read( eng, 14 );
	if ( (eng->phy.PHY_0eh & 0x0018) != 0x0018 ) {
		printf("\n\n[Warning] Device addrress = 7, Addrress ofset = 0x8016, bit 4~3 must be 3 [%04lx]\n\n", eng->phy.PHY_0eh);
		if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Device addrress = 7, Addrress ofset = 0x8016, bit 4~3 must be 3 [%04lx]\n\n", eng->phy.PHY_0eh );
		if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Device addrress = 7, Addrress ofset = 0x8016, bit 4~3 must be 3 [%04lx]\n\n", eng->phy.PHY_0eh );

		phy_write( eng, 14, (eng->phy.PHY_0eh & 0xffe7) | 0x0018 );
	}

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_write( eng,  0, eng->phy.PHY_00h );
		}
		else {
			phy_write( eng,  0, eng->phy.PHY_00h );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng,  0, eng->phy.PHY_00h );
	}
	else {
		phy_Read_Write( eng, 11, 0x8000, 0x0000 );//clr set//Disable hibernate: Reg0Bh[15] = 0
		phy_Read_Write( eng, 17, 0x0000, 0x0001 );//clr set//Enable external loopback: Reg11h[0] = 1

		phy_write( eng,  0, eng->phy.PHY_00h | 0x8000 );
#ifdef Delay_PHYRst
		phy_delay( Delay_PHYRst );
#endif
//		if ( eng->run.Speed_sel[ 0 ] ) {
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {
//		}
	}
}

//------------------------------------------------------------
void phy_default (MAC_ENGINE *eng) {
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)%s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );
}

//------------------------------------------------------------
// PHY Init
//------------------------------------------------------------
BOOLEAN find_phyadr (MAC_ENGINE *eng) {
	ULONG      PHY_val;
	BOOLEAN    ret = FALSE;
	CHAR       PHY_ADR_org;

#ifdef  DbgPrn_FuncHeader
	printf("find_phyadr\n");
	Debug_delay();
#endif

	if ( eng->env.AST2300 ) {
#ifdef Force_Enable_NewMDIO
		Write_Reg_PHY_DD( eng, 0x40, Read_Reg_PHY_DD( eng, 0x40 ) | 0x80000000 );
#endif
		eng->inf.NewMDIO = ( Read_Reg_PHY_DD( eng, 0x40 ) & 0x80000000 ) ? 1 : 0;
	} else
		eng->inf.NewMDIO = 0;

	PHY_ADR_org = eng->phy.Adr;
	// Check current PHY address by user setting
	PHY_val = phy_read( eng, PHY_REG_ID_1 );
	if ( PHY_IS_VALID( PHY_val ) ) {
		ret = TRUE;
	}
	else if ( eng->arg.GEn_SkipChkPHY ) {
		PHY_val = phy_read( eng, PHY_REG_BMCR );

		if ( ( PHY_val & 0x8000 ) & eng->arg.GEn_InitPHY ) {
		}
		else {
			ret = TRUE;
		}
	}

#ifdef Enable_SearchPHYID
	if ( ret == FALSE ) {
		// Scan PHY address from 0 to 31
		if ( eng->arg.GEn_InitPHY )
			printf("Search PHY address\n");
		for ( eng->phy.Adr = 0; eng->phy.Adr < 32; eng->phy.Adr++ ) {
			PHY_val = phy_read( eng, PHY_REG_ID_1 );
			if ( PHY_IS_VALID( PHY_val ) ) {
				ret = TRUE;
				break;
			}
		}
		// Don't find PHY address
	}
	if ( ret == FALSE )
		eng->phy.Adr = eng->arg.GPHYADR;
#endif

	if ( eng->arg.GEn_InitPHY ) {
		if ( ret == TRUE ) {
			if ( PHY_ADR_org != eng->phy.Adr ) {
				phy_id( eng, STD_OUT );
				if ( !eng->run.TM_Burst )
					phy_id( eng, FP_LOG );
			}
		}
		else {
			phy_id( eng, STD_OUT );
			if ( !eng->run.TM_Burst )
				phy_id( eng, FP_LOG );

			FindErr( eng, Err_Flag_PHY_Type );
		}
	}

	eng->phy.PHY_ID2 = phy_read( eng, PHY_REG_ID_1 );
	eng->phy.PHY_ID3 = phy_read( eng, PHY_REG_ID_2 );

	if      ( (eng->phy.PHY_ID2 == 0xffff) && ( eng->phy.PHY_ID3 == 0xffff ) && !eng->arg.GEn_SkipChkPHY ) {
		sprintf( eng->phy.PHYName, "--" );
		if ( eng->arg.GEn_InitPHY )
			FindErr( eng, Err_Flag_PHY_Type );
	}
#ifdef Enable_CheckZeroPHYID
	else if ( (eng->phy.PHY_ID2 == 0x0000) && ( eng->phy.PHY_ID3 == 0x0000 ) && !eng->arg.GEn_SkipChkPHY ) {
		sprintf( eng->phy.PHYName, "--" );
		if ( eng->arg.GEn_InitPHY )
			FindErr( eng, Err_Flag_PHY_Type );
	}
#endif

	return ret;
} // End BOOLEAN find_phyadr (MAC_ENGINE *eng)

//------------------------------------------------------------
char phy_chk (MAC_ENGINE *eng, ULONG id2, ULONG id3, ULONG id3_mask) {
	if ( ( eng->phy.PHY_ID2 == id2 ) && ( ( eng->phy.PHY_ID3 & id3_mask ) == ( id3 & id3_mask ) ) )
		return(1);
	else
		return(0);
}

//------------------------------------------------------------
void phy_set00h (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("phy_set00h\n");
	Debug_delay();
#endif

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x0140;
			else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x2100;
			else                                eng->phy.PHY_00h = 0x0100;
		}
		else {
			if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x0140;
			else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x2100;
			else                                eng->phy.PHY_00h = 0x0100;
//			if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x1140;
//			else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x3100;
//			else                                eng->phy.PHY_00h = 0x1100;
		}
	}
	else if ( eng->phy.loop_phy ) {
		if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x4140;
		else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x6100;
		else                                eng->phy.PHY_00h = 0x4100;
	}
	else {
		if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x0140;
		else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x2100;
		else                                eng->phy.PHY_00h = 0x0100;
	}
}

//------------------------------------------------------------
void phy_sel (MAC_ENGINE *eng, PHY_ENGINE *phyeng) {
#ifdef  DbgPrn_FuncHeader
	printf("phy_sel\n");
	Debug_delay();
#endif

	if      ( phy_chk( eng, 0x001c, 0xc916, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211F"          ); phyeng->fp_set = phy_realtek5 ; phyeng->fp_clr = recov_phy_realtek5 ;}//RTL8211F         1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x001c, 0xc915, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211E"          ); phyeng->fp_set = phy_realtek2 ; phyeng->fp_clr = recov_phy_realtek2 ;}//RTL8211E         1G/100/10M  GMII(RTL8211EG only), RGMII
	else if ( phy_chk( eng, 0x001c, 0xc914, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211D"          ); phyeng->fp_set = phy_realtek1 ; phyeng->fp_clr = recov_phy_realtek1 ;}//RTL8211D         1G/100/10M  GMII(RTL8211DN/RTL8211DG only), MII(RTL8211DN/RTL8211DG only), RGMII
	else if ( phy_chk( eng, 0x001c, 0xc912, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211C"          ); phyeng->fp_set = phy_realtek3 ; phyeng->fp_clr = recov_phy_realtek3 ;}//RTL8211C         1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x001c, 0xc930, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8363S"          ); phyeng->fp_set = phy_realtek6 ;                                      }//RTL8363S         1G/100/10M  RGMII Switch
	else if ( phy_chk( eng, 0x001c, 0xc816, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8201F"          ); phyeng->fp_set = phy_realtek4 ;                                      }//RTL8201F            100/10M  MII, RMII
	else if ( phy_chk( eng, 0x001c, 0xc815, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "RTL8201E"          ); phyeng->fp_set = phy_realtek0 ;                                      }//RTL8201E            100/10M  MII, RMII(RTL8201E(L)-VC only)
	else if ( phy_chk( eng, 0x0000, 0x8201, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "RTL8201N"          ); phyeng->fp_set = phy_realtek  ;                                      }//RTL8201N            100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0143, 0xbcb2, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM5482"           ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM5482          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0143, 0xbca0, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM5481"           ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM5481          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0362, 0x5e6a, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM54612"          ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM54612         1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0362, 0x5d10, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM54616S"         ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM54616S        1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0020, 0x60b0, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM5464SR"         ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM5464SR        1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0020, 0x60c1, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM5461S"          ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM5461S
    else if ( phy_chk( eng, 0x600d, 0x84a2, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM54210E"         ); phyeng->fp_set = phy_broadcom0; phyeng->fp_clr = recov_phy_broadcom0;}//BCM54210E
	else if ( phy_chk( eng, 0x0040, 0x61e0, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "BCM5221"           ); phyeng->fp_set = phy_broadcom ;                                      }//BCM5221             100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0141, 0x0e22, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "88E3019"           ); phyeng->fp_set = phy_marvell3 ;                                      }//88E3019             100/10M  RMII, RGMII
	else if ( phy_chk( eng, 0x0141, 0x0dd0, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "88E15 10/12/14/18" ); phyeng->fp_set = phy_marvell2 ; phyeng->fp_clr = recov_phy_marvell2 ;}//88E1512          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0xff00, 0x1761, 0xffff      ) ) { sprintf( eng->phy.PHYName, "88E6176(IntLoop)"  ); phyeng->fp_set = phy_marvell1 ; phyeng->fp_clr = recov_phy_marvell1 ;}//88E6176          1G/100/10M  2 RGMII Switch
	else if ( phy_chk( eng, 0xff00, 0x1152, 0xffff      ) ) { sprintf( eng->phy.PHYName, "88E6320(IntLoop)"  ); phyeng->fp_set = phy_marvell1 ; phyeng->fp_clr = recov_phy_marvell1 ;}//88E6320          1G/100/10M  2 RGMII Switch
	else if ( phy_chk( eng, 0x0141, 0x0e90, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "88E1310"           ); phyeng->fp_set = phy_marvell0 ; phyeng->fp_clr = recov_phy_marvell0 ;}//88E1310          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0141, 0x0cc0, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "88E1111"           ); phyeng->fp_set = phy_marvell  ; phyeng->fp_clr = recov_phy_marvell  ;}//88E1111          1G/100/10M  GMII, MII, RGMII
	else if ( phy_chk( eng, 0x0022, 0x1555, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ8031/KSZ8051"   ); phyeng->fp_set = phy_micrel0  ;                                      }//KSZ8051/KSZ8031     100/10M  RMII
	else if ( phy_chk( eng, 0x0022, 0x1622, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ9031"           ); phyeng->fp_set = phy_micrel1  ;                                      }//KSZ9031          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0022, 0x1562, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ8081"           ); phyeng->fp_set = phy_micrel2  ;                                      }//KSZ8081             100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0022, 0x1512, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ8041"           ); phyeng->fp_set = phy_micrel   ;                                      }//KSZ8041             100/10M  RMII
	else if ( phy_chk( eng, 0x004d, 0xd072, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "AR8035"            ); phyeng->fp_set = phy_atheros  ; phyeng->fp_clr = recov_phy_atheros  ;}//AR8035           1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0007, 0xc0c4, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "LAN8700"           ); phyeng->fp_set = phy_smsc     ;                                      }//LAN8700             100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0007, 0x0421, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "VSC8601"           ); phyeng->fp_set = phy_vitesse  ; phyeng->fp_clr = recov_phy_vitesse  ;}//VSC8601          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0143, 0xbd70, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "[S]BCM5389"        ); phyeng->fp_set = 0            ; phyeng->fp_clr = 0                  ;}//BCM5389          1G/100/10M  RGMII(IMP Port)
	else                                                    { sprintf( eng->phy.PHYName, "default"           ); phyeng->fp_set = phy_default  ;                                      }//

	if ( eng->arg.GEn_InitPHY ) {
		if ( eng->arg.GDis_RecovPHY )
			phyeng->fp_clr = 0;
	} else {
		phyeng->fp_set = 0;
		phyeng->fp_clr = 0;
	}
}

//------------------------------------------------------------
void recov_phy (MAC_ENGINE *eng, PHY_ENGINE *phyeng) {
#ifdef  DbgPrn_FuncHeader
	printf("recov_phy\n");
	Debug_delay();
#endif

	(*phyeng->fp_clr)( eng );
}

//------------------------------------------------------------
void init_phy (MAC_ENGINE *eng, PHY_ENGINE *phyeng) {
#ifdef  DbgPrn_FuncHeader
	printf("init_phy\n");
	Debug_delay();
#endif

	if ( DbgPrn_PHYInit )
		phy_dump( eng );

	phy_set00h( eng );
	(*phyeng->fp_set)( eng );

	if ( DbgPrn_PHYInit )
		phy_dump( eng );
}

