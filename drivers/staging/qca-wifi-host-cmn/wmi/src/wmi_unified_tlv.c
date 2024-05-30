/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wmi_unified_wds_api.h"

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

#include <wmi_unified_vdev_api.h>
#include <wmi_unified_vdev_tlv.h>
#include <wmi_unified_11be_tlv.h>

/*
 * If FW supports WMI_SERVICE_SCAN_CONFIG_PER_CHANNEL,
 * then channel_list may fill the upper 12 bits with channel flags,
 * while using only the lower 20 bits for channel frequency.
 * If FW doesn't support WMI_SERVICE_SCAN_CONFIG_PER_CHANNEL,
 * then channel_list only holds the frequency value.
 */
#define CHAN_LIST_FLAG_MASK_POS 20
#define TARGET_SET_FREQ_IN_CHAN_LIST_TLV(buf, freq) \
			((buf) |= ((freq) & WMI_SCAN_CHANNEL_FREQ_MASK))
#define TARGET_SET_FLAGS_IN_CHAN_LIST_TLV(buf, flags) \
			((buf) |= ((flags) << CHAN_LIST_FLAG_MASK_POS))

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
	[wmi_pdev_param_soft_tx_chain_mask] = WMI_PDEV_PARAM_TX_CHAIN_MASK,
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
	[wmi_pdev_param_nav_override_config] =
			WMI_PDEV_PARAM_NAV_OVERRIDE_CONFIG,
	[wmi_pdev_param_set_mgmt_ttl] = WMI_PDEV_PARAM_SET_MGMT_TTL,
	[wmi_pdev_param_set_prb_rsp_ttl] =
			WMI_PDEV_PARAM_SET_PROBE_RESP_TTL,
	[wmi_pdev_param_set_mu_ppdu_duration] =
			WMI_PDEV_PARAM_SET_MU_PPDU_DURATION,
	[wmi_pdev_param_set_tbtt_ctrl] =
			WMI_PDEV_PARAM_SET_TBTT_CTRL,
	[wmi_pdev_param_set_cmd_obss_pd_threshold] =
			WMI_PDEV_PARAM_SET_CMD_OBSS_PD_THRESHOLD,
	[wmi_pdev_param_set_cmd_obss_pd_per_ac] =
			WMI_PDEV_PARAM_SET_CMD_OBSS_PD_PER_AC,
	[wmi_pdev_param_set_cong_ctrl_max_msdus] =
			WMI_PDEV_PARAM_SET_CONG_CTRL_MAX_MSDUS,
	[wmi_pdev_param_enable_fw_dynamic_he_edca] =
			WMI_PDEV_PARAM_ENABLE_FW_DYNAMIC_HE_EDCA,
	[wmi_pdev_param_enable_srp] = WMI_PDEV_PARAM_ENABLE_SRP,
	[wmi_pdev_param_enable_sr_prohibit] = WMI_PDEV_PARAM_ENABLE_SR_PROHIBIT,
	[wmi_pdev_param_sr_trigger_margin] = WMI_PDEV_PARAM_SR_TRIGGER_MARGIN,
	[wmi_pdev_param_pream_punct_bw] = WMI_PDEV_PARAM_SET_PREAM_PUNCT_BW,
	[wmi_pdev_param_enable_mbssid_ctrl_frame] = WMI_PDEV_PARAM_ENABLE_MBSSID_CTRL_FRAME,
	[wmi_pdev_param_set_mesh_params] = WMI_PDEV_PARAM_SET_MESH_PARAMS,
	[wmi_pdev_param_mpd_userpd_ssr] = WMI_PDEV_PARAM_MPD_USERPD_SSR,
	[wmi_pdev_param_low_latency_mode] =
			WMI_PDEV_PARAM_LOW_LATENCY_SCHED_MODE,
	[wmi_pdev_param_scan_radio_tx_on_dfs] =
					WMI_PDEV_PARAM_SCAN_RADIO_TX_ON_DFS,
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
#ifdef WLAN_FEATURE_11BE
	[wmi_vdev_param_set_ehtop] = WMI_VDEV_PARAM_EHTOPS_0_31,
	[wmi_vdev_param_set_eht_mu_mode] = WMI_VDEV_PARAM_SET_EHT_MU_MODE,
	[wmi_vdev_param_set_eht_puncturing_mode] =
					WMI_VDEV_PARAM_SET_EHT_PUNCTURING_MODE,
	[wmi_vdev_param_set_eht_ltf] = WMI_VDEV_PARAM_EHT_LTF,
	[wmi_vdev_param_set_ul_eht_ltf] = WMI_VDEV_PARAM_UL_EHT_LTF,
	[wmi_vdev_param_set_eht_dcm] = WMI_VDEV_PARAM_EHT_DCM,
	[wmi_vdev_param_set_eht_range_ext] = WMI_VDEV_PARAM_EHT_RANGE_EXT,
	[wmi_vdev_param_set_non_data_eht_range_ext] =
					WMI_VDEV_PARAM_NON_DATA_EHT_RANGE_EXT,
#endif
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
	[wmi_vdev_param_max_mtu_size] = WMI_VDEV_PARAM_MAX_MTU_SIZE,
	[wmi_vdev_param_mcast_rc_stale_period] =
					WMI_VDEV_PARAM_MCAST_RC_STALE_PERIOD,
	[wmi_vdev_param_enable_multi_group_key] =
				WMI_VDEV_PARAM_ENABLE_MULTI_GROUP_KEY,
	[wmi_vdev_param_max_group_keys] = WMI_VDEV_PARAM_NUM_GROUP_KEYS,
	[wmi_vdev_param_enable_mcast_rc] = WMI_VDEV_PARAM_ENABLE_MCAST_RC,
	[wmi_vdev_param_6ghz_params] = WMI_VDEV_PARAM_6GHZ_PARAMS,
	[wmi_vdev_param_enable_disable_roam_reason_vsie] =
				WMI_VDEV_PARAM_ENABLE_DISABLE_ROAM_REASON_VSIE,
	[wmi_vdev_param_set_cmd_obss_pd_threshold] =
			WMI_VDEV_PARAM_SET_CMD_OBSS_PD_THRESHOLD,
	[wmi_vdev_param_set_cmd_obss_pd_per_ac] =
			WMI_VDEV_PARAM_SET_CMD_OBSS_PD_PER_AC,
	[wmi_vdev_param_enable_srp] = WMI_VDEV_PARAM_ENABLE_SRP,
	[wmi_vdev_param_nan_config_features] =
			WMI_VDEV_PARAM_ENABLE_DISABLE_NAN_CONFIG_FEATURES,
	[wmi_vdev_param_enable_disable_rtt_responder_role] =
			WMI_VDEV_PARAM_ENABLE_DISABLE_RTT_RESPONDER_ROLE,
	[wmi_vdev_param_enable_disable_rtt_initiator_role] =
			WMI_VDEV_PARAM_ENABLE_DISABLE_RTT_INITIATOR_ROLE,
	[wmi_vdev_param_mcast_steer] = WMI_VDEV_PARAM_MCAST_STEERING,
#ifdef MULTI_CLIENT_LL_SUPPORT
	[wmi_vdev_param_set_normal_latency_flags_config] =
			WMI_VDEV_PARAM_NORMAL_LATENCY_FLAGS_CONFIGURATION,
	[wmi_vdev_param_set_xr_latency_flags_config] =
			WMI_VDEV_PARAM_XR_LATENCY_FLAGS_CONFIGURATION,
	[wmi_vdev_param_set_low_latency_flags_config] =
			WMI_VDEV_PARAM_LOW_LATENCY_FLAGS_CONFIGURATION,
	[wmi_vdev_param_set_ultra_low_latency_flags_config] =
			WMI_VDEV_PARAM_ULTRA_LOW_LATENCY_FLAGS_CONFIGURATION,
	[wmi_vdev_param_set_normal_latency_ul_dl_config] =
			WMI_VDEV_PARAM_NORMAL_LATENCY_UL_DL_CONFIGURATION,
	[wmi_vdev_param_set_xr_latency_ul_dl_config] =
			WMI_VDEV_PARAM_XR_LATENCY_UL_DL_CONFIGURATION,
	[wmi_vdev_param_set_low_latency_ul_dl_config] =
			WMI_VDEV_PARAM_LOW_LATENCY_UL_DL_CONFIGURATION,
	[wmi_vdev_param_set_ultra_low_latency_ul_dl_config] =
			WMI_VDEV_PARAM_ULTRA_LOW_LATENCY_UL_DL_CONFIGURATION,
	[wmi_vdev_param_set_default_ll_config] =
			WMI_VDEV_PARAM_DEFAULT_LATENCY_LEVEL_CONFIGURATION,
	[wmi_vdev_param_set_multi_client_ll_feature_config] =
			WMI_VDEV_PARAM_MULTI_CLIENT_LL_FEATURE_CONFIGURATION,
#endif
};
#endif

#ifndef WMI_PKTLOG_EVENT_CBF
#define WMI_PKTLOG_EVENT_CBF 0x100
#endif

/**
 * Populate the pktlog event tlv array, where
 * the values are the FW WMI events, which host
 * uses to communicate with FW for pktlog
 */

static const uint32_t pktlog_event_tlv[] =  {
	[WMI_HOST_PKTLOG_EVENT_RX_BIT] = WMI_PKTLOG_EVENT_RX,
	[WMI_HOST_PKTLOG_EVENT_TX_BIT] = WMI_PKTLOG_EVENT_TX,
	[WMI_HOST_PKTLOG_EVENT_RCF_BIT] = WMI_PKTLOG_EVENT_RCF,
	[WMI_HOST_PKTLOG_EVENT_RCU_BIT] = WMI_PKTLOG_EVENT_RCU,
	[WMI_HOST_PKTLOG_EVENT_DBG_PRINT_BIT] = 0,
	[WMI_HOST_PKTLOG_EVENT_SMART_ANTENNA_BIT] =
		WMI_PKTLOG_EVENT_SMART_ANTENNA,
	[WMI_HOST_PKTLOG_EVENT_H_INFO_BIT] = 0,
	[WMI_HOST_PKTLOG_EVENT_STEERING_BIT] = 0,
	[WMI_HOST_PKTLOG_EVENT_TX_DATA_CAPTURE_BIT] = 0,
	[WMI_HOST_PKTLOG_EVENT_PHY_LOGGING_BIT] = WMI_PKTLOG_EVENT_PHY,
	[WMI_HOST_PKTLOG_EVENT_CBF_BIT] = WMI_PKTLOG_EVENT_CBF,
#ifdef QCA_WIFI_QCN9224
	[WMI_HOST_PKTLOG_EVENT_HYBRID_TX_BIT] = WMI_PKTLOG_EVENT_HYBRID_TX,
#endif
};

/**
 * convert_host_pdev_id_to_target_pdev_id() - Convert pdev_id from
 *	   host to target defines.
 * @wmi_handle: pointer to wmi_handle
 * @param pdev_id: host pdev_id to be converted.
 * Return: target pdev_id after conversion.
 */
static uint32_t convert_host_pdev_id_to_target_pdev_id(wmi_unified_t wmi_handle,
						       uint32_t pdev_id)
{
	if (pdev_id <= WMI_HOST_PDEV_ID_2 && pdev_id >= WMI_HOST_PDEV_ID_0) {
		if (!wmi_handle->soc->is_pdev_is_map_enable) {
			switch (pdev_id) {
			case WMI_HOST_PDEV_ID_0:
				return WMI_PDEV_ID_1ST;
			case WMI_HOST_PDEV_ID_1:
				return WMI_PDEV_ID_2ND;
			case WMI_HOST_PDEV_ID_2:
				return WMI_PDEV_ID_3RD;
			}
		} else {
			return wmi_handle->cmd_pdev_id_map[pdev_id];
		}
	} else {
		return WMI_PDEV_ID_SOC;
	}

	QDF_ASSERT(0);

	return WMI_PDEV_ID_SOC;
}

/**
 * convert_target_pdev_id_to_host_pdev_id() - Convert pdev_id from
 *	   target to host defines.
 * @wmi_handle: pointer to wmi_handle
 * @param pdev_id: target pdev_id to be converted.
 * Return: host pdev_id after conversion.
 */
static uint32_t convert_target_pdev_id_to_host_pdev_id(wmi_unified_t wmi_handle,
						       uint32_t pdev_id)
{

	if (pdev_id <= WMI_PDEV_ID_3RD && pdev_id >= WMI_PDEV_ID_1ST) {
		if (!wmi_handle->soc->is_pdev_is_map_enable) {
			switch (pdev_id) {
			case WMI_PDEV_ID_1ST:
				return WMI_HOST_PDEV_ID_0;
			case WMI_PDEV_ID_2ND:
				return WMI_HOST_PDEV_ID_1;
			case WMI_PDEV_ID_3RD:
				return WMI_HOST_PDEV_ID_2;
			}
		} else {
			return wmi_handle->evt_pdev_id_map[pdev_id - 1];
		}
	} else if (pdev_id == WMI_PDEV_ID_SOC) {
		return WMI_HOST_PDEV_ID_SOC;
	} else {
		wmi_err("Invalid pdev_id");
	}

	return WMI_HOST_PDEV_ID_INVALID;
}

/**
 * convert_host_phy_id_to_target_phy_id() - Convert phy_id from
 *	   host to target defines.
 * @wmi_handle: pointer to wmi_handle
 * @param phy_id: host pdev_id to be converted.
 * Return: target phy_id after conversion.
 */
static uint32_t convert_host_phy_id_to_target_phy_id(wmi_unified_t wmi_handle,
						     uint32_t phy_id)
{
	if (!wmi_handle->soc->is_phy_id_map_enable ||
	    phy_id >= WMI_MAX_RADIOS) {
		return phy_id;
	}

	return wmi_handle->cmd_phy_id_map[phy_id];
}

/**
 * convert_target_phy_id_to_host_phy_id() - Convert phy_id from
 *	   target to host defines.
 * @wmi_handle: pointer to wmi_handle
 * @param phy_id: target phy_id to be converted.
 * Return: host phy_id after conversion.
 */
static uint32_t convert_target_phy_id_to_host_phy_id(wmi_unified_t wmi_handle,
						     uint32_t phy_id)
{
	if (!wmi_handle->soc->is_phy_id_map_enable ||
	    phy_id >= WMI_MAX_RADIOS) {
		return phy_id;
	}

	return wmi_handle->evt_phy_id_map[phy_id];
}

/**
 * wmi_tlv_pdev_id_conversion_enable() - Enable pdev_id conversion
 *
 * Return None.
 */
static void wmi_tlv_pdev_id_conversion_enable(wmi_unified_t wmi_handle,
					      uint32_t *pdev_id_map,
					      uint8_t size)
{
	int i = 0;

	if (pdev_id_map && (size <= WMI_MAX_RADIOS)) {
		for (i = 0; i < size; i++) {
			wmi_handle->cmd_pdev_id_map[i] = pdev_id_map[i];
			wmi_handle->evt_pdev_id_map[i] =
				WMI_HOST_PDEV_ID_INVALID;
			wmi_handle->cmd_phy_id_map[i] = pdev_id_map[i] - 1;
			wmi_handle->evt_phy_id_map[i] =
				WMI_HOST_PDEV_ID_INVALID;
		}

		for (i = 0; i < size; i++) {
			if (wmi_handle->cmd_pdev_id_map[i] !=
					WMI_HOST_PDEV_ID_INVALID) {
				wmi_handle->evt_pdev_id_map
				[wmi_handle->cmd_pdev_id_map[i] - 1] = i;
			}
			if (wmi_handle->cmd_phy_id_map[i] !=
					WMI_HOST_PDEV_ID_INVALID) {
				wmi_handle->evt_phy_id_map
					[wmi_handle->cmd_phy_id_map[i]] = i;
			}
		}
		wmi_handle->soc->is_pdev_is_map_enable = true;
		wmi_handle->soc->is_phy_id_map_enable = true;
	} else {
		wmi_handle->soc->is_pdev_is_map_enable = false;
		wmi_handle->soc->is_phy_id_map_enable = false;
	}

	wmi_handle->ops->convert_pdev_id_host_to_target =
		convert_host_pdev_id_to_target_pdev_id;
	wmi_handle->ops->convert_pdev_id_target_to_host =
		convert_target_pdev_id_to_host_pdev_id;

	/* phy_id convert function assignments */
	wmi_handle->ops->convert_phy_id_host_to_target =
		convert_host_phy_id_to_target_phy_id;
	wmi_handle->ops->convert_phy_id_target_to_host =
		convert_target_phy_id_to_host_phy_id;
}

/* copy_vdev_create_pdev_id() - copy pdev from host params to target command
 *			      buffer.
 * @wmi_handle: pointer to wmi_handle
 * @cmd: pointer target vdev create command buffer
 * @param: pointer host params for vdev create
 *
 * Return: None
 */
static inline void copy_vdev_create_pdev_id(
		struct wmi_unified *wmi_handle,
		wmi_vdev_create_cmd_fixed_param * cmd,
		struct vdev_create_params *param)
{
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							param->pdev_id);
}

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

QDF_STATUS wmi_unified_cmd_send_pm_chk(struct wmi_unified *wmi_handle,
				       wmi_buf_t buf,
				       uint32_t buflen, uint32_t cmd_id,
				       bool is_qmi_send_support)
{
	if (!is_qmi_send_support)
		goto send_over_wmi;

	if (!wmi_is_qmi_stats_enabled(wmi_handle))
		goto send_over_wmi;

	if (wmi_is_target_suspend_acked(wmi_handle)) {
		if (QDF_IS_STATUS_SUCCESS(
		    wmi_unified_cmd_send_over_qmi(wmi_handle, buf,
						  buflen, cmd_id)))
			return QDF_STATUS_SUCCESS;
	}

send_over_wmi:
	qdf_atomic_set(&wmi_handle->num_stats_over_qmi, 0);

	return wmi_unified_cmd_send(wmi_handle, buf, buflen, cmd_id);
}

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
	len += vdev_create_mlo_params_size(param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_create_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->vdev_type = param->type;
	cmd->vdev_subtype = param->subtype;
	cmd->flags = param->mbssid_flags;
	cmd->flags |= (param->special_vdev_mode ? VDEV_FLAGS_SCAN_MODE_VAP : 0);
	cmd->vdevid_trans = param->vdevid_trans;
	cmd->num_cfg_txrx_streams = num_bands;
#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
	cmd->vdev_stats_id_valid = param->vdev_stats_id_valid;
	cmd->vdev_stats_id = param->vdev_stats_id;
#endif
	copy_vdev_create_pdev_id(wmi_handle, cmd, param);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	wmi_debug("ID = %d[pdev:%d] VAP Addr = "QDF_MAC_ADDR_FMT,
		 param->vdev_id, cmd->pdev_id,
		 QDF_MAC_ADDR_REF(macaddr));
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			(num_bands * sizeof(wmi_vdev_txrx_streams)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	wmi_debug("type %d, subtype %d, nss_2g %d, nss_5g %d",
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

	buf_ptr += (num_bands * sizeof(wmi_vdev_txrx_streams));
	buf_ptr = vdev_create_add_mlo_params(buf_ptr, param);

	wmi_mtrace(WMI_VDEV_CREATE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_CREATE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI_VDEV_CREATE_CMDID");
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
		wmi_err("Failed to send WMI_VDEV_DELETE_CMDID");
		wmi_buf_free(buf);
	}
	wmi_debug("vdev id = %d", if_id);

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
		wmi_err("Failed to send WMI_VDEV_CHAINMASK_CONFIG_CMDID");
		wmi_buf_free(buf);
	}
	wmi_debug("vdev_id %d", vdev_id);

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
		wmi_err("Failed to send vdev stop command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debug("vdev id = %d", vdev_id);

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
		wmi_err("Failed to send vdev down");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debug("vdev_id %d", vdev_id);

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

	if (req->channel.is_stadfs_en)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_STA_DFS);

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
 * vdev_start_cmd_fill_11be() - 11be information fiiling in vdev_ststart
 * @cmd: wmi cmd
 * @req: vdev start params
 *
 * Return: QDF status
 */
#ifdef WLAN_FEATURE_11BE
static void
vdev_start_cmd_fill_11be(wmi_vdev_start_request_cmd_fixed_param *cmd,
			 struct vdev_start_params *req)
{
	cmd->eht_ops = req->eht_ops;
	cmd->puncture_20mhz_bitmap = ~req->channel.puncture_bitmap;
	wmi_info("EHT ops: %x puncture_bitmap %x wmi cmd puncture bitmap %x",
		 req->eht_ops, req->channel.puncture_bitmap,
		 cmd->puncture_20mhz_bitmap);
}
#else
static void
vdev_start_cmd_fill_11be(wmi_vdev_start_request_cmd_fixed_param *cmd,
			 struct vdev_start_params *req)
{
}
#endif

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
	if (!req->is_restart)
		len += vdev_start_mlo_params_size(req);
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
	cmd->beacon_interval = req->beacon_interval;
	cmd->dtim_period = req->dtim_period;

	cmd->bcn_tx_rate = req->bcn_tx_rate_code;
	if (req->bcn_tx_rate_code)
		wmi_enable_bcn_ratecode(&cmd->flags);

	if (!req->is_restart) {
		if (req->pmf_enabled)
			cmd->flags |= WMI_UNIFIED_VDEV_START_PMF_ENABLED;

		cmd->mbss_capability_flags = req->mbssid_flags;
		cmd->vdevid_trans = req->vdevid_trans;
	}

	/* Copy the SSID */
	if (req->ssid.length) {
		if (req->ssid.length < sizeof(cmd->ssid.ssid))
			cmd->ssid.ssid_len = req->ssid.length;
		else
			cmd->ssid.ssid_len = sizeof(cmd->ssid.ssid);
		qdf_mem_copy(cmd->ssid.ssid, req->ssid.ssid,
			     cmd->ssid.ssid_len);
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
	if (!req->is_restart) {
		buf_ptr += WMI_TLV_HDR_SIZE +
			   (cmd->num_noa_descriptors * sizeof(wmi_p2p_noa_descriptor));

		buf_ptr = vdev_start_add_mlo_params(buf_ptr, req);
		buf_ptr = vdev_start_add_ml_partner_links(buf_ptr, req);
	}
	wmi_info("vdev_id %d freq %d chanmode %d ch_info: 0x%x is_dfs %d "
		 "beacon interval %d dtim %d center_chan %d center_freq2 %d "
		 "reg_info_1: 0x%x reg_info_2: 0x%x, req->max_txpow: 0x%x "
		 "Tx SS %d, Rx SS %d, ldpc_rx: %d, cac %d, regd %d, HE ops: %d"
		 "req->dis_hw_ack: %d ", req->vdev_id,
		 chan->mhz, req->channel.phy_mode, chan->info,
		 req->channel.dfs_set, req->beacon_interval, cmd->dtim_period,
		 chan->band_center_freq1, chan->band_center_freq2,
		 chan->reg_info_1, chan->reg_info_2, req->channel.maxregpower,
		 req->preferred_tx_streams, req->preferred_rx_streams,
		 req->ldpc_rx_enabled, req->cac_duration_ms,
		 req->regdomain, req->he_ops,
		 req->disable_hw_ack);

	vdev_start_cmd_fill_11be(cmd, req);

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
		wmi_err("Failed to send vdev start command");
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
	wmi_debug("peer_addr "QDF_MAC_ADDR_FMT" vdev_id %d and peer bitmap %d",
		 QDF_MAC_ADDR_REF(peer_addr), param->vdev_id,
		 param->peer_tid_bitmap);
	wmi_mtrace(WMI_PEER_FLUSH_TIDS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_TIDS_CMDID)) {
		wmi_err("Failed to send flush tid command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

#ifdef WLAN_FEATURE_PEER_TXQ_FLUSH_CONF
/**
 * map_to_wmi_flush_policy() - Map flush policy to firmware defined values
 * @policy: The target i/f flush policy value
 *
 * Return: WMI layer flush policy
 */
static wmi_peer_flush_policy
map_to_wmi_flush_policy(enum peer_txq_flush_policy policy)
{
	switch (policy) {
	case PEER_TXQ_FLUSH_POLICY_NONE:
		return WMI_NO_FLUSH;
	case PEER_TXQ_FLUSH_POLICY_TWT_SP_END:
		return WMI_TWT_FLUSH;
	default:
		return WMI_MAX_FLUSH_POLICY;
	}
}

/**
 * send_peer_txq_flush_config_cmd_tlv() - Send peer TID queue flush config
 * @wmi: wmi handle
 * @para: Peer txq flush configuration
 *
 * Return: QDF status
 */
static QDF_STATUS
send_peer_txq_flush_config_cmd_tlv(wmi_unified_t wmi,
				   struct peer_txq_flush_config_params *param)
{
	wmi_peer_flush_policy_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_flush_policy_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_flush_policy_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_flush_policy_cmd_fixed_param));

	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = param->tid_mask;
	cmd->vdev_id = param->vdev_id;
	cmd->flush_policy = map_to_wmi_flush_policy(param->policy);
	if (cmd->flush_policy == WMI_MAX_FLUSH_POLICY) {
		wmi_buf_free(buf);
		wmi_err("Invalid policy");
		return QDF_STATUS_E_INVAL;
	}
	wmi_debug("peer_addr " QDF_MAC_ADDR_FMT "vdev %d tid %x policy %d",
		  QDF_MAC_ADDR_REF(param->peer), param->vdev_id,
		  param->tid_mask, param->policy);
	wmi_mtrace(WMI_PEER_FLUSH_POLICY_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_POLICY_CMDID)) {
		wmi_err("Failed to send flush policy command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif
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

	wmi_debug("peer_addr "QDF_MAC_ADDR_FMT" vdev_id %d",
		 QDF_MAC_ADDR_REF(peer_addr), vdev_id);
	wmi_mtrace(WMI_PEER_DELETE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_DELETE_CMDID)) {
		wmi_err("Failed to send peer delete command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_peer_delete_all_cmd_tlv() - send PEER delete all command to fw
 * @wmi: wmi handle
 * @param: pointer to hold peer delete all parameter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_delete_all_cmd_tlv(
				wmi_unified_t wmi,
				struct peer_delete_all_params *param)
{
	wmi_vdev_delete_all_peer_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_delete_all_peer_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_delete_all_peer_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
				(wmi_vdev_delete_all_peer_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;

	wmi_debug("vdev_id %d", cmd->vdev_id);
	wmi_mtrace(WMI_VDEV_DELETE_ALL_PEER_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len,
				 WMI_VDEV_DELETE_ALL_PEER_CMDID)) {
		wmi_err("Failed to send peer del all command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

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
		wmi_err("Unavailable param %d", param->param_id);
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

	wmi_debug("vdev_id %d peer_mac: "QDF_MAC_ADDR_FMT" param_id: %u param_value: %x",
		 cmd->vdev_id,
		 QDF_MAC_ADDR_REF(peer_addr), param->param_id,
		 cmd->param_value);

	wmi_mtrace(WMI_PEER_SET_PARAM_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi, buf,
				   sizeof(wmi_peer_set_param_cmd_fixed_param),
				   WMI_PEER_SET_PARAM_CMDID);
	if (err) {
		wmi_err("Failed to send set_param cmd");
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

	wmi_debug("VDEV_UP");
	wmi_debug("vdev_id %d aid %d bssid "QDF_MAC_ADDR_FMT,
		 params->vdev_id, params->assoc_id, QDF_MAC_ADDR_REF(bssid));
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
		wmi_err("Failed to send vdev up command");
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
	uint8_t *buf_ptr;
	int32_t len = sizeof(*cmd);

	len += peer_create_mlo_params_size(param);
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

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	buf_ptr += sizeof(*cmd);
	buf_ptr = peer_create_add_mlo_params(buf_ptr, param);
	wmi_mtrace(WMI_PEER_CREATE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_CREATE_CMDID)) {
		wmi_err("Failed to send WMI_PEER_CREATE_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debug("peer_addr "QDF_MAC_ADDR_FMT" vdev_id %d",
		 QDF_MAC_ADDR_REF(param->peer_addr),
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
		wmi_err("Fail to send WMI_PEER_REORDER_QUEUE_SETUP_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debug("peer_macaddr "QDF_MAC_ADDR_FMT" vdev_id %d, tid %d",
		 QDF_MAC_ADDR_REF(param->peer_macaddr),
		 param->vdev_id, param->tid);

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
		wmi_err("Fail to send WMI_PEER_REORDER_QUEUE_REMOVE_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debug("peer_macaddr "QDF_MAC_ADDR_FMT" vdev_id %d, tid_map %d",
		 QDF_MAC_ADDR_REF(param->peer_macaddr),
		 param->vdev_id, param->peer_tid_bitmap);

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

	wmi_debug("Set Green AP PS val %d", value);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_green_ap_ps_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		   WMITLV_TAG_STRUC_wmi_pdev_green_ap_ps_enable_cmd_fixed_param,
		   WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_green_ap_ps_enable_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								pdev_id);
	cmd->enable = value;

	wmi_mtrace(WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID)) {
		wmi_err("Set Green AP PS param Failed val %d", value);
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

		wmi_debug("segHdrInfo.len = %d, segHdrInfo.msgref = %d,"
			 " segHdrInfo.segmentInfo = %d",
			 segHdrInfo.len, segHdrInfo.msgref,
			 segHdrInfo.segmentInfo);

		wmi_debug("total_bytes %d segNumber %d totalSegments %d"
			 " chunk len %d", total_bytes, segNumber,
			 numSegments, chunk_len);

		segNumber++;

		WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE,
			       (chunk_len + sizeof(segHdrInfo)));
		cmd += WMI_TLV_HDR_SIZE;
		memcpy(cmd, &segHdrInfo, sizeof(segHdrInfo));   /* 4 bytes */
		WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&cmd[sizeof(segHdrInfo)],
						bufpos, chunk_len);

		wmi_mtrace(WMI_PDEV_UTF_CMDID, NO_SESSION, 0);
		ret = wmi_unified_cmd_send(wmi_handle, buf,
					   (chunk_len + sizeof(segHdrInfo) +
					    WMI_TLV_HDR_SIZE),
					   WMI_PDEV_UTF_CMDID);

		if (QDF_IS_STATUS_ERROR(ret)) {
			wmi_err("Failed to send WMI_PDEV_UTF_CMDID command");
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
		wmi_err("Unavailable param %d", param->param_id);
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
	if (param->is_host_pdev_id)
		cmd->pdev_id = wmi_handle->ops->convert_host_pdev_id_to_target(
								wmi_handle,
								mac_id);
	else
		cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								mac_id);
	cmd->param_id = pdev_param;
	cmd->param_value = param->param_value;
	wmi_debug("Setting pdev param = %x, value = %u", param->param_id,
		 param->param_value);
	wmi_mtrace(WMI_PDEV_SET_PARAM_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_pdev_set_hw_mode_cmd_tlv() - Send WMI_PDEV_SET_HW_MODE_CMDID to FW
 * @wmi_handle: wmi handle
 * @msg: Structure containing the following parameters
 * @hw_mode_index: The HW_Mode field is a enumerated type that is selected
 * from the HW_Mode table, which is returned in the WMI_SERVICE_READY_EVENTID.
 *
 * Provides notification to the WLAN firmware that host driver is requesting a
 * HardWare (HW) Mode change. This command is needed to support iHelium in the
 * configurations that include the Dual Band Simultaneous (DBS) feature.
 *
 * Return: Success if the cmd is sent successfully to the firmware
 */
static QDF_STATUS send_pdev_set_hw_mode_cmd_tlv(wmi_unified_t wmi_handle,
						uint32_t hw_mode_index)
{
	wmi_pdev_set_hw_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_set_hw_mode_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_hw_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_set_hw_mode_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);
	cmd->hw_mode_index = hw_mode_index;
	wmi_debug("HW mode index:%d", cmd->hw_mode_index);

	wmi_mtrace(WMI_PDEV_SET_HW_MODE_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_HW_MODE_CMDID)) {
		wmi_err("Failed to send WMI_PDEV_SET_HW_MODE_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
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

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								mac_id);

	wmi_mtrace(WMI_PDEV_SUSPEND_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, len,
				 WMI_PDEV_SUSPEND_CMDID);
	if (ret) {
		wmi_buf_free(wmibuf);
		wmi_err("Failed to send WMI_PDEV_SUSPEND_CMDID command");
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
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								mac_id);
	wmi_mtrace(WMI_PDEV_RESUME_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, sizeof(*cmd),
				   WMI_PDEV_RESUME_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI_PDEV_RESUME_CMDID command");
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

	wmi_debug("suspend type: %s flag is 0x%x",
		  cmd->pause_iface_config == WOW_IFACE_PAUSE_ENABLED ?
		  "WOW_IFACE_PAUSE_ENABLED" : "WOW_IFACE_PAUSE_DISABLED",
		  cmd->flags);

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
		wmi_err("Failed to send set_ap_ps_param cmd");
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
	cmd->param = param->param_id;
	cmd->value = param->value;

	wmi_mtrace(WMI_STA_POWERSAVE_PARAM_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_STA_POWERSAVE_PARAM_CMDID)) {
		wmi_err("Set Sta Ps param Failed vdevId %d Param %d val %d",
			 param->vdev_id, param->param_id, param->value);
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
		wmi_err("Failed to send set param command, ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

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
		wmi_err("Vdev param %d not available", param->param_id);
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
	cmd->vdev_id = param->vdev_id;
	cmd->param_id = vdev_param;
	cmd->param_value = param->param_value;
	wmi_debug("Setting vdev %d param = %x, value = %u",
		 cmd->vdev_id, cmd->param_id, cmd->param_value);
	wmi_mtrace(WMI_VDEV_SET_PARAM_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_vdev_set_mu_snif_cmd_tlv() - WMI vdev set mu snif function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold mu sniffer parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static
QDF_STATUS send_vdev_set_mu_snif_cmd_tlv(wmi_unified_t wmi_handle,
					 struct vdev_set_mu_snif_param *param)
{
	QDF_STATUS ret;
	wmi_vdev_set_mu_snif_cmd_param *cmd;
	wmi_buf_t buf;
	uint32_t *tmp_ptr;
	uint16_t len = sizeof(*cmd);
	uint8_t *buf_ptr;
	uint32_t i;

	/* Length TLV placeholder for array of uint32_t */
	len += WMI_TLV_HDR_SIZE;

	if (param->num_aid)
		len += param->num_aid * sizeof(uint32_t);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_vdev_set_mu_snif_cmd_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_mu_snif_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_vdev_set_mu_snif_cmd_param));

	cmd->vdev_id = param->vdev_id;
	cmd->mode = param->mode;
	cmd->max_num_user = param->num_user;

	buf_ptr += sizeof(*cmd);

	tmp_ptr = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);

	for (i = 0; i < param->num_aid; ++i)
		tmp_ptr[i] = param->aid[i];

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_UINT32,
		       (param->num_aid * sizeof(uint32_t)));

	wmi_debug("Setting vdev %d mode = %x, max user = %u aids= %u",
		  cmd->vdev_id, cmd->mode, cmd->max_num_user, param->num_aid);
	wmi_mtrace(WMI_VDEV_SET_PARAM_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_MU_SNIF_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
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
		wmi_err("Failed to send peer based pktlog command to FW =%d"
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
	int32_t ret, idx, max_idx;
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
	max_idx = sizeof(pktlog_event_tlv) / (sizeof(pktlog_event_tlv[0]));
	cmd->evlist = 0;
	for (idx = 0; idx < max_idx; idx++) {
		if (PKTLOG_EVENT & (1 << idx))
			cmd->evlist |=  pktlog_event_tlv[idx];
	}
	cmd->pdev_id = mac_id;
	wmi_mtrace(WMI_PDEV_PKTLOG_ENABLE_CMDID, cmd->pdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_ENABLE_CMDID);
	if (ret) {
		wmi_err("Failed to send pktlog enable cmd to FW =%d", ret);
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
		wmi_err("Failed to send pktlog disable cmd to FW =%d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

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
	wmi_debug("WMA --> DBGLOG_TIME_STAMP_SYNC_CMDID mode %d time_stamp low %d high %d",
		 time_stamp->mode, time_stamp->time_stamp_low,
		 time_stamp->time_stamp_high);

	wmi_mtrace(WMI_DBGLOG_TIME_STAMP_SYNC_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_DBGLOG_TIME_STAMP_SYNC_CMDID);
	if (status) {
		wmi_err("Failed to send WMI_DBGLOG_TIME_STAMP_SYNC_CMDID command");
		wmi_buf_free(buf);
	}

}

/**
 *  send_fd_tmpl_cmd_tlv() - WMI FILS Discovery send function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold FILS Discovery send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_fd_tmpl_cmd_tlv(wmi_unified_t wmi_handle,
				struct fils_discovery_tmpl_params *param)
{
	int32_t ret;
	wmi_fd_tmpl_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t wmi_buf_len;

	wmi_buf_len = sizeof(wmi_fd_tmpl_cmd_fixed_param) +
		      WMI_TLV_HDR_SIZE + param->tmpl_len_aligned;
	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_fd_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_fd_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_fd_tmpl_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->buf_len = param->tmpl_len;
	buf_ptr += sizeof(wmi_fd_tmpl_cmd_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, param->tmpl_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->frm, param->tmpl_len);

	wmi_mtrace(WMI_FD_TMPL_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				wmi_buf, wmi_buf_len, WMI_FD_TMPL_CMDID);

	if (ret) {
		wmi_err("Failed to send fd tmpl: %d", ret);
		wmi_buf_free(wmi_buf);
		return ret;
	}

	return 0;
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
	cmd->mu_edca_ie_offset = param->mu_edca_ie_offset;
	cmd->ema_params = param->ema_params;
	cmd->buf_len = param->tmpl_len;
	cmd->csa_event_bitmap = param->csa_event_bitmap;

	WMI_BEACON_PROTECTION_EN_SET(cmd->feature_enable_bitmap,
				     param->enable_bigtk);
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

	/* for big endian host, copy engine byte_swap is enabled
	 * But the frame content is in network byte order
	 * Need to byte swap the frame content - so when copy engine
	 * does byte_swap - target gets frame content in the correct order
	 */
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(buf_ptr, param->frm,
					param->tmpl_len);

	buf_ptr += param->tmpl_len;
	buf_ptr = bcn_tmpl_add_ml_partner_links(buf_ptr, param);

	wmi_mtrace(WMI_BCN_TMPL_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_BCN_TMPL_CMDID);
	if (ret) {
		wmi_err("Failed to send bcn tmpl: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	return 0;
}

#ifdef WLAN_FEATURE_11BE
static inline void copy_peer_flags_tlv_11be(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param)
{
	if (param->bw_320)
		cmd->peer_flags_ext |= WMI_PEER_EXT_320MHZ;
	if (param->eht_flag)
		cmd->peer_flags_ext |= WMI_PEER_EXT_EHT;

	wmi_debug("peer_flags_ext 0x%x", cmd->peer_flags_ext);
}
#else
static inline void copy_peer_flags_tlv_11be(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param)
{
}
#endif

/* Need to remove below flag after fw change merging */
#ifdef PEER_DMS_CAPABLE
static inline void set_peer_dms_capable(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd)
{
	cmd->peer_flags_ext |= WMI_PEER_EXT_DMS_CAPABLE;
}
#else
static inline void set_peer_dms_capable(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd)
{
}
#endif

static inline void copy_peer_flags_tlv(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param)
{
	/*
	 * The target only needs a subset of the flags maintained in the host.
	 * Just populate those flags and send it down
	 */
	cmd->peer_flags = 0;
	if (param->peer_dms_capable)
		set_peer_dms_capable(cmd);
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

		copy_peer_flags_tlv_11be(cmd, param);

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
	/* inter BSS peer */
	if (param->inter_bss_peer)
		cmd->peer_flags |= WMI_PEER_INTER_BSS_PEER;
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

#ifdef WLAN_FEATURE_11BE
static uint8_t *update_peer_flags_tlv_ehtinfo(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param, uint8_t *buf_ptr)
{
	wmi_eht_rate_set *eht_mcs;
	int i;

	cmd->peer_eht_ops = param->peer_eht_ops;
	cmd->puncture_20mhz_bitmap = ~param->puncture_bitmap;

	qdf_mem_copy(&cmd->peer_eht_cap_mac, &param->peer_eht_cap_macinfo,
		     sizeof(param->peer_eht_cap_macinfo));
	qdf_mem_copy(&cmd->peer_eht_cap_phy, &param->peer_eht_cap_phyinfo,
		     sizeof(param->peer_eht_cap_phyinfo));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       (param->peer_eht_mcs_count * sizeof(wmi_eht_rate_set)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Loop through the EHT rate set */
	for (i = 0; i < param->peer_eht_mcs_count; i++) {
		eht_mcs = (wmi_eht_rate_set *)buf_ptr;
		WMITLV_SET_HDR(eht_mcs, WMITLV_TAG_STRUC_wmi_eht_rate_set,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_eht_rate_set));

		eht_mcs->rx_mcs_set = param->peer_eht_rx_mcs_set[i];
		eht_mcs->tx_mcs_set = param->peer_eht_tx_mcs_set[i];
		wmi_debug("EHT idx %d RxMCSmap %x TxMCSmap %x ",
			  i, eht_mcs->rx_mcs_set, eht_mcs->tx_mcs_set);
		buf_ptr += sizeof(wmi_eht_rate_set);
	}

	if ((param->eht_flag) && (param->peer_eht_mcs_count > 1) &&
	    (param->peer_eht_rx_mcs_set[WMI_HOST_EHT_TXRX_MCS_NSS_IDX_160]
	     == WMI_HOST_EHT_INVALID_MCSNSSMAP ||
	     param->peer_eht_tx_mcs_set[WMI_HOST_EHT_TXRX_MCS_NSS_IDX_160]
	     == WMI_HOST_HE_INVALID_MCSNSSMAP)) {
		wmi_debug("param->peer_eht_tx_mcs_set[160MHz]=%x",
			  param->peer_eht_tx_mcs_set
			  [WMI_HOST_HE_TXRX_MCS_NSS_IDX_160]);
		wmi_debug("param->peer_eht_rx_mcs_set[160MHz]=%x",
			  param->peer_eht_rx_mcs_set
			  [WMI_HOST_HE_TXRX_MCS_NSS_IDX_160]);
		wmi_debug("peer_mac="QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(param->peer_mac));
	}

	wmi_debug("EHT cap_mac %x %x ehtops %x  EHT phy %x  %x  %x  pp %x",
		  cmd->peer_eht_cap_mac[0],
		  cmd->peer_eht_cap_mac[1], cmd->peer_eht_ops,
		  cmd->peer_eht_cap_phy[0], cmd->peer_he_cap_phy[1],
		  cmd->peer_eht_cap_phy[2], cmd->puncture_20mhz_bitmap);

	return buf_ptr;
}
#else
static uint8_t *update_peer_flags_tlv_ehtinfo(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param, uint8_t *buf_ptr)
{
	return buf_ptr;
}
#endif

#ifdef WLAN_FEATURE_11BE
static
uint32_t wmi_eht_peer_assoc_params_len(struct peer_assoc_params *param)
{
	return (sizeof(wmi_he_rate_set) * param->peer_eht_mcs_count
			 + WMI_TLV_HDR_SIZE);
}

static void wmi_populate_service_11be(uint32_t *wmi_service)
{
	wmi_service[wmi_service_11be] = WMI_SERVICE_11BE;
}

#else
static
uint32_t wmi_eht_peer_assoc_params_len(struct peer_assoc_params *param)
{
	return 0;
}

static void wmi_populate_service_11be(uint32_t *wmi_service)
{
}

#endif

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
		+ WMI_TLV_HDR_SIZE)
		+ wmi_eht_peer_assoc_params_len(param) +
		peer_assoc_mlo_params_size(param);

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
	cmd->bss_max_idle_option = param->peer_bss_max_idle_option;

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
	cmd->peer_he_caps_6ghz = param->peer_he_caps_6ghz;

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

	cmd->auth_mode = param->akm;
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
		mcs->tx_max_mcs_nss = param->tx_max_mcs_nss;
	}

	/* HE Rates */
	cmd->min_data_rate = param->min_data_rate;
	cmd->peer_he_mcs = param->peer_he_mcs_count;
	buf_ptr += sizeof(wmi_vht_rate_set);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(param->peer_he_mcs_count * sizeof(wmi_he_rate_set)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMI_PEER_STA_TYPE_SET(cmd->sta_type, param->peer_bsscolor_rept_info);
	/* Loop through the HE rate set */
	for (i = 0; i < param->peer_he_mcs_count; i++) {
		he_mcs = (wmi_he_rate_set *) buf_ptr;
		WMITLV_SET_HDR(he_mcs, WMITLV_TAG_STRUC_wmi_he_rate_set,
			WMITLV_GET_STRUCT_TLVLEN(wmi_he_rate_set));

		he_mcs->rx_mcs_set = param->peer_he_rx_mcs_set[i];
		he_mcs->tx_mcs_set = param->peer_he_tx_mcs_set[i];
		wmi_debug("HE idx %d RxMCSmap %x TxMCSmap %x ",
			 i, he_mcs->rx_mcs_set, he_mcs->tx_mcs_set);
		buf_ptr += sizeof(wmi_he_rate_set);
	}

	if ((param->he_flag) && (param->peer_he_mcs_count > 1) &&
	    (param->peer_he_rx_mcs_set[WMI_HOST_HE_TXRX_MCS_NSS_IDX_160]
	     == WMI_HOST_HE_INVALID_MCSNSSMAP ||
	     param->peer_he_tx_mcs_set[WMI_HOST_HE_TXRX_MCS_NSS_IDX_160]
	     == WMI_HOST_HE_INVALID_MCSNSSMAP)) {
		wmi_debug("param->peer_he_tx_mcs_set[160MHz]=%x",
			 param->peer_he_tx_mcs_set[WMI_HOST_HE_TXRX_MCS_NSS_IDX_160]);
		wmi_debug("param->peer_he_rx_mcs_set[160MHz]=%x",
			 param->peer_he_rx_mcs_set[WMI_HOST_HE_TXRX_MCS_NSS_IDX_160]);
		wmi_debug("peer_mac="QDF_MAC_ADDR_FMT,
			 QDF_MAC_ADDR_REF(param->peer_mac));
	}

	wmi_debug("vdev_id %d associd %d peer_flags %x rate_caps %x "
		 "peer_caps %x listen_intval %d ht_caps %x max_mpdu %d "
		 "nss %d phymode %d peer_mpdu_density %d "
		 "cmd->peer_vht_caps %x "
		 "HE cap_info %x ops %x "
		 "HE cap_info_ext %x "
		 "HE phy %x  %x  %x  "
		 "peer_bw_rxnss_override %x",
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

	buf_ptr = peer_assoc_add_mlo_params(buf_ptr, param);

	buf_ptr = update_peer_flags_tlv_ehtinfo(cmd, param, buf_ptr);

	buf_ptr = peer_assoc_add_ml_partner_links(buf_ptr, param);

	wmi_mtrace(WMI_PEER_ASSOC_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_ASSOC_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send peer assoc command ret = %d", ret);
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
	wmi_hint_freq_short_ssid *s_ssid = NULL;
	wmi_hint_freq_bssid *hint_bssid = NULL;

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

	len += WMI_TLV_HDR_SIZE;
	if (params->num_hint_bssid)
		len += params->num_hint_bssid * sizeof(wmi_hint_freq_bssid);

	len += WMI_TLV_HDR_SIZE;
	if (params->num_hint_s_ssid)
		len += params->num_hint_s_ssid * sizeof(wmi_hint_freq_short_ssid);

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
	cmd->min_dwell_time_6ghz = params->min_dwell_time_6g;
	cmd->dwell_time_active_6ghz = params->dwell_time_active_6g;
	cmd->dwell_time_passive_6ghz = params->dwell_time_passive_6g;
	cmd->scan_start_offset = params->scan_offset_time;
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
	for (i = 0; i < params->chan_list.num_chan; ++i) {
		TARGET_SET_FREQ_IN_CHAN_LIST_TLV(tmp_ptr[i],
					params->chan_list.chan[i].freq);
		TARGET_SET_FLAGS_IN_CHAN_LIST_TLV(tmp_ptr[i],
					params->chan_list.chan[i].flags);
	}

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_UINT32,
		       (params->chan_list.num_chan * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE +
			(params->chan_list.num_chan * sizeof(uint32_t));

	if (params->num_ssids > WLAN_SCAN_MAX_NUM_SSID) {
		wmi_err("Invalid value for num_ssids %d", params->num_ssids);
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
		buf_ptr += WMI_TLV_HDR_SIZE;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       (params->num_hint_s_ssid * sizeof(wmi_hint_freq_short_ssid)));
	if (params->num_hint_s_ssid) {
		s_ssid = (wmi_hint_freq_short_ssid *)(buf_ptr + WMI_TLV_HDR_SIZE);
		for (i = 0; i < params->num_hint_s_ssid; ++i) {
			s_ssid->freq_flags = params->hint_s_ssid[i].freq_flags;
			s_ssid->short_ssid = params->hint_s_ssid[i].short_ssid;
			s_ssid++;
		}
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		(params->num_hint_s_ssid * sizeof(wmi_hint_freq_short_ssid));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       (params->num_hint_bssid * sizeof(wmi_hint_freq_bssid)));
	if (params->num_hint_bssid) {
		hint_bssid = (wmi_hint_freq_bssid *)(buf_ptr + WMI_TLV_HDR_SIZE);
		for (i = 0; i < params->num_hint_bssid; ++i) {
			hint_bssid->freq_flags =
				params->hint_bssid[i].freq_flags;
			WMI_CHAR_ARRAY_TO_MAC_ADDR(&params->hint_bssid[i].bssid.bytes[0],
						   &hint_bssid->bssid);
			hint_bssid++;
		}
	}

	wmi_mtrace(WMI_START_SCAN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf,
				   len, WMI_START_SCAN_CMDID);
	if (ret) {
		wmi_err("Failed to start scan: %d", ret);
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
								wmi_handle,
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
	} else if (param->req_type == WLAN_SCAN_CANCEL_HOST_VDEV_ALL) {
		cmd->req_type = WMI_SCN_STOP_HOST_VAP_ALL;
	} else {
		wmi_err("Invalid Scan cancel req type: %d", param->req_type);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_INVAL;
	}

	wmi_mtrace(WMI_STOP_SCAN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf,
				   len, WMI_STOP_SCAN_CMDID);
	if (ret) {
		wmi_err("Failed to send stop scan: %d", ret);
		wmi_buf_free(wmi_buf);
	}

error:
	return ret;
}

#define WMI_MAX_CHAN_INFO_LOG 192

/**
 * wmi_scan_chanlist_dump() - Dump scan channel list info
 * @scan_chan_list: scan channel list
 *
 * Return: void
 */
static void wmi_scan_chanlist_dump(struct scan_chan_list_params *scan_chan_list)
{
	uint32_t i;
	uint8_t info[WMI_MAX_CHAN_INFO_LOG];
	uint32_t len = 0;
	struct channel_param *chan;
	int ret;

	wmi_debug("Total chan %d", scan_chan_list->nallchans);
	for (i = 0; i < scan_chan_list->nallchans; i++) {
		chan = &scan_chan_list->ch_param[i];
		ret = qdf_scnprintf(info + len, sizeof(info) - len,
				    " %d[%d][%d][%d]", chan->mhz,
				    chan->maxregpower,
				    chan->dfs_set, chan->nan_disabled);
		if (ret <= 0)
			break;
		len += ret;
		if (len >= (sizeof(info) - 20)) {
			wmi_nofl_debug("Chan[TXPwr][DFS][nan_disabled]:%s",
				       info);
			len = 0;
		}
	}
	if (len)
		wmi_nofl_debug("Chan[TXPwr][DFS]:%s", info);
}

static QDF_STATUS send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *chan_list)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	wmi_scan_chan_list_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	wmi_channel *chan_info;
	struct channel_param *tchan_info;
	uint16_t len;
	uint16_t num_send_chans, num_sends = 0;

	wmi_scan_chanlist_dump(chan_list);
	tchan_info = &chan_list->ch_param[0];
	while (chan_list->nallchans) {
		len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
		if (chan_list->nallchans > MAX_NUM_CHAN_PER_WMI_CMD)
			num_send_chans =  MAX_NUM_CHAN_PER_WMI_CMD;
		else
			num_send_chans = chan_list->nallchans;

		chan_list->nallchans -= num_send_chans;
		len += sizeof(wmi_channel) * num_send_chans;
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			qdf_status = QDF_STATUS_E_NOMEM;
			goto end;
		}

		buf_ptr = (uint8_t *)wmi_buf_data(buf);
		cmd = (wmi_scan_chan_list_cmd_fixed_param *)buf_ptr;
		WMITLV_SET_HDR(&cmd->tlv_header,
			       WMITLV_TAG_STRUC_wmi_scan_chan_list_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_chan_list_cmd_fixed_param));

		wmi_debug("no of channels = %d, len = %d", num_send_chans, len);

		if (num_sends)
			cmd->flags |= APPEND_TO_EXISTING_CHAN_LIST;

		if (chan_list->max_bw_support_present)
			cmd->flags |= CHANNEL_MAX_BANDWIDTH_VALID;

		cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
						wmi_handle,
						chan_list->pdev_id);

		wmi_mtrace(WMI_SCAN_CHAN_LIST_CMDID, cmd->pdev_id, 0);

		cmd->num_scan_chans = num_send_chans;
		WMITLV_SET_HDR((buf_ptr +
				sizeof(wmi_scan_chan_list_cmd_fixed_param)),
			       WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_channel) * num_send_chans);
		chan_info = (wmi_channel *)(buf_ptr + sizeof(*cmd) +
					    WMI_TLV_HDR_SIZE);

		for (i = 0; i < num_send_chans; ++i) {
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

			if (tchan_info->dfs_set_cfreq2)
				WMI_SET_CHANNEL_FLAG(chan_info,
						     WMI_CHAN_FLAG_DFS_CFREQ2);

			if (tchan_info->allow_he)
				WMI_SET_CHANNEL_FLAG(chan_info,
						     WMI_CHAN_FLAG_ALLOW_HE);

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

			if (tchan_info->psc_channel)
				WMI_SET_CHANNEL_FLAG(chan_info,
						     WMI_CHAN_FLAG_PSC);

			if (tchan_info->nan_disabled)
				WMI_SET_CHANNEL_FLAG(chan_info,
					     WMI_CHAN_FLAG_NAN_DISABLED);

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
			WMI_SET_CHANNEL_MAX_BANDWIDTH(chan_info,
						      tchan_info->max_bw_supported);

			tchan_info++;
			chan_info++;
		}

		qdf_status = wmi_unified_cmd_send(
			wmi_handle,
			buf, len, WMI_SCAN_CHAN_LIST_CMDID);

		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			wmi_err("Failed to send WMI_SCAN_CHAN_LIST_CMDID");
			wmi_buf_free(buf);
			goto end;
		}
		num_sends++;
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
	WMI_TX_SEND_PARAM_CFR_CAPTURE_SET(tx_param->tx_param_dword1,
					  param.cfr_enable);
	WMI_TX_SEND_PARAM_BEAMFORM_SET(tx_param->tx_param_dword1,
				       param.en_beamforming);
	WMI_TX_SEND_PARAM_RETRY_LIMIT_EXT_SET(tx_param->tx_param_dword1,
					      param.retry_limit_ext);

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
		wmi_err("mgmt frame len %u exceeds %u",
			 param->frm_len, mgmt_tx_dl_frm_len);
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
	cmd->tx_flags = param->tx_flags;
	cmd->peer_rssi = param->peer_rssi;

	wmi_mgmt_cmd_record(wmi_handle, WMI_MGMT_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	bufp += roundup(bufp_len, sizeof(uint32_t));
	if (param->tx_params_valid) {
		if (populate_tx_send_params(bufp, param->tx_param) !=
		    QDF_STATUS_SUCCESS) {
			wmi_err("Populate TX send params failed");
			goto free_buf;
		}
		cmd_len += sizeof(wmi_tx_send_params);
	}

	wmi_mtrace(WMI_MGMT_TX_SEND_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				      WMI_MGMT_TX_SEND_CMDID)) {
		wmi_err("Failed to send mgmt Tx");
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
	cmd->peer_rssi = param->peer_rssi;
	bufp += sizeof(wmi_mgmt_tx_send_cmd_fixed_param);
	WMITLV_SET_HDR(bufp, WMITLV_TAG_ARRAY_BYTE, roundup(bufp_len,
							    sizeof(uint32_t)));
	bufp += WMI_TLV_HDR_SIZE;

	/* for big endian host, copy engine byte_swap is enabled
	 * But the frame content is in network byte order
	 * Need to byte swap the frame content - so when copy engine
	 * does byte_swap - target gets frame content in the correct order
	 */
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(bufp, param->pdata, bufp_len);

	status = qdf_nbuf_map_single(qdf_ctx, param->tx_frame,
				     QDF_DMA_TO_DEVICE);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("wmi buf map failed");
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
	cmd->tx_flags = param->tx_flags;

	wmi_mgmt_cmd_record(wmi_handle, WMI_MGMT_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	bufp += roundup(bufp_len, sizeof(uint32_t));
	if (param->tx_params_valid) {
		status = populate_tx_send_params(bufp, param->tx_param);
		if (status != QDF_STATUS_SUCCESS) {
			wmi_err("Populate TX send params failed");
			goto unmap_tx_frame;
		}
		cmd_len += sizeof(wmi_tx_send_params);
	}

	wmi_mtrace(WMI_MGMT_TX_SEND_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				      WMI_MGMT_TX_SEND_CMDID)) {
		wmi_err("Failed to send mgmt Tx");
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
			wmi_err("Populate TX send params failed");
			goto err1;
		}
		cmd_len += sizeof(wmi_tx_send_params);
	}

	wmi_mtrace(WMI_OFFCHAN_DATA_TX_SEND_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				WMI_OFFCHAN_DATA_TX_SEND_CMDID)) {
		wmi_err("Failed to offchan data Tx");
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
	wmi_debug("Setting cmd->modem_power_state = %u", param_value);
	wmi_mtrace(WMI_MODEM_POWER_STATE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				     WMI_MODEM_POWER_STATE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send notify cmd ret = %d", ret);
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

	wmi_debug("Set Sta Mode Ps vdevId %d val %d", vdev_id, val);

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
		wmi_err("Set Sta Mode Ps Failed vdevId %d val %d",
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

	wmi_debug("val: %d", cmd->idle_trigger_monitor);

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
		wmi_err("INVALID MIMO PS CONFIG: %d", value);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	wmi_debug("Setting vdev %d value = %u", vdev_id, value);

	wmi_mtrace(WMI_STA_SMPS_FORCE_MODE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_SMPS_FORCE_MODE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set Mimo PS ret = %d", ret);
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

	wmi_debug("Setting vdev %d value = %x param %x", vdev_id, cmd->value,
		 cmd->param);

	wmi_mtrace(WMI_STA_SMPS_PARAM_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_SMPS_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set Mimo PS ret = %d", ret);
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
		wmi_err("WMI is closed, can not issue cmd");
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
		wmi_err("Failed to send get temperature command");
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
		wmi_err("Failed to send set uapsd param ret = %d", ret);
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
	uint8_t action;

	switch (thermal_info->thermal_action) {
	case THERMAL_MGMT_ACTION_DEFAULT:
		action = WMI_THERMAL_MGMT_ACTION_DEFAULT;
		break;

	case THERMAL_MGMT_ACTION_HALT_TRAFFIC:
		action = WMI_THERMAL_MGMT_ACTION_HALT_TRAFFIC;
		break;

	case THERMAL_MGMT_ACTION_NOTIFY_HOST:
		action = WMI_THERMAL_MGMT_ACTION_NOTIFY_HOST;
		break;

	case THERMAL_MGMT_ACTION_CHAINSCALING:
		action = WMI_THERMAL_MGMT_ACTION_CHAINSCALING;
		break;

	default:
		wmi_err("Invalid thermal_action code %d",
			thermal_info->thermal_action);
		return QDF_STATUS_E_FAILURE;
	}

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
	cmd->action = action;

	wmi_debug("TM Sending thermal mgmt cmd: low temp %d, upper temp %d, enabled %d action %d",
		 cmd->lower_thresh_degreeC, cmd->upper_thresh_degreeC,
		 cmd->enable, cmd->action);

	wmi_mtrace(WMI_THERMAL_MGMT_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_THERMAL_MGMT_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		wmi_err("Failed to send thermal mgmt command");
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

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								pdev_id);
	wmi_debug("WMI_LRO_CONFIG: lro_enable %d, tcp_flag 0x%x, pdev_id: %d",
		 cmd->lro_enable, cmd->tcp_flag_u32, cmd->pdev_id);

	wmi_mtrace(WMI_LRO_CONFIG_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
		 sizeof(*cmd), WMI_LRO_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		wmi_err("Failed to send WMI_LRO_CONFIG_CMDID");
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

	wmi_debug("enable %d backoff_time %d period %d",
		  cmd->enable_rate_report,
		  cmd->report_backoff_time, cmd->report_timer_period);

	wmi_mtrace(WMI_PEER_SET_RATE_REPORT_CONDITION_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_SET_RATE_REPORT_CONDITION_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		wmi_err("Failed to send peer_set_report_cond command");
	}
	return status;
}

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
	wmi_err("Failed to set WMM Parameters");
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

	wmi_debug("Send probe response template for vdev %d", vdev_id);

	tmpl_len = probe_rsp_info->prb_rsp_template_len;
	tmpl_len_aligned = roundup(tmpl_len, sizeof(uint32_t));

	wmi_buf_len = sizeof(wmi_prb_tmpl_cmd_fixed_param) +
			sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
			tmpl_len_aligned;

	if (wmi_buf_len > WMI_BEACON_TX_BUFFER_SIZE) {
		wmi_err("wmi_buf_len: %d > %d. Can't send wmi cmd",
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
		wmi_err("Failed to send PRB RSP tmpl: %d", ret);
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
	if (key_params->group_key_idx) {
		cmd->is_group_key_ix_valid = 1;
		cmd->group_key_ix = key_params->group_key_idx;
	}


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

	/* for big endian host, copy engine byte_swap is enabled
	 * But key_data is in network byte order
	 * Need to byte swap the key_data - so when copy engine
	 * does byte_swap - target gets key_data in the correct order
	 */
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY((void *)key_data,
					(const void *)key_params->key_data,
					key_params->key_len);
	qdf_mem_copy(&cmd->key_rsc_counter, &key_params->key_rsc_ctr,
		     sizeof(wmi_key_seq_counter));
	cmd->key_len = key_params->key_len;

	qdf_mem_copy(&cmd->key_tsc_counter, &key_params->key_tsc_counter,
		     sizeof(wmi_key_seq_counter));
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
		wmi_err("Invalid p2p ie length %u", ie_len);
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

	wmi_debug("Sending WMI_P2P_GO_SET_BEACON_IE");

	wmi_mtrace(WMI_P2P_GO_SET_BEACON_IE, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len,
				   WMI_P2P_GO_SET_BEACON_IE);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send bcn tmpl: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	wmi_debug("Successfully sent WMI_P2P_GO_SET_BEACON_IE");
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
	wmi_debug("wmi:oui received from hdd %08x", cmd->prob_req_oui);

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
		wmi_err("Failed to send command WMI_SCAN_PROB_REQ_OUI_CMDID");
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

	wmi_debug("offload_type=%d, enable=%d",
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
		wmi_err("Failed to send WMI_IPA_OFFLOAD_ENABLE_DISABLE_CMDID");
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
		wmi_err("Failed to send nlo wmi cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_obss_disable_cmd_tlv() - disable obss scan request
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function request FW to disable ongoing obss scan operation.
 *
 * Return: QDF status
 */
static QDF_STATUS send_obss_disable_cmd_tlv(wmi_unified_t wmi_handle,
					    uint8_t vdev_id)
{
	QDF_STATUS status;
	wmi_buf_t buf;
	wmi_obss_scan_disable_cmd_fixed_param *cmd;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	wmi_debug("cmd %x vdev_id %d", WMI_OBSS_SCAN_DISABLE_CMDID, vdev_id);

	cmd = (wmi_obss_scan_disable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_obss_scan_disable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			wmi_obss_scan_disable_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_OBSS_SCAN_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
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
	wmi_debug("enable: %d, top_k_num: %d, stat_thresh: %d, full_scan: %d",
		 channel_prediction_cfg->enable,
		 channel_prediction_cfg->top_k_num,
		 channel_prediction_cfg->stationary_threshold,
		 channel_prediction_cfg->full_scan_period_ms);
}

/**
 * send_cp_stats_cmd_tlv() - Send cp stats wmi command
 * @wmi_handle: wmi handle
 * @buf_ptr: Buffer passed by upper layers
 * @buf_len: Length of passed buffer by upper layer
 *
 * Copy the buffer passed by the upper layers and send it
 * down to the firmware.
 *
 * Return: None
 */
static QDF_STATUS send_cp_stats_cmd_tlv(wmi_unified_t wmi_handle,
					void *buf_ptr, uint32_t buf_len)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *data_ptr;

	len = buf_len;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	data_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_copy(data_ptr, buf_ptr, len);

	wmi_mtrace(WMI_REQUEST_CTRL_PATH_STATS_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_REQUEST_CTRL_PATH_STATS_CMDID);

	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_cp_stats_more_pending_tlv - api to extract more flag from event data
 * @wmi_handle: wmi handle
 * @evt_buf:    event buffer
 * @more_flag:  buffer to populate more flag
 *
 * Return: status of operation
 */
static QDF_STATUS
extract_cp_stats_more_pending_tlv(wmi_unified_t wmi, void *evt_buf,
				  uint32_t *more_flag)
{
	WMI_CTRL_PATH_STATS_EVENTID_param_tlvs *param_buf;
	wmi_ctrl_path_stats_event_fixed_param *ev;

	param_buf = (WMI_CTRL_PATH_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err_rl("param_buf is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ev = (wmi_ctrl_path_stats_event_fixed_param *)param_buf->fixed_param;

	*more_flag = ev->more;
	return QDF_STATUS_SUCCESS;
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
	wmi_debug("MAWC NLO en=%d, vdev=%d, ratio=%d, SCAN init=%d, max=%d",
		 wmi_nlo_mawc_params->enable, wmi_nlo_mawc_params->vdev_id,
		 wmi_nlo_mawc_params->exp_backoff_ratio,
		 wmi_nlo_mawc_params->init_scan_interval,
		 wmi_nlo_mawc_params->max_scan_interval);

	wmi_mtrace(WMI_NLO_CONFIGURE_MAWC_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_NLO_CONFIGURE_MAWC_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("WMI_NLO_CONFIGURE_MAWC_CMDID failed, Error %d",
			status);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_dump_pno_scan_freq_list() - dump frequency list associated with pno
 * scan
 * @scan_freq_list: frequency list for pno scan
 *
 * Return: void
 */
static void wmi_dump_pno_scan_freq_list(struct chan_list *scan_freq_list)
{
	uint32_t i;
	uint8_t info[WMI_MAX_CHAN_INFO_LOG];
	uint32_t len = 0;
	struct chan_info *chan_info;
	int ret;

	wmi_debug("[PNO_SCAN] Total freq %d", scan_freq_list->num_chan);
	for (i = 0; i < scan_freq_list->num_chan; i++) {
		chan_info = &scan_freq_list->chan[i];
		ret = qdf_scnprintf(info + len, sizeof(info) - len,
				    " %d[%d]", chan_info->freq,
				    chan_info->flags);
		if (ret <= 0)
			break;
		len += ret;
		if (len >= (sizeof(info) - 20)) {
			wmi_nofl_debug("Freq[flag]:%s",
				       info);
			len = 0;
		}
	}
	if (len)
		wmi_nofl_debug("Freq[flag]:%s", info);
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
	qdf_freq_t freq;
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

	len += sizeof(uint32_t) * pno->networks_list[0].pno_chan_list.num_chan;
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

		/* Copy rssi threshold */
		if (pno->networks_list[i].rssi_thresh &&
		    pno->networks_list[i].rssi_thresh >
		    WMI_RSSI_THOLD_DEFAULT) {
			nlo_list[i].rssi_cond.valid = true;
			nlo_list[i].rssi_cond.rssi =
				pno->networks_list[i].rssi_thresh;
		}
		nlo_list[i].bcast_nw_type.valid = true;
		nlo_list[i].bcast_nw_type.bcast_nw_type =
			pno->networks_list[i].bc_new_type;
	}
	buf_ptr += cmd->no_of_ssids * sizeof(nlo_configured_parameters);

	/* Copy channel info */
	cmd->num_of_channels = pno->networks_list[0].pno_chan_list.num_chan;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (cmd->num_of_channels * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	channel_list = (uint32_t *) buf_ptr;
	for (i = 0; i < cmd->num_of_channels; i++) {
		TARGET_SET_FREQ_IN_CHAN_LIST_TLV(channel_list[i],
			pno->networks_list[0].pno_chan_list.chan[i].freq);

		if (channel_list[i] < WMI_NLO_FREQ_THRESH) {
			freq = pno->networks_list[0].pno_chan_list.chan[i].freq;
			TARGET_SET_FREQ_IN_CHAN_LIST_TLV(channel_list[i],
						     wlan_chan_to_freq(freq));
		}

		TARGET_SET_FLAGS_IN_CHAN_LIST_TLV(channel_list[i],
		     pno->networks_list[0].pno_chan_list.chan[i].flags);
	}

	wmi_dump_pno_scan_freq_list(&pno->networks_list[0].pno_chan_list);

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
	}
	buf_ptr += cmd->num_cnlo_band_pref * sizeof(*nlo_band_rssi);

	wmi_mtrace(WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (ret) {
		wmi_err("Failed to send nlo wmi cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

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
		wmi_err("WMI service bit map is not saved yet");
		return false;
	}

	/* if wmi_service_enabled was received with extended2 bitmap,
	 * use WMI_SERVICE_EXT2_IS_ENABLED to check the services.
	 */
	if (soc->wmi_ext2_service_bitmap) {
		if (!soc->wmi_ext_service_bitmap) {
			wmi_err("WMI service ext bit map is not saved yet");
			return false;
		}

		return WMI_SERVICE_EXT2_IS_ENABLED(soc->wmi_service_bitmap,
				soc->wmi_ext_service_bitmap,
				soc->wmi_ext2_service_bitmap,
				service_id);
	}

	if (service_id >= WMI_MAX_EXT_SERVICE) {
		wmi_err_rl("Service id %d but WMI ext2 service bitmap is NULL",
			   service_id);
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
		wmi_err("Service id %d but WMI ext service bitmap is NULL",
			service_id);
		return false;
	}

	return WMI_SERVICE_IS_ENABLED(soc->wmi_service_bitmap,
				service_id);
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

	wmi_debug("LINK_LAYER_STATS - Clear Request Params");
	wmi_debug("StopReq: %d Vdev Id: %d Clear Stat Mask: %d"
		 " Peer MAC Addr: "QDF_MAC_ADDR_FMT,
		 cmd->stop_stats_collection_req,
		 cmd->vdev_id, cmd->stats_clear_req_mask,
		 QDF_MAC_ADDR_REF(clear_req->peer_macaddr.bytes));

	wmi_mtrace(WMI_CLEAR_LINK_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_CLEAR_LINK_STATS_CMDID);
	if (ret) {
		wmi_err("Failed to send clear link stats req");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	wmi_debug("Clear Link Layer Stats request sent successfully");
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

	wmi_debug("LINK_LAYER_STATS - Start/Set Params MPDU Size Thresh : %d Aggressive Gather: %d",
		 cmd->mpdu_size_threshold,
		 cmd->aggressive_statistics_gathering);

	wmi_mtrace(WMI_START_LINK_STATS_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_START_LINK_STATS_CMDID);
	if (ret) {
		wmi_err("Failed to send set link stats request");
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
	bool is_link_stats_over_qmi;

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

	wmi_debug("LINK_LAYER_STATS - Get Request Params Request ID: %u Stats Type: %0x Vdev ID: %d Peer MAC Addr: "QDF_MAC_ADDR_FMT,
		 cmd->request_id, cmd->stats_type, cmd->vdev_id,
		 QDF_MAC_ADDR_REF(get_req->peer_macaddr.bytes));

	wmi_mtrace(WMI_REQUEST_LINK_STATS_CMDID, cmd->vdev_id, 0);
	is_link_stats_over_qmi = is_service_enabled_tlv(
			wmi_handle,
			WMI_SERVICE_UNIFIED_LL_GET_STA_OVER_QMI_SUPPORT);

	if (is_link_stats_over_qmi) {
		ret = wmi_unified_cmd_send_over_qmi(
					wmi_handle, buf, len,
					WMI_REQUEST_LINK_STATS_CMDID);
	} else {
		ret = wmi_unified_cmd_send_pm_chk(
					wmi_handle, buf, len,
					WMI_REQUEST_LINK_STATS_CMDID, true);
	}

	if (ret) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_CLUB_LL_STATS_AND_GET_STATION
/**
 * send_unified_ll_stats_get_sta_cmd_tlv() - unified link layer stats and get
 *                                           station request
 * @wmi_handle: wmi handle
 * @get_req: ll stats get request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_unified_ll_stats_get_sta_cmd_tlv(
				wmi_unified_t wmi_handle,
				const struct ll_stats_get_params *get_req)
{
	wmi_request_unified_ll_get_sta_cmd_fixed_param *unified_cmd;
	int32_t len;
	wmi_buf_t buf;
	void *buf_ptr;
	QDF_STATUS ret;
	bool is_ll_get_sta_stats_over_qmi;

	len = sizeof(*unified_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = wmi_buf_data(buf);

	unified_cmd = buf_ptr;
	WMITLV_SET_HDR(
		&unified_cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_request_unified_ll_get_sta_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_request_unified_ll_get_sta_cmd_fixed_param));

	unified_cmd->link_stats_type = get_req->param_id_mask;
	unified_cmd->get_sta_stats_id = (WMI_REQUEST_AP_STAT |
					 WMI_REQUEST_PEER_STAT |
					 WMI_REQUEST_VDEV_STAT |
					 WMI_REQUEST_PDEV_STAT |
					 WMI_REQUEST_PEER_EXTD2_STAT |
					 WMI_REQUEST_RSSI_PER_CHAIN_STAT);
	unified_cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);

	unified_cmd->vdev_id = get_req->vdev_id;
	unified_cmd->request_id = get_req->req_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(get_req->peer_macaddr.bytes,
				   &unified_cmd->peer_macaddr);

	wmi_debug("UNIFIED_LINK_STATS_GET_STA - Get Request Params Request ID: %u Stats Type: %0x Vdev ID: %d Peer MAC Addr: "
		  QDF_MAC_ADDR_FMT,
		  get_req->req_id, get_req->param_id_mask, get_req->vdev_id,
		  QDF_MAC_ADDR_REF(get_req->peer_macaddr.bytes));

	wmi_mtrace(WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID, get_req->vdev_id, 0);

	/**
	 * FW support for LL_get_sta command. True represents the unified
	 * ll_get_sta command should be sent over QMI always irrespective of
	 * WOW state.
	 */
	is_ll_get_sta_stats_over_qmi = is_service_enabled_tlv(
			wmi_handle,
			WMI_SERVICE_UNIFIED_LL_GET_STA_OVER_QMI_SUPPORT);

	if (is_ll_get_sta_stats_over_qmi) {
		ret = wmi_unified_cmd_send_over_qmi(
					wmi_handle, buf, len,
					WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID);
	} else {
		ret = wmi_unified_cmd_send_pm_chk(
					wmi_handle, buf, len,
					WMI_REQUEST_UNIFIED_LL_GET_STA_CMDID,
					true);
	}

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return ret;
}
#endif
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
	wmi_debug("STATS REQ VDEV_ID:%d stats_id %d -->",
		 cmd->vdev_id, cmd->stats_id);

	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		wmi_err("Failed to send WMI_REQUEST_STATS_CMDID");
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
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		wmi_err("Failed to send host stats request to fw");
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
		wmi_err("Failed to send host stats request to fw");
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
		wmi_err("Failed to send WMI link  status request to fw");
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
		wmi_err("Failed to send ap_ps_egap cmd");
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

	wmi_debug("vdev_id %d", vdev_id);
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
		wmi_err("Failed to send CSA offload enable command");
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
	wmi_debug("Sending OEM Data Request to target, data len %lu",
		 sizeof(*cfg));
	wmi_mtrace(WMI_OEM_DMA_RING_CFG_REQ_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cfg),
				WMI_OEM_DMA_RING_CFG_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI_OEM_DMA_RING_CFG_REQ_CMDID");
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

	wmi_debug("vdev %d sending 11D scan start req", cmd->vdev_id);

	wmi_mtrace(WMI_11D_SCAN_START_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_11D_SCAN_START_CMDID);
	if (ret) {
		wmi_err("Failed to send start 11d scan wmi cmd");
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

	wmi_debug("vdev %d sending 11D scan stop req", cmd->vdev_id);

	wmi_mtrace(WMI_11D_SCAN_STOP_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_11D_SCAN_STOP_CMDID);
	if (ret) {
		wmi_err("Failed to send stop 11d scan wmi cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_start_oem_data_cmd_tlv() - start OEM data request to target
 * @wmi_handle: wmi handle
 * @data_len: the length of @data
 * @data: the pointer to data buf
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

	wmi_debug("Sending OEM Data Request to target, data len %d", data_len);

	wmi_mtrace(WMI_OEM_REQ_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf,
				   (data_len +
				    WMI_TLV_HDR_SIZE), WMI_OEM_REQ_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI_OEM_REQ_CMDID");
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef FEATURE_OEM_DATA
/**
 * send_start_oemv2_data_cmd_tlv() - start OEM data to target
 * @wmi_handle: wmi handle
 * @oem_data: the pointer to oem data
 *
 * Return: QDF status
 */
static QDF_STATUS send_start_oemv2_data_cmd_tlv(wmi_unified_t wmi_handle,
						struct oem_data *oem_data)
{
	QDF_STATUS ret;
	wmi_oem_data_cmd_fixed_param *cmd;
	struct wmi_ops *ops;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);
	uint16_t oem_data_len_aligned;
	uint8_t *buf_ptr;
	uint32_t pdev_id;

	if (!oem_data || !oem_data->data) {
		wmi_err_rl("oem data is not valid");
		return QDF_STATUS_E_FAILURE;
	}

	oem_data_len_aligned = roundup(oem_data->data_len, sizeof(uint32_t));
	if (oem_data_len_aligned < oem_data->data_len) {
		wmi_err_rl("integer overflow while rounding up data_len");
		return QDF_STATUS_E_FAILURE;
	}

	if (oem_data_len_aligned > WMI_SVC_MSG_MAX_SIZE - WMI_TLV_HDR_SIZE) {
		wmi_err_rl("wmi_max_msg_size overflow for given data_len");
		return QDF_STATUS_E_FAILURE;
	}

	len += WMI_TLV_HDR_SIZE + oem_data_len_aligned;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_oem_data_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_oem_data_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_oem_data_cmd_fixed_param));

	pdev_id = oem_data->pdev_id;
	if (oem_data->pdev_vdev_flag) {
		ops = wmi_handle->ops;
		if (oem_data->is_host_pdev_id)
			pdev_id =
				ops->convert_host_pdev_id_to_target(wmi_handle,
								    pdev_id);
		else
			pdev_id =
				ops->convert_pdev_id_host_to_target(wmi_handle,
								    pdev_id);
	}

	cmd->vdev_id = oem_data->vdev_id;
	cmd->data_len = oem_data->data_len;
	cmd->pdev_vdev_flag = oem_data->pdev_vdev_flag;
	cmd->pdev_id = pdev_id;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, oem_data_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, oem_data->data, oem_data->data_len);

	wmi_mtrace(WMI_OEM_DATA_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_OEM_DATA_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err_rl("Failed with ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}
#endif

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
		wmi_debug("Phyerror Filtering offload is Disabled in ini");
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
			wmi_err("Failed to send WMI_DFS_PHYERR_FILTER_DIS_CMDID ret=%d",
				ret);
			wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
		}
		wmi_debug("WMI_DFS_PHYERR_FILTER_DIS_CMDID Send Success");
	} else {
		wmi_debug("Phyerror Filtering offload is Enabled in ini");

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
			wmi_err("Failed to send DFS PHYERR CMD ret=%d", ret);
			wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
		}
		wmi_debug("WMI_DFS_PHYERR_FILTER_ENA_CMDID Send Success");
	}

	return QDF_STATUS_SUCCESS;
}

#if !defined(REMOVE_PKT_LOG) && defined(FEATURE_PKTLOG)
/**
 * send_pktlog_wmi_send_cmd_tlv() - send pktlog enable/disable command to target
 * @wmi_handle: wmi handle
 * @pktlog_event: pktlog event
 * @cmd_id: pktlog cmd id
 * @user_triggered: user triggered input for PKTLOG enable mode
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
	int32_t idx, max_idx;

	PKTLOG_EVENT = pktlog_event;
	CMD_ID = cmd_id;

	max_idx = sizeof(pktlog_event_tlv) / (sizeof(pktlog_event_tlv[0]));
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
		cmd->evlist = 0;
		for (idx = 0; idx < max_idx; idx++) {
			if (PKTLOG_EVENT & (1 << idx))
				cmd->evlist |= pktlog_event_tlv[idx];
		}
		cmd->enable = user_triggered ? WMI_PKTLOG_ENABLE_FORCE
					: WMI_PKTLOG_ENABLE_AUTO;
		cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);
		wmi_mtrace(WMI_PDEV_PKTLOG_ENABLE_CMDID, NO_SESSION, 0);
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_ENABLE_CMDID)) {
			wmi_err("Failed to send pktlog enable cmdid");
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
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);
		wmi_mtrace(WMI_PDEV_PKTLOG_DISABLE_CMDID, NO_SESSION, 0);
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_DISABLE_CMDID)) {
			wmi_err("failed to send pktlog disable cmdid");
			goto wmi_send_failed;
		}
		break;
	default:
		wmi_debug("Invalid PKTLOG command: %d", CMD_ID);
		break;
	}

	return QDF_STATUS_SUCCESS;

wmi_send_failed:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}
#endif /* !REMOVE_PKT_LOG && FEATURE_PKTLOG */

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
	uint16_t max_wmi_msg_size = wmi_get_max_msg_len(wmi_handle);

	if (preq->request_data_len > (max_wmi_msg_size - WMI_TLV_HDR_SIZE -
				      sizeof(*cmd))) {
		wmi_err("Data length=%d is greater than max wmi msg size",
			preq->request_data_len);
		return QDF_STATUS_E_FAILURE;
	}

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

	wmi_debug("The data len value is %u and vdev id set is %u",
		 preq->request_data_len, preq->vdev_id);

	buf_ptr += sizeof(wmi_req_stats_ext_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, cmd->data_len);

	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, preq->request_data, cmd->data_len);

	wmi_mtrace(WMI_REQUEST_STATS_EXT_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_REQUEST_STATS_EXT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send notify cmd ret = %d", ret);
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
		wmi_err("Failed to send set_dhcp_server_offload cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debug("Set dhcp server offload to vdevId %d", params->vdev_id);

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
							wmi_handle,
							param->pdev_id);

	wmi_mtrace(WMI_PDEV_SET_REGDOMAIN_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_REGDOMAIN_CMDID)) {
		wmi_err("Failed to send pdev set regdomain command");
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

	wmi_debug("regd = %x, regd_2g = %x, regd_5g = %x, ctl_2g = %x, ctl_5g = %x",
		  cmd->reg_domain, cmd->reg_domain_2G, cmd->reg_domain_5G,
		  cmd->conformance_test_limit_2G,
		  cmd->conformance_test_limit_5G);

	wmi_mtrace(WMI_PDEV_SET_REGDOMAIN_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_REGDOMAIN_CMDID)) {
		wmi_err("Failed to send pdev set regdomain command");
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
	WMI_VDEV_CUSTOM_AGGR_256_BA_EN_SET(cmd->enable_bitmap,
					   param->aggr_ba_enable);
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

	wmi_debug("Set custom aggr: vdev id=0x%X, tx aggr size=0x%X "
		 "rx_aggr_size=0x%X access category=0x%X, agg_type=0x%X "
		 "tx_aggr_size_disable=0x%X, rx_aggr_size_disable=0x%X "
		 "tx_ac_enable=0x%X",
		 param->vdev_id, param->tx_aggr_size, param->rx_aggr_size,
		 param->ac, param->aggr_type, param->tx_aggr_size_disable,
		 param->rx_aggr_size_disable, param->tx_ac_enable);

	wmi_mtrace(WMI_VDEV_SET_CUSTOM_AGGR_SIZE_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_CUSTOM_AGGR_SIZE_CMDID)) {
		wmi_err("Setting custom aggregation size failed");
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
		wmi_err("Invalid Update Count!");
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
		wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							param->pdev_id);
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
		wmi_debug("Set QDepth Threshold: vdev=0x%X pdev=0x%X, tid=0x%X "
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
		wmi_err("Failed to send WMI_PEER_TID_MSDUQ_QDEPTH_THRESH_UPDATE_CMDID");
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

	wmi_debug("Setting dscp for vap id: %d", cmd->vdev_id);
	wmi_mtrace(WMI_VDEV_SET_DSCP_TID_MAP_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_DSCP_TID_MAP_CMDID)) {
			wmi_err("Failed to set dscp cmd");
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
		wmi_err("Setting FW test param failed");
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
							wmi_handle,
							WMI_HOST_PDEV_ID_SOC);

	wmi_mtrace(WMI_PDEV_DFS_DISABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_PDEV_DFS_DISABLE_CMDID);

	if (ret != 0) {
		wmi_err("Sending PDEV DFS disable cmd failed");
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
		wmi_err("Sending PDEV DFS enable cmd failed");
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
						wmi_handle,
						param->pdev_id);

	wmi_mtrace(WMI_SET_PERIODIC_CHANNEL_STATS_CONFIG_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_SET_PERIODIC_CHANNEL_STATS_CONFIG_CMDID);

	if (ret != 0) {
		wmi_err("Sending periodic chan stats config failed");
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef WLAN_IOT_SIM_SUPPORT
/**
 * send_simulation_test_cmd_tlv() - send simulation test command to fw
 *
 * @wmi_handle: wmi handle
 * @param: pointer to hold simulation test parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_simulation_test_cmd_tlv(wmi_unified_t wmi_handle,
					       struct simulation_test_params
					       *param)
{
	wmi_simulation_test_cmd_fixed_param *cmd;
	u32 wmi_buf_len;
	wmi_buf_t buf;
	u8 *buf_ptr;
	u32 aligned_len = 0;

	wmi_buf_len = sizeof(*cmd);
	if (param->buf_len) {
		aligned_len = roundup(param->buf_len, sizeof(A_UINT32));
		wmi_buf_len += WMI_TLV_HDR_SIZE + aligned_len;
	}

	buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_simulation_test_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_simulation_test_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
					wmi_simulation_test_cmd_fixed_param));
	cmd->pdev_id = param->pdev_id;
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_mac, &cmd->peer_macaddr);
	cmd->test_cmd_type = param->test_cmd_type;
	cmd->test_subcmd_type = param->test_subcmd_type;
	WMI_SIM_FRAME_TYPE_SET(cmd->frame_type_subtype_seq, param->frame_type);
	WMI_SIM_FRAME_SUBTYPE_SET(cmd->frame_type_subtype_seq,
				  param->frame_subtype);
	WMI_SIM_FRAME_SEQ_SET(cmd->frame_type_subtype_seq, param->seq);
	WMI_SIM_FRAME_OFFSET_SET(cmd->frame_offset_length, param->offset);
	WMI_SIM_FRAME_LENGTH_SET(cmd->frame_offset_length, param->frame_length);
	cmd->buf_len = param->buf_len;

	if (param->buf_len) {
		buf_ptr += sizeof(*cmd);
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, aligned_len);
		buf_ptr += WMI_TLV_HDR_SIZE;
		qdf_mem_copy(buf_ptr, param->bufp, param->buf_len);
	}

	if (wmi_unified_cmd_send(wmi_handle, buf, wmi_buf_len,
				 WMI_SIMULATION_TEST_CMDID)) {
		wmi_err("Failed to send test simulation cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_11BE
#define WLAN_PHY_CH_WIDTH_320MHZ CH_WIDTH_320MHZ
#else
#define WLAN_PHY_CH_WIDTH_320MHZ CH_WIDTH_INVALID
#endif
enum phy_ch_width wmi_map_ch_width(A_UINT32 wmi_width)
{
	switch (wmi_width) {
	case WMI_CHAN_WIDTH_20:
		return CH_WIDTH_20MHZ;
	case WMI_CHAN_WIDTH_40:
		return CH_WIDTH_40MHZ;
	case WMI_CHAN_WIDTH_80:
		return CH_WIDTH_80MHZ;
	case WMI_CHAN_WIDTH_160:
		return CH_WIDTH_160MHZ;
	case WMI_CHAN_WIDTH_80P80:
		return CH_WIDTH_80P80MHZ;
	case WMI_CHAN_WIDTH_5:
		return CH_WIDTH_5MHZ;
	case WMI_CHAN_WIDTH_10:
		return CH_WIDTH_10MHZ;
	case WMI_CHAN_WIDTH_320:
		return WLAN_PHY_CH_WIDTH_320MHZ;
	default:
		return CH_WIDTH_INVALID;
	}
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
	cmd->spectral_scan_mode = param->mode;
	cmd->spectral_scan_center_freq1 = param->center_freq1;
	cmd->spectral_scan_center_freq2 = param->center_freq2;
	cmd->spectral_scan_chan_width = param->chan_width;
	/* Not used, fill with zeros */
	cmd->spectral_scan_chan_freq = 0;

	wmi_mtrace(WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID);

	if (ret != 0) {
		wmi_err("Sending set quiet cmd failed");
		wmi_buf_free(buf);
	}

	wmi_debug("Sent WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID");
	wmi_debug("vdev_id: %u spectral_scan_count: %u",
		 param->vdev_id, param->count);
	wmi_debug("spectral_scan_period: %u spectral_scan_priority: %u",
		 param->period, param->spectral_pri);
	wmi_debug("spectral_scan_fft_size: %u spectral_scan_gc_ena: %u",
		 param->fft_size, param->gc_enable);
	wmi_debug("spectral_scan_restart_ena: %u", param->restart_enable);
	wmi_debug("spectral_scan_noise_floor_ref: %u", param->noise_floor_ref);
	wmi_debug("spectral_scan_init_delay: %u", param->init_delay);
	wmi_debug("spectral_scan_nb_tone_thr: %u", param->nb_tone_thr);
	wmi_debug("spectral_scan_str_bin_thr: %u", param->str_bin_thr);
	wmi_debug("spectral_scan_wb_rpt_mode: %u", param->wb_rpt_mode);
	wmi_debug("spectral_scan_rssi_rpt_mode: %u", param->rssi_rpt_mode);
	wmi_debug("spectral_scan_rssi_thr: %u spectral_scan_pwr_format: %u",
		 param->rssi_thr, param->pwr_format);
	wmi_debug("spectral_scan_rpt_mode: %u spectral_scan_bin_scale: %u",
		 param->rpt_mode, param->bin_scale);
	wmi_debug("spectral_scan_dBm_adj: %u spectral_scan_chn_mask: %u",
		 param->dbm_adj, param->chn_mask);
	wmi_debug("spectral_scan_mode: %u spectral_scan_center_freq1: %u",
		 param->mode, param->center_freq1);
	wmi_debug("spectral_scan_center_freq2: %u spectral_scan_chan_freq: %u",
		 param->center_freq2, param->chan_freq);
	wmi_debug("spectral_scan_chan_width: %u Status: %d",
		 param->chan_width, ret);

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
	cmd->spectral_scan_mode = param->mode;

	wmi_debug("vdev_id = %u trigger_cmd = %u enable_cmd = %u",
		 cmd->vdev_id, cmd->trigger_cmd, cmd->enable_cmd);
	wmi_debug("spectral_scan_mode = %u", cmd->spectral_scan_mode);

	wmi_mtrace(WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID);

	if (ret != 0) {
		wmi_err("Sending scan enable CMD failed");
		wmi_buf_free(buf);
	}

	wmi_debug("Sent WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID, Status: %d",
		  ret);

	return ret;
}

#ifdef WLAN_CONV_SPECTRAL_ENABLE
static QDF_STATUS
extract_pdev_sscan_fw_cmd_fixed_param_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *event, struct spectral_startscan_resp_params *param)
{
	WMI_PDEV_SSCAN_FW_PARAM_EVENTID_param_tlvs *param_buf;
	wmi_pdev_sscan_fw_cmd_fixed_param *ev;

	if (!wmi_handle) {
		wmi_err("WMI handle is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!event) {
		wmi_err("WMI event is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		wmi_err("Spectral startscan response params is null");
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_PDEV_SSCAN_FW_PARAM_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_INVAL;

	param->pdev_id = wmi_handle->ops->convert_target_pdev_id_to_host(
								wmi_handle,
								ev->pdev_id);
	param->smode = ev->spectral_scan_mode;
	param->num_fft_bin_index = param_buf->num_fft_bin_index;
	wmi_debug("pdev id %u scan mode %u num_fft_bin_index %u",
		 param->pdev_id, param->smode, param->num_fft_bin_index);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_pdev_sscan_fft_bin_index_tlv(
			wmi_unified_t wmi_handle, uint8_t *event,
			struct spectral_fft_bin_markers_160_165mhz *param)
{
	WMI_PDEV_SSCAN_FW_PARAM_EVENTID_param_tlvs *param_buf;
	wmi_pdev_sscan_fft_bin_index *ev;

	param_buf = (WMI_PDEV_SSCAN_FW_PARAM_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->fft_bin_index;
	if (!ev)
		return QDF_STATUS_E_INVAL;

	param->start_pri80 = WMI_SSCAN_PRI80_START_BIN_GET(ev->pri80_bins);
	param->num_pri80 = WMI_SSCAN_PRI80_END_BIN_GET(ev->pri80_bins) -
			   param->start_pri80 + 1;
	param->start_sec80 = WMI_SSCAN_SEC80_START_BIN_GET(ev->sec80_bins);
	param->num_sec80 = WMI_SSCAN_SEC80_END_BIN_GET(ev->sec80_bins) -
			   param->start_sec80 + 1;
	param->start_5mhz = WMI_SSCAN_MID_5MHZ_START_BIN_GET(ev->mid_5mhz_bins);
	param->num_5mhz = WMI_SSCAN_MID_5MHZ_END_BIN_GET(ev->mid_5mhz_bins) -
			  param->start_5mhz + 1;
	param->is_valid = true;

	wmi_debug("start_pri80: %u num_pri80: %u start_sec80: %u num_sec80: %u start_5mhz: %u, num_5mhz: %u",
		 param->start_pri80, param->num_pri80,
		 param->start_sec80, param->num_sec80,
		 param->start_5mhz, param->num_5mhz);

	return QDF_STATUS_SUCCESS;
}

#ifdef SPECTRAL_BERYLLIUM
/**
 * extract_pdev_spectral_session_chan_info_tlv() - Extract channel information
 * for a spectral scan session
 * @wmi_handle: handle to WMI.
 * @event: Event buffer
 * @chan_info: Spectral session channel information data structure to be filled
 * by this API
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
extract_pdev_spectral_session_chan_info_tlv(
			wmi_unified_t wmi_handle, void *event,
			struct spectral_session_chan_info *chan_info)
{
	WMI_PDEV_SSCAN_FW_PARAM_EVENTID_param_tlvs *param_buf = event;
	wmi_pdev_sscan_chan_info *chan_info_tlv;

	if (!param_buf) {
		wmi_err("param_buf is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!chan_info) {
		wmi_err("chan_info is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	chan_info_tlv = param_buf->chan_info;
	if (!chan_info_tlv) {
		wmi_err("chan_info tlv is not present in the event");
		return QDF_STATUS_E_NULL_VALUE;
	}

	chan_info->operating_pri20_freq =
			(qdf_freq_t)chan_info_tlv->operating_pri20_freq;
	chan_info->operating_cfreq1 =
			(qdf_freq_t)chan_info_tlv->operating_cfreq1;
	chan_info->operating_cfreq2 =
			(qdf_freq_t)chan_info_tlv->operating_cfreq2;
	chan_info->operating_bw = wmi_map_ch_width(chan_info_tlv->operating_bw);
	chan_info->operating_puncture_20mhz_bitmap =
		chan_info_tlv->operating_puncture_20mhz_bitmap;

	chan_info->sscan_cfreq1 = (qdf_freq_t)chan_info_tlv->sscan_cfreq1;
	chan_info->sscan_cfreq2 = (qdf_freq_t)chan_info_tlv->sscan_cfreq2;
	chan_info->sscan_bw = wmi_map_ch_width(chan_info_tlv->sscan_bw);
	chan_info->sscan_puncture_20mhz_bitmap =
		chan_info_tlv->sscan_puncture_20mhz_bitmap;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_pdev_spectral_session_detector_info_tlv(
			wmi_unified_t wmi_handle, void *event,
			struct spectral_session_det_info *det_info, uint8_t idx)
{
	WMI_PDEV_SSCAN_FW_PARAM_EVENTID_param_tlvs *param_buf = event;
	wmi_pdev_sscan_detector_info *det_info_tlv;

	if (!param_buf) {
		wmi_err("param_buf is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!det_info) {
		wmi_err("chan_info is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!param_buf->det_info) {
		wmi_err("det_info tlv is not present in the event");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (idx >= param_buf->num_det_info) {
		wmi_err("det_info index(%u) is greater than or equal to %u",
			idx, param_buf->num_det_info);
		return QDF_STATUS_E_FAILURE;
	}

	det_info_tlv = &param_buf->det_info[idx];

	det_info->det_id = det_info_tlv->detector_id;
	det_info->start_freq = (qdf_freq_t)det_info_tlv->start_freq;
	det_info->end_freq = (qdf_freq_t)det_info_tlv->end_freq;

	return QDF_STATUS_SUCCESS;
}
#endif /* SPECTRAL_BERYLLIUM */
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

#ifdef FEATURE_WPSS_THERMAL_MITIGATION
static inline void
wmi_fill_client_id_priority(wmi_therm_throt_config_request_fixed_param *tt_conf,
			    struct thermal_mitigation_params *param)
{
	tt_conf->client_id = param->client_id;
	tt_conf->priority = param->priority;
}
#else
static inline void
wmi_fill_client_id_priority(wmi_therm_throt_config_request_fixed_param *tt_conf,
			    struct thermal_mitigation_params *param)
{
}
#endif

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
			param->num_thermal_conf *
			sizeof(wmi_therm_throt_level_config_info);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	tt_conf = (wmi_therm_throt_config_request_fixed_param *) wmi_buf_data(buf);

	/* init fixed params */
	WMITLV_SET_HDR(tt_conf,
		WMITLV_TAG_STRUC_wmi_therm_throt_config_request_fixed_param,
		(WMITLV_GET_STRUCT_TLVLEN(wmi_therm_throt_config_request_fixed_param)));

	tt_conf->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								param->pdev_id);
	tt_conf->enable = param->enable;
	tt_conf->dc = param->dc;
	tt_conf->dc_per_event = param->dc_per_event;
	tt_conf->therm_throt_levels = param->num_thermal_conf;
	wmi_fill_client_id_priority(tt_conf, param);
	buf_ptr = (uint8_t *) ++tt_conf;
	/* init TLV params */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			(param->num_thermal_conf *
			sizeof(wmi_therm_throt_level_config_info)));

	lvl_conf = (wmi_therm_throt_level_config_info *) (buf_ptr +  WMI_TLV_HDR_SIZE);
	for (i = 0; i < param->num_thermal_conf; i++) {
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
		wmi_err("Failed to send WMI_THERM_THROT_SET_CONF_CMDID command");
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
		wmi_err("Sending COEX CONFIG CMD failed");
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

#ifdef WLAN_FEATURE_NAN
static void wmi_set_nan_channel_support(wmi_resource_config *resource_cfg)
{
	WMI_RSRC_CFG_HOST_SERVICE_FLAG_NAN_CHANNEL_SUPPORT_SET(
		resource_cfg->host_service_flags, 1);
}
#else
static inline
void wmi_set_nan_channel_support(wmi_resource_config *resource_cfg)
{
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
	resource_cfg->max_num_group_keys = tgt_res_cfg->max_num_group_keys;
	/* Deferred AI: Max rnr neighbors supported in multisoc case
	 * where in SoC can support 6ghz. During WMI init of a SoC
	 * currently there is no way to figure if another SOC is plugged in
	 * and it can support 6Ghz.
	 */
	resource_cfg->max_rnr_neighbours = MAX_SUPPORTED_NEIGHBORS;
	resource_cfg->ema_max_vap_cnt = tgt_res_cfg->ema_max_vap_cnt;
	resource_cfg->ema_max_profile_period =
			tgt_res_cfg->ema_max_profile_period;
	resource_cfg->ema_init_config = tgt_res_cfg->ema_init_config;
	resource_cfg->carrier_config = tgt_res_cfg->carrier_profile_config;

	if (tgt_res_cfg->max_ndp_sessions)
		resource_cfg->max_ndp_sessions =
				tgt_res_cfg->max_ndp_sessions;
	resource_cfg->max_ndi_interfaces = tgt_res_cfg->max_ndi;

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
	if (tgt_res_cfg->enable_pci_gen)
		WMI_RSRC_CFG_FLAG_PCIE_GEN_SWITCH_CAPABLITY_SET(
			resource_cfg->flag1, 1);
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

	if (tgt_res_cfg->three_way_coex_config_legacy_en)
		WMI_RSRC_CFG_FLAG_THREE_WAY_COEX_CONFIG_LEGACY_SUPPORT_SET(
						resource_cfg->flag1, 1);
	if (tgt_res_cfg->pktcapture_support)
		WMI_RSRC_CFG_FLAG_PACKET_CAPTURE_SUPPORT_SET(
				resource_cfg->flag1, 1);

	/*
	 * Control padding using config param/ini of iphdr_pad_config
	 */
	if (tgt_res_cfg->iphdr_pad_config)
		WMI_RSRC_CFG_FLAG_IPHR_PAD_CONFIG_ENABLE_SET(
			resource_cfg->flag1, 1);

	WMI_RSRC_CFG_FLAG_IPA_DISABLE_SET(resource_cfg->flag1,
					  tgt_res_cfg->ipa_disable);

	if (tgt_res_cfg->time_sync_ftm)
		WMI_RSRC_CFG_FLAG_AUDIO_SYNC_SUPPORT_SET(resource_cfg->flag1,
							 1);

	wmi_copy_twt_resource_config(resource_cfg, tgt_res_cfg);
	resource_cfg->peer_map_unmap_versions =
		tgt_res_cfg->peer_map_unmap_version;
	resource_cfg->smart_ant_cap = tgt_res_cfg->smart_ant_cap;
	if (tgt_res_cfg->re_ul_resp)
		WMI_SET_BITS(resource_cfg->flags2, 0, 4,
			     tgt_res_cfg->re_ul_resp);


	/*
	 * Enable ast flow override per peer
	 */
	resource_cfg->msdu_flow_override_config0 = 0;
	WMI_MSDU_FLOW_AST_ENABLE_SET(
			resource_cfg->msdu_flow_override_config0,
			WMI_CONFIG_MSDU_AST_INDEX_1,
			tgt_res_cfg->ast_1_valid_mask_enable);

	WMI_MSDU_FLOW_AST_ENABLE_SET(
			resource_cfg->msdu_flow_override_config0,
			WMI_CONFIG_MSDU_AST_INDEX_2,
			tgt_res_cfg->ast_2_valid_mask_enable);

	WMI_MSDU_FLOW_AST_ENABLE_SET(
			resource_cfg->msdu_flow_override_config0,
			WMI_CONFIG_MSDU_AST_INDEX_3,
			tgt_res_cfg->ast_3_valid_mask_enable);

	/*
	 * Enable ast flow mask and TID valid mask configurations
	 */
	resource_cfg->msdu_flow_override_config1 = 0;

	/*Enable UDP flow for Ast index 0*/
	WMI_MSDU_FLOW_ASTX_MSDU_FLOW_MASKS_SET(
		resource_cfg->msdu_flow_override_config1,
		WMI_CONFIG_MSDU_AST_INDEX_0,
		tgt_res_cfg->ast_0_flow_mask_enable);

	/*Enable Non UDP flow for Ast index 1*/
	WMI_MSDU_FLOW_ASTX_MSDU_FLOW_MASKS_SET(
		resource_cfg->msdu_flow_override_config1,
		WMI_CONFIG_MSDU_AST_INDEX_1,
		tgt_res_cfg->ast_1_flow_mask_enable);

	/*Enable Hi-Priority flow for Ast index 2*/
	WMI_MSDU_FLOW_ASTX_MSDU_FLOW_MASKS_SET(
		resource_cfg->msdu_flow_override_config1,
		WMI_CONFIG_MSDU_AST_INDEX_2,
		tgt_res_cfg->ast_2_flow_mask_enable);

	/*Enable Low-Priority flow for Ast index 3*/
	WMI_MSDU_FLOW_ASTX_MSDU_FLOW_MASKS_SET(
		resource_cfg->msdu_flow_override_config1,
		WMI_CONFIG_MSDU_AST_INDEX_3,
		tgt_res_cfg->ast_3_flow_mask_enable);

	/*Enable all 8 tid for Hi-Pririty Flow Queue*/
	WMI_MSDU_FLOW_TID_VALID_HI_MASKS_SET(
		resource_cfg->msdu_flow_override_config1,
		tgt_res_cfg->ast_tid_high_mask_enable);

	/*Enable all 8 tid for Low-Pririty Flow Queue*/
	WMI_MSDU_FLOW_TID_VALID_LOW_MASKS_SET(
		resource_cfg->msdu_flow_override_config1,
		tgt_res_cfg->ast_tid_low_mask_enable);
	WMI_RSRC_CFG_HOST_SERVICE_FLAG_NAN_IFACE_SUPPORT_SET(
		resource_cfg->host_service_flags,
		tgt_res_cfg->nan_separate_iface_support);
	WMI_RSRC_CFG_HOST_SERVICE_FLAG_HOST_SUPPORT_MULTI_RADIO_EVTS_PER_RADIO_SET(
		resource_cfg->host_service_flags, 1);

	WMI_RSRC_CFG_FLAG_VIDEO_OVER_WIFI_ENABLE_SET(
		resource_cfg->flag1, tgt_res_cfg->carrier_vow_optimization);

	if (tgt_res_cfg->is_sap_connected_d3wow_enabled)
		WMI_RSRC_CFG_FLAGS2_IS_SAP_CONNECTED_D3WOW_ENABLED_SET(
			resource_cfg->flags2, 1);
	if (tgt_res_cfg->is_go_connected_d3wow_enabled)
		WMI_RSRC_CFG_FLAGS2_IS_GO_CONNECTED_D3WOW_ENABLED_SET(
			resource_cfg->flags2, 1);

	if (tgt_res_cfg->sae_eapol_offload)
		WMI_RSRC_CFG_HOST_SERVICE_FLAG_SAE_EAPOL_OFFLOAD_SUPPORT_SET(
			resource_cfg->host_service_flags, 1);

	WMI_RSRC_CFG_HOST_SERVICE_FLAG_REG_CC_EXT_SUPPORT_SET(
		resource_cfg->host_service_flags,
		tgt_res_cfg->is_reg_cc_ext_event_supported);

	WMI_RSRC_CFG_HOST_SERVICE_FLAG_LPI_SP_MODE_SUPPORT_SET(
		resource_cfg->host_service_flags,
		tgt_res_cfg->is_6ghz_sp_pwrmode_supp_enabled);

	WMI_RSRC_CFG_HOST_SERVICE_FLAG_REG_DISCARD_AFC_TIMER_CHECK_SET(
		resource_cfg->host_service_flags,
		tgt_res_cfg->afc_timer_check_disable);

	WMI_RSRC_CFG_HOST_SERVICE_FLAG_REG_DISCARD_AFC_REQ_ID_CHECK_SET(
		resource_cfg->host_service_flags,
		tgt_res_cfg->afc_req_id_check_disable);

	wmi_set_nan_channel_support(resource_cfg);

	if (tgt_res_cfg->twt_ack_support_cap)
		WMI_RSRC_CFG_HOST_SERVICE_FLAG_STA_TWT_SYNC_EVT_SUPPORT_SET(
			resource_cfg->host_service_flags, 1);

	WMI_RSRC_CFG_FLAGS2_RX_PEER_METADATA_VERSION_SET(resource_cfg->flags2,
						 tgt_res_cfg->target_cap_flags);
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
					wmi_handle,
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
		wmi_err("HE TB action frame cmnd send fail, ret %d", ret);
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
		wmi_debug("Error: Incompatible WMI version."
			"Host: %d,%d,0x%x 0x%x 0x%x 0x%x, FW: %d,%d,0x%x 0x%x 0x%x 0x%x",
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

	wmi_debug("Received WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID");

	param_buf = (WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		wmi_err("Invalid log supported event buffer");
		return QDF_STATUS_E_INVAL;
	}
	wmi_event = param_buf->fixed_param;
	num_of_diag_events_logs = wmi_event->num_of_diag_events_logs;

	if (num_of_diag_events_logs >
	    param_buf->num_diag_events_logs_list) {
		wmi_err("message number of events %d is more than tlv hdr content %d",
			 num_of_diag_events_logs,
			 param_buf->num_diag_events_logs_list);
		return QDF_STATUS_E_INVAL;
	}

	evt_args = param_buf->diag_events_logs_list;
	if (!evt_args) {
		wmi_err("Event list is empty, num_of_diag_events_logs=%d",
			num_of_diag_events_logs);
		return QDF_STATUS_E_INVAL;
	}

	wmi_debug("num_of_diag_events_logs=%d", num_of_diag_events_logs);

	/* Free any previous allocation */
	if (wmi_handle->events_logs_list) {
		qdf_mem_free(wmi_handle->events_logs_list);
		wmi_handle->events_logs_list = NULL;
	}

	if (num_of_diag_events_logs >
		(WMI_SVC_MSG_MAX_SIZE / sizeof(uint32_t))) {
		wmi_err("excess num of logs: %d", num_of_diag_events_logs);
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
		wmi_err("WMI_DIAG_EVENT_LOG_CONFIG_CMDID failed");
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
		wmi_debug("Not received event/log list from FW, yet");
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

	wmi_debug("Length: %d Log_level: %d", total_len, log_level);
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
			wmi_debug("Idx:%d, val:%x", i, val);
			count++;
		}
	}

	wmi_mtrace(WMI_DIAG_EVENT_LOG_CONFIG_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_DIAG_EVENT_LOG_CONFIG_CMDID)) {
		wmi_err("WMI_DIAG_EVENT_LOG_CONFIG_CMDID failed");
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
		wmi_err("Failed to send WMI_DEBUG_MESG_FLUSH_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}
	wmi_debug("Sent WMI_DEBUG_MESG_FLUSH_CMDID to FW");

	return ret;
}

#ifdef BIG_ENDIAN_HOST
#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
/**
 * fips_extend_align_data_be() - LE to BE conversion of FIPS extend ev data
 * @param - fips extend param related parameters
 *
 * Return: QDF_STATUS - success or error status
*/
static QDF_STATUS fips_extend_align_data_be(wmi_unified_t wmi_handle,
					    struct fips_extend_params *param)
{
	unsigned char *key_unaligned, *nonce_iv_unaligned, *data_unaligned;
	int c;
	u_int8_t *key_aligned = NULL;
	u_int8_t *nonce_iv_aligned = NULL;
	u_int8_t *data_aligned = NULL;
	int ret = QDF_STATUS_SUCCESS;

	/* Assigning unaligned space to copy the key */
	key_unaligned = qdf_mem_malloc(sizeof(u_int8_t) *
				       param->cmd_params.key_len + FIPS_ALIGN);
	/* Checking if kmalloc is successful to allocate space */
	if (!key_unaligned)
		return QDF_STATUS_E_INVAL;

	data_unaligned = qdf_mem_malloc(sizeof(u_int8_t) * param->data_len +
					FIPS_ALIGN);
	/* Checking if kmalloc is successful to allocate space */
	if (!data_unaligned) {
		ret = QDF_STATUS_E_INVAL;
		goto fips_align_fail_data;
	}

	/* Checking if space is aligned */
	if (!FIPS_IS_ALIGNED(key_unaligned, FIPS_ALIGN)) {
		/* align to 4 */
		key_aligned = (u_int8_t *)FIPS_ALIGNTO(key_unaligned,
						       FIPS_ALIGN);
	} else {
		key_aligned = (u_int8_t *)key_unaligned;
	}

	/* memset and copy content from key to key aligned */
	OS_MEMSET(key_aligned, 0, param->cmd_params.key_len);
	OS_MEMCPY(key_aligned, param->cmd_params.key,
		  param->cmd_params.key_len);

	/* print a hexdump for host debug */
	wmi_debug("Aligned and Copied Key: ");
	qdf_trace_hex_dump(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   key_aligned, param->cmd_params.key_len);

	/* Checking of space is aligned */
	if (!FIPS_IS_ALIGNED(data_unaligned, FIPS_ALIGN)) {
		/* align to 4 */
		data_aligned =
		(u_int8_t *)FIPS_ALIGNTO(data_unaligned, FIPS_ALIGN);
	} else {
		data_aligned = (u_int8_t *)data_unaligned;
	}

	/* memset and copy content from data to data aligned */
	OS_MEMSET(data_aligned, 0, param->data_len);
	OS_MEMCPY(data_aligned, param->data, param->data_len);

	/* print a hexdump for host debug */
	wmi_debug("\t Properly Aligned and Copied Data: ");
	qdf_trace_hex_dump(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   data_aligned, param->data_len);

	/* converting to little Endian */
	for (c = 0; c < param->cmd_params.key_len / 4; c++) {
		*((u_int32_t *)key_aligned + c) =
		qdf_cpu_to_le32(*((u_int32_t *)key_aligned + c));
	}
	for (c = 0; c < param->data_len / 4; c++) {
		*((u_int32_t *)data_aligned + c) =
		qdf_cpu_to_le32(*((u_int32_t *)data_aligned + c));
	}

	/* update endian data */
	OS_MEMCPY(param->cmd_params.key, key_aligned,
		  param->cmd_params.key_len);
	OS_MEMCPY(param->data, data_aligned, param->data_len);

	if (param->cmd_params.nonce_iv_len) {
		nonce_iv_unaligned = qdf_mem_malloc(sizeof(u_int8_t) *
						    param->cmd_params.nonce_iv_len +
						    FIPS_ALIGN);

		/* Checking if kmalloc is successful to allocate space */
		if (!nonce_iv_unaligned) {
			ret = QDF_STATUS_E_INVAL;
			goto fips_align_fail_nonce_iv;
		}
		/* Checking if space is aligned */
		if (!FIPS_IS_ALIGNED(nonce_iv_unaligned, FIPS_ALIGN)) {
			/* align to 4 */
			nonce_iv_aligned =
			(u_int8_t *)FIPS_ALIGNTO(nonce_iv_unaligned,
				FIPS_ALIGN);
		} else {
			nonce_iv_aligned = (u_int8_t *)nonce_iv_unaligned;
		}

		/* memset and copy content from iv to iv aligned */
		OS_MEMSET(nonce_iv_aligned, 0, param->cmd_params.nonce_iv_len);
		OS_MEMCPY(nonce_iv_aligned, param->cmd_params.nonce_iv,
			  param->cmd_params.nonce_iv_len);

		/* print a hexdump for host debug */
		wmi_debug("\t Aligned and Copied Nonce_IV: ");
		qdf_trace_hex_dump(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
				   nonce_iv_aligned,
				   param->cmd_params.nonce_iv_len);

		for (c = 0; c < param->cmd_params.nonce_iv_len / 4; c++) {
			*((u_int32_t *)nonce_iv_aligned + c) =
			qdf_cpu_to_le32(*((u_int32_t *)nonce_iv_aligned + c));
		}
	}

	/* clean up allocated spaces */
	qdf_mem_free(nonce_iv_unaligned);
	nonce_iv_unaligned = NULL;
	nonce_iv_aligned = NULL;

fips_align_fail_nonce_iv:
	qdf_mem_free(data_unaligned);
	data_unaligned = NULL;
	data_aligned = NULL;

fips_align_fail_data:
	qdf_mem_free(key_unaligned);
	key_unaligned = NULL;
	key_aligned = NULL;

	return ret;
}
#endif

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
#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
/**
 * fips_extend_align_data_be() - DUMMY for LE platform
 *
 * Return: QDF_STATUS - success
 */
static QDF_STATUS fips_extend_align_data_be(wmi_unified_t wmi_handle,
					    struct fips_extend_params *param)
{
	return QDF_STATUS_SUCCESS;
}
#endif

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

#ifdef WLAN_FEATURE_DISA
/**
 * send_encrypt_decrypt_send_cmd() - send encrypt/decrypt cmd to fw
 * @wmi_handle: wmi handle
 * @params: encrypt/decrypt params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_encrypt_decrypt_send_cmd_tlv(wmi_unified_t wmi_handle,
				  struct disa_encrypt_decrypt_req_params
				  *encrypt_decrypt_params)
{
	wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	QDF_STATUS ret;
	uint32_t len;

	wmi_debug("Send encrypt decrypt cmd");

	len = sizeof(*cmd) +
			encrypt_decrypt_params->data_len +
			WMI_TLV_HDR_SIZE;
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = wmi_buf_data(wmi_buf);
	cmd = (wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param));

	cmd->vdev_id = encrypt_decrypt_params->vdev_id;
	cmd->key_flag = encrypt_decrypt_params->key_flag;
	cmd->key_idx = encrypt_decrypt_params->key_idx;
	cmd->key_cipher = encrypt_decrypt_params->key_cipher;
	cmd->key_len = encrypt_decrypt_params->key_len;
	cmd->key_txmic_len = encrypt_decrypt_params->key_txmic_len;
	cmd->key_rxmic_len = encrypt_decrypt_params->key_rxmic_len;

	qdf_mem_copy(cmd->key_data, encrypt_decrypt_params->key_data,
		     encrypt_decrypt_params->key_len);

	qdf_mem_copy(cmd->mac_hdr, encrypt_decrypt_params->mac_header,
		     MAX_MAC_HEADER_LEN);

	cmd->data_len = encrypt_decrypt_params->data_len;

	if (cmd->data_len) {
		buf_ptr += sizeof(*cmd);
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			       roundup(encrypt_decrypt_params->data_len,
				       sizeof(uint32_t)));
		buf_ptr += WMI_TLV_HDR_SIZE;
		qdf_mem_copy(buf_ptr, encrypt_decrypt_params->data,
			     encrypt_decrypt_params->data_len);
	}

	/* This conversion is to facilitate data to FW in little endian */
	cmd->pn[5] = encrypt_decrypt_params->pn[0];
	cmd->pn[4] = encrypt_decrypt_params->pn[1];
	cmd->pn[3] = encrypt_decrypt_params->pn[2];
	cmd->pn[2] = encrypt_decrypt_params->pn[3];
	cmd->pn[1] = encrypt_decrypt_params->pn[4];
	cmd->pn[0] = encrypt_decrypt_params->pn[5];

	wmi_mtrace(WMI_VDEV_ENCRYPT_DECRYPT_DATA_REQ_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, len,
				   WMI_VDEV_ENCRYPT_DECRYPT_DATA_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send ENCRYPT DECRYPT cmd: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}
#endif /* WLAN_FEATURE_DISA */

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
								wmi_handle,
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

#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
/**
 * send_pdev_fips_extend_cmd_tlv() - send pdev fips cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold pdev fips param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_fips_extend_cmd_tlv(wmi_unified_t wmi_handle,
			      struct fips_extend_params *param)
{
	wmi_pdev_fips_extend_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len = sizeof(wmi_pdev_fips_extend_cmd_fixed_param);
	uint32_t data_len_aligned;
	QDF_STATUS retval = QDF_STATUS_SUCCESS;

	len += WMI_TLV_HDR_SIZE;
	if (param->frag_idx == 0)
		len += sizeof(wmi_fips_extend_cmd_init_params);

	/* Length TLV placeholder for array of bytes */
	len += WMI_TLV_HDR_SIZE;
	if (param->data_len) {
		data_len_aligned = roundup(param->data_len, sizeof(uint32_t));
		len += (data_len_aligned * sizeof(uint8_t));
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_pdev_fips_extend_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_fips_extend_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_pdev_fips_extend_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								param->pdev_id);

	cmd->fips_cookie = param->cookie;
	cmd->frag_idx = param->frag_idx;
	cmd->more_bit = param->more_bit;
	cmd->data_len = param->data_len;

	if (fips_extend_align_data_be(wmi_handle, param) !=
	    QDF_STATUS_SUCCESS) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)(cmd + 1);
	if (cmd->frag_idx == 0) {
		wmi_fips_extend_cmd_init_params *cmd_params;

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_fips_extend_cmd_init_params));
		buf_ptr += WMI_TLV_HDR_SIZE;
		cmd_params = (wmi_fips_extend_cmd_init_params *)buf_ptr;
		WMITLV_SET_HDR(buf_ptr,
			       WMITLV_TAG_STRUC_wmi_fips_extend_cmd_init_params,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_fips_extend_cmd_init_params));
		cmd_params->fips_cmd = param->cmd_params.fips_cmd;
		cmd_params->key_cipher = param->cmd_params.key_cipher;
		cmd_params->key_len = param->cmd_params.key_len;
		cmd_params->nonce_iv_len = param->cmd_params.nonce_iv_len;
		cmd_params->tag_len = param->cmd_params.tag_len;
		cmd_params->aad_len = param->cmd_params.aad_len;
		cmd_params->payload_len = param->cmd_params.payload_len;

		qdf_mem_copy(cmd_params->key, param->cmd_params.key,
			     param->cmd_params.key_len);
		qdf_mem_copy(cmd_params->nonce_iv, param->cmd_params.nonce_iv,
			     param->cmd_params.nonce_iv_len);

		wmi_debug("Key len = %d, IVNoncelen = %d, Tlen = %d, Alen = %d, Plen = %d",
			  cmd_params->key_len, cmd_params->nonce_iv_len,
			  cmd_params->tag_len, cmd_params->aad_len,
			  cmd_params->payload_len);

		buf_ptr += sizeof(wmi_fips_extend_cmd_init_params);
	} else {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;
	}

	if (param->data_len && param->data) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			       data_len_aligned);

		buf_ptr += WMI_TLV_HDR_SIZE;
		if (param->data_len)
			qdf_mem_copy(buf_ptr,
				     (uint8_t *)param->data, param->data_len);

		wmi_debug("Data: ");
		qdf_trace_hex_dump(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
				   buf_ptr, cmd->data_len);

		if (param->data_len)
			buf_ptr += param->data_len;

		wmi_mtrace(WMI_PDEV_FIPS_EXTEND_CMDID, NO_SESSION, 0);
		retval = wmi_unified_cmd_send(wmi_handle, buf, len,
					      WMI_PDEV_FIPS_EXTEND_CMDID);
		wmi_debug("return value %d", retval);
	} else {
		wmi_debug("Key or Data is NULL");
		wmi_buf_free(buf);
		retval = QDF_STATUS_E_BADMSG;
	}

	return retval;
}

/**
 * send_pdev_fips_mode_set_cmd_tlv() - send pdev fips cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold pdev fips param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_fips_mode_set_cmd_tlv(wmi_unified_t wmi_handle,
				struct fips_mode_set_params *param)
{
	wmi_pdev_fips_mode_set_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len = sizeof(wmi_pdev_fips_mode_set_cmd_fixed_param);
	QDF_STATUS retval = QDF_STATUS_SUCCESS;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_pdev_fips_mode_set_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_fips_mode_set_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_pdev_fips_mode_set_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								param->pdev_id);

	cmd->fips_mode_set = param->mode;
	wmi_mtrace(WMI_PDEV_FIPS_MODE_SET_CMDID, NO_SESSION, 0);
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PDEV_FIPS_MODE_SET_CMDID);
	if (retval) {
		wmi_err("Failed to send FIPS mode enable cmd");
		wmi_buf_free(buf);
	}
	return retval;
}
#endif

/**
 * send_wlan_profile_enable_cmd_tlv() - send wlan profile enable command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wlan profile param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wlan_profile_enable_cmd_tlv(wmi_unified_t wmi_handle,
				 struct wlan_profile_params *param)
{
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;
	wmi_wlan_profile_enable_profile_id_cmd_fixed_param *profile_enable_cmd;

	len = sizeof(wmi_wlan_profile_enable_profile_id_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to send WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID");
		return QDF_STATUS_E_NOMEM;
	}

	profile_enable_cmd =
		(wmi_wlan_profile_enable_profile_id_cmd_fixed_param *)
			wmi_buf_data(buf);
	WMITLV_SET_HDR(&profile_enable_cmd->tlv_header,
	    WMITLV_TAG_STRUC_wmi_wlan_profile_enable_profile_id_cmd_fixed_param,
	    WMITLV_GET_STRUCT_TLVLEN
	    (wmi_wlan_profile_enable_profile_id_cmd_fixed_param));

	profile_enable_cmd->profile_id = param->profile_id;
	profile_enable_cmd->enable = param->enable;
	wmi_mtrace(WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
		   NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID);
	if (ret) {
		wmi_err("Failed to send PROFILE_ENABLE_PROFILE_ID_CMDID");
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_wlan_profile_trigger_cmd_tlv() - send wlan profile trigger command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wlan profile param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wlan_profile_trigger_cmd_tlv(wmi_unified_t wmi_handle,
				  struct wlan_profile_params *param)
{
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;
	wmi_wlan_profile_trigger_cmd_fixed_param *prof_trig_cmd;

	len = sizeof(wmi_wlan_profile_trigger_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to send WMI_WLAN_PROFILE_TRIGGER_CMDID");
		return QDF_STATUS_E_NOMEM;
	}

	prof_trig_cmd =
		(wmi_wlan_profile_trigger_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(&prof_trig_cmd->tlv_header,
	     WMITLV_TAG_STRUC_wmi_wlan_profile_trigger_cmd_fixed_param,
	     WMITLV_GET_STRUCT_TLVLEN
			(wmi_wlan_profile_trigger_cmd_fixed_param));

	prof_trig_cmd->enable = param->enable;
	wmi_mtrace(WMI_WLAN_PROFILE_TRIGGER_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WLAN_PROFILE_TRIGGER_CMDID);
	if (ret) {
		wmi_err("Failed to send WMI_WLAN_PROFILE_TRIGGER_CMDID");
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_wlan_profile_hist_intvl_cmd_tlv() - send wlan profile interval command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wlan profile param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wlan_profile_hist_intvl_cmd_tlv(wmi_unified_t wmi_handle,
				     struct wlan_profile_params *param)
{
	wmi_buf_t buf;
	int32_t len = 0;
	QDF_STATUS ret;
	wmi_wlan_profile_set_hist_intvl_cmd_fixed_param *hist_intvl_cmd;

	len = sizeof(wmi_wlan_profile_set_hist_intvl_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to send WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID");
		return QDF_STATUS_E_NOMEM;
	}

	hist_intvl_cmd =
		(wmi_wlan_profile_set_hist_intvl_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(&hist_intvl_cmd->tlv_header,
	      WMITLV_TAG_STRUC_wmi_wlan_profile_set_hist_intvl_cmd_fixed_param,
	      WMITLV_GET_STRUCT_TLVLEN
	      (wmi_wlan_profile_set_hist_intvl_cmd_fixed_param));

	hist_intvl_cmd->profile_id = param->profile_id;
	hist_intvl_cmd->value = param->enable;
	wmi_mtrace(WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
		   NO_SESSION, 0);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID);
	if (ret) {
		wmi_err("Failed to send PROFILE_SET_HIST_INTVL_CMDID");
		wmi_buf_free(buf);
	}
	return ret;
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
		wmi_err("Failed to send fw test command");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static uint16_t wfa_config_param_len(enum wfa_test_cmds config)
{
	uint16_t len = 0;

	if (config == WFA_CONFIG_RXNE)
		len += WMI_TLV_HDR_SIZE + sizeof(wmi_wfa_config_rsnxe);
	else
		len += WMI_TLV_HDR_SIZE;

	if (config == WFA_CONFIG_CSA)
		len += WMI_TLV_HDR_SIZE + sizeof(wmi_wfa_config_csa);
	else
		len += WMI_TLV_HDR_SIZE;

	if (config == WFA_CONFIG_OCV)
		len += WMI_TLV_HDR_SIZE + sizeof(wmi_wfa_config_ocv);
	else
		len += WMI_TLV_HDR_SIZE;

	if (config == WFA_CONFIG_SA_QUERY)
		len += WMI_TLV_HDR_SIZE + sizeof(wmi_wfa_config_saquery);
	else
		len += WMI_TLV_HDR_SIZE;

	return len;
}

/**
 * wmi_fill_ocv_frame_type() - Fill host ocv frm type into WMI ocv frm type.
 * @host_frmtype: Host defined OCV frame type
 * @ocv_frmtype: Pointer to hold WMI OCV frame type
 *
 * This function converts and fills host defined OCV frame type into WMI OCV
 * frame type.
 *
 * Return: CDF STATUS
 */
static QDF_STATUS
wmi_fill_ocv_frame_type(uint32_t host_frmtype, uint32_t *ocv_frmtype)
{
	switch (host_frmtype) {
	case WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_SAQUERY_REQ:
		*ocv_frmtype = WMI_WFA_CONFIG_OCV_FRMTYPE_SAQUERY_REQ;
		break;

	case WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_SAQUERY_RSP:
		*ocv_frmtype = WMI_WFA_CONFIG_OCV_FRMTYPE_SAQUERY_RSP;
		break;

	case WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_FT_REASSOC_REQ:
		*ocv_frmtype = WMI_WFA_CONFIG_OCV_FRMTYPE_FT_REASSOC_REQ;
		break;

	case WMI_HOST_WFA_CONFIG_OCV_FRMTYPE_FILS_REASSOC_REQ:
		*ocv_frmtype = WMI_WFA_CONFIG_OCV_FRMTYPE_FILS_REASSOC_REQ;
		break;

	default:
		wmi_err("Invalid command type cmd %d", host_frmtype);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_wfa_test_cmd_tlv() - send wfa test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_wfatest: wfa test command
 *
 * This function sends wfa test command to fw.
 *
 * Return: CDF STATUS
 */
static
QDF_STATUS send_wfa_test_cmd_tlv(wmi_unified_t wmi_handle,
				 struct set_wfatest_params *wmi_wfatest)
{
	wmi_wfa_config_cmd_fixed_param *cmd;
	wmi_wfa_config_rsnxe *rxne;
	wmi_wfa_config_csa *csa;
	wmi_wfa_config_ocv *ocv;
	wmi_wfa_config_saquery *saquery;
	wmi_buf_t wmi_buf;
	uint16_t len = sizeof(*cmd);
	uint8_t *buf_ptr;

	len += wfa_config_param_len(wmi_wfatest->cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_wfa_config_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_wfa_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
					 wmi_wfa_config_cmd_fixed_param));

	cmd->vdev_id = wmi_wfatest->vdev_id;
	buf_ptr = (uint8_t *)(cmd + 1);

	if (wmi_wfatest->cmd == WFA_CONFIG_RXNE) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_wfa_config_rsnxe));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_wfa_config_rsnxe,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wfa_config_rsnxe));
		rxne = (wmi_wfa_config_rsnxe *)buf_ptr;
		rxne->rsnxe_param = wmi_wfatest->value;
		buf_ptr += sizeof(wmi_wfa_config_rsnxe);
	} else {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;
	}

	if (wmi_wfatest->cmd == WFA_CONFIG_CSA) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_wfa_config_csa));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_wfa_config_csa,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wfa_config_csa));
		csa = (wmi_wfa_config_csa *)buf_ptr;
		csa->ignore_csa = wmi_wfatest->value;
		buf_ptr += sizeof(wmi_wfa_config_csa);
	} else {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;
	}

	if (wmi_wfatest->cmd == WFA_CONFIG_OCV) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_wfa_config_ocv));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_wfa_config_ocv,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wfa_config_ocv));
		ocv = (wmi_wfa_config_ocv *)buf_ptr;

		if (wmi_fill_ocv_frame_type(wmi_wfatest->ocv_param->frame_type,
					    &ocv->frame_types))
			goto error;

		ocv->chan_freq = wmi_wfatest->ocv_param->freq;
		buf_ptr += sizeof(wmi_wfa_config_ocv);
	} else {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;
	}

	if (wmi_wfatest->cmd == WFA_CONFIG_SA_QUERY) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_wfa_config_saquery));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_wfa_config_saquery,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wfa_config_saquery));

		saquery = (wmi_wfa_config_saquery *)buf_ptr;
		saquery->remain_connect_on_saquery_timeout = wmi_wfatest->value;
	} else {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;
	}

	wmi_mtrace(WMI_WFA_CONFIG_CMDID, wmi_wfatest->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_WFA_CONFIG_CMDID)) {
		wmi_err("Failed to send wfa test command");
		goto error;
	}

	return QDF_STATUS_SUCCESS;

error:
	wmi_buf_free(wmi_buf);
	return QDF_STATUS_E_FAILURE;
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
	wmi_debug("VDEV ID: %d MODULE ID: %d TOKEN: %d",
		 cmd->vdev_id, cmd->module_id, cmd->diag_token);
	wmi_debug("%d num of args = ", wmi_utest->num_args);
	for (i = 0; (i < wmi_utest->num_args && i < WMI_UNIT_TEST_MAX_NUM_ARGS); i++) {
		unit_test_cmd_args[i] = wmi_utest->args[i];
		wmi_debug("%d,", wmi_utest->args[i]);
	}
	wmi_mtrace(WMI_UNIT_TEST_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_UNIT_TEST_CMDID)) {
		wmi_err("Failed to send unit test command");
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
								wmi_handle,
								param->pdev_id);
	cmd->module_id = param->module_id;
	cmd->num_args = param->num_args;
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (param->num_args * sizeof(uint32_t)));
	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	wmi_debug("%d num of args = ", param->num_args);
	for (i = 0; (i < param->num_args && i < WMI_MAX_POWER_DBG_ARGS); i++) {
		cmd_args[i] = param->args[i];
		wmi_debug("%d,", param->args[i]);
	}

	wmi_mtrace(WMI_PDEV_WAL_POWER_DEBUG_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_PDEV_WAL_POWER_DEBUG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("wmi_unified_cmd_send WMI_PDEV_WAL_POWER_DEBUG_CMDID returned Error %d",
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

	wmi_debug("pdev_id=%d", pdev_id);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								pdev_id);
	wmi_mtrace(WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send cmd to fw, ret=%d, pdev_id=%d",
			ret, pdev_id);
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

	wmi_debug("pdev_id=%d", pdev_id);

	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								pdev_id);
	wmi_mtrace(WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send cmd to fw, ret=%d, pdev_id=%d",
			ret, pdev_id);
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
		wmi_err("wmi_buf_alloc failed");
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
	cmd->center_freq1 = param->center_freq1;
	cmd->center_freq2 = param->center_freq2;
	cmd->chan_freq = param->chan_freq;
	cmd->chan_width = param->chan_width;
	cmd->min_duration_ms = param->min_duration_ms;
	cmd->max_duration_ms = param->max_duration_ms;
	wmi_debug("cmd->vdev_id: %d ,cmd->ocac_mode: %d cmd->center_freq: %d",
		 cmd->vdev_id, cmd->ocac_mode,
		 cmd->center_freq);

	wmi_mtrace(WMI_VDEV_ADFS_CH_CFG_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_ADFS_CH_CFG_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send cmd to fw, ret=%d", ret);
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
		wmi_err("wmi_buf_alloc failed");
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
		wmi_err("Failed to send cmd to fw, ret=%d", ret);
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
		if (is_service_enabled_tlv(wmi_handle,
					   WMI_SERVICE_SUPPORT_EXTEND_ADDRESS))
			host_mem_chunks[idx].ptr_high =
				qdf_get_upper_32_bits(
					param->mem_chunks[idx].paddr);
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

	wmi_debug("num peers: %d , num offload peers: %d, num vdevs: %d, num tids: %d, num tdls conn tb entries: %d, num tdls vdevs: %d",
		 resource_cfg->num_peers, resource_cfg->num_offload_peers,
		 resource_cfg->num_vdevs, resource_cfg->num_tids,
		 resource_cfg->num_tdls_conn_table_entries,
		 resource_cfg->num_tdls_vdevs);

	/* Fill hw mode id config */
	buf_ptr = copy_hw_mode_in_init_cmd(wmi_handle, buf_ptr, &len, param);

	/* Fill fw_abi_vers */
	copy_fw_abi_version_tlv(wmi_handle, cmd);

	wmi_mtrace(WMI_INIT_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_INIT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("wmi_unified_cmd_send WMI_INIT_CMDID returned Error %d",
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
		wmi_err("Failed to send cmd to fw, ret=%d", ret);
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
		wmi_err("Failed to send cmd to fw, ret=%d", ret);
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
		wmi_err("Failed to send cmd to fw, ret=%d", ret);
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
		wmi_err(
		 "WMI_PDEV_OBSS_PD_SPATIAL_REUSE_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_self_srg_bss_color_bitmap_set_cmd_tlv() - Send 64-bit BSS color bitmap
 * to be used by SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSS color bitmap
 * @bitmap_1: upper 32 bits in BSS color bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_self_srg_bss_color_bitmap_set_cmd_tlv(
	wmi_unified_t wmi_handle, uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	wmi_buf_t buf;
	wmi_pdev_srg_bss_color_bitmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_srg_bss_color_bitmap_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_srg_bss_color_bitmap_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_srg_bss_color_bitmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle, pdev_id);
	cmd->srg_bss_color_bitmap[0] = bitmap_0;
	cmd->srg_bss_color_bitmap[1] = bitmap_1;

	ret = wmi_unified_cmd_send(
			wmi_handle, buf, len,
			WMI_PDEV_SET_SRG_BSS_COLOR_BITMAP_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_SET_SRG_BSS_COLOR_BITMAP_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_self_srg_partial_bssid_bitmap_set_cmd_tlv() - Send 64-bit partial BSSID
 * bitmap to be used by SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in partial BSSID bitmap
 * @bitmap_1: upper 32 bits in partial BSSID bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_self_srg_partial_bssid_bitmap_set_cmd_tlv(
	wmi_unified_t wmi_handle, uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	wmi_buf_t buf;
	wmi_pdev_srg_partial_bssid_bitmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_srg_partial_bssid_bitmap_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_srg_partial_bssid_bitmap_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_srg_partial_bssid_bitmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle, pdev_id);

	cmd->srg_partial_bssid_bitmap[0] = bitmap_0;
	cmd->srg_partial_bssid_bitmap[1] = bitmap_1;

	ret = wmi_unified_cmd_send(
			wmi_handle, buf, len,
			WMI_PDEV_SET_SRG_PARTIAL_BSSID_BITMAP_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_SET_SRG_PARTIAL_BSSID_BITMAP_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_self_srg_obss_color_enable_bitmap_cmd_tlv() - Send 64-bit BSS color
 * enable bitmap to be used by SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSS color enable bitmap
 * @bitmap_1: upper 32 bits in BSS color enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_self_srg_obss_color_enable_bitmap_cmd_tlv(
	wmi_unified_t wmi_handle, uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	wmi_buf_t buf;
	wmi_pdev_srg_obss_color_enable_bitmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_srg_obss_color_enable_bitmap_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_srg_obss_color_enable_bitmap_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_srg_obss_color_enable_bitmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle, pdev_id);
	cmd->srg_obss_en_color_bitmap[0] = bitmap_0;
	cmd->srg_obss_en_color_bitmap[1] = bitmap_1;

	ret = wmi_unified_cmd_send(
			wmi_handle, buf, len,
			WMI_PDEV_SET_SRG_OBSS_COLOR_ENABLE_BITMAP_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_SET_SRG_OBSS_COLOR_ENABLE_BITMAP_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_self_srg_obss_bssid_enable_bitmap_cmd_tlv() - Send 64-bit OBSS BSSID
 * enable bitmap to be used by SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSSID enable bitmap
 * @bitmap_1: upper 32 bits in BSSID enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_self_srg_obss_bssid_enable_bitmap_cmd_tlv(
	wmi_unified_t wmi_handle, uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	wmi_buf_t buf;
	wmi_pdev_srg_obss_bssid_enable_bitmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_srg_obss_bssid_enable_bitmap_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_srg_obss_bssid_enable_bitmap_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_srg_obss_bssid_enable_bitmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle, pdev_id);
	cmd->srg_obss_en_bssid_bitmap[0] = bitmap_0;
	cmd->srg_obss_en_bssid_bitmap[1] = bitmap_1;

	ret = wmi_unified_cmd_send(
			wmi_handle, buf, len,
			WMI_PDEV_SET_SRG_OBSS_BSSID_ENABLE_BITMAP_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_SET_SRG_OBSS_BSSID_ENABLE_BITMAP_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_self_non_srg_obss_color_enable_bitmap_cmd_tlv() - Send 64-bit BSS color
 * enable bitmap to be used by Non-SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSS color enable bitmap
 * @bitmap_1: upper 32 bits in BSS color enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_self_non_srg_obss_color_enable_bitmap_cmd_tlv(
	wmi_unified_t wmi_handle, uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	wmi_buf_t buf;
	wmi_pdev_non_srg_obss_color_enable_bitmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_non_srg_obss_color_enable_bitmap_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_non_srg_obss_color_enable_bitmap_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_non_srg_obss_color_enable_bitmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle, pdev_id);
	cmd->non_srg_obss_en_color_bitmap[0] = bitmap_0;
	cmd->non_srg_obss_en_color_bitmap[1] = bitmap_1;

	ret = wmi_unified_cmd_send(
			wmi_handle, buf, len,
			WMI_PDEV_SET_NON_SRG_OBSS_COLOR_ENABLE_BITMAP_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_SET_NON_SRG_OBSS_COLOR_ENABLE_BITMAP_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_self_non_srg_obss_bssid_enable_bitmap_cmd_tlv() - Send 64-bit OBSS BSSID
 * enable bitmap to be used by Non-SRG based Spatial Reuse feature to the FW
 * @wmi_handle: wmi handle
 * @bitmap_0: lower 32 bits in BSSID enable bitmap
 * @bitmap_1: upper 32 bits in BSSID enable bitmap
 * @pdev_id: pdev ID
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_self_non_srg_obss_bssid_enable_bitmap_cmd_tlv(
	wmi_unified_t wmi_handle, uint32_t bitmap_0,
	uint32_t bitmap_1, uint8_t pdev_id)
{
	wmi_buf_t buf;
	wmi_pdev_non_srg_obss_bssid_enable_bitmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_non_srg_obss_bssid_enable_bitmap_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_non_srg_obss_bssid_enable_bitmap_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_non_srg_obss_bssid_enable_bitmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
					wmi_handle, pdev_id);
	cmd->non_srg_obss_en_bssid_bitmap[0] = bitmap_0;
	cmd->non_srg_obss_en_bssid_bitmap[1] = bitmap_1;

	ret = wmi_unified_cmd_send(
			wmi_handle, buf, len,
			WMI_PDEV_SET_NON_SRG_OBSS_BSSID_ENABLE_BITMAP_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_SET_NON_SRG_OBSS_BSSID_ENABLE_BITMAP_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}
#endif

static
QDF_STATUS send_injector_config_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_host_injector_frame_params *inject_config_params)
{
	wmi_buf_t buf;
	wmi_frame_inject_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_frame_inject_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_frame_inject_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_frame_inject_cmd_fixed_param));

	cmd->vdev_id = inject_config_params->vdev_id;
	cmd->enable = inject_config_params->enable;
	cmd->frame_type = inject_config_params->frame_type;
	cmd->frame_inject_period = inject_config_params->frame_inject_period;
	cmd->fc_duration = inject_config_params->frame_duration;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(inject_config_params->dstmac,
			&cmd->frame_addr1);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_FRAME_INJECT_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err(
		 "WMI_PDEV_FRAME_INJECT_CMDID send returned Error %d",
		 ret);
		wmi_buf_free(buf);
	}

	return ret;
}
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
	if (!congestion_stats)
		return QDF_STATUS_E_INVAL;

	out_buff->vdev_id = congestion_stats->vdev_id;
	out_buff->congestion = congestion_stats->congestion;

	wmi_debug("cca stats event processed");
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
		wmi_err("Invalid ctl_failsafe event buffer");
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
	uint32_t i = 0;

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

	wmi_debug("wmi_ext_service_bitmap 0:0x%x, 1:0x%x, 2:0x%x, 3:0x%x",
		 soc->wmi_ext_service_bitmap[0], soc->wmi_ext_service_bitmap[1],
		 soc->wmi_ext_service_bitmap[2], soc->wmi_ext_service_bitmap[3]);

	if (bitmap_buf)
		qdf_mem_copy(bitmap_buf,
			soc->wmi_ext_service_bitmap,
			(WMI_SERVICE_SEGMENT_BM_SIZE32 * sizeof(uint32_t)));

	if (!param_buf->wmi_service_ext_bitmap) {
		wmi_debug("wmi_service_ext_bitmap not available");
		return QDF_STATUS_SUCCESS;
	}

	if (!soc->wmi_ext2_service_bitmap ||
	    (param_buf->num_wmi_service_ext_bitmap >
	     soc->wmi_ext2_service_bitmap_len)) {
		if (soc->wmi_ext2_service_bitmap) {
			qdf_mem_free(soc->wmi_ext2_service_bitmap);
			soc->wmi_ext2_service_bitmap = NULL;
		}
		soc->wmi_ext2_service_bitmap =
			qdf_mem_malloc(param_buf->num_wmi_service_ext_bitmap *
				       sizeof(uint32_t));
		if (!soc->wmi_ext2_service_bitmap)
			return QDF_STATUS_E_NOMEM;

		soc->wmi_ext2_service_bitmap_len =
			param_buf->num_wmi_service_ext_bitmap;
	}

	qdf_mem_copy(soc->wmi_ext2_service_bitmap,
		     param_buf->wmi_service_ext_bitmap,
		     (param_buf->num_wmi_service_ext_bitmap *
		      sizeof(uint32_t)));

	for (i = 0; i < param_buf->num_wmi_service_ext_bitmap; i++) {
		wmi_debug("wmi_ext2_service_bitmap %u:0x%x",
			 i, soc->wmi_ext2_service_bitmap[i]);
	}

	return QDF_STATUS_SUCCESS;
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
 *	 to host internal HOST_REGDMN_MODE values.
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

	wmi_debug("Target wireless mode: 0x%x", target_wireless_mode);

	if (target_wireless_mode & REGDMN_MODE_11A)
		wireless_modes |= HOST_REGDMN_MODE_11A;

	if (target_wireless_mode & REGDMN_MODE_TURBO)
		wireless_modes |= HOST_REGDMN_MODE_TURBO;

	if (target_wireless_mode & REGDMN_MODE_11B)
		wireless_modes |= HOST_REGDMN_MODE_11B;

	if (target_wireless_mode & REGDMN_MODE_PUREG)
		wireless_modes |= HOST_REGDMN_MODE_PUREG;

	if (target_wireless_mode & REGDMN_MODE_11G)
		wireless_modes |= HOST_REGDMN_MODE_11G;

	if (target_wireless_mode & REGDMN_MODE_108G)
		wireless_modes |= HOST_REGDMN_MODE_108G;

	if (target_wireless_mode & REGDMN_MODE_108A)
		wireless_modes |= HOST_REGDMN_MODE_108A;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT20_2G)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT20_2G;

	if (target_wireless_mode & REGDMN_MODE_XR)
		wireless_modes |= HOST_REGDMN_MODE_XR;

	if (target_wireless_mode & REGDMN_MODE_11A_HALF_RATE)
		wireless_modes |= HOST_REGDMN_MODE_11A_HALF_RATE;

	if (target_wireless_mode & REGDMN_MODE_11A_QUARTER_RATE)
		wireless_modes |= HOST_REGDMN_MODE_11A_QUARTER_RATE;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT20)
		wireless_modes |= HOST_REGDMN_MODE_11NG_HT20;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT20)
		wireless_modes |= HOST_REGDMN_MODE_11NA_HT20;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT40PLUS)
		wireless_modes |= HOST_REGDMN_MODE_11NG_HT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT40MINUS)
		wireless_modes |= HOST_REGDMN_MODE_11NG_HT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT40PLUS)
		wireless_modes |= HOST_REGDMN_MODE_11NA_HT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT40MINUS)
		wireless_modes |= HOST_REGDMN_MODE_11NA_HT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT20)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT20;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT40PLUS)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT40MINUS)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT80)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT80;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT160)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT160;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT80_80)
		wireless_modes |= HOST_REGDMN_MODE_11AC_VHT80_80;

	return wireless_modes;
}

/**
 * convert_11be_phybitmap_to_reg_flags() - Convert 11BE phybitmap to
 * to regulatory flags.
 * @target_phybitmap: target phybitmap.
 * @phybitmap: host internal REGULATORY_PHYMODE set based on target
 * phybitmap.
 *
 * Return: None
 */

#ifdef WLAN_FEATURE_11BE
static void convert_11be_phybitmap_to_reg_flags(uint32_t target_phybitmap,
						uint32_t *phybitmap)
{
	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11BE)
		*phybitmap |= REGULATORY_PHYMODE_NO11BE;
}
#else
static void convert_11be_phybitmap_to_reg_flags(uint32_t target_phybitmap,
						uint32_t *phybitmap)
{
}
#endif

/* convert_phybitmap_tlv() - Convert  WMI_REGULATORY_PHYBITMAP values sent by
 * target to host internal REGULATORY_PHYMODE values.
 *
 * @target_target_phybitmap: target phybitmap received in the message.
 *
 * Return: returns the host internal REGULATORY_PHYMODE.
 */
static uint32_t convert_phybitmap_tlv(uint32_t target_phybitmap)
{
	uint32_t phybitmap = 0;

	wmi_debug("Target phybitmap: 0x%x", target_phybitmap);

	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11A)
		phybitmap |= REGULATORY_PHYMODE_NO11A;

	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11B)
		phybitmap |= REGULATORY_PHYMODE_NO11B;

	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11G)
		phybitmap |= REGULATORY_PHYMODE_NO11G;

	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11N)
		phybitmap |= REGULATORY_CHAN_NO11N;

	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11AC)
		phybitmap |= REGULATORY_PHYMODE_NO11AC;

	if (target_phybitmap & WMI_REGULATORY_PHYMODE_NO11AX)
		phybitmap |= REGULATORY_PHYMODE_NO11AX;

	convert_11be_phybitmap_to_reg_flags(target_phybitmap, &phybitmap);

	return phybitmap;
}

/**
 * convert_11be_flags_to_modes_ext() - Convert 11BE wireless mode flag
 * advertised by the target to wireless mode ext flags.
 * @target_wireless_modes_ext: Target wireless mode
 * @wireless_modes_ext: Variable to hold all the target wireless mode caps.
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_11BE
static void convert_11be_flags_to_modes_ext(uint32_t target_wireless_modes_ext,
					    uint64_t *wireless_modes_ext)
{
	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEG_EHT20)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEG_EHT20;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEG_EHT40PLUS)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEG_EHT40PLUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEG_EHT40MINUS)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEG_EHT40MINUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEA_EHT20)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEA_EHT20;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEA_EHT40PLUS)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEA_EHT40PLUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEA_EHT40MINUS)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEA_EHT40MINUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEA_EHT80)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEA_EHT80;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEA_EHT160)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEA_EHT160;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11BEA_EHT320)
		*wireless_modes_ext |= HOST_REGDMN_MODE_11BEA_EHT320;
}
#else
static void convert_11be_flags_to_modes_ext(uint32_t target_wireless_modes_ext,
					    uint64_t *wireless_modes_ext)
{
}
#endif

static inline uint64_t convert_wireless_modes_ext_tlv(
		uint32_t target_wireless_modes_ext)
{
	uint64_t wireless_modes_ext = 0;

	wmi_debug("Target wireless mode: 0x%x", target_wireless_modes_ext);

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXG_HE20)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXG_HE20;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXG_HE40PLUS)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXG_HE40PLUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXG_HE40MINUS)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXG_HE40MINUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXA_HE20)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXA_HE20;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXA_HE40PLUS)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXA_HE40PLUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXA_HE40MINUS)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXA_HE40MINUS;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXA_HE80)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXA_HE80;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXA_HE160)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXA_HE160;

	if (target_wireless_modes_ext & REGDMN_MODE_U32_11AXA_HE80_80)
		wireless_modes_ext |= HOST_REGDMN_MODE_11AXA_HE80_80;

	convert_11be_flags_to_modes_ext(target_wireless_modes_ext,
					&wireless_modes_ext);

	return wireless_modes_ext;
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
		wmi_err("Invalid arguments");
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
 * extract_hal_reg_cap_ext2_tlv() - extract HAL registered capability ext
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param cap: pointer to hold HAL reg capabilities
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_hal_reg_cap_ext2_tlv(
		wmi_unified_t wmi_handle, void *evt_buf, uint8_t phy_idx,
		struct wlan_psoc_host_hal_reg_capabilities_ext2 *param)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	WMI_HAL_REG_CAPABILITIES_EXT2 *reg_caps;

	if (!evt_buf) {
		wmi_err("null evt_buf");
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)evt_buf;

	if (!param_buf->num_hal_reg_caps)
		return QDF_STATUS_SUCCESS;

	if (phy_idx >= param_buf->num_hal_reg_caps)
		return QDF_STATUS_E_INVAL;

	reg_caps = &param_buf->hal_reg_caps[phy_idx];

	param->phy_id = reg_caps->phy_id;
	param->wireless_modes_ext = convert_wireless_modes_ext_tlv(
			reg_caps->wireless_modes_ext);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_num_mem_reqs_tlv() - Extract number of memory entries requested
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 *
 * Return: Number of entries requested
 */
static uint32_t extract_num_mem_reqs_tlv(wmi_unified_t wmi_handle,
					 void *evt_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed", __func__);
		return 0;
	}

	if (ev->num_mem_reqs > param_buf->num_mem_reqs) {
		wmi_err("Invalid num_mem_reqs %d:%d",
			 ev->num_mem_reqs, param_buf->num_mem_reqs);
		return 0;
	}

	return ev->num_mem_reqs;
}

/**
 * extract_host_mem_req_tlv() - Extract host memory required from
 *				service ready event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @mem_reqs: pointer to host memory request structure
 * @num_active_peers: number of active peers for peer cache
 * @num_peers: number of peers
 * @fw_prio: FW priority
 * @idx: index for memory request
 *
 * Return: Host memory request parameters requested by target
 */
static QDF_STATUS extract_host_mem_req_tlv(wmi_unified_t wmi_handle,
					   void *evt_buf,
					   host_mem_req *mem_reqs,
					   uint32_t num_active_peers,
					   uint32_t num_peers,
					   enum wmi_fw_mem_prio fw_prio,
					   uint16_t idx)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *)evt_buf;

	mem_reqs->req_id = (uint32_t)param_buf->mem_reqs[idx].req_id;
	mem_reqs->unit_size = (uint32_t)param_buf->mem_reqs[idx].unit_size;
	mem_reqs->num_unit_info =
		(uint32_t)param_buf->mem_reqs[idx].num_unit_info;
	mem_reqs->num_units = (uint32_t)param_buf->mem_reqs[idx].num_units;
	mem_reqs->tgt_num_units = 0;

	if (((fw_prio == WMI_FW_MEM_HIGH_PRIORITY) &&
	     (mem_reqs->num_unit_info &
	      REQ_TO_HOST_FOR_CONT_MEMORY)) ||
	    ((fw_prio == WMI_FW_MEM_LOW_PRIORITY) &&
	     (!(mem_reqs->num_unit_info &
	      REQ_TO_HOST_FOR_CONT_MEMORY)))) {
		/* First allocate the memory that requires contiguous memory */
		mem_reqs->tgt_num_units = mem_reqs->num_units;
		if (mem_reqs->num_unit_info) {
			if (mem_reqs->num_unit_info &
					NUM_UNITS_IS_NUM_PEERS) {
				/*
				 * number of units allocated is equal to number
				 * of peers, 1 extra for self peer on target.
				 * this needs to be fixed, host and target can
				 * get out of sync
				 */
				mem_reqs->tgt_num_units = num_peers + 1;
			}
			if (mem_reqs->num_unit_info &
					NUM_UNITS_IS_NUM_ACTIVE_PEERS) {
				/*
				 * Requesting allocation of memory using
				 * num_active_peers in qcache. if qcache is
				 * disabled in host, then it should allocate
				 * memory for num_peers instead of
				 * num_active_peers.
				 */
				if (num_active_peers)
					mem_reqs->tgt_num_units =
						num_active_peers + 1;
				else
					mem_reqs->tgt_num_units =
						num_peers + 1;
			}
		}

		wmi_debug("idx %d req %d  num_units %d num_unit_info %d"
			 "unit size %d actual units %d",
			 idx, mem_reqs->req_id,
			 mem_reqs->num_units,
			 mem_reqs->num_unit_info,
			 mem_reqs->unit_size,
			 mem_reqs->tgt_num_units);
	}

	return QDF_STATUS_SUCCESS;
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
	ev_param->pktlog_defs_checksum = ev->pktlog_defs_checksum;
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


#ifdef MGMT_FRAME_RX_DECRYPT_ERROR
#define IS_WMI_RX_MGMT_FRAME_STATUS_INVALID(_status) false
#else
#define IS_WMI_RX_MGMT_FRAME_STATUS_INVALID(_status) \
			((_status) & WMI_RXERR_DECRYPT)
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
		wmi_err("Get NULL point message from FW");
		return QDF_STATUS_E_INVAL;
	}

	ev_hdr = param_tlvs->hdr;
	if (!hdr) {
		wmi_err("Rx event is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (IS_WMI_RX_MGMT_FRAME_STATUS_INVALID(ev_hdr->status)) {
		wmi_err("RX mgmt frame decrypt error, discard it");
		return QDF_STATUS_E_INVAL;
	}

	if (ev_hdr->buf_len > param_tlvs->num_bufp) {
		wmi_err("Rx mgmt frame length mismatch, discard it");
		return QDF_STATUS_E_INVAL;
	}

	hdr->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev_hdr->pdev_id);
	hdr->chan_freq = ev_hdr->chan_freq;
	hdr->channel = ev_hdr->channel;
	hdr->snr = ev_hdr->snr;
	hdr->rate = ev_hdr->rate;
	hdr->phy_mode = ev_hdr->phy_mode;
	hdr->buf_len = ev_hdr->buf_len;
	hdr->status = ev_hdr->status;
	hdr->flags = ev_hdr->flags;
	hdr->rssi = ev_hdr->rssi;
	hdr->tsf_delta = ev_hdr->tsf_delta;
	hdr->tsf_l32 = ev_hdr->rx_tsf_l32;
	for (i = 0; i < ATH_MAX_ANTENNA; i++)
		hdr->rssi_ctl[i] = ev_hdr->rssi_ctl[i];

	*bufp = param_tlvs->bufp;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_MGMT_RX_REO_SUPPORT
/**
 * extract_mgmt_rx_fw_consumed_tlv() - extract MGMT Rx FW consumed event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @params: Pointer to MGMT Rx REO parameters
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_mgmt_rx_fw_consumed_tlv(wmi_unified_t wmi_handle,
				void *evt_buf,
				struct mgmt_rx_reo_params *params)
{
	WMI_MGMT_RX_FW_CONSUMED_EVENTID_param_tlvs *param_tlvs;
	wmi_mgmt_rx_fw_consumed_hdr *ev_hdr;

	param_tlvs = evt_buf;
	if (!param_tlvs) {
		wmi_err("param_tlvs is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev_hdr = param_tlvs->hdr;
	if (!params) {
		wmi_err("Rx REO parameters is NULL");
		return QDF_STATUS_E_INVAL;
	}

	params->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev_hdr->pdev_id);
	params->valid = WMI_MGMT_RX_FW_CONSUMED_PARAM_MGMT_PKT_CTR_VALID_GET(
				ev_hdr->mgmt_pkt_ctr_info);
	params->global_timestamp = ev_hdr->global_timestamp;
	params->mgmt_pkt_ctr = WMI_MGMT_RX_FW_CONSUMED_PARAM_MGMT_PKT_CTR_GET(
				ev_hdr->mgmt_pkt_ctr_info);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mgmt_rx_reo_params_tlv() - extract MGMT Rx REO params from
 * MGMT_RX_EVENT_ID
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @params: Pointer to MGMT Rx REO parameters
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_mgmt_rx_reo_params_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct mgmt_rx_reo_params *reo_params)
{
	WMI_MGMT_RX_EVENTID_param_tlvs *param_tlvs;
	wmi_mgmt_rx_reo_params *reo_params_tlv;
	wmi_mgmt_rx_hdr *ev_hdr;

	param_tlvs = evt_buf;
	if (!param_tlvs) {
		wmi_err("param_tlvs is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ev_hdr = param_tlvs->hdr;
	if (!ev_hdr) {
		wmi_err("Rx event is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reo_params_tlv = param_tlvs->reo_params;
	if (!reo_params_tlv) {
		wmi_err("mgmt_rx_reo_params TLV is not sent by FW");
		return QDF_STATUS_E_INVAL;
	}

	if (!reo_params) {
		wmi_err("MGMT Rx REO params is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reo_params->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev_hdr->pdev_id);
	reo_params->valid = WMI_MGMT_RX_REO_PARAM_MGMT_PKT_CTR_VALID_GET(
					reo_params_tlv->mgmt_pkt_ctr_link_info);
	reo_params->global_timestamp = reo_params_tlv->global_timestamp;
	reo_params->mgmt_pkt_ctr = WMI_MGMT_RX_REO_PARAM_MGMT_PKT_CTR_GET(
					reo_params_tlv->mgmt_pkt_ctr_link_info);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_mgmt_rx_reo_filter_config_cmd_tlv() - Send MGMT Rx REO filter
 * configuration command
 * @wmi_handle: wmi handle
 * @pdev_id: pdev ID of the radio
 * @filter: Pointer to MGMT Rx REO filter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_mgmt_rx_reo_filter_config_cmd_tlv(
					wmi_unified_t wmi_handle,
					uint8_t pdev_id,
					struct mgmt_rx_reo_filter *filter)
{
	QDF_STATUS ret;
	wmi_buf_t buf;
	wmi_mgmt_rx_reo_filter_configuration_cmd_fixed_param *cmd;
	size_t len = sizeof(*cmd);

	if (!filter) {
		wmi_err("mgmt_rx_reo_filter is NULL");
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_mgmt_rx_reo_filter_configuration_cmd_fixed_param *)
			wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_mgmt_rx_reo_filter_configuration_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_mgmt_rx_reo_filter_configuration_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_host_pdev_id_to_target(
						wmi_handle,
						pdev_id);
	cmd->filter_low = filter->low;
	cmd->filter_high = filter->high;

	wmi_mtrace(WMI_MGMT_RX_REO_FILTER_CONFIGURATION_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(
				wmi_handle, buf, len,
				WMI_MGMT_RX_REO_FILTER_CONFIGURATION_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI command");
		wmi_buf_free(buf);
	}

	return ret;
}
#endif

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
		wmi_err("Invalid roam event buffer");
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

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * extract_nlo_match_ev_param_tlv() - extract NLO match param from event
 * @wmi_handle: pointer to WMI handle
 * @evt_buf: pointer to WMI event buffer
 * @param: pointer to scan event param for NLO match
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_nlo_match_ev_param_tlv(wmi_unified_t wmi_handle,
						 void *evt_buf,
						 struct scan_event *param)
{
	WMI_NLO_MATCH_EVENTID_param_tlvs *param_buf = evt_buf;
	wmi_nlo_event *evt = param_buf->fixed_param;

	qdf_mem_zero(param, sizeof(*param));

	param->type = SCAN_EVENT_TYPE_NLO_MATCH;
	param->vdev_id = evt->vdev_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_nlo_complete_ev_param_tlv() - extract NLO complete param from event
 * @wmi_handle: pointer to WMI handle
 * @evt_buf: pointer to WMI event buffer
 * @param: pointer to scan event param for NLO complete
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_nlo_complete_ev_param_tlv(wmi_unified_t wmi_handle,
						    void *evt_buf,
						    struct scan_event *param)
{
	WMI_NLO_SCAN_COMPLETE_EVENTID_param_tlvs *param_buf = evt_buf;
	wmi_nlo_event *evt = param_buf->fixed_param;

	qdf_mem_zero(param, sizeof(*param));

	param->type = SCAN_EVENT_TYPE_NLO_COMPLETE;
	param->vdev_id = evt->vdev_id;

	return QDF_STATUS_SUCCESS;
}
#endif

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
	wmi_debug("vdev_id:%d mod_id:%d diag_token:%d flag:%d",
			ev_param->vdev_id,
			ev_param->module_id,
			ev_param->diag_token,
			ev_param->flag);
	wmi_debug("Unit-test data given below %d", num_bufp);
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
			((ev_param->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
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
 * extract_vdev_prb_fils_stats_tlv() - extract vdev probe and fils
 * stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_prb_fd_stats: Pointer to hold vdev probe and fils stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_vdev_prb_fils_stats_tlv(wmi_unified_t wmi_handle,
				void *evt_buf, uint32_t index,
				struct wmi_host_vdev_prb_fils_stats *vdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_vdev_extd_stats *ev;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;

	if (param_buf->vdev_extd_stats) {
		ev = (wmi_vdev_extd_stats *)(param_buf->vdev_extd_stats +
					     index);
		vdev_stats->vdev_id = ev->vdev_id;
		vdev_stats->fd_succ_cnt = ev->fd_succ_cnt;
		vdev_stats->fd_fail_cnt = ev->fd_fail_cnt;
		vdev_stats->unsolicited_prb_succ_cnt =
			ev->unsolicited_prb_succ_cnt;
		vdev_stats->unsolicited_prb_fail_cnt =
			ev->unsolicited_prb_fail_cnt;
		wmi_debug("vdev: %d, fd_s: %d, fd_f: %d, prb_s: %d, prb_f: %d",
			 ev->vdev_id, ev->fd_succ_cnt, ev->fd_fail_cnt,
			 ev->unsolicited_prb_succ_cnt,
			 ev->unsolicited_prb_fail_cnt);
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
			((ev_param->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
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
	WMI_WLAN_PROFILE_DATA_EVENTID_param_tlvs *param_buf;

	wmi_wlan_profile_ctx_t *ev;

	param_buf = (WMI_WLAN_PROFILE_DATA_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid profile data event buf");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->profile_ctx;

	profile_ctx->tot = ev->tot;
	profile_ctx->tx_msdu_cnt = ev->tx_msdu_cnt;
	profile_ctx->tx_mpdu_cnt = ev->tx_mpdu_cnt;
	profile_ctx->tx_ppdu_cnt = ev->tx_ppdu_cnt;
	profile_ctx->rx_msdu_cnt = ev->rx_msdu_cnt;
	profile_ctx->rx_mpdu_cnt = ev->rx_mpdu_cnt;
	profile_ctx->bin_count   = ev->bin_count;

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
	WMI_WLAN_PROFILE_DATA_EVENTID_param_tlvs *param_buf;
	wmi_wlan_profile_t *ev;

	param_buf = (WMI_WLAN_PROFILE_DATA_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid profile data event buf");
		return QDF_STATUS_E_INVAL;
	}

	ev = &param_buf->profile_data[idx];
	profile_data->id  = ev->id;
	profile_data->cnt = ev->cnt;
	profile_data->tot = ev->tot;
	profile_data->min = ev->min;
	profile_data->max = ev->max;
	profile_data->hist_intvl = ev->hist_intvl;
	qdf_mem_copy(profile_data->hist, ev->hist, sizeof(profile_data->hist));

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
		wmi_err("Invalid datalen: %d", event->datalen);
		return QDF_STATUS_E_INVAL;
	}
	seg_hdr = (struct wmi_host_utf_seg_header_info *)param_buf->data;
	/* Set pdev_id=1 until FW adds support to include pdev_id */
	event->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							seg_hdr->pdev_id);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
static QDF_STATUS extract_num_rf_characterization_entries_tlv(wmi_unified_t wmi_handle,
	uint8_t *event,
	uint32_t *num_rf_characterization_entries)
{
	WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	*num_rf_characterization_entries =
			param_buf->num_wmi_chan_rf_characterization_info;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_rf_characterization_entries_tlv(wmi_unified_t wmi_handle,
	uint8_t *event,
	uint32_t num_rf_characterization_entries,
	struct wmi_host_rf_characterization_event_param *rf_characterization_entries)
{
	WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENTID_param_tlvs *param_buf;
	WMI_CHAN_RF_CHARACTERIZATION_INFO *wmi_rf_characterization_entry;
	uint8_t ix;

	param_buf = (WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	wmi_rf_characterization_entry =
			param_buf->wmi_chan_rf_characterization_info;
	if (!wmi_rf_characterization_entry)
		return QDF_STATUS_E_INVAL;

	/*
	 * Using num_wmi_chan_rf_characterization instead of param_buf value
	 * since memory for rf_characterization_entries was allocated using
	 * the former.
	 */
	for (ix = 0; ix < num_rf_characterization_entries; ix++) {
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

#ifdef WLAN_FEATURE_11BE
static void
extract_11be_chainmask(struct wlan_psoc_host_chainmask_capabilities *cap,
		       WMI_MAC_PHY_CHAINMASK_CAPABILITY *chainmask_caps)
{
	cap->supports_chan_width_320 =
		WMI_SUPPORT_CHAN_WIDTH_320_GET(chainmask_caps->supported_flags);
}
#else
static void
extract_11be_chainmask(struct wlan_psoc_host_chainmask_capabilities *cap,
		       WMI_MAC_PHY_CHAINMASK_CAPABILITY *chainmask_caps)
{
}
#endif /* WLAN_FEATURE_11BE */

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

			chainmask_table[i].cap_list[j].supports_aSpectral =
				WMI_SUPPORT_AGILE_SPECTRAL_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_aSpectral_160 =
				WMI_SUPPORT_AGILE_SPECTRAL_160_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_aDFS_160 =
				WMI_SUPPORT_ADFS_160_GET(chainmask_caps->supported_flags);

			extract_11be_chainmask(&chainmask_table[i].cap_list[j],
					       chainmask_caps);

			wmi_nofl_debug("supported_flags: 0x%08x  chainmasks: 0x%08x",
				       chainmask_caps->supported_flags,
				       chainmask_caps->chainmask);
			chainmask_caps++;
		}
	}

	return QDF_STATUS_SUCCESS;
}

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
 * extract_service_ready_ext2_tlv() - extract service ready ext2 params from
 * event
 * @wmi_handle: wmi handle
 * @event: pointer to event buffer
 * @param: Pointer to hold the params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_service_ready_ext2_tlv(wmi_unified_t wmi_handle, uint8_t *event,
			       struct wlan_psoc_host_service_ext2_param *param)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_ext2_event_fixed_param *ev;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_INVAL;

	param->reg_db_version_major =
			WMI_REG_DB_VERSION_MAJOR_GET(
				ev->reg_db_version);
	param->reg_db_version_minor =
			WMI_REG_DB_VERSION_MINOR_GET(
				ev->reg_db_version);
	param->bdf_reg_db_version_major =
			WMI_BDF_REG_DB_VERSION_MAJOR_GET(
				ev->reg_db_version);
	param->bdf_reg_db_version_minor =
			WMI_BDF_REG_DB_VERSION_MINOR_GET(
				ev->reg_db_version);
	param->chwidth_num_peer_caps = ev->chwidth_num_peer_caps;

	param->num_dbr_ring_caps = param_buf->num_dma_ring_caps;

	if (param_buf->nan_cap)
		param->max_ndp_sessions =
			param_buf->nan_cap->max_ndp_sessions;
	else
		param->max_ndp_sessions = 0;

	param->preamble_puncture_bw_cap = ev->preamble_puncture_bw;
	param->num_scan_radio_caps = param_buf->num_wmi_scan_radio_caps;
	param->max_users_dl_ofdma = WMI_MAX_USER_PER_PPDU_DL_OFDMA_GET(
						ev->max_user_per_ppdu_ofdma);
	param->max_users_ul_ofdma = WMI_MAX_USER_PER_PPDU_UL_OFDMA_GET(
						ev->max_user_per_ppdu_ofdma);
	param->max_users_dl_mumimo = WMI_MAX_USER_PER_PPDU_DL_MUMIMO_GET(
						ev->max_user_per_ppdu_mumimo);
	param->max_users_ul_mumimo = WMI_MAX_USER_PER_PPDU_UL_MUMIMO_GET(
						ev->max_user_per_ppdu_mumimo);
	param->target_cap_flags = ev->target_cap_flags;
	wmi_debug("htt peer data :%d", ev->target_cap_flags);
	return QDF_STATUS_SUCCESS;
}

/*
 * extract_dbs_or_sbs_cap_service_ready_ext2_tlv() - extract dbs_or_sbs cap from
 *                                                   service ready ext 2
 *
 * @wmi_handle: wmi handle
 * @event: pointer to event buffer
 * @sbs_lower_band_end_freq: If sbs_lower_band_end_freq is set to non-zero,
 *                           it indicates async SBS mode is supported, and
 *                           lower-band/higher band to MAC mapping is
 *                           switch-able. unit: mhz. examples 5180, 5320
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS extract_dbs_or_sbs_cap_service_ready_ext2_tlv(
				wmi_unified_t wmi_handle, uint8_t *event,
				uint32_t *sbs_lower_band_end_freq)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	wmi_dbs_or_sbs_cap_ext *dbs_or_sbs_caps;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	dbs_or_sbs_caps = param_buf->dbs_or_sbs_cap_ext;
	if (!dbs_or_sbs_caps)
		return QDF_STATUS_E_INVAL;

	*sbs_lower_band_end_freq = dbs_or_sbs_caps->sbs_lower_band_end_freq;

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

#ifdef FEATURE_SAR_LIMITS
/**
 * wlan_convert_sar_flag() - Convert FW enum to Host enum
 * @wmi_sar_flag: sar flag enum
 *
 * Return: host equivalent sar flag
 */
static enum sar_flag wlan_convert_sar_flag(uint32_t wmi_sar_flag)
{
	switch (wmi_sar_flag) {
	case WMI_SAR_SET_CTL_GROUPING_DISABLE:
		return SAR_SET_CTL_GROUPING_DISABLE;
	case WMI_SAR_DBS_WITH_BT_DISABLE:
		return SAR_DBS_WITH_BT_DISABLE;
	default:
		return SAR_FLAG_NONE;
	}
}
#else
static inline
uint32_t wlan_convert_sar_flag(uint32_t wmi_sar_flag)
{
	return 0;
}
#endif

/**
 * extract_sar_cap_service_ready_ext2_tlv() -
 *       extract SAR cap - flag from service ready event
 * @wmi_handle: wmi handle
 * @event: pointer to event buffer
 * @ext2_param: extended target info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_sar_cap_service_ready_ext2_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event,
			struct wlan_psoc_host_service_ext2_param *ext2_param)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	wmi_sar_flag_tlv_param *sar_flag_tlv;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;

	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	if (!param_buf->num_sar_flags)
		return QDF_STATUS_E_INVAL;

	sar_flag_tlv = param_buf->sar_flags;
	if (sar_flag_tlv)
		ext2_param->sar_flag =
			wlan_convert_sar_flag(sar_flag_tlv->sar_flags);
	else
		ext2_param->sar_flag = 0;

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
 * extract_mac_phy_cap_service_ready_11be_support - api to extract 11be support
 * @param param: host mac phy capabilities
 * @param mac_phy_caps: mac phy capabilities
 *
 * Return: void
 */
#ifdef WLAN_FEATURE_11BE
static void
extract_service_ready_11be_support(struct wlan_psoc_host_mac_phy_caps *param,
				   WMI_MAC_PHY_CAPABILITIES *mac_phy_caps)
{
	param->supports_11be =
			WMI_SUPPORT_11BE_GET(mac_phy_caps->supported_flags);

	wmi_debug("11be support %d", param->supports_11be);
}
#else
static void
extract_service_ready_11be_support(struct wlan_psoc_host_mac_phy_caps *param,
				   WMI_MAC_PHY_CAPABILITIES *mac_phy_caps)
{
}
#endif

#if defined(WLAN_FEATURE_11BE_MLO) && defined(WLAN_MLO_MULTI_CHIP)
static void extract_hw_link_id(struct wlan_psoc_host_mac_phy_caps *param,
			       WMI_MAC_PHY_CAPABILITIES *mac_phy_caps)
{
	param->hw_link_id = WMI_PHY_GET_HW_LINK_ID(mac_phy_caps->pdev_id);
}
#else
static void extract_hw_link_id(struct wlan_psoc_host_mac_phy_caps *param,
			       WMI_MAC_PHY_CAPABILITIES *mac_phy_caps)
{
}
#endif /*WLAN_FEATURE_11BE_MLO && WLAN_MLO_MULTI_CHIP*/

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
	uint8_t hw_idx, phy_idx = 0, pdev_id;

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
	param->phy_idx = phy_idx;
	pdev_id = WMI_PHY_GET_PDEV_ID(mac_phy_caps->pdev_id);
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							pdev_id);
	param->tgt_pdev_id = pdev_id;
	extract_hw_link_id(param, mac_phy_caps);
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

	extract_service_ready_11be_support(param, mac_phy_caps);

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
	param->reg_cap_ext.wireless_modes = convert_wireless_modes_tlv
						(mac_phy_caps->wireless_modes);
	param->reg_cap_ext.low_2ghz_chan  = mac_phy_caps->low_2ghz_chan_freq;
	param->reg_cap_ext.high_2ghz_chan = mac_phy_caps->high_2ghz_chan_freq;
	param->reg_cap_ext.low_5ghz_chan  = mac_phy_caps->low_5ghz_chan_freq;
	param->reg_cap_ext.high_5ghz_chan = mac_phy_caps->high_5ghz_chan_freq;
	param->nss_ratio_enabled = WMI_NSS_RATIO_ENABLE_DISABLE_GET(
			mac_phy_caps->nss_ratio);
	param->nss_ratio_info = WMI_NSS_RATIO_INFO_GET(mac_phy_caps->nss_ratio);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mac_phy_cap_ehtcaps- api to extract eht mac phy caps
 * @param param: host ext2 mac phy capabilities
 * @param mac_phy_caps: ext mac phy capabilities
 *
 * Return: void
 */
#ifdef WLAN_FEATURE_11BE
static void extract_mac_phy_cap_ehtcaps(
	struct wlan_psoc_host_mac_phy_caps_ext2 *param,
	WMI_MAC_PHY_CAPABILITIES_EXT *mac_phy_caps)
{
	uint32_t i;

	param->eht_supp_mcs_2G = mac_phy_caps->eht_supp_mcs_2G;
	param->eht_supp_mcs_5G = mac_phy_caps->eht_supp_mcs_5G;
	param->eht_cap_info_internal = mac_phy_caps->eht_cap_info_internal;

	qdf_mem_copy(&param->eht_cap_info_2G,
		     &mac_phy_caps->eht_cap_mac_info_2G,
		     sizeof(param->eht_cap_info_2G));
	qdf_mem_copy(&param->eht_cap_info_5G,
		     &mac_phy_caps->eht_cap_mac_info_5G,
		     sizeof(param->eht_cap_info_5G));

	qdf_mem_copy(&param->eht_cap_phy_info_2G,
		     &mac_phy_caps->eht_cap_phy_info_2G,
		     sizeof(param->eht_cap_phy_info_2G));
	qdf_mem_copy(&param->eht_cap_phy_info_5G,
		     &mac_phy_caps->eht_cap_phy_info_5G,
		     sizeof(param->eht_cap_phy_info_5G));

	qdf_mem_copy(&param->eht_supp_mcs_ext_2G,
		     &mac_phy_caps->eht_supp_mcs_ext_2G,
		     sizeof(param->eht_supp_mcs_ext_2G));
	qdf_mem_copy(&param->eht_supp_mcs_ext_5G,
		     &mac_phy_caps->eht_supp_mcs_ext_5G,
		     sizeof(param->eht_supp_mcs_ext_5G));

	wmi_debug("EHT mac caps: mac cap_info_2G %x, mac cap_info_5G %x, supp_mcs_2G %x, supp_mcs_5G %x, info_internal %x",
		  mac_phy_caps->eht_cap_mac_info_2G[0],
		  mac_phy_caps->eht_cap_mac_info_5G[0],
		  mac_phy_caps->eht_supp_mcs_2G, mac_phy_caps->eht_supp_mcs_5G,
		  mac_phy_caps->eht_cap_info_internal);

	wmi_nofl_debug("EHT phy caps: ");

	wmi_nofl_debug("2G:");
	for (i = 0; i < PSOC_HOST_MAX_EHT_PHY_SIZE; i++) {
		wmi_nofl_debug("index %d value %d",
			       i, param->eht_cap_phy_info_2G[i]);
	}
	wmi_nofl_debug("5G:");
	for (i = 0; i < PSOC_HOST_MAX_EHT_PHY_SIZE; i++) {
		wmi_nofl_debug("index %d value %d",
			       i, param->eht_cap_phy_info_5G[i]);
	}
	wmi_nofl_debug("2G MCS ext Map:");
	for (i = 0; i < PSOC_HOST_EHT_MCS_NSS_MAP_2G_SIZE; i++) {
		wmi_nofl_debug("index %d value %d",
			       i, param->eht_supp_mcs_ext_2G[i]);
	}
	wmi_nofl_debug("5G MCS ext Map:");
	for (i = 0; i < PSOC_HOST_EHT_MCS_NSS_MAP_5G_SIZE; i++) {
		wmi_nofl_debug("index %d value %d",
			       i, param->eht_supp_mcs_ext_5G[i]);
	}
}
#else
static void extract_mac_phy_cap_ehtcaps(
	struct wlan_psoc_host_mac_phy_caps_ext2 *param,
	WMI_MAC_PHY_CAPABILITIES_EXT *mac_phy_caps)
{
}
#endif
static QDF_STATUS extract_mac_phy_cap_service_ready_ext2_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t hw_mode_id, uint8_t phy_id,
			uint8_t phy_idx,
			struct wlan_psoc_host_mac_phy_caps_ext2 *param)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	WMI_MAC_PHY_CAPABILITIES_EXT *mac_phy_caps;

	if (!event) {
		wmi_err("null evt_buf");
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;

	if (!param_buf->num_mac_phy_caps)
		return QDF_STATUS_SUCCESS;

	if (phy_idx >= param_buf->num_mac_phy_caps)
		return QDF_STATUS_E_INVAL;

	mac_phy_caps = &param_buf->mac_phy_caps[phy_idx];

	if ((hw_mode_id != mac_phy_caps->hw_mode_id) ||
	    (phy_id != mac_phy_caps->phy_id))
		return QDF_STATUS_E_INVAL;

	param->hw_mode_id = mac_phy_caps->hw_mode_id;
	param->phy_id = mac_phy_caps->phy_id;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
			wmi_handle, mac_phy_caps->pdev_id);
	param->wireless_modes_ext = convert_wireless_modes_ext_tlv(
			mac_phy_caps->wireless_modes_ext);

	extract_mac_phy_cap_ehtcaps(param, mac_phy_caps);

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

static QDF_STATUS validate_dbr_ring_caps_idx(uint8_t idx,
					     uint8_t num_dma_ring_caps)
{
	/* If dma_ring_caps is populated, num_dbr_ring_caps is non-zero */
	if (!num_dma_ring_caps) {
		wmi_err("dma_ring_caps %d", num_dma_ring_caps);
		return QDF_STATUS_E_INVAL;
	}
	if (idx >= num_dma_ring_caps) {
		wmi_err("Index %d exceeds range", idx);
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

static void
populate_dbr_ring_cap_elems(wmi_unified_t wmi_handle,
			    struct wlan_psoc_host_dbr_ring_caps *param,
			    WMI_DMA_RING_CAPABILITIES *dbr_ring_caps)
{
	param->pdev_id = wmi_handle->ops->convert_target_pdev_id_to_host(
				wmi_handle,
				dbr_ring_caps->pdev_id);
	param->mod_id = dbr_ring_caps->mod_id;
	param->ring_elems_min = dbr_ring_caps->ring_elems_min;
	param->min_buf_size = dbr_ring_caps->min_buf_size;
	param->min_buf_align = dbr_ring_caps->min_buf_align;
}

static QDF_STATUS extract_dbr_ring_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	QDF_STATUS status;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	status = validate_dbr_ring_caps_idx(idx, param_buf->num_dma_ring_caps);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	populate_dbr_ring_cap_elems(wmi_handle, param,
				    &param_buf->dma_ring_caps[idx]);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_dbr_ring_cap_service_ready_ext2_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t idx,
			struct wlan_psoc_host_dbr_ring_caps *param)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	QDF_STATUS status;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	status = validate_dbr_ring_caps_idx(idx, param_buf->num_dma_ring_caps);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	populate_dbr_ring_cap_elems(wmi_handle, param,
				    &param_buf->dma_ring_caps[idx]);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_scan_radio_cap_service_ready_ext2_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t idx,
			struct wlan_psoc_host_scan_radio_caps *param)
{
	WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *param_buf;
	WMI_SCAN_RADIO_CAPABILITIES_EXT2 *scan_radio_caps;

	param_buf = (WMI_SERVICE_READY_EXT2_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	if (idx >= param_buf->num_wmi_scan_radio_caps)
		return QDF_STATUS_E_INVAL;

	scan_radio_caps = &param_buf->wmi_scan_radio_caps[idx];
	param->phy_id = scan_radio_caps->phy_id;
	param->scan_radio_supported =
		WMI_SCAN_RADIO_CAP_SCAN_RADIO_FLAG_GET(scan_radio_caps->flags);
	param->dfs_en =
		WMI_SCAN_RADIO_CAP_DFS_FLAG_GET(scan_radio_caps->flags);

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_tgt_thermal_level_to_host() - Convert target thermal level to host enum
 * @level: target thermal level from WMI_THERM_THROT_STATS_EVENTID event
 *
 * Return: host thermal throt level
 */
static enum thermal_throttle_level
wmi_tgt_thermal_level_to_host(uint32_t level)
{
	switch (level) {
	case WMI_THERMAL_FULLPERF:
		return THERMAL_FULLPERF;
	case WMI_THERMAL_MITIGATION:
		return THERMAL_MITIGATION;
	case WMI_THERMAL_SHUTOFF:
		return THERMAL_SHUTOFF;
	case WMI_THERMAL_SHUTDOWN_TGT:
		return THERMAL_SHUTDOWN_TARGET;
	default:
		return THERMAL_UNKNOWN;
	}
}

#ifdef THERMAL_STATS_SUPPORT
static void
populate_thermal_stats(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf,
		       uint32_t *therm_throt_levels,
		       struct thermal_throt_level_stats *tt_temp_range_stats)
{
	uint8_t lvl_idx;
	wmi_therm_throt_stats_event_fixed_param *tt_stats_event;
	wmi_thermal_throt_temp_range_stats *wmi_tt_stats;

	tt_stats_event = param_buf->fixed_param;
	*therm_throt_levels = (tt_stats_event->therm_throt_levels >
			       WMI_THERMAL_STATS_TEMP_THRESH_LEVEL_MAX) ?
			       WMI_THERMAL_STATS_TEMP_THRESH_LEVEL_MAX :
			       tt_stats_event->therm_throt_levels;

	if (*therm_throt_levels > param_buf->num_temp_range_stats) {
		wmi_err("therm_throt_levels:%u oob num_temp_range_stats:%u",
			*therm_throt_levels,
			param_buf->num_temp_range_stats);
		return;
	}

	wmi_tt_stats = param_buf->temp_range_stats;
	if (!wmi_tt_stats) {
		wmi_err("wmi_tt_stats Null");
		return;
	}

	for (lvl_idx = 0; lvl_idx < *therm_throt_levels; lvl_idx++) {
		tt_temp_range_stats[lvl_idx].start_temp_level =
					wmi_tt_stats[lvl_idx].start_temp_level;
		tt_temp_range_stats[lvl_idx].end_temp_level =
					wmi_tt_stats[lvl_idx].end_temp_level;
		tt_temp_range_stats[lvl_idx].total_time_ms_lo =
					wmi_tt_stats[lvl_idx].total_time_ms_lo;
		tt_temp_range_stats[lvl_idx].total_time_ms_hi =
					wmi_tt_stats[lvl_idx].total_time_ms_hi;
		tt_temp_range_stats[lvl_idx].num_entry =
					wmi_tt_stats[lvl_idx].num_entry;
		wmi_debug("level %d, start temp %d, end temp %d, total time low %d, total time high %d, counter %d",
			  lvl_idx, wmi_tt_stats[lvl_idx].start_temp_level,
			  wmi_tt_stats[lvl_idx].end_temp_level,
			  wmi_tt_stats[lvl_idx].total_time_ms_lo,
			  wmi_tt_stats[lvl_idx].total_time_ms_hi,
			  wmi_tt_stats[lvl_idx].num_entry);
	}
}
#else
static void
populate_thermal_stats(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf,
		       uint32_t *therm_throt_levels,
		       struct thermal_throt_level_stats *tt_temp_range_stats)
{
}
#endif

/**
 * extract_thermal_stats_tlv() - extract thermal stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param temp: Pointer to hold extracted temperature
 * @param level: Pointer to hold extracted level in host enum
 * @param therm_throt_levels: Pointer to hold extracted thermal throttle temp
 *        range
 * @param tt_temp_range_stats_event: Pointer to hold extracted thermal stats for
 *                                   every level
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_thermal_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t *temp,
		enum thermal_throttle_level *level,
		uint32_t *therm_throt_levels,
		struct thermal_throt_level_stats *tt_temp_range_stats_event,
		uint32_t *pdev_id)
{
	WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf;
	wmi_therm_throt_stats_event_fixed_param *tt_stats_event;

	param_buf =
		(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	tt_stats_event = param_buf->fixed_param;
	wmi_debug("thermal temperature %d level %d",
		  tt_stats_event->temp, tt_stats_event->level);
	*pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
						wmi_handle,
						tt_stats_event->pdev_id);
	*temp = tt_stats_event->temp;
	*level = wmi_tgt_thermal_level_to_host(tt_stats_event->level);

	if (tt_stats_event->therm_throt_levels)
		populate_thermal_stats(param_buf, therm_throt_levels,
				       tt_temp_range_stats_event);

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
* send_pdev_get_pn_cmd_tlv() - send get PN request params to fw
* @wmi_handle - wmi handle
* @params - PN request params for peer
*
* Return: QDF_STATUS - success or error status
*/
static QDF_STATUS
send_pdev_get_pn_cmd_tlv(wmi_unified_t wmi_handle,
			 struct peer_request_pn_param *params)
{
	wmi_peer_tx_pn_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len = sizeof(wmi_peer_tx_pn_request_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_tx_pn_request_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_tx_pn_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_peer_tx_pn_request_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->peer_macaddr, &cmd->peer_macaddr);
	cmd->key_type = params->key_type;
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PEER_TX_PN_REQUEST_CMDID)) {
		wmi_err("Failed to send WMI command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
* extract_get_pn_data_tlv() - extract pn resp
* @wmi_handle - wmi handle
* @params - PN response params for peer
*
* Return: QDF_STATUS - success or error status
*/
static QDF_STATUS
extract_get_pn_data_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			struct wmi_host_get_pn_event *param)
{
	WMI_PEER_TX_PN_RESPONSE_EVENTID_param_tlvs *param_buf;
	wmi_peer_tx_pn_response_event_fixed_param *event = NULL;

	param_buf = (WMI_PEER_TX_PN_RESPONSE_EVENTID_param_tlvs *)evt_buf;
	event =
	(wmi_peer_tx_pn_response_event_fixed_param *)param_buf->fixed_param;

	param->vdev_id = event->vdev_id;
	param->key_type = event->key_type;
	qdf_mem_copy(param->pn, event->pn, sizeof(event->pn));
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->peer_macaddr, param->mac_addr);

	return QDF_STATUS_SUCCESS;
}

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

	if (event->data_len > param_buf->num_data)
		return QDF_STATUS_E_FAILURE;

	if (fips_conv_data_be(event->data_len, param_buf->data) !=
							QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	param->data = (uint32_t *)param_buf->data;
	param->data_len = event->data_len;
	param->error_status = event->error_status;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								wmi_handle,
								event->pdev_id);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
/**
 * extract_fips_extend_event_data_tlv() - extract fips event data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: pointer FIPS event params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_fips_extend_event_data_tlv(wmi_unified_t wmi_handle,
				   void *evt_buf,
				   struct wmi_host_fips_extend_event_param
				    *param)
{
	WMI_PDEV_FIPS_EXTEND_EVENTID_param_tlvs *param_buf;
	wmi_pdev_fips_extend_event_fixed_param *event;

	param_buf = (WMI_PDEV_FIPS_EXTEND_EVENTID_param_tlvs *)evt_buf;
	event = (wmi_pdev_fips_extend_event_fixed_param *)param_buf->fixed_param;

	if (fips_conv_data_be(event->data_len, param_buf->data) !=
							QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	param->data = (uint32_t *)param_buf->data;
	param->data_len = event->data_len;
	param->error_status = event->error_status;
	param->fips_cookie = event->fips_cookie;
	param->cmd_frag_idx = event->cmd_frag_idx;
	param->more_bit = event->more_bit;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								wmi_handle,
								event->pdev_id);

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_DISA
/**
 * extract_encrypt_decrypt_resp_event_tlv() - extract encrypt decrypt resp
 *      params from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: Pointer to hold resp parameters
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_encrypt_decrypt_resp_event_tlv(wmi_unified_t wmi_handle,
				       void *evt_buf,
				       struct disa_encrypt_decrypt_resp_params
				       *resp)
{
	WMI_VDEV_ENCRYPT_DECRYPT_DATA_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_encrypt_decrypt_data_resp_event_fixed_param *data_event;

	param_buf = evt_buf;
	if (!param_buf) {
		wmi_err("encrypt decrypt resp evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	data_event = param_buf->fixed_param;

	resp->vdev_id = data_event->vdev_id;
	resp->status = data_event->status;

	if ((data_event->data_length > param_buf->num_enc80211_frame) ||
	    (data_event->data_length > WMI_SVC_MSG_MAX_SIZE -
		 WMI_TLV_HDR_SIZE - sizeof(*data_event))) {
		wmi_err("FW msg data_len %d more than TLV hdr %d",
			 data_event->data_length,
			 param_buf->num_enc80211_frame);
		return QDF_STATUS_E_INVAL;
	}

	resp->data_len = data_event->data_length;

	if (resp->data_len)
		resp->data = (uint8_t *)param_buf->enc80211_frame;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_DISA */

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

static uint16_t wmi_tag_fw_hang_cmd(wmi_unified_t wmi_handle)
{
	uint16_t tag = 0;

	if (qdf_atomic_read(&wmi_handle->is_target_suspended)) {
		qdf_nofl_err("%s: Target is already suspended, Ignore FW Hang Command",
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
	case WMI_HB_SET_ENABLE_CMDID:
	case WMI_WOW_SET_ACTION_WAKE_UP_CMDID:
#ifdef FEATURE_WLAN_D0WOW
	case WMI_D0_WOW_ENABLE_DISABLE_CMDID:
#endif
		htc_tx_tag = HTC_TX_PACKET_TAG_AUTO_PM;
		break;
	case WMI_FORCE_FW_HANG_CMDID:
		htc_tx_tag = wmi_tag_fw_hang_cmd(wmi_handle);
		break;
	default:
		break;
	}

	return htc_tx_tag;
}

#ifdef CONFIG_BAND_6GHZ
static struct cur_reg_rule
*create_ext_reg_rules_from_wmi(uint32_t num_reg_rules,
		wmi_regulatory_rule_ext_struct *wmi_reg_rule)
{
	struct cur_reg_rule *reg_rule_ptr;
	uint32_t count;

	if (!num_reg_rules)
		return NULL;

	reg_rule_ptr = qdf_mem_malloc(num_reg_rules *
				      sizeof(*reg_rule_ptr));

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
		reg_rule_ptr[count].psd_flag =
			WMI_REG_RULE_PSD_FLAG_GET(
					wmi_reg_rule[count].psd_power_info);
		reg_rule_ptr[count].psd_eirp =
			WMI_REG_RULE_PSD_EIRP_GET(
					wmi_reg_rule[count].psd_power_info);
	}

	return reg_rule_ptr;
}
#endif

static struct cur_reg_rule
*create_reg_rules_from_wmi(uint32_t num_reg_rules,
		wmi_regulatory_rule_struct *wmi_reg_rule)
{
	struct cur_reg_rule *reg_rule_ptr;
	uint32_t count;

	if (!num_reg_rules)
		return NULL;

	reg_rule_ptr = qdf_mem_malloc(num_reg_rules *
				      sizeof(*reg_rule_ptr));

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

static enum cc_setting_code wmi_reg_status_to_reg_status(
				WMI_REG_SET_CC_STATUS_CODE wmi_status_code)
{
	if (wmi_status_code == WMI_REG_SET_CC_STATUS_PASS)
		return REG_SET_CC_STATUS_PASS;
	else if (wmi_status_code == WMI_REG_CURRENT_ALPHA2_NOT_FOUND)
		return REG_CURRENT_ALPHA2_NOT_FOUND;
	else if (wmi_status_code == WMI_REG_INIT_ALPHA2_NOT_FOUND)
		return REG_INIT_ALPHA2_NOT_FOUND;
	else if (wmi_status_code == WMI_REG_SET_CC_CHANGE_NOT_ALLOWED)
		return REG_SET_CC_CHANGE_NOT_ALLOWED;
	else if (wmi_status_code == WMI_REG_SET_CC_STATUS_NO_MEMORY)
		return REG_SET_CC_STATUS_NO_MEMORY;
	else if (wmi_status_code == WMI_REG_SET_CC_STATUS_FAIL)
		return REG_SET_CC_STATUS_FAIL;

	wmi_debug("Unknown reg status code from WMI");
	return REG_SET_CC_STATUS_FAIL;
}

#ifdef CONFIG_BAND_6GHZ
static QDF_STATUS extract_reg_chan_list_ext_update_event_tlv(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct cur_regulatory_info *reg_info, uint32_t len)
{
	uint32_t i, j, k;
	WMI_REG_CHAN_LIST_CC_EXT_EVENTID_param_tlvs *param_buf;
	wmi_reg_chan_list_cc_event_ext_fixed_param *ext_chan_list_event_hdr;
	wmi_regulatory_rule_ext_struct *ext_wmi_reg_rule;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;
	uint32_t num_6g_reg_rules_ap[REG_CURRENT_MAX_AP_TYPE];
	uint32_t *num_6g_reg_rules_client[REG_CURRENT_MAX_AP_TYPE];
	uint32_t total_reg_rules = 0;

	param_buf = (WMI_REG_CHAN_LIST_CC_EXT_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("invalid channel list event buf");
		return QDF_STATUS_E_FAILURE;
	}

	ext_chan_list_event_hdr = param_buf->fixed_param;

	reg_info->num_2g_reg_rules = ext_chan_list_event_hdr->num_2g_reg_rules;
	reg_info->num_5g_reg_rules = ext_chan_list_event_hdr->num_5g_reg_rules;
	reg_info->num_6g_reg_rules_ap[REG_STANDARD_POWER_AP] =
		ext_chan_list_event_hdr->num_6g_reg_rules_ap_sp;
	reg_info->num_6g_reg_rules_ap[REG_INDOOR_AP] =
		ext_chan_list_event_hdr->num_6g_reg_rules_ap_lpi;
	reg_info->num_6g_reg_rules_ap[REG_VERY_LOW_POWER_AP] =
		ext_chan_list_event_hdr->num_6g_reg_rules_ap_vlp;

	wmi_debug("num reg rules from fw");
	wmi_debug("AP SP %d, LPI %d, VLP %d",
		  reg_info->num_6g_reg_rules_ap[REG_STANDARD_POWER_AP],
		  reg_info->num_6g_reg_rules_ap[REG_INDOOR_AP],
		  reg_info->num_6g_reg_rules_ap[REG_VERY_LOW_POWER_AP]);

	for (i = 0; i < REG_MAX_CLIENT_TYPE; i++) {
		reg_info->num_6g_reg_rules_client[REG_STANDARD_POWER_AP][i] =
			ext_chan_list_event_hdr->num_6g_reg_rules_client_sp[i];
		reg_info->num_6g_reg_rules_client[REG_INDOOR_AP][i] =
			ext_chan_list_event_hdr->num_6g_reg_rules_client_lpi[i];
		reg_info->num_6g_reg_rules_client[REG_VERY_LOW_POWER_AP][i] =
			ext_chan_list_event_hdr->num_6g_reg_rules_client_vlp[i];
		wmi_debug("client %d SP %d, LPI %d, VLP %d", i,
			ext_chan_list_event_hdr->num_6g_reg_rules_client_sp[i],
			ext_chan_list_event_hdr->num_6g_reg_rules_client_lpi[i],
			ext_chan_list_event_hdr->num_6g_reg_rules_client_vlp[i]);
	}

	num_2g_reg_rules = reg_info->num_2g_reg_rules;
	total_reg_rules += num_2g_reg_rules;
	num_5g_reg_rules = reg_info->num_5g_reg_rules;
	total_reg_rules += num_5g_reg_rules;
	for (i = 0; i < REG_CURRENT_MAX_AP_TYPE; i++) {
		num_6g_reg_rules_ap[i] = reg_info->num_6g_reg_rules_ap[i];
		if (num_6g_reg_rules_ap[i] > MAX_6G_REG_RULES) {
			wmi_err_rl("Invalid num_6g_reg_rules_ap: %u",
				   num_6g_reg_rules_ap[i]);
			return QDF_STATUS_E_FAILURE;
		}
		total_reg_rules += num_6g_reg_rules_ap[i];
		num_6g_reg_rules_client[i] =
			reg_info->num_6g_reg_rules_client[i];
	}

	for (i = 0; i < REG_MAX_CLIENT_TYPE; i++) {
		total_reg_rules +=
			num_6g_reg_rules_client[REG_STANDARD_POWER_AP][i];
		total_reg_rules += num_6g_reg_rules_client[REG_INDOOR_AP][i];
		total_reg_rules +=
			num_6g_reg_rules_client[REG_VERY_LOW_POWER_AP][i];
		if ((num_6g_reg_rules_client[REG_STANDARD_POWER_AP][i] >
		     MAX_6G_REG_RULES) ||
		    (num_6g_reg_rules_client[REG_INDOOR_AP][i] >
		     MAX_6G_REG_RULES) ||
		    (num_6g_reg_rules_client[REG_VERY_LOW_POWER_AP][i] >
		     MAX_6G_REG_RULES)) {
			wmi_err_rl("Invalid num_6g_reg_rules_client_sp: %u, num_6g_reg_rules_client_lpi: %u, num_6g_reg_rules_client_vlp: %u, client %d",
				num_6g_reg_rules_client[REG_STANDARD_POWER_AP][i],
				num_6g_reg_rules_client[REG_INDOOR_AP][i],
				num_6g_reg_rules_client[REG_VERY_LOW_POWER_AP][i],
				i);
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (total_reg_rules != param_buf->num_reg_rule_array) {
		wmi_err_rl("Total reg rules %u does not match event params num reg rule %u",
			   total_reg_rules, param_buf->num_reg_rule_array);
		return QDF_STATUS_E_FAILURE;
	}

	if ((num_2g_reg_rules > MAX_REG_RULES) ||
	    (num_5g_reg_rules > MAX_REG_RULES)) {
		wmi_err_rl("Invalid num_2g_reg_rules: %u, num_5g_reg_rules: %u",
			   num_2g_reg_rules, num_5g_reg_rules);
		return QDF_STATUS_E_FAILURE;
	}

	if ((num_6g_reg_rules_ap[REG_STANDARD_POWER_AP] > MAX_6G_REG_RULES) ||
	    (num_6g_reg_rules_ap[REG_INDOOR_AP] > MAX_6G_REG_RULES) ||
	    (num_6g_reg_rules_ap[REG_VERY_LOW_POWER_AP] > MAX_6G_REG_RULES)) {
		wmi_err_rl("Invalid num_6g_reg_rules_ap_sp: %u, num_6g_reg_rules_ap_lpi: %u, num_6g_reg_rules_ap_vlp: %u",
			   num_6g_reg_rules_ap[REG_STANDARD_POWER_AP],
			   num_6g_reg_rules_ap[REG_INDOOR_AP],
			   num_6g_reg_rules_ap[REG_VERY_LOW_POWER_AP]);
		return QDF_STATUS_E_FAILURE;
	}

	if (param_buf->num_reg_rule_array >
		(WMI_SVC_MSG_MAX_SIZE - sizeof(*ext_chan_list_event_hdr)) /
		sizeof(*ext_wmi_reg_rule)) {
		wmi_err_rl("Invalid ext_num_reg_rule_array: %u",
			   param_buf->num_reg_rule_array);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(reg_info->alpha2, &ext_chan_list_event_hdr->alpha2,
		     REG_ALPHA2_LEN);
	reg_info->dfs_region = ext_chan_list_event_hdr->dfs_region;
	reg_info->phybitmap = convert_phybitmap_tlv(
			ext_chan_list_event_hdr->phybitmap);
	reg_info->offload_enabled = true;
	reg_info->num_phy = ext_chan_list_event_hdr->num_phy;
	reg_info->phy_id = wmi_handle->ops->convert_phy_id_target_to_host(
				wmi_handle, ext_chan_list_event_hdr->phy_id);
	reg_info->ctry_code = ext_chan_list_event_hdr->country_id;
	reg_info->reg_dmn_pair = ext_chan_list_event_hdr->domain_code;

	reg_info->status_code =
		wmi_reg_status_to_reg_status(ext_chan_list_event_hdr->
								status_code);

	reg_info->min_bw_2g = ext_chan_list_event_hdr->min_bw_2g;
	reg_info->max_bw_2g = ext_chan_list_event_hdr->max_bw_2g;
	reg_info->min_bw_5g = ext_chan_list_event_hdr->min_bw_5g;
	reg_info->max_bw_5g = ext_chan_list_event_hdr->max_bw_5g;
	reg_info->min_bw_6g_ap[REG_STANDARD_POWER_AP] =
		ext_chan_list_event_hdr->min_bw_6g_ap_sp;
	reg_info->max_bw_6g_ap[REG_STANDARD_POWER_AP] =
		ext_chan_list_event_hdr->max_bw_6g_ap_sp;
	reg_info->min_bw_6g_ap[REG_INDOOR_AP] =
		ext_chan_list_event_hdr->min_bw_6g_ap_lpi;
	reg_info->max_bw_6g_ap[REG_INDOOR_AP] =
		ext_chan_list_event_hdr->max_bw_6g_ap_lpi;
	reg_info->min_bw_6g_ap[REG_VERY_LOW_POWER_AP] =
		ext_chan_list_event_hdr->min_bw_6g_ap_vlp;
	reg_info->max_bw_6g_ap[REG_VERY_LOW_POWER_AP] =
		ext_chan_list_event_hdr->max_bw_6g_ap_vlp;

	for (i = 0; i < REG_MAX_CLIENT_TYPE; i++) {
		reg_info->min_bw_6g_client[REG_STANDARD_POWER_AP][i] =
			ext_chan_list_event_hdr->min_bw_6g_client_sp[i];
		reg_info->max_bw_6g_client[REG_STANDARD_POWER_AP][i] =
			ext_chan_list_event_hdr->max_bw_6g_client_sp[i];
		reg_info->min_bw_6g_client[REG_INDOOR_AP][i] =
			ext_chan_list_event_hdr->min_bw_6g_client_lpi[i];
		reg_info->max_bw_6g_client[REG_INDOOR_AP][i] =
			ext_chan_list_event_hdr->max_bw_6g_client_lpi[i];
		reg_info->min_bw_6g_client[REG_VERY_LOW_POWER_AP][i] =
			ext_chan_list_event_hdr->min_bw_6g_client_vlp[i];
		reg_info->max_bw_6g_client[REG_VERY_LOW_POWER_AP][i] =
			ext_chan_list_event_hdr->max_bw_6g_client_vlp[i];
	}

	wmi_debug("num_phys = %u and phy_id = %u",
		  reg_info->num_phy, reg_info->phy_id);

	wmi_debug("cc %s dfs %d BW: min_2g %d max_2g %d min_5g %d max_5g %d",
		  reg_info->alpha2, reg_info->dfs_region, reg_info->min_bw_2g,
		  reg_info->max_bw_2g, reg_info->min_bw_5g,
		  reg_info->max_bw_5g);

	wmi_debug("min_bw_6g_ap_sp %d max_bw_6g_ap_sp %d min_bw_6g_ap_lpi %d max_bw_6g_ap_lpi %d min_bw_6g_ap_vlp %d max_bw_6g_ap_vlp %d",
		  reg_info->min_bw_6g_ap[REG_STANDARD_POWER_AP],
		  reg_info->max_bw_6g_ap[REG_STANDARD_POWER_AP],
		  reg_info->min_bw_6g_ap[REG_INDOOR_AP],
		  reg_info->max_bw_6g_ap[REG_INDOOR_AP],
		  reg_info->min_bw_6g_ap[REG_VERY_LOW_POWER_AP],
		  reg_info->max_bw_6g_ap[REG_VERY_LOW_POWER_AP]);

	wmi_debug("min_bw_6g_def_cli_sp %d max_bw_6g_def_cli_sp %d min_bw_6g_def_cli_lpi %d max_bw_6g_def_cli_lpi %d min_bw_6g_def_cli_vlp %d max_bw_6g_def_cli_vlp %d",
		  reg_info->min_bw_6g_client[REG_STANDARD_POWER_AP][REG_DEFAULT_CLIENT],
		  reg_info->max_bw_6g_client[REG_STANDARD_POWER_AP][REG_DEFAULT_CLIENT],
		  reg_info->min_bw_6g_client[REG_INDOOR_AP][REG_DEFAULT_CLIENT],
		  reg_info->max_bw_6g_client[REG_INDOOR_AP][REG_DEFAULT_CLIENT],
		  reg_info->min_bw_6g_client[REG_VERY_LOW_POWER_AP][REG_DEFAULT_CLIENT],
		  reg_info->max_bw_6g_client[REG_VERY_LOW_POWER_AP][REG_DEFAULT_CLIENT]);

	wmi_debug("min_bw_6g_sub_client_sp %d max_bw_6g_sub_client_sp %d min_bw_6g_sub_client_lpi %d max_bw_6g_sub_client_lpi %d min_bw_6g_sub_client_vlp %d max_bw_6g_sub_client_vlp %d",
		  reg_info->min_bw_6g_client[REG_STANDARD_POWER_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->max_bw_6g_client[REG_STANDARD_POWER_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->min_bw_6g_client[REG_INDOOR_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->max_bw_6g_client[REG_INDOOR_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->min_bw_6g_client[REG_VERY_LOW_POWER_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->max_bw_6g_client[REG_VERY_LOW_POWER_AP][REG_SUBORDINATE_CLIENT]);

	wmi_debug("num_2g_reg_rules %d num_5g_reg_rules %d",
		  num_2g_reg_rules, num_5g_reg_rules);

	wmi_debug("num_6g_ap_sp_reg_rules %d num_6g_ap_lpi_reg_rules %d num_6g_ap_vlp_reg_rules %d",
		  reg_info->num_6g_reg_rules_ap[REG_STANDARD_POWER_AP],
		  reg_info->num_6g_reg_rules_ap[REG_INDOOR_AP],
		  reg_info->num_6g_reg_rules_ap[REG_VERY_LOW_POWER_AP]);

	wmi_debug("num_6g_def_cli_sp_reg_rules %d num_6g_def_cli_lpi_reg_rules %d num_6g_def_cli_vlp_reg_rules %d",
		  reg_info->num_6g_reg_rules_client[REG_STANDARD_POWER_AP][REG_DEFAULT_CLIENT],
		  reg_info->num_6g_reg_rules_client[REG_INDOOR_AP][REG_DEFAULT_CLIENT],
		  reg_info->num_6g_reg_rules_client[REG_VERY_LOW_POWER_AP][REG_DEFAULT_CLIENT]);

	wmi_debug("num_6g_sub_cli_sp_reg_rules %d num_6g_sub_cli_lpi_reg_rules %d num_6g_sub_cli_vlp_reg_rules %d",
		  reg_info->num_6g_reg_rules_client[REG_STANDARD_POWER_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->num_6g_reg_rules_client[REG_INDOOR_AP][REG_SUBORDINATE_CLIENT],
		  reg_info->num_6g_reg_rules_client[REG_VERY_LOW_POWER_AP][REG_SUBORDINATE_CLIENT]);

	ext_wmi_reg_rule =
		(wmi_regulatory_rule_ext_struct *)
			((uint8_t *)ext_chan_list_event_hdr +
			sizeof(wmi_reg_chan_list_cc_event_ext_fixed_param) +
			WMI_TLV_HDR_SIZE);
	reg_info->reg_rules_2g_ptr =
		create_ext_reg_rules_from_wmi(num_2g_reg_rules,
					      ext_wmi_reg_rule);
	ext_wmi_reg_rule += num_2g_reg_rules;
	for (i = 0; i < num_2g_reg_rules; i++) {
		if (!reg_info->reg_rules_2g_ptr)
			break;
		wmi_debug("2g rule %d start freq %d end freq %d flags %d",
			  i, reg_info->reg_rules_2g_ptr[i].start_freq,
			  reg_info->reg_rules_2g_ptr[i].end_freq,
			  reg_info->reg_rules_2g_ptr[i].flags);
	}
	reg_info->reg_rules_5g_ptr =
		create_ext_reg_rules_from_wmi(num_5g_reg_rules,
					      ext_wmi_reg_rule);
	ext_wmi_reg_rule += num_5g_reg_rules;
	for (i = 0; i < num_5g_reg_rules; i++) {
		if (!reg_info->reg_rules_5g_ptr)
			break;
		wmi_debug("5g rule %d start freq %d end freq %d flags %d",
			  i, reg_info->reg_rules_5g_ptr[i].start_freq,
			  reg_info->reg_rules_5g_ptr[i].end_freq,
			  reg_info->reg_rules_5g_ptr[i].flags);
	}

	for (i = 0; i < REG_CURRENT_MAX_AP_TYPE; i++) {
		reg_info->reg_rules_6g_ap_ptr[i] =
			create_ext_reg_rules_from_wmi(num_6g_reg_rules_ap[i],
						      ext_wmi_reg_rule);

		ext_wmi_reg_rule += num_6g_reg_rules_ap[i];
		for (j = 0; j < num_6g_reg_rules_ap[i]; j++) {
			if (!reg_info->reg_rules_6g_ap_ptr[i])
				break;
			wmi_debug("6g pwr type %d AP rule %d start freq %d end freq %d flags %d",
				  i, j,
				  reg_info->reg_rules_6g_ap_ptr[i][j].start_freq,
				  reg_info->reg_rules_6g_ap_ptr[i][j].end_freq,
				  reg_info->reg_rules_6g_ap_ptr[i][j].flags);
		}
	}

	for (j = 0; j < REG_CURRENT_MAX_AP_TYPE; j++) {
		for (i = 0; i < REG_MAX_CLIENT_TYPE; i++) {
			reg_info->reg_rules_6g_client_ptr[j][i] =
				create_ext_reg_rules_from_wmi(
					num_6g_reg_rules_client[j][i],
					ext_wmi_reg_rule);

			ext_wmi_reg_rule += num_6g_reg_rules_client[j][i];
			for (k = 0; k < num_6g_reg_rules_client[j][i]; k++) {
				if (!reg_info->reg_rules_6g_client_ptr[j][i])
					break;
				wmi_debug("6g pwr type %d cli type %d CLI rule %d start freq %d end freq %d flags %d",
					  j, i, k,
					  reg_info->reg_rules_6g_client_ptr[j][i][k].start_freq,
					  reg_info->reg_rules_6g_client_ptr[j][i][k].end_freq,
					  reg_info->reg_rules_6g_client_ptr[j][i][k].flags);
			}
		}
	}

	reg_info->client_type = ext_chan_list_event_hdr->client_type;
	reg_info->rnr_tpe_usable = ext_chan_list_event_hdr->rnr_tpe_usable;
	reg_info->unspecified_ap_usable =
		ext_chan_list_event_hdr->unspecified_ap_usable;
	reg_info->domain_code_6g_ap[REG_STANDARD_POWER_AP] =
		ext_chan_list_event_hdr->domain_code_6g_ap_sp;
	reg_info->domain_code_6g_ap[REG_INDOOR_AP] =
		ext_chan_list_event_hdr->domain_code_6g_ap_lpi;
	reg_info->domain_code_6g_ap[REG_VERY_LOW_POWER_AP] =
		ext_chan_list_event_hdr->domain_code_6g_ap_vlp;

	wmi_debug("client type %d", reg_info->client_type);
	wmi_debug("RNR TPE usable %d", reg_info->rnr_tpe_usable);
	wmi_debug("unspecified AP usable %d", reg_info->unspecified_ap_usable);
	wmi_debug("domain code AP SP %d, LPI %d, VLP %d",
		  reg_info->domain_code_6g_ap[REG_STANDARD_POWER_AP],
		  reg_info->domain_code_6g_ap[REG_INDOOR_AP],
		  reg_info->domain_code_6g_ap[REG_VERY_LOW_POWER_AP]);

	for (i = 0; i < REG_MAX_CLIENT_TYPE; i++) {
		reg_info->domain_code_6g_client[REG_STANDARD_POWER_AP][i] =
			ext_chan_list_event_hdr->domain_code_6g_client_sp[i];
		reg_info->domain_code_6g_client[REG_INDOOR_AP][i] =
			ext_chan_list_event_hdr->domain_code_6g_client_lpi[i];
		reg_info->domain_code_6g_client[REG_VERY_LOW_POWER_AP][i] =
			ext_chan_list_event_hdr->domain_code_6g_client_vlp[i];
		wmi_debug("domain code client %d SP %d, LPI %d, VLP %d", i,
			reg_info->domain_code_6g_client[REG_STANDARD_POWER_AP][i],
			reg_info->domain_code_6g_client[REG_INDOOR_AP][i],
			reg_info->domain_code_6g_client[REG_VERY_LOW_POWER_AP][i]);
	}

	reg_info->domain_code_6g_super_id =
		ext_chan_list_event_hdr->domain_code_6g_super_id;

	wmi_debug("processed regulatory extended channel list");

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_AFC_SUPPORT
/**
 * copy_afc_chan_eirp_info() - Copy the channel EIRP object from
 * chan_eirp_power_info_hdr to the internal buffer chan_eirp_info. Since the
 * cfi and eirp is continuously filled in chan_eirp_power_info_hdr, there is
 * an index pointer required to store the current index of
 * chan_eirp_power_info_hdr, to fill into the chan_eirp_info object.
 * @chan_eirp_info: pointer to chan_eirp_info
 * @num_chans: Number of channels
 * @chan_eirp_power_info_hdr: Pointer to chan_eirp_power_info_hdr
 * @index: Pointer to index
 *
 * Return: void
 */
static void
copy_afc_chan_eirp_info(struct chan_eirp_obj *chan_eirp_info,
			uint8_t num_chans,
			wmi_afc_chan_eirp_power_info *chan_eirp_power_info_hdr,
			uint8_t *index)
{
	uint8_t chan_idx;

	for (chan_idx = 0; chan_idx < num_chans; chan_idx++, (*index)++) {
		chan_eirp_info[chan_idx].cfi =
				chan_eirp_power_info_hdr[*index].channel_cfi;
		chan_eirp_info[chan_idx].eirp_power =
				chan_eirp_power_info_hdr[*index].eirp_pwr;
	}
}

/**
 * copy_afc_chan_obj_info() - Copy the channel object from channel_info_hdr to
 * to the internal buffer afc_chan_info.
 * @afc_chan_info: pointer to afc_chan_info
 * @num_chan_objs: Number of channel objects
 * @channel_info_hdr: Pointer to channel_info_hdr
 * @chan_eirp_power_info_hdr: Pointer to chan_eirp_power_info_hdr
 *
 * Return: void
 */
static void
copy_afc_chan_obj_info(struct afc_chan_obj *afc_chan_info,
		       uint8_t num_chan_objs,
		       wmi_6g_afc_channel_info *channel_info_hdr,
		       wmi_afc_chan_eirp_power_info *chan_eirp_power_info_hdr)
{
	uint8_t count;
	uint8_t src_pwr_index = 0;

	for (count = 0; count < num_chan_objs; count++) {
		afc_chan_info[count].global_opclass =
			channel_info_hdr[count].global_operating_class;
		afc_chan_info[count].num_chans =
					channel_info_hdr[count].num_channels;

		if (afc_chan_info[count].num_chans > 0) {
			struct chan_eirp_obj *chan_eirp_info;

			chan_eirp_info =
				qdf_mem_malloc(afc_chan_info[count].num_chans *
					       sizeof(*chan_eirp_info));

			if (!chan_eirp_info)
				return;

			copy_afc_chan_eirp_info(chan_eirp_info,
						afc_chan_info[count].num_chans,
						chan_eirp_power_info_hdr,
						&src_pwr_index);
			afc_chan_info[count].chan_eirp_info = chan_eirp_info;
		} else {
			wmi_err("Number of channels is zero in object idx %d",
				count);
		}
	}
}

static void copy_afc_freq_obj_info(struct afc_freq_obj *afc_freq_info,
				   uint8_t num_freq_objs,
				   wmi_6g_afc_frequency_info *freq_info_hdr)
{
	uint8_t count;

	for (count = 0; count < num_freq_objs; count++) {
		afc_freq_info[count].low_freq =
		WMI_REG_RULE_START_FREQ_GET(freq_info_hdr[count].freq_info);
		afc_freq_info[count].high_freq =
		WMI_REG_RULE_END_FREQ_GET(freq_info_hdr[count].freq_info);
		afc_freq_info[count].max_psd =
					freq_info_hdr[count].psd_power_info;
	}
}

/**
 * copy_afc_event_fixed_hdr_power_info() - Copy the fixed header portion of
 * the power event info from the WMI AFC event buffer to the internal buffer
 * power_info.
 * @power_info: pointer to power_info
 * @afc_power_event_hdr: pointer to afc_power_event_hdr
 *
 * Return: void
 */
static void
copy_afc_event_fixed_hdr_power_info(
		struct reg_fw_afc_power_event *power_info,
		wmi_afc_power_event_param *afc_power_event_hdr)
{
	power_info->fw_status_code = afc_power_event_hdr->fw_status_code;
	power_info->resp_id = afc_power_event_hdr->resp_id;
	power_info->serv_resp_code = afc_power_event_hdr->afc_serv_resp_code;
	power_info->afc_wfa_version =
	WMI_AFC_WFA_MINOR_VERSION_GET(afc_power_event_hdr->afc_wfa_version);
	power_info->afc_wfa_version |=
	WMI_AFC_WFA_MAJOR_VERSION_GET(afc_power_event_hdr->afc_wfa_version);

	power_info->avail_exp_time_d =
	WMI_AVAIL_EXPIRY_TIME_DAY_GET(afc_power_event_hdr->avail_exp_time_d);
	power_info->avail_exp_time_d |=
	WMI_AVAIL_EXPIRY_TIME_MONTH_GET(afc_power_event_hdr->avail_exp_time_d);
	power_info->avail_exp_time_d |=
	WMI_AVAIL_EXPIRY_TIME_YEAR_GET(afc_power_event_hdr->avail_exp_time_d);

	power_info->avail_exp_time_t =
	WMI_AVAIL_EXPIRY_TIME_SEC_GET(afc_power_event_hdr->avail_exp_time_t);
	power_info->avail_exp_time_t |=
	WMI_AVAIL_EXPIRY_TIME_MINUTE_GET(afc_power_event_hdr->avail_exp_time_t);
	power_info->avail_exp_time_t |=
	WMI_AVAIL_EXPIRY_TIME_HOUR_GET(afc_power_event_hdr->avail_exp_time_t);
}

/**
 * copy_power_event() - Copy the power event parameters from the AFC event
 * buffer to the power_info within the afc_info.
 * @afc_info: pointer to afc_info
 * @param_buf: pointer to param_buf
 *
 * Return: void
 */
static void copy_power_event(struct afc_regulatory_info *afc_info,
			     WMI_AFC_EVENTID_param_tlvs *param_buf)
{
	struct reg_fw_afc_power_event *power_info;
	wmi_afc_power_event_param *afc_power_event_hdr;
	struct afc_freq_obj *afc_freq_info;

	power_info = qdf_mem_malloc(sizeof(*power_info));

	if (!power_info)
		return;

	afc_power_event_hdr = param_buf->afc_power_event_param;
	copy_afc_event_fixed_hdr_power_info(power_info, afc_power_event_hdr);
	afc_info->power_info = power_info;

	power_info->num_freq_objs = param_buf->num_freq_info_array;
	if (power_info->num_freq_objs > 0) {
		wmi_6g_afc_frequency_info *freq_info_hdr;

		freq_info_hdr = param_buf->freq_info_array;
		afc_freq_info = qdf_mem_malloc(power_info->num_freq_objs *
					       sizeof(*afc_freq_info));

		if (!afc_freq_info)
			return;

		copy_afc_freq_obj_info(afc_freq_info, power_info->num_freq_objs,
				       freq_info_hdr);
		power_info->afc_freq_info = afc_freq_info;
	} else {
		wmi_err("Number of frequency objects is zero");
	}

	power_info->num_chan_objs = param_buf->num_channel_info_array;
	if (power_info->num_chan_objs > 0) {
		struct afc_chan_obj *afc_chan_info;
		wmi_6g_afc_channel_info *channel_info_hdr;

		channel_info_hdr = param_buf->channel_info_array;
		afc_chan_info = qdf_mem_malloc(power_info->num_chan_objs *
					       sizeof(*afc_chan_info));

		if (!afc_chan_info)
			return;

		copy_afc_chan_obj_info(afc_chan_info,
				       power_info->num_chan_objs,
				       channel_info_hdr,
				       param_buf->chan_eirp_power_info_array);
		power_info->afc_chan_info = afc_chan_info;
	} else {
		wmi_err("Number of channel objects is zero");
	}
}

static void copy_expiry_event(struct afc_regulatory_info *afc_info,
			      WMI_AFC_EVENTID_param_tlvs *param_buf)
{
	struct reg_afc_expiry_event *expiry_info;

	expiry_info = qdf_mem_malloc(sizeof(*expiry_info));

	if (!expiry_info)
		return;

	expiry_info->request_id =
				param_buf->expiry_event_param->request_id;
	expiry_info->event_subtype =
				param_buf->expiry_event_param->event_subtype;
	afc_info->expiry_info = expiry_info;
}

/**
 * copy_afc_event_common_info() - Copy the phy_id and event_type parameters
 * in the AFC event. 'Common' indicates that these parameters are common for
 * WMI_AFC_EVENT_POWER_INFO and WMI_AFC_EVENT_TIMER_EXPIRY.
 * @wmi_handle: wmi handle
 * @afc_info: pointer to afc_info
 * @event_fixed_hdr: pointer to event_fixed_hdr
 *
 * Return: void
 */
static void
copy_afc_event_common_info(wmi_unified_t wmi_handle,
			   struct afc_regulatory_info *afc_info,
			   wmi_afc_event_fixed_param *event_fixed_hdr)
{
	afc_info->phy_id = wmi_handle->ops->convert_phy_id_target_to_host(
				wmi_handle, event_fixed_hdr->phy_id);
	afc_info->event_type = event_fixed_hdr->event_type;
}

static QDF_STATUS extract_afc_event_tlv(wmi_unified_t wmi_handle,
					uint8_t *evt_buf,
					struct afc_regulatory_info *afc_info,
					uint32_t len)
{
	WMI_AFC_EVENTID_param_tlvs *param_buf;
	wmi_afc_event_fixed_param *event_fixed_hdr;

	param_buf = (WMI_AFC_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid AFC event buf");
		return QDF_STATUS_E_FAILURE;
	}

	event_fixed_hdr = param_buf->fixed_param;
	copy_afc_event_common_info(wmi_handle, afc_info, event_fixed_hdr);

	switch (afc_info->event_type) {
	case WMI_AFC_EVENT_POWER_INFO:
		copy_power_event(afc_info, param_buf);
		break;
	case WMI_AFC_EVENT_TIMER_EXPIRY:
		copy_expiry_event(afc_info, param_buf);
		return QDF_STATUS_SUCCESS;
	default:
		wmi_err("Invalid event type");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif
#endif

static QDF_STATUS extract_reg_chan_list_update_event_tlv(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct cur_regulatory_info *reg_info, uint32_t len)
{
	WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *param_buf;
	wmi_reg_chan_list_cc_event_fixed_param *chan_list_event_hdr;
	wmi_regulatory_rule_struct *wmi_reg_rule;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;

	wmi_debug("processing regulatory channel list");

	param_buf = (WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("invalid channel list event buf");
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
	reg_info->phybitmap = convert_phybitmap_tlv(
			chan_list_event_hdr->phybitmap);
	reg_info->offload_enabled = true;
	reg_info->num_phy = chan_list_event_hdr->num_phy;
	reg_info->phy_id = wmi_handle->ops->convert_phy_id_target_to_host(
				wmi_handle, chan_list_event_hdr->phy_id);
	reg_info->ctry_code = chan_list_event_hdr->country_id;
	reg_info->reg_dmn_pair = chan_list_event_hdr->domain_code;

	reg_info->status_code =
		wmi_reg_status_to_reg_status(chan_list_event_hdr->status_code);

	reg_info->min_bw_2g = chan_list_event_hdr->min_bw_2g;
	reg_info->max_bw_2g = chan_list_event_hdr->max_bw_2g;
	reg_info->min_bw_5g = chan_list_event_hdr->min_bw_5g;
	reg_info->max_bw_5g = chan_list_event_hdr->max_bw_5g;

	wmi_debug("num_phys = %u and phy_id = %u",
		 reg_info->num_phy, reg_info->phy_id);

	wmi_debug("cc %s dfs %d BW: min_2g %d max_2g %d min_5g %d max_5g %d",
		 reg_info->alpha2, reg_info->dfs_region,
		 reg_info->min_bw_2g, reg_info->max_bw_2g,
		 reg_info->min_bw_5g, reg_info->max_bw_5g);

	wmi_debug("num_2g_reg_rules %d num_5g_reg_rules %d",
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

	wmi_debug("processed regulatory channel list");

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
		wmi_err("invalid 11d country event buf");
		return QDF_STATUS_E_FAILURE;
	}

	reg_11d_country_event = param_buf->fixed_param;

	qdf_mem_copy(reg_11d_country->alpha2,
			&reg_11d_country_event->new_alpha2, REG_ALPHA2_LEN);
	reg_11d_country->alpha2[REG_ALPHA2_LEN] = '\0';

	wmi_debug("processed 11d country event, new cc %s",
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
		wmi_err("Invalid channel avoid event buffer");
		return QDF_STATUS_E_INVAL;
	}

	afr_fixed_param = param_buf->fixed_param;
	if (!afr_fixed_param) {
		wmi_err("Invalid channel avoid event fixed param buffer");
		return QDF_STATUS_E_INVAL;
	}

	if (!ch_avoid_ind) {
		wmi_err("Invalid channel avoid indication buffer");
		return QDF_STATUS_E_INVAL;
	}
	if (param_buf->num_avd_freq_range < afr_fixed_param->num_freq_ranges) {
		wmi_err("no.of freq ranges exceeded the limit");
		return QDF_STATUS_E_INVAL;
	}
	num_freq_ranges = (afr_fixed_param->num_freq_ranges >
			CH_AVOID_MAX_RANGE) ? CH_AVOID_MAX_RANGE :
			afr_fixed_param->num_freq_ranges;

	wmi_debug("Channel avoid event received with %d ranges",
		 num_freq_ranges);

	ch_avoid_ind->ch_avoid_range_cnt = num_freq_ranges;
	afr_desc = (wmi_avoid_freq_range_desc *)(param_buf->avd_freq_range);
	for (freq_range_idx = 0; freq_range_idx < num_freq_ranges;
	     freq_range_idx++) {
		ch_avoid_ind->avoid_freq_range[freq_range_idx].start_freq =
			afr_desc->start_freq;
		ch_avoid_ind->avoid_freq_range[freq_range_idx].end_freq =
			afr_desc->end_freq;
		wmi_debug("range %d tlv id %u, start freq %u, end freq %u",
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
		wmi_err("invalid cac complete event buf");
		return QDF_STATUS_E_FAILURE;
	}

	cac_event = param_tlvs->fixed_param;
	*vdev_id = cac_event->vdev_id;
	wmi_debug("processed cac complete event vdev %d", *vdev_id);

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
		wmi_err("invalid ocac complete event buf");
		return QDF_STATUS_E_FAILURE;
	}

	if (!param_tlvs->fixed_param) {
		wmi_err("invalid param_tlvs->fixed_param");
		return QDF_STATUS_E_FAILURE;
	}

	ocac_complete_status = param_tlvs->fixed_param;
	param->vdev_id = ocac_complete_status->vdev_id;
	param->chan_freq = ocac_complete_status->chan_freq;
	param->center_freq1 = ocac_complete_status->center_freq1;
	param->center_freq2 = ocac_complete_status->center_freq2;
	param->ocac_status = ocac_complete_status->status;
	param->chan_width = ocac_complete_status->chan_width;
	wmi_debug("processed ocac complete event vdev %d"
		 " agile chan %d %d width %d status %d",
		 param->vdev_id,
		 param->center_freq1,
		 param->center_freq2,
		 param->chan_width,
		 param->ocac_status);

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
		wmi_err("invalid radar detection event buf");
		return QDF_STATUS_E_FAILURE;
	}

	radar_event = param_tlv->fixed_param;

	radar_found->pdev_id = convert_target_pdev_id_to_host_pdev_id(
						wmi_handle,
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

	wmi_debug("processed radar found event pdev %d,"
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

#ifdef MOBILE_DFS_SUPPORT
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
		wmi_err("invalid wlan radar event buf");
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
	wmi_debug("RCPI REQ VDEV_ID:%d-->", cmd->vdev_id);
	wmi_mtrace(WMI_REQUEST_RCPI_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_RCPI_CMDID)) {

		wmi_err("Failed to send WMI_REQUEST_RCPI_CMDID");
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
		wmi_err("Invalid rcpi event");
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
		wmi_err("Invalid rcpi measurement type from firmware");
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
						       wmi_unified_t wmi_handle,
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
						       wmi_unified_t wmi_handle,
						       uint32_t pdev_id)
{
	/*No conversion required*/
	return pdev_id;
}

/**
 * convert_host_phy_id_to_target_phy_id_legacy() - Convert phy_id from
 *	   host to target defines. For legacy there is not conversion
 *	   required. Just return phy_id as it is.
 * @param pdev_id: host phy_id to be converted.
 * Return: target phy_id after conversion.
 */
static uint32_t convert_host_phy_id_to_target_phy_id_legacy(
						       wmi_unified_t wmi_handle,
						       uint32_t phy_id)
{
	/*No conversion required*/
	return phy_id;
}

/**
 * convert_target_phy_id_to_host_phy_id_legacy() - Convert phy_id from
 *	   target to host defines. For legacy there is not conversion
 *	   required. Just return phy_id as it is.
 * @param pdev_id: target phy_id to be converted.
 * Return: host phy_id after conversion.
 */
static uint32_t convert_target_phy_id_to_host_phy_id_legacy(
						       wmi_unified_t wmi_handle,
						       uint32_t phy_id)
{
	/*No conversion required*/
	return phy_id;
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

	cmd->pdev_id = wmi_handle->ops->convert_host_pdev_id_to_target(
							wmi_handle,
							pdev_id);
	wmi_debug("setting current country to  %s and target pdev_id = %u",
		 params->country, cmd->pdev_id);

	qdf_mem_copy((uint8_t *)&cmd->new_alpha2, params->country, 3);

	wmi_mtrace(WMI_SET_CURRENT_COUNTRY_CMDID, NO_SESSION, 0);
	qdf_status = wmi_unified_cmd_send(wmi_handle,
			buf, len, WMI_SET_CURRENT_COUNTRY_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		wmi_err("Failed to send WMI_SET_CURRENT_COUNTRY_CMDID");
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

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								pdev_id);

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
		WMI_SET_BITS(cmd->country_code.domain_code, 0, 16,
			     rd->cc.regdmn.reg_2g_5g_pair_id);
		WMI_SET_BITS(cmd->country_code.domain_code, 16, 16,
			     rd->cc.regdmn.sixg_superdmn_id);
	}

	wmi_mtrace(WMI_SET_INIT_COUNTRY_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_SET_INIT_COUNTRY_CMDID);
	if (ret) {
		wmi_err("Failed to config wow wakeup event");
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
		wmi_err("Failed to send WMI_SAP_OBSS_DETECTION_CFG_CMDID");
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
		wmi_err("Invalid obss_detection event buffer");
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_SAP_OBSS_DETECTION_REPORT_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid evt_buf");
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
		wmi_err("Invalid reason: %d", fix_param->reason);
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

	wmi_debug("Roam Scan Stats Req vdev_id: %u", cmd->vdev_id);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_ROAM_SCAN_STATS_CMDID)) {
		wmi_err("Failed to send WMI_REQUEST_ROAM_SCAN_STATS_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_roam_scan_ch_list_req_cmd_tlv() - send wmi cmd to get roam scan
 * channel list from firmware
 * @wmi_handle: wmi handler
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_roam_scan_ch_list_req_cmd_tlv(wmi_unified_t wmi_handle,
						     uint32_t vdev_id)
{
	wmi_buf_t buf;
	wmi_roam_get_scan_channel_list_cmd_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	int ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_roam_get_scan_channel_list_cmd_fixed_param *)
					wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_roam_get_scan_channel_list_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_roam_get_scan_channel_list_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	wmi_mtrace(WMI_ROAM_GET_SCAN_CHANNEL_LIST_CMDID, vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_ROAM_GET_SCAN_CHANNEL_LIST_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send get roam scan channels request = %d",
			 ret);
		wmi_buf_free(buf);
	}
	return ret;
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
		wmi_err("Invalid roam scan stats event");
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
		wmi_err("Invalid offload bcn tx status event buffer");
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
		wmi_err("Invalid EGAP Info status event buffer");
		return QDF_STATUS_E_INVAL;
	}

	egap_info_event = (wmi_ap_ps_egap_info_event_fixed_param *)
				param_buf->fixed_param;
	chainmask_event = (wmi_ap_ps_egap_info_chainmask_list *)
				param_buf->chainmask_list;

	if (!egap_info_event || !chainmask_event) {
		wmi_err("Invalid EGAP Info event or chainmask event");
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
		wmi_debug("Received null data from FW");
		return QDF_STATUS_E_FAILURE;
	}

	pe_hdr = param_tlvs->hdr;
	if (!pe_hdr) {
		wmi_debug("Received Data PE Header is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* Ensure it's at least the size of the header */
	if (datalen < sizeof(*pe_hdr)) {
		wmi_debug("Expected minimum size %zu, received %d",
			 sizeof(*pe_hdr), datalen);
		return QDF_STATUS_E_FAILURE;
	}

	phyerr->pdev_id = wmi_handle->ops->
		convert_pdev_id_target_to_host(wmi_handle, pe_hdr->pdev_id);
	phyerr->tsf64 = pe_hdr->tsf_l32;
	phyerr->tsf64 |= (((uint64_t)pe_hdr->tsf_u32) << 32);
	phyerr->bufp = param_tlvs->bufp;

	if (pe_hdr->buf_len > param_tlvs->num_bufp) {
		wmi_debug("Invalid buf_len %d, num_bufp %d",
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
			wmi_debug("Not enough space. len=%d, n=%d, hdr=%zu",
				 datalen, n, sizeof(ev->hdr));
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
			wmi_debug("buf_len is garbage 0x%x", ev->hdr.buf_len);
			return QDF_STATUS_E_FAILURE;
		}

		if ((n + ev->hdr.buf_len) > datalen) {
			wmi_debug("len exceeds n=%d, buf_len=%d, datalen=%d",
				 n, ev->hdr.buf_len, datalen);
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
		wmi_err("Invalid ESP Estimate Event buffer");
		return QDF_STATUS_E_INVAL;
	}
	esp_event = param_buf->fixed_param;
	param->ac_airtime_percentage = esp_event->ac_airtime_percentage;

	param->pdev_id = convert_target_pdev_id_to_host_pdev_id(
						wmi_handle,
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
		wmi_err("Failed to send WMI_BSS_COLOR_CHANGE_ENABLE_CMDID");
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
		wmi_err("Invalid event type: %d", cfg_param->evt_type);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	wmi_debug("evt_type: %d vdev id: %d current_bss_color: %d "
		 "detection_period_ms: %d scan_period_ms: %d "
		 "free_slot_expiry_timer_ms: %d",
		 cmd->evt_type, cmd->vdev_id, cmd->current_bss_color,
		 cmd->detection_period_ms, cmd->scan_period_ms,
		 cmd->free_slot_expiry_time_ms);

	wmi_mtrace(WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_OBSS_COLOR_COLLISION_DET_CONFIG_CMDID)) {
		wmi_err("Sending OBSS color det cmd failed, vdev_id: %d",
			 cfg_param->vdev_id);
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
		wmi_err("Invalid obss color buffer");
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_OBSS_COLOR_COLLISION_DETECTION_EVENTID_param_tlvs *)
		    evt_buf;
	if (!param_buf) {
		wmi_err("Invalid evt_buf");
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
		wmi_err("Invalid event type: %d, vdev_id: %d",
			 fix_param->evt_type, fix_param->vdev_id);
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

#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
static QDF_STATUS
send_vdev_fils_enable_cmd_send(struct wmi_unified *wmi_handle,
			       struct config_fils_params *param)
{
	wmi_buf_t buf;
	wmi_enable_fils_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_enable_fils_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_enable_fils_cmd_fixed_param *)wmi_buf_data(
			buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_enable_fils_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_enable_fils_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->fd_period = param->fd_period;
	if (param->send_prb_rsp_frame)
		cmd->flags |= WMI_FILS_FLAGS_BITMAP_BCAST_PROBE_RSP;
	wmi_debug("vdev id: %d fd_period: %d cmd->Flags %d",
		 cmd->vdev_id, cmd->fd_period, cmd->flags);
	wmi_mtrace(WMI_ENABLE_FILS_CMDID, cmd->vdev_id, cmd->fd_period);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_ENABLE_FILS_CMDID)) {
		wmi_err("Sending FILS cmd failed, vdev_id: %d", param->vdev_id);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

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
		wmi_err("Failed to allocate wmi buffer");
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
		wmi_err("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}
#endif

#ifdef FEATURE_MEC_OFFLOAD
static QDF_STATUS
send_pdev_set_mec_timer_cmd_tlv(struct wmi_unified *wmi_handle,
				struct set_mec_timer_params *param)
{
	wmi_pdev_mec_aging_timer_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_mec_aging_timer_config_cmd_fixed_param *)
		wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_mec_aging_timer_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_mec_aging_timer_config_cmd_fixed_param));
	cmd->pdev_id = param->pdev_id;
	cmd->mec_aging_timer_threshold = param->mec_aging_timer_threshold;

	wmi_mtrace(WMI_PDEV_MEC_AGING_TIMER_CONFIG_CMDID, param->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_MEC_AGING_TIMER_CONFIG_CMDID)) {
		wmi_err("Failed to set mec aging timer param");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WIFI_POS_CONVERGED
/**
 * extract_oem_response_param_tlv() - Extract oem response params
 * @wmi_handle: wmi handle
 * @resp_buf: response buffer
 * @oem_resp_param: pointer to hold oem response params
 *
 * Return: QDF_STATUS_SUCCESS on success or proper error code.
 */
static QDF_STATUS
extract_oem_response_param_tlv(wmi_unified_t wmi_handle, void *resp_buf,
			       struct wmi_oem_response_param *oem_resp_param)
{
	uint64_t temp_addr;
	WMI_OEM_RESPONSE_EVENTID_param_tlvs *param_buf =
		(WMI_OEM_RESPONSE_EVENTID_param_tlvs *)resp_buf;

	if (!param_buf) {
		wmi_err("Invalid OEM response");
		return QDF_STATUS_E_INVAL;
	}

	if (param_buf->num_data) {
		oem_resp_param->num_data1 = param_buf->num_data;
		oem_resp_param->data_1    = param_buf->data;
	}

	if (param_buf->num_data2) {
		oem_resp_param->num_data2 = param_buf->num_data2;
		oem_resp_param->data_2    = param_buf->data2;
	}

	if (param_buf->indirect_data) {
		oem_resp_param->indirect_data.pdev_id =
			param_buf->indirect_data->pdev_id;
		temp_addr = (param_buf->indirect_data->addr_hi) & 0xf;
		oem_resp_param->indirect_data.addr =
			param_buf->indirect_data->addr_lo +
			((uint64_t)temp_addr << 32);
		oem_resp_param->indirect_data.len =
			param_buf->indirect_data->len;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WIFI_POS_CONVERGED */

/**
 * extract_hw_mode_resp_event_status_tlv() - Extract HW mode change status
 * @wmi_handle: wmi handle
 * @event_buf: pointer to event buffer
 * @cmd_status: status of HW mode change command
 *
 * Return QDF_STATUS_SUCCESS on success or proper error code.
 */
static QDF_STATUS
extract_hw_mode_resp_event_status_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				      uint32_t *cmd_status)
{
	WMI_PDEV_SET_HW_MODE_RESP_EVENTID_param_tlvs *param_buf;
	wmi_pdev_set_hw_mode_response_event_fixed_param *fixed_param;

	param_buf = (WMI_PDEV_SET_HW_MODE_RESP_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid mode change event buffer");
		return QDF_STATUS_E_INVAL;
	}

	fixed_param = param_buf->fixed_param;
	if (!fixed_param) {
		wmi_err("Invalid fixed param");
		return QDF_STATUS_E_INVAL;
	}

	*cmd_status = fixed_param->status;
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_ANI_LEVEL_REQUEST
static QDF_STATUS send_ani_level_cmd_tlv(wmi_unified_t wmi_handle,
					 uint32_t *freqs,
					 uint8_t num_freqs)
{
	wmi_buf_t buf;
	wmi_get_channel_ani_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;
	A_UINT32 *chan_list;
	uint8_t i, *buf_ptr;

	len = sizeof(wmi_get_channel_ani_cmd_fixed_param) +
	      WMI_TLV_HDR_SIZE +
	      num_freqs * sizeof(A_UINT32);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_get_channel_ani_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_get_channel_ani_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_get_channel_ani_cmd_fixed_param));

	buf_ptr += sizeof(wmi_get_channel_ani_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (num_freqs * sizeof(A_UINT32)));

	chan_list = (A_UINT32 *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < num_freqs; i++) {
		chan_list[i] = freqs[i];
		wmi_debug("Requesting ANI for channel[%d]", chan_list[i]);
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_GET_CHANNEL_ANI_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("WMI_GET_CHANNEL_ANI_CMDID send error %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

static QDF_STATUS extract_ani_level_tlv(uint8_t *evt_buf,
					struct wmi_host_ani_level_event **info,
					uint32_t *num_freqs)
{
	WMI_GET_CHANNEL_ANI_EVENTID_param_tlvs *param_buf;
	wmi_get_channel_ani_event_fixed_param *fixed_param;
	wmi_channel_ani_info_tlv_param *tlv_params;
	uint8_t *buf_ptr, i;

	param_buf = (WMI_GET_CHANNEL_ANI_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid ani level event buffer");
		return QDF_STATUS_E_INVAL;
	}

	fixed_param =
		(wmi_get_channel_ani_event_fixed_param *)param_buf->fixed_param;
	if (!fixed_param) {
		wmi_err("Invalid fixed param");
		return QDF_STATUS_E_INVAL;
	}

	buf_ptr = (uint8_t *)fixed_param;
	buf_ptr += sizeof(wmi_get_channel_ani_event_fixed_param);
	buf_ptr += WMI_TLV_HDR_SIZE;

	*num_freqs = param_buf->num_ani_info;
	if (*num_freqs > MAX_NUM_FREQS_FOR_ANI_LEVEL) {
		wmi_err("Invalid number of freqs received");
		return QDF_STATUS_E_INVAL;
	}

	*info = qdf_mem_malloc(*num_freqs *
				   sizeof(struct wmi_host_ani_level_event));
	if (!(*info))
		return QDF_STATUS_E_NOMEM;

	tlv_params = (wmi_channel_ani_info_tlv_param *)buf_ptr;
	for (i = 0; i < param_buf->num_ani_info; i++) {
		(*info)[i].ani_level = tlv_params->ani_level;
		(*info)[i].chan_freq = tlv_params->chan_freq;
		tlv_params++;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_ANI_LEVEL_REQUEST */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * convert_wtc_scan_mode() - Function to convert TLV specific
 * ROAM_TRIGGER_SCAN_MODE scan mode to unified Roam trigger scan mode enum
 * @scan_mode: scan freq scheme coming from firmware
 *
 * Return: ROAM_TRIGGER_SCAN_MODE
 */
static enum roam_scan_freq_scheme
convert_wtc_scan_mode(WMI_ROAM_TRIGGER_SCAN_MODE scan_mode)
{
	switch (scan_mode) {
	case ROAM_TRIGGER_SCAN_MODE_NO_SCAN_DISCONNECTION:
		return ROAM_SCAN_FREQ_SCHEME_NO_SCAN;
	case ROAM_TRIGGER_SCAN_MODE_PARTIAL:
		return ROAM_SCAN_FREQ_SCHEME_PARTIAL_SCAN;
	case ROAM_TRIGGER_SCAN_MODE_FULL:
		return ROAM_SCAN_FREQ_SCHEME_FULL_SCAN;
	default:
		return ROAM_SCAN_FREQ_SCHEME_NONE;
	}
}

static uint32_t wmi_convert_fw_to_cm_trig_reason(uint32_t fw_trig_reason)
{
	switch (fw_trig_reason) {
	case WMI_ROAM_TRIGGER_REASON_NONE:
		return ROAM_TRIGGER_REASON_NONE;
	case WMI_ROAM_TRIGGER_REASON_PER:
		return ROAM_TRIGGER_REASON_PER;
	case WMI_ROAM_TRIGGER_REASON_BMISS:
		return ROAM_TRIGGER_REASON_BMISS;
	case WMI_ROAM_TRIGGER_REASON_LOW_RSSI:
		return ROAM_TRIGGER_REASON_LOW_RSSI;
	case WMI_ROAM_TRIGGER_REASON_HIGH_RSSI:
		return ROAM_TRIGGER_REASON_HIGH_RSSI;
	case WMI_ROAM_TRIGGER_REASON_PERIODIC:
		return ROAM_TRIGGER_REASON_PERIODIC;
	case WMI_ROAM_TRIGGER_REASON_MAWC:
		return ROAM_TRIGGER_REASON_MAWC;
	case WMI_ROAM_TRIGGER_REASON_DENSE:
		return ROAM_TRIGGER_REASON_DENSE;
	case WMI_ROAM_TRIGGER_REASON_BACKGROUND:
		return ROAM_TRIGGER_REASON_BACKGROUND;
	case WMI_ROAM_TRIGGER_REASON_FORCED:
		return ROAM_TRIGGER_REASON_FORCED;
	case WMI_ROAM_TRIGGER_REASON_BTM:
		return ROAM_TRIGGER_REASON_BTM;
	case WMI_ROAM_TRIGGER_REASON_UNIT_TEST:
		return ROAM_TRIGGER_REASON_UNIT_TEST;
	case WMI_ROAM_TRIGGER_REASON_BSS_LOAD:
		return ROAM_TRIGGER_REASON_BSS_LOAD;
	case WMI_ROAM_TRIGGER_REASON_DEAUTH:
		return ROAM_TRIGGER_REASON_DEAUTH;
	case WMI_ROAM_TRIGGER_REASON_IDLE:
		return ROAM_TRIGGER_REASON_IDLE;
	case WMI_ROAM_TRIGGER_REASON_STA_KICKOUT:
		return ROAM_TRIGGER_REASON_STA_KICKOUT;
	case WMI_ROAM_TRIGGER_REASON_ESS_RSSI:
		return ROAM_TRIGGER_REASON_ESS_RSSI;
	case WMI_ROAM_TRIGGER_REASON_WTC_BTM:
		return ROAM_TRIGGER_REASON_WTC_BTM;
	case WMI_ROAM_TRIGGER_REASON_PMK_TIMEOUT:
		return ROAM_TRIGGER_REASON_PMK_TIMEOUT;
	case WMI_ROAM_TRIGGER_REASON_BTC:
		return ROAM_TRIGGER_REASON_BTC;
	case WMI_ROAM_TRIGGER_EXT_REASON_MAX:
		return ROAM_TRIGGER_REASON_MAX;
	default:
		return ROAM_TRIGGER_REASON_NONE;
	}
}

/**
 * extract_roam_11kv_candidate_info  - Extract btm candidate info
 * @wmi_handle: wmi_handle
 * @evt_buf: Event buffer
 * @dst_info: Destination buffer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
extract_roam_11kv_candidate_info(wmi_unified_t wmi_handle, void *evt_buf,
				 struct wmi_btm_req_candidate_info *dst_info,
				 uint8_t btm_idx, uint16_t num_cand)
{
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_btm_request_candidate_info *src_data;
	uint8_t i;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf || !param_buf->roam_btm_request_candidate_info ||
	    !param_buf->num_roam_btm_request_candidate_info ||
	    (btm_idx +
	     num_cand) > param_buf->num_roam_btm_request_candidate_info)
		return QDF_STATUS_SUCCESS;

	src_data = &param_buf->roam_btm_request_candidate_info[btm_idx];
	if (num_cand > WLAN_MAX_BTM_CANDIDATE)
		num_cand = WLAN_MAX_BTM_CANDIDATE;
	for (i = 0; i < num_cand; i++) {
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&src_data->btm_candidate_bssid,
					   dst_info->candidate_bssid.bytes);
		dst_info->preference = src_data->preference;
		src_data++;
		dst_info++;
	}

	return QDF_STATUS_SUCCESS;
}

static enum roam_trigger_sub_reason
wmi_convert_roam_sub_reason(WMI_ROAM_TRIGGER_SUB_REASON_ID subreason)
{
	switch (subreason) {
	case WMI_ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER:
		return ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER;
	case WMI_ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER:
		return ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER_LOW_RSSI;
	case WMI_ROAM_TRIGGER_SUB_REASON_BTM_DI_TIMER:
		return ROAM_TRIGGER_SUB_REASON_BTM_DI_TIMER;
	case WMI_ROAM_TRIGGER_SUB_REASON_FULL_SCAN:
		return ROAM_TRIGGER_SUB_REASON_FULL_SCAN;
	case WMI_ROAM_TRIGGER_SUB_REASON_LOW_RSSI_PERIODIC:
		return ROAM_TRIGGER_SUB_REASON_LOW_RSSI_PERIODIC;
	case WMI_ROAM_TRIGGER_SUB_REASON_CU_PERIODIC:
		return ROAM_TRIGGER_SUB_REASON_CU_PERIODIC;
	case WMI_ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY:
		return ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY;
	case WMI_ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY_CU:
		return ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY_CU;
	case WMI_ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER_CU:
		return ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER_CU;
	default:
		break;
	}

	return 0;
}

/**
 * extract_roam_trigger_stats_tlv() - Extract the Roam trigger stats
 * from the WMI_ROAM_STATS_EVENTID
 * @wmi_handle: wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @trig:       Pointer to destination structure to fill data
 * @idx:        TLV id
 */
static QDF_STATUS
extract_roam_trigger_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			       struct wmi_roam_trigger_info *trig, uint8_t idx,
			       uint8_t btm_idx)
{
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_trigger_reason *src_data = NULL;
	uint32_t trig_reason;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf || !param_buf->roam_trigger_reason)
		return QDF_STATUS_E_FAILURE;

	src_data = &param_buf->roam_trigger_reason[idx];

	trig->present = true;
	trig_reason = src_data->trigger_reason;
	trig->trigger_reason = wmi_convert_fw_to_cm_trig_reason(trig_reason);
	trig->trigger_sub_reason =
		wmi_convert_roam_sub_reason(src_data->trigger_sub_reason);
	trig->current_rssi = src_data->current_rssi;
	trig->timestamp = src_data->timestamp;

	switch (trig_reason) {
	case WMI_ROAM_TRIGGER_REASON_PER:
	case WMI_ROAM_TRIGGER_REASON_BMISS:
	case WMI_ROAM_TRIGGER_REASON_HIGH_RSSI:
	case WMI_ROAM_TRIGGER_REASON_MAWC:
	case WMI_ROAM_TRIGGER_REASON_DENSE:
	case WMI_ROAM_TRIGGER_REASON_BACKGROUND:
	case WMI_ROAM_TRIGGER_REASON_IDLE:
	case WMI_ROAM_TRIGGER_REASON_FORCED:
	case WMI_ROAM_TRIGGER_REASON_UNIT_TEST:
	case WMI_ROAM_TRIGGER_REASON_BTC:
		return QDF_STATUS_SUCCESS;

	case WMI_ROAM_TRIGGER_REASON_BTM:
		trig->btm_trig_data.btm_request_mode =
				src_data->btm_request_mode;
		trig->btm_trig_data.disassoc_timer =
				src_data->disassoc_imminent_timer;
		trig->btm_trig_data.validity_interval =
				src_data->validity_internal;
		trig->btm_trig_data.candidate_list_count =
				src_data->candidate_list_count;
		trig->btm_trig_data.btm_resp_status =
				src_data->btm_response_status_code;
		trig->btm_trig_data.btm_bss_termination_timeout =
				src_data->btm_bss_termination_timeout;
		trig->btm_trig_data.btm_mbo_assoc_retry_timeout =
				src_data->btm_mbo_assoc_retry_timeout;
		trig->btm_trig_data.token = src_data->btm_req_dialog_token;
		if ((btm_idx + trig->btm_trig_data.candidate_list_count) <=
		    param_buf->num_roam_btm_request_candidate_info)
			extract_roam_11kv_candidate_info(
					wmi_handle, evt_buf,
					trig->btm_trig_data.btm_cand,
					btm_idx,
					src_data->candidate_list_count);

		return QDF_STATUS_SUCCESS;

	case WMI_ROAM_TRIGGER_REASON_BSS_LOAD:
		trig->cu_trig_data.cu_load = src_data->cu_load;
		return QDF_STATUS_SUCCESS;

	case WMI_ROAM_TRIGGER_REASON_DEAUTH:
		trig->deauth_trig_data.type = src_data->deauth_type;
		trig->deauth_trig_data.reason = src_data->deauth_reason;
		return QDF_STATUS_SUCCESS;

	case WMI_ROAM_TRIGGER_REASON_PERIODIC:
	case WMI_ROAM_TRIGGER_REASON_LOW_RSSI:
		trig->rssi_trig_data.threshold = src_data->roam_rssi_threshold;
		return QDF_STATUS_SUCCESS;

	case WMI_ROAM_TRIGGER_REASON_WTC_BTM:
		trig->wtc_btm_trig_data.roaming_mode =
					src_data->vendor_specific1[0];
		trig->wtc_btm_trig_data.vsie_trigger_reason =
					src_data->vendor_specific1[1];
		trig->wtc_btm_trig_data.sub_code =
					src_data->vendor_specific1[2];
		trig->wtc_btm_trig_data.wtc_mode =
					src_data->vendor_specific1[3];
		trig->wtc_btm_trig_data.wtc_scan_mode =
			convert_wtc_scan_mode(src_data->vendor_specific1[4]);
		trig->wtc_btm_trig_data.wtc_rssi_th =
					src_data->vendor_specific1[5];
		trig->wtc_btm_trig_data.wtc_candi_rssi_th =
					src_data->vendor_specific1[6];

		trig->wtc_btm_trig_data.wtc_candi_rssi_ext_present =
					src_data->vendor_specific2[0];
		trig->wtc_btm_trig_data.wtc_candi_rssi_th_5g =
					src_data->vendor_specific2[1];
		trig->wtc_btm_trig_data.wtc_candi_rssi_th_6g =
					src_data->vendor_specific2[2];
		trig->wtc_btm_trig_data.duration =
					src_data->vendor_specific2[3];

		return QDF_STATUS_SUCCESS;
	default:
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_roam_scan_ap_stats_tlv() - Extract the Roam trigger stats
 * from the WMI_ROAM_STATS_EVENTID
 * @wmi_handle: wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @ap_idx:     TLV index for this roam scan
 * @num_cand:   number of candidates list in the roam scan
 */
static QDF_STATUS
extract_roam_scan_ap_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			       struct wmi_roam_candidate_info *dst,
			       uint8_t ap_idx, uint16_t num_cand)
{
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_ap_info *src = NULL;
	uint8_t i;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Param buf is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (ap_idx >= param_buf->num_roam_ap_info) {
		wmi_err("Invalid roam scan AP tlv ap_idx:%d total_ap:%d",
			ap_idx, param_buf->num_roam_ap_info);
		return QDF_STATUS_E_FAILURE;
	}

	src = &param_buf->roam_ap_info[ap_idx];

	for (i = 0; i < num_cand; i++) {
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&src->bssid, dst->bssid.bytes);
		dst->type = src->candidate_type;
		dst->freq = src->channel;
		dst->etp = src->etp;
		dst->rssi = src->rssi;
		dst->rssi_score = src->rssi_score;
		dst->cu_load = src->cu_load;
		dst->cu_score = src->cu_score;
		dst->total_score = src->total_score;
		dst->timestamp = src->timestamp;
		dst->bl_reason = src->bl_reason;
		dst->bl_source = src->bl_source;
		dst->bl_timestamp = src->bl_timestamp;
		dst->bl_original_timeout = src->bl_original_timeout;

		src++;
		dst++;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_roam_scan_stats_tlv() - Extract the Roam trigger stats
 * from the WMI_ROAM_STATS_EVENTID
 * @wmi_handle: wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @idx:        TLV id
 * @chan_idx:   Index of the channel tlv for the current roam trigger
 * @ap_idx:     Index of the candidate AP TLV for the current roam trigger
 */
static QDF_STATUS
extract_roam_scan_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			    struct wmi_roam_scan_data *dst, uint8_t idx,
			    uint8_t chan_idx, uint8_t ap_idx)
{
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_scan_info *src_data = NULL;
	wmi_roam_scan_channel_info *src_chan = NULL;
	QDF_STATUS status;
	uint8_t i;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf || !param_buf->roam_scan_info ||
	    idx >= param_buf->num_roam_scan_info)
		return QDF_STATUS_E_FAILURE;

	src_data = &param_buf->roam_scan_info[idx];

	dst->present = true;
	dst->type = src_data->roam_scan_type;
	dst->num_chan = src_data->roam_scan_channel_count;
	dst->next_rssi_threshold = src_data->next_rssi_trigger_threshold;
	dst->frame_info_count = src_data->frame_info_count;
	if (dst->frame_info_count >  WLAN_ROAM_MAX_FRAME_INFO)
		dst->frame_info_count =  WLAN_ROAM_MAX_FRAME_INFO;

	/* Read the channel data only for dst->type is 0 (partial scan) */
	if (dst->num_chan && !dst->type && param_buf->num_roam_scan_chan_info &&
	    chan_idx < param_buf->num_roam_scan_chan_info) {
		if (dst->num_chan > MAX_ROAM_SCAN_CHAN)
			dst->num_chan = MAX_ROAM_SCAN_CHAN;

		src_chan = &param_buf->roam_scan_chan_info[chan_idx];

		if ((dst->num_chan + chan_idx) >
		    param_buf->num_roam_scan_chan_info) {
			wmi_err("Invalid TLV. num_chan %d chan_idx %d num_roam_scan_chan_info %d",
				dst->num_chan, chan_idx,
				param_buf->num_roam_scan_chan_info);
			return QDF_STATUS_SUCCESS;
		}

		for (i = 0; i < dst->num_chan; i++) {
			dst->chan_freq[i] = src_chan->channel;
			src_chan++;
		}
	}

	if (!src_data->roam_ap_count || !param_buf->num_roam_ap_info)
		return QDF_STATUS_SUCCESS;

	dst->num_ap = src_data->roam_ap_count;
	if (dst->num_ap > MAX_ROAM_CANDIDATE_AP)
		dst->num_ap = MAX_ROAM_CANDIDATE_AP;

	status = extract_roam_scan_ap_stats_tlv(wmi_handle, evt_buf, dst->ap,
						ap_idx, dst->num_ap);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Extract candidate stats for tlv[%d] failed", idx);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_roam_fail_reason_code() - Convert FW enum to Host enum
 * @wmi_roam_fail_reason: roam fail enum
 *
 * Return: Roaming failure reason codes
 */
static enum wlan_roam_failure_reason_code
wlan_roam_fail_reason_code(uint16_t wmi_roam_fail_reason)
{
	switch (wmi_roam_fail_reason) {
	case WMI_ROAM_FAIL_REASON_NO_SCAN_START:
		return ROAM_FAIL_REASON_NO_SCAN_START;
	case WMI_ROAM_FAIL_REASON_NO_AP_FOUND:
		return ROAM_FAIL_REASON_NO_AP_FOUND;
	case WMI_ROAM_FAIL_REASON_NO_CAND_AP_FOUND:
		return ROAM_FAIL_REASON_NO_CAND_AP_FOUND;
	case WMI_ROAM_FAIL_REASON_HOST:
		return ROAM_FAIL_REASON_HOST;
	case WMI_ROAM_FAIL_REASON_AUTH_SEND:
		return ROAM_FAIL_REASON_AUTH_SEND;
	case WMI_ROAM_FAIL_REASON_AUTH_RECV:
		return ROAM_FAIL_REASON_AUTH_RECV;
	case WMI_ROAM_FAIL_REASON_NO_AUTH_RESP:
		return ROAM_FAIL_REASON_NO_AUTH_RESP;
	case WMI_ROAM_FAIL_REASON_REASSOC_SEND:
		return ROAM_FAIL_REASON_REASSOC_SEND;
	case WMI_ROAM_FAIL_REASON_REASSOC_RECV:
		return ROAM_FAIL_REASON_REASSOC_RECV;
	case WMI_ROAM_FAIL_REASON_NO_REASSOC_RESP:
		return ROAM_FAIL_REASON_NO_REASSOC_RESP;
	case WMI_ROAM_FAIL_REASON_EAPOL_TIMEOUT:
		return ROAM_FAIL_REASON_EAPOL_TIMEOUT;
	case WMI_ROAM_FAIL_REASON_MLME:
		return ROAM_FAIL_REASON_MLME;
	case WMI_ROAM_FAIL_REASON_INTERNAL_ABORT:
		return ROAM_FAIL_REASON_INTERNAL_ABORT;
	case WMI_ROAM_FAIL_REASON_SCAN_START:
		return ROAM_FAIL_REASON_SCAN_START;
	case WMI_ROAM_FAIL_REASON_AUTH_NO_ACK:
		return ROAM_FAIL_REASON_AUTH_NO_ACK;
	case WMI_ROAM_FAIL_REASON_AUTH_INTERNAL_DROP:
		return ROAM_FAIL_REASON_AUTH_INTERNAL_DROP;
	case WMI_ROAM_FAIL_REASON_REASSOC_NO_ACK:
		return ROAM_FAIL_REASON_REASSOC_NO_ACK;
	case WMI_ROAM_FAIL_REASON_REASSOC_INTERNAL_DROP:
		return ROAM_FAIL_REASON_REASSOC_INTERNAL_DROP;
	case WMI_ROAM_FAIL_REASON_EAPOL_M2_SEND:
		return ROAM_FAIL_REASON_EAPOL_M2_SEND;
	case WMI_ROAM_FAIL_REASON_EAPOL_M2_INTERNAL_DROP:
		return ROAM_FAIL_REASON_EAPOL_M2_INTERNAL_DROP;
	case WMI_ROAM_FAIL_REASON_EAPOL_M2_NO_ACK:
		return ROAM_FAIL_REASON_EAPOL_M2_NO_ACK;
	case WMI_ROAM_FAIL_REASON_EAPOL_M3_TIMEOUT:
		return ROAM_FAIL_REASON_EAPOL_M3_TIMEOUT;
	case WMI_ROAM_FAIL_REASON_EAPOL_M4_SEND:
		return ROAM_FAIL_REASON_EAPOL_M4_SEND;
	case WMI_ROAM_FAIL_REASON_EAPOL_M4_INTERNAL_DROP:
		return ROAM_FAIL_REASON_EAPOL_M4_INTERNAL_DROP;
	case WMI_ROAM_FAIL_REASON_EAPOL_M4_NO_ACK:
		return ROAM_FAIL_REASON_EAPOL_M4_NO_ACK;
	case WMI_ROAM_FAIL_REASON_NO_SCAN_FOR_FINAL_BMISS:
		return ROAM_FAIL_REASON_NO_SCAN_FOR_FINAL_BMISS;
	case WMI_ROAM_FAIL_REASON_DISCONNECT:
		return ROAM_FAIL_REASON_DISCONNECT;
	case WMI_ROAM_FAIL_REASON_SYNC:
		return ROAM_FAIL_REASON_SYNC;
	case WMI_ROAM_FAIL_REASON_SAE_INVALID_PMKID:
		return ROAM_FAIL_REASON_SAE_INVALID_PMKID;
	case WMI_ROAM_FAIL_REASON_SAE_PREAUTH_TIMEOUT:
		return ROAM_FAIL_REASON_SAE_PREAUTH_TIMEOUT;
	case WMI_ROAM_FAIL_REASON_SAE_PREAUTH_FAIL:
		return ROAM_FAIL_REASON_SAE_PREAUTH_FAIL;
	case WMI_ROAM_FAIL_REASON_UNABLE_TO_START_ROAM_HO:
		return ROAM_FAIL_REASON_UNABLE_TO_START_ROAM_HO;
	default:
		return ROAM_FAIL_REASON_UNKNOWN;
	}
}

#define ROAM_SUCCESS 0

/**
 * extract_roam_scan_stats_tlv() - Extract the Roam trigger stats
 * from the WMI_ROAM_STATS_EVENTID
 * @wmi_handle: wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @idx:        TLV id
 */
static QDF_STATUS
extract_roam_result_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			      struct wmi_roam_result *dst, uint8_t idx)
{
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_result *src_data = NULL;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf || !param_buf->roam_result ||
	    idx >= param_buf->num_roam_result)
		return QDF_STATUS_E_FAILURE;

	src_data = &param_buf->roam_result[idx];

	dst->present = true;
	dst->status = src_data->roam_status;
	dst->timestamp = src_data->timestamp;
	if (src_data->roam_fail_reason != ROAM_SUCCESS)
		dst->fail_reason =
			wlan_roam_fail_reason_code(src_data->roam_fail_reason);
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&src_data->bssid, dst->fail_bssid.bytes);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_roam_11kv_stats_tlv() - Extract the Roam trigger stats
 * from the WMI_ROAM_STATS_EVENTID
 * @wmi_handle: wmi handle
 * @evt_buf:    Pointer to the event buffer
 * @dst:        Pointer to destination structure to fill data
 * @idx:        TLV id
 * @rpt_idx:    Neighbor report Channel index
 */
static QDF_STATUS
extract_roam_11kv_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			    struct wmi_neighbor_report_data *dst,
			    uint8_t idx, uint8_t rpt_idx)
{
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_neighbor_report_info *src_data = NULL;
	wmi_roam_neighbor_report_channel_info *src_freq = NULL;
	uint8_t i;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf || !param_buf->roam_neighbor_report_info ||
	    !param_buf->num_roam_neighbor_report_info ||
	    idx >= param_buf->num_roam_neighbor_report_info) {
		wmi_debug("Invalid 1kv param buf");
		return QDF_STATUS_E_FAILURE;
	}

	src_data = &param_buf->roam_neighbor_report_info[idx];

	dst->present = true;
	dst->req_type = src_data->request_type;
	dst->num_freq = src_data->neighbor_report_channel_count;
	dst->req_time = src_data->neighbor_report_request_timestamp;
	dst->resp_time = src_data->neighbor_report_response_timestamp;
	dst->btm_query_token = src_data->btm_query_token;
	dst->btm_query_reason = src_data->btm_query_reason_code;

	if (!dst->num_freq || !param_buf->num_roam_neighbor_report_chan_info ||
	    rpt_idx >= param_buf->num_roam_neighbor_report_chan_info)
		return QDF_STATUS_SUCCESS;

	if (!param_buf->roam_neighbor_report_chan_info) {
		wmi_debug("11kv channel present, but TLV is NULL num_freq:%d",
			 dst->num_freq);
		dst->num_freq = 0;
		/* return success as its optional tlv and we can print neighbor
		 * report received info
		 */
		return QDF_STATUS_SUCCESS;
	}

	src_freq = &param_buf->roam_neighbor_report_chan_info[rpt_idx];

	if (dst->num_freq > MAX_ROAM_SCAN_CHAN)
		dst->num_freq = MAX_ROAM_SCAN_CHAN;

	if ((dst->num_freq + rpt_idx) >
	    param_buf->num_roam_neighbor_report_chan_info) {
		wmi_err("Invalid TLV. num_freq %d rpt_idx %d num_roam_neighbor_report_chan_info %d",
			dst->num_freq, rpt_idx,
			param_buf->num_roam_scan_chan_info);
		return QDF_STATUS_SUCCESS;
	}

	for (i = 0; i < dst->num_freq; i++) {
		dst->freq[i] = src_freq->channel;
		src_freq++;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_roam_set_param_cmd_tlv() - WMI roam set parameter function
 *  @wmi_handle      : handle to WMI.
 *  @roam_param    : pointer to hold roam set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS
send_roam_set_param_cmd_tlv(wmi_unified_t wmi_handle,
			    struct vdev_set_params *roam_param)
{
	QDF_STATUS ret;
	wmi_roam_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_roam_set_param_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_set_param_cmd_fixed_param));
	cmd->vdev_id = roam_param->vdev_id;
	cmd->param_id = roam_param->param_id;
	cmd->param_value = roam_param->param_value;
	wmi_debug("Setting vdev %d roam_param = %x, value = %u",
		  cmd->vdev_id, cmd->param_id, cmd->param_value);
	wmi_mtrace(WMI_ROAM_SET_PARAM_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_ROAM_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send roam set param command, ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}
#else
static inline QDF_STATUS
extract_roam_trigger_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			       struct wmi_roam_trigger_info *trig, uint8_t idx,
			       uint8_t btm_idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
extract_roam_result_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			      struct wmi_roam_result *dst, uint8_t idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static QDF_STATUS
extract_roam_11kv_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			    struct wmi_neighbor_report_data *dst,
			    uint8_t idx, uint8_t rpt_idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static QDF_STATUS
extract_roam_scan_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			    struct wmi_roam_scan_data *dst, uint8_t idx,
			    uint8_t chan_idx, uint8_t ap_idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE
static QDF_STATUS
extract_vdev_mgmt_offload_event_tlv(void *handle, void *evt_buf,
				    struct mgmt_offload_event_params *params)
{
	WMI_VDEV_MGMT_OFFLOAD_EVENTID_param_tlvs *param_tlvs;
	wmi_mgmt_hdr *hdr;

	param_tlvs = (WMI_VDEV_MGMT_OFFLOAD_EVENTID_param_tlvs *)evt_buf;
	if (!param_tlvs)
		return QDF_STATUS_E_INVAL;

	hdr = param_tlvs->fixed_param;
	if (!hdr)
		return QDF_STATUS_E_INVAL;

	if (hdr->buf_len > param_tlvs->num_bufp)
		return QDF_STATUS_E_INVAL;

	params->tsf_l32 = hdr->tsf_l32;
	params->chan_freq = hdr->chan_freq;
	params->rate_kbps = hdr->rate_kbps;
	params->rssi = hdr->rssi;
	params->buf_len = hdr->buf_len;
	params->tx_status = hdr->tx_status;
	params->buf = param_tlvs->bufp;
	params->tx_retry_cnt = hdr->tx_retry_cnt;
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_PKT_CAPTURE */

#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
static QDF_STATUS
extract_smart_monitor_event_tlv(void *handle, void *evt_buf,
				struct smu_event_params *params)
{
	WMI_VDEV_SMART_MONITOR_EVENTID_param_tlvs *param_buf = NULL;
	wmi_vdev_smart_monitor_event_fixed_param *smu_event = NULL;

	param_buf = (WMI_VDEV_SMART_MONITOR_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid smart monitor event");
		return QDF_STATUS_E_INVAL;
	}

	smu_event = param_buf->fixed_param;
	if (!smu_event) {
		wmi_err("smart monitor event fixed param is NULL");
		return QDF_STATUS_E_INVAL;
	}

	params->vdev_id = smu_event->vdev_id;
	if (params->vdev_id >= WLAN_UMAC_PDEV_MAX_VDEVS)
		return QDF_STATUS_E_INVAL;

	params->rx_vht_sgi = smu_event->rx_vht_sgi;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_PKT_CAPTURE_V2 */

#ifdef FEATURE_WLAN_TIME_SYNC_FTM
/**
 * send_wlan_ts_ftm_trigger_cmd_tlv(): send wlan time sync cmd to FW
 *
 * @wmi: wmi handle
 * @vdev_id: vdev id
 * @burst_mode: Indicates whether relation derived using FTM is needed for
 *		each FTM frame or only aggregated result is required.
 *
 * Send WMI_AUDIO_SYNC_TRIGGER_CMDID to FW.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_wlan_ts_ftm_trigger_cmd_tlv(wmi_unified_t wmi,
						   uint32_t vdev_id,
						   bool burst_mode)
{
	wmi_audio_sync_trigger_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_audio_sync_trigger_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_audio_sync_trigger_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_audio_sync_trigger_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->agg_relation = burst_mode ? false : true;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_AUDIO_SYNC_TRIGGER_CMDID)) {
		wmi_err("Failed to send audio sync trigger cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS send_wlan_ts_qtime_cmd_tlv(wmi_unified_t wmi,
					     uint32_t vdev_id,
					     uint64_t lpass_ts)
{
	wmi_audio_sync_qtimer_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_audio_sync_qtimer_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_audio_sync_qtimer_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_audio_sync_qtimer_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->qtimer_u32 = (uint32_t)((lpass_ts & 0xffffffff00000000LL) >> 32);
	cmd->qtimer_l32 = (uint32_t)(lpass_ts & 0xffffffffLL);

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_AUDIO_SYNC_QTIMER_CMDID)) {
		wmi_err("Failed to send audio qtime command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_time_sync_ftm_start_stop_event_tlv(
				wmi_unified_t wmi, void *buf,
				struct ftm_time_sync_start_stop_params *param)
{
	WMI_VDEV_AUDIO_SYNC_START_STOP_EVENTID_param_tlvs *param_buf;
	wmi_audio_sync_start_stop_event_fixed_param *resp_event;

	param_buf = (WMI_VDEV_AUDIO_SYNC_START_STOP_EVENTID_param_tlvs *)buf;
	if (!param_buf) {
		wmi_err("Invalid audio sync start stop event buffer");
		return QDF_STATUS_E_FAILURE;
	}

	resp_event = param_buf->fixed_param;
	if (!resp_event) {
		wmi_err("Invalid audio sync start stop fixed param buffer");
		return QDF_STATUS_E_FAILURE;
	}

	param->vdev_id = resp_event->vdev_id;
	param->timer_interval = resp_event->periodicity;
	param->num_reads = resp_event->reads_needed;
	param->qtime = ((uint64_t)resp_event->qtimer_u32 << 32) |
			resp_event->qtimer_l32;
	param->mac_time = ((uint64_t)resp_event->mac_timer_u32 << 32) |
			   resp_event->mac_timer_l32;

	wmi_debug("FTM time sync time_interval %d, num_reads %d",
		 param->timer_interval, param->num_reads);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
extract_time_sync_ftm_offset_event_tlv(wmi_unified_t wmi, void *buf,
				       struct ftm_time_sync_offset *param)
{
	WMI_VDEV_AUDIO_SYNC_Q_MASTER_SLAVE_OFFSET_EVENTID_param_tlvs *param_buf;
	wmi_audio_sync_q_master_slave_offset_event_fixed_param *resp_event;
	wmi_audio_sync_q_master_slave_times *q_pair;
	int iter;

	param_buf =
	(WMI_VDEV_AUDIO_SYNC_Q_MASTER_SLAVE_OFFSET_EVENTID_param_tlvs *)buf;
	if (!param_buf) {
		wmi_err("Invalid timesync ftm offset event buffer");
		return QDF_STATUS_E_FAILURE;
	}

	resp_event = param_buf->fixed_param;
	if (!resp_event) {
		wmi_err("Invalid timesync ftm offset fixed param buffer");
		return QDF_STATUS_E_FAILURE;
	}

	param->vdev_id = resp_event->vdev_id;
	param->num_qtime = param_buf->num_audio_sync_q_master_slave_times;
	if (param->num_qtime > FTM_TIME_SYNC_QTIME_PAIR_MAX)
		param->num_qtime = FTM_TIME_SYNC_QTIME_PAIR_MAX;

	q_pair = param_buf->audio_sync_q_master_slave_times;
	if (!q_pair) {
		wmi_err("Invalid q_master_slave_times buffer");
		return QDF_STATUS_E_FAILURE;
	}

	for (iter = 0; iter < param->num_qtime; iter++) {
		param->pairs[iter].qtime_master = (
			(uint64_t)q_pair[iter].qmaster_u32 << 32) |
			 q_pair[iter].qmaster_l32;
		param->pairs[iter].qtime_slave = (
			(uint64_t)q_pair[iter].qslave_u32 << 32) |
			 q_pair[iter].qslave_l32;
	}
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_TIME_SYNC_FTM */

/**
 * send_vdev_tsf_tstamp_action_cmd_tlv() - send vdev tsf action command
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * TSF_TSTAMP_READ_VALUE is the only operation supported
 * Return: QDF_STATUS_SUCCESS for success or erro code
 */
static QDF_STATUS
send_vdev_tsf_tstamp_action_cmd_tlv(wmi_unified_t wmi, uint8_t vdev_id)
{
	wmi_vdev_tsf_tstamp_action_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_tsf_tstamp_action_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_tsf_tstamp_action_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_tsf_tstamp_action_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->tsf_action = TSF_TSTAMP_READ_VALUE;
	wmi_mtrace(WMI_VDEV_TSF_TSTAMP_ACTION_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len,
				 WMI_VDEV_TSF_TSTAMP_ACTION_CMDID)) {
		wmi_err("%s: Failed to send WMI_VDEV_TSF_TSTAMP_ACTION_CMDID",
			__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_tsf_report_event_tlv() - extract vdev tsf report from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @wmi_host_tsf_event param: Pointer to struct to hold event info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
extract_vdev_tsf_report_event_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				  struct wmi_host_tsf_event *param)
{
	WMI_VDEV_TSF_REPORT_EVENTID_param_tlvs *param_buf;
	wmi_vdev_tsf_report_event_fixed_param *evt;

	param_buf = (WMI_VDEV_TSF_REPORT_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid tsf report event buffer");
		return QDF_STATUS_E_INVAL;
	}

	evt = param_buf->fixed_param;
	param->tsf = ((uint64_t)(evt->tsf_high) << 32) | evt->tsf_low;
	param->vdev_id = evt->vdev_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_csa_switch_count_status_tlv() - extract pdev csa switch count
 *					      status tlv
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold csa switch count status event param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_csa_switch_count_status_tlv(
				wmi_unified_t wmi_handle,
				void *evt_buf,
				struct pdev_csa_switch_count_status *param)
{
	WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_csa_switch_count_status_event_fixed_param *csa_status;

	param_buf = (WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID_param_tlvs *)
		     evt_buf;
	if (!param_buf) {
		wmi_err("Invalid CSA status event");
		return QDF_STATUS_E_INVAL;
	}

	csa_status = param_buf->fixed_param;

	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							csa_status->pdev_id);
	param->current_switch_count = csa_status->current_switch_count;
	param->num_vdevs = csa_status->num_vdevs;
	param->vdev_ids = param_buf->vdev_ids;

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_AFC_SUPPORT
/**
 * send_afc_cmd_tlv() - Sends the AFC indication to FW
 * @wmi_handle: wmi handle
 * @pdev_id: Pdev id
 * @param: Pointer to hold AFC indication.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static
QDF_STATUS send_afc_cmd_tlv(wmi_unified_t wmi_handle,
			    uint8_t pdev_id,
			    struct reg_afc_resp_rx_ind_info *param)
{
	wmi_buf_t buf;
	wmi_afc_cmd_fixed_param *cmd;
	uint32_t len;
	uint8_t *buf_ptr;
	QDF_STATUS ret;

	len = sizeof(wmi_afc_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_afc_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_afc_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_afc_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								pdev_id);
	cmd->cmd_type = param->cmd_type;
	cmd->serv_resp_format = param->serv_resp_format;

	wmi_mtrace(WMI_AFC_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_AFC_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI_AFC_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * send_set_tpc_power_cmd_tlv() - Sends the set TPC power level to FW
 * @wmi_handle: wmi handle
 * @param: Pointer to hold TX power info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_tpc_power_cmd_tlv(wmi_unified_t wmi_handle,
					     uint8_t vdev_id,
					     struct reg_tpc_power_info *param)
{
	wmi_buf_t buf;
	wmi_vdev_set_tpc_power_fixed_param *tpc_power_info_param;
	wmi_vdev_ch_power_info *ch_power_info;
	uint8_t *buf_ptr;
	uint16_t idx;
	uint32_t len;
	QDF_STATUS ret;

	len = sizeof(wmi_vdev_set_tpc_power_fixed_param) + WMI_TLV_HDR_SIZE;
	len += (sizeof(wmi_vdev_ch_power_info) * param->num_pwr_levels);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	tpc_power_info_param = (wmi_vdev_set_tpc_power_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&tpc_power_info_param->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_set_tpc_power_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_set_tpc_power_fixed_param));

	tpc_power_info_param->vdev_id = vdev_id;
	tpc_power_info_param->psd_power = param->is_psd_power;
	tpc_power_info_param->eirp_power = param->eirp_power;
	tpc_power_info_param->power_type_6ghz = param->power_type_6g;

	buf_ptr += sizeof(wmi_vdev_set_tpc_power_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       param->num_pwr_levels * sizeof(wmi_vdev_ch_power_info));

	buf_ptr += WMI_TLV_HDR_SIZE;
	ch_power_info = (wmi_vdev_ch_power_info *)buf_ptr;

	for (idx = 0; idx < param->num_pwr_levels; ++idx) {
		WMITLV_SET_HDR(&ch_power_info[idx].tlv_header,
			WMITLV_TAG_STRUC_wmi_vdev_ch_power_info,
			WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_ch_power_info));
		ch_power_info[idx].chan_cfreq =
			param->chan_power_info[idx].chan_cfreq;
		ch_power_info[idx].tx_power =
			param->chan_power_info[idx].tx_power;
		buf_ptr += sizeof(wmi_vdev_ch_power_info);
	}

	wmi_mtrace(WMI_VDEV_SET_TPC_POWER_CMDID, vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_TPC_POWER_CMDID);
	if (QDF_IS_STATUS_ERROR(ret))
		wmi_buf_free(buf);


	return ret;
}

/**
 * extract_dpd_status_ev_param_tlv() - extract dpd status from FW event
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @param: dpd status info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_dpd_status_ev_param_tlv(wmi_unified_t wmi_handle,
				void *evt_buf,
				struct wmi_host_pdev_get_dpd_status_event *param)
{
	WMI_PDEV_GET_DPD_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_get_dpd_status_evt_fixed_param *dpd_status;

	param_buf = (WMI_PDEV_GET_DPD_STATUS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid get dpd_status event");
		return QDF_STATUS_E_INVAL;
	}

	dpd_status = param_buf->fixed_param;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host
		(wmi_handle, dpd_status->pdev_id);
	param->dpd_status = dpd_status->dpd_status;

	return QDF_STATUS_SUCCESS;
}

static int
convert_halphy_status(wmi_pdev_get_halphy_cal_status_evt_fixed_param *status,
		      WMI_HALPHY_CAL_VALID_BITMAP_STATUS valid_bit)
{
	if (status->halphy_cal_valid_bmap && valid_bit)
		return (status->halphy_cal_status && valid_bit);

	return 0;
}

static QDF_STATUS
extract_halphy_cal_status_ev_param_tlv(wmi_unified_t wmi_handle,
				       void *evt_buf,
				       struct wmi_host_pdev_get_halphy_cal_status_event *param)
{
	WMI_PDEV_GET_HALPHY_CAL_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_get_halphy_cal_status_evt_fixed_param *halphy_cal_status;

	param_buf = (WMI_PDEV_GET_HALPHY_CAL_STATUS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid get halphy cal status event");
		return QDF_STATUS_E_INVAL;
	}

	halphy_cal_status = param_buf->fixed_param;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host
		(wmi_handle, halphy_cal_status->pdev_id);
	param->halphy_cal_adc_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_ADC_BMAP);
	param->halphy_cal_bwfilter_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_BWFILTER_BMAP);
	param->halphy_cal_pdet_and_pal_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_PDET_AND_PAL_BMAP);
	param->halphy_cal_rxdco_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_RXDCO_BMAP);
	param->halphy_cal_comb_txiq_rxiq_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_COMB_TXLO_TXIQ_RXIQ_BMAP);
	param->halphy_cal_ibf_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_IBF_BMAP);
	param->halphy_cal_pa_droop_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_PA_DROOP_BMAP);
	param->halphy_cal_dac_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_DAC_BMAP);
	param->halphy_cal_ani_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_ANI_BMAP);
	param->halphy_cal_noise_floor_status =
		convert_halphy_status(halphy_cal_status,
				      WMI_HALPHY_CAL_NOISE_FLOOR_BMAP);

	return QDF_STATUS_SUCCESS;
}

/**
 * set_halphy_cal_fw_status_to_host_status() - Convert set halphy cal status to host enum
 * @fw_status: set halphy cal status from WMI_PDEV_SET_HALPHY_CAL_BMAP_EVENTID event
 *
 * Return: host_set_halphy_cal_status
 */
static enum wmi_host_set_halphy_cal_status
set_halphy_cal_fw_status_to_host_status(uint32_t fw_status)
{
	if (fw_status == 0)
		return WMI_HOST_SET_HALPHY_CAL_STATUS_SUCCESS;
	else if (fw_status == 1)
		return WMI_HOST_SET_HALPHY_CAL_STATUS_FAIL;

	wmi_debug("Unknown set halphy status code(%u) from WMI", fw_status);
	return WMI_HOST_SET_HALPHY_CAL_STATUS_FAIL;
}

/**
 * extract_halphy_cal_ev_param_tlv() - extract dpd status from FW event
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @param: set halphy cal status info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_halphy_cal_ev_param_tlv(wmi_unified_t wmi_handle,
				void *evt_buf,
				struct wmi_host_pdev_set_halphy_cal_event *param)
{
	WMI_PDEV_SET_HALPHY_CAL_BMAP_EVENTID_param_tlvs *param_buf;
	wmi_pdev_set_halphy_cal_bmap_evt_fixed_param *set_halphy_status;

	param_buf = (WMI_PDEV_SET_HALPHY_CAL_BMAP_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid set halphy_status event");
		return QDF_STATUS_E_INVAL;
	}

	set_halphy_status = param_buf->fixed_param;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host
		(wmi_handle, set_halphy_status->pdev_id);
	param->status = set_halphy_cal_fw_status_to_host_status(set_halphy_status->status);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_install_key_comp_event_tlv() - extract install key complete event tlv
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @len: length of the event buffer
 * @param: Pointer to hold install key complete event param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_install_key_comp_event_tlv(wmi_unified_t wmi_handle,
				   void *evt_buf, uint32_t len,
				   struct wmi_install_key_comp_event *param)
{
	WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_vdev_install_key_complete_event_fixed_param *key_fp;

	if (len < sizeof(*param_buf)) {
		wmi_err("invalid event buf len %d", len);
		return QDF_STATUS_E_INVAL;
	}

	param_buf = (WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("received null buf from target");
		return QDF_STATUS_E_INVAL;
	}

	key_fp = param_buf->fixed_param;
	if (!key_fp) {
		wmi_err("received null event data from target");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = key_fp->vdev_id;
	param->key_ix = key_fp->key_ix;
	param->key_flags = key_fp->key_flags;
	param->status = key_fp->status;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&key_fp->peer_macaddr,
				   param->peer_macaddr);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
send_set_halphy_cal_tlv(wmi_unified_t wmi_handle,
			struct wmi_host_send_set_halphy_cal_info *param)
{
	wmi_buf_t buf;
	wmi_pdev_set_halphy_cal_bmap_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (void *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_halphy_cal_bmap_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_pdev_set_halphy_cal_bmap_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(wmi_handle,
								       param->pdev_id);
	cmd->online_halphy_cals_bmap = param->value;
	cmd->home_scan_channel = param->chan_sel;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_HALPHY_CAL_BMAP_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("WMI_PDEV_SET_HALPHY_CAL_BMAP_CMDID send returned Error %d",ret);
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
/**
 * send_set_mac_address_cmd_tlv() - send set MAC address command to fw
 * @wmi: wmi handle
 * @params: set MAC address command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_set_mac_address_cmd_tlv(wmi_unified_t wmi,
			     struct set_mac_addr_params *params)
{
	wmi_vdev_update_mac_addr_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_update_mac_addr_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_update_mac_addr_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_update_mac_addr_cmd_fixed_param));
	cmd->vdev_id = params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->mac_addr.bytes, &cmd->vdev_macaddr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->mld_addr.bytes, &cmd->mld_macaddr);

	wmi_debug("vdev %d mac_addr " QDF_MAC_ADDR_FMT " mld_addr "
		  QDF_MAC_ADDR_FMT, cmd->vdev_id,
		  QDF_MAC_ADDR_REF(params->mac_addr.bytes),
		  QDF_MAC_ADDR_REF(params->mld_addr.bytes));
	wmi_mtrace(WMI_VDEV_UPDATE_MAC_ADDR_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi, buf, len,
				 WMI_VDEV_UPDATE_MAC_ADDR_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_update_mac_address_event_tlv() - extract update MAC address event
 * @wmi_handle: WMI handle
 * @evt_buf: event buffer
 * @vdev_id: VDEV ID
 * @status: FW status of the set MAC address operation
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_update_mac_address_event_tlv(
				wmi_unified_t wmi_handle, void *evt_buf,
				uint8_t *vdev_id, uint8_t *status)
{
	WMI_VDEV_UPDATE_MAC_ADDR_CONF_EVENTID_param_tlvs *param_buf;
	wmi_vdev_update_mac_addr_conf_event_fixed_param *event;

	param_buf =
		(WMI_VDEV_UPDATE_MAC_ADDR_CONF_EVENTID_param_tlvs *)evt_buf;

	event = param_buf->fixed_param;

	*vdev_id = event->vdev_id;
	*status = event->status;

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * extract_quiet_offload_event_tlv() - extract quiet offload event
 * @wmi_handle: WMI handle
 * @evt_buf: event buffer
 * @mld_mac: mld mac address
 * @link_mac: link mac address
 * @link_id: link id
 * @quiet_status: quiet is started or stopped
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS extract_quiet_offload_event_tlv(
				wmi_unified_t wmi_handle, void *evt_buf,
				struct vdev_sta_quiet_event *quiet_event)
{
	WMI_QUIET_HANDLING_EVENTID_param_tlvs *param_buf;
	wmi_quiet_event_fixed_param *event;

	param_buf = (WMI_QUIET_HANDLING_EVENTID_param_tlvs *)evt_buf;

	event = param_buf->fixed_param;

	if (!(event->mld_mac_address_present && event->linkid_present) &&
	    !event->link_mac_address_present) {
		wmi_err("Invalid sta quiet offload event. present bit: mld mac %d link mac %d linkid %d",
			event->mld_mac_address_present,
			event->linkid_present,
			event->link_mac_address_present);
		return QDF_STATUS_E_INVAL;
	}

	if (event->mld_mac_address_present)
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->mld_mac_address,
					   quiet_event->mld_mac.bytes);
	if (event->link_mac_address_present)
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->link_mac_address,
					   quiet_event->link_mac.bytes);
	if (event->linkid_present)
		quiet_event->link_id = event->linkid;
	quiet_event->quiet_status = (event->quiet_status ==
				     WMI_QUIET_EVENT_START);

	return QDF_STATUS_SUCCESS;
}
#endif

struct wmi_ops tlv_ops =  {
	.send_vdev_create_cmd = send_vdev_create_cmd_tlv,
	.send_vdev_delete_cmd = send_vdev_delete_cmd_tlv,
	.send_vdev_nss_chain_params_cmd = send_vdev_nss_chain_params_cmd_tlv,
	.send_vdev_down_cmd = send_vdev_down_cmd_tlv,
	.send_vdev_start_cmd = send_vdev_start_cmd_tlv,
	.send_peer_flush_tids_cmd = send_peer_flush_tids_cmd_tlv,
	.send_peer_param_cmd = send_peer_param_cmd_tlv,
	.send_vdev_up_cmd = send_vdev_up_cmd_tlv,
	.send_vdev_stop_cmd = send_vdev_stop_cmd_tlv,
	.send_peer_create_cmd = send_peer_create_cmd_tlv,
	.send_peer_delete_cmd = send_peer_delete_cmd_tlv,
	.send_peer_delete_all_cmd = send_peer_delete_all_cmd_tlv,
	.send_peer_rx_reorder_queue_setup_cmd =
		send_peer_rx_reorder_queue_setup_cmd_tlv,
	.send_peer_rx_reorder_queue_remove_cmd =
		send_peer_rx_reorder_queue_remove_cmd_tlv,
	.send_pdev_utf_cmd = send_pdev_utf_cmd_tlv,
	.send_pdev_param_cmd = send_pdev_param_cmd_tlv,
	.send_pdev_set_hw_mode_cmd = send_pdev_set_hw_mode_cmd_tlv,
	.send_suspend_cmd = send_suspend_cmd_tlv,
	.send_resume_cmd = send_resume_cmd_tlv,
	.send_wow_enable_cmd = send_wow_enable_cmd_tlv,
	.send_set_ap_ps_param_cmd = send_set_ap_ps_param_cmd_tlv,
	.send_set_sta_ps_param_cmd = send_set_sta_ps_param_cmd_tlv,
	.send_crash_inject_cmd = send_crash_inject_cmd_tlv,
	.send_dbglog_cmd = send_dbglog_cmd_tlv,
	.send_vdev_set_param_cmd = send_vdev_set_param_cmd_tlv,
	.send_vdev_set_mu_snif_cmd = send_vdev_set_mu_snif_cmd_tlv,
	.send_packet_log_enable_cmd = send_packet_log_enable_cmd_tlv,
	.send_peer_based_pktlog_cmd = send_peer_based_pktlog_cmd,
	.send_time_stamp_sync_cmd = send_time_stamp_sync_cmd_tlv,
	.send_packet_log_disable_cmd = send_packet_log_disable_cmd_tlv,
	.send_beacon_tmpl_send_cmd = send_beacon_tmpl_send_cmd_tlv,
	.send_fd_tmpl_cmd = send_fd_tmpl_cmd_tlv,
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
	.send_obss_disable_cmd = send_obss_disable_cmd_tlv,
	.send_nlo_mawc_cmd = send_nlo_mawc_cmd_tlv,
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	.send_process_ll_stats_clear_cmd = send_process_ll_stats_clear_cmd_tlv,
	.send_process_ll_stats_set_cmd = send_process_ll_stats_set_cmd_tlv,
	.send_process_ll_stats_get_cmd = send_process_ll_stats_get_cmd_tlv,
#ifdef FEATURE_CLUB_LL_STATS_AND_GET_STATION
	.send_unified_ll_stats_get_sta_cmd =
					send_unified_ll_stats_get_sta_cmd_tlv,
#endif /* FEATURE_CLUB_LL_STATS_AND_GET_STATION */
#endif /* WLAN_FEATURE_LINK_LAYER_STATS*/
	.send_congestion_cmd = send_congestion_cmd_tlv,
	.send_snr_request_cmd = send_snr_request_cmd_tlv,
	.send_snr_cmd = send_snr_cmd_tlv,
	.send_link_status_req_cmd = send_link_status_req_cmd_tlv,
#if !defined(REMOVE_PKT_LOG) && defined(FEATURE_PKTLOG)
	.send_pktlog_wmi_send_cmd = send_pktlog_wmi_send_cmd_tlv,
#endif
#ifdef WLAN_SUPPORT_GREEN_AP
	.send_egap_conf_params_cmd = send_egap_conf_params_cmd_tlv,
	.send_green_ap_ps_cmd = send_green_ap_ps_cmd_tlv,
	.extract_green_ap_egap_status_info =
			extract_green_ap_egap_status_info_tlv,
#endif
	.send_csa_offload_enable_cmd = send_csa_offload_enable_cmd_tlv,
	.send_start_oem_data_cmd = send_start_oem_data_cmd_tlv,
#ifdef FEATURE_OEM_DATA
	.send_start_oemv2_data_cmd = send_start_oemv2_data_cmd_tlv,
#endif
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
#ifdef WLAN_IOT_SIM_SUPPORT
	.send_simulation_test_cmd = send_simulation_test_cmd_tlv,
#endif
	.send_vdev_spectral_configure_cmd =
				send_vdev_spectral_configure_cmd_tlv,
	.send_vdev_spectral_enable_cmd =
				send_vdev_spectral_enable_cmd_tlv,
#ifdef WLAN_CONV_SPECTRAL_ENABLE
	.extract_pdev_sscan_fw_cmd_fixed_param =
				extract_pdev_sscan_fw_cmd_fixed_param_tlv,
	.extract_pdev_sscan_fft_bin_index =
				extract_pdev_sscan_fft_bin_index_tlv,
#ifdef SPECTRAL_BERYLLIUM
	.extract_pdev_spectral_session_chan_info =
				extract_pdev_spectral_session_chan_info_tlv,
	.extract_pdev_spectral_session_detector_info =
				extract_pdev_spectral_session_detector_info_tlv,
#endif /* SPECTRAL_BERYLLIUM */
#endif /* WLAN_CONV_SPECTRAL_ENABLE */
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
	.extract_num_mem_reqs = extract_num_mem_reqs_tlv,
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
#ifdef FEATURE_WLAN_SCAN_PNO
	.extract_nlo_match_ev_param = extract_nlo_match_ev_param_tlv,
	.extract_nlo_complete_ev_param = extract_nlo_complete_ev_param_tlv,
#endif
	.extract_unit_test = extract_unit_test_tlv,
	.extract_pdev_ext_stats = extract_pdev_ext_stats_tlv,
	.extract_bcn_stats = extract_bcn_stats_tlv,
	.extract_bcnflt_stats = extract_bcnflt_stats_tlv,
	.extract_chan_stats = extract_chan_stats_tlv,
	.extract_vdev_prb_fils_stats = extract_vdev_prb_fils_stats_tlv,
	.extract_profile_ctx = extract_profile_ctx_tlv,
	.extract_profile_data = extract_profile_data_tlv,
	.send_fw_test_cmd = send_fw_test_cmd_tlv,
	.send_wfa_test_cmd = send_wfa_test_cmd_tlv,
	.send_power_dbg_cmd = send_power_dbg_cmd_tlv,
	.extract_service_ready_ext = extract_service_ready_ext_tlv,
	.extract_service_ready_ext2 = extract_service_ready_ext2_tlv,
	.extract_dbs_or_sbs_service_ready_ext2 =
				extract_dbs_or_sbs_cap_service_ready_ext2_tlv,
	.extract_hw_mode_cap_service_ready_ext =
				extract_hw_mode_cap_service_ready_ext_tlv,
	.extract_mac_phy_cap_service_ready_ext =
				extract_mac_phy_cap_service_ready_ext_tlv,
	.extract_mac_phy_cap_service_ready_ext2 =
				extract_mac_phy_cap_service_ready_ext2_tlv,
	.extract_reg_cap_service_ready_ext =
				extract_reg_cap_service_ready_ext_tlv,
	.extract_hal_reg_cap_ext2 = extract_hal_reg_cap_ext2_tlv,
	.extract_dbr_ring_cap_service_ready_ext =
				extract_dbr_ring_cap_service_ready_ext_tlv,
	.extract_dbr_ring_cap_service_ready_ext2 =
				extract_dbr_ring_cap_service_ready_ext2_tlv,
	.extract_scan_radio_cap_service_ready_ext2 =
				extract_scan_radio_cap_service_ready_ext2_tlv,
	.extract_sar_cap_service_ready_ext =
				extract_sar_cap_service_ready_ext_tlv,
	.extract_sar_cap_service_ready_ext2 =
				extract_sar_cap_service_ready_ext2_tlv,
	.extract_pdev_utf_event = extract_pdev_utf_event_tlv,
	.wmi_set_htc_tx_tag = wmi_set_htc_tx_tag_tlv,
	.extract_fips_event_data = extract_fips_event_data_tlv,
#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
	.extract_fips_extend_ev_data = extract_fips_extend_event_data_tlv,
#endif
#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
	.send_vdev_fils_enable_cmd = send_vdev_fils_enable_cmd_send,
#endif
#ifdef WLAN_FEATURE_DISA
	.extract_encrypt_decrypt_resp_event =
				extract_encrypt_decrypt_resp_event_tlv,
#endif
	.send_pdev_fips_cmd = send_pdev_fips_cmd_tlv,
#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
	.send_pdev_fips_extend_cmd = send_pdev_fips_extend_cmd_tlv,
	.send_pdev_fips_mode_set_cmd = send_pdev_fips_mode_set_cmd_tlv,
#endif
	.extract_get_pn_data = extract_get_pn_data_tlv,
	.send_pdev_get_pn_cmd = send_pdev_get_pn_cmd_tlv,
	.send_wlan_profile_enable_cmd = send_wlan_profile_enable_cmd_tlv,
#ifdef WLAN_FEATURE_DISA
	.send_encrypt_decrypt_send_cmd = send_encrypt_decrypt_send_cmd_tlv,
#endif
	.send_wlan_profile_trigger_cmd = send_wlan_profile_trigger_cmd_tlv,
	.send_wlan_profile_hist_intvl_cmd =
				send_wlan_profile_hist_intvl_cmd_tlv,
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
#ifdef CONFIG_BAND_6GHZ
	.extract_reg_chan_list_ext_update_event =
		extract_reg_chan_list_ext_update_event_tlv,
#ifdef CONFIG_AFC_SUPPORT
	.extract_afc_event = extract_afc_event_tlv,
#endif
#endif
#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
	.extract_num_rf_characterization_entries =
		extract_num_rf_characterization_entries_tlv,
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

	.convert_host_vdev_param_tlv = convert_host_vdev_param_tlv,

	.convert_phy_id_host_to_target =
		convert_host_phy_id_to_target_phy_id_legacy,
	.convert_phy_id_target_to_host =
		convert_target_phy_id_to_host_phy_id_legacy,

	.convert_host_phy_id_to_target =
		convert_host_phy_id_to_target_phy_id,
	.convert_target_phy_id_to_host =
		convert_target_phy_id_to_host_phy_id,

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
	.send_self_srg_bss_color_bitmap_set =
		send_self_srg_bss_color_bitmap_set_cmd_tlv,
	.send_self_srg_partial_bssid_bitmap_set =
		send_self_srg_partial_bssid_bitmap_set_cmd_tlv,
	.send_self_srg_obss_color_enable_bitmap =
		send_self_srg_obss_color_enable_bitmap_cmd_tlv,
	.send_self_srg_obss_bssid_enable_bitmap =
		send_self_srg_obss_bssid_enable_bitmap_cmd_tlv,
	.send_self_non_srg_obss_color_enable_bitmap =
		send_self_non_srg_obss_color_enable_bitmap_cmd_tlv,
	.send_self_non_srg_obss_bssid_enable_bitmap =
		send_self_non_srg_obss_bssid_enable_bitmap_cmd_tlv,
#endif
	.extract_offload_bcn_tx_status_evt = extract_offload_bcn_tx_status_evt,
	.extract_ctl_failsafe_check_ev_param =
		extract_ctl_failsafe_check_ev_param_tlv,
#ifdef WIFI_POS_CONVERGED
	.extract_oem_response_param = extract_oem_response_param_tlv,
#endif /* WIFI_POS_CONVERGED */
#ifdef WLAN_MWS_INFO_DEBUGFS
	.send_mws_coex_status_req_cmd = send_mws_coex_status_req_cmd_tlv,
#endif
	.extract_hw_mode_resp_event = extract_hw_mode_resp_event_status_tlv,
#ifdef FEATURE_ANI_LEVEL_REQUEST
	.send_ani_level_cmd = send_ani_level_cmd_tlv,
	.extract_ani_level = extract_ani_level_tlv,
#endif /* FEATURE_ANI_LEVEL_REQUEST */
	.extract_roam_trigger_stats = extract_roam_trigger_stats_tlv,
	.extract_roam_scan_stats = extract_roam_scan_stats_tlv,
	.extract_roam_result_stats = extract_roam_result_stats_tlv,
	.extract_roam_11kv_stats = extract_roam_11kv_stats_tlv,
#ifdef WLAN_FEATURE_PKT_CAPTURE
	.extract_vdev_mgmt_offload_event = extract_vdev_mgmt_offload_event_tlv,
#endif
#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
	.extract_smart_monitor_event = extract_smart_monitor_event_tlv,
#endif

#ifdef FEATURE_WLAN_TIME_SYNC_FTM
	.send_wlan_time_sync_ftm_trigger_cmd = send_wlan_ts_ftm_trigger_cmd_tlv,
	.send_wlan_ts_qtime_cmd = send_wlan_ts_qtime_cmd_tlv,
	.extract_time_sync_ftm_start_stop_event =
				extract_time_sync_ftm_start_stop_event_tlv,
	.extract_time_sync_ftm_offset_event =
					extract_time_sync_ftm_offset_event_tlv,
#endif /* FEATURE_WLAN_TIME_SYNC_FTM */
	.send_roam_scan_ch_list_req_cmd = send_roam_scan_ch_list_req_cmd_tlv,
	.send_injector_config_cmd = send_injector_config_cmd_tlv,
	.send_cp_stats_cmd = send_cp_stats_cmd_tlv,
#ifdef FEATURE_MEC_OFFLOAD
	.send_pdev_set_mec_timer_cmd = send_pdev_set_mec_timer_cmd_tlv,
#endif
#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
	.extract_infra_cp_stats = extract_infra_cp_stats_tlv,
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */
	.extract_cp_stats_more_pending =
				extract_cp_stats_more_pending_tlv,
	.send_vdev_tsf_tstamp_action_cmd = send_vdev_tsf_tstamp_action_cmd_tlv,
	.extract_vdev_tsf_report_event = extract_vdev_tsf_report_event_tlv,
	.extract_pdev_csa_switch_count_status =
		extract_pdev_csa_switch_count_status_tlv,
	.send_set_tpc_power_cmd = send_set_tpc_power_cmd_tlv,
#ifdef CONFIG_AFC_SUPPORT
	.send_afc_cmd = send_afc_cmd_tlv,
#endif
	.extract_dpd_status_ev_param = extract_dpd_status_ev_param_tlv,
	.extract_install_key_comp_event = extract_install_key_comp_event_tlv,
	.extract_halphy_cal_status_ev_param = extract_halphy_cal_status_ev_param_tlv,
	.send_set_halphy_cal = send_set_halphy_cal_tlv,
	.extract_halphy_cal_ev_param = extract_halphy_cal_ev_param_tlv,
#ifdef WLAN_MGMT_RX_REO_SUPPORT
	.extract_mgmt_rx_fw_consumed = extract_mgmt_rx_fw_consumed_tlv,
	.extract_mgmt_rx_reo_params = extract_mgmt_rx_reo_params_tlv,
	.send_mgmt_rx_reo_filter_config_cmd =
		send_mgmt_rx_reo_filter_config_cmd_tlv,
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	.send_roam_set_param_cmd = send_roam_set_param_cmd_tlv,
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	.send_set_mac_address_cmd = send_set_mac_address_cmd_tlv,
	.extract_update_mac_address_event =
					extract_update_mac_address_event_tlv,
#endif

#ifdef WLAN_FEATURE_11BE_MLO
	.extract_quiet_offload_event =
				extract_quiet_offload_event_tlv,
#endif

#ifdef WLAN_FEATURE_PEER_TXQ_FLUSH_CONF
	.send_peer_txq_flush_config_cmd = send_peer_txq_flush_config_cmd_tlv,
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
	event_ids[wmi_service_ready_ext2_event_id] =
						WMI_SERVICE_READY_EXT2_EVENTID;
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
	event_ids[wmi_peer_create_conf_event_id] =
					WMI_PEER_CREATE_CONF_EVENTID;
	event_ids[wmi_peer_delete_response_event_id] =
					WMI_PEER_DELETE_RESP_EVENTID;
	event_ids[wmi_peer_delete_all_response_event_id] =
					WMI_VDEV_DELETE_ALL_PEER_RESP_EVENTID;
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
#ifdef WLAN_FEATURE_FIPS_BER_CCMGCM
	event_ids[wmi_pdev_fips_extend_event_id] = WMI_PDEV_FIPS_EXTEND_EVENTID;
#endif
	event_ids[wmi_pdev_csa_switch_count_status_event_id] =
				WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID;
	event_ids[wmi_vdev_ocac_complete_event_id] =
				WMI_VDEV_ADFS_OCAC_COMPLETE_EVENTID;
	event_ids[wmi_reg_chan_list_cc_event_id] = WMI_REG_CHAN_LIST_CC_EVENTID;
	event_ids[wmi_reg_chan_list_cc_ext_event_id] =
					WMI_REG_CHAN_LIST_CC_EXT_EVENTID;
#ifdef CONFIG_AFC_SUPPORT
	event_ids[wmi_afc_event_id] = WMI_AFC_EVENTID,
#endif
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
#ifdef WLAN_SUPPORT_TWT
	event_ids[wmi_twt_enable_complete_event_id] =
		WMI_TWT_ENABLE_COMPLETE_EVENTID;
	event_ids[wmi_twt_disable_complete_event_id] =
		WMI_TWT_DISABLE_COMPLETE_EVENTID;
	event_ids[wmi_twt_add_dialog_complete_event_id] =
		WMI_TWT_ADD_DIALOG_COMPLETE_EVENTID;
	event_ids[wmi_twt_del_dialog_complete_event_id] =
		WMI_TWT_DEL_DIALOG_COMPLETE_EVENTID;
	event_ids[wmi_twt_pause_dialog_complete_event_id] =
		WMI_TWT_PAUSE_DIALOG_COMPLETE_EVENTID;
	event_ids[wmi_twt_resume_dialog_complete_event_id] =
		WMI_TWT_RESUME_DIALOG_COMPLETE_EVENTID;
	event_ids[wmi_twt_nudge_dialog_complete_event_id] =
		WMI_TWT_NUDGE_DIALOG_COMPLETE_EVENTID;
	event_ids[wmi_twt_session_stats_event_id] =
		WMI_TWT_SESSION_STATS_EVENTID;
	event_ids[wmi_twt_notify_event_id] =
		WMI_TWT_NOTIFY_EVENTID;
	event_ids[wmi_twt_ack_complete_event_id] =
		WMI_TWT_ACK_EVENTID;
#endif
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
	event_ids[wmi_coex_report_antenna_isolation_event_id] =
				WMI_COEX_REPORT_ANTENNA_ISOLATION_EVENTID;
	event_ids[wmi_peer_ratecode_list_event_id] =
				WMI_PEER_RATECODE_LIST_EVENTID;
	event_ids[wmi_chan_rf_characterization_info_event_id] =
				WMI_CHAN_RF_CHARACTERIZATION_INFO_EVENTID;
	event_ids[wmi_roam_auth_offload_event_id] =
				WMI_ROAM_PREAUTH_START_EVENTID;
	event_ids[wmi_get_elna_bypass_event_id] = WMI_GET_ELNA_BYPASS_EVENTID;
	event_ids[wmi_motion_det_host_eventid] = WMI_MOTION_DET_HOST_EVENTID;
	event_ids[wmi_motion_det_base_line_host_eventid] =
				WMI_MOTION_DET_BASE_LINE_HOST_EVENTID;
	event_ids[wmi_get_ani_level_event_id] = WMI_GET_CHANNEL_ANI_EVENTID;
	event_ids[wmi_peer_tx_pn_response_event_id] =
		WMI_PEER_TX_PN_RESPONSE_EVENTID;
	event_ids[wmi_roam_stats_event_id] = WMI_ROAM_STATS_EVENTID;
	event_ids[wmi_oem_data_event_id] = WMI_OEM_DATA_EVENTID;
	event_ids[wmi_mgmt_offload_data_event_id] =
				WMI_VDEV_MGMT_OFFLOAD_EVENTID;
	event_ids[wmi_nan_dmesg_event_id] =
				WMI_NAN_DMESG_EVENTID;
	event_ids[wmi_pdev_multi_vdev_restart_response_event_id] =
				WMI_PDEV_MULTIPLE_VDEV_RESTART_RESP_EVENTID;
	event_ids[wmi_roam_pmkid_request_event_id] =
				WMI_ROAM_PMKID_REQUEST_EVENTID;
#ifdef FEATURE_WLAN_TIME_SYNC_FTM
	event_ids[wmi_wlan_time_sync_ftm_start_stop_event_id] =
				WMI_VDEV_AUDIO_SYNC_START_STOP_EVENTID;
	event_ids[wmi_wlan_time_sync_q_master_slave_offset_eventid] =
			WMI_VDEV_AUDIO_SYNC_Q_MASTER_SLAVE_OFFSET_EVENTID;
#endif
event_ids[wmi_roam_scan_chan_list_id] =
			WMI_ROAM_SCAN_CHANNEL_LIST_EVENTID;
	event_ids[wmi_muedca_params_config_eventid] =
			WMI_MUEDCA_PARAMS_CONFIG_EVENTID;
	event_ids[wmi_pdev_sscan_fw_param_eventid] =
			WMI_PDEV_SSCAN_FW_PARAM_EVENTID;
	event_ids[wmi_roam_cap_report_event_id] =
			WMI_ROAM_CAPABILITY_REPORT_EVENTID;
	event_ids[wmi_vdev_bcn_latency_event_id] =
			WMI_VDEV_BCN_LATENCY_EVENTID;
	event_ids[wmi_vdev_disconnect_event_id] =
			WMI_VDEV_DISCONNECT_EVENTID;
	event_ids[wmi_peer_create_conf_event_id] =
			WMI_PEER_CREATE_CONF_EVENTID;
	event_ids[wmi_pdev_cp_fwstats_eventid] =
			WMI_CTRL_PATH_STATS_EVENTID;
	event_ids[wmi_vdev_send_big_data_p2_eventid] =
			WMI_VDEV_SEND_BIG_DATA_P2_EVENTID;
	event_ids[wmi_pdev_get_dpd_status_event_id] =
			WMI_PDEV_GET_DPD_STATUS_EVENTID;
#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
	event_ids[wmi_vdev_smart_monitor_event_id] =
			WMI_VDEV_SMART_MONITOR_EVENTID;
#endif
	event_ids[wmi_pdev_get_halphy_cal_status_event_id] =
			WMI_PDEV_GET_HALPHY_CAL_STATUS_EVENTID;
	event_ids[wmi_pdev_set_halphy_cal_event_id] =
			WMI_PDEV_SET_HALPHY_CAL_BMAP_EVENTID;
	event_ids[wmi_pdev_aoa_phasedelta_event_id] =
			WMI_PDEV_AOA_PHASEDELTA_EVENTID;
#ifdef WLAN_MGMT_RX_REO_SUPPORT
	event_ids[wmi_mgmt_rx_fw_consumed_eventid] =
			WMI_MGMT_RX_FW_CONSUMED_EVENTID;
#endif
#ifdef WLAN_FEATURE_11BE_MLO
	event_ids[wmi_mlo_setup_complete_event_id] =
			WMI_MLO_SETUP_COMPLETE_EVENTID;
	event_ids[wmi_mlo_teardown_complete_event_id] =
			WMI_MLO_TEARDOWN_COMPLETE_EVENTID;
	event_ids[wmi_mlo_link_set_active_resp_eventid] =
			WMI_MLO_LINK_SET_ACTIVE_RESP_EVENTID;
#endif
	event_ids[wmi_roam_frame_event_id] =
				WMI_ROAM_FRAME_EVENTID;
#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	event_ids[wmi_vdev_update_mac_addr_conf_eventid] =
			WMI_VDEV_UPDATE_MAC_ADDR_CONF_EVENTID;
#endif
#ifdef WLAN_FEATURE_11BE_MLO
	event_ids[wmi_vdev_quiet_offload_eventid] =
			WMI_QUIET_HANDLING_EVENTID;
#endif
#ifdef MULTI_CLIENT_LL_SUPPORT
	event_ids[wmi_vdev_latency_event_id] = WMI_VDEV_LATENCY_LEVEL_EVENTID;
#endif
#ifdef WLAN_FEATURE_COAP
	event_ids[wmi_wow_coap_buf_info_eventid] =
		WMI_WOW_COAP_BUF_INFO_EVENTID;
#endif
}

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
#ifdef FEATURE_CLUB_LL_STATS_AND_GET_STATION
static void wmi_populate_service_get_sta_in_ll_stats_req(uint32_t *wmi_service)
{
	wmi_service[wmi_service_get_station_in_ll_stats_req] =
				WMI_SERVICE_UNIFIED_LL_GET_STA_CMD_SUPPORT;
}
#else
static void wmi_populate_service_get_sta_in_ll_stats_req(uint32_t *wmi_service)
{
}
#endif /* FEATURE_CLUB_LL_STATS_AND_GET_STATION */
#else
static void wmi_populate_service_get_sta_in_ll_stats_req(uint32_t *wmi_service)
{
}
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

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
	wmi_service[wmi_service_ext2_msg] = WMI_SERVICE_EXT2_MSG;
	wmi_service[wmi_service_mawc] = WMI_SERVICE_MAWC;
	wmi_service[wmi_service_multiple_vdev_restart] =
			WMI_SERVICE_MULTIPLE_VDEV_RESTART;
	wmi_service[wmi_service_smart_antenna_sw_support] =
				WMI_SERVICE_SMART_ANTENNA_SW_SUPPORT;
	wmi_service[wmi_service_smart_antenna_hw_support] =
				WMI_SERVICE_SMART_ANTENNA_HW_SUPPORT;

	wmi_service[wmi_service_roam_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ratectrl] = WMI_SERVICE_UNAVAILABLE;
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
	wmi_service[wmi_service_ema_ap_support] = WMI_SERVICE_EMA_AP_SUPPORT;
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
	wmi_service[wmi_service_vdev_delete_all_peer] =
			WMI_SERVICE_DELETE_ALL_PEER_SUPPORT;
	wmi_service[wmi_service_three_way_coex_config_legacy] =
			WMI_SERVICE_THREE_WAY_COEX_CONFIG_LEGACY;
	wmi_service[wmi_service_rx_fse_support] =
			WMI_SERVICE_RX_FSE_SUPPORT;
	wmi_service[wmi_service_sae_roam_support] =
			WMI_SERVICE_WPA3_SAE_ROAM_SUPPORT;
	wmi_service[wmi_service_owe_roam_support] =
			WMI_SERVICE_WPA3_OWE_ROAM_SUPPORT;
	wmi_service[wmi_service_6ghz_support] =
			WMI_SERVICE_6GHZ_SUPPORT;
	wmi_service[wmi_service_bw_165mhz_support] =
			WMI_SERVICE_BW_165MHZ_SUPPORT;
	wmi_service[wmi_service_bw_restricted_80p80_support] =
			WMI_SERVICE_BW_RESTRICTED_80P80_SUPPORT;
	wmi_service[wmi_service_packet_capture_support] =
			WMI_SERVICE_PACKET_CAPTURE_SUPPORT;
	wmi_service[wmi_service_nan_vdev] = WMI_SERVICE_NAN_VDEV_SUPPORT;
	wmi_service[wmi_service_peer_delete_no_peer_flush_tids_cmd] =
		WMI_SERVICE_PEER_DELETE_NO_PEER_FLUSH_TIDS_CMD;
	wmi_service[wmi_service_multiple_vdev_restart_ext] =
			WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_time_sync_ftm] =
			WMI_SERVICE_AUDIO_SYNC_SUPPORT;
	wmi_service[wmi_service_nss_ratio_to_host_support] =
			WMI_SERVICE_NSS_RATIO_TO_HOST_SUPPORT;
	wmi_service[wmi_roam_scan_chan_list_to_host_support] =
			WMI_SERVICE_ROAM_SCAN_CHANNEL_LIST_TO_HOST_SUPPORT;
	wmi_service[wmi_beacon_protection_support] =
			WMI_SERVICE_BEACON_PROTECTION_SUPPORT;
	wmi_service[wmi_service_sta_nan_ndi_four_port] =
			WMI_SERVICE_NDI_NDI_STA_SUPPORT;
	wmi_service[wmi_service_host_scan_stop_vdev_all] =
		WMI_SERVICE_HOST_SCAN_STOP_VDEV_ALL_SUPPORT;
	wmi_service[wmi_support_extend_address] =
			WMI_SERVICE_SUPPORT_EXTEND_ADDRESS;
	wmi_service[wmi_service_srg_srp_spatial_reuse_support] =
		WMI_SERVICE_SRG_SRP_SPATIAL_REUSE_SUPPORT;
	wmi_service[wmi_service_suiteb_roam_support] =
			WMI_SERVICE_WPA3_SUITEB_ROAM_SUPPORT;
	wmi_service[wmi_service_no_interband_mcc_support] =
			WMI_SERVICE_NO_INTERBAND_MCC_SUPPORT;
	wmi_service[wmi_service_dual_sta_roam_support] =
			WMI_SERVICE_DUAL_STA_ROAM_SUPPORT;
	wmi_service[wmi_service_peer_create_conf] =
			WMI_SERVICE_PEER_CREATE_CONF;
	wmi_service[wmi_service_configure_roam_trigger_param_support] =
			WMI_SERVICE_CONFIGURE_ROAM_TRIGGER_PARAM_SUPPORT;
	wmi_service[wmi_service_5dot9_ghz_support] =
			WMI_SERVICE_5_DOT_9GHZ_SUPPORT;
	wmi_service[wmi_service_cfr_ta_ra_as_fp_support] =
			WMI_SERVICE_CFR_TA_RA_AS_FP_SUPPORT;
	wmi_service[wmi_service_cfr_capture_count_support] =
			WMI_SERVICE_CFR_CAPTURE_COUNT_SUPPORT;
	wmi_service[wmi_service_ocv_support] =
			WMI_SERVICE_OCV_SUPPORT;
	wmi_service[wmi_service_ll_stats_per_chan_rx_tx_time] =
			WMI_SERVICE_LL_STATS_PER_CHAN_RX_TX_TIME_SUPPORT;
	wmi_service[wmi_service_thermal_multi_client_support] =
			WMI_SERVICE_THERMAL_MULTI_CLIENT_SUPPORT;
	wmi_service[wmi_service_mbss_param_in_vdev_start_support] =
			WMI_SERVICE_MBSS_PARAM_IN_VDEV_START_SUPPORT;
	wmi_service[wmi_service_fse_cmem_alloc_support] =
			WMI_SERVICE_FSE_CMEM_ALLOC_SUPPORT;
	wmi_service[wmi_service_scan_conf_per_ch_support] =
			WMI_SERVICE_SCAN_CONFIG_PER_CHANNEL;
	wmi_service[wmi_service_csa_beacon_template] =
			WMI_SERVICE_CSA_BEACON_TEMPLATE;
#if defined(WIFI_POS_CONVERGED) && defined(WLAN_FEATURE_RTT_11AZ_SUPPORT)
	wmi_service[wmi_service_rtt_11az_ntb_support] =
			WMI_SERVICE_RTT_11AZ_NTB_SUPPORT;
	wmi_service[wmi_service_rtt_11az_tb_support] =
			WMI_SERVICE_RTT_11AZ_TB_SUPPORT;
	wmi_service[wmi_service_rtt_11az_mac_sec_support] =
			WMI_SERVICE_RTT_11AZ_MAC_SEC_SUPPORT;
	wmi_service[wmi_service_rtt_11az_mac_phy_sec_support] =
			WMI_SERVICE_RTT_11AZ_MAC_PHY_SEC_SUPPORT;
#endif
#ifdef WLAN_FEATURE_IGMP_OFFLOAD
	wmi_service[wmi_service_igmp_offload_support] =
			WMI_SERVICE_IGMP_OFFLOAD_SUPPORT;
#endif
#ifdef WLAN_FEATURE_11AX
#ifdef FEATURE_WLAN_TDLS
	wmi_service[wmi_service_tdls_ax_support] =
			WMI_SERVICE_11AX_TDLS_SUPPORT;
#endif
#endif
#ifdef WLAN_SUPPORT_TWT
	wmi_service[wmi_service_twt_bcast_req_support] =
			WMI_SERVICE_BROADCAST_TWT_REQUESTER;
	wmi_service[wmi_service_twt_bcast_resp_support] =
			WMI_SERVICE_BROADCAST_TWT_RESPONDER;
	wmi_service[wmi_service_twt_nudge] =
			WMI_SERVICE_TWT_NUDGE;
	wmi_service[wmi_service_all_twt] =
			WMI_SERVICE_TWT_ALL_DIALOG_ID;
	wmi_service[wmi_service_twt_statistics] =
			WMI_SERVICE_TWT_STATS;
#endif
	wmi_service[wmi_service_spectral_scan_disabled] =
			WMI_SERVICE_SPECTRAL_SCAN_DISABLED;
	wmi_service[wmi_service_sae_eapol_offload_support] =
			WMI_SERVICE_SAE_EAPOL_OFFLOAD_SUPPORT;
	wmi_populate_service_get_sta_in_ll_stats_req(wmi_service);

	wmi_service[wmi_service_wapi_concurrency_supported] =
			WMI_SERVICE_WAPI_CONCURRENCY_SUPPORTED;
	wmi_service[wmi_service_sap_connected_d3_wow] =
			WMI_SERVICE_SAP_CONNECTED_D3WOW;
	wmi_service[wmi_service_go_connected_d3_wow] =
			WMI_SERVICE_SAP_CONNECTED_D3WOW;
	wmi_service[wmi_service_ext_tpc_reg_support] =
			WMI_SERVICE_EXT_TPC_REG_SUPPORT;
	wmi_service[wmi_service_ndi_txbf_support] =
			WMI_SERVICE_NDI_TXBF_SUPPORT;
	wmi_service[wmi_service_reg_cc_ext_event_support] =
			WMI_SERVICE_REG_CC_EXT_EVENT_SUPPORT;
#if defined(CONFIG_BAND_6GHZ)
	wmi_service[wmi_service_lower_6g_edge_ch_supp] =
			WMI_SERVICE_ENABLE_LOWER_6G_EDGE_CH_SUPP;
	wmi_service[wmi_service_disable_upper_6g_edge_ch_supp] =
			WMI_SERVICE_DISABLE_UPPER_6G_EDGE_CH_SUPP;
#endif
	wmi_service[wmi_service_dcs_awgn_int_support] =
			WMI_SERVICE_DCS_AWGN_INT_SUPPORT;
	wmi_populate_service_11be(wmi_service);

#ifdef WLAN_FEATURE_BIG_DATA_STATS
	wmi_service[wmi_service_big_data_support] =
			WMI_SERVICE_BIG_DATA_SUPPORT;
#endif
	wmi_service[wmi_service_ampdu_tx_buf_size_256_support] =
			WMI_SERVICE_AMPDU_TX_BUF_SIZE_256_SUPPORT;
	wmi_service[wmi_service_halphy_cal_enable_disable_support] =
			WMI_SERVICE_HALPHY_CAL_ENABLE_DISABLE_SUPPORT;
	wmi_service[wmi_service_halphy_cal_status] =
			WMI_SERVICE_HALPHY_CAL_STATUS;
	wmi_service[wmi_service_rtt_ap_initiator_staggered_mode_supported] =
			WMI_SERVICE_RTT_AP_INITIATOR_STAGGERED_MODE_SUPPORTED;
	wmi_service[wmi_service_rtt_ap_initiator_bursted_mode_supported] =
			WMI_SERVICE_RTT_AP_INITIATOR_BURSTED_MODE_SUPPORTED;
	wmi_service[wmi_service_ema_multiple_group_supported] =
			WMI_SERVICE_EMA_MULTIPLE_GROUP_SUPPORT;
	wmi_service[wmi_service_large_beacon_supported] =
			WMI_SERVICE_LARGE_BEACON_SUPPORT;
	wmi_service[wmi_service_aoa_for_rcc_supported] =
			WMI_SERVICE_AOA_FOR_RCC_SUPPORTED;
#ifdef WLAN_FEATURE_P2P_P2P_STA
	wmi_service[wmi_service_p2p_p2p_cc_support] =
			WMI_SERVICE_P2P_P2P_CONCURRENCY_SUPPORT;
#endif
#ifdef THERMAL_STATS_SUPPORT
	wmi_service[wmi_service_thermal_stats_temp_range_supported] =
			WMI_SERVICE_THERMAL_THROT_STATS_TEMP_RANGE_SUPPORT;
#endif
	wmi_service[wmi_service_hw_mode_policy_offload_support] =
			WMI_SERVICE_HW_MODE_POLICY_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_mgmt_rx_reo_supported] =
			WMI_SERVICE_MGMT_RX_REO_SUPPORTED;
	wmi_service[wmi_service_phy_dma_byte_swap_support] =
			WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_spectral_session_info_support] =
			WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mu_snif] = WMI_SERVICE_MU_SNIF;
#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	wmi_service[wmi_service_dynamic_update_vdev_macaddr_support] =
			WMI_SERVICE_DYNAMIC_VDEV_MAC_ADDR_UPDATE_SUPPORT;
#endif
	wmi_service[wmi_service_pno_scan_conf_per_ch_support] =
			WMI_SERVICE_PNO_SCAN_CONFIG_PER_CHANNEL;
#ifdef WLAN_FEATURE_11BE_MLO
	wmi_service[wmi_service_mlo_sta_nan_ndi_support] =
			WMI_SERVICE_MLO_STA_NAN_NDI_SUPPORT;
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	wmi_service[wmi_service_roam_stats_per_candidate_frame_info] =
		WMI_SERVICE_ROAM_STAT_PER_CANDIDATE_FRAME_INFO_SUPPORT;
#endif
#ifdef MULTI_CLIENT_LL_SUPPORT
	wmi_service[wmi_service_configure_multi_client_ll_support] =
				WMI_SERVICE_MULTI_CLIENT_LL_SUPPORT;
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	wmi_service[wmi_service_5ghz_hi_rssi_roam_support] =
					WMI_SERVICE_5GHZ_HI_RSSI_ROAM_SUPPORT;
#endif
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
	wmi_wds_attach_tlv(wmi_handle);
	wmi_twt_attach_tlv(wmi_handle);
	wmi_extscan_attach_tlv(wmi_handle);
	wmi_smart_ant_attach_tlv(wmi_handle);
	wmi_dbr_attach_tlv(wmi_handle);
	wmi_atf_attach_tlv(wmi_handle);
	wmi_ap_attach_tlv(wmi_handle);
	wmi_bcn_attach_tlv(wmi_handle);
	wmi_ocb_attach_tlv(wmi_handle);
	wmi_nan_attach_tlv(wmi_handle);
	wmi_p2p_attach_tlv(wmi_handle);
	wmi_interop_issues_ap_attach_tlv(wmi_handle);
	wmi_dcs_attach_tlv(wmi_handle);
	wmi_roam_attach_tlv(wmi_handle);
	wmi_concurrency_attach_tlv(wmi_handle);
	wmi_pmo_attach_tlv(wmi_handle);
	wmi_sta_attach_tlv(wmi_handle);
	wmi_11ax_bss_color_attach_tlv(wmi_handle);
	wmi_fwol_attach_tlv(wmi_handle);
	wmi_vdev_attach_tlv(wmi_handle);
	wmi_cfr_attach_tlv(wmi_handle);
	wmi_cp_stats_attach_tlv(wmi_handle);
	wmi_gpio_attach_tlv(wmi_handle);
	wmi_11be_attach_tlv(wmi_handle);
	wmi_coap_attach_tlv(wmi_handle);
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
