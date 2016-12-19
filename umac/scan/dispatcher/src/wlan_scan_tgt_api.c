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

#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_tgt_api.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_lmac_if_def.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <../../core/src/wlan_scan_manager.h>

static inline struct wlan_lmac_if_scan_tx_ops *
wlan_psoc_get_scan_txops(struct wlan_objmgr_psoc *psoc)
{
	return &((psoc->soc_cb.tx_ops.scan));
}

static inline struct wlan_lmac_if_scan_tx_ops *
wlan_vdev_get_scan_txops(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_vdev_get_psoc(vdev);

	return wlan_psoc_get_scan_txops(psoc);
}

static inline struct wlan_lmac_if_scan_rx_ops *
wlan_vdev_get_scan_rxops(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_vdev_get_psoc(vdev);

	return &((psoc->soc_cb.rx_ops.scan));
}

QDF_STATUS
tgt_scan_nlo_complete_evt_handler(void *handle, uint8_t *event,
	uint32_t len)
{
	/*
	 * Convert the tlv/non tlv data to struct scan_event
	 * (SCM_EVENT_NLO_COMPLETE) (same as WIN does by calling a win API) and
	 * Post msg to target_if queue
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_nlo_match_evt_handler(void *handle, uint8_t *event,
		uint32_t len)
{
	/*
	 * Convert the tlv/non tlv data to comman data
	 * and set the pno match received flag in vdev scan info
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_scan_start(struct scan_start_request *req)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(req->vdev);

	scan_ops = wlan_vdev_get_scan_txops(req->vdev);
	/* invoke wmi_unified_scan_start_cmd_send() */
	QDF_ASSERT(scan_ops->scan_start);
	if (scan_ops->scan_start)
		return scan_ops->scan_start(psoc, req);
	else
		return QDF_STATUS_SUCCESS;
}


QDF_STATUS
tgt_scan_cancel(struct scan_cancel_request *req)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(req->vdev);

	scan_ops = wlan_vdev_get_scan_txops(req->vdev);
	/* invoke wmi_unified_scan_stop_cmd_send() */
	QDF_ASSERT(scan_ops->scan_cancel);
	if (scan_ops->scan_cancel)
		return scan_ops->scan_cancel(psoc, &(req->cancel_req));
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_scan_register_ev_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops = NULL;

	scan_ops = wlan_psoc_get_scan_txops(psoc);
	/* invoke wmi_unified_register_event_handler()
	 * since event id, handler function and context is
	 * already known to offload lmac, passing NULL as argument.
	 * DA can pass necessary arguments by clubing then into
	 * some structure.
	 */
	QDF_ASSERT(scan_ops->scan_reg_ev_handler);
	if (scan_ops->scan_reg_ev_handler)
		return scan_ops->scan_reg_ev_handler(psoc, NULL);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_scan_unregister_ev_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops = NULL;

	scan_ops = wlan_psoc_get_scan_txops(psoc);
	/* invoke wmi_unified_register_event_handler()
	 * since event id, handler function and context is
	 * already known to offload lmac, passing NULL as argument.
	 * DA can pass necessary arguments by clubing then into
	 * some structure.
	 */
	QDF_ASSERT(scan_ops->scan_unreg_ev_handler);
	if (scan_ops->scan_unreg_ev_handler)
		return scan_ops->scan_unreg_ev_handler(psoc, NULL);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_scan_event_handler(struct wlan_objmgr_psoc *psoc,
		struct scan_event_info *event_info)
{
	struct scheduler_msg msg = {0,};
	struct scan_event *event = &event_info->event;
	uint8_t vdev_id = event->vdev_id;

	if (!psoc || !event_info) {
		scm_err("psoc: 0x%p, event_info: 0x%p", psoc, event_info);
		return QDF_STATUS_E_NULL_VALUE;
	}
	scm_info("vdev: %d, type: %d, reason: %d, freq: %d, req: %d, scanid: %d",
		vdev_id, event->type, event->reason, event->chan_freq,
		event->requester, event->scan_id);

	event_info->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
				vdev_id, WLAN_SCAN_ID);
	if (!event_info->vdev) {
		scm_err("null vdev, vdev_id: %d, psoc: 0x%p", vdev_id, psoc);
		return QDF_STATUS_E_INVAL;
	}
	msg.bodyptr = event_info;
	msg.callback = scm_scan_event_handler;

	return scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
}
