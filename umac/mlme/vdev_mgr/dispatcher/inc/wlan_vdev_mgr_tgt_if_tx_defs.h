/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_vdev_mgr_tgt_if_tx_defs.h
 *
 * This file provides definitions to data structures required for vdev Tx ops
 */

#ifndef __WLAN_VDEV_MGR_TX_OPS_DEFS_H__
#define __WLAN_VDEV_MGR_TX_OPS_DEFS_H__

#include <qdf_nbuf.h>

/**
 * struct mac_ssid - mac ssid structure
 * @length: ssid length
 * @mac_ssid: ssid
 */
struct mlme_mac_ssid {
	uint8_t length;
	uint8_t mac_ssid[WLAN_SSID_MAX_LEN];
} qdf_packed;

/** slot time long */
#define WLAN_MLME_VDEV_SLOT_TIME_LONG   0x1
/** slot time short */
#define WLAN_MLME_VDEV_SLOT_TIME_SHORT  0x2

/**
 * enum MLME_bcn_tx_rate_code - beacon tx rate code
 */
enum mlme_bcn_tx_rate_code {
	MLME_BCN_TX_RATE_CODE_1_M = 0x43,
	MLME_BCN_TX_RATE_CODE_2_M = 0x42,
	MLME_BCN_TX_RATE_CODE_5_5_M = 0x41,
	MLME_BCN_TX_RATE_CODE_6_M = 0x03,
	MLME_BCN_TX_RATE_CODE_9_M = 0x07,
	MLME_BCN_TX_RATE_CODE_11M = 0x40,
	MLME_BCN_TX_RATE_CODE_12_M = 0x02,
	MLME_BCN_TX_RATE_CODE_18_M = 0x06,
	MLME_BCN_TX_RATE_CODE_24_M = 0x01,
	MLME_BCN_TX_RATE_CODE_36_M = 0x05,
	MLME_BCN_TX_RATE_CODE_48_M = 0x00,
	MLME_BCN_TX_RATE_CODE_54_M = 0x04,
};

/**
 * enum wlan_mlme_host_sta_ps_param_uapsd - STA UPASD params
 */
enum wlan_mlme_host_sta_ps_param_uapsd {
	WLAN_MLME_HOST_STA_PS_UAPSD_AC0_DELIVERY_EN = (1 << 0),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC0_TRIGGER_EN  = (1 << 1),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC1_DELIVERY_EN = (1 << 2),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC1_TRIGGER_EN  = (1 << 3),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC2_DELIVERY_EN = (1 << 4),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC2_TRIGGER_EN  = (1 << 5),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC3_DELIVERY_EN = (1 << 6),
	WLAN_MLME_HOST_STA_PS_UAPSD_AC3_TRIGGER_EN  = (1 << 7),
};

/**
 * enum wlan_mlme_host_vdev_start_status - vdev start status code
 */
enum wlan_mlme_host_vdev_start_status {
	WLAN_MLME_HOST_VDEV_START_OK = 0,
	WLAN_MLME_HOST_VDEV_START_CHAN_INVALID,
	WLAN_MLME_HOST_VDEV_START_CHAN_BLOCKED,
	WLAN_MLME_HOST_VDEV_START_CHAN_DFS_VIOLATION,
	WLAN_MLME_HOST_VDEV_START_CHAN_INVALID_REGDOMAIN,
	WLAN_MLME_HOST_VDEV_START_CHAN_INVALID_BAND,
	WLAN_MLME_HOST_VDEV_START_TIMEOUT,
	/* Add new response status code from here */
	WLAN_MLME_HOST_VDEV_START_MAX_REASON,
};

/**
 * string_from_start_rsp_status() - Convert start response status to string
 * @start_rsp - start response status
 *
 * Please note to add new string in the array at index equal to
 * its enum value in wlan_mlme_host_vdev_start_status.
 */
static inline char *string_from_start_rsp_status(
			enum wlan_mlme_host_vdev_start_status start_rsp)
{
	static const char *strings[] = { "START_OK",
					"CHAN_INVALID",
					"CHAN_BLOCKED",
					"CHAN_DFS_VIOLATION",
					"CHAN_INVALID_REGDOMAIN",
					"CHAN_INVALID_BAND",
					"START_RESPONSE_TIMEOUT",
					"START_RESPONSE_UNKNOWN"};

	if (start_rsp >= WLAN_MLME_HOST_VDEV_START_MAX_REASON)
		start_rsp = WLAN_MLME_HOST_VDEV_START_MAX_REASON;

	return (char *)strings[start_rsp];
}

/**
 * enum wlan_mlme_host_start_event_param - start/restart resp event
 */
enum wlan_mlme_host_start_event_param {
	WLAN_MLME_HOST_VDEV_START_RESP_EVENT = 0,
	WLAN_MLME_HOST_VDEV_RESTART_RESP_EVENT,
};

/**
 * enum wlan_mlme_custom_aggr_type: custon aggregate type
 * @WLAN_MLME_CUSTOM_AGGR_TYPE_AMPDU: A-MPDU aggregation
 * @WLAN_MLME_CUSTOM_AGGR_TYPE_AMSDU: A-MSDU aggregation
 * @WLAN_MLME_CUSTOM_AGGR_TYPE_MAX: Max type
 */
enum wlan_mlme_custom_aggr_type {
	WLAN_MLME_CUSTOM_AGGR_TYPE_AMPDU = 0,
	WLAN_MLME_CUSTOM_AGGR_TYPE_AMSDU = 1,
	WLAN_MLME_CUSTOM_AGGR_TYPE_MAX,
};

/**
 * struct sta_ps_params - sta ps cmd parameter
 * @vdev_id: vdev id
 * @param_id: sta ps parameter
 * @value: sta ps parameter value
 */
struct sta_ps_params {
	uint32_t vdev_id;
	uint32_t param_id;
	uint32_t value;
};

/**
 * struct tbttoffset_params - Tbttoffset event params
 * @vdev_id: Virtual AP device identifier
 * @tbttoffset : Tbttoffset for the virtual AP device
 * @vdev_tbtt_qtime_lo: Tbtt qtime low value
 * @vdev_tbtt_qtime_hi: Tbtt qtime high value
 */
struct tbttoffset_params {
	uint32_t vdev_id;
	uint32_t tbttoffset;
	uint32_t vdev_tbtt_qtime_lo;
	uint32_t vdev_tbtt_qtime_hi;
};

/**
 * struct beacon_tmpl_params - beacon template cmd parameter
 * @vdev_id: vdev id
 * @tim_ie_offset: tim ie offset
 * @mbssid_ie_offset: mbssid ie offset
 * @tmpl_len: beacon template length
 * @tmpl_len_aligned: beacon template alignment
 * @csa_switch_count_offset: CSA swith count offset in beacon frame
 * @ext_csa_switch_count_offset: ECSA switch count offset in beacon frame
 * @esp_ie_offset: ESP IE offset in beacon frame
 * @mu_edca_ie_offset: Mu EDCA IE offset in beacon frame
 * @frm: beacon template parameter
 */
struct beacon_tmpl_params {
	uint8_t vdev_id;
	uint32_t tim_ie_offset;
	uint32_t mbssid_ie_offset;
	uint32_t tmpl_len;
	uint32_t tmpl_len_aligned;
	uint32_t csa_switch_count_offset;
	uint32_t ext_csa_switch_count_offset;
	uint32_t esp_ie_offset;
	uint32_t mu_edca_ie_offset;
	uint8_t *frm;
};

/**
 * struct beacon_params - beacon cmd parameter
 * @vdev_id: vdev id
 * @beacon_interval: Beacon interval
 * @wbuf: beacon buffer
 * @frame_ctrl: frame control field
 * @bcn_txant: beacon antenna
 * @is_dtim_count_zero: is it dtim beacon
 * @is_bitctl_reqd: is Bit control required
 * @is_high_latency: Is this high latency target
 */
struct beacon_params {
	uint8_t vdev_id;
	uint16_t beacon_interval;
	qdf_nbuf_t wbuf;
	uint16_t frame_ctrl;
	uint32_t bcn_txant;
	bool is_dtim_count_zero;
	bool is_bitctl_reqd;
	bool is_high_latency;
};

/* struct fils_discovery_tmpl_params - FILS Discovery template cmd parameter
 * @vdev_id: vdev ID
 * @tmpl_len: FILS Discovery template length
 * @tmpl_aligned: FILS Discovery template alignment
 * @frm: FILS Discovery template parameter
 */
struct fils_discovery_tmpl_params {
	uint8_t vdev_id;
	uint32_t tmpl_len;
	uint32_t tmpl_len_aligned;
	uint8_t *frm;
};

/**
 * struct mlme_channel_param - Channel parameters with all
 *			info required by target.
 * @chan_id: channel id
 * @pwr: channel power
 * @mhz: channel frequency
 * @half_rate: is half rate
 * @quarter_rate: is quarter rate
 * @dfs_set: is dfs channel
 * @dfs_set_cfreq2: is secondary freq dfs channel
 * @is_chan_passive: is this passive channel
 * @allow_ht: HT allowed in chan
 * @allow_vht: VHT allowed on chan
 * @set_agile: is agile mode
 * @phy_mode: phymode (vht80 or ht40 or ...)
 * @cfreq1: centre frequency on primary
 * @cfreq2: centre frequency on secondary
 * @maxpower: max power for channel
 * @minpower: min power for channel
 * @maxreqpower: Max regulatory power
 * @antennamac: Max antenna
 * @reg_class_id: Regulatory class id.
 */
struct mlme_channel_param {
	uint8_t chan_id;
	uint8_t pwr;
	uint32_t mhz;
	uint32_t half_rate:1,
		quarter_rate:1,
		dfs_set:1,
		dfs_set_cfreq2:1,
		is_chan_passive:1,
		allow_ht:1,
		allow_vht:1,
		set_agile:1;
	enum wlan_phymode phy_mode;
	uint32_t cfreq1;
	uint32_t cfreq2;
	int8_t   maxpower;
	int8_t   minpower;
	int8_t   maxregpower;
	uint8_t  antennamax;
	uint8_t  reg_class_id;
};

/**
 * struct multiple_vdev_restart_params - Multiple vdev restart cmd parameter
 * @pdev_id: Pdev identifier
 * @requestor_id: Unique id identifying the module
 * @disable_hw_ack: Flag to indicate disabling HW ACK during CAC
 * @cac_duration_ms: CAC duration on the given channel
 * @num_vdevs: No. of vdevs that need to be restarted
 * @ch_param: Pointer to channel_param
 * @vdev_ids: Pointer to array of vdev_ids
 */
struct multiple_vdev_restart_params {
	uint32_t pdev_id;
	uint32_t requestor_id;
	uint32_t disable_hw_ack;
	uint32_t cac_duration_ms;
	uint32_t num_vdevs;
	struct mlme_channel_param ch_param;
	uint32_t vdev_ids[WLAN_UMAC_PDEV_MAX_VDEVS];
};

/**
 * struct peer_flush_params - peer flush cmd parameter
 * @peer_tid_bitmap: peer tid bitmap
 * @vdev_id: vdev id
 * @peer_mac: peer mac address
 */
struct peer_flush_params {
	uint32_t peer_tid_bitmap;
	uint8_t vdev_id;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
};

/* Default FILS DISCOVERY/probe response sent in period of 20TU */
#define DEFAULT_FILS_DISCOVERY_PERIOD 20
#define DEFAULT_PROBE_RESP_PERIOD 20

/**
 * struct config_fils_params - FILS config params
 * @vdev_id:  vdev id
 * @fd_period:  0 - Disabled, non-zero - Period in ms (mili seconds)
 * @send_prb_rsp_frame: send broadcast prb resp frame
 */
struct config_fils_params {
	uint8_t vdev_id;
	uint32_t fd_period;
	uint32_t send_prb_rsp_frame: 1;
};

/**
 * struct config_ratemask_params - ratemask config parameters
 * @vdev_id: vdev id
 * @type: Type
 * @lower32: Lower 32 bits in the 1st 64-bit value
 * @higher32: Higher 32 bits in the 1st 64-bit value
 * @lower32_2: Lower 32 bits in the 2nd 64-bit value
 */
struct config_ratemask_params {
	uint8_t vdev_id;
	uint8_t type;
	uint32_t lower32;
	uint32_t higher32;
	uint32_t lower32_2;
};

/**
 * struct set_custom_aggr_size_params - custom aggr size params
 * @vdev_id : vdev id
 * @tx_aggr_size : TX aggr size
 * @rx_aggr_size : RX aggr size
 * @enable_bitmap: Bitmap for aggr size check
 */
struct set_custom_aggr_size_params {
	uint32_t  vdev_id;
	uint32_t tx_aggr_size;
	uint32_t rx_aggr_size;
	uint32_t ac:2,
		 aggr_type:1,
		 tx_aggr_size_disable:1,
		 rx_aggr_size_disable:1,
		 tx_ac_enable:1,
		 reserved:26;
};

/**
 * struct sifs_trigger_param - sifs_trigger cmd parameter
 * @vdev_id: vdev id
 * @param_value: parameter value
 */
struct sifs_trigger_param {
	uint32_t vdev_id;
	uint32_t param_value;
};

/**
 * struct set_neighbour_rx_params - Neighbour RX params
 * @vdev_id: vdev id
 * @idx: index of param
 * @action: action
 * @type: Type of param
 */
struct set_neighbour_rx_params {
	uint8_t vdev_id;
	uint32_t idx;
	uint32_t action;
	uint32_t type;
};

/**
 * struct vdev_scan_nac_rssi_params - NAC_RSSI cmd parameter
 * @vdev_id: vdev id
 * @bssid_addr: BSSID address
 * @client_addr: client address
 * @chan_num: channel number
 * @action:NAC_RSSI action,
 */
struct vdev_scan_nac_rssi_params {
	uint32_t vdev_id;
	uint8_t bssid_addr[QDF_MAC_ADDR_SIZE];
	uint8_t client_addr[QDF_MAC_ADDR_SIZE];
	uint32_t chan_num;
	uint32_t action; /* WMI_FILTER_NAC_RSSI_ACTION */
};

/**
 * struct vdev_start_params - vdev start cmd parameter
 * @vdev_id: vdev id
 * @beacon_interval: beacon interval
 * @dtim_period: dtim period
 * @is_restart: flag to check if it is vdev
 * @disable_hw_ack: to update disable hw ack flag
 * @hidden_ssid: hidden ssid
 * @pmf_enabled: pmf enabled
 * @ssid: ssid MAC
 * @num_noa_descriptors: number of noa descriptors
 * @preferred_tx_streams: preferred tx streams
 * @preferred_rx_streams: preferred rx streams
 * @cac_duration_ms: cac duration in milliseconds
 * @regdomain: Regulatory domain
 * @he_ops: HE ops
 * @channel_param: Channel params required by target.
 * @bcn_tx_rate_code: Beacon tx rate code.
 * @ldpc_rx_enabled: Enable/Disable LDPC RX for this vdev
 */
struct vdev_start_params {
	uint8_t vdev_id;
	uint32_t beacon_interval;
	uint32_t dtim_period;
	bool is_restart;
	uint32_t disable_hw_ack;
	bool hidden_ssid;
	bool pmf_enabled;
	struct mlme_mac_ssid ssid;
	uint32_t num_noa_descriptors;
	uint32_t preferred_rx_streams;
	uint32_t preferred_tx_streams;
	uint32_t cac_duration_ms;
	uint32_t regdomain;
	uint32_t he_ops;
	struct mlme_channel_param channel;
	enum mlme_bcn_tx_rate_code bcn_tx_rate_code;
	bool ldpc_rx_enabled;
};

/**
 * struct vdev_set_params - vdev set cmd parameter
 * @vdev_id: vdev id
 * @param_id: parameter id
 * @param_value: parameter value
 */
struct vdev_set_params {
	uint32_t vdev_id;
	uint32_t param_id;
	uint32_t param_value;
};

/**
 * struct vdev_create_params - vdev create cmd parameter
 * @vdev_id: interface id
 * @type: interface type
 * @subtype: interface subtype
 * @nss_2g: NSS for 2G
 * @nss_5g: NSS for 5G
 * @pdev_id: pdev id on pdev for this vdev
 * @mbssid_flags: MBSS IE flags indicating vdev type
 * @vdevid_trans: id of transmitting vdev for MBSS IE
 */
struct vdev_create_params {
	uint8_t vdev_id;
	uint32_t type;
	uint32_t subtype;
	uint8_t nss_2g;
	uint8_t nss_5g;
	uint32_t pdev_id;
	uint32_t mbssid_flags;
	uint8_t vdevid_trans;
};

/**
 * struct vdev_delete_params - vdev delete cmd parameter
 * @vdev_id: vdev id
 */
struct vdev_delete_params {
	uint8_t vdev_id;
};

/**
 * struct vdev_stop_params - vdev stop cmd parameter
 * @vdev_id: vdev id
 */
struct vdev_stop_params {
	uint8_t vdev_id;
};

/**
 * struct vdev_up_params - vdev up cmd parameter
 * @vdev_id: vdev id
 * @assoc_id: association id
 * @profile_idx: profile index of the connected non-trans ap (mbssid case).
 *		0  means invalid.
 * @profile_num: the total profile numbers of non-trans aps (mbssid case).
 *		0 means non-MBSS AP.
 * @trans_bssid: bssid of transmitted AP (MBSS IE case)
 */
struct vdev_up_params {
	uint8_t vdev_id;
	uint16_t assoc_id;
	uint32_t profile_idx;
	uint32_t profile_num;
	uint8_t trans_bssid[QDF_MAC_ADDR_SIZE];
};

/**
 * struct vdev_down_params - vdev down cmd parameter
 * @vdev_id: vdev id
 */
struct vdev_down_params {
	uint8_t vdev_id;
};

/**
 * struct peer_delete_all_params - peer delete all request parameter
 * @vdev_id: vdev id
 */
struct peer_delete_all_params {
	uint8_t vdev_id;
};

#endif /* __WLAN_VDEV_MGR_TX_OPS_DEFS_H__ */
