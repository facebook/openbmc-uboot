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
/*
 * (C) Copyright 2007 ASPEED Software
 * MAC Manufacture Test in ASPEED's SDK version 0.20.01
 *
 * Release History
 * 1. First Release, river@20071130
 * 2. Fix the endless loop when PHY is not ready, river@20071204
 *
 * Test items:
 * 1. Support MARVELL PHY only in this version
 * 2. MDC/MDIO
 * 3. GMAC/Duplex TX/RX Full_Size, Packet_Length Test
 * 4. 100M/Duplex TX/RX Full_Size, Packet_Length Test
 *
 *
 *
*/


/*
* Diagnostics support
*/
#include <common.h>
#include <command.h>
#include <post.h>
#include <malloc.h>
#include <net.h>
#include "slt.h"

#if ((CFG_CMD_SLT & CFG_CMD_MACTEST) && defined(CONFIG_SLT))
#include "mactest.h"

static int INL(u_long base, u_long addr)
{
	return le32_to_cpu(*(volatile u_long *)(addr + base));
}

static void OUTL(u_long base, int command, u_long addr)
{
	*(volatile u_long *)(addr + base) = cpu_to_le32(command);
}


static void SCU_MAC1_Enable (void)
{
	unsigned int SCU_Register;

//MAC1 RESET/PHY_LINK in SCU
        SCU_Register = INL(SCU_BASE, SCU_RESET_CONTROL_REG);
	OUTL(SCU_BASE, SCU_Register & ~(0x800), SCU_RESET_CONTROL_REG);
	
}

/*------------------------------------------------------------
 . Reads a register from the MII Management serial interface
 .-------------------------------------------------------------*/
static u16 phy_read_register (u8 PHY_Register, u8 PHY_Address)
{
	u32 Data, Status = 0, Loop_Count = 0, PHY_Ready = 1;
	u16 Return_Data;

	OUTL(MAC1_IO_BASE, (PHY_Register << 21) + (PHY_Address << 16) + MIIRD + MDC_CYCTHR, PHYCR_REG);
	do {
		Status = (INL (MAC1_IO_BASE, PHYCR_REG) & MIIRD);
		Loop_Count++;
		if (Loop_Count >= PHY_LOOP) {
			PHY_Ready = 0;
			break;
		}
	} while (Status == MIIRD);
       
	if (PHY_Ready == 0) {
		printf ("PHY NOT REDAY\n");
		return 0;
	}
	
	udelay(5*1000);
	Data = INL (MAC1_IO_BASE, PHYDATA_REG);
	Return_Data = (Data >> 16);

	return Return_Data;
}

static void phy_write_register (u8 PHY_Register, u8 PHY_Address, u16 PHY_Data)
{
	u32 Status = 0, Loop_Count = 0, PHY_Ready = 1;

	OUTL(MAC1_IO_BASE, PHY_Data, PHYDATA_REG);
	OUTL(MAC1_IO_BASE, (PHY_Register << 21) + (PHY_Address << 16) + MIIWR + MDC_CYCTHR, PHYCR_REG);
	do {
		Status = (INL (MAC1_IO_BASE, PHYCR_REG) & MIIWR);
		Loop_Count++;
		if (Loop_Count >= PHY_LOOP) {
			PHY_Ready = 0;
			break;
		}
	} while (Status == MIIWR);
	if (PHY_Ready == 0) {
		printf ("PHY NOT REDAY\n");
	}
}

static int wait_link_resolve (void)
{
	int resolved_status, Loop_Count = 0, PHY_Ready = 1;

        do {
       		resolved_status = (phy_read_register (0x11, 0) & (PHY_RESOLVED_bit | PHY_LINK_bit));
               	Loop_Count++;
               	if (Loop_Count >= PHY_LOOP) {
                   	PHY_Ready = 0;
                    	printf ("PHY NOT READY\n");
                    	break;
                }
        } while (resolved_status != (PHY_RESOLVED_bit | PHY_LINK_bit));
        
        return PHY_Ready;
}

static void set_phy_speed (int chip, int speed, int duplex)
{
	unsigned short data, status;


	if (chip == PHYID_VENDOR_MARVELL) {
		if ((speed == PHY_SPEED_1G) && (duplex == DUPLEX_FULL)) {
//Manual Control
			phy_write_register (18, 0, 0);
			data = phy_read_register (9, 0);
			phy_write_register (9, 0, data | 0x1800);
//PHY Reset
			phy_write_register (0, 0, 0x0140 | 0x8000);
			do {
				status = (phy_read_register (0, 0) & 0x8000); 
			} while (status != 0);
	
//Force 1G
			phy_write_register (29, 0, 0x07);
			data = phy_read_register (30, 0);
			phy_write_register (30, 0, data | 0x08);
			phy_write_register (29, 0, 0x10);
			data = phy_read_register (30, 0);
			phy_write_register (30, 0, data | 0x02);
			phy_write_register (29, 0, 0x12);
			data = phy_read_register (30, 0);
			phy_write_register (30, 0, data | 0x01);
	
			printf ("FORCE MARVELL PHY to 1G/DUPLEX DONE\n");
		}
		else if ((speed == PHY_SPEED_100M) && (duplex == DUPLEX_FULL)) {
//PHY Reset
			phy_write_register (0, 0, 0x2100 | 0x8000);
			do {
				status = (phy_read_register (0, 0) & 0x8000); 
			} while (status != 0);
	
//Force 100M
			data = phy_read_register (0, 0);
			phy_write_register (0, 0, data | 0x4000 | 0x8000);
			do {
				status = (phy_read_register (0, 0) & 0x8000); 
			} while (status != 0);
			data = phy_read_register (0, 0);

			printf ("FORCE MARVELL PHY to 100M/DUPLEX DONE\n");
		}
	}
	else if ( (chip == PHYID_VENDOR_RTL8201E) || (chip == PHYID_VENDOR_BROADCOM) ){
		/* basic setting */
		data  = phy_read_register (0, 0);
		data &= 0x7140;
		data |= 0x4000;
		if (speed == PHY_SPEED_100M)
		    data |= 0x2000;
                if (duplex == DUPLEX_FULL)		    
                    data |= 0x0100;
		phy_write_register (0, 0, data);

                /* reset */
		phy_write_register (0, 0, data | 0x8000);
		do {
			status = (phy_read_register (0, 0) & 0x8000); 
		} while (status != 0);
	        udelay(100*1000);
		
		/* basic setting */
		phy_write_register (0, 0, data);

                if (chip == PHYID_VENDOR_RTL8201E)
		    printf ("FORCE RTL8201E PHY to 100M/DUPLEX DONE\n");		
                else if (chip == PHYID_VENDOR_BROADCOM)
		    printf ("FORCE Broadcom PHY to 100M/DUPLEX DONE\n");		
		    
	}	
	
}

static void MAC1_reset (void)
{
    OUTL(MAC1_IO_BASE, SW_RST_bit, MACCR_REG);
    for (; (INL(MAC1_IO_BASE, MACCR_REG ) & SW_RST_bit) != 0; ) {udelay(1000);}
    OUTL(MAC1_IO_BASE, 0, IER_REG );
}

static int set_mac1_control_register (int Chip_ID)
{
	unsigned long	MAC_CR_Register = 0;
	int    PHY_Ready = 1;
	u16    PHY_Status, PHY_Speed, PHY_Duplex, Advertise, Link_Partner;

	MAC_CR_Register = SPEED_100M_MODE_bit | RX_ALLADR_bit | FULLDUP_bit | RXMAC_EN_bit | RXDMA_EN_bit | TXMAC_EN_bit | TXDMA_EN_bit | CRC_APD_bit;

	if ( (Chip_ID == PHYID_VENDOR_BROADCOM) || (Chip_ID == PHYID_VENDOR_RTL8201E)) {
		Advertise = phy_read_register (0x04, 0);
		Link_Partner = phy_read_register (0x05, 0);
		Advertise = (Advertise & PHY_SPEED_DUPLEX_MASK);
		Link_Partner = (Link_Partner & PHY_SPEED_DUPLEX_MASK);
		if ((Advertise & Link_Partner) & PHY_100M_DUPLEX) {
			MAC_CR_Register |= SPEED_100M_MODE_bit;
			MAC_CR_Register |= FULLDUP_bit;
		}
		else if ((Advertise & Link_Partner) & PHY_100M_HALF) {
			MAC_CR_Register |= SPEED_100M_MODE_bit;
			MAC_CR_Register &= ~FULLDUP_bit;
		}
		else if ((Advertise & Link_Partner) & PHY_10M_DUPLEX) {
			MAC_CR_Register &= ~SPEED_100M_MODE_bit;
			MAC_CR_Register |= FULLDUP_bit;
		}
		else if ((Advertise & Link_Partner) & PHY_10M_HALF) {
			MAC_CR_Register &= ~SPEED_100M_MODE_bit;
			MAC_CR_Register &= ~FULLDUP_bit;
		}
	}
	else if (Chip_ID == PHYID_VENDOR_MARVELL) {

		PHY_Ready = wait_link_resolve ();

		if (PHY_Ready == 1) {
			PHY_Status = phy_read_register (0x11, 0);
			PHY_Speed = (PHY_Status & PHY_SPEED_MASK) >> 14;
			PHY_Duplex = (PHY_Status & PHY_DUPLEX_MASK) >> 13;

			if (PHY_Speed == SPEED_1000M) {
				MAC_CR_Register |= GMAC_MODE_bit;
                	}
			else {
				MAC_CR_Register &= ~GMAC_MODE_bit;
				if (PHY_Speed == SPEED_10M) {
                        		MAC_CR_Register &= ~SPEED_100M_MODE_bit;
                    		}
                	}
                	if (PHY_Duplex == DUPLEX_HALF) {
				MAC_CR_Register &= ~FULLDUP_bit;
			}
		}
	}
	OUTL(MAC1_IO_BASE, MAC_CR_Register, MACCR_REG);

	return PHY_Ready;
}

static void ring_buffer_alloc (void)
{
	unsigned int i, j;

//Write data into TX buffer
	for (i = 0; i < NUM_TX; i++) {
		for (j = 0; j < TX_BUFF_SZ; j++) {
			tx_buffer[i][j] = i * 4 + j;
		}
	}
//Initialize RX buffer to 0
	for (i = 0; i < NUM_RX; i++) {
		for (j = 0; j < RX_BUFF_SZ; j++) {
			rx_buffer[i][j] = 0;
		}
	}
//Prepare descriptor
	for (i = 0; i < NUM_RX; i++) {
		rx_ring[i].status = cpu_to_le32(RXPKT_RDY + RX_BUFF_SZ);
		rx_ring[i].buf = ((u32) &rx_buffer[i]);
		rx_ring[i].reserved = 0;
	}
	for (i = 0; i < NUM_TX; i++) {
		tx_ring[i].status = 0;
		tx_ring[i].des1 = 0;
		tx_ring[i].buf = ((u32) &tx_buffer[i]);
		tx_ring[i].reserved = 0;
	}

	rx_ring[NUM_RX - 1].status |= cpu_to_le32(EDORR);
	tx_ring[NUM_TX - 1].status |= cpu_to_le32(EDOTR);

	OUTL(MAC1_IO_BASE, ((u32) &tx_ring), TXR_BADR_REG);
	OUTL(MAC1_IO_BASE, ((u32) &rx_ring), RXR_BADR_REG);
	
	tx_new = 0;
	rx_new = 0;
}

static int packet_test (void)
{
	unsigned int rx_status, length, i, Loop_Count = 0;

	tx_ring[tx_new].status |= cpu_to_le32(LTS | FTS | TX_BUFF_SZ);
	tx_ring[tx_new].status |= cpu_to_le32(TXDMA_OWN);
	OUTL(MAC1_IO_BASE, POLL_DEMAND, TXPD_REG);

//Compare result
	do {
		rx_status = rx_ring[rx_new].status;
		Loop_Count++;
	} while (!(rx_status & RXPKT_STATUS) && (Loop_Count < PHY_LOOP));
	if (rx_status & (RX_ERR | CRC_ERR | FTL | RUNT | RX_ODD_NB)) {
		/* There was an error.*/
		printf("RX error status = 0x%08X\n", rx_status);
		return PACKET_TEST_FAIL;
	} else {
		length = (rx_status & BYTE_COUNT_MASK);
		for (i = 0; i < RX_BUFF_SZ / 4; i++) {
			if (rx_buffer[rx_new][i] != tx_buffer[tx_new][i]) {
				printf ("ERROR at packet %d, address %x\n", rx_new, i);
				printf ("Gold = %8x, Real = %8x\n", tx_buffer[tx_new][i], rx_buffer[rx_new][i]);
				return PACKET_TEST_FAIL;
			}
		}
	}
    	tx_new = (tx_new + 1) % NUM_TX;
	rx_new = (rx_new + 1) % NUM_RX;

	return TEST_PASS;
}

static int packet_length_test (int packet_length)
{
	unsigned int rx_status, length, i, Loop_Count = 0;

	tx_ring[tx_new].status &= (~(BYTE_COUNT_MASK));
	tx_ring[tx_new].status |= cpu_to_le32(LTS | FTS | packet_length);
	tx_ring[tx_new].status |= cpu_to_le32(TXDMA_OWN);
	OUTL(MAC1_IO_BASE, POLL_DEMAND, TXPD_REG);

//Compare result
	do {
		rx_status = rx_ring[rx_new].status;
		Loop_Count++;
	} while (!(rx_status & RXPKT_STATUS) && (Loop_Count < PHY_LOOP));
	if (rx_status & (RX_ERR | CRC_ERR | FTL | RUNT | RX_ODD_NB)) {
		/* There was an error.*/
		printf("RX error status = 0x%08X\n", rx_status);
		return PACKET_LENGTH_TEST_FAIL;
	} else {
		length = (rx_status & BYTE_COUNT_MASK) - 4;
                if (length != packet_length) {
			printf ("Received Length ERROR. Gold = %d, Fail = %d\n",packet_length, length);
			printf ("rx_new = %d, tx_new = %d\n", rx_new, tx_new);
			return PACKET_LENGTH_TEST_FAIL;
                }
		for (i = 0; i < length; i++) {
		    if (rx_buffer[rx_new][i] != tx_buffer[tx_new][i]) {
		    	printf ("ERROR at packet %d, address %x\n", rx_new, i);
		    	printf ("Gold = %8x, Real = %8x\n", tx_buffer[tx_new][i], rx_buffer[rx_new][i]);
		    	return PACKET_LENGTH_TEST_FAIL;
		    }
		}
	}
	rx_ring[rx_new].status &= (~(RXPKT_STATUS));
    	tx_new = (tx_new + 1) % NUM_TX;
	rx_new = (rx_new + 1) % NUM_RX;

	return TEST_PASS;
}

static int MAC1_init (int id)
{
	int phy_status = 0;

	MAC1_reset ();
	phy_status = set_mac1_control_register (id);
       	ring_buffer_alloc ();
       	
       	return phy_status;
}

int do_mactest (void)
{
	unsigned int phy_id, i;
	int test_result = 0, phy_status = 0;

	SCU_MAC1_Enable();
	phy_id = ((phy_read_register (0x02, 0) << 16) + phy_read_register (0x03, 0)) & PHYID_VENDOR_MASK;
	if (phy_id == PHYID_VENDOR_MARVELL) {
        	printf ("PHY DETECTED ------> MARVELL\n");

        	set_phy_speed (phy_id, PHY_SPEED_1G, DUPLEX_FULL);
        	if ((phy_status = MAC1_init (phy_id)) != 0) {
	        	for (i = 0; i < NUM_TX; i++) {
				test_result |= packet_test ();
				if (test_result != 0)
					break;
        		}
        	}
		else if (phy_status == 0) {
			printf ("PHY FAIL: Please Check If you are using LOOP BACK Connector\n");
			test_result = 3;
			return	test_result;
		}
        	if ((phy_status = MAC1_init (phy_id)) != 0) {
        		for (i = 60; i < TX_BUFF_SZ; i++) {
				test_result |= packet_length_test (i);
				if (test_result != 0)
					break;
        		}
        	}
		else if (phy_status == 0) {
			printf ("PHY FAIL: Please Check If you are using LOOP BACK Connector\n");
			test_result = 3;
			return	test_result;
		}
        	set_phy_speed (phy_id, PHY_SPEED_100M, DUPLEX_FULL);
        	if ((phy_status = MAC1_init (phy_id)) != 0) {
        		for (i = 0; i < NUM_TX; i++) {
				test_result |= packet_test ();
				if (test_result != 0)
					break;
        		}
        	}
		else if (phy_status == 0) {
			printf ("PHY FAIL: Please Check If you are using LOOP BACK Connector\n");
			test_result = 3;
			return	test_result;
		}

        	if ((phy_status = MAC1_init (phy_id)) != 0) {
        		for (i = 60; i < TX_BUFF_SZ; i++) {
				test_result |= packet_length_test (i);
        			if (test_result != 0)
	        	    		break;
        		}
        	}
		else if (phy_status == 0) {
			printf ("PHY FAIL: Please Check If you are using LOOP BACK Connector\n");
			test_result = 3;
			return	test_result;
		}
        }
	else if ( (phy_id == PHYID_VENDOR_RTL8201E) || (phy_id == PHYID_VENDOR_BROADCOM) ){
		
        	if (phy_id == PHYID_VENDOR_RTL8201E)
        	    printf ("PHY DETECTED ------> RTL 8201E \n");
        	else if (phy_id == PHYID_VENDOR_BROADCOM)
        	    printf ("PHY DETECTED ------> Broadcom \n");
        	
        	set_phy_speed (phy_id, PHY_SPEED_100M, DUPLEX_FULL);
        	if ((phy_status = MAC1_init (phy_id)) != 0) {
        		for (i = 0; i < NUM_TX; i++) {
				test_result |= packet_test ();
				if (test_result != 0)
					break;
        		}
        	}
		else if (phy_status == 0) {
			printf ("PHY FAIL: Please Check If you are using LOOP BACK Connector\n");
			test_result = 3;
			return	test_result;
		}

        	if ((phy_status = MAC1_init (phy_id)) != 0) {
        		for (i = 60; i < TX_BUFF_SZ; i++) {
				test_result |= packet_length_test (i);
        			if (test_result != 0)
	        	    		break;
        		}
        	}
		else if (phy_status == 0) {
			printf ("PHY FAIL: Please Check If you are using LOOP BACK Connector\n");
			test_result = 3;
			return	test_result;
		}        	
        }	
          
	if ((phy_status == 0) && (test_result & PACKET_TEST_FAIL)) {
		printf ("Packet Test FAIL !\n");
	}
	else if ((phy_status == 0) && (test_result & PACKET_LENGTH_TEST_FAIL)) {
		printf ("Packet Length Test FAIL !\n");
	}

	return test_result;
	
}

#endif /* CONFIG_SLT */
