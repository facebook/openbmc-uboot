/*
 * (C) Copyright 2022-Present, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#define DEBUG
#include <common.h>
#include <dm.h>
#include "tpm-event.h"

static const union tpm_event_log_end_mark good_logend = {
	.magic = TPM_EVENT_LOG_MAGIC_CODE,
	.fmtver = TPM_EVENT_LOG_FMT_VERSION,
};

static int check_log_sanity(volatile struct tpm_event_log *eventlog)
{
	union tpm_event_log_end_mark *log_end;
	uint32_t len = eventlog->len;

	/* This could happen when SPL does not support TPM event log
	 * so the SRAM of TPM_EVENT_LOG_ADDR has uninitalized value
	 */
	if (len > TPM_EVENT_LOG_MAX_LEN)
		return -EINVAL;
	log_end = (union tpm_event_log_end_mark *)(&eventlog->data[len]);

	return (good_logend.endmark == log_end->endmark) ? 0 : -EINVAL;
}

static inline void put_log_endmark(volatile struct tpm_event_log *eventlog)
{
	union tpm_event_log_end_mark *log_end =
		(union tpm_event_log_end_mark *)(&eventlog->data[eventlog->len]);

	log_end->endmark = good_logend.endmark;
}

int log_tpm_event(union tpm_event_index eventidx, uint8_t algo, uint8_t *digest,
		  uint32_t digest_len)
{
	volatile struct tpm_event_log *eventlog =
		(volatile struct tpm_event_log *)(TPM_EVENT_LOG_ADDR);
	uint32_t len;
	struct tpm_event_log_record *event;

	/* sanity check existing event log */
	if (check_log_sanity(eventlog)) {
		printf("skip log tpm-event as event log not init or support ");
		printf("m_measue=%u, m_pcr=%u, m_index=%u, algo=%u ",
		       eventidx.m_measure, eventidx.m_pcrid, eventidx.m_index,
		       algo);
		return 0;
	}

	len = eventlog->len;
	if ((measure_unknown == eventidx.m_measure) ||
	    (eventidx.m_measure >= measure_unsupport)) {
		printf("skip log tpm-event with unknown measurement ");
		printf("m_measue=%u, m_pcr=%u, m_index=%u, algo=%u ",
		       eventidx.m_measure, eventidx.m_pcrid, eventidx.m_index,
		       algo);
		return 0;
	}
	/* make sure enough space to log this event*/
	len += offsetof(struct tpm_event_log_record, digest) + digest_len;
	if (len > TPM_EVENT_LOG_MAX_LEN) {
		printf("no space to log event(%x), curlen=%x, newlen=%x ",
		       eventidx.index, eventlog->len, len);
		return 0;
	}

	/* log the event */
	event = (struct tpm_event_log_record *)(&eventlog->data[eventlog->len]);
	event->measurement = eventidx.m_measure;
	event->pcrid = eventidx.m_pcrid;
	event->algo = algo;
	event->index = eventidx.m_index;
	memcpy(event->digest, digest, digest_len);
	eventlog->len = len;
	put_log_endmark(eventlog);

	printf("log (measue=%u pcr=%u algo=%u, index=%u diglen=%u len=%u) ",
	       event->measurement, event->pcrid, event->algo, event->index,
	       digest_len, eventlog->len);

	return 0;
}

void reset_tpm_event_log(void)
{
	volatile struct tpm_event_log *eventlog =
		(volatile struct tpm_event_log *)(TPM_EVENT_LOG_ADDR);
	printf("tpm event log reset, previous len=%u\n", eventlog->len);
	memset((void *)TPM_EVENT_LOG_ADDR, 0, TPM_EVENT_LOG_SRAM_SIZE);
	eventlog->len = 0;
	put_log_endmark(eventlog);
}
