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
#define MAC_C
static const char ThisFile[] = "mac.c";

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

#include "mac.h"

double	Avg_frame_len;
ULONG	Check_Des_Val;
ULONG	wp_fir;
ULONG	wp;
ULONG	FRAME_LEN_Cur;
ULONG	gdata;
ULONG	CheckDesFail_DesNum;
ULONG	VGAMode;
ULONG	SCU_1ch_old;
ULONG	SCU_0ch_old;
ULONG	SCU_48h_default;
ULONG	SCU_2ch_old;
ULONG	SCU_80h_old;
ULONG	SCU_74h_old;
ULONG	SCU_a4h_old;
ULONG	SCU_88h_old;
ULONG	WDT_0ch_old;
ULONG	SCU_04h_mix;
ULONG	SCU_04h_old;
ULONG	WDT_2ch_old;
char	SCU_oldvld = 0;

#ifdef SLT_UBOOT
#else
    static  double      timeused;
#endif
// -------------------------------------------------------------

void Debug_delay (void) {
    #ifdef DbgPrn_Enable_Debug_delay
    GET_CAHR();
    #endif
}
    



void dump_mac_ROreg (void) {
	DELAY(Delay_MACDump);
	printf("\n");
	printf("[MAC-H] ROReg A0h~ACh: %08lx %08lx %08lx %08lx\n", ReadSOC_DD(H_MAC_BASE+0xA0), ReadSOC_DD(H_MAC_BASE+0xA4), ReadSOC_DD(H_MAC_BASE+0xA8), ReadSOC_DD(H_MAC_BASE+0xAC));
	printf("[MAC-H] ROReg B0h~BCh: %08lx %08lx %08lx %08lx\n", ReadSOC_DD(H_MAC_BASE+0xB0), ReadSOC_DD(H_MAC_BASE+0xB4), ReadSOC_DD(H_MAC_BASE+0xB8), ReadSOC_DD(H_MAC_BASE+0xBC));
	printf("[MAC-H] ROReg C0h~C8h: %08lx %08lx %08lx      \n", ReadSOC_DD(H_MAC_BASE+0xC0), ReadSOC_DD(H_MAC_BASE+0xC4), ReadSOC_DD(H_MAC_BASE+0xC8));
}

//------------------------------------------------------------
// SCU
//------------------------------------------------------------
void recov_scu (void) {
    #ifdef  DbgPrn_FuncHeader
	    printf ("recov_scu\n"); 
	    Debug_delay();
    #endif

	//MAC
	WriteSOC_DD( H_MAC_BASE + 0x08, MAC_08h_old );
	WriteSOC_DD( H_MAC_BASE + 0x0c, MAC_0ch_old );
	WriteSOC_DD( H_MAC_BASE + 0x40, MAC_40h_old );

	//SCU
	WriteSOC_DD( SCU_BASE + 0x04, SCU_04h_old );
	WriteSOC_DD( SCU_BASE + 0x08, SCU_08h_old );
	WriteSOC_DD( SCU_BASE + 0x0c, SCU_0ch_old );
	WriteSOC_DD( SCU_BASE + 0x1c, SCU_1ch_old );
	WriteSOC_DD( SCU_BASE + 0x2c, SCU_2ch_old );
	WriteSOC_DD( SCU_BASE + 0x48, SCU_48h_old );
//	WriteSOC_DD( SCU_BASE + 0x70, SCU_70h_old );
	WriteSOC_DD( SCU_BASE + 0x74, SCU_74h_old );
	WriteSOC_DD( SCU_BASE + 0x7c, SCU_7ch_old );
	WriteSOC_DD( SCU_BASE + 0x80, SCU_80h_old );
	WriteSOC_DD( SCU_BASE + 0x88, SCU_88h_old );
	WriteSOC_DD( SCU_BASE + 0x90, SCU_90h_old );
	WriteSOC_DD( SCU_BASE + 0xa4, SCU_a4h_old );
	WriteSOC_DD( SCU_BASE + 0xac, SCU_ach_old );
    #ifdef AST1010_IOMAP
      WriteSOC_DD( SCU_BASE + 0x11C, SCU_11Ch_old );
    #endif

	//WDT
    #ifdef AST1010_IOMAP
    #else
    //    WriteSOC_DD(0x1e78500c, WDT_0ch_old);
    //    WriteSOC_DD(0x1e78502c, WDT_2ch_old);
    #endif

	if ( ASTChipType == 3 ) {
		if ( SCU_f0h_old & 0x01 ) WriteSOC_DD( SCU_BASE + 0xf0, 0xAEED0001 ); //Enable MAC34
		if ( SCU_f0h_old & 0x02 ) WriteSOC_DD( SCU_BASE + 0xf0, 0x2000DEEA ); //Enable Decode
		if ( SCU_f0h_old & 0x04 ) WriteSOC_DD( SCU_BASE + 0xf0, 0xA0E0E0D3 ); //Enable I2S
		if ( SCU_f0h_old & 0x08 ) WriteSOC_DD( SCU_BASE + 0xf0, 0x4D0E0E0A ); //Enable PCI Host
		if ( SCU_f0h_old & 0x10 ) WriteSOC_DD( SCU_BASE + 0xf0, 0x10ADDEED ); //Enable IR
		if ( SCU_f0h_old & 0x20 ) WriteSOC_DD( SCU_BASE + 0xf0, 0x66559959 ); //Enabel Buffer Merge
		if ( SCU_f0h_old & 0x40 ) WriteSOC_DD( SCU_BASE + 0xf0, 0x68961A33 ); //Enable PS2 IO
		if ( SCU_f0h_old & 0x80 ) WriteSOC_DD( SCU_BASE + 0xf0, 0x68971A33 ); //Enable PS2 IO
	}
} // End void recov_scu (void)

void read_scu (void) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("read_scu\n"); 
	    Debug_delay();
  #endif      

	if (!SCU_oldvld) {
		//SCU
		SCU_04h_old = ReadSOC_DD( SCU_BASE + 0x04 );
		SCU_08h_old = ReadSOC_DD( SCU_BASE + 0x08 );
		SCU_0ch_old = ReadSOC_DD( SCU_BASE + 0x0c );
		SCU_1ch_old = ReadSOC_DD( SCU_BASE + 0x1c );
		SCU_2ch_old = ReadSOC_DD( SCU_BASE + 0x2c );
		SCU_48h_old = ReadSOC_DD( SCU_BASE + 0x48 );
		SCU_70h_old = ReadSOC_DD( SCU_BASE + 0x70 );
		SCU_74h_old = ReadSOC_DD( SCU_BASE + 0x74 );
		SCU_7ch_old = ReadSOC_DD( SCU_BASE + 0x7c );
		SCU_80h_old = ReadSOC_DD( SCU_BASE + 0x80 );
		SCU_88h_old = ReadSOC_DD( SCU_BASE + 0x88 );
		SCU_90h_old = ReadSOC_DD( SCU_BASE + 0x90 );
		SCU_a4h_old = ReadSOC_DD( SCU_BASE + 0xa4 );
		SCU_ach_old = ReadSOC_DD( SCU_BASE + 0xac );
		SCU_f0h_old = ReadSOC_DD( SCU_BASE + 0xf0 );
        #ifdef AST1010_IOMAP
          SCU_11Ch_old = ReadSOC_DD( SCU_BASE + 0x11C );
		#endif

		//WDT
        #ifdef AST1010_IOMAP
        #else
		  WDT_0ch_old = ReadSOC_DD( 0x1e78500c );
		  WDT_2ch_old = ReadSOC_DD( 0x1e78502c );
        #endif

		SCU_oldvld = 1;
	} // End if (!SCU_oldvld) 
} // End read_scu()

void    Setting_scu (void)
{
    //SCU
    if (AST1010) {
        do {
            WriteSOC_DD( SCU_BASE + 0x00 , 0x1688a8a8);
            #ifndef SLT_UBOOT            
            WriteSOC_DD( SCU_BASE + 0x70 , SCU_70h_old & 0xfffffffe);  // Disable CPU
            #endif
        } while ( ReadSOC_DD( SCU_BASE + 0x00 ) != 0x1 ); 

        #if( AST1010_IOMAP == 1)
  		WriteSOC_DD( SCU_BASE + 0x11C, 0x00000000); // Disable Cache functionn
        #endif
    } 
    else {
        do {
            WriteSOC_DD( SCU_BASE + 0x00, 0x1688a8a8);
            #ifndef SLT_UBOOT
            WriteSOC_DD( SCU_BASE + 0x70, SCU_70h_old | 0x3 ); // Disable CPU
            #endif
        } while ( ReadSOC_DD( SCU_BASE + 0x00 ) != 0x1 );
    } // End if (AST1010)

    //WDT
    #ifdef AST1010_IOMAP
    #else
        WriteSOC_DD( 0x1e78500c, WDT_0ch_old & 0xfffffffc );
        WriteSOC_DD( 0x1e78502c, WDT_2ch_old & 0xfffffffc );
    #endif      
}

//------------------------------------------------------------
void init_scu1 (void) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("init_scu1\n"); 
	    Debug_delay();
  #endif

	if (AST3200) {
		WriteSOC_DD( SCU_BASE + 0x0c, (SCU_0ch_old & 0xffefffff) );//Clock Stop Control
	} 
	else if (AST1010) {
		WriteSOC_DD( SCU_BASE + 0x0c, ( SCU_0ch_old & 0xffffffbf ) );//Clock Stop Control
		WriteSOC_DD( SCU_BASE + 0x88, ((SCU_88h_old & 0x003fffff ) | 0xffc00000) );//Multi-function Pin Control
	} 
	else if (AST2300) {
#ifdef Enable_BufMerge
		WriteSOC_DD( SCU_BASE + 0xf0, 0x66559959 );//MAC buffer merge
#endif

#ifdef Enable_AST2300_Int125MHz
		SCU_48h_mix = (SCU_48h_old & 0xf0000000) | 0x80000000;
//		WriteSOC_DD( SCU_BASE + 0xf0, 0xa0e0e0d3 );//Enable I2S
//		WriteSOC_DD( SCU_BASE + 0x04, SCU_04h_old & 0xfffdffff );//Rst(Enable I2S)
//
////		WriteSOC_DD( 0x1e6e5020, ReadSOC_DD(0x1e6e5020) | 0x00010000 );//P_I2SPLLAdjEnable
//		WriteSOC_DD( 0x1e6e5020, ReadSOC_DD(0x1e6e5020) | 0x00000000 );//P_I2SPLLAdjEnable
//		WriteSOC_DD( 0x1e6e5024, 0x00000175 );//P_I2SPLLAdjCnt

//		WriteSOC_DD( SCU_BASE + 0x1c, 0x0000a51a );//124800000(24MHz)
//		WriteSOC_DD( SCU_BASE + 0x1c, 0x0000a92f );//125333333(24MHz)
//		WriteSOC_DD( SCU_BASE + 0x1c, 0x0000587d );//125000000(24MHz)
		WriteSOC_DD( SCU_BASE + 0x1c, 0x00006c7d );//125000000(24MHz)
		WriteSOC_DD( SCU_BASE + 0x2c, 0x00300000 | (SCU_2ch_old & 0xffcfffef) );//D-PLL assigned to VGA, D2-PLL assigned to I2S.
		WriteSOC_DD( SCU_BASE + 0x48, 0x80000000 | SCU_48h_old );//125MHz come from I2SPLL
#else
		SCU_48h_mix = (SCU_48h_old & 0xf0000000);
#endif
		switch (SelectMAC) {
			case 0  :
				WriteSOC_DD( SCU_BASE + 0x88, (SCU_88h_old & 0x3fffffff) | 0xc0000000 );//[31]MAC1 MDIO, [30]MAC1 MDC
				break;
			case 1  :
				WriteSOC_DD( SCU_BASE + 0x90, (SCU_90h_old & 0xfffffffb) | 0x00000004 );//[2 ]MAC2 MDC/MDIO
				break;
			case 2  :
			case 3  :
			default : break;
		}

		WriteSOC_DD(SCU_BASE+0x0c, (SCU_0ch_old & 0xff0fffff)             );//Clock Stop Control
//		WriteSOC_DD(SCU_BASE+0x80, (SCU_80h_old & 0xfffffff0) | 0x0000000f);//MAC1LINK/MAC2LINK
	} 
	else {
		switch (SelectMAC) {
			case 0  :
//				WriteSOC_DD(SCU_BASE+0x74, (SCU_74h_old & 0xfdffffff) | 0x02000000);//[25]MAC1 PHYLINK
				break;
			case 1  :
				if (MAC2_RMII) {
//					WriteSOC_DD(SCU_BASE+0x74, (SCU_74h_old & 0xfbefffff) | 0x04100000);//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
					WriteSOC_DD(SCU_BASE+0x74, (SCU_74h_old & 0xffefffff) | 0x00100000);//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
				} else {
//					WriteSOC_DD(SCU_BASE+0x74, (SCU_74h_old & 0xfbcfffff) | 0x04300000);//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
					WriteSOC_DD(SCU_BASE+0x74, (SCU_74h_old & 0xffcfffff) | 0x00300000);//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
				}
				break;
			default : break;
		} // End switch (SelectMAC)
	} // End if (AST3200)
} // End void init_scu1 (void)

//------------------------------------------------------------
void init_scu_macrst (void) {
    
#ifdef Enable_AST2300_Int125MHz
	if (ASTChipType == 3) {
		SCU_04h_mix = SCU_04h_old & 0xfffdffff;
	} else {
		SCU_04h_mix = SCU_04h_old;
	}
#else
	SCU_04h_mix = SCU_04h_old; 
#endif

	WriteSOC_DD ( SCU_BASE + 0x04, (SCU_04h_mix & ~SCU_04h) | SCU_04h);//Rst
	DELAY(Delay_SCU);
	WriteSOC_DD ( SCU_BASE + 0x04, (SCU_04h_mix & ~SCU_04h)          );//Enable Engine
//	DELAY(Delay_SCU);
} // End void init_scu_macrst (void)

//------------------------------------------------------------
void init_scu2 (void) {
    
#ifdef SCU_74h
	#ifdef  DbgPrn_FuncHeader
	    printf ("init_scu2\n"); 
	    Debug_delay();
  #endif

	WriteSOC_DD( SCU_BASE + 0x74, SCU_74h_old | SCU_74h );//PinMux
	delay(Delay_SCU);
#endif

} // End void init_scu2 (void)

//------------------------------------------------------------
void init_scu3 (void) {
    
#ifdef SCU_74h
	#ifdef  DbgPrn_FuncHeader
	    printf ("init_scu3\n"); 
	    Debug_delay();
  #endif

	WriteSOC_DD( SCU_BASE + 0x74, SCU_74h_old | (SCU_74h & 0xffefffff) );//PinMux
	delay(Delay_SCU);
#endif

} // End void init_scu3 (void)

//------------------------------------------------------------
// MAC
//------------------------------------------------------------
void init_mac (ULONG base, ULONG tdexbase, ULONG rdexbase) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("init_mac\n"); 
	    Debug_delay();
  #endif

#ifdef Enable_MAC_SWRst
	WriteSOC_DD( base + 0x50, 0x80000000 | MAC_50h | MAC_50h_Speed);
//	WriteSOC_DD( base + 0x50, 0x80000000);

	while (0x80000000 & ReadSOC_DD(base+0x50)) {
//printf(".");
		DELAY(Delay_MACRst);
	}
	DELAY(Delay_MACRst);
#endif

	WriteSOC_DD( base + 0x20, (tdexbase + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
	WriteSOC_DD( base + 0x24, (rdexbase + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
	
#ifdef MAC_30h          
	WriteSOC_DD( base + 0x30, MAC_30h);//Int Thr/Cnt
#endif
                  
#ifdef MAC_34h          
	WriteSOC_DD( base + 0x34, MAC_34h);//Poll Cnt
#endif

#ifdef MAC_38h          
	WriteSOC_DD( base + 0x38, MAC_38h);
#endif

#ifdef MAC_40h
	if (Enable_MACLoopback) {
		if (AST2300_NewMDIO) WriteSOC_DD( base + 0x40, MAC_40h | 0x80000000);
		else                 WriteSOC_DD( base + 0x40, MAC_40h);
	}
#endif

#ifdef MAC_48h
	WriteSOC_DD( base + 0x48, MAC_48h);
#endif

	if ( ModeSwitch == MODE_NSCI )
		WriteSOC_DD( base + 0x4c, NCSI_RxDMA_PakSize);   
	else
		WriteSOC_DD( base + 0x4c, DMA_PakSize);
	
	WriteSOC_DD( base + 0x50, MAC_50h | MAC_50h_Speed | 0xf);
	DELAY(Delay_MACRst);
} // End void init_mac (ULONG base, ULONG tdexbase, ULONG rdexbase)

//------------------------------------------------------------
// Basic
//------------------------------------------------------------
void FPri_RegValue (BYTE option) {
    
#ifdef SLT_UBOOT
#else
    time_t	timecur;
#endif

    FILE_VAR
    
    GET_OBJ( option )

	PRINT(OUT_OBJ "[SCU] 04:%08lx 08:%08lx 0c:%08lx 48:%08lx\n", SCU_04h_old, SCU_08h_old, SCU_0ch_old, SCU_48h_old);
	PRINT(OUT_OBJ "[SCU] 70:%08lx 74:%08lx 7c:%08lx\n",         SCU_70h_old, SCU_74h_old, SCU_7ch_old);
	PRINT(OUT_OBJ "[SCU] 80:%08lx 88:%08lx 90:%08lx f0:%08lx\n", SCU_80h_old, SCU_88h_old, SCU_90h_old, SCU_f0h_old);
	PRINT(OUT_OBJ "[SCU] a4:%08lx ac:%08lx\n", SCU_a4h_old, SCU_ach_old);
	PRINT(OUT_OBJ "[WDT] 0c:%08lx 2c:%08lx\n", WDT_0ch_old, WDT_2ch_old);
	PRINT(OUT_OBJ "[MAC] 08:%08lx 0c:%08lx\n", MAC_08h_old, MAC_0ch_old);
            PRINT(OUT_OBJ "[MAC] A0|%08lx %08lx %08lx %08lx\n", ReadSOC_DD( MAC_PHYBASE + 0xa0), ReadSOC_DD( MAC_PHYBASE + 0xa4 ), ReadSOC_DD( MAC_PHYBASE + 0xa8 ), ReadSOC_DD(MAC_PHYBASE + 0xac ) );
	PRINT(OUT_OBJ "[MAC] B0|%08lx %08lx %08lx %08lx\n", ReadSOC_DD( MAC_PHYBASE + 0xb0), ReadSOC_DD( MAC_PHYBASE + 0xb4 ), ReadSOC_DD( MAC_PHYBASE + 0xb8 ), ReadSOC_DD(MAC_PHYBASE + 0xbc ) );
	PRINT(OUT_OBJ "[MAC] C0|%08lx %08lx %08lx\n",      ReadSOC_DD( MAC_PHYBASE + 0xc0), ReadSOC_DD( MAC_PHYBASE + 0xc4 ), ReadSOC_DD( MAC_PHYBASE + 0xc8 ));

#ifdef SLT_UBOOT
#else
	fprintf(fp, "Time: %s", ctime(&timestart));
	time(&timecur);
	fprintf(fp, "----> %s", ctime(&timecur));
#endif
} // End void FPri_RegValue (BYTE *fp)

//------------------------------------------------------------
void FPri_End (BYTE option) {
    
    FILE_VAR
    
    GET_OBJ( option )
    
	if ( !RxDataEnable ) {
	} 
	else if ( Err_Flag ) {
		PRINT(OUT_OBJ "                    \n----> fail !!!\n");
	} else {
		PRINT(OUT_OBJ "                    \n----> All Pass !!!\n");
	}

	if ( ModeSwitch == MODE_DEDICATED ) {
		if (PHY_ADR_arg != PHY_ADR) 
		    PRINT(OUT_OBJ "\n[Warning] PHY Address change from %d to %d !!!\n", PHY_ADR_arg, PHY_ADR);
    }
	
	if ( AST1010 ) {
		Dat_ULONG = (SCU_ach_old >> 12) & 0xf;
		if (Dat_ULONG) {
			PRINT(OUT_OBJ "\n[Warning] SCUAC[15:12] == 0x%02lx is not the suggestion value 0.\n", Dat_ULONG);
			PRINT(OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n");
		}

		SCU_48h_default = SCU_48h_AST1010 & 0x01000f00;
		if ((SCU_48h_old != SCU_48h_default)) {
			PRINT(OUT_OBJ "\n[Warning] SCU48 == 0x%08lx is not the suggestion value 0x%08lx.\n", SCU_48h_old, SCU_48h_default);
			PRINT(OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n");
		}
	} 
	else if ( AST2300 ) {
		if ( AST2400 ) {
			Dat_ULONG = (SCU_90h_old >> 8) & 0xf;
			if (Dat_ULONG) {
				PRINT(OUT_OBJ "\n[Warning] SCU90[11: 8] == 0x%02lx is not the suggestion value 0.\n", Dat_ULONG);
				PRINT(OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n");
			}
		} 
		else {
			Dat_ULONG = (SCU_90h_old >> 8) & 0xff;
			if (Dat_ULONG) {
				PRINT(OUT_OBJ "\n[Warning] SCU90[15: 8] == 0x%02lx is not the suggestion value 0.\n", Dat_ULONG);
				PRINT(OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n");
			}
		}

		if (Enable_MAC34) SCU_48h_default = SCU_48h_AST2300;
		else              SCU_48h_default = SCU_48h_AST2300 & 0x0300ffff;
		    
		if ((SCU_48h_old != SCU_48h_default)) {
			PRINT(OUT_OBJ "\n[Warning] SCU48 == 0x%08lx is not the suggestion value 0x%08lx.\n", SCU_48h_old, SCU_48h_default);
			PRINT(OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n");
		}
	} // End if ( AST1010 )

	if ( ModeSwitch == MODE_NSCI ) {
		PRINT(OUT_OBJ "\n[Arg] %d %d %d %d %d %ld (%s)\n", GRun_Mode, PackageTolNum, ChannelTolNum, TestMode, IOTimingBund, (ARPNumCnt| (ULONG)PrintNCSIEn), ASTChipName);

		switch (NCSI_Cap_SLT.PCI_DID_VID) {
			case PCI_DID_VID_Intel_82574L		 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82574L       \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82575_10d6	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82575        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82575_10a7	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82575        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82575_10a9	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82575        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10c9	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10e6	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10e7	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10e8	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_1518	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_1526	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_150a	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_150d	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82599_10fb	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82599        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82599_1557	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82599        \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_I350_1521	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel I350         \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_I350_1523	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel I350         \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_I210     	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel I210         \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_X540	         : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel X540         \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5718	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5718   \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5720	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5720   \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5725	 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5725   \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
//			case PCI_DID_VID_Broadcom_BCM57810	 : PRINT( OUT_OBJ "[NC]%08x %08x: Broadcom BCM57810  \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			case PCI_DID_VID_Mellanox_ConnectX_3 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Mellanox ConnectX-3\n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
			default					:
				switch (NCSI_Cap_SLT.ManufacturerID) {
					case ManufacturerID_Intel    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel   \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
					case ManufacturerID_Broadcom : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom\n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
					case ManufacturerID_Mellanox : PRINT( OUT_OBJ "[NC]%08lx %08lx: Mellanox\n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID ); break;
					default				: PRINT(OUT_OBJ "[NC]%08lx %08lx          \n", NCSI_Cap_SLT.ManufacturerID, NCSI_Cap_SLT.PCI_DID_VID); break;
				} // End switch (NCSI_Cap_SLT.ManufacturerID)
		} // End switch (NCSI_Cap_SLT.PCI_DID_VID)
	}
	else {
		if (LOOP_INFINI) PRINT(OUT_OBJ "\n[Arg] %d %d %d # %d %d %d %lx (%s)[%d %d %d]\n" , GRun_Mode, GSpeed, GCtrl,               TestMode, PHY_ADR_arg, IOTimingBund, UserDVal, ASTChipName, Loop_rl[0], Loop_rl[1], Loop_rl[2]);
		else             PRINT(OUT_OBJ "\n[Arg] %d %d %d %ld %d %d %d %lx (%s)[%d %d %d]\n", GRun_Mode, GSpeed, GCtrl, LOOP_MAX_arg, TestMode, PHY_ADR_arg, IOTimingBund, UserDVal, ASTChipName, Loop_rl[0], Loop_rl[1], Loop_rl[2]);
		    
		PRINT(OUT_OBJ "[PHY] Adr:%d ID2:%04lx ID3:%04lx (%s)\n", PHY_ADR, PHY_ID2, PHY_ID3, PHYName);
	} // End if ( ModeSwitch == MODE_NSCI ) 
	
#ifdef SUPPORT_PHY_LAN9303
	PRINT(OUT_OBJ "[Ver II] %s (for LAN9303 with I2C%d)\n", version_name, LAN9303_I2C_BUSNUM);
#else
	PRINT(OUT_OBJ "[Ver II] %s\n", version_name);
#endif
} // End void FPri_End (BYTE option)

//------------------------------------------------------------
void FPri_ErrFlag (BYTE option) {
    
    FILE_VAR
    
    GET_OBJ( option )
    
	if (Err_Flag && Err_Flag_PrintEn) {
		PRINT(OUT_OBJ "\n\n");
//fprintf(fp, "Err_Flag: %x\n\n", Err_Flag);

		if ( Err_Flag & Err_PHY_Type                ) PRINT( OUT_OBJ "[Err] Unidentifiable PHY                                     \n" );
		if ( Err_Flag & Err_MALLOC_FrmSize          ) PRINT( OUT_OBJ "[Err] Malloc fail at frame size buffer                       \n" );
		if ( Err_Flag & Err_MALLOC_LastWP           ) PRINT( OUT_OBJ "[Err] Malloc fail at last WP buffer                          \n" );
		if ( Err_Flag & Err_Check_Buf_Data          ) PRINT( OUT_OBJ "[Err] Received data mismatch                                 \n" );
		if ( Err_Flag & Err_NCSI_Check_TxOwnTimeOut ) PRINT( OUT_OBJ "[Err] Time out of checking Tx owner bit in NCSI packet       \n" );
		if ( Err_Flag & Err_NCSI_Check_RxOwnTimeOut ) PRINT( OUT_OBJ "[Err] Time out of checking Rx owner bit in NCSI packet       \n" );
		if ( Err_Flag & Err_NCSI_Check_ARPOwnTimeOut) PRINT( OUT_OBJ "[Err] Time out of checking ARP owner bit in NCSI packet      \n" );
		if ( Err_Flag & Err_NCSI_No_PHY             ) PRINT( OUT_OBJ "[Err] Can not find NCSI PHY                                  \n" );
		if ( Err_Flag & Err_NCSI_Channel_Num        ) PRINT( OUT_OBJ "[Err] NCSI Channel Number Mismatch                           \n" );
		if ( Err_Flag & Err_NCSI_Package_Num        ) PRINT( OUT_OBJ "[Err] NCSI Package Number Mismatch                           \n" );
		if ( Err_Flag & Err_PHY_TimeOut             ) PRINT( OUT_OBJ "[Err] Time out of read/write/reset PHY register              \n" );
		if ( Err_Flag & Err_RXBUF_UNAVA             ) PRINT( OUT_OBJ "[Err] MAC00h[2]:Receiving buffer unavailable                 \n" );
		if ( Err_Flag & Err_RPKT_LOST               ) PRINT( OUT_OBJ "[Err] MAC00h[3]:Received packet lost due to RX FIFO full     \n" );
		if ( Err_Flag & Err_NPTXBUF_UNAVA           ) PRINT( OUT_OBJ "[Err] MAC00h[6]:Normal priority transmit buffer unavailable  \n" );
		if ( Err_Flag & Err_TPKT_LOST               ) PRINT( OUT_OBJ "[Err] MAC00h[7]:Packets transmitted to Ethernet lost         \n" );
		if ( Err_Flag & Err_DMABufNum               ) PRINT( OUT_OBJ "[Err] DMA Buffer is not enough                               \n" );
		if ( Err_Flag & Err_IOMargin                ) PRINT( OUT_OBJ "[Err] IO timing margin is not enough                         \n" );
		    
		if ( Err_Flag & Err_MHCLK_Ratio             ) {
			if ( AST1010 ) {
				PRINT(OUT_OBJ "[Err] Error setting of MAC AHB bus clock (SCU08[13:12])      \n");
				Dat_ULONG = (SCU_08h_old >> 12) & 0x3;
				PRINT(OUT_OBJ "      SCU08[13:12] == 0x%01lx is not the suggestion value 0.\n", Dat_ULONG);
			} 
			else {
				PRINT(OUT_OBJ "[Err] Error setting of MAC AHB bus clock (SCU08[18:16])      \n");
				Dat_ULONG = (SCU_08h_old >> 16) & 0x7;
				
				if (MAC1_1GEn | MAC2_1GEn) {
					PRINT(OUT_OBJ "      SCU08[18:16] == 0x%01lx is not the suggestion value 2.\n", Dat_ULONG);
				} 
				else {
					PRINT(OUT_OBJ "      SCU08[18:16] == 0x%01lx is not the suggestion value 4.\n", Dat_ULONG);
				}
			} // end if ( AST1010 )
		} // End if ( Err_Flag & Err_MHCLK_Ratio             )

		if (Err_Flag & Err_IOMarginOUF             ) {
			PRINT(OUT_OBJ "[Err] IO timing testing range out of boundary\n");
			if (Enable_RMII) {
#ifdef Enable_Old_Style
				PRINT(OUT_OBJ "      (%d,%d): 1x%d [%d]x[%d:%d]\n",        IOdly_out_reg_idx, IOdly_in_reg_idx, IOTimingBund, IOdly_out_reg_idx, IOdly_in_reg_idx - (IOTimingBund>>1), IOdly_in_reg_idx + (IOTimingBund>>1));
#else
				PRINT(OUT_OBJ "      (%d,%d): %dx1 [%d:%d]x[%d]\n",        IOdly_in_reg_idx, IOdly_out_reg_idx, IOTimingBund, IOdly_in_reg_idx - (IOTimingBund>>1), IOdly_in_reg_idx + (IOTimingBund>>1), IOdly_out_reg_idx);
#endif
			} else {
#ifdef Enable_Old_Style
				PRINT(OUT_OBJ "      (%d,%d): %dx%d [%d:%d]x[%d:%d]\n", IOdly_out_reg_idx, IOdly_in_reg_idx, IOTimingBund, IOTimingBund, IOdly_out_reg_idx - (IOTimingBund>>1), IOdly_out_reg_idx + (IOTimingBund>>1), IOdly_in_reg_idx - (IOTimingBund>>1), IOdly_in_reg_idx + (IOTimingBund>>1));
#else
				PRINT(OUT_OBJ "      (%d,%d): %dx%d [%d:%d]x[%d:%d]\n", IOdly_in_reg_idx, IOdly_out_reg_idx, IOTimingBund, IOTimingBund, IOdly_in_reg_idx - (IOTimingBund>>1), IOdly_in_reg_idx + (IOTimingBund>>1), IOdly_out_reg_idx - (IOTimingBund>>1), IOdly_out_reg_idx + (IOTimingBund>>1));
#endif
			}
		} // End if (Err_Flag & Err_IOMarginOUF             )
		
		if (Err_Flag & Err_Check_Des               ) {
			PRINT(OUT_OBJ "[Err] Descriptor error\n");
			if ( Check_Des_Val & Check_Des_TxOwnTimeOut ) PRINT( OUT_OBJ "[Des] Time out of checking Tx owner bit\n" );
			if ( Check_Des_Val & Check_Des_RxOwnTimeOut ) PRINT( OUT_OBJ "[Des] Time out of checking Rx owner bit\n" );
			if ( Check_Des_Val & Check_Des_RxErr        ) PRINT( OUT_OBJ "[Des] Input signal RxErr               \n" );
			if ( Check_Des_Val & Check_Des_OddNibble    ) PRINT( OUT_OBJ "[Des] Nibble bit happen                \n" );
			if ( Check_Des_Val & Check_Des_CRC          ) PRINT( OUT_OBJ "[Des] CRC error of frame               \n" );
			if ( Check_Des_Val & Check_Des_RxFIFOFull   ) PRINT( OUT_OBJ "[Des] Rx FIFO full                     \n" );
			if ( Check_Des_Val & Check_Des_FrameLen     ) PRINT( OUT_OBJ "[Des] Frame length mismatch            \n" );
		} // End if (Err_Flag & Err_Check_Des               )
		
		if (Err_Flag & Err_MACMode                ) {
			PRINT(OUT_OBJ "[Err] MAC interface mode mismatch\n");
			if ( AST1010 ) {
			} 
			else if (AST2300) {
				switch (MAC_Mode) {
					case 0 : PRINT( OUT_OBJ "      SCU70h[7:6] == 0: [MAC#1] RMII   [MAC#2] RMII \n" ); break;
					case 1 : PRINT( OUT_OBJ "      SCU70h[7:6] == 1: [MAC#1] RGMII  [MAC#2] RMII \n" ); break;
					case 2 : PRINT( OUT_OBJ "      SCU70h[7:6] == 2: [MAC#1] RMII   [MAC#2] RGMII\n" ); break;
					case 3 : PRINT( OUT_OBJ "      SCU70h[7:6] == 3: [MAC#1] RGMII  [MAC#2] RGMII\n" ); break;
				}
			} 
			else {
                switch (MAC_Mode) {
                	case 0 : PRINT( OUT_OBJ "      SCU70h[8:6] == 000: [MAC#1] GMII               \n" ); break;
                	case 1 : PRINT( OUT_OBJ "      SCU70h[8:6] == 001: [MAC#1] MII    [MAC#2] MII \n" ); break;
                	case 2 : PRINT( OUT_OBJ "      SCU70h[8:6] == 010: [MAC#1] RMII   [MAC#2] MII \n" ); break;
                	case 3 : PRINT( OUT_OBJ "      SCU70h[8:6] == 011: [MAC#1] MII                \n" ); break;
                	case 4 : PRINT( OUT_OBJ "      SCU70h[8:6] == 100: [MAC#1] RMII               \n" ); break;
                	case 5 : PRINT( OUT_OBJ "      SCU70h[8:6] == 101: Reserved                   \n" ); break;
                	case 6 : PRINT( OUT_OBJ "      SCU70h[8:6] == 110: [MAC#1] RMII   [MAC#2] RMII\n" ); break;
                	case 7 : PRINT( OUT_OBJ "      SCU70h[8:6] == 111: Disable MAC                \n" ); break;
                }
			} // End if ( AST1010 )
		} // End if (Err_Flag & Err_MACMode                )

		if ( ModeSwitch == MODE_NSCI ) {
			if (Err_Flag & Err_NCSI_LinkFail           ) {
				PRINT(OUT_OBJ "[Err] NCSI packet retry number over flows when find channel\n");
				
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Get_Version_ID         ) PRINT(OUT_OBJ "[NCSI] Time out when Get Version ID         \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Get_Capabilities       ) PRINT(OUT_OBJ "[NCSI] Time out when Get Capabilities       \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Select_Active_Package  ) PRINT(OUT_OBJ "[NCSI] Time out when Select Active Package  \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Set_MAC_Address ) PRINT(OUT_OBJ "[NCSI] Time out when Enable Set MAC Address \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Broadcast_Filter) PRINT(OUT_OBJ "[NCSI] Time out when Enable Broadcast Filter\n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Network_TX      ) PRINT(OUT_OBJ "[NCSI] Time out when Enable Network TX      \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Channel         ) PRINT(OUT_OBJ "[NCSI] Time out when Enable Channel         \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Disable_Network_TX     ) PRINT(OUT_OBJ "[NCSI] Time out when Disable Network TX     \n");
				if (NCSI_LinkFail_Val & NCSI_LinkFail_Disable_Channel        ) PRINT(OUT_OBJ "[NCSI] Time out when Disable Channel        \n");
			}
			
			if (Err_Flag & Err_NCSI_Channel_Num       ) PRINT(OUT_OBJ "[NCSI] Channel number expected: %d, real: %d\n", ChannelTolNum, number_chl);
			if (Err_Flag & Err_NCSI_Package_Num       ) PRINT(OUT_OBJ "[NCSI] Peckage number expected: %d, real: %d\n", PackageTolNum, number_pak);
		} // End if ( ModeSwitch == MODE_NSCI )
	} // End if (Err_Flag && Err_Flag_PrintEn)
} // End void FPri_ErrFlag (BYTE option)

//------------------------------------------------------------
void Finish_Close (void) {

	if (SCU_oldvld) 
	    recov_scu();

#ifdef SLT_DOS
	if (fp_io && IOTiming) 
	    fclose(fp_io);
	    
	if (fp_log) 
	    fclose(fp_log);
#endif    
} // End void Finish_Close (void)

//------------------------------------------------------------
char Finish_Check (int value) {
    ULONG temp;
    CHAR  i = 0;
    
#ifdef Disable_VGA
	if (VGAModeVld) {
		outp(0x3d4, 0x17);
		outp(0x3d5, VGAMode);
	}
#endif
	#ifdef  DbgPrn_FuncHeader
	    printf ("Finish_Check\n"); 
	    Debug_delay();
    #endif

	if ( FRAME_LEN ) 
	    free(FRAME_LEN);
	    
	if ( wp_lst   ) 
	    free(wp_lst   );

	Err_Flag = Err_Flag | value;
	
	if ( DbgPrn_ErrFlg ) 
	    printf ("\nErr_Flag: [%08lx]\n", Err_Flag);

	if ( !BurstEnable ) 
	    FPri_ErrFlag( FP_LOG );
	    
	if ( IOTiming    ) 
	    FPri_ErrFlag( FP_IO );
	    
	FPri_ErrFlag( STD_OUT );

	if ( !BurstEnable ) 
	    FPri_End( FP_LOG );
	    
	if ( IOTiming   ) 
	    FPri_End( FP_IO );
	    
	FPri_End( STD_OUT );
	

	if ( !BurstEnable ) FPri_RegValue( FP_LOG );
	if ( IOTiming     ) FPri_RegValue( FP_IO  );

	Finish_Close();

    // 20140325
    temp = ReadSOC_DD( 0x1e6e2040 );
    if ( ModeSwitch == MODE_NSCI ) 
    {
        if ( SelectMAC == 0 )
            i = 17;
        else
            i = 16;
    }
    else
    {
        if ( SelectMAC == 0 )
            i = 19;
        else
            i = 18;
    }
    WriteSOC_DD( 0x1e6e2040, (temp | (1 << i)) );
    
    
	if ( Err_Flag ) 
    {
        // Fail
	    return( 1 );
    }
	else
    {
        // Pass
	    return( 0 );
    }
} // End char Finish_Check (int value)

//------------------------------------------------------------
int FindErr (int value) {
	Err_Flag = Err_Flag | value;
	
	if ( DbgPrn_ErrFlg ) 
	    printf ("\nErr_Flag: [%08lx]\n", Err_Flag);

	return(1);
}

//------------------------------------------------------------
int FindErr_Des (int value) {
	Check_Des_Val = Check_Des_Val | value;
	Err_Flag      = Err_Flag | Err_Check_Des;
	if ( DbgPrn_ErrFlg ) 
	    printf ("\nErr_Flag: [%08lx] Check_Des_Val: [%08lx]\n", Err_Flag, Check_Des_Val);

	return(1);
}

//------------------------------------------------------------
// Get and Check status of Interrupt
//------------------------------------------------------------
int check_int ( char *type ) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("check_int  : %d\n", Loop); 
	    Debug_delay();
    #endif

	Dat_ULONG = ReadSOC_DD( H_MAC_BASE + 0x00 );//Interrupt Status
#ifdef SLT_DOS	
#ifdef CheckRxbufUNAVA
	if ( Dat_ULONG & 0x00000004 ) {
		fprintf(fp_log, "[%sIntStatus] Receiving buffer unavailable               : %08lx [loop:%d]\n", type, Dat_ULONG, Loop);
		FindErr( Err_RXBUF_UNAVA );
	}
#endif

#ifdef CheckRPktLost
	if ( Dat_ULONG & 0x00000008 ) {
		fprintf(fp_log, "[%sIntStatus] Received packet lost due to RX FIFO full   : %08lx [loop:%d]\n", type, Dat_ULONG, Loop);
		FindErr( Err_RPKT_LOST );
	}
#endif

#ifdef CheckNPTxbufUNAVA
	if ( Dat_ULONG & 0x00000040 ) {
		fprintf(fp_log, "[%sIntStatus] Normal priority transmit buffer unavailable: %08lx [loop:%d]\n", type, Dat_ULONG, Loop);
		FindErr( Err_NPTXBUF_UNAVA );
	}
#endif

#ifdef CheckTPktLost
	if ( Dat_ULONG & 0x00000080 ) {
		fprintf(fp_log, "[%sIntStatus] Packets transmitted to Ethernet lost       : %08lx [loop:%d]\n", type, Dat_ULONG, Loop);
		FindErr( Err_TPKT_LOST );
	}
#endif
#endif
	if (Err_Flag) 
	    return(1);
	else          
	    return(0);
} // End int check_int (char *type)


//------------------------------------------------------------
// Buffer
//------------------------------------------------------------
void setup_framesize (void) {
    int	i;
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("setup_framesize\n"); 
	    Debug_delay();
    #endif
	    
	//------------------------------------------------------------
	// Fill Frame Size out descriptor area
	//------------------------------------------------------------
    #ifdef SLT_UBOOT
	if (0)
    #else
	if ( ENABLE_RAND_SIZE )
    #endif
    {
		for (i = 0; i < DES_NUMBER; i++) {
			if ( FRAME_Rand_Simple ) {
				switch(rand() % 5) {
					case 0 : FRAME_LEN[i] = 0x4e ; break;
					case 1 : FRAME_LEN[i] = 0x4ba; break;
					default: FRAME_LEN[i] = 0x5ea; break;
				}
			} 
			else {
				FRAME_LEN_Cur = rand() % (MAX_FRAME_RAND_SIZE + 1);
			
				if (FRAME_LEN_Cur < MIN_FRAME_RAND_SIZE) 
				    FRAME_LEN_Cur = MIN_FRAME_RAND_SIZE;

				FRAME_LEN[i] = FRAME_LEN_Cur;
			}
#ifdef SLT_DOS			
			if (DbgPrn_FRAME_LEN) 
			    fprintf(fp_log, "[setup_framesize] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", FRAME_LEN[i], i, Loop);
#endif
		}
	} 
	else {
		for (i = 0; i < DES_NUMBER; i++) {
            #ifdef SelectSimpleLength
			if (i % FRAME_SELH_PERD) 
			    FRAME_LEN[i] = FRAME_LENH;
			else                          
			    FRAME_LEN[i] = FRAME_LENL;
            #else
			if ( BurstEnable ) {
				if (IEEETesting) {
					FRAME_LEN[i] = 1514;
				} 
				else {
                    #ifdef ENABLE_ARP_2_WOL
					FRAME_LEN[i] = 164;
                    #else
					FRAME_LEN[i] = 60;
                    #endif
				}
			} 
			else {
                #ifdef SelectLengthInc
//				FRAME_LEN[i] = (i%1455)+60;
				FRAME_LEN[i] = 1514-( i % 1455 );
                #else
				if (i % FRAME_SELH_PERD) 
				    FRAME_LEN[i] = FRAME_LENH;
				else                          
				    FRAME_LEN[i] = FRAME_LENL;
                #endif
			} // End if (BurstEnable)
            #endif
/*
			switch(i % 20) {
				case 0 : FRAME_LEN[i] = FRAME_LENH; break;
				case 1 : FRAME_LEN[i] = FRAME_LENH; break;
				case 2 : FRAME_LEN[i] = FRAME_LENH; break;
				default: FRAME_LEN[i] = FRAME_LENL; break;
			}
*/
#ifdef SLT_DOS
			if (DbgPrn_FRAME_LEN) 
			    fprintf(fp_log, "[setup_framesize] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", FRAME_LEN[i], i, Loop);
#endif
		} // End for (i = 0; i < DES_NUMBER; i++)
	} // End if ( ENABLE_RAND_SIZE )

    // Calculate average of frame size
	Avg_frame_len = 0;
	
	for ( i = 0; i < DES_NUMBER; i++ ) {
		Avg_frame_len += FRAME_LEN[i];
	}
	
	Avg_frame_len = Avg_frame_len / (double)DES_NUMBER;

	//------------------------------------------------------------
	// Write Plane
	//------------------------------------------------------------
	switch( ZeroCopy_OFFSET & 0x3 ) {
		case 0: wp_fir = 0xffffffff; break;
		case 1: wp_fir = 0xffffff00; break;
		case 2: wp_fir = 0xffff0000; break;
		case 3: wp_fir = 0xff000000; break;
	}
	
	for ( i = 0; i < DES_NUMBER; i++ ) {
		switch( ( ZeroCopy_OFFSET + FRAME_LEN[i] - 1 ) & 0x3 ) {
			case 0: wp_lst[i] = 0x000000ff; break;
			case 1: wp_lst[i] = 0x0000ffff; break;
			case 2: wp_lst[i] = 0x00ffffff; break;
			case 3: wp_lst[i] = 0xffffffff; break;
		}
	} // End for ( i = 0; i < DES_NUMBER; i++ )
} // End void setup_framesize (void)

//------------------------------------------------------------
void setup_arp (void) {
    int i;
    for (i = 0; i < 16; i++ )
        ARP_data[i] = ARP_org_data[i];
    
	ARP_data[1] = 0x0000ffff | ( SA[0] << 16 )
	                         | ( SA[1] << 24 );
	                                         
	ARP_data[2] =              ( SA[2]       )
	                         | ( SA[3] <<  8 )
	                         | ( SA[4] << 16 )
	                         | ( SA[5] << 24 );
	            
	ARP_data[5] = 0x00000100 | ( SA[0] << 16 )
	                         | ( SA[1] << 24 );
	            
	ARP_data[6] =              ( SA[2]       )
	                         | ( SA[3] <<  8 )
	                         | ( SA[4] << 16 )
	                         | ( SA[5] << 24 );
} // End void setup_arp (void)

//------------------------------------------------------------
void setup_buf (void) {
    int     i;
    int	    j;
    ULONG	adr;
    ULONG	adr_srt;
    ULONG	adr_end;
    ULONG	len;
    #ifdef SelectSimpleDA
        int     cnt;
        ULONG   Current_framelen;
    #endif
    
    #ifdef ENABLE_ARP_2_WOL
    int	    DA[3];

	DA[0] =  ( ( SelectWOLDA_DatH >>  8 ) & 0x00ff ) |
	         ( ( SelectWOLDA_DatH <<  8 ) & 0xff00 );
	         
	DA[1] =  ( ( SelectWOLDA_DatL >> 24 ) & 0x00ff ) |
	         ( ( SelectWOLDA_DatL >>  8 ) & 0xff00 );
	         
	DA[2] =  ( ( SelectWOLDA_DatL >>  8 ) & 0x00ff ) |
	         ( ( SelectWOLDA_DatL <<  8 ) & 0xff00 );
    #endif
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("setup_buf  : %d\n", Loop); 
	    Debug_delay();
    #endif

    // It need be multiple of 4
	adr_srt = GET_DMA_BASE_SETUP & 0xfffffffc;
	
	for (j = 0; j < DES_NUMBER; j++) {
		if ( DbgPrn_BufAdr ) 
		    printf("[loop:%4d][des:%4d][setup_buf  ] %08lx\n", Loop, j, adr_srt);
		    
		if ( BurstEnable ) {
			if ( IEEETesting ) {
                #ifdef ENABLE_DASA
				WriteSOC_DD( adr_srt    , 0xffffffff  );
				WriteSOC_DD( adr_srt + 4, ARP_data[1] );
				WriteSOC_DD( adr_srt + 8, ARP_data[2] );
				
				for (adr = (adr_srt + 12); adr < (adr_srt + DMA_PakSize); adr += 4 )
                #else
				for (adr = adr_srt; adr < (adr_srt + DMA_PakSize); adr += 4 )
                #endif
                {
					switch( TestMode ) {
						case 1: gdata = 0xffffffff;              break;
						case 2: gdata = 0x55555555;              break;
						case 3: gdata = rand() | (rand() << 16); break;
						case 5: gdata = UserDVal;                break;
					}
					WriteSOC_DD(adr, gdata);
				} // End for()
			} 
			else {
				for (i = 0; i < 16; i++) {
					WriteSOC_DD( adr_srt + ( i << 2 ), ARP_data[i] );
				}                
                
                #ifdef ENABLE_ARP_2_WOL          
				for (i = 16; i < 40; i += 3) {
					WriteSOC_DD( adr_srt + ( i << 2 ),     ( DA[1] << 16 ) |  DA[0] );
					WriteSOC_DD( adr_srt + ( i << 2 ) + 4, ( DA[0] << 16 ) |  DA[2] );
					WriteSOC_DD( adr_srt + ( i << 2 ) + 8, ( DA[2] << 16 ) |  DA[1] );
				}
                #endif
			} // End if ( IEEETesting ) 
		} 
		else {
		    // --------------------------------------------
            #ifdef SelectSimpleData
                #ifdef SimpleData_Fix
			    switch( j % SimpleData_FixNum ) {
			    	case  0 : gdata = SimpleData_FixVal00; break;
			    	case  1 : gdata = SimpleData_FixVal01; break;
			    	case  2 : gdata = SimpleData_FixVal02; break;
			    	case  3 : gdata = SimpleData_FixVal03; break;
			    	case  4 : gdata = SimpleData_FixVal04; break;
			    	case  5 : gdata = SimpleData_FixVal05; break;
			    	case  6 : gdata = SimpleData_FixVal06; break;
			    	case  7 : gdata = SimpleData_FixVal07; break;
			    	case  8 : gdata = SimpleData_FixVal08; break;
			    	case  9 : gdata = SimpleData_FixVal09; break;
			    	case 10 : gdata = SimpleData_FixVal10; break;
			    	default : gdata = SimpleData_FixVal11; break;
			    }
                #else
			    gdata   = 0x11111111 * ((j + SEED_START) % 256);
                #endif
 			
 			adr_end = adr_srt + DMA_PakSize;
			for ( adr = adr_srt; adr < adr_end; adr += 4 ) {
				WriteSOC_DD( adr, gdata );
			}		  
            // --------------------------------------------			
			#elif SelectSimpleDA

			gdata   = DATA_SEED(j + SEED_START);
			Current_framelen = FRAME_LEN[j];
			
			if ( DbgPrn_FRAME_LEN ) 
			    fprintf(fp_log, "[setup_buf      ] Current_framelen:%08lx[Des:%d][loop:%d]\n", Current_framelen, j, Loop);
			
			cnt     = 0;
 			len     = ( ( ( Current_framelen - 14 ) & 0xff ) << 8) | 
 			            ( ( Current_framelen - 14 ) >> 8 );

 			adr_end = adr_srt + DMA_PakSize;
			for ( adr = adr_srt; adr < adr_end; adr += 4 ) {
				cnt++;
				if      (cnt == 1 ) WriteSOC_DD( adr, SelectSimpleDA_Dat0 );
				else if (cnt == 2 ) WriteSOC_DD( adr, SelectSimpleDA_Dat1 );
				else if (cnt == 3 ) WriteSOC_DD( adr, SelectSimpleDA_Dat2 );
				else if (cnt == 4 ) WriteSOC_DD( adr, len | (len << 16)   );
				else				
				    WriteSOC_DD( adr, gdata );      
				              
				gdata += DATA_IncVal;
			}			
            // --------------------------------------------			
			#else

			gdata   = DATA_SEED(j + SEED_START);
 			adr_end = adr_srt + DMA_PakSize;
			for ( adr = adr_srt; adr < adr_end; adr += 4 ) {			
				WriteSOC_DD( adr, gdata );
                
				gdata += DATA_IncVal;
			}

			#endif
			
		} // End if ( BurstEnable )

		adr_srt += DMA_PakSize;
	} // End for (j = 0; j < DES_NUMBER; j++)
} // End void setup_buf (void)

//------------------------------------------------------------
// Check data of one packet
//------------------------------------------------------------
char check_Data (ULONG desadr, LONG number) {
	int	    index;
	int     cnt;
    ULONG	rdata;  
    ULONG	wp_lst_cur;  
    ULONG	adr_las;
    ULONG	adr;
    ULONG	adr_srt;
    ULONG	adr_end;
    ULONG	len;
    #ifdef SelectSimpleDA
        ULONG   gdata_bak;
    #endif
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("check_Data : %d\n", Loop); 
	    Debug_delay();
    #endif
    //printf("[Des:%d][loop:%d]Desadr:%08x\n", number, Loop, desadr);

	wp_lst_cur    = wp_lst[number];
	FRAME_LEN_Cur = FRAME_LEN[number];
#ifdef SLT_DOS	
	if ( DbgPrn_FRAME_LEN ) 
	    fprintf(fp_log, "[check_Data     ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", FRAME_LEN_Cur, number, Loop);
#endif	    
	adr_srt = ReadSOC_DD(desadr) & 0xfffffffc;
	adr_end = adr_srt + PktByteSize;
	
    #ifdef SelectSimpleData
        #ifdef SimpleData_Fix
	switch( number % SimpleData_FixNum ) {
		case  0 : gdata = SimpleData_FixVal00; break;
		case  1 : gdata = SimpleData_FixVal01; break;
		case  2 : gdata = SimpleData_FixVal02; break;
		case  3 : gdata = SimpleData_FixVal03; break;
		case  4 : gdata = SimpleData_FixVal04; break;
		case  5 : gdata = SimpleData_FixVal05; break;
		case  6 : gdata = SimpleData_FixVal06; break;
		case  7 : gdata = SimpleData_FixVal07; break;
		case  8 : gdata = SimpleData_FixVal08; break;
		case  9 : gdata = SimpleData_FixVal09; break;
		case 10 : gdata = SimpleData_FixVal10; break;
		default : gdata = SimpleData_FixVal11; break;
	}
        #else
	gdata   = 0x11111111 * ((number + SEED_START) % 256);
        #endif
    #else
	gdata   = DATA_SEED(number + SEED_START);
    #endif

	wp      = wp_fir;
	adr_las = adr_end - 4;

	cnt     = 0;
	len     = (((FRAME_LEN_Cur-14) & 0xff) << 8) | ((FRAME_LEN_Cur-14) >> 8);
#ifdef SLT_DOS
	if (DbgPrn_Bufdat) 
	    fprintf(fp_log, " Inf:%08lx ~ %08lx(%08lx) %08lx [Des:%d][loop:%d]\n", adr_srt, adr_end, adr_las, gdata, number, Loop);
#endif	    
	for (adr = adr_srt; adr < adr_end; adr+=4) {

        #ifdef SelectSimpleDA
		cnt++;
		if      ( cnt == 1 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat0; }
		else if ( cnt == 2 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat1; }
		else if ( cnt == 3 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat2; }
		else if ( cnt == 4 ) { gdata_bak = gdata; gdata = len | (len << 16);   }
        #endif
		rdata = ReadSOC_DD(adr);
		if (adr == adr_las) 
		    wp = wp & wp_lst_cur;
		    
		if ( (rdata & wp) != (gdata & wp) ) {
#ifdef SLT_DOS
            fprintf(fp_log, "\nError: Adr:%08lx[%3d] (%08lx) (%08lx:%08lx) [Des:%d][loop:%d]\n", adr, (adr - adr_srt) / 4, rdata, gdata, wp, number, Loop);
#endif
			for (index = 0; index < 6; index++) {
				rdata = ReadSOC_DD(adr);
#ifdef SLT_DOS                
				fprintf(fp_log, "Rep  : Adr:%08lx      (%08lx) (%08lx:%08lx) [Des:%d][loop:%d]\n", adr, rdata, gdata, wp, number, Loop);
#endif                
            }

			if ( DbgPrn_DumpMACCnt ) 
			    dump_mac_ROreg();
			    
			return( FindErr( Err_Check_Buf_Data ) );
		} // End if ( (rdata & wp) != (gdata & wp) )
#ifdef SLT_DOS		
		if ( DbgPrn_BufdatDetail ) 
		    fprintf(fp_log, " Adr:%08lx[%3d] (%08lx) (%08lx:%08lx) [Des:%d][loop:%d]\n", adr, (adr - adr_srt) / 4, rdata, gdata, wp, number, Loop);
#endif		    
        #ifdef SelectSimpleDA
		if ( cnt <= 4 ) 
		    gdata = gdata_bak;
        #endif

        #ifdef SelectSimpleData
        #else
		gdata += DATA_IncVal;
        #endif
		
		wp     = 0xffffffff;
	}
	return(0);
} // End char check_Data (ULONG desadr, LONG number)

//------------------------------------------------------------
char check_buf (int loopcnt) {
    int	    count;
    ULONG	desadr;
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("check_buf  : %d\n", Loop); 
	    Debug_delay();
    #endif

	for ( count = DES_NUMBER - 1; count >= 0; count-- ) {
		desadr = H_RDES_BASE + ( 16 * count ) + 12;
        //printf("%d:%08x\n", count, desadr);
		if (check_Data(desadr, count)) {
			check_int ("");
			
			return(1);
		}
	}
	if ( check_int ("") )
	    return(1);

	return(0);
} // End char check_buf (int loopcnt)

//------------------------------------------------------------
// Descriptor
//------------------------------------------------------------
void setup_txdes (ULONG desadr, ULONG bufbase) {
    ULONG	bufadr;
    ULONG	desval;
    int     count;
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("setup_txdes: %d\n", Loop); 
	    Debug_delay();
    #endif

	bufadr    = bufbase + ZeroCopy_OFFSET;
	
	if (TxDataEnable) {
		for (count = 0; count < DES_NUMBER; count++) {
			FRAME_LEN_Cur = FRAME_LEN[count];
			desval        = TDES_IniVal;
            #ifdef SLT_DOS	
			if (DbgPrn_FRAME_LEN) 
			    fprintf(fp_log, "[setup_txdes    ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", FRAME_LEN_Cur, count, Loop);
            #endif
			if (DbgPrn_BufAdr) 
			    printf("[loop:%4d][des:%4d][setup_txdes] %08lx\n", Loop, count, bufadr);
			    
			WriteSOC_DD( desadr + 0x04, 0      );
			WriteSOC_DD( desadr + 0x08, 0      );
			WriteSOC_DD( desadr + 0x0C, (bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
			if ( count == ( DES_NUMBER - 1 ) )
			    WriteSOC_DD( desadr       , desval | EOR_IniVal);
			else
			    WriteSOC_DD( desadr       , desval );

			bufadr += DMA_PakSize;
			desadr += 16;
		}
	} 
	else {
		WriteSOC_DD( desadr     , 0);
	}
} // End void setup_txdes (ULONG desadr, ULONG bufbase)

//------------------------------------------------------------
void setup_rxdes (ULONG desadr, ULONG bufbase) {
    ULONG	bufadr;
    ULONG	desval;    
    int     count;
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("setup_rxdes: %d\n", Loop); 
	    Debug_delay();
    #endif

	bufadr = bufbase+ZeroCopy_OFFSET;
	desval = RDES_IniVal;
	
	if ( RxDataEnable ) {
		for (count = 0; count < DES_NUMBER; count++) {
			if (DbgPrn_BufAdr) 
			    printf("[loop:%4d][des:%4d][setup_rxdes] %08lx\n", Loop, count, bufadr);
			WriteSOC_DD( desadr + 0x04, 0      );
			WriteSOC_DD( desadr + 0x08, 0      );
			WriteSOC_DD( desadr + 0x0C, ( bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
            if ( count == ( DES_NUMBER - 1 ) )
                WriteSOC_DD( desadr       , desval | EOR_IniVal   );
            else
			    WriteSOC_DD( desadr       , desval );

			desadr += 16;
			bufadr += DMA_PakSize;
		}
	} 
	else {
		WriteSOC_DD( desadr     , 0x80000000   );
	} // End if ( RxDataEnable )
} // End void setup_rxdes (ULONG desadr, ULONG bufbase)

//------------------------------------------------------------
// First setting TX and RX information
//------------------------------------------------------------
void setup_des (ULONG bufnum) {

	if ( DbgPrn_BufAdr ) {
	    printf ("setup_rxdes: %ld\n", bufnum); 
	    Debug_delay();
	}

    setup_txdes( H_TDES_BASE, GET_DMA_BASE_SETUP       );
    setup_rxdes( H_RDES_BASE, GET_DMA_BASE(0)          );
    
} // End void setup_des (ULONG bufnum)

//------------------------------------------------------------
// Move buffer point of TX and RX descriptor to next DMA buffer
//------------------------------------------------------------
void setup_des_loop (ULONG bufnum) {
    int	    count;
    ULONG	H_rx_desadr;
    ULONG	H_tx_desadr;
    ULONG	H_tx_bufadr;
    ULONG	H_rx_bufadr;

	if ( DbgPrn_BufAdr ) {
	    printf ("setup_rxdes_loop: %ld\n", bufnum); 
	    Debug_delay();
	}
	
	if (RxDataEnable) {
		H_rx_bufadr = GET_DMA_BASE( bufnum + 1 ) + ZeroCopy_OFFSET;
		H_rx_desadr = H_RDES_BASE;
//printf (" =====>setup_rxdes_loop: %ld [%lX]\n", bufnum, H_rx_bufadr);
		for (count = 0; count < DES_NUMBER; count++) {
			WriteSOC_DD(H_rx_desadr + 0x0C,  (H_rx_bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
			
			if (count == (DES_NUMBER - 1)) {
				WriteSOC_DD( H_rx_desadr, RDES_IniVal | EOR_IniVal );
			}                
			else {           
				WriteSOC_DD( H_rx_desadr, RDES_IniVal );
			}
			H_rx_bufadr += DMA_PakSize;
			H_rx_desadr += 16;
		}
	}
	
	if (TxDataEnable) {
		if (RxDataEnable) {
			H_tx_bufadr = GET_DMA_BASE( bufnum ) + ZeroCopy_OFFSET;
		} 
		else {
			H_tx_bufadr = GET_DMA_BASE( 0      ) + ZeroCopy_OFFSET;
		}
		H_tx_desadr = H_TDES_BASE;
//printf (" =====>setup_Txdes_loop: %ld [%lX]\n", bufnum, H_tx_bufadr); 		
		for (count = 0; count < DES_NUMBER; count++) {
			FRAME_LEN_Cur = FRAME_LEN[count];
			WriteSOC_DD( H_tx_desadr + 0x0C, ( H_tx_bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
			if (count == (DES_NUMBER - 1)) {
				WriteSOC_DD( H_tx_desadr, TDES_IniVal | EOR_IniVal );
			} 
			else {
				WriteSOC_DD( H_tx_desadr, TDES_IniVal );
			}
			H_tx_bufadr += DMA_PakSize;
			H_tx_desadr += 16;
		}
	}

	WriteSOC_DD( H_MAC_BASE + 0x18, 0x00000000 ); // Tx Poll
	WriteSOC_DD( H_MAC_BASE + 0x1c, 0x00000000 ); // Rx Poll
} // End void setup_des_loop (ULONG bufnum)

//------------------------------------------------------------
char check_des_header_Tx (char *type, ULONG adr, LONG desnum) {
	int    timeout = 0;
	ULONG  dat;
	
	dat = ReadSOC_DD(adr);
	
	while ( HWOwnTx(dat) ) {
	    // we will run again, if transfer has not been completed.
		if ( RxDataEnable && (++timeout > TIME_OUT_Des) ) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[%sTxDesOwn] Address %08lx = %08lx [Des:%d][loop:%d]\n", type, adr, dat, desnum, Loop);
            #endif			
			return(FindErr_Des(Check_Des_TxOwnTimeOut));
		}
		WriteSOC_DD(H_MAC_BASE + 0x18, 0x00000000);//Tx Poll
		WriteSOC_DD(H_MAC_BASE + 0x1c, 0x00000000);//Rx Poll
        
        #ifdef Delay_ChkTxOwn
		delay(Delay_ChkTxOwn);
        #endif
		dat = ReadSOC_DD(adr);
	}

	return(0);
} // End char check_des_header_Tx (char *type, ULONG adr, LONG desnum)

//------------------------------------------------------------
char check_des_header_Rx (char *type, ULONG adr, LONG desnum) {
    #ifdef CheckRxOwn
	int    timeout = 0;
	ULONG  dat;
	
	dat = ReadSOC_DD(adr);
	
	while ( HWOwnRx( dat ) ) {
	    // we will run again, if transfer has not been completed.
		if (TxDataEnable && (++timeout > TIME_OUT_Des)) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[%sRxDesOwn] Address %08lx = %08lx [Des:%d][loop:%d]\n", type, adr, dat, desnum, Loop);
            #endif
			return(FindErr_Des(Check_Des_RxOwnTimeOut));
		}

		WriteSOC_DD(H_MAC_BASE + 0x18, 0x00000000);//Tx Poll
		WriteSOC_DD(H_MAC_BASE + 0x1c, 0x00000000);//Rx Poll
        
        #ifdef Delay_ChkRxOwn
		delay(Delay_ChkRxOwn);
        #endif
		dat = ReadSOC_DD(adr);
	};

	Dat_ULONG = ReadSOC_DD( adr + 12 );

    #ifdef CheckRxLen
    #ifdef SLT_DOS
    if ( DbgPrn_FRAME_LEN ) 
	    fprintf(fp_log, "[%sRxDes          ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", type, (FRAME_LEN_Cur + 4), desnum, Loop);
    #endif	  
        
	if ((dat & 0x3fff) != (FRAME_LEN_Cur + 4)) {
        #ifdef SLT_DOS
		fprintf(fp_log, "[%sRxDes] Error Frame Length %08lx:%08lx %08lx(%4d/%4d) [Des:%d][loop:%d]\n",   type, adr, dat, Dat_ULONG, (dat & 0x3fff), (FRAME_LEN_Cur + 4), desnum, Loop);
        #endif
		FindErr_Des(Check_Des_FrameLen);
	}
    #endif // End CheckRxLen

    #ifdef CheckRxErr
	if (dat & 0x00040000) {
        #ifdef SLT_DOS
		fprintf(fp_log, "[%sRxDes] Error RxErr        %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, dat, Dat_ULONG, desnum, Loop);
        #endif
        FindErr_Des(Check_Des_RxErr);
	}
    #endif // End CheckRxErr

    #ifdef CheckOddNibble
	if (dat & 0x00400000) {
        #ifdef SLT_DOS
		fprintf(fp_log, "[%sRxDes] Odd Nibble         %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, dat, Dat_ULONG, desnum, Loop);
        #endif
		FindErr_Des(Check_Des_OddNibble);
	}
    #endif // End CheckOddNibble

    #ifdef CheckCRC
	if (dat & 0x00080000) {
        #ifdef SLT_DOS
		fprintf(fp_log, "[%sRxDes] Error CRC          %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, dat, Dat_ULONG, desnum, Loop);
        #endif
		FindErr_Des(Check_Des_CRC);
	}
    #endif // End CheckCRC

    #ifdef CheckRxFIFOFull
	if (dat & 0x00800000) {
		#ifdef SLT_DOS
        fprintf(fp_log, "[%sRxDes] Error Rx FIFO Full %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, dat, Dat_ULONG, desnum, Loop);
        #endif
		FindErr_Des(Check_Des_RxFIFOFull);
	}
    #endif // End CheckRxFIFOFull
    
    //	if (check_int ("")) {return(1);}
    #endif // End CheckRxOwn

	if (Err_Flag) 
	    return(1);
	else          
	    return(0);
} // End char check_des_header_Rx (char *type, ULONG adr, LONG desnum)

//------------------------------------------------------------
char check_des (ULONG bufnum, int checkpoint) {
    int     desnum;    
    ULONG	H_rx_desadr;
    ULONG	H_tx_desadr;
    ULONG	H_tx_bufadr;
    ULONG	H_rx_bufadr;
    
    #ifdef Delay_DesGap
    ULONG	dly_cnt = 0;
    ULONG	dly_max = Delay_CntMaxIncVal;
    #endif
    
	#ifdef  DbgPrn_FuncHeader
	    printf ("check_des  : %d(%d)\n", Loop, checkpoint); 
	    Debug_delay();
    #endif
    
	// Fire the engine to send and recvice
	WriteSOC_DD( H_MAC_BASE + 0x18, 0x00000000 );//Tx Poll
	WriteSOC_DD( H_MAC_BASE + 0x1c, 0x00000000 );//Rx Poll
	
    #ifdef SelectSimpleDes
    #else
    if ( IEEETesting == 1 ) {
        // IEEE test mode, there is the same data in every lan packet
        H_tx_bufadr = GET_DMA_BASE_SETUP;
        H_rx_bufadr = GET_DMA_BASE(0);
    }
    else {
	    H_rx_bufadr = GET_DMA_BASE( bufnum + 1 ) + ZeroCopy_OFFSET;
	    
	    if (RxDataEnable) {
	    	H_tx_bufadr = GET_DMA_BASE(bufnum  ) + ZeroCopy_OFFSET;
	    } 
	    else {
	    	H_tx_bufadr = GET_DMA_BASE( 0      ) + ZeroCopy_OFFSET;
	    }
	}
    #endif

	H_rx_desadr = H_RDES_BASE;
	H_tx_desadr = H_TDES_BASE;

    #ifdef Delay_DES
	delay(Delay_DES);
    #endif
	
	for (desnum = 0; desnum < DES_NUMBER; desnum++) {
		if ( DbgPrn_BufAdr ) 
		    printf( "[loop:%4d][des:%4d][check_des  ] %08lx %08lx [%08lx] [%08lx]\n", Loop, desnum, ( H_tx_desadr ), ( H_rx_desadr ), ReadSOC_DD( H_tx_desadr + 12 ), ReadSOC_DD( H_rx_desadr + 12 ) );

		//[Delay]--------------------
        #ifdef Delay_DesGap
		if ( dly_cnt++ > 3 ) {
			switch ( rand() % 12 ) {
				case 1 : dly_max = 00000; break;
				case 3 : dly_max = 20000; break;
				case 5 : dly_max = 40000; break;
				case 7 : dly_max = 60000; break;
				defaule: dly_max = 70000; break;
			}
			
			dly_max += ( rand() % 4 ) * 14321;

			while (dly_cnt < dly_max) {
			    dly_cnt++;
			}
			
			dly_cnt = 0;
		} 
		else {
//			timeout = 0;
//			while (timeout < 50000) {timeout++;};
		}
        #endif // End Delay_DesGap
        
		//[Check Owner Bit]--------------------
		FRAME_LEN_Cur = FRAME_LEN[desnum];
#ifdef SLT_DOS		
		if ( DbgPrn_FRAME_LEN ) 
		    fprintf(fp_log, "[check_des      ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]%d\n", FRAME_LEN_Cur, desnum, Loop, checkpoint);
#endif		    
//		if (BurstEnable) {
//			if (check_des_header_Tx("", H_tx_desadr, desnum)) {CheckDesFail_DesNum = desnum; return(1);}
//		} else {
//			if (check_des_header_Rx("", H_rx_desadr, desnum)) {CheckDesFail_DesNum = desnum; return(1);}
//			if (check_des_header_Tx("", H_tx_desadr, desnum)) {CheckDesFail_DesNum = desnum; return(1);}
//		}
        
        // Check the description of Tx and Rx
		if ( RxDataEnable && check_des_header_Rx("", H_rx_desadr, desnum) ) {
		    CheckDesFail_DesNum = desnum; 

		    return(1);
		}
		if ( TxDataEnable && check_des_header_Tx("", H_tx_desadr, desnum) ) {
		    CheckDesFail_DesNum = desnum; 

		    return(1);
		}
//		else {
//			printf(" %d                        \r", desnum);
//		}

        #ifdef SelectSimpleDes
        #else
		if ( !checkpoint ) {
		    // Setting buffer address to description of Tx and Rx on next stage
		    
//			if (!BurstEnable) {
//				WriteSOC_DD( H_rx_desadr + 0x0C, (H_rx_bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) );
//				WriteSOC_DD( H_tx_desadr + 0x0C, (H_tx_bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) );
//			}
//
//			if ( desnum == (DES_NUMBER - 1) ) {
//				WriteSOC_DD( H_rx_desadr, RDES_IniVal | EOR_IniVal );
//				WriteSOC_DD( H_tx_desadr, TDES_IniVal | EOR_IniVal );
//			} 
//          else {         
//				WriteSOC_DD( H_rx_desadr, RDES_IniVal );
//				WriteSOC_DD( H_tx_desadr, TDES_IniVal );
//			}
//			WriteSOC_DD( H_MAC_BASE+0x18, 0x00000000 ); //Tx Poll
//			WriteSOC_DD( H_MAC_BASE+0x1c, 0x00000000 ); //Rx Poll

			if ( RxDataEnable ) {
				WriteSOC_DD( H_rx_desadr + 0x0C, (H_rx_bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
				
				if ( desnum == (DES_NUMBER - 1) ) {
					WriteSOC_DD( H_rx_desadr, RDES_IniVal | EOR_IniVal );
				} else {         
					WriteSOC_DD( H_rx_desadr, RDES_IniVal );
				}
			}
			if ( TxDataEnable ) {
				WriteSOC_DD( H_tx_desadr + 0x0C, (H_tx_bufadr + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
				if ( desnum == (DES_NUMBER - 1) ) {
					WriteSOC_DD( H_tx_desadr, TDES_IniVal | EOR_IniVal );
				}                
				else {           
					WriteSOC_DD( H_tx_desadr, TDES_IniVal );
				}
			}
			WriteSOC_DD( H_MAC_BASE + 0x18, 0x00000000 ); //Tx Poll
			WriteSOC_DD( H_MAC_BASE + 0x1c, 0x00000000 ); //Rx Poll
		}
		H_rx_bufadr += DMA_PakSize;
		H_tx_bufadr += DMA_PakSize;
        #endif // End SelectSimpleDes
		
		H_rx_desadr += 16;
		H_tx_desadr += 16;
	} // End for (desnum = 0; desnum < DES_NUMBER; desnum++)
	
	return(0);
} // End char check_des (ULONG bufnum, int checkpoint)
//#endif

//------------------------------------------------------------
// Print
//------------------------------------------------------------
void PrintMode (void) {
	if (Enable_MAC34) printf ("run_mode[dec]    | 0->MAC1 1->MAC2 2->MAC3 3->MAC4\n");
	else              printf ("run_mode[dec]    | 0->MAC1 1->MAC2\n");
}

//------------------------------------------------------------
void PrintSpeed (void) {
	printf ("speed[dec]       | 0->1G   1->100M 2->10M  3->all speed (default:%3d)\n", DEF_SPEED);
}

//------------------------------------------------------------
void PrintCtrl (void) {
	    printf ("ctrl[dec]        | bit0~2: Reserved\n");
	    printf ("(default:%3d)    | bit3  : 1->Enable PHY init     0->Disable PHY init\n", GCtrl);
	    printf ("                 | bit4  : 1->Enable PHY int-loop 0->Disable PHY int-loop\n");
	    printf ("                 | bit5  : 1->Ignore PHY ID       0->Check PHY ID\n");
	if (AST2400) {
		printf ("                 | bit6  : 1->Enable MAC int-loop 0->Disable MAC int-loop\n");
	}
}

//------------------------------------------------------------
void PrintLoop (void) {
	printf ("loop_max[dec]    | 1G  : 20 will run 1 sec (default:%3d)\n", DEF_LOOP_MAX * 20);
	printf ("                 | 100M:  2 will run 1 sec (default:%3d)\n", DEF_LOOP_MAX * 2);
	printf ("                 | 10M :  1 will run 1 sec (default:%3d)\n", DEF_LOOP_MAX);
}

//------------------------------------------------------------
void PrintTest (void) {
	if ( ModeSwitch == MODE_NSCI ) {
		printf ("test_mode[dec]   | 0: NCSI configuration with    Disable_Channel request\n");
		printf ("(default:%3d)    | 1: NCSI configuration without Disable_Channel request\n", DEF_TESTMODE);
	}
	else {
		printf ("test_mode[dec]   | 0: Tx/Rx frame checking\n");
		printf ("(default:%3d)    | 1: Tx output 0xff frame\n", DEF_TESTMODE);
		printf ("                 | 2: Tx output 0x55 frame\n");
		printf ("                 | 3: Tx output random frame\n");
		printf ("                 | 4: Tx output ARP frame\n");
		printf ("                 | 5: Tx output user defined value frame (default:0x%8x)\n", DEF_USER_DEF_PACKET_VAL);
	} // End if ( ModeSwitch == MODE_NSCI )
	
	if (AST2300) {
		printf ("                 | 6: IO timing testing\n");
		printf ("                 | 7: IO timing/strength testing\n");
	}
}

//------------------------------------------------------------
void PrintPHYAdr (void) {
	printf ("phy_adr[dec]     | 0~31: PHY Address (default:%d)\n", DEF_PHY_ADR);
}

//------------------------------------------------------------
void PrintIOTimingBund (void) {
	printf ("IO margin[dec]   | 0/1/3/5 (default:%d)\n", DEF_IOTIMINGBUND);
}

//------------------------------------------------------------
void PrintPakNUm (void) {
	printf ("package_num[dec] | 1~ 8: Total Number of NCSI Package (default:%d)\n", DEF_PACKAGE2NUM);
}

//------------------------------------------------------------
void PrintChlNUm (void) {
	printf ("channel_num[dec] | 1~32: Total Number of NCSI Channel (default:%d)\n", DEF_CHANNEL2NUM);
}

//------------------------------------------------------------

void Print_Header (BYTE option) {
    
    FILE_VAR
    
    GET_OBJ( option )
    
	if      (GSpeed_sel[0]) PRINT(OUT_OBJ " 1G   ");
	else if (GSpeed_sel[1]) PRINT(OUT_OBJ " 100M ");
	else                    PRINT(OUT_OBJ " 10M  ");
	    
	switch (TestMode) {
		case 0 : PRINT(OUT_OBJ "Tx/Rx frame checking       \n"          ); break;
		case 1 : PRINT(OUT_OBJ "Tx output 0xff frame       \n"          ); break;
		case 2 : PRINT(OUT_OBJ "Tx output 0x55 frame       \n"          ); break;
		case 3 : PRINT(OUT_OBJ "Tx output random frame     \n"          ); break;
		case 4 : PRINT(OUT_OBJ "Tx output ARP frame        \n"          ); break;
		case 5 : PRINT(OUT_OBJ "Tx output 0x%08lx frame     \n", UserDVal); break;
		case 6 : PRINT(OUT_OBJ "IO delay testing           \n"          ); break;
		case 7 : PRINT(OUT_OBJ "IO delay testing(Strength) \n"          ); break;
		case 8 : PRINT(OUT_OBJ "Tx frame                   \n"          ); break;
		case 9 : PRINT(OUT_OBJ "Rx frame checking          \n"          ); break;
	}
}

//------------------------------------------------------------
void PrintIO_Header (BYTE option) {
        
    FILE_VAR
    
    GET_OBJ( option )
    
	if ( IOStrength ) {
		if      (GSpeed_sel[0]) PRINT(OUT_OBJ "[Strength %ld][1G  ]========================================\n", IOStr_i);
		else if (GSpeed_sel[1]) PRINT(OUT_OBJ "[Strength %ld][100M]========================================\n", IOStr_i);
		else                    PRINT(OUT_OBJ "[Strength %ld][10M ]========================================\n", IOStr_i);
	} else {
		if      (GSpeed_sel[0]) PRINT(OUT_OBJ "[1G  ]========================================\n");
		else if (GSpeed_sel[1]) PRINT(OUT_OBJ "[100M]========================================\n");
		else                    PRINT(OUT_OBJ "[10M ]========================================\n");
	}

#ifdef Enable_Old_Style
	if (Enable_RMII) PRINT(OUT_OBJ "Tx:SCU48[   %2d]=   ", IOdly_out_shf);
	else             PRINT(OUT_OBJ "Tx:SCU48[%2d:%2d]=   ", IOdly_out_shf+3, IOdly_out_shf);
	    
	for (IOdly_j = IOdly_out_str; IOdly_j <= IOdly_out_end; IOdly_j+=IOdly_incval) {
		IOdly_out = valary[IOdly_j];
		PRINT(OUT_OBJ "%2x", IOdly_out);
	}

	PRINT(OUT_OBJ "\n                   ");
	for (IOdly_j = IOdly_out_str; IOdly_j <= IOdly_out_end; IOdly_j+=IOdly_incval) {
		if (IOdly_out_reg_idx == IOdly_j) PRINT(OUT_OBJ " |");
		else                              PRINT(OUT_OBJ "  ");
	}
#else
	PRINT(OUT_OBJ "Rx:SCU48[%2d:%2d]=   ", IOdly_in_shf+3, IOdly_in_shf);
	
	for (IOdly_i = IOdly_in_str; IOdly_i <= IOdly_in_end; IOdly_i+=IOdly_incval) {
		IOdly_in = valary[IOdly_i];
		PRINT(OUT_OBJ "%2x", IOdly_in);
	}

	PRINT(OUT_OBJ "\n                   ");
	for (IOdly_i = IOdly_in_str; IOdly_i <= IOdly_in_end; IOdly_i+=IOdly_incval) {
		if (IOdly_in_reg_idx == IOdly_i) PRINT(OUT_OBJ " |");
		else                             PRINT(OUT_OBJ "  ");
	}
#endif

	PRINT(OUT_OBJ "\n");
} // End void PrintIO_Header (BYTE option)

//------------------------------------------------------------
void PrintIO_LineS (BYTE option) {
            
    FILE_VAR
    
    GET_OBJ( option )
    
    
#ifdef Enable_Old_Style
	if (IOdly_in_reg == IOdly_in) {
		PRINT(OUT_OBJ "Rx:SCU48[%2d:%2d]=%01x:-", IOdly_in_shf+3, IOdly_in_shf, IOdly_in);
	} 
	else {
		PRINT(OUT_OBJ "Rx:SCU48[%2d:%2d]=%01x: ", IOdly_in_shf+3, IOdly_in_shf, IOdly_in);
	}
#else
	if (Enable_RMII) {
		if (IOdly_out_reg == IOdly_out) {
			PRINT(OUT_OBJ "Tx:SCU48[   %2d]=%01x:-", IOdly_out_shf, IOdly_out);
		} 
		else {
			PRINT(OUT_OBJ "Tx:SCU48[   %2d]=%01x: ", IOdly_out_shf, IOdly_out);
		}
	} else {
		if (IOdly_out_reg == IOdly_out) {
			PRINT(OUT_OBJ "Tx:SCU48[%2d:%2d]=%01x:-", IOdly_out_shf+3, IOdly_out_shf, IOdly_out);
		} 
		else {
			PRINT(OUT_OBJ "Tx:SCU48[%2d:%2d]=%01x: ", IOdly_out_shf+3, IOdly_out_shf, IOdly_out);
		}
	}
#endif
} // void PrintIO_LineS (BYTE option)

//------------------------------------------------------------
void PrintIO_Line (BYTE option) {
            
    FILE_VAR
    
    GET_OBJ( option )
    
	if ( ( IOdly_in_reg == IOdly_in ) && ( IOdly_out_reg == IOdly_out ) ) {
		if (dlymap[IOdly_i][IOdly_j]) PRINT(OUT_OBJ " X");
		else                          PRINT(OUT_OBJ " O");
	} 
	else {
		if (dlymap[IOdly_i][IOdly_j]) PRINT(OUT_OBJ " x");
		else                          PRINT(OUT_OBJ " o");
	}
} // End void PrintIO_Line (BYTE option)

//------------------------------------------------------------
void PrintIO_Line_LOG (void) {
#ifndef SLT_UBOOT      
#ifdef Enable_Old_Style
	if (Enable_RMII) fprintf(fp_log, "\nTx:SCU48[   %2d]=%2x, ", IOdly_out_shf, IOdly_out);
	else             fprintf(fp_log, "\nTx:SCU48[%2d:%2d]=%2x, ", IOdly_out_shf+3, IOdly_out_shf, IOdly_out);

	fprintf(fp_log, "Rx:SCU48[%2d:%2d]=%01x: ", IOdly_in_shf+3, IOdly_in_shf, IOdly_in);

	if (dlymap[IOdly_i][IOdly_j]) fprintf(fp_log, " X\n");
	else                          fprintf(fp_log, " O\n");
#else
	fprintf(fp_log, "\nRx:SCU48[%2d:%2d]=%2x, ", IOdly_in_shf+3, IOdly_in_shf, IOdly_in);

	if (Enable_RMII) fprintf(fp_log, "Tx:SCU48[   %2d]=%01x: ", IOdly_out_shf, IOdly_out);
	else             fprintf(fp_log, "Tx:SCU48[%2d:%2d]=%01x: ", IOdly_out_shf+3, IOdly_out_shf, IOdly_out);

	if (dlymap[IOdly_i][IOdly_j]) fprintf(fp_log, " X\n");
	else                          fprintf(fp_log, " O\n");
#endif
#endif    
}

//------------------------------------------------------------
// main
//------------------------------------------------------------
void Calculate_LOOP_CheckNum (void) {
    
#define ONE_MBYTE    1048576

  #ifdef CheckDataEveryTime
	LOOP_CheckNum   = 1;
  #else
	if (IOTiming || IOTimingBund || (GSpeed == SET_1G_100M_10MBPS)) {
		LOOP_CheckNum = LOOP_MAX;
	} 
	else {
		switch ( GSpeed ) {
			case SET_1GBPS    : CheckBuf_MBSize =  MOVE_DATA_MB_SEC      ; break; // 1G
			case SET_100MBPS  : CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 3); break; // 100M ~ 1G / 8
			case SET_10MBPS   : CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 6); break; // 10M  ~ 1G / 64
		}
		LOOP_CheckNum = ( CheckBuf_MBSize / ( ((DES_NUMBER * DMA_PakSize) / ONE_MBYTE ) + 1) );
	}
  #endif
}

//------------------------------------------------------------
void TestingSetup (void) {
	#ifdef  DbgPrn_FuncHeader
	    printf ("TestingSetup\n"); 
	    Debug_delay();
    #endif

  #ifdef SLT_UBOOT
  #else
    #ifdef Rand_Sed
	srand((unsigned) Rand_Sed);
    #else
	srand((unsigned) timestart);
    #endif
  #endif              
  
    //[Disable VGA]--------------------
    #ifdef Disable_VGA
	if ( LOOP_INFINI & ~(BurstEnable || IOTiming) ) {
		VGAModeVld = 1;
		outp(0x3d4, 0x17);
		VGAMode = inp(0x3d5);
		outp(0x3d4, 0x17);
		outp(0x3d5, 0);
    }
    #endif
    
    //[Setup]--------------------
	setup_framesize();
	setup_buf();
}

//------------------------------------------------------------
// Return 1 ==> fail
// Return 0 ==> PASS
//------------------------------------------------------------
char TestingLoop (ULONG loop_checknum) {
    char	checkprd;
    char	looplast;
    char	checken;
    
    #ifdef SLT_UBOOT
    #else
        clock_t	timeold;
    #endif

	#ifdef  DbgPrn_FuncHeader
	    printf ("TestingLoop: %d\n", Loop); 
	    Debug_delay();
    #endif
	
	if ( DbgPrn_DumpMACCnt ) 
	    dump_mac_ROreg();

	//[Setup]--------------------
	Loop     = 0;
	checkprd = 0;
	checken  = 0;
	looplast = 0;

	setup_des( 0 );

    #ifdef SLT_UBOOT
    #else
	timeold = clock();
    #endif

	while ( (Loop < LOOP_MAX) || LOOP_INFINI ) {
		looplast = !LOOP_INFINI && (Loop == LOOP_MAX - 1);
        
        #ifdef CheckRxBuf
		if (!BurstEnable) {
			checkprd = ((Loop % loop_checknum) == (loop_checknum - 1));
		}
		checken = looplast | checkprd;
        #endif
    
		if ( DataDelay & ( Loop == 0 ) ) {
			printf ("Press any key to start...\n");
			GET_CAHR();
		}
		
#ifdef  DbgPrn_FuncHeader
        if ( DbgPrn_BufAdr ) {
            printf ("for start ======> %d/%d(%d) looplast:%d checkprd:%d checken:%d\n", Loop, LOOP_MAX, LOOP_INFINI, looplast, checkprd, checken); 
            Debug_delay();
        }
#endif
        
		//[Check DES]--------------------
		if ( check_des(Loop, checken) ) {
		    //descriptor error
            #ifdef CheckRxBuf
			DES_NUMBER = CheckDesFail_DesNum + 1;
			if ( checkprd ) {
				check_buf(loop_checknum);
			} 
			else {
				check_buf((LOOP_MAX % loop_checknum));
			}
			DES_NUMBER = DES_NUMBER_Org;
            #endif

			if (DbgPrn_DumpMACCnt) 
			    dump_mac_ROreg();
			    
			return(1);
		}

		//[Check Buf]--------------------
		if ( RxDataEnable && checken ) {
            #ifdef SLT_UBOOT
            #else
            timeused = (clock() - timeold) / (double) CLK_TCK;
            #endif
			
			if ( checkprd ) {
                #ifdef SLT_DOS
                #else
                  #ifdef SLT_UBOOT
                  #else
                printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)DES_NUMBER * Avg_frame_len * 8.0) / ((double)timeused * 1000000.0), timeused);
                fprintf(fp_log, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)DES_NUMBER * Avg_frame_len * 8.0) / ((double)timeused * 1000000.0), timeused);
                  #endif
                #endif
                
                #ifdef CheckRxBuf
                if ( check_buf( loop_checknum ) ) 
                    return(1);
                #endif
			} 
			else {
                #ifdef SLT_DOS
                #else
                  #ifdef SLT_UBOOT
                  #else
                printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (LOOP_MAX % loop_checknum), ((double)(LOOP_MAX % loop_checknum) * (double)DES_NUMBER * Avg_frame_len * 8.0) / ((double)timeused * 1000000.0), timeused);
                fprintf(fp_log, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (LOOP_MAX % loop_checknum), ((double)(LOOP_MAX % loop_checknum) * (double)DES_NUMBER * Avg_frame_len * 8.0) / ((double)timeused * 1000000.0), timeused);
                  #endif
                #endif
                
                #ifdef CheckRxBuf
                if ( check_buf( ( LOOP_MAX % loop_checknum ) ) ) 
                    return(1);                
                #endif
			} // End if ( checkprd )
            
            #ifdef SelectSimpleDes
            #else
            if ( !looplast ) 
                setup_des_loop( Loop );
            #endif

            #ifdef SLT_DOS
            #else
              #ifdef SLT_UBOOT
              #else
        	timeold = clock();
              #endif
            #endif

		} // End if ( RxDataEnable && checken )
		
        #ifdef SelectSimpleDes
		if ( !looplast ) 
		    setup_des_loop( Loop );
        #endif
        
		if ( LOOP_INFINI ) {
			    printf("===============> Loop: %d  \r", Loop);
		} 
		else if (TestMode == 0) {
			if (!(DbgPrn_BufAdr || IOTimingBund)) 
			    printf(" %d                        \r", Loop);
//			switch (Loop % 4) {
//				case 0x00: printf("| %d                        \r", Loop); break;
//				case 0x01: printf("/ %d                        \r", Loop); break;
//				case 0x02: printf("- %d                        \r", Loop); break;
//				default  : printf("\ %d                        \r", Loop); break;
//			}
		}

        if ( DbgPrn_BufAdr ) {
            printf ("for end   ======> %d/%d(%d)\n", Loop, LOOP_MAX, LOOP_INFINI); 
            Debug_delay();
        }

		Loop++;
	}  // End while ((Loop < LOOP_MAX) || LOOP_INFINI)
	
	Loop_rl[GSpeed_idx] = Loop;
	
	return(0);
} // End char TestingLoop (ULONG loop_checknum)
