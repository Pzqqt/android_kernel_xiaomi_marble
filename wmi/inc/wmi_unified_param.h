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
#define WMI_MAX_NUM_ARGS 8
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
#define WMI_IPV4_ADDR_LEN       4
#define WMI_KEEP_ALIVE_NULL_PKT              1
#define WMI_KEEP_ALIVE_UNSOLICIT_ARP_RSP     2
#define WMI_MAC_MAX_KEY_LENGTH          32
#define WMI_KRK_KEY_LEN     16
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define WMI_BTK_KEY_LEN     32
#define WMI_ROAM_R0KH_ID_MAX_LEN    48
#define WMI_ROAM_SCAN_PSK_SIZE    32
#endif
#define WMI_NOISE_FLOOR_DBM_DEFAULT      (-96)
#ifdef WLAN_NS_OFFLOAD
/* support only one IPv6 offload */
#define WMI_MAC_NS_OFFLOAD_SIZE                          1
/* Number of target IP V6 addresses for NS offload */
#define WMI_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA            16
#define WMI_MAC_IPV6_ADDR_LEN                            16
#define WMI_IPV6_ADDR_VALID                              1
#endif /* WLAN_NS_OFFLOAD */
#define WMI_EXTSCAN_MAX_HOTLIST_SSIDS                    8
#define WMI_ROAM_MAX_CHANNELS                            80
#ifdef FEATURE_WLAN_EXTSCAN
#define WMI_MAX_EXTSCAN_MSG_SIZE        1536
#define WMI_EXTSCAN_REST_TIME           100
#define WMI_EXTSCAN_MAX_SCAN_TIME       50000
#define WMI_EXTSCAN_BURST_DURATION      150
#endif
#define WMI_SCAN_NPROBES_DEFAULT            (2)
#define WMI_SEC_TO_MSEC(sec)         (sec * 1000)  /* sec to msec */
#define WMI_MSEC_TO_USEC(msec)       (msec * 1000) /* msec to usec */
#define WMI_NLO_FREQ_THRESH          1000       /* in MHz */
#include "qdf_atomic.h"

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
 * struct mac_ssid - mac ssid structure
 * @length:
 * @mac_ssid[WMI_MAC_MAX_SSID_LENGTH]:
 */
struct mac_ssid {
	uint8_t length;
	uint8_t mac_ssid[WMI_MAC_MAX_SSID_LENGTH];
} qdf_packed;

/**
 * struct vdev_start_params - vdev start cmd parameter
 * @vdev_id: vdev id
 * @chan_freq: channel frequency
 * @chan_mode: channel mode
 * @band_center_freq1: center freq 1
 * @band_center_freq2: center freq 2
 * @flags: flags to set like pmf_enabled etc.
 * @is_dfs: flag to check if dfs enabled
 * @beacon_intval: beacon interval
 * @dtim_period: dtim period
 * @max_txpow: max tx power
 * @is_restart: flag to check if it is vdev
 * @ssid: ssid and ssid length info
 * @preferred_tx_streams: preferred tx streams
 * @preferred_rx_streams: preferred rx streams
 * @intr_update: flag to check if need to update
 *               required wma interface params
 * @intr_ssid: pointer to wma interface ssid
 * @intr_flags: poiter to wma interface flags
 * @requestor_id: to update requestor id
 * @disable_hw_ack: to update disable hw ack flag
 * @info: to update channel info
 * @reg_info_1: to update min power, max power,
 *              reg power and reg class id
 * @reg_info_2: to update antennamax
 */
struct vdev_start_params {
	uint8_t vdev_id;
	uint32_t chan_freq;
	uint32_t chan_mode;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t flags;
	bool is_dfs;
	uint32_t beacon_intval;
	uint32_t dtim_period;
	int32_t max_txpow;
	bool is_restart;
	bool is_half_rate;
	bool is_quarter_rate;
	uint32_t dis_hw_ack;
	uint32_t flag_dfs;
	uint8_t hidden_ssid;
	uint8_t pmf_enabled;
	struct mac_ssid ssid;
	uint32_t num_noa_descriptors;
	uint32_t preferred_rx_streams;
	uint32_t preferred_tx_streams;
};

/**
 * struct hidden_ssid_vdev_restart_params -
 *                    vdev restart cmd parameter
 * @session_id: session id
 * @ssid_len: ssid length
 * @ssid: ssid
 * @flags: flags
 * @requestor_id: requestor id
 * @disable_hw_ack: flag to disable hw ack feature
 * @mhz: channel frequency
 * @band_center_freq1: center freq 1
 * @band_center_freq2: center freq 2
 * @info: channel info
 * @reg_info_1: contains min power, max power,
 *              reg power and reg class id
 * @reg_info_2: contains antennamax
 * @hidden_ssid_restart_in_progress:
 *      flag to check if restart is in progress
 */
struct hidden_ssid_vdev_restart_params {
	uint8_t session_id;
	uint32_t ssid_len;
	uint32_t ssid[8];
	uint32_t flags;
	uint32_t requestor_id;
	uint32_t disable_hw_ack;
	uint32_t mhz;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t info;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
	qdf_atomic_t hidden_ssid_restart_in_progress;
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
	void *qdf_ctx;
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
	struct qdf_mac_addr mac_address;
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

/**
 * struct gtx_config_t - GTX config
 * @gtx_rt_mask: for HT and VHT rate masks
 * @gtx_usrcfg: host request for GTX mask
 * @gtx_threshold: PER Threshold (default: 10%)
 * @gtx_margin: PER margin (default: 2%)
 * @gtx_tcpstep: TCP step (default: 1)
 * @gtx_tpcMin: TCP min (default: 5)
 * @gtx_bwmask: BW mask (20/40/80/160 Mhz)
 */
struct wmi_gtx_config {
	uint32_t gtx_rt_mask[2];
	uint32_t gtx_usrcfg;
	uint32_t gtx_threshold;
	uint32_t gtx_margin;
	uint32_t gtx_tpcstep;
	uint32_t gtx_tpcmin;
	uint32_t gtx_bwmask;
};

/**
 * struct wmi_probe_resp_params - send probe response parameters
 * @bssId: BSSID
 * @pProbeRespTemplate: probe response template
 * @probeRespTemplateLen: probe response template length
 * @ucProxyProbeReqValidIEBmap: valid IE bitmap
 */
struct wmi_probe_resp_params {
	uint8_t  bssId[IEEE80211_ADDR_LEN];
	uint8_t *pProbeRespTemplate;
	uint32_t probeRespTemplateLen;
	uint32_t ucProxyProbeReqValidIEBmap[8];
};

/* struct set_key_params: structure containing
 *                        installation key parameters
 * @vdev_id: vdev id
 * @key_len: key length
 * @key_idx: key index
 * @peer_mac: peer mac address
 * @key_flags: key flags, 0:pairwise key, 1:group key, 2:static key
 * @key_cipher: key cipher based on security mode
 * @key_txmic_len: tx mic length
 * @key_rxmic_len: rx mic length
 * @rx_iv: receive IV, applicable only in case of WAPI
 * @tx_iv: transmit IV, applicable only in case of WAPI
 * @key_data: key data
 */
struct set_key_params {
	uint8_t vdev_id;
	uint16_t key_len;
	uint32_t key_idx;
	uint8_t peer_mac[IEEE80211_ADDR_LEN];
	uint32_t key_flags;
	uint32_t key_cipher;
	uint32_t key_txmic_len;
	uint32_t key_rxmic_len;
#ifdef FEATURE_WLAN_WAPI
	uint8_t rx_iv[16];
	uint8_t tx_iv[16];
#endif
	uint8_t key_data[WMI_MAC_MAX_KEY_LENGTH];
};

/**
 * struct sta_params - sta keep alive parameters
 * @vdev_id: vdev id
 * @method: keep alive method
 * @timeperiod: time to keep alive
 * @hostv4addr: host ipv4 address
 * @destv4addr: destination ipv4 address
 * @destmac: destination mac address
 */
struct sta_params {
	uint8_t vdev_id;
	uint32_t method;
	uint32_t timeperiod;
	uint8_t *hostv4addr;
	uint8_t *destv4addr;
	uint8_t *destmac;
};

/**
 * struct gateway_update_req_param - gateway parameter update request
 * @request_id: request id
 * @session_id: session id
 * @max_retries: Max ARP/NS retry attempts
 * @timeout: Retry interval
 * @ipv4_addr_type: on ipv4 network
 * @ipv6_addr_type: on ipv6 network
 * @gw_mac_addr: gateway mac addr
 * @ipv4_addr: ipv4 addr
 * @ipv6_addr: ipv6 addr
 */
struct gateway_update_req_param {
	uint32_t     request_id;
	uint32_t     session_id;
	uint32_t     max_retries;
	uint32_t     timeout;
	uint32_t     ipv4_addr_type;
	uint32_t     ipv6_addr_type;
	struct qdf_mac_addr  gw_mac_addr;
	uint8_t      ipv4_addr[QDF_IPV4_ADDR_SIZE];
	uint8_t      ipv6_addr[QDF_IPV6_ADDR_SIZE];
};

/**
 * struct rssi_monitor_param - rssi monitoring
 * @request_id: request id
 * @session_id: session id
 * @min_rssi: minimum rssi
 * @max_rssi: maximum rssi
 * @control: flag to indicate start or stop
 */
struct rssi_monitor_param {
	uint32_t request_id;
	uint32_t session_id;
	int8_t   min_rssi;
	int8_t   max_rssi;
	bool     control;
};

/**
 * struct scan_mac_oui - oui paramters
 * @oui: oui parameters
 */
struct scan_mac_oui {
	uint8_t oui[WMI_WIFI_SCANNING_MAC_OUI_LENGTH];
};

#define WMI_PASSPOINT_REALM_LEN 256
#define WMI_PASSPOINT_ROAMING_CONSORTIUM_ID_NUM 16
#define WMI_PASSPOINT_PLMN_LEN 3
/**
 * struct wifi_passpoint_network_param - passpoint network block
 * @id: identifier of this network block
 * @realm: null terminated UTF8 encoded realm, 0 if unspecified
 * @roaming_consortium_ids: roaming consortium ids to match, 0s if unspecified
 * @plmn: mcc/mnc combination as per rules, 0s if unspecified
 */
struct wifi_passpoint_network_param {
	uint32_t id;
	uint8_t  realm[WMI_PASSPOINT_REALM_LEN];
	int64_t  roaming_consortium_ids[WMI_PASSPOINT_ROAMING_CONSORTIUM_ID_NUM];
	uint8_t  plmn[WMI_PASSPOINT_PLMN_LEN];
};

/**
 * struct wifi_passpoint_req_param - passpoint request
 * @request_id: request identifier
 * @num_networks: number of networks
 * @networks: passpoint networks
 */
struct wifi_passpoint_req_param {
	uint32_t request_id;
	uint32_t session_id;
	uint32_t num_networks;
	struct wifi_passpoint_network_param networks[];
};

/* struct mobility_domain_info - structure containing
 *                               mobility domain info
 * @mdie_present: mobility domain present or not
 * @mobility_domain: mobility domain
 */
struct mobility_domain_info {
	uint8_t mdie_present;
	uint16_t mobility_domain;
};

/* struct roam_offload_scan_params - structure
 *     containing roaming offload scan parameters
 * @is_roam_req_valid: flag to tell whether roam req
 *                     is valid or NULL
 * @mode: stores flags for scan
 * @vdev_id: vdev id
 * @roam_offload_enabled: flag for offload enable
 * @psk_pmk: pre shared key/pairwise master key
 * @pmk_len: length of PMK
 * @prefer_5ghz: prefer select 5G candidate
 * @roam_rssi_cat_gap: gap for every category bucket
 * @select_5ghz_margin: select 5 Ghz margin
 * @krk: KRK
 * @btk: BTK
 * @reassoc_failure_timeout: reassoc failure timeout
 * @rokh_id_length: r0kh id length
 * @rokh_id: r0kh id
 * @roam_key_mgmt_offload_enabled: roam offload flag
 * @auth_mode: authentication mode
 * @is_ese_assoc: flag to determine ese assoc
 * @mdid: mobility domain info
 * @roam_offload_params: roam offload tlv params
 */
struct roam_offload_scan_params {
	uint8_t is_roam_req_valid;
	uint32_t mode;
	uint32_t vdev_id;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t roam_offload_enabled;
	uint8_t psk_pmk[WMI_ROAM_SCAN_PSK_SIZE];
	uint32_t pmk_len;
	uint8_t prefer_5ghz;
	uint8_t roam_rssi_cat_gap;
	uint8_t select_5ghz_margin;
	uint8_t krk[WMI_KRK_KEY_LEN];
	uint8_t btk[WMI_BTK_KEY_LEN];
	uint32_t reassoc_failure_timeout;
	uint32_t rokh_id_length;
	uint8_t rokh_id[WMI_ROAM_R0KH_ID_MAX_LEN];
	uint8_t roam_key_mgmt_offload_enabled;
	int auth_mode;
#endif
	bool is_ese_assoc;
	struct mobility_domain_info mdid;
	wmi_roam_offload_tlv_param roam_offload_params;
};

/* struct roam_offload_scan_rssi_params - structure containing
 *              parameters for roam offload scan based on RSSI
 * @rssi_thresh: rssi threshold
 * @rssi_thresh_diff: difference in rssi threshold
 * @hi_rssi_scan_max_count: 5G scan max count
 * @hi_rssi_scan_rssi_delta: 5G scan rssi change threshold value
 * @hi_rssi_scan_rssi_ub: 5G scan upper bound
 * @raise_rssi_thresh_5g: flag to determine penalty and boost thresholds
 * @session_id: vdev id
 * @penalty_threshold_5g: RSSI threshold below which 5GHz RSSI is penalized
 * @boost_threshold_5g: RSSI threshold above which 5GHz RSSI is favored
 * @raise_factor_5g: factor by which 5GHz RSSI is boosted
 * @drop_factor_5g: factor by which 5GHz RSSI is penalized
 * @max_raise_rssi_5g: maximum boost that can be applied to a 5GHz RSSI
 * @max_drop_rssi_5g: maximum penalty that can be applied to a 5GHz RSSI
 * @good_rssi_threshold: RSSI below which roam is kicked in by background
 *                       scan although rssi is still good
 * @roam_earlystop_thres_min: Minimum RSSI threshold value for early stop,
 *                            unit is dB above NF
 * @roam_earlystop_thres_max: Maximum RSSI threshold value for early stop,
 *                            unit is dB above NF
 */
struct roam_offload_scan_rssi_params {
	int8_t rssi_thresh;
	uint8_t rssi_thresh_diff;
	uint32_t hi_rssi_scan_max_count;
	uint32_t hi_rssi_scan_rssi_delta;
	int32_t hi_rssi_scan_rssi_ub;
	int raise_rssi_thresh_5g;
	uint8_t session_id;
	uint32_t penalty_threshold_5g;
	uint32_t boost_threshold_5g;
	uint8_t raise_factor_5g;
	uint8_t drop_factor_5g;
	int max_raise_rssi_5g;
	int max_drop_rssi_5g;
	uint32_t good_rssi_threshold;
	uint32_t roam_earlystop_thres_min;
	uint32_t roam_earlystop_thres_max;
};

/**
 * struct wifi_epno_network - enhanced pno network block
 * @ssid: ssid
 * @rssi_threshold: threshold for considering this SSID as found, required
 *		    granularity for this threshold is 4dBm to 8dBm
 * @flags: WIFI_PNO_FLAG_XXX
 * @auth_bit_field: auth bit field for matching WPA IE
 */
struct wifi_epno_network_params {
	struct mac_ssid  ssid;
	int8_t       rssi_threshold;
	uint8_t      flags;
	uint8_t      auth_bit_field;
};

/**
 * struct wifi_enhanched_pno_params - enhanced pno network params
 * @num_networks: number of ssids
 * @networks: PNO networks
 */
struct wifi_enhanched_pno_params {
	uint32_t    request_id;
	uint32_t    session_id;
	uint32_t    num_networks;
	struct wifi_epno_network_params networks[];
};

enum {
	WMI_AP_RX_DATA_OFFLOAD             = 0x00,
	WMI_STA_RX_DATA_OFFLOAD            = 0x01,
};

/**
 * enum extscan_report_events_type - extscan report events type
 * @EXTSCAN_REPORT_EVENTS_BUFFER_FULL: report only when scan history is % full
 * @EXTSCAN_REPORT_EVENTS_EACH_SCAN: report a scan completion event after scan
 * @EXTSCAN_REPORT_EVENTS_FULL_RESULTS: forward scan results
 *		(beacons/probe responses + IEs)
 *		in real time to HAL, in addition to completion events.
 *		Note: To keep backward compatibility,
 *		fire completion events regardless of REPORT_EVENTS_EACH_SCAN.
 * @EXTSCAN_REPORT_EVENTS_NO_BATCH: controls batching,
 *		0 => batching, 1 => no batching
 */
enum wmi_extscan_report_events_type {
	WMI_EXTSCAN_REPORT_EVENTS_BUFFER_FULL   = 0x00,
	WMI_EXTSCAN_REPORT_EVENTS_EACH_SCAN     = 0x01,
	WMI_EXTSCAN_REPORT_EVENTS_FULL_RESULTS  = 0x02,
	WMI_EXTSCAN_REPORT_EVENTS_NO_BATCH      = 0x04,
};

/**
 * struct ipa_offload_control_params - ipa offload parameters
 * @offload_type: ipa offload type
 * @vdev_id: vdev id
 * @enable: ipa offload enable/disable
 */
struct ipa_offload_control_params {
	uint32_t offload_type;
	uint32_t vdev_id;
	uint32_t enable;
};

/**
 * struct extscan_capabilities_params - ext scan capablities
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_capabilities_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct extscan_capabilities_reset_params - ext scan capablities reset parameter
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_capabilities_reset_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct extscan_bssid_hotlist_reset_params - ext scan hotlist reset parameter
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_bssid_hotlist_reset_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct extscan_stop_req_params - ext scan stop parameter
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_stop_req_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct ap_threshold_params - ap threshold parameter
 * @bssid: mac address
 * @low: low threshold
 * @high: high threshold
 */
struct ap_threshold_params {
	struct qdf_mac_addr bssid;
	int32_t low;
	int32_t high;
};

/**
 * struct extscan_set_sig_changereq_params - ext scan channel parameter
 * @request_id: mac address
 * @session_id: low threshold
 * @rssi_sample_size: Number of samples for averaging RSSI
 * @lostap_sample_size: Number of missed samples to confirm AP loss
 * @min_breaching: Number of APs breaching threshold required for firmware
 * @num_ap: no of scanned ap
 * @ap: ap threshold parameter
 */
struct extscan_set_sig_changereq_params {
	uint32_t request_id;
	uint8_t session_id;
	uint32_t rssi_sample_size;
	uint32_t lostap_sample_size;
	uint32_t min_breaching;
	uint32_t num_ap;
	struct ap_threshold_params ap[WMI_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS];
};

/**
 * struct extscan_cached_result_params - ext scan cached parameter
 * @request_id: mac address
 * @session_id: low threshold
 * @flush: cached results flush
 */
struct extscan_cached_result_params {
	uint32_t request_id;
	uint8_t session_id;
	bool flush;
};

#ifdef FEATURE_WLAN_SCAN_PNO
/* Set PNO */
#define WMI_PNO_MAX_NETW_CHANNELS  26
#define WMI_PNO_MAX_NETW_CHANNELS_EX  60
#define WMI_PNO_MAX_SUPP_NETWORKS  16

/*
 * size based of dot11 declaration without extra IEs as we will not carry those
 * for PNO
 */
#define WMI_PNO_MAX_PB_REQ_SIZE    450

#define WMI_PNO_24G_DEFAULT_CH     1
#define WMI_PNO_5G_DEFAULT_CH      36

/**
 * enum pno_mode - pno mode types
 * @WMI_PNO_MODE_IMMEDIATE: immidiate mode
 * @WMI_PNO_MODE_ON_SUSPEND: suspend on mode
 * @WMI_PNO_MODE_ON_RESUME: resume on mode
 * @WMI_PNO_MODE_MAX: max range
 */
enum pno_mode {
	WMI_PNO_MODE_IMMEDIATE,
	WMI_PNO_MODE_ON_SUSPEND,
	WMI_PNO_MODE_ON_RESUME,
	WMI_PNO_MODE_MAX
};

/**
 * struct pno_nw_type - pno nw type
 * @ssid: mac ssid
 * @authentication: authentication type
 * @encryption: encryption type
 * @bcastNetwType: broadcast nw type
 * @ucChannelCount: uc channel count
 * @aChannels: pno channel
 * @rssiThreshold: rssi threshold
 */
struct pno_nw_type {
	struct mac_ssid ssid;
	uint32_t authentication;
	uint32_t encryption;
	uint32_t bcastNetwType;
	uint8_t ucChannelCount;
	uint8_t aChannels[WMI_PNO_MAX_NETW_CHANNELS_EX];
	int32_t rssiThreshold;
};

/**
 * struct pno_scan_req_params - PNO Scan request structure
 * @enable: flag to enable or disable
 * @modePNO: PNO Mode
 * @ucNetworksCount: Number of networks
 * @aNetworks: Preferred network list
 * @sessionId: Session identifier
 * @fast_scan_period: Fast Scan period
 * @slow_scan_period: Slow scan period
 * @fast_scan_max_cycles: Fast scan max cycles
 * @us24GProbeTemplateLen: 2.4G probe template length
 * @p24GProbeTemplate: 2.4G probe template
 * @us5GProbeTemplateLen: 5G probe template length
 * @p5GProbeTemplate: 5G probe template
 */
struct pno_scan_req_params {
	uint8_t enable;
	enum pno_mode modePNO;
	uint8_t ucNetworksCount;
	struct pno_nw_type aNetworks[WMI_PNO_MAX_SUPP_NETWORKS];
	uint8_t sessionId;
	uint32_t fast_scan_period;
	uint32_t slow_scan_period;
	uint8_t fast_scan_max_cycles;
	uint32_t        active_min_time;
	uint32_t        active_max_time;
	uint32_t        passive_min_time;
	uint32_t        passive_max_time;
	uint16_t us24GProbeTemplateLen;
	uint8_t p24GProbeTemplate[WMI_PNO_MAX_PB_REQ_SIZE];
	uint16_t us5GProbeTemplateLen;
	uint8_t p5GProbeTemplate[WMI_PNO_MAX_PB_REQ_SIZE];
#ifdef FEATURE_WLAN_SCAN_PNO
	bool pno_channel_prediction;
	uint8_t top_k_num_of_channels;
	uint8_t stationary_thresh;
	uint32_t channel_prediction_full_scan;
#endif
};

#endif /* FEATURE_WLAN_SCAN_PNO */

#define WMI_WLAN_EXTSCAN_MAX_CHANNELS                 36
#define WMI_WLAN_EXTSCAN_MAX_BUCKETS                  16
#define WMI_WLAN_EXTSCAN_MAX_HOTLIST_APS              128
#define WMI_WLAN_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS   64
#define WMI_EXTSCAN_MAX_HOTLIST_SSIDS            8

/**
 * struct wifi_scan_channelspec_params - wifi scan channel parameter
 * @channel: Frequency in MHz
 * @dwellTimeMs: dwell time
 * @flush: cached results flush
 * @passive: passive scan
 * @chnlClass: channel class
 */
struct wifi_scan_channelspec_params {
	uint32_t channel;
	uint32_t dwellTimeMs;
	bool passive;
	uint8_t chnlClass;
};

/**
 * enum wmi_wifi_band - wifi band
 * @WMI_WIFI_BAND_UNSPECIFIED: unspecified band
 * @WMI_WIFI_BAND_BG: 2.4 GHz
 * @WMI_WIFI_BAND_A: 5 GHz without DFS
 * @WMI_WIFI_BAND_ABG: 2.4 GHz + 5 GHz; no DFS
 * @WMI_WIFI_BAND_A_DFS_ONLY: 5 GHz DFS only
 * @WMI_WIFI_BAND_A_WITH_DFS: 5 GHz with DFS
 * @WMI_WIFI_BAND_ABG_WITH_DFS: 2.4 GHz + 5 GHz with DFS
 * @WMI_WIFI_BAND_MAX: max range
 */
enum wmi_wifi_band {
	WMI_WIFI_BAND_UNSPECIFIED,
	WMI_WIFI_BAND_BG = 1,
	WMI_WIFI_BAND_A = 2,
	WMI_WIFI_BAND_ABG = 3,
	WMI_WIFI_BAND_A_DFS_ONLY = 4,
	/* 5 is reserved */
	WMI_WIFI_BAND_A_WITH_DFS = 6,
	WMI_WIFI_BAND_ABG_WITH_DFS = 7,
	/* Keep it last */
	WMI_WIFI_BAND_MAX
};

/**
 * struct wifi_scan_bucket_params - wifi scan bucket spec
 * @bucket: bucket identifier
 * @band: wifi band
 * @period: Desired period, in millisecond; if this is too
 *		low, the firmware should choose to generate results as fast as
 *		it can instead of failing the command byte
 *		for exponential backoff bucket this is the min_period
 * @reportEvents: 0 => normal reporting (reporting rssi history
 *		only, when rssi history buffer is % full)
 *		1 => same as 0 + report a scan completion event after scanning
 *		this bucket
 *		2 => same as 1 + forward scan results
 *		(beacons/probe responses + IEs) in real time to HAL
 * @max_period: if max_period is non zero or different than period,
 *		then this bucket is an exponential backoff bucket and
 *		the scan period will grow exponentially as per formula:
 *		actual_period(N) = period ^ (N/(step_count+1)) to a
 *		maximum period of max_period
 * @exponent: for exponential back off bucket: multiplier:
 *		new_period = old_period * exponent
 * @step_count: for exponential back off bucket, number of scans performed
 *		at a given period and until the exponent is applied
 * @numChannels: channels to scan; these may include DFS channels
 *		Note that a given channel may appear in multiple buckets
 * @min_dwell_time_active: per bucket minimum active dwell time
 * @max_dwell_time_active: per bucket maximum active dwell time
 * @min_dwell_time_passive: per bucket minimum passive dwell time
 * @max_dwell_time_passive: per bucket maximum passive dwell time
 * @channels: Channel list
 */
struct wifi_scan_bucket_params {
	uint8_t         bucket;
	enum wmi_wifi_band   band;
	uint32_t        period;
	uint32_t        reportEvents;
	uint32_t        max_period;
	uint32_t        exponent;
	uint32_t        step_count;
	uint32_t        numChannels;
	uint32_t        min_dwell_time_active;
	uint32_t        max_dwell_time_active;
	uint32_t        min_dwell_time_passive;
	uint32_t        max_dwell_time_passive;
	struct wifi_scan_channelspec_params channels[WMI_WLAN_EXTSCAN_MAX_CHANNELS];
};

/**
 * struct wifi_scan_cmd_req_params - wifi scan command request params
 * @basePeriod: base timer period
 * @maxAPperScan: max ap per scan
 * @report_threshold_percent: report threshold
 *	in %, when buffer is this much full, wake up host
 * @report_threshold_num_scans: report threshold number of scans
 *	in number of scans, wake up host after these many scans
 * @requestId: request id
 * @sessionId: session id
 * @numBuckets: number of buckets
 * @min_dwell_time_active: per bucket minimum active dwell time
 * @max_dwell_time_active: per bucket maximum active dwell time
 * @min_dwell_time_passive: per bucket minimum passive dwell time
 * @max_dwell_time_passive: per bucket maximum passive dwell time
 * @configuration_flags: configuration flags
 * @buckets: buckets array
 */
struct wifi_scan_cmd_req_params {
	uint32_t basePeriod;
	uint32_t maxAPperScan;

	uint32_t report_threshold_percent;
	uint32_t report_threshold_num_scans;

	uint32_t requestId;
	uint8_t  sessionId;
	uint32_t numBuckets;

	uint32_t min_dwell_time_active;
	uint32_t max_dwell_time_active;
	uint32_t min_dwell_time_passive;
	uint32_t max_dwell_time_passive;
	uint32_t configuration_flags;
	struct wifi_scan_bucket_params buckets[WMI_WLAN_EXTSCAN_MAX_BUCKETS];
};

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
#define WMI_CFG_VALID_CHANNEL_LIST_LEN    100

/**
 * struct plm_req_params - plm req parameter
 * @diag_token: Dialog token
 * @meas_token: measurement token
 * @num_bursts: total number of bursts
 * @burst_int: burst interval in seconds
 * @meas_duration:in TU's,STA goes off-ch
 * @burst_len: no of times the STA should cycle through PLM ch list
 * @desired_tx_pwr: desired tx power
 * @mac_addr: MC dest addr
 * @plm_num_ch: channel numbers
 * @plm_ch_list: channel list
 * @session_id: session id
 * @enable:  enable/disable
 */
struct plm_req_params {
	uint16_t diag_token;
	uint16_t meas_token;
	uint16_t num_bursts;
	uint16_t burst_int;
	uint16_t meas_duration;
	/* no of times the STA should cycle through PLM ch list */
	uint8_t burst_len;
	int8_t desired_tx_pwr;
	struct qdf_mac_addr mac_addr;
	/* no of channels */
	uint8_t plm_num_ch;
	/* channel numbers */
	uint8_t plm_ch_list[WMI_CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t session_id;
	bool enable;
};
#endif
#define MAX_SSID_ALLOWED_LIST    4
#define MAX_BSSID_AVOID_LIST     16
#define MAX_BSSID_FAVORED      16


/**
 * struct mac_ts_info_tfc - mac ts info parameters
 * @burstSizeDefn: burst size
 * @reserved: reserved
 * @ackPolicy: ack policy
 * @psb: psb
 * @aggregation: aggregation
 * @accessPolicy: access policy
 * @direction: direction
 * @tsid: direction
 * @trafficType: traffic type
 */
struct mac_ts_info_tfc {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t burstSizeDefn:1;
	uint8_t reserved:7;
#else
	uint8_t reserved:7;
	uint8_t burstSizeDefn:1;
#endif

#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t ackPolicy:2;
	uint16_t userPrio:3;
	uint16_t psb:1;
	uint16_t aggregation:1;
	uint16_t accessPolicy:2;
	uint16_t direction:2;
	uint16_t tsid:4;
	uint16_t trafficType:1;
#else
	uint16_t trafficType:1;
	uint16_t tsid:4;
	uint16_t direction:2;
	uint16_t accessPolicy:2;
	uint16_t aggregation:1;
	uint16_t psb:1;
	uint16_t userPrio:3;
	uint16_t ackPolicy:2;
#endif
} qdf_packed;

/**
 * struct mac_ts_info_sch - mac ts info schedule parameters
 * @rsvd: reserved
 * @schedule: schedule bit
 */
struct mac_ts_info_sch {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t rsvd:7;
	uint8_t schedule:1;
#else
	uint8_t schedule:1;
	uint8_t rsvd:7;
#endif
} qdf_packed;

/**
 * struct mac_ts_info_sch - mac ts info schedule parameters
 * @traffic: mac tfc parameter
 * @schedule: mac schedule parameters
 */
struct mac_ts_info {
	struct mac_ts_info_tfc traffic;
	struct mac_ts_info_sch schedule;
} qdf_packed;

/**
 * struct mac_tspec_ie - mac ts spec
 * @type: type
 * @length: length
 * @tsinfo: tsinfo
 * @nomMsduSz: nomMsduSz
 * @maxMsduSz: maxMsduSz
 * @minSvcInterval: minSvcInterval
 * @maxSvcInterval: maxSvcInterval
 * @inactInterval: inactInterval
 * @suspendInterval: suspendInterval
 * @svcStartTime: svcStartTime
 * @minDataRate: minDataRate
 * @meanDataRate: meanDataRate
 * @peakDataRate: peakDataRate
 * @maxBurstSz: maxBurstSz
 * @delayBound: delayBound
 * @minPhyRate: minPhyRate
 * @surplusBw: surplusBw
 * @mediumTime: mediumTime
 */
struct mac_tspec_ie {
	uint8_t type;
	uint8_t length;
	struct mac_ts_info tsinfo;
	uint16_t nomMsduSz;
	uint16_t maxMsduSz;
	uint32_t minSvcInterval;
	uint32_t maxSvcInterval;
	uint32_t inactInterval;
	uint32_t suspendInterval;
	uint32_t svcStartTime;
	uint32_t minDataRate;
	uint32_t meanDataRate;
	uint32_t peakDataRate;
	uint32_t maxBurstSz;
	uint32_t delayBound;
	uint32_t minPhyRate;
	uint16_t surplusBw;
	uint16_t mediumTime;
} qdf_packed;

/**
 * struct add_ts_param - ADDTS related parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: CDF status
 * @sessionId: session id
 * @tsm_interval: TSM interval period passed from UMAC to WMI
 * @setRICparams: RIC parameters
 * @sme_session_id: sme session id
 */
struct add_ts_param {
	uint16_t staIdx;
	uint16_t tspecIdx;
	struct mac_tspec_ie tspec;
	QDF_STATUS status;
	uint8_t sessionId;
#ifdef FEATURE_WLAN_ESE
	uint16_t tsm_interval;
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t setRICparams;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	uint8_t sme_session_id;
};

/**
 * struct delts_req_info - DELTS request parameter
 * @tsinfo: ts info
 * @tspec: ts spec
 * @wmeTspecPresent: wme ts spec flag
 * @wsmTspecPresent: wsm ts spec flag
 * @lleTspecPresent: lle ts spec flag
 */
struct delts_req_info {
	struct mac_ts_info tsinfo;
	struct mac_tspec_ie tspec;
	uint8_t wmeTspecPresent:1;
	uint8_t wsmTspecPresent:1;
	uint8_t lleTspecPresent:1;
};

/**
 * struct del_ts_params - DELTS related parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC identifier uniquely identifying a TSPEC for a STA in a BSS
 * @bssId: BSSID
 * @sessionId: session id
 * @userPrio: user priority
 * @delTsInfo: DELTS info
 * @setRICparams: RIC parameters
 */
struct del_ts_params {
	uint16_t staIdx;
	uint16_t tspecIdx;
	uint8_t bssId[IEEE80211_ADDR_LEN];
	uint8_t sessionId;
	uint8_t userPrio;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	struct delts_req_info delTsInfo;
	uint8_t setRICparams;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
};

/**
 * struct ll_stats_clear_params - ll stats clear parameter
 * @req_id: request id
 * @sta_id: sta id
 * @stats_clear_mask: stats clear mask
 * @stop_req: stop request
 */
struct ll_stats_clear_params {
	uint32_t req_id;
	uint8_t sta_id;
	uint32_t stats_clear_mask;
	uint8_t stop_req;
};

/**
 * struct ll_stats_set_params - ll stats get parameter
 * @req_id: request id
 * @sta_id: sta id
 * @mpdu_size_threshold: mpdu sixe threshold
 * @aggressive_statistics_gathering: aggressive_statistics_gathering
 */
struct ll_stats_set_params {
	uint32_t req_id;
	uint8_t sta_id;
	uint32_t mpdu_size_threshold;
	uint32_t aggressive_statistics_gathering;
};

/**
 * struct ll_stats_get_params - ll stats parameter
 * @req_id: request id
 * @sta_id: sta id
 * @param_id_mask: param is mask
 */
struct ll_stats_get_params {
	uint32_t req_id;
	uint8_t sta_id;
	uint32_t param_id_mask;
};

/**
 * struct pe_stats_req - pe stats parameter
 * @msg_type: message type is same as the request type
 * @msg_len: length of the entire request
 * @sta_id: Per STA stats request must contain valid
 * @stats_mask: categories of stats requested
 * @session_id: wsm ts spec flag
 */
struct pe_stats_req {
	/* Common for all types are requests */
	uint16_t msg_type;
	uint16_t msg_len;
	uint32_t sta_id;
	/* categories of stats requested. look at ePEStatsMask */
	uint32_t stats_mask;
	uint8_t session_id;
};

/**
 * struct link_status_params - link stats parameter
 * @msg_type: message type is same as the request type
 * @msg_len: length of the entire request
 * @link_status: wme ts spec flag
 * @session_id: wsm ts spec flag
 */
struct link_status_params {
	uint16_t msg_type;
	uint16_t msg_len;
	uint8_t link_status;
	uint8_t session_id;
};

/**
 * struct dhcp_stop_ind_params - DHCP Stop indication message
 * @msgtype: message type is same as the request type
 * @msglen: length of the entire request
 * @device_mode: Mode of the device(ex:STA, AP)
 * @adapter_macaddr: MAC address of the adapter
 * @peer_macaddr: MAC address of the connected peer
 */
struct dhcp_stop_ind_params {
	uint16_t msgtype;
	uint16_t msglen;
	uint8_t device_mode;
	struct qdf_mac_addr adapter_macaddr;
	struct qdf_mac_addr peer_macaddr;
};

/**
 * struct aggr_add_ts_param - ADDTS parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: CDF status
 * @sessionId: session id
 */
struct aggr_add_ts_param {
	uint16_t staIdx;
	uint16_t tspecIdx;
	struct mac_tspec_ie tspec[WMI_QOS_NUM_AC_MAX];
	QDF_STATUS status[WMI_QOS_NUM_AC_MAX];
	uint8_t sessionId;
};

#define    WMI_MAX_FILTER_TEST_DATA_LEN       8
#define    WMI_MAX_NUM_MULTICAST_ADDRESS    240
#define    WMI_MAX_NUM_FILTERS               20
#define    WMI_MAX_NUM_TESTS_PER_FILTER      10

/**
 * enum packet_filter_type - packet filter type
 * @WMI_RCV_FILTER_TYPE_INVALID: invalid type
 * @WMI_RCV_FILTER_TYPE_FILTER_PKT: filter packet type
 * @WMI_RCV_FILTER_TYPE_BUFFER_PKT: buffer packet type
 * @WMI_RCV_FILTER_TYPE_MAX_ENUM_SIZE: max enum size
 */
enum packet_filter_type {
	WMI_RCV_FILTER_TYPE_INVALID,
	WMI_RCV_FILTER_TYPE_FILTER_PKT,
	WMI_RCV_FILTER_TYPE_BUFFER_PKT,
	WMI_RCV_FILTER_TYPE_MAX_ENUM_SIZE
};

/**
 * enum packet_protocol_type - packet protocol type
 * @WMI_FILTER_HDR_TYPE_INVALID: invalid type
 * @WMI_FILTER_HDR_TYPE_MAC: mac type
 * @WMI_FILTER_HDR_TYPE_ARP: trp type
 * @WMI_FILTER_HDR_TYPE_IPV4: ipv4 type
 * @WMI_FILTER_HDR_TYPE_IPV6: ipv6 type
 * @WMI_FILTER_HDR_TYPE_UDP: udp type
 * @WMI_FILTER_HDR_TYPE_MAX: max type
 */
enum packet_protocol_type {
	WMI_FILTER_HDR_TYPE_INVALID,
	WMI_FILTER_HDR_TYPE_MAC,
	WMI_FILTER_HDR_TYPE_ARP,
	WMI_FILTER_HDR_TYPE_IPV4,
	WMI_FILTER_HDR_TYPE_IPV6,
	WMI_FILTER_HDR_TYPE_UDP,
	WMI_FILTER_HDR_TYPE_MAX
};

/**
 * enum packet_filter_comp_type - packet filter comparison type
 * @WMI_FILTER_CMP_TYPE_INVALID: invalid type
 * @WMI_FILTER_CMP_TYPE_EQUAL: type equal
 * @WMI_FILTER_CMP_TYPE_MASK_EQUAL: mask equal
 * @WMI_FILTER_CMP_TYPE_NOT_EQUAL: type not equal
 * @WMI_FILTER_CMP_TYPE_MASK_NOT_EQUAL: mask not equal
 * @WMI_FILTER_CMP_TYPE_MAX: max type
 */
enum packet_filter_comp_type {
	WMI_FILTER_CMP_TYPE_INVALID,
	WMI_FILTER_CMP_TYPE_EQUAL,
	WMI_FILTER_CMP_TYPE_MASK_EQUAL,
	WMI_FILTER_CMP_TYPE_NOT_EQUAL,
	WMI_FILTER_CMP_TYPE_MASK_NOT_EQUAL,
	WMI_FILTER_CMP_TYPE_MAX
};

/**
 * struct rcv_pkt_filter_params - recieve packet filter parameters
 * @protocolLayer - protocol layer
 * @cmpFlag - comparison flag
 * @dataLength - data length
 * @dataOffset - data offset
 * @reserved - resserved
 * @compareData - compare data
 * @dataMask - data mask
 */
struct rcv_pkt_filter_params {
	enum packet_protocol_type protocolLayer;
	enum packet_filter_comp_type cmpFlag;
	uint16_t dataLength;
	uint8_t dataOffset;
	uint8_t reserved;
	uint8_t compareData[WMI_MAX_FILTER_TEST_DATA_LEN];
	uint8_t dataMask[WMI_MAX_FILTER_TEST_DATA_LEN];
};

/**
 * struct rcv_pkt_filter_config - recieve packet filter info
 * @filterId - filter id
 * @filterType - filter type
 * @numFieldParams - no of fields
 * @coalesceTime - reserved parameter
 * @self_macaddr - self mac address
 * @bssid - Bssid of the connected AP
 * @paramsData - data parameter
 */
struct rcv_pkt_filter_config {
	uint8_t filterId;
	enum packet_filter_type filterType;
	uint32_t numFieldParams;
	uint32_t coalesceTime;
	struct qdf_mac_addr self_macaddr;
	struct qdf_mac_addr bssid;
	struct rcv_pkt_filter_params paramsData[WMI_MAX_NUM_TESTS_PER_FILTER];
};

/**
 * struct vdev_ie_info_param - IE info
 * @vdev_id - vdev for which the IE is being sent
 * @ie_id - ID of the IE
 * @length - length of the IE data
 * @data - IE data
 *
 * This structure is used to store the IE information.
 */
struct vdev_ie_info_param {
	uint32_t vdev_id;
	uint32_t ie_id;
	uint32_t length;
	uint8_t *data;
};

#define WMI_MAX_NUM_FW_SEGMENTS 4

/**
 * struct fw_dump_seg_req_param - individual segment details
 * @seg_id - segment id.
 * @seg_start_addr_lo - lower address of the segment.
 * @seg_start_addr_hi - higher address of the segment.
 * @seg_length - length of the segment.
 * @dst_addr_lo - lower address of the destination buffer.
 * @dst_addr_hi - higher address of the destination buffer.
 *
 * This structure carries the information to firmware about the
 * individual segments. This structure is part of firmware memory
 * dump request.
 */
struct fw_dump_seg_req_param {
	uint8_t seg_id;
	uint32_t seg_start_addr_lo;
	uint32_t seg_start_addr_hi;
	uint32_t seg_length;
	uint32_t dst_addr_lo;
	uint32_t dst_addr_hi;
};

/**
 * struct fw_dump_req_param - firmware memory dump request details.
 * @request_id - request id.
 * @num_seg - requested number of segments.
 * @fw_dump_seg_req - individual segment information.
 *
 * This structure carries information about the firmware
 * memory dump request.
 */
struct fw_dump_req_param {
	uint32_t request_id;
	uint32_t num_seg;
	struct fw_dump_seg_req_param segment[WMI_MAX_NUM_FW_SEGMENTS];
};

#define WMI_TDLS_MAX_SUPP_CHANNELS       128
#define WMI_TDLS_MAX_SUPP_OPER_CLASSES   32
#define WMI_2_4_GHZ_MAX_FREQ  3000

/**
 * struct tdls_update_ch_params - channel parameters
 * @chanId: ID of the channel
 * @pwr: power level
 * @dfsSet: is dfs supported or not
 * @half_rate: is the channel operating at 10MHz
 * @quarter_rate: is the channel operating at 5MHz
 */
struct tdls_update_ch_params {
	uint8_t chanId;
	uint8_t pwr;
	bool dfsSet;
	bool half_rate;
	bool quarter_rate;
};

/**
 * struct tdls_peer_cap_params - TDLS peer capablities parameters
 * @isPeerResponder: is peer responder or not
 * @peerUapsdQueue: peer uapsd queue
 * @peerMaxSp: peer max SP value
 * @peerBuffStaSupport: peer buffer sta supported or not
 * @peerOffChanSupport: peer offchannel support
 * @peerCurrOperClass: peer current operating class
 * @selfCurrOperClass: self current operating class
 * @peerChanLen: peer channel length
 * @peerChan: peer channel list
 * @peerOperClassLen: peer operating class length
 * @peerOperClass: peer operating class
 * @prefOffChanNum: peer offchannel number
 * @prefOffChanBandwidth: peer offchannel bandwidth
 * @opClassForPrefOffChan: operating class for offchannel
 */
struct tdls_peer_cap_params {
	uint8_t isPeerResponder;
	uint8_t peerUapsdQueue;
	uint8_t peerMaxSp;
	uint8_t peerBuffStaSupport;
	uint8_t peerOffChanSupport;
	uint8_t peerCurrOperClass;
	uint8_t selfCurrOperClass;
	uint8_t peerChanLen;
	struct tdls_update_ch_params peerChan[WMI_TDLS_MAX_SUPP_CHANNELS];
	uint8_t peerOperClassLen;
	uint8_t peerOperClass[WMI_TDLS_MAX_SUPP_OPER_CLASSES];
	uint8_t prefOffChanNum;
	uint8_t prefOffChanBandwidth;
	uint8_t opClassForPrefOffChan;
};

/**
 * struct tdls_peer_state_params - TDLS peer state parameters
 * @vdevId: vdev id
 * @peerMacAddr: peer mac address
 * @peerCap: peer capabality
 */
struct tdls_peer_state_params {
	uint32_t vdevId;
	uint8_t peerMacAddr[IEEE80211_ADDR_LEN];
	uint32_t peerState;
	struct tdls_peer_cap_params peerCap;
};

/**
 * struct wmi_tdls_params - TDLS parameters
 * @vdev_id: vdev id
 * @tdls_state: TDLS state
 * @notification_interval_ms: notification inerval
 * @tx_discovery_threshold: tx discovery threshold
 * @tx_teardown_threshold: tx teardown threashold
 * @rssi_teardown_threshold: RSSI teardown threshold
 * @rssi_delta: RSSI delta
 * @tdls_options: TDLS options
 * @peer_traffic_ind_window: raffic indication window
 * @peer_traffic_response_timeout: traffic response timeout
 * @puapsd_mask: uapsd mask
 * @puapsd_inactivity_time: uapsd inactivity time
 * @puapsd_rx_frame_threshold: uapsd rx frame threshold
 * @teardown_notification_ms: tdls teardown notification interval
 * @tdls_peer_kickout_threshold: tdls packet threshold for
 *    peer kickout operation
 */
struct wmi_tdls_params {
	uint32_t vdev_id;
	uint32_t tdls_state;
	uint32_t notification_interval_ms;
	uint32_t tx_discovery_threshold;
	uint32_t tx_teardown_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
	uint32_t tdls_options;
	uint32_t peer_traffic_ind_window;
	uint32_t peer_traffic_response_timeout;
	uint32_t puapsd_mask;
	uint32_t puapsd_inactivity_time;
	uint32_t puapsd_rx_frame_threshold;
	uint32_t teardown_notification_ms;
	uint32_t tdls_peer_kickout_threshold;
};

/**
 * struct tdls_chan_switch_params - channel switch parameter structure
 * @vdev_id: vdev ID
 * @peer_mac_addr: Peer mac address
 * @tdls_off_ch_bw_offset: Target off-channel bandwitdh offset
 * @tdls_off_ch: Target Off Channel
 * @oper_class: Operating class for target channel
 * @is_responder: Responder or initiator
 */
struct tdls_channel_switch_params {
	uint32_t    vdev_id;
	uint8_t     peer_mac_addr[IEEE80211_ADDR_LEN];
	uint16_t    tdls_off_ch_bw_offset;
	uint8_t     tdls_off_ch;
	uint8_t     tdls_sw_mode;
	uint8_t     oper_class;
	uint8_t     is_responder;
};

/**
 * struct dhcp_offload_info_params - dhcp offload parameters
 * @vdev_id: request data length
 * @dhcpSrvOffloadEnabled: dhcp offload enabled
 * @dhcpClientNum: dhcp client no
 * @dhcpSrvIP: dhcp server ip
 */
struct dhcp_offload_info_params {
	uint32_t vdev_id;
	uint32_t dhcpSrvOffloadEnabled;
	uint32_t dhcpClientNum;
	uint32_t dhcpSrvIP;
};

/**
 * struct nan_req_params - NAN request params
 * @request_data_len: request data length
 * @request_data: request data
 */
struct nan_req_params {
	uint16_t request_data_len;
	uint8_t request_data[];
};


/**
 * struct app_type2_params - app type2parameter
 * @vdev_id: vdev id
 * @rc4_key: rc4 key
 * @rc4_key_len: rc4 key length
 * @ip_id: NC id
 * @ip_device_ip: NC IP addres
 * @ip_server_ip: Push server IP address
 * @tcp_src_port: NC TCP port
 * @tcp_dst_port: Push server TCP port
 * @tcp_seq: tcp sequence
 * @tcp_ack_seq: tcp ack sequence
 * @keepalive_init: Initial ping interval
 * @keepalive_min: Minimum ping interval
 * @keepalive_max: Maximum ping interval
 * @keepalive_inc: Increment of ping interval
 * @gateway_mac: gateway mac address
 * @tcp_tx_timeout_val: tcp tx timeout value
 * @tcp_rx_timeout_val: tcp rx timeout value
 */
struct app_type2_params {
	uint8_t vdev_id;
	uint8_t rc4_key[16];
	uint32_t rc4_key_len;
	/** ip header parameter */
	uint32_t ip_id;
	uint32_t ip_device_ip;
	uint32_t ip_server_ip;
	/** tcp header parameter */
	uint16_t tcp_src_port;
	uint16_t tcp_dst_port;
	uint32_t tcp_seq;
	uint32_t tcp_ack_seq;
	uint32_t keepalive_init;
	uint32_t keepalive_min;
	uint32_t keepalive_max;
	uint32_t keepalive_inc;
	struct qdf_mac_addr gateway_mac;
	uint32_t tcp_tx_timeout_val;
	uint32_t tcp_rx_timeout_val;
};

/**
 * struct app_type1_params - app type1 parameter
 * @vdev_id: vdev id
 * @wakee_mac_addr: mac address
 * @identification_id: identification id
 * @password: password
 * @id_length: id length
 * @pass_length: password length
 */
struct app_type1_params {
	uint8_t vdev_id;
	struct qdf_mac_addr wakee_mac_addr;
	uint8_t identification_id[8];
	uint8_t password[16];
	uint32_t id_length;
	uint32_t pass_length;
};

/**
 * enum wmi_ext_wow_type - wow type
 * @WMI_EXT_WOW_TYPE_APP_TYPE1: only enable wakeup for app type1
 * @WMI_EXT_WOW_TYPE_APP_TYPE2: only enable wakeup for app type2
 * @WMI_EXT_WOW_TYPE_APP_TYPE1_2: enable wakeup for app type1&2
 */
enum wmi_ext_wow_type {
	WMI_EXT_WOW_TYPE_APP_TYPE1,
	WMI_EXT_WOW_TYPE_APP_TYPE2,
	WMI_EXT_WOW_TYPE_APP_TYPE1_2,
};

/**
 * struct ext_wow_params - ext wow parameters
 * @vdev_id: vdev id
 * @type: wow type
 * @wakeup_pin_num: wake up gpio no
 */
struct ext_wow_params {
	uint8_t vdev_id;
	enum wmi_ext_wow_type type;
	uint32_t wakeup_pin_num;
};

/**
 * struct stats_ext_params - ext stats request
 * @vdev_id: vdev id
 * @request_data_len: request data length
 * @request_data: request data
 */
struct stats_ext_params {
	uint32_t vdev_id;
	uint32_t request_data_len;
	uint8_t request_data[];
};

#define WMI_PERIODIC_TX_PTRN_MAX_SIZE 1536
/**
 * struct periodic_tx_pattern - periodic tx pattern
 * @mac_address: MAC Address for the adapter
 * @ucPtrnId: Pattern ID
 * @ucPtrnSize: Pattern size
 * @usPtrnIntervalMs: in ms
 * @ucPattern: Pattern buffer
 */
struct periodic_tx_pattern {
	struct qdf_mac_addr mac_address;
	uint8_t ucPtrnId;
	uint16_t ucPtrnSize;
	uint32_t usPtrnIntervalMs;
	uint8_t ucPattern[WMI_PERIODIC_TX_PTRN_MAX_SIZE];
};

#define WMI_GTK_OFFLOAD_KEK_BYTES       16
#define WMI_GTK_OFFLOAD_KCK_BYTES       16
#define WMI_GTK_OFFLOAD_ENABLE          0
#define WMI_GTK_OFFLOAD_DISABLE         1

/**
 * struct gtk_offload_params - gtk offload parameters
 * @ulFlags: optional flags
 * @aKCK: Key confirmation key
 * @aKEK: key encryption key
 * @ullKeyReplayCounter: replay counter
 * @bssid: bss id
 */
struct gtk_offload_params {
	uint32_t ulFlags;
	uint8_t aKCK[WMI_GTK_OFFLOAD_KCK_BYTES];
	uint8_t aKEK[WMI_GTK_OFFLOAD_KEK_BYTES];
	uint64_t ullKeyReplayCounter;
	struct qdf_mac_addr bssid;
};

/**
 * struct flashing_req_params - led flashing parameter
 * @reqId: request id
 * @pattern_id: pattern identifier. 0: disconnected 1: connected
 * @led_x0: led flashing parameter0
 * @led_x1: led flashing parameter1
 */
struct flashing_req_params {
	uint32_t req_id;
	uint32_t pattern_id;
	uint32_t led_x0;
	uint32_t led_x1;
};

struct wmi_host_mem_chunk {
	uint32_t *vaddr;
	uint32_t paddr;
	qdf_dma_mem_context(memctx);
	uint32_t len;
	uint32_t req_id;
};

struct target_resource_config {
	uint32_t num_vdevs;
	uint32_t num_peers;
	uint32_t num_active_peers;
	uint32_t num_offload_peers;
	uint32_t num_offload_reorder_buffs;
	uint32_t num_peer_keys;
	uint32_t num_tids;
	uint32_t ast_skid_limit;
	uint32_t tx_chain_mask;
	uint32_t rx_chain_mask;
	uint32_t rx_timeout_pri[4];
	uint32_t rx_decap_mode;
	uint32_t scan_max_pending_req;
	uint32_t bmiss_offload_max_vdev;
	uint32_t roam_offload_max_vdev;
	uint32_t roam_offload_max_ap_profiles;
	uint32_t num_mcast_groups;
	uint32_t num_mcast_table_elems;
	uint32_t mcast2ucast_mode;
	uint32_t tx_dbg_log_size;
	uint32_t num_wds_entries;
	uint32_t dma_burst_size;
	uint32_t mac_aggr_delim;
	uint32_t rx_skip_defrag_timeout_dup_detection_check;
	uint32_t vow_config;
	uint32_t gtk_offload_max_vdev;
	uint32_t num_msdu_desc; /* Number of msdu desc */
	uint32_t max_frag_entries;
    /* End common */

	/* Added in MCL */
	uint32_t num_tdls_vdevs;
	uint32_t num_tdls_conn_table_entries;
	uint32_t beacon_tx_offload_max_vdev;
	uint32_t num_multicast_filter_entries;
	uint32_t num_wow_filters;
	uint32_t num_keep_alive_pattern;
	uint32_t keep_alive_pattern_size;
	uint32_t max_tdls_concurrent_sleep_sta;
	uint32_t max_tdls_concurrent_buffer_sta;
	uint32_t wmi_send_separate;
	uint32_t num_ocb_vdevs;
	uint32_t num_ocb_channels;
	uint32_t num_ocb_schedules;
};

/**
 * struct wmi_wifi_start_log - Structure to store the params sent to start/
 * stop logging
 * @name:          Attribute which indicates the type of logging like per packet
 *                 statistics, connectivity etc.
 * @verbose_level: Verbose level which can be 0,1,2,3
 * @flag:          Flag field for future use
 */
struct wmi_wifi_start_log {
	uint32_t ring_id;
	uint32_t verbose_level;
	uint32_t flag;
};

/**
 * struct wmi_pcl_list - Format of PCL
 * @pcl_list: List of preferred channels
 * @pcl_len: Number of channels in the PCL
 */
struct wmi_pcl_list {
	uint8_t pcl_list[128];
	uint32_t pcl_len;
};

/**
 * struct wmi_hw_mode_params - HW mode params
 * @mac0_tx_ss: MAC0 Tx spatial stream
 * @mac0_rx_ss: MAC0 Rx spatial stream
 * @mac1_tx_ss: MAC1 Tx spatial stream
 * @mac1_rx_ss: MAC1 Rx spatial stream
 * @mac0_bw: MAC0 bandwidth
 * @mac1_bw: MAC1 bandwidth
 * @dbs_cap: DBS capabality
 * @agile_dfs_cap: Agile DFS capabality
 */
struct wmi_hw_mode_params {
	uint8_t mac0_tx_ss;
	uint8_t mac0_rx_ss;
	uint8_t mac1_tx_ss;
	uint8_t mac1_rx_ss;
	uint8_t mac0_bw;
	uint8_t mac1_bw;
	uint8_t dbs_cap;
	uint8_t agile_dfs_cap;
};

/**
 * struct wmi_dual_mac_config - Dual MAC configuration
 * @scan_config: Scan configuration
 * @fw_mode_config: FW mode configuration
 * @set_dual_mac_cb: Callback function to be executed on response to the command
 */
struct wmi_dual_mac_config {
	uint32_t scan_config;
	uint32_t fw_mode_config;
	void *set_dual_mac_cb;
};

#ifdef WLAN_NS_OFFLOAD
/**
 * struct ns_offload_req_params - ns offload request paramter
 * @srcIPv6Addr:  src ipv6 address
 * @selfIPv6Addr:  self ipv6 address
 * @targetIPv6Addr: target ipv6 address
 * @self_macaddr: self mac address
 * @srcIPv6AddrValid: src ipv6 address valid flag
 * @targetIPv6AddrValid: target ipv6 address valid flag
 * @slotIdx: slot index
 */
struct ns_offload_req_params {
	uint8_t srcIPv6Addr[WMI_MAC_IPV6_ADDR_LEN];
	uint8_t selfIPv6Addr[WMI_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA][WMI_MAC_IPV6_ADDR_LEN];
	uint8_t targetIPv6Addr[WMI_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA][WMI_MAC_IPV6_ADDR_LEN];
	struct qdf_mac_addr self_macaddr;
	uint8_t srcIPv6AddrValid;
	uint8_t targetIPv6AddrValid[WMI_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA];
	uint8_t slotIdx;
};
#endif /* WLAN_NS_OFFLOAD */

/**
 * struct host_offload_req_param - arp offload parameter
 * @offloadType: offload type
 * @enableOrDisable: enable or disable
 * @num_ns_offload_count: offload count
 */
struct host_offload_req_param {
	uint8_t offloadType;
	uint8_t enableOrDisable;
	uint32_t num_ns_offload_count;
	union {
		uint8_t hostIpv4Addr[WMI_IPV4_ADDR_LEN];
		uint8_t hostIpv6Addr[WMI_MAC_IPV6_ADDR_LEN];
	} params;
#ifdef WLAN_NS_OFFLOAD
	struct ns_offload_req_params nsOffloadInfo;
#endif /* WLAN_NS_OFFLOAD */
	struct qdf_mac_addr bssid;
};

/**
 * struct ssid_hotlist_param - param for SSID Hotlist
 * @ssid: SSID which is being hotlisted
 * @band: Band in which the given SSID should be scanned
 * @rssi_low: Low bound on RSSI
 * @rssi_high: High bound on RSSI
 */
struct ssid_hotlist_param {
	struct mac_ssid ssid;
	uint8_t band;
	int32_t rssi_low;
	int32_t rssi_high;
};

/**
 * struct roam_scan_filter_params - Structure holding roaming scan
 *                                  parameters
 * @len:                      length
 * @op_bitmap:                bitmap to determine reason of roaming
 * @session_id:               vdev id
 * @num_bssid_black_list:     The number of BSSID's that we should
 *                            avoid connecting to. It is like a
 *                            blacklist of BSSID's.
 * @num_ssid_white_list:      The number of SSID profiles that are
 *                            in the Whitelist. When roaming, we
 *                            consider the BSSID's with this SSID
 *                            also for roaming apart from the connected one's
 * @num_bssid_preferred_list: Number of BSSID's which have a preference over
 *                            others
 * @bssid_avoid_list:         Blacklist SSID's
 * @ssid_allowed_list:        Whitelist SSID's
 * @bssid_favored:            Favorable BSSID's
 * @bssid_favored_factor:     RSSI to be added to this BSSID to prefer it
 *
 * This structure holds all the key parameters related to
 * initial connection and roaming connections.
 */

struct roam_scan_filter_params {
	uint32_t len;
	uint32_t op_bitmap;
	uint8_t session_id;
	uint32_t num_bssid_black_list;
	uint32_t num_ssid_white_list;
	uint32_t num_bssid_preferred_list;
	struct qdf_mac_addr bssid_avoid_list[MAX_BSSID_AVOID_LIST];
	struct mac_ssid ssid_allowed_list[MAX_SSID_ALLOWED_LIST];
	struct qdf_mac_addr bssid_favored[MAX_BSSID_FAVORED];
	uint8_t bssid_favored_factor[MAX_BSSID_FAVORED];
};

/**
 * struct ssid_hotlist_request_params - set SSID hotlist request struct
 * @request_id: ID of the request
 * @session_id: ID of the session
 * @lost_ssid_sample_size: Number of consecutive scans in which the SSID
 *	must not be seen in order to consider the SSID "lost"
 * @ssid_count: Number of valid entries in the @ssids array
 * @ssids: Array that defines the SSIDs that are in the hotlist
 */
struct ssid_hotlist_request_params {
	uint32_t request_id;
	uint8_t session_id;
	uint32_t lost_ssid_sample_size;
	uint32_t ssid_count;
	struct ssid_hotlist_param ssids[WMI_EXTSCAN_MAX_HOTLIST_SSIDS];
};

/**
 * struct wmi_unit_test_cmd - unit test command parameters
 * @vdev_id: vdev id
 * @module_id: module id
 * @num_args: number of arguments
 * @args: arguments
 */
struct wmi_unit_test_cmd {
	uint32_t vdev_id;
	WLAN_MODULE_ID module_id;
	uint32_t num_args;
	uint32_t args[WMI_MAX_NUM_ARGS];
};

/**
 * struct wmi_roam_invoke_cmd - roam invoke command
 * @vdev_id: vdev id
 * @bssid: mac address
 * @channel: channel
 */
struct wmi_roam_invoke_cmd {
	uint32_t vdev_id;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint32_t channel;
};

/**
 * struct ext_scan_setbssi_hotlist_params - set hotlist request
 * @requestId: request identifier
 * @sessionId: session identifier
 * @lost_ap_sample_size: number of samples to confirm AP loss
 * @numAp: Number of hotlist APs
 * @ap: hotlist APs
 */
struct ext_scan_setbssi_hotlist_params {
	uint32_t  requestId;
	uint8_t   sessionId;

	uint32_t  lost_ap_sample_size;
	uint32_t  numAp;
	struct ap_threshold_params ap[WMI_WLAN_EXTSCAN_MAX_HOTLIST_APS];
};
#endif /* _WMI_UNIFIED_PARAM_H_ */

