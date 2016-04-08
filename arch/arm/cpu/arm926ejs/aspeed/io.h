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

#include "swfunc.h"

//
// Macro
//
#if 	defined(LinuxAP)
    #define delay(val)	usleep(val*1000)
    #define ob(p,d) outb(d,p)
    #define ib(p) inb(p)
#else
    #define ob(p,d)	outp(p,d)
    #define ib(p) inp(p)
#endif

#ifdef USE_LPC
void open_aspeed_sio_password(void);
void enable_aspeed_LDU(BYTE jldu_number);
int findlpcport(BYTE jldu_number);
#endif

void WriteSOC_DD(ULONG addr, ULONG data);
ULONG ReadSOC_DD(ULONG addr);
#endif
