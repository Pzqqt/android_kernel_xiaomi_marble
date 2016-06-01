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

/* NLA policy */
static const struct nla_policy
qca_wlan_vendor_ndp_policy[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR] = { .type = NLA_STRING,
					.len = IFNAMSIZ },
	[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_SPEC_CHANNEL] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR] = {
						.type = NLA_BINARY,
						.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS] = { .type = NLA_BINARY,
					.len = NDP_QOS_INFO_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO_LEN] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO] = { .type = NLA_BINARY,
					.len = NDP_APP_INFO_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_RESPONSE_CODE] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_STATUS_CODE] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR] = { .type = NLA_BINARY,
					.len = QDF_MAC_ADDR_SIZE },
};

/**
 * hdd_ndp_print_ini_config()- Print nan datapath specific INI configuration
 * @hdd_ctx: handle to hdd context
 *
 * Return: None
 */
void hdd_ndp_print_ini_config(hdd_context_t *hdd_ctx)
{
	hddLog(LOG2, "Name = [%s] Value = [%u]",
		CFG_ENABLE_NAN_DATAPATH_NAME,
		hdd_ctx->config->enable_nan_datapath);
	hddLog(LOG2, "Name = [%s] Value = [%u]",
		CFG_ENABLE_NAN_NDI_CHANNEL_NAME,
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
	hdd_info(FL("enable_nan_datapath: %d"), hdd_ctx->nan_datapath_enabled);
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
		hdd_err(FL("Interface is not in NDI mode"));
		return -EINVAL;
	}
	netif_tx_disable(adapter->dev);
	netif_carrier_off(adapter->dev);

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
				hdd_err(FL("session close timeout"));
		}
	}

	/* We are good to close the adapter */
	hdd_close_adapter(hdd_ctx, adapter, true);

	EXIT();
	return 0;
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
		hdd_err(FL("No valid roam profile"));
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
	hdd_select_cbmode(adapter, operating_channel);

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
		hdd_err(
			FL("NDI sme_RoamConnect session %d failed with status %d -> NotConnected"),
			  adapter->sessionId, ret);
		/* change back to NotConnected */
		hdd_conn_set_connection_state(adapter,
			eConnectionState_NotConnected);
	} else {
		hddLog(LOG2, FL("sme_RoamConnect issued successfully for NDI"));
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
		hdd_err(FL("Interface name string is unavailable"));
		return -EINVAL;
	}
	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err(FL("transaction id is unavailable"));
		return -EINVAL;
	}
	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	/* Check for an existing interface of NDI type */
	adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
	if (adapter) {
		hdd_err(FL("Cannot support more than one NDI"));
		return -EEXIST;
	}

	adapter = hdd_open_adapter(hdd_ctx, QDF_NDI_MODE, iface_name,
			wlan_hdd_get_intf_addr(hdd_ctx), NET_NAME_UNKNOWN,
			true);
	if (!adapter) {
		hdd_err(FL("hdd_open_adapter failed"));
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
		hdd_err(FL("NDI start bss failed"));
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

	ENTER();

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		hdd_err(FL("Interface name string is unavailable"));
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err(FL("Transaction id is unavailable"));
		return -EINVAL;
	}

	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	/* Check if there is already an existing inteface with the same name */
	adapter = hdd_get_adapter(hdd_ctx, QDF_NDI_MODE);
	if (!adapter) {
		hdd_err(FL("NAN data interface %s is not available"),
			iface_name);
		return -EINVAL;
	}

	/* check if adapter is in NDI mode */
	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err(FL("Interface %s is not in NDI mode"),
			iface_name);
		return -EINVAL;
	}

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	if (!ndp_ctx) {
		hdd_err(FL("ndp_ctx is NULL"));
		return -EINVAL;
	}

	/* check if there are active NDP sessions on the adapter */
	if (ndp_ctx->active_ndp_sessions > 0) {
		hdd_err(FL("NDP sessions active %d, cannot delete NDI"),
			ndp_ctx->active_ndp_sessions);
		return -EINVAL;
	}

	ndp_ctx->ndp_delete_transaction_id = transaction_id;
	ndp_ctx->state = NAN_DATA_NDI_DELETING_STATE;

	/* Delete the interface */
	ret = __wlan_hdd_del_virtual_intf(hdd_ctx->wiphy, &adapter->wdev);
	if (ret < 0)
		hdd_err(FL("NDI delete request failed"));
	else
		hdd_err(FL("NDI delete request successfully issued"));

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
	return 0;
}

/**
 * hdd_ndp_end_req_handler() - NDP end request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return: 0 on success or error code on failure
 */
static int hdd_ndp_end_req_handler(hdd_context_t *hdd_ctx,
						struct nlattr **tb)
{
	return 0;
}

/**
 * hdd_ndp_schedule_req_handler() - NDP schedule request handler
 * @hdd_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * Return: 0 on success or error code on failure
 */
static int hdd_ndp_schedule_req_handler(hdd_context_t *hdd_ctx,
						struct nlattr **tb)
{
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
	uint8_t create_status = 0;

	ENTER();

	if (wlan_hdd_validate_context(hdd_ctx))
		/* No way the driver can send response back to user space */
		return;

	if (ndi_rsp) {
		create_status = ndi_rsp->status;
	} else {
		hdd_err(FL("Invalid ndi create response"));
		create_fail = true;
	}

	if (ndp_ctx) {
		create_transaction_id = ndp_ctx->ndp_create_transaction_id;
	} else {
		hdd_err(FL("ndp_ctx is NULL"));
		create_fail = true;
	}

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
				NULL,
				data_len,
				QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				cds_get_gfp_flags());

	if (!vendor_event) {
		hdd_err(FL("cfg80211_vendor_event_alloc failed"));
		create_fail = true;
		goto close_ndi;
	}

	/* Sub vendor command */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE)) {
		hdd_err(FL("QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD put fail"));
		goto nla_put_failure;
	}

	/* Transaction id */
	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		create_transaction_id)) {
		hdd_err(FL("VENDOR_ATTR_NDP_TRANSACTION_ID put fail"));
		goto nla_put_failure;
	}

	/* Status code */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
		create_status)) {
		hdd_err(FL("VENDOR_ATTR_NDP_DRV_RETURN_TYPE put fail"));
		goto nla_put_failure;
	}

	/* Status return value */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE, 0xA5)) {
		hdd_err(FL("VENDOR_ATTR_NDP_DRV_RETURN_VALUE put fail"));
		goto nla_put_failure;
	}

	hddLog(LOG2, FL("sub command: %d, value: %d"),
		QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE);
	hddLog(LOG2, FL("create transaction id: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		create_transaction_id);
	hddLog(LOG2, FL("status code: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE, create_status);
	hddLog(LOG2, FL("Return value: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE, 0xA5);

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	if (!create_fail && ndi_rsp->status == QDF_STATUS_SUCCESS) {
		hdd_err(FL("NDI interface successfully created"));
		ndp_ctx->ndp_create_transaction_id = 0;
		ndp_ctx->state = NAN_DATA_NDI_CREATED_STATE;
	} else {
		hdd_err(FL("NDI interface creation failed with reason %d"),
			ndi_rsp->reason);
	}

	/* Something went wrong while starting the BSS */
	if (create_fail)
		goto close_ndi;

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

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!ndi_rsp) {
		hdd_err(FL("Invalid ndi delete response"));
		return;
	}

	if (ndi_rsp->status == QDF_STATUS_SUCCESS)
		hdd_err(FL("NDI BSS successfully stopped"));
	else
		hdd_err(FL("NDI BSS stop failed with reason %d"),
			ndi_rsp->reason);

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

	/* Handle only if adapter is in NDI mode */
	if (QDF_NDI_MODE != adapter->device_mode) {
		hdd_err(FL("Adapter is not in NDI mode"));
		return;
	}

	ndp_ctx = WLAN_HDD_GET_NDP_CTX_PTR(adapter);
	if (!ndp_ctx) {
		hdd_err(FL("ndp context is NULL"));
		return;
	}

	/*
	 * The virtual adapters are stopped and closed even during
	 * driver unload or stop, the service layer is not required
	 * to be informed in that case (response is not expected)
	 */
	if (NAN_DATA_NDI_DELETING_STATE != ndp_ctx->state) {
		hdd_err(FL("NDI interface %s deleted"),
			adapter->dev->name);
		return;
	}

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
			NULL,
			data_len,
			QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		hdd_err(FL("cfg80211_vendor_event_alloc failed"));
		return;
	}

	/* Sub vendor command goes first */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE)) {
		hdd_err(FL("VENDOR_ATTR_NDP_SUBCMD put fail"));
		goto failure;
	}

	/* Transaction id */
	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
			ndp_ctx->ndp_delete_transaction_id)) {
		hdd_err(FL("VENDOR_ATTR_NDP_TRANSACTION_ID put fail"));
		goto failure;
	}

	/* Status code */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE, 0x0)) {
		hdd_err(FL("VENDOR_ATTR_NDP_DRV_RETURN_TYPE put fail"));
		goto failure;
	}

	/* Status return value */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE, 0x0)) {
		hdd_err(FL("VENDOR_ATTR_NDP_DRV_RETURN_VALUE put fail"));
		goto failure;
	}

	hddLog(LOG2, FL("sub command: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE);
	hddLog(LOG2, FL("delete transaction id: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		ndp_ctx->ndp_delete_transaction_id);
	hddLog(LOG2, FL("status code: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_TYPE,
		true);
	hddLog(LOG2, FL("Return value: %d, value: %d"),
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE, 0x5A);

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
 * Return: none
 */
static void hdd_ndp_initiator_rsp_handler(hdd_adapter_t *adapter,
						void *rsp_params)
{
	return;
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
	return;
}

/**
 * hdd_ndp_peer_departed_ind_handler() - NDP peer departed indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void hdd_ndp_peer_departed_ind_handler(
				hdd_adapter_t *adapter, void *ind_params)
{
	return;
}

/**
 * hdd_ndp_confirm_ind_handler() - NDP confirm indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void hdd_ndp_confirm_ind_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	return;
}

/**
 * hdd_ndp_indication_handler() - NDP indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void hdd_ndp_indication_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	return;
}

/**
 * hdd_ndp_responder_rsp_handler() - NDP responder response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Return: none
 */
static void hdd_ndp_responder_rsp_handler(hdd_adapter_t *adapter,
							void *rsp_params)
{
	return;
}

/**
 * hdd_ndp_end_rsp_handler() - NDP end response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Return: none
 */
static void hdd_ndp_end_rsp_handler(hdd_adapter_t *adapter,
						void *rsp_params)
{
	return;
}

/**
 * hdd_ndp_end_ind_handler() - NDP end indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void hdd_ndp_end_ind_handler(hdd_adapter_t *adapter,
						void *ind_params)
{
	return;
}

/**
 * hdd_ndp_schedule_update_rsp_handler() - NDP schedule update response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Return: none
 */
static void hdd_ndp_schedule_update_rsp_handler(
				hdd_adapter_t *adapter, void *rsp_params)
{
	return;
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
		case eCSR_ROAM_RESULT_NDP_CREATE_RSP:
			hdd_ndp_iface_create_rsp_handler(adapter,
				&roam_info->ndp.ndi_create_params);
			break;
		case eCSR_ROAM_RESULT_NDP_DELETE_RSP:
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
		case eCSR_ROAM_RESULT_NDP_SCHED_UPDATE_RSP:
			hdd_ndp_schedule_update_rsp_handler(adapter,
				&roam_info->ndp.ndp_sched_upd_rsp_params);
			break;
		case eCSR_ROAM_RESULT_NDP_RESPONDER_RSP:
			hdd_ndp_responder_rsp_handler(adapter,
				&roam_info->ndp.ndp_responder_rsp_params);
			break;
		case eCSR_ROAM_RESULT_NDP_END_RSP:
			hdd_ndp_end_rsp_handler(adapter,
				&roam_info->ndp.ndp_end_rsp_params);
			break;
		case eCSR_ROAM_RESULT_NDP_PEER_DEPARTED_IND:
			hdd_ndp_peer_departed_ind_handler(adapter,
				&roam_info->ndp.ndp_peer_ind_params);
			break;
		case eCSR_ROAM_RESULT_NDP_END_IND:
			hdd_ndp_end_ind_handler(adapter,
				&roam_info->ndp.ndp_end_ind_params);
			break;
		default:
			hdd_err(FL("Unknown NDP response event from SME %d"),
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
		hdd_err(FL("Command not allowed in FTM mode"));
		return -EPERM;
	}
	if (!WLAN_HDD_IS_NDP_ENABLED(hdd_ctx)) {
		hdd_err(FL("NAN datapath is not enabled"));
		return -EPERM;
	}
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX,
			data, data_len,
			qca_wlan_vendor_ndp_policy)) {
		hdd_err(FL("Invalid NDP vendor command attributes"));
		return -EINVAL;
	}

	/* Parse and fetch NDP Command Type*/
	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]) {
		hdd_err(FL("NAN datapath cmd type failed"));
		return -EINVAL;
	}
	ndp_cmd_type = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		hdd_err(FL("attr transaction id failed"));
		return -EINVAL;
	}
	transaction_id = nla_get_u16(
			tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		hdd_err(FL("Interface name string is unavailable"));
		return -EINVAL;
	}
	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	hddLog(LOG2, FL("Transaction Id: %d NDP Cmd: %d iface_name: %s"),
		transaction_id, ndp_cmd_type, iface_name);

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
	case QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_UPDATE_REQUEST:
		ret_val = hdd_ndp_schedule_req_handler(hdd_ctx, tb);
		break;
	default:
		hdd_err(FL("Unrecognized NDP vendor cmd %d"),
			ndp_cmd_type);
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
	if (QDF_STATUS_SUCCESS == status) {
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
		hdd_err(
			FL("Failed to open session, timeout code: %ld"), rc);
		ret_val = -ETIMEDOUT;
		goto error_sme_open;
	}

	/* Register wireless extensions */
	ret_val = hdd_register_wext(wlan_dev);
	if (0 > ret_val) {
		hdd_err(FL("Wext registration failed with status code %d"),
				ret_val);
		ret_val = -EAGAIN;
		goto error_register_wext;
	}

	status = hdd_init_tx_rx(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err(FL("hdd_init_tx_rx() init failed, status %d"),
				status);
		ret_val = -EAGAIN;
		goto error_init_txrx;
	}

	set_bit(INIT_TX_RX_SUCCESS, &adapter->event_flags);

	status = hdd_wmm_adapter_init(adapter);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err(FL("hdd_wmm_adapter_init() failed, status %d"),
				status);
		ret_val = -EAGAIN;
		goto error_wmm_init;
	}

	set_bit(WMM_INIT_DONE, &adapter->event_flags);

	ret_val = wma_cli_set_command((int)adapter->sessionId,
			(int)WMI_PDEV_PARAM_BURST_ENABLE,
			(int)hdd_ctx->config->enableSifsBurst,
			PDEV_CMD);
	if (0 != ret_val) {
		hdd_err(FL("WMI_PDEV_PARAM_BURST_ENABLE set failed %d"),
				ret_val);
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
				hdd_err(FL("Session close failed status %ld"),
					rc);
				ret_val = -ETIMEDOUT;
			}
		}
	}

error_sme_open:
	return ret_val;
}
