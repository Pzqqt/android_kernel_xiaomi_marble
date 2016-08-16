/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/* NLA policy */
static const struct nla_policy
qca_wlan_vendor_ndp_policy[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR] = { .type = NLA_STRING,
					.len = IFNAMSIZ },
	[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR] = {
					.type = NLA_BINARY,
					.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS] = { .type = NLA_BINARY,
					.len = NDP_QOS_INFO_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO] = { .type = NLA_BINARY,
					.len = NDP_APP_INFO_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR] = { .type = NLA_BINARY,
					.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY] = { .type = NLA_BINARY,
					.len = NDP_NUM_INSTANCE_ID },
};

/**
 * hdd_ndp_print_ini_config()- Print nan datapath specific INI configuration
 * @hdd_ctx: handle to hdd context
 *
 * Return: None
 */
void hdd_ndp_print_ini_config(hdd_context_t *hdd_ctx)
{
	hdd_info("Name = [%s] Value = [%u]", CFG_ENABLE_NAN_DATAPATH_NAME,
		hdd_ctx->config->enable_nan_datapath);
	hdd_info("Name = [%s] Value = [%u]", CFG_ENABLE_NAN_NDI_CHANNEL_NAME,
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
void hdd_nan_datapath_target_config(hdd_context_t *hdd_ctx,
					struct wma_tgt_cfg *cfg)
{
	hdd_ctx->nan_datapath_enabled =
		hdd_ctx->config->enable_nan_datapath &&
			cfg->nan_datapath_enabled;
	hdd_info("enable_nan_datapath: %d", hdd_ctx->nan_datapath_enabled);
}

/**
 * hdd_close_ndi() - close NAN Data interface
 * @adapter: adapter context
 *
 * Close the adapter if start BSS fails
 *
 * Returns: 0 on success, negative error code otherwise
 */
static int hdd_close_ndi(hdd_adapter_t *adapter)
{
	int rc;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint32_t timeout = WLAN_WAIT_TIME_SESSIONOPENCLOSE;

	ENTER();

	/* check if the adapter is in NAN Data mode */
	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err("Interface is not in NDI mode");
		return -EINVAL;
	}
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_NETIF_TX_DISABLE_N_CARRIER,
				     WLAN_CONTROL_PATH);

#ifdef WLAN_OPEN_SOURCE
	cancel_work_sync(&adapter->ipv4NotifierWorkQueue);
#endif
	hdd_deregister_tx_flow_control(adapter);

#ifdef WLAN_NS_OFFLOAD
#ifdef WLAN_OPEN_SOURCE
	cancel_work_sync(&adapter->ipv6NotifierWorkQueue);
#endif
#endif
	/* check if the session is open */
	if (test_bit(SME_SESSION_OPENED, &adapter->event_flags)) {
		INIT_COMPLETION(adapter->session_close_comp_var);
		if (QDF_STATUS_SUCCESS == sme_close_session(hdd_ctx->hHal,
				adapter->sessionId,
				hdd_sme_close_session_callback, adapter)) {
			/* Block on a timed completion variable */
			rc = wait_for_completion_timeout(
				&adapter->session_close_comp_var,
				msecs_to_jiffies(timeout));
			if (!rc)
				hdd_err("session close timeout");
		}
	}

	/* We are good to close the adapter */
	hdd_close_adapter(hdd_ctx, adapter, true);

	EXIT();
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
static bool hdd_is_ndp_allowed(hdd_context_t *hdd_ctx)
{
	hdd_adapter_t *adapter;
	hdd_station_ctx_t *sta_ctx;
	QDF_STATUS status;
	hdd_adapter_list_node_t *curr = NULL, *next = NULL;

	status = hdd_get_front_adapter(hdd_ctx, &curr);
	while (QDF_STATUS_SUCCESS == status) {
		adapter = curr->pAdapter;
		if (!adapter)
			goto next_adapter;

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
next_adapter:
		status = hdd_get_next_adapter(hdd_ctx, curr, &next);
		curr = next;
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
static int hdd_ndi_start_bss(hdd_adapter_t *adapter,
				uint8_t operating_channel)
{
	int ret;
	uint32_t roam_id;
	hdd_wext_state_t *wext_state =
		WLAN_HDD_GET_WEXT_STATE_PTR(adapter);
	tCsrRoamProfile *roam_profile = &wext_state->roamProfile;

	ENTER();

	if (!roam_profile) {
		hdd_err("No valid roam profile");
		return -EINVAL;
	}

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

	roam_profile->ChannelInfo.numOfChannels = 1;
	if (operating_channel) {
		roam_profile->ChannelInfo.ChannelList = &operating_channel;
	} else {
		roam_profile->ChannelInfo.ChannelList[0] =
			NAN_SOCIAL_CHANNEL_2_4GHZ;
	}

	roam_profile->SSIDs.numOfSSIDs = 1;
	roam_profile->SSIDs.SSIDList->SSID.length = 0;

	roam_profile->phyMode = eCSR_DOT11_MODE_11ac;
	roam_profile->BSSType = eCSR_BSS_TYPE_NDI;
	roam_profile->BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy((void *)(roam_profile->BSSIDs.bssid),
		&adapter->macAddressCurrent.bytes[0],
		QDF_MAC_ADDR_SIZE);

	roam_profile->AuthType.numEntries = 1;
	roam_profile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	roam_profile->EncryptionType.numEntries = 1;
	roam_profile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;

	ret = sme_roam_connect(WLAN_HDD_GET_HAL_CTX(adapter),
		adapter->sessionId, roam_profile, &roam_id);
	if (QDF_STATUS_SUCCESS != ret) {
		hdd_err("NDI sme_RoamConnect session %d failed with status %d -> NotConnected",
			adapter->sessionId, ret);
		/* change back to NotConnected */
		hdd_conn_set_connection_state(adapter,
			eConnectionState_NotConnected);
	} else {
		hdd_info("sme_RoamConnect issued successfully for NDI");
	}

	roam_profile->ChannelInfo.ChannelList = NULL;
	roam_profile->ChannelInfo.numOfChannels = 0;

	EXIT();

	return ret;
}


/**
 * hdd_ndi_create_req_handler() - NDI create request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return: 0 on success or error code on failure
 */
static int hdd_ndi_create_req_handler(hdd_context_t *hdd_ctx,
						struct nlattr **tb)
{
	hdd_adapter_t *adapter;
	char *iface_name;
	uint16_t transaction_id;
	int ret;
	struct nan_datapath_ctx *ndp_ctx;
	uint8_t op_channel =
		hdd_ctx->config->nan_datapath_ndi_channel;

	ENTER();

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		hdd_err("Interface name string is unavailable");
		return -EINVAL;
	}
	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err("transaction id is unavailable");
		return -EINVAL;
	}
	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	/* Check for an existing interface of NDI type */
	adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
	if (adapter) {
		hdd_err("Cannot support more than one NDI");
		return -EEXIST;
	}

	adapter = hdd_open_adapter(hdd_ctx, QDF_NDI_MODE, iface_name,
			wlan_hdd_get_intf_addr(hdd_ctx), NET_NAME_UNKNOWN,
			true);
	if (!adapter) {
		hdd_err("hdd_open_adapter failed");
		return -ENOMEM;
	}

	/*
	 * Create transaction id is required to be saved since the firmware
	 * does not honor the transaction id for create request
	 */
	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	ndp_ctx->ndp_create_transaction_id = transaction_id;
	ndp_ctx->state = NAN_DATA_NDI_CREATING_STATE;

	/*
	 * The NAN data interface has been created at this point.
	 * Unlike traditional device modes, where the higher application
	 * layer initiates connect / join / start, the NAN data interface
	 * does not have any such formal requests. The NDI create request
	 * is responsible for starting the BSS as well.
	 */
	if (op_channel != NAN_SOCIAL_CHANNEL_2_4GHZ ||
	    op_channel != NAN_SOCIAL_CHANNEL_5GHZ_LOWER_BAND ||
	    op_channel != NAN_SOCIAL_CHANNEL_5GHZ_UPPER_BAND) {
		/* start NDI on the default 2.4 GHz social channel */
		op_channel = NAN_SOCIAL_CHANNEL_2_4GHZ;
	}
	ret = hdd_ndi_start_bss(adapter, op_channel);
	if (0 > ret) {
		hdd_err("NDI start bss failed");
		/* Start BSS failed, delete the interface */
		hdd_close_ndi(adapter);
	}

	EXIT();
	return ret;
}

/**
 * hdd_ndi_delete_req_handler() - NDI delete request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return: 0 on success or error code on failure
 */
static int hdd_ndi_delete_req_handler(hdd_context_t *hdd_ctx,
						struct nlattr **tb)
{
	hdd_adapter_t *adapter;
	char *iface_name;
	uint16_t transaction_id;
	struct nan_datapath_ctx *ndp_ctx;
	int ret;
	hdd_station_ctx_t *sta_ctx;

	ENTER();

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		hdd_err("Interface name string is unavailable");
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err("Transaction id is unavailable");
		return -EINVAL;
	}

	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	/* Check if there is already an existing inteface with the same name */
	adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
	if (!adapter) {
		hdd_err("NAN data interface %s is not available", iface_name);
		return -EINVAL;
	}

	/* check if adapter is in NDI mode */
	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err("Interface %s is not in NDI mode", iface_name);
		return -EINVAL;
	}

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	if (!ndp_ctx) {
		hdd_err("ndp_ctx is NULL");
		return -EINVAL;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is NULL");
		return -EINVAL;
	}

	/* check if there are active peers on the adapter */
	if (ndp_ctx->active_ndp_peers > 0) {
		hdd_err("NDP peers active: %d, cannot delete NDI",
			ndp_ctx->active_ndp_peers);
		return -EINVAL;
	}

	/*
	 * Since, the interface is being deleted, remove the
	 * broadcast id.
	 */
	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_staid] = 0;
	sta_ctx->broadcast_staid = HDD_WLAN_INVALID_STA_ID;

	ndp_ctx->ndp_delete_transaction_id = transaction_id;
	ndp_ctx->state = NAN_DATA_NDI_DELETING_STATE;

	/* Delete the interface */
	ret = __wlan_hdd_del_virtual_intf(hdd_ctx->wiphy, &adapter->wdev);
	if (ret < 0)
		hdd_err("NDI delete request failed");
	else
		hdd_err("NDI delete request successfully issued");

	return ret;
}

/**
 * hdd_ndp_initiator_req_handler() - NDP initiator request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return:  0 on success or error code on failure
 */
static int hdd_ndp_initiator_req_handler(hdd_context_t *hdd_ctx,
						struct nlattr **tb)
{
	hdd_adapter_t *adapter;
	char *iface_name;
	struct ndp_initiator_req req = {0};
	QDF_STATUS status;
	uint32_t ndp_qos_cfg;
	tHalHandle hal = hdd_ctx->hHal;
	struct nan_datapath_ctx *ndp_ctx;

	ENTER();

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		hdd_err("Interface name string is unavailable");
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
	/* Check for interface in NDI mode */
	adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
	if (!adapter) {
		hdd_err("NAN data interface %s not available", iface_name);
		return -EINVAL;
	}

	/* NAN data path coexists only with STA interface */
	if (false == hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err("Unsupported concurrency for NAN datapath");
		return -EPERM;
	}

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);

	if (ndp_ctx->state == NAN_DATA_NDI_DELETED_STATE ||
	    ndp_ctx->state == NAN_DATA_NDI_DELETING_STATE ||
	    ndp_ctx->state == NAN_DATA_NDI_CREATING_STATE) {
		hdd_err("Data request not allowed in NDI current state: %d",
			ndp_ctx->state);
		return -EINVAL;
	}

	req.vdev_id = adapter->sessionId;

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err("Transaction ID is unavailable");
		return -EINVAL;
	}
	req.transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL]) {
		hdd_err("NDP channel is unavailable");
		return -EINVAL;
	}
	req.channel =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID]) {
		hdd_err("NDP service instance ID is unavailable");
		return -EINVAL;
	}
	req.service_instance_id =
		nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID]);

	qdf_mem_copy(req.self_ndi_mac_addr.bytes,
		     adapter->macAddressCurrent.bytes, QDF_MAC_ADDR_SIZE);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR]) {
		hdd_err("NDI peer discovery mac addr is unavailable");
		return -EINVAL;
	}
	qdf_mem_copy(req.peer_discovery_mac_addr.bytes,
		nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR]),
		QDF_MAC_ADDR_SIZE);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]) {
		req.ndp_info.ndp_app_info_len =
			nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]);
		req.ndp_info.ndp_app_info =
			nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]) {
		/* at present ndp config stores 4 bytes QOS info only */
		req.ndp_config.ndp_cfg_len = 4;
		req.ndp_config.ndp_cfg = (uint8_t *)&ndp_qos_cfg;
		ndp_qos_cfg =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]);
	}

	hdd_info("vdev_id: %d, transaction_id: %d, channel: %d, service_instance_id: %d, ndp_app_info_len: %d, peer_discovery_mac_addr: %pM",
		req.vdev_id, req.transaction_id, req.channel,
		req.service_instance_id, req.ndp_info.ndp_app_info_len,
		req.peer_discovery_mac_addr.bytes);
	status = sme_ndp_initiator_req_handler(hal, &req);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("sme_ndp_initiator_req_handler failed, status: %d",
		       status);
		return -ECOMM;
	}
	EXIT();
	return 0;
}

/**
 * hdd_ndp_responder_req_handler() - NDP responder request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return: 0 on success or error code on failure
 */
static int hdd_ndp_responder_req_handler(hdd_context_t *hdd_ctx,
						struct nlattr **tb)
{
	hdd_adapter_t *adapter;
	char *iface_name;
	struct ndp_responder_req req = {0};
	QDF_STATUS status;
	int ret = 0;
	struct nan_datapath_ctx *ndp_ctx;
	uint32_t ndp_qos_cfg;

	ENTER();

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		hdd_err("Interface name string is unavailable");
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
	/* Check if there is already an existing NAN interface */
	adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
	if (!adapter) {
		hdd_err("NAN data interface %s not available", iface_name);
		return -EINVAL;
	}

	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err("Interface %s not in NDI mode", iface_name);
		return -EINVAL;
	}

	/* NAN data path coexists only with STA interface */
	if (!hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err("Unsupported concurrency for NAN datapath");
		return -EINVAL;
	}

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);

	if (ndp_ctx->state == NAN_DATA_NDI_DELETED_STATE ||
	    ndp_ctx->state == NAN_DATA_NDI_DELETING_STATE ||
	    ndp_ctx->state == NAN_DATA_NDI_CREATING_STATE) {
		hdd_err("Data request not allowed in current NDI state: %d",
			ndp_ctx->state);
		return -EAGAIN;
	}

	req.vdev_id = adapter->sessionId;

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err("Transaction ID is unavailable");
		return -EINVAL;
	}
	req.transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID]) {
		hdd_err("Instance ID is unavailable");
		return -EINVAL;
	}
	req.ndp_instance_id =
		nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE]) {
		hdd_err("ndp_rsp is unavailable");
		return -EINVAL;
	}
	req.ndp_rsp = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]) {
		req.ndp_info.ndp_app_info_len =
			nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]);
		if (req.ndp_info.ndp_app_info_len) {
			req.ndp_info.ndp_app_info =
				nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]);
		}
	} else {
		hdd_info("NDP app info is unavailable");
	}
	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]) {
		/* at present ndp config stores 4 bytes QOS info only */
		req.ndp_config.ndp_cfg_len = 4;
		ndp_qos_cfg =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]);
		req.ndp_config.ndp_cfg = (uint8_t *)&ndp_qos_cfg;
	} else {
		hdd_info("NDP config data is unavailable");
	}

	hdd_info("vdev_id: %d, transaction_id: %d, ndp_rsp %d, ndp_instance_id: %d, ndp_app_info_len: %d",
		req.vdev_id, req.transaction_id, req.ndp_rsp,
		req.ndp_instance_id, req.ndp_info.ndp_app_info_len);

	status = sme_ndp_responder_req_handler(hdd_ctx->hHal, &req);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("sme_ndp_initiator_req_handler failed, status: %d",
			status);
		ret = -EINVAL;
	}

	EXIT();
	return ret;
}

/**
 * hdd_ndp_end_req_handler() - NDP end request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return: 0 on success or error code on failure
 */
static int hdd_ndp_end_req_handler(hdd_context_t *hdd_ctx, struct nlattr **tb)
{
	struct ndp_end_req req = {0};
	QDF_STATUS status;
	tHalHandle hal = hdd_ctx->hHal;

	ENTER();

	/* NAN data path coexists only with STA interface */
	if (!hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err("Unsupported concurrency for NAN datapath");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err("Transaction ID is unavailable");
		return -EINVAL;
	}
	req.transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY]) {
		hdd_err("NDP instance ID array is unavailable");
		return -EINVAL;
	}

	req.num_ndp_instances =
		nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY]) /
			sizeof(uint32_t);
	if (0 >= req.num_ndp_instances) {
		hdd_err("Num NDP instances is 0");
		return -EINVAL;
	}
	req.ndp_ids = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY]);

	hdd_info("sending ndp_end_req to SME, transaction_id: %d",
		req.transaction_id);

	status = sme_ndp_end_req_handler(hal, &req);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("sme_ndp_end_req_handler failed, status: %d",
		       status);
		return -ECOMM;
	}
	EXIT();
	return 0;
}

/**
 * hdd_ndp_iface_create_rsp_handler() - NDP iface create response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * The function is expected to send a response back to the user space
 * even if the creation of BSS has failed
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 * QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE
 *
 * Return: none
 */
static void hdd_ndp_iface_create_rsp_handler(hdd_adapter_t *adapter,
							void *rsp_params)
{
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndi_create_rsp *ndi_rsp = (struct ndi_create_rsp *)rsp_params;
	uint32_t data_len = (3 * sizeof(uint32_t)) + sizeof(uint16_t) +
				NLMSG_HDRLEN + (4 * NLA_HDRLEN);
	struct nan_datapath_ctx *ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	bool create_fail = false;
	uint8_t create_transaction_id = 0;
	uint32_t create_status = NDP_RSP_STATUS_ERROR;
	uint32_t create_reason = NDP_NAN_DATA_IFACE_CREATE_FAILED;
	hdd_station_ctx_t *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct qdf_mac_addr bc_mac_addr = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	tCsrRoamInfo roam_info = {0};
	tSirBssDescription tmp_bss_descp = {0};

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		/* No way the driver can send response back to user space */
		return;

	if (ndi_rsp) {
		create_status = ndi_rsp->status;
		create_reason = ndi_rsp->reason;
	} else {
		hdd_err("Invalid ndi create response");
		create_fail = true;
	}

	if (ndp_ctx) {
		create_transaction_id = ndp_ctx->ndp_create_transaction_id;
	} else {
		hdd_err("ndp_ctx is NULL");
		create_fail = true;
	}

	if (!sta_ctx) {
		hdd_err("sta_ctx is NULL");
		create_fail = true;
	}

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
				NULL,
				data_len,
				QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				cds_get_gfp_flags());

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		create_fail = true;
		goto close_ndi;
	}

	/* Sub vendor command */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD put fail");
		goto nla_put_failure;
	}

	/* Transaction id */
	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		create_transaction_id)) {
		hdd_err("VENDOR_ATTR_NDP_TRANSACTION_ID put fail");
		goto nla_put_failure;
	}

	/* Status code */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
		create_status)) {
		hdd_err("VENDOR_ATTR_NDP_DRV_RETURN_TYPE put fail");
		goto nla_put_failure;
	}

	/* Status return value */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			create_reason)) {
		hdd_err("VENDOR_ATTR_NDP_DRV_RETURN_VALUE put fail");
		goto nla_put_failure;
	}

	hdd_info("sub command: %d, value: %d",
		QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE);
	hdd_info("create transaction id: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID, create_transaction_id);
	hdd_info("status code: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE, create_status);
	hdd_info("Return value: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE, create_reason);

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	if (!create_fail && ndi_rsp->status == QDF_STATUS_SUCCESS) {
		hdd_err("NDI interface successfully created");
		ndp_ctx->ndp_create_transaction_id = 0;
		ndp_ctx->state = NAN_DATA_NDI_CREATED_STATE;
		wlan_hdd_netif_queue_control(adapter,
					WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);
	} else {
		hdd_err("NDI interface creation failed with reason %d",
			create_reason);
	}

	/* Something went wrong while starting the BSS */
	if (create_fail)
		goto close_ndi;

	sta_ctx->broadcast_staid = ndi_rsp->sta_id;
	hdd_save_peer(sta_ctx, sta_ctx->broadcast_staid, &bc_mac_addr);
	hdd_roam_register_sta(adapter, &roam_info,
				sta_ctx->broadcast_staid,
				&bc_mac_addr, &tmp_bss_descp);
	hdd_ctx->sta_to_adapter[sta_ctx->broadcast_staid] = adapter;


	EXIT();
	return;

nla_put_failure:
	kfree_skb(vendor_event);
close_ndi:
	hdd_close_ndi(adapter);
	return;
}

/**
 * hdd_ndp_iface_delete_rsp_handler() - NDP iface delete response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Return: none
 */
static void hdd_ndp_iface_delete_rsp_handler(hdd_adapter_t *adapter,
							void *rsp_params)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndi_delete_rsp *ndi_rsp = rsp_params;
	struct nan_datapath_ctx *ndp_ctx;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!ndi_rsp) {
		hdd_err("Invalid ndi delete response");
		return;
	}

	if (ndi_rsp->status == NDP_RSP_STATUS_SUCCESS)
		hdd_err("NDI BSS successfully stopped");
	else
		hdd_err("NDI BSS stop failed with reason %d", ndi_rsp->reason);

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	ndp_ctx->ndi_delete_rsp_reason = ndi_rsp->reason;
	ndp_ctx->ndi_delete_rsp_status = ndi_rsp->status;
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	complete(&adapter->disconnect_comp_var);
	return;
}

/**
 * hdd_ndp_session_end_handler() - NDI session termination handler
 * @adapter: pointer to adapter context
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *     QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE (4 bytes)
 *
 * Return: none
 */
void hdd_ndp_session_end_handler(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct sk_buff *vendor_event;
	struct nan_datapath_ctx *ndp_ctx;
	uint32_t data_len = sizeof(uint32_t) * (3 + sizeof(uint16_t)) +
				(NLA_HDRLEN * 4) + NLMSG_HDRLEN;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	if (!ndp_ctx) {
		hdd_err("ndp context is NULL");
		return;
	}

	/*
	 * The virtual adapters are stopped and closed even during
	 * driver unload or stop, the service layer is not required
	 * to be informed in that case (response is not expected)
	 */
	if (NAN_DATA_NDI_DELETING_STATE != ndp_ctx->state) {
		hdd_err("NDI interface %s deleted", adapter->dev->name);
		return;
	}

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			NULL,
			data_len,
			QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	/* Sub vendor command goes first */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE)) {
		hdd_err("VENDOR_ATTR_NDP_SUBCMD put fail");
		goto failure;
	}

	/* Transaction id */
	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
			ndp_ctx->ndp_delete_transaction_id)) {
		hdd_err("VENDOR_ATTR_NDP_TRANSACTION_ID put fail");
		goto failure;
	}

	/* Status code */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
			ndp_ctx->ndi_delete_rsp_status)) {
		hdd_err("VENDOR_ATTR_NDP_DRV_RETURN_TYPE put fail");
		goto failure;
	}

	/* Status return value */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			ndp_ctx->ndi_delete_rsp_reason)) {
		hdd_err("VENDOR_ATTR_NDP_DRV_RETURN_VALUE put fail");
		goto failure;
	}

	hdd_info("sub command: %d, value: %d", QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE);
	hdd_info("delete transaction id: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		ndp_ctx->ndp_delete_transaction_id);
	hdd_info("status code: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
		ndp_ctx->ndi_delete_rsp_status);
	hdd_info("Return value: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
		ndp_ctx->ndi_delete_rsp_reason);

	ndp_ctx->ndp_delete_transaction_id = 0;
	ndp_ctx->state = NAN_DATA_NDI_DELETED_STATE;

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	EXIT();
	return;

failure:
	kfree_skb(vendor_event);
}


/**
 * hdd_ndp_initiator_rsp_handler() - NDP initiator response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE (4 bytes)
 *
 * Return: none
 */
static void hdd_ndp_initiator_rsp_handler(hdd_adapter_t *adapter,
						void *rsp_params)
{
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndp_initiator_rsp *rsp = rsp_params;
	uint32_t data_len = (4 * sizeof(uint32_t)) + (1 * sizeof(uint16_t)) +
				NLMSG_HDRLEN + (5 * NLA_HDRLEN);

	ENTER();

	if (!rsp) {
		hdd_err("Invalid NDP Initator response");
		return;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE))
		goto ndp_initiator_rsp_nla_failed;

	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
			rsp->transaction_id))
		goto ndp_initiator_rsp_nla_failed;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID,
			rsp->ndp_instance_id))
		goto ndp_initiator_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
			rsp->status))
		goto ndp_initiator_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			rsp->reason))
		goto ndp_initiator_rsp_nla_failed;

	hdd_info("NDP Initiator rsp sent, tid:%d, instance id:%d, status:%d, reason: %d",
	       rsp->transaction_id, rsp->ndp_instance_id, rsp->status,
	       rsp->reason);
	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	EXIT();
	return;
ndp_initiator_rsp_nla_failed:
	hdd_err("nla_put api failed");
	kfree_skb(vendor_event);
	EXIT();
}

/**
 * hdd_ndp_new_peer_ind_handler() - NDP new peer indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void hdd_ndp_new_peer_ind_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	struct sme_ndp_peer_ind *new_peer_ind = ind_params;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tSirBssDescription tmp_bss_descp = {0};
	tCsrRoamInfo roam_info = {0};
	struct nan_datapath_ctx *ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	hdd_station_ctx_t *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	ENTER();

	if (NULL == ind_params) {
		hdd_err("Invalid new NDP peer params");
		return;
	}
	hdd_info("session_id: %d, peer_mac: %pM, sta_id: %d",
		new_peer_ind->session_id, new_peer_ind->peer_mac_addr.bytes,
		new_peer_ind->sta_id);

	/* save peer in ndp ctx */
	if (false == hdd_save_peer(sta_ctx, new_peer_ind->sta_id,
				   &new_peer_ind->peer_mac_addr)) {
		hdd_err("Ndp peer table full. cannot save new peer");
		return;
	}

	/* this function is called for each new peer */
	ndp_ctx->active_ndp_peers++;
	hdd_info("vdev_id: %d, num_peers: %d", adapter->sessionId,
		 ndp_ctx->active_ndp_peers);

	hdd_roam_register_sta(adapter, &roam_info, new_peer_ind->sta_id,
			    &new_peer_ind->peer_mac_addr, &tmp_bss_descp);
	hdd_ctx->sta_to_adapter[new_peer_ind->sta_id] = adapter;
	/* perform following steps for first new peer ind */
	if (ndp_ctx->active_ndp_peers == 1) {
		hdd_info("Set ctx connection state to connected");
		sta_ctx->conn_info.connState = eConnectionState_NdiConnected;
		hdd_wmm_connect(adapter, &roam_info, eCSR_BSS_TYPE_NDI);
		wlan_hdd_netif_queue_control(adapter,
				WLAN_WAKE_ALL_NETIF_QUEUE, WLAN_CONTROL_PATH);
	}
	EXIT();
}

/**
 * hdd_ndp_peer_departed_ind_handler() - Handle NDP peer departed indication
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void hdd_ndp_peer_departed_ind_handler(hdd_adapter_t *adapter,
							void *ind_params)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct sme_ndp_peer_ind *peer_ind = ind_params;
	struct nan_datapath_ctx *ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	hdd_station_ctx_t *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_roam_deregister_sta(adapter, peer_ind->sta_id);
	hdd_delete_peer(sta_ctx, peer_ind->sta_id);
	hdd_ctx->sta_to_adapter[peer_ind->sta_id] = 0;

	if (--ndp_ctx->active_ndp_peers == 0) {
		hdd_info("No more ndp peers.");
		sta_ctx->conn_info.connState = eConnectionState_NdiDisconnected;
		hdd_conn_set_connection_state(adapter,
			eConnectionState_NdiDisconnected);
		hdd_info("Stop netif tx queues.");
		wlan_hdd_netif_queue_control(adapter, WLAN_STOP_ALL_NETIF_QUEUE,
					     WLAN_CONTROL_PATH);
	}
}

/**
 * hdd_ndp_confirm_ind_handler() - NDP confirm indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_CONFIRM_IND (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR (6 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR (IFNAMSIZ)
 * QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO (ndp_app_info_len size)
 * QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_RETURN_VALUE (4 bytes)
 *
 * Return: none
 */
static void hdd_ndp_confirm_ind_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	int idx;
	uint32_t ndp_qos_config = 0;
	struct ndp_confirm_event *ndp_confirm = ind_params;
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct nan_datapath_ctx *ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	hdd_station_ctx_t *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	uint32_t data_len;

	ENTER();
	if (!ndp_confirm) {
		hdd_err("Invalid NDP Initator response");
		return;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	/* ndp_confirm is called each time user generated ndp req succeeds */
	idx = hdd_get_peer_idx(sta_ctx, &ndp_confirm->peer_ndi_mac_addr);
	if (idx == INVALID_PEER_IDX)
		hdd_err("can't find addr: %pM in vdev_id: %d, peer table.",
			&ndp_confirm->peer_ndi_mac_addr, adapter->sessionId);
	else if (ndp_confirm->rsp_code == NDP_RESPONSE_ACCEPT)
		ndp_ctx->active_ndp_sessions[idx]++;

	data_len = (4 * sizeof(uint32_t)) + QDF_MAC_ADDR_SIZE + IFNAMSIZ +
			+ NLMSG_HDRLEN + (7 * NLA_HDRLEN) +
			ndp_confirm->ndp_info.ndp_app_info_len;

	if (ndp_confirm->ndp_info.ndp_app_info_len)
		data_len += NLA_HDRLEN + ndp_confirm->ndp_info.ndp_app_info_len;

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_CONFIRM_IND))
		goto ndp_confirm_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID,
			ndp_confirm->ndp_instance_id))
		goto ndp_confirm_nla_failed;

	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR,
		QDF_MAC_ADDR_SIZE, ndp_confirm->peer_ndi_mac_addr.bytes))
		goto ndp_confirm_nla_failed;

	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR,
		    IFNAMSIZ, adapter->dev->name))
		goto ndp_confirm_nla_failed;

	if (ndp_confirm->ndp_info.ndp_app_info_len && nla_put(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO,
			ndp_confirm->ndp_info.ndp_app_info_len,
			ndp_confirm->ndp_info.ndp_app_info))
		goto ndp_confirm_nla_failed;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE,
			ndp_confirm->rsp_code))
		goto ndp_confirm_nla_failed;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			ndp_confirm->reason_code))
		goto ndp_confirm_nla_failed;

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	hdd_info("NDP confim sent, ndp instance id: %d, peer addr: %pM, ndp_cfg: %d, rsp_code: %d, reason_code: %d",
		ndp_confirm->ndp_instance_id,
		ndp_confirm->peer_ndi_mac_addr.bytes,
		ndp_qos_config, ndp_confirm->rsp_code,
		ndp_confirm->reason_code);

	hdd_info("NDP confim, ndp app info dump");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   ndp_confirm->ndp_info.ndp_app_info,
			   ndp_confirm->ndp_info.ndp_app_info_len);
	EXIT();
	return;
ndp_confirm_nla_failed:
	hdd_err("nla_put api failed");
	kfree_skb(vendor_event);
	EXIT();
}

/**
 * hdd_ndp_indication_handler() - NDP indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR (IFNAMSIZ)
 * QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR (6 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR (6 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO (ndp_app_info_len size)
 * QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS (4 bytes)
 *
 * Return: none
 */
static void hdd_ndp_indication_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndp_indication_event *event = ind_params;
	uint32_t ndp_qos_config;
	struct nan_datapath_ctx *ndp_ctx;
	uint16_t data_len;

	ENTER();
	if (!ind_params) {
		hdd_err("Invalid NDP Indication");
		return;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	/* Handle only if adapter is in NDI mode */
	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err("Adapter is not in NDI mode");
		return;
	}

	hdd_info("NDP Indication, policy: %d", event->policy);

	/* Policy check */
	if (!WLAN_HDD_IS_NDP_ENABLED(hdd_ctx)) {
		hdd_err("NAN datapath is not suported");
		return;
	}

	/* NAN data path coexists only with STA interface */
	if (!hdd_is_ndp_allowed(hdd_ctx)) {
		hdd_err("Unsupported concurrency for NAN datapath");
		return;
	}

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);

	/* check if we are in middle of deleting/creating the interface */
	if (ndp_ctx->state == NAN_DATA_NDI_DELETED_STATE ||
	    ndp_ctx->state == NAN_DATA_NDI_DELETING_STATE ||
	    ndp_ctx->state == NAN_DATA_NDI_CREATING_STATE) {
		hdd_err("Data request not allowed in current NDI state: %d",
			ndp_ctx->state);
		return;
	}

	data_len = (4 * sizeof(uint32_t)) + (2 * QDF_MAC_ADDR_SIZE) + IFNAMSIZ +
		event->ndp_info.ndp_app_info_len + (8 * NLA_HDRLEN) +
		NLMSG_HDRLEN;

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
					NULL, data_len,
					QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
					GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
	   QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND))
		goto ndp_indication_nla_failed;

	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR,
	   IFNAMSIZ, adapter->dev->name))
		goto ndp_indication_nla_failed;

	if (nla_put_u32(vendor_event,
	   QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID,
	   event->service_instance_id))
		goto ndp_indication_nla_failed;

	if (nla_put(vendor_event,
	   QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR,
	   QDF_MAC_ADDR_SIZE, event->peer_mac_addr.bytes))
		goto ndp_indication_nla_failed;

	if (nla_put(vendor_event,
	   QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR ,
	   QDF_MAC_ADDR_SIZE, event->peer_discovery_mac_addr.bytes))
		goto ndp_indication_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID,
	   event->ndp_instance_id))
		goto ndp_indication_nla_failed;

	if (event->ndp_info.ndp_app_info_len)
		if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO,
		   event->ndp_info.ndp_app_info_len,
		   event->ndp_info.ndp_app_info))
			goto ndp_indication_nla_failed;

	if (event->ndp_config.ndp_cfg_len) {
		ndp_qos_config = *((uint32_t *)event->ndp_config.ndp_cfg);
		/* at present ndp config stores 4 bytes QOS info only */
		if (nla_put_u32(vendor_event,
		   QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS,
		   ndp_qos_config))
			goto ndp_indication_nla_failed;
	}

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	EXIT();
	return;
ndp_indication_nla_failed:
	hdd_err("nla_put api failed");
	kfree_skb(vendor_event);
	EXIT();
}

/**
 * hdd_ndp_responder_rsp_handler() - NDP responder response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE (4 bytes)
 *
 * Return: none
 */
static void hdd_ndp_responder_rsp_handler(hdd_adapter_t *adapter,
							void *rsp_params)
{
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndp_responder_rsp_event *rsp = rsp_params;
	uint16_t data_len;

	ENTER();
	if (!rsp) {
		hdd_err("Invalid NDP Responder response");
		return;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_info("NDP Responder,vdev id %d transaction_id %d status code: %d reason %d",
		rsp->vdev_id, rsp->transaction_id,
		rsp->status, rsp->reason);

	data_len = 3 * sizeof(uint32_t) + sizeof(uint16_t) +
		4 * NLA_HDRLEN + NLMSG_HDRLEN;
	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
					NULL, data_len,
					QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
					GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
	   QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE))
		goto ndp_responder_rsp_nla_failed;

	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
	   rsp->transaction_id))
		goto ndp_responder_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
	   rsp->status))
		goto ndp_responder_rsp_nla_failed;

	if (nla_put_u32(vendor_event,
	   QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE,
	   rsp->reason))
		goto ndp_responder_rsp_nla_failed;

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	EXIT();
	return;
ndp_responder_rsp_nla_failed:
	hdd_err("nla_put api failed");
	kfree_skb(vendor_event);
	EXIT();
}

/**
 * hdd_ndp_end_rsp_handler() - NDP end response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE(4 bytest)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 *
 * Return: none
 */
static void hdd_ndp_end_rsp_handler(hdd_adapter_t *adapter, void *rsp_params)
{
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndp_end_rsp_event *rsp = rsp_params;
	uint32_t data_len;

	ENTER();

	if (!rsp) {
		hdd_err("Invalid ndp end response");
		return;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	data_len = NLMSG_HDRLEN + (4 * NLA_HDRLEN) + (3 * sizeof(uint32_t)) +
		   sizeof(uint16_t);

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE))
		goto ndp_end_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
			rsp->status))
		goto ndp_end_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			rsp->reason))
		goto ndp_end_rsp_nla_failed;

	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
			rsp->transaction_id))
		goto ndp_end_rsp_nla_failed;

	hdd_info("NDP End rsp sent, transaction id: %d, status: %d, reason: %d",
	       rsp->transaction_id, rsp->status, rsp->reason);
	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	EXIT();
	return;

ndp_end_rsp_nla_failed:
	hdd_err("nla_put api failed");
	kfree_skb(vendor_event);
	EXIT();
}

/**
 * hdd_ndp_end_ind_handler() - NDP end indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_END_IND (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY (4 * num of NDP Instances)
 *
 * Return: none
 */
static void hdd_ndp_end_ind_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	struct sk_buff *vendor_event;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct ndp_end_indication_event *end_ind = ind_params;
	uint32_t data_len, i;
	struct nan_datapath_ctx *ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	hdd_station_ctx_t *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	uint32_t *ndp_instance_array;
	hdd_adapter_t *ndi_adapter;

	ENTER();

	if (!end_ind) {
		hdd_err("Invalid ndp end indication");
		return;
	}

	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	ndp_instance_array = qdf_mem_malloc(end_ind->num_ndp_ids *
		sizeof(*ndp_instance_array));
	if (!ndp_instance_array) {
		hdd_err("Failed to allocate ndp_instance_array");
		return;
	}
	for (i = 0; i < end_ind->num_ndp_ids; i++) {
		int idx;

		ndp_instance_array[i] = end_ind->ndp_map[i].ndp_instance_id;
		ndi_adapter = hdd_get_adapter_by_vdev(hdd_ctx,
					end_ind->ndp_map[i].vdev_id);
		if (ndi_adapter == NULL) {
			hdd_err("Adapter not found for vdev_id: %d",
				end_ind->ndp_map[i].vdev_id);
			continue;
		}
		ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(ndi_adapter);
		if (!ndp_ctx) {
			hdd_err("ndp_ctx is NULL for vdev id: %d",
				end_ind->ndp_map[i].vdev_id);
			continue;
		}
		idx = hdd_get_peer_idx(sta_ctx,
				&end_ind->ndp_map[i].peer_ndi_mac_addr);
		if (idx == INVALID_PEER_IDX) {
			hdd_err("can't find addr: %pM in sta_ctx.",
				&end_ind->ndp_map[i].peer_ndi_mac_addr);
			continue;
		}
		/* save the value of active sessions on each peer */
		ndp_ctx->active_ndp_sessions[idx] =
			end_ind->ndp_map[i].num_active_ndp_sessions;
	}

	data_len = (sizeof(uint32_t)) + NLMSG_HDRLEN + (2 * NLA_HDRLEN) +
			end_ind->num_ndp_ids * sizeof(*ndp_instance_array);

	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_KERNEL);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_END_IND))
		goto ndp_end_ind_nla_failed;

	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY,
			end_ind->num_ndp_ids * sizeof(*ndp_instance_array),
			ndp_instance_array))
		goto ndp_end_ind_nla_failed;

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
	qdf_mem_free(ndp_instance_array);
	EXIT();
	return;

ndp_end_ind_nla_failed:
	hdd_err("nla_put api failed");
	kfree_skb(vendor_event);
	qdf_mem_free(ndp_instance_array);
	EXIT();
}

/**
 * hdd_ndp_event_handler() - ndp response and indication handler
 * @adapter: adapter context
 * @roam_info: pointer to roam_info structure
 * @roam_id: roam id as indicated by SME
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * Return: none
 */
void hdd_ndp_event_handler(hdd_adapter_t *adapter,
	tCsrRoamInfo *roam_info, uint32_t roam_id, eRoamCmdStatus roam_status,
	eCsrRoamResult roam_result)
{
	if (roam_status == eCSR_ROAM_NDP_STATUS_UPDATE) {
		switch (roam_result) {
		case eCSR_ROAM_RESULT_NDI_CREATE_RSP:
			hdd_ndp_iface_create_rsp_handler(adapter,
				&roam_info->ndp.ndi_create_params);
			break;
		case eCSR_ROAM_RESULT_NDI_DELETE_RSP:
			hdd_ndp_iface_delete_rsp_handler(adapter,
				&roam_info->ndp.ndi_delete_params);
			break;
		case eCSR_ROAM_RESULT_NDP_INITIATOR_RSP:
			hdd_ndp_initiator_rsp_handler(adapter,
				&roam_info->ndp.ndp_init_rsp_params);
			break;
		case eCSR_ROAM_RESULT_NDP_NEW_PEER_IND:
			hdd_ndp_new_peer_ind_handler(adapter,
				&roam_info->ndp.ndp_peer_ind_params);
			break;
		case eCSR_ROAM_RESULT_NDP_CONFIRM_IND:
			hdd_ndp_confirm_ind_handler(adapter,
				&roam_info->ndp.ndp_confirm_params);
			break;
		case eCSR_ROAM_RESULT_NDP_INDICATION:
			hdd_ndp_indication_handler(adapter,
				&roam_info->ndp.ndp_indication_params);
			break;
		case eCSR_ROAM_RESULT_NDP_RESPONDER_RSP:
			hdd_ndp_responder_rsp_handler(adapter,
				&roam_info->ndp.ndp_responder_rsp_params);
			break;
		case eCSR_ROAM_RESULT_NDP_END_RSP:
			hdd_ndp_end_rsp_handler(adapter,
				roam_info->ndp.ndp_end_rsp_params);
			break;
		case eCSR_ROAM_RESULT_NDP_PEER_DEPARTED_IND:
			hdd_ndp_peer_departed_ind_handler(adapter,
				&roam_info->ndp.ndp_peer_ind_params);
			break;
		case eCSR_ROAM_RESULT_NDP_END_IND:
			hdd_ndp_end_ind_handler(adapter,
				roam_info->ndp.ndp_end_ind_params);
			break;
		default:
			hdd_err("Unknown NDP response event from SME %d",
				roam_result);
			break;
		}
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
	uint32_t ndp_cmd_type;
	uint16_t transaction_id;
	int ret_val;
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1];
	char *iface_name;

	ENTER();

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
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX,
			data, data_len,
			qca_wlan_vendor_ndp_policy)) {
		hdd_err("Invalid NDP vendor command attributes");
		return -EINVAL;
	}

	/* Parse and fetch NDP Command Type*/
	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]) {
		hdd_err("NAN datapath cmd type failed");
		return -EINVAL;
	}
	ndp_cmd_type = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err("attr transaction id failed");
		return -EINVAL;
	}
	transaction_id = nla_get_u16(
			tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
		hdd_err("Transaction Id: %d NDP Cmd: %d iface_name: %s",
			transaction_id, ndp_cmd_type, iface_name);
	} else {
		hdd_err("Transaction Id: %d NDP Cmd: %d iface_name: unspecified",
			transaction_id, ndp_cmd_type);
	}

	switch (ndp_cmd_type) {
	case QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE:
		ret_val  = hdd_ndi_create_req_handler(hdd_ctx, tb);
		break;
	case QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE:
		ret_val = hdd_ndi_delete_req_handler(hdd_ctx, tb);
		break;
	case QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_REQUEST:
		ret_val = hdd_ndp_initiator_req_handler(hdd_ctx, tb);
		break;
	case QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_REQUEST:
		ret_val = hdd_ndp_responder_req_handler(hdd_ctx, tb);
		break;
	case QCA_WLAN_VENDOR_ATTR_NDP_END_REQUEST:
		ret_val = hdd_ndp_end_req_handler(hdd_ctx, tb);
		break;
	default:
		hdd_err("Unrecognized NDP vendor cmd %d", ndp_cmd_type);
		ret_val = -EINVAL;
		break;
	}

	return ret_val;
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

/**
 * hdd_init_nan_data_mode() - initialize nan data mode
 * @adapter: adapter context
 *
 * Returns: 0 on success negative error code on error
 */
int hdd_init_nan_data_mode(struct hdd_adapter_s *adapter)
{
	struct net_device *wlan_dev = adapter->dev;
	struct nan_datapath_ctx *ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;
	uint32_t type, sub_type;
	int32_t ret_val = 0;
	unsigned long rc;
	uint32_t timeout = WLAN_WAIT_TIME_SESSIONOPENCLOSE;

	INIT_COMPLETION(adapter->session_open_comp_var);
	sme_set_curr_device_mode(hdd_ctx->hHal, adapter->device_mode);
	status = cds_get_vdev_types(adapter->device_mode, &type, &sub_type);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("failed to get vdev type");
		goto error_sme_open;
	}

	/* open sme session for future use */
	status = sme_open_session(hdd_ctx->hHal, hdd_sme_roam_callback,
			adapter, (uint8_t *)&adapter->macAddressCurrent,
			&adapter->sessionId, type, sub_type);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("sme_open_session() failed with status code %d",
			status);
		ret_val = -EAGAIN;
		goto error_sme_open;
	}

	/* Block on a completion variable. Can't wait forever though */
	rc = wait_for_completion_timeout(
			&adapter->session_open_comp_var,
			msecs_to_jiffies(timeout));
	if (!rc) {
		hdd_err("Failed to open session, timeout code: %ld", rc);
		ret_val = -ETIMEDOUT;
		goto error_sme_open;
	}

	/* Register wireless extensions */
	ret_val = hdd_register_wext(wlan_dev);
	if (0 > ret_val) {
		hdd_err("Wext registration failed with status code %d",
			ret_val);
		ret_val = -EAGAIN;
		goto error_register_wext;
	}

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

	ret_val = wma_cli_set_command((int)adapter->sessionId,
			(int)WMI_PDEV_PARAM_BURST_ENABLE,
			(int)hdd_ctx->config->enableSifsBurst,
			PDEV_CMD);
	if (0 != ret_val) {
		hdd_err("WMI_PDEV_PARAM_BURST_ENABLE set failed %d", ret_val);
	}

	ndp_ctx->state = NAN_DATA_NDI_CREATING_STATE;
	return ret_val;

error_wmm_init:
	clear_bit(INIT_TX_RX_SUCCESS, &adapter->event_flags);
	hdd_deinit_tx_rx(adapter);

error_init_txrx:
	hdd_unregister_wext(wlan_dev);

error_register_wext:
	if (test_bit(SME_SESSION_OPENED, &adapter->event_flags)) {
		INIT_COMPLETION(adapter->session_close_comp_var);
		if (QDF_STATUS_SUCCESS ==
				sme_close_session(hdd_ctx->hHal,
					adapter->sessionId,
					hdd_sme_close_session_callback,
					adapter)) {
			rc = wait_for_completion_timeout(
					&adapter->session_close_comp_var,
					msecs_to_jiffies(timeout));
			if (rc <= 0) {
				hdd_err("Session close failed status %ld", rc);
				ret_val = -ETIMEDOUT;
			}
		}
	}

error_sme_open:
	return ret_val;
}
