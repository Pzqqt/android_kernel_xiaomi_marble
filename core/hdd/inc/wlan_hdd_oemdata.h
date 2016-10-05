/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_oemdata.h
 *
 * Internal includes for the oem data
 */

#ifndef __WLAN_HDD_OEM_DATA_H__
#define __WLAN_HDD_OEM_DATA_H__

#ifdef FEATURE_OEM_DATA_SUPPORT

#ifndef OEM_DATA_REQ_SIZE
#define OEM_DATA_REQ_SIZE 500
#endif

#ifndef OEM_DATA_RSP_SIZE
#define OEM_DATA_RSP_SIZE 1724
#endif

#define OEM_APP_SIGNATURE_LEN      16
#define OEM_APP_SIGNATURE_STR      "QUALCOMM-OEM-APP"

#define OEM_TARGET_SIGNATURE_LEN   8
#define OEM_TARGET_SIGNATURE       "QUALCOMM"

#define OEM_CAP_MAX_NUM_CHANNELS   128

/**
 * typedef eOemErrorCode - OEM error codes
 * @OEM_ERR_NULL_CONTEXT: %NULL context
 * @OEM_ERR_APP_NOT_REGISTERED: OEM App is not registered
 * @OEM_ERR_INVALID_SIGNATURE: Invalid signature
 * @OEM_ERR_NULL_MESSAGE_HEADER: Invalid message header
 * @OEM_ERR_INVALID_MESSAGE_TYPE: Invalid message type
 * @OEM_ERR_INVALID_MESSAGE_LENGTH: Invalid length in message body
 */
typedef enum {
	OEM_ERR_NULL_CONTEXT = 1,
	OEM_ERR_APP_NOT_REGISTERED,
	OEM_ERR_INVALID_SIGNATURE,
	OEM_ERR_NULL_MESSAGE_HEADER,
	OEM_ERR_INVALID_MESSAGE_TYPE,
	OEM_ERR_INVALID_MESSAGE_LENGTH
} eOemErrorCode;

/**
 * typedef tDriverVersion - Driver version identifier (w.x.y.z)
 * @major: Version ID major number
 * @minor: Version ID minor number
 * @patch: Version ID patch number
 * @build: Version ID build number
 */
typedef struct qdf_packed {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint8_t build;
} tDriverVersion;

/**
 * typedef t_iw_oem_data_cap - OEM Data Capabilities
 * @oem_target_signature: Signature of chipset vendor, e.g. QUALCOMM
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
typedef struct qdf_packed {
	uint8_t oem_target_signature[OEM_TARGET_SIGNATURE_LEN];
	uint32_t oem_target_type;
	uint32_t oem_fw_version;
	tDriverVersion driver_version;
	uint16_t allowed_dwell_time_min;
	uint16_t allowed_dwell_time_max;
	uint16_t curr_dwell_time_min;
	uint16_t curr_dwell_time_max;
	uint16_t supported_bands;
	uint16_t num_channels;
	uint8_t channel_list[OEM_CAP_MAX_NUM_CHANNELS];
} t_iw_oem_data_cap;

/**
 * typedef tHddChannelInfo - Channel information
 * @chan_id: channel id
 * @reserved0: reserved for padding and future use
 * @mhz: primary 20 MHz channel frequency in mhz
 * @band_center_freq1: Center frequency 1 in MHz
 * @band_center_freq2: Center frequency 2 in MHz, valid only for 11ac
 *	VHT 80+80 mode
 * @info: channel info
 * @reg_info_1: regulatory information field 1 which contains min power,
 *	max power, reg power and reg class id
 * @reg_info_2: regulatory information field 2 which contains antennamax
 */
typedef struct qdf_packed {
	uint32_t chan_id;
	uint32_t reserved0;
	uint32_t mhz;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t info;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
} tHddChannelInfo;

/**
 * typedef tPeerStatusInfo - Status information for a given peer
 * @peer_mac_addr: peer mac address
 * @peer_status: peer status: 1: CONNECTED, 2: DISCONNECTED
 * @vdev_id: vdev_id for the peer mac
 * @peer_capability: peer capability: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @reserved0: reserved0
 * @peer_chan_info: channel info on which peer is connected
 */
typedef struct qdf_packed {
	uint8_t peer_mac_addr[ETH_ALEN];
	uint8_t peer_status;
	uint8_t vdev_id;
	uint32_t peer_capability;
	uint32_t reserved0;
	tHddChannelInfo peer_chan_info;
} tPeerStatusInfo;

/**
 * enum oem_capability_mask - mask field for userspace client capabilities
 * @OEM_CAP_RM_FTMRR: FTM range report mask bit
 * @OEM_CAP_RM_LCI: LCI capability mask bit
 */
enum oem_capability_mask {
	OEM_CAP_RM_FTMRR = (1 << (0)),
	OEM_CAP_RM_LCI = (1 << (1)),
};

/**
 * struct oem_get_capability_rsp - capabilites set by userspace and target.
 * @target_cap: target capabilities
 * @client_capabilities: capabilities set by userspace via set request
 */
struct oem_get_capability_rsp {
	t_iw_oem_data_cap target_cap;
	struct sme_oem_capability cap;
};

void hdd_send_peer_status_ind_to_oem_app(struct qdf_mac_addr *peerMac,
					 uint8_t peerStatus,
					 uint8_t peerTimingMeasCap,
					 uint8_t sessionId,
					 tSirSmeChanInfo * chan_info,
					 enum tQDF_ADAPTER_MODE dev_mode);

int iw_get_oem_data_cap(struct net_device *dev, struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra);

int oem_activate_service(struct hdd_context_s *hdd_ctx);

void hdd_send_oem_data_rsp_msg(struct oem_data_rsp *oem_rsp);
#else
static inline int oem_activate_service(struct hdd_context_s *hdd_ctx)
{
	return 0;
}
#endif /* FEATURE_OEM_DATA_SUPPORT */
#endif /* __WLAN_HDD_OEM_DATA_H__ */
