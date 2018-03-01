/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wma_he.c
 *
 * WLAN Host Device Driver 802.11ax - High Efficiency Implementation
 */

#include "wma_he.h"
#include "wmi_unified_api.h"
#include "cds_utils.h"
#include "wma_internal.h"

/**
 * wma_he_ppet_merge() - Merge PPET8 and PPET16 for a given ru and nss
 * @host_ppet: pointer to dot11f array
 * @byte_idx_p: pointer to byte index position where ppet should be added
 * @used_p: pointer to used position
 * @ppet: incoming PPET to be merged
 *
 * This function does the actual packing of dot11f structure. Split the
 * incoming PPET(3 bits) to fit into an octet. If there are more than
 * 3 bits available in a given byte_idx no splitting is required.
 *
 * Return: None
 */
static void wma_he_ppet_merge(uint8_t *host_ppet, int *byte_idx_p, int *used_p,
			      uint8_t ppet)
{
	int byte_idx = *byte_idx_p, used = *used_p;
	int lshift, rshift;

	if (used <= (HE_BYTE_SIZE - HE_PPET_SIZE)) {
		/* Enough space to fit the incoming PPET */
		lshift = used;
		host_ppet[byte_idx] |= (ppet << lshift);
		used += HE_PPET_SIZE;
		if (used == HE_BYTE_SIZE) {
			used = 0;
			byte_idx++;
		}
	} else {
		/* Need to split the PPET */
		lshift = used;
		rshift = HE_BYTE_SIZE - used;
		host_ppet[byte_idx] |= (ppet << lshift);
		byte_idx++;
		used = 0;
		host_ppet[byte_idx] |= (ppet >> rshift);
		used +=  HE_PPET_SIZE - rshift;
	}

	*byte_idx_p = byte_idx;
	*used_p = used;
}

/**
 * wma_he_populate_ppet() - Helper function for PPET conversion
 * @ppet: pointer to fw array
 * @nss: Number of NSS
 * @ru: Number of RU
 * @host_ppet: pointer to dot11f array
 * @req_byte: Number of bytes in dot11f array
 *
 * This function retrieves PPET16/PPET8 pair for combination of NSS/RU
 * and try to pack them in the OTA type dot11f structure by calling
 * wma_he_ppet_merge.
 *
 * Return: None
 */
static void wma_he_populate_ppet(uint32_t *ppet, int nss, int ru,
				 uint8_t *host_ppet, int req_byte)
{
	int byte_idx = 0, used, i, j;
	uint8_t ppet16, ppet8;

	WMA_LOGD(FL("nss: %d ru: %d req_byte: %d\n"), nss, ru, req_byte);
	/* NSS and RU_IDX are already populated */
	used = HE_PPET_NSS_RU_LEN;

	for (i = 0; i < nss; i++) {
		for (j = 1; j <= ru; j++) {
			ppet16 = WMI_GET_PPET16(ppet, j, i);
			ppet8 = WMI_GET_PPET8(ppet, j, i);
			WMA_LOGD(FL("ppet16 (nss:%d ru:%d): %04x"),
				 i, j, ppet16);
			WMA_LOGD(FL("ppet8 (nss:%d ru:%d): %04x"),
				 i, j, ppet8);
			wma_he_ppet_merge(host_ppet, &byte_idx, &used, ppet16);
			wma_he_ppet_merge(host_ppet, &byte_idx, &used, ppet8);
		}
	}

}

/**
 * wma_convert_he_ppet() - convert WMI ppet structure to dot11f structure
 * @he_ppet: pointer to dot11f ppet structure
 * @ppet: pointer to FW ppet structure
 *
 * PPET info coming from FW is stored as described in WMI definition. Convert
 * that into equivalent dot11f structure.
 *
 * Return: None
 */
static void wma_convert_he_ppet(uint8_t *he_ppet,
				struct wmi_host_ppe_threshold *ppet)
{
	int bits, req_byte;
	struct ppet_hdr *hdr;
	uint8_t ru_count, mask;
	struct ppe_threshold *ppet_1;

	ppet_1 = NULL;
	if (!ppet) {
		WMA_LOGE(FL("PPET is NULL"));
		qdf_mem_zero(he_ppet, HE_MAX_PPET_SIZE);
		return;
	}

	hdr = (struct ppet_hdr *)he_ppet;
	hdr->nss = ppet->numss_m1;
	hdr->ru_idx_mask = ppet->ru_bit_mask;
	mask = hdr->ru_idx_mask;
	for (ru_count = 0; mask; mask >>= 1)
		if (mask & 0x01)
			ru_count++;

	/*
	 * there will be two PPET for each NSS/RU pair
	 * PPET8 and PPET16, hence HE_PPET_SIZE * 2 bits for PPET
	 */
	bits = HE_PPET_NSS_RU_LEN + ((hdr->nss + 1) * ru_count) *
				     (HE_PPET_SIZE * 2);

	req_byte = (bits / HE_BYTE_SIZE) + 1;
	wma_he_populate_ppet(ppet->ppet16_ppet8_ru3_ru0, hdr->nss + 1,
			 ru_count, he_ppet, req_byte);
}

/**
 * wma_convert_he_cap() - convert HE capabilities into dot11f structure
 * @he_cap: pointer to dot11f structure
 * @mac_cap: Received HE MAC capability
 * @phy_cap: Received HE PHY capability
 * @supp_mcs: Max MCS supported (Tx/Rx)
 * @tx_chain_mask: Tx chain mask
 * @rx_chain_mask: Rx chain mask
 *
 * This function converts various HE capability received as part of extended
 * service ready event into dot11f structure. GET macros are defined at WMI
 * layer, use them to unpack the incoming FW capability.
 *
 * Return: None
 */
static void wma_convert_he_cap(tDot11fIEhe_cap *he_cap, uint32_t mac_cap,
			       uint32_t *phy_cap, uint32_t supp_mcs,
			       uint32_t tx_chain_mask, uint32_t rx_chain_mask)
{
	uint8_t nss, chan_width;
	uint16_t rx_mcs_le_80, tx_mcs_le_80, rx_mcs_160, tx_mcs_160;

	nss = QDF_MAX(wma_get_num_of_setbits_from_bitmask(tx_chain_mask),
			wma_get_num_of_setbits_from_bitmask(rx_chain_mask));

	he_cap->present = true;
	/* HE MAC capabilities */
	he_cap->htc_he = WMI_HECAP_MAC_HECTRL_GET(mac_cap);
	he_cap->twt_request = WMI_HECAP_MAC_TWTREQ_GET(mac_cap);
	he_cap->twt_responder = WMI_HECAP_MAC_TWTRSP_GET(mac_cap);
	he_cap->fragmentation =  WMI_HECAP_MAC_HEFRAG_GET(mac_cap);
	he_cap->max_num_frag_msdu = WMI_HECAP_MAC_MAXFRAGMSDU_GET(mac_cap);
	he_cap->min_frag_size = WMI_HECAP_MAC_MINFRAGSZ_GET(mac_cap);
	he_cap->trigger_frm_mac_pad = WMI_HECAP_MAC_TRIGPADDUR_GET(mac_cap);
	he_cap->multi_tid_aggr = WMI_HECAP_MAC_ACKMTIDAMPDU_GET(mac_cap);
	he_cap->he_link_adaptation = WMI_HECAP_MAC_HELKAD_GET(mac_cap);
	he_cap->all_ack = WMI_HECAP_MAC_AACK_GET(mac_cap);
	he_cap->ul_mu_rsp_sched = WMI_HECAP_MAC_ULMURSP_GET(mac_cap);
	he_cap->a_bsr = WMI_HECAP_MAC_BSR_GET(mac_cap);
	he_cap->broadcast_twt = WMI_HECAP_MAC_BCSTTWT_GET(mac_cap);
	he_cap->ba_32bit_bitmap = WMI_HECAP_MAC_32BITBA_GET(mac_cap);
	he_cap->mu_cascade = WMI_HECAP_MAC_MUCASCADE_GET(mac_cap);
	he_cap->ack_enabled_multitid = WMI_HECAP_MAC_ACKMTIDAMPDU_GET(mac_cap);
	he_cap->dl_mu_ba = WMI_HECAP_MAC_GROUPMSTABA_GET(mac_cap);
	he_cap->omi_a_ctrl = WMI_HECAP_MAC_OMI_GET(mac_cap);
	he_cap->ofdma_ra = WMI_HECAP_MAC_OFDMARA_GET(mac_cap);
	he_cap->max_ampdu_len = WMI_HECAP_MAC_MAXAMPDULEN_EXP_GET(mac_cap);
	he_cap->amsdu_frag = WMI_HECAP_MAC_AMSDUFRAG_GET(mac_cap);
	he_cap->flex_twt_sched = WMI_HECAP_MAC_FLEXTWT_GET(mac_cap);
	he_cap->rx_ctrl_frame = WMI_HECAP_MAC_MBSS_GET(mac_cap);
	he_cap->bsrp_ampdu_aggr = WMI_HECAP_MAC_BSRPAMPDU_GET(mac_cap);
	he_cap->qtp = WMI_HECAP_MAC_QTP_GET(mac_cap);
	he_cap->a_bqr = WMI_HECAP_MAC_ABQR_GET(mac_cap);
	he_cap->sr_responder = WMI_HECAP_MAC_SRRESP_GET(mac_cap);
	he_cap->ndp_feedback_supp = WMI_HECAP_MAC_NDPFDBKRPT_GET(mac_cap);
	he_cap->ops_supp = WMI_HECAP_MAC_OPS_GET(mac_cap);
	he_cap->amsdu_in_ampdu = WMI_HECAP_MAC_AMSDUINAMPDU_GET(mac_cap);
	/* HE PHY capabilities */
	he_cap->dual_band = WMI_HECAP_PHY_DB_GET(phy_cap);
	chan_width = WMI_HECAP_PHY_CBW_GET(phy_cap);
	he_cap->chan_width_0 = HE_CH_WIDTH_GET_BIT(chan_width, 0);
	he_cap->chan_width_1 = HE_CH_WIDTH_GET_BIT(chan_width, 1);
	he_cap->chan_width_2 = HE_CH_WIDTH_GET_BIT(chan_width, 2);
	he_cap->chan_width_3 = HE_CH_WIDTH_GET_BIT(chan_width, 3);
	he_cap->chan_width_4 = HE_CH_WIDTH_GET_BIT(chan_width, 4);
	he_cap->chan_width_5 = HE_CH_WIDTH_GET_BIT(chan_width, 5);
	he_cap->chan_width_6 = HE_CH_WIDTH_GET_BIT(chan_width, 6);
	he_cap->rx_pream_puncturing = WMI_HECAP_PHY_PREAMBLEPUNCRX_GET(phy_cap);
	he_cap->device_class = WMI_HECAP_PHY_COD_GET(phy_cap);
	he_cap->ldpc_coding = WMI_HECAP_PHY_LDPC_GET(phy_cap);
	he_cap->he_1x_ltf_800_gi_ppdu = WMI_HECAP_PHY_LTFGIFORHE_GET(phy_cap);
	he_cap->midamble_rx_max_nsts =
		WMI_HECAP_PHY_MIDAMBLERXMAXNSTS_GET(phy_cap);
	he_cap->he_4x_ltf_3200_gi_ndp = WMI_HECAP_PHY_LTFGIFORNDP_GET(phy_cap);
	he_cap->rx_stbc_lt_80mhz = WMI_HECAP_PHY_RXSTBC_GET(phy_cap);
	he_cap->tx_stbc_lt_80mhz = WMI_HECAP_PHY_TXSTBC_GET(phy_cap);
	he_cap->rx_stbc_gt_80mhz = WMI_HECAP_PHY_STBCRXGT80_GET(phy_cap);
	he_cap->tx_stbc_gt_80mhz = WMI_HECAP_PHY_STBCTXGT80_GET(phy_cap);

	he_cap->doppler = (WMI_HECAP_PHY_RXDOPPLER_GET(phy_cap) << 1) |
				WMI_HECAP_PHY_TXDOPPLER_GET(phy_cap);
	he_cap->ul_mu = (WMI_HECAP_PHY_ULMUMIMOOFDMA_GET(phy_cap) << 1) |
			 WMI_HECAP_PHY_UL_MU_MIMO_GET(phy_cap);
	he_cap->dcm_enc_tx = WMI_HECAP_PHY_DCMTX_GET(phy_cap);
	he_cap->dcm_enc_rx = WMI_HECAP_PHY_DCMRX_GET(phy_cap);
	he_cap->ul_he_mu = WMI_HECAP_PHY_ULHEMU_GET(phy_cap);
	he_cap->su_beamformer = WMI_HECAP_PHY_SUBFMR_GET(phy_cap);
	he_cap->su_beamformee = WMI_HECAP_PHY_SUBFME_GET(phy_cap);
	he_cap->mu_beamformer = WMI_HECAP_PHY_MUBFMR_GET(phy_cap);
	he_cap->bfee_sts_lt_80 = WMI_HECAP_PHY_SUBFMESTS_GET(phy_cap);
	he_cap->bfee_sts_gt_80 = WMI_HECAP_PHY_BFMESTSGT80MHZ_GET(phy_cap);
	he_cap->num_sounding_lt_80 = WMI_HECAP_PHY_NUMSOUNDLT80MHZ_GET(phy_cap);
	he_cap->num_sounding_gt_80 = WMI_HECAP_PHY_NUMSOUNDGT80MHZ_GET(phy_cap);
	he_cap->su_feedback_tone16 =
		WMI_HECAP_PHY_NG16SUFEEDBACKLT80_GET(phy_cap);
	he_cap->mu_feedback_tone16 =
		WMI_HECAP_PHY_NG16MUFEEDBACKGT80_GET(phy_cap);
	he_cap->codebook_su = WMI_HECAP_PHY_CODBK42SU_GET(phy_cap);
	he_cap->codebook_mu = WMI_HECAP_PHY_CODBK75MU_GET(phy_cap);
	he_cap->beamforming_feedback =
		WMI_HECAP_PHY_BFFEEDBACKTRIG_GET(phy_cap);
	he_cap->he_er_su_ppdu = WMI_HECAP_PHY_HEERSU_GET(phy_cap);
	he_cap->dl_mu_mimo_part_bw =
		WMI_HECAP_PHY_DLMUMIMOPARTIALBW_GET(phy_cap);
	he_cap->ppet_present = WMI_HECAP_PHY_PETHRESPRESENT_GET(phy_cap);
	he_cap->srp = WMI_HECAP_PHY_SRPSPRESENT_GET(phy_cap);
	he_cap->power_boost = WMI_HECAP_PHY_PWRBOOSTAR_GET(phy_cap);
	he_cap->he_ltf_800_gi_4x =
			WMI_HECAP_PHY_4XLTFAND800NSECSGI_GET(phy_cap);
	he_cap->max_nc = WMI_HECAP_PHY_MAXNC_GET(phy_cap);
	he_cap->er_he_ltf_800_gi_4x =
			WMI_HECAP_PHY_ERSU4X800NSECGI_GET(phy_cap);
	he_cap->he_ppdu_20_in_40Mhz_2G =
		WMI_HECAP_PHY_HEPPDU20IN40MHZ2G_GET(phy_cap);
	he_cap->he_ppdu_20_in_160_80p80Mhz =
		WMI_HECAP_PHY_HEPPDU20IN160OR80P80MHZ_GET(phy_cap);
	he_cap->he_ppdu_80_in_160_80p80Mhz =
		WMI_HECAP_PHY_HEPPDU80IN160OR80P80MHZ_GET(phy_cap);
	he_cap->er_1x_he_ltf_gi = WMI_HECAP_PHY_ERSU1X800NSECGI_GET(phy_cap);
	he_cap->midamble_rx_1x_he_ltf =
		WMI_HECAP_PHY_MIDAMBLERX2XAND1XHELTF_GET(phy_cap);

	/*
	 * supp_mcs is split into 16 bits with lower indicating le_80 and
	 * upper indicating 160 and 80_80.
	 */
	WMA_LOGD(FL("supported_mcs: 0x%08x\n"), supp_mcs);
	rx_mcs_le_80 = supp_mcs & 0xFFFF;
	tx_mcs_le_80 = supp_mcs & 0xFFFF;
	rx_mcs_160 = (supp_mcs & 0xFFFF0000) >> 16;
	tx_mcs_160 = (supp_mcs & 0xFFFF0000) >> 16;
	/* if FW indicated it is using 1x1 disable upper NSS-MCS sets */
	if (nss == NSS_1x1_MODE) {
		rx_mcs_le_80 |= HE_MCS_INV_MSK_4_NSS(1);
		tx_mcs_le_80 |= HE_MCS_INV_MSK_4_NSS(1);
		rx_mcs_160 |= HE_MCS_INV_MSK_4_NSS(1);
		tx_mcs_160 |= HE_MCS_INV_MSK_4_NSS(1);
	}
	he_cap->rx_he_mcs_map_lt_80 = rx_mcs_le_80;
	he_cap->tx_he_mcs_map_lt_80 = tx_mcs_le_80;
	*((uint16_t *)he_cap->tx_he_mcs_map_160) = rx_mcs_160;
	*((uint16_t *)he_cap->rx_he_mcs_map_160) = tx_mcs_160;
	*((uint16_t *)he_cap->rx_he_mcs_map_80_80) = rx_mcs_160;
	*((uint16_t *)he_cap->tx_he_mcs_map_80_80) = tx_mcs_160;
}

/**
 * wma_derive_ext_he_cap() - Derive HE caps based on given value
 * @he_cap: pointer to given HE caps to be filled
 * @new_he_cap: new HE cap info provided.
 *
 * This function takes the value provided in and combines it wht the incoming
 * HE capability. After decoding, what ever value it gets,
 * it takes the union(max) or intersection(min) with previously derived values.
 * Currently, intersection(min) is taken for all the capabilities.
 *
 * Return: none
 */
static void wma_derive_ext_he_cap(tDot11fIEhe_cap *he_cap,
				  tDot11fIEhe_cap *new_cap)
{
	uint16_t mcs_1, mcs_2;

	if (!he_cap->present) {
		/* First time update, copy the capability as is */
		qdf_mem_copy(he_cap, new_cap, sizeof(*he_cap));
		he_cap->present = true;
		return;
	}
	/* Take union(max) or intersection(min) of the capabilities */
	he_cap->htc_he = QDF_MIN(he_cap->htc_he, new_cap->htc_he);
	he_cap->twt_request = QDF_MIN(he_cap->twt_request,
			new_cap->twt_request);
	he_cap->twt_responder = QDF_MIN(he_cap->twt_responder,
			new_cap->twt_responder);
	he_cap->fragmentation = QDF_MIN(he_cap->fragmentation,
			new_cap->fragmentation);
	he_cap->max_num_frag_msdu = QDF_MIN(he_cap->max_num_frag_msdu,
			new_cap->max_num_frag_msdu);
	he_cap->min_frag_size = QDF_MIN(he_cap->min_frag_size,
			new_cap->min_frag_size);
	he_cap->trigger_frm_mac_pad =
		QDF_MIN(he_cap->trigger_frm_mac_pad,
				new_cap->trigger_frm_mac_pad);
	he_cap->multi_tid_aggr = QDF_MIN(he_cap->multi_tid_aggr,
			new_cap->multi_tid_aggr);
	he_cap->he_link_adaptation = QDF_MIN(he_cap->he_link_adaptation,
			new_cap->he_link_adaptation);
	he_cap->all_ack = QDF_MIN(he_cap->all_ack,
			new_cap->all_ack);
	he_cap->ul_mu_rsp_sched = QDF_MIN(he_cap->ul_mu_rsp_sched,
			new_cap->ul_mu_rsp_sched);
	he_cap->a_bsr = QDF_MIN(he_cap->a_bsr,
			new_cap->a_bsr);
	he_cap->broadcast_twt = QDF_MIN(he_cap->broadcast_twt,
			new_cap->broadcast_twt);
	he_cap->ba_32bit_bitmap = QDF_MIN(he_cap->ba_32bit_bitmap,
			new_cap->ba_32bit_bitmap);
	he_cap->mu_cascade = QDF_MIN(he_cap->mu_cascade,
			new_cap->mu_cascade);
	he_cap->ack_enabled_multitid =
		QDF_MIN(he_cap->ack_enabled_multitid,
				new_cap->ack_enabled_multitid);
	he_cap->dl_mu_ba = QDF_MIN(he_cap->dl_mu_ba,
			new_cap->dl_mu_ba);
	he_cap->omi_a_ctrl = QDF_MIN(he_cap->omi_a_ctrl,
			new_cap->omi_a_ctrl);
	he_cap->ofdma_ra = QDF_MIN(he_cap->ofdma_ra,
			new_cap->ofdma_ra);
	he_cap->max_ampdu_len = QDF_MIN(he_cap->max_ampdu_len,
			new_cap->max_ampdu_len);
	he_cap->amsdu_frag = QDF_MIN(he_cap->amsdu_frag,
			new_cap->amsdu_frag);
	he_cap->flex_twt_sched = QDF_MIN(he_cap->flex_twt_sched,
			new_cap->flex_twt_sched);
	he_cap->rx_ctrl_frame = QDF_MIN(he_cap->rx_ctrl_frame,
			new_cap->rx_ctrl_frame);
	he_cap->bsrp_ampdu_aggr = QDF_MIN(he_cap->bsrp_ampdu_aggr,
			new_cap->bsrp_ampdu_aggr);
	he_cap->qtp = QDF_MIN(he_cap->qtp, new_cap->qtp);
	he_cap->a_bqr = QDF_MIN(he_cap->a_bqr, new_cap->a_bqr);
	he_cap->sr_responder = QDF_MIN(he_cap->sr_responder,
			new_cap->sr_responder);
	he_cap->ndp_feedback_supp = QDF_MIN(he_cap->ndp_feedback_supp,
			new_cap->ndp_feedback_supp);
	he_cap->ops_supp = QDF_MIN(he_cap->ops_supp, new_cap->ops_supp);
	he_cap->amsdu_in_ampdu = QDF_MIN(he_cap->amsdu_in_ampdu,
			new_cap->amsdu_in_ampdu);
	he_cap->reserved1 = QDF_MIN(he_cap->reserved1,
			new_cap->reserved1);

	he_cap->dual_band = QDF_MIN(he_cap->dual_band,
			new_cap->dual_band);

	he_cap->chan_width_0 = he_cap->chan_width_0 | new_cap->chan_width_0;
	he_cap->chan_width_1 = he_cap->chan_width_1 | new_cap->chan_width_1;
	he_cap->chan_width_2 = he_cap->chan_width_2 | new_cap->chan_width_2;
	he_cap->chan_width_3 = he_cap->chan_width_3 | new_cap->chan_width_3;
	he_cap->chan_width_4 = he_cap->chan_width_4 | new_cap->chan_width_4;
	he_cap->chan_width_5 = he_cap->chan_width_5 | new_cap->chan_width_5;
	he_cap->chan_width_6 = he_cap->chan_width_6 | new_cap->chan_width_6;

	he_cap->rx_pream_puncturing =
		QDF_MIN(he_cap->rx_pream_puncturing,
				new_cap->rx_pream_puncturing);
	he_cap->device_class = QDF_MIN(he_cap->device_class,
			new_cap->device_class);
	he_cap->ldpc_coding = QDF_MIN(he_cap->ldpc_coding,
			new_cap->ldpc_coding);
	he_cap->he_1x_ltf_800_gi_ppdu =
		QDF_MIN(he_cap->he_1x_ltf_800_gi_ppdu,
				new_cap->he_1x_ltf_800_gi_ppdu);
	he_cap->midamble_rx_max_nsts =
		QDF_MIN(he_cap->midamble_rx_max_nsts,
				new_cap->midamble_rx_max_nsts);
	he_cap->he_4x_ltf_3200_gi_ndp =
		QDF_MIN(he_cap->he_4x_ltf_3200_gi_ndp,
				new_cap->he_4x_ltf_3200_gi_ndp);
	he_cap->tx_stbc_lt_80mhz = QDF_MIN(he_cap->tx_stbc_lt_80mhz,
			new_cap->tx_stbc_lt_80mhz);
	he_cap->rx_stbc_lt_80mhz = QDF_MIN(he_cap->rx_stbc_lt_80mhz,
			new_cap->rx_stbc_lt_80mhz);
	he_cap->doppler = QDF_MIN(he_cap->doppler,
			new_cap->doppler);
	he_cap->ul_mu = QDF_MIN(he_cap->ul_mu, new_cap->ul_mu);
	he_cap->dcm_enc_tx = QDF_MIN(he_cap->dcm_enc_tx,
			new_cap->dcm_enc_tx);
	he_cap->dcm_enc_rx = QDF_MIN(he_cap->dcm_enc_rx,
			new_cap->dcm_enc_rx);
	he_cap->ul_he_mu = QDF_MIN(he_cap->ul_he_mu, new_cap->ul_he_mu);
	he_cap->su_beamformer = QDF_MIN(he_cap->su_beamformer,
			new_cap->su_beamformer);
	he_cap->su_beamformee = QDF_MIN(he_cap->su_beamformee,
			new_cap->su_beamformee);
	he_cap->mu_beamformer = QDF_MIN(he_cap->mu_beamformer,
			new_cap->mu_beamformer);
	he_cap->bfee_sts_lt_80 = QDF_MIN(he_cap->bfee_sts_lt_80,
			new_cap->bfee_sts_lt_80);
	he_cap->bfee_sts_gt_80 = QDF_MIN(he_cap->bfee_sts_gt_80,
			new_cap->bfee_sts_gt_80);
	he_cap->num_sounding_lt_80 = QDF_MIN(he_cap->num_sounding_lt_80,
			new_cap->num_sounding_lt_80);
	he_cap->num_sounding_gt_80 = QDF_MIN(he_cap->num_sounding_gt_80,
			new_cap->num_sounding_gt_80);
	he_cap->su_feedback_tone16 = QDF_MIN(he_cap->su_feedback_tone16,
			new_cap->su_feedback_tone16);
	he_cap->mu_feedback_tone16 = QDF_MIN(he_cap->mu_feedback_tone16,
			new_cap->mu_feedback_tone16);
	he_cap->codebook_su = QDF_MIN(he_cap->codebook_su,
			new_cap->codebook_su);
	he_cap->codebook_mu = QDF_MIN(he_cap->codebook_mu,
			new_cap->codebook_mu);
	he_cap->beamforming_feedback =
		QDF_MIN(he_cap->beamforming_feedback,
				new_cap->beamforming_feedback);
	he_cap->he_er_su_ppdu = QDF_MIN(he_cap->he_er_su_ppdu,
			new_cap->he_er_su_ppdu);
	he_cap->dl_mu_mimo_part_bw = QDF_MIN(he_cap->dl_mu_mimo_part_bw,
			new_cap->dl_mu_mimo_part_bw);
	he_cap->ppet_present = QDF_MIN(he_cap->ppet_present,
			new_cap->ppet_present);
	he_cap->srp = QDF_MIN(he_cap->srp, new_cap->srp);
	he_cap->power_boost = QDF_MIN(he_cap->power_boost,
			new_cap->power_boost);
	he_cap->he_ltf_800_gi_4x = QDF_MIN(he_cap->he_ltf_800_gi_4x,
			new_cap->he_ltf_800_gi_4x);
	he_cap->er_he_ltf_800_gi_4x =
		QDF_MIN(he_cap->er_he_ltf_800_gi_4x,
				new_cap->er_he_ltf_800_gi_4x);
	he_cap->he_ppdu_20_in_40Mhz_2G =
		QDF_MIN(he_cap->he_ppdu_20_in_40Mhz_2G,
				new_cap->he_ppdu_20_in_40Mhz_2G);
	he_cap->he_ppdu_20_in_160_80p80Mhz =
		QDF_MIN(he_cap->he_ppdu_20_in_160_80p80Mhz,
				new_cap->he_ppdu_20_in_160_80p80Mhz);
	he_cap->he_ppdu_80_in_160_80p80Mhz =
		QDF_MIN(he_cap->he_ppdu_80_in_160_80p80Mhz,
				new_cap->he_ppdu_80_in_160_80p80Mhz);
	he_cap->er_1x_he_ltf_gi = QDF_MIN(he_cap->er_1x_he_ltf_gi,
			new_cap->er_1x_he_ltf_gi);
	he_cap->midamble_rx_1x_he_ltf =
		QDF_MIN(he_cap->midamble_rx_1x_he_ltf,
				new_cap->midamble_rx_1x_he_ltf);
	he_cap->reserved2 = QDF_MIN(he_cap->reserved2,
			new_cap->reserved2);

	/* take intersection for MCS map */
	mcs_1 = he_cap->rx_he_mcs_map_lt_80;
	mcs_2 = new_cap->rx_he_mcs_map_lt_80;
	he_cap->rx_he_mcs_map_lt_80 = HE_INTERSECT_MCS(mcs_1, mcs_2);
	mcs_1 = he_cap->tx_he_mcs_map_lt_80;
	mcs_2 = new_cap->tx_he_mcs_map_lt_80;
	he_cap->tx_he_mcs_map_lt_80 = HE_INTERSECT_MCS(mcs_1, mcs_2);
	mcs_1 = *((uint16_t *)he_cap->rx_he_mcs_map_160);
	mcs_2 = *((uint16_t *)new_cap->rx_he_mcs_map_160);
	*((uint16_t *)he_cap->rx_he_mcs_map_160) =
		HE_INTERSECT_MCS(mcs_1, mcs_2);
	mcs_1 = *((uint16_t *)he_cap->tx_he_mcs_map_160);
	mcs_2 = *((uint16_t *)new_cap->tx_he_mcs_map_160);
	*((uint16_t *)he_cap->tx_he_mcs_map_160) =
		HE_INTERSECT_MCS(mcs_1, mcs_2);
	mcs_1 = *((uint16_t *)he_cap->rx_he_mcs_map_80_80);
	mcs_2 = *((uint16_t *)new_cap->rx_he_mcs_map_80_80);
	*((uint16_t *)he_cap->rx_he_mcs_map_80_80) =
		HE_INTERSECT_MCS(mcs_1, mcs_2);
	mcs_1 = *((uint16_t *)he_cap->tx_he_mcs_map_80_80);
	mcs_2 = *((uint16_t *)new_cap->tx_he_mcs_map_80_80);
	*((uint16_t *)he_cap->tx_he_mcs_map_80_80) =
		HE_INTERSECT_MCS(mcs_1, mcs_2);
}

void wma_print_he_cap(tDot11fIEhe_cap *he_cap)
{
	uint8_t chan_width;
	struct ppet_hdr *hdr;

	if (!he_cap->present) {
		WMA_LOGI(FL("HE Capabilities not present"));
		return;
	}

	WMA_LOGD(FL("HE Capabilities:"));

	/* HE MAC capabilities */
	WMA_LOGD("\tHTC-HE conrol: 0x%01x", he_cap->htc_he);
	WMA_LOGD("\tTWT Requestor support: 0x%01x", he_cap->twt_request);
	WMA_LOGD("\tTWT Responder support: 0x%01x", he_cap->twt_responder);
	WMA_LOGD("\tFragmentation support: 0x%02x", he_cap->fragmentation);
	WMA_LOGD("\tMax no.of frag MSDUs: 0x%03x", he_cap->max_num_frag_msdu);
	WMA_LOGD("\tMin. frag size: 0x%02x", he_cap->min_frag_size);
	WMA_LOGD("\tTrigger MAC pad duration: 0x%02x",
			he_cap->trigger_frm_mac_pad);
	WMA_LOGD("\tMulti-TID aggr support: 0x%03x", he_cap->multi_tid_aggr);
	WMA_LOGD("\tLink adaptation: 0x%02x", he_cap->he_link_adaptation);
	WMA_LOGD("\tAll ACK support: 0x%01x", he_cap->all_ack);
	WMA_LOGD("\tUL MU resp. scheduling: 0x%01x", he_cap->ul_mu_rsp_sched);
	WMA_LOGD("\tA-Buff status report: 0x%01x", he_cap->a_bsr);
	WMA_LOGD("\tBroadcast TWT support: 0x%01x", he_cap->broadcast_twt);
	WMA_LOGD("\t32bit BA bitmap support: 0x%01x", he_cap->ba_32bit_bitmap);
	WMA_LOGD("\tMU Cascading support: 0x%01x", he_cap->mu_cascade);
	WMA_LOGD("\tACK enabled Multi-TID: 0x%01x",
			he_cap->ack_enabled_multitid);
	WMA_LOGD("\tMulti-STA BA in DL MU: 0x%01x", he_cap->dl_mu_ba);
	WMA_LOGD("\tOMI A-Control support: 0x%01x", he_cap->omi_a_ctrl);
	WMA_LOGD("\tOFDMA RA support: 0x%01x", he_cap->ofdma_ra);
	WMA_LOGD("\tMax A-MPDU Length: 0x%02x", he_cap->max_ampdu_len);
	WMA_LOGD("\tA-MSDU Fragmentation: 0x%01x", he_cap->amsdu_frag);
	WMA_LOGD("\tFlex. TWT sched support: 0x%01x", he_cap->flex_twt_sched);
	WMA_LOGD("\tRx Ctrl frame to MBSS: 0x%01x", he_cap->rx_ctrl_frame);
	WMA_LOGD("\tBSRP A-MPDU Aggregation: 0x%01x", he_cap->bsrp_ampdu_aggr);
	WMA_LOGD("\tQuite Time Period support: 0x%01x", he_cap->qtp);
	WMA_LOGD("\tA-BQR support: 0x%01x", he_cap->a_bqr);
	WMA_LOGD("\t SR Responder: 0x%01x", he_cap->sr_responder);
	WMA_LOGD("\t ndp feedback supp: 0x%01x", he_cap->ndp_feedback_supp);
	WMA_LOGD("\t ops supp: 0x%01x", he_cap->ops_supp);
	WMA_LOGD("\t amsdu in ampdu: 0x%01x", he_cap->amsdu_in_ampdu);

	/* HE PHY capabilities */
	WMA_LOGD("\tDual band support: 0x%01x", he_cap->dual_band);
	chan_width = HE_CH_WIDTH_COMBINE(he_cap->chan_width_0,
				he_cap->chan_width_1, he_cap->chan_width_2,
				he_cap->chan_width_3, he_cap->chan_width_4,
				he_cap->chan_width_5, he_cap->chan_width_6);

	WMA_LOGD("\tChannel width support: 0x%07x", chan_width);
	WMA_LOGD("\tPreamble puncturing Rx: 0x%04x",
			he_cap->rx_pream_puncturing);
	WMA_LOGD("\tClass of device: 0x%01x", he_cap->device_class);
	WMA_LOGD("\tLDPC coding support: 0x%01x", he_cap->ldpc_coding);
	WMA_LOGD("\tLTF and GI for HE PPDUs: 0x%02x",
		 he_cap->he_1x_ltf_800_gi_ppdu);
	WMA_LOGD("\tMidamble Rx MAX NSTS: 0x%02x",
		 he_cap->midamble_rx_max_nsts);
	WMA_LOGD("\tLTF and GI for NDP: 0x%02x", he_cap->he_4x_ltf_3200_gi_ndp);
	WMA_LOGD("\tSTBC Tx support <= 80M: 0x%01x", he_cap->tx_stbc_lt_80mhz);
	WMA_LOGD("\tSTBC Rx support <= 80M: 0x%01x", he_cap->rx_stbc_lt_80mhz);
	WMA_LOGD("\tDoppler support: 0x%02x", he_cap->doppler);
	WMA_LOGD("\tUL MU: 0x%02x", he_cap->ul_mu);
	WMA_LOGD("\tDCM encoding Tx: 0x%03x", he_cap->dcm_enc_tx);
	WMA_LOGD("\tDCM encoding Tx: 0x%03x", he_cap->dcm_enc_rx);
	WMA_LOGD("\tHE MU PPDU payload support: 0x%01x", he_cap->ul_he_mu);
	WMA_LOGD("\tSU Beamformer: 0x%01x", he_cap->su_beamformer);
	WMA_LOGD("\tSU Beamformee: 0x%01x", he_cap->su_beamformee);
	WMA_LOGD("\tMU Beamformer: 0x%01x", he_cap->mu_beamformer);
	WMA_LOGD("\tBeamformee STS for <= 80Mhz: 0x%03x",
			he_cap->bfee_sts_lt_80);
	WMA_LOGD("\tBeamformee STS for > 80Mhz: 0x%03x",
			he_cap->bfee_sts_gt_80);
	WMA_LOGD("\tNo. of sounding dim <= 80Mhz: 0x%03x",
			he_cap->num_sounding_lt_80);
	WMA_LOGD("\tNo. of sounding dim > 80Mhz: 0x%03x",
			he_cap->num_sounding_gt_80);
	WMA_LOGD("\tNg=16 for SU feedback support: 0x%01x",
			he_cap->su_feedback_tone16);
	WMA_LOGD("\tNg=16 for MU feedback support: 0x%01x",
			he_cap->mu_feedback_tone16);
	WMA_LOGD("\tCodebook size for SU: 0x%01x", he_cap->codebook_su);
	WMA_LOGD("\tCodebook size for MU: 0x%01x ", he_cap->codebook_mu);
	WMA_LOGD("\tBeamforming trigger w/ Trigger: 0x%01x",
			he_cap->beamforming_feedback);
	WMA_LOGD("\tHE ER SU PPDU payload: 0x%01x", he_cap->he_er_su_ppdu);
	WMA_LOGD("\tDL MUMIMO on partial BW: 0x%01x",
			he_cap->dl_mu_mimo_part_bw);
	WMA_LOGD("\tPPET present: 0x%01x", he_cap->ppet_present);
	WMA_LOGD("\tSRP based SR-support: 0x%01x", he_cap->srp);
	WMA_LOGD("\tPower boost factor: 0x%01x", he_cap->power_boost);
	WMA_LOGD("\t4x HE LTF support: 0x%01x", he_cap->he_ltf_800_gi_4x);

	WMA_LOGD("\tMax NC: 0x%01x", he_cap->max_nc);
	WMA_LOGD("\tstbc Tx gt 80mhz: 0x%01x", he_cap->tx_stbc_gt_80mhz);
	WMA_LOGD("\tstbc Rx gt 80mhz: 0x%01x", he_cap->rx_stbc_gt_80mhz);
	WMA_LOGD("\ter_he_ltf_800_gi_4x: 0x%01x", he_cap->er_he_ltf_800_gi_4x);
	WMA_LOGD("\the_ppdu_20_in_40Mhz_2G: 0x%01x",
					he_cap->he_ppdu_20_in_40Mhz_2G);
	WMA_LOGD("\the_ppdu_20_in_160_80p80Mhz: 0x%01x",
					he_cap->he_ppdu_20_in_160_80p80Mhz);
	WMA_LOGD("\the_ppdu_80_in_160_80p80Mhz: 0x%01x",
					he_cap->he_ppdu_80_in_160_80p80Mhz);
	WMA_LOGD("\ter_1x_he_ltf_gi: 0x%01x",
					he_cap->er_1x_he_ltf_gi);
	WMA_LOGD("\tmidamble_rx_1x_he_ltf: 0x%01x",
					he_cap->midamble_rx_1x_he_ltf);
	WMA_LOGD("\tRx MCS MAP for BW <= 80 MHz: 0x%x",
		he_cap->rx_he_mcs_map_lt_80);
	WMA_LOGD("\tTx MCS MAP for BW <= 80 MHz: 0x%x",
		he_cap->tx_he_mcs_map_lt_80);
	WMA_LOGD("\tRx MCS MAP for BW = 160 MHz: 0x%x",
		*((uint16_t *)he_cap->rx_he_mcs_map_160));
	WMA_LOGD("\tTx MCS MAP for BW = 160 MHz: 0x%x",
		*((uint16_t *)he_cap->tx_he_mcs_map_160));
	WMA_LOGD("\tRx MCS MAP for BW = 80 + 80 MHz: 0x%x",
		*((uint16_t *)he_cap->rx_he_mcs_map_80_80));
	WMA_LOGD("\tTx MCS MAP for BW = 80 + 80 MHz: 0x%x",
		*((uint16_t *)he_cap->tx_he_mcs_map_80_80));

	hdr = (struct ppet_hdr *)&he_cap->ppet.ppe_threshold.ppe_th[0];
	/* HE PPET */
	WMA_LOGD("\tNSS: %d", hdr->nss + 1);
	WMA_LOGD("\tRU Index mask: 0x%04x", hdr->ru_idx_mask);
	WMA_LOGD("\tnum_ppet: %d", he_cap->ppet.ppe_threshold.num_ppe_th);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
		he_cap->ppet.ppe_threshold.ppe_th,
		he_cap->ppet.ppe_threshold.num_ppe_th);
}

void wma_print_he_ppet(void *he_ppet)
{
	int numss, ru_count, ru_bit_mask, i, j;
	struct wmi_host_ppe_threshold *ppet = he_ppet;

	if (!ppet) {
		WMA_LOGI(FL("PPET is NULL"));
		return;
	}

	numss = ppet->numss_m1 + 1;
	ru_bit_mask = ppet->ru_bit_mask;

	WMA_LOGD(FL("HE PPET: ru_idx_mask: %04x"), ru_bit_mask);
	for (ru_count = 0; ru_bit_mask; ru_bit_mask >>= 1)
		if (ru_bit_mask & 0x1)
			ru_count++;

	if (ru_count > 0) {
		WMA_LOGD(FL("PPET has following RU INDEX,"));
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX0_MASK)
			WMA_LOGD("\tRU ALLOCATION INDEX 0");
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX1_MASK)
			WMA_LOGD("\tRU ALLOCATION INDEX 1");
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX2_MASK)
			WMA_LOGD("\tRU ALLOCATION INDEX 2");
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX3_MASK)
			WMA_LOGD("\tRU ALLOCATION INDEX 3");
	}

	WMA_LOGD(FL("HE PPET: nss: %d, ru_count: %d"), numss, ru_count);

	for (i = 0; i < numss; i++) {
		WMA_LOGD("PPET for NSS[%d]", i);
		for (j = 1; j <= ru_count; j++) {
			WMA_LOGD("\tNSS[%d],RU[%d]: PPET16: %02x PPET8: %02x",
			    i, j,
			    WMI_GET_PPET16(ppet->ppet16_ppet8_ru3_ru0, j, i),
			    WMI_GET_PPET8(ppet->ppet16_ppet8_ru3_ru0, j, i));
		}
	}

}

void wma_print_he_phy_cap(uint32_t *phy_cap)
{
	WMA_LOGD(FL("HE PHY Capabilities:"));

	WMA_LOGD("\tDual band support: 0x%01x",
		WMI_HECAP_PHY_DB_GET(phy_cap));
	WMA_LOGD("\tChannel width support: 0x%07x",
		WMI_HECAP_PHY_CBW_GET(phy_cap));
	WMA_LOGD("\tPreamble puncturing Rx: 0x%04x",
		WMI_HECAP_PHY_PREAMBLEPUNCRX_GET(phy_cap));
	WMA_LOGD("\tClass of device: 0x%01x", WMI_HECAP_PHY_COD_GET(phy_cap));
	WMA_LOGD("\tLDPC coding support: 0x%01x",
		WMI_HECAP_PHY_LDPC_GET(phy_cap));
	WMA_LOGD("\tLTF and GI for HE PPDUs: 0x%02x",
		WMI_HECAP_PHY_LTFGIFORHE_GET(phy_cap));
	WMA_LOGD("\tLTF and GI for NDP: 0x%02x",
		WMI_HECAP_PHY_LTFGIFORNDP_GET(phy_cap));
	WMA_LOGD("\tSTBC Tx & Rx support (BW <= 80Mhz): 0x%02x",
			(WMI_HECAP_PHY_RXSTBC_GET(phy_cap) << 1) |
			 WMI_HECAP_PHY_TXSTBC_GET(phy_cap));
	WMA_LOGD("\tDoppler support: 0x%02x",
			(WMI_HECAP_PHY_RXDOPPLER_GET(phy_cap) << 1) |
			 WMI_HECAP_PHY_TXDOPPLER_GET(phy_cap));
	WMA_LOGD("\tUL MU (Full BW): 0x%01x",
		  WMI_HECAP_PHY_UL_MU_MIMO_GET(phy_cap));
	WMA_LOGD("\tUL MU (Partial BW): 0x%01x",
		  WMI_HECAP_PHY_ULMUMIMOOFDMA_GET(phy_cap));
	WMA_LOGD("\tDCM encoding Tx: 0x%03x", WMI_HECAP_PHY_DCMTX_GET(phy_cap));
	WMA_LOGD("\tDCM encoding Tx: 0x%03x", WMI_HECAP_PHY_DCMRX_GET(phy_cap));
	WMA_LOGD("\tHE MU PPDU payload support: 0x%01x",
		WMI_HECAP_PHY_ULHEMU_GET(phy_cap));
	WMA_LOGD("\tSU Beamformer: 0x%01x", WMI_HECAP_PHY_SUBFMR_GET(phy_cap));
	WMA_LOGD("\tSU Beamformee: 0x%01x", WMI_HECAP_PHY_SUBFME_GET(phy_cap));
	WMA_LOGD("\tMU Beamformer: 0x%01x", WMI_HECAP_PHY_MUBFMR_GET(phy_cap));
	WMA_LOGD("\tBeamformee STS for <= 80Mhz: 0x%03x",
			WMI_HECAP_PHY_SUBFMESTS_GET(phy_cap));
	WMA_LOGD("\tNSTS total for <= 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NSTSLT80MHZ_GET(phy_cap));
	WMA_LOGD("\tBeamformee STS for > 80Mhz: 0x%03x",
		WMI_HECAP_PHY_BFMESTSGT80MHZ_GET(phy_cap));
	WMA_LOGD("\tNSTS total for > 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NSTSGT80MHZ_GET(phy_cap));
	WMA_LOGD("\tNo. of sounding dim <= 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NUMSOUNDLT80MHZ_GET(phy_cap));
	WMA_LOGD("\tNo. of sounding dim > 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NUMSOUNDGT80MHZ_GET(phy_cap));
	WMA_LOGD("\tNg=16 for SU feedback support: 0x%01x",
		WMI_HECAP_PHY_NG16SUFEEDBACKLT80_GET(phy_cap));
	WMA_LOGD("\tNg=16 for MU feedback support: 0x%01x",
		WMI_HECAP_PHY_NG16MUFEEDBACKGT80_GET(phy_cap));
	WMA_LOGD("\tCodebook size for SU: 0x%01x",
		WMI_HECAP_PHY_CODBK42SU_GET(phy_cap));
	WMA_LOGD("\tCodebook size for MU: 0x%01x ",
		WMI_HECAP_PHY_CODBK75MU_GET(phy_cap));
	WMA_LOGD("\tBeamforming trigger w/ Trigger: 0x%01x",
		WMI_HECAP_PHY_BFFEEDBACKTRIG_GET(phy_cap));
	WMA_LOGD("\tHE ER SU PPDU payload: 0x%01x",
		WMI_HECAP_PHY_HEERSU_GET(phy_cap));
	WMA_LOGD("\tDL MUMIMO on partial BW: 0x%01x",
		WMI_HECAP_PHY_DLMUMIMOPARTIALBW_GET(phy_cap));
	WMA_LOGD("\tPPET present: 0x%01x", WMI_HECAP_PHY_PADDING_GET(phy_cap));
	WMA_LOGD("\tSRP based SR-support: 0x%01x",
		WMI_HECAP_PHY_SRPSPRESENT_GET(phy_cap));
	WMA_LOGD("\tPower boost factor: 0x%01x",
		WMI_HECAP_PHY_PWRBOOSTAR_GET(phy_cap));
	WMA_LOGD("\t4x HE LTF support: 0x%01x",
		WMI_HECAP_PHY_4XLTFAND800NSECSGI_GET(phy_cap));
	WMA_LOGD("\tMax Nc supported: 0x%03x",
		WMI_HECAP_PHY_MAXNC_GET(phy_cap));
	WMA_LOGD("\tSTBC Tx & Rx support (BW > 80Mhz): 0x%02x",
			(WMI_HECAP_PHY_STBCRXGT80_GET(phy_cap) << 1) |
			 WMI_HECAP_PHY_STBCTXGT80_GET(phy_cap));
	WMA_LOGD("\tER 4x HE LTF support: 0x%01x",
		 WMI_HECAP_PHY_ERSU4X800NSECGI_GET(phy_cap));
}

void wma_print_he_mac_cap(uint32_t mac_cap)
{
	WMA_LOGD(FL("HE MAC Capabilities:"));

	WMA_LOGD("\tHTC-HE conrol: 0x%01x", WMI_HECAP_MAC_HECTRL_GET(mac_cap));
	WMA_LOGD("\tTWT Requestor support: 0x%01x",
			WMI_HECAP_MAC_TWTREQ_GET(mac_cap));
	WMA_LOGD("\tTWT Responder support: 0x%01x",
			WMI_HECAP_MAC_TWTRSP_GET(mac_cap));
	WMA_LOGD("\tFragmentation support: 0x%02x",
			WMI_HECAP_MAC_HEFRAG_GET(mac_cap));
	WMA_LOGD("\tMax no.of frag MSDUs: 0x%03x",
			WMI_HECAP_MAC_MAXFRAGMSDU_GET(mac_cap));
	WMA_LOGD("\tMin. frag size: 0x%02x",
			WMI_HECAP_MAC_MINFRAGSZ_GET(mac_cap));
	WMA_LOGD("\tTrigger MAC pad duration: 0x%02x",
			WMI_HECAP_MAC_TRIGPADDUR_GET(mac_cap));
	WMA_LOGD("\tMulti-TID aggr support: 0x%03x",
			WMI_HECAP_MAC_ACKMTIDAMPDU_GET(mac_cap));
	WMA_LOGD("\tLink adaptation: 0x%02x",
			WMI_HECAP_MAC_HELKAD_GET(mac_cap));
	WMA_LOGD("\tAll ACK support: 0x%01x",
			WMI_HECAP_MAC_AACK_GET(mac_cap));
	WMA_LOGD("\tUL MU resp. scheduling: 0x%01x",
			WMI_HECAP_MAC_ULMURSP_GET(mac_cap));
	WMA_LOGD("\tA-Buff status report: 0x%01x",
			WMI_HECAP_MAC_BSR_GET(mac_cap));
	WMA_LOGD("\tBroadcast TWT support: 0x%01x",
			WMI_HECAP_MAC_BCSTTWT_GET(mac_cap));
	WMA_LOGD("\t32bit BA bitmap support: 0x%01x",
			WMI_HECAP_MAC_32BITBA_GET(mac_cap));
	WMA_LOGD("\tMU Cascading support: 0x%01x",
			WMI_HECAP_MAC_MUCASCADE_GET(mac_cap));
	WMA_LOGD("\tACK enabled Multi-TID: 0x%01x",
			WMI_HECAP_MAC_ACKMTIDAMPDU_GET(mac_cap));
	WMA_LOGD("\tMulti-STA BA in DL MU: 0x%01x",
			WMI_HECAP_MAC_GROUPMSTABA_GET(mac_cap));
	WMA_LOGD("\tOMI A-Control support: 0x%01x",
			WMI_HECAP_MAC_OMI_GET(mac_cap));
	WMA_LOGD("\tOFDMA RA support: 0x%01x",
			WMI_HECAP_MAC_OFDMARA_GET(mac_cap));
	WMA_LOGD("\tMax A-MPDU Length: 0x%02x",
		WMI_HECAP_MAC_MAXAMPDULEN_EXP_GET(mac_cap));
	WMA_LOGD("\tA-MSDU Fragmentation: 0x%01x",
		WMI_HECAP_MAC_AMSDUFRAG_GET(mac_cap));
	WMA_LOGD("\tFlex. TWT sched support: 0x%01x",
		WMI_HECAP_MAC_FLEXTWT_GET(mac_cap));
	WMA_LOGD("\tRx Ctrl frame to MBSS: 0x%01x",
			WMI_HECAP_MAC_MBSS_GET(mac_cap));
	WMA_LOGD("\tBSRP A-MPDU Aggregation: 0x%01x",
		WMI_HECAP_MAC_BSRPAMPDU_GET(mac_cap));
	WMA_LOGD("\tQuite Time Period support: 0x%01x",
		WMI_HECAP_MAC_QTP_GET(mac_cap));
	WMA_LOGD("\tA-BQR support: 0x%01x", WMI_HECAP_MAC_ABQR_GET(mac_cap));
	WMA_LOGD("\tSR Responder support: 0x%01x",
		 WMI_HECAP_MAC_SRRESP_GET(mac_cap));
	WMA_LOGD("\tOPS Support: 0x%01x",
		 WMI_HECAP_MAC_OPS_GET(mac_cap));
	WMA_LOGD("\tNDP Feedback Support: 0x%01x",
		 WMI_HECAP_MAC_NDPFDBKRPT_GET(mac_cap));
}

void wma_update_target_ext_he_cap(struct target_psoc_info *tgt_hdl,
				  struct wma_tgt_cfg *tgt_cfg)
{
	tDot11fIEhe_cap *he_cap = &tgt_cfg->he_cap;
	int i, num_hw_modes, total_mac_phy_cnt;
	struct wlan_psoc_host_mac_phy_caps *mac_cap, *mac_phy_cap;
	tDot11fIEhe_cap he_cap_mac;
	tDot11fIEhe_cap tmp_he_cap = {0};

	num_hw_modes = target_psoc_get_num_hw_modes(tgt_hdl);
	mac_phy_cap = target_psoc_get_mac_phy_cap(tgt_hdl);
	total_mac_phy_cnt = target_psoc_get_total_mac_phy_cnt(tgt_hdl);

	if (!num_hw_modes) {
		WMA_LOGE(FL("No extended HE cap for current SOC"));
		he_cap->present = false;
		return;
	}

	if (!tgt_cfg->services.en_11ax) {
		WMA_LOGI(FL("Target does not support 11AX"));
		he_cap->present = false;
		return;
	}

	for (i = 0; i < total_mac_phy_cnt; i++) {
		qdf_mem_zero(&he_cap_mac,
				sizeof(tDot11fIEhe_cap));
		mac_cap = &mac_phy_cap[i];
		if (mac_cap->supported_bands & WLAN_2G_CAPABILITY) {
			wma_convert_he_cap(&he_cap_mac,
					mac_cap->he_cap_info_2G,
					mac_cap->he_cap_phy_info_2G,
					mac_cap->he_supp_mcs_2G,
					mac_cap->tx_chain_mask_2G,
					mac_cap->rx_chain_mask_2G);
			WMA_LOGD(FL("2g phy: nss: %d, ru_idx_msk: %d"),
					mac_cap->he_ppet2G.numss_m1,
					mac_cap->he_ppet2G.ru_bit_mask);
			wma_convert_he_ppet(tgt_cfg->ppet_2g,
					(struct wmi_host_ppe_threshold *)
					&mac_cap->he_ppet2G);
		}

		if (he_cap_mac.present)
			wma_derive_ext_he_cap(&tmp_he_cap,
					&he_cap_mac);

		qdf_mem_zero(&he_cap_mac,
				sizeof(tDot11fIEhe_cap));
		if (mac_cap->supported_bands & WLAN_5G_CAPABILITY) {
			wma_convert_he_cap(&he_cap_mac,
					mac_cap->he_cap_info_5G,
					mac_cap->he_cap_phy_info_5G,
					mac_cap->he_supp_mcs_5G,
					mac_cap->tx_chain_mask_5G,
					mac_cap->rx_chain_mask_5G);
			WMA_LOGD(FL("5g phy: nss: %d, ru_idx_msk: %d"),
					mac_cap->he_ppet2G.numss_m1,
					mac_cap->he_ppet2G.ru_bit_mask);
			wma_convert_he_ppet(tgt_cfg->ppet_5g,
					(struct wmi_host_ppe_threshold *)
					&mac_cap->he_ppet5G);
		}
		if (he_cap_mac.present)
			wma_derive_ext_he_cap(&tmp_he_cap,
					&he_cap_mac);
	}

	qdf_mem_copy(he_cap, &tmp_he_cap, sizeof(*he_cap));
	wma_print_he_cap(he_cap);
}

void wma_he_update_tgt_services(struct wmi_unified *wmi_handle,
				struct wma_tgt_services *cfg)
{
	if (wmi_service_enabled(wmi_handle, wmi_service_11ax)) {
		cfg->en_11ax = true;
		wma_set_fw_wlan_feat_caps(DOT11AX);
		WMA_LOGI(FL("11ax is enabled"));
	} else {
		WMA_LOGI(FL("11ax is not enabled"));
	}
}

void wma_print_he_op(tDot11fIEhe_op *he_ops)
{
	WMA_LOGD(FL("bss_color: %0x, default_pe_duration: %0x, twt_required: %0x, rts_threshold: %0x, vht_oper_present: %0x"),
		he_ops->bss_color, he_ops->default_pe,
		he_ops->twt_required, he_ops->rts_threshold,
		he_ops->vht_oper_present);
	WMA_LOGD(FL("\tpartial_bss_color: %0x, MBSSID AP: %0x, Tx BSSID Indicator: %0x, BSS color disabled: %0x"),
		he_ops->partial_bss_col, he_ops->mbssid_ap,
		he_ops->tx_bssid_ind, he_ops->bss_col_disabled);
}

/**
 * wma_parse_he_ppet() - Convert PPET stored in dot11f structure into FW
 *                       structure.
 * @rcvd_ppet: pointer to dot11f format PPET
 * @peer_ppet: pointer peer_ppet to be sent in peer assoc
 *
 * This function converts the sequence of PPET stored in the host in OTA type
 * structure into FW understandable structure to be sent as part of peer assoc
 * command.
 *
 * Return: None
 */
static void wma_parse_he_ppet(int8_t *rcvd_ppet,
			      struct wmi_host_ppe_threshold *peer_ppet)
{
	struct ppet_hdr *hdr;
	uint8_t num_ppet, mask, mask1, mask2;
	uint32_t ppet1, ppet2, ppet;
	uint8_t bits, pad, pad_bits, req_byte;
	uint8_t byte_idx, start, i, j, parsed;
	uint32_t *ppet_r = peer_ppet->ppet16_ppet8_ru3_ru0;
	uint8_t nss, ru;

	hdr = (struct ppet_hdr *)rcvd_ppet;
	nss = hdr->nss + 1;
	mask = hdr->ru_idx_mask;
	peer_ppet->numss_m1 = nss - 1;
	peer_ppet->ru_bit_mask = mask;

	for (ru = 0; mask; mask >>= 1) {
		if (mask & 0x1)
			ru++;
	}

	WMA_LOGD(FL("Rcvd nss=%d ru_idx_mask: %0x ru_count=%d"),
		 nss, hdr->ru_idx_mask, ru);

	/* each nss-ru pair have 2 PPET (PPET8/PPET16) */
	bits = HE_PPET_NSS_RU_LEN + (nss + ru) * (HE_PPET_SIZE * 2);
	pad = bits % HE_BYTE_SIZE;
	pad_bits = HE_BYTE_SIZE - pad;
	req_byte = (bits + pad_bits) / HE_BYTE_SIZE;

	/*
	 * PPE Threshold Field Format
	 * +-----------+--------------+--------------------+-------------+
	 * |   NSS     | RU idx mask  | PPE Threshold info |  Padding    |
	 * +-----------+--------------+--------------------+-------------+
	 *        3           4             1 + variable       variable   (bits)
	 *
	 * PPE Threshold Info field:
	 * number of NSS:n, number of RU: m
	 * +------------+-----------+-----+------------+-----------+-----+-----------+-----------+
	 * | PPET16 for | PPET8 for | ... | PPET16 for | PPET8 for | ... | PET16 for | PPET8 for |
	 * | NSS1, RU1  | NSS1, RU1 | ... | NSS1, RUm  | NSS1, RUm | ... | NSSn, RUm | NSSn, RUm |
	 * +------------+-----------+-----+------------+-----------+-----+-----------+-----------+
	 *        3           3       ...       3           3        ...       3           3
	 */

	/* first bit of first PPET is in the last bit of first byte */
	parsed = HE_PPET_NSS_RU_LEN;

	/*
	 * refer wmi_ppe_threshold defn to understand how ppet is stored.
	 * Index of ppet array(ppet16_ppet8_ru3_ru0) is the NSS value.
	 * Each item in ppet16_ppet8_ru3_ru0 holds ppet for all the RUs.
	 */
	num_ppet = ru * 2; /* for each NSS */
	for (i = 0; i < nss; i++) {
		for (j = 1; j <= num_ppet; j++) {
			start = parsed + (i * (num_ppet * HE_PPET_SIZE)) +
				(j-1) * HE_PPET_SIZE;
			byte_idx = start / HE_BYTE_SIZE;
			start = start % HE_BYTE_SIZE;

			if (start <= HE_BYTE_SIZE - HE_PPET_SIZE) {
				mask = 0x07 << start;
				ppet = (rcvd_ppet[byte_idx] & mask) >> start;
				ppet_r[i] |= (ppet << (j - 1) * HE_PPET_SIZE);
			} else {
				mask1 = 0x07 << start;
				ppet1 = (rcvd_ppet[byte_idx] & mask1) >> start;
				mask2 = 0x07 >> (HE_BYTE_SIZE - start);
				ppet2 = (rcvd_ppet[byte_idx + 1] & mask2) <<
						(HE_BYTE_SIZE - start);
				ppet = ppet1 | ppet2;
				ppet_r[i] |= (ppet << (j - 1) * HE_PPET_SIZE);
			}
			WMA_LOGD(FL("nss:%d ru:%d ppet_r:%0x"), i, j/2,
				 ppet_r[i]);
		}
	}
}

void wma_populate_peer_he_cap(struct peer_assoc_params *peer,
			      tpAddStaParams params)
{
	tDot11fIEhe_cap *he_cap = &params->he_config;
	tDot11fIEhe_op *he_op = &params->he_op;
	uint32_t *phy_cap = peer->peer_he_cap_phyinfo;
	uint32_t mac_cap = 0, he_ops = 0;
	uint8_t temp, i, chan_width;

	if (params->he_capable)
		peer->peer_flags |= WMI_PEER_HE;
	else
		return;

	/* HE MAC capabilities */
	WMI_HECAP_MAC_HECTRL_SET(mac_cap, he_cap->htc_he);
	WMI_HECAP_MAC_TWTREQ_SET(mac_cap, he_cap->twt_request);
	WMI_HECAP_MAC_TWTRSP_SET(mac_cap, he_cap->twt_responder);
	WMI_HECAP_MAC_HEFRAG_SET(mac_cap, he_cap->fragmentation);
	WMI_HECAP_MAC_MAXFRAGMSDU_SET(mac_cap, he_cap->max_num_frag_msdu);
	WMI_HECAP_MAC_MINFRAGSZ_SET(mac_cap, he_cap->min_frag_size);
	WMI_HECAP_MAC_TRIGPADDUR_SET(mac_cap, he_cap->trigger_frm_mac_pad);
	WMI_HECAP_MAC_ACKMTIDAMPDU_SET(mac_cap, he_cap->multi_tid_aggr);
	WMI_HECAP_MAC_HELKAD_SET(mac_cap, he_cap->he_link_adaptation);
	WMI_HECAP_MAC_AACK_SET(mac_cap, he_cap->all_ack);
	WMI_HECAP_MAC_ULMURSP_SET(mac_cap, he_cap->ul_mu_rsp_sched);
	WMI_HECAP_MAC_BSR_SET(mac_cap, he_cap->a_bsr);
	WMI_HECAP_MAC_BCSTTWT_SET(mac_cap, he_cap->broadcast_twt);
	WMI_HECAP_MAC_32BITBA_SET(mac_cap, he_cap->ba_32bit_bitmap);
	WMI_HECAP_MAC_MUCASCADE_SET(mac_cap, he_cap->mu_cascade);
	WMI_HECAP_MAC_ACKMTIDAMPDU_SET(mac_cap, he_cap->ack_enabled_multitid);
	WMI_HECAP_MAC_GROUPMSTABA_SET(mac_cap, he_cap->dl_mu_ba);
	WMI_HECAP_MAC_OMI_SET(mac_cap, he_cap->omi_a_ctrl);
	WMI_HECAP_MAC_OFDMARA_SET(mac_cap, he_cap->ofdma_ra);
	WMI_HECAP_MAC_MAXAMPDULEN_EXP_SET(mac_cap, he_cap->max_ampdu_len);
	WMI_HECAP_MAC_AMSDUFRAG_SET(mac_cap, he_cap->amsdu_frag);
	WMI_HECAP_MAC_FLEXTWT_SET(mac_cap, he_cap->flex_twt_sched);
	WMI_HECAP_MAC_MBSS_SET(mac_cap, he_cap->rx_ctrl_frame);
	WMI_HECAP_MAC_BSRPAMPDU_SET(mac_cap, he_cap->bsrp_ampdu_aggr);
	WMI_HECAP_MAC_QTP_SET(mac_cap, he_cap->qtp);
	WMI_HECAP_MAC_ABQR_SET(mac_cap, he_cap->a_bqr);
	WMI_HECAP_MAC_SRRESP_SET(mac_cap, he_cap->sr_responder);
	WMI_HECAP_MAC_OPS_SET(mac_cap, he_cap->ops_supp);
	WMI_HECAP_MAC_NDPFDBKRPT_SET(mac_cap, he_cap->ndp_feedback_supp);
	WMI_HECAP_MAC_AMSDUINAMPDU_SET(mac_cap, he_cap->amsdu_in_ampdu);
	peer->peer_he_cap_macinfo = mac_cap;

	/* HE PHY capabilities */
	WMI_HECAP_PHY_DB_SET(phy_cap, he_cap->dual_band);
	chan_width = HE_CH_WIDTH_COMBINE(he_cap->chan_width_0,
				he_cap->chan_width_1, he_cap->chan_width_2,
				he_cap->chan_width_3, he_cap->chan_width_4,
				he_cap->chan_width_5, he_cap->chan_width_6);
	WMI_HECAP_PHY_CBW_SET(phy_cap, chan_width);
	WMI_HECAP_PHY_PREAMBLEPUNCRX_SET(phy_cap, he_cap->rx_pream_puncturing);
	WMI_HECAP_PHY_COD_SET(phy_cap, he_cap->device_class);
	WMI_HECAP_PHY_LDPC_SET(phy_cap, he_cap->ldpc_coding);
	WMI_HECAP_PHY_LTFGIFORHE_SET(phy_cap, he_cap->he_1x_ltf_800_gi_ppdu);
	WMI_HECAP_PHY_MIDAMBLERXMAXNSTS_SET(phy_cap,
				he_cap->midamble_rx_max_nsts);
	WMI_HECAP_PHY_LTFGIFORNDP_SET(phy_cap, he_cap->he_4x_ltf_3200_gi_ndp);

	WMI_HECAP_PHY_RXSTBC_SET(phy_cap, he_cap->rx_stbc_lt_80mhz);
	WMI_HECAP_PHY_TXSTBC_SET(phy_cap, he_cap->tx_stbc_lt_80mhz);

	temp = he_cap->doppler & 0x1;
	WMI_HECAP_PHY_RXDOPPLER_SET(phy_cap, temp);
	temp = he_cap->doppler >> 0x1;
	WMI_HECAP_PHY_TXDOPPLER_SET(phy_cap, temp);

	temp = he_cap->ul_mu & 0x1;
	WMI_HECAP_PHY_UL_MU_MIMO_SET(phy_cap, temp);
	temp = he_cap->ul_mu >>  0x1;
	WMI_HECAP_PHY_ULMUMIMOOFDMA_SET(phy_cap, temp);

	WMI_HECAP_PHY_DCMTX_SET(phy_cap, he_cap->dcm_enc_tx);
	WMI_HECAP_PHY_DCMRX_SET(phy_cap, he_cap->dcm_enc_rx);
	WMI_HECAP_PHY_ULHEMU_SET(phy_cap, he_cap->ul_he_mu);
	WMI_HECAP_PHY_SUBFMR_SET(phy_cap, he_cap->su_beamformer);
	WMI_HECAP_PHY_SUBFME_SET(phy_cap, he_cap->su_beamformee);
	WMI_HECAP_PHY_MUBFMR_SET(phy_cap, he_cap->mu_beamformer);
	WMI_HECAP_PHY_BFMESTSLT80MHZ_SET(phy_cap, he_cap->bfee_sts_lt_80);
	WMI_HECAP_PHY_BFMESTSGT80MHZ_SET(phy_cap, he_cap->bfee_sts_gt_80);
	WMI_HECAP_PHY_NUMSOUNDLT80MHZ_SET(phy_cap, he_cap->num_sounding_lt_80);
	WMI_HECAP_PHY_NUMSOUNDGT80MHZ_SET(phy_cap, he_cap->num_sounding_gt_80);
	WMI_HECAP_PHY_NG16SUFEEDBACKLT80_SET(phy_cap,
					     he_cap->su_feedback_tone16);
	WMI_HECAP_PHY_NG16MUFEEDBACKGT80_SET(phy_cap,
					     he_cap->mu_feedback_tone16);
	WMI_HECAP_PHY_CODBK42SU_SET(phy_cap, he_cap->codebook_su);
	WMI_HECAP_PHY_CODBK75MU_SET(phy_cap, he_cap->codebook_mu);
	WMI_HECAP_PHY_BFFEEDBACKTRIG_SET(phy_cap, he_cap->beamforming_feedback);
	WMI_HECAP_PHY_HEERSU_SET(phy_cap, he_cap->he_er_su_ppdu);
	WMI_HECAP_PHY_DLMUMIMOPARTIALBW_SET(phy_cap,
					    he_cap->dl_mu_mimo_part_bw);
	WMI_HECAP_PHY_PETHRESPRESENT_SET(phy_cap, he_cap->ppet_present);
	WMI_HECAP_PHY_SRPPRESENT_SET(phy_cap, he_cap->srp);
	WMI_HECAP_PHY_PWRBOOSTAR_SET(phy_cap, he_cap->power_boost);
	WMI_HECAP_PHY_4XLTFAND800NSECSGI_SET(phy_cap, he_cap->he_ltf_800_gi_4x);

	WMI_HECAP_PHY_MAXNC_SET(phy_cap, he_cap->max_nc);

	WMI_HECAP_PHY_STBCRXGT80_SET(phy_cap, he_cap->rx_stbc_gt_80mhz);
	WMI_HECAP_PHY_STBCTXGT80_SET(phy_cap, he_cap->tx_stbc_gt_80mhz);

	WMI_HECAP_PHY_ERSU4X800NSECGI_SET(phy_cap, he_cap->er_he_ltf_800_gi_4x);
	WMI_HECAP_PHY_HEPPDU20IN40MHZ2G_SET(phy_cap,
					he_cap->he_ppdu_20_in_40Mhz_2G);
	WMI_HECAP_PHY_HEPPDU20IN160OR80P80MHZ_SET(phy_cap,
					he_cap->he_ppdu_20_in_160_80p80Mhz);
	WMI_HECAP_PHY_HEPPDU80IN160OR80P80MHZ_SET(phy_cap,
					he_cap->he_ppdu_80_in_160_80p80Mhz);
	WMI_HECAP_PHY_ERSU1X800NSECGI_SET(phy_cap, he_cap->er_1x_he_ltf_gi);
	WMI_HECAP_PHY_MIDAMBLERX2XAND1XHELTF_SET(phy_cap,
					he_cap->midamble_rx_1x_he_ltf);

	/* as per 11ax draft 1.4 */
	peer->peer_he_mcs_count = 1;
	peer->peer_he_rx_mcs_set[0] =
		params->supportedRates.rx_he_mcs_map_lt_80;
	peer->peer_he_tx_mcs_set[0] =
		params->supportedRates.tx_he_mcs_map_lt_80;

	if (params->ch_width > CH_WIDTH_80MHZ) {
		peer->peer_he_mcs_count = WMI_HOST_MAX_HE_RATE_SET;
		peer->peer_he_rx_mcs_set[1] =
			params->supportedRates.rx_he_mcs_map_160;
		peer->peer_he_tx_mcs_set[1] =
			params->supportedRates.tx_he_mcs_map_160;
		peer->peer_he_rx_mcs_set[2] =
			params->supportedRates.rx_he_mcs_map_80_80;
		peer->peer_he_tx_mcs_set[2] =
			params->supportedRates.tx_he_mcs_map_80_80;
	}

	for (i = 0; i < peer->peer_he_mcs_count; i++)
		WMA_LOGD(FL("[HE - MCS Map: %d] rx_mcs: 0x%x, tx_mcs: 0x%x"), i,
			peer->peer_he_rx_mcs_set[i],
			peer->peer_he_tx_mcs_set[i]);

	WMI_HEOPS_COLOR_SET(he_ops, he_op->bss_color);
	WMI_HEOPS_DEFPE_SET(he_ops, he_op->default_pe);
	WMI_HEOPS_TWT_SET(he_ops, he_op->twt_required);
	WMI_HEOPS_RTSTHLD_SET(he_ops, he_op->rts_threshold);
	WMI_HEOPS_PARTBSSCOLOR_SET(he_ops, he_op->partial_bss_col);
	WMI_HEOPS_TXBSSID_SET(he_ops, he_op->tx_bssid_ind);
	WMI_HEOPS_BSSCOLORDISABLE_SET(he_ops, he_op->bss_col_disabled);
	peer->peer_he_ops = he_ops;

	wma_parse_he_ppet(he_cap->ppet.ppe_threshold.ppe_th, &peer->peer_ppet);

	wma_print_he_cap(he_cap);
	WMA_LOGD(FL("Peer HE Capabilities:"));
	wma_print_he_phy_cap(phy_cap);
	wma_print_he_mac_cap(mac_cap);
	wma_print_he_ppet(&peer->peer_ppet);

	return;
}

void wma_update_vdev_he_ops(struct wma_vdev_start_req *req,
		tpAddBssParams add_bss)
{
	uint32_t he_ops = 0;
	tDot11fIEhe_op *he_op = &add_bss->he_op;

	req->he_capable = add_bss->he_capable;

	WMI_HEOPS_COLOR_SET(he_ops, he_op->bss_color);
	WMI_HEOPS_DEFPE_SET(he_ops, he_op->default_pe);
	WMI_HEOPS_TWT_SET(he_ops, he_op->twt_required);
	WMI_HEOPS_RTSTHLD_SET(he_ops, he_op->rts_threshold);
	WMI_HEOPS_PARTBSSCOLOR_SET(he_ops, he_op->partial_bss_col);
	WMI_HEOPS_TXBSSID_SET(he_ops, he_op->tx_bssid_ind);
	WMI_HEOPS_BSSCOLORDISABLE_SET(he_ops, he_op->bss_col_disabled);

	req->he_ops = he_ops;
}

void wma_copy_txrxnode_he_ops(struct wma_txrx_node *node,
		struct wma_vdev_start_req *req)
{
	node->he_capable = req->he_capable;
	node->he_ops = req->he_ops;
}

void wma_copy_vdev_start_he_ops(struct vdev_start_params *params,
		struct wma_vdev_start_req *req)
{
	params->he_ops = req->he_ops;
}

void wma_vdev_set_he_bss_params(tp_wma_handle wma, uint8_t vdev_id,
				struct wma_vdev_start_req *req)
{
	QDF_STATUS ret;
	struct wma_txrx_node *intr = wma->interfaces;

	if (!req->he_capable)
		return;

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_HEOPS_0_31, req->he_ops);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE(FL("Failed to set HE OPs"));
	else
		intr[vdev_id].he_ops = req->he_ops;
}

void wma_vdev_set_he_config(tp_wma_handle wma, uint8_t vdev_id,
				tpAddBssParams add_bss)
{
	QDF_STATUS ret;
	int8_t pd_min, pd_max, sec_ch_ed, tx_pwr;

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
				 WMI_VDEV_PARAM_OBSSPD, add_bss->he_sta_obsspd);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE(FL("Failed to set HE Config"));
	pd_min = add_bss->he_sta_obsspd & 0xff,
	pd_max = (add_bss->he_sta_obsspd & 0xff00) >> 8,
	sec_ch_ed = (add_bss->he_sta_obsspd & 0xff0000) >> 16,
	tx_pwr = (add_bss->he_sta_obsspd & 0xff000000) >> 24;
	WMA_LOGD(FL("HE_STA_OBSSPD: PD_MIN: %d PD_MAX: %d SEC_CH_ED: %d TX_PWR: %d"),
		 pd_min, pd_max, sec_ch_ed, tx_pwr);
}

void wma_update_vdev_he_capable(struct wma_vdev_start_req *req,
		tpSwitchChannelParams params)
{
	req->he_capable = params->he_capable;
}

QDF_STATUS wma_update_he_ops_ie(tp_wma_handle wma, uint8_t vdev_id,
				tDot11fIEhe_op *he_op)
{
	QDF_STATUS ret;
	uint32_t dword_he_op = 0;

	if (!wma) {
		WMA_LOGE(FL("wrong wma_handle...."));
		return QDF_STATUS_E_FAILURE;
	}

	WMI_HEOPS_COLOR_SET(dword_he_op, he_op->bss_color);
	WMI_HEOPS_DEFPE_SET(dword_he_op, he_op->default_pe);
	WMI_HEOPS_TWT_SET(dword_he_op, he_op->twt_required);
	WMI_HEOPS_RTSTHLD_SET(dword_he_op, he_op->rts_threshold);
	WMI_HEOPS_PARTBSSCOLOR_SET(dword_he_op, he_op->partial_bss_col);
	WMI_HEOPS_TXBSSID_SET(dword_he_op, he_op->tx_bssid_ind);
	WMI_HEOPS_BSSCOLORDISABLE_SET(dword_he_op, he_op->bss_col_disabled);

	WMA_LOGD("vdev_id: %d HE_OPs: 0x%x", vdev_id, dword_he_op);
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_HEOPS_0_31, dword_he_op);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE(FL("Failed to set HE OPs"));

	return ret;
}

QDF_STATUS wma_get_he_capabilities(struct he_capability *he_cap)
{
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		WMA_LOGE(FL("Invalid WMA handle"));
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(he_cap->phy_cap,
		     &wma_handle->he_cap.phy_cap,
		     WMI_MAX_HECAP_PHY_SIZE);
	he_cap->mac_cap = wma_handle->he_cap.mac_cap;
	he_cap->mcs = wma_handle->he_cap.mcs;

	he_cap->ppet.numss_m1 = wma_handle->he_cap.ppet.numss_m1;
	he_cap->ppet.ru_bit_mask = wma_handle->he_cap.ppet.ru_bit_mask;
	qdf_mem_copy(&he_cap->ppet.ppet16_ppet8_ru3_ru0,
		     &wma_handle->he_cap.ppet.ppet16_ppet8_ru3_ru0,
		     WMI_MAX_NUM_SS);

	return QDF_STATUS_SUCCESS;
}

void wma_set_he_vdev_param(struct wma_txrx_node *intr, WMI_VDEV_PARAM param_id,
			   uint32_t value)
{
	switch (param_id) {
	case WMI_VDEV_PARAM_HE_DCM:
		intr->config.dcm = value;
		break;
	case WMI_VDEV_PARAM_HE_RANGE_EXT:
		intr->config.range_ext = value;
		break;
	default:
		WMA_LOGE(FL("Unhandled HE vdev param: %0x"), param_id);
		break;
	}
}

uint32_t wma_get_he_vdev_param(struct wma_txrx_node *intr,
			       WMI_VDEV_PARAM param_id)
{
	switch (param_id) {
	case WMI_VDEV_PARAM_HE_DCM:
		return intr->config.dcm;
	case WMI_VDEV_PARAM_HE_RANGE_EXT:
		return intr->config.range_ext;
	default:
		WMA_LOGE(FL("Unhandled HE vdev param: %0x"), param_id);
		break;
	}
	return 0;
}
