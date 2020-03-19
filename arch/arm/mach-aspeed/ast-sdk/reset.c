/*
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
*/

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/ast_scu.h>

void reset_cpu(ulong addr)
{
	u32 reset_mask = 0x3; /* Clear after | Enable */
	/* Some boards may request the reset to trigger the EXT reset GPIO.
	 * On Linux this is defined as WDT_CTRL_B_EXT.
	 */
#ifdef CONFIG_ASPEED_WATCHDOG_TRIGGER_GPIO
	writel(__raw_readl(AST_SCU_BASE + 0xA8) | 0x4, AST_SCU_BASE + 0xA8);
	reset_mask |= 0x08; /* Ext */
#endif

	ast_wdt_reset(0x10, reset_mask);

	while (1)
	/*nothing*/;
}
