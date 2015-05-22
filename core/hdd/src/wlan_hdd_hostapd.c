/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
 * DOC:  wlan_hdd_hostapd.c
 *
 * WLAN Host Device Driver implementation
 */

/* Include Files */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <linux/semaphore.h>
#include <linux/compat.h>
#include <cds_api.h>
#include <cds_sched.h>
#include <linux/etherdevice.h>
#include <wlan_hdd_includes.h>
#include <qc_sap_ioctl.h>
#include <wlan_hdd_hostapd.h>
#include <sap_api.h>
#include <sap_internal.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_ioctl.h>
#include <wlan_hdd_stats.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/mmc/sdio_func.h>
#include "wlan_hdd_p2p.h"
#include <wlan_hdd_ipa.h>
#include "cfg_api.h"
#include "wni_cfg.h"
#include "wlan_hdd_misc.h"
#include <cds_utils.h>
#if defined CONFIG_CNSS
#include <net/cnss.h>
#endif

#include "wma.h"
#ifdef DEBUG
#include "wma_api.h"
#endif
#include "wlan_hdd_trace.h"
#include "cdf_types.h"
#include "cdf_trace.h"
#include "wlan_hdd_cfg.h"
#include "cds_concurrency.h"
#include "wlan_hdd_green_ap.h"

#define    IS_UP(_dev) \
	(((_dev)->flags & (IFF_RUNNING|IFF_UP)) == (IFF_RUNNING|IFF_UP))
#define    IS_UP_AUTO(_ic) \
	(IS_UP((_ic)->ic_dev) && (_ic)->ic_roaming == IEEE80211_ROAMING_AUTO)
#define WE_WLAN_VERSION     1
#define WE_GET_STA_INFO_SIZE 30
/* WEXT limitation: MAX allowed buf len for any *
 * IW_PRIV_TYPE_CHAR is 2Kbytes *
 */
#define WE_SAP_MAX_STA_INFO 0x7FF

#define RC_2_RATE_IDX(_rc)        ((_rc) & 0x7)
#define HT_RC_2_STREAMS(_rc)    ((((_rc) & 0x78) >> 3) + 1)
#define RC_2_RATE_IDX_11AC(_rc)        ((_rc) & 0xf)
#define HT_RC_2_STREAMS_11AC(_rc)    ((((_rc) & 0x30) >> 4) + 1)

#define SAP_24GHZ_CH_COUNT (14)
#define ACS_SCAN_EXPIRY_TIMEOUT_S 4

#define DUMP_DP_TRACE       0

/* Function definitions */

/**
 * hdd_hostapd_channel_wakelock_init() - init the channel wakelock
 * @pHddCtx: pointer to hdd context
 *
 * Return: None
 */
void hdd_hostapd_channel_wakelock_init(hdd_context_t *pHddCtx)
{
	/* Initialize the wakelock */
	cdf_wake_lock_init(&pHddCtx->sap_dfs_wakelock, "sap_dfs_wakelock");
	atomic_set(&pHddCtx->sap_dfs_ref_cnt, 0);
}

/**
 * hdd_hostapd_channel_allow_suspend() - allow suspend in a channel.
 * Called when, 1. bss stopped, 2. channel switch
 *
 * @pAdapter: pointer to hdd adapter
 * @channel: current channel
 *
 * Return: None
 */
void hdd_hostapd_channel_allow_suspend(hdd_adapter_t *pAdapter,
				       uint8_t channel)
{

	hdd_context_t *pHddCtx = (hdd_context_t *) (pAdapter->pHddCtx);
	hdd_hostapd_state_t *pHostapdState =
		WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

	hddLog(LOG1, FL("bssState: %d, channel: %d, dfs_ref_cnt: %d"),
	       pHostapdState->bssState, channel,
	       atomic_read(&pHddCtx->sap_dfs_ref_cnt));

	/* Return if BSS is already stopped */
	if (pHostapdState->bssState == BSS_STOP)
		return;

	/* Release wakelock when no more DFS channels are used */
	if (CHANNEL_STATE_DFS == cds_get_channel_state(channel)) {
		if (atomic_dec_and_test(&pHddCtx->sap_dfs_ref_cnt)) {
			hddLog(LOGE, FL("DFS: allowing suspend (chan %d)"),
			       channel);
			cdf_wake_lock_release(&pHddCtx->sap_dfs_wakelock,
					      WIFI_POWER_EVENT_WAKELOCK_DFS);
		}
	}
}

/**
 * hdd_hostapd_channel_prevent_suspend() - prevent suspend in a channel.
 * Called when, 1. bss started, 2. channel switch
 *
 * @pAdapter: pointer to hdd adapter
 * @channel: current channel
 *
 * Return - None
 */
void hdd_hostapd_channel_prevent_suspend(hdd_adapter_t *pAdapter,
					 uint8_t channel)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) (pAdapter->pHddCtx);
	hdd_hostapd_state_t *pHostapdState =
		WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

	hddLog(LOG1, FL("bssState: %d, channel: %d, dfs_ref_cnt: %d"),
	       pHostapdState->bssState, channel,
	       atomic_read(&pHddCtx->sap_dfs_ref_cnt));

	/* Return if BSS is already started && wakelock is acquired */
	if ((pHostapdState->bssState == BSS_START) &&
		(atomic_read(&pHddCtx->sap_dfs_ref_cnt) >= 1))
		return;

	/* Acquire wakelock if we have at least one DFS channel in use */
	if (CHANNEL_STATE_DFS == cds_get_channel_state(channel)) {
		if (atomic_inc_return(&pHddCtx->sap_dfs_ref_cnt) == 1) {
			hddLog(LOGE, FL("DFS: preventing suspend (chan %d)"),
			       channel);
			cdf_wake_lock_acquire(&pHddCtx->sap_dfs_wakelock,
					      WIFI_POWER_EVENT_WAKELOCK_DFS);
		}
	}
}

/**
 * hdd_hostapd_channel_wakelock_deinit() - destroy the channel wakelock
 *
 * @pHddCtx: pointer to hdd context
 *
 * Return: None
 */
void hdd_hostapd_channel_wakelock_deinit(hdd_context_t *pHddCtx)
{
	if (atomic_read(&pHddCtx->sap_dfs_ref_cnt)) {
		/* Release wakelock */
		cdf_wake_lock_release(&pHddCtx->sap_dfs_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_DRIVER_EXIT);
		/* Reset the reference count */
		atomic_set(&pHddCtx->sap_dfs_ref_cnt, 0);
		hddLog(LOGE, FL("DFS: allowing suspend"));
	}

	/* Destroy lock */
	cdf_wake_lock_destroy(&pHddCtx->sap_dfs_wakelock);
}

/**
 * __hdd_hostapd_open() - hdd open function for hostapd interface
 * This is called in response to ifconfig up
 * @dev: pointer to net_device structure
 *
 * Return - 0 for success non-zero for failure
 */
static int __hdd_hostapd_open(struct net_device *dev)
{
	hdd_adapter_t *pAdapter = netdev_priv(dev);

	ENTER();

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_HOSTAPD_OPEN_REQUEST, NO_SESSION, 0));

	if (WLAN_HDD_GET_CTX(pAdapter)->isLoadInProgress ||
		WLAN_HDD_GET_CTX(pAdapter)->isUnloadInProgress) {
		hddLog(LOGE,
		       FL("Driver load/unload in progress, ignore adapter open"));
		goto done;
	}
	/* Enable all Tx queues */
	hddLog(LOG1, FL("Enabling queues"));
	wlan_hdd_netif_queue_control(pAdapter,
				   WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
				   WLAN_CONTROL_PATH);
done:
	EXIT();
	return 0;
}

/**
 * hdd_hostapd_open() - SSR wrapper for __hdd_hostapd_open
 * @dev: pointer to net device
 *
 * Return: 0 on success, error number otherwise
 */
static int hdd_hostapd_open(struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_hostapd_open(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_hostapd_stop() - hdd stop function for hostapd interface
 * This is called in response to ifconfig down
 *
 * @dev: pointer to net_device structure
 *
 * Return - 0 for success non-zero for failure
 */
static int __hdd_hostapd_stop(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	ENTER();

	/* Stop all tx queues */
	hddLog(LOG1, FL("Disabling queues"));
	wlan_hdd_netif_queue_control(adapter, WLAN_NETIF_TX_DISABLE_N_CARRIER,
				   WLAN_CONTROL_PATH);

	EXIT();
	return 0;
}

/**
 * hdd_hostapd_stop() - SSR wrapper for__hdd_hostapd_stop
 * @dev: pointer to net_device
 *
 * This is called in response to ifconfig down
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_hostapd_stop(struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_hostapd_stop(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_hostapd_uninit() - hdd uninit function
 * This is called during the netdev unregister to uninitialize all data
 * associated with the device.
 *
 * @dev: pointer to net_device structure
 *
 * Return: None
 */
static void __hdd_hostapd_uninit(struct net_device *dev)
{
	hdd_adapter_t *adapter = netdev_priv(dev);
	hdd_context_t *hdd_ctx;

	ENTER();

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hddLog(LOGE, FL("Invalid magic"));
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (NULL == hdd_ctx) {
		hddLog(LOGE, FL("NULL hdd_ctx"));
		return;
	}

	hdd_deinit_adapter(hdd_ctx, adapter, true);

	/* after uninit our adapter structure will no longer be valid */
	adapter->dev = NULL;
	adapter->magic = 0;

	EXIT();
}

/**
 * hdd_hostapd_uninit() - SSR wrapper for __hdd_hostapd_uninit
 * @dev: pointer to net_device
 *
 * Return: 0 on success, error number otherwise
 */
static void hdd_hostapd_uninit(struct net_device *dev)
{
	cds_ssr_protect(__func__);
	__hdd_hostapd_uninit(dev);
	cds_ssr_unprotect(__func__);
}

/**
 * __hdd_hostapd_change_mtu() - change mtu
 * @dev: pointer to net_device
 * @new_mtu: new mtu
 *
 * Return: 0 on success, error number otherwise
 */
static int __hdd_hostapd_change_mtu(struct net_device *dev, int new_mtu)
{
	return 0;
}

/**
 * hdd_hostapd_change_mtu() - SSR wrapper for __hdd_hostapd_change_mtu
 * @dev: pointer to net_device
 * @new_mtu: new mtu
 *
 * Return: 0 on success, error number otherwise
 */
static int hdd_hostapd_change_mtu(struct net_device *dev, int new_mtu)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_hostapd_change_mtu(dev, new_mtu);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef QCA_HT_2040_COEX
CDF_STATUS hdd_set_sap_ht2040_mode(hdd_adapter_t *pHostapdAdapter,
				   uint8_t channel_type)
{
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	void *hHal = NULL;

	hddLog(LOGE, FL("change HT20/40 mode"));

	if (WLAN_HDD_SOFTAP == pHostapdAdapter->device_mode) {
		hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
		if (NULL == hHal) {
			hddLog(LOGE, FL("Hal ctx is null"));
			return CDF_STATUS_E_FAULT;
		}
		cdf_ret_status =
			sme_set_ht2040_mode(hHal, pHostapdAdapter->sessionId,
					channel_type, true);
		if (cdf_ret_status == CDF_STATUS_E_FAILURE) {
			hddLog(LOGE, FL("Failed to change HT20/40 mode"));
			return CDF_STATUS_E_FAILURE;
		}
	}
	return CDF_STATUS_SUCCESS;
}
#endif

/**
 * __hdd_hostapd_set_mac_address() -
 * This function sets the user specified mac address using
 * the command ifconfig wlanX hw ether <mac address>.
 *
 * @dev: pointer to the net device.
 * @addr: pointer to the sockaddr.
 *
 * Return: 0 for success, non zero for failure
 */
static int __hdd_hostapd_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *psta_mac_addr = addr;
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret = 0;

	ENTER();

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	memcpy(dev->dev_addr, psta_mac_addr->sa_data, ETH_ALEN);
	EXIT();
	return 0;
}

/**
 * hdd_hostapd_set_mac_address() - set mac address
 * @dev: pointer to net_device
 * @addr: mac address
 *
 * Return: 0 on success, error number otherwise
 */
static int hdd_hostapd_set_mac_address(struct net_device *dev, void *addr)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_hostapd_set_mac_address(dev, addr);
	cds_ssr_unprotect(__func__);

	return ret;
}

void hdd_hostapd_inactivity_timer_cb(void *usrDataForCallback)
{
	struct net_device *dev = (struct net_device *)usrDataForCallback;
	uint8_t we_custom_event[64];
	union iwreq_data wrqu;
#ifdef DISABLE_CONCURRENCY_AUTOSAVE
	CDF_STATUS cdf_status;
	hdd_adapter_t *pHostapdAdapter;
	hdd_ap_ctx_t *pHddApCtx;
#endif /* DISABLE_CONCURRENCY_AUTOSAVE */

	/* event_name space-delimiter driver_module_name
	 * Format of the event is "AUTO-SHUT.indication" " " "module_name"
	 */
	char *autoShutEvent = "AUTO-SHUT.indication" " " KBUILD_MODNAME;

	/* For the NULL at the end */
	int event_len = strlen(autoShutEvent) + 1;

	ENTER();

#ifdef DISABLE_CONCURRENCY_AUTOSAVE
	if (cds_concurrent_open_sessions_running()) {
		/*
		 * This timer routine is going to be called only when AP
		 * persona is up.
		 * If there are concurrent sessions running we do not want
		 * to shut down the Bss.Instead we run the timer again so
		 * that if Autosave is enabled next time and other session
		   was down only then we bring down AP
		 */
		pHostapdAdapter = netdev_priv(dev);
		pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
		cdf_status =
			cdf_mc_timer_start(&pHddApCtx->hdd_ap_inactivity_timer,
					(WLAN_HDD_GET_CTX(pHostapdAdapter))->
					config->nAPAutoShutOff * 1000);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			hddLog(LOGE, FL("Failed to init AP inactivity timer"));
		}
		EXIT();
		return;
	}
#endif /* DISABLE_CONCURRENCY_AUTOSAVE */
	memset(&we_custom_event, '\0', sizeof(we_custom_event));
	memcpy(&we_custom_event, autoShutEvent, event_len);

	memset(&wrqu, 0, sizeof(wrqu));
	wrqu.data.length = event_len;

	hddLog(LOG1, FL("Shutting down AP interface due to inactivity"));
	wireless_send_event(dev, IWEVCUSTOM, &wrqu, (char *)we_custom_event);

	EXIT();
}

void hdd_clear_all_sta(hdd_adapter_t *pHostapdAdapter,
		       void *usrDataForCallback)
{
	uint8_t staId = 0;
	struct net_device *dev;
	dev = (struct net_device *)usrDataForCallback;

	hddLog(LOGE, FL("Clearing all the STA entry...."));
	for (staId = 0; staId < WLAN_MAX_STA_COUNT; staId++) {
		if (pHostapdAdapter->aStaInfo[staId].isUsed &&
		    (staId !=
		     (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uBCStaId)) {
			/* Disconnect all the stations */
			hdd_softap_sta_disassoc(pHostapdAdapter,
						&pHostapdAdapter->
						aStaInfo[staId].macAddrSTA.
						bytes[0]);
		}
	}
}

static int hdd_stop_bss_link(hdd_adapter_t *pHostapdAdapter,
			     void *usrDataForCallback)
{
	struct net_device *dev;
	hdd_context_t *pHddCtx = NULL;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	dev = (struct net_device *)usrDataForCallback;
	ENTER();

	pHddCtx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	if (test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags)) {
#ifdef WLAN_FEATURE_MBSSID
		status =
			wlansap_stop_bss(WLAN_HDD_GET_SAP_CTX_PTR(
					 pHostapdAdapter));
#else
		status =
			wlansap_stop_bss((WLAN_HDD_GET_CTX(pHostapdAdapter))->
					 pcds_context);
#endif
		if (CDF_IS_STATUS_SUCCESS(status))
			hddLog(LOGE, FL("Deleting SAP/P2P link!!!!!!"));

		clear_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);
		cds_decr_session_set_pcl(pHddCtx,
					     pHostapdAdapter->device_mode,
					     pHostapdAdapter->sessionId);
	}
	EXIT();
	return (status == CDF_STATUS_SUCCESS) ? 0 : -EBUSY;
}

/**
 * hdd_issue_stored_joinreq() - This function will trigger stations's
 *                              cached connect request to proceed.
 * @hdd_ctx: pointer to hdd context.
 * @sta_adater: pointer to station adapter.
 *
 * This function will call SME to release station's stored/cached connect
 * request to proceed.
 *
 * Return: none.
 */
static void hdd_issue_stored_joinreq(hdd_adapter_t *sta_adapter,
		hdd_context_t *hdd_ctx)
{
	tHalHandle hal_handle;
	uint32_t roam_id;

	if (NULL == sta_adapter) {
		hddLog(LOGE,
			FL
			("Invalid station adapter, ignore issueing join req"));
		return;
	}
	hal_handle = WLAN_HDD_GET_HAL_CTX(sta_adapter);

	if (true ==  cds_is_sta_connection_pending(hdd_ctx)) {
		MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
				TRACE_CODE_HDD_ISSUE_JOIN_REQ,
				sta_adapter->sessionId, roam_id));
		if (CDF_STATUS_SUCCESS !=
			sme_issue_stored_joinreq(hal_handle,
				&roam_id,
				sta_adapter->sessionId)) {
			/* change back to NotAssociated */
			hdd_conn_set_connection_state(sta_adapter,
				eConnectionState_NotConnected);
		}
		cds_change_sta_conn_pending_status(hdd_ctx, false);
	}
}

/**
 * hdd_chan_change_notify() - Function to notify hostapd about channel change
 * @hostapd_adapter	hostapd adapter
 * @dev:		Net device structure
 * @oper_chan:		New operating channel
 *
 * This function is used to notify hostapd about the channel change
 *
 * Return: Success on intimating userspace
 *
 */
CDF_STATUS hdd_chan_change_notify(hdd_adapter_t *hostapd_adapter,
		struct net_device *dev,
		uint8_t oper_chan)
{
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	enum nl80211_channel_type channel_type;
	eCsrPhyMode phy_mode;
	ePhyChanBondState cb_mode;
	uint32_t freq;
	tHalHandle  hal = WLAN_HDD_GET_HAL_CTX(hostapd_adapter);

	if (NULL == hal) {
		hdd_err("hal is NULL");
		return CDF_STATUS_E_FAILURE;
	}

	freq = cds_chan_to_freq(oper_chan);

	chan = __ieee80211_get_channel(hostapd_adapter->wdev.wiphy, freq);

	if (!chan) {
		hdd_err("Invalid input frequency for channel conversion");
		return CDF_STATUS_E_FAILURE;
	}

	phy_mode = sme_get_phy_mode(hal);

	if (oper_chan <= 14)
		cb_mode = sme_get_cb_phy_state_from_cb_ini_value(
				sme_get_channel_bonding_mode24_g(hal));
	else
		cb_mode = sme_get_cb_phy_state_from_cb_ini_value(
				sme_get_channel_bonding_mode5_g(hal));

	switch (phy_mode) {
	case eCSR_DOT11_MODE_11n:
	case eCSR_DOT11_MODE_11n_ONLY:
	case eCSR_DOT11_MODE_11ac:
	case eCSR_DOT11_MODE_11ac_ONLY:
		if (cb_mode == PHY_SINGLE_CHANNEL_CENTERED)
			channel_type = NL80211_CHAN_HT20;
		else if (cb_mode == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
			channel_type = NL80211_CHAN_HT40MINUS;
		else if (cb_mode == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
			channel_type = NL80211_CHAN_HT40PLUS;
		else
			channel_type = NL80211_CHAN_HT40PLUS;
		break;
	default:
		channel_type = NL80211_CHAN_NO_HT;
		break;
	}

	cfg80211_chandef_create(&chandef, chan, channel_type);

	cfg80211_ch_switch_notify(dev, &chandef);

	return CDF_STATUS_SUCCESS;
}

/**
 * hdd_send_radar_event() - Function to send radar events to user space
 * @hdd_context:	HDD context
 * @event:		Type of radar event
 * @dfs_info:		Structure containing DFS channel and country
 * @wdev:		Wireless device structure
 *
 * This function is used to send radar events such as CAC start, CAC
 * end etc., to userspace
 *
 * Return: Success on sending notifying userspace
 *
 */
CDF_STATUS hdd_send_radar_event(hdd_context_t *hdd_context,
		eSapHddEvent event,
		struct wlan_dfs_info dfs_info,
		struct wireless_dev *wdev)
{

	struct sk_buff *vendor_event;
	enum qca_nl80211_vendor_subcmds_index index;
	uint32_t freq, ret;
	uint32_t data_size;

	if (!hdd_context) {
		hddLog(LOGE, FL("HDD context is NULL"));
		return CDF_STATUS_E_FAILURE;
	}

	freq = cds_chan_to_freq(dfs_info.channel);

	switch (event) {
	case eSAP_DFS_CAC_START:
		index =
		    QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED_INDEX;
		data_size = sizeof(uint32_t);
		break;
	case eSAP_DFS_CAC_END:
		index =
		    QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED_INDEX;
		data_size = sizeof(uint32_t);
		break;
	case eSAP_DFS_RADAR_DETECT:
		index =
		    QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED_INDEX;
		data_size = sizeof(uint32_t);
		break;
	default:
		return CDF_STATUS_E_FAILURE;
	}

	vendor_event = cfg80211_vendor_event_alloc(hdd_context->wiphy,
			wdev,
			data_size + NLMSG_HDRLEN,
			index,
			GFP_KERNEL);
	if (!vendor_event) {
		hddLog(LOGE,
			FL("cfg80211_vendor_event_alloc failed for %d"), index);
		return CDF_STATUS_E_FAILURE;
	}

	ret = nla_put_u32(vendor_event, NL80211_ATTR_WIPHY_FREQ, freq);

	if (ret) {
		hddLog(LOGE, FL("NL80211_ATTR_WIPHY_FREQ put fail"));
		kfree_skb(vendor_event);
		return CDF_STATUS_E_FAILURE;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	return CDF_STATUS_SUCCESS;
}

CDF_STATUS hdd_hostapd_sap_event_cb(tpSap_Event pSapEvent,
				    void *usrDataForCallback)
{
	hdd_adapter_t *pHostapdAdapter;
	hdd_ap_ctx_t *pHddApCtx;
	hdd_hostapd_state_t *pHostapdState;
	struct net_device *dev;
	eSapHddEvent sapEvent;
	union iwreq_data wrqu;
	uint8_t *we_custom_event_generic = NULL;
	int we_event = 0;
	int i = 0;
	uint8_t staId;
	CDF_STATUS cdf_status;
	bool bWPSState;
	bool bAuthRequired = true;
	tpSap_AssocMacAddr pAssocStasArray = NULL;
	char unknownSTAEvent[IW_CUSTOM_MAX + 1];
	char maxAssocExceededEvent[IW_CUSTOM_MAX + 1];
	uint8_t we_custom_start_event[64];
	char *startBssEvent;
	hdd_context_t *pHddCtx;
	hdd_scaninfo_t *pScanInfo = NULL;
	struct iw_michaelmicfailure msg;
	uint8_t ignoreCAC = 0;
	struct hdd_config *cfg = NULL;
	struct wlan_dfs_info dfs_info;
	uint8_t cc_len = WLAN_SVC_COUNTRY_CODE_LEN;
	hdd_adapter_t *con_sap_adapter;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
#if defined CONFIG_CNSS
	int ret = 0;
#endif

	dev = (struct net_device *)usrDataForCallback;
	if (!dev) {
		hddLog(LOGE, FL("usrDataForCallback is null"));
		return CDF_STATUS_E_FAILURE;
	}

	pHostapdAdapter = netdev_priv(dev);

	if ((NULL == pHostapdAdapter) ||
	    (WLAN_HDD_ADAPTER_MAGIC != pHostapdAdapter->magic)) {
		hddLog(LOGE, "invalid adapter or adapter has invalid magic");
		return CDF_STATUS_E_FAILURE;
	}

	pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);
	pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);

	if (!pSapEvent) {
		hddLog(LOGE, FL("pSapEvent is null"));
		return CDF_STATUS_E_FAILURE;
	}

	sapEvent = pSapEvent->sapHddEventCode;
	memset(&wrqu, '\0', sizeof(wrqu));
	pHddCtx = (hdd_context_t *) (pHostapdAdapter->pHddCtx);

	if (!pHddCtx) {
		hddLog(LOGE, FL("HDD context is null"));
		return CDF_STATUS_E_FAILURE;
	}

	cfg = pHddCtx->config;

	if (!cfg) {
		hddLog(LOGE, FL("HDD config is null"));
		return CDF_STATUS_E_FAILURE;
	}

	dfs_info.channel = pHddApCtx->operatingChannel;
	sme_get_country_code(pHddCtx->hHal, dfs_info.country_code, &cc_len);

	switch (sapEvent) {
	case eSAP_START_BSS_EVENT:
		hddLog(LOG1,
		       FL("BSS status = %s, channel = %u, bc sta Id = %d"),
		       pSapEvent->sapevt.sapStartBssCompleteEvent.
		       status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS",
		       pSapEvent->sapevt.sapStartBssCompleteEvent.
		       operatingChannel,
		       pSapEvent->sapevt.sapStartBssCompleteEvent.staId);

		pHostapdAdapter->sessionId =
			pSapEvent->sapevt.sapStartBssCompleteEvent.sessionId;

		pHostapdState->cdf_status =
			pSapEvent->sapevt.sapStartBssCompleteEvent.status;
		cdf_status = cdf_event_set(&pHostapdState->cdf_event);

		if (!CDF_IS_STATUS_SUCCESS(cdf_status)
		    || pHostapdState->cdf_status) {
			hddLog(LOGE, ("ERROR: startbss event failed!!"));
			goto stopbss;
		} else {
			sme_ch_avoid_update_req(pHddCtx->hHal);

			pHddApCtx->uBCStaId =
				pSapEvent->sapevt.sapStartBssCompleteEvent.staId;

			hdd_register_tx_flow_control(pHostapdAdapter,
				hdd_softap_tx_resume_timer_expired_handler,
				hdd_softap_tx_resume_cb);

			/* @@@ need wep logic here to set privacy bit */
			cdf_status =
				hdd_softap_register_bc_sta(pHostapdAdapter,
							   pHddApCtx->uPrivacy);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
				hddLog(LOGW, FL("Failed to register BC STA %d"),
				       cdf_status);
				hdd_stop_bss_link(pHostapdAdapter,
						  usrDataForCallback);
			}
		}

		if (hdd_ipa_is_enabled(pHddCtx)) {
			status = hdd_ipa_wlan_evt(pHostapdAdapter,
					pHddApCtx->uBCStaId,
					WLAN_AP_CONNECT,
					pHostapdAdapter->dev->dev_addr);
			if (status) {
				hddLog(LOGE,
					("WLAN_AP_CONNECT event failed!!"));
				goto stopbss;
			}
		}

		if (0 !=
		    (WLAN_HDD_GET_CTX(pHostapdAdapter))->config->
		     nAPAutoShutOff) {
			/* AP Inactivity timer init and start */
			cdf_status =
				cdf_mc_timer_init(&pHddApCtx->
						  hdd_ap_inactivity_timer,
						  CDF_TIMER_TYPE_SW,
						  hdd_hostapd_inactivity_timer_cb,
						  dev);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status))
				hddLog(LOGE,
					FL("Failed to init inactivity timer"));

			cdf_status =
				cdf_mc_timer_start(&pHddApCtx->
						   hdd_ap_inactivity_timer,
						   (WLAN_HDD_GET_CTX
						    (pHostapdAdapter))->config->
						    nAPAutoShutOff * 1000);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status))
				hddLog(LOGE,
					FL("Failed to init inactivity timer"));

		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif
		pHddApCtx->operatingChannel =
			pSapEvent->sapevt.sapStartBssCompleteEvent.operatingChannel;

		hdd_hostapd_channel_prevent_suspend(pHostapdAdapter,
						    pHddApCtx->
						    operatingChannel);

		pHostapdState->bssState = BSS_START;
		hdd_wlan_green_ap_start_bss(pHddCtx);

		/* Set group key / WEP key every time when BSS is restarted */
		if (pHddApCtx->groupKey.keyLength) {
			status = wlansap_set_key_sta(
#ifdef WLAN_FEATURE_MBSSID
				WLAN_HDD_GET_SAP_CTX_PTR
					(pHostapdAdapter),
#else
				(WLAN_HDD_GET_CTX
					 (pHostapdAdapter))->
				pcds_context,
#endif
				&pHddApCtx->groupKey);
			if (!CDF_IS_STATUS_SUCCESS(status))
				hddLog(LOGE, FL("wlansap_set_key_sta failed"));
		} else {
			for (i = 0; i < CSR_MAX_NUM_KEY; i++) {
				if (!pHddApCtx->wepKey[i].keyLength)
					continue;

				status = wlansap_set_key_sta(
#ifdef WLAN_FEATURE_MBSSID
					WLAN_HDD_GET_SAP_CTX_PTR
						(pHostapdAdapter),
#else
					(WLAN_HDD_GET_CTX(pHostapdAdapter))->
					 pcds_context,
#endif
					&pHddApCtx->
					wepKey[i]);
				if (!CDF_IS_STATUS_SUCCESS(status)) {
					hddLog(LOGE,
					       FL("set_key failed idx %d"), i);
				}
			}
		}

		mutex_lock(&pHddCtx->dfs_lock);
		pHddCtx->dfs_radar_found = false;
		mutex_unlock(&pHddCtx->dfs_lock);

		wlansap_get_dfs_ignore_cac(pHddCtx->hHal, &ignoreCAC);

		/* DFS requirement: DO NOT transmit during CAC. */
		if ((CHANNEL_STATE_DFS !=
			cds_get_channel_state(pHddApCtx->operatingChannel))
			|| ignoreCAC
			|| pHddCtx->dev_dfs_cac_status == DFS_CAC_ALREADY_DONE)
			pHddApCtx->dfs_cac_block_tx = false;
		else
			pHddApCtx->dfs_cac_block_tx = true;

		hddLog(LOG3, "The value of dfs_cac_block_tx[%d] for ApCtx[%p]",
		       pHddApCtx->dfs_cac_block_tx, pHddApCtx);

		if ((CHANNEL_STATE_DFS ==
		     cds_get_channel_state(pHddApCtx->operatingChannel))
		    && (pHddCtx->config->IsSapDfsChSifsBurstEnabled == 0)) {

			hddLog(LOG1,
			       FL("Set SIFS Burst disable for DFS channel %d"),
			       pHddApCtx->operatingChannel);

			if (wma_cli_set_command(pHostapdAdapter->sessionId,
						WMI_PDEV_PARAM_BURST_ENABLE,
						0, PDEV_CMD)) {
				hddLog(LOGE,
				       FL("Failed to Set SIFS Burst %d"),
				       pHddApCtx->operatingChannel);
			}
		}
		/* Fill the params for sending IWEVCUSTOM Event with SOFTAP.enabled */
		startBssEvent = "SOFTAP.enabled";
		memset(&we_custom_start_event, '\0',
		       sizeof(we_custom_start_event));
		memcpy(&we_custom_start_event, startBssEvent,
		       strlen(startBssEvent));
		memset(&wrqu, 0, sizeof(wrqu));
		wrqu.data.length = strlen(startBssEvent);
		we_event = IWEVCUSTOM;
		we_custom_event_generic = we_custom_start_event;
		cds_dump_concurrency_info(pHddCtx);
		/* Send SCC/MCC Switching event to IPA */
		hdd_ipa_send_mcc_scc_msg(pHddCtx, pHddCtx->mcc_mode);
		break;          /* Event will be sent after Switch-Case stmt */

	case eSAP_STOP_BSS_EVENT:
		hddLog(LOG1, FL("BSS stop status = %s"),
		       pSapEvent->sapevt.sapStopBssCompleteEvent.
		       status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");

		hdd_hostapd_channel_allow_suspend(pHostapdAdapter,
						  pHddApCtx->operatingChannel);

		hdd_wlan_green_ap_stop_bss(pHddCtx);

		/* Free up Channel List incase if it is set */
#ifdef WLAN_FEATURE_MBSSID
		sap_cleanup_channel_list(WLAN_HDD_GET_SAP_CTX_PTR
					 (pHostapdAdapter));
#else
		sap_cleanup_channel_list();
#endif
		/* Invalidate the channel info. */
		pHddApCtx->operatingChannel = 0;
		if (hdd_ipa_is_enabled(pHddCtx)) {
			status = hdd_ipa_wlan_evt(pHostapdAdapter,
					pHddApCtx->uBCStaId,
					WLAN_AP_DISCONNECT,
					pHostapdAdapter->dev->dev_addr);
			if (status) {
				hddLog(LOGE,
				       ("WLAN_AP_DISCONNECT event failed!!"));
				goto stopbss;
			}
		}

		/* reset the dfs_cac_status and dfs_cac_block_tx flag only when
		 * the last BSS is stopped
		 */
		con_sap_adapter = hdd_get_con_sap_adapter(pHostapdAdapter, true);
		if (!con_sap_adapter) {
			pHddApCtx->dfs_cac_block_tx = true;
			pHddCtx->dev_dfs_cac_status = DFS_CAC_NEVER_DONE;
		}
		if (pHddCtx->config->conc_custom_rule2 &&
			(WLAN_HDD_P2P_GO == pHostapdAdapter->device_mode)) {
			hdd_adapter_t *sta_adapter = hdd_get_adapter(pHddCtx,
					WLAN_HDD_INFRA_STATION);
			hddLog(LOG2,
				FL("P2PGO is going down now"));
			hdd_issue_stored_joinreq(sta_adapter, pHddCtx);
		}
		goto stopbss;

	case eSAP_DFS_CAC_START:
		wlan_hdd_send_svc_nlink_msg(WLAN_SVC_DFS_CAC_START_IND,
					    &dfs_info,
					    sizeof(struct wlan_dfs_info));
		pHddCtx->dev_dfs_cac_status = DFS_CAC_IN_PROGRESS;
		if (CDF_STATUS_SUCCESS !=
			hdd_send_radar_event(pHddCtx, eSAP_DFS_CAC_START,
				dfs_info, &pHostapdAdapter->wdev)) {
			hddLog(LOGE, FL("Unable to indicate CAC start NL event"));
		} else {
			hdd_info("Sent CAC start to user space");
		}

		mutex_lock(&pHddCtx->dfs_lock);
		pHddCtx->dfs_radar_found = false;
		mutex_unlock(&pHddCtx->dfs_lock);
		break;
	case eSAP_DFS_CAC_INTERRUPTED:
		/*
		 * The CAC timer did not run completely and a radar was detected
		 * during the CAC time. This new state will keep the tx path
		 * blocked since we do not want any transmission on the DFS
		 * channel. CAC end will only be reported here since the user
		 * space applications are waiting on CAC end for their state
		 * management.
		 */
		if (CDF_STATUS_SUCCESS !=
			hdd_send_radar_event(pHddCtx, eSAP_DFS_CAC_END,
				dfs_info, &pHostapdAdapter->wdev)) {
			hdd_err("Unable to indicate CAC end (interrupted) event");
		} else {
			hdd_info("Sent CAC end (interrupted) to user space");
		}
		break;
	case eSAP_DFS_CAC_END:
		wlan_hdd_send_svc_nlink_msg(WLAN_SVC_DFS_CAC_END_IND,
					    &dfs_info,
					    sizeof(struct wlan_dfs_info));
		pHddApCtx->dfs_cac_block_tx = false;
		pHddCtx->dev_dfs_cac_status = DFS_CAC_ALREADY_DONE;
		if (CDF_STATUS_SUCCESS !=
			hdd_send_radar_event(pHddCtx, eSAP_DFS_CAC_END,
				dfs_info, &pHostapdAdapter->wdev)) {
			hddLog(LOGE, FL("Unable to indicate CAC end NL event"));
		} else {
			hdd_info("Sent CAC end to user space");
		}
		break;

	case eSAP_DFS_RADAR_DETECT:
		wlan_hdd_send_svc_nlink_msg(WLAN_SVC_DFS_RADAR_DETECT_IND,
					    &dfs_info,
					    sizeof(struct wlan_dfs_info));
		pHddCtx->dev_dfs_cac_status = DFS_CAC_NEVER_DONE;
		if (CDF_STATUS_SUCCESS !=
			hdd_send_radar_event(pHddCtx, eSAP_DFS_RADAR_DETECT,
				dfs_info, &pHostapdAdapter->wdev)) {
			hddLog(LOGE, FL("Unable to indicate Radar detect NL event"));
		} else {
			hdd_info("Sent radar detected to user space");
		}
		break;

	case eSAP_DFS_NO_AVAILABLE_CHANNEL:
		wlan_hdd_send_svc_nlink_msg
			(WLAN_SVC_DFS_ALL_CHANNEL_UNAVAIL_IND, &dfs_info,
			sizeof(struct wlan_dfs_info));
		break;

	case eSAP_STA_SET_KEY_EVENT:
		/* TODO:
		 * forward the message to hostapd once implementation
		 * is done for now just print
		 */
		hddLog(LOG1, FL("SET Key: configured status = %s"),
		       pSapEvent->sapevt.sapStationSetKeyCompleteEvent.
		       status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");
		return CDF_STATUS_SUCCESS;
	case eSAP_STA_MIC_FAILURE_EVENT:
	{
		memset(&msg, '\0', sizeof(msg));
		msg.src_addr.sa_family = ARPHRD_ETHER;
		memcpy(msg.src_addr.sa_data,
		       &pSapEvent->sapevt.sapStationMICFailureEvent.
		       staMac, CDF_MAC_ADDR_SIZE);
		hddLog(LOG1, "MIC MAC " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(msg.src_addr.sa_data));
		if (pSapEvent->sapevt.sapStationMICFailureEvent.
		    multicast == eSAP_TRUE)
			msg.flags = IW_MICFAILURE_GROUP;
		else
			msg.flags = IW_MICFAILURE_PAIRWISE;
		memset(&wrqu, 0, sizeof(wrqu));
		wrqu.data.length = sizeof(msg);
		we_event = IWEVMICHAELMICFAILURE;
		we_custom_event_generic = (uint8_t *) &msg;
	}
		/* inform mic failure to nl80211 */
		cfg80211_michael_mic_failure(dev,
					     pSapEvent->
					     sapevt.sapStationMICFailureEvent.
					     staMac.bytes,
					     ((pSapEvent->sapevt.
					       sapStationMICFailureEvent.
					       multicast ==
					       eSAP_TRUE) ?
					      NL80211_KEYTYPE_GROUP :
					      NL80211_KEYTYPE_PAIRWISE),
					     pSapEvent->sapevt.
					     sapStationMICFailureEvent.keyId,
					     pSapEvent->sapevt.
					     sapStationMICFailureEvent.TSC,
					     GFP_KERNEL);
		break;

	case eSAP_STA_ASSOC_EVENT:
	case eSAP_STA_REASSOC_EVENT:
		wrqu.addr.sa_family = ARPHRD_ETHER;
		memcpy(wrqu.addr.sa_data,
		       &pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.
		       staMac, CDF_MAC_ADDR_SIZE);
		hddLog(LOG1, " associated " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(wrqu.addr.sa_data));
		we_event = IWEVREGISTERED;

#ifdef WLAN_FEATURE_MBSSID
		wlansap_get_wps_state(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter),
				      &bWPSState);
#else
		wlansap_get_wps_state((WLAN_HDD_GET_CTX(pHostapdAdapter))->
				      pcds_context, &bWPSState);
#endif

		if ((eCSR_ENCRYPT_TYPE_NONE == pHddApCtx->ucEncryptType) ||
		    (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY ==
		     pHddApCtx->ucEncryptType)
		    || (eCSR_ENCRYPT_TYPE_WEP104_STATICKEY ==
			pHddApCtx->ucEncryptType)) {
			bAuthRequired = false;
		}

		if (bAuthRequired || bWPSState == true) {
			cdf_status = hdd_softap_register_sta(
						pHostapdAdapter,
						true,
						pHddApCtx->uPrivacy,
						pSapEvent->sapevt.
						sapStationAssocReassocCompleteEvent.
						staId, 0, 0,
						(struct cdf_mac_addr *)
						wrqu.addr.sa_data,
						pSapEvent->sapevt.
						sapStationAssocReassocCompleteEvent.
						wmmEnabled);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status))
				hddLog(LOGW,
				       FL("Failed to register STA %d "
					  MAC_ADDRESS_STR ""), cdf_status,
				       MAC_ADDR_ARRAY(wrqu.addr.sa_data));
		} else {
			cdf_status = hdd_softap_register_sta(
						pHostapdAdapter,
						false,
						pHddApCtx->uPrivacy,
						pSapEvent->sapevt.
						sapStationAssocReassocCompleteEvent.
						staId, 0, 0,
						(struct cdf_mac_addr *)
						wrqu.addr.sa_data,
						pSapEvent->sapevt.
						sapStationAssocReassocCompleteEvent.
						wmmEnabled);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status))
				hddLog(LOGW,
				       FL("Failed to register STA %d "
					  MAC_ADDRESS_STR ""), cdf_status,
				       MAC_ADDR_ARRAY(wrqu.addr.sa_data));
		}

		staId =
		   pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.staId;
		if (CDF_IS_STATUS_SUCCESS(cdf_status)) {
			pHostapdAdapter->aStaInfo[staId].nss =
				pSapEvent->sapevt.
				sapStationAssocReassocCompleteEvent.
				chan_info.nss;
			pHostapdAdapter->aStaInfo[staId].rate_flags =
				pSapEvent->sapevt.
				sapStationAssocReassocCompleteEvent.
				chan_info.rate_flags;
		}

		if (hdd_ipa_is_enabled(pHddCtx)) {
			status = hdd_ipa_wlan_evt(pHostapdAdapter,
					pSapEvent->sapevt.
					sapStationAssocReassocCompleteEvent.
					staId, WLAN_CLIENT_CONNECT_EX,
					pSapEvent->sapevt.
					sapStationAssocReassocCompleteEvent.
					staMac.bytes);
			if (status) {
				hddLog(LOGE,
				     FL("WLAN_CLIENT_CONNECT_EX event failed"));
				goto stopbss;
			}
		}

#ifdef QCA_PKT_PROTO_TRACE
		/* Peer associated, update into trace buffer */
		if (pHddCtx->config->gEnableDebugLog) {
			cds_pkt_trace_buf_update("HA:ASSOC");
		}
#endif /* QCA_PKT_PROTO_TRACE */

#ifdef MSM_PLATFORM
		/* start timer in sap/p2p_go */
		if (pHddApCtx->bApActive == false) {
			spin_lock_bh(&pHddCtx->bus_bw_lock);
			pHostapdAdapter->prev_tx_packets =
				pHostapdAdapter->stats.tx_packets;
			pHostapdAdapter->prev_rx_packets =
				pHostapdAdapter->stats.rx_packets;
			spin_unlock_bh(&pHddCtx->bus_bw_lock);
			hdd_start_bus_bw_compute_timer(pHostapdAdapter);
		}
#endif
		pHddApCtx->bApActive = true;
		/* Stop AP inactivity timer */
		if (pHddApCtx->hdd_ap_inactivity_timer.state ==
		    CDF_TIMER_STATE_RUNNING) {
			cdf_status =
				cdf_mc_timer_stop(&pHddApCtx->
						  hdd_ap_inactivity_timer);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
				hddLog(LOGE,
				       FL("Failed to start inactivity timer"));
			}
		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, false);
#endif
		cdf_wake_lock_timeout_acquire(&pHddCtx->sap_wake_lock,
					      HDD_SAP_WAKE_LOCK_DURATION,
					      WIFI_POWER_EVENT_WAKELOCK_SAP);
		{
			struct station_info staInfo;
			uint16_t iesLen =
				pSapEvent->sapevt.
				sapStationAssocReassocCompleteEvent.iesLen;

			memset(&staInfo, 0, sizeof(staInfo));
			if (iesLen <= MAX_ASSOC_IND_IE_LEN) {
				staInfo.assoc_req_ies =
					(const u8 *)&pSapEvent->sapevt.
					sapStationAssocReassocCompleteEvent.ies[0];
				staInfo.assoc_req_ies_len = iesLen;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 31))
				staInfo.filled |= STATION_INFO_ASSOC_REQ_IES;
#endif
				cfg80211_new_sta(dev,
						 (const u8 *)&pSapEvent->sapevt.
						 sapStationAssocReassocCompleteEvent.
						 staMac.bytes[0], &staInfo,
						 GFP_KERNEL);
			} else {
				hddLog(LOGE,
				       FL("Assoc Ie length is too long"));
			}
		}

		pScanInfo = &pHostapdAdapter->scan_info;
		/* Lets do abort scan to ensure smooth authentication for client */
		if ((pScanInfo != NULL) && pScanInfo->mScanPending) {
			hdd_abort_mac_scan(pHddCtx, pHostapdAdapter->sessionId,
					   eCSR_SCAN_ABORT_DEFAULT);
		}
		if (pHostapdAdapter->device_mode == WLAN_HDD_P2P_GO) {
			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_oem_app(&pSapEvent->sapevt.
							    sapStationAssocReassocCompleteEvent.
							    staMac, ePeerConnected,
							    pSapEvent->sapevt.
							    sapStationAssocReassocCompleteEvent.
							    timingMeasCap,
							    pHostapdAdapter->
							    sessionId,
							    &pSapEvent->sapevt.
							    sapStationAssocReassocCompleteEvent.
							    chan_info);
		}
		hdd_wlan_green_ap_add_sta(pHddCtx);
		break;

	case eSAP_STA_DISASSOC_EVENT:
		memcpy(wrqu.addr.sa_data,
		       &pSapEvent->sapevt.sapStationDisassocCompleteEvent.
		       staMac, CDF_MAC_ADDR_SIZE);
		hddLog(LOG1, " disassociated " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(wrqu.addr.sa_data));

		cdf_status = cdf_event_set(&pHostapdState->cdf_event);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status))
			hddLog(LOGE, "ERR: Station Deauth event Set failed");

		if (pSapEvent->sapevt.sapStationDisassocCompleteEvent.reason ==
		    eSAP_USR_INITATED_DISASSOC)
			hddLog(LOG1, " User initiated disassociation");
		else
			hddLog(LOG1, " MAC initiated disassociation");
		we_event = IWEVEXPIRED;
		cdf_status =
			hdd_softap_get_sta_id(pHostapdAdapter,
					      &pSapEvent->sapevt.
					      sapStationDisassocCompleteEvent.staMac,
					      &staId);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			hddLog(LOGE, FL("ERROR: HDD Failed to find sta id!!"));
			return CDF_STATUS_E_FAILURE;
		}
#ifdef IPA_OFFLOAD
		if (hdd_ipa_is_enabled(pHddCtx)) {
			status = hdd_ipa_wlan_evt(pHostapdAdapter, staId,
					WLAN_CLIENT_DISCONNECT,
					pSapEvent->sapevt.
					sapStationDisassocCompleteEvent.
					staMac.bytes);

			if (status) {
				hddLog(LOGE,
				       ("ERROR: WLAN_CLIENT_DISCONNECT event failed!!"));
				goto stopbss;
			}
		}
#endif
#ifdef QCA_PKT_PROTO_TRACE
		/* Peer dis-associated, update into trace buffer */
		if (pHddCtx->config->gEnableDebugLog) {
			cds_pkt_trace_buf_update("HA:DISASC");
		}
#endif /* QCA_PKT_PROTO_TRACE */
		hdd_softap_deregister_sta(pHostapdAdapter, staId);

		pHddApCtx->bApActive = false;
		spin_lock_bh(&pHostapdAdapter->staInfo_lock);
		for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
			if (pHostapdAdapter->aStaInfo[i].isUsed
			    && i !=
			    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->
			    uBCStaId) {
				pHddApCtx->bApActive = true;
				break;
			}
		}
		spin_unlock_bh(&pHostapdAdapter->staInfo_lock);

		/* Start AP inactivity timer if no stations associated with it */
		if ((0 !=
		     (WLAN_HDD_GET_CTX(pHostapdAdapter))->config->
		     nAPAutoShutOff)) {
			if (pHddApCtx->bApActive == false) {
				if (pHddApCtx->hdd_ap_inactivity_timer.state ==
				    CDF_TIMER_STATE_STOPPED) {
					cdf_status =
						cdf_mc_timer_start(&pHddApCtx->
								   hdd_ap_inactivity_timer,
								   (WLAN_HDD_GET_CTX
								    (pHostapdAdapter))->
								   config->
								   nAPAutoShutOff *
								   1000);
					if (!CDF_IS_STATUS_SUCCESS(cdf_status))
						hddLog(LOGE,
						       FL("Failed to init AP inactivity timer"));
				} else
					CDF_ASSERT
						(cdf_mc_timer_get_current_state
							(&pHddApCtx->
							hdd_ap_inactivity_timer) ==
						CDF_TIMER_STATE_STOPPED);
			}
		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif

		cfg80211_del_sta(dev,
				 (const u8 *)&pSapEvent->sapevt.
				 sapStationDisassocCompleteEvent.staMac.
				 bytes[0], GFP_KERNEL);

		/* Update the beacon Interval if it is P2P GO */
		cdf_status = cds_change_mcc_go_beacon_interval(pHostapdAdapter);
		if (CDF_STATUS_SUCCESS != cdf_status) {
			hddLog(LOGE, FL("failed to update Beacon interval %d"),
				cdf_status);
		}
		if (pHostapdAdapter->device_mode == WLAN_HDD_P2P_GO) {
			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_oem_app(&pSapEvent->sapevt.
							    sapStationDisassocCompleteEvent.
							    staMac, ePeerDisconnected,
							    0,
							    pHostapdAdapter->
							    sessionId, NULL);
		}
#ifdef MSM_PLATFORM
		/*stop timer in sap/p2p_go */
		if (pHddApCtx->bApActive == false) {
			spin_lock_bh(&pHddCtx->bus_bw_lock);
			pHostapdAdapter->prev_tx_packets = 0;
			pHostapdAdapter->prev_rx_packets = 0;
			spin_unlock_bh(&pHddCtx->bus_bw_lock);
			hdd_stop_bus_bw_compute_timer(pHostapdAdapter);
		}
#endif
		hdd_wlan_green_ap_del_sta(pHddCtx);
		break;

	case eSAP_WPS_PBC_PROBE_REQ_EVENT:
	{
		static const char *message =
			"MLMEWPSPBCPROBEREQ.indication";
		union iwreq_data wreq;

		down(&pHddApCtx->semWpsPBCOverlapInd);
		pHddApCtx->WPSPBCProbeReq.probeReqIELen =
			pSapEvent->sapevt.sapPBCProbeReqEvent.
			WPSPBCProbeReq.probeReqIELen;

		cdf_mem_copy(pHddApCtx->WPSPBCProbeReq.probeReqIE,
			     pSapEvent->sapevt.sapPBCProbeReqEvent.
			     WPSPBCProbeReq.probeReqIE,
			     pHddApCtx->WPSPBCProbeReq.probeReqIELen);

		cdf_mem_copy(pHddApCtx->WPSPBCProbeReq.peerMacAddr,
			     pSapEvent->sapevt.sapPBCProbeReqEvent.
			     WPSPBCProbeReq.peerMacAddr,
			     CDF_MAC_ADDR_SIZE);
		hddLog(LOG1, "WPS PBC probe req " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(pHddApCtx->WPSPBCProbeReq.
				      peerMacAddr));
		memset(&wreq, 0, sizeof(wreq));
		wreq.data.length = strlen(message);             /* This is length of message */
		wireless_send_event(dev, IWEVCUSTOM, &wreq,
				    (char *)message);

		return CDF_STATUS_SUCCESS;
	}
	case eSAP_ASSOC_STA_CALLBACK_EVENT:
		pAssocStasArray =
			pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas;
		if (pSapEvent->sapevt.sapAssocStaListEvent.noOfAssocSta != 0) { /* List of associated stations */
			for (i = 0;
			     i <
			     pSapEvent->sapevt.sapAssocStaListEvent.
			     noOfAssocSta; i++) {
				hddLog(LOG1,
				       "Associated Sta Num %d:assocId=%d, staId=%d, staMac="
				       MAC_ADDRESS_STR, i + 1,
				       pAssocStasArray->assocId,
				       pAssocStasArray->staId,
				       MAC_ADDR_ARRAY(pAssocStasArray->staMac.
						      bytes));
				pAssocStasArray++;
			}
		}
		cdf_mem_free(pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas);        /* Release caller allocated memory here */
		pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas = NULL;
		return CDF_STATUS_SUCCESS;
	case eSAP_INDICATE_MGMT_FRAME:
		hdd_indicate_mgmt_frame(pHostapdAdapter,
					pSapEvent->sapevt.sapManagementFrameInfo.
					nFrameLength,
					pSapEvent->sapevt.sapManagementFrameInfo.
					pbFrames,
					pSapEvent->sapevt.sapManagementFrameInfo.
					frameType,
					pSapEvent->sapevt.sapManagementFrameInfo.
					rxChan, 0);
		return CDF_STATUS_SUCCESS;
	case eSAP_REMAIN_CHAN_READY:
		hdd_remain_chan_ready_handler(pHostapdAdapter,
			pSapEvent->sapevt.sap_roc_ind.scan_id);
		return CDF_STATUS_SUCCESS;
	case eSAP_SEND_ACTION_CNF:
		hdd_send_action_cnf(pHostapdAdapter,
				    (eSAP_STATUS_SUCCESS ==
				     pSapEvent->sapevt.sapActionCnf.
				     actionSendSuccess) ? true : false);
		return CDF_STATUS_SUCCESS;
	case eSAP_UNKNOWN_STA_JOIN:
		snprintf(unknownSTAEvent, IW_CUSTOM_MAX,
			 "JOIN_UNKNOWN_STA-%02x:%02x:%02x:%02x:%02x:%02x",
			 pSapEvent->sapevt.sapUnknownSTAJoin.macaddr.bytes[0],
			 pSapEvent->sapevt.sapUnknownSTAJoin.macaddr.bytes[1],
			 pSapEvent->sapevt.sapUnknownSTAJoin.macaddr.bytes[2],
			 pSapEvent->sapevt.sapUnknownSTAJoin.macaddr.bytes[3],
			 pSapEvent->sapevt.sapUnknownSTAJoin.macaddr.bytes[4],
			 pSapEvent->sapevt.sapUnknownSTAJoin.macaddr.bytes[5]);
		we_event = IWEVCUSTOM;  /* Discovered a new node (AP mode). */
		wrqu.data.pointer = unknownSTAEvent;
		wrqu.data.length = strlen(unknownSTAEvent);
		we_custom_event_generic = (uint8_t *) unknownSTAEvent;
		hddLog(LOGE, "%s", unknownSTAEvent);
		break;

	case eSAP_MAX_ASSOC_EXCEEDED:
		snprintf(maxAssocExceededEvent, IW_CUSTOM_MAX,
			 "Peer %02x:%02x:%02x:%02x:%02x:%02x denied"
			 " assoc due to Maximum Mobile Hotspot connections reached. Please disconnect"
			 " one or more devices to enable the new device connection",
			 pSapEvent->sapevt.sapMaxAssocExceeded.macaddr.bytes[0],
			 pSapEvent->sapevt.sapMaxAssocExceeded.macaddr.bytes[1],
			 pSapEvent->sapevt.sapMaxAssocExceeded.macaddr.bytes[2],
			 pSapEvent->sapevt.sapMaxAssocExceeded.macaddr.bytes[3],
			 pSapEvent->sapevt.sapMaxAssocExceeded.macaddr.bytes[4],
			 pSapEvent->sapevt.sapMaxAssocExceeded.macaddr.
			 bytes[5]);
		we_event = IWEVCUSTOM;  /* Discovered a new node (AP mode). */
		wrqu.data.pointer = maxAssocExceededEvent;
		wrqu.data.length = strlen(maxAssocExceededEvent);
		we_custom_event_generic = (uint8_t *) maxAssocExceededEvent;
		hddLog(LOG1, "%s", maxAssocExceededEvent);
		break;
	case eSAP_STA_ASSOC_IND:
		return CDF_STATUS_SUCCESS;

	case eSAP_DISCONNECT_ALL_P2P_CLIENT:
		hddLog(LOG1, FL(" Disconnecting all the P2P Clients...."));
		hdd_clear_all_sta(pHostapdAdapter, usrDataForCallback);
		return CDF_STATUS_SUCCESS;

	case eSAP_MAC_TRIG_STOP_BSS_EVENT:
		cdf_status =
			hdd_stop_bss_link(pHostapdAdapter, usrDataForCallback);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			hddLog(LOGW, FL("hdd_stop_bss_link failed %d"),
			       cdf_status);
		}
		return CDF_STATUS_SUCCESS;

	case eSAP_CHANNEL_CHANGE_EVENT:
		hddLog(LOG1, FL("Received eSAP_CHANNEL_CHANGE_EVENT event"));
		/* Prevent suspend for new channel */
		hdd_hostapd_channel_prevent_suspend(pHostapdAdapter,
					pSapEvent->sapevt.sap_ch_selected.pri_ch);
		/* Allow suspend for old channel */
		hdd_hostapd_channel_allow_suspend(pHostapdAdapter,
					pHddApCtx->operatingChannel);
		/* SME/PE is already updated for new operation channel. So update
		* HDD layer also here. This resolves issue in AP-AP mode where
		* AP1 channel is changed due to RADAR then CAC is going on and
		* START_BSS on new channel has not come to HDD. At this case if
		* AP2 is start it needs current operation channel for MCC DFS
		* restiction
		*/
		pHddApCtx->operatingChannel =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		pHddApCtx->sapConfig.acs_cfg.pri_ch =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		pHddApCtx->sapConfig.acs_cfg.ht_sec_ch =
			pSapEvent->sapevt.sap_ch_selected.ht_sec_ch;
		pHddApCtx->sapConfig.acs_cfg.vht_seg0_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg0_center_ch;
		pHddApCtx->sapConfig.acs_cfg.vht_seg1_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg1_center_ch;
		pHddApCtx->sapConfig.acs_cfg.ch_width =
			pSapEvent->sapevt.sap_ch_selected.ch_width;

		/* Indicate operating channel change to hostapd
		 * only for non driver override acs
		 */
		if (pHostapdAdapter->device_mode == WLAN_HDD_SOFTAP &&
						pHddCtx->config->force_sap_acs)
			return CDF_STATUS_SUCCESS;
		else
			return hdd_chan_change_notify(pHostapdAdapter, dev,
				   pSapEvent->sapevt.sap_ch_selected.pri_ch);

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	case eSAP_ACS_SCAN_SUCCESS_EVENT:
		pHddCtx->skip_acs_scan_status = eSAP_SKIP_ACS_SCAN;
		hddLog(LOG1, FL("Reusing Last ACS scan result for %d sec"),
		       ACS_SCAN_EXPIRY_TIMEOUT_S);
		cdf_mc_timer_stop(&pHddCtx->skip_acs_scan_timer);
		cdf_status = cdf_mc_timer_start(&pHddCtx->skip_acs_scan_timer,
						ACS_SCAN_EXPIRY_TIMEOUT_S *
						1000);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status))
			hddLog(LOGE,
			       FL("Failed to start ACS scan expiry timer"));
		return CDF_STATUS_SUCCESS;
#endif

	case eSAP_DFS_NOL_GET:
		hddLog(LOG1,
		       FL("Received eSAP_DFS_NOL_GET event"));
#if defined CONFIG_CNSS
		/* get the dfs nol from cnss */
		ret =
			cnss_wlan_get_dfs_nol(pSapEvent->sapevt.sapDfsNolInfo.
					      pDfsList,
					      pSapEvent->sapevt.sapDfsNolInfo.
					      sDfsList);

		if (ret > 0) {
			hddLog(LOG2,
				FL("Get %d bytes of dfs nol from cnss"), ret);
			return CDF_STATUS_SUCCESS;
		} else {
			hddLog(LOG2,
				FL("No dfs nol entry in CNSS, ret: %d"), ret);
			return CDF_STATUS_E_FAULT;
		}
#else
		return CDF_STATUS_E_FAILURE;
#endif
	case eSAP_DFS_NOL_SET:
		hddLog(LOG1, FL("Received eSAP_DFS_NOL_SET event"));
#if defined CONFIG_CNSS
		/* set the dfs nol to cnss */
		ret =
			cnss_wlan_set_dfs_nol(pSapEvent->sapevt.sapDfsNolInfo.
					      pDfsList,
					      pSapEvent->sapevt.sapDfsNolInfo.
					      sDfsList);

		if (ret) {
			hddLog(LOG2,
			       FL("Failed to set dfs nol - ret: %d"),
			       ret);
		} else {
			hddLog(LOG2, FL(" Set %d bytes dfs nol to cnss"),
			       pSapEvent->sapevt.sapDfsNolInfo.sDfsList);
		}
#else
		return CDF_STATUS_E_FAILURE;
#endif
		return CDF_STATUS_SUCCESS;
	case eSAP_ACS_CHANNEL_SELECTED:
		hddLog(LOG1, FL("ACS Completed for wlan%d"),
					pHostapdAdapter->dev->ifindex);
		clear_bit(ACS_PENDING, &pHostapdAdapter->event_flags);
		clear_bit(ACS_IN_PROGRESS, &pHddCtx->g_event_flags);
		pHddApCtx->sapConfig.acs_cfg.pri_ch =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		pHddApCtx->sapConfig.acs_cfg.ht_sec_ch =
			pSapEvent->sapevt.sap_ch_selected.ht_sec_ch;
		pHddApCtx->sapConfig.acs_cfg.vht_seg0_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg0_center_ch;
		pHddApCtx->sapConfig.acs_cfg.vht_seg1_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg1_center_ch;
		pHddApCtx->sapConfig.acs_cfg.ch_width =
			pSapEvent->sapevt.sap_ch_selected.ch_width;
		/* send vendor event to hostapd only for hostapd based acs*/
		if (!pHddCtx->config->force_sap_acs)
			wlan_hdd_cfg80211_acs_ch_select_evt(pHostapdAdapter);
		return CDF_STATUS_SUCCESS;
	case eSAP_ECSA_CHANGE_CHAN_IND:
		hddLog(LOG1,
		  FL("Channel change indication from peer for channel %d"),
				pSapEvent->sapevt.sap_chan_cng_ind.new_chan);
		if (hdd_softap_set_channel_change(dev,
			 pSapEvent->sapevt.sap_chan_cng_ind.new_chan))
			return CDF_STATUS_E_FAILURE;
		else
			return CDF_STATUS_SUCCESS;

	default:
		hddLog(LOG1, "SAP message is not handled");
		goto stopbss;
		return CDF_STATUS_SUCCESS;
	}
	wireless_send_event(dev, we_event, &wrqu,
			    (char *)we_custom_event_generic);
	return CDF_STATUS_SUCCESS;

stopbss:
	{
		uint8_t we_custom_event[64];
		char *stopBssEvent = "STOP-BSS.response";       /* 17 */
		int event_len = strlen(stopBssEvent);

		hddLog(LOG1, FL("BSS stop status = %s"),
		       pSapEvent->sapevt.sapStopBssCompleteEvent.status ?
		       "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");

		/* Change the BSS state now since, as we are shutting things down,
		 * we don't want interfaces to become re-enabled */
		pHostapdState->bssState = BSS_STOP;

		if (0 !=
		    (WLAN_HDD_GET_CTX(pHostapdAdapter))->config->
		    nAPAutoShutOff) {
			if (CDF_TIMER_STATE_RUNNING ==
			    pHddApCtx->hdd_ap_inactivity_timer.state) {
				cdf_status =
					cdf_mc_timer_stop(&pHddApCtx->
							  hdd_ap_inactivity_timer);
				if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
					hddLog(LOGE,
					       FL("Failed to stop AP inactivity timer"));
				}
			}

			cdf_status =
				cdf_mc_timer_destroy(&pHddApCtx->
						hdd_ap_inactivity_timer);
			if (!CDF_IS_STATUS_SUCCESS(cdf_status))
				hddLog(LOGE, FL("Failed to Destroy AP inactivity timer"));
		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif

		/* Stop the pkts from n/w stack as we are going to free all of
		 * the TX WMM queues for all STAID's */
		hdd_hostapd_stop(dev);

		/* reclaim all resources allocated to the BSS */
		cdf_status = hdd_softap_stop_bss(pHostapdAdapter);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			hddLog(LOGW,
			       FL("hdd_softap_stop_bss failed %d"),
			       cdf_status);
		}

		/* once the event is set, structure dev/pHostapdAdapter should
		 * not be touched since they are now subject to being deleted
		 * by another thread */
		if (eSAP_STOP_BSS_EVENT == sapEvent)
			cdf_event_set(&pHostapdState->cdf_stop_bss_event);

		/* notify userspace that the BSS has stopped */
		memset(&we_custom_event, '\0', sizeof(we_custom_event));
		memcpy(&we_custom_event, stopBssEvent, event_len);
		memset(&wrqu, 0, sizeof(wrqu));
		wrqu.data.length = event_len;
		we_event = IWEVCUSTOM;
		we_custom_event_generic = we_custom_event;
		wireless_send_event(dev, we_event, &wrqu,
				    (char *)we_custom_event_generic);
		cds_dump_concurrency_info(pHddCtx);
		/* Send SCC/MCC Switching event to IPA */
		hdd_ipa_send_mcc_scc_msg(pHddCtx, pHddCtx->mcc_mode);
	}
	return CDF_STATUS_SUCCESS;
}

int hdd_softap_unpack_ie(tHalHandle halHandle,
			 eCsrEncryptionType *pEncryptType,
			 eCsrEncryptionType *mcEncryptType,
			 eCsrAuthType *pAuthType,
			 bool *pMFPCapable,
			 bool *pMFPRequired,
			 uint16_t gen_ie_len, uint8_t *gen_ie)
{
	tDot11fIERSN dot11RSNIE;
	tDot11fIEWPA dot11WPAIE;

	uint8_t *pRsnIe;
	uint16_t RSNIeLen;

	if (NULL == halHandle) {
		hddLog(LOGE, FL("Error haHandle returned NULL"));
		return -EINVAL;
	}
	/* Validity checks */
	if ((gen_ie_len < CDF_MIN(DOT11F_IE_RSN_MIN_LEN, DOT11F_IE_WPA_MIN_LEN))
	    || (gen_ie_len >
		CDF_MAX(DOT11F_IE_RSN_MAX_LEN, DOT11F_IE_WPA_MAX_LEN)))
		return -EINVAL;
	/* Type check */
	if (gen_ie[0] == DOT11F_EID_RSN) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_RSN_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_RSN_MAX_LEN)) {
			return CDF_STATUS_E_FAILURE;
		}
		/* Skip past the EID byte and length byte */
		pRsnIe = gen_ie + 2;
		RSNIeLen = gen_ie_len - 2;
		/* Unpack the RSN IE */
		memset(&dot11RSNIE, 0, sizeof(tDot11fIERSN));
		dot11f_unpack_ie_rsn((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11RSNIE);
		/* Copy out the encryption and authentication types */
		hddLog(LOG1, FL("pairwise cipher suite count: %d"),
		       dot11RSNIE.pwise_cipher_suite_count);
		hddLog(LOG1, FL("authentication suite count: %d"),
		       dot11RSNIE.akm_suite_count);
		/*Here we have followed the apple base code,
		   but probably I suspect we can do something different */
		/* dot11RSNIE.akm_suite_count */
		/* Just translate the FIRST one */
		*pAuthType =
			hdd_translate_rsn_to_csr_auth_type(dot11RSNIE.akm_suites[0]);
		/* dot11RSNIE.pwise_cipher_suite_count */
		*pEncryptType =
			hdd_translate_rsn_to_csr_encryption_type(dot11RSNIE.
								 pwise_cipher_suites[0]);
		/* dot11RSNIE.gp_cipher_suite_count */
		*mcEncryptType =
			hdd_translate_rsn_to_csr_encryption_type(dot11RSNIE.
								 gp_cipher_suite);
		/* Set the PMKSA ID Cache for this interface */
		*pMFPCapable = 0 != (dot11RSNIE.RSN_Cap[0] & 0x80);
		*pMFPRequired = 0 != (dot11RSNIE.RSN_Cap[0] & 0x40);
		/* Calling csr_roam_set_pmkid_cache to configure the PMKIDs into the cache */
	} else if (gen_ie[0] == DOT11F_EID_WPA) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_WPA_MAX_LEN)) {
			return CDF_STATUS_E_FAILURE;
		}
		/* Skip past the EID byte and length byte - and four byte WiFi OUI */
		pRsnIe = gen_ie + 2 + 4;
		RSNIeLen = gen_ie_len - (2 + 4);
		/* Unpack the WPA IE */
		memset(&dot11WPAIE, 0, sizeof(tDot11fIEWPA));
		dot11f_unpack_ie_wpa((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11WPAIE);
		/* Copy out the encryption and authentication types */
		hddLog(LOG1, FL("WPA unicast cipher suite count: %d"),
		       dot11WPAIE.unicast_cipher_count);
		hddLog(LOG1, FL("WPA authentication suite count: %d"),
		       dot11WPAIE.auth_suite_count);
		/* dot11WPAIE.auth_suite_count */
		/* Just translate the FIRST one */
		*pAuthType =
			hdd_translate_wpa_to_csr_auth_type(dot11WPAIE.auth_suites[0]);
		/* dot11WPAIE.unicast_cipher_count */
		*pEncryptType =
			hdd_translate_wpa_to_csr_encryption_type(dot11WPAIE.
								 unicast_ciphers[0]);
		/* dot11WPAIE.unicast_cipher_count */
		*mcEncryptType =
			hdd_translate_wpa_to_csr_encryption_type(dot11WPAIE.
								 multicast_cipher);
		*pMFPCapable = false;
		*pMFPRequired = false;
	} else {
		hddLog(LOGW, FL("gen_ie[0]: %d"), gen_ie[0]);
		return CDF_STATUS_E_FAILURE;
	}
	return CDF_STATUS_SUCCESS;
}

/**
 * hdd_softap_set_channel_change() -
 * This function to support SAP channel change with CSA IE
 * set in the beacons.
 *
 * @dev: pointer to the net device.
 * @target_channel: target channel number.
 *
 * Return: 0 for success, non zero for failure
 */
int hdd_softap_set_channel_change(struct net_device *dev, int target_channel)
{
	CDF_STATUS status;
	int ret = 0;
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *pHddCtx = NULL;
	hdd_adapter_t *sta_adapter;
	hdd_station_ctx_t *sta_ctx;

#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t p_cds_context =
		(WLAN_HDD_GET_CTX(pHostapdAdapter))->pcds_context;
#endif

	pHddCtx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);
	if (ret) {
		hddLog(LOGE, FL("invalid HDD context"));
		return ret;
	}

	sta_adapter = hdd_get_adapter(pHddCtx, WLAN_HDD_INFRA_STATION);
	/*
	 * conc_custom_rule1:
	 * Force SCC for SAP + STA
	 * if STA is already connected then we shouldn't allow
	 * channel switch in SAP interface.
	 */
	if (sta_adapter && pHddCtx->config->conc_custom_rule1) {
		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(sta_adapter);
		if (hdd_conn_is_connected(sta_ctx)) {
			hdd_err("Channel switch not allowed after STA connection with conc_custom_rule1 enabled");
			return -EBUSY;
		}
	}

	mutex_lock(&pHddCtx->dfs_lock);
	if (pHddCtx->dfs_radar_found == true) {
		mutex_unlock(&pHddCtx->dfs_lock);
		hddLog(LOGE, FL("Channel switch in progress!!"));
		return -EBUSY;
	}
	/*
	 * Set the dfs_radar_found flag to mimic channel change
	 * when a radar is found. This will enable synchronizing
	 * SAP and HDD states similar to that of radar indication.
	 * Suspend the netif queues to stop queuing Tx frames
	 * from upper layers.  netif queues will be resumed
	 * once the channel change is completed and SAP will
	 * post eSAP_START_BSS_EVENT success event to HDD.
	 */
	pHddCtx->dfs_radar_found = true;
	mutex_unlock(&pHddCtx->dfs_lock);
	/*
	 * Post the Channel Change request to SAP.
	 */
	status = wlansap_set_channel_change_with_csa(
#ifdef WLAN_FEATURE_MBSSID
		WLAN_HDD_GET_SAP_CTX_PTR
			(pHostapdAdapter),
#else
		p_cds_context,
#endif
		(uint32_t)
		target_channel);

	if (CDF_STATUS_SUCCESS != status) {
		hddLog(LOGE,
		       FL("SAP set channel failed for channel = %d"),
		       target_channel);
		/*
		 * If channel change command fails then clear the
		 * radar found flag and also restart the netif
		 * queues.
		 */
		mutex_lock(&pHddCtx->dfs_lock);
		pHddCtx->dfs_radar_found = false;
		mutex_unlock(&pHddCtx->dfs_lock);

		ret = -EINVAL;
	}

	return ret;
}

int
static __iw_softap_set_ini_cfg(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	CDF_STATUS vstatus;
	int ret = 0;            /* success */
	hdd_adapter_t *pAdapter = (netdev_priv(dev));
	hdd_context_t *pHddCtx;

	if (pAdapter == NULL) {
		hddLog(LOGE, FL("pAdapter is NULL!"));
		return -EINVAL;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);
	if (ret != 0) {
		hddLog(LOGE, FL("HDD context is not valid"));
		return ret;
	}

	hddLog(LOG1, FL("Received data %s"), extra);

	vstatus = hdd_execute_global_config_command(pHddCtx, extra);
	if (CDF_STATUS_SUCCESS != vstatus) {
		ret = -EINVAL;
	}

	return ret;
}

int
static iw_softap_set_ini_cfg(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_set_ini_cfg(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

int
static __iw_softap_get_ini_cfg(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	int ret = 0;

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);
	if (ret != 0) {
		hddLog(LOGE, FL("HDD context is not valid"));
		return ret;
	}
	hddLog(LOG1, FL("Printing CLD global INI Config"));
	hdd_cfg_get_global_config(pHddCtx, extra, QCSAP_IOCTL_MAX_STR_LEN);
	wrqu->data.length = strlen(extra) + 1;

	return 0;
}

int
static iw_softap_get_ini_cfg(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_get_ini_cfg(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_softap_set_two_ints_getnone() - Generic "set two integer" ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_softap_set_two_ints_getnone(struct net_device *dev,
					    struct iw_request_info *info,
					    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int ret;
	int *value = (int *)extra;
	int sub_cmd = value[0];
	hdd_context_t *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		goto out;

	switch (sub_cmd) {
#ifdef DEBUG
	case QCSAP_IOCTL_SET_FW_CRASH_INJECT:
		hddLog(LOGE, "WE_SET_FW_CRASH_INJECT: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->sessionId,
					   GEN_PARAM_CRASH_INJECT,
					   value[1], value[2],
					   GEN_CMD);
		break;
#endif
	case QCSAP_IOCTL_DUMP_DP_TRACE_LEVEL:
		hdd_info("WE_DUMP_DP_TRACE: %d %d",
		       value[1], value[2]);
		if (value[1] == DUMP_DP_TRACE)
			cdf_dp_trace_dump_all(value[2]);
		break;
	case QCSAP_ENABLE_FW_PROFILE:
		hddLog(LOG1, "QCSAP_ENABLE_FW_PROFILE: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->sessionId,
				 WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
					value[1], value[2], DBG_CMD);
		break;
	case QCSAP_SET_FW_PROFILE_HIST_INTVL:
		hddLog(LOG1, "QCSAP_SET_FW_PROFILE_HIST_INTVL: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->sessionId,
					WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
					value[1], value[2], DBG_CMD);
	default:
		hddLog(LOGE, FL("Invalid IOCTL command %d"), sub_cmd);
		break;
	}

out:
	return ret;
}

static int iw_softap_set_two_ints_getnone(struct net_device *dev,
					  struct iw_request_info *info,
					  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_set_two_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static void print_mac_list(struct cdf_mac_addr *macList, uint8_t size)
{
	int i;
	uint8_t *macArray;

	for (i = 0; i < size; i++) {
		macArray = (macList + i)->bytes;
		pr_info("** ACL entry %i - %02x:%02x:%02x:%02x:%02x:%02x \n",
			i, MAC_ADDR_ARRAY(macArray));
	}
	return;
}

static CDF_STATUS hdd_print_acl(hdd_adapter_t *pHostapdAdapter)
{
	eSapMacAddrACL acl_mode;
	struct cdf_mac_addr MacList[MAX_ACL_MAC_ADDRESS];
	uint8_t listnum;
	void *p_cds_gctx = NULL;

#ifdef WLAN_FEATURE_MBSSID
	p_cds_gctx = WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter);
#else
	p_cds_gctx = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pcds_context;
#endif
	cdf_mem_zero(&MacList[0], sizeof(MacList));
	if (CDF_STATUS_SUCCESS == wlansap_get_acl_mode(p_cds_gctx, &acl_mode)) {
		pr_info("******** ACL MODE *********\n");
		switch (acl_mode) {
		case eSAP_ACCEPT_UNLESS_DENIED:
			pr_info("ACL Mode = ACCEPT_UNLESS_DENIED\n");
			break;
		case eSAP_DENY_UNLESS_ACCEPTED:
			pr_info("ACL Mode = DENY_UNLESS_ACCEPTED\n");
			break;
		case eSAP_SUPPORT_ACCEPT_AND_DENY:
			pr_info("ACL Mode = ACCEPT_AND_DENY\n");
			break;
		case eSAP_ALLOW_ALL:
			pr_info("ACL Mode = ALLOW_ALL\n");
			break;
		default:
			pr_info("Invalid SAP ACL Mode = %d\n", acl_mode);
			return CDF_STATUS_E_FAILURE;
		}
	} else {
		return CDF_STATUS_E_FAILURE;
	}

	if (CDF_STATUS_SUCCESS == wlansap_get_acl_accept_list(p_cds_gctx,
							      &MacList[0],
							      &listnum)) {
		pr_info("******* WHITE LIST ***********\n");
		if (listnum <= MAX_ACL_MAC_ADDRESS)
			print_mac_list(&MacList[0], listnum);
	} else {
		return CDF_STATUS_E_FAILURE;
	}

	if (CDF_STATUS_SUCCESS == wlansap_get_acl_deny_list(p_cds_gctx,
							    &MacList[0],
							    &listnum)) {
		pr_info("******* BLACK LIST ***********\n");
		if (listnum <= MAX_ACL_MAC_ADDRESS)
			print_mac_list(&MacList[0], listnum);
	} else {
		return CDF_STATUS_E_FAILURE;
	}
	return CDF_STATUS_SUCCESS;
}

int
static __iw_softap_setparam(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	tHalHandle hHal;
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int set_value = value[1];
	CDF_STATUS status;
	int ret = 0;            /* success */
	v_CONTEXT_t p_cds_context;
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
	if (!hHal) {
		hddLog(LOGE, FL("Hal ctx is null"));
		return -EINVAL;
	}

	p_cds_context = hdd_ctx->pcds_context;
	if (!p_cds_context) {
		hddLog(LOGE, FL("cds ctx is null"));
		return -ENOENT;
	}

	switch (sub_cmd) {
	case QCASAP_SET_RADAR_DBG:
		hddLog(LOG1, FL("QCASAP_SET_RADAR_DBG called with: value: %d"),
		       set_value);
		wlan_sap_enable_phy_error_logs(hHal, (bool) set_value);
		break;

	case QCSAP_PARAM_CLR_ACL:
		if (CDF_STATUS_SUCCESS != wlansap_clear_acl(
#ifdef WLAN_FEATURE_MBSSID
			    WLAN_HDD_GET_SAP_CTX_PTR
				    (pHostapdAdapter)
#else
			    p_cds_context
#endif
			    )) {
			ret = -EIO;
		}
		break;

	case QCSAP_PARAM_ACL_MODE:
		if ((eSAP_ALLOW_ALL < (eSapMacAddrACL) set_value) ||
		    (eSAP_ACCEPT_UNLESS_DENIED > (eSapMacAddrACL) set_value)) {
			hddLog(LOGE, FL("Invalid ACL Mode value %d"),
			       set_value);
			ret = -EINVAL;
		} else {
#ifdef WLAN_FEATURE_MBSSID
			wlansap_set_mode(WLAN_HDD_GET_SAP_CTX_PTR
						 (pHostapdAdapter), set_value);
#else
			wlansap_set_mode(p_cds_context, set_value);
#endif

		}
		break;

	case QCSAP_PARAM_SET_CHANNEL_CHANGE:
		if ((WLAN_HDD_SOFTAP == pHostapdAdapter->device_mode) ||
		   (WLAN_HDD_P2P_GO == pHostapdAdapter->device_mode)) {
			hddLog(LOG1,
			       "SET Channel Change to new channel= %d",
			       set_value);
			ret = hdd_softap_set_channel_change(dev, set_value);
		} else {
			hddLog(LOGE,
			       FL("Channel Change Failed, Device in test mode"));
			ret = -EINVAL;
		}
		break;
	case QCSAP_PARAM_AUTO_CHANNEL:
		if (set_value == 0 || set_value == 1)
			(WLAN_HDD_GET_CTX(
				pHostapdAdapter))->config->force_sap_acs =
								set_value;
		else
			ret = -EINVAL;
		break;

	case QCSAP_PARAM_MAX_ASSOC:
		if (WNI_CFG_ASSOC_STA_LIMIT_STAMIN > set_value) {
			hddLog(LOGE, FL("Invalid setMaxAssoc value %d"),
			       set_value);
			ret = -EINVAL;
		} else {
			if (WNI_CFG_ASSOC_STA_LIMIT_STAMAX < set_value) {
				hddLog(LOGW,
				       FL("setMaxAssoc %d > max allowed %d."),
				       set_value,
				       WNI_CFG_ASSOC_STA_LIMIT_STAMAX);
				hddLog(LOGW,
				       FL("Setting it to max allowed and continuing"));
				set_value = WNI_CFG_ASSOC_STA_LIMIT_STAMAX;
			}
			status = sme_cfg_set_int(hHal, WNI_CFG_ASSOC_STA_LIMIT,
					set_value);
			if (status != CDF_STATUS_SUCCESS) {
				hddLog(LOGE,
				       FL("setMaxAssoc failure, status %d"),
				       status);
				ret = -EIO;
			}
		}
		break;

	case QCSAP_PARAM_HIDE_SSID:
	{
		CDF_STATUS status = CDF_STATUS_SUCCESS;
		status =
			sme_hide_ssid(hHal, pHostapdAdapter->sessionId,
				      set_value);
		if (CDF_STATUS_SUCCESS != status) {
			hddLog(LOGE, FL("QCSAP_PARAM_HIDE_SSID failed"));
			return status;
		}
		break;
	}
	case QCSAP_PARAM_SET_MC_RATE:
	{
		tSirRateUpdateInd rateUpdate = {0};
		struct hdd_config *pConfig = hdd_ctx->config;

		hddLog(LOG1, "MC Target rate %d", set_value);
		cdf_copy_macaddr(&rateUpdate.bssid,
				 &pHostapdAdapter->macAddressCurrent);
		rateUpdate.nss = (pConfig->enable2x2 == 0) ? 0 : 1;
		rateUpdate.dev_mode = pHostapdAdapter->device_mode;
		rateUpdate.mcastDataRate24GHz = set_value;
		rateUpdate.mcastDataRate24GHzTxFlag = 1;
		rateUpdate.mcastDataRate5GHz = set_value;
		rateUpdate.bcastDataRate = -1;
		status = sme_send_rate_update_ind(hHal, &rateUpdate);
		if (CDF_STATUS_SUCCESS != status) {
			hddLog(LOGE, FL("SET_MC_RATE failed"));
			ret = -1;
		}
		break;
	}

	case QCSAP_PARAM_SET_TXRX_FW_STATS:
	{
		hddLog(LOG1, "QCSAP_PARAM_SET_TXRX_FW_STATS val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID,
					  set_value, VDEV_CMD);
		break;
	}
	/* Firmware debug log */
	case QCSAP_DBGLOG_LOG_LEVEL:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_LOG_LEVEL val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_VAP_ENABLE:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_VAP_ENABLE val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_VAP_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_VAP_DISABLE:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_VAP_DISABLE val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_VAP_DISABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_MODULE_ENABLE:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_MODULE_ENABLE val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_MODULE_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_MODULE_DISABLE:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_MODULE_DISABLE val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_MODULE_DISABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_MOD_LOG_LEVEL:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_MOD_LOG_LEVEL val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_MOD_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_TYPE:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_TYPE val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_TYPE,
					  set_value, DBG_CMD);
		break;
	}
	case QCSAP_DBGLOG_REPORT_ENABLE:
	{
		hddLog(LOG1, "QCSAP_DBGLOG_REPORT_ENABLE val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_DBGLOG_REPORT_ENABLE,
					  set_value, DBG_CMD);
		break;
	}
	case QCSAP_PARAM_SET_MCC_CHANNEL_LATENCY:
	{
		cds_set_mcc_latency(pHostapdAdapter, set_value);
		break;
	}

	case QCSAP_PARAM_SET_MCC_CHANNEL_QUOTA:
	{
		hddLog(LOG1,
		       FL("iwpriv cmd to set MCC quota value %dms"),
		       set_value);
		ret = cds_go_set_mcc_p2p_quota(pHostapdAdapter,
						    set_value);
		break;
	}

	case QCASAP_TXRX_FWSTATS_RESET:
	{
		hddLog(LOG1, "WE_TXRX_FWSTATS_RESET val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMA_VDEV_TXRX_FWSTATS_RESET_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case QCSAP_PARAM_RTSCTS:
	{
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_ENABLE_RTSCTS,
					  set_value, VDEV_CMD);
		if (ret) {
			hddLog(LOGE, "FAILED TO SET RTSCTS at SAP");
			ret = -EIO;
		}
		break;
	}
	case QCASAP_SET_11N_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;
		tsap_Config_t *pConfig =
			&pHostapdAdapter->sessionCtx.ap.sapConfig;

		hddLog(LOG1, "SET_HT_RATE val %d", set_value);

		if (set_value != 0xff) {
			rix = RC_2_RATE_IDX(set_value);
			if (set_value & 0x80) {
				if (pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11b
				    || pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11b_ONLY
				    || pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11g
				    || pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11g_ONLY
				    || pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_abg
				    || pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11a) {
					hddLog(LOGE,
					       "Not valid mode for HT");
					ret = -EIO;
					break;
				}
				preamble = WMI_RATE_PREAMBLE_HT;
				nss = HT_RC_2_STREAMS(set_value) - 1;
			} else if (set_value & 0x10) {
				if (pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11a) {
					hddLog(LOGE, "Not valid for cck");
					ret = -EIO;
					break;
				}
				preamble = WMI_RATE_PREAMBLE_CCK;
				/* Enable Short preamble always
				 * for CCK except 1mbps
				 */
				if (rix != 0x3)
					rix |= 0x4;
			} else {
				if (pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11b
				    || pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11b_ONLY) {
					hddLog(LOGE, "Not valid for OFDM");
					ret = -EIO;
					break;
				}
				preamble = WMI_RATE_PREAMBLE_OFDM;
			}
			set_value = (preamble << 6) | (nss << 4) | rix;
		}
		hddLog(LOG1, "SET_HT_RATE val %d rix %d preamble %x nss %d",
		       set_value, rix, preamble, nss);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case QCASAP_SET_VHT_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;
		tsap_Config_t *pConfig =
			&pHostapdAdapter->sessionCtx.ap.sapConfig;

		if (pConfig->SapHw_mode != eCSR_DOT11_MODE_11ac &&
		    pConfig->SapHw_mode != eCSR_DOT11_MODE_11ac_ONLY) {
			hddLog(LOGE,
			       FL("SET_VHT_RATE error: SapHw_mode= 0x%x, ch = %d"),
			       pConfig->SapHw_mode, pConfig->channel);
			ret = -EIO;
			break;
		}

		if (set_value != 0xff) {
		rix = RC_2_RATE_IDX_11AC(set_value);
		preamble = WMI_RATE_PREAMBLE_VHT;
		nss = HT_RC_2_STREAMS_11AC(set_value) - 1;

			set_value = (preamble << 6) | (nss << 4) | rix;
		}
		hddLog(LOG1, "SET_VHT_RATE val %d rix %d preamble %x nss %d",
		       set_value, rix, preamble, nss);

		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case QCASAP_SHORT_GI:
	{
		hddLog(LOG1, "QCASAP_SET_SHORT_GI val %d", set_value);

		/* same as 40MHZ */
		ret = sme_update_ht_config(hHal, pHostapdAdapter->sessionId,
					   WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ,
					   set_value);
		if (ret)
			hddLog(LOGE,
			       "Failed to set ShortGI value ret(%d)", ret);
		break;
	}

	case QCSAP_SET_AMPDU:
	{
		hddLog(LOG1, "QCSAP_SET_AMPDU %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  GEN_VDEV_PARAM_AMPDU,
					  set_value, GEN_CMD);
		break;
	}

	case QCSAP_SET_AMSDU:
	{
		hddLog(LOG1, "QCSAP_SET_AMSDU %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  GEN_VDEV_PARAM_AMSDU,
					  set_value, GEN_CMD);
		break;
	}
	case QCSAP_GTX_HT_MCS:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_HT_MCS %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_HT_MCS,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_VHT_MCS:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_VHT_MCS %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_VHT_MCS,
						set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_USRCFG:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_USR_CFG %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_USR_CFG,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_THRE:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_THRE %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_THRE,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_MARGIN:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_MARGIN %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_MARGIN,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_STEP:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_STEP %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_STEP,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_MINTPC:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_MINTPC %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_MINTPC,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_BWMASK:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_BWMASK %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_BW_MASK,
					  set_value, GTX_CMD);
		break;
	}

#ifdef QCA_PKT_PROTO_TRACE
	case QCASAP_SET_DEBUG_LOG:
	{
		hdd_context_t *pHddCtx =
			WLAN_HDD_GET_CTX(pHostapdAdapter);

		hddLog(LOG1, "QCASAP_SET_DEBUG_LOG val %d", set_value);
		/* Trace buffer dump only */
		if (CDS_PKT_TRAC_DUMP_CMD == set_value) {
			cds_pkt_trace_buf_dump();
			break;
		}
		pHddCtx->config->gEnableDebugLog = set_value;
		break;
	}
#endif /* QCA_PKT_PROTO_TRACE */

	case QCASAP_SET_TM_LEVEL:
	{
		hddLog(LOG1, "Set Thermal Mitigation Level %d", set_value);
		(void)sme_set_thermal_level(hHal, set_value);
		break;
	}

	case QCASAP_SET_DFS_IGNORE_CAC:
	{
		hddLog(LOG1, "Set Dfs ignore CAC  %d", set_value);

		if (pHostapdAdapter->device_mode != WLAN_HDD_SOFTAP)
			return -EINVAL;

		ret = wlansap_set_dfs_ignore_cac(hHal, set_value);
		break;
	}

	case QCASAP_SET_DFS_TARGET_CHNL:
	{
		hddLog(LOG1, "Set Dfs target channel  %d", set_value);

		if (pHostapdAdapter->device_mode != WLAN_HDD_SOFTAP)
			return -EINVAL;

		ret = wlansap_set_dfs_target_chnl(hHal, set_value);
		break;
	}

	case QCASAP_SET_DFS_NOL:
		wlansap_set_dfs_nol(
#ifdef WLAN_FEATURE_MBSSID
			WLAN_HDD_GET_SAP_CTX_PTR
				(pHostapdAdapter),
#else
			p_cds_context,
#endif
			(eSapDfsNolType) set_value);
		break;

	case QCASAP_SET_RADAR_CMD:
	{
		hdd_context_t *pHddCtx =
			WLAN_HDD_GET_CTX(pHostapdAdapter);
		uint8_t ch =
			(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->
			operatingChannel;
		bool isDfsch;

		isDfsch = (CHANNEL_STATE_DFS ==
			   cds_get_channel_state(ch));

		hddLog(LOG1, FL("Set QCASAP_SET_RADAR_CMD val %d"), set_value);

		if (!pHddCtx->dfs_radar_found && isDfsch) {
			ret = wma_cli_set_command(pHostapdAdapter->sessionId,
						  WMA_VDEV_DFS_CONTROL_CMDID,
						  set_value, VDEV_CMD);
		} else {
			hddLog(LOGE,
			       FL("Ignore, radar_found: %d,  dfs_channel: %d"),
			       pHddCtx->dfs_radar_found, isDfsch);
		}
		break;
	}
	case QCASAP_TX_CHAINMASK_CMD:
	{
		hddLog(LOG1, "QCASAP_TX_CHAINMASK_CMD val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_PDEV_PARAM_TX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		break;
	}

	case QCASAP_RX_CHAINMASK_CMD:
	{
		hddLog(LOG1, "QCASAP_RX_CHAINMASK_CMD val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_PDEV_PARAM_RX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		break;
	}

	case QCASAP_NSS_CMD:
	{
		hddLog(LOG1, "QCASAP_NSS_CMD val %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					  WMI_VDEV_PARAM_NSS,
					  set_value, VDEV_CMD);
		break;
	}

	case QCSAP_IPA_UC_STAT:
	{
		/* If input value is non-zero get stats */
		switch (set_value) {
		case 1:
			hdd_ipa_uc_stat_request(pHostapdAdapter, set_value);
			break;
		case 3:
			hdd_ipa_uc_rt_debug_host_dump(
					WLAN_HDD_GET_CTX(pHostapdAdapter));
			break;
		default:
			/* place holder for stats clean up
			 * Stats clean not implemented yet on firmware and ipa
			 */
			break;
		}
		return ret;
	}

	case QCASAP_SET_PHYMODE:
	{
		hdd_context_t *phddctx =
			WLAN_HDD_GET_CTX(pHostapdAdapter);

		ret =
			wlan_hdd_update_phymode(dev, hHal, set_value,
						phddctx);
		break;
	}
	case QCASAP_DUMP_STATS:
	{
		hddLog(LOG1, "QCASAP_DUMP_STATS val %d", set_value);
		hdd_wlan_dump_stats(pHostapdAdapter, set_value);
		break;
	}
	case QCASAP_CLEAR_STATS:
	{
		hdd_context_t *hdd_ctx =
			WLAN_HDD_GET_CTX(pHostapdAdapter);
		hddLog(LOG1, "QCASAP_CLEAR_STATS val %d", set_value);
		switch (set_value) {
		case WLAN_HDD_STATS:
			memset(&pHostapdAdapter->stats, 0,
						sizeof(pHostapdAdapter->stats));
			memset(&pHostapdAdapter->hdd_stats, 0,
					sizeof(pHostapdAdapter->hdd_stats));
			break;
		case WLAN_TXRX_HIST_STATS:
			wlan_hdd_clear_tx_rx_histogram(hdd_ctx);
			break;
		case WLAN_HDD_NETIF_OPER_HISTORY:
			wlan_hdd_clear_netif_queue_history(hdd_ctx);
			break;
		default:
			ol_txrx_clear_stats(set_value);
		}
		break;
	}
	case QCSAP_START_FW_PROFILING:
		hddLog(LOG1, "QCSAP_START_FW_PROFILING %d", set_value);
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
					WMI_WLAN_PROFILE_TRIGGER_CMDID,
					set_value, DBG_CMD);
		break;
	default:
		hddLog(LOGE, FL("Invalid setparam command %d value %d"),
		       sub_cmd, set_value);
		ret = -EINVAL;
		break;
	}
	EXIT();
	return ret;
}

int
static iw_softap_setparam(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_setparam(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

int
static __iw_softap_getparam(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
	int *value = (int *)extra;
	int sub_cmd = value[0];
	CDF_STATUS status;
	int ret;
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case QCSAP_PARAM_MAX_ASSOC:
		status =
			sme_cfg_get_int(hHal, WNI_CFG_ASSOC_STA_LIMIT,
					(uint32_t *) value);
		if (CDF_STATUS_SUCCESS != status) {
			hddLog(LOGE,
			       FL("failed to get WNI_CFG_ASSOC_STA_LIMIT from cfg %d"),
			       status);
			ret = -EIO;
		}
		break;

	case QCSAP_PARAM_AUTO_CHANNEL:
		*value = (WLAN_HDD_GET_CTX
			(pHostapdAdapter))->config->force_sap_acs;
		break;

	case QCSAP_PARAM_GET_WLAN_DBG:
	{
		cdf_trace_display();
		*value = 0;
		break;
	}

	case QCSAP_PARAM_RTSCTS:
	{
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_ENABLE_RTSCTS,
					     VDEV_CMD);
		break;
	}

	case QCASAP_SHORT_GI:
	{
		*value = (int)sme_get_ht_config(hHal,
						pHostapdAdapter->
						sessionId,
						WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ);
		break;
	}

	case QCSAP_GTX_HT_MCS:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_HT_MCS");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_HT_MCS,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_VHT_MCS:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_VHT_MCS");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_VHT_MCS,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_USRCFG:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_USR_CFG");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_USR_CFG,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_THRE:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_THRE");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_THRE,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_MARGIN:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_MARGIN");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_MARGIN,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_STEP:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_STEP");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_STEP,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_MINTPC:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_MINTPC");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_MINTPC,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_BWMASK:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_BW_MASK");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_BW_MASK,
					     GTX_CMD);
		break;
	}

	case QCASAP_GET_DFS_NOL:
	{
		wlansap_get_dfs_nol(
#ifdef WLAN_FEATURE_MBSSID
			WLAN_HDD_GET_SAP_CTX_PTR
				(pHostapdAdapter)
#else
			pHddCtx->pcds_context
#endif
			);
	}
	break;

	case QCSAP_GET_ACL:
	{
		hddLog(LOG1, FL("QCSAP_GET_ACL"));
		if (hdd_print_acl(pHostapdAdapter) !=
		    CDF_STATUS_SUCCESS) {
			hddLog(LOGE,
			       FL
			       ("QCSAP_GET_ACL returned Error: not completed"));
		}
		*value = 0;
		break;
	}

	case QCASAP_TX_CHAINMASK_CMD:
	{
		hddLog(LOG1, "QCASAP_TX_CHAINMASK_CMD");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_PDEV_PARAM_TX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case QCASAP_RX_CHAINMASK_CMD:
	{
		hddLog(LOG1, "QCASAP_RX_CHAINMASK_CMD");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_PDEV_PARAM_RX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case QCASAP_NSS_CMD:
	{
		hddLog(LOG1, "QCASAP_NSS_CMD");
		*value = wma_cli_get_command(pHostapdAdapter->sessionId,
					     WMI_VDEV_PARAM_NSS,
					     VDEV_CMD);
		break;
	}
	case QCASAP_GET_TEMP_CMD:
	{
		hddLog(LOG1, "QCASAP_GET_TEMP_CMD");
		ret = wlan_hdd_get_temperature(pHostapdAdapter, value);
		break;
	}
	case QCSAP_GET_FW_PROFILE_DATA:
		hddLog(LOG1, "QCSAP_GET_FW_PROFILE_DATA");
		ret = wma_cli_set_command(pHostapdAdapter->sessionId,
				WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
				0, DBG_CMD);
		break;
	default:
		hddLog(LOGE, FL("Invalid getparam command %d"), sub_cmd);
		ret = -EINVAL;
		break;

	}
	EXIT();
	return ret;
}

int
static iw_softap_getparam(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_getparam(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/* Usage:
    BLACK_LIST  = 0
    WHITE_LIST  = 1
    ADD MAC = 0
    REMOVE MAC  = 1

    mac addr will be accepted as a 6 octet mac address with each octet inputted in hex
    for e.g. 00:0a:f5:11:22:33 will be represented as 0x00 0x0a 0xf5 0x11 0x22 0x33
    while using this ioctl

    Syntax:
    iwpriv softap.0 modify_acl
    <6 octet mac addr> <list type> <cmd type>

    Examples:
    eg 1. to add a mac addr 00:0a:f5:89:89:90 to the black list
    iwpriv softap.0 modify_acl 0x00 0x0a 0xf5 0x89 0x89 0x90 0 0
    eg 2. to delete a mac addr 00:0a:f5:89:89:90 from white list
    iwpriv softap.0 modify_acl 0x00 0x0a 0xf5 0x89 0x89 0x90 1 1
 */
static
int __iw_softap_modify_acl(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t cds_ctx;
#endif
	uint8_t *value = (uint8_t *) extra;
	uint8_t pPeerStaMac[CDF_MAC_ADDR_SIZE];
	int listType, cmd, i;
	int ret;
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

#ifndef WLAN_FEATURE_MBSSID
	cds_ctx =  hdd_ctx->pcds_context;
	if (NULL == cds_ctx) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: Vos Context is NULL", __func__);
		return -EINVAL;
	}
#endif

	for (i = 0; i < CDF_MAC_ADDR_SIZE; i++)
		pPeerStaMac[i] = *(value + i);

	listType = (int)(*(value + i));
	i++;
	cmd = (int)(*(value + i));

	hddLog(LOG1, FL("Modify ACL mac:" MAC_ADDRESS_STR " type: %d cmd: %d"),
	       MAC_ADDR_ARRAY(pPeerStaMac), listType, cmd);

#ifdef WLAN_FEATURE_MBSSID
	cdf_status =
		wlansap_modify_acl(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter),
				   pPeerStaMac, (eSapACLType) listType,
				   (eSapACLCmdType) cmd);
#else
	cdf_status =
		wlansap_modify_acl(p_cds_context, pPeerStaMac,
				   (eSapACLType) listType, (eSapACLCmdType) cmd);
#endif
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		hddLog(LOGE, FL("Modify ACL failed"));
		ret = -EIO;
	}
	EXIT();
	return ret;
}

static
int iw_softap_modify_acl(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_modify_acl(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

int
static __iw_softap_getchannel(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx;
	int *value = (int *)extra;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	*value = 0;
	if (test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags))
		*value = (WLAN_HDD_GET_AP_CTX_PTR(
					pHostapdAdapter))->operatingChannel;
	EXIT();
	return 0;
}

int
static iw_softap_getchannel(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_getchannel(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

int
static __iw_softap_set_max_tx_power(struct net_device *dev,
				    struct iw_request_info *info,
				    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
	int *value = (int *)extra;
	int set_value;
	int ret;
	struct cdf_mac_addr bssid = CDF_MAC_ADDR_BROADCAST_INITIALIZER;
	struct cdf_mac_addr selfMac = CDF_MAC_ADDR_BROADCAST_INITIALIZER;

	ENTER();

	if (NULL == value)
		return -ENOMEM;

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* Assign correct slef MAC address */
	cdf_copy_macaddr(&bssid, &pHostapdAdapter->macAddressCurrent);
	cdf_copy_macaddr(&selfMac, &pHostapdAdapter->macAddressCurrent);

	set_value = value[0];
	if (CDF_STATUS_SUCCESS !=
	    sme_set_max_tx_power(hHal, bssid, selfMac, set_value)) {
		hddLog(LOGE, FL("Setting maximum tx power failed"));
		return -EIO;
	}
	EXIT();
	return 0;
}

int
static iw_softap_set_max_tx_power(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_set_max_tx_power(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

int
static __iw_softap_set_tx_power(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
	hdd_context_t *hdd_ctx;
	int *value = (int *)extra;
	int set_value;
	struct cdf_mac_addr bssid;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (NULL == value)
		return -ENOMEM;

	cdf_copy_macaddr(&bssid, &pHostapdAdapter->macAddressCurrent);

	set_value = value[0];
	if (CDF_STATUS_SUCCESS !=
	    sme_set_tx_power(hHal, pHostapdAdapter->sessionId, bssid,
			     pHostapdAdapter->device_mode, set_value)) {
		hddLog(LOGE, FL("Setting tx power failed"));
		return -EIO;
	}
	EXIT();
	return 0;
}

int
static iw_softap_set_tx_power(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_set_tx_power(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#define IS_BROADCAST_MAC(x) (((x[0] & x[1] & x[2] & x[3] & x[4] & x[5]) == 0xff) ? 1 : 0)

int
static __iw_softap_getassoc_stamacaddr(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_station_info_t *pStaInfo = pHostapdAdapter->aStaInfo;
	hdd_context_t *hdd_ctx;
	char *buf;
	int cnt = 0;
	int left;
	int ret;
	/* maclist_index must be u32 to match userspace */
	u32 maclist_index;

	ENTER();

	/*
	 * NOTE WELL: this is a "get" ioctl but it uses an even ioctl
	 * number, and even numbered iocts are supposed to have "set"
	 * semantics.  Hence the wireless extensions support in the kernel
	 * won't correctly copy the result to userspace, so the ioctl
	 * handler itself must copy the data.  Output format is 32-bit
	 * record length, followed by 0 or more 6-byte STA MAC addresses.
	 *
	 * Further note that due to the incorrect semantics, the "iwpriv"
	 * userspace application is unable to correctly invoke this API,
	 * hence it is not registered in the hostapd_private_args.  This
	 * API can only be invoked by directly invoking the ioctl() system
	 * call.
	 */

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* make sure userspace allocated a reasonable buffer size */
	if (wrqu->data.length < sizeof(maclist_index)) {
		hddLog(LOG1, FL("invalid userspace buffer"));
		return -EINVAL;
	}

	/* allocate local buffer to build the response */
	buf = kmalloc(wrqu->data.length, GFP_KERNEL);
	if (!buf) {
		hddLog(LOG1, FL("failed to allocate response buffer"));
		return -ENOMEM;
	}

	/* start indexing beyond where the record count will be written */
	maclist_index = sizeof(maclist_index);
	left = wrqu->data.length - maclist_index;

	spin_lock_bh(&pHostapdAdapter->staInfo_lock);
	while ((cnt < WLAN_MAX_STA_COUNT) && (left >= CDF_MAC_ADDR_SIZE)) {
		if ((pStaInfo[cnt].isUsed) &&
		    (!IS_BROADCAST_MAC(pStaInfo[cnt].macAddrSTA.bytes))) {
			memcpy(&buf[maclist_index], &(pStaInfo[cnt].macAddrSTA),
			       CDF_MAC_ADDR_SIZE);
			maclist_index += CDF_MAC_ADDR_SIZE;
			left -= CDF_MAC_ADDR_SIZE;
		}
		cnt++;
	}
	spin_unlock_bh(&pHostapdAdapter->staInfo_lock);

	*((u32 *) buf) = maclist_index;
	wrqu->data.length = maclist_index;
	if (copy_to_user(wrqu->data.pointer, buf, maclist_index)) {
		hddLog(LOG1, FL("failed to copy response to user buffer"));
		ret = -EFAULT;
	}
	kfree(buf);
	EXIT();
	return ret;
}

int
static iw_softap_getassoc_stamacaddr(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_getassoc_stamacaddr(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/* Usage:
    mac addr will be accepted as a 6 octet mac address with each octet inputted in hex
    for e.g. 00:0a:f5:11:22:33 will be represented as 0x00 0x0a 0xf5 0x11 0x22 0x33
    while using this ioctl

    Syntax:
    iwpriv softap.0 disassoc_sta <6 octet mac address>

    e.g.
    disassociate sta with mac addr 00:0a:f5:11:22:33 from softap
    iwpriv softap.0 disassoc_sta 0x00 0x0a 0xf5 0x11 0x22 0x33
 */

int
static __iw_softap_disassoc_sta(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx;
	uint8_t *peerMacAddr;
	int ret;

	ENTER();

	if (!capable(CAP_NET_ADMIN)) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			FL("permission check failed"));
		return -EPERM;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* iwpriv tool or framework calls this ioctl with
	 * data passed in extra (less than 16 octets);
	 */
	peerMacAddr = (uint8_t *) (extra);

	hddLog(LOG1, FL("data " MAC_ADDRESS_STR),
	       MAC_ADDR_ARRAY(peerMacAddr));
	hdd_softap_sta_disassoc(pHostapdAdapter, peerMacAddr);
	EXIT();
	return 0;
}

int
static iw_softap_disassoc_sta(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_disassoc_sta(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_get_char_setnone() - Generic "get char" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_char_setnone(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int ret;
	int sub_cmd = wrqu->data.flags;
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	switch (sub_cmd) {
	case QCSAP_GET_STATS:
		hdd_wlan_get_stats(adapter, &(wrqu->data.length),
					extra, WE_MAX_STR_LEN);
		break;
	case QCSAP_LIST_FW_PROFILE:
		hdd_wlan_list_fw_profile(&(wrqu->data.length),
					extra, WE_MAX_STR_LEN);
		break;
	}

	EXIT();
	return ret;
}

static int iw_get_char_setnone(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_char_setnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int wlan_hdd_set_force_acs_ch_range(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int *value = (int *)extra;

	if (!capable(CAP_NET_ADMIN)) {
		hddLog(LOGE, FL("permission check failed"));
		return -EPERM;
	}

	if (wlan_hdd_validate_operation_channel(adapter, value[0]) !=
					 CDF_STATUS_SUCCESS ||
		wlan_hdd_validate_operation_channel(adapter, value[1]) !=
					 CDF_STATUS_SUCCESS) {
		return -EINVAL;
	} else {
		hdd_ctx->config->force_sap_acs_st_ch = value[0];
		hdd_ctx->config->force_sap_acs_end_ch = value[1];
	}

	return 0;
}

static int iw_softap_set_force_acs_ch_range(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	int ret;
	cds_ssr_protect(__func__);
	ret = wlan_hdd_set_force_acs_ch_range(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);
	return ret;
}

static int __iw_softap_get_channel_list(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	uint32_t num_channels = 0;
	uint8_t i = 0;
	uint8_t bandStartChannel = RF_CHAN_1;
	uint8_t bandEndChannel = RF_CHAN_184;
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
	tpChannelListInfo channel_list = (tpChannelListInfo) extra;
	eCsrBand curBand = eCSR_BAND_ALL;
	hdd_context_t *hdd_ctx;
	int ret;

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (CDF_STATUS_SUCCESS != sme_get_freq_band(hHal, &curBand)) {
		hddLog(LOGE, FL("not able get the current frequency band"));
		return -EIO;
	}
	wrqu->data.length = sizeof(tChannelListInfo);
	ENTER();

	if (eCSR_BAND_24 == curBand) {
		bandStartChannel = RF_CHAN_1;
		bandEndChannel = RF_CHAN_14;
	} else if (eCSR_BAND_5G == curBand) {
		bandStartChannel = RF_CHAN_36;
		bandEndChannel = RF_CHAN_184;
	}

	hddLog(LOG1, FL("curBand = %d, StartChannel = %hu, EndChannel = %hu "),
	       curBand, bandStartChannel, bandEndChannel);

	for (i = bandStartChannel; i <= bandEndChannel; i++) {
		if ((CHANNEL_STATE_ENABLE == reg_channels[i].enabled) ||
		    (CHANNEL_STATE_DFS == reg_channels[i].enabled)) {
			channel_list->channels[num_channels] =
				rf_channels[i].channelNum;
			num_channels++;
		}
	}

	hddLog(LOG1, FL(" number of channels %d"), num_channels);

	if (num_channels > IW_MAX_FREQUENCIES) {
		num_channels = IW_MAX_FREQUENCIES;
	}

	channel_list->num_channels = num_channels;
	EXIT();

	return 0;
}

int iw_softap_get_channel_list(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_get_channel_list(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static
int __iw_get_genie(struct net_device *dev,
		   struct iw_request_info *info,
		   union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx;
	int ret;
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t cds_ctx;
#endif
	CDF_STATUS status;
	uint32_t length = DOT11F_IE_RSN_MAX_LEN;
	uint8_t genIeBytes[DOT11F_IE_RSN_MAX_LEN];

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

#ifndef WLAN_FEATURE_MBSSID
	cds_ctx = hdd_ctx->pcds_context;
	if (NULL == cds_ctx) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: vos context is not valid ", __func__);
		return -EINVAL;
	}
#endif

	/*
	 * Actually retrieve the RSN IE from CSR.
	 * (We previously sent it down in the CSR Roam Profile.)
	 */
	status = wlan_sap_getstation_ie_information(
#ifdef WLAN_FEATURE_MBSSID
		WLAN_HDD_GET_SAP_CTX_PTR
			(pHostapdAdapter),
#else
		cds_ctx,
#endif
		&length, genIeBytes);
	if (status == CDF_STATUS_SUCCESS) {
		length = CDF_MIN(length, DOT11F_IE_RSN_MAX_LEN);
		if (wrqu->data.length < length ||
		copy_to_user(wrqu->data.pointer, (void *)genIeBytes, length)) {
			hddLog(LOG1, FL("failed to copy data to user buffer"));
			return -EFAULT;
		}
		wrqu->data.length = length;
		hddLog(LOG1, FL(" RSN IE of %d bytes returned"),
				wrqu->data.length);
	} else {
		wrqu->data.length = 0;
		hddLog(LOG1, FL(" RSN IE failed to populate"));
	}

	EXIT();
	return 0;
}

static
int iw_get_genie(struct net_device *dev,
		 struct iw_request_info *info,
		 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_genie(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static
int __iw_get_wpspbc_probe_req_ies(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	sQcSapreq_WPSPBCProbeReqIES_t WPSPBCProbeReqIEs;
	hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	hddLog(LOG1, FL("get_WPSPBCProbeReqIEs ioctl"));
	memset((void *)&WPSPBCProbeReqIEs, 0, sizeof(WPSPBCProbeReqIEs));

	WPSPBCProbeReqIEs.probeReqIELen =
		pHddApCtx->WPSPBCProbeReq.probeReqIELen;
	cdf_mem_copy(&WPSPBCProbeReqIEs.probeReqIE,
		     pHddApCtx->WPSPBCProbeReq.probeReqIE,
		     WPSPBCProbeReqIEs.probeReqIELen);
	cdf_mem_copy(&WPSPBCProbeReqIEs.macaddr,
		     pHddApCtx->WPSPBCProbeReq.peerMacAddr,
		     CDF_MAC_ADDR_SIZE);
	if (copy_to_user(wrqu->data.pointer,
			 (void *)&WPSPBCProbeReqIEs,
			 sizeof(WPSPBCProbeReqIEs))) {
		hddLog(LOG1, FL("failed to copy data to user buffer"));
		return -EFAULT;
	}
	wrqu->data.length = 12 + WPSPBCProbeReqIEs.probeReqIELen;
	hddLog(LOG1, FL("Macaddress : " MAC_ADDRESS_STR),
	       MAC_ADDR_ARRAY(WPSPBCProbeReqIEs.macaddr));
	up(&pHddApCtx->semWpsPBCOverlapInd);
	EXIT();
	return 0;
}

static
int iw_get_wpspbc_probe_req_ies(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_wpspbc_probe_req_ies(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_auth_hostap() -
 * This function sets the auth type received from the wpa_supplicant.
 *
 * @dev: pointer to the net device.
 * @info: pointer to the iw_request_info.
 * @wrqu: pointer to the iwreq_data.
 * @extra: pointer to the data.
 *
 * Return: 0 for success, non zero for failure
 */
static
int __iw_set_auth_hostap(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (wrqu->param.flags & IW_AUTH_INDEX) {
	case IW_AUTH_TKIP_COUNTERMEASURES:
	{
		if (wrqu->param.value) {
			hddLog(LOG2,
			       "Counter Measure started %d", wrqu->param.value);
			pWextState->mTKIPCounterMeasures =
				TKIP_COUNTER_MEASURE_STARTED;
		} else {
			hddLog(LOG2,
			       "Counter Measure stopped=%d", wrqu->param.value);
			pWextState->mTKIPCounterMeasures =
				TKIP_COUNTER_MEASURE_STOPED;
		}

		hdd_softap_tkip_mic_fail_counter_measure(pAdapter,
							 wrqu->param.
							 value);
	}
	break;

	default:

		hddLog(LOGW, FL("called with unsupported auth type %d"),
		       wrqu->param.flags & IW_AUTH_INDEX);
		break;
	}

	EXIT();
	return 0;
}

/**
 * iw_set_auth_hostap() - Wrapper function to protect __iw_set_auth_hostap
 *			from the SSR.
 *
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int
iw_set_auth_hostap(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_auth_hostap(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_ap_encodeext() - set ap encode
 *
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int __iw_set_ap_encodeext(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t cds_ctx;
#endif
	hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
	hdd_context_t *hdd_ctx;
	int ret;
	CDF_STATUS vstatus;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
	int key_index;
	struct iw_point *encoding = &wrqu->encoding;
	tCsrRoamSetKey setKey;
	int i;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

#ifndef WLAN_FEATURE_MBSSID
	cds_ctx = hdd_ctx->pcds_context;
	if (NULL == cds_ctx) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: pVosContext is NULL", __func__);
		return -EINVAL;
	}
#endif

	key_index = encoding->flags & IW_ENCODE_INDEX;

	if (key_index > 0) {
		/*Convert from 1-based to 0-based keying */
		key_index--;
	}
	if (!ext->key_len)
		return ret;

	cdf_mem_zero(&setKey, sizeof(tCsrRoamSetKey));

	setKey.keyId = key_index;
	setKey.keyLength = ext->key_len;

	if (ext->key_len <= CSR_MAX_KEY_LEN) {
		cdf_mem_copy(&setKey.Key[0], ext->key, ext->key_len);
	}

	if (ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
		/*Key direction for group is RX only */
		setKey.keyDirection = eSIR_RX_ONLY;
		cdf_set_macaddr_broadcast(&setKey.peerMac);
	} else {

		setKey.keyDirection = eSIR_TX_RX;
		cdf_mem_copy(setKey.peerMac.bytes, ext->addr.sa_data,
			     CDF_MAC_ADDR_SIZE);
	}
	if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
		setKey.keyDirection = eSIR_TX_DEFAULT;
		cdf_mem_copy(setKey.peerMac.bytes, ext->addr.sa_data,
			     CDF_MAC_ADDR_SIZE);
	}

	/*For supplicant pae role is zero */
	setKey.paeRole = 0;

	switch (ext->alg) {
	case IW_ENCODE_ALG_NONE:
		setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
		break;

	case IW_ENCODE_ALG_WEP:
		setKey.encType =
			(ext->key_len ==
			 5) ? eCSR_ENCRYPT_TYPE_WEP40 : eCSR_ENCRYPT_TYPE_WEP104;
		pHddApCtx->uPrivacy = 1;
		hddLog(LOG1, FL("uPrivacy=%d"), pHddApCtx->uPrivacy);
		break;

	case IW_ENCODE_ALG_TKIP:
	{
		uint8_t *pKey = &setKey.Key[0];

		setKey.encType = eCSR_ENCRYPT_TYPE_TKIP;

		cdf_mem_zero(pKey, CSR_MAX_KEY_LEN);

		/*Supplicant sends the 32bytes key in this order

		   |--------------|----------|----------|
		 |   Tk1        |TX-MIC    |  RX Mic  |
		 |||--------------|----------|----------|
		   <---16bytes---><--8bytes--><--8bytes-->

		 */
		/*Sme expects the 32 bytes key to be in the below order

		   |--------------|----------|----------|
		 |   Tk1        |RX-MIC    |  TX Mic  |
		 |||--------------|----------|----------|
		   <---16bytes---><--8bytes--><--8bytes-->
		 */
		/* Copy the Temporal Key 1 (TK1) */
		cdf_mem_copy(pKey, ext->key, 16);

		/*Copy the rx mic first */
		cdf_mem_copy(&pKey[16], &ext->key[24], 8);

		/*Copy the tx mic */
		cdf_mem_copy(&pKey[24], &ext->key[16], 8);

	}
	break;

	case IW_ENCODE_ALG_CCMP:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES;
		break;

	default:
		setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
		break;
	}

	hddLog(LOG1, FL(":EncryptionType:%d key_len:%d, KeyId:%d"),
	       setKey.encType, setKey.keyLength, setKey.keyId);
	for (i = 0; i < ext->key_len; i++)
		hddLog(LOG1, "%02x", setKey.Key[i]);

#ifdef WLAN_FEATURE_MBSSID
	vstatus =
		wlansap_set_key_sta(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter),
				    &setKey);
#else
	vstatus = wlansap_set_key_sta(cds_ctx, &setKey);
#endif

	if (vstatus != CDF_STATUS_SUCCESS) {
		hddLog(LOGE, FL("wlansap_set_key_sta failed, status= %d"),
		       vstatus);
		ret = -EINVAL;
	}
	EXIT();
	return ret;
}

/**
 * iw_set_ap_encodeext() - Wrapper function to protect __iw_set_ap_encodeext
 *			from the SSR.
 *
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int iw_set_ap_encodeext(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_ap_encodeext(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_ap_mlme() - set ap mlme
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu; pointer to iwreq_data
 * @extra: extra
 *
 * Return; 0 on success, error number otherwise
 */
static int __iw_set_ap_mlme(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	return 0;
}

/**
 * iw_set_ap_mlme() - SSR wrapper for __iw_set_ap_mlme
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu; pointer to iwreq_data
 * @extra: extra
 *
 * Return; 0 on success, error number otherwise
 */
static int iw_set_ap_mlme(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu,
			  char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_ap_mlme(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_ap_rts_threshold() - get ap rts threshold
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int __iw_get_ap_rts_threshold(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra) {

	hdd_adapter_t *pHostapdAdapter = netdev_priv(dev);
	int ret;
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;
	ret = hdd_wlan_get_rts_threshold(pHostapdAdapter, wrqu);

	return ret;
}

/**
 * iw_get_ap_rts_threshold() - Wrapper function to protect
 *			__iw_get_ap_rts_threshold from the SSR.
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int iw_get_ap_rts_threshold(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_ap_rts_threshold(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_ap_frag_threshold() - get ap fragmentation threshold
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int __iw_get_ap_frag_threshold(struct net_device *dev,
				    struct iw_request_info *info,
				    union iwreq_data *wrqu, char *extra) {

	hdd_adapter_t *pHostapdAdapter = netdev_priv(dev);
	hdd_context_t *hdd_ctx;
	int ret = 0;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_wlan_get_frag_threshold(pHostapdAdapter, wrqu);

	return ret;
}

/**
 * iw_get_ap_frag_threshold() - Wrapper function to protect
 *			__iw_get_ap_frag_threshold from the SSR.
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int iw_get_ap_frag_threshold(struct net_device *dev,
				    struct iw_request_info *info,
				    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_ap_frag_threshold(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_ap_freq() - get ap frequency
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int __iw_get_ap_freq(struct net_device *dev,
			  struct iw_request_info *info, struct iw_freq *fwrq,
			  char *extra) {
	uint32_t status = false, channel = 0, freq = 0;
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx;
	tHalHandle hHal;
	hdd_hostapd_state_t *pHostapdState;
	hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);
	hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);

	if (pHostapdState->bssState == BSS_STOP) {
		if (sme_cfg_get_int(hHal, WNI_CFG_CURRENT_CHANNEL, &channel)
		    != CDF_STATUS_SUCCESS) {
			return -EIO;
		} else {
			status = hdd_wlan_get_freq(channel, &freq);
			if (true == status) {
				/* Set Exponent parameter as 6 (MHZ) in struct
				 * iw_freq * iwlist & iwconfig command
				 * shows frequency into proper
				 * format (2.412 GHz instead of 246.2 MHz)
				 */
				fwrq->m = freq;
				fwrq->e = MHZ;
			}
		}
	} else {
		channel = pHddApCtx->operatingChannel;
		status = hdd_wlan_get_freq(channel, &freq);
		if (true == status) {
			/* Set Exponent parameter as 6 (MHZ) in struct iw_freq
			 * iwlist & iwconfig command shows frequency into proper
			 * format (2.412 GHz instead of 246.2 MHz)*/
			fwrq->m = freq;
			fwrq->e = MHZ;
		}
	}
	EXIT();
	return 0;
}

/**
 * iw_get_ap_freq() - Wrapper function to protect
 *                    __iw_get_ap_freq from the SSR.
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int iw_get_ap_freq(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_freq *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_ap_freq(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_mode() - get mode
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int __iw_get_mode(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra) {

	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	wrqu->mode = IW_MODE_MASTER;

	return ret;
}

/**
 * iw_get_mode() - Wrapper function to protect __iw_get_mode from the SSR.
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int iw_get_mode(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_mode(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int
__iw_softap_setwpsie(struct net_device *dev,
		     struct iw_request_info *info,
		     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t cds_ctx;
#endif
	hdd_hostapd_state_t *pHostapdState;
	CDF_STATUS cdf_ret_status = CDF_STATUS_SUCCESS;
	uint8_t *wps_genie;
	uint8_t *fwps_genie;
	uint8_t *pos;
	tpSap_WPSIE pSap_WPSIe;
	uint8_t WPSIeType;
	uint16_t length;
	struct iw_point s_priv_data;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	if (!capable(CAP_NET_ADMIN)) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			FL("permission check failed"));
		return -EPERM;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

#ifndef WLAN_FEATURE_MBSSID
	cds_ctx = hdd_ctx->pcds_context;
	if (NULL == cds_ctx) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: HDD context is not valid ", __func__);
		return -EINVAL;
	}
#endif

	/* helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&s_priv_data, wrqu)) {
		return -EINVAL;
	}

	if ((NULL == s_priv_data.pointer) ||
	    (s_priv_data.length < QCSAP_MAX_WSC_IE)) {
		return -EINVAL;
	}

	wps_genie = mem_alloc_copy_from_user_helper(s_priv_data.pointer,
						    s_priv_data.length);

	if (NULL == wps_genie) {
		hddLog(LOG1, FL("failed to alloc mem and copy data"));
		return -EFAULT;
	}

	fwps_genie = wps_genie;

	pSap_WPSIe = cdf_mem_malloc(sizeof(tSap_WPSIE));
	if (NULL == pSap_WPSIe) {
		hddLog(LOGE, "CDF unable to allocate memory");
		kfree(fwps_genie);
		return -ENOMEM;
	}
	cdf_mem_zero(pSap_WPSIe, sizeof(tSap_WPSIE));

	hddLog(LOG1, FL("WPS IE type[0x%X] IE[0x%X], LEN[%d]"),
		wps_genie[0], wps_genie[1], wps_genie[2]);
	WPSIeType = wps_genie[0];
	if (wps_genie[0] == eQC_WPS_BEACON_IE) {
		pSap_WPSIe->sapWPSIECode = eSAP_WPS_BEACON_IE;
		wps_genie = wps_genie + 1;
		switch (wps_genie[0]) {
		case DOT11F_EID_WPA:
			if (wps_genie[1] < 2 + 4) {
				cdf_mem_free(pSap_WPSIe);
				kfree(fwps_genie);
				return -EINVAL;
			} else if (memcmp(&wps_genie[2],
				   "\x00\x50\xf2\x04", 4) == 0) {
				hddLog(LOG1, FL("Set WPS BEACON IE(len %d)"),
				       wps_genie[1] + 2);
				pos = &wps_genie[6];
				while (((size_t) pos -
					(size_t) &wps_genie[6]) <
				       (wps_genie[1] - 4)) {
					switch ((uint16_t) (*pos << 8) |
						*(pos + 1)) {
					case HDD_WPS_ELEM_VERSION:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.Version =
							*pos;
						hddLog(LOG1, "WPS version %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.Version);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_VER_PRESENT;
						pos += 1;
						break;

					case HDD_WPS_ELEM_WPS_STATE:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.wpsState =
							*pos;
						hddLog(LOG1, "WPS State %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.wpsState);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_STATE_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_APSETUPLOCK:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						APSetupLocked = *pos;
						hddLog(LOG1, "AP setup lock %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.
						       APSetupLocked);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_APSETUPLOCK_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_SELECTEDREGISTRA:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						SelectedRegistra = *pos;
						hddLog(LOG1,
						       "Selected Registra %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.
						       SelectedRegistra);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_SELECTEDREGISTRA_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_DEVICE_PASSWORD_ID:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						DevicePasswordID =
							(*pos << 8) | *(pos + 1);
						hddLog(LOG1, "Password ID: %x",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.
						       DevicePasswordID);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_DEVICEPASSWORDID_PRESENT;
						pos += 2;
						break;
					case HDD_WPS_ELEM_REGISTRA_CONF_METHODS:
						pos +=
							4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						SelectedRegistraCfgMethod =
							(*pos << 8) | *(pos + 1);
						hddLog(LOG1,
						       "Select Registra Config Methods: %x",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.
						       SelectedRegistraCfgMethod);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_SELECTEDREGISTRACFGMETHOD_PRESENT;
						pos += 2;
						break;

					case HDD_WPS_ELEM_UUID_E:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSBeaconIE.
							     UUID_E, pos,
							     length);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_UUIDE_PRESENT;
						pos += length;
						break;
					case HDD_WPS_ELEM_RF_BANDS:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.RFBand =
							*pos;
						hddLog(LOG1, "RF band: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSBeaconIE.RFBand);
						pSap_WPSIe->sapwpsie.
						sapWPSBeaconIE.
						FieldPresent |=
							WPS_BEACON_RF_BANDS_PRESENT;
						pos += 1;
						break;

					default:
						hddLog(LOGW,
						       "UNKNOWN TLV in WPS IE(%x)",
						       (*pos << 8 |
							*(pos + 1)));
						cdf_mem_free(pSap_WPSIe);
						kfree(fwps_genie);
						return -EINVAL;
					}
				}
			} else {
				hddLog(LOGE, FL("WPS IE Mismatch %X"),
				       wps_genie[0]);
			}
			break;

		default:
			hddLog(LOGE, FL("Set UNKNOWN IE %X"), wps_genie[0]);
			cdf_mem_free(pSap_WPSIe);
			kfree(fwps_genie);
			return 0;
		}
	} else if (wps_genie[0] == eQC_WPS_PROBE_RSP_IE) {
		pSap_WPSIe->sapWPSIECode = eSAP_WPS_PROBE_RSP_IE;
		wps_genie = wps_genie + 1;
		switch (wps_genie[0]) {
		case DOT11F_EID_WPA:
			if (wps_genie[1] < 2 + 4) {
				cdf_mem_free(pSap_WPSIe);
				kfree(fwps_genie);
				return -EINVAL;
			} else if (memcmp(&wps_genie[2], "\x00\x50\xf2\x04", 4)
				   == 0) {
				hddLog(LOG1, FL("Set WPS PROBE RSP IE(len %d)"),
				       wps_genie[1] + 2);
				pos = &wps_genie[6];
				while (((size_t) pos -
					(size_t) &wps_genie[6]) <
				       (wps_genie[1] - 4)) {
					switch ((uint16_t) (*pos << 8) |
						*(pos + 1)) {
					case HDD_WPS_ELEM_VERSION:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.Version =
							*pos;
						hddLog(LOG1, "WPS version %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       Version);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_VER_PRESENT;
						pos += 1;
						break;

					case HDD_WPS_ELEM_WPS_STATE:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.wpsState =
							*pos;
						hddLog(LOG1, "WPS State %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       wpsState);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_STATE_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_APSETUPLOCK:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						APSetupLocked = *pos;
						hddLog(LOG1, "AP setup lock %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       APSetupLocked);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_APSETUPLOCK_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_SELECTEDREGISTRA:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						SelectedRegistra = *pos;
						hddLog(LOG1,
						       "Selected Registra %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       SelectedRegistra);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_SELECTEDREGISTRA_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_DEVICE_PASSWORD_ID:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						DevicePasswordID =
							(*pos << 8) | *(pos + 1);
						hddLog(LOG1, "Password ID: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       DevicePasswordID);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_DEVICEPASSWORDID_PRESENT;
						pos += 2;
						break;
					case HDD_WPS_ELEM_REGISTRA_CONF_METHODS:
						pos +=
							4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						SelectedRegistraCfgMethod =
							(*pos << 8) | *(pos + 1);
						hddLog(LOG1,
						       "Select Registra Config Methods: %x",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       SelectedRegistraCfgMethod);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_SELECTEDREGISTRACFGMETHOD_PRESENT;
						pos += 2;
						break;
					case HDD_WPS_ELEM_RSP_TYPE:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						ResponseType = *pos;
						hddLog(LOG1,
						       "Config Methods: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       ResponseType);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_RESPONSETYPE_PRESENT;
						pos += 1;
						break;
					case HDD_WPS_ELEM_UUID_E:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     UUID_E, pos,
							     length);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_UUIDE_PRESENT;
						pos += length;
						break;

					case HDD_WPS_ELEM_MANUFACTURER:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						Manufacture.num_name =
							length;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     Manufacture.name,
							     pos, length);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_MANUFACTURE_PRESENT;
						pos += length;
						break;

					case HDD_WPS_ELEM_MODEL_NAME:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.ModelName.
						num_text = length;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     ModelName.text,
							     pos, length);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_MODELNAME_PRESENT;
						pos += length;
						break;
					case HDD_WPS_ELEM_MODEL_NUM:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						ModelNumber.num_text =
							length;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     ModelNumber.text,
							     pos, length);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_MODELNUMBER_PRESENT;
						pos += length;
						break;
					case HDD_WPS_ELEM_SERIAL_NUM:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						SerialNumber.num_text =
							length;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     SerialNumber.text,
							     pos, length);
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_SERIALNUMBER_PRESENT;
						pos += length;
						break;
					case HDD_WPS_ELEM_PRIMARY_DEVICE_TYPE:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						PrimaryDeviceCategory =
							(*pos << 8 | *(pos + 1));
						hddLog(LOG1,
						       "primary dev category: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       PrimaryDeviceCategory);
						pos += 2;

						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     PrimaryDeviceOUI,
							     pos,
							     HDD_WPS_DEVICE_OUI_LEN);
						hddLog(LOG1,
						       "primary dev oui: %02x, %02x, %02x, %02x",
						       pos[0], pos[1], pos[2],
						       pos[3]);
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						DeviceSubCategory =
							(*pos << 8 | *(pos + 1));
						hddLog(LOG1,
						       "primary dev sub category: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       DeviceSubCategory);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_PRIMARYDEVICETYPE_PRESENT;
						break;
					case HDD_WPS_ELEM_DEVICE_NAME:
						pos += 2;
						length = *pos << 8 | *(pos + 1);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.DeviceName.
						num_text = length;
						cdf_mem_copy(pSap_WPSIe->
							     sapwpsie.
							     sapWPSProbeRspIE.
							     DeviceName.text,
							     pos, length);
						pos += length;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_DEVICENAME_PRESENT;
						break;
					case HDD_WPS_ELEM_CONFIG_METHODS:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						ConfigMethod =
							(*pos << 8) | *(pos + 1);
						hddLog(LOG1,
						       "Config Methods: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.
						       SelectedRegistraCfgMethod);
						pos += 2;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_CONFIGMETHODS_PRESENT;
						break;

					case HDD_WPS_ELEM_RF_BANDS:
						pos += 4;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.RFBand =
							*pos;
						hddLog(LOG1, "RF band: %d",
						       pSap_WPSIe->sapwpsie.
						       sapWPSProbeRspIE.RFBand);
						pos += 1;
						pSap_WPSIe->sapwpsie.
						sapWPSProbeRspIE.
						FieldPresent |=
							WPS_PROBRSP_RF_BANDS_PRESENT;
						break;
					} /* switch */
				}
			} else {
				hddLog(LOGE,
				       FL("WPS IE Mismatch %X"), wps_genie[0]);
			}

		} /* switch */
	}
#ifdef WLAN_FEATURE_MBSSID
	cdf_ret_status =
		wlansap_set_wps_ie(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter),
				   pSap_WPSIe);
#else
	cdf_ret_status = wlansap_set_wps_ie(p_cds_context, pSap_WPSIe);
#endif
	pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);
	if (pHostapdState->bCommit && WPSIeType == eQC_WPS_PROBE_RSP_IE) {
#ifdef WLAN_FEATURE_MBSSID
		wlansap_update_wps_ie(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter));
#else
		wlansap_update_wps_ie(p_cds_context);
#endif
	}

	cdf_mem_free(pSap_WPSIe);
	kfree(fwps_genie);
	EXIT();
	return cdf_ret_status;
}

static int iw_softap_setwpsie(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu,
			      char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_setwpsie(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int
__iw_softap_stopbss(struct net_device *dev,
		    struct iw_request_info *info,
		    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	hdd_context_t *hdd_ctx;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	if (test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags)) {
		hdd_hostapd_state_t *pHostapdState =
			WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);

		cdf_event_reset(&pHostapdState->cdf_stop_bss_event);
#ifdef WLAN_FEATURE_MBSSID
		status =
			wlansap_stop_bss(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter));
#else
		status =
			wlansap_stop_bss((WLAN_HDD_GET_CTX(pHostapdAdapter))->
					 pcds_context);
#endif
		if (CDF_IS_STATUS_SUCCESS(status)) {
			status =
				cdf_wait_single_event(&pHostapdState->
						      cdf_stop_bss_event,
						      10000);

			if (!CDF_IS_STATUS_SUCCESS(status)) {
				hddLog(LOGE,
				       FL("wait for single_event failed!!"));
				CDF_ASSERT(0);
			}
		}
		clear_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);
		cds_decr_session_set_pcl(hdd_ctx,
					     pHostapdAdapter->device_mode,
					     pHostapdAdapter->sessionId);
	}
	EXIT();
	return (status == CDF_STATUS_SUCCESS) ? 0 : -EBUSY;
}

static int iw_softap_stopbss(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_stopbss(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int
__iw_softap_version(struct net_device *dev,
		    struct iw_request_info *info,
		    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = netdev_priv(dev);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	hdd_wlan_get_version(pHostapdAdapter, wrqu, extra);
	EXIT();
	return 0;
}

static int iw_softap_version(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_version(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static
CDF_STATUS hdd_softap_get_sta_info(hdd_adapter_t *pAdapter, uint8_t *pBuf,
				   int buf_len) {
	uint8_t i;
	uint8_t maxSta = 0;
	int len = 0;
	const char sta_info_header[] = "staId staAddress";
	hdd_context_t *hdd_ctx;

	ENTER();

	if (NULL == pAdapter) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: Adapter is NULL", __func__);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return CDF_STATUS_E_FAULT;

	len = scnprintf(pBuf, buf_len, sta_info_header);
	pBuf += len;
	buf_len -= len;

	maxSta = hdd_ctx->config->maxNumberOfPeers;

	for (i = 0; i <= maxSta; i++) {
		if (pAdapter->aStaInfo[i].isUsed) {
			len =
				scnprintf(pBuf, buf_len,
					  "%5d .%02x:%02x:%02x:%02x:%02x:%02x",
					  pAdapter->aStaInfo[i].ucSTAId,
					  pAdapter->aStaInfo[i].macAddrSTA.bytes[0],
					  pAdapter->aStaInfo[i].macAddrSTA.bytes[1],
					  pAdapter->aStaInfo[i].macAddrSTA.bytes[2],
					  pAdapter->aStaInfo[i].macAddrSTA.bytes[3],
					  pAdapter->aStaInfo[i].macAddrSTA.bytes[4],
					  pAdapter->aStaInfo[i].macAddrSTA.
					  bytes[5]);
			pBuf += len;
			buf_len -= len;
		}
		if (WE_GET_STA_INFO_SIZE > buf_len) {
			break;
		}
	}
	EXIT();
	return CDF_STATUS_SUCCESS;
}

static int __iw_softap_get_sta_info(struct net_device *dev,
				    struct iw_request_info *info,
				    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = netdev_priv(dev);
	CDF_STATUS status;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	status =
		hdd_softap_get_sta_info(pHostapdAdapter, extra,
					WE_SAP_MAX_STA_INFO);

	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("Failed to get sta info: %d"), status);
		return -EINVAL;
	}
	wrqu->data.length = strlen(extra);
	EXIT();
	return 0;
}

static int iw_softap_get_sta_info(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu,
				  char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_get_sta_info(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_ap_genie() - set ap wpa/rsn ie
 *
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int __iw_set_ap_genie(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra) {

	hdd_adapter_t *pHostapdAdapter = netdev_priv(dev);
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t cds_ctx;
#endif
	hdd_context_t *hdd_ctx;
	CDF_STATUS cdf_ret_status = CDF_STATUS_SUCCESS;
	uint8_t *genie = (uint8_t *)extra;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

#ifndef WLAN_FEATURE_MBSSID
	cds_ctx = hdd_ctx->pcds_context;
	if (NULL == cds_ctx) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: CDF Context is NULL", __func__);
		return -EINVAL;
	}
#endif

	if (!wrqu->data.length) {
		EXIT();
		return 0;
	}

	switch (genie[0]) {
	case DOT11F_EID_WPA:
	case DOT11F_EID_RSN:
		if ((WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy == 0) {
			hdd_softap_deregister_bc_sta(pHostapdAdapter);
			hdd_softap_register_bc_sta(pHostapdAdapter, 1);
		}
		(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy = 1;
#ifdef WLAN_FEATURE_MBSSID
		cdf_ret_status =
			wlansap_set_wparsn_ies(WLAN_HDD_GET_SAP_CTX_PTR
						       (pHostapdAdapter), genie,
					       wrqu->data.length);
#else
		cdf_ret_status =
			wlansap_set_wparsn_ies(cds_ctx, genie,
					       wrqu->data.length);
#endif
		break;

	default:
		hddLog(LOGE, FL("Set UNKNOWN IE %X"), genie[0]);
		cdf_ret_status = 0;
	}

	EXIT();
	return cdf_ret_status;
}

/**
 * iw_set_ap_genie() - Wrapper function to protect __iw_set_ap_genie
 *                      from the SSR.
 *
 * @dev - Pointer to the net device.
 * @info - Pointer to the iw_request_info.
 * @wrqu - Pointer to the iwreq_data.
 * @extra - Pointer to the data.
 *
 * Return: 0 for success, non zero for failure.
 */
static int
iw_set_ap_genie(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_ap_genie(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static
int __iw_get_softap_linkspeed(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx;
	char *pLinkSpeed = (char *)extra;
	uint32_t link_speed = 0;
	int len = sizeof(uint32_t) + 1;
	tSirMacAddr macAddress;
	char pmacAddress[MAC_ADDRESS_STR_LEN + 1];
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	int rc, valid, i;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	valid = wlan_hdd_validate_context(hdd_ctx);
	if (0 != valid)
		return valid;

	hddLog(LOG1, FL("wrqu->data.length(%d)"), wrqu->data.length);

	if (wrqu->data.length >= MAC_ADDRESS_STR_LEN - 1) {
		if (copy_from_user((void *)pmacAddress,
				   wrqu->data.pointer, MAC_ADDRESS_STR_LEN)) {
			hddLog(LOG1, FL("failed to copy data to user buffer"));
			return -EFAULT;
		}
		pmacAddress[MAC_ADDRESS_STR_LEN] = '\0';

		if (!mac_pton(pmacAddress, macAddress)) {
			hddLog(LOGE, FL("String to Hex conversion Failed"));
			return -EINVAL;
		}
	}
	/* If no mac address is passed and/or its length is less than 17,
	 * link speed for first connected client will be returned.
	 */
	if (wrqu->data.length < 17 || !CDF_IS_STATUS_SUCCESS(status)) {
		for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
			if (pHostapdAdapter->aStaInfo[i].isUsed &&
			    (!cdf_is_macaddr_broadcast
				  (&pHostapdAdapter->aStaInfo[i].macAddrSTA))) {
				cdf_copy_macaddr(
					(struct cdf_mac_addr *) macAddress,
					 &pHostapdAdapter->aStaInfo[i].
					 macAddrSTA);
				status = CDF_STATUS_SUCCESS;
				break;
			}
		}
	}
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("Invalid peer macaddress"));
		return -EINVAL;
	}
	status = wlan_hdd_get_linkspeed_for_peermac(pHostapdAdapter,
						    macAddress);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, FL("Unable to retrieve SME linkspeed"));
		return -EINVAL;
	}

	link_speed = pHostapdAdapter->ls_stats.estLinkSpeed;

	/* linkspeed in units of 500 kbps */
	link_speed = link_speed / 500;
	wrqu->data.length = len;
	rc = snprintf(pLinkSpeed, len, "%u", link_speed);
	if ((rc < 0) || (rc >= len)) {
		/* encoding or length error? */
		hddLog(LOGE, FL("Unable to encode link speed"));
		return -EIO;
	}
	EXIT();
	return 0;
}

static int
iw_get_softap_linkspeed(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu,
			char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_softap_linkspeed(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const iw_handler hostapd_handler[] = {
	(iw_handler) NULL,      /* SIOCSIWCOMMIT */
	(iw_handler) NULL,      /* SIOCGIWNAME */
	(iw_handler) NULL,      /* SIOCSIWNWID */
	(iw_handler) NULL,      /* SIOCGIWNWID */
	(iw_handler) NULL,      /* SIOCSIWFREQ */
	(iw_handler) iw_get_ap_freq,            /* SIOCGIWFREQ */
	(iw_handler) NULL,      /* SIOCSIWMODE */
	(iw_handler) iw_get_mode,       /* SIOCGIWMODE */
	(iw_handler) NULL,      /* SIOCSIWSENS */
	(iw_handler) NULL,      /* SIOCGIWSENS */
	(iw_handler) NULL,      /* SIOCSIWRANGE */
	(iw_handler) NULL,      /* SIOCGIWRANGE */
	(iw_handler) NULL,      /* SIOCSIWPRIV */
	(iw_handler) NULL,      /* SIOCGIWPRIV */
	(iw_handler) NULL,      /* SIOCSIWSTATS */
	(iw_handler) NULL,      /* SIOCGIWSTATS */
	(iw_handler) NULL,      /* SIOCSIWSPY */
	(iw_handler) NULL,      /* SIOCGIWSPY */
	(iw_handler) NULL,      /* SIOCSIWTHRSPY */
	(iw_handler) NULL,      /* SIOCGIWTHRSPY */
	(iw_handler) NULL,      /* SIOCSIWAP */
	(iw_handler) NULL,      /* SIOCGIWAP */
	(iw_handler) iw_set_ap_mlme,            /* SIOCSIWMLME */
	(iw_handler) NULL,      /* SIOCGIWAPLIST */
	(iw_handler) NULL,      /* SIOCSIWSCAN */
	(iw_handler) NULL,      /* SIOCGIWSCAN */
	(iw_handler) NULL,      /* SIOCSIWESSID */
	(iw_handler) NULL,      /* SIOCGIWESSID */
	(iw_handler) NULL,      /* SIOCSIWNICKN */
	(iw_handler) NULL,      /* SIOCGIWNICKN */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) NULL,      /* SIOCSIWRATE */
	(iw_handler) NULL,      /* SIOCGIWRATE */
	(iw_handler) NULL,      /* SIOCSIWRTS */
	(iw_handler) iw_get_ap_rts_threshold,           /* SIOCGIWRTS */
	(iw_handler) NULL,      /* SIOCSIWFRAG */
	(iw_handler) iw_get_ap_frag_threshold,          /* SIOCGIWFRAG */
	(iw_handler) NULL,      /* SIOCSIWTXPOW */
	(iw_handler) NULL,      /* SIOCGIWTXPOW */
	(iw_handler) NULL,      /* SIOCSIWRETRY */
	(iw_handler) NULL,      /* SIOCGIWRETRY */
	(iw_handler) NULL,      /* SIOCSIWENCODE */
	(iw_handler) NULL,      /* SIOCGIWENCODE */
	(iw_handler) NULL,      /* SIOCSIWPOWER */
	(iw_handler) NULL,      /* SIOCGIWPOWER */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) iw_set_ap_genie,           /* SIOCSIWGENIE */
	(iw_handler) NULL,      /* SIOCGIWGENIE */
	(iw_handler) iw_set_auth_hostap,        /* SIOCSIWAUTH */
	(iw_handler) NULL,      /* SIOCGIWAUTH */
	(iw_handler) iw_set_ap_encodeext,       /* SIOCSIWENCODEEXT */
	(iw_handler) NULL,      /* SIOCGIWENCODEEXT */
	(iw_handler) NULL,      /* SIOCSIWPMKSA */
};

/*
 * Note that the following ioctls were defined with semantics which
 * cannot be handled by the "iwpriv" userspace application and hence
 * they are not included in the hostapd_private_args array
 *     QCSAP_IOCTL_ASSOC_STA_MACADDR
 */

static const struct iw_priv_args hostapd_private_args[] = {
	{
		QCSAP_IOCTL_SETPARAM,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "setparam"
	}, {
		QCSAP_IOCTL_SETPARAM,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, ""
	}, {
		QCSAP_PARAM_MAX_ASSOC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setMaxAssoc"
	}, {
		QCSAP_PARAM_HIDE_SSID,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "hideSSID"
	}, {
		QCSAP_PARAM_SET_MC_RATE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setMcRate"
	},
	{
		QCSAP_PARAM_SET_TXRX_FW_STATS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"txrx_fw_stats"
	}, {
		QCSAP_PARAM_SET_MCC_CHANNEL_LATENCY,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setMccLatency"
	}, {
		QCSAP_PARAM_SET_MCC_CHANNEL_QUOTA,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setMccQuota"
	}, {
		QCSAP_PARAM_SET_CHANNEL_CHANGE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setChanChange"
	}, {
		QCSAP_PARAM_AUTO_CHANNEL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setAutoChannel"
	},
	/* Sub-cmds DBGLOG specific commands */
	{
		QCSAP_DBGLOG_LOG_LEVEL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "dl_loglevel"
	}, {
		QCSAP_DBGLOG_VAP_ENABLE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "dl_vapon"
	}, {
		QCSAP_DBGLOG_VAP_DISABLE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "dl_vapoff"
	}, {
		QCSAP_DBGLOG_MODULE_ENABLE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "dl_modon"
	}, {
		QCSAP_DBGLOG_MODULE_DISABLE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "dl_modoff"
	}, {
		QCSAP_DBGLOG_MOD_LOG_LEVEL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "dl_mod_loglevel"
	}, {
		QCSAP_DBGLOG_TYPE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "dl_type"
	}, {
		QCSAP_DBGLOG_REPORT_ENABLE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "dl_report"
	}, {
		QCASAP_TXRX_FWSTATS_RESET,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "txrx_fw_st_rst"
	}, {
		QCSAP_PARAM_RTSCTS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "enablertscts"
	}, {
		QCASAP_SET_11N_RATE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set11NRates"
	}, {
		QCASAP_SET_VHT_RATE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set11ACRates"
	}, {
		QCASAP_SHORT_GI,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "enable_short_gi"
	}, {
		QCSAP_SET_AMPDU,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "ampdu"
	}, {
		QCSAP_SET_AMSDU,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "amsdu"
	}, {
		QCSAP_GTX_HT_MCS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "gtxHTMcs"
	}, {
		QCSAP_GTX_VHT_MCS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "gtxVHTMcs"
	}, {
		QCSAP_GTX_USRCFG,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "gtxUsrCfg"
	}, {
		QCSAP_GTX_THRE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "gtxThre"
	}, {
		QCSAP_GTX_MARGIN,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "gtxMargin"
	}, {
		QCSAP_GTX_STEP,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "gtxStep"
	}, {
		QCSAP_GTX_MINTPC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "gtxMinTpc"
	}, {
		QCSAP_GTX_BWMASK,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "gtxBWMask"
	}, {
		QCSAP_PARAM_CLR_ACL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setClearAcl"
	}, {
		QCSAP_PARAM_ACL_MODE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setAclMode"
	},
#ifdef QCA_PKT_PROTO_TRACE
	{
		QCASAP_SET_DEBUG_LOG,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setDbgLvl"
	},
#endif /* QCA_PKT_PROTO_TRACE */
	{
		QCASAP_SET_TM_LEVEL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setTmLevel"
	}, {
		QCASAP_SET_DFS_IGNORE_CAC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setDfsIgnoreCAC"
	}, {
		QCASAP_SET_DFS_NOL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setdfsnol"
	}, {
		QCASAP_SET_DFS_TARGET_CHNL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setNextChnl"
	}, {
		QCASAP_SET_RADAR_CMD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setRadar"
	},
	{
		QCSAP_IPA_UC_STAT,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "ipaucstat"
	},
	{
		QCASAP_TX_CHAINMASK_CMD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_txchainmask"
	}, {
		QCASAP_RX_CHAINMASK_CMD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_rxchainmask"
	}, {
		QCASAP_NSS_CMD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_nss"
	}, {
		QCASAP_SET_PHYMODE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setphymode"
	}, {
		QCASAP_DUMP_STATS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "dumpStats"
	}, {
		QCASAP_CLEAR_STATS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "clearStats"
	}, {
		QCSAP_START_FW_PROFILING,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "startProfile"
	}, {
		QCSAP_IOCTL_GETPARAM, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getparam"
	}, {
		QCSAP_IOCTL_GETPARAM, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, ""
	}, {
		QCSAP_PARAM_MAX_ASSOC, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getMaxAssoc"
	}, {
		QCSAP_PARAM_GET_WLAN_DBG, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getwlandbg"
	}, {
		QCSAP_PARAM_AUTO_CHANNEL, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getAutoChannel"
	}, {
		QCSAP_GTX_BWMASK, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxBWMask"
	}, {
		QCSAP_GTX_MINTPC, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxMinTpc"
	}, {
		QCSAP_GTX_STEP, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxStep"
	}, {
		QCSAP_GTX_MARGIN, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxMargin"
	}, {
		QCSAP_GTX_THRE, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxThre"
	}, {
		QCSAP_GTX_USRCFG, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxUsrCfg"
	}, {
		QCSAP_GTX_VHT_MCS, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxVHTMcs"
	}, {
		QCSAP_GTX_HT_MCS, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_gtxHTMcs"
	}, {
		QCASAP_SHORT_GI, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_short_gi"
	}, {
		QCSAP_PARAM_RTSCTS, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_rtscts"
	}, {
		QCASAP_GET_DFS_NOL, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getdfsnol"
	}, {
		QCSAP_GET_ACL, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_acl_list"
	}, {
		QCASAP_TX_CHAINMASK_CMD, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_txchainmask"
	}, {
		QCASAP_RX_CHAINMASK_CMD, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_rxchainmask"
	}, {
		QCASAP_NSS_CMD, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_nss"
	}, {
		QCASAP_GET_TEMP_CMD, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_temp"
	}, {
		QCSAP_GET_FW_PROFILE_DATA, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getProfileData"
	}, {
		QCSAP_IOCTL_GET_STAWPAIE,
		IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, 0,
		"get_staWPAIE"
	}, {
		QCSAP_IOCTL_SETWPAIE,
		IW_PRIV_TYPE_BYTE | QCSAP_MAX_WSC_IE |
		IW_PRIV_SIZE_FIXED, 0, "setwpaie"
	}, {
		QCSAP_IOCTL_STOPBSS, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED, 0,
		"stopbss"
	}, {
		QCSAP_IOCTL_VERSION, 0, IW_PRIV_TYPE_CHAR | QCSAP_MAX_WSC_IE,
		"version"
	}, {
		QCSAP_IOCTL_GET_STA_INFO, 0,
		IW_PRIV_TYPE_CHAR | WE_SAP_MAX_STA_INFO, "get_sta_info"
	}, {
		QCSAP_IOCTL_GET_WPS_PBC_PROBE_REQ_IES,
		IW_PRIV_TYPE_BYTE |
		sizeof(sQcSapreq_WPSPBCProbeReqIES_t) |
		IW_PRIV_SIZE_FIXED, 0, "getProbeReqIEs"
	}
	, {
		QCSAP_IOCTL_GET_CHANNEL, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getchannel"
	}
	, {
		QCSAP_IOCTL_DISASSOC_STA,
		IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 6, 0,
		"disassoc_sta"
	}
	/* handler for main ioctl */
	, {
		QCSAP_PRIV_GET_CHAR_SET_NONE, 0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN, ""
	}
	/* handler for sub-ioctl */
	, {
		QCSAP_GET_STATS, 0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN, "getStats"
	}
	, {
		QCSAP_LIST_FW_PROFILE, 0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN, "listProfile"
	}
	, {
		QCSAP_IOCTL_PRIV_GET_SOFTAP_LINK_SPEED,
		IW_PRIV_TYPE_CHAR | 18,
		IW_PRIV_TYPE_CHAR | 5, "getLinkSpeed"
	}
	, {
		QCSAP_IOCTL_PRIV_SET_THREE_INT_GET_NONE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, ""
	}
	,
	/* handlers for sub-ioctl */
	{
		WE_SET_WLAN_DBG,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "setwlandbg"
	}, {
		WE_SET_SAP_CHANNELS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "setsapchannels"
	}
	,
	/* handlers for sub-ioctl */
	{
		WE_SET_DP_TRACE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "set_dp_trace"
	}
	,
	/* handlers for main ioctl */
	{
		QCSAP_IOCTL_PRIV_SET_VAR_INT_GET_NONE,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS, 0, ""
	}
	, {
		WE_P2P_NOA_CMD, IW_PRIV_TYPE_INT | MAX_VAR_ARGS, 0, "SetP2pPs"
	}
	, {
		WE_UNIT_TEST_CMD, IW_PRIV_TYPE_INT | MAX_VAR_ARGS, 0,
		"setUnitTestCmd"
	}
	,
	/* handlers for main ioctl */
	{
		QCSAP_IOCTL_MODIFY_ACL,
		IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 8, 0, "modify_acl"
	}
	,
	/* handlers for main ioctl */
	{
		QCSAP_IOCTL_GET_CHANNEL_LIST,
		0,
		IW_PRIV_TYPE_BYTE | sizeof(tChannelListInfo),
		"getChannelList"
	}
	,
	/* handlers for main ioctl */
	{
		QCSAP_IOCTL_SET_TX_POWER,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setTxPower"
	}
	,
	/* handlers for main ioctl */
	{
		QCSAP_IOCTL_SET_MAX_TX_POWER,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "setTxMaxPower"
	}
	,
	/* Set HDD CFG Ini param */
	{
		QCSAP_IOCTL_SET_INI_CFG,
		IW_PRIV_TYPE_CHAR | QCSAP_IOCTL_MAX_STR_LEN, 0, "setConfig"
	}
	,
	/* Get HDD CFG Ini param */
	{
		QCSAP_IOCTL_GET_INI_CFG,
		0, IW_PRIV_TYPE_CHAR | QCSAP_IOCTL_MAX_STR_LEN, "getConfig"
	}
	,
	/* handlers for main ioctl */
	{
	/* handlers for main ioctl */
		QCSAP_IOCTL_SET_TWO_INT_GET_NONE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, ""
	}
	,
	/* handlers for sub-ioctl */
#ifdef DEBUG
	{
		QCSAP_IOCTL_SET_FW_CRASH_INJECT,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		0, "crash_inject"
	}
	,
#endif
	{
		QCASAP_SET_RADAR_DBG,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,  "setRadarDbg"
	}
	,
	/* dump dp trace - descriptor or dp trace records */
	{
		QCSAP_IOCTL_DUMP_DP_TRACE_LEVEL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		0, "dump_dp_trace"
	}
	,
	{
		QCSAP_ENABLE_FW_PROFILE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		0, "enableProfile"
	}
	,
	{
		QCSAP_SET_FW_PROFILE_HIST_INTVL,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		0, "set_hist_intvl"
	}
};

static const iw_handler hostapd_private[] = {
	/* set priv ioctl */
	[QCSAP_IOCTL_SETPARAM - SIOCIWFIRSTPRIV] = iw_softap_setparam,
	/* get priv ioctl */
	[QCSAP_IOCTL_GETPARAM - SIOCIWFIRSTPRIV] = iw_softap_getparam,
	/* get station genIE */
	[QCSAP_IOCTL_GET_STAWPAIE - SIOCIWFIRSTPRIV] = iw_get_genie,
	[QCSAP_IOCTL_SETWPAIE - SIOCIWFIRSTPRIV] = iw_softap_setwpsie,
	/* stop bss */
	[QCSAP_IOCTL_STOPBSS - SIOCIWFIRSTPRIV] = iw_softap_stopbss,
	/* get driver version */
	[QCSAP_IOCTL_VERSION - SIOCIWFIRSTPRIV] = iw_softap_version,
	[QCSAP_IOCTL_GET_WPS_PBC_PROBE_REQ_IES - SIOCIWFIRSTPRIV] =
		iw_get_wpspbc_probe_req_ies,
	[QCSAP_IOCTL_GET_CHANNEL - SIOCIWFIRSTPRIV] =
		iw_softap_getchannel,
	[QCSAP_IOCTL_ASSOC_STA_MACADDR - SIOCIWFIRSTPRIV] =
		iw_softap_getassoc_stamacaddr,
	[QCSAP_IOCTL_DISASSOC_STA - SIOCIWFIRSTPRIV] =
		iw_softap_disassoc_sta,
	[QCSAP_PRIV_GET_CHAR_SET_NONE - SIOCIWFIRSTPRIV] =
		iw_get_char_setnone,
	[QCSAP_IOCTL_PRIV_SET_THREE_INT_GET_NONE -
	 SIOCIWFIRSTPRIV] =
		iw_set_three_ints_getnone,
	[QCSAP_IOCTL_PRIV_SET_VAR_INT_GET_NONE -
	 SIOCIWFIRSTPRIV] =
		iw_set_var_ints_getnone,
	[QCSAP_IOCTL_SET_CHANNEL_RANGE - SIOCIWFIRSTPRIV] =
					iw_softap_set_force_acs_ch_range,
	[QCSAP_IOCTL_MODIFY_ACL - SIOCIWFIRSTPRIV] =
		iw_softap_modify_acl,
	[QCSAP_IOCTL_GET_CHANNEL_LIST - SIOCIWFIRSTPRIV] =
		iw_softap_get_channel_list,
	[QCSAP_IOCTL_GET_STA_INFO - SIOCIWFIRSTPRIV] =
		iw_softap_get_sta_info,
	[QCSAP_IOCTL_PRIV_GET_SOFTAP_LINK_SPEED -
	 SIOCIWFIRSTPRIV] =
		iw_get_softap_linkspeed,
	[QCSAP_IOCTL_SET_TX_POWER - SIOCIWFIRSTPRIV] =
		iw_softap_set_tx_power,
	[QCSAP_IOCTL_SET_MAX_TX_POWER - SIOCIWFIRSTPRIV] =
		iw_softap_set_max_tx_power,
	[QCSAP_IOCTL_SET_INI_CFG - SIOCIWFIRSTPRIV] =
		iw_softap_set_ini_cfg,
	[QCSAP_IOCTL_GET_INI_CFG - SIOCIWFIRSTPRIV] =
		iw_softap_get_ini_cfg,
	[QCSAP_IOCTL_SET_TWO_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_softap_set_two_ints_getnone,
};
const struct iw_handler_def hostapd_handler_def = {
	.num_standard = CDF_ARRAY_SIZE(hostapd_handler),
	.num_private = CDF_ARRAY_SIZE(hostapd_private),
	.num_private_args = CDF_ARRAY_SIZE(hostapd_private_args),
	.standard = (iw_handler *) hostapd_handler,
	.private = (iw_handler *) hostapd_private,
	.private_args = hostapd_private_args,
	.get_wireless_stats = NULL,
};

struct net_device_ops net_ops_struct = {
	.ndo_open = hdd_hostapd_open,
	.ndo_stop = hdd_hostapd_stop,
	.ndo_uninit = hdd_hostapd_uninit,
	.ndo_start_xmit = hdd_softap_hard_start_xmit,
	.ndo_tx_timeout = hdd_softap_tx_timeout,
	.ndo_get_stats = hdd_get_stats,
	.ndo_set_mac_address = hdd_hostapd_set_mac_address,
	.ndo_do_ioctl = hdd_ioctl,
	.ndo_change_mtu = hdd_hostapd_change_mtu,
	.ndo_select_queue = hdd_hostapd_select_queue,
};

static int hdd_set_hostapd(hdd_adapter_t *pAdapter)
{
	return CDF_STATUS_SUCCESS;
}

void hdd_set_ap_ops(struct net_device *pWlanHostapdDev)
{
	pWlanHostapdDev->netdev_ops = &net_ops_struct;
}

CDF_STATUS hdd_init_ap_mode(hdd_adapter_t *pAdapter)
{
	hdd_hostapd_state_t *phostapdBuf;
	struct net_device *dev = pAdapter->dev;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	CDF_STATUS status;
#ifdef WLAN_FEATURE_MBSSID
	v_CONTEXT_t p_cds_context = (WLAN_HDD_GET_CTX(pAdapter))->pcds_context;
	v_CONTEXT_t sapContext = NULL;
#endif
	int ret;

	ENTER();

#ifdef WLAN_FEATURE_MBSSID
	sapContext = wlansap_open(p_cds_context);
	if (sapContext == NULL) {
		hddLog(LOGE, ("ERROR: wlansap_open failed!!"));
		return CDF_STATUS_E_FAULT;
	}

	pAdapter->sessionCtx.ap.sapContext = sapContext;

	status = wlansap_start(sapContext);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, ("ERROR: wlansap_start failed!!"));
		wlansap_close(sapContext);
		return status;
	}
#endif

	/* Allocate the Wireless Extensions state structure */
	phostapdBuf = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

	sme_set_curr_device_mode(pHddCtx->hHal, pAdapter->device_mode);

	/* Zero the memory.  This zeros the profile structure. */
	memset(phostapdBuf, 0, sizeof(hdd_hostapd_state_t));

	/* Set up the pointer to the Wireless Extensions state structure */
	/* NOP */
	status = hdd_set_hostapd(pAdapter);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, ("ERROR: hdd_set_hostapd failed!!"));
#ifdef WLAN_FEATURE_MBSSID
		wlansap_close(sapContext);
#endif
		return status;
	}

	status = cdf_event_init(&phostapdBuf->cdf_event);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE, ("ERROR: Hostapd HDD cdf event init failed!!"));
#ifdef WLAN_FEATURE_MBSSID
		wlansap_close(sapContext);
#endif
		return status;
	}

	status = cdf_event_init(&phostapdBuf->cdf_stop_bss_event);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  ("ERROR: Hostapd HDD stop bss event init failed!!"));
#ifdef WLAN_FEATURE_MBSSID
		wlansap_close(sapContext);
#endif
		return status;
	}

	init_completion(&pAdapter->session_close_comp_var);
	init_completion(&pAdapter->session_open_comp_var);

	sema_init(&(WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->semWpsPBCOverlapInd, 1);

	/* Register as a wireless device */
	dev->wireless_handlers = (struct iw_handler_def *)&hostapd_handler_def;

	/* Initialize the data path module */
	status = hdd_softap_init_tx_rx(pAdapter);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGP, FL("hdd_softap_init_tx_rx failed"));
	}

	status = hdd_wmm_adapter_init(pAdapter);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
		       "hdd_wmm_adapter_init() failed code %08d [x%08x]",
		       status, status);
		goto error_wmm_init;
	}

	set_bit(WMM_INIT_DONE, &pAdapter->event_flags);

	ret = wma_cli_set_command(pAdapter->sessionId,
				  WMI_PDEV_PARAM_BURST_ENABLE,
				  pHddCtx->config->enableSifsBurst,
				  PDEV_CMD);

	if (0 != ret) {
		hddLog(LOGE,
		       FL("WMI_PDEV_PARAM_BURST_ENABLE set failed %d"), ret);
	}
	pAdapter->sessionCtx.ap.sapConfig.acs_cfg.acs_mode = false;
	cdf_mem_free(pAdapter->sessionCtx.ap.sapConfig.acs_cfg.ch_list);
	cdf_mem_zero(&pAdapter->sessionCtx.ap.sapConfig.acs_cfg,
						sizeof(struct sap_acs_cfg));
	return status;

error_wmm_init:
	hdd_softap_deinit_tx_rx(pAdapter);
#ifdef WLAN_FEATURE_MBSSID
	wlansap_close(sapContext);
#endif
	EXIT();
	return status;
}

/**
 * hdd_wlan_create_ap_dev() - create an AP-mode device
 * @pHddCtx: Global HDD context
 * @macAddr: MAC address to assign to the interface
 * @iface_name: User-visible name of the interface
 *
 * This function will allocate a Linux net_device and configuration it
 * for an AP mode of operation.  Note that the device is NOT actually
 * registered with the kernel at this time.
 *
 * Return: A pointer to the private data portion of the net_device if
 * the allocation and initialization was successful, NULL otherwise.
 */
hdd_adapter_t *hdd_wlan_create_ap_dev(hdd_context_t *pHddCtx,
				      tSirMacAddr macAddr,
				      uint8_t *iface_name) {
	struct net_device *pWlanHostapdDev = NULL;
	hdd_adapter_t *pHostapdAdapter = NULL;

	hddLog(LOG4, FL("iface_name = %s"), iface_name);

	pWlanHostapdDev =
		alloc_netdev_mq(sizeof(hdd_adapter_t), iface_name,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
				NET_NAME_UNKNOWN,
#endif
				ether_setup,
				NUM_TX_QUEUES);

	if (pWlanHostapdDev != NULL) {
		pHostapdAdapter = netdev_priv(pWlanHostapdDev);

		/* Init the net_device structure */
		ether_setup(pWlanHostapdDev);

		/* Initialize the adapter context to zeros. */
		cdf_mem_zero(pHostapdAdapter, sizeof(hdd_adapter_t));
		pHostapdAdapter->dev = pWlanHostapdDev;
		pHostapdAdapter->pHddCtx = pHddCtx;
		pHostapdAdapter->magic = WLAN_HDD_ADAPTER_MAGIC;

		hddLog(LOG4,
		       FL("pWlanHostapdDev = %p, pHostapdAdapter = %p, concurrency_mode=0x%x"),
		       pWlanHostapdDev,
		       pHostapdAdapter,
		       (int)cds_get_concurrency_mode());

		/* Init the net_device structure */
		strlcpy(pWlanHostapdDev->name, (const char *)iface_name,
			IFNAMSIZ);

		hdd_set_ap_ops(pHostapdAdapter->dev);

		pWlanHostapdDev->watchdog_timeo = HDD_TX_TIMEOUT;
		pWlanHostapdDev->mtu = HDD_DEFAULT_MTU;
		pWlanHostapdDev->tx_queue_len = HDD_NETDEV_TX_QUEUE_LEN;

		cdf_mem_copy(pWlanHostapdDev->dev_addr, (void *)macAddr,
			     sizeof(tSirMacAddr));
		cdf_mem_copy(pHostapdAdapter->macAddressCurrent.bytes,
			     (void *)macAddr, sizeof(tSirMacAddr));

		pHostapdAdapter->offloads_configured = false;
		pWlanHostapdDev->destructor = free_netdev;
		pWlanHostapdDev->ieee80211_ptr = &pHostapdAdapter->wdev;
		pHostapdAdapter->wdev.wiphy = pHddCtx->wiphy;
		pHostapdAdapter->wdev.netdev = pWlanHostapdDev;
		init_completion(&pHostapdAdapter->tx_action_cnf_event);
		init_completion(&pHostapdAdapter->cancel_rem_on_chan_var);
		init_completion(&pHostapdAdapter->rem_on_chan_ready_event);
		init_completion(&pHostapdAdapter->sta_authorized_event);
		init_completion(&pHostapdAdapter->offchannel_tx_event);
		init_completion(&pHostapdAdapter->scan_info.
				abortscan_event_var);

		SET_NETDEV_DEV(pWlanHostapdDev, pHddCtx->parent_dev);
		spin_lock_init(&pHostapdAdapter->pause_map_lock);
	}
	return pHostapdAdapter;
}

/**
 * hdd_register_hostapd() - register hostapd
 * @pAdapter: Pointer to hostapd adapter
 * @rtnl_lock_held: RTNL lock held
 *
 * Return: CDF status
 */
CDF_STATUS hdd_register_hostapd(hdd_adapter_t *pAdapter,
				uint8_t rtnl_lock_held) {
	struct net_device *dev = pAdapter->dev;
	CDF_STATUS status = CDF_STATUS_SUCCESS;

	ENTER();

	if (rtnl_lock_held) {
		if (strnchr(dev->name, strlen(dev->name), '%')) {
			if (dev_alloc_name(dev, dev->name) < 0) {
				hddLog(LOGE, FL("Failed:dev_alloc_name"));
				return CDF_STATUS_E_FAILURE;
			}
		}
		if (register_netdevice(dev)) {
			hddLog(LOGE, FL("Failed:register_netdevice"));
			return CDF_STATUS_E_FAILURE;
		}
	} else {
		if (register_netdev(dev)) {
			hddLog(LOGE, FL("Failed:register_netdev"));
			return CDF_STATUS_E_FAILURE;
		}
	}
	set_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);

	EXIT();
	return status;
}

/**
 * hdd_unregister_hostapd() - unregister hostapd
 * @pAdapter: Pointer to hostapd adapter
 * @rtnl_held: true if rtnl lock held; false otherwise
 *
 * Return: CDF_STATUS enumaration
 */
CDF_STATUS hdd_unregister_hostapd(hdd_adapter_t *pAdapter, bool rtnl_held)
{
#ifdef WLAN_FEATURE_MBSSID
	CDF_STATUS status;
	void *sapContext = WLAN_HDD_GET_SAP_CTX_PTR(pAdapter);
#endif

	ENTER();

	hdd_softap_deinit_tx_rx(pAdapter);

	/* if we are being called during driver unload,
	 * then the dev has already been invalidated.
	 * if we are being called at other times, then we can
	 * detach the wireless device handlers
	 */
	if (pAdapter->dev) {
		if (rtnl_held)
			pAdapter->dev->wireless_handlers = NULL;
		else {
			rtnl_lock();
			pAdapter->dev->wireless_handlers = NULL;
			rtnl_unlock();
		}
	}

#ifdef WLAN_FEATURE_MBSSID
	status = wlansap_stop(sapContext);
	if (!CDF_IS_STATUS_SUCCESS(status))
		hddLog(LOGE, FL("Failed:wlansap_stop"));

	status = wlansap_close(sapContext);
	if (!CDF_IS_STATUS_SUCCESS(status))
		hddLog(LOGE, FL("Failed:WLANSAP_close"));
	pAdapter->sessionCtx.ap.sapContext = NULL;
#endif

	EXIT();
	return 0;
}

/**
 * wlan_hdd_rate_is_11g() - check if rate is 11g rate or not
 * @rate: Rate to be checked
 *
 * Return: true if rate if 11g else false
 */
static bool wlan_hdd_rate_is_11g(u8 rate)
{
	static const u8 gRateArray[8] = {12, 18, 24, 36, 48, 72,
					 96, 108}; /* actual rate * 2 */
	u8 i;
	for (i = 0; i < 8; i++) {
		if (rate == gRateArray[i])
			return true;
	}
	return false;
}

#ifdef QCA_HT_2040_COEX
/**
 * wlan_hdd_get_sap_obss() - Get SAP OBSS enable config based on HT_CAPAB IE
 * @pHostapdAdapter: Pointer to hostapd adapter
 *
 * Return: HT support channel width config value
 */
static bool wlan_hdd_get_sap_obss(hdd_adapter_t *pHostapdAdapter)
{
	uint8_t ht_cap_ie[DOT11F_IE_HTCAPS_MAX_LEN];
	tDot11fIEHTCaps dot11_ht_cap_ie = {0};
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	beacon_data_t *beacon = pHostapdAdapter->sessionCtx.ap.beacon;
	uint8_t *ie = NULL;

	ie = wlan_hdd_cfg80211_get_ie_ptr(beacon->tail, beacon->tail_len,
						WLAN_EID_HT_CAPABILITY);
	if (ie && ie[1]) {
		cdf_mem_copy(ht_cap_ie, &ie[2], DOT11F_IE_HTCAPS_MAX_LEN);
		dot11f_unpack_ie_ht_caps((tpAniSirGlobal)hdd_ctx->hHal,
					ht_cap_ie, ie[1], &dot11_ht_cap_ie);
		return dot11_ht_cap_ie.supportedChannelWidthSet;
	}

	return false;
}
#else
static bool wlan_hdd_get_sap_obss(hdd_adapter_t *pHostapdAdapter)
{
	return false;
}
#endif
/**
 * wlan_hdd_set_channel() - set channel in sap mode
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 * @chandef: Pointer to channel definition structure
 * @channel_type: Channel type
 *
 * Return: 0 for success non-zero for failure
 */
static int wlan_hdd_set_channel(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_chan_def *chandef,
				enum nl80211_channel_type channel_type)
{
	hdd_adapter_t *pAdapter = NULL;
	uint32_t num_ch = 0;
	int channel = 0;
	int channel_seg2 = 0;
	hdd_context_t *pHddCtx;
	int status;

	tSmeConfigParams smeConfig;
	tsap_Config_t *sap_config;

	ENTER();


	if (NULL == dev) {
		hddLog(LOGE, FL("Called with dev = NULL."));
		return -ENODEV;
	}
	pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_CHANNEL,
			 pAdapter->sessionId, channel_type));

	hddLog(LOG1, FL("Device_mode %s(%d)  freq = %d"),
	       hdd_device_mode_to_string(pAdapter->device_mode),
	       pAdapter->device_mode, chandef->chan->center_freq);

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status) {
		hddLog(LOGE, FL("HDD context is not valid"));
		return status;
	}

	/*
	 * Do freq to chan conversion
	 * TODO: for 11a
	 */

	channel = ieee80211_frequency_to_channel(chandef->chan->center_freq);
	if (NL80211_CHAN_WIDTH_80P80 == chandef->width)
		channel_seg2 = ieee80211_frequency_to_channel(chandef->center_freq2);
	else
		channel_seg2 = 0;

	/* Check freq range */
	if ((WNI_CFG_CURRENT_CHANNEL_STAMIN > channel) ||
	    (WNI_CFG_CURRENT_CHANNEL_STAMAX < channel)) {
		hddLog(LOGE,
		       FL("Channel [%d] is outside valid range from %d to %d"),
		       channel, WNI_CFG_CURRENT_CHANNEL_STAMIN,
		       WNI_CFG_CURRENT_CHANNEL_STAMAX);
		return -EINVAL;
	}

	/* Check freq range */

	if ((WNI_CFG_CURRENT_CHANNEL_STAMIN > channel_seg2) ||
	    (WNI_CFG_CURRENT_CHANNEL_STAMAX < channel_seg2)) {
		hddLog(LOGE,
		       FL("Channel [%d] is outside valid range from %d to %d"),
		       channel_seg2, WNI_CFG_CURRENT_CHANNEL_STAMIN,
		       WNI_CFG_CURRENT_CHANNEL_STAMAX);
		return -EINVAL;
	}

	num_ch = WNI_CFG_VALID_CHANNEL_LIST_LEN;

	if ((WLAN_HDD_SOFTAP != pAdapter->device_mode) &&
	    (WLAN_HDD_P2P_GO != pAdapter->device_mode)) {
		if (CDF_STATUS_SUCCESS !=
		    wlan_hdd_validate_operation_channel(pAdapter, channel)) {
			hddLog(LOGE, FL("Invalid Channel [%d]"), channel);
			return -EINVAL;
		}
		hddLog(LOG2,
		       FL("set channel to [%d] for device mode %s(%d)"),
		       channel,
		       hdd_device_mode_to_string(pAdapter->device_mode),
		       pAdapter->device_mode);
	}

	if ((pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
	    || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
	    ) {
		hdd_wext_state_t *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
		tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

		if (eConnectionState_IbssConnected ==
		    pHddStaCtx->conn_info.connState) {
			/* Link is up then return cant set channel */
			hddLog(LOGE,
			       FL("IBSS Associated, can't set the channel"));
			return -EINVAL;
		}

		num_ch = pRoamProfile->ChannelInfo.numOfChannels = 1;
		pHddStaCtx->conn_info.operationChannel = channel;
		pRoamProfile->ChannelInfo.ChannelList =
			&pHddStaCtx->conn_info.operationChannel;
	} else if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
		   || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
		   ) {
		sap_config = &((WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->sapConfig);
		if (WLAN_HDD_P2P_GO == pAdapter->device_mode) {
			if (CDF_STATUS_SUCCESS !=
			    wlan_hdd_validate_operation_channel(pAdapter,
								channel)) {
				hddLog(LOGE,
				       FL("Invalid Channel [%d]"), channel);
				return -EINVAL;
			}
			sap_config->channel = channel;
			sap_config->ch_params.center_freq_seg1 = channel_seg2;
		} else {
			/* set channel to what hostapd configured */
			if (CDF_STATUS_SUCCESS !=
				wlan_hdd_validate_operation_channel(pAdapter,
								channel)) {
				hddLog(LOGE,
				       FL("Invalid Channel [%d]"), channel);
				return -EINVAL;
			}

			sap_config->channel = channel;
			sap_config->ch_params.center_freq_seg1 = channel_seg2;

			cdf_mem_zero(&smeConfig, sizeof(smeConfig));
			sme_get_config_param(pHddCtx->hHal, &smeConfig);
			switch (channel_type) {
			case NL80211_CHAN_HT20:
			case NL80211_CHAN_NO_HT:
				smeConfig.csrConfig.obssEnabled = false;
				if (channel <= 14)
					smeConfig.csrConfig.
						channelBondingMode24GHz =
					eCSR_INI_SINGLE_CHANNEL_CENTERED;
				else
					smeConfig.csrConfig.
						channelBondingMode5GHz =
					eCSR_INI_SINGLE_CHANNEL_CENTERED;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
				sap_config->ch_width_orig =
						eHT_CHANNEL_WIDTH_20MHZ;
#endif
				sap_config->sec_ch = 0;
				break;

			case NL80211_CHAN_HT40MINUS:
				if (channel <= 14)
					smeConfig.csrConfig.
					channelBondingMode24GHz =
					eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY;
				else
					smeConfig.csrConfig.
					channelBondingMode5GHz =
					eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY;
				sap_config->sec_ch = sap_config->channel - 4;
				break;
			case NL80211_CHAN_HT40PLUS:
				if (channel <= 14)
					smeConfig.csrConfig.
					channelBondingMode24GHz =
					eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY;
				else
					smeConfig.csrConfig.
					channelBondingMode5GHz =
					eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY;
				sap_config->sec_ch = sap_config->channel + 4;
				break;
			default:
				hddLog(LOGE,
				       FL("Error!!! Invalid HT20/40 mode !"));
				return -EINVAL;
			}
			smeConfig.csrConfig.obssEnabled = wlan_hdd_get_sap_obss(
								pAdapter);
			sme_update_config(pHddCtx->hHal, &smeConfig);
		}
	} else {
		hddLog(LOGE,
		       FL("Invalid device mode failed to set valid channel"));
		return -EINVAL;
	}
	EXIT();
	return status;
}

/**
 * wlan_hdd_check_11gmode() - check for 11g mode
 * @pIe: Pointer to IE
 * @require_ht: Pointer to require ht
 * @require_vht: Pointer to require vht
 * @pCheckRatesfor11g: Pointer to check rates for 11g mode
 * @pSapHw_mode: SAP HW mode
 *
 * Check for 11g rate and set proper 11g only mode
 *
 * Return: none
 */
static void wlan_hdd_check_11gmode(u8 *pIe, u8 *require_ht, u8 *require_vht,
				   u8 *pCheckRatesfor11g,
				   eCsrPhyMode *pSapHw_mode)
{
	u8 i, num_rates = pIe[0];

	pIe += 1;
	for (i = 0; i < num_rates; i++) {
		if (*pCheckRatesfor11g
		    && (true == wlan_hdd_rate_is_11g(pIe[i] & RATE_MASK))) {
			/* If rate set have 11g rate than change the mode
			 * to 11G
			 */
			*pSapHw_mode = eCSR_DOT11_MODE_11g;
			if (pIe[i] & BASIC_RATE_MASK) {
				/* If we have 11g rate as  basic rate, it
				 * means mode is 11g only mode.
				 */
				*pSapHw_mode = eCSR_DOT11_MODE_11g_ONLY;
				*pCheckRatesfor11g = false;
			}
		} else {
			if ((BASIC_RATE_MASK |
				WLAN_BSS_MEMBERSHIP_SELECTOR_HT_PHY) == pIe[i])
				*require_ht = true;
			else if ((BASIC_RATE_MASK |
				WLAN_BSS_MEMBERSHIP_SELECTOR_VHT_PHY) == pIe[i])
				*require_vht = true;
		}
	}
	return;
}

/**
 * wlan_hdd_add_ie() - add ie
 * @pHostapdAdapter: Pointer to hostapd adapter
 * @genie: Pointer to ie to be added
 * @total_ielen: Pointer to store total ie length
 * @oui: Pointer to oui
 * @oui_size: Size of oui
 *
 * Return: 0 for success non-zero for failure
 */
static int wlan_hdd_add_ie(hdd_adapter_t *pHostapdAdapter, uint8_t *genie,
			   uint8_t *total_ielen, uint8_t *oui,
			   uint8_t oui_size)
{
	uint16_t ielen = 0;
	uint8_t *pIe = NULL;
	beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;

	pIe = wlan_hdd_get_vendor_oui_ie_ptr(oui, oui_size,
					     pBeacon->tail, pBeacon->tail_len);

	if (pIe) {
		ielen = pIe[1] + 2;
		if ((*total_ielen + ielen) <= MAX_GENIE_LEN) {
			cdf_mem_copy(&genie[*total_ielen], pIe, ielen);
		} else {
			hddLog(LOGE,
			       "**Ie Length is too big***");
			return -EINVAL;
		}
		*total_ielen += ielen;
	}
	return 0;
}

/**
 * wlan_hdd_add_hostapd_conf_vsie() - configure vsie in sap mode
 * @pHostapdAdapter: Pointer to hostapd adapter
 * @genie: Pointer to vsie
 * @total_ielen: Pointer to store total ie length
 *
 * Return: none
 */
static void wlan_hdd_add_hostapd_conf_vsie(hdd_adapter_t *pHostapdAdapter,
					   uint8_t *genie,
					   uint8_t *total_ielen)
{
	beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;
	int left = pBeacon->tail_len;
	uint8_t *ptr = pBeacon->tail;
	uint8_t elem_id, elem_len;
	uint16_t ielen = 0;

	if (NULL == ptr || 0 == left)
		return;

	while (left >= 2) {
		elem_id = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left) {
			hddLog(LOGE,
				FL("****Invalid IEs eid = %d elem_len=%d left=%d*****"),
				elem_id, elem_len, left);
			return;
		}
		if (IE_EID_VENDOR == elem_id) {
			/* skipping the VSIE's which we don't want to include or
			 * it will be included by existing code
			 */
			if ((memcmp(&ptr[2], WPS_OUI_TYPE, WPS_OUI_TYPE_SIZE) !=
			     0) &&
#ifdef WLAN_FEATURE_WFD
			    (memcmp(&ptr[2], WFD_OUI_TYPE, WFD_OUI_TYPE_SIZE) !=
			     0) &&
#endif
			    (memcmp
				     (&ptr[2], WHITELIST_OUI_TYPE,
				     WPA_OUI_TYPE_SIZE) != 0)
			    &&
			    (memcmp
				     (&ptr[2], BLACKLIST_OUI_TYPE,
				     WPA_OUI_TYPE_SIZE) != 0)
			    &&
			    (memcmp
				     (&ptr[2], "\x00\x50\xf2\x02",
				     WPA_OUI_TYPE_SIZE) != 0)
			    && (memcmp(&ptr[2], WPA_OUI_TYPE, WPA_OUI_TYPE_SIZE)
				!= 0)
			    && (memcmp(&ptr[2], P2P_OUI_TYPE, P2P_OUI_TYPE_SIZE)
				!= 0)) {
				ielen = ptr[1] + 2;
				if ((*total_ielen + ielen) <= MAX_GENIE_LEN) {
					cdf_mem_copy(&genie[*total_ielen], ptr,
						     ielen);
					*total_ielen += ielen;
				} else {
					hddLog(LOGE,
					       FL("IE Length is too big IEs eid=%d elem_len=%d total_ie_lent=%d"),
					       elem_id, elem_len, *total_ielen);
				}
			}
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
	return;
}

/**
 * wlan_hdd_add_extra_ie() - add extra ies in beacon
 * @pHostapdAdapter: Pointer to hostapd adapter
 * @genie: Pointer to extra ie
 * @total_ielen: Pointer to store total ie length
 * @temp_ie_id: ID of extra ie
 *
 * Return: none
 */
static void wlan_hdd_add_extra_ie(hdd_adapter_t *pHostapdAdapter,
				  uint8_t *genie, uint8_t *total_ielen,
				  uint8_t temp_ie_id)
{
	beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;
	int left = pBeacon->tail_len;
	uint8_t *ptr = pBeacon->tail;
	uint8_t elem_id, elem_len;
	uint16_t ielen = 0;

	if (NULL == ptr || 0 == left)
		return;

	while (left >= 2) {
		elem_id = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left) {
			hddLog(LOGE,
			       FL("****Invalid IEs eid = %d elem_len=%d left=%d*****"),
			       elem_id, elem_len, left);
			return;
		}

		if (temp_ie_id == elem_id) {
			ielen = ptr[1] + 2;
			if ((*total_ielen + ielen) <= MAX_GENIE_LEN) {
				cdf_mem_copy(&genie[*total_ielen], ptr, ielen);
				*total_ielen += ielen;
			} else {
				hddLog(LOGE,
				       FL("IE Length is too big IEs eid=%d elem_len=%d total_ie_lent=%d"),
				       elem_id, elem_len, *total_ielen);
			}
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
	return;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
/**
 * wlan_hdd_cfg80211_alloc_new_beacon() - alloc beacon in ap mode
 * @pAdapter: Pointer to hostapd adapter
 * @ppBeacon: Pointer to pointer to beacon data
 * @params: Pointer to beacon parameters
 *
 * Return: 0 for success non-zero for failure
 */
int wlan_hdd_cfg80211_alloc_new_beacon(hdd_adapter_t *pAdapter,
				       beacon_data_t **ppBeacon,
				       struct beacon_parameters *params)
#else
/**
 * wlan_hdd_cfg80211_alloc_new_beacon() - alloc beacon in ap mode
 * @pAdapter: Pointer to hostapd adapter
 * @ppBeacon: Pointer to pointer to beacon data
 * @params: Pointer to beacon parameters
 * @dtim_period: DTIM period
 *
 * Return: 0 for success non-zero for failure
 */
int wlan_hdd_cfg80211_alloc_new_beacon(hdd_adapter_t *pAdapter,
				       beacon_data_t **ppBeacon,
				       struct cfg80211_beacon_data *params,
				       int dtim_period)
#endif
{
	int size;
	beacon_data_t *beacon = NULL;
	beacon_data_t *old = NULL;
	int head_len, tail_len, proberesp_ies_len, assocresp_ies_len;
	const u8 *head, *tail, *proberesp_ies, *assocresp_ies;

	ENTER();
	if (params->head && !params->head_len) {
		hddLog(LOGE, FL("head_len is NULL"));
		return -EINVAL;
	}

	old = pAdapter->sessionCtx.ap.beacon;

	if (!params->head && !old) {
		hddLog(LOGE, FL("session(%d) old and new heads points to NULL"),
		       pAdapter->sessionId);
		return -EINVAL;
	}

	if (params->head) {
		head_len = params->head_len;
		head = params->head;
	} else {
		head_len = old->head_len;
		head = old->head;
	}

	if (params->tail || !old) {
		tail_len = params->tail_len;
		tail = params->tail;
	} else {
		tail_len = old->tail_len;
		tail = old->tail;
	}

	if (params->proberesp_ies || !old) {
		proberesp_ies_len = params->proberesp_ies_len;
		proberesp_ies = params->proberesp_ies;
	} else {
		proberesp_ies_len = old->proberesp_ies_len;
		proberesp_ies = old->proberesp_ies;
	}

	if (params->assocresp_ies || !old) {
		assocresp_ies_len = params->assocresp_ies_len;
		assocresp_ies = params->assocresp_ies;
	} else {
		assocresp_ies_len = old->assocresp_ies_len;
		assocresp_ies = old->assocresp_ies;
	}

	size = sizeof(beacon_data_t) + head_len + tail_len +
		proberesp_ies_len + assocresp_ies_len;

	beacon = kzalloc(size, GFP_KERNEL);

	if (beacon == NULL) {
		hddLog(LOGE,
		       FL("Mem allocation for beacon failed"));
		return -ENOMEM;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
	if (params->dtim_period)
		beacon->dtim_period = params->dtim_period;
	else if (old)
		beacon->dtim_period = old->dtim_period;
#else
	if (dtim_period)
		beacon->dtim_period = dtim_period;
	else if (old)
		beacon->dtim_period = old->dtim_period;
#endif
	/* -----------------------------------------------
	 * | head | tail | proberesp_ies | assocresp_ies |
	 * -----------------------------------------------
	 */
	beacon->head = ((u8 *) beacon) + sizeof(beacon_data_t);
	beacon->tail = beacon->head + head_len;
	beacon->proberesp_ies = beacon->tail + tail_len;
	beacon->assocresp_ies = beacon->proberesp_ies + proberesp_ies_len;

	beacon->head_len = head_len;
	beacon->tail_len = tail_len;
	beacon->proberesp_ies_len = proberesp_ies_len;
	beacon->assocresp_ies_len = assocresp_ies_len;

	if (head && head_len)
		memcpy(beacon->head, head, head_len);
	if (tail && tail_len)
		memcpy(beacon->tail, tail, tail_len);
	if (proberesp_ies && proberesp_ies_len)
		memcpy(beacon->proberesp_ies, proberesp_ies, proberesp_ies_len);
	if (assocresp_ies && assocresp_ies_len)
		memcpy(beacon->assocresp_ies, assocresp_ies, assocresp_ies_len);

	*ppBeacon = beacon;

	kfree(old);

	return 0;

}

/**
 * wlan_hdd_cfg80211_update_apies() - update ap mode ies
 * @adapter: Pointer to hostapd adapter
 *
 * Return: 0 for success non-zero for failure
 */
int wlan_hdd_cfg80211_update_apies(hdd_adapter_t *adapter)
{
	uint8_t *genie;
	uint8_t total_ielen = 0;
	int ret = 0;
	tsap_Config_t *pConfig;
	tSirUpdateIE updateIE;
	beacon_data_t *beacon = NULL;

	pConfig = &adapter->sessionCtx.ap.sapConfig;
	beacon = adapter->sessionCtx.ap.beacon;

	genie = cdf_mem_malloc(MAX_GENIE_LEN);

	if (genie == NULL)
		return -ENOMEM;

	if (0 != wlan_hdd_add_ie(adapter, genie,
		&total_ielen, WPS_OUI_TYPE, WPS_OUI_TYPE_SIZE)) {
		hddLog(LOGE, FL("Adding WPS IE failed"));
		ret = -EINVAL;
		goto done;
	}
#ifdef WLAN_FEATURE_WFD
	if (0 != wlan_hdd_add_ie(adapter, genie,
		&total_ielen, WFD_OUI_TYPE, WFD_OUI_TYPE_SIZE)) {
		hddLog(LOGE, FL("Adding WFD IE failed"));
		ret = -EINVAL;
		goto done;
	}
#endif

#ifdef FEATURE_WLAN_WAPI
	if (WLAN_HDD_SOFTAP == adapter->device_mode) {
		wlan_hdd_add_extra_ie(adapter, genie, &total_ielen,
				      WLAN_EID_WAPI);
	}
#endif

	if (adapter->device_mode == WLAN_HDD_SOFTAP ||
		adapter->device_mode == WLAN_HDD_P2P_GO)
		wlan_hdd_add_hostapd_conf_vsie(adapter, genie,
					       &total_ielen);

	if (wlan_hdd_add_ie(adapter, genie,
		&total_ielen, P2P_OUI_TYPE, P2P_OUI_TYPE_SIZE) != 0) {
		hddLog(LOGE, FL("Adding P2P IE failed"));
		ret = -EINVAL;
		goto done;
	}

#ifdef QCA_HT_2040_COEX
	if (WLAN_HDD_SOFTAP == adapter->device_mode) {
		tSmeConfigParams smeConfig;
		cdf_mem_zero(&smeConfig, sizeof(smeConfig));
		sme_get_config_param(WLAN_HDD_GET_HAL_CTX(adapter),
				     &smeConfig);
		if (smeConfig.csrConfig.obssEnabled)
			wlan_hdd_add_extra_ie(adapter, genie,
					      &total_ielen,
					      WLAN_EID_OVERLAP_BSS_SCAN_PARAM);
	}
#endif
	cdf_copy_macaddr(&updateIE.bssid, &adapter->macAddressCurrent);
	updateIE.smeSessionId = adapter->sessionId;

	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		updateIE.ieBufferlength = total_ielen;
		updateIE.pAdditionIEBuffer = genie;
		updateIE.append = false;
		updateIE.notify = true;
		if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
				      &updateIE,
				      eUPDATE_IE_PROBE_BCN) ==
		    CDF_STATUS_E_FAILURE) {
			hddLog(LOGE,
			       FL("Could not pass on Add Ie probe beacon data"));
			ret = -EINVAL;
			goto done;
		}
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_PROBE_BCN);
	} else {
		wlansap_update_sap_config_add_ie(pConfig,
						 genie,
						 total_ielen,
						 eUPDATE_IE_PROBE_BCN);
	}

	/* Added for Probe Response IE */
	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		updateIE.ieBufferlength = beacon->proberesp_ies_len;
		updateIE.pAdditionIEBuffer = (uint8_t *) beacon->proberesp_ies;
		updateIE.append = false;
		updateIE.notify = false;
		if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
				      &updateIE,
				      eUPDATE_IE_PROBE_RESP) ==
		    CDF_STATUS_E_FAILURE) {
			hddLog(LOGE,
			       FL("Could not pass on PROBE_RESP add Ie data"));
			ret = -EINVAL;
			goto done;
		}
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_PROBE_RESP);
	} else {
		wlansap_update_sap_config_add_ie(pConfig,
						 beacon->proberesp_ies,
						 beacon->proberesp_ies_len,
						 eUPDATE_IE_PROBE_RESP);
	}

	/* Assoc resp Add ie Data */
	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		updateIE.ieBufferlength = beacon->assocresp_ies_len;
		updateIE.pAdditionIEBuffer = (uint8_t *) beacon->assocresp_ies;
		updateIE.append = false;
		updateIE.notify = false;
		if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
				      &updateIE,
				      eUPDATE_IE_ASSOC_RESP) ==
		    CDF_STATUS_E_FAILURE) {
			hddLog(LOGE,
				FL("Could not pass on Add Ie Assoc Response data"));
			ret = -EINVAL;
			goto done;
		}
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ASSOC_RESP);
	} else {
		wlansap_update_sap_config_add_ie(pConfig,
						 beacon->assocresp_ies,
						 beacon->assocresp_ies_len,
						 eUPDATE_IE_ASSOC_RESP);
	}

done:
	cdf_mem_free(genie);
	return ret;
}

/**
 * wlan_hdd_set_sap_hwmode() - set sap hw mode
 * @pHostapdAdapter: Pointer to hostapd adapter
 *
 * Return: none
 */
static void wlan_hdd_set_sap_hwmode(hdd_adapter_t *pHostapdAdapter)
{
	tsap_Config_t *pConfig = &pHostapdAdapter->sessionCtx.ap.sapConfig;
	beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;
	struct ieee80211_mgmt *pMgmt_frame =
		(struct ieee80211_mgmt *)pBeacon->head;
	u8 checkRatesfor11g = true;
	u8 require_ht = false, require_vht = false;
	u8 *pIe = NULL;

	pConfig->SapHw_mode = eCSR_DOT11_MODE_11b;

	pIe = wlan_hdd_cfg80211_get_ie_ptr(&pMgmt_frame->u.beacon.variable[0],
					   pBeacon->head_len,
					   WLAN_EID_SUPP_RATES);
	if (pIe != NULL) {
		pIe += 1;
		wlan_hdd_check_11gmode(pIe, &require_ht, &require_vht,
			&checkRatesfor11g, &pConfig->SapHw_mode);
	}

	pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
					   WLAN_EID_EXT_SUPP_RATES);
	if (pIe != NULL) {
		pIe += 1;
		wlan_hdd_check_11gmode(pIe, &require_ht, &require_vht,
			&checkRatesfor11g, &pConfig->SapHw_mode);
	}

	if (pConfig->channel > 14)
		pConfig->SapHw_mode = eCSR_DOT11_MODE_11a;

	pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
					   WLAN_EID_HT_CAPABILITY);

	if (pIe) {
		pConfig->SapHw_mode = eCSR_DOT11_MODE_11n;
		if (require_ht)
			pConfig->SapHw_mode = eCSR_DOT11_MODE_11n_ONLY;
	}

	pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
					   WLAN_EID_VHT_CAPABILITY);

	if (pIe) {
		pConfig->SapHw_mode = eCSR_DOT11_MODE_11ac;
		if (require_vht)
			pConfig->SapHw_mode = eCSR_DOT11_MODE_11ac_ONLY;
	}
}

/**
 * wlan_hdd_config_acs() - config ACS needed parameters
 * @hdd_ctx: HDD context
 * @adapter: Adapter pointer
 *
 * This function get ACS related INI paramters and populated
 * sap config and smeConfig for ACS needed configurations.
 *
 * Return: The CDF_STATUS code associated with performing the operation.
 */
CDF_STATUS wlan_hdd_config_acs(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter)
{
	tsap_Config_t *sap_config;
	struct hdd_config *ini_config;
	tHalHandle hal;

	hal = WLAN_HDD_GET_HAL_CTX(adapter);
	sap_config = &adapter->sessionCtx.ap.sapConfig;
	ini_config = hdd_ctx->config;

	sap_config->enOverLapCh = !!hdd_ctx->config->gEnableOverLapCh;

#if defined(WLAN_FEATURE_MBSSID) && defined(FEATURE_WLAN_AP_AP_ACS_OPTIMIZE)
	hddLog(LOG1, FL("HDD_ACS_SKIP_STATUS = %d"),
						hdd_ctx->skip_acs_scan_status);
	if (hdd_ctx->skip_acs_scan_status == eSAP_SKIP_ACS_SCAN) {
		hdd_adapter_t *con_sap_adapter;
		tsap_Config_t *con_sap_config = NULL;

		con_sap_adapter = hdd_get_con_sap_adapter(adapter, false);

		if (con_sap_adapter)
			con_sap_config =
				&con_sap_adapter->sessionCtx.ap.sapConfig;

		sap_config->acs_cfg.skip_scan_status = eSAP_DO_NEW_ACS_SCAN;

		if (con_sap_config &&
			con_sap_config->acs_cfg.acs_mode == true &&
			hdd_ctx->skip_acs_scan_status == eSAP_SKIP_ACS_SCAN &&
			con_sap_config->acs_cfg.hw_mode ==
						sap_config->acs_cfg.hw_mode) {
			uint8_t con_sap_st_ch, con_sap_end_ch;
			uint8_t cur_sap_st_ch, cur_sap_end_ch;
			uint8_t bandStartChannel, bandEndChannel;

			con_sap_st_ch =
					con_sap_config->acs_cfg.start_ch;
			con_sap_end_ch =
					con_sap_config->acs_cfg.end_ch;
			cur_sap_st_ch = sap_config->acs_cfg.start_ch;
			cur_sap_end_ch = sap_config->acs_cfg.end_ch;

			wlansap_extend_to_acs_range(
					&cur_sap_st_ch, &cur_sap_end_ch,
					&bandStartChannel, &bandEndChannel);

			wlansap_extend_to_acs_range(
					&con_sap_st_ch, &con_sap_end_ch,
					&bandStartChannel, &bandEndChannel);

			if (con_sap_st_ch <= cur_sap_st_ch &&
					con_sap_end_ch >= cur_sap_end_ch) {
				sap_config->acs_cfg.skip_scan_status =
							eSAP_SKIP_ACS_SCAN;

			} else if (con_sap_st_ch >= cur_sap_st_ch &&
					con_sap_end_ch >= cur_sap_end_ch) {
				sap_config->acs_cfg.skip_scan_status =
							eSAP_DO_PAR_ACS_SCAN;

				sap_config->acs_cfg.skip_scan_range1_stch =
							cur_sap_st_ch;
				sap_config->acs_cfg.skip_scan_range1_endch =
							con_sap_st_ch - 1;
				sap_config->acs_cfg.skip_scan_range2_stch =
							0;
				sap_config->acs_cfg.skip_scan_range2_endch =
							0;

			} else if (con_sap_st_ch <= cur_sap_st_ch &&
				con_sap_end_ch <= cur_sap_end_ch) {
				sap_config->acs_cfg.skip_scan_status =
							eSAP_DO_PAR_ACS_SCAN;

				sap_config->acs_cfg.skip_scan_range1_stch =
							con_sap_end_ch + 1;
				sap_config->acs_cfg.skip_scan_range1_endch =
							cur_sap_end_ch;
				sap_config->acs_cfg.skip_scan_range2_stch =
							0;
				sap_config->acs_cfg.skip_scan_range2_endch =
							0;

			} else if (con_sap_st_ch >= cur_sap_st_ch &&
				con_sap_end_ch <= cur_sap_end_ch) {
				sap_config->acs_cfg.skip_scan_status =
							eSAP_DO_PAR_ACS_SCAN;

				sap_config->acs_cfg.skip_scan_range1_stch =
							cur_sap_st_ch;
				sap_config->acs_cfg.skip_scan_range1_endch =
							con_sap_st_ch - 1;
				sap_config->acs_cfg.skip_scan_range2_stch =
							con_sap_end_ch;
				sap_config->acs_cfg.skip_scan_range2_endch =
							cur_sap_end_ch + 1;

			} else
				sap_config->acs_cfg.skip_scan_status =
							eSAP_DO_NEW_ACS_SCAN;


			hddLog(LOG1, FL(
				"SecAP ACS Skip=%d, ACS CH RANGE=%d-%d, %d-%d"),
				sap_config->acs_cfg.skip_scan_status,
				sap_config->acs_cfg.skip_scan_range1_stch,
				sap_config->acs_cfg.skip_scan_range1_endch,
				sap_config->acs_cfg.skip_scan_range2_stch,
				sap_config->acs_cfg.skip_scan_range2_endch);
		}
	}
#endif

	return CDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_setup_driver_overrides : Overrides SAP / P2P GO Params
 * @adapter: pointer to adapter struct
 *
 * This function overrides SAP / P2P Go configuration based on driver INI
 * parameters for 11AC override and ACS. This overrides are done to support
 * android legacy configuration method.
 *
 * NOTE: Non android platform supports concurrency and these overrides shall
 * not be used. Also future driver based overrides shall be consolidated in this
 * function only. Avoid random overrides in other location based on ini.
 *
 * Return: 0 for Success or Negative error codes.
 */
int wlan_hdd_setup_driver_overrides(hdd_adapter_t *ap_adapter)
{
	tsap_Config_t *sap_cfg = &ap_adapter->sessionCtx.ap.sapConfig;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);
	tHalHandle h_hal = WLAN_HDD_GET_HAL_CTX(ap_adapter);

	if (ap_adapter->device_mode == WLAN_HDD_SOFTAP &&
				hdd_ctx->config->force_sap_acs)
		goto setup_acs_overrides;

	/* Fixed channel 11AC override:
	 * 11AC override in qcacld is introduced for following reasons:
	 * 1. P2P GO also follows start_bss and since p2p GO could not be
	 *    configured to setup VHT channel width in wpa_supplicant
	 * 2. Android UI does not provide advanced configuration options for SAP
	 *
	 * Default override enabled (for android). MDM shall disable this in ini
	 */
	if (hdd_ctx->config->sap_p2p_11ac_override &&
			(sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11n ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac_ONLY)) {
		hddLog(LOG1, FL("** Driver force 11AC override for SAP/Go **"));

		/* 11n only shall not be overridden since it may be on purpose*/
		if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11n)
			sap_cfg->SapHw_mode = eCSR_DOT11_MODE_11ac;

		if (sap_cfg->channel >= 36)
			sap_cfg->ch_width_orig =
					hdd_ctx->config->vhtChannelWidth;
		else
			sap_cfg->ch_width_orig =
				hdd_ctx->config->nChannelBondingMode24GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
	}
	sap_cfg->ch_params.ch_width = sap_cfg->ch_width_orig;
	sme_set_ch_params(h_hal, sap_cfg->SapHw_mode, sap_cfg->channel,
				sap_cfg->sec_ch, &sap_cfg->ch_params);

	return 0;

setup_acs_overrides:
	hddLog(LOGE, FL("** Driver force ACS override **"));

	sap_cfg->channel = AUTO_CHANNEL_SELECT;
	sap_cfg->acs_cfg.acs_mode = true;
	sap_cfg->acs_cfg.start_ch = hdd_ctx->config->force_sap_acs_st_ch;
	sap_cfg->acs_cfg.end_ch = hdd_ctx->config->force_sap_acs_end_ch;

	if (sap_cfg->acs_cfg.start_ch > sap_cfg->acs_cfg.end_ch) {
		hddLog(LOGE, FL("Driver force ACS start ch (%d) > end ch (%d)"),
			sap_cfg->acs_cfg.start_ch,  sap_cfg->acs_cfg.end_ch);
		return -EINVAL;
	}

	/* Derive ACS HW mode */
	sap_cfg->SapHw_mode = hdd_cfg_xlate_to_csr_phy_mode(
						hdd_ctx->config->dot11Mode);
	if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_AUTO)
		sap_cfg->SapHw_mode = eCSR_DOT11_MODE_11ac;

	if ((sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11b ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11g ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11g_ONLY) &&
			sap_cfg->acs_cfg.start_ch > 14) {
		hddLog(LOGE, FL("Invalid ACS HW Mode %d + CH range <%d - %d>"),
			sap_cfg->SapHw_mode, sap_cfg->acs_cfg.start_ch,
			sap_cfg->acs_cfg.end_ch);
		return -EINVAL;
	}
	sap_cfg->acs_cfg.hw_mode = sap_cfg->SapHw_mode;

	/* Derive ACS BW */
	sap_cfg->ch_width_orig = eHT_CHANNEL_WIDTH_20MHZ;
	if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac_ONLY) {

		sap_cfg->ch_width_orig = hdd_ctx->config->vhtChannelWidth;
		/* VHT in 2.4G depends on gChannelBondingMode24GHz INI param */
		if (sap_cfg->acs_cfg.end_ch <= 14)
			sap_cfg->ch_width_orig =
				hdd_ctx->config->nChannelBondingMode24GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
	}

	if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11n ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11n_ONLY) {
		if (sap_cfg->acs_cfg.end_ch <= 14)
			sap_cfg->ch_width_orig =
				hdd_ctx->config->nChannelBondingMode24GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
		else
			sap_cfg->ch_width_orig =
				hdd_ctx->config->nChannelBondingMode5GHz ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ;
	}
	sap_cfg->acs_cfg.ch_width = sap_cfg->ch_width_orig;

	hddLog(LOG1, FL("Force ACS Config: HW_MODE: %d ACS_BW: %d"),
		sap_cfg->acs_cfg.hw_mode, sap_cfg->acs_cfg.ch_width);
	hddLog(LOG1, FL("Force ACS Config: ST_CH: %d END_CH: %d"),
		sap_cfg->acs_cfg.start_ch, sap_cfg->acs_cfg.end_ch);

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
/**
 * wlan_hdd_cfg80211_start_bss() - start bss
 * @pHostapdAdapter: Pointer to hostapd adapter
 * @params: Pointer to start bss beacon parameters
 *
 * Return: 0 for success non-zero for failure
 */
static int wlan_hdd_cfg80211_start_bss(hdd_adapter_t *pHostapdAdapter,
				       struct beacon_parameters *params)
#else
/**
 * wlan_hdd_cfg80211_start_bss() - start bss
 * @pHostapdAdapter: Pointer to hostapd adapter
 * @params: Pointer to start bss beacon parameters
 * @ssid: Pointer ssid
 * @ssid_len: Length of ssid
 * @hidden_ssid: Hidden SSID parameter
 *
 * Return: 0 for success non-zero for failure
 */
static int wlan_hdd_cfg80211_start_bss(hdd_adapter_t *pHostapdAdapter,
				       struct cfg80211_beacon_data *params,
				       const u8 *ssid, size_t ssid_len,
				       enum nl80211_hidden_ssid hidden_ssid)
#endif
{
	tsap_Config_t *pConfig;
	beacon_data_t *pBeacon = NULL;
	struct ieee80211_mgmt *pMgmt_frame;
	uint8_t *pIe = NULL;
	uint16_t capab_info;
	eCsrAuthType RSNAuthType;
	eCsrEncryptionType RSNEncryptType;
	eCsrEncryptionType mcRSNEncryptType;
	int status = CDF_STATUS_SUCCESS, ret;
	tpWLAN_SAPEventCB pSapEventCallback;
	hdd_hostapd_state_t *pHostapdState;
#ifndef WLAN_FEATURE_MBSSID
	v_CONTEXT_t p_cds_context =
		(WLAN_HDD_GET_CTX(pHostapdAdapter))->pcds_context;
#endif
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
	struct qc_mac_acl_entry *acl_entry = NULL;
	int32_t i;
	struct hdd_config *iniConfig;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pHostapdAdapter);
	tSmeConfigParams sme_config;
	bool MFPCapable = false;
	bool MFPRequired = false;
	uint16_t prev_rsn_length = 0;
	ENTER();

	if (cds_is_connection_in_progress(pHddCtx)) {
		hdd_err("Can't start BSS: connection is in progress");
		return -EINVAL;
	}

	iniConfig = pHddCtx->config;
	pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);

	clear_bit(ACS_PENDING, &pHostapdAdapter->event_flags);
	clear_bit(ACS_IN_PROGRESS, &pHddCtx->g_event_flags);

	pConfig = &pHostapdAdapter->sessionCtx.ap.sapConfig;

	pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;

	pMgmt_frame = (struct ieee80211_mgmt *)pBeacon->head;

	pConfig->beacon_int = pMgmt_frame->u.beacon.beacon_int;

	pConfig->disableDFSChSwitch = iniConfig->disableDFSChSwitch;

	/* channel is already set in the set_channel Call back */
	/* pConfig->channel = pCommitConfig->channel; */

	/* Protection parameter to enable or disable */
	pConfig->protEnabled = iniConfig->apProtEnabled;

	pConfig->dtim_period = pBeacon->dtim_period;

	hddLog(LOG2, FL("****pConfig->dtim_period=%d***"),
		pConfig->dtim_period);

	if (pHostapdAdapter->device_mode == WLAN_HDD_SOFTAP) {
		pIe =
			wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail,
						     pBeacon->tail_len,
						     WLAN_EID_COUNTRY);
		if (pIe) {
			pConfig->ieee80211d = 1;
			cdf_mem_copy(pConfig->countryCode, &pIe[2], 3);
			sme_set_reg_info(hHal, pConfig->countryCode);
			sme_apply_channel_power_info_to_fw(hHal);
		} else {
			pConfig->countryCode[0] = pHddCtx->reg.alpha2[0];
			pConfig->countryCode[1] = pHddCtx->reg.alpha2[1];
			pConfig->ieee80211d = 0;
		}

		ret = wlan_hdd_sap_cfg_dfs_override(pHostapdAdapter);
		if (ret < 0) {
			return ret;
		} else {
			if (ret == 0) {
				if (CDS_IS_DFS_CH(pConfig->channel))
					pHddCtx->dev_dfs_cac_status =
							DFS_CAC_NEVER_DONE;
			}
		}

		if (CDF_STATUS_SUCCESS !=
			    wlan_hdd_validate_operation_channel(pHostapdAdapter,
							pConfig->channel)) {
			hddLog(LOGE, FL("Invalid Channel [%d]"),
							pConfig->channel);
				return -EINVAL;
		}

		/* reject SAP if DFS channel scan is not allowed */
		if (!(pHddCtx->config->enableDFSChnlScan) &&
		    (CHANNEL_STATE_DFS == cds_get_channel_state(
					     pConfig->channel))) {
			hddLog(LOGE,
				FL("not allowed to start SAP on DFS channel"));
				return -EOPNOTSUPP;
		}
		wlansap_set_dfs_ignore_cac(hHal, iniConfig->ignoreCAC);
		wlansap_set_dfs_restrict_japan_w53(hHal,
			iniConfig->gDisableDfsJapanW53);
		wlansap_set_dfs_preferred_channel_location(hHal,
			iniConfig->gSapPreferredChanLocation);
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		wlan_sap_set_channel_avoidance(hHal,
					iniConfig->sap_channel_avoidance);
#endif
	} else if (pHostapdAdapter->device_mode == WLAN_HDD_P2P_GO) {
		pConfig->countryCode[0] = pHddCtx->reg.alpha2[0];
		pConfig->countryCode[1] = pHddCtx->reg.alpha2[1];
		pConfig->ieee80211d = 0;
	} else {
		pConfig->ieee80211d = 0;
	}

	capab_info = pMgmt_frame->u.beacon.capab_info;

	pConfig->privacy = (pMgmt_frame->u.beacon.capab_info &
			    WLAN_CAPABILITY_PRIVACY) ? true : false;

	(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy = pConfig->privacy;

	/*Set wps station to configured */
	pIe = wlan_hdd_get_wps_ie_ptr(pBeacon->tail, pBeacon->tail_len);

	if (pIe) {
		if (pIe[1] < (2 + WPS_OUI_TYPE_SIZE)) {
			hddLog(LOGE,
			       FL("**Wps Ie Length is too small***"));
			return -EINVAL;
		} else if (memcmp(&pIe[2], WPS_OUI_TYPE, WPS_OUI_TYPE_SIZE) ==
			   0) {
			hddLog(LOG1, FL("** WPS IE(len %d) ***"), (pIe[1] + 2));
			/* Check 15 bit of WPS IE as it contain information for
			 * wps state
			 */
			if (SAP_WPS_ENABLED_UNCONFIGURED == pIe[15]) {
				pConfig->wps_state =
					SAP_WPS_ENABLED_UNCONFIGURED;
			} else if (SAP_WPS_ENABLED_CONFIGURED == pIe[15]) {
				pConfig->wps_state = SAP_WPS_ENABLED_CONFIGURED;
			}
		}
	} else {
		hdd_info("WPS disabled");
		pConfig->wps_state = SAP_WPS_DISABLED;
	}
	/* Forward WPS PBC probe request frame up */
	pConfig->fwdWPSPBCProbeReq = 1;

	pConfig->RSNEncryptType = eCSR_ENCRYPT_TYPE_NONE;
	pConfig->mcRSNEncryptType = eCSR_ENCRYPT_TYPE_NONE;
	(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->ucEncryptType =
		eCSR_ENCRYPT_TYPE_NONE;

	pConfig->RSNWPAReqIELength = 0;
	memset(&pConfig->RSNWPAReqIE[0], 0, sizeof(pConfig->RSNWPAReqIE));
	pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
					   WLAN_EID_RSN);
	if (pIe && pIe[1]) {
		pConfig->RSNWPAReqIELength = pIe[1] + 2;
		if (pConfig->RSNWPAReqIELength < sizeof(pConfig->RSNWPAReqIE))
			memcpy(&pConfig->RSNWPAReqIE[0], pIe,
			       pConfig->RSNWPAReqIELength);
		else
			hddLog(LOGE,
			       FL("RSNWPA IE MAX Length exceeded; length =%d"),
			       pConfig->RSNWPAReqIELength);
		/* The actual processing may eventually be more extensive than
		 * this. Right now, just consume any PMKIDs that are  sent in
		 * by the app.
		 * */
		status =
			hdd_softap_unpack_ie(cds_get_context
						     (CDF_MODULE_ID_SME),
					     &RSNEncryptType, &mcRSNEncryptType,
					     &RSNAuthType, &MFPCapable,
					     &MFPRequired,
					     pConfig->RSNWPAReqIE[1] + 2,
					     pConfig->RSNWPAReqIE);

		if (CDF_STATUS_SUCCESS == status) {
			/* Now copy over all the security attributes you have
			 * parsed out. Use the cipher type in the RSN IE
			 */
			pConfig->RSNEncryptType = RSNEncryptType;
			pConfig->mcRSNEncryptType = mcRSNEncryptType;
			(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->
			ucEncryptType = RSNEncryptType;
			hddLog(LOG1,
			       FL("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d"),
			       RSNAuthType, RSNEncryptType, mcRSNEncryptType);
		}
	}

	pIe = wlan_hdd_get_vendor_oui_ie_ptr(WPA_OUI_TYPE, WPA_OUI_TYPE_SIZE,
					     pBeacon->tail, pBeacon->tail_len);

	if (pIe && pIe[1] && (pIe[0] == DOT11F_EID_WPA)) {
		if (pConfig->RSNWPAReqIE[0]) {
			/*Mixed mode WPA/WPA2 */
			prev_rsn_length = pConfig->RSNWPAReqIELength;
			pConfig->RSNWPAReqIELength += pIe[1] + 2;
			if (pConfig->RSNWPAReqIELength <
			    sizeof(pConfig->RSNWPAReqIE))
				memcpy(&pConfig->RSNWPAReqIE[0] +
				       prev_rsn_length, pIe, pIe[1] + 2);
			else
				hddLog(LOGE,
				       "RSNWPA IE MAX Length exceeded; length =%d",
				       pConfig->RSNWPAReqIELength);
		} else {
			pConfig->RSNWPAReqIELength = pIe[1] + 2;
			if (pConfig->RSNWPAReqIELength <
			    sizeof(pConfig->RSNWPAReqIE))
				memcpy(&pConfig->RSNWPAReqIE[0], pIe,
				       pConfig->RSNWPAReqIELength);
			else
				hddLog(LOGE,
				       "RSNWPA IE MAX Length exceeded; length =%d",
				       pConfig->RSNWPAReqIELength);
			status = hdd_softap_unpack_ie
					(cds_get_context(CDF_MODULE_ID_SME),
					 &RSNEncryptType,
					 &mcRSNEncryptType, &RSNAuthType,
					 &MFPCapable, &MFPRequired,
					 pConfig->RSNWPAReqIE[1] + 2,
					 pConfig->RSNWPAReqIE);

			if (CDF_STATUS_SUCCESS == status) {
				/* Now copy over all the security attributes
				 * you have parsed out. Use the cipher type
				 * in the RSN IE
				 */
				pConfig->RSNEncryptType = RSNEncryptType;
				pConfig->mcRSNEncryptType = mcRSNEncryptType;
				(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->
				ucEncryptType = RSNEncryptType;
				hddLog(LOG1,
				       FL("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d"),
				       RSNAuthType, RSNEncryptType,
				       mcRSNEncryptType);
			}
		}
	}

	if (pConfig->RSNWPAReqIELength > sizeof(pConfig->RSNWPAReqIE)) {
		hddLog(LOGE,
		       FL("**RSNWPAReqIELength is too large***"));
		return -EINVAL;
	}

	pConfig->SSIDinfo.ssidHidden = false;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
	if (params->ssid != NULL) {
		cdf_mem_copy(pConfig->SSIDinfo.ssid.ssId, params->ssid,
			     params->ssid_len);
		pConfig->SSIDinfo.ssid.length = params->ssid_len;

		switch (params->hidden_ssid) {
		case NL80211_HIDDEN_SSID_NOT_IN_USE:
			hddLog(LOG1, "HIDDEN_SSID_NOT_IN_USE");
			pConfig->SSIDinfo.ssidHidden = eHIDDEN_SSID_NOT_IN_USE;
			break;
		case NL80211_HIDDEN_SSID_ZERO_LEN:
			hddLog(LOG1, "HIDDEN_SSID_ZERO_LEN");
			pConfig->SSIDinfo.ssidHidden = eHIDDEN_SSID_ZERO_LEN;
			break;
		case NL80211_HIDDEN_SSID_ZERO_CONTENTS:
			hddLog(LOG1, "HIDDEN_SSID_ZERO_CONTENTS");
			pConfig->SSIDinfo.ssidHidden =
				eHIDDEN_SSID_ZERO_CONTENTS;
			break;
		default:
			hddLog(LOGE, "Wrong hidden_ssid param %d",
			       params->hidden_ssid);
			break;
		}
	}
#else
	if (ssid != NULL) {
		cdf_mem_copy(pConfig->SSIDinfo.ssid.ssId, ssid, ssid_len);
		pConfig->SSIDinfo.ssid.length = ssid_len;

		switch (hidden_ssid) {
		case NL80211_HIDDEN_SSID_NOT_IN_USE:
			hddLog(LOG1, "HIDDEN_SSID_NOT_IN_USE");
			pConfig->SSIDinfo.ssidHidden = eHIDDEN_SSID_NOT_IN_USE;
			break;
		case NL80211_HIDDEN_SSID_ZERO_LEN:
			hddLog(LOG1, "HIDDEN_SSID_ZERO_LEN");
			pConfig->SSIDinfo.ssidHidden = eHIDDEN_SSID_ZERO_LEN;
			break;
		case NL80211_HIDDEN_SSID_ZERO_CONTENTS:
			hddLog(LOG1, "HIDDEN_SSID_ZERO_CONTENTS");
			pConfig->SSIDinfo.ssidHidden =
				eHIDDEN_SSID_ZERO_CONTENTS;
			break;
		default:
			hddLog(LOGE, "Wrong hidden_ssid param %d", hidden_ssid);
			break;
		}
	}
#endif

	cdf_mem_copy(pConfig->self_macaddr.bytes,
		     pHostapdAdapter->macAddressCurrent.bytes,
		     CDF_MAC_ADDR_SIZE);

	/* default value */
	pConfig->SapMacaddr_acl = eSAP_ACCEPT_UNLESS_DENIED;
	pConfig->num_accept_mac = 0;
	pConfig->num_deny_mac = 0;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	/*
	 * We don't want P2PGO to follow STA's channel
	 * so lets limit the logic for SAP only.
	 * Later if we decide to make p2pgo follow STA's
	 * channel then remove this check.
	 */
	if ((0 == pHddCtx->config->conc_custom_rule1) ||
		(pHddCtx->config->conc_custom_rule1 &&
		WLAN_HDD_SOFTAP == pHostapdAdapter->device_mode))
		pConfig->cc_switch_mode = iniConfig->WlanMccToSccSwitchMode;
#endif

	pIe =
		wlan_hdd_get_vendor_oui_ie_ptr(BLACKLIST_OUI_TYPE,
					       WPA_OUI_TYPE_SIZE, pBeacon->tail,
					       pBeacon->tail_len);

	/* pIe for black list is following form:
	 * type    : 1 byte
	 * length  : 1 byte
	 * OUI     : 4 bytes
	 * acl type : 1 byte
	 * no of mac addr in black list: 1 byte
	 * list of mac_acl_entries: variable, 6 bytes per mac
	 * address + sizeof(int) for vlan id
	 */
	if ((pIe != NULL) && (pIe[1] != 0)) {
		pConfig->SapMacaddr_acl = pIe[6];
		pConfig->num_deny_mac = pIe[7];
		hddLog(LOG1,
			FL("acl type = %d no deny mac = %d"), pIe[6], pIe[7]);
		if (pConfig->num_deny_mac > MAX_ACL_MAC_ADDRESS)
			pConfig->num_deny_mac = MAX_ACL_MAC_ADDRESS;
		acl_entry = (struct qc_mac_acl_entry *)(pIe + 8);
		for (i = 0; i < pConfig->num_deny_mac; i++) {
			cdf_mem_copy(&pConfig->deny_mac[i], acl_entry->addr,
				     sizeof(qcmacaddr));
			acl_entry++;
		}
	}
	pIe = wlan_hdd_get_vendor_oui_ie_ptr(WHITELIST_OUI_TYPE,
			WPA_OUI_TYPE_SIZE, pBeacon->tail,
			pBeacon->tail_len);

	/* pIe for white list is following form:
	 * type    : 1 byte
	 * length  : 1 byte
	 * OUI     : 4 bytes
	 * acl type : 1 byte
	 * no of mac addr in white list: 1 byte
	 * list of mac_acl_entries: variable, 6 bytes per mac
	 * address + sizeof(int) for vlan id
	 */
	if ((pIe != NULL) && (pIe[1] != 0)) {
		pConfig->SapMacaddr_acl = pIe[6];
		pConfig->num_accept_mac = pIe[7];
		hddLog(LOG1, FL("acl type = %d no accept mac = %d"),
		       pIe[6], pIe[7]);
		if (pConfig->num_accept_mac > MAX_ACL_MAC_ADDRESS)
			pConfig->num_accept_mac = MAX_ACL_MAC_ADDRESS;
		acl_entry = (struct qc_mac_acl_entry *)(pIe + 8);
		for (i = 0; i < pConfig->num_accept_mac; i++) {
			cdf_mem_copy(&pConfig->accept_mac[i], acl_entry->addr,
				     sizeof(qcmacaddr));
			acl_entry++;
		}
	}

	wlan_hdd_set_sap_hwmode(pHostapdAdapter);
	cdf_mem_zero(&sme_config, sizeof(tSmeConfigParams));
	sme_get_config_param(pHddCtx->hHal, &sme_config);
	/* Override hostapd.conf wmm_enabled only for 11n and 11AC configs (IOT)
	 * As per spec 11N/AC STA are QOS STA and may not connect or throughput
	 * may not be good with non QOS 11N AP
	 * Default: enable QOS for SAP unless WMM IE not present for 11bga
	 */
	sme_config.csrConfig.WMMSupportMode = eCsrRoamWmmAuto;
	pIe = wlan_hdd_get_vendor_oui_ie_ptr(WMM_OUI_TYPE, WMM_OUI_TYPE_SIZE,
					pBeacon->tail, pBeacon->tail_len);
	if (!pIe && (pConfig->SapHw_mode == eCSR_DOT11_MODE_11a ||
		pConfig->SapHw_mode == eCSR_DOT11_MODE_11g ||
		pConfig->SapHw_mode == eCSR_DOT11_MODE_11b))
		sme_config.csrConfig.WMMSupportMode = eCsrRoamWmmNoQos;
	sme_update_config(pHddCtx->hHal, &sme_config);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
	/* Linux kernel < 3.8 does not support ch width param. So for
	 * 11AC get from ch width from ini file only if ht40 is enabled
	 * VHT80 depends on HT40 config.
	 */
	if (pConfig->SapHw_mode == eCSR_DOT11_MODE_11ac)
		if (pConfig->ch_width_orig == NL80211_CHAN_WIDTH_40)
			pConfig->ch_width_orig = iniConfig->vhtChannelWidth;
#endif

	if (pConfig->ch_width_orig == NL80211_CHAN_WIDTH_80P80) {
		if (pHddCtx->isVHT80Allowed == false)
			pConfig->ch_width_orig = CH_WIDTH_40MHZ;
		else
			pConfig->ch_width_orig = CH_WIDTH_80P80MHZ;
	} else if (pConfig->ch_width_orig == NL80211_CHAN_WIDTH_160) {
		if (pHddCtx->isVHT80Allowed == false)
			pConfig->ch_width_orig = CH_WIDTH_40MHZ;
		else
			pConfig->ch_width_orig = CH_WIDTH_160MHZ;
	} else if (pConfig->ch_width_orig == NL80211_CHAN_WIDTH_80) {
		if (pHddCtx->isVHT80Allowed == false)
			pConfig->ch_width_orig = CH_WIDTH_40MHZ;
		else
			pConfig->ch_width_orig = CH_WIDTH_80MHZ;
	} else if (pConfig->ch_width_orig == NL80211_CHAN_WIDTH_40) {
		pConfig->ch_width_orig = CH_WIDTH_40MHZ;
	} else {
		pConfig->ch_width_orig = CH_WIDTH_20MHZ;
	}

	if (wlan_hdd_setup_driver_overrides(pHostapdAdapter))
		return -EINVAL;

	/* ht_capab is not what the name conveys,this is used for protection
	 * bitmap */
	pConfig->ht_capab = iniConfig->apProtection;

	if (0 != wlan_hdd_cfg80211_update_apies(pHostapdAdapter)) {
		hddLog(LOGE, FL("SAP Not able to set AP IEs"));
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);
		return -EINVAL;
	}
	/* Uapsd Enabled Bit */
	pConfig->UapsdEnable = iniConfig->apUapsdEnabled;
	/* Enable OBSS protection */
	pConfig->obssProtEnabled = iniConfig->apOBSSProtEnabled;

	if (pHostapdAdapter->device_mode == WLAN_HDD_SOFTAP)
		pConfig->sap_dot11mc =
		    (WLAN_HDD_GET_CTX(pHostapdAdapter))->config->sap_dot11mc;
	else /* for P2P-Go case */
		pConfig->sap_dot11mc = 1;

	hddLog(LOG1, FL("11MC Support Enabled : %d\n"),
		pConfig->sap_dot11mc);

#ifdef WLAN_FEATURE_11W
	pConfig->mfpCapable = MFPCapable;
	pConfig->mfpRequired = MFPRequired;
	hddLog(LOG1, FL("Soft AP MFP capable %d, MFP required %d"),
	       pConfig->mfpCapable, pConfig->mfpRequired);
#endif

	hddLog(LOGW, FL("SOftAP macaddress : " MAC_ADDRESS_STR),
	       MAC_ADDR_ARRAY(pHostapdAdapter->macAddressCurrent.bytes));
	hddLog(LOGW, FL("ssid =%s, beaconint=%d, channel=%d"),
	       pConfig->SSIDinfo.ssid.ssId, (int)pConfig->beacon_int,
	       (int)pConfig->channel);
	hddLog(LOGW, FL("hw_mode=%x, privacy=%d, authType=%d"),
	       pConfig->SapHw_mode, pConfig->privacy, pConfig->authType);
	hddLog(LOGW, FL("RSN/WPALen=%d, Uapsd = %d"),
	       (int)pConfig->RSNWPAReqIELength, pConfig->UapsdEnable);
	hddLog(LOGW, FL("ProtEnabled = %d, OBSSProtEnabled = %d"),
	       pConfig->protEnabled, pConfig->obssProtEnabled);

	if (test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags)) {
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);
		/* Bss already started. just return. */
		/* TODO Probably it should update some beacon params. */
		hddLog(LOGE, "Bss Already started...Ignore the request");
		EXIT();
		return 0;
	}

	if (!cds_allow_concurrency(pHddCtx,
				cds_convert_device_mode_to_hdd_type(
				pHostapdAdapter->device_mode),
				pConfig->channel, HW_MODE_20_MHZ)) {
		hddLog(LOGW,
			FL("This concurrency combination is not allowed"));
		return -EINVAL;
	}

	if (!cds_set_connection_in_progress(pHddCtx, true)) {
		hdd_err("Can't start BSS: set connnection in progress failed");
		return -EINVAL;
	}

	pConfig->persona = pHostapdAdapter->device_mode;

	pSapEventCallback = hdd_hostapd_sap_event_cb;

	(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->dfs_cac_block_tx = true;

	cdf_event_reset(&pHostapdState->cdf_event);
	status = wlansap_start_bss(
#ifdef WLAN_FEATURE_MBSSID
		WLAN_HDD_GET_SAP_CTX_PTR
			(pHostapdAdapter),
#else
		p_cds_context,
#endif
		pSapEventCallback, pConfig,
		pHostapdAdapter->dev);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);
		cds_set_connection_in_progress(pHddCtx, false);
		hddLog(LOGE, FL("SAP Start Bss fail"));
		return -EINVAL;
	}

	hddLog(LOG1,
	       FL("Waiting for Scan to complete(auto mode) and BSS to start"));

	status = cdf_wait_single_event(&pHostapdState->cdf_event, 10000);

	wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);

	if (!CDF_IS_STATUS_SUCCESS(status)) {
		hddLog(LOGE,
			FL("ERROR: HDD cdf wait for single_event failed!!"));
		cds_set_connection_in_progress(pHddCtx, false);
		sme_get_command_q_status(hHal);
#ifdef WLAN_FEATURE_MBSSID
		wlansap_stop_bss(WLAN_HDD_GET_SAP_CTX_PTR(pHostapdAdapter));
#else
		wlansap_stop_bss(p_cds_context);
#endif
		CDF_ASSERT(0);
		return -EINVAL;
	}
	/* Succesfully started Bss update the state bit. */
	set_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);
	/* Initialize WMM configuation */
	hdd_wmm_init(pHostapdAdapter);
	cds_incr_active_session(pHddCtx, pHostapdAdapter->device_mode,
					 pHostapdAdapter->sessionId);
#ifdef DHCP_SERVER_OFFLOAD
	if (iniConfig->enableDHCPServerOffload)
		wlan_hdd_set_dhcp_server_offload(pHostapdAdapter);
#endif /* DHCP_SERVER_OFFLOAD */

#ifdef WLAN_FEATURE_P2P_DEBUG
	if (pHostapdAdapter->device_mode == WLAN_HDD_P2P_GO) {
		if (global_p2p_connection_status == P2P_GO_NEG_COMPLETED) {
			global_p2p_connection_status = P2P_GO_COMPLETED_STATE;
			hddLog(LOGE,
				FL("[P2P State] From Go nego completed to Non-autonomous Group started"));
		} else if (global_p2p_connection_status == P2P_NOT_ACTIVE) {
			global_p2p_connection_status = P2P_GO_COMPLETED_STATE;
			hddLog(LOGE,
				FL("[P2P State] From Inactive to Autonomous Group started"));
		}
	}
#endif

	cds_set_connection_in_progress(pHddCtx, false);
	pHostapdState->bCommit = true;
	EXIT();

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
/**
 * __wlan_hdd_cfg80211_add_beacon() - add beacon in soft ap mode
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 * @params: Pointer to add beacon parameters
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_add_beacon(struct wiphy *wiphy,
					struct net_device *dev,
					struct beacon_parameters *params)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	int status;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_ADD_BEACON,
			 pAdapter->sessionId, params->interval));
	hddLog(LOG2, FL("Device mode %s(%d)"),
		hdd_device_mode_to_string(pAdapter->device_mode),
		pAdapter->device_mode);

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	if (!cds_allow_concurrency(pHddCtx,
				cds_convert_device_mode_to_hdd_type(
				pAdapter->device_mode), 0, HDD_20_MHZ)) {
		hddLog(LOGE,
			FL("This concurrency combination is not allowed"));
		return -EINVAL;
	}

	if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) ||
	    (pAdapter->device_mode == WLAN_HDD_P2P_GO)) {
		beacon_data_t *old, *new;

		old = pAdapter->sessionCtx.ap.beacon;

		if (old) {
			hddLog(LOGW,
				FL("already beacon info added to session(%d)"),
				pAdapter->sessionId);
			return -EALREADY;
		}

		status =
			wlan_hdd_cfg80211_alloc_new_beacon(pAdapter,
							   &new, params);
		if (status != CDF_STATUS_SUCCESS) {
			hddLog(LOGE,
				FL("Error!!! Allocating the new beacon"));
			return -EINVAL;
		}

		pAdapter->sessionCtx.ap.beacon = new;

		status = wlan_hdd_cfg80211_start_bss(pAdapter, params);
		if (0 != status) {
			pAdapter->sessionCtx.ap.beacon = NULL;
			kfree(new);
		}
	}

	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_add_beacon() - add beacon in sap mode
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to netdev
 * @param: Pointer to beacon parameters
 *
 * Return: zero for success non-zero for failure
 */
int wlan_hdd_cfg80211_add_beacon(struct wiphy *wiphy,
				struct net_device *dev,
				struct beacon_parameters *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_add_beacon(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_beacon() - set beacon in soft ap mode
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 * @params: Pointer to set beacon parameters
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_beacon(struct wiphy *wiphy,
					struct net_device *dev,
					struct beacon_parameters *params)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx;
	int status;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_BEACON,
			 pAdapter->sessionId, pHddStaCtx->conn_info.authType));
	hddLog(LOG1, FL("Device_mode %s(%d)"),
		hdd_device_mode_to_string(pAdapter->device_mode),
		pAdapter->device_mode);

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) ||
	    (pAdapter->device_mode == WLAN_HDD_P2P_GO)) {
		beacon_data_t *old, *new;

		old = pAdapter->sessionCtx.ap.beacon;

		if (!old) {
			hddLog(LOGE,
				FL("session(%d) old and new heads points to NULL"),
				pAdapter->sessionId);
			return -ENOENT;
		}

		status =
			wlan_hdd_cfg80211_alloc_new_beacon(pAdapter,
							   &new, params);

		if (status != CDF_STATUS_SUCCESS) {
			hddLog(LOGE,
				FL("Error!!! Allocating the new beacon"));
			return -EINVAL;
		}

		pAdapter->sessionCtx.ap.beacon = new;
		status = wlan_hdd_cfg80211_start_bss(pAdapter, params);
	}

	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_set_beacon() - set beacon in sap mode
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to netdev
 * @param: Pointer to beacon parameters
 *
 * Return: zero for success non-zero for failure
 */
int wlan_hdd_cfg80211_set_beacon(struct wiphy *wiphy,
				struct net_device *dev,
				struct beacon_parameters *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_beacon(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
/**
 * __wlan_hdd_cfg80211_del_beacon() - stop soft ap
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_del_beacon(struct wiphy *wiphy,
					struct net_device *dev)
#else
/**
 * __wlan_hdd_cfg80211_stop_ap() - stop soft ap
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_stop_ap(struct wiphy *wiphy,
					struct net_device *dev)
#endif
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = NULL;
	hdd_scaninfo_t *pScanInfo = NULL;
	hdd_adapter_t *staAdapter = NULL;
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	tSirUpdateIE updateIE;
	beacon_data_t *old;
	int ret;
	unsigned long rc;
	hdd_adapter_list_node_t *pAdapterNode = NULL;
	hdd_adapter_list_node_t *pNext = NULL;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_STOP_AP,
			 pAdapter->sessionId, pAdapter->device_mode));

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);
	if (0 != ret)
		return ret;

	if (!(pAdapter->device_mode == WLAN_HDD_SOFTAP ||
	      pAdapter->device_mode == WLAN_HDD_P2P_GO)) {
		return -EOPNOTSUPP;
	}

	hddLog(LOG1, FL("Device_mode %s(%d)"),
		hdd_device_mode_to_string(pAdapter->device_mode),
		pAdapter->device_mode);

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && CDF_STATUS_SUCCESS == status) {
		staAdapter = pAdapterNode->pAdapter;

		if (WLAN_HDD_INFRA_STATION == staAdapter->device_mode ||
		    (WLAN_HDD_P2P_CLIENT == staAdapter->device_mode) ||
		    (WLAN_HDD_P2P_GO == staAdapter->device_mode)) {
			pScanInfo = &staAdapter->scan_info;

			if (pScanInfo && pScanInfo->mScanPending) {
				hddLog(LOG1, FL("Aborting pending scan for device mode:%d"),
				       staAdapter->device_mode);
				INIT_COMPLETION(pScanInfo->abortscan_event_var);
				hdd_abort_mac_scan(staAdapter->pHddCtx,
						   staAdapter->sessionId,
						   eCSR_SCAN_ABORT_DEFAULT);
				rc = wait_for_completion_timeout(
					&pScanInfo->abortscan_event_var,
					msecs_to_jiffies(
						WLAN_WAIT_TIME_ABORTSCAN));
				if (!rc) {
					hddLog(LOGE,
					       FL("Timeout occurred while waiting for abortscan"));
					CDF_ASSERT(pScanInfo->mScanPending);
				}
			}
		}

		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	/*
	 * When ever stop ap adapter gets called, we need to check
	 * whether any restart AP work is pending. If any restart is pending
	 * then lets finish it and go ahead from there.
	 */
	if (pHddCtx->config->conc_custom_rule1 &&
	    (WLAN_HDD_SOFTAP == pAdapter->device_mode)) {
		cds_flush_work(&pHddCtx->sap_start_work);
		hddLog(LOGW, FL("Canceled the pending restart work"));
		spin_lock(&pHddCtx->sap_update_info_lock);
		pHddCtx->is_sap_restart_required = false;
		spin_unlock(&pHddCtx->sap_update_info_lock);
	}
	pAdapter->sessionCtx.ap.sapConfig.acs_cfg.acs_mode = false;
	if (pAdapter->sessionCtx.ap.sapConfig.acs_cfg.ch_list)
		cdf_mem_free(pAdapter->sessionCtx.ap.sapConfig.acs_cfg.ch_list);
	cdf_mem_zero(&pAdapter->sessionCtx.ap.sapConfig.acs_cfg,
						sizeof(struct sap_acs_cfg));
	hdd_hostapd_stop(dev);

	old = pAdapter->sessionCtx.ap.beacon;
	if (!old) {
		hddLog(LOGE,
		       FL("Session(%d) beacon data points to NULL"),
		       pAdapter->sessionId);
		return -EINVAL;
	}

	hdd_cleanup_actionframe(pHddCtx, pAdapter);

	mutex_lock(&pHddCtx->sap_lock);
	if (test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags)) {
		hdd_hostapd_state_t *pHostapdState =
			WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

		cdf_event_reset(&pHostapdState->cdf_stop_bss_event);
#ifdef WLAN_FEATURE_MBSSID
		status = wlansap_stop_bss(WLAN_HDD_GET_SAP_CTX_PTR(pAdapter));
#else
		status = wlansap_stop_bss(pHddCtx->pcds_context);
#endif
		if (CDF_IS_STATUS_SUCCESS(status)) {
			status =
				cdf_wait_single_event(&pHostapdState->
						      cdf_stop_bss_event,
						      10000);

			if (!CDF_IS_STATUS_SUCCESS(status)) {
				hddLog(LOGE,
				       FL("HDD cdf wait for single_event failed!!"));
				CDF_ASSERT(0);
			}
		}
		clear_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags);
		/*BSS stopped, clear the active sessions for this device mode*/
		cds_decr_session_set_pcl(pHddCtx,
						pAdapter->device_mode,
						pAdapter->sessionId);
		pAdapter->sessionCtx.ap.beacon = NULL;
		kfree(old);
	}
	mutex_unlock(&pHddCtx->sap_lock);

	if (status != CDF_STATUS_SUCCESS) {
		hddLog(LOGE, FL("Stopping the BSS"));
		return -EINVAL;
	}

	cdf_copy_macaddr(&updateIE.bssid, &pAdapter->macAddressCurrent);
	updateIE.smeSessionId = pAdapter->sessionId;
	updateIE.ieBufferlength = 0;
	updateIE.pAdditionIEBuffer = NULL;
	updateIE.append = true;
	updateIE.notify = true;
	if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(pAdapter),
			      &updateIE,
			      eUPDATE_IE_PROBE_BCN) == CDF_STATUS_E_FAILURE) {
		hddLog(LOGE, FL("Could not pass on PROBE_RSP_BCN data to PE"));
	}

	if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(pAdapter),
			      &updateIE,
			      eUPDATE_IE_ASSOC_RESP) == CDF_STATUS_E_FAILURE) {
		hddLog(LOGE, FL("Could not pass on ASSOC_RSP data to PE"));
	}
	/* Reset WNI_CFG_PROBE_RSP Flags */
	wlan_hdd_reset_prob_rspies(pAdapter);

#ifdef WLAN_FEATURE_P2P_DEBUG
	if ((pAdapter->device_mode == WLAN_HDD_P2P_GO) &&
	    (global_p2p_connection_status == P2P_GO_COMPLETED_STATE)) {
		hddLog(LOGE,
			"[P2P State] From GO completed to Inactive state GO got removed");
		global_p2p_connection_status = P2P_NOT_ACTIVE;
	}
#endif
	EXIT();
	return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
/**
 * wlan_hdd_get_channel_bw() - get channel bandwidth
 * @width: input channel width in nl80211_chan_width value
 *
 * Return: channel width value defined by driver
 */
static enum hw_mode_bandwidth wlan_hdd_get_channel_bw(
					enum nl80211_chan_width width)
{
	enum hw_mode_bandwidth ch_bw = HW_MODE_20_MHZ;

	switch (width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		ch_bw = HW_MODE_20_MHZ;
		break;
	case NL80211_CHAN_WIDTH_40:
		ch_bw = HW_MODE_40_MHZ;
		break;
	case NL80211_CHAN_WIDTH_80:
		ch_bw = HW_MODE_80_MHZ;
		break;
	case NL80211_CHAN_WIDTH_80P80:
		ch_bw = HW_MODE_80_PLUS_80_MHZ;
		break;
	case NL80211_CHAN_WIDTH_160:
		ch_bw = HW_MODE_160_MHZ;
		break;
	default:
		hdd_err("Invalid width: %d, using default 20MHz", width);
		break;
	}

	return ch_bw;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) && !defined(WITH_BACKPORTS)
/**
 * wlan_hdd_cfg80211_del_beacon() - delete beacon in sap mode
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to netdev
 *
 * Return: zero for success non-zero for failure
 */
int wlan_hdd_cfg80211_del_beacon(struct wiphy *wiphy,
				struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_del_beacon(wiphy, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#else
/**
 * wlan_hdd_cfg80211_stop_ap() - stop sap
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to netdev
 *
 * Return: zero for success non-zero for failure
 */
int wlan_hdd_cfg80211_stop_ap(struct wiphy *wiphy,
				struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_stop_ap(wiphy, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0))
/**
 * __wlan_hdd_cfg80211_start_ap() - start soft ap mode
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 * @params: Pointer to AP settings parameters
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_start_ap(struct wiphy *wiphy,
					struct net_device *dev,
					struct cfg80211_ap_settings *params)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	enum hw_mode_bandwidth channel_width;
	int status;
	uint8_t channel;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_START_AP, pAdapter->sessionId,
			 params->beacon_interval));
	if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) {
		hddLog(LOGE, FL("HDD adapter magic is invalid"));
		return -ENODEV;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	hddLog(LOG2, FL("pAdapter = %p, Device mode %s(%d)"), pAdapter,
	       hdd_device_mode_to_string(pAdapter->device_mode),
	       pAdapter->device_mode);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
	channel_width = wlan_hdd_get_channel_bw(params->chandef.width);
	channel = ieee80211_frequency_to_channel(
				params->chandef.chan->center_freq);
#else
	channel_width = HW_MODE_20_MHZ;
	channel = 0;
#endif
	/* check if concurrency is allowed */
	if (!cds_allow_concurrency(pHddCtx,
				cds_convert_device_mode_to_hdd_type(
				pAdapter->device_mode),
				channel,
				channel_width)) {
		hdd_err("Connection failed due to concurrency check failure");
		return -EINVAL;
	}
	if (pHddCtx->config->policy_manager_enabled) {
		status = cdf_reset_connection_update();
		if (!CDF_IS_STATUS_SUCCESS(status))
			hdd_err("ERR: clear event failed");

		status = cds_current_connections_update(pAdapter->sessionId,
				channel,
				CDS_UPDATE_REASON_START_AP);
		if (CDF_STATUS_E_FAILURE == status) {
			hdd_err("ERROR: connections update failed!!");
			return -EINVAL;
		}

		if (CDF_STATUS_SUCCESS == status) {
			status = cdf_wait_for_connection_update();
			if (!CDF_IS_STATUS_SUCCESS(status)) {
				hdd_err("ERROR: cdf wait for event failed!!");
				return -EINVAL;
			}
		}
	}

	if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
	    || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
	    ) {
		beacon_data_t *old, *new;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
		enum nl80211_channel_type channel_type;
#endif

		old = pAdapter->sessionCtx.ap.beacon;

		if (old)
			return -EALREADY;

		status =
			wlan_hdd_cfg80211_alloc_new_beacon(pAdapter, &new,
							   &params->beacon,
							   params->dtim_period);

		if (status != 0) {
			hddLog(LOGE, FL("Error!!! Allocating the new beacon"));
			return -EINVAL;
		}
		pAdapter->sessionCtx.ap.beacon = new;

		if (params->chandef.width < NL80211_CHAN_WIDTH_80)
			channel_type = cfg80211_get_chandef_type(
						&(params->chandef));
		else
			channel_type = NL80211_CHAN_HT40PLUS;


		wlan_hdd_set_channel(wiphy, dev,
				     &params->chandef,
				     channel_type);

		/* set authentication type */
		switch (params->auth_type) {
		case NL80211_AUTHTYPE_OPEN_SYSTEM:
			pAdapter->sessionCtx.ap.sapConfig.authType =
				eSAP_OPEN_SYSTEM;
			break;
		case NL80211_AUTHTYPE_SHARED_KEY:
			pAdapter->sessionCtx.ap.sapConfig.authType =
				eSAP_SHARED_KEY;
			break;
		default:
			pAdapter->sessionCtx.ap.sapConfig.authType =
				eSAP_AUTO_SWITCH;
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
		pAdapter->sessionCtx.ap.sapConfig.ch_width_orig =
						params->chandef.width;
#endif
		status =
			wlan_hdd_cfg80211_start_bss(pAdapter,
				&params->beacon,
				params->ssid, params->ssid_len,
				params->hidden_ssid);
	}

	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_start_ap() - start sap
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to netdev
 * @params: Pointer to start ap configuration parameters
 *
 * Return: zero for success non-zero for failure
 */
int wlan_hdd_cfg80211_start_ap(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_ap_settings *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_start_ap(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_change_beacon() - change beacon for sofatap/p2p go
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 * @params: Pointer to change beacon parameters
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_change_beacon(struct wiphy *wiphy,
					struct net_device *dev,
					struct cfg80211_beacon_data *params)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	beacon_data_t *old, *new;
	int status;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_CHANGE_BEACON,
			 pAdapter->sessionId, pAdapter->device_mode));
	hddLog(LOG1, FL("Device_mode %s(%d)"),
	       hdd_device_mode_to_string(pAdapter->device_mode),
	       pAdapter->device_mode);

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	if (!(pAdapter->device_mode == WLAN_HDD_SOFTAP ||
	      pAdapter->device_mode == WLAN_HDD_P2P_GO)) {
		return -EOPNOTSUPP;
	}

	old = pAdapter->sessionCtx.ap.beacon;

	if (!old) {
		hddLog(LOGE, FL("session(%d) beacon data points to NULL"),
		       pAdapter->sessionId);
		return -EINVAL;
	}

	status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter, &new, params, 0);

	if (status != CDF_STATUS_SUCCESS) {
		hddLog(LOGE, FL("new beacon alloc failed"));
		return -EINVAL;
	}

	pAdapter->sessionCtx.ap.beacon = new;
	status = wlan_hdd_cfg80211_start_bss(pAdapter, params, NULL, 0, 0);

	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_change_beacon() - change beacon content in sap mode
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to netdev
 * @params: Pointer to change beacon parameters
 *
 * Return: zero for success non-zero for failure
 */
int wlan_hdd_cfg80211_change_beacon(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_beacon_data *params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_change_beacon(wiphy, dev, params);
	cds_ssr_unprotect(__func__);

	return ret;
}

#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)) */
