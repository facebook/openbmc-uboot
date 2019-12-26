/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 *
 * Copyright 2016 IBM Corporation
 * (C) Copyright 2016 Google, Inc
 */

#ifndef __ASPEED_COMMON_CONFIG_H
#define __ASPEED_COMMON_CONFIG_H

#include <asm/arch/platform.h>

#define CONFIG_BOOTFILE		"all.bin"

#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.0.45
#define CONFIG_SERVERIP		192.168.0.81

/* Misc CPU related */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* Enable cache controller */
#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_SYS_SDRAM_BASE		ASPEED_DRAM_BASE

#ifdef CONFIG_PRE_CON_BUF_SZ
#define CONFIG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE + CONFIG_PRE_CON_BUF_SZ)
#define CONFIG_SYS_INIT_RAM_SIZE	(36*1024 - CONFIG_PRE_CON_BUF_SZ)
#else
#define CONFIG_SYS_INIT_RAM_ADDR	(ASPEED_SRAM_BASE)
#define CONFIG_SYS_INIT_RAM_SIZE	(36*1024)
#endif

#define SYS_INIT_RAM_END \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(SYS_INIT_RAM_END - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_BOOTMAPSZ		(256 * 1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(32 << 20)

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */
#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND		"bootm 20080000"
#endif
#define CONFIG_ENV_OVERWRITE

#define CONFIG_SYS_BOOTM_LEN 		(0x800000 * 2)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0"	\
	"spi_dma=no\0" \
	""

/*
 * Ethernet related
 */
#define PHY_ANEG_TIMEOUT		800
#define CONFIG_PHY_GIGE
#endif	/* __ASPEED_COMMON_CONFIG_H */
