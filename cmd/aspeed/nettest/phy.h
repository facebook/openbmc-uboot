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

#ifndef PHY_H
#define PHY_H

//
// Define
//

#define PHY_IS_VALID(dat)                                                      \
	(((dat & 0xffff) != 0xffff) && ((dat & 0xffff) != 0x0))

// Define PHY basic register
#define PHY_REG_BMCR    0x00 // Basic Mode Control Register
#define PHY_REG_BMSR    0x01 // Basic Mode Status Register
#define PHY_REG_ID_1    0x02
#define PHY_REG_ID_2    0x03
#define PHY_ANER        0x06 // Auto-negotiation Expansion Register
#define PHY_GBCR        0x09 // 1000Base-T Control Register
#define PHY_SR          0x11 // PHY Specific Status Register
#define PHY_INER        0x12 // Interrupt Enable Register

#define PHYID3_Mask                0xfc00         //0xffc0

/* --- Note for SettingPHY chip ---
void phy_xxxx (int loop_phy) {

	if ( BurstEnable ) {
        // IEEE test
	}
	else if (loop_phy) {
        // Internal loop back
	}
	else {
        // external loop back
	}
}
----------------------------------- */

#endif // PHY_H
