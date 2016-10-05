/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 * DOC: cds_config.h
 *
 * Defines the configuration Information for various modules. Default values
 * are read from the INI file and saved into cds_config_info which are passed
 * to various modules for the initialization.
 */

#if !defined(__CDS_CONFIG_H)
#define __CDS_CONFIG_H

/**
 * enum driver_type - Indicate the driver type to the cds, and based on this
 * do appropriate initialization.
 *
 * @DRIVER_TYPE_PRODUCTION: Driver used in the production
 * @DRIVER_TYPE_MFG: Driver used in the Factory
 *
 */
enum driver_type {
	DRIVER_TYPE_PRODUCTION = 0,
	DRIVER_TYPE_MFG = 1,
};

/**
 * enum cfg_sub_20_channel_width: ini values for su 20 mhz channel width
 * @WLAN_SUB_20_CH_WIDTH_5: Use 5 mhz channel width
 * @WLAN_SUB_20_CH_WIDTH_10: Use 10 mhz channel width
 */
enum cfg_sub_20_channel_width {
	WLAN_SUB_20_CH_WIDTH_NONE = 0,
	WLAN_SUB_20_CH_WIDTH_5 = 1,
	WLAN_SUB_20_CH_WIDTH_10 = 2,
};

/**
 * struct cds_config_info - Place Holder for cds configuration
 * @max_station: Max station supported
 * @max_bssid: Max Bssid Supported
 * @frame_xln_reqd: frame transmission required
 * @powersave_offload_enabled: Indicate if powersave offload is enabled
 * @sta_maxlimod_dtim: station max listen interval
 * @sta_mod_dtim: station mode DTIM
 * @sta_dynamic_dtim: station dynamic DTIM
 * @driver_type: Enumeration of Driver Type whether FTM or Mission mode
 * @max_wow_filters: Max wow filters to be configured to fw
 * @wow_enable: Indicate whether wow is enabled or not
 * @ol_ini_info: Status of offload enabled from ini 1st bit for arm,2nd for NS
 * currently rest of bits are not used
 * @ssdp: Indicate ssdp is enabled or not
 * @enable_mc_list : To Check if Multicast list filtering is enabled in FW
 * @dfs_phyerr_filter_offload: DFS Phyerror Filtering offload status from ini
 * Indicates whether support is enabled or not
 * @ap_disable_intrabss_fwd: pass intra-bss-fwd info to txrx module
 * @ap_maxoffload_peers: max offload peer
 * @ap_maxoffload_reorderbuffs: max offload reorder buffs
 * @ra_ratelimit_interval: RA rate limit value
 * @is_ra_ratelimit_enabled: Indicate RA rate limit enabled or not
 * @reorder_offload: is RX re-ordering offloaded to the fw
 * @dfs_pri_multiplier: dfs radar pri multiplier
 * @uc_offload_enabled: IPA Micro controller data path offload enable flag
 * @uc_txbuf_count: IPA Micro controller data path offload TX buffer count
 * @uc_txbuf_size: IPA Micro controller data path offload TX buffer size
 * @uc_rxind_ringcount: IPA Micro controller data path offload RX indication
 * ring count
 * @uc_tx_partition_base: IPA Micro controller datapath offload TX partition
 * base
 * @enable_rxthread: Rx processing in thread from TXRX
 * @ip_tcp_udp_checksum_offload: checksum offload enabled or not
 * @ce_classify_enabled: CE based classification enabled
 * @max_scan: Maximum number of parallel scans
 * @tx_flow_stop_queue_th: Threshold to stop queue in percentage
 * @tx_flow_start_queue_offset: Start queue offset in percentage
 * @is_lpass_enabled: Indicate whether LPASS is enabled or not
 * @is_nan_enabled: Indicate whether NAN is enabled or not
 * @bool bpf_packet_filter_enable; Indicate bpf filter enabled or not
 * @tx_chain_mask_cck: Tx chain mask enabled or not
 * @self_gen_frm_pwr: Self gen from power
 * @sub_20_channel_width: Sub 20 MHz ch width, ini intersected with fw cap
 * @flow_steering_enabled: Receive flow steering.
 * Structure for holding cds ini parameters.
 */

struct cds_config_info {
	uint16_t max_station;
	uint16_t max_bssid;
	uint32_t frame_xln_reqd;
	uint8_t powersave_offload_enabled;
	uint8_t sta_maxlimod_dtim;
	uint8_t sta_mod_dtim;
	uint8_t sta_dynamic_dtim;
	enum driver_type driver_type;
	uint8_t max_wow_filters;
	uint8_t wow_enable;
	uint8_t ol_ini_info;
	bool ssdp;
	bool enable_mc_list;
	uint8_t dfs_phyerr_filter_offload;
	uint8_t ap_disable_intrabss_fwd;
	uint8_t ap_maxoffload_peers;
	uint8_t ap_maxoffload_reorderbuffs;
#ifdef FEATURE_WLAN_RA_FILTERING
	uint16_t ra_ratelimit_interval;
	bool is_ra_ratelimit_enabled;
#endif
	uint8_t reorder_offload;
	int32_t dfs_pri_multiplier;
	uint8_t uc_offload_enabled;
	uint32_t uc_txbuf_count;
	uint32_t uc_txbuf_size;
	uint32_t uc_rxind_ringcount;
	uint32_t uc_tx_partition_base;
	bool enable_rxthread;
	bool ip_tcp_udp_checksum_offload;
	bool ce_classify_enabled;
	uint8_t max_scan;
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	uint32_t tx_flow_stop_queue_th;
	uint32_t tx_flow_start_queue_offset;
#endif
#ifdef WLAN_FEATURE_LPSS
	bool is_lpass_enabled;
#endif
#ifdef WLAN_FEATURE_NAN
	bool is_nan_enabled;
#endif
	bool bpf_packet_filter_enable;
	bool tx_chain_mask_cck;
	uint16_t self_gen_frm_pwr;
	enum cfg_sub_20_channel_width sub_20_channel_width;
	bool flow_steering_enabled;
};
#endif /* !defined( __CDS_CONFIG_H ) */
