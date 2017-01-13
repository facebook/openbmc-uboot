/*
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
*/

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

void reset_cpu(ulong addr)
{
	__raw_writel(0x10 , AST_WDT_BASE+0x04);
	__raw_writel(0x4755, AST_WDT_BASE+0x08);
	u32 val = 0x3; /* Clear after | Enable */
	/* Some boards may request the reset to trigger the EXT reset GPIO.
	 * On Linux this is defined as WDT_CTRL_B_EXT.
	 */
#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
	val |= 0x08; /* Ext */
#endif
	__raw_writel(val, AST_WDT_BASE+0x0c); /* reset the full chip */

	while (1)
	/*nothing*/;
}
