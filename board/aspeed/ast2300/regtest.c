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

#if ((CFG_CMD_SLT & CFG_CMD_REGTEST) && defined(CONFIG_SLT))
#include "regtest.h"

int do_regtest (void)
{
	_SOCRegTestInfo	*pjSOCRegInfo;
	_SOCRegTestTbl *pjRegTable;
	unsigned long ulRegBase;
        unsigned long ulIndex, ulBack, ulAND, ulMask, ulData, ulTemp;
        unsigned long Flags = 0;
	
	/* unlock reg */
        *(unsigned long *) (0x1e600000) = 0xaeed1a03;	/* AHBC */
        *(unsigned long *) (0x1e6e0000) = 0xfc600309;	/* MMC */
        *(unsigned long *) (0x1e6e2000) = 0x1688a8a8;	/* SCU */

        /* SCU */

        /* do test */	
        pjSOCRegInfo = SOCRegTestInfo;
        while (strcmp(pjSOCRegInfo->jName, "END"))
        {
            /* Reg. Test Start */
            ulRegBase = pjSOCRegInfo->ulRegOffset;
            pjRegTable = pjSOCRegInfo->pjTblIndex;
            
            while (pjRegTable->ulIndex != 0xFFFFFFFF)
            {
                ulIndex = ulRegBase + pjRegTable->ulIndex;	

                ulBack = *(unsigned long *) (ulIndex);

                ulMask  = pjRegTable->ulMask;
                ulAND  = ~pjRegTable->ulMask;
       	        	    	
                ulData  = 0xFFFFFFFF & pjRegTable->ulMask;
                *(unsigned long *) (ulIndex) = ulData;                
                ulTemp = *(volatile unsigned long *) (ulIndex) & pjRegTable->ulMask;
                if (ulData != ulTemp)
                {
                    Flags |= pjSOCRegInfo->ulFlags;
                    printf("[DBG] RegTest: Failed Index:%x, Data:%x, Temp:%x \n", ulIndex, ulData, ulTemp);
                }
                
                ulData  = 0x00000000 & pjRegTable->ulMask;
                *(unsigned long *) (ulIndex) = ulData;                
                ulTemp = *(volatile unsigned long *) (ulIndex) & pjRegTable->ulMask;
                if (ulData != ulTemp)
                {
                    Flags |= pjSOCRegInfo->ulFlags;
                    printf("[DBG] RegTest: Failed Index:%x, Data:%x, Temp:%x \n", ulIndex, ulData, ulTemp);
                }
                             
                *(unsigned long *) (ulIndex) = ulBack;
       
                pjRegTable++;
          	
           } /* Individual Reg. Test */

           if (Flags & pjSOCRegInfo->ulFlags)
               printf("[INFO] RegTest: %s Failed \n", pjSOCRegInfo->jName);
           
           pjSOCRegInfo++;
           
        } /* Reg. Test */

	return Flags;
	
}

#endif /* CONFIG_SLT */
