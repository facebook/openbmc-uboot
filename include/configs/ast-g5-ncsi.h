/*
 * Copyright 2016 IBM Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __AST_G5_NCSI_CONFIG_H
#define __AST_G5_NCSI_CONFIG_H

#define CONFIG_ARCH_AST2500
#define CONFIG_SYS_LOAD_ADDR		0x83000000

#include <configs/ast-common.h>

/* arm1176/start.S */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* Ethernet */
#define CONFIG_LIB_RAND
#define CONFIG_ASPEEDNIC

/* platform.S settings */
#define	CONFIG_DRAM_ECC_SIZE		0x10000000

#endif	/* __AST_G5_NCSI_CONFIG_H */
