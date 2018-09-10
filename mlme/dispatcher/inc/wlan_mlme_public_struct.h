/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains mlme structure definations
 */

#ifndef _WLAN_MLME_STRUCT_H_
#define _WLAN_MLME_STRUCT_H_

#include <wlan_cmn.h>

#define CFG_PMKID_MODES_OKC                        (0x1)
#define CFG_PMKID_MODES_PMKSA_CACHING              (0x2)

/**
 * struct mlme_ht_capabilities_info - HT Capabilities Info
 * @l_sig_tx_op_protection: L-SIG TXOP Protection Mechanism support
 * @stbc_control_frame: STBC Control frame support
 * @psmp: PSMP Support
 * @dsss_cck_mode_40_mhz: To indicate use of DSSS/CCK in 40Mhz
 * @maximal_amsdu_size: Maximum AMSDU Size - 0:3839 octes, 1:7935 octets
 * @delayed_ba: Support of Delayed Block Ack
 * @rx_stbc: Rx STBC Support - 0:Not Supported, 1: 1SS, 2: 1,2SS, 3: 1,2,3SS
 * @tx_stbc: Tx STBC Support
 * @short_gi_40_mhz: Short GI Support for HT40
 * @short_gi_20_mhz: Short GI support for HT20
 * @green_field: Support for HT Greenfield PPDUs
 * @mimo_power_save: SM Power Save Mode - 0:Static, 1:Dynamic, 3:Disabled, 2:Res
 * @supported_channel_width_set: Supported Chan Width - 0:20Mhz, 1:20Mhz & 40Mhz
 * @adv_coding_cap: Rx LDPC support
 */
#ifndef ANI_LITTLE_BIT_ENDIAN
struct mlme_ht_capabilities_info {
	uint16_t l_sig_tx_op_protection:1;
	uint16_t stbc_control_frame:1;
	uint16_t psmp:1;
	uint16_t dsss_cck_mode_40_mhz:1;
	uint16_t maximal_amsdu_size:1;
	uint16_t delayed_ba:1;
	uint16_t rx_stbc:2;
	uint16_t tx_stbc:1;
	uint16_t short_gi_40_mhz:1;
	uint16_t short_gi_20_mhz:1;
	uint16_t green_field:1;
	uint16_t mimo_power_save:2;
	uint16_t supported_channel_width_set:1;
	uint16_t adv_coding_cap:1;
} qdf_packed;
#else
struct mlme_ht_capabilities_info {
	uint16_t adv_coding_cap:1;
	uint16_t supported_channel_width_set:1;
	uint16_t mimo_power_save:2;
	uint16_t green_field:1;
	uint16_t short_gi_20_mhz:1;
	uint16_t short_gi_40_mhz:1;
	uint16_t tx_stbc:1;
	uint16_t rx_stbc:2;
	uint16_t delayed_ba:1;
	uint16_t maximal_amsdu_size:1;
	uint16_t dsss_cck_mode_40_mhz:1;
	uint16_t psmp:1;
	uint16_t stbc_control_frame:1;
	uint16_t l_sig_tx_op_protection:1;
} qdf_packed;
#endif

/**
 * struct wlan_mlme_ht_caps - HT Capabilities related config items
 * @ht_cap_info: HT capabilities Info Structure
 */
struct wlan_mlme_ht_caps {
	struct mlme_ht_capabilities_info ht_cap_info;
};

/**
 * struct wlan_mlme_ - HT Capabilities related config items
 * @ht_cap_info: HT capabilities Info Structure
 */
struct wlan_mlme_cfg_sap {
	uint8_t cfg_ssid[32];
	uint16_t beacon_interval;
	uint16_t dtim_interval;
	uint16_t listen_interval;
	bool sap_11g_policy;
	uint8_t assoc_sta_limit;
	bool enable_lte_coex;
	uint16_t rmc_action_period_freq;
	uint8_t rate_tx_mgmt;
	uint8_t rate_tx_mgmt_2g;
	uint8_t rate_tx_mgmt_5g;
	bool tele_bcn_wakeup_en;
	uint8_t tele_bcn_max_li;
	bool sap_get_peer_info;
	bool sap_allow_all_chan_param_name;
	uint8_t sap_max_no_peers;
	uint8_t sap_max_offload_peers;
	uint8_t sap_max_offload_reorder_buffs;
	uint8_t sap_ch_switch_beacon_cnt;
	bool sap_ch_switch_mode;
	bool sap_internal_restart_name;
	bool chan_switch_hostapd_rate_enabled_name;
	uint8_t reduced_beacon_interval;
};

struct wlan_mlme_vht_caps {
	/* VHT related configs */
};

/**
 * struct wlan_mlme_qos - QOS TX/RX aggregation related CFG items
 * @tx_aggregation_size: TX aggr size in number of MPDUs
 * @tx_aggregation_size_be: No. of MPDUs for BE queue for TX aggr
 * @tx_aggregation_size_bk: No. of MPDUs for BK queue for TX aggr
 * @tx_aggregation_size_vi: No. of MPDUs for VI queue for TX aggr
 * @tx_aggregation_size_vo: No. of MPDUs for VO queue for TX aggr
 * @rx_aggregation_size: No. of MPDUs for RX aggr
 * @tx_aggr_sw_retry_threshold_be: Tx aggregation sw retry for BE
 * @tx_aggr_sw_retry_threshold_bk: Tx aggregation sw retry for BK
 * @tx_aggr_sw_retry_threshold_vi: Tx aggregation sw retry for VI
 * @tx_aggr_sw_retry_threshold_vo: Tx aggregation sw retry for VO
 * @sap_max_inactivity_override: Override updating ap_sta_inactivity from
 * hostapd.conf
 */
struct wlan_mlme_qos {
	uint32_t tx_aggregation_size;
	uint32_t tx_aggregation_size_be;
	uint32_t tx_aggregation_size_bk;
	uint32_t tx_aggregation_size_vi;
	uint32_t tx_aggregation_size_vo;
	uint32_t rx_aggregation_size;
	uint32_t tx_aggr_sw_retry_threshold_be;
	uint32_t tx_aggr_sw_retry_threshold_bk;
	uint32_t tx_aggr_sw_retry_threshold_vi;
	uint32_t tx_aggr_sw_retry_threshold_vo;
	bool sap_max_inactivity_override;
};

/**
 * struct wlan_mlme_rates - RATES related config items
 * @cfpPeriod: cfp period info
 * @cfpMaxDuration: cfp Max duration info
 * @max_htmcs_txdata: max HT mcs info for Tx
 * @disable_abg_rate_txdata: disable abg rate info for tx data
 * @sap_max_mcs_txdata: sap max mcs info
 * @disable_high_ht_mcs_2x2: disable high mcs for 2x2 info
 */
struct wlan_mlme_rates {
	uint8_t cfp_period;
	uint16_t cfp_max_duration;
	uint16_t max_htmcs_txdata;
	bool disable_abg_rate_txdata;
	uint16_t sap_max_mcs_txdata;
	uint8_t disable_high_ht_mcs_2x2;
};

/*
 * struct wlan_mlme_sap_protection_cfg - SAP erp protection config items
 *
 * @protection_enabled - Force enable protection. static via cfg
 * @protection_force_policy - Protection force policy. Static via cfg
 * @ignore_peer_ht_mode - ignore the ht opmode of the peer. Dynamic via INI.
 *
 */
struct wlan_mlme_sap_protection {
	uint32_t protection_enabled;
	uint8_t protection_force_policy;
	bool ignore_peer_ht_mode;
};

/*
 * struct wlan_mlme_chainmask - All chainmask related cfg items
 *
 * @txchainmask1x1 - to set transmit chainmask
 * @rxchainmask1x1 - to set rx chainmask
 * @tx_chain_mask_cck - Used to enable/disable Cck ChainMask
 * @tx_chain_mask_1ss - Enables/disables tx chain Mask1ss
 * @num_11b_tx_chains - Number of Tx Chains in 11b mode
 * @num_11ag_tx_chains - Number of Tx Chains in 11ag mode
 * @tx_chain_mask_2g - tx chain mask for 2g
 * @rx_chain_mask_2g - rx chain mask for 2g
 * @tx_chain_mask_5g - tx chain mask for 5g
 * @rx_chain_mask_5g - rx chain mask for 5g
 */
struct wlan_mlme_chainmask {
	uint8_t txchainmask1x1;
	uint8_t rxchainmask1x1;
	bool tx_chain_mask_cck;
	uint8_t tx_chain_mask_1ss;
	uint16_t num_11b_tx_chains;
	uint16_t num_11ag_tx_chains;
	uint8_t tx_chain_mask_2g;
	uint8_t rx_chain_mask_2g;
	uint8_t tx_chain_mask_5g;
	uint8_t rx_chain_mask_5g;
};

/**
 * struct wlan_mlme_obss_ht40 - OBSS HT40 config items
 * @active_dwelltime:        obss active dwelltime
 * @passive_dwelltime:       obss passive dwelltime
 * @width_trigger_interval:  obss trigger interval
 * @passive_per_channel:     obss scan passive total duration per channel
 * @active_per_channel:      obss scan active total duration per channel
 * @width_trans_delay:       obss width transition delay
 * @scan_activity_threshold: obss scan activity threshold
 */
struct wlan_mlme_obss_ht40 {
	uint32_t active_dwelltime;
	uint32_t passive_dwelltime;
	uint32_t width_trigger_interval;
	uint32_t passive_per_channel;
	uint32_t active_per_channel;
	uint32_t width_trans_delay;
	uint32_t scan_activity_threshold;
};

/**
 * enum dot11p_mode - The 802.11p mode of operation
 * @WLAN_HDD_11P_DISABLED:   802.11p mode is disabled
 * @WLAN_HDD_11P_STANDALONE: 802.11p-only operation
 * @WLAN_HDD_11P_CONCURRENT: 802.11p and WLAN operate concurrently
 */
enum dot11p_mode {
	CFG_11P_DISABLED = 0,
	CFG_11P_STANDALONE,
	CFG_11P_CONCURRENT,
};

/**
 * struct wlan_mlme_sta_cfg - MLME STA configuration items
 * @sta_keep_alive_period:          Sends NULL frame to AP period
 * @tgt_gtx_usr_cfg:                Target gtx user config
 * @pmkid_modes:                    Enable PMKID modes
 * @wait_cnf_timeout:               Wait assoc cnf timeout
 * @dot11p_mode:                    Set 802.11p mode
 * @fils_max_chan_guard_time:       Set maximum channel guard time
 * @current_rssi:                   Current rssi
 * @ignore_peer_erp_info:           Ignore peer infrormation
 * @sta_prefer_80mhz_over_160mhz:   Set Sta preference to connect in 80HZ/160HZ
 * @enable_5g_ebt:                  Set default 5G early beacon termination
 * @deauth_before_connection:       Send deauth before connection or not
 * @enable_go_cts2self_for_sta:     Stop NOA and start using cts2self
 * @qcn_ie_support:                 QCN IE support
 * @force_rsne_override:            Force rsnie override from user
 * @single_tid:                     Set replay counter for all TID
 */
struct wlan_mlme_sta_cfg {
	uint32_t sta_keep_alive_period;
	uint32_t tgt_gtx_usr_cfg;
	uint32_t pmkid_modes;
	uint32_t wait_cnf_timeout;
	uint8_t dot11p_mode;
	uint8_t fils_max_chan_guard_time;
	uint8_t current_rssi;
	bool ignore_peer_erp_info;
	bool sta_prefer_80mhz_over_160mhz;
	bool enable_5g_ebt;
	bool deauth_before_connection;
	bool enable_go_cts2self_for_sta;
	bool qcn_ie_support;
	bool force_rsne_override;
	bool single_tid;
};

/**
 * struct wlan_mlme_cfg - MLME config items
 * @ht_cfg: HT related CFG Items
 * @obss_ht40:obss ht40 CFG Items
 * @vht_cfg: VHT related CFG Items
 * @rates: Rates related cfg items
 * @sap_protection_cfg: SAP erp protection related CFG items
 * @sta: sta CFG Items
 */
struct wlan_mlme_cfg {
	struct wlan_mlme_ht_caps ht_caps;
	struct wlan_mlme_obss_ht40 obss_ht40;
	struct wlan_mlme_vht_caps vht_caps;
	struct wlan_mlme_qos qos_mlme_params;
	struct wlan_mlme_rates rates;
	struct wlan_mlme_sap_protection sap_protection_cfg;
	struct wlan_mlme_chainmask chainmask_cfg;
	struct wlan_mlme_cfg_sap sap_cfg;
	struct wlan_mlme_sta_cfg sta;
};

#endif
