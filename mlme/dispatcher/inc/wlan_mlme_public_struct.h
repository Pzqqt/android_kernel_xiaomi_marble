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
#include <reg_services_public_struct.h>

#define CFG_PMKID_MODES_OKC                        (0x1)
#define CFG_PMKID_MODES_PMKSA_CACHING              (0x2)

#define CFG_STR_DATA_LEN     17
#define CFG_EDCA_DATA_LEN    17

/**
 * struct mlme_cfg_str - generic structure for all mlme CFG string items
 *
 * @max_len: maximum data length allowed
 * @len: valid no. of elements of the data
 * @data: uint8_t array to store values
 */
struct mlme_cfg_str {
	qdf_size_t max_len;
	qdf_size_t len;
	uint8_t data[CFG_STR_DATA_LEN];
};

/**
 * enum e_edca_type - to index edca params for edca profile
 *			 EDCA profile   AC   unicast/bcast
 * @edca_ani_acbe_local:    ani          BE      unicast
 * @edca_ani_acbk_local:    ani          BK      unicast
 * @edca_ani_acvi_local:    ani          VI      unicast
 * @edca_ani_acvo_local:    ani          VO      unicast
 * @edca_ani_acbe_bcast:    ani          BE      bcast
 * @edca_ani_acbk_bcast:    ani          BK      bcast
 * @edca_ani_acvi_bcast:    ani          VI      bcast
 * @edca_ani_acvo_bcast:    ani          VO      bcast
 * @edca_wme_acbe_local:    wme          BE      unicast
 * @edca_wme_acbk_local:    wme          BK      unicast
 * @edca_wme_acvi_local:    wme          VI      unicast
 * @edca_wme_acvo_local:    wme          VO      unicast
 * @edca_wme_acbe_bcast:    wme          BE      bcast
 * @edca_wme_acbk_bcast:    wme          BK      bcast
 * @edca_wme_acvi_bcast:    wme          VI      bcast
 * @edca_wme_acvo_bcast:    wme          VO      bcast
 * @edca_etsi_acbe_local:   etsi         BE      unicast
 * @edca_etsi_acbk_local:   etsi         BK      unicast
 * @edca_etsi_acvi_local:   etsi         VI      unicast
 * @edca_etsi_acvo_local:   etsi         VO      unicast
 * @edca_etsi_acbe_bcast:   etsi         BE      bcast
 * @edca_etsi_acbk_bcast:   etsi         BK      bcast
 * @edca_etsi_acvi_bcast:   etsi         VI      bcast
 * @edca_etsi_acvo_bcast:   etsi         VO      bcast
 */
enum e_edca_type {
	edca_ani_acbe_local,
	edca_ani_acbk_local,
	edca_ani_acvi_local,
	edca_ani_acvo_local,
	edca_ani_acbe_bcast,
	edca_ani_acbk_bcast,
	edca_ani_acvi_bcast,
	edca_ani_acvo_bcast,
	edca_wme_acbe_local,
	edca_wme_acbk_local,
	edca_wme_acvi_local,
	edca_wme_acvo_local,
	edca_wme_acbe_bcast,
	edca_wme_acbk_bcast,
	edca_wme_acvi_bcast,
	edca_wme_acvo_bcast,
	edca_etsi_acbe_local,
	edca_etsi_acbk_local,
	edca_etsi_acvi_local,
	edca_etsi_acvo_local,
	edca_etsi_acbe_bcast,
	edca_etsi_acbk_bcast,
	edca_etsi_acvi_bcast,
	edca_etsi_acvo_bcast
};

#define CFG_EDCA_PROFILE_ACM_IDX      0
#define CFG_EDCA_PROFILE_AIFSN_IDX    1
#define CFG_EDCA_PROFILE_CWMINA_IDX   2
#define CFG_EDCA_PROFILE_CWMAXA_IDX   4
#define CFG_EDCA_PROFILE_TXOPA_IDX    6
#define CFG_EDCA_PROFILE_CWMINB_IDX   7
#define CFG_EDCA_PROFILE_CWMAXB_IDX   9
#define CFG_EDCA_PROFILE_TXOPB_IDX    11
#define CFG_EDCA_PROFILE_CWMING_IDX   12
#define CFG_EDCA_PROFILE_CWMAXG_IDX   14
#define CFG_EDCA_PROFILE_TXOPG_IDX    16

/**
 * struct mlme_edca_params - EDCA pramaters related config items
 *
 * @ani_acbk_l:  EDCA parameters for ANI local access category background
 * @ani_acbe_l:  EDCA parameters for ANI local access category best effort
 * @ani_acvi_l:  EDCA parameters for ANI local access category video
 * @ani_acvo_l:  EDCA parameters for ANI local access category voice
 * @ani_acbk_b:  EDCA parameters for ANI bcast access category background
 * @ani_acbe_b:  EDCA parameters for ANI bcast access category best effort
 * @ani_acvi_b:  EDCA parameters for ANI bcast access category video
 * @ani_acvo_b:  EDCA parameters for ANI bcast access category voice
 * @wme_acbk_l:  EDCA parameters for WME local access category background
 * @wme_acbe_l:  EDCA parameters for WME local access category best effort
 * @wme_acvi_l:  EDCA parameters for WME local access category video
 * @wme_acvo_l:  EDCA parameters for WME local access category voice
 * @wme_acbk_b:  EDCA parameters for WME bcast access category background
 * @wme_acbe_b:  EDCA parameters for WME bcast access category best effort
 * @wme_acvi_b:  EDCA parameters for WME bcast access category video
 * @wme_acvo_b:  EDCA parameters for WME bcast access category voice
 * @etsi_acbk_l: EDCA parameters for ETSI local access category background
 * @etsi_acbe_l: EDCA parameters for ETSI local access category best effort
 * @etsi_acvi_l: EDCA parameters for ETSI local access category video
 * @etsi_acvo_l: EDCA parameters for ETSI local access category voice
 * @etsi_acbk_b: EDCA parameters for ETSI bcast access category background
 * @etsi_acbe_b: EDCA parameters for ETSI bcast access category best effort
 * @etsi_acvi_b: EDCA parameters for ETSI bcast access category video
 * @etsi_acvo_b: EDCA parameters for ETSI bcast access category voice
 */
struct wlan_mlme_edca_params {
	struct mlme_cfg_str ani_acbk_l;
	struct mlme_cfg_str ani_acbe_l;
	struct mlme_cfg_str ani_acvi_l;
	struct mlme_cfg_str ani_acvo_l;
	struct mlme_cfg_str ani_acbk_b;
	struct mlme_cfg_str ani_acbe_b;
	struct mlme_cfg_str ani_acvi_b;
	struct mlme_cfg_str ani_acvo_b;

	struct mlme_cfg_str wme_acbk_l;
	struct mlme_cfg_str wme_acbe_l;
	struct mlme_cfg_str wme_acvi_l;
	struct mlme_cfg_str wme_acvo_l;
	struct mlme_cfg_str wme_acbk_b;
	struct mlme_cfg_str wme_acbe_b;
	struct mlme_cfg_str wme_acvi_b;
	struct mlme_cfg_str wme_acvo_b;

	struct mlme_cfg_str etsi_acbk_l;
	struct mlme_cfg_str etsi_acbe_l;
	struct mlme_cfg_str etsi_acvi_l;
	struct mlme_cfg_str etsi_acvo_l;
	struct mlme_cfg_str etsi_acbk_b;
	struct mlme_cfg_str etsi_acbe_b;
	struct mlme_cfg_str etsi_acvi_b;
	struct mlme_cfg_str etsi_acvo_b;
};

#define WLAN_CFG_MFR_NAME_LEN (63)
#define WLAN_CFG_MODEL_NUMBER_LEN (31)
#define WLAN_CFG_MODEL_NAME_LEN (31)
#define WLAN_CFG_MFR_PRODUCT_NAME_LEN (31)
#define WLAN_CFG_MFR_PRODUCT_VERSION_LEN (31)

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
	bool sap_internal_restart;
	bool sap_ch_switch_mode;
	bool chan_switch_hostapd_rate_enabled_name;
	uint8_t reduced_beacon_interval;
};

/**
 * struct wlan_mlme_mbo - Multiband Operation related ini configs
 * @mbo_candidate_rssi_thres:
 * @mbo_current_rssi_thres:
 * @mbo_current_rssi_mcc_thres:
 * @mbo_candidate_rssi_btc_thres:
 */
struct wlan_mlme_mbo {
	int8_t mbo_candidate_rssi_thres;
	int8_t mbo_current_rssi_thres;
	int8_t mbo_current_rssi_mcc_thres;
	int8_t mbo_candidate_rssi_btc_thres;
};

/**
 * struct wlan_mlme_vht_caps - MLME VHT config items
 * @supp_chan_width: Supported Channel Width
 * @ldpc_coding_cap: LDPC Coding Capability
 * @short_gi_80mhz: 80MHz Short Guard Interval
 * @short_gi_160mhz: 160MHz Short Guard Interval
 * @tx_stbc: Tx STBC cap
 * @rx_stbc: Rx STBC cap
 * @su_bformer: SU Beamformer cap
 * @su_bformee: SU Beamformee cap
 * @tx_bfee_ant_supp: Tx beamformee anti supp
 * @num_soundingdim: Number of sounding dimensions
 * @mu_bformer: MU Beamformer cap
 * @txop_ps: Tx OPs in power save
 * @htc_vhtc: htc_vht capability
 * @link_adap_cap: Link adaptation capability
 * @rx_antpattern: Rx Antenna Pattern cap
 * @tx_antpattern: Tx Antenna Pattern cap
 * @rx_mcs_map: Rx MCS Map
 * @tx_mcs_map: Tx MCS Map
 * @rx_supp_data_rate: Rx highest supported data rate
 * @tx_supp_data_rate: Tx highest supported data rate
 * @basic_mcs_set: Basic MCS set
 * @enable_txbf_20mhz: enable tx bf for 20mhz
 * @channel_width: Channel width capability for 11ac
 * @rx_mcs: VHT Rx MCS capability for 1x1 mode
 * @tx_mcs: VHT Tx MCS capability for 1x1 mode
 * @rx_mcs2x2: VHT Rx MCS capability for 2x2 mode
 * @tx_mcs2x2: VHT Tx MCS capability for 2x2 mode
 * @enable_vht20_mcs9: Enables VHT MCS9 in 20M BW operation
 * @enable2x2: Enables/disables VHT Tx/Rx MCS values for 2x2
 * @enable_mu_bformee: Enables/disables multi-user (MU)
 * beam formee capability
 * @enable_paid: Enables/disables paid
 * @enable_gid: Enables/disables gid
 * @b24ghz_band: To control VHT support in 2.4 GHz band
 * @vendor_24ghz_band: to control VHT support based on vendor
 * ie in 2.4 GHz band
 * @ampdu_len_exponent: To handle maximum receive AMPDU ampdu len exponent
 * @ampdu_len: To handle maximum receive AMPDU ampdu len
 */
struct mlme_vht_capabilities_info {
	uint8_t supp_chan_width;
	bool ldpc_coding_cap;
	bool short_gi_80mhz;
	bool short_gi_160mhz;
	bool tx_stbc;
	bool rx_stbc;
	bool su_bformer;
	bool su_bformee;
	uint8_t tx_bfee_ant_supp;
	uint8_t num_soundingdim;
	bool mu_bformer;
	bool txop_ps;
	bool htc_vhtc;
	uint8_t link_adap_cap;
	bool rx_antpattern;
	bool tx_antpattern;
	uint32_t rx_mcs_map;
	uint32_t tx_mcs_map;
	uint32_t rx_supp_data_rate;
	uint32_t tx_supp_data_rate;
	uint32_t basic_mcs_set;
	bool enable_txbf_20mhz;
	uint8_t channel_width;
	uint8_t rx_mcs;
	uint8_t tx_mcs;
	uint8_t rx_mcs2x2;
	uint8_t tx_mcs2x2;
	bool enable_vht20_mcs9;
	bool enable2x2;
	bool enable_mu_bformee;
	bool enable_paid;
	bool enable_gid;
	bool b24ghz_band;
	bool vendor_24ghz_band;
	uint8_t ampdu_len_exponent;
	uint8_t ampdu_len;
};

/**
 * struct wlan_mlme_vht_caps - VHT Capabilities related config items
 * @vht_cap_info: VHT capabilities Info Structure
 */
struct wlan_mlme_vht_caps {
	struct mlme_vht_capabilities_info vht_cap_info;
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

#define MLME_HE_PPET_LEN 25
/**
 * struct wlan_mlme_he_caps - HE Capabilities related config items
 */
struct wlan_mlme_he_caps {
	uint8_t he_control;
	uint8_t he_twt_requestor;
	uint8_t he_twt_responder;
	uint8_t he_twt_fragmentation;
	uint8_t he_max_frag_msdu;
	uint8_t he_min_frag_size;
	uint8_t he_trig_pad;
	uint8_t he_mtid_aggr_rx;
	uint8_t he_link_adaptation;
	uint8_t he_all_ack;
	uint8_t he_trigd_rsp_scheduling;
	uint8_t he_buffer_status_rpt;
	uint8_t he_bcast_twt;
	uint8_t he_ba_32bit;
	uint8_t he_mu_cascading;
	uint8_t he_multi_tid;
	uint8_t he_dl_mu_ba;
	uint8_t he_omi;
	uint8_t he_ofdma_ra;
	uint8_t he_max_ampdu_len;
	uint8_t he_amsdu_frag;
	uint8_t he_flex_twt_sched;
	uint8_t he_rx_ctrl;
	uint8_t he_bsrp_ampdu_aggr;
	uint8_t he_qtp;
	uint8_t he_a_bqr;
	uint8_t he_sr_responder;
	uint8_t he_ndp_feedback_supp;
	uint8_t he_ops_supp;
	uint8_t he_amsdu_in_ampdu;
	uint8_t he_chan_width;
	uint8_t he_mtid_aggr_tx;
	uint8_t he_sub_ch_sel_tx;
	uint8_t he_ul_2x996_ru;
	uint8_t he_om_ctrl_ul_mu_dis_rx;
	uint8_t he_rx_pream_punc;
	uint8_t he_class_of_device;
	uint8_t he_ldpc;
	uint8_t he_ltf_ppdu;
	uint8_t he_midamble_rx_nsts;
	uint8_t he_ltf_ndp;
	uint8_t he_tx_stbc_lt80;
	uint8_t he_rx_stbc_lt80;
	uint8_t he_doppler;
	uint8_t he_ul_mumimo;
	uint8_t he_dcm_tx;
	uint8_t he_dcm_rx;
	uint8_t he_mu_ppdu;
	uint8_t he_su_beamformer;
	uint8_t he_su_beamformee;
	uint8_t he_mu_beamformer;
	uint8_t he_bfee_sts_lt80;
	uint8_t he_bfee_sts_gt80;
	uint8_t he_num_sound_lt80;
	uint8_t he_num_sound_gt80;
	uint8_t he_su_feed_tone16;
	uint8_t he_mu_feed_tone16;
	uint8_t he_codebook_su;
	uint8_t he_codebook_mu;
	uint8_t he_bfrm_feed;
	uint8_t he_er_su_ppdu;
	uint8_t he_dl_part_bw;
	uint8_t he_ppet_present;
	uint8_t he_srp;
	uint8_t he_power_boost;
	uint8_t he_4x_ltf_gi;
	uint8_t he_max_nc;
	uint8_t he_rx_stbc_gt80;
	uint8_t he_tx_stbc_gt80;
	uint8_t he_er_4x_ltf_gi;
	uint8_t he_ppdu_20_in_40mhz_2g;
	uint8_t he_ppdu_20_in_160_80p80mhz;
	uint8_t he_ppdu_80_in_160_80p80mhz;
	uint8_t he_er_1x_he_ltf_gi;
	uint8_t he_midamble_rx_1x_he_ltf;
	uint8_t he_dcm_max_bw;
	uint8_t he_longer_16_sigb_ofdm_sym;
	uint8_t he_non_trig_cqi_feedback;
	uint8_t he_tx_1024_qam_lt_242_ru;
	uint8_t he_rx_1024_qam_lt_242_ru;
	uint8_t he_rx_full_bw_mu_cmpr_sigb;
	uint8_t he_rx_full_bw_mu_non_cmpr_sigb;
	uint32_t he_rx_mcs_map_lt_80;
	uint32_t he_tx_mcs_map_lt_80;
	uint32_t he_rx_mcs_map_160;
	uint32_t he_tx_mcs_map_160;
	uint32_t he_rx_mcs_map_80_80;
	uint32_t he_tx_mcs_map_80_80;
	uint8_t he_ppet_2g[MLME_HE_PPET_LEN];
	uint8_t he_ppet_5g[MLME_HE_PPET_LEN];
	uint32_t he_ops_basic_mcs_nss;
	uint8_t he_twt_dynamic_fragmentation;
	uint8_t enable_ul_mimo;
	uint8_t enable_ul_ofdm;
	uint32_t he_sta_obsspd;
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


/* Flags for gLimProtectionControl that is updated in pe session*/
#define MLME_FORCE_POLICY_PROTECTION_DISABLE        0
#define MLME_FORCE_POLICY_PROTECTION_CTS            1
#define MLME_FORCE_POLICY_PROTECTION_RTS            2
#define MLME_FORCE_POLICY_PROTECTION_DUAL_CTS       3
#define MLME_FORCE_POLICY_PROTECTION_RTS_ALWAYS     4
#define MLME_FORCE_POLICY_PROTECTION_AUTO           5

/* protection_enabled bits*/
#define MLME_PROTECTION_ENABLED_FROM_llA            0
#define MLME_PROTECTION_ENABLED_FROM_llB            1
#define MLME_PROTECTION_ENABLED_FROM_llG            2
#define MLME_PROTECTION_ENABLED_HT_20               3
#define MLME_PROTECTION_ENABLED_NON_GF              4
#define MLME_PROTECTION_ENABLED_LSIG_TXOP           5
#define MLME_PROTECTION_ENABLED_RIFS                6
#define MLME_PROTECTION_ENABLED_OBSS                7
#define MLME_PROTECTION_ENABLED_OLBC_FROM_llA       8
#define MLME_PROTECTION_ENABLED_OLBC_FROM_llB       9
#define MLME_PROTECTION_ENABLED_OLBC_FROM_llG       10
#define MLME_PROTECTION_ENABLED_OLBC_HT20           11
#define MLME_PROTECTION_ENABLED_OLBC_NON_GF         12
#define MLME_PROTECTION_ENABLED_OLBC_LSIG_TXOP      13
#define MLME_PROTECTION_ENABLED_OLBC_RIFS           14
#define MLME_PROTECTION_ENABLED_OLBC_OBSS           15

/**
 * struct wlan_mlme_feature_flag - feature related information
 * @accept_short_slot_assoc: enable short sloc feature
 * @enable_hcf: enable HCF feature
 * @enable_rsn: enable RSN for connection
 * @enable_short_preamble_11g: enable short preamble for 11g
 * @channel_bonding_mode: channel bonding mode
 * @enable_block_ack: enable block ack feature
 * @enable_ampdu: Enable AMPDU feature
 */

struct wlan_mlme_feature_flag {
	bool accept_short_slot_assoc;
	bool enable_hcf;
	bool enable_rsn;
	bool enable_short_preamble_11g;
	bool enable_short_slot_time_11g;
	uint32_t channel_bonding_mode;
	uint32_t enable_block_ack;
	bool enable_ampdu;
};

/*
 * struct wlan_mlme_sap_protection_cfg - SAP erp protection config items
 *
 * @protection_enabled:        Force enable protection. static via cfg
 * @protection_force_policy:   Protection force policy. Static via cfg
 * @ignore_peer_ht_opmode:     Ignore the ht opmode of the peer. Dynamic via INI
 *
 */
struct wlan_mlme_sap_protection {
	uint32_t protection_enabled;
	uint8_t protection_force_policy;
	bool ignore_peer_ht_opmode;
};

/*
 * struct wlan_mlme_chainmask - All chainmask related cfg items
 *
 * @txchainmask1x1:     To set transmit chainmask
 * @rxchainmask1x1:     To set rx chainmask
 * @tx_chain_mask_cck:  Used to enable/disable Cck ChainMask
 * @tx_chain_mask_1ss:  Enables/disables tx chain Mask1ss
 * @num_11b_tx_chains:  Number of Tx Chains in 11b mode
 * @num_11ag_tx_chains: Number of Tx Chains in 11ag mode
 * @tx_chain_mask_2g:   Tx chain mask for 2g
 * @rx_chain_mask_2g:   Tx chain mask for 2g
 * @tx_chain_mask_5g:   Tx chain mask for 5g
 * @rx_chain_mask_5g:   Rx chain mask for 5g
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

/* struct wlan_mlme_generic - Generic CFG config items
 *
 * @band_capability: HW Band Capability - Both or 2.4G only or 5G only
 * @band: Current Band - Internal variable, initialized to INI and updated later
 * @select_5ghz_margin: RSSI margin to select 5Ghz over 2.4 Ghz
 * @sub_20_chan_width: Sub 20Mhz Channel Width
 * @ito_repeat_count: ITO Repeat Count
 * @pmf_sa_query_max_retries: PMF query max retries for SAP
 * @pmf_sa_query_retry_interval: PMF query retry interval for SAP
 * @dropped_pkt_disconnect_thresh: Threshold for dropped pkts before disconnect
 * @rtt3_enabled: RTT3 enable or disable info
 * @prevent_link_down: Enable/Disable prevention of link down
 * @memory_deep_sleep: Enable/Disable memory deep sleep
 * @cck_tx_fir_override: Enable/Disable CCK Tx FIR Override
 * @crash_inject: Enable/Disable Crash Inject
 * @lpass_support: Enable/Disable LPASS Support
 * @self_recovery: Enable/Disable Self Recovery
 * @sap_dot11mc: Enable/Disable SAP 802.11mc support
 * @fatal_event_trigger: Enable/Disable Fatal Events Trigger
 * @optimize_ca_event: Enable/Disable Optimization of CA events
 * @fw_timeout_crash: Enable/Disable FW Timeout Crash *
 */
struct wlan_mlme_generic {
	enum band_info band_capability;
	enum band_info band;
	uint8_t select_5ghz_margin;
	uint8_t sub_20_chan_width;
	uint8_t ito_repeat_count;
	uint8_t pmf_sa_query_max_retries;
	uint16_t pmf_sa_query_retry_interval;
	uint16_t dropped_pkt_disconnect_thresh;
	bool rtt3_enabled;
	bool prevent_link_down;
	bool memory_deep_sleep;
	bool cck_tx_fir_override;
	bool crash_inject;
	bool lpass_support;
	bool self_recovery;
	bool sap_dot11mc;
	bool fatal_event_trigger;
	bool optimize_ca_event;
	bool fw_timeout_crash;
};

/*
 * struct wlan_mlme_product_details_cfg - product details config items
 * @manufacturer_name: manufacture name
 * @model_number: model number
 * @model_name: model name
 * @manufacture_product_name: manufacture product name
 * @manufacture_product_version: manufacture product version
 */
struct wlan_mlme_product_details_cfg {
	char manufacturer_name[WLAN_CFG_MFR_NAME_LEN + 1];
	char model_number[WLAN_CFG_MODEL_NUMBER_LEN + 1];
	char model_name[WLAN_CFG_MODEL_NAME_LEN + 1];
	char manufacture_product_name[WLAN_CFG_MFR_PRODUCT_NAME_LEN + 1];
	char manufacture_product_version[WLAN_CFG_MFR_PRODUCT_VERSION_LEN + 1];
};

/*
 * struct wlan_mlme_acs - All acs related cfg items
 * @is_acs_with_more_param - to enable acs with more param
 * @auto_channel_select_weight - to set acs channel weight
 * @is_vendor_acs_support - enable application based channel selection
 * @is_acs_support_for_dfs_ltecoex - enable channel for dfs and lte coex
 * @is_external_acs_policy - control external policy
 */
struct wlan_mlme_acs {
	bool is_acs_with_more_param;
	uint32_t auto_channel_select_weight;
	bool is_vendor_acs_support;
	bool is_acs_support_for_dfs_ltecoex;
	bool is_external_acs_policy;
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

/*
 * @mawc_roam_enabled:              Enable/Disable MAWC during roaming
 * @enable_fast_roam_in_concurrency:Enable LFR roaming on STA during concurrency
 * @lfr3_roaming_offload:           Enable/disable roam offload feature
 * @early_stop_scan_enable:         Set early stop scan
 * @lfr3_enable_subnet_detection:   Enable LFR3 subnet detection
 * @enable_5g_band_pref:            Enable preference for 5G from INI
 * @mawc_roam_traffic_threshold:    Configure traffic threshold
 * @mawc_roam_ap_rssi_threshold:    Best AP RSSI threshold
 * @mawc_roam_rssi_high_adjust:     Adjust MAWC roam high RSSI
 * @mawc_roam_rssi_low_adjust:      Adjust MAWC roam low RSSI
 * @roam_rssi_abs_threshold:        The min RSSI of the candidate AP
 * @rssi_threshold_offset_5g:       Lookup threshold offset for 5G band
 * @early_stop_scan_min_threshold:  Set early stop scan min
 * @early_stop_scan_max_threshold:  Set early stop scan max
 * @first_scan_bucket_threshold:    Set first scan bucket
 * @roam_dense_traffic_threshold:   Dense traffic threshold
 * @roam_dense_rssi_thre_offset:    Sets dense roam RSSI threshold diff
 * @roam_dense_min_aps:             Sets minimum number of AP for dense roam
 * @roam_bg_scan_bad_rssi_threshold:RSSI threshold for background roam
 * @roam_bg_scan_client_bitmap:     Bitmap used to identify the scan clients
 * @roam_bg_scan_bad_rssi_offset_2g:RSSI threshold offset for 2G to 5G roam
 * @adaptive_roamscan_dwell_mode:   Sets dwell time adaptive mode
 * @per_roam_enable:                To enabled/disable PER based roaming in FW
 * @per_roam_config_high_rate_th:   Rate at which PER based roam will stop
 * @per_roam_config_low_rate_th:    Rate at which PER based roam will start
 * @per_roam_config_rate_th_percent:Percentage at which FW will issue roam scan
 * @per_roam_rest_time:             FW will wait once it issues a roam scan.
 * @per_roam_monitor_time:          Min time to be considered as valid scenario
 * @per_roam_min_candidate_rssi:    Min roamable AP RSSI for candidate selection
 * @lfr3_disallow_duration:         Disallow duration before roaming
 * @lfr3_rssi_channel_penalization: RSSI penalization
 * @lfr3_num_disallowed_aps:        Max number of AP's to maintain in LCA list
 * @rssi_boost_threshold_5g:        Boost threshold above which 5 GHz is favored
 * @rssi_boost_factor_5g:           Factor by which 5GHz RSSI is boosted
 * @max_rssi_boost_5g:              Maximum boost that can be applied to 5G RSSI
 * @rssi_penalize_threshold_5g:     Penalize thres above which 5G isn't favored
 * @rssi_penalize_factor_5g:        Factor by which 5GHz RSSI is penalizeed
 * @max_rssi_penalize_5g:           Max penalty that can be applied to 5G RSSI
 * @max_num_pre_auth:               Configure max number of pre-auth
 */
struct wlan_mlme_lfr_cfg {
	bool mawc_roam_enabled;
	bool enable_fast_roam_in_concurrency;
	bool lfr3_roaming_offload;
	bool early_stop_scan_enable;
	bool lfr3_enable_subnet_detection;
	bool enable_5g_band_pref;
	uint32_t mawc_roam_traffic_threshold;
	uint32_t mawc_roam_ap_rssi_threshold;
	uint32_t mawc_roam_rssi_high_adjust;
	uint32_t mawc_roam_rssi_low_adjust;
	uint32_t roam_rssi_abs_threshold;
	uint8_t rssi_threshold_offset_5g;
	uint8_t early_stop_scan_min_threshold;
	uint8_t early_stop_scan_max_threshold;
	uint8_t first_scan_bucket_threshold;
	uint32_t roam_dense_traffic_threshold;
	uint32_t roam_dense_rssi_thre_offset;
	uint32_t roam_dense_min_aps;
	uint32_t roam_bg_scan_bad_rssi_threshold;
	uint32_t roam_bg_scan_client_bitmap;
	uint32_t roam_bg_scan_bad_rssi_offset_2g;
	uint32_t adaptive_roamscan_dwell_mode;
	uint32_t per_roam_enable;
	uint32_t per_roam_config_high_rate_th;
	uint32_t per_roam_config_low_rate_th;
	uint32_t per_roam_config_rate_th_percent;
	uint32_t per_roam_rest_time;
	uint32_t per_roam_monitor_time;
	uint32_t per_roam_min_candidate_rssi;
	uint32_t lfr3_disallow_duration;
	uint32_t lfr3_rssi_channel_penalization;
	uint32_t lfr3_num_disallowed_aps;
	uint32_t rssi_boost_threshold_5g;
	uint32_t rssi_boost_factor_5g;
	uint32_t max_rssi_boost_5g;
	uint32_t rssi_penalize_threshold_5g;
	uint32_t rssi_penalize_factor_5g;
	uint32_t max_rssi_penalize_5g;
	uint32_t max_num_pre_auth;
};

/**
 * struct wlan_mlme_wmm_ac_vo - Default TSPEC parameters
 * for AC_VO
 * @dir_ac_vo: TSPEC direction for VO
 * @nom_msdu_size_ac_vo: normal MSDU size for VO
 * @mean_data_rate_ac_vo: mean data rate for VO
 * @min_phy_rate_ac_vo: min PHY rate for VO
 * @sba_ac_vo: surplus bandwidth allowance for VO
 * @uapsd_vo_srv_intv: Uapsd service interval for voice
 * @uapsd_vo_sus_intv: Uapsd suspension interval for voice
 */
struct wlan_mlme_wmm_ac_vo {
	uint8_t dir_ac_vo;
	uint16_t nom_msdu_size_ac_vo;
	uint32_t mean_data_rate_ac_vo;
	uint32_t min_phy_rate_ac_vo;
	uint16_t sba_ac_vo;
	uint32_t uapsd_vo_srv_intv;
	uint32_t uapsd_vo_sus_intv;
};

/**
 * struct wlan_mlme_wmm_params - WMM CFG Items
 * @qos_enabled: AP is enabled with 11E
 * @wme_enabled: AP is enabled with WMM
 * @max_sp_length: Maximum SP Length
 * @wsm_enabled: AP is enabled with WSM
 * @ac_vo: Default TSPEC parameters for AC_VO
 */
struct wlan_mlme_wmm_params {
	bool qos_enabled;
	bool wme_enabled;
	uint8_t max_sp_length;
	bool wsm_enabled;
	uint32_t edca_profile;
	struct wlan_mlme_wmm_ac_vo ac_vo;
};

/**
 * struct wlan_mlme_weight_config - weight params to
 * calculate best candidate
 *
 * @rssi_weightage: RSSI weightage
 * @ht_caps_weightage: HT caps weightage
 * @vht_caps_weightage: VHT caps weightage
 * @he_caps_weightage: HE caps weightage
 * @chan_width_weightage: Channel width weightage
 * @chan_band_weightage: Channel band weightage
 * @nss_weightage: NSS weightage
 * @beamforming_cap_weightage: Beamforming caps weightage
 * @pcl_weightage: PCL weightage
 * @channel_congestion_weightage: channel congestion weightage
 * @oce_wan_weightage: OCE WAN metrics weightage
 */
struct  wlan_mlme_weight_config {
	uint8_t rssi_weightage;
	uint8_t ht_caps_weightage;
	uint8_t vht_caps_weightage;
	uint8_t he_caps_weightage;
	uint8_t chan_width_weightage;
	uint8_t chan_band_weightage;
	uint8_t nss_weightage;
	uint8_t beamforming_cap_weightage;
	uint8_t pcl_weightage;
	uint8_t channel_congestion_weightage;
	uint8_t oce_wan_weightage;
};

/**
 * struct wlan_mlme_rssi_cfg_score - RSSI params to
 * calculate best candidate
 *
 * @best_rssi_threshold: Best RSSI threshold
 * @good_rssi_threshold: Good RSSI threshold
 * @bad_rssi_threshold: Bad RSSI threshold
 * @good_rssi_pcnt: Good RSSI Percentage
 * @bad_rssi_pcnt: Bad RSSI Percentage
 * @good_rssi_bucket_size: Good RSSI Bucket Size
 * @bad_rssi_bucket_size: Bad RSSI Bucket Size
 * @rssi_pref_5g_rssi_thresh: Preffered 5G RSSI threshold
 */
struct wlan_mlme_rssi_cfg_score  {
	uint32_t best_rssi_threshold;
	uint32_t good_rssi_threshold;
	uint32_t bad_rssi_threshold;
	uint32_t good_rssi_pcnt;
	uint32_t bad_rssi_pcnt;
	uint32_t good_rssi_bucket_size;
	uint32_t bad_rssi_bucket_size;
	uint32_t rssi_pref_5g_rssi_thresh;
};

/**
 * struct wlan_mlme_per_slot_scoring - define % score for differents slots
 *				for a scoring param.
 * num_slot: number of slots in which the param will be divided.
 *           Max 15. index 0 is used for 'not_present. Num_slot will
 *           equally divide 100. e.g, if num_slot = 4 slot 0 = 0-25%, slot
 *           1 = 26-50% slot 2 = 51-75%, slot 3 = 76-100%
 * score_pcnt3_to_0: Conatins score percentage for slot 0-3
 *             BITS 0-7   :- the scoring pcnt when not present
 *             BITS 8-15  :- SLOT_1
 *             BITS 16-23 :- SLOT_2
 *             BITS 24-31 :- SLOT_3
 * score_pcnt7_to_4: Conatins score percentage for slot 4-7
 *             BITS 0-7   :- SLOT_4
 *             BITS 8-15  :- SLOT_5
 *             BITS 16-23 :- SLOT_6
 *             BITS 24-31 :- SLOT_7
 * score_pcnt11_to_8: Conatins score percentage for slot 8-11
 *             BITS 0-7   :- SLOT_8
 *             BITS 8-15  :- SLOT_9
 *             BITS 16-23 :- SLOT_10
 *             BITS 24-31 :- SLOT_11
 * score_pcnt15_to_12: Conatins score percentage for slot 12-15
 *             BITS 0-7   :- SLOT_12
 *             BITS 8-15  :- SLOT_13
 *             BITS 16-23 :- SLOT_14
 *             BITS 24-31 :- SLOT_15
 */
struct wlan_mlme_per_slot_scoring {
	uint32_t num_slot;
	uint32_t score_pcnt3_to_0;
	uint32_t score_pcnt7_to_4;
	uint32_t score_pcnt11_to_8;
	uint32_t score_pcnt15_to_12;
};

/*
 * struct wlan_mlme_score_config - MLME BSS Scoring related config
 * @enable_scoring_for_roam: Enable/disable BSS Scoring for Roaming
 * @weight_cfg: Various Weight related Scoring Configs
 * @rssi_score: RSSI Scoring related thresholds/percentages config
 * @esp_qbss_scoring: ESP QBSS Scoring configs
 * @oce_wan_scoring: OCE WAN Scoring Configs
 * @bandwidth_weight_per_index: Bandwidth weight per index for scoring logic
 * @nss_weight_per_index: NSS weight per index for scoring logic
 * @band_weight_per_index: Band weight per index for scoring logic
 */
struct wlan_mlme_scoring_cfg {
	bool enable_scoring_for_roam;
	struct wlan_mlme_weight_config weight_cfg;
	struct wlan_mlme_rssi_cfg_score rssi_score;
	struct wlan_mlme_per_slot_scoring esp_qbss_scoring;
	struct wlan_mlme_per_slot_scoring oce_wan_scoring;
	uint32_t bandwidth_weight_per_index;
	uint32_t nss_weight_per_index;
	uint32_t band_weight_per_index;
};

/* struct wlan_mlme_threshold - Threshold related config items
 * @rts_threshold: set rts threshold
 * @frag_threshold: set fragmentation threshold
 */
struct wlan_mlme_threshold {
	uint32_t rts_threshold;
	uint32_t frag_threshold;
};

/*
 * struct wlan_mlme_timeout - mlme timeout related config items
 * @join_failure_timeout: join failure timeout
 * @auth_failure_timeout: authenticate failure timeout
 * @auth_rsp_timeout: authenticate response timeout
 * @assoc_failure_timeout: assoc failure timeout
 * @reassoc_failure_timeout: re-assoc failure timeout
 * @probe_after_hb_fail_timeout: Probe after HB fail timeout
 * @olbc_detect_timeout: OLBC detect timeout
 * @addts_rsp_timeout: ADDTS rsp timeout value
 * @heart_beat_threshold: Heart beat threshold
 * @ap_keep_alive_timeout: AP keep alive timeout value
 * @ap_link_monitor_timeout: AP link monitor timeout value
 * @ps_data_inactivity_timeout: PS data inactivity timeout
 */
struct wlan_mlme_timeout {
	uint32_t join_failure_timeout;
	uint32_t auth_failure_timeout;
	uint32_t auth_rsp_timeout;
	uint32_t assoc_failure_timeout;
	uint32_t reassoc_failure_timeout;
	uint32_t probe_after_hb_fail_timeout;
	uint32_t olbc_detect_timeout;
	uint32_t addts_rsp_timeout;
	uint32_t heart_beat_threshold;
	uint32_t ap_keep_alive_timeout;
	uint32_t ap_link_monitor_timeout;
	uint32_t ps_data_inactivity_timeout;
};

/**
 * struct wlan_mlme_oce - OCE related config items
 * @enable_bcast_probe_rsp: enable broadcast probe response
 * @oce_sta_enabled: enable/disable oce feature for sta
 * @oce_sap_enabled: enable/disable oce feature for sap
 * @fils_enabled: enable/disable fils support
 * @feature_bitmap: oce feature bitmap
 *
 */
struct wlan_mlme_oce {
	bool enable_bcast_probe_rsp;
	bool oce_sta_enabled;
	bool oce_sap_enabled;
	bool fils_enabled;
	uint8_t feature_bitmap;
};

#define MLME_WEP_MAX_KEY_LEN (13)

/**
 * enum wep_key_id  - values passed to get/set wep default keys
 * @MLME_WEP_DEFAULT_KEY_1: wep default key 1
 * @MLME_WEP_DEFAULT_KEY_2: wep default key 2
 * @MLME_WEP_DEFAULT_KEY_3: wep default key 3
 * @MLME_WEP_DEFAULT_KEY_4: wep default key 4
 */
enum wep_key_id {
	MLME_WEP_DEFAULT_KEY_1 = 0,
	MLME_WEP_DEFAULT_KEY_2,
	MLME_WEP_DEFAULT_KEY_3,
	MLME_WEP_DEFAULT_KEY_4
};

/**
 * struct wlan_mlme_wep_cfg - WEP related configs
 * @is_privacy_enabled:     Flag to check if encryption is enabled
 * @is_shared_key_auth:     Flag to check if the auth type is shared key
 * @is_auth_open_system:    Flag to check if the auth type is open
 * @auth_type:              Authentication type value
 * @wep_default_key_id:     Default WEP key id
 * @wep_default_key_1:      WEP encryption key 1
 * @wep_default_key_2:      WEP encryption key 2
 * @wep_default_key_3:      WEP encryption key 3
 * @wep_default_key_4:      WEP encryption key 4
 */
struct wlan_mlme_wep_cfg {
	bool is_privacy_enabled;
	bool is_shared_key_auth;
	bool is_auth_open_system;
	uint8_t auth_type;
	uint8_t wep_default_key_id;
	struct mlme_cfg_str wep_default_key_1;
	struct mlme_cfg_str wep_default_key_2;
	struct mlme_cfg_str wep_default_key_3;
	struct mlme_cfg_str wep_default_key_4;
};

/**
 * struct wlan_mlme_cfg - MLME config items
 * @chainmask_cfg: VHT chainmask related cfg items
 * @edca_params: edca related CFG items
 * @gen: Generic CFG items
 * @ht_caps: HT related CFG Items
 * @he_caps: HE related cfg items
 * @lfr: LFR related CFG Items
 * @obss_ht40:obss ht40 CFG Items
 * @mbo_cfg: Multiband Operation related CFG items
 * @vht_caps: VHT related CFG Items
 * @qos_mlme_params: QOS CFG Items
 * @rates: Rates related cfg items
 * @product_details: product details related CFG Items
 * @sap_protection_cfg: SAP erp protection related CFG items
 * @sap_cfg: sap CFG items
 * @sta: sta CFG Items
 * @scoring: BSS Scoring related CFG Items
 * @oce: OCE related CFG items
 * @threshold: threshold related cfg items
 * @timeouts: mlme timeout related CFG items
 * @acs: ACS related CFG items
 * @feature_flags: Feature flag config items
 * @wep_params:  WEP related config items
 * @wmm_params: WMM related CFG & INI Items
 */
struct wlan_mlme_cfg {
	struct wlan_mlme_chainmask chainmask_cfg;
	struct wlan_mlme_edca_params edca_params;
	struct wlan_mlme_generic gen;
	struct wlan_mlme_ht_caps ht_caps;
	struct wlan_mlme_he_caps he_caps;
	struct wlan_mlme_lfr_cfg lfr;
	struct wlan_mlme_obss_ht40 obss_ht40;
	struct wlan_mlme_mbo mbo_cfg;
	struct wlan_mlme_vht_caps vht_caps;
	struct wlan_mlme_qos qos_mlme_params;
	struct wlan_mlme_rates rates;
	struct wlan_mlme_product_details_cfg product_details;
	struct wlan_mlme_sap_protection sap_protection_cfg;
	struct wlan_mlme_cfg_sap sap_cfg;
	struct wlan_mlme_sta_cfg sta;
	struct wlan_mlme_scoring_cfg scoring;
	struct wlan_mlme_oce oce;
	struct wlan_mlme_threshold threshold;
	struct wlan_mlme_timeout timeouts;
	struct wlan_mlme_acs acs;
	struct wlan_mlme_feature_flag feature_flags;
	struct wlan_mlme_wep_cfg wep_params;
	struct wlan_mlme_wmm_params wmm_params;
};

#endif
