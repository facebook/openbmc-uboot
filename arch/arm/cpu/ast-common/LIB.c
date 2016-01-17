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

#define LIB_C
static const char ThisFile[] = "LIB.c";

#include "SWFUNC.H"

#ifdef SLT_UBOOT
  #include <common.h>
  #include <command.h>
#endif
#ifdef DOS_ALONE
  #include <stdlib.h>
  #include <stdio.h>
  #include <time.h>
  #include <conio.h>
  #include <dos.h>
  #include <mem.h>
#endif
#if defined(LinuxAP)
  #include <stdio.h>
#endif

#include "LIB.H"
#include "TYPEDEF.H"

#ifdef USE_P2A
//------------------------------------------------------------
// PCI
//------------------------------------------------------------
ULONG ReadPCIReg (ULONG ulPCIConfigAddress, BYTE jOffest, ULONG ulMask)
{
#ifndef Windows
    OUTDWPORT(0xcf8, ulPCIConfigAddress + jOffest);

    return (((ULONG)INDWPORT(0xcfc)) & ulMask);
#else
    WRITE_PORT_ULONG((PULONG)0xcf8, ulPCIConfigAddress + jOffest);

    return (READ_PORT_ULONG((PULONG)0xcfc) & ulMask);
#endif
}

//------------------------------------------------------------
VOID WritePCIReg (ULONG ulPCIConfigAddress, BYTE jOffest, ULONG ulMask, ULONG ulData)
{
#ifndef Windows
    OUTDWPORT(0xcf8, ulPCIConfigAddress + jOffest);
    OUTDWPORT(0xcfc, (INDWPORT(0xcfc) & ulMask | ulData));
#else
    WRITE_PORT_ULONG((PULONG)0xcf8, ulPCIConfigAddress + jOffest);
    WRITE_PORT_ULONG((PULONG)0xcfc, (READ_PORT_ULONG((PULONG)0xcfc) & ulMask | ulData));
#endif
}

//------------------------------------------------------------
ULONG FindPCIDevice (USHORT usVendorID, USHORT usDeviceID, USHORT usBusType)
{
//Return: ulPCIConfigAddress
//usBusType: ACTIVE/PCI/AGP/PCI-E

    ULONG   Base[256];
    ULONG   ebx;
    USHORT  i;
    USHORT  j;

    for (i = 0; i < 256; i++) {
        Base[i] = 0x80000000 + 0x10000 * i;
    }

    if (usBusType == PCI)
    {
      ebx = 0x80000000;
    }
    else if (usBusType == PCIE)
    {
      ebx = 0x80020000;
    }
    else     // AGP and ACTIVE
    {
      ebx = 0x80010000;
    }

    if ( usBusType != ACTIVE )    //AGP, PCI, PCIE
    {
      for (i = 0; i < 32; i++)
      {
        ebx = ebx + (0x800);
        if (((USHORT)ReadPCIReg(ebx, 0, 0xffff) == usVendorID) && ((USHORT)(ReadPCIReg(ebx, 0, 0xffff0000) >> 16) == usDeviceID))
        {
          return ebx;
        }
      }
      return 0;
    }
    else     //ACTIVE
    {
      for (j = 0; j < 256; j++)
      {
        ebx = Base[j];
        for (i = 0; i < 32; i++)
        {
          ebx = ebx + (0x800);
          if (((USHORT)ReadPCIReg(ebx, 0, 0xffff) == usVendorID) && ((USHORT)(ReadPCIReg(ebx, 0, 0xffff0000) >> 16) == usDeviceID))
          {
            return ebx;
          }
        }
      }
      return 0;
    }
} // End ULONG FindPCIDevice (USHORT usVendorID, USHORT usDeviceID, USHORT usBusType)
#endif
//------------------------------------------------------------
// Allocate Resource
//------------------------------------------------------------
#ifdef DOS_ALONE
ULONG InitDOS32()
{
  union REGS regs ;

  regs.w.ax = 0xee00;
  INTFUNC(0x31, &regs, &regs) ;

  if(regs.w.ax >= 0x301)    // DOS32 version >= 3.01 ?
    return 1;
  else
    return 0;
}

//------------------------------------------------------------
USHORT CheckDOS()
{
    union REGS  regs;

    regs.w.ax = 0xeeff;
    int386(0x31, &regs, &regs);
    if (regs.x.eax == 0x504d4457)
    {
        return 0;
    } else {
        printf("PMODEW Init. fail\n");
        return 1;
    }
}

//------------------------------------------------------------
ULONG MapPhysicalToLinear (ULONG ulBaseAddress, ULONG ulSize)
{
  union REGS regs;

  regs.w.ax = 0x0800;                        // map physcial memory
  regs.w.bx = ulBaseAddress >> 16;           // bx:cx = physical address
  regs.w.cx = ulBaseAddress;
  regs.w.si = ulSize >> 16;                  // si:di = mapped memory block size
  regs.w.di = ulSize;
  INTFUNC(0x31, &regs, &regs);               // int386(0x31, &regs, &regs);
  if (regs.w.cflag == 0)
    return (ULONG) (regs.w.bx << 16 + regs.w.cx);  // Linear Addr = bx:cx
  else
    return 0;
}

//------------------------------------------------------------
USHORT FreePhysicalMapping(ULONG udwLinAddress)
{
    union REGS regs;

    regs.w.ax = 0x0801;
    regs.w.bx = udwLinAddress >> 16;
    regs.w.cx = udwLinAddress & 0xFFFF;
    int386(0x31, &regs, &regs);

    if (regs.x.cflag)
        return ((USHORT) 0);
    else return ((USHORT) 1);
}
#endif

