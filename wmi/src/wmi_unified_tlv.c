/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_api.h"
#include "wmi.h"
#include "wmi_version.h"
#include "wmi_unified_priv.h"
#include "wmi_version_whitelist.h"
#include <qdf_module.h>
#include <wlan_defs.h>
#include <wlan_cmn.h>
#include <htc_services.h>
#ifdef FEATURE_WLAN_APF
#include "wmi_unified_apf_tlv.h"
#endif
#ifdef WLAN_FEATURE_ACTION_OUI
#include "wmi_unified_action_oui_tlv.h"
#endif
#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
#include "wlan_pmo_hw_filter_public_struct.h"
#endif
#include <wlan_utility.h>
#ifdef WLAN_SUPPORT_GREEN_AP
#include "wlan_green_ap_api.h"
#endif

#include "wmi_unified_twt_api.h"

#ifdef WLAN_POLICY_MGR_ENABLE
#include "wlan_policy_mgr_public_struct.h"
#endif

#ifdef WMI_SMART_ANT_SUPPORT
#include "wmi_unified_smart_ant_api.h"
#endif

#ifdef WMI_DBR_SUPPORT
#include "wmi_unified_dbr_api.h"
#endif

#ifdef WMI_ATF_SUPPORT
#include "wmi_unified_atf_api.h"
#endif

#ifdef WMI_AP_SUPPORT
#include "wmi_unified_ap_api.h"
#endif

/* HTC service ids for WMI for multi-radio */
static const uint32_t multi_svc_ids[] = {WMI_CONTROL_SVC,
				WMI_CONTROL_SVC_WMAC1,
				WMI_CONTROL_SVC_WMAC2};

#ifdef ENABLE_HOST_TO_TARGET_CONVERSION
/*Populate peer_param array whose index as host id and
 *value as target id
 */
static const uint32_t peer_param_tlv[] = {
	[WMI_HOST_PEER_MIMO_PS_STATE] = WMI_PEER_MIMO_PS_STATE,
	[WMI_HOST_PEER_AMPDU] = WMI_PEER_AMPDU,
	[WMI_HOST_PEER_AUTHORIZE] =  WMI_PEER_AUTHORIZE,
	[WMI_HOST_PEER_CHWIDTH] = WMI_PEER_CHWIDTH,
	[WMI_HOST_PEER_NSS] = WMI_PEER_NSS,
	[WMI_HOST_PEER_USE_4ADDR] =  WMI_PEER_USE_4ADDR,
	[WMI_HOST_PEER_MEMBERSHIP] = WMI_PEER_MEMBERSHIP,
	[WMI_HOST_PEER_USERPOS] = WMI_PEER_USERPOS,
	[WMI_HOST_PEER_CRIT_PROTO_HINT_ENABLED] =
				   WMI_PEER_CRIT_PROTO_HINT_ENABLED,
	[WMI_HOST_PEER_TX_FAIL_CNT_THR] = WMI_PEER_TX_FAIL_CNT_THR,
	[WMI_HOST_PEER_SET_HW_RETRY_CTS2S] = WMI_PEER_SET_HW_RETRY_CTS2S,
	[WMI_HOST_PEER_IBSS_ATIM_WINDOW_LENGTH] =
				  WMI_PEER_IBSS_ATIM_WINDOW_LENGTH,
	[WMI_HOST_PEER_PHYMODE] = WMI_PEER_PHYMODE,
	[WMI_HOST_PEER_USE_FIXED_PWR] = WMI_PEER_USE_FIXED_PWR,
	[WMI_HOST_PEER_PARAM_FIXED_RATE] = WMI_PEER_PARAM_FIXED_RATE,
	[WMI_HOST_PEER_SET_MU_WHITELIST] = WMI_PEER_SET_MU_WHITELIST,
	[WMI_HOST_PEER_SET_MAC_TX_RATE] = WMI_PEER_SET_MAX_TX_RATE,
	[WMI_HOST_PEER_SET_MIN_TX_RATE] = WMI_PEER_SET_MIN_TX_RATE,
	[WMI_HOST_PEER_SET_DEFAULT_ROUTING] = WMI_PEER_SET_DEFAULT_ROUTING,
	[WMI_HOST_PEER_NSS_VHT160] = WMI_PEER_NSS_VHT160,
	[WMI_HOST_PEER_NSS_VHT80_80] = WMI_PEER_NSS_VHT80_80,
	[WMI_HOST_PEER_PARAM_SU_TXBF_SOUNDING_INTERVAL] =
				    WMI_PEER_PARAM_SU_TXBF_SOUNDING_INTERVAL,
	[WMI_HOST_PEER_PARAM_MU_TXBF_SOUNDING_INTERVAL] =
	     WMI_PEER_PARAM_MU_TXBF_SOUNDING_INTERVAL,
	[WMI_HOST_PEER_PARAM_TXBF_SOUNDING_ENABLE] =
				  WMI_PEER_PARAM_TXBF_SOUNDING_ENABLE,
	[WMI_HOST_PEER_PARAM_MU_ENABLE] = WMI_PEER_PARAM_MU_ENABLE,
	[WMI_HOST_PEER_PARAM_OFDMA_ENABLE] = WMI_PEER_PARAM_OFDMA_ENABLE,
	[WMI_HOST_PEER_PARAM_ENABLE_FT] = WMI_PEER_PARAM_ENABLE_FT,
};

/**
 * Populate pdev_param_value whose index is host param and value is target
 * param
 */
static const uint32_t pdev_param_tlv[] = {
	[wmi_pdev_param_tx_chain_mask] = WMI_PDEV_PARAM_TX_CHAIN_MASK,
	[wmi_pdev_param_rx_chain_mask] = WMI_PDEV_PARAM_RX_CHAIN_MASK,
	[wmi_pdev_param_txpower_limit2g] = WMI_PDEV_PARAM_TXPOWER_LIMIT2G,
	[wmi_pdev_param_txpower_limit5g] = WMI_PDEV_PARAM_TXPOWER_LIMIT5G,
	[wmi_pdev_param_txpower_scale] = WMI_PDEV_PARAM_TXPOWER_SCALE,
	[wmi_pdev_param_beacon_gen_mode] = WMI_PDEV_PARAM_BEACON_GEN_MODE,
	[wmi_pdev_param_beacon_tx_mode] = WMI_PDEV_PARAM_BEACON_TX_MODE,
	[wmi_pdev_param_resmgr_offchan_mode] =
				WMI_PDEV_PARAM_RESMGR_OFFCHAN_MODE,
	[wmi_pdev_param_protection_mode] = WMI_PDEV_PARAM_PROTECTION_MODE,
	[wmi_pdev_param_dynamic_bw] = WMI_PDEV_PARAM_DYNAMIC_BW,
	[wmi_pdev_param_non_agg_sw_retry_th] =
				WMI_PDEV_PARAM_NON_AGG_SW_RETRY_TH,
	[wmi_pdev_param_agg_sw_retry_th] = WMI_PDEV_PARAM_AGG_SW_RETRY_TH,
	[wmi_pdev_param_sta_kickout_th] = WMI_PDEV_PARAM_STA_KICKOUT_TH,
	[wmi_pdev_param_ac_aggrsize_scaling] =
					WMI_PDEV_PARAM_AC_AGGRSIZE_SCALING,
	[wmi_pdev_param_ltr_enable] = WMI_PDEV_PARAM_LTR_ENABLE,
	[wmi_pdev_param_ltr_ac_latency_be] =
				WMI_PDEV_PARAM_LTR_AC_LATENCY_BE,
	[wmi_pdev_param_ltr_ac_latency_bk] = WMI_PDEV_PARAM_LTR_AC_LATENCY_BK,
	[wmi_pdev_param_ltr_ac_latency_vi] = WMI_PDEV_PARAM_LTR_AC_LATENCY_VI,
	[wmi_pdev_param_ltr_ac_latency_vo] = WMI_PDEV_PARAM_LTR_AC_LATENCY_VO,
	[wmi_pdev_param_ltr_ac_latency_timeout] =
					WMI_PDEV_PARAM_LTR_AC_LATENCY_TIMEOUT,
	[wmi_pdev_param_ltr_sleep_override] = WMI_PDEV_PARAM_LTR_SLEEP_OVERRIDE,
	[wmi_pdev_param_ltr_rx_override] = WMI_PDEV_PARAM_LTR_RX_OVERRIDE,
	[wmi_pdev_param_ltr_tx_activity_timeout] =
					WMI_PDEV_PARAM_LTR_TX_ACTIVITY_TIMEOUT,
	[wmi_pdev_param_l1ss_enable] = WMI_PDEV_PARAM_L1SS_ENABLE,
	[wmi_pdev_param_dsleep_enable] = WMI_PDEV_PARAM_DSLEEP_ENABLE,
	[wmi_pdev_param_pcielp_txbuf_flush] = WMI_PDEV_PARAM_PCIELP_TXBUF_FLUSH,
	[wmi_pdev_param_pcielp_txbuf_watermark] =
					 WMI_PDEV_PARAM_PCIELP_TXBUF_WATERMARK,
	[wmi_pdev_param_pcielp_txbuf_tmo_en] =
					 WMI_PDEV_PARAM_PCIELP_TXBUF_TMO_EN,
	[wmi_pdev_param_pcielp_txbuf_tmo_value] =
				WMI_PDEV_PARAM_PCIELP_TXBUF_TMO_VALUE,
	[wmi_pdev_param_pdev_stats_update_period] =
				WMI_PDEV_PARAM_PDEV_STATS_UPDATE_PERIOD,
	[wmi_pdev_param_vdev_stats_update_period] =
				WMI_PDEV_PARAM_VDEV_STATS_UPDATE_PERIOD,
	[wmi_pdev_param_peer_stats_update_period] =
				WMI_PDEV_PARAM_PEER_STATS_UPDATE_PERIOD,
	[wmi_pdev_param_bcnflt_stats_update_period] =
				WMI_PDEV_PARAM_BCNFLT_STATS_UPDATE_PERIOD,
	[wmi_pdev_param_pmf_qos] = WMI_PDEV_PARAM_PMF_QOS,
	[wmi_pdev_param_arp_ac_override] = WMI_PDEV_PARAM_ARP_AC_OVERRIDE,
	[wmi_pdev_param_dcs] = WMI_PDEV_PARAM_DCS,
	[wmi_pdev_param_ani_enable] = WMI_PDEV_PARAM_ANI_ENABLE,
	[wmi_pdev_param_ani_poll_period] = WMI_PDEV_PARAM_ANI_POLL_PERIOD,
	[wmi_pdev_param_ani_listen_period] = WMI_PDEV_PARAM_ANI_LISTEN_PERIOD,
	[wmi_pdev_param_ani_ofdm_level] = WMI_PDEV_PARAM_ANI_OFDM_LEVEL,
	[wmi_pdev_param_ani_cck_level] = WMI_PDEV_PARAM_ANI_CCK_LEVEL,
	[wmi_pdev_param_dyntxchain] = WMI_PDEV_PARAM_DYNTXCHAIN,
	[wmi_pdev_param_proxy_sta] = WMI_PDEV_PARAM_PROXY_STA,
	[wmi_pdev_param_idle_ps_config] = WMI_PDEV_PARAM_IDLE_PS_CONFIG,
	[wmi_pdev_param_power_gating_sleep] = WMI_PDEV_PARAM_POWER_GATING_SLEEP,
	[wmi_pdev_param_rfkill_enable] = WMI_PDEV_PARAM_RFKILL_ENABLE,
	[wmi_pdev_param_burst_dur] = WMI_PDEV_PARAM_BURST_DUR,
	[wmi_pdev_param_burst_enable] = WMI_PDEV_PARAM_BURST_ENABLE,
	[wmi_pdev_param_hw_rfkill_config] = WMI_PDEV_PARAM_HW_RFKILL_CONFIG,
	[wmi_pdev_param_low_power_rf_enable] =
					WMI_PDEV_PARAM_LOW_POWER_RF_ENABLE,
	[wmi_pdev_param_l1ss_track] = WMI_PDEV_PARAM_L1SS_TRACK,
	[wmi_pdev_param_hyst_en] = WMI_PDEV_PARAM_HYST_EN,
	[wmi_pdev_param_power_collapse_enable] =
					WMI_PDEV_PARAM_POWER_COLLAPSE_ENABLE,
	[wmi_pdev_param_led_sys_state] = WMI_PDEV_PARAM_LED_SYS_STATE,
	[wmi_pdev_param_led_enable] = WMI_PDEV_PARAM_LED_ENABLE,
	[wmi_pdev_param_audio_over_wlan_latency] =
				WMI_PDEV_PARAM_AUDIO_OVER_WLAN_LATENCY,
	[wmi_pdev_param_audio_over_wlan_enable] =
				WMI_PDEV_PARAM_AUDIO_OVER_WLAN_ENABLE,
	[wmi_pdev_param_whal_mib_stats_update_enable] =
		WMI_PDEV_PARAM_WHAL_MIB_STATS_UPDATE_ENABLE,
	[wmi_pdev_param_vdev_rate_stats_update_period] =
		WMI_PDEV_PARAM_VDEV_RATE_STATS_UPDATE_PERIOD,
	[wmi_pdev_param_cts_cbw] = WMI_PDEV_PARAM_CTS_CBW,
	[wmi_pdev_param_wnts_config] = WMI_PDEV_PARAM_WNTS_CONFIG,
	[wmi_pdev_param_adaptive_early_rx_enable] =
		WMI_PDEV_PARAM_ADAPTIVE_EARLY_RX_ENABLE,
	[wmi_pdev_param_adaptive_early_rx_min_sleep_slop] =
		WMI_PDEV_PARAM_ADAPTIVE_EARLY_RX_MIN_SLEEP_SLOP,
	[wmi_pdev_param_adaptive_early_rx_inc_dec_step] =
		WMI_PDEV_PARAM_ADAPTIVE_EARLY_RX_INC_DEC_STEP,
	[wmi_pdev_param_early_rx_fix_sleep_slop] =
		WMI_PDEV_PARAM_EARLY_RX_FIX_SLEEP_SLOP,
	[wmi_pdev_param_bmiss_based_adaptive_bto_enable] =
		WMI_PDEV_PARAM_BMISS_BASED_ADAPTIVE_BTO_ENABLE,
	[wmi_pdev_param_bmiss_bto_min_bcn_timeout] =
		WMI_PDEV_PARAM_BMISS_BTO_MIN_BCN_TIMEOUT,
	[wmi_pdev_param_bmiss_bto_inc_dec_step] =
		WMI_PDEV_PARAM_BMISS_BTO_INC_DEC_STEP,
	[wmi_pdev_param_bto_fix_bcn_timeout] =
		WMI_PDEV_PARAM_BTO_FIX_BCN_TIMEOUT,
	[wmi_pdev_param_ce_based_adaptive_bto_enable] =
		WMI_PDEV_PARAM_CE_BASED_ADAPTIVE_BTO_ENABLE,
	[wmi_pdev_param_ce_bto_combo_ce_value] =
		WMI_PDEV_PARAM_CE_BTO_COMBO_CE_VALUE,
	[wmi_pdev_param_tx_chain_mask_2g] = WMI_PDEV_PARAM_TX_CHAIN_MASK_2G,
	[wmi_pdev_param_rx_chain_mask_2g] = WMI_PDEV_PARAM_RX_CHAIN_MASK_2G,
	[wmi_pdev_param_tx_chain_mask_5g] = WMI_PDEV_PARAM_TX_CHAIN_MASK_5G,
	[wmi_pdev_param_rx_chain_mask_5g] = WMI_PDEV_PARAM_RX_CHAIN_MASK_5G,
	[wmi_pdev_param_tx_chain_mask_cck] = WMI_PDEV_PARAM_TX_CHAIN_MASK_CCK,
	[wmi_pdev_param_tx_chain_mask_1ss] = WMI_PDEV_PARAM_TX_CHAIN_MASK_1SS,
	[wmi_pdev_param_rx_filter] = WMI_PDEV_PARAM_RX_FILTER,
	[wmi_pdev_set_mcast_to_ucast_tid] = WMI_PDEV_SET_MCAST_TO_UCAST_TID,
	[wmi_pdev_param_mgmt_retry_limit] = WMI_PDEV_PARAM_MGMT_RETRY_LIMIT,
	[wmi_pdev_param_aggr_burst] = WMI_PDEV_PARAM_AGGR_BURST,
	[wmi_pdev_peer_sta_ps_statechg_enable] =
		WMI_PDEV_PEER_STA_PS_STATECHG_ENABLE,
	[wmi_pdev_param_proxy_sta_mode] = WMI_PDEV_PARAM_PROXY_STA_MODE,
	[wmi_pdev_param_mu_group_policy] = WMI_PDEV_PARAM_MU_GROUP_POLICY,
	[wmi_pdev_param_noise_detection] = WMI_PDEV_PARAM_NOISE_DETECTION,
	[wmi_pdev_param_noise_threshold] = WMI_PDEV_PARAM_NOISE_THRESHOLD,
	[wmi_pdev_param_dpd_enable] = WMI_PDEV_PARAM_DPD_ENABLE,
	[wmi_pdev_param_set_mcast_bcast_echo] =
		WMI_PDEV_PARAM_SET_MCAST_BCAST_ECHO,
	[wmi_pdev_param_atf_strict_sch] = WMI_PDEV_PARAM_ATF_STRICT_SCH,
	[wmi_pdev_param_atf_sched_duration] = WMI_PDEV_PARAM_ATF_SCHED_DURATION,
	[wmi_pdev_param_ant_plzn] = WMI_PDEV_PARAM_ANT_PLZN,
	[wmi_pdev_param_sensitivity_level] = WMI_PDEV_PARAM_SENSITIVITY_LEVEL,
	[wmi_pdev_param_signed_txpower_2g] = WMI_PDEV_PARAM_SIGNED_TXPOWER_2G,
	[wmi_pdev_param_signed_txpower_5g] = WMI_PDEV_PARAM_SIGNED_TXPOWER_5G,
	[wmi_pdev_param_enable_per_tid_amsdu] =
			WMI_PDEV_PARAM_ENABLE_PER_TID_AMSDU,
	[wmi_pdev_param_enable_per_tid_ampdu] =
			WMI_PDEV_PARAM_ENABLE_PER_TID_AMPDU,
	[wmi_pdev_param_cca_threshold] = WMI_PDEV_PARAM_CCA_THRESHOLD,
	[wmi_pdev_param_rts_fixed_rate] = WMI_PDEV_PARAM_RTS_FIXED_RATE,
	[wmi_pdev_param_cal_period] = WMI_UNAVAILABLE_PARAM,
	[wmi_pdev_param_pdev_reset] = WMI_PDEV_PARAM_PDEV_RESET,
	[wmi_pdev_param_wapi_mbssid_offset] = WMI_PDEV_PARAM_WAPI_MBSSID_OFFSET,
	[wmi_pdev_param_arp_srcaddr] = WMI_PDEV_PARAM_ARP_DBG_SRCADDR,
	[wmi_pdev_param_arp_dstaddr] = WMI_PDEV_PARAM_ARP_DBG_DSTADDR,
	[wmi_pdev_param_txpower_decr_db] = WMI_PDEV_PARAM_TXPOWER_DECR_DB,
	[wmi_pdev_param_rx_batchmode] = WMI_UNAVAILABLE_PARAM,
	[wmi_pdev_param_packet_aggr_delay] = WMI_UNAVAILABLE_PARAM,
	[wmi_pdev_param_atf_obss_noise_sch] =
		WMI_PDEV_PARAM_ATF_OBSS_NOISE_SCH,
	[wmi_pdev_param_atf_obss_noise_scaling_factor] =
		WMI_PDEV_PARAM_ATF_OBSS_NOISE_SCALING_FACTOR,
	[wmi_pdev_param_cust_txpower_scale] = WMI_PDEV_PARAM_CUST_TXPOWER_SCALE,
	[wmi_pdev_param_atf_dynamic_enable] = WMI_PDEV_PARAM_ATF_DYNAMIC_ENABLE,
	[wmi_pdev_param_atf_ssid_group_policy] = WMI_UNAVAILABLE_PARAM,
	[wmi_pdev_param_igmpmld_override] = WMI_PDEV_PARAM_IGMPMLD_AC_OVERRIDE,
	[wmi_pdev_param_igmpmld_tid] = WMI_PDEV_PARAM_IGMPMLD_AC_OVERRIDE,
	[wmi_pdev_param_antenna_gain] = WMI_PDEV_PARAM_ANTENNA_GAIN,
	[wmi_pdev_param_block_interbss] = WMI_PDEV_PARAM_BLOCK_INTERBSS,
	[wmi_pdev_param_set_disable_reset_cmdid] =
			WMI_PDEV_PARAM_SET_DISABLE_RESET_CMDID,
	[wmi_pdev_param_set_msdu_ttl_cmdid] = WMI_PDEV_PARAM_SET_MSDU_TTL_CMDID,
	[wmi_pdev_param_txbf_sound_period_cmdid] =
			WMI_PDEV_PARAM_TXBF_SOUND_PERIOD_CMDID,
	[wmi_pdev_param_set_burst_mode_cmdid] =
			WMI_PDEV_PARAM_SET_BURST_MODE_CMDID,
	[wmi_pdev_param_en_stats] = WMI_PDEV_PARAM_EN_STATS,
	[wmi_pdev_param_mesh_mcast_enable] = WMI_PDEV_PARAM_MESH_MCAST_ENABLE,
	[wmi_pdev_param_set_promisc_mode_cmdid] =
				WMI_PDEV_PARAM_SET_PROMISC_MODE_CMDID,
	[wmi_pdev_param_set_ppdu_duration_cmdid] =
			WMI_PDEV_PARAM_SET_PPDU_DURATION_CMDID,
	[wmi_pdev_param_remove_mcast2ucast_buffer] =
		WMI_PDEV_PARAM_REMOVE_MCAST2UCAST_BUFFER,
	[wmi_pdev_param_set_mcast2ucast_buffer] =
		WMI_PDEV_PARAM_SET_MCAST2UCAST_BUFFER,
	[wmi_pdev_param_set_mcast2ucast_mode] =
		WMI_PDEV_PARAM_SET_MCAST2UCAST_MODE,
	[wmi_pdev_param_smart_antenna_default_antenna] =
		WMI_PDEV_PARAM_SMART_ANTENNA_DEFAULT_ANTENNA,
	[wmi_pdev_param_fast_channel_reset] =
		WMI_PDEV_PARAM_FAST_CHANNEL_RESET,
	[wmi_pdev_param_rx_decap_mode] = WMI_PDEV_PARAM_RX_DECAP_MODE,
	[wmi_pdev_param_tx_ack_timeout] = WMI_PDEV_PARAM_ACK_TIMEOUT,
	[wmi_pdev_param_cck_tx_enable] = WMI_PDEV_PARAM_CCK_TX_ENABLE,
	[wmi_pdev_param_antenna_gain_half_db] =
		WMI_PDEV_PARAM_ANTENNA_GAIN_HALF_DB,
	[wmi_pdev_param_esp_indication_period] =
				WMI_PDEV_PARAM_ESP_INDICATION_PERIOD,
	[wmi_pdev_param_esp_ba_window] = WMI_PDEV_PARAM_ESP_BA_WINDOW,
	[wmi_pdev_param_esp_airtime_fraction] =
			WMI_PDEV_PARAM_ESP_AIRTIME_FRACTION,
	[wmi_pdev_param_esp_ppdu_duration] = WMI_PDEV_PARAM_ESP_PPDU_DURATION,
	[wmi_pdev_param_ru26_allowed] = WMI_PDEV_PARAM_UL_RU26_ALLOWED,
	[wmi_pdev_param_use_nol] = WMI_PDEV_PARAM_USE_NOL,
	/* Trigger interval for all trigger types. */
	[wmi_pdev_param_ul_trig_int] = WMI_PDEV_PARAM_SET_UL_BSR_TRIG_INTERVAL,
	[wmi_pdev_param_sub_channel_marking] =
					WMI_PDEV_PARAM_SUB_CHANNEL_MARKING,
	[wmi_pdev_param_ul_ppdu_duration] = WMI_PDEV_PARAM_SET_UL_PPDU_DURATION,
	[wmi_pdev_param_equal_ru_allocation_enable] =
				WMI_PDEV_PARAM_EQUAL_RU_ALLOCATION_ENABLE,
	[wmi_pdev_param_per_peer_prd_cfr_enable] =
			WMI_PDEV_PARAM_PER_PEER_PERIODIC_CFR_ENABLE,
	[wmi_pdev_param_set_mgmt_ttl] = WMI_PDEV_PARAM_SET_MGMT_TTL,
	[wmi_pdev_param_set_prb_rsp_ttl] =
			WMI_PDEV_PARAM_SET_PROBE_RESP_TTL,
	[wmi_pdev_param_set_mu_ppdu_duration] =
			WMI_PDEV_PARAM_SET_MU_PPDU_DURATION,
	[wmi_pdev_param_set_tbtt_ctrl] =
			WMI_PDEV_PARAM_SET_TBTT_CTRL,
	[wmi_pdev_param_set_cmd_obss_pd_threshold] =
			WMI_PDEV_PARAM_SET_CMD_OBSS_PD_THRESHOLD,
};

/**
 * Populate vdev_param_value_tlv array whose index is host param
 * and value is target param
 */
static const uint32_t vdev_param_tlv[] = {
	[wmi_vdev_param_rts_threshold] = WMI_VDEV_PARAM_RTS_THRESHOLD,
	[wmi_vdev_param_fragmentation_threshold] =
			WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD,
	[wmi_vdev_param_beacon_interval] = WMI_VDEV_PARAM_BEACON_INTERVAL,
	[wmi_vdev_param_listen_interval] = WMI_VDEV_PARAM_LISTEN_INTERVAL,
	[wmi_vdev_param_multicast_rate] = WMI_VDEV_PARAM_MULTICAST_RATE,
	[wmi_vdev_param_mgmt_tx_rate] = WMI_VDEV_PARAM_MGMT_TX_RATE,
	[wmi_vdev_param_slot_time] = WMI_VDEV_PARAM_SLOT_TIME,
	[wmi_vdev_param_preamble] = WMI_VDEV_PARAM_PREAMBLE,
	[wmi_vdev_param_swba_time] = WMI_VDEV_PARAM_SWBA_TIME,
	[wmi_vdev_stats_update_period] = WMI_VDEV_STATS_UPDATE_PERIOD,
	[wmi_vdev_pwrsave_ageout_time] = WMI_VDEV_PWRSAVE_AGEOUT_TIME,
	[wmi_vdev_host_swba_interval] = WMI_VDEV_HOST_SWBA_INTERVAL,
	[wmi_vdev_param_dtim_period] = WMI_VDEV_PARAM_DTIM_PERIOD,
	[wmi_vdev_oc_scheduler_air_time_limit] =
			WMI_VDEV_OC_SCHEDULER_AIR_TIME_LIMIT,
	[wmi_vdev_param_wds] = WMI_VDEV_PARAM_WDS,
	[wmi_vdev_param_atim_window] = WMI_VDEV_PARAM_ATIM_WINDOW,
	[wmi_vdev_param_bmiss_count_max] = WMI_VDEV_PARAM_BMISS_COUNT_MAX,
	[wmi_vdev_param_bmiss_first_bcnt] = WMI_VDEV_PARAM_BMISS_FIRST_BCNT,
	[wmi_vdev_param_bmiss_final_bcnt] = WMI_VDEV_PARAM_BMISS_FINAL_BCNT,
	[wmi_vdev_param_feature_wmm] = WMI_VDEV_PARAM_FEATURE_WMM,
	[wmi_vdev_param_chwidth] = WMI_VDEV_PARAM_CHWIDTH,
	[wmi_vdev_param_chextoffset] = WMI_VDEV_PARAM_CHEXTOFFSET,
	[wmi_vdev_param_disable_htprotection] =
			WMI_VDEV_PARAM_DISABLE_HTPROTECTION,
	[wmi_vdev_param_sta_quickkickout] = WMI_VDEV_PARAM_STA_QUICKKICKOUT,
	[wmi_vdev_param_mgmt_rate] = WMI_VDEV_PARAM_MGMT_RATE,
	[wmi_vdev_param_protection_mode] = WMI_VDEV_PARAM_PROTECTION_MODE,
	[wmi_vdev_param_fixed_rate] = WMI_VDEV_PARAM_FIXED_RATE,
	[wmi_vdev_param_sgi] = WMI_VDEV_PARAM_SGI,
	[wmi_vdev_param_ldpc] = WMI_VDEV_PARAM_LDPC,
	[wmi_vdev_param_tx_stbc] = WMI_VDEV_PARAM_TX_STBC,
	[wmi_vdev_param_rx_stbc] = WMI_VDEV_PARAM_RX_STBC,
	[wmi_vdev_param_intra_bss_fwd] = WMI_VDEV_PARAM_INTRA_BSS_FWD,
	[wmi_vdev_param_def_keyid] = WMI_VDEV_PARAM_DEF_KEYID,
	[wmi_vdev_param_nss] = WMI_VDEV_PARAM_NSS,
	[wmi_vdev_param_bcast_data_rate] = WMI_VDEV_PARAM_BCAST_DATA_RATE,
	[wmi_vdev_param_mcast_data_rate] = WMI_VDEV_PARAM_MCAST_DATA_RATE,
	[wmi_vdev_param_mcast_indicate] = WMI_VDEV_PARAM_MCAST_INDICATE,
	[wmi_vdev_param_dhcp_indicate] = WMI_VDEV_PARAM_DHCP_INDICATE,
	[wmi_vdev_param_unknown_dest_indicate] =
			WMI_VDEV_PARAM_UNKNOWN_DEST_INDICATE,
	[wmi_vdev_param_ap_keepalive_min_idle_inactive_time_secs] =
		WMI_VDEV_PARAM_AP_KEEPALIVE_MIN_IDLE_INACTIVE_TIME_SECS,
	[wmi_vdev_param_ap_keepalive_max_idle_inactive_time_secs] =
			WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_IDLE_INACTIVE_TIME_SECS,
	[wmi_vdev_param_ap_keepalive_max_unresponsive_time_secs] =
			WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS,
	[wmi_vdev_param_ap_enable_nawds] = WMI_VDEV_PARAM_AP_ENABLE_NAWDS,
	[wmi_vdev_param_enable_rtscts] = WMI_VDEV_PARAM_ENABLE_RTSCTS,
	[wmi_vdev_param_txbf] = WMI_VDEV_PARAM_TXBF,
	[wmi_vdev_param_packet_powersave] = WMI_VDEV_PARAM_PACKET_POWERSAVE,
	[wmi_vdev_param_drop_unencry] = WMI_VDEV_PARAM_DROP_UNENCRY,
	[wmi_vdev_param_tx_encap_type] = WMI_VDEV_PARAM_TX_ENCAP_TYPE,
	[wmi_vdev_param_ap_detect_out_of_sync_sleeping_sta_time_secs] =
		WMI_VDEV_PARAM_AP_DETECT_OUT_OF_SYNC_SLEEPING_STA_TIME_SECS,
	[wmi_vdev_param_early_rx_adjust_enable] =
			WMI_VDEV_PARAM_EARLY_RX_ADJUST_ENABLE,
	[wmi_vdev_param_early_rx_tgt_bmiss_num] =
		WMI_VDEV_PARAM_EARLY_RX_TGT_BMISS_NUM,
	[wmi_vdev_param_early_rx_bmiss_sample_cycle] =
		WMI_VDEV_PARAM_EARLY_RX_BMISS_SAMPLE_CYCLE,
	[wmi_vdev_param_early_rx_slop_step] = WMI_VDEV_PARAM_EARLY_RX_SLOP_STEP,
	[wmi_vdev_param_early_rx_init_slop] = WMI_VDEV_PARAM_EARLY_RX_INIT_SLOP,
	[wmi_vdev_param_early_rx_adjust_pause] =
		WMI_VDEV_PARAM_EARLY_RX_ADJUST_PAUSE,
	[wmi_vdev_param_tx_pwrlimit] = WMI_VDEV_PARAM_TX_PWRLIMIT,
	[wmi_vdev_param_snr_num_for_cal] = WMI_VDEV_PARAM_SNR_NUM_FOR_CAL,
	[wmi_vdev_param_roam_fw_offload] = WMI_VDEV_PARAM_ROAM_FW_OFFLOAD,
	[wmi_vdev_param_enable_rmc] = WMI_VDEV_PARAM_ENABLE_RMC,
	[wmi_vdev_param_ibss_max_bcn_lost_ms] =
			WMI_VDEV_PARAM_IBSS_MAX_BCN_LOST_MS,
	[wmi_vdev_param_max_rate] = WMI_VDEV_PARAM_MAX_RATE,
	[wmi_vdev_param_early_rx_drift_sample] =
			WMI_VDEV_PARAM_EARLY_RX_DRIFT_SAMPLE,
	[wmi_vdev_param_set_ibss_tx_fail_cnt_thr] =
			WMI_VDEV_PARAM_SET_IBSS_TX_FAIL_CNT_THR,
	[wmi_vdev_param_ebt_resync_timeout] =
			WMI_VDEV_PARAM_EBT_RESYNC_TIMEOUT,
	[wmi_vdev_param_aggr_trig_event_enable] =
			WMI_VDEV_PARAM_AGGR_TRIG_EVENT_ENABLE,
	[wmi_vdev_param_is_ibss_power_save_allowed] =
			WMI_VDEV_PARAM_IS_IBSS_POWER_SAVE_ALLOWED,
	[wmi_vdev_param_is_power_collapse_allowed] =
			WMI_VDEV_PARAM_IS_POWER_COLLAPSE_ALLOWED,
	[wmi_vdev_param_is_awake_on_txrx_enabled] =
			WMI_VDEV_PARAM_IS_AWAKE_ON_TXRX_ENABLED,
	[wmi_vdev_param_inactivity_cnt] = WMI_VDEV_PARAM_INACTIVITY_CNT,
	[wmi_vdev_param_txsp_end_inactivity_time_ms] =
			WMI_VDEV_PARAM_TXSP_END_INACTIVITY_TIME_MS,
	[wmi_vdev_param_dtim_policy] = WMI_VDEV_PARAM_DTIM_POLICY,
	[wmi_vdev_param_ibss_ps_warmup_time_secs] =
			WMI_VDEV_PARAM_IBSS_PS_WARMUP_TIME_SECS,
	[wmi_vdev_param_ibss_ps_1rx_chain_in_atim_window_enable] =
		WMI_VDEV_PARAM_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_ENABLE,
	[wmi_vdev_param_rx_leak_window] = WMI_VDEV_PARAM_RX_LEAK_WINDOW,
	[wmi_vdev_param_stats_avg_factor] =
			WMI_VDEV_PARAM_STATS_AVG_FACTOR,
	[wmi_vdev_param_disconnect_th] = WMI_VDEV_PARAM_DISCONNECT_TH,
	[wmi_vdev_param_rtscts_rate] = WMI_VDEV_PARAM_RTSCTS_RATE,
	[wmi_vdev_param_mcc_rtscts_protection_enable] =
			WMI_VDEV_PARAM_MCC_RTSCTS_PROTECTION_ENABLE,
	[wmi_vdev_param_mcc_broadcast_probe_enable] =
			WMI_VDEV_PARAM_MCC_BROADCAST_PROBE_ENABLE,
	[wmi_vdev_param_mgmt_tx_power] = WMI_VDEV_PARAM_MGMT_TX_POWER,
	[wmi_vdev_param_beacon_rate] = WMI_VDEV_PARAM_BEACON_RATE,
	[wmi_vdev_param_rx_decap_type] = WMI_VDEV_PARAM_RX_DECAP_TYPE,
	[wmi_vdev_param_he_dcm_enable] = WMI_VDEV_PARAM_HE_DCM,
	[wmi_vdev_param_he_range_ext_enable] = WMI_VDEV_PARAM_HE_RANGE_EXT,
	[wmi_vdev_param_he_bss_color] = WMI_VDEV_PARAM_BSS_COLOR,
	[wmi_vdev_param_set_hemu_mode] = WMI_VDEV_PARAM_SET_HEMU_MODE,
	[wmi_vdev_param_set_he_sounding_mode] =
			WMI_VDEV_PARAM_SET_HE_SOUNDING_MODE,
	[wmi_vdev_param_set_heop] = WMI_VDEV_PARAM_HEOPS_0_31,
	[wmi_vdev_param_sensor_ap] = WMI_VDEV_PARAM_SENSOR_AP,
	[wmi_vdev_param_dtim_enable_cts] = WMI_VDEV_PARAM_DTIM_ENABLE_CTS,
	[wmi_vdev_param_atf_ssid_sched_policy] =
			WMI_VDEV_PARAM_ATF_SSID_SCHED_POLICY,
	[wmi_vdev_param_disable_dyn_bw_rts] = WMI_VDEV_PARAM_DISABLE_DYN_BW_RTS,
	[wmi_vdev_param_mcast2ucast_set] = WMI_VDEV_PARAM_MCAST2UCAST_SET,
	[wmi_vdev_param_rc_num_retries] = WMI_VDEV_PARAM_RC_NUM_RETRIES,
	[wmi_vdev_param_cabq_maxdur] = WMI_VDEV_PARAM_CABQ_MAXDUR,
	[wmi_vdev_param_mfptest_set] = WMI_VDEV_PARAM_MFPTEST_SET,
	[wmi_vdev_param_rts_fixed_rate] = WMI_VDEV_PARAM_RTS_FIXED_RATE,
	[wmi_vdev_param_vht_sgimask] = WMI_VDEV_PARAM_VHT_SGIMASK,
	[wmi_vdev_param_vht80_ratemask] = WMI_VDEV_PARAM_VHT80_RATEMASK,
	[wmi_vdev_param_proxy_sta] = WMI_VDEV_PARAM_PROXY_STA,
	[wmi_vdev_param_bw_nss_ratemask] = WMI_VDEV_PARAM_BW_NSS_RATEMASK,
	[wmi_vdev_param_set_he_ltf] = WMI_VDEV_PARAM_HE_LTF,
	[wmi_vdev_param_disable_cabq] = WMI_VDEV_PARAM_DISABLE_CABQ,
	[wmi_vdev_param_rate_dropdown_bmap] = WMI_VDEV_PARAM_RATE_DROPDOWN_BMAP,
	[wmi_vdev_param_set_ba_mode] = WMI_VDEV_PARAM_BA_MODE,
	[wmi_vdev_param_capabilities] = WMI_VDEV_PARAM_CAPABILITIES,
	[wmi_vdev_param_autorate_misc_cfg] = WMI_VDEV_PARAM_AUTORATE_MISC_CFG,
	[wmi_vdev_param_ul_shortgi] = WMI_VDEV_PARAM_UL_GI,
	[wmi_vdev_param_ul_he_ltf] = WMI_VDEV_PARAM_UL_HE_LTF,
	[wmi_vdev_param_ul_nss] = WMI_VDEV_PARAM_UL_NSS,
	[wmi_vdev_param_ul_ppdu_bw] = WMI_VDEV_PARAM_UL_PPDU_BW,
	[wmi_vdev_param_ul_ldpc] = WMI_VDEV_PARAM_UL_LDPC,
	[wmi_vdev_param_ul_stbc] = WMI_VDEV_PARAM_UL_STBC,
	[wmi_vdev_param_ul_fixed_rate] = WMI_VDEV_PARAM_UL_FIXED_RATE,
	[wmi_vdev_param_rawmode_open_war] = WMI_VDEV_PARAM_RAW_IS_ENCRYPTED,
};
#endif

/**
 * convert_host_pdev_id_to_target_pdev_id() - Convert pdev_id from
 *	   host to target defines.
 * @param pdev_id: host pdev_id to be converted.
 * Return: target pdev_id after conversion.
 */
static uint32_t convert_host_pdev_id_to_target_pdev_id(uint32_t pdev_id)
{
	switch (pdev_id) {
	case WMI_HOST_PDEV_ID_SOC:
		return WMI_PDEV_ID_SOC;
	case WMI_HOST_PDEV_ID_0:
		return WMI_PDEV_ID_1ST;
	case WMI_HOST_PDEV_ID_1:
		return WMI_PDEV_ID_2ND;
	case WMI_HOST_PDEV_ID_2:
		return WMI_PDEV_ID_3RD;
	}

	QDF_ASSERT(0);

	return WMI_PDEV_ID_SOC;
}

/**
 * convert_target_pdev_id_to_host_pdev_id() - Convert pdev_id from
 *	   target to host defines.
 * @param pdev_id: target pdev_id to be converted.
 * Return: host pdev_id after conversion.
 */
static uint32_t convert_target_pdev_id_to_host_pdev_id(uint32_t pdev_id)
{
	switch (pdev_id) {
	case WMI_PDEV_ID_SOC:
		return WMI_HOST_PDEV_ID_SOC;
	case WMI_PDEV_ID_1ST:
		return WMI_HOST_PDEV_ID_0;
	case WMI_PDEV_ID_2ND:
		return WMI_HOST_PDEV_ID_1;
	case WMI_PDEV_ID_3RD:
		return WMI_HOST_PDEV_ID_2;
	}

	WMI_LOGE("Invalid pdev_id");

	return WMI_HOST_PDEV_ID_INVALID;
}

/**
 * wmi_tlv_pdev_id_conversion_enable() - Enable pdev_id conversion
 *
 * Return None.
 */
static void wmi_tlv_pdev_id_conversion_enable(wmi_unified_t wmi_handle)
{
	wmi_handle->ops->convert_pdev_id_host_to_target =
		convert_host_pdev_id_to_target_pdev_id;
	wmi_handle->ops->convert_pdev_id_target_to_host =
		convert_target_pdev_id_to_host_pdev_id;
}

/* copy_vdev_create_pdev_id() - copy pdev from host params to target command
 *			      buffer.
 * @wmi_handle: pointer to wmi_handle
 * @cmd: pointer target vdev create command buffer
 * @param: pointer host params for vdev create
 *
 * Return: None
 */
#ifdef CONFIG_MCL
static inline void copy_vdev_create_pdev_id(
		struct wmi_unified *wmi_handle,
		wmi_vdev_create_cmd_fixed_param * cmd,
		struct vdev_create_params *param)
{
	cmd->pdev_id = WMI_PDEV_ID_SOC;
}
#else
static inline void copy_vdev_create_pdev_id(
		struct wmi_unified *wmi_handle,
		wmi_vdev_create_cmd_fixed_param * cmd,
		struct vdev_create_params *param)
{
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							param->pdev_id);
}
#endif

void wmi_mtrace(uint32_t message_id, uint16_t vdev_id, uint32_t data)
{
	uint16_t mtrace_message_id;

	mtrace_message_id = QDF_WMI_MTRACE_CMD_ID(message_id) |
		(QDF_WMI_MTRACE_GRP_ID(message_id) <<
						QDF_WMI_MTRACE_CMD_NUM_BITS);
	qdf_mtrace(QDF_MODULE_ID_WMI, QDF_MODULE_ID_TARGET,
		   mtrace_message_id, vdev_id, data);
}

qdf_export_symbol(wmi_mtrace);

/**
 * send_vdev_create_cmd_tlv() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_create_cmd_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				 struct vdev_create_params *param)
{
	wmi_vdev_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);
	QDF_STATUS ret;
	int num_bands = 2;
	uint8_t *buf_ptr;
	wmi_vdev_txrx_streams *txrx_streams;

	len += (num_bands * sizeof(*txrx_streams) + WMI_TLV_HDR_SIZE);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_create_cmd_fixed_param));
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
	cmd->vdev_id = param->vdev_id;
#else
	cmd->vdev_id = param->if_id;
#endif
	cmd->vdev_type = param->type;
	cmd->vdev_subtype = param->subtype;
	cmd->flags = param->mbssid_flags;
	cmd->vdevid_trans = param->vdevid_trans;
	cmd->num_cfg_txrx_streams = num_bands;
	copy_vdev_create_pdev_id(wmi_handle, cmd, param);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	WMI_LOGD("%s: ID = %d[pdev:%d] VAP Addr = %02x:%02x:%02x:%02x:%02x:%02x",
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
		 __func__, param->vdev_id, cmd->pdev_id,
#else
		 __func__, param->if_id, cmd->pdev_id,
#endif
		 macaddr[0], macaddr[1], macaddr[2],
		 macaddr[3], macaddr[4], macaddr[5]);
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			(num_bands * sizeof(wmi_vdev_txrx_streams)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMI_LOGD("%s: type %d, subtype %d, nss_2g %d, nss_5g %d", __func__,
			param->type, param->subtype,
			param->nss_2g, param->nss_5g);
	txrx_streams = (wmi_vdev_txrx_streams *)buf_ptr;
	txrx_streams->band = WMI_TPC_CHAINMASK_CONFIG_BAND_2G;
	txrx_streams->supported_tx_streams = param->nss_2g;
	txrx_streams->supported_rx_streams = param->nss_2g;
	WMITLV_SET_HDR(&txrx_streams->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_txrx_streams,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_txrx_streams));

	txrx_streams++;
	txrx_streams->band = WMI_TPC_CHAINMASK_CONFIG_BAND_5G;
	txrx_streams->supported_tx_streams = param->nss_5g;
	txrx_streams->supported_rx_streams = param->nss_5g;
	WMITLV_SET_HDR(&txrx_streams->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_txrx_streams,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_txrx_streams));
	wmi_mtrace(WMI_VDEV_CREATE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_CREATE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_VDEV_CREATE_CMDID");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_vdev_delete_cmd_tlv() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_delete_cmd_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id)
{
	wmi_vdev_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_delete_cmd_fixed_param));
	cmd->vdev_id = if_id;
	wmi_mtrace(WMI_VDEV_DELETE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(wmi_vdev_delete_cmd_fixed_param),
				   WMI_VDEV_DELETE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_VDEV_DELETE_CMDID");
		wmi_buf_free(buf);
	}
	WMI_LOGD("%s:vdev id = %d", __func__, if_id);

	return ret;
}

/**
 * send_vdev_nss_chain_params_cmd_tlv() - send VDEV nss chain params to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @nss_chains_user_cfg: user configured nss chain params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_vdev_nss_chain_params_cmd_tlv(wmi_unified_t wmi_handle,
				   uint8_t vdev_id,
				   struct vdev_nss_chains *user_cfg)
{
	wmi_vdev_chainmask_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_chainmask_config_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_vdev_chainmask_config_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_chainmask_config_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->disable_rx_mrc_2g = user_cfg->disable_rx_mrc[NSS_CHAINS_BAND_2GHZ];
	cmd->disable_tx_mrc_2g = user_cfg->disable_tx_mrc[NSS_CHAINS_BAND_2GHZ];
	cmd->disable_rx_mrc_5g = user_cfg->disable_rx_mrc[NSS_CHAINS_BAND_5GHZ];
	cmd->disable_tx_mrc_5g = user_cfg->disable_tx_mrc[NSS_CHAINS_BAND_5GHZ];
	cmd->num_rx_chains_2g = user_cfg->num_rx_chains[NSS_CHAINS_BAND_2GHZ];
	cmd->num_tx_chains_2g = user_cfg->num_tx_chains[NSS_CHAINS_BAND_2GHZ];
	cmd->num_rx_chains_5g = user_cfg->num_rx_chains[NSS_CHAINS_BAND_5GHZ];
	cmd->num_tx_chains_5g = user_cfg->num_tx_chains[NSS_CHAINS_BAND_5GHZ];
	cmd->rx_nss_2g = user_cfg->rx_nss[NSS_CHAINS_BAND_2GHZ];
	cmd->tx_nss_2g = user_cfg->tx_nss[NSS_CHAINS_BAND_2GHZ];
	cmd->rx_nss_5g = user_cfg->rx_nss[NSS_CHAINS_BAND_5GHZ];
	cmd->tx_nss_5g = user_cfg->tx_nss[NSS_CHAINS_BAND_5GHZ];
	cmd->num_tx_chains_a = user_cfg->num_tx_chains_11a;
	cmd->num_tx_chains_b = user_cfg->num_tx_chains_11b;
	cmd->num_tx_chains_g = user_cfg->num_tx_chains_11g;

	wmi_mtrace(WMI_VDEV_CHAINMASK_CONFIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf,
			sizeof(wmi_vdev_chainmask_config_cmd_fixed_param),
			WMI_VDEV_CHAINMASK_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_VDEV_CHAINMASK_CONFIG_CMDID");
		wmi_buf_free(buf);
	}
	WMI_LOGD("%s: vdev_id %d", __func__, vdev_id);

	return ret;
}

/**
 * send_vdev_stop_cmd_tlv() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or erro code
 */
static QDF_STATUS send_vdev_stop_cmd_tlv(wmi_unified_t wmi,
					uint8_t vdev_id)
{
	wmi_vdev_stop_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_stop_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_stop_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	wmi_mtrace(WMI_VDEV_STOP_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_STOP_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev stop command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s:vdev id = %d", __func__, vdev_id);

	return 0;
}

/**
 * send_vdev_down_cmd_tlv() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_down_cmd_tlv(wmi_unified_t wmi, uint8_t vdev_id)
{
	wmi_vdev_down_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_down_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_down_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_down_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	wmi_mtrace(WMI_VDEV_DOWN_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_DOWN_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev down", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: vdev_id %d", __func__, vdev_id);

	return 0;
}

static inline void copy_channel_info(
		wmi_vdev_start_request_cmd_fixed_param * cmd,
		wmi_channel *chan,
		struct vdev_start_params *req)
{
	chan->mhz = req->channel.mhz;

	WMI_SET_CHANNEL_MODE(chan, req->channel.phy_mode);

	chan->band_center_freq1 = req->channel.cfreq1;
	chan->band_center_freq2 = req->channel.cfreq2;

	if (req->channel.half_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_HALF_RATE);
	else if (req->channel.quarter_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_QUARTER_RATE);

	if (req->channel.dfs_set) {
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_DFS);
		cmd->disable_hw_ack = req->disable_hw_ack;
	}

	if (req->channel.dfs_set_cfreq2)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_DFS_CFREQ2);

	/* According to firmware both reg power and max tx power
	 * on set channel power is used and set it to max reg
	 * power from regulatory.
	 */
	WMI_SET_CHANNEL_MIN_POWER(chan, req->channel.minpower);
	WMI_SET_CHANNEL_MAX_POWER(chan, req->channel.maxpower);
	WMI_SET_CHANNEL_REG_POWER(chan, req->channel.maxregpower);
	WMI_SET_CHANNEL_ANTENNA_MAX(chan, req->channel.antennamax);
	WMI_SET_CHANNEL_REG_CLASSID(chan, req->channel.reg_class_id);
	WMI_SET_CHANNEL_MAX_TX_POWER(chan, req->channel.maxregpower);

}

/**
 * send_vdev_start_cmd_tlv() - send vdev start request to fw
 * @wmi_handle: wmi handle
 * @req: vdev start params
 *
 * Return: QDF status
 */
static QDF_STATUS send_vdev_start_cmd_tlv(wmi_unified_t wmi_handle,
			  struct vdev_start_params *req)
{
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	wmi_channel *chan;
	int32_t len, ret;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + sizeof(wmi_channel) + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_start_request_cmd_fixed_param *) buf_ptr;
	chan = (wmi_channel *) (buf_ptr + sizeof(*cmd));
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_start_request_cmd_fixed_param));
	WMITLV_SET_HDR(&chan->tlv_header, WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
	cmd->vdev_id = req->vdev_id;

	/* Fill channel info */
	copy_channel_info(cmd, chan, req);
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
	cmd->beacon_interval = req->beacon_interval;
#else
	cmd->beacon_interval = req->beacon_intval;
#endif
	cmd->dtim_period = req->dtim_period;

	cmd->bcn_tx_rate = req->bcn_tx_rate_code;
	if (req->bcn_tx_rate_code)
		cmd->flags |= WMI_UNIFIED_VDEV_START_BCN_TX_RATE_PRESENT;

	if (!req->is_restart) {
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
		cmd->beacon_interval = req->beacon_interval;
#else
		cmd->beacon_interval = req->beacon_intval;
#endif
		cmd->dtim_period = req->dtim_period;

		/* Copy the SSID */
		if (req->ssid.length) {
			if (req->ssid.length < sizeof(cmd->ssid.ssid))
				cmd->ssid.ssid_len = req->ssid.length;
			else
				cmd->ssid.ssid_len = sizeof(cmd->ssid.ssid);
			qdf_mem_copy(cmd->ssid.ssid, req->ssid.mac_ssid,
				     cmd->ssid.ssid_len);
		}

		if (req->pmf_enabled)
			cmd->flags |= WMI_UNIFIED_VDEV_START_PMF_ENABLED;
	}

	if (req->hidden_ssid)
		cmd->flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;

	cmd->flags |= WMI_UNIFIED_VDEV_START_LDPC_RX_ENABLED;
	cmd->num_noa_descriptors = req->num_noa_descriptors;
	cmd->preferred_rx_streams = req->preferred_rx_streams;
	cmd->preferred_tx_streams = req->preferred_tx_streams;
	cmd->cac_duration_ms = req->cac_duration_ms;
	cmd->regdomain = req->regdomain;
	cmd->he_ops = req->he_ops;

	buf_ptr = (uint8_t *) (((uintptr_t) cmd) + sizeof(*cmd) +
			       sizeof(wmi_channel));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->num_noa_descriptors *
		       sizeof(wmi_p2p_noa_descriptor));
	WMI_LOGI("%s: vdev_id %d freq %d chanmode %d ch_info: 0x%x is_dfs %d "
		"beacon interval %d dtim %d center_chan %d center_freq2 %d "
		"reg_info_1: 0x%x reg_info_2: 0x%x, req->max_txpow: 0x%x "
		"Tx SS %d, Rx SS %d, ldpc_rx: %d, cac %d, regd %d, HE ops: %d"
		"req->dis_hw_ack: %d ", __func__, req->vdev_id,
		chan->mhz, req->channel.phy_mode, chan->info,
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
		req->channel.dfs_set, req->beacon_interval, cmd->dtim_period,
#else
		req->channel.dfs_set, req->beacon_intval, cmd->dtim_period,
#endif
		chan->band_center_freq1, chan->band_center_freq2,
		chan->reg_info_1, chan->reg_info_2, req->channel.maxregpower,
		req->preferred_tx_streams, req->preferred_rx_streams,
		req->ldpc_rx_enabled, req->cac_duration_ms,
		req->regdomain, req->he_ops,
		req->disable_hw_ack);

	if (req->is_restart) {
		wmi_mtrace(WMI_VDEV_RESTART_REQUEST_CMDID, cmd->vdev_id, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_VDEV_RESTART_REQUEST_CMDID);
	} else {
		wmi_mtrace(WMI_VDEV_START_REQUEST_CMDID, cmd->vdev_id, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_VDEV_START_REQUEST_CMDID);
	}
	if (ret) {
		WMI_LOGP("%s: Failed to send vdev start command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	 }

	return QDF_STATUS_SUCCESS;
}

/**
 * send_hidden_ssid_vdev_restart_cmd_tlv() - restart vdev to set hidden ssid
 * @wmi_handle: wmi handle
 * @restart_params: vdev restart params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_hidden_ssid_vdev_restart_cmd_tlv(wmi_unified_t wmi_handle,
			struct hidden_ssid_vdev_restart_params *restart_params)
{
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	wmi_channel *chan;
	int32_t len;
	uint8_t *buf_ptr;
	QDF_STATUS ret = 0;

	len = sizeof(*cmd) + sizeof(wmi_channel) + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_start_request_cmd_fixed_param *) buf_ptr;
	chan = (wmi_channel *) (buf_ptr + sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_start_request_cmd_fixed_param));

	WMITLV_SET_HDR(&chan->tlv_header,
		       WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));

	cmd->vdev_id = restart_params->vdev_id;
	cmd->ssid.ssid_len = restart_params->ssid_len;
	qdf_mem_copy(cmd->ssid.ssid,
		     restart_params->ssid,
		     cmd->ssid.ssid_len);
	cmd->flags = restart_params->flags;
	cmd->requestor_id = restart_params->requestor_id;
	cmd->disable_hw_ack = restart_params->disable_hw_ack;

	chan->mhz = restart_params->mhz;
	chan->band_center_freq1 =
			restart_params->band_center_freq1;
	chan->band_center_freq2 =
			restart_params->band_center_freq2;
	chan->info = restart_params->info;
	chan->reg_info_1 = restart_params->reg_info_1;
	chan->reg_info_2 = restart_params->reg_info_2;

	cmd->num_noa_descriptors = 0;
	buf_ptr = (uint8_t *) (((uint8_t *) cmd) + sizeof(*cmd) +
			       sizeof(wmi_channel));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->num_noa_descriptors *
		       sizeof(wmi_p2p_noa_descriptor));

	wmi_mtrace(WMI_VDEV_RESTART_REQUEST_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_RESTART_REQUEST_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}


/**
 * send_peer_flush_tids_cmd_tlv() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_flush_tids_cmd_tlv(wmi_unified_t wmi,
					 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
					 struct peer_flush_params *param)
{
	wmi_peer_flush_tids_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_flush_tids_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_flush_tids_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_flush_tids_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = param->peer_tid_bitmap;
	cmd->vdev_id = param->vdev_id;
	WMI_LOGD("%s: peer_addr %pM vdev_id %d and peer bitmap %d", __func__,
				peer_addr, param->vdev_id,
				param->peer_tid_bitmap);
	wmi_mtrace(WMI_PEER_FLUSH_TIDS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_TIDS_CMDID)) {
		WMI_LOGP("%s: Failed to send flush tid command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_peer_delete_cmd_tlv() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_delete_cmd_tlv(wmi_unified_t wmi,
				 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				 uint8_t vdev_id)
{
	wmi_peer_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_delete_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;

	WMI_LOGD("%s: peer_addr %pM vdev_id %d", __func__, peer_addr, vdev_id);
	wmi_mtrace(WMI_PEER_DELETE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_DELETE_CMDID)) {
		WMI_LOGP("%s: Failed to send peer delete command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * crash_on_send_peer_rx_reorder_queue_remove_cmd() - crash on reorder queue cmd
 *
 * On MCL side, we are suspecting this cmd to trigger drop of ARP
 * response frames from REO by the FW. This function causes a crash if this
 * command is sent out by the host, so we can track this issue. Ideally no one
 * should be calling this API from the MCL side
 *
 * Return: None
 */
#ifdef CONFIG_MCL
static void crash_on_send_peer_rx_reorder_queue_remove_cmd(void)
{
	QDF_BUG(0);
}
#else
static void crash_on_send_peer_rx_reorder_queue_remove_cmd(void)
{
	/* No-OP */
}
#endif

/**
 * convert_host_peer_param_id_to_target_id_tlv - convert host peer param_id
 * to target id.
 * @peer_param_id: host param id.
 *
 * Return: Target param id.
 */
#ifdef ENABLE_HOST_TO_TARGET_CONVERSION
static inline uint32_t convert_host_peer_param_id_to_target_id_tlv(
		uint32_t peer_param_id)
{
	if (peer_param_id < QDF_ARRAY_SIZE(peer_param_tlv))
		return peer_param_tlv[peer_param_id];
	return WMI_UNAVAILABLE_PARAM;
}
#else
static inline uint32_t convert_host_peer_param_id_to_target_id_tlv(
		uint32_t peer_param_id)
{
	return peer_param_id;
}
#endif

/**
 * send_peer_param_cmd_tlv() - set peer parameter in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param    : pointer to hold peer set parameter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_param_cmd_tlv(wmi_unified_t wmi,
				uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				struct peer_set_params *param)
{
	wmi_peer_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;
	uint32_t param_id;

	param_id = convert_host_peer_param_id_to_target_id_tlv(param->param_id);
	if (param_id == WMI_UNAVAILABLE_PARAM) {
		WMI_LOGW("%s: Unavailable param %d", __func__, param->param_id);
		return QDF_STATUS_E_NOSUPPORT;
	}

	buf = wmi_buf_alloc(wmi, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_set_param_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param_id = param_id;
	cmd->param_value = param->param_value;
	wmi_mtrace(WMI_PEER_SET_PARAM_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi, buf,
				   sizeof(wmi_peer_set_param_cmd_fixed_param),
				   WMI_PEER_SET_PARAM_CMDID);
	if (err) {
		WMI_LOGE("Failed to send set_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_vdev_up_cmd_tlv() - send vdev up command in fw
 * @wmi: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_up_cmd_tlv(wmi_unified_t wmi,
			     uint8_t bssid[QDF_MAC_ADDR_SIZE],
				 struct vdev_up_params *params)
{
	wmi_vdev_up_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("%s: VDEV_UP", __func__);
	WMI_LOGD("%s: vdev_id %d aid %d bssid %pM", __func__,
		 params->vdev_id, params->assoc_id, bssid);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_up_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_up_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_up_cmd_fixed_param));
	cmd->vdev_id = params->vdev_id;
	cmd->vdev_assoc_id = params->assoc_id;
	cmd->profile_idx = params->profile_idx;
	cmd->profile_num = params->profile_num;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->trans_bssid, &cmd->trans_bssid);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid, &cmd->vdev_bssid);
	wmi_mtrace(WMI_VDEV_UP_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_UP_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev up command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_peer_create_cmd_tlv() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_create_cmd_tlv(wmi_unified_t wmi,
					struct peer_create_params *param)
{
	wmi_peer_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_create_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);
	cmd->peer_type = param->peer_type;
	cmd->vdev_id = param->vdev_id;

	wmi_mtrace(WMI_PEER_CREATE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_CREATE_CMDID)) {
		WMI_LOGP("%s: failed to send WMI_PEER_CREATE_CMDID", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: peer_addr %pM vdev_id %d", __func__, param->peer_addr,
			param->vdev_id);

	return 0;
}

/**
 * send_peer_rx_reorder_queue_setup_cmd_tlv() - send rx reorder setup
 * 	command to fw
 * @wmi: wmi handle
 * @rx_reorder_queue_setup_params: Rx reorder queue setup parameters
 *
 * Return: 0 for success or error code
 */
static
QDF_STATUS send_peer_rx_reorder_queue_setup_cmd_tlv(wmi_unified_t wmi,
		struct rx_reorder_queue_setup_params *param)
{
	wmi_peer_reorder_queue_setup_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_reorder_queue_setup_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_peer_reorder_queue_setup_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_peer_reorder_queue_setup_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_macaddr, &cmd->peer_macaddr);
	cmd->vdev_id = param->vdev_id;
	cmd->tid = param->tid;
	cmd->queue_ptr_lo = param->hw_qdesc_paddr_lo;
	cmd->queue_ptr_hi = param->hw_qdesc_paddr_hi;
	cmd->queue_no = param->queue_no;
	cmd->ba_window_size_valid = param->ba_window_size_valid;
	cmd->ba_window_size = param->ba_window_size;


	wmi_mtrace(WMI_PEER_REORDER_QUEUE_SETUP_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len,
			WMI_PEER_REORDER_QUEUE_SETUP_CMDID)) {
		WMI_LOGP("%s: fail to send WMI_PEER_REORDER_QUEUE_SETUP_CMDID",
			__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: peer_macaddr %pM vdev_id %d, tid %d", __func__,
		param->peer_macaddr, param->vdev_id, param->tid);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_peer_rx_reorder_queue_remove_cmd_tlv() - send rx reorder remove
 * 	command to fw
 * @wmi: wmi handle
 * @rx_reorder_queue_remove_params: Rx reorder queue remove parameters
 *
 * Return: 0 for success or error code
 */
static
QDF_STATUS send_peer_rx_reorder_queue_remove_cmd_tlv(wmi_unified_t wmi,
		struct rx_reorder_queue_remove_params *param)
{
	wmi_peer_reorder_queue_remove_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	crash_on_send_peer_rx_reorder_queue_remove_cmd();

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_reorder_queue_remove_cmd_fixed_param *)
			wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_peer_reorder_queue_remove_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_peer_reorder_queue_remove_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_macaddr, &cmd->peer_macaddr);
	cmd->vdev_id = param->vdev_id;
	cmd->tid_mask = param->peer_tid_bitmap;

	wmi_mtrace(WMI_PEER_REORDER_QUEUE_REMOVE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len,
			WMI_PEER_REORDER_QUEUE_REMOVE_CMDID)) {
		WMI_LOGP("%s: fail to send WMI_PEER_REORDER_QUEUE_REMOVE_CMDID",
			__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: peer_macaddr %pM vdev_id %d, tid_map %d", __func__,
		param->peer_macaddr, param->vdev_id, param->peer_tid_bitmap);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_GREEN_AP
/**
 * send_green_ap_ps_cmd_tlv() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @pdev_id: pdev id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_green_ap_ps_cmd_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t pdev_id)
{
	wmi_pdev_green_ap_ps_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("Set Green AP PS val %d", value);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_green_ap_ps_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		   WMITLV_TAG_STRUC_wmi_pdev_green_ap_ps_enable_cmd_fixed_param,
		   WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_green_ap_ps_enable_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);
	cmd->enable = value;

	wmi_mtrace(WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID)) {
		WMI_LOGE("Set Green AP PS param Failed val %d", value);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}
#endif

/**
 * send_pdev_utf_cmd_tlv() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_pdev_utf_cmd_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	/* if param->len is 0 no data is sent, return error */
	QDF_STATUS ret = QDF_STATUS_E_INVAL;
	static uint8_t msgref = 1;
	uint8_t segNumber = 0, segInfo, numSegments;
	uint16_t chunk_len, total_bytes;
	uint8_t *bufpos;
	struct seg_hdr_info segHdrInfo;

	bufpos = param->utf_payload;
	total_bytes = param->len;
	ASSERT(total_bytes / MAX_WMI_UTF_LEN ==
	       (uint8_t) (total_bytes / MAX_WMI_UTF_LEN));
	numSegments = (uint8_t) (total_bytes / MAX_WMI_UTF_LEN);

	if (param->len - (numSegments * MAX_WMI_UTF_LEN))
		numSegments++;

	while (param->len) {
		if (param->len > MAX_WMI_UTF_LEN)
			chunk_len = MAX_WMI_UTF_LEN;    /* MAX message */
		else
			chunk_len = param->len;

		buf = wmi_buf_alloc(wmi_handle,
				    (chunk_len + sizeof(segHdrInfo) +
				     WMI_TLV_HDR_SIZE));
		if (!buf)
			return QDF_STATUS_E_NOMEM;

		cmd = (uint8_t *) wmi_buf_data(buf);

		segHdrInfo.len = total_bytes;
		segHdrInfo.msgref = msgref;
		segInfo = ((numSegments << 4) & 0xF0) | (segNumber & 0xF);
		segHdrInfo.segmentInfo = segInfo;
		segHdrInfo.pad = 0;

		WMI_LOGD("%s:segHdrInfo.len = %d, segHdrInfo.msgref = %d,"
			 " segHdrInfo.segmentInfo = %d",
			 __func__, segHdrInfo.len, segHdrInfo.msgref,
			 segHdrInfo.segmentInfo);

		WMI_LOGD("%s:total_bytes %d segNumber %d totalSegments %d"
			 "chunk len %d", __func__, total_bytes, segNumber,
			 numSegments, chunk_len);

		segNumber++;

		WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE,
			       (chunk_len + sizeof(segHdrInfo)));
		cmd += WMI_TLV_HDR_SIZE;
		memcpy(cmd, &segHdrInfo, sizeof(segHdrInfo));   /* 4 bytes */
		memcpy(&cmd[sizeof(segHdrInfo)], bufpos, chunk_len);

		wmi_mtrace(WMI_PDEV_UTF_CMDID, NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf,
					   (chunk_len + sizeof(segHdrInfo) +
					    WMI_TLV_HDR_SIZE),
					   WMI_PDEV_UTF_CMDID);

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMI_LOGE("Failed to send WMI_PDEV_UTF_CMDID command");
			wmi_buf_free(buf);
			break;
		}

		param->len -= chunk_len;
		bufpos += chunk_len;
	}

	msgref++;

	return ret;
}

#ifdef ENABLE_HOST_TO_TARGET_CONVERSION
static inline uint32_t convert_host_pdev_param_tlv(uint32_t host_param)
{
	if (host_param < QDF_ARRAY_SIZE(pdev_param_tlv))
		return pdev_param_tlv[host_param];
	return WMI_UNAVAILABLE_PARAM;
}
#else
static inline uint32_t convert_host_pdev_param_tlv(uint32_t host_param)
{
	return host_param;
}
#endif

/**
 * send_pdev_param_cmd_tlv() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: 0 on success, errno on failure
 */
static QDF_STATUS
send_pdev_param_cmd_tlv(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id)
{
	QDF_STATUS ret;
	wmi_pdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);
	uint32_t pdev_param;

	pdev_param = convert_host_pdev_param_tlv(param->param_id);
	if (pdev_param == WMI_UNAVAILABLE_PARAM) {
		WMI_LOGW("%s: Unavailable param %d",
				__func__, param->param_id);
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_param_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(mac_id);
	cmd->param_id = pdev_param;
	cmd->param_value = param->param_value;
	WMI_LOGD("Setting pdev param = %x, value = %u", param->param_id,
				param->param_value);
	wmi_mtrace(WMI_PDEV_SET_PARAM_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_suspend_cmd_tlv() - WMI suspend function
 * @param wmi_handle      : handle to WMI.
 * @param param    : pointer to hold suspend parameter
 * @mac_id: radio context
 *
 * Return 0  on success and -ve on failure.
 */
static QDF_STATUS send_suspend_cmd_tlv(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id)
{
	wmi_pdev_suspend_cmd_fixed_param *cmd;
	wmi_buf_t wmibuf;
	uint32_t len = sizeof(*cmd);
	int32_t ret;

	/*
	 * send the command to Target to ignore the
	 * PCIE reset so as to ensure that Host and target
	 * states are in sync
	 */
	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (!wmibuf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_suspend_cmd_fixed_param *) wmi_buf_data(wmibuf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_suspend_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_suspend_cmd_fixed_param));
	if (param->disable_target_intr)
		cmd->suspend_opt = WMI_PDEV_SUSPEND_AND_DISABLE_INTR;
	else
		cmd->suspend_opt = WMI_PDEV_SUSPEND;

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(mac_id);

	wmi_mtrace(WMI_PDEV_SUSPEND_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, len,
				 WMI_PDEV_SUSPEND_CMDID);
	if (ret) {
		wmi_buf_free(wmibuf);
		WMI_LOGE("Failed to send WMI_PDEV_SUSPEND_CMDID command");
	}

	return ret;
}

/**
 * send_resume_cmd_tlv() - WMI resume function
 * @param wmi_handle      : handle to WMI.
 * @mac_id: radio context
 *
 * Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_resume_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id)
{
	wmi_buf_t wmibuf;
	wmi_pdev_resume_cmd_fixed_param *cmd;
	QDF_STATUS ret;

	wmibuf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!wmibuf)
		return QDF_STATUS_E_NOMEM;
	cmd = (wmi_pdev_resume_cmd_fixed_param *) wmi_buf_data(wmibuf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_resume_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_resume_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(mac_id);
	wmi_mtrace(WMI_PDEV_RESUME_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, sizeof(*cmd),
				   WMI_PDEV_RESUME_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_PDEV_RESUME_CMDID command");
		wmi_buf_free(wmibuf);
	}

	return ret;
}

/**
 *  send_wow_enable_cmd_tlv() - WMI wow enable function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wow enable parameter
 *  @mac_id: radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_wow_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id)
{
	wmi_wow_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int32_t ret;

	len = sizeof(wmi_wow_enable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_wow_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_wow_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_wow_enable_cmd_fixed_param));
	cmd->enable = param->enable;
	if (param->can_suspend_link)
		cmd->pause_iface_config = WOW_IFACE_PAUSE_ENABLED;
	else
		cmd->pause_iface_config = WOW_IFACE_PAUSE_DISABLED;
	cmd->flags = param->flags;

	WMI_LOGI("suspend type: %s",
		cmd->pause_iface_config == WOW_IFACE_PAUSE_ENABLED ?
		"WOW_IFACE_PAUSE_ENABLED" : "WOW_IFACE_PAUSE_DISABLED");

	wmi_mtrace(WMI_WOW_ENABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ENABLE_CMDID);
	if (ret)
		wmi_buf_free(buf);

	return ret;
}

/**
 * send_set_ap_ps_param_cmd_tlv() - set ap powersave parameters
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_ap_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	wmi_ap_ps_peer_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_ap_ps_peer_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ap_ps_peer_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_ap_ps_peer_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param = param->param;
	cmd->value = param->value;
	wmi_mtrace(WMI_AP_PS_PEER_PARAM_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd), WMI_AP_PS_PEER_PARAM_CMDID);
	if (err) {
		WMI_LOGE("Failed to send set_ap_ps_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_set_sta_ps_param_cmd_tlv() - set sta powersave parameters
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to sta_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_sta_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param)
{
	wmi_sta_powersave_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_sta_powersave_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_powersave_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_powersave_param_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
	cmd->param = param->param_id;
#else
	cmd->param = param->param;
#endif
	cmd->value = param->value;

	wmi_mtrace(WMI_STA_POWERSAVE_PARAM_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_STA_POWERSAVE_PARAM_CMDID)) {
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
		WMI_LOGE("Set Sta Ps param Failed vdevId %d Param %d val %d",
			 param->vdev_id, param->param_id, param->value);
#else
		WMI_LOGE("Set Sta Ps param Failed vdevId %d Param %d val %d",
			 param->vdev_id, param->param, param->value);
#endif
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_crash_inject_cmd_tlv() - inject fw crash
 * @wmi_handle: wmi handle
 * @param: ponirt to crash inject parameter structure
 *
 * Return: QDF_STATUS_SUCCESS for success or return error
 */
static QDF_STATUS send_crash_inject_cmd_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param)
{
	int32_t ret = 0;
	WMI_FORCE_FW_HANG_CMD_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (WMI_FORCE_FW_HANG_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_FORCE_FW_HANG_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_FORCE_FW_HANG_CMD_fixed_param));
	cmd->type = param->type;
	cmd->delay_time_ms = param->delay_time_ms;

	wmi_mtrace(WMI_FORCE_FW_HANG_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_FORCE_FW_HANG_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send set param command, ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef FEATURE_FW_LOG_PARSING
/**
 *  send_dbglog_cmd_tlv() - set debug log level
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold dbglog level parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
 static QDF_STATUS
send_dbglog_cmd_tlv(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param)
{
	wmi_buf_t buf;
	wmi_debug_log_config_cmd_fixed_param *configmsg;
	QDF_STATUS status;
	int32_t i;
	int32_t len;
	int8_t *buf_ptr;
	int32_t *module_id_bitmap_array;     /* Used to fomr the second tlv */

	ASSERT(dbglog_param->bitmap_len < MAX_MODULE_ID_BITMAP_WORDS);

	/* Allocate size for 2 tlvs - including tlv hdr space for second tlv */
	len = sizeof(wmi_debug_log_config_cmd_fixed_param) + WMI_TLV_HDR_SIZE +
	      (sizeof(int32_t) * MAX_MODULE_ID_BITMAP_WORDS);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	configmsg =
		(wmi_debug_log_config_cmd_fixed_param *) (wmi_buf_data(buf));
	buf_ptr = (int8_t *) configmsg;
	WMITLV_SET_HDR(&configmsg->tlv_header,
		       WMITLV_TAG_STRUC_wmi_debug_log_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_debug_log_config_cmd_fixed_param));
	configmsg->dbg_log_param = dbglog_param->param;
	configmsg->value = dbglog_param->val;
	/* Filling in the data part of second tlv -- should
	 * follow first tlv _ WMI_TLV_HDR_SIZE */
	module_id_bitmap_array = (uint32_t *) (buf_ptr +
				       sizeof
				       (wmi_debug_log_config_cmd_fixed_param)
				       + WMI_TLV_HDR_SIZE);
	WMITLV_SET_HDR(buf_ptr + sizeof(wmi_debug_log_config_cmd_fixed_param),
		       WMITLV_TAG_ARRAY_UINT32,
		       sizeof(uint32_t) * MAX_MODULE_ID_BITMAP_WORDS);
	if (dbglog_param->module_id_bitmap) {
		for (i = 0; i < dbglog_param->bitmap_len; ++i) {
			module_id_bitmap_array[i] =
					dbglog_param->module_id_bitmap[i];
		}
	}

	wmi_mtrace(WMI_DBGLOG_CFG_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_DBGLOG_CFG_CMDID);

	if (status != QDF_STATUS_SUCCESS)
		wmi_buf_free(buf);

	return status;
}
#endif

#ifdef ENABLE_HOST_TO_TARGET_CONVERSION
static inline uint32_t convert_host_vdev_param_tlv(uint32_t host_param)
{
	if (host_param < QDF_ARRAY_SIZE(vdev_param_tlv))
		return vdev_param_tlv[host_param];
	return WMI_UNAVAILABLE_PARAM;
}
#else
static inline uint32_t convert_host_vdev_param_tlv(uint32_t host_param)
{
	return host_param;
}
#endif

/**
 *  send_vdev_set_param_cmd_tlv() - WMI vdev set parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr	: MAC address
 *  @param param    : pointer to hold vdev set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	QDF_STATUS ret;
	wmi_vdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);
	uint32_t vdev_param;

	vdev_param = convert_host_vdev_param_tlv(param->param_id);
	if (vdev_param == WMI_UNAVAILABLE_PARAM) {
		WMI_LOGW("%s:Vdev param %d not available", __func__,
				param->param_id);
		return QDF_STATUS_E_INVAL;

	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_param_cmd_fixed_param));
#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
	cmd->vdev_id = param->vdev_id;
#else
	cmd->vdev_id = param->if_id;
#endif
	cmd->param_id = vdev_param;
	cmd->param_value = param->param_value;
	WMI_LOGD("Setting vdev %d param = %x, value = %u",
		 cmd->vdev_id, cmd->param_id, cmd->param_value);
	wmi_mtrace(WMI_VDEV_SET_PARAM_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_stats_request_cmd_tlv() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr	: MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct stats_request_params *param)
{
	int32_t ret;
	wmi_request_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return -QDF_STATUS_E_NOMEM;

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = param->stats_id;
	cmd->vdev_id = param->vdev_id;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							param->pdev_id);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	WMI_LOGD("STATS REQ STATS_ID:%d VDEV_ID:%d PDEV_ID:%d-->",
				cmd->stats_id, cmd->vdev_id, cmd->pdev_id);

	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_REQUEST_STATS_CMDID);

	if (ret) {
		WMI_LOGE("Failed to send status request to fw =%d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef CONFIG_WIN

/**
 *  send_peer_based_pktlog_cmd() - Send WMI command to enable packet-log
 *  @wmi_handle: handle to WMI.
 *  @macaddr: Peer mac address to be filter
 *  @mac_id: mac id to have radio context
 *  @enb_dsb: Enable MAC based filtering or Disable
 *
 *  Return: QDF_STATUS
 */
static QDF_STATUS send_peer_based_pktlog_cmd(wmi_unified_t wmi_handle,
					     uint8_t *macaddr,
					     uint8_t mac_id,
					     uint8_t enb_dsb)
{
	int32_t ret;
	wmi_pdev_pktlog_filter_cmd_fixed_param *cmd;
	wmi_pdev_pktlog_filter_info *mac_info;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint16_t len = sizeof(wmi_pdev_pktlog_filter_cmd_fixed_param) +
			sizeof(wmi_pdev_pktlog_filter_info) + WMI_TLV_HDR_SIZE;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_pdev_pktlog_filter_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_pktlog_filter_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_pktlog_filter_cmd_fixed_param));
	cmd->pdev_id = mac_id;
	cmd->enable = enb_dsb;
	cmd->num_of_mac_addresses = 1;
	wmi_mtrace(WMI_PDEV_PKTLOG_FILTER_CMDID, cmd->pdev_id, 0);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_pdev_pktlog_filter_info));
	buf_ptr += WMI_TLV_HDR_SIZE;

	mac_info = (wmi_pdev_pktlog_filter_info *)(buf_ptr);

	WMITLV_SET_HDR(&mac_info->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_pktlog_filter_info,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_pdev_pktlog_filter_info));

	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &mac_info->peer_mac_address);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_PKTLOG_FILTER_CMDID);
	if (ret) {
		WMI_LOGE("Failed to send peer based pktlog command to FW =%d"
			 , ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_packet_log_enable_cmd_tlv() - Send WMI command to enable packet-log
 *  @param wmi_handle      : handle to WMI.
 *  @param PKTLOG_EVENT	: packet log event
 *  @mac_id: mac id to have radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_packet_log_enable_cmd_tlv(wmi_unified_t wmi_handle,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id)
{
	int32_t ret;
	wmi_pdev_pktlog_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_pdev_pktlog_enable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return -QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_pktlog_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_pktlog_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_pktlog_enable_cmd_fixed_param));
	cmd->evlist = PKTLOG_EVENT;
	cmd->pdev_id = mac_id;
	wmi_mtrace(WMI_PDEV_PKTLOG_ENABLE_CMDID, cmd->pdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_ENABLE_CMDID);
	if (ret) {
		WMI_LOGE("Failed to send pktlog enable cmd to FW =%d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_packet_log_disable_cmd_tlv() - Send WMI command to disable packet-log
 *  @param wmi_handle      : handle to WMI.
 *  @mac_id: mac id to have radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_packet_log_disable_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t mac_id)
{
	int32_t ret;
	wmi_pdev_pktlog_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_pdev_pktlog_disable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return -QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_pktlog_disable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_pktlog_disable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_pktlog_disable_cmd_fixed_param));
	cmd->pdev_id = mac_id;
	wmi_mtrace(WMI_PDEV_PKTLOG_DISABLE_CMDID, cmd->pdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_DISABLE_CMDID);
	if (ret) {
		WMI_LOGE("Failed to send pktlog disable cmd to FW =%d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}
#else
/**
 *  send_packet_log_enable_cmd_tlv() - Send WMI command to enable
 *  packet-log
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr	: MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: QDF_STATUS.
 */
static QDF_STATUS send_packet_log_enable_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct packet_enable_params *param)
{
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_peer_based_pktlog_cmd() - Send WMI command to enable packet-log
 *  @wmi_handle: handle to WMI.
 *  @macaddr: Peer mac address to be filter
 *  @mac_id: mac id to have radio context
 *  @enb_dsb: Enable MAC based filtering or Disable
 *
 *  Return: QDF_STATUS
 */
static QDF_STATUS send_peer_based_pktlog_cmd(wmi_unified_t wmi_handle,
					     uint8_t *macaddr,
					     uint8_t mac_id,
					     uint8_t enb_dsb)
{
	return QDF_STATUS_SUCCESS;
}
/**
 *  send_packet_log_disable_cmd_tlv() - Send WMI command to disable
 *  packet-log
 *  @param wmi_handle      : handle to WMI.
 *  @mac_id: mac id to have radio context
 *
 *  Return: QDF_STATUS.
 */
static QDF_STATUS send_packet_log_disable_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#define WMI_FW_TIME_STAMP_LOW_MASK 0xffffffff
/**
 *  send_time_stamp_sync_cmd_tlv() - Send WMI command to
 *  sync time between bwtween host and firmware
 *  @param wmi_handle      : handle to WMI.
 *
 *  Return: None
 */
static void send_time_stamp_sync_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	WMI_DBGLOG_TIME_STAMP_SYNC_CMD_fixed_param *time_stamp;
	int32_t len;
	qdf_time_t time_ms;

	len = sizeof(*time_stamp);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return;

	time_stamp =
		(WMI_DBGLOG_TIME_STAMP_SYNC_CMD_fixed_param *)
			(wmi_buf_data(buf));
	WMITLV_SET_HDR(&time_stamp->tlv_header,
		WMITLV_TAG_STRUC_wmi_dbglog_time_stamp_sync_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		WMI_DBGLOG_TIME_STAMP_SYNC_CMD_fixed_param));

	time_ms = qdf_get_time_of_the_day_ms();
	time_stamp->mode = WMI_TIME_STAMP_SYNC_MODE_MS;
	time_stamp->time_stamp_low = time_ms &
		WMI_FW_TIME_STAMP_LOW_MASK;
	/*
	 * Send time_stamp_high 0 as the time converted from HR:MIN:SEC:MS to ms
	 * wont exceed 27 bit
	 */
	time_stamp->time_stamp_high = 0;
	WMI_LOGD(FL("WMA --> DBGLOG_TIME_STAMP_SYNC_CMDID mode %d time_stamp low %d high %d"),
		time_stamp->mode, time_stamp->time_stamp_low,
		time_stamp->time_stamp_high);

	wmi_mtrace(WMI_DBGLOG_TIME_STAMP_SYNC_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_DBGLOG_TIME_STAMP_SYNC_CMDID);
	if (status) {
		WMI_LOGE("Failed to send WMI_DBGLOG_TIME_STAMP_SYNC_CMDID command");
		wmi_buf_free(buf);
	}

}

/**
 *  send_beacon_send_tmpl_cmd_tlv() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_beacon_tmpl_send_cmd_tlv(wmi_unified_t wmi_handle,
				struct beacon_tmpl_params *param)
{
	int32_t ret;
	wmi_bcn_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t wmi_buf_len;

	wmi_buf_len = sizeof(wmi_bcn_tmpl_cmd_fixed_param) +
		      sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
		      param->tmpl_len_aligned;
	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_bcn_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_tmpl_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->tim_ie_offset = param->tim_ie_offset;
	cmd->mbssid_ie_offset = param->mbssid_ie_offset;
	cmd->csa_switch_count_offset = param->csa_switch_count_offset;
	cmd->ext_csa_switch_count_offset = param->ext_csa_switch_count_offset;
	cmd->esp_ie_offset = param->esp_ie_offset;
	cmd->buf_len = param->tmpl_len;
	buf_ptr += sizeof(wmi_bcn_tmpl_cmd_fixed_param);

	bcn_prb_info = (wmi_bcn_prb_info *) buf_ptr;
	WMITLV_SET_HDR(&bcn_prb_info->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_prb_info,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_prb_info));
	bcn_prb_info->caps = 0;
	bcn_prb_info->erp = 0;
	buf_ptr += sizeof(wmi_bcn_prb_info);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, param->tmpl_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->frm, param->tmpl_len);

	wmi_mtrace(WMI_BCN_TMPL_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_BCN_TMPL_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send bcn tmpl: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

	return 0;
}

static inline void copy_peer_flags_tlv(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param)
{
	/*
	 * The target only needs a subset of the flags maintained in the host.
	 * Just populate those flags and send it down
	 */
	cmd->peer_flags = 0;

	/*
	 * Do not enable HT/VHT if WMM/wme is disabled for vap.
	 */
	if (param->is_wme_set) {

		if (param->qos_flag)
			cmd->peer_flags |= WMI_PEER_QOS;
		if (param->apsd_flag)
			cmd->peer_flags |= WMI_PEER_APSD;
		if (param->ht_flag)
			cmd->peer_flags |= WMI_PEER_HT;
		if (param->bw_40)
			cmd->peer_flags |= WMI_PEER_40MHZ;
		if (param->bw_80)
			cmd->peer_flags |= WMI_PEER_80MHZ;
		if (param->bw_160)
			cmd->peer_flags |= WMI_PEER_160MHZ;

		/* Typically if STBC is enabled for VHT it should be enabled
		 * for HT as well
		 **/
		if (param->stbc_flag)
			cmd->peer_flags |= WMI_PEER_STBC;

		/* Typically if LDPC is enabled for VHT it should be enabled
		 * for HT as well
		 **/
		if (param->ldpc_flag)
			cmd->peer_flags |= WMI_PEER_LDPC;

		if (param->static_mimops_flag)
			cmd->peer_flags |= WMI_PEER_STATIC_MIMOPS;
		if (param->dynamic_mimops_flag)
			cmd->peer_flags |= WMI_PEER_DYN_MIMOPS;
		if (param->spatial_mux_flag)
			cmd->peer_flags |= WMI_PEER_SPATIAL_MUX;
		if (param->vht_flag)
			cmd->peer_flags |= WMI_PEER_VHT;
		if (param->he_flag)
			cmd->peer_flags |= WMI_PEER_HE;
		if (param->p2p_capable_sta)
			cmd->peer_flags |= WMI_PEER_IS_P2P_CAPABLE;
	}

	if (param->is_pmf_enabled)
		cmd->peer_flags |= WMI_PEER_PMF;
	/*
	 * Suppress authorization for all AUTH modes that need 4-way handshake
	 * (during re-association).
	 * Authorization will be done for these modes on key installation.
	 */
	if (param->auth_flag)
		cmd->peer_flags |= WMI_PEER_AUTH;
	if (param->need_ptk_4_way)
		cmd->peer_flags |= WMI_PEER_NEED_PTK_4_WAY;
	else
		cmd->peer_flags &= ~WMI_PEER_NEED_PTK_4_WAY;
	if (param->need_gtk_2_way)
		cmd->peer_flags |= WMI_PEER_NEED_GTK_2_WAY;
	/* safe mode bypass the 4-way handshake */
	if (param->safe_mode_enabled)
		cmd->peer_flags &=
		    ~(WMI_PEER_NEED_PTK_4_WAY | WMI_PEER_NEED_GTK_2_WAY);
	/* Disable AMSDU for station transmit, if user configures it */
	/* Disable AMSDU for AP transmit to 11n Stations, if user configures
	 * it
	 * if (param->amsdu_disable) Add after FW support
	 **/

	/* Target asserts if node is marked HT and all MCS is set to 0.
	 * Mark the node as non-HT if all the mcs rates are disabled through
	 * iwpriv
	 **/
	if (param->peer_ht_rates.num_rates == 0)
		cmd->peer_flags &= ~WMI_PEER_HT;

	if (param->twt_requester)
		cmd->peer_flags |= WMI_PEER_TWT_REQ;

	if (param->twt_responder)
		cmd->peer_flags |= WMI_PEER_TWT_RESP;
}

static inline void copy_peer_mac_addr_tlv(
		wmi_peer_assoc_complete_cmd_fixed_param * cmd,
		struct peer_assoc_params *param)
{
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_mac, &cmd->peer_macaddr);
}

/**
 *  send_peer_assoc_cmd_tlv() - WMI peer assoc function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to peer assoc parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_peer_assoc_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param)
{
	wmi_peer_assoc_complete_cmd_fixed_param *cmd;
	wmi_vht_rate_set *mcs;
	wmi_he_rate_set *he_mcs;
	wmi_buf_t buf;
	int32_t len;
	uint8_t *buf_ptr;
	QDF_STATUS ret;
	uint32_t peer_legacy_rates_align;
	uint32_t peer_ht_rates_align;
	int32_t i;


	peer_legacy_rates_align = wmi_align(param->peer_legacy_rates.num_rates);
	peer_ht_rates_align = wmi_align(param->peer_ht_rates.num_rates);

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		(peer_legacy_rates_align * sizeof(uint8_t)) +
		WMI_TLV_HDR_SIZE +
		(peer_ht_rates_align * sizeof(uint8_t)) +
		sizeof(wmi_vht_rate_set) +
		(sizeof(wmi_he_rate_set) * param->peer_he_mcs_count
		+ WMI_TLV_HDR_SIZE);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_peer_assoc_complete_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_assoc_complete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_assoc_complete_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	cmd->peer_new_assoc = param->peer_new_assoc;
	cmd->peer_associd = param->peer_associd;

	copy_peer_flags_tlv(cmd, param);
	copy_peer_mac_addr_tlv(cmd, param);

	cmd->peer_rate_caps = param->peer_rate_caps;
	cmd->peer_caps = param->peer_caps;
	cmd->peer_listen_intval = param->peer_listen_intval;
	cmd->peer_ht_caps = param->peer_ht_caps;
	cmd->peer_max_mpdu = param->peer_max_mpdu;
	cmd->peer_mpdu_density = param->peer_mpdu_density;
	cmd->peer_vht_caps = param->peer_vht_caps;
	cmd->peer_phymode = param->peer_phymode;

	/* Update 11ax capabilities */
	cmd->peer_he_cap_info =
		param->peer_he_cap_macinfo[WMI_HOST_HECAP_MAC_WORD1];
	cmd->peer_he_cap_info_ext =
		param->peer_he_cap_macinfo[WMI_HOST_HECAP_MAC_WORD2];
	cmd->peer_he_cap_info_internal = param->peer_he_cap_info_internal;
	cmd->peer_he_ops = param->peer_he_ops;
	qdf_mem_copy(&cmd->peer_he_cap_phy, &param->peer_he_cap_phyinfo,
				sizeof(param->peer_he_cap_phyinfo));
	qdf_mem_copy(&cmd->peer_ppet, &param->peer_ppet,
				sizeof(param->peer_ppet));

	/* Update peer legacy rate information */
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				peer_legacy_rates_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_legacy_rates = param->peer_legacy_rates.num_rates;
	qdf_mem_copy(buf_ptr, param->peer_legacy_rates.rates,
		     param->peer_legacy_rates.num_rates);

	/* Update peer HT rate information */
	buf_ptr += peer_legacy_rates_align;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			  peer_ht_rates_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_ht_rates = param->peer_ht_rates.num_rates;
	qdf_mem_copy(buf_ptr, param->peer_ht_rates.rates,
				 param->peer_ht_rates.num_rates);

	/* VHT Rates */
	buf_ptr += peer_ht_rates_align;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_vht_rate_set,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vht_rate_set));

	cmd->peer_nss = param->peer_nss;

	/* Update bandwidth-NSS mapping */
	cmd->peer_bw_rxnss_override = 0;
	cmd->peer_bw_rxnss_override |= param->peer_bw_rxnss_override;

	mcs = (wmi_vht_rate_set *) buf_ptr;
	if (param->vht_capable) {
		mcs->rx_max_rate = param->rx_max_rate;
		mcs->rx_mcs_set = param->rx_mcs_set;
		mcs->tx_max_rate = param->tx_max_rate;
		mcs->tx_mcs_set = param->tx_mcs_set;
	}

	/* HE Rates */
	cmd->peer_he_mcs = param->peer_he_mcs_count;
	buf_ptr += sizeof(wmi_vht_rate_set);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(param->peer_he_mcs_count * sizeof(wmi_he_rate_set)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Loop through the HE rate set */
	for (i = 0; i < param->peer_he_mcs_count; i++) {
		he_mcs = (wmi_he_rate_set *) buf_ptr;
		WMITLV_SET_HDR(he_mcs, WMITLV_TAG_STRUC_wmi_he_rate_set,
			WMITLV_GET_STRUCT_TLVLEN(wmi_he_rate_set));

		he_mcs->rx_mcs_set = param->peer_he_rx_mcs_set[i];
		he_mcs->tx_mcs_set = param->peer_he_tx_mcs_set[i];
		WMI_LOGD("%s:HE idx %d RxMCSmap %x TxMCSmap %x ", __func__,
			i, he_mcs->rx_mcs_set, he_mcs->tx_mcs_set);
		buf_ptr += sizeof(wmi_he_rate_set);
	}


	WMI_LOGD("%s: vdev_id %d associd %d peer_flags %x rate_caps %x "
		 "peer_caps %x listen_intval %d ht_caps %x max_mpdu %d "
		 "nss %d phymode %d peer_mpdu_density %d "
		 "cmd->peer_vht_caps %x "
		 "HE cap_info %x ops %x "
		 "HE cap_info_ext %x "
		 "HE phy %x  %x  %x  "
		 "peer_bw_rxnss_override %x", __func__,
		 cmd->vdev_id, cmd->peer_associd, cmd->peer_flags,
		 cmd->peer_rate_caps, cmd->peer_caps,
		 cmd->peer_listen_intval, cmd->peer_ht_caps,
		 cmd->peer_max_mpdu, cmd->peer_nss, cmd->peer_phymode,
		 cmd->peer_mpdu_density,
		 cmd->peer_vht_caps, cmd->peer_he_cap_info,
		 cmd->peer_he_ops, cmd->peer_he_cap_info_ext,
		 cmd->peer_he_cap_phy[0], cmd->peer_he_cap_phy[1],
		 cmd->peer_he_cap_phy[2],
		 cmd->peer_bw_rxnss_override);

	wmi_mtrace(WMI_PEER_ASSOC_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_ASSOC_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGP("%s: Failed to send peer assoc command ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/* copy_scan_notify_events() - Helper routine to copy scan notify events
 */
static inline void copy_scan_event_cntrl_flags(
		wmi_start_scan_cmd_fixed_param * cmd,
		struct scan_req_params *param)
{

	/* Scan events subscription */
	if (param->scan_ev_started)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_STARTED;
	if (param->scan_ev_completed)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_COMPLETED;
	if (param->scan_ev_bss_chan)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_BSS_CHANNEL;
	if (param->scan_ev_foreign_chan)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_FOREIGN_CHANNEL;
	if (param->scan_ev_dequeued)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_DEQUEUED;
	if (param->scan_ev_preempted)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_PREEMPTED;
	if (param->scan_ev_start_failed)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_START_FAILED;
	if (param->scan_ev_restarted)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_RESTARTED;
	if (param->scan_ev_foreign_chn_exit)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_FOREIGN_CHANNEL_EXIT;
	if (param->scan_ev_suspended)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_SUSPENDED;
	if (param->scan_ev_resumed)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_RESUMED;

	/** Set scan control flags */
	cmd->scan_ctrl_flags = 0;
	if (param->scan_f_passive)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_PASSIVE;
	if (param->scan_f_strict_passive_pch)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_STRICT_PASSIVE_ON_PCHN;
	if (param->scan_f_promisc_mode)
		cmd->scan_ctrl_flags |= WMI_SCAN_FILTER_PROMISCOUS;
	if (param->scan_f_capture_phy_err)
		cmd->scan_ctrl_flags |= WMI_SCAN_CAPTURE_PHY_ERROR;
	if (param->scan_f_half_rate)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_HALF_RATE_SUPPORT;
	if (param->scan_f_quarter_rate)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_QUARTER_RATE_SUPPORT;
	if (param->scan_f_cck_rates)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_CCK_RATES;
	if (param->scan_f_ofdm_rates)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_OFDM_RATES;
	if (param->scan_f_chan_stat_evnt)
		cmd->scan_ctrl_flags |= WMI_SCAN_CHAN_STAT_EVENT;
	if (param->scan_f_filter_prb_req)
		cmd->scan_ctrl_flags |= WMI_SCAN_FILTER_PROBE_REQ;
	if (param->scan_f_bcast_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_BCAST_PROBE_REQ;
	if (param->scan_f_offchan_mgmt_tx)
		cmd->scan_ctrl_flags |= WMI_SCAN_OFFCHAN_MGMT_TX;
	if (param->scan_f_offchan_data_tx)
		cmd->scan_ctrl_flags |= WMI_SCAN_OFFCHAN_DATA_TX;
	if (param->scan_f_force_active_dfs_chn)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_FORCE_ACTIVE_ON_DFS;
	if (param->scan_f_add_tpc_ie_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_TPC_IE_IN_PROBE_REQ;
	if (param->scan_f_add_ds_ie_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_DS_IE_IN_PROBE_REQ;
	if (param->scan_f_add_spoofed_mac_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_SPOOFED_MAC_IN_PROBE_REQ;
	if (param->scan_f_add_rand_seq_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_RANDOM_SEQ_NO_IN_PROBE_REQ;
	if (param->scan_f_en_ie_whitelist_in_probe)
		cmd->scan_ctrl_flags |=
			WMI_SCAN_ENABLE_IE_WHTELIST_IN_PROBE_REQ;

	/* for adaptive scan mode using 3 bits (21 - 23 bits) */
	WMI_SCAN_SET_DWELL_MODE(cmd->scan_ctrl_flags,
		param->adaptive_dwell_time_mode);
}

/* scan_copy_ie_buffer() - Copy scan ie_data */
static inline void scan_copy_ie_buffer(uint8_t *buf_ptr,
				struct scan_req_params *params)
{
	qdf_mem_copy(buf_ptr, params->extraie.ptr, params->extraie.len);
}

/**
 * wmi_copy_scan_random_mac() - To copy scan randomization attrs to wmi buffer
 * @mac: random mac addr
 * @mask: random mac mask
 * @mac_addr: wmi random mac
 * @mac_mask: wmi random mac mask
 *
 * Return None.
 */
static inline
void wmi_copy_scan_random_mac(uint8_t *mac, uint8_t *mask,
			      wmi_mac_addr *mac_addr, wmi_mac_addr *mac_mask)
{
	WMI_CHAR_ARRAY_TO_MAC_ADDR(mac, mac_addr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(mask, mac_mask);
}

/*
 * wmi_fill_vendor_oui() - fill vendor OUIs
 * @buf_ptr: pointer to wmi tlv buffer
 * @num_vendor_oui: number of vendor OUIs to be filled
 * @param_voui: pointer to OUI buffer
 *
 * This function populates the wmi tlv buffer when vendor specific OUIs are
 * present.
 *
 * Return: None
 */
static inline
void wmi_fill_vendor_oui(uint8_t *buf_ptr, uint32_t num_vendor_oui,
			 uint32_t *pvoui)
{
	wmi_vendor_oui *voui = NULL;
	uint32_t i;

	voui = (wmi_vendor_oui *)buf_ptr;

	for (i = 0; i < num_vendor_oui; i++) {
		WMITLV_SET_HDR(&voui[i].tlv_header,
			       WMITLV_TAG_STRUC_wmi_vendor_oui,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_vendor_oui));
		voui[i].oui_type_subtype = pvoui[i];
	}
}

/*
 * wmi_fill_ie_whitelist_attrs() - fill IE whitelist attrs
 * @ie_bitmap: output pointer to ie bit map in cmd
 * @num_vendor_oui: output pointer to num vendor OUIs
 * @ie_whitelist: input parameter
 *
 * This function populates the IE whitelist attrs of scan, pno and
 * scan oui commands for ie_whitelist parameter.
 *
 * Return: None
 */
static inline
void wmi_fill_ie_whitelist_attrs(uint32_t *ie_bitmap,
				 uint32_t *num_vendor_oui,
				 struct probe_req_whitelist_attr *ie_whitelist)
{
	uint32_t i = 0;

	for (i = 0; i < PROBE_REQ_BITMAP_LEN; i++)
		ie_bitmap[i] = ie_whitelist->ie_bitmap[i];

	*num_vendor_oui = ie_whitelist->num_vendor_oui;
}

/**
 *  send_scan_start_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_scan_start_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_req_params *params)
{
	int32_t ret = 0;
	int32_t i;
	wmi_buf_t wmi_buf;
	wmi_start_scan_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t *tmp_ptr;
	wmi_ssid *ssid = NULL;
	wmi_mac_addr *bssid;
	size_t len = sizeof(*cmd);
	uint16_t extraie_len_with_pad = 0;
	uint8_t phymode_roundup = 0;
	struct probe_req_whitelist_attr *ie_whitelist = &params->ie_whitelist;

	/* Length TLV placeholder for array of uint32_t */
	len += WMI_TLV_HDR_SIZE;
	/* calculate the length of buffer required */
	if (params->chan_list.num_chan)
		len += params->chan_list.num_chan * sizeof(uint32_t);

	/* Length TLV placeholder for array of wmi_ssid structures */
	len += WMI_TLV_HDR_SIZE;
	if (params->num_ssids)
		len += params->num_ssids * sizeof(wmi_ssid);

	/* Length TLV placeholder for array of wmi_mac_addr structures */
	len += WMI_TLV_HDR_SIZE;
	if (params->num_bssid)
		len += sizeof(wmi_mac_addr) * params->num_bssid;

	/* Length TLV placeholder for array of bytes */
	len += WMI_TLV_HDR_SIZE;
	if (params->extraie.len)
		extraie_len_with_pad =
		roundup(params->extraie.len, sizeof(uint32_t));
	len += extraie_len_with_pad;

	len += WMI_TLV_HDR_SIZE; /* Length of TLV for array of wmi_vendor_oui */
	if (ie_whitelist->num_vendor_oui)
		len += ie_whitelist->num_vendor_oui * sizeof(wmi_vendor_oui);

	len += WMI_TLV_HDR_SIZE; /* Length of TLV for array of scan phymode */
	if (params->scan_f_wide_band)
		phymode_roundup =
			qdf_roundup(params->chan_list.num_chan * sizeof(uint8_t),
					sizeof(uint32_t));
	len += phymode_roundup;

	/* Allocate the memory */
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_start_scan_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_start_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_start_scan_cmd_fixed_param));

	cmd->scan_id = params->scan_id;
	cmd->scan_req_id = params->scan_req_id;
	cmd->vdev_id = params->vdev_id;
	cmd->scan_priority = params->scan_priority;

	copy_scan_event_cntrl_flags(cmd, params);

	cmd->dwell_time_active = params->dwell_time_active;
	cmd->dwell_time_active_2g = params->dwell_time_active_2g;
	cmd->dwell_time_passive = params->dwell_time_passive;
	cmd->min_rest_time = params->min_rest_time;
	cmd->max_rest_time = params->max_rest_time;
	cmd->repeat_probe_time = params->repeat_probe_time;
	cmd->probe_spacing_time = params->probe_spacing_time;
	cmd->idle_time = params->idle_time;
	cmd->max_scan_time = params->max_scan_time;
	cmd->probe_delay = params->probe_delay;
	cmd->burst_duration = params->burst_duration;
	cmd->num_chan = params->chan_list.num_chan;
	cmd->num_bssid = params->num_bssid;
	cmd->num_ssids = params->num_ssids;
	cmd->ie_len = params->extraie.len;
	cmd->n_probes = params->n_probes;
	cmd->scan_ctrl_flags_ext = params->scan_ctrl_flags_ext;

	WMI_LOGD("scan_ctrl_flags_ext = %x", cmd->scan_ctrl_flags_ext);

	if (params->scan_random.randomize)
		wmi_copy_scan_random_mac(params->scan_random.mac_addr,
					 params->scan_random.mac_mask,
					 &cmd->mac_addr,
					 &cmd->mac_mask);

	if (ie_whitelist->white_list)
		wmi_fill_ie_whitelist_attrs(cmd->ie_bitmap,
					    &cmd->num_vendor_oui,
					    ie_whitelist);

	buf_ptr += sizeof(*cmd);
	tmp_ptr = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < params->chan_list.num_chan; ++i)
		tmp_ptr[i] = params->chan_list.chan[i].freq;

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_UINT32,
		       (params->chan_list.num_chan * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE +
			(params->chan_list.num_chan * sizeof(uint32_t));

	if (params->num_ssids > WLAN_SCAN_MAX_NUM_SSID) {
		WMI_LOGE("Invalid value for num_ssids %d", params->num_ssids);
		goto error;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
	       (params->num_ssids * sizeof(wmi_ssid)));

	if (params->num_ssids) {
		ssid = (wmi_ssid *) (buf_ptr + WMI_TLV_HDR_SIZE);
		for (i = 0; i < params->num_ssids; ++i) {
			ssid->ssid_len = params->ssid[i].length;
			qdf_mem_copy(ssid->ssid, params->ssid[i].ssid,
				     params->ssid[i].length);
			ssid++;
		}
	}
	buf_ptr += WMI_TLV_HDR_SIZE + (params->num_ssids * sizeof(wmi_ssid));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       (params->num_bssid * sizeof(wmi_mac_addr)));
	bssid = (wmi_mac_addr *) (buf_ptr + WMI_TLV_HDR_SIZE);

	if (params->num_bssid) {
		for (i = 0; i < params->num_bssid; ++i) {
			WMI_CHAR_ARRAY_TO_MAC_ADDR(
				&params->bssid_list[i].bytes[0], bssid);
			bssid++;
		}
	}

	buf_ptr += WMI_TLV_HDR_SIZE +
		(params->num_bssid * sizeof(wmi_mac_addr));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, extraie_len_with_pad);
	if (params->extraie.len)
		scan_copy_ie_buffer(buf_ptr + WMI_TLV_HDR_SIZE,
			     params);

	buf_ptr += WMI_TLV_HDR_SIZE + extraie_len_with_pad;

	/* probe req ie whitelisting */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       ie_whitelist->num_vendor_oui * sizeof(wmi_vendor_oui));

	buf_ptr += WMI_TLV_HDR_SIZE;

	if (cmd->num_vendor_oui) {
		wmi_fill_vendor_oui(buf_ptr, cmd->num_vendor_oui,
				    ie_whitelist->voui);
		buf_ptr += cmd->num_vendor_oui * sizeof(wmi_vendor_oui);
	}

	/* Add phy mode TLV if it's a wide band scan */
	if (params->scan_f_wide_band) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, phymode_roundup);
		buf_ptr = (uint8_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
		for (i = 0; i < params->chan_list.num_chan; ++i)
			buf_ptr[i] =
				WMI_SCAN_CHAN_SET_MODE(params->chan_list.chan[i].phymode);
		buf_ptr += phymode_roundup;
	} else {
		/* Add ZERO legth phy mode TLV */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, 0);
	}

	wmi_mtrace(WMI_START_SCAN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf,
				   len, WMI_START_SCAN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to start scan: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}
	return ret;
error:
	wmi_buf_free(wmi_buf);
	return QDF_STATUS_E_FAILURE;
}

/**
 *  send_scan_stop_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan cancel cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_scan_stop_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_cancel_param *param)
{
	wmi_stop_scan_cmd_fixed_param *cmd;
	int ret;
	int len = sizeof(*cmd);
	wmi_buf_t wmi_buf;

	/* Allocate the memory */
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		ret = QDF_STATUS_E_NOMEM;
		goto error;
	}

	cmd = (wmi_stop_scan_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_stop_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_stop_scan_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->requestor = param->requester;
	cmd->scan_id = param->scan_id;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	/* stop the scan with the corresponding scan_id */
	if (param->req_type == WLAN_SCAN_CANCEL_PDEV_ALL) {
		/* Cancelling all scans */
		cmd->req_type = WMI_SCAN_STOP_ALL;
	} else if (param->req_type == WLAN_SCAN_CANCEL_VDEV_ALL) {
		/* Cancelling VAP scans */
		cmd->req_type = WMI_SCN_STOP_VAP_ALL;
	} else if (param->req_type == WLAN_SCAN_CANCEL_SINGLE) {
		/* Cancelling specific scan */
		cmd->req_type = WMI_SCAN_STOP_ONE;
	} else {
		WMI_LOGE("%s: Invalid Command : ", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_INVAL;
	}

	wmi_mtrace(WMI_STOP_SCAN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf,
				   len, WMI_STOP_SCAN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send stop scan: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

error:
	return ret;
}

static QDF_STATUS send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *chan_list)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_scan_chan_list_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	wmi_channel *chan_info;
	struct channel_param *tchan_info;
	uint16_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_channel) * chan_list->nallchans;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_scan_chan_list_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_chan_list_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_chan_list_cmd_fixed_param));

	WMI_LOGD("no of channels = %d, len = %d", chan_list->nallchans, len);

	if (chan_list->append)
		cmd->flags |= APPEND_TO_EXISTING_CHAN_LIST;

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							chan_list->pdev_id);
	cmd->num_scan_chans = chan_list->nallchans;
	WMITLV_SET_HDR((buf_ptr + sizeof(wmi_scan_chan_list_cmd_fixed_param)),
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_channel) * chan_list->nallchans);
	chan_info = (wmi_channel *) (buf_ptr + sizeof(*cmd) + WMI_TLV_HDR_SIZE);
	tchan_info = &(chan_list->ch_param[0]);

	for (i = 0; i < chan_list->nallchans; ++i) {
		WMITLV_SET_HDR(&chan_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan_info->mhz = tchan_info->mhz;
		chan_info->band_center_freq1 =
				 tchan_info->cfreq1;
		chan_info->band_center_freq2 =
				tchan_info->cfreq2;

		if (tchan_info->is_chan_passive)
			WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_PASSIVE);
		if (tchan_info->dfs_set)
			WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_DFS);

		if (tchan_info->allow_vht)
			WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_ALLOW_VHT);
		if (tchan_info->allow_ht)
			WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_ALLOW_HT);
		WMI_SET_CHANNEL_MODE(chan_info,
				     tchan_info->phy_mode);

		if (tchan_info->half_rate)
			WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_HALF_RATE);

		if (tchan_info->quarter_rate)
			WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_QUARTER_RATE);

		/* also fill in power information */
		WMI_SET_CHANNEL_MIN_POWER(chan_info,
					  tchan_info->minpower);
		WMI_SET_CHANNEL_MAX_POWER(chan_info,
					  tchan_info->maxpower);
		WMI_SET_CHANNEL_REG_POWER(chan_info,
					  tchan_info->maxregpower);
		WMI_SET_CHANNEL_ANTENNA_MAX(chan_info,
					    tchan_info->antennamax);
		WMI_SET_CHANNEL_REG_CLASSID(chan_info,
					    tchan_info->reg_class_id);
		WMI_SET_CHANNEL_MAX_TX_POWER(chan_info,
					     tchan_info->maxregpower);

		WMI_LOGD("chan[%d] = %u", i, chan_info->mhz);

		tchan_info++;
		chan_info++;
	}
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							chan_list->pdev_id);

	wmi_mtrace(WMI_SCAN_CHAN_LIST_CMDID, cmd->pdev_id, 0);
	qdf_status = wmi_unified_cmd_send(
			wmi_handle,
			buf, len, WMI_SCAN_CHAN_LIST_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("Failed to send WMI_SCAN_CHAN_LIST_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}

/**
 * populate_tx_send_params - Populate TX param TLV for mgmt and offchan tx
 *
 * @bufp: Pointer to buffer
 * @param: Pointer to tx param
 *
 * Return: QDF_STATUS_SUCCESS for success and QDF_STATUS_E_FAILURE for failure
 */
static inline QDF_STATUS populate_tx_send_params(uint8_t *bufp,
					struct tx_send_params param)
{
	wmi_tx_send_params *tx_param;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!bufp) {
		status = QDF_STATUS_E_FAILURE;
		return status;
	}
	tx_param = (wmi_tx_send_params *)bufp;
	WMITLV_SET_HDR(&tx_param->tlv_header,
		       WMITLV_TAG_STRUC_wmi_tx_send_params,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_tx_send_params));
	WMI_TX_SEND_PARAM_PWR_SET(tx_param->tx_param_dword0, param.pwr);
	WMI_TX_SEND_PARAM_MCS_MASK_SET(tx_param->tx_param_dword0,
				       param.mcs_mask);
	WMI_TX_SEND_PARAM_NSS_MASK_SET(tx_param->tx_param_dword0,
				       param.nss_mask);
	WMI_TX_SEND_PARAM_RETRY_LIMIT_SET(tx_param->tx_param_dword0,
					  param.retry_limit);
	WMI_TX_SEND_PARAM_CHAIN_MASK_SET(tx_param->tx_param_dword1,
					 param.chain_mask);
	WMI_TX_SEND_PARAM_BW_MASK_SET(tx_param->tx_param_dword1,
				      param.bw_mask);
	WMI_TX_SEND_PARAM_PREAMBLE_SET(tx_param->tx_param_dword1,
				       param.preamble_type);
	WMI_TX_SEND_PARAM_FRAME_TYPE_SET(tx_param->tx_param_dword1,
					 param.frame_type);

	return status;
}

#ifdef CONFIG_HL_SUPPORT
/**
 *  send_mgmt_cmd_tlv() - WMI scan start function
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold mgmt cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param)
{
	wmi_buf_t buf;
	uint8_t *bufp;
	int32_t cmd_len;
	wmi_mgmt_tx_send_cmd_fixed_param *cmd;
	int32_t bufp_len = (param->frm_len < mgmt_tx_dl_frm_len) ? param->frm_len :
		mgmt_tx_dl_frm_len;

	if (param->frm_len > mgmt_tx_dl_frm_len) {
		WMI_LOGE("%s:mgmt frame len %u exceeds %u",
			 __func__, param->frm_len, mgmt_tx_dl_frm_len);
		return QDF_STATUS_E_INVAL;
	}

	cmd_len = sizeof(wmi_mgmt_tx_send_cmd_fixed_param) +
		  WMI_TLV_HDR_SIZE +
		  roundup(bufp_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_tx_send_params) + cmd_len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_mgmt_tx_send_cmd_fixed_param *)wmi_buf_data(buf);
	bufp = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_mgmt_tx_send_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_mgmt_tx_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	cmd->desc_id = param->desc_id;
	cmd->chanfreq = param->chanfreq;
	bufp += sizeof(wmi_mgmt_tx_send_cmd_fixed_param);
	WMITLV_SET_HDR(bufp, WMITLV_TAG_ARRAY_BYTE, roundup(bufp_len,
							    sizeof(uint32_t)));
	bufp += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(bufp, param->pdata, bufp_len);

	cmd->frame_len = param->frm_len;
	cmd->buf_len = bufp_len;
	cmd->tx_params_valid = param->tx_params_valid;

	wmi_mgmt_cmd_record(wmi_handle, WMI_MGMT_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	bufp += roundup(bufp_len, sizeof(uint32_t));
	if (param->tx_params_valid) {
		if (populate_tx_send_params(bufp, param->tx_param) !=
		    QDF_STATUS_SUCCESS) {
			WMI_LOGE("%s: Populate TX send params failed",
				 __func__);
			goto free_buf;
		}
		cmd_len += sizeof(wmi_tx_send_params);
	}

	wmi_mtrace(WMI_MGMT_TX_SEND_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				      WMI_MGMT_TX_SEND_CMDID)) {
		WMI_LOGE("%s: Failed to send mgmt Tx", __func__);
		goto free_buf;
	}
	return QDF_STATUS_SUCCESS;

free_buf:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}
#else
/**
 *  send_mgmt_cmd_tlv() - WMI scan start function
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold mgmt cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param)
{
	wmi_buf_t buf;
	wmi_mgmt_tx_send_cmd_fixed_param *cmd;
	int32_t cmd_len;
	uint64_t dma_addr;
	void *qdf_ctx = param->qdf_ctx;
	uint8_t *bufp;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int32_t bufp_len = (param->frm_len < mgmt_tx_dl_frm_len) ? param->frm_len :
		mgmt_tx_dl_frm_len;

	cmd_len = sizeof(wmi_mgmt_tx_send_cmd_fixed_param) +
		  WMI_TLV_HDR_SIZE +
		  roundup(bufp_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_tx_send_params) + cmd_len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_mgmt_tx_send_cmd_fixed_param *)wmi_buf_data(buf);
	bufp = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_mgmt_tx_send_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_mgmt_tx_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	cmd->desc_id = param->desc_id;
	cmd->chanfreq = param->chanfreq;
	bufp += sizeof(wmi_mgmt_tx_send_cmd_fixed_param);
	WMITLV_SET_HDR(bufp, WMITLV_TAG_ARRAY_BYTE, roundup(bufp_len,
							    sizeof(uint32_t)));
	bufp += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(bufp, param->pdata, bufp_len);

	status = qdf_nbuf_map_single(qdf_ctx, param->tx_frame,
				     QDF_DMA_TO_DEVICE);
	if (status != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: wmi buf map failed", __func__);
		goto free_buf;
	}

	dma_addr = qdf_nbuf_get_frag_paddr(param->tx_frame, 0);
	cmd->paddr_lo = (uint32_t)(dma_addr & 0xffffffff);
#if defined(HTT_PADDR64)
	cmd->paddr_hi = (uint32_t)((dma_addr >> 32) & 0x1F);
#endif
	cmd->frame_len = param->frm_len;
	cmd->buf_len = bufp_len;
	cmd->tx_params_valid = param->tx_params_valid;

	wmi_mgmt_cmd_record(wmi_handle, WMI_MGMT_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	bufp += roundup(bufp_len, sizeof(uint32_t));
	if (param->tx_params_valid) {
		status = populate_tx_send_params(bufp, param->tx_param);
		if (status != QDF_STATUS_SUCCESS) {
			WMI_LOGE("%s: Populate TX send params failed",
				 __func__);
			goto unmap_tx_frame;
		}
		cmd_len += sizeof(wmi_tx_send_params);
	}

	wmi_mtrace(WMI_MGMT_TX_SEND_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				      WMI_MGMT_TX_SEND_CMDID)) {
		WMI_LOGE("%s: Failed to send mgmt Tx", __func__);
		goto unmap_tx_frame;
	}
	return QDF_STATUS_SUCCESS;

unmap_tx_frame:
	qdf_nbuf_unmap_single(qdf_ctx, param->tx_frame,
				     QDF_DMA_TO_DEVICE);
free_buf:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}
#endif /* CONFIG_HL_SUPPORT */

/**
 *  send_offchan_data_tx_send_cmd_tlv() - Send off-chan tx data
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to offchan data tx cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS  on success and error on failure.
 */
static QDF_STATUS send_offchan_data_tx_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_offchan_data_tx_params *param)
{
	wmi_buf_t buf;
	wmi_offchan_data_tx_send_cmd_fixed_param *cmd;
	int32_t cmd_len;
	uint64_t dma_addr;
	void *qdf_ctx = param->qdf_ctx;
	uint8_t *bufp;
	int32_t bufp_len = (param->frm_len < mgmt_tx_dl_frm_len) ?
					param->frm_len : mgmt_tx_dl_frm_len;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	cmd_len = sizeof(wmi_offchan_data_tx_send_cmd_fixed_param) +
		  WMI_TLV_HDR_SIZE +
		  roundup(bufp_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_tx_send_params) + cmd_len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_offchan_data_tx_send_cmd_fixed_param *) wmi_buf_data(buf);
	bufp = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_offchan_data_tx_send_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_offchan_data_tx_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	cmd->desc_id = param->desc_id;
	cmd->chanfreq = param->chanfreq;
	bufp += sizeof(wmi_offchan_data_tx_send_cmd_fixed_param);
	WMITLV_SET_HDR(bufp, WMITLV_TAG_ARRAY_BYTE, roundup(bufp_len,
							    sizeof(uint32_t)));
	bufp += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(bufp, param->pdata, bufp_len);
	qdf_nbuf_map_single(qdf_ctx, param->tx_frame, QDF_DMA_TO_DEVICE);
	dma_addr = qdf_nbuf_get_frag_paddr(param->tx_frame, 0);
	cmd->paddr_lo = (uint32_t)(dma_addr & 0xffffffff);
#if defined(HTT_PADDR64)
	cmd->paddr_hi = (uint32_t)((dma_addr >> 32) & 0x1F);
#endif
	cmd->frame_len = param->frm_len;
	cmd->buf_len = bufp_len;
	cmd->tx_params_valid = param->tx_params_valid;

	wmi_mgmt_cmd_record(wmi_handle, WMI_OFFCHAN_DATA_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	bufp += roundup(bufp_len, sizeof(uint32_t));
	if (param->tx_params_valid) {
		status = populate_tx_send_params(bufp, param->tx_param);
		if (status != QDF_STATUS_SUCCESS) {
			WMI_LOGE("%s: Populate TX send params failed",
				 __func__);
			goto err1;
		}
		cmd_len += sizeof(wmi_tx_send_params);
	}

	wmi_mtrace(WMI_OFFCHAN_DATA_TX_SEND_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				WMI_OFFCHAN_DATA_TX_SEND_CMDID)) {
		WMI_LOGE("%s: Failed to offchan data Tx", __func__);
		goto err1;
	}

	return QDF_STATUS_SUCCESS;

err1:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}

/**
 * send_modem_power_state_cmd_tlv() - set modem power state to fw
 * @wmi_handle: wmi handle
 * @param_value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_modem_power_state_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t param_value)
{
	QDF_STATUS ret;
	wmi_modem_power_state_cmd_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_modem_power_state_cmd_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_modem_power_state_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_modem_power_state_cmd_param));
	cmd->modem_power_state = param_value;
	WMI_LOGD("%s: Setting cmd->modem_power_state = %u", __func__,
		 param_value);
	wmi_mtrace(WMI_MODEM_POWER_STATE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				     WMI_MODEM_POWER_STATE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send notify cmd ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_sta_ps_mode_cmd_tlv() - set sta powersave mode in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @val: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_sta_ps_mode_cmd_tlv(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val)
{
	wmi_sta_powersave_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("Set Sta Mode Ps vdevId %d val %d", vdev_id, val);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_sta_powersave_mode_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_powersave_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_powersave_mode_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (val)
		cmd->sta_ps_mode = WMI_STA_PS_MODE_ENABLED;
	else
		cmd->sta_ps_mode = WMI_STA_PS_MODE_DISABLED;

	wmi_mtrace(WMI_STA_POWERSAVE_MODE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_STA_POWERSAVE_MODE_CMDID)) {
		WMI_LOGE("Set Sta Mode Ps Failed vdevId %d val %d",
			 vdev_id, val);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_idle_roam_monitor_cmd_tlv() - send idle monitor command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_idle_roam_monitor_cmd_tlv(wmi_unified_t wmi_handle,
						 uint8_t val)
{
	wmi_idle_trigger_monitor_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	size_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_idle_trigger_monitor_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_idle_trigger_monitor_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_idle_trigger_monitor_cmd_fixed_param));

	cmd->idle_trigger_monitor = (val ? WMI_IDLE_TRIGGER_MONITOR_ON :
					   WMI_IDLE_TRIGGER_MONITOR_OFF);

	WMI_LOGD("val:%d", cmd->idle_trigger_monitor);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_IDLE_TRIGGER_MONITOR_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_mimops_cmd_tlv() - set MIMO powersave
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_mimops_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t vdev_id, int value)
{
	QDF_STATUS ret;
	wmi_sta_smps_force_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_sta_smps_force_mode_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_smps_force_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_smps_force_mode_cmd_fixed_param));

	cmd->vdev_id = vdev_id;

	/* WMI_SMPS_FORCED_MODE values do not directly map
	 * to SM power save values defined in the specification.
	 * Make sure to send the right mapping.
	 */
	switch (value) {
	case 0:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_NONE;
		break;
	case 1:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_DISABLED;
		break;
	case 2:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_STATIC;
		break;
	case 3:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_DYNAMIC;
		break;
	default:
		WMI_LOGE("%s:INVALID Mimo PS CONFIG", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("Setting vdev %d value = %u", vdev_id, value);

	wmi_mtrace(WMI_STA_SMPS_FORCE_MODE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_SMPS_FORCE_MODE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set Mimo PS ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_smps_params_cmd_tlv() - set smps params
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_smps_params_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
			       int value)
{
	QDF_STATUS ret;
	wmi_sta_smps_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_sta_smps_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_smps_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_smps_param_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->value = value & WMI_SMPS_MASK_LOWER_16BITS;
	cmd->param =
		(value >> WMI_SMPS_PARAM_VALUE_S) & WMI_SMPS_MASK_UPPER_3BITS;

	WMI_LOGD("Setting vdev %d value = %x param %x", vdev_id, cmd->value,
		 cmd->param);

	wmi_mtrace(WMI_STA_SMPS_PARAM_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_SMPS_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set Mimo PS ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_get_temperature_cmd_tlv() - get pdev temperature req
 * @wmi_handle: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_get_temperature_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_pdev_get_temperature_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len = sizeof(wmi_pdev_get_temperature_cmd_fixed_param);
	uint8_t *buf_ptr;

	if (!wmi_handle) {
		WMI_LOGE(FL("WMI is closed, can not issue cmd"));
		return QDF_STATUS_E_INVAL;
	}

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_pdev_get_temperature_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_get_temperature_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_get_temperature_cmd_fixed_param));

	wmi_mtrace(WMI_PDEV_GET_TEMPERATURE_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_PDEV_GET_TEMPERATURE_CMDID)) {
		WMI_LOGE(FL("failed to send get temperature command"));
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_sta_uapsd_auto_trig_cmd_tlv() - set uapsd auto trigger command
 * @wmi_handle: wmi handle
 * @vdevid: vdev id
 * @peer_addr: peer mac address
 * @auto_triggerparam: auto trigger parameters
 * @num_ac: number of access category
 *
 * This function sets the trigger
 * uapsd params such as service interval, delay interval
 * and suspend interval which will be used by the firmware
 * to send trigger frames periodically when there is no
 * traffic on the transmit side.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_sta_uapsd_auto_trig_cmd_tlv(wmi_unified_t wmi_handle,
				struct sta_uapsd_trig_params *param)
{
	wmi_sta_uapsd_auto_trig_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t param_len = param->num_ac * sizeof(wmi_sta_uapsd_auto_trig_param);
	uint32_t cmd_len = sizeof(*cmd) + param_len + WMI_TLV_HDR_SIZE;
	uint32_t i;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	struct sta_uapsd_params *uapsd_param;
	wmi_sta_uapsd_auto_trig_param *trig_param;

	buf = wmi_buf_alloc(wmi_handle, cmd_len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_sta_uapsd_auto_trig_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_uapsd_auto_trig_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_uapsd_auto_trig_cmd_fixed_param));
	cmd->vdev_id = param->vdevid;
	cmd->num_ac = param->num_ac;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, param_len);

	buf_ptr += WMI_TLV_HDR_SIZE;

	/*
	 * Update tag and length for uapsd auto trigger params (this will take
	 * care of updating tag and length if it is not pre-filled by caller).
	 */
	uapsd_param = (struct sta_uapsd_params *)param->auto_triggerparam;
	trig_param = (wmi_sta_uapsd_auto_trig_param *)buf_ptr;
	for (i = 0; i < param->num_ac; i++) {
		WMITLV_SET_HDR((buf_ptr +
				(i * sizeof(wmi_sta_uapsd_auto_trig_param))),
			       WMITLV_TAG_STRUC_wmi_sta_uapsd_auto_trig_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_sta_uapsd_auto_trig_param));
		trig_param->wmm_ac = uapsd_param->wmm_ac;
		trig_param->user_priority = uapsd_param->user_priority;
		trig_param->service_interval = uapsd_param->service_interval;
		trig_param->suspend_interval = uapsd_param->suspend_interval;
		trig_param->delay_interval = uapsd_param->delay_interval;
		trig_param++;
		uapsd_param++;
	}

	wmi_mtrace(WMI_STA_UAPSD_AUTO_TRIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				   WMI_STA_UAPSD_AUTO_TRIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set uapsd param ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_thermal_mgmt_cmd_tlv() - set thermal mgmt command to fw
 * @wmi_handle: Pointer to wmi handle
 * @thermal_info: Thermal command information
 *
 * This function sends the thermal management command
 * to the firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_set_thermal_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct thermal_cmd_params *thermal_info)
{
	wmi_thermal_mgmt_cmd_fixed_param *cmd = NULL;
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	uint32_t len = 0;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_thermal_mgmt_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_thermal_mgmt_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_thermal_mgmt_cmd_fixed_param));

	cmd->lower_thresh_degreeC = thermal_info->min_temp;
	cmd->upper_thresh_degreeC = thermal_info->max_temp;
	cmd->enable = thermal_info->thermal_enable;

	WMI_LOGE("TM Sending thermal mgmt cmd: low temp %d, upper temp %d, enabled %d",
		cmd->lower_thresh_degreeC, cmd->upper_thresh_degreeC, cmd->enable);

	wmi_mtrace(WMI_THERMAL_MGMT_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_THERMAL_MGMT_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		WMI_LOGE("%s:Failed to send thermal mgmt command", __func__);
	}

	return status;
}

/**
 * send_lro_config_cmd_tlv() - process the LRO config command
 * @wmi_handle: Pointer to WMI handle
 * @wmi_lro_cmd: Pointer to LRO configuration parameters
 *
 * This function sends down the LRO configuration parameters to
 * the firmware to enable LRO, sets the TCP flags and sets the
 * seed values for the toeplitz hash generation
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_lro_config_cmd_tlv(wmi_unified_t wmi_handle,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd)
{
	wmi_lro_info_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint8_t pdev_id = wmi_lro_cmd->pdev_id;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_lro_info_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_lro_info_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(wmi_lro_info_cmd_fixed_param));

	cmd->lro_enable = wmi_lro_cmd->lro_enable;
	WMI_LRO_INFO_TCP_FLAG_VALS_SET(cmd->tcp_flag_u32,
		 wmi_lro_cmd->tcp_flag);
	WMI_LRO_INFO_TCP_FLAGS_MASK_SET(cmd->tcp_flag_u32,
		 wmi_lro_cmd->tcp_flag_mask);
	cmd->toeplitz_hash_ipv4_0_3 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[0];
	cmd->toeplitz_hash_ipv4_4_7 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[1];
	cmd->toeplitz_hash_ipv4_8_11 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[2];
	cmd->toeplitz_hash_ipv4_12_15 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[3];
	cmd->toeplitz_hash_ipv4_16 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[4];

	cmd->toeplitz_hash_ipv6_0_3 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[0];
	cmd->toeplitz_hash_ipv6_4_7 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[1];
	cmd->toeplitz_hash_ipv6_8_11 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[2];
	cmd->toeplitz_hash_ipv6_12_15 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[3];
	cmd->toeplitz_hash_ipv6_16_19 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[4];
	cmd->toeplitz_hash_ipv6_20_23 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[5];
	cmd->toeplitz_hash_ipv6_24_27 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[6];
	cmd->toeplitz_hash_ipv6_28_31 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[7];
	cmd->toeplitz_hash_ipv6_32_35 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[8];
	cmd->toeplitz_hash_ipv6_36_39 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[9];
	cmd->toeplitz_hash_ipv6_40 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[10];

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);
	WMI_LOGD("WMI_LRO_CONFIG: lro_enable %d, tcp_flag 0x%x, pdev_id: %d",
		 cmd->lro_enable, cmd->tcp_flag_u32, cmd->pdev_id);

	wmi_mtrace(WMI_LRO_CONFIG_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
		 sizeof(*cmd), WMI_LRO_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		WMI_LOGE("%s:Failed to send WMI_LRO_CONFIG_CMDID", __func__);
	}

	return status;
}

/**
 * send_peer_rate_report_cmd_tlv() - process the peer rate report command
 * @wmi_handle: Pointer to wmi handle
 * @rate_report_params: Pointer to peer rate report parameters
 *
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_peer_rate_report_cmd_tlv(wmi_unified_t wmi_handle,
	 struct wmi_peer_rate_report_params *rate_report_params)
{
	wmi_peer_set_rate_report_condition_fixed_param *cmd = NULL;
	wmi_buf_t buf = NULL;
	QDF_STATUS status = 0;
	uint32_t len = 0;
	uint32_t i, j;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_peer_set_rate_report_condition_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(
	&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_set_rate_report_condition_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_peer_set_rate_report_condition_fixed_param));

	cmd->enable_rate_report  = rate_report_params->rate_report_enable;
	cmd->report_backoff_time = rate_report_params->backoff_time;
	cmd->report_timer_period = rate_report_params->timer_period;
	for (i = 0; i < PEER_RATE_REPORT_COND_MAX_NUM; i++) {
		cmd->cond_per_phy[i].val_cond_flags	=
			rate_report_params->report_per_phy[i].cond_flags;
		cmd->cond_per_phy[i].rate_delta.min_delta  =
			rate_report_params->report_per_phy[i].delta.delta_min;
		cmd->cond_per_phy[i].rate_delta.percentage =
			rate_report_params->report_per_phy[i].delta.percent;
		for (j = 0; j < MAX_NUM_OF_RATE_THRESH; j++) {
			cmd->cond_per_phy[i].rate_threshold[j] =
			rate_report_params->report_per_phy[i].
						report_rate_threshold[j];
		}
	}

	WMI_LOGE("%s enable %d backoff_time %d period %d", __func__,
		 cmd->enable_rate_report,
		 cmd->report_backoff_time, cmd->report_timer_period);

	wmi_mtrace(WMI_PEER_SET_RATE_REPORT_CONDITION_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_SET_RATE_REPORT_CONDITION_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		WMI_LOGE("%s:Failed to send peer_set_report_cond command",
			 __func__);
	}
	return status;
}

#ifdef CONFIG_MCL
/**
 * send_bcn_buf_ll_cmd_tlv() - prepare and send beacon buffer to fw for LL
 * @wmi_handle: wmi handle
 * @param: bcn ll cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_bcn_buf_ll_cmd_tlv(wmi_unified_t wmi_handle,
			wmi_bcn_send_from_host_cmd_fixed_param *param)
{
	wmi_bcn_send_from_host_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	QDF_STATUS ret;

	wmi_buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!wmi_buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_bcn_send_from_host_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_send_from_host_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_bcn_send_from_host_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->data_len = param->data_len;
	cmd->frame_ctrl = param->frame_ctrl;
	cmd->frag_ptr = param->frag_ptr;
	cmd->dtim_flag = param->dtim_flag;

	wmi_mtrace(WMI_PDEV_SEND_BCN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf, sizeof(*cmd),
				      WMI_PDEV_SEND_BCN_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_PDEV_SEND_BCN_CMDID command");
		wmi_buf_free(wmi_buf);
	}

	return ret;
}
#endif /* CONFIG_MCL */

/**
 * send_process_update_edca_param_cmd_tlv() - update EDCA params
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id.
 * @wmm_vparams: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: CDF Status
 */
static QDF_STATUS send_process_update_edca_param_cmd_tlv(wmi_unified_t wmi_handle,
				    uint8_t vdev_id, bool mu_edca_param,
				    struct wmi_host_wme_vparams wmm_vparams[WMI_MAX_NUM_AC])
{
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	wmi_vdev_set_wmm_params_cmd_fixed_param *cmd;
	wmi_wmm_vparams *wmm_param;
	struct wmi_host_wme_vparams *twmm_param;
	int len = sizeof(*cmd);
	int ac;

	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_set_wmm_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_wmm_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_wmm_params_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->wmm_param_type = mu_edca_param;

	for (ac = 0; ac < WMI_MAX_NUM_AC; ac++) {
		wmm_param = (wmi_wmm_vparams *) (&cmd->wmm_params[ac]);
		twmm_param = (struct wmi_host_wme_vparams *) (&wmm_vparams[ac]);
		WMITLV_SET_HDR(&wmm_param->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vdev_set_wmm_params_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wmm_vparams));
		wmm_param->cwmin = twmm_param->cwmin;
		wmm_param->cwmax = twmm_param->cwmax;
		wmm_param->aifs = twmm_param->aifs;
		if (mu_edca_param)
			wmm_param->mu_edca_timer = twmm_param->mu_edca_timer;
		else
			wmm_param->txoplimit = twmm_param->txoplimit;
		wmm_param->acm = twmm_param->acm;
		wmm_param->no_ack = twmm_param->noackpolicy;
	}

	wmi_mtrace(WMI_VDEV_SET_WMM_PARAMS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_WMM_PARAMS_CMDID))
		goto fail;

	return QDF_STATUS_SUCCESS;

fail:
	wmi_buf_free(buf);
	WMI_LOGE("%s: Failed to set WMM Paremeters", __func__);
	return QDF_STATUS_E_FAILURE;
}

/**
 * send_probe_rsp_tmpl_send_cmd_tlv() - send probe response template to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @probe_rsp_info: probe response info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_probe_rsp_tmpl_send_cmd_tlv(wmi_unified_t wmi_handle,
				   uint8_t vdev_id,
				   struct wmi_probe_resp_params *probe_rsp_info)
{
	wmi_prb_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint32_t tmpl_len, tmpl_len_aligned, wmi_buf_len;
	uint8_t *buf_ptr;
	QDF_STATUS ret;

	WMI_LOGD(FL("Send probe response template for vdev %d"), vdev_id);

	tmpl_len = probe_rsp_info->prb_rsp_template_len;
	tmpl_len_aligned = roundup(tmpl_len, sizeof(uint32_t));

	wmi_buf_len = sizeof(wmi_prb_tmpl_cmd_fixed_param) +
			sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
			tmpl_len_aligned;

	if (wmi_buf_len > WMI_BEACON_TX_BUFFER_SIZE) {
		WMI_LOGE(FL("wmi_buf_len: %d > %d. Can't send wmi cmd"),
		wmi_buf_len, WMI_BEACON_TX_BUFFER_SIZE);
		return QDF_STATUS_E_INVAL;
	}

	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_prb_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_prb_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_prb_tmpl_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->buf_len = tmpl_len;
	buf_ptr += sizeof(wmi_prb_tmpl_cmd_fixed_param);

	bcn_prb_info = (wmi_bcn_prb_info *) buf_ptr;
	WMITLV_SET_HDR(&bcn_prb_info->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_prb_info,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_prb_info));
	bcn_prb_info->caps = 0;
	bcn_prb_info->erp = 0;
	buf_ptr += sizeof(wmi_bcn_prb_info);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, tmpl_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, probe_rsp_info->prb_rsp_template_frm, tmpl_len);

	wmi_mtrace(WMI_PRB_TMPL_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_PRB_TMPL_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to send PRB RSP tmpl: %d"), ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

#if defined(ATH_SUPPORT_WAPI) || defined(FEATURE_WLAN_WAPI)
#define WPI_IV_LEN 16

/**
 * wmi_update_wpi_key_counter() - update WAPI tsc and rsc key counters
 *
 * @dest_tx: destination address of tsc key counter
 * @src_tx: source address of tsc key counter
 * @dest_rx: destination address of rsc key counter
 * @src_rx: source address of rsc key counter
 *
 * This function copies WAPI tsc and rsc key counters in the wmi buffer.
 *
 * Return: None
 *
 */
static void wmi_update_wpi_key_counter(uint8_t *dest_tx, uint8_t *src_tx,
					uint8_t *dest_rx, uint8_t *src_rx)
{
	qdf_mem_copy(dest_tx, src_tx, WPI_IV_LEN);
	qdf_mem_copy(dest_rx, src_rx, WPI_IV_LEN);
}
#else
static void wmi_update_wpi_key_counter(uint8_t *dest_tx, uint8_t *src_tx,
					uint8_t *dest_rx, uint8_t *src_rx)
{
	return;
}
#endif

/**
 * send_setup_install_key_cmd_tlv() - set key parameters
 * @wmi_handle: wmi handle
 * @key_params: key parameters
 *
 * This function fills structure from information
 * passed in key_params.
 *
 * Return: QDF_STATUS_SUCCESS - success
 *	 QDF_STATUS_E_FAILURE - failure
 *	 QDF_STATUS_E_NOMEM - not able to allocate buffer
 */
static QDF_STATUS send_setup_install_key_cmd_tlv(wmi_unified_t wmi_handle,
					   struct set_key_params *key_params)
{
	wmi_vdev_install_key_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	uint8_t *key_data;
	QDF_STATUS status;

	len = sizeof(*cmd) + roundup(key_params->key_len, sizeof(uint32_t)) +
	       WMI_TLV_HDR_SIZE;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_install_key_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_install_key_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_install_key_cmd_fixed_param));
	cmd->vdev_id = key_params->vdev_id;
	cmd->key_ix = key_params->key_idx;


	WMI_CHAR_ARRAY_TO_MAC_ADDR(key_params->peer_mac, &cmd->peer_macaddr);
	cmd->key_flags |= key_params->key_flags;
	cmd->key_cipher = key_params->key_cipher;
	if ((key_params->key_txmic_len) &&
			(key_params->key_rxmic_len)) {
		cmd->key_txmic_len = key_params->key_txmic_len;
		cmd->key_rxmic_len = key_params->key_rxmic_len;
	}
#if defined(ATH_SUPPORT_WAPI) || defined(FEATURE_WLAN_WAPI)
	wmi_update_wpi_key_counter(cmd->wpi_key_tsc_counter,
				   key_params->tx_iv,
				   cmd->wpi_key_rsc_counter,
				   key_params->rx_iv);
#endif
	buf_ptr += sizeof(wmi_vdev_install_key_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       roundup(key_params->key_len, sizeof(uint32_t)));
	key_data = (uint8_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	qdf_mem_copy((void *)key_data,
		     (const void *)key_params->key_data, key_params->key_len);
	qdf_mem_copy(&cmd->key_rsc_counter, &key_params->key_rsc_ctr,
		     sizeof(wmi_key_seq_counter));
	cmd->key_len = key_params->key_len;

	wmi_mtrace(WMI_VDEV_INSTALL_KEY_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
					      WMI_VDEV_INSTALL_KEY_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_zero(wmi_buf_data(buf), len);
		wmi_buf_free(buf);
	}
	return status;
}

/**
 * send_p2p_go_set_beacon_ie_cmd_tlv() - set beacon IE for p2p go
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @p2p_ie: p2p IE
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_p2p_go_set_beacon_ie_cmd_tlv(wmi_unified_t wmi_handle,
				    uint32_t vdev_id, uint8_t *p2p_ie)
{
	QDF_STATUS ret;
	wmi_p2p_go_set_beacon_ie_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t ie_len, ie_len_aligned, wmi_buf_len;
	uint8_t *buf_ptr;

	ie_len = (uint32_t) (p2p_ie[1] + 2);

	/* More than one P2P IE may be included in a single frame.
	   If multiple P2P IEs are present, the complete P2P attribute
	   data consists of the concatenation of the P2P Attribute
	   fields of the P2P IEs. The P2P Attributes field of each
	   P2P IE may be any length up to the maximum (251 octets).
	   In this case host sends one P2P IE to firmware so the length
	   should not exceed more than 251 bytes
	 */
	if (ie_len > 251) {
		WMI_LOGE("%s : invalid p2p ie length %u", __func__, ie_len);
		return QDF_STATUS_E_INVAL;
	}

	ie_len_aligned = roundup(ie_len, sizeof(uint32_t));

	wmi_buf_len =
		sizeof(wmi_p2p_go_set_beacon_ie_fixed_param) + ie_len_aligned +
		WMI_TLV_HDR_SIZE;

	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_p2p_go_set_beacon_ie_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_p2p_go_set_beacon_ie_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_p2p_go_set_beacon_ie_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->ie_buf_len = ie_len;

	buf_ptr += sizeof(wmi_p2p_go_set_beacon_ie_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, p2p_ie, ie_len);

	WMI_LOGD("%s: Sending WMI_P2P_GO_SET_BEACON_IE", __func__);

	wmi_mtrace(WMI_P2P_GO_SET_BEACON_IE, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len,
				   WMI_P2P_GO_SET_BEACON_IE);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send bcn tmpl: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	WMI_LOGD("%s: Successfully sent WMI_P2P_GO_SET_BEACON_IE", __func__);
	return ret;
}

/**
 * send_scan_probe_setoui_cmd_tlv() - set scan probe OUI
 * @wmi_handle: wmi handle
 * @psetoui: OUI parameters
 *
 * set scan probe OUI parameters in firmware
 *
 * Return: CDF status
 */
static QDF_STATUS send_scan_probe_setoui_cmd_tlv(wmi_unified_t wmi_handle,
			  struct scan_mac_oui *psetoui)
{
	wmi_scan_prob_req_oui_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	uint32_t *oui_buf;
	struct probe_req_whitelist_attr *ie_whitelist = &psetoui->ie_whitelist;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		ie_whitelist->num_vendor_oui * sizeof(wmi_vendor_oui);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_scan_prob_req_oui_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_prob_req_oui_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_prob_req_oui_cmd_fixed_param));

	oui_buf = &cmd->prob_req_oui;
	qdf_mem_zero(oui_buf, sizeof(cmd->prob_req_oui));
	*oui_buf = psetoui->oui[0] << 16 | psetoui->oui[1] << 8
		   | psetoui->oui[2];
	WMI_LOGD("%s: wmi:oui received from hdd %08x", __func__,
		 cmd->prob_req_oui);

	cmd->vdev_id = psetoui->vdev_id;
	cmd->flags = WMI_SCAN_PROBE_OUI_SPOOFED_MAC_IN_PROBE_REQ;
	if (psetoui->enb_probe_req_sno_randomization)
		cmd->flags |= WMI_SCAN_PROBE_OUI_RANDOM_SEQ_NO_IN_PROBE_REQ;

	if (ie_whitelist->white_list) {
		wmi_fill_ie_whitelist_attrs(cmd->ie_bitmap,
					    &cmd->num_vendor_oui,
					    ie_whitelist);
		cmd->flags |=
			WMI_SCAN_PROBE_OUI_ENABLE_IE_WHITELIST_IN_PROBE_REQ;
	}

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       ie_whitelist->num_vendor_oui * sizeof(wmi_vendor_oui));
	buf_ptr += WMI_TLV_HDR_SIZE;

	if (cmd->num_vendor_oui != 0) {
		wmi_fill_vendor_oui(buf_ptr, cmd->num_vendor_oui,
				    ie_whitelist->voui);
		buf_ptr += cmd->num_vendor_oui * sizeof(wmi_vendor_oui);
	}

	wmi_mtrace(WMI_SCAN_PROB_REQ_OUI_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_SCAN_PROB_REQ_OUI_CMDID)) {
		WMI_LOGE("%s: failed to send command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef IPA_OFFLOAD
/** send_ipa_offload_control_cmd_tlv() - ipa offload control parameter
 * @wmi_handle: wmi handle
 * @ipa_offload: ipa offload control parameter
 *
 * Returns: 0 on success, error number otherwise
 */
static QDF_STATUS send_ipa_offload_control_cmd_tlv(wmi_unified_t wmi_handle,
		struct ipa_uc_offload_control_params *ipa_offload)
{
	wmi_ipa_offload_enable_disable_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	u_int8_t *buf_ptr;

	len  = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	WMI_LOGD("%s: offload_type=%d, enable=%d", __func__,
		ipa_offload->offload_type, ipa_offload->enable);

	buf_ptr = (u_int8_t *)wmi_buf_data(wmi_buf);

	cmd = (wmi_ipa_offload_enable_disable_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUCT_wmi_ipa_offload_enable_disable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_ipa_offload_enable_disable_cmd_fixed_param));

	cmd->offload_type = ipa_offload->offload_type;
	cmd->vdev_id = ipa_offload->vdev_id;
	cmd->enable = ipa_offload->enable;

	wmi_mtrace(WMI_IPA_OFFLOAD_ENABLE_DISABLE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
		WMI_IPA_OFFLOAD_ENABLE_DISABLE_CMDID)) {
		WMI_LOGE("%s: failed to command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * send_pno_stop_cmd_tlv() - PNO stop request
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function request FW to stop ongoing PNO operation.
 *
 * Return: CDF status
 */
static QDF_STATUS send_pno_stop_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	wmi_nlo_config_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	/*
	 * TLV place holder for array of structures nlo_configured_parameters
	 * TLV place holder for array of uint32_t channel_list
	 * TLV place holder for chnl prediction cfg
	 */
	len += WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_nlo_config_cmd_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_nlo_config_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->flags = WMI_NLO_CONFIG_STOP;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	wmi_mtrace(WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send nlo wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_set_pno_channel_prediction() - Set PNO channel prediction
 * @buf_ptr:      Buffer passed by upper layers
 * @pno:	  Buffer to be sent to the firmware
 *
 * Copy the PNO Channel prediction configuration parameters
 * passed by the upper layers to a WMI format TLV and send it
 * down to the firmware.
 *
 * Return: None
 */
static void wmi_set_pno_channel_prediction(uint8_t *buf_ptr,
		struct pno_scan_req_params *pno)
{
	nlo_channel_prediction_cfg *channel_prediction_cfg =
		(nlo_channel_prediction_cfg *) buf_ptr;
	WMITLV_SET_HDR(&channel_prediction_cfg->tlv_header,
			WMITLV_TAG_ARRAY_BYTE,
			WMITLV_GET_STRUCT_TLVLEN(nlo_channel_prediction_cfg));
#ifdef FEATURE_WLAN_SCAN_PNO
	channel_prediction_cfg->enable = pno->pno_channel_prediction;
	channel_prediction_cfg->top_k_num = pno->top_k_num_of_channels;
	channel_prediction_cfg->stationary_threshold = pno->stationary_thresh;
	channel_prediction_cfg->full_scan_period_ms =
		pno->channel_prediction_full_scan;
#endif
	buf_ptr += sizeof(nlo_channel_prediction_cfg);
	WMI_LOGD("enable: %d, top_k_num: %d, stat_thresh: %d, full_scan: %d",
			channel_prediction_cfg->enable,
			channel_prediction_cfg->top_k_num,
			channel_prediction_cfg->stationary_threshold,
			channel_prediction_cfg->full_scan_period_ms);
}

/**
 * send_nlo_mawc_cmd_tlv() - Send MAWC NLO configuration
 * @wmi_handle: wmi handle
 * @params: configuration parameters
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_nlo_mawc_cmd_tlv(wmi_unified_t wmi_handle,
		struct nlo_mawc_params *params)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_nlo_configure_mawc_cmd_fixed_param *wmi_nlo_mawc_params;

	len = sizeof(*wmi_nlo_mawc_params);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	wmi_nlo_mawc_params =
		(wmi_nlo_configure_mawc_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&wmi_nlo_mawc_params->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_configure_mawc_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_nlo_configure_mawc_cmd_fixed_param));
	wmi_nlo_mawc_params->vdev_id = params->vdev_id;
	if (params->enable)
		wmi_nlo_mawc_params->enable = 1;
	else
		wmi_nlo_mawc_params->enable = 0;
	wmi_nlo_mawc_params->exp_backoff_ratio = params->exp_backoff_ratio;
	wmi_nlo_mawc_params->init_scan_interval = params->init_scan_interval;
	wmi_nlo_mawc_params->max_scan_interval = params->max_scan_interval;
	WMI_LOGD(FL("MAWC NLO en=%d, vdev=%d, ratio=%d, SCAN init=%d, max=%d"),
		wmi_nlo_mawc_params->enable, wmi_nlo_mawc_params->vdev_id,
		wmi_nlo_mawc_params->exp_backoff_ratio,
		wmi_nlo_mawc_params->init_scan_interval,
		wmi_nlo_mawc_params->max_scan_interval);

	wmi_mtrace(WMI_NLO_CONFIGURE_MAWC_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_NLO_CONFIGURE_MAWC_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("WMI_NLO_CONFIGURE_MAWC_CMDID failed, Error %d",
			status);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_pno_start_cmd_tlv() - PNO start request
 * @wmi_handle: wmi handle
 * @pno: PNO request
 *
 * This function request FW to start PNO request.
 * Request: CDF status
 */
static QDF_STATUS send_pno_start_cmd_tlv(wmi_unified_t wmi_handle,
		   struct pno_scan_req_params *pno)
{
	wmi_nlo_config_cmd_fixed_param *cmd;
	nlo_configured_parameters *nlo_list;
	uint32_t *channel_list;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint8_t i;
	int ret;
	struct probe_req_whitelist_attr *ie_whitelist = &pno->ie_whitelist;
	connected_nlo_rssi_params *nlo_relative_rssi;
	connected_nlo_bss_band_rssi_pref *nlo_band_rssi;

	/*
	 * TLV place holder for array nlo_configured_parameters(nlo_list)
	 * TLV place holder for array of uint32_t channel_list
	 * TLV place holder for chnnl prediction cfg
	 * TLV place holder for array of wmi_vendor_oui
	 * TLV place holder for array of connected_nlo_bss_band_rssi_pref
	 */
	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE +
		WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE;

	len += sizeof(uint32_t) * QDF_MIN(pno->networks_list[0].channel_cnt,
					  WMI_NLO_MAX_CHAN);
	len += sizeof(nlo_configured_parameters) *
	       QDF_MIN(pno->networks_cnt, WMI_NLO_MAX_SSIDS);
	len += sizeof(nlo_channel_prediction_cfg);
	len += sizeof(enlo_candidate_score_params);
	len += sizeof(wmi_vendor_oui) * ie_whitelist->num_vendor_oui;
	len += sizeof(connected_nlo_rssi_params);
	len += sizeof(connected_nlo_bss_band_rssi_pref);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_nlo_config_cmd_fixed_param *) wmi_buf_data(buf);

	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_nlo_config_cmd_fixed_param));
	cmd->vdev_id = pno->vdev_id;
	cmd->flags = WMI_NLO_CONFIG_START | WMI_NLO_CONFIG_SSID_HIDE_EN;

#ifdef FEATURE_WLAN_SCAN_PNO
	WMI_SCAN_SET_DWELL_MODE(cmd->flags,
			pno->adaptive_dwell_mode);
#endif
	/* Current FW does not support min-max range for dwell time */
	cmd->active_dwell_time = pno->active_dwell_time;
	cmd->passive_dwell_time = pno->passive_dwell_time;

	if (pno->do_passive_scan)
		cmd->flags |= WMI_NLO_CONFIG_SCAN_PASSIVE;
	/* Copy scan interval */
	cmd->fast_scan_period = pno->fast_scan_period;
	cmd->slow_scan_period = pno->slow_scan_period;
	cmd->delay_start_time = WMI_SEC_TO_MSEC(pno->delay_start_time);
	cmd->fast_scan_max_cycles = pno->fast_scan_max_cycles;
	cmd->scan_backoff_multiplier = pno->scan_backoff_multiplier;
	WMI_LOGD("fast_scan_period: %d msec slow_scan_period: %d msec",
			cmd->fast_scan_period, cmd->slow_scan_period);
	WMI_LOGD("fast_scan_max_cycles: %d", cmd->fast_scan_max_cycles);

	/* mac randomization attributes */
	if (pno->scan_random.randomize) {
		cmd->flags |= WMI_NLO_CONFIG_SPOOFED_MAC_IN_PROBE_REQ |
				WMI_NLO_CONFIG_RANDOM_SEQ_NO_IN_PROBE_REQ;
		wmi_copy_scan_random_mac(pno->scan_random.mac_addr,
					 pno->scan_random.mac_mask,
					 &cmd->mac_addr,
					 &cmd->mac_mask);
	}

	buf_ptr += sizeof(wmi_nlo_config_cmd_fixed_param);

	cmd->no_of_ssids = QDF_MIN(pno->networks_cnt, WMI_NLO_MAX_SSIDS);
	WMI_LOGD("SSID count : %d", cmd->no_of_ssids);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->no_of_ssids * sizeof(nlo_configured_parameters));
	buf_ptr += WMI_TLV_HDR_SIZE;

	nlo_list = (nlo_configured_parameters *) buf_ptr;
	for (i = 0; i < cmd->no_of_ssids; i++) {
		WMITLV_SET_HDR(&nlo_list[i].tlv_header,
			       WMITLV_TAG_ARRAY_BYTE,
			       WMITLV_GET_STRUCT_TLVLEN
				       (nlo_configured_parameters));
		/* Copy ssid and it's length */
		nlo_list[i].ssid.valid = true;
		nlo_list[i].ssid.ssid.ssid_len =
			pno->networks_list[i].ssid.length;
		qdf_mem_copy(nlo_list[i].ssid.ssid.ssid,
			     pno->networks_list[i].ssid.ssid,
			     nlo_list[i].ssid.ssid.ssid_len);
		WMI_LOGD("index: %d ssid: %.*s len: %d", i,
			 nlo_list[i].ssid.ssid.ssid_len,
			 (char *)nlo_list[i].ssid.ssid.ssid,
			 nlo_list[i].ssid.ssid.ssid_len);

		/* Copy rssi threshold */
		if (pno->networks_list[i].rssi_thresh &&
		    pno->networks_list[i].rssi_thresh >
		    WMI_RSSI_THOLD_DEFAULT) {
			nlo_list[i].rssi_cond.valid = true;
			nlo_list[i].rssi_cond.rssi =
				pno->networks_list[i].rssi_thresh;
			WMI_LOGD("RSSI threshold : %d dBm",
				 nlo_list[i].rssi_cond.rssi);
		}
		nlo_list[i].bcast_nw_type.valid = true;
		nlo_list[i].bcast_nw_type.bcast_nw_type =
			pno->networks_list[i].bc_new_type;
		WMI_LOGD("Broadcast NW type (%u)",
			 nlo_list[i].bcast_nw_type.bcast_nw_type);
	}
	buf_ptr += cmd->no_of_ssids * sizeof(nlo_configured_parameters);

	/* Copy channel info */
	cmd->num_of_channels = QDF_MIN(pno->networks_list[0].channel_cnt,
				       WMI_NLO_MAX_CHAN);
	WMI_LOGD("Channel count: %d", cmd->num_of_channels);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (cmd->num_of_channels * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	channel_list = (uint32_t *) buf_ptr;
	for (i = 0; i < cmd->num_of_channels; i++) {
		channel_list[i] = pno->networks_list[0].channels[i];

		if (channel_list[i] < WMI_NLO_FREQ_THRESH)
			channel_list[i] =
				wlan_chan_to_freq(pno->
					networks_list[0].channels[i]);

		WMI_LOGD("Ch[%d]: %d MHz", i, channel_list[i]);
	}
	buf_ptr += cmd->num_of_channels * sizeof(uint32_t);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			sizeof(nlo_channel_prediction_cfg));
	buf_ptr += WMI_TLV_HDR_SIZE;
	wmi_set_pno_channel_prediction(buf_ptr, pno);
	buf_ptr += sizeof(nlo_channel_prediction_cfg);
	/** TODO: Discrete firmware doesn't have command/option to configure
	 * App IE which comes from wpa_supplicant as of part PNO start request.
	 */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_enlo_candidate_score_param,
		       WMITLV_GET_STRUCT_TLVLEN(enlo_candidate_score_params));
	buf_ptr += sizeof(enlo_candidate_score_params);

	if (ie_whitelist->white_list) {
		cmd->flags |= WMI_NLO_CONFIG_ENABLE_IE_WHITELIST_IN_PROBE_REQ;
		wmi_fill_ie_whitelist_attrs(cmd->ie_bitmap,
					    &cmd->num_vendor_oui,
					    ie_whitelist);
	}

	/* ie white list */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       ie_whitelist->num_vendor_oui * sizeof(wmi_vendor_oui));
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (cmd->num_vendor_oui != 0) {
		wmi_fill_vendor_oui(buf_ptr, cmd->num_vendor_oui,
				    ie_whitelist->voui);
		buf_ptr += cmd->num_vendor_oui * sizeof(wmi_vendor_oui);
	}

	if (pno->relative_rssi_set)
		cmd->flags |= WMI_NLO_CONFIG_ENABLE_CNLO_RSSI_CONFIG;

	/*
	 * Firmware calculation using connected PNO params:
	 * New AP's RSSI >= (Connected AP's RSSI + relative_rssi +/- rssi_pref)
	 * deduction of rssi_pref for chosen band_pref and
	 * addition of rssi_pref for remaining bands (other than chosen band).
	 */
	nlo_relative_rssi = (connected_nlo_rssi_params *) buf_ptr;
	WMITLV_SET_HDR(&nlo_relative_rssi->tlv_header,
		WMITLV_TAG_STRUC_wmi_connected_nlo_rssi_params,
		WMITLV_GET_STRUCT_TLVLEN(connected_nlo_rssi_params));
	nlo_relative_rssi->relative_rssi = pno->relative_rssi;
	WMI_LOGD("relative_rssi %d", nlo_relative_rssi->relative_rssi);
	buf_ptr += sizeof(*nlo_relative_rssi);

	/*
	 * As of now Kernel and Host supports one band and rssi preference.
	 * Firmware supports array of band and rssi preferences
	 */
	cmd->num_cnlo_band_pref = 1;
	WMITLV_SET_HDR(buf_ptr,
		WMITLV_TAG_ARRAY_STRUC,
		cmd->num_cnlo_band_pref *
		sizeof(connected_nlo_bss_band_rssi_pref));
	buf_ptr += WMI_TLV_HDR_SIZE;

	nlo_band_rssi = (connected_nlo_bss_band_rssi_pref *) buf_ptr;
	for (i = 0; i < cmd->num_cnlo_band_pref; i++) {
		WMITLV_SET_HDR(&nlo_band_rssi[i].tlv_header,
			WMITLV_TAG_STRUC_wmi_connected_nlo_bss_band_rssi_pref,
			WMITLV_GET_STRUCT_TLVLEN(
				connected_nlo_bss_band_rssi_pref));
		nlo_band_rssi[i].band = pno->band_rssi_pref.band;
		nlo_band_rssi[i].rssi_pref = pno->band_rssi_pref.rssi;
		WMI_LOGI("band_pref %d, rssi_pref %d",
			nlo_band_rssi[i].band,
			nlo_band_rssi[i].rssi_pref);
	}
	buf_ptr += cmd->num_cnlo_band_pref * sizeof(*nlo_band_rssi);

	wmi_mtrace(WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send nlo wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
/**
 * send_process_ll_stats_clear_cmd_tlv() - clear link layer stats
 * @wmi_handle: wmi handle
 * @clear_req: ll stats clear request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_ll_stats_clear_cmd_tlv(wmi_unified_t wmi_handle,
		const struct ll_stats_clear_params *clear_req)
{
	wmi_clear_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_clear_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_clear_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_clear_link_stats_cmd_fixed_param));

	cmd->stop_stats_collection_req = clear_req->stop_req;
	cmd->vdev_id = clear_req->vdev_id;
	cmd->stats_clear_req_mask = clear_req->stats_clear_mask;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(clear_req->peer_macaddr.bytes,
				   &cmd->peer_macaddr);

	WMI_LOGD("LINK_LAYER_STATS - Clear Request Params");
	WMI_LOGD("StopReq: %d", cmd->stop_stats_collection_req);
	WMI_LOGD("Vdev Id: %d", cmd->vdev_id);
	WMI_LOGD("Clear Stat Mask: %d", cmd->stats_clear_req_mask);
	WMI_LOGD("Peer MAC Addr: %pM", clear_req->peer_macaddr.bytes);

	wmi_mtrace(WMI_CLEAR_LINK_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_CLEAR_LINK_STATS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send clear link stats req", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("Clear Link Layer Stats request sent successfully");
	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_ll_stats_set_cmd_tlv() - link layer stats set request
 * @wmi_handle: wmi handle
 * @set_req: ll stats set request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_ll_stats_set_cmd_tlv(wmi_unified_t wmi_handle,
		const struct ll_stats_set_params *set_req)
{
	wmi_start_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_start_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_start_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_start_link_stats_cmd_fixed_param));

	cmd->mpdu_size_threshold = set_req->mpdu_size_threshold;
	cmd->aggressive_statistics_gathering =
		set_req->aggressive_statistics_gathering;

	WMI_LOGD("LINK_LAYER_STATS - Start/Set Request Params");
	WMI_LOGD("MPDU Size Thresh : %d", cmd->mpdu_size_threshold);
	WMI_LOGD("Aggressive Gather: %d", cmd->aggressive_statistics_gathering);

	wmi_mtrace(WMI_START_LINK_STATS_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_START_LINK_STATS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send set link stats request", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_ll_stats_get_cmd_tlv() - link layer stats get request
 * @wmi_handle: wmi handle
 * @get_req: ll stats get request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_ll_stats_get_cmd_tlv(wmi_unified_t wmi_handle,
				const struct ll_stats_get_params  *get_req)
{
	wmi_request_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_request_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_link_stats_cmd_fixed_param));

	cmd->request_id = get_req->req_id;
	cmd->stats_type = get_req->param_id_mask;
	cmd->vdev_id = get_req->vdev_id;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(get_req->peer_macaddr.bytes,
				   &cmd->peer_macaddr);

	WMI_LOGD("LINK_LAYER_STATS - Get Request Params");
	WMI_LOGD("Request ID: %u", cmd->request_id);
	WMI_LOGD("Stats Type: %0x", cmd->stats_type);
	WMI_LOGD("Vdev ID: %d", cmd->vdev_id);
	WMI_LOGD("Peer MAC Addr: %pM", get_req->peer_macaddr.bytes);

	wmi_mtrace(WMI_REQUEST_LINK_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_REQUEST_LINK_STATS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send get link stats request", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * send_congestion_cmd_tlv() - send request to fw to get CCA
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_congestion_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t vdev_id)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_request_stats_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));

	cmd->stats_id = WMI_REQUEST_CONGESTION_STAT;
	cmd->vdev_id = vdev_id;
	WMI_LOGD("STATS REQ VDEV_ID:%d stats_id %d -->",
			cmd->vdev_id, cmd->stats_id);

	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("%s: Failed to send WMI_REQUEST_STATS_CMDID",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_snr_request_cmd_tlv() - send request to fw to get RSSI stats
 * @wmi_handle: wmi handle
 * @rssi_req: get RSSI request
 *
 * Return: CDF status
 */
static QDF_STATUS send_snr_request_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = WMI_REQUEST_VDEV_STAT;
	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send
		    (wmi_handle, buf, len, WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("Failed to send host stats request to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_snr_cmd_tlv() - get RSSI from fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_snr_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = WMI_REQUEST_VDEV_STAT;
	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("Failed to send host stats request to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_link_status_req_cmd_tlv() - process link status request from UMAC
 * @wmi_handle: wmi handle
 * @link_status: get link params
 *
 * Return: CDF status
 */
static QDF_STATUS send_link_status_req_cmd_tlv(wmi_unified_t wmi_handle,
				 struct link_status_params *link_status)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = WMI_REQUEST_VDEV_RATE_STAT;
	cmd->vdev_id = link_status->vdev_id;
	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("Failed to send WMI link  status request to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_GREEN_AP
/**
 * send_egap_conf_params_cmd_tlv() - send wmi cmd of egap configuration params
 * @wmi_handle:	 wmi handler
 * @egap_params: pointer to egap_params
 *
 * Return:	 0 for success, otherwise appropriate error code
 */
static QDF_STATUS send_egap_conf_params_cmd_tlv(wmi_unified_t wmi_handle,
		     struct wlan_green_ap_egap_params *egap_params)
{
	wmi_ap_ps_egap_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_ap_ps_egap_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ap_ps_egap_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_ap_ps_egap_param_cmd_fixed_param));

	cmd->enable = egap_params->host_enable_egap;
	cmd->inactivity_time = egap_params->egap_inactivity_time;
	cmd->wait_time = egap_params->egap_wait_time;
	cmd->flags = egap_params->egap_feature_flags;
	wmi_mtrace(WMI_AP_PS_EGAP_PARAM_CMDID, NO_SESSION, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd), WMI_AP_PS_EGAP_PARAM_CMDID);
	if (err) {
		WMI_LOGE("Failed to send ap_ps_egap cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * wmi_unified_csa_offload_enable() - sen CSA offload enable command
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_csa_offload_enable_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t vdev_id)
{
	wmi_csa_offload_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("%s: vdev_id %d", __func__, vdev_id);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_csa_offload_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_csa_offload_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_csa_offload_enable_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->csa_offload_enable = WMI_CSA_OFFLOAD_ENABLE;
	wmi_mtrace(WMI_CSA_OFFLOAD_ENABLE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_CSA_OFFLOAD_ENABLE_CMDID)) {
		WMI_LOGP("%s: Failed to send CSA offload enable command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

#ifdef WLAN_FEATURE_CIF_CFR
/**
 * send_oem_dma_cfg_cmd_tlv() - configure OEM DMA rings
 * @wmi_handle: wmi handle
 * @data_len: len of dma cfg req
 * @data: dma cfg req
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS send_oem_dma_cfg_cmd_tlv(wmi_unified_t wmi_handle,
				wmi_oem_dma_ring_cfg_req_fixed_param *cfg)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	QDF_STATUS ret;

	WMITLV_SET_HDR(cfg,
		WMITLV_TAG_STRUC_wmi_oem_dma_ring_cfg_req_fixed_param,
		(sizeof(*cfg) - WMI_TLV_HDR_SIZE));

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cfg));
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_copy(cmd, cfg, sizeof(*cfg));
	WMI_LOGI(FL("Sending OEM Data Request to target, data len %lu"),
		sizeof(*cfg));
	wmi_mtrace(WMI_OEM_DMA_RING_CFG_REQ_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cfg),
				WMI_OEM_DMA_RING_CFG_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL(":wmi cmd send failed"));
		wmi_buf_free(buf);
	}

	return ret;
}
#endif

/**
 * send_start_11d_scan_cmd_tlv() - start 11d scan request
 * @wmi_handle: wmi handle
 * @start_11d_scan: 11d scan start request parameters
 *
 * This function request FW to start 11d scan.
 *
 * Return: QDF status
 */
static QDF_STATUS send_start_11d_scan_cmd_tlv(wmi_unified_t wmi_handle,
			  struct reg_start_11d_scan_req *start_11d_scan)
{
	wmi_11d_scan_start_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_11d_scan_start_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_11d_scan_start_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_11d_scan_start_cmd_fixed_param));

	cmd->vdev_id = start_11d_scan->vdev_id;
	cmd->scan_period_msec = start_11d_scan->scan_period_msec;
	cmd->start_interval_msec = start_11d_scan->start_interval_msec;

	WMI_LOGD("vdev %d sending 11D scan start req", cmd->vdev_id);

	wmi_mtrace(WMI_11D_SCAN_START_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_11D_SCAN_START_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send start 11d scan wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_stop_11d_scan_cmd_tlv() - stop 11d scan request
 * @wmi_handle: wmi handle
 * @start_11d_scan: 11d scan stop request parameters
 *
 * This function request FW to stop 11d scan.
 *
 * Return: QDF status
 */
static QDF_STATUS send_stop_11d_scan_cmd_tlv(wmi_unified_t wmi_handle,
			  struct reg_stop_11d_scan_req *stop_11d_scan)
{
	wmi_11d_scan_stop_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_11d_scan_stop_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_11d_scan_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_11d_scan_stop_cmd_fixed_param));

	cmd->vdev_id = stop_11d_scan->vdev_id;

	WMI_LOGD("vdev %d sending 11D scan stop req", cmd->vdev_id);

	wmi_mtrace(WMI_11D_SCAN_STOP_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_11D_SCAN_STOP_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send stop 11d scan wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_start_oem_data_cmd_tlv() - start OEM data request to target
 * @wmi_handle: wmi handle
 * @startOemDataReq: start request params
 *
 * Return: CDF status
 */
static QDF_STATUS send_start_oem_data_cmd_tlv(wmi_unified_t wmi_handle,
			  uint32_t data_len,
			  uint8_t *data)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle,
			    (data_len + WMI_TLV_HDR_SIZE));
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (uint8_t *) wmi_buf_data(buf);

	WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE, data_len);
	cmd += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(cmd, data,
		     data_len);

	WMI_LOGD(FL("Sending OEM Data Request to target, data len %d"),
		 data_len);

	wmi_mtrace(WMI_OEM_REQ_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf,
				   (data_len +
				    WMI_TLV_HDR_SIZE), WMI_OEM_REQ_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL(":wmi cmd send failed"));
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dfs_phyerr_filter_offload_en_cmd_tlv() - enable dfs phyerr filter
 * @wmi_handle: wmi handle
 * @dfs_phyerr_filter_offload: is dfs phyerr filter offload
 *
 * Send WMI_DFS_PHYERR_FILTER_ENA_CMDID or
 * WMI_DFS_PHYERR_FILTER_DIS_CMDID command
 * to firmware based on phyerr filtering
 * offload status.
 *
 * Return: 1 success, 0 failure
 */
static QDF_STATUS
send_dfs_phyerr_filter_offload_en_cmd_tlv(wmi_unified_t wmi_handle,
			bool dfs_phyerr_filter_offload)
{
	wmi_dfs_phyerr_filter_ena_cmd_fixed_param *enable_phyerr_offload_cmd;
	wmi_dfs_phyerr_filter_dis_cmd_fixed_param *disable_phyerr_offload_cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;


	if (false == dfs_phyerr_filter_offload) {
		WMI_LOGD("%s:Phyerror Filtering offload is Disabled in ini",
			 __func__);
		len = sizeof(*disable_phyerr_offload_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf)
			return 0;

		disable_phyerr_offload_cmd =
			(wmi_dfs_phyerr_filter_dis_cmd_fixed_param *)
			wmi_buf_data(buf);

		WMITLV_SET_HDR(&disable_phyerr_offload_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_dfs_phyerr_filter_dis_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		     (wmi_dfs_phyerr_filter_dis_cmd_fixed_param));

		/*
		 * Send WMI_DFS_PHYERR_FILTER_DIS_CMDID
		 * to the firmware to disable the phyerror
		 * filtering offload.
		 */
		wmi_mtrace(WMI_DFS_PHYERR_FILTER_DIS_CMDID, NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_DFS_PHYERR_FILTER_DIS_CMDID);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMI_LOGE("%s: Failed to send WMI_DFS_PHYERR_FILTER_DIS_CMDID ret=%d",
				__func__, ret);
			wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
		}
		WMI_LOGD("%s: WMI_DFS_PHYERR_FILTER_DIS_CMDID Send Success",
			 __func__);
	} else {
		WMI_LOGD("%s:Phyerror Filtering offload is Enabled in ini",
			 __func__);

		len = sizeof(*enable_phyerr_offload_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf)
			return QDF_STATUS_E_FAILURE;

		enable_phyerr_offload_cmd =
			(wmi_dfs_phyerr_filter_ena_cmd_fixed_param *)
			wmi_buf_data(buf);

		WMITLV_SET_HDR(&enable_phyerr_offload_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_dfs_phyerr_filter_ena_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		     (wmi_dfs_phyerr_filter_ena_cmd_fixed_param));

		/*
		 * Send a WMI_DFS_PHYERR_FILTER_ENA_CMDID
		 * to the firmware to enable the phyerror
		 * filtering offload.
		 */
		wmi_mtrace(WMI_DFS_PHYERR_FILTER_ENA_CMDID, NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_DFS_PHYERR_FILTER_ENA_CMDID);

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMI_LOGE("%s: Failed to send DFS PHYERR CMD ret=%d",
				__func__, ret);
			wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
		}
		WMI_LOGD("%s: WMI_DFS_PHYERR_FILTER_ENA_CMDID Send Success",
			 __func__);
	}

	return QDF_STATUS_SUCCESS;
}

#if !defined(REMOVE_PKT_LOG) && defined(CONFIG_MCL)
/**
 * send_pktlog_wmi_send_cmd_tlv() - send pktlog enable/disable command to target
 * @wmi_handle: wmi handle
 * @pktlog_event: pktlog event
 * @cmd_id: pktlog cmd id
 *
 * Return: CDF status
 */
static QDF_STATUS send_pktlog_wmi_send_cmd_tlv(wmi_unified_t wmi_handle,
				   WMI_PKTLOG_EVENT pktlog_event,
				   WMI_CMD_ID cmd_id, uint8_t user_triggered)
{
	WMI_PKTLOG_EVENT PKTLOG_EVENT;
	WMI_CMD_ID CMD_ID;
	wmi_pdev_pktlog_enable_cmd_fixed_param *cmd;
	wmi_pdev_pktlog_disable_cmd_fixed_param *disable_cmd;
	int len = 0;
	wmi_buf_t buf;

	PKTLOG_EVENT = pktlog_event;
	CMD_ID = cmd_id;

	switch (CMD_ID) {
	case WMI_PDEV_PKTLOG_ENABLE_CMDID:
		len = sizeof(*cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf)
			return QDF_STATUS_E_NOMEM;

		cmd = (wmi_pdev_pktlog_enable_cmd_fixed_param *)
			wmi_buf_data(buf);
		WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_pktlog_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_pdev_pktlog_enable_cmd_fixed_param));
		cmd->evlist = PKTLOG_EVENT;
		cmd->enable = user_triggered ? WMI_PKTLOG_ENABLE_FORCE
					: WMI_PKTLOG_ENABLE_AUTO;
		cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
		wmi_mtrace(WMI_PDEV_PKTLOG_ENABLE_CMDID, NO_SESSION, 0);
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_ENABLE_CMDID)) {
			WMI_LOGE("failed to send pktlog enable cmdid");
			goto wmi_send_failed;
		}
		break;
	case WMI_PDEV_PKTLOG_DISABLE_CMDID:
		len = sizeof(*disable_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf)
			return QDF_STATUS_E_NOMEM;

		disable_cmd = (wmi_pdev_pktlog_disable_cmd_fixed_param *)
			      wmi_buf_data(buf);
		WMITLV_SET_HDR(&disable_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_pdev_pktlog_disable_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		     (wmi_pdev_pktlog_disable_cmd_fixed_param));
		disable_cmd->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
		wmi_mtrace(WMI_PDEV_PKTLOG_DISABLE_CMDID, NO_SESSION, 0);
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_DISABLE_CMDID)) {
			WMI_LOGE("failed to send pktlog disable cmdid");
			goto wmi_send_failed;
		}
		break;
	default:
		WMI_LOGD("%s: invalid PKTLOG command", __func__);
		break;
	}

	return QDF_STATUS_SUCCESS;

wmi_send_failed:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}
#endif /* !REMOVE_PKT_LOG &&  CONFIG_MCL*/

/**
 * send_stats_ext_req_cmd_tlv() - request ext stats from fw
 * @wmi_handle: wmi handle
 * @preq: stats ext params
 *
 * Return: CDF status
 */
static QDF_STATUS send_stats_ext_req_cmd_tlv(wmi_unified_t wmi_handle,
			struct stats_ext_params *preq)
{
	QDF_STATUS ret;
	wmi_req_stats_ext_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	size_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + preq->request_data_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_req_stats_ext_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_req_stats_ext_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_req_stats_ext_cmd_fixed_param));
	cmd->vdev_id = preq->vdev_id;
	cmd->data_len = preq->request_data_len;

	WMI_LOGD("%s: The data len value is %u and vdev id set is %u ",
		 __func__, preq->request_data_len, preq->vdev_id);

	buf_ptr += sizeof(wmi_req_stats_ext_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, cmd->data_len);

	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, preq->request_data, cmd->data_len);

	wmi_mtrace(WMI_REQUEST_STATS_EXT_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_REQUEST_STATS_EXT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send notify cmd ret = %d", __func__,
			 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_process_dhcpserver_offload_cmd_tlv() - enable DHCP server offload
 * @wmi_handle: wmi handle
 * @params: DHCP server offload info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_process_dhcpserver_offload_cmd_tlv(wmi_unified_t wmi_handle,
					struct dhcp_offload_info_params *params)
{
	wmi_set_dhcp_server_offload_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_set_dhcp_server_offload_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_set_dhcp_server_offload_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_set_dhcp_server_offload_cmd_fixed_param));
	cmd->vdev_id = params->vdev_id;
	cmd->enable = params->dhcp_offload_enabled;
	cmd->num_client = params->dhcp_client_num;
	cmd->srv_ipv4 = params->dhcp_srv_addr;
	cmd->start_lsb = 0;
	wmi_mtrace(WMI_SET_DHCP_SERVER_OFFLOAD_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd),
				   WMI_SET_DHCP_SERVER_OFFLOAD_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send set_dhcp_server_offload cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("Set dhcp server offload to vdevId %d",
		 params->vdev_id);

	return status;
}

/**
 * send_pdev_set_regdomain_cmd_tlv() - send set regdomain command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev regdomain params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_set_regdomain_cmd_tlv(wmi_unified_t wmi_handle,
				struct pdev_set_regdomain_params *param)
{
	wmi_buf_t buf;
	wmi_pdev_set_regdomain_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_set_regdomain_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_regdomain_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_regdomain_cmd_fixed_param));

	cmd->reg_domain = param->currentRDinuse;
	cmd->reg_domain_2G = param->currentRD2G;
	cmd->reg_domain_5G = param->currentRD5G;
	cmd->conformance_test_limit_2G = param->ctl_2G;
	cmd->conformance_test_limit_5G = param->ctl_5G;
	cmd->dfs_domain = param->dfsDomain;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							param->pdev_id);

	wmi_mtrace(WMI_PDEV_SET_REGDOMAIN_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_REGDOMAIN_CMDID)) {
		WMI_LOGE("%s: Failed to send pdev set regdomain command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_regdomain_info_to_fw_cmd_tlv() - send regdomain info to fw
 * @wmi_handle: wmi handle
 * @reg_dmn: reg domain
 * @regdmn2G: 2G reg domain
 * @regdmn5G: 5G reg domain
 * @ctl2G: 2G test limit
 * @ctl5G: 5G test limit
 *
 * Return: none
 */
static QDF_STATUS send_regdomain_info_to_fw_cmd_tlv(wmi_unified_t wmi_handle,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, uint8_t ctl2G,
				   uint8_t ctl5G)
{
	wmi_buf_t buf;
	wmi_pdev_set_regdomain_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_set_regdomain_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_regdomain_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_regdomain_cmd_fixed_param));
	cmd->reg_domain = reg_dmn;
	cmd->reg_domain_2G = regdmn2G;
	cmd->reg_domain_5G = regdmn5G;
	cmd->conformance_test_limit_2G = ctl2G;
	cmd->conformance_test_limit_5G = ctl5G;

	wmi_mtrace(WMI_PDEV_SET_REGDOMAIN_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_REGDOMAIN_CMDID)) {
		WMI_LOGP("%s: Failed to send pdev set regdomain command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * copy_custom_aggr_bitmap() - copies host side bitmap using FW APIs
 * @param: param sent from the host side
 * @cmd: param to be sent to the fw side
 */
static inline void copy_custom_aggr_bitmap(
		struct set_custom_aggr_size_params *param,
		wmi_vdev_set_custom_aggr_size_cmd_fixed_param *cmd)
{
	WMI_VDEV_CUSTOM_AGGR_AC_SET(cmd->enable_bitmap,
				    param->ac);
	WMI_VDEV_CUSTOM_AGGR_TYPE_SET(cmd->enable_bitmap,
				      param->aggr_type);
	WMI_VDEV_CUSTOM_TX_AGGR_SZ_DIS_SET(cmd->enable_bitmap,
					   param->tx_aggr_size_disable);
	WMI_VDEV_CUSTOM_RX_AGGR_SZ_DIS_SET(cmd->enable_bitmap,
					   param->rx_aggr_size_disable);
	WMI_VDEV_CUSTOM_TX_AC_EN_SET(cmd->enable_bitmap,
				     param->tx_ac_enable);
}

/**
 * send_vdev_set_custom_aggr_size_cmd_tlv() - custom aggr size param in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold custom aggr size params
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_custom_aggr_size_cmd_tlv(
			wmi_unified_t wmi_handle,
			struct set_custom_aggr_size_params *param)
{
	wmi_vdev_set_custom_aggr_size_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_vdev_set_custom_aggr_size_cmd_fixed_param *)
		wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_set_custom_aggr_size_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_vdev_set_custom_aggr_size_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->tx_aggr_size = param->tx_aggr_size;
	cmd->rx_aggr_size = param->rx_aggr_size;
	copy_custom_aggr_bitmap(param, cmd);

	WMI_LOGD("Set custom aggr: vdev id=0x%X, tx aggr size=0x%X "
		"rx_aggr_size=0x%X access category=0x%X, agg_type=0x%X "
		"tx_aggr_size_disable=0x%X, rx_aggr_size_disable=0x%X "
		"tx_ac_enable=0x%X",
		param->vdev_id, param->tx_aggr_size, param->rx_aggr_size,
		param->ac, param->aggr_type, param->tx_aggr_size_disable,
		param->rx_aggr_size_disable, param->tx_ac_enable);

	wmi_mtrace(WMI_VDEV_SET_CUSTOM_AGGR_SIZE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_CUSTOM_AGGR_SIZE_CMDID)) {
		WMI_LOGE("Seting custom aggregation size failed");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_vdev_set_qdepth_thresh_cmd_tlv() - WMI set qdepth threshold
 *  @param wmi_handle  : handle to WMI.
 *  @param param       : pointer to tx antenna param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */

static QDF_STATUS send_vdev_set_qdepth_thresh_cmd_tlv(wmi_unified_t wmi_handle,
				struct set_qdepth_thresh_params *param)
{
	wmi_peer_tid_msduq_qdepth_thresh_update_cmd_fixed_param *cmd;
	wmi_msduq_qdepth_thresh_update *cmd_update;
	wmi_buf_t buf;
	int32_t len = 0;
	int i;
	uint8_t *buf_ptr;
	QDF_STATUS ret;

	if (param->num_of_msduq_updates > QDEPTH_THRESH_MAX_UPDATES) {
		WMI_LOGE("%s: Invalid Update Count!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += (sizeof(wmi_msduq_qdepth_thresh_update) *
			param->num_of_msduq_updates);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_tid_msduq_qdepth_thresh_update_cmd_fixed_param *)
								buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_tid_msduq_qdepth_thresh_update_cmd_fixed_param
	 , WMITLV_GET_STRUCT_TLVLEN(
		wmi_peer_tid_msduq_qdepth_thresh_update_cmd_fixed_param));

	cmd->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(param->pdev_id);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->mac_addr, &cmd->peer_mac_address);
	cmd->num_of_msduq_updates = param->num_of_msduq_updates;

	buf_ptr += sizeof(
		wmi_peer_tid_msduq_qdepth_thresh_update_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			param->num_of_msduq_updates *
			sizeof(wmi_msduq_qdepth_thresh_update));
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd_update = (wmi_msduq_qdepth_thresh_update *)buf_ptr;

	for (i = 0; i < cmd->num_of_msduq_updates; i++) {
		WMITLV_SET_HDR(&cmd_update->tlv_header,
		    WMITLV_TAG_STRUC_wmi_msduq_qdepth_thresh_update,
		    WMITLV_GET_STRUCT_TLVLEN(
				wmi_msduq_qdepth_thresh_update));
		cmd_update->tid_num = param->update_params[i].tid_num;
		cmd_update->msduq_update_mask =
				param->update_params[i].msduq_update_mask;
		cmd_update->qdepth_thresh_value =
				param->update_params[i].qdepth_thresh_value;
		WMI_LOGD("Set QDepth Threshold: vdev=0x%X pdev=0x%X, tid=0x%X "
			 "mac_addr_upper4=%X, mac_addr_lower2:%X,"
			 " update mask=0x%X thresh val=0x%X",
			 cmd->vdev_id, cmd->pdev_id, cmd_update->tid_num,
			 cmd->peer_mac_address.mac_addr31to0,
			 cmd->peer_mac_address.mac_addr47to32,
			 cmd_update->msduq_update_mask,
			 cmd_update->qdepth_thresh_value);
		cmd_update++;
	}

	wmi_mtrace(WMI_PEER_TID_MSDUQ_QDEPTH_THRESH_UPDATE_CMDID,
		   cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PEER_TID_MSDUQ_QDEPTH_THRESH_UPDATE_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed", __func__);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_vap_dscp_tid_map_cmd_tlv() - send vap dscp tid map cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vap dscp tid map param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_vap_dscp_tid_map_cmd_tlv(wmi_unified_t wmi_handle,
				  struct vap_dscp_tid_map_params *param)
{
	wmi_buf_t buf;
	wmi_vdev_set_dscp_tid_map_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_vdev_set_dscp_tid_map_cmd_fixed_param *)wmi_buf_data(buf);
	qdf_mem_copy(cmd->dscp_to_tid_map, param->dscp_to_tid_map,
		     sizeof(uint32_t) * WMI_DSCP_MAP_MAX);

	cmd->vdev_id = param->vdev_id;
	cmd->enable_override = 0;

	WMI_LOGI("Setting dscp for vap id: %d", cmd->vdev_id);
	wmi_mtrace(WMI_VDEV_SET_DSCP_TID_MAP_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_DSCP_TID_MAP_CMDID)) {
			WMI_LOGE("Failed to set dscp cmd");
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_vdev_set_fwtest_param_cmd_tlv() - send fwtest param in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold fwtest param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_fwtest_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct set_fwtest_params *param)
{
	wmi_fwtest_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_fwtest_set_param_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_fwtest_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_fwtest_set_param_cmd_fixed_param));
	cmd->param_id = param->arg;
	cmd->param_value = param->value;

	wmi_mtrace(WMI_FWTEST_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_FWTEST_CMDID)) {
		WMI_LOGE("Setting FW test param failed");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_phyerr_disable_cmd_tlv() - WMI phyerr disable function
 *
 *  @param wmi_handle     : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_phyerr_disable_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_pdev_dfs_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_dfs_disable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_dfs_disable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_dfs_disable_cmd_fixed_param));
	/* Filling it with WMI_PDEV_ID_SOC for now */
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);

	wmi_mtrace(WMI_PDEV_DFS_DISABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_PDEV_DFS_DISABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending PDEV DFS disable cmd failed");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_phyerr_enable_cmd_tlv() - WMI phyerr disable function
 *
 *  @param wmi_handle     : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_phyerr_enable_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_pdev_dfs_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_dfs_enable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_dfs_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_dfs_enable_cmd_fixed_param));
	/* Reserved for future use */
	cmd->reserved0 = 0;

	wmi_mtrace(WMI_PDEV_DFS_ENABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_PDEV_DFS_ENABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending PDEV DFS enable cmd failed");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_periodic_chan_stats_config_cmd_tlv() - send periodic chan stats cmd
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold periodic chan stats param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_periodic_chan_stats_config_cmd_tlv(wmi_unified_t wmi_handle,
				struct periodic_chan_stats_params *param)
{
	wmi_set_periodic_channel_stats_config_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_set_periodic_channel_stats_config_fixed_param *)
					wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_set_periodic_channel_stats_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_set_periodic_channel_stats_config_fixed_param));
	cmd->enable = param->enable;
	cmd->stats_period = param->stats_period;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
						param->pdev_id);

	wmi_mtrace(WMI_SET_PERIODIC_CHANNEL_STATS_CONFIG_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_SET_PERIODIC_CHANNEL_STATS_CONFIG_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending periodic chan stats config failed");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_vdev_spectral_configure_cmd_tlv() - send VDEV spectral configure
 * command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold spectral config parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_spectral_configure_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_spectral_configure_params *param)
{
	wmi_vdev_spectral_configure_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_vdev_spectral_configure_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_spectral_configure_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_vdev_spectral_configure_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->spectral_scan_count = param->count;
	cmd->spectral_scan_period = param->period;
	cmd->spectral_scan_priority = param->spectral_pri;
	cmd->spectral_scan_fft_size = param->fft_size;
	cmd->spectral_scan_gc_ena = param->gc_enable;
	cmd->spectral_scan_restart_ena = param->restart_enable;
	cmd->spectral_scan_noise_floor_ref = param->noise_floor_ref;
	cmd->spectral_scan_init_delay = param->init_delay;
	cmd->spectral_scan_nb_tone_thr = param->nb_tone_thr;
	cmd->spectral_scan_str_bin_thr = param->str_bin_thr;
	cmd->spectral_scan_wb_rpt_mode = param->wb_rpt_mode;
	cmd->spectral_scan_rssi_rpt_mode = param->rssi_rpt_mode;
	cmd->spectral_scan_rssi_thr = param->rssi_thr;
	cmd->spectral_scan_pwr_format = param->pwr_format;
	cmd->spectral_scan_rpt_mode = param->rpt_mode;
	cmd->spectral_scan_bin_scale = param->bin_scale;
	cmd->spectral_scan_dBm_adj = param->dbm_adj;
	cmd->spectral_scan_chn_mask = param->chn_mask;

	wmi_mtrace(WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending set quiet cmd failed");
		wmi_buf_free(buf);
	}

	WMI_LOGI("%s: Sent WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID",
		 __func__);

	WMI_LOGI("vdev_id = %u\n"
		 "spectral_scan_count = %u\n"
		 "spectral_scan_period = %u\n"
		 "spectral_scan_priority = %u\n"
		 "spectral_scan_fft_size = %u\n"
		 "spectral_scan_gc_ena = %u\n"
		 "spectral_scan_restart_ena = %u\n"
		 "spectral_scan_noise_floor_ref = %u\n"
		 "spectral_scan_init_delay = %u\n"
		 "spectral_scan_nb_tone_thr = %u\n"
		 "spectral_scan_str_bin_thr = %u\n"
		 "spectral_scan_wb_rpt_mode = %u\n"
		 "spectral_scan_rssi_rpt_mode = %u\n"
		 "spectral_scan_rssi_thr = %u\n"
		 "spectral_scan_pwr_format = %u\n"
		 "spectral_scan_rpt_mode = %u\n"
		 "spectral_scan_bin_scale = %u\n"
		 "spectral_scan_dBm_adj = %u\n"
		 "spectral_scan_chn_mask = %u",
		 param->vdev_id,
		 param->count,
		 param->period,
		 param->spectral_pri,
		 param->fft_size,
		 param->gc_enable,
		 param->restart_enable,
		 param->noise_floor_ref,
		 param->init_delay,
		 param->nb_tone_thr,
		 param->str_bin_thr,
		 param->wb_rpt_mode,
		 param->rssi_rpt_mode,
		 param->rssi_thr,
		 param->pwr_format,
		 param->rpt_mode,
		 param->bin_scale,
		 param->dbm_adj,
		 param->chn_mask);
	WMI_LOGI("%s: Status: %d", __func__, ret);

	return ret;
}

/**
 * send_vdev_spectral_enable_cmd_tlv() - send VDEV spectral configure
 * command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold spectral enable parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_spectral_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_spectral_enable_params *param)
{
	wmi_vdev_spectral_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_vdev_spectral_enable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_spectral_enable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_vdev_spectral_enable_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	if (param->active_valid) {
		cmd->trigger_cmd = param->active ? 1 : 2;
		/* 1: Trigger, 2: Clear Trigger */
	} else {
		cmd->trigger_cmd = 0; /* 0: Ignore */
	}

	if (param->enabled_valid) {
		cmd->enable_cmd = param->enabled ? 1 : 2;
		/* 1: Enable 2: Disable */
	} else {
		cmd->enable_cmd = 0; /* 0: Ignore */
	}

	WMI_LOGI("vdev_id = %u\n"
				 "trigger_cmd = %u\n"
				 "enable_cmd = %u",
				 cmd->vdev_id,
				 cmd->trigger_cmd,
				 cmd->enable_cmd);

	wmi_mtrace(WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending scan enable CMD failed");
		wmi_buf_free(buf);
	}

	WMI_LOGI("%s: Sent WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID", __func__);

	WMI_LOGI("%s: Status: %d", __func__, ret);

	return ret;
}

/**
 * send_thermal_mitigation_param_cmd_tlv() - configure thermal mitigation params
 * @param wmi_handle : handle to WMI.
 * @param param : pointer to hold thermal mitigation param
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_thermal_mitigation_param_cmd_tlv(
		wmi_unified_t wmi_handle,
		struct thermal_mitigation_params *param)
{
	wmi_therm_throt_config_request_fixed_param *tt_conf = NULL;
	wmi_therm_throt_level_config_info *lvl_conf = NULL;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr = NULL;
	int error;
	int32_t len;
	int i;

	len = sizeof(*tt_conf) + WMI_TLV_HDR_SIZE +
			THERMAL_LEVELS * sizeof(wmi_therm_throt_level_config_info);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	tt_conf = (wmi_therm_throt_config_request_fixed_param *) wmi_buf_data(buf);

	/* init fixed params */
	WMITLV_SET_HDR(tt_conf,
		WMITLV_TAG_STRUC_wmi_therm_throt_config_request_fixed_param,
		(WMITLV_GET_STRUCT_TLVLEN(wmi_therm_throt_config_request_fixed_param)));

	tt_conf->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	tt_conf->enable = param->enable;
	tt_conf->dc = param->dc;
	tt_conf->dc_per_event = param->dc_per_event;
	tt_conf->therm_throt_levels = THERMAL_LEVELS;

	buf_ptr = (uint8_t *) ++tt_conf;
	/* init TLV params */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			(THERMAL_LEVELS * sizeof(wmi_therm_throt_level_config_info)));

	lvl_conf = (wmi_therm_throt_level_config_info *) (buf_ptr +  WMI_TLV_HDR_SIZE);
	for (i = 0; i < THERMAL_LEVELS; i++) {
		WMITLV_SET_HDR(&lvl_conf->tlv_header,
			WMITLV_TAG_STRUC_wmi_therm_throt_level_config_info,
			WMITLV_GET_STRUCT_TLVLEN(wmi_therm_throt_level_config_info));
		lvl_conf->temp_lwm = param->levelconf[i].tmplwm;
		lvl_conf->temp_hwm = param->levelconf[i].tmphwm;
		lvl_conf->dc_off_percent = param->levelconf[i].dcoffpercent;
		lvl_conf->prio = param->levelconf[i].priority;
		lvl_conf++;
	}

	wmi_mtrace(WMI_THERM_THROT_SET_CONF_CMDID, NO_SESSION, 0);
	error = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_THERM_THROT_SET_CONF_CMDID);
	if (QDF_IS_STATUS_ERROR(error)) {
		wmi_buf_free(buf);
		WMI_LOGE("Failed to send WMI_THERM_THROT_SET_CONF_CMDID command");
	}

	return error;
}

/**
 * send_coex_config_cmd_tlv() - send coex config command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to coex config param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_coex_config_cmd_tlv(wmi_unified_t wmi_handle,
			 struct coex_config_params *param)
{
	WMI_COEX_CONFIG_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (WMI_COEX_CONFIG_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_COEX_CONFIG_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       WMI_COEX_CONFIG_CMD_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->config_type = param->config_type;
	cmd->config_arg1 = param->config_arg1;
	cmd->config_arg2 = param->config_arg2;
	cmd->config_arg3 = param->config_arg3;
	cmd->config_arg4 = param->config_arg4;
	cmd->config_arg5 = param->config_arg5;
	cmd->config_arg6 = param->config_arg6;

	wmi_mtrace(WMI_COEX_CONFIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_COEX_CONFIG_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending COEX CONFIG CMD failed");
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef WLAN_SUPPORT_TWT
static void wmi_copy_twt_resource_config(wmi_resource_config *resource_cfg,
					target_resource_config *tgt_res_cfg)
{
	resource_cfg->twt_ap_pdev_count = tgt_res_cfg->twt_ap_pdev_count;
	resource_cfg->twt_ap_sta_count = tgt_res_cfg->twt_ap_sta_count;
}
#else
static void wmi_copy_twt_resource_config(wmi_resource_config *resource_cfg,
					target_resource_config *tgt_res_cfg)
{
	resource_cfg->twt_ap_pdev_count = 0;
	resource_cfg->twt_ap_sta_count = 0;
}
#endif

static
void wmi_copy_resource_config(wmi_resource_config *resource_cfg,
				target_resource_config *tgt_res_cfg)
{
	resource_cfg->num_vdevs = tgt_res_cfg->num_vdevs;
	resource_cfg->num_peers = tgt_res_cfg->num_peers;
	resource_cfg->num_offload_peers = tgt_res_cfg->num_offload_peers;
	resource_cfg->num_offload_reorder_buffs =
			tgt_res_cfg->num_offload_reorder_buffs;
	resource_cfg->num_peer_keys = tgt_res_cfg->num_peer_keys;
	resource_cfg->num_tids = tgt_res_cfg->num_tids;
	resource_cfg->ast_skid_limit = tgt_res_cfg->ast_skid_limit;
	resource_cfg->tx_chain_mask = tgt_res_cfg->tx_chain_mask;
	resource_cfg->rx_chain_mask = tgt_res_cfg->rx_chain_mask;
	resource_cfg->rx_timeout_pri[0] = tgt_res_cfg->rx_timeout_pri[0];
	resource_cfg->rx_timeout_pri[1] = tgt_res_cfg->rx_timeout_pri[1];
	resource_cfg->rx_timeout_pri[2] = tgt_res_cfg->rx_timeout_pri[2];
	resource_cfg->rx_timeout_pri[3] = tgt_res_cfg->rx_timeout_pri[3];
	resource_cfg->rx_decap_mode = tgt_res_cfg->rx_decap_mode;
	resource_cfg->scan_max_pending_req =
			tgt_res_cfg->scan_max_pending_req;
	resource_cfg->bmiss_offload_max_vdev =
			tgt_res_cfg->bmiss_offload_max_vdev;
	resource_cfg->roam_offload_max_vdev =
			tgt_res_cfg->roam_offload_max_vdev;
	resource_cfg->roam_offload_max_ap_profiles =
			tgt_res_cfg->roam_offload_max_ap_profiles;
	resource_cfg->num_mcast_groups = tgt_res_cfg->num_mcast_groups;
	resource_cfg->num_mcast_table_elems =
			tgt_res_cfg->num_mcast_table_elems;
	resource_cfg->mcast2ucast_mode = tgt_res_cfg->mcast2ucast_mode;
	resource_cfg->tx_dbg_log_size = tgt_res_cfg->tx_dbg_log_size;
	resource_cfg->num_wds_entries = tgt_res_cfg->num_wds_entries;
	resource_cfg->dma_burst_size = tgt_res_cfg->dma_burst_size;
	resource_cfg->mac_aggr_delim = tgt_res_cfg->mac_aggr_delim;
	resource_cfg->rx_skip_defrag_timeout_dup_detection_check =
		tgt_res_cfg->rx_skip_defrag_timeout_dup_detection_check;
	resource_cfg->vow_config = tgt_res_cfg->vow_config;
	resource_cfg->gtk_offload_max_vdev = tgt_res_cfg->gtk_offload_max_vdev;
	resource_cfg->num_msdu_desc = tgt_res_cfg->num_msdu_desc;
	resource_cfg->max_frag_entries = tgt_res_cfg->max_frag_entries;
	resource_cfg->num_tdls_vdevs = tgt_res_cfg->num_tdls_vdevs;
	resource_cfg->num_tdls_conn_table_entries =
			tgt_res_cfg->num_tdls_conn_table_entries;
	resource_cfg->beacon_tx_offload_max_vdev =
			tgt_res_cfg->beacon_tx_offload_max_vdev;
	resource_cfg->num_multicast_filter_entries =
			tgt_res_cfg->num_multicast_filter_entries;
	resource_cfg->num_wow_filters =
			tgt_res_cfg->num_wow_filters;
	resource_cfg->num_keep_alive_pattern =
			tgt_res_cfg->num_keep_alive_pattern;
	resource_cfg->keep_alive_pattern_size =
			tgt_res_cfg->keep_alive_pattern_size;
	resource_cfg->max_tdls_concurrent_sleep_sta =
			tgt_res_cfg->max_tdls_concurrent_sleep_sta;
	resource_cfg->max_tdls_concurrent_buffer_sta =
			tgt_res_cfg->max_tdls_concurrent_buffer_sta;
	resource_cfg->wmi_send_separate =
			tgt_res_cfg->wmi_send_separate;
	resource_cfg->num_ocb_vdevs =
			tgt_res_cfg->num_ocb_vdevs;
	resource_cfg->num_ocb_channels =
			tgt_res_cfg->num_ocb_channels;
	resource_cfg->num_ocb_schedules =
			tgt_res_cfg->num_ocb_schedules;
	resource_cfg->bpf_instruction_size = tgt_res_cfg->apf_instruction_size;
	resource_cfg->max_bssid_rx_filters = tgt_res_cfg->max_bssid_rx_filters;
	resource_cfg->use_pdev_id = tgt_res_cfg->use_pdev_id;
	resource_cfg->max_num_dbs_scan_duty_cycle =
		tgt_res_cfg->max_num_dbs_scan_duty_cycle;
	resource_cfg->sched_params = tgt_res_cfg->scheduler_params;
	resource_cfg->num_packet_filters = tgt_res_cfg->num_packet_filters;
	resource_cfg->num_max_sta_vdevs = tgt_res_cfg->num_max_sta_vdevs;
	resource_cfg->max_bssid_indicator = tgt_res_cfg->max_bssid_indicator;
	if (tgt_res_cfg->atf_config)
		WMI_RSRC_CFG_FLAG_ATF_CONFIG_ENABLE_SET(resource_cfg->flag1, 1);
	if (tgt_res_cfg->mgmt_comp_evt_bundle_support)
		WMI_RSRC_CFG_FLAG_MGMT_COMP_EVT_BUNDLE_SUPPORT_SET(
			resource_cfg->flag1, 1);
	if (tgt_res_cfg->tx_msdu_new_partition_id_support)
		WMI_RSRC_CFG_FLAG_TX_MSDU_ID_NEW_PARTITION_SUPPORT_SET(
			resource_cfg->flag1, 1);
	if (tgt_res_cfg->cce_disable)
		WMI_RSRC_CFG_FLAG_TCL_CCE_DISABLE_SET(resource_cfg->flag1, 1);
	if (tgt_res_cfg->eapol_minrate_set) {
		WMI_RSRC_CFG_FLAG_EAPOL_REKEY_MINRATE_SUPPORT_ENABLE_SET(
			resource_cfg->flag1, 1);
		if (tgt_res_cfg->eapol_minrate_ac_set != 3) {
			WMI_RSRC_CFG_FLAG_EAPOL_AC_OVERRIDE_VALID_SET(
				resource_cfg->flag1, 1);
			WMI_RSRC_CFG_FLAG_EAPOL_AC_OVERRIDE_SET(
				resource_cfg->flag1,
				tgt_res_cfg->eapol_minrate_ac_set);
		}
	}
	if (tgt_res_cfg->new_htt_msg_format) {
		WMI_RSRC_CFG_FLAG_HTT_H2T_NO_HTC_HDR_LEN_IN_MSG_LEN_SET(
			resource_cfg->flag1, 1);
	}

	if (tgt_res_cfg->peer_unmap_conf_support)
		WMI_RSRC_CFG_FLAG_PEER_UNMAP_RESPONSE_SUPPORT_SET(
			resource_cfg->flag1, 1);

	if (tgt_res_cfg->tstamp64_en)
		WMI_RSRC_CFG_FLAG_TX_COMPLETION_TX_TSF64_ENABLE_SET(
						resource_cfg->flag1, 1);

	wmi_copy_twt_resource_config(resource_cfg, tgt_res_cfg);
	resource_cfg->peer_map_unmap_v2_support =
		tgt_res_cfg->peer_map_unmap_v2;
}

/* copy_hw_mode_id_in_init_cmd() - Helper routine to copy hw_mode in init cmd
 * @wmi_handle: pointer to wmi handle
 * @buf_ptr: pointer to current position in init command buffer
 * @len: pointer to length. This will be updated with current length of cmd
 * @param: point host parameters for init command
 *
 * Return: Updated pointer of buf_ptr.
 */
static inline uint8_t *copy_hw_mode_in_init_cmd(struct wmi_unified *wmi_handle,
		uint8_t *buf_ptr, int *len, struct wmi_init_cmd_param *param)
{
	uint16_t idx;

	if (param->hw_mode_id != WMI_HOST_HW_MODE_MAX) {
		wmi_pdev_set_hw_mode_cmd_fixed_param *hw_mode;
		wmi_pdev_band_to_mac *band_to_mac;

		hw_mode = (wmi_pdev_set_hw_mode_cmd_fixed_param *)
			(buf_ptr + sizeof(wmi_init_cmd_fixed_param) +
			 sizeof(wmi_resource_config) +
			 WMI_TLV_HDR_SIZE + (param->num_mem_chunks *
				 sizeof(wlan_host_memory_chunk)));

		WMITLV_SET_HDR(&hw_mode->tlv_header,
			WMITLV_TAG_STRUC_wmi_pdev_set_hw_mode_cmd_fixed_param,
			(WMITLV_GET_STRUCT_TLVLEN
			 (wmi_pdev_set_hw_mode_cmd_fixed_param)));

		hw_mode->hw_mode_index = param->hw_mode_id;
		hw_mode->num_band_to_mac = param->num_band_to_mac;

		buf_ptr = (uint8_t *) (hw_mode + 1);
		band_to_mac = (wmi_pdev_band_to_mac *) (buf_ptr +
				WMI_TLV_HDR_SIZE);
		for (idx = 0; idx < param->num_band_to_mac; idx++) {
			WMITLV_SET_HDR(&band_to_mac[idx].tlv_header,
					WMITLV_TAG_STRUC_wmi_pdev_band_to_mac,
					WMITLV_GET_STRUCT_TLVLEN
					(wmi_pdev_band_to_mac));
			band_to_mac[idx].pdev_id =
				wmi_handle->ops->convert_pdev_id_host_to_target(
					param->band_to_mac[idx].pdev_id);
			band_to_mac[idx].start_freq =
				param->band_to_mac[idx].start_freq;
			band_to_mac[idx].end_freq =
				param->band_to_mac[idx].end_freq;
		}
		*len += sizeof(wmi_pdev_set_hw_mode_cmd_fixed_param) +
			(param->num_band_to_mac *
			 sizeof(wmi_pdev_band_to_mac)) +
			WMI_TLV_HDR_SIZE;

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
				(param->num_band_to_mac *
				 sizeof(wmi_pdev_band_to_mac)));
	}

	return buf_ptr;
}

static inline void copy_fw_abi_version_tlv(wmi_unified_t wmi_handle,
		wmi_init_cmd_fixed_param *cmd)
{
	int num_whitelist;
	wmi_abi_version my_vers;

	num_whitelist = sizeof(version_whitelist) /
		sizeof(wmi_whitelist_version_info);
	my_vers.abi_version_0 = WMI_ABI_VERSION_0;
	my_vers.abi_version_1 = WMI_ABI_VERSION_1;
	my_vers.abi_version_ns_0 = WMI_ABI_VERSION_NS_0;
	my_vers.abi_version_ns_1 = WMI_ABI_VERSION_NS_1;
	my_vers.abi_version_ns_2 = WMI_ABI_VERSION_NS_2;
	my_vers.abi_version_ns_3 = WMI_ABI_VERSION_NS_3;

	wmi_cmp_and_set_abi_version(num_whitelist, version_whitelist,
			&my_vers,
			(struct _wmi_abi_version *)&wmi_handle->fw_abi_version,
			&cmd->host_abi_vers);

	qdf_print("%s: INIT_CMD version: %d, %d, 0x%x, 0x%x, 0x%x, 0x%x",
			__func__,
			WMI_VER_GET_MAJOR(cmd->host_abi_vers.abi_version_0),
			WMI_VER_GET_MINOR(cmd->host_abi_vers.abi_version_0),
			cmd->host_abi_vers.abi_version_ns_0,
			cmd->host_abi_vers.abi_version_ns_1,
			cmd->host_abi_vers.abi_version_ns_2,
			cmd->host_abi_vers.abi_version_ns_3);

	/* Save version sent from host -
	 * Will be used to check ready event
	 */
	qdf_mem_copy(&wmi_handle->final_abi_vers, &cmd->host_abi_vers,
			sizeof(wmi_abi_version));
}

/*
 * send_cfg_action_frm_tb_ppdu_cmd_tlv() - send action frame tb ppdu cfg to FW
 * @wmi_handle:    Pointer to WMi handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends action frame tb ppdu cfg to FW
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS send_cfg_action_frm_tb_ppdu_cmd_tlv(wmi_unified_t wmi_handle,
				struct cfg_action_frm_tb_ppdu_param *cfg_msg)
{
	wmi_pdev_he_tb_action_frm_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len, frm_len_aligned;
	QDF_STATUS ret;

	frm_len_aligned = roundup(cfg_msg->frm_len, sizeof(uint32_t));
	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + frm_len_aligned;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_pdev_he_tb_action_frm_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_he_tb_action_frm_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_he_tb_action_frm_cmd_fixed_param));
	cmd->enable = cfg_msg->cfg;
	cmd->data_len = cfg_msg->frm_len;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, frm_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;

	qdf_mem_copy(buf_ptr, cfg_msg->data, cmd->data_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_HE_TB_ACTION_FRM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("HE TB action frame cmnd send fail, ret %d"), ret);
		wmi_buf_free(buf);
	}

	return ret;
}

static QDF_STATUS save_fw_version_cmd_tlv(wmi_unified_t wmi_handle, void *evt_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;


	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_FAILURE;

	/*Save fw version from service ready message */
	/*This will be used while sending INIT message */
	qdf_mem_copy(&wmi_handle->fw_abi_version, &ev->fw_abi_vers,
			sizeof(wmi_handle->fw_abi_version));

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_unified_save_fw_version_cmd() - save fw version
 * @wmi_handle:      pointer to wmi handle
 * @res_cfg:	 resource config
 * @num_mem_chunks:  no of mem chunck
 * @mem_chunk:       pointer to mem chunck structure
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS check_and_update_fw_version_cmd_tlv(wmi_unified_t wmi_handle,
					  void *evt_buf)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;

	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;
	if (!wmi_versions_are_compatible((struct _wmi_abi_version *)
				&wmi_handle->final_abi_vers,
				&ev->fw_abi_vers)) {
		/*
		 * Error: Our host version and the given firmware version
		 * are incompatible.
		 **/
		WMI_LOGD("%s: Error: Incompatible WMI version."
			"Host: %d,%d,0x%x 0x%x 0x%x 0x%x, FW: %d,%d,0x%x 0x%x 0x%x 0x%x",
				__func__,
			WMI_VER_GET_MAJOR(wmi_handle->final_abi_vers.
				abi_version_0),
			WMI_VER_GET_MINOR(wmi_handle->final_abi_vers.
				abi_version_0),
			wmi_handle->final_abi_vers.abi_version_ns_0,
			wmi_handle->final_abi_vers.abi_version_ns_1,
			wmi_handle->final_abi_vers.abi_version_ns_2,
			wmi_handle->final_abi_vers.abi_version_ns_3,
			WMI_VER_GET_MAJOR(ev->fw_abi_vers.abi_version_0),
			WMI_VER_GET_MINOR(ev->fw_abi_vers.abi_version_0),
			ev->fw_abi_vers.abi_version_ns_0,
			ev->fw_abi_vers.abi_version_ns_1,
			ev->fw_abi_vers.abi_version_ns_2,
			ev->fw_abi_vers.abi_version_ns_3);

		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(&wmi_handle->final_abi_vers, &ev->fw_abi_vers,
			sizeof(wmi_abi_version));
	qdf_mem_copy(&wmi_handle->fw_abi_version, &ev->fw_abi_vers,
			sizeof(wmi_abi_version));

	return QDF_STATUS_SUCCESS;
}

/**
 * send_log_supported_evt_cmd_tlv() - Enable/Disable FW diag/log events
 * @handle: wmi handle
 * @event:  Event received from FW
 * @len:    Length of the event
 *
 * Enables the low frequency events and disables the high frequency
 * events. Bit 17 indicates if the event if low/high frequency.
 * 1 - high frequency, 0 - low frequency
 *
 * Return: 0 on successfully enabling/disabling the events
 */
static QDF_STATUS send_log_supported_evt_cmd_tlv(wmi_unified_t wmi_handle,
		uint8_t *event,
		uint32_t len)
{
	uint32_t num_of_diag_events_logs;
	wmi_diag_event_log_config_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t *cmd_args, *evt_args;
	uint32_t buf_len, i;

	WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID_param_tlvs *param_buf;
	wmi_diag_event_log_supported_event_fixed_params *wmi_event;

	WMI_LOGI("Received WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID");

	param_buf = (WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMI_LOGE("Invalid log supported event buffer");
		return QDF_STATUS_E_INVAL;
	}
	wmi_event = param_buf->fixed_param;
	num_of_diag_events_logs = wmi_event->num_of_diag_events_logs;

	if (num_of_diag_events_logs >
	    param_buf->num_diag_events_logs_list) {
		WMI_LOGE("message number of events %d is more than tlv hdr content %d",
			 num_of_diag_events_logs,
			 param_buf->num_diag_events_logs_list);
		return QDF_STATUS_E_INVAL;
	}

	evt_args = param_buf->diag_events_logs_list;
	if (!evt_args) {
		WMI_LOGE("%s: Event list is empty, num_of_diag_events_logs=%d",
				__func__, num_of_diag_events_logs);
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("%s: num_of_diag_events_logs=%d",
			__func__, num_of_diag_events_logs);

	/* Free any previous allocation */
	if (wmi_handle->events_logs_list) {
		qdf_mem_free(wmi_handle->events_logs_list);
		wmi_handle->events_logs_list = NULL;
	}

	if (num_of_diag_events_logs >
		(WMI_SVC_MSG_MAX_SIZE / sizeof(uint32_t))) {
		WMI_LOGE("%s: excess num of logs:%d", __func__,
			num_of_diag_events_logs);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}
	/* Store the event list for run time enable/disable */
	wmi_handle->events_logs_list = qdf_mem_malloc(num_of_diag_events_logs *
			sizeof(uint32_t));
	if (!wmi_handle->events_logs_list)
		return QDF_STATUS_E_NOMEM;

	wmi_handle->num_of_diag_events_logs = num_of_diag_events_logs;

	/* Prepare the send buffer */
	buf_len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		(num_of_diag_events_logs * sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, buf_len);
	if (!buf) {
		qdf_mem_free(wmi_handle->events_logs_list);
		wmi_handle->events_logs_list = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_diag_event_log_config_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_diag_event_log_config_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_diag_event_log_config_fixed_param));

	cmd->num_of_diag_events_logs = num_of_diag_events_logs;

	buf_ptr += sizeof(wmi_diag_event_log_config_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(num_of_diag_events_logs * sizeof(uint32_t)));

	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);

	/* Populate the events */
	for (i = 0; i < num_of_diag_events_logs; i++) {
		/* Low freq (0) - Enable (1) the event
		 * High freq (1) - Disable (0) the event
		 */
		WMI_DIAG_ID_ENABLED_DISABLED_SET(cmd_args[i],
				!(WMI_DIAG_FREQUENCY_GET(evt_args[i])));
		/* Set the event ID */
		WMI_DIAG_ID_SET(cmd_args[i],
				WMI_DIAG_ID_GET(evt_args[i]));
		/* Set the type */
		WMI_DIAG_TYPE_SET(cmd_args[i],
				WMI_DIAG_TYPE_GET(evt_args[i]));
		/* Storing the event/log list in WMI */
		wmi_handle->events_logs_list[i] = evt_args[i];
	}

	wmi_mtrace(WMI_DIAG_EVENT_LOG_CONFIG_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, buf_len,
				WMI_DIAG_EVENT_LOG_CONFIG_CMDID)) {
		WMI_LOGE("%s: WMI_DIAG_EVENT_LOG_CONFIG_CMDID failed",
				__func__);
		wmi_buf_free(buf);
		/* Not clearing events_logs_list, though wmi cmd failed.
		 * Host can still have this list
		 */
		return QDF_STATUS_E_INVAL;
	}

	return 0;
}

/**
 * send_enable_specific_fw_logs_cmd_tlv() - Start/Stop logging of diag log id
 * @wmi_handle: wmi handle
 * @start_log: Start logging related parameters
 *
 * Send the command to the FW based on which specific logging of diag
 * event/log id can be started/stopped
 *
 * Return: None
 */
static QDF_STATUS send_enable_specific_fw_logs_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_wifi_start_log *start_log)
{
	wmi_diag_event_log_config_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len, count, log_level, i;
	uint32_t *cmd_args;
	uint32_t total_len;
	count = 0;

	if (!wmi_handle->events_logs_list) {
		WMI_LOGD("%s: Not received event/log list from FW, yet",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}
	/* total_len stores the number of events where BITS 17 and 18 are set.
	 * i.e., events of high frequency (17) and for extended debugging (18)
	 */
	total_len = 0;
	for (i = 0; i < wmi_handle->num_of_diag_events_logs; i++) {
		if ((WMI_DIAG_FREQUENCY_GET(wmi_handle->events_logs_list[i])) &&
		    (WMI_DIAG_EXT_FEATURE_GET(wmi_handle->events_logs_list[i])))
			total_len++;
	}

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		(total_len * sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_diag_event_log_config_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_diag_event_log_config_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_diag_event_log_config_fixed_param));

	cmd->num_of_diag_events_logs = total_len;

	buf_ptr += sizeof(wmi_diag_event_log_config_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(total_len * sizeof(uint32_t)));

	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);

	if (start_log->verbose_level >= WMI_LOG_LEVEL_ACTIVE)
		log_level = 1;
	else
		log_level = 0;

	WMI_LOGD("%s: Length:%d, Log_level:%d", __func__, total_len, log_level);
	for (i = 0; i < wmi_handle->num_of_diag_events_logs; i++) {
		uint32_t val = wmi_handle->events_logs_list[i];
		if ((WMI_DIAG_FREQUENCY_GET(val)) &&
				(WMI_DIAG_EXT_FEATURE_GET(val))) {

			WMI_DIAG_ID_SET(cmd_args[count],
					WMI_DIAG_ID_GET(val));
			WMI_DIAG_TYPE_SET(cmd_args[count],
					WMI_DIAG_TYPE_GET(val));
			WMI_DIAG_ID_ENABLED_DISABLED_SET(cmd_args[count],
					log_level);
			WMI_LOGD("%s: Idx:%d, val:%x", __func__, i, val);
			count++;
		}
	}

	wmi_mtrace(WMI_DIAG_EVENT_LOG_CONFIG_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_DIAG_EVENT_LOG_CONFIG_CMDID)) {
		WMI_LOGE("%s: WMI_DIAG_EVENT_LOG_CONFIG_CMDID failed",
				__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_flush_logs_to_fw_cmd_tlv() - Send log flush command to FW
 * @wmi_handle: WMI handle
 *
 * This function is used to send the flush command to the FW,
 * that will flush the fw logs that are residue in the FW
 *
 * Return: None
 */
static QDF_STATUS send_flush_logs_to_fw_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_debug_mesg_flush_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_debug_mesg_flush_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_debug_mesg_flush_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_debug_mesg_flush_fixed_param));
	cmd->reserved0 = 0;

	wmi_mtrace(WMI_DEBUG_MESG_FLUSH_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
			buf,
			len,
			WMI_DEBUG_MESG_FLUSH_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_DEBUG_MESG_FLUSH_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}
	WMI_LOGD("Sent WMI_DEBUG_MESG_FLUSH_CMDID to FW");

	return ret;
}

#ifdef BIG_ENDIAN_HOST
/**
* fips_conv_data_be() - LE to BE conversion of FIPS ev data
* @param data_len - data length
* @param data - pointer to data
*
* Return: QDF_STATUS - success or error status
*/
static QDF_STATUS fips_align_data_be(wmi_unified_t wmi_handle,
			struct fips_params *param)
{
	unsigned char *key_unaligned, *data_unaligned;
	int c;
	u_int8_t *key_aligned = NULL;
	u_int8_t *data_aligned = NULL;

	/* Assigning unaligned space to copy the key */
	key_unaligned = qdf_mem_malloc(
		sizeof(u_int8_t)*param->key_len + FIPS_ALIGN);
	data_unaligned = qdf_mem_malloc(
		sizeof(u_int8_t)*param->data_len + FIPS_ALIGN);

	/* Checking if kmalloc is successful to allocate space */
	if (!key_unaligned)
		return QDF_STATUS_SUCCESS;
	/* Checking if space is aligned */
	if (!FIPS_IS_ALIGNED(key_unaligned, FIPS_ALIGN)) {
		/* align to 4 */
		key_aligned =
		(u_int8_t *)FIPS_ALIGNTO(key_unaligned,
			FIPS_ALIGN);
	} else {
		key_aligned = (u_int8_t *)key_unaligned;
	}

	/* memset and copy content from key to key aligned */
	OS_MEMSET(key_aligned, 0, param->key_len);
	OS_MEMCPY(key_aligned, param->key, param->key_len);

	/* print a hexdump for host debug */
	print_hex_dump(KERN_DEBUG,
		"\t Aligned and Copied Key:@@@@ ",
		DUMP_PREFIX_NONE,
		16, 1, key_aligned, param->key_len, true);

	/* Checking if kmalloc is successful to allocate space */
	if (!data_unaligned)
		return QDF_STATUS_SUCCESS;
	/* Checking of space is aligned */
	if (!FIPS_IS_ALIGNED(data_unaligned, FIPS_ALIGN)) {
		/* align to 4 */
		data_aligned =
		(u_int8_t *)FIPS_ALIGNTO(data_unaligned,
				FIPS_ALIGN);
	} else {
		data_aligned = (u_int8_t *)data_unaligned;
	}

	/* memset and copy content from data to data aligned */
	OS_MEMSET(data_aligned, 0, param->data_len);
	OS_MEMCPY(data_aligned, param->data, param->data_len);

	/* print a hexdump for host debug */
	print_hex_dump(KERN_DEBUG,
		"\t Properly Aligned and Copied Data:@@@@ ",
	DUMP_PREFIX_NONE,
	16, 1, data_aligned, param->data_len, true);

	/* converting to little Endian both key_aligned and
	* data_aligned*/
	for (c = 0; c < param->key_len/4; c++) {
		*((u_int32_t *)key_aligned+c) =
		qdf_cpu_to_le32(*((u_int32_t *)key_aligned+c));
	}
	for (c = 0; c < param->data_len/4; c++) {
		*((u_int32_t *)data_aligned+c) =
		qdf_cpu_to_le32(*((u_int32_t *)data_aligned+c));
	}

	/* update endian data to key and data vectors */
	OS_MEMCPY(param->key, key_aligned, param->key_len);
	OS_MEMCPY(param->data, data_aligned, param->data_len);

	/* clean up allocated spaces */
	qdf_mem_free(key_unaligned);
	key_unaligned = NULL;
	key_aligned = NULL;

	qdf_mem_free(data_unaligned);
	data_unaligned = NULL;
	data_aligned = NULL;

	return QDF_STATUS_SUCCESS;
}
#else
/**
* fips_align_data_be() - DUMMY for LE platform
*
* Return: QDF_STATUS - success
*/
static QDF_STATUS fips_align_data_be(wmi_unified_t wmi_handle,
		struct fips_params *param)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * send_pdev_fips_cmd_tlv() - send pdev fips cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold pdev fips param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_fips_cmd_tlv(wmi_unified_t wmi_handle,
		struct fips_params *param)
{
	wmi_pdev_fips_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len = sizeof(wmi_pdev_fips_cmd_fixed_param);
	QDF_STATUS retval = QDF_STATUS_SUCCESS;

	/* Length TLV placeholder for array of bytes */
	len += WMI_TLV_HDR_SIZE;
	if (param->data_len)
		len += (param->data_len*sizeof(uint8_t));

	/*
	* Data length must be multiples of 16 bytes - checked against 0xF -
	* and must be less than WMI_SVC_MSG_SIZE - static size of
	* wmi_pdev_fips_cmd structure
	*/

	/* do sanity on the input */
	if (!(((param->data_len & 0xF) == 0) &&
			((param->data_len > 0) &&
			(param->data_len < (WMI_HOST_MAX_BUFFER_SIZE -
		sizeof(wmi_pdev_fips_cmd_fixed_param)))))) {
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_pdev_fips_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_fips_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_fips_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	if (param->key && param->data) {
		cmd->key_len = param->key_len;
		cmd->data_len = param->data_len;
		cmd->fips_cmd = !!(param->op);

		if (fips_align_data_be(wmi_handle, param) != QDF_STATUS_SUCCESS)
			return QDF_STATUS_E_FAILURE;

		qdf_mem_copy(cmd->key, param->key, param->key_len);

		if (param->mode == FIPS_ENGINE_AES_CTR ||
			param->mode == FIPS_ENGINE_AES_MIC) {
			cmd->mode = param->mode;
		} else {
			cmd->mode = FIPS_ENGINE_AES_CTR;
		}
		qdf_print("Key len = %d, Data len = %d",
			  cmd->key_len, cmd->data_len);

		print_hex_dump(KERN_DEBUG, "Key: ", DUMP_PREFIX_NONE, 16, 1,
				cmd->key, cmd->key_len, true);
		buf_ptr += sizeof(*cmd);

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, param->data_len);

		buf_ptr += WMI_TLV_HDR_SIZE;
		if (param->data_len)
			qdf_mem_copy(buf_ptr,
				(uint8_t *) param->data, param->data_len);

		print_hex_dump(KERN_DEBUG, "Plain text: ", DUMP_PREFIX_NONE,
			16, 1, buf_ptr, cmd->data_len, true);

		buf_ptr += param->data_len;

		wmi_mtrace(WMI_PDEV_FIPS_CMDID, NO_SESSION, 0);
		retval = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_FIPS_CMDID);
		qdf_print("%s return value %d", __func__, retval);
	} else {
		qdf_print("\n%s:%d Key or Data is NULL", __func__, __LINE__);
		wmi_buf_free(buf);
		retval = -QDF_STATUS_E_BADMSG;
	}

	return retval;
}

/**
 * send_fw_test_cmd_tlv() - send fw test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_fwtest: fw test command
 *
 * This function sends fw test command to fw.
 *
 * Return: CDF STATUS
 */
static
QDF_STATUS send_fw_test_cmd_tlv(wmi_unified_t wmi_handle,
			       struct set_fwtest_params *wmi_fwtest)
{
	wmi_fwtest_set_param_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint16_t len;

	len = sizeof(*cmd);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_fwtest_set_param_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_fwtest_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_fwtest_set_param_cmd_fixed_param));
	cmd->param_id = wmi_fwtest->arg;
	cmd->param_value = wmi_fwtest->value;

	wmi_mtrace(WMI_FWTEST_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_FWTEST_CMDID)) {
		WMI_LOGP("%s: failed to send fw test command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_unit_test_cmd_tlv() - send unit test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_utest: unit test command
 *
 * This function send unit test command to fw.
 *
 * Return: CDF STATUS
 */
static QDF_STATUS send_unit_test_cmd_tlv(wmi_unified_t wmi_handle,
			       struct wmi_unit_test_cmd *wmi_utest)
{
	wmi_unit_test_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	int i;
	uint16_t len, args_tlv_len;
	uint32_t *unit_test_cmd_args;

	args_tlv_len =
		WMI_TLV_HDR_SIZE + wmi_utest->num_args * sizeof(uint32_t);
	len = sizeof(wmi_unit_test_cmd_fixed_param) + args_tlv_len;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_unit_test_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_unit_test_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_unit_test_cmd_fixed_param));
	cmd->vdev_id = wmi_utest->vdev_id;
	cmd->module_id = wmi_utest->module_id;
	cmd->num_args = wmi_utest->num_args;
	cmd->diag_token = wmi_utest->diag_token;
	buf_ptr += sizeof(wmi_unit_test_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (wmi_utest->num_args * sizeof(uint32_t)));
	unit_test_cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_LOGI("%s: VDEV ID: %d", __func__, cmd->vdev_id);
	WMI_LOGI("%s: MODULE ID: %d", __func__, cmd->module_id);
	WMI_LOGI("%s: TOKEN: %d", __func__, cmd->diag_token);
	WMI_LOGI("%s: %d num of args = ", __func__, wmi_utest->num_args);
	for (i = 0; (i < wmi_utest->num_args && i < WMI_UNIT_TEST_MAX_NUM_ARGS); i++) {
		unit_test_cmd_args[i] = wmi_utest->args[i];
		WMI_LOGI("%d,", wmi_utest->args[i]);
	}
	wmi_mtrace(WMI_UNIT_TEST_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_UNIT_TEST_CMDID)) {
		WMI_LOGP("%s: failed to send unit test command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_power_dbg_cmd_tlv() - send power debug commands
 * @wmi_handle: wmi handle
 * @param: wmi power debug parameter
 *
 * Send WMI_POWER_DEBUG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_power_dbg_cmd_tlv(wmi_unified_t wmi_handle,
					 struct wmi_power_dbg_params *param)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len, args_tlv_len;
	uint8_t *buf_ptr;
	uint8_t i;
	wmi_pdev_wal_power_debug_cmd_fixed_param *cmd;
	uint32_t *cmd_args;

	/* Prepare and send power debug cmd parameters */
	args_tlv_len = WMI_TLV_HDR_SIZE + param->num_args * sizeof(uint32_t);
	len = sizeof(*cmd) + args_tlv_len;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_pdev_wal_power_debug_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		  WMITLV_TAG_STRUC_wmi_pdev_wal_power_debug_cmd_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN
		  (wmi_pdev_wal_power_debug_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	cmd->module_id = param->module_id;
	cmd->num_args = param->num_args;
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (param->num_args * sizeof(uint32_t)));
	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_LOGI("%s: %d num of args = ", __func__, param->num_args);
	for (i = 0; (i < param->num_args && i < WMI_MAX_POWER_DBG_ARGS); i++) {
		cmd_args[i] = param->args[i];
		WMI_LOGI("%d,", param->args[i]);
	}

	wmi_mtrace(WMI_PDEV_WAL_POWER_DEBUG_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_PDEV_WAL_POWER_DEBUG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_PDEV_WAL_POWER_DEBUG_CMDID returned Error %d",
			status);
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/**
 * send_dfs_phyerr_offload_en_cmd_tlv() - send dfs phyerr offload enable cmd
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Send WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_dfs_phyerr_offload_en_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t pdev_id)
{
	wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	WMI_LOGI("%s: pdev_id=%d", __func__, pdev_id);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);
	wmi_mtrace(WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d, pdev_id=%d",
			__func__, ret, pdev_id);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_dfs_phyerr_offload_dis_cmd_tlv() - send dfs phyerr offload disable cmd
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Send WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_dfs_phyerr_offload_dis_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t pdev_id)
{
	wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	WMI_LOGI("%s: pdev_id=%d", __func__, pdev_id);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);
	wmi_mtrace(WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d, pdev_id=%d",
			__func__, ret, pdev_id);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_SUPPORT_AGILE_DFS
static
QDF_STATUS send_adfs_ch_cfg_cmd_tlv(wmi_unified_t wmi_handle,
				    struct vdev_adfs_ch_cfg_params *param)
{
	/* wmi_unified_cmd_send set request of agile ADFS channel*/
	wmi_vdev_adfs_ch_cfg_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	uint16_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_adfs_ch_cfg_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_adfs_ch_cfg_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_vdev_adfs_ch_cfg_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->ocac_mode = param->ocac_mode;
	cmd->center_freq = param->center_freq;
	cmd->chan_freq = param->chan_freq;
	cmd->chan_width = param->chan_width;
	cmd->min_duration_ms = param->min_duration_ms;
	cmd->max_duration_ms = param->max_duration_ms;
	WMI_LOGD("%s:cmd->vdev_id: %d ,cmd->ocac_mode: %d cmd->center_freq: %d",
		 __func__, cmd->vdev_id, cmd->ocac_mode,
		 cmd->center_freq);

	wmi_mtrace(WMI_VDEV_ADFS_CH_CFG_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_ADFS_CH_CFG_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d",
			 __func__, ret);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS send_adfs_ocac_abort_cmd_tlv(wmi_unified_t wmi_handle,
					struct vdev_adfs_abort_params *param)
{
	/*wmi_unified_cmd_send with ocac abort on ADFS channel*/
	wmi_vdev_adfs_ocac_abort_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	uint16_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_adfs_ocac_abort_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR
		(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_vdev_adfs_ocac_abort_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN
		 (wmi_vdev_adfs_ocac_abort_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	wmi_mtrace(WMI_VDEV_ADFS_OCAC_ABORT_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_ADFS_OCAC_ABORT_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d",
			 __func__, ret);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * init_cmd_send_tlv() - send initialization cmd to fw
 * @wmi_handle: wmi handle
 * @param param: pointer to wmi init param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS init_cmd_send_tlv(wmi_unified_t wmi_handle,
				struct wmi_init_cmd_param *param)
{
	wmi_buf_t buf;
	wmi_init_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	wmi_resource_config *resource_cfg;
	wlan_host_memory_chunk *host_mem_chunks;
	uint32_t mem_chunk_len = 0, hw_mode_len = 0;
	uint16_t idx;
	int len;
	QDF_STATUS ret;

	len = sizeof(*cmd) + sizeof(wmi_resource_config) +
		WMI_TLV_HDR_SIZE;
	mem_chunk_len = (sizeof(wlan_host_memory_chunk) * MAX_MEM_CHUNKS);

	if (param->hw_mode_id != WMI_HOST_HW_MODE_MAX)
		hw_mode_len = sizeof(wmi_pdev_set_hw_mode_cmd_fixed_param) +
			WMI_TLV_HDR_SIZE +
			(param->num_band_to_mac * sizeof(wmi_pdev_band_to_mac));

	buf = wmi_buf_alloc(wmi_handle, len + mem_chunk_len + hw_mode_len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_init_cmd_fixed_param *) buf_ptr;
	resource_cfg = (wmi_resource_config *) (buf_ptr + sizeof(*cmd));

	host_mem_chunks = (wlan_host_memory_chunk *)
		(buf_ptr + sizeof(*cmd) + sizeof(wmi_resource_config)
		 + WMI_TLV_HDR_SIZE);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_init_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(wmi_init_cmd_fixed_param));

	wmi_copy_resource_config(resource_cfg, param->res_cfg);
	WMITLV_SET_HDR(&resource_cfg->tlv_header,
			WMITLV_TAG_STRUC_wmi_resource_config,
			WMITLV_GET_STRUCT_TLVLEN(wmi_resource_config));

	for (idx = 0; idx < param->num_mem_chunks; ++idx) {
		WMITLV_SET_HDR(&(host_mem_chunks[idx].tlv_header),
				WMITLV_TAG_STRUC_wlan_host_memory_chunk,
				WMITLV_GET_STRUCT_TLVLEN
				(wlan_host_memory_chunk));
		host_mem_chunks[idx].ptr = param->mem_chunks[idx].paddr;
		host_mem_chunks[idx].size = param->mem_chunks[idx].len;
		host_mem_chunks[idx].req_id = param->mem_chunks[idx].req_id;
		QDF_TRACE(QDF_MODULE_ID_ANY, QDF_TRACE_LEVEL_DEBUG,
				"chunk %d len %d requested ,ptr  0x%x ",
				idx, host_mem_chunks[idx].size,
				host_mem_chunks[idx].ptr);
	}
	cmd->num_host_mem_chunks = param->num_mem_chunks;
	len += (param->num_mem_chunks * sizeof(wlan_host_memory_chunk));

	WMITLV_SET_HDR((buf_ptr + sizeof(*cmd) + sizeof(wmi_resource_config)),
			WMITLV_TAG_ARRAY_STRUC,
			(sizeof(wlan_host_memory_chunk) *
			 param->num_mem_chunks));

	/* Fill hw mode id config */
	buf_ptr = copy_hw_mode_in_init_cmd(wmi_handle, buf_ptr, &len, param);

	/* Fill fw_abi_vers */
	copy_fw_abi_version_tlv(wmi_handle, cmd);

	wmi_mtrace(WMI_INIT_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_INIT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_INIT_CMDID returned Error %d",
			ret);
		wmi_buf_free(buf);
	}

	return ret;

}

/**
 * send_addba_send_cmd_tlv() - send addba send command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to delba send params
 * @macaddr: peer mac address
 *
 * Send WMI_ADDBA_SEND_CMDID command to firmware
 * Return: QDF_STATUS_SUCCESS on success. QDF_STATUS_E** on error
 */
static QDF_STATUS
send_addba_send_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct addba_send_params *param)
{
	wmi_addba_send_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_addba_send_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_addba_send_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(wmi_addba_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->tid = param->tidno;
	cmd->buffersize = param->buffersize;

	wmi_mtrace(WMI_ADDBA_SEND_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_ADDBA_SEND_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d", __func__, ret);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_delba_send_cmd_tlv() - send delba send command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to delba send params
 * @macaddr: peer mac address
 *
 * Send WMI_DELBA_SEND_CMDID command to firmware
 * Return: QDF_STATUS_SUCCESS on success. QDF_STATUS_E** on error
 */
static QDF_STATUS
send_delba_send_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct delba_send_params *param)
{
	wmi_delba_send_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_delba_send_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_delba_send_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(wmi_delba_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->tid = param->tidno;
	cmd->initiator = param->initiator;
	cmd->reasoncode = param->reasoncode;

	wmi_mtrace(WMI_DELBA_SEND_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_DELBA_SEND_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d", __func__, ret);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_addba_clearresponse_cmd_tlv() - send addba clear response command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to addba clearresp params
 * @macaddr: peer mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_addba_clearresponse_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t macaddr[QDF_MAC_ADDR_SIZE],
			struct addba_clearresponse_params *param)
{
	wmi_addba_clear_resp_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_addba_clear_resp_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_addba_clear_resp_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_addba_clear_resp_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	wmi_mtrace(WMI_ADDBA_CLEAR_RESP_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				buf, len, WMI_ADDBA_CLEAR_RESP_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d", __func__, ret);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef OBSS_PD
/**
 * send_obss_spatial_reuse_set_def_thresh_cmd_tlv - send obss spatial reuse set
 * def thresh to fw
 * @wmi_handle: wmi handle
 * @thresh: pointer to obss_spatial_reuse_def_thresh
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static
QDF_STATUS send_obss_spatial_reuse_set_def_thresh_cmd_tlv(
			wmi_unified_t wmi_handle,
			struct wmi_host_obss_spatial_reuse_set_def_thresh
			*thresh)
{
	wmi_buf_t buf;
	wmi_obss_spatial_reuse_set_def_obss_thresh_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t cmd_len;
	uint32_t tlv_len;

	cmd_len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, cmd_len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_obss_spatial_reuse_set_def_obss_thresh_cmd_fixed_param *)
		wmi_buf_data(buf);

	tlv_len = WMITLV_GET_STRUCT_TLVLEN(
		wmi_obss_spatial_reuse_set_def_obss_thresh_cmd_fixed_param);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_obss_spatial_reuse_set_def_obss_thresh_cmd_fixed_param,
	tlv_len);

	cmd->obss_min = thresh->obss_min;
	cmd->obss_max = thresh->obss_max;
	cmd->vdev_type = thresh->vdev_type;
	ret = wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
		WMI_PDEV_OBSS_PD_SPATIAL_REUSE_SET_DEF_OBSS_THRESH_CMDID);
	if (QDF_IS_STATUS_ERROR(ret))
		wmi_buf_free(buf);

	return ret;
}

/**
 * send_obss_spatial_reuse_set_cmd_tlv - send obss spatial reuse set cmd to fw
 * @wmi_handle: wmi handle
 * @obss_spatial_reuse_param: pointer to obss_spatial_reuse_param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static
QDF_STATUS send_obss_spatial_reuse_set_cmd_tlv(wmi_unified_t wmi_handle,
			struct wmi_host_obss_spatial_reuse_set_param
			*obss_spatial_reuse_param)
{
	wmi_buf_t buf;
	wmi_obss_spatial_reuse_set_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_obss_spatial_reuse_set_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_obss_spatial_reuse_set_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_obss_spatial_reuse_set_cmd_fixed_param));

	cmd->enable = obss_spatial_reuse_param->enable;
	cmd->obss_min = obss_spatial_reuse_param->obss_min;
	cmd->obss_max = obss_spatial_reuse_param->obss_max;
	cmd->vdev_id = obss_spatial_reuse_param->vdev_id;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_OBSS_PD_SPATIAL_REUSE_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(
		 "WMI_PDEV_OBSS_PD_SPATIAL_REUSE_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}
#endif

#ifdef QCA_SUPPORT_CP_STATS
/**
 * extract_cca_stats_tlv - api to extract congestion stats from event buffer
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @out_buff: buffer to populated after stats extraction
 *
 * Return: status of operation
 */
static QDF_STATUS extract_cca_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_congestion_stats *out_buff)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_congestion_stats *congestion_stats;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;
	congestion_stats = param_buf->congestion_stats;
	if (!congestion_stats) {
		WMI_LOGD("%s: no cca stats in event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	out_buff->vdev_id = congestion_stats->vdev_id;
	out_buff->congestion = congestion_stats->congestion;

	WMI_LOGD("%s: cca stats event processed", __func__);
	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_SUPPORT_CP_STATS */

/**
 * extract_ctl_failsafe_check_ev_param_tlv() - extract ctl data from
 * event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold peer ctl data
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_ctl_failsafe_check_ev_param_tlv(
			wmi_unified_t wmi_handle,
			void *evt_buf,
			struct wmi_host_pdev_ctl_failsafe_event *param)
{
	WMI_PDEV_CTL_FAILSAFE_CHECK_EVENTID_param_tlvs *param_buf;
	wmi_pdev_ctl_failsafe_check_fixed_param *fix_param;

	param_buf = (WMI_PDEV_CTL_FAILSAFE_CHECK_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid ctl_failsafe event buffer");
		return QDF_STATUS_E_INVAL;
	}

	fix_param = param_buf->fixed_param;
	param->ctl_failsafe_status = fix_param->ctl_FailsafeStatus;

	return QDF_STATUS_SUCCESS;
}

/**
 * save_service_bitmap_tlv() - save service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param bitmap_buf: bitmap buffer, for converged legacy support
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS save_service_bitmap_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			     void *bitmap_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	struct wmi_soc *soc = wmi_handle->soc;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	/* If it is already allocated, use that buffer. This can happen
	 * during target stop/start scenarios where host allocation is skipped.
	 */
	if (!soc->wmi_service_bitmap) {
		soc->wmi_service_bitmap =
			qdf_mem_malloc(WMI_SERVICE_BM_SIZE * sizeof(uint32_t));
		if (!soc->wmi_service_bitmap)
			return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(soc->wmi_service_bitmap,
			param_buf->wmi_service_bitmap,
			(WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));

	if (bitmap_buf)
		qdf_mem_copy(bitmap_buf,
			     param_buf->wmi_service_bitmap,
			     (WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));

	return QDF_STATUS_SUCCESS;
}

/**
 * save_ext_service_bitmap_tlv() - save extendend service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param bitmap_buf: bitmap buffer, for converged legacy support
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS save_ext_service_bitmap_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			     void *bitmap_buf)
{
	WMI_SERVICE_AVAILABLE_EVENTID_param_tlvs *param_buf;
	wmi_service_available_event_fixed_param *ev;
	struct wmi_soc *soc = wmi_handle->soc;

	param_buf = (WMI_SERVICE_AVAILABLE_EVENTID_param_tlvs *) evt_buf;

	ev = param_buf->fixed_param;

	/* If it is already allocated, use that buffer. This can happen
	 * during target stop/start scenarios where host allocation is skipped.
	 */
	if (!soc->wmi_ext_service_bitmap) {
		soc->wmi_ext_service_bitmap = qdf_mem_malloc(
			WMI_SERVICE_SEGMENT_BM_SIZE32 * sizeof(uint32_t));
		if (!soc->wmi_ext_service_bitmap)
			return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(soc->wmi_ext_service_bitmap,
			ev->wmi_service_segment_bitmap,
			(WMI_SERVICE_SEGMENT_BM_SIZE32 * sizeof(uint32_t)));

	WMI_LOGD("wmi_ext_service_bitmap 0:0x%x, 1:0x%x, 2:0x%x, 3:0x%x",
			soc->wmi_ext_service_bitmap[0], soc->wmi_ext_service_bitmap[1],
			soc->wmi_ext_service_bitmap[2], soc->wmi_ext_service_bitmap[3]);

	if (bitmap_buf)
		qdf_mem_copy(bitmap_buf,
			soc->wmi_ext_service_bitmap,
			(WMI_SERVICE_SEGMENT_BM_SIZE32 * sizeof(uint32_t)));

	return QDF_STATUS_SUCCESS;
}
/**
 * is_service_enabled_tlv() - Check if service enabled
 * @param wmi_handle: wmi handle
 * @param service_id: service identifier
 *
 * Return: 1 enabled, 0 disabled
 */
static bool is_service_enabled_tlv(wmi_unified_t wmi_handle,
		uint32_t service_id)
{
	struct wmi_soc *soc = wmi_handle->soc;

	if (!soc->wmi_service_bitmap) {
		WMI_LOGE("WMI service bit map is not saved yet");
		return false;
	}

	/* if wmi_service_enabled was received with extended bitmap,
	 * use WMI_SERVICE_EXT_IS_ENABLED to check the services.
	 */
	if (soc->wmi_ext_service_bitmap)
		return WMI_SERVICE_EXT_IS_ENABLED(soc->wmi_service_bitmap,
				soc->wmi_ext_service_bitmap,
				service_id);

	if (service_id >= WMI_MAX_SERVICE) {
		WMI_LOGE("Service id %d but WMI ext service bitmap is NULL",
			 service_id);
		return false;
	}

	return WMI_SERVICE_IS_ENABLED(soc->wmi_service_bitmap,
				service_id);
}

static inline void copy_ht_cap_info(uint32_t ev_target_cap,
		struct wlan_psoc_target_capability_info *cap)
{
       /* except LDPC all flags are common betwen legacy and here
	*  also IBFEER is not defined for TLV
	*/
	cap->ht_cap_info |= ev_target_cap & (
					WMI_HT_CAP_ENABLED
					| WMI_HT_CAP_HT20_SGI
					| WMI_HT_CAP_DYNAMIC_SMPS
					| WMI_HT_CAP_TX_STBC
					| WMI_HT_CAP_TX_STBC_MASK_SHIFT
					| WMI_HT_CAP_RX_STBC
					| WMI_HT_CAP_RX_STBC_MASK_SHIFT
					| WMI_HT_CAP_LDPC
					| WMI_HT_CAP_L_SIG_TXOP_PROT
					| WMI_HT_CAP_MPDU_DENSITY
					| WMI_HT_CAP_MPDU_DENSITY_MASK_SHIFT
					| WMI_HT_CAP_HT40_SGI);
	if (ev_target_cap & WMI_HT_CAP_LDPC)
		cap->ht_cap_info |= WMI_HOST_HT_CAP_RX_LDPC |
			WMI_HOST_HT_CAP_TX_LDPC;
}
/**
 * extract_service_ready_tlv() - extract service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to received event buffer
 * @param cap: pointer to hold target capability information extracted from even
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_service_ready_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wlan_psoc_target_capability_info *cap)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;


	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cap->phy_capability = ev->phy_capability;
	cap->max_frag_entry = ev->max_frag_entry;
	cap->num_rf_chains = ev->num_rf_chains;
	copy_ht_cap_info(ev->ht_cap_info, cap);
	cap->vht_cap_info = ev->vht_cap_info;
	cap->vht_supp_mcs = ev->vht_supp_mcs;
	cap->hw_min_tx_power = ev->hw_min_tx_power;
	cap->hw_max_tx_power = ev->hw_max_tx_power;
	cap->sys_cap_info = ev->sys_cap_info;
	cap->min_pkt_size_enable = ev->min_pkt_size_enable;
	cap->max_bcn_ie_size = ev->max_bcn_ie_size;
	cap->max_num_scan_channels = ev->max_num_scan_channels;
	cap->max_supported_macs = ev->max_supported_macs;
	cap->wmi_fw_sub_feat_caps = ev->wmi_fw_sub_feat_caps;
	cap->txrx_chainmask = ev->txrx_chainmask;
	cap->default_dbs_hw_mode_index = ev->default_dbs_hw_mode_index;
	cap->num_msdu_desc = ev->num_msdu_desc;
	cap->fw_version = ev->fw_build_vers;
	/* fw_version_1 is not available in TLV. */
	cap->fw_version_1 = 0;

	return QDF_STATUS_SUCCESS;
}

/* convert_wireless_modes_tlv() - Convert REGDMN_MODE values sent by target
 *	 to host internal WMI_HOST_REGDMN_MODE values.
 *	 REGULATORY TODO : REGDMN_MODE_11AC_VHT*_2G values are not used by the
 *	 host currently. Add this in the future if required.
 *	 11AX (Phase II) : 11ax related values are not currently
 *	 advertised separately by FW. As part of phase II regulatory bring-up,
 *	 finalize the advertisement mechanism.
 * @target_wireless_mode: target wireless mode received in message
 *
 * Return: returns the host internal wireless mode.
 */
static inline uint32_t convert_wireless_modes_tlv(uint32_t target_wireless_mode)
{

	uint32_t wireless_modes = 0;

	WMI_LOGD("Target wireless mode: 0x%x", target_wireless_mode);

	if (target_wireless_mode & REGDMN_MODE_11A)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11A;

	if (target_wireless_mode & REGDMN_MODE_TURBO)
		wireless_modes |= WMI_HOST_REGDMN_MODE_TURBO;

	if (target_wireless_mode & REGDMN_MODE_11B)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11B;

	if (target_wireless_mode & REGDMN_MODE_PUREG)
		wireless_modes |= WMI_HOST_REGDMN_MODE_PUREG;

	if (target_wireless_mode & REGDMN_MODE_11G)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11G;

	if (target_wireless_mode & REGDMN_MODE_108G)
		wireless_modes |= WMI_HOST_REGDMN_MODE_108G;

	if (target_wireless_mode & REGDMN_MODE_108A)
		wireless_modes |= WMI_HOST_REGDMN_MODE_108A;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT20_2G)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT20_2G;

	if (target_wireless_mode & REGDMN_MODE_XR)
		wireless_modes |= WMI_HOST_REGDMN_MODE_XR;

	if (target_wireless_mode & REGDMN_MODE_11A_HALF_RATE)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11A_HALF_RATE;

	if (target_wireless_mode & REGDMN_MODE_11A_QUARTER_RATE)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11A_QUARTER_RATE;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT20)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT20;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT20)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT20;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT40PLUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT40MINUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT40PLUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT40MINUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT20)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT20;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT40PLUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT40MINUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT80)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT80;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT160)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT160;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT80_80)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT80_80;

	return wireless_modes;
}

/**
 * extract_hal_reg_cap_tlv() - extract HAL registered capabilities
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param cap: pointer to hold HAL reg capabilities
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_hal_reg_cap_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct wlan_psoc_hal_reg_capability *cap)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf || !param_buf->hal_reg_capabilities) {
		WMI_LOGE("%s: Invalid arguments", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(cap, (((uint8_t *)param_buf->hal_reg_capabilities) +
		sizeof(uint32_t)),
		sizeof(struct wlan_psoc_hal_reg_capability));

	cap->wireless_modes = convert_wireless_modes_tlv(
			param_buf->hal_reg_capabilities->wireless_modes);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_host_mem_req_tlv() - Extract host memory request event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_entries: pointer to hold number of entries requested
 *
 * Return: Number of entries requested
 */
static host_mem_req *extract_host_mem_req_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t *num_entries)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed", __func__);
		return NULL;
	}

	if (ev->num_mem_reqs > param_buf->num_mem_reqs) {
		WMI_LOGE("Invalid num_mem_reqs %d:%d",
			 ev->num_mem_reqs, param_buf->num_mem_reqs);
		return NULL;
	}

	*num_entries = ev->num_mem_reqs;

	return (host_mem_req *)param_buf->mem_reqs;
}

/**
 * save_fw_version_in_service_ready_tlv() - Save fw version in service
 * ready function
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
save_fw_version_in_service_ready_tlv(wmi_unified_t wmi_handle, void *evt_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;


	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/*Save fw version from service ready message */
	/*This will be used while sending INIT message */
	qdf_mem_copy(&wmi_handle->fw_abi_version, &ev->fw_abi_vers,
			sizeof(wmi_handle->fw_abi_version));

	return QDF_STATUS_SUCCESS;
}

/**
 * ready_extract_init_status_tlv() - Extract init status from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 *
 * Return: ready status
 */
static uint32_t ready_extract_init_status_tlv(wmi_unified_t wmi_handle,
	void *evt_buf)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;

	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;

	qdf_print("%s:%d", __func__, ev->status);

	return ev->status;
}

/**
 * ready_extract_mac_addr_tlv() - extract mac address from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param macaddr: Pointer to hold MAC address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS ready_extract_mac_addr_tlv(wmi_unified_t wmi_hamdle,
	void *evt_buf, uint8_t *macaddr)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;


	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->mac_addr, macaddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * ready_extract_mac_addr_list_tlv() - extract MAC address list from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param macaddr: Pointer to hold number of MAC addresses
 *
 * Return: Pointer to addr list
 */
static wmi_host_mac_addr *ready_extract_mac_addr_list_tlv(wmi_unified_t wmi_hamdle,
	void *evt_buf, uint8_t *num_mac)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;

	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;

	*num_mac = ev->num_extra_mac_addr;

	return (wmi_host_mac_addr *) param_buf->mac_addr_list;
}

/**
 * extract_ready_params_tlv() - Extract data from ready event apart from
 *		     status, macaddr and version.
 * @wmi_handle: Pointer to WMI handle.
 * @evt_buf: Pointer to Ready event buffer.
 * @ev_param: Pointer to host defined struct to copy the data from event.
 *
 * Return: QDF_STATUS_SUCCESS on success.
 */
static QDF_STATUS extract_ready_event_params_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_ready_ev_param *ev_param)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;

	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;

	ev_param->status = ev->status;
	ev_param->num_dscp_table = ev->num_dscp_table;
	ev_param->num_extra_mac_addr = ev->num_extra_mac_addr;
	ev_param->num_total_peer = ev->num_total_peers;
	ev_param->num_extra_peer = ev->num_extra_peers;
	/* Agile_capability in ready event is supported in TLV target,
	 * as per aDFS FR
	 */
	ev_param->max_ast_index = ev->max_ast_index;
	ev_param->agile_capability = 1;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dbglog_data_len_tlv() - extract debuglog data length
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: length
 */
static uint8_t *extract_dbglog_data_len_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t *len)
{
	 WMI_DEBUG_MESG_EVENTID_param_tlvs *param_buf;

	 param_buf = (WMI_DEBUG_MESG_EVENTID_param_tlvs *) evt_buf;

	 *len = param_buf->num_bufp;

	 return param_buf->bufp;
}


#ifdef CONFIG_MCL
#define IS_WMI_RX_MGMT_FRAME_STATUS_INVALID(_status) \
			((_status) & WMI_RXERR_DECRYPT)
#else
#define IS_WMI_RX_MGMT_FRAME_STATUS_INVALID(_status) false
#endif

/**
 * extract_mgmt_rx_params_tlv() - extract management rx params from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hdr: Pointer to hold header
 * @param bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_mgmt_rx_params_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct mgmt_rx_event_params *hdr,
	uint8_t **bufp)
{
	WMI_MGMT_RX_EVENTID_param_tlvs *param_tlvs = NULL;
	wmi_mgmt_rx_hdr *ev_hdr = NULL;
	int i;

	param_tlvs = (WMI_MGMT_RX_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlvs) {
		WMI_LOGE("Get NULL point message from FW");
		return QDF_STATUS_E_INVAL;
	}

	ev_hdr = param_tlvs->hdr;
	if (!hdr) {
		WMI_LOGE("Rx event is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (IS_WMI_RX_MGMT_FRAME_STATUS_INVALID(ev_hdr->status)) {
		WMI_LOGE("%s: RX mgmt frame decrypt error, discard it",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	hdr->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							ev_hdr->pdev_id);

	hdr->channel = ev_hdr->channel;
	hdr->snr = ev_hdr->snr;
	hdr->rate = ev_hdr->rate;
	hdr->phy_mode = ev_hdr->phy_mode;
	hdr->buf_len = ev_hdr->buf_len;
	hdr->status = ev_hdr->status;
	hdr->flags = ev_hdr->flags;
	hdr->rssi = ev_hdr->rssi;
	hdr->tsf_delta = ev_hdr->tsf_delta;
	for (i = 0; i < ATH_MAX_ANTENNA; i++)
		hdr->rssi_ctl[i] = ev_hdr->rssi_ctl[i];

	*bufp = param_tlvs->bufp;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_roam_param_tlv() - extract vdev roam param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold roam param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_roam_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_roam_event *param)
{
	WMI_ROAM_EVENTID_param_tlvs *param_buf;
	wmi_roam_event_fixed_param *evt;

	param_buf = (WMI_ROAM_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid roam event buffer");
		return QDF_STATUS_E_INVAL;
	}

	evt = param_buf->fixed_param;
	qdf_mem_zero(param, sizeof(*param));

	param->vdev_id = evt->vdev_id;
	param->reason = evt->reason;
	param->rssi = evt->rssi;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_scan_ev_param_tlv() - extract vdev scan param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev scan param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_scan_ev_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct scan_event *param)
{
	WMI_SCAN_EVENTID_param_tlvs *param_buf = NULL;
	wmi_scan_event_fixed_param *evt = NULL;

	param_buf = (WMI_SCAN_EVENTID_param_tlvs *) evt_buf;
	evt = param_buf->fixed_param;

	qdf_mem_zero(param, sizeof(*param));

	switch (evt->event) {
	case WMI_SCAN_EVENT_STARTED:
		param->type = SCAN_EVENT_TYPE_STARTED;
		break;
	case WMI_SCAN_EVENT_COMPLETED:
		param->type = SCAN_EVENT_TYPE_COMPLETED;
		break;
	case WMI_SCAN_EVENT_BSS_CHANNEL:
		param->type = SCAN_EVENT_TYPE_BSS_CHANNEL;
		break;
	case WMI_SCAN_EVENT_FOREIGN_CHANNEL:
		param->type = SCAN_EVENT_TYPE_FOREIGN_CHANNEL;
		break;
	case WMI_SCAN_EVENT_DEQUEUED:
		param->type = SCAN_EVENT_TYPE_DEQUEUED;
		break;
	case WMI_SCAN_EVENT_PREEMPTED:
		param->type = SCAN_EVENT_TYPE_PREEMPTED;
		break;
	case WMI_SCAN_EVENT_START_FAILED:
		param->type = SCAN_EVENT_TYPE_START_FAILED;
		break;
	case WMI_SCAN_EVENT_RESTARTED:
		param->type = SCAN_EVENT_TYPE_RESTARTED;
		break;
	case WMI_SCAN_EVENT_FOREIGN_CHANNEL_EXIT:
		param->type = SCAN_EVENT_TYPE_FOREIGN_CHANNEL_EXIT;
		break;
	case WMI_SCAN_EVENT_MAX:
	default:
		param->type = SCAN_EVENT_TYPE_MAX;
		break;
	};

	switch (evt->reason) {
	case WMI_SCAN_REASON_NONE:
		param->reason = SCAN_REASON_NONE;
		break;
	case WMI_SCAN_REASON_COMPLETED:
		param->reason = SCAN_REASON_COMPLETED;
		break;
	case WMI_SCAN_REASON_CANCELLED:
		param->reason = SCAN_REASON_CANCELLED;
		break;
	case WMI_SCAN_REASON_PREEMPTED:
		param->reason = SCAN_REASON_PREEMPTED;
		break;
	case WMI_SCAN_REASON_TIMEDOUT:
		param->reason = SCAN_REASON_TIMEDOUT;
		break;
	case WMI_SCAN_REASON_INTERNAL_FAILURE:
		param->reason = SCAN_REASON_INTERNAL_FAILURE;
		break;
	case WMI_SCAN_REASON_SUSPENDED:
		param->reason = SCAN_REASON_SUSPENDED;
		break;
	case WMI_SCAN_REASON_DFS_VIOLATION:
		param->reason = SCAN_REASON_DFS_VIOLATION;
		break;
	case WMI_SCAN_REASON_MAX:
		param->reason = SCAN_REASON_MAX;
		break;
	default:
		param->reason = SCAN_REASON_MAX;
		break;
	};

	param->chan_freq = evt->channel_freq;
	param->requester = evt->requestor;
	param->scan_id = evt->scan_id;
	param->vdev_id = evt->vdev_id;
	param->timestamp = evt->tsf_timestamp;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_all_stats_counts_tlv() - extract all stats count from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param stats_param: Pointer to hold stats count
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_all_stats_counts_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_stats_event *stats_param)
{
	wmi_stats_event_fixed_param *ev;
	wmi_per_chain_rssi_stats *rssi_event;
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	uint64_t min_data_len;

	qdf_mem_zero(stats_param, sizeof(*stats_param));
	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	rssi_event = param_buf->chain_stats;
	if (!ev) {
		WMI_LOGE("%s: event fixed param NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (param_buf->num_data > WMI_SVC_MSG_MAX_SIZE - sizeof(*ev)) {
		WMI_LOGE("num_data : %u is invalid", param_buf->num_data);
		return QDF_STATUS_E_FAULT;
	}

	switch (ev->stats_id) {
	case WMI_REQUEST_PEER_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_PEER_STAT;
		break;

	case WMI_REQUEST_AP_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_AP_STAT;
		break;

	case WMI_REQUEST_PDEV_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_PDEV_STAT;
		break;

	case WMI_REQUEST_VDEV_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_VDEV_STAT;
		break;

	case WMI_REQUEST_BCNFLT_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_BCNFLT_STAT;
		break;

	case WMI_REQUEST_VDEV_RATE_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_VDEV_RATE_STAT;
		break;

	case WMI_REQUEST_BCN_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_BCN_STAT;
		break;
	case WMI_REQUEST_PEER_EXTD_STAT:
		stats_param->stats_id |= WMI_REQUEST_PEER_EXTD_STAT;
		break;

	case WMI_REQUEST_PEER_EXTD2_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_PEER_ADV_STATS;
		break;

	default:
		stats_param->stats_id = 0;
		break;

	}

	/* ev->num_*_stats may cause uint32_t overflow, so use uint64_t
	 * to save total length calculated
	 */
	min_data_len =
		(((uint64_t)ev->num_pdev_stats) * sizeof(wmi_pdev_stats_v2)) +
		(((uint64_t)ev->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
		(((uint64_t)ev->num_peer_stats) * sizeof(wmi_peer_stats)) +
		(((uint64_t)ev->num_bcnflt_stats) *
		 sizeof(wmi_bcnfilter_stats_t)) +
		(((uint64_t)ev->num_chan_stats) * sizeof(wmi_chan_stats)) +
		(((uint64_t)ev->num_mib_stats) * sizeof(wmi_mib_stats)) +
		(((uint64_t)ev->num_bcn_stats) * sizeof(wmi_bcn_stats)) +
		(((uint64_t)ev->num_peer_extd_stats) *
		 sizeof(wmi_peer_extd_stats));
	if (param_buf->num_data != min_data_len) {
		WMI_LOGE("data len: %u isn't same as calculated: %llu",
			 param_buf->num_data, min_data_len);
		return QDF_STATUS_E_FAULT;
	}

	stats_param->last_event = ev->last_event;
	stats_param->num_pdev_stats = ev->num_pdev_stats;
	stats_param->num_pdev_ext_stats = 0;
	stats_param->num_vdev_stats = ev->num_vdev_stats;
	stats_param->num_peer_stats = ev->num_peer_stats;
	stats_param->num_peer_extd_stats = ev->num_peer_extd_stats;
	stats_param->num_bcnflt_stats = ev->num_bcnflt_stats;
	stats_param->num_chan_stats = ev->num_chan_stats;
	stats_param->num_bcn_stats = ev->num_bcn_stats;
	stats_param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							ev->pdev_id);

	/* if chain_stats is not populated */
	if (!param_buf->chain_stats || !param_buf->num_chain_stats)
		return QDF_STATUS_SUCCESS;

	if (WMITLV_TAG_STRUC_wmi_per_chain_rssi_stats !=
	    WMITLV_GET_TLVTAG(rssi_event->tlv_header))
		return QDF_STATUS_SUCCESS;

	if (WMITLV_GET_STRUCT_TLVLEN(wmi_per_chain_rssi_stats) !=
	    WMITLV_GET_TLVLEN(rssi_event->tlv_header))
		return QDF_STATUS_SUCCESS;

	if (rssi_event->num_per_chain_rssi_stats >=
	    WMITLV_GET_TLVLEN(rssi_event->tlv_header)) {
		WMI_LOGE("num_per_chain_rssi_stats:%u is out of bounds",
			 rssi_event->num_per_chain_rssi_stats);
		return QDF_STATUS_E_INVAL;
	}
	stats_param->num_rssi_stats = rssi_event->num_per_chain_rssi_stats;

	/* if peer_adv_stats is not populated */
	if (!param_buf->num_peer_extd2_stats)
		return QDF_STATUS_SUCCESS;

	stats_param->num_peer_adv_stats = param_buf->num_peer_extd2_stats;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_tx_stats() - extract pdev tx stats from event
 */
static void extract_pdev_tx_stats(wmi_host_dbg_tx_stats *tx,
				  struct wlan_dbg_tx_stats_v2 *tx_stats)
{
	/* Tx Stats */
	tx->comp_queued = tx_stats->comp_queued;
	tx->comp_delivered = tx_stats->comp_delivered;
	tx->msdu_enqued = tx_stats->msdu_enqued;
	tx->mpdu_enqued = tx_stats->mpdu_enqued;
	tx->wmm_drop = tx_stats->wmm_drop;
	tx->local_enqued = tx_stats->local_enqued;
	tx->local_freed = tx_stats->local_freed;
	tx->hw_queued = tx_stats->hw_queued;
	tx->hw_reaped = tx_stats->hw_reaped;
	tx->underrun = tx_stats->underrun;
	tx->tx_abort = tx_stats->tx_abort;
	tx->mpdus_requed = tx_stats->mpdus_requed;
	tx->data_rc = tx_stats->data_rc;
	tx->self_triggers = tx_stats->self_triggers;
	tx->sw_retry_failure = tx_stats->sw_retry_failure;
	tx->illgl_rate_phy_err = tx_stats->illgl_rate_phy_err;
	tx->pdev_cont_xretry = tx_stats->pdev_cont_xretry;
	tx->pdev_tx_timeout = tx_stats->pdev_tx_timeout;
	tx->pdev_resets = tx_stats->pdev_resets;
	tx->stateless_tid_alloc_failure = tx_stats->stateless_tid_alloc_failure;
	tx->phy_underrun = tx_stats->phy_underrun;
	tx->txop_ovf = tx_stats->txop_ovf;

	return;
}


/**
 * extract_pdev_rx_stats() - extract pdev rx stats from event
 */
static void extract_pdev_rx_stats(wmi_host_dbg_rx_stats *rx,
				  struct wlan_dbg_rx_stats_v2 *rx_stats)
{
	/* Rx Stats */
	rx->mid_ppdu_route_change = rx_stats->mid_ppdu_route_change;
	rx->status_rcvd = rx_stats->status_rcvd;
	rx->r0_frags = rx_stats->r0_frags;
	rx->r1_frags = rx_stats->r1_frags;
	rx->r2_frags = rx_stats->r2_frags;
	/* Only TLV */
	rx->r3_frags = 0;
	rx->htt_msdus = rx_stats->htt_msdus;
	rx->htt_mpdus = rx_stats->htt_mpdus;
	rx->loc_msdus = rx_stats->loc_msdus;
	rx->loc_mpdus = rx_stats->loc_mpdus;
	rx->oversize_amsdu = rx_stats->oversize_amsdu;
	rx->phy_errs = rx_stats->phy_errs;
	rx->phy_err_drop = rx_stats->phy_err_drop;
	rx->mpdu_errs = rx_stats->mpdu_errs;

	return;
}

/**
 * extract_pdev_stats_tlv() - extract pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into pdev stats
 * @param pdev_stats: Pointer to hold pdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_pdev_stats *pdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;

	data = param_buf->data;

	if (index < ev_param->num_pdev_stats) {
		wmi_pdev_stats_v2 *ev = (wmi_pdev_stats_v2 *) ((data) +
				(index * sizeof(wmi_pdev_stats_v2)));

		pdev_stats->chan_nf = ev->chan_nf;
		pdev_stats->tx_frame_count = ev->tx_frame_count;
		pdev_stats->rx_frame_count = ev->rx_frame_count;
		pdev_stats->rx_clear_count = ev->rx_clear_count;
		pdev_stats->cycle_count = ev->cycle_count;
		pdev_stats->phy_err_count = ev->phy_err_count;
		pdev_stats->chan_tx_pwr = ev->chan_tx_pwr;

		extract_pdev_tx_stats(&(pdev_stats->pdev_stats.tx),
			&(ev->pdev_stats.tx));
		extract_pdev_rx_stats(&(pdev_stats->pdev_stats.rx),
			&(ev->pdev_stats.rx));
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_unit_test_tlv() - extract unit test data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param unit_test: pointer to hold unit test data
 * @param maxspace: Amount of space in evt_buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_unit_test_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_unit_test_event *unit_test, uint32_t maxspace)
{
	WMI_UNIT_TEST_EVENTID_param_tlvs *param_buf;
	wmi_unit_test_event_fixed_param *ev_param;
	uint32_t num_bufp;
	uint32_t copy_size;
	uint8_t *bufp;

	param_buf = (WMI_UNIT_TEST_EVENTID_param_tlvs *) evt_buf;
	ev_param = param_buf->fixed_param;
	bufp = param_buf->bufp;
	num_bufp = param_buf->num_bufp;
	unit_test->vdev_id = ev_param->vdev_id;
	unit_test->module_id = ev_param->module_id;
	unit_test->diag_token = ev_param->diag_token;
	unit_test->flag = ev_param->flag;
	unit_test->payload_len = ev_param->payload_len;
	WMI_LOGI("%s:vdev_id:%d mod_id:%d diag_token:%d flag:%d", __func__,
			ev_param->vdev_id,
			ev_param->module_id,
			ev_param->diag_token,
			ev_param->flag);
	WMI_LOGD("%s: Unit-test data given below %d", __func__, num_bufp);
	qdf_trace_hex_dump(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			bufp, num_bufp);
	copy_size = (num_bufp < maxspace) ? num_bufp : maxspace;
	qdf_mem_copy(unit_test->buffer, bufp, copy_size);
	unit_test->buffer_len = copy_size;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_ext_stats_tlv() - extract extended pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended pdev stats
 * @param pdev_ext_stats: Pointer to hold extended pdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_ext_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_pdev_ext_stats *pdev_ext_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stats_tlv() - extract vdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_stats: Pointer to hold vdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_vdev_stats *vdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_vdev_stats) {
		wmi_vdev_stats *ev = (wmi_vdev_stats *) ((data) +
				((ev_param->num_pdev_stats) *
				sizeof(wmi_pdev_stats_v2)) +
				(index * sizeof(wmi_vdev_stats)));

		vdev_stats->vdev_id = ev->vdev_id;
		vdev_stats->vdev_snr.bcn_snr = ev->vdev_snr.bcn_snr;
		vdev_stats->vdev_snr.dat_snr = ev->vdev_snr.dat_snr;

		OS_MEMCPY(vdev_stats->tx_frm_cnt, ev->tx_frm_cnt,
			sizeof(ev->tx_frm_cnt));
		vdev_stats->rx_frm_cnt = ev->rx_frm_cnt;
		OS_MEMCPY(vdev_stats->multiple_retry_cnt,
				ev->multiple_retry_cnt,
				sizeof(ev->multiple_retry_cnt));
		OS_MEMCPY(vdev_stats->fail_cnt, ev->fail_cnt,
				sizeof(ev->fail_cnt));
		vdev_stats->rts_fail_cnt = ev->rts_fail_cnt;
		vdev_stats->rts_succ_cnt = ev->rts_succ_cnt;
		vdev_stats->rx_err_cnt = ev->rx_err_cnt;
		vdev_stats->rx_discard_cnt = ev->rx_discard_cnt;
		vdev_stats->ack_fail_cnt = ev->ack_fail_cnt;
		OS_MEMCPY(vdev_stats->tx_rate_history, ev->tx_rate_history,
			sizeof(ev->tx_rate_history));
		OS_MEMCPY(vdev_stats->bcn_rssi_history, ev->bcn_rssi_history,
			sizeof(ev->bcn_rssi_history));

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_per_chain_rssi_stats_tlv() - api to extract rssi stats from event
 * buffer
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into vdev stats
 * @rssi_stats: Pointer to hold rssi stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_per_chain_rssi_stats_tlv(wmi_unified_t wmi_handle,
			void *evt_buf, uint32_t index,
			struct wmi_host_per_chain_rssi_stats *rssi_stats)
{
	uint8_t *data;
	wmi_rssi_stats *fw_rssi_stats;
	wmi_per_chain_rssi_stats *rssi_event;
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;

	if (!evt_buf) {
		WMI_LOGE("evt_buf is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	rssi_event = param_buf->chain_stats;

	if (index >= rssi_event->num_per_chain_rssi_stats) {
		WMI_LOGE("invalid index");
		return QDF_STATUS_E_INVAL;
	}

	data = ((uint8_t *)(&rssi_event[1])) + WMI_TLV_HDR_SIZE;
	fw_rssi_stats = &((wmi_rssi_stats *)data)[index];

	rssi_stats->vdev_id = fw_rssi_stats->vdev_id;
	qdf_mem_copy(rssi_stats->rssi_avg_beacon,
		     fw_rssi_stats->rssi_avg_beacon,
		     sizeof(fw_rssi_stats->rssi_avg_beacon));
	qdf_mem_copy(rssi_stats->rssi_avg_data,
		     fw_rssi_stats->rssi_avg_data,
		     sizeof(fw_rssi_stats->rssi_avg_data));
	qdf_mem_copy(&rssi_stats->peer_macaddr,
		     &fw_rssi_stats->peer_macaddr,
		     sizeof(fw_rssi_stats->peer_macaddr));

	return QDF_STATUS_SUCCESS;
}



/**
 * extract_bcn_stats_tlv() - extract bcn stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param bcn_stats: Pointer to hold bcn stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_bcn_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_bcn_stats *bcn_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_bcn_stats) {
		wmi_bcn_stats *ev = (wmi_bcn_stats *) ((data) +
			((ev_param->num_pdev_stats) *
			 sizeof(wmi_pdev_stats_v2)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			((ev_param->num_peer_stats) * sizeof(wmi_peer_stats)) +
			((ev_param->num_chan_stats) * sizeof(wmi_chan_stats)) +
			((ev_param->num_mib_stats) * sizeof(wmi_mib_stats)) +
			(index * sizeof(wmi_bcn_stats)));

		bcn_stats->vdev_id = ev->vdev_id;
		bcn_stats->tx_bcn_succ_cnt = ev->tx_bcn_succ_cnt;
		bcn_stats->tx_bcn_outage_cnt = ev->tx_bcn_outage_cnt;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_stats_tlv() - extract peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into peer stats
 * @param peer_stats: Pointer to hold peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_peer_stats *peer_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_peer_stats) {
		wmi_peer_stats *ev = (wmi_peer_stats *) ((data) +
			((ev_param->num_pdev_stats) *
			 sizeof(wmi_pdev_stats_v2)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			(index * sizeof(wmi_peer_stats)));

		OS_MEMSET(peer_stats, 0, sizeof(wmi_host_peer_stats));

		OS_MEMCPY(&(peer_stats->peer_macaddr),
			&(ev->peer_macaddr), sizeof(wmi_mac_addr));

		peer_stats->peer_rssi = ev->peer_rssi;
		peer_stats->peer_tx_rate = ev->peer_tx_rate;
		peer_stats->peer_rx_rate = ev->peer_rx_rate;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_bcnflt_stats_tlv() - extract bcn fault stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into bcn fault stats
 * @param bcnflt_stats: Pointer to hold bcn fault stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_bcnflt_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_bcnflt_stats *peer_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_adv_stats_tlv() - extract adv peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_adv_stats: Pointer to hold adv peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_adv_stats_tlv(wmi_unified_t wmi_handle,
					     void *evt_buf,
					     struct wmi_host_peer_adv_stats
					     *peer_adv_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_peer_extd2_stats *adv_stats;
	int i;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;

	adv_stats = param_buf->peer_extd2_stats;
	if (!adv_stats) {
		WMI_LOGD("%s: no peer_adv stats in event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	for (i = 0; i < param_buf->num_peer_extd2_stats; i++) {
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&adv_stats[i].peer_macaddr,
					   peer_adv_stats[i].peer_macaddr);
		peer_adv_stats[i].fcs_count = adv_stats[i].rx_fcs_err;
		peer_adv_stats[i].rx_bytes =
				(uint64_t)adv_stats[i].rx_bytes_u32 <<
				WMI_LOWER_BITS_SHIFT_32 |
				adv_stats[i].rx_bytes_l32;
		peer_adv_stats[i].rx_count = adv_stats[i].rx_mpdus;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_extd_stats_tlv() - extract extended peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_extd_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t index,
		wmi_host_peer_extd_stats *peer_extd_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;
	ev_param = (wmi_stats_event_fixed_param *)param_buf->fixed_param;
	data = (uint8_t *)param_buf->data;
	if (!data)
		return QDF_STATUS_E_FAILURE;

	if (index < ev_param->num_peer_extd_stats) {
		wmi_peer_extd_stats *ev = (wmi_peer_extd_stats *) (data +
			(ev_param->num_pdev_stats * sizeof(wmi_pdev_stats)) +
			(ev_param->num_vdev_stats * sizeof(wmi_vdev_stats)) +
			(ev_param->num_peer_stats * sizeof(wmi_peer_stats)) +
			(ev_param->num_bcnflt_stats *
			sizeof(wmi_bcnfilter_stats_t)) +
			(ev_param->num_chan_stats * sizeof(wmi_chan_stats)) +
			(ev_param->num_mib_stats * sizeof(wmi_mib_stats)) +
			(ev_param->num_bcn_stats * sizeof(wmi_bcn_stats)) +
			(index * sizeof(wmi_peer_extd_stats)));

		qdf_mem_zero(peer_extd_stats, sizeof(wmi_host_peer_extd_stats));
		qdf_mem_copy(&peer_extd_stats->peer_macaddr, &ev->peer_macaddr,
			     sizeof(wmi_mac_addr));

		peer_extd_stats->rx_mc_bc_cnt = ev->rx_mc_bc_cnt;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * extract_chan_stats_tlv() - extract chan stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into chan stats
 * @param vdev_extd_stats: Pointer to hold chan stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_chan_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_chan_stats *chan_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_chan_stats) {
		wmi_chan_stats *ev = (wmi_chan_stats *) ((data) +
			((ev_param->num_pdev_stats) *
			 sizeof(wmi_pdev_stats_v2)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			((ev_param->num_peer_stats) * sizeof(wmi_peer_stats)) +
			(index * sizeof(wmi_chan_stats)));


		/* Non-TLV doesn't have num_chan_stats */
		chan_stats->chan_mhz = ev->chan_mhz;
		chan_stats->sampling_period_us = ev->sampling_period_us;
		chan_stats->rx_clear_count = ev->rx_clear_count;
		chan_stats->tx_duration_us = ev->tx_duration_us;
		chan_stats->rx_duration_us = ev->rx_duration_us;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_profile_ctx_tlv() - extract profile context from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @idx: profile stats index to extract
 * @param profile_ctx: Pointer to hold profile context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_profile_ctx_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_wlan_profile_ctx_t *profile_ctx)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_profile_data_tlv() - extract profile data from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param profile_data: Pointer to hold profile data
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_profile_data_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint8_t idx, wmi_host_wlan_profile_t *profile_data)
{

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_utf_event_tlv() - extract UTF data info from event
 * @wmi_handle: WMI handle
 * @param evt_buf: Pointer to event buffer
 * @param param: Pointer to hold data
 *
 * Return : QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_utf_event_tlv(wmi_unified_t wmi_handle,
			     uint8_t *evt_buf,
			     struct wmi_host_pdev_utf_event *event)
{
	WMI_PDEV_UTF_EVENTID_param_tlvs *param_buf;
	struct wmi_host_utf_seg_header_info *seg_hdr;

	param_buf = (WMI_PDEV_UTF_EVENTID_param_tlvs *)evt_buf;
	event->data = param_buf->data;
	event->datalen = param_buf->num_data;

	if (event->datalen < sizeof(struct wmi_host_utf_seg_header_info)) {
		WMI_LOGE("%s: Invalid datalen: %d ", __func__, event->datalen);
		return QDF_STATUS_E_INVAL;
	}
	seg_hdr = (struct wmi_host_utf_seg_header_info *)param_buf->data;
	/* Set pdev_id=1 until FW adds support to include pdev_id */
	event->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							seg_hdr->pdev_id);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
static QDF_STATUS extract_rf_characterization_entries_tlv(wmi_unified_t wmi_handle,
	uint8_t *event,
	struct wlan_psoc_host_rf_characterization_entry *rf_characterization_entries)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_CHAN_RF_CHARACTERIZATION_INFO *wmi_rf_characterization_entry;
	uint8_t ix;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	wmi_rf_characterization_entry =
				param_buf->wmi_chan_rf_characterization_info;
	if (!wmi_rf_characterization_entry)
		return QDF_STATUS_E_INVAL;

	for (ix = 0; ix < param_buf->num_wmi_chan_rf_characterization_info; ix++) {
		rf_characterization_entries[ix].freq =
				WMI_CHAN_RF_CHARACTERIZATION_FREQ_GET(
					&wmi_rf_characterization_entry[ix]);

		rf_characterization_entries[ix].bw =
				WMI_CHAN_RF_CHARACTERIZATION_BW_GET(
					&wmi_rf_characterization_entry[ix]);

		rf_characterization_entries[ix].chan_metric =
				WMI_CHAN_RF_CHARACTERIZATION_CHAN_METRIC_GET(
					&wmi_rf_characterization_entry[ix]);

		wmi_nofl_debug("rf_characterization_entries[%u]: freq: %u, "
			       "bw: %u, chan_metric: %u",
			       ix, rf_characterization_entries[ix].freq,
			       rf_characterization_entries[ix].bw,
			       rf_characterization_entries[ix].chan_metric);
	}

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * extract_chainmask_tables_tlv() - extract chain mask tables from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_chainmask_tables_tlv(wmi_unified_t wmi_handle,
		uint8_t *event, struct wlan_psoc_host_chainmask_table *chainmask_table)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_MAC_PHY_CHAINMASK_CAPABILITY *chainmask_caps;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;
	uint8_t i = 0, j = 0;
	uint32_t num_mac_phy_chainmask_caps = 0;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	hw_caps = param_buf->soc_hw_mode_caps;
	if (!hw_caps)
		return QDF_STATUS_E_INVAL;

	if ((!hw_caps->num_chainmask_tables) ||
	    (hw_caps->num_chainmask_tables > PSOC_MAX_CHAINMASK_TABLES) ||
	    (hw_caps->num_chainmask_tables >
	     param_buf->num_mac_phy_chainmask_combo))
		return QDF_STATUS_E_INVAL;

	chainmask_caps = param_buf->mac_phy_chainmask_caps;

	if (!chainmask_caps)
		return QDF_STATUS_E_INVAL;

	for (i = 0; i < hw_caps->num_chainmask_tables; i++) {
		if (chainmask_table[i].num_valid_chainmasks >
		    (UINT_MAX - num_mac_phy_chainmask_caps)) {
			wmi_err_rl("integer overflow, num_mac_phy_chainmask_caps:%d, i:%d, um_valid_chainmasks:%d",
				   num_mac_phy_chainmask_caps, i,
				   chainmask_table[i].num_valid_chainmasks);
			return QDF_STATUS_E_INVAL;
		}
		num_mac_phy_chainmask_caps +=
			chainmask_table[i].num_valid_chainmasks;
	}

	if (num_mac_phy_chainmask_caps >
	    param_buf->num_mac_phy_chainmask_caps) {
		wmi_err_rl("invalid chainmask caps num, num_mac_phy_chainmask_caps:%d, param_buf->num_mac_phy_chainmask_caps:%d",
			   num_mac_phy_chainmask_caps,
			   param_buf->num_mac_phy_chainmask_caps);
		return QDF_STATUS_E_INVAL;
	}

	for (i = 0; i < hw_caps->num_chainmask_tables; i++) {

		wmi_nofl_debug("Dumping chain mask combo data for table : %d",
			       i);
		for (j = 0; j < chainmask_table[i].num_valid_chainmasks; j++) {

			chainmask_table[i].cap_list[j].chainmask =
				chainmask_caps->chainmask;

			chainmask_table[i].cap_list[j].supports_chan_width_20 =
				WMI_SUPPORT_CHAN_WIDTH_20_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_40 =
				WMI_SUPPORT_CHAN_WIDTH_40_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_80 =
				WMI_SUPPORT_CHAN_WIDTH_80_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_160 =
				WMI_SUPPORT_CHAN_WIDTH_160_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_80P80 =
				WMI_SUPPORT_CHAN_WIDTH_80P80_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_2G =
				WMI_SUPPORT_CHAIN_MASK_2G_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_5G =
				WMI_SUPPORT_CHAIN_MASK_5G_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_tx =
				WMI_SUPPORT_CHAIN_MASK_TX_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_rx =
				WMI_SUPPORT_CHAIN_MASK_RX_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_aDFS =
				WMI_SUPPORT_CHAIN_MASK_ADFS_GET(chainmask_caps->supported_flags);

			wmi_nofl_debug("supported_flags: 0x%08x  chainmasks: 0x%08x",
				       chainmask_caps->supported_flags,
				       chainmask_caps->chainmask);
			chainmask_caps++;
		}
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
static void populate_num_rf_characterization_entries(
			struct wlan_psoc_host_service_ext_param *param,
			WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf)
{
	param->num_rf_characterization_entries =
			param_buf->num_wmi_chan_rf_characterization_info;
}
#else
static void populate_num_rf_characterization_entries(
			struct wlan_psoc_host_service_ext_param *param,
			WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf)
{
}
#endif

/**
 * extract_service_ready_ext_tlv() - extract basic extended service ready params
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_service_ready_ext_tlv(wmi_unified_t wmi_handle,
		uint8_t *event, struct wlan_psoc_host_service_ext_param *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_ext_event_fixed_param *ev;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;
	WMI_SOC_HAL_REG_CAPABILITIES *reg_caps;
	WMI_MAC_PHY_CHAINMASK_COMBO *chain_mask_combo;
	uint8_t i = 0;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_INVAL;

	/* Move this to host based bitmap */
	param->default_conc_scan_config_bits =
				ev->default_conc_scan_config_bits;
	param->default_fw_config_bits = ev->default_fw_config_bits;
	param->he_cap_info = ev->he_cap_info;
	param->mpdu_density = ev->mpdu_density;
	param->max_bssid_rx_filters = ev->max_bssid_rx_filters;
	param->fw_build_vers_ext = ev->fw_build_vers_ext;
	param->num_dbr_ring_caps = param_buf->num_dma_ring_caps;
	param->num_bin_scaling_params = param_buf->num_wmi_bin_scaling_params;
	param->max_bssid_indicator = ev->max_bssid_indicator;
	populate_num_rf_characterization_entries(param, param_buf);
	qdf_mem_copy(&param->ppet, &ev->ppet, sizeof(param->ppet));

	hw_caps = param_buf->soc_hw_mode_caps;
	if (hw_caps)
		param->num_hw_modes = hw_caps->num_hw_modes;
	else
		param->num_hw_modes = 0;

	reg_caps = param_buf->soc_hal_reg_caps;
	if (reg_caps)
		param->num_phy = reg_caps->num_phy;
	else
		param->num_phy = 0;

	if (hw_caps) {
		param->num_chainmask_tables = hw_caps->num_chainmask_tables;
		wmi_nofl_debug("Num chain mask tables: %d",
			       hw_caps->num_chainmask_tables);
	} else
		param->num_chainmask_tables = 0;

	if (param->num_chainmask_tables > PSOC_MAX_CHAINMASK_TABLES ||
	    param->num_chainmask_tables >
		param_buf->num_mac_phy_chainmask_combo) {
		wmi_err_rl("num_chainmask_tables is OOB: %u",
			   param->num_chainmask_tables);
		return QDF_STATUS_E_INVAL;
	}
	chain_mask_combo = param_buf->mac_phy_chainmask_combo;

	if (!chain_mask_combo)
		return QDF_STATUS_SUCCESS;

	wmi_nofl_debug("Dumping chain mask combo data");

	for (i = 0; i < param->num_chainmask_tables; i++) {

		wmi_nofl_debug("table_id : %d Num valid chainmasks: %d",
			       chain_mask_combo->chainmask_table_id,
			       chain_mask_combo->num_valid_chainmask);

		param->chainmask_table[i].table_id =
			chain_mask_combo->chainmask_table_id;
		param->chainmask_table[i].num_valid_chainmasks =
			chain_mask_combo->num_valid_chainmask;
		chain_mask_combo++;
	}
	wmi_nofl_debug("chain mask combo end");

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_sar_cap_service_ready_ext_tlv() -
 *       extract SAR cap from service ready event
 * @wmi_handle: wmi handle
 * @event: pointer to event buffer
 * @ext_param: extended target info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_sar_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event,
			struct wlan_psoc_host_service_ext_param *ext_param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_SAR_CAPABILITIES *sar_caps;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *)event;

	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	sar_caps = param_buf->sar_caps;
	if (sar_caps)
		ext_param->sar_version = sar_caps->active_version;
	else
		ext_param->sar_version = 0;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_hw_mode_cap_service_ready_ext_tlv() -
 *       extract HW mode cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param hw_mode_idx: hw mode idx should be less than num_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_hw_mode_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	hw_caps = param_buf->soc_hw_mode_caps;
	if (!hw_caps)
		return QDF_STATUS_E_INVAL;

	if (!hw_caps->num_hw_modes ||
	    !param_buf->hw_mode_caps ||
	    hw_caps->num_hw_modes > PSOC_MAX_HW_MODE ||
	    hw_caps->num_hw_modes > param_buf->num_hw_mode_caps)
		return QDF_STATUS_E_INVAL;

	if (hw_mode_idx >= hw_caps->num_hw_modes)
		return QDF_STATUS_E_INVAL;

	param->hw_mode_id = param_buf->hw_mode_caps[hw_mode_idx].hw_mode_id;
	param->phy_id_map = param_buf->hw_mode_caps[hw_mode_idx].phy_id_map;

	param->hw_mode_config_type =
		param_buf->hw_mode_caps[hw_mode_idx].hw_mode_config_type;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mac_phy_cap_service_ready_ext_tlv() -
 *       extract MAC phy cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param hw_mode_idx: hw mode idx should be less than num_mode
 * @param phy_id: phy id within hw_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_mac_phy_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t hw_mode_id, uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_MAC_PHY_CAPABILITIES *mac_phy_caps;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;
	uint32_t phy_map;
	uint8_t hw_idx, phy_idx = 0;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	hw_caps = param_buf->soc_hw_mode_caps;
	if (!hw_caps)
		return QDF_STATUS_E_INVAL;
	if (hw_caps->num_hw_modes > PSOC_MAX_HW_MODE ||
	    hw_caps->num_hw_modes > param_buf->num_hw_mode_caps) {
		wmi_err_rl("invalid num_hw_modes %d, num_hw_mode_caps %d",
			   hw_caps->num_hw_modes, param_buf->num_hw_mode_caps);
		return QDF_STATUS_E_INVAL;
	}

	for (hw_idx = 0; hw_idx < hw_caps->num_hw_modes; hw_idx++) {
		if (hw_mode_id == param_buf->hw_mode_caps[hw_idx].hw_mode_id)
			break;

		phy_map = param_buf->hw_mode_caps[hw_idx].phy_id_map;
		while (phy_map) {
			phy_map >>= 1;
			phy_idx++;
		}
	}

	if (hw_idx == hw_caps->num_hw_modes)
		return QDF_STATUS_E_INVAL;

	phy_idx += phy_id;
	if (phy_idx >= param_buf->num_mac_phy_caps)
		return QDF_STATUS_E_INVAL;

	mac_phy_caps = &param_buf->mac_phy_caps[phy_idx];

	param->hw_mode_id = mac_phy_caps->hw_mode_id;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							mac_phy_caps->pdev_id);
	param->phy_id = mac_phy_caps->phy_id;
	param->supports_11b =
			WMI_SUPPORT_11B_GET(mac_phy_caps->supported_flags);
	param->supports_11g =
			WMI_SUPPORT_11G_GET(mac_phy_caps->supported_flags);
	param->supports_11a =
			WMI_SUPPORT_11A_GET(mac_phy_caps->supported_flags);
	param->supports_11n =
			WMI_SUPPORT_11N_GET(mac_phy_caps->supported_flags);
	param->supports_11ac =
			WMI_SUPPORT_11AC_GET(mac_phy_caps->supported_flags);
	param->supports_11ax =
			WMI_SUPPORT_11AX_GET(mac_phy_caps->supported_flags);

	param->supported_bands = mac_phy_caps->supported_bands;
	param->ampdu_density = mac_phy_caps->ampdu_density;
	param->max_bw_supported_2G = mac_phy_caps->max_bw_supported_2G;
	param->ht_cap_info_2G = mac_phy_caps->ht_cap_info_2G;
	param->vht_cap_info_2G = mac_phy_caps->vht_cap_info_2G;
	param->vht_supp_mcs_2G = mac_phy_caps->vht_supp_mcs_2G;
	param->he_cap_info_2G[WMI_HOST_HECAP_MAC_WORD1] =
		mac_phy_caps->he_cap_info_2G;
	param->he_cap_info_2G[WMI_HOST_HECAP_MAC_WORD2] =
		mac_phy_caps->he_cap_info_2G_ext;
	param->he_supp_mcs_2G = mac_phy_caps->he_supp_mcs_2G;
	param->tx_chain_mask_2G = mac_phy_caps->tx_chain_mask_2G;
	param->rx_chain_mask_2G = mac_phy_caps->rx_chain_mask_2G;
	param->max_bw_supported_5G = mac_phy_caps->max_bw_supported_5G;
	param->ht_cap_info_5G = mac_phy_caps->ht_cap_info_5G;
	param->vht_cap_info_5G = mac_phy_caps->vht_cap_info_5G;
	param->vht_supp_mcs_5G = mac_phy_caps->vht_supp_mcs_5G;
	param->he_cap_info_5G[WMI_HOST_HECAP_MAC_WORD1] =
		mac_phy_caps->he_cap_info_5G;
	param->he_cap_info_5G[WMI_HOST_HECAP_MAC_WORD2] =
		mac_phy_caps->he_cap_info_5G_ext;
	param->he_supp_mcs_5G = mac_phy_caps->he_supp_mcs_5G;
	param->he_cap_info_internal = mac_phy_caps->he_cap_info_internal;
	param->tx_chain_mask_5G = mac_phy_caps->tx_chain_mask_5G;
	param->rx_chain_mask_5G = mac_phy_caps->rx_chain_mask_5G;
	qdf_mem_copy(&param->he_cap_phy_info_2G,
			&mac_phy_caps->he_cap_phy_info_2G,
			sizeof(param->he_cap_phy_info_2G));
	qdf_mem_copy(&param->he_cap_phy_info_5G,
			&mac_phy_caps->he_cap_phy_info_5G,
			sizeof(param->he_cap_phy_info_5G));
	qdf_mem_copy(&param->he_ppet2G, &mac_phy_caps->he_ppet2G,
				 sizeof(param->he_ppet2G));
	qdf_mem_copy(&param->he_ppet5G, &mac_phy_caps->he_ppet5G,
				sizeof(param->he_ppet5G));
	param->chainmask_table_id = mac_phy_caps->chainmask_table_id;
	param->lmac_id = mac_phy_caps->lmac_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_reg_cap_service_ready_ext_tlv() -
 *       extract REG cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param phy_idx: phy idx should be less than num_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_reg_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_SOC_HAL_REG_CAPABILITIES *reg_caps;
	WMI_HAL_REG_CAPABILITIES_EXT *ext_reg_cap;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	reg_caps = param_buf->soc_hal_reg_caps;
	if (!reg_caps)
		return QDF_STATUS_E_INVAL;

	if (reg_caps->num_phy > param_buf->num_hal_reg_caps)
		return QDF_STATUS_E_INVAL;

	if (phy_idx >= reg_caps->num_phy)
		return QDF_STATUS_E_INVAL;

	if (!param_buf->hal_reg_caps)
		return QDF_STATUS_E_INVAL;

	ext_reg_cap = &param_buf->hal_reg_caps[phy_idx];

	param->phy_id = ext_reg_cap->phy_id;
	param->eeprom_reg_domain = ext_reg_cap->eeprom_reg_domain;
	param->eeprom_reg_domain_ext = ext_reg_cap->eeprom_reg_domain_ext;
	param->regcap1 = ext_reg_cap->regcap1;
	param->regcap2 = ext_reg_cap->regcap2;
	param->wireless_modes = convert_wireless_modes_tlv(
						ext_reg_cap->wireless_modes);
	param->low_2ghz_chan = ext_reg_cap->low_2ghz_chan;
	param->high_2ghz_chan = ext_reg_cap->high_2ghz_chan;
	param->low_5ghz_chan = ext_reg_cap->low_5ghz_chan;
	param->high_5ghz_chan = ext_reg_cap->high_5ghz_chan;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_dbr_ring_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_DMA_RING_CAPABILITIES *dbr_ring_caps;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	dbr_ring_caps = &param_buf->dma_ring_caps[idx];

	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
				dbr_ring_caps->pdev_id);
	param->mod_id = dbr_ring_caps->mod_id;
	param->ring_elems_min = dbr_ring_caps->ring_elems_min;
	param->min_buf_size = dbr_ring_caps->min_buf_size;
	param->min_buf_align = dbr_ring_caps->min_buf_align;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_thermal_stats_tlv() - extract thermal stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param temp: Pointer to hold extracted temperature
 * @param level: Pointer to hold extracted level
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_thermal_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t *temp,
		uint32_t *level, uint32_t *pdev_id)
{
	WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf;
	wmi_therm_throt_stats_event_fixed_param *tt_stats_event;

	param_buf =
		(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	tt_stats_event = param_buf->fixed_param;

	*pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
						tt_stats_event->pdev_id);
	*temp = tt_stats_event->temp;
	*level = tt_stats_event->level;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_thermal_level_stats_tlv() - extract thermal level stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to level stats
 * @param levelcount: Pointer to hold levelcount
 * @param dccount: Pointer to hold dccount
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_thermal_level_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t idx, uint32_t *levelcount,
		uint32_t *dccount)
{
	WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf;
	wmi_therm_throt_level_stats_info *tt_level_info;

	param_buf =
		(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	tt_level_info = param_buf->therm_throt_level_stats_info;

	if (idx < THERMAL_LEVELS) {
		*levelcount = tt_level_info[idx].level_count;
		*dccount = tt_level_info[idx].dc_count;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
#ifdef BIG_ENDIAN_HOST
/**
 * fips_conv_data_be() - LE to BE conversion of FIPS ev data
 * @param data_len - data length
 * @param data - pointer to data
 *
 * Return: QDF_STATUS - success or error status
 */
static QDF_STATUS fips_conv_data_be(uint32_t data_len, uint8_t *data)
{
	uint8_t *data_aligned = NULL;
	int c;
	unsigned char *data_unaligned;

	data_unaligned = qdf_mem_malloc(((sizeof(uint8_t) * data_len) +
					FIPS_ALIGN));
	/* Assigning unaligned space to copy the data */
	/* Checking if kmalloc does successful allocation */
	if (!data_unaligned)
		return QDF_STATUS_E_FAILURE;

	/* Checking if space is alligned */
	if (!FIPS_IS_ALIGNED(data_unaligned, FIPS_ALIGN)) {
		/* align the data space */
		data_aligned =
			(uint8_t *)FIPS_ALIGNTO(data_unaligned, FIPS_ALIGN);
	} else {
		data_aligned = (u_int8_t *)data_unaligned;
	}

	/* memset and copy content from data to data aligned */
	OS_MEMSET(data_aligned, 0, data_len);
	OS_MEMCPY(data_aligned, data, data_len);
	/* Endianness to LE */
	for (c = 0; c < data_len/4; c++) {
		*((u_int32_t *)data_aligned + c) =
			qdf_le32_to_cpu(*((u_int32_t *)data_aligned + c));
	}

	/* Copy content to event->data */
	OS_MEMCPY(data, data_aligned, data_len);

	/* clean up allocated space */
	qdf_mem_free(data_unaligned);
	data_aligned = NULL;
	data_unaligned = NULL;

	/*************************************************************/

	return QDF_STATUS_SUCCESS;
}
#else
/**
 * fips_conv_data_be() - DUMMY for LE platform
 *
 * Return: QDF_STATUS - success
 */
static QDF_STATUS fips_conv_data_be(uint32_t data_len, uint8_t *data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * extract_fips_event_data_tlv() - extract fips event data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: pointer FIPS event params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_fips_event_data_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_fips_event_param *param)
{
	WMI_PDEV_FIPS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_fips_event_fixed_param *event;

	param_buf = (WMI_PDEV_FIPS_EVENTID_param_tlvs *) evt_buf;
	event = (wmi_pdev_fips_event_fixed_param *) param_buf->fixed_param;

	if (fips_conv_data_be(event->data_len, param_buf->data) !=
							QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	param->data = (uint32_t *)param_buf->data;
	param->data_len = event->data_len;
	param->error_status = event->error_status;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								event->pdev_id);

	return QDF_STATUS_SUCCESS;
}

static bool is_management_record_tlv(uint32_t cmd_id)
{
	switch (cmd_id) {
	case WMI_MGMT_TX_SEND_CMDID:
	case WMI_MGMT_TX_COMPLETION_EVENTID:
	case WMI_OFFCHAN_DATA_TX_SEND_CMDID:
	case WMI_MGMT_RX_EVENTID:
		return true;
	default:
		return false;
	}
}

static bool is_diag_event_tlv(uint32_t event_id)
{
	if (WMI_DIAG_EVENTID == event_id)
		return true;

	return false;
}

static uint16_t wmi_tag_vdev_set_cmd(wmi_unified_t wmi_hdl, wmi_buf_t buf)
{
	wmi_vdev_set_param_cmd_fixed_param *set_cmd;

	set_cmd = (wmi_vdev_set_param_cmd_fixed_param *)wmi_buf_data(buf);

	switch (set_cmd->param_id) {
	case WMI_VDEV_PARAM_LISTEN_INTERVAL:
	case WMI_VDEV_PARAM_DTIM_POLICY:
	case WMI_STA_PS_PARAM_INACTIVITY_TIME:
	case WMI_STA_PS_PARAM_MAX_RESET_ITO_COUNT_ON_TIM_NO_TXRX:
		return HTC_TX_PACKET_TAG_AUTO_PM;
	default:
		break;
	}

	return 0;
}

static uint16_t wmi_tag_sta_powersave_cmd(wmi_unified_t wmi_hdl, wmi_buf_t buf)
{
	wmi_sta_powersave_param_cmd_fixed_param *ps_cmd;

	ps_cmd = (wmi_sta_powersave_param_cmd_fixed_param *)wmi_buf_data(buf);

	switch (ps_cmd->param) {
	case WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD:
	case WMI_STA_PS_PARAM_INACTIVITY_TIME:
	case WMI_STA_PS_ENABLE_QPOWER:
		return HTC_TX_PACKET_TAG_AUTO_PM;
	default:
		break;
	}

	return 0;
}

static uint16_t wmi_tag_common_cmd(wmi_unified_t wmi_hdl, wmi_buf_t buf,
				   uint32_t cmd_id)
{
	if (qdf_atomic_read(&wmi_hdl->is_wow_bus_suspended))
		return 0;

	switch (cmd_id) {
	case WMI_VDEV_SET_PARAM_CMDID:
		return wmi_tag_vdev_set_cmd(wmi_hdl, buf);
	case WMI_STA_POWERSAVE_PARAM_CMDID:
		return wmi_tag_sta_powersave_cmd(wmi_hdl, buf);
	default:
		break;
	}

	return 0;
}

static uint16_t wmi_tag_fw_hang_cmd(wmi_unified_t wmi_handle)
{
	uint16_t tag = 0;

	if (qdf_atomic_read(&wmi_handle->is_target_suspended)) {
		pr_err("%s: Target is already suspended, Ignore FW Hang Command",
			__func__);
		return tag;
	}

	if (wmi_handle->tag_crash_inject)
		tag = HTC_TX_PACKET_TAG_AUTO_PM;

	wmi_handle->tag_crash_inject = false;
	return tag;
}

/**
 * wmi_set_htc_tx_tag_tlv() - set HTC TX tag for WMI commands
 * @wmi_handle: WMI handle
 * @buf:	WMI buffer
 * @cmd_id:	WMI command Id
 *
 * Return htc_tx_tag
 */
static uint16_t wmi_set_htc_tx_tag_tlv(wmi_unified_t wmi_handle,
				wmi_buf_t buf,
				uint32_t cmd_id)
{
	uint16_t htc_tx_tag = 0;

	switch (cmd_id) {
	case WMI_WOW_ENABLE_CMDID:
	case WMI_PDEV_SUSPEND_CMDID:
	case WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID:
	case WMI_PDEV_RESUME_CMDID:
#ifdef FEATURE_WLAN_D0WOW
	case WMI_D0_WOW_ENABLE_DISABLE_CMDID:
#endif
		htc_tx_tag = HTC_TX_PACKET_TAG_AUTO_PM;
		break;
	case WMI_FORCE_FW_HANG_CMDID:
		htc_tx_tag = wmi_tag_fw_hang_cmd(wmi_handle);
		break;
	case WMI_VDEV_SET_PARAM_CMDID:
	case WMI_STA_POWERSAVE_PARAM_CMDID:
		htc_tx_tag = wmi_tag_common_cmd(wmi_handle, buf, cmd_id);
	default:
		break;
	}

	return htc_tx_tag;
}

static struct cur_reg_rule
*create_reg_rules_from_wmi(uint32_t num_reg_rules,
		wmi_regulatory_rule_struct *wmi_reg_rule)
{
	struct cur_reg_rule *reg_rule_ptr;
	uint32_t count;

	reg_rule_ptr = qdf_mem_malloc(num_reg_rules * sizeof(*reg_rule_ptr));

	if (!reg_rule_ptr)
		return NULL;

	for (count = 0; count < num_reg_rules; count++) {
		reg_rule_ptr[count].start_freq =
			WMI_REG_RULE_START_FREQ_GET(
					wmi_reg_rule[count].freq_info);
		reg_rule_ptr[count].end_freq =
			WMI_REG_RULE_END_FREQ_GET(
					wmi_reg_rule[count].freq_info);
		reg_rule_ptr[count].max_bw =
			WMI_REG_RULE_MAX_BW_GET(
					wmi_reg_rule[count].bw_pwr_info);
		reg_rule_ptr[count].reg_power =
			WMI_REG_RULE_REG_POWER_GET(
					wmi_reg_rule[count].bw_pwr_info);
		reg_rule_ptr[count].ant_gain =
			WMI_REG_RULE_ANTENNA_GAIN_GET(
					wmi_reg_rule[count].bw_pwr_info);
		reg_rule_ptr[count].flags =
			WMI_REG_RULE_FLAGS_GET(
					wmi_reg_rule[count].flag_info);
	}

	return reg_rule_ptr;
}

static QDF_STATUS extract_reg_chan_list_update_event_tlv(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct cur_regulatory_info *reg_info, uint32_t len)
{
	WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *param_buf;
	wmi_reg_chan_list_cc_event_fixed_param *chan_list_event_hdr;
	wmi_regulatory_rule_struct *wmi_reg_rule;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;

	WMI_LOGD("processing regulatory channel list");

	param_buf = (WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("invalid channel list event buf");
		return QDF_STATUS_E_FAILURE;
	}

	chan_list_event_hdr = param_buf->fixed_param;

	reg_info->num_2g_reg_rules = chan_list_event_hdr->num_2g_reg_rules;
	reg_info->num_5g_reg_rules = chan_list_event_hdr->num_5g_reg_rules;
	num_2g_reg_rules = reg_info->num_2g_reg_rules;
	num_5g_reg_rules = reg_info->num_5g_reg_rules;
	if ((num_2g_reg_rules > MAX_REG_RULES) ||
	    (num_5g_reg_rules > MAX_REG_RULES) ||
	    (num_2g_reg_rules + num_5g_reg_rules > MAX_REG_RULES) ||
	    (num_2g_reg_rules + num_5g_reg_rules !=
	     param_buf->num_reg_rule_array)) {
		wmi_err_rl("Invalid num_2g_reg_rules: %u, num_5g_reg_rules: %u",
			   num_2g_reg_rules, num_5g_reg_rules);
		return QDF_STATUS_E_FAILURE;
	}
	if (param_buf->num_reg_rule_array >
		(WMI_SVC_MSG_MAX_SIZE - sizeof(*chan_list_event_hdr)) /
		sizeof(*wmi_reg_rule)) {
		wmi_err_rl("Invalid num_reg_rule_array: %u",
			   param_buf->num_reg_rule_array);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(reg_info->alpha2, &(chan_list_event_hdr->alpha2),
		     REG_ALPHA2_LEN);
	reg_info->dfs_region = chan_list_event_hdr->dfs_region;
	reg_info->phybitmap = chan_list_event_hdr->phybitmap;
	reg_info->offload_enabled = true;
	reg_info->num_phy = chan_list_event_hdr->num_phy;
	reg_info->phy_id = chan_list_event_hdr->phy_id;
	reg_info->ctry_code = chan_list_event_hdr->country_id;
	reg_info->reg_dmn_pair = chan_list_event_hdr->domain_code;
	if (chan_list_event_hdr->status_code == WMI_REG_SET_CC_STATUS_PASS)
		reg_info->status_code = REG_SET_CC_STATUS_PASS;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_CURRENT_ALPHA2_NOT_FOUND)
		reg_info->status_code = REG_CURRENT_ALPHA2_NOT_FOUND;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_INIT_ALPHA2_NOT_FOUND)
		reg_info->status_code = REG_INIT_ALPHA2_NOT_FOUND;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_SET_CC_CHANGE_NOT_ALLOWED)
		reg_info->status_code = REG_SET_CC_CHANGE_NOT_ALLOWED;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_SET_CC_STATUS_NO_MEMORY)
		reg_info->status_code = REG_SET_CC_STATUS_NO_MEMORY;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_SET_CC_STATUS_FAIL)
		reg_info->status_code = REG_SET_CC_STATUS_FAIL;

	reg_info->min_bw_2g = chan_list_event_hdr->min_bw_2g;
	reg_info->max_bw_2g = chan_list_event_hdr->max_bw_2g;
	reg_info->min_bw_5g = chan_list_event_hdr->min_bw_5g;
	reg_info->max_bw_5g = chan_list_event_hdr->max_bw_5g;

	WMI_LOGD(FL("num_phys = %u and phy_id = %u"),
		 reg_info->num_phy, reg_info->phy_id);

	WMI_LOGD("%s:cc %s dfs %d BW: min_2g %d max_2g %d min_5g %d max_5g %d",
		 __func__, reg_info->alpha2, reg_info->dfs_region,
		 reg_info->min_bw_2g, reg_info->max_bw_2g,
		 reg_info->min_bw_5g, reg_info->max_bw_5g);

	WMI_LOGD("%s: num_2g_reg_rules %d num_5g_reg_rules %d", __func__,
			num_2g_reg_rules, num_5g_reg_rules);
	wmi_reg_rule =
		(wmi_regulatory_rule_struct *)((uint8_t *)chan_list_event_hdr
			+ sizeof(wmi_reg_chan_list_cc_event_fixed_param)
			+ WMI_TLV_HDR_SIZE);
	reg_info->reg_rules_2g_ptr = create_reg_rules_from_wmi(num_2g_reg_rules,
			wmi_reg_rule);
	wmi_reg_rule += num_2g_reg_rules;

	reg_info->reg_rules_5g_ptr = create_reg_rules_from_wmi(num_5g_reg_rules,
			wmi_reg_rule);

	WMI_LOGD("processed regulatory channel list");

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_reg_11d_new_country_event_tlv(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct reg_11d_new_country *reg_11d_country, uint32_t len)
{
	wmi_11d_new_country_event_fixed_param *reg_11d_country_event;
	WMI_11D_NEW_COUNTRY_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_11D_NEW_COUNTRY_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("invalid 11d country event buf");
		return QDF_STATUS_E_FAILURE;
	}

	reg_11d_country_event = param_buf->fixed_param;

	qdf_mem_copy(reg_11d_country->alpha2,
			&reg_11d_country_event->new_alpha2, REG_ALPHA2_LEN);
	reg_11d_country->alpha2[REG_ALPHA2_LEN] = '\0';

	WMI_LOGD("processed 11d country event, new cc %s",
			reg_11d_country->alpha2);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_reg_ch_avoid_event_tlv(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct ch_avoid_ind_type *ch_avoid_ind, uint32_t len)
{
	wmi_avoid_freq_ranges_event_fixed_param *afr_fixed_param;
	wmi_avoid_freq_range_desc *afr_desc;
	uint32_t num_freq_ranges, freq_range_idx;
	WMI_WLAN_FREQ_AVOID_EVENTID_param_tlvs *param_buf =
		(WMI_WLAN_FREQ_AVOID_EVENTID_param_tlvs *) evt_buf;

	if (!param_buf) {
		WMI_LOGE("Invalid channel avoid event buffer");
		return QDF_STATUS_E_INVAL;
	}

	afr_fixed_param = param_buf->fixed_param;
	if (!afr_fixed_param) {
		WMI_LOGE("Invalid channel avoid event fixed param buffer");
		return QDF_STATUS_E_INVAL;
	}

	if (!ch_avoid_ind) {
		WMI_LOGE("Invalid channel avoid indication buffer");
		return QDF_STATUS_E_INVAL;
	}
	if (param_buf->num_avd_freq_range < afr_fixed_param->num_freq_ranges) {
		WMI_LOGE(FL("no.of freq ranges exceeded the limit"));
		return QDF_STATUS_E_INVAL;
	}
	num_freq_ranges = (afr_fixed_param->num_freq_ranges >
			CH_AVOID_MAX_RANGE) ? CH_AVOID_MAX_RANGE :
			afr_fixed_param->num_freq_ranges;

	WMI_LOGD("Channel avoid event received with %d ranges",
		 num_freq_ranges);

	ch_avoid_ind->ch_avoid_range_cnt = num_freq_ranges;
	afr_desc = (wmi_avoid_freq_range_desc *)(param_buf->avd_freq_range);
	for (freq_range_idx = 0; freq_range_idx < num_freq_ranges;
	     freq_range_idx++) {
		ch_avoid_ind->avoid_freq_range[freq_range_idx].start_freq =
			afr_desc->start_freq;
		ch_avoid_ind->avoid_freq_range[freq_range_idx].end_freq =
			afr_desc->end_freq;
		WMI_LOGD("range %d tlv id %u, start freq %u, end freq %u",
				freq_range_idx, afr_desc->tlv_header,
				afr_desc->start_freq, afr_desc->end_freq);
		afr_desc++;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef DFS_COMPONENT_ENABLE
/**
 * extract_dfs_cac_complete_event_tlv() - extract cac complete event
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @vdev_id: vdev id
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dfs_cac_complete_event_tlv(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len)
{
	WMI_VDEV_DFS_CAC_COMPLETE_EVENTID_param_tlvs *param_tlvs;
	wmi_vdev_dfs_cac_complete_event_fixed_param  *cac_event;

	param_tlvs = (WMI_VDEV_DFS_CAC_COMPLETE_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlvs) {
		WMI_LOGE("invalid cac complete event buf");
		return QDF_STATUS_E_FAILURE;
	}

	cac_event = param_tlvs->fixed_param;
	*vdev_id = cac_event->vdev_id;
	WMI_LOGD("processed cac complete event vdev %d", *vdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dfs_ocac_complete_event_tlv() - extract cac complete event
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @vdev_id: vdev id
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_dfs_ocac_complete_event_tlv(wmi_unified_t wmi_handle,
				    uint8_t *evt_buf,
				    struct vdev_adfs_complete_status *param)
{
	WMI_VDEV_ADFS_OCAC_COMPLETE_EVENTID_param_tlvs *param_tlvs;
	wmi_vdev_adfs_ocac_complete_event_fixed_param  *ocac_complete_status;

	param_tlvs = (WMI_VDEV_ADFS_OCAC_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_tlvs) {
		WMI_LOGE("invalid ocac complete event buf");
		return QDF_STATUS_E_FAILURE;
	}

	if (!param_tlvs->fixed_param) {
		WMI_LOGE("invalid param_tlvs->fixed_param");
		return QDF_STATUS_E_FAILURE;
	}

	ocac_complete_status = param_tlvs->fixed_param;
	param->vdev_id = ocac_complete_status->vdev_id;
	param->chan_freq = ocac_complete_status->chan_freq;
	param->center_freq = ocac_complete_status->center_freq;
	param->ocac_status = ocac_complete_status->status;
	param->chan_width = ocac_complete_status->chan_width;
	WMI_LOGD("processed ocac complete event vdev %d agile chan %d",
		 param->vdev_id, param->center_freq);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dfs_radar_detection_event_tlv() - extract radar found event
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @radar_found: radar found event info
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dfs_radar_detection_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len)
{
	WMI_PDEV_DFS_RADAR_DETECTION_EVENTID_param_tlvs *param_tlv;
	wmi_pdev_dfs_radar_detection_event_fixed_param *radar_event;

	param_tlv = (WMI_PDEV_DFS_RADAR_DETECTION_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlv) {
		WMI_LOGE("invalid radar detection event buf");
		return QDF_STATUS_E_FAILURE;
	}

	radar_event = param_tlv->fixed_param;

	radar_found->pdev_id = convert_target_pdev_id_to_host_pdev_id(
						radar_event->pdev_id);

	if (radar_found->pdev_id == WMI_HOST_PDEV_ID_INVALID)
		return QDF_STATUS_E_FAILURE;

	radar_found->detection_mode = radar_event->detection_mode;
	radar_found->chan_freq = radar_event->chan_freq;
	radar_found->chan_width = radar_event->chan_width;
	radar_found->detector_id = radar_event->detector_id;
	radar_found->segment_id = radar_event->segment_id;
	radar_found->timestamp = radar_event->timestamp;
	radar_found->is_chirp = radar_event->is_chirp;
	radar_found->freq_offset = radar_event->freq_offset;
	radar_found->sidx = radar_event->sidx;

	WMI_LOGI("processed radar found event pdev %d,"
		"Radar Event Info:pdev_id %d,timestamp %d,chan_freq  (dur) %d,"
		"chan_width (RSSI) %d,detector_id (false_radar) %d,"
		"freq_offset (radar_check) %d,segment_id %d,sidx %d,"
		"is_chirp %d,detection mode %d",
		radar_event->pdev_id, radar_found->pdev_id,
		radar_event->timestamp, radar_event->chan_freq,
		radar_event->chan_width, radar_event->detector_id,
		radar_event->freq_offset, radar_event->segment_id,
		radar_event->sidx, radar_event->is_chirp,
		radar_event->detection_mode);

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * extract_wlan_radar_event_info_tlv() - extract radar pulse event
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @wlan_radar_event: Pointer to struct radar_event_info
 * @len: length of buffer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_wlan_radar_event_info_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len)
{
	WMI_DFS_RADAR_EVENTID_param_tlvs *param_tlv;
	wmi_dfs_radar_event_fixed_param *radar_event;

	param_tlv = (WMI_DFS_RADAR_EVENTID_param_tlvs *)evt_buf;
	if (!param_tlv) {
		WMI_LOGE("invalid wlan radar event buf");
		return QDF_STATUS_E_FAILURE;
	}

	radar_event = param_tlv->fixed_param;
	wlan_radar_event->pulse_is_chirp = radar_event->pulse_is_chirp;
	wlan_radar_event->pulse_center_freq = radar_event->pulse_center_freq;
	wlan_radar_event->pulse_duration = radar_event->pulse_duration;
	wlan_radar_event->rssi = radar_event->rssi;
	wlan_radar_event->pulse_detect_ts = radar_event->pulse_detect_ts;
	wlan_radar_event->upload_fullts_high = radar_event->upload_fullts_high;
	wlan_radar_event->upload_fullts_low = radar_event->upload_fullts_low;
	wlan_radar_event->peak_sidx = radar_event->peak_sidx;
	wlan_radar_event->delta_peak = radar_event->pulse_delta_peak;
	wlan_radar_event->delta_diff = radar_event->pulse_delta_diff;
	if (radar_event->pulse_flags &
			WMI_DFS_RADAR_PULSE_FLAG_MASK_PSIDX_DIFF_VALID) {
		wlan_radar_event->is_psidx_diff_valid = true;
		wlan_radar_event->psidx_diff = radar_event->psidx_diff;
	} else {
		wlan_radar_event->is_psidx_diff_valid = false;
	}

	wlan_radar_event->pdev_id = radar_event->pdev_id;

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS extract_wlan_radar_event_info_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif

/**
 * send_get_rcpi_cmd_tlv() - send request for rcpi value
 * @wmi_handle: wmi handle
 * @get_rcpi_param: rcpi params
 *
 * Return: QDF status
 */
static QDF_STATUS send_get_rcpi_cmd_tlv(wmi_unified_t wmi_handle,
					struct rcpi_req  *get_rcpi_param)
{
	wmi_buf_t buf;
	wmi_request_rcpi_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_rcpi_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_request_rcpi_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_rcpi_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_request_rcpi_cmd_fixed_param));

	cmd->vdev_id = get_rcpi_param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(get_rcpi_param->mac_addr,
				   &cmd->peer_macaddr);

	switch (get_rcpi_param->measurement_type) {

	case RCPI_MEASUREMENT_TYPE_AVG_MGMT:
		cmd->measurement_type = WMI_RCPI_MEASUREMENT_TYPE_AVG_MGMT;
		break;

	case RCPI_MEASUREMENT_TYPE_AVG_DATA:
		cmd->measurement_type = WMI_RCPI_MEASUREMENT_TYPE_AVG_DATA;
		break;

	case RCPI_MEASUREMENT_TYPE_LAST_MGMT:
		cmd->measurement_type = WMI_RCPI_MEASUREMENT_TYPE_LAST_MGMT;
		break;

	case RCPI_MEASUREMENT_TYPE_LAST_DATA:
		cmd->measurement_type = WMI_RCPI_MEASUREMENT_TYPE_LAST_DATA;
		break;

	default:
		/*
		 * invalid rcpi measurement type, fall back to
		 * RCPI_MEASUREMENT_TYPE_AVG_MGMT
		 */
		cmd->measurement_type = WMI_RCPI_MEASUREMENT_TYPE_AVG_MGMT;
		break;
	}
	WMI_LOGD("RCPI REQ VDEV_ID:%d-->", cmd->vdev_id);
	wmi_mtrace(WMI_REQUEST_RCPI_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_RCPI_CMDID)) {

		WMI_LOGE("%s: Failed to send WMI_REQUEST_RCPI_CMDID",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_rcpi_response_event_tlv() - Extract RCPI event params
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @res: pointer to hold rcpi response from firmware
 *
 * Return: QDF_STATUS_SUCCESS for successful event parse
 *	 else QDF_STATUS_E_INVAL or QDF_STATUS_E_FAILURE
 */
static QDF_STATUS
extract_rcpi_response_event_tlv(wmi_unified_t wmi_handle,
				void *evt_buf, struct rcpi_res *res)
{
	WMI_UPDATE_RCPI_EVENTID_param_tlvs *param_buf;
	wmi_update_rcpi_event_fixed_param *event;

	param_buf = (WMI_UPDATE_RCPI_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE(FL("Invalid rcpi event"));
		return QDF_STATUS_E_INVAL;
	}

	event = param_buf->fixed_param;
	res->vdev_id = event->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->peer_macaddr, res->mac_addr);

	switch (event->measurement_type) {

	case WMI_RCPI_MEASUREMENT_TYPE_AVG_MGMT:
		res->measurement_type = RCPI_MEASUREMENT_TYPE_AVG_MGMT;
		break;

	case WMI_RCPI_MEASUREMENT_TYPE_AVG_DATA:
		res->measurement_type = RCPI_MEASUREMENT_TYPE_AVG_DATA;
		break;

	case WMI_RCPI_MEASUREMENT_TYPE_LAST_MGMT:
		res->measurement_type = RCPI_MEASUREMENT_TYPE_LAST_MGMT;
		break;

	case WMI_RCPI_MEASUREMENT_TYPE_LAST_DATA:
		res->measurement_type = RCPI_MEASUREMENT_TYPE_LAST_DATA;
		break;

	default:
		WMI_LOGE(FL("Invalid rcpi measurement type from firmware"));
		res->measurement_type = RCPI_MEASUREMENT_TYPE_INVALID;
		return QDF_STATUS_E_FAILURE;
	}

	if (event->status)
		return QDF_STATUS_E_FAILURE;
	else
		return QDF_STATUS_SUCCESS;
}

/**
 * convert_host_pdev_id_to_target_pdev_id_legacy() - Convert pdev_id from
 *	   host to target defines. For legacy there is not conversion
 *	   required. Just return pdev_id as it is.
 * @param pdev_id: host pdev_id to be converted.
 * Return: target pdev_id after conversion.
 */
static uint32_t convert_host_pdev_id_to_target_pdev_id_legacy(
							uint32_t pdev_id)
{
	if (pdev_id == WMI_HOST_PDEV_ID_SOC)
		return WMI_PDEV_ID_SOC;

	/*No conversion required*/
	return pdev_id;
}

/**
 * convert_target_pdev_id_to_host_pdev_id_legacy() - Convert pdev_id from
 *	   target to host defines. For legacy there is not conversion
 *	   required. Just return pdev_id as it is.
 * @param pdev_id: target pdev_id to be converted.
 * Return: host pdev_id after conversion.
 */
static uint32_t convert_target_pdev_id_to_host_pdev_id_legacy(
							uint32_t pdev_id)
{
	/*No conversion required*/
	return pdev_id;
}

/**
 *  send_set_country_cmd_tlv() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_set_country_cmd_tlv(wmi_unified_t wmi_handle,
				struct set_country *params)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_set_current_country_cmd_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	uint8_t pdev_id = params->pdev_id;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	cmd = (wmi_set_current_country_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_set_current_country_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_set_current_country_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_host_pdev_id_to_target(pdev_id);
	WMI_LOGD("setting current country to  %s and target pdev_id = %u",
		 params->country, cmd->pdev_id);

	qdf_mem_copy((uint8_t *)&cmd->new_alpha2, params->country, 3);

	wmi_mtrace(WMI_SET_CURRENT_COUNTRY_CMDID, NO_SESSION, 0);
	qdf_status = wmi_unified_cmd_send(wmi_handle,
			buf, len, WMI_SET_CURRENT_COUNTRY_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("Failed to send WMI_SET_CURRENT_COUNTRY_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}

#define WMI_REG_COUNTRY_ALPHA_SET(alpha, val0, val1, val2)	  do { \
	    WMI_SET_BITS(alpha, 0, 8, val0); \
	    WMI_SET_BITS(alpha, 8, 8, val1); \
	    WMI_SET_BITS(alpha, 16, 8, val2); \
	    } while (0)

static QDF_STATUS send_user_country_code_cmd_tlv(wmi_unified_t wmi_handle,
		uint8_t pdev_id, struct cc_regdmn_s *rd)
{
	wmi_set_init_country_cmd_fixed_param *cmd;
	uint16_t len;
	wmi_buf_t buf;
	int ret;

	len = sizeof(wmi_set_init_country_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_set_init_country_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_set_init_country_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_set_init_country_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);

	if (rd->flags == CC_IS_SET) {
		cmd->countrycode_type = WMI_COUNTRYCODE_COUNTRY_ID;
		cmd->country_code.country_id = rd->cc.country_code;
	} else if (rd->flags == ALPHA_IS_SET) {
		cmd->countrycode_type = WMI_COUNTRYCODE_ALPHA2;
		WMI_REG_COUNTRY_ALPHA_SET(cmd->country_code.alpha2,
				rd->cc.alpha[0],
				rd->cc.alpha[1],
				rd->cc.alpha[2]);
	} else if (rd->flags == REGDMN_IS_SET) {
		cmd->countrycode_type = WMI_COUNTRYCODE_DOMAIN_CODE;
		cmd->country_code.domain_code = rd->cc.regdmn_id;
	}

	wmi_mtrace(WMI_SET_INIT_COUNTRY_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_SET_INIT_COUNTRY_CMDID);
	if (ret) {
		WMI_LOGE("Failed to config wow wakeup event");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_obss_detection_cfg_cmd_tlv() - send obss detection
 *   configurations to firmware.
 * @wmi_handle: wmi handle
 * @obss_cfg_param: obss detection configurations
 *
 * Send WMI_SAP_OBSS_DETECTION_CFG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_obss_detection_cfg_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_obss_detection_cfg_param *obss_cfg_param)
{
	wmi_buf_t buf;
	wmi_sap_obss_detection_cfg_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_sap_obss_detection_cfg_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_sap_obss_detection_cfg_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_sap_obss_detection_cfg_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_sap_obss_detection_cfg_cmd_fixed_param));

	cmd->vdev_id = obss_cfg_param->vdev_id;
	cmd->detect_period_ms = obss_cfg_param->obss_detect_period_ms;
	cmd->b_ap_detect_mode = obss_cfg_param->obss_11b_ap_detect_mode;
	cmd->b_sta_detect_mode = obss_cfg_param->obss_11b_sta_detect_mode;
	cmd->g_ap_detect_mode = obss_cfg_param->obss_11g_ap_detect_mode;
	cmd->a_detect_mode = obss_cfg_param->obss_11a_detect_mode;
	cmd->ht_legacy_detect_mode = obss_cfg_param->obss_ht_legacy_detect_mode;
	cmd->ht_mixed_detect_mode = obss_cfg_param->obss_ht_mixed_detect_mode;
	cmd->ht_20mhz_detect_mode = obss_cfg_param->obss_ht_20mhz_detect_mode;

	wmi_mtrace(WMI_SAP_OBSS_DETECTION_CFG_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_SAP_OBSS_DETECTION_CFG_CMDID)) {
		WMI_LOGE("Failed to send WMI_SAP_OBSS_DETECTION_CFG_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_obss_detection_info_tlv() - Extract obss detection info
 *   received from firmware.
 * @evt_buf: pointer to event buffer
 * @obss_detection: Pointer to hold obss detection info
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_obss_detection_info_tlv(uint8_t *evt_buf,
						  struct wmi_obss_detect_info
						  *obss_detection)
{
	WMI_SAP_OBSS_DETECTION_REPORT_EVENTID_param_tlvs *param_buf;
	wmi_sap_obss_detection_info_evt_fixed_param *fix_param;

	if (!obss_detection) {
		WMI_LOGE("%s: Invalid obss_detection event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_SAP_OBSS_DETECTION_REPORT_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid evt_buf", __func__);
		return QDF_STATUS_E_INVAL;
	}

	fix_param = param_buf->fixed_param;
	obss_detection->vdev_id = fix_param->vdev_id;
	obss_detection->matched_detection_masks =
		fix_param->matched_detection_masks;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fix_param->matched_bssid_addr,
				   &obss_detection->matched_bssid_addr[0]);
	switch (fix_param->reason) {
	case WMI_SAP_OBSS_DETECTION_EVENT_REASON_NOT_SUPPORT:
		obss_detection->reason = OBSS_OFFLOAD_DETECTION_DISABLED;
		break;
	case WMI_SAP_OBSS_DETECTION_EVENT_REASON_PRESENT_NOTIFY:
		obss_detection->reason = OBSS_OFFLOAD_DETECTION_PRESENT;
		break;
	case WMI_SAP_OBSS_DETECTION_EVENT_REASON_ABSENT_TIMEOUT:
		obss_detection->reason = OBSS_OFFLOAD_DETECTION_ABSENT;
		break;
	default:
		WMI_LOGE("%s: Invalid reason %d", __func__, fix_param->reason);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_roam_scan_stats_cmd_tlv() - Send roam scan stats req command to fw
 * @wmi_handle: wmi handle
 * @params: pointer to request structure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_roam_scan_stats_cmd_tlv(wmi_unified_t wmi_handle,
			     struct wmi_roam_scan_stats_req *params)
{
	wmi_buf_t buf;
	wmi_request_roam_scan_stats_cmd_fixed_param *cmd;
	WMITLV_TAG_ID tag;
	uint32_t size;
	uint32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_request_roam_scan_stats_cmd_fixed_param *)wmi_buf_data(buf);

	tag = WMITLV_TAG_STRUC_wmi_request_roam_scan_stats_cmd_fixed_param;
	size = WMITLV_GET_STRUCT_TLVLEN(
			wmi_request_roam_scan_stats_cmd_fixed_param);
	WMITLV_SET_HDR(&cmd->tlv_header, tag, size);

	cmd->vdev_id = params->vdev_id;

	WMI_LOGD(FL("Roam Scan Stats Req vdev_id: %u"), cmd->vdev_id);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_ROAM_SCAN_STATS_CMDID)) {
		WMI_LOGE("%s: Failed to send WMI_REQUEST_ROAM_SCAN_STATS_CMDID",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_roam_scan_stats_res_evt_tlv() - Extract roam scan stats event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @vdev_id: output pointer to hold vdev id
 * @res_param: output pointer to hold the allocated response
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
extract_roam_scan_stats_res_evt_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				    uint32_t *vdev_id,
				    struct wmi_roam_scan_stats_res **res_param)
{
	WMI_ROAM_SCAN_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_scan_stats_event_fixed_param *fixed_param;
	uint32_t *client_id = NULL;
	wmi_roaming_timestamp *timestamp = NULL;
	uint32_t *num_channels = NULL;
	uint32_t *chan_info = NULL;
	wmi_mac_addr *old_bssid = NULL;
	uint32_t *is_roaming_success = NULL;
	wmi_mac_addr *new_bssid = NULL;
	uint32_t *num_roam_candidates = NULL;
	wmi_roam_scan_trigger_reason *roam_reason = NULL;
	wmi_mac_addr *bssid = NULL;
	uint32_t *score = NULL;
	uint32_t *channel = NULL;
	uint32_t *rssi = NULL;
	int chan_idx = 0, cand_idx = 0;
	uint32_t total_len;
	struct wmi_roam_scan_stats_res *res;
	uint32_t i, j;
	uint32_t num_scans, scan_param_size;

	*res_param = NULL;
	*vdev_id = 0xFF; /* Initialize to invalid vdev id */
	param_buf = (WMI_ROAM_SCAN_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE(FL("Invalid roam scan stats event"));
		return QDF_STATUS_E_INVAL;
	}

	fixed_param = param_buf->fixed_param;

	num_scans = fixed_param->num_roam_scans;
	scan_param_size = sizeof(struct wmi_roam_scan_stats_params);
	*vdev_id = fixed_param->vdev_id;
	if (num_scans > WMI_ROAM_SCAN_STATS_MAX) {
		wmi_err_rl("%u exceeded maximum roam scan stats: %u",
			   num_scans, WMI_ROAM_SCAN_STATS_MAX);
		return QDF_STATUS_E_INVAL;
	}

	total_len = sizeof(*res) + num_scans * scan_param_size;

	res = qdf_mem_malloc(total_len);
	if (!res)
		return QDF_STATUS_E_NOMEM;

	if (!num_scans) {
		*res_param = res;
		return QDF_STATUS_SUCCESS;
	}

	if (param_buf->client_id &&
	    param_buf->num_client_id == num_scans)
		client_id = param_buf->client_id;

	if (param_buf->timestamp &&
	    param_buf->num_timestamp == num_scans)
		timestamp = param_buf->timestamp;

	if (param_buf->old_bssid &&
	    param_buf->num_old_bssid == num_scans)
		old_bssid = param_buf->old_bssid;

	if (param_buf->new_bssid &&
	    param_buf->num_new_bssid == num_scans)
		new_bssid = param_buf->new_bssid;

	if (param_buf->is_roaming_success &&
	    param_buf->num_is_roaming_success == num_scans)
		is_roaming_success = param_buf->is_roaming_success;

	if (param_buf->roam_reason &&
	    param_buf->num_roam_reason == num_scans)
		roam_reason = param_buf->roam_reason;

	if (param_buf->num_channels &&
	    param_buf->num_num_channels == num_scans) {
		uint32_t count, chan_info_sum = 0;

		num_channels = param_buf->num_channels;
		for (count = 0; count < param_buf->num_num_channels; count++) {
			if (param_buf->num_channels[count] >
			    WMI_ROAM_SCAN_STATS_CHANNELS_MAX) {
				wmi_err_rl("%u exceeded max scan channels %u",
					   param_buf->num_channels[count],
					   WMI_ROAM_SCAN_STATS_CHANNELS_MAX);
				goto error;
			}
			chan_info_sum += param_buf->num_channels[count];
		}

		if (param_buf->chan_info &&
		    param_buf->num_chan_info == chan_info_sum)
			chan_info = param_buf->chan_info;
	}

	if (param_buf->num_roam_candidates &&
	    param_buf->num_num_roam_candidates == num_scans) {
		uint32_t cnt, roam_cand_sum = 0;

		num_roam_candidates = param_buf->num_roam_candidates;
		for (cnt = 0; cnt < param_buf->num_num_roam_candidates; cnt++) {
			if (param_buf->num_roam_candidates[cnt] >
			    WMI_ROAM_SCAN_STATS_CANDIDATES_MAX) {
				wmi_err_rl("%u exceeded max scan cand %u",
					   param_buf->num_roam_candidates[cnt],
					   WMI_ROAM_SCAN_STATS_CANDIDATES_MAX);
				goto error;
			}
			roam_cand_sum += param_buf->num_roam_candidates[cnt];
		}

		if (param_buf->bssid &&
		    param_buf->num_bssid == roam_cand_sum)
			bssid = param_buf->bssid;

		if (param_buf->score &&
		    param_buf->num_score == roam_cand_sum)
			score = param_buf->score;

		if (param_buf->channel &&
		    param_buf->num_channel == roam_cand_sum)
			channel = param_buf->channel;

		if (param_buf->rssi &&
		    param_buf->num_rssi == roam_cand_sum)
			rssi = param_buf->rssi;
	}

	res->num_roam_scans = num_scans;
	for (i = 0; i < num_scans; i++) {
		struct wmi_roam_scan_stats_params *roam = &res->roam_scan[i];

		if (timestamp)
			roam->time_stamp = timestamp[i].lower32bit |
						(timestamp[i].upper32bit << 31);

		if (client_id)
			roam->client_id = client_id[i];

		if (num_channels) {
			roam->num_scan_chans = num_channels[i];
			if (chan_info) {
				for (j = 0; j < num_channels[i]; j++)
					roam->scan_freqs[j] =
							chan_info[chan_idx++];
			}
		}

		if (is_roaming_success)
			roam->is_roam_successful = is_roaming_success[i];

		if (roam_reason) {
			roam->trigger_id = roam_reason[i].trigger_id;
			roam->trigger_value = roam_reason[i].trigger_value;
		}

		if (num_roam_candidates) {
			roam->num_roam_candidates = num_roam_candidates[i];

			for (j = 0; j < num_roam_candidates[i]; j++) {
				if (score)
					roam->cand[j].score = score[cand_idx];
				if (rssi)
					roam->cand[j].rssi = rssi[cand_idx];
				if (channel)
					roam->cand[j].freq =
						channel[cand_idx];

				if (bssid)
					WMI_MAC_ADDR_TO_CHAR_ARRAY(
							&bssid[cand_idx],
							roam->cand[j].bssid);

				cand_idx++;
			}
		}

		if (old_bssid)
			WMI_MAC_ADDR_TO_CHAR_ARRAY(&old_bssid[i],
						   roam->old_bssid);

		if (new_bssid)
			WMI_MAC_ADDR_TO_CHAR_ARRAY(&new_bssid[i],
						   roam->new_bssid);
	}

	*res_param = res;

	return QDF_STATUS_SUCCESS;
error:
	qdf_mem_free(res);
	return QDF_STATUS_E_FAILURE;
}

/**
 * extract_offload_bcn_tx_status_evt() - Extract beacon-tx status event
 * @wmi_handle: wmi handle
 * @evt_buf:   pointer to event buffer
 * @vdev_id:   output pointer to hold vdev id
 * @tx_status: output pointer to hold the tx_status
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_offload_bcn_tx_status_evt(wmi_unified_t wmi_handle,
							void *evt_buf,
							uint32_t *vdev_id,
							uint32_t *tx_status) {
	WMI_OFFLOAD_BCN_TX_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_offload_bcn_tx_status_event_fixed_param *bcn_tx_status_event;

	param_buf = (WMI_OFFLOAD_BCN_TX_STATUS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid offload bcn tx status event buffer");
		return QDF_STATUS_E_INVAL;
	}

	bcn_tx_status_event = param_buf->fixed_param;
	*vdev_id   = bcn_tx_status_event->vdev_id;
	*tx_status = bcn_tx_status_event->tx_status;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_GREEN_AP
static QDF_STATUS extract_green_ap_egap_status_info_tlv(
		uint8_t *evt_buf,
		struct wlan_green_ap_egap_status_info *egap_status_info_params)
{
	WMI_AP_PS_EGAP_INFO_EVENTID_param_tlvs *param_buf;
	wmi_ap_ps_egap_info_event_fixed_param  *egap_info_event;
	wmi_ap_ps_egap_info_chainmask_list *chainmask_event;

	param_buf = (WMI_AP_PS_EGAP_INFO_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid EGAP Info status event buffer");
		return QDF_STATUS_E_INVAL;
	}

	egap_info_event = (wmi_ap_ps_egap_info_event_fixed_param *)
				param_buf->fixed_param;
	chainmask_event = (wmi_ap_ps_egap_info_chainmask_list *)
				param_buf->chainmask_list;

	if (!egap_info_event || !chainmask_event) {
		WMI_LOGE("Invalid EGAP Info event or chainmask event");
		return QDF_STATUS_E_INVAL;
	}

	egap_status_info_params->status = egap_info_event->status;
	egap_status_info_params->mac_id = chainmask_event->mac_id;
	egap_status_info_params->tx_chainmask = chainmask_event->tx_chainmask;
	egap_status_info_params->rx_chainmask = chainmask_event->rx_chainmask;

	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * extract_comb_phyerr_tlv() - extract comb phy error from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @datalen: data length of event buffer
 * @buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @phyerr: Pointer to hold phyerr
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_comb_phyerr_tlv(wmi_unified_t wmi_handle,
					  void *evt_buf,
					  uint16_t datalen,
					  uint16_t *buf_offset,
					  wmi_host_phyerr_t *phyerr)
{
	WMI_PHYERR_EVENTID_param_tlvs *param_tlvs;
	wmi_comb_phyerr_rx_hdr *pe_hdr;

	param_tlvs = (WMI_PHYERR_EVENTID_param_tlvs *)evt_buf;
	if (!param_tlvs) {
		WMI_LOGD("%s: Received null data from FW", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pe_hdr = param_tlvs->hdr;
	if (!pe_hdr) {
		WMI_LOGD("%s: Received Data PE Header is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Ensure it's at least the size of the header */
	if (datalen < sizeof(*pe_hdr)) {
		WMI_LOGD("%s: Expected minimum size %zu, received %d",
			 __func__, sizeof(*pe_hdr), datalen);
		return QDF_STATUS_E_FAILURE;
	}

	phyerr->pdev_id = wmi_handle->ops->
		convert_pdev_id_target_to_host(pe_hdr->pdev_id);
	phyerr->tsf64 = pe_hdr->tsf_l32;
	phyerr->tsf64 |= (((uint64_t)pe_hdr->tsf_u32) << 32);
	phyerr->bufp = param_tlvs->bufp;

	if (pe_hdr->buf_len > param_tlvs->num_bufp) {
		WMI_LOGD("Invalid buf_len %d, num_bufp %d",
			 pe_hdr->buf_len, param_tlvs->num_bufp);
		return QDF_STATUS_E_FAILURE;
	}

	phyerr->buf_len = pe_hdr->buf_len;
	phyerr->phy_err_mask0 = pe_hdr->rsPhyErrMask0;
	phyerr->phy_err_mask1 = pe_hdr->rsPhyErrMask1;
	*buf_offset = sizeof(*pe_hdr) + sizeof(uint32_t);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_single_phyerr_tlv() - extract single phy error from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @datalen: data length of event buffer
 * @buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @phyerr: Pointer to hold phyerr
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_single_phyerr_tlv(wmi_unified_t wmi_handle,
					    void *evt_buf,
					    uint16_t datalen,
					    uint16_t *buf_offset,
					    wmi_host_phyerr_t *phyerr)
{
	wmi_single_phyerr_rx_event *ev;
	uint16_t n = *buf_offset;
	uint8_t *data = (uint8_t *)evt_buf;

	if (n < datalen) {
		if ((datalen - n) < sizeof(ev->hdr)) {
			WMI_LOGD("%s: Not enough space. len=%d, n=%d, hdr=%zu",
				 __func__, datalen, n, sizeof(ev->hdr));
			return QDF_STATUS_E_FAILURE;
		}

		/*
		 * Obtain a pointer to the beginning of the current event.
		 * data[0] is the beginning of the WMI payload.
		 */
		ev = (wmi_single_phyerr_rx_event *)&data[n];

		/*
		 * Sanity check the buffer length of the event against
		 * what we currently have.
		 *
		 * Since buf_len is 32 bits, we check if it overflows
		 * a large 32 bit value.  It's not 0x7fffffff because
		 * we increase n by (buf_len + sizeof(hdr)), which would
		 * in itself cause n to overflow.
		 *
		 * If "int" is 64 bits then this becomes a moot point.
		 */
		if (ev->hdr.buf_len > PHYERROR_MAX_BUFFER_LENGTH) {
			WMI_LOGD("%s: buf_len is garbage 0x%x",
				 __func__, ev->hdr.buf_len);
			return QDF_STATUS_E_FAILURE;
		}

		if ((n + ev->hdr.buf_len) > datalen) {
			WMI_LOGD("%s: len exceeds n=%d, buf_len=%d, datalen=%d",
				 __func__, n, ev->hdr.buf_len, datalen);
			return QDF_STATUS_E_FAILURE;
		}

		phyerr->phy_err_code = WMI_UNIFIED_PHYERRCODE_GET(&ev->hdr);
		phyerr->tsf_timestamp = ev->hdr.tsf_timestamp;
		phyerr->bufp = &ev->bufp[0];
		phyerr->buf_len = ev->hdr.buf_len;
		phyerr->rf_info.rssi_comb = WMI_UNIFIED_RSSI_COMB_GET(&ev->hdr);

		/*
		 * Advance the buffer pointer to the next PHY error.
		 * buflen is the length of this payload, so we need to
		 * advance past the current header _AND_ the payload.
		 */
		n += sizeof(*ev) + ev->hdr.buf_len;
	}
	*buf_offset = n;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_CFR_ENABLE
/**
 * send_peer_cfr_capture_cmd_tlv() - configure cfr params in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold peer cfr config parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_cfr_capture_cmd_tlv(wmi_unified_t wmi_handle,
						struct peer_cfr_params *param)
{
	wmi_peer_cfr_capture_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	int ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_cfr_capture_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_cfr_capture_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_peer_cfr_capture_cmd_fixed_param));

	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->macaddr, &cmd->mac_addr);
	cmd->request = param->request;
	cmd->vdev_id = param->vdev_id;
	cmd->periodicity = param->periodicity;
	cmd->bandwidth = param->bandwidth;
	cmd->capture_method = param->capture_method;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_CFR_CAPTURE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_PEER_CFR_CAPTURE_CMDID");
		wmi_buf_free(buf);
	}

	return ret;
}
#endif /* WLAN_CFR_ENABLE */

/**
 * extract_esp_estimation_ev_param_tlv() - extract air time from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold esp event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_INVAL on failure
 */
static QDF_STATUS
extract_esp_estimation_ev_param_tlv(wmi_unified_t wmi_handle,
				    void *evt_buf,
				    struct esp_estimation_event *param)
{
	WMI_ESP_ESTIMATE_EVENTID_param_tlvs *param_buf;
	wmi_esp_estimate_event_fixed_param *esp_event;

	param_buf = (WMI_ESP_ESTIMATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid ESP Estimate Event buffer");
		return QDF_STATUS_E_INVAL;
	}
	esp_event = param_buf->fixed_param;
	param->ac_airtime_percentage = esp_event->ac_airtime_percentage;

	param->pdev_id = convert_target_pdev_id_to_host_pdev_id(
						esp_event->pdev_id);

	if (param->pdev_id == WMI_HOST_PDEV_ID_INVALID)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/*
 * send_bss_color_change_enable_cmd_tlv() - Send command to enable or disable of
 * updating bss color change within firmware when AP announces bss color change.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev ID
 * @enable: enable bss color change within firmware
 *
 * Send WMI_BSS_COLOR_CHANGE_ENABLE_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_bss_color_change_enable_cmd_tlv(wmi_unified_t wmi_handle,
						       uint32_t vdev_id,
						       bool enable)
{
	wmi_buf_t buf;
	wmi_bss_color_change_enable_fixed_param *cmd;
	uint8_t len = sizeof(wmi_bss_color_change_enable_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_bss_color_change_enable_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bss_color_change_enable_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_bss_color_change_enable_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->enable = enable;
	wmi_mtrace(WMI_BSS_COLOR_CHANGE_ENABLE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_BSS_COLOR_CHANGE_ENABLE_CMDID)) {
		WMI_LOGE("Failed to send WMI_BSS_COLOR_CHANGE_ENABLE_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_obss_color_collision_cfg_cmd_tlv() - send bss color detection
 *   configurations to firmware.
 * @wmi_handle: wmi handle
 * @cfg_param: obss detection configurations
 *
 * Send WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_obss_color_collision_cfg_cmd_tlv(
		wmi_unified_t wmi_handle,
		struct wmi_obss_color_collision_cfg_param *cfg_param)
{
	wmi_buf_t buf;
	wmi_obss_color_collision_det_config_fixed_param *cmd;
	uint8_t len = sizeof(wmi_obss_color_collision_det_config_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_obss_color_collision_det_config_fixed_param *)wmi_buf_data(
			buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_obss_color_collision_det_config_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_obss_color_collision_det_config_fixed_param));
	cmd->vdev_id = cfg_param->vdev_id;
	cmd->flags = cfg_param->flags;
	cmd->current_bss_color = cfg_param->current_bss_color;
	cmd->detection_period_ms = cfg_param->detection_period_ms;
	cmd->scan_period_ms = cfg_param->scan_period_ms;
	cmd->free_slot_expiry_time_ms = cfg_param->free_slot_expiry_time_ms;

	switch (cfg_param->evt_type) {
	case OBSS_COLOR_COLLISION_DETECTION_DISABLE:
		cmd->evt_type = WMI_BSS_COLOR_COLLISION_DISABLE;
		break;
	case OBSS_COLOR_COLLISION_DETECTION:
		cmd->evt_type = WMI_BSS_COLOR_COLLISION_DETECTION;
		break;
	case OBSS_COLOR_FREE_SLOT_TIMER_EXPIRY:
		cmd->evt_type = WMI_BSS_COLOR_FREE_SLOT_TIMER_EXPIRY;
		break;
	case OBSS_COLOR_FREE_SLOT_AVAILABLE:
		cmd->evt_type = WMI_BSS_COLOR_FREE_SLOT_AVAILABLE;
		break;
	default:
		WMI_LOGE("%s: invalid event type: %d",
			 __func__, cfg_param->evt_type);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("%s: evt_type: %d vdev id: %d current_bss_color: %d\n"
		 "detection_period_ms: %d scan_period_ms: %d\n"
		 "free_slot_expiry_timer_ms: %d",
		 __func__, cmd->evt_type, cmd->vdev_id, cmd->current_bss_color,
		 cmd->detection_period_ms, cmd->scan_period_ms,
		 cmd->free_slot_expiry_time_ms);

	wmi_mtrace(WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID)) {
		WMI_LOGE("%s: Sending OBSS color det cmd failed, vdev_id: %d",
			 __func__, cfg_param->vdev_id);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_obss_color_collision_info_tlv() - Extract bss color collision info
 *   received from firmware.
 * @evt_buf: pointer to event buffer
 * @info: Pointer to hold bss collision  info
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_obss_color_collision_info_tlv(uint8_t *evt_buf,
		struct wmi_obss_color_collision_info *info)
{
	WMI_OBSS_COLOR_COLLISION_DETECTION_EVENTID_param_tlvs *param_buf;
	wmi_obss_color_collision_evt_fixed_param *fix_param;

	if (!info) {
		WMI_LOGE("%s: Invalid obss color buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_OBSS_COLOR_COLLISION_DETECTION_EVENTID_param_tlvs *)
		    evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid evt_buf", __func__);
		return QDF_STATUS_E_INVAL;
	}

	fix_param = param_buf->fixed_param;
	info->vdev_id = fix_param->vdev_id;
	info->obss_color_bitmap_bit0to31  =
				fix_param->bss_color_bitmap_bit0to31;
	info->obss_color_bitmap_bit32to63 =
		fix_param->bss_color_bitmap_bit32to63;

	switch (fix_param->evt_type) {
	case WMI_BSS_COLOR_COLLISION_DISABLE:
		info->evt_type = OBSS_COLOR_COLLISION_DETECTION_DISABLE;
		break;
	case WMI_BSS_COLOR_COLLISION_DETECTION:
		info->evt_type = OBSS_COLOR_COLLISION_DETECTION;
		break;
	case WMI_BSS_COLOR_FREE_SLOT_TIMER_EXPIRY:
		info->evt_type = OBSS_COLOR_FREE_SLOT_TIMER_EXPIRY;
		break;
	case WMI_BSS_COLOR_FREE_SLOT_AVAILABLE:
		info->evt_type = OBSS_COLOR_FREE_SLOT_AVAILABLE;
		break;
	default:
		WMI_LOGE("%s: invalid event type: %d, vdev_id: %d",
			 __func__, fix_param->evt_type, fix_param->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static void wmi_11ax_bss_color_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_obss_color_collision_cfg_cmd =
		send_obss_color_collision_cfg_cmd_tlv;
	ops->extract_obss_color_collision_info =
		extract_obss_color_collision_info_tlv;
}

#ifdef WLAN_CFR_ENABLE
/**
 * extract_cfr_peer_tx_event_param_tlv() - Extract peer cfr tx event params
 * @wmi_handle: wmi handle
 * @event_buf: pointer to event buffer
 * @peer_tx_event: Pointer to hold peer cfr tx event params
 *
 * Return QDF_STATUS_SUCCESS on success or proper error code.
 */
static QDF_STATUS
extract_cfr_peer_tx_event_param_tlv(wmi_unified_t *wmi_handle, void *evt_buf,
				    wmi_cfr_peer_tx_event_param *peer_tx_event)
{
	WMI_PEER_CFR_CAPTURE_EVENTID_param_tlvs *param_buf;
	wmi_peer_cfr_capture_event_fixed_param *peer_tx_event_ev;

	param_buf = (WMI_PEER_CFR_CAPTURE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid cfr capture buffer");
		return QDF_STATUS_E_INVAL;
	}

	peer_tx_event_ev = param_buf->fixed_param;
	if (!peer_tx_event_ev) {
		qdf_err("peer cfr capture buffer is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	peer_tx_event->capture_method = peer_tx_event_ev->capture_method;
	peer_tx_event->vdev_id = peer_tx_event_ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&peer_tx_event_ev->mac_addr,
				   &peer_tx_event->peer_mac_addr.bytes[0]);
	peer_tx_event->primary_20mhz_chan =
		peer_tx_event_ev->chan_mhz;
	peer_tx_event->bandwidth = peer_tx_event_ev->bandwidth;
	peer_tx_event->phy_mode = peer_tx_event_ev->phy_mode;
	peer_tx_event->band_center_freq1 = peer_tx_event_ev->band_center_freq1;
	peer_tx_event->band_center_freq2 = peer_tx_event_ev->band_center_freq2;
	peer_tx_event->spatial_streams = peer_tx_event_ev->sts_count;
	peer_tx_event->correlation_info_1 =
		peer_tx_event_ev->correlation_info_1;
	peer_tx_event->correlation_info_2 =
		peer_tx_event_ev->correlation_info_2;
	peer_tx_event->status = peer_tx_event_ev->status;
	peer_tx_event->timestamp_us = peer_tx_event_ev->timestamp_us;
	peer_tx_event->counter = peer_tx_event_ev->counter;
	qdf_mem_copy(peer_tx_event->chain_rssi, peer_tx_event_ev->chain_rssi,
		     sizeof(peer_tx_event->chain_rssi));

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_CFR_ENABLE */

#ifdef WLAN_MWS_INFO_DEBUGFS
/**
 * send_mws_coex_status_req_cmd_tlv() - send coex cmd to fw
 *
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @cmd_id: Coex command id
 *
 * Send WMI_VDEV_GET_MWS_COEX_INFO_CMDID to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_mws_coex_status_req_cmd_tlv(wmi_unified_t wmi_handle,
						   uint32_t vdev_id,
						   uint32_t cmd_id)
{
	wmi_buf_t buf;
	wmi_vdev_get_mws_coex_info_cmd_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	int ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed to allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_get_mws_coex_info_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_get_mws_coex_info_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		      (wmi_vdev_get_mws_coex_info_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->cmd_id  = cmd_id;
	wmi_mtrace(WMI_VDEV_GET_MWS_COEX_INFO_CMDID, vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_GET_MWS_COEX_INFO_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}
#endif

struct wmi_ops tlv_ops =  {
	.send_vdev_create_cmd = send_vdev_create_cmd_tlv,
	.send_vdev_delete_cmd = send_vdev_delete_cmd_tlv,
	.send_vdev_nss_chain_params_cmd = send_vdev_nss_chain_params_cmd_tlv,
	.send_vdev_down_cmd = send_vdev_down_cmd_tlv,
	.send_vdev_start_cmd = send_vdev_start_cmd_tlv,
	.send_hidden_ssid_vdev_restart_cmd =
		send_hidden_ssid_vdev_restart_cmd_tlv,
	.send_peer_flush_tids_cmd = send_peer_flush_tids_cmd_tlv,
	.send_peer_param_cmd = send_peer_param_cmd_tlv,
	.send_vdev_up_cmd = send_vdev_up_cmd_tlv,
	.send_vdev_stop_cmd = send_vdev_stop_cmd_tlv,
	.send_peer_create_cmd = send_peer_create_cmd_tlv,
	.send_peer_delete_cmd = send_peer_delete_cmd_tlv,
	.send_peer_rx_reorder_queue_setup_cmd =
		send_peer_rx_reorder_queue_setup_cmd_tlv,
	.send_peer_rx_reorder_queue_remove_cmd =
		send_peer_rx_reorder_queue_remove_cmd_tlv,
	.send_pdev_utf_cmd = send_pdev_utf_cmd_tlv,
	.send_pdev_param_cmd = send_pdev_param_cmd_tlv,
	.send_suspend_cmd = send_suspend_cmd_tlv,
	.send_resume_cmd = send_resume_cmd_tlv,
	.send_wow_enable_cmd = send_wow_enable_cmd_tlv,
	.send_set_ap_ps_param_cmd = send_set_ap_ps_param_cmd_tlv,
	.send_set_sta_ps_param_cmd = send_set_sta_ps_param_cmd_tlv,
	.send_crash_inject_cmd = send_crash_inject_cmd_tlv,
#ifdef FEATURE_FW_LOG_PARSING
	.send_dbglog_cmd = send_dbglog_cmd_tlv,
#endif
	.send_vdev_set_param_cmd = send_vdev_set_param_cmd_tlv,
	.send_stats_request_cmd = send_stats_request_cmd_tlv,
	.send_packet_log_enable_cmd = send_packet_log_enable_cmd_tlv,
	.send_peer_based_pktlog_cmd = send_peer_based_pktlog_cmd,
	.send_time_stamp_sync_cmd = send_time_stamp_sync_cmd_tlv,
	.send_packet_log_disable_cmd = send_packet_log_disable_cmd_tlv,
	.send_beacon_tmpl_send_cmd = send_beacon_tmpl_send_cmd_tlv,
	.send_peer_assoc_cmd = send_peer_assoc_cmd_tlv,
	.send_scan_start_cmd = send_scan_start_cmd_tlv,
	.send_scan_stop_cmd = send_scan_stop_cmd_tlv,
	.send_scan_chan_list_cmd = send_scan_chan_list_cmd_tlv,
	.send_mgmt_cmd = send_mgmt_cmd_tlv,
	.send_offchan_data_tx_cmd = send_offchan_data_tx_cmd_tlv,
	.send_modem_power_state_cmd = send_modem_power_state_cmd_tlv,
	.send_set_sta_ps_mode_cmd = send_set_sta_ps_mode_cmd_tlv,
	.send_idle_roam_monitor_cmd = send_idle_roam_monitor_cmd_tlv,
	.send_set_sta_uapsd_auto_trig_cmd =
		send_set_sta_uapsd_auto_trig_cmd_tlv,
	.send_get_temperature_cmd = send_get_temperature_cmd_tlv,
	.send_set_smps_params_cmd = send_set_smps_params_cmd_tlv,
	.send_set_mimops_cmd = send_set_mimops_cmd_tlv,
	.send_set_thermal_mgmt_cmd = send_set_thermal_mgmt_cmd_tlv,
	.send_lro_config_cmd = send_lro_config_cmd_tlv,
	.send_peer_rate_report_cmd = send_peer_rate_report_cmd_tlv,
	.send_probe_rsp_tmpl_send_cmd =
				send_probe_rsp_tmpl_send_cmd_tlv,
	.send_p2p_go_set_beacon_ie_cmd =
				send_p2p_go_set_beacon_ie_cmd_tlv,
	.send_setup_install_key_cmd =
				send_setup_install_key_cmd_tlv,
	.send_scan_probe_setoui_cmd =
				send_scan_probe_setoui_cmd_tlv,
#ifdef IPA_OFFLOAD
	.send_ipa_offload_control_cmd =
			 send_ipa_offload_control_cmd_tlv,
#endif
	.send_pno_stop_cmd = send_pno_stop_cmd_tlv,
	.send_pno_start_cmd = send_pno_start_cmd_tlv,
	.send_nlo_mawc_cmd = send_nlo_mawc_cmd_tlv,
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	.send_process_ll_stats_clear_cmd = send_process_ll_stats_clear_cmd_tlv,
	.send_process_ll_stats_set_cmd = send_process_ll_stats_set_cmd_tlv,
	.send_process_ll_stats_get_cmd = send_process_ll_stats_get_cmd_tlv,
#endif /* WLAN_FEATURE_LINK_LAYER_STATS*/
	.send_congestion_cmd = send_congestion_cmd_tlv,
	.send_snr_request_cmd = send_snr_request_cmd_tlv,
	.send_snr_cmd = send_snr_cmd_tlv,
	.send_link_status_req_cmd = send_link_status_req_cmd_tlv,
#ifdef CONFIG_MCL
	.send_bcn_buf_ll_cmd = send_bcn_buf_ll_cmd_tlv,
#ifndef REMOVE_PKT_LOG
	.send_pktlog_wmi_send_cmd = send_pktlog_wmi_send_cmd_tlv,
#endif
#endif
#ifdef WLAN_SUPPORT_GREEN_AP
	.send_egap_conf_params_cmd = send_egap_conf_params_cmd_tlv,
	.send_green_ap_ps_cmd = send_green_ap_ps_cmd_tlv,
	.extract_green_ap_egap_status_info =
			extract_green_ap_egap_status_info_tlv,
#endif
	.send_csa_offload_enable_cmd = send_csa_offload_enable_cmd_tlv,
	.send_start_oem_data_cmd = send_start_oem_data_cmd_tlv,
#ifdef WLAN_FEATURE_CIF_CFR
	.send_oem_dma_cfg_cmd = send_oem_dma_cfg_cmd_tlv,
#endif
	.send_dfs_phyerr_filter_offload_en_cmd =
		 send_dfs_phyerr_filter_offload_en_cmd_tlv,
	.send_stats_ext_req_cmd = send_stats_ext_req_cmd_tlv,
	.send_process_dhcpserver_offload_cmd =
		send_process_dhcpserver_offload_cmd_tlv,
	.send_pdev_set_regdomain_cmd =
				send_pdev_set_regdomain_cmd_tlv,
	.send_regdomain_info_to_fw_cmd = send_regdomain_info_to_fw_cmd_tlv,
	.send_cfg_action_frm_tb_ppdu_cmd = send_cfg_action_frm_tb_ppdu_cmd_tlv,
	.save_fw_version_cmd = save_fw_version_cmd_tlv,
	.check_and_update_fw_version =
		 check_and_update_fw_version_cmd_tlv,
	.send_log_supported_evt_cmd = send_log_supported_evt_cmd_tlv,
	.send_enable_specific_fw_logs_cmd =
		 send_enable_specific_fw_logs_cmd_tlv,
	.send_flush_logs_to_fw_cmd = send_flush_logs_to_fw_cmd_tlv,
	.send_unit_test_cmd = send_unit_test_cmd_tlv,
#ifdef FEATURE_WLAN_APF
	.send_set_active_apf_mode_cmd = wmi_send_set_active_apf_mode_cmd_tlv,
	.send_apf_enable_cmd = wmi_send_apf_enable_cmd_tlv,
	.send_apf_write_work_memory_cmd =
				wmi_send_apf_write_work_memory_cmd_tlv,
	.send_apf_read_work_memory_cmd =
				wmi_send_apf_read_work_memory_cmd_tlv,
	.extract_apf_read_memory_resp_event =
				wmi_extract_apf_read_memory_resp_event_tlv,
#endif /* FEATURE_WLAN_APF */
	.init_cmd_send = init_cmd_send_tlv,
	.send_vdev_set_custom_aggr_size_cmd =
		send_vdev_set_custom_aggr_size_cmd_tlv,
	.send_vdev_set_qdepth_thresh_cmd =
		send_vdev_set_qdepth_thresh_cmd_tlv,
	.send_set_vap_dscp_tid_map_cmd = send_set_vap_dscp_tid_map_cmd_tlv,
	.send_vdev_set_fwtest_param_cmd = send_vdev_set_fwtest_param_cmd_tlv,
	.send_phyerr_disable_cmd = send_phyerr_disable_cmd_tlv,
	.send_phyerr_enable_cmd = send_phyerr_enable_cmd_tlv,
	.send_periodic_chan_stats_config_cmd =
		send_periodic_chan_stats_config_cmd_tlv,
	.send_vdev_spectral_configure_cmd =
				send_vdev_spectral_configure_cmd_tlv,
	.send_vdev_spectral_enable_cmd =
				send_vdev_spectral_enable_cmd_tlv,
	.send_thermal_mitigation_param_cmd =
		send_thermal_mitigation_param_cmd_tlv,
	.send_process_update_edca_param_cmd =
				 send_process_update_edca_param_cmd_tlv,
	.send_bss_color_change_enable_cmd =
		send_bss_color_change_enable_cmd_tlv,
	.send_coex_config_cmd = send_coex_config_cmd_tlv,
	.send_set_country_cmd = send_set_country_cmd_tlv,
	.send_addba_send_cmd = send_addba_send_cmd_tlv,
	.send_delba_send_cmd = send_delba_send_cmd_tlv,
	.send_addba_clearresponse_cmd = send_addba_clearresponse_cmd_tlv,
	.get_target_cap_from_service_ready = extract_service_ready_tlv,
	.extract_hal_reg_cap = extract_hal_reg_cap_tlv,
	.extract_host_mem_req = extract_host_mem_req_tlv,
	.save_service_bitmap = save_service_bitmap_tlv,
	.save_ext_service_bitmap = save_ext_service_bitmap_tlv,
	.is_service_enabled = is_service_enabled_tlv,
	.save_fw_version = save_fw_version_in_service_ready_tlv,
	.ready_extract_init_status = ready_extract_init_status_tlv,
	.ready_extract_mac_addr = ready_extract_mac_addr_tlv,
	.ready_extract_mac_addr_list = ready_extract_mac_addr_list_tlv,
	.extract_ready_event_params = extract_ready_event_params_tlv,
	.extract_dbglog_data_len = extract_dbglog_data_len_tlv,
	.extract_mgmt_rx_params = extract_mgmt_rx_params_tlv,
	.extract_vdev_roam_param = extract_vdev_roam_param_tlv,
	.extract_vdev_scan_ev_param = extract_vdev_scan_ev_param_tlv,
	.extract_all_stats_count = extract_all_stats_counts_tlv,
	.extract_pdev_stats = extract_pdev_stats_tlv,
	.extract_unit_test = extract_unit_test_tlv,
	.extract_pdev_ext_stats = extract_pdev_ext_stats_tlv,
	.extract_vdev_stats = extract_vdev_stats_tlv,
	.extract_per_chain_rssi_stats = extract_per_chain_rssi_stats_tlv,
	.extract_peer_stats = extract_peer_stats_tlv,
	.extract_bcn_stats = extract_bcn_stats_tlv,
	.extract_bcnflt_stats = extract_bcnflt_stats_tlv,
	.extract_peer_extd_stats = extract_peer_extd_stats_tlv,
	.extract_peer_adv_stats = extract_peer_adv_stats_tlv,
	.extract_chan_stats = extract_chan_stats_tlv,
	.extract_profile_ctx = extract_profile_ctx_tlv,
	.extract_profile_data = extract_profile_data_tlv,
	.send_fw_test_cmd = send_fw_test_cmd_tlv,
	.send_power_dbg_cmd = send_power_dbg_cmd_tlv,
	.extract_service_ready_ext = extract_service_ready_ext_tlv,
	.extract_hw_mode_cap_service_ready_ext =
				extract_hw_mode_cap_service_ready_ext_tlv,
	.extract_mac_phy_cap_service_ready_ext =
				extract_mac_phy_cap_service_ready_ext_tlv,
	.extract_reg_cap_service_ready_ext =
				extract_reg_cap_service_ready_ext_tlv,
	.extract_dbr_ring_cap_service_ready_ext =
				extract_dbr_ring_cap_service_ready_ext_tlv,
	.extract_sar_cap_service_ready_ext =
				extract_sar_cap_service_ready_ext_tlv,
	.extract_pdev_utf_event = extract_pdev_utf_event_tlv,
	.wmi_set_htc_tx_tag = wmi_set_htc_tx_tag_tlv,
	.extract_fips_event_data = extract_fips_event_data_tlv,
	.send_pdev_fips_cmd = send_pdev_fips_cmd_tlv,
	.is_management_record = is_management_record_tlv,
	.is_diag_event = is_diag_event_tlv,
#ifdef WLAN_FEATURE_ACTION_OUI
	.send_action_oui_cmd = send_action_oui_cmd_tlv,
#endif
	.send_dfs_phyerr_offload_en_cmd = send_dfs_phyerr_offload_en_cmd_tlv,
#ifdef QCA_SUPPORT_AGILE_DFS
	.send_adfs_ch_cfg_cmd = send_adfs_ch_cfg_cmd_tlv,
	.send_adfs_ocac_abort_cmd = send_adfs_ocac_abort_cmd_tlv,
#endif
	.send_dfs_phyerr_offload_dis_cmd = send_dfs_phyerr_offload_dis_cmd_tlv,
	.extract_reg_chan_list_update_event =
		extract_reg_chan_list_update_event_tlv,
#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
	.extract_rf_characterization_entries =
		extract_rf_characterization_entries_tlv,
#endif
	.extract_chainmask_tables =
		extract_chainmask_tables_tlv,
	.extract_thermal_stats = extract_thermal_stats_tlv,
	.extract_thermal_level_stats = extract_thermal_level_stats_tlv,
	.send_get_rcpi_cmd = send_get_rcpi_cmd_tlv,
	.extract_rcpi_response_event = extract_rcpi_response_event_tlv,
#ifdef DFS_COMPONENT_ENABLE
	.extract_dfs_cac_complete_event = extract_dfs_cac_complete_event_tlv,
	.extract_dfs_ocac_complete_event = extract_dfs_ocac_complete_event_tlv,
	.extract_dfs_radar_detection_event =
		extract_dfs_radar_detection_event_tlv,
	.extract_wlan_radar_event_info = extract_wlan_radar_event_info_tlv,
#endif
	.convert_pdev_id_host_to_target =
		convert_host_pdev_id_to_target_pdev_id_legacy,
	.convert_pdev_id_target_to_host =
		convert_target_pdev_id_to_host_pdev_id_legacy,

	.convert_host_pdev_id_to_target =
		convert_host_pdev_id_to_target_pdev_id,
	.convert_target_pdev_id_to_host =
		convert_target_pdev_id_to_host_pdev_id,

	.send_start_11d_scan_cmd = send_start_11d_scan_cmd_tlv,
	.send_stop_11d_scan_cmd = send_stop_11d_scan_cmd_tlv,
	.extract_reg_11d_new_country_event =
		extract_reg_11d_new_country_event_tlv,
	.send_user_country_code_cmd = send_user_country_code_cmd_tlv,
	.extract_reg_ch_avoid_event =
		extract_reg_ch_avoid_event_tlv,
	.send_obss_detection_cfg_cmd = send_obss_detection_cfg_cmd_tlv,
	.extract_obss_detection_info = extract_obss_detection_info_tlv,
	.wmi_pdev_id_conversion_enable = wmi_tlv_pdev_id_conversion_enable,
	.wmi_free_allocated_event = wmitlv_free_allocated_event_tlvs,
	.wmi_check_and_pad_event = wmitlv_check_and_pad_event_tlvs,
	.wmi_check_command_params = wmitlv_check_command_tlv_params,
	.extract_comb_phyerr = extract_comb_phyerr_tlv,
	.extract_single_phyerr = extract_single_phyerr_tlv,
#ifdef QCA_SUPPORT_CP_STATS
	.extract_cca_stats = extract_cca_stats_tlv,
#endif
	.extract_esp_estimation_ev_param =
				extract_esp_estimation_ev_param_tlv,
	.send_roam_scan_stats_cmd = send_roam_scan_stats_cmd_tlv,
	.extract_roam_scan_stats_res_evt = extract_roam_scan_stats_res_evt_tlv,
#ifdef OBSS_PD
	.send_obss_spatial_reuse_set = send_obss_spatial_reuse_set_cmd_tlv,
	.send_obss_spatial_reuse_set_def_thresh =
		send_obss_spatial_reuse_set_def_thresh_cmd_tlv,
#endif
	.extract_offload_bcn_tx_status_evt = extract_offload_bcn_tx_status_evt,
	.extract_ctl_failsafe_check_ev_param =
		extract_ctl_failsafe_check_ev_param_tlv,
#ifdef WLAN_CFR_ENABLE
	.send_peer_cfr_capture_cmd =
		send_peer_cfr_capture_cmd_tlv,
	.extract_cfr_peer_tx_event_param = extract_cfr_peer_tx_event_param_tlv,
#endif /* WLAN_CFR_ENABLE */
#ifdef WLAN_MWS_INFO_DEBUGFS
	.send_mws_coex_status_req_cmd = send_mws_coex_status_req_cmd_tlv,
#endif
};

/**
 * populate_tlv_event_id() - populates wmi event ids
 *
 * @param event_ids: Pointer to hold event ids
 * Return: None
 */
static void populate_tlv_events_id(uint32_t *event_ids)
{
	event_ids[wmi_service_ready_event_id] = WMI_SERVICE_READY_EVENTID;
	event_ids[wmi_ready_event_id] = WMI_READY_EVENTID;
	event_ids[wmi_scan_event_id] = WMI_SCAN_EVENTID;
	event_ids[wmi_pdev_tpc_config_event_id] = WMI_PDEV_TPC_CONFIG_EVENTID;
	event_ids[wmi_chan_info_event_id] = WMI_CHAN_INFO_EVENTID;
	event_ids[wmi_phyerr_event_id] = WMI_PHYERR_EVENTID;
	event_ids[wmi_pdev_dump_event_id] = WMI_PDEV_DUMP_EVENTID;
	event_ids[wmi_tx_pause_event_id] = WMI_TX_PAUSE_EVENTID;
	event_ids[wmi_dfs_radar_event_id] = WMI_DFS_RADAR_EVENTID;
	event_ids[wmi_pdev_l1ss_track_event_id] = WMI_PDEV_L1SS_TRACK_EVENTID;
	event_ids[wmi_pdev_temperature_event_id] = WMI_PDEV_TEMPERATURE_EVENTID;
	event_ids[wmi_service_ready_ext_event_id] =
						WMI_SERVICE_READY_EXT_EVENTID;
	event_ids[wmi_vdev_start_resp_event_id] = WMI_VDEV_START_RESP_EVENTID;
	event_ids[wmi_vdev_stopped_event_id] = WMI_VDEV_STOPPED_EVENTID;
	event_ids[wmi_vdev_install_key_complete_event_id] =
				WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID;
	event_ids[wmi_vdev_mcc_bcn_intvl_change_req_event_id] =
				WMI_VDEV_MCC_BCN_INTERVAL_CHANGE_REQ_EVENTID;

	event_ids[wmi_vdev_tsf_report_event_id] = WMI_VDEV_TSF_REPORT_EVENTID;
	event_ids[wmi_peer_sta_kickout_event_id] = WMI_PEER_STA_KICKOUT_EVENTID;
	event_ids[wmi_peer_info_event_id] = WMI_PEER_INFO_EVENTID;
	event_ids[wmi_peer_tx_fail_cnt_thr_event_id] =
				WMI_PEER_TX_FAIL_CNT_THR_EVENTID;
	event_ids[wmi_peer_estimated_linkspeed_event_id] =
				WMI_PEER_ESTIMATED_LINKSPEED_EVENTID;
	event_ids[wmi_peer_state_event_id] = WMI_PEER_STATE_EVENTID;
	event_ids[wmi_peer_delete_response_event_id] =
					WMI_PEER_DELETE_RESP_EVENTID;
	event_ids[wmi_mgmt_rx_event_id] = WMI_MGMT_RX_EVENTID;
	event_ids[wmi_host_swba_event_id] = WMI_HOST_SWBA_EVENTID;
	event_ids[wmi_tbttoffset_update_event_id] =
					WMI_TBTTOFFSET_UPDATE_EVENTID;
	event_ids[wmi_ext_tbttoffset_update_event_id] =
					WMI_TBTTOFFSET_EXT_UPDATE_EVENTID;
	event_ids[wmi_offload_bcn_tx_status_event_id] =
				WMI_OFFLOAD_BCN_TX_STATUS_EVENTID;
	event_ids[wmi_offload_prob_resp_tx_status_event_id] =
				WMI_OFFLOAD_PROB_RESP_TX_STATUS_EVENTID;
	event_ids[wmi_mgmt_tx_completion_event_id] =
				WMI_MGMT_TX_COMPLETION_EVENTID;
	event_ids[wmi_pdev_nfcal_power_all_channels_event_id] =
				WMI_PDEV_NFCAL_POWER_ALL_CHANNELS_EVENTID;
	event_ids[wmi_tx_delba_complete_event_id] =
					WMI_TX_DELBA_COMPLETE_EVENTID;
	event_ids[wmi_tx_addba_complete_event_id] =
					WMI_TX_ADDBA_COMPLETE_EVENTID;
	event_ids[wmi_ba_rsp_ssn_event_id] = WMI_BA_RSP_SSN_EVENTID;

	event_ids[wmi_aggr_state_trig_event_id] = WMI_AGGR_STATE_TRIG_EVENTID;

	event_ids[wmi_roam_event_id] = WMI_ROAM_EVENTID;
	event_ids[wmi_profile_match] = WMI_PROFILE_MATCH;

	event_ids[wmi_roam_synch_event_id] = WMI_ROAM_SYNCH_EVENTID;
	event_ids[wmi_roam_synch_frame_event_id] = WMI_ROAM_SYNCH_FRAME_EVENTID;

	event_ids[wmi_p2p_disc_event_id] = WMI_P2P_DISC_EVENTID;

	event_ids[wmi_p2p_noa_event_id] = WMI_P2P_NOA_EVENTID;
	event_ids[wmi_p2p_lo_stop_event_id] =
				WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID;
	event_ids[wmi_vdev_add_macaddr_rx_filter_event_id] =
			WMI_VDEV_ADD_MAC_ADDR_TO_RX_FILTER_STATUS_EVENTID;
	event_ids[wmi_pdev_resume_event_id] = WMI_PDEV_RESUME_EVENTID;
	event_ids[wmi_wow_wakeup_host_event_id] = WMI_WOW_WAKEUP_HOST_EVENTID;
	event_ids[wmi_d0_wow_disable_ack_event_id] =
				WMI_D0_WOW_DISABLE_ACK_EVENTID;
	event_ids[wmi_wow_initial_wakeup_event_id] =
				WMI_WOW_INITIAL_WAKEUP_EVENTID;

	event_ids[wmi_rtt_meas_report_event_id] =
				WMI_RTT_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_tsf_meas_report_event_id] =
				WMI_TSF_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_rtt_error_report_event_id] = WMI_RTT_ERROR_REPORT_EVENTID;
	event_ids[wmi_stats_ext_event_id] = WMI_STATS_EXT_EVENTID;
	event_ids[wmi_iface_link_stats_event_id] = WMI_IFACE_LINK_STATS_EVENTID;
	event_ids[wmi_peer_link_stats_event_id] = WMI_PEER_LINK_STATS_EVENTID;
	event_ids[wmi_radio_link_stats_link] = WMI_RADIO_LINK_STATS_EVENTID;
	event_ids[wmi_diag_event_id_log_supported_event_id] =
				WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID;
	event_ids[wmi_nlo_match_event_id] = WMI_NLO_MATCH_EVENTID;
	event_ids[wmi_nlo_scan_complete_event_id] =
					WMI_NLO_SCAN_COMPLETE_EVENTID;
	event_ids[wmi_apfind_event_id] = WMI_APFIND_EVENTID;
	event_ids[wmi_passpoint_match_event_id] = WMI_PASSPOINT_MATCH_EVENTID;

	event_ids[wmi_gtk_offload_status_event_id] =
				WMI_GTK_OFFLOAD_STATUS_EVENTID;
	event_ids[wmi_gtk_rekey_fail_event_id] = WMI_GTK_REKEY_FAIL_EVENTID;
	event_ids[wmi_csa_handling_event_id] = WMI_CSA_HANDLING_EVENTID;
	event_ids[wmi_chatter_pc_query_event_id] = WMI_CHATTER_PC_QUERY_EVENTID;

	event_ids[wmi_echo_event_id] = WMI_ECHO_EVENTID;

	event_ids[wmi_pdev_utf_event_id] = WMI_PDEV_UTF_EVENTID;

	event_ids[wmi_dbg_msg_event_id] = WMI_DEBUG_MESG_EVENTID;
	event_ids[wmi_update_stats_event_id] = WMI_UPDATE_STATS_EVENTID;
	event_ids[wmi_debug_print_event_id] = WMI_DEBUG_PRINT_EVENTID;
	event_ids[wmi_dcs_interference_event_id] = WMI_DCS_INTERFERENCE_EVENTID;
	event_ids[wmi_pdev_qvit_event_id] = WMI_PDEV_QVIT_EVENTID;
	event_ids[wmi_wlan_profile_data_event_id] =
						WMI_WLAN_PROFILE_DATA_EVENTID;
	event_ids[wmi_pdev_ftm_intg_event_id] = WMI_PDEV_FTM_INTG_EVENTID;
	event_ids[wmi_wlan_freq_avoid_event_id] = WMI_WLAN_FREQ_AVOID_EVENTID;
	event_ids[wmi_vdev_get_keepalive_event_id] =
				WMI_VDEV_GET_KEEPALIVE_EVENTID;
	event_ids[wmi_thermal_mgmt_event_id] = WMI_THERMAL_MGMT_EVENTID;

	event_ids[wmi_diag_container_event_id] =
						WMI_DIAG_DATA_CONTAINER_EVENTID;

	event_ids[wmi_host_auto_shutdown_event_id] =
				WMI_HOST_AUTO_SHUTDOWN_EVENTID;

	event_ids[wmi_update_whal_mib_stats_event_id] =
				WMI_UPDATE_WHAL_MIB_STATS_EVENTID;

	/*update ht/vht info based on vdev (rx and tx NSS and preamble) */
	event_ids[wmi_update_vdev_rate_stats_event_id] =
				WMI_UPDATE_VDEV_RATE_STATS_EVENTID;

	event_ids[wmi_diag_event_id] = WMI_DIAG_EVENTID;
	event_ids[wmi_unit_test_event_id] = WMI_UNIT_TEST_EVENTID;

	/** Set OCB Sched Response, deprecated */
	event_ids[wmi_ocb_set_sched_event_id] = WMI_OCB_SET_SCHED_EVENTID;

	event_ids[wmi_dbg_mesg_flush_complete_event_id] =
				WMI_DEBUG_MESG_FLUSH_COMPLETE_EVENTID;
	event_ids[wmi_rssi_breach_event_id] = WMI_RSSI_BREACH_EVENTID;

	/* GPIO Event */
	event_ids[wmi_gpio_input_event_id] = WMI_GPIO_INPUT_EVENTID;
	event_ids[wmi_uploadh_event_id] = WMI_UPLOADH_EVENTID;

	event_ids[wmi_captureh_event_id] = WMI_CAPTUREH_EVENTID;
	event_ids[wmi_rfkill_state_change_event_id] =
				WMI_RFKILL_STATE_CHANGE_EVENTID;

	/* TDLS Event */
	event_ids[wmi_tdls_peer_event_id] = WMI_TDLS_PEER_EVENTID;

	event_ids[wmi_batch_scan_enabled_event_id] =
				WMI_BATCH_SCAN_ENABLED_EVENTID;
	event_ids[wmi_batch_scan_result_event_id] =
				WMI_BATCH_SCAN_RESULT_EVENTID;
	/* OEM Event */
	event_ids[wmi_oem_cap_event_id] = WMI_OEM_CAPABILITY_EVENTID;
	event_ids[wmi_oem_meas_report_event_id] =
				WMI_OEM_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_oem_report_event_id] = WMI_OEM_ERROR_REPORT_EVENTID;

	/* NAN Event */
	event_ids[wmi_nan_event_id] = WMI_NAN_EVENTID;

	/* LPI Event */
	event_ids[wmi_lpi_result_event_id] = WMI_LPI_RESULT_EVENTID;
	event_ids[wmi_lpi_status_event_id] = WMI_LPI_STATUS_EVENTID;
	event_ids[wmi_lpi_handoff_event_id] = WMI_LPI_HANDOFF_EVENTID;

	/* ExtScan events */
	event_ids[wmi_extscan_start_stop_event_id] =
				WMI_EXTSCAN_START_STOP_EVENTID;
	event_ids[wmi_extscan_operation_event_id] =
				WMI_EXTSCAN_OPERATION_EVENTID;
	event_ids[wmi_extscan_table_usage_event_id] =
				WMI_EXTSCAN_TABLE_USAGE_EVENTID;
	event_ids[wmi_extscan_cached_results_event_id] =
				WMI_EXTSCAN_CACHED_RESULTS_EVENTID;
	event_ids[wmi_extscan_wlan_change_results_event_id] =
				WMI_EXTSCAN_WLAN_CHANGE_RESULTS_EVENTID;
	event_ids[wmi_extscan_hotlist_match_event_id] =
				WMI_EXTSCAN_HOTLIST_MATCH_EVENTID;
	event_ids[wmi_extscan_capabilities_event_id] =
				WMI_EXTSCAN_CAPABILITIES_EVENTID;
	event_ids[wmi_extscan_hotlist_ssid_match_event_id] =
				WMI_EXTSCAN_HOTLIST_SSID_MATCH_EVENTID;

	/* mDNS offload events */
	event_ids[wmi_mdns_stats_event_id] = WMI_MDNS_STATS_EVENTID;

	/* SAP Authentication offload events */
	event_ids[wmi_sap_ofl_add_sta_event_id] = WMI_SAP_OFL_ADD_STA_EVENTID;
	event_ids[wmi_sap_ofl_del_sta_event_id] = WMI_SAP_OFL_DEL_STA_EVENTID;

	/** Out-of-context-of-bss (OCB) events */
	event_ids[wmi_ocb_set_config_resp_event_id] =
				WMI_OCB_SET_CONFIG_RESP_EVENTID;
	event_ids[wmi_ocb_get_tsf_timer_resp_event_id] =
				WMI_OCB_GET_TSF_TIMER_RESP_EVENTID;
	event_ids[wmi_dcc_get_stats_resp_event_id] =
				WMI_DCC_GET_STATS_RESP_EVENTID;
	event_ids[wmi_dcc_update_ndl_resp_event_id] =
				WMI_DCC_UPDATE_NDL_RESP_EVENTID;
	event_ids[wmi_dcc_stats_event_id] = WMI_DCC_STATS_EVENTID;
	/* System-On-Chip events */
	event_ids[wmi_soc_set_hw_mode_resp_event_id] =
				WMI_SOC_SET_HW_MODE_RESP_EVENTID;
	event_ids[wmi_soc_hw_mode_transition_event_id] =
				WMI_SOC_HW_MODE_TRANSITION_EVENTID;
	event_ids[wmi_soc_set_dual_mac_config_resp_event_id] =
				WMI_SOC_SET_DUAL_MAC_CONFIG_RESP_EVENTID;
	event_ids[wmi_pdev_fips_event_id] = WMI_PDEV_FIPS_EVENTID;
	event_ids[wmi_pdev_csa_switch_count_status_event_id] =
				WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID;
	event_ids[wmi_vdev_ocac_complete_event_id] =
				WMI_VDEV_ADFS_OCAC_COMPLETE_EVENTID;
	event_ids[wmi_reg_chan_list_cc_event_id] = WMI_REG_CHAN_LIST_CC_EVENTID;
	event_ids[wmi_inst_rssi_stats_event_id] = WMI_INST_RSSI_STATS_EVENTID;
	event_ids[wmi_pdev_tpc_config_event_id] = WMI_PDEV_TPC_CONFIG_EVENTID;
	event_ids[wmi_peer_sta_ps_statechg_event_id] =
					WMI_PEER_STA_PS_STATECHG_EVENTID;
	event_ids[wmi_pdev_channel_hopping_event_id] =
					WMI_PDEV_CHANNEL_HOPPING_EVENTID;
	event_ids[wmi_offchan_data_tx_completion_event] =
				WMI_OFFCHAN_DATA_TX_COMPLETION_EVENTID;
	event_ids[wmi_dfs_cac_complete_id] = WMI_VDEV_DFS_CAC_COMPLETE_EVENTID;
	event_ids[wmi_dfs_radar_detection_event_id] =
		WMI_PDEV_DFS_RADAR_DETECTION_EVENTID;
	event_ids[wmi_tt_stats_event_id] = WMI_THERM_THROT_STATS_EVENTID;
	event_ids[wmi_11d_new_country_event_id] = WMI_11D_NEW_COUNTRY_EVENTID;
	event_ids[wmi_pdev_tpc_event_id] = WMI_PDEV_TPC_EVENTID;
	event_ids[wmi_get_arp_stats_req_id] = WMI_VDEV_GET_ARP_STAT_EVENTID;
	event_ids[wmi_service_available_event_id] =
						WMI_SERVICE_AVAILABLE_EVENTID;
	event_ids[wmi_update_rcpi_event_id] = WMI_UPDATE_RCPI_EVENTID;
	event_ids[wmi_pdev_check_cal_version_event_id] = WMI_PDEV_CHECK_CAL_VERSION_EVENTID;
	/* NDP events */
	event_ids[wmi_ndp_initiator_rsp_event_id] =
		WMI_NDP_INITIATOR_RSP_EVENTID;
	event_ids[wmi_ndp_indication_event_id] = WMI_NDP_INDICATION_EVENTID;
	event_ids[wmi_ndp_confirm_event_id] = WMI_NDP_CONFIRM_EVENTID;
	event_ids[wmi_ndp_responder_rsp_event_id] =
		WMI_NDP_RESPONDER_RSP_EVENTID;
	event_ids[wmi_ndp_end_indication_event_id] =
		WMI_NDP_END_INDICATION_EVENTID;
	event_ids[wmi_ndp_end_rsp_event_id] = WMI_NDP_END_RSP_EVENTID;
	event_ids[wmi_ndl_schedule_update_event_id] =
					WMI_NDL_SCHEDULE_UPDATE_EVENTID;
	event_ids[wmi_ndp_event_id] = WMI_NDP_EVENTID;

	event_ids[wmi_oem_response_event_id] = WMI_OEM_RESPONSE_EVENTID;
	event_ids[wmi_peer_stats_info_event_id] = WMI_PEER_STATS_INFO_EVENTID;
	event_ids[wmi_pdev_chip_power_stats_event_id] =
		WMI_PDEV_CHIP_POWER_STATS_EVENTID;
	event_ids[wmi_ap_ps_egap_info_event_id] = WMI_AP_PS_EGAP_INFO_EVENTID;
	event_ids[wmi_peer_assoc_conf_event_id] = WMI_PEER_ASSOC_CONF_EVENTID;
	event_ids[wmi_vdev_delete_resp_event_id] = WMI_VDEV_DELETE_RESP_EVENTID;
	event_ids[wmi_apf_capability_info_event_id] =
		WMI_BPF_CAPABILIY_INFO_EVENTID;
	event_ids[wmi_vdev_encrypt_decrypt_data_rsp_event_id] =
		WMI_VDEV_ENCRYPT_DECRYPT_DATA_RESP_EVENTID;
	event_ids[wmi_report_rx_aggr_failure_event_id] =
		WMI_REPORT_RX_AGGR_FAILURE_EVENTID;
	event_ids[wmi_pdev_chip_pwr_save_failure_detect_event_id] =
		WMI_PDEV_CHIP_POWER_SAVE_FAILURE_DETECTED_EVENTID;
	event_ids[wmi_peer_antdiv_info_event_id] = WMI_PEER_ANTDIV_INFO_EVENTID;
	event_ids[wmi_pdev_set_hw_mode_rsp_event_id] =
		WMI_PDEV_SET_HW_MODE_RESP_EVENTID;
	event_ids[wmi_pdev_hw_mode_transition_event_id] =
		WMI_PDEV_HW_MODE_TRANSITION_EVENTID;
	event_ids[wmi_pdev_set_mac_config_resp_event_id] =
		WMI_PDEV_SET_MAC_CONFIG_RESP_EVENTID;
	event_ids[wmi_coex_bt_activity_event_id] =
		WMI_WLAN_COEX_BT_ACTIVITY_EVENTID;
	event_ids[wmi_mgmt_tx_bundle_completion_event_id] =
		WMI_MGMT_TX_BUNDLE_COMPLETION_EVENTID;
	event_ids[wmi_radio_tx_power_level_stats_event_id] =
		WMI_RADIO_TX_POWER_LEVEL_STATS_EVENTID;
	event_ids[wmi_report_stats_event_id] = WMI_REPORT_STATS_EVENTID;
	event_ids[wmi_dma_buf_release_event_id] =
					WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID;
	event_ids[wmi_sap_obss_detection_report_event_id] =
		WMI_SAP_OBSS_DETECTION_REPORT_EVENTID;
	event_ids[wmi_host_swfda_event_id] = WMI_HOST_SWFDA_EVENTID;
	event_ids[wmi_sar_get_limits_event_id] = WMI_SAR_GET_LIMITS_EVENTID;
	event_ids[wmi_obss_color_collision_report_event_id] =
		WMI_OBSS_COLOR_COLLISION_DETECTION_EVENTID;
	event_ids[wmi_pdev_div_rssi_antid_event_id] =
		WMI_PDEV_DIV_RSSI_ANTID_EVENTID;
	event_ids[wmi_twt_enable_complete_event_id] =
		WMI_TWT_ENABLE_COMPLETE_EVENTID;
	event_ids[wmi_apf_get_vdev_work_memory_resp_event_id] =
		WMI_BPF_GET_VDEV_WORK_MEMORY_RESP_EVENTID;
	event_ids[wmi_wlan_sar2_result_event_id] = WMI_SAR2_RESULT_EVENTID;
	event_ids[wmi_esp_estimate_event_id] = WMI_ESP_ESTIMATE_EVENTID;
	event_ids[wmi_roam_scan_stats_event_id] = WMI_ROAM_SCAN_STATS_EVENTID;
#ifdef WLAN_FEATURE_INTEROP_ISSUES_AP
	event_ids[wmi_pdev_interop_issues_ap_event_id] =
						WMI_PDEV_RAP_INFO_EVENTID;
#endif
#ifdef AST_HKV1_WORKAROUND
	event_ids[wmi_wds_peer_event_id] = WMI_WDS_PEER_EVENTID;
#endif
	event_ids[wmi_pdev_ctl_failsafe_check_event_id] =
		WMI_PDEV_CTL_FAILSAFE_CHECK_EVENTID;
	event_ids[wmi_vdev_bcn_reception_stats_event_id] =
		WMI_VDEV_BCN_RECEPTION_STATS_EVENTID;
	event_ids[wmi_roam_blacklist_event_id] = WMI_ROAM_BLACKLIST_EVENTID;
	event_ids[wmi_wlm_stats_event_id] = WMI_WLM_STATS_EVENTID;
	event_ids[wmi_peer_cfr_capture_event_id] = WMI_PEER_CFR_CAPTURE_EVENTID;
	event_ids[wmi_pdev_cold_boot_cal_event_id] =
					    WMI_PDEV_COLD_BOOT_CAL_DATA_EVENTID;
#ifdef WLAN_MWS_INFO_DEBUGFS
	event_ids[wmi_vdev_get_mws_coex_state_eventid] =
			WMI_VDEV_GET_MWS_COEX_STATE_EVENTID;
	event_ids[wmi_vdev_get_mws_coex_dpwb_state_eventid] =
			WMI_VDEV_GET_MWS_COEX_DPWB_STATE_EVENTID;
	event_ids[wmi_vdev_get_mws_coex_tdm_state_eventid] =
			WMI_VDEV_GET_MWS_COEX_TDM_STATE_EVENTID;
	event_ids[wmi_vdev_get_mws_coex_idrx_state_eventid] =
			WMI_VDEV_GET_MWS_COEX_IDRX_STATE_EVENTID;
	event_ids[wmi_vdev_get_mws_coex_antenna_sharing_state_eventid] =
			WMI_VDEV_GET_MWS_COEX_ANTENNA_SHARING_STATE_EVENTID;
#endif
}

/**
 * populate_tlv_service() - populates wmi services
 *
 * @param wmi_service: Pointer to hold wmi_service
 * Return: None
 */
static void populate_tlv_service(uint32_t *wmi_service)
{
	wmi_service[wmi_service_beacon_offload] = WMI_SERVICE_BEACON_OFFLOAD;
	wmi_service[wmi_service_ack_timeout] = WMI_SERVICE_ACK_TIMEOUT;
	wmi_service[wmi_service_scan_offload] = WMI_SERVICE_SCAN_OFFLOAD;
	wmi_service[wmi_service_roam_scan_offload] =
					WMI_SERVICE_ROAM_SCAN_OFFLOAD;
	wmi_service[wmi_service_bcn_miss_offload] =
					WMI_SERVICE_BCN_MISS_OFFLOAD;
	wmi_service[wmi_service_sta_pwrsave] = WMI_SERVICE_STA_PWRSAVE;
	wmi_service[wmi_service_sta_advanced_pwrsave] =
				WMI_SERVICE_STA_ADVANCED_PWRSAVE;
	wmi_service[wmi_service_ap_uapsd] = WMI_SERVICE_AP_UAPSD;
	wmi_service[wmi_service_ap_dfs] = WMI_SERVICE_AP_DFS;
	wmi_service[wmi_service_11ac] = WMI_SERVICE_11AC;
	wmi_service[wmi_service_blockack] = WMI_SERVICE_BLOCKACK;
	wmi_service[wmi_service_phyerr] = WMI_SERVICE_PHYERR;
	wmi_service[wmi_service_bcn_filter] = WMI_SERVICE_BCN_FILTER;
	wmi_service[wmi_service_rtt] = WMI_SERVICE_RTT;
	wmi_service[wmi_service_wow] = WMI_SERVICE_WOW;
	wmi_service[wmi_service_ratectrl_cache] = WMI_SERVICE_RATECTRL_CACHE;
	wmi_service[wmi_service_iram_tids] = WMI_SERVICE_IRAM_TIDS;
	wmi_service[wmi_service_arpns_offload] = WMI_SERVICE_ARPNS_OFFLOAD;
	wmi_service[wmi_service_nlo] = WMI_SERVICE_NLO;
	wmi_service[wmi_service_gtk_offload] = WMI_SERVICE_GTK_OFFLOAD;
	wmi_service[wmi_service_scan_sch] = WMI_SERVICE_SCAN_SCH;
	wmi_service[wmi_service_csa_offload] = WMI_SERVICE_CSA_OFFLOAD;
	wmi_service[wmi_service_chatter] = WMI_SERVICE_CHATTER;
	wmi_service[wmi_service_coex_freqavoid] = WMI_SERVICE_COEX_FREQAVOID;
	wmi_service[wmi_service_packet_power_save] =
					WMI_SERVICE_PACKET_POWER_SAVE;
	wmi_service[wmi_service_force_fw_hang] = WMI_SERVICE_FORCE_FW_HANG;
	wmi_service[wmi_service_gpio] = WMI_SERVICE_GPIO;
	wmi_service[wmi_service_sta_dtim_ps_modulated_dtim] =
				WMI_SERVICE_STA_DTIM_PS_MODULATED_DTIM;
	wmi_service[wmi_sta_uapsd_basic_auto_trig] =
					WMI_STA_UAPSD_BASIC_AUTO_TRIG;
	wmi_service[wmi_sta_uapsd_var_auto_trig] = WMI_STA_UAPSD_VAR_AUTO_TRIG;
	wmi_service[wmi_service_sta_keep_alive] = WMI_SERVICE_STA_KEEP_ALIVE;
	wmi_service[wmi_service_tx_encap] = WMI_SERVICE_TX_ENCAP;
	wmi_service[wmi_service_ap_ps_detect_out_of_sync] =
				WMI_SERVICE_AP_PS_DETECT_OUT_OF_SYNC;
	wmi_service[wmi_service_early_rx] = WMI_SERVICE_EARLY_RX;
	wmi_service[wmi_service_sta_smps] = WMI_SERVICE_STA_SMPS;
	wmi_service[wmi_service_fwtest] = WMI_SERVICE_FWTEST;
	wmi_service[wmi_service_sta_wmmac] = WMI_SERVICE_STA_WMMAC;
	wmi_service[wmi_service_tdls] = WMI_SERVICE_TDLS;
	wmi_service[wmi_service_burst] = WMI_SERVICE_BURST;
	wmi_service[wmi_service_mcc_bcn_interval_change] =
				WMI_SERVICE_MCC_BCN_INTERVAL_CHANGE;
	wmi_service[wmi_service_adaptive_ocs] = WMI_SERVICE_ADAPTIVE_OCS;
	wmi_service[wmi_service_ba_ssn_support] = WMI_SERVICE_BA_SSN_SUPPORT;
	wmi_service[wmi_service_filter_ipsec_natkeepalive] =
				WMI_SERVICE_FILTER_IPSEC_NATKEEPALIVE;
	wmi_service[wmi_service_wlan_hb] = WMI_SERVICE_WLAN_HB;
	wmi_service[wmi_service_lte_ant_share_support] =
				WMI_SERVICE_LTE_ANT_SHARE_SUPPORT;
	wmi_service[wmi_service_batch_scan] = WMI_SERVICE_BATCH_SCAN;
	wmi_service[wmi_service_qpower] = WMI_SERVICE_QPOWER;
	wmi_service[wmi_service_plmreq] = WMI_SERVICE_PLMREQ;
	wmi_service[wmi_service_thermal_mgmt] = WMI_SERVICE_THERMAL_MGMT;
	wmi_service[wmi_service_rmc] = WMI_SERVICE_RMC;
	wmi_service[wmi_service_mhf_offload] = WMI_SERVICE_MHF_OFFLOAD;
	wmi_service[wmi_service_coex_sar] = WMI_SERVICE_COEX_SAR;
	wmi_service[wmi_service_bcn_txrate_override] =
				WMI_SERVICE_BCN_TXRATE_OVERRIDE;
	wmi_service[wmi_service_nan] = WMI_SERVICE_NAN;
	wmi_service[wmi_service_l1ss_stat] = WMI_SERVICE_L1SS_STAT;
	wmi_service[wmi_service_estimate_linkspeed] =
				WMI_SERVICE_ESTIMATE_LINKSPEED;
	wmi_service[wmi_service_obss_scan] = WMI_SERVICE_OBSS_SCAN;
	wmi_service[wmi_service_tdls_offchan] = WMI_SERVICE_TDLS_OFFCHAN;
	wmi_service[wmi_service_tdls_uapsd_buffer_sta] =
				WMI_SERVICE_TDLS_UAPSD_BUFFER_STA;
	wmi_service[wmi_service_tdls_uapsd_sleep_sta] =
				WMI_SERVICE_TDLS_UAPSD_SLEEP_STA;
	wmi_service[wmi_service_ibss_pwrsave] = WMI_SERVICE_IBSS_PWRSAVE;
	wmi_service[wmi_service_lpass] = WMI_SERVICE_LPASS;
	wmi_service[wmi_service_extscan] = WMI_SERVICE_EXTSCAN;
	wmi_service[wmi_service_d0wow] = WMI_SERVICE_D0WOW;
	wmi_service[wmi_service_hsoffload] = WMI_SERVICE_HSOFFLOAD;
	wmi_service[wmi_service_roam_ho_offload] = WMI_SERVICE_ROAM_HO_OFFLOAD;
	wmi_service[wmi_service_rx_full_reorder] = WMI_SERVICE_RX_FULL_REORDER;
	wmi_service[wmi_service_dhcp_offload] = WMI_SERVICE_DHCP_OFFLOAD;
	wmi_service[wmi_service_sta_rx_ipa_offload_support] =
				WMI_SERVICE_STA_RX_IPA_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_mdns_offload] = WMI_SERVICE_MDNS_OFFLOAD;
	wmi_service[wmi_service_sap_auth_offload] =
					WMI_SERVICE_SAP_AUTH_OFFLOAD;
	wmi_service[wmi_service_dual_band_simultaneous_support] =
				WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT;
	wmi_service[wmi_service_ocb] = WMI_SERVICE_OCB;
	wmi_service[wmi_service_ap_arpns_offload] =
					WMI_SERVICE_AP_ARPNS_OFFLOAD;
	wmi_service[wmi_service_per_band_chainmask_support] =
				WMI_SERVICE_PER_BAND_CHAINMASK_SUPPORT;
	wmi_service[wmi_service_packet_filter_offload] =
				WMI_SERVICE_PACKET_FILTER_OFFLOAD;
	wmi_service[wmi_service_mgmt_tx_htt] = WMI_SERVICE_MGMT_TX_HTT;
	wmi_service[wmi_service_mgmt_tx_wmi] = WMI_SERVICE_MGMT_TX_WMI;
	wmi_service[wmi_service_ext_msg] = WMI_SERVICE_EXT_MSG;
	wmi_service[wmi_service_mawc] = WMI_SERVICE_MAWC;
	wmi_service[wmi_service_multiple_vdev_restart] =
			WMI_SERVICE_MULTIPLE_VDEV_RESTART;

	wmi_service[wmi_service_roam_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ratectrl] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_smart_antenna_sw_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_smart_antenna_hw_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_enhanced_proxy_sta] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tt] = WMI_SERVICE_THERM_THROT;
	wmi_service[wmi_service_atf] = WMI_SERVICE_ATF;
	wmi_service[wmi_service_peer_caching] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_coex_gpio] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_aux_spectral_intf] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_aux_chan_load_intf] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_bss_channel_info_64] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ext_res_cfg_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mesh] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_restrt_chnl_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_peer_stats] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mesh_11s] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_periodic_chan_stat_support] =
			WMI_SERVICE_PERIODIC_CHAN_STAT_SUPPORT;
	wmi_service[wmi_service_tx_mode_push_only] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tx_mode_push_pull] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tx_mode_dynamic] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_btcoex_duty_cycle] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_4_wire_coex_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mesh] = WMI_SERVICE_ENTERPRISE_MESH;
	wmi_service[wmi_service_peer_assoc_conf] = WMI_SERVICE_PEER_ASSOC_CONF;
	wmi_service[wmi_service_egap] = WMI_SERVICE_EGAP;
	wmi_service[wmi_service_sta_pmf_offload] = WMI_SERVICE_STA_PMF_OFFLOAD;
	wmi_service[wmi_service_unified_wow_capability] =
				WMI_SERVICE_UNIFIED_WOW_CAPABILITY;
	wmi_service[wmi_service_enterprise_mesh] = WMI_SERVICE_ENTERPRISE_MESH;
	wmi_service[wmi_service_apf_offload] = WMI_SERVICE_BPF_OFFLOAD;
	wmi_service[wmi_service_sync_delete_cmds] =
				WMI_SERVICE_SYNC_DELETE_CMDS;
	wmi_service[wmi_service_ratectrl_limit_max_min_rates] =
				WMI_SERVICE_RATECTRL_LIMIT_MAX_MIN_RATES;
	wmi_service[wmi_service_nan_data] = WMI_SERVICE_NAN_DATA;
	wmi_service[wmi_service_nan_rtt] = WMI_SERVICE_NAN_RTT;
	wmi_service[wmi_service_11ax] = WMI_SERVICE_11AX;
	wmi_service[wmi_service_deprecated_replace] =
				WMI_SERVICE_DEPRECATED_REPLACE;
	wmi_service[wmi_service_tdls_conn_tracker_in_host_mode] =
				WMI_SERVICE_TDLS_CONN_TRACKER_IN_HOST_MODE;
	wmi_service[wmi_service_enhanced_mcast_filter] =
				WMI_SERVICE_ENHANCED_MCAST_FILTER;
	wmi_service[wmi_service_half_rate_quarter_rate_support] =
				WMI_SERVICE_HALF_RATE_QUARTER_RATE_SUPPORT;
	wmi_service[wmi_service_vdev_rx_filter] = WMI_SERVICE_VDEV_RX_FILTER;
	wmi_service[wmi_service_p2p_listen_offload_support] =
				WMI_SERVICE_P2P_LISTEN_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_mark_first_wakeup_packet] =
				WMI_SERVICE_MARK_FIRST_WAKEUP_PACKET;
	wmi_service[wmi_service_multiple_mcast_filter_set] =
				WMI_SERVICE_MULTIPLE_MCAST_FILTER_SET;
	wmi_service[wmi_service_host_managed_rx_reorder] =
				WMI_SERVICE_HOST_MANAGED_RX_REORDER;
	wmi_service[wmi_service_flash_rdwr_support] =
				WMI_SERVICE_FLASH_RDWR_SUPPORT;
	wmi_service[wmi_service_wlan_stats_report] =
				WMI_SERVICE_WLAN_STATS_REPORT;
	wmi_service[wmi_service_tx_msdu_id_new_partition_support] =
				WMI_SERVICE_TX_MSDU_ID_NEW_PARTITION_SUPPORT;
	wmi_service[wmi_service_dfs_phyerr_offload] =
				WMI_SERVICE_DFS_PHYERR_OFFLOAD;
	wmi_service[wmi_service_rcpi_support] = WMI_SERVICE_RCPI_SUPPORT;
	wmi_service[wmi_service_fw_mem_dump_support] =
				WMI_SERVICE_FW_MEM_DUMP_SUPPORT;
	wmi_service[wmi_service_peer_stats_info] = WMI_SERVICE_PEER_STATS_INFO;
	wmi_service[wmi_service_regulatory_db] = WMI_SERVICE_REGULATORY_DB;
	wmi_service[wmi_service_11d_offload] = WMI_SERVICE_11D_OFFLOAD;
	wmi_service[wmi_service_hw_data_filtering] =
				WMI_SERVICE_HW_DATA_FILTERING;
	wmi_service[wmi_service_pkt_routing] = WMI_SERVICE_PKT_ROUTING;
	wmi_service[wmi_service_offchan_tx_wmi] = WMI_SERVICE_OFFCHAN_TX_WMI;
	wmi_service[wmi_service_chan_load_info] = WMI_SERVICE_CHAN_LOAD_INFO;
	wmi_service[wmi_service_extended_nss_support] =
				WMI_SERVICE_EXTENDED_NSS_SUPPORT;
	wmi_service[wmi_service_widebw_scan] = WMI_SERVICE_SCAN_PHYMODE_SUPPORT;
	wmi_service[wmi_service_bcn_offload_start_stop_support] =
				WMI_SERVICE_BCN_OFFLOAD_START_STOP_SUPPORT;
	wmi_service[wmi_service_offchan_data_tid_support] =
				WMI_SERVICE_OFFCHAN_DATA_TID_SUPPORT;
	wmi_service[wmi_service_support_dma] =
				WMI_SERVICE_SUPPORT_DIRECT_DMA;
	wmi_service[wmi_service_8ss_tx_bfee] = WMI_SERVICE_8SS_TX_BFEE;
	wmi_service[wmi_service_fils_support] = WMI_SERVICE_FILS_SUPPORT;
	wmi_service[wmi_service_mawc_support] = WMI_SERVICE_MAWC_SUPPORT;
	wmi_service[wmi_service_wow_wakeup_by_timer_pattern] =
				WMI_SERVICE_WOW_WAKEUP_BY_TIMER_PATTERN;
	wmi_service[wmi_service_11k_neighbour_report_support] =
				WMI_SERVICE_11K_NEIGHBOUR_REPORT_SUPPORT;
	wmi_service[wmi_service_ap_obss_detection_offload] =
				WMI_SERVICE_AP_OBSS_DETECTION_OFFLOAD;
	wmi_service[wmi_service_bss_color_offload] =
				WMI_SERVICE_BSS_COLOR_OFFLOAD;
	wmi_service[wmi_service_gmac_offload_support] =
				WMI_SERVICE_GMAC_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_dual_beacon_on_single_mac_scc_support] =
			WMI_SERVICE_DUAL_BEACON_ON_SINGLE_MAC_SCC_SUPPORT;
	wmi_service[wmi_service_dual_beacon_on_single_mac_mcc_support] =
			WMI_SERVICE_DUAL_BEACON_ON_SINGLE_MAC_MCC_SUPPORT;
	wmi_service[wmi_service_twt_requestor] = WMI_SERVICE_STA_TWT;
	wmi_service[wmi_service_twt_responder] = WMI_SERVICE_AP_TWT;
	wmi_service[wmi_service_listen_interval_offload_support] =
			WMI_SERVICE_LISTEN_INTERVAL_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_esp_support] = WMI_SERVICE_ESP_SUPPORT;
	wmi_service[wmi_service_obss_spatial_reuse] =
			WMI_SERVICE_OBSS_SPATIAL_REUSE;
	wmi_service[wmi_service_per_vdev_chain_support] =
			WMI_SERVICE_PER_VDEV_CHAINMASK_CONFIG_SUPPORT;
	wmi_service[wmi_service_new_htt_msg_format] =
			WMI_SERVICE_HTT_H2T_NO_HTC_HDR_LEN_IN_MSG_LEN;
	wmi_service[wmi_service_peer_unmap_cnf_support] =
			WMI_SERVICE_PEER_UNMAP_RESPONSE_SUPPORT;
	wmi_service[wmi_service_beacon_reception_stats] =
			WMI_SERVICE_BEACON_RECEPTION_STATS;
	wmi_service[wmi_service_vdev_latency_config] =
			WMI_SERVICE_VDEV_LATENCY_CONFIG;
	wmi_service[wmi_service_nan_dbs_support] = WMI_SERVICE_NAN_DBS_SUPPORT;
	wmi_service[wmi_service_ndi_dbs_support] = WMI_SERVICE_NDI_DBS_SUPPORT;
	wmi_service[wmi_service_nan_sap_support] = WMI_SERVICE_NAN_SAP_SUPPORT;
	wmi_service[wmi_service_ndi_sap_support] = WMI_SERVICE_NDI_SAP_SUPPORT;
	wmi_service[wmi_service_nan_disable_support] =
			WMI_SERVICE_NAN_DISABLE_SUPPORT;
	wmi_service[wmi_service_sta_plus_sta_support] =
				WMI_SERVICE_STA_PLUS_STA_SUPPORT;
	wmi_service[wmi_service_hw_db2dbm_support] =
			WMI_SERVICE_HW_DB2DBM_CONVERSION_SUPPORT;
	wmi_service[wmi_service_wlm_stats_support] =
			WMI_SERVICE_WLM_STATS_REQUEST;
	wmi_service[wmi_service_infra_mbssid] = WMI_SERVICE_INFRA_MBSSID;
	wmi_service[wmi_service_ul_ru26_allowed] = WMI_SERVICE_UL_RU26_ALLOWED;
	wmi_service[wmi_service_cfr_capture_support] =
			WMI_SERVICE_CFR_CAPTURE_SUPPORT;
	wmi_service[wmi_service_bcast_twt_support] =
			WMI_SERVICE_BROADCAST_TWT;
	wmi_service[wmi_service_wpa3_ft_sae_support] =
			WMI_SERVICE_WPA3_FT_SAE_SUPPORT;
	wmi_service[wmi_service_wpa3_ft_suite_b_support] =
			WMI_SERVICE_WPA3_FT_SUITE_B_SUPPORT;
	wmi_service[wmi_service_ft_fils] =
			WMI_SERVICE_WPA3_FT_FILS;
	wmi_service[wmi_service_adaptive_11r_support] =
			WMI_SERVICE_ADAPTIVE_11R_ROAM;
	wmi_service[wmi_service_tx_compl_tsf64] =
			WMI_SERVICE_TX_COMPL_TSF64;
	wmi_service[wmi_service_data_stall_recovery_support] =
			WMI_SERVICE_DSM_ROAM_FILTER;
}

/**
 * wmi_ocb_ut_attach() - Attach OCB test framework
 * @wmi_handle: wmi handle
 *
 * Return: None
 */
#ifdef WLAN_OCB_UT
void wmi_ocb_ut_attach(struct wmi_unified *wmi_handle);
#else
static inline void wmi_ocb_ut_attach(struct wmi_unified *wmi_handle)
{
	return;
}
#endif

/**
 * wmi_tlv_attach() - Attach TLV APIs
 *
 * Return: None
 */
void wmi_tlv_attach(wmi_unified_t wmi_handle)
{
	wmi_handle->ops = &tlv_ops;
	wmi_ocb_ut_attach(wmi_handle);
	wmi_handle->soc->svc_ids = &multi_svc_ids[0];
#ifdef WMI_INTERFACE_EVENT_LOGGING
	/* Skip saving WMI_CMD_HDR and TLV HDR */
	wmi_handle->soc->buf_offset_command = 8;
	/* WMI_CMD_HDR is already stripped, skip saving TLV HDR */
	wmi_handle->soc->buf_offset_event = 4;
#endif
	populate_tlv_events_id(wmi_handle->wmi_events);
	populate_tlv_service(wmi_handle->services);
	wmi_twt_attach_tlv(wmi_handle);
	wmi_extscan_attach_tlv(wmi_handle);
	wmi_smart_ant_attach_tlv(wmi_handle);
	wmi_dbr_attach_tlv(wmi_handle);
	wmi_atf_attach_tlv(wmi_handle);
	wmi_ap_attach_tlv(wmi_handle);
	wmi_ocb_attach_tlv(wmi_handle);
	wmi_nan_attach_tlv(wmi_handle);
	wmi_p2p_attach_tlv(wmi_handle);
	wmi_interop_issues_ap_attach_tlv(wmi_handle);
	wmi_roam_attach_tlv(wmi_handle);
	wmi_concurrency_attach_tlv(wmi_handle);
	wmi_pmo_attach_tlv(wmi_handle);
	wmi_sta_attach_tlv(wmi_handle);
	wmi_11ax_bss_color_attach_tlv(wmi_handle);
}
qdf_export_symbol(wmi_tlv_attach);

/**
 * wmi_tlv_init() - Initialize WMI TLV module by registering TLV attach routine
 *
 * Return: None
 */
void wmi_tlv_init(void)
{
	wmi_unified_register_module(WMI_TLV_TARGET, &wmi_tlv_attach);
}
