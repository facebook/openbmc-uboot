// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 (C) Facebook Technology Inc.
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/fmc_dual_boot_ast2600.h>

void fmc_enable_dual_boot(void)
{
    /* Enable boot SPI or eMMC ABR, and enable boot SPI 3B address mode auto-clear */
    setbits_le32(SCU_HW_STRAPPING_REG, SCU_ENABLE_ABR);
    setbits_le32(SCU_HW_CLEAR_STRAPPING_REG, SCU_3B_MODE_CLEAR | SCU_3B_4B_AUTO_DETECTION);

    /* Enable Auto Soft-Reset Command Control*/
    setbits_le32(AUTO_SOFT_RESET_CONTROL, WAIT_SPI_WIP_IDLE);
    clrbits_le32(AUTO_SOFT_RESET_CONTROL, GENERATE_SOFT_RESET_COMMAND);

    /* Disable WDT */
    writel(WDT_DISABLE, WDT_CONTROL_STATUS_REG);

    /* Set the Timeout value to max possible 5 Minutes */
    writel(WDT_TIMEOUT, WDT_TIMER_RELOAD_REG);

    /* Restart watchdog timer register */
    writel(WDT_RESTART_MAGIC, WDT_CNT_RESTART_REG);

    /* Enable WDT */
    writel(WDT_ENABLE, WDT_CONTROL_STATUS_REG);
}
