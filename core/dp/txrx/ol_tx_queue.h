/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/**
 * @file ol_tx_queue.h
 * @brief API definitions for the tx frame queue module within the data SW.
 */
#ifndef _OL_TX_QUEUE__H_
#define _OL_TX_QUEUE__H_

#include <cdf_nbuf.h>           /* cdf_nbuf_t */
#include <ol_txrx_types.h>      /* ol_txrx_vdev_t, etc. */
#include <qdf_types.h>          /* bool */

/*--- function prototypes for optional queue log feature --------------------*/
#if defined(ENABLE_TX_QUEUE_LOG)

void
ol_tx_queue_log_enqueue(struct ol_txrx_pdev_t *pdev,
			struct ol_txrx_msdu_info_t *msdu_info,
			int frms, int bytes);
void
ol_tx_queue_log_dequeue(struct ol_txrx_pdev_t *pdev,
			struct ol_tx_frms_queue_t *txq, int frms, int bytes);
void
ol_tx_queue_log_free(struct ol_txrx_pdev_t *pdev,
		     struct ol_tx_frms_queue_t *txq,
		     int tid, int frms, int bytes);
#define OL_TX_QUEUE_LOG_ENQUEUE ol_tx_queue_log_enqueue
#define OL_TX_QUEUE_LOG_DEQUEUE ol_tx_queue_log_dequeue
#define OL_TX_QUEUE_LOG_FREE    ol_tx_queue_log_free

#else

#define OL_TX_QUEUE_LOG_ENQUEUE(pdev, msdu_info, frms, bytes)   /* no-op */
#define OL_TX_QUEUE_LOG_DEQUEUE(pdev, txq, frms, bytes) /* no-op */
#define OL_TX_QUEUE_LOG_FREE(pdev, txq, tid, frms, bytes)       /* no-op */

#endif /* TXRX_DEBUG_LEVEL > 5 */

#define ol_tx_enqueue(pdev, txq, tx_desc, tx_msdu_info) /* no-op */
#define ol_tx_dequeue(pdev, ext_tid, txq, head, num_frames, credit, bytes) 0
#define ol_tx_queue_free(pdev, txq, tid)        /* no-op */
#define ol_tx_queue_discard(pdev, flush, tx_descs)      /* no-op */

void
ol_tx_queue_log_sched(struct ol_txrx_pdev_t *pdev,
		      int credit,
		      int *num_active_tids,
		      uint32_t **active_bitmap, uint8_t **data);

#define OL_TX_QUEUE_LOG_SCHED( \
		pdev, credit, num_active_tids, active_bitmap, data)

#define ol_tx_queues_display(pdev)      /* no-op */

#define ol_tx_queue_decs_reinit(peer, peer_id)  /* no-op */

#ifdef QCA_SUPPORT_TX_THROTTLE
/**
 * @brief - initialize the throttle context
 * @param pdev - the physical device object, which stores the txqs
 */
void ol_tx_throttle_init(struct ol_txrx_pdev_t *pdev);
#else
#define ol_tx_throttle_init(pdev)       /*no op */
#endif
#endif /* _OL_TX_QUEUE__H_ */
