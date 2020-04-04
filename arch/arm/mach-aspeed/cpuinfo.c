// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/aspeed_scu_info.h>
#include <asm/arch/platform.h>

int print_cpuinfo(void)
{
	aspeed_print_soc_id();
	aspeed_print_sysrst_info();
	aspeed_print_security_info();
	aspeed_print_2nd_wdt_mode();
	aspeed_print_spi_strap_mode();
	aspeed_print_espi_mode();
	aspeed_print_mac_info();
	return 0;
}
