
/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Facebook Inc.
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
#define CONFIG_ENV_SIZE 0x10000
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET 0xE0000

/*
 * Autoboot configuration
 */
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds (stop with 'Delete' key)...\n"
#define CONFIG_AUTOBOOT_STOP_STR	"\x1b\x5b\x33\x7e"	/* 'Delete', ESC[3~ */
#define CONFIG_AUTOBOOT_KEYED

#endif	/* __CONFIG_H */
