// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Facebook Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/fmc_dual_boot_ast2600.h>

void set_gpio_g6_output_low(void)
{
#define GPIO_G		16
#define GPIO_G6_MASK  	BIT(GPIO_G + 6)
	/* GPIO020: GPIO_E/F/G/H Data Value Register
	 * 31:24 Port GPIOH[7:0] data register
	 * 23:16 Port GPIOG[7:0] data register
	 * 15:8 Port GPIOF[7:0] data register
	 * 7:0 Port GPIOE[7:0] data register
	 */
	u32 value = readl(0x1e780020);
	/* GPIO024: GPIO_E/F/G/H Direction Value Register
	 * 31:24 Port GPIOH[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 23:16 Port GPIOG[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 15:8 Port GPIOF[7:0] direction control
	 * 0: Select input mode 1: Select output mode
	 * 7:0 Port GPIOE[7:0] direction control
	 */
	u32 direction = readl(0x1e780024);

	direction |= GPIO_G6_MASK;
	value &= ~GPIO_G6_MASK;
	writel(direction, 0x1e780024);
	writel(value, 0x1e780020);
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(FMC_DUAL_BOOT)
	fmc_enable_dual_boot();
#endif
	/* Because PFR led feature requirement, In DVT Design MAX10 report it’s
	 * status first and the LED will drove by CPLD. This cause BMC bound the
	 * PCA9548 driver fail during loading Kernel.
	 * To fix this issue, after BMC boot up, BMC should switch the control
	 * path to BMC by pull down the GPIO G6.
	 */
	set_gpio_g6_output_low();
	return 0;
}
