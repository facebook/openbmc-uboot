/*
 * (C) Copyright 2019-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef _BOARD_ASPEED_AST_G5_UTIL_H
#define _BOARD_ASPEED_AST_G5_UTIL_H

int watchdog_init(u32 timeout_sec);
int pfr_checkpoint(uint cmd);

#endif/*_BOARD_ASPEED_AST_G5_UTIL_H*/
