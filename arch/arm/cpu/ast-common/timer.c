/*
 * (C) Copyright 2016-present
 * ASPEED and OpenBMC development.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

#if CONFIG_ASPEED_TIMER_CLK < CONFIG_SYS_HZ
#error "CONFIG_ASPEED_TIMER_CLK must be as large as CONFIG_SYS_HZ"
#endif

#define TIMER_LOAD_VAL 0xFFFFFFFF
#define CLK_PER_HZ (CONFIG_ASPEED_TIMER_CLK / CONFIG_SYS_HZ)

/* macro to read the 32 bit timer */
#define READ_CLK (*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 0))
#define READ_TIMER (READ_CLK / CLK_PER_HZ)

static ulong timestamp;
static ulong lastdec;

int timer_init(void)
{
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 4) = TIMER_LOAD_VAL;
	/* enable timer1 */
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 0x30) = 0x3;

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

/* delay x seconds AND preserve advance timestamp value */
void __udelay(unsigned long usec)
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

void reset_timer_masked(void)
{
	lastdec = READ_TIMER;  /* capure current decrementer value time */
	timestamp = 0;         /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = READ_TIMER;

	/* normal mode (non roll) */
	if (lastdec >= now) {
		/* move stamp forward with absolute diff ticks */
		timestamp += lastdec - now;
	} else {
		/*
		 * We have overflow of the count down timer:
		 * nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"
		 */
		timestamp += lastdec + (TIMER_LOAD_VAL / CLK_PER_HZ) - now;
	}
	lastdec = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
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
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
