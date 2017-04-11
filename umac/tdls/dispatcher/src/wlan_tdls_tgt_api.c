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
 * DOC: wlan_tdls_tgt_api.c
 *
 * TDLS south bound interface definitions
 */

#include "qdf_status.h"
#include <wlan_tdls_tgt_api.h>
#include "../../core/src/wlan_tdls_main.h"
#include "../../core/src/wlan_tdls_cmds_process.h"

static inline struct wlan_lmac_if_tdls_tx_ops *
wlan_psoc_get_tdls_txops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.tx_ops.tdls_tx_ops;
}

static inline struct wlan_lmac_if_tdls_rx_ops *
wlan_psoc_get_tdls_rxops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.tdls_rx_ops;
}

QDF_STATUS tgt_tdls_set_fw_state(struct wlan_objmgr_psoc *psoc,
				 struct tdls_info *tdls_param)
{
	struct wlan_lmac_if_tdls_tx_ops *tdls_ops = NULL;

	tdls_ops = wlan_psoc_get_tdls_txops(psoc);
	if (tdls_ops && tdls_ops->update_fw_state)
		return tdls_ops->update_fw_state(psoc, tdls_param);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_tdls_set_peer_state(struct wlan_objmgr_psoc *psoc,
				   struct tdls_peer_update_state *peer_param)
{
	struct wlan_lmac_if_tdls_tx_ops *tdls_ops = NULL;

	tdls_ops = wlan_psoc_get_tdls_txops(psoc);
	if (tdls_ops && tdls_ops->update_peer_state)
		return tdls_ops->update_peer_state(psoc, peer_param);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_tdls_set_offchan_mode(struct wlan_objmgr_psoc *psoc,
				     struct tdls_channel_switch_params *param)
{
	struct wlan_lmac_if_tdls_tx_ops *tdls_ops = NULL;

	tdls_ops = wlan_psoc_get_tdls_txops(psoc);
	if (tdls_ops && tdls_ops->set_offchan_mode)
		return tdls_ops->set_offchan_mode(psoc, param);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_tdls_set_uapsd(struct wlan_objmgr_psoc *psoc,
			      struct sta_uapsd_trig_params *params)
{
	struct wlan_lmac_if_tdls_tx_ops *tdls_ops = NULL;

	tdls_ops = wlan_psoc_get_tdls_txops(psoc);
	if (tdls_ops && tdls_ops->tdls_set_uapsd)
		return tdls_ops->tdls_set_uapsd(psoc, params);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_tdls_add_peer_rsp(struct scheduler_msg *pmsg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!pmsg || !pmsg->bodyptr) {
		tdls_err("msg: 0x%p", pmsg);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = tdls_process_add_peer_rsp(pmsg->bodyptr);

	return status;
}

QDF_STATUS tgt_tdls_del_peer_rsp(struct scheduler_msg *pmsg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!pmsg || !pmsg->bodyptr) {
		tdls_err("msg: 0x%p", pmsg);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = tdls_process_del_peer_rsp(pmsg->bodyptr);

	return status;
}

QDF_STATUS tgt_tdls_register_ev_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tdls_tx_ops *tdls_ops = NULL;

	tdls_ops = wlan_psoc_get_tdls_txops(psoc);
	if (tdls_ops && tdls_ops->tdls_reg_ev_handler)
		return tdls_ops->tdls_reg_ev_handler(psoc, NULL);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_tdls_unregister_ev_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tdls_tx_ops *tdls_ops = NULL;

	tdls_ops = wlan_psoc_get_tdls_txops(psoc);
	if (tdls_ops->tdls_unreg_ev_handler)
		return tdls_ops->tdls_unreg_ev_handler(psoc, NULL);
	else
		return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_tdls_event_handler(struct wlan_objmgr_psoc *psoc,
		       struct tdls_event_info *info)
{
	struct scheduler_msg msg = {0,};
	struct tdls_event_notify *notify;
	uint8_t vdev_id;
	QDF_STATUS status;

	if (!psoc || !info) {
		tdls_err("psoc: 0x%p, info: 0x%p", psoc, info);
		return QDF_STATUS_E_NULL_VALUE;
	}
	tdls_debug("vdev: %d, type: %d, reason: %d" QDF_MAC_ADDRESS_STR,
		   info->vdev_id, info->message_type, info->peer_reason,
		   QDF_MAC_ADDR_ARRAY(info->peermac.bytes));
	notify = qdf_mem_malloc(sizeof(*notify));
	if (!notify) {
		tdls_err("mem allocate fail");
		return QDF_STATUS_E_NOMEM;
	}

	vdev_id = info->vdev_id;
	notify->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						     vdev_id, WLAN_TDLS_SB_ID);
	if (!notify->vdev) {
		tdls_err("null vdev, vdev_id: %d, psoc: 0x%p", vdev_id, psoc);
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_copy(&notify->event, info, sizeof(*info));

	msg.bodyptr = notify;
	msg.callback = tdls_process_evt;

	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't post msg to handle tdls event");
		wlan_objmgr_vdev_release_ref(notify->vdev, WLAN_TDLS_SB_ID);
		qdf_mem_free(notify);
	}

	return status;
}
