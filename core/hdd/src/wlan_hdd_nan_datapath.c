/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include "osif_sync.h"
#include "wma_api.h"
#include "wlan_hdd_assoc.h"
#include "sme_nan_datapath.h"
#include "wlan_hdd_object_manager.h"
#include <qca_vendor.h>
#include "os_if_nan.h"
#include "wlan_nan_api.h"
#include "nan_public_structs.h"
#include "cfg_nan_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "qdf_util.h"

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
					struct wma_tgt_cfg *tgt_cfg)
{
	hdd_ctx->nan_datapath_enabled =
			cfg_nan_get_datapath_enable(hdd_ctx->psoc) &&
			tgt_cfg->nan_datapath_enabled;
	hdd_debug("NAN Datapath Enable: %d (Host: %d FW: %d)",
		  hdd_ctx->nan_datapath_enabled,
		  cfg_nan_get_datapath_enable(hdd_ctx->psoc),
		  tgt_cfg->nan_datapath_enabled);
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
	hdd_deregister_hl_netdev_fc_timer(adapter);
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
	QDF_STATUS status;
	uint32_t roam_id;
	struct csr_roam_profile *roam_profile;
	mac_handle_t mac_handle;
	uint8_t wmm_mode = 0;
	struct hdd_context *hdd_ctx;
	uint8_t value = 0;

	hdd_enter();

	roam_profile = hdd_roam_profile(adapter);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = ucfg_mlme_get_wmm_mode(hdd_ctx->psoc, &wmm_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("Get wmm_mode failed");
		return -EINVAL;
	}

	if (HDD_WMM_USER_MODE_NO_QOS == wmm_mode) {
		/* QoS not enabled in cfg file*/
		roam_profile->uapsd_mask = 0;
	} else {
		/* QoS enabled, update uapsd mask from cfg file*/
		status = ucfg_mlme_get_wmm_uapsd_mask(hdd_ctx->psoc, &value);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hdd_err("Get uapsd_mask failed");
			return -EINVAL;
		}
		roam_profile->uapsd_mask = value;
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

	mac_handle = hdd_adapter_get_mac_handle(adapter);
	status = sme_roam_connect(mac_handle, adapter->vdev_id,
				  roam_profile, &roam_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("NDI sme_RoamConnect session %d failed with status %d -> NotConnected",
			adapter->vdev_id, status);
		/* change back to NotConnected */
		hdd_conn_set_connection_state(adapter,
					      eConnectionState_NotConnected);
	} else {
		hdd_info("sme_RoamConnect issued successfully for NDI");
	}

	roam_profile->ChannelInfo.ChannelList = NULL;
	roam_profile->ChannelInfo.numOfChannels = 0;

	hdd_exit();

	return 0;
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
	bool found;

	for (attempts = 0; attempts < max_attempt; attempts++) {
		found = false;
		/* if NDI is present next addr is required to be 1 bit apart  */
		adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
		if (adapter) {
			hdd_debug("NDI already exists, deriving next mac");
			qdf_mem_copy(mac_addr, &adapter->mac_addr,
				     sizeof(*mac_addr));
			qdf_get_random_bytes(&pos, sizeof(pos));
			/* skipping byte 0, 5 leaves 8*4=32 positions */
			pos = pos % 32;
			bit_pos = pos % 8;
			byte_pos = pos / 8;
			mask = 1 << bit_pos;
			/* flip the required bit */
			mac_addr->bytes[byte_pos + 1] ^= mask;
		} else {
			qdf_get_random_bytes(mac_addr, sizeof(*mac_addr));
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
		for (i = 0; i < hdd_ctx->num_provisioned_addr; i++) {
			if ((!qdf_mem_cmp(hdd_ctx->
					  provisioned_mac_addr[i].bytes,
			      mac_addr, sizeof(*mac_addr)))) {
				found = true;
				break;
			}
		}

		if (found)
			continue;

		for (i = 0; i < hdd_ctx->num_derived_addr; i++) {
			if ((!qdf_mem_cmp(hdd_ctx->
					  derived_mac_addr[i].bytes,
			      mac_addr, sizeof(*mac_addr)))) {
				found = true;
				break;
			}
		}
		if (found)
			continue;

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
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(adapter->vdev);

	if (roam_status == eCSR_ROAM_NDP_STATUS_UPDATE) {
		switch (roam_result) {
		case eCSR_ROAM_RESULT_NDI_CREATE_RSP:
			success = (roam_info->ndp.ndi_create_params.status ==
					NAN_DATAPATH_RSP_STATUS_SUCCESS);
			hdd_debug("posting ndi create status: %d to umac",
				success);
			os_if_nan_post_ndi_create_rsp(psoc, adapter->vdev_id,
							success);
			return;
		case eCSR_ROAM_RESULT_NDI_DELETE_RSP:
			success = (roam_info->ndp.ndi_create_params.status ==
					NAN_DATAPATH_RSP_STATUS_SUCCESS);
			hdd_debug("posting ndi delete status: %d to umac",
				success);
			os_if_nan_post_ndi_delete_rsp(psoc, adapter->vdev_id,
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
		hdd_err_rl("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (!WLAN_HDD_IS_NDP_ENABLED(hdd_ctx)) {
		hdd_err_rl("NAN datapath is not enabled");
		return -EPERM;
	}
	/* NAN data path coexists only with STA interface */
	if (false == hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err_rl("Unsupported concurrency for NAN datapath");
		return -EPERM;
	}

	/* NAN data path coexists only with STA interface */
	if (false == hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err_rl("Unsupported concurrency for NAN datapath");
		return -EPERM;
	}

	return os_if_nan_process_ndp_cmd(hdd_ctx->psoc,
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
				      struct wireless_dev *wdev,
				      const void *data, int data_len)
{
	/* This call is intentionally not protected by op_start/op_stop, due to
	 * the various protection needs of the callbacks dispatched within.
	 */
	return __wlan_hdd_cfg80211_process_ndp_cmd(wiphy, wdev,
						   data, data_len);
}

static int update_ndi_state(struct hdd_adapter *adapter, uint32_t state)
{
	return os_if_nan_set_ndi_state(adapter->vdev, state);
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
	int32_t ret_val;
	mac_handle_t mac_handle;
	bool bval = false;

	ret_val = hdd_vdev_create(adapter, hdd_sme_roam_callback, adapter);
	if (ret_val) {
		hdd_err("failed to create vdev: %d", ret_val);
		return ret_val;
	}

	mac_handle = hdd_ctx->mac_handle;

	/* Configure self HT/VHT capabilities */
	sme_set_curr_device_mode(mac_handle, adapter->device_mode);

	status = ucfg_mlme_get_vht_enable2x2(hdd_ctx->psoc, &bval);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("unable to get vht_enable2x2");

	sme_set_pdev_ht_vht_ies(mac_handle, bval);
	sme_set_vdev_ies_per_band(mac_handle, adapter->vdev_id);

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

	ret_val = wma_cli_set_command((int)adapter->vdev_id,
			(int)WMI_PDEV_PARAM_BURST_ENABLE,
			(int)HDD_ENABLE_SIFS_BURST_DEFAULT,
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

int hdd_ndi_open(char *iface_name)
{
	struct hdd_adapter *adapter;
	struct qdf_mac_addr random_ndi_mac;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	uint8_t ndi_adapter_count = 0;
	uint8_t *ndi_mac_addr;

	hdd_enter();
	if (!hdd_ctx) {
		hdd_err("hdd_ctx null");
		return -EINVAL;
	}

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (WLAN_HDD_IS_NDI(adapter))
			ndi_adapter_count++;
	}
	if (ndi_adapter_count >= MAX_NDI_ADAPTERS) {
		hdd_err("Can't allow more than %d NDI adapters",
			MAX_NDI_ADAPTERS);
		return -EINVAL;
	}

	if (cfg_nan_get_ndi_mac_randomize(hdd_ctx->psoc)) {
		if (hdd_get_random_nan_mac_addr(hdd_ctx, &random_ndi_mac)) {
			hdd_err("get random mac address failed");
			return -EFAULT;
		}
		ndi_mac_addr = &random_ndi_mac.bytes[0];
	} else {
		ndi_mac_addr = wlan_hdd_get_intf_addr(hdd_ctx, QDF_NDI_MODE);
		if (!ndi_mac_addr) {
			hdd_err("get intf address failed");
			return -EFAULT;
		}
	}

	adapter = hdd_open_adapter(hdd_ctx, QDF_NDI_MODE, iface_name,
				   ndi_mac_addr, NET_NAME_UNKNOWN, true);
	if (!adapter) {
		hdd_err("hdd_open_adapter failed");
		return -EINVAL;
	}

	hdd_exit();
	return 0;
}

int hdd_ndi_start(char *iface_name, uint16_t transaction_id)
{
	int ret;
	QDF_STATUS status;
	uint8_t op_channel;
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	hdd_enter();
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return -EINVAL;
	}

	adapter = hdd_get_adapter_by_iface_name(hdd_ctx, iface_name);
	if (!adapter) {
		hdd_err("adapter is null");
		return -EINVAL;
	}

	/* create nan vdev */
	status = hdd_init_nan_data_mode(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("failed to init nan data intf, status :%d", status);
		ret = -EFAULT;
		goto err_handler;
	}

	/*
	 * Create transaction id is required to be saved since the firmware
	 * does not honor the transaction id for create request
	 */
	ucfg_nan_set_ndp_create_transaction_id(adapter->vdev,
					       transaction_id);
	ucfg_nan_set_ndi_state(adapter->vdev,
			       NAN_DATA_NDI_CREATING_STATE);

	/*
	 * The NAN data interface has been created at this point.
	 * Unlike traditional device modes, where the higher application
	 * layer initiates connect / join / start, the NAN data
	 * interface does not have any such formal requests. The NDI
	 * create request is responsible for starting the BSS as well.
	 * Use the 5GHz Band NAN Social channel for BSS start if target
	 * supports it, since a 2.4GHz channel will require a DBS HW mode change
	 * first on a DBS 2x2 MAC target. Use a 2.4 GHz Band NAN Social channel
	 * if the target is not 5GHz capable.
	 */

	if (hdd_is_5g_supported(hdd_ctx))
		op_channel = NAN_SOCIAL_CHANNEL_5GHZ_LOWER_BAND;
	else
		op_channel = NAN_SOCIAL_CHANNEL_2_4GHZ;

	if (hdd_ndi_start_bss(adapter, op_channel)) {
		hdd_err("NDI start bss failed");
		ret = -EFAULT;
		goto err_handler;
	}

	hdd_exit();
	return 0;

err_handler:

	/* Start BSS failed, delete the interface */
	hdd_close_ndi(adapter);
	return ret;
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
	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_sta_id] = 0;
	sta_ctx->broadcast_sta_id = HDD_WLAN_INVALID_STA_ID;

	os_if_nan_set_ndp_delete_transaction_id(adapter->vdev,
						transaction_id);
	os_if_nan_set_ndi_state(adapter->vdev, NAN_DATA_NDI_DELETING_STATE);

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
	struct csr_roam_info *roam_info;
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

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return;

	if (ndi_rsp->status == QDF_STATUS_SUCCESS) {
		hdd_alert("NDI interface successfully created");
		os_if_nan_set_ndp_create_transaction_id(adapter->vdev, 0);
		os_if_nan_set_ndi_state(adapter->vdev,
					NAN_DATA_NDI_CREATED_STATE);
		wlan_hdd_netif_queue_control(adapter,
					WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);
	} else {
		hdd_alert("NDI interface creation failed with reason %d",
			ndi_rsp->reason /* create_reason */);
	}

	sta_ctx->broadcast_sta_id = ndi_rsp->sta_id;
	hdd_save_peer(sta_ctx, sta_ctx->broadcast_sta_id, &bc_mac_addr);
	hdd_roam_register_sta(adapter, roam_info,
			      sta_ctx->broadcast_sta_id,
			      &tmp_bss_descp);
	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_sta_id] = adapter;
	qdf_mem_free(roam_info);
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

	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_sta_id] = NULL;
	hdd_roam_deregister_sta(adapter, sta_ctx->broadcast_sta_id);
	hdd_delete_peer(sta_ctx, sta_ctx->broadcast_sta_id);
	sta_ctx->broadcast_sta_id = HDD_WLAN_INVALID_STA_ID;

	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	complete(&adapter->disconnect_comp_var);
}

void hdd_ndp_session_end_handler(struct hdd_adapter *adapter)
{
	os_if_nan_ndi_session_end(adapter->vdev);
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
	struct csr_roam_info *roam_info;

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

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return -ENOMEM;

	/* this function is called for each new peer */
	hdd_roam_register_sta(adapter, roam_info, sta_id, &tmp_bss_descp);
	hdd_ctx->sta_to_adapter[sta_id] = adapter;
	/* perform following steps for first new peer ind */
	if (fist_peer) {
		hdd_info("Set ctx connection state to connected");
		sta_ctx->conn_info.conn_state = eConnectionState_NdiConnected;
		hdd_wmm_connect(adapter, roam_info, eCSR_BSS_TYPE_NDI);
		wlan_hdd_netif_queue_control(adapter,
				WLAN_WAKE_ALL_NETIF_QUEUE, WLAN_CONTROL_PATH);
	}
	qdf_mem_free(roam_info);
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
		sta_ctx->conn_info.conn_state = eConnectionState_NdiDisconnected;
		hdd_conn_set_connection_state(adapter,
			eConnectionState_NdiDisconnected);
		hdd_info("Stop netif tx queues.");
		wlan_hdd_netif_queue_control(adapter, WLAN_STOP_ALL_NETIF_QUEUE,
					     WLAN_CONTROL_PATH);
	}
	hdd_exit();
}
