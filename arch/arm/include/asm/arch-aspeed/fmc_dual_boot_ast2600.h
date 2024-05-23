// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 (C) Facebook Technology Inc.
 *
 */

#ifndef __ASPEED_FMC_DUAL_BOOT_H_INCLUDED
#define __ASPEED_FMC_DUAL_BOOT_H_INCLUDED

/* SCU Base Address */
#define AST_SCU_BASE             0x1E6E2000
/* FMC Base Address */
#define AST_FMC_BASE             0x1E620000

#define SCU_HW_STRAPPING_REG        (AST_SCU_BASE + 0x510)
#define SCU_HW_CLEAR_STRAPPING_REG  (AST_SCU_BASE + 0x514)

#define AUTO_SOFT_RESET_CONTROL     (AST_FMC_BASE + 0x50)
#define WDT_CONTROL_STATUS_REG      (AST_FMC_BASE + 0x64)
#define WDT_TIMER_RELOAD_REG        (AST_FMC_BASE + 0x68)
#define WDT_CNT_RESTART_REG         (AST_FMC_BASE + 0x6C)

/* FMC[64] */
#define WDT_DISABLE                 0x00                /* Disable WDT2 */
#define WDT_ENABLE                  0x01                /* Enable WDT2 */
#define WDT_CLEAR_EVNET_COUNTER     0xEA000000          /* Clear WDT2 event counter */
#define WDT_RESTART_MAGIC           0x4755              /* Restart WDT2 timer register */
#define WDT_TIMEOUT                 (10 * 60 * 10)      /* 10 minutes (units of 0.1 sec) */
#if WDT_TIMEOUT >= 8192
#error "WDT_TIMEOUT must be < 8192 (register is 13 bits wide)"
#endif


/* SCU[510] */
#define SCU_ENABLE_ABR              (1 << 11)           /* Enable ABR boot */
#define SCU_3B_4B_AUTO_DETECTION    (1 << 10)           /* Boot 3B/4B address mode auto detection */
#define SCU_3B_MODE_CLEAR           (1 << 9)            /* Boot 3B address mode auto clear */

/* FMC[50] */
#define WAIT_SPI_WIP_IDLE           (1 << 1)            /* Enable wait SPI WIP idel */
#define GENERATE_SOFT_RESET_COMMAND (1 << 0)            /* Enable generate soft-reset command */

extern void fmc_enable_dual_boot(void);

#endif
