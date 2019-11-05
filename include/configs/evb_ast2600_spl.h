/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/aspeed-common.h>

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x300000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x5000000)

#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* Memory Info */
#define CONFIG_SYS_LOAD_ADDR		0x83000000

/* Environment */
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_OFFSET		0x90000
#define CONFIG_ENV_SECT_SIZE		(4 << 10)

#ifdef CONFIG_SPL_TINY
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_NS16550_REG_SIZE 2
#endif
#endif

/* SPL */
#define CONFIG_SPL_TEXT_BASE		0x00000000
#define CONFIG_SPL_MAX_SIZE			0x00010000
#define CONFIG_SPL_STACK			0x10010000

#define CONFIG_SPL_BSS_START_ADDR	0x90000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000

#endif	/* __CONFIG_H */
