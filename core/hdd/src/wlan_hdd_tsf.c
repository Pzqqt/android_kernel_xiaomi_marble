/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include "wlan_hdd_tsf.h"
#include "wma_api.h"
#include <qca_vendor.h>

static struct completion tsf_sync_get_completion_evt;
#define WLAN_TSF_SYNC_GET_TIMEOUT 2000

/**
 * enum hdd_tsf_op_result - result of tsf operation
 *
 * HDD_TSF_OP_SUCC:  succeed
 * HDD_TSF_OP_FAIL:  fail
 */
enum hdd_tsf_op_result {
	HDD_TSF_OP_SUCC,
	HDD_TSF_OP_FAIL
};

static
enum hdd_tsf_get_state hdd_tsf_check_conn_state(hdd_adapter_t *adapter)
{
	enum hdd_tsf_get_state ret = TSF_RETURN;
	hdd_station_ctx_t *hdd_sta_ctx;

	if (adapter->device_mode == QDF_STA_MODE ||
			adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (hdd_sta_ctx->conn_info.connState !=
				eConnectionState_Associated) {
			hdd_err("failed to cap tsf, not connect with ap");
			ret = TSF_STA_NOT_CONNECTED_NO_TSF;
		}
	} else if ((adapter->device_mode == QDF_SAP_MODE ||
				adapter->device_mode == QDF_P2P_GO_MODE) &&
			!(test_bit(SOFTAP_BSS_STARTED,
					&adapter->event_flags))) {
		hdd_err("Soft AP / P2p GO not beaconing");
		ret = TSF_SAP_NOT_STARTED_NO_TSF;
	}
	return ret;
}

static bool hdd_tsf_is_initialized(hdd_adapter_t *adapter)
{
	hdd_context_t *hddctx;

	if (!adapter) {
		hdd_err("invalid adapter");
		return false;
	}

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return false;
	}

	if (!qdf_atomic_read(&hddctx->tsf_ready_flag)) {
		hdd_err("TSF is not initialized");
		return false;
	}

	return true;
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
static int hdd_tsf_reset_gpio(struct hdd_adapter_s *adapter)
{
	/* No GPIO Host timer sync for integrated WIFI Device */
	return TSF_RETURN;
}
#else
static int hdd_tsf_reset_gpio(struct hdd_adapter_s *adapter)
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

static enum hdd_tsf_op_result hdd_capture_tsf_internal(
	hdd_adapter_t *adapter, uint32_t *buf, int len)
{
	int ret;
	hdd_context_t *hddctx;

	if (adapter == NULL || buf == NULL) {
		hdd_err("invalid pointer");
		return HDD_TSF_OP_FAIL;
	}

	if (len != 1)
		return HDD_TSF_OP_FAIL;

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return HDD_TSF_OP_FAIL;
	}

	if (!hdd_tsf_is_initialized(adapter)) {
		buf[0] = TSF_NOT_READY;
		return HDD_TSF_OP_SUCC;
	}

	buf[0] = hdd_tsf_check_conn_state(adapter);
	if (buf[0] != TSF_RETURN)
		return HDD_TSF_OP_SUCC;

	if (qdf_atomic_inc_return(&hddctx->cap_tsf_flag) > 1) {
		hdd_err("current in capture state");
		buf[0] = TSF_CURRENT_IN_CAP_STATE;
		return HDD_TSF_OP_SUCC;
	}

	/* record adapter for cap_tsf_irq_handler  */
	hddctx->cap_tsf_context = adapter;

	hdd_err("+ioctl issue cap tsf cmd");

	/* Reset TSF value for new capture */
	adapter->cur_target_time = 0;

	buf[0] = TSF_RETURN;
	init_completion(&tsf_sync_get_completion_evt);
	ret = wma_cli_set_command((int)adapter->sessionId,
				  (int)GEN_PARAM_CAPTURE_TSF,
				  adapter->sessionId, GEN_CMD);
	if (QDF_STATUS_SUCCESS != ret) {
		hdd_err("cap tsf fail");
		buf[0] = TSF_CAPTURE_FAIL;
		hddctx->cap_tsf_context = NULL;
		qdf_atomic_set(&hddctx->cap_tsf_flag, 0);
		return HDD_TSF_OP_SUCC;
	}
	hdd_err("-ioctl return cap tsf cmd");
	return HDD_TSF_OP_SUCC;
}

static enum hdd_tsf_op_result hdd_indicate_tsf_internal(
	hdd_adapter_t *adapter, uint32_t *buf, int len)
{
	int ret;
	hdd_context_t *hddctx;

	if (!adapter || !buf) {
		hdd_err("invalid pointer");
		return HDD_TSF_OP_FAIL;
	}

	if (len != 3)
		return HDD_TSF_OP_FAIL;

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return HDD_TSF_OP_FAIL;
	}

	buf[1] = 0;
	buf[2] = 0;

	if (!hdd_tsf_is_initialized(adapter)) {
		buf[0] = TSF_NOT_READY;
		return HDD_TSF_OP_SUCC;
	}

	buf[0] = hdd_tsf_check_conn_state(adapter);
	if (buf[0] != TSF_RETURN)
		return HDD_TSF_OP_SUCC;

	if (adapter->cur_target_time == 0) {
		hdd_info("TSF value not received");
		buf[0] = TSF_NOT_RETURNED_BY_FW;
		return HDD_TSF_OP_SUCC;
	} else {
		buf[0] = TSF_RETURN;
		buf[1] = (uint32_t)(adapter->cur_target_time & 0xffffffff);
		buf[2] = (uint32_t)((adapter->cur_target_time >> 32) &
				0xffffffff);

		if (!qdf_atomic_read(&hddctx->cap_tsf_flag)) {
			hdd_info("old: status=%u, tsf_low=%u, tsf_high=%u",
				 buf[0], buf[1], buf[2]);
			return HDD_TSF_OP_SUCC;
		}

		ret = hdd_tsf_reset_gpio(adapter);
		if (0 != ret) {
			hdd_err("reset tsf gpio fail");
			buf[0] = TSF_RESET_GPIO_FAIL;
			return HDD_TSF_OP_SUCC;
		}
		hddctx->cap_tsf_context = NULL;
		qdf_atomic_set(&hddctx->cap_tsf_flag, 0);
		hdd_info("get tsf cmd,status=%u, tsf_low=%u, tsf_high=%u",
			buf[0], buf[1], buf[2]);
		return HDD_TSF_OP_SUCC;
	}
}

int hdd_capture_tsf(hdd_adapter_t *adapter, uint32_t *buf, int len)
{
	return (hdd_capture_tsf_internal(adapter, buf, len) ==
		HDD_TSF_OP_SUCC) ? 0 : -EINVAL;
}

int hdd_indicate_tsf(hdd_adapter_t *adapter, uint32_t *buf, int len)
{
	return (hdd_indicate_tsf_internal(adapter, buf, len) ==
		HDD_TSF_OP_SUCC) ? 0 : -EINVAL;
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
	if (0 != status)
		return -EINVAL;

	adapter = hdd_get_adapter_by_vdev(hddctx, ptsf->vdev_id);

	if (NULL == adapter) {
		hdd_err("failed to find adapter");
		return -EINVAL;
	}

	if (!hdd_tsf_is_initialized(adapter)) {
		hdd_err("tsf is not init, ignore tsf event");
		return -EINVAL;
	}

	hdd_info("tsf cb handle event, device_mode is %d",
		adapter->device_mode);

	adapter->cur_target_time = ((uint64_t)ptsf->tsf_high << 32 |
			 ptsf->tsf_low);
	adapter->tsf_sync_soc_timer = ((uint64_t) ptsf->soc_timer_high << 32 |
						  ptsf->soc_timer_low);

	complete(&tsf_sync_get_completion_evt);
	hdd_info("Vdev=%u, tsf_low=%u, tsf_high=%u soc_timer=%llu",
		ptsf->vdev_id, ptsf->tsf_low, ptsf->tsf_high,
		adapter->tsf_sync_soc_timer);
	return 0;
}

static const struct nla_policy tsf_policy[QCA_WLAN_VENDOR_ATTR_TSF_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TSF_CMD] = {.type = NLA_U32},
};

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
		      data_len, tsf_policy)) {
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
		if (hdd_wlan_nla_put_u64(reply_skb,
				QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE,
				adapter->cur_target_time) ||
		    hdd_wlan_nla_put_u64(reply_skb,
				QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE,
				adapter->tsf_sync_soc_timer)) {
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
	QDF_STATUS hal_status;

	if (!hdd_ctx)
		return;

	if (qdf_atomic_inc_return(&hdd_ctx->tsf_ready_flag) > 1)
		return;

	qdf_atomic_init(&hdd_ctx->cap_tsf_flag);

	if (hdd_ctx->config->tsf_gpio_pin == TSF_GPIO_PIN_INVALID)
		goto fail;

	hal_status = sme_set_tsf_gpio(hdd_ctx->hHal,
				      hdd_ctx->config->tsf_gpio_pin);
	if (QDF_STATUS_SUCCESS != hal_status) {
		hdd_err("set tsf GPIO failed, status: %d", hal_status);
		goto fail;
	}

	return;

fail:
	qdf_atomic_set(&hdd_ctx->tsf_ready_flag, 0);
}

void wlan_hdd_tsf_deinit(hdd_context_t *hdd_ctx)
{
	if (!hdd_ctx)
		return;

	if (!qdf_atomic_read(&hdd_ctx->tsf_ready_flag))
		return;

	qdf_atomic_set(&hdd_ctx->tsf_ready_flag, 0);
	qdf_atomic_set(&hdd_ctx->cap_tsf_flag, 0);
}
