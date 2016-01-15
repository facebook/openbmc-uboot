/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>

void watchdog_init()
{
#ifdef CONFIG_ASPEED_ENABLE_WATCHDOG
#define AST_WDT_CLK (1*1000*1000) /* 1M clock source */
    u32 reload = AST_WDT_CLK * CONFIG_ASPEED_WATCHDOG_TIMEOUT;
    /* set the reload value */
    __raw_writel(reload, AST_WDT_BASE + 0x04);
    /* magic word to reload */
    __raw_writel(0x4755, AST_WDT_BASE + 0x08);
    /* start the watchdog with 1M clk src and reset whole chip */
    __raw_writel(0x33, AST_WDT_BASE + 0x0c);
    printf("Watchdog: %us\n", CONFIG_ASPEED_WATCHDOG_TIMEOUT);
#endif
}
