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
typedef struct {
    unsigned long ulIndex;
    unsigned long ulMask;
} _SOCRegTestTbl;

typedef struct {
    unsigned char jName[10];
    unsigned long ulRegOffset;
    _SOCRegTestTbl *pjTblIndex;
    unsigned long ulFlags;
} _SOCRegTestInfo;

_SOCRegTestTbl SMCRegTestTbl[] = {
    {0x00000000, 0x00001FF3},
    {0x00000004, 0xFFFFFFFF},
    {0x00000008, 0x0FFF17FF},
    {0x0000000C, 0xFFFFFFFF},
    {0x00000010, 0xFF5FFFF3},
    {0x00000018, 0x0FFFFFFF},
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl AHBCRegTestTbl[] = {
    {0x00000080, 0x0000FFFE},        	
    {0x00000088, 0x01000000},
    {0x0000008c, 0x00000031},	
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl MICRegTestTbl[] = {
/*	
    {0x00000000, 0x0FFFFFF8},
    {0x00000004, 0x0FFFFFF8},
    {0x00000008, 0x0000FFFF},
    {0x0000000C, 0x0FFFF000},
    {0x00000010, 0xFFFFFFFF},
*/
    {0xFFFFFFFF, 0xFFFFFFFF},
};

_SOCRegTestTbl MAC1RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl MAC2RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl USB2RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl VICRegTestTbl[] = {
    {0x0000000C, 0xFFFFFFFF},
    {0x00000024, 0xFFFFFFFF},
    {0x00000028, 0xFFFFFFFF},
    {0x0000002C, 0xFFFFFFFF},
    {0xFFFFFFFF, 0xFFFFFFFF},
};

_SOCRegTestTbl MMCRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl USB11RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl SCURegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl HASERegTestTbl[] = {
    {0x00000000, 0x0FFFFFF8},
    {0x00000004, 0x0FFFFFF8},
    {0x00000008, 0x0FFFFFF8},
    {0x0000000C, 0x0FFFFFF8},
    //{0x00000010, 0x00001FFF},
    {0x00000020, 0x0FFFFFF8},
    {0x00000024, 0x0FFFFFF8},
    {0x00000028, 0x0FFFFFc0},
    {0x0000002C, 0x0FFFFFFF},
    //{0x00000030, 0x000003FF},
    {0xFFFFFFFF, 0xFFFFFFFF},
};

_SOCRegTestTbl I2SRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl CRTRegTestTbl[] = {
/*	
    {0x00000000, 0x001F3703},
    {0x00000004, 0x0000FFC1},
*/    
    {0x00000010, 0x0FFF0FFF},
    {0x00000014, 0x0FFF0FFF},
    {0x00000018, 0x07FF07FF},
    {0x0000001C, 0x07FF07FF},
    {0x00000020, 0x0FFFFFF8},
    {0x00000024, 0x07FF3FF8},
/*    
    {0x00000028, 0x003F003F},
    {0x00000030, 0x003F003F},
    {0x00000034, 0x0FFF0FFF},
    {0x00000038, 0x0FFFFFF8},
*/    
    {0x00000040, 0x0FFF0FFF},
    {0x00000044, 0x07FF07FF},
    {0x00000048, 0x0FFFFFF8},
    {0x0000004C, 0x00FF07F8},
    {0x00000050, 0x000F0F0F},
/*    
    {0x00000060, 0x001F3703},
    {0x00000064, 0x0000FFC1},
*/    
    {0x00000070, 0x0FFF0FFF},
    {0x00000074, 0x0FFF0FFF},
    {0x00000078, 0x07FF07FF},
    {0x0000007C, 0x07FF07FF},
    {0x00000080, 0x0FFFFFF8},
    {0x00000084, 0x07FF3FF8},
/*    
    {0x00000088, 0x003F003F},
    {0x00000090, 0x003F003F},
    {0x00000094, 0x0FFF0FFF},
    {0x00000098, 0x0FFFFFF8},
*/    
    {0x000000A0, 0x0FFF0FFF},
    {0x000000A4, 0x07FF07FF},
    {0x000000A8, 0x0FFFFFF8},
    {0x000000AC, 0x00FF07F8},
    {0x000000B0, 0x000F0F0F},
    {0xFFFFFFFF, 0xFFFFFFFF},
};

_SOCRegTestTbl VIDEORegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl A2PRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl MDMARegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl M2DRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl GPIORegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl RTCRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl TIMERRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl UART1RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl UART2RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl WDTRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl PWMRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl VUART1RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl VUART2RegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl LPCRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl I2CRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl PECIRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl PCIARegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};

_SOCRegTestTbl PCIRegTestTbl[] = {
    {0xFFFFFFFF, 0xFFFFFFFF},        	
};


/* Test List */
_SOCRegTestInfo SOCRegTestInfo[] = {
    /* Test Name, Reg. Offset,      Test Table, Error Code  */
    {   "SMCREG", 0x16000000,    SMCRegTestTbl, 0x00000001},
    {  "AHBCREG", 0x1e600000,   AHBCRegTestTbl, 0x00000002},
    {   "MICREG", 0x1e640000,    MICRegTestTbl, 0x00000004},
    {  "MAC1REG", 0x1e660000,   MAC1RegTestTbl, 0x00000008},
    {  "MAC2REG", 0x1e680000,   MAC2RegTestTbl, 0x00000010},
    {  "USB2REG", 0x1e6a0000,   USB2RegTestTbl, 0x00000020},
    {   "VICREG", 0x1e6c0000,    VICRegTestTbl, 0x00000040},
    {   "MMCREG", 0x1e6e0000,    MMCRegTestTbl, 0x00000080},
    { "USB11REG", 0x1e6e1000,  USB11RegTestTbl, 0x00000100},
    {   "SCUREG", 0x1e6e2000,    SCURegTestTbl, 0x00000200},
    {  "HASEREG", 0x1e6e3000,   HASERegTestTbl, 0x00000400},
    {   "I2SREG", 0x1e6e5000,    I2SRegTestTbl, 0x00000800},
    {   "CRTREG", 0x1e6e6000,    CRTRegTestTbl, 0x00001000},
    { "VIDEOREG", 0x1e700000,  VIDEORegTestTbl, 0x00002000},
    {   "A2PREG", 0x1e720000,    A2PRegTestTbl, 0x00004000},
    {  "MDMAREG", 0x1e740000,   MDMARegTestTbl, 0x00008000},
    {    "2DREG", 0x1e760000,    M2DRegTestTbl, 0x00010000},
    {  "GPIOREG", 0x1e780000,   GPIORegTestTbl, 0x00020000},
    {   "RTCREG", 0x1e781000,    RTCRegTestTbl, 0x00040000},
    { "TIMERREG", 0x1e782000,  TIMERRegTestTbl, 0x00080000},
    { "UART1REG", 0x1e783000,  UART1RegTestTbl, 0x00100000},
    { "UART2REG", 0x1e784000,  UART2RegTestTbl, 0x00200000},
    {   "WDTREG", 0x1e785000,    WDTRegTestTbl, 0x00400000},
    {   "PWMREG", 0x1e786000,    PWMRegTestTbl, 0x00800000},
    {"VUART1REG", 0x1e787000, VUART1RegTestTbl, 0x01000000},
    {"VUART2REG", 0x1e788000, VUART2RegTestTbl, 0x02000000},
    {   "LPCREG", 0x1e789000,    LPCRegTestTbl, 0x04000000},
    {   "I2CREG", 0x1e78A000,    I2CRegTestTbl, 0x08000000},
    {  "PECIREG", 0x1e78B000,   PECIRegTestTbl, 0x10000000},
    {  "PCIAREG", 0x1e78C000,   PCIARegTestTbl, 0x20000000},
    {   "PCIREG", 0x60000000,    PCIRegTestTbl, 0x40000000},        
    {      "END", 0xffffffff,             NULL, 0xffffffff}
};
