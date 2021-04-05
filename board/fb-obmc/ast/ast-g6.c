// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Facebook Inc.
 */

#include <common.h>
#include "util.h"

int board_init(void)
{
	watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT);
	vboot_check_enforce();
	return 0;
}