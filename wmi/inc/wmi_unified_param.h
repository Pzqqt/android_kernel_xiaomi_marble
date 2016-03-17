/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_PARAM_H_
#define _WMI_UNIFIED_PARAM_H_
#include "wmi_unified.h"
#define IEEE80211_ADDR_LEN  6  /* size of 802.11 address */
#define WMI_MAC_MAX_SSID_LENGTH              32
#define WMI_SCAN_MAX_NUM_SSID                0x0A
#define mgmt_tx_dl_frm_len 64
#define WMI_SMPS_MASK_LOWER_16BITS 0xFF
#define WMI_SMPS_MASK_UPPER_3BITS 0x7
#define WMI_SMPS_PARAM_VALUE_S 29
/* The size of the utc time in bytes. */
#define WMI_SIZE_UTC_TIME (10)
/* The size of the utc time error in bytes. */
#define WMI_SIZE_UTC_TIME_ERROR (5)
#define WMI_MCC_MIN_CHANNEL_QUOTA             20
#define WMI_MCC_MAX_CHANNEL_QUOTA             80
#define WMI_MCC_MIN_NON_ZERO_CHANNEL_LATENCY  30
#define WMI_BEACON_TX_BUFFER_SIZE             (512)
#define WMI_WIFI_SCANNING_MAC_OUI_LENGTH      3
#define WMI_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS   64
#define WMI_RSSI_THOLD_DEFAULT   -300
#define WMI_NLO_FREQ_THRESH          1000
#define WMI_SEC_TO_MSEC(sec)         (sec * 1000)
#define WMI_MSEC_TO_USEC(msec)	     (msec * 1000)
#define WMI_ETH_LEN      64
#define WMI_QOS_NUM_TSPEC_MAX 2
#define WMI_QOS_NUM_AC_MAX 4
/**
 * struct vdev_create_params - vdev create cmd parameter
 * @if_id: interface id
 * @type: interface type
 * @subtype: interface subtype
 */
struct vdev_create_params {
	uint8_t if_id;
	uint32_t type;
	uint32_t subtype;
};

/**
 * struct vdev_delete_params - vdev delete cmd parameter
 * @if_id: interface id
 */
struct vdev_delete_params {
	uint8_t if_id;
};

/**
 * struct vdev_start_params - vdev start cmd parameter
 * @beacon_intval: beacon intval
 * @dtim_period: dtim period
 * @max_txpow: max tx power
 * @phy_ch_width chan_width: channel width
 * @is_dfs: flag to check if dfs enabled
 * @vdev_id: vdev id
 * @chan: channel no
 * @oper_mode: operating mode
 * @length: length
 * @ssId[32]: ssid
 * @hidden_ssid: hidden ssid
 * @pmf_enabled: is pmf enabled
 * @vht_capable: is vht capable
 * @ch_center_freq_seg0: center freq seq 0
 * @ch_center_freq_seg1: center freq seq 1
 * @ht_capable: is ht capable
 * @dfs_pri_multiplier: DFS multiplier
 * @dot11_mode: dot11 mode
 * @is_half_rate: Indicates half rate channel
 * @is_quarter_rate: Indicates quarter rate channel
 * @preferred_tx_streams: preferred tx streams
 * @preferred_rx_streams: preferred rx streams
 */
struct vdev_start_params {
	uint32_t beacon_intval;
	uint32_t dtim_period;
	int32_t max_txpow;
	bool is_dfs;
	uint8_t vdev_id;
	uint8_t chan;
	uint8_t oper_mode;
	uint8_t length;
	uint8_t ssId[32];
	uint8_t hidden_ssid;
	uint8_t pmf_enabled;
	uint8_t vht_capable;
	uint8_t ch_center_freq_seg0;
	uint8_t ch_center_freq_seg1;
	uint8_t ht_capable;
	int32_t dfs_pri_multiplier;
	uint8_t dot11_mode;
	bool is_half_rate;
	bool is_quarter_rate;
	uint32_t preferred_tx_streams;
	uint32_t preferred_rx_streams;
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
 */
struct vdev_up_params {
	uint8_t vdev_id;
	uint16_t assoc_id;
};

/**
 * struct vdev_down_params - vdev down cmd parameter
 * @vdev_id: vdev id
 */
struct vdev_down_params {
	uint8_t vdev_id;
};

/**
 * struct vdev_set_params - vdev set cmd parameter
 * @if_id: vdev id
 * @param_id: parameter id
 * @param_value: parameter value
 */
struct vdev_set_params {
	uint32_t if_id;
	uint32_t param_id;
	uint32_t param_value;
};

/**
 * struct peer_delete_params - peer delete cmd parameter
 * @vdev_id: vdev id
 */
struct peer_delete_params {
	uint8_t vdev_id;
};

/**
 * struct peer_flush_params - peer flush cmd parameter
 * @peer_tid_bitmap: peer tid bitmap
 * @vdev_id: vdev id
 */
struct peer_flush_params {
	uint32_t peer_tid_bitmap;
	uint8_t vdev_id;
};

/**
 * struct peer_set_params - peer set cmd parameter
 * @param_id: parameter id
 * @param_value: parameter value
 * @vdev_id: vdev id
 */
struct peer_set_params {
	uint32_t param_id;
	uint32_t param_value;
	uint32_t vdev_id;
};

/**
 * struct peer_create_params - peer create cmd parameter
 * @peer_addr: peer mac addr
 * @peer_type: peer type
 * @vdev_id: vdev id
 */
struct peer_create_params {
	const uint8_t *peer_addr;
	uint32_t peer_type;
	uint32_t vdev_id;
};

/**
 * struct peer_remove_params - peer remove cmd parameter
 * @bssid: bss id
 * @vdev_id: vdev id
 * @roam_synch_in_progress: flag to indicate if roaming is in progress
 */
struct peer_remove_params {
	uint8_t *bssid;
	uint8_t vdev_id;
	bool roam_synch_in_progress;
};

/**
 * struct stats_request_params - stats_request cmd parameter
 * @stats_id: statistics id
 * @vdev_id: vdev id
 */
struct stats_request_params {
	uint32_t stats_id;
	uint32_t vdev_id;
};

/**
 * struct green_ap_ps_params - green ap ps cmd parameter
 * @value: parameter value
 */
struct green_ap_ps_params {
	uint32_t value;
};

/**
 * struct wow_cmd_params - wow cmd parameter
 * @enable: wow enable or disable flag
 * @can_suspend_link: flag to indicate if link can be suspended
 * @pause_iface_config: interface config
 */
struct wow_cmd_params {
	bool enable;
	bool can_suspend_link;
	uint8_t pause_iface_config;
};

/**
 * struct packet_enable_params - packet enable cmd parameter
 * @vdev_id: vdev id
 * @enable: flag to indicate if parameter can be enabled or disabled
 */
struct packet_enable_params {
	uint8_t vdev_id;
	bool enable;
};

/**
 * struct suspend_params - suspend cmd parameter
 * @disable_target_intr: disable target interrupt
 */
struct suspend_params {
	uint8_t disable_target_intr;
};

/**
 * struct pdev_params - pdev set cmd parameter
 * @param_id: parameter id
 * @param_value: parameter value
 */
struct pdev_params {
	uint32_t param_id;
	uint32_t param_value;
};

/**
 * struct beacon_params - beacon template cmd parameter
 * @vdev_id: vdev id
 * @tim_ie_offset: tim ie offset
 * @tmpl_len: beacon template length
 * @tmpl_len_aligned: beacon template alignment
 * @frm: beacon template parameter
 */
struct beacon_params {
	uint8_t vdev_id;
	uint32_t tim_ie_offset;
	uint32_t tmpl_len;
	uint32_t tmpl_len_aligned;
	uint8_t *frm;
};

/**
 * struct peer_assoc_params - peer assoc cmd parameter
 * @peer_macaddr: peer mac address
 * @vdev_id: vdev id
 * @peer_new_assoc: peer association type
 * @peer_associd: peer association id
 * @peer_flags: peer flags
 * @peer_caps: peer capabalities
 * @peer_listen_intval: peer listen interval
 * @peer_ht_caps: HT capabalities
 * @peer_max_mpdu: 0 : 8k , 1 : 16k, 2 : 32k, 3 : 64k
 * @peer_mpdu_density: 3 : 0~7 : 2^(11nAMPDUdensity -4)
 * @peer_rate_caps: peer rate capabalities
 * @peer_nss: peer nss
 * @peer_phymode: peer phymode
 * @peer_ht_info: peer HT info
 * @peer_legacy_rates: peer legacy rates
 * @peer_ht_rates: peer ht rates
 * @num_peer_legacy_rates: no of peer legacy rates
 * @num_peer_ht_rates: no of peer ht rates
 * @rx_max_rate: max rx rates
 * @rx_mcs_set: rx mcs
 * @tx_max_rate: max tx rates
 * @tx_mcs_set: tx mcs
 * @vht_capable: VHT capabalities
 */
struct peer_assoc_params {
	wmi_mac_addr peer_macaddr;
	uint32_t vdev_id;
	uint32_t peer_new_assoc;
	uint32_t peer_associd;
	uint32_t peer_flags;
	uint32_t peer_caps;
	uint32_t peer_listen_intval;
	uint32_t peer_ht_caps;
	uint32_t peer_max_mpdu;
	uint32_t peer_mpdu_density;
	uint32_t peer_rate_caps;
	uint32_t peer_nss;
	uint32_t peer_vht_caps;
	uint32_t peer_phymode;
	uint32_t peer_ht_info[2];
	wmi_rate_set peer_legacy_rates;
	wmi_rate_set peer_ht_rates;
	uint32_t num_peer_legacy_rates;
	uint32_t num_peer_ht_rates;
	uint32_t rx_max_rate;
	uint32_t rx_mcs_set;
	uint32_t tx_max_rate;
	uint32_t tx_mcs_set;
	uint8_t vht_capable;
};

/**
 * struct sta_ps_params - sta ps cmd parameter
 * @vdev_id: vdev id
 * @param: sta ps paramter
 * @value: sta ps parameter value
 */
struct sta_ps_params {
	uint32_t vdev_id;
	uint32_t param;
	uint32_t value;
};

/**
 * struct ap_ps_params - ap ps cmd parameter
 * @vdev_id: vdev id
 * @param: ap ps paramter
 * @value: ap ps paramter value
 */
struct ap_ps_params {
	uint32_t vdev_id;
	uint32_t param;
	uint32_t value;
};

/**
 * struct mac_ssid - mac ssid structure
 * @length:
 * @mac_ssid[WMI_MAC_MAX_SSID_LENGTH]:
 */
struct mac_ssid {
	uint8_t length;
	uint8_t mac_ssid[WMI_MAC_MAX_SSID_LENGTH];
} cdf_packed;

/**
 * struct scan_start_params - start scan cmd parameter
 * @scan_id: scan id
 * @scan_req_id: requeted scan id
 * @vdev_id: vdev id
 * @scan_priority: scan priority
 * @notify_scan_events: flag to indicate if scan to be notified
 * @dwell_time_active: active dwell time
 * @dwell_time_passive: passive dwell time
 * @min_rest_time: min rest time
 * @max_rest_time: max rest time
 * @repeat_probe_time: repeat probe time
 * @probe_spacing_time: probe spacing time
 * @idle_time: idle time
 * @max_scan_time: max scan time
 * @probe_delay: probe delay
 * @scan_ctrl_flags: scan control flag
 * @burst_duration: burst duration
 * @num_chan: no of channel
 * @num_bssid: no of bssid
 * @num_ssids: no of ssid
 * @ie_len: ie length
 * @n_probes: no of probe
 * @chan_list: channel list
 * @ie_len_with_pad: ie length with padding
 * @num_ssid: no of ssid
 * @sid: pointer to mac_ssid structure
 * @uie_fieldOffset: ie field offset
 * @mac_add_bytes: mac address bytes
 */
struct scan_start_params {
	uint32_t scan_id;
	uint32_t scan_req_id;
	uint32_t vdev_id;
	uint32_t scan_priority;
	uint32_t notify_scan_events;
	uint32_t dwell_time_active;
	uint32_t dwell_time_passive;
	uint32_t min_rest_time;
	uint32_t max_rest_time;
	uint32_t repeat_probe_time;
	uint32_t probe_spacing_time;
	uint32_t idle_time;
	uint32_t max_scan_time;
	uint32_t probe_delay;
	uint32_t scan_ctrl_flags;
	uint32_t burst_duration;
	uint32_t num_chan;
	uint32_t num_bssid;
	uint32_t num_ssids;
	uint32_t ie_len;
	uint32_t n_probes;
	uint32_t *chan_list;
	uint32_t ie_len_with_pad;
	struct mac_ssid ssid[WMI_SCAN_MAX_NUM_SSID];
	uint8_t  *ie_base;
	uint16_t uie_fieldOffset;
	uint8_t  mac_add_bytes[IEEE80211_ADDR_LEN];
};

/**
 * struct scan_stop_params - stop scan cmd parameter
 * @requestor: scan requestor
 * @scan_id: scan id
 * @req_type: scan request type
 * @vdev_id: vdev id
 */
struct scan_stop_params {
	uint32_t requestor;
	uint32_t scan_id;
	uint32_t req_type;
	uint32_t vdev_id;
};

/**
 * struct scan_chan_list_params  - scan channel list cmd parameter
 * @num_scan_chans: no of scan channels
 * @chan_info: pointer to wmi channel info
 */
struct scan_chan_list_params {
	uint8_t num_scan_chans;
	wmi_channel *chan_info;
};

/**
 * struct fw_hang_params - fw hang command parameters
 * @type: 0:unused 1: ASSERT, 2:not respond detect command, 3:simulate ep-full
 * @delay_time_ms: 0xffffffff means the simulate will delay for random time (0 ~0xffffffff ms)
 */
struct fw_hang_params {
	uint32_t type;
	uint32_t delay_time_ms;
};

/**
 * struct pdev_utf_params - pdev utf command parameters
 * @utf_payload:
 * @len:
 */
struct pdev_utf_params {
	uint8_t *utf_payload;
	uint32_t len;
};

/**
 * struct crash_inject - crash inject command parameters
 * @type: crash inject type
 * @delay_time_ms: time in milliseconds for FW to delay the crash
 */
struct crash_inject {
	uint32_t type;
	uint32_t delay_time_ms;
};

/**
 * struct dbglog_params - fw deboglog command parameters
 * @param: command parameter
 * @val: parameter value
 * @module_id_bitmap: fixed length module id bitmap
 * @bitmap_len: module id bitmap length
 */
struct dbglog_params {
	uint32_t param;
	uint32_t val;
	uint32_t *module_id_bitmap;
	uint32_t bitmap_len;
};

/**
 * struct seg_hdr_info - header info
 * @len: length
 * @msgref: message refrence
 * @segmentInfo: segment info
 * @pad: padding
 */
struct seg_hdr_info {
	uint32_t len;
	uint32_t msgref;
	uint32_t segmentInfo;
	uint32_t pad;
};

/**
 * struct wmi_mgmt_params - wmi mgmt cmd paramters
 * @tx_frame: management tx frame
 * @frmLen: frame length
 * @vdev_id: vdev id
 * @tx_complete_cb: tx download callback handler
 * @tx_ota_post_proc_cb: OTA complition handler
 * @chanfreq: channel frequency
 * @pdata: frame data
 * @wmi_desc: command descriptor
 */
struct wmi_mgmt_params {
	void *tx_frame;
	uint16_t frm_len;
	uint8_t vdev_id;
	void *tx_complete_cb;
	void  *tx_ota_post_proc_cb;
	uint16_t chanfreq;
	void *pdata;
	struct wmi_desc_t *wmi_desc;
	void *cdf_ctx;
};

/**
 * struct p2p_ps_params - P2P powersave related params
 * @opp_ps: opportunistic power save
 * @ctwindow: CT window
 * @count: count
 * @duration: duration
 * @interval: interval
 * @single_noa_duration: single shot noa duration
 * @ps_selection: power save selection
 * @session_id: session id
 */
struct p2p_ps_params {
	uint8_t opp_ps;
	uint32_t ctwindow;
	uint8_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint8_t ps_selection;
	uint8_t session_id;
};


/**
 * struct ta_uapsd_trig_params - uapsd trigger parameter
 * @vdevid: vdev id
 * @peer_addr: peer address
 * @auto_triggerparam: trigger parameters
 * @num_ac: no of access category
 */
struct sta_uapsd_trig_params {
		uint32_t vdevid;
		uint8_t peer_addr[IEEE80211_ADDR_LEN];
		uint8_t *auto_triggerparam;
		uint32_t num_ac;
};

/**
 * struct ocb_utc_param
 * @vdev_id: session id
 * @utc_time: number of nanoseconds from Jan 1st 1958
 * @time_error: the error in the UTC time. All 1's for unknown
 */
struct ocb_utc_param {
	uint32_t vdev_id;
	uint8_t utc_time[WMI_SIZE_UTC_TIME];
	uint8_t time_error[WMI_SIZE_UTC_TIME_ERROR];
};

/**
 * struct ocb_timing_advert_param
 * @vdev_id: session id
 * @chan_freq: frequency on which to advertise
 * @repeat_rate: the number of times it will send TA in 5 seconds
 * @timestamp_offset: offset of the timestamp field in the TA frame
 * @time_value_offset: offset of the time_value field in the TA frame
 * @template_length: size in bytes of the TA frame
 * @template_value: the TA frame
 */
struct ocb_timing_advert_param {
	uint32_t vdev_id;
	uint32_t chan_freq;
	uint32_t repeat_rate;
	uint32_t timestamp_offset;
	uint32_t time_value_offset;
	uint32_t template_length;
	uint8_t *template_value;
};

/**
 * struct dcc_get_stats_param
 * @vdev_id: session id
 * @channel_count: number of dcc channels
 * @request_array_len: size in bytes of the request array
 * @request_array: the request array
 */
struct dcc_get_stats_param {
	uint32_t vdev_id;
	uint32_t channel_count;
	uint32_t request_array_len;
	void *request_array;
};

/**
 * struct dcc_update_ndl_param
 * @vdev_id: session id
 * @channel_count: number of channels to be updated
 * @dcc_ndl_chan_list_len: size in bytes of the ndl_chan array
 * @dcc_ndl_chan_list: the ndl_chan array
 * @dcc_ndl_active_state_list_len: size in bytes of the active_state array
 * @dcc_ndl_active_state_list: the active state array
 */
struct dcc_update_ndl_param {
	uint32_t vdev_id;
	uint32_t channel_count;
	uint32_t dcc_ndl_chan_list_len;
	void *dcc_ndl_chan_list;
	uint32_t dcc_ndl_active_state_list_len;
	void *dcc_ndl_active_state_list;
};

/**
 * struct ocb_config_sched
 * @chan_freq: frequency of the channel
 * @total_duration: duration of the schedule
 * @guard_interval: guard interval on the start of the schedule
 */
struct ocb_config_sched {
	uint32_t chan_freq;
	uint32_t total_duration;
	uint32_t guard_interval;
};

/**
 * OCB structures
 */

#define WMI_NUM_AC			(4)
#define WMI_OCB_CHANNEL_MAX	(5)
#define WMI_MAX_NUM_AC 4
struct wmi_ocb_qos_params {
	uint8_t aifsn;
	uint8_t cwmin;
	uint8_t cwmax;
};
/**
 * struct ocb_config_channel
 * @chan_freq: frequency of the channel
 * @bandwidth: bandwidth of the channel, either 10 or 20 MHz
 * @mac_address: MAC address assigned to this channel
 * @qos_params: QoS parameters
 * @max_pwr: maximum transmit power of the channel (dBm)
 * @min_pwr: minimum transmit power of the channel (dBm)
 * @reg_pwr: maximum transmit power specified by the regulatory domain (dBm)
 * @antenna_max: maximum antenna gain specified by the regulatory domain (dB)
 */
struct ocb_config_channel {
	uint32_t chan_freq;
	uint32_t bandwidth;
	struct cdf_mac_addr mac_address;
	struct wmi_ocb_qos_params qos_params[WMI_MAX_NUM_AC];
	uint32_t max_pwr;
	uint32_t min_pwr;
	uint8_t reg_pwr;
	uint8_t antenna_max;
	uint16_t flags;
};

/**
 * struct ocb_config_param
 * @session_id: session id
 * @channel_count: number of channels
 * @schedule_size: size of the channel schedule
 * @flags: reserved
 * @channels: array of OCB channels
 * @schedule: array of OCB schedule elements
 * @dcc_ndl_chan_list_len: size of the ndl_chan array
 * @dcc_ndl_chan_list: array of dcc channel info
 * @dcc_ndl_active_state_list_len: size of the active state array
 * @dcc_ndl_active_state_list: array of active states
 * @adapter: the OCB adapter
 * @dcc_stats_callback: callback for the response event
 */
struct ocb_config_param {
	uint8_t session_id;
	uint32_t channel_count;
	uint32_t schedule_size;
	uint32_t flags;
	struct ocb_config_channel *channels;
	struct ocb_config_sched *schedule;
	uint32_t dcc_ndl_chan_list_len;
	void *dcc_ndl_chan_list;
	uint32_t dcc_ndl_active_state_list_len;
	void *dcc_ndl_active_state_list;
};

/**
 * struct t_thermal_cmd_params - thermal command parameters
 * @min_temp: minimum temprature
 * @max_temp: maximum temprature
 * @thermal_enable: thermal enable
 */
struct thermal_cmd_params {
	uint16_t min_temp;
	uint16_t max_temp;
	uint8_t thermal_enable;
};

#define WMI_LRO_IPV4_SEED_ARR_SZ 5
#define WMI_LRO_IPV6_SEED_ARR_SZ 11

/**
 * struct wmi_lro_config_cmd_t - set LRO init parameters
 * @lro_enable: indicates whether lro is enabled
 * @tcp_flag: If the TCP flags from the packet do not match
 * the values in this field after masking with TCP flags mask
 * below, packet is not LRO eligible
 * @tcp_flag_mask: field for comparing the TCP values provided
 * above with the TCP flags field in the received packet
 * @toeplitz_hash_ipv4: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv4 packets
 * @toeplitz_hash_ipv6: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv6 packets
 */
struct wmi_lro_config_cmd_t {
	uint32_t lro_enable;
	uint32_t tcp_flag:9,
		tcp_flag_mask:9;
	uint32_t toeplitz_hash_ipv4[WMI_LRO_IPV4_SEED_ARR_SZ];
	uint32_t toeplitz_hash_ipv6[WMI_LRO_IPV6_SEED_ARR_SZ];
};
#endif /* _WMI_UNIFIED_PARAM_H_ */

