/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 *  DOC:  wma_mgmt.c
 *
 *  This file contains STA/SAP/IBSS and protocol related functions.
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
#include "cfg_api.h"
#include "ol_txrx_ctrl_api.h"
#include "wlan_tgt_def_config.h"

#include "cdf_nbuf.h"
#include "qdf_types.h"
#include "ol_txrx_api.h"
#include "cdf_memory.h"
#include "ol_txrx_types.h"
#include "ol_txrx_peer_find.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "csr_api.h"
#include "ol_fw.h"
#include "dfs.h"
#include "wma_internal.h"
#include "cds_concurrency.h"

/**
 * wma_send_bcn_buf_ll() - prepare and send beacon buffer to fw for LL
 * @wma: wma handle
 * @pdev: txrx pdev
 * @vdev_id: vdev id
 * @param_buf: SWBA parameters
 *
 * Return: none
 */
static void wma_send_bcn_buf_ll(tp_wma_handle wma,
				ol_txrx_pdev_handle pdev,
				uint8_t vdev_id,
				WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf)
{
	wmi_bcn_send_from_host_cmd_fixed_param *cmd;
	struct ieee80211_frame *wh;
	struct beacon_info *bcn;
	wmi_tim_info *tim_info = param_buf->tim_info;
	uint8_t *bcn_payload;
	wmi_buf_t wmi_buf;
	QDF_STATUS ret;
	struct beacon_tim_ie *tim_ie;
	wmi_p2p_noa_info *p2p_noa_info = param_buf->p2p_noa_info;
	struct p2p_sub_element_noa noa_ie;
	uint8_t i;
	int status;

	bcn = wma->interfaces[vdev_id].beacon;
	if (!bcn->buf) {
		WMA_LOGE("%s: Invalid beacon buffer", __func__);
		return;
	}

	wmi_buf = wmi_buf_alloc(wma->wmi_handle, sizeof(*cmd));
	if (!wmi_buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return;
	}

	cdf_spin_lock_bh(&bcn->lock);

	bcn_payload = cdf_nbuf_data(bcn->buf);

	tim_ie = (struct beacon_tim_ie *)(&bcn_payload[bcn->tim_ie_offset]);

	if (tim_info->tim_changed) {
		if (tim_info->tim_num_ps_pending)
			cdf_mem_copy(&tim_ie->tim_bitmap, tim_info->tim_bitmap,
				     WMA_TIM_SUPPORTED_PVB_LENGTH);
		else
			cdf_mem_zero(&tim_ie->tim_bitmap,
				     WMA_TIM_SUPPORTED_PVB_LENGTH);
		/*
		 * Currently we support fixed number of
		 * peers as limited by HAL_NUM_STA.
		 * tim offset is always 0
		 */
		tim_ie->tim_bitctl = 0;
	}

	/* Update DTIM Count */
	if (tim_ie->dtim_count == 0)
		tim_ie->dtim_count = tim_ie->dtim_period - 1;
	else
		tim_ie->dtim_count--;

	/*
	 * DTIM count needs to be backedup so that
	 * when umac updates the beacon template
	 * current dtim count can be updated properly
	 */
	bcn->dtim_count = tim_ie->dtim_count;

	/* update state for buffered multicast frames on DTIM */
	if (tim_info->tim_mcast && (tim_ie->dtim_count == 0 ||
				    tim_ie->dtim_period == 1))
		tim_ie->tim_bitctl |= 1;
	else
		tim_ie->tim_bitctl &= ~1;

	/* To avoid sw generated frame sequence the same as H/W generated frame,
	 * the value lower than min_sw_seq is reserved for HW generated frame */
	if ((bcn->seq_no & IEEE80211_SEQ_MASK) < MIN_SW_SEQ)
		bcn->seq_no = MIN_SW_SEQ;

	wh = (struct ieee80211_frame *)bcn_payload;
	*(uint16_t *) &wh->i_seq[0] = htole16(bcn->seq_no
					      << IEEE80211_SEQ_SEQ_SHIFT);
	bcn->seq_no++;

	if (WMI_UNIFIED_NOA_ATTR_IS_MODIFIED(p2p_noa_info)) {
		cdf_mem_zero(&noa_ie, sizeof(noa_ie));

		noa_ie.index =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_INDEX_GET(p2p_noa_info);
		noa_ie.oppPS =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_OPP_PS_GET(p2p_noa_info);
		noa_ie.ctwindow =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_CTWIN_GET(p2p_noa_info);
		noa_ie.num_descriptors =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(p2p_noa_info);
		WMA_LOGI("%s: index %u, oppPs %u, ctwindow %u, "
			 "num_descriptors = %u", __func__, noa_ie.index,
			 noa_ie.oppPS, noa_ie.ctwindow, noa_ie.num_descriptors);
		for (i = 0; i < noa_ie.num_descriptors; i++) {
			noa_ie.noa_descriptors[i].type_count =
				(uint8_t) p2p_noa_info->noa_descriptors[i].
				type_count;
			noa_ie.noa_descriptors[i].duration =
				p2p_noa_info->noa_descriptors[i].duration;
			noa_ie.noa_descriptors[i].interval =
				p2p_noa_info->noa_descriptors[i].interval;
			noa_ie.noa_descriptors[i].start_time =
				p2p_noa_info->noa_descriptors[i].start_time;
			WMA_LOGI("%s: NoA descriptor[%d] type_count %u, "
				 "duration %u, interval %u, start_time = %u",
				 __func__, i,
				 noa_ie.noa_descriptors[i].type_count,
				 noa_ie.noa_descriptors[i].duration,
				 noa_ie.noa_descriptors[i].interval,
				 noa_ie.noa_descriptors[i].start_time);
		}
		wma_update_noa(bcn, &noa_ie);

		/* Send a msg to LIM to update the NoA IE in probe response
		 * frames transmitted by the host */
		wma_update_probe_resp_noa(wma, &noa_ie);
	}

	if (bcn->dma_mapped) {
		cdf_nbuf_unmap_single(pdev->osdev, bcn->buf, QDF_DMA_TO_DEVICE);
		bcn->dma_mapped = 0;
	}
	ret = cdf_nbuf_map_single(pdev->osdev, bcn->buf, QDF_DMA_TO_DEVICE);
	if (ret != QDF_STATUS_SUCCESS) {
		cdf_nbuf_free(wmi_buf);
		WMA_LOGE("%s: failed map beacon buf to DMA region", __func__);
		cdf_spin_unlock_bh(&bcn->lock);
		return;
	}

	bcn->dma_mapped = 1;
	cmd = (wmi_bcn_send_from_host_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_send_from_host_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_bcn_send_from_host_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->data_len = bcn->len;
	cmd->frame_ctrl = *((A_UINT16 *) wh->i_fc);
	cmd->frag_ptr = cdf_nbuf_get_frag_paddr(bcn->buf, 0);

	/* notify Firmware of DTM and mcast/bcast traffic */
	if (tim_ie->dtim_count == 0) {
		cmd->dtim_flag |= WMI_BCN_SEND_DTIM_ZERO;
		/* deliver mcast/bcast traffic in next DTIM beacon */
		if (tim_ie->tim_bitctl & 0x01)
			cmd->dtim_flag |= WMI_BCN_SEND_DTIM_BITCTL_SET;
	}

	status = wmi_unified_cmd_send(wma->wmi_handle, wmi_buf, sizeof(*cmd),
				      WMI_PDEV_SEND_BCN_CMDID);

	if (status != EOK) {
		WMA_LOGE("Failed to send WMI_PDEV_SEND_BCN_CMDID command");
		wmi_buf_free(wmi_buf);
	}
	cdf_spin_unlock_bh(&bcn->lock);
}

/**
 * wma_beacon_swba_handler() - swba event handler
 * @handle: wma handle
 * @event: event data
 * @len: data length
 *
 * SWBA event is alert event to Host requesting host to Queue a beacon
 * for transmission use only in host beacon mode
 *
 * Return: 0 for success or error code
 */
int wma_beacon_swba_handler(void *handle, uint8_t *event, uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf;
	wmi_host_swba_event_fixed_param *swba_event;
	uint32_t vdev_map;
	ol_txrx_pdev_handle pdev;
	uint8_t vdev_id = 0;

	param_buf = (WMI_HOST_SWBA_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid swba event buffer");
		return -EINVAL;
	}
	swba_event = param_buf->fixed_param;
	vdev_map = swba_event->vdev_map;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is NULL", __func__);
		return -EINVAL;
	}

	for (; vdev_map; vdev_id++, vdev_map >>= 1) {
		if (!(vdev_map & 0x1))
			continue;
		if (!ol_cfg_is_high_latency(pdev->ctrl_pdev))
			wma_send_bcn_buf_ll(wma, pdev, vdev_id, param_buf);
		break;
	}
	return 0;
}

/**
 * wma_peer_sta_kickout_event_handler() - kickout event handler
 * @handle: wma handle
 * @event: event data
 * @len: data length
 *
 * Kickout event is received from firmware on observing beacon miss
 * It handles kickout event for different modes and indicate to
 * upper layers.
 *
 * Return: 0 for success or error code
 */
int wma_peer_sta_kickout_event_handler(void *handle, u8 *event, u32 len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_PEER_STA_KICKOUT_EVENTID_param_tlvs *param_buf = NULL;
	wmi_peer_sta_kickout_event_fixed_param *kickout_event = NULL;
	uint8_t vdev_id, peer_id, macaddr[IEEE80211_ADDR_LEN];
	ol_txrx_peer_handle peer;
	ol_txrx_pdev_handle pdev;
	tpDeleteStaContext del_sta_ctx;
	tpSirIbssPeerInactivityInd p_inactivity;

	WMA_LOGD("%s: Enter", __func__);
	param_buf = (WMI_PEER_STA_KICKOUT_EVENTID_param_tlvs *) event;
	kickout_event = param_buf->fixed_param;
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is NULL", __func__);
		return -EINVAL;
	}
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&kickout_event->peer_macaddr, macaddr);
	peer = ol_txrx_find_peer_by_addr(pdev, macaddr, &peer_id);
	if (!peer) {
		WMA_LOGE("PEER [%pM] not found", macaddr);
		return -EINVAL;
	}

	if (ol_txrx_get_vdevid(peer, &vdev_id) != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Not able to find BSSID for peer [%pM]", macaddr);
		return -EINVAL;
	}

	WMA_LOGA("%s: PEER:[%pM], ADDR:[%pN], INTERFACE:%d, peer_id:%d, reason:%d",
		__func__, macaddr, wma->interfaces[vdev_id].addr, vdev_id,
		 peer_id, kickout_event->reason);

	switch (kickout_event->reason) {
	case WMI_PEER_STA_KICKOUT_REASON_IBSS_DISCONNECT:
		p_inactivity = (tpSirIbssPeerInactivityInd)
			       cdf_mem_malloc(sizeof(tSirIbssPeerInactivityInd));
		if (!p_inactivity) {
			WMA_LOGE("CDF MEM Alloc Failed for tSirIbssPeerInactivity");
			return -ENOMEM;
		}

		p_inactivity->staIdx = peer_id;
		cdf_mem_copy(p_inactivity->peer_addr.bytes, macaddr,
			     IEEE80211_ADDR_LEN);
		wma_send_msg(wma, WMA_IBSS_PEER_INACTIVITY_IND,
			     (void *)p_inactivity, 0);
		goto exit_handler;
		break;

#ifdef FEATURE_WLAN_TDLS
	case WMI_PEER_STA_KICKOUT_REASON_TDLS_DISCONNECT:
		del_sta_ctx = (tpDeleteStaContext)
			cdf_mem_malloc(sizeof(tDeleteStaContext));
		if (!del_sta_ctx) {
			WMA_LOGE("%s: mem alloc failed for tDeleteStaContext for TDLS peer: %pM",
				__func__, macaddr);
			return -ENOMEM;
		}

		del_sta_ctx->staId = peer_id;
		cdf_mem_copy(del_sta_ctx->addr2, macaddr, IEEE80211_ADDR_LEN);
		cdf_mem_copy(del_sta_ctx->bssId, wma->interfaces[vdev_id].bssid,
			     IEEE80211_ADDR_LEN);
		del_sta_ctx->reasonCode = HAL_DEL_STA_REASON_CODE_KEEP_ALIVE;
		wma_send_msg(wma, SIR_LIM_DELETE_STA_CONTEXT_IND,
			     (void *)del_sta_ctx, 0);
		goto exit_handler;
		break;
#endif /* FEATURE_WLAN_TDLS */

	case WMI_PEER_STA_KICKOUT_REASON_XRETRY:
		if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_STA &&
		    (wma->interfaces[vdev_id].sub_type == 0 ||
		     wma->interfaces[vdev_id].sub_type ==
		     WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT) &&
		    cdf_mem_compare(wma->interfaces[vdev_id].bssid,
				    macaddr, IEEE80211_ADDR_LEN)) {
			/*
			 * KICKOUT event is for current station-AP connection.
			 * Treat it like final beacon miss. Station may not have
			 * missed beacons but not able to transmit frames to AP
			 * for a long time. Must disconnect to get out of
			 * this sticky situation.
			 * In future implementation, roaming module will also
			 * handle this event and perform a scan.
			 */
			WMA_LOGW("%s: WMI_PEER_STA_KICKOUT_REASON_XRETRY event for STA",
				__func__);
			wma_beacon_miss_handler(wma, vdev_id);
			goto exit_handler;
		}
		break;

	case WMI_PEER_STA_KICKOUT_REASON_UNSPECIFIED:
		/*
		 * Default legacy value used by original firmware implementation.
		 */
		if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_STA &&
		    (wma->interfaces[vdev_id].sub_type == 0 ||
		     wma->interfaces[vdev_id].sub_type ==
		     WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT) &&
		    cdf_mem_compare(wma->interfaces[vdev_id].bssid,
				    macaddr, IEEE80211_ADDR_LEN)) {
			/*
			 * KICKOUT event is for current station-AP connection.
			 * Treat it like final beacon miss. Station may not have
			 * missed beacons but not able to transmit frames to AP
			 * for a long time. Must disconnect to get out of
			 * this sticky situation.
			 * In future implementation, roaming module will also
			 * handle this event and perform a scan.
			 */
			WMA_LOGW("%s: WMI_PEER_STA_KICKOUT_REASON_UNSPECIFIED event for STA",
				__func__);
			wma_beacon_miss_handler(wma, vdev_id);
			goto exit_handler;
		}
		break;

	case WMI_PEER_STA_KICKOUT_REASON_INACTIVITY:
	/* Handle SA query kickout is same as inactivity kickout */
	case WMI_PEER_STA_KICKOUT_REASON_SA_QUERY_TIMEOUT:
	default:
		break;
	}

	/*
	 * default action is to send delete station context indication to LIM
	 */
	del_sta_ctx =
		(tpDeleteStaContext) cdf_mem_malloc(sizeof(tDeleteStaContext));
	if (!del_sta_ctx) {
		WMA_LOGE("CDF MEM Alloc Failed for tDeleteStaContext");
		return -ENOMEM;
	}

	del_sta_ctx->staId = peer_id;
	cdf_mem_copy(del_sta_ctx->addr2, macaddr, IEEE80211_ADDR_LEN);
	cdf_mem_copy(del_sta_ctx->bssId, wma->interfaces[vdev_id].addr,
		     IEEE80211_ADDR_LEN);
	del_sta_ctx->reasonCode = HAL_DEL_STA_REASON_CODE_KEEP_ALIVE;
	del_sta_ctx->rssi = kickout_event->rssi + WMA_TGT_NOISE_FLOOR_DBM;
	wma_send_msg(wma, SIR_LIM_DELETE_STA_CONTEXT_IND, (void *)del_sta_ctx,
		     0);

exit_handler:
	WMA_LOGD("%s: Exit", __func__);
	return 0;
}

/**
 * wma_unified_bcntx_status_event_handler() - beacon tx status event handler
 * @handle: wma handle
 * @cmd_param_info: event data
 * @len: data length
 *
 * WMI Handler for WMI_OFFLOAD_BCN_TX_STATUS_EVENTID event from firmware.
 * This event is generated by FW when the beacon transmission is offloaded
 * and the host performs beacon template modification using WMI_BCN_TMPL_CMDID
 * The FW generates this event when the first successful beacon transmission
 * after template update
 *
 * Return: 0 for success or error code
 */
int wma_unified_bcntx_status_event_handler(void *handle,
					   uint8_t *cmd_param_info,
					   uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_OFFLOAD_BCN_TX_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_offload_bcn_tx_status_event_fixed_param *resp_event;
	tSirFirstBeaconTxCompleteInd *beacon_tx_complete_ind;

	param_buf =
		(WMI_OFFLOAD_BCN_TX_STATUS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid bcn tx response event buffer");
		return -EINVAL;
	}

	resp_event = param_buf->fixed_param;

	/* Check for valid handle to ensure session is not
	 * deleted in any race
	 */
	if (!wma->interfaces[resp_event->vdev_id].handle) {
		WMA_LOGE("%s: The session does not exist", __func__);
		return -EINVAL;
	}

	/* Beacon Tx Indication supports only AP mode. Ignore in other modes */
	if (wma_is_vdev_in_ap_mode(wma, resp_event->vdev_id) == false) {
		WMA_LOGI("%s: Beacon Tx Indication does not support type %d and sub_type %d",
			__func__, wma->interfaces[resp_event->vdev_id].type,
			wma->interfaces[resp_event->vdev_id].sub_type);
		return 0;
	}

	beacon_tx_complete_ind = (tSirFirstBeaconTxCompleteInd *)
			cdf_mem_malloc(sizeof(tSirFirstBeaconTxCompleteInd));
	if (!beacon_tx_complete_ind) {
		WMA_LOGE("%s: Failed to alloc beacon_tx_complete_ind",
			 __func__);
		return -ENOMEM;
	}

	beacon_tx_complete_ind->messageType = WMA_DFS_BEACON_TX_SUCCESS_IND;
	beacon_tx_complete_ind->length = sizeof(tSirFirstBeaconTxCompleteInd);
	beacon_tx_complete_ind->bssIdx = resp_event->vdev_id;

	wma_send_msg(wma, WMA_DFS_BEACON_TX_SUCCESS_IND,
		     (void *)beacon_tx_complete_ind, 0);
	return 0;
}

/**
 * wma_get_link_probe_timeout() - get link timeout based on sub type
 * @mac: UMAC handler
 * @sub_type: vdev syb type
 * @max_inactive_time: return max inactive time
 * @max_unresponsive_time: return max unresponsive time
 *
 * Return: none
 */
static inline void wma_get_link_probe_timeout(struct sAniSirGlobal *mac,
					      uint32_t sub_type,
					      uint32_t *max_inactive_time,
					      uint32_t *max_unresponsive_time)
{
	uint32_t keep_alive;
	uint16_t lm_id, ka_id;

	switch (sub_type) {
	case WMI_UNIFIED_VDEV_SUBTYPE_P2P_GO:
		lm_id = WNI_CFG_GO_LINK_MONITOR_TIMEOUT;
		ka_id = WNI_CFG_GO_KEEP_ALIVE_TIMEOUT;
		break;
	default:
		/*For softAp the subtype value will be zero */
		lm_id = WNI_CFG_AP_LINK_MONITOR_TIMEOUT;
		ka_id = WNI_CFG_AP_KEEP_ALIVE_TIMEOUT;
	}

	if (wlan_cfg_get_int(mac, lm_id, max_inactive_time) != eSIR_SUCCESS) {
		WMA_LOGE("Failed to read link monitor for subtype %d",
			 sub_type);
		*max_inactive_time = WMA_LINK_MONITOR_DEFAULT_TIME_SECS;
	}

	if (wlan_cfg_get_int(mac, ka_id, &keep_alive) != eSIR_SUCCESS) {
		WMA_LOGE("Failed to read keep alive for subtype %d", sub_type);
		keep_alive = WMA_KEEP_ALIVE_DEFAULT_TIME_SECS;
	}
	*max_unresponsive_time = *max_inactive_time + keep_alive;
}

/**
 * wma_set_sap_keepalive() - set SAP keep alive parameters to fw
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: none
 */
void wma_set_sap_keepalive(tp_wma_handle wma, uint8_t vdev_id)
{
	uint32_t min_inactive_time, max_inactive_time, max_unresponsive_time;
	struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (NULL == mac) {
		WMA_LOGE("%s: Failed to get mac", __func__);
		return;
	}

	wma_get_link_probe_timeout(mac, wma->interfaces[vdev_id].sub_type,
				   &max_inactive_time, &max_unresponsive_time);

	min_inactive_time = max_inactive_time / 2;

	if (wmi_unified_vdev_set_param_send(wma->wmi_handle,
		vdev_id, WMI_VDEV_PARAM_AP_KEEPALIVE_MIN_IDLE_INACTIVE_TIME_SECS,
					min_inactive_time))
		WMA_LOGE("Failed to Set AP MIN IDLE INACTIVE TIME");

	if (wmi_unified_vdev_set_param_send(wma->wmi_handle,
		vdev_id, WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_IDLE_INACTIVE_TIME_SECS,
					    max_inactive_time))
		WMA_LOGE("Failed to Set AP MAX IDLE INACTIVE TIME");

	if (wmi_unified_vdev_set_param_send(wma->wmi_handle,
		vdev_id, WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS,
					    max_unresponsive_time))
		WMA_LOGE("Failed to Set MAX UNRESPONSIVE TIME");

	WMA_LOGD("%s:vdev_id:%d min_inactive_time: %u max_inactive_time: %u"
		 " max_unresponsive_time: %u", __func__, vdev_id,
		 min_inactive_time, max_inactive_time, max_unresponsive_time);
}

/**
 * wma_set_sta_sa_query_param() - set sta sa query parameters
 * @wma: wma handle
 * @vdev_id: vdev id

 * This function sets sta query related parameters in fw.
 *
 * Return: none
 */

void wma_set_sta_sa_query_param(tp_wma_handle wma,
				  uint8_t vdev_id)
{
	struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t max_retries, retry_interval;
	wmi_buf_t buf;
	WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param *cmd;
	int len;

	WMA_LOGD(FL("Enter:"));
	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE(FL("wmi_buf_alloc failed"));
		return;
	}

	cmd = (WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param));

	if (wlan_cfg_get_int
		    (mac, WNI_CFG_PMF_SA_QUERY_MAX_RETRIES,
		    &max_retries) != eSIR_SUCCESS) {
		max_retries = DEFAULT_STA_SA_QUERY_MAX_RETRIES_COUNT;
		WMA_LOGE(FL("Failed to get value for WNI_CFG_PMF_SA_QUERY_MAX_RETRIES"));
	}
	if (wlan_cfg_get_int
		    (mac, WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL,
		    &retry_interval) != eSIR_SUCCESS) {
		retry_interval = DEFAULT_STA_SA_QUERY_RETRY_INTERVAL;
		WMA_LOGE(FL("Failed to get value for WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL"));
	}

	cmd->vdev_id = vdev_id;
	cmd->sa_query_max_retry_count = max_retries;
	cmd->sa_query_retry_interval = retry_interval;

	WMA_LOGD(FL("STA sa query: vdev_id:%d interval:%u retry count:%d"),
		 vdev_id, retry_interval, max_retries);

	if (wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				 WMI_PMF_OFFLOAD_SET_SA_QUERY_CMDID)) {
		WMA_LOGE(FL("Failed to offload STA SA Query"));
		cdf_nbuf_free(buf);
	}

	WMA_LOGD(FL("Exit :"));
	return;
}

/**
 * wma_set_sta_keep_alive() - set sta keep alive parameters
 * @wma: wma handle
 * @vdev_id: vdev id
 * @method: method for keep alive
 * @timeperiod: time period
 * @hostv4addr: host ipv4 address
 * @destv4addr: dst ipv4 address
 * @destmac: destination mac
 *
 * This function sets keep alive related parameters in fw.
 *
 * Return: none
 */
void wma_set_sta_keep_alive(tp_wma_handle wma, uint8_t vdev_id,
			    uint32_t method, uint32_t timeperiod,
			    uint8_t *hostv4addr, uint8_t *destv4addr,
			    uint8_t *destmac)
{
	wmi_buf_t buf;
	WMI_STA_KEEPALIVE_CMD_fixed_param *cmd;
	WMI_STA_KEEPALVE_ARP_RESPONSE *arp_rsp;
	uint8_t *buf_ptr;
	int len;

	WMA_LOGD("%s: Enter", __func__);

	if (timeperiod > WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD_STAMAX) {
		WMA_LOGE("Invalid period %d Max limit %d", timeperiod,
			WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD_STAMAX);
		return;
	}

	len = sizeof(*cmd) + sizeof(*arp_rsp);
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("wmi_buf_alloc failed");
		return;
	}

	cmd = (WMI_STA_KEEPALIVE_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_STA_KEEPALIVE_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_STA_KEEPALIVE_CMD_fixed_param));
	cmd->interval = timeperiod;
	cmd->enable = (timeperiod) ? 1 : 0;
	cmd->vdev_id = vdev_id;
	WMA_LOGD("Keep Alive: vdev_id:%d interval:%u method:%d", vdev_id,
		 timeperiod, method);
	arp_rsp = (WMI_STA_KEEPALVE_ARP_RESPONSE *) (buf_ptr + sizeof(*cmd));
	WMITLV_SET_HDR(&arp_rsp->tlv_header,
		       WMITLV_TAG_STRUC_WMI_STA_KEEPALVE_ARP_RESPONSE,
		       WMITLV_GET_STRUCT_TLVLEN(WMI_STA_KEEPALVE_ARP_RESPONSE));

	if (method == SIR_KEEP_ALIVE_UNSOLICIT_ARP_RSP) {
		if ((NULL == hostv4addr) ||
			(NULL == destv4addr) ||
			(NULL == destmac)) {
			WMA_LOGE("%s: received null pointer, hostv4addr:%p "
			   "destv4addr:%p destmac:%p ", __func__,
			   hostv4addr, destv4addr, destmac);
			cdf_nbuf_free(buf);
			return;
		}
		cmd->method = WMI_STA_KEEPALIVE_METHOD_UNSOLICITED_ARP_RESPONSE;
		cdf_mem_copy(&arp_rsp->sender_prot_addr, hostv4addr,
			     SIR_IPV4_ADDR_LEN);
		cdf_mem_copy(&arp_rsp->target_prot_addr, destv4addr,
			     SIR_IPV4_ADDR_LEN);
		WMI_CHAR_ARRAY_TO_MAC_ADDR(destmac, &arp_rsp->dest_mac_addr);
	} else {
		cmd->method = WMI_STA_KEEPALIVE_METHOD_NULL_FRAME;
	}

	if (wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				 WMI_STA_KEEPALIVE_CMDID)) {
		WMA_LOGE("Failed to set KeepAlive");
		cdf_nbuf_free(buf);
	}

	WMA_LOGD("%s: Exit", __func__);
	return;
}

/**
 * wma_vdev_install_key_complete_event_handler() - install key complete handler
 * @handle: wma handle
 * @event: event data
 * @len: data length
 *
 * This event is sent by fw once WPA/WPA2 keys are installed in fw.
 *
 * Return: 0 for success or error code
 */
int wma_vdev_install_key_complete_event_handler(void *handle,
						uint8_t *event,
						uint32_t len)
{
	WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID_param_tlvs *param_buf = NULL;
	wmi_vdev_install_key_complete_event_fixed_param *key_fp = NULL;

	if (!event) {
		WMA_LOGE("%s: event param null", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("%s: received null buf from target", __func__);
		return -EINVAL;
	}

	key_fp = param_buf->fixed_param;
	if (!key_fp) {
		WMA_LOGE("%s: received null event data from target", __func__);
		return -EINVAL;
	}
	/*
	 * Do nothing for now. Completion of set key is already indicated to lim
	 */
	WMA_LOGI("%s: WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID", __func__);
	return 0;
}
/*
 * 802.11n D2.0 defined values for "Minimum MPDU Start Spacing":
 *   0 for no restriction
 *   1 for 1/4 us - Our lower layer calculations limit our precision to 1 msec
 *   2 for 1/2 us - Our lower layer calculations limit our precision to 1 msec
 *   3 for 1 us
 *   4 for 2 us
 *   5 for 4 us
 *   6 for 8 us
 *   7 for 16 us
 */
static const uint8_t wma_mpdu_spacing[] = { 0, 1, 1, 1, 2, 4, 8, 16 };

/**
 * wma_parse_mpdudensity() - give mpdu spacing from mpdu density
 * @mpdudensity: mpdu density
 *
 * Return: mpdu spacing or 0 for error
 */
static inline uint8_t wma_parse_mpdudensity(uint8_t mpdudensity)
{
	if (mpdudensity < sizeof(wma_mpdu_spacing))
		return wma_mpdu_spacing[mpdudensity];
	else
		return 0;
}

/**
 * wmi_unified_send_peer_assoc() - send peer assoc command to fw
 * @wma: wma handle
 * @nw_type: nw type
 * @params: add sta params
 *
 * This function send peer assoc command to firmware with
 * different parameters.
 *
 * Return: 0 for success or error code
 */
int32_t wmi_unified_send_peer_assoc(tp_wma_handle wma,
				    tSirNwType nw_type,
				    tpAddStaParams params)
{
	ol_txrx_pdev_handle pdev;
	wmi_peer_assoc_complete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int32_t ret, max_rates, i;
	uint8_t rx_stbc, tx_stbc;
	uint8_t *rate_pos, *buf_ptr;
	wmi_rate_set peer_legacy_rates, peer_ht_rates;
	wmi_vht_rate_set *mcs;
	uint32_t num_peer_legacy_rates;
	uint32_t num_peer_ht_rates;
	uint32_t num_peer_11b_rates = 0;
	uint32_t num_peer_11a_rates = 0;
	uint32_t phymode;
	uint32_t peer_nss = 1;
	struct wma_txrx_node *intr = NULL;

	if (NULL == params) {
		WMA_LOGE("%s: params is NULL", __func__);
		return -EINVAL;
	}
	intr = &wma->interfaces[params->smesessionId];

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		return -EINVAL;
	}

	cdf_mem_zero(&peer_legacy_rates, sizeof(wmi_rate_set));
	cdf_mem_zero(&peer_ht_rates, sizeof(wmi_rate_set));

	phymode = wma_peer_phymode(nw_type, params->staType,
				   params->htCapable,
				   params->ch_width,
				   params->vhtCapable);

	/* Legacy Rateset */
	rate_pos = (uint8_t *) peer_legacy_rates.rates;
	for (i = 0; i < SIR_NUM_11B_RATES; i++) {
		if (!params->supportedRates.llbRates[i])
			continue;
		rate_pos[peer_legacy_rates.num_rates++] =
			params->supportedRates.llbRates[i];
		num_peer_11b_rates++;
	}
	for (i = 0; i < SIR_NUM_11A_RATES; i++) {
		if (!params->supportedRates.llaRates[i])
			continue;
		rate_pos[peer_legacy_rates.num_rates++] =
			params->supportedRates.llaRates[i];
		num_peer_11a_rates++;
	}

	if ((phymode == MODE_11A && num_peer_11a_rates == 0) ||
	    (phymode == MODE_11B && num_peer_11b_rates == 0)) {
		WMA_LOGW("%s: Invalid phy rates. phymode 0x%x, 11b_rates %d, 11a_rates %d",
			__func__, phymode, num_peer_11b_rates, num_peer_11a_rates);
		return -EINVAL;
	}
	/* Set the Legacy Rates to Word Aligned */
	num_peer_legacy_rates = roundup(peer_legacy_rates.num_rates,
					sizeof(uint32_t));

	/* HT Rateset */
	max_rates = sizeof(peer_ht_rates.rates) /
		    sizeof(peer_ht_rates.rates[0]);
	rate_pos = (uint8_t *) peer_ht_rates.rates;
	for (i = 0; i < MAX_SUPPORTED_RATES; i++) {
		if (params->supportedRates.supportedMCSSet[i / 8] &
		    (1 << (i % 8))) {
			rate_pos[peer_ht_rates.num_rates++] = i;
			if (i >= 8) {
				/* MCS8 or higher rate is present, must be 2x2 */
				peer_nss = 2;
			}
		}
		if (peer_ht_rates.num_rates == max_rates)
			break;
	}

	if (params->htCapable && !peer_ht_rates.num_rates) {
		uint8_t temp_ni_rates[8] = { 0x0, 0x1, 0x2, 0x3,
					     0x4, 0x5, 0x6, 0x7};
		/*
		 * Workaround for EV 116382: The peer is marked HT but with
		 * supported rx mcs set is set to 0. 11n spec mandates MCS0-7
		 * for a HT STA. So forcing the supported rx mcs rate to
		 * MCS 0-7. This workaround will be removed once we get
		 * clarification from WFA regarding this STA behavior.
		 */

		/* TODO: Do we really need this? */
		WMA_LOGW("Peer is marked as HT capable but supported mcs rate is 0");
		peer_ht_rates.num_rates = sizeof(temp_ni_rates);
		cdf_mem_copy((uint8_t *) peer_ht_rates.rates, temp_ni_rates,
			     peer_ht_rates.num_rates);
	}

	/* Set the Peer HT Rates to Word Aligned */
	num_peer_ht_rates = roundup(peer_ht_rates.num_rates, sizeof(uint32_t));

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + /* Place holder for peer legacy rate array */
	      (num_peer_legacy_rates * sizeof(uint8_t)) + /* peer legacy rate array size */
	      WMI_TLV_HDR_SIZE + /* Place holder for peer Ht rate array */
	      (num_peer_ht_rates * sizeof(uint8_t)) +   /* peer HT rate array size */
	      sizeof(wmi_vht_rate_set);

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_peer_assoc_complete_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_assoc_complete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_assoc_complete_cmd_fixed_param));

	/* in ap/ibss mode and for tdls peer, use mac address of the peer in
	 * the other end as the new peer address; in sta mode, use bss id to
	 * be the new peer address
	 */
	if ((wma_is_vdev_in_ap_mode(wma, params->smesessionId))
	    || (wma_is_vdev_in_ibss_mode(wma, params->smesessionId))
#ifdef FEATURE_WLAN_TDLS
	    || (STA_ENTRY_TDLS_PEER == params->staType)
#endif /* FEATURE_WLAN_TDLS */
	    )
		WMI_CHAR_ARRAY_TO_MAC_ADDR(params->staMac, &cmd->peer_macaddr);
	else
		WMI_CHAR_ARRAY_TO_MAC_ADDR(params->bssId, &cmd->peer_macaddr);
	cmd->vdev_id = params->smesessionId;
	cmd->peer_new_assoc = 1;
	cmd->peer_associd = params->assocId;

	/*
	 * The target only needs a subset of the flags maintained in the host.
	 * Just populate those flags and send it down
	 */
	cmd->peer_flags = 0;

	if (params->wmmEnabled)
		cmd->peer_flags |= WMI_PEER_QOS;

	if (params->uAPSD) {
		cmd->peer_flags |= WMI_PEER_APSD;
		WMA_LOGD("Set WMI_PEER_APSD: uapsd Mask %d", params->uAPSD);
	}

	if (params->htCapable) {
		cmd->peer_flags |= (WMI_PEER_HT | WMI_PEER_QOS);
		cmd->peer_rate_caps |= WMI_RC_HT_FLAG;
	}

	if (params->ch_width) {
		cmd->peer_flags |= WMI_PEER_40MHZ;
		cmd->peer_rate_caps |= WMI_RC_CW40_FLAG;
		if (params->fShortGI40Mhz)
			cmd->peer_rate_caps |= WMI_RC_SGI_FLAG;
	} else if (params->fShortGI20Mhz)
		cmd->peer_rate_caps |= WMI_RC_SGI_FLAG;

#ifdef WLAN_FEATURE_11AC
	if (params->vhtCapable) {
		cmd->peer_flags |= (WMI_PEER_HT | WMI_PEER_VHT | WMI_PEER_QOS);
		cmd->peer_rate_caps |= WMI_RC_HT_FLAG;
	}

	if (params->ch_width == CH_WIDTH_80MHZ)
		cmd->peer_flags |= WMI_PEER_80MHZ;
	else if (params->ch_width == CH_WIDTH_160MHZ)
		cmd->peer_flags |= WMI_PEER_160MHZ;
	else if (params->ch_width == CH_WIDTH_80P80MHZ)
		cmd->peer_flags |= WMI_PEER_160MHZ;

	cmd->peer_vht_caps = params->vht_caps;
#endif /* WLAN_FEATURE_11AC */

	if (params->rmfEnabled)
		cmd->peer_flags |= WMI_PEER_PMF;

	rx_stbc = (params->ht_caps & IEEE80211_HTCAP_C_RXSTBC) >>
		  IEEE80211_HTCAP_C_RXSTBC_S;
	if (rx_stbc) {
		cmd->peer_flags |= WMI_PEER_STBC;
		cmd->peer_rate_caps |= (rx_stbc << WMI_RC_RX_STBC_FLAG_S);
	}

	tx_stbc = (params->ht_caps & IEEE80211_HTCAP_C_TXSTBC) >>
		  IEEE80211_HTCAP_C_TXSTBC_S;
	if (tx_stbc) {
		cmd->peer_flags |= WMI_PEER_STBC;
		cmd->peer_rate_caps |= (tx_stbc << WMI_RC_TX_STBC_FLAG_S);
	}

	if (params->htLdpcCapable || params->vhtLdpcCapable)
		cmd->peer_flags |= WMI_PEER_LDPC;

	switch (params->mimoPS) {
	case eSIR_HT_MIMO_PS_STATIC:
		cmd->peer_flags |= WMI_PEER_STATIC_MIMOPS;
		break;
	case eSIR_HT_MIMO_PS_DYNAMIC:
		cmd->peer_flags |= WMI_PEER_DYN_MIMOPS;
		break;
	case eSIR_HT_MIMO_PS_NO_LIMIT:
		cmd->peer_flags |= WMI_PEER_SPATIAL_MUX;
		break;
	default:
		break;
	}

#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType)
		cmd->peer_flags |= WMI_PEER_AUTH;
#endif /* FEATURE_WLAN_TDLS */

	if (params->wpa_rsn
#ifdef FEATURE_WLAN_WAPI
	    || params->encryptType == eSIR_ED_WPI
#endif /* FEATURE_WLAN_WAPI */
	    )
		cmd->peer_flags |= WMI_PEER_NEED_PTK_4_WAY;
	if (params->wpa_rsn >> 1)
		cmd->peer_flags |= WMI_PEER_NEED_GTK_2_WAY;

	ol_txrx_peer_state_update(pdev, params->bssId, ol_txrx_peer_state_auth);

#ifdef FEATURE_WLAN_WAPI
	if (params->encryptType == eSIR_ED_WPI) {
		ret = wmi_unified_vdev_set_param_send(wma->wmi_handle,
						      params->smesessionId,
						      WMI_VDEV_PARAM_DROP_UNENCRY,
						      false);
		if (ret) {
			WMA_LOGE
				("Set WMI_VDEV_PARAM_DROP_UNENCRY Param status:%d\n",
				ret);
			cdf_nbuf_free(buf);
			return ret;
		}
	}
#endif /* FEATURE_WLAN_WAPI */

	cmd->peer_caps = params->capab_info;
	cmd->peer_listen_intval = params->listenInterval;
	cmd->peer_ht_caps = params->ht_caps;
	cmd->peer_max_mpdu = (1 << (IEEE80211_HTCAP_MAXRXAMPDU_FACTOR +
				    params->maxAmpduSize)) - 1;
	cmd->peer_mpdu_density = wma_parse_mpdudensity(params->maxAmpduDensity);

	if (params->supportedRates.supportedMCSSet[1] &&
	    params->supportedRates.supportedMCSSet[2])
		cmd->peer_rate_caps |= WMI_RC_TS_FLAG;
	else if (params->supportedRates.supportedMCSSet[1])
		cmd->peer_rate_caps |= WMI_RC_DS_FLAG;

	/* Update peer legacy rate information */
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, num_peer_legacy_rates);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_legacy_rates = peer_legacy_rates.num_rates;
	cdf_mem_copy(buf_ptr, peer_legacy_rates.rates,
		     peer_legacy_rates.num_rates);

	/* Update peer HT rate information */
	buf_ptr += num_peer_legacy_rates;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, num_peer_ht_rates);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_ht_rates = peer_ht_rates.num_rates;
	cdf_mem_copy(buf_ptr, peer_ht_rates.rates, peer_ht_rates.num_rates);

	/* VHT Rates */
	buf_ptr += num_peer_ht_rates;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_vht_rate_set,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vht_rate_set));

	cmd->peer_nss = peer_nss;
	/*
	 * Because of DBS a vdev may come up in any of the two MACs with
	 * different capabilities. STBC capab should be fetched for given
	 * hard_mode->MAC_id combo. It is planned that firmware should provide
	 * these dev capabilities. But for now number of tx streams can be used
	 * to identify if Tx STBC needs to be disabled.
	 */
	if (intr->tx_streams < 2) {
		cmd->peer_vht_caps &= ~(1 << SIR_MAC_VHT_CAP_TXSTBC);
		WMA_LOGD("Num tx_streams: %d, Disabled txSTBC",
			 intr->tx_streams);
	}
	WMA_LOGD("peer_nss %d peer_ht_rates.num_rates %d ", cmd->peer_nss,
		 peer_ht_rates.num_rates);

	mcs = (wmi_vht_rate_set *) buf_ptr;
	if (params->vhtCapable) {
#define VHT2x2MCSMASK 0xc
		mcs->rx_max_rate = params->supportedRates.vhtRxHighestDataRate;
		mcs->rx_mcs_set = params->supportedRates.vhtRxMCSMap;
		mcs->tx_max_rate = params->supportedRates.vhtTxHighestDataRate;
		mcs->tx_mcs_set = params->supportedRates.vhtTxMCSMap;

		if (params->vhtSupportedRxNss) {
			cmd->peer_nss = params->vhtSupportedRxNss;
		} else {
			cmd->peer_nss = ((mcs->rx_mcs_set & VHT2x2MCSMASK)
					 == VHT2x2MCSMASK) ? 1 : 2;
		}
	}

	/*
	 * Limit nss to max number of rf chain supported by target
	 * Otherwise Fw will crash
	 */
	wma_update_txrx_chainmask(wma->num_rf_chains, &cmd->peer_nss);

	intr->nss = cmd->peer_nss;
	cmd->peer_phymode = phymode;
	WMA_LOGD("%s: vdev_id %d associd %d peer_flags %x rate_caps %x "
		 "peer_caps %x listen_intval %d ht_caps %x max_mpdu %d "
		 "nss %d phymode %d peer_mpdu_density %d encr_type %d "
		 "cmd->peer_vht_caps %x", __func__,
		 cmd->vdev_id, cmd->peer_associd, cmd->peer_flags,
		 cmd->peer_rate_caps, cmd->peer_caps,
		 cmd->peer_listen_intval, cmd->peer_ht_caps,
		 cmd->peer_max_mpdu, cmd->peer_nss, cmd->peer_phymode,
		 cmd->peer_mpdu_density, params->encryptType,
		 cmd->peer_vht_caps);

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_PEER_ASSOC_CMDID);
	if (ret != EOK) {
		WMA_LOGP("%s: Failed to send peer assoc command ret = %d",
			 __func__, ret);
		cdf_nbuf_free(buf);
	}
	return ret;
}

/**
 * wmi_unified_vdev_set_gtx_cfg_send() - set GTX params
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 * @gtx_info: GTX config params
 *
 * This function set GTX related params in firmware.
 *
 * Return: 0 for success or error code
 */
int wmi_unified_vdev_set_gtx_cfg_send(wmi_unified_t wmi_handle, uint32_t if_id,
				  gtx_config_t *gtx_info)
{
	wmi_vdev_set_gtx_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_set_gtx_params_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s:wmi_buf_alloc failed", __FUNCTION__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_set_gtx_params_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_gtx_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_gtx_params_cmd_fixed_param));
	cmd->vdev_id = if_id;

	cmd->gtxRTMask[0] = gtx_info->gtxRTMask[0];
	cmd->gtxRTMask[1] = gtx_info->gtxRTMask[1];
	cmd->userGtxMask = gtx_info->gtxUsrcfg;
	cmd->gtxPERThreshold = gtx_info->gtxPERThreshold;
	cmd->gtxPERMargin = gtx_info->gtxPERMargin;
	cmd->gtxTPCstep = gtx_info->gtxTPCstep;
	cmd->gtxTPCMin = gtx_info->gtxTPCMin;
	cmd->gtxBWMask = gtx_info->gtxBWMask;

	WMA_LOGD("Setting vdev%d GTX values:htmcs 0x%x, vhtmcs 0x%x, usermask 0x%x, \
		gtxPERThreshold %d, gtxPERMargin %d, gtxTPCstep %d, gtxTPCMin %d, \
		gtxBWMask 0x%x.", if_id, cmd->gtxRTMask[0], cmd->gtxRTMask[1],
		 cmd->userGtxMask, cmd->gtxPERThreshold, cmd->gtxPERMargin,
		 cmd->gtxTPCstep, cmd->gtxTPCMin, cmd->gtxBWMask);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
				    WMI_VDEV_SET_GTX_PARAMS_CMDID);
}

/**
 * wma_update_protection_mode() - update protection mode
 * @wma: wma handle
 * @vdev_id: vdev id
 * @llbcoexist: protection mode info
 *
 * This function set protection mode(RTS/CTS) to fw for passed vdev id.
 *
 * Return: none
 */
void wma_update_protection_mode(tp_wma_handle wma, uint8_t vdev_id,
			   uint8_t llbcoexist)
{
	int ret;
	enum ieee80211_protmode prot_mode;

	prot_mode = llbcoexist ? IEEE80211_PROT_CTSONLY : IEEE80211_PROT_NONE;

	ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_PROTECTION_MODE,
					      prot_mode);

	if (ret)
		WMA_LOGE("Failed to send wmi protection mode cmd");
	else
		WMA_LOGD("Updated protection mode %d to target", prot_mode);
}

/**
 * wma_update_beacon_interval() - update beacon interval in fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @beaconInterval: becon interval
 *
 * Return: none
 */
static void
wma_update_beacon_interval(tp_wma_handle wma, uint8_t vdev_id,
			   uint16_t beaconInterval)
{
	int ret;

	ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_BEACON_INTERVAL,
					      beaconInterval);

	if (ret)
		WMA_LOGE("Failed to update beacon interval");
	else
		WMA_LOGI("Updated beacon interval %d for vdev %d",
			 beaconInterval, vdev_id);
}

/**
 * wma_process_update_beacon_params() - update beacon parameters to target
 * @wma: wma handle
 * @bcn_params: beacon parameters
 *
 * Return: none
 */
void
wma_process_update_beacon_params(tp_wma_handle wma,
				 tUpdateBeaconParams *bcn_params)
{
	if (!bcn_params) {
		WMA_LOGE("bcn_params NULL");
		return;
	}

	if (bcn_params->smeSessionId >= wma->max_bssid) {
		WMA_LOGE("Invalid vdev id %d", bcn_params->smeSessionId);
		return;
	}

	if (bcn_params->paramChangeBitmap & PARAM_BCN_INTERVAL_CHANGED) {
		wma_update_beacon_interval(wma, bcn_params->smeSessionId,
					   bcn_params->beaconInterval);
	}

	if (bcn_params->paramChangeBitmap & PARAM_llBCOEXIST_CHANGED)
		wma_update_protection_mode(wma, bcn_params->smeSessionId,
					   bcn_params->llbCoexist);
}

/**
 * wma_update_cfg_params() - update cfg parameters to target
 * @wma: wma handle
 * @cfgParam: cfg parameter
 *
 * Return: none
 */
void wma_update_cfg_params(tp_wma_handle wma, tSirMsgQ *cfgParam)
{
	uint8_t vdev_id;
	uint32_t param_id;
	uint32_t cfg_val;
	int ret;
	/* get mac to acess CFG data base */
	struct sAniSirGlobal *pmac;

	switch (cfgParam->bodyval) {
	case WNI_CFG_RTS_THRESHOLD:
		param_id = WMI_VDEV_PARAM_RTS_THRESHOLD;
		break;
	case WNI_CFG_FRAGMENTATION_THRESHOLD:
		param_id = WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD;
		break;
	default:
		WMA_LOGD("Unhandled cfg parameter %d", cfgParam->bodyval);
		return;
	}

	pmac = cds_get_context(QDF_MODULE_ID_PE);

	if (NULL == pmac) {
		WMA_LOGE("%s: Failed to get pmac", __func__);
		return;
	}

	if (wlan_cfg_get_int(pmac, (uint16_t) cfgParam->bodyval,
			     &cfg_val) != eSIR_SUCCESS) {
		WMA_LOGE("Failed to get value for CFG PARAMS %d. returning without updating",
			cfgParam->bodyval);
		return;
	}

	for (vdev_id = 0; vdev_id < wma->max_bssid; vdev_id++) {
		if (wma->interfaces[vdev_id].handle != 0) {
			ret = wmi_unified_vdev_set_param_send(wma->wmi_handle,
							      vdev_id, param_id,
							      cfg_val);
			if (ret)
				WMA_LOGE("Update cfg params failed for vdevId %d",
					vdev_id);
		}
	}
}

/**
 * wma_read_cfg_wepkey() - fill key_info for WEP key
 * @wma_handle: wma handle
 * @key_info: key_info ptr
 * @def_key_idx: default key index
 * @num_keys: number of keys
 *
 * This function reads WEP keys from cfg and fills
 * up key_info.
 *
 * Return: none
 */
static void wma_read_cfg_wepkey(tp_wma_handle wma_handle,
				tSirKeys *key_info, uint32_t *def_key_idx,
				uint8_t *num_keys)
{
	tSirRetStatus status;
	uint32_t val = SIR_MAC_KEY_LENGTH;
	uint8_t i, j;

	WMA_LOGD("Reading WEP keys from cfg");
	/* NOTE:def_key_idx is initialized to 0 by the caller */
	status = wlan_cfg_get_int(wma_handle->mac_context,
				  WNI_CFG_WEP_DEFAULT_KEYID, def_key_idx);
	if (status != eSIR_SUCCESS)
		WMA_LOGE("Unable to read default id, defaulting to 0");

	for (i = 0, j = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; i++) {
		status = wlan_cfg_get_str(wma_handle->mac_context,
					  (uint16_t) WNI_CFG_WEP_DEFAULT_KEY_1 +
					  i, key_info[j].key, &val);
		if (status != eSIR_SUCCESS) {
			WMA_LOGE("WEP key is not configured at :%d", i);
		} else {
			key_info[j].keyId = i;
			key_info[j].keyLength = (uint16_t) val;
			j++;
		}
	}
	*num_keys = j;
}

/**
 * wma_setup_install_key_cmd() - fill wmi buffer as per key parameters
 * @wma_handle: wma handle
 * @key_params: key parameters
 * @len: length
 * @mode: op mode
 *
 * This function setsup wmi buffer from information
 * passed in key_params.
 *
 * Return: filled wmi buffer ptr or NULL for error
 */
static wmi_buf_t wma_setup_install_key_cmd(tp_wma_handle wma_handle,
					   struct wma_set_key_params
					   *key_params, uint32_t *len,
					   uint8_t mode)
{
	wmi_vdev_install_key_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint8_t *key_data;
#ifdef WLAN_FEATURE_11W
	struct wma_txrx_node *iface = NULL;
#endif /* WLAN_FEATURE_11W */
	if ((key_params->key_type == eSIR_ED_NONE &&
	     key_params->key_len) || (key_params->key_type != eSIR_ED_NONE &&
				      !key_params->key_len)) {
		WMA_LOGE("%s:Invalid set key request", __func__);
		return NULL;
	}

	*len = sizeof(*cmd) + roundup(key_params->key_len, sizeof(uint32_t)) +
	       WMI_TLV_HDR_SIZE;

	buf = wmi_buf_alloc(wma_handle->wmi_handle, *len);
	if (!buf) {
		WMA_LOGE("Failed to allocate buffer to send set key cmd");
		return NULL;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_install_key_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_install_key_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_install_key_cmd_fixed_param));
	cmd->vdev_id = key_params->vdev_id;
	cmd->key_ix = key_params->key_idx;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(key_params->peer_mac, &cmd->peer_macaddr);
	if (key_params->unicast)
		cmd->key_flags |= PAIRWISE_USAGE;
	else
		cmd->key_flags |= GROUP_USAGE;

	switch (key_params->key_type) {
	case eSIR_ED_NONE:
		cmd->key_cipher = WMI_CIPHER_NONE;
		break;
	case eSIR_ED_WEP40:
	case eSIR_ED_WEP104:
		cmd->key_cipher = WMI_CIPHER_WEP;
		if (key_params->unicast &&
		    cmd->key_ix == key_params->def_key_idx) {
			WMA_LOGD("STA Mode: cmd->key_flags |= TX_USAGE");
			cmd->key_flags |= TX_USAGE;
		} else if ((mode == wlan_op_mode_ap) &&
			(cmd->key_ix == key_params->def_key_idx)) {
			WMA_LOGD("AP Mode: cmd->key_flags |= TX_USAGE");
			cmd->key_flags |= TX_USAGE;
		}
		break;
	case eSIR_ED_TKIP:
		cmd->key_txmic_len = WMA_TXMIC_LEN;
		cmd->key_rxmic_len = WMA_RXMIC_LEN;
		cmd->key_cipher = WMI_CIPHER_TKIP;
		break;
#ifdef FEATURE_WLAN_WAPI
#define WPI_IV_LEN 16
	case eSIR_ED_WPI:
	{
		/*initialize receive and transmit IV with default values */
		/* **Note: tx_iv must be sent in reverse** */
		unsigned char tx_iv[16] = { 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
					    0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
					    0x36, 0x5c, 0x36, 0x5c};
		unsigned char rx_iv[16] = { 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
					    0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
					    0x5c, 0x36, 0x5c, 0x37};
		if (mode == wlan_op_mode_ap) {
			/* Authenticator initializes the value of PN as
			 * 0x5C365C365C365C365C365C365C365C36 for MCastkey Update
			 */
			if (key_params->unicast)
				tx_iv[0] = 0x37;

			rx_iv[WPI_IV_LEN - 1] = 0x36;
		} else {
			if (!key_params->unicast)
				rx_iv[WPI_IV_LEN - 1] = 0x36;
		}

		cmd->key_txmic_len = WMA_TXMIC_LEN;
		cmd->key_rxmic_len = WMA_RXMIC_LEN;

		cdf_mem_copy(&cmd->wpi_key_rsc_counter, &rx_iv,
			     WPI_IV_LEN);
		cdf_mem_copy(&cmd->wpi_key_tsc_counter, &tx_iv,
			     WPI_IV_LEN);
		cmd->key_cipher = WMI_CIPHER_WAPI;
		break;
	}
#endif /* FEATURE_WLAN_WAPI */
	case eSIR_ED_CCMP:
		cmd->key_cipher = WMI_CIPHER_AES_CCM;
		break;
#ifdef WLAN_FEATURE_11W
	case eSIR_ED_AES_128_CMAC:
		cmd->key_cipher = WMI_CIPHER_AES_CMAC;
		break;
#endif /* WLAN_FEATURE_11W */
	default:
		/* TODO: MFP ? */
		WMA_LOGE("%s:Invalid encryption type:%d", __func__,
			 key_params->key_type);
		cdf_nbuf_free(buf);
		return NULL;
	}

	buf_ptr += sizeof(wmi_vdev_install_key_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       roundup(key_params->key_len, sizeof(uint32_t)));
	key_data = (A_UINT8 *) (buf_ptr + WMI_TLV_HDR_SIZE);
#ifdef BIG_ENDIAN_HOST
	{
		/* for big endian host, copy engine byte_swap is enabled
		 * But the key data content is in network byte order
		 * Need to byte swap the key data content - so when copy engine
		 * does byte_swap - target gets key_data content in the correct
		 * order.
		 */
		int8_t i;
		uint32_t *destp, *srcp;

		destp = (uint32_t *) key_data;
		srcp = (uint32_t *) key_params->key_data;
		for (i = 0;
		     i < roundup(key_params->key_len, sizeof(uint32_t)) / 4;
		     i++) {
			*destp = le32_to_cpu(*srcp);
			destp++;
			srcp++;
		}
	}
#else
	cdf_mem_copy((void *)key_data,
		     (const void *)key_params->key_data, key_params->key_len);
#endif /* BIG_ENDIAN_HOST */
	cmd->key_len = key_params->key_len;

#ifdef WLAN_FEATURE_11W
	if (key_params->key_type == eSIR_ED_AES_128_CMAC) {
		iface = &wma_handle->interfaces[key_params->vdev_id];
		if (iface) {
			iface->key.key_length = key_params->key_len;
			cdf_mem_copy(iface->key.key,
				     (const void *)key_params->key_data,
				     iface->key.key_length);
			if ((cmd->key_ix == WMA_IGTK_KEY_INDEX_4) ||
			    (cmd->key_ix == WMA_IGTK_KEY_INDEX_5))
				cdf_mem_zero(iface->key.key_id[cmd->key_ix -
						    WMA_IGTK_KEY_INDEX_4].ipn,
					     CMAC_IPN_LEN);
		}
	}
#endif /* WLAN_FEATURE_11W */

	WMA_LOGD("Key setup : vdev_id %d key_idx %d key_type %d key_len %d"
		 " unicast %d peer_mac %pM def_key_idx %d", key_params->vdev_id,
		 key_params->key_idx, key_params->key_type, key_params->key_len,
		 key_params->unicast, key_params->peer_mac,
		 key_params->def_key_idx);

	return buf;
}

/**
 * wma_set_bsskey() - set encryption key to fw.
 * @wma_handle: wma handle
 * @key_info: key info
 *
 * Return: none
 */
void wma_set_bsskey(tp_wma_handle wma_handle, tpSetBssKeyParams key_info)
{
	struct wma_set_key_params key_params;
	wmi_buf_t buf;
	int32_t status;
	uint32_t len = 0, i;
	uint32_t def_key_idx = 0;
	ol_txrx_vdev_handle txrx_vdev;

	WMA_LOGD("BSS key setup");
	txrx_vdev = wma_find_vdev_by_id(wma_handle, key_info->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	/*
	 * For IBSS, WMI expects the BSS key to be set per peer key
	 * So cache the BSS key in the wma_handle and re-use it when the
	 * STA key is been setup for a peer
	 */
	if (wlan_op_mode_ibss == txrx_vdev->opmode) {
		key_info->status = QDF_STATUS_SUCCESS;
		if (wma_handle->ibss_started > 0)
			goto out;
		WMA_LOGD("Caching IBSS Key");
		cdf_mem_copy(&wma_handle->ibsskey_info, key_info,
			     sizeof(tSetBssKeyParams));
	}

	cdf_mem_set(&key_params, sizeof(key_params), 0);
	key_params.vdev_id = key_info->smesessionId;
	key_params.key_type = key_info->encType;
	key_params.singl_tid_rc = key_info->singleTidRc;
	key_params.unicast = false;
	if (txrx_vdev->opmode == wlan_op_mode_sta) {
		cdf_mem_copy(key_params.peer_mac,
			wma_handle->interfaces[key_info->smesessionId].bssid,
			IEEE80211_ADDR_LEN);
	} else {
		/* vdev mac address will be passed for all other modes */
		cdf_mem_copy(key_params.peer_mac, txrx_vdev->mac_addr.raw,
			     IEEE80211_ADDR_LEN);
		WMA_LOGA("BSS Key setup with vdev_mac %pM\n",
			 txrx_vdev->mac_addr.raw);
	}

	if (key_info->numKeys == 0 &&
	    (key_info->encType == eSIR_ED_WEP40 ||
	     key_info->encType == eSIR_ED_WEP104)) {
		wma_read_cfg_wepkey(wma_handle, key_info->key,
				    &def_key_idx, &key_info->numKeys);
	}

	for (i = 0; i < key_info->numKeys; i++) {
		if (key_params.key_type != eSIR_ED_NONE &&
		    !key_info->key[i].keyLength)
			continue;
		if (key_info->encType == eSIR_ED_WPI) {
			key_params.key_idx = key_info->key[i].keyId;
			key_params.def_key_idx = key_info->key[i].keyId;
		} else
			key_params.key_idx = key_info->key[i].keyId;

		key_params.key_len = key_info->key[i].keyLength;
		if (key_info->encType == eSIR_ED_TKIP) {
			cdf_mem_copy(key_params.key_data,
				     key_info->key[i].key, 16);
			cdf_mem_copy(&key_params.key_data[16],
				     &key_info->key[i].key[24], 8);
			cdf_mem_copy(&key_params.key_data[24],
				     &key_info->key[i].key[16], 8);
		} else
			cdf_mem_copy((void *)key_params.key_data,
				     (const void *)key_info->key[i].key,
				     key_info->key[i].keyLength);

		WMA_LOGD("%s: bss key[%d] length %d", __func__, i,
			 key_info->key[i].keyLength);

		buf = wma_setup_install_key_cmd(wma_handle, &key_params, &len,
						txrx_vdev->opmode);
		if (!buf) {
			WMA_LOGE("%s:Failed to setup install key buf",
				 __func__);
			key_info->status = QDF_STATUS_E_NOMEM;
			goto out;
		}

		status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
					      WMI_VDEV_INSTALL_KEY_CMDID);
		if (status) {
			cdf_nbuf_free(buf);
			WMA_LOGE("%s:Failed to send install key command",
				 __func__);
			key_info->status = QDF_STATUS_E_FAILURE;
			goto out;
		}
	}

	wma_handle->ibss_started++;
	/* TODO: Should we wait till we get HTT_T2H_MSG_TYPE_SEC_IND? */
	key_info->status = QDF_STATUS_SUCCESS;

out:
	wma_send_msg(wma_handle, WMA_SET_BSSKEY_RSP, (void *)key_info, 0);
}

#ifdef QCA_IBSS_SUPPORT
/**
 * wma_calc_ibss_heart_beat_timer() - calculate IBSS heart beat timer
 * @peer_num: number of peers
 *
 * Return: heart beat timer value
 */
static uint16_t wma_calc_ibss_heart_beat_timer(int16_t peer_num)
{
	/* heart beat timer value look-up table */
	/* entry index : (the number of currently connected peers) - 1
	   entry value : the heart time threshold value in seconds for
	   detecting ibss peer departure */
	static const uint16_t heart_beat_timer[MAX_IBSS_PEERS] = {
		4, 4, 4, 4, 4, 4, 4, 4,
		8, 8, 8, 8, 8, 8, 8, 8,
		12, 12, 12, 12, 12, 12, 12, 12,
		16, 16, 16, 16, 16, 16, 16, 16
	};

	if (peer_num < 1 || peer_num > MAX_IBSS_PEERS)
		return 0;

	return heart_beat_timer[peer_num - 1];

}

/**
 * wma_adjust_ibss_heart_beat_timer() - set ibss heart beat timer in fw.
 * @wma: wma handle
 * @vdev_id: vdev id
 * @peer_num_delta: peer number delta value
 *
 * Return: none
 */
void wma_adjust_ibss_heart_beat_timer(tp_wma_handle wma,
				      uint8_t vdev_id,
				      int8_t peer_num_delta)
{
	ol_txrx_vdev_handle vdev;
	int16_t new_peer_num;
	uint16_t new_timer_value_sec;
	uint32_t new_timer_value_ms;

	if (peer_num_delta != 1 && peer_num_delta != -1) {
		WMA_LOGE("Invalid peer_num_delta value %d", peer_num_delta);
		return;
	}

	vdev = wma_find_vdev_by_id(wma, vdev_id);
	if (!vdev) {
		WMA_LOGE("vdev not found : vdev_id %d", vdev_id);
		return;
	}

	new_peer_num = vdev->ibss_peer_num + peer_num_delta;
	if (new_peer_num > MAX_IBSS_PEERS || new_peer_num < 0) {
		WMA_LOGE("new peer num %d out of valid boundary", new_peer_num);
		return;
	}

	/* adjust peer numbers */
	vdev->ibss_peer_num = new_peer_num;

	/* reset timer value if all peers departed */
	if (new_peer_num == 0) {
		vdev->ibss_peer_heart_beat_timer = 0;
		return;
	}

	/* calculate new timer value */
	new_timer_value_sec = wma_calc_ibss_heart_beat_timer(new_peer_num);
	if (new_timer_value_sec == 0) {
		WMA_LOGE("timer value %d is invalid for peer number %d",
			 new_timer_value_sec, new_peer_num);
		return;
	}
	if (new_timer_value_sec == vdev->ibss_peer_heart_beat_timer) {
		WMA_LOGD("timer value %d stays same, no need to notify target",
			 new_timer_value_sec);
		return;
	}

	/* send new timer value to target */
	vdev->ibss_peer_heart_beat_timer = new_timer_value_sec;

	new_timer_value_ms = ((uint32_t) new_timer_value_sec) * 1000;

	if (wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					    WMI_VDEV_PARAM_IBSS_MAX_BCN_LOST_MS,
					    new_timer_value_ms)) {
		WMA_LOGE("Failed to set IBSS link monitoring timer value");
		return;
	}

	WMA_LOGD("Set IBSS link monitor timer: peer_num = %d timer_value = %d",
		 new_peer_num, new_timer_value_ms);
}

#endif /* QCA_IBSS_SUPPORT */
/**
 * wma_set_ibsskey_helper() - cached IBSS key in wma handle
 * @wma_handle: wma handle
 * @key_info: set bss key info
 * @peerMacAddr: peer mac address
 *
 * Return: none
 */
static void wma_set_ibsskey_helper(tp_wma_handle wma_handle,
				   tpSetBssKeyParams key_info,
				   struct qdf_mac_addr peer_macaddr)
{
	struct wma_set_key_params key_params;
	wmi_buf_t buf;
	int32_t status;
	uint32_t len = 0, i;
	uint32_t def_key_idx = 0;
	ol_txrx_vdev_handle txrx_vdev;

	WMA_LOGD("BSS key setup for peer");
	txrx_vdev = wma_find_vdev_by_id(wma_handle, key_info->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		return;
	}

	cdf_mem_set(&key_params, sizeof(key_params), 0);
	key_params.vdev_id = key_info->smesessionId;
	key_params.key_type = key_info->encType;
	key_params.singl_tid_rc = key_info->singleTidRc;
	key_params.unicast = false;
	ASSERT(wlan_op_mode_ibss == txrx_vdev->opmode);

	cdf_mem_copy(key_params.peer_mac, peer_macaddr.bytes,
			IEEE80211_ADDR_LEN);

	if (key_info->numKeys == 0 &&
	    (key_info->encType == eSIR_ED_WEP40 ||
	     key_info->encType == eSIR_ED_WEP104)) {
		wma_read_cfg_wepkey(wma_handle, key_info->key,
				    &def_key_idx, &key_info->numKeys);
	}

	for (i = 0; i < key_info->numKeys; i++) {
		if (key_params.key_type != eSIR_ED_NONE &&
		    !key_info->key[i].keyLength)
			continue;
		key_params.key_idx = key_info->key[i].keyId;
		key_params.key_len = key_info->key[i].keyLength;
		if (key_info->encType == eSIR_ED_TKIP) {
			cdf_mem_copy(key_params.key_data,
				     key_info->key[i].key, 16);
			cdf_mem_copy(&key_params.key_data[16],
				     &key_info->key[i].key[24], 8);
			cdf_mem_copy(&key_params.key_data[24],
				     &key_info->key[i].key[16], 8);
		} else
			cdf_mem_copy((void *)key_params.key_data,
				     (const void *)key_info->key[i].key,
				     key_info->key[i].keyLength);

		WMA_LOGD("%s: peer bcast key[%d] length %d", __func__, i,
			 key_info->key[i].keyLength);

		buf = wma_setup_install_key_cmd(wma_handle, &key_params, &len,
						txrx_vdev->opmode);
		if (!buf) {
			WMA_LOGE("%s:Failed to setup install key buf",
				 __func__);
			return;
		}

		status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
					      WMI_VDEV_INSTALL_KEY_CMDID);
		if (status) {
			cdf_nbuf_free(buf);
			WMA_LOGE("%s:Failed to send install key command",
				 __func__);
		}
	}
}

/**
 * wma_set_stakey() - set encryption key
 * @wma_handle: wma handle
 * @key_info: station key info
 *
 * This function sets encryption key for WEP/WPA/WPA2
 * encryption mode in firmware and send response to upper layer.
 *
 * Return: none
 */
void wma_set_stakey(tp_wma_handle wma_handle, tpSetStaKeyParams key_info)
{
	wmi_buf_t buf;
	int32_t status, i;
	uint32_t len = 0;
	ol_txrx_pdev_handle txrx_pdev;
	ol_txrx_vdev_handle txrx_vdev;
	struct ol_txrx_peer_t *peer;
	uint8_t num_keys = 0, peer_id;
	struct wma_set_key_params key_params;
	uint32_t def_key_idx = 0;

	WMA_LOGD("STA key setup");

	/* Get the txRx Pdev handle */
	txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!txrx_pdev) {
		WMA_LOGE("%s:Invalid txrx pdev handle", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	peer = ol_txrx_find_peer_by_addr(txrx_pdev,
					 key_info->peer_macaddr.bytes,
					 &peer_id);
	if (!peer) {
		WMA_LOGE("%s:Invalid peer for key setting", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	txrx_vdev = wma_find_vdev_by_id(wma_handle, key_info->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:TxRx Vdev Handle is NULL", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	if (key_info->defWEPIdx == WMA_INVALID_KEY_IDX &&
	    (key_info->encType == eSIR_ED_WEP40 ||
	     key_info->encType == eSIR_ED_WEP104) &&
	    txrx_vdev->opmode != wlan_op_mode_ap) {
		wma_read_cfg_wepkey(wma_handle, key_info->key,
				    &def_key_idx, &num_keys);
		key_info->defWEPIdx = def_key_idx;
	} else {
		num_keys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
		if (key_info->encType != eSIR_ED_NONE) {
			for (i = 0; i < num_keys; i++) {
				if (key_info->key[i].keyDirection ==
				    eSIR_TX_DEFAULT) {
					key_info->defWEPIdx = i;
					break;
				}
			}
		}
	}
	cdf_mem_set(&key_params, sizeof(key_params), 0);
	key_params.vdev_id = key_info->smesessionId;
	key_params.key_type = key_info->encType;
	key_params.singl_tid_rc = key_info->singleTidRc;
	key_params.unicast = true;
	key_params.def_key_idx = key_info->defWEPIdx;
	cdf_mem_copy((void *)key_params.peer_mac,
		     (const void *)key_info->peer_macaddr.bytes,
		     IEEE80211_ADDR_LEN);
	for (i = 0; i < num_keys; i++) {
		if (key_params.key_type != eSIR_ED_NONE &&
		    !key_info->key[i].keyLength)
			continue;
		if (key_info->encType == eSIR_ED_TKIP) {
			cdf_mem_copy(key_params.key_data,
				     key_info->key[i].key, 16);
			cdf_mem_copy(&key_params.key_data[16],
				     &key_info->key[i].key[24], 8);
			cdf_mem_copy(&key_params.key_data[24],
				     &key_info->key[i].key[16], 8);
		} else
			cdf_mem_copy(key_params.key_data, key_info->key[i].key,
				     key_info->key[i].keyLength);
		if (key_info->encType == eSIR_ED_WPI) {
			key_params.key_idx = key_info->key[i].keyId;
			key_params.def_key_idx = key_info->key[i].keyId;
		} else
			key_params.key_idx = i;

		key_params.key_len = key_info->key[i].keyLength;
		buf = wma_setup_install_key_cmd(wma_handle, &key_params, &len,
						txrx_vdev->opmode);
		if (!buf) {
			WMA_LOGE("%s:Failed to setup install key buf",
				 __func__);
			key_info->status = QDF_STATUS_E_NOMEM;
			goto out;
		}

		WMA_LOGD("%s: peer unicast key[%d] %d ", __func__, i,
			 key_info->key[i].keyLength);

		status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
					      WMI_VDEV_INSTALL_KEY_CMDID);
		if (status) {
			cdf_nbuf_free(buf);
			WMA_LOGE("%s:Failed to send install key command",
				 __func__);
			key_info->status = QDF_STATUS_E_FAILURE;
			goto out;
		}
	}

	/* In IBSS mode, set the BSS KEY for this peer
	 * BSS key is supposed to be cache into wma_handle
	 */
	if (wlan_op_mode_ibss == txrx_vdev->opmode) {
		wma_set_ibsskey_helper(wma_handle, &wma_handle->ibsskey_info,
				       key_info->peer_macaddr);
	}

	/* TODO: Should we wait till we get HTT_T2H_MSG_TYPE_SEC_IND? */
	key_info->status = QDF_STATUS_SUCCESS;
out:
	if (key_info->sendRsp)
		wma_send_msg(wma_handle, WMA_SET_STAKEY_RSP, (void *)key_info,
			     0);
}

/**
 * wma_process_update_edca_param_req() - update EDCA params
 * @handle: wma handle
 * @edca_params: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: CDF Status
 */
QDF_STATUS wma_process_update_edca_param_req(WMA_HANDLE handle,
					     tEdcaParams *edca_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	wmi_vdev_set_wmm_params_cmd_fixed_param *cmd;
	wmi_wmm_vparams *wmm_param;
	tSirMacEdcaParamRecord *edca_record;
	int ac;
	int len = sizeof(*cmd);
	ol_txrx_pdev_handle pdev;
	struct ol_tx_wmm_param_t ol_tx_wmm_param;

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);

	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_set_wmm_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_wmm_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_wmm_params_cmd_fixed_param));
	cmd->vdev_id = edca_params->bssIdx;

	for (ac = 0; ac < WME_NUM_AC; ac++) {
		wmm_param = (wmi_wmm_vparams *) (&cmd->wmm_params[ac]);
		WMITLV_SET_HDR(&wmm_param->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vdev_set_wmm_params_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wmm_vparams));
		switch (ac) {
		case WME_AC_BE:
			edca_record = &edca_params->acbe;
			break;
		case WME_AC_BK:
			edca_record = &edca_params->acbk;
			break;
		case WME_AC_VI:
			edca_record = &edca_params->acvi;
			break;
		case WME_AC_VO:
			edca_record = &edca_params->acvo;
			break;
		default:
			goto fail;
		}

		wma_update_edca_params_for_ac(edca_record, wmm_param, ac);

		ol_tx_wmm_param.ac[ac].aifs = wmm_param->aifs;
		ol_tx_wmm_param.ac[ac].cwmin = wmm_param->cwmin;
		ol_tx_wmm_param.ac[ac].cwmax = wmm_param->cwmax;
	}

	if (wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				 WMI_VDEV_SET_WMM_PARAMS_CMDID))
		goto fail;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (pdev)
		ol_txrx_set_wmm_param(pdev, ol_tx_wmm_param);
	else
		CDF_ASSERT(0);

	return QDF_STATUS_SUCCESS;

fail:
	wmi_buf_free(buf);
	WMA_LOGE("%s: Failed to set WMM Paremeters", __func__);
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_probe_rsp_tmpl_send() - send probe response template to fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @probe_rsp_info: probe response info
 *
 * Return: 0 for success or error code
 */
static int wmi_unified_probe_rsp_tmpl_send(tp_wma_handle wma,
					   uint8_t vdev_id,
					   tpSendProbeRespParams probe_rsp_info)
{
	wmi_prb_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint32_t tmpl_len, tmpl_len_aligned, wmi_buf_len;
	uint8_t *frm, *buf_ptr;
	int ret;
	uint64_t adjusted_tsf_le;
	struct ieee80211_frame *wh;

	WMA_LOGD(FL("Send probe response template for vdev %d"), vdev_id);

	frm = probe_rsp_info->pProbeRespTemplate;
	tmpl_len = probe_rsp_info->probeRespTemplateLen;
	tmpl_len_aligned = roundup(tmpl_len, sizeof(A_UINT32));
	/*
	 * Make the TSF offset negative so probe response in the same
	 * staggered batch have the same TSF.
	 */
	adjusted_tsf_le = cpu_to_le64(0ULL -
				      wma->interfaces[vdev_id].tsfadjust);
	/* Update the timstamp in the probe response buffer with adjusted TSF */
	wh = (struct ieee80211_frame *)frm;
	A_MEMCPY(&wh[1], &adjusted_tsf_le, sizeof(adjusted_tsf_le));

	wmi_buf_len = sizeof(wmi_prb_tmpl_cmd_fixed_param) +
			sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
			tmpl_len_aligned;

	if (wmi_buf_len > BEACON_TX_BUFFER_SIZE) {
		WMA_LOGE(FL("wmi_buf_len: %d > %d. Can't send wmi cmd"),
		wmi_buf_len, BEACON_TX_BUFFER_SIZE);
		return -EINVAL;
	}

	wmi_buf = wmi_buf_alloc(wma->wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMA_LOGE(FL("wmi_buf_alloc failed"));
		return -ENOMEM;
	}

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
	cdf_mem_copy(buf_ptr, frm, tmpl_len);

	ret = wmi_unified_cmd_send(wma->wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_PRB_TMPL_CMDID);
	if (ret) {
		WMA_LOGE(FL("Failed to send PRB RSP tmpl: %d"), ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

/**
 * wmi_unified_bcn_tmpl_send() - send beacon template to fw
 * @wma:wma handle
 * @vdev_id: vdev id
 * @bcn_info: beacon info
 * @bytes_to_strip: bytes to strip
 *
 * Return: 0 for success or error code
 */
static int wmi_unified_bcn_tmpl_send(tp_wma_handle wma,
				     uint8_t vdev_id,
				     tpSendbeaconParams bcn_info,
				     uint8_t bytes_to_strip)
{
	wmi_bcn_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint32_t tmpl_len, tmpl_len_aligned, wmi_buf_len;
	uint8_t *frm, *buf_ptr;
	int ret;
	uint8_t *p2p_ie;
	uint16_t p2p_ie_len = 0;
	uint64_t adjusted_tsf_le;
	struct ieee80211_frame *wh;

	WMA_LOGD("Send beacon template for vdev %d", vdev_id);

	if (bcn_info->p2pIeOffset) {
		p2p_ie = bcn_info->beacon + bcn_info->p2pIeOffset;
		p2p_ie_len = (uint16_t) p2p_ie[1] + 2;
	}

	/*
	 * XXX: The first byte of beacon buffer contains beacon length
	 * only when UMAC in sending the beacon template. In othercases
	 * (ex: from tbtt update) beacon length is read from beacon
	 * information.
	 */
	if (bytes_to_strip)
		tmpl_len = *(uint32_t *) &bcn_info->beacon[0];
	else
		tmpl_len = bcn_info->beaconLength;
	if (p2p_ie_len) {
		tmpl_len -= (uint32_t) p2p_ie_len;
	}

	frm = bcn_info->beacon + bytes_to_strip;
	tmpl_len_aligned = roundup(tmpl_len, sizeof(A_UINT32));
	/*
	 * Make the TSF offset negative so beacons in the same
	 * staggered batch have the same TSF.
	 */
	adjusted_tsf_le = cpu_to_le64(0ULL -
				      wma->interfaces[vdev_id].tsfadjust);
	/* Update the timstamp in the beacon buffer with adjusted TSF */
	wh = (struct ieee80211_frame *)frm;
	A_MEMCPY(&wh[1], &adjusted_tsf_le, sizeof(adjusted_tsf_le));

	wmi_buf_len = sizeof(wmi_bcn_tmpl_cmd_fixed_param) +
		      sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
		      tmpl_len_aligned;

	wmi_buf = wmi_buf_alloc(wma->wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMA_LOGE("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_bcn_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_tmpl_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->tim_ie_offset = bcn_info->timIeOffset - bytes_to_strip;
	cmd->buf_len = tmpl_len;
	buf_ptr += sizeof(wmi_bcn_tmpl_cmd_fixed_param);

	bcn_prb_info = (wmi_bcn_prb_info *) buf_ptr;
	WMITLV_SET_HDR(&bcn_prb_info->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_prb_info,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_prb_info));
	bcn_prb_info->caps = 0;
	bcn_prb_info->erp = 0;
	buf_ptr += sizeof(wmi_bcn_prb_info);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, tmpl_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cdf_mem_copy(buf_ptr, frm, tmpl_len);

	ret = wmi_unified_cmd_send(wma->wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_BCN_TMPL_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send bcn tmpl: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

/**
 * wma_store_bcn_tmpl() - store beacon template
 * @wma: wma handle
 * @vdev_id: vdev id
 * @bcn_info: beacon params
 *
 * This function stores beacon template locally.
 * This will send to target on the reception of
 * SWBA event.
 *
 * Return: CDF status
 */
QDF_STATUS wma_store_bcn_tmpl(tp_wma_handle wma, uint8_t vdev_id,
			      tpSendbeaconParams bcn_info)
{
	struct beacon_info *bcn;
	uint32_t len;
	uint8_t *bcn_payload;
	struct beacon_tim_ie *tim_ie;

	bcn = wma->interfaces[vdev_id].beacon;
	if (!bcn || !bcn->buf) {
		WMA_LOGE("%s: Memory is not allocated to hold bcn template",
			 __func__);
		return QDF_STATUS_E_INVAL;
	}

	len = *(u32 *) &bcn_info->beacon[0];
	if (len > WMA_BCN_BUF_MAX_SIZE) {
		WMA_LOGE("%s: Received beacon len %d exceeding max limit %d",
			 __func__, len, WMA_BCN_BUF_MAX_SIZE);
		return QDF_STATUS_E_INVAL;
	}
	WMA_LOGD("%s: Storing received beacon template buf to local buffer",
		 __func__);
	cdf_spin_lock_bh(&bcn->lock);

	/*
	 * Copy received beacon template content in local buffer.
	 * this will be send to target on the reception of SWBA
	 * event from target.
	 */
	cdf_nbuf_trim_tail(bcn->buf, cdf_nbuf_len(bcn->buf));
	memcpy(cdf_nbuf_data(bcn->buf),
	       bcn_info->beacon + 4 /* Exclude beacon length field */,
	       len);
	if (bcn_info->timIeOffset > 3) {
		bcn->tim_ie_offset = bcn_info->timIeOffset - 4;
	} else {
		bcn->tim_ie_offset = bcn_info->timIeOffset;
	}

	if (bcn_info->p2pIeOffset > 3) {
		bcn->p2p_ie_offset = bcn_info->p2pIeOffset - 4;
	} else {
		bcn->p2p_ie_offset = bcn_info->p2pIeOffset;
	}
	bcn_payload = cdf_nbuf_data(bcn->buf);
	if (bcn->tim_ie_offset) {
		tim_ie =
			(struct beacon_tim_ie *)(&bcn_payload[bcn->tim_ie_offset]);
		/*
		 * Intial Value of bcn->dtim_count will be 0.
		 * But if the beacon gets updated then current dtim
		 * count will be restored
		 */
		tim_ie->dtim_count = bcn->dtim_count;
		tim_ie->tim_bitctl = 0;
	}

	cdf_nbuf_put_tail(bcn->buf, len);
	bcn->len = len;

	cdf_spin_unlock_bh(&bcn->lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_tbttoffset_update_event_handler() - tbtt offset update handler
 * @handle: wma handle
 * @event: event buffer
 * @len: data length
 *
 * Return: 0 for success or error code
 */
int wma_tbttoffset_update_event_handler(void *handle, uint8_t *event,
					       uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_event_fixed_param *tbtt_offset_event;
	struct wma_txrx_node *intf;
	struct beacon_info *bcn;
	tSendbeaconParams bcn_info;
	uint32_t *adjusted_tsf = NULL;
	uint32_t if_id = 0, vdev_map;

	if (!wma) {
		WMA_LOGE("Invalid wma handle");
		return -EINVAL;
	}

	param_buf = (WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid tbtt update event buffer");
		return -EINVAL;
	}

	tbtt_offset_event = param_buf->fixed_param;
	intf = wma->interfaces;
	vdev_map = tbtt_offset_event->vdev_map;
	adjusted_tsf = param_buf->tbttoffset_list;
	if (!adjusted_tsf) {
		WMA_LOGE("%s: Invalid adjusted_tsf", __func__);
		return -EINVAL;
	}

	for (; (vdev_map); vdev_map >>= 1, if_id++) {
		if (!(vdev_map & 0x1) || (!(intf[if_id].handle)))
			continue;

		bcn = intf[if_id].beacon;
		if (!bcn) {
			WMA_LOGE("%s: Invalid beacon", __func__);
			return -EINVAL;
		}
		if (!bcn->buf) {
			WMA_LOGE("%s: Invalid beacon buffer", __func__);
			return -EINVAL;
		}
		/* Save the adjusted TSF */
		intf[if_id].tsfadjust = adjusted_tsf[if_id];

		cdf_spin_lock_bh(&bcn->lock);
		cdf_mem_zero(&bcn_info, sizeof(bcn_info));
		bcn_info.beacon = cdf_nbuf_data(bcn->buf);
		bcn_info.p2pIeOffset = bcn->p2p_ie_offset;
		bcn_info.beaconLength = bcn->len;
		bcn_info.timIeOffset = bcn->tim_ie_offset;
		cdf_spin_unlock_bh(&bcn->lock);

		/* Update beacon template in firmware */
		wmi_unified_bcn_tmpl_send(wma, if_id, &bcn_info, 0);
	}
	return 0;
}

/**
 * wma_p2p_go_set_beacon_ie() - set beacon IE for p2p go
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @p2pIe: p2p IE
 *
 * Return: 0 for success or error code
 */
static int wma_p2p_go_set_beacon_ie(t_wma_handle *wma_handle,
				    A_UINT32 vdev_id, uint8_t *p2pIe)
{
	int ret;
	wmi_p2p_go_set_beacon_ie_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t ie_len, ie_len_aligned, wmi_buf_len;
	uint8_t *buf_ptr;

	ie_len = (uint32_t) (p2pIe[1] + 2);

	/* More than one P2P IE may be included in a single frame.
	   If multiple P2P IEs are present, the complete P2P attribute
	   data consists of the concatenation of the P2P Attribute
	   fields of the P2P IEs. The P2P Attributes field of each
	   P2P IE may be any length up to the maximum (251 octets).
	   In this case host sends one P2P IE to firmware so the length
	   should not exceed more than 251 bytes
	 */
	if (ie_len > 251) {
		WMA_LOGE("%s : invalid p2p ie length %u", __func__, ie_len);
		return -EINVAL;
	}

	ie_len_aligned = roundup(ie_len, sizeof(A_UINT32));

	wmi_buf_len =
		sizeof(wmi_p2p_go_set_beacon_ie_fixed_param) + ie_len_aligned +
		WMI_TLV_HDR_SIZE;

	wmi_buf = wmi_buf_alloc(wma_handle->wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMA_LOGE("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}

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
	cdf_mem_copy(buf_ptr, p2pIe, ie_len);

	WMA_LOGI("%s: Sending WMI_P2P_GO_SET_BEACON_IE", __func__);

	ret = wmi_unified_cmd_send(wma_handle->wmi_handle,
				   wmi_buf, wmi_buf_len,
				   WMI_P2P_GO_SET_BEACON_IE);
	if (ret) {
		WMA_LOGE("Failed to send bcn tmpl: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	WMA_LOGI("%s: Successfully sent WMI_P2P_GO_SET_BEACON_IE", __func__);
	return ret;
}

/**
 * wma_send_probe_rsp_tmpl() - send probe resp template
 * @wma: wma handle
 * @probe_rsp_info: probe response info
 *
 * This funciton sends probe response template to fw which
 * firmware will use in case of probe response offload.
 *
 * Return: none
 */
void wma_send_probe_rsp_tmpl(tp_wma_handle wma,
				    tpSendProbeRespParams probe_rsp_info)
{
	ol_txrx_vdev_handle vdev;
	uint8_t vdev_id;
	tpAniProbeRspStruct probe_rsp;

	if (!probe_rsp_info) {
		WMA_LOGE(FL("probe_rsp_info is NULL"));
		return;
	}

	probe_rsp = (tpAniProbeRspStruct) (probe_rsp_info->pProbeRespTemplate);
	if (!probe_rsp) {
		WMA_LOGE(FL("probe_rsp is NULL"));
		return;
	}

	vdev = wma_find_vdev_by_addr(wma, probe_rsp->macHdr.sa, &vdev_id);
	if (!vdev) {
		WMA_LOGE(FL("failed to get vdev handle"));
		return;
	}

	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				   WMI_SERVICE_BEACON_OFFLOAD)) {
		WMA_LOGI("Beacon Offload Enabled Sending Unified command");
		if (wmi_unified_probe_rsp_tmpl_send(wma, vdev_id,
						    probe_rsp_info) < 0) {
			WMA_LOGE(FL("wmi_unified_probe_rsp_tmpl_send Failed "));
			return;
		}
	}
}

/**
 * wma_send_beacon() - send beacon template
 * @wma: wma handle
 * @bcn_info: beacon info
 *
 * This funciton store beacon template locally and
 * update keep alive parameters
 *
 * Return: none
 */
void wma_send_beacon(tp_wma_handle wma, tpSendbeaconParams bcn_info)
{
	ol_txrx_vdev_handle vdev;
	uint8_t vdev_id;
	QDF_STATUS status;
	uint8_t *p2p_ie;
	tpAniBeaconStruct beacon;

	beacon = (tpAniBeaconStruct) (bcn_info->beacon);
	vdev = wma_find_vdev_by_addr(wma, beacon->macHdr.sa, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s : failed to get vdev handle", __func__);
		return;
	}

	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				   WMI_SERVICE_BEACON_OFFLOAD)) {
		WMA_LOGI("Beacon Offload Enabled Sending Unified command");
		if (wmi_unified_bcn_tmpl_send(wma, vdev_id, bcn_info, 4) < 0) {
			WMA_LOGE("%s : wmi_unified_bcn_tmpl_send Failed ",
				 __func__);
			return;
		}

		if (bcn_info->p2pIeOffset) {
			p2p_ie = bcn_info->beacon + bcn_info->p2pIeOffset;
			WMA_LOGI
				(" %s: p2pIe is present - vdev_id %hu, p2p_ie = %p, p2p ie len = %hu",
				__func__, vdev_id, p2p_ie, p2p_ie[1]);
			if (wma_p2p_go_set_beacon_ie(wma, vdev_id, p2p_ie) < 0) {
				WMA_LOGE
					("%s : wmi_unified_bcn_tmpl_send Failed ",
					__func__);
				return;
			}
		}
	}
	status = wma_store_bcn_tmpl(wma, vdev_id, bcn_info);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s : wma_store_bcn_tmpl Failed", __func__);
		return;
	}
	if (!wma->interfaces[vdev_id].vdev_up) {
		if (wmi_unified_vdev_up_send(wma->wmi_handle, vdev_id, 0,
					     bcn_info->bssId) < 0) {
			WMA_LOGE("%s : failed to send vdev up", __func__);
			return;
		}
		wma->interfaces[vdev_id].vdev_up = true;
		wma_set_sap_keepalive(wma, vdev_id);
	}
}

/**
 * wma_set_keepalive_req() - send keep alive request to fw
 * @wma: wma handle
 * @keepalive: keep alive parameters
 *
 * Return: none
 */
void wma_set_keepalive_req(tp_wma_handle wma,
			   tSirKeepAliveReq *keepalive)
{
	WMA_LOGD("KEEPALIVE:PacketType:%d", keepalive->packetType);
	wma_set_sta_keep_alive(wma, keepalive->sessionId,
			       keepalive->packetType,
			       keepalive->timePeriod,
			       keepalive->hostIpv4Addr,
			       keepalive->destIpv4Addr,
			       keepalive->dest_macaddr.bytes);

	cdf_mem_free(keepalive);
}

/**
 * wma_beacon_miss_handler() - beacon miss event handler
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * This function send beacon miss indication to upper layers.
 *
 * Return: none
 */
void wma_beacon_miss_handler(tp_wma_handle wma, uint32_t vdev_id)
{
	tSirSmeMissedBeaconInd *beacon_miss_ind;

	beacon_miss_ind = (tSirSmeMissedBeaconInd *) cdf_mem_malloc
				  (sizeof(tSirSmeMissedBeaconInd));

	if (NULL == beacon_miss_ind) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return;
	}
	beacon_miss_ind->messageType = WMA_MISSED_BEACON_IND;
	beacon_miss_ind->length = sizeof(tSirSmeMissedBeaconInd);
	beacon_miss_ind->bssIdx = vdev_id;

	wma_send_msg(wma, WMA_MISSED_BEACON_IND, (void *)beacon_miss_ind, 0);
}

/**
 * wma_mgmt_tx_completion_handler() - wma mgmt Tx completion event handler
 * @handle: wma handle
 * @cmpl_event_params: completion event handler data
 * @len: length of @cmpl_event_params
 *
 * Return: 0 on success; error number otherwise
 */

int wma_mgmt_tx_completion_handler(void *handle, uint8_t *cmpl_event_params,
				   uint32_t len)
{
	tp_wma_handle wma_handle = (tp_wma_handle)handle;
	WMI_MGMT_TX_COMPLETION_EVENTID_param_tlvs *param_buf;
	wmi_mgmt_tx_compl_event_fixed_param	*cmpl_params;
	struct wmi_desc_t *wmi_desc;

	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	param_buf = (WMI_MGMT_TX_COMPLETION_EVENTID_param_tlvs *)
		cmpl_event_params;
	if (!param_buf && !wma_handle) {
		WMA_LOGE("%s: Invalid mgmt Tx completion event", __func__);
		return -EINVAL;
	}
	cmpl_params = param_buf->fixed_param;

	WMA_LOGI("%s: status:%d wmi_desc_id:%d", __func__, cmpl_params->status,
		 cmpl_params->desc_id);

	wmi_desc = (struct wmi_desc_t *)
		(&wma_handle->wmi_desc_pool.array[cmpl_params->desc_id]);

	if (!wmi_desc) {
		WMA_LOGE("%s: Invalid wmi desc", __func__);
		return -EINVAL;
	}

	if (wmi_desc->nbuf)
		cdf_nbuf_unmap_single(pdev->osdev, wmi_desc->nbuf,
				      QDF_DMA_TO_DEVICE);
	if (wmi_desc->tx_cmpl_cb)
		wmi_desc->tx_cmpl_cb(wma_handle->mac_context,
				       wmi_desc->nbuf, 1);

	if (wmi_desc->ota_post_proc_cb)
		wmi_desc->ota_post_proc_cb((tpAniSirGlobal)
					     wma_handle->mac_context,
					     cmpl_params->status);

	wmi_desc_put(wma_handle, wmi_desc);

	return 0;
}

/**
 * wma_process_update_opmode() - process update VHT opmode cmd from UMAC
 * @wma_handle: wma handle
 * @update_vht_opmode: vht opmode
 *
 * Return: none
 */
void wma_process_update_opmode(tp_wma_handle wma_handle,
			       tUpdateVHTOpMode *update_vht_opmode)
{
	WMA_LOGD("%s: opMode = %d", __func__, update_vht_opmode->opMode);

	wma_set_peer_param(wma_handle, update_vht_opmode->peer_mac,
			   WMI_PEER_CHWIDTH, update_vht_opmode->opMode,
			   update_vht_opmode->smesessionId);
}

/**
 * wma_process_update_rx_nss() - process update RX NSS cmd from UMAC
 * @wma_handle: wma handle
 * @update_rx_nss: rx nss value
 *
 * Return: none
 */
void wma_process_update_rx_nss(tp_wma_handle wma_handle,
			       tUpdateRxNss *update_rx_nss)
{
	struct wma_txrx_node *intr =
		&wma_handle->interfaces[update_rx_nss->smesessionId];
	int rx_nss = update_rx_nss->rxNss;

	wma_update_txrx_chainmask(wma_handle->num_rf_chains, &rx_nss);

	intr->nss = (uint8_t)rx_nss;
	update_rx_nss->rxNss = (uint32_t)rx_nss;

	WMA_LOGD("%s: Rx Nss = %d", __func__, update_rx_nss->rxNss);

	wma_set_peer_param(wma_handle, update_rx_nss->peer_mac,
			   WMI_PEER_NSS, update_rx_nss->rxNss,
			   update_rx_nss->smesessionId);
}

#ifdef WLAN_FEATURE_11AC
/**
 * wma_process_update_membership() - process update group membership cmd
 * @wma_handle: wma handle
 * @membership: group membership info
 *
 * Return: none
 */
void wma_process_update_membership(tp_wma_handle wma_handle,
				   tUpdateMembership *membership)
{
	WMA_LOGD("%s: membership = %x ", __func__, membership->membership);

	wma_set_peer_param(wma_handle, membership->peer_mac,
			   WMI_PEER_MEMBERSHIP, membership->membership,
			   membership->smesessionId);
}

/**
 * wma_process_update_userpos() - process update user pos cmd from UMAC
 * @wma_handle: wma handle
 * @userpos: user pos value
 *
 * Return: none
 */
void wma_process_update_userpos(tp_wma_handle wma_handle,
				tUpdateUserPos *userpos)
{
	WMA_LOGD("%s: userPos = %x ", __func__, userpos->userPos);

	wma_set_peer_param(wma_handle, userpos->peer_mac,
			   WMI_PEER_USERPOS, userpos->userPos,
			   userpos->smesessionId);

	/* Now that membership/userpos is updated in fw,
	 * enable GID PPS.
	 */
	wma_set_ppsconfig(userpos->smesessionId, WMA_VHT_PPS_GID_MATCH, 1);

}
#endif /* WLAN_FEATURE_11AC */

/**
 * wma_set_htconfig() - set ht config parameters to target
 * @vdev_id: vdev id
 * @ht_capab: ht capablity
 * @value: value of ht param
 *
 * Return: CDF status
 */
QDF_STATUS wma_set_htconfig(uint8_t vdev_id, uint16_t ht_capab, int value)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	int ret = -EIO;

	if (NULL == wma) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		return QDF_STATUS_E_INVAL;
	}

	switch (ht_capab) {
	case WNI_CFG_HT_CAP_INFO_ADVANCE_CODING:
		ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_LDPC,
						      value);
		break;
	case WNI_CFG_HT_CAP_INFO_TX_STBC:
		ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_TX_STBC,
						      value);
		break;
	case WNI_CFG_HT_CAP_INFO_RX_STBC:
		ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_RX_STBC,
						      value);
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ:
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		WMA_LOGE("%s: ht_capab = %d, value = %d", __func__, ht_capab,
			 value);
		ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
						WMI_VDEV_PARAM_SGI, value);
		if (ret == 0)
			wma->interfaces[vdev_id].config.shortgi = value;
		break;
	default:
		WMA_LOGE("%s:INVALID HT CONFIG", __func__);
	}

	return (ret) ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/**
 * wma_hidden_ssid_vdev_restart() - vdev restart for hidden ssid
 * @wma_handle: wma handle
 * @pReq: hidden ssid vdev restart request
 *
 * Return: none
 */
void wma_hidden_ssid_vdev_restart(tp_wma_handle wma_handle,
				  tHalHiddenSsidVdevRestart *pReq)
{
	struct wma_txrx_node *intr = wma_handle->interfaces;

	if ((pReq->sessionId !=
	     intr[pReq->sessionId].vdev_restart_params.vdev_id)
	    || !((intr[pReq->sessionId].type == WMI_VDEV_TYPE_AP)
		 && (intr[pReq->sessionId].sub_type == 0))) {
		WMA_LOGE("%s : invalid session id", __func__);
		return;
	}

	intr[pReq->sessionId].vdev_restart_params.ssidHidden = pReq->ssidHidden;
	qdf_atomic_set(&intr[pReq->sessionId].vdev_restart_params.
		       hidden_ssid_restart_in_progress, 1);

	/* vdev stop -> vdev restart -> vdev up */
	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP",
		 __func__, pReq->sessionId);
	ol_txrx_vdev_pause(wma_handle->interfaces[pReq->sessionId].handle,
			   OL_TXQ_PAUSE_REASON_VDEV_STOP);
	wma_handle->interfaces[pReq->sessionId].pause_bitmap |=
							(1 << PAUSE_TYPE_HOST);
	if (wmi_unified_vdev_stop_send(wma_handle->wmi_handle, pReq->sessionId)) {
		WMA_LOGE("%s: %d Failed to send vdev stop", __func__, __LINE__);
		qdf_atomic_set(&intr[pReq->sessionId].vdev_restart_params.
			       hidden_ssid_restart_in_progress, 0);
		return;
	}
}


#ifdef WLAN_FEATURE_11W

/**
 * wma_extract_ccmp_pn() - extract 6 byte PN from the CCMP header
 * @ccmp_ptr: CCMP header
 *
 * Return: PN extracted from header.
 */
static uint64_t wma_extract_ccmp_pn(uint8_t *ccmp_ptr)
{
	uint8_t rsvd, key, pn[6];
	uint64_t new_pn;

	/*
	 *   +-----+-----+------+----------+-----+-----+-----+-----+
	 *   | PN0 | PN1 | rsvd | rsvd/key | PN2 | PN3 | PN4 | PN5 |
	 *   +-----+-----+------+----------+-----+-----+-----+-----+
	 *                   CCMP Header Format
	 */

	/* Extract individual bytes */
	pn[0] = (uint8_t) *ccmp_ptr;
	pn[1] = (uint8_t) *(ccmp_ptr + 1);
	rsvd = (uint8_t) *(ccmp_ptr + 2);
	key = (uint8_t) *(ccmp_ptr + 3);
	pn[2] = (uint8_t) *(ccmp_ptr + 4);
	pn[3] = (uint8_t) *(ccmp_ptr + 5);
	pn[4] = (uint8_t) *(ccmp_ptr + 6);
	pn[5] = (uint8_t) *(ccmp_ptr + 7);

	/* Form 6 byte PN with 6 individual bytes of PN */
	new_pn = ((uint64_t) pn[5] << 40) |
		 ((uint64_t) pn[4] << 32) |
		 ((uint64_t) pn[3] << 24) |
		 ((uint64_t) pn[2] << 16) |
		 ((uint64_t) pn[1] << 8) | ((uint64_t) pn[0] << 0);

	WMA_LOGE("PN of received packet is %llu", new_pn);
	return new_pn;
}

/**
 * wma_is_ccmp_pn_replay_attack() - detect replay attacking using PN in CCMP
 * @cds_ctx: cds context
 * @wh: 802.11 frame header
 * @ccmp_ptr: CCMP frame header
 *
 * Return: true/false
 */
static bool
wma_is_ccmp_pn_replay_attack(void *cds_ctx, struct ieee80211_frame *wh,
			 uint8_t *ccmp_ptr)
{
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	ol_txrx_peer_handle peer;
	uint8_t vdev_id, peer_id;
	uint8_t *last_pn_valid;
	uint64_t *last_pn, new_pn;
	uint32_t *rmf_pn_replays;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		return true;
	}

	vdev = wma_find_vdev_by_bssid(cds_ctx, wh->i_addr3, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		return true;
	}

	/* Retrieve the peer based on vdev and addr */
	peer = ol_txrx_find_peer_by_addr_and_vdev(pdev, vdev, wh->i_addr2,
						  &peer_id);

	if (NULL == peer) {
		WMA_LOGE("%s: Failed to find peer, Not able to validate PN",
			    __func__);
		return true;
	}

	new_pn = wma_extract_ccmp_pn(ccmp_ptr);
	last_pn_valid = &peer->last_rmf_pn_valid;
	last_pn = &peer->last_rmf_pn;
	rmf_pn_replays = &peer->rmf_pn_replays;

	if (*last_pn_valid) {
		if (new_pn > *last_pn) {
			*last_pn = new_pn;
			WMA_LOGE("%s: PN validation successful", __func__);
		} else {
			WMA_LOGE("%s: PN Replay attack detected", __func__);
			/* per 11W amendment, keeping track of replay attacks */
			*rmf_pn_replays += 1;
			return true;
		}
	} else {
		*last_pn_valid = 1;
		*last_pn = new_pn;
	}

	return false;
}

/**
 * wma_process_bip() - process mmie in rmf frame
 * @wma_handle: wma handle
 * @iface: txrx node
 * @wh: 80211 frame
 * @wbuf: Buffer
 *
 * Return: 0 for success or error code
 */

static
int wma_process_bip(tp_wma_handle wma_handle,
	struct wma_txrx_node *iface,
	struct ieee80211_frame *wh,
	cdf_nbuf_t wbuf
)
{
	uint16_t key_id;
	uint8_t *efrm;

	efrm = cdf_nbuf_data(wbuf) + cdf_nbuf_len(wbuf);
	key_id = (uint16_t)*(efrm - cds_get_mmie_size() + 2);

	if (!((key_id == WMA_IGTK_KEY_INDEX_4)
	     || (key_id == WMA_IGTK_KEY_INDEX_5))) {
		WMA_LOGE(FL("Invalid KeyID(%d) dropping the frame"), key_id);
		return -EINVAL;
	}
	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				WMI_SERVICE_STA_PMF_OFFLOAD)) {
		/*
		 * if 11w offload is enabled then mmie validation is performed
		 * in firmware, host just need to trim the mmie.
		 */
		cdf_nbuf_trim_tail(wbuf, cds_get_mmie_size());
	} else {
		if (cds_is_mmie_valid(iface->key.key,
			iface->key.key_id[key_id - WMA_IGTK_KEY_INDEX_4].ipn,
			(uint8_t *) wh, efrm)) {
			WMA_LOGE(FL("Protected BC/MC frame MMIE validation successful"));
			/* Remove MMIE */
			cdf_nbuf_trim_tail(wbuf, cds_get_mmie_size());
		} else {
			WMA_LOGE(FL("BC/MC MIC error or MMIE not present, dropping the frame"));
			return -EINVAL;
		}
	}
	return 0;
}

/**
 * wma_process_rmf_frame() - process rmf frame
 * @wma_handle: wma handle
 * @iface: txrx node
 * @wh: 80211 frame
 * @rx_pkt: rx packet
 * @wbuf: Buffer
 *
 * Return: 0 for success or error code
 */
static
int wma_process_rmf_frame(tp_wma_handle wma_handle,
	struct wma_txrx_node *iface,
	struct ieee80211_frame *wh,
	cds_pkt_t *rx_pkt,
	cdf_nbuf_t wbuf)
{
	uint8_t *orig_hdr;
	uint8_t *ccmp;

	if ((wh)->i_fc[1] & IEEE80211_FC1_WEP) {
		if (IEEE80211_IS_BROADCAST(wh->i_addr1) ||
		    IEEE80211_IS_MULTICAST(wh->i_addr1)) {
			WMA_LOGE("Encrypted BC/MC frame dropping the frame");
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}

		orig_hdr = (uint8_t *) cdf_nbuf_data(wbuf);
		/* Pointer to head of CCMP header */
		ccmp = orig_hdr + sizeof(*wh);
		if (wma_is_ccmp_pn_replay_attack(
			wma_handle, wh, ccmp)) {
			WMA_LOGE("Dropping the frame");
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}

		/* Strip privacy headers (and trailer)
		 * for a received frame
		 */
		cdf_mem_move(orig_hdr +
			IEEE80211_CCMP_HEADERLEN, wh,
			sizeof(*wh));
		cdf_nbuf_pull_head(wbuf,
			IEEE80211_CCMP_HEADERLEN);
			cdf_nbuf_trim_tail(wbuf, IEEE80211_CCMP_MICLEN);

		rx_pkt->pkt_meta.mpdu_hdr_ptr =
				cdf_nbuf_data(wbuf);
		rx_pkt->pkt_meta.mpdu_len = cdf_nbuf_len(wbuf);
		rx_pkt->pkt_meta.mpdu_data_len =
		rx_pkt->pkt_meta.mpdu_len -
		rx_pkt->pkt_meta.mpdu_hdr_len;
		rx_pkt->pkt_meta.mpdu_data_ptr =
		rx_pkt->pkt_meta.mpdu_hdr_ptr +
		rx_pkt->pkt_meta.mpdu_hdr_len;
		rx_pkt->pkt_meta.tsf_delta = rx_pkt->pkt_meta.tsf_delta;
		rx_pkt->pkt_buf = wbuf;
		WMA_LOGD(FL("BSSID: "MAC_ADDRESS_STR" tsf_delta: %u"),
		    MAC_ADDR_ARRAY(wh->i_addr3), rx_pkt->pkt_meta.tsf_delta);
	} else {
		if (IEEE80211_IS_BROADCAST(wh->i_addr1) ||
		    IEEE80211_IS_MULTICAST(wh->i_addr1)) {
			if (0 != wma_process_bip(wma_handle, iface, wh, wbuf)) {
					cds_pkt_return_packet(rx_pkt);
					return -EINVAL;
			}
		} else {
			WMA_LOGE("Rx unprotected unicast mgmt frame");
			rx_pkt->pkt_meta.dpuFeedback =
				DPU_FEEDBACK_UNPROTECTED_ERROR;
		}
	}
    return 0;
}
#endif

/**
 * wma_mgmt_rx_process() - process management rx frame.
 * @handle: wma handle
 * @data: rx data
 * @data_len: data length
 *
 * Return: 0 for success or error code
 */
static int wma_mgmt_rx_process(void *handle, uint8_t *data,
				  uint32_t data_len)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	WMI_MGMT_RX_EVENTID_param_tlvs *param_tlvs = NULL;
	wmi_mgmt_rx_hdr *hdr = NULL;
	struct wma_txrx_node *iface = NULL;
	uint8_t vdev_id = WMA_INVALID_VDEV_ID;
	cds_pkt_t *rx_pkt;
	cdf_nbuf_t wbuf;
	struct ieee80211_frame *wh;
	uint8_t mgt_type, mgt_subtype;
	int status;

	if (!wma_handle) {
		WMA_LOGE("%s: Failed to get WMA  context", __func__);
		return -EINVAL;
	}

	param_tlvs = (WMI_MGMT_RX_EVENTID_param_tlvs *) data;
	if (!param_tlvs) {
		WMA_LOGE("Get NULL point message from FW");
		return -EINVAL;
	}

	hdr = param_tlvs->hdr;
	if (!hdr) {
		WMA_LOGE("Rx event is NULL");
		return -EINVAL;
	}

	if (hdr->buf_len < sizeof(struct ieee80211_frame)) {
		WMA_LOGE("Invalid rx mgmt packet");
		return -EINVAL;
	}

	rx_pkt = cdf_mem_malloc(sizeof(*rx_pkt));
	if (!rx_pkt) {
		WMA_LOGE("Failed to allocate rx packet");
		return -ENOMEM;
	}

	if (cds_is_load_or_unload_in_progress()) {
		WMA_LOGE("Load/Unload in progress");
		return -EINVAL;
	}

	cdf_mem_zero(rx_pkt, sizeof(*rx_pkt));

	/*
	 * Fill in meta information needed by pe/lim
	 * TODO: Try to maintain rx metainfo as part of skb->data.
	 */
	rx_pkt->pkt_meta.channel = hdr->channel;
	rx_pkt->pkt_meta.scan_src = hdr->flags;

	/*
	 * Get the rssi value from the current snr value
	 * using standard noise floor of -96.
	 */
	rx_pkt->pkt_meta.rssi = hdr->snr + WMA_NOISE_FLOOR_DBM_DEFAULT;
	rx_pkt->pkt_meta.snr = hdr->snr;

	/* If absolute rssi is available from firmware, use it */
	if (hdr->rssi != 0)
		rx_pkt->pkt_meta.rssi_raw = hdr->rssi;
	else
		rx_pkt->pkt_meta.rssi_raw = rx_pkt->pkt_meta.rssi;


	/*
	 * FIXME: Assigning the local timestamp as hw timestamp is not
	 * available. Need to see if pe/lim really uses this data.
	 */
	rx_pkt->pkt_meta.timestamp = (uint32_t) jiffies;
	rx_pkt->pkt_meta.mpdu_hdr_len = sizeof(struct ieee80211_frame);
	rx_pkt->pkt_meta.mpdu_len = hdr->buf_len;
	rx_pkt->pkt_meta.mpdu_data_len = hdr->buf_len -
					 rx_pkt->pkt_meta.mpdu_hdr_len;

	rx_pkt->pkt_meta.roamCandidateInd = 0;

	/* Why not just use rx_event->hdr.buf_len? */
	wbuf = cdf_nbuf_alloc(NULL, roundup(hdr->buf_len, 4), 0, 4, false);
	if (!wbuf) {
		WMA_LOGE("%s: Failed to allocate wbuf for mgmt rx len(%u)",
			    __func__, hdr->buf_len);
		cdf_mem_free(rx_pkt);
		return -ENOMEM;
	}

	cdf_nbuf_put_tail(wbuf, hdr->buf_len);
	cdf_nbuf_set_protocol(wbuf, ETH_P_CONTROL);
	wh = (struct ieee80211_frame *)cdf_nbuf_data(wbuf);

	rx_pkt->pkt_meta.mpdu_hdr_ptr = cdf_nbuf_data(wbuf);
	rx_pkt->pkt_meta.mpdu_data_ptr = rx_pkt->pkt_meta.mpdu_hdr_ptr +
					 rx_pkt->pkt_meta.mpdu_hdr_len;
	rx_pkt->pkt_meta.tsf_delta = hdr->tsf_delta;
	rx_pkt->pkt_buf = wbuf;

	WMA_LOGD(FL("BSSID: "MAC_ADDRESS_STR" tsf_delta: %u"),
		 MAC_ADDR_ARRAY(wh->i_addr3), hdr->tsf_delta);

#ifdef BIG_ENDIAN_HOST
	{
		/*
		 * for big endian host, copy engine byte_swap is enabled
		 * But the rx mgmt frame buffer content is in network byte order
		 * Need to byte swap the mgmt frame buffer content - so when
		 * copy engine does byte_swap - host gets buffer content in the
		 * correct byte order.
		 */
		int i;
		uint32_t *destp, *srcp;
		destp = (uint32_t *) wh;
		srcp = (uint32_t *) param_tlvs->bufp;
		for (i = 0;
		     i < (roundup(hdr->buf_len, sizeof(uint32_t)) / 4); i++) {
			*destp = cpu_to_le32(*srcp);
			destp++;
			srcp++;
		}
	}
#else
	cdf_mem_copy(wh, param_tlvs->bufp, hdr->buf_len);
#endif

	WMA_LOGD(
		FL("BSSID: "MAC_ADDRESS_STR" snr = %d, rssi = %d, rssi_raw = %d"),
			MAC_ADDR_ARRAY(wh->i_addr3),
			hdr->snr, rx_pkt->pkt_meta.rssi,
			rx_pkt->pkt_meta.rssi_raw);
	if (!wma_handle->mgmt_rx) {
		WMA_LOGE("Not registered for Mgmt rx, dropping the frame");
		cds_pkt_return_packet(rx_pkt);
		return -EINVAL;
	}

	/* If it is a beacon/probe response, save it for future use */
	mgt_type = (wh)->i_fc[0] & IEEE80211_FC0_TYPE_MASK;
	mgt_subtype = (wh)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK;

#ifdef WLAN_FEATURE_11W
	if (mgt_type == IEEE80211_FC0_TYPE_MGT &&
	    (mgt_subtype == IEEE80211_FC0_SUBTYPE_DISASSOC ||
	     mgt_subtype == IEEE80211_FC0_SUBTYPE_DEAUTH ||
	     mgt_subtype == IEEE80211_FC0_SUBTYPE_ACTION)) {
		if (wma_find_vdev_by_bssid(
			wma_handle, wh->i_addr3, &vdev_id)) {
			iface = &(wma_handle->interfaces[vdev_id]);
			if (iface->rmfEnabled) {
				status = wma_process_rmf_frame(wma_handle,
					iface, wh, rx_pkt, wbuf);
				if (status != 0)
					return status;
			}
		}
	}
#endif /* WLAN_FEATURE_11W */
	rx_pkt->pkt_meta.sessionId =
		(vdev_id == WMA_INVALID_VDEV_ID ? 0 : vdev_id);
	wma_handle->mgmt_rx(wma_handle, rx_pkt);
	return 0;
}

/**
 * wma_de_register_mgmt_frm_client() - deregister management frame
 * @cds_ctx: cds context
 *
 * Return: CDF status
 */
QDF_STATUS wma_de_register_mgmt_frm_client(void *cds_ctx)
{
	tp_wma_handle wma_handle;

#ifdef QCA_WIFI_FTM
	if (cds_get_conparam() == QDF_GLOBAL_FTM_MODE)
		return QDF_STATUS_SUCCESS;
#endif

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		WMA_LOGE("%s: Failed to get WMA context", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
						 WMI_MGMT_RX_EVENTID) != 0) {
		WMA_LOGE("Failed to Unregister rx mgmt handler with wmi");
		return QDF_STATUS_E_FAILURE;
	}
	wma_handle->mgmt_rx = NULL;
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_register_roaming_callbacks() - Register roaming callbacks
 * @cds_ctx: CDS Context
 * @csr_roam_synch_cb: CSR roam synch callback routine pointer
 * @pe_roam_synch_cb: PE roam synch callback routine pointer
 *
 * Register the SME and PE callback routines with WMA for
 * handling roaming
 *
 * Return: Success or Failure Status
 */
QDF_STATUS wma_register_roaming_callbacks(void *cds_ctx,
	void (*csr_roam_synch_cb)(tpAniSirGlobal mac,
		roam_offload_synch_ind *roam_synch_data,
		tpSirBssDescription  bss_desc_ptr, uint8_t reason),
	QDF_STATUS (*pe_roam_synch_cb)(tpAniSirGlobal mac,
		roam_offload_synch_ind *roam_synch_data,
		tpSirBssDescription  bss_desc_ptr))
{

	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma) {
		WMA_LOGE("%s: Failed to get WMA context", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	wma->csr_roam_synch_cb = csr_roam_synch_cb;
	wma->pe_roam_synch_cb = pe_roam_synch_cb;
	WMA_LOGD("Registered roam synch callbacks with WMA successfully");
	return QDF_STATUS_SUCCESS;
}
/**
 * wma_register_mgmt_frm_client() - register management frame callback
 * @cds_ctx: cds context
 * @mgmt_frm_rx: management frame
 *
 * Return: CDF status
 */
QDF_STATUS wma_register_mgmt_frm_client(
	void *cds_ctx, wma_mgmt_frame_rx_callback mgmt_frm_rx)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGE("%s: Failed to get WMA context", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (wmi_unified_register_event_handler(wma_handle->wmi_handle,
					       WMI_MGMT_RX_EVENTID,
					       wma_mgmt_rx_process) != 0) {
		WMA_LOGE("Failed to register rx mgmt handler with wmi");
		return QDF_STATUS_E_FAILURE;
	}
	wma_handle->mgmt_rx = mgmt_frm_rx;

	return QDF_STATUS_SUCCESS;
}
