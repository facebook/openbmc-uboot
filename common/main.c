// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#include <common.h>
#include <autoboot.h>
#include <cli.h>
#include <console.h>
#include <version.h>

extern int watchdog_init(u32 timeout_sec);
extern int pfr_checkpoint(uint cmd);

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

/*
 * Custom tests need the watchdog re-initialized.
 */

static void run_preboot_environment_command(void)
{
#ifdef CONFIG_PREBOOT
	char *p;

	p = env_get("preboot");
	if (p != NULL) {
		int prev = 0;

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
#endif /* CONFIG_PREBOOT */
}

/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	int ret;
	const char *s;
#ifdef CONFIG_CMD_MEMTEST2
	char *mtest;
#endif /* CONFIG_CMD_MEMTEST2 */

#ifdef CONFIG_CMD_CS1TEST
    char *cs1test;
#endif /*CONFIG_CMD_CS1TEST*/

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

#ifdef CONFIG_CMD_MEMTEST2
	mtest = env_get("do_mtest");
	if(!(strcmp(mtest,"obmtest"))) {
		run_command(mtest,0);
		watchdog_init(CONFIG_ASPEED_WATCHDOG_TIMEOUT); // Cover the time consumed by mtest
	}
#endif /* CONFIG_CMD_MEMTEST2 */

#ifdef CONFIG_CMD_CS1TEST
    cs1test = env_get("do_cs1test");
    if(!(strcmp(cs1test,"obcs1test"))) {
            run_command(cs1test,0);
    }
#endif /*CONFIG_CMD_CS1TEST*/

	if (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	run_preboot_environment_command();

	if (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, NULL, NULL);

	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	ret = autoboot_command(s);

	if (!ret) {
#if defined(CONFIG_PRECLICOMMAND)
		s = CONFIG_PRECLICOMMAND;
		run_command_list(s, -1, 0);
#endif
	}

	pfr_checkpoint(0x09);  // CHKPT_COMPLETE
	cli_loop();
	panic("No CLI available");
}
