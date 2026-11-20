// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once

#include <chiaki/takion.h>

#define TAKION_AV_REORDER_TIMEOUT_US 16000

typedef struct
{
	int64_t received_us;
	uint8_t base_type;
	uint8_t *buf;
	size_t buf_size;
	ChiakiTakionAVPacket packet;
} TakionAVPacketEntry;

// Called when the head is missing. A buffered packet after the gap proves
// that the gap already existed when that packet arrived.
static inline uint64_t takion_av_expired_gap(ChiakiReorderQueue *queue, int64_t now,
		int64_t *wait_start, uint64_t *wait_seq)
{
	if(*wait_start && *wait_seq == queue->begin && now - *wait_start < TAKION_AV_REORDER_TIMEOUT_US)
		return 0;

	uint64_t first = queue->count;
	int64_t oldest = now;
	for(uint64_t i = 1; i < queue->count; i++)
	{
		uint64_t seq;
		void *user;
		if(chiaki_reorder_queue_peek(queue, i, &seq, &user))
		{
			const TakionAVPacketEntry *entry = user;
			if(first == queue->count)
				first = i;
			if(entry->received_us < oldest)
				oldest = entry->received_us;
		}
	}
	*wait_start = oldest;
	*wait_seq = queue->begin;
	return first < queue->count && now - oldest >= TAKION_AV_REORDER_TIMEOUT_US ? first : 0;
}
