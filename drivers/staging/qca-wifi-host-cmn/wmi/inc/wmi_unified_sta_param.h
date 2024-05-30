/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * This file contains the API definitions for the STA WMI APIs.
 */

#ifndef _WMI_UNIFIED_STA_PARAM_H_
#define _WMI_UNIFIED_STA_PARAM_H_

/**
 * struct sta_keep_alive_params - sta keep alive parameters
 * @vdev_id: vdev id
 * @method: keep alive method
 * @timeperiod: time to keep alive
 * @hostv4addr: host ipv4 address
 * @destv4addr: destination ipv4 address
 * @destmac: destination mac address
 */
struct sta_keep_alive_params {
	uint8_t vdev_id;
	uint32_t method;
	uint32_t timeperiod;
	uint8_t hostv4addr[QDF_IPV4_ADDR_SIZE];
	uint8_t destv4addr[QDF_IPV4_ADDR_SIZE];
	uint8_t destmac[QDF_MAC_ADDR_SIZE];
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
 * struct wlm_latency_level_param - WLM parameters
 * @wlm_latency_level: wlm latency level to set
 *  0 - normal, 1 - moderate, 2 - low, 3 - ultralow
 * @wlm_latency_flags: wlm latency flags to set
 *  |31  12|  11  |  10  |9    8|7    6|5    4|3    2|  1  |  0  |
 *  +------+------+------+------+------+------+------+-----+-----+
 *  | RSVD | SSLP | CSLP | RSVD | Roam | RSVD | DWLT | DFS | SUP |
 *  +------+-------------+-------------+-------------------------+
 *  |  WAL |      PS     |     Roam    |         Scan            |
 *
 *  bit 0: Avoid scan request from HLOS if setting
 *  bit 1: Skip DFS channel SCAN if setting
 *  bit 2-3: Define policy of dwell time/duration for each foreign channel
 *     (b2 b3)
 *     (0  0 ): Default scan dwell time
 *     (0  1 ): Reserve
 *     (1  0 ): Shrink off channel dwell time
 *     (1  1 ): Reserve
 *  bit 4-5: Reserve for scan
 *  bit 6-7: Define roaming policy
 *     (b6 b7)
 *     (0  0 ): Default roaming behavior, allow roaming in all scenarios
 *     (0  1 ): Disallow all roaming
 *     (1  0 ): Allow roaming when final bmissed
 *     (1  1 ): Reserve
 *  bit 8-9: Reserve for roaming
 *  bit 10: Disable css power collapse if setting
 *  bit 11: Disable sys sleep if setting
 *  bit 12-31: Reserve for future useage
 * @vdev_id: vdev id
 * @force_reset: bit 0 used as force reset to override the latency level as
 * default for all the wlm clients
 * @client_id_bitmask: clients of WLM Arbiter
 */
struct wlm_latency_level_param {
	uint16_t wlm_latency_level;
	uint32_t wlm_latency_flags;
	uint16_t vdev_id;
#ifdef MULTI_CLIENT_LL_SUPPORT
	uint32_t force_reset;
	uint32_t client_id_bitmask;
#endif
};

#define WMI_2_4_GHZ_MAX_FREQ  3000

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
	uint32_t ie_source;
	uint32_t band;
	uint8_t *data;
};

#define MAX_SAR_LIMIT_ROWS_SUPPORTED 64
/**
 * struct sar_limit_cmd_row - sar limits row
 * @band_id: Optional param for frequency band
 *           See %enum wmi_sar_band_id_flags for possible values
 * @chain_id: Optional param for antenna chain id
 * @mod_id: Optional param for modulation scheme
 *          See %enum wmi_sar_mod_id_flags for possible values
 * @limit_value: Mandatory param providing power limits in steps of 0.5 dbm
 * @validity_bitmap: bitmap of valid optional params in sar_limit_cmd_row struct
 *                   See WMI_SAR_*_VALID_MASK for possible values
 */
struct sar_limit_cmd_row {
	uint32_t band_id;
	uint32_t chain_id;
	uint32_t mod_id;
	uint32_t limit_value;
	uint32_t validity_bitmap;
};

/**
 * struct sar_limit_cmd_params - sar limits params
 * @sar_enable: flag to enable SAR
 *              See %enum wmi_sar_feature_state_flags for possible values
 * @num_limit_rows: number of items in sar_limits
 * @commit_limits: indicates firmware to start apply new SAR values
 * @sar_limit_row_list: pointer to array of sar limit rows
 */
struct sar_limit_cmd_params {
	uint32_t sar_enable;
	uint32_t num_limit_rows;
	uint32_t commit_limits;
	struct sar_limit_cmd_row *sar_limit_row_list;
};

/**
 * struct sar_limit_event_row - sar limits row
 * @band_id: Frequency band.
 *           See %enum wmi_sar_band_id_flags for possible values
 * @chain_id: Chain id
 * @mod_id: Modulation scheme
 *          See %enum wmi_sar_mod_id_flags for possible values
 * @limit_value: Power limits in steps of 0.5 dbm that is currently active for
 *     the given @band_id, @chain_id, and @mod_id
 */
struct sar_limit_event_row {
	uint32_t band_id;
	uint32_t chain_id;
	uint32_t mod_id;
	uint32_t limit_value;
};

/**
 * struct sar_limit_event - sar limits params
 * @sar_enable: Current status of SAR enablement.
 *              See %enum wmi_sar_feature_state_flags for possible values
 * @num_limit_rows: number of items in sar_limits
 * @sar_limit_row: array of sar limit rows. Only @num_limit_rows
 *                 should be considered valid.
 */
struct sar_limit_event {
	uint32_t sar_enable;
	uint32_t num_limit_rows;
	struct sar_limit_event_row
			sar_limit_row[MAX_SAR_LIMIT_ROWS_SUPPORTED];
};

#define WMI_UNIFIED_MAX_PMKID_LEN   16
#define WMI_UNIFIED_MAX_PMK_LEN     64

/**
 * struct wmi_unified_pmk_cache - used to set del pmkid cache
 * @vdev_id: ID of the vdev being configured
 * @pmk_len: PMK len
 *	for big-endian hosts, manual endian conversion will be needed to keep
 *	the array values in their original order in spite of the automatic
 *	byte-swap applied to WMI messages during download
 * @pmk: PMK array
 * @pmkid_len: PMK ID Len
 * @pmkid: PMK ID Array
 * @bssid: BSSID
 * @ssid: SSID
 * @cache_id: PMK Cache ID
 * @cat_flag: whether (bssid) or (ssid,cache_id) is valid
 * @action_flag: add/delete the entry
 * @is_flush_all: FLAG to indicate PMKSA flush. True if PMKSA cache flush is
 * needed.
 */
struct wmi_unified_pmk_cache {
	uint8_t             vdev_id;
	uint32_t            pmk_len;
	uint8_t             pmk[WMI_UNIFIED_MAX_PMK_LEN];
	uint32_t            pmkid_len;
	uint8_t             pmkid[WMI_UNIFIED_MAX_PMKID_LEN];
	wmi_host_mac_addr   bssid;
	struct wlan_ssid    ssid;
	uint32_t            cache_id;
	uint32_t            cat_flag;
	uint32_t            action_flag;
	bool                is_flush_all;
};

#define WMI_QOS_NUM_AC_MAX 4

/**
 * struct aggr_add_ts_param - ADDTS parameters
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: CDF status
 * @sessionId: session id
 * @vdev_id: vdev id
 */
struct aggr_add_ts_param {
	uint16_t tspecIdx;
	struct mac_tspec_ie tspec[WMI_QOS_NUM_AC_MAX];
	QDF_STATUS status[WMI_QOS_NUM_AC_MAX];
	uint8_t sessionId;
	uint8_t vdev_id;
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

/**
 * struct wmi_pcl_chan_weights - Params to get the valid weighed list
 * @pcl_list: channel freq list sorted in preferred order
 * @pcl_len: Length of the PCL
 * @saved_chan_list: Valid channel freq list updated as part of
 * WMA_UPDATE_CHAN_LIST_REQ
 * @saved_num_chan: Length of the valid channel list
 * @weighed_valid_list: Weights of the valid channel list. This will have one
 * to one mapping with valid_chan_list. FW expects channel order and size to be
 * as per the list provided in WMI_SCAN_CHAN_LIST_CMDID.
 * @weight_list: Weights assigned by policy manager
 */
struct wmi_pcl_chan_weights {
	uint32_t pcl_list[NUM_CHANNELS];
	uint32_t pcl_len;
	uint32_t saved_chan_list[NUM_CHANNELS];
	uint32_t saved_num_chan;
	uint8_t weighed_valid_list[NUM_CHANNELS];
	uint8_t weight_list[NUM_CHANNELS];
};

/**
 * struct wmi_adaptive_dwelltime_params - the adaptive dwelltime params
 * @vdev_id: vdev id
 * @is_enabled: Adaptive dwell time is enabled/disabled
 * @dwelltime_mode: global default adaptive dwell mode
 * @lpf_weight: weight to calculate the average low pass
 * filter for channel congestion
 * @passive_mon_intval: intval to monitor wifi activity in passive scan in msec
 * @wifi_act_threshold: % of wifi activity used in passive scan 0-100
 *
 */
struct wmi_adaptive_dwelltime_params {
	uint32_t vdev_id;
	bool is_enabled;
	enum scan_dwelltime_adaptive_mode dwelltime_mode;
	uint8_t lpf_weight;
	uint8_t passive_mon_intval;
	uint8_t wifi_act_threshold;
};

#define WMI_SCAN_CLIENT_MAX        7

/**
 * struct wmi_dbs_scan_sel_params - DBS scan selection params
 * @num_clients: Number of scan clients dutycycle
 * @pdev_id: pdev_id for identifying the MAC
 * @module_id: scan client module id
 * @num_dbs_scans: number of DBS scans
 * @num_non_dbs_scans: number of non-DBS scans
 */
struct wmi_dbs_scan_sel_params {
	uint32_t num_clients;
	uint32_t pdev_id;
	uint32_t module_id[WMI_SCAN_CLIENT_MAX];
	uint32_t num_dbs_scans[WMI_SCAN_CLIENT_MAX];
	uint32_t num_non_dbs_scans[WMI_SCAN_CLIENT_MAX];
};

/**
 * struct set_arp_stats - set/reset arp stats
 * @vdev_id: session id
 * @flag: enable/disable stats
 * @pkt_type: type of packet(1 - arp)
 * @ip_addr: subnet ipv4 address in case of encrypted packets
 * @pkt_type_bitmap: pkt bitmap
 * @tcp_src_port: tcp src port for pkt tracking
 * @tcp_dst_port: tcp dst port for pkt tracking
 * @icmp_ipv4: target ipv4 address to track ping packets
 * @reserved: reserved
 */
struct set_arp_stats {
	uint32_t vdev_id;
	uint8_t flag;
	uint8_t pkt_type;
	uint32_t ip_addr;
	uint32_t pkt_type_bitmap;
	uint32_t tcp_src_port;
	uint32_t tcp_dst_port;
	uint32_t icmp_ipv4;
	uint32_t reserved;
};

/**
 * struct get_arp_stats - get arp stats from firmware
 * @pkt_type: packet type(1 - ARP)
 * @vdev_id: session id
 */
struct get_arp_stats {
	uint8_t pkt_type;
	uint32_t vdev_id;
};

#endif /* _WMI_UNIFIED_STA_PARAM_H_ */
