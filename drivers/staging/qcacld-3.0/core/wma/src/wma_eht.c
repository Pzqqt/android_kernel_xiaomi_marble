/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wma_eht.c
 *
 * WLAN Host Device Driver 802.11be - Extremely High Throughput Implementation
 */

#include "wma_eht.h"
#include "wmi_unified.h"
#include "service_ready_param.h"
#include "target_if.h"
#include "wma_internal.h"

#if defined(WLAN_FEATURE_11BE)
/**
 * wma_convert_eht_cap() - convert EHT capabilities into dot11f structure
 * @eht_cap: pointer to dot11f structure
 * @mac_cap: Received EHT MAC capability
 * @phy_cap: Received EHT PHY capability
 *
 * This function converts various EHT capability received as part of extended
 * service ready event into dot11f structure.
 *
 * Return: None
 */
static void wma_convert_eht_cap(tDot11fIEeht_cap *eht_cap, uint32_t *mac_cap,
				uint32_t *phy_cap)
{
	eht_cap->present = true;

	/* EHT MAC capabilities */
	eht_cap->nsep_pri_access = WMI_EHTCAP_MAC_NSEPPRIACCESS_GET(mac_cap);
	eht_cap->eht_om_ctl = WMI_EHTCAP_MAC_EHTOMCTRL_GET(mac_cap);
	eht_cap->triggered_txop_sharing = WMI_EHTCAP_MAC_TRIGTXOP_GET(mac_cap);

	/* EHT PHY capabilities */
	eht_cap->support_320mhz_6ghz = WMI_EHTCAP_PHY_320MHZIN6GHZ_GET(phy_cap);
	eht_cap->ru_242tone_wt_20mhz = WMI_EHTCAP_PHY_242TONERUBWLT20MHZ_GET(
			phy_cap);
	eht_cap->ndp_4x_eht_ltf_3dot2_us_gi =
		WMI_EHTCAP_PHY_NDP4XEHTLTFAND320NSGI_GET(phy_cap);
	eht_cap->partial_bw_mu_mimo = WMI_EHTCAP_PHY_PARTIALBWULMU_GET(phy_cap);
	eht_cap->su_beamformer = WMI_EHTCAP_PHY_SUBFMR_GET(phy_cap);
	eht_cap->su_beamformee = WMI_EHTCAP_PHY_SUBFME_GET(phy_cap);
	eht_cap->bfee_ss_le_80mhz = WMI_EHTCAP_PHY_BFMESSLT80MHZ_GET(phy_cap);
	eht_cap->bfee_ss_160mhz = WMI_EHTCAP_PHY_BFMESS160MHZ_GET(phy_cap);
	eht_cap->bfee_ss_320mhz = WMI_EHTCAP_PHY_BFMESS320MHZ_GET(phy_cap);
	eht_cap->num_sounding_dim_le_80mhz = WMI_EHTCAP_PHY_NUMSOUNDLT80MHZ_GET(
			phy_cap);
	eht_cap->num_sounding_dim_160mhz = WMI_EHTCAP_PHY_NUMSOUND160MHZ_GET(
			phy_cap);
	eht_cap->num_sounding_dim_320mhz = WMI_EHTCAP_PHY_NUMSOUND320MHZ_GET(
			phy_cap);
	eht_cap->ng_16_su_feedback = WMI_EHTCAP_PHY_NG16SUFB_GET(phy_cap);
	eht_cap->ng_16_mu_feedback = WMI_EHTCAP_PHY_NG16MUFB_GET(phy_cap);
	eht_cap->cb_sz_4_2_su_feedback = WMI_EHTCAP_PHY_CODBK42SUFB_GET(
			phy_cap);
	eht_cap->cb_sz_7_5_su_feedback = WMI_EHTCAP_PHY_CODBK75MUFB_GET(
			phy_cap);
	eht_cap->trig_su_bforming_feedback = WMI_EHTCAP_PHY_TRIGSUBFFB_GET(
			phy_cap);
	eht_cap->trig_mu_bforming_partial_bw_feedback =
		WMI_EHTCAP_PHY_TRIGMUBFPARTBWFB_GET(phy_cap);
	eht_cap->triggered_cqi_feedback = WMI_EHTCAP_PHY_TRIGCQIFB_GET(phy_cap);
	eht_cap->partial_bw_dl_mu_mimo = WMI_EHTCAP_PHY_PARTBWDLMUMIMO_GET(
			phy_cap);
	eht_cap->psr_based_sr = WMI_EHTCAP_PHY_PSRSR_GET(phy_cap);
	eht_cap->power_boost_factor = WMI_EHTCAP_PHY_PWRBSTFACTOR_GET(phy_cap);
	eht_cap->eht_mu_ppdu_4x_ltf_0_8_us_gi =
		WMI_EHTCAP_PHY_4XEHTLTFAND800NSGI_GET(phy_cap);
	eht_cap->max_nc = WMI_EHTCAP_PHY_MAXNC_GET(phy_cap);
	eht_cap->non_trig_cqi_feedback = WMI_EHTCAP_PHY_NONTRIGCQIFB_GET(
			phy_cap);
	eht_cap->tx_1024_4096_qam_lt_242_tone_ru =
		WMI_EHTCAP_PHY_TX1024AND4096QAMLS242TONERU_GET(phy_cap);
	eht_cap->rx_1024_4096_qam_lt_242_tone_ru =
		WMI_EHTCAP_PHY_RX1024AND4096QAMLS242TONERU_GET(phy_cap);
	eht_cap->ppet_present = WMI_EHTCAP_PHY_PPETHRESPRESENT_GET(phy_cap);
	eht_cap->common_nominal_pkt_padding = WMI_EHTCAP_PHY_CMNNOMPKTPAD_GET(
			phy_cap);
	eht_cap->max_num_eht_ltf = WMI_EHTCAP_PHY_MAXNUMEHTLTF_GET(phy_cap);
	eht_cap->mcs_15 = WMI_EHTCAP_PHY_SUPMCS15_GET(phy_cap);
	eht_cap->eht_dup_6ghz = WMI_EHTCAP_PHY_EHTDUPIN6GHZ_GET(phy_cap);
	eht_cap->op_sta_rx_ndp_wider_bw_20mhz =
		WMI_EHTCAP_PHY_20MHZOPSTARXNDPWIDERBW_GET(phy_cap);
	eht_cap->non_ofdma_ul_mu_mimo_le_80mhz =
		WMI_EHTCAP_PHY_NONOFDMAULMUMIMOLT80MHZ_GET(phy_cap);
	eht_cap->non_ofdma_ul_mu_mimo_160mhz =
		WMI_EHTCAP_PHY_NONOFDMAULMUMIMO160MHZ_GET(phy_cap);
	eht_cap->non_ofdma_ul_mu_mimo_320mhz =
		WMI_EHTCAP_PHY_NONOFDMAULMUMIMO320MHZ_GET(phy_cap);
	eht_cap->mu_bformer_le_80mhz = WMI_EHTCAP_PHY_MUBFMRLT80MHZ_GET(
			phy_cap);
	eht_cap->mu_bformer_160mhz = WMI_EHTCAP_PHY_MUBFMR160MHZ_GET(phy_cap);
	eht_cap->mu_bformer_320mhz = WMI_EHTCAP_PHY_MUBFMR320MHZ_GET(phy_cap);

	/* TODO: MCS map and PPET */
}

void wma_eht_update_tgt_services(struct wmi_unified *wmi_handle,
				 struct wma_tgt_services *cfg)
{
	if (wmi_service_enabled(wmi_handle, wmi_service_11be)) {
		cfg->en_11be = true;
		wma_set_fw_wlan_feat_caps(DOT11BE);
		wma_debug("11be is enabled");
	} else {
		wma_debug("11be is not enabled");
	}
}

void wma_update_target_ext_eht_cap(struct target_psoc_info *tgt_hdl,
				   struct wma_tgt_cfg *tgt_cfg)
{
	tDot11fIEeht_cap *eht_cap = &tgt_cfg->eht_cap;
	tDot11fIEeht_cap *eht_cap_2g = &tgt_cfg->eht_cap_2g;
	tDot11fIEeht_cap *eht_cap_5g = &tgt_cfg->eht_cap_5g;
	int i, num_hw_modes, total_mac_phy_cnt;
	tDot11fIEeht_cap eht_cap_mac;
	struct wlan_psoc_host_mac_phy_caps_ext2 *mac_cap, *mac_phy_cap;
	struct wlan_psoc_host_mac_phy_caps *host_cap;
	uint32_t supported_bands;

	qdf_mem_zero(eht_cap_2g, sizeof(tDot11fIEeht_cap));
	qdf_mem_zero(eht_cap_5g, sizeof(tDot11fIEeht_cap));
	num_hw_modes = target_psoc_get_num_hw_modes(tgt_hdl);
	mac_phy_cap = target_psoc_get_mac_phy_cap_ext2(tgt_hdl);
	host_cap = target_psoc_get_mac_phy_cap(tgt_hdl);
	total_mac_phy_cnt = target_psoc_get_total_mac_phy_cnt(tgt_hdl);
	if (!mac_phy_cap || !host_cap) {
		wma_err("Invalid MAC PHY capabilities handle");
		eht_cap->present = false;
		return;
	}

	if (!num_hw_modes) {
		wma_err("No extended EHT cap for current SOC");
		eht_cap->present = false;
		return;
	}

	if (!tgt_cfg->services.en_11be) {
		wma_info("Target does not support 11BE");
		eht_cap->present = false;
		return;
	}

	supported_bands = host_cap->supported_bands;
	for (i = 0; i < total_mac_phy_cnt; i++) {
		qdf_mem_zero(&eht_cap_mac, sizeof(tDot11fIEeht_cap));
		mac_cap = &mac_phy_cap[i];
		if (supported_bands & WLAN_2G_CAPABILITY) {
			wma_convert_eht_cap(&eht_cap_mac,
					    mac_cap->eht_cap_info_2G,
					    mac_cap->eht_cap_phy_info_2G);
			wma_convert_eht_cap(eht_cap_2g,
					    mac_cap->eht_cap_info_2G,
					    mac_cap->eht_cap_phy_info_2G);
		}

		if (supported_bands & WLAN_5G_CAPABILITY) {
			qdf_mem_zero(&eht_cap_mac, sizeof(tDot11fIEeht_cap));
			wma_convert_eht_cap(&eht_cap_mac,
					    mac_cap->eht_cap_info_5G,
					    mac_cap->eht_cap_phy_info_5G);
			wma_convert_eht_cap(eht_cap_5g,
					    mac_cap->eht_cap_info_5G,
					    mac_cap->eht_cap_phy_info_5G);
		}
	}
	qdf_mem_copy(eht_cap, &eht_cap_mac, sizeof(tDot11fIEeht_cap));
	wma_print_eht_cap(eht_cap);
}

void wma_update_vdev_eht_ops(uint32_t *eht_ops, tDot11fIEeht_op *eht_op)
{
}

void wma_print_eht_cap(tDot11fIEeht_cap *eht_cap)
{
	if (!eht_cap->present)
		return;

	wma_debug("EHT Capabilities:");

	/* EHT MAC Capabilities */
	wma_nofl_debug("\tNSEP Priority Access: 0x%01x",
		       eht_cap->nsep_pri_access);
	wma_nofl_debug("\tOM Control: 0x%01x", eht_cap->eht_om_ctl);
	wma_nofl_debug("\tTriggered TXOP Sharing: 0x%01x",
		       eht_cap->triggered_txop_sharing);

	/* EHT PHY Capabilities */
	wma_nofl_debug("\t320 MHz In 6 GHz: 0x%01x",
		       eht_cap->support_320mhz_6ghz);
	wma_nofl_debug("\t242-tone RU In BW Wider Than 20 MHz: 0x%01x",
		       eht_cap->ru_242tone_wt_20mhz);
	wma_nofl_debug("\tNDP With 4x EHT-LTF And 3.2 us GI: 0x%01x",
		       eht_cap->ndp_4x_eht_ltf_3dot2_us_gi);
	wma_nofl_debug("\tPartial Bandwidth UL MU-MIMO: 0x%01x",
		       eht_cap->partial_bw_mu_mimo);
	wma_nofl_debug("\tSU Beamformer: 0x%01x", eht_cap->su_beamformer);
	wma_nofl_debug("\tSU Beamformee: 0x%01x", eht_cap->su_beamformee);
	wma_nofl_debug("\tBeamformee SS <= 80 MHz: 0x%03x",
		       eht_cap->bfee_ss_le_80mhz);
	wma_nofl_debug("\tBeamformee SS = 160 MHz: 0x%03x",
		       eht_cap->bfee_ss_160mhz);
	wma_nofl_debug("\tBeamformee SS = 320 MHz: 0x%03x",
		       eht_cap->bfee_ss_320mhz);
	wma_nofl_debug("\tNumber Of Sounding Dimensions <= 80 MHz: 0x%03x",
		       eht_cap->num_sounding_dim_le_80mhz);
	wma_nofl_debug("\tNumber Of Sounding Dimensions = 160 MHz: 0x%03x",
		       eht_cap->num_sounding_dim_160mhz);
	wma_nofl_debug("\tNumber Of Sounding Dimensions = 320 MHz: 0x%03x",
		       eht_cap->num_sounding_dim_320mhz);
	wma_nofl_debug("\tNg = 16 SU Feedback: 0x%01x",
		       eht_cap->ng_16_su_feedback);
	wma_nofl_debug("\tNg = 16 MU Feedback: 0x%01x",
		       eht_cap->ng_16_mu_feedback);
	wma_nofl_debug("\tCodebook Size 4 2 SU Feedback: 0x%01x",
		       eht_cap->cb_sz_4_2_su_feedback);
	wma_nofl_debug("\tCodebook Size 7 5 MU Feedback: 0x%01x",
		       eht_cap->cb_sz_7_5_su_feedback);
	wma_nofl_debug("\tTriggered SU Beamforming Feedback: 0x%01x",
		       eht_cap->trig_su_bforming_feedback);
	wma_nofl_debug("\tTriggered MU Beamforming Partial BW Feedback: 0x%01x",
		       eht_cap->trig_mu_bforming_partial_bw_feedback);
	wma_nofl_debug("\tTriggered CQI Feedback: 0x%01x",
		       eht_cap->triggered_cqi_feedback);
	wma_nofl_debug("\tPartial Bandwidth DL MU-MIMO: 0x%01x",
		       eht_cap->partial_bw_dl_mu_mimo);
	wma_nofl_debug("\tPSR-Based SR: 0x%01x", eht_cap->psr_based_sr);
	wma_nofl_debug("\tPower Boost Factor: 0x%01x",
		       eht_cap->power_boost_factor);
	wma_nofl_debug("\tMU PPDU With 4x EHT-LTF 0.8 us GI: 0x%01x",
		       eht_cap->eht_mu_ppdu_4x_ltf_0_8_us_gi);
	wma_nofl_debug("\tMax Nc: 0x%04x", eht_cap->max_nc);
	wma_nofl_debug("\tNon-Triggered CQI Feedback: 0x%01x",
		       eht_cap->non_trig_cqi_feedback);
	wma_nofl_debug("\tTx 1024-QAM 4096-QAM < 242-tone RU: 0x%01x",
		       eht_cap->tx_1024_4096_qam_lt_242_tone_ru);
	wma_nofl_debug("\tRx 1024-QAM 4096-QAM < 242-tone RU: 0x%01x",
		       eht_cap->rx_1024_4096_qam_lt_242_tone_ru);
	wma_nofl_debug("\tPPE Thresholds Present: 0x%01x",
		       eht_cap->ppet_present);
	wma_nofl_debug("\tCommon Nominal Packet Padding: 0x%02x",
		       eht_cap->common_nominal_pkt_padding);
	wma_nofl_debug("\tMaximum Number Of Supported EHT-LTFs: 0x%05x",
		       eht_cap->max_num_eht_ltf);
	wma_nofl_debug("\tSupport of MCS 15: 0x%04x", eht_cap->mcs_15);
	wma_nofl_debug("\tSupport Of EHT DUP In 6 GHz: 0x%01x",
		       eht_cap->eht_dup_6ghz);
	wma_nofl_debug("\t20 MHz STA RX NDP With Wider BW: 0x%01x",
		       eht_cap->op_sta_rx_ndp_wider_bw_20mhz);
	wma_nofl_debug("\tNon-OFDMA UL MU-MIMO BW <= 80 MHz: 0x%01x",
		       eht_cap->non_ofdma_ul_mu_mimo_le_80mhz);
	wma_nofl_debug("\tNon-OFDMA UL MU-MIMO BW = 160 MHz: 0x%01x",
		       eht_cap->non_ofdma_ul_mu_mimo_160mhz);
	wma_nofl_debug("\tNon-OFDMA UL MU-MIMO BW = 320 MHz: 0x%01x",
		       eht_cap->non_ofdma_ul_mu_mimo_320mhz);
	wma_nofl_debug("\tMU Beamformer BW <= 80 MHz: 0x%01x",
		       eht_cap->mu_bformer_le_80mhz);
	wma_nofl_debug("\tMU Beamformer BW = 160 MHz: 0x%01x",
		       eht_cap->mu_bformer_160mhz);
	wma_nofl_debug("\tMU Beamformer BW = 320 MHz: 0x%01x",
		       eht_cap->mu_bformer_320mhz);
}

void wma_print_eht_phy_cap(uint32_t *phy_cap)
{
	wma_debug("EHT PHY Capabilities:");

	wma_nofl_debug("\t320 MHz In 6 GHz: 0x%01x",
		       WMI_EHTCAP_PHY_320MHZIN6GHZ_GET(phy_cap));
	wma_nofl_debug("\t242-tone RU In BW Wider Than 20 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_242TONERUBWLT20MHZ_GET(phy_cap));
	wma_nofl_debug("\tNDP With 4x EHT-LTF And 3.2 us GI: 0x%01x",
		       WMI_EHTCAP_PHY_NDP4XEHTLTFAND320NSGI_GET(phy_cap));
	wma_nofl_debug("\tPartial Bandwidth UL MU-MIMO: 0x%01x",
		       WMI_EHTCAP_PHY_PARTIALBWULMU_GET(phy_cap));
	wma_nofl_debug("\tSU Beamformer: 0x%01x",
		       WMI_EHTCAP_PHY_SUBFMR_GET(phy_cap));
	wma_nofl_debug("\tSU Beamformee: 0x%01x",
		       WMI_EHTCAP_PHY_SUBFME_GET(phy_cap));
	wma_nofl_debug("\tBeamformee SS <= 80 MHz: 0x%03x",
		       WMI_EHTCAP_PHY_BFMESSLT80MHZ_GET(phy_cap));
	wma_nofl_debug("\tBeamformee SS = 160 MHz: 0x%03x",
		       WMI_EHTCAP_PHY_BFMESS160MHZ_GET(phy_cap));
	wma_nofl_debug("\tBeamformee SS = 320 MHz: 0x%03x",
		       WMI_EHTCAP_PHY_BFMESS320MHZ_GET(phy_cap));
	wma_nofl_debug("\tNumber Of Sounding Dimensions <= 80 MHz: 0x%03x",
		       WMI_EHTCAP_PHY_NUMSOUNDLT80MHZ_GET(phy_cap));
	wma_nofl_debug("\tNumber Of Sounding Dimensions = 160 MHz: 0x%03x",
		       WMI_EHTCAP_PHY_NUMSOUND160MHZ_GET(phy_cap));
	wma_nofl_debug("\tNumber Of Sounding Dimensions = 320 MHz: 0x%03x",
		       WMI_EHTCAP_PHY_NUMSOUND320MHZ_GET(phy_cap));
	wma_nofl_debug("\tNg = 16 SU Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_NG16SUFB_GET(phy_cap));
	wma_nofl_debug("\tNg = 16 MU Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_NG16MUFB_GET(phy_cap));
	wma_nofl_debug("\tCodebook Size 4 2 SU Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_CODBK42SUFB_GET(phy_cap));
	wma_nofl_debug("\tCodebook Size 7 5 MU Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_CODBK75MUFB_GET(phy_cap));
	wma_nofl_debug("\tTriggered SU Beamforming Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_TRIGSUBFFB_GET(phy_cap));
	wma_nofl_debug("\tTriggered MU Beamforming Partial BW Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_TRIGMUBFPARTBWFB_GET(phy_cap));
	wma_nofl_debug("\tTriggered CQI Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_TRIGCQIFB_GET(phy_cap));
	wma_nofl_debug("\tPartial Bandwidth DL MU-MIMO: 0x%01x",
		       WMI_EHTCAP_PHY_TRIGMUBFPARTBWFB_GET(phy_cap));
	wma_nofl_debug("\tPSR-Based SR: 0x%01x",
		       WMI_EHTCAP_PHY_PSRSR_GET(phy_cap));
	wma_nofl_debug("\tPower Boost Factor: 0x%01x",
		       WMI_EHTCAP_PHY_PWRBSTFACTOR_GET(phy_cap));
	wma_nofl_debug("\tMU PPDU With 4x EHT-LTF 0.8 us GI: 0x%01x",
		       WMI_EHTCAP_PHY_4XEHTLTFAND800NSGI_GET(phy_cap));
	wma_nofl_debug("\tMax Nc: 0x%04x", WMI_EHTCAP_PHY_MAXNC_GET(phy_cap));
	wma_nofl_debug("\tNon-Triggered CQI Feedback: 0x%01x",
		       WMI_EHTCAP_PHY_NONTRIGCQIFB_GET(phy_cap));
	wma_nofl_debug("\tTx 1024-QAM 4096-QAM < 242-tone RU: 0x%01x",
		       WMI_EHTCAP_PHY_TX1024AND4096QAMLS242TONERU_GET(phy_cap));
	wma_nofl_debug("\tRx 1024-QAM 4096-QAM < 242-tone RU: 0x%01x",
		       WMI_EHTCAP_PHY_RX1024AND4096QAMLS242TONERU_GET(phy_cap));
	wma_nofl_debug("\tPPE Thresholds Present: 0x%01x",
		       WMI_EHTCAP_PHY_PPETHRESPRESENT_GET(phy_cap));
	wma_nofl_debug("\tCommon Nominal Packet Padding: 0x%02x",
		       WMI_EHTCAP_PHY_CMNNOMPKTPAD_GET(phy_cap));
	wma_nofl_debug("\tMaximum Number Of Supported EHT-LTFs: 0x%05x",
		       WMI_EHTCAP_PHY_MAXNUMEHTLTF_GET(phy_cap));
	wma_nofl_debug("\tSupport of MCS 15: 0x%04x",
		       WMI_EHTCAP_PHY_SUPMCS15_GET(phy_cap));
	wma_nofl_debug("\tSupport Of EHT DUP In 6 GHz: 0x%01x",
		       WMI_EHTCAP_PHY_EHTDUPIN6GHZ_GET(phy_cap));
	wma_nofl_debug("\t20 MHz STA RX NDP With Wider BW: 0x%01x",
		       WMI_EHTCAP_PHY_20MHZOPSTARXNDPWIDERBW_GET(phy_cap));
	wma_nofl_debug("\tNon-OFDMA UL MU-MIMO BW <= 80 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_NONOFDMAULMUMIMOLT80MHZ_GET(phy_cap));
	wma_nofl_debug("\tNon-OFDMA UL MU-MIMO BW = 160 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_NONOFDMAULMUMIMO160MHZ_GET(phy_cap));
	wma_nofl_debug("\tNon-OFDMA UL MU-MIMO BW = 320 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_NONOFDMAULMUMIMO320MHZ_GET(phy_cap));
	wma_nofl_debug("\tMU Beamformer BW <= 80 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_MUBFMRLT80MHZ_GET(phy_cap));
	wma_nofl_debug("\tMU Beamformer BW = 160 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_MUBFMR160MHZ_GET(phy_cap));
	wma_nofl_debug("\tMU Beamformer BW = 320 MHz: 0x%01x",
		       WMI_EHTCAP_PHY_MUBFMR320MHZ_GET(phy_cap));
}

void wma_print_eht_mac_cap(uint32_t *mac_cap)
{
	wma_debug("EHT MAC Capabilities:");

	wma_nofl_debug("\tNSEP Priority Access: 0x%01x",
		       WMI_EHTCAP_MAC_NSEPPRIACCESS_GET(mac_cap));
	wma_nofl_debug("\tOM Control: 0x%01x",
		       WMI_EHTCAP_MAC_EHTOMCTRL_GET(mac_cap));
	wma_nofl_debug("\tTriggered TXOP Sharing: 0x%01x",
		       WMI_EHTCAP_MAC_TRIGTXOP_GET(mac_cap));
}

void wma_print_eht_op(tDot11fIEeht_op *eht_ops)
{
}

void wma_populate_peer_eht_cap(struct peer_assoc_params *peer,
			       tpAddStaParams params)
{
	tDot11fIEeht_cap *eht_cap = &params->eht_config;
	uint32_t *phy_cap = peer->peer_eht_cap_phyinfo;
	uint32_t *mac_cap = peer->peer_eht_cap_macinfo;

	if (!params->eht_capable)
		return;

	peer->eht_flag = 1;
	peer->qos_flag = 1;

	/* EHT MAC Capabilities */
	WMI_EHTCAP_MAC_NSEPPRIACCESS_SET(mac_cap, eht_cap->nsep_pri_access);
	WMI_EHTCAP_MAC_EHTOMCTRL_SET(mac_cap, eht_cap->eht_om_ctl);
	WMI_EHTCAP_MAC_TRIGTXOP_SET(mac_cap, eht_cap->triggered_txop_sharing);

	/* EHT PHY Capabilities */
	WMI_EHTCAP_PHY_320MHZIN6GHZ_SET(phy_cap, eht_cap->support_320mhz_6ghz);
	WMI_EHTCAP_PHY_242TONERUBWLT20MHZ_SET(phy_cap,
					      eht_cap->ru_242tone_wt_20mhz);
	WMI_EHTCAP_PHY_NDP4XEHTLTFAND320NSGI_SET(
			phy_cap, eht_cap->ndp_4x_eht_ltf_3dot2_us_gi);
	WMI_EHTCAP_PHY_PARTIALBWULMU_SET(phy_cap, eht_cap->partial_bw_mu_mimo);
	WMI_EHTCAP_PHY_SUBFMR_SET(phy_cap, eht_cap->su_beamformer);
	WMI_EHTCAP_PHY_SUBFME_SET(phy_cap, eht_cap->su_beamformee);
	WMI_EHTCAP_PHY_BFMESSLT80MHZ_SET(phy_cap, eht_cap->bfee_ss_le_80mhz);
	WMI_EHTCAP_PHY_BFMESS160MHZ_SET(phy_cap, eht_cap->bfee_ss_160mhz);
	WMI_EHTCAP_PHY_BFMESS320MHZ_SET(phy_cap, eht_cap->bfee_ss_320mhz);
	WMI_EHTCAP_PHY_NUMSOUNDLT80MHZ_SET(
			phy_cap, eht_cap->num_sounding_dim_le_80mhz);
	WMI_EHTCAP_PHY_NUMSOUND160MHZ_SET(phy_cap,
					  eht_cap->num_sounding_dim_160mhz);
	WMI_EHTCAP_PHY_NUMSOUND320MHZ_SET(phy_cap,
					  eht_cap->num_sounding_dim_320mhz);
	WMI_EHTCAP_PHY_NG16SUFB_SET(phy_cap, eht_cap->ng_16_su_feedback);
	WMI_EHTCAP_PHY_NG16MUFB_SET(phy_cap, eht_cap->ng_16_mu_feedback);
	WMI_EHTCAP_PHY_CODBK42SUFB_SET(phy_cap, eht_cap->cb_sz_4_2_su_feedback);
	WMI_EHTCAP_PHY_CODBK75MUFB_SET(phy_cap, eht_cap->cb_sz_7_5_su_feedback);
	WMI_EHTCAP_PHY_TRIGSUBFFB_SET(phy_cap,
				      eht_cap->trig_su_bforming_feedback);
	WMI_EHTCAP_PHY_TRIGMUBFPARTBWFB_SET(
			phy_cap, eht_cap->trig_mu_bforming_partial_bw_feedback);
	WMI_EHTCAP_PHY_TRIGCQIFB_SET(phy_cap, eht_cap->triggered_cqi_feedback);
	WMI_EHTCAP_PHY_PARTBWDLMUMIMO_SET(phy_cap,
					  eht_cap->partial_bw_dl_mu_mimo);
	WMI_EHTCAP_PHY_PSRSR_SET(phy_cap, eht_cap->psr_based_sr);
	WMI_EHTCAP_PHY_PWRBSTFACTOR_SET(phy_cap, eht_cap->power_boost_factor);
	WMI_EHTCAP_PHY_4XEHTLTFAND800NSGI_SET(
			phy_cap, eht_cap->eht_mu_ppdu_4x_ltf_0_8_us_gi);
	WMI_EHTCAP_PHY_MAXNC_SET(phy_cap, eht_cap->max_nc);
	WMI_EHTCAP_PHY_NONTRIGCQIFB_SET(phy_cap,
					eht_cap->non_trig_cqi_feedback);
	WMI_EHTCAP_PHY_TX1024AND4096QAMLS242TONERU_SET(
			phy_cap, eht_cap->tx_1024_4096_qam_lt_242_tone_ru);
	WMI_EHTCAP_PHY_RX1024AND4096QAMLS242TONERU_SET(
			phy_cap, eht_cap->rx_1024_4096_qam_lt_242_tone_ru);
	WMI_EHTCAP_PHY_PPETHRESPRESENT_SET(phy_cap, eht_cap->ppet_present);
	WMI_EHTCAP_PHY_CMNNOMPKTPAD_SET(phy_cap,
					eht_cap->common_nominal_pkt_padding);
	WMI_EHTCAP_PHY_MAXNUMEHTLTF_SET(phy_cap, eht_cap->max_num_eht_ltf);
	WMI_EHTCAP_PHY_SUPMCS15_SET(phy_cap, eht_cap->mcs_15);
	WMI_EHTCAP_PHY_EHTDUPIN6GHZ_SET(phy_cap, eht_cap->eht_dup_6ghz);
	WMI_EHTCAP_PHY_20MHZOPSTARXNDPWIDERBW_SET(
			phy_cap, eht_cap->op_sta_rx_ndp_wider_bw_20mhz);
	WMI_EHTCAP_PHY_NONOFDMAULMUMIMOLT80MHZ_SET(
			phy_cap, eht_cap->non_ofdma_ul_mu_mimo_le_80mhz);
	WMI_EHTCAP_PHY_NONOFDMAULMUMIMO160MHZ_SET(
			phy_cap, eht_cap->non_ofdma_ul_mu_mimo_160mhz);
	WMI_EHTCAP_PHY_NONOFDMAULMUMIMO320MHZ_SET(
			phy_cap, eht_cap->non_ofdma_ul_mu_mimo_320mhz);
	WMI_EHTCAP_PHY_MUBFMRLT80MHZ_SET(phy_cap, eht_cap->mu_bformer_le_80mhz);
	WMI_EHTCAP_PHY_MUBFMR160MHZ_SET(phy_cap, eht_cap->mu_bformer_160mhz);
	WMI_EHTCAP_PHY_MUBFMR320MHZ_SET(phy_cap, eht_cap->mu_bformer_320mhz);

	qdf_mem_copy(peer->peer_eht_rx_mcs_set, peer->peer_he_rx_mcs_set,
		     sizeof(peer->peer_he_rx_mcs_set));
	qdf_mem_copy(peer->peer_eht_tx_mcs_set, peer->peer_he_tx_mcs_set,
		     sizeof(peer->peer_he_tx_mcs_set));

	peer->peer_eht_mcs_count = peer->peer_he_mcs_count;

	wma_print_eht_cap(eht_cap);
	wma_debug("Peer EHT Capabilities:");
	wma_print_eht_phy_cap(phy_cap);
	wma_print_eht_mac_cap(mac_cap);
}

void wma_vdev_set_eht_bss_params(tp_wma_handle wma, uint8_t vdev_id,
				 struct vdev_mlme_eht_ops_info *eht_info)
{
	if (!eht_info->eht_ops)
		return;
}

QDF_STATUS wma_get_eht_capabilities(struct eht_capability *eht_cap)
{
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_copy(eht_cap->phy_cap,
		     &wma_handle->eht_cap.phy_cap,
		     WMI_MAX_EHTCAP_PHY_SIZE);
	eht_cap->mac_cap = wma_handle->eht_cap.mac_cap;
	return QDF_STATUS_SUCCESS;
}

void wma_set_peer_assoc_params_bw_320(struct peer_assoc_params *params,
				      enum phy_ch_width ch_width)
{
	if (ch_width == CH_WIDTH_320MHZ)
		params->bw_320 = 1;
}

void wma_set_eht_txbf_cfg(struct mac_context *mac, uint8_t vdev_id)
{
	wma_set_eht_txbf_params(
		vdev_id, mac->mlme_cfg->eht_caps.dot11_eht_cap.su_beamformer,
		mac->mlme_cfg->eht_caps.dot11_eht_cap.su_beamformee,
		mac->mlme_cfg->eht_caps.dot11_eht_cap.mu_bformer_le_80mhz ||
		mac->mlme_cfg->eht_caps.dot11_eht_cap.mu_bformer_160mhz ||
		mac->mlme_cfg->eht_caps.dot11_eht_cap.mu_bformer_320mhz);
}

void wma_set_eht_txbf_params(uint8_t vdev_id, bool su_bfer,
			     bool su_bfee, bool mu_bfer)
{
	uint32_t ehtmu_mode = 0;
	QDF_STATUS status;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma)
		return;

	if (su_bfer)
		WMI_VDEV_EHT_SUBFER_ENABLE(ehtmu_mode);
	if (su_bfee) {
		WMI_VDEV_EHT_SUBFEE_ENABLE(ehtmu_mode);
		WMI_VDEV_EHT_MUBFEE_ENABLE(ehtmu_mode);
	}
	if (mu_bfer)
		WMI_VDEV_EHT_MUBFER_ENABLE(ehtmu_mode);

	WMI_VDEV_EHT_DLOFDMA_ENABLE(ehtmu_mode);
	WMI_VDEV_EHT_ULOFDMA_ENABLE(ehtmu_mode);

	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
				    WMI_VDEV_PARAM_SET_EHT_MU_MODE, ehtmu_mode);
	wma_debug("set EHTMU_MODE (ehtmu_mode = 0x%x)", ehtmu_mode);

	if (QDF_IS_STATUS_ERROR(status))
		wma_err("failed to set EHTMU_MODE(status = %d)", status);
}
#endif
