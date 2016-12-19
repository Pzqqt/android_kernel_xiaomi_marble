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

/*
 * DOC: contains scan south bound interface definitions
 */

#ifndef _WLAN_SCAN_TGT_API_H_
#define _WLAN_SCAN_TGT_API_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_scan_public_structs.h>
#include <wlan_mgmt_txrx_utils_api.h>

/**
 * tgt_scan_bcn_probe_rx_callback() - The callbeack registered to tx/rx module
 * @psoc: psoc context
 * @peer: peer
 * @buf: frame buf
 * @params: rx event params
 * @frm_type: frame type
 *
 * The callbeack registered to tx/rx module and is called when beacon
 * or probe resp is recived. This will post a msg to target_if queue.
 *
 * Return: success or error code.
 */
QDF_STATUS tgt_scan_bcn_probe_rx_callback(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_peer *peer, qdf_nbuf_t buf,
	struct mgmt_rx_event_params *rx_param, uint32_t frm_type);

/**
 * tgt_scan_nlo_complete_evt_handler() - The callbeack registered
 * to WMI for PNO complete
 * @handle: psoc handle
 * @event: event handler
 * @len: length of data
 *
 * This function handles NLO scan completion event.
 *
 * Return: 0 for success or error code.
 */

QDF_STATUS
tgt_scan_nlo_complete_evt_handler(void *handle, uint8_t *event,
	uint32_t len);

/**
 * tgt_nlo_match_evt_handler() - nlo match event handler
 * @handle: psoc handle
 * @event: event data
 * @len: data length
 *
 * Record NLO match event comes from FW. It's a indication that
 * one of the profile is matched.
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
tgt_nlo_match_evt_handler(void *handle, uint8_t *event,
	uint32_t len);

/**
 * tgt_scan_event_handler() - The callbeack registered to WMI for scan events
 * @psoc: psoc handle
 * @event_info: event info
 *
 * The callbeack registered to WMI for scan events and is called
 * event for scan is received. This will post a msg to target_if queue.
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
tgt_scan_event_handler(struct wlan_objmgr_psoc *psoc,
	struct scan_event_info *event_info);

/**
 * tgt_scan_start() - invoke lmac scan start
 * @req: scan request object
 *
 * This API invokes lmac API function to start scan
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS
tgt_scan_start(struct scan_start_request *req);


/**
 * tgt_scan_cancel() - invoke lmac scan cancel
 * @req: scan request object
 *
 * This API invokes lmac API function to cancel scan
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS
tgt_scan_cancel(struct scan_cancel_request *req);

/**
 * tgt_scan_register_ev_handler() - invoke lmac register scan event handler
 * @psoc: psoc object
 *
 * This API invokes lmac API function to register for scan events
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS
tgt_scan_register_ev_handler(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_scan_unregister_ev_handler() - invoke lmac unregister scan event handler
 * @psoc: psoc object
 *
 * This API invokes lmac API function to unregister for scan events
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
QDF_STATUS
tgt_scan_unregister_ev_handler(struct wlan_objmgr_psoc *psoc);
#endif
