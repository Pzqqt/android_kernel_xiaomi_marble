/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include "htc_debug.h"
#include "htc_internal.h"
#include "htc_credit_history.h"
#include <qdf_lock.h>

struct HTC_CREDIT_HISTORY {
	enum htc_credit_exchange_type type;
	uint64_t time;
	uint32_t tx_credit;
	uint32_t htc_tx_queue_depth;
};

static qdf_spinlock_t g_htc_credit_lock;
static uint32_t g_htc_credit_history_idx;
static uint32_t g_htc_credit_history_length;
static
struct HTC_CREDIT_HISTORY htc_credit_history_buffer[HTC_CREDIT_HISTORY_MAX];


#ifdef QCA_WIFI_NAPIER_EMULATION
#define HTC_EMULATION_DELAY_IN_MS 20
/**
 * htc_add_delay(): Adds a delay in before proceeding, only for emulation
 *
 * Return: None
 */
static inline void htc_add_emulation_delay(void)
{
	qdf_mdelay(HTC_EMULATION_DELAY_IN_MS);
}
#else
static inline void htc_add_emulation_delay(void)
{
}
#endif

void htc_credit_history_init(void)
{
	qdf_spinlock_create(&g_htc_credit_lock);
	g_htc_credit_history_idx = 0;
	g_htc_credit_history_length = 0;
}

/**
 * htc_credit_record() - records tx que state & credit transactions
 * @type:		type of echange can be HTC_REQUEST_CREDIT
 *			or HTC_PROCESS_CREDIT_REPORT
 * @tx_credits:		current number of tx_credits
 * @htc_tx_queue_depth:	current hct tx queue depth
 *
 * This function records the credits and pending commands whenever a command is
 * sent or credits are returned.  Call this after the credits have been updated
 * according to the transaction.  Call this before dequeing commands.
 *
 * Consider making this function accept an HTC_ENDPOINT and find the current
 * credits and queue depth itself.
 *
 */
void htc_credit_record(enum htc_credit_exchange_type type, uint32_t tx_credit,
		       uint32_t htc_tx_queue_depth)
{
	qdf_spin_lock_bh(&g_htc_credit_lock);
	if (g_htc_credit_history_idx >= HTC_CREDIT_HISTORY_MAX)
		g_htc_credit_history_idx = 0;

	htc_credit_history_buffer[g_htc_credit_history_idx].type = type;
	htc_credit_history_buffer[g_htc_credit_history_idx].time =
		qdf_get_log_timestamp();
	htc_credit_history_buffer[g_htc_credit_history_idx].tx_credit =
		tx_credit;
	htc_credit_history_buffer[g_htc_credit_history_idx].htc_tx_queue_depth =
		htc_tx_queue_depth;

	g_htc_credit_history_idx++;
	g_htc_credit_history_length++;
	htc_add_emulation_delay();
	qdf_spin_unlock_bh(&g_htc_credit_lock);
}

void htc_print_credit_history(HTC_HANDLE htc, uint32_t count,
			      qdf_abstract_print *print, void *print_priv)
{
	uint32_t idx;

	print(print_priv, "HTC Credit History (count %u)", count);
	qdf_spin_lock_bh(&g_htc_credit_lock);

	if (count > HTC_CREDIT_HISTORY_MAX)
		count = HTC_CREDIT_HISTORY_MAX;
	if (count > g_htc_credit_history_length)
		count = g_htc_credit_history_length;

	/* subtract count from index, and wrap if necessary */
	idx = HTC_CREDIT_HISTORY_MAX + g_htc_credit_history_idx - count;
	idx %= HTC_CREDIT_HISTORY_MAX;

	print(print_priv,
	      "Time (seconds)     Type                         Credits    Queue Depth");
	while (count) {
		struct HTC_CREDIT_HISTORY *hist =
						&htc_credit_history_buffer[idx];
		uint64_t secs, usecs;

		qdf_log_timestamp_to_secs(hist->time, &secs, &usecs);
		print(print_priv, "% 8lld.%06lld    %-25s    %-7.d    %d",
		      secs,
		      usecs,
		      htc_credit_exchange_type_str(hist->type),
		      hist->tx_credit,
		      hist->htc_tx_queue_depth);

		--count;
		++idx;
		if (idx >= HTC_CREDIT_HISTORY_MAX)
			idx = 0;
	}

	qdf_spin_unlock_bh(&g_htc_credit_lock);
}
