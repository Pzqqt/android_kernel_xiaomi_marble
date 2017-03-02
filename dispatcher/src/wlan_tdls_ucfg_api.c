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
#include "../../core/src/wlan_tdls_main.h"
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

	soc_obj->connected_peer_count = 0;
	soc_obj->tdls_nss_switch_in_progress = false;
	soc_obj->tdls_teardown_peers_cnt = 0;
	soc_obj->tdls_nss_teardown_complete = false;
	soc_obj->tdls_nss_transition_mode = TDLS_NSS_TRANSITION_S_UNKNOWN;
	soc_obj->tdls_user_config_mode = TDLS_SUPPORT_DISABLED;
	soc_obj->max_num_tdls_sta = WLAN_TDLS_STA_MAX_NUM;

	for (sta_idx = 0; sta_idx < soc_obj->max_num_tdls_sta; sta_idx++) {
		soc_obj->tdls_conn_info[sta_idx].sta_id = 0;
		soc_obj->tdls_conn_info[sta_idx].session_id = 255;
		qdf_mem_zero(&soc_obj->tdls_conn_info[sta_idx].peer_mac,
			     QDF_MAC_ADDR_SIZE);
	}
	soc_obj->enable_tdls_connection_tracker = false;
	soc_obj->tdls_external_peer_count = 0;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_tdls_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *soc_obj;

	tdls_notice("tdls psoc open");
	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (soc_obj == NULL) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	status = tdls_global_init(soc_obj);

	return status;
}

QDF_STATUS ucfg_tdls_psoc_start(struct wlan_objmgr_psoc *psoc,
				struct tdls_start_params *req)
{
	struct tdls_soc_priv_obj *soc_obj;

	tdls_notice("tdls psoc start");
	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (soc_obj == NULL) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj->tdls_event_cb = req->tdls_event_cb;
	soc_obj->tdls_evt_cb_data = req->tdls_evt_cb_data;

	soc_obj->tdls_tx_cnf_cb = req->ack_cnf_cb;
	soc_obj->tx_ack_cnf_cb_data = req->tx_ack_cnf_cb_data;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_tdls_psoc_stop(struct wlan_objmgr_psoc *psoc)
{
	struct tdls_soc_priv_obj *soc_obj = NULL;

	tdls_notice("tdls psoc stop");
	soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TDLS);
	if (soc_obj == NULL) {
		tdls_err("Failed to get tdls psoc component");
		return QDF_STATUS_E_FAILURE;
	}

	soc_obj->tdls_event_cb = NULL;
	soc_obj->tdls_evt_cb_data = NULL;

	soc_obj->tdls_tx_cnf_cb = NULL;
	soc_obj->tx_ack_cnf_cb_data = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_tdls_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	tdls_notice("tdls psoc close");

	return status;
}
