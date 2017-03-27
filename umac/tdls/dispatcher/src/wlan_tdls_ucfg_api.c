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
 * DOC: wlan_tdls_ucfg_api.c
 *
 * TDLS north bound interface definitions
 */

#include <wlan_tdls_ucfg_api.h>
#include <wlan_tdls_tgt_api.h>
#include "../../core/src/wlan_tdls_main.h"
#include "../../core/src/wlan_tdls_cmds_process.h"
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_cmn.h>

QDF_STATUS ucfg_tdls_init(void)
{
	QDF_STATUS status;

	tdls_notice("tdls module dispatcher init");
	status = wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_TDLS,
		tdls_psoc_obj_create_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to register psoc create handler for tdls");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(WLAN_UMAC_COMP_TDLS,
		tdls_psoc_obj_destroy_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to register psoc delete handler for tdls");
		goto fail_delete_psoc;
	}

	status = wlan_objmgr_register_vdev_create_handler(WLAN_UMAC_COMP_TDLS,
		tdls_vdev_obj_create_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to register vdev create handler for tdls");
		goto fail_create_vdev;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(WLAN_UMAC_COMP_TDLS,
		tdls_vdev_obj_destroy_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to register vdev create handler for tdls");
		goto fail_delete_vdev;
	}
	tdls_notice("tdls module dispatcher init done");

	return status;
fail_delete_vdev:
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_TDLS,
		tdls_vdev_obj_create_notification, NULL);

fail_create_vdev:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_TDLS,
		tdls_psoc_obj_destroy_notification, NULL);

fail_delete_psoc:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_TDLS,
		tdls_psoc_obj_create_notification, NULL);

	return status;
}

QDF_STATUS ucfg_tdls_deinit(void)
{
	QDF_STATUS ret;

	tdls_notice("tdls module dispatcher deinit");
	ret = wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_TDLS,
				tdls_psoc_obj_create_notification, NULL);
	if (QDF_IS_STATUS_ERROR(ret))
		tdls_err("Failed to unregister psoc create handler");

	ret = wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_TDLS,
				tdls_psoc_obj_destroy_notification, NULL);
	if (QDF_IS_STATUS_ERROR(ret))
		tdls_err("Failed to unregister psoc delete handler");

	ret = wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_TDLS,
				tdls_vdev_obj_create_notification, NULL);
	if (QDF_IS_STATUS_ERROR(ret))
		tdls_err("Failed to unregister vdev create handler");

	ret = wlan_objmgr_unregister_vdev_destroy_handler(WLAN_UMAC_COMP_TDLS,
				tdls_vdev_obj_destroy_notification, NULL);

	if (QDF_IS_STATUS_ERROR(ret))
		tdls_err("Failed to unregister vdev delete handler");

	return ret;
}

static QDF_STATUS tdls_global_init(struct tdls_soc_priv_obj *soc_obj)
{
	uint8_t sta_idx;
	uint32_t feature;

	soc_obj->connected_peer_count = 0;
	soc_obj->tdls_nss_switch_in_progress = false;
	soc_obj->tdls_teardown_peers_cnt = 0;
	soc_obj->tdls_nss_teardown_complete = false;
	soc_obj->tdls_nss_transition_mode = TDLS_NSS_TRANSITION_S_UNKNOWN;
	soc_obj->tdls_user_config_mode = TDLS_SUPPORT_DISABLED;

	feature = soc_obj->tdls_configs.tdls_feature_flags;
	if (TDLS_IS_BUFFER_STA_ENABLED(feature) ||
	    TDLS_IS_SLEEP_STA_ENABLED(feature) ||
	    TDLS_IS_OFF_CHANNEL_ENABLED(feature))
		soc_obj->max_num_tdls_sta =
			WLAN_TDLS_STA_P_UAPSD_OFFCHAN_MAX_NUM;
		else
			soc_obj->max_num_tdls_sta = WLAN_TDLS_STA_MAX_NUM;


	for (sta_idx = 0; sta_idx < soc_obj->max_num_tdls_sta; sta_idx++) {
		soc_obj->tdls_conn_info[sta_idx].sta_id = 0;
		soc_obj->tdls_conn_info[sta_idx].session_id = 255;
		qdf_mem_zero(&soc_obj->tdls_conn_info[sta_idx].peer_mac,
			     QDF_MAC_ADDR_SIZE);
	}
	soc_obj->enable_tdls_connection_tracker = false;
	soc_obj->tdls_external_peer_count = 0;
	soc_obj->tdls_disable_in_progress = false;

	qdf_spinlock_create(&soc_obj->tdls_ct_spinlock);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_global_deinit(struct tdls_soc_priv_obj *soc_obj)
{
	qdf_spinlock_destroy(&soc_obj->tdls_ct_spinlock);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_tdls_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *soc_obj;

	tdls_notice("tdls psoc open");
	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (!soc_obj) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	status = tdls_global_init(soc_obj);

	return status;
}

QDF_STATUS ucfg_tdls_update_config(struct wlan_objmgr_psoc *psoc,
				   struct tdls_start_params *req)
{
	struct tdls_soc_priv_obj *soc_obj;
	uint32_t tdls_feature_flags;

	tdls_notice("tdls update config ");
	if (!psoc || !req) {
		tdls_err("psoc: 0x%p, req: 0x%p", psoc, req);
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (!soc_obj) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj->tdls_event_cb = req->tdls_event_cb;
	soc_obj->tdls_evt_cb_data = req->tdls_evt_cb_data;

	soc_obj->tdls_tx_cnf_cb = req->ack_cnf_cb;
	soc_obj->tx_ack_cnf_cb_data = req->tx_ack_cnf_cb_data;

	/* Save callbacks to register/deregister TDLS sta with TL */
	soc_obj->tdls_reg_tl_peer = req->tdls_reg_tl_peer;
	soc_obj->tdls_dereg_tl_peer = req->tdls_dereg_tl_peer;
	soc_obj->tdls_tl_peer_data = req->tdls_tl_peer_data;

	/* Save legacy PE/WMA commands in TDLS soc object */
	soc_obj->tdls_send_mgmt_req = req->tdls_send_mgmt_req;
	soc_obj->tdls_add_sta_req = req->tdls_add_sta_req;
	soc_obj->tdls_del_sta_req = req->tdls_del_sta_req;
	soc_obj->tdls_update_peer_state = req->tdls_update_peer_state;

	/* Update TDLS user config */
	qdf_mem_copy(&soc_obj->tdls_configs, &req->config, sizeof(req->config));
	tdls_feature_flags = soc_obj->tdls_configs.tdls_feature_flags;

	if (!TDLS_IS_IMPLICIT_TRIG_ENABLED(tdls_feature_flags))
		soc_obj->tdls_current_mode = TDLS_SUPPORT_EXP_TRIG_ONLY;
	else if (TDLS_IS_EXTERNAL_CONTROL_ENABLED(tdls_feature_flags))
		soc_obj->tdls_current_mode = TDLS_SUPPORT_EXT_CONTROL;
	else
		soc_obj->tdls_current_mode = TDLS_SUPPORT_IMP_MODE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_tdls_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	tdls_notice("psoc tdls enable: 0x%p", psoc);
	if (!psoc) {
		tdls_err("NULL psoc");
		return QDF_STATUS_E_FAILURE;
	}

	status = tgt_tdls_register_ev_handler(psoc);

	return status;
}

QDF_STATUS ucfg_tdls_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *soc_obj = NULL;

	tdls_notice("psoc tdls disable: 0x%p", psoc);
	if (!psoc) {
		tdls_err("NULL psoc");
		return QDF_STATUS_E_FAILURE;
	}
	status = tgt_tdls_unregister_ev_handler(psoc);
	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (!soc_obj) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj->tdls_event_cb = NULL;
	soc_obj->tdls_evt_cb_data = NULL;

	soc_obj->tdls_tx_cnf_cb = NULL;
	soc_obj->tx_ack_cnf_cb_data = NULL;

	return status;
}

QDF_STATUS ucfg_tdls_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct tdls_soc_priv_obj *tdls_soc;

	tdls_notice("tdls psoc close");
	tdls_soc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (!tdls_soc) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	status = tdls_global_deinit(tdls_soc);

	return status;
}

QDF_STATUS ucfg_tdls_add_peer(struct wlan_objmgr_vdev *vdev,
			      struct tdls_add_peer_params *add_peer_req)
{
	struct scheduler_msg msg = {0, };
	struct tdls_add_peer_request *req;
	QDF_STATUS status;

	if (!vdev || !add_peer_req) {
		tdls_err("vdev: %p, req %p", vdev, add_peer_req);
		return QDF_STATUS_E_NULL_VALUE;
	}
	tdls_debug("vdevid: %d, peertype: %d",
		   add_peer_req->vdev_id, add_peer_req->peer_type);

	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get vdev");
		return status;
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		tdls_err("mem allocate fail");
		status = QDF_STATUS_E_NOMEM;
		goto dec_ref;
	}

	qdf_mem_copy(&req->add_peer_req, add_peer_req, sizeof(*add_peer_req));
	req->vdev = vdev;

	msg.bodyptr = req;
	msg.callback = tdls_process_cmd;
	msg.type = TDLS_CMD_ADD_STA;
	status = scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("post add peer msg fail");
		qdf_mem_free(req);
		goto dec_ref;
	}

	return status;
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
	return status;
}

QDF_STATUS ucfg_tdls_update_peer(struct wlan_objmgr_vdev *vdev,
				 struct tdls_update_peer_params *update_peer)
{
	struct scheduler_msg msg = {0,};
	struct tdls_update_peer_request *req;
	QDF_STATUS status;

	if (!vdev || !update_peer) {
		tdls_err("vdev: %p, update_peer: %p", vdev, update_peer);
		return QDF_STATUS_E_NULL_VALUE;
	}

	tdls_debug("vdev_id: %d, peertype: %d",
		   update_peer->vdev_id, update_peer->peer_type);
	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get vdev");
		return status;
	}
	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		tdls_err("mem allocate fail");
		status = QDF_STATUS_E_NOMEM;
		goto dec_ref;
	}
	qdf_mem_copy(&req->update_peer_req, update_peer, sizeof(*update_peer));
	req->vdev = vdev;

	msg.bodyptr = req;
	msg.callback = tdls_process_cmd;
	msg.type = TDLS_CMD_CHANGE_STA;
	status = scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("post update peer msg fail");
		qdf_mem_free(req);
		goto dec_ref;
	}

	return status;
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
	return status;
}

static char *tdls_get_oper_str(enum tdls_command_type cmd_type)
{
	switch (cmd_type) {
	case TDLS_CMD_ENABLE_LINK:
		return "Enable_TDLS_LINK";
	case TDLS_CMD_DISABLE_LINK:
		return "DISABLE_TDLS_LINK";
	case TDLS_CMD_REMOVE_FORCE_PEER:
		return "REMOVE_FORCE_PEER";
	case TDLS_CMD_CONFIG_FORCE_PEER:
		return "CONFIG_FORCE_PEER";
	default:
		return "ERR:UNKNOWN OPER";
	}
}

QDF_STATUS ucfg_tdls_oper(struct wlan_objmgr_vdev *vdev,
			  const uint8_t *macaddr, enum tdls_command_type cmd)
{
	struct scheduler_msg msg = {0,};
	struct tdls_oper_request *req;
	QDF_STATUS status;

	if (!vdev || !macaddr) {
		tdls_err("vdev: %p, mac %p", vdev, macaddr);
		return QDF_STATUS_E_NULL_VALUE;
	}

	tdls_debug("%s for peer " QDF_MAC_ADDRESS_STR,
		   tdls_get_oper_str(cmd),
		   QDF_MAC_ADDR_ARRAY(macaddr));

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		tdls_err("%s: mem allocate fail", tdls_get_oper_str(cmd));
		return QDF_STATUS_E_NOMEM;
	}

	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get vdev");
		goto error;
	}

	qdf_mem_copy(req->peer_addr, macaddr, QDF_MAC_ADDR_SIZE);
	req->vdev = vdev;

	msg.bodyptr = req;
	msg.callback = tdls_process_cmd;
	msg.type = cmd;
	status = scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("post msg for %s fail", tdls_get_oper_str(cmd));
		goto dec_ref;
	}

	return status;
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
error:
	qdf_mem_free(req);
	return status;
}

QDF_STATUS ucfg_tdls_send_mgmt_frame(
				struct tdls_action_frame_request *req)
{
	struct scheduler_msg msg = {0, };
	struct tdls_action_frame_request *mgmt_req;

	if (!req || !req->vdev) {
		tdls_err("Invalid mgmt req params %p", req);
		return QDF_STATUS_E_NULL_VALUE;
	}

	mgmt_req = qdf_mem_malloc(sizeof(*mgmt_req) +
					req->len);
	if (!mgmt_req) {
		tdls_err("mem allocate fail");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(mgmt_req, req, sizeof(*req));

	/*populate the additional IE's */
	if ((0 != req->len) && (NULL != req->cmd_buf)) {
		qdf_mem_copy(mgmt_req->tdls_mgmt.buf, req->cmd_buf,
				req->len);
		mgmt_req->tdls_mgmt.len = req->len;
	} else {
		mgmt_req->tdls_mgmt.len = 0;
	}

	tdls_notice("vdev id: %d, session id : %d", mgmt_req->vdev_id,
		    mgmt_req->session_id);
	msg.bodyptr = mgmt_req;
	msg.callback = tdls_process_cmd;
	msg.type = TDLS_CMD_TX_ACTION;
	scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);

	return QDF_STATUS_SUCCESS;
}
