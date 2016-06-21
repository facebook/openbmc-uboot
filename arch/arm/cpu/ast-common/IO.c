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

#define IO_C
static const char ThisFile[] = "IO.c";

#include "SWFUNC.H"

#if defined(LinuxAP)
  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <unistd.h>
  #include <string.h>
  #include <fcntl.h>
  #include <pthread.h>
  #include <sys/mman.h>
  #if defined(__i386__) || defined(__amd64__)
    #include <sys/io.h>
  #endif
  #include "COMMINF.H"
#endif
#if defined(SLT_UBOOT)
  #include <common.h>
  #include <command.h>
  #include <post.h>
  #include <malloc.h>
  #include <net.h>
  #include "COMMINF.H"
#endif
#if defined(DOS_ALONE)
  #include <stdlib.h>
  #include <stdio.h>
  #include <time.h>
  #include <conio.h>
  #include <dos.h>
  #include <mem.h>
  #include "TYPEDEF.H"
  #include "LIB.H"
  #include "COMMINF.H"
#endif
#if defined(SLT_NEW_ARCH)
  #include <stdlib.h>
  #include <conio.h>
  #include "TYPEDEF.H"
  #include "LIB.H"
  #include "COMMINF.H"
#endif

#include "TYPEDEF.H"
#include "IO.H"
#include "LIB_SPI.H"

#ifdef SPI_BUS
#endif
#if defined(USE_LPC)
    USHORT	usLPCPort;
#endif
#if defined(USE_P2A)
#endif

#if defined(USE_LPC)
//------------------------------------------------------------
// LPC access
//------------------------------------------------------------
void open_aspeed_sio_password(void)
{
    ob (usLPCPort, 0xaa);

    ob (usLPCPort, 0xa5);
    ob (usLPCPort, 0xa5);
}

//------------------------------------------------------------
void close_aspeed_sio_password(void)
{
    ob (usLPCPort, 0xaa);
}

//------------------------------------------------------------
void enable_aspeed_LDU(BYTE jldu_number)
{
    ob (usLPCPort, 0x07);
    ob ((usLPCPort + 1), jldu_number);
    ob (usLPCPort, 0x30);
    ob ((usLPCPort + 1), 0x01);
}

//------------------------------------------------------------
void disable_aspeed_LDU(BYTE jldu_number)
{
    ob (usLPCPort, 0x07);
    ob ((usLPCPort + 1), jldu_number);
    ob (usLPCPort, 0x30);
    ob ((usLPCPort + 1), 0x00);
}

//------------------------------------------------------------
/*
ulAddress = AHB address
jmode = 0: byte mode
        1: word mode
        2: dword mode
*/
static ULONG lpc_read (ULONG ulAddress, BYTE jmode)
{
    ULONG    uldata = 0;
    ULONG    ultemp = 0;
    BYTE     jtemp;

    //Write Address
    ob (  usLPCPort,        0xf0);
    ob ( (usLPCPort + 1  ), ((ulAddress & 0xff000000) >> 24));
    ob (  usLPCPort,        0xf1);
    ob ( (usLPCPort + 1)  , ((ulAddress & 0x00ff0000) >> 16));
    ob (  usLPCPort,        0xf2);
    ob ( (usLPCPort + 1),   ((ulAddress & 0x0000ff00) >> 8));
    ob (  usLPCPort,        0xf3);
    ob ( (usLPCPort + 1),   ulAddress & 0xff);

    //Write Mode
    ob (usLPCPort, 0xf8);
    jtemp = ib ((usLPCPort + 1));
    ob ((usLPCPort + 1), ((jtemp & 0xfc) | jmode));

    //Fire
    ob (usLPCPort, 0xfe);
    jtemp = ib ((usLPCPort + 1));

    //Get Data
    switch ( jmode )
    {
        case 0:
            ob (usLPCPort, 0xf7);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= (ultemp);
            break;

        case 1:
            ob (usLPCPort, 0xf6);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= (ultemp << 8);
            ob (usLPCPort, 0xf7);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= (ultemp << 0);
            break;

        case 2:
            ob (usLPCPort, 0xf4);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= (ultemp << 24);
            ob (usLPCPort, 0xf5);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= (ultemp << 16);
            ob (usLPCPort, 0xf6);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= (ultemp << 8);
            ob (usLPCPort, 0xf7);
            ultemp  = ib ((usLPCPort + 1));
            uldata |= ultemp;
            break;
    } // End switch ( jmode )

    return uldata;
} // End static ULONG lpc_read (ULONG ulAddress, BYTE jmode)

//------------------------------------------------------------
static void lpc_write (ULONG ulAddress, ULONG uldata, BYTE jmode)
{
    BYTE     jtemp;

    //Write Address
    ob ( usLPCPort,      0xf0);
    ob ((usLPCPort + 1), ((ulAddress & 0xff000000) >> 24));
    ob ( usLPCPort,      0xf1);
    ob ((usLPCPort + 1), ((ulAddress & 0x00ff0000) >> 16));
    ob ( usLPCPort,      0xf2);
    ob ((usLPCPort + 1), ((ulAddress & 0x0000ff00) >> 8));
    ob ( usLPCPort,      0xf3);
    ob ((usLPCPort + 1), ulAddress & 0xff);

    //Write Data
    switch ( jmode )
    {
        case 0:
            ob ( usLPCPort,      0xf7);
            ob ((usLPCPort + 1), (uldata & 0xff));
            break;
        case 1:
            ob ( usLPCPort,      0xf6);
            ob ((usLPCPort + 1), ((uldata & 0xff00) >> 8));
            ob ( usLPCPort,      0xf7);
            ob ((usLPCPort + 1), (uldata & 0x00ff));
            break;
        case 2:
            ob ( usLPCPort,      0xf4);
            ob ((usLPCPort + 1), ((uldata & 0xff000000) >> 24));
            ob ( usLPCPort,      0xf5);
            ob ((usLPCPort + 1), ((uldata & 0x00ff0000) >> 16));
            ob ( usLPCPort,      0xf6);
            ob ((usLPCPort + 1), ((uldata & 0x0000ff00) >> 8));
            ob ( usLPCPort,      0xf7);
            ob ((usLPCPort + 1), uldata & 0xff);
            break;
    } // End switch ( jmode )

    //Write Mode
    ob (usLPCPort, 0xf8);
    jtemp = ib ((usLPCPort + 1));
    ob ((usLPCPort + 1), ((jtemp & 0xfc) | jmode));

    //Fire
    ob (usLPCPort, 0xfe);
    ob ((usLPCPort + 1), 0xcf);

} // End static void lpc_write (ULONG ulAddress, ULONG uldata, BYTE jmode)

//------------------------------------------------------------
static USHORT usLPCPortList[] = {0x2e, 0x4e, 0xff};
int findlpcport(BYTE jldu_number)
{
    USHORT  *jLPCPortPtr;
    ULONG   ulData;

    jLPCPortPtr = usLPCPortList;
    while (*(USHORT *)(jLPCPortPtr) != 0xff )
    {
        usLPCPort = *(USHORT *)(jLPCPortPtr++);

        open_aspeed_sio_password();
        enable_aspeed_LDU(0x0d);

        ulData  = lpc_read( (SCU_BASE + 0x7C), 2);

        if ( (ulData != 0x00000000)	&&
             (ulData != 0xFFFFFFFF)   )
        {
            printf("Find LPC IO port at 0x%2x \n", usLPCPort );
            return 1;
        }

        disable_aspeed_LDU(0x0d);
        close_aspeed_sio_password();
    }

    //printf("[Error] Fail to find proper LPC IO Port \n");
    return 0;
}
#endif // End #if defined(USE_LPC)

#if defined(USE_P2A)
//------------------------------------------------------------
// A2P Access
//------------------------------------------------------------
void mm_write (ULONG addr, ULONG data, BYTE jmode)
{
    *((volatile ULONG *) (mmiobase + 0xF004)) = (ULONG) ((addr) & 0xFFFF0000);
#if defined(__powerpc64__)
    asm("eieio");
#endif
    *((volatile ULONG *) (mmiobase + 0xF000)) = (ULONG) 0x00000001;

    switch ( jmode )
    {
        case 0:
            *((volatile BYTE *) (mmiobase + 0x10000 + ((addr) & 0x0000FFFF))) = (BYTE) data;
            break;
        case 1:
            *((volatile USHORT *) (mmiobase + 0x10000 + ((addr) & 0x0000FFFF))) = (USHORT) data;
            break;
        case 2:
        default:
            *((volatile ULONG *) (mmiobase + 0x10000 + ((addr) & 0x0000FFFF))) = data;
            break;
    } //switch
}

//------------------------------------------------------------
ULONG mm_read (ULONG addr, BYTE jmode)
{
    *((volatile ULONG *) (mmiobase + 0xF004)) = (ULONG) ((addr) & 0xFFFF0000);
#if defined(__powerpc64__)
    asm("eieio");
#endif
    *((volatile ULONG *) (mmiobase + 0xF000)) = (ULONG) 0x00000001;
    switch ( jmode )
    {
    case 0:
        return ( *((volatile BYTE *) (mmiobase + 0x10000 + ((addr) & 0x0000FFFF))) );
        break;
    case 1:
        return ( *((volatile USHORT *) (mmiobase + 0x10000 + ((addr) & 0x0000FFFF))) );
        break;
    default:
    case 2:
        return ( *((volatile ULONG *) (mmiobase + 0x10000 + ((addr) & 0x0000FFFF))) );
        break;
    } //switch

    return 0;
}
#endif // End #if defined(USE_P2A)

//------------------------------------------------------------
// General Access API
//------------------------------------------------------------
#if ( defined(USE_LPC) && ( defined(LinuxAP) || defined(SLT_NEW_ARCH) ) )
void SetLPCport( UCHAR port )
{
    usLPCPort = (USHORT )port;
    printf("LPC port: [%X]\n", usLPCPort );
}
#endif // End #if ( defined(USE_LPC) && ( defined(LinuxAP) || defined(SLT_NEW_ARCH) ) )

#ifdef SLT_UBOOT
BYTE Check_BEorLN ( ULONG chkaddr )
{
    BYTE ret = BIG_ENDIAN_ADDRESS;
    BYTE i   = 0;

    do {
        if ( LittleEndianArea[i].StartAddr == LittleEndianArea[i].EndAddr )
            break;

        if ( ( LittleEndianArea[i].StartAddr <= chkaddr ) &&
             ( LittleEndianArea[i].EndAddr   >= chkaddr )    ) {
            ret = LITTLE_ENDIAN_ADDRESS;
            break;
        }
        i++;
    } while ( 1 );

    return ret;
}
#endif // End #ifdef SLT_UBOOT

void WriteSOC_DD(ULONG addr, ULONG data)
{
#ifdef SLT_UBOOT
    if ( Check_BEorLN( addr ) == BIG_ENDIAN_ADDRESS )
        *(volatile unsigned long *)(addr) = cpu_to_le32(data);
    else
        *(volatile unsigned long *)(addr) = data;
#else
  #if defined(USE_LPC)
        lpc_write(addr, data, 2);
  #endif
  #if defined(USE_P2A)
        mm_write(addr, data, 2);
  #endif
#endif
}

//------------------------------------------------------------
ULONG ReadSOC_DD(ULONG addr)
{
#if defined(SLT_UBOOT)
    if ( Check_BEorLN( addr ) == BIG_ENDIAN_ADDRESS )
        return le32_to_cpu(*(volatile unsigned long *) (addr));
    else
        return (*(volatile unsigned long *) (addr));
#else
  #if defined(USE_LPC)
        return (lpc_read(addr, 2));
  #endif
  #if defined(USE_P2A)
        return (mm_read(addr, 2));
  #endif
#endif
    return 0;
}

