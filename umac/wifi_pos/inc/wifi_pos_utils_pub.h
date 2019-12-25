/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wifi_pos_utils_pub.h
 * This file declares public utils of wifi positioning component
 */
#ifndef _WIFI_POS_UTILS_PUB_H_
#define _WIFI_POS_UTILS_PUB_H_

/* Include files */
#include "qdf_types.h"
#include "qdf_status.h"
#include "qdf_trace.h"

#define WIFIPOS_RESERVE_BYTES      100
#define OEM_TARGET_SIGNATURE_LEN   8
#define OEM_TARGET_SIGNATURE       "QUALCOMM"

#define MAX_CHANNELS               255
#define OEM_CAP_MAX_NUM_CHANNELS   128

#define WIFI_POS_RSP_V1_FLAT_MEMORY  0x00000001
#define WIFI_POS_RSP_V2_NL  0x00000002

/**
 * enum wifi_pos_cmd_ids
 * @WIFI_POS_CMD_REGISTRATION: app registration
 * @WIFI_POS_CMD_GET_CAPS: get driver capabilities
 * @WIFI_POS_CMD_GET_CH_INFO: get channel info
 * @WIFI_POS_CMD_OEM_DATA: oem data req/rsp
 * @WIFI_POS_CMD_MAX: Max cld80211 vendor sub cmds
 */

enum wifi_pos_cmd_ids {
	WIFI_POS_CMD_INVALID = 0,
	WIFI_POS_CMD_REGISTRATION = 1,
	WIFI_POS_CMD_SET_CAPS = 2,
	WIFI_POS_CMD_GET_CAPS = 3,
	WIFI_POS_CMD_GET_CH_INFO = 4,
	WIFI_POS_CMD_OEM_DATA = 5,
	WIFI_POS_CMD_ERROR = 6,
	WIFI_POS_PEER_STATUS_IND = 7,
	/* keep last */
	WIFI_POS_CMD__AFTER_LAST,
	WIFI_POS_CMD_MAX =
		WIFI_POS_CMD__AFTER_LAST - 1
};


/**
 * struct wifi_pos_driver_version - Driver version identifier (w.x.y.z)
 * @major: Version ID major number
 * @minor: Version ID minor number
 * @patch: Version ID patch number
 * @build: Version ID build number
 */
struct qdf_packed wifi_pos_driver_version {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint8_t build;
};

/**
 * struct wifi_pos_channel_power
 * @center_freq: Channel Center Frequency
 * @chan_num: channel number
 * @tx_power: TX power
 */
struct wifi_pos_channel_power {
	uint32_t center_freq;
	uint32_t chan_num;
	uint32_t tx_power;
};

/**
 * struct wifi_pos_channel_list
 * @valid_channels: no of valid channels
 * @chan_info: channel info
 */
struct qdf_packed wifi_pos_channel_list {
	uint16_t num_channels;
	struct wifi_pos_channel_power chan_info[MAX_CHANNELS];
};

/**
 * struct wifi_pos_driver_caps - OEM Data Capabilities
 * @oem_target_signature: Signature of chipset vendor
 * @oem_target_type: Chip type
 * @oem_fw_version: Firmware version
 * @driver_version: Host software version
 * @allowed_dwell_time_min: Channel dwell time - allowed minimum
 * @allowed_dwell_time_max: Channel dwell time - allowed maximum
 * @curr_dwell_time_min: Channel dwell time - current minimim
 * @curr_dwell_time_max: Channel dwell time - current maximum
 * @supported_bands: Supported bands, 2.4G or 5G Hz
 * @num_channels: Num of channels IDs to follow
 * @channel_list: List of channel IDs
 */
struct qdf_packed wifi_pos_driver_caps {
	uint8_t oem_target_signature[OEM_TARGET_SIGNATURE_LEN];
	uint32_t oem_target_type;
	uint32_t oem_fw_version;
	struct wifi_pos_driver_version driver_version;
	uint16_t allowed_dwell_time_min;
	uint16_t allowed_dwell_time_max;
	uint16_t curr_dwell_time_min;
	uint16_t curr_dwell_time_max;
	uint16_t supported_bands;
	uint16_t num_channels;
	uint8_t channel_list[OEM_CAP_MAX_NUM_CHANNELS];
};

/**
 * struct wifi_pos_user_defined_caps - OEM capability to be exchanged between
 * host and userspace
 * @ftm_rr: FTM range report capability bit
 * @lci_capability: LCI capability bit
 * @reserved1: reserved
 * @reserved2: reserved
 */
struct wifi_pos_user_defined_caps {
	uint32_t ftm_rr:1;
	uint32_t lci_capability:1;
	uint32_t reserved1:30;
	uint32_t reserved2;
};

/**
 * struct wifi_pos_oem_get_cap_rsp - capabilities set by userspace and target.
 * @driver_cap: target capabilities
 * @user_defined_cap: capabilities set by userspace via set request
 */
struct qdf_packed wifi_pos_oem_get_cap_rsp {
	struct wifi_pos_driver_caps driver_cap;
	struct wifi_pos_user_defined_caps user_defined_cap;
};
#endif
