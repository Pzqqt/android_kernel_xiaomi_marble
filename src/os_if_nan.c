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
 * DOC: defines nan component os interface APIs
 */

#include "qdf_str.h"
#include "qdf_trace.h"
#include "qdf_types.h"
#include "os_if_nan.h"
#include "wlan_nan_api.h"
#include "nan_ucfg_api.h"
#include "nan_public_structs.h"
#include "wlan_osif_priv.h"
#include <net/cfg80211.h>
#include "wlan_cfg80211.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_utility.h"

/* NLA policy */
static const struct nla_policy
vendor_attr_policy[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR] = { .type = NLA_NUL_STRING,
					.len = IFNAMSIZ - 1 },
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
	[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PMK] = { .type = NLA_BINARY,
					.len = NDP_PMK_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_SCID] = { .type = NLA_BINARY,
					.len = NDP_SCID_BUF_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE] = { .type =
					NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE] = { .type = NLA_BINARY,
					.len = NAN_PASSPHRASE_MAX_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME] = { .type = NLA_BINARY,
					.len = NAN_MAX_SERVICE_NAME_LEN },
};

static int os_if_nan_process_ndi_create(struct wlan_objmgr_psoc *psoc,
					struct nlattr **tb)
{
	char *iface_name;
	QDF_STATUS status;
	uint16_t transaction_id;
	struct wlan_objmgr_vdev *nan_vdev;
	struct nan_callbacks cb_obj;

	cfg80211_debug("enter");
	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		cfg80211_err("Interface name string is unavailable");
		return -EINVAL;
	}
	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	nan_vdev = wlan_util_get_vdev_by_ifname(psoc, iface_name, WLAN_NAN_ID);
	if (nan_vdev) {
		cfg80211_err("NAN data interface %s is already present",
			     iface_name);
		wlan_objmgr_vdev_release_ref(nan_vdev, WLAN_NAN_ID);
		return -EEXIST;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("transaction id is unavailable");
		return -EINVAL;
	}
	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Couldn't get ballback object");
		return -EINVAL;
	}

	nan_vdev = cb_obj.ndi_open(iface_name);

	if (!nan_vdev) {
		cfg80211_err("ndi_open failed");
		return -EINVAL;
	}

	/*
	 * Create transaction id is required to be saved since the firmware
	 * does not honor the transaction id for create request
	 */
	ucfg_nan_set_ndp_create_transaction_id(nan_vdev, transaction_id);
	ucfg_nan_set_ndi_state(nan_vdev, NAN_DATA_NDI_CREATING_STATE);

	return cb_obj.ndi_start(wlan_vdev_get_id(nan_vdev));
}

static int os_if_nan_process_ndi_delete(struct wlan_objmgr_psoc *psoc,
					struct nlattr **tb)
{
	uint8_t vdev_id;
	char *iface_name;
	QDF_STATUS status;
	uint32_t num_peers;
	uint16_t transaction_id;
	struct nan_callbacks cb_obj;
	struct wlan_objmgr_vdev *nan_vdev = NULL;

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		cfg80211_err("Interface name string is unavailable");
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("Transaction id is unavailable");
		return -EINVAL;
	}

	nan_vdev = wlan_util_get_vdev_by_ifname(psoc, iface_name, WLAN_NAN_ID);
	if (!nan_vdev) {
		cfg80211_err("Nan datapath interface is not present");
		return -EINVAL;
	}

	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);
	vdev_id = wlan_vdev_get_id(nan_vdev);
	num_peers = ucfg_nan_get_active_peers(nan_vdev);
	/*
	 * wlan_util_get_vdev_by_ifname increments ref count
	 * decrement here since vdev returned by that api is not used any more
	 */
	wlan_objmgr_vdev_release_ref(nan_vdev, WLAN_NAN_ID);

	/* check if there are active peers on the adapter */
	if (num_peers)
		cfg80211_err("NDP peers active: %d, active NDPs may not be terminated",
			     num_peers);

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Couldn't get ballback object");
		return -EINVAL;
	}

	return cb_obj.ndi_delete(vdev_id, iface_name, transaction_id);
}

/**
 * os_if_nan_parse_security_params() - parse vendor attributes for security
 * params.
 * @tb: parsed NL attribute list
 * @ncs_sk_type: out parameter to populate ncs_sk_type
 * @pmk: out parameter to populate pmk
 * @passphrase: out parameter to populate passphrase
 * @service_name: out parameter to populate service_name
 *
 * Return:  0 on success or error code on failure
 */
static int os_if_nan_parse_security_params(struct nlattr **tb,
			uint32_t *ncs_sk_type, struct nan_datapath_pmk *pmk,
			struct ndp_passphrase *passphrase,
			struct ndp_service_name *service_name)
{
	if (!ncs_sk_type || !pmk || !passphrase || !service_name) {
		cfg80211_err("out buffers for one ore more parameters is null");
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE]) {
		*ncs_sk_type =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE]);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_PMK]) {
		pmk->pmk_len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_PMK]);
		qdf_mem_copy(pmk->pmk,
			     nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_PMK]),
			     pmk->pmk_len);
		cfg80211_err("pmk len: %d", pmk->pmk_len);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				   pmk->pmk, pmk->pmk_len);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE]) {
		passphrase->passphrase_len =
			nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE]);
		qdf_mem_copy(passphrase->passphrase,
			     nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE]),
			     passphrase->passphrase_len);
		cfg80211_err("passphrase len: %d", passphrase->passphrase_len);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			passphrase->passphrase, passphrase->passphrase_len);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME]) {
		service_name->service_name_len =
			nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME]);
		qdf_mem_copy(service_name->service_name,
			nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME]),
			service_name->service_name_len);
		cfg80211_err("service_name len: %d",
			     service_name->service_name_len);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				service_name->service_name,
				service_name->service_name_len);
	}

	return 0;
}

/**
 * os_if_nan_process_ndp_initiator_req() - NDP initiator request handler
 * @ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * tb will contain following vendor attributes:
 * QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID
 * QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG
 * QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID
 * QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR
 * QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_PMK - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME - optional
 *
 * Return:  0 on success or error code on failure
 */
static int os_if_nan_process_ndp_initiator_req(struct wlan_objmgr_psoc *psoc,
					       struct nlattr **tb)
{
	int ret = 0;
	char *iface_name;
	QDF_STATUS status;
	enum nan_datapath_state state;
	struct wlan_objmgr_vdev *nan_vdev;
	struct nan_datapath_initiator_req req = {0};

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		cfg80211_err("Interface name string is unavailable");
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
	nan_vdev = wlan_util_get_vdev_by_ifname(psoc, iface_name, WLAN_NAN_ID);
	if (!nan_vdev) {
		cfg80211_err("NAN data interface %s not available", iface_name);
		return -EINVAL;
	}

	if (nan_vdev->vdev_mlme.vdev_opmode != QDF_NDI_MODE) {
		cfg80211_err("Interface found is not NDI");
		return -EINVAL;
	}

	state = ucfg_nan_get_ndi_state(nan_vdev);
	if (state == NAN_DATA_NDI_DELETED_STATE ||
	    state == NAN_DATA_NDI_DELETING_STATE ||
	    state == NAN_DATA_NDI_CREATING_STATE) {
		cfg80211_err("Data request not allowed in NDI current state: %d",
			  state);
		ret = -EINVAL;
		goto initiator_req_failed;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("Transaction ID is unavailable");
		ret = -EINVAL;
		goto initiator_req_failed;
	}
	req.transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL]) {
		req.channel = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL]);

		if (tb[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG]) {
			req.channel_cfg = nla_get_u32(
				tb[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG]);
		} else {
			cfg80211_err("Channel config is unavailable");
			ret = -EINVAL;
			goto initiator_req_failed;
		}
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID]) {
		cfg80211_err("NDP service instance ID is unavailable");
		ret = -EINVAL;
		goto initiator_req_failed;
	}
	req.service_instance_id =
		nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID]);

	qdf_mem_copy(req.self_ndi_mac_addr.bytes,
		     wlan_vdev_mlme_get_macaddr(nan_vdev), QDF_MAC_ADDR_SIZE);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR]) {
		cfg80211_err("NDI peer discovery mac addr is unavailable");
		ret = -EINVAL;
		goto initiator_req_failed;
	}
	qdf_mem_copy(req.peer_discovery_mac_addr.bytes,
		nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR]),
		QDF_MAC_ADDR_SIZE);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]) {
		req.ndp_info.ndp_app_info_len =
			nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]);
		qdf_mem_copy(req.ndp_info.ndp_app_info,
			     nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]),
			     req.ndp_info.ndp_app_info_len);
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]) {
		/* at present ndp config stores 4 bytes QOS info only */
		req.ndp_config.ndp_cfg_len = 4;
		*((uint32_t *)req.ndp_config.ndp_cfg) =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]);
	}

	if (os_if_nan_parse_security_params(tb, &req.ncs_sk_type, &req.pmk,
			&req.passphrase, &req.service_name)) {
		cfg80211_err("inconsistent security params in request.");
		ret = -EINVAL;
		goto initiator_req_failed;
	}

	cfg80211_debug("vdev_id: %d, transaction_id: %d, channel: %d, service_instance_id: %d, ndp_app_info_len: %d, csid: %d, peer_discovery_mac_addr: %pM",
		wlan_vdev_get_id(nan_vdev), req.transaction_id, req.channel,
		req.service_instance_id, req.ndp_info.ndp_app_info_len,
		req.ncs_sk_type, req.peer_discovery_mac_addr.bytes);

	status = ucfg_nan_req_processor(nan_vdev, &req, NDP_INITIATOR_REQ);
	ret = qdf_status_to_os_return(status);
initiator_req_failed:
	wlan_objmgr_vdev_release_ref(nan_vdev, WLAN_NAN_ID);

	return ret;
}

/**
 * os_if_nan_process_ndp_responder_req() - NDP responder request handler
 * @nan_ctx: hdd context
 * @tb: parsed NL attribute list
 *
 * tb includes following vendor attributes:
 * QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID
 * QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE
 * QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_PMK - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE - optional
 * QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME - optional
 *
 * Return: 0 on success or error code on failure
 */
static int os_if_nan_process_ndp_responder_req(struct wlan_objmgr_psoc *psoc,
					       struct nlattr **tb)
{
	int ret = 0;
	char *iface_name;
	QDF_STATUS status;
	enum nan_datapath_state state;
	struct wlan_objmgr_vdev *nan_vdev;
	struct nan_datapath_responder_req req = {0};

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		cfg80211_err("Interface name string is unavailable");
		return -EINVAL;
	}

	iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
	/* Check if there is already an existing NAN interface */
	nan_vdev = wlan_util_get_vdev_by_ifname(psoc, iface_name, WLAN_NAN_ID);
	if (!nan_vdev) {
		cfg80211_err("NAN data interface %s not available", iface_name);
		return -EINVAL;
	}

	if (nan_vdev->vdev_mlme.vdev_opmode != QDF_NDI_MODE) {
		cfg80211_err("Interface found is not NDI");
		return -EINVAL;
	}

	state = ucfg_nan_get_ndi_state(nan_vdev);
	if (state == NAN_DATA_NDI_DELETED_STATE ||
	    state == NAN_DATA_NDI_DELETING_STATE ||
	    state == NAN_DATA_NDI_CREATING_STATE) {
		cfg80211_err("Data request not allowed in current NDI state:%d",
			state);
		ret = -EAGAIN;
		goto responder_req_failed;
	}

	req.vdev = nan_vdev;

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("Transaction ID is unavailable");
		ret = -EINVAL;
		goto responder_req_failed;
	}
	req.transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID]) {
		cfg80211_err("Instance ID is unavailable");
		ret = -EINVAL;
		goto responder_req_failed;
	}
	req.ndp_instance_id =
		nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE]) {
		cfg80211_err("ndp_rsp is unavailable");
		ret = -EINVAL;
		goto responder_req_failed;
	}
	req.ndp_rsp = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]) {
		req.ndp_info.ndp_app_info_len =
			nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]);
		qdf_mem_copy(req.ndp_info.ndp_app_info,
			     nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO]),
			     req.ndp_info.ndp_app_info_len);
	} else {
		cfg80211_debug("NDP app info is unavailable");
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]) {
		/* at present ndp config stores 4 bytes QOS info only */
		req.ndp_config.ndp_cfg_len = 4;
		*((uint32_t *)req.ndp_config.ndp_cfg) =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS]);
	} else {
		cfg80211_debug("NDP config data is unavailable");
	}

	if (os_if_nan_parse_security_params(tb, &req.ncs_sk_type, &req.pmk,
			&req.passphrase, &req.service_name)) {
		cfg80211_err("inconsistent security params in request.");
		ret = -EINVAL;
		goto responder_req_failed;
	}

	cfg80211_debug("vdev_id: %d, transaction_id: %d, ndp_rsp %d, ndp_instance_id: %d, ndp_app_info_len: %d, csid: %d",
		wlan_vdev_get_id(nan_vdev), req.transaction_id, req.ndp_rsp,
		req.ndp_instance_id, req.ndp_info.ndp_app_info_len,
		req.ncs_sk_type);

	status = ucfg_nan_req_processor(nan_vdev, &req, NDP_RESPONDER_REQ);
	ret = qdf_status_to_os_return(status);

responder_req_failed:
	wlan_objmgr_vdev_release_ref(nan_vdev, WLAN_NAN_ID);

	return ret;

}

/**
 * os_if_nan_process_ndp_end_req() - NDP end request handler
 * @psoc: pointer to psoc object
 *
 * @tb: parsed NL attribute list
 * tb includes following vendor attributes:
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID
 *
 * Return: 0 on success or error code on failure
 */
static int os_if_nan_process_ndp_end_req(struct wlan_objmgr_psoc *psoc,
					 struct nlattr **tb)
{
	int ret = 0;
	QDF_STATUS status;
	struct wlan_objmgr_vdev *nan_vdev;
	struct nan_datapath_end_req req = {0};

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("Transaction ID is unavailable");
		return -EINVAL;
	}
	req.transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY]) {
		cfg80211_err("NDP instance ID array is unavailable");
		return -EINVAL;
	}

	req.num_ndp_instances =
		nla_len(tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY]) /
			sizeof(uint32_t);
	if (0 >= req.num_ndp_instances) {
		cfg80211_err("Num NDP instances is 0");
		return -EINVAL;
	}
	qdf_mem_copy(req.ndp_ids,
		     tb[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY],
		     req.num_ndp_instances * sizeof(uint32_t));

	cfg80211_debug("sending ndp_end_req to SME, transaction_id: %d",
		req.transaction_id);

	nan_vdev = wlan_objmgr_get_vdev_by_opmode_from_psoc(psoc, QDF_NDI_MODE,
							    WLAN_NAN_ID);
	if (!nan_vdev) {
		cfg80211_err("NAN data interface is not available");
		return -EINVAL;
	}

	status = ucfg_nan_req_processor(nan_vdev, &req, NDP_END_REQ);
	ret = qdf_status_to_os_return(status);
	wlan_objmgr_vdev_release_ref(nan_vdev, WLAN_NAN_ID);

	return ret;
}

int os_if_nan_process_ndp_cmd(struct wlan_objmgr_psoc *psoc,
			      const void *data, int data_len)
{
	uint32_t ndp_cmd_type;
	uint16_t transaction_id;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1];
	char *iface_name;

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX,
				    data, data_len, vendor_attr_policy)) {
		cfg80211_err("Invalid NDP vendor command attributes");
		return -EINVAL;
	}

	/* Parse and fetch NDP Command Type*/
	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]) {
		cfg80211_err("NAN datapath cmd type failed");
		return -EINVAL;
	}
	ndp_cmd_type = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("attr transaction id failed");
		return -EINVAL;
	}
	transaction_id = nla_get_u16(
			tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
		cfg80211_err("Transaction Id: %d NDPCmd: %d iface_name: %s",
			transaction_id, ndp_cmd_type, iface_name);
	} else {
		cfg80211_err("Transaction Id: %d NDPCmd: %d iface_name: unspecified",
			transaction_id, ndp_cmd_type);
	}

	cfg80211_debug("Received NDP cmd: %d", ndp_cmd_type);
	switch (ndp_cmd_type) {
	case QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE:
		return os_if_nan_process_ndi_create(psoc, tb);
	case QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE:
		return os_if_nan_process_ndi_delete(psoc, tb);
	case QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_REQUEST:
		return os_if_nan_process_ndp_initiator_req(psoc, tb);
	case QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_REQUEST:
		return os_if_nan_process_ndp_responder_req(psoc, tb);
	case QCA_WLAN_VENDOR_ATTR_NDP_END_REQUEST:
		return os_if_nan_process_ndp_end_req(psoc, tb);
	default:
		cfg80211_err("Unrecognized NDP vendor cmd %d", ndp_cmd_type);
		return -EINVAL;
	}

	return -EINVAL;
}

/**
 * os_if_ndp_initiator_rsp_handler() - NDP initiator response handler
 * @vdev: pointer to vdev object
 * @rsp_params: response parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE (4 bytes)
 *
 * Return: none
 */
static void os_if_ndp_initiator_rsp_handler(struct wlan_objmgr_vdev *vdev,
					struct nan_datapath_initiator_rsp *rsp)
{
	uint32_t data_len;
	struct sk_buff *vendor_event;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);

	if (!rsp) {
		cfg80211_err("Invalid NDP Initator response");
		return;
	}

	data_len = (4 * sizeof(uint32_t)) + (1 * sizeof(uint16_t)) +
				NLMSG_HDRLEN + (5 * NLA_HDRLEN);
	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_ATOMIC);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
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

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
			rsp->status))
		goto ndp_initiator_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			rsp->reason))
		goto ndp_initiator_rsp_nla_failed;

	cfg80211_debug("NDP Initiator rsp sent, tid:%d, instance id:%d, status:%d, reason: %d",
	       rsp->transaction_id, rsp->ndp_instance_id, rsp->status,
	       rsp->reason);
	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
	return;
ndp_initiator_rsp_nla_failed:
	cfg80211_err("nla_put api failed");
	kfree_skb(vendor_event);
}


/*
 * os_if_ndp_responder_rsp_handler() - NDP responder response handler
 * @vdev: pointer to vdev object
 * @rsp: response parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE (4 bytes)
 *
 * Return: none
 */
static void os_if_ndp_responder_rsp_handler(struct wlan_objmgr_vdev *vdev,
				      struct nan_datapath_responder_rsp *rsp)
{
	uint16_t data_len;
	struct sk_buff *vendor_event;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);

	if (!rsp) {
		cfg80211_err("Invalid NDP Responder response");
		return;
	}

	cfg80211_debug("NDP Responder,vdev id %d transaction_id %d status code: %d reason %d",
		wlan_vdev_get_id(rsp->vdev), rsp->transaction_id,
		rsp->status, rsp->reason);
	data_len = 3 * sizeof(uint32_t) + sizeof(uint16_t) +
		4 * NLA_HDRLEN + NLMSG_HDRLEN;
	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_ATOMIC);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
	   QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE))
		goto ndp_responder_rsp_nla_failed;

	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
	   rsp->transaction_id))
		goto ndp_responder_rsp_nla_failed;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
	   rsp->status))
		goto ndp_responder_rsp_nla_failed;

	if (nla_put_u32(vendor_event,
	   QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
	   rsp->reason))
		goto ndp_responder_rsp_nla_failed;

	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
	return;
ndp_responder_rsp_nla_failed:
	cfg80211_err("nla_put api failed");
	kfree_skb(vendor_event);
}

/**
 * os_if_ndp_indication_handler() - NDP indication handler
 * @vdev: pointer to vdev object
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
 * QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE(4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_SCID(scid_len in size)
 *
 * Return: none
 */
static void os_if_ndp_indication_handler(struct wlan_objmgr_vdev *vdev,
				struct nan_datapath_indication_event *event)
{
	uint8_t *ifname;
	uint16_t data_len;
	qdf_size_t ifname_len;
	uint32_t ndp_qos_config;
	struct sk_buff *vendor_event;
	enum nan_datapath_state state;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);

	if (!event) {
		cfg80211_err("Invalid NDP Indication");
		return;
	}

	cfg80211_debug("NDP Indication, policy: %d", event->policy);
	state = ucfg_nan_get_ndi_state(vdev);
	/* check if we are in middle of deleting/creating the interface */

	if (state == NAN_DATA_NDI_DELETED_STATE ||
	    state == NAN_DATA_NDI_DELETING_STATE ||
	    state == NAN_DATA_NDI_CREATING_STATE) {
		cfg80211_err("Data request not allowed in current NDI state: %d",
			state);
		return;
	}

	ifname = wlan_util_vdev_get_if_name(vdev);
	if (!ifname) {
		cfg80211_err("ifname is null");
		return;
	}
	ifname_len = qdf_str_len(ifname);
	if (ifname_len > IFNAMSIZ) {
		cfg80211_err("ifname(%zu) too long", ifname_len);
		return;
	}

	data_len = (5 * sizeof(uint32_t)) + (2 * QDF_MAC_ADDR_SIZE) +
		ifname_len + event->ndp_info.ndp_app_info_len +
		event->scid.scid_len + (10 * NLA_HDRLEN) + NLMSG_HDRLEN;

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy,
					NULL, data_len,
					QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
					GFP_ATOMIC);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND))
		goto ndp_indication_nla_failed;

	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR,
		    ifname_len, ifname))
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
		    QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR,
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

	if (event->scid.scid_len) {
		if (nla_put_u32(vendor_event,
				QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE,
				event->ncs_sk_type))
			goto ndp_indication_nla_failed;

		if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SCID,
			    event->scid.scid_len,
			    event->scid.scid))
			goto ndp_indication_nla_failed;

		cfg80211_debug("csid: %d, scid_len: %d",
			       event->ncs_sk_type, event->scid.scid_len);

		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
				   event->scid.scid, event->scid.scid_len);
	}

	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
	return;
ndp_indication_nla_failed:
	cfg80211_err("nla_put api failed");
	kfree_skb(vendor_event);
}

/**
 * os_if_ndp_confirm_ind_handler() - NDP confirm indication handler
 * @vdev: pointer to vdev object
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
static void os_if_ndp_confirm_ind_handler(struct wlan_objmgr_vdev *vdev,
				struct nan_datapath_confirm_event *ndp_confirm)
{
	int idx = 0;
	uint8_t *ifname;
	uint32_t data_len;
	QDF_STATUS status;
	qdf_size_t ifname_len;
	uint32_t ndp_qos_config = 0;
	struct sk_buff *vendor_event;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);
	struct nan_callbacks cb_obj;

	if (!ndp_confirm) {
		cfg80211_err("Invalid NDP Initator response");
		return;
	}

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("couldn't get callbacks");
		return;
	}

	/* ndp_confirm is called each time user generated ndp req succeeds */
	idx = cb_obj.get_peer_idx(wlan_vdev_get_id(vdev),
				&ndp_confirm->peer_ndi_mac_addr);

	if (idx < 0)
		cfg80211_err("can't find addr: %pM in vdev_id: %d, peer table.",
			&ndp_confirm->peer_ndi_mac_addr,
			wlan_vdev_get_id(vdev));
	else if (ndp_confirm->rsp_code == NAN_DATAPATH_RESPONSE_ACCEPT) {
		uint32_t active_sessions =
			ucfg_nan_get_active_ndp_sessions(vdev, idx);
		ucfg_nan_set_active_ndp_sessions(vdev, active_sessions + 1,
						 idx);
	}

	ifname = wlan_util_vdev_get_if_name(vdev);
	if (!ifname) {
		cfg80211_err("ifname is null");
		return;
	}
	ifname_len = qdf_str_len(ifname);
	if (ifname_len > IFNAMSIZ) {
		cfg80211_err("ifname(%zu) too long", ifname_len);
		return;
	}

	data_len = (4 * sizeof(uint32_t)) + QDF_MAC_ADDR_SIZE + ifname_len +
			+ NLMSG_HDRLEN + (7 * NLA_HDRLEN) +
			ndp_confirm->ndp_info.ndp_app_info_len;

	if (ndp_confirm->ndp_info.ndp_app_info_len)
		data_len += NLA_HDRLEN + ndp_confirm->ndp_info.ndp_app_info_len;

	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_ATOMIC);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
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
		    ifname_len, ifname))
		goto ndp_confirm_nla_failed;

	if (ndp_confirm->ndp_info.ndp_app_info_len &&
		nla_put(vendor_event,
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

	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
	cfg80211_debug("NDP confim sent, ndp instance id: %d, peer addr: %pM, ndp_cfg: %d, rsp_code: %d, reason_code: %d",
		       ndp_confirm->ndp_instance_id,
		       ndp_confirm->peer_ndi_mac_addr.bytes,
		       ndp_qos_config, ndp_confirm->rsp_code,
		       ndp_confirm->reason_code);

	cfg80211_debug("NDP confim, ndp app info dump");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   ndp_confirm->ndp_info.ndp_app_info,
			   ndp_confirm->ndp_info.ndp_app_info_len);
	return;
ndp_confirm_nla_failed:
	cfg80211_err("nla_put api failed");
	kfree_skb(vendor_event);
}

/**
 * os_if_ndp_end_rsp_handler() - NDP end response handler
 * @vdev: pointer to vdev object
 * @rsp_params: response parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE(4 bytest)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID (2 bytes)
 *
 * Return: none
 */
static void os_if_ndp_end_rsp_handler(struct wlan_objmgr_vdev *vdev,
				struct nan_datapath_end_rsp_event *rsp)
{
	uint32_t data_len;
	struct sk_buff *vendor_event;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);

	if (!rsp) {
		cfg80211_err("Invalid ndp end response");
		return;
	}

	data_len = NLMSG_HDRLEN + (4 * NLA_HDRLEN) + (3 * sizeof(uint32_t)) +
		   sizeof(uint16_t);

	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_ATOMIC);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE))
		goto ndp_end_rsp_nla_failed;

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
			rsp->status))
		goto ndp_end_rsp_nla_failed;

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			rsp->reason))
		goto ndp_end_rsp_nla_failed;

	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
			rsp->transaction_id))
		goto ndp_end_rsp_nla_failed;

	cfg80211_debug("NDP End rsp sent, transaction id: %d, status: %d, reason: %d",
	       rsp->transaction_id, rsp->status, rsp->reason);
	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
	return;

ndp_end_rsp_nla_failed:
	cfg80211_err("nla_put api failed");
	kfree_skb(vendor_event);
}

/**
 * os_if_ndp_end_ind_handler() - NDP end indication handler
 * @vdev: pointer to vdev object
 * @ind_params: indication parameters
 *
 * Following vendor event is sent to cfg80211:
 * QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD =
 *         QCA_WLAN_VENDOR_ATTR_NDP_END_IND (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY (4 * num of NDP Instances)
 *
 * Return: none
 */
static void os_if_ndp_end_ind_handler(struct wlan_objmgr_vdev *vdev,
			struct nan_datapath_end_indication_event *end_ind)
{
	QDF_STATUS status;
	uint32_t data_len, i;
	struct nan_callbacks cb_obj;
	uint32_t *ndp_instance_array;
	struct sk_buff *vendor_event;
	struct wlan_objmgr_vdev *vdev_itr;
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("failed to get callbacks");
		return;
	}

	if (!end_ind) {
		cfg80211_err("Invalid ndp end indication");
		return;
	}

	ndp_instance_array = qdf_mem_malloc(end_ind->num_ndp_ids *
		sizeof(*ndp_instance_array));
	if (!ndp_instance_array) {
		cfg80211_err("Failed to allocate ndp_instance_array");
		return;
	}
	for (i = 0; i < end_ind->num_ndp_ids; i++) {
		int idx = 0;

		ndp_instance_array[i] = end_ind->ndp_map[i].ndp_instance_id;
		vdev_itr = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
				end_ind->ndp_map[i].vdev_id, WLAN_NAN_ID);

		if (vdev_itr == NULL) {
			cfg80211_err("vdev not found for vdev_id: %d",
				end_ind->ndp_map[i].vdev_id);
			continue;
		}

		idx = cb_obj.get_peer_idx(wlan_vdev_get_id(vdev),
				&end_ind->ndp_map[i].peer_ndi_mac_addr);
		if (idx < 0) {
			cfg80211_err("can't find addr: %pM in sta_ctx.",
				&end_ind->ndp_map[i].peer_ndi_mac_addr);
			continue;
		}
		/* save the value of active sessions on each peer */
		ucfg_nan_set_active_ndp_sessions(vdev,
				end_ind->ndp_map[i].num_active_ndp_sessions,
				idx);
	}

	data_len = (sizeof(uint32_t)) + NLMSG_HDRLEN + (2 * NLA_HDRLEN) +
			end_ind->num_ndp_ids * sizeof(*ndp_instance_array);

	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy, NULL,
				data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_ATOMIC);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_END_IND))
		goto ndp_end_ind_nla_failed;

	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY,
			end_ind->num_ndp_ids * sizeof(*ndp_instance_array),
			ndp_instance_array))
		goto ndp_end_ind_nla_failed;

	cfg80211_vendor_event(vendor_event, GFP_ATOMIC);
	qdf_mem_free(ndp_instance_array);
	return;

ndp_end_ind_nla_failed:
	cfg80211_err("nla_put api failed");
	kfree_skb(vendor_event);
	qdf_mem_free(ndp_instance_array);
}

/**
 * os_if_new_peer_ind_handler() - NDP new peer indication handler
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void os_if_new_peer_ind_handler(struct wlan_objmgr_vdev *vdev,
			struct nan_datapath_peer_ind *peer_ind)
{
	int ret;
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	uint32_t active_peers = ucfg_nan_get_active_peers(vdev);
	struct nan_callbacks cb_obj;

	if (NULL == peer_ind) {
		cfg80211_err("Invalid new NDP peer params");
		return;
	}

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("failed to get callbacks");
		return;
	}

	cfg80211_debug("session_id: %d, peer_mac: %pM, sta_id: %d",
		peer_ind->session_id, peer_ind->peer_mac_addr.bytes,
		peer_ind->sta_id);
	ret = cb_obj.new_peer_ind(vdev_id, peer_ind->sta_id,
				&peer_ind->peer_mac_addr,
				(active_peers == 0 ? true : false));
	if (ret) {
		cfg80211_err("new peer handling at HDD failed %d", ret);
		return;
	}

	active_peers++;
	ucfg_nan_set_active_peers(vdev, active_peers);
	cfg80211_debug("vdev_id: %d, num_peers: %d", vdev_id, active_peers);
}

/**
 * os_if_peer_departed_ind_handler() - Handle NDP peer departed indication
 * @adapter: pointer to adapter context
 * @ind_params: indication parameters
 *
 * Return: none
 */
static void os_if_peer_departed_ind_handler(struct wlan_objmgr_vdev *vdev,
			struct nan_datapath_peer_ind *peer_ind)
{
	QDF_STATUS status;
	struct nan_callbacks cb_obj;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	uint32_t active_peers = ucfg_nan_get_active_peers(vdev);

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("failed to get callbacks");
		return;
	}

	if (NULL == peer_ind) {
		cfg80211_err("Invalid new NDP peer params");
		return;
	}
	cfg80211_debug("session_id: %d, peer_mac: %pM, sta_id: %d",
		peer_ind->session_id, peer_ind->peer_mac_addr.bytes,
		peer_ind->sta_id);
	active_peers--;
	ucfg_nan_set_active_peers(vdev, active_peers);
	cb_obj.peer_departed_ind(vdev_id, peer_ind->sta_id,
				&peer_ind->peer_mac_addr,
				(active_peers == 0 ? true : false));
}

/**
 * os_if_ndp_iface_create_rsp_handler() - NDP iface create response handler
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
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE (4 bytes)
 * QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE
 *
 * Return: none
 */
static void os_if_ndp_iface_create_rsp_handler(struct wlan_objmgr_psoc *psoc,
					       struct wlan_objmgr_vdev *vdev,
					       void *rsp_params)
{
	QDF_STATUS status;
	bool create_fail = false;
	struct nan_callbacks cb_obj;
	struct sk_buff *vendor_event;
	uint16_t create_transaction_id;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);
	uint32_t create_status = NAN_DATAPATH_RSP_STATUS_ERROR;
	uint32_t create_reason = NAN_DATAPATH_NAN_DATA_IFACE_CREATE_FAILED;
	struct nan_datapath_inf_create_rsp *ndi_rsp =
			(struct nan_datapath_inf_create_rsp *)rsp_params;
	uint32_t data_len = (3 * sizeof(uint32_t)) + sizeof(uint16_t) +
				NLMSG_HDRLEN + (4 * NLA_HDRLEN);

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Couldn't get ballback object");
		return;
	}

	if (ndi_rsp) {
		create_status = ndi_rsp->status;
		create_reason = ndi_rsp->reason;
	} else {
		cfg80211_err("Invalid ndi create response");
		create_fail = true;
	}

	create_transaction_id = ucfg_nan_get_ndp_create_transaction_id(vdev);

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy,
				NULL,
				data_len,
				QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
				GFP_KERNEL);
	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
		create_fail = true;
		goto close_ndi;
	}

	/* Sub vendor command */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE)) {
		cfg80211_err("QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD put fail");
		goto nla_put_failure;
	}

	/* Transaction id */
	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		create_transaction_id)) {
		cfg80211_err("VENDOR_ATTR_NDP_TRANSACTION_ID put fail");
		goto nla_put_failure;
	}

	/* Status code */
	if (nla_put_u32(vendor_event,
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
		create_status)) {
		cfg80211_err("VENDOR_ATTR_NDP_DRV_RETURN_TYPE put fail");
		goto nla_put_failure;
	}

	/* Status return value */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			create_reason)) {
		cfg80211_err("VENDOR_ATTR_NDP_DRV_RETURN_VALUE put fail");
		goto nla_put_failure;
	}

	cfg80211_debug("sub command: %d, value: %d",
		QCA_NL80211_VENDOR_SUBCMD_NDP,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE);
	cfg80211_debug("create transaction id: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID, create_transaction_id);
	cfg80211_debug("status code: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
		create_status);
	cfg80211_debug("Return value: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE, create_reason);

	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	if (!create_fail) {
		/* update txrx queues and register self sta */
		cb_obj.drv_ndi_create_rsp_handler(wlan_vdev_get_id(vdev),
						  ndi_rsp);
	} else {
		cfg80211_err("NDI interface creation failed with reason %d",
			     create_reason);
		goto close_ndi;
	}

	return;

nla_put_failure:
	kfree_skb(vendor_event);
close_ndi:
	cb_obj.ndi_close(wlan_vdev_get_id(vdev));
	return;
}

/**
 * os_if_ndp_iface_delete_rsp_handler() - NDP iface delete response handler
 * @adapter: pointer to adapter context
 * @rsp_params: response parameters
 *
 * Return: none
 */
static void os_if_ndp_iface_delete_rsp_handler(struct wlan_objmgr_psoc *psoc,
					      struct wlan_objmgr_vdev *vdev,
					      void *rsp_params)
{
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct nan_datapath_inf_delete_rsp *ndi_rsp = rsp_params;
	struct nan_callbacks cb_obj;

	if (!ndi_rsp) {
		cfg80211_err("Invalid ndi delete response");
		return;
	}

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Couldn't get ballback object");
		return;
	}

	if (ndi_rsp->status == NAN_DATAPATH_RSP_STATUS_SUCCESS)
		cfg80211_debug("NDI BSS successfully stopped");
	else
		cfg80211_debug("NDI BSS stop failed with reason %d",
				ndi_rsp->reason);

	ucfg_nan_set_ndi_delete_rsp_reason(vdev, ndi_rsp->reason);
	ucfg_nan_set_ndi_delete_rsp_status(vdev, ndi_rsp->status);
	cb_obj.drv_ndi_delete_rsp_handler(vdev_id);
}

void os_if_nan_event_handler(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_vdev *vdev,
			     uint32_t type, void *msg)
{
	switch (type) {
	case NAN_DATAPATH_INF_CREATE_RSP:
		os_if_ndp_iface_create_rsp_handler(psoc, vdev, msg);
		break;
	case NAN_DATAPATH_INF_DELETE_RSP:
		os_if_ndp_iface_delete_rsp_handler(psoc, vdev, msg);
		break;
	case NDP_CONFIRM:
		os_if_ndp_confirm_ind_handler(vdev, msg);
		break;
	case NDP_INITIATOR_RSP:
		os_if_ndp_initiator_rsp_handler(vdev, msg);
		break;
	case NDP_INDICATION:
		os_if_ndp_indication_handler(vdev, msg);
		break;
	case NDP_NEW_PEER:
		os_if_new_peer_ind_handler(vdev, msg);
		break;
	case NDP_RESPONDER_RSP:
		os_if_ndp_responder_rsp_handler(vdev, msg);
		break;
	case NDP_END_RSP:
		os_if_ndp_end_rsp_handler(vdev, msg);
		break;
	case NDP_END_IND:
		os_if_ndp_end_ind_handler(vdev, msg);
		break;
	case NDP_PEER_DEPARTED:
		os_if_peer_departed_ind_handler(vdev, msg);
		break;
	default:
		break;
	}
}

int os_if_nan_register_hdd_callbacks(struct wlan_objmgr_psoc *psoc,
				     struct nan_callbacks *cb_obj)
{
	return ucfg_nan_register_hdd_callbacks(psoc, cb_obj,
						os_if_nan_event_handler);
}

int os_if_nan_register_lim_callbacks(struct wlan_objmgr_psoc *psoc,
				     struct nan_callbacks *cb_obj)
{
	return ucfg_nan_register_lim_callbacks(psoc, cb_obj);
}

void os_if_nan_post_ndi_create_rsp(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id, bool success)
{
	struct nan_datapath_inf_create_rsp rsp = {0};
	struct wlan_objmgr_vdev *vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
						psoc, vdev_id, WLAN_NAN_ID);

	if (!vdev) {
		cfg80211_err("vdev is null");
		return;
	}

	if (success) {
		rsp.status = NAN_DATAPATH_RSP_STATUS_SUCCESS;
		rsp.reason = 0;
		os_if_nan_event_handler(psoc, vdev,
					NAN_DATAPATH_INF_CREATE_RSP, &rsp);
	} else {
		rsp.status = NAN_DATAPATH_RSP_STATUS_ERROR;
		rsp.reason = NAN_DATAPATH_NAN_DATA_IFACE_CREATE_FAILED;
		os_if_nan_event_handler(psoc, vdev,
					NAN_DATAPATH_INF_CREATE_RSP, &rsp);
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
}

void os_if_nan_post_ndi_delete_rsp(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id, bool success)
{
	struct nan_datapath_inf_delete_rsp rsp = {0};
	struct wlan_objmgr_vdev *vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
						psoc, vdev_id, WLAN_NAN_ID);
	if (!vdev) {
		cfg80211_err("vdev is null");
		return;
	}

	if (success) {
		rsp.status = NAN_DATAPATH_RSP_STATUS_SUCCESS;
		rsp.reason = 0;
		os_if_nan_event_handler(psoc, vdev,
					NAN_DATAPATH_INF_DELETE_RSP, &rsp);
	} else {
		rsp.status = NAN_DATAPATH_RSP_STATUS_ERROR;
		rsp.reason = NAN_DATAPATH_NAN_DATA_IFACE_DELETE_FAILED;
		os_if_nan_event_handler(psoc, vdev,
					NAN_DATAPATH_INF_DELETE_RSP, &rsp);
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
}

void os_if_nan_ndi_session_end(struct wlan_objmgr_vdev *vdev)
{
	struct sk_buff *vendor_event;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct pdev_osif_priv *os_priv = wlan_pdev_get_ospriv(pdev);
	uint32_t data_len = sizeof(uint32_t) * (3 + sizeof(uint16_t)) +
				(NLA_HDRLEN * 4) + NLMSG_HDRLEN;

	/*
	 * The virtual adapters are stopped and closed even during
	 * driver unload or stop, the service layer is not required
	 * to be informed in that case (response is not expected)
	 */
	if (NAN_DATA_NDI_DELETING_STATE != ucfg_nan_get_ndi_state(vdev)) {
		cfg80211_err("NDI interface deleted");
		return;
	}

	/* notify response to the upper layer */
	vendor_event = cfg80211_vendor_event_alloc(os_priv->wiphy, NULL,
			data_len, QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
			GFP_KERNEL);

	if (!vendor_event) {
		cfg80211_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	/* Sub vendor command goes first */
	if (nla_put_u32(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
			QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE)) {
		cfg80211_err("VENDOR_ATTR_NDP_SUBCMD put fail");
		goto failure;
	}

	/* Transaction id */
	if (nla_put_u16(vendor_event, QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
			ucfg_nan_get_ndp_delete_transaction_id(vdev))) {
		cfg80211_err("VENDOR_ATTR_NDP_TRANSACTION_ID put fail");
		goto failure;
	}

	/* Status code */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
			ucfg_nan_get_ndi_delete_rsp_status(vdev))) {
		cfg80211_err("VENDOR_ATTR_NDP_DRV_RETURN_TYPE put fail");
		goto failure;
	}

	/* Status return value */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
			ucfg_nan_get_ndi_delete_rsp_reason(vdev))) {
		cfg80211_err("VENDOR_ATTR_NDP_DRV_RETURN_VALUE put fail");
		goto failure;
	}

	cfg80211_debug("sub command: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
		QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE);
	cfg80211_debug("delete transaction id: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
		ucfg_nan_get_ndp_delete_transaction_id(vdev));
	cfg80211_debug("status code: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
		ucfg_nan_get_ndi_delete_rsp_status(vdev));
	cfg80211_debug("Return value: %d, value: %d",
		QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
		ucfg_nan_get_ndi_delete_rsp_reason(vdev));

	ucfg_nan_set_ndp_delete_transaction_id(vdev, 0);
	ucfg_nan_set_ndi_state(vdev, NAN_DATA_NDI_DELETED_STATE);
	cfg80211_vendor_event(vendor_event, GFP_KERNEL);

	return;
failure:
	kfree_skb(vendor_event);
}
