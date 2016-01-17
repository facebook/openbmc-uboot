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

#define DRAM_SPI_C
static const char ThisFile[] = "DRAM_SPI.c";

#include "SWFUNC.H"

#ifdef SPI_BUS
#include <stdio.h>
#include "DEF_SPI.H"
#include "LIB_SPI.H"

VOID Set_MMIO_Base(ULONG PCI_BASE, ULONG addr)
{
  static ULONG MMIO_BASE = -1;

  if(MMIO_BASE != (addr & 0xffff0000)){
    if(MMIO_BASE == -1){
      *(ULONG *)(PCI_BASE + 0xF000) = 1;
    }
    *(ULONG *)(PCI_BASE + 0xF004) = addr;
    MMIO_BASE = addr & 0xffff0000;
  }
}

VOID  MOutbm(ULONG PCI_BASE, ULONG Offset, BYTE Data)
{
  Set_MMIO_Base(PCI_BASE, Offset);
  *(BYTE *)(PCI_BASE + 0x10000 + (Offset & 0xffff)) = Data;
}

VOID  MOutwm(ULONG PCI_BASE, ULONG Offset, USHORT Data)
{
  Set_MMIO_Base(PCI_BASE, Offset);
  *(USHORT *)(PCI_BASE + 0x10000 + (Offset & 0xffff)) = Data;
}

VOID  MOutdwm(ULONG PCI_BASE, ULONG Offset, ULONG Data)
{
  Set_MMIO_Base(PCI_BASE, Offset);
  *(ULONG *)(PCI_BASE + 0x10000 + (Offset & 0xffff)) = Data;
}

BYTE  MInbm(ULONG PCI_BASE, ULONG Offset)
{
  BYTE jData;

  Set_MMIO_Base(PCI_BASE, Offset);
  jData = *(BYTE *)(PCI_BASE + 0x10000 + (Offset & 0xffff));
  return(jData);
}

USHORT  MInwm(ULONG PCI_BASE, ULONG Offset)
{
  USHORT usData;

  Set_MMIO_Base(PCI_BASE, Offset);
  usData = *(USHORT *)(PCI_BASE + 0x10000 + (Offset & 0xffff));
  return(usData);
}

ULONG  MIndwm(ULONG PCI_BASE, ULONG Offset)
{
  ULONG ulData;

  Set_MMIO_Base(PCI_BASE, Offset);
  ulData = *(ULONG *)(PCI_BASE + 0x10000 + (Offset & 0xffff));
  return(ulData);
}
#endif // End SPI_BUS
