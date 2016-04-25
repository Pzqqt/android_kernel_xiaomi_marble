/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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
 *  DOC: wlan_hdd_main.c
 *
 *  WLAN Host Device Driver implementation
 *
 */

/* Include Files */
#include <wlan_hdd_includes.h>
#include <cds_api.h>
#include <cds_sched.h>
#ifdef WLAN_FEATURE_LPSS
#include <cds_utils.h>
#endif
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <wlan_hdd_tx_rx.h>
#include <wni_api.h>
#include <wlan_hdd_cfg.h>
#include <wlan_ptt_sock_svc.h>
#include <dbglog_host.h>
#include <wlan_logging_sock_svc.h>
#include <wlan_hdd_wowl.h>
#include <wlan_hdd_misc.h>
#include <wlan_hdd_wext.h>
#include "wlan_hdd_trace.h"
#include "wlan_hdd_ioctl.h"
#include "wlan_hdd_ftm.h"
#include "wlan_hdd_power.h"
#include "wlan_hdd_stats.h"
#include "wlan_hdd_scan.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include <cdp_txrx_peer_ops.h>

#include <net/addrconf.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_ext_scan.h"
#include "wlan_hdd_p2p.h"
#include <linux/rtnetlink.h>
#include "sap_api.h"
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include <linux/compat.h>
#ifdef MSM_PLATFORM
#ifdef CONFIG_CNSS
#include <soc/qcom/subsystem_restart.h>
#endif
#endif
#include <wlan_hdd_hostapd.h>
#include <wlan_hdd_softap_tx_rx.h>
#include "cfg_api.h"
#include "qwlan_version.h"
#include "wma_types.h"
#include "wlan_hdd_tdls.h"
#ifdef FEATURE_WLAN_CH_AVOID
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include "cds_regdomain.h"
#include "cdp_txrx_flow_ctrl_v2.h"
#endif /* FEATURE_WLAN_CH_AVOID */
#include "pld_common.h"
#include "wlan_hdd_ocb.h"
#include "wlan_hdd_nan.h"
#include "wlan_hdd_debugfs.h"
#include "wlan_hdd_driver_ops.h"
#include "epping_main.h"
#include "wlan_hdd_memdump.h"

#include <wlan_hdd_ipa.h>
#include "hif.h"
#include "wma.h"
#include "cds_concurrency.h"
#include "wlan_hdd_tsf.h"
#include "wlan_hdd_green_ap.h"
#include "platform_icnss.h"
#include "bmi.h"
#include <wlan_hdd_regulatory.h>

#ifdef MODULE
#define WLAN_MODULE_NAME  module_name(THIS_MODULE)
#else
#define WLAN_MODULE_NAME  "wlan"
#endif

#ifdef TIMER_MANAGER
#define TIMER_MANAGER_STR " +TIMER_MANAGER"
#else
#define TIMER_MANAGER_STR ""
#endif

#ifdef MEMORY_DEBUG
#define MEMORY_DEBUG_STR " +MEMORY_DEBUG"
#else
#define MEMORY_DEBUG_STR ""
#endif

/* the Android framework expects this param even though we don't use it */
#define BUF_LEN 20
static char fwpath_buffer[BUF_LEN];
static struct kparam_string fwpath = {
	.string = fwpath_buffer,
	.maxlen = BUF_LEN,
};

static char *country_code;
static int enable_11d = -1;
static int enable_dfs_chan_scan = -1;

#ifndef MODULE
static int wlan_hdd_inited;
#endif

/*
 * spinlock for synchronizing asynchronous request/response
 * (full description of use in wlan_hdd_main.h)
 */
DEFINE_SPINLOCK(hdd_context_lock);

#define WLAN_NLINK_CESIUM 30

static qdf_wake_lock_t wlan_wake_lock;

#define WOW_MAX_FILTER_LISTS 1
#define WOW_MAX_FILTERS_PER_LIST 4
#define WOW_MIN_PATTERN_SIZE 6
#define WOW_MAX_PATTERN_SIZE 64

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
static const struct wiphy_wowlan_support wowlan_support_reg_init = {
	.flags = WIPHY_WOWLAN_ANY |
		 WIPHY_WOWLAN_MAGIC_PKT |
		 WIPHY_WOWLAN_DISCONNECT |
		 WIPHY_WOWLAN_SUPPORTS_GTK_REKEY |
		 WIPHY_WOWLAN_GTK_REKEY_FAILURE |
		 WIPHY_WOWLAN_EAP_IDENTITY_REQ |
		 WIPHY_WOWLAN_4WAY_HANDSHAKE |
		 WIPHY_WOWLAN_RFKILL_RELEASE,
	.n_patterns = WOW_MAX_FILTER_LISTS * WOW_MAX_FILTERS_PER_LIST,
	.pattern_min_len = WOW_MIN_PATTERN_SIZE,
	.pattern_max_len = WOW_MAX_PATTERN_SIZE,
};
#endif

/* internal function declaration */

struct sock *cesium_nl_srv_sock;

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
void wlan_hdd_auto_shutdown_cb(void);
#endif

/**
 * wlan_hdd_txrx_pause_cb() - pause callback from txrx layer
 * @vdev_id: vdev_id
 * @action: action type
 * @reason: reason type
 *
 * Return: none
 */
void wlan_hdd_txrx_pause_cb(uint8_t vdev_id,
		enum netif_action_type action, enum netif_reason_type reason)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	hdd_adapter_t *adapter;

	if (!hdd_ctx) {
		hdd_err("hdd ctx is NULL");
		return;
	}
	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);

	wlan_hdd_netif_queue_control(adapter, action, reason);
	return;
}

/*
 * Store WLAN driver version info in a global variable such that crash debugger
 * can extract it from driver debug symbol and crashdump for post processing
 */
uint8_t g_wlan_driver_version[] = QWLAN_VERSIONSTR;

/**
 * hdd_device_mode_to_string() - return string conversion of device mode
 * @device_mode: device mode
 *
 * This utility function helps log string conversion of device mode.
 *
 * Return: string conversion of device mode, if match found;
 *	   "Unknown" otherwise.
 */
const char *hdd_device_mode_to_string(uint8_t device_mode)
{
	switch (device_mode) {
	CASE_RETURN_STRING(QDF_STA_MODE);
	CASE_RETURN_STRING(QDF_SAP_MODE);
	CASE_RETURN_STRING(QDF_P2P_CLIENT_MODE);
	CASE_RETURN_STRING(QDF_P2P_GO_MODE);
	CASE_RETURN_STRING(QDF_FTM_MODE);
	CASE_RETURN_STRING(QDF_IBSS_MODE);
	CASE_RETURN_STRING(QDF_P2P_DEVICE_MODE);
	CASE_RETURN_STRING(QDF_OCB_MODE);
	default:
		return "Unknown";
	}
}

/**
 * hdd_validate_channel_and_bandwidth() - Validate the channel-bandwidth combo
 * @adapter: HDD adapter
 * @chan_number: Channel number
 * @chan_bw: Bandwidth
 *
 * Checks if the given bandwidth is valid for the given channel number.
 *
 * Return: 0 for success, non-zero for failure
 */
int hdd_validate_channel_and_bandwidth(hdd_adapter_t *adapter,
		uint32_t chan_number,
		enum phy_ch_width chan_bw)
{
	uint8_t chan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint32_t len = WNI_CFG_VALID_CHANNEL_LIST_LEN, i;
	bool found = false;
	tHalHandle hal;

	hal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (!hal) {
		hdd_err("Invalid HAL context");
		return -EINVAL;
	}

	if (0 != sme_cfg_get_str(hal, WNI_CFG_VALID_CHANNEL_LIST, chan, &len)) {
		hdd_err("No valid channel list");
		return -EOPNOTSUPP;
	}

	for (i = 0; i < len; i++) {
		if (chan[i] == chan_number) {
			found = true;
			break;
		}
	}

	if (found == false) {
		hdd_err("Channel not in driver's valid channel list");
		return -EOPNOTSUPP;
	}

	if ((!CDS_IS_CHANNEL_24GHZ(chan_number)) &&
			(!CDS_IS_CHANNEL_5GHZ(chan_number))) {
		hdd_err("CH %d is not in 2.4GHz or 5GHz", chan_number);
		return -EINVAL;
	}

	if (CDS_IS_CHANNEL_24GHZ(chan_number)) {
		if (chan_bw == CH_WIDTH_80MHZ) {
			hdd_err("BW80 not possible in 2.4GHz band");
			return -EINVAL;
		}
		if ((chan_bw != CH_WIDTH_20MHZ) && (chan_number == 14) &&
				(chan_bw != CH_WIDTH_MAX)) {
			hdd_err("Only BW20 possible on channel 14");
			return -EINVAL;
		}
	}

	if (CDS_IS_CHANNEL_5GHZ(chan_number)) {
		if ((chan_bw != CH_WIDTH_20MHZ) && (chan_number == 165) &&
				(chan_bw != CH_WIDTH_MAX)) {
			hdd_err("Only BW20 possible on channel 165");
			return -EINVAL;
		}
	}

	return 0;
}

static int __hdd_netdev_notifier_call(struct notifier_block *nb,
				    unsigned long state, void *data)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	struct netdev_notifier_info *dev_notif_info = data;
	struct net_device *dev = dev_notif_info->dev;
#else
	struct net_device *dev = data;
#endif
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;

	ENTER_DEV(dev);

	/* Make sure that this callback corresponds to our device. */
	if ((strncmp(dev->name, "wlan", 4)) && (strncmp(dev->name, "p2p", 3)))
		return NOTIFY_DONE;

	if ((adapter->magic != WLAN_HDD_ADAPTER_MAGIC) &&
	    (adapter->dev != dev)) {
		hddLog(LOGE, FL("device adapter is not matching!!!"));
		return NOTIFY_DONE;
	}

	if (!dev->ieee80211_ptr) {
		hddLog(LOGE, FL("ieee80211_ptr is NULL!!!"));
		return NOTIFY_DONE;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (NULL == hdd_ctx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("HDD Context Null Pointer"));
		QDF_ASSERT(0);
		return NOTIFY_DONE;
	}
	if (cds_is_driver_recovering())
		return NOTIFY_DONE;

	hddLog(QDF_TRACE_LEVEL_INFO, FL("%s New Net Device State = %lu"),
	       dev->name, state);

	switch (state) {
	case NETDEV_REGISTER:
		break;

	case NETDEV_UNREGISTER:
		break;

	case NETDEV_UP:
		sme_ch_avoid_update_req(hdd_ctx->hHal);
		break;

	case NETDEV_DOWN:
		break;

	case NETDEV_CHANGE:
		if (true == adapter->isLinkUpSvcNeeded)
			complete(&adapter->linkup_event_var);
		break;

	case NETDEV_GOING_DOWN:
		if (adapter->scan_info.mScanPending != false) {
			unsigned long rc;
			INIT_COMPLETION(adapter->scan_info.
					abortscan_event_var);
			hdd_abort_mac_scan(adapter->pHddCtx,
					   adapter->sessionId,
					   eCSR_SCAN_ABORT_DEFAULT);
			rc = wait_for_completion_timeout(
				&adapter->scan_info.abortscan_event_var,
				msecs_to_jiffies(WLAN_WAIT_TIME_ABORTSCAN));
			if (!rc) {
				hddLog(LOGE,
				       FL("Timeout occurred while waiting for abortscan"));
			}
		} else {
			hddLog(QDF_TRACE_LEVEL_INFO,
			       FL("Scan is not Pending from user"));
		}
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

/**
 * hdd_netdev_notifier_call() - netdev notifier callback function
 * @nb: pointer to notifier block
 * @state: state
 * @ndev: ndev pointer
 *
 * Return: 0 on success, error number otherwise.
 */
static int hdd_netdev_notifier_call(struct notifier_block *nb,
					unsigned long state,
					void *ndev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_netdev_notifier_call(nb, state, ndev);
	cds_ssr_unprotect(__func__);

	return ret;
}

struct notifier_block hdd_netdev_notifier = {
	.notifier_call = hdd_netdev_notifier_call,
};

/* variable to hold the insmod parameters */
static int con_mode;

/* Variable to hold connection mode including module parameter con_mode */
static int curr_con_mode;

/**
 * hdd_map_nl_chan_width() - Map NL channel width to internal representation
 * @ch_width: NL channel width
 *
 * Converts the NL channel width to the driver's internal representation
 *
 * Return: Converted channel width. In case of non matching NL channel width,
 * CH_WIDTH_MAX will be returned.
 */
enum phy_ch_width hdd_map_nl_chan_width(enum nl80211_chan_width ch_width)
{
	switch (ch_width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		return CH_WIDTH_20MHZ;
	case NL80211_CHAN_WIDTH_40:
		return CH_WIDTH_40MHZ;
		break;
	case NL80211_CHAN_WIDTH_80:
		return CH_WIDTH_80MHZ;
	case NL80211_CHAN_WIDTH_80P80:
		return CH_WIDTH_80P80MHZ;
	case NL80211_CHAN_WIDTH_160:
		return CH_WIDTH_160MHZ;
	case NL80211_CHAN_WIDTH_5:
		return CH_WIDTH_5MHZ;
	case NL80211_CHAN_WIDTH_10:
		return CH_WIDTH_10MHZ;
	default:
		hdd_err("Invalid channel width %d, setting to default",
				ch_width);
		return CH_WIDTH_INVALID;
	}
}

/* wlan_hdd_find_opclass() - Find operating class for a channel
 * @hal: handler to HAL
 * @channel: channel id
 * @bw_offset: bandwidth offset
 *
 * Function invokes sme api to find the operating class
 *
 * Return: operating class
 */
uint8_t wlan_hdd_find_opclass(tHalHandle hal, uint8_t channel,
				uint8_t bw_offset)
{
	uint8_t opclass = 0;

	sme_get_opclass(hal, channel, bw_offset, &opclass);
	return opclass;
}

/**
 * hdd_qdf_trace_enable() - configure initial QDF Trace enable
 * @moduleId:	Module whose trace level is being configured
 * @bitmask:	Bitmask of log levels to be enabled
 *
 * Called immediately after the cfg.ini is read in order to configure
 * the desired trace levels.
 *
 * Return: None
 */
static void hdd_qdf_trace_enable(QDF_MODULE_ID moduleId, uint32_t bitmask)
{
	QDF_TRACE_LEVEL level;

	/*
	 * if the bitmask is the default value, then a bitmask was not
	 * specified in cfg.ini, so leave the logging level alone (it
	 * will remain at the "compiled in" default value)
	 */
	if (CFG_QDF_TRACE_ENABLE_DEFAULT == bitmask) {
		return;
	}

	/* a mask was specified.  start by disabling all logging */
	qdf_trace_set_value(moduleId, QDF_TRACE_LEVEL_NONE, 0);

	/* now cycle through the bitmask until all "set" bits are serviced */
	level = QDF_TRACE_LEVEL_FATAL;
	while (0 != bitmask) {
		if (bitmask & 1) {
			qdf_trace_set_value(moduleId, level, 1);
		}
		level++;
		bitmask >>= 1;
	}
}

/**
 * wlan_hdd_validate_context() - check the HDD context
 * @hdd_ctx:	HDD context pointer
 *
 * Return: 0 if the context is valid. Error code otherwise
 */
int wlan_hdd_validate_context(hdd_context_t *hdd_ctx)
{
	if (NULL == hdd_ctx || NULL == hdd_ctx->config) {
		hddLog(LOGE, FL("HDD context is Null"));
		return -ENODEV;
	}

	if (cds_is_driver_recovering()) {
		hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
			 cds_get_driver_state());
		return -EAGAIN;
	}

	if (cds_is_load_or_unload_in_progress()) {
		hdd_err("Unloading/Loading in Progress. Ignore!!!: 0x%x",
			cds_get_driver_state());
		return -EAGAIN;
	}
	return 0;
}

void hdd_checkandupdate_phymode(hdd_context_t *hdd_ctx)
{
	hdd_adapter_t *adapter = NULL;
	hdd_station_ctx_t *pHddStaCtx = NULL;
	eCsrPhyMode phyMode;
	struct hdd_config *cfg_param = NULL;

	if (NULL == hdd_ctx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("HDD Context is null !!"));
		return;
	}

	adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
	if (NULL == adapter) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("adapter is null !!"));
		return;
	}

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	cfg_param = hdd_ctx->config;
	if (NULL == cfg_param) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("cfg_params not available !!"));
		return;
	}

	phyMode = sme_get_phy_mode(WLAN_HDD_GET_HAL_CTX(adapter));

	if (!hdd_ctx->isVHT80Allowed) {
		if ((eCSR_DOT11_MODE_AUTO == phyMode) ||
		    (eCSR_DOT11_MODE_11ac == phyMode) ||
		    (eCSR_DOT11_MODE_11ac_ONLY == phyMode)) {
			hddLog(QDF_TRACE_LEVEL_INFO,
			       FL("Setting phymode to 11n!!"));
			sme_set_phy_mode(WLAN_HDD_GET_HAL_CTX(adapter),
					 eCSR_DOT11_MODE_11n);
		}
	} else {
		/*
		 * New country Supports 11ac as well resetting value back from
		 * .ini
		 */
		sme_set_phy_mode(WLAN_HDD_GET_HAL_CTX(adapter),
				 hdd_cfg_xlate_to_csr_phy_mode(cfg_param->
							       dot11Mode));
		return;
	}

	if ((eConnectionState_Associated == pHddStaCtx->conn_info.connState) &&
	    ((eCSR_CFG_DOT11_MODE_11AC_ONLY == pHddStaCtx->conn_info.dot11Mode)
	     || (eCSR_CFG_DOT11_MODE_11AC ==
		 pHddStaCtx->conn_info.dot11Mode))) {
		QDF_STATUS qdf_status;

		/* need to issue a disconnect to CSR. */
		INIT_COMPLETION(adapter->disconnect_comp_var);
		qdf_status = sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(adapter),
						 adapter->sessionId,
						 eCSR_DISCONNECT_REASON_UNSPECIFIED);

		if (QDF_STATUS_SUCCESS == qdf_status) {
			unsigned long rc;

			rc = wait_for_completion_timeout(
				&adapter->disconnect_comp_var,
				msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
			if (!rc)
				hddLog(LOGE,
				       FL("failure waiting for disconnect_comp_var"));
		}
	}
}

/**
 * hdd_set_ibss_power_save_params() - update IBSS Power Save params to WMA.
 * @hdd_adapter_t Hdd adapter.
 *
 * This function sets the IBSS power save config parameters to WMA
 * which will send it to firmware if FW supports IBSS power save
 * before vdev start.
 *
 * Return: QDF_STATUS QDF_STATUS_SUCCESS on Success and QDF_STATUS_E_FAILURE
 *         on failure.
 */
QDF_STATUS hdd_set_ibss_power_save_params(hdd_adapter_t *adapter)
{
	int ret;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (hdd_ctx == NULL) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("HDD context is null"));
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_SET_ATIM_WINDOW_SIZE,
				  hdd_ctx->config->ibssATIMWinSize,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WMA_VDEV_IBSS_SET_ATIM_WINDOW_SIZE failed %d"), ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_SET_POWER_SAVE_ALLOWED,
				  hdd_ctx->config->isIbssPowerSaveAllowed,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WMA_VDEV_IBSS_SET_POWER_SAVE_ALLOWED failed %d"),
		       ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_SET_POWER_COLLAPSE_ALLOWED,
				  hdd_ctx->config->
				  isIbssPowerCollapseAllowed, VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WMA_VDEV_IBSS_SET_POWER_COLLAPSE_ALLOWED failed %d"),
		       ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_SET_AWAKE_ON_TX_RX,
				  hdd_ctx->config->isIbssAwakeOnTxRx,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WMA_VDEV_IBSS_SET_AWAKE_ON_TX_RX failed %d"), ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_SET_INACTIVITY_TIME,
				  hdd_ctx->config->ibssInactivityCount,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WMA_VDEV_IBSS_SET_INACTIVITY_TIME failed %d"), ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_SET_TXSP_END_INACTIVITY_TIME,
				  hdd_ctx->config->ibssTxSpEndInactivityTime,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL(
			  "WMA_VDEV_IBSS_SET_TXSP_END_INACTIVITY_TIME failed %d"
			 ),
		       ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_PS_SET_WARMUP_TIME_SECS,
				  hdd_ctx->config->ibssPsWarmupTime,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WMA_VDEV_IBSS_PS_SET_WARMUP_TIME_SECS failed %d"),
		       ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_cli_set_command(adapter->sessionId,
				  WMA_VDEV_IBSS_PS_SET_1RX_CHAIN_IN_ATIM_WINDOW,
				  hdd_ctx->config->ibssPs1RxChainInAtimEnable,
				  VDEV_CMD);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL(
			  "WMA_VDEV_IBSS_PS_SET_1RX_CHAIN_IN_ATIM_WINDOW failed %d"
			 ),
		       ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#define INTF_MACADDR_MASK       0x7

/**
 * hdd_update_macaddr() - update mac address
 * @config:	hdd configuration
 * @hw_macaddr:	mac address
 *
 * Mac address for multiple virtual interface is found as following
 * i) The mac address of the first interface is just the actual hw mac address.
 * ii) MSM 3 or 4 bits of byte5 of the actual mac address are used to
 *     define the mac address for the remaining interfaces and locally
 *     admistered bit is set. INTF_MACADDR_MASK is based on the number of
 *     supported virtual interfaces, right now this is 0x07 (meaning 8
 *     interface).
 *     Byte[3] of second interface will be hw_macaddr[3](bit5..7) + 1,
 *     for third interface it will be hw_macaddr[3](bit5..7) + 2, etc.
 *
 * Return: None
 */
void hdd_update_macaddr(struct hdd_config *config,
			struct qdf_mac_addr hw_macaddr)
{
	int8_t i;
	uint8_t macaddr_b3, tmp_br3;

	qdf_mem_copy(config->intfMacAddr[0].bytes, hw_macaddr.bytes,
		     QDF_MAC_ADDR_SIZE);
	for (i = 1; i < QDF_MAX_CONCURRENCY_PERSONA; i++) {
		qdf_mem_copy(config->intfMacAddr[i].bytes, hw_macaddr.bytes,
			     QDF_MAC_ADDR_SIZE);
		macaddr_b3 = config->intfMacAddr[i].bytes[3];
		tmp_br3 = ((macaddr_b3 >> 4 & INTF_MACADDR_MASK) + i) &
			  INTF_MACADDR_MASK;
		macaddr_b3 += tmp_br3;

		/* XOR-ing bit-24 of the mac address. This will give enough
		 * mac address range before collision
		 */
		macaddr_b3 ^= (1 << 7);

		/* Set locally administered bit */
		config->intfMacAddr[i].bytes[0] |= 0x02;
		config->intfMacAddr[i].bytes[3] = macaddr_b3;
		hddLog(QDF_TRACE_LEVEL_INFO, "config->intfMacAddr[%d]: "
		       MAC_ADDRESS_STR, i,
		       MAC_ADDR_ARRAY(config->intfMacAddr[i].bytes));
	}
}

static void hdd_update_tgt_services(hdd_context_t *hdd_ctx,
				    struct wma_tgt_services *cfg)
{
	struct hdd_config *config = hdd_ctx->config;

	/* Set up UAPSD */
	config->apUapsdEnabled &= cfg->uapsd;

	/* 11AC mode support */
	if ((config->dot11Mode == eHDD_DOT11_MODE_11ac ||
	     config->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY) && !cfg->en_11ac)
		config->dot11Mode = eHDD_DOT11_MODE_AUTO;

	/* ARP offload: override user setting if invalid  */
	config->fhostArpOffload &= cfg->arp_offload;

#ifdef FEATURE_WLAN_SCAN_PNO
	/* PNO offload */
	hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
	       FL("PNO Capability in f/w = %d"), cfg->pno_offload);
	if (cfg->pno_offload)
		config->PnoOffload = true;
#endif
#ifdef FEATURE_WLAN_TDLS
	config->fEnableTDLSSupport &= cfg->en_tdls;
	config->fEnableTDLSOffChannel &= cfg->en_tdls_offchan;
	config->fEnableTDLSBufferSta &= cfg->en_tdls_uapsd_buf_sta;
	if (config->fTDLSUapsdMask && cfg->en_tdls_uapsd_sleep_sta) {
		config->fEnableTDLSSleepSta = true;
	} else {
		config->fEnableTDLSSleepSta = false;
	}
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	config->isRoamOffloadEnabled &= cfg->en_roam_offload;
#endif
	sme_update_tgt_services(hdd_ctx->hHal, cfg);

}

static void hdd_update_tgt_ht_cap(hdd_context_t *hdd_ctx,
				  struct wma_tgt_ht_cap *cfg)
{
	QDF_STATUS status;
	uint32_t value, val32;
	uint16_t val16;
	struct hdd_config *pconfig = hdd_ctx->config;
	tSirMacHTCapabilityInfo *phtCapInfo;
	uint8_t mcs_set[SIZE_OF_SUPPORTED_MCS_SET];
	uint8_t enable_tx_stbc;

	/* check and update RX STBC */
	if (pconfig->enableRxSTBC && !cfg->ht_rx_stbc)
		pconfig->enableRxSTBC = cfg->ht_rx_stbc;

	/* get the MPDU density */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_MPDU_DENSITY, &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get MPDU DENSITY"));
		value = 0;
	}

	/*
	 * MPDU density:
	 * override user's setting if value is larger
	 * than the one supported by target
	 */
	if (value > cfg->mpdu_density) {
		status = sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_MPDU_DENSITY,
					 cfg->mpdu_density);

		if (status == QDF_STATUS_E_FAILURE)
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set MPDU DENSITY to CCM"));
	}

	/* get the HT capability info */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_HT_CAP_INFO, &val32);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get HT capability info"));
		return;
	}
	val16 = (uint16_t) val32;
	phtCapInfo = (tSirMacHTCapabilityInfo *) &val16;

	/* Set the LDPC capability */
	phtCapInfo->advCodingCap = cfg->ht_rx_ldpc;

	if (pconfig->ShortGI20MhzEnable && !cfg->ht_sgi_20)
		pconfig->ShortGI20MhzEnable = cfg->ht_sgi_20;

	if (pconfig->ShortGI40MhzEnable && !cfg->ht_sgi_40)
		pconfig->ShortGI40MhzEnable = cfg->ht_sgi_40;

	hdd_ctx->num_rf_chains = cfg->num_rf_chains;
	hdd_ctx->ht_tx_stbc_supported = cfg->ht_tx_stbc;

	enable_tx_stbc = pconfig->enableTxSTBC;

	if (pconfig->enable2x2 && (cfg->num_rf_chains == 2)) {
		pconfig->enable2x2 = 1;
	} else {
		pconfig->enable2x2 = 0;
		enable_tx_stbc = 0;

		/* 1x1 */
		/* Update Rx Highest Long GI data Rate */
		if (sme_cfg_set_int(hdd_ctx->hHal,
				    WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE,
				    HDD_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_1_1)
				== QDF_STATUS_E_FAILURE) {
			hddLog(LOGE,
			       FL(
				  "Could not pass on WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE to CCM"
				 ));
		}

		/* Update Tx Highest Long GI data Rate */
		if (sme_cfg_set_int
			    (hdd_ctx->hHal,
			     WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE,
			     HDD_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_1_1) ==
			    QDF_STATUS_E_FAILURE) {
			hddLog(LOGE,
			       FL(
				  "Could not pass on HDD_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_1_1 to CCM"
				 ));
		}
	}
	if (!(cfg->ht_tx_stbc && pconfig->enable2x2))
		enable_tx_stbc = 0;
	phtCapInfo->txSTBC = enable_tx_stbc;

	val32 = val16;
	status = sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_HT_CAP_INFO, val32);
	if (status != QDF_STATUS_SUCCESS)
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("could not set HT capability to CCM"));
#define WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES 0xff
	value = SIZE_OF_SUPPORTED_MCS_SET;
	if (sme_cfg_get_str(hdd_ctx->hHal, WNI_CFG_SUPPORTED_MCS_SET, mcs_set,
			    &value) == QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_INFO, FL("Read MCS rate set"));

		if (pconfig->enable2x2) {
			for (value = 0; value < cfg->num_rf_chains; value++)
				mcs_set[value] =
					WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES;

			status =
				sme_cfg_set_str(hdd_ctx->hHal,
						WNI_CFG_SUPPORTED_MCS_SET,
						mcs_set,
						SIZE_OF_SUPPORTED_MCS_SET);
			if (status == QDF_STATUS_E_FAILURE)
				hddLog(QDF_TRACE_LEVEL_FATAL,
				       FL("could not set MCS SET to CCM"));
		}
	}
#undef WLAN_HDD_RX_MCS_ALL_NSTREAM_RATES
}

static void hdd_update_tgt_vht_cap(hdd_context_t *hdd_ctx,
				   struct wma_tgt_vht_cap *cfg)
{
	QDF_STATUS status;
	uint32_t value = 0;
	struct hdd_config *pconfig = hdd_ctx->config;
	struct wiphy *wiphy = hdd_ctx->wiphy;
	struct ieee80211_supported_band *band_5g =
		wiphy->bands[IEEE80211_BAND_5GHZ];

	/* Get the current MPDU length */
	status =
		sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_MAX_MPDU_LENGTH,
				&value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("could not get MPDU LENGTH"));
		value = 0;
	}

	/*
	 * VHT max MPDU length:
	 * override if user configured value is too high
	 * that the target cannot support
	 */
	if (value > cfg->vht_max_mpdu) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_MAX_MPDU_LENGTH,
					 cfg->vht_max_mpdu);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set VHT MAX MPDU LENGTH"));
		}
	}

	/* Get the current supported chan width */
	status = sme_cfg_get_int(hdd_ctx->hHal,
				WNI_CFG_VHT_SUPPORTED_CHAN_WIDTH_SET,
				&value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get MPDU LENGTH"));
		value = 0;
	}

	/* Get the current RX LDPC setting */
	status =
		sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_LDPC_CODING_CAP,
				&value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT LDPC CODING CAP"));
		value = 0;
	}

	/* Set the LDPC capability */
	if (value && !cfg->vht_rx_ldpc) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_LDPC_CODING_CAP,
					 cfg->vht_rx_ldpc);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set VHT LDPC CODING CAP to CCM"));
		}
	}

	/* Get current GI 80 value */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_SHORT_GI_80MHZ,
				&value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get SHORT GI 80MHZ"));
		value = 0;
	}

	/* set the Guard interval 80MHz */
	if (value && !cfg->vht_short_gi_80) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_SHORT_GI_80MHZ,
					 cfg->vht_short_gi_80);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set SHORT GI 80MHZ to CCM"));
		}
	}

	/* Get current GI 160 value */
	status = sme_cfg_get_int(hdd_ctx->hHal,
				 WNI_CFG_VHT_SHORT_GI_160_AND_80_PLUS_80MHZ,
				 &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get SHORT GI 80 & 160"));
		value = 0;
	}

	/* Get VHT TX STBC cap */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_TXSTBC, &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT TX STBC"));
		value = 0;
	}

	/* VHT TX STBC cap */
	if (value && !cfg->vht_tx_stbc) {
		status = sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_VHT_TXSTBC,
					 cfg->vht_tx_stbc);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set the VHT TX STBC to CCM"));
		}
	}

	/* Get VHT RX STBC cap */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_RXSTBC, &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT RX STBC"));
		value = 0;
	}

	/* VHT RX STBC cap */
	if (value && !cfg->vht_rx_stbc) {
		status = sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_VHT_RXSTBC,
					 cfg->vht_rx_stbc);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set the VHT RX STBC to CCM"));
		}
	}

	/* Get VHT SU Beamformer cap */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_SU_BEAMFORMER_CAP,
				 &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT SU BEAMFORMER CAP"));
		value = 0;
	}

	/* set VHT SU Beamformer cap */
	if (value && !cfg->vht_su_bformer) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_SU_BEAMFORMER_CAP,
					 cfg->vht_su_bformer);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set VHT SU BEAMFORMER CAP"));
		}
	}

	/* check and update SU BEAMFORMEE capabality */
	if (pconfig->enableTxBF && !cfg->vht_su_bformee)
		pconfig->enableTxBF = cfg->vht_su_bformee;

	status = sme_cfg_set_int(hdd_ctx->hHal,
				 WNI_CFG_VHT_SU_BEAMFORMEE_CAP,
				 pconfig->enableTxBF);

	if (status == QDF_STATUS_E_FAILURE) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("could not set VHT SU BEAMFORMEE CAP"));
	}

	/* Get VHT MU Beamformer cap */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_MU_BEAMFORMER_CAP,
				 &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT MU BEAMFORMER CAP"));
		value = 0;
	}

	/* set VHT MU Beamformer cap */
	if (value && !cfg->vht_mu_bformer) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_MU_BEAMFORMER_CAP,
					 cfg->vht_mu_bformer);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL(
				  "could not set the VHT MU BEAMFORMER CAP to CCM"
				 ));
		}
	}

	/* Get VHT MU Beamformee cap */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_MU_BEAMFORMEE_CAP,
				 &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT MU BEAMFORMEE CAP"));
		value = 0;
	}

	/* set VHT MU Beamformee cap */
	if (value && !cfg->vht_mu_bformee) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_MU_BEAMFORMEE_CAP,
					 cfg->vht_mu_bformee);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set VHT MU BEAMFORMER CAP"));
		}
	}

	/* Get VHT MAX AMPDU Len exp */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_AMPDU_LEN_EXPONENT,
				 &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT AMPDU LEN"));
		value = 0;
	}

	/*
	 * VHT max AMPDU len exp:
	 * override if user configured value is too high
	 * that the target cannot support.
	 * Even though Rome publish ampdu_len=7, it can
	 * only support 4 because of some h/w bug.
	 */

	if (value > cfg->vht_max_ampdu_len_exp) {
		status = sme_cfg_set_int(hdd_ctx->hHal,
					 WNI_CFG_VHT_AMPDU_LEN_EXPONENT,
					 cfg->vht_max_ampdu_len_exp);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set the VHT AMPDU LEN EXP"));
		}
	}

	/* Get VHT TXOP PS CAP */
	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_VHT_TXOP_PS, &value);

	if (status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("could not get VHT TXOP PS"));
		value = 0;
	}

	/* set VHT TXOP PS cap */
	if (value && !cfg->vht_txop_ps) {
		status = sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_VHT_TXOP_PS,
					 cfg->vht_txop_ps);

		if (status == QDF_STATUS_E_FAILURE) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("could not set the VHT TXOP PS"));
		}
	}

	if (WMI_VHT_CAP_MAX_MPDU_LEN_11454 == cfg->vht_max_mpdu)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454;
	else if (WMI_VHT_CAP_MAX_MPDU_LEN_7935 == cfg->vht_max_mpdu)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991;
	else
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895;


	if (cfg->supp_chan_width & (1 << eHT_CHANNEL_WIDTH_80P80MHZ))
		band_5g->vht_cap.cap |=
			IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ;
	else if (cfg->supp_chan_width & (1 << eHT_CHANNEL_WIDTH_160MHZ))
		band_5g->vht_cap.cap |=
			IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;

	if (cfg->vht_rx_ldpc & WMI_VHT_CAP_RX_LDPC)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_RXLDPC;

	if (cfg->vht_short_gi_80 & WMI_VHT_CAP_SGI_80MHZ)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
	if (cfg->vht_short_gi_160 & WMI_VHT_CAP_SGI_160MHZ)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_160;

	if (cfg->vht_tx_stbc & WMI_VHT_CAP_TX_STBC)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_TXSTBC;

	if (cfg->vht_rx_stbc & WMI_VHT_CAP_RX_STBC_1SS)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
	if (cfg->vht_rx_stbc & WMI_VHT_CAP_RX_STBC_2SS)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_2;
	if (cfg->vht_rx_stbc & WMI_VHT_CAP_RX_STBC_3SS)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_3;

	band_5g->vht_cap.cap |=
		(cfg->vht_max_ampdu_len_exp <<
		 IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT);

	if (cfg->vht_su_bformer & WMI_VHT_CAP_SU_BFORMER)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
	if (cfg->vht_su_bformee & WMI_VHT_CAP_SU_BFORMEE)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
	if (cfg->vht_mu_bformer & WMI_VHT_CAP_MU_BFORMER)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;
	if (cfg->vht_mu_bformee & WMI_VHT_CAP_MU_BFORMEE)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;

	if (cfg->vht_txop_ps & WMI_VHT_CAP_TXOP_PS)
		band_5g->vht_cap.cap |= IEEE80211_VHT_CAP_VHT_TXOP_PS;

}

void hdd_update_tgt_cfg(void *context, void *param)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *) context;
	struct wma_tgt_cfg *cfg = param;
	uint8_t temp_band_cap;

	/* first store the INI band capability */
	temp_band_cap = hdd_ctx->config->nBandCapability;

	hdd_ctx->config->nBandCapability = cfg->band_cap;

	/* now overwrite the target band capability with INI
	   setting if INI setting is a subset */

	if ((hdd_ctx->config->nBandCapability == eCSR_BAND_ALL) &&
	    (temp_band_cap != eCSR_BAND_ALL))
		hdd_ctx->config->nBandCapability = temp_band_cap;
	else if ((hdd_ctx->config->nBandCapability != eCSR_BAND_ALL) &&
		 (temp_band_cap != eCSR_BAND_ALL) &&
		 (hdd_ctx->config->nBandCapability != temp_band_cap)) {
		hddLog(QDF_TRACE_LEVEL_WARN,
		       FL("ini BandCapability not supported by the target"));
	}

	if (!cds_is_driver_recovering()) {
		hdd_ctx->reg.reg_domain = cfg->reg_domain;
		hdd_ctx->reg.eeprom_rd_ext = cfg->eeprom_rd_ext;
	}

	/* This can be extended to other configurations like ht, vht cap... */

	if (!qdf_is_macaddr_zero(&cfg->hw_macaddr)) {
		hdd_update_macaddr(hdd_ctx->config, cfg->hw_macaddr);
	} else {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL(
			  "Invalid MAC passed from target, using MAC from ini file"
			  MAC_ADDRESS_STR),
		       MAC_ADDR_ARRAY(hdd_ctx->config->intfMacAddr[0].bytes));
	}

	hdd_ctx->target_fw_version = cfg->target_fw_version;

	hdd_ctx->max_intf_count = cfg->max_intf_count;

#ifdef WLAN_FEATURE_LPSS
	hdd_ctx->lpss_support = cfg->lpss_support;
#endif

	hdd_wlan_set_egap_support(hdd_ctx, cfg);

	hdd_ctx->ap_arpns_support = cfg->ap_arpns_support;
	hdd_update_tgt_services(hdd_ctx, &cfg->services);

	hdd_update_tgt_ht_cap(hdd_ctx, &cfg->ht_cap);

	hdd_update_tgt_vht_cap(hdd_ctx, &cfg->vht_cap);
	hdd_ctx->config->fine_time_meas_cap &= cfg->fine_time_measurement_cap;
	hdd_ctx->fine_time_meas_cap_target = cfg->fine_time_measurement_cap;
	hdd_info(FL("fine_time_meas_cap: 0x%x"),
		hdd_ctx->config->fine_time_meas_cap);

	hdd_ctx->current_antenna_mode =
		(hdd_ctx->config->enable2x2 == 0x01) ?
		HDD_ANTENNA_MODE_2X2 : HDD_ANTENNA_MODE_1X1;
	hdd_info("Init current antenna mode: %d",
		 hdd_ctx->current_antenna_mode);

	hdd_ctx->bpf_enabled = cfg->bpf_enabled;
}

/**
 * hdd_dfs_indicate_radar() - handle radar detection on current SAP channel
 * @context:	HDD context pointer
 * @param:	HDD radar indication pointer
 *
 * This function is invoked when a radar in found on the
 * SAP current operating channel and Data Tx from netif
 * has to be stopped to honor the DFS regulations.
 * Actions: Stop the netif Tx queues,Indicate Radar present
 * in HDD context for future usage.
 *
 * Return: true to allow radar indication to host else false
 */
bool hdd_dfs_indicate_radar(void *context, void *param)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *) context;
	struct wma_dfs_radar_ind *hdd_radar_event =
		(struct wma_dfs_radar_ind *)param;
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *adapter;
	QDF_STATUS status;

	if (!hdd_ctx || !hdd_radar_event ||
		 hdd_ctx->config->disableDFSChSwitch)
		return true;

	if (true == hdd_radar_event->dfs_radar_status) {
		mutex_lock(&hdd_ctx->dfs_lock);
		if (hdd_ctx->dfs_radar_found) {
			/*
			 * Application already triggered channel switch
			 * on current channel, so return here.
			 */
			mutex_unlock(&hdd_ctx->dfs_lock);
			return false;
		}

		hdd_ctx->dfs_radar_found = true;
		mutex_unlock(&hdd_ctx->dfs_lock);

		status = hdd_get_front_adapter(hdd_ctx, &adapterNode);
		while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
			adapter = adapterNode->pAdapter;
			if (QDF_SAP_MODE == adapter->device_mode ||
			    QDF_P2P_GO_MODE == adapter->device_mode) {
				WLAN_HDD_GET_AP_CTX_PTR(adapter)->
				dfs_cac_block_tx = true;
			}

			status = hdd_get_next_adapter(hdd_ctx,
						      adapterNode,
						      &pNext);
			adapterNode = pNext;
		}
	}

	return true;
}

/**
 * hdd_is_valid_mac_address() - validate MAC address
 * @pMacAddr:	Pointer to the input MAC address
 *
 * This function validates whether the given MAC address is valid or not
 * Expected MAC address is of the format XX:XX:XX:XX:XX:XX
 * where X is the hexa decimal digit character and separated by ':'
 * This algorithm works even if MAC address is not separated by ':'
 *
 * This code checks given input string mac contains exactly 12 hexadecimal
 * digits and a separator colon : appears in the input string only after
 * an even number of hex digits.
 *
 * Return: 1 for valid and 0 for invalid
 */
bool hdd_is_valid_mac_address(const uint8_t *pMacAddr)
{
	int xdigit = 0;
	int separator = 0;
	while (*pMacAddr) {
		if (isxdigit(*pMacAddr)) {
			xdigit++;
		} else if (':' == *pMacAddr) {
			if (0 == xdigit || ((xdigit / 2) - 1) != separator)
				break;

			++separator;
		} else {
			/* Invalid MAC found */
			return 0;
		}
		++pMacAddr;
	}
	return xdigit == 12 && (separator == 5 || separator == 0);
}

/**
 * __hdd__mon_open() - HDD Open function
 * @dev: Pointer to net_device structure
 *
 * This is called in response to ifconfig up
 *
 * Return: 0 for success; non-zero for failure
 */
static int __hdd_mon_open(struct net_device *dev)
{
	int ret;

	ENTER_DEV(dev);
	ret = hdd_set_mon_rx_cb(dev);
	return ret;
}

/**
 * hdd_mon_open() - Wrapper function for __hdd_mon_open to protect it from SSR
 * @dev:	Pointer to net_device structure
 *
 * This is called in response to ifconfig up
 *
 * Return: 0 for success; non-zero for failure
 */
int hdd_mon_open(struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_mon_open(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_open() - HDD Open function
 * @dev:	Pointer to net_device structure
 *
 * This is called in response to ifconfig up
 *
 * Return: 0 for success; non-zero for failure
 */
static int __hdd_open(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;

	ENTER_DEV(dev);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD, TRACE_CODE_HDD_OPEN_REQUEST,
			 adapter->sessionId, adapter->device_mode));

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret) {
		hddLog(LOGE, FL("HDD context is not valid"));
		return ret;
	}

	set_bit(DEVICE_IFACE_OPENED, &adapter->event_flags);
	if (hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hddLog(LOG1, FL("Enabling Tx Queues"));
		/* Enable TX queues only when we are connected */
		wlan_hdd_netif_queue_control(adapter,
					WLAN_START_ALL_NETIF_QUEUE,
					WLAN_CONTROL_PATH);
	}

	return ret;
}

/**
 * hdd_open() - Wrapper function for __hdd_open to protect it from SSR
 * @dev:	Pointer to net_device structure
 *
 * This is called in response to ifconfig up
 *
 * Return: 0 for success; non-zero for failure
 */
int hdd_open(struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_open(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_stop() - HDD stop function
 * @dev:	Pointer to net_device structure
 *
 * This is called in response to ifconfig down
 *
 * Return: 0 for success; non-zero for failure
 */
static int __hdd_stop(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;

	ENTER_DEV(dev);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD, TRACE_CODE_HDD_STOP_REQUEST,
			 adapter->sessionId, adapter->device_mode));

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* Nothing to be done if the interface is not opened */
	if (false == test_bit(DEVICE_IFACE_OPENED, &adapter->event_flags)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("NETDEV Interface is not OPENED"));
		return -ENODEV;
	}

	/* Make sure the interface is marked as closed */
	clear_bit(DEVICE_IFACE_OPENED, &adapter->event_flags);
	hddLog(QDF_TRACE_LEVEL_INFO, FL("Disabling OS Tx queues"));

	/*
	 * Disable TX on the interface, after this hard_start_xmit() will not
	 * be called on that interface
	 */
	hddLog(LOG1, FL("Disabling queues"));
	wlan_hdd_netif_queue_control(adapter, WLAN_NETIF_TX_DISABLE_N_CARRIER,
				   WLAN_CONTROL_PATH);

	/*
	 * The interface is marked as down for outside world (aka kernel)
	 * But the driver is pretty much alive inside. The driver needs to
	 * tear down the existing connection on the netdev (session)
	 * cleanup the data pipes and wait until the control plane is stabilized
	 * for this interface. The call also needs to wait until the above
	 * mentioned actions are completed before returning to the caller.
	 * Notice that the hdd_stop_adapter is requested not to close the session
	 * That is intentional to be able to scan if it is a STA/P2P interface
	 */
	hdd_stop_adapter(hdd_ctx, adapter, false);

	/* DeInit the adapter. This ensures datapath cleanup as well */
	hdd_deinit_adapter(hdd_ctx, adapter, true);

	EXIT();
	return 0;
}

/**
 * hdd_stop() - Wrapper function for __hdd_stop to protect it from SSR
 * @dev: pointer to net_device structure
 *
 * This is called in response to ifconfig down
 *
 * Return: 0 for success and error number for failure
 */
int hdd_stop(struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_stop(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_uninit() - HDD uninit function
 * @dev:	Pointer to net_device structure
 *
 * This is called during the netdev unregister to uninitialize all data
 * associated with the device
 *
 * Return: None
 */
static void __hdd_uninit(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER_DEV(dev);

	do {
		if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
			hddLog(LOGP, FL("Invalid magic"));
			break;
		}

		if (NULL == adapter->pHddCtx) {
			hddLog(LOGP, FL("NULL hdd_ctx"));
			break;
		}

		if (dev != adapter->dev) {
			hddLog(LOGP, FL("Invalid device reference"));
			/*
			 * we haven't validated all cases so let this go for
			 * now
			 */
		}

		hdd_deinit_adapter(adapter->pHddCtx, adapter, true);

		/* after uninit our adapter structure will no longer be valid */
		adapter->dev = NULL;
		adapter->magic = 0;
	} while (0);

	EXIT();
}

/**
 * hdd_uninit() - Wrapper function to protect __hdd_uninit from SSR
 * @dev: pointer to net_device structure
 *
 * This is called during the netdev unregister to uninitialize all data
 * associated with the device
 *
 * Return: none
 */
static void hdd_uninit(struct net_device *dev)
{
	cds_ssr_protect(__func__);
	__hdd_uninit(dev);
	cds_ssr_unprotect(__func__);
}

static int hdd_open_cesium_nl_sock(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	struct netlink_kernel_cfg cfg = {
		.groups = WLAN_NLINK_MCAST_GRP_ID,
		.input = NULL
	};
#endif
	int ret = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	cesium_nl_srv_sock = netlink_kernel_create(&init_net, WLAN_NLINK_CESIUM,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
						   THIS_MODULE,
#endif
						   &cfg);
#else
	cesium_nl_srv_sock = netlink_kernel_create(&init_net, WLAN_NLINK_CESIUM,
						   WLAN_NLINK_MCAST_GRP_ID,
						   NULL, NULL, THIS_MODULE);
#endif

	if (cesium_nl_srv_sock == NULL) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("NLINK:  cesium netlink_kernel_create failed"));
		ret = -ECONNREFUSED;
	}

	return ret;
}

static void hdd_close_cesium_nl_sock(void)
{
	if (NULL != cesium_nl_srv_sock) {
		netlink_kernel_release(cesium_nl_srv_sock);
		cesium_nl_srv_sock = NULL;
	}
}

/**
 * __hdd_set_mac_address() - set the user specified mac address
 * @dev:	Pointer to the net device.
 * @addr:	Pointer to the sockaddr.
 *
 * This function sets the user specified mac address using
 * the command ifconfig wlanX hw ether <mac adress>.
 *
 * Return: 0 for success, non zero for failure
 */
static int __hdd_set_mac_address(struct net_device *dev, void *addr)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	struct sockaddr *psta_mac_addr = addr;
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	memcpy(&adapter->macAddressCurrent, psta_mac_addr->sa_data, ETH_ALEN);
	memcpy(dev->dev_addr, psta_mac_addr->sa_data, ETH_ALEN);

	EXIT();
	return qdf_ret_status;
}

/**
 * hdd_set_mac_address() - Wrapper function to protect __hdd_set_mac_address()
 *			function from SSR
 * @dev: pointer to net_device structure
 * @addr: Pointer to the sockaddr
 *
 * This function sets the user specified mac address using
 * the command ifconfig wlanX hw ether <mac adress>.
 *
 * Return: 0 for success.
 */
static int hdd_set_mac_address(struct net_device *dev, void *addr)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_set_mac_address(dev, addr);
	cds_ssr_unprotect(__func__);

	return ret;
}

uint8_t *wlan_hdd_get_intf_addr(hdd_context_t *hdd_ctx)
{
	int i;
	for (i = 0; i < QDF_MAX_CONCURRENCY_PERSONA; i++) {
		if (0 == ((hdd_ctx->config->intfAddrMask) & (1 << i)))
			break;
	}

	if (QDF_MAX_CONCURRENCY_PERSONA == i)
		return NULL;

	hdd_ctx->config->intfAddrMask |= (1 << i);
	return &hdd_ctx->config->intfMacAddr[i].bytes[0];
}

void wlan_hdd_release_intf_addr(hdd_context_t *hdd_ctx, uint8_t *releaseAddr)
{
	int i;
	for (i = 0; i < QDF_MAX_CONCURRENCY_PERSONA; i++) {
		if (!memcmp(releaseAddr,
			    &hdd_ctx->config->intfMacAddr[i].bytes[0],
			    6)) {
			hdd_ctx->config->intfAddrMask &= ~(1 << i);
			break;
		}
	}
	return;
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
/**
 * __hdd_set_multicast_list() - set the multicast address list
 * @dev:	Pointer to the WLAN device.
 * @skb:	Pointer to OS packet (sk_buff).
 *
 * This funciton sets the multicast address list.
 *
 * Return: None
 */
static void __hdd_set_multicast_list(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int mc_count;
	int i = 0, status;
	struct netdev_hw_addr *ha;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	static const uint8_t ipv6_router_solicitation[]
			= {0x33, 0x33, 0x00, 0x00, 0x00, 0x02};

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam())
		return;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return;

	if (dev->flags & IFF_ALLMULTI) {
		hddLog(QDF_TRACE_LEVEL_INFO,
		       FL("allow all multicast frames"));
		adapter->mc_addr_list.mc_cnt = 0;
	} else {
		mc_count = netdev_mc_count(dev);
		hddLog(QDF_TRACE_LEVEL_INFO,
		       FL("mc_count = %u"), mc_count);
		if (mc_count > WLAN_HDD_MAX_MC_ADDR_LIST) {
			hddLog(QDF_TRACE_LEVEL_INFO,
			       FL(
				  "No free filter available; allow all multicast frames"
				 ));
			adapter->mc_addr_list.mc_cnt = 0;
			return;
		}

		adapter->mc_addr_list.mc_cnt = mc_count;

		netdev_for_each_mc_addr(ha, dev) {
			if (i == mc_count)
				break;
			/*
			 * Skip following addresses:
			 * 1)IPv6 router solicitation address
			 * 2)Any other address pattern if its set during
			 *  RXFILTER REMOVE driver command based on
			 *  addr_filter_pattern
			 */
			if ((!memcmp(ha->addr, ipv6_router_solicitation,
			   ETH_ALEN)) ||
			   (adapter->addr_filter_pattern && (!memcmp(ha->addr,
			    &adapter->addr_filter_pattern, 1)))) {
				hdd_info("MC/BC filtering Skip addr ="MAC_ADDRESS_STR,
					MAC_ADDR_ARRAY(ha->addr));
				adapter->mc_addr_list.mc_cnt--;
				continue;
			}

			memset(&(adapter->mc_addr_list.addr[i][0]), 0,
			       ETH_ALEN);
			memcpy(&(adapter->mc_addr_list.addr[i][0]), ha->addr,
			       ETH_ALEN);
			hddLog(QDF_TRACE_LEVEL_INFO,
			       FL("mlist[%d] = " MAC_ADDRESS_STR), i,
			       MAC_ADDR_ARRAY(adapter->mc_addr_list.addr[i]));
			i++;
		}
	}
	if (hdd_ctx->config->active_mode_offload) {
		hdd_info("enable mc filtering");
		wlan_hdd_set_mc_addr_list(adapter, true);
	} else {
		hdd_info("skip mc filtering enable it during cfg80211 suspend");
	}
	EXIT();
	return;
}

/**
 * hdd_set_multicast_list() - SSR wrapper function for __hdd_set_multicast_list
 * @dev: pointer to net_device
 *
 * Return: none
 */
static void hdd_set_multicast_list(struct net_device *dev)
{
	cds_ssr_protect(__func__);
	__hdd_set_multicast_list(dev);
	cds_ssr_unprotect(__func__);
}
#endif

/**
 * hdd_select_queue() - used by Linux OS to decide which queue to use first
 * @dev:	Pointer to the WLAN device.
 * @skb:	Pointer to OS packet (sk_buff).
 *
 * This function is registered with the Linux OS for network
 * core to decide which queue to use first.
 *
 * Return: ac, Queue Index/access category corresponding to UP in IP header
 */
static uint16_t hdd_select_queue(struct net_device *dev, struct sk_buff *skb
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
			  , void *accel_priv
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
			  , select_queue_fallback_t fallback
#endif
)
{
	return hdd_wmm_select_queue(dev, skb);
}

static struct net_device_ops wlan_drv_ops = {
	.ndo_open = hdd_open,
	.ndo_stop = hdd_stop,
	.ndo_uninit = hdd_uninit,
	.ndo_start_xmit = hdd_hard_start_xmit,
	.ndo_tx_timeout = hdd_tx_timeout,
	.ndo_get_stats = hdd_get_stats,
	.ndo_do_ioctl = hdd_ioctl,
	.ndo_set_mac_address = hdd_set_mac_address,
	.ndo_select_queue = hdd_select_queue,
#ifdef WLAN_FEATURE_PACKET_FILTERING
	.ndo_set_rx_mode = hdd_set_multicast_list,
#endif
};

/* Monitor mode net_device_ops, doesnot Tx and most of operations. */
static struct net_device_ops wlan_mon_drv_ops = {
	.ndo_open = hdd_mon_open,
	.ndo_stop = hdd_stop,
	.ndo_get_stats = hdd_get_stats,
};

/**
 * hdd_set_station_ops() - update net_device ops for monitor mode
 * @pWlanDev: Handle to struct net_device to be updated.
 * Return: None
 */
void hdd_set_station_ops(struct net_device *pWlanDev)
{
	if (QDF_GLOBAL_MONITOR_MODE == cds_get_conparam())
		pWlanDev->netdev_ops = &wlan_mon_drv_ops;
	else
		pWlanDev->netdev_ops = &wlan_drv_ops;
}

/**
 * hdd_mon_mode_ether_setup() - Update monitor mode struct net_device.
 * @dev: Handle to struct net_device to be updated.
 *
 * Return: None
 */
static void hdd_mon_mode_ether_setup(struct net_device *dev)
{
	dev->header_ops         = NULL;
	dev->type               = ARPHRD_IEEE80211_RADIOTAP;
	dev->hard_header_len    = ETH_HLEN;
	dev->mtu                = ETH_DATA_LEN;
	dev->addr_len           = ETH_ALEN;
	dev->tx_queue_len       = 1000; /* Ethernet wants good queues */
	dev->flags              = IFF_BROADCAST|IFF_MULTICAST;
	dev->priv_flags        |= IFF_TX_SKB_SHARING;

	memset(dev->broadcast, 0xFF, ETH_ALEN);
}

/**
 * hdd_alloc_station_adapter() - allocate the station hdd adapter
 * @hdd_ctx: global hdd context
 * @macAddr: mac address to assign to the interface
 * @name: User-visible name of the interface
 *
 * hdd adapter pointer would point to the netdev->priv space, this function
 * would retrive the pointer, and setup the hdd adapter configuration.
 *
 * Return: the pointer to hdd adapter, otherwise NULL
 */
static hdd_adapter_t *hdd_alloc_station_adapter(hdd_context_t *hdd_ctx,
						tSirMacAddr macAddr,
						unsigned char name_assign_type,
						const char *name)
{
	struct net_device *pWlanDev = NULL;
	hdd_adapter_t *adapter = NULL;
	/*
	 * cfg80211 initialization and registration....
	 */
	pWlanDev = alloc_netdev_mq(sizeof(hdd_adapter_t), name,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)) || defined(WITH_BACKPORTS)
				   name_assign_type,
#endif
				(QDF_GLOBAL_MONITOR_MODE == cds_get_conparam() ?
				hdd_mon_mode_ether_setup : ether_setup),
				NUM_TX_QUEUES);

	if (pWlanDev != NULL) {

		/* Save the pointer to the net_device in the HDD adapter */
		adapter = (hdd_adapter_t *) netdev_priv(pWlanDev);

		qdf_mem_zero(adapter, sizeof(hdd_adapter_t));

		adapter->dev = pWlanDev;
		adapter->pHddCtx = hdd_ctx;
		adapter->magic = WLAN_HDD_ADAPTER_MAGIC;

		init_completion(&adapter->session_open_comp_var);
		init_completion(&adapter->session_close_comp_var);
		init_completion(&adapter->disconnect_comp_var);
		init_completion(&adapter->linkup_event_var);
		init_completion(&adapter->cancel_rem_on_chan_var);
		init_completion(&adapter->rem_on_chan_ready_event);
		init_completion(&adapter->sta_authorized_event);
		init_completion(&adapter->offchannel_tx_event);
		init_completion(&adapter->tx_action_cnf_event);
#ifdef FEATURE_WLAN_TDLS
		init_completion(&adapter->tdls_add_station_comp);
		init_completion(&adapter->tdls_del_station_comp);
		init_completion(&adapter->tdls_mgmt_comp);
		init_completion(&adapter->tdls_link_establish_req_comp);
#endif
		init_completion(&adapter->ibss_peer_info_comp);
		init_completion(&adapter->change_country_code);


		init_completion(&adapter->scan_info.abortscan_event_var);

		adapter->offloads_configured = false;
		adapter->isLinkUpSvcNeeded = false;
		adapter->higherDtimTransition = true;
		/* Init the net_device structure */
		strlcpy(pWlanDev->name, name, IFNAMSIZ);

		qdf_mem_copy(pWlanDev->dev_addr, (void *)macAddr,
			     sizeof(tSirMacAddr));
		qdf_mem_copy(adapter->macAddressCurrent.bytes, macAddr,
			     sizeof(tSirMacAddr));
		pWlanDev->watchdog_timeo = HDD_TX_TIMEOUT;

		if (hdd_ctx->config->enable_ip_tcp_udp_checksum_offload)
			pWlanDev->features |=
				NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
		pWlanDev->features |= NETIF_F_RXCSUM;

		hdd_set_tso_flags(hdd_ctx, pWlanDev);

		hdd_set_station_ops(adapter->dev);

		pWlanDev->destructor = free_netdev;
		pWlanDev->ieee80211_ptr = &adapter->wdev;
		pWlanDev->tx_queue_len = HDD_NETDEV_TX_QUEUE_LEN;
		adapter->wdev.wiphy = hdd_ctx->wiphy;
		adapter->wdev.netdev = pWlanDev;
		/* set pWlanDev's parent to underlying device */
		SET_NETDEV_DEV(pWlanDev, hdd_ctx->parent_dev);
		hdd_wmm_init(adapter);
		spin_lock_init(&adapter->pause_map_lock);
		adapter->start_time = adapter->last_time = qdf_system_ticks();
	}

	return adapter;
}

QDF_STATUS hdd_register_interface(hdd_adapter_t *adapter,
				  bool rtnl_held)
{
	struct net_device *pWlanDev = adapter->dev;
	/* hdd_station_ctx_t *pHddStaCtx = &adapter->sessionCtx.station; */
	/* hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX( adapter ); */
	/* QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS; */

	if (rtnl_held) {
		if (strnchr(pWlanDev->name, strlen(pWlanDev->name), '%')) {
			if (dev_alloc_name(pWlanDev, pWlanDev->name) < 0) {
				hddLog(QDF_TRACE_LEVEL_ERROR,
				       FL("Failed:dev_alloc_name"));
				return QDF_STATUS_E_FAILURE;
			}
		}
		if (register_netdevice(pWlanDev)) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("Failed:register_netdev"));
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		if (register_netdev(pWlanDev)) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("Failed:register_netdev"));
			return QDF_STATUS_E_FAILURE;
		}
	}
	set_bit(NET_DEVICE_REGISTERED, &adapter->event_flags);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS hdd_sme_close_session_callback(void *pContext)
{
	hdd_adapter_t *adapter = pContext;

	if (NULL == adapter) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("NULL adapter"));
		return QDF_STATUS_E_INVAL;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("Invalid magic"));
		return QDF_STATUS_NOT_INITIALIZED;
	}

	clear_bit(SME_SESSION_OPENED, &adapter->event_flags);

#if !defined (CONFIG_CNSS) && \
	!defined (WLAN_OPEN_SOURCE)
	/*
	 * need to make sure all of our scheduled work has completed.
	 * This callback is called from MC thread context, so it is safe to
	 * to call below flush workqueue API from here.
	 *
	 * Even though this is called from MC thread context, if there is a faulty
	 * work item in the system, that can hang this call forever.  So flushing
	 * this global work queue is not safe; and now we make sure that
	 * individual work queues are stopped correctly. But the cancel work queue
	 * is a GPL only API, so the proprietary  version of the driver would still
	 * rely on the global work queue flush.
	 */
	flush_scheduled_work();
#endif

	/*
	 * We can be blocked while waiting for scheduled work to be
	 * flushed, and the adapter structure can potentially be freed, in
	 * which case the magic will have been reset.  So make sure the
	 * magic is still good, and hence the adapter structure is still
	 * valid, before signaling completion
	 */
	if (WLAN_HDD_ADAPTER_MAGIC == adapter->magic)
		complete(&adapter->session_close_comp_var);

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_check_and_init_tdls() - check and init TDLS operation for desired mode
 * @adapter: pointer to device adapter
 * @type: type of interface
 *
 * This routine will check the mode of adapter and if it is required then it
 * will initialize the TDLS operations
 *
 * Return: QDF_STATUS
 */
#ifdef FEATURE_WLAN_TDLS
static QDF_STATUS hdd_check_and_init_tdls(hdd_adapter_t *adapter, uint32_t type)
{
	if (QDF_IBSS_MODE != type) {
		if (0 != wlan_hdd_tdls_init(adapter)) {
			hddLog(LOGE, FL("wlan_hdd_tdls_init failed"));
			return QDF_STATUS_E_FAILURE;
		}
		set_bit(TDLS_INIT_DONE, &adapter->event_flags);
	}
	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS hdd_check_and_init_tdls(hdd_adapter_t *adapter, uint32_t type)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS hdd_init_station_mode(hdd_adapter_t *adapter)
{
	struct net_device *pWlanDev = adapter->dev;
	hdd_station_ctx_t *pHddStaCtx = &adapter->sessionCtx.station;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t type, subType;
	unsigned long rc;
	int ret_val;

	INIT_COMPLETION(adapter->session_open_comp_var);
	sme_set_curr_device_mode(hdd_ctx->hHal, adapter->device_mode);
	status = cds_get_vdev_types(adapter->device_mode, &type, &subType);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(LOGE, FL("failed to get vdev type"));
		goto error_sme_open;
	}
	/* Open a SME session for future operation */
	qdf_ret_status =
		sme_open_session(hdd_ctx->hHal, hdd_sme_roam_callback, adapter,
				 (uint8_t *) &adapter->macAddressCurrent,
				 &adapter->sessionId, type, subType);
	if (!QDF_IS_STATUS_SUCCESS(qdf_ret_status)) {
		hddLog(LOGP,
		       FL("sme_open_session() failed, status code %08d [x%08x]"),
		       qdf_ret_status, qdf_ret_status);
		status = QDF_STATUS_E_FAILURE;
		goto error_sme_open;
	}
	/* Block on a completion variable. Can't wait forever though. */
	rc = wait_for_completion_timeout(
		&adapter->session_open_comp_var,
		msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
	if (!rc) {
		hddLog(LOGP,
			FL("Session is not opened within timeout period code %ld"),
			rc);
		status = QDF_STATUS_E_FAILURE;
		goto error_sme_open;
	}

	/* Register wireless extensions */
	qdf_ret_status = hdd_register_wext(pWlanDev);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		hddLog(LOGP,
		       FL("hdd_register_wext() failed, status code %08d [x%08x]"),
		       qdf_ret_status, qdf_ret_status);
		status = QDF_STATUS_E_FAILURE;
		goto error_register_wext;
	}
	/* Set the Connection State to Not Connected */
	hddLog(LOG1,
	       FL("Set HDD connState to eConnectionState_NotConnected"));
	pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;

	/* Set the default operation channel */
	pHddStaCtx->conn_info.operationChannel =
		hdd_ctx->config->OperatingChannel;

	/* Make the default Auth Type as OPEN */
	pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	status = hdd_init_tx_rx(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(LOGP,
		       FL("hdd_init_tx_rx() failed, status code %08d [x%08x]"),
		       status, status);
		goto error_init_txrx;
	}

	set_bit(INIT_TX_RX_SUCCESS, &adapter->event_flags);

	status = hdd_wmm_adapter_init(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(LOGP,
		       FL("hdd_wmm_adapter_init() failed, status code %08d [x%08x]"),
		       status, status);
		goto error_wmm_init;
	}

	set_bit(WMM_INIT_DONE, &adapter->event_flags);

	ret_val = wma_cli_set_command(adapter->sessionId,
				      WMI_PDEV_PARAM_BURST_ENABLE,
				      hdd_ctx->config->enableSifsBurst,
				      PDEV_CMD);

	if (0 != ret_val) {
		hddLog(LOGE,
		       FL("WMI_PDEV_PARAM_BURST_ENABLE set failed %d"),
		       ret_val);
	}
	status = hdd_check_and_init_tdls(adapter, type);
	if (status != QDF_STATUS_SUCCESS)
		goto error_tdls_init;

	return QDF_STATUS_SUCCESS;

#ifdef FEATURE_WLAN_TDLS
error_tdls_init:
	clear_bit(WMM_INIT_DONE, &adapter->event_flags);
	hdd_wmm_adapter_close(adapter);
#endif
error_wmm_init:
	clear_bit(INIT_TX_RX_SUCCESS, &adapter->event_flags);
	hdd_deinit_tx_rx(adapter);
error_init_txrx:
	hdd_unregister_wext(pWlanDev);
error_register_wext:
	if (test_bit(SME_SESSION_OPENED, &adapter->event_flags)) {
		INIT_COMPLETION(adapter->session_close_comp_var);
		if (QDF_STATUS_SUCCESS == sme_close_session(hdd_ctx->hHal,
							    adapter->sessionId,
							    hdd_sme_close_session_callback,
							    adapter)) {
			unsigned long rc;

			/*
			 * Block on a completion variable.
			 * Can't wait forever though.
			 */
			rc = wait_for_completion_timeout(
				&adapter->session_close_comp_var,
				msecs_to_jiffies
					(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
			if (rc <= 0)
				hddLog(LOGE,
				       FL("Session is not opened within timeout period code %ld"),
				       rc);
		}
	}
error_sme_open:
	return status;
}

void hdd_cleanup_actionframe(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter)
{
	hdd_cfg80211_state_t *cfgState;

	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(adapter);

	if (NULL != cfgState->buf) {
		unsigned long rc;
		rc = wait_for_completion_timeout(
			&adapter->tx_action_cnf_event,
			msecs_to_jiffies(ACTION_FRAME_TX_TIMEOUT));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("HDD Wait for Action Confirmation Failed!!"));
			/*
			 * Inform tx status as FAILURE to upper layer and free
			 * cfgState->buf
			 */
			 hdd_send_action_cnf(adapter, false);
		}
	}
	return;
}

void hdd_deinit_adapter(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter,
			bool rtnl_held)
{
	ENTER();
	switch (adapter->device_mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_DEVICE_MODE:
	{
		if (test_bit
			    (INIT_TX_RX_SUCCESS, &adapter->event_flags)) {
			hdd_deinit_tx_rx(adapter);
			clear_bit(INIT_TX_RX_SUCCESS,
				  &adapter->event_flags);
		}

		if (test_bit(WMM_INIT_DONE, &adapter->event_flags)) {
			hdd_wmm_adapter_close(adapter);
			clear_bit(WMM_INIT_DONE,
				  &adapter->event_flags);
		}

		hdd_cleanup_actionframe(hdd_ctx, adapter);
		wlan_hdd_tdls_exit(adapter);
		break;
	}

	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
	{

		if (test_bit(WMM_INIT_DONE, &adapter->event_flags)) {
			hdd_wmm_adapter_close(adapter);
			clear_bit(WMM_INIT_DONE,
				  &adapter->event_flags);
		}

		hdd_cleanup_actionframe(hdd_ctx, adapter);

		hdd_unregister_hostapd(adapter, rtnl_held);

		break;
	}

	default:
		break;
	}

	EXIT();
}

void hdd_cleanup_adapter(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter,
			 bool rtnl_held)
{
	struct net_device *pWlanDev = NULL;

	if (adapter)
		pWlanDev = adapter->dev;
	else {
		hddLog(LOGE, FL("adapter is Null"));
		return;
	}

	hdd_lro_disable(hdd_ctx, adapter);
	hdd_debugfs_exit(adapter);
	/*
	 * The adapter is marked as closed. When hdd_wlan_exit() call returns,
	 * the driver is almost closed and cannot handle either control
	 * messages or data. However, unregister_netdevice() call above will
	 * eventually invoke hdd_stop (ndo_close) driver callback, which attempts
	 * to close the active connections (basically excites control path) which
	 * is not right. Setting this flag helps hdd_stop() to recognize that
	 * the interface is closed and restricts any operations on that
	 */
	clear_bit(DEVICE_IFACE_OPENED, &adapter->event_flags);

	if (test_bit(NET_DEVICE_REGISTERED, &adapter->event_flags)) {
		if (rtnl_held) {
			unregister_netdevice(pWlanDev);
		} else {
			unregister_netdev(pWlanDev);
		}
		/*
		 * Note that the adapter is no longer valid at this point
		 * since the memory has been reclaimed
		 */
	}
}

QDF_STATUS hdd_check_for_existing_macaddr(hdd_context_t *hdd_ctx,
					  tSirMacAddr macAddr)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *adapter;
	QDF_STATUS status;
	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);
	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		if (adapter
		    && !qdf_mem_cmp(adapter->macAddressCurrent.bytes,
				       macAddr, sizeof(tSirMacAddr))) {
			return QDF_STATUS_E_FAILURE;
		}
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_open_adapter() - open and setup the hdd adatper
 * @hdd_ctx: global hdd context
 * @session_type: type of the interface to be created
 * @iface_name: User-visible name of the interface
 * @macAddr: MAC address to assign to the interface
 * @name_assign_type: the name of assign type of the netdev
 * @rtnl_held: the rtnl lock hold flag
 *
 * This function open and setup the hdd adpater according to the device
 * type request, assign the name, the mac address assigned, and then prepared
 * the hdd related parameters, queue, lock and ready to start.
 *
 * Return: the pointer of hdd adapter, otherwise NULL.
 */
hdd_adapter_t *hdd_open_adapter(hdd_context_t *hdd_ctx, uint8_t session_type,
				const char *iface_name, tSirMacAddr macAddr,
				unsigned char name_assign_type,
				bool rtnl_held)
{
	hdd_adapter_t *adapter = NULL;
	hdd_adapter_list_node_t *pHddAdapterNode = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	hdd_cfg80211_state_t *cfgState;
	int ret;

	hddLog(LOG2, FL("iface(%s) type(%d)"), iface_name, session_type);

	if (hdd_ctx->current_intf_count >= hdd_ctx->max_intf_count) {
		/*
		 * Max limit reached on the number of vdevs configured by the
		 * host. Return error
		 */
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL(
			  "Unable to add virtual intf: currentVdevCnt=%d,hostConfiguredVdevCnt=%d"
			 ),
		       hdd_ctx->current_intf_count, hdd_ctx->max_intf_count);
		return NULL;
	}

	if (macAddr == NULL) {
		/* Not received valid macAddr */
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL(
			  "Unable to add virtual intf: Not able to get valid mac address"
			 ));
		return NULL;
	}
	status = hdd_check_for_existing_macaddr(hdd_ctx, macAddr);
	if (QDF_STATUS_E_FAILURE == status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "Duplicate MAC addr: " MAC_ADDRESS_STR
		       " already exists",
		       MAC_ADDR_ARRAY(macAddr));
		return NULL;
	}

	switch (session_type) {
	case QDF_STA_MODE:
		/* Reset locally administered bit if the device mode is STA */
		WLAN_HDD_RESET_LOCALLY_ADMINISTERED_BIT(macAddr);
	/* fall through */
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_DEVICE_MODE:
	case QDF_OCB_MODE:
	case QDF_MONITOR_MODE:
	{
		adapter = hdd_alloc_station_adapter(hdd_ctx, macAddr,
						    name_assign_type,
						    iface_name);

		if (NULL == adapter) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("failed to allocate adapter for session %d"),
			       session_type);
			return NULL;
		}

		if (QDF_P2P_CLIENT_MODE == session_type)
			adapter->wdev.iftype = NL80211_IFTYPE_P2P_CLIENT;
		else if (QDF_P2P_DEVICE_MODE == session_type)
			adapter->wdev.iftype = NL80211_IFTYPE_P2P_DEVICE;
		else if (QDF_MONITOR_MODE == session_type)
			adapter->wdev.iftype = NL80211_IFTYPE_MONITOR;
		else
			adapter->wdev.iftype = NL80211_IFTYPE_STATION;

		adapter->device_mode = session_type;

		status = hdd_init_station_mode(adapter);
		if (QDF_STATUS_SUCCESS != status)
			goto err_free_netdev;

		hdd_lro_enable(hdd_ctx, adapter);

		/*
		 * Workqueue which gets scheduled in IPv4 notification
		 * callback
		 */
		INIT_WORK(&adapter->ipv4NotifierWorkQueue,
			  hdd_ipv4_notifier_work_queue);

#ifdef WLAN_NS_OFFLOAD
		/*
		 * Workqueue which gets scheduled in IPv6
		 * notification callback.
		 */
		INIT_WORK(&adapter->ipv6NotifierWorkQueue,
			  hdd_ipv6_notifier_work_queue);
#endif
		status = hdd_register_interface(adapter, rtnl_held);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_deinit_adapter(hdd_ctx, adapter, rtnl_held);
			goto err_lro_cleanup;
		}

		/* Stop the Interface TX queue. */
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_NETIF_TX_DISABLE_N_CARRIER,
					   WLAN_CONTROL_PATH);

		hdd_register_tx_flow_control(adapter,
				hdd_tx_resume_timer_expired_handler,
				hdd_tx_resume_cb);

		break;
	}

	case QDF_P2P_GO_MODE:
	case QDF_SAP_MODE:
	{
		adapter = hdd_wlan_create_ap_dev(hdd_ctx, macAddr,
						 name_assign_type,
						 (uint8_t *) iface_name);
		if (NULL == adapter) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("failed to allocate adapter for session %d"),
			       session_type);
			return NULL;
		}

		adapter->wdev.iftype =
			(session_type ==
			 QDF_SAP_MODE) ? NL80211_IFTYPE_AP :
			NL80211_IFTYPE_P2P_GO;
		adapter->device_mode = session_type;

		status = hdd_init_ap_mode(adapter);
		if (QDF_STATUS_SUCCESS != status)
			goto err_free_netdev;

		status = hdd_register_hostapd(adapter, rtnl_held);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_deinit_adapter(hdd_ctx, adapter, rtnl_held);
			goto err_free_netdev;
		}

		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_NETIF_TX_DISABLE_N_CARRIER,
					   WLAN_CONTROL_PATH);

		break;
	}
	case QDF_FTM_MODE:
	{
		adapter = hdd_alloc_station_adapter(hdd_ctx, macAddr,
						    name_assign_type,
						    iface_name);

		if (NULL == adapter) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("failed to allocate adapter for session %d"),
			       session_type);
			return NULL;
		}

		/*
		 * Assign NL80211_IFTYPE_STATION as interface type to resolve
		 * Kernel Warning message while loading driver in FTM mode.
		 */
		adapter->wdev.iftype = NL80211_IFTYPE_STATION;
		adapter->device_mode = session_type;
		status = hdd_register_interface(adapter, rtnl_held);

		hdd_init_tx_rx(adapter);

		/* Stop the Interface TX queue. */
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_NETIF_TX_DISABLE_N_CARRIER,
					   WLAN_CONTROL_PATH);
	}
	break;
	default:
	{
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("Invalid session type %d"),
		       session_type);
		QDF_ASSERT(0);
		return NULL;
	}
	}

	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	mutex_init(&cfgState->remain_on_chan_ctx_lock);

	if (QDF_STATUS_SUCCESS == status) {
		/* Add it to the hdd's session list. */
		pHddAdapterNode =
			qdf_mem_malloc(sizeof(hdd_adapter_list_node_t));
		if (NULL == pHddAdapterNode) {
			status = QDF_STATUS_E_NOMEM;
		} else {
			pHddAdapterNode->pAdapter = adapter;
			status = hdd_add_adapter_back(hdd_ctx, pHddAdapterNode);
		}
	}

	if (QDF_STATUS_SUCCESS != status) {
		if (NULL != adapter) {
			hdd_cleanup_adapter(hdd_ctx, adapter, rtnl_held);
			adapter = NULL;
		}
		if (NULL != pHddAdapterNode) {
			qdf_mem_free(pHddAdapterNode);
		}
		return NULL;
	}

	if (QDF_STATUS_SUCCESS == status) {
		cds_set_concurrency_mode(session_type);

		/* Initialize the WoWL service */
		if (!hdd_init_wowl(adapter)) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL("hdd_init_wowl failed"));
			goto err_lro_cleanup;
		}

		/* Adapter successfully added. Increment the vdev count */
		hdd_ctx->current_intf_count++;

		hddLog(QDF_TRACE_LEVEL_DEBUG, FL("current_intf_count=%d"),
		       hdd_ctx->current_intf_count);

		cds_check_and_restart_sap_with_non_dfs_acs();
	}

	if ((cds_get_conparam() != QDF_GLOBAL_FTM_MODE)
	    && (!hdd_ctx->config->enable2x2)) {
#define HDD_DTIM_1CHAIN_RX_ID 0x5
#define HDD_SMPS_PARAM_VALUE_S 29

		/*
		 * Disable DTIM 1 chain Rx when in 1x1, we are passing two value
		 * as param_id << 29 | param_value.
		 * Below param_value = 0(disable)
		 */
		ret = wma_cli_set_command(adapter->sessionId,
					  WMI_STA_SMPS_PARAM_CMDID,
					  HDD_DTIM_1CHAIN_RX_ID <<
						HDD_SMPS_PARAM_VALUE_S,
					  VDEV_CMD);

		if (ret != 0) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("DTIM 1 chain set failed %d"), ret);
			goto err_lro_cleanup;
		}

		ret = wma_cli_set_command(adapter->sessionId,
					  WMI_PDEV_PARAM_TX_CHAIN_MASK,
					  hdd_ctx->config->txchainmask1x1,
					  PDEV_CMD);
		if (ret != 0) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("WMI_PDEV_PARAM_TX_CHAIN_MASK set failed %d"),
			       ret);
			goto err_lro_cleanup;
		}
		ret = wma_cli_set_command(adapter->sessionId,
					  WMI_PDEV_PARAM_RX_CHAIN_MASK,
					  hdd_ctx->config->rxchainmask1x1,
					  PDEV_CMD);
		if (ret != 0) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("WMI_PDEV_PARAM_RX_CHAIN_MASK set failed %d"),
			       ret);
			goto err_lro_cleanup;
		}
#undef HDD_DTIM_1CHAIN_RX_ID
#undef HDD_SMPS_PARAM_VALUE_S
	}

	if (QDF_GLOBAL_FTM_MODE != cds_get_conparam()) {
		ret = wma_cli_set_command(adapter->sessionId,
					  WMI_PDEV_PARAM_HYST_EN,
					  hdd_ctx->config->enableMemDeepSleep,
					  PDEV_CMD);

		if (ret != 0) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("WMI_PDEV_PARAM_HYST_EN set failed %d"),
			       ret);
			goto err_lro_cleanup;
		}
	}

#ifdef CONFIG_FW_LOGS_BASED_ON_INI

	/* Enable FW logs based on INI configuration */
	if ((QDF_GLOBAL_FTM_MODE != cds_get_conparam()) &&
	    (hdd_ctx->config->enable_fw_log)) {
		uint8_t count = 0;
		uint32_t value = 0;
		uint8_t numEntries = 0;
		uint8_t moduleLoglevel[FW_MODULE_LOG_LEVEL_STRING_LENGTH];

		hdd_ctx->fw_log_settings.dl_type =
					hdd_ctx->config->enableFwLogType;
		ret = wma_cli_set_command(adapter->sessionId,
					  WMI_DBGLOG_TYPE,
					  hdd_ctx->config->enableFwLogType,
					  DBG_CMD);
		if (ret != 0) {
			hddLog(LOGE, FL("Failed to enable FW log type ret %d"),
			       ret);
		}

		hdd_ctx->fw_log_settings.dl_loglevel =
					hdd_ctx->config->enableFwLogLevel;
		ret = wma_cli_set_command(adapter->sessionId,
					  WMI_DBGLOG_LOG_LEVEL,
					  hdd_ctx->config->enableFwLogLevel,
					  DBG_CMD);
		if (ret != 0) {
			hddLog(LOGE, FL("Failed to enable FW log level ret %d"),
			       ret);
		}

		hdd_string_to_u8_array(hdd_ctx->config->enableFwModuleLogLevel,
				       moduleLoglevel,
				       &numEntries,
				       FW_MODULE_LOG_LEVEL_STRING_LENGTH);
		while (count < numEntries) {
			/*
			 * FW module log level input string looks like below:
			 * gFwDebugModuleLoglevel=<FW Module ID>,<Log Level>,...
			 * For example:
			 * gFwDebugModuleLoglevel=1,0,2,1,3,2,4,3,5,4,6,5,7,6
			 * Above input string means :
			 * For FW module ID 1 enable log level 0
			 * For FW module ID 2 enable log level 1
			 * For FW module ID 3 enable log level 2
			 * For FW module ID 4 enable log level 3
			 * For FW module ID 5 enable log level 4
			 * For FW module ID 6 enable log level 5
			 * For FW module ID 7 enable log level 6
			 */

			/* FW expects WMI command value =
			 *               Module ID * 10 + Module Log level
			 */
			value =	((moduleLoglevel[count] * 10) +
				 moduleLoglevel[count + 1]);
			ret = wma_cli_set_command(adapter->sessionId,
						  WMI_DBGLOG_MOD_LOG_LEVEL,
						  value, DBG_CMD);
			if (ret != 0) {
				hddLog(LOGE,
				       FL
					       ("Failed to enable FW module log level %d ret %d"),
				       value, ret);
			}

			count += 2;
		}
	}
#endif
	if (QDF_STATUS_SUCCESS != hdd_debugfs_init(adapter))
		hdd_err("Interface %s wow debug_fs init failed", iface_name);

	return adapter;

err_lro_cleanup:
	hdd_lro_disable(hdd_ctx, adapter);
err_free_netdev:
	free_netdev(adapter->dev);
	wlan_hdd_release_intf_addr(hdd_ctx, adapter->macAddressCurrent.bytes);

	return NULL;
}

QDF_STATUS hdd_close_adapter(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter,
			     bool rtnl_held)
{
	hdd_adapter_list_node_t *adapterNode, *pCurrent, *pNext;
	QDF_STATUS status;

	status = hdd_get_front_adapter(hdd_ctx, &pCurrent);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(QDF_TRACE_LEVEL_WARN, FL("adapter list empty %d"),
		       status);
		return status;
	}

	while (pCurrent->pAdapter != adapter) {
		status = hdd_get_next_adapter(hdd_ctx, pCurrent, &pNext);
		if (QDF_STATUS_SUCCESS != status)
			break;

		pCurrent = pNext;
	}
	adapterNode = pCurrent;
	if (QDF_STATUS_SUCCESS == status) {
		cds_clear_concurrency_mode(adapter->device_mode);
		hdd_cleanup_adapter(hdd_ctx, adapterNode->pAdapter, rtnl_held);

		hdd_remove_adapter(hdd_ctx, adapterNode);
		qdf_mem_free(adapterNode);
		adapterNode = NULL;

		/* Adapter removed. Decrement vdev count */
		if (hdd_ctx->current_intf_count != 0)
			hdd_ctx->current_intf_count--;

		/* Fw will take care incase of concurrency */
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * hdd_close_all_adapters - Close all open adapters
 * @hdd_ctx:	Hdd context
 * rtnl_held:	True if RTNL lock held
 *
 * Close all open adapters.
 *
 * Return: QDF status code
 */
QDF_STATUS hdd_close_all_adapters(hdd_context_t *hdd_ctx, bool rtnl_held)
{
	hdd_adapter_list_node_t *pHddAdapterNode;
	QDF_STATUS status;

	ENTER();

	do {
		status = hdd_remove_front_adapter(hdd_ctx, &pHddAdapterNode);
		if (pHddAdapterNode && QDF_STATUS_SUCCESS == status) {
			hdd_cleanup_adapter(hdd_ctx, pHddAdapterNode->pAdapter,
					    rtnl_held);
			qdf_mem_free(pHddAdapterNode);
		}
	} while (NULL != pHddAdapterNode && QDF_STATUS_E_EMPTY != status);

	EXIT();

	return QDF_STATUS_SUCCESS;
}

void wlan_hdd_reset_prob_rspies(hdd_adapter_t *pHostapdAdapter)
{
	struct qdf_mac_addr *bssid = NULL;
	tSirUpdateIE updateIE;
	switch (pHostapdAdapter->device_mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	{
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pHostapdAdapter);
		bssid = &pHddStaCtx->conn_info.bssId;
		break;
	}
	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
	case QDF_IBSS_MODE:
	{
		bssid = &pHostapdAdapter->macAddressCurrent;
		break;
	}
	case QDF_FTM_MODE:
	case QDF_P2P_DEVICE_MODE:
	default:
		/*
		 * wlan_hdd_reset_prob_rspies should not have been called
		 * for these kind of devices
		 */
		hddLog(LOGE,
		       FL("Unexpected request for the current device type %d"),
		       pHostapdAdapter->device_mode);
		return;
	}

	qdf_copy_macaddr(&updateIE.bssid, bssid);
	updateIE.smeSessionId = pHostapdAdapter->sessionId;
	updateIE.ieBufferlength = 0;
	updateIE.pAdditionIEBuffer = NULL;
	updateIE.append = true;
	updateIE.notify = false;
	if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(pHostapdAdapter),
			      &updateIE,
			      eUPDATE_IE_PROBE_RESP) == QDF_STATUS_E_FAILURE) {
		hddLog(LOGE, FL("Could not pass on PROBE_RSP_BCN data to PE"));
	}
}

/**
 * hdd_wait_for_sme_close_sesion() - Close and wait for SME session close
 * @hdd_ctx: HDD context which is already NULL validated
 * @adapter: HDD adapter which is already NULL validated
 *
 * Close the SME session and wait for its completion, if needed.
 *
 * Return: None
 */
static void hdd_wait_for_sme_close_sesion(hdd_context_t *hdd_ctx,
					hdd_adapter_t *adapter)
{
	unsigned long rc;

	if (!test_bit(SME_SESSION_OPENED, &adapter->event_flags)) {
		hdd_err("session is not opened:%d", adapter->sessionId);
		return;
	}

	INIT_COMPLETION(adapter->session_close_comp_var);
	if (QDF_STATUS_SUCCESS ==
			sme_close_session(hdd_ctx->hHal, adapter->sessionId,
				hdd_sme_close_session_callback,
				adapter)) {
		/*
		 * Block on a completion variable. Can't wait
		 * forever though.
		 */
		rc = wait_for_completion_timeout(
				&adapter->session_close_comp_var,
				msecs_to_jiffies
				(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
		if (!rc)
			hdd_err("failure waiting for session_close_comp_var");
	}
}

QDF_STATUS hdd_stop_adapter(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter,
			    const bool bCloseSession)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	union iwreq_data wrqu;
	tSirUpdateIE updateIE;
	unsigned long rc;

	ENTER();

	hddLog(LOG1, FL("Disabling queues"));
	wlan_hdd_netif_queue_control(adapter, WLAN_NETIF_TX_DISABLE_N_CARRIER,
				   WLAN_CONTROL_PATH);
	switch (adapter->device_mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_IBSS_MODE:
	case QDF_P2P_DEVICE_MODE:
		if (hdd_conn_is_connected(
				WLAN_HDD_GET_STATION_CTX_PTR(adapter)) ||
			hdd_is_connecting(
				WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
			if (pWextState->roamProfile.BSSType ==
			    eCSR_BSS_TYPE_START_IBSS)
				qdf_ret_status =
					sme_roam_disconnect(hdd_ctx->hHal,
							    adapter->sessionId,
							    eCSR_DISCONNECT_REASON_IBSS_LEAVE);
			else
				qdf_ret_status =
					sme_roam_disconnect(hdd_ctx->hHal,
							    adapter->sessionId,
							    eCSR_DISCONNECT_REASON_UNSPECIFIED);
			/* success implies disconnect command got queued up successfully */
			if (qdf_ret_status == QDF_STATUS_SUCCESS) {
				rc = wait_for_completion_timeout(
					&adapter->disconnect_comp_var,
					msecs_to_jiffies
						(WLAN_WAIT_TIME_DISCONNECT));
				if (!rc) {
					hddLog(QDF_TRACE_LEVEL_ERROR,
					       FL(
						  "wait on disconnect_comp_var failed"
						 ));
				}
			} else {
				hddLog(LOGE,
				       FL(
					  "failed to post disconnect event to SME"
					 ));
			}
			memset(&wrqu, '\0', sizeof(wrqu));
			wrqu.ap_addr.sa_family = ARPHRD_ETHER;
			memset(wrqu.ap_addr.sa_data, '\0', ETH_ALEN);
			wireless_send_event(adapter->dev, SIOCGIWAP, &wrqu,
					    NULL);
		} else {
			hdd_abort_mac_scan(hdd_ctx, adapter->sessionId,
					   eCSR_SCAN_ABORT_DEFAULT);
		}
		wlan_hdd_cleanup_remain_on_channel_ctx(adapter);

#ifdef WLAN_OPEN_SOURCE
		cancel_work_sync(&adapter->ipv4NotifierWorkQueue);
#endif

		hdd_deregister_tx_flow_control(adapter);

#ifdef WLAN_NS_OFFLOAD
#ifdef WLAN_OPEN_SOURCE
		cancel_work_sync(&adapter->ipv6NotifierWorkQueue);
#endif
#endif

		/*
		 * It is possible that the caller of this function does not
		 * wish to close the session
		 */
		if (true == bCloseSession)
			hdd_wait_for_sme_close_sesion(hdd_ctx, adapter);
		break;

	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
		if (hdd_ctx->config->conc_custom_rule1 &&
			(QDF_SAP_MODE == adapter->device_mode)) {
			/*
			 * Before stopping the sap adapter, lets make sure there
			 * is no sap restart work pending.
			 */
			cds_flush_work(&hdd_ctx->sap_start_work);
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
			       FL("Canceled the pending SAP restart work"));
			cds_change_sap_restart_required_status(false);
		}
		/* Any softap specific cleanup here... */
		if (adapter->device_mode == QDF_P2P_GO_MODE)
			wlan_hdd_cleanup_remain_on_channel_ctx(adapter);

		hdd_deregister_tx_flow_control(adapter);

		mutex_lock(&hdd_ctx->sap_lock);
		if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
			QDF_STATUS status;
			QDF_STATUS qdf_status;

			/* Stop Bss. */
#ifdef WLAN_FEATURE_MBSSID
			status = wlansap_stop_bss(
					WLAN_HDD_GET_SAP_CTX_PTR(adapter));
#else
			status = wlansap_stop_bss(
				(WLAN_HDD_GET_CTX(adapter))->pcds_context);
#endif

			if (QDF_IS_STATUS_SUCCESS(status)) {
				hdd_hostapd_state_t *hostapd_state =
					WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
				qdf_event_reset(&hostapd_state->
						qdf_stop_bss_event);
				qdf_status =
					qdf_wait_single_event(&hostapd_state->
							qdf_stop_bss_event,
							BSS_WAIT_TIMEOUT);

				if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
					hddLog(LOGE,
					       FL(
						  "failure waiting for wlansap_stop_bss %d"
						 ),
					       qdf_status);
				}
			} else {
				hddLog(LOGE, FL("failure in wlansap_stop_bss"));
			}
			clear_bit(SOFTAP_BSS_STARTED, &adapter->event_flags);
			cds_decr_session_set_pcl(
						     adapter->device_mode,
							adapter->sessionId);

			qdf_copy_macaddr(&updateIE.bssid,
					 &adapter->macAddressCurrent);
			updateIE.smeSessionId = adapter->sessionId;
			updateIE.ieBufferlength = 0;
			updateIE.pAdditionIEBuffer = NULL;
			updateIE.append = false;
			updateIE.notify = false;
			/* Probe bcn reset */
			if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
					      &updateIE, eUPDATE_IE_PROBE_BCN)
			    == QDF_STATUS_E_FAILURE) {
				hddLog(LOGE,
				       FL(
					  "Could not pass on PROBE_RSP_BCN data to PE"
					 ));
			}
			/* Assoc resp reset */
			if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
					      &updateIE,
					      eUPDATE_IE_ASSOC_RESP) ==
			    QDF_STATUS_E_FAILURE) {
				hddLog(LOGE,
				       FL(
					  "Could not pass on ASSOC_RSP data to PE"
					 ));
			}
			/* Reset WNI_CFG_PROBE_RSP Flags */
			wlan_hdd_reset_prob_rspies(adapter);
			kfree(adapter->sessionCtx.ap.beacon);
			adapter->sessionCtx.ap.beacon = NULL;
		}
		mutex_unlock(&hdd_ctx->sap_lock);
		if (true == bCloseSession)
			hdd_wait_for_sme_close_sesion(hdd_ctx, adapter);
		break;
	case QDF_OCB_MODE:
		ol_txrx_clear_peer(WLAN_HDD_GET_STATION_CTX_PTR(adapter)->
			conn_info.staId[0]);
		break;
	default:
		break;
	}

	EXIT();
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_stop_all_adapters(hdd_context_t *hdd_ctx)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;

	ENTER();

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		hdd_stop_adapter(hdd_ctx, adapter, true);
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	EXIT();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_reset_all_adapters(hdd_context_t *hdd_ctx)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;

	ENTER();

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_NETIF_TX_DISABLE_N_CARRIER,
					   WLAN_CONTROL_PATH);

		adapter->sessionCtx.station.hdd_ReassocScenario = false;

		hdd_deinit_tx_rx(adapter);
		cds_decr_session_set_pcl(adapter->device_mode,
						adapter->sessionId);
		if (test_bit(WMM_INIT_DONE, &adapter->event_flags)) {
			hdd_wmm_adapter_close(adapter);
			clear_bit(WMM_INIT_DONE, &adapter->event_flags);
		}

		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	EXIT();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_start_all_adapters(hdd_context_t *hdd_ctx)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;
#ifndef MSM_PLATFORM
	struct qdf_mac_addr bcastMac = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
#endif
	eConnectionState connState;

	ENTER();

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;

		hdd_wmm_init(adapter);

		switch (adapter->device_mode) {
		case QDF_STA_MODE:
		case QDF_P2P_CLIENT_MODE:
		case QDF_P2P_DEVICE_MODE:

			connState = (WLAN_HDD_GET_STATION_CTX_PTR(adapter))
					->conn_info.connState;

			hdd_init_station_mode(adapter);
			/* Open the gates for HDD to receive Wext commands */
			adapter->isLinkUpSvcNeeded = false;
			adapter->scan_info.mScanPending = false;

			/* Indicate disconnect event to supplicant if associated previously */
			if (eConnectionState_Associated == connState ||
			    eConnectionState_IbssConnected == connState ||
			    eConnectionState_NotConnected == connState ||
			    eConnectionState_IbssDisconnected == connState ||
			    eConnectionState_Disconnecting == connState) {
				union iwreq_data wrqu;
				memset(&wrqu, '\0', sizeof(wrqu));
				wrqu.ap_addr.sa_family = ARPHRD_ETHER;
				memset(wrqu.ap_addr.sa_data, '\0', ETH_ALEN);
				wireless_send_event(adapter->dev, SIOCGIWAP,
						    &wrqu, NULL);
				adapter->sessionCtx.station.
				hdd_ReassocScenario = false;

				/* indicate disconnected event to nl80211 */
				cfg80211_disconnected(adapter->dev,
						      WLAN_REASON_UNSPECIFIED,
						      NULL, 0,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)) || defined(WITH_BACKPORTS)
						      true,
#endif
						      GFP_KERNEL);
			} else if (eConnectionState_Connecting == connState) {
				/*
				 * Indicate connect failure to supplicant if we were in the
				 * process of connecting
				 */
				cfg80211_connect_result(adapter->dev, NULL,
							NULL, 0, NULL, 0,
							WLAN_STATUS_ASSOC_DENIED_UNSPEC,
							GFP_KERNEL);
			}

			hdd_register_tx_flow_control(adapter,
					hdd_tx_resume_timer_expired_handler,
					hdd_tx_resume_cb);

			break;

		case QDF_SAP_MODE:
			/* softAP can handle SSR */
			break;

		case QDF_P2P_GO_MODE:
#ifdef MSM_PLATFORM
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("[SSR] send stop ap to supplicant"));
			cfg80211_ap_stopped(adapter->dev, GFP_KERNEL);
#else
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("[SSR] send restart supplicant"));
			/* event supplicant to restart */
			cfg80211_del_sta(adapter->dev,
					 (const u8 *)&bcastMac.bytes[0],
					 GFP_KERNEL);
#endif
			break;

		default:
			break;
		}

		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	EXIT();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_get_front_adapter(hdd_context_t *hdd_ctx,
				 hdd_adapter_list_node_t **padapterNode)
{
	QDF_STATUS status;
	qdf_spin_lock_bh(&hdd_ctx->hdd_adapter_lock);
	status = qdf_list_peek_front(&hdd_ctx->hddAdapters,
				     (qdf_list_node_t **) padapterNode);
	qdf_spin_unlock_bh(&hdd_ctx->hdd_adapter_lock);
	return status;
}

QDF_STATUS hdd_get_next_adapter(hdd_context_t *hdd_ctx,
				hdd_adapter_list_node_t *adapterNode,
				hdd_adapter_list_node_t **pNextAdapterNode)
{
	QDF_STATUS status;
	qdf_spin_lock_bh(&hdd_ctx->hdd_adapter_lock);
	status = qdf_list_peek_next(&hdd_ctx->hddAdapters,
				    (qdf_list_node_t *) adapterNode,
				    (qdf_list_node_t **) pNextAdapterNode);

	qdf_spin_unlock_bh(&hdd_ctx->hdd_adapter_lock);
	return status;
}

QDF_STATUS hdd_remove_adapter(hdd_context_t *hdd_ctx,
			      hdd_adapter_list_node_t *adapterNode)
{
	QDF_STATUS status;
	qdf_spin_lock_bh(&hdd_ctx->hdd_adapter_lock);
	status = qdf_list_remove_node(&hdd_ctx->hddAdapters,
				      &adapterNode->node);
	qdf_spin_unlock_bh(&hdd_ctx->hdd_adapter_lock);
	return status;
}

QDF_STATUS hdd_remove_front_adapter(hdd_context_t *hdd_ctx,
				    hdd_adapter_list_node_t **padapterNode)
{
	QDF_STATUS status;
	qdf_spin_lock_bh(&hdd_ctx->hdd_adapter_lock);
	status = qdf_list_remove_front(&hdd_ctx->hddAdapters,
				       (qdf_list_node_t **) padapterNode);
	qdf_spin_unlock_bh(&hdd_ctx->hdd_adapter_lock);
	return status;
}

QDF_STATUS hdd_add_adapter_back(hdd_context_t *hdd_ctx,
				hdd_adapter_list_node_t *adapterNode)
{
	QDF_STATUS status;
	qdf_spin_lock_bh(&hdd_ctx->hdd_adapter_lock);
	status = qdf_list_insert_back(&hdd_ctx->hddAdapters,
				      (qdf_list_node_t *) adapterNode);
	qdf_spin_unlock_bh(&hdd_ctx->hdd_adapter_lock);
	return status;
}

QDF_STATUS hdd_add_adapter_front(hdd_context_t *hdd_ctx,
				 hdd_adapter_list_node_t *adapterNode)
{
	QDF_STATUS status;
	qdf_spin_lock_bh(&hdd_ctx->hdd_adapter_lock);
	status = qdf_list_insert_front(&hdd_ctx->hddAdapters,
				       (qdf_list_node_t *) adapterNode);
	qdf_spin_unlock_bh(&hdd_ctx->hdd_adapter_lock);
	return status;
}

hdd_adapter_t *hdd_get_adapter_by_macaddr(hdd_context_t *hdd_ctx,
					  tSirMacAddr macAddr)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *adapter;
	QDF_STATUS status;

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;

		if (adapter
		    && !qdf_mem_cmp(adapter->macAddressCurrent.bytes,
				       macAddr, sizeof(tSirMacAddr))) {
			return adapter;
		}
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	return NULL;

}

hdd_adapter_t *hdd_get_adapter_by_vdev(hdd_context_t *hdd_ctx,
				       uint32_t vdev_id)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *adapter;
	QDF_STATUS qdf_status;

	qdf_status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while ((NULL != adapterNode) && (QDF_STATUS_SUCCESS == qdf_status)) {
		adapter = adapterNode->pAdapter;

		if (adapter->sessionId == vdev_id)
			return adapter;

		qdf_status =
			hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	hddLog(QDF_TRACE_LEVEL_ERROR,
	       FL("vdev_id %d does not exist with host"), vdev_id);

	return NULL;
}

/**
 * hdd_get_adapter_by_sme_session_id() - Return adapter with
 * the sessionid
 * @hdd_ctx: hdd context.
 * @sme_session_id: sme session is for the adapter to get.
 *
 * This function is used to get the adapter with provided session id
 *
 * Return: adapter pointer if found
 *
 */
hdd_adapter_t *hdd_get_adapter_by_sme_session_id(hdd_context_t *hdd_ctx,
						uint32_t sme_session_id)
{
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	hdd_adapter_t *adapter;
	QDF_STATUS qdf_status;


	qdf_status = hdd_get_front_adapter(hdd_ctx, &adapter_node);

	while ((NULL != adapter_node) &&
			(QDF_STATUS_SUCCESS == qdf_status)) {
		adapter = adapter_node->pAdapter;

		if (adapter &&
			 adapter->sessionId == sme_session_id)
			return adapter;

		qdf_status =
			hdd_get_next_adapter(hdd_ctx,
				 adapter_node, &next);
		adapter_node = next;
	}
	return NULL;
}

/**
 * hdd_get_adapter() - to get adapter matching the mode
 * @hdd_ctx: hdd context
 * @mode: adapter mode
 *
 * This routine will return the pointer to adapter matching
 * with the passed mode.
 *
 * Return: pointer to adapter or null
 */
hdd_adapter_t *hdd_get_adapter(hdd_context_t *hdd_ctx,
			enum tQDF_ADAPTER_MODE mode)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *adapter;
	QDF_STATUS status;

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;

		if (adapter && (mode == adapter->device_mode))
			return adapter;

		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	return NULL;

}

/**
 * hdd_get_operating_channel() - return operating channel of the device mode
 * @hdd_ctx:	Pointer to the HDD context.
 * @mode:	Device mode for which operating channel is required.
 *              Suported modes:
 *			QDF_STA_MODE,
 *			QDF_P2P_CLIENT_MODE,
 *			QDF_SAP_MODE,
 *			QDF_P2P_GO_MODE.
 *
 * This API returns the operating channel of the requested device mode
 *
 * Return: channel number. "0" id the requested device is not found OR it is
 *	   not connected.
 */
uint8_t hdd_get_operating_channel(hdd_context_t *hdd_ctx,
			enum tQDF_ADAPTER_MODE mode)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;
	uint8_t operatingChannel = 0;

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;

		if (mode == adapter->device_mode) {
			switch (adapter->device_mode) {
			case QDF_STA_MODE:
			case QDF_P2P_CLIENT_MODE:
				if (hdd_conn_is_connected
					    (WLAN_HDD_GET_STATION_CTX_PTR
						(adapter))) {
					operatingChannel =
						(WLAN_HDD_GET_STATION_CTX_PTR
						(adapter))->conn_info.
							operationChannel;
				}
				break;
			case QDF_SAP_MODE:
			case QDF_P2P_GO_MODE:
				/* softap connection info */
				if (test_bit
					    (SOFTAP_BSS_STARTED,
					    &adapter->event_flags))
					operatingChannel =
						(WLAN_HDD_GET_AP_CTX_PTR
						(adapter))->operatingChannel;
				break;
			default:
				break;
			}

			break;  /* Found the device of interest. break the loop */
		}

		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}
	return operatingChannel;
}

static inline QDF_STATUS hdd_unregister_wext_all_adapters(hdd_context_t *
							  hdd_ctx)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;

	ENTER();

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		if ((adapter->device_mode == QDF_STA_MODE) ||
		    (adapter->device_mode == QDF_P2P_CLIENT_MODE) ||
		    (adapter->device_mode == QDF_IBSS_MODE) ||
		    (adapter->device_mode == QDF_P2P_DEVICE_MODE) ||
		    (adapter->device_mode == QDF_SAP_MODE) ||
		    (adapter->device_mode == QDF_P2P_GO_MODE)) {
			wlan_hdd_cfg80211_deregister_frames(adapter);
			hdd_unregister_wext(adapter->dev);
		}
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	EXIT();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_abort_mac_scan_all_adapters(hdd_context_t *hdd_ctx)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;

	ENTER();

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		if ((adapter->device_mode == QDF_STA_MODE) ||
		    (adapter->device_mode == QDF_P2P_CLIENT_MODE) ||
		    (adapter->device_mode == QDF_IBSS_MODE) ||
		    (adapter->device_mode == QDF_P2P_DEVICE_MODE) ||
		    (adapter->device_mode == QDF_SAP_MODE) ||
		    (adapter->device_mode == QDF_P2P_GO_MODE)) {
			hdd_abort_mac_scan(hdd_ctx, adapter->sessionId,
					   eCSR_SCAN_ABORT_DEFAULT);
		}
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	EXIT();

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_NS_OFFLOAD
/**
 * hdd_wlan_unregister_ip6_notifier() - unregister IPv6 change notifier
 * @hdd_ctx: Pointer to hdd context
 *
 * Unregister for IPv6 address change notifications.
 *
 * Return: None
 */
static void hdd_wlan_unregister_ip6_notifier(hdd_context_t *hdd_ctx)
{
	unregister_inet6addr_notifier(&hdd_ctx->ipv6_notifier);

	return;
}

/**
 * hdd_wlan_register_ip6_notifier() - register IPv6 change notifier
 * @hdd_ctx: Pointer to hdd context
 *
 * Register for IPv6 address change notifications.
 *
 * Return: 0 on success and errno on failure.
 */
static int hdd_wlan_register_ip6_notifier(hdd_context_t *hdd_ctx)
{
	int ret;

	hdd_ctx->ipv6_notifier.notifier_call = wlan_hdd_ipv6_changed;
	ret = register_inet6addr_notifier(&hdd_ctx->ipv6_notifier);
	if (ret) {
		hdd_err("Failed to register IPv6 notifier: %d", ret);
		goto out;
	}

	hdd_info("Registered IPv6 notifier");
out:
	return ret;
}
#else
/**
 * hdd_wlan_unregister_ip6_notifier() - unregister IPv6 change notifier
 * @hdd_ctx: Pointer to hdd context
 *
 * Unregister for IPv6 address change notifications.
 *
 * Return: None
 */
static void hdd_wlan_unregister_ip6_notifier(hdd_context_t *hdd_ctx)
{
}

/**
 * hdd_wlan_register_ip6_notifier() - register IPv6 change notifier
 * @hdd_ctx: Pointer to hdd context
 *
 * Register for IPv6 address change notifications.
 *
 * Return: None
 */
static int hdd_wlan_register_ip6_notifier(hdd_context_t *hdd_ctx)
{
	return 0;
}
#endif

#ifdef QCA_WIFI_FTM
/**
 * hdd_disable_ftm() - Disable FTM mode
 * @hdd_ctx:	HDD context
 *
 * Helper function to disable FTM mode.
 *
 * Return: None.
 */
static void hdd_disable_ftm(hdd_context_t *hdd_ctx)
{
	hdd_notice("Disabling FTM mode");

	if (hdd_ftm_stop(hdd_ctx)) {
		hdd_alert("hdd_ftm_stop Failed!");
		QDF_ASSERT(0);
	}

	hdd_ctx->ftm.ftm_state = WLAN_FTM_STOPPED;

	wlan_hdd_ftm_close(hdd_ctx);

	return;
}

/**
 * hdd_enable_ftm() - Enable FTM mode
 * @hdd_ctx:	HDD context
 *
 * Helper function to enable FTM mode.
 *
 * Return: 0 on success and errno on failure.
 */
int hdd_enable_ftm(hdd_context_t *hdd_ctx)
{
	int ret;

	ret = wlan_hdd_ftm_open(hdd_ctx);
	if (ret) {
		hdd_alert("wlan_hdd_ftm_open Failed: %d", ret);
		goto err_out;
	}

	ret = hdd_ftm_start(hdd_ctx);

	if (ret) {
		hdd_alert("hdd_ftm_start Failed: %d", ret);
		goto err_ftm_close;
	}

	ret = wiphy_register(hdd_ctx->wiphy);
	if (ret) {
		hdd_alert("wiphy register failed: %d", ret);
		goto err_ftm_stop;
	}

	hdd_err("FTM driver loaded");

	return 0;

err_ftm_stop:
	hdd_ftm_stop(hdd_ctx);
err_ftm_close:
	wlan_hdd_ftm_close(hdd_ctx);
err_out:
	return ret;

}
#else
int hdd_enable_ftm(hdd_context_t *hdd_ctx)
{
	hdd_err("Driver built without FTM feature enabled!");

	return -ENOTSUPP;
}

static inline void hdd_disable_ftm(hdd_context_t *hdd_ctx) { }
#endif

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
/**
 * hdd_logging_sock_activate_svc() - Activate logging
 * @hdd_ctx: HDD context
 *
 * Activates the logging service
 *
 * Return: Zero in case of success, negative value otherwise
 */
static int hdd_logging_sock_activate_svc(hdd_context_t *hdd_ctx)
{
	int ret;
	struct hdd_config *config = hdd_ctx->config;

	if (!config->wlanLoggingEnable)
		return 0;

	ret = wlan_logging_sock_activate_svc(config->wlanLoggingFEToConsole,
					     config->wlanLoggingNumBuf);
	if (ret)
		hdd_err("wlan_logging_sock_activate_svc failed: %d", ret);
	return ret;
}

/**
 * wlan_hdd_logging_sock_deactivate_svc() - Deactivate logging
 * @hdd_ctx: HDD context
 *
 * Deactivates the logging service
 *
 * Return: 0 on deactivating the logging service
 */
static int hdd_logging_sock_deactivate_svc(hdd_context_t *hdd_ctx)
{
	if (hdd_ctx && hdd_ctx->config->wlanLoggingEnable)
		return wlan_logging_sock_deactivate_svc();

	return 0;
}
#else
static inline int hdd_logging_sock_activate_svc(hdd_context_t *hdd_ctx)
{
	return 0;
}

static inline int hdd_logging_sock_deactivate_svc(hdd_context_t *hdd_ctx)
{
	return 0;
}
#endif

/**
 * hdd_register_notifiers - Register netdev notifiers.
 * @hdd_ctx: HDD context
 *
 * Register netdev notifiers like IPv4 and IPv6.
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_register_notifiers(hdd_context_t *hdd_ctx)
{
	int ret;

	ret = register_netdevice_notifier(&hdd_netdev_notifier);
	if (ret) {
		hdd_err("register_netdevice_notifier failed: %d", ret);
		goto out;
	}

	ret = hdd_wlan_register_ip6_notifier(hdd_ctx);
	if (ret)
		goto unregister_notifier;

	hdd_ctx->ipv4_notifier.notifier_call = wlan_hdd_ipv4_changed;
	ret = register_inetaddr_notifier(&hdd_ctx->ipv4_notifier);
	if (ret) {
		hdd_err("Failed to register IPv4 notifier: %d", ret);
		goto unregister_ip6_notifier;
	}

	return 0;

unregister_ip6_notifier:
	hdd_wlan_unregister_ip6_notifier(hdd_ctx);
unregister_notifier:
	unregister_netdevice_notifier(&hdd_netdev_notifier);
out:
	return ret;

}

/**
 * hdd_unregister_notifiers - Unregister netdev notifiers.
 * @hdd_ctx: HDD context
 *
 * Unregister netdev notifiers like IPv4 and IPv6.
 *
 * Return: None.
 */
static void hdd_unregister_notifiers(hdd_context_t *hdd_ctx)
{
	hdd_wlan_unregister_ip6_notifier(hdd_ctx);

	unregister_inetaddr_notifier(&hdd_ctx->ipv4_notifier);

	unregister_netdevice_notifier(&hdd_netdev_notifier);
}

/**
 * hdd_exit_netlink_services - Exit netlink services
 * @hdd_ctx: HDD context
 *
 * Exit netlink services like cnss_diag, cesium netlink socket, ptt socket and
 * nl service.
 *
 * Return: None.
 */
static void hdd_exit_netlink_services(hdd_context_t *hdd_ctx)
{
	cnss_diag_notify_wlan_close();

	hdd_close_cesium_nl_sock();

	ptt_sock_deactivate_svc();

	nl_srv_exit();
}

/**
 * hdd_init_netlink_services- Init netlink services
 * @hdd_ctx: HDD context
 *
 * Init netlink services like cnss_diag, cesium netlink socket, ptt socket and
 * nl service.
 *
 * Return: 0 on success and errno on failure.
 */
static int hdd_init_netlink_services(hdd_context_t *hdd_ctx)
{
	int ret;

	ret = nl_srv_init();
	if (ret) {
		hdd_alert("nl_srv_init failed: %d", ret);
		goto out;
	}

	ret = oem_activate_service(hdd_ctx);
	if (ret) {
		hdd_alert("oem_activate_service failed: %d", ret);
		goto err_nl_srv;
	}

	ret = ptt_sock_activate_svc();
	if (ret) {
		hdd_alert("ptt_sock_activate_svc failed: %d", ret);
		goto err_nl_srv;
	}

	ret = hdd_open_cesium_nl_sock();
	if (ret) {
		hdd_alert("hdd_open_cesium_nl_sock failed");
		goto err_ptt_deactivate;
	}

	ret = cnss_diag_activate_service();
	if (ret) {
		hdd_alert("cnss_diag_activate_service failed: %d", ret);
		goto err_close_cesium;
	}

	return 0;

err_close_cesium:
	hdd_close_cesium_nl_sock();
err_ptt_deactivate:
	ptt_sock_deactivate_svc();
err_nl_srv:
	nl_srv_exit();
out:
	return ret;
}

#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
/**
 * hdd_rx_wake_lock_destroy() - Destroy RX wakelock
 * @hdd_ctx:	HDD context.
 *
 * Destroy RX wakelock.
 *
 * Return: None.
 */
static void hdd_rx_wake_lock_destroy(hdd_context_t *hdd_ctx)
{
	qdf_wake_lock_destroy(&hdd_ctx->rx_wake_lock);
}

/**
 * hdd_rx_wake_lock_create() - Create RX wakelock
 * @hdd_ctx:	HDD context.
 *
 * Create RX wakelock.
 *
 * Return: None.
 */
static void hdd_rx_wake_lock_create(hdd_context_t *hdd_ctx)
{
	qdf_wake_lock_create(&hdd_ctx->rx_wake_lock, "qcom_rx_wakelock");
}
#else
static void hdd_rx_wake_lock_destroy(hdd_context_t *hdd_ctx) { }
static void hdd_rx_wake_lock_create(hdd_context_t *hdd_ctx) { }
#endif

/**
 * hdd_roc_context_init() - Init ROC context
 * @hdd_ctx:	HDD context.
 *
 * Initialize ROC context.
 *
 * Return: 0 on success and errno on failure.
 */
static int hdd_roc_context_init(hdd_context_t *hdd_ctx)
{
	qdf_spinlock_create(&hdd_ctx->hdd_roc_req_q_lock);
	qdf_list_create(&hdd_ctx->hdd_roc_req_q, MAX_ROC_REQ_QUEUE_ENTRY);

	INIT_DELAYED_WORK(&hdd_ctx->roc_req_work, wlan_hdd_roc_request_dequeue);

	return 0;
}

/**
 * hdd_roc_context_destroy() - Destroy ROC context
 * @hdd_ctx:	HDD context.
 *
 * Destroy roc list and flush the pending roc work.
 *
 * Return: None.
 */
static void hdd_roc_context_destroy(hdd_context_t *hdd_ctx)
{
	flush_delayed_work(&hdd_ctx->roc_req_work);
	qdf_list_destroy(&hdd_ctx->hdd_roc_req_q);
}

/**
 * hdd_context_destroy() - Destroy HDD context
 * @hdd_ctx:	HDD context to be destroyed.
 *
 * Free config and HDD context as well as destroy all the resources.
 *
 * Return: None
 */
static void hdd_context_destroy(hdd_context_t *hdd_ctx)
{
	if (QDF_GLOBAL_FTM_MODE != hdd_get_conparam())
		hdd_logging_sock_deactivate_svc(hdd_ctx);

	hdd_roc_context_destroy(hdd_ctx);

	hdd_sap_context_destroy(hdd_ctx);

	hdd_rx_wake_lock_destroy(hdd_ctx);

	hdd_tdls_context_destroy(hdd_ctx);

	hdd_scan_context_destroy(hdd_ctx);

	qdf_list_destroy(&hdd_ctx->hddAdapters);

	qdf_mem_free(hdd_ctx->config);
	hdd_ctx->config = NULL;

	wiphy_free(hdd_ctx->wiphy);
}

/**
 * hdd_wlan_exit() - HDD WLAN exit function
 * @hdd_ctx:	Pointer to the HDD Context
 *
 * This is the driver exit point (invoked during rmmod)
 *
 * Return: None
 */
void hdd_wlan_exit(hdd_context_t *hdd_ctx)
{
	v_CONTEXT_t p_cds_context = hdd_ctx->pcds_context;
	QDF_STATUS qdf_status;
	struct wiphy *wiphy = hdd_ctx->wiphy;

	ENTER();

	hdd_unregister_wext_all_adapters(hdd_ctx);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_disable_ftm(hdd_ctx);

		hdd_alert("FTM driver unloaded");
		goto free_hdd_ctx;
	}

	hdd_unregister_notifiers(hdd_ctx);

	/*
	 * Cancel any outstanding scan requests.  We are about to close all
	 * of our adapters, but an adapter structure is what SME passes back
	 * to our callback function.  Hence if there are any outstanding scan
	 * requests then there is a race condition between when the adapter
	 * is closed and when the callback is invoked.  We try to resolve that
	 * race condition here by canceling any outstanding scans before we
	 * close the adapters.
	 * Note that the scans may be cancelled in an asynchronous manner, so
	 * ideally there needs to be some kind of synchronization.  Rather than
	 * introduce a new synchronization here, we will utilize the fact that
	 * we are about to Request Full Power, and since that is synchronized,
	 * the expectation is that by the time Request Full Power has completed,
	 * all scans will be cancelled
	 */
	hdd_abort_mac_scan_all_adapters(hdd_ctx);

#ifdef MSM_PLATFORM
	if (QDF_TIMER_STATE_RUNNING ==
	    qdf_mc_timer_get_current_state(&hdd_ctx->bus_bw_timer)) {
		qdf_mc_timer_stop(&hdd_ctx->bus_bw_timer);
	}

	if (!QDF_IS_STATUS_SUCCESS
		    (qdf_mc_timer_destroy(&hdd_ctx->bus_bw_timer))) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Cannot deallocate Bus bandwidth timer"));
	}
#endif

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	if (QDF_TIMER_STATE_RUNNING ==
	    qdf_mc_timer_get_current_state(&hdd_ctx->skip_acs_scan_timer)) {
		qdf_mc_timer_stop(&hdd_ctx->skip_acs_scan_timer);
	}

	if (!QDF_IS_STATUS_SUCCESS
		    (qdf_mc_timer_destroy(&hdd_ctx->skip_acs_scan_timer))) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Cannot deallocate ACS Skip timer"));
	}
#endif

	/*
	 * Powersave Offload Case
	 * Disable Idle Power Save Mode
	 */
	hdd_set_idle_ps_config(hdd_ctx, false);

	/* Unregister the Net Device Notifier */
	unregister_netdevice_notifier(&hdd_netdev_notifier);

	/*
	 * Stop all adapters, this will ensure the termination of active
	 * connections on the interface. Make sure the cds_scheduler is
	 * still available to handle those control messages
	 */
	hdd_stop_all_adapters(hdd_ctx);

	/* Stop all the modules */
	qdf_status = cds_disable(p_cds_context);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("Failed to stop CDS"));
		QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));
	}

	/*
	 * Close the scheduler before calling cds_close to make sure no thread
	 * is scheduled after the each module close is called i.e after all the
	 * data structures are freed.
	 */
	qdf_status = cds_sched_close(p_cds_context);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("Failed to close CDS Scheduler"));
		QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));
	}

	qdf_spinlock_destroy(&hdd_ctx->hdd_adapter_lock);
	qdf_spinlock_destroy(&hdd_ctx->sta_update_info_lock);
	qdf_spinlock_destroy(&hdd_ctx->connection_status_lock);

	/*
	 * Close CDS
	 * This frees pMac(HAL) context. There should not be any call
	 * that requires pMac access after this.
	 */
	cds_close(p_cds_context);

	hdd_wlan_green_ap_deinit(hdd_ctx);

	hdd_exit_netlink_services(hdd_ctx);

	hdd_close_all_adapters(hdd_ctx, false);

	hdd_ipa_cleanup(hdd_ctx);

	/* Free up RoC request queue and flush workqueue */
	cds_flush_work(&hdd_ctx->roc_req_work);

	if (!QDF_IS_STATUS_SUCCESS(cds_deinit_policy_mgr())) {
		hdd_err("Failed to deinit policy manager");
		/* Proceed and complete the clean up */
	}

free_hdd_ctx:

	wlan_hdd_deinit_tx_rx_histogram(hdd_ctx);
	wiphy_unregister(wiphy);

	hdd_context_destroy(hdd_ctx);
}

void __hdd_wlan_exit(void)
{
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("Invalid HDD Context"));
		EXIT();
		return;
	}

	/* Check IPA HW Pipe shutdown */
	hdd_ipa_uc_force_pipe_shutdown(hdd_ctx);

#ifdef WLAN_FEATURE_LPSS
	wlan_hdd_send_status_pkg(NULL, NULL, 0, 0);
#endif

	memdump_deinit();

	/* Do all the cleanup before deregistering the driver */
	hdd_wlan_exit(hdd_ctx);

	EXIT();
}

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
void hdd_skip_acs_scan_timer_handler(void *data)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *) data;

	hddLog(LOG1, FL("ACS Scan result expired. Reset ACS scan skip"));
	hdd_ctx->skip_acs_scan_status = eSAP_DO_NEW_ACS_SCAN;

	if (!hdd_ctx->hHal)
		return;
	sme_scan_flush_result(hdd_ctx->hHal);
}
#endif

#ifdef QCA_HT_2040_COEX
/**
 * hdd_wlan_set_ht2040_mode() - notify FW with HT20/HT40 mode
 * @adapter: pointer to adapter
 * @staId: station id
 * @macAddrSTA: station MAC address
 * @channel_type: channel type
 *
 * This function notifies FW with HT20/HT40 mode
 *
 * Return: 0 if successful, error number otherwise
 */
int hdd_wlan_set_ht2040_mode(hdd_adapter_t *adapter, uint16_t staId,
			     struct qdf_mac_addr macAddrSTA, int channel_type)
{
	int status;
	QDF_STATUS qdf_status;
	hdd_context_t *hdd_ctx = NULL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status) {
		hddLog(LOGE, FL("HDD context is not valid"));
		return status;
	}
	if (!hdd_ctx->hHal)
		return -EINVAL;

	qdf_status = sme_notify_ht2040_mode(hdd_ctx->hHal, staId, macAddrSTA,
					    adapter->sessionId, channel_type);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hddLog(LOGE, "Fail to send notification with ht2040 mode");
		return -EINVAL;
	}

	return 0;
}
#endif

/**
 * hdd_wlan_notify_modem_power_state() - notify FW with modem power status
 * @state: state
 *
 * This function notifies FW with modem power status
 *
 * Return: 0 if successful, error number otherwise
 */
int hdd_wlan_notify_modem_power_state(int state)
{
	int status;
	QDF_STATUS qdf_status;
	hdd_context_t *hdd_ctx;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status) {
		hddLog(LOGE, FL("HDD context is not valid"));
		return status;
	}
	if (!hdd_ctx->hHal)
		return -EINVAL;

	qdf_status = sme_notify_modem_power_state(hdd_ctx->hHal, state);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hddLog(LOGE,
		       "Fail to send notification with modem power state %d",
		       state);
		return -EINVAL;
	}
	return 0;
}

/**
 *
 * hdd_post_cds_enable_config() - HDD post cds start config helper
 * @adapter - Pointer to the HDD
 *
 * Return: None
 */
QDF_STATUS hdd_post_cds_enable_config(hdd_context_t *hdd_ctx)
{
	QDF_STATUS qdf_ret_status;

	/*
	 * Send ready indication to the HDD.  This will kick off the MAC
	 * into a 'running' state and should kick off an initial scan.
	 */
	qdf_ret_status = sme_hdd_ready_ind(hdd_ctx->hHal);
	if (!QDF_IS_STATUS_SUCCESS(qdf_ret_status)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL(
			  "sme_hdd_ready_ind() failed with status code %08d [x%08x]"
			 ),
		       qdf_ret_status, qdf_ret_status);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* wake lock APIs for HDD */
void hdd_prevent_suspend(uint32_t reason)
{
	qdf_wake_lock_acquire(&wlan_wake_lock, reason);
}

void hdd_allow_suspend(uint32_t reason)
{
	qdf_wake_lock_release(&wlan_wake_lock, reason);
}

void hdd_prevent_suspend_timeout(uint32_t timeout, uint32_t reason)
{
	qdf_wake_lock_timeout_acquire(&wlan_wake_lock, timeout, reason);
}

/**
 * hdd_exchange_version_and_caps() - exchange version and capability with target
 * @hdd_ctx:	Pointer to HDD context
 *
 * This is the HDD function to exchange version and capability information
 * between Host and Target
 *
 * This function gets reported version of FW.
 * It also finds the version of target headers used to compile the host;
 * It compares the above two and prints a warning if they are different;
 * It gets the SW and HW version string;
 * Finally, it exchanges capabilities between host and target i.e. host
 * and target exchange a msg indicating the features they support through a
 * bitmap
 *
 * Return: None
 */
void hdd_exchange_version_and_caps(hdd_context_t *hdd_ctx)
{

	tSirVersionType versionCompiled;
	tSirVersionType versionReported;
	tSirVersionString versionString;
	uint8_t fwFeatCapsMsgSupported = 0;
	QDF_STATUS vstatus;

	memset(&versionCompiled, 0, sizeof(versionCompiled));
	memset(&versionReported, 0, sizeof(versionReported));

	/* retrieve and display WCNSS version information */
	do {

		vstatus = sme_get_wcnss_wlan_compiled_version(hdd_ctx->hHal,
							      &versionCompiled);
		if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL(
				  "unable to retrieve WCNSS WLAN compiled version"
				 ));
			break;
		}

		vstatus = sme_get_wcnss_wlan_reported_version(hdd_ctx->hHal,
							      &versionReported);
		if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL(
				  "unable to retrieve WCNSS WLAN reported version"
				 ));
			break;
		}

		if ((versionCompiled.major != versionReported.major) ||
		    (versionCompiled.minor != versionReported.minor) ||
		    (versionCompiled.version != versionReported.version) ||
		    (versionCompiled.revision != versionReported.revision)) {
			pr_err("%s: WCNSS WLAN Version %u.%u.%u.%u, "
			       "Host expected %u.%u.%u.%u\n",
			       WLAN_MODULE_NAME,
			       (int)versionReported.major,
			       (int)versionReported.minor,
			       (int)versionReported.version,
			       (int)versionReported.revision,
			       (int)versionCompiled.major,
			       (int)versionCompiled.minor,
			       (int)versionCompiled.version,
			       (int)versionCompiled.revision);
		} else {
			pr_info("%s: WCNSS WLAN version %u.%u.%u.%u\n",
				WLAN_MODULE_NAME,
				(int)versionReported.major,
				(int)versionReported.minor,
				(int)versionReported.version,
				(int)versionReported.revision);
		}

		vstatus = sme_get_wcnss_software_version(hdd_ctx->hHal,
							 versionString,
							 sizeof(versionString));
		if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL(
				  "unable to retrieve WCNSS software version string"
				 ));
			break;
		}

		pr_info("%s: WCNSS software version %s\n",
			WLAN_MODULE_NAME, versionString);

		vstatus = sme_get_wcnss_hardware_version(hdd_ctx->hHal,
							 versionString,
							 sizeof(versionString));
		if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL(
				  "unable to retrieve WCNSS hardware version string"
				 ));
			break;
		}

		pr_info("%s: WCNSS hardware version %s\n",
			WLAN_MODULE_NAME, versionString);

		/*
		 * 1.Check if FW version is greater than 0.1.1.0. Only then
		 * send host-FW capability exchange message
		 * 2.Host-FW capability exchange message  is only present on
		 * target 1.1 so send the message only if it the target is 1.1
		 * minor numbers for different target branches:
		 * 0 -> (1.0)Mainline Build
		 * 1 -> (1.1)Mainline Build
		 * 2->(1.04) Stability Build
		 */
		if (((versionReported.major > 0) || (versionReported.minor > 1)
		     || ((versionReported.minor >= 1)
			 && (versionReported.version >= 1)))
		    && ((versionReported.major == 1)
			&& (versionReported.minor >= 1)))
			fwFeatCapsMsgSupported = 1;

		if (fwFeatCapsMsgSupported) {
			/*
			 * Indicate if IBSS heartbeat monitoring needs to be
			 * offloaded
			 */
			if (!hdd_ctx->config->enableIbssHeartBeatOffload) {
				sme_disable_feature_capablity
					(IBSS_HEARTBEAT_OFFLOAD);
			}

			sme_feature_caps_exchange(hdd_ctx->hHal);
		}

	} while (0);

}

/* Initialize channel list in sme based on the country code */
QDF_STATUS hdd_set_sme_chan_list(hdd_context_t *hdd_ctx)
{
	return sme_init_chan_list(hdd_ctx->hHal, hdd_ctx->reg.alpha2,
				  hdd_ctx->reg.cc_src);
}

/**
 * hdd_is_5g_supported() - check if hardware supports 5GHz
 * @hdd_ctx:	Pointer to the hdd context
 *
 * HDD function to know if hardware supports 5GHz
 *
 * Return:  true if hardware supports 5GHz
 */
bool hdd_is_5g_supported(hdd_context_t *hdd_ctx)
{
	/*
	 * If wcnss_wlan_iris_xo_mode() returns WCNSS_XO_48MHZ(1);
	 * then hardware support 5Ghz.
	 */
	return true;
}

static int hdd_wiphy_init(hdd_context_t *hdd_ctx)
{
	struct wiphy *wiphy;
	int ret_val;

	wiphy = hdd_ctx->wiphy;

	/*
	 * The channel information in
	 * wiphy needs to be initialized before wiphy registration
	 */
	ret_val = hdd_regulatory_init(hdd_ctx, wiphy);
	if (ret_val) {
		hdd_alert("regulatory init failed");
		return ret_val;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	wiphy->wowlan = &wowlan_support_reg_init;
#else
	wiphy->wowlan.flags = WIPHY_WOWLAN_ANY |
			      WIPHY_WOWLAN_MAGIC_PKT |
			      WIPHY_WOWLAN_DISCONNECT |
			      WIPHY_WOWLAN_SUPPORTS_GTK_REKEY |
			      WIPHY_WOWLAN_GTK_REKEY_FAILURE |
			      WIPHY_WOWLAN_EAP_IDENTITY_REQ |
			      WIPHY_WOWLAN_4WAY_HANDSHAKE |
			      WIPHY_WOWLAN_RFKILL_RELEASE;

	wiphy->wowlan.n_patterns = (WOW_MAX_FILTER_LISTS *
				    WOW_MAX_FILTERS_PER_LIST);
	wiphy->wowlan.pattern_min_len = WOW_MIN_PATTERN_SIZE;
	wiphy->wowlan.pattern_max_len = WOW_MAX_PATTERN_SIZE;
#endif

	/* registration of wiphy dev with cfg80211 */
	ret_val = wlan_hdd_cfg80211_register(wiphy);
	if (0 > ret_val)
		hdd_err("wiphy registration failed");

	return ret_val;
}

/**
 * hdd_cnss_request_bus_bandwidth() - Function to control bus bandwidth
 * @hdd_ctx - handle to hdd context
 * @tx_packets - transmit packet count
 * @rx_packets - receive packet count
 *
 * The function controls the bus bandwidth and dynamic control of
 * tcp delayed ack configuration
 *
 * Returns: None
 */
#ifdef MSM_PLATFORM
void hdd_cnss_request_bus_bandwidth(hdd_context_t *hdd_ctx,
			const uint64_t tx_packets, const uint64_t rx_packets)
{
#ifdef CONFIG_CNSS
	uint64_t total = tx_packets + rx_packets;
	uint64_t temp_rx = 0;
	uint64_t temp_tx = 0;
	enum cnss_bus_width_type next_vote_level = CNSS_BUS_WIDTH_NONE;
	enum wlan_tp_level next_rx_level = WLAN_SVC_TP_NONE;
	enum wlan_tp_level next_tx_level = WLAN_SVC_TP_NONE;


	if (total > hdd_ctx->config->busBandwidthHighThreshold)
		next_vote_level = CNSS_BUS_WIDTH_HIGH;
	else if (total > hdd_ctx->config->busBandwidthMediumThreshold)
		next_vote_level = CNSS_BUS_WIDTH_MEDIUM;
	else if (total > hdd_ctx->config->busBandwidthLowThreshold)
		next_vote_level = CNSS_BUS_WIDTH_LOW;
	else
		next_vote_level = CNSS_BUS_WIDTH_NONE;

	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].next_vote_level =
							next_vote_level;

	if (hdd_ctx->cur_vote_level != next_vote_level) {
		hdd_debug("trigger level %d, tx_packets: %lld, rx_packets: %lld",
			 next_vote_level, tx_packets, rx_packets);
		hdd_ctx->cur_vote_level = next_vote_level;
		cnss_request_bus_bandwidth(next_vote_level);
	}

	/* fine-tuning parameters for RX Flows */
	temp_rx = (rx_packets + hdd_ctx->prev_rx) / 2;

	hdd_ctx->prev_rx = rx_packets;
	if (temp_rx > hdd_ctx->config->tcpDelackThresholdHigh)
		next_rx_level = WLAN_SVC_TP_HIGH;
	else
		next_rx_level = WLAN_SVC_TP_LOW;

	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].next_rx_level =
								next_rx_level;

	if (hdd_ctx->cur_rx_level != next_rx_level) {
		hdd_debug("TCP DELACK trigger level %d, average_rx: %llu",
		       next_rx_level, temp_rx);
		hdd_ctx->cur_rx_level = next_rx_level;
		/* Send throughput indication only if it is enabled.
		 * Disabling tcp_del_ack will revert the tcp stack behavior
		 * to default delayed ack. Note that this will disable the
		 * dynamic delayed ack mechanism across the system
		 */
		if (hdd_ctx->config->enable_tcp_delack)
			wlan_hdd_send_svc_nlink_msg(WLAN_SVC_WLAN_TP_IND,
					    &next_rx_level,
					    sizeof(next_rx_level));
	}

	/* fine-tuning parameters for TX Flows */
	temp_tx = (tx_packets + hdd_ctx->prev_tx) / 2;
	hdd_ctx->prev_tx = tx_packets;
	if (temp_tx > hdd_ctx->config->tcp_tx_high_tput_thres)
		next_tx_level = WLAN_SVC_TP_HIGH;
	else
		next_tx_level = WLAN_SVC_TP_LOW;

	 if (hdd_ctx->cur_tx_level != next_tx_level) {
		hdd_debug("change TCP TX trigger level %d, average_tx: %llu",
				next_tx_level, temp_tx);
		hdd_ctx->cur_tx_level = next_tx_level;
		wlan_hdd_send_svc_nlink_msg(WLAN_SVC_WLAN_TP_TX_IND,
				&next_tx_level,
				sizeof(next_tx_level));
	}

	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].next_tx_level =
								next_tx_level;
	hdd_ctx->hdd_txrx_hist_idx++;
	hdd_ctx->hdd_txrx_hist_idx &= NUM_TX_RX_HISTOGRAM_MASK;
#endif
}

#define HDD_BW_GET_DIFF(_x, _y) (unsigned long)((ULONG_MAX - (_y)) + (_x) + 1)
static void hdd_bus_bw_compute_cbk(void *priv)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *) priv;
	hdd_adapter_t *adapter = NULL;
	uint64_t tx_packets = 0, rx_packets = 0;
	uint64_t total_tx = 0, total_rx = 0;
	hdd_adapter_list_node_t *adapterNode = NULL;
	QDF_STATUS status = 0;
	bool connected = false;
	uint32_t ipa_tx_packets = 0, ipa_rx_packets = 0;

	for (status = hdd_get_front_adapter(hdd_ctx, &adapterNode);
	     NULL != adapterNode && QDF_STATUS_SUCCESS == status;
	     status =
		     hdd_get_next_adapter(hdd_ctx, adapterNode, &adapterNode)) {

		if (adapterNode->pAdapter == NULL)
			continue;
		adapter = adapterNode->pAdapter;

		if ((adapter->device_mode == QDF_STA_MODE ||
		     adapter->device_mode == QDF_P2P_CLIENT_MODE) &&
		    WLAN_HDD_GET_STATION_CTX_PTR(adapter)->conn_info.connState
		    != eConnectionState_Associated) {

			continue;
		}

		if ((adapter->device_mode == QDF_SAP_MODE ||
		     adapter->device_mode == QDF_P2P_GO_MODE) &&
		    WLAN_HDD_GET_AP_CTX_PTR(adapter)->bApActive == false) {

			continue;
		}

		tx_packets += HDD_BW_GET_DIFF(adapter->stats.tx_packets,
					      adapter->prev_tx_packets);
		rx_packets += HDD_BW_GET_DIFF(adapter->stats.rx_packets,
					      adapter->prev_rx_packets);

		total_rx += adapter->stats.rx_packets;
		total_tx += adapter->stats.tx_packets;

		spin_lock_bh(&hdd_ctx->bus_bw_lock);
		adapter->prev_tx_packets = adapter->stats.tx_packets;
		adapter->prev_rx_packets = adapter->stats.rx_packets;
		spin_unlock_bh(&hdd_ctx->bus_bw_lock);
		connected = true;
	}

	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].total_rx = total_rx;
	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].total_tx = total_tx;
	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].interval_rx =
								rx_packets;
	hdd_ctx->hdd_txrx_hist[hdd_ctx->hdd_txrx_hist_idx].interval_tx =
								tx_packets;

	hdd_ipa_uc_stat_query(hdd_ctx, &ipa_tx_packets, &ipa_rx_packets);
	tx_packets += (uint64_t)ipa_tx_packets;
	rx_packets += (uint64_t)ipa_rx_packets;

	if (!connected) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("bus bandwidth timer running in disconnected state"));
		return;
	}

	hdd_cnss_request_bus_bandwidth(hdd_ctx, tx_packets, rx_packets);

	hdd_ipa_set_perf_level(hdd_ctx, tx_packets, rx_packets);
	hdd_ipa_uc_stat_request(adapter, 2);

	qdf_mc_timer_start(&hdd_ctx->bus_bw_timer,
			   hdd_ctx->config->busBandwidthComputeInterval);
}
#endif

/**
 * wlan_hdd_init_tx_rx_histogram() - init tx/rx histogram stats
 * @hdd_ctx: hdd context
 *
 * Return: 0 for success or error code
 */
int wlan_hdd_init_tx_rx_histogram(hdd_context_t *hdd_ctx)
{
	hdd_ctx->hdd_txrx_hist = qdf_mem_malloc(
		(sizeof(struct hdd_tx_rx_histogram) * NUM_TX_RX_HISTOGRAM));
	if (hdd_ctx->hdd_txrx_hist == NULL) {
		hdd_err("%s: Failed malloc for hdd_txrx_hist", __func__);
		return -ENOMEM;
	}
	return 0;
}

/**
 * wlan_hdd_deinit_tx_rx_histogram() - deinit tx/rx histogram stats
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void wlan_hdd_deinit_tx_rx_histogram(hdd_context_t *hdd_ctx)
{
	if (hdd_ctx->hdd_txrx_hist) {
		qdf_mem_free(hdd_ctx->hdd_txrx_hist);
		hdd_ctx->hdd_txrx_hist = NULL;
	}
}


/**
 * wlan_hdd_display_tx_rx_histogram() - display tx rx histogram
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void wlan_hdd_display_tx_rx_histogram(hdd_context_t *hdd_ctx)
{
	int i;

#ifdef MSM_PLATFORM
	hddLog(QDF_TRACE_LEVEL_ERROR, "BW Interval: %d curr_index %d",
		hdd_ctx->config->busBandwidthComputeInterval,
		hdd_ctx->hdd_txrx_hist_idx);
	hddLog(QDF_TRACE_LEVEL_ERROR,
		"BW High TH: %d BW Med TH: %d BW Low TH: %d",
		hdd_ctx->config->busBandwidthHighThreshold,
		hdd_ctx->config->busBandwidthMediumThreshold,
		hdd_ctx->config->busBandwidthLowThreshold);
	hddLog(QDF_TRACE_LEVEL_ERROR, "TCP DEL High TH: %d TCP DEL Low TH: %d",
		hdd_ctx->config->tcpDelackThresholdHigh,
		hdd_ctx->config->tcpDelackThresholdLow);
#endif

	hddLog(QDF_TRACE_LEVEL_ERROR,
		"index, total_rx, interval_rx, total_tx, interval_tx, next_vote_level, next_rx_level, next_tx_level");

	for (i = 0; i < NUM_TX_RX_HISTOGRAM; i++) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
			"%d: %llu, %llu, %llu, %llu, %d, %d, %d",
			i, hdd_ctx->hdd_txrx_hist[i].total_rx,
			hdd_ctx->hdd_txrx_hist[i].interval_rx,
			hdd_ctx->hdd_txrx_hist[i].total_tx,
			hdd_ctx->hdd_txrx_hist[i].interval_tx,
			hdd_ctx->hdd_txrx_hist[i].next_vote_level,
			hdd_ctx->hdd_txrx_hist[i].next_rx_level,
			hdd_ctx->hdd_txrx_hist[i].next_tx_level);
	}
	return;
}

/**
 * wlan_hdd_clear_tx_rx_histogram() - clear tx rx histogram
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void wlan_hdd_clear_tx_rx_histogram(hdd_context_t *hdd_ctx)
{
	hdd_ctx->hdd_txrx_hist_idx = 0;
	qdf_mem_zero(hdd_ctx->hdd_txrx_hist,
		(sizeof(struct hdd_tx_rx_histogram) * NUM_TX_RX_HISTOGRAM));
}

/**
 * wlan_hdd_display_netif_queue_history() - display netif queue operation history
 * @pHddCtx: hdd context
 *
 * Return: none
 */
void wlan_hdd_display_netif_queue_history(hdd_context_t *hdd_ctx)
{

	hdd_adapter_t *adapter = NULL;
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	QDF_STATUS status;
	int i;
	qdf_time_t total, pause, unpause, curr_time;

	status = hdd_get_front_adapter(hdd_ctx, &adapter_node);
	while (NULL != adapter_node && QDF_STATUS_SUCCESS == status) {
		adapter = adapter_node->pAdapter;

		hddLog(QDF_TRACE_LEVEL_ERROR,
			"Session_id %d device mode %d",
			adapter->sessionId, adapter->device_mode);

		hddLog(QDF_TRACE_LEVEL_ERROR,
			"Netif queue operation statistics:");
		hddLog(QDF_TRACE_LEVEL_ERROR,
			"Current pause_map value %x", adapter->pause_map);
		curr_time = qdf_system_ticks();
		total = curr_time - adapter->start_time;
		if (adapter->pause_map) {
			pause = adapter->total_pause_time +
				curr_time - adapter->last_time;
			unpause = adapter->total_unpause_time;
		} else {
			unpause = adapter->total_unpause_time +
				  curr_time - adapter->last_time;
			pause = adapter->total_pause_time;
		}
		hddLog(QDF_TRACE_LEVEL_ERROR,
			"Total: %ums Pause: %ums Unpause: %ums",
			qdf_system_ticks_to_msecs(total),
			qdf_system_ticks_to_msecs(pause),
			qdf_system_ticks_to_msecs(unpause));
		hddLog(QDF_TRACE_LEVEL_ERROR,
			"reason_type: pause_cnt: unpause_cnt");

		for (i = 1; i < WLAN_REASON_TYPE_MAX; i++) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
				"%s: %d: %d",
				hdd_reason_type_to_string(i),
				adapter->queue_oper_stats[i].pause_count,
				adapter->queue_oper_stats[i].unpause_count);
		}

		hddLog(QDF_TRACE_LEVEL_ERROR,
			"Netif queue operation history: current index %d",
			adapter->history_index);
		hddLog(QDF_TRACE_LEVEL_ERROR,
			"index: time: action_type: reason_type: pause_map");

		for (i = 0; i < WLAN_HDD_MAX_HISTORY_ENTRY; i++) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
				"%d: %u: %s: %s: %x",
				i, qdf_system_ticks_to_msecs(
					adapter->queue_oper_history[i].time),
				hdd_action_type_to_string(
				adapter->queue_oper_history[i].netif_action),
				hdd_reason_type_to_string(
				adapter->queue_oper_history[i].netif_reason),
				adapter->queue_oper_history[i].pause_map);
		}

		status = hdd_get_next_adapter(hdd_ctx, adapter_node, &next);
		adapter_node = next;
	}


}

/**
 * wlan_hdd_clear_netif_queue_history() - clear netif queue operation history
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void wlan_hdd_clear_netif_queue_history(hdd_context_t *hdd_ctx)
{
	hdd_adapter_t *adapter = NULL;
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	QDF_STATUS status;

	status = hdd_get_front_adapter(hdd_ctx, &adapter_node);
	while (NULL != adapter_node && QDF_STATUS_SUCCESS == status) {
		adapter = adapter_node->pAdapter;

		qdf_mem_zero(adapter->queue_oper_stats,
					sizeof(adapter->queue_oper_stats));
		qdf_mem_zero(adapter->queue_oper_history,
					sizeof(adapter->queue_oper_history));
		adapter->history_index = 0;
		adapter->start_time = adapter->last_time = qdf_system_ticks();
		adapter->total_pause_time = 0;
		adapter->total_unpause_time = 0;
		status = hdd_get_next_adapter(hdd_ctx, adapter_node, &next);
		adapter_node = next;
	}
}

/**
 * hdd_11d_scan_done() - callback for 11d scan completion of flushing results
 * @halHandle:	Hal handle
 * @pContext:	Pointer to the context
 * @sessionId:	Session ID
 * @scanId:	Scan ID
 * @status:	Status
 *
 * This is the callback to be executed when 11d scan is completed to flush out
 * the scan results
 *
 * 11d scan is done during driver load and is a passive scan on all
 * channels supported by the device, 11d scans may find some APs on
 * frequencies which are forbidden to be used in the regulatory domain
 * the device is operating in. If these APs are notified to the supplicant
 * it may try to connect to these APs, thus flush out all the scan results
 * which are present in SME after 11d scan is done.
 *
 * Return:  QDF_STATUS_SUCCESS
 */
static QDF_STATUS hdd_11d_scan_done(tHalHandle halHandle, void *pContext,
				    uint8_t sessionId, uint32_t scanId,
				    eCsrScanStatus status)
{
	ENTER();

	sme_scan_flush_result(halHandle);

	EXIT();

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
/**
 * hdd_init_offloaded_packets_ctx() - Initialize offload packets context
 * @hdd_ctx: hdd global context
 *
 * Return: none
 */
static void hdd_init_offloaded_packets_ctx(hdd_context_t *hdd_ctx)
{
	uint8_t i;

	mutex_init(&hdd_ctx->op_ctx.op_lock);
	for (i = 0; i < MAXNUM_PERIODIC_TX_PTRNS; i++) {
		hdd_ctx->op_ctx.op_table[i].request_id = MAX_REQUEST_ID;
		hdd_ctx->op_ctx.op_table[i].pattern_id = i;
	}
}
#else
static void hdd_init_offloaded_packets_ctx(hdd_context_t *hdd_ctx)
{
}
#endif

#ifdef WLAN_FEATURE_FASTPATH
/**
 * hdd_enable_fastpath() - Enable fastpath if enabled in config INI
 * @hdd_cfg: hdd config
 * @context: lower layer context
 *
 * Return: none
 */
static void hdd_enable_fastpath(struct hdd_config *hdd_cfg,
				void *context)
{
	if (hdd_cfg->fastpath_enable)
		hif_enable_fastpath(context);
}
#else
static void hdd_enable_fastpath(struct hdd_config *hdd_cfg,
				void *context)
{
}
#endif

#if defined(FEATURE_WLAN_CH_AVOID) && defined(CONFIG_CNSS)
/**
 * hdd_set_thermal_level_cb() - set thermal level callback function
 * @context:	hdd context pointer
 * @level:	thermal level
 *
 * Change IPA data path to SW path when the thermal throttle level greater
 * than 0, and restore the original data path when throttle level is 0
 *
 * Return: none
 */
static void hdd_set_thermal_level_cb(void *context, u_int8_t level)
{
	hdd_context_t *hdd_ctx = context;

	/* Change IPA to SW path when throttle level greater than 0 */
	if (level > THROTTLE_LEVEL_0)
		hdd_ipa_send_mcc_scc_msg(hdd_ctx, true);
	else
		/* restore original concurrency mode */
		hdd_ipa_send_mcc_scc_msg(hdd_ctx, hdd_ctx->mcc_mode);
}

/**
 * hdd_find_prefd_safe_chnl() - find safe channel within preferred channel
 * @hdd_ctxt:	hdd context pointer
 * @ap_adapter: hdd hostapd adapter pointer
 *
 * Try to find safe channel within preferred channel
 * In case auto channel selection enabled
 *  - Preferred and safe channel should be used
 *  - If no overlapping, preferred channel should be used
 *
 * Return: 1: found preferred safe channel
 *         0: could not found preferred safe channel
 */
static uint8_t hdd_find_prefd_safe_chnl(hdd_context_t *hdd_ctxt,
					hdd_adapter_t *ap_adapter)
{
	uint16_t safe_channels[NUM_CHANNELS];
	uint16_t safe_channel_count;
	uint16_t unsafe_channel_count;
	uint8_t is_unsafe = 1;
	uint16_t i;
	uint16_t channel_loop;

	if (!hdd_ctxt || !ap_adapter) {
		hdd_err("invalid context/adapter");
		return 0;
	}

	safe_channel_count = 0;
	unsafe_channel_count = QDF_MIN((uint16_t)hdd_ctxt->unsafe_channel_count,
				       (uint16_t)NUM_CHANNELS);

	for (i = 0; i < NUM_CHANNELS; i++) {
		is_unsafe = 0;
		for (channel_loop = 0;
		     channel_loop < unsafe_channel_count; channel_loop++) {
			if (CDS_CHANNEL_NUM(i) ==
			    hdd_ctxt->unsafe_channel_list[channel_loop]) {
				is_unsafe = 1;
				break;
			}
		}
		if (!is_unsafe) {
			safe_channels[safe_channel_count] =
			  CDS_CHANNEL_NUM(i);
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
			       FL("safe channel %d"),
			       safe_channels[safe_channel_count]);
			safe_channel_count++;
		}
	}
	hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
	       FL("perferred range %d - %d"),
		ap_adapter->sessionCtx.ap.sapConfig.acs_cfg.start_ch,
		ap_adapter->sessionCtx.ap.sapConfig.acs_cfg.end_ch);
	for (i = 0; i < safe_channel_count; i++) {
		if (safe_channels[i] >=
			ap_adapter->sessionCtx.ap.sapConfig.acs_cfg.start_ch
		    && safe_channels[i] <=
			ap_adapter->sessionCtx.ap.sapConfig.acs_cfg.end_ch) {
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
			       FL("safe channel %d is in perferred range"),
			       safe_channels[i]);
			return 1;
		}
	}
	return 0;
}

/**
 * hdd_ch_avoid_cb() - Avoid notified channels from FW handler
 * @adapter:	HDD adapter pointer
 * @indParam:	Channel avoid notification parameter
 *
 * Avoid channel notification from FW handler.
 * FW will send un-safe channel list to avoid over wrapping.
 * hostapd should not use notified channel
 *
 * Return: None
 */
static void hdd_ch_avoid_cb(void *hdd_context, void *indi_param)
{
	hdd_adapter_t *hostapd_adapter = NULL;
	hdd_context_t *hdd_ctxt;
	tSirChAvoidIndType *ch_avoid_indi;
	uint8_t range_loop;
	enum channel_enum channel_loop, start_channel_idx = INVALID_CHANNEL,
					end_channel_idx = INVALID_CHANNEL;
	uint16_t start_channel;
	uint16_t end_channel;
	v_CONTEXT_t cds_context;
	static int restart_sap_in_progress;
	tHddAvoidFreqList hdd_avoid_freq_list;
	uint32_t i;

	/* Basic sanity */
	if (!hdd_context || !indi_param) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("Invalid arguments"));
		return;
	}

	hdd_ctxt = (hdd_context_t *) hdd_context;
	ch_avoid_indi = (tSirChAvoidIndType *) indi_param;
	cds_context = hdd_ctxt->pcds_context;

	/* Make unsafe channel list */
	hddLog(QDF_TRACE_LEVEL_INFO,
	       FL("band count %d"),
	       ch_avoid_indi->avoid_range_count);

	/* generate vendor specific event */
	qdf_mem_zero((void *)&hdd_avoid_freq_list, sizeof(tHddAvoidFreqList));
	for (i = 0; i < ch_avoid_indi->avoid_range_count; i++) {
		hdd_avoid_freq_list.avoidFreqRange[i].startFreq =
			ch_avoid_indi->avoid_freq_range[i].start_freq;
		hdd_avoid_freq_list.avoidFreqRange[i].endFreq =
			ch_avoid_indi->avoid_freq_range[i].end_freq;
	}
	hdd_avoid_freq_list.avoidFreqRangeCount =
		ch_avoid_indi->avoid_range_count;

	wlan_hdd_send_avoid_freq_event(hdd_ctxt, &hdd_avoid_freq_list);

	/* clear existing unsafe channel cache */
	hdd_ctxt->unsafe_channel_count = 0;
	qdf_mem_zero(hdd_ctxt->unsafe_channel_list,
					sizeof(hdd_ctxt->unsafe_channel_list));

	for (range_loop = 0; range_loop < ch_avoid_indi->avoid_range_count;
								range_loop++) {
		if (hdd_ctxt->unsafe_channel_count >= NUM_CHANNELS) {
			hddLog(LOGW, FL("LTE Coex unsafe channel list full"));
			break;
		}

		start_channel = ieee80211_frequency_to_channel(
			ch_avoid_indi->avoid_freq_range[range_loop].start_freq);
		end_channel   = ieee80211_frequency_to_channel(
			ch_avoid_indi->avoid_freq_range[range_loop].end_freq);
		hddLog(LOG1, "%s : start %d : %d, end %d : %d", __func__,
			ch_avoid_indi->avoid_freq_range[range_loop].start_freq,
			start_channel,
			ch_avoid_indi->avoid_freq_range[range_loop].end_freq,
			end_channel);

		/* do not process frequency bands that are not mapped to
		 * predefined channels
		 */
		if (start_channel == 0 || end_channel == 0)
			continue;

		for (channel_loop = CHAN_ENUM_1; channel_loop <=
					CHAN_ENUM_184; channel_loop++) {
			if (CDS_CHANNEL_FREQ(channel_loop) >=
						ch_avoid_indi->avoid_freq_range[
						range_loop].start_freq) {
				start_channel_idx = channel_loop;
				break;
			}
		}
		for (channel_loop = CHAN_ENUM_1; channel_loop <=
					CHAN_ENUM_184; channel_loop++) {
			if (CDS_CHANNEL_FREQ(channel_loop) >=
						ch_avoid_indi->avoid_freq_range[
						range_loop].end_freq) {
				end_channel_idx = channel_loop;
				if (CDS_CHANNEL_FREQ(channel_loop) >
						ch_avoid_indi->avoid_freq_range[
						range_loop].end_freq)
					end_channel_idx--;
				break;
			}
		}

		if (start_channel_idx == INVALID_CHANNEL ||
					end_channel_idx == INVALID_CHANNEL)
			continue;

		for (channel_loop = start_channel_idx; channel_loop <=
					end_channel_idx; channel_loop++) {
			hdd_ctxt->unsafe_channel_list[
				hdd_ctxt->unsafe_channel_count++] =
				CDS_CHANNEL_FREQ(channel_loop);
			if (hdd_ctxt->unsafe_channel_count >=
							NUM_CHANNELS) {
				hddLog(LOGW, FL("LTECoex unsafe ch list full"));
				break;
			}
		}
	}

	hddLog(QDF_TRACE_LEVEL_INFO,
	       FL("number of unsafe channels is %d "),
	       hdd_ctxt->unsafe_channel_count);

	if (cnss_set_wlan_unsafe_channel(hdd_ctxt->unsafe_channel_list,
				hdd_ctxt->unsafe_channel_count)) {
		hdd_err("Failed to set unsafe channel");

		/* clear existing unsafe channel cache */
		hdd_ctxt->unsafe_channel_count = 0;
		qdf_mem_zero(hdd_ctxt->unsafe_channel_list,
			sizeof(hdd_ctxt->unsafe_channel_list));

		return;
	}

	for (channel_loop = 0;
	     channel_loop < hdd_ctxt->unsafe_channel_count; channel_loop++) {
		hddLog(QDF_TRACE_LEVEL_INFO,
		       FL("channel %d is not safe "),
		       hdd_ctxt->unsafe_channel_list[channel_loop]);
	}

	/*
	 * If auto channel select is enabled
	 * preferred channel is in safe channel,
	 * re-start softap interface with safe channel.
	 * no overlap with preferred channel and safe channel
	 * do not re-start softap interface
	 * stay current operating channel.
	 */
	if (hdd_ctxt->unsafe_channel_count) {
		hostapd_adapter = hdd_get_adapter(hdd_ctxt, QDF_SAP_MODE);
		if (hostapd_adapter) {
			if ((hostapd_adapter->sessionCtx.ap.sapConfig.
				acs_cfg.acs_mode) &&
				(!hdd_find_prefd_safe_chnl(hdd_ctxt,
				hostapd_adapter)))
				return;

			hddLog(QDF_TRACE_LEVEL_INFO,
			       FL(
				  "Current operation channel %d, sessionCtx.ap.sapConfig.channel %d"
				 ),
			       hostapd_adapter->sessionCtx.ap.
			       operatingChannel,
			       hostapd_adapter->sessionCtx.ap.sapConfig.
			       channel);
			for (channel_loop = 0;
			     channel_loop < hdd_ctxt->unsafe_channel_count;
			     channel_loop++) {
				if (((hdd_ctxt->
					unsafe_channel_list[channel_loop] ==
						hostapd_adapter->sessionCtx.ap.
						      operatingChannel)) &&
					(hostapd_adapter->sessionCtx.ap.
						sapConfig.acs_cfg.acs_mode
								 == true) &&
					!restart_sap_in_progress) {
					hddLog(QDF_TRACE_LEVEL_INFO,
					       FL("Restarting SAP"));
					wlan_hdd_send_svc_nlink_msg
						(WLAN_SVC_LTE_COEX_IND, NULL, 0);
					restart_sap_in_progress = 1;
					/*
					 * current operating channel is un-safe
					 * channel, restart driver
					 */
					hdd_hostapd_stop(hostapd_adapter->dev);
					break;
				}
			}
		}
	}
	return;
}

/**
 * hdd_init_channel_avoidance() - Initialize channel avoidance
 * @hdd_ctx:	HDD global context
 *
 * Initialize the channel avoidance logic by retrieving the unsafe
 * channel list from the CNSS platform driver and plumbing the data
 * down to the lower layers.  Then subscribe to subsequent channel
 * avoidance events.
 *
 * Return: None
 */
static void hdd_init_channel_avoidance(hdd_context_t *hdd_ctx)
{
	uint16_t unsafe_channel_count;
	int index;

	cnss_get_wlan_unsafe_channel(hdd_ctx->unsafe_channel_list,
				     &(hdd_ctx->unsafe_channel_count),
				     sizeof(uint16_t) * NUM_CHANNELS);

	hddLog(QDF_TRACE_LEVEL_INFO, FL("num of unsafe channels is %d"),
	       hdd_ctx->unsafe_channel_count);

	unsafe_channel_count = QDF_MIN((uint16_t)hdd_ctx->unsafe_channel_count,
				       (uint16_t)NUM_CHANNELS);

	for (index = 0; index < unsafe_channel_count; index++) {
		hddLog(QDF_TRACE_LEVEL_INFO, FL("channel %d is not safe"),
		       hdd_ctx->unsafe_channel_list[index]);

	}

	/* Plug in avoid channel notification callback */
	sme_add_ch_avoid_callback(hdd_ctx->hHal, hdd_ch_avoid_cb);
}
#else
static void hdd_init_channel_avoidance(hdd_context_t *hdd_ctx)
{
}
static void hdd_set_thermal_level_cb(void *context, u_int8_t level)
{
}
#endif /* defined(FEATURE_WLAN_CH_AVOID) && defined(CONFIG_CNSS) */

/**
 * hdd_indicate_mgmt_frame() - Wrapper to indicate management frame to
 * user space
 * @frame_ind: Management frame data to be informed.
 *
 * This function is used to indicate management frame to
 * user space
 *
 * Return: None
 *
 */
void hdd_indicate_mgmt_frame(tSirSmeMgmtFrameInd *frame_ind)
{
	hdd_context_t *hdd_ctx = NULL;
	hdd_adapter_t *adapter = NULL;
	void *cds_context = NULL;
	int i;

	/* Get the global VOSS context.*/
	cds_context = cds_get_global_context();
	if (!cds_context) {
		hdd_err("Global CDS context is Null");
		return;
	}
	/* Get the HDD context.*/
	hdd_ctx = (hdd_context_t *)cds_get_context(QDF_MODULE_ID_HDD);

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	if (SME_SESSION_ID_ANY == frame_ind->sessionId) {
		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
			adapter =
				hdd_get_adapter_by_sme_session_id(hdd_ctx, i);
			if (adapter)
				break;
		}
	} else {
		adapter = hdd_get_adapter_by_sme_session_id(hdd_ctx,
					frame_ind->sessionId);
	}

	if ((NULL != adapter) &&
		(WLAN_HDD_ADAPTER_MAGIC == adapter->magic))
		__hdd_indicate_mgmt_frame(adapter,
						frame_ind->frame_len,
						frame_ind->frameBuf,
						frame_ind->frameType,
						frame_ind->rxChan,
						frame_ind->rxRssi);
	return;
}

/**
 * wlan_hdd_disable_all_dual_mac_features() - Disable dual mac features
 * @hdd_ctx: HDD context
 *
 * Disables all the dual mac features like DBS, Agile DFS etc.
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS wlan_hdd_disable_all_dual_mac_features(hdd_context_t *hdd_ctx)
{
	struct sir_dual_mac_config cfg;
	QDF_STATUS status;

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	cfg.scan_config = 0;
	cfg.fw_mode_config = 0;
	cfg.set_dual_mac_cb =
		(void *)cds_soc_set_dual_mac_cfg_cb;

	hdd_debug("Disabling all dual mac features...");

	status = sme_soc_set_dual_mac_config(hdd_ctx->hHal, cfg);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("sme_soc_set_dual_mac_config failed %d", status);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_override_ini_config - Override INI config
 * @hdd_ctx: HDD context
 *
 * Override INI config based on module parameter.
 *
 * Return: None
 */
static void hdd_override_ini_config(hdd_context_t *hdd_ctx)
{

	if (0 == enable_dfs_chan_scan || 1 == enable_dfs_chan_scan) {
		hdd_ctx->config->enableDFSChnlScan = enable_dfs_chan_scan;
		hdd_notice("Module enable_dfs_chan_scan set to %d",
			   enable_dfs_chan_scan);
	}
	if (0 == enable_11d || 1 == enable_11d) {
		hdd_ctx->config->Is11dSupportEnabled = enable_11d;
		hdd_notice("Module enable_11d set to %d", enable_11d);
	}
}

/**
 * hdd_set_trace_level_for_each - Set trace level for each INI config
 * @hdd_ctx - HDD context
 *
 * Set trace level for each module based on INI config.
 *
 * Return: None
 */
static void hdd_set_trace_level_for_each(hdd_context_t *hdd_ctx)
{
	hdd_qdf_trace_enable(QDF_MODULE_ID_WMI,
			     hdd_ctx->config->qdf_trace_enable_wdi);
	hdd_qdf_trace_enable(QDF_MODULE_ID_HDD,
			     hdd_ctx->config->qdf_trace_enable_hdd);
	hdd_qdf_trace_enable(QDF_MODULE_ID_SME,
			     hdd_ctx->config->qdf_trace_enable_sme);
	hdd_qdf_trace_enable(QDF_MODULE_ID_PE,
			     hdd_ctx->config->qdf_trace_enable_pe);
	hdd_qdf_trace_enable(QDF_MODULE_ID_WMA,
			     hdd_ctx->config->qdf_trace_enable_wma);
	hdd_qdf_trace_enable(QDF_MODULE_ID_SYS,
			     hdd_ctx->config->qdf_trace_enable_sys);
	hdd_qdf_trace_enable(QDF_MODULE_ID_QDF,
			     hdd_ctx->config->qdf_trace_enable_qdf);
	hdd_qdf_trace_enable(QDF_MODULE_ID_SAP,
			     hdd_ctx->config->qdf_trace_enable_sap);
	hdd_qdf_trace_enable(QDF_MODULE_ID_HDD_SOFTAP,
			     hdd_ctx->config->qdf_trace_enable_hdd_sap);
	hdd_qdf_trace_enable(QDF_MODULE_ID_BMI,
				hdd_ctx->config->qdf_trace_enable_bmi);
	hdd_qdf_trace_enable(QDF_MODULE_ID_CFG,
				hdd_ctx->config->qdf_trace_enable_cfg);
	hdd_qdf_trace_enable(QDF_MODULE_ID_EPPING,
				hdd_ctx->config->qdf_trace_enable_epping);
	hdd_qdf_trace_enable(QDF_MODULE_ID_QDF_DEVICE,
				hdd_ctx->config->qdf_trace_enable_qdf_devices);
	hdd_qdf_trace_enable(QDF_MODULE_ID_TXRX,
				hdd_ctx->config->cfd_trace_enable_txrx);
	hdd_qdf_trace_enable(QDF_MODULE_ID_HTC,
				hdd_ctx->config->qdf_trace_enable_htc);
	hdd_qdf_trace_enable(QDF_MODULE_ID_HIF,
				hdd_ctx->config->qdf_trace_enable_hif);
	hdd_qdf_trace_enable(QDF_MODULE_ID_HDD_SAP_DATA,
				hdd_ctx->config->qdf_trace_enable_hdd_sap_data);
	hdd_qdf_trace_enable(QDF_MODULE_ID_HDD_DATA,
				hdd_ctx->config->qdf_trace_enable_hdd_data);

	hdd_cfg_print(hdd_ctx);
}

/**
 * hdd_context_init() - Initialize HDD context
 * @hdd_ctx:	HDD context.
 *
 * Initialize HDD context along with all the feature specific contexts.
 *
 * return: 0 on success and errno on failure.
 */
static int hdd_context_init(hdd_context_t *hdd_ctx)
{
	int ret;

	hdd_ctx->ioctl_scan_mode = eSIR_ACTIVE_SCAN;
	hdd_ctx->max_intf_count = CSR_ROAM_SESSION_MAX;

	hdd_init_ll_stats_ctx();

	init_completion(&hdd_ctx->mc_sus_event_var);
	init_completion(&hdd_ctx->ready_to_suspend);

	hdd_init_bpf_completion();

	qdf_spinlock_create(&hdd_ctx->connection_status_lock);
	qdf_spinlock_create(&hdd_ctx->sta_update_info_lock);
	qdf_spinlock_create(&hdd_ctx->hdd_adapter_lock);

	qdf_list_create(&hdd_ctx->hddAdapters, MAX_NUMBER_OF_ADAPTERS);

	init_completion(&hdd_ctx->set_antenna_mode_cmpl);

	ret = hdd_scan_context_init(hdd_ctx);
	if (ret)
		goto list_destroy;

	hdd_tdls_context_init(hdd_ctx);

	hdd_rx_wake_lock_create(hdd_ctx);

	ret = hdd_sap_context_init(hdd_ctx);
	if (ret)
		goto scan_destroy;

	ret = hdd_roc_context_init(hdd_ctx);
	if (ret)
		goto sap_destroy;

	wlan_hdd_cfg80211_extscan_init(hdd_ctx);

	hdd_init_offloaded_packets_ctx(hdd_ctx);

	ret = wlan_hdd_cfg80211_init(hdd_ctx->parent_dev, hdd_ctx->wiphy,
				     hdd_ctx->config);
	if (ret)
		goto roc_destroy;

	return 0;

roc_destroy:
	hdd_roc_context_destroy(hdd_ctx);

sap_destroy:
	hdd_sap_context_destroy(hdd_ctx);

scan_destroy:
	hdd_scan_context_destroy(hdd_ctx);
	hdd_rx_wake_lock_destroy(hdd_ctx);
	hdd_tdls_context_destroy(hdd_ctx);

list_destroy:
	qdf_list_destroy(&hdd_ctx->hddAdapters);
	return ret;
}

/**
 * hdd_context_create() - Allocate and inialize HDD context.
 * @dev:	Pointer to the underlying device
 * @hif_sc:	HIF context
 *
 * Allocate and initialize HDD context. HDD context is allocated as part of
 * wiphy allocation and then context is initialized.
 *
 * Return: HDD context on success and ERR_PTR on failure
 */
hdd_context_t *hdd_context_create(struct device *dev, void *hif_sc)
{
	QDF_STATUS status;
	int ret = 0;
	hdd_context_t *hdd_ctx;
	v_CONTEXT_t p_cds_context;
	struct hif_target_info *tgt_info = hif_get_target_info_handle(hif_sc);

	ENTER();

	p_cds_context = cds_get_global_context();
	if (p_cds_context == NULL) {
		hdd_alert("Failed to get CDS global context");
		ret = -EINVAL;
		goto err_out;
	}

	hdd_ctx = hdd_cfg80211_wiphy_alloc(sizeof(hdd_context_t));

	if (hdd_ctx == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	hdd_ctx->pcds_context = p_cds_context;
	hdd_ctx->parent_dev = dev;

	hdd_ctx->config = qdf_mem_malloc(sizeof(struct hdd_config));
	if (hdd_ctx->config == NULL) {
		hdd_alert("Failed to alloc memory for HDD config!");
		ret = -ENOMEM;
		goto err_free_hdd_context;
	}

	/* Read and parse the qcom_cfg.ini file */
	status = hdd_parse_config_ini(hdd_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_alert("Error (status: %d) parsing INI file: %s", status,
			  WLAN_INI_FILE);
		ret = -EINVAL;
		goto err_free_config;
	}

	hdd_ctx->configuredMcastBcastFilter =
		hdd_ctx->config->mcastBcastFilterSetting;

	hdd_notice("Setting configuredMcastBcastFilter: %d",
		   hdd_ctx->config->mcastBcastFilterSetting);

	cds_set_fatal_event(hdd_ctx->config->enable_fatal_event);

	hdd_override_ini_config(hdd_ctx);

	((cds_context_type *) (p_cds_context))->pHDDContext = (void *)hdd_ctx;

	ret = hdd_context_init(hdd_ctx);

	if (ret)
		goto err_free_config;

	hdd_ctx->target_type = tgt_info->target_type;

	icnss_set_fw_debug_mode(hdd_ctx->config->enable_fw_log);

	hdd_enable_fastpath(hdd_ctx->config, hif_sc);

	/* Uses to enabled logging after SSR */
	hdd_ctx->fw_log_settings.enable = hdd_ctx->config->enable_fw_log;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam())
		goto skip_multicast_logging;

	cds_set_multicast_logging(hdd_ctx->config->multicast_host_fw_msgs);

	status = wlan_hdd_init_tx_rx_histogram(hdd_ctx);
	if (status)
		goto err_free_config;

	ret = hdd_logging_sock_activate_svc(hdd_ctx);
	if (ret)
		goto err_free_histogram;


	/*
	 * Update QDF trace levels based upon the code. The multicast
	 * levels of the code need not be set when the logger thread
	 * is not enabled.
	 */
	if (cds_is_multicast_logging())
		wlan_logging_set_log_level();

skip_multicast_logging:
	hdd_set_trace_level_for_each(hdd_ctx);

	return hdd_ctx;

err_free_histogram:
	wlan_hdd_deinit_tx_rx_histogram(hdd_ctx);

err_free_config:
	qdf_mem_free(hdd_ctx->config);

err_free_hdd_context:
	wiphy_free(hdd_ctx->wiphy);

err_out:
	return ERR_PTR(ret);
}

#ifdef WLAN_OPEN_P2P_INTERFACE
/**
 * hdd_open_p2p_interface - Open P2P interface
 * @hdd_ctx:	HDD context
 * @rtnl_held:	True if RTNL lock held
 *
 * Open P2P interface during probe. This function called to open the P2P
 * interface at probe along with STA interface.
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_open_p2p_interface(hdd_context_t *hdd_ctx, bool rtnl_held)
{
	hdd_adapter_t *adapter;
	uint8_t *p2p_dev_addr;

	if (hdd_ctx->config->isP2pDeviceAddrAdministrated &&
	    !(hdd_ctx->config->intfMacAddr[0].bytes[0] & 0x02)) {
		qdf_mem_copy(hdd_ctx->p2pDeviceAddress.bytes,
			     hdd_ctx->config->intfMacAddr[0].bytes,
			     sizeof(tSirMacAddr));

		/*
		 * Generate the P2P Device Address.  This consists of
		 * the device's primary MAC address with the locally
		 * administered bit set.
		 */
		hdd_ctx->p2pDeviceAddress.bytes[0] |= 0x02;
	} else {
		p2p_dev_addr = wlan_hdd_get_intf_addr(hdd_ctx);
		if (p2p_dev_addr == NULL) {
			hdd_alert("Failed to allocate mac_address for p2p_device");
			return -ENOSPC;
		}

		qdf_mem_copy(&hdd_ctx->p2pDeviceAddress.bytes[0], p2p_dev_addr,
			     QDF_MAC_ADDR_SIZE);
	}

	adapter = hdd_open_adapter(hdd_ctx, QDF_P2P_DEVICE_MODE, "p2p%d",
				   &hdd_ctx->p2pDeviceAddress.bytes[0],
				   NET_NAME_UNKNOWN, rtnl_held);

	if (NULL == adapter) {
		hdd_alert("Failed to do hdd_open_adapter for P2P Device Interface");
		return -ENOSPC;
	}

	return 0;
}
#else
static inline int hdd_open_p2p_interface(struct hdd_context_t *hdd_ctx,
					 bool rtnl_held)
{
	return 0;
}
#endif

/**
 * hdd_open_monitor_interface() - Open monitor mode interface
 * @hdd_ctx: HDD context
 * @rtnl_held: True if RTNL lock is held
 *
 * Return: Primary adapter on success and PTR_ERR on failure
 */
static hdd_adapter_t *hdd_open_monitor_interface(hdd_context_t *hdd_ctx,
						 bool rtnl_held)
{
	return hdd_open_adapter(hdd_ctx, QDF_MONITOR_MODE, "wlan%d",
				wlan_hdd_get_intf_addr(hdd_ctx),
				NET_NAME_UNKNOWN, rtnl_held);
}

/**
 * hdd_open_interfaces - Open all required interfaces
 * hdd_ctx:	HDD context
 * rtnl_held: True if RTNL lock is held
 *
 * Open all the interfaces like STA, P2P and OCB based on the configuration.
 *
 * Return: Primary adapter on success and PTR_ERR on failure
 */
static hdd_adapter_t *hdd_open_interfaces(hdd_context_t *hdd_ctx,
					  bool rtnl_held)
{
	hdd_adapter_t *adapter = NULL;
	hdd_adapter_t *adapter_11p = NULL;
	int ret;

	/* Create only 802.11p interface */
	if (hdd_ctx->config->dot11p_mode == WLAN_HDD_11P_STANDALONE) {
		adapter = hdd_open_adapter(hdd_ctx, QDF_OCB_MODE, "wlanocb%d",
					   wlan_hdd_get_intf_addr(hdd_ctx),
					   NET_NAME_UNKNOWN, rtnl_held);

		if (adapter == NULL)
			return ERR_PTR(-ENOSPC);

		return adapter;
	}

	adapter = hdd_open_adapter(hdd_ctx, QDF_STA_MODE, "wlan%d",
				   wlan_hdd_get_intf_addr(hdd_ctx),
				   NET_NAME_UNKNOWN, rtnl_held);

	if (adapter == NULL)
		return ERR_PTR(-ENOSPC);

	ret = hdd_open_p2p_interface(hdd_ctx, rtnl_held);
	if (ret)
		goto err_close_adapter;

	/* Open 802.11p Interface */
	if (hdd_ctx->config->dot11p_mode == WLAN_HDD_11P_CONCURRENT) {
		adapter_11p = hdd_open_adapter(hdd_ctx, QDF_OCB_MODE,
					       "wlanocb%d",
					       wlan_hdd_get_intf_addr(hdd_ctx),
					       NET_NAME_UNKNOWN, rtnl_held);
		if (adapter_11p == NULL) {
			hdd_err("Failed to open 802.11p interface");
			goto err_close_adapter;
		}
	}

	return adapter;

err_close_adapter:
	hdd_close_all_adapters(hdd_ctx, rtnl_held);
	return ERR_PTR(ret);
}

/**
 * hdd_update_country_code - Update country code
 * @hdd_ctx: HDD context
 * @adapter: Primary adapter context
 *
 * Update country code based on module parameter country_code at SME and wait
 * for the settings to take effect.
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_update_country_code(hdd_context_t *hdd_ctx,
				  hdd_adapter_t *adapter)
{
	QDF_STATUS status;
	int ret = 0;
	unsigned long rc;

	if (country_code == NULL)
		return 0;

	INIT_COMPLETION(adapter->change_country_code);

	status = sme_change_country_code(hdd_ctx->hHal,
					 wlan_hdd_change_country_code_callback,
					 country_code, adapter,
					 hdd_ctx->pcds_context, eSIR_TRUE,
					 eSIR_TRUE);


	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("SME Change Country code from module param fail ret=%d",
			ret);
		return -EINVAL;
	}

	rc = wait_for_completion_timeout(&adapter->change_country_code,
			 msecs_to_jiffies(WLAN_WAIT_TIME_COUNTRY));
	if (!rc) {
		hdd_err("SME while setting country code timed out");
		ret = -ETIMEDOUT;
	}

	return ret;
}

/**
 * hdd_init_thermal_info - Initialize thermal level
 * @hdd_ctx:	HDD context
 *
 * Initialize thermal level at SME layer and set the thermal level callback
 * which would be called when a configured thermal threshold is hit.
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_init_thermal_info(hdd_context_t *hdd_ctx)
{
	tSmeThermalParams thermal_param;
	QDF_STATUS status;

	thermal_param.smeThermalMgmtEnabled =
		hdd_ctx->config->thermalMitigationEnable;
	thermal_param.smeThrottlePeriod = hdd_ctx->config->throttlePeriod;

	thermal_param.sme_throttle_duty_cycle_tbl[0] =
		hdd_ctx->config->throttle_dutycycle_level0;
	thermal_param.sme_throttle_duty_cycle_tbl[1] =
		hdd_ctx->config->throttle_dutycycle_level1;
	thermal_param.sme_throttle_duty_cycle_tbl[2] =
		hdd_ctx->config->throttle_dutycycle_level2;
	thermal_param.sme_throttle_duty_cycle_tbl[3] =
		hdd_ctx->config->throttle_dutycycle_level3;

	thermal_param.smeThermalLevels[0].smeMinTempThreshold =
		hdd_ctx->config->thermalTempMinLevel0;
	thermal_param.smeThermalLevels[0].smeMaxTempThreshold =
		hdd_ctx->config->thermalTempMaxLevel0;
	thermal_param.smeThermalLevels[1].smeMinTempThreshold =
		hdd_ctx->config->thermalTempMinLevel1;
	thermal_param.smeThermalLevels[1].smeMaxTempThreshold =
		hdd_ctx->config->thermalTempMaxLevel1;
	thermal_param.smeThermalLevels[2].smeMinTempThreshold =
		hdd_ctx->config->thermalTempMinLevel2;
	thermal_param.smeThermalLevels[2].smeMaxTempThreshold =
		hdd_ctx->config->thermalTempMaxLevel2;
	thermal_param.smeThermalLevels[3].smeMinTempThreshold =
		hdd_ctx->config->thermalTempMinLevel3;
	thermal_param.smeThermalLevels[3].smeMaxTempThreshold =
		hdd_ctx->config->thermalTempMaxLevel3;

	status = sme_init_thermal_info(hdd_ctx->hHal, thermal_param);

	if (!QDF_IS_STATUS_SUCCESS(status))
		return qdf_status_to_os_return(status);

	sme_add_set_thermal_level_callback(hdd_ctx->hHal,
					   hdd_set_thermal_level_cb);

	return 0;

}

#if defined(CONFIG_HDD_INIT_WITH_RTNL_LOCK)
/**
 * hdd_hold_rtnl_lock - Hold RTNL lock
 *
 * Hold RTNL lock
 *
 * Return: True if held and false otherwise
 */
static inline bool hdd_hold_rtnl_lock(void)
{
	rtnl_lock();
	return true;
}

/**
 * hdd_release_rtnl_lock - Release RTNL lock
 *
 * Release RTNL lock
 *
 * Return: None
 */
static inline void hdd_release_rtnl_lock(void)
{
	rtnl_unlock();
}
#else
static inline bool hdd_hold_rtnl_lock(void) { return false; }
static inline void hdd_release_rtnl_lock(void) { }
#endif

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * hdd_register_for_sap_restart_with_channel_switch() - Register for SAP channel
 * switch without restart
 *
 * Registers callback function to change the operating channel of SAP by using
 * channel switch announcements instead of restarting SAP.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_register_for_sap_restart_with_channel_switch(void)
{
	QDF_STATUS status;

	status = cds_register_sap_restart_channel_switch_cb(
			(void *)hdd_sap_restart_with_channel_switch);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("restart cb registration failed");

	return status;
}
#endif

/**
 * hdd_tsf_init() - Initialize the TSF synchronization interface
 * @hdd_ctx: HDD global context
 *
 * When TSF synchronization via GPIO is supported by the driver and
 * has been enabled in the configuration file, this function plumbs
 * the GPIO value down to firmware via SME.
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_TSF
static void hdd_tsf_init(hdd_context_t *hdd_ctx)
{
	QDF_STATUS status;

	if (hdd_ctx->config->tsf_gpio_pin == TSF_GPIO_PIN_INVALID)
		return;

	status = sme_set_tsf_gpio(hdd_ctx->hHal,
				      hdd_ctx->config->tsf_gpio_pin);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("Set tsf GPIO failed, status: %d", status);
}
#else
static void hdd_tsf_init(hdd_context_t *hdd_ctx)
{
}
#endif

/**
 * hdd_pre_enable_configure() - Configurations prior to cds_enable
 * @hdd_ctx:	HDD context
 *
 * Pre configurations to be done at lower layer before calling cds enable.
 *
 * Return: 0 on success and errno on failure.
 */
static int hdd_pre_enable_configure(hdd_context_t *hdd_ctx)
{
	int ret;
	QDF_STATUS status;
	tSirRetStatus hal_status;

	ol_txrx_register_pause_cb(wlan_hdd_txrx_pause_cb);

	/*
	 * Set 802.11p config
	 * TODO-OCB: This has been temporarily added here to ensure this
	 * parameter is set in CSR when we init the channel list. This should
	 * be removed once the 5.9 GHz channels are added to the regulatory
	 * domain.
	 */
	hdd_set_dot11p_config(hdd_ctx);

	/*
	 * Note that the cds_pre_enable() sequence triggers the cfg download.
	 * The cfg download must occur before we update the SME config
	 * since the SME config operation must access the cfg database
	 */
	status = hdd_set_sme_config(hdd_ctx);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_alert("Failed hdd_set_sme_config: %d", status);
		ret = qdf_status_to_os_return(status);
		goto out;
	}

	ret = wma_cli_set_command(0, WMI_PDEV_PARAM_TX_CHAIN_MASK_1SS,
				  hdd_ctx->config->tx_chain_mask_1ss,
				  PDEV_CMD);
	if (0 != ret) {
		hdd_err("WMI_PDEV_PARAM_TX_CHAIN_MASK_1SS failed %d", ret);
		goto out;
	}

	hdd_program_country_code(hdd_ctx);

	status = hdd_set_sme_chan_list(hdd_ctx);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_alert("Failed to init channel list: %d", status);
		ret = qdf_status_to_os_return(status);
		goto out;
	}

	/* Apply the cfg.ini to cfg.dat */
	if (!hdd_update_config_dat(hdd_ctx)) {
		hdd_alert("config update failed");
		ret = -EINVAL;
		goto out;
	}

	status = hdd_update_mac_config(hdd_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_warn("can't update mac config, using MAC from ini file: %d",
			 status);
	}

	/*
	 * Set the MAC Address Currently this is used by HAL to add self sta.
	 * Remove this once self sta is added as part of session open.
	 */
	hal_status = cfg_set_str(hdd_ctx->hHal, WNI_CFG_STA_ID,
				     hdd_ctx->config->intfMacAddr[0].bytes,
				     sizeof(hdd_ctx->config->intfMacAddr[0]));

	if (!IS_SIR_STATUS_SUCCESS(hal_status)) {
		hdd_err("Failed to set MAC Address. HALStatus is %08d [x%08x]",
			hal_status, hal_status);
		ret = -EINVAL;
		goto out;
	}

	hdd_init_channel_avoidance(hdd_ctx);

out:
	return ret;
}

/**
 * hdd_wlan_startup() - HDD init function
 * @dev:	Pointer to the underlying device
 *
 * This is the driver startup code executed once a WLAN device has been detected
 *
 * Return:  0 for success, < 0 for failure
 */
int hdd_wlan_startup(struct device *dev, void *hif_sc)
{
	QDF_STATUS status;
	hdd_adapter_t *adapter = NULL;
	hdd_context_t *hdd_ctx = NULL;
	int ret;
	tSirTxPowerLimit hddtxlimit;
	bool rtnl_held;

	ENTER();

	if (QDF_IS_EPPING_ENABLED(con_mode)) {
		ret = epping_enable(dev);
		EXIT();
		return ret;
	}

	hdd_ctx = hdd_context_create(dev, hif_sc);

	if (IS_ERR(hdd_ctx))
		return PTR_ERR(hdd_ctx);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		ret = hdd_enable_ftm(hdd_ctx);

		if (ret)
			goto err_hdd_free_context;

		goto success;
	}

	hdd_wlan_green_ap_init(hdd_ctx);

	status = cds_open();
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("cds_open failed"));
		goto err_hdd_free_context;
	}

	wlan_hdd_update_wiphy(hdd_ctx->wiphy, hdd_ctx->config);

	hdd_ctx->hHal = cds_get_context(QDF_MODULE_ID_SME);

	if (NULL == hdd_ctx->hHal) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("HAL context is null"));
		goto err_cds_close;
	}

	status = cds_pre_enable(hdd_ctx->pcds_context);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("cds_pre_enable failed"));
		goto err_cds_close;
	}

	ret = hdd_wiphy_init(hdd_ctx);
	if (ret) {
		hdd_alert("Failed to initialize wiphy: %d", ret);
		goto err_cds_close;
	}

	ret = hdd_pre_enable_configure(hdd_ctx);
	if (ret)
		goto err_wiphy_unregister;

	if (hdd_ipa_init(hdd_ctx) == QDF_STATUS_E_FAILURE)
		goto err_wiphy_unregister;

	/*
	 * Start CDS which starts up the SME/MAC/HAL modules and everything
	 * else
	 */
	status = cds_enable(hdd_ctx->pcds_context);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("cds_enable failed"));
		goto err_ipa_cleanup;
	}

	status = hdd_post_cds_enable_config(hdd_ctx);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("hdd_post_cds_enable_config failed"));
		goto err_cds_disable;
	}

	rtnl_held = hdd_hold_rtnl_lock();

	if (QDF_GLOBAL_MONITOR_MODE == hdd_get_conparam())
		adapter = hdd_open_monitor_interface(hdd_ctx, rtnl_held);
	else
		adapter = hdd_open_interfaces(hdd_ctx, rtnl_held);

	if (IS_ERR(adapter)) {
		ret = PTR_ERR(adapter);
		goto err_cds_disable;
	}

	/*
	 * target hw version/revision would only be retrieved after firmware
	 * donwload
	 */
	hif_get_hw_info(hif_sc, &hdd_ctx->target_hw_version,
			&hdd_ctx->target_hw_revision,
			&hdd_ctx->target_hw_name);

	/* Get the wlan hw/fw version */
	hdd_wlan_get_version(adapter, NULL, NULL);

	ret = hdd_update_country_code(hdd_ctx, adapter);

	if (ret)
		goto err_close_adapter;

	sme_register11d_scan_done_callback(hdd_ctx->hHal, hdd_11d_scan_done);

	sme_register_oem_data_rsp_callback(hdd_ctx->hHal,
					hdd_send_oem_data_rsp_msg);

	/* FW capabilities received, Set the Dot11 mode */
	sme_setdef_dot11mode(hdd_ctx->hHal);

	ret = hdd_init_netlink_services(hdd_ctx);

	if (ret)
		goto err_close_adapter;

	/*
	 * Action frame registered in one adapter which will
	 * applicable to all interfaces
	 */
	wlan_hdd_cfg80211_register_frames(adapter);

	hdd_release_rtnl_lock();
	rtnl_held = false;

	if (hdd_ctx->config->fIsImpsEnabled)
		hdd_set_idle_ps_config(hdd_ctx, true);
	else
		hdd_set_idle_ps_config(hdd_ctx, false);
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	if (hdd_ctx->config->WlanAutoShutdown != 0)
		if (sme_set_auto_shutdown_cb
			    (hdd_ctx->hHal, wlan_hdd_auto_shutdown_cb)
		    != QDF_STATUS_SUCCESS)
			hddLog(LOGE,
			       FL(
				  "Auto shutdown feature could not be enabled"
				 ));
#endif

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	status = qdf_mc_timer_init(&hdd_ctx->skip_acs_scan_timer,
				   QDF_TIMER_TYPE_SW,
				   hdd_skip_acs_scan_timer_handler,
				   (void *)hdd_ctx);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hddLog(LOGE, FL("Failed to init ACS Skip timer"));
#endif

	wlan_hdd_nan_init(hdd_ctx);
	status = cds_init_policy_mgr();
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Policy manager initialization failed");
		goto err_exit_nl_srv;
	}

	ret = hdd_init_thermal_info(hdd_ctx);

	if (ret) {
		hdd_err("Error while initializing thermal information");
		goto err_exit_nl_srv;
	}

	if (0 != hdd_lro_init(hdd_ctx))
		hdd_err("Unable to initialize LRO in fw");

	hddtxlimit.txPower2g = hdd_ctx->config->TxPower2g;
	hddtxlimit.txPower5g = hdd_ctx->config->TxPower5g;
	status = sme_txpower_limit(hdd_ctx->hHal, &hddtxlimit);
	if (QDF_IS_STATUS_SUCCESS(status))
		hdd_err("Error setting txlimit in sme: %d", status);

	hdd_tsf_init(hdd_ctx);
#ifdef MSM_PLATFORM
	spin_lock_init(&hdd_ctx->bus_bw_lock);
	qdf_mc_timer_init(&hdd_ctx->bus_bw_timer,
			  QDF_TIMER_TYPE_SW,
			  hdd_bus_bw_compute_cbk, (void *)hdd_ctx);
#endif

	wlan_hdd_cfg80211_stats_ext_init(hdd_ctx);

	sme_ext_scan_register_callback(hdd_ctx->hHal,
				       wlan_hdd_cfg80211_extscan_callback);

	status = hdd_register_for_sap_restart_with_channel_switch();
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto err_exit_nl_srv;

	sme_set_rssi_threshold_breached_cb(hdd_ctx->hHal,
				hdd_rssi_threshold_breached);

	status = sme_bpf_offload_register_callback(hdd_ctx->hHal,
							hdd_get_bpf_offload_cb);
	if (QDF_IS_STATUS_SUCCESS(status))
		hdd_err("set bpf offload callback failed");

	hdd_cfg80211_link_layer_stats_init(hdd_ctx);
	wlan_hdd_tsf_init(hdd_ctx);
	wlan_hdd_send_all_scan_intf_info(hdd_ctx);
	wlan_hdd_send_version_pkg(hdd_ctx->target_fw_version,
				  hdd_ctx->target_hw_version,
				  hdd_ctx->target_hw_name);

	wlan_hdd_dcc_register_for_dcc_stats_event(hdd_ctx);

	if (hdd_ctx->config->dual_mac_feature_disable) {
		status = wlan_hdd_disable_all_dual_mac_features(hdd_ctx);
		if (status != QDF_STATUS_SUCCESS) {
			hdd_err("Failed to disable dual mac features");
			goto err_exit_nl_srv;
		}
	}

	ret = hdd_register_notifiers(hdd_ctx);
	if (ret)
		goto err_exit_nl_srv;

	memdump_init();

	goto success;

err_exit_nl_srv:
	hdd_exit_netlink_services(hdd_ctx);

	if (!QDF_IS_STATUS_SUCCESS(cds_deinit_policy_mgr())) {
		hdd_err("Failed to deinit policy manager");
		/* Proceed and complete the clean up */
	}

err_close_adapter:
	hdd_release_rtnl_lock();

	hdd_close_all_adapters(hdd_ctx, false);

err_cds_disable:
	cds_disable(hdd_ctx->pcds_context);

err_ipa_cleanup:
	hdd_ipa_cleanup(hdd_ctx);

err_wiphy_unregister:
	wiphy_unregister(hdd_ctx->wiphy);

err_cds_close:
	status = cds_sched_close(hdd_ctx->pcds_context);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       FL("Failed to close CDS Scheduler"));
		QDF_ASSERT(QDF_IS_STATUS_SUCCESS(status));
	}
	cds_close(hdd_ctx->pcds_context);

err_hdd_free_context:
	hdd_context_destroy(hdd_ctx);
	QDF_BUG(1);

	return -EIO;

success:
	EXIT();
	return 0;
}

/**
 * hdd_softap_sta_deauth() - handle deauth req from HDD
 * @adapter:	Pointer to the HDD
 * @enable:	bool value
 *
 * This to take counter measure to handle deauth req from HDD
 *
 * Return: None
 */
QDF_STATUS hdd_softap_sta_deauth(hdd_adapter_t *adapter,
				 struct tagCsrDelStaParams *pDelStaParams)
{
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t p_cds_context = (WLAN_HDD_GET_CTX(adapter))->pcds_context;
#endif
	QDF_STATUS qdf_status = QDF_STATUS_E_FAULT;

	ENTER();

	hddLog(LOG1, FL("hdd_softap_sta_deauth:(%p, false)"),
	       (WLAN_HDD_GET_CTX(adapter))->pcds_context);

	/* Ignore request to deauth bcmc station */
	if (pDelStaParams->peerMacAddr.bytes[0] & 0x1)
		return qdf_status;

#ifdef WLAN_FEATURE_MBSSID
	qdf_status =
		wlansap_deauth_sta(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				   pDelStaParams);
#else
	qdf_status = wlansap_deauth_sta(p_cds_context, pDelStaParams);
#endif

	EXIT();
	return qdf_status;
}

/**
 * hdd_softap_sta_disassoc() - take counter measure to handle deauth req from HDD
 * @adapter:	Pointer to the HDD
 *
 * This to take counter measure to handle deauth req from HDD
 *
 * Return: None
 */
void hdd_softap_sta_disassoc(hdd_adapter_t *adapter,
			     uint8_t *pDestMacAddress)
{
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t p_cds_context = (WLAN_HDD_GET_CTX(adapter))->pcds_context;
#endif

	ENTER();

	hddLog(LOGE, FL("hdd_softap_sta_disassoc:(%p, false)"),
	       (WLAN_HDD_GET_CTX(adapter))->pcds_context);

	/* Ignore request to disassoc bcmc station */
	if (pDestMacAddress[0] & 0x1)
		return;

#ifdef WLAN_FEATURE_MBSSID
	wlansap_disassoc_sta(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
			     pDestMacAddress);
#else
	wlansap_disassoc_sta(p_cds_context, pDestMacAddress);
#endif
}

void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t *adapter,
					      bool enable)
{
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t p_cds_context = (WLAN_HDD_GET_CTX(adapter))->pcds_context;
#endif

	ENTER();

	hddLog(LOGE, FL("hdd_softap_tkip_mic_fail_counter_measure:(%p, false)"),
	       (WLAN_HDD_GET_CTX(adapter))->pcds_context);

#ifdef WLAN_FEATURE_MBSSID
	wlansap_set_counter_measure(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				    (bool) enable);
#else
	wlansap_set_counter_measure(p_cds_context, (bool) enable);
#endif
}

/**
 * hdd_issta_p2p_clientconnected() - check if sta or p2p client is connected
 * @hdd_ctx:	HDD Context
 *
 * API to find if there is any STA or P2P-Client is connected
 *
 * Return: true if connected; false otherwise
 */
QDF_STATUS hdd_issta_p2p_clientconnected(hdd_context_t *hdd_ctx)
{
	return sme_is_sta_p2p_client_connected(hdd_ctx->hHal);
}

#ifdef WLAN_FEATURE_LPSS
int wlan_hdd_gen_wlan_status_pack(struct wlan_status_data *data,
				  hdd_adapter_t *adapter,
				  hdd_station_ctx_t *pHddStaCtx,
				  uint8_t is_on, uint8_t is_connected)
{
	hdd_context_t *hdd_ctx = NULL;
	uint8_t buflen = WLAN_SVC_COUNTRY_CODE_LEN;

	if (!data) {
		hddLog(LOGE, FL("invalid data pointer"));
		return -EINVAL;
	}
	if (!adapter) {
		if (is_on) {
			/* no active interface */
			data->lpss_support = 0;
			data->is_on = is_on;
			return 0;
		}
		hddLog(LOGE, FL("invalid adapter pointer"));
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx->lpss_support && hdd_ctx->config->enable_lpass_support)
		data->lpss_support = 1;
	else
		data->lpss_support = 0;
	data->numChannels = WLAN_SVC_MAX_NUM_CHAN;
	sme_get_cfg_valid_channels(hdd_ctx->hHal, data->channel_list,
				   &data->numChannels);
	sme_get_country_code(hdd_ctx->hHal, data->country_code, &buflen);
	data->is_on = is_on;
	data->vdev_id = adapter->sessionId;
	data->vdev_mode = adapter->device_mode;
	if (pHddStaCtx) {
		data->is_connected = is_connected;
		data->rssi = adapter->rssi;
		data->freq =
			cds_chan_to_freq(pHddStaCtx->conn_info.operationChannel);
		if (WLAN_SVC_MAX_SSID_LEN >=
		    pHddStaCtx->conn_info.SSID.SSID.length) {
			data->ssid_len = pHddStaCtx->conn_info.SSID.SSID.length;
			memcpy(data->ssid,
			       pHddStaCtx->conn_info.SSID.SSID.ssId,
			       pHddStaCtx->conn_info.SSID.SSID.length);
		}
		if (QDF_MAC_ADDR_SIZE >=
		    sizeof(pHddStaCtx->conn_info.bssId))
			memcpy(data->bssid, pHddStaCtx->conn_info.bssId.bytes,
			       QDF_MAC_ADDR_SIZE);
	}
	return 0;
}

int wlan_hdd_gen_wlan_version_pack(struct wlan_version_data *data,
				   uint32_t fw_version,
				   uint32_t chip_id, const char *chip_name)
{
	if (!data) {
		hddLog(LOGE, FL("invalid data pointer"));
		return -EINVAL;
	}

	data->chip_id = chip_id;
	strlcpy(data->chip_name, chip_name, WLAN_SVC_MAX_STR_LEN);
	if (strncmp(chip_name, "Unknown", 7))
		strlcpy(data->chip_from, "Qualcomm", WLAN_SVC_MAX_STR_LEN);
	else
		strlcpy(data->chip_from, "Unknown", WLAN_SVC_MAX_STR_LEN);
	strlcpy(data->host_version, QWLAN_VERSIONSTR, WLAN_SVC_MAX_STR_LEN);
	scnprintf(data->fw_version, WLAN_SVC_MAX_STR_LEN, "%d.%d.%d.%d",
		  (fw_version & 0xf0000000) >> 28,
		  (fw_version & 0xf000000) >> 24,
		  (fw_version & 0xf00000) >> 20, (fw_version & 0x7fff));
	return 0;
}
#endif

/**
 * wlan_hdd_disable_roaming() - disable roaming on all STAs except the input one
 * @adapter:	HDD adapter pointer
 *
 * This function loop through each adapter and disable roaming on each STA
 * device mode except the input adapter.
 *
 * Note: On the input adapter roaming is not enabled yet hence no need to
 *       disable.
 *
 * Return: None
 */
void wlan_hdd_disable_roaming(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_adapter_t *adapterIdx = NULL;
	hdd_adapter_list_node_t *adapterNode = NULL;
	hdd_adapter_list_node_t *pNext = NULL;
	QDF_STATUS status;

	if (hdd_ctx->config->isFastRoamIniFeatureEnabled &&
	    hdd_ctx->config->isRoamOffloadScanEnabled &&
	    QDF_STA_MODE == adapter->device_mode &&
	    cds_is_sta_active_connection_exists()) {
		hddLog(LOG1, FL("Connect received on STA sessionId(%d)"),
		       adapter->sessionId);
		/*
		 * Loop through adapter and disable roaming for each STA device
		 * mode except the input adapter.
		 */
		status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

		while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
			adapterIdx = adapterNode->pAdapter;

			if (QDF_STA_MODE == adapterIdx->device_mode
			    && adapter->sessionId != adapterIdx->sessionId) {
				hddLog(LOG1,
				       FL("Disable Roaming on sessionId(%d)"),
				       adapterIdx->sessionId);
				sme_stop_roaming(WLAN_HDD_GET_HAL_CTX
							 (adapterIdx),
						 adapterIdx->sessionId, 0);
			}

			status = hdd_get_next_adapter(hdd_ctx,
						      adapterNode,
						      &pNext);
			adapterNode = pNext;
		}
	}
}

/**
 * wlan_hdd_enable_roaming() - enable roaming on all STAs except the input one
 * @adapter:	HDD adapter pointer
 *
 * This function loop through each adapter and enable roaming on each STA
 * device mode except the input adapter.
 * Note: On the input adapter no need to enable roaming because link got
 *       disconnected on this.
 *
 * Return: None
 */
void wlan_hdd_enable_roaming(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_adapter_t *adapterIdx = NULL;
	hdd_adapter_list_node_t *adapterNode = NULL;
	hdd_adapter_list_node_t *pNext = NULL;
	QDF_STATUS status;

	if (hdd_ctx->config->isFastRoamIniFeatureEnabled &&
	    hdd_ctx->config->isRoamOffloadScanEnabled &&
	    QDF_STA_MODE == adapter->device_mode &&
	    cds_is_sta_active_connection_exists()) {
		hddLog(LOG1, FL("Disconnect received on STA sessionId(%d)"),
		       adapter->sessionId);
		/*
		 * Loop through adapter and enable roaming for each STA device
		 * mode except the input adapter.
		 */
		status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

		while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
			adapterIdx = adapterNode->pAdapter;

			if (QDF_STA_MODE == adapterIdx->device_mode
			    && adapter->sessionId != adapterIdx->sessionId) {
				hddLog(LOG1,
				       FL("Enabling Roaming on sessionId(%d)"),
				       adapterIdx->sessionId);
				sme_start_roaming(WLAN_HDD_GET_HAL_CTX
							  (adapterIdx),
						  adapterIdx->sessionId,
						  REASON_CONNECT);
			}

			status = hdd_get_next_adapter(hdd_ctx,
						      adapterNode,
						      &pNext);
			adapterNode = pNext;
		}
	}
}

void wlan_hdd_send_svc_nlink_msg(int type, void *data, int len)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *ani_hdr;
	void *nl_data = NULL;
	int flags = GFP_KERNEL;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	skb = alloc_skb(NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD), flags);

	if (skb == NULL) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("alloc_skb failed"));
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_SVC;

	ani_hdr = NLMSG_DATA(nlh);
	ani_hdr->type = type;

	switch (type) {
	case WLAN_SVC_FW_CRASHED_IND:
	case WLAN_SVC_LTE_COEX_IND:
	case WLAN_SVC_WLAN_AUTO_SHUTDOWN_IND:
	case WLAN_SVC_WLAN_AUTO_SHUTDOWN_CANCEL_IND:
		ani_hdr->length = 0;
		nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr)));
		skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr)));
		break;
	case WLAN_SVC_WLAN_STATUS_IND:
	case WLAN_SVC_WLAN_VERSION_IND:
	case WLAN_SVC_DFS_CAC_START_IND:
	case WLAN_SVC_DFS_CAC_END_IND:
	case WLAN_SVC_DFS_RADAR_DETECT_IND:
	case WLAN_SVC_DFS_ALL_CHANNEL_UNAVAIL_IND:
	case WLAN_SVC_WLAN_TP_IND:
	case WLAN_SVC_WLAN_TP_TX_IND:
		ani_hdr->length = len;
		nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + len));
		nl_data = (char *)ani_hdr + sizeof(tAniMsgHdr);
		memcpy(nl_data, data, len);
		skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr) + len));
		break;

	default:
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("WLAN SVC: Attempt to send unknown nlink message %d"),
		       type);
		kfree_skb(skb);
		return;
	}

	nl_srv_bcast(skb);

	return;
}

#ifdef WLAN_FEATURE_LPSS
void wlan_hdd_send_status_pkg(hdd_adapter_t *adapter,
			      hdd_station_ctx_t *pHddStaCtx,
			      uint8_t is_on, uint8_t is_connected)
{
	int ret = 0;
	struct wlan_status_data data;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam())
		return;

	memset(&data, 0, sizeof(struct wlan_status_data));
	if (is_on)
		ret = wlan_hdd_gen_wlan_status_pack(&data, adapter, pHddStaCtx,
						    is_on, is_connected);
	if (!ret)
		wlan_hdd_send_svc_nlink_msg(WLAN_SVC_WLAN_STATUS_IND,
					    &data,
					    sizeof(struct wlan_status_data));
}

void wlan_hdd_send_version_pkg(uint32_t fw_version,
			       uint32_t chip_id, const char *chip_name)
{
	int ret = 0;
	struct wlan_version_data data;
#ifdef CONFIG_CNSS
	struct cnss_platform_cap cap;

	ret = cnss_get_platform_cap(&cap);
	if (ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("platform capability info from CNSS not available"));
		return;
	}

	if (!(cap.cap_flag & CNSS_HAS_UART_ACCESS))
		return;
#endif

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam())
		return;

	memset(&data, 0, sizeof(struct wlan_version_data));
	ret =
		wlan_hdd_gen_wlan_version_pack(&data, fw_version, chip_id,
					       chip_name);
	if (!ret)
		wlan_hdd_send_svc_nlink_msg(WLAN_SVC_WLAN_VERSION_IND,
					    &data,
					    sizeof(struct wlan_version_data));
}

void wlan_hdd_send_all_scan_intf_info(hdd_context_t *hdd_ctx)
{
	hdd_adapter_t *pDataAdapter = NULL;
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	bool scan_intf_found = false;
	QDF_STATUS status;

	if (!hdd_ctx) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("NULL pointer for hdd_ctx"));
		return;
	}

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);
	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		pDataAdapter = adapterNode->pAdapter;
		if (pDataAdapter) {
			if (pDataAdapter->device_mode == QDF_STA_MODE
			    || pDataAdapter->device_mode == QDF_P2P_CLIENT_MODE
			    || pDataAdapter->device_mode ==
			    QDF_P2P_DEVICE_MODE) {
				scan_intf_found = true;
				wlan_hdd_send_status_pkg(pDataAdapter, NULL, 1,
							 0);
			}
		}
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	if (!scan_intf_found)
		wlan_hdd_send_status_pkg(pDataAdapter, NULL, 1, 0);
}
#endif

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
void wlan_hdd_auto_shutdown_cb(void)
{
	hddLog(LOGE, FL("Wlan Idle. Sending Shutdown event.."));
	wlan_hdd_send_svc_nlink_msg(WLAN_SVC_WLAN_AUTO_SHUTDOWN_IND, NULL, 0);
}

void wlan_hdd_auto_shutdown_enable(hdd_context_t *hdd_ctx, bool enable)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;
	bool ap_connected = false, sta_connected = false;
	tHalHandle hal_handle;

	hal_handle = hdd_ctx->hHal;
	if (hal_handle == NULL)
		return;

	if (hdd_ctx->config->WlanAutoShutdown == 0)
		return;

	if (enable == false) {
		if (sme_set_auto_shutdown_timer(hal_handle, 0) !=
							QDF_STATUS_SUCCESS) {
			hddLog(LOGE,
			       FL("Failed to stop wlan auto shutdown timer"));
		}
		wlan_hdd_send_svc_nlink_msg(
			WLAN_SVC_WLAN_AUTO_SHUTDOWN_CANCEL_IND, NULL, 0);
		return;
	}

	/* To enable shutdown timer check conncurrency */
	if (cds_concurrent_open_sessions_running()) {
		status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

		while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
			adapter = adapterNode->pAdapter;
			if (adapter
			    && adapter->device_mode ==
			    QDF_STA_MODE) {
				if (WLAN_HDD_GET_STATION_CTX_PTR(adapter)->
				    conn_info.connState ==
				    eConnectionState_Associated) {
					sta_connected = true;
					break;
				}
			}
			if (adapter
			    && adapter->device_mode == QDF_SAP_MODE) {
				if (WLAN_HDD_GET_AP_CTX_PTR(adapter)->
				    bApActive == true) {
					ap_connected = true;
					break;
				}
			}
			status = hdd_get_next_adapter(hdd_ctx,
						      adapterNode,
						      &pNext);
			adapterNode = pNext;
		}
	}

	if (ap_connected == true || sta_connected == true) {
		hddLog(LOG1,
		       FL("CC Session active. Shutdown timer not enabled"));
		return;
	} else {
		if (sme_set_auto_shutdown_timer(hal_handle,
						hdd_ctx->config->
						WlanAutoShutdown)
		    != QDF_STATUS_SUCCESS)
			hddLog(LOGE,
			       FL("Failed to start wlan auto shutdown timer"));
		else
			hddLog(LOG1,
			       FL("Auto Shutdown timer for %d seconds enabled"),
			       hdd_ctx->config->WlanAutoShutdown);

	}
}
#endif

hdd_adapter_t *hdd_get_con_sap_adapter(hdd_adapter_t *this_sap_adapter,
							bool check_start_bss)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(this_sap_adapter);
	hdd_adapter_t *adapter, *con_sap_adapter;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;

	con_sap_adapter = NULL;

	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);
	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		if (adapter && ((adapter->device_mode == QDF_SAP_MODE) ||
				(adapter->device_mode == QDF_P2P_GO_MODE)) &&
						adapter != this_sap_adapter) {
			if (check_start_bss) {
				if (test_bit(SOFTAP_BSS_STARTED,
						&adapter->event_flags)) {
					con_sap_adapter = adapter;
					break;
				}
			} else {
				con_sap_adapter = adapter;
				break;
			}
		}
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}

	return con_sap_adapter;
}

#ifdef MSM_PLATFORM
void hdd_start_bus_bw_compute_timer(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (QDF_TIMER_STATE_RUNNING ==
	    qdf_mc_timer_get_current_state(&hdd_ctx->bus_bw_timer))
		return;

	qdf_mc_timer_start(&hdd_ctx->bus_bw_timer,
			   hdd_ctx->config->busBandwidthComputeInterval);
}

void hdd_stop_bus_bw_compute_timer(hdd_adapter_t *adapter)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	bool can_stop = true;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (QDF_TIMER_STATE_RUNNING !=
	    qdf_mc_timer_get_current_state(&hdd_ctx->bus_bw_timer)) {
		/* trying to stop timer, when not running is not good */
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("bus band width compute timer is not running"));
		return;
	}

	if (cds_concurrent_open_sessions_running()) {
		status = hdd_get_front_adapter(hdd_ctx, &adapterNode);

		while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
			adapter = adapterNode->pAdapter;
			if (adapter
			    && (adapter->device_mode == QDF_STA_MODE
				|| adapter->device_mode == QDF_P2P_CLIENT_MODE)
			    && WLAN_HDD_GET_STATION_CTX_PTR(adapter)->
			    conn_info.connState ==
			    eConnectionState_Associated) {
				can_stop = false;
				break;
			}
			if (adapter
			    && (adapter->device_mode == QDF_SAP_MODE
				|| adapter->device_mode == QDF_P2P_GO_MODE)
			    && WLAN_HDD_GET_AP_CTX_PTR(adapter)->bApActive ==
			    true) {
				can_stop = false;
				break;
			}
			status = hdd_get_next_adapter(hdd_ctx,
						      adapterNode,
						      &pNext);
			adapterNode = pNext;
		}
	}

	if (can_stop == true)
		qdf_mc_timer_stop(&hdd_ctx->bus_bw_timer);
}
#endif

/**
 * wlan_hdd_check_custom_con_channel_rules() - This function checks the sap's
 *                                            and sta's operating channel.
 * @sta_adapter:  Describe the first argument to foobar.
 * @ap_adapter:   Describe the second argument to foobar.
 * @roam_profile: Roam profile of AP to which STA wants to connect.
 * @concurrent_chnl_same: If both SAP and STA channels are same then
 *                        set this flag to true else false.
 *
 * This function checks the sap's operating channel and sta's operating channel.
 * if both are same then it will return false else it will restart the sap in
 * sta's channel and return true.
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE.
 */
QDF_STATUS wlan_hdd_check_custom_con_channel_rules(hdd_adapter_t *sta_adapter,
						  hdd_adapter_t *ap_adapter,
						  tCsrRoamProfile *roam_profile,
						  tScanResultHandle *scan_cache,
						  bool *concurrent_chnl_same)
{
	hdd_ap_ctx_t *hdd_ap_ctx;
	uint8_t channel_id;
	QDF_STATUS status;
	enum tQDF_ADAPTER_MODE device_mode = ap_adapter->device_mode;
	*concurrent_chnl_same = true;

	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	status =
	 sme_get_ap_channel_from_scan_cache(WLAN_HDD_GET_HAL_CTX(sta_adapter),
					    roam_profile,
					    scan_cache,
					    &channel_id);
	if ((QDF_STATUS_SUCCESS == status)) {
		if ((QDF_SAP_MODE == device_mode) &&
			(channel_id < SIR_11A_CHANNEL_BEGIN)) {
			if (hdd_ap_ctx->operatingChannel != channel_id) {
				*concurrent_chnl_same = false;
				hddLog(QDF_TRACE_LEVEL_INFO_MED,
					FL("channels are different"));
			}
		} else if ((QDF_P2P_GO_MODE == device_mode) &&
				(channel_id >= SIR_11A_CHANNEL_BEGIN)) {
			if (hdd_ap_ctx->operatingChannel != channel_id) {
				*concurrent_chnl_same = false;
				hddLog(QDF_TRACE_LEVEL_INFO_MED,
					FL("channels are different"));
			}
		}
	} else {
		/*
		 * Lets handle worst case scenario here, Scan cache lookup is
		 * failed so we have to stop the SAP to avoid any channel
		 * discrepancy  between SAP's channel and STA's channel.
		 * Return the status as failure so caller function could know
		 * that scan look up is failed.
		 */
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Finding AP from scan cache failed"));
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_MBSSID
/**
 * wlan_hdd_stop_sap() - This function stops bss of SAP.
 * @ap_adapter: SAP adapter
 *
 * This function will process the stopping of sap adapter.
 *
 * Return: None
 */
void wlan_hdd_stop_sap(hdd_adapter_t *ap_adapter)
{
	hdd_ap_ctx_t *hdd_ap_ctx;
	hdd_hostapd_state_t *hostapd_state;
	QDF_STATUS qdf_status;
	hdd_context_t *hdd_ctx;

	if (NULL == ap_adapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("ap_adapter is NULL here"));
		return;
	}

	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);
	if (0 != wlan_hdd_validate_context(hdd_ctx)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("HDD context is not valid"));
		return;
	}
	mutex_lock(&hdd_ctx->sap_lock);
	if (test_bit(SOFTAP_BSS_STARTED, &ap_adapter->event_flags)) {
		wlan_hdd_del_station(ap_adapter);
		hdd_cleanup_actionframe(hdd_ctx, ap_adapter);
		hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(ap_adapter);
		hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
		       FL("Now doing SAP STOPBSS"));
		qdf_event_reset(&hostapd_state->qdf_stop_bss_event);
		if (QDF_STATUS_SUCCESS == wlansap_stop_bss(hdd_ap_ctx->
							sapContext)) {
			qdf_status = qdf_wait_single_event(&hostapd_state->
							   qdf_stop_bss_event,
							   BSS_WAIT_TIMEOUT);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				mutex_unlock(&hdd_ctx->sap_lock);
				hddLog(QDF_TRACE_LEVEL_ERROR,
				       FL("SAP Stop Failed"));
				return;
			}
		}
		clear_bit(SOFTAP_BSS_STARTED, &ap_adapter->event_flags);
		cds_decr_session_set_pcl(ap_adapter->device_mode,
						ap_adapter->sessionId);
		hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
		       FL("SAP Stop Success"));
	} else {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Can't stop ap because its not started"));
	}
	mutex_unlock(&hdd_ctx->sap_lock);
	return;
}

/**
 * wlan_hdd_start_sap() - this function starts bss of SAP.
 * @ap_adapter: SAP adapter
 *
 * This function will process the starting of sap adapter.
 *
 * Return: None
 */
void wlan_hdd_start_sap(hdd_adapter_t *ap_adapter)
{
	hdd_ap_ctx_t *hdd_ap_ctx;
	hdd_hostapd_state_t *hostapd_state;
	QDF_STATUS qdf_status;
	hdd_context_t *hdd_ctx;
	tsap_Config_t *sap_config;

	if (NULL == ap_adapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("ap_adapter is NULL here"));
		return;
	}

	if (QDF_SAP_MODE != ap_adapter->device_mode) {
		hdd_err("SoftAp role has not been enabled");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);
	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(ap_adapter);
	sap_config = &ap_adapter->sessionCtx.ap.sapConfig;

	if (0 != wlan_hdd_validate_context(hdd_ctx)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("HDD context is not valid"));
		return;
	}
	mutex_lock(&hdd_ctx->sap_lock);
	if (test_bit(SOFTAP_BSS_STARTED, &ap_adapter->event_flags))
		goto end;

	if (0 != wlan_hdd_cfg80211_update_apies(ap_adapter)) {
		hddLog(LOGE, FL("SAP Not able to set AP IEs"));
		wlansap_reset_sap_config_add_ie(sap_config, eUPDATE_IE_ALL);
		goto end;
	}

	if (wlansap_start_bss(hdd_ap_ctx->sapContext, hdd_hostapd_sap_event_cb,
			      &hdd_ap_ctx->sapConfig,
			      ap_adapter->dev)
			      != QDF_STATUS_SUCCESS)
		goto end;

	hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
	       FL("Waiting for SAP to start"));
	qdf_status = qdf_wait_single_event(&hostapd_state->qdf_event,
					BSS_WAIT_TIMEOUT);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("SAP Start failed"));
		goto end;
	}
	hddLog(QDF_TRACE_LEVEL_INFO_HIGH, FL("SAP Start Success"));
	set_bit(SOFTAP_BSS_STARTED, &ap_adapter->event_flags);
	cds_incr_active_session(ap_adapter->device_mode,
					ap_adapter->sessionId);
	hostapd_state->bCommit = true;

end:
	mutex_unlock(&hdd_ctx->sap_lock);
	return;
}
#endif

/**
 * wlan_hdd_soc_set_antenna_mode_cb() - Callback for set dual
 * mac scan config
 * @status: Status of set antenna mode
 *
 * Callback on setting the dual mac configuration
 *
 * Return: None
 */
void wlan_hdd_soc_set_antenna_mode_cb(
	enum set_antenna_mode_status status)
{
	hdd_context_t *hdd_ctx;

	hdd_info("Status: %d", status);

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	/* Signal the completion of set dual mac config */
	complete(&hdd_ctx->set_antenna_mode_cmpl);
}

/**
 * hdd_get_fw_version() - Get FW version
 * @hdd_ctx:     pointer to HDD context.
 * @major_spid:  FW version - major spid.
 * @minor_spid:  FW version - minor spid
 * @ssid:        FW version - ssid
 * @crmid:       FW version - crmid
 *
 * This function is called to get the firmware build version stored
 * as part of the HDD context
 *
 * Return:   None
 */
void hdd_get_fw_version(hdd_context_t *hdd_ctx,
			uint32_t *major_spid, uint32_t *minor_spid,
			uint32_t *siid, uint32_t *crmid)
{
	*major_spid = (hdd_ctx->target_fw_version & 0xf0000000) >> 28;
	*minor_spid = (hdd_ctx->target_fw_version & 0xf000000) >> 24;
	*siid = (hdd_ctx->target_fw_version & 0xf00000) >> 20;
	*crmid = hdd_ctx->target_fw_version & 0x7fff;
}

#ifdef QCA_CONFIG_SMP
/**
 * wlan_hdd_get_cpu() - get cpu_index
 *
 * Return: cpu_index
 */
int wlan_hdd_get_cpu(void)
{
	int cpu_index = get_cpu();
	put_cpu();
	return cpu_index;
}
#endif

/**
 * hdd_get_fwpath() - get framework path
 *
 * This function is used to get the string written by
 * userspace to start the wlan driver
 *
 * Return: string
 */
const char *hdd_get_fwpath(void)
{
	return fwpath.string;
}

/**
 * hdd_init() - Initialize Driver
 *
 * This function initilizes CDS global context with the help of cds_init. This
 * has to be the first function called after probe to get a valid global
 * context.
 *
 * Return: 0 for success, errno on failure
 */
int hdd_init(void)
{
	v_CONTEXT_t p_cds_context = NULL;
	int ret = 0;

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	wlan_logging_sock_init_svc();
#endif
	p_cds_context = cds_init();

	if (p_cds_context == NULL) {
		hdd_alert("Failed to allocate CDS context");
		ret = -ENOMEM;
		goto err_out;
	}

	hdd_trace_init();

err_out:
	return ret;
}

/**
 * hdd_deinit() - Deinitialize Driver
 *
 * This function frees CDS global context with the help of cds_deinit. This
 * has to be the last function call in remove callback to free the global
 * context.
 */
void hdd_deinit(void)
{
	cds_deinit();

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	wlan_logging_sock_deinit_svc();
#endif
}

#ifdef QCA_WIFI_3_0_ADRASTEA
#define HDD_WLAN_START_WAIT_TIME (3600 * 1000)
#else
#define HDD_WLAN_START_WAIT_TIME (CDS_WMA_TIMEOUT + 5000)
#endif

/**
 * __hdd_module_init - Module init helper
 *
 * Module init helper function used by both module and static driver.
 *
 * Return: 0 for success, errno on failure
 */
static int __hdd_module_init(void)
{
	int ret = 0;

	pr_info("%s: Loading driver v%s\n", WLAN_MODULE_NAME,
		QWLAN_VERSIONSTR TIMER_MANAGER_STR MEMORY_DEBUG_STR);

	pld_init();

	qdf_wake_lock_create(&wlan_wake_lock, "wlan");

	hdd_set_conparam((uint32_t) con_mode);

	ret = wlan_hdd_register_driver();
	if (ret) {
		pr_err("%s: driver load failure\n", WLAN_MODULE_NAME);
		goto out;
	}

	pr_info("%s: driver loaded\n", WLAN_MODULE_NAME);

	return 0;
out:
	qdf_wake_lock_destroy(&wlan_wake_lock);
	pld_deinit();
	return ret;
}

/**
 * __hdd_module_exit - Module exit helper
 *
 * Module exit helper function used by both module and static driver.
 */
static void __hdd_module_exit(void)
{
	pr_info("%s: Unloading driver v%s\n", WLAN_MODULE_NAME,
		QWLAN_VERSIONSTR);

	wlan_hdd_unregister_driver();

	qdf_wake_lock_destroy(&wlan_wake_lock);

	pld_deinit();

	return;
}

/**
 * hdd_module_init() - Init Function
 *
 * This is the driver entry point (invoked when module is loaded using insmod)
 *
 * Return: 0 for success, non zero for failure
 */
#ifdef MODULE
static int __init hdd_module_init(void)
{
	return __hdd_module_init();
}
#else /* #ifdef MODULE */
static int __init hdd_module_init(void)
{
	/* Driver initialization is delayed to fwpath_changed_handler */
	return 0;
}
#endif /* #ifdef MODULE */

/**
 * hdd_module_exit() - Exit function
 *
 * This is the driver exit point (invoked when module is unloaded using rmmod)
 *
 * Return: None
 */
static void __exit hdd_module_exit(void)
{
	__hdd_module_exit();
}

#ifdef MODULE
static int fwpath_changed_handler(const char *kmessage, struct kernel_param *kp)
{
	return param_set_copystring(kmessage, kp);
}
#else /* #ifdef MODULE */

/**
 * kickstart_driver() - driver entry point
 *
 * This is the driver entry point
 * - delayed driver initialization when driver is statically linked
 * - invoked when module parameter fwpath is modified from userspace to signal
 *   initializing the WLAN driver or when con_mode is modified from userspace
 *   to signal a switch in operating mode
 *
 * Return: 0 for success, non zero for failure
 */
static int kickstart_driver(void)
{
	int ret = 0;

	if (!wlan_hdd_inited) {
		ret = __hdd_module_init();
		wlan_hdd_inited = ret ? 0 : 1;

		return ret;
	}

	__hdd_module_exit();

	msleep(200);

	ret = __hdd_module_init();

	wlan_hdd_inited = ret ? 0 : 1;

	return ret;
}

/**
 * fwpath_changed_handler() - Handler Function
 *
 * Handle changes to the fwpath parameter
 *
 * Return: 0 for success, non zero for failure
 */
static int fwpath_changed_handler(const char *kmessage, struct kernel_param *kp)
{
	int ret;

	ret = param_set_copystring(kmessage, kp);
	if (0 == ret)
		ret = kickstart_driver();
	return ret;
}

#ifdef QCA_WIFI_FTM
/**
 * con_mode_handler() - Handles module param con_mode change
 *
 * Handler function for module param con_mode when it is changed by userspace
 * Dynamically linked - do nothing
 * Statically linked - exit and init driver, as in rmmod and insmod
 *
 * Return -
 */
static int con_mode_handler(const char *kmessage, struct kernel_param *kp)
{
	int ret;

	ret = param_set_int(kmessage, kp);
	if (0 == ret)
		ret = kickstart_driver();
	return ret;
}
#endif /* QCA_WIFI_FTM */
#endif /* #ifdef MODULE */

/**
 * hdd_get_conparam() - driver exit point
 *
 * This is the driver exit point (invoked when module is unloaded using rmmod)
 *
 * Return: enum tQDF_GLOBAL_CON_MODE
 */
enum tQDF_GLOBAL_CON_MODE hdd_get_conparam(void)
{
	return (enum tQDF_GLOBAL_CON_MODE) curr_con_mode;
}

void hdd_set_conparam(uint32_t con_param)
{
	curr_con_mode = con_param;
}

#ifdef WLAN_FEATURE_LPSS
static inline bool hdd_is_lpass_supported(hdd_context_t *hdd_ctx)
{
	return hdd_ctx->config->enable_lpass_support;
}
#else
static inline bool hdd_is_lpass_supported(hdd_context_t *hdd_ctx)
{
	return false;
}
#endif

/**
 * hdd_update_ol_config - API to update ol configuration parameters
 * @hdd_ctx: HDD context
 *
 * Return: void
 */
static void hdd_update_ol_config(hdd_context_t *hdd_ctx)
{
	struct ol_config_info cfg;
	struct ol_context *ol_ctx = cds_get_context(QDF_MODULE_ID_BMI);

	if (!ol_ctx)
		return;

	cfg.enable_self_recovery = hdd_ctx->config->enableSelfRecovery;
	cfg.enable_uart_print = hdd_ctx->config->enablefwprint;
	cfg.enable_fw_log = hdd_ctx->config->enable_fw_log;
	cfg.enable_ramdump_collection = hdd_ctx->config->is_ramdump_enabled;
	cfg.enable_lpass_support = hdd_is_lpass_supported(hdd_ctx);

	ol_init_ini_config(ol_ctx, &cfg);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * hdd_populate_runtime_cfg() - populate runtime configuration
 * @hdd_ctx: hdd context
 * @cfg: pointer to the configuration memory being populated
 *
 * Return: void
 */
static void hdd_populate_runtime_cfg(hdd_context_t *hdd_ctx,
				     struct hif_config_info *cfg)
{
	cfg->enable_runtime_pm = hdd_ctx->config->runtime_pm;
	cfg->runtime_pm_delay = hdd_ctx->config->runtime_pm_delay;
}
#else
static void hdd_populate_runtime_cfg(hdd_context_t *hdd_ctx,
				     struct hif_config_info *cfg)
{
}
#endif

/**
 * hdd_update_hif_config - API to update HIF configuration parameters
 * @hdd_ctx: HDD Context
 *
 * Return: void
 */
static void hdd_update_hif_config(hdd_context_t *hdd_ctx)
{
	struct hif_opaque_softc *scn = cds_get_context(QDF_MODULE_ID_HIF);
	struct hif_config_info cfg;

	if (!scn)
		return;

	cfg.enable_self_recovery = hdd_ctx->config->enableSelfRecovery;
	hdd_populate_runtime_cfg(hdd_ctx, &cfg);
	hif_init_ini_config(scn, &cfg);
}

/**
 * hdd_update_config() - Initialize driver per module ini parameters
 * @hdd_ctx: HDD Context
 *
 * API is used to initialize all driver per module configuration parameters
 * Return: void
 */
void hdd_update_config(hdd_context_t *hdd_ctx)
{
	hdd_update_ol_config(hdd_ctx);
	hdd_update_hif_config(hdd_ctx);
}

/* Register the module init/exit functions */
module_init(hdd_module_init);
module_exit(hdd_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Qualcomm Atheros, Inc.");
MODULE_DESCRIPTION("WLAN HOST DEVICE DRIVER");

#if !defined(MODULE) && defined(QCA_WIFI_FTM)
module_param_call(con_mode, con_mode_handler, param_get_int, &con_mode,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#else
module_param(con_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif

module_param_call(fwpath, fwpath_changed_handler, param_get_string, &fwpath,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

module_param(enable_dfs_chan_scan, int, S_IRUSR | S_IRGRP | S_IROTH);

module_param(enable_11d, int, S_IRUSR | S_IRGRP | S_IROTH);

module_param(country_code, charp, S_IRUSR | S_IRGRP | S_IROTH);
