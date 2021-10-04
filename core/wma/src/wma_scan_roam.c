 /*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
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
#include <cdp_txrx_ctrl.h>

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"
#include "wlan_blm_api.h"

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
#include "wlan_mgmt_txrx_utils_api.h"

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
#include <cdp_txrx_ctrl.h>
#include "wlan_blm_api.h"
#include "wlan_cm_roam_api.h"
#ifdef FEATURE_WLAN_DIAG_SUPPORT    /* FEATURE_WLAN_DIAG_SUPPORT */
#include "host_diag_core_log.h"
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
#include <../../core/src/wlan_cm_roam_i.h>
#include "wlan_cm_roam_api.h"
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
#ifdef WLAN_FEATURE_11BE
	case CH_WIDTH_320MHZ:
		return WMI_HOST_CHAN_WIDTH_320;
#endif
	default:
		return WMI_HOST_CHAN_WIDTH_20;
	}
}

#define WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ      0
#define WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ         1
#define WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ        2
#define WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ 3

#ifdef WLAN_FEATURE_11BE
static void wma_update_ch_list_11be_params(struct ch_params *ch)
{
	ch->ch_width = CH_WIDTH_320MHZ;
}
#else /* !WLAN_FEATURE_11BE */
static void wma_update_ch_list_11be_params(struct ch_params *ch)
{
	ch->ch_width = CH_WIDTH_160MHZ;
}
#endif /* WLAN_FEATURE_11BE */

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
	wma_debug("no of channels = %d", chan_list->numChan);
	chan_p = &scan_ch_param->ch_param[0];
	scan_ch_param->nallchans = chan_list->numChan;
	scan_ch_param->max_bw_support_present = true;
	wma_handle->saved_chan.num_channels = chan_list->numChan;
	wma_debug("ht %d, vht %d, vht_24 %d", chan_list->ht_en,
		  chan_list->vht_en, chan_list->vht_24_en);

	for (i = 0; i < chan_list->numChan; ++i) {
		chan_p->mhz = chan_list->chanParam[i].freq;
		chan_p->cfreq1 = chan_p->mhz;
		chan_p->cfreq2 = 0;
		wma_handle->saved_chan.ch_freq_list[i] =
					chan_list->chanParam[i].freq;

		if (chan_list->chanParam[i].dfsSet) {
			chan_p->is_chan_passive = 1;
			chan_p->dfs_set = 1;
		}

		if (chan_list->chanParam[i].nan_disabled)
			chan_p->nan_disabled = 1;

		if (chan_p->mhz < WMA_2_4_GHZ_MAX_FREQ) {
			chan_p->phy_mode = MODE_11G;
			if (chan_list->vht_en && chan_list->vht_24_en)
				chan_p->allow_vht = 1;
		} else {
			chan_p->phy_mode = MODE_11A;
			if (chan_list->vht_en &&
			    !(WLAN_REG_IS_6GHZ_CHAN_FREQ(chan_p->mhz)))
				chan_p->allow_vht = 1;
		}

		if (chan_list->ht_en &&
		    !(WLAN_REG_IS_6GHZ_CHAN_FREQ(chan_p->mhz)))
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

		wma_update_ch_list_11be_params(&ch_params);

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
		wma_err("Failed to send WMI_SCAN_CHAN_LIST_CMDID");

	qdf_mem_free(scan_ch_param);

	return qdf_status;
}

/**
 * wma_handle_disconnect_reason() - Send del sta msg to lim on receiving
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @reason: disconnection reason from fw
 *
 * Return: None
 */
static void wma_handle_disconnect_reason(tp_wma_handle wma_handle,
					 uint32_t vdev_id, uint32_t reason)
{
	tpDeleteStaContext del_sta_ctx;

	del_sta_ctx = qdf_mem_malloc(sizeof(tDeleteStaContext));
	if (!del_sta_ctx)
		return;

	del_sta_ctx->vdev_id = vdev_id;
	del_sta_ctx->reasonCode = reason;
	wma_send_msg(wma_handle, SIR_LIM_DELETE_STA_CONTEXT_IND,
		     (void *)del_sta_ctx, 0);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
cm_handle_auth_offload(struct auth_offload_event *auth_event)
{
	QDF_STATUS status;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct mac_context *mac_ctx;

	mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac_ctx || !wma) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	cds_host_diag_log_work(&wma->roam_preauth_wl,
			       WMA_ROAM_PREAUTH_WAKE_LOCK_DURATION,
			       WIFI_POWER_EVENT_WAKELOCK_WOW);

	qdf_wake_lock_timeout_acquire(&wma->roam_ho_wl,
			       WMA_ROAM_HO_WAKE_LOCK_DURATION);

	lim_sae_auth_cleanup_retry(mac_ctx, auth_event->vdev_id);

	status = wma->csr_roam_auth_event_handle_cb(mac_ctx, auth_event->vdev_id,
						    auth_event->ap_bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err_rl("Trigger pre-auth failed");
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_disconnect_reason(struct vdev_disconnect_event_data *data)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	switch (data->reason) {
	case CM_DISCONNECT_REASON_CSA_SA_QUERY_TIMEOUT:
		wma_handle_disconnect_reason(wma, data->vdev_id,
			HAL_DEL_STA_REASON_CODE_SA_QUERY_TIMEOUT);
		break;
	case CM_DISCONNECT_REASON_MOVE_TO_CELLULAR:
		wma_handle_disconnect_reason(wma, data->vdev_id,
			HAL_DEL_STA_REASON_CODE_BTM_DISASSOC_IMMINENT);
		break;
	default:
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_scan_ch_list_data(struct cm_roam_scan_ch_resp *data)
{
	struct scheduler_msg sme_msg = {0};

	sme_msg.type = eWNI_SME_GET_ROAM_SCAN_CH_LIST_EVENT;
	sme_msg.bodyptr = data;

	if (scheduler_post_message(QDF_MODULE_ID_WMA,
				   QDF_MODULE_ID_SME,
				   QDF_MODULE_ID_SME, &sme_msg)) {
		wma_err("Failed to post msg to SME");
		qdf_mem_free(sme_msg.bodyptr);
		return -EINVAL;
	}

	return QDF_STATUS_SUCCESS;
}

#endif

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

	cmd = (wmi_pdev_set_ht_ie_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_ht_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_pdev_set_ht_ie_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->ie_len = ie_params->ie_len;
	cmd->tx_streams = ie_params->nss;
	cmd->rx_streams = ie_params->nss;
	wma_debug("Setting pdev HT ie with Nss = %u", ie_params->nss);
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

	cmd = (wmi_pdev_set_vht_ie_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_vht_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
					wmi_pdev_set_vht_ie_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->ie_len = ie_params->ie_len;
	cmd->tx_streams = ie_params->nss;
	cmd->rx_streams = ie_params->nss;
	wma_debug("Setting pdev VHT ie with Nss = %u", ie_params->nss);
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_pad);
	if (ie_params->ie_len) {
		qdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
			     (uint8_t *)ie_params->ie_ptr, ie_params->ie_len);
	}

	status = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				      WMI_PDEV_SET_VHT_CAP_IE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);
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

	wma_debug("first_bcnt: %d, final_bcnt: %d", first_bcnt, final_bcnt);

	status = wma_vdev_set_param(wma_handle->wmi_handle, vdev_id,
				    WMI_VDEV_PARAM_BMISS_FIRST_BCNT,
				    first_bcnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err("wma_vdev_set_param WMI_VDEV_PARAM_BMISS_FIRST_BCNT returned Error %d",
			status);
		return status;
	}

	status = wma_vdev_set_param(wma_handle->wmi_handle, vdev_id,
				    WMI_VDEV_PARAM_BMISS_FINAL_BCNT,
				    final_bcnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err("wma_vdev_set_param WMI_VDEV_PARAM_BMISS_FINAL_BCNT returned Error %d",
			status);
		return status;
	}

	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void
wma_send_roam_preauth_status(tp_wma_handle wma_handle,
			     struct wmi_roam_auth_status_params *params)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma_handle))
		return;

	wmi_handle = wma_handle->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return;

	status = wmi_unified_send_roam_preauth_status(wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		wma_err("failed to send disconnect roam preauth status");
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
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
	uint8_t vdev_id, *bssid;
	int32_t uc_cipher, cipher_cap;

	vdev_id = roam_synch_ind_ptr->roamed_vdev_id;
	wma->interfaces[vdev_id].nss = roam_synch_ind_ptr->nss;
	/* update freq and channel width */
	wma->interfaces[vdev_id].ch_freq =
		roam_synch_ind_ptr->chan_freq;
	wma->interfaces[vdev_id].chan_width =
		roam_synch_ind_ptr->chan_width;

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
		wma_err("Failed to get bssid for vdev_%d", vdev_id);
		return;
	}

	wma_delete_sta(wma, del_sta_params);
	wma_delete_bss(wma, vdev_id);
	wma_create_peer(wma, roam_synch_ind_ptr->bssid.bytes,
			WMI_PEER_TYPE_DEFAULT, vdev_id, NULL, false);

	/* Update new peer's uc cipher */
	uc_cipher = wlan_crypto_get_param(wma->interfaces[vdev_id].vdev,
					   WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	cipher_cap = wlan_crypto_get_param(wma->interfaces[vdev_id].vdev,
					   WLAN_CRYPTO_PARAM_CIPHER_CAP);
	wma_set_peer_ucast_cipher(roam_synch_ind_ptr->bssid.bytes, uc_cipher,
				  cipher_cap);
	wma_add_bss_lfr3(wma, roam_synch_ind_ptr->add_bss_params);
	wma_add_sta(wma, add_sta_params);
	qdf_mem_copy(bssid, roam_synch_ind_ptr->bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	lim_fill_roamed_peer_twt_caps(wma->mac_context, vdev_id,
				      roam_synch_ind_ptr);
	qdf_mem_free(add_sta_params);
}

static void wma_update_phymode_on_roam(tp_wma_handle wma, uint8_t *bssid,
				       wmi_channel *chan,
				       struct wma_txrx_node *iface)
{
	enum wlan_phymode bss_phymode;
	struct wlan_channel *des_chan;
	struct wlan_channel *bss_chan;
	struct vdev_mlme_obj *vdev_mlme;
	uint8_t channel;
	struct wlan_objmgr_pdev *pdev = NULL;
	qdf_freq_t sec_ch_2g_freq = 0;
	struct ch_params ch_params = {0};

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
	if (!vdev_mlme)
		return;

	pdev = wlan_vdev_get_pdev(vdev_mlme->vdev);

	channel = wlan_reg_freq_to_chan(pdev, iface->ch_freq);
	if (chan)
		bss_phymode =
			wma_fw_to_host_phymode(WMI_GET_CHANNEL_MODE(chan));
	else
		wma_get_phy_mode_cb(iface->ch_freq,
				    iface->chan_width, &bss_phymode);

	/* Update vdev mlme channel info after roaming */
	des_chan = wlan_vdev_mlme_get_des_chan(iface->vdev);
	bss_chan = wlan_vdev_mlme_get_bss_chan(iface->vdev);
	des_chan->ch_phymode = bss_phymode;
	des_chan->ch_width = iface->chan_width;
	if (chan) {
		des_chan->ch_freq = chan->mhz;
		ch_params.ch_width = des_chan->ch_width;
		if (wlan_reg_is_24ghz_ch_freq(des_chan->ch_freq) &&
		    des_chan->ch_width == CH_WIDTH_40MHZ &&
		    chan->band_center_freq1) {
			if (des_chan->ch_freq < chan->band_center_freq1)
				sec_ch_2g_freq = des_chan->ch_freq + 20;
			else
				sec_ch_2g_freq = des_chan->ch_freq - 20;
		}
		wlan_reg_set_channel_params_for_freq(pdev, des_chan->ch_freq,
						     sec_ch_2g_freq,
						     &ch_params);
		if (ch_params.ch_width != des_chan->ch_width ||
		    ch_params.mhz_freq_seg0 != chan->band_center_freq1 ||
		    ch_params.mhz_freq_seg1 != chan->band_center_freq2)
			wma_err("ch mismatch host & fw bw (%d %d) seg0 (%d, %d) seg1 (%d, %d)",
				ch_params.ch_width, des_chan->ch_width,
				ch_params.mhz_freq_seg0,
				chan->band_center_freq1,
				ch_params.mhz_freq_seg1,
				chan->band_center_freq2);
		des_chan->ch_cfreq1 = ch_params.mhz_freq_seg0;
		des_chan->ch_cfreq2 = ch_params.mhz_freq_seg1;
		des_chan->ch_width = ch_params.ch_width;
	} else {
		wma_err("LFR3: invalid chan");
	}
	qdf_mem_copy(bss_chan, des_chan, sizeof(struct wlan_channel));

	/* Till conversion is not done in WMI we need to fill fw phy mode */
	vdev_mlme->mgmt.generic.phy_mode = wma_host_to_fw_phymode(bss_phymode);

	/* update new phymode to peer */
	wma_objmgr_set_peer_mlme_phymode(wma, bssid, bss_phymode);

	wma_debug("LFR3: new phymode %d freq %d (bw %d, %d %d)",
		  bss_phymode, des_chan->ch_freq, des_chan->ch_width,
		  des_chan->ch_cfreq1, des_chan->ch_cfreq2);
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
	wma_debug("Received Roam auth offload event for bss:"QDF_MAC_ADDR_FMT" vdev_id:%d",
		  QDF_MAC_ADDR_REF(ap_bssid.bytes), vdev_id);

	lim_sae_auth_cleanup_retry(mac_ctx, vdev_id);
	status = wma->csr_roam_auth_event_handle_cb(mac_ctx, vdev_id, ap_bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err_rl("Trigger pre-auth failed");
		return -EINVAL;
	}

	return 0;
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
		wma_err("wma handle is NULL");
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
		wma_err("wma handle is NULL");
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
		wma_err("Invalid mac/wma context");
		return -EINVAL;
	}
	if (!mac->sme.rssi_threshold_breached_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_RSSI_BREACH_EVENTID_param_tlvs *)cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid rssi breached event");
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

	wma_debug("req_id: %u vdev_id: %d curr_rssi: %d",
		 rssi.request_id, rssi.session_id, rssi.curr_rssi);
	wma_debug("curr_bssid: "QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(rssi.curr_bssid.bytes));

	mac->sme.rssi_threshold_breached_cb(mac->hdd_handle, &rssi);
	wma_debug("Invoke HDD rssi breached callback");
	return 0;
}
#endif /* FEATURE_RSSI_MONITOR */

QDF_STATUS wma_pre_chan_switch_setup(uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wma_txrx_node *intr;
	uint16_t beacon_interval_ori;
	bool restart;
	uint16_t reduced_beacon_interval;
	struct vdev_mlme_obj *mlme_obj;
	struct wlan_objmgr_vdev *vdev;

	if (!wma) {
		pe_err("wma is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	intr = &wma->interfaces[vdev_id];
	if (!intr) {
		pe_err("wma txrx node is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	vdev = intr->vdev;
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

		wma_debug("Changing beacon interval to %d",
			 reduced_beacon_interval);

		/* Add a timer to reset the beacon interval back*/
		beacon_interval_ori = mlme_obj->proto.generic.beacon_interval;
		mlme_obj->proto.generic.beacon_interval =
			reduced_beacon_interval;
		if (wma_fill_beacon_interval_reset_req(wma,
			vdev_id,
			beacon_interval_ori,
			RESET_BEACON_INTERVAL_TIMEOUT)) {

			wma_debug("Failed to fill beacon interval reset req");
		}
	}

	status = wma_vdev_pre_start(vdev_id, restart);

	return status;
}

QDF_STATUS wma_post_chan_switch_setup(uint8_t vdev_id)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wma_txrx_node *intr;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_channel *des_chan;
	cdp_config_param_type val;

	if (!wma) {
		pe_err("wma is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	intr = &wma->interfaces[vdev_id];
	if (!intr) {
		pe_err("wma txrx node is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * Record monitor mode channel here in case HW
	 * indicate RX PPDU TLV with invalid channel number.
	 */
	if (intr->type == WMI_VDEV_TYPE_MONITOR) {
		des_chan = intr->vdev->vdev_mlme.des_chan;
		val.cdp_pdev_param_monitor_chan = des_chan->ch_ieee;
		cdp_txrx_set_pdev_param(soc,
					wlan_objmgr_pdev_get_pdev_id(wma->pdev),
					CDP_MONITOR_CHANNEL, val);
		val.cdp_pdev_param_mon_freq = des_chan->ch_freq;
		cdp_txrx_set_pdev_param(soc,
					wlan_objmgr_pdev_get_pdev_id(wma->pdev),
					CDP_MONITOR_FREQUENCY, val);
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

	wma_debug("PLM Start");

	status = wmi_unified_plm_start_cmd(wma->wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma->interfaces[params->vdev_id].plm_in_progress = true;

	wma_debug("Plm start request sent successfully for vdev %d",
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
		wma_err("No active plm req found, skip plm stop req");
		return QDF_STATUS_E_FAILURE;
	}

	wma_debug("PLM Stop");

	status = wmi_unified_plm_stop_cmd(wma->wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma->interfaces[params->vdev_id].plm_in_progress = false;

	wma_debug("Plm stop request sent successfully for vdev %d",
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
		wma_err("PLM %s failed %d",
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
		wma_err("Unknown tag: %d", tag);
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
	if (wma_validate_handle(wma_handle))
		return;

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

	if (!mac)
		return -EINVAL;

	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_START_STOP_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid extscan event");
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
		wma_err("Unknown event(%d) from target", event->status);
		qdf_mem_free(extscan_ind);
		return -EINVAL;
	}
	mac->sme.ext_scan_ind_cb(mac->hdd_handle, event_type, extscan_ind);
	wma_debug("sending event to umac for requestid %u with status %d",
		 extscan_ind->request_id, extscan_ind->status);
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

	if (!mac)
		return -EINVAL;

	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_OPERATION_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid scan operation event");
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
		wma_debug("received WMI_EXTSCAN_CYCLE_STARTED_EVENT");

		if (oprn_event->num_buckets > param_buf->num_bucket_id) {
			wma_err("FW mesg num_buk %d more than TLV hdr %d",
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
		wma_debug("num_buckets %u request_id %u buckets_scanned %u",
			oprn_event->num_buckets, oprn_ind->requestId,
			oprn_ind->buckets_scanned);
		break;
	case WMI_EXTSCAN_CYCLE_COMPLETED_EVENT:
		wma_debug("received WMI_EXTSCAN_CYCLE_COMPLETED_EVENT");
		qdf_wake_lock_release(&wma->extscan_wake_lock,
				      WIFI_POWER_EVENT_WAKELOCK_EXT_SCAN);
		oprn_ind->scanEventType = WIFI_EXTSCAN_CYCLE_COMPLETED_EVENT;
		oprn_ind->status = 0;
		/* Set bucket scanned mask to zero on cycle complete */
		oprn_ind->buckets_scanned = 0;
		break;
	case WMI_EXTSCAN_BUCKET_STARTED_EVENT:
		wma_debug("received WMI_EXTSCAN_BUCKET_STARTED_EVENT");
		oprn_ind->scanEventType = WIFI_EXTSCAN_BUCKET_STARTED_EVENT;
		oprn_ind->status = 0;
		goto exit_handler;
	case WMI_EXTSCAN_THRESHOLD_NUM_SCANS:
		wma_debug("received WMI_EXTSCAN_THRESHOLD_NUM_SCANS");
		oprn_ind->scanEventType = WIFI_EXTSCAN_THRESHOLD_NUM_SCANS;
		oprn_ind->status = 0;
		break;
	case WMI_EXTSCAN_THRESHOLD_PERCENT:
		wma_debug("received WMI_EXTSCAN_THRESHOLD_PERCENT");
		oprn_ind->scanEventType = WIFI_EXTSCAN_THRESHOLD_PERCENT;
		oprn_ind->status = 0;
		break;
	default:
		wma_err("Unknown event(%d) from target", oprn_event->event);
		qdf_mem_free(oprn_ind);
		return -EINVAL;
	}
	mac->sme.ext_scan_ind_cb(mac->hdd_handle,
				eSIR_EXTSCAN_SCAN_PROGRESS_EVENT_IND, oprn_ind);
	wma_debug("sending scan progress event to hdd");
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

	if (!mac)
		return -EINVAL;

	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_TABLE_USAGE_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid table usage event");
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
	wma_debug("sending scan_res available event to hdd");
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

	if (!mac)
		return -EINVAL;

	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_CAPABILITIES_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid capabilities event");
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_cache = param_buf->extscan_cache_capabilities;
	src_hotlist = param_buf->hotlist_capabilities;
	src_change = param_buf->wlan_change_capabilities;

	if (!src_cache || !src_hotlist || !src_change) {
		wma_err("Invalid capabilities list");
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

	wma_debug("request_id: %u status: %d",
		 dest_capab->requestId, dest_capab->status);

	wma_debug("Capabilities: max_scan_buckets: %d, max_hotlist_bssids: %d, max_scan_cache_size: %d, max_ap_cache_per_scan: %d",
		 dest_capab->max_scan_buckets,
		 dest_capab->max_hotlist_bssids, dest_capab->max_scan_cache_size,
		 dest_capab->max_ap_cache_per_scan);
	wma_debug("max_scan_reporting_threshold: %d, max_rssi_sample_size: %d, max_bssid_history_entries: %d, max_significant_wifi_change_aps: %d",
		 dest_capab->max_scan_reporting_threshold,
		 dest_capab->max_rssi_sample_size,
		 dest_capab->max_bssid_history_entries,
		 dest_capab->max_significant_wifi_change_aps);

	wma_debug("Capabilities: max_hotlist_ssids: %d, max_number_epno_networks: %d, max_number_epno_networks_by_ssid: %d",
		 dest_capab->max_hotlist_ssids,
		 dest_capab->max_number_epno_networks,
		 dest_capab->max_number_epno_networks_by_ssid);
	wma_debug("max_number_of_white_listed_ssid: %d, max_number_of_black_listed_bssid: %d",
		 dest_capab->max_number_of_white_listed_ssid,
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

	if (!mac)
		return -EINVAL;

	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_HOTLIST_MATCH_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid hotlist match event");
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_hotlist = param_buf->hotlist_match;
	numap = event->total_entries;

	if (!src_hotlist || !numap) {
		wma_err("Hotlist AP's list invalid");
		return -EINVAL;
	}
	if (numap > param_buf->num_hotlist_match) {
		wma_err("Invalid no of total enteries %d", numap);
		return -EINVAL;
	}
	if (numap > WMA_EXTSCAN_MAX_HOTLIST_ENTRIES) {
		wma_err("Total Entries %u greater than max", numap);
		numap = WMA_EXTSCAN_MAX_HOTLIST_ENTRIES;
	}

	buf_len = sizeof(wmi_extscan_hotlist_match_event_fixed_param) +
		  WMI_TLV_HDR_SIZE +
		  (numap * sizeof(wmi_extscan_wlan_descriptor));

	if (buf_len > len) {
		wma_err("Invalid buf len from FW %d numap %d", len, numap);
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

	wma_debug("Hotlist match: requestId: %u numOfAps: %d",
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
			wma_err("Invalid SSID len %d, truncating",
				src_hotlist->ssid.ssid_len);
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
	wma_debug("sending hotlist match event to hdd");
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
		wma_err("num_scan_ids %d, num_results %d num_bssid_list %d",
			 t_cached_result->num_scan_ids,
			 t_scan_id_grp->num_results,
			 param_buf->num_bssid_list);
		return -EINVAL;
	}

	wma_debug("num_scan_ids:%d",
			t_cached_result->num_scan_ids);
	for (i = 0; i < t_cached_result->num_scan_ids; i++) {
		wma_debug("num_results:%d", t_scan_id_grp->num_results);
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
				wma_debug("Invalid SSID len %d, truncating",
					 src_hotlist->ssid.ssid_len);
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
		wma_err("Invalid mac");
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_CACHED_RESULTS_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid cached results event");
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_hotlist = param_buf->bssid_list;
	src_rssi = param_buf->rssi_list;
	wma_debug("Total_entries: %u first_entry_index: %u num_entries_in_page: %d",
		 event->total_entries,
		 event->first_entry_index,
		 event->num_entries_in_page);

	if (!src_hotlist || !src_rssi || !event->num_entries_in_page) {
		wma_warn("Cached results empty, send 0 results");
		goto noresults;
	}

	if (event->num_entries_in_page >
	    (WMI_SVC_MSG_MAX_SIZE - sizeof(*event))/sizeof(*src_hotlist) ||
	    event->num_entries_in_page > param_buf->num_bssid_list) {
		wma_err("excess num_entries_in_page %d in WMI event. num_bssid_list %d",
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
			wma_debug("total len IE: %d", total_len);
		}

		if (src_hotlist[i].number_rssi_samples >
		    (WMI_SVC_MSG_MAX_SIZE - total_len) / sizeof(*src_rssi)) {
			excess_data = true;
			break;
		} else {
			total_len += (src_hotlist[i].number_rssi_samples *
					sizeof(*src_rssi));
			wma_debug("total len RSSI samples: %d", total_len);
		}
	}
	if (excess_data) {
		wma_err("excess data in WMI event");
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
	wma_debug("scan_ids_cnt %d", scan_ids_cnt);
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
		wma_debug("wma_group_num_bss_to_scan_id failed, not calling callback");

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
		wma_err("Invalid mac");
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}
	param_buf = (WMI_EXTSCAN_WLAN_CHANGE_RESULTS_EVENTID_param_tlvs *)
		    cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid change monitor event");
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	src_chglist = param_buf->bssid_signal_descriptor_list;
	src_rssi = param_buf->rssi_list;
	numap = event->num_entries_in_page;

	if (!src_chglist || !numap) {
		wma_err("Results invalid");
		return -EINVAL;
	}
	if (numap > param_buf->num_bssid_signal_descriptor_list) {
		wma_err("Invalid num of entries in page: %d", numap);
		return -EINVAL;
	}
	for (i = 0; i < numap; i++) {
		if (src_chglist->num_rssi_samples > (UINT_MAX - rssi_num)) {
			wma_err("Invalid num of rssi samples %d numap %d rssi_num %d",
				 src_chglist->num_rssi_samples,
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
		wma_err("buffer len exceeds WMI payload,numap:%d, rssi_num:%d",
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
				wma_err("Invalid num in rssi list: %d",
					dest_ap->numOfRssi);
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
	wma_debug("sending change monitor results");
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
		wma_err("Invalid mac");
		return -EINVAL;
	}
	if (!mac->sme.ext_scan_ind_cb) {
		wma_err("Callback not registered");
		return -EINVAL;
	}

	param_buf = (WMI_PASSPOINT_MATCH_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		wma_err("Invalid passpoint match event");
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
		wma_err("IE Length: %u or ANQP Length: %u is huge, num_bufp: %u",
			event->ie_length, event->anqp_length,
			param_buf->num_bufp);
		return -EINVAL;
	}

	if (event->ssid.ssid_len > WLAN_SSID_MAX_LEN) {
		wma_debug("Invalid ssid len %d, truncating",
			 event->ssid.ssid_len);
		event->ssid.ssid_len = WLAN_SSID_MAX_LEN;
	}

	dest_match = qdf_mem_malloc(sizeof(*dest_match) + buf_len);
	if (!dest_match)
		return -EINVAL;

	dest_ap = &dest_match->ap;
	dest_match->request_id = 0;
	dest_match->id = event->id;
	dest_match->anqp_len = event->anqp_length;
	wma_info("passpoint match: id: %u anqp length %u",
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
	wma_debug("sending passpoint match event to hdd");
	qdf_mem_free(dest_match);
	return 0;
}

QDF_STATUS wma_start_extscan(tp_wma_handle wma,
			     struct wifi_scan_cmd_req_params *params)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	if (!params) {
		wma_err("NULL param");
		return QDF_STATUS_E_NOMEM;
	}

	status = wmi_unified_start_extscan_cmd(wmi_handle, params);
	if (QDF_IS_STATUS_SUCCESS(status))
		wma->interfaces[params->vdev_id].extscan_in_progress = true;

	wma_debug("Exit, vdev %d, status %d", params->vdev_id, status);

	return status;
}

QDF_STATUS wma_stop_extscan(tp_wma_handle wma,
			    struct extscan_stop_req_params *params)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	status = wmi_unified_stop_extscan_cmd(wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	wma->interfaces[params->vdev_id].extscan_in_progress = false;
	wma_debug("Extscan stop request sent successfully for vdev %d",
		 params->vdev_id);

	return status;
}

QDF_STATUS wma_extscan_start_hotlist_monitor(tp_wma_handle wma,
			struct extscan_bssid_hotlist_set_params *params)
{
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!params) {
		wma_err("Invalid params");
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_extscan_start_hotlist_monitor_cmd(wmi_handle,
							     params);
}

QDF_STATUS wma_extscan_stop_hotlist_monitor(tp_wma_handle wma,
		    struct extscan_bssid_hotlist_reset_params *params)
{
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!params) {
		wma_err("Invalid params");
		return QDF_STATUS_E_INVAL;
	}
	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_stop_hotlist_monitor_cmd(wmi_handle,
							    params);
}

QDF_STATUS
wma_extscan_start_change_monitor(tp_wma_handle wma,
			struct extscan_set_sig_changereq_params *params)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!params) {
		wma_err("NULL params");
		return QDF_STATUS_E_NOMEM;
	}

	status = wmi_unified_extscan_start_change_monitor_cmd(wmi_handle,
							      params);
	return status;
}

QDF_STATUS wma_extscan_stop_change_monitor(tp_wma_handle wma,
			struct extscan_capabilities_reset_params *params)
{
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("ext scan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_stop_change_monitor_cmd(wmi_handle,
							   params);
}

QDF_STATUS
wma_extscan_get_cached_results(tp_wma_handle wma,
			       struct extscan_cached_result_params *params)
{
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_get_cached_results_cmd(wmi_handle,
							  params);
}

QDF_STATUS
wma_extscan_get_capabilities(tp_wma_handle wma,
			     struct extscan_capabilities_params *params)
{
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_extscan_get_capabilities_cmd(wmi_handle,
							params);
}

QDF_STATUS wma_set_epno_network_list(tp_wma_handle wma,
				     struct wifi_enhanced_pno_params *req)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	wma_debug("Enter");

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_FAILURE;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_FAILURE;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = wmi_unified_set_epno_network_list_cmd(wmi_handle, req);
	wma_debug("Exit, vdev %d, status %d", req->vdev_id, status);

	return status;
}

QDF_STATUS
wma_set_passpoint_network_list(tp_wma_handle wma,
			       struct wifi_passpoint_req_param *params)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	wma_debug("Enter");

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_FAILURE;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_FAILURE;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = wmi_unified_set_passpoint_network_list_cmd(wmi_handle,
							    params);
	wma_debug("Exit, vdev %d, status %d", params->vdev_id, status);

	return status;
}

QDF_STATUS
wma_reset_passpoint_network_list(tp_wma_handle wma,
				 struct wifi_passpoint_req_param *params)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	wma_debug("Enter");

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_FAILURE;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_FAILURE;

	if (!wmi_service_enabled(wmi_handle, wmi_service_extscan)) {
		wma_err("extscan not enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	status = wmi_unified_reset_passpoint_network_list_cmd(wmi_handle,
							      params);
	wma_debug("Exit, vdev %d, status %d", params->vdev_id, status);

	return status;
}

#endif

QDF_STATUS wma_scan_probe_setoui(tp_wma_handle wma,
				 struct scan_mac_oui *set_oui)
{
	struct wmi_unified *wmi_handle;

	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

	wmi_handle = wma->wmi_handle;
	if (wmi_validate_handle(wmi_handle))
		return QDF_STATUS_E_INVAL;

	if (!wma_is_vdev_valid(set_oui->vdev_id)) {
		wma_err("vdev_id: %d is not active", set_oui->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	return wmi_unified_scan_probe_setoui_cmd(wmi_handle, set_oui);
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
	struct scheduler_msg msg = {0};
	QDF_STATUS status;
	struct cm_host_roam_start_ind *ind;

	ind = qdf_mem_malloc(sizeof(*ind));
	if (!ind)
		return;

	ind->pdev = wma->pdev;
	ind->vdev_id = vdev_id;
	msg.bodyptr = ind;
	msg.callback = wlan_cm_host_roam_start;
	wma_debug("Posting ROam start ind to connection manager, vdev %d",
		  vdev_id);
	status = scheduler_post_message(QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_OS_IF,
					QDF_MODULE_ID_SCAN, &msg);

	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(msg.bodyptr);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
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
					    enum cm_roam_notif notif)
{
	struct roam_offload_synch_ind *roam_synch_data;
	enum sir_roam_op_code op_code;

	if (notif == CM_ROAM_NOTIF_ROAM_START) {
		op_code = SIR_ROAMING_START;
	} else if (notif == CM_ROAM_NOTIF_ROAM_ABORT) {
		op_code = SIR_ROAMING_ABORT;
		lim_sae_auth_cleanup_retry(wma_handle->mac_context, vdev_id);
	} else {
		wma_debug("Invalid notif %d", notif);
		return;
	}
	roam_synch_data = qdf_mem_malloc(sizeof(*roam_synch_data));
	if (!roam_synch_data)
		return;

	roam_synch_data->roamed_vdev_id = vdev_id;
	if (notif != CM_ROAM_NOTIF_ROAM_START)
		wma_handle->pe_roam_synch_cb(wma_handle->mac_context,
					     roam_synch_data, 0, op_code);

	if (notif == CM_ROAM_NOTIF_ROAM_START)
		cm_fw_roam_start_req(wma_handle->psoc, vdev_id);
	else
		cm_fw_roam_abort_req(wma_handle->psoc, vdev_id);

	qdf_mem_free(roam_synch_data);
}

void wma_handle_roam_sync_timeout(tp_wma_handle wma_handle,
				  struct roam_sync_timeout_timer_info *info)
{
	wma_invalid_roam_reason_handler(wma_handle, info->vdev_id,
					CM_ROAM_NOTIF_ROAM_ABORT);
}

void cm_invalid_roam_reason_handler(uint32_t vdev_id, enum cm_roam_notif notif)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	wma_invalid_roam_reason_handler(wma_handle, vdev_id, notif);
}
#endif

static void
wma_handle_roam_reason_invoke_roam_fail(tp_wma_handle wma_handle,
					uint8_t vdev_id, uint32_t notif_params)
{
	struct roam_offload_synch_ind *roam_synch_data;

	roam_synch_data = qdf_mem_malloc(sizeof(*roam_synch_data));
	if (!roam_synch_data)
		return;

	lim_sae_auth_cleanup_retry(wma_handle->mac_context, vdev_id);
	roam_synch_data->roamed_vdev_id = vdev_id;
	cm_fw_roam_invoke_fail(wma_handle->psoc, vdev_id);
	wlan_cm_update_roam_states(wma_handle->psoc, vdev_id,
				   notif_params,
				   ROAM_INVOKE_FAIL_REASON);
	qdf_mem_free(roam_synch_data);
}

static void wma_handle_roam_reason_btm(uint8_t vdev_id)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	/*
	 * This event is received from firmware if firmware is unable to
	 * find candidate AP after roam scan and BTM request from AP
	 * has disassoc imminent bit set.
	 */
	wma_debug("Kickout due to btm request");
	wma_sta_kickout_event(HOST_STA_KICKOUT_REASON_BTM, vdev_id, NULL);
	wma_handle_disconnect_reason(wma_handle, vdev_id,
			HAL_DEL_STA_REASON_CODE_BTM_DISASSOC_IMMINENT);
}

static void wma_handle_roam_reason_bmiss(uint8_t vdev_id, uint32_t rssi)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	/*
	 * WMI_ROAM_REASON_BMISS can get called in soft IRQ context, so
	 * avoid using CSR/PE structure directly
	 */
	wma_debug("Beacon Miss for vdevid %x", vdev_id);
	wma_beacon_miss_handler(wma_handle, vdev_id, rssi);
	wma_sta_kickout_event(HOST_STA_KICKOUT_REASON_BMISS, vdev_id, NULL);
}

static void wma_handle_roam_reason_better_ap(uint8_t vdev_id, uint32_t rssi)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	/*
	 * WMI_ROAM_REASON_BETTER_AP can get called in soft IRQ context,
	 * so avoid using CSR/PE structure directly.
	 */
	wma_debug("Better AP found for vdevid %x, rssi %d", vdev_id, rssi);
	mlme_set_roam_reason_better_ap(wma_handle->interfaces[vdev_id].vdev,
				       false);
	wma_roam_better_ap_handler(wma_handle, vdev_id);
}

static void wma_handle_roam_reason_suitable_ap(uint8_t vdev_id, uint32_t rssi)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	/*
	 * WMI_ROAM_REASON_SUITABLE_AP can get called in soft IRQ
	 * context, so avoid using CSR/PE structure directly.
	 */
	mlme_set_roam_reason_better_ap(wma_handle->interfaces[vdev_id].vdev,
				       true);
	mlme_set_hb_ap_rssi(wma_handle->interfaces[vdev_id].vdev, rssi);
	wma_debug("Bmiss scan AP found for vdevid %x, rssi %d", vdev_id, rssi);
	wma_roam_better_ap_handler(wma_handle, vdev_id);
}

static void
wma_update_pdev_hw_mode_trans_ind(tp_wma_handle wma,
				  struct cm_hw_mode_trans_ind *trans_ind)
{
	uint32_t i;

	/* Store the vdev-mac map in WMA and send to policy manager */
	for (i = 0; i < trans_ind->num_vdev_mac_entries; i++)
		wma_update_intf_hw_mode_params(
				trans_ind->vdev_mac_map[i].vdev_id,
				trans_ind->vdev_mac_map[i].mac_id,
				trans_ind->new_hw_mode_index);

	wma->old_hw_mode_index = trans_ind->old_hw_mode_index;
	wma->new_hw_mode_index = trans_ind->new_hw_mode_index;
	policy_mgr_update_new_hw_mode_index(wma->psoc,
					    trans_ind->new_hw_mode_index);
	policy_mgr_update_old_hw_mode_index(wma->psoc,
					    trans_ind->old_hw_mode_index);

	wma_debug("Updated: old_hw_mode_index:%d new_hw_mode_index:%d",
		  wma->old_hw_mode_index, wma->new_hw_mode_index);
}

static void
wma_handle_hw_mode_trans_ind(tp_wma_handle wma_handle,
			     struct cm_hw_mode_trans_ind *hw_mode_trans_ind)
{
	struct scheduler_msg sme_msg = {0};
	QDF_STATUS status;

	if (hw_mode_trans_ind) {
		wma_update_pdev_hw_mode_trans_ind(wma_handle,
						  hw_mode_trans_ind);
		wma_debug("Update HW mode");
		sme_msg.type = eWNI_SME_HW_MODE_TRANS_IND;
		sme_msg.bodyptr = hw_mode_trans_ind;

		status = scheduler_post_message(QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_SME,
						QDF_MODULE_ID_SME, &sme_msg);
		if (QDF_IS_STATUS_ERROR(status))
			qdf_mem_free(hw_mode_trans_ind);
	} else {
		wma_debug("hw_mode transition fixed param is NULL");
	}
}

int cm_rso_cmd_status_event_handler(uint8_t vdev_id, enum cm_roam_notif notif)
{
	return wma_rso_cmd_status_event_handler(vdev_id, notif);
}

void
cm_handle_roam_reason_invoke_roam_fail(uint8_t vdev_id,	uint32_t notif_params,
				       struct cm_hw_mode_trans_ind *trans_ind)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	wma_handle_hw_mode_trans_ind(wma_handle, trans_ind);
	wma_handle_roam_reason_invoke_roam_fail(wma_handle, vdev_id,
						notif_params);
}

static void
wma_handle_roam_reason_deauth(uint8_t vdev_id, uint32_t notif_params,
			      uint32_t notif_params1,
			      uint8_t *deauth_disassoc_frame)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	struct roam_offload_synch_ind *roam_synch_data;

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	wma_debug("Received disconnect roam event reason:%d", notif_params);
	wma_handle->pe_disconnect_cb(wma_handle->mac_context,
				     vdev_id,
				     deauth_disassoc_frame, notif_params1,
				     notif_params);
	roam_synch_data = qdf_mem_malloc(sizeof(*roam_synch_data));
	if (!roam_synch_data)
		return;

	roam_synch_data->roamed_vdev_id = vdev_id;
	qdf_mem_free(roam_synch_data);
}

void cm_handle_roam_reason_deauth(uint8_t vdev_id, uint32_t notif_params,
				  uint8_t *deauth_disassoc_frame,
				  uint32_t frame_len)
{
	wma_handle_roam_reason_deauth(vdev_id, notif_params, frame_len,
				      deauth_disassoc_frame);
}

void cm_handle_roam_reason_btm(uint8_t vdev_id)
{
	wma_handle_roam_reason_btm(vdev_id);
}

void cm_handle_roam_reason_bmiss(uint8_t vdev_id, uint32_t rssi)
{
	wma_handle_roam_reason_bmiss(vdev_id, rssi);
}

void cm_handle_roam_reason_better_ap(uint8_t vdev_id, uint32_t rssi)
{
	wma_handle_roam_reason_better_ap(vdev_id, rssi);
}

void cm_handle_roam_reason_suitable_ap(uint8_t vdev_id, uint32_t rssi)
{
	wma_handle_roam_reason_suitable_ap(vdev_id, rssi);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void
wma_handle_roam_reason_ho_failed(uint8_t vdev_id, struct qdf_mac_addr bssid,
				 struct cm_hw_mode_trans_ind *hw_mode_trans_ind)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		QDF_ASSERT(0);
		return;
	}
	/*
	 * WMI_ROAM_REASON_HO_FAILED can get called in soft IRQ context,
	 * so avoid using CSR/PE structure directly.
	 */
	wma_err("LFR3:Hand-Off Failed for vdevid %x", vdev_id);
	wma_debug("mac addr to avoid " QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(bssid.bytes));
	wma_handle_hw_mode_trans_ind(wma_handle, hw_mode_trans_ind);
	cm_fw_ho_fail_req(wma_handle->psoc, vdev_id, bssid);
	lim_sae_auth_cleanup_retry(wma_handle->mac_context, vdev_id);
}

void
cm_handle_roam_reason_ho_failed(uint8_t vdev_id, struct qdf_mac_addr bssid,
				struct cm_hw_mode_trans_ind *hw_mode_trans_ind)
{
	wma_handle_roam_reason_ho_failed(vdev_id, bssid, hw_mode_trans_ind);
}
#endif

#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS wma_set_gateway_params(tp_wma_handle wma,
				  struct gateway_update_req_param *req)
{
	if (wma_validate_handle(wma))
		return QDF_STATUS_E_INVAL;

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

	wma_debug("cmd %x vdev_id %d", WMI_OBSS_SCAN_DISABLE_CMDID, vdev_id);

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

	wma_debug("cmdlen %d vdev_id %d channel count %d iefield_len %d",
		 len, req->bss_id, req->channel_count, req->iefield_len);

	wma_debug("scantype %d active_time %d passive %d Obss interval %d",
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
		wma_nofl_debug("Ch[%d]: %d ", i, channel_list[i]);
	}

	buf_ptr += qdf_roundup(sizeof(uint8_t) * req->channel_count,
				sizeof(uint32_t));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			qdf_roundup(1, sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	buf_ptr += qdf_roundup(sizeof(uint8_t) * 1, sizeof(uint32_t));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       sizeof(uint32_t) * req->channel_count);
	buf_ptr += WMI_TLV_HDR_SIZE;

	chan_freq_list = (uint32_t *)buf_ptr;
	for (i = 0; i < req->channel_count; i++) {
		chan_freq_list[i] = req->chan_freq_list[i];
		wma_nofl_debug("freq[%u]: %u ", i, chan_freq_list[i]);
	}

	buf_ptr += sizeof(uint32_t) * req->channel_count;

	status = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				      WMI_OBSS_SCAN_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void cm_roam_update_vdev(struct roam_offload_synch_ind *sync_ind)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma)
		return;

	wma_roam_update_vdev(wma, sync_ind);
}

QDF_STATUS
cm_roam_pe_sync_callback(struct roam_offload_synch_ind *sync_ind,
			 uint16_t ie_len)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	QDF_STATUS status;

	if (!wma)
		return QDF_STATUS_E_INVAL;

	status = wma->pe_roam_synch_cb(wma->mac_context,
				sync_ind, ie_len,
				SIR_ROAM_SYNCH_PROPAGATION);

	return status;
}

void cm_update_phymode_on_roam(uint8_t vdev_id, uint8_t *bssid,
			       wmi_channel *chan)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma)
		return;

	wma_update_phymode_on_roam(wma, bssid, chan, &wma->interfaces[vdev_id]);
}

enum wlan_phymode
wlan_cm_fw_to_host_phymode(WMI_HOST_WLAN_PHY_MODE phymode)
{
	return wma_fw_to_host_phymode(phymode);
}
#endif
