/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Location in SRAM used for verified boot content/flags. */
#define AST_SRAM_VBS_BASE   0x1E720200

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

#define VBS_HANDOFF     0xADEFAD8B

#define VBS_KEYS_PATH   "/keys"

struct vbs {
  u32 uboot_exec_address; /* Location in MMIO where U-Boot/Recovery U-Boot is execution */
  u32 rom_exec_address;   /* Location in MMIO where ROM is executing from */
  u32 rom_keys;           /* Location in MMIO where the ROM FDT is located */
  u32 subordinate_keys;   /* Location in MMIO where subordinate FDT is located */
  u32 rom_handoff;        /* Marker set when ROM is handing execution to U-Boot. */
  u8 force_recovery;      /* Set by ROM when recovery is requested */
  u8 hardware_enforce;    /* Set by ROM when WP pin of SPI0.0 is active low */
  u8 software_enforce;    /* Set by ROM when RW environment uses verify=yes */
  u8 recovery_boot;       /* Set by ROM when recovery is used */
  u8 recovery_retries;    /* Number of attempts to recovery from verification failure */
  u8 error_type;          /* Type of error, or 0 for success */
  u8 error_code;          /* Unique error code, or 0 for success */
};
