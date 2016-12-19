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
 * DOC: offload lmac interface APIs for scan
 */
#ifndef __TARGET_SCAN_IF_H__
#define __TARGET_SCAN_IF_H__

struct scan_req_params;
struct scan_cancel_param;
struct wlan_objmgr_psoc;

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
 * @psoc: psoc object
 * @req: scan_req_params object
 *
 * Return: QDF_STATUS
 */

QDF_STATUS
target_if_scan_start(struct wlan_objmgr_psoc *psoc,
		struct scan_start_request *req);

/**
 * target_if_scan_cancel() - lmac handler API to cancel a previous active scan
 * @psoc: psoc object
 * @req: scan_cancel_param object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_scan_cancel(struct wlan_objmgr_psoc *psoc,
		struct scan_cancel_param *req);
#endif
