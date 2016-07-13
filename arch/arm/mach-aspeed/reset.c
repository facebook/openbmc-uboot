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
	__raw_writel(0x3, AST_WDT_BASE+0x0c);

	while (1)
	/*nothing*/;
}
