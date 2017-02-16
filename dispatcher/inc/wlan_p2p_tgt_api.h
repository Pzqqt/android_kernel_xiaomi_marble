/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Contains p2p south bound interface definitions
 */

#ifndef _WLAN_P2P_TGT_API_H_
#define _WLAN_P2P_TGT_API_H_

#include <qdf_types.h>

struct scan_event;
struct wlan_objmgr_psoc;
struct wlan_objmgr_peer;
enum mgmt_frame_type;

/**
 * tgt_p2p_scan_event_cb() - Callback for scan event
 * @vdev: vdev id
 * @event: event type
 * @arg: registered arguments
 *
 * This function gets called from scan component when getting P2P
 * scan event.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS tgt_p2p_scan_event_cb(uint8_t vdev,
	struct scan_event *event, void *arg);

/**
 * tgt_p2p_mgmt_download_comp_cb() - Callback for mgmt frame tx
 * complete
 * @context: tx context
 * @buf: buffer address
 * @free: need to free or not
 *
 * This function gets called from mgmt tx/rx component when mgmt
 * frame tx complete.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS tgt_p2p_mgmt_download_comp_cb(void *context,
	qdf_nbuf_t buf, bool free);

/**
 * tgt_p2p_mgmt_ota_comp_cb() - Callback for mgmt frame tx ack
 * @context: tx context
 * @buf: buffer address
 * @status: tx status
 * @tx_compl_params: tx complete parameters
 *
 * This function gets called from mgmt tx/rx component when getting
 * mgmt frame tx ack.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS tgt_p2p_mgmt_ota_comp_cb(void *context, qdf_nbuf_t buf,
	uint32_t status, void *tx_compl_params);

/**
 * tgt_p2p_mgmt_frame_rx_cb() - Callback for rx mgmt frame
 * @psoc: soc context
 * @peer: peer context
 * @buf: rx buffer
 * @params: rx parameters
 * @frm_type: frame type
 *
 * This function gets called from mgmt tx/rx component when rx mgmt
 * frame.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS tgt_p2p_mgmt_frame_rx_cb(
	struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_peer *peer,
	qdf_nbuf_t buf, void *params,
	enum mgmt_frame_type frm_type);

/**
 * tgt_p2p_noa_event_cb() - Callback for noa event
 * @data:
 * @event_buf: event buffer
 * @len: buffer length
 *
 * This function gets called from WMI when triggered WMI event
 * WMI_P2P_NOA_EVENTID.
 *
 * Return: 0      - success
 *         others - failure
 */
int tgt_p2p_noa_event_cb(void *data, uint8_t *event_buf,
	uint32_t len);

/**
 * tgt_p2p_lo_event_cb() - Listen offload stop request
 * @data:
 * @event_buf: event buffer
 * @len: buffer length
 *
 * This function gets called from WMI when triggered WMI event
 * WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID.
 *
 * Return: 0      - success
 *         others - failure
 */
int tgt_p2p_lo_event_cb(void *data, uint8_t *event_buf, uint32_t len);

#endif /* _WLAN_P2P_TGT_API_H_ */
