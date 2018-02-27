/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
#include <cdp_txrx_stats.h>
#include <cdp_txrx_cmn.h>
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
#include "pld_common.h"

#include "wma.h"
#ifdef WLAN_DEBUG
#include "wma_api.h"
#endif
#include "wlan_hdd_trace.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_hdd_cfg.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_hdd_tsf.h"
#include <cdp_txrx_misc.h>
#include "wlan_hdd_power.h"
#include "wlan_hdd_object_manager.h"
#include <qca_vendor.h>
#include <cds_api.h>
#include <cdp_txrx_stats.h>
#include "wlan_hdd_he.h"
#include "wlan_dfs_tgt_api.h"
#include "wlan_dfs_utils_api.h"
#include <wlan_reg_ucfg_api.h>
#include "wlan_utility.h"
#include <wlan_p2p_ucfg_api.h>
#include "sir_api.h"
#include <wlan_green_ap_ucfg_api.h>

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

/*
 * 11B, 11G Rate table include Basic rate and Extended rate
 * The IDX field is the rate index
 * The HI field is the rate when RSSI is strong or being ignored
 * (in this case we report actual rate)
 * The MID field is the rate when RSSI is moderate
 * (in this case we cap 11b rates at 5.5 and 11g rates at 24)
 * The LO field is the rate when RSSI is low
 * (in this case we don't report rates, actual current rate used)
 */
static const struct index_data_rate_type supported_data_rate[] = {
	/* IDX     HI  HM  LM LO (RSSI-based index */
	{2,   { 10,  10, 10, 0} },
	{4,   { 20,  20, 10, 0} },
	{11,  { 55,  20, 10, 0} },
	{12,  { 60,  55, 20, 0} },
	{18,  { 90,  55, 20, 0} },
	{22,  {110,  55, 20, 0} },
	{24,  {120,  90, 60, 0} },
	{36,  {180, 120, 60, 0} },
	{44,  {220, 180, 60, 0} },
	{48,  {240, 180, 90, 0} },
	{66,  {330, 180, 90, 0} },
	{72,  {360, 240, 90, 0} },
	{96,  {480, 240, 120, 0} },
	{108, {540, 240, 120, 0} }
};

/* MCS Based rate table */
/* HT MCS parameters with Nss = 1 */
static const struct index_data_rate_type supported_mcs_rate_nss1[] = {
	/* MCS  L20   L40   S20  S40 */
	{0,  { 65,  135,  72,  150} },
	{1,  { 130, 270,  144, 300} },
	{2,  { 195, 405,  217, 450} },
	{3,  { 260, 540,  289, 600} },
	{4,  { 390, 810,  433, 900} },
	{5,  { 520, 1080, 578, 1200} },
	{6,  { 585, 1215, 650, 1350} },
	{7,  { 650, 1350, 722, 1500} }
};

/* HT MCS parameters with Nss = 2 */
static const struct index_data_rate_type supported_mcs_rate_nss2[] = {
	/* MCS  L20    L40   S20   S40 */
	{0,  {130,  270,  144,  300} },
	{1,  {260,  540,  289,  600} },
	{2,  {390,  810,  433,  900} },
	{3,  {520,  1080, 578,  1200} },
	{4,  {780,  1620, 867,  1800} },
	{5,  {1040, 2160, 1156, 2400} },
	{6,  {1170, 2430, 1300, 2700} },
	{7,  {1300, 2700, 1444, 3000} }
};

/* MCS Based VHT rate table */
/* MCS parameters with Nss = 1*/
static const struct index_vht_data_rate_type supported_vht_mcs_rate_nss1[] = {
	/* MCS  L80    S80     L40   S40    L20   S40*/
	{0,  {293,  325},  {135,  150},  {65,   72} },
	{1,  {585,  650},  {270,  300},  {130,  144} },
	{2,  {878,  975},  {405,  450},  {195,  217} },
	{3,  {1170, 1300}, {540,  600},  {260,  289} },
	{4,  {1755, 1950}, {810,  900},  {390,  433} },
	{5,  {2340, 2600}, {1080, 1200}, {520,  578} },
	{6,  {2633, 2925}, {1215, 1350}, {585,  650} },
	{7,  {2925, 3250}, {1350, 1500}, {650,  722} },
	{8,  {3510, 3900}, {1620, 1800}, {780,  867} },
	{9,  {3900, 4333}, {1800, 2000}, {780,  867} }
};

/*MCS parameters with Nss = 2*/
static const struct index_vht_data_rate_type supported_vht_mcs_rate_nss2[] = {
	/* MCS  L80    S80     L40   S40    L20   S40*/
	{0,  {585,  650},  {270,  300},  {130,  144} },
	{1,  {1170, 1300}, {540,  600},  {260,  289} },
	{2,  {1755, 1950}, {810,  900},  {390,  433} },
	{3,  {2340, 2600}, {1080, 1200}, {520,  578} },
	{4,  {3510, 3900}, {1620, 1800}, {780,  867} },
	{5,  {4680, 5200}, {2160, 2400}, {1040, 1156} },
	{6,  {5265, 5850}, {2430, 2700}, {1170, 1300} },
	{7,  {5850, 6500}, {2700, 3000}, {1300, 1444} },
	{8,  {7020, 7800}, {3240, 3600}, {1560, 1733} },
	{9,  {7800, 8667}, {3600, 4000}, {1560, 1733} }
};

/* Function definitions */

/**
 * hdd_sap_context_init() - Initialize SAP context.
 * @hdd_ctx:	HDD context.
 *
 * Initialize SAP context.
 *
 * Return: 0 on success.
 */
int hdd_sap_context_init(struct hdd_context *hdd_ctx)
{
	qdf_wake_lock_create(&hdd_ctx->sap_dfs_wakelock, "sap_dfs_wakelock");
	atomic_set(&hdd_ctx->sap_dfs_ref_cnt, 0);

	mutex_init(&hdd_ctx->sap_lock);
	qdf_wake_lock_create(&hdd_ctx->sap_wake_lock, "qcom_sap_wakelock");
	qdf_spinlock_create(&hdd_ctx->sap_update_info_lock);

	qdf_atomic_init(&hdd_ctx->is_acs_allowed);

	return 0;
}

/**
 * hdd_hostapd_init_sap_session() - To init the sap session completely
 * @adapter: SAP/GO adapter
 *
 * This API will do
 * 1) sap_init_ctx()
 *
 * Return: 0 if success else non-zero value.
 */
static struct sap_context *
hdd_hostapd_init_sap_session(struct hdd_adapter *adapter)
{
	struct sap_context *sap_ctx;
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("invalid adapter");
		return NULL;
	}

	sap_ctx = adapter->session.ap.sap_context;

	if (!sap_ctx) {
		hdd_err("can't allocate the sap_ctx");
		return NULL;
	}
	status = sap_init_ctx(sap_ctx, adapter->device_mode,
			       adapter->mac_addr.bytes,
			       adapter->session_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("wlansap_start failed!! status: %d", status);
		adapter->session.ap.sap_context = NULL;
		goto error;
	}
	return sap_ctx;
error:
	wlansap_context_put(sap_ctx);
	hdd_err("releasing the sap context for session-id:%d",
		adapter->session_id);

	return NULL;
}

/**
 * hdd_hostapd_deinit_sap_session() - To de-init the sap session completely
 * @adapter: SAP/GO adapter
 *
 * This API will do
 * 1) sap_init_ctx()
 * 2) sap_destroy_ctx()
 *
 * Return: 0 if success else non-zero value.
 */
static int hdd_hostapd_deinit_sap_session(struct hdd_adapter *adapter)
{
	struct sap_context *sap_ctx;
	int status = 0;

	if (!adapter) {
		hdd_err("invalid adapter");
		return -EINVAL;
	}

	sap_ctx = WLAN_HDD_GET_SAP_CTX_PTR(adapter);
	if (!sap_ctx) {
		hdd_debug("sap context already released, nothing to be done");
		return 0;
	}

	if (!QDF_IS_STATUS_SUCCESS(sap_deinit_ctx(sap_ctx))) {
		hdd_err("Error stopping the sap session");
		status = -EINVAL;
	}

	if (!QDF_IS_STATUS_SUCCESS(sap_destroy_ctx(sap_ctx))) {
		hdd_err("Error closing the sap session");
		status = -EINVAL;
	}
	adapter->session.ap.sap_context = NULL;

	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_debug("sap has issue closing the session");
	else
		hdd_debug("sap has been closed successfully");


	return status;
}

/**
 * hdd_hostapd_channel_allow_suspend() - allow suspend in a channel.
 * Called when, 1. bss stopped, 2. channel switch
 *
 * @adapter: pointer to hdd adapter
 * @channel: current channel
 *
 * Return: None
 */
static void hdd_hostapd_channel_allow_suspend(struct hdd_adapter *adapter,
					      uint8_t channel)
{

	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_hostapd_state *hostapd_state =
		WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	hdd_debug("bss_state: %d, channel: %d, dfs_ref_cnt: %d",
	       hostapd_state->bss_state, channel,
	       atomic_read(&hdd_ctx->sap_dfs_ref_cnt));

	/* Return if BSS is already stopped */
	if (hostapd_state->bss_state == BSS_STOP)
		return;

	if (CHANNEL_STATE_DFS != wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
				channel))
		return;

	/* Release wakelock when no more DFS channels are used */
	if (atomic_dec_and_test(&hdd_ctx->sap_dfs_ref_cnt)) {
		hdd_err("DFS: allowing suspend (chan: %d)", channel);
		qdf_wake_lock_release(&hdd_ctx->sap_dfs_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_DFS);
		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.dfs);

	}
}

/**
 * hdd_hostapd_channel_prevent_suspend() - prevent suspend in a channel.
 * Called when, 1. bss started, 2. channel switch
 *
 * @adapter: pointer to hdd adapter
 * @channel: current channel
 *
 * Return - None
 */
static void hdd_hostapd_channel_prevent_suspend(struct hdd_adapter *adapter,
						uint8_t channel)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_hostapd_state *hostapd_state =
		WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	hdd_debug("bss_state: %d, channel: %d, dfs_ref_cnt: %d",
	       hostapd_state->bss_state, channel,
	       atomic_read(&hdd_ctx->sap_dfs_ref_cnt));

	/* Return if BSS is already started && wakelock is acquired */
	if ((hostapd_state->bss_state == BSS_START) &&
		(atomic_read(&hdd_ctx->sap_dfs_ref_cnt) >= 1))
		return;

	if (CHANNEL_STATE_DFS != wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
				channel))
		return;

	/* Acquire wakelock if we have at least one DFS channel in use */
	if (atomic_inc_return(&hdd_ctx->sap_dfs_ref_cnt) == 1) {
		hdd_err("DFS: preventing suspend (chan: %d)", channel);
		qdf_runtime_pm_prevent_suspend(&hdd_ctx->runtime_context.dfs);
		qdf_wake_lock_acquire(&hdd_ctx->sap_dfs_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_DFS);
	}
}

/**
 * hdd_sap_context_destroy() - Destroy SAP context
 *
 * @hdd_ctx:	HDD context.
 *
 * Destroy SAP context.
 *
 * Return: None
 */
void hdd_sap_context_destroy(struct hdd_context *hdd_ctx)
{
	if (atomic_read(&hdd_ctx->sap_dfs_ref_cnt)) {
		qdf_wake_lock_release(&hdd_ctx->sap_dfs_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_DRIVER_EXIT);

		atomic_set(&hdd_ctx->sap_dfs_ref_cnt, 0);
		hdd_debug("DFS: Allowing suspend");
	}

	qdf_wake_lock_destroy(&hdd_ctx->sap_dfs_wakelock);

	mutex_destroy(&hdd_ctx->sap_lock);
	qdf_wake_lock_destroy(&hdd_ctx->sap_wake_lock);

	qdf_spinlock_destroy(&hdd_ctx->sap_update_info_lock);

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
	struct hdd_adapter *adapter = netdev_priv(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;

	ENTER_DEV(dev);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_HOSTAPD_OPEN_REQUEST, NO_SESSION, 0));
	/* Nothing to be done if device is unloading */
	if (cds_is_driver_unloading()) {
		hdd_err("Driver is unloading can not open the hdd");
		return -EBUSY;
	}

	if (cds_is_driver_recovering()) {
		hdd_err("WLAN is currently recovering; Please try again.");
		return -EBUSY;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;
	/*
	 * Check statemachine state and also stop iface change timer if running
	 */
	ret = hdd_wlan_start_modules(hdd_ctx, adapter, false);

	if (ret) {
		hdd_err("Failed to start WLAN modules return");
		return ret;
	}

	ret = hdd_start_adapter(adapter);
	if (ret) {
		hdd_err("Error Initializing the AP mode: %d", ret);
		return ret;
	}

	set_bit(DEVICE_IFACE_OPENED, &adapter->event_flags);

	/* Enable all Tx queues */
	hdd_debug("Enabling queues");
	wlan_hdd_netif_queue_control(adapter,
				   WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
				   WLAN_CONTROL_PATH);
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;

	ENTER_DEV(dev);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	/*
	 * Some tests requires to do "ifconfig down" only to bring
	 * down the SAP/GO without killing hostapd/wpa_supplicant.
	 * In such case, user will do "ifconfig up" to bring-back
	 * the SAP/GO session. to fulfill this requirement, driver
	 * needs to de-init the sap session here and re-init when
	 * __hdd_hostapd_open() API
	 */
	hdd_stop_adapter(hdd_ctx, adapter);
	hdd_deinit_adapter(hdd_ctx, adapter, true);
	clear_bit(DEVICE_IFACE_OPENED, &adapter->event_flags);

	if (!hdd_is_cli_iface_up(hdd_ctx))
		sme_scan_flush_result(hdd_ctx->hHal);

	/* Stop all tx queues */
	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
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
	struct hdd_adapter *adapter = netdev_priv(dev);
	struct hdd_context *hdd_ctx;

	ENTER_DEV(dev);

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("Invalid magic");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (NULL == hdd_ctx) {
		hdd_err("NULL hdd_ctx");
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
	ENTER_DEV(dev);

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
QDF_STATUS hdd_set_sap_ht2040_mode(struct hdd_adapter *adapter,
				   uint8_t channel_type)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;

	hdd_debug("change HT20/40 mode");

	if (QDF_SAP_MODE == adapter->device_mode) {
		hHal = WLAN_HDD_GET_HAL_CTX(adapter);
		if (NULL == hHal) {
			hdd_err("Hal ctx is null");
			return QDF_STATUS_E_FAULT;
		}
		qdf_ret_status =
			sme_set_ht2040_mode(hHal, adapter->session_id,
					channel_type, true);
		if (qdf_ret_status == QDF_STATUS_E_FAILURE) {
			hdd_err("Failed to change HT20/40 mode");
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
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
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	int ret = 0;

	ENTER_DEV(dev);

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

/**
 * hdd_hostapd_inactivity_timer_cb() - Inactivity timeout handler
 * @context: Context registered with qdf_mc_timer_init()
 *
 * This is the callback function registered with qdf_mc_timer_init()
 * to handle the AP inactivity timer. The @context registered is the
 * struct net_device associated with the interface.  When this
 * function is called it means the AP inactivity timer has fired, and
 * this function in turn indicates the timeout to userspace.
 */

static void hdd_hostapd_inactivity_timer_cb(void *context)
{
	struct net_device *dev = (struct net_device *)context;
	uint8_t we_custom_event[64];
	union iwreq_data wrqu;
#ifdef DISABLE_CONCURRENCY_AUTOSAVE
	QDF_STATUS qdf_status;
	struct hdd_adapter *adapter;
	struct hdd_ap_ctx *ap_ctx;
	struct hdd_context *hdd_ctx;
#endif /* DISABLE_CONCURRENCY_AUTOSAVE */

	/* event_name space-delimiter driver_module_name
	 * Format of the event is "AUTO-SHUT.indication" " " "module_name"
	 */
	char *autoShutEvent = "AUTO-SHUT.indication" " " KBUILD_MODNAME;

	/* For the NULL at the end */
	int event_len = strlen(autoShutEvent) + 1;

	ENTER_DEV(dev);

#ifdef DISABLE_CONCURRENCY_AUTOSAVE
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (policy_mgr_concurrent_open_sessions_running(hdd_ctx->hdd_psoc)) {
		/*
		 * This timer routine is going to be called only when AP
		 * persona is up.
		 * If there are concurrent sessions running we do not want
		 * to shut down the Bss.Instead we run the timer again so
		 * that if Autosave is enabled next time and other session
		   was down only then we bring down AP
		 */
		adapter = netdev_priv(dev);
		if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
			hdd_err("invalid adapter: %pK", adapter);
			return;
		}
		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
		qdf_status =
			qdf_mc_timer_start(&ap_ctx->hdd_ap_inactivity_timer,
					(WLAN_HDD_GET_CTX(adapter))->
					config->nAPAutoShutOff * 1000);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			hdd_err("Failed to init AP inactivity timer");

		EXIT();
		return;
	}
#endif /* DISABLE_CONCURRENCY_AUTOSAVE */
	memset(&we_custom_event, '\0', sizeof(we_custom_event));
	memcpy(&we_custom_event, autoShutEvent, event_len);

	memset(&wrqu, 0, sizeof(wrqu));
	wrqu.data.length = event_len;

	hdd_debug("Shutting down AP interface due to inactivity");
	wireless_send_event(dev, IWEVCUSTOM, &wrqu, (char *)we_custom_event);

	EXIT();
}

static void hdd_clear_sta(struct hdd_adapter *adapter, uint8_t sta_id)
{
	struct hdd_ap_ctx *ap_ctx;
	struct hdd_station_info *sta_info;
	struct csr_del_sta_params del_sta_params;

	ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);

	if (sta_id == ap_ctx->broadcast_sta_id)
		return;

	sta_info = &adapter->sta_info[sta_id];
	if (!sta_info->in_use)
		return;

	wlansap_populate_del_sta_params(sta_info->sta_mac.bytes,
					eSIR_MAC_DEAUTH_LEAVING_BSS_REASON,
					(SIR_MAC_MGMT_DISASSOC >> 4),
					&del_sta_params);

	hdd_softap_sta_disassoc(adapter, &del_sta_params);
}

static void hdd_clear_all_sta(struct hdd_adapter *adapter)
{
	uint8_t sta_id;

	ENTER_DEV(adapter->dev);
	for (sta_id = 0; sta_id < WLAN_MAX_STA_COUNT; sta_id++)
		hdd_clear_sta(adapter, sta_id);
}

static int hdd_stop_bss_link(struct hdd_adapter *adapter,
			     void *usrDataForCallback)
{
	struct net_device *dev;
	struct hdd_context *hdd_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	dev = (struct net_device *)usrDataForCallback;
	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		status = wlansap_stop_bss(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter));
		if (QDF_IS_STATUS_SUCCESS(status))
			hdd_debug("Deleting SAP/P2P link!!!!!!");

		clear_bit(SOFTAP_BSS_STARTED, &adapter->event_flags);
		policy_mgr_decr_session_set_pcl(hdd_ctx->hdd_psoc,
					adapter->device_mode,
					adapter->session_id);
	}
	EXIT();
	return (status == QDF_STATUS_SUCCESS) ? 0 : -EBUSY;
}

/**
 * hdd_chan_change_notify() - Function to notify hostapd about channel change
 * @hostapd_adapter:	hostapd adapter
 * @dev:		Net device structure
 * @chan_change:	New channel change parameters
 * @legacy_phymode:	is the phymode legacy
 *
 * This function is used to notify hostapd about the channel change
 *
 * Return: Success on intimating userspace
 *
 */
QDF_STATUS hdd_chan_change_notify(struct hdd_adapter *adapter,
		struct net_device *dev,
		struct hdd_chan_change_params chan_change,
		bool legacy_phymode)
{
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	enum nl80211_channel_type channel_type;
	uint32_t freq;
	tHalHandle  hal = WLAN_HDD_GET_HAL_CTX(adapter);

	if (NULL == hal) {
		hdd_err("hal is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	hdd_debug("chan:%d width:%d sec_ch_offset:%d seg0:%d seg1:%d",
		chan_change.chan, chan_change.chan_params.ch_width,
		chan_change.chan_params.sec_ch_offset,
		chan_change.chan_params.center_freq_seg0,
		chan_change.chan_params.center_freq_seg1);

	freq = cds_chan_to_freq(chan_change.chan);

	chan = ieee80211_get_channel(adapter->wdev.wiphy, freq);

	if (!chan) {
		hdd_err("Invalid input frequency for channel conversion");
		return QDF_STATUS_E_FAILURE;
	}

	if (legacy_phymode) {
		channel_type = NL80211_CHAN_NO_HT;
	} else {
		switch (chan_change.chan_params.sec_ch_offset) {
		case PHY_SINGLE_CHANNEL_CENTERED:
			channel_type = NL80211_CHAN_HT20;
			break;
		case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
			channel_type = NL80211_CHAN_HT40MINUS;
			break;
		case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
			channel_type = NL80211_CHAN_HT40PLUS;
			break;
		default:
			channel_type = NL80211_CHAN_NO_HT;
			break;
		}
	}

	cfg80211_chandef_create(&chandef, chan, channel_type);

	/* cfg80211_chandef_create() does update of width and center_freq1
	 * only for NL80211_CHAN_NO_HT, NL80211_CHAN_HT20, NL80211_CHAN_HT40PLUS
	 * and NL80211_CHAN_HT40MINUS.
	 */
	if (chan_change.chan_params.ch_width == CH_WIDTH_80MHZ)
		chandef.width = NL80211_CHAN_WIDTH_80;
	else if (chan_change.chan_params.ch_width == CH_WIDTH_80P80MHZ)
		chandef.width = NL80211_CHAN_WIDTH_80P80;
	else if (chan_change.chan_params.ch_width == CH_WIDTH_160MHZ)
		chandef.width = NL80211_CHAN_WIDTH_160;

	if ((chan_change.chan_params.ch_width == CH_WIDTH_80MHZ) ||
	    (chan_change.chan_params.ch_width == CH_WIDTH_80P80MHZ) ||
	    (chan_change.chan_params.ch_width == CH_WIDTH_160MHZ)) {
		if (chan_change.chan_params.center_freq_seg0)
			chandef.center_freq1 = cds_chan_to_freq(
				chan_change.chan_params.center_freq_seg0);

		if (chan_change.chan_params.center_freq_seg1)
			chandef.center_freq2 = cds_chan_to_freq(
				chan_change.chan_params.center_freq_seg1);
	}

	hdd_debug("notify: chan:%d width:%d freq1:%d freq2:%d",
		chandef.chan->center_freq, chandef.width, chandef.center_freq1,
		chandef.center_freq2);

	cfg80211_ch_switch_notify(dev, &chandef);

	return QDF_STATUS_SUCCESS;
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
static QDF_STATUS hdd_send_radar_event(struct hdd_context *hdd_context,
				       eSapHddEvent event,
				       struct wlan_dfs_info dfs_info,
				       struct wireless_dev *wdev)
{

	struct sk_buff *vendor_event;
	enum qca_nl80211_vendor_subcmds_index index;
	uint32_t freq, ret;
	uint32_t data_size;

	if (!hdd_context) {
		hdd_err("HDD context is NULL");
		return QDF_STATUS_E_FAILURE;
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
		return QDF_STATUS_E_FAILURE;
	}

	vendor_event = cfg80211_vendor_event_alloc(hdd_context->wiphy,
			wdev,
			data_size + NLMSG_HDRLEN,
			index,
			GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed for %d", index);
		return QDF_STATUS_E_FAILURE;
	}

	ret = nla_put_u32(vendor_event, NL80211_ATTR_WIPHY_FREQ, freq);

	if (ret) {
		hdd_err("NL80211_ATTR_WIPHY_FREQ put fail");
		kfree_skb(vendor_event);
		return QDF_STATUS_E_FAILURE;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_send_conditional_chan_switch_status() - Send conditional channel switch
 * status
 * @hdd_ctx: HDD context
 * @wdev: Wireless device structure
 * @status: Status of conditional channel switch
 * (0: Success, Non-zero: Failure)
 *
 * Sends the status of conditional channel switch to user space. This is named
 * conditional channel switch because the SAP will move to the provided channel
 * after some condition (pre-cac) is met.
 *
 * Return: None
 */
static void hdd_send_conditional_chan_switch_status(struct hdd_context *hdd_ctx,
						struct wireless_dev *wdev,
						bool status)
{
	struct sk_buff *event;

	ENTER_DEV(wdev->netdev);

	if (!hdd_ctx) {
		hdd_err("Invalid HDD context pointer");
		return;
	}

	event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
		  wdev, sizeof(uint32_t) + NLMSG_HDRLEN,
		  QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH_INDEX,
		  GFP_KERNEL);
	if (!event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(event,
			QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS,
			status)) {
		hdd_err("nla put failed");
		kfree_skb(event);
		return;
	}

	cfg80211_vendor_event(event, GFP_KERNEL);
}

/**
 * wlan_hdd_set_pre_cac_complete_status() - Set pre cac complete status
 * @ap_adapter: AP adapter
 * @status: Status which can be true or false
 *
 * Sets the status of pre cac i.e., whether it is complete or not
 *
 * Return: Zero on success, non-zero on failure
 */
static int wlan_hdd_set_pre_cac_complete_status(struct hdd_adapter *ap_adapter,
		bool status)
{
	QDF_STATUS ret;

	ret = wlan_sap_set_pre_cac_complete_status(
			WLAN_HDD_GET_SAP_CTX_PTR(ap_adapter), status);
	if (QDF_IS_STATUS_ERROR(ret))
		return -EINVAL;

	return 0;
}

/**
 * __wlan_hdd_sap_pre_cac_failure() - Process the pre cac failure
 * @data: AP adapter
 *
 * Deletes the pre cac adapter
 *
 * Return: None
 */
static void __wlan_hdd_sap_pre_cac_failure(void *data)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;

	ENTER();

	adapter = (struct hdd_adapter *) data;
	if (!adapter ||
	    adapter->magic != WLAN_HDD_ADAPTER_MAGIC) {
		hdd_err("SAP Pre CAC adapter invalid");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (wlan_hdd_validate_context(hdd_ctx)) {
		hdd_err("HDD context is null");
		return;
	}

	wlan_hdd_release_intf_addr(hdd_ctx,
				   adapter->mac_addr.bytes);
	hdd_stop_adapter(hdd_ctx, adapter);
	hdd_close_adapter(hdd_ctx, adapter, false);
}

/**
 * wlan_hdd_sap_pre_cac_failure() - Process the pre cac failure
 * @data: AP adapter
 *
 * Deletes the pre cac adapter
 *
 * Return: None
 */
void wlan_hdd_sap_pre_cac_failure(void *data)
{
	cds_ssr_protect(__func__);
	__wlan_hdd_sap_pre_cac_failure(data);
	cds_ssr_unprotect(__func__);
}

/**
 * wlan_hdd_sap_pre_cac_success() - Process the pre cac result
 * @data: AP adapter
 *
 * Deletes the pre cac adapter and moves the existing SAP to the pre cac
 * channel
 *
 * Return: None
 */
static void wlan_hdd_sap_pre_cac_success(void *data)
{
	struct hdd_adapter *adapter, *ap_adapter;
	int i;
	struct hdd_context *hdd_ctx;

	ENTER();

	adapter = (struct hdd_adapter *) data;
	if (!adapter) {
		hdd_err("AP adapter is NULL");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return;
	}

	cds_ssr_protect(__func__);
	wlan_hdd_release_intf_addr(hdd_ctx,
				   adapter->mac_addr.bytes);
	hdd_stop_adapter(hdd_ctx, adapter);
	hdd_close_adapter(hdd_ctx, adapter, false);
	cds_ssr_unprotect(__func__);

	/* Prepare to switch AP from 2.4GHz channel to the pre CAC channel */
	ap_adapter = hdd_get_adapter(hdd_ctx, QDF_SAP_MODE);
	if (!ap_adapter) {
		hdd_err("failed to get SAP adapter, no restart on pre CAC channel");
		return;
	}

	/*
	 * Setting of the pre cac complete status will ensure that on channel
	 * switch to the pre CAC DFS channel, there is no CAC again.
	 */
	wlan_hdd_set_pre_cac_complete_status(ap_adapter, true);
	i = hdd_softap_set_channel_change(ap_adapter->dev,
			ap_adapter->pre_cac_chan,
			CH_WIDTH_MAX, false);
	if (0 != i) {
		hdd_err("failed to change channel");
		wlan_hdd_set_pre_cac_complete_status(ap_adapter, false);
	}
}

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
/**
 * hdd_handle_acs_scan_event() - handle acs scan event for SAP
 * @sap_event: tpSap_Event
 * @adapter: struct hdd_adapter for SAP
 *
 * The function is to handle the eSAP_ACS_SCAN_SUCCESS_EVENT event.
 * It will update scan result to cfg80211 and start a timer to flush the
 * cached acs scan result.
 *
 * Return: QDF_STATUS_SUCCESS on success,
 *      other value on failure
 */
static QDF_STATUS hdd_handle_acs_scan_event(tpSap_Event sap_event,
		struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;
	struct sap_acs_scan_complete_event *comp_evt;
	QDF_STATUS qdf_status;
	int chan_list_size;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return QDF_STATUS_E_FAILURE;
	}
	comp_evt = &sap_event->sapevt.sap_acs_scan_comp;
	hdd_ctx->skip_acs_scan_status = eSAP_SKIP_ACS_SCAN;
	qdf_spin_lock(&hdd_ctx->acs_skip_lock);
	qdf_mem_free(hdd_ctx->last_acs_channel_list);
	hdd_ctx->last_acs_channel_list = NULL;
	hdd_ctx->num_of_channels = 0;
	/* cache the previous ACS scan channel list .
	 * If the following OBSS scan chan list is covered by ACS chan list,
	 * we can skip OBSS Scan to save SAP starting total time.
	 */
	if (comp_evt->num_of_channels && comp_evt->channellist) {
		chan_list_size = comp_evt->num_of_channels *
			sizeof(comp_evt->channellist[0]);
		hdd_ctx->last_acs_channel_list = qdf_mem_malloc(
			chan_list_size);
		if (hdd_ctx->last_acs_channel_list) {
			qdf_mem_copy(hdd_ctx->last_acs_channel_list,
				comp_evt->channellist,
				chan_list_size);
			hdd_ctx->num_of_channels = comp_evt->num_of_channels;
		}
	}
	qdf_spin_unlock(&hdd_ctx->acs_skip_lock);

	hdd_debug("Reusing Last ACS scan result for %d sec",
		ACS_SCAN_EXPIRY_TIMEOUT_S);
	qdf_mc_timer_stop(&hdd_ctx->skip_acs_scan_timer);
	qdf_status = qdf_mc_timer_start(&hdd_ctx->skip_acs_scan_timer,
			ACS_SCAN_EXPIRY_TIMEOUT_S * 1000);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		hdd_err("Failed to start ACS scan expiry timer");
	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS hdd_handle_acs_scan_event(tpSap_Event sap_event,
		struct hdd_adapter *adapter)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * get_max_rate_vht() - calculate max rate for VHT mode
 * @nss: num of streams
 * @ch_width: channel width
 * @sgi: short gi
 * @vht_mcs_map: vht mcs map
 *
 * This function calculate max rate for VHT mode
 *
 * Return: max rate
 */
static int get_max_rate_vht(int nss, int ch_width, int sgi, int vht_mcs_map)
{
	const struct index_vht_data_rate_type *supported_vht_mcs_rate;
	enum data_rate_11ac_max_mcs vht_max_mcs;
	int maxrate = 0;
	int maxidx;

	if (nss == 1) {
		supported_vht_mcs_rate = supported_vht_mcs_rate_nss1;
	} else if (nss == 2) {
		supported_vht_mcs_rate = supported_vht_mcs_rate_nss2;
	} else {
		/* Not Supported */
		hdd_err("nss %d not supported", nss);
		return maxrate;
	}

	vht_max_mcs =
		(enum data_rate_11ac_max_mcs)
		(vht_mcs_map & DATA_RATE_11AC_MCS_MASK);

	if (vht_max_mcs == DATA_RATE_11AC_MAX_MCS_7) {
		maxidx = 7;
	} else if (vht_max_mcs == DATA_RATE_11AC_MAX_MCS_8) {
		maxidx = 8;
	} else if (vht_max_mcs == DATA_RATE_11AC_MAX_MCS_9) {
		if (ch_width == eHT_CHANNEL_WIDTH_20MHZ)
			/* MCS9 is not valid for VHT20 when nss=1,2 */
			maxidx = 8;
		else
			maxidx = 9;
	} else {
		hdd_err("vht mcs map %x not supported",
			vht_mcs_map & DATA_RATE_11AC_MCS_MASK);
		return maxrate;
	}

	if (ch_width == eHT_CHANNEL_WIDTH_20MHZ) {
		maxrate =
		supported_vht_mcs_rate[maxidx].supported_VHT20_rate[sgi];
	} else if (ch_width == eHT_CHANNEL_WIDTH_40MHZ) {
		maxrate =
		supported_vht_mcs_rate[maxidx].supported_VHT40_rate[sgi];
	} else if (ch_width == eHT_CHANNEL_WIDTH_80MHZ) {
		maxrate =
		supported_vht_mcs_rate[maxidx].supported_VHT80_rate[sgi];
	} else {
		hdd_err("ch_width %d not supported", ch_width);
		return maxrate;
	}

	return maxrate;
}

/**
 * calculate_max_phy_rate() - calcuate maximum phy rate (100kbps)
 * @mode: phymode: Legacy, 11a/b/g, HT, VHT
 * @nss: num of stream (maximum num is 2)
 * @ch_width: channel width
 * @sgi: short gi enabled or not
 * @supp_idx: max supported idx
 * @ext_idx: max extended idx
 * @ht_mcs_idx: max mcs index for HT
 * @vht_mcs_map: mcs map for VHT
 *
 * return: maximum phy rate in 100kbps
 */
static int calcuate_max_phy_rate(int mode, int nss, int ch_width,
				 int sgi, int supp_idx, int ext_idx,
				 int ht_mcs_idx, int vht_mcs_map)
{
	const struct index_data_rate_type *supported_mcs_rate;
	int maxidx = 12; /*default 6M mode*/
	int maxrate = 0, tmprate;
	int i;

	/* check supported rates */
	if (supp_idx != 0xff && maxidx < supp_idx)
		maxidx = supp_idx;

	/* check extended rates */
	if (ext_idx != 0xff && maxidx < ext_idx)
		maxidx = ext_idx;

	for (i = 0; i < QDF_ARRAY_SIZE(supported_data_rate); i++) {
		if (supported_data_rate[i].beacon_rate_index == maxidx)
			maxrate = supported_data_rate[i].supported_rate[0];
	}

	if (mode == SIR_SME_PHY_MODE_HT) {
		/* check for HT Mode */
		maxidx = ht_mcs_idx;
		if (nss == 1) {
			supported_mcs_rate = supported_mcs_rate_nss1;
		} else if (nss == 2) {
			supported_mcs_rate = supported_mcs_rate_nss2;
		} else {
			/* Not Supported */
			hdd_err("nss %d not supported", nss);
			return maxrate;
		}

		if (ch_width == eHT_CHANNEL_WIDTH_20MHZ) {
			tmprate = sgi ?
				supported_mcs_rate[maxidx].supported_rate[2] :
				supported_mcs_rate[maxidx].supported_rate[0];
		} else if (ch_width == eHT_CHANNEL_WIDTH_40MHZ) {
			tmprate = sgi ?
				supported_mcs_rate[maxidx].supported_rate[3] :
				supported_mcs_rate[maxidx].supported_rate[1];
		} else {
			hdd_err("invalid mode %d ch_width %d",
				mode, ch_width);
			return maxrate;
		}

		if (maxrate < tmprate)
			maxrate = tmprate;
	}

	if (mode == SIR_SME_PHY_MODE_VHT) {
		/* check for VHT Mode */
		tmprate = get_max_rate_vht(nss, ch_width, sgi, vht_mcs_map);
		if (maxrate < tmprate)
			maxrate = tmprate;
	}

	return maxrate;
}

/**
 * hdd_convert_dot11mode_from_phymode() - get dot11 mode from phymode
 * @phymode: phymode of sta associated to SAP
 *
 * The function is to convert the phymode to corresponding dot11 mode
 *
 * Return: dot11mode.
 */


static int hdd_convert_dot11mode_from_phymode(int phymode)
{

	switch (phymode) {

	case MODE_11A:
		return QCA_WLAN_802_11_MODE_11A;

	case MODE_11B:
		return QCA_WLAN_802_11_MODE_11B;

	case MODE_11G:
	case MODE_11GONLY:
		return QCA_WLAN_802_11_MODE_11G;

	case MODE_11NA_HT20:
	case MODE_11NG_HT20:
	case MODE_11NA_HT40:
	case MODE_11NG_HT40:
		return QCA_WLAN_802_11_MODE_11N;

	case MODE_11AC_VHT20:
	case MODE_11AC_VHT40:
	case MODE_11AC_VHT80:
	case MODE_11AC_VHT20_2G:
	case MODE_11AC_VHT40_2G:
	case MODE_11AC_VHT80_2G:
#ifdef CONFIG_160MHZ_SUPPORT
	case MODE_11AC_VHT80_80:
	case MODE_11AC_VHT160:
#endif
		return QCA_WLAN_802_11_MODE_11AC;

	default:
		return QCA_WLAN_802_11_MODE_INVALID;
	}

}

/**
 * hdd_fill_station_info() - fill stainfo once connected
 * @stainfo: peer stainfo associate to SAP
 * @event: associate/reassociate event received
 *
 * The function is to update rate stats to stainfo
 *
 * Return: None.
 */
static void hdd_fill_station_info(struct hdd_adapter *adapter,
				  tSap_StationAssocReassocCompleteEvent *event)
{
	struct hdd_station_info *stainfo;
	uint8_t i = 0;

	if (event->staId >= WLAN_MAX_STA_COUNT) {
		hdd_err("invalid sta id");
		return;
	}

	stainfo = &adapter->sta_info[event->staId];

	if (!stainfo) {
		hdd_err("invalid stainfo");
		return;
	}

	stainfo->freq = cds_chan_to_freq(event->chan_info.chan_id);
	stainfo->sta_type = event->staType;
	stainfo->dot11_mode =
		hdd_convert_dot11mode_from_phymode(event->chan_info.info);

	stainfo->nss = event->chan_info.nss;
	stainfo->rate_flags = event->chan_info.rate_flags;
	stainfo->ampdu = event->ampdu;
	stainfo->sgi_enable = event->sgi_enable;
	stainfo->tx_stbc = event->tx_stbc;
	stainfo->rx_stbc = event->rx_stbc;
	stainfo->ch_width = event->ch_width;
	stainfo->mode = event->mode;
	stainfo->max_supp_idx = event->max_supp_idx;
	stainfo->max_ext_idx = event->max_ext_idx;
	stainfo->max_mcs_idx = event->max_mcs_idx;
	stainfo->rx_mcs_map = event->rx_mcs_map;
	stainfo->tx_mcs_map = event->tx_mcs_map;
	stainfo->assoc_ts = qdf_system_ticks();
	stainfo->max_phy_rate =
		calcuate_max_phy_rate(stainfo->mode,
				      stainfo->nss,
				      stainfo->ch_width,
				      stainfo->sgi_enable,
				      stainfo->max_supp_idx,
				      stainfo->max_ext_idx,
				      stainfo->max_mcs_idx,
				      stainfo->rx_mcs_map);
	/* expect max_phy_rate report in kbps */
	stainfo->max_phy_rate *= 100;

	if (event->vht_caps.present) {
		stainfo->vht_present = true;
		hdd_copy_vht_caps(&stainfo->vht_caps, &event->vht_caps);
	}
	if (event->ht_caps.present) {
		stainfo->ht_present = true;
		hdd_copy_ht_caps(&stainfo->ht_caps, &event->ht_caps);
	}

	while (i < WLAN_MAX_STA_COUNT) {
		if (!qdf_mem_cmp(adapter->cache_sta_info[i].sta_mac.bytes,
				 event->staMac.bytes,
				 QDF_MAC_ADDR_SIZE)) {
			qdf_mem_zero(&adapter->cache_sta_info[i],
				     sizeof(*stainfo));
			break;
		}
		i++;
	}
	if (i >= WLAN_MAX_STA_COUNT) {
		i = 0;
		while (i < WLAN_MAX_STA_COUNT) {
			if (adapter->cache_sta_info[i].in_use != TRUE)
				break;
			i++;
		}
	}
	if (i < WLAN_MAX_STA_COUNT)
		qdf_mem_copy(&adapter->cache_sta_info[i],
			     stainfo, sizeof(struct hdd_station_info));
	else
		hdd_debug("reached max staid, stainfo can't be cached");

	hdd_debug("cap %d %d %d %d %d %d %d %d %d %x %d",
		  stainfo->ampdu,
		  stainfo->sgi_enable,
		  stainfo->tx_stbc,
		  stainfo->rx_stbc,
		  stainfo->is_qos_enabled,
		  stainfo->ch_width,
		  stainfo->mode,
		  event->wmmEnabled,
		  event->chan_info.nss,
		  event->chan_info.rate_flags,
		  stainfo->max_phy_rate);
	hdd_debug("rate info %d %d %d %d %d",
		  stainfo->max_supp_idx,
		  stainfo->max_ext_idx,
		  stainfo->max_mcs_idx,
		  stainfo->rx_mcs_map,
		  stainfo->tx_mcs_map);
}

/**
 * hdd_stop_sap_due_to_invalid_channel() - to stop sap in case of invalid chnl
 * @work: pointer to work structure
 *
 * Let's say SAP detected RADAR and trying to select the new channel and if no
 * valid channel is found due to none of the channels are available or
 * regulatory restriction then SAP needs to be stopped. so SAP state-machine
 * will create a work to stop the bss
 *
 * stop bss has to happen through worker thread because radar indication comes
 * from FW through mc thread or main host thread and if same thread is used to
 * do stopbss then waiting for stopbss to finish operation will halt mc thread
 * to freeze which will trigger stopbss timeout. Instead worker thread can do
 * the stopbss operation while mc thread waits for stopbss to finish.
 *
 * Return: none
 */
static void
hdd_stop_sap_due_to_invalid_channel(struct work_struct *work)
{
	/*
	 * Extract the adapter from work structure. sap_stop_bss_work
	 * is part of adapter context.
	 */
	struct hdd_adapter *sap_adapter = container_of(work,
						  struct hdd_adapter,
						  sap_stop_bss_work);
	cds_ssr_protect(__func__);
	if (sap_adapter == NULL) {
	    cds_err("sap_adapter is NULL, no work needed");
	    cds_ssr_unprotect(__func__);
	    return;
	}
	hdd_debug("work started for sap session[%d]", sap_adapter->session_id);
	wlan_hdd_stop_sap(sap_adapter);
	wlansap_set_invalid_session(WLAN_HDD_GET_SAP_CTX_PTR(sap_adapter));
	wlansap_cleanup_cac_timer(WLAN_HDD_GET_SAP_CTX_PTR(sap_adapter));
	hdd_debug("work finished for sap");
	cds_ssr_unprotect(__func__);
}

QDF_STATUS hdd_hostapd_sap_event_cb(tpSap_Event pSapEvent,
				    void *usrDataForCallback)
{
	struct hdd_adapter *adapter;
	struct hdd_ap_ctx *ap_ctx;
	struct hdd_hostapd_state *hostapd_state;
	struct net_device *dev;
	eSapHddEvent sapEvent;
	union iwreq_data wrqu;
	uint8_t *we_custom_event_generic = NULL;
	int we_event = 0;
	int i = 0;
	uint8_t staId;
	QDF_STATUS qdf_status;
	bool bAuthRequired = true;
	tpSap_AssocMacAddr pAssocStasArray = NULL;
	char unknownSTAEvent[IW_CUSTOM_MAX + 1];
	char maxAssocExceededEvent[IW_CUSTOM_MAX + 1];
	uint8_t we_custom_start_event[64];
	char *startBssEvent;
	struct hdd_context *hdd_ctx;
	struct iw_michaelmicfailure msg;
	uint8_t ignoreCAC = 0;
	struct hdd_config *cfg = NULL;
	struct wlan_dfs_info dfs_info;
	uint8_t cc_len = WLAN_SVC_COUNTRY_CODE_LEN;
	struct hdd_adapter *con_sap_adapter;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hdd_chan_change_params chan_change;
	tSap_StationAssocReassocCompleteEvent *event;
	int ret = 0;
	struct ch_params sap_ch_param = {0};
	eCsrPhyMode phy_mode;
	bool legacy_phymode;
	tSap_StationDisassocCompleteEvent *disassoc_comp;
	struct hdd_station_info *stainfo;

	dev = (struct net_device *)usrDataForCallback;
	if (!dev) {
		hdd_err("usrDataForCallback is null");
		return QDF_STATUS_E_FAILURE;
	}

	adapter = netdev_priv(dev);

	if ((NULL == adapter) ||
	    (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("invalid adapter or adapter has invalid magic");
		return QDF_STATUS_E_FAILURE;
	}

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
	ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);

	if (!pSapEvent) {
		hdd_err("pSapEvent is null");
		return QDF_STATUS_E_FAILURE;
	}

	sapEvent = pSapEvent->sapHddEventCode;
	memset(&wrqu, '\0', sizeof(wrqu));
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (!hdd_ctx) {
		hdd_err("HDD context is null");
		return QDF_STATUS_E_FAILURE;
	}

	cfg = hdd_ctx->config;

	if (!cfg) {
		hdd_err("HDD config is null");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_info.channel = ap_ctx->operating_channel;
	sme_get_country_code(hdd_ctx->hHal, dfs_info.country_code, &cc_len);
	staId = pSapEvent->sapevt.sapStartBssCompleteEvent.staId;

	switch (sapEvent) {
	case eSAP_START_BSS_EVENT:
		hdd_debug("BSS status = %s, channel = %u, bc sta Id = %d",
		       pSapEvent->sapevt.sapStartBssCompleteEvent.
		       status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS",
		       pSapEvent->sapevt.sapStartBssCompleteEvent.
		       operatingChannel,
		       pSapEvent->sapevt.sapStartBssCompleteEvent.staId);

		adapter->session_id =
			pSapEvent->sapevt.sapStartBssCompleteEvent.sessionId;

		hostapd_state->qdf_status =
			pSapEvent->sapevt.sapStartBssCompleteEvent.status;

		qdf_atomic_set(&adapter->dfs_radar_found, 0);
		wlansap_get_dfs_ignore_cac(hdd_ctx->hHal, &ignoreCAC);

		/* DFS requirement: DO NOT transmit during CAC. */
		if ((CHANNEL_STATE_DFS !=
			wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
				ap_ctx->operating_channel))
			|| ignoreCAC
			|| hdd_ctx->dev_dfs_cac_status == DFS_CAC_ALREADY_DONE)
			ap_ctx->dfs_cac_block_tx = false;
		else
			ap_ctx->dfs_cac_block_tx = true;

		hdd_debug("The value of dfs_cac_block_tx[%d] for ApCtx[%pK]:%d",
				ap_ctx->dfs_cac_block_tx, ap_ctx,
				adapter->session_id);

		if (hostapd_state->qdf_status) {
			hdd_err("startbss event failed!!");
			/*
			 * Make sure to set the event before proceeding
			 * for error handling otherwise caller thread will
			 * wait till 10 secs and no other connection will
			 * go through before that.
			 */
			hostapd_state->bss_state = BSS_STOP;
			qdf_event_set(&hostapd_state->qdf_event);
			goto stopbss;
		} else {
			sme_ch_avoid_update_req(hdd_ctx->hHal);

			ap_ctx->broadcast_sta_id =
				pSapEvent->sapevt.sapStartBssCompleteEvent.staId;

			hdd_register_tx_flow_control(adapter,
				hdd_softap_tx_resume_timer_expired_handler,
				hdd_softap_tx_resume_cb,
				hdd_tx_flow_control_is_pause);

			/* @@@ need wep logic here to set privacy bit */
			qdf_status =
				hdd_softap_register_bc_sta(adapter,
							   ap_ctx->privacy);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				hdd_warn("Failed to register BC STA %d",
				       qdf_status);
				hdd_stop_bss_link(adapter,
						  usrDataForCallback);
			}
		}

		if (hdd_ipa_is_enabled(hdd_ctx)) {
			status = hdd_ipa_wlan_evt(adapter,
					ap_ctx->broadcast_sta_id,
					HDD_IPA_AP_CONNECT,
					adapter->dev->dev_addr);
			if (status) {
				hdd_err("WLAN_AP_CONNECT event failed!!");
				/*
				 * Make sure to set the event before proceeding
				 * for error handling otherwise caller thread
				 * will wait till 10 secs and no other
				 * connection will go through before that.
				 */
				qdf_event_set(&hostapd_state->qdf_event);
				goto stopbss;
			}
		}

		if (0 !=
		    (WLAN_HDD_GET_CTX(adapter))->config->
		     nAPAutoShutOff) {
			/* AP Inactivity timer init and start */
			qdf_status =
				qdf_mc_timer_init(&ap_ctx->
						  hdd_ap_inactivity_timer,
						  QDF_TIMER_TYPE_SW,
						  hdd_hostapd_inactivity_timer_cb,
						  dev);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_err("Failed to init inactivity timer");

			qdf_status =
				qdf_mc_timer_start(&ap_ctx->
						   hdd_ap_inactivity_timer,
						   (WLAN_HDD_GET_CTX
						    (adapter))->config->
						    nAPAutoShutOff * 1000);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_err("Failed to init inactivity timer");

		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(hdd_ctx, true);
#endif
		ap_ctx->operating_channel =
			pSapEvent->sapevt.sapStartBssCompleteEvent.operatingChannel;

		hdd_hostapd_channel_prevent_suspend(adapter,
						    ap_ctx->operating_channel);

		hostapd_state->bss_state = BSS_START;

		/* Set default key index */
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			"%s: default key index %hu", __func__,
			ap_ctx->wep_def_key_idx);

		sme_roam_set_default_key_index(
			WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->session_id,
			ap_ctx->wep_def_key_idx);

		/* Set group key / WEP key every time when BSS is restarted */
		if (ap_ctx->group_key.keyLength) {
			status = wlansap_set_key_sta(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				&ap_ctx->group_key);
			if (!QDF_IS_STATUS_SUCCESS(status))
				hdd_err("wlansap_set_key_sta failed");
		} else {
			for (i = 0; i < CSR_MAX_NUM_KEY; i++) {
				if (!ap_ctx->wep_key[i].keyLength)
					continue;

				status = wlansap_set_key_sta(
					WLAN_HDD_GET_SAP_CTX_PTR
						(adapter),
					&ap_ctx->wep_key[i]);
				if (!QDF_IS_STATUS_SUCCESS(status))
					hdd_err("set_key failed idx: %d", i);
			}
		}

		if ((CHANNEL_STATE_DFS == wlan_reg_get_channel_state(
						hdd_ctx->hdd_pdev,
						ap_ctx->operating_channel))
		    && (hdd_ctx->config->IsSapDfsChSifsBurstEnabled == 0)) {

			hdd_debug("Set SIFS Burst disable for DFS channel %d",
			       ap_ctx->operating_channel);

			if (wma_cli_set_command(adapter->session_id,
						WMI_PDEV_PARAM_BURST_ENABLE,
						0, PDEV_CMD)) {
				hdd_err("Failed to Set SIFS Burst channel: %d",
				       ap_ctx->operating_channel);
			}
		}
		/* Fill the params for sending IWEVCUSTOM Event
		 * with SOFTAP.enabled
		 */
		startBssEvent = "SOFTAP.enabled";
		memset(&we_custom_start_event, '\0',
		       sizeof(we_custom_start_event));
		memcpy(&we_custom_start_event, startBssEvent,
		       strlen(startBssEvent));
		memset(&wrqu, 0, sizeof(wrqu));
		wrqu.data.length = strlen(startBssEvent);
		we_event = IWEVCUSTOM;
		we_custom_event_generic = we_custom_start_event;
		hdd_ipa_set_tx_flow_info();
		/* Send SCC/MCC Switching event to IPA */
		hdd_ipa_send_mcc_scc_msg(hdd_ctx, hdd_ctx->mcc_mode);

		if (policy_mgr_is_hw_mode_change_after_vdev_up(
			hdd_ctx->hdd_psoc)) {
			hdd_debug("check for possible hw mode change");
			status = policy_mgr_set_hw_mode_on_channel_switch(
				hdd_ctx->hdd_psoc, adapter->session_id);
			if (QDF_IS_STATUS_ERROR(status))
				hdd_debug("set hw mode change not done");
			policy_mgr_set_do_hw_mode_change_flag(
					hdd_ctx->hdd_psoc, false);
		}
		/*
		 * set this event at the very end because once this events
		 * get set, caller thread is waiting to do further processing.
		 * so once this event gets set, current worker thread might get
		 * pre-empted by caller thread.
		 */
		qdf_status = qdf_event_set(&hostapd_state->qdf_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("qdf_event_set failed! status: %d", qdf_status);
			goto stopbss;
		}
		break;          /* Event will be sent after Switch-Case stmt */

	case eSAP_STOP_BSS_EVENT:
		hdd_debug("BSS stop status = %s",
		       pSapEvent->sapevt.sapStopBssCompleteEvent.
		       status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");

		hdd_hostapd_channel_allow_suspend(adapter,
						  ap_ctx->operating_channel);

		/* Invalidate the channel info. */
		ap_ctx->operating_channel = 0;
		if (hdd_ipa_is_enabled(hdd_ctx)) {
			status = hdd_ipa_wlan_evt(adapter,
					ap_ctx->broadcast_sta_id,
					HDD_IPA_AP_DISCONNECT,
					adapter->dev->dev_addr);
			if (status) {
				hdd_err("WLAN_AP_DISCONNECT event failed!!");
				goto stopbss;
			}
		}

		/* reset the dfs_cac_status and dfs_cac_block_tx flag only when
		 * the last BSS is stopped
		 */
		con_sap_adapter = hdd_get_con_sap_adapter(adapter, true);
		if (!con_sap_adapter) {
			ap_ctx->dfs_cac_block_tx = true;
			hdd_ctx->dev_dfs_cac_status = DFS_CAC_NEVER_DONE;
		}
		hdd_debug("bss_stop_reason=%d", ap_ctx->bss_stop_reason);
		if ((BSS_STOP_DUE_TO_MCC_SCC_SWITCH !=
			ap_ctx->bss_stop_reason) &&
		    (BSS_STOP_DUE_TO_VENDOR_CONFIG_CHAN !=
			ap_ctx->bss_stop_reason)) {
			/*
			 * when MCC to SCC switching or vendor subcmd
			 * setting sap config channel happens, key storage
			 * should not be cleared due to hostapd will not
			 * repopulate the original keys
			 */
			ap_ctx->group_key.keyLength = 0;
			for (i = 0; i < CSR_MAX_NUM_KEY; i++)
				ap_ctx->wep_key[i].keyLength = 0;
		}

		/* clear the reason code in case BSS is stopped
		 * in another place
		 */
		ap_ctx->bss_stop_reason = BSS_STOP_REASON_INVALID;
		goto stopbss;

	case eSAP_DFS_CAC_START:
		wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
					WLAN_SVC_DFS_CAC_START_IND,
					    &dfs_info,
					    sizeof(struct wlan_dfs_info));
		hdd_ctx->dev_dfs_cac_status = DFS_CAC_IN_PROGRESS;
		if (QDF_STATUS_SUCCESS !=
			hdd_send_radar_event(hdd_ctx, eSAP_DFS_CAC_START,
				dfs_info, &adapter->wdev)) {
			hdd_err("Unable to indicate CAC start NL event");
		} else {
			hdd_debug("Sent CAC start to user space");
		}

		qdf_atomic_set(&adapter->dfs_radar_found, 0);
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
		if (QDF_STATUS_SUCCESS !=
			hdd_send_radar_event(hdd_ctx, eSAP_DFS_CAC_END,
				dfs_info, &adapter->wdev)) {
			hdd_err("Unable to indicate CAC end (interrupted) event");
		} else {
			hdd_debug("Sent CAC end (interrupted) to user space");
		}
		break;
	case eSAP_DFS_CAC_END:
		wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
					WLAN_SVC_DFS_CAC_END_IND,
					    &dfs_info,
					    sizeof(struct wlan_dfs_info));
		ap_ctx->dfs_cac_block_tx = false;
		hdd_ctx->dev_dfs_cac_status = DFS_CAC_ALREADY_DONE;
		if (QDF_STATUS_SUCCESS !=
			hdd_send_radar_event(hdd_ctx, eSAP_DFS_CAC_END,
				dfs_info, &adapter->wdev)) {
			hdd_err("Unable to indicate CAC end NL event");
		} else {
			hdd_debug("Sent CAC end to user space");
		}
		break;
	case eSAP_DFS_RADAR_DETECT:
	{
		int i;
		tsap_Config_t *sap_config =
				&adapter->session.ap.sap_config;

		hdd_dfs_indicate_radar(hdd_ctx);
		wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
					WLAN_SVC_DFS_RADAR_DETECT_IND,
					    &dfs_info,
					    sizeof(struct wlan_dfs_info));
		hdd_ctx->dev_dfs_cac_status = DFS_CAC_NEVER_DONE;
		for (i = 0; i < sap_config->channel_info_count; i++) {
			if (sap_config->channel_info[i].ieee_chan_number
							== dfs_info.channel)
				sap_config->channel_info[i].flags |=
					IEEE80211_CHAN_RADAR_DFS;
		}
		if (QDF_STATUS_SUCCESS !=
			hdd_send_radar_event(hdd_ctx, eSAP_DFS_RADAR_DETECT,
				dfs_info, &adapter->wdev)) {
			hdd_err("Unable to indicate Radar detect NL event");
		} else {
			hdd_debug("Sent radar detected to user space");
		}
		break;
	}
	case eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC:
		hdd_debug("notification for radar detect during pre cac:%d",
			adapter->session_id);
		hdd_send_conditional_chan_switch_status(hdd_ctx,
			&adapter->wdev, false);
		hdd_ctx->dev_dfs_cac_status = DFS_CAC_NEVER_DONE;
		qdf_create_work(0, &hdd_ctx->sap_pre_cac_work,
				wlan_hdd_sap_pre_cac_failure,
				(void *)adapter);
		qdf_sched_work(0, &hdd_ctx->sap_pre_cac_work);
		break;
	case eSAP_DFS_PRE_CAC_END:
		hdd_debug("pre cac end notification received:%d",
			adapter->session_id);
		hdd_send_conditional_chan_switch_status(hdd_ctx,
			&adapter->wdev, true);
		ap_ctx->dfs_cac_block_tx = false;
		hdd_ctx->dev_dfs_cac_status = DFS_CAC_ALREADY_DONE;

		qdf_create_work(0, &hdd_ctx->sap_pre_cac_work,
				wlan_hdd_sap_pre_cac_success,
				(void *)adapter);
		qdf_sched_work(0, &hdd_ctx->sap_pre_cac_work);
		break;
	case eSAP_DFS_NO_AVAILABLE_CHANNEL:
		wlan_hdd_send_svc_nlink_msg
			(hdd_ctx->radio_index,
			WLAN_SVC_DFS_ALL_CHANNEL_UNAVAIL_IND, &dfs_info,
			sizeof(struct wlan_dfs_info));
		break;

	case eSAP_STA_SET_KEY_EVENT:
		/* TODO:
		 * forward the message to hostapd once implementation
		 * is done for now just print
		 */
		hdd_debug("SET Key: configured status = %s",
		       pSapEvent->sapevt.sapStationSetKeyCompleteEvent.
		       status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");
		return QDF_STATUS_SUCCESS;
	case eSAP_STA_MIC_FAILURE_EVENT:
	{
		memset(&msg, '\0', sizeof(msg));
		msg.src_addr.sa_family = ARPHRD_ETHER;
		memcpy(msg.src_addr.sa_data,
		       &pSapEvent->sapevt.sapStationMICFailureEvent.
		       staMac, QDF_MAC_ADDR_SIZE);
		hdd_debug("MIC MAC " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(msg.src_addr.sa_data));
		if (pSapEvent->sapevt.sapStationMICFailureEvent.
		    multicast == true)
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
					       true) ?
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
		event = &pSapEvent->sapevt.sapStationAssocReassocCompleteEvent;
		if (eSAP_STATUS_FAILURE == event->status) {
			hdd_notice("assoc failure: " MAC_ADDRESS_STR,
				   MAC_ADDR_ARRAY(wrqu.addr.sa_data));
			break;
		}

		wrqu.addr.sa_family = ARPHRD_ETHER;
		memcpy(wrqu.addr.sa_data,
		       &event->staMac, QDF_MAC_ADDR_SIZE);
		hdd_notice(" associated " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(wrqu.addr.sa_data));
		we_event = IWEVREGISTERED;

		if ((eCSR_ENCRYPT_TYPE_NONE == ap_ctx->encryption_type) ||
		    (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY ==
		     ap_ctx->encryption_type)
		    || (eCSR_ENCRYPT_TYPE_WEP104_STATICKEY ==
			ap_ctx->encryption_type)) {
			bAuthRequired = false;
		}

		if (bAuthRequired) {
			qdf_status = hdd_softap_register_sta(
						adapter,
						true,
						ap_ctx->privacy,
						event->staId, 0, 0,
						(struct qdf_mac_addr *)
						wrqu.addr.sa_data,
						event->wmmEnabled);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_err("Failed to register STA %d "
					  MAC_ADDRESS_STR "", qdf_status,
				       MAC_ADDR_ARRAY(wrqu.addr.sa_data));
		} else {
			qdf_status = hdd_softap_register_sta(
						adapter,
						false,
						ap_ctx->privacy,
						event->staId, 0, 0,
						(struct qdf_mac_addr *)
						wrqu.addr.sa_data,
						event->wmmEnabled);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_err("Failed to register STA %d "
					  MAC_ADDRESS_STR "", qdf_status,
				       MAC_ADDR_ARRAY(wrqu.addr.sa_data));
		}

		staId = event->staId;
		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			hdd_fill_station_info(adapter, event);

		adapter->sta_info[staId].ecsa_capable = event->ecsa_capable;

		if (hdd_ipa_is_enabled(hdd_ctx)) {
			status = hdd_ipa_wlan_evt(adapter,
					event->staId, HDD_IPA_CLIENT_CONNECT_EX,
					event->staMac.bytes);
			if (status) {
				hdd_err("WLAN_CLIENT_CONNECT_EX event failed");
				goto stopbss;
			}
		}

		DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
			adapter->session_id,
			QDF_TRACE_DEFAULT_PDEV_ID,
			QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_ASSOC));

#ifdef MSM_PLATFORM
		/* start timer in sap/p2p_go */
		if (ap_ctx->ap_active == false) {
			spin_lock_bh(&hdd_ctx->bus_bw_lock);
			adapter->prev_tx_packets =
				adapter->stats.tx_packets;
			adapter->prev_rx_packets =
				adapter->stats.rx_packets;

			cdp_get_intra_bss_fwd_pkts_count(
				cds_get_context(QDF_MODULE_ID_SOC),
				adapter->session_id,
				&adapter->prev_fwd_tx_packets,
				&adapter->prev_fwd_rx_packets);

			spin_unlock_bh(&hdd_ctx->bus_bw_lock);
			hdd_bus_bw_compute_timer_start(hdd_ctx);
		}
#endif
		ap_ctx->ap_active = true;
		/* Stop AP inactivity timer */
		if (ap_ctx->hdd_ap_inactivity_timer.state ==
		    QDF_TIMER_STATE_RUNNING) {
			qdf_status =
				qdf_mc_timer_stop(&ap_ctx->
						  hdd_ap_inactivity_timer);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_err("Failed to start inactivity timer");
		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(hdd_ctx, false);
#endif
		cds_host_diag_log_work(&hdd_ctx->sap_wake_lock,
				       HDD_SAP_WAKE_LOCK_DURATION,
				       WIFI_POWER_EVENT_WAKELOCK_SAP);
		qdf_wake_lock_timeout_acquire(&hdd_ctx->sap_wake_lock,
					      HDD_SAP_WAKE_LOCK_DURATION);
		{
			struct station_info *sta_info;
			uint16_t iesLen = event->iesLen;

			sta_info = qdf_mem_malloc(sizeof(*sta_info));
			if (!sta_info) {
				hdd_err("Failed to allocate station info");
				return QDF_STATUS_E_FAILURE;
			}
			if (iesLen <= MAX_ASSOC_IND_IE_LEN) {
				sta_info->assoc_req_ies =
					(const u8 *)&event->ies[0];
				sta_info->assoc_req_ies_len = iesLen;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && !defined(WITH_BACKPORTS)
				/*
				 * After Kernel 4.0, it's no longer need to set
				 * STATION_INFO_ASSOC_REQ_IES flag, as it
				 * changed to use assoc_req_ies_len length to
				 * check the existance of request IE.
				 */
				sta_info->filled |= STATION_INFO_ASSOC_REQ_IES;
#endif
				cfg80211_new_sta(dev,
					(const u8 *)&event->staMac.bytes[0],
					sta_info, GFP_KERNEL);
			} else {
				hdd_err("Assoc Ie length is too long");
			}
			qdf_mem_free(sta_info);
		}
		/* Lets abort scan to ensure smooth authentication for client */
		if (ucfg_scan_get_vdev_status(adapter->hdd_vdev) !=
				SCAN_NOT_IN_PROGRESS) {
			wlan_abort_scan(hdd_ctx->hdd_pdev, INVAL_PDEV_ID,
				adapter->session_id, INVALID_SCAN_ID, false);
		}
		if (adapter->device_mode == QDF_P2P_GO_MODE) {
			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_app(
				&event->staMac,
				ePeerConnected,
				event->timingMeasCap,
				adapter->session_id,
				&event->chan_info,
				adapter->device_mode);
		}

		ret = hdd_objmgr_add_peer_object(
			adapter->hdd_vdev,
			adapter->device_mode,
			event->staMac.bytes,
			(adapter->sta_info[event->staId].sta_type
							== eSTA_TYPE_P2P_CLI));
		if (ret)
			hdd_err("Peer object "MAC_ADDRESS_STR" add fails!",
					MAC_ADDR_ARRAY(event->staMac.bytes));

		wlan_green_ap_add_sta(hdd_ctx->hdd_pdev);
		break;

	case eSAP_STA_DISASSOC_EVENT:
		disassoc_comp =
			&pSapEvent->sapevt.sapStationDisassocCompleteEvent;
		memcpy(wrqu.addr.sa_data,
		       &disassoc_comp->staMac, QDF_MAC_ADDR_SIZE);
		hdd_notice(" disassociated " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(wrqu.addr.sa_data));

		stainfo = hdd_get_stainfo(adapter->cache_sta_info,
					  disassoc_comp->staMac);
		if (stainfo) {
			stainfo->rssi = disassoc_comp->rssi;
			stainfo->tx_rate = disassoc_comp->tx_rate;
			stainfo->rx_rate = disassoc_comp->rx_rate;
			stainfo->reason_code = disassoc_comp->reason_code;
		}

		qdf_status = qdf_event_set(&hostapd_state->qdf_sta_disassoc_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			hdd_err("Station Deauth event Set failed");

		if (pSapEvent->sapevt.sapStationDisassocCompleteEvent.reason ==
		    eSAP_USR_INITATED_DISASSOC)
			hdd_debug(" User initiated disassociation");
		else
			hdd_debug(" MAC initiated disassociation");
		we_event = IWEVEXPIRED;
		qdf_status =
			hdd_softap_get_sta_id(adapter,
					      &pSapEvent->sapevt.
					      sapStationDisassocCompleteEvent.staMac,
					      &staId);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("Failed to find sta id status: %d", qdf_status);
			return QDF_STATUS_E_FAILURE;
		}

		DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
			adapter->session_id,
			QDF_TRACE_DEFAULT_PDEV_ID,
			QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_DISASSOC));

		hdd_softap_deregister_sta(adapter, staId);

		ap_ctx->ap_active = false;
		spin_lock_bh(&adapter->sta_info_lock);
		for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
			if (adapter->sta_info[i].in_use
			    && i !=
			    (WLAN_HDD_GET_AP_CTX_PTR(adapter))->
			    broadcast_sta_id) {
				ap_ctx->ap_active = true;
				break;
			}
		}
		spin_unlock_bh(&adapter->sta_info_lock);

		/* Start AP inactivity timer if no stations associated */
		if ((0 !=
		     (WLAN_HDD_GET_CTX(adapter))->config->
		     nAPAutoShutOff)) {
			if (ap_ctx->ap_active == false) {
				if (ap_ctx->hdd_ap_inactivity_timer.state ==
				    QDF_TIMER_STATE_STOPPED) {
					qdf_status =
						qdf_mc_timer_start(&ap_ctx->
								   hdd_ap_inactivity_timer,
								   (WLAN_HDD_GET_CTX
								    (adapter))->
								   config->
								   nAPAutoShutOff *
								   1000);
					if (!QDF_IS_STATUS_SUCCESS(qdf_status))
						hdd_err("Failed to init AP inactivity timer");
				} else
					QDF_ASSERT
						(qdf_mc_timer_get_current_state
							(&ap_ctx->
							hdd_ap_inactivity_timer) ==
						QDF_TIMER_STATE_STOPPED);
			}
		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(hdd_ctx, true);
#endif

		cds_host_diag_log_work(&hdd_ctx->sap_wake_lock,
				       HDD_SAP_WAKE_LOCK_DURATION,
				       WIFI_POWER_EVENT_WAKELOCK_SAP);
		qdf_wake_lock_timeout_acquire(&hdd_ctx->sap_wake_lock,
			 HDD_SAP_CLIENT_DISCONNECT_WAKE_LOCK_DURATION);
		cfg80211_del_sta(dev,
				 (const u8 *)&pSapEvent->sapevt.
				 sapStationDisassocCompleteEvent.staMac.
				 bytes[0], GFP_KERNEL);

		/* Update the beacon Interval if it is P2P GO */
		qdf_status = policy_mgr_change_mcc_go_beacon_interval(
			hdd_ctx->hdd_psoc, adapter->session_id,
			adapter->device_mode);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			hdd_err("Failed to update Beacon interval status: %d",
				qdf_status);
		}
		if (adapter->device_mode == QDF_P2P_GO_MODE) {
			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_app(&pSapEvent->sapevt.
						sapStationDisassocCompleteEvent.
						staMac, ePeerDisconnected,
						0,
						adapter->session_id,
						NULL,
						adapter->device_mode);
		}
#ifdef MSM_PLATFORM
		/*stop timer in sap/p2p_go */
		if (ap_ctx->ap_active == false) {
			spin_lock_bh(&hdd_ctx->bus_bw_lock);
			adapter->prev_tx_packets = 0;
			adapter->prev_rx_packets = 0;
			adapter->prev_fwd_tx_packets = 0;
			adapter->prev_fwd_rx_packets = 0;
			spin_unlock_bh(&hdd_ctx->bus_bw_lock);
			hdd_bus_bw_compute_timer_try_stop(hdd_ctx);
		}
#endif
		wlan_green_ap_del_sta(hdd_ctx->hdd_pdev);
		break;

	case eSAP_WPS_PBC_PROBE_REQ_EVENT:
		hdd_debug("WPS PBC probe req");
		return QDF_STATUS_SUCCESS;

	case eSAP_ASSOC_STA_CALLBACK_EVENT:
		pAssocStasArray =
			pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas;
		if (pSapEvent->sapevt.sapAssocStaListEvent.noOfAssocSta != 0) {
			for (i = 0;
			     i <
			     pSapEvent->sapevt.sapAssocStaListEvent.
			     noOfAssocSta; i++) {
				hdd_notice("Associated Sta Num %d:assocId=%d, staId=%d, staMac="
				       MAC_ADDRESS_STR, i + 1,
				       pAssocStasArray->assocId,
				       pAssocStasArray->staId,
				       MAC_ADDR_ARRAY(pAssocStasArray->staMac.
						      bytes));
				pAssocStasArray++;
			}
		}
		qdf_mem_free(pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas);
		pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas = NULL;
		return QDF_STATUS_SUCCESS;
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
		hdd_err("%s", unknownSTAEvent);
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
		hdd_debug("%s", maxAssocExceededEvent);
		break;
	case eSAP_STA_ASSOC_IND:
		return QDF_STATUS_SUCCESS;

	case eSAP_DISCONNECT_ALL_P2P_CLIENT:
		hdd_clear_all_sta(adapter);
		return QDF_STATUS_SUCCESS;

	case eSAP_MAC_TRIG_STOP_BSS_EVENT:
		qdf_status =
			hdd_stop_bss_link(adapter, usrDataForCallback);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_warn("hdd_stop_bss_link failed %d",
			       qdf_status);
		}
		return QDF_STATUS_SUCCESS;

	case eSAP_CHANNEL_CHANGE_EVENT:
		hdd_debug("Received eSAP_CHANNEL_CHANGE_EVENT event");
		if (hostapd_state->bss_state != BSS_STOP) {
			/* Prevent suspend for new channel */
			hdd_hostapd_channel_prevent_suspend(adapter,
				pSapEvent->sapevt.sap_ch_selected.pri_ch);
			/* Allow suspend for old channel */
			hdd_hostapd_channel_allow_suspend(adapter,
				ap_ctx->operating_channel);
		}
		/* SME/PE is already updated for new operation
		 * channel. So update HDD layer also here. This
		 * resolves issue in AP-AP mode where AP1 channel is
		 * changed due to RADAR then CAC is going on and
		 * START_BSS on new channel has not come to HDD. At
		 * this case if AP2 is started it needs current
		 * operation channel for MCC DFS restriction
		 */
		ap_ctx->operating_channel =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		ap_ctx->sap_config.acs_cfg.pri_ch =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		ap_ctx->sap_config.acs_cfg.ht_sec_ch =
			pSapEvent->sapevt.sap_ch_selected.ht_sec_ch;
		ap_ctx->sap_config.acs_cfg.vht_seg0_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg0_center_ch;
		ap_ctx->sap_config.acs_cfg.vht_seg1_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg1_center_ch;
		ap_ctx->sap_config.acs_cfg.ch_width =
			pSapEvent->sapevt.sap_ch_selected.ch_width;

		/* Indicate operating channel change to hostapd
		 * only for non driver override acs
		 */
		if (adapter->device_mode == QDF_SAP_MODE &&
		    hdd_ctx->config->force_sap_acs)
			return QDF_STATUS_SUCCESS;
		sap_ch_param.ch_width =
			pSapEvent->sapevt.sap_ch_selected.ch_width;
		sap_ch_param.center_freq_seg0 =
			pSapEvent->sapevt.sap_ch_selected.vht_seg0_center_ch;
		sap_ch_param.center_freq_seg1 =
			pSapEvent->sapevt.sap_ch_selected.vht_seg1_center_ch;
		wlan_reg_set_channel_params(hdd_ctx->hdd_pdev,
			pSapEvent->sapevt.sap_ch_selected.pri_ch,
			pSapEvent->sapevt.sap_ch_selected.ht_sec_ch,
			&sap_ch_param);

		phy_mode = wlan_sap_get_phymode(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter));

		switch (phy_mode) {
		case eCSR_DOT11_MODE_11n:
		case eCSR_DOT11_MODE_11n_ONLY:
		case eCSR_DOT11_MODE_11ac:
		case eCSR_DOT11_MODE_11ac_ONLY:
			legacy_phymode = false;
			break;
		default:
			legacy_phymode = true;
			break;
		}

		chan_change.chan =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		chan_change.chan_params.ch_width =
			pSapEvent->sapevt.sap_ch_selected.ch_width;
		chan_change.chan_params.sec_ch_offset =
			sap_ch_param.sec_ch_offset;
		chan_change.chan_params.center_freq_seg0 =
			pSapEvent->sapevt.sap_ch_selected.vht_seg0_center_ch;
		chan_change.chan_params.center_freq_seg1 =
			pSapEvent->sapevt.sap_ch_selected.vht_seg1_center_ch;

		return hdd_chan_change_notify(adapter, dev,
					      chan_change, legacy_phymode);
	case eSAP_ACS_SCAN_SUCCESS_EVENT:
		return hdd_handle_acs_scan_event(pSapEvent, adapter);

	case eSAP_ACS_CHANNEL_SELECTED:
		hdd_debug("ACS Completed for wlan%d",
					adapter->dev->ifindex);
		clear_bit(ACS_PENDING, &adapter->event_flags);
		clear_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags);
		ap_ctx->sap_config.acs_cfg.pri_ch =
			pSapEvent->sapevt.sap_ch_selected.pri_ch;
		ap_ctx->sap_config.acs_cfg.ht_sec_ch =
			pSapEvent->sapevt.sap_ch_selected.ht_sec_ch;
		ap_ctx->sap_config.acs_cfg.vht_seg0_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg0_center_ch;
		ap_ctx->sap_config.acs_cfg.vht_seg1_center_ch =
			pSapEvent->sapevt.sap_ch_selected.vht_seg1_center_ch;
		ap_ctx->sap_config.acs_cfg.ch_width =
			pSapEvent->sapevt.sap_ch_selected.ch_width;
		/* send vendor event to hostapd only for hostapd based acs*/
		if (!hdd_ctx->config->force_sap_acs)
			wlan_hdd_cfg80211_acs_ch_select_evt(adapter);
		qdf_atomic_set(&hdd_ctx->is_acs_allowed, 0);
		return QDF_STATUS_SUCCESS;
	case eSAP_ECSA_CHANGE_CHAN_IND:
		hdd_debug("Channel change indication from peer for channel %d",
				pSapEvent->sapevt.sap_chan_cng_ind.new_chan);
		if (hdd_softap_set_channel_change(dev,
			 pSapEvent->sapevt.sap_chan_cng_ind.new_chan,
			 CH_WIDTH_MAX, false))
			return QDF_STATUS_E_FAILURE;
		else
			return QDF_STATUS_SUCCESS;

	case eSAP_DFS_NEXT_CHANNEL_REQ:
		hdd_notice("Sending next channel query to userspace");
		hdd_update_acs_timer_reason(adapter,
				QCA_WLAN_VENDOR_ACS_SELECT_REASON_DFS);
		return QDF_STATUS_SUCCESS;

	case eSAP_STOP_BSS_DUE_TO_NO_CHNL:
		hdd_debug("Stop sap session[%d]",
			  adapter->session_id);
		INIT_WORK(&adapter->sap_stop_bss_work,
			  hdd_stop_sap_due_to_invalid_channel);
		schedule_work(&adapter->sap_stop_bss_work);
		return QDF_STATUS_SUCCESS;

	default:
		hdd_debug("SAP message is not handled");
		goto stopbss;
		return QDF_STATUS_SUCCESS;
	}
	wireless_send_event(dev, we_event, &wrqu,
			    (char *)we_custom_event_generic);

	return QDF_STATUS_SUCCESS;

stopbss:
	{
		uint8_t we_custom_event[64];
		char *stopBssEvent = "STOP-BSS.response";       /* 17 */
		int event_len = strlen(stopBssEvent);

		hdd_debug("BSS stop status = %s",
		       pSapEvent->sapevt.sapStopBssCompleteEvent.status ?
		       "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");

		/* Change the BSS state now since, as we are shutting
		 * things down, we don't want interfaces to become
		 * re-enabled
		 */
		hostapd_state->bss_state = BSS_STOP;

		if (0 !=
		    (WLAN_HDD_GET_CTX(adapter))->config->
		    nAPAutoShutOff) {
			if (QDF_TIMER_STATE_RUNNING ==
			    ap_ctx->hdd_ap_inactivity_timer.state) {
				qdf_status =
					qdf_mc_timer_stop(&ap_ctx->
							  hdd_ap_inactivity_timer);
				if (!QDF_IS_STATUS_SUCCESS(qdf_status))
					hdd_err("Failed to stop AP inactivity timer");
			}

			qdf_status =
				qdf_mc_timer_destroy(&ap_ctx->
						hdd_ap_inactivity_timer);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				hdd_err("Failed to Destroy AP inactivity timer");
		}
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(hdd_ctx, true);
#endif

		/* Stop the pkts from n/w stack as we are going to free all of
		 * the TX WMM queues for all STAID's
		 */
		hdd_debug("Disabling queues");
		wlan_hdd_netif_queue_control(adapter,
					WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);

		/* reclaim all resources allocated to the BSS */
		qdf_status = hdd_softap_stop_bss(adapter);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_warn("hdd_softap_stop_bss failed %d",
			       qdf_status);
		}

		/* notify userspace that the BSS has stopped */
		memset(&we_custom_event, '\0', sizeof(we_custom_event));
		memcpy(&we_custom_event, stopBssEvent, event_len);
		memset(&wrqu, 0, sizeof(wrqu));
		wrqu.data.length = event_len;
		we_event = IWEVCUSTOM;
		we_custom_event_generic = we_custom_event;
		wireless_send_event(dev, we_event, &wrqu,
				    (char *)we_custom_event_generic);

		/* once the event is set, structure dev/adapter should
		 * not be touched since they are now subject to being deleted
		 * by another thread
		 */
		if (eSAP_STOP_BSS_EVENT == sapEvent)
			qdf_event_set(&hostapd_state->qdf_stop_bss_event);

		hdd_ipa_set_tx_flow_info();
		/* Send SCC/MCC Switching event to IPA */
		hdd_ipa_send_mcc_scc_msg(hdd_ctx, hdd_ctx->mcc_mode);
	}
	return QDF_STATUS_SUCCESS;
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
		hdd_err("Error haHandle returned NULL");
		return -EINVAL;
	}
	/* Validity checks */
	if ((gen_ie_len < QDF_MIN(DOT11F_IE_RSN_MIN_LEN, DOT11F_IE_WPA_MIN_LEN))
	    || (gen_ie_len >
		QDF_MAX(DOT11F_IE_RSN_MAX_LEN, DOT11F_IE_WPA_MAX_LEN)))
		return -EINVAL;
	/* Type check */
	if (gen_ie[0] == DOT11F_EID_RSN) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_RSN_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_RSN_MAX_LEN)) {
			return QDF_STATUS_E_FAILURE;
		}
		/* Skip past the EID byte and length byte */
		pRsnIe = gen_ie + 2;
		RSNIeLen = gen_ie_len - 2;
		/* Unpack the RSN IE */
		memset(&dot11RSNIE, 0, sizeof(tDot11fIERSN));
		sme_unpack_rsn_ie(halHandle, pRsnIe, RSNIeLen,
				  &dot11RSNIE, false);
		/* Copy out the encryption and authentication types */
		hdd_debug("pairwise cipher suite count: %d",
		       dot11RSNIE.pwise_cipher_suite_count);
		hdd_debug("authentication suite count: %d",
		       dot11RSNIE.akm_suite_count);
		/*
		 * Here we have followed the apple base code,
		 * but probably I suspect we can do something different
		 * dot11RSNIE.akm_suite_count
		 * Just translate the FIRST one
		 */
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
	} else if (gen_ie[0] == DOT11F_EID_WPA) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_WPA_MAX_LEN)) {
			return QDF_STATUS_E_FAILURE;
		}
		/* Skip past the EID byte and length byte and 4 byte WiFi OUI */
		pRsnIe = gen_ie + 2 + 4;
		RSNIeLen = gen_ie_len - (2 + 4);
		/* Unpack the WPA IE */
		memset(&dot11WPAIE, 0, sizeof(tDot11fIEWPA));
		dot11f_unpack_ie_wpa((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11WPAIE, false);
		/* Copy out the encryption and authentication types */
		hdd_debug("WPA unicast cipher suite count: %d",
		       dot11WPAIE.unicast_cipher_count);
		hdd_debug("WPA authentication suite count: %d",
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
		hdd_err("gen_ie[0]: %d", gen_ie[0]);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_softap_set_channel_change() -
 * This function to support SAP channel change with CSA IE
 * set in the beacons.
 *
 * @dev: pointer to the net device.
 * @target_channel: target channel number.
 * @target_bw: Target bandwidth to move.
 * If no bandwidth is specified, the value is CH_WIDTH_MAX
 * @forced: Force to switch channel, ignore SCC/MCC check
 *
 * Return: 0 for success, non zero for failure
 */
int hdd_softap_set_channel_change(struct net_device *dev, int target_channel,
				 enum phy_ch_width target_bw, bool forced)
{
	QDF_STATUS status;
	int ret = 0;
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx = NULL;
	struct hdd_adapter *sta_adapter;
	struct hdd_station_ctx *sta_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_validate_channel_and_bandwidth(adapter,
						target_channel, target_bw);
	if (ret) {
		hdd_err("Invalid CH and BW combo");
		return ret;
	}

	sta_adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
	/*
	 * conc_custom_rule1:
	 * Force SCC for SAP + STA
	 * if STA is already connected then we shouldn't allow
	 * channel switch in SAP interface.
	 */
	if (sta_adapter && hdd_ctx->config->conc_custom_rule1) {
		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(sta_adapter);
		if (hdd_conn_is_connected(sta_ctx)) {
			hdd_err("Channel switch not allowed after STA connection with conc_custom_rule1 enabled");
			return -EBUSY;
		}
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
	if (qdf_atomic_inc_return(&adapter->dfs_radar_found) > 1) {
		hdd_err("Channel switch in progress!!");
		return -EBUSY;
	}

	/*
	 * Post the Channel Change request to SAP.
	 */
	status = wlansap_set_channel_change_with_csa(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		(uint32_t)target_channel,
		target_bw, forced);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("SAP set channel failed for channel: %d, bw: %d",
		       target_channel, target_bw);
		/*
		 * If channel change command fails then clear the
		 * radar found flag and also restart the netif
		 * queues.
		 */
		qdf_atomic_set(&adapter->dfs_radar_found, 0);

		ret = -EINVAL;
	}

	return ret;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * hdd_sap_restart_with_channel_switch() - SAP channel change with E/CSA
 * @ap_adapter: HDD adapter
 * @target_channel: Channel to which switch must happen
 * @target_bw: Bandwidth of the target channel
 * @forced: Force to switch channel, ignore SCC/MCC check
 *
 * Invokes the necessary API to perform channel switch for the SAP or GO
 *
 * Return: None
 */
void hdd_sap_restart_with_channel_switch(struct hdd_adapter *ap_adapter,
					uint32_t target_channel,
					uint32_t target_bw,
					bool forced)
{
	struct net_device *dev = ap_adapter->dev;
	int ret;

	ENTER();

	if (!dev) {
		hdd_err("Invalid dev pointer");
		return;
	}

	ret = hdd_softap_set_channel_change(dev, target_channel,
					    target_bw, forced);
	if (ret) {
		hdd_err("channel switch failed");
		return;
	}
}

void hdd_sap_restart_chan_switch_cb(struct wlan_objmgr_psoc *psoc,
				    uint8_t vdev_id, uint32_t channel,
				    uint32_t channel_bw,
				    bool forced)
{
	struct hdd_adapter *ap_adapter =
		wlan_hdd_get_adapter_from_vdev(psoc, vdev_id);

	if (!ap_adapter) {
		hdd_err("Adapter is NULL");
		return;
	}
	hdd_sap_restart_with_channel_switch(ap_adapter, channel,
					    channel_bw, forced);
}

QDF_STATUS wlan_hdd_get_channel_for_sap_restart(
				struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id, uint8_t *channel,
				uint8_t *sec_ch)
{
	tHalHandle *hal_handle;
	struct hdd_ap_ctx *hdd_ap_ctx;
	uint8_t intf_ch = 0;
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *hdd_sta_ctx;
	struct hdd_adapter *sta_adapter;
	struct hdd_adapter *ap_adapter = wlan_hdd_get_adapter_from_vdev(
					psoc, vdev_id);
	if (!ap_adapter) {
		hdd_err("ap_adapter is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* TODO: need work for 3 port case with sta+sta */
	sta_adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
	if (!sta_adapter) {
		hdd_err("sta_adapter is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (NULL == channel || NULL == sec_ch) {
		hdd_err("Null parameters");
		return QDF_STATUS_E_FAILURE;
	}

	if (!test_bit(SOFTAP_BSS_STARTED, &ap_adapter->event_flags)) {
		hdd_err("SOFTAP_BSS_STARTED not set");
		return QDF_STATUS_E_FAILURE;
	}

	hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(ap_adapter);
	hal_handle = WLAN_HDD_GET_HAL_CTX(ap_adapter);
	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(sta_adapter);

	if (!hal_handle) {
		hdd_err("hal_handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Check if STA's channel is DFS or passive or part of LTE avoided
	 * channel list. In that case move SAP to other band if DBS is
	 * supported, return from here if DBS is not supported.
	 * Need to take care of 3 port cases with 2 STA iface in future.
	 */
	intf_ch = wlansap_check_cc_intf(hdd_ap_ctx->sap_context);
	hdd_info("intf_ch: %d", intf_ch);
	if (QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION !=
		hdd_ctx->config->WlanMccToSccSwitchMode) {
		if (QDF_IS_STATUS_ERROR(
			policy_mgr_valid_sap_conc_channel_check(
				hdd_ctx->hdd_psoc,
				&intf_ch,
				policy_mgr_mode_specific_get_channel(
					hdd_ctx->hdd_psoc, PM_SAP_MODE)))) {
			hdd_debug("can't move sap to %d",
				hdd_sta_ctx->conn_info.operationChannel);
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (intf_ch == 0) {
		hdd_debug("interface channel is 0");
		return QDF_STATUS_E_FAILURE;
	}

	hdd_info("SAP restart orig chan: %d, new chan: %d",
		 hdd_ap_ctx->sap_config.channel, intf_ch);
	hdd_ap_ctx->sap_config.channel = intf_ch;
	hdd_ap_ctx->sap_config.ch_params.ch_width =
		hdd_ap_ctx->sap_config.ch_width_orig;
	hdd_ap_ctx->bss_stop_reason = BSS_STOP_DUE_TO_MCC_SCC_SWITCH;

	wlan_reg_set_channel_params(hdd_ctx->hdd_pdev,
			hdd_ap_ctx->sap_config.channel,
			hdd_ap_ctx->sap_config.sec_ch,
			&hdd_ap_ctx->sap_config.ch_params);
	*channel = hdd_ap_ctx->sap_config.channel;
	*sec_ch = hdd_ap_ctx->sap_config.sec_ch;

	hdd_info("SAP channel change with CSA/ECSA");
	hdd_sap_restart_chan_switch_cb(psoc, vdev_id,
		hdd_ap_ctx->sap_config.channel,
		hdd_ap_ctx->sap_config.ch_params.ch_width, false);

	return QDF_STATUS_SUCCESS;
}
#endif

static int __iw_softap_set_ini_cfg(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu,
				   char *extra)
{
	QDF_STATUS status;
	int errno;
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	char *value;
	size_t len;

	ENTER_DEV(dev);

	adapter = netdev_priv(dev);
	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = hdd_check_private_wext_control(hdd_ctx, info);
	if (errno)
		return errno;

	/* ensure null termination by copying into a larger, zeroed buffer */
	len = min_t(size_t, wrqu->data.length, QCSAP_IOCTL_MAX_STR_LEN);
	value = qdf_mem_malloc(len + 1);
	if (!value)
		return -ENOMEM;

	qdf_mem_copy(value, extra, len);
	hdd_debug("Received data %s", value);
	status = hdd_execute_global_config_command(hdd_ctx, value);
	qdf_mem_free(value);

	EXIT();

	return qdf_status_to_os_return(status);
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

static int hdd_sap_get_chan_width(struct hdd_adapter *adapter, int *value)
{
	struct sap_context *sap_ctx;
	struct hdd_hostapd_state *hostapdstate;

	ENTER();
	hostapdstate = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	if (hostapdstate->bss_state != BSS_START) {
		*value = -EINVAL;
		return -EINVAL;
	}

	sap_ctx = WLAN_HDD_GET_SAP_CTX_PTR(adapter);

	*value = wlansap_get_chan_width(sap_ctx);
	hdd_debug("chan_width = %d", *value);

	return 0;
}

int
static __iw_softap_get_ini_cfg(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hdd_debug("Printing CLD global INI Config");
	hdd_cfg_get_global_config(hdd_ctx, extra, QCSAP_IOCTL_MAX_STR_LEN);
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int ret;
	int *value = (int *)extra;
	int sub_cmd = value[0];
	struct hdd_context *hdd_ctx;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
#ifdef WLAN_DEBUG
	case QCSAP_IOCTL_SET_FW_CRASH_INJECT:
		hdd_err("WE_SET_FW_CRASH_INJECT: %d %d",
		       value[1], value[2]);
		if (!hdd_ctx->config->crash_inject_enabled) {
			hdd_err("Crash Inject ini disabled, Ignore Crash Inject");
			return 0;
		}
		if (value[1] == 3) {
			cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
			return 0;
		}
		ret = wma_cli_set2_command(adapter->session_id,
					   GEN_PARAM_CRASH_INJECT,
					   value[1], value[2],
					   GEN_CMD);
		break;
#endif
	case QCSAP_IOCTL_DUMP_DP_TRACE_LEVEL:
		hdd_debug("WE_DUMP_DP_TRACE: %d %d",
		       value[1], value[2]);
		if (value[1] == DUMP_DP_TRACE)
			qdf_dp_trace_dump_all(value[2],
				QDF_TRACE_DEFAULT_PDEV_ID);
		else if (value[1] == ENABLE_DP_TRACE_LIVE_MODE)
			qdf_dp_trace_enable_live_mode();
		else if (value[1] == CLEAR_DP_TRACE_BUFFER)
			qdf_dp_trace_clear_buffer();
		else if (value[1] == DISABLE_DP_TRACE_LIVE_MODE)
			qdf_dp_trace_disable_live_mode();
		break;
	case QCSAP_ENABLE_FW_PROFILE:
		hdd_debug("QCSAP_ENABLE_FW_PROFILE: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->session_id,
				 WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
					value[1], value[2], DBG_CMD);
		break;
	case QCSAP_SET_FW_PROFILE_HIST_INTVL:
		hdd_debug("QCSAP_SET_FW_PROFILE_HIST_INTVL: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(adapter->session_id,
					WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
					value[1], value[2], DBG_CMD);
	case QCSAP_SET_WLAN_SUSPEND:
		hdd_info("SAP unit-test suspend(%d, %d)", value[1], value[2]);
		ret = hdd_wlan_fake_apps_suspend(hdd_ctx->wiphy, dev,
						 value[1], value[2]);
		break;
	case QCSAP_SET_WLAN_RESUME:
		ret = hdd_wlan_fake_apps_resume(hdd_ctx->wiphy, dev);
		break;
	default:
		hdd_err("Invalid IOCTL command: %d", sub_cmd);
		break;
	}

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

static void print_mac_list(struct qdf_mac_addr *macList, uint8_t size)
{
	int i;
	uint8_t *macArray;

	for (i = 0; i < size; i++) {
		macArray = (macList + i)->bytes;
		pr_info("ACL entry %i - %02x:%02x:%02x:%02x:%02x:%02x\n",
			i, MAC_ADDR_ARRAY(macArray));
	}
}

static QDF_STATUS hdd_print_acl(struct hdd_adapter *adapter)
{
	eSapMacAddrACL acl_mode;
	struct qdf_mac_addr maclist[MAX_ACL_MAC_ADDRESS];
	uint8_t listnum;
	struct sap_context *sap_ctx;

	sap_ctx = WLAN_HDD_GET_SAP_CTX_PTR(adapter);
	qdf_mem_zero(&maclist[0], sizeof(maclist));
	if (QDF_STATUS_SUCCESS == wlansap_get_acl_mode(sap_ctx, &acl_mode)) {
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
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_STATUS_SUCCESS == wlansap_get_acl_accept_list(sap_ctx,
							      &maclist[0],
							      &listnum)) {
		pr_info("******* WHITE LIST ***********\n");
		if (listnum <= MAX_ACL_MAC_ADDRESS)
			print_mac_list(&maclist[0], listnum);
	} else {
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_STATUS_SUCCESS == wlansap_get_acl_deny_list(sap_ctx,
							    &maclist[0],
							    &listnum)) {
		pr_info("******* BLACK LIST ***********\n");
		if (listnum <= MAX_ACL_MAC_ADDRESS)
			print_mac_list(&maclist[0], listnum);
	} else {
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

int
static __iw_softap_setparam(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	tHalHandle hHal;
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int set_value = value[1];
	QDF_STATUS status;
	int ret = 0;
	struct hdd_context *hdd_ctx;
	struct cdp_vdev *vdev = NULL;
	struct cdp_pdev *pdev = NULL;
	void *soc = NULL;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return -EINVAL;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (!hHal) {
		hdd_err("Hal ctx is null");
		return -EINVAL;
	}

	switch (sub_cmd) {
	case QCASAP_SET_RADAR_DBG:
		hdd_debug("QCASAP_SET_RADAR_DBG called with: value: %x",
				set_value);
		wlan_sap_enable_phy_error_logs(hHal, set_value);
		break;

	case QCSAP_PARAM_CLR_ACL:
		if (QDF_STATUS_SUCCESS != wlansap_clear_acl(
		    WLAN_HDD_GET_SAP_CTX_PTR(adapter))) {
			ret = -EIO;
		}
		break;

	case QCSAP_PARAM_ACL_MODE:
		if ((eSAP_ALLOW_ALL < (eSapMacAddrACL) set_value) ||
		    (eSAP_ACCEPT_UNLESS_DENIED > (eSapMacAddrACL) set_value)) {
			hdd_err("Invalid ACL Mode value: %d", set_value);
			ret = -EINVAL;
		} else {
			wlansap_set_acl_mode(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				set_value);
		}
		break;

	case QCSAP_PARAM_SET_CHANNEL_CHANGE:
		if ((QDF_SAP_MODE == adapter->device_mode) ||
		   (QDF_P2P_GO_MODE == adapter->device_mode)) {
			hdd_debug("SET Channel Change to new channel= %d",
			       set_value);
			ret = hdd_softap_set_channel_change(dev, set_value,
								CH_WIDTH_MAX,
								false);
		} else {
			hdd_err("Channel Change Failed, Device in test mode");
			ret = -EINVAL;
		}
		break;
	case QCSAP_PARAM_AUTO_CHANNEL:
		if (set_value == 0 || set_value == 1)
			(WLAN_HDD_GET_CTX(
				adapter))->config->force_sap_acs =
								set_value;
		else
			ret = -EINVAL;
		break;
	case QCSAP_PARAM_CONC_SYSTEM_PREF:
		hdd_debug("New preference: %d", set_value);
		if (!((set_value >= CFG_CONC_SYSTEM_PREF_MIN) &&
				(set_value <= CFG_CONC_SYSTEM_PREF_MAX))) {
			hdd_err("Invalid system preference: %d", set_value);
			return -EINVAL;
		}
		/* hdd_ctx, hdd_ctx->config are already checked for null */
		hdd_ctx->config->conc_system_pref = set_value;
		break;
	case QCSAP_PARAM_MAX_ASSOC:
		if (WNI_CFG_ASSOC_STA_LIMIT_STAMIN > set_value) {
			hdd_err("Invalid setMaxAssoc value %d",
			       set_value);
			ret = -EINVAL;
		} else {
			if (WNI_CFG_ASSOC_STA_LIMIT_STAMAX < set_value) {
				hdd_warn("setMaxAssoc %d > max allowed %d.",
				       set_value,
				       WNI_CFG_ASSOC_STA_LIMIT_STAMAX);
				hdd_warn("Setting it to max allowed and continuing");
				set_value = WNI_CFG_ASSOC_STA_LIMIT_STAMAX;
			}
			status = sme_cfg_set_int(hHal, WNI_CFG_ASSOC_STA_LIMIT,
					set_value);
			if (status != QDF_STATUS_SUCCESS) {
				hdd_err("setMaxAssoc failure, status: %d",
				       status);
				ret = -EIO;
			}
		}
		break;

	case QCSAP_PARAM_HIDE_SSID:
	{
		QDF_STATUS status;

		status = sme_update_session_param(hHal,
				adapter->session_id,
				SIR_PARAM_SSID_HIDDEN, set_value);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("QCSAP_PARAM_HIDE_SSID failed");
			return -EIO;
		}
		break;
	}
	case QCSAP_PARAM_SET_MC_RATE:
	{
		tSirRateUpdateInd rateUpdate = {0};
		struct hdd_config *pConfig = hdd_ctx->config;

		hdd_debug("MC Target rate %d", set_value);
		qdf_copy_macaddr(&rateUpdate.bssid,
				 &adapter->mac_addr);
		rateUpdate.nss = (pConfig->enable2x2 == 0) ? 0 : 1;
		rateUpdate.dev_mode = adapter->device_mode;
		rateUpdate.mcastDataRate24GHz = set_value;
		rateUpdate.mcastDataRate24GHzTxFlag = 1;
		rateUpdate.mcastDataRate5GHz = set_value;
		rateUpdate.bcastDataRate = -1;
		status = sme_send_rate_update_ind(hHal, &rateUpdate);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("SET_MC_RATE failed");
			ret = -1;
		}
		break;
	}

	case QCSAP_PARAM_SET_TXRX_FW_STATS:
	{
		hdd_debug("QCSAP_PARAM_SET_TXRX_FW_STATS val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case QCSAP_PARAM_SET_TXRX_STATS:
	{
		ret = cds_get_datapath_handles(&soc, &pdev, &vdev,
				 adapter->session_id);
		if (ret != 0) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				"Invalid Handles");
			break;
		}

		hdd_notice("QCSAP_PARAM_SET_TXRX_STATS val %d", set_value);
		ret = cdp_txrx_stats(soc, vdev, set_value);
		break;
	}

	/* Firmware debug log */
	case QCSAP_DBGLOG_LOG_LEVEL:
	{
		hdd_debug("QCSAP_DBGLOG_LOG_LEVEL val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_VAP_ENABLE:
	{
		hdd_debug("QCSAP_DBGLOG_VAP_ENABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_VAP_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_VAP_DISABLE:
	{
		hdd_debug("QCSAP_DBGLOG_VAP_DISABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_VAP_DISABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_MODULE_ENABLE:
	{
		hdd_debug("QCSAP_DBGLOG_MODULE_ENABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_MODULE_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_MODULE_DISABLE:
	{
		hdd_debug("QCSAP_DBGLOG_MODULE_DISABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_MODULE_DISABLE,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_MOD_LOG_LEVEL:
	{
		hdd_debug("QCSAP_DBGLOG_MOD_LOG_LEVEL val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_MOD_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case QCSAP_DBGLOG_TYPE:
	{
		hdd_debug("QCSAP_DBGLOG_TYPE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_TYPE,
					  set_value, DBG_CMD);
		break;
	}
	case QCSAP_DBGLOG_REPORT_ENABLE:
	{
		hdd_debug("QCSAP_DBGLOG_REPORT_ENABLE val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_DBGLOG_REPORT_ENABLE,
					  set_value, DBG_CMD);
		break;
	}
	case QCSAP_PARAM_SET_MCC_CHANNEL_LATENCY:
	{
		wlan_hdd_set_mcc_latency(adapter, set_value);
		break;
	}

	case QCSAP_PARAM_SET_MCC_CHANNEL_QUOTA:
	{
		hdd_debug("iwpriv cmd to set MCC quota value %dms",
		       set_value);
		ret = wlan_hdd_go_set_mcc_p2p_quota(adapter,
						    set_value);
		break;
	}

	case QCASAP_TXRX_FWSTATS_RESET:
	{
		hdd_debug("WE_TXRX_FWSTATS_RESET val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMA_VDEV_TXRX_FWSTATS_RESET_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case QCSAP_PARAM_RTSCTS:
	{
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_ENABLE_RTSCTS,
					  set_value, VDEV_CMD);
		if (ret) {
			hdd_err("FAILED TO SET RTSCTS at SAP");
			ret = -EIO;
		}
		break;
	}
	case QCASAP_SET_11N_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;
		tsap_Config_t *pConfig =
			&adapter->session.ap.sap_config;

		hdd_debug("SET_HT_RATE val %d", set_value);

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
					hdd_err("Not valid mode for HT");
					ret = -EIO;
					break;
				}
				preamble = WMI_RATE_PREAMBLE_HT;
				nss = HT_RC_2_STREAMS(set_value) - 1;
			} else if (set_value & 0x10) {
				if (pConfig->SapHw_mode ==
				    eCSR_DOT11_MODE_11a) {
					hdd_err("Not valid for cck");
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
					hdd_err("Not valid for OFDM");
					ret = -EIO;
					break;
				}
				preamble = WMI_RATE_PREAMBLE_OFDM;
			}
			set_value = hdd_assemble_rate_code(preamble, nss, rix);
		}
		hdd_debug("SET_HT_RATE val %d rix %d preamble %x nss %d",
		       set_value, rix, preamble, nss);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case QCASAP_SET_VHT_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;
		tsap_Config_t *pConfig =
			&adapter->session.ap.sap_config;

		if (pConfig->SapHw_mode != eCSR_DOT11_MODE_11ac &&
		    pConfig->SapHw_mode != eCSR_DOT11_MODE_11ac_ONLY) {
			hdd_err("SET_VHT_RATE error: SapHw_mode= 0x%x, ch: %d",
			       pConfig->SapHw_mode, pConfig->channel);
			ret = -EIO;
			break;
		}

		if (set_value != 0xff) {
			rix = RC_2_RATE_IDX_11AC(set_value);
			preamble = WMI_RATE_PREAMBLE_VHT;
			nss = HT_RC_2_STREAMS_11AC(set_value) - 1;

			set_value = hdd_assemble_rate_code(preamble, nss, rix);
		}
		hdd_debug("SET_VHT_RATE val %d rix %d preamble %x nss %d",
		       set_value, rix, preamble, nss);

		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case QCASAP_SHORT_GI:
	{
		hdd_debug("QCASAP_SET_SHORT_GI val %d", set_value);
		/*
		 * wma_cli_set_command should be called instead of
		 * sme_update_ht_config since SGI is used for HT/HE.
		 * This should be refactored.
		 *
		 * SGI is same for 20MHZ and 40MHZ.
		 */
		ret = sme_update_ht_config(hHal, adapter->session_id,
					   WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ,
					   set_value);
		if (ret)
			hdd_err("Failed to set ShortGI value ret: %d", ret);
		break;
	}

	case QCSAP_SET_AMPDU:
	{
		hdd_debug("QCSAP_SET_AMPDU %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  GEN_VDEV_PARAM_AMPDU,
					  set_value, GEN_CMD);
		break;
	}

	case QCSAP_SET_AMSDU:
	{
		hdd_debug("QCSAP_SET_AMSDU %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  GEN_VDEV_PARAM_AMSDU,
					  set_value, GEN_CMD);
		break;
	}
	case QCSAP_GTX_HT_MCS:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_HT_MCS %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_HT_MCS,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_VHT_MCS:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_VHT_MCS %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_VHT_MCS,
						set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_USRCFG:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_USR_CFG %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_USR_CFG,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_THRE:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_THRE %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_THRE,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_MARGIN:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_MARGIN %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_MARGIN,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_STEP:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_STEP %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_STEP,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_MINTPC:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_MINTPC %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_MINTPC,
					  set_value, GTX_CMD);
		break;
	}

	case QCSAP_GTX_BWMASK:
	{
		hdd_debug("WMI_VDEV_PARAM_GTX_BWMASK %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_GTX_BW_MASK,
					  set_value, GTX_CMD);
		break;
	}

	case QCASAP_SET_TM_LEVEL:
	{
		hdd_debug("Set Thermal Mitigation Level %d", set_value);
		(void)sme_set_thermal_level(hHal, set_value);
		break;
	}

	case QCASAP_SET_DFS_IGNORE_CAC:
	{
		hdd_debug("Set Dfs ignore CAC  %d", set_value);

		if (adapter->device_mode != QDF_SAP_MODE)
			return -EINVAL;

		ret = wlansap_set_dfs_ignore_cac(hHal, set_value);
		break;
	}

	case QCASAP_SET_DFS_TARGET_CHNL:
	{
		hdd_debug("Set Dfs target channel  %d", set_value);

		if (adapter->device_mode != QDF_SAP_MODE)
			return -EINVAL;

		ret = wlansap_set_dfs_target_chnl(hHal, set_value);
		break;
	}

	case QCASAP_SET_HE_BSS_COLOR:
		if (adapter->device_mode != QDF_SAP_MODE)
			return -EINVAL;

		status = sme_set_he_bss_color(hHal, adapter->session_id,
				set_value);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("SET_HE_BSS_COLOR failed");
			return -EIO;
		}
		break;
	case QCASAP_SET_DFS_NOL:
		wlansap_set_dfs_nol(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter),
			(eSapDfsNolType) set_value);
		break;

	case QCASAP_SET_RADAR_CMD:
	{
		struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
		struct hdd_ap_ctx *ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
		uint8_t ch = ap_ctx->operating_channel;
		struct wlan_objmgr_pdev *pdev;
		struct radar_found_info radar;

		hdd_debug("Set QCASAP_SET_RADAR_CMD val %d", set_value);

		pdev = hdd_ctx->hdd_pdev;
		if (!pdev) {
			hdd_err("null pdev");
			return -EINVAL;
		}

		qdf_mem_zero(&radar, sizeof(radar));
		if (wlan_reg_is_dfs_ch(pdev, ch))
			tgt_dfs_process_radar_ind(pdev, &radar);
		else
			hdd_err("Ignore set radar, op ch(%d) is not dfs", ch);

		break;
	}
	case QCASAP_TX_CHAINMASK_CMD:
	{
		hdd_debug("QCASAP_TX_CHAINMASK_CMD val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_TX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		break;
	}

	case QCASAP_RX_CHAINMASK_CMD:
	{
		hdd_debug("QCASAP_RX_CHAINMASK_CMD val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_PDEV_PARAM_RX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		break;
	}

	case QCASAP_NSS_CMD:
	{
		hdd_debug("QCASAP_NSS_CMD val %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_NSS,
					  set_value, VDEV_CMD);
		break;
	}

	case QCSAP_IPA_UC_STAT:
	{
		/* If input value is non-zero get stats */
		switch (set_value) {
		case 1:
			hdd_ipa_uc_stat(adapter);
			break;
		case 2:
			hdd_ipa_uc_info(hdd_ctx);
			break;
		case 3:
			hdd_ipa_uc_rt_debug_host_dump(hdd_ctx);
			break;
		case 4:
			hdd_ipa_dump_info(hdd_ctx);
			break;
		default:
			/* place holder for stats clean up
			 * Stats clean not implemented yet on FW and IPA
			 */
			break;
		}
		return ret;
	}

	case QCASAP_SET_PHYMODE:
	{
		struct hdd_context *phddctx =
			WLAN_HDD_GET_CTX(adapter);

		ret =
			wlan_hdd_update_phymode(dev, hHal, set_value,
						phddctx);
		break;
	}
	case QCASAP_DUMP_STATS:
	{
		hdd_debug("QCASAP_DUMP_STATS val %d", set_value);
		ret = hdd_wlan_dump_stats(adapter, set_value);
		break;
	}
	case QCASAP_CLEAR_STATS:
	{
		struct hdd_context *hdd_ctx =
			WLAN_HDD_GET_CTX(adapter);
		void *soc = cds_get_context(QDF_MODULE_ID_SOC);

		hdd_debug("QCASAP_CLEAR_STATS val %d", set_value);
		switch (set_value) {
		case CDP_HDD_STATS:
			memset(&adapter->stats, 0,
						sizeof(adapter->stats));
			memset(&adapter->hdd_stats, 0,
					sizeof(adapter->hdd_stats));
			break;
		case CDP_TXRX_HIST_STATS:
			wlan_hdd_clear_tx_rx_histogram(hdd_ctx);
			break;
		case CDP_HDD_NETIF_OPER_HISTORY:
			wlan_hdd_clear_netif_queue_history(hdd_ctx);
			break;
		case CDP_HIF_STATS:
			hdd_clear_hif_stats();
			break;
		default:
			if (soc)
				cdp_clear_stats(soc, set_value);
		}
		break;
	}
	case QCSAP_START_FW_PROFILING:
		hdd_debug("QCSAP_START_FW_PROFILING %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					WMI_WLAN_PROFILE_TRIGGER_CMDID,
					set_value, DBG_CMD);
		break;
	case QCASAP_PARAM_LDPC:
		ret = hdd_set_ldpc(adapter, set_value);
		break;
	case QCASAP_PARAM_TX_STBC:
		ret = hdd_set_tx_stbc(adapter, set_value);
		break;
	case QCASAP_PARAM_RX_STBC:
		ret = hdd_set_rx_stbc(adapter, set_value);
		break;
	case QCASAP_SET_11AX_RATE:
		ret = hdd_set_11ax_rate(adapter, set_value,
					&adapter->session.ap.
					sap_config);
		break;
	case QCASAP_SET_PEER_RATE:
		ret = hdd_set_peer_rate(adapter, set_value);
		break;
	case QCASAP_PARAM_DCM:
		hdd_notice("Set WMI_VDEV_PARAM_HE_DCM: %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_HE_DCM, set_value,
					  VDEV_CMD);
		break;
	case QCASAP_PARAM_RANGE_EXT:
		hdd_notice("Set WMI_VDEV_PARAM_HE_RANGE_EXT: %d", set_value);
		ret = wma_cli_set_command(adapter->session_id,
					  WMI_VDEV_PARAM_HE_RANGE_EXT,
					  set_value, VDEV_CMD);
		break;
	case QCSAP_SET_DEFAULT_AMPDU:
		hdd_debug("QCSAP_SET_DEFAULT_AMPDU val %d", set_value);
		ret = wma_cli_set_command((int)adapter->session_id,
				(int)WMI_PDEV_PARAM_MAX_MPDUS_IN_AMPDU,
				set_value, PDEV_CMD);
		break;
	case QCSAP_ENABLE_RTS_BURSTING:
		hdd_debug("QCSAP_ENABLE_RTS_BURSTING val %d", set_value);
		ret = wma_cli_set_command((int)adapter->session_id,
				(int)WMI_PDEV_PARAM_ENABLE_RTS_SIFS_BURSTING,
				set_value, PDEV_CMD);
		break;
	default:
		hdd_err("Invalid setparam command %d value %d",
		       sub_cmd, set_value);
		ret = -EINVAL;
		break;
	}
	EXIT();
	return ret;
}

/**
 * __iw_softap_get_three() - return three value to upper layer.
 * @dev: pointer of net_device of this wireless card
 * @info: meta data about Request sent
 * @wrqu: include request info
 * @extra: buf used for in/out
 *
 * Return: execute result
 */
static int __iw_softap_get_three(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	uint32_t *value = (uint32_t *)extra;
	uint32_t sub_cmd = value[0];
	int ret = 0; /* success */
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case QCSAP_GET_TSF:
		ret = hdd_indicate_tsf(adapter, value, 3);
		break;
	default:
		hdd_err("Invalid getparam command: %d", sub_cmd);
		ret = -EINVAL;
		break;
	}
	return ret;
}


/**
 * iw_softap_get_three() - return three value to upper layer.
 *
 * @dev: pointer of net_device of this wireless card
 * @info: meta data about Request sent
 * @wrqu: include request info
 * @extra: buf used for in/Output
 *
 * Return: execute result
 */
static int iw_softap_get_three(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_get_three(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

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
	struct hdd_adapter *adapter = (netdev_priv(dev));
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	int *value = (int *)extra;
	int sub_cmd = value[0];
	QDF_STATUS status;
	int ret;
	struct hdd_context *hdd_ctx;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case QCSAP_PARAM_MAX_ASSOC:
		status = sme_cfg_get_int(hHal, WNI_CFG_ASSOC_STA_LIMIT,
					(uint32_t *) value);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("get WNI_CFG_ASSOC_STA_LIMIT failed status: %d",
				status);
			ret = -EIO;
		}
		break;

	case QCSAP_PARAM_AUTO_CHANNEL:
		*value = (WLAN_HDD_GET_CTX
			(adapter))->config->force_sap_acs;
		break;

	case QCSAP_PARAM_GET_WLAN_DBG:
	{
		qdf_trace_display();
		*value = 0;
		break;
	}

	case QCSAP_PARAM_RTSCTS:
	{
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_ENABLE_RTSCTS,
					     VDEV_CMD);
		break;
	}

	case QCASAP_SHORT_GI:
	{
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_SGI,
					     VDEV_CMD);
		break;
	}

	case QCSAP_GTX_HT_MCS:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_HT_MCS");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_HT_MCS,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_VHT_MCS:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_VHT_MCS");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_VHT_MCS,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_USRCFG:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_USR_CFG");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_USR_CFG,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_THRE:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_THRE");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_THRE,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_MARGIN:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_MARGIN");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_MARGIN,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_STEP:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_STEP");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_STEP,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_MINTPC:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_MINTPC");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_MINTPC,
					     GTX_CMD);
		break;
	}

	case QCSAP_GTX_BWMASK:
	{
		hdd_debug("GET WMI_VDEV_PARAM_GTX_BW_MASK");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_GTX_BW_MASK,
					     GTX_CMD);
		break;
	}

	case QCASAP_GET_DFS_NOL:
	{
		struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
		struct wlan_objmgr_pdev *pdev;

		pdev = hdd_ctx->hdd_pdev;
		if (!pdev) {
			hdd_err("null pdev");
			return -EINVAL;
		}

		utils_dfs_print_nol_channels(pdev);
	}
	break;

	case QCSAP_GET_ACL:
	{
		hdd_debug("QCSAP_GET_ACL");
		if (hdd_print_acl(adapter) !=
		    QDF_STATUS_SUCCESS) {
			hdd_err("QCSAP_GET_ACL returned Error: not completed");
		}
		*value = 0;
		break;
	}

	case QCASAP_TX_CHAINMASK_CMD:
	{
		hdd_debug("QCASAP_TX_CHAINMASK_CMD");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_TX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case QCASAP_RX_CHAINMASK_CMD:
	{
		hdd_debug("QCASAP_RX_CHAINMASK_CMD");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_PDEV_PARAM_RX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case QCASAP_NSS_CMD:
	{
		hdd_debug("QCASAP_NSS_CMD");
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_NSS,
					     VDEV_CMD);
		break;
	}
	case QCSAP_CAP_TSF:
		ret = hdd_capture_tsf(adapter, (uint32_t *)value, 1);
		break;
	case QCASAP_GET_TEMP_CMD:
	{
		hdd_debug("QCASAP_GET_TEMP_CMD");
		ret = wlan_hdd_get_temperature(adapter, value);
		break;
	}
	case QCSAP_GET_FW_PROFILE_DATA:
		hdd_debug("QCSAP_GET_FW_PROFILE_DATA");
		ret = wma_cli_set_command(adapter->session_id,
				WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
				0, DBG_CMD);
		break;
	case QCASAP_PARAM_LDPC:
	{
		ret = hdd_get_ldpc(adapter, value);
		break;
	}
	case QCASAP_PARAM_TX_STBC:
	{
		ret = hdd_get_tx_stbc(adapter, value);
		break;
	}
	case QCASAP_PARAM_RX_STBC:
	{
		ret = hdd_get_rx_stbc(adapter, value);
		break;
	}
	case QCSAP_PARAM_CHAN_WIDTH:
	{
		ret = hdd_sap_get_chan_width(adapter, value);
		break;
	}
	case QCASAP_PARAM_DCM:
	{
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_HE_DCM,
					     VDEV_CMD);
		break;
	}
	case QCASAP_PARAM_RANGE_EXT:
	{
		*value = wma_cli_get_command(adapter->session_id,
					     WMI_VDEV_PARAM_HE_RANGE_EXT,
					     VDEV_CMD);
		break;
	}
	default:
		hdd_err("Invalid getparam command: %d", sub_cmd);
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
 *  BLACK_LIST  = 0
 *  WHITE_LIST  = 1
 *  ADD MAC = 0
 *  REMOVE MAC  = 1
 *
 *  mac addr will be accepted as a 6 octet mac address with each octet
 *  inputted in hex for e.g. 00:0a:f5:11:22:33 will be represented as
 *  0x00 0x0a 0xf5 0x11 0x22 0x33 while using this ioctl
 *
 *  Syntax:
 *  iwpriv softap.0 modify_acl
 *  <6 octet mac addr> <list type> <cmd type>
 *
 *  Examples:
 *  eg 1. to add a mac addr 00:0a:f5:89:89:90 to the black list
 *  iwpriv softap.0 modify_acl 0x00 0x0a 0xf5 0x89 0x89 0x90 0 0
 *  eg 2. to delete a mac addr 00:0a:f5:89:89:90 from white list
 *  iwpriv softap.0 modify_acl 0x00 0x0a 0xf5 0x89 0x89 0x90 1 1
 */
static
int __iw_softap_modify_acl(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	uint8_t *value = (uint8_t *) extra;
	uint8_t pPeerStaMac[QDF_MAC_ADDR_SIZE];
	int listType, cmd, i;
	int ret;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct hdd_context *hdd_ctx;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	for (i = 0; i < QDF_MAC_ADDR_SIZE; i++)
		pPeerStaMac[i] = *(value + i);

	listType = (int)(*(value + i));
	i++;
	cmd = (int)(*(value + i));

	hdd_debug("Modify ACL mac:" MAC_ADDRESS_STR " type: %d cmd: %d",
	       MAC_ADDR_ARRAY(pPeerStaMac), listType, cmd);

	qdf_status = wlansap_modify_acl(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		pPeerStaMac, (eSapACLType) listType, (eSapACLCmdType) cmd);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("Modify ACL failed");
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
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx;
	int *value = (int *)extra;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	*value = 0;
	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags))
		*value = (WLAN_HDD_GET_AP_CTX_PTR(
					adapter))->operating_channel;
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
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	int *value = (int *)extra;
	int set_value;
	int ret;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_BCAST_INIT;
	struct qdf_mac_addr selfMac = QDF_MAC_ADDR_BCAST_INIT;

	ENTER_DEV(dev);

	if (NULL == value)
		return -ENOMEM;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* Assign correct self MAC address */
	qdf_copy_macaddr(&bssid, &adapter->mac_addr);
	qdf_copy_macaddr(&selfMac, &adapter->mac_addr);

	set_value = value[0];
	if (QDF_STATUS_SUCCESS !=
	    sme_set_max_tx_power(hHal, bssid, selfMac, set_value)) {
		hdd_err("Setting maximum tx power failed");
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
static __iw_softap_set_pktlog(struct net_device *dev,
				    struct iw_request_info *info,
				    union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = netdev_priv(dev);
	struct hdd_context *hdd_ctx;
	int *value = (int *)extra;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (wrqu->data.length < 1 || wrqu->data.length > 2) {
		hdd_err("pktlog: either 1 or 2 parameters are required");
		return -EINVAL;
	}

	return hdd_process_pktlog_command(hdd_ctx, value[0], value[1]);
}

int
static iw_softap_set_pktlog(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_softap_set_pktlog(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}


int
static __iw_softap_set_tx_power(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	struct hdd_context *hdd_ctx;
	int *value = (int *)extra;
	int set_value;
	struct qdf_mac_addr bssid;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	qdf_copy_macaddr(&bssid, &adapter->mac_addr);

	set_value = value[0];
	if (QDF_STATUS_SUCCESS !=
	    sme_set_tx_power(hHal, adapter->session_id, bssid,
			     adapter->device_mode, set_value)) {
		hdd_err("Setting tx power failed");
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
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_station_info *pStaInfo = adapter->sta_info;
	struct hdd_context *hdd_ctx;
	char *buf;
	int cnt = 0;
	int left;
	int ret;
	/* maclist_index must be u32 to match userspace */
	u32 maclist_index;

	ENTER_DEV(dev);

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

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* make sure userspace allocated a reasonable buffer size */
	if (wrqu->data.length < sizeof(maclist_index)) {
		hdd_err("invalid userspace buffer");
		return -EINVAL;
	}

	/* allocate local buffer to build the response */
	buf = qdf_mem_malloc(wrqu->data.length);
	if (!buf) {
		hdd_err("failed to allocate response buffer");
		return -ENOMEM;
	}

	/* start indexing beyond where the record count will be written */
	maclist_index = sizeof(maclist_index);
	left = wrqu->data.length - maclist_index;

	spin_lock_bh(&adapter->sta_info_lock);
	while ((cnt < WLAN_MAX_STA_COUNT) && (left >= QDF_MAC_ADDR_SIZE)) {
		if ((pStaInfo[cnt].in_use) &&
		    (!IS_BROADCAST_MAC(pStaInfo[cnt].sta_mac.bytes))) {
			memcpy(&buf[maclist_index], &(pStaInfo[cnt].sta_mac),
			       QDF_MAC_ADDR_SIZE);
			maclist_index += QDF_MAC_ADDR_SIZE;
			left -= QDF_MAC_ADDR_SIZE;
		}
		cnt++;
	}
	spin_unlock_bh(&adapter->sta_info_lock);

	*((u32 *) buf) = maclist_index;
	wrqu->data.length = maclist_index;
	if (copy_to_user(wrqu->data.pointer, buf, maclist_index)) {
		hdd_err("failed to copy response to user buffer");
		ret = -EFAULT;
	}
	qdf_mem_free(buf);
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
 *  mac addr will be accepted as a 6 octet mac address with each octet
 *  inputted in hex for e.g. 00:0a:f5:11:22:33 will be represented as
 *  0x00 0x0a 0xf5 0x11 0x22 0x33 while using this ioctl
 *
 *  Syntax:
 *  iwpriv softap.0 disassoc_sta <6 octet mac address>
 *
 *  e.g.
 *  disassociate sta with mac addr 00:0a:f5:11:22:33 from softap
 *  iwpriv softap.0 disassoc_sta 0x00 0x0a 0xf5 0x11 0x22 0x33
 */

int
static __iw_softap_disassoc_sta(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx;
	uint8_t *peerMacAddr;
	int ret;
	struct csr_del_sta_params del_sta_params;

	ENTER_DEV(dev);

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* iwpriv tool or framework calls this ioctl with
	 * data passed in extra (less than 16 octets);
	 */
	peerMacAddr = (uint8_t *) (extra);

	hdd_debug("data " MAC_ADDRESS_STR,
	       MAC_ADDR_ARRAY(peerMacAddr));
	wlansap_populate_del_sta_params(peerMacAddr,
			eSIR_MAC_DEAUTH_LEAVING_BSS_REASON,
			(SIR_MAC_MGMT_DISASSOC >> 4),
			&del_sta_params);
	hdd_softap_sta_disassoc(adapter, &del_sta_params);

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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int ret;
	int sub_cmd = wrqu->data.flags;
	struct hdd_context *hdd_ctx;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
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
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;
	int *value = (int *)extra;

	ENTER_DEV(dev);

	if (!capable(CAP_NET_ADMIN)) {
		hdd_err("permission check failed");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (wlan_hdd_validate_operation_channel(adapter, value[0]) !=
					 QDF_STATUS_SUCCESS ||
		wlan_hdd_validate_operation_channel(adapter, value[1]) !=
					 QDF_STATUS_SUCCESS) {
		return -EINVAL;
	}
	hdd_ctx->config->force_sap_acs_st_ch = value[0];
	hdd_ctx->config->force_sap_acs_end_ch = value[1];

	hdd_ctx->config->force_sap_acs_st_ch = value[0];
	hdd_ctx->config->force_sap_acs_end_ch = value[1];

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

static int __iw_get_channel_list(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	uint32_t num_channels = 0;
	uint8_t i = 0;
	uint8_t band_start_channel = CHAN_ENUM_1;
	uint8_t band_end_channel = CHAN_ENUM_184;
	struct hdd_adapter *hostapd_adapter = (netdev_priv(dev));
	tHalHandle hal = WLAN_HDD_GET_HAL_CTX(hostapd_adapter);
	struct channel_list_info *channel_list =
					(struct channel_list_info *) extra;
	enum band_info cur_band = BAND_ALL;
	struct hdd_context *hdd_ctx;
	int ret;
	bool is_dfs_mode_enabled = false;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(hostapd_adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (QDF_STATUS_SUCCESS != sme_get_freq_band(hal, &cur_band)) {
		hdd_err("not able get the current frequency band");
		return -EIO;
	}
	wrqu->data.length = sizeof(struct channel_list_info);

	if (BAND_2G == cur_band) {
		band_start_channel = CHAN_ENUM_1;
		band_end_channel = CHAN_ENUM_14;
	} else if (BAND_5G == cur_band) {
		band_start_channel = CHAN_ENUM_36;
		band_end_channel = CHAN_ENUM_184;
	}

	if (cur_band != BAND_2G) {
		if (hdd_ctx->config->dot11p_mode)
			band_end_channel = CHAN_ENUM_184;
		else
			band_end_channel = CHAN_ENUM_165;
	}

	if (hostapd_adapter->device_mode == QDF_STA_MODE &&
	    hdd_ctx->config->enableDFSChnlScan) {
		is_dfs_mode_enabled = true;
	} else if (hostapd_adapter->device_mode == QDF_SAP_MODE &&
		   hdd_ctx->config->enableDFSMasterCap) {
		is_dfs_mode_enabled = true;
	}

	hdd_debug("curBand = %d, StartChannel = %hu, EndChannel = %hu is_dfs_mode_enabled  = %d ",
			cur_band, band_start_channel, band_end_channel,
			is_dfs_mode_enabled);

	for (i = band_start_channel; i <= band_end_channel; i++) {
		if ((CHANNEL_STATE_ENABLE ==
		     wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
						WLAN_REG_CH_NUM(i))) ||
		    (is_dfs_mode_enabled && CHANNEL_STATE_DFS ==
		     wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
						WLAN_REG_CH_NUM(i)))) {
			channel_list->channels[num_channels] =
						WLAN_REG_CH_NUM(i);
			num_channels++;
		}
	}

	hdd_debug("number of channels %d", num_channels);

	channel_list->num_channels = num_channels;
	EXIT();

	return 0;
}

int iw_get_channel_list(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_channel_list(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

static
int __iw_get_genie(struct net_device *dev,
		   struct iw_request_info *info,
		   union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx;
	int ret;
	QDF_STATUS status;
	uint32_t length = DOT11F_IE_RSN_MAX_LEN;
	uint8_t genIeBytes[DOT11F_IE_RSN_MAX_LEN];

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/*
	 * Actually retrieve the RSN IE from CSR.
	 * (We previously sent it down in the CSR Roam Profile.)
	 */
	status = wlan_sap_getstation_ie_information(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		&length, genIeBytes);
	if (status == QDF_STATUS_SUCCESS) {
		wrqu->data.length = length;
		if (length > DOT11F_IE_RSN_MAX_LEN) {
			hdd_err("Invalid buffer length: %d", length);
			return -E2BIG;
		}
		qdf_mem_copy(extra, genIeBytes, length);
		hdd_debug(" RSN IE of %d bytes returned",
				wrqu->data.length);
	} else {
		wrqu->data.length = 0;
		hdd_debug(" RSN IE failed to populate");
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
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx;
	tHalHandle hHal;
	struct hdd_hostapd_state *hostapd_state;
	struct hdd_ap_ctx *ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
	hHal = WLAN_HDD_GET_HAL_CTX(adapter);

	if (hostapd_state->bss_state == BSS_STOP) {
		if (sme_cfg_get_int(hHal, WNI_CFG_CURRENT_CHANNEL, &channel)
		    != QDF_STATUS_SUCCESS) {
			return -EIO;
		}
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
	} else {
		channel = ap_ctx->operating_channel;
		status = hdd_wlan_get_freq(channel, &freq);
		if (true == status) {
			/* Set Exponent parameter as 6 (MHZ) in struct iw_freq
			 * iwlist & iwconfig command shows frequency into proper
			 * format (2.412 GHz instead of 246.2 MHz)
			 */
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

static int
__iw_softap_stopbss(struct net_device *dev,
		    struct iw_request_info *info,
		    union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	QDF_STATUS status;
	struct hdd_context *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		struct hdd_hostapd_state *hostapd_state =
			WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

		qdf_event_reset(&hostapd_state->qdf_stop_bss_event);
		status = wlansap_stop_bss(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter));
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status =
				qdf_wait_for_event_completion(&hostapd_state->
					qdf_stop_bss_event,
					SME_CMD_TIMEOUT_VALUE);

			if (!QDF_IS_STATUS_SUCCESS(status)) {
				hdd_err("wait for single_event failed!!");
				QDF_ASSERT(0);
			}
		}
		clear_bit(SOFTAP_BSS_STARTED, &adapter->event_flags);
		policy_mgr_decr_session_set_pcl(hdd_ctx->hdd_psoc,
					     adapter->device_mode,
					     adapter->session_id);
		ret = qdf_status_to_os_return(status);
	}
	EXIT();
	return ret;
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
	struct hdd_adapter *adapter = netdev_priv(dev);
	struct hdd_context *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hdd_wlan_get_version(hdd_ctx, wrqu, extra);
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

static int hdd_softap_get_sta_info(struct hdd_adapter *adapter,
				   uint8_t *buf,
				   int size)
{
	int i;
	int written;
	uint8_t bc_sta_id;

	ENTER();

	bc_sta_id = WLAN_HDD_GET_AP_CTX_PTR(adapter)->broadcast_sta_id;

	written = scnprintf(buf, size, "\nstaId staAddress\n");
	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		struct hdd_station_info *sta = &adapter->sta_info[i];

		if (written >= size - 1)
			break;

		if (!sta->in_use)
			continue;

		if (i == bc_sta_id)
			continue;

		written += scnprintf(buf + written, size - written,
				     "%5d %02x:%02x:%02x:%02x:%02x:%02x ecsa=%d\n",
				     sta->sta_id,
				     sta->sta_mac.bytes[0],
				     sta->sta_mac.bytes[1],
				     sta->sta_mac.bytes[2],
				     sta->sta_mac.bytes[3],
				     sta->sta_mac.bytes[4],
				     sta->sta_mac.bytes[5],
				     sta->ecsa_capable);
	}

	EXIT();

	return 0;
}

static int __iw_softap_get_sta_info(struct net_device *dev,
				    struct iw_request_info *info,
				    union iwreq_data *wrqu, char *extra)
{
	int errno;
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;

	ENTER_DEV(dev);

	adapter = netdev_priv(dev);
	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = hdd_check_private_wext_control(hdd_ctx, info);
	if (errno)
		return errno;

	errno = hdd_softap_get_sta_info(adapter, extra, WE_SAP_MAX_STA_INFO);
	if (errno) {
		hdd_err("Failed to get sta info; errno:%d", errno);
		return errno;
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

static
int __iw_get_softap_linkspeed(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = (netdev_priv(dev));
	struct hdd_context *hdd_ctx;
	char *pLinkSpeed = (char *)extra;
	uint32_t link_speed = 0;
	int len = sizeof(uint32_t) + 1;
	struct qdf_mac_addr macAddress;
	char pmacAddress[MAC_ADDRESS_STR_LEN + 1];
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int rc, ret, i;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_private_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hdd_debug("wrqu->data.length(%d)", wrqu->data.length);

	/* Linkspeed is allowed only for P2P mode */
	if (adapter->device_mode != QDF_P2P_GO_MODE) {
		hdd_err("Link Speed is not allowed in Device mode %s(%d)",
			hdd_device_mode_to_string(
				adapter->device_mode),
			adapter->device_mode);
		return -ENOTSUPP;
	}

	if (wrqu->data.length >= MAC_ADDRESS_STR_LEN - 1) {
		if (copy_from_user((void *)pmacAddress,
				   wrqu->data.pointer, MAC_ADDRESS_STR_LEN)) {
			hdd_err("failed to copy data to user buffer");
			return -EFAULT;
		}
		pmacAddress[MAC_ADDRESS_STR_LEN - 1] = '\0';

		if (!mac_pton(pmacAddress, macAddress.bytes)) {
			hdd_err("String to Hex conversion Failed");
			return -EINVAL;
		}
	}
	/* If no mac address is passed and/or its length is less than 17,
	 * link speed for first connected client will be returned.
	 */
	if (wrqu->data.length < 17 || !QDF_IS_STATUS_SUCCESS(status)) {
		for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
			if (adapter->sta_info[i].in_use &&
			    (!qdf_is_macaddr_broadcast
				  (&adapter->sta_info[i].sta_mac))) {
				qdf_copy_macaddr(
					&macAddress,
					&adapter->sta_info[i].
					 sta_mac);
				status = QDF_STATUS_SUCCESS;
				break;
			}
		}
	}
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Invalid peer macaddress");
		return -EINVAL;
	}
	rc = wlan_hdd_get_linkspeed_for_peermac(adapter, &macAddress,
						&link_speed);
	if (!rc) {
		hdd_err("Unable to retrieve SME linkspeed");
		return rc;
	}

	/* linkspeed in units of 500 kbps */
	link_speed = link_speed / 500;
	wrqu->data.length = len;
	rc = snprintf(pLinkSpeed, len, "%u", link_speed);
	if ((rc < 0) || (rc >= len)) {
		/* encoding or length error? */
		hdd_err("Unable to encode link speed");
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

/**
 * __iw_get_peer_rssi() - get station's rssi
 * @dev: net device
 * @info: iwpriv request information
 * @wrqu: iwpriv command parameter
 * @extra
 *
 * This function will call wlan_hdd_get_peer_rssi
 * to get rssi
 *
 * Return: 0 on success, otherwise error value
 */
static int
__iw_get_peer_rssi(struct net_device *dev, struct iw_request_info *info,
		   union iwreq_data *wrqu, char *extra)
{
	struct hdd_adapter *adapter = netdev_priv(dev);
	struct hdd_context *hddctx;
	char macaddrarray[MAC_ADDRESS_STR_LEN];
	struct qdf_mac_addr macaddress = QDF_MAC_ADDR_BCAST_INIT;
	int ret;
	char *rssi_info_output = extra;
	struct sir_peer_sta_info peer_sta_info;
	struct sir_peer_info *rssi_info;
	int i;
	int buf;
	int length;

	ENTER();

	hddctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hddctx);
	if (ret != 0)
		return ret;

	ret = hdd_check_private_wext_control(hddctx, info);
	if (0 != ret)
		return ret;

	hdd_debug("wrqu->data.length= %d", wrqu->data.length);

	if (wrqu->data.length >= MAC_ADDRESS_STR_LEN - 1) {
		if (copy_from_user(macaddrarray,
				   wrqu->data.pointer,
				   MAC_ADDRESS_STR_LEN - 1)) {
			hdd_info("failed to copy data from user buffer");
			return -EFAULT;
		}

		macaddrarray[MAC_ADDRESS_STR_LEN - 1] = '\0';
		hdd_debug("%s", macaddrarray);

		if (!mac_pton(macaddrarray, macaddress.bytes))
			hdd_err("String to Hex conversion Failed");
	}

	ret = wlan_hdd_get_peer_rssi(adapter, &macaddress, &peer_sta_info);
	if (ret) {
		hdd_err("Unable to retrieve peer rssi: %d", ret);
		return ret;
	}
	/*
	 * The iwpriv tool default print is before mac addr and rssi.
	 * Add '\n' before first rssi item to align the first rssi item
	 * with others
	 *
	 * wlan     getRSSI:
	 * [macaddr1] [rssi1]
	 * [macaddr2] [rssi2]
	 * [macaddr3] [rssi3]
	 */
	length = scnprintf(rssi_info_output, WE_MAX_STR_LEN, "\n");
	rssi_info = &peer_sta_info.info[0];
	for (i = 0; i < peer_sta_info.sta_num; i++) {
		buf = scnprintf
			(
			rssi_info_output + length, WE_MAX_STR_LEN - length,
			"[%pM] [%d]\n",
			rssi_info[i].peer_macaddr.bytes,
			rssi_info[i].rssi
			);
		length += buf;
	}
	wrqu->data.length = length + 1;

	EXIT();

	return 0;
}

/**
 * iw_get_peer_rssi() - get station's rssi
 * @dev: net device
 * @info: iwpriv request information
 * @wrqu: iwpriv command parameter
 * @extra
 *
 * This function will call __iw_get_peer_rssi
 *
 * Return: 0 on success, otherwise error value
 */
static int
iw_get_peer_rssi(struct net_device *dev, struct iw_request_info *info,
		 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_peer_rssi(dev, info, wrqu, extra);
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
	(iw_handler) NULL,      /* SIOCGIWMODE */
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
	(iw_handler) NULL,      /* SIOCSIWMLME */
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
	(iw_handler) NULL,      /* SIOCGIWRTS */
	(iw_handler) NULL,      /* SIOCSIWFRAG */
	(iw_handler) NULL,      /* SIOCGIWFRAG */
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
	(iw_handler) NULL,      /* SIOCSIWGENIE */
	(iw_handler) NULL,      /* SIOCGIWGENIE */
	(iw_handler) NULL,      /* SIOCSIWAUTH */
	(iw_handler) NULL,      /* SIOCGIWAUTH */
	(iw_handler) NULL,      /* SIOCSIWENCODEEXT */
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
	}, {
		QCSAP_PARAM_SET_TXRX_FW_STATS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"txrx_fw_stats"
	}, {
		QCSAP_PARAM_SET_TXRX_STATS,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"txrx_stats"
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
	}, {
		QCSAP_PARAM_CONC_SYSTEM_PREF,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setConcSysPref"
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
		QCASAP_SET_HE_BSS_COLOR,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_he_bss_clr"
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
		QCASAP_PARAM_LDPC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_ldpc"
	}, {
		QCASAP_PARAM_TX_STBC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_tx_stbc"
	}, {
		QCASAP_PARAM_RX_STBC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_rx_stbc"
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
		QCASAP_PARAM_LDPC, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_ldpc"
	}, {
		QCASAP_PARAM_TX_STBC, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_tx_stbc"
	}, {
		QCASAP_PARAM_RX_STBC, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_rx_stbc"
	}, {
		QCSAP_PARAM_CHAN_WIDTH, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"get_chwidth"
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
		QCSAP_CAP_TSF, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"cap_tsf"
	}, {
		QCSAP_IOCTL_SET_NONE_GET_THREE, 0, IW_PRIV_TYPE_INT |
		IW_PRIV_SIZE_FIXED | 3,    ""
	}, {
		QCSAP_GET_TSF, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
		"get_tsf"
	}, {
		QCASAP_GET_TEMP_CMD, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_temp"
	}, {
		QCSAP_GET_FW_PROFILE_DATA, 0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getProfileData"
	}, {
		QCSAP_IOCTL_GET_STAWPAIE,
		0, IW_PRIV_TYPE_BYTE | DOT11F_IE_RSN_MAX_LEN,
		"get_staWPAIE"
	}, {
		QCSAP_IOCTL_STOPBSS, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED, 0,
		"stopbss"
	}, {
		QCSAP_IOCTL_VERSION, 0, IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"version"
	}, {
		QCSAP_IOCTL_GET_STA_INFO, 0,
		IW_PRIV_TYPE_CHAR | WE_SAP_MAX_STA_INFO, "get_sta_info"
	}, {
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
		QCSAP_IOCTL_PRIV_GET_RSSI,
		IW_PRIV_TYPE_CHAR | 18,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN, "getRSSI"
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
#ifdef WLAN_DEBUG
	,
	{
		WE_SET_CHAN_AVOID,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
		0,
		"ch_avoid"
	}
#endif
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
		IW_PRIV_TYPE_BYTE | sizeof(struct channel_list_info),
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
	{
		QCSAP_IOCTL_SET_PKTLOG,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
		0, "pktlog"
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
#ifdef WLAN_DEBUG
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
	,
#ifdef WLAN_SUSPEND_RESUME_TEST
	{
		QCSAP_SET_WLAN_SUSPEND,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		0, "wlan_suspend"
	}
	,
	{
		QCSAP_SET_WLAN_RESUME,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		0, "wlan_resume"
	}
	,
#endif
	{
		QCASAP_SET_11AX_RATE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_11ax_rate"
	}
	,
	{
		QCASAP_SET_PEER_RATE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "set_peer_rate"
	}
	,
	{
		QCASAP_PARAM_DCM,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "enable_dcm"
	}
	,
	{
		QCASAP_PARAM_RANGE_EXT,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "range_ext"
	}
	,
	{	QCSAP_SET_DEFAULT_AMPDU,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "def_ampdu"
	}
	,
	{	QCSAP_ENABLE_RTS_BURSTING,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0, "rts_bursting"
	}
	,
};

static const iw_handler hostapd_private[] = {
	/* set priv ioctl */
	[QCSAP_IOCTL_SETPARAM - SIOCIWFIRSTPRIV] = iw_softap_setparam,
	/* get priv ioctl */
	[QCSAP_IOCTL_GETPARAM - SIOCIWFIRSTPRIV] = iw_softap_getparam,
	[QCSAP_IOCTL_SET_NONE_GET_THREE - SIOCIWFIRSTPRIV] =
							iw_softap_get_three,
	/* get station genIE */
	[QCSAP_IOCTL_GET_STAWPAIE - SIOCIWFIRSTPRIV] = iw_get_genie,
	/* stop bss */
	[QCSAP_IOCTL_STOPBSS - SIOCIWFIRSTPRIV] = iw_softap_stopbss,
	/* get driver version */
	[QCSAP_IOCTL_VERSION - SIOCIWFIRSTPRIV] = iw_softap_version,
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
		iw_get_channel_list,
	[QCSAP_IOCTL_GET_STA_INFO - SIOCIWFIRSTPRIV] =
		iw_softap_get_sta_info,
	[QCSAP_IOCTL_PRIV_GET_SOFTAP_LINK_SPEED -
	 SIOCIWFIRSTPRIV] =
		iw_get_softap_linkspeed,
	[QCSAP_IOCTL_PRIV_GET_RSSI - SIOCIWFIRSTPRIV] =
		iw_get_peer_rssi,
	[QCSAP_IOCTL_SET_TX_POWER - SIOCIWFIRSTPRIV] =
		iw_softap_set_tx_power,
	[QCSAP_IOCTL_SET_MAX_TX_POWER - SIOCIWFIRSTPRIV] =
		iw_softap_set_max_tx_power,
	[QCSAP_IOCTL_SET_PKTLOG - SIOCIWFIRSTPRIV] =
		iw_softap_set_pktlog,
	[QCSAP_IOCTL_SET_INI_CFG - SIOCIWFIRSTPRIV] =
		iw_softap_set_ini_cfg,
	[QCSAP_IOCTL_GET_INI_CFG - SIOCIWFIRSTPRIV] =
		iw_softap_get_ini_cfg,
	[QCSAP_IOCTL_SET_TWO_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_softap_set_two_ints_getnone,
};

const struct iw_handler_def hostapd_handler_def = {
	.num_standard = QDF_ARRAY_SIZE(hostapd_handler),
	.num_private = QDF_ARRAY_SIZE(hostapd_private),
	.num_private_args = QDF_ARRAY_SIZE(hostapd_private_args),
	.standard = (iw_handler *) hostapd_handler,
	.private = (iw_handler *) hostapd_private,
	.private_args = hostapd_private_args,
	.get_wireless_stats = NULL,
};

const struct net_device_ops net_ops_struct = {
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

void hdd_set_ap_ops(struct net_device *dev)
{
	dev->netdev_ops = &net_ops_struct;
}

bool hdd_sap_create_ctx(struct hdd_adapter *adapter)
{
	hdd_debug("creating sap context");
	adapter->session.ap.sap_context = sap_create_ctx();
	if (adapter->session.ap.sap_context)
		return true;

	return false;
}

bool hdd_sap_destroy_ctx(struct hdd_adapter *adapter)
{
	hdd_debug("destroying sap context");
	sap_destroy_ctx(adapter->session.ap.sap_context);
	adapter->session.ap.sap_context = NULL;

	return true;
}

QDF_STATUS hdd_init_ap_mode(struct hdd_adapter *adapter, bool reinit)
{
	struct hdd_hostapd_state *phostapdBuf;
	struct net_device *dev = adapter->dev;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sap_context *sapContext = NULL;
	int ret;
	enum dfs_mode acs_dfs_mode;

	ENTER();

	hdd_info("SSR in progress: %d", reinit);

	sapContext = hdd_hostapd_init_sap_session(adapter);
	if (!sapContext) {
		hdd_err("Invalid sap_ctx");
		goto error_release_vdev;
	}

	if (!reinit) {
		adapter->session.ap.sap_config.channel =
			hdd_ctx->acs_policy.acs_channel;
		acs_dfs_mode = hdd_ctx->acs_policy.acs_dfs_mode;
		adapter->session.ap.sap_config.acs_dfs_mode =
			wlan_hdd_get_dfs_mode(acs_dfs_mode);
	}

	/* Allocate the Wireless Extensions state structure */
	phostapdBuf = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	sme_set_curr_device_mode(hdd_ctx->hHal, adapter->device_mode);

	/* Zero the memory.  This zeros the profile structure. */
	memset(phostapdBuf, 0, sizeof(struct hdd_hostapd_state));

	status = qdf_event_create(&phostapdBuf->qdf_event);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Hostapd HDD qdf event init failed!!");
		goto error_release_sap_session;
	}

	status = qdf_event_create(&phostapdBuf->qdf_stop_bss_event);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Hostapd HDD stop bss event init failed!!");
		goto error_release_sap_session;
	}

	status = qdf_event_create(&phostapdBuf->qdf_sta_disassoc_event);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Hostapd HDD sta disassoc event init failed!!");
		goto error_release_sap_session;
	}


	/* Register as a wireless device */
	dev->wireless_handlers = (struct iw_handler_def *)&hostapd_handler_def;

	/* Initialize the data path module */
	status = hdd_softap_init_tx_rx(adapter);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("hdd_softap_init_tx_rx failed");
		goto error_release_sap_session;
	}

	status = hdd_wmm_adapter_init(adapter);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("hdd_wmm_adapter_init() failed code: %08d [x%08x]",
		       status, status);
		goto error_release_wmm;
	}

	set_bit(WMM_INIT_DONE, &adapter->event_flags);

	ret = wma_cli_set_command(adapter->session_id,
				  WMI_PDEV_PARAM_BURST_ENABLE,
				  hdd_ctx->config->enableSifsBurst,
				  PDEV_CMD);

	if (0 != ret)
		hdd_err("WMI_PDEV_PARAM_BURST_ENABLE set failed: %d", ret);

	if (!reinit) {
		adapter->session.ap.sap_config.acs_cfg.acs_mode = false;
		wlan_hdd_undo_acs(adapter);
		qdf_mem_zero(&adapter->session.ap.sap_config.acs_cfg,
			     sizeof(struct sap_acs_cfg));
	}

	/* rcpi info initialization */
	qdf_mem_zero(&adapter->rcpi, sizeof(adapter->rcpi));

	EXIT();

	return status;

error_release_wmm:
	hdd_softap_deinit_tx_rx(adapter);
error_release_sap_session:
	hdd_hostapd_deinit_sap_session(adapter);
error_release_vdev:
	QDF_BUG(!hdd_vdev_destroy(adapter));

	EXIT();
	return status;
}

void hdd_deinit_ap_mode(struct hdd_context *hdd_ctx,
			struct hdd_adapter *adapter,
			bool rtnl_held)
{
	ENTER_DEV(adapter->dev);

	if (test_bit(WMM_INIT_DONE, &adapter->event_flags)) {
		hdd_wmm_adapter_close(adapter);
		clear_bit(WMM_INIT_DONE, &adapter->event_flags);
	}
	wlan_hdd_undo_acs(adapter);
	hdd_softap_deinit_tx_rx(adapter);
	/*
	 * if we are being called during driver unload,
	 * then the dev has already been invalidated.
	 * if we are being called at other times, then we can
	 * detach the wireless device handlers
	 */
	if (adapter->dev) {
		if (rtnl_held) {
			adapter->dev->wireless_handlers = NULL;
		} else {
			rtnl_lock();
			adapter->dev->wireless_handlers = NULL;
			rtnl_unlock();
		}
	}
	if (hdd_hostapd_deinit_sap_session(adapter))
		hdd_err("Failed:hdd_hostapd_deinit_sap_session");

	EXIT();
}

/**
 * hdd_wlan_create_ap_dev() - create an AP-mode device
 * @hdd_ctx: Global HDD context
 * @macAddr: MAC address to assign to the interface
 * @name_assign_type: the name of assign type of the netdev
 * @iface_name: User-visible name of the interface
 *
 * This function will allocate a Linux net_device and configuration it
 * for an AP mode of operation.  Note that the device is NOT actually
 * registered with the kernel at this time.
 *
 * Return: A pointer to the private data portion of the net_device if
 * the allocation and initialization was successful, NULL otherwise.
 */
struct hdd_adapter *hdd_wlan_create_ap_dev(struct hdd_context *hdd_ctx,
				      tSirMacAddr macAddr,
				      unsigned char name_assign_type,
				      uint8_t *iface_name)
{
	struct net_device *dev;
	struct hdd_adapter *adapter;
	QDF_STATUS qdf_status;

	hdd_debug("iface_name = %s", iface_name);

	dev = alloc_netdev_mq(sizeof(struct hdd_adapter), iface_name,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)) || defined(WITH_BACKPORTS)
					  name_assign_type,
#endif
					  ether_setup, NUM_TX_QUEUES);

	if (!dev)
		return NULL;

	adapter = netdev_priv(dev);

	/* Init the net_device structure */
	ether_setup(dev);

	/* Initialize the adapter context to zeros. */
	qdf_mem_zero(adapter, sizeof(struct hdd_adapter));
	adapter->dev = dev;
	adapter->hdd_ctx = hdd_ctx;
	adapter->magic = WLAN_HDD_ADAPTER_MAGIC;
	adapter->session_id = HDD_SESSION_ID_INVALID;

	hdd_debug("dev = %pK, adapter = %pK, concurrency_mode=0x%x",
		dev, adapter,
		(int)policy_mgr_get_concurrency_mode(hdd_ctx->hdd_psoc));

	/* Init the net_device structure */
	strlcpy(dev->name, (const char *)iface_name, IFNAMSIZ);

	hdd_set_ap_ops(dev);

	dev->watchdog_timeo = HDD_TX_TIMEOUT;
	dev->mtu = HDD_DEFAULT_MTU;
	dev->tx_queue_len = HDD_NETDEV_TX_QUEUE_LEN;

	if (hdd_ctx->config->enable_ip_tcp_udp_checksum_offload)
		dev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	dev->features |= NETIF_F_RXCSUM;

	qdf_mem_copy(dev->dev_addr, (void *)macAddr,
		     sizeof(tSirMacAddr));
	qdf_mem_copy(adapter->mac_addr.bytes,
		     (void *)macAddr, sizeof(tSirMacAddr));

	adapter->offloads_configured = false;
	hdd_dev_setup_destructor(dev);
	dev->ieee80211_ptr = &adapter->wdev;
	adapter->wdev.wiphy = hdd_ctx->wiphy;
	adapter->wdev.netdev = dev;
	hdd_set_tso_flags(hdd_ctx, dev);

	qdf_status = qdf_event_create(
			&adapter->qdf_session_open_event);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("failed to create session open QDF event!");
		free_netdev(adapter->dev);
		return NULL;
	}

	qdf_status = qdf_event_create(
			&adapter->qdf_session_close_event);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("failed to create session close QDF event!");
		free_netdev(adapter->dev);
		return NULL;
	}

	init_completion(&adapter->tx_action_cnf_event);
	init_completion(&adapter->cancel_rem_on_chan_var);
	init_completion(&adapter->rem_on_chan_ready_event);
	init_completion(&adapter->sta_authorized_event);
	init_completion(&adapter->offchannel_tx_event);

	SET_NETDEV_DEV(dev, hdd_ctx->parent_dev);
	spin_lock_init(&adapter->pause_map_lock);
	adapter->start_time = adapter->last_time = qdf_system_ticks();

	qdf_atomic_init(&adapter->dfs_radar_found);

	return adapter;
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
 * @adapter: Pointer to hostapd adapter
 *
 * Return: HT support channel width config value
 */
static bool wlan_hdd_get_sap_obss(struct hdd_adapter *adapter)
{
	uint8_t ht_cap_ie[DOT11F_IE_HTCAPS_MAX_LEN];
	tDot11fIEHTCaps dot11_ht_cap_ie = {0};
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_beacon_data *beacon = adapter->session.ap.beacon;
	const uint8_t *ie = NULL;

	ie = wlan_get_ie_ptr_from_eid(WLAN_EID_HT_CAPABILITY,
					beacon->tail, beacon->tail_len);
	if (ie && ie[1]) {
		qdf_mem_copy(ht_cap_ie, &ie[2], DOT11F_IE_HTCAPS_MAX_LEN);
		dot11f_unpack_ie_ht_caps((tpAniSirGlobal)hdd_ctx->hHal,
					ht_cap_ie, ie[1], &dot11_ht_cap_ie,
					false);
		return dot11_ht_cap_ie.supportedChannelWidthSet;
	}

	return false;
}
#else
static bool wlan_hdd_get_sap_obss(struct hdd_adapter *adapter)
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
int wlan_hdd_set_channel(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_chan_def *chandef,
				enum nl80211_channel_type channel_type)
{
	struct hdd_adapter *adapter = NULL;
	uint32_t num_ch = 0;
	int channel = 0;
	int channel_seg2 = 0;
	struct hdd_context *hdd_ctx;
	int status;

	tSmeConfigParams *sme_config;
	tsap_Config_t *sap_config;

	ENTER();


	if (NULL == dev) {
		hdd_err("Called with dev = NULL");
		return -ENODEV;
	}
	adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_CHANNEL,
			 adapter->session_id, channel_type));

	hdd_debug("Device_mode %s(%d)  freq = %d",
	       hdd_device_mode_to_string(adapter->device_mode),
	       adapter->device_mode, chandef->chan->center_freq);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;


	/*
	 * Do freq to chan conversion
	 * TODO: for 11a
	 */

	channel = ieee80211_frequency_to_channel(chandef->chan->center_freq);

	if (NL80211_CHAN_WIDTH_80P80 == chandef->width ||
	    NL80211_CHAN_WIDTH_160 == chandef->width) {
		if (chandef->center_freq2)
			channel_seg2 = ieee80211_frequency_to_channel(
					chandef->center_freq2);
		else
			hdd_err("Invalid center_freq2");
	}

	/* Check freq range */
	if ((WNI_CFG_CURRENT_CHANNEL_STAMIN > channel) ||
	    (WNI_CFG_CURRENT_CHANNEL_STAMAX < channel)) {
		hdd_err("Channel: %d is outside valid range from %d to %d",
		       channel, WNI_CFG_CURRENT_CHANNEL_STAMIN,
		       WNI_CFG_CURRENT_CHANNEL_STAMAX);
		return -EINVAL;
	}

	/* Check freq range */

	if ((WNI_CFG_CURRENT_CHANNEL_STAMIN > channel_seg2) ||
	    (WNI_CFG_CURRENT_CHANNEL_STAMAX < channel_seg2)) {
		hdd_err("Channel: %d is outside valid range from %d to %d",
		       channel_seg2, WNI_CFG_CURRENT_CHANNEL_STAMIN,
		       WNI_CFG_CURRENT_CHANNEL_STAMAX);
		return -EINVAL;
	}

	num_ch = WNI_CFG_VALID_CHANNEL_LIST_LEN;

	if ((QDF_SAP_MODE != adapter->device_mode) &&
	    (QDF_P2P_GO_MODE != adapter->device_mode)) {
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_validate_operation_channel(adapter, channel)) {
			hdd_err("Invalid Channel: %d", channel);
			return -EINVAL;
		}
		hdd_debug("set channel to [%d] for device mode %s(%d)",
		       channel,
		       hdd_device_mode_to_string(adapter->device_mode),
		       adapter->device_mode);
	}

	if ((adapter->device_mode == QDF_STA_MODE)
	    || (adapter->device_mode == QDF_P2P_CLIENT_MODE)
	    ) {
		struct hdd_wext_state *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
		tCsrRoamProfile *roam_profile = &pWextState->roamProfile;
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		if (eConnectionState_IbssConnected ==
		    sta_ctx->conn_info.connState) {
			/* Link is up then return cant set channel */
			hdd_err("IBSS Associated, can't set the channel");
			return -EINVAL;
		}

		num_ch = roam_profile->ChannelInfo.numOfChannels = 1;
		sta_ctx->conn_info.operationChannel = channel;
		roam_profile->ChannelInfo.ChannelList =
			&sta_ctx->conn_info.operationChannel;
	} else if ((adapter->device_mode == QDF_SAP_MODE)
		   || (adapter->device_mode == QDF_P2P_GO_MODE)
		   ) {
		sap_config = &((WLAN_HDD_GET_AP_CTX_PTR(adapter))->sap_config);
		if (QDF_P2P_GO_MODE == adapter->device_mode) {
			if (QDF_STATUS_SUCCESS !=
			    wlan_hdd_validate_operation_channel(adapter,
								channel)) {
				hdd_err("Invalid Channel: %d", channel);
				return -EINVAL;
			}
			sap_config->channel = channel;
			sap_config->ch_params.center_freq_seg1 = channel_seg2;
		} else {
			/* set channel to what hostapd configured */
			if (QDF_STATUS_SUCCESS !=
				wlan_hdd_validate_operation_channel(adapter,
								channel)) {
				hdd_err("Invalid Channel: %d", channel);
				return -EINVAL;
			}

			sap_config->channel = channel;
			sap_config->ch_params.center_freq_seg1 = channel_seg2;

			sme_config = qdf_mem_malloc(sizeof(*sme_config));

			if (!sme_config) {
				hdd_err("Unable to allocate memory for smeconfig!");
				return -ENOMEM;
			}
			sme_get_config_param(hdd_ctx->hHal, sme_config);
			switch (channel_type) {
			case NL80211_CHAN_HT20:
			case NL80211_CHAN_NO_HT:
				sme_config->csrConfig.obssEnabled = false;
				sap_config->sec_ch = 0;
				break;
			case NL80211_CHAN_HT40MINUS:
				sap_config->sec_ch = sap_config->channel - 4;
				break;
			case NL80211_CHAN_HT40PLUS:
				sap_config->sec_ch = sap_config->channel + 4;
				break;
			default:
				hdd_err("Error!!! Invalid HT20/40 mode !");
				qdf_mem_free(sme_config);
				return -EINVAL;
			}
			sme_config->csrConfig.obssEnabled =
				wlan_hdd_get_sap_obss(adapter);

			sme_update_config(hdd_ctx->hHal, sme_config);
			qdf_mem_free(sme_config);
		}
	} else {
		hdd_err("Invalid device mode failed to set valid channel");
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
static void wlan_hdd_check_11gmode(const u8 *pIe, u8 *require_ht,
				   u8 *require_vht, u8 *pCheckRatesfor11g,
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
}

#ifdef WLAN_FEATURE_11AX
/**
 * wlan_hdd_add_extn_ie() - add extension IE
 * @adapter: Pointer to hostapd adapter
 * @genie: Pointer to ie to be added
 * @total_ielen: Pointer to store total ie length
 * @oui: Pointer to oui
 * @oui_size: Size of oui
 *
 * Return: 0 for success non-zero for failure
 */
static int wlan_hdd_add_extn_ie(struct hdd_adapter *adapter, uint8_t *genie,
			   uint16_t *total_ielen, uint8_t *oui,
			   uint8_t oui_size)
{
	const uint8_t *ie;
	uint16_t ielen = 0;
	struct hdd_beacon_data *beacon = adapter->session.ap.beacon;

	ie = wlan_get_ext_ie_ptr_from_ext_id(oui, oui_size,
					      beacon->tail,
					      beacon->tail_len);
	if (ie) {
		ielen = ie[1] + 2;
		if ((*total_ielen + ielen) <= MAX_GENIE_LEN) {
			qdf_mem_copy(&genie[*total_ielen], ie, ielen);
		} else {
			hdd_err("**Ie Length is too big***");
			return -EINVAL;
		}
		*total_ielen += ielen;
	}
	return 0;
}
#endif

/**
 * wlan_hdd_add_hostapd_conf_vsie() - configure Vendor IE in sap mode
 * @adapter: Pointer to hostapd adapter
 * @genie: Pointer to Vendor IE
 * @total_ielen: Pointer to store total ie length
 *
 * Return: none
 */
static void wlan_hdd_add_hostapd_conf_vsie(struct hdd_adapter *adapter,
					   uint8_t *genie,
					   uint16_t *total_ielen)
{
	struct hdd_beacon_data *pBeacon = adapter->session.ap.beacon;
	int left = pBeacon->tail_len;
	uint8_t *ptr = pBeacon->tail;
	uint8_t elem_id, elem_len;
	uint16_t ielen = 0;
	bool skip_ie;

	if (NULL == ptr || 0 == left)
		return;

	while (left >= 2) {
		elem_id = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left) {
			hdd_err("**Invalid IEs eid: %d elem_len: %d left: %d**",
				elem_id, elem_len, left);
			return;
		}
		if (IE_EID_VENDOR == elem_id) {
			/*
			 * skipping the Vendor IE's which we don't want to
			 * include or it will be included by existing code.
			 */
			if (elem_len >= WPS_OUI_TYPE_SIZE &&
			    (!qdf_mem_cmp(&ptr[2], WHITELIST_OUI_TYPE,
					  WPA_OUI_TYPE_SIZE) ||
			     !qdf_mem_cmp(&ptr[2], BLACKLIST_OUI_TYPE,
					  WPA_OUI_TYPE_SIZE) ||
			     !qdf_mem_cmp(&ptr[2], "\x00\x50\xf2\x02",
					  WPA_OUI_TYPE_SIZE) ||
			     !qdf_mem_cmp(&ptr[2], WPA_OUI_TYPE,
					  WPA_OUI_TYPE_SIZE)))
				skip_ie = true;
			else
				skip_ie = false;

			if (!skip_ie) {
				ielen = ptr[1] + 2;
				if ((*total_ielen + ielen) <= MAX_GENIE_LEN) {
					qdf_mem_copy(&genie[*total_ielen], ptr,
						     ielen);
					*total_ielen += ielen;
				} else {
					hdd_err("IE Length is too big IEs eid: %d elem_len: %d total_ie_lent: %d",
					       elem_id, elem_len, *total_ielen);
				}
			}
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
}

/**
 * wlan_hdd_add_extra_ie() - add extra ies in beacon
 * @adapter: Pointer to hostapd adapter
 * @genie: Pointer to extra ie
 * @total_ielen: Pointer to store total ie length
 * @temp_ie_id: ID of extra ie
 *
 * Return: none
 */
static void wlan_hdd_add_extra_ie(struct hdd_adapter *adapter,
				  uint8_t *genie, uint16_t *total_ielen,
				  uint8_t temp_ie_id)
{
	struct hdd_beacon_data *pBeacon = adapter->session.ap.beacon;
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
			hdd_err("**Invalid IEs eid: %d elem_len: %d left: %d**",
			       elem_id, elem_len, left);
			return;
		}

		if (temp_ie_id == elem_id) {
			ielen = ptr[1] + 2;
			if ((*total_ielen + ielen) <= MAX_GENIE_LEN) {
				qdf_mem_copy(&genie[*total_ielen], ptr, ielen);
				*total_ielen += ielen;
			} else {
				hdd_err("IE Length is too big IEs eid: %d elem_len: %d total_ie_len: %d",
				       elem_id, elem_len, *total_ielen);
			}
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
}

/**
 * wlan_hdd_cfg80211_alloc_new_beacon() - alloc beacon in ap mode
 * @adapter: Pointer to hostapd adapter
 * @ppBeacon: Pointer to pointer to beacon data
 * @params: Pointer to beacon parameters
 * @dtim_period: DTIM period
 *
 * Return: 0 for success non-zero for failure
 */
static int
wlan_hdd_cfg80211_alloc_new_beacon(struct hdd_adapter *adapter,
				   struct hdd_beacon_data **ppBeacon,
				   struct cfg80211_beacon_data *params,
				   int dtim_period)
{
	int size;
	struct hdd_beacon_data *beacon = NULL;
	struct hdd_beacon_data *old = NULL;
	int head_len, tail_len, proberesp_ies_len, assocresp_ies_len;
	const u8 *head, *tail, *proberesp_ies, *assocresp_ies;

	ENTER();
	if (params->head && !params->head_len) {
		hdd_err("head_len is NULL");
		return -EINVAL;
	}

	old = adapter->session.ap.beacon;

	if (!params->head && !old) {
		hdd_err("session: %d old and new heads points to NULL",
		       adapter->session_id);
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

	size = sizeof(struct hdd_beacon_data) + head_len + tail_len +
		proberesp_ies_len + assocresp_ies_len;

	beacon = qdf_mem_malloc(size);

	if (beacon == NULL) {
		hdd_err("Mem allocation for beacon failed");
		return -ENOMEM;
	}
	if (dtim_period)
		beacon->dtim_period = dtim_period;
	else if (old)
		beacon->dtim_period = old->dtim_period;
	/* -----------------------------------------------
	 * | head | tail | proberesp_ies | assocresp_ies |
	 * -----------------------------------------------
	 */
	beacon->head = ((u8 *) beacon) + sizeof(struct hdd_beacon_data);
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

	adapter->session.ap.beacon = NULL;
	qdf_mem_free(old);

	return 0;

}

#ifdef QCA_HT_2040_COEX
static void wlan_hdd_add_sap_obss_scan_ie(
	struct hdd_adapter *hostapd_adapter, uint8_t *ie_buf, uint16_t *ie_len)
{
	if (QDF_SAP_MODE == hostapd_adapter->device_mode) {
		if (wlan_hdd_get_sap_obss(hostapd_adapter))
			wlan_hdd_add_extra_ie(hostapd_adapter, ie_buf, ie_len,
					WLAN_EID_OVERLAP_BSS_SCAN_PARAM);
	}
}
#else
static void wlan_hdd_add_sap_obss_scan_ie(
	struct hdd_adapter *hostapd_adapter, uint8_t *ie_buf, uint16_t *ie_len)
{
}
#endif

/**
 * wlan_hdd_cfg80211_update_apies() - update ap mode 11ax ies
 * @adapter: Pointer to hostapd adapter
 * @genie: generic IE buffer
 * @total_ielen: out param to update total ielen
 *
 * Return: 0 for success non-zero for failure
 */

#ifdef WLAN_FEATURE_11AX
static int hdd_update_11ax_apies(struct hdd_adapter *adapter,
				 uint8_t *genie, uint16_t *total_ielen)
{
	if (wlan_hdd_add_extn_ie(adapter, genie, total_ielen,
			    HE_CAP_OUI_TYPE, HE_CAP_OUI_SIZE)) {
		hdd_err("Adding HE Cap ie failed");
		return -EINVAL;
	}

	if (wlan_hdd_add_extn_ie(adapter, genie, total_ielen,
			    HE_OP_OUI_TYPE, HE_OP_OUI_SIZE)) {
		hdd_err("Adding HE Op ie failed");
		return -EINVAL;
	}

	return 0;
}
#else
static int hdd_update_11ax_apies(struct hdd_adapter *adapter,
				 uint8_t *genie, uint16_t *total_ielen)
{
	return 0;
}
#endif

/**
 * wlan_hdd_cfg80211_update_apies() - update ap mode ies
 * @adapter: Pointer to hostapd adapter
 *
 * Return: 0 for success non-zero for failure
 */
int wlan_hdd_cfg80211_update_apies(struct hdd_adapter *adapter)
{
	uint8_t *genie;
	uint16_t total_ielen = 0;
	int ret = 0;
	tsap_Config_t *pConfig;
	tSirUpdateIE updateIE;
	struct hdd_beacon_data *beacon = NULL;
	uint16_t proberesp_ies_len;
	uint8_t *proberesp_ies = NULL;

	pConfig = &adapter->session.ap.sap_config;
	beacon = adapter->session.ap.beacon;

	genie = qdf_mem_malloc(MAX_GENIE_LEN);

	if (genie == NULL)
		return -ENOMEM;

	wlan_hdd_add_extra_ie(adapter, genie, &total_ielen,
			      WLAN_EID_VHT_TX_POWER_ENVELOPE);

	/* Extract and add the extended capabilities and interworking IE */
	wlan_hdd_add_extra_ie(adapter, genie, &total_ielen,
			      WLAN_EID_EXT_CAPABILITY);

	wlan_hdd_add_extra_ie(adapter, genie, &total_ielen,
			      WLAN_EID_INTERWORKING);

#ifdef FEATURE_WLAN_WAPI
	if (QDF_SAP_MODE == adapter->device_mode) {
		wlan_hdd_add_extra_ie(adapter, genie, &total_ielen,
				      WLAN_EID_WAPI);
	}
#endif

	wlan_hdd_add_hostapd_conf_vsie(adapter, genie,
				       &total_ielen);

	ret = hdd_update_11ax_apies(adapter, genie, &total_ielen);
	if (ret)
		goto done;

	wlan_hdd_add_sap_obss_scan_ie(adapter, genie, &total_ielen);

	qdf_copy_macaddr(&updateIE.bssid, &adapter->mac_addr);
	updateIE.smeSessionId = adapter->session_id;

	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		updateIE.ieBufferlength = total_ielen;
		updateIE.pAdditionIEBuffer = genie;
		updateIE.append = false;
		updateIE.notify = true;
		if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
				      &updateIE,
				      eUPDATE_IE_PROBE_BCN) ==
		    QDF_STATUS_E_FAILURE) {
			hdd_err("Could not pass on Add Ie probe beacon data");
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
	proberesp_ies = qdf_mem_malloc(beacon->proberesp_ies_len +
				      MAX_GENIE_LEN);
	if (proberesp_ies == NULL) {
		hdd_err("mem alloc failed for probe resp ies, size: %d",
			beacon->proberesp_ies_len + MAX_GENIE_LEN);
		ret = -EINVAL;
		goto done;
	}
	qdf_mem_copy(proberesp_ies, beacon->proberesp_ies,
		    beacon->proberesp_ies_len);
	proberesp_ies_len = beacon->proberesp_ies_len;

	wlan_hdd_add_sap_obss_scan_ie(adapter, proberesp_ies,
				     &proberesp_ies_len);

	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		updateIE.ieBufferlength = proberesp_ies_len;
		updateIE.pAdditionIEBuffer = proberesp_ies;
		updateIE.append = false;
		updateIE.notify = false;
		if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
				      &updateIE,
				      eUPDATE_IE_PROBE_RESP) ==
		    QDF_STATUS_E_FAILURE) {
			hdd_err("Could not pass on PROBE_RESP add Ie data");
			ret = -EINVAL;
			goto done;
		}
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_PROBE_RESP);
	} else {
		wlansap_update_sap_config_add_ie(pConfig,
						 proberesp_ies,
						 proberesp_ies_len,
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
		    QDF_STATUS_E_FAILURE) {
			hdd_err("Could not pass on Add Ie Assoc Response data");
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
	qdf_mem_free(genie);
	qdf_mem_free(proberesp_ies);
	return ret;
}

/**
 * wlan_hdd_set_sap_hwmode() - set sap hw mode
 * @adapter: Pointer to hostapd adapter
 *
 * Return: none
 */
static void wlan_hdd_set_sap_hwmode(struct hdd_adapter *adapter)
{
	tsap_Config_t *pConfig = &adapter->session.ap.sap_config;
	struct hdd_beacon_data *pBeacon = adapter->session.ap.beacon;
	struct ieee80211_mgmt *pMgmt_frame =
		(struct ieee80211_mgmt *)pBeacon->head;
	u8 checkRatesfor11g = true;
	u8 require_ht = false, require_vht = false;
	const u8 *pIe = NULL;

	pConfig->SapHw_mode = eCSR_DOT11_MODE_11b;

	pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_HT_CAPABILITY,
				       &pMgmt_frame->u.beacon.variable[0],
				       pBeacon->head_len);
	if (pIe != NULL) {
		pIe += 1;
		wlan_hdd_check_11gmode(pIe, &require_ht, &require_vht,
			&checkRatesfor11g, &pConfig->SapHw_mode);
	}

	pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_EXT_SUPP_RATES,
					pBeacon->tail, pBeacon->tail_len);
	if (pIe != NULL) {
		pIe += 1;
		wlan_hdd_check_11gmode(pIe, &require_ht, &require_vht,
			&checkRatesfor11g, &pConfig->SapHw_mode);
	}

	if (pConfig->channel > 14)
		pConfig->SapHw_mode = eCSR_DOT11_MODE_11a;

	pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_HT_CAPABILITY,
					pBeacon->tail, pBeacon->tail_len);
	if (pIe) {
		pConfig->SapHw_mode = eCSR_DOT11_MODE_11n;
		if (require_ht)
			pConfig->SapHw_mode = eCSR_DOT11_MODE_11n_ONLY;
	}

	pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_VHT_CAPABILITY,
					pBeacon->tail, pBeacon->tail_len);
	if (pIe) {
		pConfig->SapHw_mode = eCSR_DOT11_MODE_11ac;
		if (require_vht)
			pConfig->SapHw_mode = eCSR_DOT11_MODE_11ac_ONLY;
	}

	wlan_hdd_check_11ax_support(pBeacon, pConfig);

	hdd_info("SAP hw_mode: %d", pConfig->SapHw_mode);
}

/**
 * wlan_hdd_config_acs() - config ACS needed parameters
 * @hdd_ctx: HDD context
 * @adapter: Adapter pointer
 *
 * This function get ACS related INI paramters and populated
 * sap config and smeConfig for ACS needed configurations.
 *
 * Return: The QDF_STATUS code associated with performing the operation.
 */
QDF_STATUS wlan_hdd_config_acs(struct hdd_context *hdd_ctx, struct hdd_adapter *adapter)
{
	tsap_Config_t *sap_config;
	struct hdd_config *ini_config;
	tHalHandle hal;

	hal = WLAN_HDD_GET_HAL_CTX(adapter);
	sap_config = &adapter->session.ap.sap_config;
	ini_config = hdd_ctx->config;

	sap_config->enOverLapCh = !!hdd_ctx->config->gEnableOverLapCh;

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	hdd_debug("HDD_ACS_SKIP_STATUS = %d",
						hdd_ctx->skip_acs_scan_status);
	if (hdd_ctx->skip_acs_scan_status == eSAP_SKIP_ACS_SCAN) {
		struct hdd_adapter *con_sap_adapter;
		tsap_Config_t *con_sap_config = NULL;

		con_sap_adapter = hdd_get_con_sap_adapter(adapter, false);

		if (con_sap_adapter)
			con_sap_config =
				&con_sap_adapter->session.ap.sap_config;

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

			wlansap_extend_to_acs_range(hal, &cur_sap_st_ch,
					&cur_sap_end_ch, &bandStartChannel,
					&bandEndChannel);

			wlansap_extend_to_acs_range(hal,
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


			hdd_debug(FL(
				"SecAP ACS Skip=%d, ACS CH RANGE=%d-%d, %d-%d"),
				sap_config->acs_cfg.skip_scan_status,
				sap_config->acs_cfg.skip_scan_range1_stch,
				sap_config->acs_cfg.skip_scan_range1_endch,
				sap_config->acs_cfg.skip_scan_range2_stch,
				sap_config->acs_cfg.skip_scan_range2_endch);
		}
	}
#endif

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_sap_p2p_11ac_overrides: API to overwrite 11ac config in case of
 * SAP or p2p go
 * @ap_adapter: pointer to adapter
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
static int wlan_hdd_sap_p2p_11ac_overrides(struct hdd_adapter *ap_adapter)
{
	tsap_Config_t *sap_cfg = &ap_adapter->session.ap.sap_config;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);

	/* Fixed channel 11AC override:
	 * 11AC override in qcacld is introduced for following reasons:
	 * 1. P2P GO also follows start_bss and since p2p GO could not be
	 *    configured to setup VHT channel width in wpa_supplicant
	 * 2. Android UI does not provide advanced configuration options for SAP
	 *
	 * Default override enabled (for android). MDM shall disable this in ini
	 */
	/*
	 * sub_20 MHz channel width is incompatible with 11AC rates, hence do
	 * not allow 11AC rates or more than 20 MHz channel width when
	 * enable_sub_20_channel_width is non zero
	 */
	if (!hdd_ctx->config->enable_sub_20_channel_width &&
			hdd_ctx->config->sap_p2p_11ac_override &&
			(sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11n ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac_ONLY ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ax ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ax_ONLY) &&
			!(((ap_adapter->device_mode == QDF_SAP_MODE) &&
			(hdd_ctx->config->sap_force_11n_for_11ac)) ||
			((ap_adapter->device_mode == QDF_P2P_GO_MODE) &&
			(hdd_ctx->config->go_force_11n_for_11ac)))) {
		hdd_debug("** Driver force 11AC override for SAP/Go **");

		/* 11n only shall not be overridden since it may be on purpose*/
		if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11n)
			sap_cfg->SapHw_mode = eCSR_DOT11_MODE_11ac;

		if (sap_cfg->channel >= 36) {
			sap_cfg->ch_width_orig =
					hdd_ctx->config->vhtChannelWidth;
		} else {
			/*
			 * Allow 40 Mhz in 2.4 Ghz only if indicated by
			 * supplicant after OBSS scan and if 2.4 Ghz channel
			 * bonding is set in INI
			 */
			if (sap_cfg->ch_width_orig >= eHT_CHANNEL_WIDTH_40MHZ &&
			   hdd_ctx->config->nChannelBondingMode24GHz)
				sap_cfg->ch_width_orig =
					eHT_CHANNEL_WIDTH_40MHZ;
			else
				sap_cfg->ch_width_orig =
					eHT_CHANNEL_WIDTH_20MHZ;
		}
	}

	sap_cfg->ch_params.ch_width = sap_cfg->ch_width_orig;
	wlan_reg_set_channel_params(hdd_ctx->hdd_pdev, sap_cfg->channel,
				sap_cfg->sec_ch, &sap_cfg->ch_params);

	return 0;
}

/**
 * wlan_hdd_setup_acs_overrides : Overrides ACS configurations
 * @adapter: pointer to adapter struct
 *
 * This function overrides ACS configuration based on driver INI
 * parameters. These overrides are done to support android legacy
 * configuration method.
 *
 * NOTE: Non android platform supports concurrency and these overrides shall
 * not be used. Also future driver based overrides shall be consolidated in this
 * function only. Avoid random overrides in other location based on ini.
 *
 * Return: 0 for Success or Negative error codes.
 */
static int wlan_hdd_setup_acs_overrides(struct hdd_adapter *ap_adapter)
{
	tsap_Config_t *sap_cfg = &ap_adapter->session.ap.sap_config;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);

	hdd_debug("** Driver force ACS override **");

	sap_cfg->channel = AUTO_CHANNEL_SELECT;
	sap_cfg->acs_cfg.acs_mode = true;
	sap_cfg->acs_cfg.start_ch = hdd_ctx->config->force_sap_acs_st_ch;
	sap_cfg->acs_cfg.end_ch = hdd_ctx->config->force_sap_acs_end_ch;

	if (sap_cfg->acs_cfg.start_ch > sap_cfg->acs_cfg.end_ch) {
		hdd_err("Driver force ACS start ch (%d) > end ch (%d)",
			sap_cfg->acs_cfg.start_ch,  sap_cfg->acs_cfg.end_ch);
		return -EINVAL;
	}

	/* Derive ACS HW mode */
	sap_cfg->SapHw_mode = hdd_cfg_xlate_to_csr_phy_mode(
						hdd_ctx->config->dot11Mode);
	if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_AUTO) {
		if (sme_is_feature_supported_by_fw(DOT11AX))
			sap_cfg->SapHw_mode = eCSR_DOT11_MODE_11ax;
		else
			sap_cfg->SapHw_mode = eCSR_DOT11_MODE_11ac;
	}

	if (((ap_adapter->device_mode == QDF_SAP_MODE) &&
	     (hdd_ctx->config->sap_force_11n_for_11ac)) ||
	     ((ap_adapter->device_mode == QDF_P2P_GO_MODE) &&
	     (hdd_ctx->config->go_force_11n_for_11ac))) {
		if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac ||
		    sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac_ONLY)
			sap_cfg->SapHw_mode = eCSR_DOT11_MODE_11n;
	}

	if ((sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11b ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11g ||
			sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11g_ONLY) &&
			sap_cfg->acs_cfg.start_ch > 14) {
		hdd_err("Invalid ACS HW Mode %d + CH range <%d - %d>",
			sap_cfg->SapHw_mode, sap_cfg->acs_cfg.start_ch,
			sap_cfg->acs_cfg.end_ch);
		return -EINVAL;
	}
	sap_cfg->acs_cfg.hw_mode = sap_cfg->SapHw_mode;

	/* Derive ACS BW */
	sap_cfg->ch_width_orig = eHT_CHANNEL_WIDTH_20MHZ;
	if (sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac ||
	    sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ac_ONLY ||
	    sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ax ||
	    sap_cfg->SapHw_mode == eCSR_DOT11_MODE_11ax_ONLY) {
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

	hdd_debug("Force ACS Config: HW_MODE: %d ACS_BW: %d",
		sap_cfg->acs_cfg.hw_mode, sap_cfg->acs_cfg.ch_width);
	hdd_debug("Force ACS Config: ST_CH: %d END_CH: %d",
		sap_cfg->acs_cfg.start_ch, sap_cfg->acs_cfg.end_ch);

	return 0;
}

/**
 * wlan_hdd_setup_driver_overrides : Overrides SAP / P2P GO Params
 * @adapter: pointer to adapter struct
 *
 * This function overrides SAP / P2P Go configuration based on driver INI
 * parameters for 11AC override and ACS. These overrides are done to support
 * android legacy configuration method.
 *
 * NOTE: Non android platform supports concurrency and these overrides shall
 * not be used. Also future driver based overrides shall be consolidated in this
 * function only. Avoid random overrides in other location based on ini.
 *
 * Return: 0 for Success or Negative error codes.
 */
static int wlan_hdd_setup_driver_overrides(struct hdd_adapter *ap_adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(ap_adapter);

	if (!hdd_ctx->config->vendor_acs_support) {
		if (ap_adapter->device_mode == QDF_SAP_MODE &&
		    hdd_ctx->config->force_sap_acs)
			return wlan_hdd_setup_acs_overrides(ap_adapter);
		else
			return wlan_hdd_sap_p2p_11ac_overrides(ap_adapter);
	} else
		return 0;
}

/**
 * wlan_hdd_cfg80211_start_bss() - start bss
 * @adapter: Pointer to hostapd adapter
 * @params: Pointer to start bss beacon parameters
 * @ssid: Pointer ssid
 * @ssid_len: Length of ssid
 * @hidden_ssid: Hidden SSID parameter
 * @check_for_concurrency: Flag to indicate if check for concurrency is needed
 *
 * Return: 0 for success non-zero for failure
 */
int wlan_hdd_cfg80211_start_bss(struct hdd_adapter *adapter,
				       struct cfg80211_beacon_data *params,
				       const u8 *ssid, size_t ssid_len,
				       enum nl80211_hidden_ssid hidden_ssid,
				       bool check_for_concurrency)
{
	tsap_Config_t *pConfig;
	struct hdd_beacon_data *pBeacon = NULL;
	struct ieee80211_mgmt *pMgmt_frame;
	const uint8_t *pIe = NULL;
	uint16_t capab_info;
	eCsrAuthType RSNAuthType;
	eCsrEncryptionType RSNEncryptType;
	eCsrEncryptionType mcRSNEncryptType;
	int status = QDF_STATUS_SUCCESS, ret;
	int qdf_status = QDF_STATUS_SUCCESS;
	tpWLAN_SAPEventCB pSapEventCallback;
	struct hdd_hostapd_state *hostapd_state;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	struct qc_mac_acl_entry *acl_entry = NULL;
	int32_t i;
	struct hdd_config *iniConfig;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tSmeConfigParams *sme_config;
	bool MFPCapable = false;
	bool MFPRequired = false;
	uint16_t prev_rsn_length = 0;
	enum dfs_mode mode;
	struct hdd_adapter *sta_adapter;

	ENTER();


	hdd_notify_teardown_tdls_links(adapter->hdd_vdev);

	if (policy_mgr_is_hw_mode_change_in_progress(hdd_ctx->hdd_psoc)) {
		status = policy_mgr_wait_for_connection_update(
			hdd_ctx->hdd_psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("qdf wait for event failed!!");
			return -EINVAL;
		}
	}

	/*
	 * For STA+SAP concurrency support from GUI, first STA connection gets
	 * triggered and while it is in progress, SAP start also comes up.
	 * Once STA association is successful, STA connect event is sent to
	 * kernel which gets queued in kernel workqueue and supplicant won't
	 * process M1 received from AP and send M2 until this NL80211_CONNECT
	 * event is received. Workqueue is not scheduled as RTNL lock is already
	 * taken by hostapd thread which has issued start_bss command to driver.
	 * Driver cannot complete start_bss as the pending command at the head
	 * of the SME command pending list is hw_mode_update for STA session
	 * which cannot be processed as SME is in WAITforKey state for STA
	 * interface. The start_bss command for SAP interface is queued behind
	 * the hw_mode_update command and so it cannot be processed until
	 * hw_mode_update command is processed. This is causing a deadlock so
	 * disconnect the STA interface first if connection or key exchange is
	 * in progress and then start SAP interface.
	 */
	sta_adapter = hdd_get_sta_connection_in_progress(hdd_ctx);
	if (sta_adapter) {
		hdd_debug("Disconnecting STA with session id: %d",
			  sta_adapter->session_id);
		wlan_hdd_disconnect(sta_adapter, eCSR_DISCONNECT_REASON_DEAUTH);
	}

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		hdd_err("failed to allocate memory");
		return -ENOMEM;
	}

	iniConfig = hdd_ctx->config;
	hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

	clear_bit(ACS_PENDING, &adapter->event_flags);
	clear_bit(ACS_IN_PROGRESS, &hdd_ctx->g_event_flags);

	pConfig = &adapter->session.ap.sap_config;

	pBeacon = adapter->session.ap.beacon;

	pMgmt_frame = (struct ieee80211_mgmt *)pBeacon->head;

	pConfig->beacon_int = pMgmt_frame->u.beacon.beacon_int;
	pConfig->dfs_cac_offload = hdd_ctx->dfs_cac_offload;

	pConfig->auto_channel_select_weight =
			     iniConfig->auto_channel_select_weight;
	pConfig->disableDFSChSwitch = iniConfig->disableDFSChSwitch;
	pConfig->sap_chanswitch_beacon_cnt =
			    iniConfig->sap_chanswitch_beacon_cnt;
	pConfig->sap_chanswitch_mode = iniConfig->sap_chanswitch_mode;

	/* channel is already set in the set_channel Call back */
	/* pConfig->channel = pCommitConfig->channel; */

	/* Protection parameter to enable or disable */
	pConfig->protEnabled = iniConfig->apProtEnabled;

	pConfig->chan_switch_hostapd_rate_enabled =
		iniConfig->chan_switch_hostapd_rate_enabled;

	if (iniConfig->WlanMccToSccSwitchMode !=
			QDF_MCC_TO_SCC_SWITCH_DISABLE) {
		pConfig->chan_switch_hostapd_rate_enabled = false;
	}

	pConfig->enOverLapCh = iniConfig->gEnableOverLapCh;
	pConfig->dtim_period = pBeacon->dtim_period;
	pConfig->dfs_beacon_tx_enhanced = iniConfig->dfs_beacon_tx_enhanced;
	pConfig->reduced_beacon_interval =
			iniConfig->reduced_beacon_interval;
	hdd_debug("acs_mode %d", pConfig->acs_cfg.acs_mode);

	if (pConfig->acs_cfg.acs_mode == true) {
		hdd_debug("acs_channel %d, acs_dfs_mode %d",
			hdd_ctx->acs_policy.acs_channel,
			hdd_ctx->acs_policy.acs_dfs_mode);

		if (hdd_ctx->acs_policy.acs_channel)
			pConfig->channel = hdd_ctx->acs_policy.acs_channel;
		mode = hdd_ctx->acs_policy.acs_dfs_mode;
		pConfig->acs_dfs_mode = wlan_hdd_get_dfs_mode(mode);
	}

	hdd_debug("pConfig->channel %d, pConfig->acs_dfs_mode %d",
		pConfig->channel, pConfig->acs_dfs_mode);

	hdd_debug("****pConfig->dtim_period=%d***",
		pConfig->dtim_period);

	if (adapter->device_mode == QDF_SAP_MODE) {
		pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_COUNTRY,
					pBeacon->tail, pBeacon->tail_len);
		if (pIe) {
			if (pIe[1] < IEEE80211_COUNTRY_IE_MIN_LEN) {
				hdd_err("Invalid Country IE len: %d", pIe[1]);
				ret = -EINVAL;
				goto error;
			}

			pConfig->ieee80211d = 1;
			qdf_mem_copy(pConfig->countryCode, &pIe[2], 3);
			status = ucfg_reg_set_country(hdd_ctx->hdd_pdev,
					pConfig->countryCode);
			if (QDF_IS_STATUS_ERROR(status)) {
				hdd_err("Failed to set country");
				pConfig->ieee80211d = 0;
			}
		} else {
			pConfig->countryCode[0] = hdd_ctx->reg.alpha2[0];
			pConfig->countryCode[1] = hdd_ctx->reg.alpha2[1];
			pConfig->ieee80211d = 0;
		}

		ret = wlan_hdd_sap_cfg_dfs_override(adapter);
		if (ret < 0) {
			goto error;
		} else {
			if (ret == 0) {
				if (wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev,
							pConfig->channel))
					hdd_ctx->dev_dfs_cac_status =
							DFS_CAC_NEVER_DONE;
			}
		}

		/*
		 * If auto channel is configured i.e. channel is 0,
		 * so skip channel validation.
		 */
		if (AUTO_CHANNEL_SELECT != pConfig->channel) {
			if (QDF_STATUS_SUCCESS !=
			    wlan_hdd_validate_operation_channel(adapter,
							pConfig->channel)) {
				hdd_err("Invalid Channel: %d", pConfig->channel);
				ret = -EINVAL;
				goto error;
			}

			/* reject SAP if DFS channel scan is not allowed */
			if (!(hdd_ctx->config->enableDFSChnlScan) &&
				(CHANNEL_STATE_DFS ==
				 wlan_reg_get_channel_state(hdd_ctx->hdd_pdev,
					 pConfig->channel))) {
				hdd_err("No SAP start on DFS channel");
				ret = -EOPNOTSUPP;
				goto error;
			}
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
	} else if (adapter->device_mode == QDF_P2P_GO_MODE) {
		pConfig->countryCode[0] = hdd_ctx->reg.alpha2[0];
		pConfig->countryCode[1] = hdd_ctx->reg.alpha2[1];
		pConfig->ieee80211d = 0;
	} else {
		pConfig->ieee80211d = 0;
	}

	wlansap_set_tx_leakage_threshold(hHal,
		iniConfig->sap_tx_leakage_threshold);

	capab_info = pMgmt_frame->u.beacon.capab_info;

	pConfig->privacy = (pMgmt_frame->u.beacon.capab_info &
			    WLAN_CAPABILITY_PRIVACY) ? true : false;

	(WLAN_HDD_GET_AP_CTX_PTR(adapter))->privacy = pConfig->privacy;

	/*Set wps station to configured */
	pIe = wlan_hdd_get_wps_ie_ptr(pBeacon->tail, pBeacon->tail_len);

	if (pIe) {
		if (pIe[1] < (2 + WPS_OUI_TYPE_SIZE)) {
			hdd_err("**Wps Ie Length is too small***");
			ret = -EINVAL;
			goto error;
		} else if (memcmp(&pIe[2], WPS_OUI_TYPE, WPS_OUI_TYPE_SIZE) ==
			   0) {
			hdd_debug("** WPS IE(len %d) ***", (pIe[1] + 2));
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
		hdd_debug("WPS disabled");
		pConfig->wps_state = SAP_WPS_DISABLED;
	}
	/* Forward WPS PBC probe request frame up */
	pConfig->fwdWPSPBCProbeReq = 1;

	pConfig->RSNEncryptType = eCSR_ENCRYPT_TYPE_NONE;
	pConfig->mcRSNEncryptType = eCSR_ENCRYPT_TYPE_NONE;
	(WLAN_HDD_GET_AP_CTX_PTR(adapter))->encryption_type =
		eCSR_ENCRYPT_TYPE_NONE;

	pConfig->RSNWPAReqIELength = 0;
	memset(&pConfig->RSNWPAReqIE[0], 0, sizeof(pConfig->RSNWPAReqIE));
	pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_RSN, pBeacon->tail,
				       pBeacon->tail_len);
	if (pIe && pIe[1]) {
		pConfig->RSNWPAReqIELength = pIe[1] + 2;
		if (pConfig->RSNWPAReqIELength < sizeof(pConfig->RSNWPAReqIE))
			memcpy(&pConfig->RSNWPAReqIE[0], pIe,
			       pConfig->RSNWPAReqIELength);
		else
			hdd_err("RSNWPA IE MAX Length exceeded; length =%d",
			       pConfig->RSNWPAReqIELength);
		/* The actual processing may eventually be more extensive than
		 * this. Right now, just consume any PMKIDs that are sent in
		 * by the app.
		 */
		status =
			hdd_softap_unpack_ie(cds_get_context
						     (QDF_MODULE_ID_SME),
					     &RSNEncryptType, &mcRSNEncryptType,
					     &RSNAuthType, &MFPCapable,
					     &MFPRequired,
					     pConfig->RSNWPAReqIE[1] + 2,
					     pConfig->RSNWPAReqIE);

		if (QDF_STATUS_SUCCESS == status) {
			/* Now copy over all the security attributes you have
			 * parsed out. Use the cipher type in the RSN IE
			 */
			pConfig->RSNEncryptType = RSNEncryptType;
			pConfig->mcRSNEncryptType = mcRSNEncryptType;
			(WLAN_HDD_GET_AP_CTX_PTR(adapter))->
			encryption_type = RSNEncryptType;
			hdd_debug("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d",
			       RSNAuthType, RSNEncryptType, mcRSNEncryptType);
		}
	}

	pIe = wlan_get_vendor_ie_ptr_from_oui(WPA_OUI_TYPE, WPA_OUI_TYPE_SIZE,
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
				hdd_err("RSNWPA IE MAX Length exceeded; length: %d",
				       pConfig->RSNWPAReqIELength);
		} else {
			pConfig->RSNWPAReqIELength = pIe[1] + 2;
			if (pConfig->RSNWPAReqIELength <
			    sizeof(pConfig->RSNWPAReqIE))
				memcpy(&pConfig->RSNWPAReqIE[0], pIe,
				       pConfig->RSNWPAReqIELength);
			else
				hdd_err("RSNWPA IE MAX Length exceeded; length: %d",
				       pConfig->RSNWPAReqIELength);
			status = hdd_softap_unpack_ie
					(cds_get_context(QDF_MODULE_ID_SME),
					 &RSNEncryptType,
					 &mcRSNEncryptType, &RSNAuthType,
					 &MFPCapable, &MFPRequired,
					 pConfig->RSNWPAReqIE[1] + 2,
					 pConfig->RSNWPAReqIE);

			if (QDF_STATUS_SUCCESS == status) {
				/* Now copy over all the security attributes
				 * you have parsed out. Use the cipher type
				 * in the RSN IE
				 */
				pConfig->RSNEncryptType = RSNEncryptType;
				pConfig->mcRSNEncryptType = mcRSNEncryptType;
				(WLAN_HDD_GET_AP_CTX_PTR(adapter))->
				encryption_type = RSNEncryptType;
				hdd_debug("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d",
				       RSNAuthType, RSNEncryptType,
				       mcRSNEncryptType);
			}
		}
	}

	if (pConfig->RSNWPAReqIELength > sizeof(pConfig->RSNWPAReqIE)) {
		hdd_err("**RSNWPAReqIELength is too large***");
		ret = -EINVAL;
		goto error;
	}

	pConfig->SSIDinfo.ssidHidden = false;

	if (ssid != NULL) {
		qdf_mem_copy(pConfig->SSIDinfo.ssid.ssId, ssid, ssid_len);
		pConfig->SSIDinfo.ssid.length = ssid_len;

		switch (hidden_ssid) {
		case NL80211_HIDDEN_SSID_NOT_IN_USE:
			hdd_debug("HIDDEN_SSID_NOT_IN_USE");
			pConfig->SSIDinfo.ssidHidden = eHIDDEN_SSID_NOT_IN_USE;
			break;
		case NL80211_HIDDEN_SSID_ZERO_LEN:
			hdd_debug("HIDDEN_SSID_ZERO_LEN");
			pConfig->SSIDinfo.ssidHidden = eHIDDEN_SSID_ZERO_LEN;
			break;
		case NL80211_HIDDEN_SSID_ZERO_CONTENTS:
			hdd_debug("HIDDEN_SSID_ZERO_CONTENTS");
			pConfig->SSIDinfo.ssidHidden =
				eHIDDEN_SSID_ZERO_CONTENTS;
			break;
		default:
			hdd_err("Wrong hidden_ssid param: %d", hidden_ssid);
			break;
		}
	}

	qdf_mem_copy(pConfig->self_macaddr.bytes,
		     adapter->mac_addr.bytes,
		     QDF_MAC_ADDR_SIZE);

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
	if ((0 == hdd_ctx->config->conc_custom_rule1) ||
		(hdd_ctx->config->conc_custom_rule1 &&
		QDF_SAP_MODE == adapter->device_mode))
		pConfig->cc_switch_mode = iniConfig->WlanMccToSccSwitchMode;
#endif

	pIe = wlan_get_vendor_ie_ptr_from_oui(BLACKLIST_OUI_TYPE,
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
		hdd_debug("acl type = %d no deny mac = %d", pIe[6], pIe[7]);
		if (pConfig->num_deny_mac > MAX_ACL_MAC_ADDRESS)
			pConfig->num_deny_mac = MAX_ACL_MAC_ADDRESS;
		acl_entry = (struct qc_mac_acl_entry *)(pIe + 8);
		for (i = 0; i < pConfig->num_deny_mac; i++) {
			qdf_mem_copy(&pConfig->deny_mac[i], acl_entry->addr,
				     sizeof(qcmacaddr));
			acl_entry++;
		}
	}
	pIe = wlan_get_vendor_ie_ptr_from_oui(WHITELIST_OUI_TYPE,
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
		hdd_debug("acl type = %d no accept mac = %d",
		       pIe[6], pIe[7]);
		if (pConfig->num_accept_mac > MAX_ACL_MAC_ADDRESS)
			pConfig->num_accept_mac = MAX_ACL_MAC_ADDRESS;
		acl_entry = (struct qc_mac_acl_entry *)(pIe + 8);
		for (i = 0; i < pConfig->num_accept_mac; i++) {
			qdf_mem_copy(&pConfig->accept_mac[i], acl_entry->addr,
				     sizeof(qcmacaddr));
			acl_entry++;
		}
	}
	if (!hdd_ctx->config->force_sap_acs &&
	    !(ssid && qdf_str_len(PRE_CAC_SSID) == ssid_len &&
	      (0 == qdf_mem_cmp(ssid, PRE_CAC_SSID, ssid_len)))) {
		pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_SUPP_RATES,
					&pMgmt_frame->u.beacon.variable[0],
					pBeacon->head_len);

		if (pIe != NULL) {
			pIe++;
			pConfig->supported_rates.numRates = pIe[0];
			pIe++;
			for (i = 0;
			     i < pConfig->supported_rates.numRates; i++) {
				if (pIe[i]) {
					pConfig->supported_rates.rate[i] = pIe[i];
					hdd_debug("Configured Supported rate is %2x",
						  pConfig->supported_rates.rate[i]);
				}
			}
		}
		pIe = wlan_get_ie_ptr_from_eid(WLAN_EID_EXT_SUPP_RATES,
					       pBeacon->tail,
					       pBeacon->tail_len);
		if (pIe != NULL) {
			pIe++;
			pConfig->extended_rates.numRates = pIe[0];
			pIe++;
			for (i = 0; i < pConfig->extended_rates.numRates; i++) {
				if (pIe[i]) {
					pConfig->extended_rates.rate[i] = pIe[i];
					hdd_debug("Configured ext Supported rate is %2x",
						  pConfig->extended_rates.rate[i]);
				}
			}
		}
	}

	if (!cds_is_sub_20_mhz_enabled())
		wlan_hdd_set_sap_hwmode(adapter);

	if (((adapter->device_mode == QDF_SAP_MODE) &&
	     (hdd_ctx->config->sap_force_11n_for_11ac)) ||
	     ((adapter->device_mode == QDF_P2P_GO_MODE) &&
	     (hdd_ctx->config->go_force_11n_for_11ac))) {
		if (pConfig->SapHw_mode == eCSR_DOT11_MODE_11ac ||
		    pConfig->SapHw_mode == eCSR_DOT11_MODE_11ac_ONLY)
			pConfig->SapHw_mode = eCSR_DOT11_MODE_11n;
	}

	qdf_mem_zero(sme_config, sizeof(*sme_config));
	sme_get_config_param(hdd_ctx->hHal, sme_config);
	/* Override hostapd.conf wmm_enabled only for 11n and 11AC configs (IOT)
	 * As per spec 11N/AC STA are QOS STA and may not connect or throughput
	 * may not be good with non QOS 11N AP
	 * Default: enable QOS for SAP unless WMM IE not present for 11bga
	 */
	sme_config->csrConfig.WMMSupportMode = eCsrRoamWmmAuto;
	pIe = wlan_get_vendor_ie_ptr_from_oui(WMM_OUI_TYPE, WMM_OUI_TYPE_SIZE,
					pBeacon->tail, pBeacon->tail_len);
	if (!pIe && (pConfig->SapHw_mode == eCSR_DOT11_MODE_11a ||
		pConfig->SapHw_mode == eCSR_DOT11_MODE_11g ||
		pConfig->SapHw_mode == eCSR_DOT11_MODE_11b))
		sme_config->csrConfig.WMMSupportMode = eCsrRoamWmmNoQos;
	sme_update_config(hdd_ctx->hHal, sme_config);

	if (!((adapter->device_mode == QDF_SAP_MODE) &&
	     (hdd_ctx->config->sap_force_11n_for_11ac)) ||
	     ((adapter->device_mode == QDF_P2P_GO_MODE) &&
	     (hdd_ctx->config->go_force_11n_for_11ac))) {
		pConfig->ch_width_orig =
			hdd_map_nl_chan_width(pConfig->ch_width_orig);
	} else {
		if (pConfig->ch_width_orig >= NL80211_CHAN_WIDTH_40)
			pConfig->ch_width_orig = CH_WIDTH_40MHZ;
		else
			pConfig->ch_width_orig = CH_WIDTH_20MHZ;
	}

	if (wlan_hdd_setup_driver_overrides(adapter)) {
		ret = -EINVAL;
		goto error;
	}

	/* ht_capab is not what the name conveys,
	 * this is used for protection bitmap
	 */
	pConfig->ht_capab = iniConfig->apProtection;

	if (0 != wlan_hdd_cfg80211_update_apies(adapter)) {
		hdd_err("SAP Not able to set AP IEs");
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);
		ret = -EINVAL;
		goto error;
	}
	/* Uapsd Enabled Bit */
	pConfig->UapsdEnable = iniConfig->apUapsdEnabled;
	/* Enable OBSS protection */
	pConfig->obssProtEnabled = iniConfig->apOBSSProtEnabled;

	if (adapter->device_mode == QDF_SAP_MODE)
		pConfig->sap_dot11mc =
		    (WLAN_HDD_GET_CTX(adapter))->config->sap_dot11mc;
	else /* for P2P-Go case */
		pConfig->sap_dot11mc = 1;

	hdd_debug("11MC Support Enabled : %d\n",
		pConfig->sap_dot11mc);

#ifdef WLAN_FEATURE_11W
	pConfig->mfpCapable = MFPCapable;
	pConfig->mfpRequired = MFPRequired;
	hdd_debug("Soft AP MFP capable %d, MFP required %d",
	       pConfig->mfpCapable, pConfig->mfpRequired);
#endif

	hdd_debug("SOftAP macaddress : " MAC_ADDRESS_STR,
	       MAC_ADDR_ARRAY(adapter->mac_addr.bytes));
	hdd_debug("ssid =%s, beaconint=%d, channel=%d",
	       pConfig->SSIDinfo.ssid.ssId, (int)pConfig->beacon_int,
	       (int)pConfig->channel);
	hdd_debug("hw_mode=%x, privacy=%d, authType=%d",
	       pConfig->SapHw_mode, pConfig->privacy, pConfig->authType);
	hdd_debug("RSN/WPALen=%d, Uapsd = %d",
	       (int)pConfig->RSNWPAReqIELength, pConfig->UapsdEnable);
	hdd_debug("ProtEnabled = %d, OBSSProtEnabled = %d",
	       pConfig->protEnabled, pConfig->obssProtEnabled);
	hdd_debug("ChanSwitchHostapdRateEnabled = %d",
		pConfig->chan_switch_hostapd_rate_enabled);

	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);
		/* Bss already started. just return. */
		/* TODO Probably it should update some beacon params. */
		hdd_debug("Bss Already started...Ignore the request");
		EXIT();
		ret = 0;
		goto free;
	}

	if (check_for_concurrency) {
		if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
				policy_mgr_convert_device_mode_to_qdf_type(
					adapter->device_mode),
					pConfig->channel, HW_MODE_20_MHZ)) {
			hdd_err("This concurrency combination is not allowed");
			ret = -EINVAL;
			goto error;
		}
	}

	if (!hdd_set_connection_in_progress(true)) {
		hdd_err("Can't start BSS: set connnection in progress failed");
		ret = -EINVAL;
		goto error;
	}

	pConfig->persona = adapter->device_mode;

	pSapEventCallback = hdd_hostapd_sap_event_cb;

	(WLAN_HDD_GET_AP_CTX_PTR(adapter))->dfs_cac_block_tx = true;
	set_bit(SOFTAP_INIT_DONE, &adapter->event_flags);

	qdf_event_reset(&hostapd_state->qdf_event);
	status = wlansap_start_bss(
		WLAN_HDD_GET_SAP_CTX_PTR(adapter),
		pSapEventCallback, pConfig, adapter->dev);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);
		hdd_set_connection_in_progress(false);
		hdd_err("SAP Start Bss fail");
		ret = -EINVAL;
		goto error;
	}

	hdd_debug("Waiting for Scan to complete(auto mode) and BSS to start");

	qdf_status = qdf_wait_for_event_completion(&hostapd_state->qdf_event,
						SME_CMD_TIMEOUT_VALUE);

	wlansap_reset_sap_config_add_ie(pConfig, eUPDATE_IE_ALL);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("qdf wait for single_event failed!!");
		hdd_set_connection_in_progress(false);
		sme_get_command_q_status(hHal);
		wlansap_stop_bss(WLAN_HDD_GET_SAP_CTX_PTR(adapter));
		QDF_ASSERT(0);
		ret = -EINVAL;
		goto error;
	}
	/* Succesfully started Bss update the state bit. */
	set_bit(SOFTAP_BSS_STARTED, &adapter->event_flags);
	/* Initialize WMM configuation */
	hdd_wmm_init(adapter);
	if (hostapd_state->bss_state == BSS_START)
		policy_mgr_incr_active_session(hdd_ctx->hdd_psoc,
					adapter->device_mode,
					adapter->session_id);
#ifdef DHCP_SERVER_OFFLOAD
	if (iniConfig->enableDHCPServerOffload)
		wlan_hdd_set_dhcp_server_offload(adapter);
#endif /* DHCP_SERVER_OFFLOAD */

	ucfg_p2p_status_start_bss(adapter->hdd_vdev);

	/* Check and restart SAP if it is on unsafe channel */
	hdd_unsafe_channel_restart_sap(hdd_ctx);

	hdd_set_connection_in_progress(false);
	EXIT();

	ret = 0;
	goto free;

error:
	clear_bit(SOFTAP_INIT_DONE, &adapter->event_flags);
	wlan_hdd_undo_acs(adapter);

free:
	qdf_mem_free(sme_config);
	return ret;
}

int hdd_destroy_acs_timer(struct hdd_adapter *adapter)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (!adapter->session.ap.vendor_acs_timer_initialized)
		return 0;

	adapter->session.ap.vendor_acs_timer_initialized = false;

	if (QDF_TIMER_STATE_RUNNING ==
			adapter->session.ap.vendor_acs_timer.state) {
		qdf_status =
			qdf_mc_timer_stop(&adapter->session.ap.
					vendor_acs_timer);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			hdd_err("Failed to stop ACS timer");
	}

	if (adapter->session.ap.vendor_acs_timer.user_data)
		qdf_mem_free(adapter->session.ap.vendor_acs_timer.user_data);

	qdf_mc_timer_destroy(&adapter->session.ap.vendor_acs_timer);

	return 0;
}

/**
 * __wlan_hdd_cfg80211_stop_ap() - stop soft ap
 * @wiphy: Pointer to wiphy structure
 * @dev: Pointer to net_device structure
 *
 * Return: 0 for success non-zero for failure
 */
static int __wlan_hdd_cfg80211_stop_ap(struct wiphy *wiphy,
					struct net_device *dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_adapter *sta_adapter;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tSirUpdateIE updateIE;
	struct hdd_beacon_data *old;
	int ret;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (hdd_ctx->driver_status == DRIVER_MODULES_CLOSED) {
		hdd_err("Driver module is closed; dropping request");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_STOP_AP,
			 adapter->session_id, adapter->device_mode));

	if (!(adapter->device_mode == QDF_SAP_MODE ||
	      adapter->device_mode == QDF_P2P_GO_MODE)) {
		return -EOPNOTSUPP;
	}

	/* Clear SOFTAP_INIT_DONE flag to mark stop_ap deinit. So that we do
	 * not restart SAP after SSR as SAP is already stopped from user space.
	 * This update is moved to start of this function to resolve stop_ap
	 * call during SSR case. Adapter gets cleaned up as part of SSR.
	 */
	clear_bit(SOFTAP_INIT_DONE, &adapter->event_flags);
	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/*
	 * If a STA connection is in progress in another adapter, disconnect
	 * the STA and complete the SAP operation. STA will reconnect
	 * after SAP stop is done.
	 */
	sta_adapter = hdd_get_sta_connection_in_progress(hdd_ctx);
	if (sta_adapter) {
		hdd_debug("Disconnecting STA with session id: %d",
			  sta_adapter->session_id);
		wlan_hdd_disconnect(sta_adapter, eCSR_DISCONNECT_REASON_DEAUTH);
	}

	if (adapter->device_mode == QDF_SAP_MODE) {
		uint8_t num_sessions;

		wlan_hdd_del_station(adapter);

		/*
		 * For AP+AP mode, only trigger GREEN_AP_PS_STOP_EVENT, when the
		 * last AP stops.
		 */
		status = policy_mgr_mode_specific_num_open_sessions(
				hdd_ctx->hdd_psoc, QDF_SAP_MODE, &num_sessions);
		if (status == QDF_STATUS_SUCCESS) {
			if (num_sessions == 1) {
				hdd_debug("Disabling Green AP");
				ucfg_green_ap_set_ps_config(hdd_ctx->hdd_pdev,
							    false);
				wlan_green_ap_stop(hdd_ctx->hdd_pdev);
			}
		} else {
			hdd_err("Failed to get num open sessions for QDF_SAP_MODE");
		}
	}

	cds_flush_work(&adapter->sap_stop_bss_work);
	/*
	 * When ever stop ap adapter gets called, we need to check
	 * whether any restart AP work is pending. If any restart is pending
	 * then lets finish it and go ahead from there.
	 */
	if (hdd_ctx->config->conc_custom_rule1 &&
	    (QDF_SAP_MODE == adapter->device_mode)) {
		cds_flush_work(&hdd_ctx->sap_start_work);
		hdd_debug("Canceled the pending restart work");
		qdf_spin_lock(&hdd_ctx->sap_update_info_lock);
		hdd_ctx->is_sap_restart_required = false;
		qdf_spin_unlock(&hdd_ctx->sap_update_info_lock);
	}
	adapter->session.ap.sap_config.acs_cfg.acs_mode = false;
	wlan_hdd_undo_acs(adapter);
	qdf_mem_zero(&adapter->session.ap.sap_config.acs_cfg,
						sizeof(struct sap_acs_cfg));
	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	old = adapter->session.ap.beacon;
	if (!old) {
		hdd_err("Session id: %d beacon data points to NULL",
		       adapter->session_id);
		return -EINVAL;
	}
	wlan_hdd_cleanup_remain_on_channel_ctx(adapter);
	mutex_lock(&hdd_ctx->sap_lock);
	if (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags)) {
		struct hdd_hostapd_state *hostapd_state =
			WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);

		/* Set the stop_bss_in_progress flag */
		wlansap_set_stop_bss_inprogress(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter), true);

		qdf_event_reset(&hostapd_state->qdf_stop_bss_event);
		status = wlansap_stop_bss(WLAN_HDD_GET_SAP_CTX_PTR(adapter));
		if (QDF_IS_STATUS_SUCCESS(status)) {
			qdf_status =
				qdf_wait_for_event_completion(&hostapd_state->
					qdf_stop_bss_event,
					SME_CMD_TIMEOUT_VALUE);

			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				hdd_err("qdf wait for single_event failed!!");
				QDF_ASSERT(0);
			}
		}
		clear_bit(SOFTAP_BSS_STARTED, &adapter->event_flags);

		/* Clear the stop_bss_in_progress flag */
		wlansap_set_stop_bss_inprogress(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter), false);

		/*BSS stopped, clear the active sessions for this device mode*/
		policy_mgr_decr_session_set_pcl(hdd_ctx->hdd_psoc,
						adapter->device_mode,
						adapter->session_id);
		adapter->session.ap.beacon = NULL;
		qdf_mem_free(old);
	}
	mutex_unlock(&hdd_ctx->sap_lock);

	if (wlan_sap_is_pre_cac_active(hdd_ctx->hHal))
		hdd_clean_up_pre_cac_interface(hdd_ctx);

	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Stopping the BSS");
		return -EINVAL;
	}

	qdf_copy_macaddr(&updateIE.bssid, &adapter->mac_addr);
	updateIE.smeSessionId = adapter->session_id;
	updateIE.ieBufferlength = 0;
	updateIE.pAdditionIEBuffer = NULL;
	updateIE.append = true;
	updateIE.notify = true;
	if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
			      &updateIE,
			      eUPDATE_IE_PROBE_BCN) == QDF_STATUS_E_FAILURE) {
		hdd_err("Could not pass on PROBE_RSP_BCN data to PE");
	}

	if (sme_update_add_ie(WLAN_HDD_GET_HAL_CTX(adapter),
			      &updateIE,
			      eUPDATE_IE_ASSOC_RESP) == QDF_STATUS_E_FAILURE) {
		hdd_err("Could not pass on ASSOC_RSP data to PE");
	}
	/* Reset WNI_CFG_PROBE_RSP Flags */
	wlan_hdd_reset_prob_rspies(adapter);
	hdd_destroy_acs_timer(adapter);

	ucfg_p2p_status_stop_bss(adapter->hdd_vdev);

	EXIT();

	return ret;
}

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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) || \
	defined(CFG80211_BEACON_TX_RATE_CUSTOM_BACKPORT)
/**
 * hdd_get_data_rate_from_rate_mask() - convert mask to rate
 * @wiphy: Pointer to wiphy
 * @band: band
 * @bit_rate_mask: pointer to bit_rake_mask
 *
 * This function takes band and bit_rate_mask as input and
 * derives the beacon_tx_rate based on the supported rates
 * published as part of wiphy register.
 *
 * Return: data rate for success or zero for failure
 */
static uint16_t hdd_get_data_rate_from_rate_mask(struct wiphy *wiphy,
		enum nl80211_band band,
		struct cfg80211_bitrate_mask *bit_rate_mask)
{
	struct ieee80211_supported_band *sband = wiphy->bands[band];
	int sband_n_bitrates;
	struct ieee80211_rate *sband_bitrates;
	int i;

	if (sband) {
		sband_bitrates = sband->bitrates;
		sband_n_bitrates = sband->n_bitrates;
		for (i = 0; i < sband_n_bitrates; i++) {
			if (bit_rate_mask->control[band].legacy ==
			    sband_bitrates[i].hw_value)
				return sband_bitrates[i].bitrate;
		}
	}
	return 0;
}

/**
 * hdd_update_beacon_rate() - Update beacon tx rate
 * @adapter: Pointer to hdd_adapter_t
 * @wiphy: Pointer to wiphy
 * @params: Pointet to cfg80211_ap_settings
 *
 * This function updates the beacon tx rate which is provided
 * as part of cfg80211_ap_settions in to the sap_config
 * structure
 *
 * Return: none
 */
static void hdd_update_beacon_rate(struct hdd_adapter *adapter,
		struct wiphy *wiphy,
		struct cfg80211_ap_settings *params)
{
	struct cfg80211_bitrate_mask *beacon_rate_mask;
	enum nl80211_band band;

	band = params->chandef.chan->band;
	beacon_rate_mask = &params->beacon_rate;
	if (beacon_rate_mask->control[band].legacy) {
		adapter->session.ap.sap_config.beacon_tx_rate =
			hdd_get_data_rate_from_rate_mask(wiphy, band,
					beacon_rate_mask);
		hdd_debug("beacon mask value %u, rate %hu",
			  params->beacon_rate.control[0].legacy,
			  adapter->session.ap.sap_config.beacon_tx_rate);
	}
}
#else
static void hdd_update_beacon_rate(struct hdd_adapter *adapter,
		struct wiphy *wiphy,
		struct cfg80211_ap_settings *params)
{
}
#endif


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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	enum hw_mode_bandwidth channel_width;
	int status;
	struct sme_sta_inactivity_timeout  *sta_inactivity_timer;
	uint8_t channel;
	bool is_enabled = false;
	uint8_t ret;

	ENTER();

	clear_bit(SOFTAP_INIT_DONE, &adapter->event_flags);
	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("Invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_START_AP, adapter->session_id,
			 params->beacon_interval));
	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("HDD adapter magic is invalid");
		return -ENODEV;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	hdd_debug("adapter = %pK, Device mode %s(%d) sub20 %d",
		adapter, hdd_device_mode_to_string(adapter->device_mode),
		adapter->device_mode, cds_is_sub_20_mhz_enabled());


	if (policy_mgr_is_hw_mode_change_in_progress(hdd_ctx->hdd_psoc)) {
		status = policy_mgr_wait_for_connection_update(
			hdd_ctx->hdd_psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("qdf wait for event failed!!");
			return -EINVAL;
		}
	}

	channel_width = wlan_hdd_get_channel_bw(params->chandef.width);
	channel = ieee80211_frequency_to_channel(
				params->chandef.chan->center_freq);

	if (cds_is_sub_20_mhz_enabled()) {
		enum channel_state ch_state;
		enum phy_ch_width sub_20_ch_width = CH_WIDTH_INVALID;
		tsap_Config_t *sap_cfg = &adapter->session.ap.sap_config;

		/* Avoid ACS/DFS, and overwrite ch wd to 20 */
		if (channel == 0) {
			hdd_err("Can't start SAP-ACS (channel=0) with sub 20 MHz ch width");
			return -EINVAL;
		}
		if (CHANNEL_STATE_DFS == wlan_reg_get_channel_state(
					hdd_ctx->hdd_pdev, channel)) {
			hdd_err("Can't start SAP-DFS (channel=%d)with sub 20 MHz ch wd",
				channel);
			return -EINVAL;
		}
		if (channel_width != HW_MODE_20_MHZ) {
			hdd_err("Hostapd (20+ MHz) conflits with config.ini (sub 20 MHz)");
			return -EINVAL;
		}
		if (cds_is_5_mhz_enabled())
			sub_20_ch_width = CH_WIDTH_5MHZ;
		if (cds_is_10_mhz_enabled())
			sub_20_ch_width = CH_WIDTH_10MHZ;
		if (WLAN_REG_IS_5GHZ_CH(channel))
			ch_state = wlan_reg_get_5g_bonded_channel_state(
					hdd_ctx->hdd_pdev, channel,
					sub_20_ch_width);
		else
			ch_state = wlan_reg_get_2g_bonded_channel_state(
					hdd_ctx->hdd_pdev, channel,
					sub_20_ch_width, 0);
		if (CHANNEL_STATE_DISABLE == ch_state) {
			hdd_err("Given ch width not supported by reg domain");
			return -EINVAL;
		}
		sap_cfg->SapHw_mode = eCSR_DOT11_MODE_abg;
	}

	/* check if concurrency is allowed */
	if (!policy_mgr_allow_concurrency(hdd_ctx->hdd_psoc,
				policy_mgr_convert_device_mode_to_qdf_type(
				adapter->device_mode),
				channel,
				channel_width)) {
		hdd_err("Connection failed due to concurrency check failure");
		return -EINVAL;
	}

	status = policy_mgr_reset_connection_update(hdd_ctx->hdd_psoc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("ERR: clear event failed");

	status = policy_mgr_current_connections_update(hdd_ctx->hdd_psoc,
			adapter->session_id, channel,
			SIR_UPDATE_REASON_START_AP);
	if (QDF_STATUS_E_FAILURE == status) {
		hdd_err("ERROR: connections update failed!!");
		return -EINVAL;
	}

	if (QDF_STATUS_SUCCESS == status) {
		status = policy_mgr_wait_for_connection_update(hdd_ctx->hdd_psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("ERROR: qdf wait for event failed!!");
			return -EINVAL;
		}
	}

	ret = hdd_check_green_ap_enable(hdd_ctx, &is_enabled);
	if (!ret) {
		hdd_debug("Green AP enable status: %d", is_enabled);
		if (is_enabled) {
			hdd_debug("Enabling Green AP");
			ucfg_green_ap_set_ps_config(hdd_ctx->hdd_pdev, true);
			wlan_green_ap_start(hdd_ctx->hdd_pdev);
		}
	} else {
		hdd_err("Failed to check if Green AP should be enabled or not");
	}

	if (adapter->device_mode == QDF_P2P_GO_MODE) {
		struct hdd_adapter  *p2p_adapter;

		p2p_adapter = hdd_get_adapter(hdd_ctx, QDF_P2P_DEVICE_MODE);
		if (p2p_adapter) {
			hdd_debug("Cancel active p2p device ROC before GO starting");
			wlan_hdd_cancel_existing_remain_on_channel(
				p2p_adapter);
		}
	}

	if ((adapter->device_mode == QDF_SAP_MODE)
	    || (adapter->device_mode == QDF_P2P_GO_MODE)
	    ) {
		struct hdd_beacon_data *old, *new;
		enum nl80211_channel_type channel_type;
		tsap_Config_t *sap_config =
			&((WLAN_HDD_GET_AP_CTX_PTR(adapter))->sap_config);

		old = adapter->session.ap.beacon;

		if (old)
			return -EALREADY;

		status =
			wlan_hdd_cfg80211_alloc_new_beacon(adapter, &new,
							   &params->beacon,
							   params->dtim_period);

		if (status != 0) {
			hdd_err("Error!!! Allocating the new beacon");
			return -EINVAL;
		}
		adapter->session.ap.beacon = new;

		if (params->chandef.width < NL80211_CHAN_WIDTH_80)
			channel_type = cfg80211_get_chandef_type(
						&(params->chandef));
		else
			channel_type = NL80211_CHAN_HT40PLUS;


		wlan_hdd_set_channel(wiphy, dev,
				     &params->chandef,
				     channel_type);

		hdd_update_beacon_rate(adapter, wiphy, params);

		/* set authentication type */
		switch (params->auth_type) {
		case NL80211_AUTHTYPE_OPEN_SYSTEM:
			adapter->session.ap.sap_config.authType =
				eSAP_OPEN_SYSTEM;
			break;
		case NL80211_AUTHTYPE_SHARED_KEY:
			adapter->session.ap.sap_config.authType =
				eSAP_SHARED_KEY;
			break;
		default:
			adapter->session.ap.sap_config.authType =
				eSAP_AUTO_SWITCH;
		}
		adapter->session.ap.sap_config.ch_width_orig =
						params->chandef.width;
		status =
			wlan_hdd_cfg80211_start_bss(adapter,
				&params->beacon,
				params->ssid, params->ssid_len,
				params->hidden_ssid, true);

		if (status != 0) {
			hdd_err("Error Start bss Failed");
			goto err_start_bss;
		}

		/*
		 * If Do_Not_Break_Stream enabled send avoid channel list
		 * to application.
		 */
		if (policy_mgr_is_dnsc_set(adapter->hdd_vdev) &&
		    sap_config->channel) {
			wlan_hdd_send_avoid_freq_for_dnbs(hdd_ctx,
							  sap_config->channel);
		}
		if (hdd_ctx->config->sap_max_inactivity_override) {
			sta_inactivity_timer = qdf_mem_malloc(
					sizeof(*sta_inactivity_timer));
			if (!sta_inactivity_timer) {
				hdd_err("Failed to allocate Memory");
				return QDF_STATUS_E_FAILURE;
			}
			sta_inactivity_timer->session_id = adapter->session_id;
			sta_inactivity_timer->sta_inactivity_timeout =
				params->inactivity_timeout;
			sme_update_sta_inactivity_timeout(WLAN_HDD_GET_HAL_CTX
					(adapter), sta_inactivity_timer);
			qdf_mem_free(sta_inactivity_timer);
		}
	}

	goto success;

err_start_bss:
	if (adapter->session.ap.beacon)
		qdf_mem_free(adapter->session.ap.beacon);
	adapter->session.ap.beacon = NULL;

success:
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct hdd_beacon_data *old, *new;
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_CHANGE_BEACON,
			 adapter->session_id, adapter->device_mode));
	hdd_debug("Device_mode %s(%d)",
	       hdd_device_mode_to_string(adapter->device_mode),
	       adapter->device_mode);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (!(adapter->device_mode == QDF_SAP_MODE ||
	      adapter->device_mode == QDF_P2P_GO_MODE)) {
		return -EOPNOTSUPP;
	}

	old = adapter->session.ap.beacon;

	if (!old) {
		hdd_err("session id: %d beacon data points to NULL",
		       adapter->session_id);
		return -EINVAL;
	}

	status = wlan_hdd_cfg80211_alloc_new_beacon(adapter, &new, params, 0);

	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("new beacon alloc failed");
		return -EINVAL;
	}

	adapter->session.ap.beacon = new;
	hdd_debug("update beacon for P2P GO/SAP");
	status = wlan_hdd_cfg80211_start_bss(adapter, params, NULL,
					0, 0, false);

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

/**
 * hdd_sap_indicate_disconnect_for_sta() - Indicate disconnect indication
 * to supplicant, if there any clients connected to SAP interface.
 * @adapter: sap adapter context
 *
 * Return:   nothing
 */
void hdd_sap_indicate_disconnect_for_sta(struct hdd_adapter *adapter)
{
	tSap_Event sap_event;
	int sta_id;
	struct sap_context *sap_ctx;

	ENTER();

	sap_ctx = WLAN_HDD_GET_SAP_CTX_PTR(adapter);
	if (!sap_ctx) {
		hdd_err("invalid sap context");
		return;
	}

	for (sta_id = 0; sta_id < WLAN_MAX_STA_COUNT; sta_id++) {
		if (adapter->sta_info[sta_id].in_use) {
			hdd_debug("sta_id: %d in_use: %d %pK",
				 sta_id, adapter->sta_info[sta_id].in_use,
				 adapter);

			if (qdf_is_macaddr_broadcast(
				&adapter->sta_info[sta_id].sta_mac))
				continue;

			sap_event.sapHddEventCode = eSAP_STA_DISASSOC_EVENT;
			qdf_mem_copy(
				&sap_event.sapevt.
				sapStationDisassocCompleteEvent.staMac,
				&adapter->sta_info[sta_id].sta_mac,
				sizeof(struct qdf_mac_addr));
			sap_event.sapevt.sapStationDisassocCompleteEvent.
			reason =
				eSAP_MAC_INITATED_DISASSOC;
			sap_event.sapevt.sapStationDisassocCompleteEvent.
			statusCode =
				QDF_STATUS_E_RESOURCES;
			hdd_hostapd_sap_event_cb(&sap_event,
					sap_ctx->pUsrContext);
		}
	}

	EXIT();
}

bool hdd_is_peer_associated(struct hdd_adapter *adapter,
			    struct qdf_mac_addr *mac_addr)
{
	uint32_t cnt;
	struct hdd_station_info *sta_info;

	if (!adapter || !mac_addr) {
		hdd_err("Invalid adapter or mac_addr");
		return false;
	}

	sta_info = adapter->sta_info;
	spin_lock_bh(&adapter->sta_info_lock);
	for (cnt = 0; cnt < WLAN_MAX_STA_COUNT; cnt++) {
		if ((sta_info[cnt].in_use) &&
		    !qdf_mem_cmp(&(sta_info[cnt].sta_mac), mac_addr,
		    QDF_MAC_ADDR_SIZE))
			break;
	}
	spin_unlock_bh(&adapter->sta_info_lock);
	if (cnt != WLAN_MAX_STA_COUNT)
		return true;

	return false;
}
