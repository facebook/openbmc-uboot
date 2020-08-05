// SPDX-License-Identifier: GPL-2.0+
/*
 * Hacking to have a common function to dump buffer
 *
 */
#define LOG_DEBUG
#include <common.h>
#include <log.h>

DECLARE_GLOBAL_DATA_PTR;

void _log_debug_buffer(const u8 *buf, const size_t length)
{
	int fmt = gd->log_fmt;
	size_t i;

	gd->log_fmt = (1 << LOGF_MSG);
	for (i = 0; i < length; i++) {
		if ((i & 0xF) == 0) log_debug("0x%08x: ", i);
		log_debug(" %02x", buf[i]);
		if ((i & 0xF) == 0xF) log_debug("\n");
	}
	if (length & 0xF) log_debug("\n");
	gd->log_fmt = fmt;
}
