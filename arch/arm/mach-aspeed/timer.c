/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
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

#if CONFIG_ASPEED_TIMER_CLK < CONFIG_SYS_HZ
#error "CONFIG_ASPEED_TIMER_CLK must be as large as CONFIG_SYS_HZ"
#endif

#define TIMER_LOAD_VAL 0xffffffff
#define CLK_PER_HZ (CONFIG_ASPEED_TIMER_CLK / CONFIG_SYS_HZ)

/* macro to read the 32 bit timer */
#define READ_CLK (*(volatile ulong *)(AST_TIMER_BASE + 0))
#define READ_TIMER (READ_CLK / CLK_PER_HZ)

static ulong timestamp;
static ulong lastdec;

int timer_init (void)
{
	*(volatile ulong *)(AST_TIMER_BASE + 4)    = TIMER_LOAD_VAL;
	*(volatile ulong *)(AST_TIMER_BASE + 0x30) = 0x3; /* enable timer1 */

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */

void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void set_timer (ulong t)
{
	timestamp = t;
}

/* delay x useconds AND perserve advance timstamp value */
void __udelay (unsigned long usec)
{
	ulong last = READ_CLK;
	ulong clks;
	ulong elapsed = 0;

	/* translate usec to clocks */
	clks = (usec / 1000) * CLK_PER_HZ;
	clks += (usec % 1000) * CLK_PER_HZ / 1000;

	while (clks > elapsed) {
		ulong now = READ_CLK;
		if (now <= last) {
			elapsed += last - now;
		} else {
			elapsed += TIMER_LOAD_VAL - (now - last);
		}
		last = now;
	}
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER;  /* capure current decrementer value time */
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;	/* current tick value */

	if (lastdec >= now) {	/* normal mode (non roll) */
		 /* move stamp fordward with absolute diff ticks */
		timestamp += lastdec - now;
	} else { /* we have overflow of the count down timer */

		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"... it could also roll and
		 * cause problems.
		 */
		timestamp += lastdec + (TIMER_LOAD_VAL / CLK_PER_HZ) - now;
	}
	lastdec = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
  __udelay(usec);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	return CONFIG_SYS_HZ;
}
