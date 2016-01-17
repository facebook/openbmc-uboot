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

#ifndef LIB_SPI_H
#define LIB_SPI_H

#ifdef SPI_BUS
    // MMIO Functions
    VOID       MOutwm (ULONG, ULONG, USHORT);
    VOID       MOutdwm (ULONG, ULONG, ULONG);
    ULONG      MIndwm (ULONG, ULONG);

    void spim_init(int cs);
#endif

#endif // LIB_SPI_H
