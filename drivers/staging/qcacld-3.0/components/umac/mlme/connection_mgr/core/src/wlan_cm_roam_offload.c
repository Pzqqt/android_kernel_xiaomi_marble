/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cm_roam_offload.c
 *
 * Implementation for the common roaming offload api interfaces.
 */

#include "wlan_mlme_main.h"
#include "wlan_cm_roam_offload.h"
#include "wlan_cm_tgt_if_tx_api.h"
#include "wlan_cm_roam_api.h"
#include "wlan_mlme_vdev_mgr_interface.h"
#include "wlan_crypto_global_api.h"
#include "wlan_psoc_mlme_api.h"
#include "pld_common.h"
#include "wlan_blm_api.h"
#include "wlan_scan_api.h"
#include "wlan_vdev_mgr_ucfg_api.h"
#include "wlan_p2p_cfg_api.h"
#include "wlan_cm_vdev_api.h"
#include "cfg_nan_api.h"
#include "wlan_mlme_api.h"
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include "connection_mgr/core/src/wlan_cm_main.h"
#include "connection_mgr/core/src/wlan_cm_sm.h"
#include "wlan_reg_ucfg_api.h"
#include "wlan_connectivity_logging.h"
#include "wlan_if_mgr_roam.h"
#include "wlan_mlo_mgr_roam.h"
#include "wlan_mlo_mgr_sta.h"
#include "wlan_mlme_api.h"
#include "wlan_policy_mgr_api.h"


#ifdef WLAN_FEATURE_SAE
#define CM_IS_FW_FT_SAE_SUPPORTED(fw_akm_bitmap) \
	(((fw_akm_bitmap) & (1 << AKM_FT_SAE)) ? true : false)

#define CM_IS_FW_SAE_ROAM_SUPPORTED(fw_akm_bitmap) \
	(((fw_akm_bitmap) & (1 << AKM_SAE)) ? true : false)
#else
#define CM_IS_FW_FT_SAE_SUPPORTED(fw_akm_bitmap) (false)

#define CM_IS_FW_SAE_ROAM_SUPPORTED(fw_akm_bitmap) (false)
#endif

/**
 * cm_roam_scan_bmiss_cnt() - set roam beacon miss count
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: roam beacon miss count parameters
 *
 * This function is used to set roam beacon miss count parameters
 *
 * Return: None
 */
static void
cm_roam_scan_bmiss_cnt(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		       struct wlan_roam_beacon_miss_cnt *params)
{
	uint8_t beacon_miss_count;

	params->vdev_id = vdev_id;

	wlan_mlme_get_roam_bmiss_first_bcnt(psoc, &beacon_miss_count);
	params->roam_bmiss_first_bcnt = beacon_miss_count;

	wlan_mlme_get_roam_bmiss_final_bcnt(psoc, &beacon_miss_count);
	params->roam_bmiss_final_bcnt = beacon_miss_count;
}

QDF_STATUS
cm_roam_fill_rssi_change_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				struct wlan_roam_rssi_change_params *params)
{
	struct cm_roam_values_copy temp;

	params->vdev_id = vdev_id;
	wlan_cm_roam_cfg_get_value(psoc, vdev_id,
				   RSSI_CHANGE_THRESHOLD, &temp);
	params->rssi_change_thresh = temp.int_value;

	wlan_cm_roam_cfg_get_value(psoc, vdev_id,
				   BEACON_RSSI_WEIGHT, &temp);
	params->bcn_rssi_weight = temp.uint_value;

	wlan_cm_roam_cfg_get_value(psoc, vdev_id,
				   HI_RSSI_DELAY_BTW_SCANS, &temp);
	params->hirssi_delay_btw_scans = temp.uint_value;

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_is_per_roam_allowed()  - Check if PER roam trigger needs to be
 * disabled based on the current connected rates.
 * @psoc:   Pointer to the psoc object
 * @vdev_id: Vdev id
 *
 * Return: true if PER roam trigger is allowed
 */
static bool
cm_roam_is_per_roam_allowed(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct qdf_mac_addr connected_bssid = {0};
	struct wlan_objmgr_vdev *vdev;
	enum wlan_phymode peer_phymode = WLAN_PHYMODE_AUTO;
	QDF_STATUS status;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("Vdev is null for vdev_id:%d", vdev_id);
		return false;
	}

	status = wlan_vdev_get_bss_peer_mac(vdev, &connected_bssid);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	if (QDF_IS_STATUS_ERROR(status))
		return false;

	mlme_get_peer_phymode(psoc, connected_bssid.bytes, &peer_phymode);
	if (peer_phymode < WLAN_PHYMODE_11NA_HT20) {
		mlme_debug("vdev:%d PER roam trigger disabled for phymode:%d",
			   peer_phymode, vdev_id);
		return false;
	}

	return true;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_roam_reason_vsie() - set roam reason vsie
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: roam reason vsie parameters
 *
 * This function is used to set roam reason vsie parameters
 *
 * Return: None
 */
static void
cm_roam_reason_vsie(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		    struct wlan_roam_reason_vsie_enable *params)
{
	uint8_t enable_roam_reason_vsie;

	params->vdev_id = vdev_id;

	wlan_mlme_get_roam_reason_vsie_status(psoc, &enable_roam_reason_vsie);
	params->enable_roam_reason_vsie = enable_roam_reason_vsie;
}

/**
 * cm_is_only_2g_band_supported()  - Check if BTC trigger and IDLE trigger needs
 * to be disabled based on the current connected band.
 * @psoc:   Pointer to the psoc object
 * @vdev_id: Vdev id
 *
 * Return: true if BTC trigger and IDLE trigger are allowed or not
 */
static bool
cm_is_only_2g_band_supported(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	bool only_2g_band_supported = false;
	uint32_t band_bitmap;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("Vdev is null for vdev_id:%d", vdev_id);
		return false;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("pdev is null for vdev_id:%d", vdev_id);
		goto release;
	}

	if (QDF_IS_STATUS_ERROR(ucfg_reg_get_band(pdev, &band_bitmap))) {
		mlme_debug("Failed to get band");
		goto release;
	}

	mlme_debug("Current band bitmap:%d", band_bitmap);

	if (band_bitmap & BIT(REG_BAND_2G) &&
	    !(band_bitmap & BIT(REG_BAND_5G)) &&
	    !(band_bitmap & BIT(REG_BAND_6G)))
		only_2g_band_supported = true;


release:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	return only_2g_band_supported;
}

/**
 * cm_roam_triggers() - set roam triggers
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: roam triggers parameters
 *
 * This function is used to set roam triggers parameters
 *
 * Return: None
 */
static void
cm_roam_triggers(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		 struct wlan_roam_triggers *params)
{
	bool is_per_roam_enabled;

	params->vdev_id = vdev_id;
	params->trigger_bitmap =
		mlme_get_roam_trigger_bitmap(psoc, vdev_id);

	/*
	 * Disable PER trigger for phymode less than 11n to avoid
	 * frequent roams as the PER rate threshold is greater than
	 * 11a/b/g rates
	 */
	is_per_roam_enabled = cm_roam_is_per_roam_allowed(psoc, vdev_id);
	if (!is_per_roam_enabled)
		params->trigger_bitmap &= ~BIT(ROAM_TRIGGER_REASON_PER);

	/*
	 * Enable BTC trigger and IDLE trigger only when DUT is dual band
	 * capable(2g + 5g/6g)
	 */
	if (cm_is_only_2g_band_supported(psoc, vdev_id)) {
		params->trigger_bitmap &= ~BIT(ROAM_TRIGGER_REASON_IDLE);
		params->trigger_bitmap &= ~BIT(ROAM_TRIGGER_REASON_BTC);
	}

	mlme_debug("[ROAM_TRIGGER] trigger_bitmap:%d", params->trigger_bitmap);

	params->roam_scan_scheme_bitmap =
		wlan_cm_get_roam_scan_scheme_bitmap(psoc, vdev_id);
	wlan_cm_roam_get_vendor_btm_params(psoc, &params->vendor_btm_param);
	wlan_cm_roam_get_score_delta_params(psoc, params);
	wlan_cm_roam_get_min_rssi_params(psoc, params);
}

/**
 * cm_roam_bss_load_config() - set bss load config
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: bss load config parameters
 *
 * This function is used to set bss load config parameters
 *
 * Return: None
 */
static void
cm_roam_bss_load_config(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			struct wlan_roam_bss_load_config *params)
{
	params->vdev_id = vdev_id;
	wlan_mlme_get_bss_load_threshold(psoc, &params->bss_load_threshold);
	wlan_mlme_get_bss_load_sample_time(psoc, &params->bss_load_sample_time);
	wlan_mlme_get_bss_load_rssi_threshold_5ghz(
					psoc, &params->rssi_threshold_5ghz);
	wlan_mlme_get_bss_load_rssi_threshold_24ghz(
					psoc, &params->rssi_threshold_24ghz);
}

/**
 * cm_roam_disconnect_params() - set disconnect roam parameters
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: disconnect roam parameters
 *
 * This function is used to set disconnect roam parameters
 *
 * Return: None
 */
static void
cm_roam_disconnect_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			  struct wlan_roam_disconnect_params *params)
{
	params->vdev_id = vdev_id;
	wlan_mlme_get_enable_disconnect_roam_offload(psoc, &params->enable);
}

/**
 * cm_roam_idle_params() - set roam idle parameters
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: roam idle parameters
 *
 * This function is used to set roam idle parameters
 *
 * Return: None
 */
static void
cm_roam_idle_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		    struct wlan_roam_idle_params *params)
{
	params->vdev_id = vdev_id;
	wlan_mlme_get_enable_idle_roam(psoc, &params->enable);
	wlan_mlme_get_idle_roam_rssi_delta(psoc, &params->conn_ap_rssi_delta);
	wlan_mlme_get_idle_roam_inactive_time(psoc, &params->inactive_time);
	wlan_mlme_get_idle_data_packet_count(psoc, &params->data_pkt_count);
	wlan_mlme_get_idle_roam_min_rssi(psoc, &params->conn_ap_min_rssi);
	wlan_mlme_get_idle_roam_band(psoc, &params->band);
}

#define RSN_CAPS_SHIFT 16

#ifdef WLAN_ADAPTIVE_11R
static inline void
cm_update_rso_adaptive_11r(struct wlan_rso_11r_params *dst,
			   struct rso_config *rso_cfg)
{
	dst->is_adaptive_11r = rso_cfg->is_adaptive_11r_connection;
}

QDF_STATUS
cm_exclude_rm_partial_scan_freq(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id, uint8_t param_value)
{
	QDF_STATUS status;

	wlan_cm_set_exclude_rm_partial_scan_freq(psoc, param_value);
	status = wlan_cm_tgt_exclude_rm_partial_scan_freq(psoc, vdev_id,
							  param_value);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to exclude roam partial scan channels");

	return status;
}

QDF_STATUS cm_roam_full_scan_6ghz_on_disc(struct wlan_objmgr_psoc *psoc,
					  uint8_t vdev_id,
					  uint8_t param_value)
{
	QDF_STATUS status;

	wlan_cm_roam_set_full_scan_6ghz_on_disc(psoc, param_value);
	status = wlan_cm_tgt_send_roam_full_scan_6ghz_on_disc(psoc, vdev_id,
							      param_value);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send 6 GHz channels inclusion in full scan");

	return status;
}
#else
static inline void
cm_update_rso_adaptive_11r(struct wlan_rso_11r_params *dst,
			   struct rso_config *rso_cfg)
{}
#endif

#ifdef FEATURE_WLAN_ESE
static void
cm_update_rso_ese_info(struct rso_config *rso_cfg,
		       struct wlan_roam_scan_offload_params *rso_config)
{
	rso_config->rso_ese_info.is_ese_assoc = rso_cfg->is_ese_assoc;
	rso_config->rso_11r_info.is_11r_assoc = rso_cfg->is_11r_assoc;
	if (rso_cfg->is_ese_assoc) {
		qdf_mem_copy(rso_config->rso_ese_info.krk, rso_cfg->krk,
			     WMI_KRK_KEY_LEN);
		qdf_mem_copy(rso_config->rso_ese_info.btk, rso_cfg->btk,
			     WMI_BTK_KEY_LEN);
		rso_config->rso_11i_info.fw_okc = 0;
		rso_config->rso_11i_info.fw_pmksa_cache = 0;
		rso_config->rso_11i_info.pmk_len = 0;
		qdf_mem_zero(&rso_config->rso_11i_info.psk_pmk[0],
			     sizeof(rso_config->rso_11i_info.psk_pmk));
	}
}
#else
static inline void
cm_update_rso_ese_info(struct rso_config *rso_cfg,
		       struct wlan_roam_scan_offload_params *rso_config)
{}
#endif

#ifdef WLAN_SAE_SINGLE_PMK
static bool
cm_fill_rso_sae_single_pmk_info(struct wlan_objmgr_vdev *vdev,
				struct wlan_mlme_psoc_ext_obj *mlme_obj,
				struct wlan_roam_scan_offload_params *rso_cfg)
{
	struct wlan_mlme_sae_single_pmk single_pmk = {0};
	struct wlan_rso_11i_params *rso_11i_info = &rso_cfg->rso_11i_info;
	uint64_t time_expired;

	wlan_mlme_get_sae_single_pmk_info(vdev, &single_pmk);

	if (single_pmk.pmk_info.pmk_len && single_pmk.sae_single_pmk_ap &&
	    mlme_obj->cfg.lfr.sae_single_pmk_feature_enabled) {

		rso_11i_info->pmk_len = single_pmk.pmk_info.pmk_len;
		/* Update sae same pmk info in rso */
		qdf_mem_copy(rso_11i_info->psk_pmk, single_pmk.pmk_info.pmk,
			     rso_11i_info->pmk_len);
		rso_11i_info->is_sae_same_pmk = single_pmk.sae_single_pmk_ap;

		/* get the time expired in seconds */
		time_expired = (qdf_get_system_timestamp() -
				single_pmk.pmk_info.spmk_timestamp) / 1000;

		rso_cfg->sae_offload_params.spmk_timeout = 0;
		if (time_expired < single_pmk.pmk_info.spmk_timeout_period)
			rso_cfg->sae_offload_params.spmk_timeout =
				(single_pmk.pmk_info.spmk_timeout_period  -
				 time_expired);

		mlme_debug("Update spmk with len:%d is_spmk_ap:%d time_exp:%lld time left:%d",
			   single_pmk.pmk_info.pmk_len,
			   single_pmk.sae_single_pmk_ap, time_expired,
			   rso_cfg->sae_offload_params.spmk_timeout);

		return true;
	}

	return false;
}
#else
static inline bool
cm_fill_rso_sae_single_pmk_info(struct wlan_objmgr_vdev *vdev,
				struct wlan_mlme_psoc_ext_obj *mlme_obj,
				struct wlan_roam_scan_offload_params *rso_cfg)
{
	return false;
}
#endif

static QDF_STATUS
cm_roam_scan_offload_fill_lfr3_config(struct wlan_objmgr_vdev *vdev,
			struct rso_config *rso_cfg,
			struct wlan_roam_scan_offload_params *rso_config,
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			uint8_t command, uint32_t *mode)
{
	tSirMacCapabilityInfo self_caps;
	tSirMacQosInfoStation sta_qos_info;
	uint16_t *final_caps_val;
	uint8_t *qos_cfg_val, temp_val;
	uint32_t pmkid_modes = mlme_obj->cfg.sta.pmkid_modes;
	uint32_t val = 0;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	qdf_size_t val_len;
	QDF_STATUS status;
	uint16_t rsn_caps = 0;
	int32_t uccipher, authmode, akm;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct cm_roam_values_copy roam_config;
	struct mlme_legacy_priv *mlme_priv;
	uint8_t uapsd_mask;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		return QDF_STATUS_E_FAILURE;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_INVAL;
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

	rso_config->roam_offload_enabled =
		mlme_obj->cfg.lfr.lfr3_roaming_offload;
	if (!rso_config->roam_offload_enabled)
		return QDF_STATUS_SUCCESS;

	/* FILL LFR3 specific roam scan mode TLV parameters */
	rso_config->rso_lfr3_params.roam_rssi_cat_gap =
		mlme_obj->cfg.lfr.rso_user_config.cat_rssi_offset;
	rso_config->rso_lfr3_params.prefer_5ghz =
		mlme_obj->cfg.lfr.roam_prefer_5ghz;
	rso_config->rso_lfr3_params.select_5ghz_margin =
		mlme_obj->cfg.gen.select_5ghz_margin;
	rso_config->rso_lfr3_params.reassoc_failure_timeout =
		mlme_obj->cfg.timeouts.reassoc_failure_timeout;
	rso_config->rso_lfr3_params.ho_delay_for_rx =
		mlme_obj->cfg.lfr.ho_delay_for_rx;
	rso_config->rso_lfr3_params.roam_retry_count =
		mlme_obj->cfg.lfr.roam_preauth_retry_count;
	rso_config->rso_lfr3_params.roam_preauth_no_ack_timeout =
		mlme_obj->cfg.lfr.roam_preauth_no_ack_timeout;
	rso_config->rso_lfr3_params.rct_validity_timer =
		mlme_obj->cfg.btm.rct_validity_timer;
	rso_config->rso_lfr3_params.disable_self_roam =
		!mlme_obj->cfg.lfr.enable_self_bss_roam;
	if (!rso_cfg->roam_control_enable &&
	    mlme_obj->cfg.lfr.roam_force_rssi_trigger)
		*mode |= WMI_ROAM_SCAN_MODE_RSSI_CHANGE;
	/*
	 * Self rsn caps aren't sent to firmware, so in case of PMF required,
	 * the firmware connects to a non PMF AP advertising PMF not required
	 * in the re-assoc request which violates protocol.
	 * So send self RSN caps to firmware in roam SCAN offload command to
	 * let it configure the params in the re-assoc request too.
	 * Instead of making another infra, send the RSN-CAPS in MSB of
	 * beacon Caps.
	 */
	/* RSN caps with global user MFP which can be used for cross-AKM roam */
	rsn_caps = rso_cfg->rso_rsn_caps;

	/* Fill LFR3 specific self capabilities for roam scan mode TLV */
	self_caps.ess = 1;
	self_caps.ibss = 0;

	val = mlme_obj->cfg.wep_params.is_privacy_enabled;
	if (val)
		self_caps.privacy = 1;

	if (mlme_obj->cfg.ht_caps.short_preamble)
		self_caps.shortPreamble = 1;

	self_caps.criticalUpdateFlag = 0;
	self_caps.channelAgility = 0;

	if (mlme_obj->cfg.feature_flags.enable_short_slot_time_11g)
		self_caps.shortSlotTime = 1;

	if (mlme_obj->cfg.gen.enabled_11h)
		self_caps.spectrumMgt = 1;

	if (mlme_obj->cfg.wmm_params.qos_enabled)
		self_caps.qos = 1;

	if (mlme_obj->cfg.roam_scoring.apsd_enabled)
		self_caps.apsd = 1;

	self_caps.rrm = mlme_obj->cfg.rrm_config.rrm_enabled;

	val = mlme_obj->cfg.feature_flags.enable_block_ack;
	self_caps.delayedBA =
		(uint16_t)((val >> WNI_CFG_BLOCK_ACK_ENABLED_DELAYED) & 1);
	self_caps.immediateBA =
		(uint16_t)((val >> WNI_CFG_BLOCK_ACK_ENABLED_IMMEDIATE) & 1);
	final_caps_val = (uint16_t *)&self_caps;

	rso_config->rso_lfr3_caps.capability =
		(rsn_caps << RSN_CAPS_SHIFT) | ((*final_caps_val) & 0xFFFF);

	rso_config->rso_lfr3_caps.ht_caps_info =
		*(uint16_t *)&mlme_obj->cfg.ht_caps.ht_cap_info;
	rso_config->rso_lfr3_caps.ampdu_param =
		*(uint8_t *)&mlme_obj->cfg.ht_caps.ampdu_params;
	rso_config->rso_lfr3_caps.ht_ext_cap =
		*(uint16_t *)&mlme_obj->cfg.ht_caps.ext_cap_info;

	temp_val = (uint8_t)mlme_obj->cfg.vht_caps.vht_cap_info.tx_bf_cap;
	rso_config->rso_lfr3_caps.ht_txbf = temp_val & 0xFF;
	temp_val = (uint8_t)mlme_obj->cfg.vht_caps.vht_cap_info.as_cap;
	rso_config->rso_lfr3_caps.asel_cap = temp_val & 0xFF;

	qdf_mem_zero(&sta_qos_info, sizeof(tSirMacQosInfoStation));
	sta_qos_info.maxSpLen =
		(uint8_t)mlme_obj->cfg.wmm_params.max_sp_length;
	sta_qos_info.moreDataAck = 0;
	sta_qos_info.qack = 0;
	wlan_cm_roam_cfg_get_value(psoc, vdev_id, UAPSD_MASK, &roam_config);
	uapsd_mask = roam_config.uint_value;
	sta_qos_info.acbe_uapsd = SIR_UAPSD_GET(ACBE, uapsd_mask);
	sta_qos_info.acbk_uapsd = SIR_UAPSD_GET(ACBK, uapsd_mask);
	sta_qos_info.acvi_uapsd = SIR_UAPSD_GET(ACVI, uapsd_mask);
	sta_qos_info.acvo_uapsd = SIR_UAPSD_GET(ACVO, uapsd_mask);
	qos_cfg_val = (uint8_t *)&sta_qos_info;
	rso_config->rso_lfr3_caps.qos_caps = (*qos_cfg_val) & 0xFF;
	if (rso_config->rso_lfr3_caps.qos_caps)
		rso_config->rso_lfr3_caps.qos_enabled = true;

	rso_config->rso_lfr3_caps.wmm_caps = 0x4;

	val_len = ROAM_OFFLOAD_NUM_MCS_SET;
	status =
	    wlan_mlme_get_cfg_str((uint8_t *)rso_config->rso_lfr3_caps.mcsset,
				  &mlme_obj->cfg.rates.supported_mcs_set,
				  &val_len);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to get CFG_SUPPORTED_MCS_SET");
		return QDF_STATUS_E_FAILURE;
	}

	/* Update 11i TLV related Fields */
	rso_config->rso_11i_info.roam_key_mgmt_offload_enabled =
			mlme_obj->cfg.lfr.lfr3_roaming_offload;
	rso_config->rso_11i_info.fw_okc =
			(pmkid_modes & CFG_PMKID_MODES_OKC) ? 1 : 0;
	rso_config->rso_11i_info.fw_pmksa_cache =
			(pmkid_modes & CFG_PMKID_MODES_PMKSA_CACHING) ? 1 : 0;

	/* Check whether to send psk_pmk or sae_single pmk info */
	if (!cm_fill_rso_sae_single_pmk_info(vdev, mlme_obj, rso_config)) {
		rso_config->rso_11i_info.is_sae_same_pmk = false;
		wlan_cm_get_psk_pmk(pdev, vdev_id,
				    rso_config->rso_11i_info.psk_pmk,
				    &rso_config->rso_11i_info.pmk_len);
	}

	rso_config->rso_11r_info.enable_ft_im_roaming =
		mlme_obj->cfg.lfr.enable_ft_im_roaming;
	rso_config->rso_11r_info.mdid.mdie_present =
		rso_cfg->mdid.mdie_present;
	rso_config->rso_11r_info.mdid.mobility_domain =
		rso_cfg->mdid.mobility_domain;
	rso_config->rso_11r_info.r0kh_id_length =
			mlme_priv->connect_info.ft_info.r0kh_id_len;
	qdf_mem_copy(rso_config->rso_11r_info.r0kh_id,
		     mlme_priv->connect_info.ft_info.r0kh_id,
		     mlme_priv->connect_info.ft_info.r0kh_id_len);
	wlan_cm_get_psk_pmk(pdev, vdev_id,
			    rso_config->rso_11r_info.psk_pmk,
			    &rso_config->rso_11r_info.pmk_len);
	rso_config->rso_11r_info.enable_ft_over_ds =
		mlme_obj->cfg.lfr.enable_ft_over_ds;

	cm_update_rso_adaptive_11r(&rso_config->rso_11r_info, rso_cfg);
	cm_update_rso_ese_info(rso_cfg, rso_config);
	uccipher = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	authmode = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_AUTH_MODE);
	akm = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);

	rso_config->akm =
		cm_crypto_authmode_to_wmi_authmode(authmode, akm, uccipher);

	return QDF_STATUS_SUCCESS;
}

bool
cm_roam_is_change_in_band_allowed(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint32_t roam_band_mask)
{
	struct wlan_objmgr_vdev *vdev;
	bool concurrency_is_dbs;
	struct wlan_channel *chan;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("vdev is NULL");
		return false;
	}

	chan = wlan_vdev_get_active_channel(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
	if (!chan) {
		mlme_err("no active channel");
		return false;
	}

	concurrency_is_dbs = policy_mgr_concurrent_sta_doing_dbs(psoc);
	if (!concurrency_is_dbs)
		return true;

	mlme_debug("STA + STA concurrency is in DBS. ch freq %d, roam band:%d",
		   chan->ch_freq, roam_band_mask);

	if (wlan_reg_freq_to_band(chan->ch_freq) == REG_BAND_2G &&
	    (!(roam_band_mask & BIT(REG_BAND_2G)))) {
		mlme_debug("Change in band (2G to 5G/6G) not allowed");
		return false;
	}

	if ((wlan_reg_freq_to_band(chan->ch_freq) == REG_BAND_5G ||
	     wlan_reg_freq_to_band(chan->ch_freq) == REG_BAND_6G) &&
	    (!(roam_band_mask & BIT(REG_BAND_5G)) &&
	     !(roam_band_mask & BIT(REG_BAND_6G)))) {
		mlme_debug("Change in band (5G/6G to 2G) not allowed");
		return false;
	}

	return true;
}

/**
 * cm_roam_send_rt_stats_config() - set roam stats parameters
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @param_value: roam stats param value
 *
 * This function is used to set roam event stats parameters
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_send_rt_stats_config(struct wlan_objmgr_psoc *psoc,
			     uint8_t vdev_id, uint8_t param_value)
{
	struct roam_disable_cfg *req;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->vdev_id = vdev_id;
	req->cfg = param_value;

	status = wlan_cm_tgt_send_roam_rt_stats_config(psoc, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send roam rt stats config");

	qdf_mem_free(req);

	return status;
}

#else
static inline void
cm_roam_reason_vsie(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		    struct wlan_roam_reason_vsie_enable *params)
{
}

static inline void
cm_roam_triggers(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		 struct wlan_roam_triggers *params)
{
}

static void
cm_roam_bss_load_config(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			struct wlan_roam_bss_load_config *params)
{
}

static void
cm_roam_disconnect_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			  struct wlan_roam_disconnect_params *params)
{
}

static void
cm_roam_idle_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		    struct wlan_roam_idle_params *params)
{
}
static inline QDF_STATUS
cm_roam_scan_offload_fill_lfr3_config(struct wlan_objmgr_vdev *vdev,
			struct rso_config *rso_cfg,
			struct wlan_roam_scan_offload_params *rso_config,
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			uint8_t command, uint32_t *mode)
{
	if (mlme_obj->cfg.lfr.roam_force_rssi_trigger)
		*mode |= WMI_ROAM_SCAN_MODE_RSSI_CHANGE;

	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(WLAN_FEATURE_ROAM_OFFLOAD) && defined(WLAN_FEATURE_FILS_SK)
static QDF_STATUS
cm_roam_scan_offload_add_fils_params(struct wlan_objmgr_psoc *psoc,
				struct wlan_roam_scan_offload_params *rso_cfg,
				uint8_t vdev_id)
{
	QDF_STATUS status;
	uint32_t usr_name_len;
	struct wlan_fils_connection_info *fils_info;
	struct wlan_roam_fils_params *fils_roam_config =
				&rso_cfg->fils_roam_config;

	fils_info = wlan_cm_get_fils_connection_info(psoc, vdev_id);
	if (!fils_info)
		return QDF_STATUS_SUCCESS;

	if (fils_info->key_nai_length > FILS_MAX_KEYNAME_NAI_LENGTH ||
	    fils_info->r_rk_length > WLAN_FILS_MAX_RRK_LENGTH) {
		mlme_err("Fils info len error: keyname nai len(%d) rrk len(%d)",
			 fils_info->key_nai_length, fils_info->r_rk_length);
		return QDF_STATUS_E_FAILURE;
	}

	fils_roam_config->next_erp_seq_num = fils_info->erp_sequence_number;

	usr_name_len =
		qdf_str_copy_all_before_char(fils_info->keyname_nai,
					     sizeof(fils_info->keyname_nai),
					     fils_roam_config->username,
					     sizeof(fils_roam_config->username),
					     '@');
	if (fils_info->key_nai_length <= usr_name_len) {
		mlme_err("Fils info len error: key nai len %d, user name len %d",
			 fils_info->key_nai_length, usr_name_len);
		return QDF_STATUS_E_INVAL;
	}

	fils_roam_config->username_length = usr_name_len;
	qdf_mem_copy(fils_roam_config->rrk, fils_info->r_rk,
		     fils_info->r_rk_length);
	fils_roam_config->rrk_length = fils_info->r_rk_length;
	fils_roam_config->realm_len = fils_info->key_nai_length -
				fils_roam_config->username_length - 1;
	qdf_mem_copy(fils_roam_config->realm,
		     (fils_info->keyname_nai +
		     fils_roam_config->username_length + 1),
		     fils_roam_config->realm_len);

	/*
	 * Set add FILS tlv true for initial FULL EAP connection and subsequent
	 * FILS connection.
	 */
	rso_cfg->add_fils_tlv = true;
	mlme_debug("Fils: next_erp_seq_num %d rrk_len %d realm_len:%d",
		   fils_info->erp_sequence_number,
		   fils_info->r_rk_length,
		   fils_info->realm_len);
	if (!fils_info->is_fils_connection)
		return QDF_STATUS_SUCCESS;

	/* Update rik from crypto to fils roam config buffer */
	status = wlan_crypto_create_fils_rik(fils_info->r_rk,
					     fils_info->r_rk_length,
					     fils_info->rik,
					     &fils_info->rik_length);
	qdf_mem_copy(fils_roam_config->rik, fils_info->rik,
		     fils_info->rik_length);
	fils_roam_config->rik_length = fils_info->rik_length;

	fils_roam_config->fils_ft_len = fils_info->fils_ft_len;
	qdf_mem_copy(fils_roam_config->fils_ft, fils_info->fils_ft,
		     fils_info->fils_ft_len);

	return status;
}
#else
static inline
QDF_STATUS cm_roam_scan_offload_add_fils_params(struct wlan_objmgr_psoc *psoc,
			struct wlan_roam_scan_offload_params *rso_cfg,
			uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * cm_roam_mawc_params() - set roam mawc parameters
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @params: roam mawc parameters
 *
 * This function is used to set roam mawc parameters
 *
 * Return: None
 */
static void
cm_roam_mawc_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		    struct wlan_roam_mawc_params *params)
{
	bool mawc_enabled;
	bool mawc_roam_enabled;

	params->vdev_id = vdev_id;
	wlan_mlme_get_mawc_enabled(psoc, &mawc_enabled);
	wlan_mlme_get_mawc_roam_enabled(psoc, &mawc_roam_enabled);
	params->enable = mawc_enabled && mawc_roam_enabled;
	wlan_mlme_get_mawc_roam_traffic_threshold(
				psoc, &params->traffic_load_threshold);
	wlan_mlme_get_mawc_roam_ap_rssi_threshold(
				psoc, &params->best_ap_rssi_threshold);
	wlan_mlme_get_mawc_roam_rssi_high_adjust(
				psoc, &params->rssi_stationary_high_adjust);
	wlan_mlme_get_mawc_roam_rssi_low_adjust(
				psoc, &params->rssi_stationary_low_adjust);
}

QDF_STATUS
cm_roam_send_disable_config(struct wlan_objmgr_psoc *psoc,
			    uint8_t vdev_id, uint8_t cfg)
{
	struct roam_disable_cfg *req;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->vdev_id = vdev_id;
	req->cfg = cfg;

	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_debug("MLO ROAM: skip RSO cmd for link vdev %d", vdev_id);
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	status = wlan_cm_tgt_send_roam_disable_config(psoc, vdev_id, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send roam disable config");

end:
	qdf_mem_free(req);

	return status;
}

/**
 * cm_roam_init_req() - roam init request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_init_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id, bool enable)
{
	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_debug("MLO ROAM: skip RSO cmd for link vdev %d", vdev_id);
		return QDF_STATUS_SUCCESS;
	}

	return wlan_cm_tgt_send_roam_offload_init(psoc, vdev_id, enable);
}

QDF_STATUS cm_rso_set_roam_trigger(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id,
				   struct wlan_roam_triggers *trigger)
{
	QDF_STATUS status;
	uint8_t reason = REASON_SUPPLICANT_DE_INIT_ROAMING;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc)
		return QDF_STATUS_E_INVAL;

	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_debug("MLO ROAM: skip RSO cmd for link vdev %d", vdev_id);
		return QDF_STATUS_SUCCESS;
	}

	mlme_set_roam_trigger_bitmap(psoc, trigger->vdev_id,
				     trigger->trigger_bitmap);

	if (trigger->trigger_bitmap)
		reason = REASON_SUPPLICANT_INIT_ROAMING;

	status = cm_roam_state_change(pdev, vdev_id,
			trigger->trigger_bitmap ? WLAN_ROAM_RSO_ENABLED :
			WLAN_ROAM_DEINIT,
			reason, NULL, false);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	return wlan_cm_tgt_send_roam_triggers(psoc, vdev_id, trigger);
}

static void cm_roam_set_roam_reason_better_ap(struct wlan_objmgr_psoc *psoc,
					      uint8_t vdev_id, bool set)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return;
	mlme_set_roam_reason_better_ap(vdev, set);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
}

/**
 * cm_roam_scan_offload_rssi_thresh() - set roam offload scan rssi
 * parameters
 * @psoc: psoc ctx
 * @vdev_id: vdev id
 * @params:  roam offload scan rssi related parameters
 * @rso_cfg: rso config
 *
 * This function is used to set roam offload scan rssi related parameters
 *
 * Return: None
 */
static void
cm_roam_scan_offload_rssi_thresh(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			struct wlan_roam_offload_scan_rssi_params *params,
			struct rso_config *rso_cfg)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_mlme_lfr_cfg *lfr_cfg;
	struct rso_config_params *rso_config;
	struct wlan_objmgr_vdev *vdev;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;
	rso_config = &mlme_obj->cfg.lfr.rso_user_config;

	lfr_cfg = &mlme_obj->cfg.lfr;

	if (rso_config->alert_rssi_threshold)
		params->rssi_thresh = rso_config->alert_rssi_threshold;
	else
		params->rssi_thresh =
			(int8_t)rso_cfg->cfg_param.neighbor_lookup_threshold *
			(-1);

	params->vdev_id = vdev_id;
	params->rssi_thresh_diff =
		rso_cfg->cfg_param.opportunistic_threshold_diff & 0x000000ff;
	params->hi_rssi_scan_max_count =
		rso_cfg->cfg_param.hi_rssi_scan_max_count;
	/*
	 * If the current operation channel is 5G frequency band, then
	 * there is no need to enable the HI_RSSI feature. This feature
	 * is useful only if we are connected to a 2.4 GHz AP and we wish
	 * to connect to a better 5GHz AP is available.
	 */
	if (rso_cfg->disable_hi_rssi)
		params->hi_rssi_scan_rssi_delta = 0;
	else
		params->hi_rssi_scan_rssi_delta =
			rso_cfg->cfg_param.hi_rssi_scan_rssi_delta;
	/*
	 * When the STA operating band is 2.4/5 GHz and if the high RSSI delta
	 * is configured through vendor command then the priority should be
	 * given to it and the high RSSI delta value will be overridden with it.
	 */
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("Cannot set high RSSI offset as vdev object is NULL for vdev %d",
			 vdev_id);
	} else {
		qdf_freq_t op_freq;

		op_freq = wlan_get_operation_chan_freq(vdev);
		if (!WLAN_REG_IS_6GHZ_CHAN_FREQ(op_freq)) {
			uint8_t roam_high_rssi_delta;

			roam_high_rssi_delta =
				wlan_cm_get_roam_scan_high_rssi_offset(psoc);
			if (roam_high_rssi_delta)
				params->hi_rssi_scan_rssi_delta =
							roam_high_rssi_delta;
			/*
			 * Firmware will use this flag to enable 5 to 6 GHz
			 * high RSSI roam
			 */
			if (roam_high_rssi_delta &&
			    WLAN_REG_IS_5GHZ_CH_FREQ(op_freq))
				params->flags |=
					ROAM_SCAN_RSSI_THRESHOLD_FLAG_ROAM_HI_RSSI_EN_ON_5G;
		}
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	}

	params->hi_rssi_scan_rssi_ub =
		rso_cfg->cfg_param.hi_rssi_scan_rssi_ub;
	params->raise_rssi_thresh_5g = lfr_cfg->rssi_boost_threshold_5g;
	params->dense_rssi_thresh_offset = lfr_cfg->roam_dense_rssi_thre_offset;
	params->dense_min_aps_cnt = lfr_cfg->roam_dense_min_aps;
	params->traffic_threshold = lfr_cfg->roam_dense_traffic_threshold;

	/* Set initial dense roam status */
	if (rso_cfg->roam_candidate_count > params->dense_min_aps_cnt)
		params->initial_dense_status = true;

	params->bg_scan_bad_rssi_thresh =
			lfr_cfg->roam_bg_scan_bad_rssi_threshold;
	params->bg_scan_client_bitmap = lfr_cfg->roam_bg_scan_client_bitmap;
	params->roam_bad_rssi_thresh_offset_2g =
			lfr_cfg->roam_bg_scan_bad_rssi_offset_2g;
	params->roam_data_rssi_threshold_triggers =
		lfr_cfg->roam_data_rssi_threshold_triggers;
	params->roam_data_rssi_threshold = lfr_cfg->roam_data_rssi_threshold;
	params->rx_data_inactivity_time = lfr_cfg->rx_data_inactivity_time;

	params->drop_rssi_thresh_5g = lfr_cfg->rssi_penalize_threshold_5g;

	params->raise_factor_5g = lfr_cfg->rssi_boost_factor_5g;
	params->drop_factor_5g = lfr_cfg->rssi_penalize_factor_5g;
	params->max_raise_rssi_5g = lfr_cfg->max_rssi_boost_5g;
	params->max_drop_rssi_5g = lfr_cfg->max_rssi_penalize_5g;

	if (rso_config->good_rssi_roam)
		params->good_rssi_threshold = NOISE_FLOOR_DBM_DEFAULT;
	else
		params->good_rssi_threshold = 0;

	params->early_stop_scan_enable = lfr_cfg->early_stop_scan_enable;
	if (params->early_stop_scan_enable) {
		params->roam_earlystop_thres_min =
			lfr_cfg->early_stop_scan_min_threshold;
		params->roam_earlystop_thres_max =
			lfr_cfg->early_stop_scan_max_threshold;
	}

	params->rssi_thresh_offset_5g =
		rso_cfg->cfg_param.rssi_thresh_offset_5g;
}

/**
 * cm_roam_scan_offload_scan_period() - set roam offload scan period
 * parameters
 * @vdev_id: vdev id
 * @params:  roam offload scan period related parameters
 * @rso_cfg: rso config
 *
 * This function is used to set roam offload scan period related parameters
 *
 * Return: None
 */
static void
cm_roam_scan_offload_scan_period(uint8_t vdev_id,
				 struct wlan_roam_scan_period_params *params,
				 struct rso_config *rso_cfg)
{
	struct rso_cfg_params *cfg_params;

	cfg_params = &rso_cfg->cfg_param;

	params->vdev_id = vdev_id;
	params->empty_scan_refresh_period =
				cfg_params->empty_scan_refresh_period;
	params->scan_period = params->empty_scan_refresh_period;
	params->scan_age = (3 * params->empty_scan_refresh_period);
	params->roam_scan_inactivity_time =
				cfg_params->roam_scan_inactivity_time;
	params->roam_inactive_data_packet_count =
			cfg_params->roam_inactive_data_packet_count;
	params->roam_scan_period_after_inactivity =
			cfg_params->roam_scan_period_after_inactivity;
	params->full_scan_period =
			cfg_params->full_roam_scan_period;
}

static void
cm_roam_fill_11w_params(struct wlan_objmgr_vdev *vdev,
			struct ap_profile *profile)
{
	uint32_t group_mgmt_cipher;
	bool peer_rmf_capable = false;
	uint32_t keymgmt;
	uint16_t rsn_caps;

	/*
	 * Get rsn cap of link, intersection of self cap and bss cap,
	 * Only set PMF flags when both STA and current AP has MFP enabled
	 */
	rsn_caps = (uint16_t)wlan_crypto_get_param(vdev,
						   WLAN_CRYPTO_PARAM_RSN_CAP);
	if (wlan_crypto_vdev_has_mgmtcipher(vdev,
					(1 << WLAN_CRYPTO_CIPHER_AES_GMAC) |
					(1 << WLAN_CRYPTO_CIPHER_AES_GMAC_256) |
					(1 << WLAN_CRYPTO_CIPHER_AES_CMAC)) &&
					(rsn_caps &
					 WLAN_CRYPTO_RSN_CAP_MFP_ENABLED))
		peer_rmf_capable = true;

	keymgmt = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MGMT_CIPHER);

	if (keymgmt & (1 << WLAN_CRYPTO_CIPHER_AES_CMAC))
		group_mgmt_cipher = WMI_CIPHER_AES_CMAC;
	else if (keymgmt & (1 << WLAN_CRYPTO_CIPHER_AES_GMAC))
		group_mgmt_cipher = WMI_CIPHER_AES_GMAC;
	else if (keymgmt & (1 << WLAN_CRYPTO_CIPHER_AES_GMAC_256))
		group_mgmt_cipher = WMI_CIPHER_BIP_GMAC_256;
	else
		group_mgmt_cipher = WMI_CIPHER_NONE;

	if (peer_rmf_capable) {
		profile->rsn_mcastmgmtcipherset = group_mgmt_cipher;
		profile->flags |= WMI_AP_PROFILE_FLAG_PMF;
	} else {
		profile->rsn_mcastmgmtcipherset = WMI_CIPHER_NONE;
	}
}

#ifdef WLAN_FEATURE_11BE_MLO
static void
cm_update_mlo_score_params(struct scoring_param *req_score_params,
			   struct weight_cfg *weight_config)
{
	req_score_params->eht_caps_weightage =
		weight_config->eht_caps_weightage;
	req_score_params->mlo_weightage =
		weight_config->mlo_weightage;
}
#else
static void
cm_update_mlo_score_params(struct scoring_param *req_score_params,
			   struct weight_cfg *weight_config)
{
}
#endif

void cm_update_owe_info(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_resp *rsp, uint8_t vdev_id)
{
	struct rso_config *rso_cfg;
	struct owe_transition_mode_info *owe_info;
	uint8_t *ie_ptr;
	uint32_t ie_len, akm;
	const uint8_t *owe_transition_ie = NULL;
	uint8_t length;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return;
	owe_info = &rso_cfg->owe_info;

	akm = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	if (!QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OWE))
		goto reset;

	mlme_debug("[OWE_TRANSITION]:Update the owe open bss's ssid");

	if (!rsp->connect_ies.bcn_probe_rsp.ptr ||
	    !rsp->connect_ies.bcn_probe_rsp.len ||
	    (rsp->connect_ies.bcn_probe_rsp.len <=
		(sizeof(struct wlan_frame_hdr) +
		offsetof(struct wlan_bcn_frame, ie)))) {
		mlme_debug("invalid beacon probe rsp len %d",
			   rsp->connect_ies.bcn_probe_rsp.len);
		goto reset;
	}

	ie_len = (rsp->connect_ies.bcn_probe_rsp.len -
			sizeof(struct wlan_frame_hdr) -
			offsetof(struct wlan_bcn_frame, ie));
	ie_ptr = (uint8_t *)(rsp->connect_ies.bcn_probe_rsp.ptr +
			     sizeof(struct wlan_frame_hdr) +
			     offsetof(struct wlan_bcn_frame, ie));

	owe_transition_ie = wlan_get_vendor_ie_ptr_from_oui(
				OWE_TRANSITION_OUI_TYPE,
				OWE_TRANSITION_OUI_SIZE, ie_ptr, ie_len);
	if (!owe_transition_ie || owe_transition_ie[1] <= OWE_SSID_OFFSET) {
		mlme_debug("[OWE_TRANSITION]: Invalid owe transition ie");
		goto reset;
	}

	owe_info->is_owe_transition_conn = true;

	length = *(owe_transition_ie + OWE_SSID_LEN_OFFSET);
	if (length > WLAN_SSID_MAX_LEN) {
		mlme_debug("[OWE_TRANSITION] Invalid ssid len %d", length);
		goto reset;
	}
	owe_info->ssid.length = length;
	qdf_mem_copy(owe_info->ssid.ssid, owe_transition_ie + OWE_SSID_OFFSET,
		     owe_info->ssid.length);

	mlme_debug("[OWE_TRANSITION] open bss ssid: \"%.*s\"",
		   owe_info->ssid.length, owe_info->ssid.ssid);
	return;

reset:
	if (owe_info->is_owe_transition_conn)
		owe_info->is_owe_transition_conn = false;

	return;
}

/**
 * cm_update_owe_ap_profile() - set owe ap profile
 * @params:  roam offload scan period related parameters
 * @rso_cfg: rso config
 *
 * This function is used to set OPEN SSID value when STA is connected to OWE
 * transition AP in OWE security
 *
 * Return: None
 */
static void cm_update_owe_ap_profile(struct ap_profile_params *params,
				     struct rso_config *rso_cfg)
{
	struct owe_transition_mode_info *owe_ap_profile;
	bool is_owe_transition_conn;

	owe_ap_profile = &params->owe_ap_profile;
	is_owe_transition_conn = rso_cfg->owe_info.is_owe_transition_conn;
	mlme_debug("set owe ap profile:%d", is_owe_transition_conn);
	owe_ap_profile->is_owe_transition_conn = is_owe_transition_conn;
	owe_ap_profile->ssid.length = rso_cfg->owe_info.ssid.length;
	qdf_mem_copy(owe_ap_profile->ssid.ssid, rso_cfg->owe_info.ssid.ssid,
		     rso_cfg->owe_info.ssid.length);
}

static void cm_update_score_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_mlme_psoc_ext_obj *mlme_obj,
				   struct scoring_param *req_score_params,
				   struct rso_config *rso_cfg)
{
	struct wlan_mlme_roam_scoring_cfg *roam_score_params;
	struct weight_cfg *weight_config;
	struct psoc_mlme_obj *mlme_psoc_obj;
	struct scoring_cfg *score_config;
	struct dual_sta_policy *dual_sta_policy;

	mlme_psoc_obj = wlan_psoc_mlme_get_cmpt_obj(psoc);
	if (!mlme_psoc_obj)
		return;

	dual_sta_policy = &mlme_obj->cfg.gen.dual_sta_policy;

	score_config = &mlme_psoc_obj->psoc_cfg.score_config;
	roam_score_params = &mlme_obj->cfg.roam_scoring;
	weight_config = &score_config->weight_config;

	if (!rso_cfg->cfg_param.enable_scoring_for_roam)
		req_score_params->disable_bitmap =
			WLAN_ROAM_SCORING_DISABLE_ALL;

	req_score_params->rssi_weightage = weight_config->rssi_weightage;
	req_score_params->ht_weightage = weight_config->ht_caps_weightage;
	req_score_params->vht_weightage = weight_config->vht_caps_weightage;
	req_score_params->he_weightage = weight_config->he_caps_weightage;
	req_score_params->bw_weightage = weight_config->chan_width_weightage;
	req_score_params->band_weightage = weight_config->chan_band_weightage;
	req_score_params->nss_weightage = weight_config->nss_weightage;
	req_score_params->security_weightage =
					weight_config->security_weightage;
	req_score_params->esp_qbss_weightage =
		weight_config->channel_congestion_weightage;
	req_score_params->beamforming_weightage =
		weight_config->beamforming_cap_weightage;

	/*
	 * Don’t consider pcl weightage for STA connection,
	 * if primary interface is configured.
	 */
	if (policy_mgr_is_pcl_weightage_required(psoc))
		req_score_params->pcl_weightage = weight_config->pcl_weightage;

	req_score_params->oce_wan_weightage = weight_config->oce_wan_weightage;
	req_score_params->oce_ap_tx_pwr_weightage =
		weight_config->oce_ap_tx_pwr_weightage;
	req_score_params->oce_subnet_id_weightage =
		weight_config->oce_subnet_id_weightage;
	req_score_params->sae_pk_ap_weightage =
		weight_config->sae_pk_ap_weightage;

	cm_update_mlo_score_params(req_score_params, weight_config);

	/* TODO: update scoring params corresponding to ML scoring */
	req_score_params->bw_index_score =
		score_config->bandwidth_weight_per_index[0];
	req_score_params->band_index_score =
		score_config->band_weight_per_index;
	req_score_params->nss_index_score =
		score_config->nss_weight_per_index[0];
	req_score_params->security_index_score =
		score_config->security_weight_per_index;
	req_score_params->vendor_roam_score_algorithm =
			score_config->vendor_roam_score_algorithm;

	req_score_params->roam_score_delta =
				roam_score_params->roam_score_delta;
	req_score_params->roam_trigger_bitmap =
				roam_score_params->roam_trigger_bitmap;

	qdf_mem_copy(&req_score_params->rssi_scoring, &score_config->rssi_score,
		     sizeof(struct rssi_config_score));
	qdf_mem_copy(&req_score_params->esp_qbss_scoring,
		     &score_config->esp_qbss_scoring,
		     sizeof(struct per_slot_score));
	qdf_mem_copy(&req_score_params->oce_wan_scoring,
		     &score_config->oce_wan_scoring,
		     sizeof(struct per_slot_score));
	req_score_params->cand_min_roam_score_delta =
					roam_score_params->min_roam_score_delta;
}

static uint32_t cm_crpto_cipher_wmi_cipher(int32_t cipherset)
{

	if (!cipherset || cipherset < 0)
		return WMI_CIPHER_NONE;

	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GCM) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GCM_256))
		return WMI_CIPHER_AES_GCM;
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CCM) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_OCB) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CCM_256))
		return WMI_CIPHER_AES_CCM;
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_TKIP))
		return WMI_CIPHER_TKIP;
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CMAC) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CMAC_256))
		return WMI_CIPHER_AES_CMAC;
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WAPI_GCM4) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WAPI_SMS4))
		return WMI_CIPHER_WAPI;
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GMAC) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GMAC_256))
		return WMI_CIPHER_AES_GMAC;
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP_40) ||
	    QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP_104))
		return WMI_CIPHER_WEP;

	return WMI_CIPHER_NONE;
}

static uint32_t cm_get_rsn_wmi_auth_type(int32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384))
		return WMI_AUTH_FT_RSNA_FILS_SHA384;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256))
		return WMI_AUTH_FT_RSNA_FILS_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA384))
		return WMI_AUTH_RSNA_FILS_SHA384;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA256))
		return WMI_AUTH_RSNA_FILS_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE))
		return WMI_AUTH_FT_RSNA_SAE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE))
		return WMI_AUTH_WPA3_SAE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OWE))
		return WMI_AUTH_WPA3_OWE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X))
		return WMI_AUTH_FT_RSNA;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_PSK))
		return WMI_AUTH_FT_RSNA_PSK;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		return WMI_AUTH_RSNA;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK))
		return WMI_AUTH_RSNA_PSK;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		return WMI_AUTH_CCKM_RSNA;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK_SHA256))
		return WMI_AUTH_RSNA_PSK_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256))
		return WMI_AUTH_RSNA_8021X_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B))
		return WMI_AUTH_RSNA_SUITE_B_8021X_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192))
		return WMI_AUTH_RSNA_SUITE_B_8021X_SHA384;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384))
		return WMI_AUTH_FT_RSNA_SUITE_B_8021X_SHA384;
	else
		return WMI_AUTH_NONE;
}

static uint32_t cm_get_wpa_wmi_auth_type(int32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		return WMI_AUTH_WPA;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK))
		return WMI_AUTH_WPA_PSK;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		return WMI_AUTH_CCKM_WPA;
	else
		return WMI_AUTH_NONE;
}

static uint32_t cm_get_wapi_wmi_auth_type(int32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_CERT))
		return WMI_AUTH_WAPI;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_PSK))
		return WMI_AUTH_WAPI_PSK;
	else
		return WMI_AUTH_NONE;
}

uint32_t cm_crypto_authmode_to_wmi_authmode(int32_t authmodeset,
					    int32_t akm, int32_t ucastcipherset)
{
	if (!authmodeset || authmodeset < 0)
		return WMI_AUTH_OPEN;

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_OPEN))
		return WMI_AUTH_OPEN;

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_AUTO) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_NONE)) {
		if ((QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP) ||
		     QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_40) ||
		     QDF_HAS_PARAM(ucastcipherset,
				   WLAN_CRYPTO_CIPHER_WEP_104) ||
		     QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_TKIP) ||
		     QDF_HAS_PARAM(ucastcipherset,
				   WLAN_CRYPTO_CIPHER_AES_GCM) ||
		     QDF_HAS_PARAM(ucastcipherset,
				   WLAN_CRYPTO_CIPHER_AES_GCM_256) ||
		     QDF_HAS_PARAM(ucastcipherset,
				   WLAN_CRYPTO_CIPHER_AES_CCM) ||
		     QDF_HAS_PARAM(ucastcipherset,
				   WLAN_CRYPTO_CIPHER_AES_OCB) ||
		     QDF_HAS_PARAM(ucastcipherset,
				   WLAN_CRYPTO_CIPHER_AES_CCM_256)))
			return WMI_AUTH_OPEN;
		else
			return WMI_AUTH_NONE;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_SHARED))
		return WMI_AUTH_SHARED;

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_8021X) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_RSNA) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_CCKM) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_SAE) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_FILS_SK))
		return cm_get_rsn_wmi_auth_type(akm);


	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_WPA))
		return cm_get_wpa_wmi_auth_type(akm);

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_WAPI))
		return cm_get_wapi_wmi_auth_type(akm);

	return WMI_AUTH_OPEN;
}

static void cm_update_crypto_params(struct wlan_objmgr_vdev *vdev,
				    struct ap_profile *profile)
{
	int32_t keymgmt, connected_akm, authmode, uccipher, mccipher;
	enum wlan_crypto_key_mgmt i;
	int32_t num_allowed_authmode = 0;
	struct rso_config *rso_cfg;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return;

	/* Pairwise cipher suite */
	uccipher = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	profile->rsn_ucastcipherset = cm_crpto_cipher_wmi_cipher(uccipher);

	/* Group cipher suite */
	mccipher = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MCAST_CIPHER);
	profile->rsn_mcastcipherset = cm_crpto_cipher_wmi_cipher(mccipher);

	/* Group management cipher suite */
	cm_roam_fill_11w_params(vdev, profile);

	authmode = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_AUTH_MODE);
	/* Get connected akm */
	connected_akm = wlan_crypto_get_param(vdev,
					WLAN_CRYPTO_PARAM_KEY_MGMT);
	profile->rsn_authmode =
			cm_crypto_authmode_to_wmi_authmode(authmode,
							   connected_akm,
							   uccipher);
	/* Get keymgmt from self security info */
	keymgmt = rso_cfg->orig_sec_info.key_mgmt;

	for (i = 0; i < WLAN_CRYPTO_KEY_MGMT_MAX; i++) {
		/*
		 * Send AKM in allowed list which are not present in connected
		 * akm
		 */
		if (QDF_HAS_PARAM(keymgmt, i) &&
		    num_allowed_authmode < WLAN_CRYPTO_AUTH_MAX) {
			profile->allowed_authmode[num_allowed_authmode++] =
			cm_crypto_authmode_to_wmi_authmode(authmode,
							   (keymgmt & (1 << i)),
							   uccipher);
		}
	}

	profile->num_allowed_authmode = num_allowed_authmode;
}

/**
 * cm_roam_scan_offload_ap_profile() - set roam ap profile parameters
 * @psoc: psoc ctx
 * @vdev: vdev id
 * @rso_cfg: rso config
 * @params:  roam ap profile related parameters
 *
 * This function is used to set roam ap profile related parameters
 *
 * Return: None
 */
static void
cm_roam_scan_offload_ap_profile(struct wlan_objmgr_psoc *psoc,
				struct wlan_objmgr_vdev *vdev,
				struct rso_config *rso_cfg,
				struct ap_profile_params *params)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct ap_profile *profile = &params->profile;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	params->vdev_id = vdev_id;
	wlan_vdev_mlme_get_ssid(vdev, profile->ssid.ssid,
				&profile->ssid.length);

	cm_update_crypto_params(vdev, profile);

	profile->rssi_threshold = rso_cfg->cfg_param.roam_rssi_diff;
	profile->bg_rssi_threshold = rso_cfg->cfg_param.bg_rssi_threshold;
	/*
	 * rssi_diff which is updated via framework is equivalent to the
	 * INI RoamRssiDiff parameter and hence should be updated.
	 */
	if (mlme_obj->cfg.lfr.rso_user_config.rssi_diff)
		profile->rssi_threshold =
				mlme_obj->cfg.lfr.rso_user_config.rssi_diff;

	profile->rssi_abs_thresh =
			mlme_obj->cfg.lfr.roam_rssi_abs_threshold;

	if (rso_cfg->owe_info.is_owe_transition_conn)
		cm_update_owe_ap_profile(params, rso_cfg);

	cm_update_score_params(psoc, mlme_obj, &params->param, rso_cfg);

	params->min_rssi_params[DEAUTH_MIN_RSSI] =
			mlme_obj->cfg.trig_min_rssi[DEAUTH_MIN_RSSI];
	params->min_rssi_params[BMISS_MIN_RSSI] =
			mlme_obj->cfg.trig_min_rssi[BMISS_MIN_RSSI];
	params->min_rssi_params[MIN_RSSI_2G_TO_5G_ROAM] =
			mlme_obj->cfg.trig_min_rssi[MIN_RSSI_2G_TO_5G_ROAM];

	params->score_delta_param[IDLE_ROAM_TRIGGER] =
			mlme_obj->cfg.trig_score_delta[IDLE_ROAM_TRIGGER];
	params->score_delta_param[BTM_ROAM_TRIGGER] =
			mlme_obj->cfg.trig_score_delta[BTM_ROAM_TRIGGER];
}

static bool
cm_check_band_freq_match(enum band_info band, qdf_freq_t freq)
{
	if (band == BAND_ALL)
		return true;

	if (band == BAND_2G && WLAN_REG_IS_24GHZ_CH_FREQ(freq))
		return true;

	if (band == BAND_5G && WLAN_REG_IS_5GHZ_CH_FREQ(freq))
		return true;

	/*
	 * Not adding the band check for now as band_info will be soon
	 * replaced with reg_wifi_band enum
	 */
	if (WLAN_REG_IS_6GHZ_CHAN_FREQ(freq))
		return true;

	return false;
}

/**
 * cm_is_dfs_unsafe_extra_band_chan() - check if dfs unsafe or extra band
 * channel
 * @vdev: vdev
 * @mlme_obj: psoc mlme obj
 * @freq: channel freq to check
 * @band: band for intra band check
.*.
 * Return: bool if match else false
 */
static bool
cm_is_dfs_unsafe_extra_band_chan(struct wlan_objmgr_vdev *vdev,
				 struct wlan_mlme_psoc_ext_obj *mlme_obj,
				 qdf_freq_t freq,
				 enum band_info band)
{
	uint16_t  unsafe_chan[NUM_CHANNELS];
	uint16_t  unsafe_chan_cnt = 0;
	uint16_t  cnt = 0;
	bool is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	struct rso_roam_policy_params *roam_policy;
	struct wlan_objmgr_pdev *pdev;

	if (!qdf_ctx)
		return true;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return true;

	roam_policy = &mlme_obj->cfg.lfr.rso_user_config.policy_params;
	if ((mlme_obj->cfg.lfr.roaming_dfs_channel ==
	     ROAMING_DFS_CHANNEL_DISABLED ||
	     roam_policy->dfs_mode == STA_ROAM_POLICY_DFS_DISABLED) &&
	    (wlan_reg_is_dfs_for_freq(pdev, freq)))
		return true;

	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
				    &unsafe_chan_cnt,
				    sizeof(unsafe_chan));
	if (roam_policy->skip_unsafe_channels && unsafe_chan_cnt) {
		is_unsafe_chan = false;
		for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
			if (unsafe_chan[cnt] == freq) {
				is_unsafe_chan = true;
				mlme_debug("ignoring unsafe channel freq %d",
					   freq);
				return true;
			}
		}
	}
	if (!cm_check_band_freq_match(band, freq)) {
		mlme_debug("ignoring non-intra band freq %d", freq);
		return true;
	}

	return false;
}

/**
 * cm_populate_roam_chan_list() - Populate roam channel list
 * parameters
 * @vdev: vdev
 * @mlme_obj:  mlme_obj
 * @dst: Destination roam channel buf to populate the roam chan list
 * @src: Source channel list
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
cm_populate_roam_chan_list(struct wlan_objmgr_vdev *vdev,
			   struct wlan_mlme_psoc_ext_obj *mlme_obj,
			   struct wlan_roam_scan_channel_list *dst,
			   struct rso_chan_info *src)
{
	enum band_info band;
	uint32_t band_cap;
	uint8_t i = 0;
	uint8_t num_channels = 0;
	qdf_freq_t *freq_lst = src->freq_list;

	/*
	 * The INI channels need to be filtered with respect to the current band
	 * that is supported.
	 */
	band_cap = mlme_obj->cfg.gen.band_capability;
	if (!band_cap) {
		mlme_err("Invalid band_cap(%d), roam scan offload req aborted",
			 band_cap);
		return QDF_STATUS_E_FAILURE;
	}

	band = wlan_reg_band_bitmap_to_band_info(band_cap);
	num_channels = dst->chan_count;
	for (i = 0; i < src->num_chan; i++) {
		if (wlan_is_channel_present_in_list(dst->chan_freq_list,
						    num_channels, *freq_lst)) {
			freq_lst++;
			continue;
		}
		if (cm_is_dfs_unsafe_extra_band_chan(vdev, mlme_obj,
						     *freq_lst, band)) {
			freq_lst++;
			continue;
		}
		dst->chan_freq_list[num_channels++] = *freq_lst;
		freq_lst++;
	}
	dst->chan_count = num_channels;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
cm_fetch_ch_lst_from_ini(struct wlan_objmgr_vdev *vdev,
			 struct wlan_mlme_psoc_ext_obj *mlme_obj,
			 struct wlan_roam_scan_channel_list *chan_info,
			 struct rso_chan_info *specific_chan_info)
{
	QDF_STATUS status;

	status = cm_populate_roam_chan_list(vdev, mlme_obj, chan_info,
					    specific_chan_info);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to copy channels to roam list");
		return status;
	}
	chan_info->chan_cache_type = CHANNEL_LIST_STATIC;

	return QDF_STATUS_SUCCESS;
}

static void
cm_fetch_ch_lst_from_occupied_lst(struct wlan_objmgr_vdev *vdev,
				  struct wlan_mlme_psoc_ext_obj *mlme_obj,
				  struct rso_config *rso_cfg,
				  struct wlan_roam_scan_channel_list *chan_info,
				  uint8_t reason)
{
	uint8_t i = 0;
	uint8_t num_channels = 0;
	qdf_freq_t op_freq;
	enum band_info band = BAND_ALL;
	struct wlan_chan_list *occupied_channels;

	occupied_channels = &rso_cfg->occupied_chan_lst;
	op_freq = wlan_get_operation_chan_freq(vdev);
	if (mlme_obj->cfg.lfr.roam_intra_band) {
		if (WLAN_REG_IS_5GHZ_CH_FREQ(op_freq))
			band = BAND_5G;
		else if (WLAN_REG_IS_24GHZ_CH_FREQ(op_freq))
			band = BAND_2G;
		else
			band = BAND_UNKNOWN;
	}

	for (i = 0; i < occupied_channels->num_chan &&
	     occupied_channels->num_chan < CFG_VALID_CHANNEL_LIST_LEN; i++) {
		if (cm_is_dfs_unsafe_extra_band_chan(vdev, mlme_obj,
				occupied_channels->freq_list[i], band))
			continue;

		chan_info->chan_freq_list[num_channels++] =
					occupied_channels->freq_list[i];
	}
	chan_info->chan_count = num_channels;
	chan_info->chan_cache_type = CHANNEL_LIST_DYNAMIC;
}

/**
 * cm_add_ch_lst_from_roam_scan_list() - channel from roam scan chan list
 * parameters
 * @vdev: vdev
 * @mlme_obj:  mlme_obj
 * @chan_info: RSO channel info
 * @rso_cfg: rso config
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_add_ch_lst_from_roam_scan_list(struct wlan_objmgr_vdev *vdev,
				  struct wlan_mlme_psoc_ext_obj *mlme_obj,
				  struct wlan_roam_scan_channel_list *chan_info,
				  struct rso_config *rso_cfg)
{
	QDF_STATUS status;
	struct rso_chan_info *pref_chan_info =
			&rso_cfg->cfg_param.pref_chan_info;

	if (!pref_chan_info->num_chan)
		return QDF_STATUS_SUCCESS;

	status = cm_populate_roam_chan_list(vdev, mlme_obj, chan_info,
					    pref_chan_info);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to copy channels to roam list");
		return status;
	}
	cm_dump_freq_list(pref_chan_info);
	chan_info->chan_cache_type = CHANNEL_LIST_DYNAMIC;

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_ESE
static void
cm_fetch_ch_lst_from_received_list(struct wlan_objmgr_vdev *vdev,
				  struct wlan_mlme_psoc_ext_obj *mlme_obj,
				  struct rso_config *rso_cfg,
				  struct wlan_roam_scan_channel_list *chan_info)
{
	uint8_t i = 0;
	uint8_t num_channels = 0;
	qdf_freq_t *freq_lst = NULL;
	enum band_info band = BAND_ALL;
	struct rso_chan_info *curr_ch_lst_info = &rso_cfg->roam_scan_freq_lst;

	if (curr_ch_lst_info->num_chan == 0)
		return;

	freq_lst = curr_ch_lst_info->freq_list;
	for (i = 0; i < curr_ch_lst_info->num_chan; i++) {
		if (cm_is_dfs_unsafe_extra_band_chan(vdev, mlme_obj,
						     freq_lst[i], band))
			continue;

		chan_info->chan_freq_list[num_channels++] = freq_lst[i];
	}
	chan_info->chan_count = num_channels;
	chan_info->chan_cache_type = CHANNEL_LIST_DYNAMIC;
}

static inline bool cm_is_ese_assoc(struct rso_config *rso_cfg)
{
	return rso_cfg->is_ese_assoc;
}
static void cm_esr_populate_version_ie(
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			struct wlan_roam_scan_offload_params *rso_mode_cfg)
{
	static const uint8_t ese_ie[] = {0x0, 0x40, 0x96, 0x3,
					 ESE_VERSION_SUPPORTED};

	/* Append ESE version IE if isEseIniFeatureEnabled INI is enabled */
	if (mlme_obj->cfg.lfr.ese_enabled)
		wlan_cm_append_assoc_ies(rso_mode_cfg, WLAN_ELEMID_VENDOR,
					 sizeof(ese_ie), ese_ie);
}

#else
static inline void
cm_fetch_ch_lst_from_received_list(struct wlan_objmgr_vdev *vdev,
				  struct wlan_mlme_psoc_ext_obj *mlme_obj,
				  struct rso_config *rso_cfg,
				  struct wlan_roam_scan_channel_list *chan_info)
{}
static inline bool cm_is_ese_assoc(struct rso_config *rso_cfg)
{
	return false;
}
static inline void cm_esr_populate_version_ie(
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			struct wlan_roam_scan_offload_params *rso_mode_cfg)
{}
#endif

/**
 * cm_fetch_valid_ch_lst() - fetch channel list from valid channel list and
 * update rso req msg parameters
 * @vdev: vdev
 * @mlme_obj:  mlme_obj
 * @chan_info: channel info
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_fetch_valid_ch_lst(struct wlan_objmgr_vdev *vdev,
		      struct wlan_mlme_psoc_ext_obj *mlme_obj,
		      struct wlan_roam_scan_channel_list *chan_info)
{
	qdf_freq_t *ch_freq_list = NULL;
	uint8_t i = 0, num_channels = 0;
	enum band_info band = BAND_ALL;
	qdf_freq_t op_freq;

	op_freq = wlan_get_operation_chan_freq(vdev);
	if (mlme_obj->cfg.lfr.roam_intra_band) {
		if (WLAN_REG_IS_5GHZ_CH_FREQ(op_freq))
			band = BAND_5G;
		else if (WLAN_REG_IS_24GHZ_CH_FREQ(op_freq))
			band = BAND_2G;
		else
			band = BAND_UNKNOWN;
	}

	ch_freq_list = mlme_obj->cfg.reg.valid_channel_freq_list;
	for (i = 0; i < mlme_obj->cfg.reg.valid_channel_list_num; i++) {
		if (wlan_reg_is_dsrc_freq(ch_freq_list[i]))
			continue;
		if (cm_is_dfs_unsafe_extra_band_chan(vdev, mlme_obj,
						     ch_freq_list[i],
						     band))
			continue;
		chan_info->chan_freq_list[num_channels++] = ch_freq_list[i];
	}
	chan_info->chan_count = num_channels;
	chan_info->chan_cache_type = CHANNEL_LIST_DYNAMIC;

	return QDF_STATUS_SUCCESS;
}

static void
cm_fill_rso_channel_list(struct wlan_objmgr_psoc *psoc,
			 struct wlan_objmgr_vdev *vdev,
			 struct rso_config *rso_cfg,
			 struct wlan_roam_scan_channel_list *chan_info,
			 uint8_t reason)
{
	QDF_STATUS status;
	uint8_t ch_cache_str[128] = {0};
	uint8_t i, j;
	struct rso_chan_info *specific_chan_info;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	specific_chan_info = &rso_cfg->cfg_param.specific_chan_info;
	chan_info->vdev_id = vdev_id;
	if (cm_is_ese_assoc(rso_cfg) ||
	    rso_cfg->roam_scan_freq_lst.num_chan == 0) {
		/*
		 * Retrieve the Channel Cache either from ini or from
		 * the occupied channels list along with preferred
		 * channel list configured by the client.
		 * Give Preference to INI Channels
		 */
		if (specific_chan_info->num_chan) {
			status = cm_fetch_ch_lst_from_ini(vdev, mlme_obj,
							  chan_info,
							  specific_chan_info);
			if (QDF_IS_STATUS_ERROR(status)) {
				mlme_err("Fetch channel list from ini failed");
				return;
			}
		} else if (reason == REASON_FLUSH_CHANNEL_LIST) {
			chan_info->chan_cache_type = CHANNEL_LIST_STATIC;
			chan_info->chan_count = 0;
		} else {
			cm_fetch_ch_lst_from_occupied_lst(vdev, mlme_obj,
							  rso_cfg,
							  chan_info, reason);
			/* Add the preferred channel list configured by
			 * client to the roam channel list along with
			 * occupied channel list.
			 */
			cm_add_ch_lst_from_roam_scan_list(vdev, mlme_obj,
							  chan_info, rso_cfg);
		}
	} else {
		/*
		 * If ESE is enabled, and a neighbor Report is received,
		 * then Ignore the INI Channels or the Occupied Channel
		 * List. Consider the channels in the neighbor list sent
		 * by the ESE AP
		 */
		cm_fetch_ch_lst_from_received_list(vdev, mlme_obj, rso_cfg,
						   chan_info);
	}

	if (!chan_info->chan_count &&
	    reason != REASON_FLUSH_CHANNEL_LIST) {
		/* Maintain the Valid Channels List */
		status = cm_fetch_valid_ch_lst(vdev, mlme_obj, chan_info);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err("Fetch channel list fail");
			return;
		}
	}

	for (i = 0, j = 0; i < chan_info->chan_count; i++) {
		if (j < sizeof(ch_cache_str))
			j += snprintf(ch_cache_str + j,
				      sizeof(ch_cache_str) - j, " %d",
				      chan_info->chan_freq_list[i]);
		else
			break;
	}

	mlme_debug("chan_cache_type:%d, No of chan:%d, chan: %s",
		   chan_info->chan_cache_type,
		   chan_info->chan_count, ch_cache_str);
}

static void
cm_add_blacklist_ap_list(struct wlan_objmgr_pdev *pdev,
			 struct roam_scan_filter_params *params)
{
	int i = 0;
	struct reject_ap_config_params *reject_list;

	reject_list = qdf_mem_malloc(sizeof(*reject_list) *
				     MAX_RSSI_AVOID_BSSID_LIST);
	if (!reject_list)
		return;

	params->num_bssid_black_list =
		wlan_blm_get_bssid_reject_list(pdev, reject_list,
					       MAX_RSSI_AVOID_BSSID_LIST,
					       USERSPACE_BLACKLIST_TYPE);
	if (!params->num_bssid_black_list) {
		qdf_mem_free(reject_list);
		return;
	}

	for (i = 0; i < params->num_bssid_black_list; i++) {
		qdf_copy_macaddr(&params->bssid_avoid_list[i],
				 &reject_list[i].bssid);
		mlme_debug("Blacklist bssid[%d]:" QDF_MAC_ADDR_FMT, i,
			   QDF_MAC_ADDR_REF(params->bssid_avoid_list[i].bytes));
	}

	qdf_mem_free(reject_list);
}

/**
 * cm_roam_scan_filter() - set roam scan filter parameters
 * @psoc: psoc
 * @pdev: pdev
 * @vdev_id: vdev id
 * @command: rso command
 * @reason:  reason to roam
 * @scan_filter_params:  roam scan filter related parameters
 *
 * There are filters such as whitelist, blacklist and preferred
 * list that need to be applied to the scan results to form the
 * probable candidates for roaming.
 *
 * Return: None
 */
static void
cm_roam_scan_filter(struct wlan_objmgr_psoc *psoc,
		    struct wlan_objmgr_pdev *pdev,
		    uint8_t vdev_id, uint8_t command, uint8_t reason,
		    struct wlan_roam_scan_filter_params *scan_filter_params)
{
	int i;
	uint32_t num_ssid_white_list = 0, num_bssid_preferred_list = 0;
	uint32_t op_bitmap = 0;
	struct roam_scan_filter_params *params;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct rso_config_params *rso_usr_cfg;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	scan_filter_params->reason = reason;
	params = &scan_filter_params->filter_params;
	rso_usr_cfg = &mlme_obj->cfg.lfr.rso_user_config;
	if (command != ROAM_SCAN_OFFLOAD_STOP) {
		switch (reason) {
		case REASON_ROAM_SET_BLACKLIST_BSSID:
			op_bitmap |= ROAM_FILTER_OP_BITMAP_BLACK_LIST;
			cm_add_blacklist_ap_list(pdev, params);
			break;
		case REASON_ROAM_SET_SSID_ALLOWED:
			op_bitmap |= ROAM_FILTER_OP_BITMAP_WHITE_LIST;
			num_ssid_white_list =
				rso_usr_cfg->num_ssid_allowed_list;
			break;
		case REASON_ROAM_SET_FAVORED_BSSID:
			op_bitmap |= ROAM_FILTER_OP_BITMAP_PREFER_BSSID;
			num_bssid_preferred_list =
				rso_usr_cfg->num_bssid_favored;
			break;
		case REASON_CTX_INIT:
			if (command == ROAM_SCAN_OFFLOAD_START) {
				num_ssid_white_list =
					rso_usr_cfg->num_ssid_allowed_list;
				if (num_ssid_white_list)
					op_bitmap |=
					ROAM_FILTER_OP_BITMAP_WHITE_LIST;
				cm_add_blacklist_ap_list(pdev, params);
				if (params->num_bssid_black_list)
					op_bitmap |=
					ROAM_FILTER_OP_BITMAP_BLACK_LIST;

				params->lca_disallow_config_present = true;
				/*
				 * If rssi disallow bssid list have any member
				 * fill it and send it to firmware so that
				 * firmware does not try to roam to these BSS
				 * until RSSI OR time condition are matched.
				 */
				params->num_rssi_rejection_ap =
					wlan_blm_get_bssid_reject_list(pdev,
						params->rssi_rejection_ap,
						MAX_RSSI_AVOID_BSSID_LIST,
						DRIVER_RSSI_REJECT_TYPE);
			} else {
				mlme_debug("Roam Filter need not be sent, no need to fill parameters");
				return;
			}
			break;
		default:
			if (command == ROAM_SCAN_OFFLOAD_START) {
				num_ssid_white_list =
					rso_usr_cfg->num_ssid_allowed_list;
				if (num_ssid_white_list)
					op_bitmap |=
					ROAM_FILTER_OP_BITMAP_WHITE_LIST;
				cm_add_blacklist_ap_list(pdev, params);
				if (params->num_bssid_black_list)
					op_bitmap |=
					ROAM_FILTER_OP_BITMAP_BLACK_LIST;
			}
			if (!op_bitmap) {
				mlme_debug("Roam Filter need not be sent, no need to fill parameters");
				return;
			}
			break;
		}
	} else {
		/* In case of STOP command, reset all the variables
		 * except for blacklist BSSID which should be retained
		 * across connections.
		 */
		op_bitmap = ROAM_FILTER_OP_BITMAP_WHITE_LIST |
			    ROAM_FILTER_OP_BITMAP_PREFER_BSSID;
		if (reason == REASON_ROAM_SET_SSID_ALLOWED)
			num_ssid_white_list =
					rso_usr_cfg->num_ssid_allowed_list;
		num_bssid_preferred_list = rso_usr_cfg->num_bssid_favored;
	}

	/* fill in fixed values */
	params->vdev_id = vdev_id;
	params->op_bitmap = op_bitmap;
	params->num_ssid_white_list = num_ssid_white_list;
	params->num_bssid_preferred_list = num_bssid_preferred_list;
	params->delta_rssi =
		wlan_blm_get_rssi_blacklist_threshold(pdev);

	for (i = 0; i < num_ssid_white_list; i++) {
		qdf_mem_copy(params->ssid_allowed_list[i].ssid,
			     rso_usr_cfg->ssid_allowed_list[i].ssid,
			     rso_usr_cfg->ssid_allowed_list[i].length);
		params->ssid_allowed_list[i].length =
				rso_usr_cfg->ssid_allowed_list[i].length;
		mlme_debug("SSID %d: %.*s", i,
			   params->ssid_allowed_list[i].length,
			   params->ssid_allowed_list[i].ssid);
	}

	if (params->num_bssid_preferred_list) {
		qdf_mem_copy(params->bssid_favored, rso_usr_cfg->bssid_favored,
			     MAX_BSSID_FAVORED * sizeof(struct qdf_mac_addr));
		qdf_mem_copy(params->bssid_favored_factor,
			     rso_usr_cfg->bssid_favored_factor,
			     MAX_BSSID_FAVORED);
	}
	for (i = 0; i < params->num_rssi_rejection_ap; i++)
		mlme_debug("RSSI reject BSSID "QDF_MAC_ADDR_FMT" expected rssi %d remaining duration %d",
			   QDF_MAC_ADDR_REF(params->rssi_rejection_ap[i].bssid.bytes),
			   params->rssi_rejection_ap[i].expected_rssi,
			   params->rssi_rejection_ap[i].reject_duration);

	for (i = 0; i < params->num_bssid_preferred_list; i++)
		mlme_debug("Preferred Bssid[%d]:"QDF_MAC_ADDR_FMT" score: %d", i,
			   QDF_MAC_ADDR_REF(params->bssid_favored[i].bytes),
			   params->bssid_favored_factor[i]);

	if (params->lca_disallow_config_present) {
		params->disallow_duration
				= mlme_obj->cfg.lfr.lfr3_disallow_duration;
		params->rssi_channel_penalization
			= mlme_obj->cfg.lfr.lfr3_rssi_channel_penalization;
		params->num_disallowed_aps
			= mlme_obj->cfg.lfr.lfr3_num_disallowed_aps;
		mlme_debug("disallow_dur %d rssi_chan_pen %d num_disallowed_aps %d",
			   params->disallow_duration,
			   params->rssi_channel_penalization,
			   params->num_disallowed_aps);
	}
}

static void
cm_roam_scan_offload_fill_scan_params(struct wlan_objmgr_psoc *psoc,
			struct rso_config *rso_cfg,
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			struct wlan_roam_scan_offload_params *rso_mode_cfg,
			struct wlan_roam_scan_channel_list *rso_chan_info,
			uint8_t command)
{
	struct wlan_roam_scan_params *scan_params =
			&rso_mode_cfg->rso_scan_params;
	uint8_t channels_per_burst = 0;
	uint16_t roam_scan_home_away_time;
	enum roaming_dfs_channel_type allow_dfs_ch_roam;
	struct rso_cfg_params *cfg_params;

	qdf_mem_zero(scan_params, sizeof(*scan_params));
	if (command == ROAM_SCAN_OFFLOAD_STOP)
		return;

	cfg_params = &rso_cfg->cfg_param;

	/* Parameters updated after association is complete */
	wlan_scan_cfg_get_passive_dwelltime(psoc,
					    &scan_params->dwell_time_passive);
	wlan_scan_cfg_get_min_dwelltime_6g(psoc,
					   &scan_params->min_dwell_time_6ghz);
	/*
	 * Here is the formula,
	 * T(HomeAway) = N * T(dwell) + (N+1) * T(cs)
	 * where N is number of channels scanned in single burst
	 */
	scan_params->dwell_time_active = cfg_params->max_chan_scan_time;

	roam_scan_home_away_time = cfg_params->roam_scan_home_away_time;
	if (roam_scan_home_away_time <
	    (scan_params->dwell_time_active +
	     (2 * ROAM_SCAN_CHANNEL_SWITCH_TIME))) {
		mlme_debug("Disable Home away time(%d) as it is less than (2*RF switching time + channel max time)(%d)",
			  roam_scan_home_away_time,
			  (scan_params->dwell_time_active +
			   (2 * ROAM_SCAN_CHANNEL_SWITCH_TIME)));
		roam_scan_home_away_time = 0;
	}

	if (roam_scan_home_away_time < (2 * ROAM_SCAN_CHANNEL_SWITCH_TIME)) {
		/* clearly we can't follow home away time.
		 * Make it a split scan.
		 */
		scan_params->burst_duration = 0;
	} else {
		channels_per_burst =
		  (roam_scan_home_away_time - ROAM_SCAN_CHANNEL_SWITCH_TIME) /
		  (scan_params->dwell_time_active + ROAM_SCAN_CHANNEL_SWITCH_TIME);

		if (channels_per_burst < 1) {
			/* dwell time and home away time conflicts */
			/* we will override dwell time */
			scan_params->dwell_time_active =
				roam_scan_home_away_time -
				(2 * ROAM_SCAN_CHANNEL_SWITCH_TIME);
			scan_params->burst_duration =
				scan_params->dwell_time_active;
		} else {
			scan_params->burst_duration =
				channels_per_burst *
				scan_params->dwell_time_active;
		}
	}

	allow_dfs_ch_roam = mlme_obj->cfg.lfr.roaming_dfs_channel;
	/* Roaming on DFS channels is supported and it is not
	 * app channel list. It is ok to override homeAwayTime
	 * to accommodate DFS dwell time in burst
	 * duration.
	 */
	if (allow_dfs_ch_roam == ROAMING_DFS_CHANNEL_ENABLED_NORMAL &&
	    roam_scan_home_away_time > 0  &&
	    rso_chan_info->chan_cache_type != CHANNEL_LIST_STATIC)
		scan_params->burst_duration =
			QDF_MAX(scan_params->burst_duration,
				scan_params->dwell_time_passive);

	scan_params->min_rest_time = cfg_params->neighbor_scan_min_period;
	scan_params->max_rest_time = cfg_params->neighbor_scan_period;
	scan_params->repeat_probe_time =
		(cfg_params->roam_scan_n_probes > 0) ?
			QDF_MAX(scan_params->dwell_time_active /
				cfg_params->roam_scan_n_probes, 1) : 0;
	scan_params->probe_spacing_time = 0;
	scan_params->probe_delay = 0;
	/* 30 seconds for full scan cycle */
	scan_params->max_scan_time = ROAM_SCAN_HW_DEF_SCAN_MAX_DURATION;
	scan_params->idle_time = scan_params->min_rest_time;
	scan_params->n_probes = cfg_params->roam_scan_n_probes;

	if (allow_dfs_ch_roam == ROAMING_DFS_CHANNEL_DISABLED) {
		scan_params->scan_ctrl_flags |= WMI_SCAN_BYPASS_DFS_CHN;
	} else {
		/* Roaming scan on DFS channel is allowed.
		 * No need to change any flags for default
		 * allowDFSChannelRoam = 1.
		 * Special case where static channel list is given by\
		 * application that contains DFS channels.
		 * Assume that the application has knowledge of matching
		 * APs being active and that probe request transmission
		 * is permitted on those channel.
		 * Force active scans on those channels.
		 */

		if (allow_dfs_ch_roam ==
		    ROAMING_DFS_CHANNEL_ENABLED_ACTIVE &&
		    rso_chan_info->chan_cache_type == CHANNEL_LIST_STATIC &&
		    rso_chan_info->chan_count)
			scan_params->scan_ctrl_flags |=
				WMI_SCAN_FLAG_FORCE_ACTIVE_ON_DFS;
	}

	scan_params->rso_adaptive_dwell_mode =
		mlme_obj->cfg.lfr.adaptive_roamscan_dwell_mode;
}

void wlan_cm_append_assoc_ies(struct wlan_roam_scan_offload_params *rso_mode_cfg,
			      uint8_t ie_id, uint8_t ie_len,
			      const uint8_t *ie_data)
{
	uint32_t curr_length = rso_mode_cfg->assoc_ie_length;

	if ((MAC_MAX_ADD_IE_LENGTH - curr_length) < ie_len) {
		mlme_err("Appending IE id: %d failed", ie_id);
		return;
	}

	rso_mode_cfg->assoc_ie[curr_length] = ie_id;
	rso_mode_cfg->assoc_ie[curr_length + 1] = ie_len;
	qdf_mem_copy(&rso_mode_cfg->assoc_ie[curr_length + 2], ie_data, ie_len);
	rso_mode_cfg->assoc_ie_length += (ie_len + 2);
}

void wlan_add_supported_5Ghz_channels(struct wlan_objmgr_psoc *psoc,
				      struct wlan_objmgr_pdev *pdev,
				      uint8_t *chan_list,
				      uint8_t *num_chnl,
				      bool supp_chan_ie)
{
	uint16_t i, j = 0;
	uint32_t size = 0;
	uint32_t *freq_list;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	if (!chan_list) {
		mlme_err("chan_list buffer NULL");
		*num_chnl = 0;
		return;
	}
	size = mlme_obj->cfg.reg.valid_channel_list_num;
	freq_list = mlme_obj->cfg.reg.valid_channel_freq_list;
	for (i = 0, j = 0; i < size; i++) {
		if (wlan_reg_is_dsrc_freq(freq_list[i]))
			continue;
		/* Only add 5ghz channels.*/
		if (WLAN_REG_IS_5GHZ_CH_FREQ(freq_list[i])) {
			chan_list[j] =
				wlan_reg_freq_to_chan(pdev,
						      freq_list[i]);
				j++;

			if (supp_chan_ie) {
				chan_list[j] = 1;
				j++;
			}
		}
	}
	*num_chnl = (uint8_t)j;
}

static void cm_update_driver_assoc_ies(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_vdev *vdev,
			struct rso_config *rso_cfg,
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			struct wlan_roam_scan_offload_params *rso_mode_cfg)
{
	bool power_caps_populated = false;
	uint8_t *rrm_cap_ie_data;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	uint8_t power_cap_ie_data[DOT11F_IE_POWERCAPS_MAX_LEN] = {
		MIN_TX_PWR_CAP, MAX_TX_PWR_CAP};
	uint8_t max_tx_pwr_cap = 0;
	struct wlan_objmgr_pdev *pdev;
	uint8_t supp_chan_ie[DOT11F_IE_SUPPCHANNELS_MAX_LEN], supp_chan_ie_len;
	static const uint8_t qcn_ie[] = {0x8C, 0xFD, 0xF0, 0x1,
					 QCN_IE_VERSION_SUBATTR_ID,
					 QCN_IE_VERSION_SUBATTR_DATA_LEN,
					 QCN_IE_VERSION_SUPPORTED,
					 QCN_IE_SUBVERSION_SUPPORTED};

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return;

	rrm_cap_ie_data = wlan_cm_get_rrm_cap_ie_data();
	/* Re-Assoc IE TLV parameters */
	rso_mode_cfg->assoc_ie_length = rso_cfg->assoc_ie.len;
	qdf_mem_copy(rso_mode_cfg->assoc_ie, rso_cfg->assoc_ie.ptr,
		     rso_mode_cfg->assoc_ie_length);

	max_tx_pwr_cap = wlan_get_cfg_max_tx_power(psoc, pdev,
					wlan_get_operation_chan_freq(vdev));

	if (max_tx_pwr_cap && max_tx_pwr_cap < MAX_TX_PWR_CAP)
		power_cap_ie_data[1] = max_tx_pwr_cap;
	else
		power_cap_ie_data[1] = MAX_TX_PWR_CAP;

	if (mlme_obj->cfg.gen.enabled_11h) {
		/* Append power cap IE */
		wlan_cm_append_assoc_ies(rso_mode_cfg, WLAN_ELEMID_PWRCAP,
					 DOT11F_IE_POWERCAPS_MAX_LEN,
					 power_cap_ie_data);
		power_caps_populated = true;

		/* Append Supported channels IE */
		wlan_add_supported_5Ghz_channels(psoc, pdev, supp_chan_ie,
						&supp_chan_ie_len, true);

		wlan_cm_append_assoc_ies(rso_mode_cfg,
					 WLAN_ELEMID_SUPPCHAN,
					 supp_chan_ie_len, supp_chan_ie);
	}

	cm_esr_populate_version_ie(mlme_obj, rso_mode_cfg);

	if (mlme_obj->cfg.rrm_config.rrm_enabled) {
		/* Append RRM IE */
		if (rrm_cap_ie_data)
			wlan_cm_append_assoc_ies(rso_mode_cfg, WLAN_ELEMID_RRM,
						 DOT11F_IE_RRMENABLEDCAP_MAX_LEN,
						 rrm_cap_ie_data);

		/* Append Power cap IE if not appended already */
		if (!power_caps_populated)
			wlan_cm_append_assoc_ies(rso_mode_cfg,
						 WLAN_ELEMID_PWRCAP,
						 DOT11F_IE_POWERCAPS_MAX_LEN,
						  power_cap_ie_data);
	}

	wlan_cm_ese_populate_addtional_ies(pdev, mlme_obj, vdev_id,
					  rso_mode_cfg);

	/* Append QCN IE if g_support_qcn_ie INI is enabled */
	if (mlme_obj->cfg.sta.qcn_ie_support)
		wlan_cm_append_assoc_ies(rso_mode_cfg, WLAN_ELEMID_VENDOR,
					 sizeof(qcn_ie), qcn_ie);
}

static void
cm_roam_scan_offload_fill_rso_configs(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_vdev *vdev,
			struct rso_config *rso_cfg,
			struct wlan_roam_scan_offload_params *rso_mode_cfg,
			struct wlan_roam_scan_channel_list *rso_chan_info,
			uint8_t command, uint16_t reason)
{
	uint32_t mode = 0;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	qdf_mem_zero(rso_mode_cfg, sizeof(*rso_mode_cfg));
	rso_mode_cfg->vdev_id = vdev_id;
	rso_mode_cfg->is_rso_stop = (command == ROAM_SCAN_OFFLOAD_STOP);
	rso_mode_cfg->roaming_scan_policy =
		mlme_obj->cfg.lfr.roaming_scan_policy;

	/* Fill ROAM SCAN mode TLV parameters */
	if (rso_cfg->cfg_param.empty_scan_refresh_period)
		mode |= WMI_ROAM_SCAN_MODE_PERIODIC;

	rso_mode_cfg->rso_mode_info.min_delay_btw_scans =
			mlme_obj->cfg.lfr.min_delay_btw_roam_scans;
	rso_mode_cfg->rso_mode_info.min_delay_roam_trigger_bitmask =
			mlme_obj->cfg.lfr.roam_trigger_reason_bitmask;

	if (command == ROAM_SCAN_OFFLOAD_STOP) {
		if (reason == REASON_ROAM_STOP_ALL ||
		    reason == REASON_DISCONNECTED ||
		    reason == REASON_ROAM_SYNCH_FAILED ||
		    reason == REASON_ROAM_SET_PRIMARY) {
			mode = WMI_ROAM_SCAN_MODE_NONE;
		} else {
			if (wlan_is_roam_offload_enabled(mlme_obj->cfg.lfr))
				mode = WMI_ROAM_SCAN_MODE_NONE |
					WMI_ROAM_SCAN_MODE_ROAMOFFLOAD;
			else
				mode = WMI_ROAM_SCAN_MODE_NONE;
		}
	}

	rso_mode_cfg->rso_mode_info.roam_scan_mode = mode;
	if (command == ROAM_SCAN_OFFLOAD_STOP)
		return;

	cm_roam_scan_offload_fill_lfr3_config(vdev, rso_cfg, rso_mode_cfg,
					      mlme_obj, command, &mode);
	rso_mode_cfg->rso_mode_info.roam_scan_mode = mode;
	cm_roam_scan_offload_fill_scan_params(psoc, rso_cfg, mlme_obj,
					      rso_mode_cfg, rso_chan_info,
					      command);
	cm_update_driver_assoc_ies(psoc, vdev, rso_cfg, mlme_obj, rso_mode_cfg);
	cm_roam_scan_offload_add_fils_params(psoc, rso_mode_cfg, vdev_id);
}

/**
 * cm_update_btm_offload_config() - Update btm config param to fw
 * @psoc: psoc
 * @vdev: vdev
 * @command: Roam offload command
 * @btm_offload_config: btm offload config
 *
 * Return: None
 */
static void
cm_update_btm_offload_config(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_vdev *vdev,
			     uint8_t command, uint32_t *btm_offload_config)

{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_mlme_btm *btm_cfg;
	struct wlan_objmgr_peer *peer;
	uint8_t bssid[QDF_MAC_ADDR_SIZE];
	struct cm_roam_values_copy temp;
	bool is_hs_20_ap, is_pmf_enabled, is_open_connection = false;
	uint8_t vdev_id;
	uint32_t mbo_oce_enabled_ap;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	btm_cfg = &mlme_obj->cfg.btm;
	*btm_offload_config = btm_cfg->btm_offload_config;

	/* Return if INI is disabled */
	if (!(*btm_offload_config))
		return;

	vdev_id = wlan_vdev_get_id(vdev);
	wlan_cm_roam_cfg_get_value(psoc, vdev_id, HS_20_AP, &temp);
	is_hs_20_ap = temp.bool_value;

	/*
	 * For RSO Stop/Passpoint R2 cert test case 5.11(when STA is connected
	 * to Hotspot-2.0 AP), disable BTM offload to firmware
	 */
	if (command == ROAM_SCAN_OFFLOAD_STOP || is_hs_20_ap) {
		mlme_debug("RSO cmd: %d is_hs_20_ap:%d", command,
			   is_hs_20_ap);
		*btm_offload_config = 0;
		return;
	}

	ucfg_wlan_vdev_mgr_get_param_bssid(vdev, bssid);
	peer = wlan_objmgr_get_peer(psoc,
				    wlan_objmgr_pdev_get_pdev_id(
					wlan_vdev_get_pdev(vdev)),
				    bssid,
				    WLAN_MLME_CM_ID);
	if (!peer) {
		mlme_debug("Peer of peer_mac "QDF_MAC_ADDR_FMT" not found",
			   QDF_MAC_ADDR_REF(bssid));
		return;
	}

	is_pmf_enabled = mlme_get_peer_pmf_status(peer);

	wlan_objmgr_peer_release_ref(peer, WLAN_MLME_CM_ID);

	if (cm_is_open_mode(vdev))
		is_open_connection = true;

	wlan_cm_roam_cfg_get_value(psoc, vdev_id, MBO_OCE_ENABLED_AP, &temp);
	mbo_oce_enabled_ap = temp.uint_value;
	/*
	 * If peer does not support PMF in case of OCE/MBO
	 * Connection, Disable BTM offload to firmware.
	 */
	if (mbo_oce_enabled_ap && (!is_pmf_enabled && !is_open_connection))
		*btm_offload_config = 0;

	mlme_debug("is_open:%d is_pmf_enabled %d btm_offload_cfg:%d for "QDF_MAC_ADDR_FMT,
		   is_open_connection, is_pmf_enabled, *btm_offload_config,
		   QDF_MAC_ADDR_REF(bssid));
}

/**
 * cm_roam_scan_btm_offload() - set roam scan btm offload parameters
 * @psoc: psoc ctx
 * @vdev: vdev
 * @params:  roam scan btm offload parameters
 * @rso_cfg: rso config
 *
 * This function is used to set roam scan btm offload related parameters
 *
 * Return: None
 */
static void
cm_roam_scan_btm_offload(struct wlan_objmgr_psoc *psoc,
			 struct wlan_objmgr_vdev *vdev,
			 struct wlan_roam_btm_config *params,
			 struct rso_config *rso_cfg)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_mlme_btm *btm_cfg;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	btm_cfg = &mlme_obj->cfg.btm;
	params->vdev_id = wlan_vdev_get_id(vdev);
	cm_update_btm_offload_config(psoc, vdev, ROAM_SCAN_OFFLOAD_START,
				     &params->btm_offload_config);
	params->btm_solicited_timeout = btm_cfg->btm_solicited_timeout;
	params->btm_max_attempt_cnt = btm_cfg->btm_max_attempt_cnt;
	params->btm_sticky_time = btm_cfg->btm_sticky_time;
	params->disassoc_timer_threshold = btm_cfg->disassoc_timer_threshold;
	params->btm_query_bitmask = btm_cfg->btm_query_bitmask;
	params->btm_candidate_min_score = btm_cfg->btm_trig_min_candidate_score;
}

/**
 * cm_roam_offload_11k_params() - set roam 11k offload parameters
 * @psoc: psoc ctx
 * @vdev: vdev
 * @params:  roam 11k offload parameters
 * @enabled: 11k offload enabled/disabled
 *
 * This function is used to set roam 11k offload related parameters
 *
 * Return: None
 */
static void
cm_roam_offload_11k_params(struct wlan_objmgr_psoc *psoc,
			   struct wlan_objmgr_vdev *vdev,
			   struct wlan_roam_11k_offload_params *params,
			   bool enabled)
{
	struct cm_roam_neighbor_report_offload_params *neighbor_report_offload;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	neighbor_report_offload =
		&mlme_obj->cfg.lfr.rso_user_config.neighbor_report_offload;

	params->vdev_id = wlan_vdev_get_id(vdev);

	if (enabled) {
		params->offload_11k_bitmask =
			neighbor_report_offload->offload_11k_enable_bitmask;
	} else {
		params->offload_11k_bitmask = 0;
		return;
	}

	/*
	 * If none of the parameters are enabled, then set the
	 * offload_11k_bitmask to 0, so that we don't send the command
	 * to the FW and drop it in WMA
	 */
	if ((neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_ALL) == 0) {
		mlme_err("No valid neighbor report offload params %x",
			 neighbor_report_offload->params_bitmask);
		params->offload_11k_bitmask = 0;
		return;
	}

	/*
	 * First initialize all params to NEIGHBOR_REPORT_PARAM_INVALID
	 * Then set the values that are enabled
	 */
	params->neighbor_report_params.time_offset =
		NEIGHBOR_REPORT_PARAM_INVALID;
	params->neighbor_report_params.low_rssi_offset =
		NEIGHBOR_REPORT_PARAM_INVALID;
	params->neighbor_report_params.bmiss_count_trigger =
		NEIGHBOR_REPORT_PARAM_INVALID;
	params->neighbor_report_params.per_threshold_offset =
		NEIGHBOR_REPORT_PARAM_INVALID;
	params->neighbor_report_params.neighbor_report_cache_timeout =
		NEIGHBOR_REPORT_PARAM_INVALID;
	params->neighbor_report_params.max_neighbor_report_req_cap =
		NEIGHBOR_REPORT_PARAM_INVALID;

	if (neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_TIME_OFFSET)
		params->neighbor_report_params.time_offset =
			neighbor_report_offload->time_offset;

	if (neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_LOW_RSSI_OFFSET)
		params->neighbor_report_params.low_rssi_offset =
			neighbor_report_offload->low_rssi_offset;

	if (neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_BMISS_COUNT_TRIGGER)
		params->neighbor_report_params.bmiss_count_trigger =
			neighbor_report_offload->bmiss_count_trigger;

	if (neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_PER_THRESHOLD_OFFSET)
		params->neighbor_report_params.per_threshold_offset =
			neighbor_report_offload->per_threshold_offset;

	if (neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_CACHE_TIMEOUT)
		params->neighbor_report_params.neighbor_report_cache_timeout =
			neighbor_report_offload->neighbor_report_cache_timeout;

	if (neighbor_report_offload->params_bitmask &
	    NEIGHBOR_REPORT_PARAMS_MAX_REQ_CAP)
		params->neighbor_report_params.max_neighbor_report_req_cap =
			neighbor_report_offload->max_neighbor_report_req_cap;

	wlan_vdev_mlme_get_ssid(vdev, params->neighbor_report_params.ssid.ssid,
				&params->neighbor_report_params.ssid.length);
}

/**
 * cm_roam_start_req() - roam start request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_start_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		  uint8_t reason)
{
	struct wlan_roam_start_config *start_req;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct cm_roam_values_copy temp;

	start_req = qdf_mem_malloc(sizeof(*start_req));
	if (!start_req)
		return QDF_STATUS_E_NOMEM;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		goto free_mem;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto rel_vdev_ref;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		goto rel_vdev_ref;

	cm_roam_set_roam_reason_better_ap(psoc, vdev_id, false);
	/* fill from mlme directly */
	cm_roam_scan_bmiss_cnt(psoc, vdev_id, &start_req->beacon_miss_cnt);
	cm_roam_reason_vsie(psoc, vdev_id, &start_req->reason_vsie_enable);
	cm_roam_triggers(psoc, vdev_id, &start_req->roam_triggers);
	cm_roam_fill_rssi_change_params(psoc, vdev_id,
					&start_req->rssi_change_params);
	cm_roam_mawc_params(psoc, vdev_id, &start_req->mawc_params);
	cm_roam_bss_load_config(psoc, vdev_id, &start_req->bss_load_config);
	cm_roam_disconnect_params(psoc, vdev_id, &start_req->disconnect_params);
	cm_roam_idle_params(psoc, vdev_id, &start_req->idle_params);
	if (!(BIT(ROAM_TRIGGER_REASON_IDLE) &
	    start_req->roam_triggers.trigger_bitmap))
		start_req->idle_params.enable = false;

	cm_roam_scan_offload_rssi_thresh(psoc, vdev_id,
					 &start_req->rssi_params, rso_cfg);
	cm_roam_scan_offload_scan_period(vdev_id,
					 &start_req->scan_period_params,
					 rso_cfg);
	cm_roam_scan_offload_ap_profile(psoc, vdev, rso_cfg,
					&start_req->profile_params);
	cm_fill_rso_channel_list(psoc, vdev, rso_cfg, &start_req->rso_chan_info,
				 reason);
	cm_roam_scan_filter(psoc, pdev, vdev_id, ROAM_SCAN_OFFLOAD_START,
			    reason, &start_req->scan_filter_params);
	cm_roam_scan_offload_fill_rso_configs(psoc, vdev, rso_cfg,
					      &start_req->rso_config,
					      &start_req->rso_chan_info,
					      ROAM_SCAN_OFFLOAD_START,
					      reason);
	cm_roam_scan_btm_offload(psoc, vdev, &start_req->btm_config, rso_cfg);
	cm_roam_offload_11k_params(psoc, vdev, &start_req->roam_11k_params,
				   true);
	start_req->wlan_roam_rt_stats_config =
			wlan_cm_get_roam_rt_stats(psoc, ROAM_RT_STATS_ENABLE);

	start_req->wlan_roam_ho_delay_config =
			wlan_cm_roam_get_ho_delay_config(psoc);

	start_req->wlan_exclude_rm_partial_scan_freq =
				wlan_cm_get_exclude_rm_partial_scan_freq(psoc);

	start_req->wlan_roam_full_scan_6ghz_on_disc =
				wlan_cm_roam_get_full_scan_6ghz_on_disc(psoc);

	wlan_cm_roam_cfg_get_value(psoc, vdev_id, ROAM_RSSI_DIFF_6GHZ, &temp);
	start_req->wlan_roam_rssi_diff_6ghz = temp.uint_value;

	status = wlan_cm_tgt_send_roam_start_req(psoc, vdev_id, start_req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send roam start");

rel_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
free_mem:
	qdf_mem_free(start_req);

	return status;
}

/**
 * cm_roam_update_config_req() - roam update config request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_update_config_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			  uint8_t reason)
{
	struct wlan_roam_update_config *update_req;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct cm_roam_values_copy temp;

	cm_roam_set_roam_reason_better_ap(psoc, vdev_id, false);

	update_req = qdf_mem_malloc(sizeof(*update_req));
	if (!update_req)
		return QDF_STATUS_E_NOMEM;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		goto free_mem;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto rel_vdev_ref;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		goto rel_vdev_ref;

	/* fill from mlme directly */
	cm_roam_scan_bmiss_cnt(psoc, vdev_id, &update_req->beacon_miss_cnt);
	cm_roam_fill_rssi_change_params(psoc, vdev_id,
					&update_req->rssi_change_params);
	if (MLME_IS_ROAM_STATE_RSO_ENABLED(psoc, vdev_id)) {
		cm_roam_disconnect_params(psoc, vdev_id,
					  &update_req->disconnect_params);
		cm_roam_triggers(psoc, vdev_id,
				 &update_req->roam_triggers);
		cm_roam_idle_params(psoc, vdev_id,
				    &update_req->idle_params);
		if (!(BIT(ROAM_TRIGGER_REASON_IDLE) &
		    update_req->roam_triggers.trigger_bitmap))
			update_req->idle_params.enable = false;
	}
	cm_roam_scan_offload_rssi_thresh(psoc, vdev_id,
					 &update_req->rssi_params, rso_cfg);
	cm_roam_scan_offload_scan_period(vdev_id,
					 &update_req->scan_period_params,
					 rso_cfg);
	cm_roam_scan_offload_ap_profile(psoc, vdev, rso_cfg,
					&update_req->profile_params);
	cm_fill_rso_channel_list(psoc, vdev, rso_cfg,
				 &update_req->rso_chan_info, reason);
	cm_roam_scan_filter(psoc, pdev, vdev_id, ROAM_SCAN_OFFLOAD_UPDATE_CFG,
			    reason, &update_req->scan_filter_params);
	cm_roam_scan_offload_fill_rso_configs(psoc, vdev, rso_cfg,
					      &update_req->rso_config,
					      &update_req->rso_chan_info,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      reason);
	update_req->wlan_roam_rt_stats_config =
			wlan_cm_get_roam_rt_stats(psoc, ROAM_RT_STATS_ENABLE);

	update_req->wlan_roam_ho_delay_config =
			wlan_cm_roam_get_ho_delay_config(psoc);

	update_req->wlan_exclude_rm_partial_scan_freq =
				wlan_cm_get_exclude_rm_partial_scan_freq(psoc);

	update_req->wlan_roam_full_scan_6ghz_on_disc =
				wlan_cm_roam_get_full_scan_6ghz_on_disc(psoc);

	wlan_cm_roam_cfg_get_value(psoc, vdev_id, ROAM_RSSI_DIFF_6GHZ, &temp);
	update_req->wlan_roam_rssi_diff_6ghz = temp.uint_value;

	status = wlan_cm_tgt_send_roam_update_req(psoc, vdev_id, update_req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send update config");

rel_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
free_mem:
	qdf_mem_free(update_req);

	return status;
}

/**
 * cm_roam_restart_req() - roam restart req for LFR2
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_restart_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		    uint8_t reason)
{

	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_INVAL;

	/* Rome offload engine does not stop after any scan.
	 * If this command is sent because all preauth attempts failed
	 * and WMI_ROAM_REASON_SUITABLE_AP event was received earlier,
	 * now it is time to call it heartbeat failure.
	 */
	if ((reason == REASON_PREAUTH_FAILED_FOR_ALL ||
	     reason == REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW) &&
	     mlme_get_roam_reason_better_ap(vdev)) {
		mlme_err("Sending heartbeat failure, reason %d", reason);
		wlan_cm_send_beacon_miss(vdev_id, mlme_get_hb_ap_rssi(vdev));
		mlme_set_roam_reason_better_ap(vdev, false);
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_abort_req() - roam scan abort req
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_abort_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		  uint8_t reason)
{
	QDF_STATUS status;

	status = wlan_cm_tgt_send_roam_abort_req(psoc, vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send abort start");

	return status;
}

static void cm_fill_stop_reason(struct wlan_roam_stop_config *stop_req,
				uint8_t reason)
{
	if (reason == REASON_ROAM_SYNCH_FAILED)
		stop_req->reason = REASON_ROAM_SYNCH_FAILED;
	else if (reason == REASON_DRIVER_DISABLED)
		stop_req->reason = REASON_ROAM_STOP_ALL;
	else if (reason == REASON_SUPPLICANT_DISABLED_ROAMING)
		stop_req->reason = REASON_SUPPLICANT_DISABLED_ROAMING;
	else if (reason == REASON_DISCONNECTED)
		stop_req->reason = REASON_DISCONNECTED;
	else if (reason == REASON_OS_REQUESTED_ROAMING_NOW)
		stop_req->reason = REASON_OS_REQUESTED_ROAMING_NOW;
	else if (reason == REASON_ROAM_SET_PRIMARY)
		stop_req->reason = REASON_ROAM_SET_PRIMARY;
	else
		stop_req->reason = REASON_SME_ISSUED;
}

QDF_STATUS
cm_roam_stop_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		 uint8_t reason, bool *send_resp, bool start_timer)
{
	struct wlan_roam_stop_config *stop_req;
	QDF_STATUS status;
	struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	cm_roam_set_roam_reason_better_ap(psoc, vdev_id, false);
	stop_req = qdf_mem_malloc(sizeof(*stop_req));
	if (!stop_req)
		return QDF_STATUS_E_NOMEM;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		goto free_mem;
	}

	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_debug("MLO ROAM: skip RSO cmd for link vdev %d", vdev_id);
		return QDF_STATUS_SUCCESS;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto rel_vdev_ref;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		goto rel_vdev_ref;

	stop_req->btm_config.vdev_id = vdev_id;
	stop_req->disconnect_params.vdev_id = vdev_id;
	stop_req->idle_params.vdev_id = vdev_id;
	stop_req->roam_triggers.vdev_id = vdev_id;
	stop_req->rssi_params.vdev_id = vdev_id;
	stop_req->roam_11k_params.vdev_id = vdev_id;
	cm_fill_stop_reason(stop_req, reason);
	if (wlan_cm_host_roam_in_progress(psoc, vdev_id))
		stop_req->middle_of_roaming = 1;
	if (send_resp)
		stop_req->send_rso_stop_resp = *send_resp;
	stop_req->start_rso_stop_timer = start_timer;
	/*
	 * If roam synch propagation is in progress and an user space
	 * disconnect is requested, then there is no need to send the
	 * RSO STOP to firmware, since the roaming is already complete.
	 * If the RSO STOP is sent to firmware, then an HO_FAIL will be
	 * generated and the expectation from firmware would be to
	 * clean up the peer context on the host and not send down any
	 * WMI PEER DELETE commands to firmware. But, if the user space
	 * disconnect gets processed first, then there is a chance to
	 * send down the PEER DELETE commands. Hence, if we do not
	 * receive the HO_FAIL, and we complete the roam sync
	 * propagation, then the host and firmware will be in sync with
	 * respect to the peer and then the user space disconnect can
	 * be handled gracefully in a normal way.
	 *
	 * Ensure to check the reason code since the RSO Stop might
	 * come when roam sync failed as well and at that point it
	 * should go through to the firmware and receive HO_FAIL
	 * and clean up.
	 */
	if (MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id) &&
	    stop_req->reason == REASON_ROAM_STOP_ALL) {
		mlme_info("vdev_id:%d : Drop RSO stop during roam sync",
			  vdev_id);
		goto rel_vdev_ref;
	}

	wlan_mlme_defer_pmk_set_in_roaming(psoc, vdev_id, false);

	cm_roam_scan_filter(psoc, pdev, vdev_id, ROAM_SCAN_OFFLOAD_STOP,
			    reason, &stop_req->scan_filter_params);
	cm_roam_scan_offload_fill_rso_configs(psoc, vdev, rso_cfg,
					      &stop_req->rso_config,
					      NULL, ROAM_SCAN_OFFLOAD_STOP,
					      stop_req->reason);

	status = wlan_cm_tgt_send_roam_stop_req(psoc, vdev_id, stop_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_debug("fail to send roam stop");
	}
	if (send_resp)
		*send_resp = stop_req->send_rso_stop_resp;

rel_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
free_mem:
	qdf_mem_free(stop_req);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_fill_per_roam_request() - create PER roam offload config request
 * @psoc: psoc context
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_fill_per_roam_request(struct wlan_objmgr_psoc *psoc,
			      struct wlan_per_roam_config_req *req)
{
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	req->per_config.enable = mlme_obj->cfg.lfr.per_roam_enable;
	req->per_config.tx_high_rate_thresh =
		mlme_obj->cfg.lfr.per_roam_config_high_rate_th;
	req->per_config.rx_high_rate_thresh =
		mlme_obj->cfg.lfr.per_roam_config_high_rate_th;
	req->per_config.tx_low_rate_thresh =
		mlme_obj->cfg.lfr.per_roam_config_low_rate_th;
	req->per_config.rx_low_rate_thresh =
		mlme_obj->cfg.lfr.per_roam_config_low_rate_th;
	req->per_config.per_rest_time = mlme_obj->cfg.lfr.per_roam_rest_time;
	req->per_config.tx_per_mon_time =
		mlme_obj->cfg.lfr.per_roam_monitor_time;
	req->per_config.rx_per_mon_time =
		mlme_obj->cfg.lfr.per_roam_monitor_time;
	req->per_config.tx_rate_thresh_percnt =
		mlme_obj->cfg.lfr.per_roam_config_rate_th_percent;
	req->per_config.rx_rate_thresh_percnt =
		mlme_obj->cfg.lfr.per_roam_config_rate_th_percent;
	req->per_config.min_candidate_rssi =
		mlme_obj->cfg.lfr.per_roam_min_candidate_rssi;

	mlme_debug("PER based roaming configuaration enable: %d vdev: %d high_rate_thresh: %d low_rate_thresh: %d rate_thresh_percnt: %d per_rest_time: %d monitor_time: %d min cand rssi: %d",
		   req->per_config.enable, req->vdev_id,
		   req->per_config.tx_high_rate_thresh,
		   req->per_config.tx_low_rate_thresh,
		   req->per_config.tx_rate_thresh_percnt,
		   req->per_config.per_rest_time,
		   req->per_config.tx_per_mon_time,
		   req->per_config.min_candidate_rssi);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_offload_per_config() - populates roam offload scan request and sends
 * to fw
 * @psoc: psoc context
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_offload_per_config(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct wlan_per_roam_config_req *req;
	bool is_per_roam_enabled;
	QDF_STATUS status;

	/*
	 * Disable PER trigger for phymode less than 11n to avoid
	 * frequent roams as the PER rate threshold is greater than
	 * 11a/b/g rates
	 */
	is_per_roam_enabled = cm_roam_is_per_roam_allowed(psoc, vdev_id);
	if (!is_per_roam_enabled)
		return QDF_STATUS_SUCCESS;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->vdev_id = vdev_id;
	status = cm_roam_fill_per_roam_request(psoc, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(req);
		mlme_debug("fail to fill per config");
		return status;
	}

	status = wlan_cm_tgt_send_roam_per_config(psoc, vdev_id, req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send roam stop");

	qdf_mem_free(req);

	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
cm_akm_roam_allowed(struct wlan_objmgr_psoc *psoc,
		    struct wlan_objmgr_vdev *vdev)
{
	int32_t akm;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	uint32_t fw_akm_bitmap;

	akm = wlan_crypto_get_param(vdev,
				    WLAN_CRYPTO_PARAM_KEY_MGMT);
	mlme_debug("akm %x", akm);

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	if ((QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA384) ||
	     QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA256)) &&
	    !mlme_obj->cfg.lfr.rso_user_config.is_fils_roaming_supported) {
		mlme_info("FILS Roaming not suppprted by fw");
		return QDF_STATUS_E_NOSUPPORT;
	}
	fw_akm_bitmap = mlme_obj->cfg.lfr.fw_akm_bitmap;
	/* Roaming is not supported currently for OWE akm */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OWE) &&
	    !(fw_akm_bitmap & (1 << AKM_OWE))) {
		mlme_info("OWE Roaming not suppprted by fw");
		return QDF_STATUS_E_NOSUPPORT;
	}

	/* Roaming is not supported for SAE authentication */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE) &&
	    !CM_IS_FW_SAE_ROAM_SUPPORTED(fw_akm_bitmap)) {
		mlme_info("Roaming not suppprted for SAE connection");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if ((QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B) ||
	     QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192)) &&
	     !(fw_akm_bitmap & (1 << AKM_SUITEB))) {
		mlme_info("Roaming not supported for SUITEB connection");
		return QDF_STATUS_E_NOSUPPORT;
	}

	/*
	 * If fw doesn't advertise FT SAE, FT-FILS or FT-Suite-B capability,
	 * don't support roaming to that profile
	 */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE) &&
	    !CM_IS_FW_FT_SAE_SUPPORTED(fw_akm_bitmap)) {
		mlme_info("Roaming not suppprted for FT SAE akm");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384) &&
	    !(fw_akm_bitmap & (1 << AKM_FT_SUITEB_SHA384))) {
		mlme_info("Roaming not suppprted for FT Suite-B akm");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if ((QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384) ||
	    QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256)) &&
	    !(fw_akm_bitmap & (1 << AKM_FT_FILS))) {
		mlme_info("Roaming not suppprted for FT FILS akm");
		return QDF_STATUS_E_NOSUPPORT;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_set_roam_scan_high_rssi_offset(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id,
					     uint8_t param_value)
{
	struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_roam_offload_scan_rssi_params *roam_rssi_params;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	qdf_freq_t op_freq;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	op_freq = wlan_get_operation_chan_freq(vdev);
	if (WLAN_REG_IS_6GHZ_CHAN_FREQ(op_freq)) {
		mlme_err("vdev:%d High RSSI offset can't be set in 6 GHz band",
			 vdev_id);
		goto rel_vdev_ref;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto rel_vdev_ref;

	roam_rssi_params = qdf_mem_malloc(sizeof(*roam_rssi_params));
	if (!roam_rssi_params)
		goto rel_vdev_ref;

	wlan_cm_set_roam_scan_high_rssi_offset(psoc, param_value);
	qdf_mem_zero(roam_rssi_params, sizeof(*roam_rssi_params));
	cm_roam_scan_offload_rssi_thresh(psoc, vdev_id,
					 roam_rssi_params, rso_cfg);
	mlme_debug("vdev:%d Configured high RSSI delta=%d, 5 GHZ roam flag=%d",
		   vdev_id, roam_rssi_params->hi_rssi_scan_rssi_delta,
		   (roam_rssi_params->flags &
		    ROAM_SCAN_RSSI_THRESHOLD_FLAG_ROAM_HI_RSSI_EN_ON_5G));

	status = wlan_cm_tgt_send_roam_scan_offload_rssi_params(
							vdev, roam_rssi_params);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("fail to set roam scan high RSSI offset");

	qdf_mem_free(roam_rssi_params);
rel_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}
#endif

#ifdef WLAN_ADAPTIVE_11R
static bool
cm_is_adaptive_11r_roam_supported(struct wlan_mlme_psoc_ext_obj *mlme_obj,
				  struct rso_config *rso_cfg)
{
	if (rso_cfg->is_adaptive_11r_connection)
		return mlme_obj->cfg.lfr.tgt_adaptive_11r_cap;

	return true;
}
#else
static bool
cm_is_adaptive_11r_roam_supported(struct wlan_mlme_psoc_ext_obj *mlme_obj,
				  struct rso_config *rso_cfg)

{
	return true;
}
#endif

static QDF_STATUS
cm_roam_cmd_allowed(struct wlan_objmgr_psoc *psoc,
		    struct wlan_objmgr_vdev *vdev,
		    uint8_t command, uint8_t reason)
{
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct rso_config *rso_cfg;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	bool p2p_disable_sta_roaming = 0, nan_disable_sta_roaming = 0;
	QDF_STATUS  status;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_FAILURE;

	mlme_debug("RSO Command %d, vdev %d, Reason %d",
		   command, vdev_id, reason);

	if (!cm_is_vdev_connected(vdev) &&
	    (command == ROAM_SCAN_OFFLOAD_UPDATE_CFG ||
	     command == ROAM_SCAN_OFFLOAD_START ||
	     command == ROAM_SCAN_OFFLOAD_RESTART)) {
		mlme_debug("vdev not in connected state and command %d ",
			   command);
		return QDF_STATUS_E_FAILURE;
	}

	if (!cm_is_adaptive_11r_roam_supported(mlme_obj, rso_cfg)) {
		mlme_info("Adaptive 11r Roaming not suppprted by fw");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = cm_akm_roam_allowed(psoc, vdev);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	p2p_disable_sta_roaming =
		(cfg_p2p_is_roam_config_disabled(psoc) &&
		(policy_mgr_mode_specific_connection_count(
					psoc, PM_P2P_CLIENT_MODE, NULL) ||
		policy_mgr_mode_specific_connection_count(
					psoc, PM_P2P_GO_MODE, NULL)));
	nan_disable_sta_roaming =
	    (cfg_nan_is_roam_config_disabled(psoc) &&
	    policy_mgr_mode_specific_connection_count(psoc, PM_NDI_MODE, NULL));

	if ((command == ROAM_SCAN_OFFLOAD_START ||
	     command == ROAM_SCAN_OFFLOAD_UPDATE_CFG) &&
	     (p2p_disable_sta_roaming || nan_disable_sta_roaming)) {
		mlme_info("roaming not supported for active %s connection",
			 p2p_disable_sta_roaming ? "p2p" : "ndi");
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * The Dynamic Config Items Update may happen even if the state is in
	 * INIT. It is important to ensure that the command is passed down to
	 * the FW only if the Infra Station is in a connected state. A connected
	 * station could also be in a PREAUTH or REASSOC states.
	 * 1) Block all CMDs that are not STOP in INIT State. For STOP always
	 *    inform firmware irrespective of state.
	 * 2) Block update cfg CMD if its for REASON_ROAM_SET_BLACKLIST_BSSID,
	 *    because we need to inform firmware of blacklisted AP for PNO in
	 *    all states.
	 */
	if ((cm_is_vdev_disconnecting(vdev) ||
	     cm_is_vdev_disconnected(vdev)) &&
	    (command != ROAM_SCAN_OFFLOAD_STOP) &&
	    (reason != REASON_ROAM_SET_BLACKLIST_BSSID)) {
		mlme_info("Scan Command not sent to FW and cmd=%d", command);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cm_is_rso_allowed(struct wlan_objmgr_psoc *psoc,
				    uint8_t vdev_id, uint8_t command,
				    uint8_t reason)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d: vdev not found", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	status = cm_roam_cmd_allowed(psoc, vdev, command, reason);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

void cm_handle_sta_sta_roaming_enablement(struct wlan_objmgr_psoc *psoc,
					  uint8_t curr_vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	uint32_t sta_count, conn_idx = 0;
	struct dual_sta_policy *dual_sta_policy;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	uint8_t temp_vdev_id;
	uint8_t vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS];

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, curr_vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_debug("vdev object is NULL");
		return;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		goto rel_ref;

	dual_sta_policy = &mlme_obj->cfg.gen.dual_sta_policy;
	sta_count = policy_mgr_get_mode_specific_conn_info(psoc, NULL,
							   vdev_id_list,
							   PM_STA_MODE);

	if (!(wlan_mlme_get_dual_sta_roaming_enabled(psoc) && sta_count >= 2)) {
		mlme_debug("Dual sta roaming is not enabled or count:%d",
			   sta_count);
		goto rel_ref;
	}

	if (policy_mgr_concurrent_sta_doing_dbs(psoc)) {
		mlme_debug("After roam on vdev_id:%d, STA + STA concurrency is in DBS:%d",
			   curr_vdev_id, sta_count);
		for (conn_idx = 0; conn_idx < sta_count; conn_idx++) {
			temp_vdev_id = vdev_id_list[conn_idx];
			if (temp_vdev_id == curr_vdev_id) {
				wlan_cm_roam_activate_pcl_per_vdev(psoc,
								   curr_vdev_id,
								   true);
				/* Set PCL after sending roam complete */
				policy_mgr_set_pcl_for_existing_combo(psoc,
								PM_STA_MODE,
								curr_vdev_id);
			} else {
				/* Enable roaming on secondary vdev */
				if_mgr_enable_roaming(pdev, vdev, RSO_SET_PCL);
			}
		}
	} else {
		mlme_debug("After roam STA + STA concurrency is in MCC/SCC");
	}
rel_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

QDF_STATUS cm_roam_send_rso_cmd(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id, uint8_t rso_command,
				uint8_t reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = cm_is_rso_allowed(psoc, vdev_id, rso_command, reason);

	if (status == QDF_STATUS_E_NOSUPPORT)
		return QDF_STATUS_SUCCESS;
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_debug("ROAM: not allowed");
		return status;
	}

	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_debug("MLO ROAM: skip RSO cmd for link vdev %d", vdev_id);
		return QDF_STATUS_SUCCESS;
	}

	/*
	 * Update PER config to FW. No need to update in case of stop command,
	 * FW takes care of stopping this internally
	 */
	if (rso_command != ROAM_SCAN_OFFLOAD_STOP)
		cm_roam_offload_per_config(psoc, vdev_id);

	if (rso_command == ROAM_SCAN_OFFLOAD_START)
		status = cm_roam_start_req(psoc, vdev_id, reason);
	else if (rso_command == ROAM_SCAN_OFFLOAD_UPDATE_CFG)
		status = cm_roam_update_config_req(psoc, vdev_id, reason);
	else if (rso_command == ROAM_SCAN_OFFLOAD_RESTART)
		status = cm_roam_restart_req(psoc, vdev_id, reason);
	else if (rso_command == ROAM_SCAN_OFFLOAD_ABORT_SCAN)
		status = cm_roam_abort_req(psoc, vdev_id, reason);
	else
		mlme_debug("ROAM: invalid RSO command %d", rso_command);

	return status;
}

/**
 * cm_roam_switch_to_rso_stop() - roam state handling for rso stop
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_RSO_STOPPED roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_switch_to_rso_stop(struct wlan_objmgr_pdev *pdev,
			   uint8_t vdev_id,
			   uint8_t reason, bool *send_resp, bool start_timer)
{
	enum roam_offload_state cur_state;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cur_state = mlme_get_roam_state(psoc, vdev_id);
	switch (cur_state) {
	case WLAN_ROAM_RSO_ENABLED:
	case WLAN_ROAMING_IN_PROG:
	case WLAN_ROAM_SYNCH_IN_PROG:
		status = cm_roam_stop_req(psoc, vdev_id, reason,
					  send_resp, start_timer);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err("ROAM: Unable to switch to RSO STOP State");
			return QDF_STATUS_E_FAILURE;
		}
		break;

	case WLAN_ROAM_DEINIT:
	case WLAN_ROAM_RSO_STOPPED:
	case WLAN_ROAM_INIT:
	/*
	 * Already the roaming module is initialized at fw,
	 * nothing to do here
	 */
	default:

		return QDF_STATUS_SUCCESS;
	}
	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_RSO_STOPPED);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_deinit() - roam state handling for roam deinit
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_DEINIT roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_switch_to_deinit(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id,
			 uint8_t reason)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	enum roam_offload_state cur_state = mlme_get_roam_state(psoc, vdev_id);
	bool sup_disabled_roam;

	switch (cur_state) {
	/*
	 * If RSO stop is not done already, send RSO stop first and
	 * then post deinit.
	 */
	case WLAN_ROAM_RSO_ENABLED:
	case WLAN_ROAMING_IN_PROG:
	case WLAN_ROAM_SYNCH_IN_PROG:
		cm_roam_switch_to_rso_stop(pdev, vdev_id, reason, NULL, false);
		break;
	case WLAN_ROAM_RSO_STOPPED:
		/*
		 * When Supplicant disabled roaming is set and roam invoke
		 * command is received from userspace, fw starts to roam.
		 * But meanwhile if a disassoc/deauth is received from AP or if
		 * NB disconnect is initiated while supplicant disabled roam,
		 * RSO stop with ROAM scan mode as 0 is not sent to firmware
		 * since the previous state was RSO_STOPPED. This could lead
		 * to firmware not sending peer unmap event for the current
		 * AP. To avoid this, if previous RSO stop was sent with
		 * ROAM scan mode as 4, send RSO stop with Roam scan mode as 0
		 * and then switch to ROAM_DEINIT.
		 */
		sup_disabled_roam =
			mlme_get_supplicant_disabled_roaming(psoc,
							     vdev_id);
		if (sup_disabled_roam) {
			mlme_err("vdev[%d]: supplicant disabled roam. clear roam scan mode",
				 vdev_id);
			status = cm_roam_stop_req(psoc, vdev_id,
						  REASON_DISCONNECTED,
						  NULL, false);
			if (QDF_IS_STATUS_ERROR(status))
				mlme_err("ROAM: Unable to clear roam scan mode");
		}
		break;
	case WLAN_ROAM_INIT:
		break;

	case WLAN_MLO_ROAM_SYNCH_IN_PROG:
		mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_DEINIT);
		break;

	case WLAN_ROAM_DEINIT:
	/*
	 * Already the roaming module is de-initialized at fw,
	 * do nothing here
	 */
	default:
		return QDF_STATUS_SUCCESS;
	}

	status = cm_roam_init_req(psoc, vdev_id, false);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_DEINIT);
	mlme_clear_operations_bitmap(psoc, vdev_id);
	wlan_cm_roam_activate_pcl_per_vdev(psoc, vdev_id, false);

	/* In case of roaming getting disabled due to
	 * REASON_ROAM_SET_PRIMARY reason, don't enable roaming on
	 * the other vdev as that is taken care by the caller once roaming
	 * on this "vdev_id" is disabled.
	 */
	if (reason != REASON_SUPPLICANT_INIT_ROAMING &&
	    reason != REASON_ROAM_SET_PRIMARY) {
		mlme_debug("vdev_id:%d enable roaming on other connected sta - reason:%d",
			   vdev_id, reason);
		wlan_cm_enable_roaming_on_connected_sta(pdev, vdev_id);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_init() - roam state handling for roam init
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_INIT roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_switch_to_init(struct wlan_objmgr_pdev *pdev,
		       uint8_t vdev_id,
		       uint8_t reason)
{
	enum roam_offload_state cur_state;
	uint8_t temp_vdev_id, roam_enabled_vdev_id;
	uint32_t roaming_bitmap;
	bool dual_sta_roam_active, usr_disabled_roaming, sta_concurrency_is_dbs;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct dual_sta_policy *dual_sta_policy;
	struct wlan_objmgr_vdev *vdev;
	bool is_vdev_primary = false;

	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_FAILURE;

	dual_sta_policy = &mlme_obj->cfg.gen.dual_sta_policy;
	dual_sta_roam_active = wlan_mlme_get_dual_sta_roaming_enabled(psoc);
	sta_concurrency_is_dbs = policy_mgr_concurrent_sta_doing_dbs(psoc);
	cur_state = mlme_get_roam_state(psoc, vdev_id);

	mlme_info("dual_sta_roam_active:%d, is_dbs:%d, state:%d",
		  dual_sta_roam_active, sta_concurrency_is_dbs,
		  cur_state);

	switch (cur_state) {
	case WLAN_ROAM_DEINIT:
		roaming_bitmap = mlme_get_roam_trigger_bitmap(psoc, vdev_id);
		if (!roaming_bitmap) {
			mlme_info("CM_RSO: Cannot change to INIT state for vdev[%d]",
				  vdev_id);
			return QDF_STATUS_E_FAILURE;
		}

		/*
		 * Enable roaming on other interface only if STA + STA
		 * concurrency is in DBS.
		 */
		if (dual_sta_roam_active && sta_concurrency_is_dbs) {
			mlme_info("STA + STA concurrency is in DBS");
			break;
		}
		/*
		 * Disable roaming on the enabled sta if supplicant wants to
		 * enable roaming on this vdev id
		 */
		temp_vdev_id = policy_mgr_get_roam_enabled_sta_session_id(
								psoc, vdev_id);
		if (temp_vdev_id != WLAN_UMAC_VDEV_ID_MAX) {
			/*
			 * Roam init state can be requested as part of
			 * initial connection or due to enable from
			 * supplicant via vendor command. This check will
			 * ensure roaming does not get enabled on this STA
			 * vdev id if it is not an explicit enable from
			 * supplicant.
			 */
			mlme_debug("Interface vdev_id: %d, roaming enabled on vdev_id: %d, primary vdev_id:%d, reason:%d",
				   vdev_id, temp_vdev_id,
				   dual_sta_policy->primary_vdev_id, reason);

			if (vdev_id == dual_sta_policy->primary_vdev_id)
				is_vdev_primary = true;

			if (is_vdev_primary ||
			    reason == REASON_SUPPLICANT_INIT_ROAMING) {
				cm_roam_state_change(pdev, temp_vdev_id,
						     WLAN_ROAM_DEINIT,
						     is_vdev_primary ?
					     REASON_ROAM_SET_PRIMARY : reason,
						     NULL, false);
			} else {
				mlme_info("CM_RSO: Roam module already initialized on vdev:[%d]",
					  temp_vdev_id);
				return QDF_STATUS_E_FAILURE;
			}
		}
		break;

	case WLAN_ROAM_SYNCH_IN_PROG:
		mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_INIT);
		return QDF_STATUS_SUCCESS;

	case WLAN_ROAM_INIT:
	case WLAN_ROAM_RSO_STOPPED:
	case WLAN_ROAM_RSO_ENABLED:
	case WLAN_ROAMING_IN_PROG:
	/*
	 * Already the roaming module is initialized at fw,
	 * just return from here
	 */
	default:
		return QDF_STATUS_SUCCESS;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev) {
		mlme_err("CM_RSO: vdev is null");
		return QDF_STATUS_E_INVAL;
	}

	if (cm_is_vdev_disconnecting(vdev) ||
	    cm_is_vdev_disconnected(vdev)) {
		mlme_debug("CM_RSO: RSO Init received in disconnected state");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	status = cm_roam_init_req(psoc, vdev_id, true);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_INIT);

	roam_enabled_vdev_id =
		policy_mgr_get_roam_enabled_sta_session_id(psoc, vdev_id);

	/* Send PDEV pcl command if only one STA is in connected state
	 * If there is another STA connection exist, then set the
	 * PCL type to vdev level
	 */
	if (roam_enabled_vdev_id != WLAN_UMAC_VDEV_ID_MAX &&
	    dual_sta_roam_active && sta_concurrency_is_dbs)
		wlan_cm_roam_activate_pcl_per_vdev(psoc, vdev_id, true);

	/* Set PCL before sending RSO start */
	policy_mgr_set_pcl_for_existing_combo(psoc, PM_STA_MODE, vdev_id);

	wlan_mlme_get_usr_disabled_roaming(psoc, &usr_disabled_roaming);
	if (usr_disabled_roaming) {
		status =
		cm_roam_send_disable_config(
			psoc, vdev_id,
			WMI_VDEV_ROAM_11KV_CTRL_DISABLE_FW_TRIGGER_ROAMING);

		if (!QDF_IS_STATUS_SUCCESS(status))
			mlme_err("ROAM: fast roaming disable failed. status %d",
				 status);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_rso_enable() - roam state handling for rso started
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_RSO_ENABLED roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_switch_to_rso_enable(struct wlan_objmgr_pdev *pdev,
			     uint8_t vdev_id,
			     uint8_t reason)
{
	enum roam_offload_state cur_state, new_roam_state;
	QDF_STATUS status;
	uint8_t control_bitmap;
	bool sup_disabled_roaming;
	bool rso_allowed;
	uint8_t rso_command = ROAM_SCAN_OFFLOAD_START;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	wlan_mlme_get_roam_scan_offload_enabled(psoc, &rso_allowed);
	sup_disabled_roaming = mlme_get_supplicant_disabled_roaming(psoc,
								    vdev_id);
	control_bitmap = mlme_get_operations_bitmap(psoc, vdev_id);

	cur_state = mlme_get_roam_state(psoc, vdev_id);
	mlme_debug("CM_RSO: vdev%d: cur_state : %d", vdev_id, cur_state);

	switch (cur_state) {
	case WLAN_ROAM_INIT:
	case WLAN_ROAM_RSO_STOPPED:
		break;

	case WLAN_ROAM_DEINIT:
		status = cm_roam_switch_to_init(pdev, vdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		break;
	case WLAN_ROAM_RSO_ENABLED:
		/*
		 * Send RSO update config if roaming already enabled
		 */
		rso_command = ROAM_SCAN_OFFLOAD_UPDATE_CFG;
		break;
	case WLAN_ROAMING_IN_PROG:
		/*
		 * When roam abort happens, the roam offload
		 * state machine moves to RSO_ENABLED state.
		 * But if Supplicant disabled roaming is set in case
		 * of roam invoke or if roaming was disabled due to
		 * other reasons like SAP start/connect on other vdev,
		 * the state should be transitioned to RSO STOPPED.
		 */
		if (sup_disabled_roaming || control_bitmap)
			new_roam_state = WLAN_ROAM_RSO_STOPPED;
		else
			new_roam_state = WLAN_ROAM_RSO_ENABLED;

		mlme_set_roam_state(psoc, vdev_id, new_roam_state);

		return QDF_STATUS_SUCCESS;
	case WLAN_ROAM_SYNCH_IN_PROG:
		if (reason == REASON_ROAM_ABORT) {
			mlme_debug("Roam synch in progress, drop Roam abort");
			return QDF_STATUS_SUCCESS;
		}
		/*
		 * After roam sych propagation is complete, send
		 * RSO start command to firmware to update AP profile,
		 * new PCL.
		 * If this is roam invoke case and supplicant has already
		 * disabled firmware roaming, then move to RSO stopped state
		 * instead of RSO enabled.
		 */
		if (sup_disabled_roaming || control_bitmap) {
			new_roam_state = WLAN_ROAM_RSO_STOPPED;
			mlme_set_roam_state(psoc, vdev_id, new_roam_state);

			return QDF_STATUS_SUCCESS;
		}

		break;
	default:
		return QDF_STATUS_SUCCESS;
	}

	if (!rso_allowed) {
		mlme_debug("ROAM: RSO disabled via INI");
		return QDF_STATUS_E_FAILURE;
	}

	if (control_bitmap) {
		mlme_debug("ROAM: RSO Disabled internaly: vdev[%d] bitmap[0x%x]",
			   vdev_id, control_bitmap);
		return QDF_STATUS_E_FAILURE;
	}

	status = cm_roam_send_rso_cmd(psoc, vdev_id, rso_command, reason);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_debug("ROAM: RSO start failed");
		return status;
	}
	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_RSO_ENABLED);

	/* If the set_key for the connected bssid was received during Roam sync
	 * in progress, then the RSO update to the FW will be rejected. The RSO
	 * start which might be in progress during set_key could send stale pmk
	 * to the FW. Therefore, once RSO is enabled, send the RSO update with
	 * the PMK received from the __wlan_hdd_cfg80211_keymgmt_set_key.
	 */
	if (wlan_mlme_is_pmk_set_deferred(psoc, vdev_id)) {
		cm_roam_send_rso_cmd(psoc, vdev_id,
				     ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				     REASON_ROAM_PSK_PMK_CHANGED);
		wlan_mlme_defer_pmk_set_in_roaming(psoc, vdev_id, false);
	}

	/*
	 * If supplicant disabled roaming, driver does not send
	 * RSO cmd to fw. This causes roam invoke to fail in FW
	 * since RSO start never happened at least once to
	 * configure roaming engine in FW. So send RSO start followed
	 * by RSO stop if supplicant disabled roaming is true.
	 */
	if (!sup_disabled_roaming)
		return QDF_STATUS_SUCCESS;

	mlme_debug("ROAM: RSO disabled by Supplicant on vdev[%d]", vdev_id);
	return cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_STOPPED,
				    REASON_SUPPLICANT_DISABLED_ROAMING,
				    NULL, false);
}

/**
 * cm_roam_switch_to_roam_start() - roam state handling for ROAMING_IN_PROG
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAMING_IN_PROG roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_switch_to_roam_start(struct wlan_objmgr_pdev *pdev,
			     uint8_t vdev_id,
			     uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	enum roam_offload_state cur_state =
				mlme_get_roam_state(psoc, vdev_id);
	switch (cur_state) {
	case WLAN_ROAM_RSO_ENABLED:
		mlme_set_roam_state(psoc, vdev_id, WLAN_ROAMING_IN_PROG);
		break;

	case WLAN_ROAM_RSO_STOPPED:
		/*
		 * When supplicant has disabled roaming, roam invoke triggered
		 * from supplicant can cause firmware to send roam start
		 * notification. Allow roam start in this condition.
		 */
		if (mlme_get_supplicant_disabled_roaming(psoc, vdev_id) &&

		    wlan_cm_roaming_in_progress(pdev, vdev_id)) {
			mlme_set_roam_state(psoc, vdev_id,
					    WLAN_ROAMING_IN_PROG);
			break;
		}
		fallthrough;
	case WLAN_ROAM_INIT:
	case WLAN_ROAM_DEINIT:
	case WLAN_ROAM_SYNCH_IN_PROG:
	default:
		mlme_err("ROAM: Roaming start received in invalid state: %d",
			 cur_state);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_roam_sync() - roam state handling for roam sync
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_SYNCH_IN_PROG roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_switch_to_roam_sync(struct wlan_objmgr_pdev *pdev,
			    uint8_t vdev_id,
			    uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	enum roam_offload_state cur_state = mlme_get_roam_state(psoc, vdev_id);

	switch (cur_state) {
	case WLAN_ROAM_RSO_ENABLED:
		/*
		 * Roam synch can come directly without roam start
		 * after waking up from power save mode or in case of
		 * deauth roam trigger to stop data path queues
		 */
	case WLAN_ROAMING_IN_PROG:
		if (!cm_is_vdevid_active(pdev, vdev_id)) {
			mlme_err("ROAM: STA not in connected state");
			return QDF_STATUS_E_FAILURE;
		}

		mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_SYNCH_IN_PROG);
		break;
	case WLAN_ROAM_RSO_STOPPED:
		/*
		 * If roaming is disabled by Supplicant and if this transition
		 * is due to roaming invoked by the supplicant, then allow
		 * this state transition
		 */
		if (mlme_get_supplicant_disabled_roaming(psoc, vdev_id) &&
		    wlan_cm_roaming_in_progress(pdev, vdev_id)) {
			mlme_set_roam_state(psoc, vdev_id,
					    WLAN_ROAM_SYNCH_IN_PROG);
			break;
		}
		/*
		 * transition to WLAN_ROAM_SYNCH_IN_PROG not allowed otherwise
		 * if we're already RSO stopped, fall through to return failure
		 */
		fallthrough;
	case WLAN_ROAM_INIT:
	case WLAN_ROAM_DEINIT:
	case WLAN_ROAM_SYNCH_IN_PROG:
	default:
		mlme_err("ROAM: Roam synch not allowed in [%d] state",
			 cur_state);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * cm_mlo_roam_switch_for_link() - roam state handling during mlo roam
  for link/s.
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_MLO_ROAM_SYNCH_IN_PROG roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_mlo_roam_switch_for_link(struct wlan_objmgr_pdev *pdev,
			    uint8_t vdev_id,
			    uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	enum roam_offload_state cur_state = mlme_get_roam_state(psoc, vdev_id);

	if (reason != REASON_ROAM_HANDOFF_DONE)
		return QDF_STATUS_E_FAILURE;

	switch (cur_state) {
	case WLAN_ROAM_DEINIT:
		/* Only used for link vdev during MLO roaming */
		mlme_set_roam_state(psoc, vdev_id, WLAN_MLO_ROAM_SYNCH_IN_PROG);
		break;
	case WLAN_MLO_ROAM_SYNCH_IN_PROG:
		mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_DEINIT);
		break;
	default:
		mlme_err("ROAM: MLO Roam synch not allowed in [%d] state",
			 cur_state);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_mlo_rso_state_change(struct wlan_objmgr_pdev *pdev,
			       uint8_t *vdev_id,
			       uint8_t reason,
			       bool *is_rso_skip)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *assoc_vdev = NULL;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, *vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	if (wlan_vdev_mlme_get_is_mlo_vdev(wlan_pdev_get_psoc(pdev),
					   *vdev_id)) {
		if ((reason == REASON_DISCONNECTED ||
		     reason == REASON_DRIVER_DISABLED) &&
		    (cm_is_vdev_disconnecting(vdev))) {
			mlme_debug("MLO ROAM: skip RSO cmd on assoc vdev %d",
				   *vdev_id);
			*is_rso_skip = true;
		}
	}

	if (wlan_vdev_mlme_get_is_mlo_link(wlan_pdev_get_psoc(pdev),
					   *vdev_id)) {
		if (reason == REASON_ROAM_HANDOFF_DONE) {
			status = cm_mlo_roam_switch_for_link(pdev, *vdev_id,
							     reason);
			mlme_debug("MLO ROAM: update rso state on link vdev %d",
				   *vdev_id);
			*is_rso_skip = true;
		} else if ((reason == REASON_DISCONNECTED ||
			    reason == REASON_DRIVER_DISABLED) &&
			   (cm_is_vdev_disconnecting(vdev))) {
			assoc_vdev = wlan_mlo_get_assoc_link_vdev(vdev);

			if (!assoc_vdev) {
				mlme_err("Assoc vdev is NULL");
				status = QDF_STATUS_E_FAILURE;
				goto end;
			}
			/* Update the vdev id to send RSO stop on assoc vdev */
			*vdev_id = wlan_vdev_get_id(assoc_vdev);
			*is_rso_skip = false;
			mlme_debug("MLO ROAM: process RSO stop on assoc vdev %d",
				   *vdev_id);
			goto end;
		} else {
			mlme_debug("MLO ROAM: skip RSO cmd on link vdev %d",
				   *vdev_id);
			*is_rso_skip = true;
		}
	}
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);
	return status;
}
#endif

QDF_STATUS
cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
		     uint8_t vdev_id,
		     enum roam_offload_state requested_state,
		     uint8_t reason, bool *send_resp, bool start_timer)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	bool is_up;
	bool is_rso_skip = false;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return status;

	is_up = QDF_IS_STATUS_SUCCESS(wlan_vdev_is_up(vdev));
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	if (requested_state != WLAN_ROAM_DEINIT && !is_up) {
		mlme_debug("ROAM: roam state change requested in disconnected state");
		return status;
	}

	status = cm_handle_mlo_rso_state_change(pdev, &vdev_id,
						reason, &is_rso_skip);
	if (is_rso_skip)
		return status;

	switch (requested_state) {
	case WLAN_ROAM_DEINIT:
		status = cm_roam_switch_to_deinit(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_INIT:
		status = cm_roam_switch_to_init(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_RSO_ENABLED:
		status = cm_roam_switch_to_rso_enable(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_RSO_STOPPED:
		status = cm_roam_switch_to_rso_stop(pdev, vdev_id, reason,
						    send_resp, start_timer);
		break;
	case WLAN_ROAMING_IN_PROG:
		status = cm_roam_switch_to_roam_start(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_SYNCH_IN_PROG:
		status = cm_roam_switch_to_roam_sync(pdev, vdev_id, reason);
		break;
	default:
		mlme_debug("ROAM: Invalid roam state %d", requested_state);
		break;
	}

	return status;
}

#ifdef FEATURE_WLAN_ESE
static QDF_STATUS
cm_roam_channels_filter_by_current_band(struct wlan_objmgr_pdev *pdev,
					uint8_t vdev_id,
					qdf_freq_t *in_chan_freq_list,
					uint8_t in_num_chan,
					qdf_freq_t *out_chan_freq_list,
					uint8_t *merged_num_chan)
{
	uint8_t i = 0;
	uint8_t num_chan = 0;
	uint32_t curr_ap_op_chan_freq =
		wlan_get_operation_chan_freq_vdev_id(pdev, vdev_id);

	/* Check for NULL pointer */
	if (!in_chan_freq_list)
		return QDF_STATUS_E_INVAL;

	/* Check for NULL pointer */
	if (!out_chan_freq_list)
		return QDF_STATUS_E_INVAL;

	if (in_num_chan > CFG_VALID_CHANNEL_LIST_LEN) {
		mlme_err("Wrong Number of Input Channels %d", in_num_chan);
		return QDF_STATUS_E_INVAL;
	}
	for (i = 0; i < in_num_chan; i++) {
		if (WLAN_REG_IS_SAME_BAND_FREQS(curr_ap_op_chan_freq,
						in_chan_freq_list[i])) {
			out_chan_freq_list[num_chan] = in_chan_freq_list[i];
			num_chan++;
		}
	}

	/* Return final number of channels */
	*merged_num_chan = num_chan;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cm_roam_merge_channel_lists(qdf_freq_t *in_chan_freq_list,
					      uint8_t in_num_chan,
					      qdf_freq_t *out_chan_freq_list,
					      uint8_t out_num_chan,
					      uint8_t *merged_num_chan)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t num_chan = out_num_chan;

	/* Check for NULL pointer */
	if (!in_chan_freq_list)
		return QDF_STATUS_E_INVAL;

	/* Check for NULL pointer */
	if (!out_chan_freq_list)
		return QDF_STATUS_E_INVAL;

	if (in_num_chan > CFG_VALID_CHANNEL_LIST_LEN) {
		mlme_err("Wrong Number of Input Channels %d", in_num_chan);
		return QDF_STATUS_E_INVAL;
	}
	if (out_num_chan >= CFG_VALID_CHANNEL_LIST_LEN) {
		mlme_err("Wrong Number of Output Channels %d", out_num_chan);
		return QDF_STATUS_E_INVAL;
	}
	/* Add the "new" channels in the input list to the end of the
	 * output list.
	 */
	for (i = 0; i < in_num_chan; i++) {
		for (j = 0; j < out_num_chan; j++) {
			if (in_chan_freq_list[i] == out_chan_freq_list[j])
				break;
		}
		if (j == out_num_chan) {
			if (in_chan_freq_list[i]) {
				mlme_debug("Adding extra %d to roam channel list",
					   in_chan_freq_list[i]);
				out_chan_freq_list[num_chan] =
							in_chan_freq_list[i];
				num_chan++;
			}
		}
		if (num_chan >= CFG_VALID_CHANNEL_LIST_LEN) {
			mlme_debug("Merge Neighbor channel list reached Max limit %d",
				   in_num_chan);
			break;
		}
	}

	/* Return final number of channels */
	*merged_num_chan = num_chan;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_create_roam_scan_channel_list(struct wlan_objmgr_pdev *pdev,
					    struct rso_config *rso_cfg,
					    uint8_t vdev_id,
					    qdf_freq_t *chan_freq_list,
					    uint8_t num_chan,
					    const enum band_info band)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t out_num_chan = 0;
	uint8_t in_chan_num = num_chan;
	qdf_freq_t *in_ptr = chan_freq_list;
	uint8_t i = 0;
	qdf_freq_t *freq_list;
	qdf_freq_t *tmp_chan_freq_list;
	uint8_t merged_out_chan_num = 0;
	struct rso_chan_info *chan_lst;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_mlme_reg *reg;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return QDF_STATUS_E_INVAL;

	reg = &mlme_obj->cfg.reg;
	chan_lst = &rso_cfg->roam_scan_freq_lst;
	/*
	 * Create a Union of occupied channel list learnt by the DUT along
	 * with the Neighbor report Channels. This increases the chances of
	 * the DUT to get a candidate AP while roaming even if the Neighbor
	 * Report is not able to provide sufficient information.
	 */
	if (rso_cfg->occupied_chan_lst.num_chan) {
		cm_roam_merge_channel_lists(rso_cfg->occupied_chan_lst.freq_list,
					    rso_cfg->occupied_chan_lst.num_chan,
					    in_ptr, in_chan_num,
					    &merged_out_chan_num);
		in_chan_num = merged_out_chan_num;
	}

	freq_list = qdf_mem_malloc(CFG_VALID_CHANNEL_LIST_LEN *
				       sizeof(qdf_freq_t));
	if (!freq_list)
		return QDF_STATUS_E_NOMEM;

	if (band == BAND_2G) {
		for (i = 0; i < in_chan_num; i++) {
			if (WLAN_REG_IS_24GHZ_CH_FREQ(in_ptr[i]) &&
			    wlan_roam_is_channel_valid(reg, in_ptr[i])) {
				freq_list[out_num_chan++] = in_ptr[i];
			}
		}
	} else if (band == BAND_5G) {
		for (i = 0; i < in_chan_num; i++) {
			/* Add 5G Non-DFS channel */
			if (WLAN_REG_IS_5GHZ_CH_FREQ(in_ptr[i]) &&
			    wlan_roam_is_channel_valid(reg, in_ptr[i]) &&
			    !wlan_reg_is_dfs_for_freq(pdev, in_ptr[i])) {
				freq_list[out_num_chan++] = in_ptr[i];
			}
		}
	} else if (band == BAND_ALL) {
		for (i = 0; i < in_chan_num; i++) {
			if (wlan_roam_is_channel_valid(reg, in_ptr[i]) &&
			    !wlan_reg_is_dfs_for_freq(pdev, in_ptr[i])) {
				freq_list[out_num_chan++] = in_ptr[i];
			}
		}
	} else {
		mlme_warn("Invalid band, No operation carried out (Band %d)",
			  band);
		qdf_mem_free(freq_list);
		return QDF_STATUS_E_INVAL;
	}

	tmp_chan_freq_list = qdf_mem_malloc(CFG_VALID_CHANNEL_LIST_LEN *
					    sizeof(qdf_freq_t));
	if (!tmp_chan_freq_list) {
		qdf_mem_free(freq_list);
		return QDF_STATUS_E_NOMEM;
	}

	/*
	 * if roaming within band is enabled, then select only the
	 * in band channels .
	 * This is required only if the band capability is set to ALL,
	 * E.g., if band capability is only 2.4G then all the channels in the
	 * list are already filtered for 2.4G channels, hence ignore this check
	 */
	if ((band == BAND_ALL) && mlme_obj->cfg.lfr.roam_intra_band)
		cm_roam_channels_filter_by_current_band(pdev, vdev_id,
							freq_list, out_num_chan,
							tmp_chan_freq_list,
							&out_num_chan);

	/* Prepare final roam scan channel list */
	if (out_num_chan) {
		/* Clear the channel list first */
		cm_flush_roam_channel_list(chan_lst);
		chan_lst->freq_list =
			qdf_mem_malloc(out_num_chan * sizeof(qdf_freq_t));
		if (!chan_lst->freq_list) {
			chan_lst->num_chan = 0;
			status = QDF_STATUS_E_NOMEM;
			goto error;
		}
		for (i = 0; i < out_num_chan; i++)
			chan_lst->freq_list[i] = tmp_chan_freq_list[i];

		chan_lst->num_chan = out_num_chan;
	}

error:
	qdf_mem_free(tmp_chan_freq_list);
	qdf_mem_free(freq_list);

	return status;
}
#endif

static const char *cm_get_config_item_string(uint8_t reason)
{
	switch (reason) {
	CASE_RETURN_STRING(REASON_LOOKUP_THRESH_CHANGED);
	CASE_RETURN_STRING(REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_BMISS_FIRST_BCNT_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_BMISS_FINAL_BCNT_CHANGED);
	default:
		return "unknown";
	}
}

QDF_STATUS cm_neighbor_roam_update_config(struct wlan_objmgr_pdev *pdev,
					  uint8_t vdev_id, uint8_t value,
					  uint8_t reason)
{
	uint8_t old_value;
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;
	struct rso_cfg_params *cfg_params;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return QDF_STATUS_E_FAILURE;
	}
	cfg_params = &rso_cfg->cfg_param;
	switch (reason) {
	case REASON_LOOKUP_THRESH_CHANGED:
		old_value = cfg_params->neighbor_lookup_threshold;
		cfg_params->neighbor_lookup_threshold = value;
		break;
	case REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED:
		old_value = cfg_params->opportunistic_threshold_diff;
		cfg_params->opportunistic_threshold_diff = value;
		break;
	case REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED:
		old_value = cfg_params->roam_rescan_rssi_diff;
		cfg_params->roam_rescan_rssi_diff = value;
		rso_cfg->rescan_rssi_delta = value;
		break;
	case REASON_ROAM_BMISS_FIRST_BCNT_CHANGED:
		old_value = cfg_params->roam_bmiss_first_bcn_cnt;
		cfg_params->roam_bmiss_first_bcn_cnt = value;
		break;
	case REASON_ROAM_BMISS_FINAL_BCNT_CHANGED:
		old_value = cfg_params->roam_bmiss_final_cnt;
		cfg_params->roam_bmiss_final_cnt = value;
		break;
	default:
		mlme_debug("Unknown update cfg reason");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	mlme_debug("CONNECTED, send update cfg cmd");
	wlan_roam_update_cfg(psoc, vdev_id, reason);

	mlme_debug("LFR config for %s changed from %d to %d",
		   cm_get_config_item_string(reason), old_value, value);

	return QDF_STATUS_SUCCESS;
}

void cm_flush_roam_channel_list(struct rso_chan_info *channel_info)
{
	/* Free up the memory first (if required) */
	if (channel_info->freq_list) {
		qdf_mem_free(channel_info->freq_list);
		channel_info->freq_list = NULL;
		channel_info->num_chan = 0;
	}
}

static void
cm_restore_default_roaming_params(struct wlan_mlme_psoc_ext_obj *mlme_obj,
				   struct wlan_objmgr_vdev *vdev)
{
	struct rso_config *rso_cfg;
	struct rso_cfg_params *cfg_params;

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return;
	cfg_params = &rso_cfg->cfg_param;
	cfg_params->enable_scoring_for_roam =
			mlme_obj->cfg.roam_scoring.enable_scoring_for_roam;
	cfg_params->empty_scan_refresh_period =
			mlme_obj->cfg.lfr.empty_scan_refresh_period;
	cfg_params->full_roam_scan_period =
			mlme_obj->cfg.lfr.roam_full_scan_period;
	cfg_params->neighbor_scan_period =
			mlme_obj->cfg.lfr.neighbor_scan_timer_period;
	cfg_params->neighbor_lookup_threshold =
			mlme_obj->cfg.lfr.neighbor_lookup_rssi_threshold;
	cfg_params->roam_rssi_diff =
			mlme_obj->cfg.lfr.roam_rssi_diff;
	cfg_params->roam_rssi_diff_6ghz =
			mlme_obj->cfg.lfr.roam_rssi_diff_6ghz;
	cfg_params->bg_rssi_threshold =
			mlme_obj->cfg.lfr.bg_rssi_threshold;

	cfg_params->max_chan_scan_time =
			mlme_obj->cfg.lfr.neighbor_scan_max_chan_time;
	cfg_params->roam_scan_home_away_time =
			mlme_obj->cfg.lfr.roam_scan_home_away_time;
	cfg_params->roam_scan_n_probes =
			mlme_obj->cfg.lfr.roam_scan_n_probes;
	cfg_params->roam_scan_inactivity_time =
			mlme_obj->cfg.lfr.roam_scan_inactivity_time;
	cfg_params->roam_inactive_data_packet_count =
			mlme_obj->cfg.lfr.roam_inactive_data_packet_count;
	cfg_params->roam_scan_period_after_inactivity =
			mlme_obj->cfg.lfr.roam_scan_period_after_inactivity;
}

QDF_STATUS cm_roam_control_restore_default_config(struct wlan_objmgr_pdev *pdev,
						  uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct rso_chan_info *chan_info;
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;
	struct rso_cfg_params *cfg_params;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		goto out;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		goto out;

	if (!mlme_obj->cfg.lfr.roam_scan_offload_enabled) {
		mlme_err("roam_scan_offload_enabled is not supported");
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		goto out;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		goto out;
	}
	cfg_params = &rso_cfg->cfg_param;
	mlme_obj->cfg.lfr.rso_user_config.roam_scan_control = false;

	chan_info = &cfg_params->pref_chan_info;
	cm_flush_roam_channel_list(chan_info);

	chan_info = &cfg_params->specific_chan_info;
	cm_flush_roam_channel_list(chan_info);

	mlme_reinit_control_config_lfr_params(psoc, &mlme_obj->cfg.lfr);

	cm_restore_default_roaming_params(mlme_obj, vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	if (MLME_IS_ROAM_STATE_RSO_ENABLED(psoc, vdev_id)) {
		cm_roam_send_rso_cmd(psoc, vdev_id,
				     ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				     REASON_FLUSH_CHANNEL_LIST);
		cm_roam_send_rso_cmd(psoc, vdev_id,
				     ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				     REASON_SCORING_CRITERIA_CHANGED);
	}

	status = QDF_STATUS_SUCCESS;
out:
	return status;
}

void cm_update_pmk_cache_ft(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			    struct wlan_crypto_pmksa *pmk_cache)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_crypto_pmksa pmksa;
	enum QDF_OPMODE vdev_mode;
	struct cm_roam_values_copy src_cfg;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev is NULL");
		return;
	}

	vdev_mode = wlan_vdev_mlme_get_opmode(vdev);
	/* If vdev mode is STA then proceed further */
	if (vdev_mode != QDF_STA_MODE) {
		mlme_err("vdev mode is not STA");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}

	/*
	 * In FT connection fetch the MDID from Session or scan result whichever
	 * and send it to crypto so that it will update the crypto PMKSA table
	 * with the MDID for the matching BSSID or SSID PMKSA entry. And delete
	 * the old/stale PMK cache entries for the same mobility domain as of
	 * the newly added entry to avoid multiple PMK cache entries for the
	 * same MDID.
	 */
	wlan_vdev_get_bss_peer_mac(vdev, &pmksa.bssid);
	wlan_vdev_mlme_get_ssid(vdev, pmksa.ssid, &pmksa.ssid_len);
	wlan_cm_roam_cfg_get_value(psoc, vdev_id, MOBILITY_DOMAIN, &src_cfg);

	if (pmk_cache)
		qdf_mem_copy(pmksa.cache_id, pmk_cache->cache_id,
			     WLAN_CACHE_ID_LEN);

	if (src_cfg.bool_value) {
		pmksa.mdid.mdie_present = 1;
		pmksa.mdid.mobility_domain = src_cfg.uint_value;
		mlme_debug("MDID:0x%x copied to PMKSA", src_cfg.uint_value);

		status = wlan_crypto_update_pmk_cache_ft(vdev, &pmksa);
		if (QDF_IS_STATUS_ERROR(status))
			mlme_debug("Failed to update the crypto table");
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

bool cm_lookup_pmkid_using_bssid(struct wlan_objmgr_psoc *psoc,
				 uint8_t vdev_id,
				 struct wlan_crypto_pmksa *pmk_cache)
{
	struct wlan_crypto_pmksa *pmksa;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("Invalid vdev");
		return false;
	}

	pmksa = wlan_crypto_get_pmksa(vdev, &pmk_cache->bssid);
	if (!pmksa) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return false;
	}
	qdf_mem_copy(pmk_cache->pmkid, pmksa->pmkid, sizeof(pmk_cache->pmkid));
	qdf_mem_copy(pmk_cache->pmk, pmksa->pmk, pmksa->pmk_len);
	pmk_cache->pmk_len = pmksa->pmk_len;
	pmk_cache->pmk_lifetime = pmksa->pmk_lifetime;
	pmk_cache->pmk_lifetime_threshold = pmksa->pmk_lifetime_threshold;
	pmk_cache->pmk_entry_ts = pmksa->pmk_entry_ts;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return true;
}

void cm_roam_restore_default_config(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id)
{
	struct cm_roam_values_copy src_config;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	uint32_t roam_trigger_bitmap;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	if (mlme_obj->cfg.lfr.roam_scan_offload_enabled) {
		/*
		 * When vendor handoff is enabled and disconnection is received,
		 * then restore the roam trigger bitmap from the ini
		 * configuration
		 */
		wlan_cm_roam_cfg_get_value(psoc, vdev_id, ROAM_CONFIG_ENABLE,
					   &src_config);
		if (src_config.bool_value) {
			roam_trigger_bitmap =
					wlan_mlme_get_roaming_triggers(psoc);
			mlme_set_roam_trigger_bitmap(psoc, vdev_id,
						     roam_trigger_bitmap);
		}

		src_config.bool_value = 0;
		wlan_cm_roam_cfg_set_value(psoc, vdev_id, ROAM_CONFIG_ENABLE,
					   &src_config);
	}

	cm_roam_control_restore_default_config(pdev, vdev_id);
}

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
void
cm_store_sae_single_pmk_to_global_cache(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev)
{
	struct mlme_pmk_info *pmk_info;
	struct wlan_crypto_pmksa *pmksa;
	struct cm_roam_values_copy src_cfg;
	struct qdf_mac_addr bssid;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	int32_t akm;

	akm = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	wlan_cm_roam_cfg_get_value(psoc, vdev_id,
				   IS_SINGLE_PMK, &src_cfg);
	if (!src_cfg.bool_value ||
	    !QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE))
		return;
	/*
	 * Mark the AP as single PMK capable in Crypto Table
	 */
	wlan_vdev_get_bss_peer_mac(vdev, &bssid);
	wlan_crypto_set_sae_single_pmk_bss_cap(vdev, &bssid, true);

	pmk_info = qdf_mem_malloc(sizeof(*pmk_info));
	if (!pmk_info)
		return;

	wlan_cm_get_psk_pmk(pdev, vdev_id, pmk_info->pmk, &pmk_info->pmk_len);

	pmksa = wlan_crypto_get_pmksa(vdev, &bssid);
	if (pmksa) {
		pmk_info->spmk_timeout_period =
			(pmksa->pmk_lifetime *
			 pmksa->pmk_lifetime_threshold / 100);
		pmk_info->spmk_timestamp = pmksa->pmk_entry_ts;
		mlme_debug("spmk_ts:%ld spmk_timeout_prd:%d secs",
			   pmk_info->spmk_timestamp,
			   pmk_info->spmk_timeout_period);
	} else {
		mlme_debug("PMK entry not found for bss:" QDF_MAC_ADDR_FMT,
			   QDF_MAC_ADDR_REF(bssid.bytes));
	}

	wlan_mlme_update_sae_single_pmk(vdev, pmk_info);

	qdf_mem_zero(pmk_info, sizeof(*pmk_info));
	qdf_mem_free(pmk_info);
}

void cm_check_and_set_sae_single_pmk_cap(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id, uint8_t *psk_pmk,
					 uint8_t pmk_len)
{
	struct wlan_objmgr_vdev *vdev;
	struct mlme_pmk_info *pmk_info;
	struct wlan_crypto_pmksa *pmkid_cache, *roam_sync_pmksa;
	int32_t keymgmt;
	bool lookup_success;
	QDF_STATUS status;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_ZERO_INIT;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("get vdev failed");
		return;
	}
	status = wlan_vdev_get_bss_peer_mac(vdev, &bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to find connected bssid");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}
	keymgmt = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	if (keymgmt < 0) {
		mlme_err("Invalid mgmt cipher");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}

	if (keymgmt & (1 << WLAN_CRYPTO_KEY_MGMT_SAE)) {
		struct cm_roam_values_copy src_cfg;

		wlan_cm_roam_cfg_get_value(psoc, vdev_id, IS_SINGLE_PMK,
					   &src_cfg);
		wlan_mlme_set_sae_single_pmk_bss_cap(psoc, vdev_id,
						     src_cfg.bool_value);
		if (!src_cfg.bool_value)
			goto end;

		roam_sync_pmksa = qdf_mem_malloc(sizeof(*roam_sync_pmksa));
		if (roam_sync_pmksa) {
			qdf_copy_macaddr(&roam_sync_pmksa->bssid, &bssid);
			roam_sync_pmksa->single_pmk_supported = true;
			roam_sync_pmksa->pmk_len = pmk_len;
			qdf_mem_copy(roam_sync_pmksa->pmk, psk_pmk,
				     roam_sync_pmksa->pmk_len);
			mlme_debug("SPMK received for " QDF_MAC_ADDR_FMT "pmk_len:%d",
				QDF_MAC_ADDR_REF(roam_sync_pmksa->bssid.bytes),
				roam_sync_pmksa->pmk_len);
			/* update single pmk info for roamed ap to pmk table */
			wlan_crypto_set_sae_single_pmk_info(vdev,
							    roam_sync_pmksa);

			qdf_mem_zero(roam_sync_pmksa, sizeof(*roam_sync_pmksa));
			qdf_mem_free(roam_sync_pmksa);
		} else {
			goto end;
		}

		pmkid_cache = qdf_mem_malloc(sizeof(*pmkid_cache));
		if (!pmkid_cache)
			goto end;

		qdf_copy_macaddr(&pmkid_cache->bssid, &bssid);
		/*
		 * In SAE single pmk roaming case, there will
		 * be no PMK entry found for the AP in pmk cache.
		 * So if the lookup is successful, then we have done
		 * a FULL sae here. In that case, clear all other
		 * single pmk entries.
		 */
		lookup_success =
			cm_lookup_pmkid_using_bssid(psoc, vdev_id, pmkid_cache);
		if (lookup_success) {
			wlan_crypto_selective_clear_sae_single_pmk_entries(vdev,
					&bssid);

			pmk_info = qdf_mem_malloc(sizeof(*pmk_info));
			if (!pmk_info) {
				qdf_mem_zero(pmkid_cache, sizeof(*pmkid_cache));
				qdf_mem_free(pmkid_cache);
				goto end;
			}

			qdf_mem_copy(pmk_info->pmk, pmkid_cache->pmk,
				     pmkid_cache->pmk_len);
			pmk_info->pmk_len = pmkid_cache->pmk_len;
			pmk_info->spmk_timestamp = pmkid_cache->pmk_entry_ts;
			pmk_info->spmk_timeout_period  =
				(pmkid_cache->pmk_lifetime *
				 pmkid_cache->pmk_lifetime_threshold / 100);

			wlan_mlme_update_sae_single_pmk(vdev, pmk_info);

			qdf_mem_zero(pmk_info, sizeof(*pmk_info));
			qdf_mem_free(pmk_info);
		}

		qdf_mem_zero(pmkid_cache, sizeof(*pmkid_cache));
		qdf_mem_free(pmkid_cache);
	}

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}
#endif

bool cm_is_auth_type_11r(struct wlan_mlme_psoc_ext_obj *mlme_obj,
			 struct wlan_objmgr_vdev *vdev,
			 bool mdie_present)
{
	int32_t akm;

	akm = wlan_crypto_get_param(vdev,
				    WLAN_CRYPTO_PARAM_KEY_MGMT);

	if (cm_is_open_mode(vdev)) {
		if (mdie_present && mlme_obj->cfg.lfr.enable_ftopen)
			return true;
	} else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384) ||
		   QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256) ||
		   QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE) ||
		   QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X) ||
		   QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_PSK) ||
		   QDF_HAS_PARAM(akm,
				 WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384)) {
		return true;
	}

	return false;
}

#ifdef FEATURE_WLAN_ESE
bool
cm_ese_open_present(struct wlan_objmgr_vdev *vdev,
		    struct wlan_mlme_psoc_ext_obj *mlme_obj,
		    bool ese_version_present)
{
	if (cm_is_open_mode(vdev) && ese_version_present &&
	    mlme_obj->cfg.lfr.ese_enabled)
		return true;

	return false;
}

bool
cm_is_ese_connection(struct wlan_objmgr_vdev *vdev, bool ese_version_present)
{
	int32_t akm;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("psoc not found");
		return false;
	}
	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return false;

	if (!mlme_obj->cfg.lfr.ese_enabled)
		return false;

	akm = wlan_crypto_get_param(vdev,
				    WLAN_CRYPTO_PARAM_KEY_MGMT);

	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		return true;

	/*
	 * A profile can not be both ESE and 11R. But an 802.11R AP
	 * may be advertising support for ESE as well. So if we are
	 * associating Open or explicitly ESE then we will get ESE.
	 * If we are associating explicitly 11R only then we will get
	 * 11R.
	 */
	return cm_ese_open_present(vdev, mlme_obj, ese_version_present);
}
#endif

static void cm_roam_start_init(struct wlan_objmgr_psoc *psoc,
			       struct wlan_objmgr_pdev *pdev,
			       struct wlan_objmgr_vdev *vdev)
{
	struct cm_roam_values_copy src_cfg;
	bool mdie_present;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	enum QDF_OPMODE opmode;

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE) {
		mlme_debug("Wrong opmode %d", opmode);
		return;
	}
	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;

	wlan_cm_init_occupied_ch_freq_list(pdev, psoc, vdev_id);

	/*
	 * Update RSSI change params to vdev
	 */
	src_cfg.uint_value = mlme_obj->cfg.lfr.roam_rescan_rssi_diff;
	wlan_cm_roam_cfg_set_value(psoc, vdev_id,
				   RSSI_CHANGE_THRESHOLD, &src_cfg);

	src_cfg.uint_value = mlme_obj->cfg.lfr.roam_scan_hi_rssi_delay;
	wlan_cm_roam_cfg_set_value(psoc, vdev_id,
				   HI_RSSI_DELAY_BTW_SCANS, &src_cfg);

	wlan_cm_update_roam_scan_scheme_bitmap(psoc, vdev_id,
					       DEFAULT_ROAM_SCAN_SCHEME_BITMAP);
	wlan_cm_roam_cfg_get_value(psoc, vdev_id,
				   MOBILITY_DOMAIN, &src_cfg);

	mdie_present = src_cfg.bool_value;
	/* Based on the auth scheme tell if we are 11r */
	if (cm_is_auth_type_11r(mlme_obj, vdev, mdie_present)) {
		src_cfg.bool_value = true;
	} else {
		src_cfg.bool_value = false;
	}
	wlan_cm_roam_cfg_set_value(psoc, vdev_id,
				   IS_11R_CONNECTION, &src_cfg);

	src_cfg.uint_value = mlme_obj->cfg.lfr.roam_rssi_diff_6ghz;
	wlan_cm_roam_cfg_set_value(psoc, vdev_id,
				   ROAM_RSSI_DIFF_6GHZ, &src_cfg);

	if (!mlme_obj->cfg.lfr.roam_scan_offload_enabled)
		return;
	/*
	 * Store the current PMK info of the AP
	 * to the single pmk global cache if the BSS allows
	 * single pmk roaming capable.
	 */
	cm_store_sae_single_pmk_to_global_cache(psoc, pdev, vdev);

	if (!MLME_IS_ROAM_SYNCH_IN_PROGRESS(psoc, vdev_id))
		wlan_cm_roam_state_change(pdev, vdev_id,
					  WLAN_ROAM_RSO_ENABLED,
					  REASON_CTX_INIT);
}

void cm_roam_start_init_on_connect(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d: vdev not found", vdev_id);
		return;
	}
	cm_roam_start_init(psoc, pdev, vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

void cm_update_session_assoc_ie(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id,
				struct element_info *assoc_ie)
{
	struct rso_config *rso_cfg;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL for vdev %d", vdev_id);
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		goto rel_vdev_ref;

	if (rso_cfg->assoc_ie.ptr) {
		qdf_mem_free(rso_cfg->assoc_ie.ptr);
		rso_cfg->assoc_ie.ptr = NULL;
		rso_cfg->assoc_ie.len = 0;
	}
	if (!assoc_ie->len) {
		sme_debug("Assoc IE len 0");
		goto rel_vdev_ref;
	}
	rso_cfg->assoc_ie.ptr = qdf_mem_malloc(assoc_ie->len);
	if (!rso_cfg->assoc_ie.ptr)
		goto rel_vdev_ref;

	rso_cfg->assoc_ie.len = assoc_ie->len;
	qdf_mem_copy(rso_cfg->assoc_ie.ptr, assoc_ie->ptr, assoc_ie->len);
rel_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

QDF_STATUS cm_start_roam_invoke(struct wlan_objmgr_psoc *psoc,
				struct wlan_objmgr_vdev *vdev,
				struct qdf_mac_addr *bssid,
				qdf_freq_t chan_freq,
				enum wlan_cm_source source)
{
	struct cm_req *cm_req;
	QDF_STATUS status;
	uint8_t roam_control_bitmap;
	struct qdf_mac_addr connected_bssid;
	uint8_t vdev_id = vdev->vdev_objmgr.vdev_id;
	bool roam_offload_enabled = cm_roam_offload_enabled(psoc);
	struct rso_config *rso_cfg;

	roam_control_bitmap = mlme_get_operations_bitmap(psoc, vdev_id);
	if (roam_offload_enabled && (roam_control_bitmap ||
	    !MLME_IS_ROAM_INITIALIZED(psoc, vdev_id))) {
		mlme_debug("ROAM: RSO Disabled internaly: vdev[%d] bitmap[0x%x]",
			   vdev_id, roam_control_bitmap);
		return QDF_STATUS_E_FAILURE;
	}

	cm_req = qdf_mem_malloc(sizeof(*cm_req));
	if (!cm_req)
		return QDF_STATUS_E_NOMEM;

	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_err("MLO ROAM: Invalid Roam req on link vdev %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_NULL_VALUE;

	/* Ignore BSSID and channel validation for FW host roam */
	if (source == CM_ROAMING_FW)
		goto send_evt;

	if (qdf_is_macaddr_zero(bssid)) {
		if (!wlan_mlme_is_data_stall_recovery_fw_supported(psoc)) {
			mlme_debug("FW does not support data stall recovery, aborting roam invoke");
			qdf_mem_free(cm_req);
			return QDF_STATUS_E_NOSUPPORT;
		}

		cm_req->roam_req.req.forced_roaming = true;
		if (source == CM_ROAMING_HOST)
			rso_cfg->is_forced_roaming = true;
		source = CM_ROAMING_NUD_FAILURE;
		goto send_evt;
	}

	if (qdf_is_macaddr_broadcast(bssid)) {
		qdf_copy_macaddr(&cm_req->roam_req.req.bssid, bssid);
		qdf_copy_macaddr(&rso_cfg->roam_invoke_bssid, bssid);
		mlme_debug("Roam only if better candidate found else stick to current AP");
		goto send_evt;
	}

	wlan_vdev_get_bss_peer_mac(vdev, &connected_bssid);
	if (qdf_is_macaddr_equal(bssid, &connected_bssid)) {
		mlme_debug("Reassoc BSSID is same as currently associated AP");
		chan_freq = wlan_get_operation_chan_freq(vdev);
	}

	if (!chan_freq || qdf_is_macaddr_zero(bssid)) {
		mlme_debug("bssid " QDF_MAC_ADDR_FMT " chan_freq %d",
			   QDF_MAC_ADDR_REF(bssid->bytes), chan_freq);
		qdf_mem_free(cm_req);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_copy_macaddr(&cm_req->roam_req.req.bssid, bssid);
	cm_req->roam_req.req.chan_freq = chan_freq;

send_evt:
	cm_req->roam_req.req.source = source;

	/* Storing source information in rso cfg as if FW aborts
	 * roam host will delete roam req from queue.
	 * In roam invoke failure, host will read rso cfg params
	 * information and disconnect if needed.
	 */
	if (source == CM_ROAMING_HOST ||
	    source == CM_ROAMING_NUD_FAILURE)
		rso_cfg->roam_invoke_source = source;

	cm_req->roam_req.req.vdev_id = vdev_id;
	/*
	 * For LFR3 WLAN_CM_SM_EV_ROAM_REQ will be converted to
	 * WLAN_CM_SM_EV_ROAM_INVOKE.
	 */
	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_ROAM_REQ,
				     sizeof(*cm_req), cm_req);

	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(cm_req);

	return status;
}

#if defined(WLAN_FEATURE_CONNECTIVITY_LOGGING) && \
    defined(WLAN_FEATURE_ROAM_OFFLOAD)
static
bool wlan_is_valid_frequency(uint32_t freq, uint32_t band_capability,
			     uint32_t band_mask)
{
	if ((band_capability == BIT(REG_BAND_5G) ||
	     band_mask == BIT(REG_BAND_5G) ||
	     band_capability == BIT(REG_BAND_6G) ||
	     band_mask == BIT(REG_BAND_6G)) &&
	     WLAN_REG_IS_24GHZ_CH_FREQ(freq))
		return false;

	if ((band_capability == BIT(REG_BAND_2G) ||
	     band_mask == BIT(REG_BAND_2G)) &&
	     !WLAN_REG_IS_24GHZ_CH_FREQ(freq))
		return false;

	return true;
}

void cm_roam_scan_info_event(struct wlan_objmgr_psoc *psoc,
			     struct wmi_roam_scan_data *scan, uint8_t vdev_id)
{
	int i;
	struct wlan_log_record *log_record = NULL;
	struct wmi_roam_candidate_info *ap = scan->ap;
	uint32_t chan_freq[NUM_CHANNELS];
	uint8_t count = 0, status, num_chan;
	uint32_t band_capability = 0, band_mask = 0;
	bool is_full_scan;
	struct wlan_objmgr_vdev *vdev = NULL;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return;

	log_record->vdev_id = vdev_id;
	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->fw_timestamp_us = (uint64_t)scan->ap->timestamp * 1000;
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->log_subtype = WLAN_ROAM_SCAN_DONE;

	qdf_copy_macaddr(&log_record->bssid, &ap->bssid);

	/*
	 * scan->num_ap includes current connected AP also
	 * so subtract 1 from the count to get total candidate APs
	 */
	if (scan->num_ap)
		log_record->roam_scan.cand_ap_count = scan->num_ap - 1;


	is_full_scan = scan->type & scan->present;

	if (is_full_scan) {
		status = mlme_get_fw_scan_channels(psoc, chan_freq, &num_chan);
		if (QDF_IS_STATUS_ERROR(status))
			goto out;

		status = wlan_mlme_get_band_capability(psoc, &band_capability);
		if (QDF_IS_STATUS_ERROR(status))
			goto out;

		vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
							WLAN_MLME_OBJMGR_ID);
		if (!vdev)
			goto out;

		band_mask =
		policy_mgr_get_connected_roaming_vdev_band_mask(psoc, vdev_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
		for (i = 0; i < num_chan; i++) {
			if (!wlan_is_valid_frequency(chan_freq[i],
						     band_capability,
						     band_mask))
				continue;

			log_record->roam_scan.scan_freq[count] = chan_freq[i];
			count++;
		}

		log_record->roam_scan.num_scanned_freq = count;
	} else {
		if (scan->num_chan > MAX_ROAM_SCAN_CHAN)
			scan->num_chan = MAX_ROAM_SCAN_CHAN;

		log_record->roam_scan.num_scanned_freq = scan->num_chan;
		for (i = 0; i < scan->num_chan; i++)
			log_record->roam_scan.scan_freq[i] = scan->chan_freq[i];
	}

out:
	wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);
}

void cm_roam_trigger_info_event(struct wmi_roam_trigger_info *data,
				struct wmi_roam_scan_data *scan_data,
				uint8_t vdev_id, bool is_full_scan)
{
	struct wlan_log_record *log_record = NULL;
	uint8_t i;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return;

	log_record->vdev_id = vdev_id;
	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->log_subtype = WLAN_ROAM_SCAN_START;
	log_record->roam_trig.trigger_reason  = data->trigger_reason;
	log_record->roam_trig.trigger_sub_reason = data->trigger_sub_reason;

	log_record->roam_trig.current_rssi = 0;
	log_record->roam_trig.cu_load = 0;

	/*
	 * Get the current AP rssi & CU load from the
	 * wmi_roam_ap_info tlv in roam scan results
	 */
	if (scan_data->present) {
		for (i = 0; i < scan_data->num_ap; i++) {
			if (i >= MAX_ROAM_CANDIDATE_AP)
				break;

			if (scan_data->ap[i].type ==
			    WLAN_ROAM_SCAN_CURRENT_AP) {
				log_record->roam_trig.current_rssi =
					(-1) * scan_data->ap[i].rssi;
				log_record->roam_trig.cu_load =
						scan_data->ap[i].cu_load;
				break;
			}
		}
	}

	if (data->trigger_reason == ROAM_TRIGGER_REASON_PERIODIC ||
	    data->trigger_reason == ROAM_TRIGGER_REASON_LOW_RSSI)
		log_record->roam_trig.rssi_threshold =
			(-1) * data->rssi_trig_data.threshold;

	log_record->roam_trig.is_full_scan = is_full_scan;
	log_record->fw_timestamp_us = (uint64_t)data->timestamp * 1000;

	wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);
}

#define ETP_MAX_VALUE 10000000

void cm_roam_candidate_info_event(struct wmi_roam_candidate_info *ap,
				  uint8_t cand_ap_idx)
{
	struct wlan_log_record *log_record = NULL;
	uint32_t etp;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return;

	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->fw_timestamp_us = (uint64_t)ap->timestamp * 1000;
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->ap.is_current_ap = (ap->type == 1);
	if (log_record->ap.is_current_ap)
		log_record->log_subtype = WLAN_ROAM_SCORE_CURR_AP;
	else
		log_record->log_subtype = WLAN_ROAM_SCORE_CAND_AP;

	qdf_copy_macaddr(&log_record->ap.cand_bssid, &ap->bssid);

	log_record->ap.rssi  = (-1) * ap->rssi;
	log_record->ap.cu_load = ap->cu_load;
	log_record->ap.total_score = ap->total_score;
	etp = ap->etp * 1000;

	if (etp > ETP_MAX_VALUE)
		log_record->ap.etp = ETP_MAX_VALUE;
	else
		log_record->ap.etp = etp;

	log_record->ap.idx = cand_ap_idx;
	log_record->ap.freq = ap->freq;

	wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);
}

#define WLAN_ROAM_SCAN_TYPE_PARTIAL_SCAN 0
#define WLAN_ROAM_SCAN_TYPE_FULL_SCAN 1

void cm_roam_result_info_event(struct wlan_objmgr_psoc *psoc,
			       struct wmi_roam_trigger_info *trigger,
			       struct wmi_roam_result *res,
			       struct wmi_roam_scan_data *scan_data,
			       uint8_t vdev_id)
{
	struct wlan_log_record *log_record = NULL;
	struct qdf_mac_addr bssid = {0};
	uint8_t i;
	bool ap_found_in_roam_scan = false, bmiss_skip_full_scan = false;
	bool roam_abort = (res->fail_reason == ROAM_FAIL_REASON_SYNC ||
			   res->fail_reason == ROAM_FAIL_REASON_DISCONNECT ||
			   res->fail_reason == ROAM_FAIL_REASON_HOST ||
			   res->fail_reason ==
					ROAM_FAIL_REASON_INTERNAL_ABORT ||
			   res->fail_reason ==
				ROAM_FAIL_REASON_UNABLE_TO_START_ROAM_HO);
	bool is_full_scan = (scan_data->present &&
			     scan_data->type == WLAN_ROAM_SCAN_TYPE_FULL_SCAN);

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return;

	log_record->vdev_id = vdev_id;
	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->fw_timestamp_us = (uint64_t)res->timestamp * 1000;
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());

	for (i = 0; i < scan_data->num_ap && scan_data->present; i++) {
		if (i >= MAX_ROAM_CANDIDATE_AP)
			break;

		if (scan_data->ap[i].type == WLAN_ROAM_SCAN_ROAMED_AP ||
		    scan_data->ap[i].type == WLAN_ROAM_SCAN_CANDIDATE_AP) {
			ap_found_in_roam_scan = true;
			break;
		}
	}

	log_record->log_subtype = WLAN_ROAM_RESULT;
	log_record->roam_result.roam_fail_reason = res->fail_reason;
	/*
	 * Print ROAM if:
	 * 1. Roaming is successful to AP
	 * 2. Atleast one candidate AP found during scan
	 *
	 * Print NO_ROAM only if:
	 * 1. No candidate AP found(eventhough other APs are found in scan)
	 */
	log_record->roam_result.is_roam_successful =
		(res->status == 0) ||
		(ap_found_in_roam_scan &&
		 res->fail_reason != ROAM_FAIL_REASON_NO_CAND_AP_FOUND);

	for (i = 0; i < scan_data->num_ap; i++) {
		if (i >= MAX_ROAM_CANDIDATE_AP)
			break;

		if (scan_data->ap[i].type ==
			   WLAN_ROAM_SCAN_CURRENT_AP) {
			log_record->bssid = scan_data->ap[i].bssid;
			bssid = scan_data->ap[i].bssid;
			break;
		}
	}

	if (!qdf_is_macaddr_zero(&res->fail_bssid))
		log_record->bssid = res->fail_bssid;

	wlan_connectivity_log_enqueue(log_record);
	qdf_mem_zero(log_record, sizeof(*log_record));

	if (roam_abort) {
		log_record->vdev_id = vdev_id;
		log_record->timestamp_us = qdf_get_time_of_the_day_us();
		log_record->fw_timestamp_us = (uint64_t)res->timestamp * 1000;
		log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());

		log_record->log_subtype = WLAN_ROAM_CANCEL;
		log_record->fw_timestamp_us = log_record->timestamp_us;
		log_record->roam_result.roam_fail_reason = res->fail_reason;

		wlan_connectivity_log_enqueue(log_record);
	}

	/*
	 * When roam trigger reason is Beacon Miss, 2 roam scan
	 * stats TLV will be received with reason as BMISS.
	 * 1. First TLV is for partial roam scan data and
	 * 2. Second TLV is for the full scan data when there is no candidate
	 * found in the partial scan.
	 * When bmiss_skip_full_scan flag is disabled, prints for 1 & 2 will be
	 * seen.
	 * when bmiss_skip_full_scan flag is enabled, only print for 1st TLV
	 * will be seen.
	 *
	 * 1. BMISS_DISCONN event should be triggered only once for BMISS roam
	 * trigger if roam result is failure after full scan TLV is received and
	 * bmiss_skip_full_scan is disabled.
	 *
	 * 2. But if bmiss_skip_full_scan is enabled, then trigger
	 * BMISS_DISCONN event after partial scan TLV is received
	 */
	wlan_mlme_get_bmiss_skip_full_scan_value(psoc, &bmiss_skip_full_scan);
	if (trigger->trigger_reason == ROAM_TRIGGER_REASON_BMISS &&
	    !log_record->roam_result.is_roam_successful &&
	    ((!bmiss_skip_full_scan && is_full_scan) ||
	     (bmiss_skip_full_scan && !is_full_scan)))
		cm_roam_beacon_loss_disconnect_event(psoc, bssid, vdev_id);

	qdf_mem_free(log_record);
}
#endif  /* WLAN_FEATURE_CONNECTIVITY_LOGGING */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static QDF_STATUS
cm_find_roam_candidate(struct wlan_objmgr_pdev *pdev,
		       struct cnx_mgr *cm_ctx,
		       struct cm_roam_req *roam_req,
		       struct roam_invoke_req *roam_invoke_req)
{
	struct scan_filter *filter;
	qdf_list_t *candidate_list;
	uint32_t num_bss = 0;
	qdf_list_node_t *cur_node = NULL;
	struct scan_cache_node *candidate = NULL;

	if (!roam_invoke_req)
		return QDF_STATUS_E_FAILURE;

	if (qdf_is_macaddr_zero(&roam_invoke_req->target_bssid) ||
	    !roam_invoke_req->ch_freq)
		return QDF_STATUS_E_FAILURE;

	filter = qdf_mem_malloc(sizeof(*filter));
	if (!filter)
		return QDF_STATUS_E_NOMEM;

	filter->num_of_bssid = 1;
	qdf_copy_macaddr(&filter->bssid_list[0],
			 &roam_invoke_req->target_bssid);
	filter->num_of_channels = 1;
	filter->chan_freq_list[0] = roam_invoke_req->ch_freq;

	candidate_list = wlan_scan_get_result(pdev, filter);
	if (candidate_list) {
		num_bss = qdf_list_size(candidate_list);
		mlme_debug(CM_PREFIX_FMT "num_entries found %d",
			   CM_PREFIX_REF(roam_req->req.vdev_id,
					 roam_req->cm_id),
					 num_bss);
	}
	qdf_mem_free(filter);

	if (!candidate_list || !qdf_list_size(candidate_list)) {
		if (candidate_list)
			wlan_scan_purge_results(candidate_list);
		mlme_info(CM_PREFIX_FMT "no valid candidate found, num_bss %d",
			  CM_PREFIX_REF(roam_req->req.vdev_id,
					roam_req->cm_id),
					num_bss);

		return QDF_STATUS_E_EMPTY;
	}

	qdf_list_peek_front(candidate_list, &cur_node);
	candidate = qdf_container_of(cur_node,
				     struct scan_cache_node,
				     node);

	roam_invoke_req->frame_len = candidate->entry->raw_frame.len;

	if (!roam_invoke_req->frame_len)
		return QDF_STATUS_E_INVAL;

	roam_invoke_req->frame_buf = qdf_mem_malloc(roam_invoke_req->frame_len);

	if (!roam_invoke_req->frame_buf) {
		roam_invoke_req->frame_len = 0;
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(roam_invoke_req->frame_buf,
		     candidate->entry->raw_frame.ptr,
		     roam_invoke_req->frame_len);

	wlan_scan_purge_results(candidate_list);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_send_roam_invoke_req(struct cnx_mgr *cm_ctx, struct cm_req *req)
{
	QDF_STATUS status;
	struct qdf_mac_addr connected_bssid;
	struct cm_roam_req *roam_req;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct roam_invoke_req *roam_invoke_req = NULL;
	wlan_cm_id cm_id;
	uint8_t vdev_id;
	uint8_t enable_self_bss_roam = false;

	if (!req)
		return QDF_STATUS_E_FAILURE;

	roam_req = &req->roam_req;
	cm_id = req->cm_id;
	vdev_id = roam_req->req.vdev_id;

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto roam_err;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto roam_err;
	}

	if (wlan_vdev_mlme_get_is_mlo_link(psoc, vdev_id)) {
		mlme_debug("MLO ROAM: skip RSO cmd for link vdev %d", vdev_id);
		status = QDF_STATUS_E_FAILURE;
		goto roam_err;
	}

	wlan_vdev_get_bss_peer_mac(cm_ctx->vdev, &connected_bssid);
	wlan_mlme_get_self_bss_roam(psoc, &enable_self_bss_roam);
	if (!enable_self_bss_roam &&
	    qdf_is_macaddr_equal(&roam_req->req.bssid, &connected_bssid)) {
		mlme_err(CM_PREFIX_FMT "self bss roam disabled",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto roam_err;
	}

	roam_invoke_req = qdf_mem_malloc(sizeof(*roam_invoke_req));
	if (!roam_invoke_req) {
		status = QDF_STATUS_E_NOMEM;
		goto roam_err;
	}

	roam_invoke_req->vdev_id = vdev_id;
	if (roam_req->req.forced_roaming) {
		roam_invoke_req->forced_roaming = true;
		goto send_cmd;
	}

	if (qdf_is_macaddr_broadcast(&roam_req->req.bssid)) {
		qdf_copy_macaddr(&roam_invoke_req->target_bssid,
				 &roam_req->req.bssid);
		goto send_cmd;
	}
	if (qdf_is_macaddr_equal(&roam_req->req.bssid, &connected_bssid))
		roam_invoke_req->is_same_bssid = true;

	qdf_copy_macaddr(&roam_invoke_req->target_bssid, &roam_req->req.bssid);
	roam_invoke_req->ch_freq = roam_req->req.chan_freq;

	status = cm_find_roam_candidate(pdev, cm_ctx, roam_req,
					roam_invoke_req);

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "No Candidate found",
			 CM_PREFIX_REF(vdev_id, cm_id));
		goto roam_err;
	}

	if (wlan_cm_get_ese_assoc(pdev, vdev_id)) {
		mlme_debug(CM_PREFIX_FMT "Beacon is not required for ESE",
			   CM_PREFIX_REF(vdev_id, cm_id));
		if (roam_invoke_req->frame_len) {
			qdf_mem_free(roam_invoke_req->frame_buf);
			roam_invoke_req->frame_buf = NULL;
			roam_invoke_req->frame_len = 0;
		}
	}
send_cmd:
	status = wlan_cm_tgt_send_roam_invoke_req(psoc, roam_invoke_req);

roam_err:
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_debug(CM_PREFIX_FMT "fail to send roam invoke req",
			   CM_PREFIX_REF(vdev_id, cm_id));
		status = cm_sm_deliver_event_sync(cm_ctx,
						  WLAN_CM_SM_EV_ROAM_INVOKE_FAIL,
						  sizeof(wlan_cm_id),
						  &cm_id);
		if (QDF_IS_STATUS_ERROR(status))
			cm_remove_cmd(cm_ctx, &cm_id);
	}

	if (roam_invoke_req) {
		if (roam_invoke_req->frame_len)
			qdf_mem_free(roam_invoke_req->frame_buf);
		qdf_mem_free(roam_invoke_req);
	}

	return status;
}

bool cm_roam_offload_enabled(struct wlan_objmgr_psoc *psoc)
{
	bool val;

	wlan_mlme_get_roaming_offload(psoc, &val);

	return val;
}

#ifdef WLAN_FEATURE_CONNECTIVITY_LOGGING
static enum wlan_main_tag
cm_roam_get_tag(enum mgmt_subtype subtype, bool is_tx)
{
	switch (subtype) {
	case MGMT_SUBTYPE_ASSOC_REQ:
		return WLAN_ASSOC_REQ;
	case MGMT_SUBTYPE_ASSOC_RESP:
		return WLAN_ASSOC_RSP;
	case MGMT_SUBTYPE_REASSOC_REQ:
		return WLAN_ASSOC_REQ;
	case MGMT_SUBTYPE_REASSOC_RESP:
		return WLAN_ASSOC_RSP;
	case MGMT_SUBTYPE_DISASSOC:
		if (is_tx)
			return WLAN_DISASSOC_TX;
		else
			return WLAN_DISASSOC_RX;
		break;
	case MGMT_SUBTYPE_AUTH:
		if (is_tx)
			return WLAN_AUTH_REQ;
		else
			return WLAN_AUTH_RESP;
		break;
	case MGMT_SUBTYPE_DEAUTH:
		if (is_tx)
			return WLAN_DEAUTH_TX;
		else
			return WLAN_DEAUTH_RX;
	default:
		break;
	}

	return WLAN_TAG_MAX;
}

static enum wlan_main_tag
cm_roam_get_eapol_tag(enum wlan_roam_frame_subtype subtype)
{
	switch (subtype) {
	case ROAM_FRAME_SUBTYPE_M1:
		return WLAN_EAPOL_M1;
	case ROAM_FRAME_SUBTYPE_M2:
		return WLAN_EAPOL_M2;
	case ROAM_FRAME_SUBTYPE_M3:
		return WLAN_EAPOL_M3;
	case ROAM_FRAME_SUBTYPE_M4:
		return WLAN_EAPOL_M4;
	case ROAM_FRAME_SUBTYPE_GTK_M1:
		return WLAN_GTK_M1;
	case ROAM_FRAME_SUBTYPE_GTK_M2:
		return WLAN_GTK_M2;
	default:
		break;
	}

	return WLAN_TAG_MAX;
}

QDF_STATUS
cm_roam_btm_query_event(struct wmi_neighbor_report_data *btm_data,
			uint8_t vdev_id)
{
	struct wlan_log_record *log_record = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return QDF_STATUS_E_NOMEM;

	log_record->log_subtype = WLAN_BTM_QUERY;
	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->fw_timestamp_us = (uint64_t)btm_data->timestamp * 1000;
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->vdev_id = vdev_id;
	log_record->btm_info.token = btm_data->btm_query_token;
	log_record->btm_info.reason = btm_data->btm_query_reason;

	status = wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);

	return status;
}

#define WTC_BTM_RESPONSE_SUBCODE 0xFF
static void
cm_roam_wtc_btm_event(struct wmi_roam_trigger_info *trigger_info,
		      struct roam_btm_response_data *btm_data,
		      uint8_t vdev_id, bool is_wtc)
{
	struct wlan_log_record *log_record = NULL;
	struct wmi_roam_wtc_btm_trigger_data *wtc_data =
			&trigger_info->wtc_btm_trig_data;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return;

	log_record->log_subtype = WLAN_ROAM_WTC;

	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->fw_timestamp_us = (uint64_t)trigger_info->timestamp * 1000;
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->vdev_id = vdev_id;
	if (is_wtc) {
		log_record->btm_info.reason = wtc_data->vsie_trigger_reason;
		log_record->btm_info.sub_reason = wtc_data->sub_code;
		log_record->btm_info.wtc_duration = wtc_data->duration;
	} else {
		if (!btm_data) {
			qdf_mem_free(log_record);
			return;
		}

		log_record->btm_info.reason = btm_data->vsie_reason;
		log_record->btm_info.sub_reason = WTC_BTM_RESPONSE_SUBCODE;
	}

	wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);
}

QDF_STATUS
cm_roam_btm_resp_event(struct wmi_roam_trigger_info *trigger_info,
		       struct roam_btm_response_data *btm_data,
		       uint8_t vdev_id, bool is_wtc)
{
	struct wlan_log_record *log_record = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (is_wtc) {
		cm_roam_wtc_btm_event(trigger_info, btm_data, vdev_id, is_wtc);
	} else {
		if (!btm_data) {
			mlme_err("vdev_id:%d btm data is NULL", vdev_id);
			return QDF_STATUS_E_FAILURE;
		}

		if (btm_data->vsie_reason)
			cm_roam_wtc_btm_event(trigger_info, btm_data,
					      vdev_id, is_wtc);

		log_record = qdf_mem_malloc(sizeof(*log_record));
		if (!log_record)
			return QDF_STATUS_E_NOMEM;

		log_record->log_subtype = WLAN_BTM_RESP;
		log_record->timestamp_us = qdf_get_time_of_the_day_us();
		log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
		log_record->fw_timestamp_us =
			(uint64_t)trigger_info->timestamp * 1000;
		log_record->vdev_id = vdev_id;

		log_record->btm_info.token =
				btm_data->btm_resp_dialog_token;
		log_record->btm_info.btm_status_code =
				btm_data->btm_status;
		log_record->btm_info.btm_delay = btm_data->btm_delay;
		log_record->btm_info.target_bssid =
				btm_data->target_bssid;
		status = wlan_connectivity_log_enqueue(log_record);
		qdf_mem_free(log_record);

	}

	return status;
}

/**
 * cm_roam_btm_candidate_event()  - Send BTM roam candidate logging event
 * @btm_data: BTM data
 * @vdev_id: Vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_roam_btm_candidate_event(struct wmi_btm_req_candidate_info *btm_data,
			    uint8_t vdev_id, uint8_t idx)
{
	struct wlan_log_record *log_record = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return QDF_STATUS_E_NOMEM;

	log_record->log_subtype = WLAN_BTM_REQ_CANDI;
	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->fw_timestamp_us = (uint64_t)btm_data->timestamp * 1000;
	log_record->vdev_id = vdev_id;
	log_record->btm_cand.preference = btm_data->preference;
	log_record->btm_cand.bssid = btm_data->candidate_bssid;
	log_record->btm_cand.idx = idx;

	status = wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);

	return status;
}

QDF_STATUS
cm_roam_btm_req_event(struct wmi_roam_btm_trigger_data *btm_data,
		      uint8_t vdev_id)
{
	struct wlan_log_record *log_record = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return QDF_STATUS_E_NOMEM;

	log_record->log_subtype = WLAN_BTM_REQ;
	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->fw_timestamp_us = (uint64_t)btm_data->timestamp * 1000;
	log_record->vdev_id = vdev_id;

	log_record->btm_info.token = btm_data->token;
	log_record->btm_info.mode = btm_data->btm_request_mode;
	/*
	 * Diassoc Timer and Validity interval are in secs in the frame
	 * firmware sends it in millisecs to the host.
	 * Send it in secs to the userspace.
	 */
	log_record->btm_info.disassoc_timer =
			btm_data->disassoc_timer / 1000;
	log_record->btm_info.validity_timer =
			btm_data->validity_interval / 1000;
	log_record->btm_info.candidate_list_count =
				btm_data->candidate_list_count;

	status = wlan_connectivity_log_enqueue(log_record);
	for (i = 0; i < log_record->btm_info.candidate_list_count; i++)
		cm_roam_btm_candidate_event(&btm_data->btm_cand[i], vdev_id, i);

	qdf_mem_free(log_record);

	return status;
}

QDF_STATUS
cm_roam_mgmt_frame_event(struct roam_frame_info *frame_data,
			 struct wmi_roam_scan_data *scan_data, uint8_t vdev_id)
{
	struct wlan_log_record *log_record = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i;

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record)
		return QDF_STATUS_E_NOMEM;

	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->fw_timestamp_us = (uint64_t)frame_data->timestamp * 1000;
	log_record->bssid = frame_data->bssid;
	log_record->vdev_id = vdev_id;

	log_record->pkt_info.seq_num = frame_data->seq_num;
	log_record->pkt_info.auth_algo = frame_data->auth_algo;
	log_record->pkt_info.rssi = (-1) * frame_data->rssi;
	log_record->pkt_info.tx_status = frame_data->tx_status;
	log_record->pkt_info.frame_status_code = frame_data->status_code;

	if (scan_data->present) {
		for (i = 0; i < scan_data->num_ap; i++) {
			if (i >= MAX_ROAM_CANDIDATE_AP)
				break;

			if (scan_data->ap[i].type == WLAN_ROAM_SCAN_ROAMED_AP) {
				log_record->pkt_info.rssi =
					(-1) * scan_data->ap[i].rssi;
				log_record->bssid = scan_data->ap[i].bssid;
				break;
			} else if (qdf_is_macaddr_equal(&log_record->bssid,
							&scan_data->ap[i].bssid)) {
				log_record->pkt_info.rssi =
					(-1) * scan_data->ap[i].rssi;
				break;
			}
		}
	}

	if (frame_data->type == ROAM_FRAME_INFO_FRAME_TYPE_EXT)
		log_record->log_subtype =
			cm_roam_get_eapol_tag(frame_data->subtype);
	else
		log_record->log_subtype = cm_roam_get_tag(frame_data->subtype,
							  !frame_data->is_rsp);

	status = wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);

	return status;
}

QDF_STATUS
cm_roam_beacon_loss_disconnect_event(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr bssid, uint8_t vdev_id)
{
	struct wlan_log_record *log_record = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev = NULL;
	uint32_t rssi;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("Vdev[%d] is null", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	log_record = qdf_mem_malloc(sizeof(*log_record));
	if (!log_record) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return QDF_STATUS_E_NOMEM;
	}

	log_record->timestamp_us = qdf_get_time_of_the_day_us();
	log_record->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	log_record->vdev_id = vdev_id;
	log_record->bssid = bssid;
	log_record->log_subtype = WLAN_DISCONN_BMISS;

	rssi = mlme_get_hb_ap_rssi(vdev);
	log_record->pkt_info.rssi = rssi;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	status = wlan_connectivity_log_enqueue(log_record);
	qdf_mem_free(log_record);

	return status;
}
#endif /* WLAN_FEATURE_CONNECTIVITY_LOGGING */

QDF_STATUS
cm_send_rso_stop(struct wlan_objmgr_vdev *vdev)
{
	bool send_resp = true, start_timer;

	if (!vdev) {
		mlme_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	start_timer = cm_roam_offload_enabled(wlan_vdev_get_psoc(vdev));

	cm_roam_state_change(wlan_vdev_get_pdev(vdev), wlan_vdev_get_id(vdev),
			     WLAN_ROAM_RSO_STOPPED, REASON_DRIVER_DISABLED,
			     &send_resp, start_timer);
	/*
	 * RSO stop resp is not supported or RSO STOP timer/req failed,
	 * then send resp from here
	 */
	if (send_resp)
		wlan_cm_rso_stop_continue_disconnect(wlan_vdev_get_psoc(vdev),
						     wlan_vdev_get_id(vdev),
						     false);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_roam_send_ho_delay_config(struct wlan_objmgr_psoc *psoc,
			     uint8_t vdev_id, uint16_t param_value)
{
	QDF_STATUS status;

	wlan_cm_roam_set_ho_delay_config(psoc, param_value);
	status = wlan_cm_tgt_send_roam_ho_delay_config(psoc,
						       vdev_id, param_value);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send roam HO delay config");

	return status;
}
#endif  /* WLAN_FEATURE_ROAM_OFFLOAD */
