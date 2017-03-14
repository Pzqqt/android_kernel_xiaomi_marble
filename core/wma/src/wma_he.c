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
				wmi_ppe_threshold *ppet)
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
	he_ppet->ru_idx_mask = ppet->ru_mask;

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
 * @ppet: Received HE PPE threshold
 * @mcs: Max MCS supported (Tx/Rx)
 * @nss: Max NSS supported (Tx/Rx)
 *
 * This function converts various HE capability received as part of extended
 * service ready event into dot11f structure. GET macros are defined at WMI
 * layer, use them to unpack the incoming FW capability.
 *
 * Return: None
 */
static void wma_convert_he_cap(tDot11fIEvendor_he_cap *he_cap, uint32_t mac_cap,
			       uint32_t *phy_cap, wmi_ppe_threshold *ppet,
			       uint8_t mcs, uint8_t nss)
{
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

		/* Remaining capabilities are not captured here.
		 * Intersect only the required capability
		 */
	}
}

/**
 * @wma_print_he_cap() - Print HE capabilities
 * @he_cap: pointer to HE Capability
 *
 * Received HE capabilities are converted into dot11f structure.
 * This function will print all the HE capabilities as stored
 * in the dot11f structure.
 *
 * Return: None
 */
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

/**
 * wma_print_he_ppet() - Prints HE PPE Threshold
 * @ppet: PPE Threshold
 *
 * This function prints HE PPE Threshold as received from FW.
 * Refer to the definition of wmi_ppe_threshold to understand
 * how PPE thresholds are packed by FW for a given NSS and RU.
 *
 * Return: none
 */
void wma_print_he_ppet(wmi_ppe_threshold *ppet)
{
	int numss, ru_count, ru_mask, i, j;

	if (!ppet) {
		WMA_LOGI(FL("PPET is NULL"));
		return;
	}

	numss = ppet->numss_m1 + 1;
	ru_mask = ppet->ru_mask;

	WMA_LOGI(FL("HE PPET: ru_idx_mask: %04x"), ru_mask);
	for (ru_count = 0; ru_mask; ru_mask >>= 1)
		if (ru_mask & 0x1)
			ru_count++;

	if (ru_count > 0) {
		WMA_LOGI(FL("PPET has following RU INDEX,"));
		if (ppet->ru_mask & HE_RU_ALLOC_INDX0_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 0");
		if (ppet->ru_mask & HE_RU_ALLOC_INDX1_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 1");
		if (ppet->ru_mask & HE_RU_ALLOC_INDX2_MASK)
			WMA_LOGI("\tRU ALLOCATION INDEX 2");
		if (ppet->ru_mask & HE_RU_ALLOC_INDX3_MASK)
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

/**
 * wma_print_he_phy_cap() - Print HE PHY Capability
 * @phy_cap: pointer to PHY Capability
 *
 * This function prints HE PHY Capability received from FW.
 *
 * Return: none
 */
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

/**
 * wma_print_he_mac_cap() - Print HE MAC Capability
 * @mac_cap: MAC Capability
 *
 * This function prints HE MAC Capability received from FW.
 *
 * Return: none
 */
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

/**
 * wma_update_target_ext_he_cap() - Update HE caps with given extended cap
 * @wma_handle: pointer to wma_handle
 * @tgt_cfg: Target config
 *
 * This function loop through each hardware mode and for each hardware mode
 * again it loop through each MAC/PHY and pull the caps 2G and 5G specific
 * HE caps and derives the final cap.
 *
 * Return: none
 *
 */
void wma_update_target_ext_he_cap(tp_wma_handle wma_handle,
				  struct wma_tgt_cfg *tgt_cfg)
{
	tDot11fIEvendor_he_cap *he_cap = &tgt_cfg->he_cap;
	int i, j = 0, max_mac;
	uint32_t he_mac;
	uint32_t he_phy[WMI_MAX_HECAP_PHY_SIZE];
	wmi_ppe_threshold he_ppet;
	struct extended_caps *phy_caps;
	WMI_MAC_PHY_CAPABILITIES *mac_cap;
	tDot11fIEvendor_he_cap he_cap_mac0 = {0}, he_cap_mac1 = {0};
	tDot11fIEvendor_he_cap tmp_he_cap = {0};
	uint8_t mcs, nss;

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
			mac_cap = &phy_caps->each_phy_cap_per_hwmode[j];
			he_mac = mac_cap->he_cap_info_2G;
			qdf_mem_copy(he_phy, mac_cap->he_cap_phy_info_2G,
				     WMI_MAX_HECAP_PHY_SIZE * 4);
			he_ppet = mac_cap->he_ppet2G;
			mcs = mac_cap->he_supp_mcs_2G;
			nss = (mac_cap->tx_chain_mask_2G >
				mac_cap->rx_chain_mask_2G) ?
					mac_cap->tx_chain_mask_2G :
					mac_cap->rx_chain_mask_2G;
			wma_convert_he_cap(&he_cap_mac0, he_mac, he_phy,
					   &he_ppet, mcs, nss);
			if (he_cap_mac0.present)
				wma_derive_ext_he_cap(wma_handle, &tmp_he_cap,
					&he_cap_mac0);

			he_mac = mac_cap->he_cap_info_5G;
			qdf_mem_copy(he_phy, mac_cap->he_cap_phy_info_5G,
				     WMI_MAX_HECAP_PHY_SIZE * 4);
			he_ppet = mac_cap->he_ppet5G;
			mcs = mac_cap->he_supp_mcs_5G;
			nss = (mac_cap->tx_chain_mask_5G >
				mac_cap->rx_chain_mask_5G) ?
					mac_cap->tx_chain_mask_5G :
					mac_cap->rx_chain_mask_5G;
			wma_convert_he_cap(&he_cap_mac1, he_mac, he_phy,
					   &he_ppet, mcs, nss);
			if (he_cap_mac1.present)
				wma_derive_ext_he_cap(wma_handle, &tmp_he_cap,
					&he_cap_mac1);
		}
	}

	qdf_mem_copy(he_cap, &tmp_he_cap, sizeof(*he_cap));
	wma_print_he_cap(he_cap);
}

/*
 * wma_he_update_tgt_services() - update tgt cfg to indicate 11ax support
 * @wma: pointer to WMA handle
 * @cfg: pointer to WMA target services
 *
 * Based on WMI SERVICES information, enable 11ax support and set DOT11AX bit
 * in feature caps bitmap.
 *
 * Return: None
 */
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
