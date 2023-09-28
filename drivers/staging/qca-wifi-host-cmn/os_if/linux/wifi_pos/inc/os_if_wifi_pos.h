/*
 * Copyright (c) 2012-2017, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: os_if_wifi_pos.h
 * This file provide declaration of wifi_pos's os_if APIs
 */
#ifndef _OS_IF_WIFI_POS_H_
#define _OS_IF_WIFI_POS_H_

#include "qdf_types.h"
#include "qdf_status.h"


/* forward declaration */
struct wifi_pos_ch_info;
struct wlan_objmgr_psoc;
struct wifi_pos_driver_caps;

#ifdef WIFI_POS_CONVERGED
/**
 * os_if_wifi_pos_register_nl() - abstration API to register callback with GENL
 * socket.
 *
 * Return: status of operation
 */
int os_if_wifi_pos_register_nl(void);

/**
 * os_if_wifi_pos_deregister_nl() - abstration API to deregister callback with
 * GENL socket.
 *
 * Return: status of operation
 */
int os_if_wifi_pos_deregister_nl(void);

/**
 * os_if_wifi_pos_send_peer_status() - Function to send peer status to a
 * registered application
 * @peer_mac: MAC address of peer
 * @peer_status: ePeerConnected or ePeerDisconnected
 * @peer_timing_meas_cap: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @session_id: SME session id, i.e. vdev_id
 * @chan_info: operating channel information
 * @dev_mode: dev mode for which indication is sent
 *
 * Return: none
 */
void os_if_wifi_pos_send_peer_status(struct qdf_mac_addr *peer_mac,
				uint8_t peer_status,
				uint8_t peer_timing_meas_cap,
				uint8_t session_id,
				struct wifi_pos_ch_info *chan_info,
				enum QDF_OPMODE dev_mode);

/**
 * os_if_wifi_pos_populate_caps() - populate oem capabilities
 * @psoc: psoc object
 * @caps: pointer to populate the capabilities
 *
 * Return: error code
 */
int os_if_wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
				struct wifi_pos_driver_caps *caps);
#else
static inline int os_if_wifi_pos_register_nl(void)
{
	return 0;
}

static inline int os_if_wifi_pos_deregister_nl(void)
{
	return 0;
}

static inline void os_if_wifi_pos_send_peer_status(
		struct qdf_mac_addr *peer_mac,
		uint8_t peer_status,
		uint8_t peer_timing_meas_cap,
		uint8_t session_id,
		struct wifi_pos_ch_info *chan_info,
		enum QDF_OPMODE dev_mode)
{
}

static inline int os_if_wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_driver_caps *caps)
{
	return 0;
}
#endif

#ifdef CNSS_GENL
/**
 * enum cld80211_vendor_sub_cmds
 * @CLD80211_VENDOR_SUB_CMD_INVALID: invalid cmd type
 * @CLD80211_VENDOR_SUB_CMD_REGISTRATION: app registration
 * @CLD80211_VENDOR_SUB_CMD_SET_CAPS: set driver capabilities
 * @CLD80211_VENDOR_SUB_CMD_GET_CAPS: get driver capabilities
 * @CLD80211_VENDOR_SUB_CMD_GET_CH_INFO: get channel info
 * @CLD80211_VENDOR_SUB_CMD_OEM_DATA: oem data req/rsp
 * @CLD80211_VENDOR_SUB_CMD_OEM_ERROR: oem error rsp
 * @CLD80211_VENDOR_SUB_CMD_PEER_STATUS_IND: peer status indication
 * @CLD80211_VENDOR_SUB_CMD_MAX: Max cld80211 vendor sub cmds
 */
enum cld80211_vendor_sub_cmds {
	CLD80211_VENDOR_SUB_CMD_INVALID = 0,
	CLD80211_VENDOR_SUB_CMD_REGISTRATION = 1,
	CLD80211_VENDOR_SUB_CMD_SET_CAPS = 2,
	CLD80211_VENDOR_SUB_CMD_GET_CAPS = 3,
	CLD80211_VENDOR_SUB_CMD_GET_CH_INFO = 4,
	CLD80211_VENDOR_SUB_CMD_OEM_DATA = 5,
	CLD80211_VENDOR_SUB_CMD_OEM_ERROR = 6,
	CLD80211_VENDOR_SUB_CMD_PEER_STATUS_IND = 7,
	/* keep last */
	CLD80211_VENDOR_SUB_CMD__AFTER_LAST,
	CLD80211_VENDOR_SUB_CMD_MAX =
		CLD80211_VENDOR_SUB_CMD__AFTER_LAST - 1
};

/**
 * enum cld80211_sub_attr_cap_rsp - Capability response sub attribute
 * @CLD80211_SUB_ATTR_CAPS_INVALID: Invalid capability
 * @CLD80211_SUB_ATTR_CAPS_OEM_TARGET_SIGNATURE: OEM target signature
 * @CLD80211_SUB_ATTR_CAPS_OEM_TARGET_TYPE: OEM target type
 * @CLD80211_SUB_ATTR_CAPS_OEM_FW_VERSION: OEM firmware version
 * @CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MAJOR: Driver version major
 * @CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MINOR: Driver version minor
 * @CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_PATCH: Driver version patch
 * @CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_BUILD: Driver version build
 * @CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MIN: Allowed dwell time min
 * @CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MAX: Allowed dwell time max
 * @CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MIN: Current dwell time min
 * @CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MAX: Current dwell time max
 * @CLD80211_SUB_ATTR_CAPS_SUPPORTED_BANDS: Supported bands
 * @CLD80211_SUB_ATTR_CAPS_USER_DEFINED_CAPS: User defined capabilities
 * @CLD80211_SUB_ATTR_CAPS_MAX: Max number for CAP sub attribute
 *
 */
enum cld80211_sub_attr_cap_rsp {
	CLD80211_SUB_ATTR_CAPS_INVALID = 0,
	CLD80211_SUB_ATTR_CAPS_OEM_TARGET_SIGNATURE = 1,
	CLD80211_SUB_ATTR_CAPS_OEM_TARGET_TYPE = 2,
	CLD80211_SUB_ATTR_CAPS_OEM_FW_VERSION = 3,
	CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MAJOR = 4,
	CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MINOR = 5,
	CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_PATCH = 6,
	CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_BUILD = 7,
	CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MIN = 8,
	CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MAX = 9,
	CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MIN = 10,
	CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MAX = 11,
	CLD80211_SUB_ATTR_CAPS_SUPPORTED_BANDS = 12,
	CLD80211_SUB_ATTR_CAPS_USER_DEFINED_CAPS = 13,

	/* keep last */
	CLD80211_SUB_ATTR_CAPS_AFTER_LAST,
	CLD80211_SUB_ATTR_CAPS_MAX =
		CLD80211_SUB_ATTR_CAPS_AFTER_LAST - 1
};

/**
 * enum cld80211_sub_attr_channel_rsp - Chan info response sub attribute
 * @CLD80211_SUB_ATTR_CH_RESP_INVALID: Invalid channel resp
 * @CLD80211_SUB_ATTR_CH_MORE_DATA: More date sub attr for frag response
 * @CLD80211_SUB_ATTR_CHANNEL_NUM_CHAN: Number of channels in response
 * @CLD80211_SUB_ATTR_CHANNEL_LIST: Channel list nesting
 * @CLD80211_SUB_ATTR_CH_CHAN_ID: Channel number
 * @CLD80211_SUB_ATTR_CH_MHZ: Channel frequency
 * @CLD80211_SUB_ATTR_CH_BAND_CF_1: Center frequency 1
 * @CLD80211_SUB_ATTR_CH_BAND_CF_2: Center frequency 2
 * @CLD80211_SUB_ATTR_CH_INFO: channel info
 * @CLD80211_SUB_ATTR_CH_REG_INFO_1: regulatory info field 1
 * @CLD80211_SUB_ATTR_CH_REG_INFO_2: regulatory info field 2
 * @CLD80211_SUB_ATTR_CAPS_MAX: Max number for CHAN Info sub attribute
 *
 */
enum cld80211_sub_attr_channel_rsp {
	CLD80211_SUB_ATTR_CH_RESP_INVALID = 0,
	CLD80211_SUB_ATTR_CH_MORE_DATA = 1,
	CLD80211_SUB_ATTR_CHANNEL_NUM_CHAN = 2,
	CLD80211_SUB_ATTR_CH_LIST = 3,
	/* CH_* belongs to CH_LIST */
	CLD80211_SUB_ATTR_CH_CHAN_ID = 4,
	CLD80211_SUB_ATTR_CH_MHZ = 5,
	CLD80211_SUB_ATTR_CH_BAND_CF_1 = 6,
	CLD80211_SUB_ATTR_CH_BAND_CF_2 = 7,
	CLD80211_SUB_ATTR_CH_INFO = 8,
	CLD80211_SUB_ATTR_CH_REG_INFO_1 = 9,
	CLD80211_SUB_ATTR_CH_REG_INFO_2 = 10,

	/* keep last */
	CLD80211_SUB_ATTR_CH_AFTER_LAST,
	CLD80211_SUB_ATTR_CH_MAX =
		CLD80211_SUB_ATTR_CH_AFTER_LAST - 1

};

/**
 * enum cld80211_sub_attr_oem_data_req - OEM data req sub attribute
 * @CLD80211_SUB_ATTR_MSG_OEM_DATA_INVALID: Invalid OEM data request
 * @CLD80211_SUB_ATTR_MSG_OEM_DATA_FW: Data to Firmware
 * @CLD80211_SUB_ATTR_MSG_OEM_DATA_DRIVER: Data to driver
 * @CLD80211_SUB_ATTR_MSG_OEM_DATA_REQ_MAX: Max number for OEM data req sub
 * attribute
 *
 * OEM data request sub attributes are NLA attributes in NLA type OEM data
 * request.
 *
 */
enum cld80211_sub_attr_oem_data_req {
	CLD80211_SUB_ATTR_MSG_OEM_DATA_INVALID = 0,
	CLD80211_SUB_ATTR_MSG_OEM_DATA_FW = 1,
	CLD80211_SUB_ATTR_MSG_OEM_DATA_DRIVER = 2,

	/* keep last */
	CLD80211_SUB_ATTR_MSG_OEM_DATA_REQ_AFTER_LAST,
	CLD80211_SUB_ATTR_MSG_OEM_DATA_REQ_MAX =
		CLD80211_SUB_ATTR_MSG_OEM_DATA_REQ_AFTER_LAST - 1
};

/**
 * enum cld80211_sub_attr_oem_data_resp - OEM message sub attribute
 * @CLD80211_SUB_ATTR_OEM_DATA_INVALID: Invalid oem data resp
 * @CLD80211_SUB_ATTR_OEM_MORE_DATA: more date sub attribute
 * @CLD80211_SUB_ATTR_BINARY_DATA: Binary data sub attribute
 * @CLD80211_SUB_ATTR_OEM_DATA_RESP_MAX: Max number for OEM data resp
 * sub attribute
 *
 * OEM message sub attributes are interface between apps and driver to
 * process NLA type request and response messages.
 *
 */
enum cld80211_sub_attr_oem_data_resp {
	CLD80211_SUB_ATTR_OEM_DATA_INVALID = 0,
	CLD80211_SUB_ATTR_OEM_MORE_DATA = 1,
	CLD80211_SUB_ATTR_BINARY_DATA = 2,

	/* keep last */
	CLD80211_SUB_ATTR_OEM_DATA_RESP_AFTER_LAST,
	CLD80211_SUB_ATTR_OEM_DATA_RESP_MAX =
		CLD80211_SUB_ATTR_OEM_DATA_RESP_AFTER_LAST - 1
};

/**
 * enum cld80211_sub_attr_peer_info - peer info sub attribute
 * @CLD80211_SUB_ATTR_PEER_INVALID: Invalid peer info
 * @CLD80211_SUB_ATTR_PEER_MAC_ADDR: peer mac address
 * @CLD80211_SUB_ATTR_PEER_STATUS: peer status
 * @CLD80211_SUB_ATTR_PEER_VDEV_ID: peer vdevid
 * @CLD80211_SUB_ATTR_PEER_CAPABILITY: peer capabilities
 * @CLD80211_SUB_ATTR_PEER_RESERVED: reserved bytes
 * @CLD80211_SUB_ATTR_PEER_CHAN_INFO: peer channel info
 *
 */
enum cld80211_sub_attr_peer_info {
	CLD80211_SUB_ATTR_PEER_INVALID = 0,
	CLD80211_SUB_ATTR_PEER_MAC_ADDR = 1,
	CLD80211_SUB_ATTR_PEER_STATUS = 2,
	CLD80211_SUB_ATTR_PEER_VDEV_ID = 3,
	CLD80211_SUB_ATTR_PEER_CAPABILITY = 4,
	CLD80211_SUB_ATTR_PEER_RESERVED = 5,
	CLD80211_SUB_ATTR_PEER_CHAN_INFO = 6,

	/* keep last */
	CLD80211_SUB_ATTR_PEER_AFTER_LAST,
	CLD80211_SUB_ATTR_PEER_MAX =
		CLD80211_SUB_ATTR_PEER_AFTER_LAST - 1
};
#endif
#endif /* _OS_IF_WIFI_POS_H_ */
