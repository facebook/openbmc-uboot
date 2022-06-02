#ifndef _BOARD_ASPEED_AST_G6_H
#define _BOARD_ASPEED_AST_G6_H


#define AST6_ESPI_BASE             (0x1E6EE000)

/* GPIO REG */
#define AST6_GPIO_BASE             (0x1E780000)

#define GPIO_ABCD_BASE_REG         (AST6_GPIO_BASE | 0x000)
#define GPIO_ABCD_DATA_REG         (GPIO_ABCD_BASE_REG)
#define GPIO_ABCD_DIR_REG          (AST6_GPIO_BASE | 0x004)

#define GPIO_EFGH_BASE_REG         (AST6_GPIO_BASE | 0x020)
#define GPIO_EFGH_DATA_REG         (GPIO_EFGH_BASE_REG)
#define GPIO_EFGH_DIR_REG          (AST6_GPIO_BASE | 0x024)

#define GPIO_IJKL_BASE_REG         (AST6_GPIO_BASE | 0x070)
#define GPIO_IJKL_DATA_REG         (GPIO_IJKL_BASE_REG)
#define GPIO_IJKL_DIR_REG          (AST6_GPIO_BASE | 0x074)

#define GPIO_MNOP_BASE_REG         (AST6_GPIO_BASE | 0x078)
#define GPIO_MNOP_DATA_REG         (GPIO_MNOP_BASE_REG)
#define GPIO_MNOP_DIR_REG          (AST6_GPIO_BASE | 0x07C)
#define GPIO_MNOP_CMD_SOURCE0      (AST6_GPIO_BASE | 0x0E0)
#define GPIO_MNOP_CMD_SOURCE1      (AST6_GPIO_BASE | 0x0E4)

#define GPIO_QRST_BASE_REG         (AST6_GPIO_BASE | 0x080)
#define GPIO_QRST_DATA_REG         (GPIO_QRST_BASE_REG)
#define GPIO_QRST_DIR_REG          (AST6_GPIO_BASE | 0x084)

#define GPIO_UVWX_BASE_REG         (AST6_GPIO_BASE | 0x088)
#define GPIO_UVWX_DATA_REG         (GPIO_UVWX_BASE_REG)
#define GPIO_UVWX_DIR_REG          (AST6_GPIO_BASE | 0x08C)

#define GPIO_NAME(base, pin)       (1 << (pin + (base -'A')%4*8))
#define GPIO_GROUP(base, val)      (val << ((base -'A')%4*8))


/* SGPIO REG */
#define SGPIO_ABCD_BASE_REG        (AST6_GPIO_BASE | 0x500)
#define SGPIO_ABCD_DATA_REG        (SGPIO_ABCD_BASE_REG)
#define SGPIO_EFGH_BASE_REG        (AST6_GPIO_BASE | 0x51C)
#define SGPIO_EFGH_DATA_REG        (SGPIO_EFGH_BASE_REG)
#define SGPIO_IJKL_BASE_REG        (AST6_GPIO_BASE | 0x538)
#define SGPIO_IJKL_DATA_REG        (SGPIO_IJKL_BASE_REG)
#define SGPIO_MNOP_BASE_REG        (AST6_GPIO_BASE | 0x590)
#define SGPIO_MNOP_DATA_REG        (SGPIO_MNOP_BASE_REG)

#define SGPIO_NAME(base, pin)       (1 << (pin + (base -'A')%4*8))

/* LPC */
#define AST6_LPC_BASE              (0x1E789000)
#define LPC_HICR5                  (AST6_LPC_BASE | 0x80)
#define HICR5_EN_SIOGIO            (1 << 31)  /* Enable SIOGIO */

#define LPC_HICR6                  (AST6_LPC_BASE | 0x84)
#define LPC_SNPWADR                (AST6_LPC_BASE | 0x90)
#define LPC_SNOOP_ADDR             (0x80)

#define LPC_HICRB                  (AST6_LPC_BASE | 0x100)
#define LPC_PCCR0                  (AST6_LPC_BASE | 0x130)

/* SCU REG */
#define AST6_SCU_BASE              (0x1E6E2000)
#define SCU_RESET_SET2_REG         (AST6_SCU_BASE | 0x50)
#define SCU_RESET_CLEAR2_REG       (AST6_SCU_BASE | 0x54)
#define SCU_SYS_RESET_STATUS_REG   (AST6_SCU_BASE | 0x64)
#define SCU_SYS_RESET_STATUS2_REG  (AST6_SCU_BASE | 0x74)
#define PWR_ON_RST_SRST            (0x1)

#define SCU_MUTI_FN_PIN_CTRL5      (AST6_SCU_BASE | 0x414)
#define SCU_MUTI_FN_PIN_CTRL9      (AST6_SCU_BASE | 0x434)
#define SCU_MUTI_FN_PIN_CTRL15     (AST6_SCU_BASE | 0x454)
#define SCU_PIN_CONTROL8_REG       (AST6_SCU_BASE | 0x430)
#define SCU_HW_STRAP2_SET_REG      (AST6_SCU_BASE | 0x510)
#define SCU_HW_STRAP2_CLR_REG      (AST6_SCU_BASE | 0x514)
#define SCU_HW_STRAP3_REG          (AST6_SCU_BASE | 0x51C)
#define ENABLE_GPIO_PASSTHROUGH    (1 << 9)

//SCU SGPIO
#define SGPIO_CLK_DIV(x)           ((x) << 16)
#define SGPIO_BYTES(x)             ((x) << 6)
#define SGPIO_ENABLE               (1)
#define SGPIO1_CFG_REG             (AST6_GPIO_BASE | 0x554)
#define SCU_SGPM_ENABLE            GENMASK(27, 24)

#endif
