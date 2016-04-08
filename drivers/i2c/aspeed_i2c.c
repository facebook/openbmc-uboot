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

#include <common.h>
#include <asm/arch/aspeed_i2c.h>

#ifdef CONFIG_DRIVER_ASPEED_I2C

void i2c_init (int speed, int slaveadd)
{
	unsigned long SCURegister;
//I2C Reset
        SCURegister = inl (SCU_BASE + SCU_RESET_CONTROL);
        outl (SCURegister & ~(0x04), SCU_BASE + SCU_RESET_CONTROL);
//I2C Multi-Pin
        SCURegister = inl (SCU_BASE + SCU_MULTIFUNCTION_PIN_CTL5_REG);
        outl ((SCURegister | 0x30000), SCU_BASE + SCU_MULTIFUNCTION_PIN_CTL5_REG);
//Reset
	outl (0, I2C_FUNCTION_CONTROL_REGISTER);
//Set AC Timing, we use fix AC timing for eeprom in u-boot
	outl (AC_TIMING, I2C_AC_TIMING_REGISTER_1);
	outl (0, I2C_AC_TIMING_REGISTER_2);
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Enable Master Mode
	outl (MASTER_ENABLE, I2C_FUNCTION_CONTROL_REGISTER);
//Enable Interrupt, STOP Interrupt has bug in AST2000
	outl (0xAF, I2C_INTERRUPT_CONTROL_REGISTER);
//Set Slave address, should not use for eeprom
	outl (slaveadd, I2C_DEVICE_ADDRESS_REGISTER);
}

static int i2c_read_byte (u8 devaddr, u16 regoffset, u8 * value, int alen)
{
	int i2c_error = 0;
	u32 status, count = 0;

//Start and Send Device Address
	outl (devaddr, I2C_BYTE_BUFFER_REGISTER);
	outl (MASTER_START_COMMAND | MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Start and Send Device Address can't get ACK back\n");
                return i2c_error;
            }
        } while (status != TX_ACK);
        count = 0;
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Check if address length equals to 16bits
        if (alen != 1) {
//Send Device Register Offset (HIGH BYTE)
	    outl ((regoffset & 0xFF00) >> 8, I2C_BYTE_BUFFER_REGISTER);
	    outl (MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
            do {
                status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
                count++;
                if (count == LOOP_COUNT) {
                    i2c_error = 1;
                    printf ("Send Device Register Offset can't get ACK back\n");
                    return i2c_error;
                }
            } while (status != TX_ACK);
            count = 0;
//Clear Interrupt
	    outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	}
//Send Device Register Offset(LOW)
	outl (regoffset & 0xFF, I2C_BYTE_BUFFER_REGISTER);
	outl (MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Send Device Register Offset can't get ACK back\n");
                return i2c_error;
            }
        } while (status != TX_ACK);
        count = 0;
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Start, Send Device Address + 1 (Read Mode), Receive Data
	outl (devaddr + 1, I2C_BYTE_BUFFER_REGISTER);
	outl (MASTER_START_COMMAND | MASTER_TX_COMMAND | MASTER_RX_COMMAND | RX_COMMAND_LIST, I2C_COMMAND_REGISTER);
//Wait Rx Done
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & RX_DONE);
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Can't get RX_DONE back\n");
                return i2c_error;
            }
        } while (status != RX_DONE);
        count = 0;
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Enable Interrupt + Stop Interrupt
	outl (0xBF, I2C_INTERRUPT_CONTROL_REGISTER);
//Issue Stop Command
	outl (MASTER_STOP_COMMAND, I2C_COMMAND_REGISTER);
//Wait Stop
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & STOP_DONE);
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Can't get STOP back\n");
                return i2c_error;
            }
        } while (status != STOP_DONE);
//Disable Stop Interrupt
	outl (0xAF, I2C_INTERRUPT_CONTROL_REGISTER);
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Read Received Data
        *value = ((inl (I2C_BYTE_BUFFER_REGISTER) & 0xFF00) >> 8);

	return i2c_error;
}

static int i2c_write_byte (u8 devaddr, u16 regoffset, u8 value, int alen)
{
	int i2c_error = 0;
        u32 status, count = 0;

//Start and Send Device Address
	outl (devaddr, I2C_BYTE_BUFFER_REGISTER);
	outl (MASTER_START_COMMAND | MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
            count++;
            if (status == TX_NACK) {
//Clear Interrupt
   	        outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Re-send Start and Send Device Address while NACK return
	        outl (devaddr, I2C_BYTE_BUFFER_REGISTER);
        	outl (MASTER_START_COMMAND | MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
            }
            else {
            	if (count == LOOP_COUNT) {
            	    i2c_error = 1;
                    printf ("Start and Send Device Address can't get ACK back\n");
                    return i2c_error;
            	}
            }
        } while (status != TX_ACK);
        count = 0;
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Check if address length equals to 16bits
        if (alen != 1) {
//Send Device Register Offset (HIGH BYTE)
	    outl ((regoffset & 0xFF00) >> 8, I2C_BYTE_BUFFER_REGISTER);
	    outl (MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
            do {
                status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
                count++;
                if (count == LOOP_COUNT) {
                    i2c_error = 1;
                    printf ("Send Device Register Offset can't get ACK back\n");
                    return i2c_error;
                }
            } while (status != TX_ACK);
            count = 0;
//Clear Interrupt
	    outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	}
//Send Device Register Offset
	outl (regoffset & 0xFF, I2C_BYTE_BUFFER_REGISTER);
	outl (MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Send Device Register Offset can't get ACK back\n");
                return i2c_error;
            }
        } while (status != TX_ACK);
        count = 0;
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Send Device Register Value
	outl (value, I2C_BYTE_BUFFER_REGISTER);
	outl (MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
//Wait Tx ACK
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & (TX_ACK | TX_NACK));
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Send Device Register Value can't get ACK back\n");
                return i2c_error;
            }
        } while (status != TX_ACK);
        count = 0;
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
//Enable Interrupt + Stop Interrupt
	outl (0xBF, I2C_INTERRUPT_CONTROL_REGISTER);
//Issue Stop Command
	outl (MASTER_STOP_COMMAND, I2C_COMMAND_REGISTER);
//Wait Stop
        do {
            status = (inl (I2C_INTERRUPT_STATUS_REGISTER) & STOP_DONE);
            count++;
            if (count == LOOP_COUNT) {
                i2c_error = 1;
                printf ("Can't get STOP back\n");
                return i2c_error;
            }
        } while (status != STOP_DONE);
//Disable Stop Interrupt
	outl (0xAF, I2C_INTERRUPT_CONTROL_REGISTER);
//Clear Interrupt
	outl (ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);

	return i2c_error;
}

int i2c_probe (uchar chip)
{
//Suppose IP is always on chip
	int res = 0;
	
	return res;
}

int i2c_read (uchar device_addr, uint register_offset, int alen, uchar * buffer, int len)
{
	int i;

        if ((alen == 1) && ((register_offset + len) > 256)) {
        	printf ("Register index overflow\n");
        }
        
        for (i = 0; i < len; i++) {
	        if (i2c_read_byte (device_addr, register_offset + i, &buffer[i], alen)) {
		        printf ("I2C read: I/O error\n");
			i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write (uchar device_addr, uint register_offset, int alen, uchar * buffer, int len)
{
	int i;

        if ((alen == 1) && ((register_offset + len) > 256)) {
        	printf ("Register index overflow\n");
        }

        for (i = 0; i < len; i++) {
	        if (i2c_write_byte (device_addr, register_offset + i, buffer[i], alen)) {
		        printf ("I2C read: I/O error\n");
			i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
   	        }
	}

	return 0;
}

#endif /* CONFIG_DRIVER_ASPEED_I2C */
