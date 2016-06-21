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
#define STRESS_C
static const char ThisFile[] = "STRESS.c";

#include "SWFUNC.H"
#include "COMMINF.H"
#include "IO.H"

#define DRAM_MapAdr			81000000
#define TIMEOUT_DRAM              5000000

/* ------------------------------------------------------------------------- */
int MMCTestBurst(unsigned int datagen)
{
  unsigned int data;
  unsigned int timeout = 0;

  WriteSOC_DD( 0x1E6E0070, 0x00000000 );
  WriteSOC_DD( 0x1E6E0070, (0x000000C1 | (datagen << 3)) );

  do {
    data = ReadSOC_DD( 0x1E6E0070 ) & 0x3000;

    if( data & 0x2000 )
      return(0);

    if( ++timeout > TIMEOUT_DRAM ) {
      printf("Timeout!!\n");
      WriteSOC_DD( 0x1E6E0070, 0x00000000 );

      return(0);
    }
  } while( !data );
  WriteSOC_DD( 0x1E6E0070, 0x00000000 );

  return(1);
}

/* ------------------------------------------------------------------------- */
int MMCTestSingle(unsigned int datagen)
{
  unsigned int data;
  unsigned int timeout = 0;

  WriteSOC_DD( 0x1E6E0070, 0x00000000 );
  WriteSOC_DD( 0x1E6E0070, (0x00000085 | (datagen << 3)) );

  do {
    data = ReadSOC_DD( 0x1E6E0070 ) & 0x3000;

    if( data & 0x2000 )
      return(0);

    if( ++timeout > TIMEOUT_DRAM ){
      printf("Timeout!!\n");
      WriteSOC_DD( 0x1E6E0070, 0x00000000 );

      return(0);
    }
  } while ( !data );
  WriteSOC_DD( 0x1E6E0070, 0x00000000 );

  return(1);
}

/* ------------------------------------------------------------------------- */
int MMCTest()
{
  unsigned int pattern;

  pattern = ReadSOC_DD( 0x1E6E2078 );
  printf("Pattern = %08X : ",pattern);

  WriteSOC_DD(0x1E6E0074, (DRAM_MapAdr | 0x7fffff) );
  WriteSOC_DD(0x1E6E007C, pattern );

  if(!MMCTestBurst(0))    return(0);
  if(!MMCTestBurst(1))    return(0);
  if(!MMCTestBurst(2))    return(0);
  if(!MMCTestBurst(3))    return(0);
  if(!MMCTestBurst(4))    return(0);
  if(!MMCTestBurst(5))    return(0);
  if(!MMCTestBurst(6))    return(0);
  if(!MMCTestBurst(7))    return(0);
  if(!MMCTestSingle(0))   return(0);
  if(!MMCTestSingle(1))   return(0);
  if(!MMCTestSingle(2))   return(0);
  if(!MMCTestSingle(3))   return(0);
  if(!MMCTestSingle(4))   return(0);
  if(!MMCTestSingle(5))   return(0);
  if(!MMCTestSingle(6))   return(0);
  if(!MMCTestSingle(7))   return(0);

  return(1);
}

/* ------------------------------------------------------------------------- */
int dram_stress_function(int argc, char *argv[])
{
    unsigned int Pass;
    unsigned int PassCnt     = 0;
    unsigned int Testcounter = 0;
    int          ret = 1;
    char         *stop_at;

    printf("**************************************************** \n");
    printf("*** ASPEED Stress DRAM                           *** \n");
    printf("***                          20131107 for u-boot *** \n");
    printf("**************************************************** \n");
    printf("\n");

    if ( argc != 2 ){
        ret = 0;
        return ( ret );
    }
    else {
        Testcounter = (unsigned int) strtoul(argv[1], &stop_at, 10);
    }

    WriteSOC_DD(0x1E6E0000, 0xFC600309);

    while( ( Testcounter > PassCnt ) || ( Testcounter == 0 ) ){
        if( !MMCTest() ) {
            printf("FAIL...%d/%d\n", PassCnt, Testcounter);
            ret = 0;

            break;
        }
        else {
            PassCnt++;
            printf("Pass %d/%d\n", PassCnt, Testcounter);
        }
     } // End while()

     return( ret );
}

