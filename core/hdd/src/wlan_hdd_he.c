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
 * DOC : wlan_hdd_he.c
 *
 * WLAN Host Device Driver file for 802.11ax (High Efficiency) support.
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_he.h"
#include "wma_he.h"

/**
 * hdd_he_wni_cfg_to_string() - return string conversion of HE WNI CFG
 * @cfg_id: Config ID.
 *
 * This utility function helps log string conversion of WNI config ID.
 *
 * Return: string conversion of the HE WNI config ID, if match found;
 *	"Invalid" otherwise.
 */
static const char *hdd_he_wni_cfg_to_string(uint16_t cfg_id)
{
	switch (cfg_id) {
	default:
		return "Invalid";
	CASE_RETURN_STRING(WNI_CFG_HE_CONTROL);
	CASE_RETURN_STRING(WNI_CFG_HE_TWT_REQUESTOR);
	CASE_RETURN_STRING(WNI_CFG_HE_TWT_RESPONDER);
	CASE_RETURN_STRING(WNI_CFG_HE_FRAGMENTATION);
	CASE_RETURN_STRING(WNI_CFG_HE_MAX_FRAG_MSDU);
	CASE_RETURN_STRING(WNI_CFG_HE_MIN_FRAG_SIZE);
	CASE_RETURN_STRING(WNI_CFG_HE_TRIG_PAD);
	CASE_RETURN_STRING(WNI_CFG_HE_MTID_AGGR);
	CASE_RETURN_STRING(WNI_CFG_HE_LINK_ADAPTATION);
	CASE_RETURN_STRING(WNI_CFG_HE_ALL_ACK);
	CASE_RETURN_STRING(WNI_CFG_HE_UL_MU_RSP_SCHEDULING);
	CASE_RETURN_STRING(WNI_CFG_HE_BUFFER_STATUS_RPT);
	CASE_RETURN_STRING(WNI_CFG_HE_BCAST_TWT);
	CASE_RETURN_STRING(WNI_CFG_HE_BA_32BIT);
	CASE_RETURN_STRING(WNI_CFG_HE_MU_CASCADING);
	CASE_RETURN_STRING(WNI_CFG_HE_MULTI_TID);
	CASE_RETURN_STRING(WNI_CFG_HE_DL_MU_BA);
	CASE_RETURN_STRING(WNI_CFG_HE_OMI);
	CASE_RETURN_STRING(WNI_CFG_HE_OFDMA_RA);
	CASE_RETURN_STRING(WNI_CFG_HE_MAX_AMPDU_LEN);
	CASE_RETURN_STRING(WNI_CFG_HE_AMSDU_FRAG);
	CASE_RETURN_STRING(WNI_CFG_HE_FLEX_TWT_SCHED);
	CASE_RETURN_STRING(WNI_CFG_HE_RX_CTRL);
	CASE_RETURN_STRING(WNI_CFG_HE_BSRP_AMPDU_AGGR);
	CASE_RETURN_STRING(WNI_CFG_HE_QTP);
	CASE_RETURN_STRING(WNI_CFG_HE_A_BQR);
	CASE_RETURN_STRING(WNI_CFG_HE_DUAL_BAND);
	CASE_RETURN_STRING(WNI_CFG_HE_CHAN_WIDTH);
	CASE_RETURN_STRING(WNI_CFG_HE_RX_PREAM_PUNC);
	CASE_RETURN_STRING(WNI_CFG_HE_CLASS_OF_DEVICE);
	CASE_RETURN_STRING(WNI_CFG_HE_LDPC);
	CASE_RETURN_STRING(WNI_CFG_HE_LTF_PPDU);
	CASE_RETURN_STRING(WNI_CFG_HE_LTF_NDP);
	CASE_RETURN_STRING(WNI_CFG_HE_STBC);
	CASE_RETURN_STRING(WNI_CFG_HE_DOPPLER);
	CASE_RETURN_STRING(WNI_CFG_HE_UL_MUMIMO);
	CASE_RETURN_STRING(WNI_CFG_HE_DCM_TX);
	CASE_RETURN_STRING(WNI_CFG_HE_DCM_RX);
	CASE_RETURN_STRING(WNI_CFG_HE_MU_PPDU);
	CASE_RETURN_STRING(WNI_CFG_HE_SU_BEAMFORMER);
	CASE_RETURN_STRING(WNI_CFG_HE_SU_BEAMFORMEE);
	CASE_RETURN_STRING(WNI_CFG_HE_MU_BEAMFORMER);
	CASE_RETURN_STRING(WNI_CFG_HE_BFEE_STS_LT80);
	CASE_RETURN_STRING(WNI_CFG_HE_NSTS_TOT_LT80);
	CASE_RETURN_STRING(WNI_CFG_HE_BFEE_STS_GT80);
	CASE_RETURN_STRING(WNI_CFG_HE_NSTS_TOT_GT80);
	CASE_RETURN_STRING(WNI_CFG_HE_NUM_SOUND_LT80);
	CASE_RETURN_STRING(WNI_CFG_HE_NUM_SOUND_GT80);
	CASE_RETURN_STRING(WNI_CFG_HE_SU_FEED_TONE16);
	CASE_RETURN_STRING(WNI_CFG_HE_MU_FEED_TONE16);
	CASE_RETURN_STRING(WNI_CFG_HE_CODEBOOK_SU);
	CASE_RETURN_STRING(WNI_CFG_HE_CODEBOOK_MU);
	CASE_RETURN_STRING(WNI_CFG_HE_BFRM_FEED);
	CASE_RETURN_STRING(WNI_CFG_HE_ER_SU_PPDU);
	CASE_RETURN_STRING(WNI_CFG_HE_DL_PART_BW);
	CASE_RETURN_STRING(WNI_CFG_HE_PPET_PRESENT);
	CASE_RETURN_STRING(WNI_CFG_HE_SRP);
	CASE_RETURN_STRING(WNI_CFG_HE_POWER_BOOST);
	CASE_RETURN_STRING(WNI_CFG_HE_4x_LTF_GI);
	CASE_RETURN_STRING(WNI_CFG_HE_NSS);
	CASE_RETURN_STRING(WNI_CFG_HE_MCS);
	CASE_RETURN_STRING(WNI_CFG_HE_PPET);
	}
}

/**
 * hdd_he_set_wni_cfg() - Update WNI CFG
 * @hdd_ctx: HDD context
 * @cfg_id: CFG to be udpated
 * @new_value: Value to be updated
 *
 * Update WNI CFG with the value passed.
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_he_set_wni_cfg(struct hdd_context_s *hdd_ctx,
				     uint16_t cfg_id, uint32_t new_value)
{
	QDF_STATUS status;

	status = sme_cfg_set_int(hdd_ctx->hHal, cfg_id, new_value);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("could not set %s", hdd_he_wni_cfg_to_string(cfg_id));

	return qdf_status_to_os_return(status);
}

void hdd_update_tgt_he_cap(struct hdd_context_s *hdd_ctx,
			   struct wma_tgt_cfg *cfg)
{
	uint32_t ppet_size = sizeof(tDot11fIEppe_threshold);
	QDF_STATUS status;
	tDot11fIEvendor_he_cap *he_cap = &cfg->he_cap;

	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_CONTROL, he_cap->htc_he);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_TWT_REQUESTOR,
			   he_cap->twt_request);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_TWT_RESPONDER,
			   he_cap->twt_responder);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_FRAGMENTATION,
			   he_cap->fragmentation);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MAX_FRAG_MSDU,
			   he_cap->max_num_frag_msdu);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MIN_FRAG_SIZE,
			   he_cap->min_frag_size);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_TRIG_PAD,
			   he_cap->trigger_frm_mac_pad);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MTID_AGGR,
			   he_cap->multi_tid_aggr);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_LINK_ADAPTATION,
			   he_cap->he_link_adaptation);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_ALL_ACK, he_cap->all_ack);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_UL_MU_RSP_SCHEDULING,
			   he_cap->ul_mu_rsp_sched);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BUFFER_STATUS_RPT,
			   he_cap->a_bsr);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BCAST_TWT,
			   he_cap->broadcast_twt);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BA_32BIT,
			   he_cap->ba_32bit_bitmap);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MU_CASCADING,
			   he_cap->mu_cascade);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MULTI_TID,
			   he_cap->ack_enabled_multitid);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_DL_MU_BA, he_cap->dl_mu_ba);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_OMI, he_cap->omi_a_ctrl);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_OFDMA_RA, he_cap->ofdma_ra);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MAX_AMPDU_LEN,
			   he_cap->max_ampdu_len);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_AMSDU_FRAG, he_cap->amsdu_frag);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_FLEX_TWT_SCHED,
			   he_cap->flex_twt_sched);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_RX_CTRL, he_cap->rx_ctrl_frame);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BSRP_AMPDU_AGGR,
			   he_cap->bsrp_ampdu_aggr);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_QTP, he_cap->qtp);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_A_BQR, he_cap->a_bqr);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_DUAL_BAND, he_cap->dual_band);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_CHAN_WIDTH, he_cap->chan_width);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_RX_PREAM_PUNC,
			   he_cap->rx_pream_puncturing);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_CLASS_OF_DEVICE,
			   he_cap->device_class);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_LDPC, he_cap->ldpc_coding);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_LTF_PPDU,
			   he_cap->he_ltf_gi_ppdu);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_LTF_NDP, he_cap->he_ltf_gi_ndp);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_STBC, he_cap->stbc);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_DOPPLER, he_cap->doppler);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_UL_MUMIMO, he_cap->ul_mu);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_DCM_TX, he_cap->dcm_enc_tx);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_DCM_RX, he_cap->dcm_enc_rx);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MU_PPDU, he_cap->ul_he_mu);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_SU_BEAMFORMER,
			   he_cap->su_beamformer);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_SU_BEAMFORMEE,
			   he_cap->su_beamformee);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MU_BEAMFORMER,
			   he_cap->mu_beamformer);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BFEE_STS_LT80,
			   he_cap->bfee_sts_lt_80);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_NSTS_TOT_LT80,
			   he_cap->nsts_tol_lt_80);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BFEE_STS_GT80,
			   he_cap->bfee_sta_gt_80);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_NSTS_TOT_GT80,
			   he_cap->nsts_tot_gt_80);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_NUM_SOUND_LT80,
			   he_cap->num_sounding_lt_80);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_NUM_SOUND_GT80,
			   he_cap->num_sounding_gt_80);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_SU_FEED_TONE16,
			   he_cap->su_feedback_tone16);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MU_FEED_TONE16,
			   he_cap->mu_feedback_tone16);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_CODEBOOK_SU,
			   he_cap->codebook_su);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_CODEBOOK_MU,
			   he_cap->codebook_mu);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_BFRM_FEED,
			   he_cap->beamforming_feedback);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_ER_SU_PPDU,
			   he_cap->he_er_su_ppdu);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_DL_PART_BW,
			   he_cap->dl_mu_mimo_part_bw);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_PPET_PRESENT,
			   he_cap->ppet_present);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_SRP, he_cap->srp);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_POWER_BOOST,
			   he_cap->power_boost);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_4x_LTF_GI, he_cap->he_ltf_gi_4x);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_NSS, he_cap->nss_supported);
	hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_MCS, he_cap->mcs_supported);

	/* PPET can not be configured by user - Set values from FW */
	status = sme_cfg_set_str(hdd_ctx->hHal, WNI_CFG_HE_PPET,
				 (void *)&he_cap->ppe_threshold, ppet_size);
	if (status == QDF_STATUS_E_FAILURE)
		hdd_alert("could not set HE PPET");
}

void wlan_hdd_check_11ax_support(beacon_data_t *beacon, tsap_Config_t *config)
{
	uint8_t *ie;

	ie = wlan_hdd_get_vendor_oui_ie_ptr(HE_CAP_OUI_TYPE, HE_CAP_OUI_SIZE,
					    beacon->tail, beacon->tail_len);
	if (ie)
		config->SapHw_mode = eCSR_DOT11_MODE_11ax;
}

void hdd_he_print_ini_config(hdd_context_t *hdd_ctx)
{
	hdd_info("Name = [%s] Value = [%d]", CFG_ENABLE_UL_MIMO_NAME,
		hdd_ctx->config->enable_ul_mimo);
	hdd_info("Name = [%s] Value = [%d]", CFG_ENABLE_UL_OFDMA_NAME,
		hdd_ctx->config->enable_ul_ofdma);
	hdd_info("Name = [%s] Value = [%d]", CFG_HE_STA_OBSSPD_NAME,
		hdd_ctx->config->he_sta_obsspd);
}

int hdd_update_he_cap_in_cfg(hdd_context_t *hdd_ctx)
{
	uint32_t val, val1 = 0;
	QDF_STATUS status;
	int ret;
	struct hdd_config *config = hdd_ctx->config;

	ret = hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_STA_OBSSPD,
				 config->he_sta_obsspd);
	if (ret)
		return ret;

	status = sme_cfg_get_int(hdd_ctx->hHal, WNI_CFG_HE_UL_MUMIMO, &val);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("could not get WNI_CFG_HE_UL_MUMIMO");
		return qdf_status_to_os_return(status);
	}

	/* In val,
	 * Bit 1 - corresponds to UL MIMO
	 * Bit 2 - corresponds to UL OFDMA
	 */
	if (val & 0x1)
		val1 = config->enable_ul_mimo & 0x1;

	if ((val >> 1) & 0x1)
		val1 |= ((config->enable_ul_ofdma & 0x1) << 1);

	ret = hdd_he_set_wni_cfg(hdd_ctx, WNI_CFG_HE_UL_MUMIMO, val1);

	return ret;
}

void hdd_he_set_sme_config(tSmeConfigParams *sme_config,
			   struct hdd_config *config)
{
	sme_config->csrConfig.enable_ul_ofdma = config->enable_ul_ofdma;
	sme_config->csrConfig.enable_ul_mimo = config->enable_ul_mimo;
}

/*
 * __wlan_hdd_cfg80211_get_he_cap() - get HE Capabilities
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 if success, non-zero for failure
 */
static int
__wlan_hdd_cfg80211_get_he_cap(struct wiphy *wiphy,
			       struct wireless_dev *wdev,
			       const void *data,
			       int data_len)
{
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	int ret;
	QDF_STATUS status;
	struct sk_buff *reply_skb;
	uint32_t nl_buf_len;
	struct he_capability he_cap;
	uint8_t he_supported = 0;

	ENTER();
	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	nl_buf_len = NLMSG_HDRLEN;
	if (sme_is_feature_supported_by_fw(DOT11AX)) {
		he_supported = 1;

		status = wma_get_he_capabilities(&he_cap);
		if (QDF_STATUS_SUCCESS != status)
			return -EINVAL;
	} else {
		hdd_info("11AX: HE not supported, send only QCA_WLAN_VENDOR_ATTR_HE_SUPPORTED");
	}

	if (he_supported) {
		nl_buf_len += NLA_HDRLEN + sizeof(he_supported) +
			      NLA_HDRLEN + sizeof(he_cap.phy_cap) +
			      NLA_HDRLEN + sizeof(he_cap.mac_cap) +
			      NLA_HDRLEN + sizeof(he_cap.mcs) +
			      NLA_HDRLEN + sizeof(he_cap.ppet.numss_m1) +
			      NLA_HDRLEN + sizeof(he_cap.ppet.ru_bit_mask) +
			      NLA_HDRLEN +
				sizeof(he_cap.ppet.ppet16_ppet8_ru3_ru0);
	} else {
		nl_buf_len += NLA_HDRLEN + sizeof(he_supported);
	}

	hdd_info("11AX: he_supported: %d", he_supported);

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, nl_buf_len);

	if (!reply_skb) {
		hdd_err("Allocate reply_skb failed");
		return -EINVAL;
	}

	if (nla_put_u8(reply_skb,
		       QCA_WLAN_VENDOR_ATTR_HE_SUPPORTED, he_supported))
		goto nla_put_failure;

	/* No need to populate other attributes if HE is not supported */
	if (0 == he_supported)
		goto end;

	if (nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_MAC_CAPAB, he_cap.mac_cap) ||
	    nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_HE_MCS, he_cap.mcs) ||
	    nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_NUM_SS, he_cap.ppet.numss_m1) ||
	    nla_put_u32(reply_skb,
			QCA_WLAN_VENDOR_ATTR_RU_IDX_MASK,
			he_cap.ppet.ru_bit_mask) ||
	    nla_put(reply_skb,
		    QCA_WLAN_VENDOR_ATTR_PHY_CAPAB,
		    sizeof(u32) * HE_MAX_PHY_CAP_SIZE, he_cap.phy_cap) ||
	    nla_put(reply_skb, QCA_WLAN_VENDOR_ATTR_PPE_THRESHOLD,
		    sizeof(u32) * PSOC_HOST_MAX_NUM_SS,
		    he_cap.ppet.ppet16_ppet8_ru3_ru0))
		goto nla_put_failure;
end:
	ret = cfg80211_vendor_cmd_reply(reply_skb);
	EXIT();
	return ret;

nla_put_failure:
	hdd_err("nla put fail");
	kfree_skb(reply_skb);
	return -EINVAL;
}

int wlan_hdd_cfg80211_get_he_cap(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data,
				 int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_he_cap(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}
