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

#ifndef LIB_H
#define LIB_H

#include "TYPEDEF.H"
#if defined(DOS_ALONE) || defined(SLT_NEW_ARCH) || defined(LinuxAP)
  #include "COMMINF.H"
#endif
//
// Macro
//
#define   INTFUNC              int386

#define   OUTDWPORT         	outpd
#define   INDWPORT          	inpd
#define   OUTPUT            	outp
#define   INPUT             	inp

//
// PCI
//
ULONG ReadPCIReg (ULONG ulPCIConfigAddress, BYTE jOffest, ULONG ulMask);
ULONG FindPCIDevice (USHORT usVendorID, USHORT usDeviceID, USHORT usBusType);
VOID  WritePCIReg (ULONG ulPCIConfigAddress, BYTE jOffest, ULONG ulMask, ULONG ulData);

//
// Map Resource
//
ULONG    MapPhysicalToLinear (ULONG ulBaseAddress, ULONG ulSize);
#endif
