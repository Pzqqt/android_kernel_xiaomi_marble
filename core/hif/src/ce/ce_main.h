/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
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

#ifndef __CE_H__
#define __CE_H__

#include "cdf_atomic.h"
#include "cdf_lock.h"
#include "hif.h"

#define CE_HTT_T2H_MSG 1
#define CE_HTT_H2T_MSG 4

/**
 * enum ce_id_type
 *
 * @ce_id_type: Copy engine ID
 */
enum ce_id_type {
	CE_ID_0,
	CE_ID_1,
	CE_ID_2,
	CE_ID_3,
	CE_ID_4,
	CE_ID_5,
	CE_ID_6,
	CE_ID_7,
	CE_ID_8,
	CE_ID_9,
	CE_ID_10,
	CE_ID_11,
	CE_ID_MAX
};

enum ol_ath_hif_pkt_ecodes {
	HIF_PIPE_NO_RESOURCE = 0
};

struct HIF_CE_state;

/* Per-pipe state. */
struct HIF_CE_pipe_info {
	/* Handle of underlying Copy Engine */
	struct CE_handle *ce_hdl;

	/* Our pipe number; facilitiates use of pipe_info ptrs. */
	uint8_t pipe_num;

	/* Convenience back pointer to HIF_CE_state. */
	struct HIF_CE_state *HIF_CE_state;

	/* Instantaneous number of receive buffers that should be posted */
	atomic_t recv_bufs_needed;
	cdf_size_t buf_sz;
	cdf_spinlock_t recv_bufs_needed_lock;

	cdf_spinlock_t completion_freeq_lock;
	/* Limit the number of outstanding send requests. */
	int num_sends_allowed;

	/* adding three counts for debugging ring buffer errors */
	uint32_t nbuf_alloc_err_count;
	uint32_t nbuf_dma_err_count;
	uint32_t nbuf_ce_enqueue_err_count;
};

/**
 * struct ce_tasklet_entry
 *
 * @intr_tq: intr_tq
 * @ce_id: ce_id
 * @inited: inited
 * @hif_ce_state: hif_ce_state
 * @from_irq: from_irq
 */
struct ce_tasklet_entry {
	struct tasklet_struct intr_tq;
	enum ce_id_type ce_id;
	bool inited;
	void *hif_ce_state;
	bool from_irq;
};

struct HIF_CE_state {
	struct ol_softc *scn;
	bool started;
	struct ce_tasklet_entry tasklets[CE_COUNT_MAX];
	cdf_spinlock_t keep_awake_lock;
	unsigned int keep_awake_count;
	bool verified_awake;
	bool fake_sleep;
	cdf_softirq_timer_t sleep_timer;
	bool sleep_timer_init;
	unsigned long sleep_ticks;

	/* Per-pipe state. */
	struct HIF_CE_pipe_info pipe_info[CE_COUNT_MAX];
	/* to be activated after BMI_DONE */
	struct hif_msg_callbacks msg_callbacks_pending;
	/* current msg callbacks in use */
	struct hif_msg_callbacks msg_callbacks_current;

	void *claimedContext;

	/* Target address used to signal a pending firmware event */
	uint32_t fw_indicator_address;

	/* Copy Engine used for Diagnostic Accesses */
	struct CE_handle *ce_diag;
};
#endif /* __CE_H__ */
