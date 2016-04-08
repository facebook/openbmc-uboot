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
#define NCSI_C
static const char ThisFile[] = "ncsi.c";

#include "swfunc.h"

#ifdef SLT_UBOOT
  #include <common.h>
  #include <command.h>
  #include "comminf.h"
  #include "ncsi.h"
  #include "io.h"
#endif
#ifdef SLT_DOS
  #include <stdio.h>
  #include <stdlib.h>
  #include <conio.h>
  #include <string.h>
  #include "comminf.h"
  #include "ncsi.h"
  #include "io.h"
#endif

NCSI_Command_Packet     NCSI_Request_SLT;
NCSI_Response_Packet    NCSI_Respond_SLT;
int             InstanceID;
int             NCSI_RxTimeOutScale;
ULONG           NCSI_RxDesBase;
ULONG           NCSI_TxDWBUF[512];
ULONG           NCSI_RxDWBUF[512];
char            NCSI_CommandStr[512];
unsigned char   *NCSI_TxByteBUF;
unsigned char   *NCSI_RxByteBUF;
unsigned char   NCSI_Payload_Data[16];
unsigned long   Payload_Checksum_NCSI = 0x00000000;
ULONG           select_flag[MAX_PACKAGE_NUM];

ULONG DWSwap_SLT (ULONG in) {
	return( ((in & 0xff000000) >> 24)
	      | ((in & 0x00ff0000) >>  8)
	      | ((in & 0x0000ff00) <<  8)
	      | ((in & 0x000000ff) << 24)
	      );
}
USHORT WDSwap_SLT (USHORT in) {
	return( ((in & 0xff00) >>  8)
	      | ((in & 0x00ff) <<  8)
	      );
}

//------------------------------------------------------------
int FindErr_NCSI (int value) {
	NCSI_LinkFail_Val = NCSI_LinkFail_Val | value;
	Err_Flag          = Err_Flag | Err_NCSI_LinkFail;
	if ( DbgPrn_ErrFlg ) 
	    printf ("\nErr_Flag: [%08lx] NCSI_LinkFail_Val: [%08lx]\n", Err_Flag, NCSI_LinkFail_Val);

	return(1);
}

//------------------------------------------------------------
// PHY IC(NC-SI)
//------------------------------------------------------------
void ncsi_respdump ( NCSI_Response_Packet *in ) {
    printf ("DA             : %02x %02x %02x %02x %02x %02x\n", in->DA[5], in->DA[4], in->DA[3], in->DA[2], in->DA[1], in->DA[0]);
    printf ("SA             : %02x %02x %02x %02x %02x %02x\n", in->SA[5], in->SA[4], in->SA[3], in->SA[2], in->SA[1], in->SA[0]);
    printf ("EtherType      : %04x\n", in->EtherType       );//DMTF NC-SI
    printf ("MC_ID          : %02x\n", in->MC_ID           );//Management Controller should set this field to 0x00
    printf ("Header_Revision: %02x\n", in->Header_Revision );//For NC-SI 1.0 spec, this field has to set 0x01
//  printf ("Reserved_1     : %02x\n", in->Reserved_1      ); //Reserved has to set to 0x00
    printf ("IID            : %02x\n", in->IID             );//Instance ID
    printf ("Command        : %02x\n", in->Command         );
    printf ("Channel_ID     : %02x\n", in->Channel_ID      );
    printf ("Payload_Length : %04x\n", in->Payload_Length  );//Payload Length = 12 bits, 4 bits are reserved
//  printf ("Reserved_2     : %04x\n", in->Reserved_2      );
//  printf ("Reserved_3     : %04x\n", in->Reserved_3      );
//  printf ("Reserved_4     : %04x\n", in->Reserved_4      );
//  printf ("Reserved_5     : %04x\n", in->Reserved_5      );
    printf ("Response_Code  : %04x\n", in->Response_Code   );
    printf ("Reason_Code    : %04x\n", in->Reason_Code     );
    printf ("Payload_Data   : %02x%02x%02x%02x\n", in->Payload_Data[ 3], in->Payload_Data[ 2], in->Payload_Data[ 1], in->Payload_Data[ 0]);
//  printf ("Payload_Data   : %02x%02x%02x%02x\n", in->Payload_Data[ 7], in->Payload_Data[ 6], in->Payload_Data[ 5], in->Payload_Data[ 4]);
//  printf ("Payload_Data   : %02x%02x%02x%02x\n", in->Payload_Data[11], in->Payload_Data[10], in->Payload_Data[ 9], in->Payload_Data[ 8]);
//  printf ("Payload_Data   : %02x%02x%02x%02x\n", in->Payload_Data[15], in->Payload_Data[14], in->Payload_Data[13], in->Payload_Data[12]);
//  printf ("Payload_Data   : %02x%02x%02x%02x\n", in->Payload_Data[19], in->Payload_Data[18], in->Payload_Data[17], in->Payload_Data[16]);
//  printf ("Payload_Data   : %02x%02x%02x%02x\n", in->Payload_Data[23], in->Payload_Data[22], in->Payload_Data[21], in->Payload_Data[20]);
}

//------------------------------------------------------------
void NCSI_Struct_Initialize_SLT (void) {
    int i;

    ULONG    NCSI_RxDatBase;

    InstanceID = 0;
    NCSI_RxTimeOutScale = 1;

    for (i = 0; i < 6; i++) {
        NCSI_Request_SLT.DA[i] = 0xFF;
    }

    for (i = 0; i < 6; i++) {
//      NCSI_Request.SA[i] = i<<2;
        NCSI_Request_SLT.SA[i] = SA[i];
    }

    NCSI_Request_SLT.EtherType       = WDSwap_SLT(0x88F8); // EtherType = 0x88F8 (DMTF NC-SI) page 50, table 8, NC-SI spec. version 1.0.0
    NCSI_Request_SLT.MC_ID           = 0;
    NCSI_Request_SLT.Header_Revision = 0x01;
    NCSI_Request_SLT.Reserved_1      = 0;
    NCSI_Request_SLT.Reserved_2      = 0;
    NCSI_Request_SLT.Reserved_3      = 0;

    NCSI_TxByteBUF = (unsigned char *) &NCSI_TxDWBUF[0];
    NCSI_RxByteBUF = (unsigned char *) &NCSI_RxDWBUF[0];

    NCSI_RxDesBase = H_RDES_BASE;
    NCSI_RxDatBase = NCSI_RxDMA_BASE;
    
    for (i = 0; i < NCSI_RxDESNum - 1; i++) {
        WriteSOC_DD( ( NCSI_RxDesBase +  0 ), 0x00000000     );
        WriteSOC_DD( ( NCSI_RxDesBase +  4 ), 0x00000000     );
        WriteSOC_DD( ( NCSI_RxDesBase +  8 ), 0x00000000     );
        WriteSOC_DD( ( NCSI_RxDesBase + 0x0C ), (NCSI_RxDatBase + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730
        NCSI_RxDesBase += 16;
        NCSI_RxDatBase += NCSI_RxDMA_PakSize;
    }
    WriteSOC_DD( ( NCSI_RxDesBase +  0 ), EOR_IniVal     );
    WriteSOC_DD( ( NCSI_RxDesBase +  4 ), 0x00000000     );
    WriteSOC_DD( ( NCSI_RxDesBase +  8 ), 0x00000000     );
    WriteSOC_DD( ( NCSI_RxDesBase + 0x0C ), (NCSI_RxDatBase + CPU_BUS_ADDR_SDRAM_OFFSET) ); // 20130730

    NCSI_RxDesBase = H_RDES_BASE;
}

//------------------------------------------------------------
void Calculate_Checksum_NCSI (unsigned char *buffer_base, int Length) {
    ULONG	CheckSum = 0;   
    ULONG	Data;
    ULONG	Data1;
    int     i;
    
    // Calculate checksum is from byte 14 of ethernet Haeder and Control packet header
    // Page 50, NC-SI spec. ver. 1.0.0 form DMTF
    for (i = 14; i < Length; i += 2 ) {
        Data      = buffer_base[i];
        Data1     = buffer_base[i + 1];
        CheckSum += ((Data << 8) + Data1);
    }
    Payload_Checksum_NCSI = DWSwap_SLT(~(CheckSum) + 1); //2's complement
}

//------------------------------------------------------------
// return 0: it is PASS
// return 1: it is FAIL
//------------------------------------------------------------
char NCSI_Rx_SLT (unsigned char command) {
    
#define NCSI_RX_RETRY_TIME  2    
    int     timeout = 0;
    int	    bytesize;
    int	    dwsize;
    int     i;
    int     retry   = 0;
    char    ret     = 1;

    ULONG   NCSI_RxDatBase;
    ULONG   NCSI_RxDesDat;
    ULONG   NCSI_RxData;

    
    do {
        WriteSOC_DD( ( H_MAC_BASE + 0x1C ), 0x00000000 );//Rx Poll
        
        do {
            NCSI_RxDesDat = ReadSOC_DD(NCSI_RxDesBase);
            if ( ++timeout > TIME_OUT_NCSI * NCSI_RxTimeOutScale ) {
                #ifdef SLT_DOS
                fprintf(fp_log, "[Cmd:%02X][NCSI-RxDesOwn] %08lX \n", command, NCSI_RxDesDat );
                #endif
                return( FindErr(Err_NCSI_Check_RxOwnTimeOut) );
            }
        } while( HWOwnRx(NCSI_RxDesDat) );
        
        #ifdef CheckRxErr
        if (NCSI_RxDesDat & 0x00040000) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[RxDes] Error RxErr        %08lx\n", NCSI_RxDesDat);
            #endif
            FindErr_Des(Check_Des_RxErr);
        }
        #endif
        
        #ifdef CheckOddNibble
        if (NCSI_RxDesDat & 0x00400000) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[RxDes] Odd Nibble         %08lx\n", NCSI_RxDesDat);
            #endif
            FindErr_Des(Check_Des_OddNibble);
        }
        #endif
        
        #ifdef CheckCRC
        if (NCSI_RxDesDat & 0x00080000) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[RxDes] Error CRC          %08lx\n", NCSI_RxDesDat);
            #endif
            FindErr_Des(Check_Des_CRC);
        }
        #endif
        
        #ifdef CheckRxFIFOFull
        if (NCSI_RxDesDat & 0x00800000) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[RxDes] Error Rx FIFO Full %08lx\n", NCSI_RxDesDat);
            #endif
            FindErr_Des(Check_Des_RxFIFOFull);
        }
        #endif
        
        // Get point of RX DMA buffer
        NCSI_RxDatBase = ReadSOC_DD( NCSI_RxDesBase + 0x0C );
        NCSI_RxData    = ReadSOC_DD( NCSI_RxDatBase + 0x0C );
        
        if ( HWEOR( NCSI_RxDesDat ) ) {
            // it is last the descriptor in the receive Ring
            WriteSOC_DD( NCSI_RxDesBase     , EOR_IniVal    );
            NCSI_RxDesBase  = H_RDES_BASE;
        } 
        else {
            WriteSOC_DD( NCSI_RxDesBase     , 0x00000000    );
            NCSI_RxDesBase += 16;
        }
        
        // Get RX valid data in offset 00h of RXDS#0
        bytesize  = (NCSI_RxDesDat & 0x3fff);
        
        // Fill up to multiple of 4
        if ( ( bytesize % 4 ) != 0 )
            dwsize = ( bytesize >> 2 ) + 1;
        else
            dwsize = bytesize >> 2;
        
        #ifdef SLT_DOS
        if ( PrintNCSIEn ) 
            fprintf(fp_log ,"[Rx] %d bytes(%xh)\n", bytesize, bytesize);
        #endif
        
        for (i = 0; i < dwsize; i++) {
            NCSI_RxDWBUF[i] = ReadSOC_DD(NCSI_RxDatBase + ( i << 2 ));
            if ( PrintNCSIEn ) {
                if ( i == ( dwsize - 1 ) ) {
                    switch (bytesize % 4) {
                        case 0  : NCSI_RxDWBUF[i] = NCSI_RxDWBUF[i] & 0xffffffff; break;
                        case 3  : NCSI_RxDWBUF[i] = NCSI_RxDWBUF[i] & 0xffffff  ; break;
                        case 2  : NCSI_RxDWBUF[i] = NCSI_RxDWBUF[i] & 0xffff    ; break;
                        case 1  : NCSI_RxDWBUF[i] = NCSI_RxDWBUF[i] & 0xff      ; break;
                    }
                    #ifdef SLT_DOS
                    switch (bytesize % 4) {
                        case 0  : fprintf(fp_log ,"[Rx%02d]%08lx %08lx\n",             i, NCSI_RxDWBUF[i], DWSwap_SLT(NCSI_RxDWBUF[i])       ); break;
                        case 3  : fprintf(fp_log ,"[Rx%02d]--%06lx %06lx--\n",         i, NCSI_RxDWBUF[i], DWSwap_SLT(NCSI_RxDWBUF[i]) >>  8 ); break;
                        case 2  : fprintf(fp_log ,"[Rx%02d]----%04lx %04lx----\n",     i, NCSI_RxDWBUF[i], DWSwap_SLT(NCSI_RxDWBUF[i]) >> 16 ); break;
                        case 1  : fprintf(fp_log ,"[Rx%02d]------%02lx %02lx------\n", i, NCSI_RxDWBUF[i], DWSwap_SLT(NCSI_RxDWBUF[i]) >> 24 ); break;
                        default : fprintf(fp_log ,"[Rx%02d]error", i); break;
                    }
                    #endif
                } 
                else {
                    #ifdef SLT_DOS
                    fprintf(fp_log ,"[Rx%02d]%08lx %08lx\n", i, NCSI_RxDWBUF[i], DWSwap_SLT(NCSI_RxDWBUF[i]));
                    #endif
                }
            }
        } // End for (i = 0; i < dwsize; i++)
        
        // EtherType field of the response packet should be 0x88F8 
        if ((NCSI_RxData & 0xffff) == 0xf888) {
            memcpy (&NCSI_Respond_SLT, NCSI_RxByteBUF, bytesize);
            
            #ifdef SLT_DOS
            if ( PrintNCSIEn ) 
                fprintf(fp_log ,"[Rx IID:%2d]\n", NCSI_Respond_SLT.IID);
            #endif
            
            NCSI_Respond_SLT.EtherType      = WDSwap_SLT( NCSI_Respond_SLT.EtherType      );
            NCSI_Respond_SLT.Payload_Length = WDSwap_SLT( NCSI_Respond_SLT.Payload_Length );
            NCSI_Respond_SLT.Response_Code  = WDSwap_SLT( NCSI_Respond_SLT.Response_Code  );
            NCSI_Respond_SLT.Reason_Code    = WDSwap_SLT( NCSI_Respond_SLT.Reason_Code    );
            
            ret = 0;
            break;
        } 
        else {
            #ifdef SLT_DOS
            if ( PrintNCSIEn ) 
                fprintf(fp_log, "[Skip] Not NCSI Response: %08lx\n", NCSI_RxData);
            #endif
                
            retry++;
        }
    } while ( retry < NCSI_RX_RETRY_TIME );

    return( ret );
} // End char NCSI_Rx_SLT (void)

//------------------------------------------------------------
char NCSI_Tx (void) {
    int	   bytesize;
    int	   dwsize;
    int    i;
    int    timeout = 0;
    ULONG  NCSI_TxDesDat;
    
    // Header of NC-SI command format is 34 bytes. page 58, NC-SI spec. ver 1.0.0 from DMTF
    // The minimum size of a NC-SI package is 64 bytes.
    bytesize = 34 + WDSwap_SLT(NCSI_Request_SLT.Payload_Length);
    if ( bytesize < 64 ) {
        memset (NCSI_TxByteBUF + bytesize, 0, 60 - bytesize);
        bytesize = 64;
    }
    
    // Fill up to multiple of 4
//    dwsize = (bytesize + 3) >> 2;
    if ( ( bytesize % 4 ) != 0 )
        dwsize = ( bytesize >> 2 ) + 1;
    else
        dwsize = bytesize >> 2;
    
    #ifdef SLT_DOS
    if ( PrintNCSIEn ) 
        fprintf(fp_log ,"[Tx IID:%2d] %d bytes(%xh)\n", NCSI_Request_SLT.IID, bytesize, bytesize);
    #endif
    
    // Copy data to DMA buffer
    for (i = 0; i < dwsize; i++) {
        WriteSOC_DD( DMA_BASE + (i << 2), NCSI_TxDWBUF[i] );
        if ( PrintNCSIEn ) {
            if (i == (dwsize - 1)) {
                switch (bytesize % 4) {
                    case 0  : NCSI_TxDWBUF[i] = NCSI_TxDWBUF[i] & 0xffffffff; break;
                    case 3  : NCSI_TxDWBUF[i] = NCSI_TxDWBUF[i] & 0x00ffffff; break;
                    case 2  : NCSI_TxDWBUF[i] = NCSI_TxDWBUF[i] & 0x0000ffff; break;
                    case 1  : NCSI_TxDWBUF[i] = NCSI_TxDWBUF[i] & 0x000000ff; break;
                }
                #ifdef SLT_DOS
                switch (bytesize % 4) {
                    case 0  : fprintf(fp_log ,"[Tx%02d]%08x %08x\n",             i, NCSI_TxDWBUF[i], DWSwap_SLT( NCSI_TxDWBUF[i])       ); break;
                    case 3  : fprintf(fp_log ,"[Tx%02d]--%06x %06x--\n",         i, NCSI_TxDWBUF[i], DWSwap_SLT( NCSI_TxDWBUF[i]) >>  8 ); break;
                    case 2  : fprintf(fp_log ,"[Tx%02d]----%04x %04x----\n",     i, NCSI_TxDWBUF[i], DWSwap_SLT( NCSI_TxDWBUF[i]) >> 16 ); break;
                    case 1  : fprintf(fp_log ,"[Tx%02d]------%02x %02x------\n", i, NCSI_TxDWBUF[i], DWSwap_SLT( NCSI_TxDWBUF[i]) >> 24 ); break;
                    default : fprintf(fp_log ,"[Tx%02d]error", i); break;
                }
                #endif
            } 
            else {
                #ifdef SLT_DOS
                fprintf( fp_log , "[Tx%02d]%08x %08x\n", i, NCSI_TxDWBUF[i], DWSwap_SLT(NCSI_TxDWBUF[i]) );
                #endif
            }
        }
    } // End for (i = 0; i < dwsize; i++)
    
    // Setting one TX descriptor
    WriteSOC_DD( H_TDES_BASE + 0x04, 0                     );
    WriteSOC_DD( H_TDES_BASE + 0x08, 0                     );
    WriteSOC_DD( H_TDES_BASE + 0x0C, (DMA_BASE + CPU_BUS_ADDR_SDRAM_OFFSET)  ); // 20130730
    WriteSOC_DD( H_TDES_BASE       , 0xf0008000 + bytesize );
    // Fire             
    WriteSOC_DD( H_MAC_BASE  + 0x18, 0x00000000            );//Tx Poll

    do {
        NCSI_TxDesDat = ReadSOC_DD(H_TDES_BASE);
        if ( ++timeout > TIME_OUT_NCSI ) {
            #ifdef SLT_DOS
            fprintf(fp_log, "[NCSI-TxDesOwn] %08lx\n", NCSI_TxDesDat);
            #endif
            
            return(FindErr(Err_NCSI_Check_TxOwnTimeOut));
        }
    } while ( HWOwnTx(NCSI_TxDesDat) );
    
    return(0);
} // End char NCSI_Tx (void)

//------------------------------------------------------------
char NCSI_ARP (void) {
    int    i;
    int    timeout = 0;
    ULONG  NCSI_TxDesDat;
    
    if ( ARPNumCnt ) {
        #ifdef SLT_DOS
        if ( PrintNCSIEn ) 
            fprintf(fp_log ,"[ARP] 60 bytes x%d\n", ARPNumCnt);
        #endif
        
        for (i = 0; i < 15; i++) {
            #ifdef SLT_DOS
            if ( PrintNCSIEn ) 
                fprintf(fp_log ,"[Tx%02d] %08x %08x\n", i, ARP_data[i], DWSwap_SLT(ARP_data[i]));
            #endif
            WriteSOC_DD( DMA_BASE + ( i << 2 ), ARP_data[i] );
        }
        WriteSOC_DD( H_TDES_BASE + 0x04, 0               );
        WriteSOC_DD( H_TDES_BASE + 0x08, 0               );
        WriteSOC_DD( H_TDES_BASE + 0x0C, (DMA_BASE + CPU_BUS_ADDR_SDRAM_OFFSET)  ); // 20130730
        WriteSOC_DD( H_TDES_BASE       , 0xf0008000 + 60 );

        for (i = 0; i < ARPNumCnt; i++) {
            WriteSOC_DD( H_TDES_BASE      , 0xf0008000 + 60);

            WriteSOC_DD( H_MAC_BASE + 0x18, 0x00000000      );//Tx Poll

            timeout = 0;
            do {
                NCSI_TxDesDat = ReadSOC_DD(H_TDES_BASE);
                
                if (++timeout > TIME_OUT_NCSI) {
                    #ifdef SLT_DOS
                    fprintf(fp_log, "[ARP-TxDesOwn] %08lx\n", NCSI_TxDesDat);
                    #endif
                    
                    return(FindErr(Err_NCSI_Check_ARPOwnTimeOut));
                }
            } while (HWOwnTx(NCSI_TxDesDat));
        }
    }
    return(0);
} // End char NCSI_ARP (void)

//------------------------------------------------------------
void WrRequest (unsigned char command, unsigned char id, unsigned short length) {

    NCSI_Request_SLT.IID            = InstanceID;
    NCSI_Request_SLT.Command        = command;
    NCSI_Request_SLT.Channel_ID     = id;
    NCSI_Request_SLT.Payload_Length = WDSwap_SLT(length);

    memcpy ( NCSI_TxByteBUF               , &NCSI_Request_SLT    , 30    );
    memcpy ((NCSI_TxByteBUF + 30         ), &NCSI_Payload_Data    , length);
    Calculate_Checksum_NCSI(NCSI_TxByteBUF, 30 + length);
    memcpy ((NCSI_TxByteBUF + 30 + length), &Payload_Checksum_NCSI, 4     );
}

//------------------------------------------------------------
void NCSI_PrintCommandStr (unsigned char command, unsigned iid) {
    switch (command & 0x80) {
        case 0x80   : sprintf(NCSI_CommandStr, "IID:%3d [%02x][Respond]", iid, command); break;
        default     : sprintf(NCSI_CommandStr, "IID:%3d [%02x][Request]", iid, command); break;
    }
    switch (command & 0x7f) {
        case 0x00   : sprintf(NCSI_CommandStr, "%s[CLEAR_INITIAL_STATE                ]", NCSI_CommandStr); break;
        case 0x01   : sprintf(NCSI_CommandStr, "%s[SELECT_PACKAGE                     ]", NCSI_CommandStr); break;
        case 0x02   : sprintf(NCSI_CommandStr, "%s[DESELECT_PACKAGE                   ]", NCSI_CommandStr); break;
        case 0x03   : sprintf(NCSI_CommandStr, "%s[ENABLE_CHANNEL                     ]", NCSI_CommandStr); break;
        case 0x04   : sprintf(NCSI_CommandStr, "%s[DISABLE_CHANNEL                    ]", NCSI_CommandStr); break;
        case 0x05   : sprintf(NCSI_CommandStr, "%s[RESET_CHANNEL                      ]", NCSI_CommandStr); break;
        case 0x06   : sprintf(NCSI_CommandStr, "%s[ENABLE_CHANNEL_NETWORK_TX          ]", NCSI_CommandStr); break;
        case 0x07   : sprintf(NCSI_CommandStr, "%s[DISABLE_CHANNEL_NETWORK_TX         ]", NCSI_CommandStr); break;
        case 0x08   : sprintf(NCSI_CommandStr, "%s[AEN_ENABLE                         ]", NCSI_CommandStr); break;
        case 0x09   : sprintf(NCSI_CommandStr, "%s[SET_LINK                           ]", NCSI_CommandStr); break;
        case 0x0A   : sprintf(NCSI_CommandStr, "%s[GET_LINK_STATUS                    ]", NCSI_CommandStr); break;
        case 0x0B   : sprintf(NCSI_CommandStr, "%s[SET_VLAN_FILTER                    ]", NCSI_CommandStr); break;
        case 0x0C   : sprintf(NCSI_CommandStr, "%s[ENABLE_VLAN                        ]", NCSI_CommandStr); break;
        case 0x0D   : sprintf(NCSI_CommandStr, "%s[DISABLE_VLAN                       ]", NCSI_CommandStr); break;
        case 0x0E   : sprintf(NCSI_CommandStr, "%s[SET_MAC_ADDRESS                    ]", NCSI_CommandStr); break;
        case 0x10   : sprintf(NCSI_CommandStr, "%s[ENABLE_BROADCAST_FILTERING         ]", NCSI_CommandStr); break;
        case 0x11   : sprintf(NCSI_CommandStr, "%s[DISABLE_BROADCAST_FILTERING        ]", NCSI_CommandStr); break;
        case 0x12   : sprintf(NCSI_CommandStr, "%s[ENABLE_GLOBAL_MULTICAST_FILTERING  ]", NCSI_CommandStr); break;
        case 0x13   : sprintf(NCSI_CommandStr, "%s[DISABLE_GLOBAL_MULTICAST_FILTERING ]", NCSI_CommandStr); break;
        case 0x14   : sprintf(NCSI_CommandStr, "%s[SET_NCSI_FLOW_CONTROL              ]", NCSI_CommandStr); break;
        case 0x15   : sprintf(NCSI_CommandStr, "%s[GET_VERSION_ID                     ]", NCSI_CommandStr); break;
        case 0x16   : sprintf(NCSI_CommandStr, "%s[GET_CAPABILITIES                   ]", NCSI_CommandStr); break;
        case 0x17   : sprintf(NCSI_CommandStr, "%s[GET_PARAMETERS                     ]", NCSI_CommandStr); break;
        case 0x18   : sprintf(NCSI_CommandStr, "%s[GET_CONTROLLER_PACKET_STATISTICS   ]", NCSI_CommandStr); break;
        case 0x19   : sprintf(NCSI_CommandStr, "%s[GET_NCSI_STATISTICS                ]", NCSI_CommandStr); break;
        case 0x1A   : sprintf(NCSI_CommandStr, "%s[GET_NCSI_PASS_THROUGH_STATISTICS   ]", NCSI_CommandStr); break;
        case 0x50   : sprintf(NCSI_CommandStr, "%s[OEM_COMMAND                        ]", NCSI_CommandStr); break;
        default     : sprintf(NCSI_CommandStr, "%s Not Support Command", NCSI_CommandStr); break ;
    }
} // End void NCSI_PrintCommandStr (unsigned char command, unsigned iid)

//------------------------------------------------------------
void NCSI_PrintCommandType (unsigned char command, unsigned iid) {
    NCSI_PrintCommandStr(command, iid);
    printf ("%s\n", NCSI_CommandStr);
}

//------------------------------------------------------------
void NCSI_PrintCommandType2File (unsigned char command, unsigned iid) {
    NCSI_PrintCommandStr(command, iid);
    #ifdef SLT_DOS
    fprintf(fp_log, "%s\n", NCSI_CommandStr);
    #endif
}

//------------------------------------------------------------
char NCSI_SentWaitPacket (unsigned char command, unsigned char id, unsigned short length) {
    int     Retry = 0;
    char    ret;
    
    do {
        InstanceID++;
        WrRequest(command, id, length);

        ret = NCSI_Tx();
        if ( ret != 0 ) 
        {
            // printf("======> NCSI_Tx return code = %X\n", ret );                  
            return(1);
        }
            
#ifdef Print_PackageName
        NCSI_PrintCommandType(command, InstanceID);
#endif

#ifdef NCSI_EnableDelay_EachPackage
        delay(Delay_EachPackage);
#endif
        if ( NCSI_Rx_SLT( command ) ) 
            return(2);
        
        #ifdef SLT_DOS
        if ( PrintNCSIEn ) 
            fprintf(fp_log, "[Request] ETyp:%04x MC_ID:%02x HeadVer:%02x IID:%02x Comm:%02x ChlID:%02x PayLen:%04x\n", WDSwap_SLT(NCSI_Request_SLT.EtherType), 
                                                                                                                       NCSI_Request_SLT.MC_ID, 
                                                                                                                       NCSI_Request_SLT.Header_Revision, 
                                                                                                                       NCSI_Request_SLT.IID, 
                                                                                                                       NCSI_Request_SLT.Command, 
                                                                                                                       NCSI_Request_SLT.Channel_ID, 
                                                                                                                       WDSwap_SLT(NCSI_Request_SLT.Payload_Length) );
        if ( PrintNCSIEn ) 
            fprintf(fp_log, "[Respond] ETyp:%04x MC_ID:%02x HeadVer:%02x IID:%02x Comm:%02x ChlID:%02x PayLen:%04x ResCd:%02x ReaCd:%02x\n",        
                                                                                                                       NCSI_Respond_SLT.EtherType, 
                                                                                                                       NCSI_Respond_SLT.MC_ID, 
                                                                                                                       NCSI_Respond_SLT.Header_Revision, 
                                                                                                                       NCSI_Respond_SLT.IID, 
                                                                                                                       NCSI_Respond_SLT.Command, 
                                                                                                                       NCSI_Respond_SLT.Channel_ID,        
                                                                                                                       NCSI_Respond_SLT.Payload_Length, 
                                                                                                                       NCSI_Respond_SLT.Response_Code, 
                                                                                                                       NCSI_Respond_SLT.Reason_Code);
        #endif
        
        if ( (NCSI_Respond_SLT.IID           != InstanceID)       || 
             (NCSI_Respond_SLT.Command       != (command | 0x80)) || 
             (NCSI_Respond_SLT.Response_Code != COMMAND_COMPLETED)  ) {
            #ifdef SLT_DOS
            if ( PrintNCSIEn ) 
                fprintf(fp_log, "Retry: Command = %x, Response_Code = %x\n", NCSI_Request_SLT.Command, NCSI_Respond_SLT.Response_Code);

            #endif
            Retry++;
        } 
        else {
            if ( PrintNCSIEn ) 
                NCSI_PrintCommandType2File(command, InstanceID);

            return(0);
        }
    } while (Retry <= SENT_RETRY_COUNT);

    return( 3 );
} // End char NCSI_SentWaitPacket (unsigned char command, unsigned char id, unsigned short length)

//------------------------------------------------------------
char Clear_Initial_State_SLT (int Channel_ID) {//Command:0x00
    return(NCSI_SentWaitPacket(CLEAR_INITIAL_STATE, (NCSI_Cap_SLT.Package_ID << 5) + Channel_ID, 0));//Internal Channel ID = 0
}

//------------------------------------------------------------
char Select_Package_SLT (int Package_ID) {//Command:0x01
    memset ((void *)NCSI_Payload_Data, 0, 4);
    NCSI_Payload_Data[3] = 1; //Arbitration Disable

    return(NCSI_SentWaitPacket(SELECT_PACKAGE, (Package_ID << 5) + 0x1F, 4));//Internal Channel ID = 0x1F, 0x1F means all channel
}

//------------------------------------------------------------
void Select_Active_Package_SLT (void) {//Command:0x01
    memset ((void *)NCSI_Payload_Data, 0, 4);
    NCSI_Payload_Data[3] = 1; //Arbitration Disable

    if (NCSI_SentWaitPacket(SELECT_PACKAGE, (NCSI_Cap_SLT.Package_ID << 5) + 0x1F, 4)) {//Internal Channel ID = 0x1F
        FindErr_NCSI(NCSI_LinkFail_Select_Active_Package);
    }
}

//------------------------------------------------------------
void DeSelect_Package_SLT (int Package_ID) {//Command:0x02
    NCSI_SentWaitPacket(DESELECT_PACKAGE, (Package_ID << 5) + 0x1F, 0);//Internal Channel ID = 0x1F, 0x1F means all channel

#ifdef NCSI_EnableDelay_DeSelectPackage
    delay(Delay_DeSelectPackage);
#endif
}

//------------------------------------------------------------
void Enable_Channel_SLT (void) {//Command:0x03
    if ( NCSI_SentWaitPacket(ENABLE_CHANNEL, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0) ) {
        FindErr_NCSI(NCSI_LinkFail_Enable_Channel);
    }
}

//------------------------------------------------------------
void Disable_Channel_SLT (void) {//Command:0x04
    memset ((void *)NCSI_Payload_Data, 0, 4);
    NCSI_Payload_Data[3] = 0x1; //ALD

    if (NCSI_SentWaitPacket(DISABLE_CHANNEL, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 4)) {
        FindErr_NCSI(NCSI_LinkFail_Disable_Channel);
    }
}
void Enable_Network_TX_SLT (void) {//Command:0x06
    if ( NCSI_SentWaitPacket(ENABLE_CHANNEL_NETWORK_TX, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0) ) {
        FindErr_NCSI(NCSI_LinkFail_Enable_Network_TX);
    }
}

//------------------------------------------------------------
void Disable_Network_TX_SLT (void) {//Command:0x07
    if ( NCSI_SentWaitPacket(DISABLE_CHANNEL_NETWORK_TX, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0) ) {
        FindErr_NCSI(NCSI_LinkFail_Disable_Network_TX);
    }
}

//------------------------------------------------------------
void Set_Link_SLT (void) {//Command:0x09
    memset ((void *)NCSI_Payload_Data, 0, 8);
    NCSI_Payload_Data[2] = 0x02; //full duplex
//  NCSI_Payload_Data[3] = 0x04; //100M, auto-disable
    NCSI_Payload_Data[3] = 0x05; //100M, auto-enable

    NCSI_SentWaitPacket(SET_LINK, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 8);
}

//------------------------------------------------------------
char Get_Link_Status_SLT (void) {//Command:0x0a
    
    if (NCSI_SentWaitPacket(GET_LINK_STATUS, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0)) {
        return(0);
    } 
    else {
        if (NCSI_Respond_SLT.Payload_Data[3] & 0x20) {
            if (NCSI_Respond_SLT.Payload_Data[3] & 0x40) {
                if (NCSI_Respond_SLT.Payload_Data[3] & 0x01) 
                    return(1); //Link Up or Not
                else                                     
                    return(0);
            } else
                return(0); //Auto Negotiate did not finish
        } else {
            if (NCSI_Respond_SLT.Payload_Data[3] & 0x01) 
                return(1); //Link Up or Not
            else                                     
                return(0);
        }
    }
} // End char Get_Link_Status_SLT (void)

//------------------------------------------------------------
void Enable_Set_MAC_Address_SLT (void) {//Command:0x0e
    int i;
    
    for ( i = 0; i < 6; i++ ) {
        NCSI_Payload_Data[i] = NCSI_Request_SLT.SA[i];
    }
    NCSI_Payload_Data[6] = 1; //MAC Address Num = 1 --> address filter 1, fixed in sample code
    NCSI_Payload_Data[7] = UNICAST + 0 + ENABLE_MAC_ADDRESS_FILTER; //AT + Reserved + E

    if ( NCSI_SentWaitPacket(SET_MAC_ADDRESS, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 8) ) {
        FindErr_NCSI(NCSI_LinkFail_Enable_Set_MAC_Address);
    }
}

//------------------------------------------------------------
void Enable_Broadcast_Filter_SLT (void) {//Command:0x10
    memset ((void *)NCSI_Payload_Data, 0, 4);
    NCSI_Payload_Data[3] = 0xF; //ARP, DHCP, NetBIOS

    if (NCSI_SentWaitPacket(ENABLE_BROADCAST_FILTERING, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 4) ) {
        FindErr_NCSI(NCSI_LinkFail_Enable_Broadcast_Filter);
    }
}

//------------------------------------------------------------
void Get_Version_ID_SLT (void) {//Command:0x15
    
    if (NCSI_SentWaitPacket(GET_VERSION_ID, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0) ) {
        FindErr_NCSI(NCSI_LinkFail_Get_Version_ID);
    } 
    else {
#ifdef Print_Version_ID
        printf ("NCSI Version        : %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[ 0], NCSI_Respond_SLT.Payload_Data[ 1], NCSI_Respond_SLT.Payload_Data[ 2], NCSI_Respond_SLT.Payload_Data[ 3]);
        printf ("NCSI Version        : %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[ 4], NCSI_Respond_SLT.Payload_Data[ 5], NCSI_Respond_SLT.Payload_Data[ 6], NCSI_Respond_SLT.Payload_Data[ 7]);
        printf ("Firmware Name String: %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[ 8], NCSI_Respond_SLT.Payload_Data[ 9], NCSI_Respond_SLT.Payload_Data[10], NCSI_Respond_SLT.Payload_Data[11]);
        printf ("Firmware Name String: %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[12], NCSI_Respond_SLT.Payload_Data[13], NCSI_Respond_SLT.Payload_Data[14], NCSI_Respond_SLT.Payload_Data[15]);
        printf ("Firmware Name String: %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[16], NCSI_Respond_SLT.Payload_Data[17], NCSI_Respond_SLT.Payload_Data[18], NCSI_Respond_SLT.Payload_Data[19]);
        printf ("Firmware Version    : %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[20], NCSI_Respond_SLT.Payload_Data[21], NCSI_Respond_SLT.Payload_Data[22], NCSI_Respond_SLT.Payload_Data[23]);
        printf ("PCI DID/VID         : %02x %02x/%02x %02x\n", NCSI_Respond_SLT.Payload_Data[24], NCSI_Respond_SLT.Payload_Data[25], NCSI_Respond_SLT.Payload_Data[26], NCSI_Respond_SLT.Payload_Data[27]);
        printf ("PCI SSID/SVID       : %02x %02x/%02x %02x\n", NCSI_Respond_SLT.Payload_Data[28], NCSI_Respond_SLT.Payload_Data[29], NCSI_Respond_SLT.Payload_Data[30], NCSI_Respond_SLT.Payload_Data[31]);
        printf ("Manufacturer ID     : %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[32], NCSI_Respond_SLT.Payload_Data[33], NCSI_Respond_SLT.Payload_Data[34], NCSI_Respond_SLT.Payload_Data[35]);
        printf ("Checksum            : %02x %02x %02x %02x\n", NCSI_Respond_SLT.Payload_Data[36], NCSI_Respond_SLT.Payload_Data[37], NCSI_Respond_SLT.Payload_Data[38], NCSI_Respond_SLT.Payload_Data[39]);
#endif
        NCSI_Cap_SLT.PCI_DID_VID    = (NCSI_Respond_SLT.Payload_Data[24]<<24)
                                | (NCSI_Respond_SLT.Payload_Data[25]<<16)
                                | (NCSI_Respond_SLT.Payload_Data[26]<< 8)
                                | (NCSI_Respond_SLT.Payload_Data[27]    );
        NCSI_Cap_SLT.ManufacturerID = (NCSI_Respond_SLT.Payload_Data[32]<<24)
                                | (NCSI_Respond_SLT.Payload_Data[33]<<16)
                                | (NCSI_Respond_SLT.Payload_Data[34]<< 8)
                                | (NCSI_Respond_SLT.Payload_Data[35]    );
    }
} // End void Get_Version_ID_SLT (void)

//------------------------------------------------------------
void Get_Capabilities_SLT (void) {//Command:0x16
    
    if (NCSI_SentWaitPacket(GET_CAPABILITIES, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0)) {
        FindErr_NCSI(NCSI_LinkFail_Get_Capabilities);
    } 
    else {
        NCSI_Cap_SLT.Capabilities_Flags                   = NCSI_Respond_SLT.Payload_Data[0];
        NCSI_Cap_SLT.Broadcast_Packet_Filter_Capabilities = NCSI_Respond_SLT.Payload_Data[1];
        NCSI_Cap_SLT.Multicast_Packet_Filter_Capabilities = NCSI_Respond_SLT.Payload_Data[2];
        NCSI_Cap_SLT.Buffering_Capabilities               = NCSI_Respond_SLT.Payload_Data[3];
        NCSI_Cap_SLT.AEN_Control_Support                  = NCSI_Respond_SLT.Payload_Data[4];
    }
}

//------------------------------------------------------------
void Get_Controller_Packet_Statistics (void) {//Command:0x18

    NCSI_SentWaitPacket(GET_CONTROLLER_PACKET_STATISTICS, (NCSI_Cap_SLT.Package_ID << 5) + NCSI_Cap_SLT.Channel_ID, 0);
}

//------------------------------------------------------------
char phy_ncsi (void) {
    ULONG   Channel_Found = 0;
    ULONG   Package_Found = 0;
    ULONG   Re_Send;
    ULONG   Err_Flag_bak;
    ULONG   pkg_idx;
    ULONG   chl_idx;
    ULONG   Link_Status;
    ULONG   NCSI_LinkFail_Val_bak;

    number_chl    = 0;
    number_pak    = 0;
    
    NCSI_LinkFail_Val = 0;
    #ifdef SLT_DOS
		fprintf(fp_log, "\n\n======> Start:\n" );
    #endif
    NCSI_Struct_Initialize_SLT();

    #ifdef NCSI_Skip_Phase1_DeSelectPackage
    #else
    
    //NCSI Start
    //Disable Channel then DeSelect Package
    for (pkg_idx = 0; pkg_idx < MAX_PACKAGE_NUM; pkg_idx++) {
        // Ignore error flag in the NCSI command
        Err_Flag_bak          = Err_Flag;
        NCSI_LinkFail_Val_bak = NCSI_LinkFail_Val;
        select_flag[pkg_idx]  = Select_Package_SLT (pkg_idx); // Command:0x01
        Err_Flag              = Err_Flag_bak;
        NCSI_LinkFail_Val     = NCSI_LinkFail_Val_bak;
        
        if ( select_flag[pkg_idx] == 0 ) {
            NCSI_Cap_SLT.Package_ID = pkg_idx;

            for ( chl_idx = 0; chl_idx < MAX_CHANNEL_NUM; chl_idx++ ) {
                NCSI_Cap_SLT.Channel_ID = chl_idx;
                // Ignore error flag in the NCSI command
                Err_Flag_bak          = Err_Flag;
                NCSI_LinkFail_Val_bak = NCSI_LinkFail_Val;
                Disable_Channel_SLT();    // Command: 0x04
                Err_Flag              = Err_Flag_bak;
                NCSI_LinkFail_Val     = NCSI_LinkFail_Val_bak;
            }
            #ifdef NCSI_Skip_DeSelectPackage
            #else
            DeSelect_Package_SLT (pkg_idx); // Command:0x02
            #endif
        } // End if ( select_flag[pkg_idx] == 0 )
    } // End for (pkg_idx = 0; pkg_idx < MAX_PACKAGE_NUM; pkg_idx++)
    #endif

    //Select Package
    for (pkg_idx = 0; pkg_idx < MAX_PACKAGE_NUM; pkg_idx++) {
        #ifdef NCSI_Skip_Phase1_DeSelectPackage
        // Ignore error flag in the NCSI command
        Err_Flag_bak          = Err_Flag;
        NCSI_LinkFail_Val_bak = NCSI_LinkFail_Val;
        select_flag[pkg_idx]  = Select_Package_SLT (pkg_idx);//Command:0x01
        Err_Flag              = Err_Flag_bak;
        NCSI_LinkFail_Val     = NCSI_LinkFail_Val_bak;
        #endif

        if (select_flag[pkg_idx] == 0) {
            //NCSI_RxTimeOutScale = 1000;
            NCSI_RxTimeOutScale = 10;
            number_pak++;
            Package_Found       = 1;
            NCSI_Cap_SLT.Package_ID = pkg_idx;

            if ( !(IOTiming||IOTimingBund) ) 
                printf ("====Find Package ID: %d\n", NCSI_Cap_SLT.Package_ID);
            #ifdef SLT_DOS
            fprintf(fp_log, "====Find Package ID: %d\n", NCSI_Cap_SLT.Package_ID);
            #endif
            
            #ifdef NCSI_Skip_Phase1_DeSelectPackage
            #else
            Select_Package_SLT (pkg_idx);//Command:0x01
            #endif
            
            // Scan all channel in the package
            for ( chl_idx = 0; chl_idx < MAX_CHANNEL_NUM; chl_idx++ ) {
                // backup error flag
                Err_Flag_bak          = Err_Flag;
                NCSI_LinkFail_Val_bak = NCSI_LinkFail_Val;
                if (Clear_Initial_State_SLT(chl_idx) == 0) { //Command:0x00
                    number_chl++;
                    Channel_Found       = 1;
                    NCSI_Cap_SLT.Channel_ID = chl_idx;

                    if ( !(IOTiming || IOTimingBund) ) 
                        printf ("--------Find Channel ID: %d\n", NCSI_Cap_SLT.Channel_ID);
                        
                    #ifdef SLT_DOS
                    fprintf(fp_log, "--------Find Channel ID: %d\n", NCSI_Cap_SLT.Channel_ID);
                    #endif
                    // Get Version and Capabilities
                    Get_Version_ID_SLT();          //Command:0x15
                    Get_Capabilities_SLT();        //Command:0x16
                    Select_Active_Package_SLT();   //Command:0x01
                    Enable_Set_MAC_Address_SLT();  //Command:0x0e
                    Enable_Broadcast_Filter_SLT(); //Command:0x10

                    // Enable TX
                    Enable_Network_TX_SLT();       //Command:0x06

                    // Enable Channel
                    Enable_Channel_SLT();          //Command:0x03
                    
                    // Get Link Status
                    Re_Send = 0;
                    do {
                        #ifdef NCSI_EnableDelay_GetLinkStatus
                        if ( Re_Send >= 2 ) 
                            delay(Delay_GetLinkStatus);
                        #endif
                        
                        Link_Status = Get_Link_Status_SLT();//Command:0x0a
                        
                        if ( Link_Status == LINK_UP ) {
                            if (!(IOTiming||IOTimingBund)) 
                                printf ("        This Channel is LINK_UP\n");
                            
                            #ifdef SLT_DOS
                            fprintf(fp_log, "        This Channel is LINK_UP\n");
                            #endif
                            
                            NCSI_ARP ();
                            
                            break;
                        } 
                        else if ( Link_Status == LINK_DOWN ) {
                            if ( Re_Send >= 2 ) {   
                                if ( !(IOTiming || IOTimingBund) ) 
                                    printf ("        This Channel is LINK_DOWN\n");
	                    	    
                                #ifdef SLT_DOS
                                fprintf(fp_log, "        This Channel is LINK_DOWN\n");
                                #endif
                                
	                    	    break;
	                    	}
                        } // End if ( Link_Status == LINK_UP )
                    } while ( Re_Send++ <= 2 );

                    #ifdef NCSI_Skip_DiSChannel
                    #else
                    if ( NCSI_DiSChannel ) {
                        // Disable TX
                        Disable_Network_TX_SLT(); //Command:0x07
                        // Disable Channel
                        Disable_Channel_SLT();    //Command:0x04
                    }
                    #endif
                } 
                else {
                    Err_Flag          = Err_Flag_bak;
                    NCSI_LinkFail_Val = NCSI_LinkFail_Val_bak;
                }
            } // End for ( chl_idx = 0; chl_idx < MAX_CHANNEL_NUM; chl_idx++ )
            
            #ifdef NCSI_Skip_DeSelectPackage
            #else
            DeSelect_Package_SLT (pkg_idx);//Command:0x02
            #endif
            NCSI_RxTimeOutScale = 1;
        } 
        else {
            if (!(IOTiming||IOTimingBund)) {
                printf ("====Absence of Package ID: %ld\n", pkg_idx);
                #ifdef SLT_DOS
                fprintf(fp_log, "====Absence of Package ID: %ld\n", pkg_idx);
                #endif
            }
        } // End if (select_flag[pkg_idx] == 0)
    } // End for (pkg_idx = 0; pkg_idx < MAX_PACKAGE_NUM; pkg_idx++)

    if ( !Package_Found              ) FindErr( Err_NCSI_No_PHY      );
    if ( ChannelTolNum != number_chl ) FindErr( Err_NCSI_Channel_Num );
    if ( PackageTolNum != number_pak ) FindErr( Err_NCSI_Package_Num );
//  if ( !Channel_Found) FindErr();

    if ( Err_Flag ) 
        return(1);
    else          
        return(0);
}

