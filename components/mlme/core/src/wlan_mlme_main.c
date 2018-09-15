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
/**
 * DOC: define internal APIs related to the mlme component
 */

#include "wlan_mlme_main.h"
#include "cfg_ucfg_api.h"
#include "wmi_unified.h"

struct wlan_mlme_psoc_obj *mlme_get_psoc_obj(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = (struct wlan_mlme_psoc_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
						      WLAN_UMAC_COMP_MLME);

	return mlme_obj;
}

QDF_STATUS
mlme_psoc_object_created_notification(struct wlan_objmgr_psoc *psoc,
				      void *arg)
{
	QDF_STATUS status;
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = qdf_mem_malloc(sizeof(struct wlan_mlme_psoc_obj));
	if (!mlme_obj) {
		mlme_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       mlme_obj,
						       QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Failed to attach psoc_ctx with psoc");
		qdf_mem_free(mlme_obj);
	}

	return status;
}

QDF_STATUS
mlme_psoc_object_destroyed_notification(struct wlan_objmgr_psoc *psoc,
					void *arg)
{
	struct wlan_mlme_psoc_obj *mlme_obj = NULL;
	QDF_STATUS status;

	mlme_obj = mlme_get_psoc_obj(psoc);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       mlme_obj);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Failed to detach psoc_ctx from psoc");
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	qdf_mem_free(mlme_obj);

out:
	return status;
}

static void mlme_init_chainmask_cfg(struct wlan_objmgr_psoc *psoc,
				    struct wlan_mlme_chainmask *chainmask_info)
{
	chainmask_info->txchainmask1x1 =
		cfg_get(psoc, CFG_VHT_ENABLE_1x1_TX_CHAINMASK);

	chainmask_info->rxchainmask1x1 =
		cfg_get(psoc, CFG_VHT_ENABLE_1x1_RX_CHAINMASK);

	chainmask_info->tx_chain_mask_cck =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_CCK);

	chainmask_info->tx_chain_mask_1ss =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_1SS);

	chainmask_info->num_11b_tx_chains =
		cfg_get(psoc, CFG_11B_NUM_TX_CHAIN);

	chainmask_info->num_11ag_tx_chains =
		cfg_get(psoc, CFG_11AG_NUM_TX_CHAIN);

	chainmask_info->tx_chain_mask_2g =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_2G);

	chainmask_info->rx_chain_mask_2g =
		cfg_get(psoc, CFG_RX_CHAIN_MASK_2G);

	chainmask_info->tx_chain_mask_5g =
		cfg_get(psoc, CFG_TX_CHAIN_MASK_5G);

	chainmask_info->rx_chain_mask_5g =
		cfg_get(psoc, CFG_RX_CHAIN_MASK_5G);
}

#ifdef WLAN_FEATURE_11W
static void mlme_init_pmf_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_generic *gen)
{
	gen->pmf_sa_query_max_retries =
		cfg_get(psoc, CFG_PMF_SA_QUERY_MAX_RETRIES);
	gen->pmf_sa_query_retry_interval =
		cfg_get(psoc, CFG_PMF_SA_QUERY_RETRY_INTERVAL);
}
#else
static void mlme_init_pmf_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_generic *gen)
{
	gen->pmf_sa_query_max_retries =
		cfg_default(CFG_PMF_SA_QUERY_MAX_RETRIES);
	gen->pmf_sa_query_retry_interval =
		cfg_default(CFG_PMF_SA_QUERY_RETRY_INTERVAL);
}
#endif /*WLAN_FEATURE_11W*/

static void mlme_init_generic_cfg(struct wlan_objmgr_psoc *psoc,
				  struct wlan_mlme_generic *gen)
{
	gen->rtt3_enabled = cfg_default(CFG_RTT3_ENABLE);
	mlme_init_pmf_cfg(psoc, gen);
}

static void mlme_init_edca_ani_cfg(struct wlan_mlme_edca_params *edca_params)
{
	/* initialize the max allowed array length for read/write */
	edca_params->ani_acbe_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->ani_acbk_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->ani_acvi_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->ani_acvo_l.max_len = CFG_EDCA_DATA_LEN;

	edca_params->ani_acbe_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->ani_acbk_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->ani_acvi_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->ani_acvo_b.max_len = CFG_EDCA_DATA_LEN;

	/* parse the ETSI edca parameters from cfg string for BK,BE,VI,VO ac */
	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACBK_LOCAL),
			      edca_params->ani_acbk_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acbk_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACBE_LOCAL),
			      edca_params->ani_acbe_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acbe_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACVI_LOCAL),
			      edca_params->ani_acvi_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acvi_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACVO_LOCAL),
			      edca_params->ani_acvo_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acvo_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACBK),
			      edca_params->ani_acbk_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acbk_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACBE),
			      edca_params->ani_acbe_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acbe_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACVI),
			      edca_params->ani_acvi_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acvi_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ANI_ACVO),
			      edca_params->ani_acvo_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->ani_acvo_b.len);
}

static void mlme_init_edca_wme_cfg(struct wlan_mlme_edca_params *edca_params)
{
	/* initialize the max allowed array length for read/write */
	edca_params->wme_acbk_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->wme_acbe_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->wme_acvi_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->wme_acvo_l.max_len = CFG_EDCA_DATA_LEN;

	edca_params->wme_acbk_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->wme_acbe_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->wme_acvi_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->wme_acvo_b.max_len = CFG_EDCA_DATA_LEN;

	/* parse the WME edca parameters from cfg string for BK,BE,VI,VO ac */
	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACBK_LOCAL),
			      edca_params->wme_acbk_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acbk_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACBE_LOCAL),
			      edca_params->wme_acbe_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acbe_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACVI_LOCAL),
			      edca_params->wme_acvi_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acvi_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACVO_LOCAL),
			      edca_params->wme_acvo_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acvo_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACBK),
			      edca_params->wme_acbk_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acbk_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACBE),
			      edca_params->wme_acbe_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acbe_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACVI),
			      edca_params->wme_acvi_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acvi_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_WME_ACVO),
			      edca_params->wme_acvo_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->wme_acvo_b.len);
}

static void mlme_init_edca_etsi_cfg(struct wlan_mlme_edca_params *edca_params)
{
	/* initialize the max allowed array length for read/write */
	edca_params->etsi_acbe_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->etsi_acbk_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->etsi_acvi_l.max_len = CFG_EDCA_DATA_LEN;
	edca_params->etsi_acvo_l.max_len = CFG_EDCA_DATA_LEN;

	edca_params->etsi_acbe_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->etsi_acbk_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->etsi_acvi_b.max_len = CFG_EDCA_DATA_LEN;
	edca_params->etsi_acvo_b.max_len = CFG_EDCA_DATA_LEN;

	/* parse the ETSI edca parameters from cfg string for BK,BE,VI,VO ac */
	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACBK_LOCAL),
			      edca_params->etsi_acbk_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acbk_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACBE_LOCAL),
			      edca_params->etsi_acbe_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acbe_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACVI_LOCAL),
			      edca_params->etsi_acvi_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acvi_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACVO_LOCAL),
			      edca_params->etsi_acvo_l.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acvo_l.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACBK),
			      edca_params->etsi_acbk_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acbk_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACBE),
			      edca_params->etsi_acbe_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acbe_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACVI),
			      edca_params->etsi_acvi_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acvi_b.len);

	qdf_uint8_array_parse(cfg_default(CFG_EDCA_ETSI_ACVO),
			      edca_params->etsi_acvo_b.data,
			      CFG_EDCA_DATA_LEN,
			      &edca_params->etsi_acvo_b.len);
}

static void mlme_init_edca_params(struct wlan_mlme_edca_params *edca_params)
{
	mlme_init_edca_ani_cfg(edca_params);
	mlme_init_edca_wme_cfg(edca_params);
	mlme_init_edca_etsi_cfg(edca_params);
}

static void mlme_init_ht_cap_in_cfg(struct wlan_objmgr_psoc *psoc,
				    struct wlan_mlme_ht_caps *ht_caps)
{
	union {
		uint16_t val_16;
		struct mlme_ht_capabilities_info default_ht_cap_info;
	} u;

	u.val_16 = (uint16_t)cfg_default(CFG_HT_CAP_INFO);

	u.default_ht_cap_info.adv_coding_cap =
				cfg_get(psoc, CFG_RX_LDPC_ENABLE);
	u.default_ht_cap_info.rx_stbc = cfg_get(psoc, CFG_RX_STBC_ENABLE);
	u.default_ht_cap_info.tx_stbc = cfg_get(psoc, CFG_TX_STBC_ENABLE);
	u.default_ht_cap_info.short_gi_20_mhz =
				cfg_get(psoc, CFG_SHORT_GI_20MHZ);
	u.default_ht_cap_info.short_gi_40_mhz =
				cfg_get(psoc, CFG_SHORT_GI_40MHZ);

	ht_caps->ht_cap_info = u.default_ht_cap_info;
}

static void mlme_init_qos_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_qos *qos_aggr_params)
{
	qos_aggr_params->tx_aggregation_size =
				cfg_get(psoc, CFG_TX_AGGREGATION_SIZE);
	qos_aggr_params->tx_aggregation_size_be =
				cfg_get(psoc, CFG_TX_AGGREGATION_SIZEBE);
	qos_aggr_params->tx_aggregation_size_bk =
				cfg_get(psoc, CFG_TX_AGGREGATION_SIZEBK);
	qos_aggr_params->tx_aggregation_size_vi =
				cfg_get(psoc, CFG_TX_AGGREGATION_SIZEVI);
	qos_aggr_params->tx_aggregation_size_vo =
				cfg_get(psoc, CFG_TX_AGGREGATION_SIZEVO);
	qos_aggr_params->rx_aggregation_size =
				cfg_get(psoc, CFG_RX_AGGREGATION_SIZE);
	qos_aggr_params->tx_aggr_sw_retry_threshold_be =
				cfg_get(psoc, CFG_TX_AGGR_SW_RETRY_BE);
	qos_aggr_params->tx_aggr_sw_retry_threshold_bk =
				cfg_get(psoc, CFG_TX_AGGR_SW_RETRY_BK);
	qos_aggr_params->tx_aggr_sw_retry_threshold_vi =
				cfg_get(psoc, CFG_TX_AGGR_SW_RETRY_VI);
	qos_aggr_params->tx_aggr_sw_retry_threshold_vo =
				cfg_get(psoc, CFG_TX_AGGR_SW_RETRY_VO);
	qos_aggr_params->sap_max_inactivity_override =
				cfg_get(psoc, CFG_SAP_MAX_INACTIVITY_OVERRIDE);
}

static void mlme_init_mbo_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_mbo *mbo_params)
{
	mbo_params->mbo_candidate_rssi_thres =
			cfg_get(psoc, CFG_MBO_CANDIDATE_RSSI_THRESHOLD);
	mbo_params->mbo_current_rssi_thres =
			cfg_get(psoc, CFG_MBO_CURRENT_RSSI_THRESHOLD);
	mbo_params->mbo_current_rssi_mcc_thres =
			cfg_get(psoc, CFG_MBO_CUR_RSSI_MCC_THRESHOLD);
	mbo_params->mbo_candidate_rssi_btc_thres =
			cfg_get(psoc, CFG_MBO_CAND_RSSI_BTC_THRESHOLD);
}

static void mlme_init_rates_in_cfg(struct wlan_objmgr_psoc *psoc,
				   struct wlan_mlme_rates *rates)
{
	rates->cfp_period = cfg_default(CFG_CFP_PERIOD);
	rates->cfp_max_duration = cfg_default(CFG_CFP_MAX_DURATION);
	rates->max_htmcs_txdata = cfg_get(psoc, CFG_INI_MAX_HT_MCS_FOR_TX_DATA);
	rates->disable_abg_rate_txdata = cfg_get(psoc,
					CFG_INI_DISABLE_ABG_RATE_FOR_TX_DATA);
	rates->sap_max_mcs_txdata = cfg_get(psoc,
					CFG_INI_SAP_MAX_MCS_FOR_TX_DATA);
	rates->disable_high_ht_mcs_2x2 = cfg_get(psoc,
					 CFG_INI_DISABLE_HIGH_HT_RX_MCS_2x2);
}

static void mlme_init_sap_protection_cfg(struct wlan_objmgr_psoc *psoc,
					 struct wlan_mlme_sap_protection
					 *sap_protection_params)
{
	sap_protection_params->protection_enabled =
				cfg_default(CFG_PROTECTION_ENABLED);
	sap_protection_params->protection_force_policy =
				cfg_default(CFG_FORCE_POLICY_PROTECTION);
	sap_protection_params->ignore_peer_ht_opmode =
				cfg_get(psoc, CFG_IGNORE_PEER_HT_MODE);
}

static void mlme_init_he_cap_in_cfg(struct wlan_objmgr_psoc *psoc,
				    struct wlan_mlme_he_caps *he_caps)
{
	he_caps->he_control = cfg_default(CFG_HE_CONTROL);
	he_caps->he_twt_requestor = cfg_default(CFG_HE_TWT_REQUESTOR);
	he_caps->he_twt_responder = cfg_default(CFG_HE_TWT_RESPONDER);
	he_caps->he_twt_fragmentation = cfg_default(CFG_HE_TWT_FRAGMENTATION);
	he_caps->he_max_frag_msdu = cfg_default(CFG_HE_MAX_FRAG_MSDU);
	he_caps->he_min_frag_size = cfg_default(CFG_HE_MIN_FRAG_SIZE);
	he_caps->he_trig_pad = cfg_default(CFG_HE_TRIG_PAD);
	he_caps->he_mtid_aggr_rx = cfg_default(CFG_HE_MTID_AGGR_RX);
	he_caps->he_link_adaptation = cfg_default(CFG_HE_LINK_ADAPTATION);
	he_caps->he_all_ack = cfg_default(CFG_HE_ALL_ACK);
	he_caps->he_trigd_rsp_scheduling =
			cfg_default(CFG_HE_TRIGD_RSP_SCHEDULING);
	he_caps->he_buffer_status_rpt = cfg_default(CFG_HE_BUFFER_STATUS_RPT);
	he_caps->he_bcast_twt = cfg_default(CFG_HE_BCAST_TWT);
	he_caps->he_ba_32bit = cfg_default(CFG_HE_BA_32BIT);
	he_caps->he_mu_cascading = cfg_default(CFG_HE_MU_CASCADING);
	he_caps->he_multi_tid = cfg_default(CFG_HE_MULTI_TID);
	he_caps->he_dl_mu_ba = cfg_default(CFG_HE_DL_MU_BA);
	he_caps->he_omi = cfg_default(CFG_HE_OMI);
	he_caps->he_ofdma_ra = cfg_default(CFG_HE_OFDMA_RA);
	he_caps->he_max_ampdu_len = cfg_default(CFG_HE_MAX_AMPDU_LEN);
	he_caps->he_amsdu_frag = cfg_default(CFG_HE_AMSDU_FRAG);
	he_caps->he_flex_twt_sched = cfg_default(CFG_HE_FLEX_TWT_SCHED);
	he_caps->he_rx_ctrl = cfg_default(CFG_HE_RX_CTRL);
	he_caps->he_bsrp_ampdu_aggr = cfg_default(CFG_HE_BSRP_AMPDU_AGGR);
	he_caps->he_qtp = cfg_default(CFG_HE_QTP);
	he_caps->he_a_bqr = cfg_default(CFG_HE_A_BQR);
	he_caps->he_sr_responder = cfg_default(CFG_HE_SR_RESPONDER);
	he_caps->he_ndp_feedback_supp = cfg_default(CFG_HE_NDP_FEEDBACK_SUPP);
	he_caps->he_ops_supp = cfg_default(CFG_HE_OPS_SUPP);
	he_caps->he_amsdu_in_ampdu = cfg_default(CFG_HE_AMSDU_IN_AMPDU);
	he_caps->he_chan_width = cfg_default(CFG_HE_CHAN_WIDTH);
	he_caps->he_mtid_aggr_tx = cfg_default(CFG_HE_MTID_AGGR_TX);
	he_caps->he_sub_ch_sel_tx = cfg_default(CFG_HE_SUB_CH_SEL_TX);
	he_caps->he_ul_2x996_ru = cfg_default(CFG_HE_UL_2X996_RU);
	he_caps->he_om_ctrl_ul_mu_dis_rx =
				cfg_default(CFG_HE_OM_CTRL_UL_MU_DIS_RX);
	he_caps->he_rx_pream_punc = cfg_default(CFG_HE_RX_PREAM_PUNC);
	he_caps->he_class_of_device = cfg_default(CFG_HE_CLASS_OF_DEVICE);
	he_caps->he_ldpc = cfg_default(CFG_HE_LDPC);
	he_caps->he_ltf_ppdu = cfg_default(CFG_HE_LTF_PPDU);
	he_caps->he_midamble_rx_nsts = cfg_default(CFG_HE_MIDAMBLE_RX_MAX_NSTS);
	he_caps->he_ltf_ndp = cfg_default(CFG_HE_LTF_NDP);
	he_caps->he_tx_stbc_lt80 = cfg_default(CFG_HE_TX_STBC_LT80);
	he_caps->he_rx_stbc_lt80 = cfg_default(CFG_HE_RX_STBC_LT80);
	he_caps->he_doppler = cfg_default(CFG_HE_DOPPLER);
	he_caps->he_ul_mumimo = cfg_default(CFG_HE_UL_MUMIMO);
	he_caps->he_dcm_tx = cfg_default(CFG_HE_DCM_TX);
	he_caps->he_dcm_rx = cfg_default(CFG_HE_DCM_RX);
	he_caps->he_mu_ppdu = cfg_default(CFG_HE_MU_PPDU);
	he_caps->he_su_beamformer = cfg_default(CFG_HE_SU_BEAMFORMER);
	he_caps->he_su_beamformee = cfg_default(CFG_HE_SU_BEAMFORMEE);
	he_caps->he_mu_beamformer = cfg_default(CFG_HE_MU_BEAMFORMER);
	he_caps->he_bfee_sts_lt80 = cfg_default(CFG_HE_BFEE_STS_LT80);
	he_caps->he_bfee_sts_gt80 = cfg_default(CFG_HE_BFEE_STS_GT80);
	he_caps->he_num_sound_lt80 = cfg_default(CFG_HE_NUM_SOUND_LT80);
	he_caps->he_num_sound_gt80 = cfg_default(CFG_HE_NUM_SOUND_GT80);
	he_caps->he_su_feed_tone16 = cfg_default(CFG_HE_SU_FEED_TONE16);
	he_caps->he_mu_feed_tone16 = cfg_default(CFG_HE_MU_FEED_TONE16);
	he_caps->he_codebook_su = cfg_default(CFG_HE_CODEBOOK_SU);
	he_caps->he_codebook_mu = cfg_default(CFG_HE_CODEBOOK_MU);
	he_caps->he_bfrm_feed = cfg_default(CFG_HE_BFRM_FEED);
	he_caps->he_er_su_ppdu = cfg_default(CFG_HE_ER_SU_PPDU);
	he_caps->he_dl_part_bw = cfg_default(CFG_HE_DL_PART_BW);
	he_caps->he_ppet_present = cfg_default(CFG_HE_PPET_PRESENT);
	he_caps->he_srp = cfg_default(CFG_HE_SRP);
	he_caps->he_power_boost = cfg_default(CFG_HE_POWER_BOOST);
	he_caps->he_4x_ltf_gi = cfg_default(CFG_HE_4x_LTF_GI);
	he_caps->he_max_nc = cfg_default(CFG_HE_MAX_NC);
	he_caps->he_tx_stbc_gt80 = cfg_default(CFG_HE_TX_STBC_GT80);
	he_caps->he_rx_stbc_gt80 = cfg_default(CFG_HE_RX_STBC_GT80);
	he_caps->he_er_4x_ltf_gi = cfg_default(CFG_HE_ER_4x_LTF_GI);
	he_caps->he_ppdu_20_in_40mhz_2g =
			cfg_default(CFG_HE_PPDU_20_IN_40MHZ_2G);
	he_caps->he_ppdu_20_in_160_80p80mhz =
			cfg_default(CFG_HE_PPDU_20_IN_160_80P80MHZ);
	he_caps->he_ppdu_80_in_160_80p80mhz =
			cfg_default(CFG_HE_PPDU_80_IN_160_80P80MHZ);
		he_caps->he_er_1x_he_ltf_gi =
			cfg_default(CFG_HE_ER_1X_HE_LTF_GI);
	he_caps->he_midamble_rx_1x_he_ltf =
			cfg_default(CFG_HE_MIDAMBLE_RX_1X_HE_LTF);
	he_caps->he_dcm_max_bw = cfg_default(CFG_HE_DCM_MAX_BW);
	he_caps->he_longer_16_sigb_ofdm_sym =
			cfg_default(CFG_HE_LONGER_16_SIGB_OFDM_SYM);
	he_caps->he_non_trig_cqi_feedback =
			cfg_default(CFG_HE_NON_TRIG_CQI_FEEDBACK);
	he_caps->he_tx_1024_qam_lt_242_ru =
			cfg_default(CFG_HE_TX_1024_QAM_LT_242_RU);
	he_caps->he_rx_1024_qam_lt_242_ru =
			cfg_default(CFG_HE_RX_1024_QAM_LT_242_RU);
	he_caps->he_rx_full_bw_mu_cmpr_sigb =
			cfg_default(CFG_HE_RX_FULL_BW_MU_CMPR_SIGB);
	he_caps->he_rx_full_bw_mu_non_cmpr_sigb =
			cfg_default(CFG_HE_RX_FULL_BW_MU_NON_CMPR_SIGB);
	he_caps->he_rx_mcs_map_lt_80 = cfg_default(CFG_HE_RX_MCS_MAP_LT_80);
	he_caps->he_tx_mcs_map_lt_80 = cfg_default(CFG_HE_TX_MCS_MAP_LT_80);
	he_caps->he_rx_mcs_map_160 = cfg_default(CFG_HE_RX_MCS_MAP_160);
	he_caps->he_tx_mcs_map_160 = cfg_default(CFG_HE_TX_MCS_MAP_160);
	he_caps->he_rx_mcs_map_80_80 = cfg_default(CFG_HE_RX_MCS_MAP_80_80);
	he_caps->he_tx_mcs_map_80_80 = cfg_default(CFG_HE_TX_MCS_MAP_80_80);
	he_caps->he_ops_basic_mcs_nss = cfg_default(CFG_HE_OPS_BASIC_MCS_NSS);
	he_caps->he_twt_dynamic_fragmentation =
			cfg_get(psoc, CFG_HE_DYNAMIC_FRAGMENTATION);
	he_caps->enable_ul_mimo =
			cfg_get(psoc, CFG_ENABLE_UL_MIMO);
	he_caps->enable_ul_ofdm =
			cfg_get(psoc, CFG_ENABLE_UL_OFDMA);
	he_caps->he_sta_obsspd =
			cfg_get(psoc, CFG_HE_STA_OBSSPD);
	qdf_mem_zero(he_caps->he_ppet_2g, MLME_HE_PPET_LEN);
	qdf_mem_zero(he_caps->he_ppet_5g, MLME_HE_PPET_LEN);
}

static void mlme_init_sap_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_cfg_sap *sap_cfg)
{
	sap_cfg->beacon_interval = cfg_get(psoc, CFG_BEACON_INTERVAL);
	sap_cfg->dtim_interval = cfg_default(CFG_DTIM_PERIOD);
	sap_cfg->listen_interval = cfg_default(CFG_LISTEN_INTERVAL);
	sap_cfg->sap_11g_policy = cfg_default(CFG_11G_ONLY_POLICY);
	sap_cfg->assoc_sta_limit = cfg_default(CFG_ASSOC_STA_LIMIT);
	sap_cfg->enable_lte_coex = cfg_get(psoc, CFG_ENABLE_LTE_COEX);
	sap_cfg->rmc_action_period_freq =
		cfg_default(CFG_RMC_ACTION_PERIOD_FREQUENCY);
	sap_cfg->rate_tx_mgmt = cfg_get(psoc, CFG_RATE_FOR_TX_MGMT);
	sap_cfg->rate_tx_mgmt_2g = cfg_get(psoc, CFG_RATE_FOR_TX_MGMT_2G);
	sap_cfg->rate_tx_mgmt_5g = cfg_get(psoc, CFG_RATE_FOR_TX_MGMT_5G);
	sap_cfg->tele_bcn_wakeup_en = cfg_get(psoc, CFG_TELE_BCN_WAKEUP_EN);
	sap_cfg->tele_bcn_max_li = cfg_get(psoc, CFG_TELE_BCN_MAX_LI);
	sap_cfg->sap_get_peer_info = cfg_get(psoc, CFG_SAP_GET_PEER_INFO);
	sap_cfg->sap_allow_all_chan_param_name =
			cfg_get(psoc, CFG_SAP_ALLOW_ALL_CHANNEL_PARAM);
	sap_cfg->sap_max_no_peers = cfg_get(psoc, CFG_SAP_MAX_NO_PEERS);
	sap_cfg->sap_max_offload_peers =
			cfg_get(psoc, CFG_SAP_MAX_OFFLOAD_PEERS);
	sap_cfg->sap_max_offload_reorder_buffs =
			cfg_get(psoc, CFG_SAP_MAX_OFFLOAD_REORDER_BUFFS);
	sap_cfg->sap_ch_switch_beacon_cnt =
			cfg_get(psoc, CFG_SAP_CH_SWITCH_BEACON_CNT);
	sap_cfg->sap_ch_switch_mode = cfg_get(psoc, CFG_SAP_CH_SWITCH_MODE);
	sap_cfg->sap_internal_restart =
			cfg_get(psoc, CFG_SAP_INTERNAL_RESTART);
	sap_cfg->chan_switch_hostapd_rate_enabled_name =
		cfg_get(psoc, CFG_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_NAME);
	sap_cfg->reduced_beacon_interval =
		cfg_get(psoc, CFG_REDUCED_BEACON_INTERVAL);

}

static void mlme_init_obss_ht40_cfg(struct wlan_objmgr_psoc *psoc,
				    struct wlan_mlme_obss_ht40 *obss_ht40)
{
	obss_ht40->active_dwelltime =
		cfg_get(psoc, CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME);
	obss_ht40->passive_dwelltime =
		cfg_get(psoc, CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME);
	obss_ht40->width_trigger_interval =
		cfg_get(psoc, CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL);
	obss_ht40->passive_per_channel = (uint32_t)
		cfg_default(CFG_OBSS_HT40_SCAN_PASSIVE_TOTAL_PER_CHANNEL);
	obss_ht40->active_per_channel = (uint32_t)
		cfg_default(CFG_OBSS_HT40_SCAN_ACTIVE_TOTAL_PER_CHANNEL);
	obss_ht40->width_trans_delay = (uint32_t)
		cfg_default(CFG_OBSS_HT40_WIDTH_CH_TRANSITION_DELAY);
	obss_ht40->scan_activity_threshold = (uint32_t)
		cfg_default(CFG_OBSS_HT40_SCAN_ACTIVITY_THRESHOLD);
}

static void mlme_init_sta_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_sta_cfg *sta)
{
	sta->sta_keep_alive_period =
		cfg_get(psoc, CFG_INFRA_STA_KEEP_ALIVE_PERIOD);
	sta->tgt_gtx_usr_cfg =
		cfg_get(psoc, CFG_TGT_GTX_USR_CFG);
	sta->pmkid_modes =
		cfg_get(psoc, CFG_PMKID_MODES);
	sta->ignore_peer_erp_info =
		cfg_get(psoc, CFG_IGNORE_PEER_ERP_INFO);
	sta->sta_prefer_80mhz_over_160mhz =
		cfg_get(psoc, CFG_STA_PREFER_80MHZ_OVER_160MHZ);
	sta->enable_5g_ebt =
		cfg_get(psoc, CFG_PPS_ENABLE_5G_EBT);
	sta->deauth_before_connection =
		cfg_get(psoc, CFG_ENABLE_DEAUTH_BEFORE_CONNECTION);
	sta->dot11p_mode =
		cfg_get(psoc, CFG_DOT11P_MODE);
	sta->enable_go_cts2self_for_sta =
		cfg_get(psoc, CFG_ENABLE_GO_CTS2SELF_FOR_STA);
	sta->qcn_ie_support =
		cfg_get(psoc, CFG_QCN_IE_SUPPORT);
	sta->fils_max_chan_guard_time =
		cfg_get(psoc, CFG_FILS_MAX_CHAN_GUARD_TIME);
	sta->force_rsne_override =
		cfg_get(psoc, CFG_FORCE_RSNE_OVERRIDE);
	sta->single_tid =
		cfg_get(psoc, CFG_SINGLE_TID_RC);
	sta->wait_cnf_timeout =
		(uint32_t)cfg_default(CFG_WT_CNF_TIMEOUT);
	sta->current_rssi =
		(uint32_t)cfg_default(CFG_CURRENT_RSSI);
}

static void mlme_init_lfr_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_lfr_cfg *lfr)
{
	lfr->mawc_roam_enabled =
		cfg_get(psoc, CFG_LFR_MAWC_ROAM_ENABLED);
	lfr->enable_fast_roam_in_concurrency =
		cfg_get(psoc, CFG_LFR_ENABLE_FAST_ROAM_IN_CONCURRENCY);
	lfr->lfr3_roaming_offload =
		cfg_get(psoc, CFG_LFR3_ROAMING_OFFLOAD);
	lfr->early_stop_scan_enable =
		cfg_get(psoc, CFG_LFR_EARLY_STOP_SCAN_ENABLE);
	lfr->lfr3_enable_subnet_detection =
		cfg_get(psoc, CFG_LFR3_ENABLE_SUBNET_DETECTION);
	lfr->enable_5g_band_pref =
		cfg_get(psoc, CFG_LFR_ENABLE_5G_BAND_PREF);
	lfr->mawc_roam_traffic_threshold =
		cfg_get(psoc, CFG_LFR_MAWC_ROAM_TRAFFIC_THRESHOLD);
	lfr->mawc_roam_ap_rssi_threshold =
		cfg_get(psoc, CFG_LFR_MAWC_ROAM_AP_RSSI_THRESHOLD);
	lfr->mawc_roam_rssi_high_adjust =
		cfg_get(psoc, CFG_LFR_MAWC_ROAM_RSSI_HIGH_ADJUST);
	lfr->mawc_roam_rssi_low_adjust =
		cfg_get(psoc, CFG_LFR_MAWC_ROAM_RSSI_LOW_ADJUST);
	lfr->roam_rssi_abs_threshold =
		cfg_get(psoc, CFG_LFR_ROAM_RSSI_ABS_THRESHOLD);
	lfr->rssi_threshold_offset_5g =
		cfg_get(psoc, CFG_LFR_5G_RSSI_THRESHOLD_OFFSET);
	lfr->early_stop_scan_min_threshold =
		cfg_get(psoc, CFG_LFR_EARLY_STOP_SCAN_MIN_THRESHOLD);
	lfr->early_stop_scan_max_threshold =
		cfg_get(psoc, CFG_LFR_EARLY_STOP_SCAN_MAX_THRESHOLD);
	lfr->first_scan_bucket_threshold =
		cfg_get(psoc, CFG_LFR_FIRST_SCAN_BUCKET_THRESHOLD);
	lfr->roam_dense_traffic_threshold =
		cfg_get(psoc, CFG_LFR_ROAM_DENSE_TRAFFIC_THRESHOLD);
	lfr->roam_dense_rssi_thre_offset =
		cfg_get(psoc, CFG_LFR_ROAM_DENSE_RSSI_THRE_OFFSET);
	lfr->roam_dense_min_aps =
		cfg_get(psoc, CFG_LFR_ROAM_DENSE_MIN_APS);
	lfr->roam_bg_scan_bad_rssi_threshold =
		cfg_get(psoc, CFG_LFR_ROAM_BG_SCAN_BAD_RSSI_THRESHOLD);
	lfr->roam_bg_scan_client_bitmap =
		cfg_get(psoc, CFG_LFR_ROAM_BG_SCAN_CLIENT_BITMAP);
	lfr->roam_bg_scan_bad_rssi_offset_2g =
		cfg_get(psoc, CFG_LFR_ROAM_BG_SCAN_BAD_RSSI_OFFSET_2G);
	lfr->adaptive_roamscan_dwell_mode =
		cfg_get(psoc, CFG_LFR_ADAPTIVE_ROAMSCAN_DWELL_MODE);
	lfr->per_roam_enable =
		cfg_get(psoc, CFG_LFR_PER_ROAM_ENABLE);
	lfr->per_roam_config_high_rate_th =
		cfg_get(psoc, CFG_LFR_PER_ROAM_CONFIG_HIGH_RATE_TH);
	lfr->per_roam_config_low_rate_th =
		cfg_get(psoc, CFG_LFR_PER_ROAM_CONFIG_LOW_RATE_TH);
	lfr->per_roam_config_rate_th_percent =
		cfg_get(psoc, CFG_LFR_PER_ROAM_CONFIG_RATE_TH_PERCENT);
	lfr->per_roam_rest_time =
		cfg_get(psoc, CFG_LFR_PER_ROAM_REST_TIME);
	lfr->per_roam_monitor_time =
		cfg_get(psoc, CFG_LFR_PER_ROAM_MONITOR_TIME);
	lfr->per_roam_min_candidate_rssi =
		cfg_get(psoc, CFG_LFR_PER_ROAM_MIN_CANDIDATE_RSSI);
	lfr->lfr3_disallow_duration =
		cfg_get(psoc, CFG_LFR3_ROAM_DISALLOW_DURATION);
	lfr->lfr3_rssi_channel_penalization =
		cfg_get(psoc, CFG_LFR3_ROAM_RSSI_CHANNEL_PENALIZATION);
	lfr->lfr3_num_disallowed_aps =
		cfg_get(psoc, CFG_LFR3_ROAM_NUM_DISALLOWED_APS);
	lfr->rssi_boost_threshold_5g =
		cfg_get(psoc, CFG_LFR_5G_RSSI_BOOST_THRESHOLD);
	lfr->rssi_boost_factor_5g =
		cfg_get(psoc, CFG_LFR_5G_RSSI_BOOST_FACTOR);
	lfr->max_rssi_boost_5g =
		cfg_get(psoc, CFG_LFR_5G_MAX_RSSI_BOOST);
	lfr->rssi_penalize_threshold_5g =
		cfg_get(psoc, CFG_LFR_5G_RSSI_PENALIZE_THRESHOLD);
	lfr->rssi_penalize_factor_5g =
		cfg_get(psoc, CFG_LFR_5G_RSSI_PENALIZE_FACTOR);
	lfr->max_rssi_penalize_5g =
		cfg_get(psoc, CFG_LFR_5G_MAX_RSSI_PENALIZE);
	lfr->max_num_pre_auth = (uint32_t)
		cfg_default(CFG_LFR_MAX_NUM_PRE_AUTH);
}

static uint32_t
mlme_limit_max_per_index_score(uint32_t per_index_score)
{
	uint8_t i, score;

	for (i = 0; i < MAX_INDEX_PER_INI; i++) {
		score = WLAN_GET_SCORE_PERCENTAGE(per_index_score, i);
		if (score > MAX_INDEX_SCORE)
			WLAN_SET_SCORE_PERCENTAGE(per_index_score,
				MAX_INDEX_SCORE, i);
	}

	return per_index_score;
}

static void mlme_init_scoring_cfg(struct wlan_objmgr_psoc *psoc,
				  struct wlan_mlme_scoring_cfg *scoring_cfg)
{
	uint32_t total_weight;

	scoring_cfg->enable_scoring_for_roam =
		cfg_get(psoc, CFG_ENABLE_SCORING_FOR_ROAM);
	scoring_cfg->weight_cfg.rssi_weightage =
		cfg_get(psoc, CFG_SCORING_RSSI_WEIGHTAGE);
	scoring_cfg->weight_cfg.ht_caps_weightage =
		cfg_get(psoc, CFG_SCORING_HT_CAPS_WEIGHTAGE);
	scoring_cfg->weight_cfg.vht_caps_weightage =
		cfg_get(psoc, CFG_SCORING_VHT_CAPS_WEIGHTAGE);
	scoring_cfg->weight_cfg.he_caps_weightage =
		cfg_get(psoc, CFG_SCORING_HE_CAPS_WEIGHTAGE);
	scoring_cfg->weight_cfg.chan_width_weightage =
		cfg_get(psoc, CFG_SCORING_CHAN_WIDTH_WEIGHTAGE);
	scoring_cfg->weight_cfg.chan_band_weightage =
		cfg_get(psoc, CFG_SCORING_CHAN_BAND_WEIGHTAGE);
	scoring_cfg->weight_cfg.nss_weightage =
		cfg_get(psoc, CFG_SCORING_NSS_WEIGHTAGE);
	scoring_cfg->weight_cfg.beamforming_cap_weightage =
		cfg_get(psoc, CFG_SCORING_BEAMFORM_CAP_WEIGHTAGE);
	scoring_cfg->weight_cfg.pcl_weightage =
		cfg_get(psoc, CFG_SCORING_PCL_WEIGHTAGE);
	scoring_cfg->weight_cfg.channel_congestion_weightage =
		cfg_get(psoc, CFG_SCORING_CHAN_CONGESTION_WEIGHTAGE);
	scoring_cfg->weight_cfg.oce_wan_weightage =
		cfg_get(psoc, CFG_SCORING_OCE_WAN_WEIGHTAGE);

	total_weight = scoring_cfg->enable_scoring_for_roam +
			scoring_cfg->weight_cfg.rssi_weightage +
			scoring_cfg->weight_cfg.ht_caps_weightage +
			scoring_cfg->weight_cfg.vht_caps_weightage +
			scoring_cfg->weight_cfg.he_caps_weightage +
			scoring_cfg->weight_cfg.chan_width_weightage +
			scoring_cfg->weight_cfg.chan_band_weightage +
			scoring_cfg->weight_cfg.nss_weightage +
			scoring_cfg->weight_cfg.beamforming_cap_weightage +
			scoring_cfg->weight_cfg.pcl_weightage +
			scoring_cfg->weight_cfg.channel_congestion_weightage +
			scoring_cfg->weight_cfg.oce_wan_weightage;

	/*
	 * If configured weights are greater than max weight,
	 * fallback to default weights
	 */
	if (total_weight > BEST_CANDIDATE_MAX_WEIGHT) {
		mlme_err("Total weight greater than %d, using default weights",
			 BEST_CANDIDATE_MAX_WEIGHT);
		scoring_cfg->weight_cfg.rssi_weightage = RSSI_WEIGHTAGE;
		scoring_cfg->weight_cfg.ht_caps_weightage =
						HT_CAPABILITY_WEIGHTAGE;
		scoring_cfg->weight_cfg.vht_caps_weightage =
						VHT_CAP_WEIGHTAGE;
		scoring_cfg->weight_cfg.he_caps_weightage = HE_CAP_WEIGHTAGE;
		scoring_cfg->weight_cfg.chan_width_weightage =
						CHAN_WIDTH_WEIGHTAGE;
		scoring_cfg->weight_cfg.chan_band_weightage =
						CHAN_BAND_WEIGHTAGE;
		scoring_cfg->weight_cfg.nss_weightage = NSS_WEIGHTAGE;
		scoring_cfg->weight_cfg.beamforming_cap_weightage =
						BEAMFORMING_CAP_WEIGHTAGE;
		scoring_cfg->weight_cfg.pcl_weightage = PCL_WEIGHT;
		scoring_cfg->weight_cfg.channel_congestion_weightage =
						CHANNEL_CONGESTION_WEIGHTAGE;
		scoring_cfg->weight_cfg.oce_wan_weightage = OCE_WAN_WEIGHTAGE;
	}

	scoring_cfg->rssi_score.best_rssi_threshold =
		cfg_get(psoc, CFG_SCORING_BEST_RSSI_THRESHOLD);
	scoring_cfg->rssi_score.good_rssi_threshold =
		cfg_get(psoc, CFG_SCORING_GOOD_RSSI_THRESHOLD);
	scoring_cfg->rssi_score.bad_rssi_threshold =
		cfg_get(psoc, CFG_SCORING_BAD_RSSI_THRESHOLD);

	scoring_cfg->rssi_score.good_rssi_pcnt =
		cfg_get(psoc, CFG_SCORING_GOOD_RSSI_PERCENT);
	scoring_cfg->rssi_score.bad_rssi_pcnt =
		cfg_get(psoc, CFG_SCORING_BAD_RSSI_PERCENT);

	scoring_cfg->rssi_score.good_rssi_bucket_size =
		cfg_get(psoc, CFG_SCORING_GOOD_RSSI_BUCKET_SIZE);
	scoring_cfg->rssi_score.bad_rssi_bucket_size =
		cfg_get(psoc, CFG_SCORING_BAD_RSSI_BUCKET_SIZE);

	scoring_cfg->rssi_score.rssi_pref_5g_rssi_thresh =
		cfg_get(psoc, CFG_SCORING_RSSI_PREF_5G_THRESHOLD);

	scoring_cfg->bandwidth_weight_per_index =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_BW_WEIGHT_PER_IDX));
	scoring_cfg->nss_weight_per_index =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_NSS_WEIGHT_PER_IDX));
	scoring_cfg->band_weight_per_index =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_BAND_WEIGHT_PER_IDX));

	scoring_cfg->esp_qbss_scoring.num_slot =
		cfg_get(psoc, CFG_SCORING_NUM_ESP_QBSS_SLOTS);
	scoring_cfg->esp_qbss_scoring.score_pcnt3_to_0 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_ESP_QBSS_SCORE_IDX_3_TO_0));
	scoring_cfg->esp_qbss_scoring.score_pcnt7_to_4 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_ESP_QBSS_SCORE_IDX_7_TO_4));
	scoring_cfg->esp_qbss_scoring.score_pcnt11_to_8 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_ESP_QBSS_SCORE_IDX_11_TO_8));
	scoring_cfg->esp_qbss_scoring.score_pcnt15_to_12 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_ESP_QBSS_SCORE_IDX_15_TO_12));

	scoring_cfg->oce_wan_scoring.num_slot =
		cfg_get(psoc, CFG_SCORING_NUM_OCE_WAN_SLOTS);
	scoring_cfg->oce_wan_scoring.score_pcnt3_to_0 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_OCE_WAN_SCORE_IDX_3_TO_0));
	scoring_cfg->oce_wan_scoring.score_pcnt7_to_4 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_OCE_WAN_SCORE_IDX_7_TO_4));
	scoring_cfg->oce_wan_scoring.score_pcnt11_to_8 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_OCE_WAN_SCORE_IDX_11_TO_8));
	scoring_cfg->oce_wan_scoring.score_pcnt15_to_12 =
		mlme_limit_max_per_index_score(
			cfg_get(psoc, CFG_SCORING_OCE_WAN_SCORE_IDX_15_TO_12));
}

static void mlme_init_oce_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_oce *oce)
{
	uint8_t val;
	bool rssi_assoc_reject_enabled;
	bool probe_req_rate_enabled;
	bool probe_resp_rate_enabled;
	bool beacon_rate_enabled;
	bool probe_req_deferral_enabled;
	bool fils_discovery_sap_enabled;
	bool esp_for_roam_enabled;

	oce->enable_bcast_probe_rsp =
		cfg_get(psoc, CFG_ENABLE_BCAST_PROBE_RESP);
	oce->oce_sta_enabled = cfg_get(psoc, CFG_OCE_ENABLE_STA);
	oce->oce_sap_enabled = cfg_get(psoc, CFG_OCE_ENABLE_SAP);
	oce->fils_enabled = cfg_get(psoc, CFG_IS_FILS_ENABLED);

	rssi_assoc_reject_enabled =
		cfg_get(psoc, CFG_OCE_ENABLE_RSSI_BASED_ASSOC_REJECT);
	probe_req_rate_enabled = cfg_get(psoc, CFG_OCE_PROBE_REQ_RATE);
	probe_resp_rate_enabled = cfg_get(psoc, CFG_OCE_PROBE_RSP_RATE);
	beacon_rate_enabled = cfg_get(psoc, CFG_OCE_BEACON_RATE);
	probe_req_deferral_enabled =
		cfg_get(psoc, CFG_ENABLE_PROBE_REQ_DEFERRAL);
	fils_discovery_sap_enabled =
		cfg_get(psoc, CFG_ENABLE_FILS_DISCOVERY_SAP);
	esp_for_roam_enabled = cfg_get(psoc, CFG_ENABLE_ESP_FEATURE);

	if (!rssi_assoc_reject_enabled ||
	    !oce->enable_bcast_probe_rsp) {
		oce->oce_sta_enabled = 0;
	}

	val = (probe_req_rate_enabled *
	WMI_VDEV_OCE_PROBE_REQUEST_RATE_FEATURE_BITMAP) +
	(probe_resp_rate_enabled *
	WMI_VDEV_OCE_PROBE_RESPONSE_RATE_FEATURE_BITMAP) +
	(beacon_rate_enabled *
	WMI_VDEV_OCE_BEACON_RATE_FEATURE_BITMAP) +
	(probe_req_deferral_enabled *
	 WMI_VDEV_OCE_PROBE_REQUEST_DEFERRAL_FEATURE_BITMAP) +
	(fils_discovery_sap_enabled *
	 WMI_VDEV_OCE_FILS_DISCOVERY_FRAME_FEATURE_BITMAP) +
	(esp_for_roam_enabled *
	 WMI_VDEV_OCE_ESP_FEATURE_BITMAP) +
	(rssi_assoc_reject_enabled *
	 WMI_VDEV_OCE_REASSOC_REJECT_FEATURE_BITMAP);
	oce->feature_bitmap = val;
}

QDF_STATUS mlme_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;
	struct wlan_mlme_cfg *mlme_cfg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		mlme_err("Failed to get MLME Obj");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_cfg = &mlme_obj->cfg;
	mlme_init_generic_cfg(psoc, &mlme_cfg->gen);
	mlme_init_edca_params(&mlme_cfg->edca_params);
	mlme_init_ht_cap_in_cfg(psoc, &mlme_cfg->ht_caps);
	mlme_init_mbo_cfg(psoc, &mlme_cfg->mbo_cfg);
	mlme_init_qos_cfg(psoc, &mlme_cfg->qos_mlme_params);
	mlme_init_rates_in_cfg(psoc, &mlme_cfg->rates);
	mlme_init_sap_protection_cfg(psoc, &mlme_cfg->sap_protection_cfg);
	mlme_init_chainmask_cfg(psoc, &mlme_cfg->chainmask_cfg);
	mlme_init_sap_cfg(psoc, &mlme_cfg->sap_cfg);
	mlme_init_he_cap_in_cfg(psoc, &mlme_cfg->he_caps);
	mlme_init_obss_ht40_cfg(psoc, &mlme_cfg->obss_ht40);
	mlme_init_sta_cfg(psoc, &mlme_cfg->sta);
	mlme_init_lfr_cfg(psoc, &mlme_cfg->lfr);
	mlme_init_scoring_cfg(psoc, &mlme_cfg->scoring);
	mlme_init_oce_cfg(psoc, &mlme_cfg->oce);

	return status;
}
