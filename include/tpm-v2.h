/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#ifndef __TPM_V2_H
#define __TPM_V2_H

#include <tpm-common.h>

#define TPM2_DIGEST_LEN		32

/**
 * TPM2 Structure Tags for command/response buffers.
 *
 * @TPM2_ST_NO_SESSIONS: the command does not need an authentication.
 * @TPM2_ST_SESSIONS: the command needs an authentication.
 */
enum tpm2_structures {
	TPM2_ST_NO_SESSIONS	= 0x8001,
	TPM2_ST_SESSIONS	= 0x8002,
};

/**
 * TPM2 type of boolean.
 */
enum tpm2_yes_no {
	TPMI_YES		= 1,
	TPMI_NO			= 0,
};

/**
 * TPM2 startup values.
 *
 * @TPM2_SU_CLEAR: reset the internal state.
 * @TPM2_SU_STATE: restore saved state (if any).
 */
enum tpm2_startup_types {
	TPM2_SU_CLEAR		= 0x0000,
	TPM2_SU_STATE		= 0x0001,
};

/**
 * TPM2 permanent handles.
 *
 * @TPM2_RH_OWNER: refers to the 'owner' hierarchy.
 * @TPM2_RS_PW: indicates a password.
 * @TPM2_RH_LOCKOUT: refers to the 'lockout' hierarchy.
 * @TPM2_RH_ENDORSEMENT: refers to the 'endorsement' hierarchy.
 * @TPM2_RH_PLATFORM: refers to the 'platform' hierarchy.
 */
enum tpm2_handles {
	TPM2_RH_OWNER		= 0x40000001,
	TPM2_RS_PW		= 0x40000009,
	TPM2_RH_LOCKOUT		= 0x4000000A,
	TPM2_RH_ENDORSEMENT	= 0x4000000B,
	TPM2_RH_PLATFORM	= 0x4000000C,
	TPM2_RH_PLATFORM_NV	= 0x4000000D,
};

/**
 * TPM2 command codes used at the beginning of a buffer, gives the command.
 *
 * @TPM2_CC_HIERCONTROL: TPM2_HierarchyControl().
 * @TPM2_CC_NV_UNDEFINESPACE: TPM2_NV_UndefineSpace().
 * @TPM2_CC_CLEAR: TPM2_Clear().
 * @TPM2_CC_CLEARCONTROL: TPM2_ClearControl().
 * @TPM2_CC_HIERCHANGEAUTH: TPM2_HierarchyChangeAuth().
 * @TPM2_CC_NV_DEFINESPACE: TPM2_NV_DefineSpace().
 * @TPM2_CC_PCR_SETAUTHPOL: TPM2_PCR_SetAuthPolicy().
 * @TPM2_CC_NV_WRITE: TPM2_NV_Write().
 * @TPM2_CC_DAM_RESET: TPM2_DictionaryAttackLockReset().
 * @TPM2_CC_DAM_PARAMETERS: TPM2_DictionaryAttackParameters().
 * @TPM2_CC_STARTUP: TPM2_Startup().
 * @TPM2_CC_SELF_TEST: TPM2_SelfTest().
 * @TPM2_CC_NV_READ: TPM2_NV_Read().
 * @TPM2_CC_NV_READPUBLIC: TPM2_NV_ReadPublic().
 * @TPM2_CC_GET_CAPABILITY: TPM2_GetCapibility().
 * @TPM2_CC_PCR_READ: TPM2_PCR_Read().
 * @TPM2_CC_PCR_EXTEND: TPM2_PCR_Extend().
 * @TPM2_CC_PCR_SETAUTHVAL: TPM2_PCR_SetAuthValue().
 */
enum tpm2_command_codes {
	TPM2_CC_HIERCONTROL	= 0x0121,
	TPM2_CC_NV_UNDEFINESPACE= 0x0122,
	TPM2_CC_CLEAR		= 0x0126,
	TPM2_CC_CLEARCONTROL	= 0x0127,
	TPM2_CC_HIERCHANGEAUTH	= 0x0129,
	TPM2_CC_NV_DEFINESPACE	= 0x012A,
	TPM2_CC_PCR_SETAUTHPOL	= 0x012C,
	TPM2_CC_NV_WRITE	= 0x0137,
	TPM2_CC_DAM_RESET	= 0x0139,
	TPM2_CC_DAM_PARAMETERS	= 0x013A,
	TPM2_CC_STARTUP		= 0x0144,
	TPM2_CC_SELF_TEST	= 0x0143,
	TPM2_CC_NV_READ		= 0x014E,
	TPM2_CC_NV_READPUBLIC	= 0x0169,
	TPM2_CC_GET_CAPABILITY	= 0x017A,
	TPM2_CC_PCR_READ	= 0x017E,
	TPM2_CC_PCR_EXTEND	= 0x0182,
	TPM2_CC_PCR_SETAUTHVAL	= 0x0183,
};

/**
 * TPM2 return codes.
 */
enum tpm2_return_codes {
	TPM2_RC_SUCCESS		= 0x0000,
	TPM2_RC_BAD_TAG		= 0x001E,
	TPM2_RC_FMT1		= 0x0080,
	TPM2_RC_HASH		= TPM2_RC_FMT1 + 0x0003,
	TPM2_RC_VALUE		= TPM2_RC_FMT1 + 0x0004,
	TPM2_RC_SIZE		= TPM2_RC_FMT1 + 0x0015,
	TPM2_RC_BAD_AUTH	= TPM2_RC_FMT1 + 0x0022,
	TPM2_RC_HANDLE		= TPM2_RC_FMT1 + 0x000B,
	TPM2_RC_HANDLE1		= TPM2_RC_HANDLE | (1 << 8),
	TPM2_RC_VER1		= 0x0100,
	TPM2_RC_INITIALIZE	= TPM2_RC_VER1 + 0x0000,
	TPM2_RC_FAILURE		= TPM2_RC_VER1 + 0x0001,
	TPM2_RC_DISABLED	= TPM2_RC_VER1 + 0x0020,
	TPM2_RC_AUTH_MISSING	= TPM2_RC_VER1 + 0x0025,
	TPM2_RC_COMMAND_CODE	= TPM2_RC_VER1 + 0x0043,
	TPM2_RC_AUTHSIZE	= TPM2_RC_VER1 + 0x0044,
	TPM2_RC_AUTH_CONTEXT	= TPM2_RC_VER1 + 0x0045,
	TPM2_RC_NEEDS_TEST	= TPM2_RC_VER1 + 0x0053,
	TPM2_RC_WARN		= 0x0900,
	TPM2_RC_TESTING		= TPM2_RC_WARN + 0x000A,
	TPM2_RC_REFERENCE_H0	= TPM2_RC_WARN + 0x0010,
	TPM2_RC_LOCKOUT		= TPM2_RC_WARN + 0x0021,
};

/**
 * TPM2 algorithms.
 */
enum tpm2_algorithms {
	TPM2_ALG_XOR		= 0x0A,
	TPM2_ALG_SHA256		= 0x0B,
	TPM2_ALG_SHA384		= 0x0C,
	TPM2_ALG_SHA512		= 0x0D,
	TPM2_ALG_NULL		= 0x10,
};

/* NV index attributes */
enum tpm_index_attrs {
	TPMA_NV_PPWRITE		= 1UL << 0,
	TPMA_NV_OWNERWRITE	= 1UL << 1,
	TPMA_NV_AUTHWRITE	= 1UL << 2,
	TPMA_NV_POLICYWRITE	= 1UL << 3,
	TPMA_NV_COUNTER		= 1UL << 4,
	TPMA_NV_BITS		= 1UL << 5,
	TPMA_NV_EXTEND		= 1UL << 6,
	TPMA_NV_POLICY_DELETE	= 1UL << 10,
	TPMA_NV_WRITELOCKED	= 1UL << 11,
	TPMA_NV_WRITEALL	= 1UL << 12,
	TPMA_NV_WRITEDEFINE	= 1UL << 13,
	TPMA_NV_WRITE_STCLEAR	= 1UL << 14,
	TPMA_NV_GLOBALLOCK	= 1UL << 15,
	TPMA_NV_PPREAD		= 1UL << 16,
	TPMA_NV_OWNERREAD	= 1UL << 17,
	TPMA_NV_AUTHREAD	= 1UL << 18,
	TPMA_NV_POLICYREAD	= 1UL << 19,
	TPMA_NV_NO_DA		= 1UL << 25,
	TPMA_NV_ORDERLY		= 1UL << 26,
	TPMA_NV_CLEAR_STCLEAR	= 1UL << 27,
	TPMA_NV_READLOCKED	= 1UL << 28,
	TPMA_NV_WRITTEN		= 1UL << 29,
	TPMA_NV_PLATFORMCREATE	= 1UL << 30,
	TPMA_NV_READ_STCLEAR	= 1UL << 31,

	TPMA_NV_MASK_READ	= TPMA_NV_PPREAD | TPMA_NV_OWNERREAD |
				TPMA_NV_AUTHREAD | TPMA_NV_POLICYREAD,
	TPMA_NV_MASK_WRITE	= TPMA_NV_PPWRITE | TPMA_NV_OWNERWRITE |
					TPMA_NV_AUTHWRITE | TPMA_NV_POLICYWRITE,
};

/* TPM2 Handle Type */
enum tpm2_handle_type {
	TPM_HT_NV_INDEX = 1,
};

enum {
	TPM_ACCESS_VALID		= 1 << 7,
	TPM_ACCESS_ACTIVE_LOCALITY	= 1 << 5,
	TPM_ACCESS_REQUEST_PENDING	= 1 << 2,
	TPM_ACCESS_REQUEST_USE		= 1 << 1,
	TPM_ACCESS_ESTABLISHMENT	= 1 << 0,
};

enum {
	TPM_STS_FAMILY_SHIFT		= 26,
	TPM_STS_FAMILY_MASK		= 0x3 << TPM_STS_FAMILY_SHIFT,
	TPM_STS_FAMILY_TPM2		= 1 << TPM_STS_FAMILY_SHIFT,
	TPM_STS_RESE_TESTABLISMENT_BIT	= 1 << 25,
	TPM_STS_COMMAND_CANCEL		= 1 << 24,
	TPM_STS_BURST_COUNT_SHIFT	= 8,
	TPM_STS_BURST_COUNT_MASK	= 0xffff << TPM_STS_BURST_COUNT_SHIFT,
	TPM_STS_VALID			= 1 << 7,
	TPM_STS_COMMAND_READY		= 1 << 6,
	TPM_STS_GO			= 1 << 5,
	TPM_STS_DATA_AVAIL		= 1 << 4,
	TPM_STS_DATA_EXPECT		= 1 << 3,
	TPM_STS_SELF_TEST_DONE		= 1 << 2,
	TPM_STS_RESPONSE_RETRY		= 1 << 1,
	TPM_STS_READ_ZERO               = 0x23
};

enum {
	TPM_CMD_COUNT_OFFSET	= 2,
	TPM_CMD_ORDINAL_OFFSET	= 6,
	TPM_MAX_BUF_SIZE	= 1260,
};

/**
 * Issue a TPM2_Startup command.
 *
 * @dev		TPM device
 * @mode	TPM startup mode
 *
 * @return code of the operation
 */
u32 tpm2_startup(struct udevice *dev, enum tpm2_startup_types mode);

/**
 * Issue a TPM2_SelfTest command.
 *
 * @dev		TPM device
 * @full_test	Asking to perform all tests or only the untested ones
 *
 * @return code of the operation
 */
u32 tpm2_self_test(struct udevice *dev, enum tpm2_yes_no full_test);

/**
 * Issue a TPM2_Clear command.
 *
 * @dev		TPM device
 * @handle	Handle
 * @pw		Password
 * @pw_sz	Length of the password
 *
 * @return code of the operation
 */
u32 tpm2_clear(struct udevice *dev, u32 handle, const char *pw,
	       const ssize_t pw_sz);

/**
 * Issue a TPM2_PCR_Extend command.
 *
 * @dev		TPM device
 * @index	Index of the PCR
 * @digest	Value representing the event to be recorded
 *
 * @return code of the operation
 */
u32 tpm2_pcr_extend(struct udevice *dev, u32 index, const uint8_t *digest);

/**
 * Issue a TPM2_PCR_Read command.
 *
 * @dev		TPM device
 * @idx		Index of the PCR
 * @idx_min_sz	Minimum size in bytes of the pcrSelect array
 * @data	Output buffer for contents of the named PCR
 * @updates	Optional out parameter: number of updates for this PCR
 *
 * @return code of the operation
 */
u32 tpm2_pcr_read(struct udevice *dev, u32 idx, unsigned int idx_min_sz,
		  void *data, unsigned int *updates);

/**
 * Issue a TPM2_GetCapability command.  This implementation is limited
 * to query property index that is 4-byte wide.
 *
 * @dev		TPM device
 * @capability	Partition of capabilities
 * @property	Further definition of capability, limited to be 4 bytes wide
 * @buf		Output buffer for capability information
 * @prop_count	Size of output buffer
 *
 * @return code of the operation
 */
u32 tpm2_get_capability(struct udevice *dev, u32 capability, u32 property,
			void *buf, size_t prop_count);

/**
 * Issue a TPM2_DictionaryAttackLockReset command.
 *
 * @dev		TPM device
 * @pw		Password
 * @pw_sz	Length of the password
 *
 * @return code of the operation
 */
u32 tpm2_dam_reset(struct udevice *dev, const char *pw, const ssize_t pw_sz);

/**
 * Issue a TPM2_DictionaryAttackParameters command.
 *
 * @dev		TPM device
 * @pw		Password
 * @pw_sz	Length of the password
 * @max_tries	Count of authorizations before lockout
 * @recovery_time Time before decrementation of the failure count
 * @lockout_recovery Time to wait after a lockout
 *
 * @return code of the operation
 */
u32 tpm2_dam_parameters(struct udevice *dev, const char *pw,
			const ssize_t pw_sz, unsigned int max_tries,
			unsigned int recovery_time,
			unsigned int lockout_recovery);

/**
 * Issue a TPM2_HierarchyChangeAuth command.
 *
 * @dev		TPM device
 * @handle	Handle
 * @newpw	New password
 * @newpw_sz	Length of the new password
 * @oldpw	Old password
 * @oldpw_sz	Length of the old password
 *
 * @return code of the operation
 */
int tpm2_change_auth(struct udevice *dev, u32 handle, const char *newpw,
		     const ssize_t newpw_sz, const char *oldpw,
		     const ssize_t oldpw_sz);

/**
 * Issue a TPM_PCR_SetAuthPolicy command.
 *
 * @dev		TPM device
 * @pw		Platform password
 * @pw_sz	Length of the password
 * @index	Index of the PCR
 * @digest	New key to access the PCR
 *
 * @return code of the operation
 */
u32 tpm2_pcr_setauthpolicy(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, u32 index, const char *key);

/**
 * Issue a TPM_PCR_SetAuthValue command.
 *
 * @dev		TPM device
 * @pw		Platform password
 * @pw_sz	Length of the password
 * @index	Index of the PCR
 * @digest	New key to access the PCR
 * @key_sz	Length of the new key
 *
 * @return code of the operation
 */
u32 tpm2_pcr_setauthvalue(struct udevice *dev, const char *pw,
			  const ssize_t pw_sz, u32 index, const char *key,
			  const ssize_t key_sz);

/**
 * Issue a TPM_NV_DefineSpace command.
 *
 * @dev			TPM device
 * @pw			Platform password
 * @pw_sz		Length of the password
 * @auth_handle	Auth handle
 * @nv_index	Requested NVRAM index
 * @hash_alg	Hash algorithm
 * @attributes	NV Attributes
 * @data_sz		Requested data size
 *
 * @return 		code of the operation
 */
u32 tpm2_nv_definespace(struct udevice *dev, const u8 *pw,
				u16 pw_sz, u32 auth_handle, u32 nv_index,
				u16 hash_alg, u32 attributes, u16 data_sz);

/**
 * Issue a TPM_NV_Write command.
 *
 * @dev			TPM device
 * @pw			Platform password
 * @pw_sz		Length of the password
 * @auth_handle	Auth handle
 * @nv_index	Requested NVRAM index
 * @data		Data to write
 * @data_sz		Data buffer size
 * @data_ofs	Data offset into the NV Area
 *
 * @return 		code of the operation
 */
u32 tpm2_nv_write(struct udevice *dev, const u8 *pw, u16 pw_sz,
			u32 auth_handle, u32 nv_index, u8 *data, u16 data_sz, u16 data_ofs);

/**
 * Issue a TPM_NV_Read command.
 *
 * @dev			TPM device
 * @pw			Platform password
 * @pw_sz		Length of the password
 * @auth_handle	Auth handle
 * @nv_index	Requested NVRAM index
 * @size		Number of byte to read
 * @ofs			Data offset into the NV Area
 * @data		Data reading buffer
 *
 * @return 		code of the operation
 */
u32 tpm2_nv_read(struct udevice *dev, const u8 *pw, u16 pw_sz,
			u32 auth_handle, u32 nv_index, u16 size, u16 ofs, u8* data);

/**
 * Issue a TPM_HierarchyControl command.
 *
 * @dev			TPM device
 * @pw			Platform password
 * @pw_sz		Length of the password
 * @auth_handle	Auth handle
 * @res_handle	Resource handle
 * @enable		Enable or disable resource handle
 *
 * @return 		code of the operation
 */
u32 tpm2_hierarchy_control(struct udevice *dev, const u8 *pw, u16 pw_sz,
			u32 auth_handle, u32 res_handle, bool enable);

/**
 * Issue a TPM_NV_ReadPublic command.
 *
 * @dev			TPM device
 * @nv_index	Requested NVRAM index
 * @hash_alg	Hash algorithm [OUT]
 * @attributes	NV Attributes [OUT]
 * @policy_sz	Size of auth policy [OUT]
 * @policy		Auth policy [OUT]
 * @data_sz		Requested data size [OUT]
 * @nv_name_sz	Size of NV index name [OUT]
 * @nv_name		NV index name [OUT]
 *
 * @return 		code of the operation
 */
u32 tpm2_nv_readpublic(struct udevice *dev, u32 nv_index,
			u16 *hash_alg, u32* attributes, u16 *policy_sz, u8* policy,
			u16* data_sz, u16 *nv_name_sz, u8 *nv_name);

/**
 * Issue a TPM_NV_UndefineSpace command.
 *
 * @dev			TPM device
 * @pw			Platform password
 * @pw_sz		Length of the password
 * @auth_handle	Auth handle
 * @nv_index	Requested NVRAM index
 *
 * @return 		code of the operation
 */
u32 tpm2_nv_undefinespace(struct udevice *dev, const u8 *pw, u16 pw_sz,
			u32 auth_handle, u32 nv_index);
#endif /* __TPM_V2_H */
