/*
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Diagnostics support
 */
#include <common.h>
#include <command.h>
#include <post.h>
#include "slt.h"

#if ((CFG_CMD_SLT & CFG_CMD_MICTEST) && defined(CONFIG_SLT))
#include "mictest.h"

static unsigned char ctrlbuf[MIC_MAX_CTRL];
static unsigned char chksumbuf[MIC_MAX_CHKSUM];

void vInitSCU(void)
{
    unsigned long ulData;

    *(unsigned long *) (0x1e6e2000) = 0x1688A8A8;

    udelay(100);

    ulData = *(unsigned long *) (0x1e6e2004);
    ulData &= 0xbffff;
    *(unsigned long *) (0x1e6e2004) = ulData;
         	
}

void vInitMIC(void)
{
    unsigned long i, j, ulPageNumber;
    unsigned char *pjctrl, *pjsum;
    
    ulPageNumber = DRAMSIZE >> 12;	

    pjctrl = (unsigned char *)(m16byteAlignment((unsigned long) ctrlbuf));
    pjsum = (unsigned char *)(m16byteAlignment((unsigned long) chksumbuf));
    
    /* init ctrl buffer (2bits for one page) */
    for (i=0; i< (ulPageNumber/4); i++)
        *(unsigned char *) (pjctrl + i) = DEFAULT_CTRL;
      
    /* init chksum buf (4bytes for one page) */
    for (i=0; i<ulPageNumber; i++)
        *(unsigned long *) (pjsum + i*4) = DEFAULT_CHKSUM;
    
    *(unsigned long *) (MIC_BASE + MIC_CTRLBUFF_REG)   = (unsigned long) pjctrl;
    *(unsigned long *) (MIC_BASE + MIC_CHKSUMBUF_REG)  = (unsigned long) pjsum;
    *(unsigned long *) (MIC_BASE + MIC_RATECTRL_REG)   = (unsigned long) DEFAULT_RATE;
    *(unsigned long *) (MIC_BASE + MIC_ENGINECTRL_REG) = MIC_ENABLE_MIC | (DRAMSIZE - 0x1000);
	
}

void vDisableMIC(void)
{
    *(unsigned long *) (MIC_BASE + MIC_ENGINECTRL_REG)   = MIC_RESET_MIC;
	
}

int do_chksum(void)
{
    unsigned long i, j, k, ulPageNumber;
    int Status = 0;
    unsigned short tmp;
    volatile unsigned long sum1, sum2;
    unsigned long goldensum, chksum;
    unsigned long len, tlen;
    unsigned char *pjsum;

    ulPageNumber = DRAMSIZE >> 12;	
    pjsum = (unsigned char *)(m16byteAlignment((unsigned long) chksumbuf));

    /* start test */    
    for (i=0; i<ulPageNumber; i++)
    {
    	
    	sum1 = 0xffff, sum2 = 0xffff;
    	len = 0x0800;
    	j = 0;
    	
    	while (len)
    	{
    	    tlen = len > 360 ? 360 : len;
    	    len -= tlen;
    	    do {
    	       tmp = *(unsigned short *) (DRAM_BASE + ((i << 12) + j));	
               sum1 += (unsigned long) tmp;   	    	
    	       sum2 += sum1;
    	       j+=2;
    	    } while (--tlen);
    	    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    	    sum2 = (sum2 & 0xffff) + (sum2 >> 16);   	        	
    	}
    	
    	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    	
    	goldensum = (sum2 << 16) | sum1;
        k= 0;
        do {
            chksum = *(unsigned long *) (pjsum + i*4);
            udelay(100);
            k++;
    	} while ((chksum == 0) && (k<1000));
    	
    	if (chksum != goldensum)
    	{
    	        Status = 1;	
                printf("[FAIL] MIC Chksum Failed at Page %x \n", i);
    	}

    } /* end of i loop */
		
    return (Status);
    	
}

int do_mictest (void)
{
        unsigned long Flags = 0;

        vInitSCU();
        vInitMIC();
        
        if (do_chksum())
            Flags = 1;
            
        vDisableMIC();
        
	return Flags;
	
}

#endif /* CONFIG_SLT */
