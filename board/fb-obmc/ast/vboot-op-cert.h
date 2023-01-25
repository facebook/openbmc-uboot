/*
 * (C) Copyright 2023-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H
#define _BOARD_FBOBMC_AST_VBOOT_OP_CERT_H

#ifdef CONFIG_GIU_HW_SUPPORT
#  define VBOOT_OP_CERT_ADDR (0x10014000)
#endif

enum GIU_MODE {
	GIU_NONE = 0,
	GIU_CERT = 1,
	GIU_OPEN = 0xEA,
};

#endif
