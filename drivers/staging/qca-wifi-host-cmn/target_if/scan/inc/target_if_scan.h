/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: offload lmac interface APIs for scan
 */
#ifndef __TARGET_SCAN_IF_H__
#define __TARGET_SCAN_IF_H__

#include <wmi_unified_api.h>

struct scan_req_params;
struct scan_cancel_param;
struct wlan_objmgr_psoc;

#define WLAN_MAX_ACTIVE_SCANS_ALLOWED   8

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * target_if_nlo_match_event_handler() - nlo match event handler
 * @scn: scn handle
 * @event: event data
 * @len: data length
 *
 * Record NLO match event comes from FW. It's a indication that
 * one of the profile is matched.
 *
 * Return: 0 for success or error code.
 */
int target_if_nlo_match_event_handler(ol_scn_t scn, uint8_t *data,
	uint32_t len);

/**
 * target_if_nlo_complete_handler() - nlo complete event handler
 * @scn: scn handle
 * @event: event data
 * @len: data length
 *
 * Record NLO match event comes from FW. It's a indication that
 * one of the profile is matched.
 *
 * Return: 0 for success or error code.
 */
int target_if_nlo_complete_handler(ol_scn_t scn, uint8_t *data,
	uint32_t len);
#endif

/**
 * target_if_scan_register_event_handler() - lmac handler API
 * to register for scan events
 * @psoc: psoc object
 * @arg: argument to lmac
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_scan_register_event_handler(struct wlan_objmgr_psoc *psoc,
		void *arg);

/**
 * target_if_scan_unregister_event_handler() - lmac handler API
 * to unregister for scan events
 * @psoc: psoc object
 * @arg: argument to lmac
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_scan_unregister_event_handler(struct wlan_objmgr_psoc *psoc,
		void *arg);

/**
 * target_if_scan_start() - lmac handler API to start scan
 * @pdev: pdev object
 * @req: scan_req_params object
 *
 * Return: QDF_STATUS
 */

QDF_STATUS
target_if_scan_start(struct wlan_objmgr_pdev *pdev,
		struct scan_start_request *req);

/**
 * target_if_scan_cancel() - lmac handler API to cancel a previous active scan
 * @pdev: pdev object
 * @req: scan_cancel_param object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_scan_cancel(struct wlan_objmgr_pdev *pdev,
		struct scan_cancel_param *req);

/**
 * target_if_scan_tx_ops_register() - lmac handler to register scan tx_ops
 * callback functions
 * @tx_ops: wlan_lmac_if_tx_ops object
 *
 * Return: QDF_STATUS
 */

QDF_STATUS
target_if_scan_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_scan_set_max_active_scans() - lmac handler to set max active scans
 * @psoc: psoc object
 * @max_active_scans: maximum active scans allowed on underlying psoc
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_scan_set_max_active_scans(struct wlan_objmgr_psoc *psoc,
		uint32_t max_active_scans);

#endif
