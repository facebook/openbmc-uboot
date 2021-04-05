/*
 * (C) Copyright 2019-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_ASPEED_AST_UTIL_H
#define _BOARD_ASPEED_AST_UTIL_H

int watchdog_init(u32 timeout_sec);
int pfr_checkpoint(uint cmd);

#if defined(CONFIG_ASPEED_AST2600)
void vboot_check_enforce(void);
#endif

#endif/*_BOARD_ASPEED_AST_UTIL_H*/
