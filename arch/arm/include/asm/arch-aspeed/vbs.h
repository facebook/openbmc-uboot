/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _VBS_H_
#define _VBS_H_

/* Location in SRAM used for verified boot content/flags. */
#if defined(CONFIG_TARGET_FB_AST_GEN_5)
#define AST_SRAM_VBS_BASE   (0x1E720200)
#elif defined(CONFIG_TARGET_FB_AST_GEN_6)
#define AST_SRAM_VBS_BASE  (0x10015800)
#else
#error "VBoot not supported target"
#endif

#define VBS_SUCCESS            0
#define VBS_ERROR_TYPE_HW      1
#define VBS_ERROR_TYPE_SPI     2
#define VBS_ERROR_TYPE_DATA    3
#define VBS_ERROR_TYPE_FW      4
#define VBS_ERROR_TYPE_FORCE   5
#define VBS_ERROR_TYPE_OS      6
#define VBS_ERROR_TYPE_TPM     7
#define VBS_ERROR_TYPE_NV      8
#define VBS_ERROR_TYPE_RB      9

#define VBS_ERROR_MISSING_SPI      10
#define VBS_ERROR_EXECUTE_FAILURE  11

#define VBS_ERROR_SPI_PROM         20
#define VBS_ERROR_SPI_SWAP         21

#define VBS_ERROR_BAD_MAGIC        30
#define VBS_ERROR_NO_IMAGES        31
#define VBS_ERROR_NO_FW            32
#define VBS_ERROR_NO_CONFIG        33
#define VBS_ERROR_NO_KEK           34
#define VBS_ERROR_NO_KEYS          35
#define VBS_ERROR_BAD_KEYS         36
#define VBS_ERROR_BAD_FW           37
#define VBS_ERROR_INVALID_SIZE     38

#define VBS_ERROR_KEYS_INVALID     40
#define VBS_ERROR_KEYS_UNVERIFIED  41
#define VBS_ERROR_FW_INVALID       42
#define VBS_ERROR_FW_UNVERIFIED    43

#define VBS_ERROR_FORCE_RECOVERY   50

#define VBS_ERROR_OS_INVALID       60

#define VBS_ERROR_TPM_SETUP           70
#define VBS_ERROR_TPM_FAILURE         71
#define VBS_ERROR_TPM_NO_PP           72
#define VBS_ERROR_TPM_INVALID_PP      73
#define VBS_ERROR_TPM_NO_PPLL         74
#define VBS_ERROR_TPM_PP_FAILED       75
#define VBS_ERROR_TPM_NOT_ENABLED     76
#define VBS_ERROR_TPM_ACTIVATE_FAILED 77
#define VBS_ERROR_TPM_RESET_NEEDED    78
#define VBS_ERROR_TPM_NOT_ACTIVATED   79

#define VBS_ERROR_TPM_NV_LOCK_FAILED  80
#define VBS_ERROR_TPM_NV_NOT_LOCKED   81
#define VBS_ERROR_TPM_NV_SPACE        82
#define VBS_ERROR_TPM_NV_BLANK        83
#define VBS_ERROR_TPM_NV_READ_FAILED  84
#define VBS_ERROR_TPM_NV_WRITE_FAILED 85
#define VBS_ERROR_TPM_NV_NOTSET       86

#define VBS_ERROR_ROLLBACK_MISSING  90
#define VBS_ERROR_ROLLBACK_FAILED   91
#define VBS_ERROR_ROLLBACK_HUGE     92
#define VBS_ERROR_ROLLBACK_FINISH   99

#define VBS_HANDOFF                 0xADEFAD8B
#define VBS_HANDOFF_TPM_RST         (VBS_HANDOFF - 1)
#define VBS_HANDOFF_TPM_SETUP       (VBS_HANDOFF - 2)
#define VBS_HANDOFF_SWAP            (VBS_HANDOFF - 3)
#define VBS_HANDOFF_OPEN_LATCH      (VBS_HANDOFF - 4)

#define VBS_KEYS_PATH   "/keys"

struct vbs {
  /* 00 */ u32 uboot_exec_address; /* Location in MMIO where U-Boot/Recovery U-Boot is execution */
  /* 04 */ u32 rom_exec_address;   /* Location in MMIO where ROM is executing from */
  /* 08 */ u32 rom_keys;           /* Location in MMIO where the ROM FDT is located */
  /* 0C */ u32 subordinate_keys;   /* Location in MMIO where subordinate FDT is located */
  /* 10 */ u32 rom_handoff;        /* Marker set when ROM is handing execution to U-Boot. */
  /* 14 */ u8 force_recovery;      /* Set by ROM when recovery is requested */
  /* 15 */ u8 hardware_enforce;    /* Set by ROM when WP pin of SPI0.0 is active low */
  /* 16 */ u8 software_enforce;    /* Set by ROM when RW environment uses verify=yes */
  /* 17 */ u8 recovery_boot;       /* Set by ROM when recovery is used */
  /* 18 */ u8 recovery_retries;    /* Number of attempts to recovery from verification failure */
  /* 19 */ u8 error_type;          /* Type of error, or 0 for success */
  /* 1A */ u8 error_code;          /* Unique error code, or 0 for success */
  /* 1B */ u8 error_tpm;           /* The last-most-recent error from the TPM. */
  /* 1C */ u16 crc;                /* A CRC of the vbs structure */
  /* 1E */ u16 error_tpm2;         /* tpm2 error code */
  /* 20 */ u32 subordinate_last;   /* Status reporting only: the last booted subordinate. */
  /* 24 */ u32 uboot_last;         /* Status reporting only: the last booted U-Boot. */
  /* 28 */ u32 kernel_last;        /* Status reporting only: the last booted kernel. */
  /* 2C */ u32 subordinate_current;/* Status reporting only: the current booted subordinate. */
  /* 30 */ u32 uboot_current;      /* Status reporting only: the current booted U-Boot. */
  /* 34 */ u32 kernel_current;     /* Status reporting only: the current booted kernel. */
  /* 38 */ u8 vbs_ver;             /* add vbs version for backward compatible */
  /* 39 */ u8 giu_mode;            /* golden image upgrade mode */
  /* 3A */ u16 op_cert_size;       /* vboot operation certificate data size */
  /* 3C */ u32 op_cert;            /* Location of vboot operation certificate data */
};

/* TPM NVram index used for rollback protection data. */
#define VBS_TPM_ROLLBACK_INDEX 0x100

/* Size of the rollback index space, allow for some reserved bytes. */
#define VBS_TPM_ROLLBACK_SIZE  192

#endif /* _VBS_H_ */
