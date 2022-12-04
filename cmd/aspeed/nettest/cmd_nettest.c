// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */


#include <common.h>
#include <command.h>

#include "swfunc.h"
#include "comminf.h"
#include "mem_io.h"
#include "mac_api.h"

extern int mac_test(int argc, char * const argv[], uint32_t mode);

int do_mactest(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	display_lantest_log_msg = 0;
	return mac_test(argc, argv, MODE_DEDICATED);
}

int do_ncsitest (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 0;
	return mac_test(argc, argv, MODE_NCSI);
}

U_BOOT_CMD(mactest, NETESTCMD_MAX_ARGS, 0, do_mactest,
	   "Dedicated LAN test program", NULL);
U_BOOT_CMD(ncsitest, NETESTCMD_MAX_ARGS, 0, do_ncsitest,
	   "Share LAN (NC-SI) test program", NULL);

// ------------------------------------------------------------------------------
int do_mactestd (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 1;
	return mac_test(argc, argv, MODE_DEDICATED);
}

int do_ncsitestd (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	display_lantest_log_msg = 1;
	return mac_test(argc, argv, MODE_NCSI);
}

U_BOOT_CMD(mactestd, NETESTCMD_MAX_ARGS, 0, do_mactestd,
	   "Dedicated LAN test program and display more information", NULL);
U_BOOT_CMD(ncsitestd, NETESTCMD_MAX_ARGS, 0, do_ncsitestd,
	   "Share LAN (NC-SI) test program and display more information", NULL);
