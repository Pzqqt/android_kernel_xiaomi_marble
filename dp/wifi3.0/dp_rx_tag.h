/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RX_TAG_H_
#define _DP_RX_TAG_H_

#include "dp_internal.h"
#include "dp_types.h"

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/**
 * dp_update_pdev_rx_protocol_tag - Add/remove a protocol tag that should be
 * applied to the desired protocol type packets
 * @txrx_pdev_handle: cdp_pdev handle
 * @enable_rx_protocol_tag - bitmask that indicates what protocol types
 * are enabled for tagging. zero indicates disable feature, non-zero indicates
 * enable feature
 * @protocol_type: new protocol type for which the tag is being added
 * @tag: user configured tag for the new protocol
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_update_pdev_rx_protocol_tag(struct cdp_pdev *pdev_handle,
			       uint32_t enable_rx_protocol_tag,
			       uint16_t protocol_type,
			       uint16_t tag);

/**
 * dp_rx_update_protocol_tag() - Reads CCE metadata from the RX MSDU end TLV
 *                              and set the corresponding tag in QDF packet
 * @soc: core txrx main context
 * @vdev: vdev on which the packet is received
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @rx_tlv_hdr: rBbase address where the RX TLVs starts
 * @ring_index: REO ring number, not used for error & monitor ring
 * @is_reo_exception: flag to indicate if rx from REO ring or exception ring
 * @is_update_stats: flag to indicate whether to update stats or not
 * Return: void
 */
void
dp_rx_update_protocol_tag(struct dp_soc *soc, struct dp_vdev *vdev,
			  qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr,
			  uint16_t ring_index,
			  bool is_reo_exception, bool is_update_stats);

#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
/**
 * dp_dump_pdev_rx_protocol_tag_stats - dump the number of packets tagged for
 * given protocol type (RX_PROTOCOL_TAG_ALL indicates for all protocol)
 * @pdev_handle: cdp_pdev handle
 * @protocol_type: protocol type for which stats should be displayed
 *
 * Return: none
 */
void
dp_dump_pdev_rx_protocol_tag_stats(struct cdp_pdev *pdev_handle,
				   uint16_t protocol_type);
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

#ifdef WLAN_SUPPORT_RX_FLOW_TAG
/**
 * dp_rx_update_flow_tag() - Reads FSE metadata from the RX MSDU end TLV
 *                           and set the corresponding tag in QDF packet
 * @soc: core txrx main context
 * @vdev: vdev on which the packet is received
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @rx_tlv_hdr: base address where the RX TLVs starts
 * @is_update_stats: flag to indicate whether to update stats or not
 *
 * Return: void
 */
void
dp_rx_update_flow_tag(struct dp_soc *soc, struct dp_vdev *vdev,
		      qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr, bool update_stats);

/**
 * dp_set_rx_flow_tag - add/delete a flow
 * @pdev_handle: cdp_pdev handle
 * @flow_info: flow tuple that is to be added to/deleted from flow search table
 *
 * Return: 0 for success, nonzero for failure
 */
QDF_STATUS
dp_set_rx_flow_tag(struct cdp_pdev *pdev_handle,
		   struct cdp_rx_flow_info *flow_info);

/**
 * dp_dump_rx_flow_tag_stats - dump the number of packets tagged for
 * given flow 5-tuple
 * @pdev_handle: cdp_pdev handle
 * @flow_info: flow 5-tuple for which stats should be displayed
 *
 * Return: 0 for success, nonzero for failure
 */
QDF_STATUS
dp_dump_rx_flow_tag_stats(struct cdp_pdev *pdev_handle,
			  struct cdp_rx_flow_info *flow_info);

/**
 * dp_rx_update_rx_flow_tag_stats() - Update stats for given flow index
 * @pdev: TXRX pdev context for which stats should be incremented
 * @flow_index: flow index for which the stats should be incremented
 *
 * Return: void
 */
extern QDF_STATUS
dp_rx_flow_update_fse_stats(struct dp_pdev *pdev, uint32_t flow_id);

static inline void
dp_rx_update_rx_flow_tag_stats(struct dp_pdev *pdev,
			       uint32_t flow_index)
{
	dp_rx_flow_update_fse_stats(pdev, flow_index);
}
#endif /* WLAN_SUPPORT_RX_FLOW_TAG */

#if defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG) ||\
	defined(WLAN_SUPPORT_RX_FLOW_TAG)
/**
 * dp_rx_mon_update_protocol_flow_tag() - Performs necessary checks for monitor
 *                                       mode and then tags appropriate packets
 * @soc: core txrx main context
 * @vdev: pdev on which packet is received
 * @msdu: QDF packet buffer on which the protocol tag should be set
 * @rx_desc: base address where the RX TLVs start
 * Return: void
 */
void dp_rx_mon_update_protocol_flow_tag(struct dp_soc *soc,
					struct dp_pdev *dp_pdev,
					qdf_nbuf_t msdu, void *rx_desc);
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG || WLAN_SUPPORT_RX_FLOW_TAG */

#endif /* _DP_RX_TAG_H_ */
