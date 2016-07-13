/*
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * History
 * 01/20/2004 - combined variants of original driver.
 * 01/22/2004 - Write performance enhancements for parallel chips (Tolunay)
 * 01/23/2004 - Support for x8/x16 chips (Rune Raknerud)
 * 01/27/2004 - Little endian support Ed Okerson
 *
 * Tested Architectures
 * Port Width  Chip Width    # of banks	   Flash Chip  Board
 * 32	       16	     1		   28F128J3    seranoa/eagle
 * 64	       16	     1		   28F128J3    seranoa/falcon
 *
 */

/* The DEBUG define must be before common to enable debugging */
/* #define DEBUG	*/

#include <common.h>
#include <asm/processor.h>
#include <asm/byteorder.h>
#include <environment.h>

#include <asm/arch/ast_scu.h>
#include <asm/arch/aspeed.h>


/*
 * This file implements a Common Flash Interface (CFI) driver for U-Boot.
 * The width of the port and the width of the chips are determined at initialization.
 * These widths are used to calculate the address for access CFI data structures.
 * It has been tested on an Intel Strataflash implementation and AMD 29F016D.
 *
 * References
 * JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 *
 * TODO
 *
 * Use Primary Extended Query table (PRI) and Alternate Algorithm Query
 * Table (ALT) to determine if protection is available
 *
 * Add support for other command sets Use the PRI and ALT to determine command set
 * Verify erase and program timeouts.
 */

#define CFI_MFR_MACRONIX		0x00C2
#define CFI_MFR_MICRON			0x0020
#define CFI_MFR_WINBOND         	0x00DA

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];		/* FLASH chips info */

/* Support Flash ID */
#define STM25P64		0x172020
#define STM25P128		0x182020
#define N25Q256			0x19ba20
#define N25Q512			0x20ba20
#define S25FL064A		0x160201
#define S25FL128P		0x182001
#define S25FL256S		0x190201
#define W25X16			0x1530ef
#define W25X64			0x1730ef
#define W25Q64BV		0x1740ef
#define W25Q128BV		0x1840ef
#define W25Q256FV		0x1940ef
#define MX25L1605D		0x1520C2
#define MX25L12805D		0x1820C2
#define MX25L25635E		0x1920C2
#define MX66L51235F		0x1A20C2
#define SST25VF016B		0x4125bf
#define SST25VF064C		0x4b25bf
#define SST25VF040B		0x8d25bf
#define AT25DF161		0x02461F
#define AT25DF321		0x01471F

/* SPI Define */
#define CS0_CTRL			0x10
#define CS1_CTRL			0x14
#define CS2_CTRL			0x18

/* for DMA */
#define REG_FLASH_INTERRUPT_STATUS	0x08
#define REG_FLASH_DMA_CONTROL		0x80
#define REG_FLASH_DMA_FLASH_BASE		0x84
#define REG_FLASH_DMA_DRAM_BASE		0x88
#define REG_FLASH_DMA_LENGTH			0x8c

#define FLASH_STATUS_DMA_BUSY			0x0000
#define FLASH_STATUS_DMA_READY		0x0800
#define FLASH_STATUS_DMA_CLEAR		0x0800

#define FLASH_DMA_ENABLE		0x01

#define CMD_MASK		0xFFFFFFF8

#define NORMALREAD		0x00
#define FASTREAD		0x01
#define NORMALWRITE	0x02
#define USERMODE		0x03

#define CE_LOW			0x00
#define CE_HIGH			0x04

/* AST2300 only */
#define IOMODEx1		0x00000000
#define IOMODEx2		0x20000000
#define IOMODEx2_dummy		0x30000000
#define IOMODEx4		0x40000000
#define IOMODEx4_dummy		0x50000000

#define DUMMY_COMMAND_OUT	0x00008000

/* specificspi */
#define SpecificSPI_N25Q512	0x00000001

/*-----------------------------------------------------------------------*/
static u32 ast_spi_calculate_divisor(u32 max_speed_hz)
{
	// [0] ->15 : HCLK , HCLK/16
	u8 SPI_DIV[16] = {16, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0};
	u32 i, hclk, spi_cdvr=0;

	hclk = ast_get_ahbclk();
	for(i=1;i<17;i++) {
		if(max_speed_hz >= (hclk/i)) {
			spi_cdvr = SPI_DIV[i-1];
//			printf("hclk = %d , spi_cdvr = %d \n",hclk, spi_cdvr);
			break;
		}
	}

//	printf("hclk is %d, divisor is %d, target :%d , cal speed %d\n", hclk, spi_cdvr, max_speed_hz, hclk/i);
	return spi_cdvr;
}

/* create an address based on the offset and the port width  */
inline uchar *flash_make_addr(flash_info_t * info, flash_sect_t sect, uint offset)
{
#ifdef CONFIG_2SPIFLASH
        if (info->start[0] >= PHYS_FLASH_2)
	    return ((uchar *) (info->start[sect] + (offset * 1) - (PHYS_FLASH_2 - PHYS_FLASH_2_BASE) ));
        else
	    return ((uchar *) (info->start[sect] + (offset * 1)));
#else
	return ((uchar *) (info->start[sect] + (offset * 1)));
#endif
}

static void reset_flash (flash_info_t * info)
{
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}


#if 1
        ulCtrlData = info->iomode | (info->readcmd << 16) | (info->tCK_Read << 8) | (info->dummybyte << 6) | FASTREAD;
#else
        ulCtrlData  = (info->readcmd << 16) | (info->tCK_Read << 8) | (info->dummybyte << 6) | FASTREAD;
        if (info->dualport)
            ulCtrlData  |= 0x08;
#endif
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;

}

static void enable_write (flash_info_t * info)
{
	ulong base;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;
	uchar jReg;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	//base = info->start[0];
	base = (ulong) flash_make_addr(info, 0, 0);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x06);
        udelay(10);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        udelay(10);
        do {
            jReg = *(volatile uchar *) (base);
        } while (!(jReg & 0x02));
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

}

static void write_status_register (flash_info_t * info, uchar data)
{
	ulong base;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;
	uchar jReg;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	//base = info->start[0];
	base = (ulong) flash_make_addr(info, 0, 0);

        enable_write (info);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x01);
        udelay(10);
        *(uchar *) (base) = (uchar) (data);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        udelay(10);
        do {
            jReg = *(volatile uchar *) (base);
        } while (jReg & 0x01);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

}

static void enable4b (flash_info_t * info)
{
	ulong base;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	//base = info->start[0];
	base = (ulong) flash_make_addr(info, 0, 0);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0xb7);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

} /* enable4b */

static void enable4b_spansion (flash_info_t * info)
{
	ulong base;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;
	uchar jReg;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	//base = info->start[0];
	base = (ulong) flash_make_addr(info, 0, 0);

	/* Enable 4B: BAR0 D[7] = 1 */
	ulCtrlData  = (info->tCK_Write << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);
	*(uchar *) (base) = (uchar) (0x17);
	udelay(10);
	*(uchar *) (base) = (uchar) (0x80);
	ulCtrlData &= CMD_MASK;
	ulCtrlData |= CE_HIGH | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);

	ulCtrlData &= CMD_MASK;
	ulCtrlData |= CE_LOW | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);
	*(uchar *) (base) = (uchar) (0x16);
	udelay(10);
	do {
            jReg = *(volatile uchar *) (base);
	} while (!(jReg & 0x80));
	ulCtrlData &= CMD_MASK;
	ulCtrlData |= CE_HIGH | USERMODE;
	 *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);

} /* enable4b_spansion */

static void enable4b_numonyx (flash_info_t * info)
{
	ulong base;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	//base = info->start[0];
	base = (ulong) flash_make_addr(info, 0, 0);

	/* Enable Write */
	enable_write (info);

	/* Enable 4B: CMD:0xB7 */
	ulCtrlData  = (info->tCK_Write << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);
	*(uchar *) (base) = (uchar) (0xB7);
	udelay(10);
	ulCtrlData &= CMD_MASK;
	ulCtrlData |= CE_HIGH | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);

} /* enable4b_numonyx */

/*-----------------------------------------------------------------------
 */
static void flash_write_buffer (flash_info_t *info, uchar *src, ulong addr, int len)
{
	ulong j, base, offset;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;
	uchar jReg;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	base = info->start[0];
	offset = addr - base;
	base = (ulong) flash_make_addr(info, 0, 0);

        enable_write (info);

        ulCtrlData  = (info->tCK_Write << 8);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x02);
        udelay(10);
        if (info->address32)
        {
            *(uchar *) (base) = (uchar) ((offset & 0xff000000) >> 24);
            udelay(10);
        }
        *(uchar *) (base) = (uchar) ((offset & 0xff0000) >> 16);
        udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0x00ff00) >> 8);
        udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0x0000ff));
        udelay(10);

        for (j=0; j<len; j++)
        {
            *(uchar *) (base) = *(uchar *) (src++);
            udelay(10);
        }

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        udelay(10);
        do {
            jReg = *(volatile uchar *) (base);
        } while ((jReg & 0x01));
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
        udelay(200);

        /* RFSR */
        if (info->specificspi == SpecificSPI_N25Q512)
        {
            ulCtrlData &= CMD_MASK;
            ulCtrlData |= CE_LOW | USERMODE;
            *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
            udelay(200);
            *(uchar *) (base) = (uchar) (0x70);
            udelay(10);
            do {
                jReg = *(volatile uchar *) (base);
            } while (!(jReg & 0x80));
            ulCtrlData &= CMD_MASK;
            ulCtrlData |= CE_HIGH | USERMODE;
            *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
            udelay(200);
        }
}

/*-----------------------------------------------------------------------
 *
 * export functions
 *
 */



#if defined(CFG_ENV_IS_IN_FLASH) || defined(CFG_ENV_ADDR_REDUND) || (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			break;
	}

	return i == CONFIG_SYS_MAX_FLASH_BANKS ? 0 : info;
}
#endif

/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int rcode = 0;
	int prot;
	flash_sect_t sect;

	ulong base, offset;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;
	uchar jReg;

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	if ((s_first < 0) || (s_first > s_last)) {
		puts ("- no sectors to erase\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		putc ('\n');
	}

        ulCtrlData  = (info->tCK_Erase << 8);
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
                        /* start erasing */
                        enable_write(info);

	                base = info->start[0];
                        offset = info->start[sect] - base;
	                base = (ulong) flash_make_addr(info, 0, 0);

                        ulCtrlData &= CMD_MASK;
                        ulCtrlData |= CE_LOW | USERMODE;
                        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
                        udelay(200);
                        *(uchar *) (base) = (uchar) (0xd8);
                        udelay(10);
                        if (info->address32)
                        {
                            *(uchar *) (base) = (uchar) ((offset & 0xff000000) >> 24);
                            udelay(10);
                        }
                        *(uchar *) (base) = (uchar) ((offset & 0xff0000) >> 16);
                        udelay(10);
                        *(uchar *) (base) = (uchar) ((offset & 0x00ff00) >> 8);
                        udelay(10);
                        *(uchar *) (base) = (uchar) ((offset & 0x0000ff));
                        udelay(10);

                        ulCtrlData &= CMD_MASK;
                        ulCtrlData |= CE_HIGH | USERMODE;
                        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
                        udelay(200);

                        ulCtrlData &= CMD_MASK;
                        ulCtrlData |= CE_LOW | USERMODE;
                        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
                        udelay(200);
                        *(uchar *) (base) = (uchar) (0x05);
                        udelay(10);
                        do {
                            jReg = *(volatile uchar *) (base);
                        } while ((jReg & 0x01));
                        ulCtrlData &= CMD_MASK;
                        ulCtrlData |= CE_HIGH | USERMODE;
                        *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
                        udelay(200);

                        /* RFSR */
                        if (info->specificspi == SpecificSPI_N25Q512)
                        {
                            ulCtrlData &= CMD_MASK;
                            ulCtrlData |= CE_LOW | USERMODE;
                            *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
                            udelay(200);
                            *(uchar *) (base) = (uchar) (0x70);
                            udelay(10);
                            do {
                                jReg = *(volatile uchar *) (base);
                            } while (!(jReg & 0x80));
                            ulCtrlData &= CMD_MASK;
                            ulCtrlData |= CE_HIGH | USERMODE;
                            *(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
                            udelay(200);
                        }

			putc ('.');
		}
	}
	puts (" done\n");

	reset_flash(info);

	return rcode;
}

void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	printf("%lx : CS#%ld: ", info->start[0] , info->CE);
	switch (info->flash_id & 0xff) {
		case CFI_MFR_MACRONIX: 	printf ("MACRONIX ");		break;
		case CFI_MFR_MICRON: 	printf ("MICRON ");		break;
		case CFI_MFR_WINBOND: 	printf ("WINBOND ");		break;
		default:		printf ("Unknown Vendor %lx", info->flash_id); break;
	}

	if (info->size >= (1 << 20)) {
		i = 20;
	} else {
		i = 10;
	}
	printf ("  Size: %ld %cB in %d Sectors\n",
		info->size >> i,
		(i == 20) ? 'M' : 'k',
		info->sector_count);

	return;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int count;
	unsigned char pat[] = {'|', '-', '/', '\\'};
	int patcnt = 0;
	ulong BufferSize = info->buffersize;
	/* get lower aligned address */
	if (addr & (BufferSize - 1))
	{
		count = cnt >= BufferSize ? (BufferSize - (addr & 0xff)):cnt;
		flash_write_buffer (info, src, addr, count);
		addr+= count;
		src += count;
		cnt -= count;
	}

	/* prog */
	while (cnt > 0) {
		count = cnt >= BufferSize ? BufferSize:cnt;
		flash_write_buffer (info, src, addr, count);
		addr+= count;
		src += count;
		cnt -= count;
		printf("%c\b", pat[(patcnt++) & 0x03]);
	}

	reset_flash(info);

	return (0);
}

static ulong flash_get_size (ulong base, flash_info_t *info)
{
	int j;
	unsigned long sector;
	int erase_region_size;
	ulong ulCtrlData, CtrlOffset = CS0_CTRL;
	ulong ulID;
	uchar ch[3];
	ulong reg;
	ulong WriteClk, EraseClk, ReadClk;
	ulong vbase;

	info->start[0] = base;
//	printf("base %x \n",base);
	vbase = (ulong) flash_make_addr(info, 0, 0);

	switch(info->CE) {
		case 0:
			CtrlOffset = CS0_CTRL;
			break;
		case 1:
			ast_scu_multi_func_romcs(1);
			CtrlOffset = CS1_CTRL;
			break;
		case 2:
			ast_scu_multi_func_romcs(2);
			CtrlOffset = CS2_CTRL;
			break;
	}

	/* Get Flash ID */
	ulCtrlData  = *(ulong *) (info->reg_base + CtrlOffset) & CMD_MASK;
	ulCtrlData |= CE_LOW | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);
	*(uchar *) (vbase) = (uchar) (0x9F);
	udelay(10);
	ch[0] = *(volatile uchar *)(vbase);
	udelay(10);
	ch[1] = *(volatile uchar *)(vbase);
	udelay(10);
	ch[2] = *(volatile uchar *)(vbase);
	udelay(10);
	ulCtrlData  = *(ulong *) (info->reg_base + CtrlOffset) & CMD_MASK;
	ulCtrlData |= CE_HIGH | USERMODE;
	*(ulong *) (info->reg_base + CtrlOffset) = ulCtrlData;
	udelay(200);
	ulID = ((ulong)ch[0]) | ((ulong)ch[1] << 8) | ((ulong)ch[2] << 16) ;
	info->flash_id = ulID;

//	printf("SPI Flash ID: %x \n", ulID);

	/* init default */
	info->iomode = IOMODEx1;
	info->address32 = 0;
	info->quadport = 0;
	info->specificspi = 0;

	switch (info->flash_id) {
		case STM25P64:
			info->sector_count = 128;
			info->size = 0x800000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 40;
			EraseClk = 20;
			ReadClk  = 40;
			break;

		case STM25P128:
			info->sector_count = 64;
			info->size = 0x1000000;
			erase_region_size  = 0x40000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
			break;

		case N25Q256:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
#if	1
			info->sector_count = 512;
			info->size = 0x2000000;
			info->address32 = 1;
#endif
			break;

		case N25Q512:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			info->specificspi = SpecificSPI_N25Q512;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
#if	1
			info->sector_count = 1024;
			info->size = 0x4000000;
			info->address32 = 1;
#endif
			break;

		case W25X16:
			info->sector_count = 32;
			info->size = 0x200000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x3b;
			info->dualport = 1;
			info->dummybyte = 1;
			info->iomode = IOMODEx2;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case W25X64:
			info->sector_count = 128;
			info->size = 0x800000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x3b;
			info->dualport = 1;
			info->dummybyte = 1;
			info->iomode = IOMODEx2;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case W25Q64BV:
			info->sector_count = 128;
			info->size = 0x800000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x3b;
			info->dualport = 1;
			info->dummybyte = 1;
			info->iomode = IOMODEx2;
			info->buffersize = 256;
			WriteClk = 80;
			EraseClk = 40;
			ReadClk  = 80;
			break;

		case W25Q128BV:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x3b;
			info->dualport = 1;
			info->dummybyte = 1;
			info->iomode = IOMODEx2;
			info->buffersize = 256;
			WriteClk = 104;
			EraseClk = 50;
			ReadClk  = 104;
			break;

		case W25Q256FV:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
#if	1
			info->sector_count = 512;
			info->size = 0x2000000;
			info->address32 = 1;
#endif
			break;

		case S25FL064A:
			info->sector_count = 128;
			info->size = 0x800000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case S25FL128P:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 100;
			EraseClk = 40;
			ReadClk  = 100;
			break;

		case S25FL256S:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
#if	1
			info->sector_count = 512;
			info->size = 0x2000000;
			info->address32 = 1;
#endif
			break;

		case MX25L25635E:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
#if	1
			info->sector_count = 512;
			info->size = 0x2000000;
			info->address32 = 1;
#if	defined(CONFIG_FLASH_SPIx2_Dummy)
			info->readcmd = 0xbb;
			info->dummybyte = 1;
			info->dualport = 1;
			info->iomode = IOMODEx2_dummy;
#elif	defined(CONFIG_FLASH_SPIx4_Dummy)
			info->readcmd = 0xeb;
			info->dummybyte = 3;
			info->dualport = 0;
			info->iomode = IOMODEx4_dummy;
			info->quadport = 1;
			info->dummydata = 0xaa;
#endif
#endif
			break;

		case MX66L51235F:
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 512;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
#if	1
			info->sector_count = 1024;
			info->size = 0x4000000;
			info->address32 = 1;
#if	defined(CONFIG_FLASH_SPIx2_Dummy)
			info->readcmd = 0xbb;
			info->dummybyte = 1;
			info->dualport = 1;
			info->iomode = IOMODEx2_dummy;
#elif	defined(CONFIG_FLASH_SPIx4_Dummy)
			info->readcmd = 0xeb;
			info->dummybyte = 3;
			info->dualport = 0;
			info->iomode = IOMODEx4_dummy;
			info->quadport = 1;
			info->dummydata = 0xaa;
#endif
#endif
			break;

		case MX25L12805D:
			info->sector_count = 256;
			info->size = 0x1000000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;

			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;

#if	1
#if	defined(CONFIG_FLASH_SPIx2_Dummy)
			info->readcmd = 0xbb;
			info->dummybyte = 1;
			info->dualport = 1;
			info->iomode = IOMODEx2_dummy;
#elif	defined(CONFIG_FLASH_SPIx4_Dummy)
			info->readcmd = 0xeb;
			info->dummybyte = 3;
			info->dualport = 0;
			info->iomode = IOMODEx4_dummy;
			info->quadport = 1;
			info->dummydata = 0xaa;
#endif
#endif
			break;

		case MX25L1605D:
			info->sector_count = 32;
			info->size = 0x200000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 256;
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 50;
			break;

		case SST25VF016B:
			info->sector_count = 32;
			info->size = 0x200000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 1;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case SST25VF064C:
			info->sector_count = 128;
			info->size = 0x800000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 1;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case SST25VF040B:
			info->sector_count = 8;
			info->size = 0x80000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 1;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case AT25DF161:
			info->sector_count = 32;
			info->size = 0x200000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 1;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		case AT25DF321:
			info->sector_count = 32;
			info->size = 0x400000;
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport = 0;
			info->dummybyte = 1;
			info->buffersize = 1;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			break;

		default:	/* use JEDEC ID */
			printf("Unsupported SPI Flash!! 0x%08lx\n", info->flash_id);
			erase_region_size  = 0x10000;
			info->readcmd = 0x0b;
			info->dualport  = 0;
			info->dummybyte = 1;
			info->buffersize = 1;
			WriteClk = 50;
			EraseClk = 25;
			ReadClk  = 50;
			if ((info->flash_id & 0xFF) == 0x1F) {
			/* Atmel */
				switch (info->flash_id & 0x001F00) {
					case 0x000400:
						info->sector_count = 8;
						info->size = 0x80000;
						break;
					case 0x000500:
						info->sector_count = 16;
						info->size = 0x100000;
						break;
					case 0x000600:
						info->sector_count = 32;
						info->size = 0x200000;
						break;
					case 0x000700:
						info->sector_count = 64;
						info->size = 0x400000;
						break;
					case 0x000800:
						info->sector_count = 128;
						info->size = 0x800000;
						break;
					case 0x000900:
						info->sector_count = 256;
						info->size = 0x1000000;
						break;
					default:
						printf("Can't support this SPI Flash!! \n");
						return 0;
				}
			} else {
				/* JDEC */
				switch (info->flash_id & 0xFF0000)
				{
					case 0x120000:
						info->sector_count = 4;
						info->size = 0x40000;
						break;
					case 0x130000:
						info->sector_count = 8;
						info->size = 0x80000;
						break;
					case 0x140000:
						info->sector_count =16;
						info->size = 0x100000;
						break;
					case 0x150000:
						info->sector_count =32;
						info->size = 0x200000;
						break;
					case 0x160000:
						info->sector_count =64;
						info->size = 0x400000;
						break;
					case 0x170000:
						info->sector_count =128;
						info->size = 0x800000;
						break;
					case 0x180000:
						info->sector_count =256;
						info->size = 0x1000000;
						break;
					case 0x190000:
						info->sector_count =256;
						info->size = 0x1000000;
#if	1
						info->sector_count = 512;
						info->size = 0x2000000;
						info->address32 = 1;
#if	defined(CONFIG_FLASH_SPIx2_Dummy)
						info->readcmd = 0xbb;
						info->dummybyte = 1;
						info->dualport = 1;
						info->iomode = IOMODEx2_dummy;
#elif	defined(CONFIG_FLASH_SPIx4_Dummy)
						info->readcmd = 0xeb;
						info->dummybyte = 3;
						info->dualport = 0;
						info->iomode = IOMODEx4_dummy;
						info->quadport = 1;
						info->dummydata = 0xaa;
#endif
#endif
						break;

					case 0x200000:
						info->sector_count =256;
						info->size = 0x1000000;
						if ((info->flash_id & 0xFF) == 0x20)	/* numonyx */
						info->specificspi = SpecificSPI_N25Q512;
#if	1
						info->sector_count = 1024;
						info->size = 0x4000000;
						info->address32 = 1;
#if	defined(CONFIG_FLASH_SPIx2_Dummy)
						info->readcmd = 0xbb;
						info->dummybyte = 1;
						info->dualport = 1;
						info->iomode = IOMODEx2_dummy;
#elif	defined(CONFIG_FLASH_SPIx4_Dummy)
						info->readcmd = 0xeb;
						info->dummybyte = 3;
						info->dualport = 0;
						info->iomode = IOMODEx4_dummy;
						info->quadport = 1;
						info->dummydata = 0xaa;
#endif
#endif
						break;

					default:
						printf("Can't support this SPI Flash!! \n");
						return 0;
				}
			} /* JDEC */
	}

	sector = base;
	for (j = 0; j < info->sector_count; j++) {

		info->start[j] = sector;
		sector += erase_region_size;
		info->protect[j] = 0; /* default: not protected */
	}

	/* limit Max SPI CLK to 50MHz (Datasheet v1.2) */
	if (WriteClk > 50) WriteClk = 50;
	if (EraseClk > 50) EraseClk = 50;
	if (ReadClk > 50)  ReadClk  = 50;

	info->tCK_Write = ast_spi_calculate_divisor(WriteClk*1000000);
	info->tCK_Erase = ast_spi_calculate_divisor(EraseClk*1000000);
	info->tCK_Read = ast_spi_calculate_divisor(ReadClk*1000000);

	/* unprotect flash */
	write_status_register(info, 0);

	if (info->quadport)
		write_status_register(info, 0x40);	/* enable QE */

	if (info->address32) {
#ifndef AST_SOC_G5
		reg = *((volatile ulong*) 0x1e6e2070);	/* set H/W Trappings */
		reg |= 0x10;
		*((volatile ulong*) 0x1e6e2070) = reg;
#endif
		reg  = *((volatile ulong*) (info->reg_base + 0x4));	/* enable 32b control bit*/
		reg |= (0x01 << info->CE);
		*((volatile ulong*) (info->reg_base + 0x4)) = reg;

		/* set flash chips to 32bits addressing mode */
		if ((info->flash_id & 0xFF) == 0x01)	/* Spansion */
			enable4b_spansion(info);
		else if ((info->flash_id & 0xFF) == 0x20)	/* Numonyx */
			enable4b_numonyx(info);
		else /* MXIC, Winbond */
			enable4b(info);
	}

	reset_flash(info);
//	printf("%08x \n", info->size);
	return (info->size);
}

/*-----------------------------------------------------------------------*/
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;

	*((volatile ulong*) AST_FMC_BASE) |= 0x800f0000;	/* enable Flash Write */

	/* Init: FMC  */
	/* BANK 0 : FMC CS0 , 1: FMC CS1, */
	for (i = 0; i < CONFIG_FMC_CS; ++i) {
		flash_info[i].sysspi = 0;
		flash_info[i].reg_base = AST_FMC_BASE;
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].CE = i;
		switch(i) {
			case 0:
				size += flash_info[i].size = flash_get_size(AST_FMC_CS0_BASE, &flash_info[i]);
				break;
			case 1:
				size += flash_info[i].size = flash_get_size(AST_FMC_CS1_BASE, &flash_info[i]);
				break;
			default:
				printf("TODO ~~~~ \n");
				break;
		}
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
				i, flash_info[i].size, flash_info[i].size << 20);
		}
	}

	/* BANK 2:SYSSPI CS0 */
#ifdef CONFIG_SPI0_CS
	//pin switch by trap[13:12]	-- [0:1] Enable SPI Master
	ast_scu_spi_master(1);	/* enable SPI master */
	*((volatile ulong*) AST_FMC_SPI0_BASE) |= 0x10000;	/* enable Flash Write */
	flash_info[CONFIG_FMC_CS].sysspi = 1;
	flash_info[CONFIG_FMC_CS].reg_base = AST_FMC_SPI0_BASE;
	flash_info[CONFIG_FMC_CS].flash_id = FLASH_UNKNOWN;
	flash_info[CONFIG_FMC_CS].CE = 0;
	size += flash_info[CONFIG_FMC_CS].size = flash_get_size(AST_SPI0_CS0_BASE, &flash_info[CONFIG_FMC_CS]);
	if (flash_info[2].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 2 SYS SPI - Size = 0x%08lx = %ld MB\n",
			flash_info[CONFIG_FMC_CS].size, flash_info[CONFIG_FMC_CS].size << 20);
	}
#endif

	/* Monitor protection ON by default */
#if (CONFIG_MONITOR_BASE >= AST_FMC_CS0_BASE)
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_MONITOR_BASE,
		       CONFIG_MONITOR_BASE + monitor_flash_len  - 1,
		       flash_get_info(CONFIG_MONITOR_BASE));
#endif

	/* Environment protection ON by default */
#ifdef CONFIG_ENV_IS_IN_FLASH
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR,
		       CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
		       flash_get_info(CONFIG_ENV_ADDR));
#endif

	/* Redundant environment protection ON by default */
#ifdef CONFIG_ENV_ADDR_REDUND
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR_REDUND,
		       CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SIZE_REDUND - 1,
		       flash_get_info(CONFIG_ENV_ADDR_REDUND));
#endif

	return (size);
}

void memmove_dma(void * dest,const void *src,size_t count)
{
	ulong count_align, poll_time, data;

	count_align = (count + 3) & 0xFFFFFFFC;	/* 4-bytes align */
        poll_time = 100;			/* set 100 us as default */

        /* force end of burst read */
	*(volatile ulong *) (AST_FMC_BASE + CS0_CTRL) |= CE_HIGH;
	*(volatile ulong *) (AST_FMC_BASE + CS0_CTRL) &= ~CE_HIGH;

	*(ulong *) (AST_FMC_BASE + REG_FLASH_DMA_CONTROL) = (ulong) (~FLASH_DMA_ENABLE);
	*(ulong *) (AST_FMC_BASE + REG_FLASH_DMA_FLASH_BASE) = (ulong) (src);
	*(ulong *) (AST_FMC_BASE + REG_FLASH_DMA_DRAM_BASE) = (ulong) (dest);
	*(ulong *) (AST_FMC_BASE + REG_FLASH_DMA_LENGTH) = (ulong) (count_align);
	*(ulong *) (AST_FMC_BASE + REG_FLASH_DMA_CONTROL) = (ulong) (FLASH_DMA_ENABLE);

	/* wait poll */
	do {
		udelay(poll_time);
		data = *(ulong *) (AST_FMC_BASE + REG_FLASH_INTERRUPT_STATUS);
	} while (!(data & FLASH_STATUS_DMA_READY));

	/* clear status */
	*(ulong *) (AST_FMC_BASE + REG_FLASH_INTERRUPT_STATUS) |= FLASH_STATUS_DMA_CLEAR;
}
