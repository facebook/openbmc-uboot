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
#ifndef TYPEDEF_H
#define TYPEDEF_H

#include "swfunc.h"

//
// Define
//
#define   PCI                   1
#define   PCIE                  2
#define   AGP                   3
#define   ACTIVE                4

#if defined(LinuxAP)
    #ifndef FLONG
        #define FLONG   unsigned long
    #endif
    #ifndef ULONG
        #define ULONG   unsigned long
    #endif
    #ifndef LONG
        #define LONG    long
    #endif
    #ifndef USHORT
        #define USHORT  unsigned short
    #endif
    #ifndef SHORT
        #define SHORT   short
    #endif
    #ifndef UCHAR
        #define UCHAR   unsigned char
    #endif
    #ifndef CHAR
        #define CHAR    char
    #endif
    #ifndef BYTE
        #define BYTE    unsigned char
    #endif
    #ifndef VOID
        #define VOID    void
    #endif
    #ifndef SCHAR
        #define SCHAR   signed char
    #endif
#else	
/* DOS Program */
    #define     VOID	  void
    #define     FLONG     unsigned long
    #define     ULONG     unsigned long
    #define     USHORT    unsigned short
    #define     UCHAR     unsigned char
    #define     LONG      long
    #define     SHORT     short
    #define     CHAR      char
    #define     BYTE	  UCHAR
    #define     BOOL	  SHORT
    #define     BOOLEAN   unsigned short
    #define     PULONG    ULONG *
    #define     SCHAR     signed char
#endif    
    #define	    TRUE      1
    #define	    FALSE     0
    
#endif // TYPEDEF_H
