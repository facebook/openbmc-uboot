// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Facebook Inc.
 */

#include <common.h>
#include <asm/io.h>

/* SCU Base Address */
#define AST_SCU_VA_BASE             0x1E6E2000
/* FMC Base Address */
#define AST_FMC_VA_BASE             0x1E620000

#define SCU_HW_STRAPPING_REG        (AST_SCU_VA_BASE + 0x510)
#define SCU_HW_CLEAR_STRAPPING_REG  (AST_SCU_VA_BASE + 0x514)

#define AUTO_SOFT_RESET_CONTROL     (AST_FMC_VA_BASE + 0x50)
#define WDT_CONTROL_STATUS_REG      (AST_FMC_VA_BASE + 0x64)
#define WDT_TIMER_RELOAD_REG        (AST_FMC_VA_BASE + 0x68)
#define WDT_CNT_RESTART_REG         (AST_FMC_VA_BASE + 0x6C)

/* FMC[64] */
#define WDT_DISABLE                 0x00                /* Disable WDT2 */
#define WDT_ENABLE                  0x01                /* Enable WDT2 */
#define WDT_CLEAR_EVNET_COUNTER     0xEA000000          /* Clear WDT2 event counter */
#define WDT_RESTART_TIMER_REG       0xFFFF4755          /* Restart WDT2 timer register */
#define WDT_TIMEOUT                 0xBB8              	/* 5 Minutes (3000 * 0.1sec) */

/* SCU[510] */
#define SCU_ENABLE_ABR              (1 << 11)           /* Enable ABR boot */
#define SCU_3B_4B_AUTO_DETECTION    (1 << 10)           /* Boot 3B/4B address mode auto detection */
#define SCU_3B_MODE_CLEAR           (1 << 9)            /* Boot 3B address mode auto clear */

/* FMC[50] */
#define WAIT_SPI_WIP_IDLE           (1 << 1)            /* Enable wait SPI WIP idel */
#define GENERATE_SOFT_RESET_COMMAND (1 << 0)            /* Enable generate soft-reset command */

void Enable_ABR_BOOT(void)
{
    /* Enable boot SPI or eMMC ABR, and enable boot SPI 3B address mode auto-clear.*/
    *((volatile unsigned long *)(SCU_HW_STRAPPING_REG)) |= (SCU_ENABLE_ABR);
	*((volatile unsigned long *)(SCU_HW_CLEAR_STRAPPING_REG)) |= (SCU_3B_MODE_CLEAR | SCU_3B_4B_AUTO_DETECTION);

    /* Enable Auto Soft-Reset Command Control*/
    *((volatile unsigned long *)(AUTO_SOFT_RESET_CONTROL)) |= (WAIT_SPI_WIP_IDLE);
	*((volatile unsigned long *)(AUTO_SOFT_RESET_CONTROL)) &= ~(GENERATE_SOFT_RESET_COMMAND);

    /* Disable WDT */
    *((volatile unsigned long *)(WDT_CONTROL_STATUS_REG)) = (WDT_DISABLE);

	/* Set the Timeout value to max possible 5 Minutes */
    *((volatile unsigned long *)(WDT_TIMER_RELOAD_REG)) = (WDT_TIMEOUT);

	/* Restart watchdog timer register */
    *((volatile unsigned long *)(WDT_CNT_RESTART_REG)) = (WDT_RESTART_TIMER_REG);

	/* Enable WDT */
    *((volatile unsigned long *)(WDT_CONTROL_STATUS_REG)) = (WDT_ENABLE);
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(ABR_BOOT)
	Enable_ABR_BOOT();
#endif
	return 0;
}
