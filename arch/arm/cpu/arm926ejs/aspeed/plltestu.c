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
#include "stduboot.h"
#include "typedef.h"
#include "io.h"
#include "plltestu.h"

/*
 * static
 */
static UCHAR jVersion[] = "v.0.57.00";
             
void print_usage( void )
{
    printf(" PLLTest [pll mode] [err rate]\n");
    printf("      [pll mode] h-pll: ARM CPU Clock PLL\n");
    printf("                 m-pll: Memory Clock PLL\n");
    printf("      [err rate] Error Rate: unit %\n");
    printf("                 default is 1%\n");
}             
             
BOOL CompareToRing(_VGAInfo *VGAInfo, ULONG ulPLLMode, ULONG ulDCLK, ULONG ulErrRate)
{
    ULONG ulCounter, ulLowLimit, ulHighLimit;
    ULONG ulData, ulValue, ulDiv;
    ULONG ulSCUBase;
    double del0;
    ULONG uldel0;
    
    if ((VGAInfo->usDeviceID == 0x1160) || (VGAInfo->usDeviceID == 0x1180))
        ulSCUBase = 0x80fc8200;    
    else
        ulSCUBase = 0x1e6e2000;

    //Fixed AST2300 H-PLL can't Get Correct Value in VGA only mode, ycchen@081711
    if ( (VGAInfo->jRevision >= 0x20) && (ulPLLMode == HPLL_PLLMODE_AST2300) )
    {
        WriteSOC_DD(ulSCUBase, 0x1688a8a8);	    
        ulData = ReadSOC_DD(ulSCUBase + 0x08);            
        WriteSOC_DD(ulSCUBase + 0x08, ulData & 0xFFFFFF00);	    
    }
	    	    
    ulCounter = (ulDCLK/1000) * 512 / 24000 - 1;
    ulLowLimit = ulCounter * (100 - ulErrRate) / 100;
    ulHighLimit = ulCounter * (100 + ulErrRate) / 100;

    DELAY(10);
    WriteSOC_DD(ulSCUBase, 0x1688a8a8);
    WriteSOC_DD(ulSCUBase + 0x28, (ulHighLimit << 16) | ulLowLimit);
    WriteSOC_DD(ulSCUBase + 0x10, ulPLLMode);
    WriteSOC_DD(ulSCUBase + 0x10, ulPLLMode | 0x03);
    DELAY(1);
    do {
        ulData = ReadSOC_DD(ulSCUBase + 0x10);            
    } while (!(ulData & 0x40));	
    ulValue = ReadSOC_DD(ulSCUBase + 0x14);            
    
    //Patch for AST1160/1180 DCLK calculate
    if ( ((VGAInfo->usDeviceID == 0x1160) || (VGAInfo->usDeviceID == 0x1180)) && (ulPLLMode == DPLL_PLLMODE_AST1160) )
    {
        ulData = ReadSOC_DD(0x80fc906c);            
    	ulDiv   = ulData & 0x000c0000;
    	ulDiv >>= 18;
    	ulDiv++;
    	ulValue /= ulDiv;
    }
    
    if ( (VGAInfo->jRevision >= 0x20) && (ulPLLMode == DEL0_PLLMODE_AST2300) )
    {
      del0 = (double)(24.0 * (ulValue + 1) / 512.0);
      del0 = 1000/del0/16/8;
      uldel0 = (ULONG) (del0 * 1000000);
      if (uldel0 < ulDCLK)
      {
          printf( "[PASS][DEL0] Actual DEL0:%f ns, Max. DEL0:%f ns \n", del0, (double)ulDCLK/1000000);    		      
          ulData |= 0x80;
      }    
      else
      {
          printf( "[ERROR][DEL0] Actual DEL0:%f ns, Max. DEL0:%f ns \n", del0, (double)ulDCLK/1000000);    		      	      
          ulData == 0x00;    
      }    
    }	
    else
    {
        printf( "[INFO] PLL Predict Count = %x, Actual Count = %x \n", ulCounter, ulValue);	
    }
    
    WriteSOC_DD(ulSCUBase + 0x10, 0x2C);	//disable ring
        
    if (ulData & 0x80)
        return (TRUE);
    else
        return(FALSE);    	
} /* CompareToRing */	

VOID GetDRAMInfo(_VGAInfo *VGAInfo)
{
    ULONG ulData, ulData2;
    ULONG ulRefPLL, ulDeNumerator, ulNumerator, ulDivider, ulOD;

    if (VGAInfo->jRevision >= 0x10)
    {
        WriteSOC_DD(0x1e6e2000, 0x1688A8A8);
	    
        //Get DRAM Bus Width
        ulData  = ReadSOC_DD(0x1e6e0004);        
        if (ulData & 0x40)
           VGAInfo->ulDRAMBusWidth = 16;
        else    
           VGAInfo->ulDRAMBusWidth = 32;    
  
        ulRefPLL = 24000;	            
        if (VGAInfo->jRevision >= 0x30)	//AST2400
        {
            ulData = ReadSOC_DD(0x1e6e2070);
            if (ulData & 0x00800000)	//D[23] = 1
                ulRefPLL = 25000;	            
        }         

        ulData  = ReadSOC_DD(0x1e6e2020);                                
        ulDeNumerator = ulData & 0x0F;
        ulNumerator = (ulData & 0x07E0) >> 5;
        ulOD = (ulData & 0x10) ? 1:2;
            
        ulData = (ulData & 0x7000) >> 12;        
        switch (ulData)
        {
            case 0x07:
                ulDivider = 16;
                break;
            case 0x06:
                ulDivider = 8;
                break;
            case 0x05:
                ulDivider = 4;
                break;
            case 0x04:
                ulDivider = 2;
                break;        
            default:
                ulDivider = 0x01;                         
        }    
    
        VGAInfo->ulMCLK = ulRefPLL * ulOD * (ulNumerator + 2) / ((ulDeNumerator + 1) * ulDivider * 1000);                  	        
    }    
} // GetDRAMInfo

VOID GetCLKInfo( _VGAInfo *VGAInfo)
{
    ULONG ulData, ulCPUTrap, ulAHBTrap;
    ULONG ulRefPLL, ulDeNumerator, ulNumerator, ulDivider, ulOD;

    if (VGAInfo->jRevision >= 0x30)
    {
        WriteSOC_DD(0x1e6e2000, 0x1688a8a8);
        ulData = ReadSOC_DD(0x1e6e2024);            
        if (ulData & 0x40000)		//from H-PLL		
        {
            ulRefPLL = 24000;	            
            ulData = ReadSOC_DD(0x1e6e2070);
            if (ulData & 0x00800000)	//D[23] = 1
                ulRefPLL = 25000;	            
	        
            ulData = ReadSOC_DD(0x1e6e2024);            
                
            ulDeNumerator = ulData & 0x0F;
            ulNumerator = (ulData & 0x07E0) >> 5;
            ulOD = (ulData & 0x10) ? 1:2;                    
            
            VGAInfo->ulCPUCLK = ulRefPLL * ulOD * (ulNumerator + 2) / ((ulDeNumerator + 1) * 1000);          
        	
        }
        else				//from trapping
        {        
             ulRefPLL = 24;	            
             ulData = ReadSOC_DD(0x1e6e2070);
             if (ulData & 0x00800000)	//D[23] = 1
                ulRefPLL = 25; 
                         
             ulCPUTrap = ulData & 0x0300;
             ulCPUTrap >>= 8;
         
             switch (ulCPUTrap)
             {
                case 0x00:
                    VGAInfo->ulCPUCLK = ulRefPLL * 16;
                    break;
                case 0x01:
                    VGAInfo->ulCPUCLK = ulRefPLL * 15;
                    break;
                case 0x02:
                    VGAInfo->ulCPUCLK = ulRefPLL * 14;
                    break;
                case 0x03:
                    VGAInfo->ulCPUCLK = ulRefPLL * 17;
                    break;
             }	
         
        }
        
        ulData = ReadSOC_DD(0x1e6e2070);            
        ulAHBTrap = ulData & 0x0c00;
        ulAHBTrap >>= 10;        
        switch (ulAHBTrap)
        {
            case 0x00:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK;
                break;	
            case 0x01:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK / 2;
                break;	        
            case 0x02:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK / 4;
                break;	        
            case 0x03:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK / 3;
            break;	
        }	
	        
    } //AST2400	    
    else if (VGAInfo->jRevision >= 0x20)
    {
        WriteSOC_DD(0x1e6e2000, 0x1688a8a8);
        ulData = ReadSOC_DD(0x1e6e2024);            
        if (ulData & 0x40000)		//from H-PLL		
        {
            ulRefPLL = 24000;
            
            ulData = ReadSOC_DD(0x1e6e2024);            
                
            ulDeNumerator = ulData & 0x0F;
            ulNumerator = (ulData & 0x07E0) >> 5;
            ulOD = (ulData & 0x10) ? 1:2;                    
            
            VGAInfo->ulCPUCLK = ulRefPLL * ulOD * (ulNumerator + 2) / ((ulDeNumerator + 1) * 1000);          
        	
        }
        else				//from trapping
        {        
             ulData = ReadSOC_DD(0x1e6e2070);            
             ulCPUTrap = ulData & 0x0300;
             ulCPUTrap >>= 8;
         
             switch (ulCPUTrap)
             {
                case 0x00:
                    VGAInfo->ulCPUCLK = 384;
                    break;
                case 0x01:
                    VGAInfo->ulCPUCLK = 360;
                    break;
                case 0x02:
                    VGAInfo->ulCPUCLK = 336;
                    break;
                case 0x03:
                    VGAInfo->ulCPUCLK = 408;
                    break;
             }	
         
        }
        
        ulData = ReadSOC_DD(0x1e6e2070);            
        ulAHBTrap = ulData & 0x0c00;
        ulAHBTrap >>= 10;        
        switch (ulAHBTrap)
        {
            case 0x00:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK;
                break;	
            case 0x01:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK / 2;
                break;	        
            case 0x02:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK / 4;
                break;	        
            case 0x03:
                VGAInfo->ulAHBCLK = VGAInfo->ulCPUCLK / 3;
            break;	
        }	
	        
    } //AST2300    
} // GetCLKInfo

int pll_function(int argc, char *argv[])
{
    _VGAInfo *pVGAInfo;
    ULONG    ulErrRate = 1;
    ULONG    PLLMode;
    ULONG    RefClk;
    CHAR     *stop_at;
    CHAR     i;
    
    printf("**************************************************** \n");       
    printf("*** ASPEED Graphics PLL Test %s Log       *** \n", jVersion);
    printf("***                                   for u-boot *** \n");
    printf("**************************************************** \n"); 
    printf("\n"); 
    
    // Check chip type
    switch ( ReadSOC_DD( 0x1e6e2000 + 0x7c ) ) {   
        case 0x02010303 :
        case 0x02000303 : 
            printf("The chip is AST2400\n" ); 
            pVGAInfo->usDeviceID = 0x2400; 
            pVGAInfo->jRevision  = 0x30;         
            break;
            
        case 0x02010103 :
        case 0x02000003 : 
            printf("The chip is AST1400\n" ); 
            pVGAInfo->usDeviceID = 0x1400; 
            pVGAInfo->jRevision  = 0x30;   
            break;
            
        case 0x01010303 :
        case 0x01000003 : 
            printf("The chip is AST2300\n" ); 
            pVGAInfo->usDeviceID = 0x2300; 
            pVGAInfo->jRevision  = 0x20;   
            break;
        
        case 0x01010203 : 
            printf("The chip is AST1050\n" ); 
            pVGAInfo->usDeviceID = 0x1050; 
            pVGAInfo->jRevision  = 0x20;   
            break;
                    
        default     : 
            printf ("Error Silicon Revision ID(SCU7C) %08lx!!!\n", ReadSOC_DD( 0x1e6e2000 + 0x7c ) ); 
            return(1);
    }
    
    
    GetDRAMInfo( pVGAInfo );
    GetCLKInfo( pVGAInfo );

    if ( ( argc <= 1 ) || ( argc >= 4 ) ){
        print_usage();
        return (ERR_FATAL);
    }
    else {
        for ( i = 1; i < argc; i++ ) {
            switch ( i ) {
                case 1:
                    if (!strcmp(argv[i], "m-pll"))
                    {
                    	if (pVGAInfo->jRevision >= 0x20)
                    	    PLLMode = MPLL_PLLMODE_AST2300;
                    	else
                    	    PLLMode = MPLL_PLLMODE;
                    	
                    	RefClk = pVGAInfo->ulMCLK * 1000000;
                    	if (pVGAInfo->jRevision >= 0x20)	//dual-edge
                    	    RefClk /= 2;            	
                    }
                    else if (!strcmp(argv[i], "h-pll"))
                    {
           	            if (pVGAInfo->jRevision >= 0x20)
                            PLLMode = HPLL_PLLMODE_AST2300;
                        else
                            PLLMode = HPLL_PLLMODE;
                        
                        //AST2300 only has HCLK ring test mode, ycchen@040512
                        RefClk = pVGAInfo->ulCPUCLK * 1000000;	//Other  : H-PLL
                        if (pVGAInfo->jRevision >= 0x20)	//AST2300: HCLK
                            RefClk = pVGAInfo->ulAHBCLK * 1000000;
                    }
                    else {
                        print_usage();
                        return (ERR_FATAL);
                    }
                    break;
                case 2:
                    ulErrRate = (ULONG) strtoul(argv[i], &stop_at, 10);	                    

                    break;
                default: 
                    break;
            } // End switch()
        } // End for
    } 
    
    /* Compare ring */
    if (CompareToRing(pVGAInfo, PLLMode, RefClk, ulErrRate ) == TRUE)
    {
	    printf("[PASS] %s PLL Check Pass!! \n", argv[1]);
        return 0;
    }
    else
    {
	    printf("[ERROR] %s PLL Check Failed!! \n", argv[1]);	
        return (ERR_FATAL);
    }
}
