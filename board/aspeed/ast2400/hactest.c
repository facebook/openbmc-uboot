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

#if ((CFG_CMD_SLT & CFG_CMD_HACTEST) && defined(CONFIG_SLT))
#include "hactest.h"

#include "aes.c"
#include "rc4.c"

static unsigned char crypto_src[CRYPTO_MAX_SRC], crypto_dst[CRYPTO_MAX_DST], crypto_context[CRYPTO_MAX_CONTEXT];
static unsigned char hash_src[HASH_MAX_SRC], hash_dst[HASH_MAX_DST], hmac_key[HMAC_MAX_KEY];

/*
 * table 
 */
static aes_test aestest[] = {
    { CRYPTOMODE_ECB, 128,
     {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c, '\0'},
     {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34, '\0'},
     {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32, '\0'} },
    {0xFF, 0xFF, "", "", ""},		/* End Mark */
};

static rc4_test rc4test[] = {
    {{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, '\0'},
     {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, '\0'}}, 	
    {{0xff}, {0xff}},				/* End Mark */	
};

static hash_test hashtest[] = {
    {HASHMODE_SHA1, 20,
     "abc",
     {0x53, 0x20, 0xb0, 0x8c, 0xa1, 0xf5, 0x74, 0x62, 0x50, 0x71, 0x89, 0x41, 0xc5, 0x0a, 0xdf, 0x4e, 0xbb, 0x55, 0x76, 0x06, '\0'}},
    {0xFF, 0xFF, "", ""},			/* End Mark */ 
};

static hmac_test hmactest[] = {
    {HASHMODE_SHA1, 64, 20,
     {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, '\0' },
     "Sample #1",
     {0xbf, 0x39, 0xda, 0xb1, 0x7d, 0xc2, 0xe1, 0x23, 0x0d, 0x28, 0x35, 0x3b, 0x8c, 0xcb, 0x14, 0xb6, 0x22, 0x02, 0x65, 0xb3, '\0'}},
    {0xFF, 0xFF, 0xFF, "", "", ""},			/* End Mark */ 
};

void EnableHMAC(void)
{
    unsigned long ulData;
    	
    /* init SCU */
    *(unsigned long *) (0x1e6e2000) = 0x1688a8a8;
    
    ulData = *(volatile unsigned long *) (0x1e6e200c);
    ulData &= 0xfdfff;
    *(unsigned long *) (0x1e6e200c) = ulData;
    udelay(100);
    ulData = *(volatile unsigned long *) (0x1e6e2004);
    ulData &= 0xfffef;
    *(unsigned long *) (0x1e6e2004) = ulData;
    
}

/* AES */
void aes_enc_ast3000(aes_context *ctx, uint8 *input, uint8 *iv, uint8 *output, uint32 ulMsgLength , uint32 ulAESMode)
{

    unsigned long i, ulTemp, ulCommand;
    unsigned char ch;
    unsigned char *pjsrc, *pjdst, *pjcontext;
    
    ulCommand = CRYPTO_ENABLE_RW | CRYPTO_ENABLE_CONTEXT_LOAD | CRYPTO_ENABLE_CONTEXT_SAVE | \
                CRYPTO_AES | CRYPTO_ENCRYPTO | CRYPTO_SYNC_MODE_ASYNC;

    switch (ctx->nr)
    {
    case 10:
        ulCommand |= CRYPTO_AES128;
        break;
    case 12:
        ulCommand |= CRYPTO_AES192;
        break;    
    case 14:
        ulCommand |= CRYPTO_AES256;
        break;    
    }

    switch (ulAESMode)
    {
    case CRYPTOMODE_ECB:
        ulCommand |= CRYPTO_AES_ECB;
        break;
    case CRYPTOMODE_CBC:
        ulCommand |= CRYPTO_AES_CBC;
        break;
    case CRYPTOMODE_CFB:
        ulCommand |= CRYPTO_AES_CFB;
        break;
    case CRYPTOMODE_OFB:
        ulCommand |= CRYPTO_AES_OFB;
        break;
    case CRYPTOMODE_CTR:
        ulCommand |= CRYPTO_AES_CTR;
        break;                
    }

    pjsrc = (unsigned char *) m16byteAlignment((unsigned long) crypto_src);
    pjdst = (unsigned char *) m16byteAlignment((unsigned long) crypto_dst);
    pjcontext = (unsigned char *) m16byteAlignment((unsigned long) crypto_context);
    	
    /* Init HW */
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_SRC_BASE_OFFSET) = (unsigned long) pjsrc;
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_DST_BASE_OFFSET) = (unsigned long) pjdst;
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_CONTEXT_BASE_OFFSET) = (unsigned long) pjcontext;
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_LEN_OFFSET) = ulMsgLength;

    /* Set source */
    for (i=0; i< ulMsgLength; i++)
    {
    	ch = *(uint8 *)(input + i);    	
    	*(uint8 *) (pjsrc + i) = ch;
    }
     
    /* Set Context */
    /* Set IV */
    for (i=0; i<16; i++)
    {
        ch = *(uint8 *) (iv + i);
    	*(uint8 *) (pjcontext + i) = ch;
    }
       
    /* Set Expansion Key */   
    for (i=0; i<(4*(ctx->nr+1)); i++)
    {
        ulTemp = ((ctx->erk[i] & 0xFF) << 24) + ((ctx->erk[i] & 0xFF00) << 8) + ((ctx->erk[i] & 0xFF0000) >> 8) + ((ctx->erk[i] & 0xFF000000) >> 24);
    	*(uint32 *) (pjcontext + i*4 + 16) = ulTemp;
    }

    /* fire cmd */
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_CMD_BASE_OFFSET) = ulCommand;
    do {
        ulTemp = *(volatile unsigned long *) (HAC_REG_BASE + REG_CRYPTO_STATUS_OFFSET);
    } while (ulTemp & CRYPTO_BUSY);	
    
    /* Output */
    for (i=0; i<ulMsgLength; i++)
    {
        ch = *(uint8 *) (pjdst + i);        
        *(uint8 *) (output + i) = ch;    
    }
    
} /* aes_enc_ast3000 */


void aes_dec_ast3000(aes_context *ctx, uint8 *input, uint8 *iv, uint8 *output, uint32 ulMsgLength , uint32 ulAESMode)
{
    unsigned long i, ulTemp, ulCommand;
    unsigned char ch;
    unsigned char *pjsrc, *pjdst, *pjcontext;
    
    ulCommand = CRYPTO_ENABLE_RW | CRYPTO_ENABLE_CONTEXT_LOAD | CRYPTO_ENABLE_CONTEXT_SAVE | \
                CRYPTO_AES | CRYPTO_DECRYPTO | CRYPTO_SYNC_MODE_ASYNC;

    switch (ctx->nr)
    {
    case 10:
        ulCommand |= CRYPTO_AES128;
        break;
    case 12:
        ulCommand |= CRYPTO_AES192;
        break;    
    case 14:
        ulCommand |= CRYPTO_AES256;
        break;    
    }

    switch (ulAESMode)
    {
    case CRYPTOMODE_ECB:
        ulCommand |= CRYPTO_AES_ECB;
        break;
    case CRYPTOMODE_CBC:
        ulCommand |= CRYPTO_AES_CBC;
        break;
    case CRYPTOMODE_CFB:
        ulCommand |= CRYPTO_AES_CFB;
        break;
    case CRYPTOMODE_OFB:
        ulCommand |= CRYPTO_AES_OFB;
        break;
    case CRYPTOMODE_CTR:
        ulCommand |= CRYPTO_AES_CTR;
        break;                
    }

    pjsrc = (unsigned char *) m16byteAlignment((unsigned long) crypto_src);
    pjdst = (unsigned char *) m16byteAlignment((unsigned long) crypto_dst);
    pjcontext = (unsigned char *) m16byteAlignment((unsigned long) crypto_context);
	
    /* Init HW */
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_SRC_BASE_OFFSET) = (unsigned long) pjsrc;
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_DST_BASE_OFFSET) = (unsigned long) pjdst;
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_CONTEXT_BASE_OFFSET) = (unsigned long) pjcontext;
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_LEN_OFFSET) = ulMsgLength;

    /* Set source */
    for (i=0; i< ulMsgLength; i++)
    {
    	ch = *(uint8 *)(input + i);    	
    	*(uint8 *) (pjsrc + i) = ch;
    }
     
    /* Set Context */
    /* Set IV */
    for (i=0; i<16; i++)
    {
        ch = *(uint8 *) (iv + i);
    	*(uint8 *) (pjcontext + i) = ch;
    }
       
    /* Set Expansion Key */   
    for (i=0; i<(4*(ctx->nr+1)); i++)
    {
        ulTemp = ((ctx->erk[i] & 0xFF) << 24) + ((ctx->erk[i] & 0xFF00) << 8) + ((ctx->erk[i] & 0xFF0000) >> 8) + ((ctx->erk[i] & 0xFF000000) >> 24);
    	*(uint32 *) (pjcontext + i*4 + 16) = ulTemp;
    }

    /* fire cmd */
    *(unsigned long *) (HAC_REG_BASE + REG_CRYPTO_CMD_BASE_OFFSET) = ulCommand;
    do {
        ulTemp = *(volatile unsigned long *) (HAC_REG_BASE + REG_CRYPTO_STATUS_OFFSET);
    } while (ulTemp & CRYPTO_BUSY);	
    
    /* Output */
    for (i=0; i<ulMsgLength; i++)
    {
        ch = *(uint8 *) (pjdst + i);        
        *(uint8 *) (output + i) = ch;    
    }

} /* aes_dec_ast3000 */

void rc4_crypt_ast3000(uint8 *data, int ulMsgLength, uint8 *rc4_key, uint32 ulKeyLength)
{
    struct rc4_state s;
    unsigned long i, ulTemp, ulCommand;
    unsigned char ch;
    unsigned char *pjsrc, *pjdst, *pjcontext;

    ulCommand = CRYPTO_ENABLE_RW | CRYPTO_ENABLE_CONTEXT_LOAD | CRYPTO_ENABLE_CONTEXT_SAVE | \
                CRYPTO_RC4 | CRYPTO_SYNC_MODE_ASYNC;

    rc4_setup( &s, rc4_key, ulKeyLength );

    pjsrc = (unsigned char *) m16byteAlignment((unsigned long) crypto_src);
    pjdst = (unsigned char *) m16byteAlignment((unsigned long) crypto_dst);
    pjcontext = (unsigned char *) m16byteAlignment((unsigned long) crypto_context);
	
    /* Init HW */
    *(uint32 *) (HAC_REG_BASE + REG_CRYPTO_SRC_BASE_OFFSET) = (unsigned long) pjsrc;	
    *(uint32 *) (HAC_REG_BASE + REG_CRYPTO_DST_BASE_OFFSET) = (unsigned long) pjdst;	
    *(uint32 *) (HAC_REG_BASE + REG_CRYPTO_CONTEXT_BASE_OFFSET) = (unsigned long) pjcontext;	
    *(uint32 *) (HAC_REG_BASE + REG_CRYPTO_LEN_OFFSET) = ulMsgLength;	
          	

    /* Set source */
    for (i=0; i< ulMsgLength; i++)
    {
    	ch = *(uint8 *)(data + i);
    	*(uint8 *) (pjsrc + i) = ch;
    }
     
    /* Set Context */
    /* Set i, j */
    *(uint32 *) (pjcontext + 8) = 0x0001;	
       
    /* Set Expansion Key */   
    for (i=0; i<(256/4); i++)
    {
       ulTemp = (s.m[i * 4] & 0xFF) + ((s.m[i * 4 + 1] & 0xFF) << 8) + ((s.m[i * 4 + 2] & 0xFF) << 16) + ((s.m[i * 4+ 3] & 0xFF) << 24);
       *(uint32 *) (pjcontext + i*4 + 16) = ulTemp;	
    }

    /* fire cmd */
    *(uint32 *) (HAC_REG_BASE + REG_CRYPTO_CMD_BASE_OFFSET) = ulCommand;    
    do {
    	ulTemp = *(volatile uint32 *) (HAC_REG_BASE + REG_CRYPTO_STATUS_OFFSET);
    } while (ulTemp & CRYPTO_BUSY);	

    /* Output */
    for (i=0; i<ulMsgLength; i++)
    {
        ch = *(volatile uint8 *) (pjdst + i);
        *(uint8 *) (data + i) = ch;    
    }

} /* rc4_crypt_ast3000	*/

/* Hash */
void hash_ast3000(uint8 *msg, uint32 ulLength, unsigned char *output, uint32 ulHashMode)
{
   uint32 i, ulTemp, ulCommand, ulDigestLength, ulMyMsgLength;
   uint8 ch;
   unsigned char *pjsrc, *pjdst;
   
   /* Get Info */
   switch (ulHashMode)
   {
   case HASHMODE_MD5:
        ulCommand = HASH_ALG_SELECT_MD5;
        ulDigestLength = 16;
       break;
   case HASHMODE_SHA1:
        ulCommand = HASH_ALG_SELECT_SHA1 | 0x08;
        ulDigestLength = 20;
       break;
   case HASHMODE_SHA256:
        ulCommand = HASH_ALG_SELECT_SHA256 | 0x08;
        ulDigestLength = 32;
       break;
   case HASHMODE_SHA224:
        ulCommand = HASH_ALG_SELECT_SHA224 | 0x08;
        ulDigestLength = 28;
       break;       	
   }   	    

    pjsrc = (unsigned char *) m16byteAlignment((unsigned long) hash_src);
    pjdst = (unsigned char *) m16byteAlignment((unsigned long) hash_dst);

    /* 16byte alignment */
    ulMyMsgLength = m16byteAlignment(ulLength);
   	
   /* Init. HW */
    *(uint32 *) (HAC_REG_BASE + REG_HASH_SRC_BASE_OFFSET) = (unsigned long) pjsrc;	
    *(uint32 *) (HAC_REG_BASE + REG_HASH_DST_BASE_OFFSET) = (unsigned long) pjdst;	
    *(uint32 *) (HAC_REG_BASE + REG_HASH_LEN_OFFSET) = ulMyMsgLength;	
        
    /* write src */
    for (i=0; i<ulLength; i++)
    {
    	ch = *(uint8 *)(msg+i);
    	*(uint8 *) (pjsrc + i) = ch;
    }    
    for (i=ulLength; i<ulMyMsgLength; i++)
        *(uint8 *) (pjsrc + i) = 0;
        
    /* fire cmd */
    *(uint32 *) (HAC_REG_BASE + REG_HASH_CMD_OFFSET) = ulCommand;	
    
    /* get digest */
    do {
    	ulTemp = *(volatile uint32 *) (HAC_REG_BASE + REG_HASH_STATUS_OFFSET);    	
    } while (ulTemp & HASH_BUSY);	
    	
    for (i=0; i<ulDigestLength; i++)
    {
        ch = *(volatile uint8 *) (pjdst + i);
       *(uint8 *) (output + i) = ch;    
    }        
 
} /* hash_ast3000 */	

/* HMAC */
void hmackey_ast3000(uint8 *key, uint32 ulKeyLength, uint32 ulHashMode)
{
    uint32 i, ulBlkLength, ulDigestLength, ulTemp, ulCommand;
    uint8 k0[64], sum[32];
    uint8 ch;
    unsigned char *pjsrc, *pjdst, *pjkey;

   /* Get Info */
   switch (ulHashMode)
   {
   case HASHMODE_MD5:
        ulCommand = HASH_ALG_SELECT_MD5;
        ulDigestLength = 16;
       break;
   case HASHMODE_SHA1:
        ulCommand = HASH_ALG_SELECT_SHA1 | 0x08;
        ulDigestLength = 20;
       break;
   case HASHMODE_SHA256:
        ulCommand = HASH_ALG_SELECT_SHA256 | 0x08;
        ulDigestLength = 32;
       break;
   case HASHMODE_SHA224:
        ulCommand = HASH_ALG_SELECT_SHA224 | 0x08;
        ulDigestLength = 28;
       break;       	
   }   	    
   ulBlkLength = 64;				/* MD5, SHA1/256/224: 64bytes */

    /* Init */
    memset( (void *) k0, 0, 64);		/* reset to zero */
    memset( (void *) sum, 0, 32);		/* reset to zero */

    /* Get k0 */
    if (ulKeyLength <= ulBlkLength)
        memcpy( (void *) k0, (void *) key, ulKeyLength );       
    else /* (ulKeyLength > ulBlkLength) */
    {
        hash_ast3000(key, ulKeyLength, sum, ulHashMode);
        memcpy( (void *) k0, (void *) sum, ulDigestLength );                           
    }    	

    pjsrc = (unsigned char *) m16byteAlignment((unsigned long) hash_src);
    pjdst = (unsigned char *) m16byteAlignment((unsigned long) hash_dst);
    pjkey = (unsigned char *) m64byteAlignment((unsigned long) hmac_key);
    
    /* Calculate digest */
    *(uint32 *) (HAC_REG_BASE + REG_HASH_SRC_BASE_OFFSET) = (unsigned long) pjsrc;
    *(uint32 *) (HAC_REG_BASE + REG_HASH_DST_BASE_OFFSET) = (unsigned long) pjdst;
    *(uint32 *) (HAC_REG_BASE + REG_HASH_KEY_BASE_OFFSET) = (unsigned long) pjkey;
    *(uint32 *) (HAC_REG_BASE + REG_HASH_LEN_OFFSET) = ulBlkLength;
        
    /* write key to src */
    for (i=0; i<ulBlkLength; i++)
    {
    	ch = *(uint8 *)(k0+i);
        *(uint8 *) (pjsrc + i) = ch;
    }    
    
    /* fire cmd for calculate */
    *(uint32 *) (HAC_REG_BASE + REG_HASH_CMD_OFFSET) = ulCommand | HAC_DIGEST_CAL_ENABLE;    
    do {
    	ulTemp = *(volatile uint32 *) (HAC_REG_BASE + REG_HASH_STATUS_OFFSET);    	
    } while (ulTemp & HASH_BUSY);	
         
} /* hmackey_ast3000 */

void hmac_ast3000(uint8 *key, uint32 ulKeyLength, uint8 *msg, uint32 ulMsgLength, uint32 ulHashMode, unsigned char *output)
{
    uint32 i, ulTemp, ulCommand, ulDigestLength, ulMyMsgLength;;
    uint8 ch;
    unsigned char *pjsrc, *pjdst, *pjkey;

    /* Calculate digest */
   switch (ulHashMode)
   {
   case HASHMODE_MD5:
        ulCommand = HASH_ALG_SELECT_MD5;
        ulDigestLength = 16;
       break;
   case HASHMODE_SHA1:
        ulCommand = HASH_ALG_SELECT_SHA1 | 0x08;
        ulDigestLength = 20;
       break;
   case HASHMODE_SHA256:
        ulCommand = HASH_ALG_SELECT_SHA256 | 0x08;
        ulDigestLength = 32;
       break;
   case HASHMODE_SHA224:
        ulCommand = HASH_ALG_SELECT_SHA224 | 0x08;
        ulDigestLength = 28;
       break;       	
   }   	    
 
    pjsrc = (unsigned char *) m16byteAlignment((unsigned long) hash_src);
    pjdst = (unsigned char *) m16byteAlignment((unsigned long) hash_dst);
    pjkey = (unsigned char *) m64byteAlignment((unsigned long) hmac_key);
    
    /* 16byte alignment */
    ulMyMsgLength = m16byteAlignment(ulMsgLength);
    		
    /* Init. HW */
    *(uint32 *) (HAC_REG_BASE + REG_HASH_SRC_BASE_OFFSET) = (unsigned long) pjsrc;
    *(uint32 *) (HAC_REG_BASE + REG_HASH_DST_BASE_OFFSET) = (unsigned long) pjdst;
    *(uint32 *) (HAC_REG_BASE + REG_HASH_KEY_BASE_OFFSET) = (unsigned long) pjkey;
    *(uint32 *) (HAC_REG_BASE + REG_HASH_LEN_OFFSET) = ulMyMsgLength;

    /* write Text to src */
    for (i=0; i<ulMsgLength; i++)
    {
    	ch = *(uint8 *)(msg+i);
        *(uint8 *) (pjsrc + i) = ch;
    }    
    for (i=ulMsgLength; i<ulMyMsgLength; i++)
        *(uint8 *) (pjsrc + i) = 0;
    
    /* fire cmd */
    *(uint32 *) (HAC_REG_BASE + REG_HASH_CMD_OFFSET) = ulCommand | HAC_ENABLE;        
    do {
    	ulTemp = *(volatile uint32 *) (HAC_REG_BASE + REG_HASH_STATUS_OFFSET);    	
    } while (ulTemp & HASH_BUSY);

    /* Output Digest */
    for (i=0; i<ulDigestLength; i++)
    {
        ch = *(uint8 *) (pjdst + i);
        *(uint8 *) (output + i) = ch;    
    }

} /* hmac_ast3000 */

/* main hactest procedure */
int do_hactest (void)
{
        unsigned long i, j, Flags = 0;
	aes_test *pjaes_test;
        aes_context aes_ctx;
        unsigned char AES_Mode[8], aes_output[64];
	unsigned long ulAESMsgLength;
	
	rc4_test *pjrc4_test;
	unsigned char rc4_buf_sw[64], rc4_buf_hw[64];
	unsigned long ulRC4KeyLength, ulRC4MsgLength;
	
	hash_test *pjhash_test;
	unsigned char HASH_Mode[8], hash_out[64];

	hmac_test *pjhmac_test;
	unsigned char HMAC_Mode[8], hmac_out[64];

	EnableHMAC();
	
	/* AES Test */
	pjaes_test = aestest;
	while (pjaes_test->aes_mode != 0xFF)
	{

            if (pjaes_test->aes_mode == CRYPTOMODE_CBC)
                strcpy (AES_Mode, "CBC");
            else if (pjaes_test->aes_mode == CRYPTOMODE_CFB)
                strcpy (AES_Mode, "CFB");
            else if (pjaes_test->aes_mode == CRYPTOMODE_OFB)
                strcpy (AES_Mode, "OFB");
            else if (pjaes_test->aes_mode == CRYPTOMODE_CTR)
                strcpy (AES_Mode, "CTR");
            else    
                strcpy (AES_Mode, "ECB");

            /* Get Msg. Length */
            ulAESMsgLength = strlen(pjaes_test->plaintext);
            j = ( (ulAESMsgLength + 15) >> 4) << 4;
            for (i=ulAESMsgLength; i<j; i++)
                pjaes_test->plaintext[i] = 0;
            ulAESMsgLength = j;    
                		
            aes_set_key(&aes_ctx, pjaes_test->key, pjaes_test->key_length);
            
            /* Encryption Test */
            aes_enc_ast3000(&aes_ctx, pjaes_test->plaintext, pjaes_test->key, aes_output, ulAESMsgLength, pjaes_test->aes_mode);	    
	    if (strncmp(aes_output, pjaes_test->ciphertext, ulAESMsgLength))
	    {
	    	Flags |= FLAG_AESTEST_FAIL;	    	
	        printf("[INFO] AES%d %s Mode Encryption Failed \n", pjaes_test->key_length, AES_Mode);
	        printf("[DBG] Golden Data Dump .... \n");	        
	        for (i=0; i< ulAESMsgLength; i++)
	        {
	            printf("%02x ", pjaes_test->ciphertext[i]);	        	
	            if (((i+1) % 8) == 0)
	                printf("\n");        
	        }        
	        printf("\n [DBG] Error Data Dump .... \n");
	        for (i=0; i< ulAESMsgLength; i++)
	        {
	            printf("%02x ", aes_output[i]);	        	
	            if (((i+1) % 8) == 0)
	                printf("\n");        
	        }
	        printf("\n");	                
	    }
	    else
	    {
	    	/*
	        printf("[INFO] AES%d %s Mode Encryption Passed \n", pjaes_test->key_length, AES_Mode);
	        */
	    }	

            /* Decryption Test */
            aes_dec_ast3000(&aes_ctx, pjaes_test->ciphertext, pjaes_test->key, aes_output, ulAESMsgLength, pjaes_test->aes_mode);	    
	    if (strncmp(aes_output, pjaes_test->plaintext, ulAESMsgLength))
	    {
	    	Flags |= FLAG_AESTEST_FAIL;
	        printf("[INFO] AES%d %s Mode Decryption Failed \n", pjaes_test->key_length, AES_Mode);	        
	        printf("[DBG] Golden Data Dump .... \n");
	        for (i=0; i< ulAESMsgLength; i++)
	        {
	            printf("%02x ", pjaes_test->plaintext[i]);	        	
	            if (((i+1) % 8) == 0)
	                printf("\n");        
	        }        
	        printf("\n [DBG] Error Data Dump .... \n");
	        for (i=0; i< ulAESMsgLength; i++)
	        {
	            printf("%02x ", aes_output[i]);	        	
	            if (((i+1) % 8) == 0)
	                printf("\n");        
	        }
	        printf("\n");	                
	    }
	    else
	    {
	    	/*
	        printf("[INFO] AES%d %s Mode Decryption Passed \n", pjaes_test->key_length, AES_Mode);
	        */
	    }	
	    
	    pjaes_test++;
        } /* AES */

	/* RC4 Test */
	pjrc4_test = rc4test;
	while ((pjrc4_test->key[0] != 0xff) && (pjrc4_test->data[0] != 0xff))
	{

            /* Get Info */
            ulRC4KeyLength = strlen(pjrc4_test->key);
            ulRC4MsgLength = strlen(pjrc4_test->data);
            memcpy( (void *) rc4_buf_sw, (void *) pjrc4_test->data, ulRC4MsgLength );
            memcpy( (void *) rc4_buf_hw, (void *) pjrc4_test->data, ulRC4MsgLength );                                       

            /* Crypto */
            rc4_crypt_sw(rc4_buf_sw, ulRC4MsgLength, pjrc4_test->key, ulRC4KeyLength);
            rc4_crypt_ast3000(rc4_buf_hw, ulRC4MsgLength, pjrc4_test->key, ulRC4KeyLength);
           
	    if (strncmp(rc4_buf_hw, rc4_buf_sw, ulRC4MsgLength))
	    {
	    	Flags |= FLAG_RC4TEST_FAIL;	    	
	        printf("[INFO] RC4 Encryption Failed \n");
	        printf("[DBG] Golden Data Dump .... \n");
	        for (i=0; i< ulRC4MsgLength; i++)
	        {
	            printf("%02x ", rc4_buf_sw[i]);
	            if (((i+1) % 8) == 0)
	                printf("\n");        
	        }
	        printf("\n [DBG] Error Data Dump .... \n");
	        for (i=0; i< ulRC4MsgLength; i++)
	        {
	            printf("%02x ", rc4_buf_hw[i]);
	            if (((i+1) % 8) == 0)
	                printf("\n");        
	        }
	        printf("\n");	        
	    }
	    else
	    {
	    	/*
	        printf("[INFO] RC4 Encryption Passed \n");
	        */
	    }	

	    pjrc4_test++;
	    
        } /* RC4 */
       
	/* Hash Test */
	pjhash_test = hashtest;
	while (pjhash_test->hash_mode != 0xFF)
	{

            if (pjhash_test->hash_mode == HASHMODE_MD5)
                strcpy (HASH_Mode, "MD5");
            else if (pjhash_test->hash_mode == HASHMODE_SHA1)
                strcpy (HASH_Mode, "SHA1");
            else if (pjhash_test->hash_mode == HASHMODE_SHA256)
                strcpy (HASH_Mode, "SHA256");
            else if (pjhash_test->hash_mode == HASHMODE_SHA224)
                strcpy (HASH_Mode, "SHA224");
            
            /* Hash */   		
            hash_ast3000(pjhash_test->input, strlen(pjhash_test->input), hash_out, pjhash_test->hash_mode);            
	    if (strncmp(hash_out, pjhash_test->digest, pjhash_test->digest_length))
	    {
	    	Flags |= FLAG_HASHTEST_FAIL;	    	
	        printf("[INFO] HASH %s Failed \n", HASH_Mode);
	        printf("[DBG] Golden Data Dump .... \n");	        
	        for (i=0; i< pjhash_test->digest_length; i++)
	        {
	            printf("%02x ",pjhash_test->digest[i]);
	            if (((i+1) % 8) == 0) 	
	                printf("\n");        
	        }        
	        printf("\n [DBG] Error Data Dump .... \n");
	        for (i=0; i< pjhash_test->digest_length; i++)
	        {
	            printf("%02x ",hash_out[i]);
	            if (((i+1) % 8) == 0) 	
	                printf("\n");        
	        }
	        printf("\n");
	    }
	    else
	    {
	    	/*
	        printf("[INFO] HASH %s Passed \n", HASH_Mode);
	        */
	    }	

            pjhash_test++;
            
        } /* Hash Test */

	/* HMAC Test */
	pjhmac_test = hmactest;
	while (pjhmac_test->hash_mode != 0xFF)
	{

            if (pjhmac_test->hash_mode == HASHMODE_MD5)
                strcpy (HMAC_Mode, "MD5");
            else if (pjhmac_test->hash_mode == HASHMODE_SHA1)
                strcpy (HMAC_Mode, "SHA1");
            else if (pjhmac_test->hash_mode == HASHMODE_SHA256)
                strcpy (HMAC_Mode, "SHA256");
            else if (pjhmac_test->hash_mode == HASHMODE_SHA224)
                strcpy (HMAC_Mode, "SHA224");
            
            /* HMAC */   		
            hmackey_ast3000(pjhmac_test->key, pjhmac_test->key_length, pjhmac_test->hash_mode);
            hmac_ast3000(pjhmac_test->key, pjhmac_test->key_length, pjhmac_test->input, strlen(pjhmac_test->input), pjhmac_test->hash_mode, hmac_out);            
	    if (strncmp(hmac_out, pjhmac_test->digest, pjhmac_test->digest_length))
	    {
	    	Flags |= FLAG_HASHTEST_FAIL;	    	
	        printf("[INFO] HMAC %s Failed \n", HMAC_Mode);
	        printf("[DBG] Golden Data Dump .... \n");	        
	        for (i=0; i< pjhmac_test->digest_length; i++)
	        {
	            printf("%02x ",pjhmac_test->digest[i]);
	            if (((i+1) % 8) == 0) 	
	                printf("\n");        
	        }        
	        printf("\n [DBG] Error Data Dump .... \n");
	        for (i=0; i< pjhmac_test->digest_length; i++)
	        {
	            printf("%02x ",hmac_out[i]);
	            if (((i+1) % 8) == 0) 	
	                printf("\n");        
	        }
	        printf("\n");
	    }
	    else
	    {
	    	/*
	        printf("[INFO] HMAC %s Passed \n", HMAC_Mode);
	        */
	    }	

            pjhmac_test++;
            
        } /* HMAC Test */

	return Flags;
	
}

#endif /* CONFIG_SLT */
