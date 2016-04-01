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
static const char ThisFile[] = "stress.c";

#include "SWFUNC.H"
#include "COMMINF.H"
#include "IO.H"

#define TIMEOUT_DRAM              5000000
#define MAX_DRAM_SIZE             0x10000000
#define MAX_TEST_LENGTH           0x00800000
#define MAX_TEST_OFFSET           (MAX_TEST_LENGTH - 1)

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
static int DoMMCTests(unsigned int offset, unsigned int length,\
                    unsigned int pattern)
{
  unsigned int size, address, i, limit;
  //validate if offset is out of 256 MB range.
  if( offset >= MAX_DRAM_SIZE )
    offset = 0;
  //validate length
  if( (offset + length) >= MAX_DRAM_SIZE )
    length = MAX_DRAM_SIZE - offset;
  //mask offset fields to make it 8MB boundary
  length += (offset % MAX_TEST_LENGTH);
  offset &= (~MAX_TEST_OFFSET);
  printf("DRAM Test offset = 0x%08X length = 0x%08X pattern = 0x%08X ",\
         offset, length, pattern);
  WriteSOC_DD(0x1E6E007C, pattern );
  for( size = 0; size < length; size += MAX_TEST_LENGTH ) {
    limit = ((length - size) < MAX_TEST_LENGTH) ? (length - size - 1) :\
                              MAX_TEST_OFFSET;
    //Make limit 8 bytes boundary
    limit &= (~0x00000007);
    address = (DRAM_MapAdr + offset + size) | limit;
    WriteSOC_DD(0x1E6E0074, address);
    for( i = 0; i < 8; i++ ) {
      if( !MMCTestBurst(i) ) {
        printf("Failed Burst Test at offset = 0x%08X datagen = %d\n",size, i);
        return(0);
      }
    }
    for( i = 0; i < 8; i++ ) {
      if( !MMCTestSingle(i) ) {
        printf("Failed Single Test at offset = 0x%08X datagen = %d\n",\
               size, i);
        return(0);
      }
    }
  }
  return(1);
}
static void print_usage( void )
{
   printf(" dramtest tests [offset] [length] [pattern]\n");
   printf("      tests:    number of tests\n");
   printf("      [offset]  offset within DRAM range in Hexadecimal\n");
   printf("                default is 0\n");
   printf("      [length]  length of DRAM in Hexadecimal\n");
   printf("                default is 8MB\n");
   printf("      [pattern] data pattern in Hexadecimal\n");
}
/* ------------------------------------------------------------------------- */
int dram_stress_function(int argc, char *argv[])
{
    unsigned int Pass;
    unsigned int PassCnt     = 0;
    unsigned int Testcounter = 0;
    int          ret = 1;
    char         *stop_at;
    unsigned int offset = 0;
    unsigned int pattern = ReadSOC_DD( 0x1E6E2078 );
    unsigned int length = 0x800000;

    printf("**************************************************** \n");       
    printf("*** ASPEED Stress DRAM                           *** \n");
    printf("***                          20131107 for u-boot *** \n");
    printf("**************************************************** \n"); 
    printf("\n"); 
  
    if ( argc < 2 || argc > 5 ){
        print_usage();
        ret = 0;
        return ( ret );
    }
    else {
        switch(argc){
          case 5:
              pattern = (unsigned int) strtoul(argv[4], &stop_at, 16);
          case 4:
              length = (unsigned int) strtoul(argv[3], &stop_at, 16);
          case 3:
              offset = (unsigned int) strtoul(argv[2], &stop_at, 16);
          case 2:
        Testcounter = (unsigned int) strtoul(argv[1], &stop_at, 10);                     
              break;
          default:
              break;
    }

    }
    WriteSOC_DD(0x1E6E0000, 0xFC600309);   

    while( ( Testcounter > PassCnt ) || ( Testcounter == 0 ) ){
        if( !DoMMCTests(offset, length, pattern) ) {
            printf("FAIL...%d/%d\n", PassCnt, Testcounter);
            ret = 0;
            break;
        }
        else {
            PassCnt++;
            printf("Pass %d/%d\n", PassCnt, Testcounter);
        }
     } // End while()
     //Lock MMC registers
     WriteSOC_DD(0x1E6E0000, 0x00000000);
     return( ret );
}
