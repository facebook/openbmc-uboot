/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/fbobmc-ast-g6.h>

/*
 * Override watchdog timeout from <configs/fbobmc-ast-g6.h>
 */
#undef CONFIG_ASPEED_WATCHDOG_TIMEOUT
#define CONFIG_ASPEED_WATCHDOG_TIMEOUT	(10*60) /* 10 minutes */

#endif	/* __CONFIG_H */
