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
/* Err Flags */
#define FLAG_AESTEST_FAIL  		0x00000001
#define FLAG_RC4TEST_FAIL  		0x00000002
#define FLAG_HASHTEST_FAIL  		0x00000004

/* Specific */
/*
#define DRAM_BASE			0x40000000
#define	CRYPTO_SRC_BASE			(DRAM_BASE + 0x100000)
#define	CRYPTO_DST_BASE			(DRAM_BASE + 0x200000)
#define	CRYPTO_CONTEXT_BASE		(DRAM_BASE + 0x300000)

#define	HASH_SRC_BASE			(DRAM_BASE + 0x400000)
#define	HASH_DST_BASE			(DRAM_BASE + 0x500000)
#define	HMAC_KEY_BASE			(DRAM_BASE + 0x600000)
*/
#define m08byteAlignment(x)		((x + 0x00000007) & 0xFFFFFFF8)
#define m16byteAlignment(x)		((x + 0x0000000F) & 0xFFFFFFF0)
#define m64byteAlignment(x)		((x + 0x0000003F) & 0xFFFFFFC0)

#define CRYPTO_ALIGNMENT		16
#define CRYPTO_MAX_SRC			(100+CRYPTO_ALIGNMENT)
#define CRYPTO_MAX_DST			(100+CRYPTO_ALIGNMENT)
#define CRYPTO_MAX_CONTEXT		(100+CRYPTO_ALIGNMENT)

#define HASH_ALIGNMENT			16
#define HMAC_KEY_ALIGNMENT	 	64
#define HASH_MAX_SRC			(100+HASH_ALIGNMENT)
#define HASH_MAX_DST			(32+HASH_ALIGNMENT)
#define HMAC_MAX_KEY			(64+HMAC_KEY_ALIGNMENT)

/* General */
#define HAC_REG_BASE 			0x1e6e3000

#define MAX_KEYLENGTH			100
#define MAX_TEXTLENGTH			100
#define MAX_AESTEXTLENGTH		256
#define MAX_RC4TEXTLENGTH		256
#define MAX_RC4KEYLENGTH		256

#define CRYPTOMODE_ECB			0x00
#define CRYPTOMODE_CBC			0x01
#define CRYPTOMODE_CFB			0x02
#define CRYPTOMODE_OFB			0x03
#define CRYPTOMODE_CTR			0x04

#define HASHMODE_MD5			0x00
#define HASHMODE_SHA1			0x01
#define HASHMODE_SHA256			0x02
#define HASHMODE_SHA224			0x03

#define MIXMODE_DISABLE                 0x00
#define MIXMODE_CRYPTO                  0x02
#define MIXMODE_HASH                    0x03

#define REG_CRYPTO_SRC_BASE_OFFSET	0x00
#define REG_CRYPTO_DST_BASE_OFFSET	0x04
#define REG_CRYPTO_CONTEXT_BASE_OFFSET	0x08
#define REG_CRYPTO_LEN_OFFSET		0x0C
#define REG_CRYPTO_CMD_BASE_OFFSET	0x10
//#define REG_CRYPTO_ENABLE_OFFSET	0x14
#define REG_CRYPTO_STATUS_OFFSET	0x1C

#define REG_HASH_SRC_BASE_OFFSET	0x20
#define REG_HASH_DST_BASE_OFFSET	0x24
#define REG_HASH_KEY_BASE_OFFSET	0x28
#define REG_HASH_LEN_OFFSET		0x2C
#define REG_HASH_CMD_OFFSET		0x30
//#define REG_HASH_ENABLE_OFFSET		0x14
#define REG_HASH_STATUS_OFFSET		0x1C

#define HASH_BUSY			0x01
#define CRYPTO_BUSY			0x02

//#define ENABLE_HASH			0x01
//#define DISABLE_HASH			0x00
//#define ENABLE_CRYPTO			0x02
//#define DISABLE_CRYPTO			0x00

#define CRYPTO_SYNC_MODE_MASK		0x03
#define CRYPTO_SYNC_MODE_ASYNC		0x00
#define CRYPTO_SYNC_MODE_PASSIVE	0x02
#define CRYPTO_SYNC_MODE_ACTIVE		0x03

#define CRYPTO_AES128			0x00
#define CRYPTO_AES192			0x04
#define CRYPTO_AES256			0x08

#define CRYPTO_AES_ECB			0x00
#define CRYPTO_AES_CBC			0x10
#define CRYPTO_AES_CFB			0x20
#define CRYPTO_AES_OFB			0x30
#define CRYPTO_AES_CTR			0x40

#define CRYPTO_ENCRYPTO			0x80
#define CRYPTO_DECRYPTO			0x00

#define CRYPTO_AES			0x000
#define CRYPTO_RC4			0x100

#define CRYPTO_ENABLE_RW		0x000
#define CRYPTO_ENABLE_CONTEXT_LOAD	0x000
#define CRYPTO_ENABLE_CONTEXT_SAVE	0x000

#define HASH_SYNC_MODE_MASK		0x03
#define HASH_SYNC_MODE_ASYNC		0x00
#define HASH_SYNC_MODE_PASSIVE		0x02
#define HASH_SYNC_MODE_ACTIVE		0x03

#define HASH_READ_SWAP_ENABLE		0x04
#define HMAC_SWAP_CONTROL_ENABLE	0x08

#define HASH_ALG_SELECT_MASK		0x70
#define HASH_ALG_SELECT_MD5		0x00
#define HASH_ALG_SELECT_SHA1		0x20
#define HASH_ALG_SELECT_SHA224		0x40
#define HASH_ALG_SELECT_SHA256		0x50

#define HAC_ENABLE			0x80
#define HAC_DIGEST_CAL_ENABLE		0x180
#define HASH_INT_ENABLE			0x200

/* AES */
#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 erk[64];     /* encryption round keys */
    uint32 drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
}
aes_context;

typedef struct
{
    int	  aes_mode;
    int	  key_length;
	
    uint8 key[32];	/* as iv in CTR mode */
    uint8 plaintext[64];
    uint8 ciphertext[64];
    
}
aes_test;

/* RC4 */
typedef struct
{
    uint8 key[32];
    uint8 data[64];
}
rc4_test;

/* Hash */
typedef struct
{
    int	  hash_mode;
    int	  digest_length;
	
    uint8 input[64];
    uint8 digest[64];
    
}
hash_test;
	
/* HMAC */
typedef struct
{
    int	  hash_mode;
    int	  key_length;
    int	  digest_length;

    uint8 key[100];	
    uint8 input[64];
    uint8 digest[64];
    
}
hmac_test;
