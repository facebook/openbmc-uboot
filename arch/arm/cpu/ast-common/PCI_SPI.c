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

#define PCI_SPI_C
static const char ThisFile[] = "PCI_SPI.c";

#include "SWFUNC.H"

#ifdef SLT_UBOOT
  #include <common.h>
  #include <command.h>
#endif
#ifdef DOS_ALONE
  #include <stdio.h>
  #include <stdlib.h>
  #include <conio.h>
  #include <string.h>
  #include <dos.h>
#endif

#include "DEF_SPI.H"
#include "LIB.H"
#include "TYPEDEF.H"

#ifdef SPI_BUS
ULONG  GetPCIInfo (DEVICE_PCI_INFO  *VGAPCIInfo)
{
    ULONG ulPCIBaseAddress, MMIOBaseAddress, LinearAddressBase, busnum, data;

    ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x2000, ACTIVE);
    busnum = 0;
    while (ulPCIBaseAddress == 0 && busnum < 256) {
        ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x2000, busnum);
        if (ulPCIBaseAddress == 0) {
            ulPCIBaseAddress = FindPCIDevice (0x1688, 0x2000, busnum);
        }
        if (ulPCIBaseAddress == 0) {
            ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x1160, busnum);
        }
        if (ulPCIBaseAddress == 0) {
            ulPCIBaseAddress = FindPCIDevice (0x1A03, 0x1180, busnum);
        }
        busnum++;
    }
    printf("ulPCIBaseAddress = %lx\n", ulPCIBaseAddress);
    if (ulPCIBaseAddress != 0) {
        VGAPCIInfo->ulPCIConfigurationBaseAddress = ulPCIBaseAddress;
        VGAPCIInfo->usVendorID = ReadPCIReg(ulPCIBaseAddress, 0, 0xFFFF);
        VGAPCIInfo->usDeviceID = ReadPCIReg(ulPCIBaseAddress, 0, 0xFFFF0000) >> 16;
        LinearAddressBase = ReadPCIReg (ulPCIBaseAddress, 0x10, 0xFFFFFFF0);
        VGAPCIInfo->ulPhysicalBaseAddress = MapPhysicalToLinear (LinearAddressBase, 64 * 1024 * 1024 + 0x200000);
        MMIOBaseAddress = ReadPCIReg (ulPCIBaseAddress, 0x14, 0xFFFF0000);
        VGAPCIInfo->ulMMIOBaseAddress = MapPhysicalToLinear (MMIOBaseAddress, 64 * 1024 * 1024);
        VGAPCIInfo->usRelocateIO = ReadPCIReg (ulPCIBaseAddress, 0x18, 0x0000FF80);
        OUTDWPORT(0xcf8, ulPCIBaseAddress + 0x4);
        data = INDWPORT(0xcfc);
        OUTDWPORT(0xcfc, data | 0x3);
        return    TRUE;
    }
    else {
        return    FALSE;
    }
} // End ULONG  GetPCIInfo (DEVICE_PCI_INFO  *VGAPCIInfo)

BOOLEAN  GetDevicePCIInfo (VIDEO_ENGINE_INFO *VideoEngineInfo)
{
    if (GetPCIInfo (&VideoEngineInfo->VGAPCIInfo) == TRUE) {
        return    TRUE;
    }
    else {
        printf("Can not find PCI device!\n");
        exit(0);
        return    FALSE;
    }
} // End
#endif // End ifdef SPI_BUS
