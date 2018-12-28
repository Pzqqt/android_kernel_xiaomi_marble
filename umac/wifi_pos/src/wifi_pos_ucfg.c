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
 * This file defines the important dispatcher APIs pertinent to
 * wifi positioning.
 */
#include "wifi_pos_utils_i.h"
#include "wifi_pos_api.h"
#include "wifi_pos_ucfg_i.h"
#include "wlan_ptt_sock_svc.h"

QDF_STATUS ucfg_wifi_pos_process_req(struct wlan_objmgr_psoc *psoc,
		struct wifi_pos_req_msg *req,
		void (*send_rsp_cb)(uint32_t, uint32_t, uint32_t, uint8_t *))
{
	uint8_t err;
	uint32_t app_pid;
	bool is_app_registered;
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

	wifi_pos_debug("enter");

	if (!wifi_pos_psoc_obj) {
		wifi_pos_err("wifi_pos_psoc_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc_obj->wifi_pos_lock);
	wifi_pos_psoc_obj->wifi_pos_send_rsp = send_rsp_cb;
	is_app_registered = wifi_pos_psoc_obj->is_app_registered;
	app_pid = wifi_pos_psoc_obj->app_pid;
	qdf_spin_unlock_bh(&wifi_pos_psoc_obj->wifi_pos_lock);

	if (!wifi_pos_psoc_obj->wifi_pos_req_handler) {
		wifi_pos_err("wifi_pos_psoc_obj->wifi_pos_req_handler is null");
		err = OEM_ERR_NULL_CONTEXT;
		send_rsp_cb(app_pid, ANI_MSG_OEM_ERROR, sizeof(err), &err);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (req->msg_type != ANI_MSG_APP_REG_REQ &&
		(!is_app_registered || app_pid != req->pid)) {
		wifi_pos_err("requesting app is not registered, app_registered: %d, requesting pid: %d, stored pid: %d",
			is_app_registered, req->pid, app_pid);
		err = OEM_ERR_APP_NOT_REGISTERED;
		send_rsp_cb(app_pid, ANI_MSG_OEM_ERROR, sizeof(err), &err);
		return QDF_STATUS_E_INVAL;
	}

	return wifi_pos_psoc_obj->wifi_pos_req_handler(psoc, req);
}


uint32_t ucfg_wifi_pos_get_ftm_cap(struct wlan_objmgr_psoc *psoc)
{
	uint32_t val = 0;
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_alert("unable to get wifi_pos psoc obj");
		return val;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	val = wifi_pos_psoc->fine_time_meas_cap;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);

	return val;
}

void ucfg_wifi_pos_set_ftm_cap(struct wlan_objmgr_psoc *psoc, uint32_t val)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc) {
		wifi_pos_alert("unable to get wifi_pos psoc obj");
		return;
	}

	qdf_spin_lock_bh(&wifi_pos_psoc->wifi_pos_lock);
	wifi_pos_psoc->fine_time_meas_cap = val;
	qdf_spin_unlock_bh(&wifi_pos_psoc->wifi_pos_lock);
}

