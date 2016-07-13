/*
 * Copyright 2016 IBM Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __AST_G4_NCSI_CONFIG_H
#define __AST_G4_NCSI_CONFIG_H

#define CONFIG_ARCH_AST2400
#define CONFIG_SYS_LOAD_ADDR		0x43000000

#define CONFIG_MISC_INIT_R

#include <configs/ast-common.h>

/* Ethernet */
#define CONFIG_LIB_RAND
#define CONFIG_ASPEEDNIC

/* platform.S settings */
#define CONFIG_CPU_420			1
#define CONFIG_DRAM_528			1

#endif	/* __AST_G4_NCSI_CONFIG_H */
