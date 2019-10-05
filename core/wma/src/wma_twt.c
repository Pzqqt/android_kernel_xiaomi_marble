/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: wma_twt.c
 *
 * WLAN Host Device Driver TWT - Target Wake Time Implementation
 */

#include "wma_twt.h"
#include "wmi_unified_twt_api.h"
#include "wma_internal.h"
#include "wmi_unified_priv.h"

void wma_send_twt_enable_cmd(uint32_t pdev_id,
			     uint32_t congestion_timeout, bool bcast_val)
{
	t_wma_handle *wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wmi_twt_enable_param twt_enable_params = {0};
	int32_t ret;

	if (!wma) {
		WMA_LOGE("Invalid WMA context, enable TWT failed");
		return;
	}
	twt_enable_params.pdev_id = pdev_id;
	twt_enable_params.sta_cong_timer_ms = congestion_timeout;
	TWT_EN_DIS_FLAGS_SET_BTWT(twt_enable_params.flags, bcast_val);
	ret = wmi_unified_twt_enable_cmd(wma->wmi_handle, &twt_enable_params);

	if (ret)
		WMA_LOGE("Failed to enable TWT");
}

int wma_twt_en_complete_event_handler(void *handle,
				      uint8_t *event, uint32_t len)
{
	struct wmi_twt_enable_complete_event_param param;
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_unified_t wmi_handle;
	struct mac_context *mac = (struct mac_context *)cds_get_context(QDF_MODULE_ID_PE);
	int status = -EINVAL;

	if (!wma_handle) {
		WMA_LOGE("Invalid wma handle for TWT complete");
		return status;
	}
	wmi_handle = (wmi_unified_t)wma_handle->wmi_handle;
	if (!wmi_handle) {
		WMA_LOGE("Invalid wmi handle for TWT complete");
		return status;
	}
	if (!mac) {
		WMA_LOGE("Invalid MAC context");
		return status;
	}
	if (wmi_handle->ops->extract_twt_enable_comp_event)
		status = wmi_handle->ops->extract_twt_enable_comp_event(
								wmi_handle,
								event,
								&param);
	WMA_LOGD("TWT: Received TWT enable comp event, status:%d", status);

	if (mac->sme.twt_enable_cb)
		mac->sme.twt_enable_cb(mac->hdd_handle, &param);

	return status;
}

void wma_send_twt_disable_cmd(uint32_t pdev_id)
{
	t_wma_handle *wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wmi_twt_disable_param twt_disable_params = {0};
	int32_t ret;

	if (!wma) {
		WMA_LOGE("Invalid WMA context, Disable TWT failed");
		return;
	}
	twt_disable_params.pdev_id = pdev_id;
	ret = wmi_unified_twt_disable_cmd(wma->wmi_handle, &twt_disable_params);

	if (ret)
		WMA_LOGE("Failed to disable TWT");
}

int wma_twt_disable_comp_event_handler(void *handle, uint8_t *event,
				       uint32_t len)
{
	struct mac_context *mac;

	mac = (struct mac_context *)cds_get_context(QDF_MODULE_ID_PE);
	if (!mac) {
		WMA_LOGE("Invalid MAC context");
		return -EINVAL;
	}

	WMA_LOGD("TWT: Rcvd TWT disable comp event");

	if (mac->sme.twt_disable_cb)
		mac->sme.twt_disable_cb(mac->hdd_handle);

	return 0;
}

void wma_set_twt_peer_caps(tpAddStaParams params, struct peer_assoc_params *cmd)
{
	if (params->twt_requestor)
		cmd->twt_requester = 1;
	if (params->twt_responder)
		cmd->twt_responder = 1;
}

QDF_STATUS wma_twt_process_add_dialog(
		struct wmi_twt_add_dialog_param *params)
{
	t_wma_handle *wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	wmi_unified_t wmi_handle;

	if (!wma_handle) {
		WMA_LOGE("Invalid WMA context, twt add dialog failed");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = (wmi_unified_t)wma_handle->wmi_handle;
	if (!wmi_handle) {
		WMA_LOGE("Invalid wmi handle, twt add dialog failed");
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_twt_add_dialog_cmd(wmi_handle, params);
}

QDF_STATUS wma_twt_process_del_dialog(
		struct wmi_twt_del_dialog_param *params)
{
	t_wma_handle *wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	wmi_unified_t wmi_handle;

	if (!wma_handle) {
		WMA_LOGE("Invalid WMA context, twt del dialog failed");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = (wmi_unified_t)wma_handle->wmi_handle;
	if (!wmi_handle) {
		WMA_LOGE("Invalid wmi handle, twt del dialog failed");
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_twt_del_dialog_cmd(wmi_handle, params);
}

