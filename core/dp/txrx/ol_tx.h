/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 * @file ol_tx.h
 * @brief Internal definitions for the high-level tx module.
 */
#ifndef _OL_TX__H_
#define _OL_TX__H_

#include <cdf_nbuf.h>           /* cdf_nbuf_t */
#include <cdf_lock.h>
#include <ol_txrx_api.h>        /* ol_txrx_vdev_handle */

#include <ol_txrx_types.h>      /* ol_tx_desc_t, ol_txrx_msdu_info_t */

cdf_nbuf_t ol_tx_ll(ol_txrx_vdev_handle vdev, cdf_nbuf_t msdu_list);
#ifdef WLAN_FEATURE_FASTPATH
cdf_nbuf_t ol_tx_ll_fast(ol_txrx_vdev_handle vdev, cdf_nbuf_t msdu_list);
#endif

cdf_nbuf_t ol_tx_ll_queue(ol_txrx_vdev_handle vdev, cdf_nbuf_t msdu_list);

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
#define OL_TX_LL ol_tx_ll_queue
#else
#define OL_TX_LL ol_tx_ll
#endif

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
void ol_tx_vdev_ll_pause_queue_send(void *context);
void ol_tx_pdev_ll_pause_queue_send_all(struct ol_txrx_pdev_t *pdev);
#else
static inline void ol_tx_vdev_ll_pause_queue_send(void *context)
{
	return;
}
static inline
void ol_tx_pdev_ll_pause_queue_send_all(struct ol_txrx_pdev_t *pdev)
{
	return;
}
#endif
cdf_nbuf_t
ol_tx_non_std_ll(ol_txrx_vdev_handle data_vdev,
		 enum ol_tx_spec tx_spec, cdf_nbuf_t msdu_list);

cdf_nbuf_t
ol_tx_reinject(struct ol_txrx_vdev_t *vdev, cdf_nbuf_t msdu, uint16_t peer_id);

void ol_txrx_mgmt_tx_complete(void *ctxt, cdf_nbuf_t netbuf, int err);


#if defined(FEATURE_TSO)
void ol_tso_seg_list_init(struct ol_txrx_pdev_t *pdev, uint32_t num_seg);
void ol_tso_seg_list_deinit(struct ol_txrx_pdev_t *pdev);
#else
static inline void ol_tso_seg_list_init(struct ol_txrx_pdev_t *pdev,
	uint32_t num_seg)
{
	return;
}
static inline void ol_tso_seg_list_deinit(struct ol_txrx_pdev_t *pdev)
{
	return;
}
#endif

#endif /* _OL_TX__H_ */
