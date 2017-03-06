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
 * DOC: This file contains p2p south bound interface definitions
 */

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_mgmt_txrx_utils_api.h>
#include <scheduler_api.h>
#include "wlan_p2p_tgt_api.h"
#include "wlan_p2p_public_struct.h"
#include "../../core/src/wlan_p2p_main.h"

static inline struct wlan_lmac_if_p2p_tx_ops *
wlan_psoc_get_p2p_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &(psoc->soc_cb.tx_ops.p2p);
}

QDF_STATUS tgt_p2p_register_lo_ev_handler(
	struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_ops = wlan_psoc_get_p2p_tx_ops(psoc);
	if (p2p_ops && p2p_ops->reg_lo_ev_handler) {
		status = p2p_ops->reg_lo_ev_handler(psoc, NULL);
		p2p_debug("register lo event, status:%d", status);
	}

	return status;
}

QDF_STATUS tgt_p2p_register_noa_ev_handler(
	struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_ops = wlan_psoc_get_p2p_tx_ops(psoc);
	if (p2p_ops && p2p_ops->reg_noa_ev_handler) {
		status = p2p_ops->reg_noa_ev_handler(psoc, NULL);
		p2p_debug("register noa event, status:%d", status);
	}

	return status;
}

QDF_STATUS tgt_p2p_unregister_lo_ev_handler(
	struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_ops = wlan_psoc_get_p2p_tx_ops(psoc);
	if (p2p_ops && p2p_ops->unreg_lo_ev_handler) {
		status = p2p_ops->unreg_lo_ev_handler(psoc, NULL);
		p2p_debug("unregister lo event, status:%d", status);
	}

	return status;
}

QDF_STATUS tgt_p2p_unregister_noa_ev_handler(
	struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_ops = wlan_psoc_get_p2p_tx_ops(psoc);
	if (p2p_ops && p2p_ops->unreg_noa_ev_handler) {
		status = p2p_ops->unreg_noa_ev_handler(psoc, NULL);
		p2p_debug("unregister noa event, status:%d", status);
	}

	return status;
}

void tgt_p2p_scan_event_cb(struct wlan_objmgr_vdev *vdev,
	struct scan_event *event, void *arg)
{
}

QDF_STATUS tgt_p2p_mgmt_download_comp_cb(void *context,
	qdf_nbuf_t buf, bool free)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_p2p_mgmt_ota_comp_cb(void *context, qdf_nbuf_t buf,
	uint32_t status, void *tx_compl_params)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_p2p_mgmt_frame_rx_cb(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_peer *peer, qdf_nbuf_t buf,
	struct mgmt_rx_event_params *mgmt_rx_params,
	enum mgmt_frame_type frm_type)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS  tgt_p2p_noa_event_cb(struct wlan_objmgr_psoc *psoc,
		struct p2p_noa_info *event_info)
{
	struct p2p_noa_event *noa_event;
	struct scheduler_msg msg;
	struct p2p_soc_priv_obj *p2p_soc_obj;

	p2p_debug("soc:%p, event_info:%p", psoc, event_info);

	if (!psoc) {
		p2p_err("psoc context passed is NULL");
		if (event_info)
			qdf_mem_free(event_info);
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc object is NULL");
		if (event_info)
			qdf_mem_free(event_info);
		return QDF_STATUS_E_INVAL;
	}

	if (!event_info) {
		p2p_err("invalid noa event information");
		return QDF_STATUS_E_INVAL;
	}

	noa_event = qdf_mem_malloc(sizeof(*noa_event));
	if (!noa_event) {
		p2p_err("Failed to allocate p2p noa event");
		qdf_mem_free(event_info);
		return QDF_STATUS_E_NOMEM;
	}

	noa_event->p2p_soc_obj = p2p_soc_obj;
	noa_event->noa_info = event_info;
	msg.type = P2P_EVENT_NOA;
	msg.bodyptr = noa_event;
	msg.callback = p2p_process_evt;
	scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_p2p_lo_event_cb(struct wlan_objmgr_psoc *psoc,
		struct p2p_lo_event *event_info)
{
	struct p2p_lo_stop_event *lo_stop_event;
	struct scheduler_msg msg;
	struct p2p_soc_priv_obj *p2p_soc_obj;

	p2p_debug("soc:%p, event_info:%p", psoc, event_info);

	if (!psoc) {
		p2p_err("psoc context passed is NULL");
		if (event_info)
			qdf_mem_free(event_info);
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc object is NULL");
		if (event_info)
			qdf_mem_free(event_info);
		return QDF_STATUS_E_INVAL;
	}

	if (!event_info) {
		p2p_err("invalid lo stop event information");
		return QDF_STATUS_E_INVAL;
	}

	lo_stop_event = qdf_mem_malloc(sizeof(*lo_stop_event));
	if (!lo_stop_event) {
		p2p_err("Failed to allocate p2p lo stop event");
		qdf_mem_free(event_info);
		return QDF_STATUS_E_NOMEM;
	}

	lo_stop_event->p2p_soc_obj = p2p_soc_obj;
	lo_stop_event->lo_event = event_info;
	msg.type = P2P_EVENT_LO_STOPPED;
	msg.bodyptr = lo_stop_event;
	msg.callback = p2p_process_evt;
	scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);

	return QDF_STATUS_SUCCESS;
}
