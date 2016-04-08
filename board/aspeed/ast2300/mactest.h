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
/* MACTest.h */

// --------------------------------------------------------------------
//              General Definition
// --------------------------------------------------------------------
#define MAC1_IO_BASE		0x1E660000
#define PHY_LOOP		100000
#define NUM_RX			48
#define NUM_TX			48
#define RX_BUFF_SZ		1514
#define TX_BUFF_SZ		1514
#define TOUT_LOOP		1000000
#define ETH_ALEN		6
#define POLL_DEMAND		1


// --------------------------------------------------------------------
//              MAC Register Index
// --------------------------------------------------------------------
#define ISR_REG			0x00				// interrups status register
#define IER_REG			0x04				// interrupt maks register
#define MAC_MADR_REG		0x08				// MAC address (Most significant)
#define MAC_LADR_REG		0x0c				// MAC address (Least significant)
#define MAHT0_REG		0x10				// Multicast Address Hash Table 0 register
#define MAHT1_REG		0x14				// Multicast Address Hash Table 1 register
#define TXPD_REG		0x18				// Transmit Poll Demand register
#define RXPD_REG		0x1c				// Receive Poll Demand register
#define TXR_BADR_REG		0x20				// Transmit Ring Base Address register
#define RXR_BADR_REG		0x24				// Receive Ring Base Address register
#define HPTXPD_REG		0x28
#define HPTXR_BADR_REG		0x2c
#define ITC_REG			0x30				// interrupt timer control register
#define APTC_REG		0x34				// Automatic Polling Timer control register
#define DBLAC_REG		0x38				// DMA Burst Length and Arbitration control register
#define DMAFIFOS_REG		0x3c
#define FEAR_REG		0x44
#define TPAFCR_REG		0x48
#define RBSR_REG		0x4c
#define MACCR_REG		0x50				// MAC control register
#define MACSR_REG		0x54				// MAC status register
#define PHYCR_REG		0x60				// PHY control register
#define PHYDATA_REG		0x64				// PHY Write Data register

// --------------------------------------------------------------------
//		PHYCR_REG
// --------------------------------------------------------------------
#define PHY_RE_AUTO_bit			(1UL<<9)
#define PHY_READ_bit			(1UL<<26)
#define PHY_WRITE_bit			(1UL<<27)
// --------------------------------------------------------------------
//		PHYCR_REG 
// --------------------------------------------------------------------
#define PHY_AUTO_OK_bit			(1UL<<5)
// --------------------------------------------------------------------
//		PHY INT_STAT_REG 
// --------------------------------------------------------------------
#define PHY_SPEED_CHG_bit		(1UL<<14)
#define PHY_DUPLEX_CHG_bit		(1UL<<13)
#define PHY_LINK_CHG_bit		(1UL<<10)
#define PHY_AUTO_COMP_bit		(1UL<<11)
// --------------------------------------------------------------------
//		PHY SPE_STAT_REG 
// --------------------------------------------------------------------
#define PHY_RESOLVED_bit		(1UL<<11)
#define PHY_LINK_bit			(1UL<<10)
#define PHY_SPEED_mask			0xC000
#define PHY_SPEED_10M			0x0
#define PHY_SPEED_100M			0x1
#define PHY_SPEED_1G			0x2
#define PHY_DUPLEX_mask                 0x2000
#define PHY_SPEED_DUPLEX_MASK           0x01E0
#define PHY_100M_DUPLEX                 0x0100
#define PHY_100M_HALF                   0x0080
#define PHY_10M_DUPLEX                  0x0040
#define PHY_10M_HALF                    0x0020
#define LINK_STATUS                     0x04
#define PHYID_VENDOR_MASK		0xfffffc00
#define PHYID_VENDOR_MARVELL		0x01410c00
#define PHYID_VENDOR_BROADCOM		0x00406000
#define PHYID_VENDOR_RTL8201E		0x001cc800
#define DUPLEX_FULL			0x01
#define DUPLEX_HALF			0x00



// --------------------------------------------------------------------
//		MACCR_REG
// --------------------------------------------------------------------

#define SW_RST_bit		(1UL<<31)				// software reset/
#define DIRPATH_bit		(1UL<<21)
#define RX_IPCS_FAIL_bit	(1UL<<20)
#define SPEED_100M_MODE_bit		(1UL<<19)
#define RX_UDPCS_FAIL_bit	(1UL<<18)
#define RX_BROADPKT_bit		(1UL<<17)				// Receiving broadcast packet
#define RX_MULTIPKT_bit		(1UL<<16)				// receiving multicast packet
#define RX_HT_EN_bit		(1UL<<15)
#define RX_ALLADR_bit		(1UL<<14)				// not check incoming packet's destination address
#define JUMBO_LF_bit		(1UL<<13)
#define RX_RUNT_bit		(1UL<<12)				// Store incoming packet even its length is les than 64 byte
#define CRC_CHK_bit		(1UL<<11)
#define CRC_APD_bit		(1UL<<10)				// append crc to transmit packet
#define GMAC_MODE_bit		(1UL<<9)
#define FULLDUP_bit		(1UL<<8)				// full duplex
#define ENRX_IN_HALFTX_bit	(1UL<<7)
#define LOOP_EN_bit		(1UL<<6)				// Internal loop-back
#define HPTXR_EN_bit		(1UL<<5)
#define REMOVE_VLAN_bit		(1UL<<4)
#define RXMAC_EN_bit		(1UL<<3)				// receiver enable
#define TXMAC_EN_bit		(1UL<<2)				// transmitter enable
#define RXDMA_EN_bit		(1UL<<1)				// enable DMA receiving channel
#define TXDMA_EN_bit		(1UL<<0)				// enable DMA transmitting channel


// --------------------------------------------------------------------
//		SCU_REG
// --------------------------------------------------------------------
#define  SCU_BASE			0x1E6E2000
#define  SCU_PROTECT_KEY_REG            0x0
#define  SCU_PROT_KEY_MAGIC             0x1688a8a8
#define  SCU_RESET_CONTROL_REG          0x04
#define  SCU_RESET_MAC1                 (1u << 11)
#define  SCU_RESET_MAC2                 (1u << 12)
#define  SCU_HARDWARE_TRAPPING_REG      0x70
#define  SCU_HT_MAC_INTF_LSBIT          6
#define  SCU_HT_MAC_INTERFACE           (0x7u << SCU_HT_MAC_INTF_LSBIT)
#define  MAC_INTF_SINGLE_PORT_MODES     (1u<<0/*GMII*/ | 1u<<3/*MII_ONLY*/ | 1u<<4/*RMII_ONLY*/)
#define  SCU_HT_MAC_GMII                0x0u
// MII and MII mode
#define  SCU_HT_MAC_MII_MII             0x1u
#define  SCU_HT_MAC_MII_ONLY            0x3u
#define  SCU_HT_MAC_RMII_ONLY           0x4u
#define  SCU_MULTIFUNCTION_PIN_REG      0x74
#define  SCU_MFP_MAC2_PHYLINK           (1u << 26)
#define  SCU_MFP_MAC1_PHYLINK           (1u << 25)
#define  SCU_MFP_MAC2_MII_INTF          (1u << 21)
#define  SCU_MFP_MAC2_MDC_MDIO          (1u << 20)
#define  SCU_SILICON_REVISION_REG       0x7C

//---------------------------------------------------
//  PHY R/W Register Bit
//---------------------------------------------------
#define MIIWR                   (1UL<<27)
#define MIIRD                   (1UL<<26)
#define MDC_CYCTHR              0x34
#define PHY_SPEED_MASK          0xC000
#define PHY_DUPLEX_MASK         0x2000
#define SPEED_1000M             0x02
#define SPEED_100M              0x01
#define SPEED_10M               0x00
#define DUPLEX_FULL             0x01
#define DUPLEX_HALF             0x00
#define RESOLVED_BIT            0x800

#define PHY_SPEED_DUPLEX_MASK   0x01E0
#define PHY_100M_DUPLEX         0x0100
#define PHY_100M_HALF           0x0080
#define PHY_10M_DUPLEX          0x0040
#define PHY_10M_HALF            0x0020

//---------------------------------------------------
// Descriptor bits.
//---------------------------------------------------
#define TXDMA_OWN	0x80000000	/* Own Bit */
#define RXPKT_RDY       0x00000000      
#define RXPKT_STATUS    0x80000000
#define EDORR		0x40000000	/* Receive End Of Ring */
#define LRS		0x10000000	/* Last Descriptor */
#define RD_ES		0x00008000	/* Error Summary */
#define EDOTR		0x40000000	/* Transmit End Of Ring */
#define T_OWN		0x80000000	/* Own Bit */
#define LTS		0x10000000	/* Last Segment */
#define FTS		0x20000000	/* First Segment */
#define CRC_ERR         0x00080000
#define TD_ES		0x00008000	/* Error Summary */
#define TD_SET		0x08000000	/* Setup Packet */
#define RX_ERR          0x00040000
#define FTL             0x00100000
#define RUNT            0x00200000
#define RX_ODD_NB       0x00400000
#define BYTE_COUNT_MASK 0x00003FFF

//---------------------------------------------------
// SPEED/DUPLEX Parameters
//---------------------------------------------------

//---------------------------------------------------
// Return Status
//---------------------------------------------------
#define TEST_PASS			0
#define PACKET_TEST_FAIL		1
#define PACKET_LENGTH_TEST_FAIL		2

struct  mac_desc {
	volatile s32 status;
	u32 des1;
	u32 reserved;
	u32 buf;	
};
static struct mac_desc rx_ring[NUM_RX] __attribute__ ((aligned(32))); /* RX descriptor ring         */
static struct mac_desc tx_ring[NUM_TX] __attribute__ ((aligned(32))); /* TX descriptor ring         */
static int rx_new;                             /* RX descriptor ring pointer */
static int tx_new;                             /* TX descriptor ring pointer */
static volatile unsigned char rx_buffer[NUM_RX][RX_BUFF_SZ] __attribute__ ((aligned(32))); /* RX buffer         */
static volatile unsigned char tx_buffer[NUM_TX][TX_BUFF_SZ] __attribute__ ((aligned(32))); /* TX buffer         */
