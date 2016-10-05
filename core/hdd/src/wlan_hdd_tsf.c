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

static struct completion tsf_sync_get_completion_evt;
#define WLAN_TSF_SYNC_GET_TIMEOUT 2000
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
		hdd_err("invalid pointer");
		return -EINVAL;
	}
	if (len != 1)
		return -EINVAL;

	/* Reset TSF value for new capture */
	adapter->tsf_high = 0;
	adapter->tsf_low = 0;
	adapter->tsf_sync_soc_timer = 0;

	if (adapter->device_mode == QDF_STA_MODE ||
		adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (hdd_sta_ctx->conn_info.connState !=
				eConnectionState_Associated) {
			hdd_err("failed to cap tsf, not connect with ap");
			buf[0] = TSF_STA_NOT_CONNECTED_NO_TSF;
			return ret;
		}
	}
	if ((adapter->device_mode == QDF_SAP_MODE ||
		adapter->device_mode == QDF_P2P_GO_MODE) &&
		!(test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags))) {
		hdd_err("Soft AP / P2p GO not beaconing");
		buf[0] = TSF_SAP_NOT_STARTED_NO_TSF;
		return ret;
	}
	if (adapter->tsf_state == TSF_CAP_STATE) {
		hdd_err("current in capture state, pls reset");
		buf[0] = TSF_CURRENT_IN_CAP_STATE;
	} else {
		hdd_info("Send TSF capture to FW");
		buf[0] = TSF_RETURN;
		adapter->tsf_state = TSF_CAP_STATE;
		init_completion(&tsf_sync_get_completion_evt);
		ret = wma_cli_set_command((int)adapter->sessionId,
				(int)GEN_PARAM_CAPTURE_TSF,
				adapter->sessionId,
				GEN_CMD);

		if (ret != QDF_STATUS_SUCCESS) {
			hdd_err("capture fail");
			buf[0] = TSF_CAPTURE_FAIL;
			adapter->tsf_state = TSF_IDLE;
		}
	}
	return ret;
}

/**
 * hdd_tsf_reset_gpio() - Reset TSF GPIO used for host timer sync
 * @adapter: pointer to adapter
 *
 * This function send WMI command to reset GPIO configured in FW after
 * TSF get operation.
 *
 * Return: TSF_RETURN on Success, TSF_RESET_GPIO_FAIL on failure
 */
#ifdef QCA_WIFI_3_0
int hdd_tsf_reset_gpio(struct hdd_adapter_s *adapter)
{
	/* No GPIO Host timer sync for integrated WIFI Device */
	return TSF_RETURN;
}
#else
int hdd_tsf_reset_gpio(struct hdd_adapter_s *adapter)
{
	int ret;
	ret = wma_cli_set_command((int)adapter->sessionId,
			(int)GEN_PARAM_RESET_TSF_GPIO, adapter->sessionId,
			GEN_CMD);

	if (ret != 0) {
		hdd_err("tsf reset GPIO fail ");
		ret = TSF_RESET_GPIO_FAIL;
	} else {
		ret = TSF_RETURN;
	}
	return ret;
}
#endif

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
	hdd_station_ctx_t *hdd_sta_ctx;

	if (adapter == NULL || buf == NULL) {
		hdd_err("invalid pointer");
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
			hdd_info("fail to get tsf, sta in disconnected");
			buf[0] = TSF_STA_NOT_CONNECTED_NO_TSF;
			return 0;
		}
	}
	if ((adapter->device_mode == QDF_SAP_MODE ||
			adapter->device_mode == QDF_P2P_GO_MODE) &&
			!(test_bit(SOFTAP_BSS_STARTED,
			&adapter->event_flags))) {
		hdd_err("Soft AP / P2p GO not beaconing");
		buf[0] = TSF_SAP_NOT_STARTED_NO_TSF;
		return 0;
	}
	if (adapter->tsf_high == 0 && adapter->tsf_low == 0) {
		hdd_info("TSF value not received");
		buf[0] = TSF_NOT_RETURNED_BY_FW;
	} else {
		buf[0] = hdd_tsf_reset_gpio(adapter);
		buf[1] = adapter->tsf_low;
		buf[2] = adapter->tsf_high;
		adapter->tsf_state = TSF_IDLE;
		hdd_info("get tsf cmd,status=%u, tsf_low=%u, tsf_high=%u",
			buf[0], buf[1], buf[2]);
	}
	return 0;
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
int hdd_get_tsf_cb(void *pcb_cxt, struct stsf *ptsf)
{
	struct hdd_context_s *hddctx;
	struct hdd_adapter_s *adapter;
	int status;

	if (pcb_cxt == NULL || ptsf == NULL) {
		hdd_err("HDD context is not valid");
			return -EINVAL;
	}

	hddctx = (struct hdd_context_s *)pcb_cxt;
	status = wlan_hdd_validate_context(hddctx);
	if (0 != status) {
		return -EINVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hddctx, ptsf->vdev_id);

	if (NULL == adapter) {
		hdd_err("failed to find adapter");
		return -EINVAL;
	}
	hdd_info("tsf cb handle event, device_mode is %d",
		adapter->device_mode);

	adapter->tsf_low = ptsf->tsf_low;
	adapter->tsf_high = ptsf->tsf_high;
	adapter->tsf_sync_soc_timer = ((uint64_t) ptsf->soc_timer_high << 32 |
						  ptsf->soc_timer_low);

	complete(&tsf_sync_get_completion_evt);
	hdd_info("Vdev=%u, tsf_low=%u, tsf_high=%u soc_timer=%llu",
		ptsf->vdev_id, ptsf->tsf_low, ptsf->tsf_high,
		adapter->tsf_sync_soc_timer);
	return 0;
}

/**
 * __wlan_hdd_cfg80211_handle_tsf_cmd(): Setup TSF operations
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Handle TSF SET / GET operation from userspace
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_handle_tsf_cmd(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb_vendor[QCA_WLAN_VENDOR_ATTR_TSF_MAX + 1];
	int status, ret;
	struct sk_buff *reply_skb;
	uint32_t tsf_op_resp[3], tsf_cmd;

	ENTER_DEV(wdev->netdev);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return -EINVAL;

	if (nla_parse(tb_vendor, QCA_WLAN_VENDOR_ATTR_TSF_MAX, data,
		      data_len, NULL)) {
		hdd_err("Invalid TSF cmd");
		return -EINVAL;
	}

	if (!tb_vendor[QCA_WLAN_VENDOR_ATTR_TSF_CMD]) {
		hdd_err("Invalid TSF cmd");
		return -EINVAL;
	}
	tsf_cmd = nla_get_u32(tb_vendor[QCA_WLAN_VENDOR_ATTR_TSF_CMD]);

	if (tsf_cmd == QCA_TSF_CAPTURE || tsf_cmd == QCA_TSF_SYNC_GET) {
		hdd_capture_tsf(adapter, tsf_op_resp, 1);
		switch (tsf_op_resp[0]) {
		case TSF_RETURN:
			status = 0;
			break;
		case TSF_CURRENT_IN_CAP_STATE:
			status = -EALREADY;
			break;
		case TSF_STA_NOT_CONNECTED_NO_TSF:
		case TSF_SAP_NOT_STARTED_NO_TSF:
			status = -EPERM;
			break;
		default:
		case TSF_CAPTURE_FAIL:
			status = -EINVAL;
			break;
		}
	}
	if (status < 0)
		goto end;

	if (tsf_cmd == QCA_TSF_SYNC_GET) {
		ret = wait_for_completion_timeout(&tsf_sync_get_completion_evt,
			msecs_to_jiffies(WLAN_TSF_SYNC_GET_TIMEOUT));
		if (ret == 0) {
			status = -ETIMEDOUT;
			goto end;
		}
	}

	if (tsf_cmd == QCA_TSF_GET || tsf_cmd == QCA_TSF_SYNC_GET) {
		hdd_indicate_tsf(adapter, tsf_op_resp, 3);
		switch (tsf_op_resp[0]) {
		case TSF_RETURN:
			status = 0;
			break;
		case TSF_NOT_RETURNED_BY_FW:
			status = -EINPROGRESS;
			break;
		case TSF_STA_NOT_CONNECTED_NO_TSF:
		case TSF_SAP_NOT_STARTED_NO_TSF:
			status = -EPERM;
			break;
		default:
			status = -EINVAL;
			break;
		}
		if (status != 0)
			goto end;

		reply_skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
					sizeof(uint64_t) * 2 + NLMSG_HDRLEN,
					QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX,
					GFP_KERNEL);
		if (!reply_skb) {
			hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
			status = -ENOMEM;
			goto end;
		}
		if (nla_put_u64(reply_skb, QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE,
				((uint64_t) adapter->tsf_high << 32 |
				adapter->tsf_low)) ||
				nla_put_u64(reply_skb,
				QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE,
				adapter->tsf_sync_soc_timer)){
			hdd_err("nla put fail");
			kfree_skb(reply_skb);
			status = -EINVAL;
			goto end;
		}
		status = cfg80211_vendor_cmd_reply(reply_skb);
	}

end:
	hdd_info("TSF operation %d Status: %d", tsf_cmd, status);
	return status;
}

/**
 * wlan_hdd_cfg80211_handle_tsf_cmd(): Setup TSF operations
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Handle TSF SET / GET operation from userspace
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_handle_tsf_cmd(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_handle_tsf_cmd(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
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
