/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 *  DOC:    wma_scan_roam.c
 *  This file contains functions related to scan and
 *  roaming functionality.
 */

/* Header files */

#include "wma.h"
#include "wma_api.h"
#include "cds_api.h"
#include "wmi_unified_api.h"
#include "wlan_qct_sys.h"
#include "wni_api.h"
#include "ani_global.h"
#include "wmi_unified.h"
#include "wni_cfg.h"
#include <cdp_txrx_peer_ops.h>
#include <cdp_txrx_cfg.h>

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"
#include "wlan_policy_mgr_api.h"
#include <wlan_utility.h>

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "csr_api.h"
#include "ol_fw.h"

#include "wma_internal.h"
#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif
#include "wlan_reg_services_api.h"
#include "wlan_roam_debug.h"
#include "wlan_mlme_public_struct.h"

/* This is temporary, should be removed */
#include "ol_htt_api.h"
#include <cdp_txrx_handle.h>
#include "wma_he.h"
#include <wlan_scan_public_structs.h>
#include <wlan_scan_ucfg_api.h>
#include "wma_nan_datapath.h"
#include "wlan_mlme_api.h"
#include <wlan_mlme_main.h>
#include <wlan_crypto_global_api.h>
#include <cdp_txrx_mon.h>

#ifdef FEATURE_WLAN_EXTSCAN
#define WMA_EXTSCAN_CYCLE_WAKE_LOCK_DURATION WAKELOCK_DURATION_RECOMMENDED

/*
 * Maximum number of entires that could be present in the
 * WMI_EXTSCAN_HOTLIST_MATCH_EVENT buffer from the firmware
 */
#define WMA_EXTSCAN_MAX_HOTLIST_ENTRIES 10
#endif

static inline wmi_host_channel_width
wma_map_phy_ch_bw_to_wmi_channel_width(enum phy_ch_width ch_width)
{
	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		return WMI_HOST_CHAN_WIDTH_20;
	case CH_WIDTH_40MHZ:
		return WMI_HOST_CHAN_WIDTH_40;
	case CH_WIDTH_80MHZ:
		return WMI_HOST_CHAN_WIDTH_80;
	case CH_WIDTH_160MHZ:
		return WMI_HOST_CHAN_WIDTH_160;
	case CH_WIDTH_5MHZ:
		return WMI_HOST_CHAN_WIDTH_5;
	case CH_WIDTH_10MHZ:
		return WMI_HOST_CHAN_WIDTH_10;
	default:
		return WMI_HOST_CHAN_WIDTH_20;
	}
}

#define WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ      0
#define WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ         1
#define WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ        2
#define WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ 3

/**
 * wma_update_channel_list() - update channel list
 * @handle: wma handle
 * @chan_list: channel list
 *
 * Function is used to update the support channel list in fw.
 *
 * Return: QDF status
 */
QDF_STATUS wma_update_channel_list(WMA_HANDLE handle,
				   tSirUpdateChanList *chan_list)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int i, len;
	struct scan_chan_list_params *scan_ch_param;
	struct channel_param *chan_p;
	struct ch_params ch_params;

	len = sizeof(struct channel_param) * chan_list->numChan +
		offsetof(struct scan_chan_list_params, ch_param[0]);
	scan_ch_param = qdf_mem_malloc(len);
	if (!scan_ch_param)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_zero(scan_ch_param, len);
	WMA_LOGD("no of channels = %d", chan_list->numChan);
	chan_p = &scan_ch_param->ch_param[0];
	scan_ch_param->nallchans = chan_list->numChan;
	scan_ch_param->max_bw_support_present = true;
	wma_handle->saved_chan.num_channels = chan_list->numChan;
	WMA_LOGD("ht %d, vht %d, vht_24 %d", chan_list->ht_en,
			chan_list->vht_en, chan_list->vht_24_en);

	for (i = 0; i < chan_list->numChan; ++i) {
		chan_p->mhz = chan_list->chanParam[i].freq;
		chan_p->cfreq1 = chan_p->mhz;
		chan_p->cfreq2 = 0;
		wma_handle->saved_chan.ch_freq_list[i] =
					chan_list->chanParam[i].freq;

		WMA_LOGD("chan[%d] = freq:%u DFS:%d tx power:%d",
			 i, chan_p->mhz,
			 chan_list->chanParam[i].dfsSet,
			 chan_list->chanParam[i].pwr);

		if (chan_list->chanParam[i].dfsSet) {
			chan_p->is_chan_passive = 1;
			chan_p->dfs_set = 1;
		}

		if (chan_p->mhz < WMA_2_4_GHZ_MAX_FREQ) {
			chan_p->phy_mode = MODE_11G;
			if (chan_list->vht_en && chan_list->vht_24_en)
				chan_p->allow_vht = 1;
		} else {
			chan_p->phy_mode = MODE_11A;
			if (chan_list->vht_en)
				chan_p->allow_vht = 1;
		}

		if (chan_list->ht_en)
			chan_p->allow_ht = 1;

		if (chan_list->he_en)
			chan_p->allow_he = 1;

		if (chan_list->chanParam[i].half_rate)
			chan_p->half_rate = 1;
		else if (chan_list->chanParam[i].quarter_rate)
			chan_p->quarter_rate = 1;

		if (wlan_reg_is_6ghz_psc_chan_freq(
			    chan_p->mhz))
			chan_p->psc_channel = 1;

		/*TODO: Set WMI_SET_CHANNEL_MIN_POWER */
		/*TODO: Set WMI_SET_CHANNEL_ANTENNA_MAX */
		/*TODO: WMI_SET_CHANNEL_REG_CLASSID */
		chan_p->maxregpower = chan_list->chanParam[i].pwr;

		ch_params.ch_width = CH_WIDTH_160MHZ;
		wlan_reg_set_channel_params_for_freq(wma_handle->pdev,
						     chan_p->mhz, 0,
						     &ch_params);

		chan_p->max_bw_supported =
		     wma_map_phy_ch_bw_to_wmi_channel_width(ch_params.ch_width);
		chan_p++;
	}

	qdf_status = wmi_unified_scan_chan_list_cmd_send(wma_handle->wmi_handle,
				scan_ch_param);

	if (QDF_IS_STATUS_ERROR(qdf_status))
		WMA_LOGE("Failed to send WMI_SCAN_CHAN_LIST_CMDID");

	qdf_mem_free(scan_ch_param);

	return qdf_status;
}

QDF_STATUS wma_roam_scan_mawc_params(tp_wma_handle wma_handle,
		struct roam_offload_scan_req *roam_req)
{
	struct wmi_mawc_roam_params *params;
	QDF_STATUS status;

	if (!roam_req) {
		WMA_LOGE("No MAWC parameters to send");
		return QDF_STATUS_E_INVAL;
	}
	params = qdf_mem_malloc(sizeof(*params));
	if (!params)
		return QDF_STATUS_E_NOMEM;

	params->vdev_id = roam_req->sessionId;
	params->enable = roam_req->mawc_roam_params.mawc_enabled &&
		roam_req->mawc_roam_params.mawc_roam_enabled;
	params->traffic_load_threshold =
		roam_req->mawc_roam_params.mawc_roam_traffic_threshold;
	if (wmi_service_enabled(wma_handle->wmi_handle,
				wmi_service_hw_db2dbm_support))
		params->best_ap_rssi_threshold =
			roam_req->mawc_roam_params.mawc_roam_ap_rssi_threshold;
	else
		params->best_ap_rssi_threshold =
			roam_req->mawc_roam_params.mawc_roam_ap_rssi_threshold -
			WMA_NOISE_FLOOR_DBM_DEFAULT;
	params->rssi_stationary_high_adjust =
		roam_req->mawc_roam_params.mawc_roam_rssi_high_adjust;
	params->rssi_stationary_low_adjust =
		roam_req->mawc_roam_params.mawc_roam_rssi_low_adjust;
	status = wmi_unified_roam_mawc_params_cmd(
			wma_handle->wmi_handle, params);
	qdf_mem_free(params);

	return status;
}
#ifdef WLAN_FEATURE_FILS_SK
/**
 * wma_roam_scan_fill_fils_params() - API to fill FILS params in RSO command
 * @wma_handle: WMA handle
 * @params: Pointer to destination RSO params to be filled
 * @roam_req: Pointer to RSO params from CSR
 *
 * Return: None
 */
static void wma_roam_scan_fill_fils_params(tp_wma_handle wma_handle,
					   struct roam_offload_scan_params
					   *params, struct roam_offload_scan_req
					   *roam_req)
{
	struct roam_fils_params *dst_fils_params, *src_fils_params;

	if (!params || !roam_req || !roam_req->is_fils_connection) {
		wma_debug("Invalid input");
		return;
	}

	src_fils_params = &roam_req->roam_fils_params;
	dst_fils_params = &params->roam_fils_params;

	params->add_fils_tlv = true;

	dst_fils_params->username_length = src_fils_params->username_length;
	qdf_mem_copy(dst_fils_params->username, src_fils_params->username,
		     dst_fils_params->username_length);

	dst_fils_params->next_erp_seq_num = src_fils_params->next_erp_seq_num;
	dst_fils_params->rrk_length = src_fils_params->rrk_length;
	qdf_mem_copy(dst_fils_params->rrk, src_fils_params->rrk,
		     dst_fils_params->rrk_length);

	dst_fils_params->rik_length = src_fils_params->rik_length;
	qdf_mem_copy(dst_fils_params->rik, src_fils_params->rik,
		     dst_fils_params->rik_length);

	dst_fils_params->fils_ft_len = src_fils_params->fils_ft_len;
	qdf_mem_copy(dst_fils_params->fils_ft, src_fils_params->fils_ft,
		     dst_fils_params->fils_ft_len);

	dst_fils_params->realm_len = src_fils_params->realm_len;
	qdf_mem_copy(dst_fils_params->realm, src_fils_params->realm,
		     dst_fils_params->realm_len);
}
#else
static inline void wma_roam_scan_fill_fils_params(
					tp_wma_handle wma_handle,
					struct roam_offload_scan_params *params,
					struct roam_offload_scan_req *roam_req)

{ }
#endif

/**
 * wma_roam_scan_offload_set_params() - Set roam scan offload params
 * @wma_handle: pointer to wma context
 * @params: pointer to roam scan offload params
 * @roam_req: roam request param
 *
 * Get roam scan offload parameters from roam req and prepare to fill
 * WMI_ROAM_SCAN_MODE TLV
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void wma_roam_scan_offload_set_params(
				tp_wma_handle wma_handle,
				struct roam_offload_scan_params *params,
				struct roam_offload_scan_req *roam_req)
{
	params->auth_mode = WMI_AUTH_NONE;
	if (!roam_req)
		return;
	params->auth_mode = e_csr_auth_type_to_rsn_authmode
				(roam_req->ConnectedNetwork.authentication,
				 roam_req->ConnectedNetwork.encryption);
	WMA_LOGD("%s : auth mode = %d", __func__, params->auth_mode);

	params->roam_offload_enabled = roam_req->roam_offload_enabled;
	params->roam_offload_params.ho_delay_for_rx =
				roam_req->ho_delay_for_rx;
	params->roam_offload_params.roam_preauth_retry_count =
				roam_req->roam_preauth_retry_count;
	params->roam_offload_params.roam_preauth_no_ack_timeout =
				roam_req->roam_preauth_no_ack_timeout;
	params->prefer_5ghz = roam_req->Prefer5GHz;
	params->roam_rssi_cat_gap = roam_req->RoamRssiCatGap;
	params->select_5ghz_margin = roam_req->Select5GHzMargin;
	params->reassoc_failure_timeout =
				roam_req->ReassocFailureTimeout;
	params->rokh_id_length = roam_req->R0KH_ID_Length;
	qdf_mem_copy(params->rokh_id, roam_req->R0KH_ID,
		     WMI_ROAM_R0KH_ID_MAX_LEN);
	qdf_mem_copy(params->krk, roam_req->KRK, WMI_KRK_KEY_LEN);
	qdf_mem_copy(params->btk, roam_req->BTK, WMI_BTK_KEY_LEN);
	qdf_mem_copy(params->psk_pmk, roam_req->PSK_PMK,
		     roam_req->pmk_len);
	params->pmk_len = roam_req->pmk_len;
	params->roam_key_mgmt_offload_enabled =
				roam_req->RoamKeyMgmtOffloadEnabled;
	wma_roam_scan_fill_self_caps(wma_handle,
				     &params->roam_offload_params, roam_req);
	params->fw_okc = roam_req->pmkid_modes.fw_okc;
	params->fw_pmksa_cache = roam_req->pmkid_modes.fw_pmksa_cache;
	params->rct_validity_timer = roam_req->rct_validity_timer;
	params->is_adaptive_11r = roam_req->is_adaptive_11r_connection;
	WMA_LOGD(FL("qos_caps: %d, qos_enabled: %d, ho_delay_for_rx: %d, roam_scan_mode: %d"),
		 params->roam_offload_params.qos_caps,
		 params->roam_offload_params.qos_enabled,
		 params->roam_offload_params.ho_delay_for_rx, params->mode);
	WMA_LOGD(FL("roam_preauth_retry_count: %d, roam_preauth_no_ack_timeout: %d"),
		 params->roam_offload_params.roam_preauth_retry_count,
		 params->roam_offload_params.roam_preauth_no_ack_timeout);
}
#else
static void wma_roam_scan_offload_set_params(
				tp_wma_handle wma_handle,
				struct roam_offload_scan_params *params,
				struct roam_offload_scan_req *roam_req)
{}
#endif

/**
 * wma_roam_scan_offload_mode() - send roam scan mode request to fw
 * @wma_handle: pointer to wma context
 * @scan_cmd_fp: start scan command ptr
 * @roam_req: roam request param
 * @mode: mode
 * @vdev_id: vdev id
 *
 * send WMI_ROAM_SCAN_MODE TLV to firmware. It has a piggyback
 * of WMI_ROAM_SCAN_MODE.
 *
 * Return: QDF status
 */
QDF_STATUS wma_roam_scan_offload_mode(tp_wma_handle wma_handle,
				      wmi_start_scan_cmd_fixed_param *
				      scan_cmd_fp,
				      struct roam_offload_scan_req *roam_req,
				      uint32_t mode, uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct roam_offload_scan_params *params =
				qdf_mem_malloc(sizeof(*params));

	if (!params)
		return QDF_STATUS_E_NOMEM;

	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__, vdev_id);
		qdf_mem_free(params);
		return QDF_STATUS_E_FAILURE;
	}

	params->is_roam_req_valid = 0;
	params->mode = mode;
	params->vdev_id = vdev_id;
	if (roam_req) {
		params->is_roam_req_valid = 1;
		params->min_delay_btw_roam_scans =
				roam_req->min_delay_btw_roam_scans;
		params->roam_trigger_reason_bitmask =
				roam_req->roam_trigger_reason_bitmask;
		params->is_ese_assoc = roam_req->IsESEAssoc;
		params->is_11r_assoc = roam_req->is_11r_assoc;
		params->mdid = roam_req->mdid;
		params->assoc_ie_length = roam_req->assoc_ie.length;
		qdf_mem_copy(params->assoc_ie, roam_req->assoc_ie.addIEdata,
						roam_req->assoc_ie.length);
		/* Configure roaming scan behavior (DBS/Non-DBS scan) */
		if (roam_req->roaming_scan_policy)
			scan_cmd_fp->scan_ctrl_flags_ext |=
					WMI_SCAN_DBS_POLICY_FORCE_NONDBS;
		else
			scan_cmd_fp->scan_ctrl_flags_ext |=
					WMI_SCAN_DBS_POLICY_DEFAULT;

		wma_roam_scan_fill_fils_params(wma_handle, params, roam_req);
	}
	WMA_LOGD(FL("min_delay_btw_roam_scans:%d, roam_tri_reason_bitmask:%d"),
		 params->min_delay_btw_roam_scans,
		 params->roam_trigger_reason_bitmask);

	wma_roam_scan_offload_set_params(
				wma_handle,
				params,
				roam_req);

	status = wmi_unified_roam_scan_offload_mode_cmd(wma_handle->wmi_handle,
				scan_cmd_fp, params);
	qdf_mem_zero(params, sizeof(*params));
	qdf_mem_free(params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	WMA_LOGD("%s: WMA --> WMI_ROAM_SCAN_MODE", __func__);
	return status;
}

QDF_STATUS
wma_roam_scan_offload_rssi_thresh(tp_wma_handle wma_handle,
				  struct roam_offload_scan_req *roam_req)
{
	struct roam_offload_scan_rssi_params params = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int rssi_thresh, rssi_thresh_diff;
	struct roam_ext_params *roam_params;
	int32_t good_rssi_threshold;
	uint32_t hirssi_scan_max_count;
	uint32_t hirssi_scan_delta;
	int32_t hirssi_upper_bound;
	bool db2dbm_enabled;

	/* Send rssi threshold */
	roam_params = &roam_req->roam_params;
	db2dbm_enabled = wmi_service_enabled(wma_handle->wmi_handle,
					     wmi_service_hw_db2dbm_support);
	if (db2dbm_enabled) {
		rssi_thresh = roam_req->LookupThreshold;
	} else {
		rssi_thresh = roam_req->LookupThreshold -
					WMA_NOISE_FLOOR_DBM_DEFAULT;
		rssi_thresh &= 0x000000ff;
	}
	rssi_thresh_diff = roam_req->OpportunisticScanThresholdDiff;
	hirssi_scan_max_count = roam_req->hi_rssi_scan_max_count;
	hirssi_scan_delta = roam_req->hi_rssi_scan_rssi_delta;
	if (db2dbm_enabled) {
		hirssi_upper_bound = roam_req->hi_rssi_scan_rssi_ub;
	} else {
		hirssi_upper_bound = roam_req->hi_rssi_scan_rssi_ub -
				     WMA_NOISE_FLOOR_DBM_DEFAULT;
	}

	/* fill in threshold values */
	params.vdev_id = roam_req->sessionId;
	params.rssi_thresh = rssi_thresh;
	params.rssi_thresh_diff = rssi_thresh_diff & 0x000000ff;
	params.hi_rssi_scan_max_count = hirssi_scan_max_count;
	params.hi_rssi_scan_rssi_delta = hirssi_scan_delta;
	params.hi_rssi_scan_rssi_ub = hirssi_upper_bound & 0x00000ff;
	params.raise_rssi_thresh_5g = roam_params->raise_rssi_thresh_5g;
	params.dense_rssi_thresh_offset =
			 roam_params->dense_rssi_thresh_offset;
	params.dense_min_aps_cnt = roam_params->dense_min_aps_cnt;
	params.traffic_threshold =
			roam_params->traffic_threshold;
	params.initial_dense_status = roam_params->initial_dense_status;
	if (db2dbm_enabled)
		params.bg_scan_bad_rssi_thresh =
					   roam_params->bg_scan_bad_rssi_thresh;
	else
		params.bg_scan_bad_rssi_thresh =
					  roam_params->bg_scan_bad_rssi_thresh -
					  WMA_NOISE_FLOOR_DBM_DEFAULT;

	params.bg_scan_client_bitmap = roam_params->bg_scan_client_bitmap;
	params.roam_bad_rssi_thresh_offset_2g =
				roam_params->roam_bad_rssi_thresh_offset_2g;
	if (params.roam_bad_rssi_thresh_offset_2g)
		params.flags |= WMI_ROAM_BG_SCAN_FLAGS_2G_TO_5G_ONLY;

	/*
	 * The current Noise floor in firmware is -96dBm. Penalty/Boost
	 * threshold is applied on a weaker signal to make it even more weaker.
	 * So, there is a chance that the user may configure a very low
	 * Penalty/Boost threshold beyond the noise floor. If that is the case,
	 * then suppress the penalty/boost threshold to the noise floor.
	 */
	if (roam_params->raise_rssi_thresh_5g < WMA_NOISE_FLOOR_DBM_DEFAULT) {
		if (db2dbm_enabled) {
			params.penalty_threshold_5g = WMA_RSSI_MIN_VALUE;
			params.boost_threshold_5g = WMA_RSSI_MAX_VALUE;
		} else {
			params.penalty_threshold_5g = 0;
		}
	} else {
		if (db2dbm_enabled)
			params.boost_threshold_5g =
				roam_params->raise_rssi_thresh_5g;
		else
			params.boost_threshold_5g =
				(roam_params->raise_rssi_thresh_5g -
				 WMA_NOISE_FLOOR_DBM_DEFAULT) & 0x000000ff;
	}
	if (roam_params->drop_rssi_thresh_5g < WMA_NOISE_FLOOR_DBM_DEFAULT) {
		if (db2dbm_enabled)
			params.penalty_threshold_5g = WMA_RSSI_MIN_VALUE;
		else
			params.penalty_threshold_5g = 0;

	} else {
		if (db2dbm_enabled)
			params.penalty_threshold_5g =
				  roam_params->drop_rssi_thresh_5g;
		else
			params.penalty_threshold_5g =
				     (roam_params->drop_rssi_thresh_5g -
				      WMA_NOISE_FLOOR_DBM_DEFAULT) & 0x000000ff;
	}
	params.raise_factor_5g = roam_params->raise_factor_5g;
	params.drop_factor_5g = roam_params->drop_factor_5g;
	params.max_raise_rssi_5g = roam_params->max_raise_rssi_5g;
	params.max_drop_rssi_5g = roam_params->max_drop_rssi_5g;

	if (roam_params->good_rssi_roam)
		good_rssi_threshold = WMA_NOISE_FLOOR_DBM_DEFAULT;
	else
		good_rssi_threshold = 0;

	if (db2dbm_enabled)
		params.good_rssi_threshold = good_rssi_threshold;
	else
		params.good_rssi_threshold = (good_rssi_threshold -
					      WMA_NOISE_FLOOR_DBM_DEFAULT) &
					      0x000000ff;

	WMA_LOGD("WMA --> good_rssi_threshold=%d",
		 params.good_rssi_threshold);

	if (roam_req->early_stop_scan_enable) {
		if (db2dbm_enabled) {
			params.roam_earlystop_thres_min =
				roam_req->early_stop_scan_min_threshold;
			params.roam_earlystop_thres_max =
				roam_req->early_stop_scan_max_threshold;
		} else {
			params.roam_earlystop_thres_min =
				roam_req->early_stop_scan_min_threshold -
				WMA_NOISE_FLOOR_DBM_DEFAULT;
			params.roam_earlystop_thres_max =
				roam_req->early_stop_scan_max_threshold -
				WMA_NOISE_FLOOR_DBM_DEFAULT;
		}
	} else {
		if (db2dbm_enabled) {
			params.roam_earlystop_thres_min =
						    WMA_RSSI_MIN_VALUE;
			params.roam_earlystop_thres_max =
						    WMA_RSSI_MIN_VALUE;
		} else {
			params.roam_earlystop_thres_min = 0;
			params.roam_earlystop_thres_max = 0;
		}
	}
	params.rssi_thresh_offset_5g =
		roam_req->rssi_thresh_offset_5g;

	WMA_LOGD("early_stop_thresholds en=%d, min=%d, max=%d",
		roam_req->early_stop_scan_enable,
		params.roam_earlystop_thres_min,
		params.roam_earlystop_thres_max);
	WMA_LOGD("rssi_thresh_offset_5g = %d", params.rssi_thresh_offset_5g);

	status = wmi_unified_roam_scan_offload_rssi_thresh_cmd(
			wma_handle->wmi_handle, &params);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("roam_scan_offload_rssi_thresh_cmd failed %d", status);
		return status;
	}

	WMA_LOGD(FL("roam_scan_rssi_thresh=%d, roam_rssi_thresh_diff=%d"),
		rssi_thresh, rssi_thresh_diff);
	WMA_LOGD(
		FL("hirssi_scan max_count=%d, delta=%d, hirssi_upper_bound=%d"),
		hirssi_scan_max_count, hirssi_scan_delta, hirssi_upper_bound);
	WMA_LOGD(
		FL("dense_rssi_thresh_offset=%d, dense_min_aps_cnt=%d, traffic_threshold=%d initial_dense_status=%d"),
			roam_params->dense_rssi_thresh_offset,
			roam_params->dense_min_aps_cnt,
			roam_params->traffic_threshold,
			roam_params->initial_dense_status);
	WMA_LOGD(FL("BG Scan Bad RSSI:%d, bitmap:0x%x Offset for 2G to 5G Roam:%d"),
			roam_params->bg_scan_bad_rssi_thresh,
			roam_params->bg_scan_client_bitmap,
			roam_params->roam_bad_rssi_thresh_offset_2g);
	return status;
}

/**
 * wma_roam_scan_offload_scan_period() - set roam offload scan period
 * @wma_handle: wma handle
 * @roam_req:  Pointer to roam_offload_scan_req
 *
 * Send WMI_ROAM_SCAN_PERIOD parameters to fw.
 *
 * Return: QDF status
 */
QDF_STATUS
wma_roam_scan_offload_scan_period(tp_wma_handle wma_handle,
				  struct roam_offload_scan_req *roam_req)
{
	uint8_t vdev_id;
	struct roam_scan_period_params scan_period_params;

	vdev_id = roam_req->sessionId;
	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__, vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	scan_period_params.vdev_id = vdev_id;
	scan_period_params.scan_period = roam_req->EmptyRefreshScanPeriod;
	scan_period_params.scan_age = (3 * roam_req->EmptyRefreshScanPeriod);
	scan_period_params.roam_scan_inactivity_time =
			roam_req->roam_scan_inactivity_time;
	scan_period_params.roam_inactive_data_packet_count =
			roam_req->roam_inactive_data_packet_count;
	scan_period_params.roam_scan_period_after_inactivity =
			roam_req->roam_scan_period_after_inactivity;
	scan_period_params.full_scan_period = roam_req->full_roam_scan_period;

	return wmi_unified_roam_scan_offload_scan_period(wma_handle->wmi_handle,
							 &scan_period_params);
}

/**
 * wma_roam_scan_offload_rssi_change() - set roam offload RSSI change threshold
 * @wma_handle: wma handle
 * @rssi_change_thresh: RSSI Change threshold
 * @bcn_rssi_weight: beacon RSSI weight
 * @vdev_id: vdev id
 *
 * Send WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD parameters to fw.
 *
 * Return: QDF status
 */
QDF_STATUS wma_roam_scan_offload_rssi_change(tp_wma_handle wma_handle,
					     uint32_t vdev_id,
					     int32_t rssi_change_thresh,
					     uint32_t bcn_rssi_weight,
					     uint32_t hirssi_delay_btw_scans)
{
	if (!wma_is_vdev_valid(vdev_id)) {
		wma_err("Invalid vdev id:%d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_roam_scan_offload_rssi_change_cmd(
				wma_handle->wmi_handle,
				vdev_id, rssi_change_thresh,
				bcn_rssi_weight, hirssi_delay_btw_scans);
}

/**
 * wma_roam_scan_offload_chan_list() - set roam offload channel list
 * @wma_handle: wma handle
 * @chan_count: channel count
 * @chan_freq_list: channel list
 * @list_type: list type
 * @vdev_id: vdev id
 *
 * Set roam offload channel list.
 *
 * Return: QDF status
 */
QDF_STATUS wma_roam_scan_offload_chan_list(tp_wma_handle wma_handle,
					   uint8_t chan_count,
					   uint32_t *chan_freq_list,
					   uint8_t list_type, uint32_t vdev_id)
{
	QDF_STATUS status;
	int i;
	uint32_t *chan_list_mhz = NULL;

	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__, vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (chan_count) {
		chan_list_mhz = qdf_mem_malloc(chan_count * sizeof(*chan_list_mhz));
		if (!chan_list_mhz)
			return QDF_STATUS_E_NOMEM;
	}

	for (i = 0; ((i < chan_count) &&
		     (i < SIR_ROAM_MAX_CHANNELS)); i++) {
		chan_list_mhz[i] = chan_freq_list[i];
		WMA_LOGD("%d,", chan_list_mhz[i]);
	}

	status = wmi_unified_roam_scan_offload_chan_list_cmd(
					wma_handle->wmi_handle,
					chan_count, chan_list_mhz,
					list_type, vdev_id);
	qdf_mem_free(chan_list_mhz);

	return status;
}

/**
 * e_csr_auth_type_to_rsn_authmode() - map csr auth type to rsn authmode
 * @authtype: CSR authtype
 * @encr: CSR Encryption
 *
 * Map CSR's authentication type into RSN auth mode used by firmware
 *
 * Return: WMI RSN auth mode
 */
A_UINT32 e_csr_auth_type_to_rsn_authmode(enum csr_akm_type authtype,
					 eCsrEncryptionType encr)
{
	switch (authtype) {
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		return WMI_AUTH_OPEN;
	case eCSR_AUTH_TYPE_WPA:
		return WMI_AUTH_WPA;
	case eCSR_AUTH_TYPE_WPA_PSK:
		return WMI_AUTH_WPA_PSK;
	case eCSR_AUTH_TYPE_RSN:
		return WMI_AUTH_RSNA;
	case eCSR_AUTH_TYPE_RSN_PSK:
		return WMI_AUTH_RSNA_PSK;
	case eCSR_AUTH_TYPE_FT_RSN:
		return WMI_AUTH_FT_RSNA;
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
		return WMI_AUTH_FT_RSNA_PSK;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
		return WMI_AUTH_WAPI;
	case eCSR_AUTH_TYPE_WAPI_WAI_PSK:
		return WMI_AUTH_WAPI_PSK;
#endif /* FEATURE_WLAN_WAPI */
#ifdef FEATURE_WLAN_ESE
	case eCSR_AUTH_TYPE_CCKM_WPA:
		return WMI_AUTH_CCKM_WPA;
	case eCSR_AUTH_TYPE_CCKM_RSN:
		return WMI_AUTH_CCKM_RSNA;
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
		return WMI_AUTH_RSNA_PSK_SHA256;
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
		return WMI_AUTH_RSNA_8021X_SHA256;
#endif /* WLAN_FEATURE_11W */
	case eCSR_AUTH_TYPE_NONE:
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		/* In case of WEP and other keys, NONE means OPEN auth */
		if (encr == eCSR_ENCRYPT_TYPE_WEP40_STATICKEY ||
		    encr == eCSR_ENCRYPT_TYPE_WEP104_STATICKEY ||
		    encr == eCSR_ENCRYPT_TYPE_WEP40 ||
		    encr == eCSR_ENCRYPT_TYPE_WEP104 ||
		    encr == eCSR_ENCRYPT_TYPE_TKIP ||
		    encr == eCSR_ENCRYPT_TYPE_AES ||
		    encr == eCSR_ENCRYPT_TYPE_AES_GCMP ||
		    encr == eCSR_ENCRYPT_TYPE_AES_GCMP_256) {
			return WMI_AUTH_OPEN;
		}
		return WMI_AUTH_NONE;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		return WMI_AUTH_SHARED;
	case eCSR_AUTH_TYPE_FILS_SHA256:
		return WMI_AUTH_RSNA_FILS_SHA256;
	case eCSR_AUTH_TYPE_FILS_SHA384:
		return WMI_AUTH_RSNA_FILS_SHA384;
	case eCSR_AUTH_TYPE_FT_FILS_SHA256:
		return WMI_AUTH_FT_RSNA_FILS_SHA256;
	case eCSR_AUTH_TYPE_FT_FILS_SHA384:
		return WMI_AUTH_FT_RSNA_FILS_SHA384;
	case eCSR_AUTH_TYPE_SUITEB_EAP_SHA256:
		return WMI_AUTH_RSNA_SUITE_B_8021X_SHA256;
	case eCSR_AUTH_TYPE_SUITEB_EAP_SHA384:
		return WMI_AUTH_RSNA_SUITE_B_8021X_SHA384;
	case eCSR_AUTH_TYPE_OWE:
		return WMI_AUTH_WPA3_OWE;
	case eCSR_AUTH_TYPE_SAE:
		return WMI_AUTH_WPA3_SAE;
	case eCSR_AUTH_TYPE_FT_SAE:
		return WMI_AUTH_FT_RSNA_SAE;
	case eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384:
		return WMI_AUTH_FT_RSNA_SUITE_B_8021X_SHA384;
	default:
		return WMI_AUTH_NONE;
	}
}

/**
 * e_csr_encryption_type_to_rsn_cipherset() - map csr enc type to ESN cipher
 * @encr: CSR Encryption
 *
 * Map CSR's encryption type into RSN cipher types used by firmware
 *
 * Return: WMI RSN cipher
 */
A_UINT32 e_csr_encryption_type_to_rsn_cipherset(eCsrEncryptionType encr)
{

	switch (encr) {
	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP40:
	case eCSR_ENCRYPT_TYPE_WEP104:
		return WMI_CIPHER_WEP;
	case eCSR_ENCRYPT_TYPE_TKIP:
		return WMI_CIPHER_TKIP;
	case eCSR_ENCRYPT_TYPE_AES:
		return WMI_CIPHER_AES_CCM;
	/* FWR will use key length to distinguish GCMP 128 or 256 */
	case eCSR_ENCRYPT_TYPE_AES_GCMP:
	case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
		return WMI_CIPHER_AES_GCM;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_ENCRYPT_TYPE_WPI:
		return WMI_CIPHER_WAPI;
#endif /* FEATURE_WLAN_WAPI */
	case eCSR_ENCRYPT_TYPE_ANY:
		return WMI_CIPHER_ANY;
	case eCSR_ENCRYPT_TYPE_NONE:
	default:
		return WMI_CIPHER_NONE;
	}
}

/**
 * wma_convert_gp_mgmt_cipher_to_target_cipher_type() - map csr ani group mgmt
 * enc type to RSN cipher
 * @encr: CSR Encryption
 *
 * Map CSR's group management cipher type into RSN cipher types used by firmware
 *
 * Return: WMI RSN cipher
 */
static uint32_t
wma_convert_gp_mgmt_cipher_to_target_cipher_type(tAniEdType cipher_type)
{
	switch (cipher_type) {
	/* BIP-CMAC-128 (00:0f:ac: 0x06) */
	case eSIR_ED_AES_128_CMAC:
		return WMI_CIPHER_AES_CMAC;

	/* BIP-GMAC-128 (00:0f:ac: 0x0b) */
	case eSIR_ED_AES_GMAC_128:
		return WMI_CIPHER_AES_GMAC;

	/* BIP-GMAC-256(00:0f:ac: 0x0c)*/
	case eSIR_ED_AES_GMAC_256:
		return WMI_CIPHER_BIP_GMAC_256;
	case eSIR_ED_NONE:
	default:
		return WMI_CIPHER_NONE;
	}
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wma_roam_scan_get_cckm_mode() - Get the CCKM auth mode
 * @roam_req: Roaming request buffer
 * @auth_mode: Auth mode to be converted
 *
 * Based on LFR2.0 or LFR3.0, return the proper auth type
 *
 * Return: if LFR2.0, then return WMI_AUTH_CCKM for backward compatibility
 *         if LFR3.0 then return the appropriate auth type
 */
static
uint32_t wma_roam_scan_get_cckm_mode(struct roam_offload_scan_req *roam_req,
				     uint32_t auth_mode)
{
	if (roam_req->roam_offload_enabled)
		return auth_mode;
	else
		return WMI_AUTH_CCKM;

}
#else
static
uint32_t wma_roam_scan_get_cckm_mode(struct roam_offload_scan_req *roam_req,
				     uint32_t auth_mode)
{
	return WMI_AUTH_CCKM;
}
#endif
/**
 * wma_roam_scan_fill_ap_profile() - fill ap_profile
 * @roam_req: roam offload scan request
 * @profile: ap profile
 *
 * Fill ap_profile structure from configured parameters
 *
 * Return: none
 */
static void
wma_roam_scan_fill_ap_profile(struct roam_offload_scan_req *roam_req,
			      struct ap_profile *profile)
{
	uint32_t rsn_authmode;

	qdf_mem_zero(profile, sizeof(*profile));
	if (!roam_req) {
		profile->ssid.length = 0;
		profile->ssid.mac_ssid[0] = 0;
		profile->rsn_authmode = WMI_AUTH_NONE;
		profile->rsn_ucastcipherset = WMI_CIPHER_NONE;
		profile->rsn_mcastcipherset = WMI_CIPHER_NONE;
		profile->rsn_mcastmgmtcipherset = WMI_CIPHER_NONE;
		profile->rssi_threshold = WMA_ROAM_RSSI_DIFF_DEFAULT;

		return;
	}

	profile->ssid.length = roam_req->ConnectedNetwork.ssId.length;
	qdf_mem_copy(profile->ssid.mac_ssid,
		     roam_req->ConnectedNetwork.ssId.ssId,
		     profile->ssid.length);
	profile->rsn_authmode =
		e_csr_auth_type_to_rsn_authmode(
			roam_req->ConnectedNetwork.authentication,
			roam_req->ConnectedNetwork.encryption);
	rsn_authmode = profile->rsn_authmode;

	if (rsn_authmode == WMI_AUTH_CCKM_WPA ||
	    rsn_authmode == WMI_AUTH_CCKM_RSNA)
		profile->rsn_authmode =
			wma_roam_scan_get_cckm_mode(roam_req, rsn_authmode);

	/* Pairwise cipher suite */
	profile->rsn_ucastcipherset =
		e_csr_encryption_type_to_rsn_cipherset(
				roam_req->ConnectedNetwork.encryption);

	/* Group cipher suite */
	profile->rsn_mcastcipherset =
		e_csr_encryption_type_to_rsn_cipherset(
			roam_req->ConnectedNetwork.mcencryption);

	/* Group management cipher suite */
	profile->rsn_mcastmgmtcipherset =
		wma_convert_gp_mgmt_cipher_to_target_cipher_type(
			roam_req->ConnectedNetwork.gp_mgmt_cipher_suite);

	profile->rssi_threshold = roam_req->RoamRssiDiff;
	if (roam_req->rssi_abs_thresh)
		profile->rssi_abs_thresh = roam_req->rssi_abs_thresh;
#ifdef WLAN_FEATURE_11W
	if (roam_req->ConnectedNetwork.mfp_enabled)
		profile->flags |= WMI_AP_PROFILE_FLAG_PMF;
#endif
}

/**
 * wma_process_set_pdev_ie_req() - process the pdev set IE req
 * @wma: Pointer to wma handle
 * @ie_params: Pointer to IE data.
 *
 * Sends the WMI req to set the IE to FW.
 *
 * Return: None
 */
void wma_process_set_pdev_ie_req(tp_wma_handle wma,
		struct set_ie_param *ie_params)
{
	if (ie_params->ie_type == DOT11_HT_IE)
		wma_process_set_pdev_ht_ie_req(wma, ie_params);
	if (ie_params->ie_type == DOT11_VHT_IE)
		wma_process_set_pdev_vht_ie_req(wma, ie_params);

	qdf_mem_free(ie_params->ie_ptr);
}

/**
 * wma_process_set_pdev_ht_ie_req() - sends HT IE data to FW
 * @wma: Pointer to wma handle
 * @ie_params: Pointer to IE data.
 * @nss: Nss values to prepare the HT IE.
 *
 * Sends the WMI req to set the HT IE to FW.
 *
 * Return: None
 */
void wma_process_set_pdev_ht_ie_req(tp_wma_handle wma,
		struct set_ie_param *ie_params)
{
	QDF_STATUS status;
	wmi_pdev_set_ht_ie_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	uint16_t ie_len_pad;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	ie_len_pad = roundup(ie_params->ie_len, sizeof(uint32_t));
	len += ie_len_pad;

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf)
		return;

	cmd = (wmi_pdev_set_ht_ie_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_ht_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_pdev_set_ht_ie_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->ie_len = ie_params->ie_len;
	cmd->tx_streams = ie_params->nss;
	cmd->rx_streams = ie_params->nss;
	WMA_LOGD("Setting pdev HT ie with Nss = %u",
			ie_params->nss);
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_pad);
	if (ie_params->ie_len) {
		qdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
			     (uint8_t *)ie_params->ie_ptr,
			     ie_params->ie_len);
	}

	status = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				      WMI_PDEV_SET_HT_CAP_IE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);
}

/**
 * wma_process_set_pdev_vht_ie_req() - sends VHT IE data to FW
 * @wma: Pointer to wma handle
 * @ie_params: Pointer to IE data.
 * @nss: Nss values to prepare the VHT IE.
 *
 * Sends the WMI req to set the VHT IE to FW.
 *
 * Return: None
 */
void wma_process_set_pdev_vht_ie_req(tp_wma_handle wma,
		struct set_ie_param *ie_params)
{
	QDF_STATUS status;
	wmi_pdev_set_vht_ie_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	uint16_t ie_len_pad;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	ie_len_pad = roundup(ie_params->ie_len, sizeof(uint32_t));
	len += ie_len_pad;

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf)
		return;

	cmd = (wmi_pdev_set_vht_ie_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_pdev_set_vht_ie_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_set_vht_ie_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->ie_len = ie_params->ie_len;
	cmd->tx_streams = ie_params->nss;
	cmd->rx_streams = ie_params->nss;
	WMA_LOGD("Setting pdev VHT ie with Nss = %u",
			ie_params->nss);
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_pad);
	if (ie_params->ie_len) {
		qdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
				(uint8_t *)ie_params->ie_ptr,
				ie_params->ie_len);
	}

	status = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				      WMI_PDEV_SET_VHT_CAP_IE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);
}

/**
 * wma_roam_scan_scan_params() - fill roam scan params
 * @wma_handle: wma handle
 * @mac: Mac ptr
 * @scan_params: scan parameters
 * @roam_req: NULL if this routine is called before connect
 *            It will be non-NULL if called after assoc.
 *
 * Fill scan_params structure from configured parameters
 *
 * Return: none
 */
void wma_roam_scan_fill_scan_params(tp_wma_handle wma_handle,
				    struct mac_context *mac,
				    struct roam_offload_scan_req *roam_req,
				    wmi_start_scan_cmd_fixed_param *
				    scan_params)
{
	uint8_t channels_per_burst = 0;

	if (!mac) {
		WMA_LOGE("%s: mac is NULL", __func__);
		return;
	}

	qdf_mem_zero(scan_params, sizeof(wmi_start_scan_cmd_fixed_param));
	scan_params->scan_ctrl_flags = WMI_SCAN_ADD_CCK_RATES |
				       WMI_SCAN_ADD_OFDM_RATES |
				       WMI_SCAN_ADD_DS_IE_IN_PROBE_REQ;
	if (roam_req) {
		/* Parameters updated after association is complete */
		WMA_LOGD("%s: NeighborScanChannelMinTime: %d NeighborScanChannelMaxTime: %d",
			 __func__,
			 roam_req->NeighborScanChannelMinTime,
			 roam_req->NeighborScanChannelMaxTime);
		WMA_LOGD("%s: NeighborScanTimerPeriod: %d "
			 "neighbor_scan_min_timer_period %d "
			 "HomeAwayTime: %d nProbes: %d",
			 __func__,
			 roam_req->NeighborScanTimerPeriod,
			 roam_req->neighbor_scan_min_timer_period,
			 roam_req->HomeAwayTime, roam_req->nProbes);

		wlan_scan_cfg_get_passive_dwelltime(wma_handle->psoc,
					&scan_params->dwell_time_passive);
		/*
		 * Here is the formula,
		 * T(HomeAway) = N * T(dwell) + (N+1) * T(cs)
		 * where N is number of channels scanned in single burst
		 */
		scan_params->dwell_time_active =
			roam_req->NeighborScanChannelMaxTime;
		if (roam_req->HomeAwayTime <
		    2 * WMA_ROAM_SCAN_CHANNEL_SWITCH_TIME) {
			/* clearly we can't follow home away time.
			 * Make it a split scan.
			 */
			scan_params->burst_duration = 0;
		} else {
			channels_per_burst =
				(roam_req->HomeAwayTime -
				 WMA_ROAM_SCAN_CHANNEL_SWITCH_TIME)
				/ (scan_params->dwell_time_active +
				   WMA_ROAM_SCAN_CHANNEL_SWITCH_TIME);

			if (channels_per_burst < 1) {
				/* dwell time and home away time conflicts */
				/* we will override dwell time */
				scan_params->dwell_time_active =
					roam_req->HomeAwayTime -
					2 * WMA_ROAM_SCAN_CHANNEL_SWITCH_TIME;
				scan_params->burst_duration =
					scan_params->dwell_time_active;
			} else {
				scan_params->burst_duration =
					channels_per_burst *
					scan_params->dwell_time_active;
			}
		}
		if (roam_req->allowDFSChannelRoam ==
		    SIR_ROAMING_DFS_CHANNEL_ENABLED_NORMAL
		    && roam_req->HomeAwayTime > 0
		    && roam_req->ChannelCacheType != CHANNEL_LIST_STATIC) {
			/* Roaming on DFS channels is supported and it is not
			 * app channel list. It is ok to override homeAwayTime
			 * to accommodate DFS dwell time in burst
			 * duration.
			 */
			scan_params->burst_duration =
				QDF_MAX(scan_params->burst_duration,
					scan_params->dwell_time_passive);
		}
		scan_params->min_rest_time =
			roam_req->neighbor_scan_min_timer_period;
		scan_params->max_rest_time = roam_req->NeighborScanTimerPeriod;
		scan_params->repeat_probe_time = (roam_req->nProbes > 0) ?
				 QDF_MAX(scan_params->dwell_time_active /
						roam_req->nProbes, 1) : 0;
		scan_params->probe_spacing_time = 0;
		scan_params->probe_delay = 0;
		/* 30 seconds for full scan cycle */
		scan_params->max_scan_time = WMA_HW_DEF_SCAN_MAX_DURATION;
		scan_params->idle_time = scan_params->min_rest_time;
		scan_params->n_probes = roam_req->nProbes;
		if (roam_req->allowDFSChannelRoam ==
		    SIR_ROAMING_DFS_CHANNEL_DISABLED) {
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

			if (roam_req->allowDFSChannelRoam ==
			    SIR_ROAMING_DFS_CHANNEL_ENABLED_ACTIVE &&
			    roam_req->ChannelCacheType == CHANNEL_LIST_STATIC &&
			    roam_req->ConnectedNetwork.ChannelCount > 0) {
				scan_params->scan_ctrl_flags |=
					WMI_SCAN_FLAG_FORCE_ACTIVE_ON_DFS;
			}
		}
		WMI_SCAN_SET_DWELL_MODE(scan_params->scan_ctrl_flags,
				roam_req->roamscan_adaptive_dwell_mode);

	} else {
		/* roam_req = NULL during initial or pre-assoc invocation */
		scan_params->dwell_time_active =
			WMA_ROAM_DWELL_TIME_ACTIVE_DEFAULT;
		scan_params->dwell_time_passive =
			WMA_ROAM_DWELL_TIME_PASSIVE_DEFAULT;
		scan_params->min_rest_time = WMA_ROAM_MIN_REST_TIME_DEFAULT;
		scan_params->max_rest_time = WMA_ROAM_MAX_REST_TIME_DEFAULT;
		scan_params->repeat_probe_time = 0;
		scan_params->probe_spacing_time = 0;
		scan_params->probe_delay = 0;
		scan_params->max_scan_time = WMA_HW_DEF_SCAN_MAX_DURATION;
		scan_params->idle_time = scan_params->min_rest_time;
		scan_params->burst_duration = 0;
		scan_params->n_probes = 0;
	}

	WMA_LOGD("%s: Rome roam scan parameters:  dwell_time_active = %d, dwell_time_passive = %d",
		 __func__, scan_params->dwell_time_active,
		 scan_params->dwell_time_passive);
	WMA_LOGD("%s: min_rest_time = %d, max_rest_time = %d, repeat_probe_time = %d n_probes = %d",
		 __func__, scan_params->min_rest_time,
		 scan_params->max_rest_time,
		 scan_params->repeat_probe_time, scan_params->n_probes);
	WMA_LOGD("%s: max_scan_time = %d, idle_time = %d, burst_duration = %d, scan_ctrl_flags = 0x%x",
		 __func__, scan_params->max_scan_time, scan_params->idle_time,
		 scan_params->burst_duration, scan_params->scan_ctrl_flags);
}

/**
 * wma_roam_scan_offload_ap_profile() - set roam ap profile in fw
 * @wma_handle: wma handle
 * @mac_ctx: Mac ptr
 * @roam_req: Request which contains the ap profile
 *
 * Send WMI_ROAM_AP_PROFILE to firmware
 *
 * Return: QDF status
 */
static QDF_STATUS wma_roam_scan_offload_ap_profile(tp_wma_handle wma_handle,
				struct roam_offload_scan_req *roam_req)
{
	struct ap_profile_params ap_profile;
	bool db2dbm_enabled;

	if (!wma_is_vdev_valid(roam_req->sessionId)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__,
			 roam_req->sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	ap_profile.vdev_id = roam_req->sessionId;
	wma_roam_scan_fill_ap_profile(roam_req, &ap_profile.profile);

	db2dbm_enabled = wmi_service_enabled(wma_handle->wmi_handle,
					     wmi_service_hw_db2dbm_support);
	if (!ap_profile.profile.rssi_abs_thresh) {
		if (db2dbm_enabled)
			ap_profile.profile.rssi_abs_thresh = WMA_RSSI_MIN_VALUE;
	} else {
		if (!db2dbm_enabled)
			ap_profile.profile.rssi_abs_thresh -=
						WMA_NOISE_FLOOR_DBM_DEFAULT;
	}
	ap_profile.param = roam_req->score_params;
	ap_profile.min_rssi_params[DEAUTH_MIN_RSSI] =
			roam_req->min_rssi_params[DEAUTH_MIN_RSSI];
	ap_profile.min_rssi_params[BMISS_MIN_RSSI] =
			roam_req->min_rssi_params[BMISS_MIN_RSSI];
	if (!db2dbm_enabled) {
		ap_profile.min_rssi_params[DEAUTH_MIN_RSSI].min_rssi -=
				       WMA_NOISE_FLOOR_DBM_DEFAULT;
		ap_profile.min_rssi_params[DEAUTH_MIN_RSSI].min_rssi &=
				       0x000000ff;

		ap_profile.min_rssi_params[BMISS_MIN_RSSI].min_rssi -=
			       WMA_NOISE_FLOOR_DBM_DEFAULT;
		ap_profile.min_rssi_params[BMISS_MIN_RSSI].min_rssi &=
				0x000000ff;
	}

	ap_profile.score_delta_param[IDLE_ROAM_TRIGGER] =
				roam_req->score_delta_param[IDLE_ROAM_TRIGGER];
	ap_profile.score_delta_param[BTM_ROAM_TRIGGER] =
				roam_req->score_delta_param[BTM_ROAM_TRIGGER];

	return wmi_unified_send_roam_scan_offload_ap_cmd(wma_handle->wmi_handle,
							 &ap_profile);
}

/**
 * wma_roam_scan_filter() - Filter to be applied while roaming
 * @wma_handle:     Global WMA Handle
 * @roam_req:       Request which contains the filters
 *
 * There are filters such as whitelist, blacklist and preferred
 * list that need to be applied to the scan results to form the
 * probable candidates for roaming.
 *
 * Return: Return success upon successfully passing the
 *         parameters to the firmware, otherwise failure.
 */
static QDF_STATUS wma_roam_scan_filter(tp_wma_handle wma_handle,
				       struct roam_offload_scan_req *roam_req)
{
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t num_bssid_black_list = 0, num_ssid_white_list = 0,
	   num_bssid_preferred_list = 0,  num_rssi_rejection_ap = 0;
	uint32_t op_bitmap = 0;
	struct roam_ext_params *roam_params;
	struct roam_scan_filter_params *params;
	struct lca_disallow_config_params *lca_config_params;

	if (!wma_is_vdev_valid(roam_req->sessionId)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__,
			 roam_req->sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	params = qdf_mem_malloc(sizeof(struct roam_scan_filter_params));
	if (!params)
		return QDF_STATUS_E_NOMEM;

	roam_params = &roam_req->roam_params;
	lca_config_params = &roam_req->lca_config_params;
	if (roam_req->Command != ROAM_SCAN_OFFLOAD_STOP) {
		switch (roam_req->reason) {
		case REASON_ROAM_SET_BLACKLIST_BSSID:
			op_bitmap |= 0x1;
			num_bssid_black_list =
				roam_params->num_bssid_avoid_list;
			break;
		case REASON_ROAM_SET_SSID_ALLOWED:
			op_bitmap |= 0x2;
			num_ssid_white_list =
				roam_params->num_ssid_allowed_list;
			break;
		case REASON_ROAM_SET_FAVORED_BSSID:
			op_bitmap |= 0x4;
			num_bssid_preferred_list =
				roam_params->num_bssid_favored;
			break;
		case REASON_CTX_INIT:
			if (roam_req->Command == ROAM_SCAN_OFFLOAD_START) {
				params->lca_disallow_config_present = true;
				op_bitmap |=
					ROAM_FILTER_OP_BITMAP_LCA_DISALLOW |
					ROAM_FILTER_OP_BITMAP_RSSI_REJECTION_OCE;
				num_rssi_rejection_ap =
					roam_params->num_rssi_rejection_ap;
			} else {
				WMA_LOGD("%s : Roam Filter need not be sent", __func__);
				qdf_mem_free(params);
				return QDF_STATUS_SUCCESS;
			}
			break;
		default:
			WMA_LOGD("%s : Roam Filter need not be sent", __func__);
			qdf_mem_free(params);
			return QDF_STATUS_SUCCESS;
		}
	} else {
		/* In case of STOP command, reset all the variables
		 * except for blacklist BSSID which should be retained
		 * across connections.
		 */
		op_bitmap = 0x2 | 0x4;
		num_ssid_white_list = roam_params->num_ssid_allowed_list;
		num_bssid_preferred_list = roam_params->num_bssid_favored;
	}

	/* fill in fixed values */
	params->vdev_id = roam_req->sessionId;
	params->op_bitmap = op_bitmap;
	params->num_bssid_black_list = num_bssid_black_list;
	params->num_ssid_white_list = num_ssid_white_list;
	params->num_bssid_preferred_list = num_bssid_preferred_list;
	params->num_rssi_rejection_ap = num_rssi_rejection_ap;
	qdf_mem_copy(params->bssid_avoid_list, roam_params->bssid_avoid_list,
			MAX_BSSID_AVOID_LIST * sizeof(struct qdf_mac_addr));

	for (i = 0; i < num_ssid_white_list; i++) {
		qdf_mem_copy(params->ssid_allowed_list[i].mac_ssid,
				roam_params->ssid_allowed_list[i].ssId,
			roam_params->ssid_allowed_list[i].length);
		params->ssid_allowed_list[i].length =
				roam_params->ssid_allowed_list[i].length;
		WMA_LOGD("%s: SSID length=%d", __func__,
				params->ssid_allowed_list[i].length);
		qdf_trace_hex_dump(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			(uint8_t *)params->ssid_allowed_list[i].mac_ssid,
			params->ssid_allowed_list[i].length);
	}
	qdf_mem_copy(params->bssid_favored, roam_params->bssid_favored,
			MAX_BSSID_FAVORED * sizeof(struct qdf_mac_addr));
	qdf_mem_copy(params->bssid_favored_factor,
			roam_params->bssid_favored_factor, MAX_BSSID_FAVORED);
	qdf_mem_copy(params->rssi_rejection_ap,
		roam_params->rssi_reject_bssid_list,
		MAX_RSSI_AVOID_BSSID_LIST *
		sizeof(struct reject_ap_config_params));

	if (params->lca_disallow_config_present) {
		params->disallow_duration
				= lca_config_params->disallow_duration;
		params->rssi_channel_penalization
				= lca_config_params->rssi_channel_penalization;
		params->num_disallowed_aps
				= lca_config_params->num_disallowed_aps;
	}
	status = wmi_unified_roam_scan_filter_cmd(wma_handle->wmi_handle,
						  params);

	qdf_mem_free(params);
	return status;
}

/**
 * wma_roam_scan_bmiss_cnt() - set bmiss count to fw
 * @wma_handle: wma handle
 * @first_bcnt: first bmiss count
 * @final_bcnt: final bmiss count
 * @vdev_id: vdev id
 *
 * set first & final biss count to fw.
 *
 * Return: QDF status
 */
QDF_STATUS wma_roam_scan_bmiss_cnt(tp_wma_handle wma_handle,
				   A_INT32 first_bcnt,
				   A_UINT32 final_bcnt, uint32_t vdev_id)
{
	QDF_STATUS status;

	WMA_LOGD("%s: first_bcnt: %d, final_bcnt: %d", __func__, first_bcnt,
		 final_bcnt);

	status = wma_vdev_set_param(wma_handle->wmi_handle,
				vdev_id, WMI_VDEV_PARAM_BMISS_FIRST_BCNT,
				first_bcnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("wma_vdev_set_param WMI_VDEV_PARAM_BMISS_FIRST_BCNT returned Error %d",
			status);
		return status;
	}

	status = wma_vdev_set_param(wma_handle->wmi_handle,
				vdev_id, WMI_VDEV_PARAM_BMISS_FINAL_BCNT,
				final_bcnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("wma_vdev_set_param WMI_VDEV_PARAM_BMISS_FINAL_BCNT returned Error %d",
			status);
		return status;
	}

	return status;
}

/**
 * wma_roam_scan_offload_command() - set roam offload command
 * @wma_handle: wma handle
 * @command: command
 * @vdev_id: vdev id
 *
 * This function set roam offload command to fw.
 *
 * Return: QDF status
 */
QDF_STATUS wma_roam_scan_offload_command(tp_wma_handle wma_handle,
					 uint32_t command, uint32_t vdev_id)
{
	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__, vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_roam_scan_offload_cmd(wma_handle->wmi_handle,
						 command, vdev_id);
}

/**
 * wma_roam_scan_btm_offload() - Send BTM offload config
 * @wma_handle: wma handle
 * @roam_req: roam request parameters
 *
 * This function is used to send BTM offload config to fw
 *
 * Return: QDF status
 */
static QDF_STATUS
wma_roam_scan_btm_offload(tp_wma_handle wma_handle,
			  struct roam_offload_scan_req *roam_req)
{
	struct wmi_btm_config *params;
	QDF_STATUS status;

	params = qdf_mem_malloc(sizeof(struct wmi_btm_config));
	if (!params)
		return QDF_STATUS_E_FAILURE;

	params->vdev_id = roam_req->sessionId;
	params->btm_offload_config = roam_req->btm_offload_config;
	params->btm_solicited_timeout = roam_req->btm_solicited_timeout;
	params->btm_max_attempt_cnt = roam_req->btm_max_attempt_cnt;
	params->btm_sticky_time = roam_req->btm_sticky_time;
	params->disassoc_timer_threshold = roam_req->disassoc_timer_threshold;
	params->btm_query_bitmask = roam_req->btm_query_bitmask;
	params->btm_candidate_min_score =
			roam_req->btm_trig_min_candidate_score;

	WMA_LOGD("%s: vdev %u btm_offload:%u btm_query_bitmask:%u btm_candidate_min_score:%d",
		 __func__, params->vdev_id, params->btm_offload_config,
		 params->btm_query_bitmask, params->btm_candidate_min_score);

	status = wmi_unified_send_btm_config(wma_handle->wmi_handle, params);
	qdf_mem_free(params);

	return status;
}

/**
 * wma_send_roam_bss_load_config() - API to send load bss trigger
 * related parameters to fw
 * @handle: WMA handle
 * @roam_req: bss load config parameters from csr to be sent to fw
 *
 * Return: None
 */
static
void wma_send_roam_bss_load_config(WMA_HANDLE handle,
				   struct wmi_bss_load_config *params)
{
	QDF_STATUS status;
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	bool db2dbm_enabled;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("WMA is closed, cannot send bss load config");
		return;
	}

	db2dbm_enabled = wmi_service_enabled(wma_handle->wmi_handle,
					     wmi_service_hw_db2dbm_support);
	if (!db2dbm_enabled) {
		params->rssi_threshold_5ghz -= WMA_NOISE_FLOOR_DBM_DEFAULT;
		params->rssi_threshold_5ghz &= 0x000000ff;

		params->rssi_threshold_24ghz -= WMA_NOISE_FLOOR_DBM_DEFAULT;
		params->rssi_threshold_24ghz &= 0x000000ff;
	}

	WMA_LOGD("%s: Bss load trig params vdev %u threshold %u sample_time: %u 5Ghz RSSI threshold:%d 2.4G rssi threshold:%d",
		 __func__, params->vdev_id, params->bss_load_threshold,
		 params->bss_load_sample_time, params->rssi_threshold_5ghz,
		 params->rssi_threshold_24ghz);

	status = wmi_unified_send_bss_load_config(wma_handle->wmi_handle,
						  params);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to send bss load trigger config command");
}

/**
 * wma_send_offload_11k_params() - API to send 11k offload params to FW
 * @handle: WMA handle
 * @params: Pointer to 11k offload params
 *
 * Return: None
 */
static
QDF_STATUS wma_send_offload_11k_params(WMA_HANDLE handle,
				    struct wmi_11k_offload_params *params)
{
	QDF_STATUS status;
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, cannot send 11k offload cmd",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!wmi_service_enabled(wma_handle->wmi_handle,
	    wmi_service_11k_neighbour_report_support)) {
		WMA_LOGE(FL("FW doesn't support 11k offload"));
		return QDF_STATUS_E_NOSUPPORT;
	}

	/*
	 * If 11k enable command and ssid length is 0, drop it
	 */
	if (params->offload_11k_bitmask &&
	    !params->neighbor_report_params.ssid.length) {
		WMA_LOGD("%s: SSID Len 0", __func__);
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_offload_11k_cmd(wma_handle->wmi_handle, params);

	if (status != QDF_STATUS_SUCCESS)
		WMA_LOGE("failed to send 11k offload command");

	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wma_send_disconnect_roam_params() - Send the disconnect roam parameters
 * to wmi
 * @handle: WMA handle
 * @roam_req: Pointer to roam_offload_scan_req sent from CSR
 *
 * Return: None
 */
static void
wma_send_disconnect_roam_params(tp_wma_handle wma_handle,
				struct roam_offload_scan_req *roam_req)
{
	QDF_STATUS status;
	struct wmi_disconnect_roam_params *params =
				&roam_req->disconnect_roam_params;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("WMA is closed, cannot send disconnect roam params");
		return;
	}

	switch (roam_req->Command) {
	case ROAM_SCAN_OFFLOAD_START:
	case ROAM_SCAN_OFFLOAD_UPDATE_CFG:
		if (!params->enable)
			return;
		break;
	case ROAM_SCAN_OFFLOAD_STOP:
		params->enable = false;
		break;
	default:
		break;
	}

	status = wmi_unified_send_disconnect_roam_params(wma_handle->wmi_handle,
							 params);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to send disconnect roam parameters");
}

/**
 * wma_send_idle_roam_params() - Send the idle roam parameters to wmi
 * @handle: WMA handle
 * @roam_req: Pointer to roam_offload_scan_req sent from CSR
 *
 * Return: None
 */
static void
wma_send_idle_roam_params(tp_wma_handle wma_handle,
			  struct roam_offload_scan_req *roam_req)
{
	QDF_STATUS status;
	bool db2dbm_enabled;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("WMA is closed, cannot send idle roam params");
		return;
	}

	switch (roam_req->Command) {
	case ROAM_SCAN_OFFLOAD_START:
	case ROAM_SCAN_OFFLOAD_UPDATE_CFG:
		if (!roam_req->idle_roam_params.enable)
			return;
		break;
	case ROAM_SCAN_OFFLOAD_STOP:
		roam_req->idle_roam_params.enable = false;
		break;
	default:
		break;
	}

	db2dbm_enabled = wmi_service_enabled(wma_handle->wmi_handle,
					     wmi_service_hw_db2dbm_support);
	if (!db2dbm_enabled) {
		roam_req->idle_roam_params.conn_ap_min_rssi -=
					WMA_NOISE_FLOOR_DBM_DEFAULT;
		roam_req->idle_roam_params.conn_ap_min_rssi &= 0x000000ff;
	}

	status = wmi_unified_send_idle_roam_params(wma_handle->wmi_handle,
						   &roam_req->idle_roam_params);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to send idle roam parameters");
}

void
wma_send_roam_preauth_status(tp_wma_handle wma_handle,
			     struct wmi_roam_auth_status_params *params)
{
	QDF_STATUS status;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("WMA is closed, cannot send roam prauth status");
		return;
	}

	status = wmi_unified_send_roam_preauth_status(wma_handle->wmi_handle,
						      params);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to send disconnect roam preauth status");
}

#else
static inline void
wma_send_disconnect_roam_params(tp_wma_handle wma_handle,
				struct roam_offload_scan_req *roam_req)
{}

static inline void
wma_send_idle_roam_params(tp_wma_handle wma_handle,
			  struct roam_offload_scan_req *roam_req)
{}
#endif

/**
 * wma_process_roaming_config() - process roam request
 * @wma_handle: wma handle
 * @roam_req: roam request parameters
 *
 * Main routine to handle ROAM commands coming from CSR module.
 *
 * Return: QDF status
 */
QDF_STATUS wma_process_roaming_config(tp_wma_handle wma_handle,
				     struct roam_offload_scan_req *roam_req)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	wmi_start_scan_cmd_fixed_param scan_params;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t mode = 0;
	struct wma_txrx_node *intr = NULL;
	struct wmi_bss_load_config *bss_load_cfg;

	if (!mac) {
		WMA_LOGE("%s: mac is NULL", __func__);
		qdf_mem_zero(roam_req, sizeof(*roam_req));
		qdf_mem_free(roam_req);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("%s: RSO Command:%d, reason:%d session ID %d en %d", __func__,
		 roam_req->Command, roam_req->reason, roam_req->sessionId,
		 roam_req->RoamScanOffloadEnabled);
	wma_handle->interfaces[roam_req->sessionId].roaming_in_progress = false;
	switch (roam_req->Command) {
	case ROAM_SCAN_OFFLOAD_START:
		intr = &wma_handle->interfaces[roam_req->sessionId];
		intr->delay_before_vdev_stop = roam_req->delay_before_vdev_stop;
		/*
		 * Scan/Roam threshold parameters are translated from
		 * fields of struct roam_offload_scan_req to WMITLV
		 * values sent to Rome firmware some of these
		 * parameters are configurable in qcom_cfg.ini
		 */

		/* First param is positive rssi value to trigger rssi based scan
		 * Opportunistic scan is started at 30dB > trigger rssi.
		 */
		wma_handle->suitable_ap_hb_failure = false;

		qdf_status = wma_roam_scan_offload_rssi_thresh(wma_handle,
								roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;
		qdf_status = wma_roam_scan_bmiss_cnt(wma_handle,
					     roam_req->RoamBmissFirstBcnt,
					     roam_req->RoamBmissFinalBcnt,
					     roam_req->sessionId);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		/* Opportunistic scan runs on a timer, value set by
		 * EmptyRefreshScanPeriod. Age out the entries after 3 such
		 * cycles.
		 */
		if (roam_req->EmptyRefreshScanPeriod > 0) {
			qdf_status =
				wma_roam_scan_offload_scan_period(wma_handle,
								  roam_req);
			if (qdf_status != QDF_STATUS_SUCCESS)
				break;

			mode = WMI_ROAM_SCAN_MODE_PERIODIC;
			/* Don't use rssi triggered roam scans if external app
			 * is in control of channel list.
			 */
			if (roam_req->ChannelCacheType != CHANNEL_LIST_STATIC ||
			    roam_req->roam_force_rssi_trigger)
				mode |= WMI_ROAM_SCAN_MODE_RSSI_CHANGE;

		} else {
			mode = WMI_ROAM_SCAN_MODE_RSSI_CHANGE;
		}

		/* Start new rssi triggered scan only if it changes by
		 * RoamRssiDiff value. Beacon weight of 14 means average rssi
		 * is taken over 14 previous samples + 2 times the current
		 * beacon's rssi.
		 */
		qdf_status = wma_roam_scan_offload_rssi_change(wma_handle,
					roam_req->sessionId,
					roam_req->RoamRescanRssiDiff,
					roam_req->RoamBeaconRssiWeight,
					roam_req->hi_rssi_scan_delay);

		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		qdf_status = wma_roam_scan_offload_ap_profile(wma_handle,
						roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		qdf_status = wma_roam_scan_offload_chan_list(wma_handle,
				roam_req->ConnectedNetwork.ChannelCount,
				&roam_req->ConnectedNetwork.chan_freq_cache[0],
				roam_req->ChannelCacheType,
				roam_req->sessionId);
		if ((qdf_status != QDF_STATUS_SUCCESS) &&
			(qdf_status != QDF_STATUS_E_EMPTY))
			break;


		wma_roam_scan_fill_scan_params(wma_handle, mac, roam_req,
					       &scan_params);
		qdf_status =
			wma_roam_scan_offload_mode(wma_handle, &scan_params,
						   roam_req, mode,
						   roam_req->sessionId);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;
		if (wmi_service_enabled(wma_handle->wmi_handle,
					wmi_service_mawc_support)) {
			qdf_status =
				wma_roam_scan_mawc_params(wma_handle, roam_req);
			if (qdf_status != QDF_STATUS_SUCCESS) {
				WMA_LOGE("Sending roaming MAWC params failed");
				break;
			}
		} else {
			WMA_LOGD("MAWC roaming not supported by firmware");
		}
		qdf_status = wma_roam_scan_filter(wma_handle, roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS) {
			WMA_LOGE("Sending start for roam scan filter failed");
			break;
		}

		/* Send BTM config as disabled during RSO Stop */
		qdf_status = wma_roam_scan_btm_offload(wma_handle,
						       roam_req);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			WMA_LOGE("Sending BTM config to fw failed");
			break;
		}

		/*
		 * Send 11k offload enable and bss load trigger parameters
		 * to FW as part of RSO Start
		 */
		qdf_status = wma_send_offload_11k_params(wma_handle,
					&roam_req->offload_11k_params);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			WMA_LOGE("11k offload enable not sent, status %d",
				 qdf_status);
			break;
		}

		if (roam_req->bss_load_trig_enabled) {
			bss_load_cfg = &roam_req->bss_load_config;
			wma_send_roam_bss_load_config(wma_handle, bss_load_cfg);
		}

		wma_send_disconnect_roam_params(wma_handle, roam_req);
		wma_send_idle_roam_params(wma_handle, roam_req);
		break;

	case ROAM_SCAN_OFFLOAD_STOP:
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
		if (wma_is_roam_synch_in_progress(wma_handle,
				roam_req->sessionId) &&
				roam_req->reason ==
				REASON_ROAM_STOP_ALL) {
				WMA_LOGD("Dont send RSO stop during roam sync");
				break;
		}

		/*
		 * Send 11k offload disable command to FW as part of RSO Stop
		 */
		qdf_status =
		    wma_send_offload_11k_params(wma_handle,
						&roam_req->offload_11k_params);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			WMA_LOGE("11k offload disable not sent, status %d",
				 qdf_status);
			break;
		}

		/* Send BTM config as disabled during RSO Stop */
		qdf_status = wma_roam_scan_btm_offload(wma_handle, roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS) {
			WMA_LOGE(FL("Sending BTM config to fw failed"));
			break;
		}

		wma_handle->suitable_ap_hb_failure = false;

		wma_roam_scan_fill_scan_params(wma_handle, mac,
					       NULL, &scan_params);

		if (roam_req->reason == REASON_ROAM_STOP_ALL ||
		    roam_req->reason == REASON_DISCONNECTED ||
		    roam_req->reason == REASON_ROAM_SYNCH_FAILED) {
			mode = WMI_ROAM_SCAN_MODE_NONE;
		} else {
			if (csr_is_roam_offload_enabled(mac))
				mode = WMI_ROAM_SCAN_MODE_NONE |
				WMI_ROAM_SCAN_MODE_ROAMOFFLOAD;
			else
				mode = WMI_ROAM_SCAN_MODE_NONE;
		}

		qdf_status = wma_roam_scan_offload_mode(
					wma_handle,
					&scan_params, NULL, mode,
					roam_req->sessionId);
		/*
		 * After sending the roam scan mode because of a disconnect,
		 * clear the scan bitmap client as well by sending
		 * the following command
		 */
		wma_roam_scan_offload_rssi_thresh(wma_handle, roam_req);
		/*
		 * If the STOP command is due to a disconnect, then
		 * send the filter command to clear all the filter
		 * entries. If it is roaming scenario, then do not
		 * send the cleared entries.
		 */
		if (!roam_req->middle_of_roaming) {
			qdf_status = wma_roam_scan_filter(wma_handle, roam_req);
			if (qdf_status != QDF_STATUS_SUCCESS) {
				WMA_LOGE("clear for roam scan filter failed");
				break;
			}
		}

		wma_send_disconnect_roam_params(wma_handle, roam_req);
		wma_send_idle_roam_params(wma_handle, roam_req);

		if (roam_req->reason ==
		    REASON_OS_REQUESTED_ROAMING_NOW) {
			struct scheduler_msg cds_msg = {0};
			struct roam_offload_scan_rsp *scan_offload_rsp;

			scan_offload_rsp =
				qdf_mem_malloc(sizeof(*scan_offload_rsp));
			if (!scan_offload_rsp) {
				qdf_mem_free(roam_req);
				return QDF_STATUS_E_NOMEM;
			}
			cds_msg.type = eWNI_SME_ROAM_SCAN_OFFLOAD_RSP;
			scan_offload_rsp->sessionId = roam_req->sessionId;
			scan_offload_rsp->reason = roam_req->reason;
			cds_msg.bodyptr = scan_offload_rsp;
			/*
			 * Since REASSOC request is processed in
			 * Roam_Scan_Offload_Rsp post a dummy rsp msg back to
			 * SME with proper reason code.
			 */
			if (QDF_STATUS_SUCCESS !=
			    scheduler_post_message(QDF_MODULE_ID_WMA,
						   QDF_MODULE_ID_SME,
						   QDF_MODULE_ID_SME,
						   &cds_msg)) {
				qdf_mem_free(scan_offload_rsp);
			}
		}
		break;

	case ROAM_SCAN_OFFLOAD_ABORT_SCAN:
		/* If roam scan is running, stop that cycle.
		 * It will continue automatically on next trigger.
		 */
		qdf_status = wma_roam_scan_offload_command(wma_handle,
						   WMI_ROAM_SCAN_STOP_CMD,
						   roam_req->sessionId);
		break;

	case ROAM_SCAN_OFFLOAD_RESTART:
		/* Rome offload engine does not stop after any scan.
		 * If this command is sent because all preauth attempts failed
		 * and WMI_ROAM_REASON_SUITABLE_AP event was received earlier,
		 * now it is time to call it heartbeat failure.
		 */
		if ((roam_req->reason == REASON_PREAUTH_FAILED_FOR_ALL)
		    && wma_handle->suitable_ap_hb_failure) {
			WMA_LOGE("%s: Sending heartbeat failure after preauth failures",
				__func__);
			wma_beacon_miss_handler(wma_handle,
				roam_req->sessionId,
				wma_handle->suitable_ap_hb_failure_rssi);
			wma_handle->suitable_ap_hb_failure = false;
		}
		break;

	case ROAM_SCAN_OFFLOAD_UPDATE_CFG:
		wma_handle->suitable_ap_hb_failure = false;

		qdf_status = wma_roam_scan_bmiss_cnt(wma_handle,
					     roam_req->RoamBmissFirstBcnt,
					     roam_req->RoamBmissFinalBcnt,
					     roam_req->sessionId);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		qdf_status = wma_roam_scan_filter(wma_handle, roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS) {
			WMA_LOGE("Sending update for roam scan filter failed");
			break;
		}


		/*
		 * Runtime (after association) changes to rssi thresholds and
		 * other parameters.
		 */
		qdf_status = wma_roam_scan_offload_chan_list(wma_handle,
			     roam_req->ConnectedNetwork.ChannelCount,
			     &roam_req->ConnectedNetwork.chan_freq_cache[0],
			     roam_req->ChannelCacheType,
			     roam_req->sessionId);
		/*
		 * Even though the channel list is empty, we can
		 * still go ahead and start Roaming.
		 */
		if ((qdf_status != QDF_STATUS_SUCCESS) &&
			(qdf_status != QDF_STATUS_E_EMPTY))
			break;


		qdf_status = wma_roam_scan_offload_rssi_thresh(wma_handle,
							       roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		if (roam_req->EmptyRefreshScanPeriod > 0) {
			qdf_status = wma_roam_scan_offload_scan_period(
						wma_handle, roam_req);
			if (qdf_status != QDF_STATUS_SUCCESS)
				break;

			mode = WMI_ROAM_SCAN_MODE_PERIODIC;
			/* Don't use rssi triggered roam scans if external app
			 * is in control of channel list.
			 */
			if (roam_req->ChannelCacheType != CHANNEL_LIST_STATIC ||
			    roam_req->roam_force_rssi_trigger)
				mode |= WMI_ROAM_SCAN_MODE_RSSI_CHANGE;

		} else {
			mode = WMI_ROAM_SCAN_MODE_RSSI_CHANGE;
		}

		qdf_status = wma_roam_scan_offload_rssi_change(wma_handle,
				    roam_req->sessionId,
				    roam_req->RoamRescanRssiDiff,
				    roam_req->RoamBeaconRssiWeight,
				    roam_req->hi_rssi_scan_delay);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		qdf_status = wma_roam_scan_offload_ap_profile(wma_handle,
					roam_req);
		if (qdf_status != QDF_STATUS_SUCCESS)
			break;

		if (!roam_req->RoamScanOffloadEnabled)
			break;

		wma_roam_scan_fill_scan_params(wma_handle, mac, roam_req,
					       &scan_params);
		qdf_status =
			wma_roam_scan_offload_mode(wma_handle, &scan_params,
						   roam_req, mode,
						   roam_req->sessionId);

		wma_send_disconnect_roam_params(wma_handle, roam_req);
		wma_send_idle_roam_params(wma_handle, roam_req);

		break;

	default:
		break;
	}
	qdf_mem_zero(roam_req, sizeof(*roam_req));
	qdf_mem_free(roam_req);
	return qdf_status;
}

void wma_update_per_roam_config(WMA_HANDLE handle,
				struct wmi_per_roam_config_req *req_buf)
{
	QDF_STATUS status;
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, cannot send per roam config",
			__func__);
		return;
	}

	status = wmi_unified_set_per_roam_config(wma_handle->wmi_handle,
						 req_buf);
	if (QDF_IS_STATUS_ERROR(status))
		wma_err("failed to set per roam config to FW");
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD

/**
 * wma_process_roam_invoke() - send roam invoke command to fw.
 * @handle: wma handle
 * @roaminvoke: roam invoke command
 *
 * Send roam invoke command to fw for fastreassoc.
 *
 * Return: none
 */
void wma_process_roam_invoke(WMA_HANDLE handle,
		struct wma_roam_invoke_cmd *roaminvoke)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	uint32_t ch_hz;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not send roam invoke",
				__func__);
		goto free_frame_buf;
	}

	if (!wma_is_vdev_valid(roaminvoke->vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__,
			 roaminvoke->vdev_id);
		goto free_frame_buf;
	}
	ch_hz = roaminvoke->ch_freq;
	wmi_unified_roam_invoke_cmd(wma_handle->wmi_handle,
				(struct wmi_roam_invoke_cmd *)roaminvoke,
				ch_hz);
free_frame_buf:
	if (roaminvoke->frame_len) {
		qdf_mem_free(roaminvoke->frame_buf);
		roaminvoke->frame_buf = NULL;
	}
}

/**
 * wma_process_roam_synch_fail() -roam synch failure handle
 * @handle: wma handle
 * @synch_fail: roam synch fail parameters
 *
 * Return: none
 */
void wma_process_roam_synch_fail(WMA_HANDLE handle,
				 struct roam_offload_synch_fail *synch_fail)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not clean-up roam synch",
			__func__);
		return;
	}
	wlan_roam_debug_log(synch_fail->session_id,
			    DEBUG_ROAM_SYNCH_FAIL,
			    DEBUG_INVALID_PEER_ID, NULL, NULL, 0, 0);

	/* Hand Off Failure could happen as an exception, when a roam synch
	 * indication is posted to Host, but a roam synch complete is not
	 * posted to the firmware.So, clear the roam synch in progress
	 * flag before disconnecting the session through this event.
	 */
	wma_handle->interfaces[synch_fail->session_id].roam_synch_in_progress =
		false;
}

/**
 * wma_free_roam_synch_frame_ind() - Free the bcn_probe_rsp, reassoc_req,
 * reassoc_rsp received as part of the ROAM_SYNC_FRAME event
 *
 * @iface - interaface corresponding to a vdev
 *
 * This API is used to free the buffer allocated during the ROAM_SYNC_FRAME
 * event
 *
 */
static void wma_free_roam_synch_frame_ind(struct wma_txrx_node *iface)
{
	if (iface->roam_synch_frame_ind.bcn_probe_rsp) {
		qdf_mem_free(iface->roam_synch_frame_ind.bcn_probe_rsp);
		iface->roam_synch_frame_ind.bcn_probe_rsp_len = 0;
		iface->roam_synch_frame_ind.bcn_probe_rsp = NULL;
	}
	if (iface->roam_synch_frame_ind.reassoc_req) {
		qdf_mem_free(iface->roam_synch_frame_ind.reassoc_req);
		iface->roam_synch_frame_ind.reassoc_req_len = 0;
		iface->roam_synch_frame_ind.reassoc_req = NULL;
	}
	if (iface->roam_synch_frame_ind.reassoc_rsp) {
		qdf_mem_free(iface->roam_synch_frame_ind.reassoc_rsp);
		iface->roam_synch_frame_ind.reassoc_rsp_len = 0;
		iface->roam_synch_frame_ind.reassoc_rsp = NULL;
	}
}

/**
 * wma_fill_data_synch_frame_event() - Fill the the roam sync data buffer using
 * synch frame event data
 * @wma: Global WMA Handle
 * @roam_synch_ind_ptr: Buffer to be filled
 * @param_buf: Source buffer
 *
 * Firmware sends all the required information required for roam
 * synch propagation as TLV's and stored in param_buf. These
 * parameters are parsed and filled into the roam synch indication
 * buffer which will be used at different layers for propagation.
 *
 * Return: None
 */
static void wma_fill_data_synch_frame_event(tp_wma_handle wma,
				struct roam_offload_synch_ind *roam_synch_ind_ptr,
				struct wma_txrx_node *iface)
{
	uint8_t *bcn_probersp_ptr;
	uint8_t *reassoc_rsp_ptr;
	uint8_t *reassoc_req_ptr;

	/* Beacon/Probe Rsp data */
	roam_synch_ind_ptr->beaconProbeRespOffset =
		sizeof(struct roam_offload_synch_ind);
	bcn_probersp_ptr = (uint8_t *) roam_synch_ind_ptr +
		roam_synch_ind_ptr->beaconProbeRespOffset;
	roam_synch_ind_ptr->beaconProbeRespLength =
		iface->roam_synch_frame_ind.bcn_probe_rsp_len;
	qdf_mem_copy(bcn_probersp_ptr,
		iface->roam_synch_frame_ind.bcn_probe_rsp,
		roam_synch_ind_ptr->beaconProbeRespLength);
	qdf_mem_free(iface->roam_synch_frame_ind.bcn_probe_rsp);
		iface->roam_synch_frame_ind.bcn_probe_rsp = NULL;

	/* ReAssoc Rsp data */
	roam_synch_ind_ptr->reassocRespOffset =
		sizeof(struct roam_offload_synch_ind) +
		roam_synch_ind_ptr->beaconProbeRespLength;
	roam_synch_ind_ptr->reassocRespLength =
		iface->roam_synch_frame_ind.reassoc_rsp_len;
	reassoc_rsp_ptr = (uint8_t *) roam_synch_ind_ptr +
			  roam_synch_ind_ptr->reassocRespOffset;
	qdf_mem_copy(reassoc_rsp_ptr,
		     iface->roam_synch_frame_ind.reassoc_rsp,
		     roam_synch_ind_ptr->reassocRespLength);
	qdf_mem_free(iface->roam_synch_frame_ind.reassoc_rsp);
	iface->roam_synch_frame_ind.reassoc_rsp = NULL;

	/* ReAssoc Req data */
	roam_synch_ind_ptr->reassoc_req_offset =
		sizeof(struct roam_offload_synch_ind) +
		roam_synch_ind_ptr->beaconProbeRespLength +
		roam_synch_ind_ptr->reassocRespLength;
	roam_synch_ind_ptr->reassoc_req_length =
		iface->roam_synch_frame_ind.reassoc_req_len;
	reassoc_req_ptr = (uint8_t *) roam_synch_ind_ptr +
			  roam_synch_ind_ptr->reassoc_req_offset;
	qdf_mem_copy(reassoc_req_ptr,
		     iface->roam_synch_frame_ind.reassoc_req,
		     roam_synch_ind_ptr->reassoc_req_length);
	qdf_mem_free(iface->roam_synch_frame_ind.reassoc_req);
	iface->roam_synch_frame_ind.reassoc_req = NULL;
}

/**
 * wma_fill_data_synch_event() - Fill the the roam sync data buffer using synch
 * event data
 * @wma: Global WMA Handle
 * @roam_synch_ind_ptr: Buffer to be filled
 * @param_buf: Source buffer
 *
 * Firmware sends all the required information required for roam
 * synch propagation as TLV's and stored in param_buf. These
 * parameters are parsed and filled into the roam synch indication
 * buffer which will be used at different layers for propagation.
 *
 * Return: None
 */
static void wma_fill_data_synch_event(tp_wma_handle wma,
				struct roam_offload_synch_ind *roam_synch_ind_ptr,
				WMI_ROAM_SYNCH_EVENTID_param_tlvs *param_buf)
{
	uint8_t *bcn_probersp_ptr;
	uint8_t *reassoc_rsp_ptr;
	uint8_t *reassoc_req_ptr;
	wmi_roam_synch_event_fixed_param *synch_event;

	synch_event = param_buf->fixed_param;

	/* Beacon/Probe Rsp data */
	roam_synch_ind_ptr->beaconProbeRespOffset =
		sizeof(struct roam_offload_synch_ind);
	bcn_probersp_ptr = (uint8_t *) roam_synch_ind_ptr +
		roam_synch_ind_ptr->beaconProbeRespOffset;
	roam_synch_ind_ptr->beaconProbeRespLength =
		synch_event->bcn_probe_rsp_len;
	qdf_mem_copy(bcn_probersp_ptr, param_buf->bcn_probe_rsp_frame,
		     roam_synch_ind_ptr->beaconProbeRespLength);
	/* ReAssoc Rsp data */
	roam_synch_ind_ptr->reassocRespOffset =
		sizeof(struct roam_offload_synch_ind) +
		roam_synch_ind_ptr->beaconProbeRespLength;
	roam_synch_ind_ptr->reassocRespLength = synch_event->reassoc_rsp_len;
	reassoc_rsp_ptr = (uint8_t *) roam_synch_ind_ptr +
			  roam_synch_ind_ptr->reassocRespOffset;
	qdf_mem_copy(reassoc_rsp_ptr,
		     param_buf->reassoc_rsp_frame,
		     roam_synch_ind_ptr->reassocRespLength);

	/* ReAssoc Req data */
	roam_synch_ind_ptr->reassoc_req_offset =
		sizeof(struct roam_offload_synch_ind) +
		roam_synch_ind_ptr->beaconProbeRespLength +
		roam_synch_ind_ptr->reassocRespLength;
	roam_synch_ind_ptr->reassoc_req_length = synch_event->reassoc_req_len;
	reassoc_req_ptr = (uint8_t *) roam_synch_ind_ptr +
			  roam_synch_ind_ptr->reassoc_req_offset;
	qdf_mem_copy(reassoc_req_ptr, param_buf->reassoc_req_frame,
		     roam_synch_ind_ptr->reassoc_req_length);
}

/**
 * wma_fill_roam_synch_buffer() - Fill the the roam sync buffer
 * @wma: Global WMA Handle
 * @roam_synch_ind_ptr: Buffer to be filled
 * @param_buf: Source buffer
 *
 * Firmware sends all the required information required for roam
 * synch propagation as TLV's and stored in param_buf. These
 * parameters are parsed and filled into the roam synch indication
 * buffer which will be used at different layers for propagation.
 *
 * Return: Success or Failure
 */
static int wma_fill_roam_synch_buffer(tp_wma_handle wma,
				struct roam_offload_synch_ind *roam_synch_ind_ptr,
				WMI_ROAM_SYNCH_EVENTID_param_tlvs *param_buf)
{
	wmi_roam_synch_event_fixed_param *synch_event;
	wmi_channel *chan;
	wmi_key_material *key;
	wmi_key_material_ext *key_ft;
	struct wma_txrx_node *iface = NULL;
	wmi_roam_fils_synch_tlv_param *fils_info;
	int status = -EINVAL;
	uint8_t kck_len;
	uint8_t kek_len;

	synch_event = param_buf->fixed_param;
	roam_synch_ind_ptr->roamed_vdev_id = synch_event->vdev_id;
	roam_synch_ind_ptr->authStatus = synch_event->auth_status;
	roam_synch_ind_ptr->roamReason = synch_event->roam_reason;
	roam_synch_ind_ptr->rssi = synch_event->rssi;
	iface = &wma->interfaces[synch_event->vdev_id];
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&synch_event->bssid,
				   roam_synch_ind_ptr->bssid.bytes);
	WMA_LOGD("%s: roamedVdevId %d authStatus %d roamReason %d rssi %d isBeacon %d",
		__func__, roam_synch_ind_ptr->roamed_vdev_id,
		roam_synch_ind_ptr->authStatus, roam_synch_ind_ptr->roamReason,
		roam_synch_ind_ptr->rssi, roam_synch_ind_ptr->isBeacon);

	if (!QDF_IS_STATUS_SUCCESS(
		wma->csr_roam_synch_cb(wma->mac_context, roam_synch_ind_ptr,
				       NULL, SIR_ROAMING_DEREGISTER_STA))) {
		WMA_LOGE("LFR3: CSR Roam synch cb failed");
		wma_free_roam_synch_frame_ind(iface);
		return status;
	}

	/*
	 * If lengths of bcn_probe_rsp, reassoc_req and reassoc_rsp are zero in
	 * synch_event driver would have received bcn_probe_rsp, reassoc_req
	 * and reassoc_rsp via the event WMI_ROAM_SYNCH_FRAME_EVENTID
	 */
	if ((!synch_event->bcn_probe_rsp_len) &&
		(!synch_event->reassoc_req_len) &&
		(!synch_event->reassoc_rsp_len)) {
		if (!iface->roam_synch_frame_ind.bcn_probe_rsp) {
			WMA_LOGE("LFR3: bcn_probe_rsp is NULL");
			QDF_ASSERT(iface->roam_synch_frame_ind.
				   bcn_probe_rsp);
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}
		if (!iface->roam_synch_frame_ind.reassoc_rsp) {
			WMA_LOGE("LFR3: reassoc_rsp is NULL");
			QDF_ASSERT(iface->roam_synch_frame_ind.
				   reassoc_rsp);
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}
		if (!iface->roam_synch_frame_ind.reassoc_req) {
			WMA_LOGE("LFR3: reassoc_req is NULL");
			QDF_ASSERT(iface->roam_synch_frame_ind.
				   reassoc_req);
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}
		wma_fill_data_synch_frame_event(wma, roam_synch_ind_ptr, iface);
	} else {
		wma_fill_data_synch_event(wma, roam_synch_ind_ptr, param_buf);
	}

	chan = param_buf->chan;
	if (chan)
		roam_synch_ind_ptr->chan_freq = chan->mhz;

	key = param_buf->key;
	key_ft = param_buf->key_ext;
	if (key) {
		roam_synch_ind_ptr->kck_len = SIR_KCK_KEY_LEN;
		qdf_mem_copy(roam_synch_ind_ptr->kck, key->kck,
			     SIR_KCK_KEY_LEN);
		roam_synch_ind_ptr->kek_len = SIR_KEK_KEY_LEN;
		qdf_mem_copy(roam_synch_ind_ptr->kek, key->kek,
			     SIR_KEK_KEY_LEN);
		qdf_mem_copy(roam_synch_ind_ptr->replay_ctr,
			     key->replay_counter, SIR_REPLAY_CTR_LEN);
	} else if (key_ft) {
		/*
		 * For AKM 00:0F:AC (FT suite-B-SHA384)
		 * KCK-bits:192 KEK-bits:256
		 * Firmware sends wmi_key_material_ext tlv now only if
		 * auth is FT Suite-B SHA-384 auth. If further new suites
		 * are added, add logic to get kck, kek bits based on
		 * akm protocol
		 */
		kck_len = KCK_192BIT_KEY_LEN;
		kek_len = KEK_256BIT_KEY_LEN;

		roam_synch_ind_ptr->kek_len = kek_len;
		qdf_mem_copy(roam_synch_ind_ptr->kek,
			     key_ft->key_buffer, kek_len);

		roam_synch_ind_ptr->kck_len = kck_len;
		qdf_mem_copy(roam_synch_ind_ptr->kck,
			     (key_ft->key_buffer + kek_len),
			     kck_len);

		qdf_mem_copy(roam_synch_ind_ptr->replay_ctr,
			     (key_ft->key_buffer + kek_len + kck_len),
			     SIR_REPLAY_CTR_LEN);
	}

	if (param_buf->hw_mode_transition_fixed_param)
		wma_process_pdev_hw_mode_trans_ind(wma,
		    param_buf->hw_mode_transition_fixed_param,
		    param_buf->wmi_pdev_set_hw_mode_response_vdev_mac_mapping,
		    &roam_synch_ind_ptr->hw_mode_trans_ind);
	else
		WMA_LOGD(FL("hw_mode transition fixed param is NULL"));

	fils_info = param_buf->roam_fils_synch_info;
	if (fils_info) {
		if ((fils_info->kek_len > SIR_KEK_KEY_LEN_FILS) ||
		    (fils_info->pmk_len > SIR_PMK_LEN)) {
			WMA_LOGE("%s: Invalid kek_len %d or pmk_len %d",
				 __func__,
				 fils_info->kek_len,
				 fils_info->pmk_len);
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}

		roam_synch_ind_ptr->kek_len = fils_info->kek_len;
		qdf_mem_copy(roam_synch_ind_ptr->kek, fils_info->kek,
			     fils_info->kek_len);

		roam_synch_ind_ptr->pmk_len = fils_info->pmk_len;
		qdf_mem_copy(roam_synch_ind_ptr->pmk, fils_info->pmk,
			     fils_info->pmk_len);

		qdf_mem_copy(roam_synch_ind_ptr->pmkid, fils_info->pmkid,
			     PMKID_LEN);

		roam_synch_ind_ptr->update_erp_next_seq_num =
				fils_info->update_erp_next_seq_num;
		roam_synch_ind_ptr->next_erp_seq_num =
				fils_info->next_erp_seq_num;

		WMA_LOGD("Update ERP Seq Num %d, Next ERP Seq Num %d",
			 roam_synch_ind_ptr->update_erp_next_seq_num,
			 roam_synch_ind_ptr->next_erp_seq_num);
	}
	wma_free_roam_synch_frame_ind(iface);
	return 0;
}

#ifdef CRYPTO_SET_KEY_CONVERGED
static void wma_update_roamed_peer_unicast_cipher(tp_wma_handle wma,
						  uint32_t uc_cipher,
						  uint32_t cipher_cap,
						  uint8_t *peer_mac)
{
	struct wlan_objmgr_peer *peer;

	if (!peer_mac) {
		wma_err("wma ctx or peer mac is NULL");
		return;
	}

	peer = wlan_objmgr_get_peer(wma->psoc,
				    wlan_objmgr_pdev_get_pdev_id(wma->pdev),
				    peer_mac, WLAN_LEGACY_WMA_ID);
	if (!peer) {
		wma_err("Peer of peer_mac %pM not found", peer_mac);
		return;
	}
	wlan_crypto_set_peer_param(peer, WLAN_CRYPTO_PARAM_CIPHER_CAP,
				   cipher_cap);
	wlan_crypto_set_peer_param(peer, WLAN_CRYPTO_PARAM_UCAST_CIPHER,
				   uc_cipher);

	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);

	wma_debug("Set unicast cipher %x and cap %x for %pM", uc_cipher,
		  cipher_cap, peer_mac);
}

static void wma_get_peer_uc_cipher(tp_wma_handle wma, uint8_t *peer_mac,
				   uint32_t *uc_cipher, uint32_t *cipher_cap)
{
	uint32_t cipher, cap;
	struct wlan_objmgr_peer *peer;

	if (!peer_mac) {
		WMA_LOGE("wma ctx or peer_mac is NULL");
		return;
	}
	peer = wlan_objmgr_get_peer(wma->psoc,
				    wlan_objmgr_pdev_get_pdev_id(wma->pdev),
				    peer_mac, WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE("Peer of peer_mac %pM not found", peer_mac);
		return;
	}

	cipher = wlan_crypto_get_peer_param(peer,
					    WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	cap = wlan_crypto_get_peer_param(peer, WLAN_CRYPTO_PARAM_CIPHER_CAP);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);

	if (uc_cipher)
		*uc_cipher = cipher;
	if (cipher_cap)
		*cipher_cap = cap;
}

#else

static void wma_update_roamed_peer_unicast_cipher(tp_wma_handle wma,
						  uint32_t uc_cipher,
						  uint32_t cipher_cap,
						  uint8_t *peer_mac)
{
	struct wlan_objmgr_peer *peer;

	if (!peer_mac) {
		WMA_LOGE("peer_mac is NULL");
		return;
	}

	peer = wlan_objmgr_get_peer(wma->psoc,
				    wlan_objmgr_pdev_get_pdev_id(wma->pdev),
				    peer_mac, WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE("Peer of peer_mac %pM not found", peer_mac);
		return;
	}

	wlan_peer_set_unicast_cipher(peer, uc_cipher);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);

	wma_debug("Set unicast cipher %d for %pM", uc_cipher, peer_mac);
}

static void wma_get_peer_uc_cipher(tp_wma_handle wma, uint8_t *peer_mac,
				   uint32_t *uc_cipher, uint32_t *cipher_cap)
{
	uint32_t cipher;
	struct wlan_objmgr_peer *peer;

	if (!peer_mac) {
		WMA_LOGE("peer_mac is NULL");
		return;
	}
	peer = wlan_objmgr_get_peer(wma->psoc,
				    wlan_objmgr_pdev_get_pdev_id(wma->pdev),
				    peer_mac, WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE("Peer of peer_mac %pM not found", peer_mac);
		return;
	}

	cipher = wlan_peer_get_unicast_cipher(peer);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);

	if (uc_cipher)
		*uc_cipher = cipher;
}
#endif

/**
 * wma_roam_update_vdev() - Update the STA and BSS
 * @wma: Global WMA Handle
 * @roam_synch_ind_ptr: Information needed for roam sync propagation
 *
 * This function will perform all the vdev related operations with
 * respect to the self sta and the peer after roaming and completes
 * the roam synch propagation with respect to WMA layer.
 *
 * Return: None
 */
static void
wma_roam_update_vdev(tp_wma_handle wma,
		     struct roam_offload_synch_ind *roam_synch_ind_ptr)
{
	tDeleteStaParams *del_sta_params;
	tAddStaParams *add_sta_params;
	uint32_t uc_cipher = 0, cipher_cap = 0;
	uint8_t vdev_id, *bssid;

	vdev_id = roam_synch_ind_ptr->roamed_vdev_id;
	wma->interfaces[vdev_id].nss = roam_synch_ind_ptr->nss;

	del_sta_params = qdf_mem_malloc(sizeof(*del_sta_params));
	if (!del_sta_params) {
		return;
	}

	add_sta_params = qdf_mem_malloc(sizeof(*add_sta_params));
	if (!add_sta_params) {
		qdf_mem_free(del_sta_params);
		return;
	}

	qdf_mem_zero(del_sta_params, sizeof(*del_sta_params));
	qdf_mem_zero(add_sta_params, sizeof(*add_sta_params));

	del_sta_params->smesessionId = vdev_id;
	add_sta_params->staType = STA_ENTRY_SELF;
	add_sta_params->smesessionId = vdev_id;
	qdf_mem_copy(&add_sta_params->bssId, &roam_synch_ind_ptr->bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	add_sta_params->assocId = roam_synch_ind_ptr->aid;

	bssid = wma_get_vdev_bssid(wma->interfaces[vdev_id].vdev);
	if (!bssid) {
		WMA_LOGE("%s: Failed to get bssid for vdev_%d",
			 __func__, vdev_id);
		return;
	}

	/*
	 * Get uc cipher of old peer to update new peer as it doesnt
	 * change in roaming
	 */
	wma_get_peer_uc_cipher(wma, bssid,
			       &uc_cipher, &cipher_cap);

	wma_delete_sta(wma, del_sta_params);
	wma_delete_bss(wma, vdev_id);
	wma_add_bss_peer_sta(roam_synch_ind_ptr->self_mac.bytes,
			     roam_synch_ind_ptr->bssid.bytes, true);
	/* Update new peer's uc cipher */
	wma_update_roamed_peer_unicast_cipher(wma, uc_cipher, cipher_cap,
					      roam_synch_ind_ptr->bssid.bytes);
	wma_add_bss_lfr3(wma, roam_synch_ind_ptr->add_bss_params);
	wma_add_sta(wma, add_sta_params);
	qdf_mem_copy(bssid, roam_synch_ind_ptr->bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_free(add_sta_params);
}

static void wma_update_phymode_on_roam(tp_wma_handle wma, uint8_t *bssid,
				       wmi_channel *chan,
				       struct wma_txrx_node *iface)
{
	enum wlan_phymode bss_phymode;
	struct wlan_channel *des_chan;
	struct vdev_mlme_obj *vdev_mlme;
	uint8_t channel;

	channel = wlan_freq_to_chan(iface->mhz);
	if (chan)
		bss_phymode =
			wma_fw_to_host_phymode(WMI_GET_CHANNEL_MODE(chan));
	else
		wma_get_phy_mode_cb(channel, iface->chan_width, &bss_phymode);

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
	des_chan = wlan_vdev_mlme_get_des_chan(iface->vdev);

	des_chan->ch_phymode = bss_phymode;
	/* Till conversion is not done in WMI we need to fill fw phy mode */
	vdev_mlme->mgmt.generic.phy_mode = wma_host_to_fw_phymode(bss_phymode);

	/* update new phymode to peer */
	wma_objmgr_set_peer_mlme_phymode(wma, bssid, bss_phymode);

	WMA_LOGD("LFR3: new phymode %d", bss_phymode);
}

int wma_mlme_roam_synch_event_handler_cb(void *handle, uint8_t *event,
					 uint32_t len)
{
	WMI_ROAM_SYNCH_EVENTID_param_tlvs *param_buf = NULL;
	wmi_roam_synch_event_fixed_param *synch_event = NULL;
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct roam_offload_synch_ind *roam_synch_ind_ptr = NULL;
	struct bss_description *bss_desc_ptr = NULL;
	uint16_t ie_len = 0;
	int status = -EINVAL;
	struct roam_offload_scan_req *roam_req;
	qdf_time_t roam_synch_received = qdf_get_system_timestamp();
	uint32_t roam_synch_data_len;
	A_UINT32 bcn_probe_rsp_len;
	A_UINT32 reassoc_rsp_len;
	A_UINT32 reassoc_req_len;

	WMA_LOGD("LFR3:%s", __func__);
	if (!event) {
		WMA_LOGE("%s: event param null", __func__);
		goto cleanup_label;
	}

	param_buf = (WMI_ROAM_SYNCH_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("%s: received null buf from target", __func__);
		goto cleanup_label;
	}

	synch_event = param_buf->fixed_param;
	if (!synch_event) {
		WMA_LOGE("%s: received null event data from target", __func__);
		goto cleanup_label;
	}

	if (synch_event->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: received invalid vdev_id %d",
				__func__, synch_event->vdev_id);
		return status;
	}

	/*
	 * This flag is set during ROAM_START and once this event is being
	 * executed which is a run to completion, no other event can interrupt
	 * this in MC thread context. So, it is OK to reset the flag here as
	 * soon as we receive this event.
	 */
	wma->interfaces[synch_event->vdev_id].roaming_in_progress = false;

	if (synch_event->bcn_probe_rsp_len >
	    param_buf->num_bcn_probe_rsp_frame ||
	    synch_event->reassoc_req_len >
	    param_buf->num_reassoc_req_frame ||
	    synch_event->reassoc_rsp_len >
	    param_buf->num_reassoc_rsp_frame) {
		WMA_LOGD("Invalid synch payload: LEN bcn:%d, req:%d, rsp:%d",
			synch_event->bcn_probe_rsp_len,
			synch_event->reassoc_req_len,
			synch_event->reassoc_rsp_len);
		goto cleanup_label;
	}

	wlan_roam_debug_log(synch_event->vdev_id, DEBUG_ROAM_SYNCH_IND,
			    DEBUG_INVALID_PEER_ID, NULL, NULL,
			    synch_event->bssid.mac_addr31to0,
			    synch_event->bssid.mac_addr47to32);
	DPTRACE(qdf_dp_trace_record_event(QDF_DP_TRACE_EVENT_RECORD,
		synch_event->vdev_id, QDF_TRACE_DEFAULT_PDEV_ID,
		QDF_PROTO_TYPE_EVENT, QDF_ROAM_SYNCH));

	if (wma_is_roam_synch_in_progress(wma, synch_event->vdev_id)) {
		WMA_LOGE("%s: Ignoring RSI since one is already in progress",
				__func__);
		goto cleanup_label;
	}
	WMA_LOGD("LFR3: Received WMA_ROAM_OFFLOAD_SYNCH_IND");

	/*
	 * All below length fields are unsigned and hence positive numbers.
	 * Maximum number during the addition would be (3 * MAX_LIMIT(UINT32) +
	 * few fixed fields).
	 */
	WMA_LOGD("synch payload: LEN bcn:%d, req:%d, rsp:%d",
			synch_event->bcn_probe_rsp_len,
			synch_event->reassoc_req_len,
			synch_event->reassoc_rsp_len);

	/*
	 * If lengths of bcn_probe_rsp, reassoc_req and reassoc_rsp are zero in
	 * synch_event driver would have received bcn_probe_rsp, reassoc_req
	 * and reassoc_rsp via the event WMI_ROAM_SYNCH_FRAME_EVENTID
	 */
	if ((!synch_event->bcn_probe_rsp_len) &&
		(!synch_event->reassoc_req_len) &&
		(!synch_event->reassoc_rsp_len)) {
		bcn_probe_rsp_len = wma->interfaces[synch_event->vdev_id].
				      roam_synch_frame_ind.
				      bcn_probe_rsp_len;
		reassoc_req_len = wma->interfaces[synch_event->vdev_id].
				      roam_synch_frame_ind.reassoc_req_len;
		reassoc_rsp_len = wma->interfaces[synch_event->vdev_id].
				      roam_synch_frame_ind.reassoc_rsp_len;

		roam_synch_data_len = bcn_probe_rsp_len + reassoc_rsp_len +
			reassoc_req_len + sizeof(struct roam_offload_synch_ind);

		WMA_LOGD("Updated synch payload: LEN bcn:%d, req:%d, rsp:%d",
			bcn_probe_rsp_len,
			reassoc_req_len,
			reassoc_rsp_len);
	} else {
		bcn_probe_rsp_len = synch_event->bcn_probe_rsp_len;
		reassoc_req_len = synch_event->reassoc_req_len;
		reassoc_rsp_len = synch_event->reassoc_rsp_len;

		if (synch_event->bcn_probe_rsp_len > WMI_SVC_MSG_MAX_SIZE)
			goto cleanup_label;
		if (synch_event->reassoc_rsp_len >
		    (WMI_SVC_MSG_MAX_SIZE - synch_event->bcn_probe_rsp_len))
			goto cleanup_label;
		if (synch_event->reassoc_req_len >
		    WMI_SVC_MSG_MAX_SIZE - (synch_event->bcn_probe_rsp_len +
			synch_event->reassoc_rsp_len))
			goto cleanup_label;

		roam_synch_data_len = bcn_probe_rsp_len +
			reassoc_rsp_len + reassoc_req_len;

		/*
		 * Below is the check for the entire size of the message
		 * received from the firmware.
		 */
		if (roam_synch_data_len > WMI_SVC_MSG_MAX_SIZE -
			(sizeof(*synch_event) + sizeof(wmi_channel) +
			 sizeof(wmi_key_material) + sizeof(uint32_t)))
			goto cleanup_label;

		roam_synch_data_len += sizeof(struct roam_offload_synch_ind);
	}

	WMA_LOGD("synch payload: LEN bcn:%d, req:%d, rsp:%d",
			bcn_probe_rsp_len,
			reassoc_req_len,
			reassoc_rsp_len);

	cds_host_diag_log_work(&wma->roam_ho_wl,
			       WMA_ROAM_HO_WAKE_LOCK_DURATION,
			       WIFI_POWER_EVENT_WAKELOCK_WOW);
	qdf_wake_lock_timeout_acquire(&wma->roam_ho_wl,
				      WMA_ROAM_HO_WAKE_LOCK_DURATION);

	wma->interfaces[synch_event->vdev_id].roam_synch_in_progress = true;

	roam_synch_ind_ptr = qdf_mem_malloc(roam_synch_data_len);
	if (!roam_synch_ind_ptr) {
		QDF_ASSERT(roam_synch_ind_ptr);
		status = -ENOMEM;
		goto cleanup_label;
	}
	qdf_mem_zero(roam_synch_ind_ptr, roam_synch_data_len);
	status = wma_fill_roam_synch_buffer(wma,
			roam_synch_ind_ptr, param_buf);
	if (status != 0)
		goto cleanup_label;
	/* 24 byte MAC header and 12 byte to ssid IE */
	if (roam_synch_ind_ptr->beaconProbeRespLength >
			(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET)) {
		ie_len = roam_synch_ind_ptr->beaconProbeRespLength -
			(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET);
	} else {
		WMA_LOGE("LFR3: Invalid Beacon Length");
		goto cleanup_label;
	}
	bss_desc_ptr = qdf_mem_malloc(sizeof(struct bss_description) + ie_len);
	if (!bss_desc_ptr) {
		QDF_ASSERT(bss_desc_ptr);
		status = -ENOMEM;
		goto cleanup_label;
	}
	qdf_mem_zero(bss_desc_ptr, sizeof(struct bss_description) + ie_len);
	if (QDF_IS_STATUS_ERROR(wma->pe_roam_synch_cb(wma->mac_context,
			roam_synch_ind_ptr, bss_desc_ptr,
			SIR_ROAM_SYNCH_PROPAGATION))) {
		WMA_LOGE("LFR3: PE roam synch cb failed");
		status = -EBUSY;
		goto cleanup_label;
	}

	wma_roam_update_vdev(wma, roam_synch_ind_ptr);
	wma->csr_roam_synch_cb(wma->mac_context, roam_synch_ind_ptr,
			       bss_desc_ptr, SIR_ROAM_SYNCH_PROPAGATION);
	wma_process_roam_synch_complete(wma, synch_event->vdev_id);

	/* update freq and channel width */
	wma->interfaces[synch_event->vdev_id].mhz =
		roam_synch_ind_ptr->chan_freq;
	if (roam_synch_ind_ptr->join_rsp)
		wma->interfaces[synch_event->vdev_id].chan_width =
			roam_synch_ind_ptr->join_rsp->vht_channel_width;
	/*
	 * update phy_mode in wma to avoid mismatch in phymode between host and
	 * firmware. The phymode stored in peer->peer_mlme.phymode is
	 * sent to firmware as part of opmode update during either - vht opmode
	 * action frame received or during opmode change detected while
	 * processing beacon. Any mismatch of this value with firmware phymode
	 * results in firmware assert.
	 */
	wma_update_phymode_on_roam(wma, roam_synch_ind_ptr->bssid.bytes,
				   param_buf->chan,
				   &wma->interfaces[synch_event->vdev_id]);

	wma->csr_roam_synch_cb(wma->mac_context, roam_synch_ind_ptr,
			       bss_desc_ptr, SIR_ROAM_SYNCH_COMPLETE);
	wma->interfaces[synch_event->vdev_id].roam_synch_delay =
		qdf_get_system_timestamp() - roam_synch_received;
	WMA_LOGD("LFR3: roam_synch_delay:%d",
		wma->interfaces[synch_event->vdev_id].roam_synch_delay);
	wma->csr_roam_synch_cb(wma->mac_context, roam_synch_ind_ptr,
			       bss_desc_ptr, SIR_ROAM_SYNCH_NAPI_OFF);

	status = 0;

cleanup_label:
	if (status != 0) {
		if (roam_synch_ind_ptr)
			wma->csr_roam_synch_cb(wma->mac_context,
					       roam_synch_ind_ptr, NULL,
					       SIR_ROAMING_ABORT);
		roam_req = qdf_mem_malloc(sizeof(struct roam_offload_scan_req));
		if (roam_req && synch_event) {
			roam_req->Command = ROAM_SCAN_OFFLOAD_STOP;
			roam_req->reason = REASON_ROAM_SYNCH_FAILED;
			roam_req->sessionId = synch_event->vdev_id;
			wma_process_roaming_config(wma, roam_req);
		}
	}
	if (roam_synch_ind_ptr && roam_synch_ind_ptr->join_rsp)
		qdf_mem_free(roam_synch_ind_ptr->join_rsp);
	if (roam_synch_ind_ptr)
		qdf_mem_free(roam_synch_ind_ptr);
	if (bss_desc_ptr)
		qdf_mem_free(bss_desc_ptr);
	if (wma && synch_event)
		wma->interfaces[synch_event->vdev_id].roam_synch_in_progress =
			false;

	return status;
}

int wma_roam_synch_frame_event_handler(void *handle, uint8_t *event,
					uint32_t len)
{
	WMI_ROAM_SYNCH_FRAME_EVENTID_param_tlvs *param_buf = NULL;
	wmi_roam_synch_frame_event_fixed_param *synch_frame_event = NULL;
	tp_wma_handle wma = (tp_wma_handle) handle;
	A_UINT32 vdev_id;
	struct wma_txrx_node *iface = NULL;
	int status = -EINVAL;

	WMA_LOGD("LFR3:Synch Frame event");
	if (!event) {
		WMA_LOGE("event param null");
		return status;
	}

	param_buf = (WMI_ROAM_SYNCH_FRAME_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("received null buf from target");
		return status;
	}

	synch_frame_event = param_buf->fixed_param;
	if (!synch_frame_event) {
		WMA_LOGE("received null event data from target");
		return status;
	}

	if (synch_frame_event->vdev_id >= wma->max_bssid) {
		WMA_LOGE("received invalid vdev_id %d",
				 synch_frame_event->vdev_id);
		return status;
	}

	if (synch_frame_event->bcn_probe_rsp_len >
	    param_buf->num_bcn_probe_rsp_frame ||
	    synch_frame_event->reassoc_req_len >
	    param_buf->num_reassoc_req_frame ||
	    synch_frame_event->reassoc_rsp_len >
	    param_buf->num_reassoc_rsp_frame) {
		WMA_LOGE("fixed/actual len err: bcn:%d/%d req:%d/%d rsp:%d/%d",
			 synch_frame_event->bcn_probe_rsp_len,
			 param_buf->num_bcn_probe_rsp_frame,
			 synch_frame_event->reassoc_req_len,
			 param_buf->num_reassoc_req_frame,
			 synch_frame_event->reassoc_rsp_len,
			 param_buf->num_reassoc_rsp_frame);
		return status;
	}

	vdev_id = synch_frame_event->vdev_id;
	iface = &wma->interfaces[vdev_id];

	if (wma_is_roam_synch_in_progress(wma, vdev_id)) {
		WMA_LOGE("Ignoring this event as it is unexpected");
		wma_free_roam_synch_frame_ind(iface);
		return status;
	}
	WMA_LOGD("LFR3: Received ROAM_SYNCH_FRAME_EVENT");

	WMA_LOGD("synch frame payload: LEN bcn:%d, req:%d, rsp:%d morefrag: %d",
				synch_frame_event->bcn_probe_rsp_len,
				synch_frame_event->reassoc_req_len,
				synch_frame_event->reassoc_rsp_len,
				synch_frame_event->more_frag);

	if (synch_frame_event->bcn_probe_rsp_len) {
		iface->roam_synch_frame_ind.bcn_probe_rsp_len =
				synch_frame_event->bcn_probe_rsp_len;
		iface->roam_synch_frame_ind.is_beacon =
				synch_frame_event->is_beacon;

		if (iface->roam_synch_frame_ind.bcn_probe_rsp)
			qdf_mem_free(iface->roam_synch_frame_ind.
				     bcn_probe_rsp);
		iface->roam_synch_frame_ind.bcn_probe_rsp =
			qdf_mem_malloc(iface->roam_synch_frame_ind.
				       bcn_probe_rsp_len);
		if (!iface->roam_synch_frame_ind.bcn_probe_rsp) {
			QDF_ASSERT(iface->roam_synch_frame_ind.
				   bcn_probe_rsp);
			status = -ENOMEM;
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}
		qdf_mem_copy(iface->roam_synch_frame_ind.
			bcn_probe_rsp,
			param_buf->bcn_probe_rsp_frame,
			iface->roam_synch_frame_ind.bcn_probe_rsp_len);
	}

	if (synch_frame_event->reassoc_req_len) {
		iface->roam_synch_frame_ind.reassoc_req_len =
				synch_frame_event->reassoc_req_len;

		if (iface->roam_synch_frame_ind.reassoc_req)
			qdf_mem_free(iface->roam_synch_frame_ind.reassoc_req);
		iface->roam_synch_frame_ind.reassoc_req =
			qdf_mem_malloc(iface->roam_synch_frame_ind.
				       reassoc_req_len);
		if (!iface->roam_synch_frame_ind.reassoc_req) {
			QDF_ASSERT(iface->roam_synch_frame_ind.
				   reassoc_req);
			status = -ENOMEM;
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}
		qdf_mem_copy(iface->roam_synch_frame_ind.reassoc_req,
			     param_buf->reassoc_req_frame,
			     iface->roam_synch_frame_ind.reassoc_req_len);
	}

	if (synch_frame_event->reassoc_rsp_len) {
		iface->roam_synch_frame_ind.reassoc_rsp_len =
				synch_frame_event->reassoc_rsp_len;

		if (iface->roam_synch_frame_ind.reassoc_rsp)
			qdf_mem_free(iface->roam_synch_frame_ind.reassoc_rsp);

		iface->roam_synch_frame_ind.reassoc_rsp =
			qdf_mem_malloc(iface->roam_synch_frame_ind.
				       reassoc_rsp_len);
		if (!iface->roam_synch_frame_ind.reassoc_rsp) {
			QDF_ASSERT(iface->roam_synch_frame_ind.
				   reassoc_rsp);
			status = -ENOMEM;
			wma_free_roam_synch_frame_ind(iface);
			return status;
		}
		qdf_mem_copy(iface->roam_synch_frame_ind.reassoc_rsp,
			     param_buf->reassoc_rsp_frame,
			     iface->roam_synch_frame_ind.reassoc_rsp_len);
	}
	return 0;
}

/**
 * __wma_roam_synch_event_handler() - roam synch event handler
 * @handle: wma handle
 * @event: event data
 * @len: length of data
 *
 * This function is roam synch event handler.It sends roam
 * indication for upper layer.
 *
 * Return: Success or Failure status
 */
int wma_roam_synch_event_handler(void *handle, uint8_t *event,
				 uint32_t len)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	int status = -EINVAL;
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct wma_txrx_node *iface = NULL;
	wmi_roam_synch_event_fixed_param *synch_event = NULL;
	WMI_ROAM_SYNCH_EVENTID_param_tlvs *param_buf = NULL;

	if (!event) {
		wma_err_rl("event param null");
		return status;
	}

	param_buf = (WMI_ROAM_SYNCH_EVENTID_param_tlvs *)event;
	if (!param_buf) {
		wma_err_rl("received null buf from target");
		return status;
	}
	synch_event = param_buf->fixed_param;
	if (!synch_event) {
		wma_err_rl("received null event data from target");
		return status;
	}

	if (synch_event->vdev_id >= wma->max_bssid) {
		wma_err_rl("received invalid vdev_id %d",
			   synch_event->vdev_id);
		return status;
	}

	iface = &wma->interfaces[synch_event->vdev_id];
	qdf_status = wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
						   WLAN_VDEV_SM_EV_ROAM,
						   len,
						   event);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		wma_err("Failed to send the EV_ROAM");
		return status;
	}
	wma_debug("Posted EV_ROAM to VDEV SM");
	return 0;
}

int wma_roam_auth_offload_event_handler(WMA_HANDLE handle, uint8_t *event,
					uint32_t len)
{
	QDF_STATUS status;
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct mac_context *mac_ctx;
	wmi_roam_preauth_start_event_fixed_param *rso_auth_start_ev;
	WMI_ROAM_PREAUTH_START_EVENTID_param_tlvs *param_buf;
	struct qdf_mac_addr ap_bssid;
	uint8_t vdev_id;

	if (!event) {
		wma_err_rl("received null event from target");
		return -EINVAL;
	}

	param_buf = (WMI_ROAM_PREAUTH_START_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		wma_err_rl("received null buf from target");
		return -EINVAL;
	}

	rso_auth_start_ev = param_buf->fixed_param;
	if (!rso_auth_start_ev) {
		wma_err_rl("received null event data from target");
		return -EINVAL;
	}

	if (rso_auth_start_ev->vdev_id > wma->max_bssid) {
		wma_err_rl("received invalid vdev_id %d",
			   rso_auth_start_ev->vdev_id);
		return -EINVAL;
	}

	mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac_ctx) {
		wma_err("NULL mac ptr");
		QDF_ASSERT(0);
		return -EINVAL;
	}

	cds_host_diag_log_work(&wma->roam_preauth_wl,
			       WMA_ROAM_PREAUTH_WAKE_LOCK_DURATION,
			       WIFI_POWER_EVENT_WAKELOCK_WOW);
	qdf_wake_lock_timeout_acquire(&wma->roam_ho_wl,
				      WMA_ROAM_HO_WAKE_LOCK_DURATION);

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&rso_auth_start_ev->candidate_ap_bssid,
				   ap_bssid.bytes);
	if (qdf_is_macaddr_zero(&ap_bssid) ||
	    qdf_is_macaddr_broadcast(&ap_bssid) ||
	    qdf_is_macaddr_group(&ap_bssid)) {
		wma_err_rl("Invalid bssid");
		return -EINVAL;
	}

	vdev_id = rso_auth_start_ev->vdev_id;
	wma_debug("Received Roam auth offload event for bss:%pM vdev_id:%d",
		  ap_bssid.bytes, vdev_id);

	status = wma->csr_roam_auth_event_handle_cb(mac_ctx, vdev_id, ap_bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err_rl("Trigger pre-auth failed");
		return -EINVAL;
	}

	return 0;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void
wma_get_trigger_detail_str(struct wmi_roam_trigger_info roam_info, char *buf)
{
	uint16_t buf_cons, buf_left = MAX_ROAM_DEBUG_BUF_SIZE;
	char *temp = buf;

	buf_cons = qdf_snprint(temp, buf_left, "Reason: \"%s\" ",
			mlme_get_roam_trigger_str(roam_info.trigger_reason));
	temp += buf_cons;
	buf_left -= buf_cons;

	if (roam_info.trigger_sub_reason) {
		buf_cons = qdf_snprint(
			    temp, buf_left, "Sub-Reason: %s",
			    mlme_get_sub_reason_str(roam_info.trigger_sub_reason));
		temp += buf_cons;
		buf_left -= buf_cons;
	}

	switch (roam_info.trigger_reason) {
	case WMI_ROAM_TRIGGER_REASON_PER:
	case WMI_ROAM_TRIGGER_REASON_BMISS:
	case WMI_ROAM_TRIGGER_REASON_HIGH_RSSI:
	case WMI_ROAM_TRIGGER_REASON_PERIODIC:
	case WMI_ROAM_TRIGGER_REASON_MAWC:
	case WMI_ROAM_TRIGGER_REASON_DENSE:
	case WMI_ROAM_TRIGGER_REASON_BACKGROUND:
	case WMI_ROAM_TRIGGER_REASON_IDLE:
	case WMI_ROAM_TRIGGER_REASON_FORCED:
	case WMI_ROAM_TRIGGER_REASON_UNIT_TEST:
		return;
	case WMI_ROAM_TRIGGER_REASON_BTM:
		buf_cons = qdf_snprint(temp, buf_left,
				       "Req_mode: %d Disassoc_timer: %d",
				       roam_info.btm_trig_data.btm_request_mode,
				       roam_info.btm_trig_data.disassoc_timer);
		temp += buf_cons;
		buf_left -= buf_cons;

		buf_cons = qdf_snprint(temp, buf_left,
			    "validity_interval: %d candidate_list_cnt: %d resp_status: %d",
			    roam_info.btm_trig_data.validity_interval,
			    roam_info.btm_trig_data.candidate_list_count,
			    roam_info.btm_trig_data.btm_resp_status);
		buf_left -= buf_cons;
		temp += buf_cons;
		return;
	case WMI_ROAM_TRIGGER_REASON_BSS_LOAD:
		buf_cons = qdf_snprint(temp, buf_left, "CU: %d %% ",
				       roam_info.cu_trig_data.cu_load);
		temp += buf_cons;
		buf_left -= buf_cons;
		return;
	case WMI_ROAM_TRIGGER_REASON_DEAUTH:
		buf_cons = qdf_snprint(temp, buf_left, "Type: %d Reason: %d ",
				       roam_info.deauth_trig_data.type,
				       roam_info.deauth_trig_data.reason);
		temp += buf_cons;
		buf_left -= buf_cons;
		return;
	case WMI_ROAM_TRIGGER_REASON_LOW_RSSI:
		buf_cons = qdf_snprint(temp, buf_left, "Low_rssi_threshold: %d",
				       roam_info.rssi_trig_data.threshold);
		temp += buf_cons;
		buf_left -= buf_cons;
		return;
	default:
		return;
	}
}

static void
wma_rso_print_trigger_info(struct wmi_roam_trigger_info data, uint8_t vdev_id)
{
	char *buf;
	char time[TIME_STRING_LEN];

	buf = qdf_mem_malloc(MAX_ROAM_DEBUG_BUF_SIZE);
	if (!buf)
		return;

	wma_get_trigger_detail_str(data, buf);
	mlme_get_converted_timestamp(data.timestamp, time);
	WMA_LOGI("[ROAM_DEBUG] %s [ROAM_TRIGGER]: VDEV[%d] Current_rssi: %d dBm %s",
		 time, vdev_id, data.current_rssi, buf);

	qdf_mem_free(buf);
}

static void
wma_log_roam_scan_candidates(struct wmi_roam_candidate_info *ap,
			     uint8_t num_entries)
{
	uint16_t i;
	char time[TIME_STRING_LEN];

	WMA_LOGI("%40s%40s%40s", LINE_STR, LINE_STR, LINE_STR);
	WMA_LOGI("%13s %20s %11s %6s %12s %8s %12s %8s %10s %13s",
		 "AP BSSID", "TIMESTAMP", "CHANNEL", "TYPE", "ETP", "RSSI",
		 "RSSI_SCORE", "CU_LOAD", "CU_SCORE", "TOTAL_SCORE");
	WMA_LOGI("%40s%40s%40s", LINE_STR, LINE_STR, LINE_STR);

	if (num_entries > MAX_ROAM_CANDIDATE_AP)
		num_entries = MAX_ROAM_CANDIDATE_AP;

	for (i = 0; i < num_entries; i++) {
		mlme_get_converted_timestamp(ap->timestamp, time);
		WMA_LOGI(QDF_MAC_ADDR_STR " %20s %5d  %-12s %4d(Mbps) %3d(dBm) %6d %7d %% %10d %12d",
			 QDF_MAC_ADDR_ARRAY(ap->bssid.bytes), time, ap->freq,
			 ((ap->type == 0) ? "CANDIDATE_AP" :
			  ((ap->type == 2) ? "ROAMED_AP" : "PREVIOUS_AP")),
			 ap->etp, ap->rssi, ap->rssi_score, ap->cu_load,
			 ap->cu_score, ap->total_score);
		ap++;
	}
}

static void
wma_rso_print_scan_info(struct wmi_roam_scan_data scan, uint8_t vdev_id,
			uint32_t trigger, uint32_t timestamp)
{
	uint16_t num_ch = scan.num_chan;
	uint16_t buf_cons = 0, buf_left = ROAM_CHANNEL_BUF_SIZE;
	uint8_t i;
	char *buf, *buf1, *tmp;
	char time[TIME_STRING_LEN];

	buf = qdf_mem_malloc(ROAM_CHANNEL_BUF_SIZE);
	if (!buf)
		return;

	tmp = buf;
	/* For partial scans, print the channel info */
	if (!scan.type) {
		buf_cons = qdf_snprint(tmp, buf_left, "{");
		buf_left -= buf_cons;
		tmp += buf_cons;

		for (i = 0; i < num_ch; i++) {
			buf_cons = qdf_snprint(tmp, buf_left, "%d ",
					       scan.chan_freq[i]);
			buf_left -= buf_cons;
			tmp += buf_cons;
		}
		buf_cons = qdf_snprint(tmp, buf_left, "}");
		buf_left -= buf_cons;
		tmp += buf_cons;
	}

	buf1 = qdf_mem_malloc(ROAM_FAILURE_BUF_SIZE);
	if (!buf1) {
		qdf_mem_free(buf);
		return;
	}

	if (WMI_ROAM_TRIGGER_REASON_LOW_RSSI == trigger) {
		qdf_snprint(buf1, ROAM_FAILURE_BUF_SIZE,
			    "next_rssi_threshold: %d dBm",
			    scan.next_rssi_threshold);
	}

	mlme_get_converted_timestamp(timestamp, time);
	WMA_LOGI("[ROAM_DEBUG] %s [ROAM_SCAN]: VDEV[%d] Scan_type: %s %s %s",
		 time, vdev_id, (scan.type ? "FULL" : "PARTIAL"),
		 buf1, buf);
	wma_log_roam_scan_candidates(scan.ap, scan.num_ap);

	qdf_mem_free(buf);
	qdf_mem_free(buf1);
}

static void
wma_rso_print_roam_result(struct wmi_roam_result res,
			  uint8_t vdev_id)
{
	char *buf;
	char time[TIME_STRING_LEN];

	buf = qdf_mem_malloc(ROAM_FAILURE_BUF_SIZE);
	if (!buf)
		return;

	if (!res.status)
		qdf_snprint(buf, ROAM_FAILURE_BUF_SIZE, "Fail-Reason: %s",
			    mlme_get_roam_fail_reason_str(res.fail_reason));

	mlme_get_converted_timestamp(res.timestamp, time);
	WMA_LOGI("[ROAM_DEBUG] %s [ROAM_RESULT]: VDEV[%d] status:%s %s",
		 time, vdev_id, (res.status) ? "SUCCESS" : "FAILED", buf);

	qdf_mem_free(buf);
}

static void
wma_rso_print_11kv_info(struct wmi_neighbor_report_data neigh_rpt,
			uint8_t vdev_id, bool is_roam_success)
{
	char time[TIME_STRING_LEN], time1[TIME_STRING_LEN];
	char *buf, *tmp;
	uint8_t type = neigh_rpt.req_type, i;
	uint16_t buf_left = ROAM_CHANNEL_BUF_SIZE, buf_cons;
	uint8_t num_ch = neigh_rpt.num_freq;

	if (!is_roam_success)
		return;

	if (!type) {
		WMA_LOGI("[ROAM_DEBUG] AP doesn't support neighbor rpt/BTM");
		return;
	}

	buf = qdf_mem_malloc(ROAM_CHANNEL_BUF_SIZE);
	if (!buf)
		return;

	tmp = buf;
	for (i = 0; i < num_ch; i++) {
		buf_cons = qdf_snprint(tmp, buf_left, "{ ");
		buf_left -= buf_cons;
		tmp += buf_cons;

		buf_cons = qdf_snprint(tmp, buf_left, "%d ",
				       neigh_rpt.freq[i]);
		buf_left -= buf_cons;
		tmp += buf_cons;

		buf_cons = qdf_snprint(tmp, buf_left, "}");
		buf_left -= buf_cons;
		tmp += buf_cons;
	}
	mlme_get_converted_timestamp(neigh_rpt.req_time, time);
	WMA_LOGI("[ROAM_DEBUG] %s [%s] RX: VDEV[%d] %s", time,
		 (type == 1) ? "BTM" : "NEIGH_RPT", vdev_id, buf);

	mlme_get_converted_timestamp(neigh_rpt.resp_time, time1);
	WMA_LOGI("[ROAM_DEBUG] %s [%s] TX: VDEV[%d]", time1,
		 (type == 1) ? "BTM" : "NEIGH_RPT", vdev_id);
	qdf_mem_free(buf);
}

int wma_roam_stats_event_handler(WMA_HANDLE handle, uint8_t *event,
				 uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_ROAM_STATS_EVENTID_param_tlvs *param_buf;
	wmi_roam_stats_event_fixed_param *fixed_param;
	struct mlme_roam_debug_info *roam_info = NULL;
	uint8_t vdev_id, i;
	uint8_t num_tlv = 0, num_chan = 0, num_ap = 0, num_rpt = 0;
	uint32_t rem_len;
	QDF_STATUS status;

	param_buf = (WMI_ROAM_STATS_EVENTID_param_tlvs *)event;
	if (!param_buf) {
		wma_err_rl("NULL event received from target");
		goto err;
	}

	fixed_param = param_buf->fixed_param;
	if (!fixed_param) {
		wma_err_rl(" NULL fixed param");
		goto err;
	}

	vdev_id = fixed_param->vdev_id;
	if (vdev_id >= wma->max_bssid) {
		wma_err_rl("Invalid vdev_id %d", vdev_id);
		goto err;
	}

	num_tlv = fixed_param->roam_scan_trigger_count;
	if (num_tlv != param_buf->num_roam_scan_info ||
	    num_tlv != param_buf->num_roam_trigger_reason ||
	    num_tlv != param_buf->num_roam_result ||
	    num_tlv != param_buf->num_roam_neighbor_report_info) {
		wma_err_rl("Invalid roam stats num_tlv:%d num_scan:%d num_res:%d num_rept:%d",
			   num_tlv, param_buf->num_roam_scan_info,
			   param_buf->num_roam_result,
			   param_buf->num_roam_neighbor_report_info);
		goto err;
	}

	rem_len = WMI_SVC_MSG_MAX_SIZE - sizeof(*fixed_param);
	if (rem_len < num_tlv * sizeof(wmi_roam_trigger_reason)) {
		wma_err_rl("Invalid roam trigger data");
		goto err;
	}

	rem_len -= num_tlv * sizeof(wmi_roam_trigger_reason);
	if (rem_len < num_tlv * sizeof(wmi_roam_scan_info)) {
		wma_err_rl("Invalid roam scan data");
		goto err;
	}

	rem_len -= num_tlv * sizeof(wmi_roam_scan_info);
	if (rem_len < num_tlv * sizeof(wmi_roam_result)) {
		wma_err_rl("Invalid roam result data");
		goto err;
	}

	rem_len -= num_tlv * sizeof(wmi_roam_result);
	if (rem_len < (num_tlv * sizeof(wmi_roam_neighbor_report_info))) {
		wma_err_rl("Invalid roam neighbor report data");
		goto err;
	}

	rem_len -= num_tlv * sizeof(wmi_roam_neighbor_report_info);
	if (rem_len < (param_buf->num_roam_scan_chan_info *
		       sizeof(wmi_roam_scan_channel_info))) {
		wma_err_rl("Invalid roam chan data num_tlv:%d",
			   param_buf->num_roam_scan_chan_info);
		goto err;
	}

	rem_len -= param_buf->num_roam_scan_chan_info *
		   sizeof(wmi_roam_scan_channel_info);

	if (rem_len < (param_buf->num_roam_ap_info *
		       sizeof(wmi_roam_ap_info))) {
		wma_err_rl("Invalid roam ap data num_tlv:%d",
			   param_buf->num_roam_ap_info);
		goto err;
	}

	rem_len -= param_buf->num_roam_ap_info * sizeof(wmi_roam_ap_info);
	if (rem_len < (param_buf->num_roam_neighbor_report_chan_info *
		       sizeof(wmi_roam_neighbor_report_channel_info))) {
		wma_err_rl("Invalid roam neigb rpt chan data num_tlv:%d",
			   param_buf->num_roam_neighbor_report_chan_info);
		goto err;
	}

	for (i = 0; i < num_tlv; i++) {
		roam_info = qdf_mem_malloc(sizeof(*roam_info));
		if (!roam_info)
			return -ENOMEM;

		/*
		 * Roam Trigger id and that specific roam trigger related
		 * details.
		 */
		status = wmi_unified_extract_roam_trigger_stats(
				wma->wmi_handle, event,
				&roam_info->trigger, i);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("%s: Extract roam trigger stats failed vdev%d",
				 __func__, vdev_id);
			qdf_mem_free(roam_info);
			return -EINVAL;
		}

		/* Roam scan related details - Scan channel, scan type .. */
		status = wmi_unified_extract_roam_scan_stats(
				wma->wmi_handle, event, &roam_info->scan, i,
				num_chan, num_ap);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("%s: Roam scan stats extract failed vdev %d",
				 __func__, vdev_id);
			qdf_mem_free(roam_info);
			return -EINVAL;
		}
		num_chan += roam_info->scan.num_chan;
		num_ap += roam_info->scan.num_ap;

		/* Roam result - Success/Failure status, failure reason */
		status = wmi_unified_extract_roam_result_stats(
						wma->wmi_handle, event,
						&roam_info->result, i);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("%s: Roam result stats extract failed vdev %d",
				 __func__, vdev_id);
			qdf_mem_free(roam_info);
			return -EINVAL;
		}

		/* BTM req/resp or Neighbor report/response info */
		status = wmi_unified_extract_roam_11kv_stats(
				wma->wmi_handle, event,
				&roam_info->data_11kv, i, num_rpt);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("%s: Roam 11kv stats extract failed vdev %d",
				 __func__, vdev_id);
			qdf_mem_free(roam_info);
			return -EINVAL;
		}
		num_rpt += roam_info->data_11kv.num_freq;

		wma_rso_print_trigger_info(roam_info->trigger, vdev_id);
		wma_rso_print_scan_info(roam_info->scan, vdev_id,
					roam_info->trigger.trigger_reason,
					roam_info->trigger.timestamp);
		wma_rso_print_roam_result(roam_info->result, vdev_id);
		wma_rso_print_11kv_info(roam_info->data_11kv, vdev_id,
					roam_info->result.status);

		qdf_mem_free(roam_info);
	}

	return 0;

err:
	return -EINVAL;
}
#endif

#define RSN_CAPS_SHIFT               16
/**
 * wma_roam_scan_fill_self_caps() - fill capabilities
 * @wma_handle: wma handle
 * @roam_offload_params: offload parameters
 * @roam_req: roam request
 *
 * This function fills roam self capablities.
 *
 * Return: QDF status
 */
QDF_STATUS wma_roam_scan_fill_self_caps(tp_wma_handle wma_handle,
					roam_offload_param *
					roam_offload_params,
					struct roam_offload_scan_req *roam_req)
{
	qdf_size_t val_len;
	struct mac_context *mac = NULL;
	tSirMacCapabilityInfo selfCaps;
	uint32_t val = 0;
	uint16_t *pCfgValue16;
	uint8_t nCfgValue8, *pCfgValue8;
	tSirMacQosInfoStation macQosInfoSta;

	qdf_mem_zero(&macQosInfoSta, sizeof(tSirMacQosInfoStation));
	/* Roaming is done only for INFRA STA type.
	 * So, ess will be one and ibss will be Zero
	 */
	mac = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac) {
		WMA_LOGE("%s:NULL mac ptr. Exiting", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	selfCaps.ess = 1;
	selfCaps.ibss = 0;

	val = mac->mlme_cfg->wep_params.is_privacy_enabled;
	if (val)
		selfCaps.privacy = 1;

	if (mac->mlme_cfg->ht_caps.short_preamble)
		selfCaps.shortPreamble = 1;

	selfCaps.pbcc = 0;
	selfCaps.channelAgility = 0;

	if (mac->mlme_cfg->feature_flags.enable_short_slot_time_11g)
		selfCaps.shortSlotTime = 1;

	if (mac->mlme_cfg->gen.enabled_11h)
		selfCaps.spectrumMgt = 1;

	if (mac->mlme_cfg->wmm_params.qos_enabled)
		selfCaps.qos = 1;

	if (mac->mlme_cfg->scoring.apsd_enabled)
		selfCaps.apsd = 1;

	selfCaps.rrm = mac->rrm.rrmSmeContext.rrmConfig.rrm_enabled;

	val = mac->mlme_cfg->feature_flags.enable_block_ack;
	selfCaps.delayedBA =
		(uint16_t) ((val >> WNI_CFG_BLOCK_ACK_ENABLED_DELAYED) & 1);
	selfCaps.immediateBA =
		(uint16_t) ((val >> WNI_CFG_BLOCK_ACK_ENABLED_IMMEDIATE) & 1);
	pCfgValue16 = (uint16_t *) &selfCaps;

	/*
	 * RSN caps arent been sent to firmware, so in case of PMF required,
	 * the firmware connects to a non PMF AP advertising PMF not required
	 * in the re-assoc request which violates protocol.
	 * So send this to firmware in the roam SCAN offload command to
	 * let it configure the params in the re-assoc request too.
	 * Instead of making another infra, send the RSN-CAPS in MSB of
	 * beacon Caps.
	 */
	roam_offload_params->capability = *((uint32_t *)(&roam_req->rsn_caps));
	roam_offload_params->capability <<= RSN_CAPS_SHIFT;
	roam_offload_params->capability |= ((*pCfgValue16) & 0xFFFF);

	roam_offload_params->ht_caps_info =
		*(uint32_t *)&mac->mlme_cfg->ht_caps.ht_cap_info;

	roam_offload_params->ampdu_param =
		*(uint32_t *)&mac->mlme_cfg->ht_caps.ampdu_params;

	roam_offload_params->ht_ext_cap =
		*(uint32_t *)&mac->mlme_cfg->ht_caps.ext_cap_info;

	val_len = ROAM_OFFLOAD_NUM_MCS_SET;
	if (wlan_mlme_get_cfg_str((uint8_t *)roam_offload_params->mcsset,
				  &mac->mlme_cfg->rates.supported_mcs_set,
				  &val_len) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			  "Failed to get CFG_SUPPORTED_MCS_SET");
		return QDF_STATUS_E_FAILURE;
	}

	/* tSirMacTxBFCapabilityInfo */
	nCfgValue8 = (uint8_t)mac->mlme_cfg->vht_caps.vht_cap_info.tx_bf_cap;
	roam_offload_params->ht_txbf = nCfgValue8 & 0xFF;
	/* tSirMacASCapabilityInfo */
	nCfgValue8 = (uint8_t)mac->mlme_cfg->vht_caps.vht_cap_info.as_cap;
	roam_offload_params->asel_cap = nCfgValue8 & 0xFF;

	/* QOS Info */
	nCfgValue8 = mac->mlme_cfg->wmm_params.max_sp_length;
	macQosInfoSta.maxSpLen = nCfgValue8;
	macQosInfoSta.moreDataAck = 0;
	macQosInfoSta.qack = 0;
	macQosInfoSta.acbe_uapsd = roam_req->AcUapsd.acbe_uapsd;
	macQosInfoSta.acbk_uapsd = roam_req->AcUapsd.acbk_uapsd;
	macQosInfoSta.acvi_uapsd = roam_req->AcUapsd.acvi_uapsd;
	macQosInfoSta.acvo_uapsd = roam_req->AcUapsd.acvo_uapsd;
	pCfgValue8 = (uint8_t *) &macQosInfoSta;
	/* macQosInfoSta Only queue_request is set.Refer to
	 * populate_dot11f_wmm_caps for more details
	 */
	roam_offload_params->qos_caps = (*pCfgValue8) & 0xFF;
	if (roam_offload_params->qos_caps)
		roam_offload_params->qos_enabled = true;
	roam_offload_params->wmm_caps = 0x4 & 0xFF;
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_set_ric_req() - set ric request element
 * @wma: wma handle
 * @msg: message
 * @is_add_ts: is addts required
 *
 * This function sets ric request element for 11r roaming.
 *
 * Return: none
 */
void wma_set_ric_req(tp_wma_handle wma, void *msg, uint8_t is_add_ts)
{
	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return;
	}

	wmi_unified_set_ric_req_cmd(wma->wmi_handle, msg, is_add_ts);
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#ifdef FEATURE_RSSI_MONITOR
QDF_STATUS wma_set_rssi_monitoring(tp_wma_handle wma,
				   struct rssi_monitor_param *req)
{
	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_set_rssi_monitoring_cmd(wma->wmi_handle, req);
}

/**
 * wma_rssi_breached_event_handler() - rssi breached event handler
 * @handle: wma handle
 * @cmd_param_info: event handler data
 * @len: length of @cmd_param_info
 *
 * Return: 0 on success; error number otherwise
 */
int wma_rssi_breached_event_handler(void *handle,
				u_int8_t  *cmd_param_info, u_int32_t len)
{
	WMI_RSSI_BREACH_EVENTID_param_tlvs *param_buf;
	wmi_rssi_breach_event_fixed_param  *event;
	struct rssi_breach_event  rssi;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!mac || !wma) {
		WMA_LOGE("%s: Invalid mac/wma context", __func__);
		return -EINVAL;
	}
	if (!mac->sme.rssi_threshold_breached_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_RSSI_BREACH_EVENTID_param_tlvs *)cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid rssi breached event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;

	rssi.request_id = event->request_id;
	rssi.session_id = event->vdev_id;
	if (wmi_service_enabled(wma->wmi_handle,
				wmi_service_hw_db2dbm_support))
		rssi.curr_rssi = event->rssi;
	else
		rssi.curr_rssi = event->rssi + WMA_TGT_NOISE_FLOOR_DBM;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->bssid, rssi.curr_bssid.bytes);

	WMA_LOGD("%s: req_id: %u vdev_id: %d curr_rssi: %d", __func__,
		 rssi.request_id, rssi.session_id, rssi.curr_rssi);
	WMA_LOGI("%s: curr_bssid: %pM", __func__, rssi.curr_bssid.bytes);

	mac->sme.rssi_threshold_breached_cb(mac->hdd_handle, &rssi);
	WMA_LOGD("%s: Invoke HDD rssi breached callback", __func__);
	return 0;
}
#endif /* FEATURE_RSSI_MONITOR */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wma_roam_ho_fail_handler() - LFR3.0 roam hand off failed handler
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: none
 */
static void wma_roam_ho_fail_handler(tp_wma_handle wma, uint32_t vdev_id)
{
	struct handoff_failure_ind *ho_failure_ind;
	struct scheduler_msg sme_msg = { 0 };
	QDF_STATUS qdf_status;

	ho_failure_ind = qdf_mem_malloc(sizeof(*ho_failure_ind));
	if (!ho_failure_ind)
		return;

	ho_failure_ind->vdev_id = vdev_id;
	sme_msg.type = eWNI_SME_HO_FAIL_IND;
	sme_msg.bodyptr = ho_failure_ind;
	sme_msg.bodyval = 0;

	qdf_status = scheduler_post_message(QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("Fail to post eWNI_SME_HO_FAIL_IND msg to SME");
		qdf_mem_free(ho_failure_ind);
		return;
	}
}

/**
 * wma_process_roam_synch_complete() - roam synch complete command to fw.
 * @handle: wma handle
 * @synchcnf: offload synch confirmation params
 *
 * This function sends roam synch complete event to fw.
 *
 * Return: none
 */
void wma_process_roam_synch_complete(WMA_HANDLE handle, uint8_t vdev_id)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	if (!wma_handle || !wma_handle->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue roam synch cnf",
			 __func__);
		return;
	}

	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: Invalid vdev id:%d", __func__, vdev_id);
		return;
	}

	if (wmi_unified_roam_synch_complete_cmd(wma_handle->wmi_handle,
				 vdev_id)) {
		return;
	}

	DPTRACE(qdf_dp_trace_record_event(QDF_DP_TRACE_EVENT_RECORD,
		vdev_id, QDF_TRACE_DEFAULT_PDEV_ID,
		QDF_PROTO_TYPE_EVENT, QDF_ROAM_COMPLETE));

	WMA_LOGI("LFR3: Posting WMA_ROAM_OFFLOAD_SYNCH_CNF");
	wlan_roam_debug_log(vdev_id, DEBUG_ROAM_SYNCH_CNF,
			    DEBUG_INVALID_PEER_ID, NULL, NULL, 0, 0);

}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

QDF_STATUS wma_pre_chan_switch_setup(uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wma_txrx_node *intr = &wma->interfaces[vdev_id];
	uint16_t beacon_interval_ori;
	bool restart;
	uint16_t reduced_beacon_interval;
	struct vdev_mlme_obj *mlme_obj;
	struct wlan_objmgr_vdev *vdev = intr->vdev;

	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!mlme_obj) {
		pe_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	restart =
		wma_get_channel_switch_in_progress(intr);
	if (restart && intr->beacon_filter_enabled)
		wma_remove_beacon_filter(wma, &intr->beacon_filter);

	reduced_beacon_interval =
		wma->mac_context->sap.SapDfsInfo.reduced_beacon_interval;
	if (wma_is_vdev_in_ap_mode(wma, vdev_id) && reduced_beacon_interval) {


		/* Reduce the beacon interval just before the channel switch.
		 * This would help in reducing the downtime on the STA side
		 * (which is waiting for beacons from the AP to resume back
		 * transmission). Switch back the beacon_interval to its
		 * original value after the channel switch based on the
		 * timeout. This would ensure there are atleast some beacons
		 * sent with increased frequency.
		 */

		WMA_LOGD("%s: Changing beacon interval to %d",
			 __func__, reduced_beacon_interval);

		/* Add a timer to reset the beacon interval back*/
		beacon_interval_ori = mlme_obj->proto.generic.beacon_interval;
		mlme_obj->proto.generic.beacon_interval =
			reduced_beacon_interval;
		if (wma_fill_beacon_interval_reset_req(wma,
			vdev_id,
			beacon_interval_ori,
			RESET_BEACON_INTERVAL_TIMEOUT)) {

			WMA_LOGD("%s: Failed to fill beacon interval reset req",
				 __func__);
		}
	}

	status = wma_vdev_pre_start(vdev_id, restart);

	return status;
}

QDF_STATUS wma_post_chan_switch_setup(uint8_t vdev_id)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct cdp_pdev *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct wma_txrx_node *intr = &wma->interfaces[vdev_id];
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_channel *des_chan;

	/*
	 * Record monitor mode channel here in case HW
	 * indicate RX PPDU TLV with invalid channel number.
	 */
	if (intr->type == WMI_VDEV_TYPE_MONITOR) {
		des_chan = intr->vdev->vdev_mlme.des_chan;
		cdp_record_monitor_chan_num(soc, pdev, des_chan->ch_ieee);
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_ESE
/**
 * wma_plm_start() - plm start request
 * @wma: wma handle
 * @params: plm request parameters
 *
 * This function request FW to start PLM.
 *
 * Return: QDF status
 */
static QDF_STATUS wma_plm_start(tp_wma_handle wma,
				struct plm_req_params *params)
{
	QDF_STATUS status;

	WMA_LOGD("PLM Start");

	status = wmi_unified_plm_start_cmd(wma->wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma->interfaces[params->vdev_id].plm_in_progress = true;

	WMA_LOGD("Plm start request sent successfully for vdev %d",
		 params->vdev_id);

	return status;
}

/**
 * wma_plm_stop() - plm stop request
 * @wma: wma handle
 * @params: plm request parameters
 *
 * This function request FW to stop PLM.
 *
 * Return: QDF status
 */
static QDF_STATUS wma_plm_stop(tp_wma_handle wma,
			       struct plm_req_params *params)
{
	QDF_STATUS status;

	if (!wma->interfaces[params->vdev_id].plm_in_progress) {
		WMA_LOGE("No active plm req found, skip plm stop req");
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("PLM Stop");

	status = wmi_unified_plm_stop_cmd(wma->wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma->interfaces[params->vdev_id].plm_in_progress = false;

	WMA_LOGD("Plm stop request sent successfully for vdev %d",
		 params->vdev_id);

	return status;
}

/**
 * wma_config_plm() - config PLM
 * @wma: wma handle
 * @params: plm request parameters
 *
 * Return: none
 */
void wma_config_plm(tp_wma_handle wma, struct plm_req_params *params)
{
	QDF_STATUS ret;

	if (!params || !wma)
		return;

	if (params->enable)
		ret = wma_plm_start(wma, params);
	else
		ret = wma_plm_stop(wma, params);

	if (ret)
		WMA_LOGE("%s: PLM %s failed %d", __func__,
			 params->enable ? "start" : "stop", ret);
}
#endif

#ifdef FEATURE_WLAN_EXTSCAN
/**
 * wma_extscan_wow_event_callback() - extscan wow event callback
 * @handle: WMA handle
 * @event: event buffer
 * @len: length of @event buffer
 *
 * In wow case, the wow event is followed by the payload of the event
 * which generated the wow event.
 * payload is 4 bytes of length followed by event buffer. the first 4 bytes
 * of event buffer is common tlv header, which is a combination
 * of tag (higher 2 bytes) and length (lower 2 bytes). The tag is used to
 * identify the event which triggered wow event.
 * Payload is extracted and converted into generic tlv structure before
 * being passed to this function.
 *
 * @Return: Errno
 */
int wma_extscan_wow_event_callback(void *handle, void *event, uint32_t len)
{
	uint32_t tag = WMITLV_GET_TLVTAG(WMITLV_GET_HDR(event));

	switch (tag) {
	case WMITLV_TAG_STRUC_wmi_extscan_start_stop_event_fixed_param:
		return wma_extscan_start_stop_event_handler(handle, event, len);

	case WMITLV_TAG_STRUC_wmi_extscan_operation_event_fixed_param:
		return wma_extscan_operations_event_handler(handle, event, len);

	case WMITLV_TAG_STRUC_wmi_extscan_table_usage_event_fixed_param:
		return wma_extscan_table_usage_event_handler(handle, event,
							     len);

	case WMITLV_TAG_STRUC_wmi_extscan_cached_results_event_fixed_param:
		return wma_extscan_cached_results_event_handler(handle, event,
								len);

	case WMITLV_TAG_STRUC_wmi_extscan_wlan_change_results_event_fixed_param:
		return wma_extscan_change_results_event_handler(handle, event,
								len);

	case WMITLV_TAG_STRUC_wmi_extscan_hotlist_match_event_fixed_param:
		return wma_extscan_hotlist_match_event_handler(handle,	event,
							       len);

	case WMITLV_TAG_STRUC_wmi_extscan_capabilities_event_fixed_param:
		return wma_extscan_capabilities_event_handler(handle, event,
							      len);

	default:
		WMA_LOGE(FL("Unknown tag: %d"), tag);
		return 0;
	}
}

/**
 * wma_register_extscan_event_handler() - register extscan event handler
 * @wma_handle: wma handle
 *
 * This function register extscan related event handlers.
 *
 * Return: none
 */
void wma_register_extscan_event_handler(tp_wma_handle wma_handle)
{
	if (!wma_handle) {
		WMA_LOGE("%s: extscan wma_handle is NULL", __func__);
		return;
	}
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   wmi_extscan_start_stop_event_id,
					   wma_extscan_start_stop_event_handler,
					   WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					wmi_extscan_capabilities_event_id,
					wma_extscan_capabilities_event_handler,
					WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				wmi_extscan_hotlist_match_event_id,
				wma_extscan_hotlist_match_event_handler,
				WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				wmi_extscan_wlan_change_results_event_id,
				wma_extscan_change_results_event_handler,
				WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				wmi_extscan_operation_event_id,
				wma_extscan_operations_event_handler,
				WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				wmi_extscan_table_usage_event_id,
				wma_extscan_table_usage_event_handler,
				WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				wmi_extscan_cached_results_event_id,
				wma_extscan_cached_results_event_handler,
				WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
			wmi_passpoint_match_event_id,
			wma_passpoint_match_event_handler,
			WMA_RX_SERIALIZER_CTX);

	/* Register BTM reject list event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   wmi_roam_blacklist_event_id,
					   wma_handle_btm_blacklist_event,
					   WMA_RX_SERIALIZER_CTX);
}

/**
 * wma_extscan_start_stop_event_handler() -  extscan start/stop event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: data length
 *
 * This function handles different extscan related commands
 * like start/stop/get results etc and indicate to upper layers.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_start_stop_event_handler(void *handle,
					 uint8_t *cmd_param_info,
					 uint32_t len)
{
	WMI_EXTSCAN_START_STOP_EVENTID_param_tlvs *param_buf;
	wmi_extscan_start_stop_event_fixed_param *event;
	struct sir_extscan_generic_response   *extscan_ind;
	uint16_t event_type;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_START_STOP_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid extscan event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	extscan_ind = qdf_mem_malloc(sizeof(*extscan_ind));
	if (!extscan_ind)
		return -ENOMEM;

	switch (event->command) {
	case WMI_EXTSCAN_START_CMDID:
		event_type = eSIR_EXTSCAN_START_RSP;
		extscan_ind->status = event->status;
		extscan_ind->request_id = event->request_id;
		break;
	case WMI_EXTSCAN_STOP_CMDID:
		event_type = eSIR_EXTSCAN_STOP_RSP;
		extscan_ind->status = event->status;
		extscan_ind->request_id = event->request_id;
		break;
	case WMI_EXTSCAN_CONFIGURE_WLAN_CHANGE_MONITOR_CMDID:
		extscan_ind->status = event->status;
		extscan_ind->request_id = event->request_id;
		if (event->mode == WMI_EXTSCAN_MODE_STOP)
			event_type =
				eSIR_EXTSCAN_RESET_SIGNIFICANT_WIFI_CHANGE_RSP;
		else
			event_type =
				eSIR_EXTSCAN_SET_SIGNIFICANT_WIFI_CHANGE_RSP;
		break;
	case WMI_EXTSCAN_CONFIGURE_HOTLIST_MONITOR_CMDID:
		extscan_ind->status = event->status;
		extscan_ind->request_id = event->request_id;
		if (event->mode == WMI_EXTSCAN_MODE_STOP)
			event_type = eSIR_EXTSCAN_RESET_BSSID_HOTLIST_RSP;
		else
			event_type = eSIR_EXTSCAN_SET_BSSID_HOTLIST_RSP;
		break;
	case WMI_EXTSCAN_GET_CACHED_RESULTS_CMDID:
		extscan_ind->status = event->status;
		extscan_ind->request_id = event->request_id;
		event_type = eSIR_EXTSCAN_CACHED_RESULTS_RSP;
		break;
	case WMI_EXTSCAN_CONFIGURE_HOTLIST_SSID_MONITOR_CMDID:
		extscan_ind->status = event->status;
		extscan_ind->request_id = event->request_id;
		if (event->mode == WMI_EXTSCAN_MODE_STOP)
			event_type =
				eSIR_EXTSCAN_RESET_SSID_HOTLIST_RSP;
		else
			event_type =
				eSIR_EXTSCAN_SET_SSID_HOTLIST_RSP;
		break;
	default:
		WMA_LOGE("%s: Unknown event(%d) from target",
			 __func__, event->status);
		qdf_mem_free(extscan_ind);
		return -EINVAL;
	}
	mac->sme.ext_scan_ind_cb(mac->hdd_handle, event_type, extscan_ind);
	WMA_LOGD("%s: sending event to umac for requestid %u with status %d",
		__func__, extscan_ind->request_id, extscan_ind->status);
	qdf_mem_free(extscan_ind);
	return 0;
}

/**
 * wma_extscan_operations_event_handler() - extscan operation event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: length
 *
 * This function handles different operations related event and indicate
 * upper layers with appropriate callback.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_operations_event_handler(void *handle,
					 uint8_t *cmd_param_info,
					 uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_EXTSCAN_OPERATION_EVENTID_param_tlvs *param_buf;
	wmi_extscan_operation_event_fixed_param *oprn_event;
	tSirExtScanOnScanEventIndParams *oprn_ind;
	uint32_t cnt;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_OPERATION_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid scan operation event", __func__);
		return -EINVAL;
	}
	oprn_event = param_buf->fixed_param;
	oprn_ind = qdf_mem_malloc(sizeof(*oprn_ind));
	if (!oprn_ind)
		return -ENOMEM;

	oprn_ind->requestId = oprn_event->request_id;

	switch (oprn_event->event) {
	case WMI_EXTSCAN_BUCKET_COMPLETED_EVENT:
		oprn_ind->status = 0;
		goto exit_handler;
	case WMI_EXTSCAN_CYCLE_STARTED_EVENT:
		WMA_LOGD("%s: received WMI_EXTSCAN_CYCLE_STARTED_EVENT",
			 __func__);

		if (oprn_event->num_buckets > param_buf->num_bucket_id) {
			WMA_LOGE("FW mesg num_buk %d more than TLV hdr %d",
				 oprn_event->num_buckets,
				 param_buf->num_bucket_id);
			qdf_mem_free(oprn_ind);
			return -EINVAL;
		}

		cds_host_diag_log_work(&wma->extscan_wake_lock,
				       WMA_EXTSCAN_CYCLE_WAKE_LOCK_DURATION,
				       WIFI_POWER_EVENT_WAKELOCK_EXT_SCAN);
		qdf_wake_lock_timeout_acquire(&wma->extscan_wake_lock,
				      WMA_EXTSCAN_CYCLE_WAKE_LOCK_DURATION);
		oprn_ind->scanEventType = WIFI_EXTSCAN_CYCLE_STARTED_EVENT;
		oprn_ind->status = 0;
		oprn_ind->buckets_scanned = 0;
		for (cnt = 0; cnt < oprn_event->num_buckets; cnt++)
			oprn_ind->buckets_scanned |=
				(1 << param_buf->bucket_id[cnt]);
		WMA_LOGD(FL("num_buckets %u request_id %u buckets_scanned %u"),
			oprn_event->num_buckets, oprn_ind->requestId,
			oprn_ind->buckets_scanned);
		break;
	case WMI_EXTSCAN_CYCLE_COMPLETED_EVENT:
		WMA_LOGD("%s: received WMI_EXTSCAN_CYCLE_COMPLETED_EVENT",
			 __func__);
		qdf_wake_lock_release(&wma->extscan_wake_lock,
				      WIFI_POWER_EVENT_WAKELOCK_EXT_SCAN);
		oprn_ind->scanEventType = WIFI_EXTSCAN_CYCLE_COMPLETED_EVENT;
		oprn_ind->status = 0;
		/* Set bucket scanned mask to zero on cycle complete */
		oprn_ind->buckets_scanned = 0;
		break;
	case WMI_EXTSCAN_BUCKET_STARTED_EVENT:
		WMA_LOGD("%s: received WMI_EXTSCAN_BUCKET_STARTED_EVENT",
			__func__);
		oprn_ind->scanEventType = WIFI_EXTSCAN_BUCKET_STARTED_EVENT;
		oprn_ind->status = 0;
		goto exit_handler;
	case WMI_EXTSCAN_THRESHOLD_NUM_SCANS:
		WMA_LOGD("%s: received WMI_EXTSCAN_THRESHOLD_NUM_SCANS",
			__func__);
		oprn_ind->scanEventType = WIFI_EXTSCAN_THRESHOLD_NUM_SCANS;
		oprn_ind->status = 0;
		break;
	case WMI_EXTSCAN_THRESHOLD_PERCENT:
		WMA_LOGD("%s: received WMI_EXTSCAN_THRESHOLD_PERCENT",
			__func__);
		oprn_ind->scanEventType = WIFI_EXTSCAN_THRESHOLD_PERCENT;
		oprn_ind->status = 0;
		break;
	default:
		WMA_LOGE("%s: Unknown event(%d) from target",
			 __func__, oprn_event->event);
		qdf_mem_free(oprn_ind);
		return -EINVAL;
	}
	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_SCAN_PROGRESS_EVENT_IND, oprn_ind);
	WMA_LOGI("%s: sending scan progress event to hdd", __func__);
exit_handler:
	qdf_mem_free(oprn_ind);
	return 0;
}

/**
 * wma_extscan_table_usage_event_handler() - extscan table usage event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: length
 *
 * This function handles table usage related event and indicate
 * upper layers with appropriate callback.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_table_usage_event_handler(void *handle,
					  uint8_t *cmd_param_info,
					  uint32_t len)
{
	WMI_EXTSCAN_TABLE_USAGE_EVENTID_param_tlvs *param_buf;
	wmi_extscan_table_usage_event_fixed_param *event;
	tSirExtScanResultsAvailableIndParams *tbl_usg_ind;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_TABLE_USAGE_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid table usage event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	tbl_usg_ind = qdf_mem_malloc(sizeof(*tbl_usg_ind));
	if (!tbl_usg_ind)
		return -ENOMEM;

	tbl_usg_ind->requestId = event->request_id;
	tbl_usg_ind->numResultsAvailable = event->entries_in_use;
	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_SCAN_RES_AVAILABLE_IND,
				tbl_usg_ind);
	WMA_LOGI("%s: sending scan_res available event to hdd", __func__);
	qdf_mem_free(tbl_usg_ind);
	return 0;
}

/**
 * wma_extscan_capabilities_event_handler() - extscan capabilities event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: length
 *
 * This function handles capabilities event and indicate
 * upper layers with registered callback.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_capabilities_event_handler(void *handle,
					   uint8_t *cmd_param_info,
					   uint32_t len)
{
	WMI_EXTSCAN_CAPABILITIES_EVENTID_param_tlvs *param_buf;
	wmi_extscan_capabilities_event_fixed_param *event;
	wmi_extscan_cache_capabilities *src_cache;
	wmi_extscan_hotlist_monitor_capabilities *src_hotlist;
	wmi_extscan_wlan_change_monitor_capabilities *src_change;
	struct ext_scan_capabilities_response  *dest_capab;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_CAPABILITIES_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid capabilities event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_cache = param_buf->extscan_cache_capabilities;
	src_hotlist = param_buf->hotlist_capabilities;
	src_change = param_buf->wlan_change_capabilities;

	if (!src_cache || !src_hotlist || !src_change) {
		WMA_LOGE("%s: Invalid capabilities list", __func__);
		return -EINVAL;
	}
	dest_capab = qdf_mem_malloc(sizeof(*dest_capab));
	if (!dest_capab)
		return -ENOMEM;

	dest_capab->requestId = event->request_id;
	dest_capab->max_scan_buckets = src_cache->max_buckets;
	dest_capab->max_scan_cache_size = src_cache->scan_cache_entry_size;
	dest_capab->max_ap_cache_per_scan = src_cache->max_bssid_per_scan;
	dest_capab->max_scan_reporting_threshold =
		src_cache->max_table_usage_threshold;

	dest_capab->max_hotlist_bssids = src_hotlist->max_hotlist_entries;
	dest_capab->max_rssi_sample_size =
					src_change->max_rssi_averaging_samples;
	dest_capab->max_bssid_history_entries =
		src_change->max_rssi_history_entries;
	dest_capab->max_significant_wifi_change_aps =
		src_change->max_wlan_change_entries;
	dest_capab->max_hotlist_ssids =
				event->num_extscan_hotlist_ssid;
	dest_capab->max_number_epno_networks =
				event->num_epno_networks;
	dest_capab->max_number_epno_networks_by_ssid =
				event->num_epno_networks;
	dest_capab->max_number_of_white_listed_ssid =
				event->num_roam_ssid_whitelist;
	dest_capab->max_number_of_black_listed_bssid =
				event->num_roam_bssid_blacklist;
	dest_capab->status = 0;

	WMA_LOGD("%s: request_id: %u status: %d",
		__func__, dest_capab->requestId, dest_capab->status);

	WMA_LOGD("%s: Capabilities: max_scan_buckets: %d, max_hotlist_bssids: %d, max_scan_cache_size: %d, max_ap_cache_per_scan: %d",
		 __func__, dest_capab->max_scan_buckets,
		dest_capab->max_hotlist_bssids, dest_capab->max_scan_cache_size,
		dest_capab->max_ap_cache_per_scan);
	WMA_LOGD("%s: max_scan_reporting_threshold: %d, max_rssi_sample_size: %d, max_bssid_history_entries: %d, max_significant_wifi_change_aps: %d",
		__func__, dest_capab->max_scan_reporting_threshold,
		dest_capab->max_rssi_sample_size,
		dest_capab->max_bssid_history_entries,
		dest_capab->max_significant_wifi_change_aps);

	WMA_LOGD("%s: Capabilities: max_hotlist_ssids: %d, max_number_epno_networks: %d, max_number_epno_networks_by_ssid: %d",
		 __func__, dest_capab->max_hotlist_ssids,
		dest_capab->max_number_epno_networks,
		dest_capab->max_number_epno_networks_by_ssid);
	WMA_LOGD("%s: max_number_of_white_listed_ssid: %d, max_number_of_black_listed_bssid: %d",
		__func__, dest_capab->max_number_of_white_listed_ssid,
		dest_capab->max_number_of_black_listed_bssid);

	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_GET_CAPABILITIES_IND, dest_capab);
	qdf_mem_free(dest_capab);
	return 0;
}

/**
 * wma_extscan_hotlist_match_event_handler() - hotlist match event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: length
 *
 * This function handles hotlist match event and indicate
 * upper layers with registered callback.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_hotlist_match_event_handler(void *handle,
					    uint8_t *cmd_param_info,
					    uint32_t len)
{
	WMI_EXTSCAN_HOTLIST_MATCH_EVENTID_param_tlvs *param_buf;
	wmi_extscan_hotlist_match_event_fixed_param *event;
	struct extscan_hotlist_match *dest_hotlist;
	tSirWifiScanResult *dest_ap;
	wmi_extscan_wlan_descriptor *src_hotlist;
	uint32_t numap;
	int j, ap_found = 0;
	uint32_t buf_len;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_HOTLIST_MATCH_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid hotlist match event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_hotlist = param_buf->hotlist_match;
	numap = event->total_entries;

	if (!src_hotlist || !numap) {
		WMA_LOGE("%s: Hotlist AP's list invalid", __func__);
		return -EINVAL;
	}
	if (numap > param_buf->num_hotlist_match) {
		WMA_LOGE("Invalid no of total enteries %d", numap);
		return -EINVAL;
	}
	if (numap > WMA_EXTSCAN_MAX_HOTLIST_ENTRIES) {
		WMA_LOGE("%s: Total Entries %u greater than max",
			__func__, numap);
		numap = WMA_EXTSCAN_MAX_HOTLIST_ENTRIES;
	}

	buf_len = sizeof(wmi_extscan_hotlist_match_event_fixed_param) +
		  WMI_TLV_HDR_SIZE +
		  (numap * sizeof(wmi_extscan_wlan_descriptor));

	if (buf_len > len) {
		WMA_LOGE("Invalid buf len from FW %d numap %d", len, numap);
		return -EINVAL;
	}

	dest_hotlist = qdf_mem_malloc(sizeof(*dest_hotlist) +
				      sizeof(*dest_ap) * numap);
	if (!dest_hotlist)
		return -ENOMEM;

	dest_ap = &dest_hotlist->ap[0];
	dest_hotlist->numOfAps = event->total_entries;
	dest_hotlist->requestId = event->config_request_id;

	if (event->first_entry_index +
		event->num_entries_in_page < event->total_entries)
		dest_hotlist->moreData = 1;
	else
		dest_hotlist->moreData = 0;

	WMA_LOGD("%s: Hotlist match: requestId: %u,"
		 "numOfAps: %d", __func__,
		 dest_hotlist->requestId, dest_hotlist->numOfAps);

	/*
	 * Currently firmware sends only one bss information in-case
	 * of both hotlist ap found and lost.
	 */
	for (j = 0; j < numap; j++) {
		dest_ap->rssi = 0;
		dest_ap->channel = src_hotlist->channel;
		dest_ap->ts = src_hotlist->tstamp;
		ap_found = src_hotlist->flags & WMI_HOTLIST_FLAG_PRESENCE;
		dest_ap->rtt = src_hotlist->rtt;
		dest_ap->rtt_sd = src_hotlist->rtt_sd;
		dest_ap->beaconPeriod = src_hotlist->beacon_interval;
		dest_ap->capability = src_hotlist->capabilities;
		dest_ap->ieLength = src_hotlist->ie_length;
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&src_hotlist->bssid,
					   dest_ap->bssid.bytes);
		if (src_hotlist->ssid.ssid_len > WLAN_SSID_MAX_LEN) {
			WMA_LOGE("%s Invalid SSID len %d, truncating",
				 __func__, src_hotlist->ssid.ssid_len);
			src_hotlist->ssid.ssid_len = WLAN_SSID_MAX_LEN;
		}
		qdf_mem_copy(dest_ap->ssid, src_hotlist->ssid.ssid,
			     src_hotlist->ssid.ssid_len);
		dest_ap->ssid[src_hotlist->ssid.ssid_len] = '\0';
		dest_ap++;
		src_hotlist++;
	}
	dest_hotlist->ap_found = ap_found;
	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_HOTLIST_MATCH_IND, dest_hotlist);
	WMA_LOGI("%s: sending hotlist match event to hdd", __func__);
	qdf_mem_free(dest_hotlist);
	return 0;
}

/** wma_extscan_find_unique_scan_ids() - find unique scan ids
 * @cmd_param_info: event data.
 *
 * This utility function parses the input bss table of information
 * and find the unique number of scan ids
 *
 * Return: 0 on success; error number otherwise
 */
static int wma_extscan_find_unique_scan_ids(const u_int8_t *cmd_param_info)
{
	WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *param_buf;
	wmi_extscan_cached_results_event_fixed_param  *event;
	wmi_extscan_wlan_descriptor  *src_hotlist;
	wmi_extscan_rssi_info  *src_rssi;
	int prev_scan_id, scan_ids_cnt, i;

	param_buf = (WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *)
						cmd_param_info;
	event = param_buf->fixed_param;
	src_hotlist = param_buf->bssid_list;
	src_rssi = param_buf->rssi_list;

	/* Find the unique number of scan_id's for grouping */
	prev_scan_id = src_rssi->scan_cycle_id;
	scan_ids_cnt = 1;
	for (i = 1; i < param_buf->num_rssi_list; i++) {
		src_rssi++;

		if (prev_scan_id != src_rssi->scan_cycle_id) {
			scan_ids_cnt++;
			prev_scan_id = src_rssi->scan_cycle_id;
		}
	}

	return scan_ids_cnt;
}

/** wma_fill_num_results_per_scan_id() - fill number of bss per scan id
 * @cmd_param_info: event data.
 * @scan_id_group: pointer to scan id group.
 *
 * This utility function parses the input bss table of information
 * and finds how many bss are there per unique scan id.
 *
 * Return: 0 on success; error number otherwise
 */
static int wma_fill_num_results_per_scan_id(const u_int8_t *cmd_param_info,
			struct extscan_cached_scan_result *scan_id_group)
{
	WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *param_buf;
	wmi_extscan_cached_results_event_fixed_param  *event;
	wmi_extscan_wlan_descriptor  *src_hotlist;
	wmi_extscan_rssi_info  *src_rssi;
	struct extscan_cached_scan_result *t_scan_id_grp;
	int i, prev_scan_id;

	param_buf = (WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *)
						cmd_param_info;
	event = param_buf->fixed_param;
	src_hotlist = param_buf->bssid_list;
	src_rssi = param_buf->rssi_list;
	t_scan_id_grp = scan_id_group;

	prev_scan_id = src_rssi->scan_cycle_id;

	t_scan_id_grp->scan_id = src_rssi->scan_cycle_id;
	t_scan_id_grp->flags = src_rssi->flags;
	t_scan_id_grp->buckets_scanned = src_rssi->buckets_scanned;
	t_scan_id_grp->num_results = 1;
	for (i = 1; i < param_buf->num_rssi_list; i++) {
		src_rssi++;
		if (prev_scan_id == src_rssi->scan_cycle_id) {
			t_scan_id_grp->num_results++;
		} else {
			t_scan_id_grp++;
			prev_scan_id = t_scan_id_grp->scan_id =
				src_rssi->scan_cycle_id;
			t_scan_id_grp->flags = src_rssi->flags;
			t_scan_id_grp->buckets_scanned =
				src_rssi->buckets_scanned;
			t_scan_id_grp->num_results = 1;
		}
	}
	return 0;
}

/** wma_group_num_bss_to_scan_id() - group bss to scan id table
 * @cmd_param_info: event data.
 * @cached_result: pointer to cached table.
 *
 * This function reads the bss information from the format
 * ------------------------------------------------------------------------
 * | bss info {rssi, channel, ssid, bssid, timestamp} | scan id_1 | flags |
 * | bss info {rssi, channel, ssid, bssid, timestamp} | scan id_2 | flags |
 * ........................................................................
 * | bss info {rssi, channel, ssid, bssid, timestamp} | scan id_N | flags |
 * ------------------------------------------------------------------------
 *
 * and converts it into the below format and store it
 *
 * ------------------------------------------------------------------------
 * | scan id_1 | -> bss info_1 -> bss info_2 -> .... bss info_M1
 * | scan id_2 | -> bss info_1 -> bss info_2 -> .... bss info_M2
 * ......................
 * | scan id_N | -> bss info_1 -> bss info_2 -> .... bss info_Mn
 * ------------------------------------------------------------------------
 *
 * Return: 0 on success; error number otherwise
 */
static int wma_group_num_bss_to_scan_id(const u_int8_t *cmd_param_info,
			struct extscan_cached_scan_results *cached_result)
{
	WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *param_buf;
	wmi_extscan_cached_results_event_fixed_param  *event;
	wmi_extscan_wlan_descriptor  *src_hotlist;
	wmi_extscan_rssi_info  *src_rssi;
	struct extscan_cached_scan_results *t_cached_result;
	struct extscan_cached_scan_result *t_scan_id_grp;
	int i, j;
	tSirWifiScanResult *ap;

	param_buf = (WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *)
						cmd_param_info;
	event = param_buf->fixed_param;
	src_hotlist = param_buf->bssid_list;
	src_rssi = param_buf->rssi_list;
	t_cached_result = cached_result;
	t_scan_id_grp = &t_cached_result->result[0];

	if ((t_cached_result->num_scan_ids *
	     QDF_MIN(t_scan_id_grp->num_results,
		     param_buf->num_bssid_list)) > param_buf->num_bssid_list) {
		WMA_LOGE("%s:num_scan_ids %d, num_results %d num_bssid_list %d",
			 __func__,
			 t_cached_result->num_scan_ids,
			 t_scan_id_grp->num_results,
			 param_buf->num_bssid_list);
		return -EINVAL;
	}

	WMA_LOGD("%s: num_scan_ids:%d", __func__,
			t_cached_result->num_scan_ids);
	for (i = 0; i < t_cached_result->num_scan_ids; i++) {
		WMA_LOGD("%s: num_results:%d", __func__,
			t_scan_id_grp->num_results);
		t_scan_id_grp->ap = qdf_mem_malloc(t_scan_id_grp->num_results *
						sizeof(*ap));
		if (!t_scan_id_grp->ap)
			return -ENOMEM;

		ap = &t_scan_id_grp->ap[0];
		for (j = 0; j < QDF_MIN(t_scan_id_grp->num_results,
					param_buf->num_bssid_list); j++) {
			ap->channel = src_hotlist->channel;
			ap->ts = WMA_MSEC_TO_USEC(src_rssi->tstamp);
			ap->rtt = src_hotlist->rtt;
			ap->rtt_sd = src_hotlist->rtt_sd;
			ap->beaconPeriod = src_hotlist->beacon_interval;
			ap->capability = src_hotlist->capabilities;
			ap->ieLength = src_hotlist->ie_length;

			/* Firmware already applied noise floor adjustment and
			 * due to WMI interface "UINT32 rssi", host driver
			 * receives a positive value, hence convert to
			 * signed char to get the absolute rssi.
			 */
			ap->rssi = (signed char) src_rssi->rssi;
			WMI_MAC_ADDR_TO_CHAR_ARRAY(&src_hotlist->bssid,
						   ap->bssid.bytes);

			if (src_hotlist->ssid.ssid_len >
			    WLAN_SSID_MAX_LEN) {
				WMA_LOGD("%s Invalid SSID len %d, truncating",
					 __func__, src_hotlist->ssid.ssid_len);
				src_hotlist->ssid.ssid_len =
						WLAN_SSID_MAX_LEN;
			}
			qdf_mem_copy(ap->ssid, src_hotlist->ssid.ssid,
					src_hotlist->ssid.ssid_len);
			ap->ssid[src_hotlist->ssid.ssid_len] = '\0';
			ap++;
			src_rssi++;
			src_hotlist++;
		}
		t_scan_id_grp++;
	}
	return 0;
}

/**
 * wma_extscan_cached_results_event_handler() - cached results event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: length of @cmd_param_info
 *
 * This function handles cached results event and indicate
 * cached results to upper layer.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_cached_results_event_handler(void *handle,
					     uint8_t *cmd_param_info,
					     uint32_t len)
{
	WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *param_buf;
	wmi_extscan_cached_results_event_fixed_param *event;
	struct extscan_cached_scan_results *dest_cachelist;
	struct extscan_cached_scan_result *dest_result;
	struct extscan_cached_scan_results empty_cachelist;
	wmi_extscan_wlan_descriptor *src_hotlist;
	wmi_extscan_rssi_info *src_rssi;
	int i, moredata, scan_ids_cnt, buf_len, status;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t total_len;
	bool excess_data = false;

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid cached results event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_hotlist = param_buf->bssid_list;
	src_rssi = param_buf->rssi_list;
	WMA_LOGI("Total_entries: %u first_entry_index: %u num_entries_in_page: %d",
			event->total_entries,
			event->first_entry_index,
			event->num_entries_in_page);

	if (!src_hotlist || !src_rssi || !event->num_entries_in_page) {
		WMA_LOGW("%s: Cached results empty, send 0 results", __func__);
		goto noresults;
	}

	if (event->num_entries_in_page >
	    (WMI_SVC_MSG_MAX_SIZE - sizeof(*event))/sizeof(*src_hotlist) ||
	    event->num_entries_in_page > param_buf->num_bssid_list) {
		WMA_LOGE("%s:excess num_entries_in_page %d in WMI event. num_bssid_list %d",
			 __func__,
			 event->num_entries_in_page, param_buf->num_bssid_list);
		return -EINVAL;
	} else {
		total_len = sizeof(*event) +
			(event->num_entries_in_page * sizeof(*src_hotlist));
	}
	for (i = 0; i < event->num_entries_in_page; i++) {
		if (src_hotlist[i].ie_length >
		    WMI_SVC_MSG_MAX_SIZE - total_len) {
			excess_data = true;
			break;
		} else {
			total_len += src_hotlist[i].ie_length;
			WMA_LOGD("total len IE: %d", total_len);
		}

		if (src_hotlist[i].number_rssi_samples >
		    (WMI_SVC_MSG_MAX_SIZE - total_len) / sizeof(*src_rssi)) {
			excess_data = true;
			break;
		} else {
			total_len += (src_hotlist[i].number_rssi_samples *
					sizeof(*src_rssi));
			WMA_LOGD("total len RSSI samples: %d", total_len);
		}
	}
	if (excess_data) {
		WMA_LOGE("%s:excess data in WMI event",
			 __func__);
		return -EINVAL;
	}

	if (event->first_entry_index +
	    event->num_entries_in_page < event->total_entries)
		moredata = 1;
	else
		moredata = 0;

	dest_cachelist = qdf_mem_malloc(sizeof(*dest_cachelist));
	if (!dest_cachelist)
		return -ENOMEM;

	qdf_mem_zero(dest_cachelist, sizeof(*dest_cachelist));
	dest_cachelist->request_id = event->request_id;
	dest_cachelist->more_data = moredata;

	scan_ids_cnt = wma_extscan_find_unique_scan_ids(cmd_param_info);
	WMA_LOGD("%s: scan_ids_cnt %d", __func__, scan_ids_cnt);
	dest_cachelist->num_scan_ids = scan_ids_cnt;

	buf_len = sizeof(*dest_result) * scan_ids_cnt;
	dest_cachelist->result = qdf_mem_malloc(buf_len);
	if (!dest_cachelist->result) {
		qdf_mem_free(dest_cachelist);
		return -ENOMEM;
	}

	dest_result = dest_cachelist->result;
	wma_fill_num_results_per_scan_id(cmd_param_info, dest_result);

	status = wma_group_num_bss_to_scan_id(cmd_param_info, dest_cachelist);
	if (!status)
	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_CACHED_RESULTS_IND,
				dest_cachelist);
	else
		WMA_LOGD("wma_group_num_bss_to_scan_id failed, not calling callback");

	dest_result = dest_cachelist->result;
	for (i = 0; i < dest_cachelist->num_scan_ids; i++) {
		if (dest_result->ap)
		qdf_mem_free(dest_result->ap);
		dest_result++;
	}
	qdf_mem_free(dest_cachelist->result);
	qdf_mem_free(dest_cachelist);
	return status;

noresults:
	empty_cachelist.request_id = event->request_id;
	empty_cachelist.more_data = 0;
	empty_cachelist.num_scan_ids = 0;

	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_CACHED_RESULTS_IND,
				&empty_cachelist);
	return 0;
}

/**
 * wma_extscan_change_results_event_handler() - change results event handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: length
 *
 * This function handles change results event and indicate
 * change results to upper layer.
 *
 * Return: 0 for success or error code.
 */
int wma_extscan_change_results_event_handler(void *handle,
					     uint8_t *cmd_param_info,
					     uint32_t len)
{
	WMI_EXTSCAN_WLAN_CHANGE_RESULTS_EVENTID_param_tlvs *param_buf;
	wmi_extscan_wlan_change_results_event_fixed_param *event;
	tSirWifiSignificantChangeEvent *dest_chglist;
	tSirWifiSignificantChange *dest_ap;
	wmi_extscan_wlan_change_result_bssid *src_chglist;

	uint32_t numap;
	int i, k;
	uint8_t *src_rssi;
	int count = 0;
	int moredata;
	uint32_t rssi_num = 0;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t buf_len;
	bool excess_data = false;

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_WLAN_CHANGE_RESULTS_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid change monitor event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_chglist = param_buf->bssid_signal_descriptor_list;
	src_rssi = param_buf->rssi_list;
	numap = event->num_entries_in_page;

	if (!src_chglist || !numap) {
		WMA_LOGE("%s: Results invalid", __func__);
		return -EINVAL;
	}
	if (numap > param_buf->num_bssid_signal_descriptor_list) {
		WMA_LOGE("%s: Invalid num of entries in page: %d", __func__, numap);
		return -EINVAL;
	}
	for (i = 0; i < numap; i++) {
		if (src_chglist->num_rssi_samples > (UINT_MAX - rssi_num)) {
			WMA_LOGE("%s: Invalid num of rssi samples %d numap %d rssi_num %d",
				 __func__, src_chglist->num_rssi_samples,
				 numap, rssi_num);
			return -EINVAL;
		}
		rssi_num += src_chglist->num_rssi_samples;
		src_chglist++;
	}
	src_chglist = param_buf->bssid_signal_descriptor_list;

	if (event->first_entry_index +
	    event->num_entries_in_page < event->total_entries) {
		moredata = 1;
	} else {
		moredata = 0;
	}

	do {
		if (event->num_entries_in_page >
			(WMI_SVC_MSG_MAX_SIZE - sizeof(*event))/
			sizeof(*src_chglist)) {
			excess_data = true;
			break;
		} else {
			buf_len =
				sizeof(*event) + (event->num_entries_in_page *
						sizeof(*src_chglist));
		}
		if (rssi_num >
			(WMI_SVC_MSG_MAX_SIZE - buf_len)/sizeof(int32_t)) {
			excess_data = true;
			break;
		}
	} while (0);

	if (excess_data) {
		WMA_LOGE("buffer len exceeds WMI payload,numap:%d, rssi_num:%d",
				numap, rssi_num);
		QDF_ASSERT(0);
		return -EINVAL;
	}
	dest_chglist = qdf_mem_malloc(sizeof(*dest_chglist) +
				      sizeof(*dest_ap) * numap +
				      sizeof(int32_t) * rssi_num);
	if (!dest_chglist)
		return -ENOMEM;

	dest_ap = &dest_chglist->ap[0];
	for (i = 0; i < numap; i++) {
		dest_ap->channel = src_chglist->channel;
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&src_chglist->bssid,
					   dest_ap->bssid.bytes);
		dest_ap->numOfRssi = src_chglist->num_rssi_samples;
		if (dest_ap->numOfRssi) {
			if ((dest_ap->numOfRssi + count) >
			    param_buf->num_rssi_list) {
				WMA_LOGE("%s: Invalid num in rssi list: %d",
					__func__, dest_ap->numOfRssi);
				qdf_mem_free(dest_chglist);
				return -EINVAL;
			}
			for (k = 0; k < dest_ap->numOfRssi; k++) {
				dest_ap->rssi[k] = WMA_TGT_NOISE_FLOOR_DBM +
						   src_rssi[count++];
			}
		}
		dest_ap = (tSirWifiSignificantChange *)((char *)dest_ap +
					dest_ap->numOfRssi * sizeof(int32_t) +
					sizeof(*dest_ap));
		src_chglist++;
	}
	dest_chglist->requestId = event->request_id;
	dest_chglist->moreData = moredata;
	dest_chglist->numResults = numap;

	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
			eSIR_EXTSCAN_SIGNIFICANT_WIFI_CHANGE_RESULTS_IND,
			dest_chglist);
	WMA_LOGI("%s: sending change monitor results", __func__);
	qdf_mem_free(dest_chglist);
	return 0;
}

/**
 * wma_passpoint_match_event_handler() - passpoint match found event handler
 * @handle: WMA handle
 * @cmd_param_info: event data
 * @len: event data length
 *
 * This is the passpoint match found event handler; it reads event data from
 * @cmd_param_info and fill in the destination buffer and sends indication
 * up layer.
 *
 * Return: 0 on success; error number otherwise
 */
int wma_passpoint_match_event_handler(void *handle,
				     uint8_t  *cmd_param_info,
				     uint32_t len)
{
	WMI_PASSPOINT_MATCH_EVENTID_param_tlvs *param_buf;
	wmi_passpoint_event_hdr  *event;
	struct wifi_passpoint_match  *dest_match;
	tSirWifiScanResult      *dest_ap;
	uint8_t *buf_ptr;
	uint32_t buf_len = 0;
	bool excess_data = false;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac", __func__);
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		WMA_LOGE("%s: Callback not registered", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_PASSPOINT_MATCH_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid passpoint match event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	buf_ptr = (uint8_t *)param_buf->fixed_param;

	do {
		if (event->ie_length > (WMI_SVC_MSG_MAX_SIZE)) {
			excess_data = true;
			break;
		} else {
			buf_len = event->ie_length;
		}

		if (event->anqp_length > (WMI_SVC_MSG_MAX_SIZE)) {
			excess_data = true;
			break;
		} else {
			buf_len += event->anqp_length;
		}

	} while (0);

	if (excess_data || buf_len > (WMI_SVC_MSG_MAX_SIZE - sizeof(*event)) ||
	    buf_len > (WMI_SVC_MSG_MAX_SIZE - sizeof(*dest_match)) ||
	    (event->ie_length + event->anqp_length) > param_buf->num_bufp) {
		WMA_LOGE("IE Length: %u or ANQP Length: %u is huge, num_bufp: %u",
			event->ie_length, event->anqp_length,
			param_buf->num_bufp);
		return -EINVAL;
	}

	if (event->ssid.ssid_len > WLAN_SSID_MAX_LEN) {
		WMA_LOGD("%s: Invalid ssid len %d, truncating",
			 __func__, event->ssid.ssid_len);
		event->ssid.ssid_len = WLAN_SSID_MAX_LEN;
	}

	dest_match = qdf_mem_malloc(sizeof(*dest_match) + buf_len);

	if (!dest_match)
		return -EINVAL;

	dest_ap = &dest_match->ap;
	dest_match->request_id = 0;
	dest_match->id = event->id;
	dest_match->anqp_len = event->anqp_length;
	WMA_LOGI("%s: passpoint match: id: %u anqp length %u", __func__,
		 dest_match->id, dest_match->anqp_len);

	dest_ap->channel = event->channel_mhz;
	dest_ap->ts = event->timestamp;
	dest_ap->rtt = event->rtt;
	dest_ap->rssi = event->rssi;
	dest_ap->rtt_sd = event->rtt_sd;
	dest_ap->beaconPeriod = event->beacon_period;
	dest_ap->capability = event->capability;
	dest_ap->ieLength = event->ie_length;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->bssid, dest_ap->bssid.bytes);
	qdf_mem_copy(dest_ap->ssid, event->ssid.ssid,
				event->ssid.ssid_len);
	dest_ap->ssid[event->ssid.ssid_len] = '\0';
	qdf_mem_copy(dest_ap->ieData, buf_ptr + sizeof(*event) +
			WMI_TLV_HDR_SIZE, dest_ap->ieLength);
	qdf_mem_copy(dest_match->anqp, buf_ptr + sizeof(*event) +
			WMI_TLV_HDR_SIZE + dest_ap->ieLength,
			dest_match->anqp_len);

	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_PASSPOINT_NETWORK_FOUND_IND,
				dest_match);
	WMA_LOGI("%s: sending passpoint match event to hdd", __func__);
	qdf_mem_free(dest_match);
	return 0;
}

QDF_STATUS wma_start_extscan(tp_wma_handle wma,
			     struct wifi_scan_cmd_req_params *params)
{
	QDF_STATUS status;

	if (!wma || !wma->wmi_handle) {
		wma_err("WMA is closed, can not issue cmd");
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	if (!params) {
		wma_err("NULL param");
		return QDF_STATUS_E_NOMEM;
	}

	status = wmi_unified_start_extscan_cmd(wma->wmi_handle, params);
	if (QDF_IS_STATUS_SUCCESS(status))
		wma->interfaces[params->vdev_id].extscan_in_progress = true;

	wma_debug("Exit, vdev %d, status %d", params->vdev_id, status);

	return status;
}

QDF_STATUS wma_stop_extscan(tp_wma_handle wma,
			    struct extscan_stop_req_params *params)
{
	QDF_STATUS status;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, cannot issue cmd", __func__);
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		WMA_LOGE("%s: extscan not enabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	status = wmi_unified_stop_extscan_cmd(wma->wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma->interfaces[params->vdev_id].extscan_in_progress = false;
	WMA_LOGD("Extscan stop request sent successfully for vdev %d",
		 params->vdev_id);

	return status;
}

QDF_STATUS wma_extscan_start_hotlist_monitor(tp_wma_handle wma,
			struct extscan_bssid_hotlist_set_params *params)
{
	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue hotlist cmd",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!params) {
		WMA_LOGE("%s: Invalid params", __func__);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_extscan_start_hotlist_monitor_cmd(wma->wmi_handle,
							     params);
}

QDF_STATUS wma_extscan_stop_hotlist_monitor(tp_wma_handle wma,
		    struct extscan_bssid_hotlist_reset_params *params)
{
	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue cmd", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!params) {
		WMA_LOGE("%s: Invalid params", __func__);
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma->wmi_handle,
				 wmi_service_extscan)) {
		WMA_LOGE("%s: extscan not enabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_stop_hotlist_monitor_cmd(wma->wmi_handle,
							    params);
}

QDF_STATUS
wma_extscan_start_change_monitor(tp_wma_handle wma,
			struct extscan_set_sig_changereq_params *params)
{
	QDF_STATUS status;

	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed,can not issue cmd",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!params) {
		WMA_LOGE("%s: NULL params", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	status = wmi_unified_extscan_start_change_monitor_cmd(wma->wmi_handle,
							      params);
	return status;
}

QDF_STATUS wma_extscan_stop_change_monitor(tp_wma_handle wma,
			struct extscan_capabilities_reset_params *params)
{
	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue  cmd", __func__);
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma->wmi_handle,
				    wmi_service_extscan)) {
		WMA_LOGE("%s: ext scan not enabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_stop_change_monitor_cmd(wma->wmi_handle,
							   params);
}

QDF_STATUS
wma_extscan_get_cached_results(tp_wma_handle wma,
			       struct extscan_cached_result_params *params)
{
	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, cannot issue cmd", __func__);
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		WMA_LOGE("%s: extscan not enabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_get_cached_results_cmd(wma->wmi_handle,
							  params);
}

QDF_STATUS
wma_extscan_get_capabilities(tp_wma_handle wma,
			     struct extscan_capabilities_params *params)
{
	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue cmd", __func__);
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		WMA_LOGE("%s: extscan not enabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_get_capabilities_cmd(wma->wmi_handle,
							params);
}

QDF_STATUS wma_set_epno_network_list(tp_wma_handle wma,
				     struct wifi_enhanced_pno_params *req)
{
	QDF_STATUS status;

	wma_debug("Enter");

	if (!wma || !wma->wmi_handle) {
		wma_err("WMA is closed, can not issue cmd");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = wmi_unified_set_epno_network_list_cmd(wma->wmi_handle, req);
	wma_debug("Exit, vdev %d, status %d", req->vdev_id, status);

	return status;
}

QDF_STATUS
wma_set_passpoint_network_list(tp_wma_handle wma,
			       struct wifi_passpoint_req_param *params)
{
	QDF_STATUS status;

	wma_debug("Enter");

	if (!wma || !wma->wmi_handle) {
		wma_err("WMA is closed, can not issue cmd");
		return QDF_STATUS_E_FAILURE;
	}
	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = wmi_unified_set_passpoint_network_list_cmd(wma->wmi_handle,
							    params);
	wma_debug("Exit, vdev %d, status %d", params->vdev_id, status);

	return status;
}

QDF_STATUS
wma_reset_passpoint_network_list(tp_wma_handle wma,
				 struct wifi_passpoint_req_param *params)
{
	QDF_STATUS status;

	wma_debug("Enter");

	if (!wma || !wma->wmi_handle) {
		wma_err("WMA is closed, can not issue cmd");
		return QDF_STATUS_E_FAILURE;
	}
	if (!wmi_service_enabled(wma->wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = wmi_unified_reset_passpoint_network_list_cmd(wma->wmi_handle,
							      params);
	wma_debug("Exit, vdev %d, status %d", params->vdev_id, status);

	return status;
}

#endif

QDF_STATUS wma_scan_probe_setoui(tp_wma_handle wma,
				 struct scan_mac_oui *set_oui)
{
	if (!wma || !wma->wmi_handle) {
		WMA_LOGE("%s: WMA is closed, can not issue  cmd", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_is_vdev_valid(set_oui->vdev_id)) {
		WMA_LOGE("%s: vdev_id: %d is not active", __func__,
			 set_oui->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_scan_probe_setoui_cmd(wma->wmi_handle, set_oui);
}

/**
 * wma_roam_better_ap_handler() - better ap event handler
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Handler for WMI_ROAM_REASON_BETTER_AP event from roam firmware in Rome.
 * This event means roam algorithm in Rome has found a better matching
 * candidate AP. The indication is sent to SME.
 *
 * Return: none
 */
void wma_roam_better_ap_handler(tp_wma_handle wma, uint32_t vdev_id)
{
	struct scheduler_msg cds_msg = {0};
	tSirSmeCandidateFoundInd *candidate_ind;

	candidate_ind = qdf_mem_malloc(sizeof(tSirSmeCandidateFoundInd));
	if (!candidate_ind)
		return;

	wma->interfaces[vdev_id].roaming_in_progress = true;
	candidate_ind->messageType = eWNI_SME_CANDIDATE_FOUND_IND;
	candidate_ind->sessionId = vdev_id;
	candidate_ind->length = sizeof(tSirSmeCandidateFoundInd);

	cds_msg.type = eWNI_SME_CANDIDATE_FOUND_IND;
	cds_msg.bodyptr = candidate_ind;
	cds_msg.callback = sme_mc_process_handler;
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_INFO,
		  FL("posting candidate ind to SME, vdev %d"), vdev_id);

	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_SCAN,
							 &cds_msg)) {
		qdf_mem_free(candidate_ind);
	}
}

/**
 * wma_handle_btm_disassoc_imminent_msg() - Send del sta msg to lim on receiving
 * BTM request from AP with disassoc imminent reason
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 *
 * Return: None
 */
static void wma_handle_btm_disassoc_imminent_msg(tp_wma_handle wma_handle,
						 uint32_t vdev_id)
{
	tpDeleteStaContext del_sta_ctx;

	del_sta_ctx = qdf_mem_malloc(sizeof(tDeleteStaContext));
	if (!del_sta_ctx)
		return;

	del_sta_ctx->vdev_id = vdev_id;
	del_sta_ctx->reasonCode = HAL_DEL_STA_REASON_CODE_BTM_DISASSOC_IMMINENT;
	wma_send_msg(wma_handle, SIR_LIM_DELETE_STA_CONTEXT_IND,
		     (void *)del_sta_ctx, 0);
}

/**
 * wma_handle_hw_mode_in_roam_fail() - Fill hw mode info if present in policy
 * manager.
 * @wma: wma handle
 * @param: roam event params
 *
 * Return: None
 */
static int wma_handle_hw_mode_transition(tp_wma_handle wma,
					 WMI_ROAM_EVENTID_param_tlvs *param)
{
	struct sir_hw_mode_trans_ind *hw_mode_trans_ind;
	struct scheduler_msg sme_msg = {0};
	QDF_STATUS status;

	if (param->hw_mode_transition_fixed_param) {
		hw_mode_trans_ind = qdf_mem_malloc(sizeof(*hw_mode_trans_ind));
		if (!hw_mode_trans_ind)
			return -ENOMEM;
		wma_process_pdev_hw_mode_trans_ind(wma,
		    param->hw_mode_transition_fixed_param,
		    param->wmi_pdev_set_hw_mode_response_vdev_mac_mapping,
		    hw_mode_trans_ind);

		WMA_LOGI(FL("Update HW mode"));
		sme_msg.type = eWNI_SME_HW_MODE_TRANS_IND;
		sme_msg.bodyptr = hw_mode_trans_ind;

		status = scheduler_post_message(QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_SME,
						QDF_MODULE_ID_SME, &sme_msg);
		if (QDF_IS_STATUS_ERROR(status))
			qdf_mem_free(hw_mode_trans_ind);
	} else {
		WMA_LOGD(FL("hw_mode transition fixed param is NULL"));
	}

	return 0;
}

/**
 * wma_invalid_roam_reason_handler() - Handle Invalid roam notification
 * @wma: wma handle
 * @vdev_id: vdev id
 * @op_code: Operation to be done by the callback
 *
 * This function calls pe and csr callbacks with proper op_code
 *
 * Return: None
 */
static void wma_invalid_roam_reason_handler(tp_wma_handle wma_handle,
					    uint32_t vdev_id,
					    uint32_t notif)
{
	struct roam_offload_synch_ind *roam_synch_data;
	enum sir_roam_op_code op_code;

	if (notif == WMI_ROAM_NOTIF_ROAM_START) {
		wma_handle->interfaces[vdev_id].roaming_in_progress = true;
		op_code = SIR_ROAMING_START;
	} else if (notif == WMI_ROAM_NOTIF_ROAM_ABORT) {
		wma_handle->interfaces[vdev_id].roaming_in_progress = false;
		op_code = SIR_ROAMING_ABORT;
	} else {
		WMA_LOGE(FL("Invalid notif %d"), notif);
		return;
	}

	roam_synch_data = qdf_mem_malloc(sizeof(*roam_synch_data));
	if (!roam_synch_data)
		return;

	roam_synch_data->roamed_vdev_id = vdev_id;
	wma_handle->pe_roam_synch_cb(wma_handle->mac_context, roam_synch_data,
				     NULL, op_code);
	wma_handle->csr_roam_synch_cb(wma_handle->mac_context, roam_synch_data,
				      NULL, op_code);

	qdf_mem_free(roam_synch_data);
}

void wma_handle_roam_sync_timeout(tp_wma_handle wma_handle,
				  struct roam_sync_timeout_timer_info *info)
{
	wma_invalid_roam_reason_handler(wma_handle, info->vdev_id,
					WMI_ROAM_NOTIF_ROAM_ABORT);
}

/**
 * wma_roam_event_callback() - roam event callback
 * @handle: wma handle
 * @event_buf: event buffer
 * @len: buffer length
 *
 * Handler for all events from roam engine in firmware
 *
 * Return: 0 for success or error code
 */
int wma_roam_event_callback(WMA_HANDLE handle, uint8_t *event_buf,
				   uint32_t len)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	WMI_ROAM_EVENTID_param_tlvs *param_buf;
	wmi_roam_event_fixed_param *wmi_event;
	struct roam_offload_synch_ind *roam_synch_data;
	uint8_t *frame = NULL;

	param_buf = (WMI_ROAM_EVENTID_param_tlvs *) event_buf;
	if (!param_buf) {
		WMA_LOGE("Invalid roam event buffer");
		return -EINVAL;
	}

	wmi_event = param_buf->fixed_param;
	WMA_LOGD("%s: Reason %x, Notif %x for vdevid %x, rssi %d",
		 __func__, wmi_event->reason, wmi_event->notif,
		 wmi_event->vdev_id, wmi_event->rssi);

	if (wmi_event->vdev_id >= wma_handle->max_bssid) {
		WMA_LOGE("Invalid vdev id from firmware");
		return -EINVAL;
	}
	wlan_roam_debug_log(wmi_event->vdev_id, DEBUG_ROAM_EVENT,
			    DEBUG_INVALID_PEER_ID, NULL, NULL,
			    wmi_event->reason,
			    (wmi_event->reason == WMI_ROAM_REASON_INVALID) ?
				wmi_event->notif : wmi_event->rssi);

	DPTRACE(qdf_dp_trace_record_event(QDF_DP_TRACE_EVENT_RECORD,
		wmi_event->vdev_id, QDF_TRACE_DEFAULT_PDEV_ID,
		QDF_PROTO_TYPE_EVENT, QDF_ROAM_EVENTID));

	switch (wmi_event->reason) {
	case WMI_ROAM_REASON_BTM:
		/*
		 * This event is received from firmware if firmware is unable to
		 * find candidate AP after roam scan and BTM request from AP
		 * has disassoc imminent bit set.
		 */
		WMA_LOGD("Kickout due to btm request");
		wma_sta_kickout_event(HOST_STA_KICKOUT_REASON_BTM,
				      wmi_event->vdev_id, NULL);
		wma_handle_btm_disassoc_imminent_msg(wma_handle,
						   wmi_event->vdev_id);
		break;
	case WMI_ROAM_REASON_BMISS:
		/*
		 * WMI_ROAM_REASON_BMISS can get called in soft IRQ context, so
		 * avoid using CSR/PE structure directly
		 */
		WMA_LOGD("Beacon Miss for vdevid %x", wmi_event->vdev_id);
		wma_beacon_miss_handler(wma_handle, wmi_event->vdev_id,
					wmi_event->rssi);
		wma_sta_kickout_event(HOST_STA_KICKOUT_REASON_BMISS,
						wmi_event->vdev_id, NULL);
		break;
	case WMI_ROAM_REASON_BETTER_AP:
		/*
		 * WMI_ROAM_REASON_BETTER_AP can get called in soft IRQ context,
		 * so avoid using CSR/PE structure directly.
		 */
		WMA_LOGD("%s:Better AP found for vdevid %x, rssi %d", __func__,
			 wmi_event->vdev_id, wmi_event->rssi);
		wma_handle->suitable_ap_hb_failure = false;
		wma_roam_better_ap_handler(wma_handle, wmi_event->vdev_id);
		break;
	case WMI_ROAM_REASON_SUITABLE_AP:
		/*
		 * WMI_ROAM_REASON_SUITABLE_AP can get called in soft IRQ
		 * context, so avoid using CSR/PE structure directly.
		 */
		wma_handle->suitable_ap_hb_failure = true;
		wma_handle->suitable_ap_hb_failure_rssi = wmi_event->rssi;
		WMA_LOGD("%s:Bmiss scan AP found for vdevid %x, rssi %d",
			 __func__, wmi_event->vdev_id, wmi_event->rssi);
		wma_roam_better_ap_handler(wma_handle, wmi_event->vdev_id);
		break;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	case WMI_ROAM_REASON_HO_FAILED:
		/*
		 * WMI_ROAM_REASON_HO_FAILED can get called in soft IRQ context,
		 * so avoid using CSR/PE structure directly.
		 */
		WMA_LOGE("LFR3:Hand-Off Failed for vdevid %x",
			 wmi_event->vdev_id);
		wma_handle_hw_mode_transition(wma_handle, param_buf);
		wma_roam_ho_fail_handler(wma_handle, wmi_event->vdev_id);
		wma_handle->interfaces[wmi_event->vdev_id].
			roaming_in_progress = false;
		break;
#endif
	case WMI_ROAM_REASON_INVALID:
		wma_invalid_roam_reason_handler(wma_handle, wmi_event->vdev_id,
						wmi_event->notif);
		break;
	case WMI_ROAM_REASON_RSO_STATUS:
		wma_rso_cmd_status_event_handler(wmi_event);
		break;
	case WMI_ROAM_REASON_INVOKE_ROAM_FAIL:
		wma_handle_hw_mode_transition(wma_handle, param_buf);
		roam_synch_data = qdf_mem_malloc(sizeof(*roam_synch_data));
		if (!roam_synch_data)
			return -ENOMEM;

		roam_synch_data->roamed_vdev_id = wmi_event->vdev_id;
		wma_handle->csr_roam_synch_cb(wma_handle->mac_context,
					      roam_synch_data, NULL,
					      SIR_ROAMING_INVOKE_FAIL);
		qdf_mem_free(roam_synch_data);
		break;
	case WMI_ROAM_REASON_DEAUTH:
		WMA_LOGD("%s: Received disconnect roam event reason:%d",
			 __func__, wmi_event->notif);

		if (wmi_event->notif_params1)
			frame = param_buf->deauth_disassoc_frame;
		wma_handle->pe_disconnect_cb(wma_handle->mac_context,
					     wmi_event->vdev_id,
					     frame, wmi_event->notif_params1);
		roam_synch_data = qdf_mem_malloc(sizeof(*roam_synch_data));
		if (!roam_synch_data)
			return -ENOMEM;

		roam_synch_data->roamed_vdev_id = wmi_event->vdev_id;
		wma_handle->csr_roam_synch_cb(
				wma_handle->mac_context,
				roam_synch_data, NULL, SIR_ROAMING_DEAUTH);
		qdf_mem_free(roam_synch_data);
		break;
	default:
		WMA_LOGD("%s:Unhandled Roam Event %x for vdevid %x", __func__,
			 wmi_event->reason, wmi_event->vdev_id);
		break;
	}
	return 0;
}

#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS wma_set_gateway_params(tp_wma_handle wma,
				  struct gateway_update_req_param *req)
{
	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_set_gateway_params_cmd(wma->wmi_handle, req);
}
#endif /* FEATURE_LFR_SUBNET_DETECTION */

/**
 * wma_ht40_stop_obss_scan() - ht40 obss stop scan
 * @wma: WMA handel
 * @vdev_id: vdev identifier
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS wma_ht40_stop_obss_scan(tp_wma_handle wma, int32_t vdev_id)
{
	QDF_STATUS status;
	wmi_buf_t buf;
	wmi_obss_scan_disable_cmd_fixed_param *cmd;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	WMA_LOGD("cmd %x vdev_id %d", WMI_OBSS_SCAN_DISABLE_CMDID, vdev_id);

	cmd = (wmi_obss_scan_disable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_obss_scan_disable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_obss_scan_disable_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	status = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				      WMI_OBSS_SCAN_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

/**
 * wma_send_ht40_obss_scanind() - ht40 obss start scan indication
 * @wma: WMA handel
 * @req: start scan request
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS wma_send_ht40_obss_scanind(tp_wma_handle wma,
				struct obss_ht40_scanind *req)
{
	QDF_STATUS status;
	wmi_buf_t buf;
	wmi_obss_scan_enable_cmd_fixed_param *cmd;
	int len = 0;
	uint8_t *buf_ptr, i;
	uint8_t *channel_list;
	uint32_t *chan_freq_list;

	len += sizeof(wmi_obss_scan_enable_cmd_fixed_param);

	len += WMI_TLV_HDR_SIZE;
	len += qdf_roundup(sizeof(uint8_t) * req->channel_count,
				sizeof(uint32_t));

	len += WMI_TLV_HDR_SIZE;
	len += qdf_roundup(sizeof(uint8_t) * 1, sizeof(uint32_t));

	/* length calculation for chan_freqs */
	len += WMI_TLV_HDR_SIZE;
	len += sizeof(uint32_t) * req->channel_count;

	WMA_LOGE("cmdlen %d vdev_id %d channel count %d iefield_len %d",
			len, req->bss_id, req->channel_count, req->iefield_len);

	WMA_LOGE("scantype %d active_time %d passive %d Obss interval %d",
			req->scan_type, req->obss_active_dwelltime,
			req->obss_passive_dwelltime,
			req->obss_width_trigger_interval);

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_obss_scan_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_obss_scan_enable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_obss_scan_enable_cmd_fixed_param));

	buf_ptr = (uint8_t *) cmd;

	cmd->vdev_id = req->bss_id;
	cmd->scan_type = req->scan_type;
	cmd->obss_scan_active_dwell =
		req->obss_active_dwelltime;
	cmd->obss_scan_passive_dwell =
		req->obss_passive_dwelltime;
	cmd->bss_channel_width_trigger_scan_interval =
		req->obss_width_trigger_interval;
	cmd->bss_width_channel_transition_delay_factor =
		req->bsswidth_ch_trans_delay;
	cmd->obss_scan_active_total_per_channel =
		req->obss_active_total_per_channel;
	cmd->obss_scan_passive_total_per_channel =
		req->obss_passive_total_per_channel;
	cmd->obss_scan_activity_threshold =
		req->obss_activity_threshold;

	cmd->channel_len = req->channel_count;
	cmd->forty_mhz_intolerant =  req->fortymhz_intolerent;
	cmd->current_operating_class = req->current_operatingclass;
	cmd->ie_len = req->iefield_len;

	buf_ptr += sizeof(wmi_obss_scan_enable_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		qdf_roundup(req->channel_count, sizeof(uint32_t)));

	buf_ptr += WMI_TLV_HDR_SIZE;
	channel_list = (uint8_t *) buf_ptr;

	for (i = 0; i < req->channel_count; i++) {
		channel_list[i] =
		  wlan_reg_freq_to_chan(wma->pdev, req->chan_freq_list[i]);
		WMA_LOGD("Ch[%d]: %d ", i, channel_list[i]);
	}

	buf_ptr += qdf_roundup(sizeof(uint8_t) * req->channel_count,
				sizeof(uint32_t));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			qdf_roundup(1, sizeof(uint32_t)));

	buf_ptr += qdf_roundup(sizeof(uint8_t) * 1, sizeof(uint32_t));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       sizeof(uint32_t) * req->channel_count);
	buf_ptr += WMI_TLV_HDR_SIZE;

	chan_freq_list = (uint32_t *)buf_ptr;
	for (i = 0; i < req->channel_count; i++) {
		chan_freq_list[i] = req->chan_freq_list[i];
		WMA_LOGD("freq[%u]: %u ", i, chan_freq_list[i]);
	}

	buf_ptr += sizeof(uint32_t) * req->channel_count;

	status = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				      WMI_OBSS_SCAN_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

int wma_handle_btm_blacklist_event(void *handle, uint8_t *cmd_param_info,
				   uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_ROAM_BLACKLIST_EVENTID_param_tlvs *param_buf;
	wmi_roam_blacklist_event_fixed_param *resp_event;
	wmi_roam_blacklist_with_timeout_tlv_param *src_list;
	struct roam_blacklist_event *dst_list;
	struct roam_blacklist_timeout *roam_blacklist;
	uint32_t num_entries, i;

	param_buf = (WMI_ROAM_BLACKLIST_EVENTID_param_tlvs *)cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid event buffer");
		return -EINVAL;
	}

	resp_event = param_buf->fixed_param;
	if (!resp_event) {
		WMA_LOGE("%s: received null event data from target", __func__);
		return -EINVAL;
	}

	if (resp_event->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: received invalid vdev_id %d",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}

	num_entries = param_buf->num_blacklist_with_timeout;
	if (num_entries == 0) {
		/* no aps to blacklist just return*/
		WMA_LOGE("%s: No APs in blacklist received", __func__);
		return 0;
	}

	if (num_entries > MAX_RSSI_AVOID_BSSID_LIST) {
		WMA_LOGE("%s: num blacklist entries:%d exceeds maximum value",
			 __func__, num_entries);
		return -EINVAL;
	}

	src_list = param_buf->blacklist_with_timeout;
	if (len < (sizeof(*resp_event) + (num_entries * sizeof(*src_list)))) {
		WMA_LOGE("%s: Invalid length:%d", __func__, len);
		return -EINVAL;
	}

	dst_list = qdf_mem_malloc(sizeof(struct roam_blacklist_event) +
				 (sizeof(struct roam_blacklist_timeout) *
				 num_entries));
	if (!dst_list)
		return -ENOMEM;

	roam_blacklist = &dst_list->roam_blacklist[0];
	for (i = 0; i < num_entries; i++) {
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&src_list->bssid,
					   roam_blacklist->bssid.bytes);
		roam_blacklist->timeout = src_list->timeout;
		roam_blacklist->received_time =
			qdf_do_div(qdf_get_monotonic_boottime(),
				   QDF_MC_TIMER_TO_MS_UNIT);
		roam_blacklist++;
		src_list++;
	}

	dst_list->num_entries = num_entries;
	wma_send_msg(wma, WMA_ROAM_BLACKLIST_MSG, (void *)dst_list, 0);
	return 0;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS wma_set_roam_triggers(tp_wma_handle wma,
				 struct roam_triggers *triggers)
{
	if (!wma_is_vdev_valid(triggers->vdev_id)) {
		WMA_LOGE("%s: vdev_id: %d is not active", __func__,
			 triggers->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_set_roam_triggers(wma->wmi_handle, triggers);
}

#else
inline QDF_STATUS
wma_set_roam_triggers(tp_wma_handle wma, struct roam_triggers *triggers)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
