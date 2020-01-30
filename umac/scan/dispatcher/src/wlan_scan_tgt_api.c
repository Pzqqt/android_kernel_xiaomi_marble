/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

#include <wlan_cmn.h>
#include <qdf_list.h>
#include "../../core/src/wlan_scan_main.h"
#include <wlan_scan_utils_api.h>
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
	if (!psoc) {
		scm_err("NULL psoc");
		return NULL;
	}

	return wlan_psoc_get_scan_txops(psoc);
}

static inline struct wlan_lmac_if_scan_rx_ops *
wlan_vdev_get_scan_rxops(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		scm_err("NULL psoc");
		return NULL;
	}

	return &((psoc->soc_cb.rx_ops.scan));
}

#ifdef FEATURE_WLAN_SCAN_PNO

QDF_STATUS tgt_scan_pno_start(struct wlan_objmgr_vdev *vdev,
	struct pno_scan_req_params *req)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);

	if (!psoc) {
		scm_err("NULL PSOC");
		return QDF_STATUS_E_FAILURE;
	}
	scan_ops = wlan_psoc_get_scan_txops(psoc);
	if (!scan_ops) {
		scm_err("NULL scan_ops");
		return QDF_STATUS_E_FAILURE;
	}
	/* invoke wmi_unified_pno_start_cmd() */
	QDF_ASSERT(scan_ops->pno_start);
	if (scan_ops->pno_start)
		return scan_ops->pno_start(psoc, req);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_scan_pno_stop(struct wlan_objmgr_vdev *vdev,
	uint8_t vdev_id)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);

	if (!psoc) {
		scm_err("NULL PSOC");
		return QDF_STATUS_E_FAILURE;
	}
	scan_ops = wlan_psoc_get_scan_txops(psoc);
	if (!scan_ops) {
		scm_err("NULL scan_ops");
		return QDF_STATUS_E_FAILURE;
	}
	/* invoke wmi_unified_pno_stop_cmd() */
	QDF_ASSERT(scan_ops->pno_stop);
	if (scan_ops->pno_stop)
		return scan_ops->pno_stop(psoc, vdev_id);

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
tgt_scan_start(struct scan_start_request *req)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev = req->vdev;

	if (!vdev) {
		scm_err("vdev is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	if (!psoc || !pdev) {
		scm_err("psoc: 0x%pK or pdev: 0x%pK is NULL", psoc, pdev);
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_ops = wlan_psoc_get_scan_txops(psoc);
	/* invoke wmi_unified_scan_start_cmd_send() */
	QDF_ASSERT(scan_ops->scan_start);
	if (scan_ops->scan_start)
		return scan_ops->scan_start(pdev, req);
	else
		return QDF_STATUS_SUCCESS;
}


QDF_STATUS
tgt_scan_cancel(struct scan_cancel_request *req)
{
	struct wlan_lmac_if_scan_tx_ops *scan_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev = req->vdev;

	if (!vdev) {
		scm_err("vdev is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}
	psoc = wlan_vdev_get_psoc(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	if (!psoc || !pdev) {
		scm_err("psoc: 0x%pK or pdev: 0x%pK is NULL", psoc, pdev);
		return QDF_STATUS_E_NULL_VALUE;
	}
	scan_ops = wlan_psoc_get_scan_txops(psoc);
	/* invoke wmi_unified_scan_stop_cmd_send() */
	QDF_ASSERT(scan_ops->scan_cancel);
	if (scan_ops->scan_cancel)
		return scan_ops->scan_cancel(pdev, &req->cancel_req);
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
	struct scheduler_msg msg = {0};
	struct scan_event *event = &event_info->event;
	uint8_t vdev_id = event->vdev_id;
	QDF_STATUS status;

	if (!psoc || !event_info) {
		scm_err("psoc: 0x%pK, event_info: 0x%pK", psoc, event_info);
		return QDF_STATUS_E_NULL_VALUE;
	}

	event_info->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
				vdev_id, WLAN_SCAN_ID);
	if (!event_info->vdev) {
		scm_err("null vdev, vdev_id: %d, psoc: 0x%pK", vdev_id, psoc);
		return QDF_STATUS_E_INVAL;
	}
	msg.bodyptr = event_info;
	msg.callback = scm_scan_event_handler;
	msg.flush_callback = scm_scan_event_flush_callback;

	status = scheduler_post_message(QDF_MODULE_ID_SCAN,
					QDF_MODULE_ID_SCAN,
					QDF_MODULE_ID_SCAN, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_vdev_release_ref(event_info->vdev, WLAN_SCAN_ID);
	}

	return status;
}

QDF_STATUS tgt_scan_bcn_probe_rx_callback(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_peer *peer, qdf_nbuf_t buf,
	struct mgmt_rx_event_params *rx_param,
	enum mgmt_frame_type frm_type)
{
	struct scheduler_msg msg = {0};
	struct scan_bcn_probe_event *bcn = NULL;
	QDF_STATUS status;
	uint32_t scan_queue_size = 0;

	if ((frm_type != MGMT_PROBE_RESP) &&
	    (frm_type != MGMT_BEACON)) {
		scm_err("frame is not beacon or probe resp");
		status = QDF_STATUS_E_INVAL;
		goto free;
	}

	bcn = qdf_mem_malloc_atomic(sizeof(*bcn));
	if (!bcn) {
		status = QDF_STATUS_E_NOMEM;
		goto free;
	}
	bcn->rx_data =
		qdf_mem_malloc_atomic(sizeof(*rx_param));
	if (!bcn->rx_data) {
		status = QDF_STATUS_E_NOMEM;
		goto free;
	}

	if (frm_type == MGMT_PROBE_RESP)
		bcn->frm_type = MGMT_SUBTYPE_PROBE_RESP;
	else
		bcn->frm_type = MGMT_SUBTYPE_BEACON;

	/* Check if the beacon/probe frame can be posted in the scan queue */
	status = scheduler_get_queue_size(QDF_MODULE_ID_SCAN, &scan_queue_size);
	if (!QDF_IS_STATUS_SUCCESS(status) ||
	    scan_queue_size > MAX_BCN_PROBE_IN_SCAN_QUEUE) {
		scm_debug_rl("Dropping beacon/probe frame, queue size %d",
			     scan_queue_size);
		status = QDF_STATUS_E_FAILURE;
		goto free;
	}

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_SCAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_info("unable to get reference");
		goto free;
	}

	bcn->psoc = psoc;
	bcn->buf = buf;
	qdf_mem_copy(bcn->rx_data, rx_param, sizeof(*rx_param));

	msg.bodyptr = bcn;
	msg.callback = scm_handle_bcn_probe;
	msg.flush_callback = scm_bcn_probe_flush_callback;

	status = scheduler_post_message(QDF_MODULE_ID_SCAN,
					QDF_MODULE_ID_SCAN,
					QDF_MODULE_ID_SCAN, &msg);

	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	wlan_objmgr_psoc_release_ref(psoc, WLAN_SCAN_ID);

free:
	if (bcn && bcn->rx_data)
		qdf_mem_free(bcn->rx_data);
	if (bcn)
		qdf_mem_free(bcn);
	if (buf)
		qdf_nbuf_free(buf);

	return status;
}

QDF_STATUS
tgt_scan_set_max_active_scans(struct wlan_objmgr_psoc *psoc,
		uint32_t max_active_scans)
{
	struct scan_default_params *scan_params = NULL;

	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_params = wlan_scan_psoc_get_def_params(psoc);
	if (!scan_params) {
		scm_err("wlan_scan_psoc_get_def_params returned NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_params->max_active_scans_allowed = max_active_scans;

	return QDF_STATUS_SUCCESS;
}
