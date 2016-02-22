/*
 * Copyright (c) 2012, 2014-2016 The Linux Foundation. All rights reserved.
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
 * @file ol_txrx_osif_api.h
 * @brief Define the host data API functions called by the host OS shim SW.
 */
#ifndef _OL_TXRX_OSIF_API__H_
#define _OL_TXRX_OSIF_API__H_

#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include "cds_sched.h"
#include "ol_txrx_ctrl_api.h"

/**
 * struct ol_rx_cached_buf - rx cached buffer
 * @list: linked list
 * @buf: skb buffer
 */
struct ol_rx_cached_buf {
	struct list_head list;
	qdf_nbuf_t buf;
};

struct txrx_rx_metainfo;

/**
 * struct ol_txrx_desc_type - txrx descriptor type
 * @sta_id: sta id
 * @is_qos_enabled: is station qos enabled
 * @is_wapi_supported: is station wapi supported
 */
struct ol_txrx_desc_type {
	uint8_t sta_id;
	uint8_t is_qos_enabled;
	uint8_t is_wapi_supported;
};


typedef QDF_STATUS (*ol_rx_callback_fp)(void *p_cds_gctx,
					 qdf_nbuf_t pDataBuff,
					 uint8_t ucSTAId);

typedef void (*ol_tx_pause_callback_fp)(uint8_t vdev_id,
					enum netif_action_type action,
					enum netif_reason_type reason);

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
QDF_STATUS ol_txrx_register_pause_cb(ol_tx_pause_callback_fp pause_cb);
#else
static inline
QDF_STATUS ol_txrx_register_pause_cb(ol_tx_pause_callback_fp pause_cb)
{
	return QDF_STATUS_SUCCESS;

}
#endif

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL

int ol_txrx_register_tx_flow_control (uint8_t vdev_id,
	ol_txrx_tx_flow_control_fp flowControl,
	void *osif_fc_ctx);

int ol_txrx_deregister_tx_flow_control_cb(uint8_t vdev_id);

void ol_txrx_flow_control_cb(ol_txrx_vdev_handle vdev,
	bool tx_resume);
bool
ol_txrx_get_tx_resource(uint8_t sta_id,
			unsigned int low_watermark,
			unsigned int high_watermark_offset);

int
ol_txrx_ll_set_tx_pause_q_depth(uint8_t vdev_id, int pause_q_depth);
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * @brief Divide a jumbo TCP frame into smaller segments.
 * @details
 *  For efficiency, the protocol stack above the WLAN driver may operate
 *  on jumbo tx frames, which are larger than the 802.11 MTU.
 *  The OSIF SW uses this txrx API function to divide the jumbo tx TCP frame
 *  into a series of segment frames.
 *  The segments are created as clones of the input jumbo frame.
 *  The txrx SW generates a new encapsulation header (ethernet + IP + TCP)
 *  for each of the output segment frames.  The exact format of this header,
 *  e.g. 802.3 vs. Ethernet II, and IPv4 vs. IPv6, is chosen to match the
 *  header format of the input jumbo frame.
 *  The input jumbo frame is not modified.
 *  After the ol_txrx_osif_tso_segment returns, the OSIF SW needs to perform
 *  DMA mapping on each of the segment network buffers, and also needs to
 *
 * @param txrx_vdev - which virtual device will transmit the TSO segments
 * @param max_seg_payload_bytes - the maximum size for the TCP payload of
 *      each segment frame.
 *      This does not include the ethernet + IP + TCP header sizes.
 * @param jumbo_tcp_frame - jumbo frame which needs to be cloned+segmented
 * @return
 *      NULL if the segmentation fails, - OR -
 *      a NULL-terminated list of segment network buffers
 */
qdf_nbuf_t ol_txrx_osif_tso_segment(ol_txrx_vdev_handle txrx_vdev,
				    int max_seg_payload_bytes,
				    qdf_nbuf_t jumbo_tcp_frame);

qdf_nbuf_t ol_tx_data(ol_txrx_vdev_handle data_vdev, qdf_nbuf_t skb);

#ifdef IPA_OFFLOAD
qdf_nbuf_t ol_tx_send_ipa_data_frame(void *vdev,
			qdf_nbuf_t skb);
#endif

QDF_STATUS ol_txrx_register_peer
			(struct ol_txrx_desc_type *sta_desc);

QDF_STATUS ol_txrx_clear_peer(uint8_t sta_id);

QDF_STATUS ol_txrx_change_peer_state(uint8_t sta_id,
				     enum ol_txrx_peer_state sta_state,
				     bool roam_synch_in_progress);

void ol_rx_data_process(struct ol_txrx_peer_t *peer,
			qdf_nbuf_t rx_buf_list);

void ol_txrx_flush_rx_frames(struct ol_txrx_peer_t *peer,
			     bool drop);

#if defined(FEATURE_LRO)
void ol_register_lro_flush_cb(void (handler)(void *), void *data);
void ol_deregister_lro_flush_cb(void);
#endif
#endif /* _OL_TXRX_OSIF_API__H_ */
