/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * DOC: declares nan component os interface APIs
 */

#ifndef _OS_IF_NAN_H_
#define _OS_IF_NAN_H_

#include "qdf_types.h"

struct wlan_objmgr_psoc;
struct wlan_objmgr_vdev;

#ifdef WLAN_FEATURE_NAN_CONVERGENCE

#define NDP_QOS_INFO_LEN 255
#define NDP_APP_INFO_LEN 255
#define NDP_PMK_LEN 32
#define NDP_SCID_BUF_LEN 256
#define NDP_NUM_INSTANCE_ID 255

/**
 * enum qca_wlan_vendor_attr_ndp_params - vendor attribute parameters
 * @QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD: NDP Sub command
 * @QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID: Transaction id reference
 * @QCA_WLAN_VENDOR_ATTR_NDP_STATUS_ID: NDP status id
 * @QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID: Service instance id
 * @QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL: Requested channel
 * @QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR: Peer discovery mac addr
 * @QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR: Iface name
 * @QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY: Security configuration
 * @QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS: Qos configuration
 * @QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO: Application info
 * @QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID: NDP instance id
 * @QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY: NDP instance id array
 * @QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE: Schedule response
 * @QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR: NDI mac address
 * @QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE: Driver return status
 * @QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE: Driver return value
 * @QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG: Channel config request type
 * @QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE: Cipher Suit ID
 * @QCA_WLAN_VENDOR_ATTR_NDP_PMK: Pairwise Master Key
 * @QCA_WLAN_VENDOR_ATTR_NDP_SCID: Security Context ID
 */
enum qca_wlan_vendor_attr_ndp_params {
	QCA_WLAN_VENDOR_ATTR_NDP_PARAM_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD,
	QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID,
	QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID,
	QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR,
	/* CONFIG_SECURITY is deprecated, use NCS_SK_TYPE/PMK/SCID instead */
	QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY,
	QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS,
	QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO,
	QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID,
	QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY,
	QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE,
	QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE,
	QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE,
	QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG,
	QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE,
	QCA_WLAN_VENDOR_ATTR_NDP_PMK,
	QCA_WLAN_VENDOR_ATTR_NDP_SCID,

	QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX =
		QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ndp_sub_cmd_value - NDP subcmd value
 * @QCA_WLAN_VENDOR_ATTR_NDP_INVALID: Unused subcmd value
 * @QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE: iface create
 * @QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE: iface delete
 * @QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_REQUEST: NDP initiator request
 * @QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE: NDP initiator response
 * @QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_REQUEST: NDP responder request
 * @QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE: NDP responder response
 * @QCA_WLAN_VENDOR_ATTR_NDP_END_REQUEST: NDP end request
 * @QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE: NDP end response
 * @QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND: NDP request indication
 * @QCA_WLAN_VENDOR_ATTR_NDP_CONFIRM_IND: NDP confirm indication
 * @QCA_WLAN_VENDOR_ATTR_NDP_END_IND: NDP End indication
 */
enum qca_wlan_vendor_attr_ndp_sub_cmd_value {
	QCA_WLAN_VENDOR_ATTR_NDP_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE = 1,
	QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE = 2,
	QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_REQUEST = 3,
	QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE = 4,
	QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_REQUEST = 5,
	QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE = 6,
	QCA_WLAN_VENDOR_ATTR_NDP_END_REQUEST = 7,
	QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE = 8,
	QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND = 9,
	QCA_WLAN_VENDOR_ATTR_NDP_CONFIRM_IND = 10,
	QCA_WLAN_VENDOR_ATTR_NDP_END_IND = 11
};

#endif /* WLAN_FEATURE_NAN_CONVERGENCE */

/**
 * os_if_nan_process_ndp_cmd: os_if api to handle nan request message
 * @psoc: pointer to psoc object
 * @data: request data. contains vendor cmd tlvs
 * @data_len: length of data
 *
 * Return: status of operation
 */
int os_if_nan_process_ndp_cmd(struct wlan_objmgr_psoc *psoc,
				const void *data, int data_len);

/**
 * os_if_nan_event_handler: os_if handler api for nan response messages
 * @psoc: pointer to psoc object
 * @vdev: pointer to vdev object
 * @type: message type
 * @msg: msg buffer
 *
 * Return: None
 */
void os_if_nan_event_handler(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_vdev *vdev,
			     uint32_t type, void *msg);

#endif
