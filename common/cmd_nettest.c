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
#if defined(CONFIG_ARCH_ASPEED)
  #include "../arch/arm/cpu/ast-common/SWFUNC.H"
  #include "../arch/arm/cpu/ast-common/COMMINF.H"
#else
  #include "SWFUNC.H"
  #include "COMMINF.H"
#endif

#ifdef SLT_UBOOT
extern int main_function(int argc, char *argv[], char mode);
extern unsigned long int strtoul(char *string, char **endPtr, int base);

int do_mactest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    display_lantest_log_msg = 0;
    return main_function( argc, argv, MODE_DEDICATED );
}

int do_ncsitest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    display_lantest_log_msg = 0;
    return main_function( argc, argv, MODE_NSCI );
}

U_BOOT_CMD(
    mactest,    NETESTCMD_MAX_ARGS, 0,  do_mactest,
    "Dedicated LAN test program",
    NULL
);
U_BOOT_CMD(
    ncsitest,    NETESTCMD_MAX_ARGS, 0,  do_ncsitest,
    "Share LAN (NC-SI) test program",
    NULL
);
// ------------------------------------------------------------------------------
int do_mactestd (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    display_lantest_log_msg = 1;
    return main_function( argc, argv, MODE_DEDICATED );
}

int do_ncsitestd (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    display_lantest_log_msg = 1;
    return main_function( argc, argv, MODE_NSCI );
}

U_BOOT_CMD(
    mactestd,    NETESTCMD_MAX_ARGS, 0,  do_mactestd,
    "Dedicated LAN test program and display more information",
    NULL
);
U_BOOT_CMD(
    ncsitestd,    NETESTCMD_MAX_ARGS, 0,  do_ncsitestd,
    "Share LAN (NC-SI) test program and display more information",
    NULL
);
// ------------------------------------------------------------------------------
int do_phyread (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	MAC_ENGINE	MACENG;
	MAC_ENGINE	*eng;
	int     MACnum;
	int     PHYreg;
	ULONG   result_data;
	int     ret = 0;
	int     PHYaddr;
	int     timeout = 0;
	ULONG   MAC_040;

	eng = &MACENG;
    do {
        if ( argc != 4 ) {
            printf(" Wrong parameter number.\n");
            printf(" phyr mac addr reg\n");
            printf("   mac     : 0 or 1.   [hex]\n");
            printf("   PHY addr: 0 to 0x1F.[hex]\n");
            printf("   register: 0 to 0xFF.[hex]\n");
            printf(" example: phyr 0 0 1\n");
            ret = -1;
            break;
        }

        MACnum  = strtoul(argv[1], NULL, 16);
        PHYaddr = strtoul(argv[2], NULL, 16);
        PHYreg  = strtoul(argv[3], NULL, 16);

        if ( MACnum == 0 ) {
            // Set MAC 0
            eng->run.MAC_BASE = MAC_BASE1;
        }
        else if ( MACnum == 1 ) {
                // Set MAC 1
                eng->run.MAC_BASE = MAC_BASE2;
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

        MAC_040 = Read_Reg_MAC_DD( eng, 0x40 );
        eng->inf.NewMDIO = (MAC_040 & 0x80000000) ? 1 : 0;

        if ( eng->inf.NewMDIO ) {
            Write_Reg_MAC_DD( eng, 0x60, MAC_PHYRd_New | (PHYaddr << 5) | ( PHYreg & 0x1f ) );
            while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
#ifdef Delay_PHYRd
            DELAY( Delay_PHYRd );
#endif
            result_data = Read_Reg_MAC_DD( eng, 0x64 ) & 0xffff;
        }
        else {
            Write_Reg_MAC_DD( eng, 0x60, MDC_Thres | MAC_PHYRd | (PHYaddr << 16) | ((PHYreg & 0x1f) << 21) );
            while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYRd ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
#ifdef Delay_PHYRd
            DELAY( Delay_PHYRd );
#endif
            result_data = Read_Reg_MAC_DD( eng, 0x64 ) >> 16;
        }
		printf(" PHY[%d] reg[0x%02X] = %04lx\n", PHYaddr, PHYreg, result_data );
    } while ( 0 );

    return ret;
}

int do_phywrite (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	MAC_ENGINE	MACENG;
	MAC_ENGINE	*eng;
	int     MACnum;
	int     PHYreg;
	int     PHYaddr;
	ULONG   reg_data;
	int     ret     = 0;
	int     timeout = 0;
	ULONG   MAC_040;

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
            eng->run.MAC_BASE  = MAC_BASE1;
        }
        else if ( MACnum == 1 ) {
                // Set MAC 1
                eng->run.MAC_BASE  = MAC_BASE2;
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

        MAC_040 = Read_Reg_MAC_DD( eng, 0x40 );
        eng->inf.NewMDIO = (MAC_040 & 0x80000000) ? 1 : 0;

        if ( eng->inf.NewMDIO ) {
            Write_Reg_MAC_DD( eng, 0x60, ( reg_data << 16 ) | MAC_PHYWr_New | (PHYaddr<<5) | (PHYreg & 0x1f) );

            while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
        }
        else {
            Write_Reg_MAC_DD( eng, 0x64, reg_data );
            Write_Reg_MAC_DD( eng, 0x60, MDC_Thres | MAC_PHYWr | (PHYaddr<<16) | ((PHYreg & 0x1f) << 21) );

            while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYWr ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
        } // End if ( eng->inf.NewMDIO )

		printf("Write: PHY[%d] reg[0x%02X] = %04lx\n", PHYaddr, PHYreg, reg_data );
	} while ( 0 );

	return ret;
}

int do_phydump (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	MAC_ENGINE	MACENG;
	MAC_ENGINE	*eng;
	int     MACnum;
	int     PHYreg;
	ULONG   result_data;
	int     ret = 0;
	int     PHYaddr;
	int     timeout = 0;
	ULONG   MAC_040;

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
			eng->run.MAC_BASE = MAC_BASE1;
		}
		else if ( MACnum == 1 ) {
			// Set MAC 1
			eng->run.MAC_BASE = MAC_BASE2;
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

		MAC_040 = Read_Reg_MAC_DD( eng, 0x40 );
		eng->inf.NewMDIO = (MAC_040 & 0x80000000) ? 1 : 0;

		if ( eng->inf.NewMDIO ) {
			for ( PHYreg = 0; PHYreg < 32; PHYreg++ ) {

				Write_Reg_MAC_DD( eng, 0x60, MAC_PHYRd_New | (PHYaddr << 5) | ( PHYreg & 0x1f ) );
				while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
					if ( ++timeout > TIME_OUT_PHY_RW ) {
						ret = -1;
						break;
					}
				}
#ifdef Delay_PHYRd
				DELAY( Delay_PHYRd );
#endif
				result_data = Read_Reg_MAC_DD( eng, 0x64 ) & 0xffff;
				switch ( PHYreg % 4 ) {
					case 0	: printf("%02d| %04lx ", PHYreg, result_data ); break;
					case 3	: printf("%04lx\n", result_data ); break;
					default	: printf("%04lx ", result_data ); break;
				}
			}
		}
		else {
			for ( PHYreg = 0; PHYreg < 32; PHYreg++ ) {
				Write_Reg_MAC_DD( eng, 0x60, MDC_Thres | MAC_PHYRd | (PHYaddr << 16) | ((PHYreg & 0x1f) << 21) );
				while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYRd ) {
					if ( ++timeout > TIME_OUT_PHY_RW ) {
						ret = -1;
						break;
					}
				}
#ifdef Delay_PHYRd
				DELAY( Delay_PHYRd );
#endif
				result_data = Read_Reg_MAC_DD( eng, 0x64 ) >> 16;
				switch ( PHYreg % 4 ) {
					case 0	: printf("%02d| %04lx ", PHYreg, result_data ); break;
					case 3	: printf("%04lx\n", result_data ); break;
					default	: printf("%04lx ", result_data ); break;
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
	Write_Reg_GPIO_DD( 0x78 , Read_Reg_GPIO_DD( 0x78 ) & 0xf7bfffff );
	Write_Reg_GPIO_DD( 0x7c , Read_Reg_GPIO_DD( 0x7c ) | 0x08400000 );
	DELAY( 100 );
	Write_Reg_GPIO_DD( 0x78 , Read_Reg_GPIO_DD( 0x78 ) | 0x08400000 );

	return 0;
}

U_BOOT_CMD(
	macgpio,    NETESTCMD_MAX_ARGS, 0,  do_macgpio,
	"Setting GPIO to trun on the system for the MACTEST/NCSITEST (OEM)",
	NULL
);
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

	Write_Reg_SCU_DD( 0x1DC, 0 );
	main_function( argc, re, MODE_DEDICATED );
	printf("SCU1DC= %lx\n", Read_Reg_SCU_DD(0x1DC) );

     for ( i = 0; i < 64; i += temp )
     {
         Write_Reg_SCU_DD( 0x1DC, ( ((ULONG)(i + 0x40) << 16) | ((ULONG)(i + 0x40) << 8) ) );
         printf("SCU1DC= %lx [%lx]\n", Read_Reg_SCU_DD(0x1DC) , (ULONG)temp );
         main_function( argc, re, MODE_DEDICATED );
     }

     return 0;
}

U_BOOT_CMD(
    clkduty,    NETESTCMD_MAX_ARGS, 0,  do_clkduty,
    "clkduty",
    NULL
);
*/
#endif // End SLT_UBOOT

