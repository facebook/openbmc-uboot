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
#include <stdlib.h>
#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <inttypes.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#define OTP_PASSWD			0x349fe38a
#define RETRY				3
#define OTP_REGION_STRAP		BIT(0)
#define OTP_REGION_CONF			BIT(1)
#define OTP_REGION_DATA			BIT(2)

#define OTP_USAGE			-1
#define OTP_FAILURE			-2
#define OTP_SUCCESS			0

#define OTP_PROG_SKIP			1

#define OTP_REG_RESERVED		-1
#define OTP_REG_VALUE			-2
#define OTP_REG_VALID_BIT		-3

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

struct otpstrap_status {
	int value;
	int option_array[7];
	int remain_times;
	int writeable_option;
	int protected;
};

struct otpconf_parse {
	int dw_offset;
	int bit;
	int length;
	int value;
	int keep;
	char status[80];
};

struct otpstrap_info {
	uint32_t bit_offset;
	uint32_t length;
	int value;
	char information[80];
};

struct otpconf_info {
	uint32_t dw_offset;
	uint32_t bit_offset;
	uint32_t length;
	int value;
	char information[80];
};

void printProgress(int numerator, int denominator, char *format, ...)
{
	int val = numerator * 100 / denominator;
	int lpad = numerator * PBWIDTH / denominator;
	int rpad = PBWIDTH - lpad;
	char buffer[256];
	va_list aptr;

	va_start(aptr, format);
	vsprintf(buffer, format, aptr);
	va_end(aptr);

	printf("\r%3d%% [%.*s%*s] %s", val, lpad, PBSTR, rpad, "", buffer);
	if (numerator == denominator)
		printf("\n");
}

struct otpstrap_info a0_strap_info[] = {
	{
		0, 1, 0, "Disable secure boot"
	}, {
		0, 1, 1, "Enable secure boot"
	}, {
		1, 1, 0, "Disable boot from eMMC"
	}, {
		1, 1, 1, "Enable boot from eMMC"
	}, {
		2, 1, 0, "Disable Boot from debug SPI"
	}, {
		2, 1, 1, "Enable Boot from debug SPI"
	}, {
		3, 1, 0, "Enable ARM CM3"
	}, {
		3, 1, 1, "Disable ARM CM3"
	}, {
		4, 1, 0, "No VGA BISO ROM, VGA BIOS is merged in the system BIOS"
	}, {
		4, 1, 1, "Enable dedicated VGA BIOS ROM"
	}, {
		5, 1, 0, "MAC 1 : RMII/NCSI"
	}, {
		5, 1, 1, "MAC 1 : RGMII"
	}, {
		6, 1, 0, "MAC 2 : RMII/NCSI"
	}, {
		6, 1, 1, "MAC 2 : RGMII"
	}, {
		7, 3, 0, "CPU Frequency : 1GHz"
	}, {
		7, 3, 1, "CPU Frequency : 800MHz"
	}, {
		7, 3, 2, "CPU Frequency : 1.2GHz"
	}, {
		7, 3, 3, "CPU Frequency : 1.4GHz"
	}, {
		10, 2, 0, "HCLK ratio AXI:AHB = 2:1"
	}, {
		10, 2, 1, "HCLK ratio AXI:AHB = 2:1"
	}, {
		10, 2, 2, "HCLK ratio AXI:AHB = 3:1"
	}, {
		10, 2, 3, "HCLK ratio AXI:AHB = 4:1"
	}, {
		12, 2, 0, "VGA memory size : 8MB"
	}, {
		12, 2, 1, "VGA memory size : 16MB"
	}, {
		12, 2, 2, "VGA memory size : 32MB"
	}, {
		12, 2, 3, "VGA memory size : 64MB"
	}, {
		14, 3, OTP_REG_RESERVED, ""
	}, {
		17, 1, 0, "VGA class code : Class Code for video device"
	}, {
		17, 1, 1, "VGA class code : Class Code for VGA device"
	}, {
		18, 1, 0, "Enable debug interfaces 0"
	}, {
		18, 1, 1, "Disable debug interfaces 0"
	}, {
		19, 1, 0, "Boot from emmc mode : High eMMC speed"
	}, {
		19, 1, 1, "Boot from emmc mode : Normal eMMC speed"
	}, {
		20, 1, 0, "Enable Pcie EHCI device"
	}, {
		20, 1, 1, "Disable Pcie EHCI device"
	}, {
		21, 1, 0, "Enable VGA XDMA function"
	}, {
		21, 1, 1, "Disable VGA XDMA function"
	}, {
		22, 1, 0, "Normal BMC mode"
	}, {
		22, 1, 1, "Disable dedicated BMC functions for non-BMC application"
	}, {
		23, 1, 0, "SSPRST# pin is for secondary processor dedicated reset pin"
	}, {
		23, 1, 1, "SSPRST# pin is for PCIE root complex dedicated reset pin"
	}, {
		24, 1, 0, "DRAM types : DDR4"
	}, {
		24, 1, 1, "DRAM types : DDR3"
	}, {
		25, 5, OTP_REG_RESERVED, ""
	}, {
		30, 2, OTP_REG_RESERVED, ""
	}, {
		32, 1, 0, "MAC 3 : RMII/NCSI"
	}, {
		32, 1, 1, "MAC 3 : RGMII"
	}, {
		33, 1, 0, "MAC 4 : RMII/NCSI"
	}, {
		33, 1, 1, "MAC 4 : RGMII"
	}, {
		34, 1, 0, "SuperIO configuration address : 0x2E"
	}, {
		34, 1, 1, "SuperIO configuration address : 0x4E"
	}, {
		35, 1, 0, "Enable LPC to decode SuperIO"
	}, {
		35, 1, 1, "Disable LPC to decode SuperIO"
	}, {
		36, 1, 0, "Enable debug interfaces 1"
	}, {
		36, 1, 1, "Disable debug interfaces 1"
	}, {
		37, 1, 0, "Disable ACPI function"
	}, {
		37, 1, 1, "Enable ACPI function"
	}, {
		38, 1, 0, "Enable eSPI mode"
	}, {
		38, 1, 1, "Enable LPC mode"
	}, {
		39, 1, 0, "Enable SAFS mode"
	}, {
		39, 1, 1, "Enable SAFS mode"
	}, {
		40, 2, OTP_REG_RESERVED, ""
	}, {
		42, 1, 0, "Disable boot SPI 3B/4B address mode auto detection"
	}, {
		42, 1, 1, "Enable boot SPI 3B/4B address mode auto detection"
	}, {
		43, 1, 0, "Disable boot SPI ABR"
	}, {
		43, 1, 1, "Enable boot SPI ABR"
	}, {
		44, 1, 0, "Boot SPI ABR mode : dual SPI flash"
	}, {
		44, 1, 1, "Boot SPI ABR mode : single SPI flash"
	}, {
		45, 3, 0, "Boot SPI flash size : no define size"
	}, {
		45, 3, 1, "Boot SPI flash size : 2MB"
	}, {
		45, 3, 2, "Boot SPI flash size : 4MB"
	}, {
		45, 3, 3, "Boot SPI flash size : 8MB"
	}, {
		45, 3, 4, "Boot SPI flash size : 16MB"
	}, {
		45, 3, 5, "Boot SPI flash size : 32MB"
	}, {
		45, 3, 6, "Boot SPI flash size : 64MB"
	}, {
		45, 3, 7, "Boot SPI flash size : 128MB"
	}, {
		48, 1, 0, "Disable host SPI ABR"
	}, {
		48, 1, 1, "Enable host SPI ABR"
	}, {
		49, 1, 0, "Disable host SPI ABR mode select pin"
	}, {
		49, 1, 1, "Enable host SPI ABR mode select pin"
	}, {
		50, 1, 0, "Host SPI ABR mode : dual SPI flash"
	}, {
		50, 1, 1, "Host SPI ABR mode : single SPI flash"
	}, {
		51, 3, 0, "Host SPI flash size : no define size"
	}, {
		51, 3, 1, "Host SPI flash size : 2MB"
	}, {
		51, 3, 2, "Host SPI flash size : 4MB"
	}, {
		51, 3, 3, "Host SPI flash size : 8MB"
	}, {
		51, 3, 4, "Host SPI flash size : 16MB"
	}, {
		51, 3, 5, "Host SPI flash size : 32MB"
	}, {
		51, 3, 6, "Host SPI flash size : 64MB"
	}, {
		51, 3, 7, "Host SPI flash size : 128MB"
	}, {
		54, 1, 0, "Disable boot SPI auxiliary control pins"
	}, {
		54, 1, 1, "Enable boot SPI auxiliary control pins"
	}, {
		55, 2, 0, "Boot SPI CRTM size : disable CRTM"
	}, {
		55, 2, 1, "Boot SPI CRTM size : 256KB"
	}, {
		55, 2, 2, "Boot SPI CRTM size : 512KB"
	}, {
		55, 2, 3, "Boot SPI CRTM size : 1MB"
	}, {
		57, 2, 0, "Host SPI CRTM size : disable CRTM"
	}, {
		57, 2, 1, "Host SPI CRTM size : 256KB"
	}, {
		57, 2, 2, "Host SPI CRTM size : 512KB"
	}, {
		57, 2, 3, "Host SPI CRTM size : 1MB"
	}, {
		59, 1, 0, "Disable host SPI auxiliary control pins"
	}, {
		59, 1, 1, "Enable host SPI auxiliary control pins"
	}, {
		60, 1, 0, "Disable GPIO pass through"
	}, {
		60, 1, 1, "Enable GPIO pass through"
	}, {
		61, 1, 0, "Enable low security secure boot key"
	}, {
		61, 1, 1, "Disable low security secure boot key"
	}, {
		62, 1, 0, "Disable dedicate GPIO strap pins"
	}, {
		62, 1, 1, "Enable dedicate GPIO strap pins"
	}, {
		63, 1, OTP_REG_RESERVED, ""
	}
};
struct otpconf_info a0_conf_info[] = {
	{
		0, 0, 1, 0, "Enable Secure Region programming"
	}, {
		0, 0, 1, 1, "Disable Secure Region programming"
	}, {
		0, 1, 1, 0, "Disable Secure Boot"
	}, {
		0, 1, 1, 1, "Enable Secure Boot"
	}, {
		0, 2, 1, 0, "Initialization programming not done"
	}, {
		0, 2, 1, 1, "Initialization programming done"
	}, {
		0, 3, 1, 0, "User region ECC disable"
	}, {
		0, 3, 1, 1, "User region ECC enable"
	}, {
		0, 4, 1, 0, "Secure Region ECC disable"
	}, {
		0, 4, 1, 1, "Secure Region ECC enable"
	}, {
		0, 5, 1, 0, "Enable low security key"
	}, {
		0, 5, 1, 1, "Disable low security key"
	}, {
		0, 6, 1, 0, "Do not ignore Secure Boot hardware strap"
	}, {
		0, 6, 1, 1, "Ignore Secure Boot hardware strap"
	}, {
		0, 7, 1, 0, "Secure Boot Mode: 1"
	}, {
		0, 7, 1, 1, "Secure Boot Mode: 2"
	}, {
		0, 8, 2, 0, "Single cell mode (recommended)"
	}, {
		0, 8, 2, 1, "Differnetial mode"
	}, {
		0, 8, 2, 2, "Differential-redundant mode"
	}, {
		0, 10, 2, 0, "RSA mode : RSA1024"
	}, {
		0, 10, 2, 1, "RSA mode : RSA2048"
	}, {
		0, 10, 2, 2, "RSA mode : RSA3072"
	}, {
		0, 10, 2, 3, "RSA mode : RSA4096"
	}, {
		0, 12, 2, 0, "SHA mode : SHA224"
	}, {
		0, 12, 2, 1, "SHA mode : SHA256"
	}, {
		0, 12, 2, 2, "SHA mode : SHA384"
	}, {
		0, 12, 2, 3, "SHA mode : SHA512"
	}, {
		0, 14, 2, OTP_REG_RESERVED, ""
	}, {
		0, 16, 6, OTP_REG_VALUE, "Secure Region size (DW): 0x%x"
	}, {
		0, 22, 1, 0, "Secure Region : Writable"
	}, {
		0, 22, 1, 1, "Secure Region : Write Protect"
	}, {
		0, 23, 1, 0, "User Region : Writable"
	}, {
		0, 23, 1, 1, "User Region : Write Protect"
	}, {
		0, 24, 1, 0, "Configure Region : Writable"
	}, {
		0, 24, 1, 1, "Configure Region : Write Protect"
	}, {
		0, 25, 1, 0, "OTP strap Region : Writable"
	}, {
		0, 25, 1, 1, "OTP strap Region : Write Protect"
	}, {
		0, 26, 1, 0, "Disable Copy Boot Image to Internal SRAM"
	}, {
		0, 26, 1, 1, "Copy Boot Image to Internal SRAM"
	}, {
		0, 27, 1, 0, "Disable image encryption"
	}, {
		0, 27, 1, 1, "Enable image encryption"
	}, {
		0, 28, 1, OTP_REG_RESERVED, ""
	}, {
		0, 29, 1, 0, "OTP key retire Region : Writable"
	}, {
		0, 29, 1, 1, "OTP key retire Region : Write Protect"
	}, {
		0, 30, 1, 0, "SIPROM RED_EN redundancy repair disable"
	}, {
		0, 30, 1, 1, "SIPROM RED_EN redundancy repair enable"
	}, {
		0, 31, 1, 0, "SIPROM Mlock memory lock disable"
	}, {
		0, 31, 1, 1, "SIPROM Mlock memory lock enable"
	}, {
		2, 0, 16, OTP_REG_VALUE, "Vender ID : 0x%x"
	}, {
		2, 16, 16, OTP_REG_VALUE, "Key Revision : 0x%x"
	}, {
		3, 0, 16, OTP_REG_VALUE, "Secure boot header offset : 0x%x"
	}, {
		4, 0, 8, OTP_REG_VALID_BIT, "Keys valid  : %d"
	}, {
		4, 16, 8, OTP_REG_VALID_BIT, "Keys retire  : %d"
	}, {
		5, 0, 32, OTP_REG_VALUE, "User define data, random number low : 0x%x"
	}, {
		6, 0, 32, OTP_REG_VALUE, "User define data, random number high : 0x%x"
	}, {
		7, 0, 1, 0, "Force enable PCI bus to AHB bus bridge"
	}, {
		7, 0, 1, 1, "Force disable PCI bus to AHB bus bridge"
	}, {
		7, 1, 1, 0, "Force enable UART5 debug port function"
	}, {
		7, 1, 1, 1, "Force disable UART5 debug port function"
	}, {
		7, 2, 1, 0, "Force enable XDMA function"
	}, {
		7, 2, 1, 1, "Force disable XDMA function"
	}, {
		7, 3, 1, 0, "Force enable APB to PCIE device bridge"
	}, {
		7, 3, 1, 1, "Force disable APB to PCIE device bridge"
	}, {
		7, 4, 1, 0, "Force enable APB to PCIE bridge config access"
	}, {
		7, 4, 1, 1, "Force disable APB to PCIE bridge config access"
	}, {
		7, 5, 1, 0, "Force enable PCIE bus trace buffer"
	}, {
		7, 5, 1, 1, "Force disable PCIE bus trace buffer"
	}, {
		7, 6, 1, 0, "Force enable the capability for PCIE device port as a Root Complex"
	}, {
		7, 6, 1, 1, "Force disable the capability for PCIE device port as a Root Complex"
	}, {
		7, 16, 1, 0, "Force enable ESPI bus to AHB bus bridge"
	}, {
		7, 16, 1, 1, "Force disable ESPI bus to AHB bus bridge"
	}, {
		7, 17, 1, 0, "Force enable LPC bus to AHB bus bridge1"
	}, {
		7, 17, 1, 1, "Force disable LPC bus to AHB bus bridge1"
	}, {
		7, 18, 1, 0, "Force enable LPC bus to AHB bus bridge2"
	}, {
		7, 18, 1, 1, "Force disable LPC bus to AHB bus bridge2"
	}, {
		7, 19, 1, 0, "Force enable UART1 debug port function"
	}, {
		7, 19, 1, 1, "Force disable UART1 debug port function"
	}, {
		7, 31, 1, 0, "Disable chip security setting"
	}, {
		7, 31, 1, 1, "Enable chip security setting"
	}, {
		8, 0, 32, OTP_REG_VALUE, "Redundancy Repair : 0x%x"
	}, {
		10, 0, 32, OTP_REG_VALUE, "Manifest ID low : 0x%x"
	}, {
		11, 0, 32, OTP_REG_VALUE, "Manifest ID high : 0x%x"
	}
};
static void otp_read_data(uint32_t offset, uint32_t *data)
{
	writel(offset, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	data[0] = readl(0x1e6f2020);
	data[1] = readl(0x1e6f2024);
}

static void otp_read_config(uint32_t offset, uint32_t *data)
{
	int config_offset;

	config_offset = 0x800;
	config_offset |= (offset / 8) * 0x200;
	config_offset |= (offset % 8) * 0x2;

	writel(config_offset, 0x1e6f2010);  //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	data[0] = readl(0x1e6f2020);
}

static int otp_print_config(uint32_t offset, int dw_count)
{
	int i;
	uint32_t ret[1];

	if (offset + dw_count > 32)
		return OTP_USAGE;
	for (i = offset; i < offset + dw_count; i ++) {
		otp_read_config(i, ret);
		printf("OTPCFG%X: %08X\n", i, ret[0]);
	}
	printf("\n");
	return OTP_SUCCESS;
}

static int otp_print_data(uint32_t offset, int dw_count)
{
	int i;
	uint32_t ret[2];

	if (offset + dw_count > 2048 || offset % 4 != 0)
		return OTP_USAGE;
	for (i = offset; i < offset + dw_count; i += 2) {
		otp_read_data(i, ret);
		if (i % 4 == 0)
			printf("%03X: %08X %08X ", i * 4, ret[0], ret[1]);
		else
			printf("%08X %08X\n", ret[0], ret[1]);

	}
	printf("\n");
	return OTP_SUCCESS;
}

static int otp_compare(uint32_t otp_addr, uint32_t addr)
{
	uint32_t ret;
	uint32_t *buf;

	buf = map_physmem(addr, 16, MAP_WRBACK);
	printf("%08X\n", buf[0]);
	printf("%08X\n", buf[1]);
	printf("%08X\n", buf[2]);
	printf("%08X\n", buf[3]);
	writel(otp_addr, 0x1e6f2010); //Compare address
	writel(buf[0], 0x1e6f2020); //Compare data 1
	writel(buf[1], 0x1e6f2024); //Compare data 2
	writel(buf[2], 0x1e6f2028); //Compare data 3
	writel(buf[3], 0x1e6f202c); //Compare data 4
	writel(0x23b1e363, 0x1e6f2004); //Compare command
	udelay(10);
	ret = readl(0x1e6f2014); //Compare command
	if (ret & 0x1)
		return 0;
	else
		return -1;
}

static void otp_write(uint32_t otp_addr, uint32_t data)
{
	writel(otp_addr, 0x1e6f2010); //write address
	writel(data, 0x1e6f2020); //write data
	writel(0x23b1e362, 0x1e6f2004); //write command
	udelay(100);
}

static void otp_prog(uint32_t otp_addr, uint32_t prog_bit)
{
	writel(otp_addr, 0x1e6f2010); //write address
	writel(prog_bit, 0x1e6f2020); //write data
	writel(0x23b1e364, 0x1e6f2004); //write command
	udelay(85);
}

static int verify_bit(uint32_t otp_addr, int bit_offset, int value)
{
	int ret;

	writel(otp_addr, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	ret = readl(0x1e6f2020);
	// printf("verify_bit = %x\n", ret);
	if (((ret >> bit_offset) & 1) == value)
		return 0;
	else
		return -1;
}

static uint32_t verify_dw(uint32_t otp_addr, uint32_t *value, uint32_t *keep, uint32_t *compare, int size)
{
	uint32_t ret[2];

	otp_addr &= ~(1 << 15);

	if (otp_addr % 2 == 0)
		writel(otp_addr, 0x1e6f2010); //Read address
	else
		writel(otp_addr - 1, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	ret[0] = readl(0x1e6f2020);
	ret[1] = readl(0x1e6f2024);
	if (size == 1) {
		if (otp_addr % 2 == 0) {
			// printf("check %x : %x = %x\n", otp_addr, ret[0], value[0]);
			if ((value[0] & ~keep[0]) == (ret[0] & ~keep[0])) {
				compare[0] = 0;
				return 0;
			} else {
				compare[0] = value[0] ^ ret[0];
				return -1;
			}

		} else {
			// printf("check %x : %x = %x\n", otp_addr, ret[1], value[0]);
			if ((value[0] & ~keep[0]) == (ret[1] & ~keep[0])) {
				compare[0] = ~0;
				return 0;
			} else {
				compare[0] = ~(value[0] ^ ret[1]);
				return -1;
			}
		}
	} else if (size == 2) {
		// otp_addr should be even
		if ((value[0] & ~keep[0]) == (ret[0] & ~keep[0]) && (value[1] & ~keep[1]) == (ret[1] & ~keep[1])) {
			// printf("check[0] %x : %x = %x\n", otp_addr, ret[0], value[0]);
			// printf("check[1] %x : %x = %x\n", otp_addr, ret[1], value[1]);
			compare[0] = 0;
			compare[1] = ~0;
			return 0;
		} else {
			// printf("check[0] %x : %x = %x\n", otp_addr, ret[0], value[0]);
			// printf("check[1] %x : %x = %x\n", otp_addr, ret[1], value[1]);
			compare[0] = value[0] ^ ret[0];
			compare[1] = ~(value[1] ^ ret[1]);
			return -1;
		}
	} else {
		return -1;
	}
}

static void otp_soak(int soak)
{
	if (soak) {
		otp_write(0x3000, 0x4021); // Write MRA
		otp_write(0x5000, 0x1027); // Write MRB
		otp_write(0x1000, 0x4820); // Write MR
		writel(0x041930d4, 0x1e602008); //soak program
	} else {
		otp_write(0x3000, 0x4061); // Write MRA
		otp_write(0x5000, 0x302f); // Write MRB
		otp_write(0x1000, 0x4020); // Write MR
		writel(0x04190760, 0x1e602008); //normal program
	}
}

static void otp_prog_dw(uint32_t value, uint32_t keep, uint32_t prog_address)
{
	int j, bit_value, prog_bit;

	for (j = 0; j < 32; j++) {
		if ((keep >> j) & 0x1)
			continue;
		bit_value = (value >> j) & 0x1;
		if (prog_address % 2 == 0) {
			if (bit_value)
				prog_bit = ~(0x1 << j);
			else
				continue;
		} else {
			prog_address |= 1 << 15;
			if (bit_value)
				continue;
			else
				prog_bit = 0x1 << j;
		}
		otp_prog(prog_address, prog_bit);
	}
}


static void otp_strp_status(struct otpstrap_status *otpstrap)
{
	uint32_t OTPSTRAP_RAW[2];
	int i, j;

	for (j = 0; j < 64; j++) {
		otpstrap[j].value = 0;
		otpstrap[j].remain_times = 7;
		otpstrap[j].writeable_option = -1;
		otpstrap[j].protected = 0;
	}

	for (i = 16; i < 30; i += 2) {
		int option = (i - 16) / 2;
		otp_read_config(i, &OTPSTRAP_RAW[0]);
		otp_read_config(i + 1, &OTPSTRAP_RAW[1]);
		for (j = 0; j < 32; j++) {
			char bit_value = ((OTPSTRAP_RAW[0] >> j) & 0x1);
			if ((bit_value == 0) && (otpstrap[j].writeable_option == -1)) {
				otpstrap[j].writeable_option = option;
			}
			if (bit_value == 1)
				otpstrap[j].remain_times --;
			otpstrap[j].value ^= bit_value;
			otpstrap[j].option_array[option] = bit_value;
		}
		for (j = 32; j < 64; j++) {
			char bit_value = ((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1);
			if ((bit_value == 0) && (otpstrap[j].writeable_option == -1)) {
				otpstrap[j].writeable_option = option;
			}
			if (bit_value == 1)
				otpstrap[j].remain_times --;
			otpstrap[j].value ^= bit_value;
			otpstrap[j].option_array[option] = bit_value;
		}
	}
	otp_read_config(30, &OTPSTRAP_RAW[0]);
	otp_read_config(31, &OTPSTRAP_RAW[1]);
	for (j = 0; j < 32; j++) {
		if (((OTPSTRAP_RAW[0] >> j) & 0x1) == 1)
			otpstrap[j].protected = 1;
	}
	for (j = 32; j < 64; j++) {
		if (((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1) == 1)
			otpstrap[j].protected = 1;
	}
}

static int otp_print_conf_image(uint32_t *OTPCFG)
{
	uint32_t *OTPCFG_KEEP = &OTPCFG[12];
	uint32_t mask;
	uint32_t dw_offset;
	uint32_t bit_offset;
	uint32_t otp_value;
	uint32_t otp_keep;
	int fail = 0;
	int valid_bit = 0;
	int i;

	printf("DW    BIT        Value       Description\n");
	printf("__________________________________________________________________________\n");
	for (i = 0; i < ARRAY_SIZE(a0_conf_info); i++) {
		dw_offset = a0_conf_info[i].dw_offset;
		bit_offset = a0_conf_info[i].bit_offset;
		mask = BIT(a0_conf_info[i].length) - 1;
		otp_value = (OTPCFG[dw_offset] >> bit_offset) & mask;
		otp_keep = (OTPCFG_KEEP[dw_offset] >> bit_offset) & mask;

		if (otp_keep == mask) {
			continue;
		} else if (otp_keep != 0) {
			fail = 1;
		}

		if ((otp_value != a0_conf_info[i].value) &&
		    a0_conf_info[i].value != OTP_REG_RESERVED &&
		    a0_conf_info[i].value != OTP_REG_VALUE &&
		    a0_conf_info[i].value != OTP_REG_VALID_BIT)
			continue;
		printf("0x%-4X", dw_offset);

		if (a0_conf_info[i].length == 1) {
			printf("0x%-9X", a0_conf_info[i].bit_offset);
		} else {
			printf("0x%-2X:0x%-4X",
			       a0_conf_info[i].bit_offset + a0_conf_info[i].length - 1,
			       a0_conf_info[i].bit_offset);
		}
		printf("0x%-10x", otp_value);

		if (fail) {
			printf("Keep mask error\n");
		} else {
			if (a0_conf_info[i].value == OTP_REG_RESERVED) {
				printf("Reserved\n");
			} else if (a0_conf_info[i].value == OTP_REG_VALUE) {
				printf(a0_conf_info[i].information, otp_value);
				printf("\n");
			} else if (a0_conf_info[i].value == OTP_REG_VALID_BIT) {
				if (otp_value != 0) {
					for (i = 0; i < 7; i++) {
						if (otp_value == (1 << i)) {
							valid_bit = i + 1;
						}
					}
				} else {
					valid_bit = 0;
				}
				printf(a0_conf_info[i].information, valid_bit);
				printf("\n");
			} else {
				printf("%s\n", a0_conf_info[i].information);
			}
		}
	}

	if (fail)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static int otp_print_conf_info(int input_offset)
{
	uint32_t OTPCFG[12];
	uint32_t mask;
	uint32_t dw_offset;
	uint32_t bit_offset;
	uint32_t otp_value;
	int valid_bit = 0;
	int i;

	for (i = 0; i < 12; i++)
		otp_read_config(i, &OTPCFG[i]);


	printf("DW    BIT        Value       Description\n");
	printf("__________________________________________________________________________\n");
	for (i = 0; i < ARRAY_SIZE(a0_conf_info); i++) {
		if (input_offset != -1 && input_offset != a0_conf_info[i].dw_offset)
			continue;
		dw_offset = a0_conf_info[i].dw_offset;
		bit_offset = a0_conf_info[i].bit_offset;
		mask = BIT(a0_conf_info[i].length) - 1;
		otp_value = (OTPCFG[dw_offset] >> bit_offset) & mask;

		if ((otp_value != a0_conf_info[i].value) &&
		    a0_conf_info[i].value != OTP_REG_RESERVED &&
		    a0_conf_info[i].value != OTP_REG_VALUE &&
		    a0_conf_info[i].value != OTP_REG_VALID_BIT)
			continue;
		printf("0x%-4X", dw_offset);

		if (a0_conf_info[i].length == 1) {
			printf("0x%-9X", a0_conf_info[i].bit_offset);
		} else {
			printf("0x%-2X:0x%-4X",
			       a0_conf_info[i].bit_offset + a0_conf_info[i].length - 1,
			       a0_conf_info[i].bit_offset);
		}
		printf("0x%-10x", otp_value);

		if (a0_conf_info[i].value == OTP_REG_RESERVED) {
			printf("Reserved\n");
		} else if (a0_conf_info[i].value == OTP_REG_VALUE) {
			printf(a0_conf_info[i].information, otp_value);
			printf("\n");
		} else if (a0_conf_info[i].value == OTP_REG_VALID_BIT) {
			if (otp_value != 0) {
				for (i = 0; i < 7; i++) {
					if (otp_value == (1 << i)) {
						valid_bit = i + 1;
					}
				}
			} else {
				valid_bit = 0;
			}
			printf(a0_conf_info[i].information, valid_bit);
			printf("\n");
		} else {
			printf("%s\n", a0_conf_info[i].information);
		}
	}
	return OTP_SUCCESS;
}

static int otp_print_strap_image(uint32_t *OTPSTRAP)
{
	uint32_t *OTPSTRAP_PRO = &OTPSTRAP[4];
	uint32_t *OTPSTRAP_KEEP = &OTPSTRAP[2];
	int i;
	int fail = 0;
	uint32_t bit_offset;
	uint32_t dw_offset;
	uint32_t mask;
	uint32_t otp_value;
	uint32_t otp_protect;
	uint32_t otp_keep;

	printf("BIT(hex)   Value       Protect     Description\n");
	printf("__________________________________________________________________________________________\n");

	for (i = 0; i < ARRAY_SIZE(a0_strap_info); i++) {
		if (a0_strap_info[i].bit_offset > 32) {
			dw_offset = 1;
			bit_offset = a0_strap_info[i].bit_offset - 32;
		} else {
			dw_offset = 0;
			bit_offset = a0_strap_info[i].bit_offset;
		}

		mask = BIT(a0_strap_info[i].length) - 1;
		otp_value = (OTPSTRAP[dw_offset] >> bit_offset) & mask;
		otp_protect = (OTPSTRAP_PRO[dw_offset] >> bit_offset) & mask;
		otp_keep = (OTPSTRAP_KEEP[dw_offset] >> bit_offset) & mask;

		if (otp_keep == mask) {
			continue;
		} else if (otp_keep != 0) {
			fail = 1;
		}

		if ((otp_value != a0_strap_info[i].value) &&
		    a0_strap_info[i].value != OTP_REG_RESERVED)
			continue;

		if (a0_strap_info[i].length == 1) {
			printf("0x%-9X", a0_strap_info[i].bit_offset);
		} else {
			printf("0x%-2X:0x%-4X",
			       a0_strap_info[i].bit_offset + a0_strap_info[i].length - 1,
			       a0_strap_info[i].bit_offset);
		}
		printf("0x%-10x", otp_value);
		printf("0x%-10x", otp_protect);

		if (fail) {
			printf("Keep mask error\n");
		} else {
			if (a0_strap_info[i].value != OTP_REG_RESERVED)
				printf("%s\n", a0_strap_info[i].information);
			else
				printf("Reserved\n");
		}
	}

	if (fail)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static int otp_print_strap_info(int view)
{
	struct otpstrap_status strap_status[64];
	int i, j;
	int fail = 0;
	uint32_t bit_offset;
	uint32_t length;
	uint32_t otp_value;
	uint32_t otp_protect;

	otp_strp_status(strap_status);

	if (view) {
		// printf("BIT(hex) Value  Option         Protect   Description\n");
		// printf("                0 1 2 3 4 5 6\n");
		printf("BIT(hex) Value  Remains  Protect   Description\n");
		printf("___________________________________________________________________________________________________\n");
	} else {
		printf("BIT(hex)   Value       Description\n");
		printf("________________________________________________________________________________\n");
	}
	for (i = 0; i < ARRAY_SIZE(a0_strap_info); i++) {
		otp_value = 0;
		bit_offset = a0_strap_info[i].bit_offset;
		length = a0_strap_info[i].length;
		for (j = 0; j < length; j++) {
			otp_value |= strap_status[bit_offset + j].value << j;
			otp_protect |= strap_status[bit_offset + j].protected << j;
		}
		if ((otp_value != a0_strap_info[i].value) &&
		    a0_strap_info[i].value != OTP_REG_RESERVED)
			continue;
		if (view) {
			for (j = 0; j < length; j++) {
				printf("0x%-7X", a0_strap_info[i].bit_offset + j);
				printf("0x%-5X", strap_status[bit_offset + j].value);
				printf("%-9d", strap_status[bit_offset + j].remain_times);
				printf("0x%-7X", strap_status[bit_offset].protected);
				if (a0_strap_info[i].value == OTP_REG_RESERVED) {
					printf(" Reserved\n");
					continue;
				}
				if (length == 1) {
					printf(" %s\n", a0_strap_info[i].information);
					continue;
				}

				if (j == 0)
					printf("/%s\n", a0_strap_info[i].information);
				else if (j == length - 1)
					printf("\\ \"\n");
				else
					printf("| \"\n");
			}
		} else {
			if (length == 1) {
				printf("0x%-9X", a0_strap_info[i].bit_offset);
			} else {
				printf("0x%-2X:0x%-4X",
				       bit_offset + length - 1, bit_offset);
			}

			printf("0x%-10X", otp_value);

			if (a0_strap_info[i].value != OTP_REG_RESERVED)
				printf("%s\n", a0_strap_info[i].information);
			else
				printf("Reserved\n");
		}
	}

	if (fail)
		return OTP_FAILURE;

	return OTP_SUCCESS;
}

static void buf_print(char *buf, int len)
{
	int i;
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	for (i = 0; i < len; i++) {
		if (i % 16 == 0) {
			printf("%04X: ", i);
		}
		printf("%02X ", buf[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
}

static int otp_print_data_info(uint32_t *buf)
{
	int key_id, key_offset, last, key_type, key_length, exp_length;
	char *byte_buf;
	int i = 0, len = 0;
	byte_buf = (char *)buf;
	while (1) {
		key_id = buf[i] & 0x7;
		key_offset = buf[i] & 0x1ff8;
		last = (buf[i] >> 13) & 1;
		key_type = (buf[i] >> 14) & 0xf;
		key_length = (buf[i] >> 18) & 0x3;
		exp_length = (buf[i] >> 20) & 0xfff;
		printf("\nKey[%d]:\n", i);
		printf("Key Type: ");
		switch (key_type) {
		case 0:
			printf("AES-256 as OEM platform key for image encryption/decryption\n");
			break;
		case 1:
			printf("AES-256 as secret vault key\n");
			break;
		case 4:
			printf("HMAC as encrypted OEM HMAC keys in Mode 1\n");
			break;
		case 8:
			printf("RSA-public as OEM DSS public keys in Mode 2\n");
			break;
		case 9:
			printf("RSA-public as SOC public key\n");
			break;
		case 10:
			printf("RSA-public as AES key decryption key\n");
			break;
		case 13:
			printf("RSA-private as SOC private key\n");
			break;
		case 14:
			printf("RSA-private as AES key decryption key\n");
			break;
		default:
			printf("key_type error: %x\n", key_type);
			return -1;
		}
		if (key_type == 4) {
			printf("HMAC SHA Type: ");
			switch (key_length) {
			case 0:
				printf("HMAC(SHA224)\n");
				break;
			case 1:
				printf("HMAC(SHA256)\n");
				break;
			case 2:
				printf("HMAC(SHA384)\n");
				break;
			case 3:
				printf("HMAC(SHA512)\n");
				break;
			}
		} else if (key_type != 0 && key_type != 1) {
			printf("RSA SHA Type: ");
			switch (key_length) {
			case 0:
				printf("RSA1024\n");
				len = 0x100;
				break;
			case 1:
				printf("RSA2048\n");
				len = 0x200;
				break;
			case 2:
				printf("RSA3072\n");
				len = 0x300;
				break;
			case 3:
				printf("RSA4096\n");
				len = 0x400;
				break;
			}
			printf("RSA exponent bit length: %d\n", exp_length);
		}
		if (key_type == 4 || key_type == 8)
			printf("Key Number ID: %d\n", key_id);
		printf("Key Value:\n");
		if (key_type == 4) {
			buf_print(&byte_buf[key_offset], 0x40);
		} else if (key_type == 0 || key_type == 1) {
			printf("AES Key:\n");
			buf_print(&byte_buf[key_offset], 0x20);
			printf("AES IV:\n");
			buf_print(&byte_buf[key_offset + 0x20], 0x10);

		} else {
			printf("RSA mod:\n");
			buf_print(&byte_buf[key_offset], len / 2);
			printf("RSA exp:\n");
			buf_print(&byte_buf[key_offset + (len / 2)], len / 2);
		}
		if (last)
			break;
		i++;
	}
	return 0;
}

static int otp_prog_conf(uint32_t *buf)
{
	int i, k;
	int pass = 0;
	int soak = 0;
	uint32_t prog_address;
	uint32_t data[12];
	uint32_t compare[2];
	uint32_t *buf_keep = &buf[12];
	uint32_t data_masked;
	uint32_t buf_masked;

	printf("Read OTP Config Region:\n");

	printProgress(0, 12, "");
	for (i = 0; i < 12 ; i ++) {
		printProgress(i + 1, 12, "");
		prog_address = 0x800;
		prog_address |= (i / 8) * 0x200;
		prog_address |= (i % 8) * 0x2;
		otp_read_data(prog_address, &data[i]);
	}

	printf("Check writable...\n");
	for (i = 0; i < 12; i++) {
		data_masked = data[i]  & ~buf_keep[i];
		buf_masked  = buf[i] & ~buf_keep[i];
		if (data_masked == buf_masked)
			continue;
		if ((data_masked | buf_masked) == buf_masked) {
			continue;
		} else {
			printf("Input image can't program into OTP, please check.\n");
			printf("OTPCFG[%X] = %x\n", i, data[i]);
			printf("Input [%X] = %x\n", i, buf[i]);
			printf("Mask  [%X] = %x\n", i, ~buf_keep[i]);
			return OTP_FAILURE;
		}
	}

	printf("Start Programing...\n");
	printProgress(0, 12, "");
	otp_soak(0);
	for (i = 0; i < 12; i++) {
		data_masked = data[i]  & ~buf_keep[i];
		buf_masked  = buf[i] & ~buf_keep[i];
		prog_address = 0x800;
		prog_address |= (i / 8) * 0x200;
		prog_address |= (i % 8) * 0x2;
		if (data_masked == buf_masked) {
			printProgress(i + 1, 12, "[%03X]=%08X HIT", prog_address, buf[i]);
			continue;
		}
		if (soak) {
			soak = 0;
			otp_soak(0);
		}
		printProgress(i + 1, 12, "[%03X]=%08X    ", prog_address, buf[i]);

		otp_prog_dw(buf[i], buf_keep[i], prog_address);

		pass = 0;
		for (k = 0; k < RETRY; k++) {
			if (verify_dw(prog_address, &buf[i], &buf_keep[i], compare, 1) != 0) {
				if (soak == 0) {
					soak = 1;
					otp_soak(1);
				}
				otp_prog_dw(compare[0], prog_address, 1);
			} else {
				pass = 1;
				break;
			}
		}
	}

	if (!pass)
		return OTP_FAILURE;

	return OTP_SUCCESS;

}


static int otp_strap_image_confirm(uint32_t *buf)
{
	int i;
	uint32_t *strap_keep = buf + 2;
	uint32_t *strap_protect = buf + 4;
	int bit, pbit, kbit;
	int fail = 0;
	int skip = -1;
	struct otpstrap_status otpstrap[64];

	otp_strp_status(otpstrap);
	for (i = 0; i < 64; i++) {
		if (i < 32) {
			bit = (buf[0] >> i) & 0x1;
			kbit = (strap_keep[0] >> i) & 0x1;
			pbit = (strap_protect[0] >> i) & 0x1;
		} else {
			bit = (buf[1] >> (i - 32)) & 0x1;
			kbit = (strap_keep[1] >> (i - 32)) & 0x1;
			pbit = (strap_protect[1] >> (i - 32)) & 0x1;
		}

		if (kbit == 1) {
			continue;
		} else {
			printf("OTPSTRAP[%X]:\n", i);
		}
		if (bit == otpstrap[i].value) {
			printf("    The value is same as before, skip it.\n");
			if (skip == -1)
				skip = 1;
			continue;
		} else {
			skip = 0;
		}
		if (otpstrap[i].protected == 1) {
			printf("    This bit is protected and is not writable\n");
			fail = 1;
			continue;
		}
		if (otpstrap[i].remain_times == 0) {
			printf("    This bit is no remaining times to write.\n");
			fail = 1;
			continue;
		}
		if (pbit == 1) {
			printf("    This bit will be protected and become non-writable.\n");
		}
		printf("    Write 1 to OTPSTRAP[%X] OPTION[%X], that value becomes from %d to %d.\n", i, otpstrap[i].writeable_option + 1, otpstrap[i].value, otpstrap[i].value ^ 1);
	}
	if (fail == 1)
		return OTP_FAILURE;
	else if (skip == 1)
		return OTP_PROG_SKIP;

	return 0;
}

static int otp_print_strap(int start, int count)
{
	int i, j;
	struct otpstrap_status otpstrap[64];

	if (start < 0 || start > 64)
		return OTP_USAGE;

	if ((start + count) < 0 || (start + count) > 64)
		return OTP_USAGE;

	otp_strp_status(otpstrap);

	printf("BIT(hex)  Value  Option           Status\n");
	printf("___________________________________________________________________________\n");

	for (i = start; i < start + count; i++) {
		printf("0x%-8X", i);
		printf("%-7d", otpstrap[i].value);
		for (j = 0; j < 7; j++)
			printf("%d ", otpstrap[i].option_array[j]);
		printf("   ");
		if (otpstrap[i].protected == 1) {
			printf("protected and not writable");
		} else {
			printf("not protected ");
			if (otpstrap[i].remain_times == 0) {
				printf("and no remaining times to write.");
			} else {
				printf("and still can write %d times", otpstrap[i].remain_times);
			}
		}
		printf("\n");
	}

	return OTP_SUCCESS;
}

static int otp_prog_strap(uint32_t *buf)
{
	int i, j;
	uint32_t *strap_keep = buf + 2;
	uint32_t *strap_protect = buf + 4;
	uint32_t prog_bit, prog_address;
	int bit, pbit, kbit, offset;
	int fail = 0;
	int pass = 0;
	int soak = 0;
	struct otpstrap_status otpstrap[64];

	printf("Read OTP Strap Region:\n");
	otp_strp_status(otpstrap);

	printf("Check writable...\n");
	if (otp_strap_image_confirm(buf) == OTP_FAILURE) {
		printf("Input image can't program into OTP, please check.\n");
		return OTP_FAILURE;
	}

	otp_soak(0);
	for (i = 0; i < 64; i++) {
		printProgress(i + 1, 64, "");
		prog_address = 0x800;
		if (i < 32) {
			offset = i;
			bit = (buf[0] >> offset) & 0x1;
			kbit = (strap_keep[0] >> offset) & 0x1;
			pbit = (strap_protect[0] >> offset) & 0x1;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 16) / 8) * 0x200;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 16) % 8) * 0x2;

		} else {
			offset = (i - 32);
			bit = (buf[1] >> offset) & 0x1;
			kbit = (strap_keep[1] >> offset) & 0x1;
			pbit = (strap_protect[1] >> offset) & 0x1;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 17) / 8) * 0x200;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 17) % 8) * 0x2;
		}
		prog_bit = ~(0x1 << offset);

		if (kbit == 1) {
			continue;
		}
		if (bit == otpstrap[i].value) {
			continue;
		}
		if (otpstrap[i].protected == 1) {
			fail = 1;
			continue;
		}
		if (otpstrap[i].remain_times == 0) {
			fail = 1;
			continue;
		}

		if (soak) {
			soak = 0;
			otp_soak(0);
		}

		otp_prog(prog_address, prog_bit);

		pass = 0;

		for (j = 0; j < RETRY; j++) {
			if (verify_bit(prog_address, offset, 1) == 0) {
				pass = 1;
				break;
			}
			if (soak == 0) {
				soak = 1;
				otp_soak(1);
			}
			otp_prog(prog_address, prog_bit);
		}
		if (!pass)
			return OTP_FAILURE;

		if (pbit == 0)
			continue;
		prog_address = 0x800;
		if (i < 32)
			prog_address |= 0x60c;
		else
			prog_address |= 0x60e;


		if (soak) {
			soak = 0;
			otp_soak(0);
		}

		otp_prog(prog_address, prog_bit);

		pass = 0;

		for (j = 0; j < RETRY; j++) {

			if (verify_bit(prog_address, offset, 1) == 0) {
				pass = 1;
				break;
			}
			if (soak == 0) {
				soak = 1;
				otp_soak(1);
			}
			otp_prog(prog_address, prog_bit);
		}
		if (!pass)
			return OTP_FAILURE;

	}
	if (fail == 1)
		return OTP_FAILURE;
	else
		return OTP_SUCCESS;

}

static void otp_prog_bit(uint32_t value, uint32_t prog_address, uint32_t bit_offset, int soak)
{
	int prog_bit;

	otp_soak(soak);

	if (prog_address % 2 == 0) {
		if (value)
			prog_bit = ~(0x1 << bit_offset);
		else
			return;
	} else {
		prog_address |= 1 << 15;
		if (!value)
			prog_bit = 0x1 << bit_offset;
		else
			return;
	}
	otp_prog(prog_address, prog_bit);
}

static int otp_prog_data(uint32_t *buf)
{
	int i, k;
	int pass;
	int soak = 0;
	uint32_t prog_address;
	uint32_t data[2048];
	uint32_t compare[2];
	uint32_t *buf_keep = &buf[2048];

	uint32_t data0_masked;
	uint32_t data1_masked;
	uint32_t buf0_masked;
	uint32_t buf1_masked;

	printf("Read OTP Data:\n");

	printProgress(0, 2048, "");
	for (i = 0; i < 2048 ; i += 2) {
		printProgress(i + 2, 2048, "");
		otp_read_data(i, &data[i]);
	}


	printf("Check writable...\n");
	for (i = 0; i < 2048; i++) {
		data0_masked = data[i]  & ~buf_keep[i];
		buf0_masked  = buf[i] & ~buf_keep[i];
		if (data0_masked == buf0_masked)
			continue;
		if (i % 2 == 0) {
			if ((data0_masked | buf0_masked) == buf0_masked) {
				continue;
			} else {
				printf("Input image can't program into OTP, please check.\n");
				printf("OTP_ADDR[%x] = %x\n", i, data[i]);
				printf("Input   [%x] = %x\n", i, buf[i]);
				printf("Mask    [%x] = %x\n", i, ~buf_keep[i]);
				return OTP_FAILURE;
			}
		} else {
			if ((data0_masked & buf0_masked) == buf0_masked) {
				continue;
			} else {
				printf("Input image can't program into OTP, please check.\n");
				printf("OTP_ADDR[%x] = %x\n", i, data[i]);
				printf("Input   [%x] = %x\n", i, buf[i]);
				printf("Mask    [%x] = %x\n", i, ~buf_keep[i]);
				return OTP_FAILURE;
			}
		}
	}

	printf("Start Programing...\n");
	printProgress(0, 2048, "");

	for (i = 0; i < 2048; i += 2) {
		prog_address = i;
		data0_masked = data[i]  & ~buf_keep[i];
		buf0_masked  = buf[i] & ~buf_keep[i];
		data1_masked = data[i + 1]  & ~buf_keep[i + 1];
		buf1_masked  = buf[i + 1] & ~buf_keep[i + 1];
		if ((data0_masked == buf0_masked) && (data1_masked == buf1_masked)) {
			printProgress(i + 2, 2048, "[%03X]=%08X HIT;[%03X]=%08X HIT", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			continue;
		}
		if (soak) {
			soak = 0;
			otp_soak(0);
		}
		if (data1_masked == buf1_masked) {
			printProgress(i + 2, 2048, "[%03X]=%08X    ;[%03X]=%08X HIT", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			otp_prog_dw(buf[i], buf_keep[i], prog_address);
		} else if (data0_masked == buf0_masked) {
			printProgress(i + 2, 2048, "[%03X]=%08X HIT;[%03X]=%08X    ", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			otp_prog_dw(buf[i + 1], buf_keep[i + 1], prog_address + 1);
		} else {
			printProgress(i + 2, 2048, "[%03X]=%08X    ;[%03X]=%08X    ", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			otp_prog_dw(buf[i], buf_keep[i], prog_address);
			otp_prog_dw(buf[i + 1], buf_keep[i + 1], prog_address + 1);
		}

		pass = 0;
		for (k = 0; k < RETRY; k++) {
			if (verify_dw(prog_address, &buf[i], &buf_keep[i], compare, 2) != 0) {
				if (soak == 0) {
					soak = 1;
					otp_soak(1);
				}
				if (compare[0] != 0) {
					otp_prog_dw(compare[0], buf_keep[i], prog_address);
				}
				if (compare[1] != ~0) {
					otp_prog_dw(compare[1], buf_keep[i], prog_address + 1);
				}
			} else {
				pass = 1;
				break;
			}
		}

		if (!pass)
			return OTP_FAILURE;
	}
	return OTP_SUCCESS;

}

static int do_otp_prog(int addr, int byte_size, int nconfirm)
{
	int ret;
	int mode = 0;
	uint32_t *buf;
	uint32_t *data_region = NULL;
	uint32_t *conf_region = NULL;
	uint32_t *strap_region = NULL;

	buf = map_physmem(addr, byte_size, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return OTP_FAILURE;
	}

	if (buf[0] & BIT(29)) {
		mode |= OTP_REGION_DATA;
		data_region = &buf[36];
	}
	if (buf[0] & BIT(30)) {
		mode |= OTP_REGION_CONF;
		conf_region = &buf[12];
	}
	if (buf[0] & BIT(31)) {
		mode |= OTP_REGION_STRAP;
		strap_region = &buf[4];
	}

	if (!nconfirm) {
		if (mode & OTP_REGION_DATA) {
			printf("\nOTP data region :\n");
			if (otp_print_data_info(data_region) < 0) {
				printf("OTP data error, please check.\n");
				return OTP_FAILURE;
			}
		}
		if (mode & OTP_REGION_STRAP) {
			printf("\nOTP strap region :\n");
			if (otp_print_strap_image(strap_region) < 0) {
				printf("OTP strap error, please check.\n");
				return OTP_FAILURE;
			}
		}
		if (mode & OTP_REGION_CONF) {
			printf("\nOTP configuration region :\n");
			if (otp_print_conf_image(conf_region) < 0) {
				printf("OTP config error, please check.\n");
				return OTP_FAILURE;
			}
		}

		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}

	if (mode & OTP_REGION_DATA) {
		printf("programing data region ...\n");
		ret = otp_prog_data(data_region);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		} else {
			printf("Done\n");
		}
	}
	if (mode & OTP_REGION_STRAP) {
		printf("programing strap region ...\n");
		ret = otp_prog_strap(strap_region);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		} else {
			printf("Done\n");
		}
	}
	if (mode & OTP_REGION_CONF) {
		printf("programing configuration region ...\n");
		ret = otp_prog_conf(conf_region);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		}
		printf("Done\n");
	}

	return OTP_SUCCESS;
}

static int do_otp_prog_bit(int mode, int otp_dw_offset, int bit_offset, int value, int nconfirm)
{
	uint32_t read[2];
	uint32_t strap_buf[6];
	uint32_t prog_address = 0;
	struct otpstrap_status otpstrap[64];
	int otp_bit;
	int i;
	int pass;
	int ret;

	switch (mode) {
	case OTP_REGION_CONF:
		otp_read_config(otp_dw_offset, read);
		prog_address = 0x800;
		prog_address |= (otp_dw_offset / 8) * 0x200;
		prog_address |= (otp_dw_offset % 8) * 0x2;
		otp_bit = (read[0] >> bit_offset) & 0x1;
		if (otp_bit == value) {
			printf("OTPCFG%X[%X] = %d\n", otp_dw_offset, bit_offset, value);
			printf("No need to program\n");
			return OTP_SUCCESS;
		}
		if (otp_bit == 1 && value == 0) {
			printf("OTPCFG%X[%X] = 1\n", otp_dw_offset, bit_offset);
			printf("OTP is programed, which can't be clean\n");
			return OTP_FAILURE;
		}
		printf("Program OTPCFG%X[%X] to 1\n", otp_dw_offset, bit_offset);
		break;
	case OTP_REGION_DATA:
		prog_address = otp_dw_offset;

		if (otp_dw_offset % 2 == 0) {
			otp_read_data(otp_dw_offset, read);
			otp_bit = (read[0] >> bit_offset) & 0x1;
		} else {
			otp_read_data(otp_dw_offset - 1, read);
			otp_bit = (read[1] >> bit_offset) & 0x1;
		}
		if (otp_bit == value) {
			printf("OTPDATA%X[%X] = %d\n", otp_dw_offset, bit_offset, value);
			printf("No need to program\n");
			return OTP_SUCCESS;
		}
		if (otp_bit == 1 && value == 0) {
			printf("OTPDATA%X[%X] = 1\n", otp_dw_offset, bit_offset);
			printf("OTP is programed, which can't be clean\n");
			return OTP_FAILURE;
		}
		printf("Program OTPDATA%X[%X] to 1\n", otp_dw_offset, bit_offset);
		break;
	case OTP_REGION_STRAP:
		otp_strp_status(otpstrap);
		otp_print_strap(bit_offset, 1);
		if (bit_offset < 32) {
			strap_buf[0] = value << bit_offset;
			strap_buf[2] = ~BIT(bit_offset);
			strap_buf[3] = ~0;
			strap_buf[5] = 0;
			// if (protect)
			// 	strap_buf[4] = BIT(bit_offset);
			// else
			// 	strap_buf[4] = 0;
		} else {
			strap_buf[1] = value << (bit_offset - 32);
			strap_buf[2] = ~0;
			strap_buf[3] = ~BIT(bit_offset - 32);
			strap_buf[4] = 0;
			// if (protect)
			// 	strap_buf[5] = BIT(bit_offset - 32);
			// else
			// 	strap_buf[5] = 0;
		}
		ret = otp_strap_image_confirm(strap_buf);
		if (ret == OTP_FAILURE)
			return OTP_FAILURE;
		else if (ret == OTP_PROG_SKIP)
			return OTP_SUCCESS;

		break;
	}

	if (!nconfirm) {
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}

	switch (mode) {
	case OTP_REGION_STRAP:
		return otp_prog_strap(strap_buf);
	case OTP_REGION_CONF:
	case OTP_REGION_DATA:
		otp_prog_bit(value, prog_address, bit_offset, 0);
		pass = -1;
		for (i = 0; i < RETRY; i++) {
			if (verify_bit(prog_address, bit_offset, value) != 0) {
				otp_prog_bit(value, prog_address, bit_offset, 1);
			} else {
				pass = 0;
				break;
			}
		}
		if (pass == 0)
			return OTP_SUCCESS;
	}

	return OTP_USAGE;
}

static int do_otpread(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t offset, count;
	int ret;

	if (argc == 4) {
		offset = simple_strtoul(argv[2], NULL, 16);
		count = simple_strtoul(argv[3], NULL, 16);
	} else if (argc == 3) {
		offset = simple_strtoul(argv[2], NULL, 16);
		count = 1;
	} else {
		return CMD_RET_USAGE;
	}


	if (!strcmp(argv[1], "conf")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = otp_print_config(offset, count);
	} else if (!strcmp(argv[1], "data")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = otp_print_data(offset, count);
	} else if (!strcmp(argv[1], "strap")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = otp_print_strap(offset, count);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else
		return CMD_RET_USAGE;

}

static int do_otpprog(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t addr;
	uint32_t byte_size;
	int ret;

	if (argc == 4) {
		if (strcmp(argv[1], "f"))
			return CMD_RET_USAGE;
		addr = simple_strtoul(argv[2], NULL, 16);
		byte_size = simple_strtoul(argv[3], NULL, 16);
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = do_otp_prog(addr, byte_size, 1);
	} else if (argc == 3) {
		addr = simple_strtoul(argv[1], NULL, 16);
		byte_size = simple_strtoul(argv[2], NULL, 16);
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = do_otp_prog(addr, byte_size, 0);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else if (ret == OTP_FAILURE)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_USAGE;
}

static int do_otppb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int mode = 0;
	int nconfirm = 0;
	int otp_addr = 0;
	int bit_offset;
	int value;
	int ret;

	if (argc != 4 && argc != 5 && argc != 6)
		return CMD_RET_USAGE;

	/* Drop the pb cmd */
	argc--;
	argv++;

	if (!strcmp(argv[0], "conf"))
		mode = OTP_REGION_CONF;
	else if (!strcmp(argv[0], "strap"))
		mode = OTP_REGION_STRAP;
	else if (!strcmp(argv[0], "data"))
		mode = OTP_REGION_DATA;
	else
		return CMD_RET_USAGE;

	/* Drop the region cmd */
	argc--;
	argv++;

	if (!strcmp(argv[0], "f")) {
		nconfirm = 1;
		/* Drop the force option */
		argc--;
		argv++;
	}

	if (mode == OTP_REGION_STRAP) {
		bit_offset = simple_strtoul(argv[0], NULL, 16);
		value = simple_strtoul(argv[1], NULL, 16);
		if (bit_offset >= 64)
			return CMD_RET_USAGE;
	} else {
		otp_addr = simple_strtoul(argv[0], NULL, 16);
		bit_offset = simple_strtoul(argv[1], NULL, 16);
		value = simple_strtoul(argv[2], NULL, 16);
		if (bit_offset >= 32)
			return CMD_RET_USAGE;
	}
	if (value != 0 && value != 1)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, 0x1e6f2000); //password
	ret = do_otp_prog_bit(mode, otp_addr, bit_offset, value, nconfirm);

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else if (ret == OTP_FAILURE)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_USAGE;
}

static int do_otpcmp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t addr;
	int otp_addr = 0;

	if (argc != 3)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, 0x1e6f2000); //password
	addr = simple_strtoul(argv[1], NULL, 16);
	otp_addr = simple_strtoul(argv[2], NULL, 16);
	if (otp_compare(otp_addr, addr) == 0) {
		printf("Compare pass\n");
		return CMD_RET_SUCCESS;
	} else {
		printf("Compare fail\n");
		return CMD_RET_FAILURE;
	}
}

static int do_otpinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int view = 0;
	int input;

	if (argc != 2 && argc != 3)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "conf")) {

		writel(OTP_PASSWD, 0x1e6f2000); //password
		if (argc == 3) {
			input = simple_strtoul(argv[2], NULL, 16);
			otp_print_conf_info(input);
		} else {
			otp_print_conf_info(-1);
		}
	} else if (!strcmp(argv[1], "strap")) {
		if (!strcmp(argv[2], "v")) {
			view = 1;
			/* Drop the view option */
			argc--;
			argv++;
		}
		writel(OTP_PASSWD, 0x1e6f2000); //password
		otp_print_strap_info(view);
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_otpprotect(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int input;
	int bit_offset;
	int prog_address;
	int pass;
	int i;
	if (argc != 3 && argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[0], "f")) {
		input = simple_strtoul(argv[2], NULL, 16);
	} else {
		input = simple_strtoul(argv[1], NULL, 16);
		printf("OTPSTRAP[%d] will be protected\n", input);
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return CMD_RET_FAILURE;
		}
	}

	prog_address = 0x800;
	if (input < 32) {
		bit_offset = input;
		prog_address |= 0x60c;
	} else if (input < 64) {
		bit_offset = input - 32;
		prog_address |= 0x60e;
	} else {
		return CMD_RET_USAGE;
	}

	if (verify_bit(prog_address, bit_offset, 1) == 0) {
		printf("OTPSTRAP[%d] already protected\n", input);
	}
	otp_prog_bit(1, prog_address, bit_offset, 0);
	pass = -1;
	for (i = 0; i < RETRY; i++) {
		if (verify_bit(prog_address, bit_offset, 1) != 0) {
			otp_prog_bit(1, prog_address, bit_offset, 1);
		} else {
			pass = 0;
			break;
		}
	}
	if (pass == 0) {
		printf("OTPSTRAP[%d] is protected\n", input);
		return CMD_RET_SUCCESS;
	}

	printf("Protect OTPSTRAP[%d] fail\n", input);
	return CMD_RET_FAILURE;

}
static cmd_tbl_t cmd_otp[] = {
	U_BOOT_CMD_MKENT(read, 4, 0, do_otpread, "", ""),
	U_BOOT_CMD_MKENT(info, 3, 0, do_otpinfo, "", ""),
	U_BOOT_CMD_MKENT(prog, 4, 0, do_otpprog, "", ""),
	U_BOOT_CMD_MKENT(pb, 6, 0, do_otppb, "", ""),
	U_BOOT_CMD_MKENT(protect, 3, 0, do_otpprotect, "", ""),
	U_BOOT_CMD_MKENT(cmp, 3, 0, do_otpcmp, "", ""),
};

static int do_ast_otp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_otp, ARRAY_SIZE(cmd_otp));

	/* Drop the otp command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	otp, 7, 0,  do_ast_otp,
	"ASPEED One-Time-Programmable sub-system",
	"read conf|data <otp_dw_offset> <dw_count>\n"
	"otp read strap <strap_bit_offset> <bit_count>\n"
	"otp info strap [v]\n"
	"otp info conf [otp_dw_offset]\n"
	"otp prog [f] <addr> <byte_size>\n"
	"otp pb conf|data [f] <otp_dw_offset> <bit_offset> <value>\n"
	"otp pb strap [f] <bit_offset> <value>\n"
	"otp protect [f] <bit_offset>\n"
	"otp cmp <addr> <otp_dw_offset>\n"
);
