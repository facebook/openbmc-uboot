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
static const char ThisFile[] = "phy.c";

#include "swfunc.h"

#ifdef SLT_UBOOT
  #include <common.h>
  #include <command.h>
  #include "comminf.h"
  #include "stduboot.h"
#endif
#ifdef SLT_DOS
  #include <stdio.h>
  #include <stdlib.h>
  #include <conio.h>
  #include <string.h>
  #include "comminf.h"
#endif
  
#include "phy.h"                    
#include "typedef.h"
#include "io.h"

ULONG	PHY_09h;
ULONG	PHY_18h;
ULONG	PHY_1fh;
ULONG	PHY_06hA[7];
ULONG	PHY_11h;
ULONG	PHY_12h;
ULONG	PHY_15h;
ULONG	PHY_06h;
char	PHYID[256];
ULONG	PHY_00h;

//------------------------------------------------------------
// PHY R/W basic
//------------------------------------------------------------
void phy_write (int adr, ULONG data) {
    int timeout = 0;
    
	if (AST2300_NewMDIO) {
		WriteSOC_DD( MAC_PHYBASE + 0x60, ( data << 16 ) | MAC_PHYWr_New | (PHY_ADR<<5) | (adr & 0x1f));

		while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYBusy_New ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if (!BurstEnable) 
#ifdef SLT_DOS
                    fprintf(fp_log, "[PHY-Write] Time out: %08lx\n", ReadSOC_DD( MAC_PHYBASE + 0x60 ) );
#endif
                    FindErr( Err_PHY_TimeOut );
				break;
			}
		}
	} 
	else {
		WriteSOC_DD( MAC_PHYBASE + 0x64, data );

		WriteSOC_DD( MAC_PHYBASE + 0x60, MDC_Thres | MAC_PHYWr | (PHY_ADR<<16) | ((adr & 0x1f) << 21));

		while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYWr ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
#ifdef SLT_DOS
                if (!BurstEnable) 
				    fprintf(fp_log, "[PHY-Write] Time out: %08lx\n", ReadSOC_DD( MAC_PHYBASE + 0x60 ) );
#endif
                FindErr( Err_PHY_TimeOut );
				break;
			}
		}
	} // End if (AST2300_NewMDIO)

	if ( DbgPrn_PHYRW ) 
	    printf ("[Wr ]%02d: %04lx\n", adr, data);
} // End void phy_write (int adr, ULONG data)

//------------------------------------------------------------
ULONG phy_read (int adr) {
    int timeout = 0;
    
	if ( AST2300_NewMDIO ) {
		WriteSOC_DD( MAC_PHYBASE + 0x60, MAC_PHYRd_New | (PHY_ADR << 5) | ( adr & 0x1f ) );

		while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYBusy_New ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !BurstEnable ) 
#ifdef SLT_DOS
                    fprintf(fp_log, "[PHY-Read] Time out: %08lx\n", ReadSOC_DD( MAC_PHYBASE + 0x60 ));
#endif
                    FindErr( Err_PHY_TimeOut );
				break;
			}
		}

		DELAY(Delay_PHYRd);
		Dat_ULONG = ReadSOC_DD( MAC_PHYBASE + 0x64 ) & 0xffff;
	} 
	else {
		WriteSOC_DD( MAC_PHYBASE + 0x60, MDC_Thres | MAC_PHYRd | (PHY_ADR << 16) | ((adr & 0x1f) << 21) );

		while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYRd ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
#ifdef SLT_DOS
                if ( !BurstEnable ) 
				    fprintf( fp_log, "[PHY-Read] Time out: %08lx\n", ReadSOC_DD( MAC_PHYBASE + 0x60 ) );
#endif
                FindErr( Err_PHY_TimeOut );
				break;
			}
		}

		DELAY( Delay_PHYRd );
		Dat_ULONG = ReadSOC_DD( MAC_PHYBASE + 0x64 ) >> 16;
	}

	if ( DbgPrn_PHYRW ) 
	    printf ("[Rd ]%02d: %04lx\n", adr, Dat_ULONG );
	    
	return( Dat_ULONG );
} // End ULONG phy_read (int adr)

//------------------------------------------------------------
void phy_Read_Write (int adr, ULONG clr_mask, ULONG set_mask) {
	if ( DbgPrn_PHYRW ) 
	    printf ("[RW ]%02d: clr:%04lx: set:%04lx\n", adr, clr_mask, set_mask);
	phy_write(adr, ((phy_read(adr) & (~clr_mask)) | set_mask));
}

//------------------------------------------------------------
void phy_out ( int adr ) {
	printf ("%02d: %04lx\n", adr, phy_read(adr));
}

//------------------------------------------------------------
//void phy_outchg ( int adr ) {
//    ULONG	PHY_valold = 0;
//    ULONG	PHY_val;
//
//	while (1) {
//		PHY_val = phy_read(adr);
//		if (PHY_valold != PHY_val) {
//			printf ("%02d: %04lx\n", adr, PHY_val);
//			PHY_valold = PHY_val;
//		}
//	}
//}

//------------------------------------------------------------
void phy_dump (char *name) {
	int	index;

	printf ("[%s][%d]----------------\n", name, PHY_ADR);
	for (index = 0; index < 32; index++) {
		printf ("%02d: %04lx ", index, phy_read(index));
		
		if ((index % 8) == 7) 
		    printf ("\n");
	}
}

//------------------------------------------------------------
void phy_id (BYTE option) {
    
    ULONG	reg_adr;
    CHAR    PHY_ADR_org;
    FILE_VAR
    
    GET_OBJ( option )

    PHY_ADR_org = PHY_ADR;
	for (PHY_ADR = 0; PHY_ADR < 32; PHY_ADR++) {

		PRINT(OUT_OBJ "[%02d] ", PHY_ADR);

        for (reg_adr = 2; reg_adr <= 3; reg_adr++)
			PRINT(OUT_OBJ "%ld:%04lx ", reg_adr, phy_read(reg_adr));

		if ((PHY_ADR % 4) == 3) 
		    PRINT(OUT_OBJ "\n");
	}
	PHY_ADR = PHY_ADR_org;
}


//------------------------------------------------------------
void phy_delay (int dt) {
    DELAY( dt );
}

//------------------------------------------------------------
// PHY IC
//------------------------------------------------------------
void phy_basic_setting (int loop_phy) {
	phy_Read_Write(0,  0x7140, PHY_00h); //clr set
	if ( DbgPrn_PHYRW ) 
	    printf ("[Set]00: %04lx\n", phy_read( PHY_REG_BMCR ));
}

//------------------------------------------------------------
void phy_Wait_Reset_Done (void) {
	int timeout = 0;
	
	while (  phy_read( PHY_REG_BMCR ) & 0x8000 ) {
		if (DbgPrn_PHYRW) 
		    printf ("00: %04lx\n", phy_read( PHY_REG_BMCR ));
		    
		if (++timeout > TIME_OUT_PHY_Rst) {
#ifdef SLT_DOS
            if (!BurstEnable) fprintf(fp_log, "[PHY-Reset] Time out: %08lx\n", ReadSOC_DD(MAC_PHYBASE+0x60));
#endif
            FindErr(Err_PHY_TimeOut);
			break;
		}
	}//wait Rst Done

	if (DbgPrn_PHYRW) printf ("[Clr]00: %04lx\n", phy_read( PHY_REG_BMCR ));
	DELAY(Delay_PHYRst);
}

//------------------------------------------------------------
void phy_Reset (int loop_phy) {
	phy_basic_setting(loop_phy);

	phy_Read_Write(0,  0x0000, 0x8000 | PHY_00h);//clr set//Rst PHY
	phy_Wait_Reset_Done();

	phy_basic_setting(loop_phy);
	DELAY(Delay_PHYRst);
}

//------------------------------------------------------------
void recov_phy_marvell (int loop_phy) {//88E1111
	if ( BurstEnable ) {
	} 
	else if ( loop_phy ) {
	} 
	else {
		if (GSpeed_sel[0]) {
			phy_write(9, PHY_09h);

			phy_Reset(loop_phy);

			phy_write(29, 0x0007);
			phy_Read_Write(30, 0x0008, 0x0000);//clr set
			phy_write(29, 0x0010);
			phy_Read_Write(30, 0x0002, 0x0000);//clr set
			phy_write(29, 0x0012);
			phy_Read_Write(30, 0x0001, 0x0000);//clr set

			phy_write(18, PHY_12h);
		}
	}
}

//------------------------------------------------------------
void phy_marvell (int loop_phy) {//88E1111
    int	Retry;

	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Marvell] %s\n", PHY_ID2, PHY_ID3, PHYName);

	if ( BurstEnable ) {
		phy_Reset(loop_phy);
	} 
	else if ( loop_phy ) {
		phy_Reset(loop_phy);
	} 
	else {
		if ( GSpeed_sel[0] ) {
			PHY_09h = phy_read( PHY_GBCR );
			PHY_12h = phy_read( PHY_INER );
			phy_write ( 18, 0x0000         );
			phy_Read_Write(  9, 0x0000, 0x1800 );//clr set
		}                              
                                       
		phy_Reset(loop_phy);           
                                       
		if (GSpeed_sel[0]) {           
			phy_write ( 29, 0x0007         );
			phy_Read_Write( 30, 0x0000, 0x0008 );//clr set
			phy_write ( 29, 0x0010         );
			phy_Read_Write( 30, 0x0000, 0x0002 );//clr set
			phy_write ( 29, 0x0012         );
			phy_Read_Write( 30, 0x0000, 0x0001 );//clr set
		}
	}

	Retry = 0;
	do {
		PHY_11h = phy_read( PHY_SR );
	} while ( !( ( PHY_11h & 0x0400 ) | loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void recov_phy_marvell0 (int loop_phy) {//88E1310
	if (BurstEnable) {
	} else if (loop_phy) {
	} else {
		if (GSpeed_sel[0]) {
			phy_write(22, 0x0006);
			phy_Read_Write(16, 0x0020, 0x0000);//clr set
			phy_write(22, 0x0000);
		}
	}
}

//------------------------------------------------------------
void phy_marvell0 (int loop_phy) {//88E1310
    int	Retry;
    
	if (DbgPrn_PHYName) 
	    printf ("--->(%04lx %04lx)[Marvell] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_write( 22, 0x0002 );

    PHY_15h = phy_read(21);
	if (PHY_15h & 0x0030) {
		printf ("\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", PHY_15h);
#ifdef SLT_DOS
        if ( IOTiming    ) 
		    fprintf (fp_io, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", PHY_15h);
		if ( !BurstEnable) 
		    fprintf (fp_log, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", PHY_15h);
#endif		    
//		phy_Read_Write(21, 0x0030, 0x0000);//clr set//[5]Rx Dly, [4]Tx Dly
        phy_write(21, PHY_15h & 0xffcf); // Set [5]Rx Dly, [4]Tx Dly to 0
	}
	phy_write(22, 0x0000);

	if ( BurstEnable ) {
		phy_Reset(loop_phy);
	} 
	else if ( loop_phy ) {
		phy_write( 22, 0x0002 );
		
		if ( GSpeed_sel[0] ) {
			phy_Read_Write( 21, 0x6040, 0x0040 );//clr set
		} 
		else if ( GSpeed_sel[1] ) {
			phy_Read_Write( 21, 0x6040, 0x2000 );//clr set
		} 
		else {
			phy_Read_Write( 21, 0x6040, 0x0000 );//clr set
		}
		phy_write( 22, 0x0000 );
		phy_Reset( loop_phy );
	} 
	else {
		if ( GSpeed_sel[0] ) {
			phy_write(  22, 0x0006         );
			phy_Read_Write( 16, 0x0000, 0x0020 );//clr set
			phy_write(  22, 0x0000         );
		}

		phy_Reset(loop_phy);
	}

	Retry = 0;
	do {
		PHY_11h = phy_read( PHY_SR );
	} while (!((PHY_11h & 0x0400) | loop_phy | (Retry++ > 20)));
}

//------------------------------------------------------------
void recov_phy_marvell1 (int loop_phy) {//88E6176
    CHAR    PHY_ADR_org;
    
	PHY_ADR_org = PHY_ADR;
	for ( PHY_ADR = 16; PHY_ADR <= 22; PHY_ADR++ ) {
		if ( BurstEnable ) {
		} 
		else {
			phy_write(6, PHY_06hA[PHY_ADR-16]);//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	for ( PHY_ADR = 21; PHY_ADR <= 22; PHY_ADR++ ) {
		phy_write(1, 0x3); //01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
	}
	PHY_ADR = PHY_ADR_org;
}

//------------------------------------------------------------
void phy_marvell1 (int loop_phy) {//88E6176
//    ULONG	PHY_01h;
    CHAR    PHY_ADR_org;
    
	if (DbgPrn_PHYName) 
	    printf ("--->(%04lx %04lx)[Marvell] %s\n", PHY_ID2, PHY_ID3, PHYName);

	//The 88E6176 is switch with 7 Port(P0~P6) and the PHYAdr will be fixed at 0x10~0x16, and only P5/P6 can be connected to the MAC.
	//Therefor, the 88E6176 only can run the internal loopback.
	PHY_ADR_org = PHY_ADR;
	for ( PHY_ADR = 16; PHY_ADR <= 20; PHY_ADR++ ) {
		if ( BurstEnable ) {
		} 
		else {
			PHY_06hA[PHY_ADR-16] = phy_read( PHY_ANER );
			phy_write(6, 0x00);//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	
	for ( PHY_ADR = 21; PHY_ADR <= 22; PHY_ADR++ ) {
//		PHY_01h = phy_read( PHY_REG_BMSR );
//		if      (GSpeed_sel[0]) phy_write(1, (PHY_01h & 0xfffc) | 0x2);//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
//		else if (GSpeed_sel[1]) phy_write(1, (PHY_01h & 0xfffc) | 0x1);//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
//		else                    phy_write(1, (PHY_01h & 0xfffc)      );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
		if      (GSpeed_sel[0]) phy_write(1, 0x2);//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
		else if (GSpeed_sel[1]) phy_write(1, 0x1);//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
		else                    phy_write(1, 0x0);//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.

		if (BurstEnable) {
		} 
		else {
			PHY_06hA[PHY_ADR-16] = phy_read( PHY_ANER );
			if (PHY_ADR == 21) phy_write(6, 0x20);//06h[5]P5 loopback, 06h[6]P6 loopback
			else               phy_write(6, 0x40);//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	PHY_ADR = PHY_ADR_org;
}

//------------------------------------------------------------
void recov_phy_marvell2 (int loop_phy) {//88E1512
	if (BurstEnable) {
	} 
	else if (loop_phy) {
	} 
	else {
		if (GSpeed_sel[0]) {
			phy_write(22, 0x0006);
			phy_Read_Write(18, 0x0008, 0x0000);//clr set
			phy_write(22, 0x0000);
		}
	}
}

//------------------------------------------------------------
void phy_marvell2 (int loop_phy) {//88E1512
    int	    Retry = 0;
    ULONG   temp_reg;

	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Marvell] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_write(22, 0x0002);
	PHY_15h = phy_read(21);
	
	if ( PHY_15h & 0x0030 ) {
		printf ("\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", PHY_15h);
#ifdef SLT_DOS
        if (IOTiming    ) fprintf (fp_io, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", PHY_15h);
		if (!BurstEnable) fprintf (fp_log, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", PHY_15h);
#endif
        //		phy_Read_Write(21, 0x0030, 0x0000);//clr set//[5]Rx Dly, [4]Tx Dly
//		phy_write(21, PHY_15h & 0xffcf);
	}
	phy_write(22, 0x0000);

	if ( BurstEnable ) {
		phy_Reset(loop_phy);
	} 
	else if (loop_phy) {
	    // Internal loopback funciton only support in copper mode
	    // switch page 18
	    phy_write(22, 0x0012);
	    // Change mode to Copper mode
	    phy_write(20, 0x8210);
	    // do software reset
	    do {
	        temp_reg = phy_read( 20 );
	    } while ( ( (temp_reg & 0x8000) == 0x8000 ) & (Retry++ < 20) );
	    
	    // switch page 2
		phy_write(22, 0x0002);
		if (GSpeed_sel[0]) {
			phy_Read_Write(21, 0x2040, 0x0040);//clr set
		} 
		else if (GSpeed_sel[1]) {
			phy_Read_Write(21, 0x2040, 0x2000);//clr set
		} 
		else {
			phy_Read_Write(21, 0x2040, 0x0000);//clr set
		}
		phy_write(22, 0x0000);

		phy_Reset(loop_phy);
	} 
	else {
		if (GSpeed_sel[0]) {
			phy_write(22, 0x0006);
			phy_Read_Write(18, 0x0000, 0x0008);//clr set
			phy_write(22, 0x0000);
		}

		phy_Reset(loop_phy);
	}

	Retry = 0;
	do {
		PHY_11h = phy_read( PHY_SR );
	} while (!((PHY_11h & 0x0400) | loop_phy | (Retry++ > 20)));
}

//------------------------------------------------------------
void phy_broadcom (int loop_phy) {//BCM5221
	if (DbgPrn_PHYName) 
	    printf ("--->(%04lx %04lx)[Broadcom] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Reset(loop_phy);

	if (IEEETesting) {
		if (IOTimingBund_arg == 0) {
			phy_write(25, 0x1f01);//Force MDI  //Measuring from channel A
		} 
		else {
			phy_Read_Write(24, 0x0000, 0x4000);//clr set//Force Link
//			phy_write( 0, PHY_00h);
//			phy_write(30, 0x1000);
		}
	}
}

//------------------------------------------------------------
void recov_phy_broadcom0 (int loop_phy) {//BCM54612

    // Need to do it for AST2400
    phy_write(0x1C, 0x8C00); // Disable GTXCLK Clock Delay Enable
    phy_write(0x18, 0xF0E7); // Disable RGMII RXD to RXC Skew            

    if (BurstEnable) {
	} 
	else if (loop_phy) {
		phy_write( 0, PHY_00h);
	} 
	else {
		phy_write(0x00, PHY_00h);
		phy_write(0x09, PHY_09h);
		phy_write(0x18, PHY_18h);
	}
}

//------------------------------------------------------------
void phy_broadcom0 (int loop_phy) {//BCM54612
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Broadcom] %s\n", PHY_ID2, PHY_ID3, PHYName);

    // Need to do it for AST2400
    phy_write(0x1C, 0x8C00); // Disable GTXCLK Clock Delay Enable
    phy_write(0x18, 0xF0E7); // Disable RGMII RXD to RXC Skew            
        
    // Backup org value
    // read reg 18H, Page 103, BCM54612EB1KMLG_Spec.pdf
    phy_write(0x18, 0x7007);
    PHY_18h = phy_read(0x18);
       
    PHY_00h = phy_read( PHY_REG_BMCR );
    PHY_09h = phy_read( PHY_GBCR );   
    
	if ( BurstEnable ) {
		phy_basic_setting(loop_phy);
	} 
	else if (loop_phy) {
		phy_basic_setting(loop_phy);
   
    // Enable Internal Loopback mode
    // Page 58, BCM54612EB1KMLG_Spec.pdf
    phy_write(0x0, 0x5140);
		DELAY(Delay_PHYRst);
    /* Only 1G Test is PASS, 100M and 10M is false @20130619 */     

// Waiting for BCM FAE's response
//        if (GSpeed_sel[0]) {
//            // Speed 1G
//            // Enable Internal Loopback mode
//            // Page 58, BCM54612EB1KMLG_Spec.pdf
//            phy_write(0x0, 0x5140);
//        } 
//        else if (GSpeed_sel[1]) {
//            // Speed 100M
//            // Enable Internal Loopback mode
//            // Page 58, BCM54612EB1KMLG_Spec.pdf
//            phy_write(0x0, 0x7100);
//            phy_write(0x1E, 0x1000);
//        } 
//        else if (GSpeed_sel[2]) {
//            // Speed 10M
//            // Enable Internal Loopback mode
//            // Page 58, BCM54612EB1KMLG_Spec.pdf
//            phy_write(0x0, 0x5100);
//            phy_write(0x1E, 0x1000);
//        }
//            
//		DELAY(Delay_PHYRst);
    } 
	else {
      
		if (GSpeed_sel[0]) {
            // Page 60, BCM54612EB1KMLG_Spec.pdf
            // need to insert loopback plug
			phy_write( 9, 0x1800);
			phy_write( 0, 0x0140);
            phy_write( 0x18, 0x8400); // Enable Transmit test mode
		} else if (GSpeed_sel[1]) {
            // Page 60, BCM54612EB1KMLG_Spec.pdf
            // need to insert loopback plug
			phy_write( 0, 0x2100);
            phy_write( 0x18, 0x8400); // Enable Transmit test mode            
		} else {
            // Page 60, BCM54612EB1KMLG_Spec.pdf
            // need to insert loopback plug
			phy_write( 0, 0x0100);
            phy_write( 0x18, 0x8400); // Enable Transmit test mode                               
		}
	}
}

//------------------------------------------------------------
void phy_realtek (int loop_phy) {//RTL8201N
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Realtek] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Reset(loop_phy);
}

//------------------------------------------------------------
//internal loop 100M: Don't support
//internal loop 10M : no  loopback stub
void phy_realtek0 (int loop_phy) {//RTL8201E
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Realtek] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Reset(loop_phy);

//	phy_Read_Write(25, 0x2800, 0x0000);//clr set
//	printf("Enable phy output RMII clock\n");
	if (IEEETesting) {
		phy_write(31, 0x0001);
		if (IOTimingBund_arg == 0) {
			phy_write(25, 0x1f01);//Force MDI  //Measuring from channel A
		} 
		else {
			phy_write(25, 0x1f00);//Force MDIX //Measuring from channel B
		}
		phy_write(31, 0x0000);
	}
}

//------------------------------------------------------------
void recov_phy_realtek1 (int loop_phy) {//RTL8211D
	if ( BurstEnable ) {
		if ( IEEETesting ) {
			if ( GSpeed_sel[0] ) {
				if (IOTimingBund_arg == 0) {
					//Test Mode 1
					phy_write( 31, 0x0002 );
					phy_write(  2, 0xc203 );
					phy_write( 31, 0x0000 );
					phy_write(  9, 0x0000 );
				} 
				else {
					//Test Mode 4
					phy_write( 31, 0x0000 );
					phy_write(  9, 0x0000 );
				}

				phy_write( 31, 0x0000 );
			} 
			else if ( GSpeed_sel[1] ) {
				phy_write( 23, 0x2100 );
				phy_write( 16, 0x016e );
			} 
			else {
//				phy_write( 31, 0x0006 );
//				phy_write(  0, 0x5a00 );
//				phy_write( 31, 0x0000 );
			}
		} // End if ( IEEETesting )
	} 
	else if (loop_phy) {
		if ( GSpeed_sel[0] ) {
			phy_write( 31, 0x0000 ); // new in Rev. 1.6
			phy_write(  0, 0x1140 ); // new in Rev. 1.6
			phy_write( 20, 0x8040 ); // new in Rev. 1.6
		}
	} 
	else {
		if ( GSpeed_sel[0] ) {
			phy_write( 31, 0x0001 );
			phy_write(  3, 0xdf41 );
			phy_write(  2, 0xdf20 );
			phy_write(  1, 0x0140 );
			phy_write(  0, 0x00bb );
			phy_write(  4, 0xb800 );
			phy_write(  4, 0xb000 );
                              
			phy_write( 31, 0x0000 );
//			phy_write( 26, 0x0020 ); // Rev. 1.2
			phy_write( 26, 0x0040 ); // new in Rev. 1.6
			phy_write(  0, 0x1140 );    
//			phy_write( 21, 0x0006 ); // Rev. 1.2
			phy_write( 21, 0x1006 ); // new in Rev. 1.6
			phy_write( 23, 0x2100 ); 
//		} else if ( GSpeed_sel[1] ) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0200 );
//			phy_write(  0, 0x1200 );
//		} else if ( GSpeed_sel[2] ) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0200 );
//			phy_write(  4, 0x05e1 );
//			phy_write(  0, 0x1200 );
		}
	} // End if ( BurstEnable )
} // End void recov_phy_realtek1 (int loop_phy)

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek1 (int loop_phy) {//RTL8211D
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Realtek] %s\n", PHY_ID2, PHY_ID3, PHYName);
         
	if ( BurstEnable ) {
		if ( IEEETesting ) {
			if ( GSpeed_sel[0] ) {
				if (IOTimingBund_arg == 0) {
					//Test Mode 1
					phy_write( 31, 0x0002 );
					phy_write(  2, 0xc22b );
					phy_write( 31, 0x0000 );
					phy_write(  9, 0x2000 );
				} 
				else {
					//Test Mode 4
					phy_write( 31, 0x0000 );
					phy_write(  9, 0x8000 );
				}
				phy_write( 31, 0x0000 );
			} 
			else if ( GSpeed_sel[1] ) {
				if ( IOTimingBund_arg == 0 ) {
					//From Channel A
					phy_write( 23, 0xa102 );
					phy_write( 16, 0x01ae );//MDI
				} 
				else {
					//From Channel B
					phy_Read_Write( 17, 0x0008, 0x0000 ); // clr set
					phy_write(  23, 0xa102 );         // MDI
					phy_write(  16, 0x010e );
				}
			} else {
//				if ( IOTimingBund_arg == 0 ) {//Pseudo-random pattern
//					phy_write( 31, 0x0006 );
//					phy_write(  0, 0x5a21 );
//					phy_write( 31, 0x0000 );
//				} 
//              else if ( IOTimingBund_arg == 1 ) {//¡§FF¡¨ pattern
//					phy_write( 31, 0x0006 );
//					phy_write(  2, 0x05ee );
//					phy_write(  0, 0xff21 );
//					phy_write( 31, 0x0000 );
//				} else {//¡§00¡¨ pattern
//					phy_write( 31, 0x0006 );
//					phy_write(  2, 0x05ee );
//					phy_write(  0, 0x0021 );
//					phy_write( 31, 0x0000 );
//				}
			}
		} 
		else {
			phy_Reset(loop_phy);
		}
	} 
	else if ( loop_phy ) {
		phy_Reset(loop_phy);
		
		if ( GSpeed_sel[0] ) {
			phy_write(20, 0x0042);//new in Rev. 1.6
		}
	} 
	else {
		if ( GSpeed_sel[0] ) {
			phy_write( 31, 0x0001 );
			phy_write(  3, 0xff41 );
			phy_write(  2, 0xd720 );
			phy_write(  1, 0x0140 );
			phy_write(  0, 0x00bb );
			phy_write(  4, 0xb800 );
			phy_write(  4, 0xb000 );
                              
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x0040 );
			phy_write( 24, 0x0008 );
                              
			phy_write( 31, 0x0000 );
			phy_write(  9, 0x0300 );
			phy_write( 26, 0x0020 );
			phy_write(  0, 0x0140 );
			phy_write( 23, 0xa101 );
			phy_write( 21, 0x0200 );
			phy_write( 23, 0xa121 );
			phy_write( 23, 0xa161 );
			phy_write(  0, 0x8000 );
			phy_Wait_Reset_Done();
			phy_delay(200); // new in Rev. 1.6
//		} 
//      else if ( GSpeed_sel[1] ) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0000 );
//			phy_write(  4, 0x0061 );
//			phy_write(  0, 0x1200 );
//			phy_delay(5000);
//		} 
//      else if (GSpeed_sel[2]) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0000 );
//			phy_write(  4, 0x05e1 );
//			phy_write(  0, 0x1200 );
//			phy_delay(5000);
		} 
		else {
			phy_Reset(loop_phy);
		}
	}
} // End void phy_realtek1 (int loop_phy)

//------------------------------------------------------------
void recov_phy_realtek2 (int loop_phy) {//RTL8211E
	if ( BurstEnable ) {
		if ( IEEETesting ) {
			phy_write( 31, 0x0005 );
			phy_write(  5, 0x8b86 );
			phy_write(  6, 0xe201 );
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x0020 );
			phy_write( 21, 0x1108 );
			phy_write( 31, 0x0000 );

			if ( GSpeed_sel[0] ) {
				phy_write( 31, 0x0000 );
				phy_write(  9, 0x0000 );
			} 
			else if ( GSpeed_sel[1] ) {
				phy_write( 31, 0x0007 );
				phy_write( 30, 0x002f );
				phy_write( 23, 0xd88f );
				phy_write( 30, 0x002d );
				phy_write( 24, 0xf050 );
				phy_write( 31, 0x0000 );
				phy_write( 16, 0x006e );
			} 
			else {
			}
		} 
		else {
		}
	} 
	else if (loop_phy) {
	} 
	else {
		if (GSpeed_sel[0]) {
			//Rev 1.5  //not stable
//			phy_write( 31, 0x0000 );
//			phy_write(  0, 0x8000 );
//			phy_Wait_Reset_Done();
//			phy_delay(30);
//			phy_write( 23, 0x2160 );
//			phy_write( 31, 0x0007 );
//			phy_write( 30, 0x0040 );
//			phy_write( 24, 0x0004 );
//			phy_write( 24, 0x1a24 );
//			phy_write( 25, 0xfd00 );
//			phy_write( 24, 0x0000 );
//			phy_write( 31, 0x0000 );
//			phy_write(  0, 0x1140 );
//			phy_write( 26, 0x0040 );
//			phy_write( 31, 0x0007 );
//			phy_write( 30, 0x002f );
//			phy_write( 23, 0xd88f );
//			phy_write( 30, 0x0023 );
//			phy_write( 22, 0x0300 );
//			phy_write( 31, 0x0000 );
//			phy_write( 21, 0x1006 );
//			phy_write( 23, 0x2100 );
/**/
			//Rev 1.6
			phy_write( 31, 0x0000 );
			phy_write(  0, 0x8000 );
			phy_Wait_Reset_Done();
			phy_delay(30);
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x0042 );
			phy_write( 21, 0x0500 );
			phy_write( 31, 0x0000 );
			phy_write(  0, 0x1140 );
			phy_write( 26, 0x0040 );
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x002f );
			phy_write( 23, 0xd88f );
			phy_write( 30, 0x0023 );
			phy_write( 22, 0x0300 );
			phy_write( 31, 0x0000 );
			phy_write( 21, 0x1006 );
			phy_write( 23, 0x2100 );
/**/
//		} else if (GSpeed_sel[1]) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0200 );
//			phy_write(  0, 0x1200 );
//		} else if (GSpeed_sel[2]) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0200 );
//			phy_write(  4, 0x05e1 );
//			phy_write(  0, 0x1200 );
		}
	}
} // End void recov_phy_realtek2 (int loop_phy)

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek2 (int loop_phy) {//RTL8211E

	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Realtek] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Read_Write( 0,  0x0000, 0x8000 | PHY_00h ); // clr set // Rst PHY
	phy_Wait_Reset_Done();
	phy_delay(30);

	if ( BurstEnable ) {
		if ( IEEETesting ) {
			phy_write( 31, 0x0005 );
			phy_write(  5, 0x8b86 );
			phy_write(  6, 0xe200 );
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x0020 );
			phy_write( 21, 0x0108 );
			phy_write( 31, 0x0000 );

			if ( GSpeed_sel[0] ) {
				phy_write( 31, 0x0000 );
				
				if ( IOTimingBund_arg == 0 ) {
					phy_write( 9, 0x2000);//Test Mode 1
				} 
				else {
					phy_write( 9, 0x8000);//Test Mode 4
				}
			} 
			else if ( GSpeed_sel[1] ) {
				phy_write( 31, 0x0007 );
				phy_write( 30, 0x002f );
				phy_write( 23, 0xd818 );
				phy_write( 30, 0x002d );
				phy_write( 24, 0xf060 );
				phy_write( 31, 0x0000 );
				
				if ( IOTimingBund_arg == 0 ) {
					phy_write(16, 0x00ae);//From Channel A
				} 
				else {
					phy_write(16, 0x008e);//From Channel B
				}
			} 
			else {
			}
		} 
		else {
			phy_basic_setting(loop_phy);
			phy_delay(30);
		}
	} 
	else if (loop_phy) {
		phy_basic_setting(loop_phy);

		phy_Read_Write(0,  0x0000, 0x8000 | PHY_00h);//clr set//Rst PHY
		phy_Wait_Reset_Done();
		phy_delay(30);

		phy_basic_setting(loop_phy);
		phy_delay(30);
	} 
	else {
		if ( GSpeed_sel[0] ) {
			//Rev 1.5  //not stable
//			phy_write( 23, 0x2160 );
//			phy_write( 31, 0x0007 );
//			phy_write( 30, 0x0040 );
//			phy_write( 24, 0x0004 );
//			phy_write( 24, 0x1a24 );
//			phy_write( 25, 0x7d00 );
//			phy_write( 31, 0x0000 );
//			phy_write( 23, 0x2100 );
//			phy_write( 31, 0x0007 );
//			phy_write( 30, 0x0040 );
//			phy_write( 24, 0x0000 );
//			phy_write( 30, 0x0023 );
//			phy_write( 22, 0x0006 );
//			phy_write( 31, 0x0000 );
//			phy_write(  0, 0x0140 );
//			phy_write( 26, 0x0060 );
//			phy_write( 31, 0x0007 );
//			phy_write( 30, 0x002f );
//			phy_write( 23, 0xd820 );
//			phy_write( 31, 0x0000 );
//			phy_write( 21, 0x0206 );
//			phy_write( 23, 0x2120 );
//			phy_write( 23, 0x2160 );
/**/                          
            //Rev 1.6                     
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x0042 );
			phy_write( 21, 0x2500 );
			phy_write( 30, 0x0023 );
			phy_write( 22, 0x0006 );
			phy_write( 31, 0x0000 );
			phy_write(  0, 0x0140 );
			phy_write( 26, 0x0060 );
			phy_write( 31, 0x0007 );
			phy_write( 30, 0x002f );
			phy_write( 23, 0xd820 );
			phy_write( 31, 0x0000 );
			phy_write( 21, 0x0206 );
			phy_write( 23, 0x2120 );
			phy_write( 23, 0x2160 );
			phy_delay(300);
/**/
//		} 
//      else if ( GSpeed_sel[1] ) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0000 );
//			phy_write(  4, 0x0061 );
//			phy_write(  0, 0x1200 );
//			phy_delay(5000);
//		} 
//      else if ( GSpeed_sel[2] ) {//option
//			phy_write( 31, 0x0000 );
//			phy_write(  9, 0x0000 );
//			phy_write(  4, 0x05e1 );
//			phy_write(  0, 0x1200 );
//			phy_delay(5000);
		} 
		else {
			phy_basic_setting(loop_phy);
			phy_delay(150);
		}
	}
} // End void phy_realtek2 (int loop_phy)

//------------------------------------------------------------
void recov_phy_realtek3 (int loop_phy) {//RTL8211C
	if ( BurstEnable ) {
		if ( IEEETesting ) {
			if ( GSpeed_sel[0] ) {
				phy_write(  9, 0x0000 );
			} 
			else if ( GSpeed_sel[1] ) {
				phy_write( 17, PHY_11h );
				phy_write( 14, 0x0000  );
				phy_write( 16, 0x00a0  );
			} 
			else {
//				phy_write( 31, 0x0006 );
//				phy_write(  0, 0x5a00 );
//				phy_write( 31, 0x0000 );
			}
		} 
		else {
		}
	} 
	else if (loop_phy) {
		if ( GSpeed_sel[0] ) {
			phy_write( 11, 0x0000 );
		}
		phy_write( 12, 0x1006 );
	} 
	else {
		if ( GSpeed_sel[0] ) {
			phy_write( 31, 0x0001 );
			phy_write(  4, 0xb000 );
			phy_write(  3, 0xff41 );
			phy_write(  2, 0xdf20 );
			phy_write(  1, 0x0140 );
			phy_write(  0, 0x00bb );
			phy_write(  4, 0xb800 );
			phy_write(  4, 0xb000 );
                              
			phy_write( 31, 0x0000 );
			phy_write( 25, 0x8c00 );
			phy_write( 26, 0x0040 );
			phy_write(  0, 0x1140 );
			phy_write( 14, 0x0000 );
			phy_write( 12, 0x1006 );
			phy_write( 23, 0x2109 );
		}
	}
}

//------------------------------------------------------------
void phy_realtek3 (int loop_phy) {//RTL8211C
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Realtek] %s\n", PHY_ID2, PHY_ID3, PHYName);

	if ( BurstEnable ) {
		if ( IEEETesting ) {
			if ( GSpeed_sel[0] ) {
				if ( IOTimingBund_arg == 0 ) {   //Test Mode 1
					phy_write( 9, 0x2000);
				} 
				else if (IOTimingBund_arg == 1) {//Test Mode 2
					phy_write( 9, 0x4000);
				} 
				else if (IOTimingBund_arg == 2) {//Test Mode 3
					phy_write( 9, 0x6000);
				} 
				else {                           //Test Mode 4
					phy_write( 9, 0x8000);
				}
			} 
			else if ( GSpeed_sel[1] ) {
				PHY_11h = phy_read( PHY_SR );
				phy_write( 17, PHY_11h & 0xfff7 );
				phy_write( 14, 0x0660 );
				
				if ( IOTimingBund_arg == 0 ) {
					phy_write( 16, 0x00a0 );//MDI  //From Channel A
				} 
				else {
					phy_write( 16, 0x0080 );//MDIX //From Channel B
				}
			} 
			else {
//				if (IOTimingBund_arg == 0) {//Pseudo-random pattern
//					phy_write( 31, 0x0006 );
//					phy_write(  0, 0x5a21 );
//					phy_write( 31, 0x0000 );
//				} 
//              else if (IOTimingBund_arg == 1) {//¡§FF¡¨ pattern
//					phy_write( 31, 0x0006 );
//					phy_write(  2, 0x05ee );
//					phy_write(  0, 0xff21 );
//					phy_write( 31, 0x0000 );
//				} 
//              else {//¡§00¡¨ pattern
//					phy_write( 31, 0x0006 );
//					phy_write(  2, 0x05ee );
//					phy_write(  0, 0x0021 );
//					phy_write( 31, 0x0000 );
//				}
			}
		} 
		else {
			phy_Reset(loop_phy);
		}
	} 
	else if (loop_phy) {
		phy_write( 0, 0x9200);
		phy_Wait_Reset_Done();
		phy_delay(30);

		phy_write( 17, 0x401c );
		phy_write( 12, 0x0006 );
		
		if ( GSpeed_sel[0] ) {
			phy_write(11, 0x0002);
		} 
		else {
			phy_basic_setting(loop_phy);
		}
	} 
	else {
		if (GSpeed_sel[0]) {
			phy_write( 31, 0x0001 );
			phy_write(  4, 0xb000 );
			phy_write(  3, 0xff41 );
			phy_write(  2, 0xd720 );
			phy_write(  1, 0x0140 );
			phy_write(  0, 0x00bb );
			phy_write(  4, 0xb800 );
			phy_write(  4, 0xb000 );
                              
			phy_write( 31, 0x0000 );
			phy_write( 25, 0x8400 );
			phy_write( 26, 0x0020 );
			phy_write(  0, 0x0140 );
			phy_write( 14, 0x0210 );
			phy_write( 12, 0x0200 );
			phy_write( 23, 0x2109 );
			phy_write( 23, 0x2139 );
		} 
		else {
			phy_Reset(loop_phy);
		}
	}
} // End void phy_realtek3 (int loop_phy)

//------------------------------------------------------------
void phy_realtek4 (int loop_phy) {//RTL8201F
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Realtek] %s\n", PHY_ID2, PHY_ID3, PHYName);

	if ( BurstEnable ) {
		if ( IEEETesting ) {
			phy_write( 31, 0x0004 );
			phy_write( 16, 0x4077 );
			phy_write( 21, 0xc5a0 );
			phy_write( 31, 0x0000 );

			if ( GSpeed_sel[1] ) {
				phy_write(  0, 0x8000 ); // Reset PHY
				phy_write( 24, 0x0310 ); // Disable ALDPS
				
				if ( IOTimingBund_arg == 0 ) {
					phy_write( 28, 0x40c2 ); //Force MDI //From Channel A (RJ45 pair 1, 2)
				}                        
				else {                   
					phy_write( 28, 0x40c0 ); //Force MDIX//From Channel B (RJ45 pair 3, 6)
				}
				phy_write( 0, 0x2100);       //Force 100M/Full Duplex)
			}
		} else {
			phy_Reset(loop_phy);
		}
	} 
	else {
		phy_Reset(loop_phy);
	}
}

//------------------------------------------------------------
void phy_smsc (int loop_phy) {//LAN8700
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[SMSC] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Reset(loop_phy);
}

//------------------------------------------------------------
void phy_micrel (int loop_phy) {//KSZ8041
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[Micrel] %s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Reset(loop_phy);

//	phy_write(24,  0x0600);
}

//------------------------------------------------------------
void phy_micrel0 (int loop_phy) {//KSZ8031/KSZ8051
	if ( DbgPrn_PHYName ) printf ("--->(%04lx %04lx)[Micrel] %s\n", PHY_ID2, PHY_ID3, PHYName);

	//For KSZ8051RNL only
	//Reg1Fh[7] = 0(default): 25MHz Mode, XI, XO(pin 9, 8) is 25MHz(crystal/oscilator).
	//Reg1Fh[7] = 1         : 50MHz Mode, XI(pin 9) is 50MHz(oscilator).
	PHY_1fh = phy_read(31);
	if (PHY_1fh & 0x0080) sprintf(PHYName, "%s-50MHz Mode", PHYName);
	else                  sprintf(PHYName, "%s-25MHz Mode", PHYName);

	if (IEEETesting) {
		phy_Read_Write( 0,  0x0000, 0x8000 | PHY_00h );//clr set//Rst PHY
		phy_Wait_Reset_Done();

		phy_Read_Write( 31, 0x0000, 0x2000 );//clr set//1Fh[13] = 1: Disable auto MDI/MDI-X
		phy_basic_setting(loop_phy);
		phy_Read_Write( 31, 0x0000, 0x0800 );//clr set//1Fh[11] = 1: Force link pass

//		phy_delay(2500);//2.5 sec
	} 
	else {
		phy_Reset(loop_phy);

		//Reg16h[6] = 1         : RMII B-to-B override
		//Reg16h[1] = 1(default): RMII override
		phy_Read_Write( 22, 0x0000, 0x0042 );//clr set
	}

	if ( PHY_1fh & 0x0080 ) 
	    phy_Read_Write( 31, 0x0000, 0x0080 );//clr set//Reset PHY will clear Reg1Fh[7]
}

//------------------------------------------------------------
void recov_phy_vitesse (int loop_phy) {//VSC8601
	if ( BurstEnable ) {
//		if (IEEETesting) {
//		} else {
//		}
	} 
	else if ( loop_phy ) {
	} 
	else {
		if ( GSpeed_sel[0] ) {
			phy_write( 24, PHY_18h );
			phy_write( 18, PHY_12h );
		}
	}
}

//------------------------------------------------------------
void phy_vitesse (int loop_phy) {//VSC8601
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)[VITESSE] %s\n", PHY_ID2, PHY_ID3, PHYName);

	if ( BurstEnable ) {
		if ( IEEETesting ) {
			phy_Reset(loop_phy);
		} 
		else {
			phy_Reset(loop_phy);
		}
	} 
	else if ( loop_phy ) {
		phy_Reset(loop_phy);
	} 
	else {
		if ( GSpeed_sel[0] ) {
			PHY_18h = phy_read( 24 );
			PHY_12h = phy_read( PHY_INER );

			phy_Reset(loop_phy);

			phy_write( 24, PHY_18h | 0x0001 );
			phy_write( 18, PHY_12h | 0x0020 );
		} 
		else {
			phy_Reset(loop_phy);
		}
	}
}

//------------------------------------------------------------
void phy_default (int loop_phy) {
	if ( DbgPrn_PHYName ) 
	    printf ("--->(%04lx %04lx)%s\n", PHY_ID2, PHY_ID3, PHYName);

	phy_Reset(loop_phy);
}

//------------------------------------------------------------
// PHY Init
//------------------------------------------------------------
BOOLEAN find_phyadr (void) {
    ULONG       PHY_val;
    BOOLEAN     ret = FALSE;
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("find_phyadr\n"); 
	    Debug_delay();
    #endif

	do {
	    // Check current PHY address by user setting
	    PHY_val = phy_read( PHY_REG_ID_1 );
    	if ( PHY_IS_VALID(PHY_val) ) {
    		ret = TRUE;
    		break;
    	}
    	 
    	if ( Enable_SkipChkPHY ) {
    		PHY_val = phy_read( PHY_REG_BMCR );
    		
    		if ((PHY_val & 0x8000) & Enable_InitPHY) {
    		    // PHY is reseting and need to inital PHY
                #ifndef Enable_SearchPHYID
    			break;
                #endif
    		} 
    		else {
    			ret = TRUE;
                break;    			
    		}
    	} 
    	
    	#ifdef Enable_SearchPHYID
    	// Scan PHY address from 0 to 31
        printf("Search PHY address\n");
        for ( PHY_ADR = 0; PHY_ADR < 32; PHY_ADR++ ) {
        	PHY_val = phy_read( PHY_REG_ID_1 );
        	if ( PHY_IS_VALID(PHY_val) ) {
        		ret = TRUE;
        		break;
        	}
        }
        // Don't find PHY address
        PHY_ADR = PHY_ADR_arg;
        #endif
    } while ( 0 );

	if ( ret == TRUE ) {
		if ( PHY_ADR_arg != PHY_ADR ) {

            if ( !BurstEnable ) 
			    phy_id( FP_LOG );

            phy_id( STD_OUT );
		}
	} 
	else {

		if ( !BurstEnable ) 
		    phy_id( FP_LOG );

        phy_id( STD_OUT );
		FindErr( Err_PHY_Type );
	}
	
	return ret;
} // End BOOLEAN find_phyadr (void)

//------------------------------------------------------------
char phy_chk (ULONG id2, ULONG id3, ULONG id3_mask) {
	if ((PHY_ID2 == id2) && ((PHY_ID3 & id3_mask) == (id3 & id3_mask))) 
	    return(1);
	else                                                                
	    return(0);
}

//------------------------------------------------------------
void phy_set00h (int loop_phy) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("phy_set00h\n"); 
	    Debug_delay();
    #endif

	if (BurstEnable) {
		if (IEEETesting) {
			if      (GSpeed_sel[0]) PHY_00h = 0x0140;
			else if (GSpeed_sel[1]) PHY_00h = 0x2100;
			else                    PHY_00h = 0x0100;
		} 
		else {
			if      (GSpeed_sel[0]) PHY_00h = 0x1140;
			else if (GSpeed_sel[1]) PHY_00h = 0x3100;
			else                    PHY_00h = 0x1100;
		}
	} 
	else if (loop_phy) {
		if      (GSpeed_sel[0]) PHY_00h = 0x4140;
		else if (GSpeed_sel[1]) PHY_00h = 0x6100;
		else                    PHY_00h = 0x4100;
	} 
	else {
		if      (GSpeed_sel[0]) PHY_00h = 0x0140;
		else if (GSpeed_sel[1]) PHY_00h = 0x2100;
		else                    PHY_00h = 0x0100;
	}
}

//------------------------------------------------------------
void phy_sel (int loop_phy) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("phy_sel\n"); 
	    Debug_delay();
    #endif

	PHY_ID2 = phy_read( PHY_REG_ID_1 );
	PHY_ID3 = phy_read( PHY_REG_ID_2 );
	phy_set00h(loop_phy);

	if      ((PHY_ID2 == 0xffff) && (PHY_ID3 == 0xffff) && !Enable_SkipChkPHY) {
	    sprintf(PHYName, "--"); 
	    FindErr(Err_PHY_Type);
	}
#ifdef Enable_CheckZeroPHYID
	else if ((PHY_ID2 == 0x0000) && (PHY_ID3 == 0x0000) && !Enable_SkipChkPHY) {
	    sprintf(PHYName, "--"); FindErr(Err_PHY_Type);
	}
#endif

	if      (phy_chk(0x0362, 0x5e6a, 0xfff0     )) {sprintf(PHYName, "BCM54612"        ); if (Enable_InitPHY) phy_broadcom0(loop_phy);}//BCM54612         1G/100/10M  RGMII
	else if (phy_chk(0x0362, 0x5d10, 0xfff0     )) {sprintf(PHYName, "BCM54616S"       ); if (Enable_InitPHY) phy_broadcom0(loop_phy);}//BCM54616A        1G/100/10M  RGMII	    
	else if (phy_chk(0x0040, 0x61e0, PHYID3_Mask)) {sprintf(PHYName, "BCM5221"         ); if (Enable_InitPHY) phy_broadcom (loop_phy);}//BCM5221             100/10M  MII, RMII
	else if (phy_chk(0x0141, 0x0dd0, 0xfff0     )) {sprintf(PHYName, "88E1512"         ); if (Enable_InitPHY) phy_marvell2 (loop_phy);}//88E1512          1G/100/10M  RGMII
	else if (phy_chk(0xff00, 0x1761, 0xffff     )) {sprintf(PHYName, "88E6176(IntLoop)"); if (Enable_InitPHY) phy_marvell1 (loop_phy);}//88E6176          1G/100/10M  2 RGMII Switch
	else if (phy_chk(0x0141, 0x0e90, 0xfff0     )) {sprintf(PHYName, "88E1310"         ); if (Enable_InitPHY) phy_marvell0 (loop_phy);}//88E1310          1G/100/10M  RGMII
	else if (phy_chk(0x0141, 0x0cc0, PHYID3_Mask)) {sprintf(PHYName, "88E1111"         ); if (Enable_InitPHY) phy_marvell  (loop_phy);}//88E1111          1G/100/10M  GMII, MII, RGMII
	else if (phy_chk(0x001c, 0xc816, 0xffff     )) {sprintf(PHYName, "RTL8201F"        ); if (Enable_InitPHY) phy_realtek4 (loop_phy);}//RTL8201F            100/10M  MII, RMII
	else if (phy_chk(0x001c, 0xc815, 0xfff0     )) {sprintf(PHYName, "RTL8201E"        ); if (Enable_InitPHY) phy_realtek0 (loop_phy);}//RTL8201E            100/10M  MII, RMII(RTL8201E(L)-VC only)
	else if (phy_chk(0x001c, 0xc912, 0xffff     )) {sprintf(PHYName, "RTL8211C"        ); if (Enable_InitPHY) phy_realtek3 (loop_phy);}//RTL8211C         1G/100/10M  RGMII
	else if (phy_chk(0x001c, 0xc914, 0xffff     )) {sprintf(PHYName, "RTL8211D"        ); if (Enable_InitPHY) phy_realtek1 (loop_phy);}//RTL8211D         1G/100/10M  GMII(RTL8211DN/RTL8211DG only), MII(RTL8211DN/RTL8211DG only), RGMII
	else if (phy_chk(0x001c, 0xc915, 0xffff     )) {sprintf(PHYName, "RTL8211E"        ); if (Enable_InitPHY) phy_realtek2 (loop_phy);}//RTL8211E         1G/100/10M  GMII(RTL8211EG only), RGMII
	else if (phy_chk(0x0000, 0x8201, PHYID3_Mask)) {sprintf(PHYName, "RTL8201N"        ); if (Enable_InitPHY) phy_realtek  (loop_phy);}//RTL8201N            100/10M  MII, RMII
	else if (phy_chk(0x0007, 0xc0c4, PHYID3_Mask)) {sprintf(PHYName, "LAN8700"         ); if (Enable_InitPHY) phy_smsc     (loop_phy);}//LAN8700             100/10M  MII, RMII
	else if (phy_chk(0x0022, 0x1555, 0xfff0     )) {sprintf(PHYName, "KSZ8031/KSZ8051" ); if (Enable_InitPHY) phy_micrel0  (loop_phy);}//KSZ8051/KSZ8031     100/10M  RMII
	else if (phy_chk(0x0022, 0x1560, 0xfff0     )) {sprintf(PHYName, "KSZ8081"         ); if (Enable_InitPHY) phy_micrel0  (loop_phy);}//KSZ8081             100/10M  RMII
	else if (phy_chk(0x0022, 0x1512, 0xfff0     )) {sprintf(PHYName, "KSZ8041"         ); if (Enable_InitPHY) phy_micrel   (loop_phy);}//KSZ8041             100/10M  RMII
	else if (phy_chk(0x0007, 0x0421, 0xfff0     )) {sprintf(PHYName, "VSC8601"         ); if (Enable_InitPHY) phy_vitesse  (loop_phy);}//VSC8601          1G/100/10M  RGMII
	else                                           {sprintf(PHYName, "default"         ); if (Enable_InitPHY) phy_default  (loop_phy);}//
}

//------------------------------------------------------------
void recov_phy (int loop_phy) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("recov_phy\n"); 
	    Debug_delay();
    #endif

	if      (phy_chk(0x0362, 0x5e6a, 0xfff0     )) recov_phy_broadcom0(loop_phy);//BCM54612  1G/100/10M  RGMII
	else if (phy_chk(0x0362, 0x5d10, 0xfff0     )) recov_phy_broadcom0(loop_phy);//BCM54616A 1G/100/10M  RGMII
	else if (phy_chk(0x0141, 0x0dd0, 0xfff0     )) recov_phy_marvell2 (loop_phy);//88E1512   1G/100/10M  RGMII
	else if (phy_chk(0xff00, 0x1761, 0xffff     )) recov_phy_marvell1 (loop_phy);//88E6176   1G/100/10M  2 RGMII Switch
	else if (phy_chk(0x0141, 0x0e90, 0xfff0     )) recov_phy_marvell0 (loop_phy);//88E1310   1G/100/10M  RGMII
	else if (phy_chk(0x0141, 0x0cc0, PHYID3_Mask)) recov_phy_marvell  (loop_phy);//88E1111   1G/100/10M  GMII, MII, RGMII
	else if (phy_chk(0x001c, 0xc914, 0xffff     )) recov_phy_realtek1 (loop_phy);//RTL8211D  1G/100/10M  GMII(RTL8211DN/RTL8211DG only), MII(RTL8211DN/RTL8211DG only), RGMII
	else if (phy_chk(0x001c, 0xc915, 0xffff     )) recov_phy_realtek2 (loop_phy);//RTL8211E  1G/100/10M  GMII(RTL8211EG only), RGMII
	else if (phy_chk(0x001c, 0xc912, 0xffff     )) recov_phy_realtek3 (loop_phy);//RTL8211C  1G/100/10M  RGMII
	else if (phy_chk(0x0007, 0x0421, 0xfff0     )) recov_phy_vitesse  (loop_phy);//VSC8601   1G/100/10M  RGMII
}

//------------------------------------------------------------
void init_phy (int loop_phy) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("init_phy\n"); 
	    Debug_delay();
    #endif
	
	sprintf( PHYID, "PHY%d", SelectMAC + 1 );

	if ( DbgPrn_PHYInit ) 
	    phy_dump( PHYID );

	if ( find_phyadr() == TRUE ) 
	    phy_sel( loop_phy );

	if ( DbgPrn_PHYInit )
	    phy_dump( PHYID );
}


