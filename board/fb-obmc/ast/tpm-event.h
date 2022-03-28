/*
 * (C) Copyright 2022-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_FBOBMC_AST_TPM_EVENT_H
#define _BOARD_FBOBMC_AST_TPM_EVENT_H

#if defined(CONFIG_TARGET_FB_AST_GEN_5)
#define TPM_EVENT_LOG_ADDR (0x1E728800)
#elif defined(CONFIG_TARGET_FB_AST_GEN_6)
#define TPM_EVENT_LOG_ADDR (0x10015000)
#else
#error "TPM event log not supported platform"
#endif

#define TPM_EVENT_LOG_FMT_VERSION (1)
#define TPM_EVENT_LOG_MAGIC_CODE (0xFBBE)

enum measurements {
	measure_unknown = 0,
	measure_spl = 1,
	measure_keystore = 2,
	measure_uboot = 3,
	measure_recv_uboot = 4,
	measure_uboot_env = 5,
	measure_vbs = 6,
	measure_os_kernel = 7,
	measure_os_rootfs = 8,
	measure_os_dtb = 9,
	measure_recv_os_kernel = 10,
	measure_recv_os_rootfs = 11,
	measure_recv_os_dtb = 12,
	// mark the end of supported measurement
	measure_unsupport,
	// mark the end
	measure_log_end = TPM_EVENT_LOG_MAGIC_CODE,
};

struct tpm_event_log_record {
	uint16_t measurement; /* enumeration of measurements */
	uint8_t pcrid; /* enumeration of ast_tpm_pcrs */
	uint8_t algo; /* enumeration of tpm2_algorithms */
	uint32_t index;
	uint8_t digest[0]; /* hash value, variable length depedents on algo */
};

/* TPM event log version 1
 * has one uint32_t len as first byte, followed with event log data
 * with one 4 byte event log end mark consist of magic code (0xFBBE)
 * and the TPM event log format version
 */
union tpm_event_log_end_mark {
	uint32_t endmark;
	struct {
		uint16_t magic;
		uint16_t fmtver;
	};
};
struct tpm_event_log {
	uint32_t len; /* length of the event logs */
	union {
		uint8_t data[0]; /* tpm_event_log_records */
		union tpm_event_log_end_mark end; /* end of log */
	};
};

/* LOG data max length is the SRAM allocated minuse the header and endmark
 */
#define TPM_EVENT_LOG_MAX_LEN                                                  \
	(TPM_EVENT_LOG_SRAM_SIZE - sizeof(struct tpm_event_log))

union tpm_event_index {
	uint32_t index;
	struct {
		uint8_t m_pcrid;
		uint8_t m_index;
		uint16_t m_measure;
	};
};

#define SET_EVENT_IDX(e, r, i, m)                                              \
	do {                                                                   \
		e.m_pcrid = (r);                                               \
		e.m_index = (i);                                               \
		e.m_measure = (m);                                             \
	} while (0)

int log_tpm_event(union tpm_event_index eventidx, uint8_t algo, uint8_t *digest,
		  uint32_t digest_len);

void reset_tpm_event_log(void);
#endif
