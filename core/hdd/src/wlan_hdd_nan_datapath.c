/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_nan_datapath.c
 *
 * WLAN Host Device Driver nan datapath API implementation
 */
#include <wlan_hdd_includes.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include "wlan_hdd_includes.h"
#include "wlan_hdd_p2p.h"
#include "wma_api.h"
#include "wlan_hdd_assoc.h"
#include "sme_nan_datapath.h"
#include "wlan_hdd_object_manager.h"
#include <qca_vendor.h>
#include "os_if_nan.h"
#include "wlan_nan_api.h"
#include "nan_public_structs.h"

/**
 * hdd_ndp_print_ini_config()- Print nan datapath specific INI configuration
 * @hdd_ctx: handle to hdd context
 *
 * Return: None
 */
void hdd_ndp_print_ini_config(struct hdd_context *hdd_ctx)
{
	hdd_debug("Name = [%s] Value = [%u]", CFG_ENABLE_NAN_DATAPATH_NAME,
		hdd_ctx->config->enable_nan_datapath);
	hdd_debug("Name = [%s] Value = [%u]", CFG_ENABLE_NAN_NDI_CHANNEL_NAME,
		hdd_ctx->config->nan_datapath_ndi_channel);
}

/**
 * hdd_nan_datapath_target_config() - Configure NAN datapath features
 * @hdd_ctx: Pointer to HDD context
 * @cfg: Pointer to target device capability information
 *
 * NAN datapath functionality is enabled if it is enabled in
 * .ini file and also supported on target device.
 *
 * Return: None
 */
void hdd_nan_datapath_target_config(struct hdd_context *hdd_ctx,
					struct wma_tgt_cfg *cfg)
{
	hdd_ctx->nan_datapath_enabled =
		hdd_ctx->config->enable_nan_datapath &&
			cfg->nan_datapath_enabled;
	hdd_info("enable_nan_datapath: final: %d, host: %d, fw: %d",
		hdd_ctx->nan_datapath_enabled,
		hdd_ctx->config->enable_nan_datapath,
		cfg->nan_datapath_enabled);
}

/**
 * hdd_close_ndi() - close NAN Data interface
 * @adapter: adapter context
 *
 * Close the adapter if start BSS fails
 *
 * Returns: 0 on success, negative error code otherwise
 */
static int hdd_close_ndi(struct hdd_adapter *adapter)
{
	int errno;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_enter();

	/* check if the adapter is in NAN Data mode */
	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err("Interface is not in NDI mode");
		return -EINVAL;
	}
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

#ifdef WLAN_OPEN_SOURCE
	cancel_work_sync(&adapter->ipv4_notifier_work);
#endif
	hdd_deregister_tx_flow_control(adapter);

#ifdef WLAN_NS_OFFLOAD
#ifdef WLAN_OPEN_SOURCE
	cancel_work_sync(&adapter->ipv6_notifier_work);
#endif
#endif
	errno = hdd_vdev_destroy(adapter);
	if (errno)
		hdd_err("failed to destroy vdev: %d", errno);

	/* We are good to close the adapter */
	hdd_close_adapter(hdd_ctx, adapter, true);

	hdd_exit();
	return 0;
}

/**
 * hdd_is_ndp_allowed() - Indicates if NDP is allowed
 * @hdd_ctx: hdd context
 *
 * NDP is not allowed with any other role active except STA.
 *
 * Return:  true if allowed, false otherwise
 */
static bool hdd_is_ndp_allowed(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;

	hdd_for_each_adapter(hdd_ctx, adapter) {
		switch (adapter->device_mode) {
		case QDF_P2P_GO_MODE:
		case QDF_SAP_MODE:
			if (test_bit(SOFTAP_BSS_STARTED,
					&adapter->event_flags))
				return false;
			break;
		case QDF_P2P_CLIENT_MODE:
		case QDF_IBSS_MODE:
			sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
			if (hdd_conn_is_connected(sta_ctx) ||
					hdd_is_connecting(sta_ctx))
				return false;
			break;
		default:
			break;
		}
	}

	return true;
}

/**
 * hdd_ndi_start_bss() - Start BSS on NAN data interface
 * @adapter: adapter context
 * @operating_channel: channel on which the BSS to be started
 *
 * Return: 0 on success, error value on failure
 */
static int hdd_ndi_start_bss(struct hdd_adapter *adapter,
				uint8_t operating_channel)
{
	int ret;
	uint32_t roam_id;
	struct csr_roam_profile *roam_profile;

	hdd_enter();

	roam_profile = hdd_roam_profile(adapter);

	if (HDD_WMM_USER_MODE_NO_QOS ==
		(WLAN_HDD_GET_CTX(adapter))->config->WmmMode) {
		/* QoS not enabled in cfg file*/
		roam_profile->uapsd_mask = 0;
	} else {
		/* QoS enabled, update uapsd mask from cfg file*/
		roam_profile->uapsd_mask =
			(WLAN_HDD_GET_CTX(adapter))->config->UapsdMask;
	}

	roam_profile->csrPersona = adapter->device_mode;

	if (!operating_channel)
		operating_channel = NAN_SOCIAL_CHANNEL_2_4GHZ;

	roam_profile->ChannelInfo.numOfChannels = 1;
	roam_profile->ChannelInfo.ChannelList = &operating_channel;

	roam_profile->SSIDs.numOfSSIDs = 1;
	roam_profile->SSIDs.SSIDList->SSID.length = 0;

	roam_profile->phyMode = eCSR_DOT11_MODE_11ac;
	roam_profile->BSSType = eCSR_BSS_TYPE_NDI;
	roam_profile->BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy((void *)(roam_profile->BSSIDs.bssid),
		&adapter->mac_addr.bytes[0],
		QDF_MAC_ADDR_SIZE);

	roam_profile->AuthType.numEntries = 1;
	roam_profile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	roam_profile->EncryptionType.numEntries = 1;
	roam_profile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;

	ret = sme_roam_connect(WLAN_HDD_GET_HAL_CTX(adapter),
		adapter->session_id, roam_profile, &roam_id);
	if (QDF_STATUS_SUCCESS != ret) {
		hdd_err("NDI sme_RoamConnect session %d failed with status %d -> NotConnected",
			adapter->session_id, ret);
		/* change back to NotConnected */
		hdd_conn_set_connection_state(adapter,
			eConnectionState_NotConnected);
	} else {
		hdd_info("sme_RoamConnect issued successfully for NDI");
	}

	roam_profile->ChannelInfo.ChannelList = NULL;
	roam_profile->ChannelInfo.numOfChannels = 0;

	hdd_exit();

	return ret;
}

/**
 * hdd_get_random_nan_mac_addr() - generate random non pre-existent mac address
 * @hdd_ctx: hdd context pointer
 * @mac_addr: mac address buffer to populate
 *
 * Return: status of operation
 */
static int hdd_get_random_nan_mac_addr(struct hdd_context *hdd_ctx,
				       struct qdf_mac_addr *mac_addr)
{
	struct hdd_adapter *adapter;
	uint8_t pos, bit_pos, byte_pos, mask;
	uint8_t i, attempts, max_attempt = 16;

	for (attempts = 0; attempts < max_attempt; attempts++) {
		/* if NDI is present next addr is required to be 1 bit apart  */
		adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
		if (adapter) {
			hdd_debug("NDI already exists, deriving next mac");
			qdf_mem_copy(mac_addr, &adapter->mac_addr,
				     sizeof(*mac_addr));
			cds_rand_get_bytes(0, &pos, sizeof(pos));
			/* skipping byte 0, 5 leaves 8*4=32 positions */
			pos = pos % 32;
			bit_pos = pos % 8;
			byte_pos = pos / 8;
			mask = 1 << bit_pos;
			/* flip the required bit */
			mac_addr->bytes[byte_pos + 1] ^= mask;
		} else {
			cds_rand_get_bytes(0, (uint8_t *)mac_addr,
					   sizeof(*mac_addr));
			/*
			 * Reset multicast bit (bit-0) and set
			 * locally-administered bit
			 */
			mac_addr->bytes[0] = 0x2;

			/*
			 * to avoid potential conflict with FW's generated NMI
			 * mac addr, host sets LSB if 6th byte to 0
			 */
			mac_addr->bytes[5] &= 0xFE;
		}
		for (i = 0; i < QDF_MAX_CONCURRENCY_PERSONA; i++) {
			if (!qdf_mem_cmp(hdd_ctx->config->intfMacAddr[i].bytes,
					 mac_addr, sizeof(*mac_addr)))
				continue;
		}

		adapter = hdd_get_adapter_by_macaddr(hdd_ctx, mac_addr->bytes);
		if (!adapter)
			return 0;
	}

	hdd_err("unable to get non-pre-existing mac address in %d attempts",
		max_attempt);

	return -EINVAL;
}

void hdd_ndp_event_handler(struct hdd_adapter *adapter,
			   struct csr_roam_info *roam_info,
			   uint32_t roam_id, eRoamCmdStatus roam_status,
			   eCsrRoamResult roam_result)
{
	bool success;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(adapter->hdd_vdev);

	if (roam_status == eCSR_ROAM_NDP_STATUS_UPDATE) {
		switch (roam_result) {
		case eCSR_ROAM_RESULT_NDI_CREATE_RSP:
			success = (roam_info->ndp.ndi_create_params.status ==
					NAN_DATAPATH_RSP_STATUS_SUCCESS);
			hdd_debug("posting ndi create status: %d to umac",
				success);
			os_if_nan_post_ndi_create_rsp(psoc, adapter->session_id,
							success);
			return;
		case eCSR_ROAM_RESULT_NDI_DELETE_RSP:
			success = (roam_info->ndp.ndi_create_params.status ==
					NAN_DATAPATH_RSP_STATUS_SUCCESS);
			hdd_debug("posting ndi delete status: %d to umac",
				success);
			os_if_nan_post_ndi_delete_rsp(psoc, adapter->session_id,
							success);
			return;
		default:
			hdd_err("in correct roam_result: %d", roam_result);
			return;
		}
	} else {
		hdd_err("in correct roam_status: %d", roam_status);
		return;
	}
}

/**
 * __wlan_hdd_cfg80211_process_ndp_cmds() - handle NDP request
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This function is invoked to handle vendor command
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_process_ndp_cmd(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
	int ret_val;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	hdd_enter();

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (!WLAN_HDD_IS_NDP_ENABLED(hdd_ctx)) {
		hdd_err("NAN datapath is not enabled");
		return -EPERM;
	}
	/* NAN data path coexists only with STA interface */
	if (false == hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err("Unsupported concurrency for NAN datapath");
		return -EPERM;
	}

	/* NAN data path coexists only with STA interface */
	if (false == hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err("Unsupported concurrency for NAN datapath");
		return -EPERM;
	}

	return os_if_nan_process_ndp_cmd(hdd_ctx->hdd_psoc,
					 data, data_len);
}

/**
 * wlan_hdd_cfg80211_process_ndp_cmd() - handle NDP request
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This function is called to send a NAN request to
 * firmware. This is an SSR-protected wrapper function.
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_process_ndp_cmd(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_process_ndp_cmd(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

static int update_ndi_state(struct hdd_adapter *adapter, uint32_t state)
{
	return os_if_nan_set_ndi_state(adapter->hdd_vdev, state);
}

/**
 * hdd_init_nan_data_mode() - initialize nan data mode
 * @adapter: adapter context
 *
 * Returns: 0 on success negative error code on error
 */
int hdd_init_nan_data_mode(struct hdd_adapter *adapter)
{
	struct net_device *wlan_dev = adapter->dev;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;
	int32_t ret_val = 0;

	ret_val = hdd_vdev_create(adapter, hdd_sme_roam_callback, adapter);
	if (ret_val) {
		hdd_err("failed to create vdev: %d", ret_val);
		return ret_val;
	}

	/* Configure self HT/VHT capabilities */
	sme_set_curr_device_mode(hdd_ctx->hHal, adapter->device_mode);
	sme_set_pdev_ht_vht_ies(hdd_ctx->hHal, hdd_ctx->config->enable2x2);
	sme_set_vdev_ies_per_band(hdd_ctx->hHal, adapter->session_id);

	hdd_roam_profile_init(adapter);
	hdd_register_wext(wlan_dev);

	status = hdd_init_tx_rx(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("hdd_init_tx_rx() init failed, status %d", status);
		ret_val = -EAGAIN;
		goto error_init_txrx;
	}

	set_bit(INIT_TX_RX_SUCCESS, &adapter->event_flags);

	status = hdd_wmm_adapter_init(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("hdd_wmm_adapter_init() failed, status %d", status);
		ret_val = -EAGAIN;
		goto error_wmm_init;
	}

	set_bit(WMM_INIT_DONE, &adapter->event_flags);

	ret_val = wma_cli_set_command((int)adapter->session_id,
			(int)WMI_PDEV_PARAM_BURST_ENABLE,
			(int)hdd_ctx->config->enableSifsBurst,
			PDEV_CMD);
	if (0 != ret_val)
		hdd_err("WMI_PDEV_PARAM_BURST_ENABLE set failed %d", ret_val);


	update_ndi_state(adapter, NAN_DATA_NDI_CREATING_STATE);
	return ret_val;

error_wmm_init:
	clear_bit(INIT_TX_RX_SUCCESS, &adapter->event_flags);
	hdd_deinit_tx_rx(adapter);

error_init_txrx:
	hdd_unregister_wext(wlan_dev);

	QDF_BUG(!hdd_vdev_destroy(adapter));

	return ret_val;
}

struct wlan_objmgr_vdev *hdd_ndi_open(char *iface_name)
{
	struct hdd_adapter *adapter;
	struct qdf_mac_addr random_ndi_mac;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	uint8_t *ndi_mac_addr;

	hdd_enter();

	if (!hdd_ctx) {
		hdd_err("hdd_ctx null");
		return NULL;
	}

	if (hdd_ctx->config->is_ndi_mac_randomized) {
		if (hdd_get_random_nan_mac_addr(hdd_ctx, &random_ndi_mac)) {
			hdd_err("get random mac address failed");
			return NULL;
		}
		ndi_mac_addr = &random_ndi_mac.bytes[0];
	} else {
		ndi_mac_addr = wlan_hdd_get_intf_addr(hdd_ctx);
		if (!ndi_mac_addr) {
			hdd_err("get intf address failed");
			return NULL;
		}
	}

	adapter = hdd_open_adapter(hdd_ctx, QDF_NDI_MODE, iface_name,
				   ndi_mac_addr, NET_NAME_UNKNOWN, true);
	if (!adapter) {
		hdd_err("hdd_open_adapter failed");
		return NULL;
	}

	hdd_exit();
	return adapter->hdd_vdev;
}

int hdd_ndi_start(uint8_t vdev_id)
{
	uint8_t op_channel;
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	hdd_enter();

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return -EINVAL;
	}

	op_channel = hdd_ctx->config->nan_datapath_ndi_channel;
	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter is null");
		return -EINVAL;
	}

	/*
	 * The NAN data interface has been created at this point.
	 * Unlike traditional device modes, where the higher application
	 * layer initiates connect / join / start, the NAN data
	 * interface does not have any such formal requests. The NDI
	 * create request is responsible for starting the BSS as well.
	 */
	if (op_channel != NAN_SOCIAL_CHANNEL_2_4GHZ ||
		op_channel != NAN_SOCIAL_CHANNEL_5GHZ_LOWER_BAND ||
		op_channel != NAN_SOCIAL_CHANNEL_5GHZ_UPPER_BAND) {
		/* start NDI on the default 2.4 GHz social channel */
		op_channel = NAN_SOCIAL_CHANNEL_2_4GHZ;
	}
	if (hdd_ndi_start_bss(adapter, op_channel)) {
		hdd_err("NDI start bss failed");
		/* Start BSS failed, delete the interface */
		hdd_close_ndi(adapter);
		hdd_exit();
		return -EINVAL;
	}

	hdd_exit();
	return 0;
}

int hdd_ndi_delete(uint8_t vdev_id, char *iface_name, uint16_t transaction_id)
{
	int ret;
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return -EINVAL;
	}

	/* check if adapter by vdev_id is valid NDI */
	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter || !WLAN_HDD_IS_NDI(adapter)) {
		hdd_err("NAN data interface %s is not available", iface_name);
		return -EINVAL;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is NULL");
		return -EINVAL;
	}

	/* Since, the interface is being deleted, remove the broadcast id. */
	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_staid] = 0;
	sta_ctx->broadcast_staid = HDD_WLAN_INVALID_STA_ID;

	os_if_nan_set_ndp_delete_transaction_id(adapter->hdd_vdev,
						transaction_id);
	os_if_nan_set_ndi_state(adapter->hdd_vdev, NAN_DATA_NDI_DELETING_STATE);

	/* Delete the interface */
	ret = __wlan_hdd_del_virtual_intf(hdd_ctx->wiphy, &adapter->wdev);
	if (ret)
		hdd_err("NDI delete request failed");
	else
		hdd_err("NDI delete request successfully issued");

	return ret;
}

void hdd_ndi_drv_ndi_create_rsp_handler(uint8_t vdev_id,
				struct nan_datapath_inf_create_rsp *ndi_rsp)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	struct csr_roam_info roam_info = {0};
	struct bss_description tmp_bss_descp = {0};
	struct qdf_mac_addr bc_mac_addr = QDF_MAC_ADDR_BCAST_INIT;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter is null");
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is null");
		return;
	}

	if (ndi_rsp->status == QDF_STATUS_SUCCESS) {
		hdd_alert("NDI interface successfully created");
		os_if_nan_set_ndp_create_transaction_id(adapter->hdd_vdev, 0);
		os_if_nan_set_ndi_state(adapter->hdd_vdev,
					NAN_DATA_NDI_CREATED_STATE);
		wlan_hdd_netif_queue_control(adapter,
					WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);
	} else {
		hdd_alert("NDI interface creation failed with reason %d",
			ndi_rsp->reason /* create_reason */);
	}

	sta_ctx->broadcast_staid = ndi_rsp->sta_id;
	hdd_save_peer(sta_ctx, sta_ctx->broadcast_staid, &bc_mac_addr);
	hdd_roam_register_sta(adapter, &roam_info,
				sta_ctx->broadcast_staid,
				&bc_mac_addr, &tmp_bss_descp);
	if (hdd_objmgr_add_peer_object(adapter->hdd_vdev,
				 QDF_NDI_MODE, bc_mac_addr.bytes, false))
		hdd_err("Peer object "MAC_ADDRESS_STR" add fails!",
			MAC_ADDR_ARRAY(bc_mac_addr.bytes));

	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_staid] = adapter;
}

void hdd_ndi_close(uint8_t vdev_id)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter is null");
		return;
	}

	hdd_close_ndi(adapter);
}

void hdd_ndi_drv_ndi_delete_rsp_handler(uint8_t vdev_id)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter is null");
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is null");
		return;
	}

	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_staid] = NULL;
	hdd_roam_deregister_sta(adapter, sta_ctx->broadcast_staid);
	hdd_delete_peer(sta_ctx, sta_ctx->broadcast_staid);
	sta_ctx->broadcast_staid = HDD_WLAN_INVALID_STA_ID;

	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	complete(&adapter->disconnect_comp_var);
}

void hdd_ndp_session_end_handler(struct hdd_adapter *adapter)
{
	os_if_nan_ndi_session_end(adapter->hdd_vdev);
}

int hdd_ndp_get_peer_idx(uint8_t vdev_id, struct qdf_mac_addr *addr)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	return hdd_get_peer_idx(sta_ctx, addr);
}

/**
 * hdd_ndp_new_peer_handler() - NDP new peer indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
int hdd_ndp_new_peer_handler(uint8_t vdev_id, uint16_t sta_id,
			struct qdf_mac_addr *peer_mac_addr, bool fist_peer)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;
	struct bss_description tmp_bss_descp = {0};
	struct csr_roam_info roam_info = {0};

	hdd_enter();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return -EINVAL;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter is null");
		return -EINVAL;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is null");
		return -EINVAL;
	}

	/* save peer in ndp ctx */
	if (false == hdd_save_peer(sta_ctx, sta_id, peer_mac_addr)) {
		hdd_err("Ndp peer table full. cannot save new peer");
		return -EPERM;
	}

	/* this function is called for each new peer */
	hdd_roam_register_sta(adapter, &roam_info, sta_id,
				peer_mac_addr, &tmp_bss_descp);
	if (hdd_objmgr_add_peer_object(adapter->hdd_vdev,
			 QDF_NDI_MODE, peer_mac_addr->bytes, false))
		hdd_err("Peer object "MAC_ADDRESS_STR" add fails!",
			MAC_ADDR_ARRAY(peer_mac_addr->bytes));
	hdd_ctx->sta_to_adapter[sta_id] = adapter;
	/* perform following steps for first new peer ind */
	if (fist_peer) {
		hdd_info("Set ctx connection state to connected");
		sta_ctx->conn_info.connState = eConnectionState_NdiConnected;
		hdd_wmm_connect(adapter, &roam_info, eCSR_BSS_TYPE_NDI);
		wlan_hdd_netif_queue_control(adapter,
				WLAN_WAKE_ALL_NETIF_QUEUE, WLAN_CONTROL_PATH);
	}
	hdd_exit();
	return 0;
}


/**
 * hdd_ndp_peer_departed_handler() - Handle NDP peer departed indication
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
void hdd_ndp_peer_departed_handler(uint8_t vdev_id, uint16_t sta_id,
			struct qdf_mac_addr *peer_mac_addr, bool last_peer)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	struct hdd_station_ctx *sta_ctx;

	hdd_enter();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return;
	}

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("adapter is null");
		return;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is null");
		return;
	}

	hdd_roam_deregister_sta(adapter, sta_id);
	hdd_delete_peer(sta_ctx, sta_id);
	hdd_ctx->sta_to_adapter[sta_id] = NULL;

	if (last_peer) {
		hdd_info("No more ndp peers.");
		sta_ctx->conn_info.connState = eConnectionState_NdiDisconnected;
		hdd_conn_set_connection_state(adapter,
			eConnectionState_NdiDisconnected);
		hdd_info("Stop netif tx queues.");
		wlan_hdd_netif_queue_control(adapter, WLAN_STOP_ALL_NETIF_QUEUE,
					     WLAN_CONTROL_PATH);
	}
	hdd_exit();
}
