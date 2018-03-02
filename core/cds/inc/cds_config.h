/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#include "osdep.h"
#include "cdp_txrx_mob_def.h"
#include "wlan_pmo_common_public_struct.h"

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
 * enum active_bpf_mode - the modes active BPF can operate in
 * @ACTIVE_BPF_DISABLED: BPF is disabled in active mode
 * @ACTIVE_BPF_ENABLED: BPF is enabled for all packets
 * @ACTIVE_BPF_ADAPTIVE: BPF is enabled for packets up to some threshold
 * @ACTIVE_BPF_MODE_COUNT: The number of active BPF modes
 */
enum active_bpf_mode {
	ACTIVE_BPF_DISABLED = 0,
	ACTIVE_BPF_ENABLED,
	ACTIVE_BPF_ADAPTIVE,
	ACTIVE_BPF_MODE_COUNT
};

/**
 * struct cds_config_info - Place Holder for cds configuration
 * @max_station: Max station supported
 * @max_bssid: Max Bssid Supported
 * @powersave_offload_enabled: Indicate if powersave offload is enabled
 * @sta_maxlimod_dtim: station max listen interval
 * @sta_mod_dtim: station mode DTIM
 * @sta_dynamic_dtim: station dynamic DTIM
 * @driver_type: Enumeration of Driver Type whether FTM or Mission mode
 * @wow_enable: Indicate whether wow is enabled or not
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
 * @is_fw_timeout: Indicate whether crash host when fw timesout or not
 * @active_uc_bpf_mode: Setting that determines how BPF is applied in active
 *	mode for uc packets
 * @active_mc_bc_bpf_mode: Setting that determines how BPF is applied in
 *	active mode for MC/BC packets
 * @auto_power_save_fail_mode: auto detect power save failure mode
 * @ito_repeat_count: Indicates ito repeated count
 * @force_target_assert_enabled: Indicate whether target assert enabled or not
 * @bandcapability: Configured band by user
 * @rps_enabled: RPS enabled in SAP mode
 * @delay_before_vdev_stop: wait time for tx complete before vdev stop
 * Structure for holding cds ini parameters.
 */

struct cds_config_info {
	uint16_t max_station;
	uint16_t max_bssid;
	uint8_t powersave_offload_enabled;
	uint8_t sta_maxlimod_dtim;
	uint8_t sta_mod_dtim;
	uint8_t sta_dynamic_dtim;
	enum qdf_driver_type driver_type;
	uint8_t wow_enable;
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
	uint8_t max_msdus_per_rxinorderind;
	bool self_recovery_enabled;
	bool fw_timeout_crash;
	struct ol_tx_sched_wrr_ac_specs_t ac_specs[TX_WMM_AC_NUM];
	enum active_bpf_mode active_uc_bpf_mode;
	enum active_bpf_mode active_mc_bc_bpf_mode;
	enum pmo_auto_pwr_detect_failure_mode auto_power_save_fail_mode;
	uint8_t ito_repeat_count;
	bool force_target_assert_enabled;
	uint8_t bandcapability;
	bool rps_enabled;
	uint8_t delay_before_vdev_stop;
};

#ifdef WLAN_FEATURE_FILS_SK
#define MAX_PMK_LEN 48
#define MAX_PMKID_LEN 16
#define FILS_MAX_KEYNAME_NAI_LENGTH 253
#define FILS_MAX_REALM_LEN 255
#define FILS_MAX_RRK_LENGTH 64
#define FILS_MAX_RIK_LENGTH FILS_MAX_RRK_LENGTH

struct cds_fils_connection_info {
	bool is_fils_connection;
	uint8_t keyname_nai[FILS_MAX_KEYNAME_NAI_LENGTH];
	uint32_t key_nai_length;
	uint16_t sequence_number;
	uint8_t r_rk[FILS_MAX_RRK_LENGTH];
	uint32_t r_rk_length;
	uint8_t realm[FILS_MAX_REALM_LEN];
	uint32_t realm_len;
	uint8_t akm_type;
	uint8_t auth_type;
	uint8_t pmk[MAX_PMK_LEN];
	uint8_t pmk_len;
	uint8_t pmkid[16];
};
#endif
#endif /* !defined( __CDS_CONFIG_H ) */
