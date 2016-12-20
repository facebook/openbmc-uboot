/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define	DEBUG	*/

#include <common.h>
#include <autoboot.h>
#include <cli.h>
#include <console.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

static void run_preboot_environment_command(void)
{
#ifdef CONFIG_PREBOOT
	char *p;

	p = getenv("preboot");
	if (p != NULL) {
# ifdef CONFIG_AUTOBOOT_KEYED
		int prev = disable_ctrlc(1);	/* disable Control C checking */
# endif

		run_command_list(p, -1, 0);

# ifdef CONFIG_AUTOBOOT_KEYED
		disable_ctrlc(prev);	/* restore Control C checking */
# endif
	}
#endif /* CONFIG_PREBOOT */
}

/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;
#ifdef CONFIG_CMD_MEMTEST2
	char *mtest;
#endif /* CONFIG_CMD_MEMTEST2 */

#ifdef CONFIG_CMD_CS1TEST
    char *cs1test;
#endif /*CONFIG_CMD_CS1TEST*/

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

#ifdef CONFIG_CMD_MEMTEST2
	mtest = getenv("do_mtest");
	if(!(strcmp(mtest,"obmtest")))
   	{
		run_command(mtest,0);
	}
#endif /* CONFIG_CMD_MEMTEST2 */

#ifdef CONFIG_CMD_CS1TEST
    cs1test = getenv("do_cs1test");
    if(!(strcmp(cs1test,"obcs1test")))
    {
            run_command(cs1test,0);
    }
#endif /*CONFIG_CMD_CS1TEST*/

#ifdef CONFIG_VERSION_VARIABLE
	setenv("ver", version_string);  /* set version variable */
#endif /* CONFIG_VERSION_VARIABLE */

	cli_init();

	run_preboot_environment_command();

#if defined(CONFIG_UPDATE_TFTP)
	update_tftp(0UL, NULL, NULL);
#endif /* CONFIG_UPDATE_TFTP */

	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	cli_loop();
	panic("No CLI available");
}
