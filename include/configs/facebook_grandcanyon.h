/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020 Facebook Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/aspeed-common.h>

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x300000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x5000000)

#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* Memory Info */
#define CONFIG_SYS_LOAD_ADDR	0x83000000

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE		0x20000
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET	0xE0000

/* u-boot reset command and reset() is implemented via sysreset
 * which will trigger the WDT to do full-chip reset by default.
 * But WDT Full-chip reset cannot successfully drive the WDTRST_N pin,
 * so define AST_SYSRESET_WITH_SOC to make sysreset use SOC reset,
 * and use AST_SYS_RESET_WITH_SOC as SOC reset mask #1.
 * Notice: For system which loop back the WDTRST_N pin to BMC SRST pin.
 * the value of AST_SYSRESET_WITH_SOC does not matter, because
 * no matter how the BMC will get full reset by SRST pin.
 */
#define AST_SYSRESET_WITH_SOC 0x030f1ff1

#endif	/* __CONFIG_H */
