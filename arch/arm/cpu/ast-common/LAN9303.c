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

#define LAN9303_C
static const char ThisFile[] = "LAN9303.c";

#include "SWFUNC.H"
#ifdef SLT_UBOOT
  #include "COMMINF.H"
  #include "MAC.H"
  #include "IO.H"
#endif

#if defined(LinuxAP) || defined(DOS_ALONE) || defined(SLT_NEW_ARCH)
  #include "COMMINF.H"
  #include <stdlib.h>
  #include "IO.H"
#endif

#ifdef SUPPORT_PHY_LAN9303
//#define LAN9303M
#define I2C_Debug           0
#define Print_DWRW          0
#define Print_PHYRW         0
#define I2C_TIMEOUT         10000000


typedef struct {
	ULONG      devbase;
	ULONG      busnum;
	ULONG      byte;
}  LAN9303_Data;

//------------------------------------------------------------
// Basic
//------------------------------------------------------------
void actime(ULONG ac1, ULONG ac2, ULONG *fact, ULONG *ckh, ULONG *ckl)
{
        static int      divcnt;

        ac1 = ac1 * 50 + 1;
        ac2 = ac2 * 50 + 1;

        divcnt = 0;
        while (ac1 > 8 || ac2 > 8) {
                divcnt++;
                ac1 >>= 1;
                ac2 >>= 1;
        }

        if (ac1 < 2  ) ac1  = 2;
        if (ac2 < 2  ) ac2  = 2;
        if (ac1 > ac2) ac2  = 1;
        else           ac1 += 1;

#ifdef PRINT_MSG
        printf("Divcnt = %d, ckdiv = %d, ckh = %d, ckl = %d\n",(1<<divcnt)*(ac1+ac2),divcnt,ac1-1,ac2-1);
        printf("CKH = %d us, CKL = %d us\n",(1<<divcnt)*ac1/50,(1<<divcnt)*ac2/50);
#endif

        *fact = divcnt;
        *ckh  = ac1 - 1;
        *ckl  = ac2 - 1;
}

//------------------------------------------------------------
ULONG PollStatus(LAN9303_Data *eng)
{
        static ULONG    status;
        static ULONG    cnt = 0;

        do {
                status = ReadSOC_DD( eng->devbase + 0x14 ) & 0xff;

                if ( ++cnt > I2C_TIMEOUT ) {
                        printf("\nWait1 Timeout at bus %d!\n", eng->busnum);
                        printf("Status 14 = %08x\n", ReadSOC_DD( eng->devbase + 0x14 ));
                        exit(0);
                }
        } while (status != 0);

        cnt = 0;
        do {
                status = ReadSOC_DD( eng->devbase + 0x10 );
                if ( ++cnt > I2C_TIMEOUT ) {
                        printf("\nWait2 Timeout at bus %d!\n", eng->busnum);
                        printf("Status 14 = %08x\n", ReadSOC_DD( eng->devbase + 0x14 ));
                        exit(0);
                }
        } while (status == 0);

        WriteSOC_DD( eng->devbase + 0x10, status );

        return(status);
}


//------------------------------------------------------------
ULONG writeb(LAN9303_Data *eng, ULONG start, ULONG data, ULONG stop)
{
        WriteSOC_DD( eng->devbase + 0x20, data);
        WriteSOC_DD( eng->devbase + 0x14, 0x02 | start | (stop << 5) );
        return( PollStatus( eng ) );
}

//------------------------------------------------------------
ULONG readb(LAN9303_Data *eng, ULONG last, ULONG stop)
{
        static ULONG    data;

        WriteSOC_DD( eng->devbase + 0x14, 0x08 | (last << 4) | (stop << 5) );
        data = PollStatus( eng );

        if (data & 0x4) {
                data = ReadSOC_DD( eng->devbase + 0x20 );
                return(data >> 8);
        }
        else {
                return(-1);
        }
}

//------------------------------------------------------------
void Initial(LAN9303_Data *eng, ULONG base, ULONG ckh, ULONG ckl)
{
        static ULONG    ackh;
        static ULONG    ackl;
        static ULONG    divx;

        actime(ckh, ckl, &divx, &ackh, &ackl);
        WriteSOC_DD(base + 0x00, 0x1);
        if (ReadSOC_DD(base + 0x00) != 0x1) {
                printf("Controller initial fail : %x\n",base);
                exit(0);
        }
        WriteSOC_DD(base + 0x04, 0x77700360 | (ackh << 16) | (ackl << 12) | divx);
        WriteSOC_DD(base + 0x08, 0x0);
        WriteSOC_DD(base + 0x0c, 0x0);
        WriteSOC_DD(base + 0x10, 0xffffffff);
        WriteSOC_DD(base + 0x14, 0x00);
        WriteSOC_DD(base + 0x1C, 0xff0000);
        WriteSOC_DD(base + 0x20, 0x00);
}

//------------------------------------------------------------
void print_status(ULONG status)
{
        if ( status & 0x02 ) printf("Device NAK\n"      );
        if ( status & 0x08 ) printf("Arbitration Loss\n");
        if ( status & 0x10 ) printf("STOP\n"            );
        if ( status & 0x20 ) printf("Abnormal STOP\n"   );
        if ( status & 0x40 ) printf("SCL Low timeout\n" );
}

//------------------------------------------------------------
void readme()
{
#ifdef LAN9303M
        printf("LAN9303M [bus] [vir_PHY_adr] [speed] [func]\n");
#else
        printf("LAN9303 [bus] [vir_PHY_adr] [speed] [func]\n" );
#endif
        printf("[bus]         | 1~14: I2C bus number\n"       );
        printf("[vir_PHY_adr] | 0~1: virtual PHY address\n"   );
        printf("[speed]       | 1: 100M\n"                    );
        printf("              | 2: 10 M\n"                    );
        printf("[func]        | 0: external loopback\n"       );
        printf("              | 1: internal loopback\n"       );
}

//------------------------------------------------------------
void quit(LAN9303_Data *eng)
{
        WriteSOC_DD( eng->devbase + 0x14, 0x20 );
        PollStatus( eng );
        readme();
}

//------------------------------------------------------------
// Double-Word Read/Write
//------------------------------------------------------------
ULONG I2C_DWRead(LAN9303_Data *eng, ULONG adr)
{
	static ULONG         status;
	int                  i;
        static ULONG         data_rd;
        static ULONG         byte;

        Initial( eng, eng->devbase, 10, 10 );

        if ( Print_DWRW )
            printf("RAdr %02x: ", adr);

        status = writeb( eng, 1, LAN9303_I2C_ADR, 0 );
        if ( I2C_Debug )
            printf("R1W[%02x]%02x ", status, LAN9303_I2C_ADR);

        if ( status != 0x1 ) {
                print_status(status);
                quit( eng );
                exit(0);
        }

        status = writeb( eng, 0, adr, 0 );
        if ( I2C_Debug )
            printf("R2W[%02x]%02x ", status, adr);
        if ( !(status & 0x1) ) {
                print_status(status);
                quit( eng );
                exit(0);
        }

        status = writeb( eng, 1, LAN9303_I2C_ADR | 0x1, 0 );
        if ( I2C_Debug )
            printf("R3W[%02x]%02x ", status, LAN9303_I2C_ADR | 0x1);
        if ( status != 0x1 ) {
                print_status(status);
                quit( eng );
                exit(0);
        }

        if ( I2C_Debug )
            printf("R4");

        data_rd = 0;
        for (i = 24; i >= 0; i-=8) {
                if (i == 0) eng->byte = readb( eng, 1, 1 );
                else        eng->byte = readb( eng, 0, 0 );

                if ( I2C_Debug )
                    printf("%02x ", eng->byte);
                data_rd = data_rd | (eng->byte << i);
        }

        if ( Print_DWRW )
            printf("%08x\n", data_rd);

        return (data_rd);
} // End ULONG I2C_DWRead(LAN9303_Data *eng, ULONG adr)

//------------------------------------------------------------
void I2C_DWWrite(LAN9303_Data *eng, ULONG adr, ULONG dwdata)
{
        static ULONG    status;
        int             i;
        ULONG           endx;

        Initial( eng, eng->devbase, 10, 10 );
        if ( Print_DWRW )
            printf("WAdr %02x: ", adr);

        status = writeb( eng, 1, LAN9303_I2C_ADR, 0 );
        if ( I2C_Debug )
            printf("W1[%02x]%02x ", status, LAN9303_I2C_ADR);
        if ( status != 0x1 ) {
                print_status(status);
                quit( eng );
                exit(0);
        }
        status = writeb( eng, 0, adr, 0 );
        if ( I2C_Debug )
            printf("W2[%02x]%02x ", status, adr);
        if ( !(status & 0x1) ) {
                print_status(status);
                quit( eng );
                exit(0);
        }

        if (I2C_Debug)
            printf("W3");
        endx = 0;
        for (i = 24; i >= 0; i-=8) {
                if (i == 0)
                    endx = 1;
                eng->byte   = (dwdata >> i) & 0xff;
                status = writeb( eng, 0, eng->byte, endx);

            if (I2C_Debug)
                printf("[%02x]%02x ", status, eng->byte);
                if (!(status & 0x1)) {
                        print_status(status);
                        quit( eng );
                        exit(0);
                }
        }

        if ( Print_DWRW ) printf("%08x\n", dwdata);
} // End void I2C_DWWrite(LAN9303_Data *eng, ULONG adr, ULONG dwdata)

//------------------------------------------------------------
// PHY Read/Write
//------------------------------------------------------------
ULONG LAN9303_PHY_Read(LAN9303_Data *eng, ULONG phy_adr, ULONG reg_adr)
{
        static ULONG         data_rd;

        I2C_DWWrite( eng, 0x2a, ((phy_adr & 0x1f) << 11) | ((reg_adr & 0x1f) << 6));//[0A8h]PMI_ACCESS
        do {
            data_rd = I2C_DWRead( eng, 0x2a );
        } while(data_rd & 0x00000001);//[0A8h]PMI_ACCESS

        data_rd = I2C_DWRead( eng, 0x29 );//[0A4h]PMI_DATA
        if ( Print_PHYRW )
            printf("PHY:%2d, Reg:%2d, Data:%08x\n", phy_adr, reg_adr, data_rd);

        return(data_rd);
}

//------------------------------------------------------------
void LAN9303_PHY_Write(LAN9303_Data *eng, ULONG phy_adr, ULONG reg_adr, ULONG data_wr)
{
        static ULONG         data_rd;

        I2C_DWWrite( eng, 0x29, data_wr );//[0A4h]PMI_DATA

        I2C_DWWrite( eng, 0x2a, ((phy_adr & 0x1f) << 11) | ((reg_adr & 0x1f) << 6) | 0x2 );//[0A8h]PMI_ACCESS
        do {
            data_rd = I2C_DWRead( eng, 0x2a );
        } while( data_rd & 0x00000001 );//[0A8h]PMI_ACCESS
}

//------------------------------------------------------------
ULONG LAN9303_PHY_Read_WD(LAN9303_Data *eng, ULONG data_ctl)
{
        static ULONG         data_rd;

        I2C_DWWrite( eng, 0x2a, data_ctl );//[0A8h]PMI_ACCESS
        do {
            data_rd = I2C_DWRead( eng, 0x2a );
        } while(data_rd & 0x00000001);//[0A8h]PMI_ACCESS

        data_rd = I2C_DWRead( eng, 0x29 );//[0A4h]PMI_DATA
        if ( Print_PHYRW )
            printf("WD Data:%08x\n", data_ctl);

        return(data_rd);
}

//------------------------------------------------------------
void LAN9303_PHY_Write_WD(LAN9303_Data *eng, ULONG data_ctl, ULONG data_wr)
{
        static ULONG         data_rd;

        I2C_DWWrite( eng,  0x29, data_wr  ); //[0A4h]PMI_DATA
        I2C_DWWrite( eng,  0x2a, data_ctl ); //[0A8h]PMI_ACCESS
        do {
            data_rd = I2C_DWRead( eng, 0x2a );
        } while(data_rd & 0x00000001); //[0A8h]PMI_ACCESS
}

//------------------------------------------------------------
// Virtual PHY Read/Write
//------------------------------------------------------------
ULONG LAN9303_VirPHY_Read(LAN9303_Data *eng, ULONG reg_adr)
{
        static ULONG         data_rd;

        data_rd = I2C_DWRead( eng, 0x70 + reg_adr );//[1C0h]
        if ( Print_PHYRW )
            printf("VirPHY Reg:%2d, Data:%08x\n", reg_adr, data_rd);

        return(data_rd);
}

//------------------------------------------------------------
void LAN9303_VirPHY_Write(LAN9303_Data *eng, ULONG reg_adr, ULONG data_wr)
{
        I2C_DWWrite( eng, 0x70+reg_adr, data_wr );//[1C0h]
}

//------------------------------------------------------------
void LAN9303_VirPHY_RW(LAN9303_Data *eng, ULONG reg_adr, ULONG data_clr, ULONG data_set)
{
        I2C_DWWrite( eng, 0x70+reg_adr, (LAN9303_VirPHY_Read( eng, reg_adr ) & (~data_clr)) | data_set );//[1C0h]
}

//------------------------------------------------------------
// PHY Read/Write
//------------------------------------------------------------
ULONG LAN9303_Read(LAN9303_Data *eng, ULONG adr)
{
        static ULONG         data_rd;

        I2C_DWWrite( eng, 0x6c, 0xc00f0000 | adr & 0xffff );//[1B0h]SWITCH_CSR_CMD
        do {
            data_rd = I2C_DWRead( eng, 0x6c );
        } while( data_rd & 0x80000000 );//[1B0h]SWITCH_CSR_CMD

        return( I2C_DWRead( eng, 0x6b ) );//[1ACh]SWITCH_CSR_DATA
}

//------------------------------------------------------------
void LAN9303_Write(LAN9303_Data *eng, ULONG adr, ULONG data)
{
        static ULONG         data_rd;

        I2C_DWWrite( eng, 0x6b, data );//[1ACh]SWITCH_CSR_DATA
        I2C_DWWrite( eng, 0x6c, 0x800f0000 | adr & 0xffff );//[1B0h]SWITCH_CSR_CMD

        do {
            data_rd = I2C_DWRead( eng, 0x6c );
        } while( data_rd & 0x80000000 );//[1B0h]SWITCH_CSR_CMD
}

//------------------------------------------------------------
void LAN9303(int num, int phy_adr, int speed, int int_loopback)
{
	LAN9303_Data         LAN9303_ENG;
	LAN9303_Data         *eng;
	static ULONG         data_rd;

	eng = &LAN9303_ENG;
    //------------------------------------------------------------
    // I2C Initial
    //------------------------------------------------------------
        eng->busnum = num;
        if (eng->busnum <= 7) eng->devbase = 0x1E78A000 + ( eng->busnum    * 0x40);
        else                  eng->devbase = 0x1E78A300 + ((eng->busnum-8) * 0x40);
        Initial( eng, eng->devbase, 10, 10 );

    //------------------------------------------------------------
    // LAN9303 Register Setting
    //------------------------------------------------------------
    printf("----> Start\n");
        if (int_loopback == 0) {
            //Force Speed & external loopback
                if (speed == 1) { //100M
                        LAN9303_VirPHY_RW( eng,  0, 0xffff, 0x2300 );      //adr clr set //VPHY_BASIC_CTRL
                        LAN9303_VirPHY_RW( eng, 11, 0xffff, 0x2300 );      //adr clr set //P1_MII_BASIC_CONTROL
                        LAN9303_PHY_Write( eng, phy_adr + 1, 0, 0x2300 );
                        LAN9303_PHY_Write( eng, phy_adr + 2, 0, 0x2300 );
                }
                else {
                        LAN9303_VirPHY_RW( eng,  0, 0xffff, 0x0100 );      //adr clr set //VPHY_BASIC_CTRL
                        LAN9303_VirPHY_RW( eng, 11, 0xffff, 0x0100 );      //adr clr set //P1_MII_BASIC_CONTROL
                        LAN9303_PHY_Write( eng, phy_adr + 1, 0, 0x0100 );
                        LAN9303_PHY_Write( eng, phy_adr + 2, 0, 0x0100 );
                }

                LAN9303_Write( eng, 0x180c, 0x00000001 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000010 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

                LAN9303_Write( eng, 0x180c, 0x00000002 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000011 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

                LAN9303_Write( eng, 0x180c, 0x00000003 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000012 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

#ifdef LAN9303M
                LAN9303_Write( eng, 0x180c, 0x00022001 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000000 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

                LAN9303_Write( eng, 0x180c, 0x00024002 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000001 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

                LAN9303_Write( eng, 0x180c, 0x0002a003 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000002 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);
#else
                LAN9303_Write( eng, 0x180c, 0x0002a001 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000000 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

                LAN9303_Write( eng, 0x180c, 0x0000a002 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000001 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);

                LAN9303_Write( eng, 0x180c, 0x00022003 ); // SWE_VLAN_WR_DATA
                LAN9303_Write( eng, 0x180b, 0x00000002 ); // SWE_VLAN_CMD
                do {data_rd = LAN9303_Read( eng, 0x1810 );} while(data_rd & 0x1);
#endif
                LAN9303_Write( eng, 0x1840, 0x00000007);
        }
        else if ( int_loopback == 1 ) {
            //Force Speed & internal loopback
                if ( speed == 1 ) {
                    //100M
                        LAN9303_VirPHY_RW( eng,  0, 0xffff, 0x6300 ); // adr clr set //VPHY_BASIC_CTRL
                        LAN9303_VirPHY_RW( eng, 11, 0xffff, 0x6300 ); // adr clr set //P1_MII_BASIC_CONTROL
                        LAN9303_PHY_Write( eng, phy_adr + 1, 0, 0x6300 );
                        LAN9303_PHY_Write( eng, phy_adr + 2, 0, 0x6300 );
                }
                else {
                        LAN9303_VirPHY_RW( eng,  0, 0xffff, 0x4100 ); // adr clr set //VPHY_BASIC_CTRL
                        LAN9303_VirPHY_RW( eng, 11, 0xffff, 0x4100 ); // adr clr set //P1_MII_BASIC_CONTROL
                        LAN9303_PHY_Write( eng, phy_adr + 1, 0, 0x4100 );
                        LAN9303_PHY_Write( eng, phy_adr + 2, 0, 0x4100 );
                }
        }
        else {
            //Force Speed
                if (speed == 1) {
                    //100M
                        LAN9303_VirPHY_RW( eng,  0, 0xffff, 0x2300 ); // adr clr set //VPHY_BASIC_CTRL
                        LAN9303_VirPHY_RW( eng, 11, 0xffff, 0x2300 ); // adr clr set //P1_MII_BASIC_CONTROL
                        LAN9303_PHY_Write( eng, phy_adr + 1, 0, 0x2300 );
                        LAN9303_PHY_Write( eng, phy_adr + 2, 0, 0x2300 );
                }
                else {
                        LAN9303_VirPHY_RW( eng,  0, 0xffff, 0x0100 ); // adr clr set //VPHY_BASIC_CTRL
                        LAN9303_VirPHY_RW( eng, 11, 0xffff, 0x0100 ); // adr clr set //P1_MII_BASIC_CONTROL
                        LAN9303_PHY_Write( eng, phy_adr + 1, 0, 0x0100 );
                        LAN9303_PHY_Write( eng, phy_adr + 2, 0, 0x0100 );
                }
#ifdef LAN9303M
#else
                if (int_loopback == 3) {
                    //[LAN9303]IEEE measurement
                        data_rd = LAN9303_PHY_Read( eng, phy_adr+1, 27 );//PHY_SPECIAL_CONTROL_STAT_IND_x
                        LAN9303_PHY_Write( eng,phy_adr+1, 27, (data_rd & 0x9fff) | 0x8000 );//PHY_SPECIAL_CONTROL_STAT_IND_x

                        data_rd = LAN9303_PHY_Read( eng, phy_adr+2, 27 );//PHY_SPECIAL_CONTROL_STAT_IND_x
                        LAN9303_PHY_Write( eng,phy_adr+2, 27, (data_rd & 0x9fff) | 0x8000 );//PHY_SPECIAL_CONTROL_STAT_IND_x
                }
#endif
        } // End if (int_loopback == 0)
} // End void LAN9303(int num, int phy_adr, int speed, int int_loopback)
#endif // SUPPORT_PHY_LAN9303

