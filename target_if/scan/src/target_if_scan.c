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
 * DOC: offload lmac interface APIs definitions for scan
 */

#include <qdf_mem.h>
#include <qdf_status.h>
#include <wmi_unified_api.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_param.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_scan_tgt_api.h>
#include <target_if.h>
#include <target_if_scan.h>

#ifdef CONFIG_MCL
inline uint32_t get_scan_event_id(void)
{
	return WMI_SCAN_EVENTID;
}
extern int wma_scan_event_callback(ol_scn_t handle, uint8_t *data,
	uint32_t len);
#else
inline uint32_t get_scan_event_id(void)
{
	return wmi_scan_event_id;
}
#endif

static inline struct wlan_lmac_if_scan_rx_ops *
target_if_scan_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.scan;
}

static int
target_if_scan_event_handler(ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct scan_event_info *event_info;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_scan_rx_ops *scan_rx_ops;
	QDF_STATUS status;

	if (!scn || !data) {
		target_if_err("scn: 0x%p, data: 0x%p\n", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc\n");
		return -EINVAL;
	}
	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	event_info = qdf_mem_malloc(sizeof(*event_info));

	if (!event_info) {
		target_if_err("unable to allocate scan_event");
		return -ENOMEM;
	}

	if (wmi_extract_vdev_scan_ev_param(wmi_handle, data,
	   &(event_info->event))) {
		target_if_err("Failed to extract wmi scan event");
		qdf_mem_free(event_info);
		return -EINVAL;
	}

#ifdef CONFIG_MCL
	/* temp change for p2p ROC*/
#define ROC_SCAN_REQUESTOR_ID   0xB000
	if (ROC_SCAN_REQUESTOR_ID == event_info->event.requester) {
		qdf_mem_free(event_info);
		return wma_scan_event_callback(scn, data, datalen);
	}
#endif

	scan_rx_ops = target_if_scan_get_rx_ops(psoc);
	if (scan_rx_ops->scan_ev_handler) {
		status = scan_rx_ops->scan_ev_handler(psoc, event_info);
		if (status != QDF_STATUS_SUCCESS) {
			qdf_mem_free(event_info);
			return -EINVAL;
		}
	}

	return 0;
}

QDF_STATUS
target_if_scan_register_event_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	uint32_t scan_event_id;

	scan_event_id = get_scan_event_id();
	return wmi_unified_register_event_handler(psoc->tgt_if_handle,
		scan_event_id, target_if_scan_event_handler,
		WMI_RX_UMAC_CTX);
}

QDF_STATUS
target_if_scan_unregister_event_handler(struct wlan_objmgr_psoc *psoc,
		void *arg)
{
	uint32_t scan_event_id;

	scan_event_id = get_scan_event_id();
	return wmi_unified_unregister_event_handler(psoc->tgt_if_handle,
		scan_event_id);
}

QDF_STATUS
target_if_scan_start(struct wlan_objmgr_psoc *psoc,
		struct scan_start_request *req)
{
	return wmi_unified_scan_start_cmd_send(psoc->tgt_if_handle,
		&req->scan_req);
}

QDF_STATUS
target_if_scan_cancel(struct wlan_objmgr_psoc *psoc,
		struct scan_cancel_param *req)
{
	return wmi_unified_scan_stop_cmd_send(psoc->tgt_if_handle, req);
}

QDF_STATUS
target_if_register_scan_tx_ops(struct wlan_lmac_if_scan_tx_ops *scan)
{
	scan->scan_start = target_if_scan_start;
	scan->scan_cancel = target_if_scan_cancel;
	scan->scan_reg_ev_handler = target_if_scan_register_event_handler;
	scan->scan_unreg_ev_handler = target_if_scan_unregister_event_handler;

	return QDF_STATUS_SUCCESS;
}
