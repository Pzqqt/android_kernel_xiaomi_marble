/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * wlan_hdd_tsf.c - WLAN Host Device Driver tsf related implementation
 */

#include "wlan_hdd_main.h"
#include "wma_api.h"

/**
 * hdd_capture_tsf() - capture tsf
 * @adapter: pointer to adapter
 * @buf: pointer to uplayer buf
 * @len : the length of buf
 *
 * This function returns tsf value to uplayer.
 *
 * Return: 0 for success or non-zero negative failure code
 */
int hdd_capture_tsf(struct hdd_adapter_s *adapter, uint32_t *buf, int len)
{
	int ret = 0;
	hdd_station_ctx_t *hdd_sta_ctx;

	if (adapter == NULL || buf == NULL) {
		hdd_err(FL("invalid pointer"));
		return -EINVAL;
	}
	if (len != 1)
		return -EINVAL;

	/* Reset TSF value for new capture */
	adapter->tsf_high = 0;
	adapter->tsf_low = 0;

	if (adapter->device_mode == QDF_STA_MODE ||
		adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (hdd_sta_ctx->conn_info.connState !=
			eConnectionState_Associated) {

			hdd_err(FL("failed to cap tsf, not connect with ap"));
			buf[0] = TSF_STA_NOT_CONNECTED_NO_TSF;
			return ret;
		}
	}
	if ((adapter->device_mode == QDF_SAP_MODE ||
		adapter->device_mode == QDF_P2P_GO_MODE) &&
		!(test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags))) {
		hdd_err(FL("Soft AP / P2p GO not beaconing"));
		buf[0] = TSF_SAP_NOT_STARTED_NO_TSF;
		return ret;
	}
	if (adapter->tsf_state == TSF_CAP_STATE) {
		hdd_err(FL("current in capture state, pls reset"));
			buf[0] = TSF_CURRENT_IN_CAP_STATE;
	} else {
		hdd_info(FL("Send TSF capture to FW"));
		buf[0] = TSF_RETURN;
		adapter->tsf_state = TSF_CAP_STATE;
		ret = wma_cli_set_command((int)adapter->sessionId,
				(int)GEN_PARAM_CAPTURE_TSF,
				adapter->sessionId,
				GEN_CMD);

		if (ret != QDF_STATUS_SUCCESS) {
			hdd_err(FL("capture fail"));
			buf[0] = TSF_CAPTURE_FAIL;
			adapter->tsf_state = TSF_IDLE;
		}
	}
	return ret;
}

/**
 * hdd_indicate_tsf() - return tsf to uplayer
 * @adapter: pointer to adapter
 * @buf: pointer to uplayer buf
 * @len : the length of buf
 *
 * This function returns tsf value to upper layer.
 *
 * Return: 0 for success or non-zero negative failure code
 */
int hdd_indicate_tsf(struct hdd_adapter_s *adapter, uint32_t *buf, int len)
{
	int ret = 0;
	hdd_station_ctx_t *hdd_sta_ctx;

	if (adapter == NULL || buf == NULL) {
		hdd_err(FL("invalid pointer"));
		return -EINVAL;
	}

	if (len != 3)
		return -EINVAL;

	buf[1] = 0;
	buf[2] = 0;
	if (adapter->device_mode == QDF_STA_MODE ||
		adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (hdd_sta_ctx->conn_info.connState !=
			eConnectionState_Associated) {

			hdd_info(FL("fail to get tsf, sta in disconnected"));
			buf[0] = TSF_STA_NOT_CONNECTED_NO_TSF;
			return ret;
		}
	}
	if ((adapter->device_mode == QDF_SAP_MODE ||
		adapter->device_mode == QDF_P2P_GO_MODE) &&
		!(test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags))) {
		hdd_err(FL("Soft AP / P2p GO not beaconing"));
		buf[0] = TSF_SAP_NOT_STARTED_NO_TSF;
		return ret;
	}
	if (adapter->tsf_high == 0 && adapter->tsf_low == 0) {
		hdd_info(FL("TSF value not received"));
		buf[0] = TSF_NOT_RETURNED_BY_FW;
	} else {
		buf[0] = TSF_RETURN;
		buf[1] = adapter->tsf_low;
		buf[2] = adapter->tsf_high;
		adapter->tsf_state = TSF_IDLE;

		ret = wma_cli_set_command((int)adapter->sessionId,
				(int)GEN_PARAM_RESET_TSF_GPIO,
				adapter->sessionId,
				GEN_CMD);

		if (0 != ret) {
			hdd_err(FL("tsf get fail "));
			buf[0] = TSF_RESET_GPIO_FAIL;
		}
		hdd_info(FL("get tsf cmd,status=%u, tsf_low=%u, tsf_high=%u"),
			buf[0], buf[1], buf[2]);
	}
	return ret;
}

/**
 * hdd_get_tsf_cb() - handle tsf callback
 * @pcb_cxt: pointer to the hdd_contex
 * @ptsf: pointer to struct stsf
 *
 * This function handle the event that reported by firmware at first.
 * The event contains the vdev_id, current tsf value of this vdev,
 * tsf value is 64bits, discripted in two varaible tsf_low and tsf_high.
 * These two values each is uint32.
 *
 * Return: 0 for success or non-zero negative failure code
 */
static int hdd_get_tsf_cb(void *pcb_cxt, struct stsf *ptsf)
{
	struct hdd_context_s *hddctx;
	struct hdd_adapter_s *adapter;
	int status;

	if (pcb_cxt == NULL || ptsf == NULL) {
		hdd_err(FL("HDD context is not valid"));
			return -EINVAL;
	}

	hddctx = (struct hdd_context_s *)pcb_cxt;
	status = wlan_hdd_validate_context(hddctx);
	if (0 != status) {
		hdd_err(FL("hdd context is not valid"));
		return -EINVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hddctx, ptsf->vdev_id);

	if (NULL == adapter) {
		hdd_err(FL("failed to find adapter"));
		return -EINVAL;
	}

	hdd_info(FL("tsf cb handle event, device_mode is %d"),
		adapter->device_mode);

	adapter->tsf_low = ptsf->tsf_low;
	adapter->tsf_high = ptsf->tsf_high;

	hdd_info(FL("hdd_get_tsf_cb sta=%u, tsf_low=%u, tsf_high=%u"),
		ptsf->vdev_id, ptsf->tsf_low, ptsf->tsf_high);
	return 0;
}

/**
 * wlan_hdd_tsf_init() - set callback to handle tsf value.
 * @hdd_ctx: pointer to the struct hdd_context_s
 *
 * This function set the callback to sme module, the callback will be
 * called when a tsf event is reported by firmware
 *
 * Return: none
 */
void wlan_hdd_tsf_init(struct hdd_context_s *hdd_ctx)
{
	sme_set_tsfcb(hdd_ctx->hHal, hdd_get_tsf_cb, hdd_ctx);
}
