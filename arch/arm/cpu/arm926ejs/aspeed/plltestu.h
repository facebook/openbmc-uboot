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
#ifndef PLLTESTU_H
#define PLLTESTU_H

//PLL Mode Definition
#define	NAND_PLLMODE		    0x00
#define	DELAY_PLLMODE		    0x04
#define	PCI_PLLMODE	            0x08
#define	DPLL_PLLMODE		    0x2c
#define	MPLL_PLLMODE		    0x10
#define	HPLL_PLLMODE		    0x14
#define	LPC_PLLMODE	            0x18
#define	VIDEOA_PLLMODE		    0x1c
#define	D2PLL_PLLMODE		    0x0c
#define	VIDEOB_PLLMODE		    0x3c

#define	PCI_PLLMODE_AST1160     0x10
#define	MPLL_PLLMODE_AST1160	0x14
#define	HPLL_PLLMODE_AST1160	0x14
#define	DPLL_PLLMODE_AST1160	0x1c

#define	PCI_PLLMODE_AST2300     0x2c
#define	MPLL_PLLMODE_AST2300	0x10
#define	HPLL_PLLMODE_AST2300	0x30
#define	DPLL_PLLMODE_AST2300	0x08
#define	DEL0_PLLMODE_AST2300	0x00

#define ERR_FATAL		0x00000001

typedef struct _VGAINFO {
    USHORT usDeviceID;
    UCHAR  jRevision;           
    
    ULONG  ulMCLK;
    ULONG  ulDRAMBusWidth;    
    
    ULONG  ulCPUCLK;
    ULONG  ulAHBCLK;    
} _VGAInfo;

#endif // End PLLTESTU_H
