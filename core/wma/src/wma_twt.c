/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

void wma_send_twt_enable_cmd(uint32_t pdev_id, uint32_t congestion_timeout)
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
	tpAniSirGlobal mac = (tpAniSirGlobal)cds_get_context(QDF_MODULE_ID_PE);
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

void wma_set_twt_peer_caps(tpAddStaParams params, struct peer_assoc_params *cmd)
{
	if (params->twt_requestor)
		cmd->peer_flags |= WMI_PEER_TWT_REQ;
	if (params->twt_responder)
		cmd->peer_flags |= WMI_PEER_TWT_RESP;
}

