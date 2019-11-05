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
#include <command.h>

#include "swfunc.h"
#include "comminf.h"
#include "mem_io.h"
#include "mac_api.h"

extern int mac_test(int argc, char * const argv[], uint32_t mode);

int do_mactest (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 0;
	return mac_test(argc, argv, MODE_DEDICATED);
}

int do_ncsitest (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 0;
	return mac_test(argc, argv, MODE_NCSI);
}

U_BOOT_CMD(mactest, NETESTCMD_MAX_ARGS, 0, do_mactest,
	   "Dedicated LAN test program", NULL);
U_BOOT_CMD(ncsitest, NETESTCMD_MAX_ARGS, 0, do_ncsitest,
	   "Share LAN (NC-SI) test program", NULL);

// ------------------------------------------------------------------------------
int do_mactestd (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 1;
	return mac_test(argc, argv, MODE_DEDICATED);
}

int do_ncsitestd (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 1;
	return mac_test(argc, argv, MODE_NCSI);
}

U_BOOT_CMD(mactestd, NETESTCMD_MAX_ARGS, 0, do_mactestd,
	   "Dedicated LAN test program and display more information", NULL);
U_BOOT_CMD(ncsitestd, NETESTCMD_MAX_ARGS, 0, do_ncsitestd,
	   "Share LAN (NC-SI) test program and display more information", NULL);

// ------------------------------------------------------------------------------
#if 0
void multi_pin_2_mdcmdio_init( MAC_ENGINE *eng )
{  
#if defined(CONFIG_ASPEED_AST2500)
	switch (eng->run.mdio_idx) {
	case 0:
		SCU_WR((SCU_RD(0x088) | 0xC0000000), 0x88);
		break;
	case 1:
		SCU_WR((SCU_RD(0x090) | 0x00000004), 0x90);
		break;
	default:
		break;
	}
#endif
} 

int do_phyread (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	MAC_ENGINE MACENG;
	MAC_ENGINE *eng;
	int MACnum;
	int PHYreg;
	uint32_t result_data;
	int ret = 0;
	int PHYaddr;
	int timeout = 0;
	uint32_t MAC_040;

	eng = &MACENG;
	do {
		if (argc != 4) {
			printf(" Wrong parameter number.\n");
			printf(" phyr mac addr reg\n");
			printf("   mac     : 0 or 1.   [hex]\n");
			printf("   PHY addr: 0 to 0x1F.[hex]\n");
			printf("   register: 0 to 0xFF.[hex]\n");
			printf(" example: phyr 0 0 1\n");
			ret = -1;
			break;
		}

		MACnum = simple_strtoul(argv[1], NULL, 16);
		PHYaddr = simple_strtoul(argv[2], NULL, 16);
		PHYreg = simple_strtoul(argv[3], NULL, 16);

		if (MACnum == 0) {
			// Set MAC 0
			eng->run.mac_base = MAC1_BASE;
			eng->run.mdio_idx = 0;
		} else if (MACnum == 1) {
			// Set MAC 1
			eng->run.mac_base = MAC2_BASE;
			eng->run.mdio_idx = 1;
		} else {
			printf("wrong parameter (mac number)\n");
			ret = -1;
			break;
		}

		if ((PHYaddr < 0) || (PHYaddr > 31)) {
			printf("wrong parameter (PHY address)\n");
			ret = -1;
			break;
		}

		multi_pin_2_mdcmdio_init(eng);
		MAC_040 = mac_reg_read(eng, 0x40);
#ifdef CONFIG_ASPEED_AST2600
		eng->env.is_new_mdio_reg[MACnum] = 1;
#else
		eng->env.is_new_mdio_reg[MACnum] = (MAC_040 & 0x80000000) ? 1 : 0;
#endif

		if (eng->env.is_new_mdio_reg[MACnum]) {
#ifdef CONFIG_ASPEED_AST2600
			mac_reg_write(eng, 0x60,
					 MAC_PHYRd_New | (PHYaddr << 21) |
					     ((PHYreg & 0x1f) << 16));
			while (mac_reg_read(eng, 0x60) &
			       MAC_PHYBusy_New) {
#else
			mac_reg_write(eng, 0x60,
					 MAC_PHYRd_New | (PHYaddr << 5) |
					     (PHYreg & 0x1f));
			while (mac_reg_read(eng, 0x60) & MAC_PHYBusy_New) {
#endif
				if (++timeout > TIME_OUT_PHY_RW) {
					ret = -1;
					break;
				}
			}
#ifdef Delay_PHYRd
			DELAY(Delay_PHYRd);
#endif
			result_data = mac_reg_read(eng, 0x64) & 0xffff;
		} else {
			mac_reg_write(eng, 0x60,
					 MDC_Thres | MAC_PHYRd |
					     (PHYaddr << 16) |
					     ((PHYreg & 0x1f) << 21));
			while (mac_reg_read(eng, 0x60) & MAC_PHYRd) {
				if (++timeout > TIME_OUT_PHY_RW) {
					ret = -1;
					break;
				}
			}
#ifdef Delay_PHYRd
			DELAY(Delay_PHYRd);
#endif
			result_data = mac_reg_read(eng, 0x64) >> 16;
		}
		printf(" PHY[%d] reg[0x%02X] = %04x\n", PHYaddr, PHYreg,
		       result_data);
	} while (0);

	return ret;
}

int do_phywrite (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	MAC_ENGINE	MACENG;
	MAC_ENGINE	*eng;
	uint32_t MACnum;
	int     PHYreg;
	int     PHYaddr;
	uint32_t   reg_data;
	int     ret     = 0;
	int     timeout = 0;
	uint32_t   MAC_040;

	eng = &MACENG;

	do {
		if ( argc != 5 )
		{
			printf(" Wrong parameter number.\n");
			printf(" phyw mac addr reg data\n");
			printf("   mac     : 0 or 1.     [hex]\n");
			printf("   PHY addr: 0 to 0x1F.  [hex]\n");
			printf("   register: 0 to 0xFF.  [hex]\n");
			printf("   data    : 0 to 0xFFFF.[hex]\n");
			printf(" example: phyw 0 0 0 610\n");
			ret = -1;
			break;
		}

		MACnum   = strtoul(argv[1], NULL, 16);
		PHYaddr  = strtoul(argv[2], NULL, 16);
		PHYreg   = strtoul(argv[3], NULL, 16);
		reg_data = strtoul(argv[4], NULL, 16);

		if ( MACnum == 0 ) {
			// Set MAC 0
			eng->run.mac_base  = MAC1_BASE;
			eng->run.mdio_idx = 0;
		}
		else if ( MACnum == 1 ) {
			// Set MAC 1
			eng->run.mac_base  = MAC2_BASE;
			eng->run.mdio_idx = 1;
		}
		else {
			printf("wrong parameter (mac number)\n");
			ret = -1;
			break;
		}

		if ( ( PHYaddr < 0 ) || ( PHYaddr > 31 ) ) {
			printf("wrong parameter (PHY address)\n");
			ret = -1;
			break;
		}

		multi_pin_2_mdcmdio_init( eng );
		MAC_040 = mac_reg_read( eng, 0x40 );
#ifdef CONFIG_ASPEED_AST2600
		eng->env.is_new_mdio_reg[MACnum] = 1;
#else
		eng->env.is_new_mdio_reg[MACnum] = (MAC_040 & 0x80000000) ? 1 : 0;
#endif

		if (eng->env.is_new_mdio_reg[MACnum]) {
#ifdef CONFIG_ASPEED_AST2600
			mac_reg_write( eng, 0x60, reg_data | MAC_PHYWr_New | (PHYaddr<<21) | ((PHYreg & 0x1f)<<16) );
			
			while ( mac_reg_read( eng, 0x60 ) & MAC_PHYBusy_New ) {
#else
			mac_reg_write( eng, 0x60, ( reg_data << 16 ) | MAC_PHYWr_New | (PHYaddr<<5) | (PHYreg & 0x1f) );

			while ( mac_reg_read( eng, 0x60 ) & MAC_PHYBusy_New ) {
#endif			
				if ( ++timeout > TIME_OUT_PHY_RW ) {
					ret = -1;
					break;
				}
			}
		}
		else {
			mac_reg_write( eng, 0x64, reg_data );
			mac_reg_write( eng, 0x60, MDC_Thres | MAC_PHYWr | (PHYaddr<<16) | ((PHYreg & 0x1f) << 21) );

			while ( mac_reg_read( eng, 0x60 ) & MAC_PHYWr ) {
				if ( ++timeout > TIME_OUT_PHY_RW ) {
					ret = -1;
					break;
				}
			}
		} // End if (eng->env.new_mdio_reg)

		printf("Write: PHY[%d] reg[0x%02X] = %04x\n", PHYaddr, PHYreg, reg_data );
	} while ( 0 );

	return ret;
}

int do_phydump (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	MAC_ENGINE	MACENG;
	MAC_ENGINE	*eng;
	int     MACnum;
	int     PHYreg;
	uint32_t   result_data;
	int     ret = 0;
	int     PHYaddr;
	int     timeout = 0;
	uint32_t   MAC_040;

	eng = &MACENG;
	do {
		if ( argc != 3 ) {
			printf(" Wrong parameter number.\n");
			printf(" phyd mac addr\n");
			printf("   mac     : 0 or 1.   [hex]\n");
			printf("   PHY addr: 0 to 0x1F.[hex]\n");
			printf(" example: phyd 0 0\n");
			ret = -1;
			break;
		}

		MACnum  = strtoul(argv[1], NULL, 16);
		PHYaddr = strtoul(argv[2], NULL, 16);

		if ( MACnum == 0 ) {
			// Set MAC 0
			eng->run.mac_base = MAC1_BASE;
			eng->run.mdio_idx = 0;
		}
		else if ( MACnum == 1 ) {
			// Set MAC 1
			eng->run.mac_base = MAC2_BASE;
			eng->run.mdio_idx = 1;
		}
		else {
			printf("wrong parameter (mac number)\n");
			ret = -1;
			break;
		}

		if ( ( PHYaddr < 0 ) || ( PHYaddr > 31 ) ) {
			printf("wrong parameter (PHY address)\n");
			ret = -1;
			break;
		}

		multi_pin_2_mdcmdio_init( eng );
		MAC_040 = mac_reg_read( eng, 0x40 );
#ifdef CONFIG_ASPEED_AST2600
		eng->env.is_new_mdio_reg[MACnum] = 1;
#else
		eng->env.is_new_mdio_reg[MACnum] = (MAC_040 & 0x80000000) ? 1 : 0;
#endif

		if (eng->env.is_new_mdio_reg[MACnum]) {
			for ( PHYreg = 0; PHYreg < 32; PHYreg++ ) {
#ifdef CONFIG_ASPEED_AST2600
				mac_reg_write( eng, 0x60, MAC_PHYRd_New | (PHYaddr << 21) | (( PHYreg & 0x1f ) << 16) );
				
				while ( mac_reg_read( eng, 0x60 ) & MAC_PHYBusy_New ) {
#else				
				mac_reg_write( eng, 0x60, MAC_PHYRd_New | (PHYaddr << 5) | ( PHYreg & 0x1f ) );
				while ( mac_reg_read( eng, 0x60 ) & MAC_PHYBusy_New ) {
#endif				
					if ( ++timeout > TIME_OUT_PHY_RW ) {
						ret = -1;
						break;
					}
				}
#ifdef Delay_PHYRd
				DELAY( Delay_PHYRd );
#endif
				result_data = mac_reg_read( eng, 0x64 ) & 0xffff;
				switch ( PHYreg % 4 ) {
					case 0	: printf("%02d| %04x ", PHYreg, result_data ); break;
					case 3	: printf("%04x\n", result_data ); break;
					default	: printf("%04x ", result_data ); break;
				}
			}
		}
		else {
			for ( PHYreg = 0; PHYreg < 32; PHYreg++ ) {
				mac_reg_write( eng, 0x60, MDC_Thres | MAC_PHYRd | (PHYaddr << 16) | ((PHYreg & 0x1f) << 21) );
				while ( mac_reg_read( eng, 0x60 ) & MAC_PHYRd ) {
					if ( ++timeout > TIME_OUT_PHY_RW ) {
						ret = -1;
						break;
					}
				}
#ifdef Delay_PHYRd
				DELAY( Delay_PHYRd );
#endif
				result_data = mac_reg_read( eng, 0x64 ) >> 16;
				switch ( PHYreg % 4 ) {
					case 0	: printf("%02d| %04x ", PHYreg, result_data ); break;
					case 3	: printf("%04x\n", result_data ); break;
					default	: printf("%04x ", result_data ); break;
				}
			}
		}
	} while ( 0 );

	return ret;
}

U_BOOT_CMD(
	phyr,    NETESTCMD_MAX_ARGS, 0,  do_phyread,
	"Read PHY register.  (phyr mac addr reg)",
	NULL
);

U_BOOT_CMD(
	phyw,    NETESTCMD_MAX_ARGS, 0,  do_phywrite,
	"Write PHY register. (phyw mac addr reg data)",
	NULL
);

U_BOOT_CMD(
	phyd,    NETESTCMD_MAX_ARGS, 0,  do_phydump,
	"Dump PHY register. (phyd mac addr)",
	NULL
);

// ------------------------------------------------------------------------------
int do_macgpio (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	GPIO_WR(GPIO_RD( 0x78 ) & 0xf7bfffff, 0x78);
	GPIO_WR(GPIO_RD( 0x7c ) | 0x08400000, 0x7c);
	DELAY( 100 );
	GPIO_WR(GPIO_RD( 0x78 ) | 0x08400000, 0x78);

	return 0;
}

U_BOOT_CMD(
	macgpio,    NETESTCMD_MAX_ARGS, 0,  do_macgpio,
	"Setting GPIO to trun on the system for the MACTEST/NCSITEST (OEM)",
	NULL	
);
#endif
/*
int do_clkduty (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int temp;
	int i;
	char *re[10];

	temp   = strtoul(argv[1], NULL, 16);
	for ( i = 1; i < argc; i++ )
	{
		re[i] = argv[i + 1];
		printf("arg[%d]= %s\n", i , re[i]);
	}
	argc--;

	SCU_WR(0,  0x1DC);
	mac_test( argc, re, MODE_DEDICATED );
	printf("SCU1DC= %x\n", SCU_RD(0x1DC) );

	for ( i = 0; i < 64; i += temp )
	{
		SCU_WR((((uint32_t)(i + 0x40) << 16) | ((uint32_t)(i + 0x40) <<
			8) ), 0x1DC);
		
		printf("SCU1DC= %x [%x]\n", SCU_RD(0x1DC) , (uint32_t)temp );
		mac_test( argc, re, MODE_DEDICATED );
	}

	return 0;
}

U_BOOT_CMD(
	clkduty,    NETESTCMD_MAX_ARGS, 0,  do_clkduty,
	"clkduty",
	NULL
);
*/
