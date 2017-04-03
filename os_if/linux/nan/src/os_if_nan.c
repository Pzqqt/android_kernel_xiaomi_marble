/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#include "qdf_types.h"
#include "qdf_trace.h"
#include "os_if_nan.h"
#include "wlan_nan_api.h"
#include "nan_public_structs.h"
#include "wlan_osif_priv.h"
#include <net/cfg80211.h>
#include "wlan_cfg80211.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"

/* NLA policy */
static const struct nla_policy
vendor_attr_policy[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1] = {
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
	[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PMK] = { .type = NLA_BINARY,
					.len = NDP_PMK_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_SCID] = { .type = NLA_BINARY,
					.len = NDP_SCID_BUF_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE] = { .type =
					NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE] = { .type = NLA_U32 },
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

	nan_vdev = ucfg_nan_get_ndi_vdev(psoc, WLAN_NAN_ID);
	if (!nan_vdev) {
		cfg80211_err("Nan datapath interface is not present");
		return -EINVAL;
	}

	transaction_id =
		nla_get_u16(tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);
	vdev_id = wlan_vdev_get_id(nan_vdev);
	num_peers = ucfg_nan_get_active_peers(nan_vdev);
	/*
	 * wlan_objmgr_get_vdev_by_opmode_from_psoc API will have incremented
	 * ref count - decrement here since vdev returned by that api is not
	 * used any more
	 */
	wlan_objmgr_vdev_release_ref(nan_vdev, WLAN_NAN_ID);

	/* check if there are active peers on the adapter */
	if (num_peers) {
		cfg80211_err("NDP peers active: %d, cannot delete NDI",
			     num_peers);
		return -EINVAL;
	}

	status = ucfg_nan_get_callbacks(psoc, &cb_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Couldn't get ballback object");
		return -EINVAL;
	}

	return cb_obj.ndi_delete(vdev_id, iface_name, transaction_id);
}

int os_if_nan_process_ndp_cmd(struct wlan_objmgr_psoc *psoc,
			      const void *data, int data_len)
{
	uint32_t ndp_cmd_type;
	uint16_t transaction_id;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1];
	char *iface_name;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX,
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
	default:
		cfg80211_err("Unrecognized NDP vendor cmd %d", ndp_cmd_type);
		return -EINVAL;
	}

	return -EINVAL;
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
	uint8_t create_transaction_id;
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
				ndi_rsp->reason);
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

void os_if_nan_post_ndi_create_rsp(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id, bool success)
{
	struct nan_datapath_inf_create_rsp rsp = {0};
	struct wlan_objmgr_vdev *vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
						psoc, vdev_id, WLAN_NAN_ID);

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
