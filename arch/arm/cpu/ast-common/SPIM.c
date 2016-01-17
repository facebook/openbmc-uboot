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

#define SPIM_C
static const char ThisFile[] = "SPIM.c";

#include "SWFUNC.H"

#ifdef SPI_BUS

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "TYPEDEF.H"
#include "LIB_SPI.H"

#define SPIM_CMD_WHA     0x01
#define SPIM_CMD_RD      0x0B
#define SPIM_CMD_DRD     0xBB
#define SPIM_CMD_WR      0x02
#define SPIM_CMD_DWR     0xD2
#define SPIM_CMD_STA     0x05
#define SPIM_CMD_ENBYTE  0x06
#define SPIM_CMD_DISBYTE 0x04

ULONG spim_cs;
ULONG spim_base;
ULONG spim_hadr;

void spim_end()
{
  ULONG data;

  data = MIndwm((ULONG)mmiobase, 0x1E620010 + (spim_cs << 2));
  MOutdwm( (ULONG)mmiobase, 0x1E620010 + (spim_cs << 2), data | 0x4);
  MOutdwm( (ULONG)mmiobase, 0x1E620010 + (spim_cs << 2), data);
}

//------------------------------------------------------------
void spim_init(int cs)
{
  ULONG data;

  spim_cs = cs;
  MOutdwm( (ULONG)mmiobase, 0x1E620000, (0x2 << (cs << 1)) | (0x10000 << cs));
  MOutdwm( (ULONG)mmiobase, 0x1E620010 + (cs << 2), 0x00000007);
  MOutdwm( (ULONG)mmiobase, 0x1E620010 + (cs << 2), 0x00002003);
  MOutdwm( (ULONG)mmiobase, 0x1E620004, 0x100 << cs);
  data = MIndwm((ULONG)mmiobase, 0x1E620030 + (cs << 2));
  spim_base = 0x20000000 | ((data & 0x007f0000) << 7);
  MOutwm ( (ULONG)mmiobase, spim_base, SPIM_CMD_WHA);
  spim_end();
  spim_hadr = 0;
}
#endif // End SPI_BUS
