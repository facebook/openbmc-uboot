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

#ifndef IO_H
#define IO_H

#include "SWFUNC.H"

//
// Macro
//
#if 	defined(LinuxAP)
    #define ob(p,d) outb(d,p)
    #define ib(p) inb(p)
#else
    #define ob(p,d)	outp(p,d)
    #define ib(p) inp(p)
#endif

#if defined(LinuxAP)
typedef enum _ACCESS_MODE {
    P2A_MODE,
    LPC_MODE
} ACCESS_MODE;

typedef enum _MAC_INTERFACE {
	MDC_INTERFACE,
	NCSI_INTERFACE
} MAC_INTERFACE;

typedef struct _MACINFO {
	USHORT  MACInterface __attribute__ ((aligned (4)));
	USHORT  AccessMode  __attribute__ ((aligned (4)));

    // PCIe interface
	ULONG   ulMMIOBaseAddress __attribute__ ((aligned (4)));
	UCHAR   *ulMMIOLinear __attribute__ ((aligned (4)));

	char    *argv[10] __attribute__ ((aligned (4)));
    int     argc __attribute__ ((aligned (4)));

    // LPC interface
    UCHAR   LPC_port __attribute__ ((aligned (4)));
} _MACInfo __attribute__ ((aligned (4)));

int  MACTest(_MACInfo  *MACInfo);
#if defined(USE_LPC)
  void SetLPCport( UCHAR port ); // LPC infterface
#endif
#endif // End defined(LinuxAP)

#ifdef USE_LPC
void open_aspeed_sio_password(void);
void enable_aspeed_LDU(BYTE jldu_number);
int findlpcport(BYTE jldu_number);
#endif

#ifndef WINSLT
void WriteSOC_DD(ULONG addr, ULONG data);
ULONG ReadSOC_DD(ULONG addr);
#endif
#endif
