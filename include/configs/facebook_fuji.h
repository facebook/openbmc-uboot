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
#define CONFIG_SYS_LOAD_ADDR		0x83000000

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE 0x20000
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET 0xE0000

#endif	/* __CONFIG_H */
