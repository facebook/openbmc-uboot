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
#define PLLTEST_C
static const char ThisFile[] = "plltest.c";

#include "swfunc.h"

#include "comminf.h"
#include "typedef.h"
#include "io.h"

#define ASTCHIP_2400    0
#define ASTCHIP_2300    1
#define ASTCHIP_1400    2
#define ASTCHIP_1300    3
#define ASTCHIP_1050    4

const UCHAR jVersion[] = "v.0.60.06";

typedef struct _TRAPINFO {
    USHORT  CPU_clk;
    UCHAR   CPU_AHB_ratio;
} _TrapInfo;

const _TrapInfo AST_default_trap_setting[] = {
    // CPU_clk, CPU_AHB_ratio
    { 384,      2 },            // AST2400 or AST1250 ( ASTCHIP_2400 )
    { 384,      2 },            // AST2300 ( ASTCHIP_2300 )
    { 384,   0xFF },            // AST1400 ( ASTCHIP_1400 )
    { 384,   0xFF },            // AST1300 ( ASTCHIP_1300 )
    { 384,      2 }             // AST1050 ( ASTCHIP_1050 )
};

int trap_function(int argc, char *argv[])
{
    UCHAR   chiptype;   
    ULONG   ulData, ulTemp; 
    UCHAR   status = TRUE;
    USHORT  val_trap;
    
    printf("**************************************************** \n");       
    printf("*** ASPEED Trap Test %s Log               *** \n", jVersion);
    printf("***                                   for u-boot *** \n");
    printf("**************************************************** \n"); 
    printf("\n"); 
    
    // Check chip type
    switch ( ReadSOC_DD( 0x1e6e2000 + 0x7c ) ) {   
        case 0x02010303 :
        case 0x02000303 : 
            printf("The chip is AST2400 or AST1250\n" ); 
            chiptype = ASTCHIP_2400; 
            break;
            
        case 0x02010103 :
        case 0x02000003 : 
            printf("The chip is AST1400\n" ); 
            chiptype = ASTCHIP_1400; 
            break;
            
        case 0x01010303 :
        case 0x01000003 : 
            printf("The chip is AST2300\n" ); 
            chiptype = ASTCHIP_2300; 
            break;
        
        case 0x01010203 : 
            printf("The chip is AST1050\n" ); 
            chiptype = ASTCHIP_1050; 
            break;
            
        case 0x01010003 : 
            printf("The chip is AST1300\n" ); 
            chiptype = ASTCHIP_1300; 
            break;
            
        default     : 
            printf ("Error Silicon Revision ID(SCU7C) %08lx!!!\n", ReadSOC_DD( 0x1e6e2000 + 0x7c ) ); 
            return(1);
    }
    
    WriteSOC_DD(0x1e6e2000, 0x1688A8A8);
    ulData = ReadSOC_DD(0x1e6e2070);
    
    // Check CPU clock
    ulTemp  = ulData;
    ulTemp &= 0x0300;
    ulTemp >>= 8;
    
    switch (ulTemp)
    {
        case 0x00:
            val_trap = 384;
            break;
        case 0x01:
            val_trap = 360;
            break;
        case 0x02:
            val_trap = 336;
            break;
        case 0x03:
            val_trap = 408;
            break;
    }
    
    if (AST_default_trap_setting[chiptype].CPU_clk != val_trap)
    {
        printf("[ERROR] CPU CLK: Correct is %d; Real is %d \n", AST_default_trap_setting[chiptype].CPU_clk, val_trap);	
    	status = FALSE;
    }
           	
    // Check cpu_ahb_ratio
    ulTemp  = ulData;
    ulTemp &= 0x0c00;
    ulTemp >>= 10;
    
    switch (ulTemp)
    {
        case 0x00:
            val_trap = 1;
            break;
        case 0x01:
            val_trap = 2;
            break;
        case 0x02:
            val_trap = 4;
            break;
        case 0x03:
            val_trap = 3;
            break;
    }
    
    if (AST_default_trap_setting[chiptype].CPU_AHB_ratio != val_trap)
    {
        printf("[ERROR] CPU:AHB: Correct is %x:1; Real is %x:1 \n", AST_default_trap_setting[chiptype].CPU_AHB_ratio, val_trap);	
    	status = FALSE;
    }
    
    if ( status == TRUE )
        printf("[PASS] hardware trap for CPU clock and CPU\\AHB ratio.\n");
    
    return status;
}
