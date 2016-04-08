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
 */
// (Sun) This CFI driver is written for fixed-width flash chips.
// It was not designed for flexible 8-bit/16-bit chips, which are the norm.
// When those chips are connected to a bus in 8-bit mode, the address wires
// right-shifted by 1.
//FIXME: Fix the driver to auto-detect "16-bit flash wired in 8-bit mode".
// Left-shift CFI offsets by 1 bit instead of doubling the #define values.

/* The DEBUG define must be before common to enable debugging */
// (Sun) Changed to DEBUG_FLASH because flash debug()s are too numerous.
// #define DEBUG

#include <common.h>
#include <asm/processor.h>
#include <asm/byteorder.h>
#include <environment.h>
#ifdef	CONFIG_SYS_FLASH_CFI

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

#ifndef CONFIG_FLASH_BANKS_LIST
#define CONFIG_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE }
#endif

#define FLASH_CMD_CFI			0x98
#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xD0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xD0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_WRITE_TO_BUFFER	0xE8
#define FLASH_CMD_WRITE_BUFFER_CONFIRM	0xD0

#define FLASH_STATUS_DONE		0x80
#define FLASH_STATUS_ESS		0x40
#define FLASH_STATUS_ECLBS		0x20
#define FLASH_STATUS_PSLBS		0x10
#define FLASH_STATUS_VPENS		0x08
#define FLASH_STATUS_PSS		0x04
#define FLASH_STATUS_DPS		0x02
#define FLASH_STATUS_R			0x01
#define FLASH_STATUS_PROTECT		0x01

#define AMD_CMD_RESET			0xF0
#define AMD_CMD_WRITE			0xA0
#define AMD_CMD_ERASE_START		0x80
#define AMD_CMD_ERASE_SECTOR		0x30
#define AMD_CMD_UNLOCK_START		0xAA
#define AMD_CMD_UNLOCK_ACK		0x55
#define AMD_CMD_WRITE_TO_BUFFER		0x25
#define AMD_CMD_BUFFER_TO_FLASH		0x29

#define AMD_STATUS_TOGGLE		0x40
#define AMD_STATUS_ERROR		0x20
//FIXME: These 3 were also changed for 8-bit/16-bit flash chips.
#define AMD_ADDR_ERASE_START		(0xAAA/info->portwidth)
#define AMD_ADDR_START			(0xAAA/info->portwidth)
#define AMD_ADDR_ACK			(0x555/info->portwidth)

//FIXME: Fix the driver to auto-detect "16-bit flash wired in 8-bit mode".
// Left-shift CFI offsets by 1 bit instead of doubling the #define values.
#define FLASH_OFFSET_CFI		(0xAA/info->portwidth)
#define FLASH_OFFSET_CFI_RESP		(0x20/info->portwidth)
#define FLASH_OFFSET_CFI_RESP1		(0x22/info->portwidth)
#define FLASH_OFFSET_CFI_RESP2		(0x24/info->portwidth)
#define FLASH_OFFSET_PRIMARY_VENDOR	(0x26/info->portwidth)
#define FLASH_OFFSET_WTOUT		(0x3E/info->portwidth)
#define FLASH_OFFSET_WBTOUT		(0x40/info->portwidth)
#define FLASH_OFFSET_ETOUT		(0x42/info->portwidth)
#define FLASH_OFFSET_CETOUT		(0x44/info->portwidth)
#define FLASH_OFFSET_WMAX_TOUT		(0x46/info->portwidth)
#define FLASH_OFFSET_WBMAX_TOUT		(0x48/info->portwidth)
#define FLASH_OFFSET_EMAX_TOUT		(0x4A/info->portwidth)
#define FLASH_OFFSET_CEMAX_TOUT		(0x4C/info->portwidth)
#define FLASH_OFFSET_SIZE		(0x4E/info->portwidth)
#define FLASH_OFFSET_INTERFACE		(0x50/info->portwidth)
#define FLASH_OFFSET_BUFFER_SIZE	(0x54/info->portwidth)
#define FLASH_OFFSET_NUM_ERASE_REGIONS	(0x58/info->portwidth)
#define FLASH_OFFSET_ERASE_REGIONS	(0x5A/info->portwidth)
#define FLASH_OFFSET_PROTECT		(0x02/info->portwidth)
#define FLASH_OFFSET_USER_PROTECTION	(0x85/info->portwidth)
#define FLASH_OFFSET_INTEL_PROTECTION	(0x81/info->portwidth)

#define MAX_NUM_ERASE_REGIONS		4

#define FLASH_MAN_CFI			0x01000000

#define CFI_CMDSET_NONE		    0
#define CFI_CMDSET_INTEL_EXTENDED   1
#define CFI_CMDSET_AMD_STANDARD	    2
#define CFI_CMDSET_INTEL_STANDARD   3
#define CFI_CMDSET_AMD_EXTENDED	    4
#define CFI_CMDSET_MITSU_STANDARD   256
#define CFI_CMDSET_MITSU_EXTENDED   257
#define CFI_CMDSET_SST		    258


#ifdef CONFIG_SYS_FLASH_CFI_AMD_RESET /* needed for STM_ID_29W320DB on UC100 */
# undef  FLASH_CMD_RESET
# define FLASH_CMD_RESET                AMD_CMD_RESET /* use AMD-Reset instead */
#endif


typedef union {
	unsigned char c;
	unsigned short w;
	unsigned long l;
	unsigned long long ll;
} cfiword_t;

typedef union {
	volatile unsigned char *cp;
	volatile unsigned short *wp;
	volatile unsigned long *lp;
	volatile unsigned long long *llp;
} cfiptr_t;

/* use CFG_MAX_FLASH_BANKS_DETECT if defined */
#ifdef CONFIG_SYS_MAX_FLASH_BANKS_DETECT
static ulong bank_base[CONFIG_SYS_MAX_FLASH_BANKS_DETECT] = CONFIG_FLASH_BANKS_LIST;
flash_info_t flash_info[CFG_MAX_FLASH_BANKS_DETECT];	/* FLASH chips info */
#else
static ulong bank_base[CONFIG_SYS_MAX_FLASH_BANKS] = CONFIG_FLASH_BANKS_LIST;
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];		/* FLASH chips info */
#endif


/*-----------------------------------------------------------------------
 * Functions
 */
static void flash_add_byte (flash_info_t * info, cfiword_t * cword, uchar c);
static void flash_make_cmd (flash_info_t * info, uchar cmd, void *cmdbuf);
static void flash_write_cmd (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static void flash_write_cmd_nodbg (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static void flash_write_cmd_int (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd, int noDebug);
static void flash_unlock_seq (flash_info_t * info, flash_sect_t sect);
static int flash_isequal (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static int flash_isset (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static int flash_toggle (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd);
static int flash_detect_cfi (flash_info_t * info);
ulong flash_get_size (ulong base, int banknum);
static int flash_write_cfiword (flash_info_t * info, ulong dest, cfiword_t cword);
static int flash_full_status_check (flash_info_t * info, flash_sect_t sector,
				    ulong tout, char *prompt);
static void write_buffer_abort_reset(flash_info_t * info, flash_sect_t sector);				    
#if defined(CFG_ENV_IS_IN_FLASH) || defined(CFG_ENV_ADDR_REDUND) || (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
static flash_info_t *flash_get_info(ulong base);
#endif
#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
static int flash_write_cfibuffer (flash_info_t * info, ulong dest, uchar * cp, int len);
static int flash_write_cfibuffer_amd (flash_info_t * info, ulong dest, uchar * cp, int len);
#endif

/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
inline uchar *flash_make_addr (flash_info_t * info, flash_sect_t sect, uint offset)
{
	return ((uchar *) (info->start[sect] + (offset * info->portwidth)));
}

/*-----------------------------------------------------------------------
 * Debug support
 */
#ifdef DEBUG_FLASH
static void print_longlong (char *str, unsigned long long data)
{
	int i;
	char *cp;

	cp = (unsigned char *) &data;
	for (i = 0; i < 8; i++)
		sprintf (&str[i * 2], "%2.2x", *cp++);
}
#endif

#if defined(DEBUG_FLASH)
static void flash_printqry (flash_info_t * info, flash_sect_t sect)
{
	cfiptr_t cptr;
	int x, y;

	for (x = 0; x < 0x40; x += 16U / info->portwidth) {
		cptr.cp =
			flash_make_addr (info, sect,
					 x + FLASH_OFFSET_CFI_RESP);
		debug ("%p : ", cptr.cp);
		for (y = 0; y < 16; y++) {
			debug ("%2.2x ", cptr.cp[y]);
		}
		debug (" ");
		for (y = 0; y < 16; y++) {
			if (cptr.cp[y] >= 0x20 && cptr.cp[y] <= 0x7e) {
				debug ("%c", cptr.cp[y]);
			} else {
				debug (".");
			}
		}
		debug ("\n");
	}
}
#endif

/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
inline uchar flash_read_uchar (flash_info_t * info, uint offset)
{
	uchar *cp;

	cp = flash_make_addr (info, 0, offset);
#if defined(__LITTLE_ENDIAN)
	return (cp[0]);
#else
	return (cp[info->portwidth - 1]);
#endif
}

/*-----------------------------------------------------------------------
 * read a short word by swapping for ppc format.
 */
#if 0
static ushort flash_read_ushort (flash_info_t * info, flash_sect_t sect, uint offset)
{
	uchar *addr;
	ushort retval;

#ifdef DEBUG_FLASH
	int x;
#endif
	addr = flash_make_addr (info, sect, offset);

#ifdef DEBUG_FLASH
	debug ("ushort addr is at %p info->portwidth = %d\n", addr,
	       info->portwidth);
	for (x = 0; x < 2 * info->portwidth; x++) {
		debug ("addr[%x] = 0x%x\n", x, addr[x]);
	}
#endif
#if defined(__LITTLE_ENDIAN)
	if (info->interface == FLASH_CFI_X8X16) {
	    retval = (addr[0] | (addr[2] << 8));
	} else {
	    retval = (addr[0] | (addr[(info->portwidth)] << 8));
	}
#else
	retval = ((addr[(2 * info->portwidth) - 1] << 8) |
		  addr[info->portwidth - 1]);
#endif

	debug ("retval = 0x%x\n", retval);
	return retval;
}
#endif

/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maximum
 * port size word. Swap for ppc format.
 */
static ulong flash_read_long (flash_info_t * info, flash_sect_t sect, uint offset)
{
	uchar *addr;
	ulong retval;
#ifdef DEBUG_FLASH
	int x;
#endif
#if 0
	switch (info->interface) {
	case FLASH_CFI_X8:
	case FLASH_CFI_X16:
	    break;
	case FLASH_CFI_X8X16:
	    offset <<= 1;
	}
#endif	
	// flash_make_addr() multiplies offset by info->portwidth.
	addr = flash_make_addr (info, sect, offset);

#ifdef DEBUG_FLASH
	debug ("long addr is at %p info->portwidth = %d\n", addr,
	       info->portwidth);
	for (x = 0; x < 4 * info->portwidth; x++) {
		debug ("addr[%x] = 0x%x\n", x, addr[x]);
	}
#endif
#if defined(__LITTLE_ENDIAN)
	if (info->interface == FLASH_CFI_X8X16) {
	    retval = (addr[0] | (addr[2] << 8) | (addr[4] << 16) | (addr[6] << 24));
	} else {
	    retval = (addr[0] | (addr[(info->portwidth)] << 8) |
		      (addr[(2 * info->portwidth)] << 16) |
		      (addr[(3 * info->portwidth)] << 24));
	}
#else
	//FIXME: This undocumented code appears to match broken bus wiring.
	retval = (addr[(2 * info->portwidth) - 1] << 24) |
		(addr[(info->portwidth) - 1] << 16) |
		(addr[(4 * info->portwidth) - 1] << 8) |
		addr[(3 * info->portwidth) - 1];
#endif
	return retval;
}

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		size += flash_info[i].size = flash_get_size (bank_base[i], i);
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
#ifndef CFG_FLASH_QUIET_TEST
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
				i, flash_info[i].size, flash_info[i].size << 20);
#endif /* CFG_FLASH_QUIET_TEST */
		}
	}

	/* Monitor protection ON by default */
#if (CONFIG_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE)
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

/*-----------------------------------------------------------------------
 */
#if defined(CONFIG_ENV_IS_IN_FLASH) || defined(CONFIG_ENV_ADDR_REDUND) || (CONFIG_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE)
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
        uchar ch;
	uchar *addr;
  
	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("Can't erase unknown flash type - aborted\n");
		return 1;
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


	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
			switch (info->vendor) {
			case CFI_CMDSET_INTEL_STANDARD:
			case CFI_CMDSET_INTEL_EXTENDED:
				flash_write_cmd (info, sect, 0, FLASH_CMD_CLEAR_STATUS);
				flash_write_cmd (info, sect, 0, FLASH_CMD_BLOCK_ERASE);
				flash_write_cmd (info, sect, 0, FLASH_CMD_ERASE_CONFIRM);
				break;
			case CFI_CMDSET_AMD_STANDARD:
			case CFI_CMDSET_AMD_EXTENDED:
				flash_unlock_seq (info, sect);
				flash_write_cmd (info, sect, AMD_ADDR_ERASE_START,
							AMD_CMD_ERASE_START);
				flash_unlock_seq (info, sect);
				flash_write_cmd (info, sect, 0, AMD_CMD_ERASE_SECTOR);
				
				/* toggle */
	                        addr = flash_make_addr (info, sect, 0);				
				do {
				    ch = *(volatile uchar *)(addr);
                                } while ( ((ch & 0x80) == 0) || (ch != 0xFF) );
				break;
			default:
				debug ("Unkown flash vendor %d\n",
				       info->vendor);
				break;
			}

			if (flash_full_status_check
			    (info, sect, info->erase_blk_tout, "erase")) {
				rcode = 1;
			} else
				putc ('.');
		}
	}
	puts (" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("missing or unknown FLASH type\n");
		return;
	}

	printf ("CFI conformant FLASH (%d x %d)",
		(info->portwidth << 3), (info->chipwidth << 3));
	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);
	printf (" Erase timeout %ld ms, write timeout %ld ms, buffer write timeout %ld ms, buffer size %d\n",
		info->erase_blk_tout,
		info->write_tout,
		info->buffer_write_tout,
		info->buffer_size);

	puts ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
#ifdef CFG_FLASH_EMPTY_INFO
		int k;
		int size;
		int erased;
		volatile unsigned long *flash;

		/*
		 * Check if whole sector is erased
		 */
		if (i != (info->sector_count - 1))
			size = info->start[i + 1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned long *) info->start[i];
		size = size >> 2;	/* divide by 4 for longword access */
		for (k = 0; k < size; k++) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		if ((i % 5) == 0)
			printf ("\n");
		/* print empty and read-only info */
		printf (" %08lX%s%s",
			info->start[i],
			erased ? " E" : "  ",
			info->protect[i] ? "RO " : "   ");
#else	/* ! CFG_FLASH_EMPTY_INFO */
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i], info->protect[i] ? " (RO)" : "     ");
#endif
	}
	putc ('\n');
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
	ulong wp;
	ulong cp;
	int aln;
	cfiword_t cword;
	int i, rc;

#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	unsigned char pat[] = {'|', '-', '/', '\\'};
       	int patcnt = 0;
	int buffered_size;
#endif
	/* get lower aligned address */
	/* get lower aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/* handle unaligned start */
	if ((aln = addr - wp) != 0) {
		cword.l = 0;
		cp = wp;
		for (i = 0; i < aln; ++i, ++cp)
			flash_add_byte (info, &cword, (*(uchar *) cp));

		for (; (i < info->portwidth) && (cnt > 0); i++) {
			flash_add_byte (info, &cword, *src++);
			cnt--;
			cp++;
		}
		for (; (cnt == 0) && (i < info->portwidth); ++i, ++cp)
			flash_add_byte (info, &cword, (*(uchar *) cp));
		if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
			return rc;
		wp = cp;
	}

	/* handle the aligned part */
#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	buffered_size = (info->portwidth / info->chipwidth);
	buffered_size *= info->buffer_size;
	while (cnt >= info->portwidth) {
		/* Show processing */
		if ((++patcnt % 256) == 0)
		    printf("%c\b", pat[(patcnt / 256) & 0x03]);

		i = buffered_size > cnt ? cnt : buffered_size;
		if ((rc = flash_write_cfibuffer (info, wp, src, i)) != ERR_OK)
			return rc;
		i -= i & (info->portwidth - 1);
		wp += i;
		src += i;
		cnt -= i;
	}
#else
	while (cnt >= info->portwidth) {
		cword.l = 0;
		for (i = 0; i < info->portwidth; i++) {
			flash_add_byte (info, &cword, *src++);
		}
		if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
			return rc;
		wp += info->portwidth;
		cnt -= info->portwidth;
	}
#endif /* CONFIG_SYS_FLASH_USE_BUFFER_WRITE */
	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	cword.l = 0;
	for (i = 0, cp = wp; (i < info->portwidth) && (cnt > 0); ++i, ++cp) {
		flash_add_byte (info, &cword, *src++);
		--cnt;
	}
	for (; i < info->portwidth; ++i, ++cp) {
		flash_add_byte (info, &cword, (*(uchar *) cp));
	}

	return flash_write_cfiword (info, wp, cword);
}

/*-----------------------------------------------------------------------
 */
#ifdef CFG_FLASH_PROTECTION

int flash_real_protect (flash_info_t * info, long sector, int prot)
{
	int retcode = 0;

	flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT);
	if (prot)
		flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT_SET);
	else
		flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT_CLEAR);

	if ((retcode =
	     flash_full_status_check (info, sector, info->erase_blk_tout,
				      prot ? "protect" : "unprotect")) == 0) {

		info->protect[sector] = prot;
		/* Intel's unprotect unprotects all locking */
		if (prot == 0) {
			flash_sect_t i;

			for (i = 0; i < info->sector_count; i++) {
				if (info->protect[i])
					flash_real_protect (info, i, 1);
			}
		}
	}
	return retcode;
}

/*-----------------------------------------------------------------------
 * flash_read_user_serial - read the OneTimeProgramming cells
 */
void flash_read_user_serial (flash_info_t * info, void *buffer, int offset,
			     int len)
{
	uchar *src;
	uchar *dst;

	dst = buffer;
	src = flash_make_addr (info, 0, FLASH_OFFSET_USER_PROTECTION);
	flash_write_cmd (info, 0, 0, FLASH_CMD_READ_ID);
	memcpy (dst, src + offset, len);
	flash_write_cmd (info, 0, 0, info->cmd_reset);
}

/*
 * flash_read_factory_serial - read the device Id from the protection area
 */
void flash_read_factory_serial (flash_info_t * info, void *buffer, int offset,
				int len)
{
	uchar *src;

	src = flash_make_addr (info, 0, FLASH_OFFSET_INTEL_PROTECTION);
	flash_write_cmd (info, 0, 0, FLASH_CMD_READ_ID);
	memcpy (buffer, src + offset, len);
	flash_write_cmd (info, 0, 0, info->cmd_reset);
}

#endif /* CFG_FLASH_PROTECTION */

/*
 * flash_is_busy - check to see if the flash is busy
 * This routine checks the status of the chip and returns true if the chip is busy
 */
static int flash_is_busy (flash_info_t * info, flash_sect_t sect)
{
	int retval;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		retval = !flash_isset (info, sect, 0, FLASH_STATUS_DONE);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		retval = flash_toggle (info, sect, 0, AMD_STATUS_TOGGLE);
		break;
	default:
		retval = 0;
	}
#ifdef DEBUG_FLASH
	if (retval)
	    debug ("flash_is_busy: %d\n", retval);
#endif
	return retval;
}

/*-----------------------------------------------------------------------
 *  wait for XSR.7 to be set. Time out with an error if it does not.
 *  This routine does not set the flash to read-array mode.
 */
static int flash_status_check (flash_info_t * info, flash_sect_t sector,
			       ulong tout, char *prompt)
{
	ulong start, now;

	/* Wait for command completion */
	// (Sun) Fix order of checking time so it works when the CPU is very
	// slow, e.g., single-stepping or emulation.
	start = get_timer (0);
	while (now = get_timer(start),
	       flash_is_busy (info, sector))
	{
		if (now > info->erase_blk_tout) {
			printf ("Flash %s timeout at address %lx data %lx\n",
				prompt, info->start[sector],
				flash_read_long (info, sector, 0));
			flash_write_cmd (info, sector, 0, info->cmd_reset);
			return ERR_TIMOUT;
		}
	}
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Wait for XSR.7 to be set, if it times out print an error, otherwise do a full status check.
 * This routine sets the flash to read-array mode.
 */
static int flash_full_status_check (flash_info_t * info, flash_sect_t sector,
				    ulong tout, char *prompt)
{
	int retcode;

	retcode = flash_status_check (info, sector, tout, prompt);
	switch (info->vendor) {
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		if ((retcode != ERR_OK)
		    && !flash_isequal (info, sector, 0, FLASH_STATUS_DONE)) {
			retcode = ERR_INVAL;
			printf ("Flash %s error at address %lx\n", prompt,
				info->start[sector]);
			if (flash_isset (info, sector, 0, FLASH_STATUS_ECLBS | FLASH_STATUS_PSLBS)) {
				puts ("Command Sequence Error.\n");
			} else if (flash_isset (info, sector, 0, FLASH_STATUS_ECLBS)) {
				puts ("Block Erase Error.\n");
				retcode = ERR_NOT_ERASED;
			} else if (flash_isset (info, sector, 0, FLASH_STATUS_PSLBS)) {
				puts ("Locking Error\n");
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_DPS)) {
				puts ("Block locked.\n");
				retcode = ERR_PROTECTED;
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_VPENS))
				puts ("Vpp Low Error.\n");
		}
		flash_write_cmd (info, sector, 0, info->cmd_reset);
		break;
	default:
		break;
	}
	return retcode;
}

static void write_buffer_abort_reset(flash_info_t * info, flash_sect_t sector)
{
     flash_write_cmd (info, sector, 0xaaa, 0xaa);
     flash_write_cmd (info, sector, 0x555, 0x55);
     flash_write_cmd (info, sector, 0xaaa, 0xf0);    	
}	

/*-----------------------------------------------------------------------
 */
static void flash_add_byte (flash_info_t * info, cfiword_t * cword, uchar c)
{
#if defined(__LITTLE_ENDIAN)
	unsigned short	w;
	unsigned int	l;
	unsigned long long ll;
#endif

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cword->c = c;
		break;
	case FLASH_CFI_16BIT:
#if defined(__LITTLE_ENDIAN)
		w = c;
		w <<= 8;
		cword->w = (cword->w >> 8) | w;
#else
		cword->w = (cword->w << 8) | c;
#endif
		break;
	case FLASH_CFI_32BIT:
#if defined(__LITTLE_ENDIAN)
		l = c;
		l <<= 24;
		cword->l = (cword->l >> 8) | l;
#else
		cword->l = (cword->l << 8) | c;
#endif
		break;
	case FLASH_CFI_64BIT:
#if defined(__LITTLE_ENDIAN)
		ll = c;
		ll <<= 56;
		cword->ll = (cword->ll >> 8) | ll;
#else
		cword->ll = (cword->ll << 8) | c;
#endif
		break;
	}
}


/*-----------------------------------------------------------------------
 * make a proper sized command based on the port and chip widths
 */
static void flash_make_cmd (flash_info_t * info, uchar cmd, void *cmdbuf)
{
	int i;
	uchar *cp = (uchar *) cmdbuf;

#if defined(__LITTLE_ENDIAN)
	for (i = info->portwidth; i > 0; i--)
#else
	for (i = 1; i <= info->portwidth; i++)
#endif
		*cp++ = (i & (info->chipwidth - 1)) ? '\0' : cmd;
}

/*
 * Write a proper sized command to the correct address
 */
static void
flash_write_cmd (flash_info_t * info, flash_sect_t sect, uint offset,
		 uchar cmd)
{
#ifdef DEBUG_FLASH
    const int noDebug = 0;
#else
    const int noDebug = 1;
#endif
    return flash_write_cmd_int(info, sect, offset, cmd, noDebug);
}
static void
flash_write_cmd_nodbg (flash_info_t * info, flash_sect_t sect, uint offset,
		       uchar cmd)
{
    return flash_write_cmd_int(info, sect, offset, cmd, 1);
}

static void
flash_write_cmd_int (flash_info_t * info, flash_sect_t sect, uint offset,
		     uchar cmd, int noDebug)
{

	volatile cfiptr_t addr;
	cfiword_t cword;

	addr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		if (noDebug == 0)
		debug ("fwc addr %p cmd %x %x 8bit x %d bit\n", addr.cp, cmd,
		       cword.c, info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		*addr.cp = cword.c;
		break;
	case FLASH_CFI_16BIT:
		if (noDebug == 0)
		debug ("fwc addr %p cmd %x %4.4x 16bit x %d bit\n", addr.wp,
		       cmd, cword.w,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		*addr.wp = cword.w;
		break;
	case FLASH_CFI_32BIT:
		if (noDebug == 0)
		debug ("fwc addr %p cmd %x %8.8lx 32bit x %d bit\n", addr.lp,
		       cmd, cword.l,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		*addr.lp = cword.l;
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG_FLASH
		if (noDebug == 0)
		{
			char str[20];

			print_longlong (str, cword.ll);

			debug ("fwrite addr %p cmd %x %s 64 bit x %d bit\n",
			       addr.llp, cmd, str,
			       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		}
#endif
		*addr.llp = cword.ll;
		break;
	}
}

static void flash_unlock_seq (flash_info_t * info, flash_sect_t sect)
{
	flash_write_cmd_nodbg (info, sect, AMD_ADDR_START, AMD_CMD_UNLOCK_START);
	flash_write_cmd_nodbg (info, sect, AMD_ADDR_ACK, AMD_CMD_UNLOCK_ACK);
}

/*-----------------------------------------------------------------------
 */
static int flash_isequal (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;
#ifdef DEBUG_FLASH
	const int dbg = 1;
#else
	const int dbg = 0;
#endif
	cptr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);

	if (dbg)
	    debug ("is= cmd %x(%c) addr %p ", cmd, cmd, cptr.cp);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		if (dbg)
		    debug ("is= %x %x\n", cptr.cp[0], cword.c);
		retval = (cptr.cp[0] == cword.c);
		break;
	case FLASH_CFI_16BIT:
		if (dbg)
		    debug ("is= %4.4x %4.4x\n", cptr.wp[0], cword.w);
		retval = (cptr.wp[0] == cword.w);
		break;
	case FLASH_CFI_32BIT:
		if (dbg)
		    debug ("is= %8.8lx %8.8lx\n", cptr.lp[0], cword.l);
		retval = (cptr.lp[0] == cword.l);
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG_FLASH
		{
			char str1[20];
			char str2[20];

			print_longlong (str1, cptr.llp[0]);
			print_longlong (str2, cword.ll);
			debug ("is= %s %s\n", str1, str2);
		}
#endif
		retval = (cptr.llp[0] == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_isset (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;

	cptr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((cptr.cp[0] & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		retval = ((cptr.wp[0] & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		retval = ((cptr.lp[0] & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		retval = ((cptr.llp[0] & cword.ll) == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_toggle (flash_info_t * info, flash_sect_t sect, uint offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;

	cptr.cp = flash_make_addr (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((cptr.cp[0] & cword.c) != (cptr.cp[0] & cword.c));
		break;
	case FLASH_CFI_16BIT:
		retval = ((cptr.wp[0] & cword.w) != (cptr.wp[0] & cword.w));
		break;
	case FLASH_CFI_32BIT:
		retval = ((cptr.lp[0] & cword.l) != (cptr.lp[0] & cword.l));
		break;
	case FLASH_CFI_64BIT:
		retval = ((cptr.llp[0] & cword.ll) !=
			  (cptr.llp[0] & cword.ll));
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 * detect if flash is compatible with the Common Flash Interface (CFI)
 * http://www.jedec.org/download/search/jesd68.pdf
 *
*/
static int flash_detect_cfi (flash_info_t * info)
{
        ulong data;
        	
	debug ("flash_detect_cfi()... ");

#if     defined(CONFIG_FLASH_AST2300)
        data = *(ulong *)(0x1e6e2070);		/* hardware traping */
        if (data & 0x10)			/* D[4]: 0/1 (8/16) */
            info->portwidth = FLASH_CFI_16BIT;
        else
            info->portwidth = FLASH_CFI_8BIT;            
#else
        info->portwidth = FLASH_CFI_8BIT;
#endif

	{     	
		for (info->chipwidth = FLASH_CFI_BY8;
		     info->chipwidth <= info->portwidth;
		     info->chipwidth <<= 1) {
			flash_write_cmd (info, 0, 0, FLASH_CMD_RESET);
			flash_write_cmd (info, 0, FLASH_OFFSET_CFI, FLASH_CMD_CFI);
			if (flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP, 'Q')
			    //FIXME: Next 3 lines were changed for 8-bit/16-bit flash chips.
			    && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP1, 'R')
			    && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP2, 'Y')) {
				info->interface = flash_read_uchar (info, FLASH_OFFSET_INTERFACE);
				debug ("device interface is %d\n",
				       info->interface);
				debug ("found port %d chip %d ",
				       info->portwidth, info->chipwidth);
				debug ("port %d bits chip %d bits\n",
				       info->portwidth << CFI_FLASH_SHIFT_WIDTH,
				       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
				return 1;
			}
		}
	}
	debug ("not found\n");
	return 0;
}

/*
 * The following code cannot be run from FLASH!
 *
 */
ulong flash_get_size (ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int i, j;
	flash_sect_t sect_cnt;
	unsigned long sector;
	unsigned long tmp;
	int size_ratio;
	uchar num_erase_regions;
	int erase_region_size;
	int erase_region_count;

	info->start[0] = base;

	if (flash_detect_cfi (info)) {
		info->vendor = flash_read_uchar (info, FLASH_OFFSET_PRIMARY_VENDOR);
#if defined(DEBUG_FLASH)
		flash_printqry (info, 0);
#endif
		switch (info->vendor) {
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
		default:
			info->cmd_reset = FLASH_CMD_RESET;
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
			info->cmd_reset = AMD_CMD_RESET;
			break;
		}

		debugX(2, "manufacturer is %d\n", info->vendor);
		size_ratio = info->portwidth / info->chipwidth;
		/* if the chip is x8/x16 reduce the ratio by half */
#if 0
		if ((info->interface == FLASH_CFI_X8X16)
		    && (info->chipwidth == FLASH_CFI_BY8)) {
			size_ratio >>= 1;
		}
#endif
		num_erase_regions = flash_read_uchar (info, FLASH_OFFSET_NUM_ERASE_REGIONS);
		debugX(2, "size_ratio %d port %d bits chip %d bits\n",
		       size_ratio, info->portwidth << CFI_FLASH_SHIFT_WIDTH,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		debugX(2, "found %d erase regions\n", num_erase_regions);
		sect_cnt = 0;
		sector = base;
		for (i = 0; i < num_erase_regions; i++) {
			if (i > MAX_NUM_ERASE_REGIONS) {
				printf ("%d erase regions found, only %d used\n",
					num_erase_regions, MAX_NUM_ERASE_REGIONS);
				break;
			}
			// CFI Erase Block Region Information:
			// Bits[31:16] = sect_size/256, 0 means 128-byte
			// Bits[15:0]  = num_sectors - 1
			tmp = flash_read_long(info, 0,
					      FLASH_OFFSET_ERASE_REGIONS + i * 4);
			debug("CFI erase block region info[%d]: 0x%08x, ",
			      i, tmp);
			erase_region_count = (tmp & 0xffff) + 1;
			tmp >>= 16;
			erase_region_size = (tmp ? tmp * 256 : 128);
			debug ("erase_region_count=%d erase_region_size=%d\n",
				erase_region_count, erase_region_size);
#if 0
                        erase_region_size  = CFG_FLASH_SECTOR_SIZE;	// Commented out
                        erase_region_count = CFG_FLASH_SECTOR_COUNT;	// Commented out
#endif
			if (sect_cnt + erase_region_count > CONFIG_SYS_MAX_FLASH_SECT) {
			    printf("Warning: Erase region %d adds too many flash sectors"
				   " %d+%d; reducing to fit total limit of %d\n",
				   i, sect_cnt, erase_region_count, CONFIG_SYS_MAX_FLASH_SECT);
			    erase_region_count = CONFIG_SYS_MAX_FLASH_SECT - sect_cnt;
			}
			for (j = 0; j < erase_region_count; j++) {
				info->start[sect_cnt] = sector;
				sector += (erase_region_size * size_ratio);

				/*
				 * Only read protection status from supported devices (intel...)
				 */
				switch (info->vendor) {
				case CFI_CMDSET_INTEL_EXTENDED:
				case CFI_CMDSET_INTEL_STANDARD:
					info->protect[sect_cnt] =
						flash_isset (info, sect_cnt,
							     FLASH_OFFSET_PROTECT,
							     FLASH_STATUS_PROTECT);
					break;
				default:
					info->protect[sect_cnt] = 0; /* default: not protected */
				}

				sect_cnt++;
			}
		}

		info->sector_count = sect_cnt;
		/* multiply the size by the number of chips */
		// info->size = (1 << flash_read_uchar (info, FLASH_OFFSET_SIZE)) * size_ratio;
		// Use only the sectors that fit within the flash_info array size.
		info->size = sector - base;
		printf("Flash bank %d at %08x has 0x%x bytes in %d sectors"
		       " (chipSize 1<<%d, size_ratio %d).\n",
		       banknum, base, info->size, info->sector_count,
		       flash_read_uchar(info, FLASH_OFFSET_SIZE), size_ratio);

		info->buffer_size = (1 << flash_read_uchar (info, FLASH_OFFSET_BUFFER_SIZE));		
		/* Limit the buffer size to 32bytes to meet most of AMD-styles flash's minimum requirement */
		if (info->buffer_size > 32)
		    info->buffer_size = 32;    
		tmp = 1 << flash_read_uchar (info, FLASH_OFFSET_ETOUT);
		info->erase_blk_tout = (tmp * (1 << flash_read_uchar (info, FLASH_OFFSET_EMAX_TOUT)));
		tmp = 1 << flash_read_uchar (info, FLASH_OFFSET_WBTOUT);
		info->buffer_write_tout = (tmp * (1 << flash_read_uchar (info, FLASH_OFFSET_WBMAX_TOUT)));
		tmp = 1 << flash_read_uchar (info, FLASH_OFFSET_WTOUT);
		info->write_tout = (tmp * (1 << flash_read_uchar (info, FLASH_OFFSET_WMAX_TOUT))) / 1000;
		info->flash_id = FLASH_MAN_CFI;
#if 0
		if ((info->interface == FLASH_CFI_X8X16) && (info->chipwidth == FLASH_CFI_BY8)) {
			info->portwidth >>= 1;	/* XXX - Need to test on x8/x16 in parallel. */
		}
#endif
	}

	flash_write_cmd (info, 0, 0, info->cmd_reset);
	return (info->size);
}


/*-----------------------------------------------------------------------
 */
static int flash_write_cfiword (flash_info_t * info, ulong dest,
				cfiword_t cword)
{

	cfiptr_t ctladdr;
	cfiptr_t cptr;
	int flag;

	ctladdr.cp = flash_make_addr (info, 0, 0);
	cptr.cp = (uchar *) dest;


	/* Check if Flash is (sufficiently) erased */
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flag = ((cptr.cp[0] & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		flag = ((cptr.wp[0] & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		flag = ((cptr.lp[0] & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		flag = ((cptr.llp[0] & cword.ll) == cword.ll);
		break;
	default:
		return 2;
	}
	if (!flag)
		return 2;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		flash_write_cmd_nodbg (info, 0, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd_nodbg (info, 0, 0, FLASH_CMD_WRITE);
		break;
	case CFI_CMDSET_AMD_EXTENDED:
	case CFI_CMDSET_AMD_STANDARD:
		flash_unlock_seq (info, 0);
		flash_write_cmd_nodbg (info, 0, AMD_ADDR_START, AMD_CMD_WRITE);
		break;
	}

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cptr.cp[0] = cword.c;
		break;
	case FLASH_CFI_16BIT:
		cptr.wp[0] = cword.w;
		break;
	case FLASH_CFI_32BIT:
		cptr.lp[0] = cword.l;
		break;
	case FLASH_CFI_64BIT:
		cptr.llp[0] = cword.ll;
		break;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	return flash_full_status_check (info, 0, info->write_tout, "write");
}

#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/* loop through the sectors from the highest address
 * when the passed address is greater or equal to the sector address
 * we have a match
 */
static flash_sect_t find_sector (flash_info_t * info, ulong addr)
{
	flash_sect_t sector;

	for (sector = info->sector_count - 1; sector >= 0; sector--) {
		if (addr >= info->start[sector])
			break;
	}
	return sector;
}

static int flash_write_cfibuffer (flash_info_t * info, ulong dest, uchar * cp,
				  int len)
{
	flash_sect_t sector;
	int cnt;
	int retcode;
	volatile cfiptr_t src;
	volatile cfiptr_t dst;

/* Add AMD write buffer mode support, ycchen@102006 */
#if 0
	/* buffered writes in the AMD chip set is not supported yet */
	if((info->vendor ==  CFI_CMDSET_AMD_STANDARD) ||
		(info->vendor == CFI_CMDSET_AMD_EXTENDED))
		return ERR_INVAL;
#endif
	if((info->vendor ==  CFI_CMDSET_AMD_STANDARD) ||
		(info->vendor == CFI_CMDSET_AMD_EXTENDED))
	{
		retcode = flash_write_cfibuffer_amd(info, dest, cp, len);
                return retcode;
        }

	src.cp = cp;
	dst.cp = (uchar *) dest;
	sector = find_sector (info, dest);
	flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd (info, sector, 0, FLASH_CMD_WRITE_TO_BUFFER);
	if ((retcode =
	     flash_status_check (info, sector, info->buffer_write_tout,
				 "write to buffer")) == ERR_OK) {
		/* reduce the number of loops by the width of the port	*/
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			cnt = len;
			break;
		case FLASH_CFI_16BIT:
			cnt = len >> 1;
			break;
		case FLASH_CFI_32BIT:
			cnt = len >> 2;
			break;
		case FLASH_CFI_64BIT:
			cnt = len >> 3;
			break;
		default:
			return ERR_INVAL;
			break;
		}
		flash_write_cmd (info, sector, 0, (uchar) cnt - 1);
		while (cnt-- > 0) {
			switch (info->portwidth) {
			case FLASH_CFI_8BIT:
				*dst.cp++ = *src.cp++;
				break;
			case FLASH_CFI_16BIT:
				*dst.wp++ = *src.wp++;
				break;
			case FLASH_CFI_32BIT:
				*dst.lp++ = *src.lp++;
				break;
			case FLASH_CFI_64BIT:
				*dst.llp++ = *src.llp++;
				break;
			default:
				return ERR_INVAL;
				break;
			}
		}
		flash_write_cmd (info, sector, 0,
				 FLASH_CMD_WRITE_BUFFER_CONFIRM);
		retcode =
			flash_full_status_check (info, sector,
						 info->buffer_write_tout,
						 "buffer write");
	}
	flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	return retcode;
}


static int flash_write_cfibuffer_amd (flash_info_t * info, ulong dest, uchar * cp,
				  int len)
{
	flash_sect_t sector;
	int cnt;
	int retcode;
	volatile cfiptr_t src;
	volatile cfiptr_t dst;
	volatile cfiword_t tmpsrc, tmpdst;

	src.cp = cp;
	dst.cp = (uchar *) dest;
	sector = find_sector (info, dest);
	flash_unlock_seq (info, 0);
	if ((retcode =
	     flash_status_check (info, sector, info->buffer_write_tout,
				 "write to buffer")) == ERR_OK) {
		/* reduce the number of loops by the width of the port	*/
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			cnt = len;
			*dst.cp = (uchar) (AMD_CMD_WRITE_TO_BUFFER);
			*dst.cp = (uchar) (cnt -1);
			break;
		case FLASH_CFI_16BIT:
			cnt = len >> 1;
			*dst.wp = (unsigned short) (AMD_CMD_WRITE_TO_BUFFER);
			*dst.wp = (unsigned short) (cnt -1);
			break;
		case FLASH_CFI_32BIT:
			cnt = len >> 2;
			*dst.lp = (unsigned long) (AMD_CMD_WRITE_TO_BUFFER);
			*dst.lp = (unsigned long) (cnt -1);
			break;
		case FLASH_CFI_64BIT:
			cnt = len >> 3;
			*dst.llp = (unsigned long long) (AMD_CMD_WRITE_TO_BUFFER);
			*dst.llp = (unsigned long long) (cnt -1);
			break;
		default:
			return ERR_INVAL;
			break;
		}
		while (cnt-- > 0) {
			switch (info->portwidth) {
			case FLASH_CFI_8BIT:
				*dst.cp++ = *src.cp++;
				break;
			case FLASH_CFI_16BIT:
				*dst.wp++ = *src.wp++;
				break;
			case FLASH_CFI_32BIT:
				*dst.lp++ = *src.lp++;
				break;
			case FLASH_CFI_64BIT:
				*dst.llp++ = *src.llp++;
				break;
			default:
				return ERR_INVAL;
				break;
			}
		}				
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
		        src.cp--;
		        dst.cp--;
			*dst.cp = (unsigned char) (AMD_CMD_BUFFER_TO_FLASH);
		        tmpsrc.c = *src.cp & 0x80;
		        
		        do {
		            tmpdst.c = *(volatile uchar *)(dst.cp);		        
		        	
		            if (tmpdst.c & 0x20) {	/* toggle DQ5 */
		                 tmpdst.c = *(volatile uchar *)(dst.cp);		        
		                 if ((tmpdst.c & 0x80) != tmpsrc.c)
		                 {
		                     printf("program error occurred\n");
			             flash_write_cmd (info, sector, 0, info->cmd_reset);		            
			             return ERR_PROG_ERROR;			             
			         }    
		            }
		            else if (tmpdst.c & 0x02) {	/* toggle DQ1 */
		                 tmpdst.c = *(volatile uchar *)(dst.cp);		        
		                 if ((tmpdst.c & 0x80) != tmpsrc.c)
		                 {
		                     printf("write buffer error occurred \n");
		                     write_buffer_abort_reset(info, sector);
			             return ERR_PROG_ERROR;			             
			         } 		            
		            }	
		            		                	
		        } while ((tmpdst.c & 0x80) != tmpsrc.c);
		        			        
			break;
		case FLASH_CFI_16BIT:
		        src.wp--;
		        dst.wp--;
			*dst.wp = (unsigned short) (AMD_CMD_BUFFER_TO_FLASH);
		        tmpsrc.w = *src.wp & 0x80;

		        do {
		            tmpdst.w = *(volatile short *)(dst.wp);		        
		        	
		            if (tmpdst.w & 0x20) {	/* toggle DQ5 */
		                 tmpdst.w = *(volatile ushort *)(dst.wp);		        
		                 if ((tmpdst.w & 0x80) != tmpsrc.w)
		                 {
		                     printf("program error occurred\n");
			             flash_write_cmd (info, sector, 0, info->cmd_reset);		            
			             return ERR_PROG_ERROR;			             
			         }    
		            }
		            else if (tmpdst.w & 0x02) {	/* toggle DQ1 */
		                 tmpdst.w = *(volatile ushort *)(dst.wp);		        
		                 if ((tmpdst.w & 0x80) != tmpsrc.w)
		                 {
		                     printf("write buffer error occurred \n");
		                     write_buffer_abort_reset(info, sector);
			             return ERR_PROG_ERROR;			             
			         } 		            
		            }	
		            		                	
		        } while ((tmpdst.w & 0x80) != tmpsrc.w);
		        
			break;
		case FLASH_CFI_32BIT:
		        src.lp--;
		        dst.lp--;
			*dst.lp = (unsigned long) (AMD_CMD_BUFFER_TO_FLASH);
		        tmpsrc.l = *src.lp & 0x80;
		        
		        do {
		            tmpdst.l = *(volatile ulong *)(dst.lp);		        
		        	
		            if (tmpdst.l & 0x20) {	/* toggle DQ5 */
		                 tmpdst.l = *(volatile ulong *)(dst.lp);		        
		                 if ((tmpdst.l & 0x80) != tmpsrc.l)
		                 {
		                     printf("program error occurred\n");
			             flash_write_cmd (info, sector, 0, info->cmd_reset);		            
			             return ERR_PROG_ERROR;			             
			         }    
		            }
		            else if (tmpdst.l & 0x02) {	/* toggle DQ1 */
		                 tmpdst.l = *(volatile ulong *)(dst.lp);		        
		                 if ((tmpdst.l & 0x80) != tmpsrc.l)
		                 {
		                     printf("write buffer error occurred \n");
		                     write_buffer_abort_reset(info, sector);
			             return ERR_PROG_ERROR;			             
			         } 		            
		            }	
		            		                	
		        } while ((tmpdst.l & 0x80) != tmpsrc.l);
		        
			break;
		case FLASH_CFI_64BIT:
		        src.llp--;
		        dst.llp--;
			*dst.llp = (unsigned long long) (AMD_CMD_BUFFER_TO_FLASH);
		        tmpsrc.ll = *src.llp & 0x80;
		        
		        do {
		            tmpdst.ll = *(volatile unsigned long long *)(dst.llp);		        
		        	
		            if (tmpdst.ll & 0x20) {	/* toggle DQ5 */
		                 tmpdst.ll = *(volatile unsigned long long *)(dst.llp);		        
		                 if ((tmpdst.ll & 0x80) != tmpsrc.ll)
		                 {
		                     printf("program error occurred\n");
			             flash_write_cmd (info, sector, 0, info->cmd_reset);		            
			             return ERR_PROG_ERROR;			             
			         }    
		            }
		            else if (tmpdst.ll & 0x02) {	/* toggle DQ1 */
		                 tmpdst.ll = *(volatile unsigned long long *)(dst.llp);		        
		                 if ((tmpdst.ll & 0x80) != tmpsrc.ll)
		                 {
		                     printf("write buffer error occurred \n");
		                     write_buffer_abort_reset(info, sector);
			             return ERR_PROG_ERROR;			             
			         } 		            
		            }	
		            		                	
		        } while ((tmpdst.ll & 0x80) != tmpsrc.ll);
		        
			break;
		default:
			return ERR_INVAL;
			break;
		}

		retcode =
			flash_full_status_check (info, sector,
						 info->buffer_write_tout,
						 "buffer write");
	}

	return retcode;
}
#endif /* CONFIG_SYS_FLASH_USE_BUFFER_WRITE */

#ifdef	CONFIG_FLASH_AST2300_DMA
#define STCBaseAddress			0x1e620000

/* for DMA */
#define REG_FLASH_INTERRUPT_STATUS	0x08
#define REG_FLASH_DMA_CONTROL		0x80
#define REG_FLASH_DMA_FLASH_BASE	0x84
#define REG_FLASH_DMA_DRAM_BASE		0x88
#define REG_FLASH_DMA_LENGTH		0x8c

#define FLASH_STATUS_DMA_BUSY		0x0000
#define FLASH_STATUS_DMA_READY		0x0800
#define FLASH_STATUS_DMA_CLEAR		0x0800

#define FLASH_DMA_ENABLE		0x01

void * memmove_dma(void * dest,const void *src,size_t count)
{
	ulong count_align, poll_time, data;
	
	count_align = (count + 3) & 0xFFFFFFFC;	/* 4-bytes align */
        poll_time = 100;			/* set 100 us as default */

	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_CONTROL) = (ulong) (~FLASH_DMA_ENABLE);
	
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_FLASH_BASE) = (ulong *) (src);
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_DRAM_BASE) = (ulong *) (dest);
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_LENGTH) = (ulong) (count_align);
        udelay(10);
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_CONTROL) = (ulong) (FLASH_DMA_ENABLE);
	
	/* wait poll */
	do {
	    udelay(poll_time);	
	    data = *(ulong *) (STCBaseAddress + REG_FLASH_INTERRUPT_STATUS);
	} while (!(data & FLASH_STATUS_DMA_READY));
	
	/* clear status */
	*(ulong *) (STCBaseAddress + REG_FLASH_INTERRUPT_STATUS) |= FLASH_STATUS_DMA_CLEAR;
}	
#endif
#endif /* CFG_FLASH_CFI */
