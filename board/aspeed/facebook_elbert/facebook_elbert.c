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
/* PMI Base Address */
#define AST_PMI_VA_BASE             0x1E650000

#define SCU_RESET_SET2_REG          (AST_SCU_VA_BASE + 0x50)
#define SCU_RESET_CLEAR2_REG        (AST_SCU_VA_BASE + 0x54)
#define SCU_PIN_CONTROL8_REG        (AST_SCU_VA_BASE + 0x430)
#define SCU_HW_STRAPPING_REG        (AST_SCU_VA_BASE + 0x510)
#define SCU_HW_CLEAR_STRAPPING_REG  (AST_SCU_VA_BASE + 0x514)

#define AUTO_SOFT_RESET_CONTROL     (AST_FMC_VA_BASE + 0x50)
#define WDT_CONTROL_STATUS_REG      (AST_FMC_VA_BASE + 0x64)
#define WDT_TIMER_RELOAD_REG        (AST_FMC_VA_BASE + 0x68)
#define WDT_CNT_RESTART_REG         (AST_FMC_VA_BASE + 0x6C)

#define PMI_MDC_MDIO_CONTROL_REG    (AST_PMI_VA_BASE + 0x00)
#define PMI_MDC_MDIO_READ_DATA_REG  (AST_PMI_VA_BASE + 0x04)

/* FMC[64] */
#define WDT_DISABLE                 0x00                /* Disable WDT2 */
#define WDT_ENABLE                  0x01                /* Enable WDT2 */
#define WDT_CLEAR_EVNET_COUNTER     0xEA000000          /* Clear WDT2 event counter */
#define WDT_RESTART_TIMER_REG       0xFFFF4755          /* Restart WDT2 timer register */
#define WDT_TIMEOUT                 (10 * 60 * 10)      /* 10 minutes (units of 0.1 sec) */
#if WDT_TIMEOUT >= 8192
#error "WDT_TIMEOUT must be < 8192 (register is 13 bits wide)"
#endif

/* SCU[50] */
#define SCU_RESET_MII_CONTROLLER    (1 << 3)            /* Reset the MII controller */

/* SCU[430] */
#define SCU_ENABLE_MDC_PIN          (1 << 16)           /* Enable MDC */
#define SCU_ENABLE_MDIO_PIN         (1 << 17)           /* Enable MDIO */

/* SCU[510] */
#define SCU_ENABLE_ABR              (1 << 11)           /* Enable ABR boot */
#define SCU_3B_4B_AUTO_DETECTION    (1 << 10)           /* Boot 3B/4B address mode auto detection */
#define SCU_3B_MODE_CLEAR           (1 << 9)            /* Boot 3B address mode auto clear */

/* FMC[50] */
#define WAIT_SPI_WIP_IDLE           (1 << 1)            /* Enable wait SPI WIP idel */
#define GENERATE_SOFT_RESET_COMMAND (1 << 0)            /* Enable generate soft-reset command */

/* MDIO OOB */
#define MDIO_OOB_PAGE_REG 16
#define MDIO_OOB_ADDR_REG 17
#define MDIO_OOB_DATA_REG 24

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

	/* Set the Timeout value */
    *((volatile unsigned long *)(WDT_TIMER_RELOAD_REG)) = (WDT_TIMEOUT);

	/* Restart watchdog timer register */
    *((volatile unsigned long *)(WDT_CNT_RESTART_REG)) = (WDT_RESTART_TIMER_REG);

	/* Enable WDT */
    *((volatile unsigned long *)(WDT_CONTROL_STATUS_REG)) = (WDT_ENABLE);
}


#define MDIO_UDELAY               10
#define OOB_CONTROL_PAGE          0x0
#define OOB_IMP_PORT_STATE_REG    0xe
#define OOB_IMP_RGMII_CONTROL_REG 0x60

void mdio_init(void)
{
    unsigned long val;
    *(volatile unsigned long *)(SCU_RESET_CLEAR2_REG) = SCU_RESET_MII_CONTROLLER;

    val = *(volatile unsigned long *)(SCU_PIN_CONTROL8_REG);
    val |= (SCU_ENABLE_MDC_PIN | SCU_ENABLE_MDIO_PIN);
    *(volatile unsigned long *)(SCU_PIN_CONTROL8_REG) = val;
}

void mdio_restore(void)
{
    unsigned long val;
    *(volatile unsigned long *)(SCU_RESET_SET2_REG) = SCU_RESET_MII_CONTROLLER;

    val = *(volatile unsigned long *)(SCU_PIN_CONTROL8_REG);
    val &= ~(SCU_ENABLE_MDC_PIN | SCU_ENABLE_MDIO_PIN);
    *(volatile unsigned long *)(SCU_PIN_CONTROL8_REG) = val;
}

int mdio_wait_op_complete(unsigned int reg_addr, unsigned long mask,
                          unsigned long wait_val)
{
    int maxTries = 30;
    int i;
    unsigned long val;
    for (i = 0; i < maxTries; ++i) {
        val = *(volatile unsigned long *)reg_addr;
        if ((val & mask) == wait_val) {
            return 0;
        }
        udelay(MDIO_UDELAY);
    }
    return -1;
}

/* mdio_read returns register values between 0x0 and 0xffff
   in the case of success and -1 in the case of failure. */
int mdio_read(unsigned long addr)
{
    unsigned long reg_val;
    unsigned long fire_busy = 1 << 31;
    unsigned long ctrl_idle = 1 << 16;

    reg_val = fire_busy
            | (1 << 28)    /* clause_22 */
            | (1 << 27)    /* read request */
            | (0x1e << 21) /* pseudo-phy port address */
            | (addr << 16);
    *(volatile unsigned long *)(PMI_MDC_MDIO_CONTROL_REG) = reg_val;
    if (mdio_wait_op_complete(PMI_MDC_MDIO_READ_DATA_REG, ctrl_idle,
                              ctrl_idle)) {
        return -1;
    }
    reg_val = *(volatile unsigned long *)(PMI_MDC_MDIO_READ_DATA_REG);
    return (int)(reg_val & 0xffff);
}

int mdio_write(unsigned long addr, unsigned long val)
{
    unsigned long reg_val;
    unsigned long fire_busy = 1 << 31;

    reg_val = fire_busy
            | (1 << 28)    /* clause_22 */
            | (1 << 26)    /* write request */
            | (0x1e << 21) /* pseudo-phy port address */
            | (addr << 16)
            | val;
    *(volatile unsigned long *)(PMI_MDC_MDIO_CONTROL_REG) = reg_val;
    return mdio_wait_op_complete(PMI_MDC_MDIO_CONTROL_REG, fire_busy, 0);
}

int reg_wait_addr_reg_acked(void)
{
    int maxTries = 30;
    int i;
    int response;
    unsigned long op;
    for (i = 0; i < maxTries; ++i) {
        response = mdio_read(MDIO_OOB_ADDR_REG);
        if (response < 0) {
            return -1;
        }
        op = ((unsigned long)response) & 0x3;
        if (!op) {
            return 0;
        }
        udelay(MDIO_UDELAY);
    }
    return -1;
}

int program_oob_register(unsigned long page, unsigned long addr, unsigned long val)
{
    unsigned long data;

    data = (page << 8) | 0x1 /* access enable */;
    if (mdio_write(MDIO_OOB_PAGE_REG, data)) {
        return -1;
    }
    if (mdio_write(MDIO_OOB_DATA_REG, val)) {
        return -1;
    }
    data = (addr << 8) | 0x1 /* write op */;
    if (mdio_write(MDIO_OOB_ADDR_REG, data)) {
        return -1;
    }

    if (reg_wait_addr_reg_acked()) {
        return -1;
    }

    data = 0x0 /* access disable */;
    if (mdio_write(MDIO_OOB_PAGE_REG, data)) {
        return -1;
    }

    return 0;
}

/* Configure the BCM53134 registers to 1G */
void configureBcm53134(void)
{
    /* Initialize the MDIO controller for programming */
    mdio_init();

    /* These values set the speed to 1G, overriding the default settings in the
       EEPROM */
    program_oob_register(OOB_CONTROL_PAGE, OOB_IMP_PORT_STATE_REG, 0x8b);
    program_oob_register(OOB_CONTROL_PAGE, OOB_IMP_RGMII_CONTROL_REG, 0x0);

    /* Restore the MDIO controller setting */
    mdio_restore();
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(ABR_BOOT)
	Enable_ABR_BOOT();
#endif
   configureBcm53134();
	return 0;
}
