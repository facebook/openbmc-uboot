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

#define PHYMISC
static const char ThisFile[] = "PHYSPECIAL.c";

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

#include "COMMINF.H"
/*
#include "PHY.H"
#include "PHYSPECIAL.H"
#include "TYPEDEF.H"
#include "IO.H"
*/

#ifdef PHY_SPECIAL

/*
ULONG   BCM5389_sample_tx[PHY_BCM5389_Pkt_DW_Len] = { //120 Byte, 30 DW
	0xffffffff,
	0x0000ffff, // SA:00-00-
	0x12345678, // SA:78-56-34-12
	0x00407488, // BRCM Type(0x8874), BRCM Tag:40-00-  //Egress Directed
	0x00000800, // BRCM Tag:00-08                      //DSTPortID [3:0] = 0x8
//	0x00607488, // BRCM Type(0x8874), BRCM Tag:60-00-  //Multiple Egress Directed
//	0x0000ff01, // BRCM Tag:01-ff
	0x00000000, // SimpleData_FixVal00
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, //
	0x00000000, // SimpleData_FixVal11
	0x937e9ade  // Original FCS
//	0x84100dad  // MAC FCS(Tx)(BRCM Tag:40-00-00-08)
//	0xdfb1c84d  // MAC FCS(Tx)(BRCM Tag:60-00-01-ff)
//	0x0983fe7b  // MAC FCS(Rx)
};
/**/

/*
ULONG   BCM5389_sample_tx[PHY_BCM5389_Pkt_DW_Len] = { //120 Byte, 30 DW
	0xffffffff,
	0x0000ffff, // SA:00-00-
	0x12345678, // SA:78-56-34-12
	0x00407488, // BRCM Type(0x8874), BRCM Tag:40-00-  //Egress Directed
	0x55550800, // BRCM Tag:00-08                      //DSTPortID [3:0] = 0x8
//	0x00607488, // BRCM Type(0x8874), BRCM Tag:60-00-  //Multiple Egress Directed
//	0x5555ff01, // BRCM Tag:01-ff
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x7ba6d41a  // Original FCS
//	0x84100dad  // MAC FCS(Tx)(BRCM Tag:40-00-00-08)
//	0xdfb1c84d  // MAC FCS(Tx)(BRCM Tag:60-00-01-ff)
//	0x0983fe7b  // MAC FCS(Rx)
};
/**/

ULONG   BCM5389_sample_tx[PHY_BCM5389_Pkt_DW_Len] = { //1524 Byte, 381 DW
	0xffffffff,
	0x0000ffff, // SA:00-00-
	0x12345678, // SA:78-56-34-12
	0x00407488, // BRCM Type(0x8874), BRCM Tag:40-00-  //Egress Directed
	0x55550800, // BRCM Tag:00-08                      //DSTPortID [3:0] = 0x8
//	0x00607488, // BRCM Type(0x8874), BRCM Tag:60-00-  //Multiple Egress Directed
//	0x5555ff01, // BRCM Tag:01-ff
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0xaaaaaaaa, //
	0x5a5a5a5a, //
	0xc3c3c3c3, //
	0x96969696, //
	0xf0f0f0f0, //
	0x5555aaaa, //
	0xffff0000, //
	0x5a5aa5a5, //
	0xc3c33c3c, // SimpleData_FixVal11
	0x00000000, // SimpleData_FixVal00
	0xffffffff, //
	0x55555555, //
	0x8dedb8b6  // Original FCS
//	0x5604efcf  // MAC FCS(Tx)(BRCM Tag:40-00-00-08)
//	0x36be85a5  // MAC FCS(Tx)(BRCM Tag:60-00-01-ff)
//	0x9f4a367a  // MAC FCS(Rx)
};
/**/

//------------------------------------------------------------
// BCM5389
//------------------------------------------------------------
void BCM5389_set_page (MAC_ENGINE *eng, ULONG page_num) {
	phy_write( eng, 16, ( ( page_num & 0xff ) << 8 ) | 0x0001 );
}

//------------------------------------------------------------
void BCM5389_reg_write (MAC_ENGINE *eng, ULONG addr, ULONG wrdat_DW_MSB, ULONG wrdat_DW_LSB) {
	int        timeout = 0;

#ifdef PHY_debug
	printf("BCM5389[Wr ]%02d: 0x%04lx 0x%04lx\n", addr, wrdat_DW_MSB, wrdat_DW_LSB);
#endif
	phy_write( eng, 24,   wrdat_DW_LSB         & 0xffff );
	phy_write( eng, 25, ( wrdat_DW_LSB >> 16 ) & 0xffff );
	phy_write( eng, 26,   wrdat_DW_MSB         & 0xffff );
	phy_write( eng, 27, ( wrdat_DW_MSB >> 16 ) & 0xffff );

	phy_write( eng, 17, ( ( addr & 0xff ) << 8 ) | 0x0001 );

	while ( ( phy_read ( eng, 17 ) & 0x0003 ) != 0x0 ) {
		if (++timeout > PHY_BCM5389_TIME_OUT_RW) {
			FindErr( eng, Err_Flag_PHY_TimeOut_Rst );
			break;
		}
	}
}

//------------------------------------------------------------
void BCM5389_reg_read_2ptr (MAC_ENGINE *eng, ULONG addr, ULONG *data_MSB, ULONG *data_LSB) {
	int        timeout = 0;

#ifdef PHY_debug
	printf("BCM5389[RdS]%02d\n", addr);
#endif
	phy_write( eng, 17, ( ( addr & 0xff ) << 8 ) | 0x0002 );

	while ( ( phy_read ( eng, 17 ) & 0x0003 ) != 0x0 ) {
		if (++timeout > PHY_BCM5389_TIME_OUT_RW) {
			FindErr( eng, Err_Flag_PHY_TimeOut_Rst );
			break;
		}
	}

	*data_LSB = phy_read ( eng, 24 ) | ( phy_read ( eng, 25 ) << 16 );
	*data_MSB = phy_read ( eng, 26 ) | ( phy_read ( eng, 27 ) << 16 );
#ifdef PHY_debug
	printf("BCM5389[RdE]%02d: 0x%04lx 0x%04lx\n", eng->BCM5389.rddata[1], eng->BCM5389.rddata[0]);
#endif
}

//------------------------------------------------------------
void BCM5389_reg_read  (MAC_ENGINE *eng, ULONG addr) {
	BCM5389_reg_read_2ptr( eng, addr, (eng->BCM5389.rddata + 1), eng->BCM5389.rddata );
}

//------------------------------------------------------------
void BCM5389_reg_chgcheck (MAC_ENGINE *eng, ULONG page, ULONG addr) {
	ULONG	olddat[2];

	olddat[0] = eng->BCM5389.rddata[0];
	olddat[1] = eng->BCM5389.rddata[1];
	BCM5389_reg_read ( eng, addr );
	PRINTF( FP_LOG, "[page %ld:0x%02lx] %08lx %08lx --> %08lx %08lx\n", page, addr, olddat[1], olddat[0], eng->BCM5389.rddata[1], eng->BCM5389.rddata[0]);
}

//------------------------------------------------------------
void BCM5389_counter (MAC_ENGINE *eng) {
	int	port, page;
	ULONG	dat[10];

	PRINTF( FP_LOG, "     TxOctets           TxDropPkts | RxOctets            RxDropPkts RxGoodOctets\n");
	page = 0x20;
	for ( port = 0; port < 8; port++ ) {
		BCM5389_set_page ( eng, page );

		BCM5389_reg_read_2ptr ( eng, 0x00, &dat[1], &dat[0]); //[TxOctets    ] 64 bits
		BCM5389_reg_read_2ptr ( eng, 0x08, &dat[3], &dat[2]); //[TxDropPkts  ] 32 bits
		BCM5389_reg_read_2ptr ( eng, 0x44, &dat[5], &dat[4]); //[RxOctets    ] 64 bits
		BCM5389_reg_read_2ptr ( eng, 0x84, &dat[7], &dat[6]); //[RxDropPkts  ] 32 bits
		BCM5389_reg_read_2ptr ( eng, 0x7c, &dat[9], &dat[8]); //[RxGoodOctets] 64 bits
		PRINTF( FP_LOG, "[P%d](%08lx %08lx) (%08lx) | (%08lx %08lx) (%08lx) (%08lx %08lx)\n", page - 0x20, dat[1], dat[0], dat[2], dat[5], dat[4], dat[6], dat[9], dat[8] );
		page++;
	}
	phy_write( eng, 16, 0x0000 );
}

//------------------------------------------------------------
void BCM5389_init (MAC_ENGINE *eng) {
	eng->BCM5389.wait_init  = 1;
}

//------------------------------------------------------------
void BCM5389_buf_init (MAC_ENGINE *eng) {
	eng->BCM5389.pkt_tx_len = PHY_BCM5389_Pkt_Byte_Len;
	eng->BCM5389.pkt_tx     = (ULONG *)BCM5389_sample_tx;

	if ( eng->arg.GUserDVal == 1 ) {
		eng->BCM5389.pkt_tx[3] = 0x00607488;                                     // BRCM Type(0x8874), BRCM Tag:60-00-
		eng->BCM5389.pkt_tx[4] = 0xff01 | (eng->BCM5389.pkt_tx[4] & 0xffff0000); // BRCM Tag:01-ff
	} else {
		eng->BCM5389.pkt_tx[3] = 0x00407488;                                     // BRCM Type(0x8874), BRCM Tag:40-00-
		eng->BCM5389.pkt_tx[4] = 0x0800 | (eng->BCM5389.pkt_tx[4] & 0xffff0000); // BRCM Tag:00-08
	}


	eng->BCM5389.pkt_rx_len = eng->BCM5389.pkt_tx_len - 6;
	memcpy( (UCHAR *)eng->BCM5389.pkt_rx, (UCHAR *)eng->BCM5389.pkt_tx, eng->BCM5389.pkt_tx_len);

	eng->BCM5389.pkt_rx[3] = 0x00007488
	                       | ( ( ( eng->BCM5389.pkt_rx_len >> 1 ) & 0xff ) << 24 )
	                       | ( ( ( eng->BCM5389.pkt_rx_len >> 9 ) & 0xff ) << 16 ); // BRCM Type(0x8874), BRCM Tag:00-00-
	eng->BCM5389.pkt_rx[4] = 0x0800
	                       |   (   eng->BCM5389.pkt_rx[4] & 0xffff0000   )
	                       |   (   eng->BCM5389.pkt_rx_len        & 0x01 );         // BRCM Tag:00-08

}

//------------------------------------------------------------
void BCM5389_reg_init (MAC_ENGINE *eng) {
	ULONG	dat_08;
	ULONG	dat_0b;

	eng->phy.Adr = 30; //Pseudo-PHY
#ifdef PHY_BCM5389_Debug
	if ( eng->BCM5389.wait_init )
		BCM5389_counter ( eng );
#endif

	//------------------------------
	// Page 0
	//------------------------------
	// [page 0:0x0e] IMP Port State Override Register
	// [page 0:0x60] IMP RGMII Control Register
	// [page 0:0x08] IMP Traffic Control Register
	// [page 0:0x0b] Switch Mode Register
	//------------------------------
	BCM5389_set_page ( eng, 0 );

	//[page 0:0x0e] IMP Port State Override Register
	if ( eng->run.Speed_sel[ 0 ] ) {
		BCM5389_reg_write ( eng, 0x0e, 0x00000000, 0x0000008b ); // 8 bit //  1 G//[0]LINK, [1]FDX, [3:2]SPEED, [7]MII_SW_OR
	}
	else if ( eng->run.Speed_sel[ 1 ] ) {
		BCM5389_reg_write ( eng, 0x0e, 0x00000000, 0x00000087 ); // 8 bit //100 M//[0]LINK, [1]FDX, [3:2]SPEED, [7]MII_SW_OR
	}
	else {
		BCM5389_reg_write ( eng, 0x0e, 0x00000000, 0x00000083 ); // 8 bit // 10 M//[0]LINK, [1]FDX, [3:2]SPEED, [7]MII_SW_OR
	}

	if ( eng->BCM5389.wait_init ) {
		//[page 0:0x60] IMP RGMII Control Register
		BCM5389_reg_read ( eng, 0x60 );
		if ( ( eng->BCM5389.rddata[0] & 0x3 ) != 0 ) {
			printf("\n\n[Warning] Page0, Register 0x60, bit 0~1 must be 0 [Reg60h_0:%08lx]\n\n", eng->BCM5389.rddata[0] );
			if ( eng->run.TM_IOTiming ) PRINTF( FP_IO, "\n\n[Warning] Page0, Register 0x60, bit 0~1 must be 0 [Reg60h_0:%08lx]\n\n", eng->BCM5389.rddata[0] );
			if ( !eng->run.TM_Burst ) PRINTF( FP_LOG, "\n\n[Warning] Page0, Register 0x60, bit 0~1 must be 0 [Reg60h_0:%08lx]\n\n", eng->BCM5389.rddata[0] );
			BCM5389_reg_write ( eng, 0x60, 0x00000000, 0x00000000 ); // 8 bit //[0]TXC_DELAY, [1]RXC_DELAY
		}
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_chgcheck( eng, 0, 0x60 );
#endif

		if ( eng->phy.loop_phy ) {
			// Enable IMP Port to receive the other Port's data
			dat_08 = 0x0000001c;
			dat_0b = 0x00000007;
		} else {
			// Disable IMP Port to receive the other Port's data
			dat_08 = 0x00000000;
			dat_0b = 0x00000006;
		}
		//[page 0:0x08] IMP Traffic Control Register
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_read ( eng, 0x08 );
#endif
		BCM5389_reg_write ( eng, 0x08, 0x00000000, dat_08 ); // 8 bit //[2]RX_BCST_EN, [3]RX_MCST_EN, [4]RX_UCST_EN
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_chgcheck( eng, 0, 0x08 );
#endif

		//[page 0:0x0b] Switch Mode Register
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_read ( eng, 0x0b );
#endif
		BCM5389_reg_write ( eng, 0x0b, 0x00000000, dat_0b ); // 8 bit //[0]SW_FWDG_MODE, [1]SW_FWDG_EN
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_chgcheck( eng, 0, 0x0b );
#endif

		//------------------------------
		// Page 2
		//------------------------------
		// [page 2:0x00] Global Management Configuration Register
		//------------------------------
		BCM5389_set_page ( eng, 2 );

		//[page 2:0x00] Global Management Configuration Register
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_read ( eng, 0x00 );
#endif
		BCM5389_reg_write ( eng, 0x00, 0x00000000, 0x00000080 ); // 8 bit //[7]IMP Port Enable
#ifdef PHY_BCM5389_Debug
		BCM5389_reg_chgcheck( eng, 2, 0x00 );
#endif

		eng->BCM5389.wait_init = 0;
	}

	phy_write( eng, 16, 0x0000 );
}





//------------------------------------------------------------
// Special PHY
//------------------------------------------------------------
void special_PHY_init (MAC_ENGINE *eng) {
	BCM5389_init( eng );
}

//------------------------------------------------------------
void special_PHY_buf_init (MAC_ENGINE *eng) {
	BCM5389_buf_init( eng );
}

//------------------------------------------------------------
void special_PHY_recov (MAC_ENGINE *eng) {
}

//------------------------------------------------------------
void special_PHY_reg_init (MAC_ENGINE *eng) {
	BCM5389_reg_init( eng );
}

//------------------------------------------------------------
void special_PHY_debug (MAC_ENGINE *eng) {
#ifdef PHY_BCM5389_Debug
	BCM5389_counter( eng );
#endif
}

//------------------------------------------------------------
ULONG special_PHY_FRAME_LEN (MAC_ENGINE *eng) {
	return ( eng->BCM5389.pkt_tx_len );
}

//------------------------------------------------------------
ULONG *special_PHY_txpkt_ptr (MAC_ENGINE *eng) {
	return ( (ULONG *)eng->BCM5389.pkt_tx );
}

//------------------------------------------------------------
ULONG *special_PHY_rxpkt_ptr (MAC_ENGINE *eng) {
	return ( (ULONG *)eng->BCM5389.pkt_rx );
}
#endif
