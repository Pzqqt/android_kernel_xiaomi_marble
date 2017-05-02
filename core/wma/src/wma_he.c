/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

	WMA_LOGI(FL("byte_idx=%d used=%d ppet=%04x"), *byte_idx_p, *used_p,
		    ppet);
	WMA_LOGI(FL("start host_ppet = %04x"), host_ppet[byte_idx]);
	if (used <= (HE_BYTE_SIZE - HE_PPET_SIZE)) {
		/* Enough space to fit the incoming PPET */
		lshift = used;
		host_ppet[byte_idx] |= (ppet << lshift);
		used += HE_PPET_SIZE;
		if (used == HE_BYTE_SIZE) {
			WMA_LOGI(FL("end1 host_ppet = %04x"),
				 host_ppet[byte_idx]);
			used = 0;
			byte_idx++;
		}
	} else {
		/* Need to split the PPET */
		lshift = used;
		rshift = HE_BYTE_SIZE - used;
		host_ppet[byte_idx] |= (ppet << lshift);
		WMA_LOGI(FL("end1 host_ppet = %04x"), host_ppet[byte_idx]);
		byte_idx++;
		used = 0;
		host_ppet[byte_idx] |= (ppet >> rshift);
		used +=  HE_PPET_SIZE - rshift;
	}

	WMA_LOGI(FL("end host_ppet = %04x"), host_ppet[byte_idx]);
	*byte_idx_p = byte_idx;
	*used_p = used;
}

/**
 * wma_he_find_ppet() - Helper function for PPET conversion
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
static void wma_he_find_ppet(uint32_t *ppet, int nss, int ru,
			     uint8_t *host_ppet, int req_byte)
{
	int byte_idx = 0, used, i, j;
	uint8_t ppet16, ppet8;

	WMA_LOGI(FL("nss: %d ru: %d req_byte: %d\n"), nss, ru, req_byte);
	/* NSS and RU_IDX are already populated */
	used = HE_PPET_NSS_RU_LEN;

	for (i = 0; i < nss; i++) {
		for (j = 1; j <= ru; j++) {
			ppet16 = WMI_GET_PPET16(ppet, j, i);
			ppet8 = WMI_GET_PPET8(ppet, j, i);
			WMA_LOGI(FL("ppet16 (nss:%d ru:%d): %04x"),
				 i, j, ppet16);
			WMA_LOGI(FL("ppet8 (nss:%d ru:%d): %04x"),
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
static void wma_convert_he_ppet(tDot11fIEppe_threshold *he_ppet,
				struct wmi_host_ppe_threshold *ppet)
{
	int bits, req_byte;
	uint8_t *host_ppet, ru_count, mask;

	if (!ppet) {
		WMA_LOGE(FL("PPET is NULL"));
		he_ppet->present = false;
		return;
	}

	he_ppet->present = true;
	he_ppet->nss_count = ppet->numss_m1;
	he_ppet->ru_idx_mask = ppet->ru_bit_mask;

	mask = he_ppet->ru_idx_mask;
	for (ru_count = 0; mask; mask >>= 1)
		if (mask & 0x01)
			ru_count++;

	/*
	 * there will be two PPET for each NSS/RU pair
	 * PPET8 and PPET16, hence HE_PPET_SIZE * 2 bits for PPET
	 */
	bits = HE_PPET_NSS_RU_LEN + ((he_ppet->nss_count + 1) * ru_count) *
				     (HE_PPET_SIZE * 2);

	req_byte = (bits / HE_BYTE_SIZE) + 1;

	host_ppet = qdf_mem_malloc(sizeof(*host_ppet) * req_byte);
	if (!host_ppet) {
		WMA_LOGE(FL("mem alloc failed for host_ppet"));
		he_ppet->present = false;
		return;
	}

	wma_he_find_ppet(ppet->ppet16_ppet8_ru3_ru0, he_ppet->nss_count + 1,
			 ru_count, host_ppet, req_byte);

	he_ppet->ppet_b1 = (host_ppet[0] << HE_PPET_NSS_RU_LEN);

	/*
	 * req_byte calculates total bytes, num_ppet stores only the bytes
	 * going into ppet data member in he_ppet. -1 to exclude the byte
	 * storing nss/ru and first PPET16.
	 */
	he_ppet->num_ppet = req_byte - 1;
	if (he_ppet->num_ppet > 0)
		qdf_mem_copy(he_ppet->ppet, &host_ppet[1], he_ppet->num_ppet);

	qdf_mem_free(host_ppet);
}

/**
 * wma_convert_he_cap() - convert HE capabilities into dot11f structure
 * @he_cap: pointer to dot11f structure
 * @mac_cap: Received HE MAC capability
 * @phy_cap: Received HE PHY capability
 * @he_ppet: Received HE PPE threshold
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
static void wma_convert_he_cap(tDot11fIEvendor_he_cap *he_cap, uint32_t mac_cap,
			       uint32_t *phy_cap, void *he_ppet,
			       uint32_t supp_mcs, uint32_t tx_chain_mask,
			       uint32_t rx_chain_mask)
{
	struct wmi_host_ppe_threshold *ppet = he_ppet;
	uint8_t mcs, nss, k, mcs_temp;

	nss = QDF_MAX(wma_get_num_of_setbits_from_bitmask(tx_chain_mask),
		      wma_get_num_of_setbits_from_bitmask(rx_chain_mask));

	mcs = 0;
	for (k = 1; k < nss; k++) {
		mcs_temp = WMI_HE_MAX_MCS_4_SS_MASK(supp_mcs, k);
		if (mcs_temp > mcs)
			mcs = mcs_temp;
	}

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

	/* HE PHY capabilities */
	he_cap->dual_band = WMI_HECAP_PHY_DB_GET(phy_cap);
	he_cap->chan_width = WMI_HECAP_PHY_CBW_GET(phy_cap);
	he_cap->rx_pream_puncturing = WMI_HECAP_PHY_PREAMBLEPUNCRX_GET(phy_cap);
	he_cap->device_class = WMI_HECAP_PHY_COD_GET(phy_cap);
	he_cap->ldpc_coding = WMI_HECAP_PHY_LDPC_GET(phy_cap);
	he_cap->he_ltf_gi_ppdu = WMI_HECAP_PHY_LTFGIFORHE_GET(phy_cap);
	he_cap->he_ltf_gi_ndp = WMI_HECAP_PHY_LTFGIFORNDP_GET(phy_cap);
	he_cap->stbc = (WMI_HECAP_PHY_RXSTBC_GET(phy_cap) << 1) |
			WMI_HECAP_PHY_TXSTBC_GET(phy_cap);
	he_cap->doppler = (WMI_HECAP_PHY_RXDOPPLER_GET(phy_cap) << 1) |
				WMI_HECAP_PHY_TXDOPPLER_GET(phy_cap);
	he_cap->ul_mu = WMI_HECAP_PHY_UL_MU_MIMO_GET(phy_cap);
	he_cap->dcm_enc_tx = WMI_HECAP_PHY_DCMTX_GET(phy_cap);
	he_cap->dcm_enc_rx = WMI_HECAP_PHY_DCMRX_GET(phy_cap);
	he_cap->ul_he_mu = WMI_HECAP_PHY_ULHEMU_GET(phy_cap);
	he_cap->su_beamformer = WMI_HECAP_PHY_SUBFMR_GET(phy_cap);
	he_cap->su_beamformee = WMI_HECAP_PHY_SUBFME_GET(phy_cap);
	he_cap->mu_beamformer = WMI_HECAP_PHY_MUBFMR_GET(phy_cap);
	he_cap->bfee_sts_lt_80 = WMI_HECAP_PHY_SUBFMESTS_GET(phy_cap);
	he_cap->nsts_tol_lt_80 = WMI_HECAP_PHY_NSTSLT80MHZ_GET(phy_cap);
	he_cap->bfee_sta_gt_80 = WMI_HECAP_PHY_BFMESTSGT80MHZ_GET(phy_cap);
	he_cap->nsts_tot_gt_80 = WMI_HECAP_PHY_NSTSGT80MHZ_GET(phy_cap);
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
	he_cap->he_ltf_gi_4x = WMI_HECAP_PHY_4XLTFAND800NSECSGI_GET(phy_cap);

	he_cap->nss_supported = nss - 1;
	he_cap->mcs_supported = mcs;

	/* For Draft 1.0, following fields will be zero */
	he_cap->tx_bw_bitmap = 0;
	he_cap->rx_bw_bitmap = 0;

	wma_convert_he_ppet(&he_cap->ppe_threshold, ppet);
}

/**
 * wma_derive_ext_he_cap() - Derive HE caps based on given value
 * @wma_handle: pointer to wma_handle
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
static void wma_derive_ext_he_cap(t_wma_handle *wma_handle,
		tDot11fIEvendor_he_cap *he_cap, tDot11fIEvendor_he_cap *new_cap)
{
	if (!he_cap->present) {
		/* First time update, copy the capability as is */
		qdf_mem_copy(he_cap, new_cap, sizeof(*he_cap));
		he_cap->present = true;
	} else {
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
		he_cap->reserved1 = QDF_MIN(he_cap->reserved1,
					    new_cap->reserved1);

		he_cap->dual_band = QDF_MIN(he_cap->dual_band,
					    new_cap->dual_band);
		he_cap->chan_width = QDF_MIN(he_cap->chan_width,
					     new_cap->chan_width);
		he_cap->rx_pream_puncturing =
			QDF_MIN(he_cap->rx_pream_puncturing,
				new_cap->rx_pream_puncturing);
		he_cap->device_class = QDF_MIN(he_cap->device_class,
					       new_cap->device_class);
		he_cap->ldpc_coding = QDF_MIN(he_cap->ldpc_coding,
					      new_cap->ldpc_coding);
		he_cap->he_ltf_gi_ppdu = QDF_MIN(he_cap->he_ltf_gi_ppdu,
						 new_cap->he_ltf_gi_ppdu);
		he_cap->he_ltf_gi_ndp = QDF_MIN(he_cap->he_ltf_gi_ndp,
						new_cap->he_ltf_gi_ndp);
		he_cap->stbc = QDF_MIN(he_cap->stbc,
				       new_cap->stbc);
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
		he_cap->nsts_tol_lt_80 = QDF_MIN(he_cap->nsts_tol_lt_80,
						 new_cap->nsts_tol_lt_80);
		he_cap->bfee_sta_gt_80 = QDF_MIN(he_cap->bfee_sta_gt_80,
						 new_cap->bfee_sta_gt_80);
		he_cap->nsts_tot_gt_80 = QDF_MIN(he_cap->nsts_tot_gt_80,
						 new_cap->nsts_tot_gt_80);
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
		he_cap->he_ltf_gi_4x = QDF_MIN(he_cap->he_ltf_gi_4x,
					       new_cap->he_ltf_gi_4x);
		he_cap->reserved2 = QDF_MIN(he_cap->reserved2,
					    new_cap->reserved2);
		he_cap->nss_supported = QDF_MIN(he_cap->nss_supported,
						new_cap->nss_supported);
		he_cap->mcs_supported = QDF_MIN(he_cap->mcs_supported,
						new_cap->mcs_supported);
		he_cap->tx_bw_bitmap = QDF_MIN(he_cap->tx_bw_bitmap,
					       new_cap->tx_bw_bitmap);
		he_cap->rx_bw_bitmap = QDF_MIN(he_cap->rx_bw_bitmap,
					       new_cap->rx_bw_bitmap);

	}
}

void wma_print_he_cap(tDot11fIEvendor_he_cap *he_cap)
{
	if (!he_cap->present) {
		WMA_LOGI(FL("HE Capabilities not present"));
		return;
	}

	WMA_LOGI(FL("HE Capabilities:"));

	/* HE MAC capabilities */
	WMA_LOGI("\tHTC-HE conrol: 0x%01x", he_cap->htc_he);
	WMA_LOGI("\tTWT Requestor support: 0x%01x", he_cap->twt_request);
	WMA_LOGI("\tTWT Responder support: 0x%01x", he_cap->twt_responder);
	WMA_LOGI("\tFragmentation support: 0x%02x", he_cap->fragmentation);
	WMA_LOGI("\tMax no.of frag MSDUs: 0x%03x", he_cap->max_num_frag_msdu);
	WMA_LOGI("\tMin. frag size: 0x%02x", he_cap->min_frag_size);
	WMA_LOGI("\tTrigger MAC pad duration: 0x%02x",
			he_cap->trigger_frm_mac_pad);
	WMA_LOGI("\tMulti-TID aggr support: 0x%03x", he_cap->multi_tid_aggr);
	WMA_LOGI("\tLink adaptation: 0x%02x", he_cap->he_link_adaptation);
	WMA_LOGI("\tAll ACK support: 0x%01x", he_cap->all_ack);
	WMA_LOGI("\tUL MU resp. scheduling: 0x%01x", he_cap->ul_mu_rsp_sched);
	WMA_LOGI("\tA-Buff status report: 0x%01x", he_cap->a_bsr);
	WMA_LOGI("\tBroadcast TWT support: 0x%01x", he_cap->broadcast_twt);
	WMA_LOGI("\t32bit BA bitmap support: 0x%01x", he_cap->ba_32bit_bitmap);
	WMA_LOGI("\tMU Cascading support: 0x%01x", he_cap->mu_cascade);
	WMA_LOGI("\tACK enabled Multi-TID: 0x%01x",
			he_cap->ack_enabled_multitid);
	WMA_LOGI("\tMulti-STA BA in DL MU: 0x%01x", he_cap->dl_mu_ba);
	WMA_LOGI("\tOMI A-Control support: 0x%01x", he_cap->omi_a_ctrl);
	WMA_LOGI("\tOFDMA RA support: 0x%01x", he_cap->ofdma_ra);
	WMA_LOGI("\tMax A-MPDU Length: 0x%02x", he_cap->max_ampdu_len);
	WMA_LOGI("\tA-MSDU Fragmentation: 0x%01x", he_cap->amsdu_frag);
	WMA_LOGI("\tFlex. TWT sched support: 0x%01x", he_cap->flex_twt_sched);
	WMA_LOGI("\tRx Ctrl frame to MBSS: 0x%01x", he_cap->rx_ctrl_frame);
	WMA_LOGI("\tBSRP A-MPDU Aggregation: 0x%01x", he_cap->bsrp_ampdu_aggr);
	WMA_LOGI("\tQuite Time Period support: 0x%01x", he_cap->qtp);
	WMA_LOGI("\tA-BQR support: 0x%01x", he_cap->a_bqr);

	/* HE PHY capabilities */
	WMA_LOGI("\tDual band support: 0x%01x", he_cap->dual_band);
	WMA_LOGI("\tChannel width support: 0x%07x", he_cap->chan_width);
	WMA_LOGI("\tPreamble puncturing Rx: 0x%04x",
			he_cap->rx_pream_puncturing);
	WMA_LOGI("\tClass of device: 0x%01x", he_cap->device_class);
	WMA_LOGI("\tLDPC coding support: 0x%01x", he_cap->ldpc_coding);
	WMA_LOGI("\tLTF and GI for HE PPDUs: 0x%02x", he_cap->he_ltf_gi_ppdu);
	WMA_LOGI("\tLTF and GI for NDP: 0x%02x", he_cap->he_ltf_gi_ndp);
	WMA_LOGI("\tSTBC Tx & Rx support: 0x%02x", he_cap->stbc);
	WMA_LOGI("\tDoppler support: 0x%02x", he_cap->doppler);
	WMA_LOGI("\tUL MU: 0x%02x", he_cap->ul_mu);
	WMA_LOGI("\tDCM encoding Tx: 0x%03x", he_cap->dcm_enc_tx);
	WMA_LOGI("\tDCM encoding Tx: 0x%03x", he_cap->dcm_enc_rx);
	WMA_LOGI("\tHE MU PPDU payload support: 0x%01x", he_cap->ul_he_mu);
	WMA_LOGI("\tSU Beamformer: 0x%01x", he_cap->su_beamformer);
	WMA_LOGI("\tSU Beamformee: 0x%01x", he_cap->su_beamformee);
	WMA_LOGI("\tMU Beamformer: 0x%01x", he_cap->mu_beamformer);
	WMA_LOGI("\tBeamformee STS for <= 80Mhz: 0x%03x",
			he_cap->bfee_sts_lt_80);
	WMA_LOGI("\tNSTS total for <= 80Mhz: 0x%03x", he_cap->nsts_tol_lt_80);
	WMA_LOGI("\tBeamformee STS for > 80Mhz: 0x%03x",
			he_cap->bfee_sta_gt_80);
	WMA_LOGI("\tNSTS total for > 80Mhz: 0x%03x", he_cap->nsts_tot_gt_80);
	WMA_LOGI("\tNo. of sounding dim <= 80Mhz: 0x%03x",
			he_cap->num_sounding_lt_80);
	WMA_LOGI("\tNo. of sounding dim > 80Mhz: 0x%03x",
			he_cap->num_sounding_gt_80);
	WMA_LOGI("\tNg=16 for SU feedback support: 0x%01x",
			he_cap->su_feedback_tone16);
	WMA_LOGI("\tNg=16 for MU feedback support: 0x%01x",
			he_cap->mu_feedback_tone16);
	WMA_LOGI("\tCodebook size for SU: 0x%01x", he_cap->codebook_su);
	WMA_LOGI("\tCodebook size for MU: 0x%01x ", he_cap->codebook_mu);
	WMA_LOGI("\tBeamforming trigger w/ Trigger: 0x%01x",
			he_cap->beamforming_feedback);
	WMA_LOGI("\tHE ER SU PPDU payload: 0x%01x", he_cap->he_er_su_ppdu);
	WMA_LOGI("\tDL MUMIMO on partial BW: 0x%01x",
			he_cap->dl_mu_mimo_part_bw);
	WMA_LOGI("\tPPET present: 0x%01x", he_cap->ppet_present);
	WMA_LOGI("\tSRP based SR-support: 0x%01x", he_cap->srp);
	WMA_LOGI("\tPower boost factor: 0x%01x", he_cap->power_boost);
	WMA_LOGI("\t4x HE LTF support: 0x%01x", he_cap->he_ltf_gi_4x);

	WMA_LOGI("\tHighest NSS supported: 0x%03x", he_cap->nss_supported);
	WMA_LOGI("\tHighest MCS supported: 0x%03x", he_cap->mcs_supported);
	WMA_LOGI("\tTX BW bitmap: 0x%05x", he_cap->tx_bw_bitmap);
	WMA_LOGI("\tRX BW bitmap: 0x%05x ", he_cap->rx_bw_bitmap);

	/* HE PPET */
	WMA_LOGI("\tNSS: %d", he_cap->ppe_threshold.nss_count + 1);
	WMA_LOGI("\tRU Index mask: 0x%04x", he_cap->ppe_threshold.ru_idx_mask);
	WMA_LOGI("\tnum_ppet: %d", he_cap->ppe_threshold.num_ppet);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
		he_cap->ppe_threshold.ppet, he_cap->ppe_threshold.num_ppet);
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

	WMA_LOGI(FL("HE PPET: ru_idx_mask: %04x"), ru_bit_mask);
	for (ru_count = 0; ru_bit_mask; ru_bit_mask >>= 1)
		if (ru_bit_mask & 0x1)
			ru_count++;

	if (ru_count > 0) {
		WMA_LOGI(FL("PPET has following RU INDEX,"));
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX0_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 0");
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX1_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 1");
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX2_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 2");
		if (ppet->ru_bit_mask & HE_RU_ALLOC_INDX3_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 3");
	}

	WMA_LOGI(FL("HE PPET: nss: %d, ru_count: %d"), numss, ru_count);

	for (i = 0; i < numss; i++) {
		WMA_LOGI("PPET for NSS[%d]", i);
		for (j = 1; j <= ru_count; j++) {
			WMA_LOGI("\tNSS[%d],RU[%d]: PPET16: %02x PPET8: %02x",
			    i, j,
			    WMI_GET_PPET16(ppet->ppet16_ppet8_ru3_ru0, j, i),
			    WMI_GET_PPET8(ppet->ppet16_ppet8_ru3_ru0, j, i));
		}
	}

}

void wma_print_he_phy_cap(uint32_t *phy_cap)
{
	WMA_LOGI(FL("HE PHY Capabilities:"));

	WMA_LOGI("\tDual band support: 0x%01x",
		WMI_HECAP_PHY_DB_GET(phy_cap));
	WMA_LOGI("\tChannel width support: 0x%07x",
		WMI_HECAP_PHY_CBW_GET(phy_cap));
	WMA_LOGI("\tPreamble puncturing Rx: 0x%04x",
		WMI_HECAP_PHY_PREAMBLEPUNCRX_GET(phy_cap));
	WMA_LOGI("\tClass of device: 0x%01x", WMI_HECAP_PHY_COD_GET(phy_cap));
	WMA_LOGI("\tLDPC coding support: 0x%01x",
		WMI_HECAP_PHY_LDPC_GET(phy_cap));
	WMA_LOGI("\tLTF and GI for HE PPDUs: 0x%02x",
		WMI_HECAP_PHY_LTFGIFORHE_GET(phy_cap));
	WMA_LOGI("\tLTF and GI for NDP: 0x%02x",
		WMI_HECAP_PHY_LTFGIFORNDP_GET(phy_cap));
	WMA_LOGI("\tSTBC Tx & Rx support: 0x%02x",
			(WMI_HECAP_PHY_RXSTBC_GET(phy_cap) << 1) |
			 WMI_HECAP_PHY_TXSTBC_GET(phy_cap));
	WMA_LOGI("\tDoppler support: 0x%02x",
			(WMI_HECAP_PHY_RXDOPPLER_GET(phy_cap) << 1) |
			 WMI_HECAP_PHY_TXDOPPLER_GET(phy_cap));
	WMA_LOGI("\tUL MU: 0x%02x", WMI_HECAP_PHY_UL_MU_MIMO_GET(phy_cap));
	WMA_LOGI("\tDCM encoding Tx: 0x%03x", WMI_HECAP_PHY_DCMTX_GET(phy_cap));
	WMA_LOGI("\tDCM encoding Tx: 0x%03x", WMI_HECAP_PHY_DCMRX_GET(phy_cap));
	WMA_LOGI("\tHE MU PPDU payload support: 0x%01x",
		WMI_HECAP_PHY_ULHEMU_GET(phy_cap));
	WMA_LOGI("\tSU Beamformer: 0x%01x", WMI_HECAP_PHY_SUBFMR_GET(phy_cap));
	WMA_LOGI("\tSU Beamformee: 0x%01x", WMI_HECAP_PHY_SUBFME_GET(phy_cap));
	WMA_LOGI("\tMU Beamformer: 0x%01x", WMI_HECAP_PHY_MUBFMR_GET(phy_cap));
	WMA_LOGI("\tBeamformee STS for <= 80Mhz: 0x%03x",
			WMI_HECAP_PHY_SUBFMESTS_GET(phy_cap));
	WMA_LOGI("\tNSTS total for <= 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NSTSLT80MHZ_GET(phy_cap));
	WMA_LOGI("\tBeamformee STS for > 80Mhz: 0x%03x",
		WMI_HECAP_PHY_BFMESTSGT80MHZ_GET(phy_cap));
	WMA_LOGI("\tNSTS total for > 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NSTSGT80MHZ_GET(phy_cap));
	WMA_LOGI("\tNo. of sounding dim <= 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NUMSOUNDLT80MHZ_GET(phy_cap));
	WMA_LOGI("\tNo. of sounding dim > 80Mhz: 0x%03x",
		WMI_HECAP_PHY_NUMSOUNDGT80MHZ_GET(phy_cap));
	WMA_LOGI("\tNg=16 for SU feedback support: 0x%01x",
		WMI_HECAP_PHY_NG16SUFEEDBACKLT80_GET(phy_cap));
	WMA_LOGI("\tNg=16 for MU feedback support: 0x%01x",
		WMI_HECAP_PHY_NG16MUFEEDBACKGT80_GET(phy_cap));
	WMA_LOGI("\tCodebook size for SU: 0x%01x",
		WMI_HECAP_PHY_CODBK42SU_GET(phy_cap));
	WMA_LOGI("\tCodebook size for MU: 0x%01x ",
		WMI_HECAP_PHY_CODBK75MU_GET(phy_cap));
	WMA_LOGI("\tBeamforming trigger w/ Trigger: 0x%01x",
		WMI_HECAP_PHY_BFFEEDBACKTRIG_GET(phy_cap));
	WMA_LOGI("\tHE ER SU PPDU payload: 0x%01x",
		WMI_HECAP_PHY_HEERSU_GET(phy_cap));
	WMA_LOGI("\tDL MUMIMO on partial BW: 0x%01x",
		WMI_HECAP_PHY_DLMUMIMOPARTIALBW_GET(phy_cap));
	WMA_LOGI("\tPPET present: 0x%01x", WMI_HECAP_PHY_PADDING_GET(phy_cap));
	WMA_LOGI("\tSRP based SR-support: 0x%01x",
		WMI_HECAP_PHY_SRPSPRESENT_GET(phy_cap));
	WMA_LOGI("\tPower boost factor: 0x%01x",
		WMI_HECAP_PHY_PWRBOOSTAR_GET(phy_cap));
	WMA_LOGI("\t4x HE LTF support: 0x%01x",
		WMI_HECAP_PHY_4XLTFAND800NSECSGI_GET(phy_cap));
}

void wma_print_he_mac_cap(uint32_t mac_cap)
{
	WMA_LOGI(FL("HE MAC Capabilities:"));

	WMA_LOGI("\tHTC-HE conrol: 0x%01x", WMI_HECAP_MAC_HECTRL_GET(mac_cap));
	WMA_LOGI("\tTWT Requestor support: 0x%01x",
			WMI_HECAP_MAC_TWTREQ_GET(mac_cap));
	WMA_LOGI("\tTWT Responder support: 0x%01x",
			WMI_HECAP_MAC_TWTRSP_GET(mac_cap));
	WMA_LOGI("\tFragmentation support: 0x%02x",
			WMI_HECAP_MAC_HEFRAG_GET(mac_cap));
	WMA_LOGI("\tMax no.of frag MSDUs: 0x%03x",
			WMI_HECAP_MAC_MAXFRAGMSDU_GET(mac_cap));
	WMA_LOGI("\tMin. frag size: 0x%02x",
			WMI_HECAP_MAC_MINFRAGSZ_GET(mac_cap));
	WMA_LOGI("\tTrigger MAC pad duration: 0x%02x",
			WMI_HECAP_MAC_TRIGPADDUR_GET(mac_cap));
	WMA_LOGI("\tMulti-TID aggr support: 0x%03x",
			WMI_HECAP_MAC_ACKMTIDAMPDU_GET(mac_cap));
	WMA_LOGI("\tLink adaptation: 0x%02x",
			WMI_HECAP_MAC_HELKAD_GET(mac_cap));
	WMA_LOGI("\tAll ACK support: 0x%01x",
			WMI_HECAP_MAC_AACK_GET(mac_cap));
	WMA_LOGI("\tUL MU resp. scheduling: 0x%01x",
			WMI_HECAP_MAC_ULMURSP_GET(mac_cap));
	WMA_LOGI("\tA-Buff status report: 0x%01x",
			WMI_HECAP_MAC_BSR_GET(mac_cap));
	WMA_LOGI("\tBroadcast TWT support: 0x%01x",
			WMI_HECAP_MAC_BCSTTWT_GET(mac_cap));
	WMA_LOGI("\t32bit BA bitmap support: 0x%01x",
			WMI_HECAP_MAC_32BITBA_GET(mac_cap));
	WMA_LOGI("\tMU Cascading support: 0x%01x",
			WMI_HECAP_MAC_MUCASCADE_GET(mac_cap));
	WMA_LOGI("\tACK enabled Multi-TID: 0x%01x",
			WMI_HECAP_MAC_ACKMTIDAMPDU_GET(mac_cap));
	WMA_LOGI("\tMulti-STA BA in DL MU: 0x%01x",
			WMI_HECAP_MAC_GROUPMSTABA_GET(mac_cap));
	WMA_LOGI("\tOMI A-Control support: 0x%01x",
			WMI_HECAP_MAC_OMI_GET(mac_cap));
	WMA_LOGI("\tOFDMA RA support: 0x%01x",
			WMI_HECAP_MAC_OFDMARA_GET(mac_cap));
	WMA_LOGI("\tMax A-MPDU Length: 0x%02x",
		WMI_HECAP_MAC_MAXAMPDULEN_EXP_GET(mac_cap));
	WMA_LOGI("\tA-MSDU Fragmentation: 0x%01x",
		WMI_HECAP_MAC_AMSDUFRAG_GET(mac_cap));
	WMA_LOGI("\tFlex. TWT sched support: 0x%01x",
		WMI_HECAP_MAC_FLEXTWT_GET(mac_cap));
	WMA_LOGI("\tRx Ctrl frame to MBSS: 0x%01x",
			WMI_HECAP_MAC_MBSS_GET(mac_cap));
	WMA_LOGI("\tBSRP A-MPDU Aggregation: 0x%01x",
		WMI_HECAP_MAC_BSRPAMPDU_GET(mac_cap));
	WMA_LOGI("\tQuite Time Period support: 0x%01x",
		WMI_HECAP_MAC_QTP_GET(mac_cap));
	WMA_LOGI("\tA-BQR support: 0x%01x", WMI_HECAP_MAC_ABQR_GET(mac_cap));
}

void wma_update_target_ext_he_cap(tp_wma_handle wma_handle,
				  struct wma_tgt_cfg *tgt_cfg)
{
	tDot11fIEvendor_he_cap *he_cap = &tgt_cfg->he_cap;
	int i, j = 0, max_mac;
	struct extended_caps *phy_caps;
	WMI_MAC_PHY_CAPABILITIES *mac_cap;
	tDot11fIEvendor_he_cap he_cap_mac;
	tDot11fIEvendor_he_cap tmp_he_cap = {0};

	if (!wma_handle ||
		(0 == wma_handle->phy_caps.num_hw_modes.num_hw_modes)) {
		WMA_LOGE(FL("No extended HE cap for current SOC"));
		he_cap->present = false;
		return;
	}

	if (!tgt_cfg->services.en_11ax) {
		WMA_LOGI(FL("Target does not support 11AX"));
		he_cap->present = false;
		return;
	}

	phy_caps = &wma_handle->phy_caps;
	for (i = 0; i < phy_caps->num_hw_modes.num_hw_modes; i++) {
		if (phy_caps->each_hw_mode_cap[i].phy_id_map == PHY1_PHY2)
			max_mac = j + 2;
		else
			max_mac = j + 1;
		for ( ; j < max_mac; j++) {
			qdf_mem_zero(&he_cap_mac,
				     sizeof(tDot11fIEvendor_he_cap));
			mac_cap = &phy_caps->each_phy_cap_per_hwmode[j];
			if (mac_cap->supported_bands & WLAN_2G_CAPABILITY) {
				wma_convert_he_cap(&he_cap_mac,
						mac_cap->he_cap_info_2G,
						mac_cap->he_cap_phy_info_2G,
						(uint8_t *)&mac_cap->he_ppet2G,
						mac_cap->he_supp_mcs_2G,
						mac_cap->tx_chain_mask_2G,
						mac_cap->rx_chain_mask_2G);

			}

			if (he_cap_mac.present)
				wma_derive_ext_he_cap(wma_handle, &tmp_he_cap,
					&he_cap_mac);

			qdf_mem_zero(&he_cap_mac,
				     sizeof(tDot11fIEvendor_he_cap));
			if (mac_cap->supported_bands & WLAN_5G_CAPABILITY) {
				wma_convert_he_cap(&he_cap_mac,
						mac_cap->he_cap_info_5G,
						mac_cap->he_cap_phy_info_5G,
						(uint8_t *)&mac_cap->he_ppet5G,
						mac_cap->he_supp_mcs_5G,
						mac_cap->tx_chain_mask_5G,
						mac_cap->rx_chain_mask_5G);

			}

			if (he_cap_mac.present)
				wma_derive_ext_he_cap(wma_handle, &tmp_he_cap,
					&he_cap_mac);
		}
	}

	qdf_mem_copy(he_cap, &tmp_he_cap, sizeof(*he_cap));
	wma_print_he_cap(he_cap);
}

void wma_he_update_tgt_services(tp_wma_handle wma, struct wma_tgt_services *cfg)
{
	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap, WMI_SERVICE_11AX)) {
		cfg->en_11ax = true;
		wma_set_fw_wlan_feat_caps(DOT11AX);
		WMA_LOGI(FL("11ax is enabled"));
	} else {
		WMA_LOGI(FL("11ax is not enabled"));
	}
}

void wma_print_he_op(tDot11fIEvendor_he_op *he_ops)
{
	WMA_LOGI(FL("bss_color: %0x, default_pe_duration: %0x, twt_required: %0x, rts_threshold: %0x"),
		he_ops->bss_color, he_ops->default_pe,
		he_ops->twt_required, he_ops->rts_threshold);
	WMA_LOGI(("\tpartial_bss_color: %0x, MaxBSSID Indicator: %0x, Tx BSSID Indicator: %0x, BSS color disabled: %0x, Dual beacon: %0x"),
		he_ops->partial_bss_col, he_ops->maxbssid_ind,
		he_ops->tx_bssid_ind, he_ops->bss_col_disabled,
		he_ops->dual_beacon);
}

/**
 * wma_parse_he_ppet() - Convert PPET stored in dot11f structure into FW
 *                       structure.
 * @dot11f_ppet: pointer to dot11f format PPET
 * @peer_ppet: pointer peer_ppet to be sent in peer assoc
 *
 * This function converts the sequence of PPET stored in the host in OTA type
 * structure into FW understandable structure to be sent as part of peer assoc
 * command.
 *
 * Return: None
 */
static void wma_parse_he_ppet(tDot11fIEppe_threshold *dot11f_ppet,
			      struct wmi_host_ppe_threshold *peer_ppet)
{
	uint8_t num_ppet, mask, mask1, mask2;
	uint32_t ppet1, ppet2, ppet;
	uint8_t bits, pad, pad_bits, req_byte;
	uint8_t byte_idx, start, i, j, parsed;
	uint32_t *ppet_r = peer_ppet->ppet16_ppet8_ru3_ru0;
	uint8_t *rcvd_ppet;
	uint8_t nss, ru;

	nss = dot11f_ppet->nss_count + 1;
	mask = dot11f_ppet->ru_idx_mask;

	for (ru = 0; mask; mask >>= 1) {
		if (mask & 0x1)
			ru++;
	}

	WMA_LOGI(FL("Rcvd nss=%d ru_idx_mask: %0x ru_count=%d"),
		 nss, mask, ru);

	/* rcvd_ppet will store the ppet array and first byte of the ppet */
	rcvd_ppet = qdf_mem_malloc(sizeof(*rcvd_ppet) *
				  (dot11f_ppet->num_ppet + 1));
	if (!rcvd_ppet) {
		WMA_LOGE(FL("mem alloc failed"));
		return;
	}

	rcvd_ppet[0] = (dot11f_ppet->ppet_b1 << 7);
	qdf_mem_copy(&rcvd_ppet[1], dot11f_ppet->ppet, dot11f_ppet->num_ppet);

	peer_ppet->numss_m1 = nss - 1;
	peer_ppet->ru_bit_mask = dot11f_ppet->ru_idx_mask;

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
	parsed = 7;

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
			WMA_LOGI(FL("nss:%d ru:%d ppet_r:%0x"), i, j/2,
				 ppet_r[i]);
		}
	}

	qdf_mem_free(rcvd_ppet);
}

void wma_populate_peer_he_cap(struct peer_assoc_params *peer,
			      tpAddStaParams params)
{
	tDot11fIEvendor_he_cap *he_cap = &params->he_config;
	tDot11fIEvendor_he_op *he_op = &params->he_op;
	uint32_t *phy_cap = peer->peer_he_cap_phyinfo;
	uint32_t mac_cap = 0, he_ops = 0;
	uint8_t temp, i;

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
	peer->peer_he_cap_macinfo = mac_cap;

	/* HE PHY capabilities */
	WMI_HECAP_PHY_DB_SET(phy_cap, he_cap->dual_band);
	WMI_HECAP_PHY_CBW_SET(phy_cap, he_cap->chan_width);
	WMI_HECAP_PHY_PREAMBLEPUNCRX_SET(phy_cap, he_cap->rx_pream_puncturing);
	WMI_HECAP_PHY_COD_SET(phy_cap, he_cap->device_class);
	WMI_HECAP_PHY_LDPC_SET(phy_cap, he_cap->ldpc_coding);
	WMI_HECAP_PHY_LTFGIFORHE_SET(phy_cap, he_cap->he_ltf_gi_ppdu);
	WMI_HECAP_PHY_LTFGIFORNDP_SET(phy_cap, he_cap->he_ltf_gi_ndp);

	temp = he_cap->stbc & 0x1;
	WMI_HECAP_PHY_RXSTBC_SET(phy_cap, temp);
	temp = he_cap->stbc >> 0x1;
	WMI_HECAP_PHY_TXSTBC_SET(phy_cap, temp);

	temp = he_cap->doppler & 0x1;
	WMI_HECAP_PHY_RXDOPPLER_SET(phy_cap, temp);
	temp = he_cap->doppler >> 0x1;
	WMI_HECAP_PHY_TXDOPPLER_SET(phy_cap, temp);

	WMI_HECAP_PHY_UL_MU_MIMO_SET(phy_cap, he_cap->ul_mu);
	WMI_HECAP_PHY_DCMTX_SET(phy_cap, he_cap->dcm_enc_tx);
	WMI_HECAP_PHY_DCMRX_SET(phy_cap, he_cap->dcm_enc_rx);
	WMI_HECAP_PHY_ULHEMU_SET(phy_cap, he_cap->ul_he_mu);
	WMI_HECAP_PHY_SUBFMR_SET(phy_cap, he_cap->su_beamformer);
	WMI_HECAP_PHY_SUBFME_SET(phy_cap, he_cap->su_beamformee);
	WMI_HECAP_PHY_MUBFMR_SET(phy_cap, he_cap->mu_beamformer);
	WMI_HECAP_PHY_BFMESTSLT80MHZ_SET(phy_cap, he_cap->bfee_sts_lt_80);
	WMI_HECAP_PHY_NSTSLT80MHZ_SET(phy_cap, he_cap->nsts_tol_lt_80);
	WMI_HECAP_PHY_BFMESTSGT80MHZ_SET(phy_cap, he_cap->bfee_sta_gt_80);
	WMI_HECAP_PHY_NSTSGT80MHZ_SET(phy_cap, he_cap->nsts_tot_gt_80);
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
	WMI_HECAP_PHY_4XLTFAND800NSECSGI_SET(phy_cap, he_cap->he_ltf_gi_4x);

	/* until further update in standard */
	peer->peer_he_mcs_count = WMI_HOST_MAX_HE_RATE_SET;
	for (i = 0; i < peer->peer_he_mcs_count; i++) {
		peer->peer_he_rx_mcs_set[i] = params->supportedRates.he_rx_mcs;
		peer->peer_he_tx_mcs_set[i] = params->supportedRates.he_tx_mcs;

		WMA_LOGD(FL("[HE - MCS Map: %d] rx_mcs: %x, tx_mcs: %x"), i,
			 peer->peer_he_rx_mcs_set[i],
			 peer->peer_he_tx_mcs_set[i]);
	}

	WMI_HEOPS_COLOR_SET(he_ops, he_op->bss_color);
	WMI_HEOPS_DEFPE_SET(he_ops, he_op->default_pe);
	WMI_HEOPS_TWT_SET(he_ops, he_op->twt_required);
	WMI_HEOPS_RTSTHLD_SET(he_ops, he_op->rts_threshold);
	WMI_HEOPS_PARTBSSCOLOR_SET(he_ops, he_op->partial_bss_col);
	WMI_HEOPS_MAXBSSID_SET(he_ops, he_op->maxbssid_ind);
	WMI_HEOPS_TXBSSID_SET(he_ops, he_op->tx_bssid_ind);
	WMI_HEOPS_BSSCOLORDISABLE_SET(he_ops, he_op->bss_col_disabled);
	WMI_HEOPS_DUALBEACON_SET(he_ops, he_op->dual_beacon);
	peer->peer_he_ops = he_ops;

	wma_parse_he_ppet(&he_cap->ppe_threshold, &peer->peer_ppet);

	wma_print_he_cap(he_cap);
	WMA_LOGI(FL("Peer HE Capabilities:"));
	wma_print_he_phy_cap(phy_cap);
	wma_print_he_mac_cap(mac_cap);
	wma_print_he_ppet(&peer->peer_ppet);

	return;
}

void wma_update_vdev_he_ops(struct wma_vdev_start_req *req,
		tpAddBssParams add_bss)
{
	uint32_t he_ops = 0;
	tDot11fIEvendor_he_op *he_op = &add_bss->he_op;

	req->he_capable = add_bss->he_capable;

	WMI_HEOPS_COLOR_SET(he_ops, he_op->bss_color);
	WMI_HEOPS_DEFPE_SET(he_ops, he_op->default_pe);
	WMI_HEOPS_TWT_SET(he_ops, he_op->twt_required);
	WMI_HEOPS_RTSTHLD_SET(he_ops, he_op->rts_threshold);
	WMI_HEOPS_PARTBSSCOLOR_SET(he_ops, he_op->partial_bss_col);
	WMI_HEOPS_MAXBSSID_SET(he_ops, he_op->maxbssid_ind);
	WMI_HEOPS_TXBSSID_SET(he_ops, he_op->tx_bssid_ind);
	WMI_HEOPS_BSSCOLORDISABLE_SET(he_ops, he_op->bss_col_disabled);
	WMI_HEOPS_DUALBEACON_SET(he_ops, he_op->dual_beacon);

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
	WMA_LOGI(FL("HE_STA_OBSSPD: PD_MIN: %d PD_MAX: %d SEC_CH_ED: %d TX_PWR: %d"),
		 pd_min, pd_max, sec_ch_ed, tx_pwr);
}

void wma_update_vdev_he_capable(struct wma_vdev_start_req *req,
		tpSwitchChannelParams params)
{
	req->he_capable = params->he_capable;
}

QDF_STATUS wma_update_he_ops_ie(tp_wma_handle wma, uint8_t vdev_id,
					   uint32_t he_ops)
{
	QDF_STATUS ret;

	if (!wma) {
		WMA_LOGE(FL("wrong wma_handle...."));
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_HEOPS_0_31, he_ops);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE(FL("Failed to set HE OPs"));
	else
		WMA_LOGD(FL("Succesfully send he_ops[0x%x]"), he_ops);

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
