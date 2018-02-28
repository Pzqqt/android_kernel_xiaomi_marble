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
 *
 * @file  wlan_hdd_p2p.c
 *
 * @brief WLAN Host Device Driver implementation for P2P commands interface
 *
 */

#include <wlan_hdd_includes.h>
#include <wlan_hdd_hostapd.h>
#include <net/cfg80211.h>
#include "sme_api.h"
#include "sme_qos_api.h"
#include "wlan_hdd_p2p.h"
#include "sap_api.h"
#include "wlan_hdd_main.h"
#include "qdf_trace.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/ieee80211_radiotap.h>
#include "wlan_hdd_tdls.h"
#include "wlan_hdd_trace.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "cds_sched.h"
#include "wlan_policy_mgr_api.h"
#include "cds_utils.h"
#include "wlan_p2p_public_struct.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_cfg80211_p2p.h"

/* Ms to Time Unit Micro Sec */
#define MS_TO_TU_MUS(x)   ((x) * 1024)
#define MAX_MUS_VAL       (INT_MAX / 1024)

#ifdef WLAN_FEATURE_P2P_DEBUG
#define MAX_P2P_ACTION_FRAME_TYPE 9
const char *p2p_action_frame_type[] = { "GO Negotiation Request",
					"GO Negotiation Response",
					"GO Negotiation Confirmation",
					"P2P Invitation Request",
					"P2P Invitation Response",
					"Device Discoverability Request",
					"Device Discoverability Response",
					"Provision Discovery Request",
					"Provision Discovery Response"};

#endif
#define MAX_TDLS_ACTION_FRAME_TYPE 11
const char *tdls_action_frame_type[] = { "TDLS Setup Request",
					 "TDLS Setup Response",
					 "TDLS Setup Confirm",
					 "TDLS Teardown",
					 "TDLS Peer Traffic Indication",
					 "TDLS Channel Switch Request",
					 "TDLS Channel Switch Response",
					 "TDLS Peer PSM Request",
					 "TDLS Peer PSM Response",
					 "TDLS Peer Traffic Response",
					 "TDLS Discovery Request"};

void wlan_hdd_cancel_existing_remain_on_channel(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("null adapter");
		return;
	}

	status = ucfg_p2p_cleanup_roc(adapter->hdd_vdev);
	hdd_debug("status:%d", status);
}

int wlan_hdd_check_remain_on_channel(struct hdd_adapter *adapter)
{
	if (QDF_P2P_GO_MODE != adapter->device_mode)
		wlan_hdd_cancel_existing_remain_on_channel(adapter);

	return 0;
}

/* Clean up RoC context at hdd_stop_adapter*/
void wlan_hdd_cleanup_remain_on_channel_ctx(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("null adapter");
		return;
	}

	status = ucfg_p2p_cleanup_roc(adapter->hdd_vdev);
	hdd_debug("status:%d", status);
}

static int __wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 struct ieee80211_channel *chan,
						 unsigned int duration,
						 u64 *cookie)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	status = wlan_cfg80211_roc(adapter->hdd_vdev, chan,
				duration, cookie);
	hdd_debug("remain on channel request, status:%d", status);

	return qdf_status_to_os_return(status);
}

int wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					struct ieee80211_channel *chan,
					unsigned int duration, u64 *cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_remain_on_channel(wiphy,
						    wdev,
						    chan,
						    duration, cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int
__wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     u64 cookie)
{
	QDF_STATUS status;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	status = wlan_cfg80211_cancel_roc(adapter->hdd_vdev, cookie);
	hdd_debug("cancel remain on channel, status:%d", status);

	return 0;
}

int wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       u64 cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_cancel_remain_on_channel(wiphy,
							   wdev,
							   cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int __wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			      struct ieee80211_channel *chan, bool offchan,
			      unsigned int wait,
			      const u8 *buf, size_t len, bool no_cck,
			      bool dont_wait_for_ack, u64 *cookie)
{
	QDF_STATUS status;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint8_t type;
	uint8_t sub_type;
	QDF_STATUS qdf_status;
	int ret;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret) {
		hdd_err("wlan_hdd_validate_context return:%d", ret);
		return ret;
	}

	type = WLAN_HDD_GET_TYPE_FRM_FC(buf[0]);
	sub_type = WLAN_HDD_GET_SUBTYPE_FRM_FC(buf[0]);

	/* When frame to be transmitted is auth mgmt, then trigger
	 * sme_send_mgmt_tx to send auth frame without need for policy manager.
	 * Where as wlan_cfg80211_mgmt_tx requires roc and requires approval
	 * from policy manager.
	 */
	if ((adapter->device_mode == QDF_STA_MODE) &&
	    (type == SIR_MAC_MGMT_FRAME &&
	    sub_type == SIR_MAC_MGMT_AUTH)) {
		qdf_status = sme_send_mgmt_tx(WLAN_HDD_GET_HAL_CTX(adapter),
					      adapter->session_id, buf, len);

		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			return 0;
		else
			return -EINVAL;
	}

	status = wlan_cfg80211_mgmt_tx(adapter->hdd_vdev, chan,
			offchan, wait, buf, len, no_cck,
			dont_wait_for_ack, cookie);
	hdd_debug("mgmt tx, status:%d", status);

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		     struct cfg80211_mgmt_tx_params *params, u64 *cookie)
#else
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		     struct ieee80211_channel *chan, bool offchan,
		     unsigned int wait,
		     const u8 *buf, size_t len, bool no_cck,
		     bool dont_wait_for_ack, u64 *cookie)
#endif /* LINUX_VERSION_CODE */
{
	int ret;

	cds_ssr_protect(__func__);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	ret = __wlan_hdd_mgmt_tx(wiphy, wdev, params->chan, params->offchan,
				 params->wait, params->buf, params->len,
				 params->no_cck, params->dont_wait_for_ack,
				 cookie);
#else
	ret = __wlan_hdd_mgmt_tx(wiphy, wdev, chan, offchan,
				 wait, buf, len, no_cck,
				 dont_wait_for_ack, cookie);
#endif /* LINUX_VERSION_CODE */
	cds_ssr_unprotect(__func__);

	return ret;
}

static int __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   u64 cookie)
{
	QDF_STATUS status;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->session_id)) {
		hdd_err("invalid session id: %d", adapter->session_id);
		return -EINVAL;
	}

	status = wlan_cfg80211_mgmt_tx_cancel(adapter->hdd_vdev,
						cookie);
	hdd_debug("cancel mgmt tx, status:%d", status);

	return 0;
}

int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
					  struct wireless_dev *wdev, u64 cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(wiphy, wdev, cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_set_p2p_noa
 *
 ***FUNCTION:
 * This function is called from hdd_hostapd_ioctl function when Driver
 * get P2P_SET_NOA comand from wpa_supplicant using private ioctl
 *
 ***LOGIC:
 * Fill noa Struct According to P2P Power save Option and Pass it to SME layer
 *
 ***ASSUMPTIONS:
 *
 *
 ***NOTE:
 *
 * @param dev          Pointer to net device structure
 * @param command      Pointer to command
 *
 * @return Status
 */

int hdd_set_p2p_noa(struct net_device *dev, uint8_t *command)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct p2p_ps_config noa = {0};
	int count, duration, interval;
	char *param;
	int ret;

	param = strnchr(command, strlen(command), ' ');
	if (param == NULL) {
		hdd_err("strnchr failed to find delimeter");
		return -EINVAL;
	}
	param++;
	ret = sscanf(param, "%d %d %d", &count, &interval, &duration);
	if (ret != 3) {
		hdd_err("P2P_SET GO noa: fail to read params, ret=%d",
			ret);
		return -EINVAL;
	}
	if (count < 0 || interval < 0 || duration < 0 ||
	    interval > MAX_MUS_VAL || duration > MAX_MUS_VAL) {
		hdd_err("Invalid NOA parameters");
		return -EINVAL;
	}
	hdd_debug("P2P_SET GO noa: count=%d interval=%d duration=%d",
		count, interval, duration);
	duration = MS_TO_TU_MUS(duration);
	/* PS Selection
	 * Periodic noa (2)
	 * Single NOA   (4)
	 */
	noa.opp_ps = 0;
	noa.ct_window = 0;
	if (count == 1) {
		noa.duration = 0;
		noa.single_noa_duration = duration;
		noa.ps_selection = P2P_POWER_SAVE_TYPE_SINGLE_NOA;
	} else {
		noa.duration = duration;
		noa.single_noa_duration = 0;
		noa.ps_selection = P2P_POWER_SAVE_TYPE_PERIODIC_NOA;
	}
	noa.interval = MS_TO_TU_MUS(interval);
	noa.count = count;
	noa.vdev_id = adapter->session_id;

	hdd_debug("P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
		  "interval %d count %d single noa duration %d "
		  "PsSelection %x", noa.opp_ps,
		  noa.ct_window, noa.duration, noa.interval,
		  noa.count, noa.single_noa_duration, noa.ps_selection);

	return wlan_hdd_set_power_save(adapter, &noa);
}

/**
 * hdd_set_p2p_opps
 *
 ***FUNCTION:
 * This function is called from hdd_hostapd_ioctl function when Driver
 * get P2P_SET_PS comand from wpa_supplicant using private ioctl
 *
 ***LOGIC:
 * Fill noa Struct According to P2P Power save Option and Pass it to SME layer
 *
 ***ASSUMPTIONS:
 *
 *
 ***NOTE:
 *
 * @param  dev         Pointer to net device structure
 * @param  command     Pointer to command
 *
 * @return Status
 */

int hdd_set_p2p_opps(struct net_device *dev, uint8_t *command)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct p2p_ps_config noa = {0};
	char *param;
	int legacy_ps, opp_ps, ctwindow;
	int ret;

	param = strnchr(command, strlen(command), ' ');
	if (param == NULL) {
		hdd_err("strnchr failed to find delimiter");
		return -EINVAL;
	}
	param++;
	ret = sscanf(param, "%d %d %d", &legacy_ps, &opp_ps, &ctwindow);
	if (ret != 3) {
		hdd_err("P2P_SET GO PS: fail to read params, ret=%d", ret);
		return -EINVAL;
	}

	if ((opp_ps != -1) && (opp_ps != 0) && (opp_ps != 1)) {
		hdd_err("Invalid opp_ps value:%d", opp_ps);
		return -EINVAL;
	}

	/* P2P spec: 3.3.2 Power Management and discovery:
	 *     CTWindow should be at least 10 TU.
	 * P2P spec: Table 27 - CTWindow and OppPS Parameters field format:
	 *     CTWindow and OppPS Parameters together is 8 bits.
	 *     CTWindow uses 7 bits (0-6, Bit 7 is for OppPS)
	 * 0 indicates that there shall be no CTWindow
	 */
	if ((ctwindow != -1) && (ctwindow != 0) &&
	    (!((ctwindow >= 10) && (ctwindow <= 127)))) {
		hdd_err("Invalid CT window value:%d", ctwindow);
		return -EINVAL;
	}

	hdd_debug("P2P_SET GO PS: legacy_ps=%d opp_ps=%d ctwindow=%d",
		  legacy_ps, opp_ps, ctwindow);

	/* PS Selection
	 * Opportunistic Power Save (1)
	 */

	/* From wpa_cli user need to use separate command to set ctWindow and
	 * Opps when user want to set ctWindow during that time other parameters
	 * values are coming from wpa_supplicant as -1.
	 * Example : User want to set ctWindow with 30 then wpa_cli command :
	 * P2P_SET ctwindow 30
	 * Command Received at hdd_hostapd_ioctl is as below:
	 * P2P_SET_PS -1 -1 30 (legacy_ps = -1, opp_ps = -1, ctwindow = 30)
	 *
	 * e.g., 1: P2P_SET_PS 1 1 30
	 * Driver sets the Opps and CTwindow as 30 and send it to FW.
	 * e.g., 2: P2P_SET_PS 1 -1 15
	 * Driver caches the CTwindow value but not send the command to FW.
	 * e.g., 3: P2P_SET_PS 1 1 -1
	 * Driver sends the command to FW with Opps enabled and CT window as
	 * 15 (last cached CTWindow value).
	 * (or) : P2P_SET_PS 1 1 20
	 * Driver sends the command to FW with opps enabled and CT window
	 * as 20.
	 *
	 * legacy_ps param remains unused until required in the future.
	 */
	if (ctwindow != -1)
		adapter->ctw = ctwindow;

	/* Send command to FW when OppPS is either enabled(1)/disbaled(0) */
	if (opp_ps != -1) {
		adapter->ops = opp_ps;
		noa.opp_ps = adapter->ops;
		noa.ct_window = adapter->ctw;
		noa.duration = 0;
		noa.single_noa_duration = 0;
		noa.interval = 0;
		noa.count = 0;
		noa.ps_selection = P2P_POWER_SAVE_TYPE_OPPORTUNISTIC;
		noa.vdev_id = adapter->session_id;

		hdd_debug("P2P_PS_ATTR: oppPS %d ctWindow %d duration %d interval %d count %d single noa duration %d PsSelection %x",
			noa.opp_ps, noa.ct_window,
			noa.duration, noa.interval, noa.count,
			noa.single_noa_duration,
			noa.ps_selection);

		wlan_hdd_set_power_save(adapter, &noa);
	}

	return 0;
}

int hdd_set_p2p_ps(struct net_device *dev, void *msgData)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct p2p_ps_config noa = {0};
	struct p2p_app_set_ps *pappnoa = (struct p2p_app_set_ps *) msgData;

	noa.opp_ps = pappnoa->opp_ps;
	noa.ct_window = pappnoa->ctWindow;
	noa.duration = pappnoa->duration;
	noa.interval = pappnoa->interval;
	noa.count = pappnoa->count;
	noa.single_noa_duration = pappnoa->single_noa_duration;
	noa.ps_selection = pappnoa->psSelection;
	noa.vdev_id = adapter->session_id;

	return wlan_hdd_set_power_save(adapter, &noa);
}

static uint8_t wlan_hdd_get_session_type(enum nl80211_iftype type)
{
	switch (type) {
	case NL80211_IFTYPE_AP:
		return QDF_SAP_MODE;
	case NL80211_IFTYPE_P2P_GO:
		return QDF_P2P_GO_MODE;
	case NL80211_IFTYPE_P2P_CLIENT:
		return QDF_P2P_CLIENT_MODE;
	case NL80211_IFTYPE_STATION:
		return QDF_STA_MODE;
	default:
		return QDF_STA_MODE;
	}
}

/**
 * wlan_hdd_allow_sap_add() - check to add new sap interface
 * @hdd_ctx: pointer to hdd context
 * @name: name of the new interface
 * @sap_dev: output pointer to hold existing interface
 *
 * Return: If able to add interface return true else false
 */
static bool
wlan_hdd_allow_sap_add(struct hdd_context *hdd_ctx, const char *name,
		       struct wireless_dev **sap_dev)
{
	struct hdd_adapter *adapter;

	*sap_dev = NULL;

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (adapter->device_mode == QDF_SAP_MODE &&
		    test_bit(NET_DEVICE_REGISTERED, &adapter->event_flags) &&
		    !strncmp(adapter->dev->name, name, IFNAMSIZ)) {
			struct hdd_beacon_data *beacon =
						adapter->session.ap.beacon;

			hdd_debug("iface already registered");
			if (beacon) {
				adapter->session.ap.beacon = NULL;
				qdf_mem_free(beacon);
			}
			if (adapter->dev && adapter->dev->ieee80211_ptr) {
				*sap_dev = adapter->dev->ieee80211_ptr;
				return false;
			}

			hdd_err("ieee80211_ptr points to NULL");
			return false;
		}
	}

	return true;
}

/**
 * __wlan_hdd_add_virtual_intf() - Add virtual interface
 * @wiphy: wiphy pointer
 * @name: User-visible name of the interface
 * @name_assign_type: the name of assign type of the netdev
 * @nl80211_iftype: (virtual) interface types
 * @flags: moniter configuraiton flags (not used)
 * @vif_params: virtual interface parameters (not used)
 *
 * Return: the pointer of wireless dev, otherwise ERR_PTR.
 */
static
struct wireless_dev *__wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
						 const char *name,
						 unsigned char name_assign_type,
						 enum nl80211_iftype type,
						 u32 *flags,
						 struct vif_params *params)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	struct hdd_adapter *adapter = NULL;
	int ret;
	uint8_t session_type;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return ERR_PTR(-EINVAL);
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ERR_PTR(ret);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_ADD_VIRTUAL_INTF, NO_SESSION, type));
	/*
	 * Allow addition multiple interfaces for QDF_P2P_GO_MODE,
	 * QDF_SAP_MODE, QDF_P2P_CLIENT_MODE and QDF_STA_MODE
	 * session type.
	 */
	session_type = wlan_hdd_get_session_type(type);
	if (hdd_get_adapter(hdd_ctx, session_type) != NULL
	    && QDF_SAP_MODE != session_type
	    && QDF_P2P_GO_MODE != session_type
	    && QDF_P2P_CLIENT_MODE != session_type
	    && QDF_STA_MODE != session_type) {
		hdd_err("Interface type %d already exists. Two interfaces of same type are not supported currently.",
			type);
		return ERR_PTR(-EINVAL);
	}

	adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
	if ((adapter != NULL) &&
		!(wlan_hdd_validate_session_id(adapter->session_id))) {
		if (ucfg_scan_get_vdev_status(adapter->hdd_vdev) !=
				SCAN_NOT_IN_PROGRESS) {
			wlan_abort_scan(hdd_ctx->hdd_pdev, INVAL_PDEV_ID,
				adapter->session_id, INVALID_SCAN_ID, false);
			hdd_debug("Abort Scan while adding virtual interface");
		}
	}

	if (session_type == QDF_SAP_MODE) {
		struct wireless_dev *sap_dev;
		bool allow_add_sap = wlan_hdd_allow_sap_add(hdd_ctx, name,
							    &sap_dev);
		if (!allow_add_sap) {
			if (sap_dev)
				return sap_dev;

			return ERR_PTR(-EINVAL);
		}
	}

	adapter = NULL;
	if (hdd_ctx->config->isP2pDeviceAddrAdministrated &&
	    ((NL80211_IFTYPE_P2P_GO == type) ||
	     (NL80211_IFTYPE_P2P_CLIENT == type))) {
		/*
		 * Generate the P2P Interface Address. this address must be
		 * different from the P2P Device Address.
		 */
		struct qdf_mac_addr p2p_device_address =
						hdd_ctx->p2p_device_address;
		p2p_device_address.bytes[4] ^= 0x80;
		adapter = hdd_open_adapter(hdd_ctx,
					    session_type,
					    name, p2p_device_address.bytes,
					    name_assign_type,
					    true);
	} else {
		adapter = hdd_open_adapter(hdd_ctx,
					    session_type,
					    name,
					    wlan_hdd_get_intf_addr(hdd_ctx),
					    name_assign_type,
					    true);
	}

	if (NULL == adapter) {
		hdd_err("hdd_open_adapter failed");
		return ERR_PTR(-ENOSPC);
	}

	/*
	 * Add interface can be requested from the upper layer at any time
	 * check the statemachine for modules state and if they are closed
	 * open the modules.
	 */
	ret = hdd_wlan_start_modules(hdd_ctx, adapter, false);
	if (ret) {
		hdd_err("Failed to start the wlan_modules");
		goto close_adapter;
	}

	/*
	 * Once the support for session creation/deletion from
	 * hdd_hostapd_open/hdd_host_stop is in place.
	 * The support for starting adapter from here can be removed.
	 */
	if (NL80211_IFTYPE_AP == type || (NL80211_IFTYPE_P2P_GO == type)) {
		ret = hdd_start_adapter(adapter);
		if (ret) {
			hdd_err("Failed to start %s", name);
			goto stop_modules;
		}
	}

	if (hdd_ctx->rps)
		hdd_send_rps_ind(adapter);

	EXIT();
	return adapter->dev->ieee80211_ptr;

stop_modules:
	/*
	 * Find if any iface is up. If there is not iface which is up
	 * start the timer to close the modules
	 */
	if (hdd_check_for_opened_interfaces(hdd_ctx)) {
		hdd_debug("Closing all modules from the add_virt_iface");
		qdf_sched_delayed_work(&hdd_ctx->iface_idle_work,
				       hdd_ctx->config->iface_change_wait_time);
		hdd_prevent_suspend_timeout(
			hdd_ctx->config->iface_change_wait_time,
			WIFI_POWER_EVENT_WAKELOCK_IFACE_CHANGE_TIMER);
	} else
		hdd_debug("Other interfaces are still up dont close modules!");

close_adapter:
	hdd_close_adapter(hdd_ctx, adapter, true);

	return ERR_PTR(-EINVAL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       unsigned char name_assign_type,
					       enum nl80211_iftype type,
					       struct vif_params *params)
{
	struct wireless_dev *wdev;

	cds_ssr_protect(__func__);
	wdev = __wlan_hdd_add_virtual_intf(wiphy, name, name_assign_type,
					   type, &params->flags, params);
	cds_ssr_unprotect(__func__);

	return wdev;
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) || defined(WITH_BACKPORTS)
/**
 * wlan_hdd_add_virtual_intf() - Add virtual interface wrapper
 * @wiphy: wiphy pointer
 * @name: User-visible name of the interface
 * @name_assign_type: the name of assign type of the netdev
 * @nl80211_iftype: (virtual) interface types
 * @flags: monitor mode configuration flags (not used)
 * @vif_params: virtual interface parameters (not used)
 *
 * Return: the pointer of wireless dev, otherwise ERR_PTR.
 */
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       unsigned char name_assign_type,
					       enum nl80211_iftype type,
					       u32 *flags,
					       struct vif_params *params)
{
	struct wireless_dev *wdev;

	cds_ssr_protect(__func__);
	wdev = __wlan_hdd_add_virtual_intf(wiphy, name, name_assign_type,
					   type, flags, params);
	cds_ssr_unprotect(__func__);
	return wdev;

}
#else
/**
 * wlan_hdd_add_virtual_intf() - Add virtual interface wrapper
 * @wiphy: wiphy pointer
 * @name: User-visible name of the interface
 * @nl80211_iftype: (virtual) interface types
 * @flags: monitor mode configuration flags (not used)
 * @vif_params: virtual interface parameters (not used)
 *
 * Return: the pointer of wireless dev, otherwise ERR_PTR.
 */
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       enum nl80211_iftype type,
					       u32 *flags,
					       struct vif_params *params)
{
	struct wireless_dev *wdev;
	unsigned char name_assign_type = 0;

	cds_ssr_protect(__func__);
	wdev = __wlan_hdd_add_virtual_intf(wiphy, name, name_assign_type,
					   type, flags, params);
	cds_ssr_unprotect(__func__);
	return wdev;

}
#endif

int __wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	struct hdd_adapter *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	/*
	 * Clear SOFTAP_INIT_DONE flag to mark SAP unload, so that we do
	 * not restart SAP after SSR as SAP is already stopped from user space.
	 */
	clear_bit(SOFTAP_INIT_DONE, &pVirtAdapter->event_flags);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_DEL_VIRTUAL_INTF,
			 pVirtAdapter->session_id, pVirtAdapter->device_mode));
	hdd_debug("Device_mode %s(%d)",
		   hdd_device_mode_to_string(pVirtAdapter->device_mode),
		   pVirtAdapter->device_mode);

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	/* check state machine state and kickstart modules if they are closed */
	status = hdd_wlan_start_modules(hdd_ctx, pVirtAdapter, false);
	if (status)
		return status;

	if (pVirtAdapter->device_mode == QDF_SAP_MODE &&
	    wlan_sap_is_pre_cac_active(hdd_ctx->hHal)) {
		hdd_clean_up_pre_cac_interface(hdd_ctx);
	} else {
		wlan_hdd_release_intf_addr(hdd_ctx,
					 pVirtAdapter->mac_addr.bytes);
		hdd_stop_adapter(hdd_ctx, pVirtAdapter);
		hdd_deinit_adapter(hdd_ctx, pVirtAdapter, true);
		hdd_close_adapter(hdd_ctx, pVirtAdapter, true);
	}

	EXIT();

	return 0;
}

int wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_del_virtual_intf(wiphy, wdev);
	cds_ssr_unprotect(__func__);

	return ret;
}

void __hdd_indicate_mgmt_frame(struct hdd_adapter *adapter,
			     uint32_t nFrameLength,
			     uint8_t *pbFrames,
			     uint8_t frameType, uint32_t rxChan, int8_t rxRssi)
{
	uint16_t freq;
	uint8_t type = 0;
	uint8_t subType = 0;
	struct hdd_context *hdd_ctx;

	hdd_debug("Frame Type = %d Frame Length = %d",
		frameType, nFrameLength);

	if (NULL == adapter) {
		hdd_err("adapter is NULL");
		return;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (0 == nFrameLength) {
		hdd_err("Frame Length is Invalid ZERO");
		return;
	}

	if (NULL == pbFrames) {
		hdd_err("pbFrames is NULL");
		return;
	}

	type = WLAN_HDD_GET_TYPE_FRM_FC(pbFrames[0]);
	subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(pbFrames[0]);

	/* Get adapter from Destination mac address of the frame */
	if ((type == SIR_MAC_MGMT_FRAME) &&
	    (subType != SIR_MAC_MGMT_PROBE_REQ) &&
	    !qdf_is_macaddr_broadcast(
	     (struct qdf_mac_addr *)&pbFrames[WLAN_HDD_80211_FRM_DA_OFFSET])) {
		adapter =
			hdd_get_adapter_by_macaddr(hdd_ctx,
						   &pbFrames
						   [WLAN_HDD_80211_FRM_DA_OFFSET]);
		if (NULL == adapter) {
			/*
			 * Under assumtion that we don't receive any action
			 * frame with BCST as destination,
			 * we are dropping action frame
			 */
			hdd_err("adapter for action frame is NULL Macaddr = "
				  MAC_ADDRESS_STR,
				  MAC_ADDR_ARRAY(&pbFrames
						 [WLAN_HDD_80211_FRM_DA_OFFSET]));
			hdd_debug("Frame Type = %d Frame Length = %d subType = %d",
				frameType, nFrameLength, subType);
			/*
			 * We will receive broadcast management frames
			 * in OCB mode
			 */
			adapter = hdd_get_adapter(hdd_ctx, QDF_OCB_MODE);
			if (NULL == adapter || !qdf_is_macaddr_broadcast(
				(struct qdf_mac_addr *)&pbFrames
				[WLAN_HDD_80211_FRM_DA_OFFSET])) {
				/*
				 * Under assumtion that we don't
				 * receive any action frame with BCST
				 * as destination, we are dropping
				 * action frame
				 */
			return;
			}
		}
	}

	if (NULL == adapter->dev) {
		hdd_err("adapter->dev is NULL");
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("adapter has invalid magic");
		return;
	}

	/* Channel indicated may be wrong. TODO */
	/* Indicate an action frame. */
	if (rxChan <= MAX_NO_OF_2_4_CHANNELS)
		freq = ieee80211_channel_to_frequency(rxChan,
						      NL80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(rxChan,
						      NL80211_BAND_5GHZ);

	/* Indicate Frame Over Normal Interface */
	hdd_debug("Indicate Frame over NL80211 sessionid : %d, idx :%d",
		   adapter->session_id, adapter->dev->ifindex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
		 freq, rxRssi * 100, pbFrames,
			 nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
	cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
			freq, rxRssi * 100, pbFrames,
			 nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED,
			 GFP_ATOMIC);
#else
	cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr, freq,
			rxRssi * 100,
			pbFrames, nFrameLength, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE */
}

int wlan_hdd_set_power_save(struct hdd_adapter *adapter,
	struct p2p_ps_config *ps_config)
{
	struct wlan_objmgr_psoc *psoc;
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;

	if (!adapter || !ps_config) {
		hdd_err("null param, adapter:%pK, ps_config:%pK",
			adapter, ps_config);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	psoc = hdd_ctx->hdd_psoc;
	if (!psoc) {
		hdd_err("psoc is null");
		return -EINVAL;
	}

	hdd_debug("opp ps:%d, ct window:%d, duration:%d, interval:%d, count:%d, single noa duration:%d, ps selection:%d, vdev id:%d",
		ps_config->opp_ps, ps_config->ct_window,
		ps_config->duration, ps_config->interval,
		ps_config->count, ps_config->single_noa_duration,
		ps_config->ps_selection, ps_config->vdev_id);

	status = ucfg_p2p_set_ps(psoc, ps_config);
	hdd_debug("p2p set power save, status:%d", status);

	return qdf_status_to_os_return(status);
}

int wlan_hdd_listen_offload_start(struct hdd_adapter *adapter,
	struct sir_p2p_lo_start *params)
{
	struct wlan_objmgr_psoc *psoc;
	struct p2p_lo_start lo_start;
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;

	if (!adapter || !params) {
		hdd_err("null param, adapter:%pK, params:%pK",
			adapter, params);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	psoc = hdd_ctx->hdd_psoc;
	if (!psoc) {
		hdd_err("psoc is null");
		return -EINVAL;
	}

	lo_start.vdev_id = params->vdev_id;
	lo_start.ctl_flags = params->ctl_flags;
	lo_start.freq = params->freq;
	lo_start.period = params->period;
	lo_start.interval = params->interval;
	lo_start.count = params->count;
	lo_start.device_types = params->device_types;
	lo_start.dev_types_len = params->dev_types_len;
	lo_start.probe_resp_tmplt = params->probe_resp_tmplt;
	lo_start.probe_resp_len = params->probe_resp_len;

	status = ucfg_p2p_lo_start(psoc, &lo_start);
	hdd_debug("p2p listen offload start, status:%d", status);

	return qdf_status_to_os_return(status);
}

int wlan_hdd_listen_offload_stop(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_psoc *psoc;
	struct hdd_context *hdd_ctx;
	uint32_t vdev_id;
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("adapter is null, adapter:%pK", adapter);
		return -EINVAL;
	}

	vdev_id = (uint32_t)adapter->session_id;
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	psoc = hdd_ctx->hdd_psoc;
	if (!psoc) {
		hdd_err("psoc is null");
		return -EINVAL;
	}

	status = ucfg_p2p_lo_stop(psoc, vdev_id);
	hdd_debug("p2p listen offload stop, status:%d", status);

	return qdf_status_to_os_return(status);
}

/**
 * wlan_hdd_update_mcc_adaptive_scheduler() - Function to update
 * MAS value to FW
 * @adapter:            adapter object data
 * @is_enable:          0-Disable, 1-Enable MAS
 *
 * This function passes down the value of MAS to UMAC
 *
 * Return: 0 for success else non zero
 *
 */
static int32_t wlan_hdd_update_mcc_adaptive_scheduler(
		struct hdd_adapter *adapter, bool is_enable)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (hdd_ctx == NULL) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	hdd_info("enable/disable MAS :%d", is_enable);
	if (hdd_ctx->config &&
	    hdd_ctx->config->enableMCCAdaptiveScheduler) {
		/* Todo check where to set the MCC apative SCHED for read */

		if (QDF_STATUS_SUCCESS != sme_set_mas(is_enable)) {
			hdd_err("Failed to enable/disable MAS");
			return -EAGAIN;
		}
	}

	return 0;
}

/**
 * wlan_hdd_update_mcc_p2p_quota() - Function to Update P2P
 * quota to FW
 * @adapter:            Pointer to HDD adapter
 * @is_set:             0-reset, 1-set
 *
 * This function passes down the value of MAS to UMAC
 *
 * Return: none
 *
 */
static void wlan_hdd_update_mcc_p2p_quota(struct hdd_adapter *adapter,
					  bool is_set)
{

	hdd_info("Set/reset P2P quota: %d", is_set);
	if (is_set) {
		if (adapter->device_mode == QDF_STA_MODE)
			wlan_hdd_set_mcc_p2p_quota(adapter,
				100 - HDD_DEFAULT_MCC_P2P_QUOTA
			);
		else if (adapter->device_mode == QDF_P2P_GO_MODE)
			wlan_hdd_go_set_mcc_p2p_quota(adapter,
				HDD_DEFAULT_MCC_P2P_QUOTA);
		else
			wlan_hdd_set_mcc_p2p_quota(adapter,
				HDD_DEFAULT_MCC_P2P_QUOTA);
	} else {
		if (adapter->device_mode == QDF_P2P_GO_MODE)
			wlan_hdd_go_set_mcc_p2p_quota(adapter,
				HDD_RESET_MCC_P2P_QUOTA);
		else
			wlan_hdd_set_mcc_p2p_quota(adapter,
				HDD_RESET_MCC_P2P_QUOTA);
	}
}

int32_t wlan_hdd_set_mas(struct hdd_adapter *adapter, uint8_t mas_value)
{
	int32_t ret = 0;

	if (!adapter) {
		hdd_err("Adapter is NULL");
		return -EINVAL;
	}

	if (mas_value) {
		hdd_info("Miracast is ON. Disable MAS and configure P2P quota");
		ret = wlan_hdd_update_mcc_adaptive_scheduler(
			adapter, false);
		if (0 != ret) {
			hdd_err("Failed to disable MAS");
			goto done;
		}

		/* Config p2p quota */
		wlan_hdd_update_mcc_p2p_quota(adapter, true);
	} else {
		hdd_info("Miracast is OFF. Enable MAS and reset P2P quota");
		wlan_hdd_update_mcc_p2p_quota(adapter, false);

		ret = wlan_hdd_update_mcc_adaptive_scheduler(
			adapter, true);
		if (0 != ret) {
			hdd_err("Failed to enable MAS");
			goto done;
		}
	}

done:
	return ret;
}

/**
 * set_first_connection_operating_channel() - Function to set
 * first connection oerating channel
 * @adapter:   adapter data
 * @set_value: Quota value for the interface
 * @dev_mode:  Device mode
 * This function is used to set the first adapter operating
 * channel
 *
 * Return: operating channel updated in set value
 *
 */
static uint32_t set_first_connection_operating_channel(
		struct hdd_context *hdd_ctx, uint32_t set_value,
		enum QDF_OPMODE dev_mode)
{
	uint8_t operating_channel;

	operating_channel = hdd_get_operating_channel(
					hdd_ctx, dev_mode);
	if (!operating_channel) {
		hdd_err(" First adpter operating channel is invalid");
		return -EINVAL;
	}

	hdd_info("First connection channel No.:%d and quota:%dms",
			operating_channel, set_value);
	/* Move the time quota for first channel to bits 15-8 */
	set_value = set_value << 8;

	/*
	 * Store the channel number of 1st channel at bits 7-0
	 * of the bit vector
	 */
	return set_value | operating_channel;
}

/**
 * set_second_connection_operating_channel() - Function to set
 * second connection oerating channel
 * @adapter:   adapter data
 * @set_value: Quota value for the interface
 * @vdev_id:  vdev id
 *
 * This function is used to set the first adapter operating
 * channel
 *
 * Return: operating channel updated in set value
 *
 */
static uint32_t set_second_connection_operating_channel(
		struct hdd_context *hdd_ctx, uint32_t set_value,
		uint8_t vdev_id)
{
	uint8_t operating_channel;

	operating_channel = policy_mgr_get_mcc_operating_channel(
		hdd_ctx->hdd_psoc, vdev_id);

	if (operating_channel == 0) {
		hdd_err("Second adapter operating channel is invalid");
		return -EINVAL;
	}

	hdd_info("Second connection channel No.:%d and quota:%dms",
			operating_channel, set_value);
	/*
	 * Now move the time quota and channel number of the
	 * 1st adapter to bits 23-16 and bits 15-8 of the bit
	 * vector, respectively.
	 */
	set_value = set_value << 8;

	/*
	 * Set the channel number for 2nd MCC vdev at bits
	 * 7-0 of set_value
	 */
	return set_value | operating_channel;
}

/**
 * wlan_hdd_set_mcc_p2p_quota() - Function to set quota for P2P
 * @psoc: PSOC object information
 * @set_value:          Qouta value for the interface
 * @operating_channel   First adapter operating channel
 * @vdev_id             vdev id
 *
 * This function is used to set the quota for P2P cases
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
int wlan_hdd_set_mcc_p2p_quota(struct hdd_adapter *adapter,
			       uint32_t set_value)
{
	int32_t ret = 0;
	uint32_t concurrent_state;
	struct hdd_context *hdd_ctx;

	if (!adapter) {
		hdd_err("Invalid adapter");
		return -EFAULT;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx == NULL) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	concurrent_state = policy_mgr_get_concurrency_mode(
		hdd_ctx->hdd_psoc);
	/*
	 * Check if concurrency mode is active.
	 * Need to modify this code to support MCC modes other than STA/P2P
	 */
	if ((concurrent_state ==
	     (QDF_STA_MASK | QDF_P2P_CLIENT_MASK)) ||
	    (concurrent_state == (QDF_STA_MASK | QDF_P2P_GO_MASK))) {
		hdd_info("STA & P2P are both enabled");

		/*
		 * The channel numbers for both adapters and the time
		 * quota for the 1st adapter, i.e., one specified in cmd
		 * are formatted as a bit vector then passed on to WMA
		 * +***********************************************************+
		 * |bit 31-24  | bit 23-16  |   bits 15-8   |   bits 7-0       |
		 * |  Unused   | Quota for  | chan. # for   |   chan. # for    |
		 * |           | 1st chan.  | 1st chan.     |   2nd chan.      |
		 * +***********************************************************+
		 */

		set_value = set_first_connection_operating_channel(
			hdd_ctx, set_value, adapter->device_mode);


		set_value = set_second_connection_operating_channel(
			hdd_ctx, set_value, adapter->session_id);


		ret = wlan_hdd_send_p2p_quota(adapter, set_value);
	} else {
		hdd_info("MCC is not active. Exit w/o setting latency");
	}

	return ret;
}

int wlan_hdd_go_set_mcc_p2p_quota(struct hdd_adapter *hostapd_adapter,
				  uint32_t set_value)
{
	return wlan_hdd_set_mcc_p2p_quota(hostapd_adapter, set_value);
}

void wlan_hdd_set_mcc_latency(struct hdd_adapter *adapter, int set_value)
{
	uint32_t concurrent_state;
	struct hdd_context *hdd_ctx;

	if (!adapter) {
		hdd_err("Invalid adapter");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx == NULL) {
		hdd_err("HDD context is null");
		return;
	}

	concurrent_state = policy_mgr_get_concurrency_mode(
		hdd_ctx->hdd_psoc);
	/**
	 * Check if concurrency mode is active.
	 * Need to modify this code to support MCC modes other than STA/P2P
	 */
	if ((concurrent_state ==
	     (QDF_STA_MASK | QDF_P2P_CLIENT_MASK)) ||
	    (concurrent_state == (QDF_STA_MASK | QDF_P2P_GO_MASK))) {
		hdd_info("STA & P2P are both enabled");
		/*
		 * The channel number and latency are formatted in
		 * a bit vector then passed on to WMA layer.
		 * +**********************************************+
		 * |bits 31-16 |      bits 15-8    |  bits 7-0    |
		 * |  Unused   | latency - Chan. 1 |  channel no. |
		 * +**********************************************+
		 */
		set_value = set_first_connection_operating_channel(
			hdd_ctx, set_value, adapter->device_mode);

		wlan_hdd_send_mcc_latency(adapter, set_value);
	} else {
		hdd_info("MCC is not active. Exit w/o setting latency");
	}
}
