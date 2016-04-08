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
#include <COMMINF.H>

#ifdef SLT_UBOOT
extern int main_function(int argc, char *argv[]);

int do_mactest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    ModeSwitch = MODE_DEDICATED;
    return main_function( argc, argv);
}

int do_ncsitest (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    ModeSwitch = MODE_NSCI;
    return main_function( argc, argv);
}

U_BOOT_CMD(
    mactest,    CONFIG_SYS_MAXARGS, 0,  do_mactest,
    "mactest - Dedicated LAN test program \n",
    NULL
);
U_BOOT_CMD(
    ncsitest,    CONFIG_SYS_MAXARGS, 0,  do_ncsitest,
    "ncsitest- Share LAN (NC-SI) test program \n",
    NULL
);

// ------------------------------------------------------------------------------
int do_phyread (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    int     MACnum;
    int     PHYreg;
    ULONG   result_data;
    int     ret = 0;
    int     PHYaddr;
    int     timeout = 0;    

    do {
        if ( argc != 4 ) {
            printf(" Wrong parameter number.\n" );
            printf(" phyr mac addr reg\n"  );
            printf("   mac     : 0 or 1.   [hex]\n"  );
            printf("   PHY addr: 0 to 31.  [hex]\n"  );
            printf("   register: 0 to 0xFF.[hex]\n"  );
            printf(" example: phyr 0 0 1\n"  );
            ret = -1;
            break;
        }

        MACnum  = strtoul(argv[1], NULL, 16);
        PHYaddr = strtoul(argv[2], NULL, 16);
        PHYreg  = strtoul(argv[3], NULL, 16);
        
        if ( MACnum == 0 ) {
            // Set MAC 0
            H_MAC_BASE  = MAC_BASE1;
        }
        else if ( MACnum == 1 ) {
                // Set MAC 1
                H_MAC_BASE  = MAC_BASE2;
        }
        else {
            printf("wrong parameter (mac number)\n");
            ret = -1;
            break;
        }
        MAC_PHYBASE = H_MAC_BASE;
        
        if ( ( PHYaddr < 0 ) || ( PHYaddr > 31 ) ) {
            printf("wrong parameter (PHY address)\n");
            ret = -1;
            break;
        }
            
        MAC_40h_old = ReadSOC_DD( H_MAC_BASE + 0x40 );
        AST2300_NewMDIO = (MAC_40h_old & 0x80000000) ? 1 : 0;
        
        if ( AST2300_NewMDIO ) {
            WriteSOC_DD( MAC_PHYBASE + 0x60, MAC_PHYRd_New | (PHYaddr << 5) | ( PHYreg & 0x1f ) );
            while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYBusy_New ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
            DELAY(Delay_PHYRd);
            result_data = ReadSOC_DD( MAC_PHYBASE + 0x64 ) & 0xffff;  
        } 
        else {
            WriteSOC_DD( MAC_PHYBASE + 0x60, MDC_Thres | MAC_PHYRd | (PHYaddr << 16) | ((PHYreg & 0x1f) << 21) );
            while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYRd ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
            DELAY( Delay_PHYRd );
            result_data = ReadSOC_DD( MAC_PHYBASE + 0x64 ) >> 16;
        }
        printf(" PHY[%d] reg[%2X] = %08lX\n", PHYaddr, PHYreg, result_data ); 
    } while ( 0 );
    
    return ret;
}


int do_phywrite (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    int     MACnum;
    int     PHYreg;
    int     PHYaddr;
    ULONG   reg_data;
    int     ret     = 0;
    int     timeout = 0;    
    
    do {
        if ( argc != 5 )
        {
            printf(" Wrong parameter number.\n" );
            printf(" phyw mac addr reg data\n"  );
            printf("   mac     : 0 or 1.     [hex]\n"  );
            printf("   PHY addr: 0 to 31.    [hex]\n"  );
            printf("   register: 0 to 0xFF.  [hex]\n"  );
            printf("   data    : 0 to 0xFFFF.[hex]\n"  );
            printf(" example: phyw 0 0 0 610\n"  );
            ret = -1;
            break;
        }
        
        MACnum   = strtoul(argv[1], NULL, 16);
        PHYaddr  = strtoul(argv[2], NULL, 16);
        PHYreg   = strtoul(argv[3], NULL, 16);
        reg_data = strtoul(argv[4], NULL, 16);
        
        if ( MACnum == 0 ) {
            // Set MAC 0
            H_MAC_BASE  = MAC_BASE1;
        }
        else if ( MACnum == 1 ) {
                // Set MAC 1
                H_MAC_BASE  = MAC_BASE2;
        }
        else {
            printf("wrong parameter (mac number)\n");
            ret = -1;
            break;
        }
        MAC_PHYBASE = H_MAC_BASE;
        
        if ( ( PHYaddr < 0 ) || ( PHYaddr > 31 ) ) {
            printf("wrong parameter (PHY address)\n");
            ret = -1;
            break;
        }
        
        MAC_40h_old = ReadSOC_DD( H_MAC_BASE + 0x40 );
        AST2300_NewMDIO = (MAC_40h_old & 0x80000000) ? 1 : 0;
        
        if (AST2300_NewMDIO) {
            WriteSOC_DD( MAC_PHYBASE + 0x60, ( reg_data << 16 ) | MAC_PHYWr_New | (PHYaddr<<5) | (PHYreg & 0x1f));

            while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYBusy_New ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
        } 
        else {
            WriteSOC_DD( MAC_PHYBASE + 0x64, reg_data );
            WriteSOC_DD( MAC_PHYBASE + 0x60, MDC_Thres | MAC_PHYWr | (PHYaddr<<16) | ((PHYreg & 0x1f) << 21));

            while ( ReadSOC_DD( MAC_PHYBASE + 0x60 ) & MAC_PHYWr ) {
                if ( ++timeout > TIME_OUT_PHY_RW ) {
                    ret = -1;
                    break;
                }
            }
        } // End if (AST2300_NewMDIO)
        
        printf("Write: PHY[%d] reg[%2X] = %08lX\n", PHYaddr, PHYreg, reg_data );     
    } while ( 0 );

    return ret;
}

U_BOOT_CMD(
    phyr,    CONFIG_SYS_MAXARGS, 0,  do_phyread,
    "phyr    - Read PHY register.  (phyr mac addr reg)\n",
    NULL
);

U_BOOT_CMD(
    phyw,    CONFIG_SYS_MAXARGS, 0,  do_phywrite,
    "phyw    - Write PHY register. (phyw mac addr reg data)\n",
    NULL
);

#endif // End SLT_UBOOT

