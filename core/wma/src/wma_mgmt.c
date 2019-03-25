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

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#else
#include "pktlog_ac_fmt.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "csr_api.h"
#include "ol_fw.h"
#include "wma_internal.h"
#include "wlan_policy_mgr_api.h"
#include "cdp_txrx_flow_ctrl_legacy.h"
#include <cdp_txrx_peer_ops.h>
#include <cdp_txrx_pmf.h>
#include <cdp_txrx_cfg.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_misc.h>
#include <cdp_txrx_misc.h>
#include "wlan_mgmt_txrx_tgt_api.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_lmac_if_api.h"
#include <cdp_txrx_handle.h>
#include "wma_he.h"
#include <qdf_crypto.h>
#include "wma_twt.h"
#include "wlan_p2p_cfg_api.h"
#include "cfg_ucfg_api.h"
#include "cfg_mlme_sta.h"
#include "wlan_mlme_api.h"
#include "wmi_unified_bcn_api.h"
#include <wlan_crypto_global_api.h>
#include <wlan_mlme_main.h>

/**
 * wma_send_bcn_buf_ll() - prepare and send beacon buffer to fw for LL
 * @wma: wma handle
 * @pdev: txrx pdev
 * @vdev_id: vdev id
 * @param_buf: SWBA parameters
 *
 * Return: none
 */
#ifdef CONFIG_WMI_BCN_OFFLOAD
static void wma_send_bcn_buf_ll(tp_wma_handle wma,
				struct cdp_pdev *pdev,
				uint8_t vdev_id,
				WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf)
{
	struct ieee80211_frame *wh;
	struct beacon_info *bcn;
	wmi_tim_info *tim_info = param_buf->tim_info;
	uint8_t *bcn_payload;
	QDF_STATUS ret;
	struct beacon_tim_ie *tim_ie;
	wmi_p2p_noa_info *p2p_noa_info = param_buf->p2p_noa_info;
	struct p2p_sub_element_noa noa_ie;
	struct wmi_bcn_send_from_host params;
	uint8_t i;

	bcn = wma->interfaces[vdev_id].beacon;
	if (!bcn || !bcn->buf) {
		WMA_LOGE("%s: Invalid beacon buffer", __func__);
		return;
	}

	if (!param_buf->tim_info || !param_buf->p2p_noa_info) {
		WMA_LOGE("%s: Invalid tim info or p2p noa info", __func__);
		return;
	}

	if (WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(p2p_noa_info) >
			WMI_P2P_MAX_NOA_DESCRIPTORS) {
		WMA_LOGE("%s: Too many descriptors %d", __func__,
			WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(p2p_noa_info));
		return;
	}

	qdf_spin_lock_bh(&bcn->lock);

	bcn_payload = qdf_nbuf_data(bcn->buf);

	tim_ie = (struct beacon_tim_ie *)(&bcn_payload[bcn->tim_ie_offset]);

	if (tim_info->tim_changed) {
		if (tim_info->tim_num_ps_pending)
			qdf_mem_copy(&tim_ie->tim_bitmap, tim_info->tim_bitmap,
				     WMA_TIM_SUPPORTED_PVB_LENGTH);
		else
			qdf_mem_zero(&tim_ie->tim_bitmap,
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
	 * the value lower than min_sw_seq is reserved for HW generated frame
	 */
	if ((bcn->seq_no & IEEE80211_SEQ_MASK) < MIN_SW_SEQ)
		bcn->seq_no = MIN_SW_SEQ;

	wh = (struct ieee80211_frame *)bcn_payload;
	*(uint16_t *) &wh->i_seq[0] = htole16(bcn->seq_no
					      << IEEE80211_SEQ_SEQ_SHIFT);
	bcn->seq_no++;

	if (WMI_UNIFIED_NOA_ATTR_IS_MODIFIED(p2p_noa_info)) {
		qdf_mem_zero(&noa_ie, sizeof(noa_ie));

		noa_ie.index =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_INDEX_GET(p2p_noa_info);
		noa_ie.oppPS =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_OPP_PS_GET(p2p_noa_info);
		noa_ie.ctwindow =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_CTWIN_GET(p2p_noa_info);
		noa_ie.num_descriptors = (uint8_t)
				WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(p2p_noa_info);
		WMA_LOGI("%s: index %u, oppPs %u, ctwindow %u, num_descriptors = %u",
			 __func__, noa_ie.index,
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
			WMA_LOGI("%s: NoA descriptor[%d] type_count %u, duration %u, interval %u, start_time = %u",
				 __func__, i,
				 noa_ie.noa_descriptors[i].type_count,
				 noa_ie.noa_descriptors[i].duration,
				 noa_ie.noa_descriptors[i].interval,
				 noa_ie.noa_descriptors[i].start_time);
		}
		wma_update_noa(bcn, &noa_ie);

		/* Send a msg to LIM to update the NoA IE in probe response
		 * frames transmitted by the host
		 */
		wma_update_probe_resp_noa(wma, &noa_ie);
	}

	if (bcn->dma_mapped) {
		qdf_nbuf_unmap_single(wma->qdf_dev, bcn->buf, QDF_DMA_TO_DEVICE);
		bcn->dma_mapped = 0;
	}
	ret = qdf_nbuf_map_single(wma->qdf_dev, bcn->buf, QDF_DMA_TO_DEVICE);
	if (ret != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: failed map beacon buf to DMA region", __func__);
		qdf_spin_unlock_bh(&bcn->lock);
		return;
	}

	bcn->dma_mapped = 1;
	params.vdev_id = vdev_id;
	params.data_len = bcn->len;
	params.frame_ctrl = *((A_UINT16 *) wh->i_fc);
	params.frag_ptr = qdf_nbuf_get_frag_paddr(bcn->buf, 0);
	params.dtim_flag = 0;
	/* notify Firmware of DTM and mcast/bcast traffic */
	if (tim_ie->dtim_count == 0) {
		params.dtim_flag |= WMI_BCN_SEND_DTIM_ZERO;
		/* deliver mcast/bcast traffic in next DTIM beacon */
		if (tim_ie->tim_bitctl & 0x01)
			params.dtim_flag |= WMI_BCN_SEND_DTIM_BITCTL_SET;
	}

	wmi_unified_bcn_buf_ll_cmd(wma->wmi_handle,
					&params);

	qdf_spin_unlock_bh(&bcn->lock);
}
#else
static inline void
wma_send_bcn_buf_ll(tp_wma_handle wma,
		    struct cdp_pdev *pdev,
		    uint8_t vdev_id,
		    WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf)
{
}
#endif
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
#ifdef CONFIG_WMI_BCN_OFFLOAD
int wma_beacon_swba_handler(void *handle, uint8_t *event, uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf;
	wmi_host_swba_event_fixed_param *swba_event;
	uint32_t vdev_map;
	struct cdp_pdev *pdev;
	uint8_t vdev_id = 0;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

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

	WMA_LOGD("vdev_map = %d", vdev_map);
	for (; vdev_map && vdev_id < wma->max_bssid;
			vdev_id++, vdev_map >>= 1) {
		if (!(vdev_map & 0x1))
			continue;
		if (!cdp_cfg_is_high_latency(soc,
			(struct cdp_cfg *)cds_get_context(QDF_MODULE_ID_CFG)))
			wma_send_bcn_buf_ll(wma, pdev, vdev_id, param_buf);
		break;
	}
	return 0;
}
#else
static inline int
wma_beacon_swba_handler(void *handle, uint8_t *event, uint32_t len)
{
	return 0;
}
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
void wma_sta_kickout_event(uint32_t kickout_reason, uint8_t vdev_id,
							uint8_t *macaddr)
{
	WLAN_HOST_DIAG_EVENT_DEF(sta_kickout, struct host_event_wlan_kickout);
	qdf_mem_zero(&sta_kickout, sizeof(sta_kickout));
	sta_kickout.reasoncode = kickout_reason;
	sta_kickout.vdev_id = vdev_id;
	if (macaddr)
		qdf_mem_copy(sta_kickout.peer_mac, macaddr,
							QDF_MAC_ADDR_SIZE);
	WLAN_HOST_DIAG_EVENT_REPORT(&sta_kickout, EVENT_WLAN_STA_KICKOUT);
}
#endif

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
int wma_peer_sta_kickout_event_handler(void *handle, uint8_t *event,
				       uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_PEER_STA_KICKOUT_EVENTID_param_tlvs *param_buf = NULL;
	wmi_peer_sta_kickout_event_fixed_param *kickout_event = NULL;
	uint8_t vdev_id, peer_id, macaddr[QDF_MAC_ADDR_SIZE];
	void *peer;
	struct cdp_pdev *pdev;
	tpDeleteStaContext del_sta_ctx;
	struct ibss_peer_inactivity_ind *inactivity;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("%s: Enter", __func__);
	param_buf = (WMI_PEER_STA_KICKOUT_EVENTID_param_tlvs *) event;
	kickout_event = param_buf->fixed_param;
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is NULL", __func__);
		return -EINVAL;
	}
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&kickout_event->peer_macaddr, macaddr);
	peer = cdp_peer_find_by_addr(soc, pdev,	macaddr, &peer_id);
	if (!peer) {
		WMA_LOGE("PEER [%pM] not found", macaddr);
		return -EINVAL;
	}

	if (cdp_peer_get_vdevid(soc, peer, &vdev_id) != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Not able to find BSSID for peer [%pM]", macaddr);
		return -EINVAL;
	}

	WMA_LOGA("%s: PEER:[%pM], ADDR:[%pN], INTERFACE:%d, peer_id:%d, reason:%d",
		__func__, macaddr, wma->interfaces[vdev_id].addr, vdev_id,
		 peer_id, kickout_event->reason);
	if (wma->interfaces[vdev_id].roaming_in_progress) {
		WMA_LOGE("Ignore STA kick out since roaming is in progress");
		return -EINVAL;
	}

	switch (kickout_event->reason) {
	case WMI_PEER_STA_KICKOUT_REASON_IBSS_DISCONNECT:
		inactivity = qdf_mem_malloc(sizeof(*inactivity));
		if (!inactivity) {
			WMA_LOGE("QDF MEM Alloc Failed for tSirIbssPeerInactivity");
			return -ENOMEM;
		}

		inactivity->staIdx = peer_id;
		qdf_mem_copy(inactivity->peer_addr.bytes, macaddr,
			     QDF_MAC_ADDR_SIZE);
		wma_send_msg(wma, WMA_IBSS_PEER_INACTIVITY_IND,
			     inactivity, 0);
		goto exit_handler;
#ifdef FEATURE_WLAN_TDLS
	case WMI_PEER_STA_KICKOUT_REASON_TDLS_DISCONNECT:
		del_sta_ctx = (tpDeleteStaContext)
			qdf_mem_malloc(sizeof(tDeleteStaContext));
		if (!del_sta_ctx) {
			WMA_LOGE("%s: mem alloc failed for struct del_sta_context for TDLS peer: %pM",
				__func__, macaddr);
			return -ENOMEM;
		}

		del_sta_ctx->is_tdls = true;
		del_sta_ctx->vdev_id = vdev_id;
		del_sta_ctx->staId = peer_id;
		qdf_mem_copy(del_sta_ctx->addr2, macaddr, QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(del_sta_ctx->bssId, wma->interfaces[vdev_id].bssid,
			     QDF_MAC_ADDR_SIZE);
		del_sta_ctx->reasonCode = HAL_DEL_STA_REASON_CODE_KEEP_ALIVE;
		wma_send_msg(wma, SIR_LIM_DELETE_STA_CONTEXT_IND,
			     (void *)del_sta_ctx, 0);
		goto exit_handler;
#endif /* FEATURE_WLAN_TDLS */
	case WMI_PEER_STA_KICKOUT_REASON_XRETRY:
		if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_STA &&
		    (wma->interfaces[vdev_id].sub_type == 0 ||
		     wma->interfaces[vdev_id].sub_type ==
		     WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT) &&
		    !qdf_mem_cmp(wma->interfaces[vdev_id].bssid,
				    macaddr, QDF_MAC_ADDR_SIZE)) {
			wma_sta_kickout_event(HOST_STA_KICKOUT_REASON_XRETRY,
							vdev_id, macaddr);
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
			wma_beacon_miss_handler(wma, vdev_id,
						kickout_event->rssi);
			goto exit_handler;
		}
		break;

	case WMI_PEER_STA_KICKOUT_REASON_UNSPECIFIED:
		/*
		 * Default legacy value used by original firmware implementation
		 */
		if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_STA &&
		    (wma->interfaces[vdev_id].sub_type == 0 ||
		     wma->interfaces[vdev_id].sub_type ==
		     WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT) &&
		    !qdf_mem_cmp(wma->interfaces[vdev_id].bssid,
				    macaddr, QDF_MAC_ADDR_SIZE)) {
			wma_sta_kickout_event(
			HOST_STA_KICKOUT_REASON_UNSPECIFIED, vdev_id, macaddr);
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
			wma_beacon_miss_handler(wma, vdev_id,
						kickout_event->rssi);
			goto exit_handler;
		}
		break;

	case WMI_PEER_STA_KICKOUT_REASON_INACTIVITY:
	/*
	 * Handle SA query kickout is same as inactivity kickout.
	 * This could be for STA or SAP role
	 */
	case WMI_PEER_STA_KICKOUT_REASON_SA_QUERY_TIMEOUT:
	default:
		break;
	}

	/*
	 * default action is to send delete station context indication to LIM
	 */
	del_sta_ctx =
		(tDeleteStaContext *) qdf_mem_malloc(sizeof(tDeleteStaContext));
	if (!del_sta_ctx) {
		WMA_LOGE("QDF MEM Alloc Failed for struct del_sta_context");
		return -ENOMEM;
	}

	del_sta_ctx->is_tdls = false;
	del_sta_ctx->vdev_id = vdev_id;
	del_sta_ctx->staId = peer_id;
	qdf_mem_copy(del_sta_ctx->addr2, macaddr, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(del_sta_ctx->bssId, wma->interfaces[vdev_id].addr,
		     QDF_MAC_ADDR_SIZE);
	del_sta_ctx->reasonCode = HAL_DEL_STA_REASON_CODE_KEEP_ALIVE;
	if (wmi_service_enabled(wma->wmi_handle,
				wmi_service_hw_db2dbm_support))
		del_sta_ctx->rssi = kickout_event->rssi;
	else
		del_sta_ctx->rssi = kickout_event->rssi +
					WMA_TGT_NOISE_FLOOR_DBM;
	wma_sta_kickout_event(HOST_STA_KICKOUT_REASON_KEEP_ALIVE,
							vdev_id, macaddr);
	wma_send_msg(wma, SIR_LIM_DELETE_STA_CONTEXT_IND, (void *)del_sta_ctx,
		     0);
	wma_lost_link_info_handler(wma, vdev_id, del_sta_ctx->rssi);
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

	WMA_LOGD("%s", __func__);

	if (resp_event->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: received invalid vdev_id %d",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}

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
			qdf_mem_malloc(sizeof(tSirFirstBeaconTxCompleteInd));
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
 * wma_get_go_probe_timeout() - get P2P GO probe timeout
 * @mac: UMAC handler
 * @max_inactive_time: return max inactive time
 * @max_unresponsive_time: return max unresponsive time
 *
 * Return: none
 */
#ifdef CONVERGED_P2P_ENABLE
static inline void
wma_get_go_probe_timeout(struct mac_context *mac,
			 uint32_t *max_inactive_time,
			 uint32_t *max_unresponsive_time)
{
	uint32_t keep_alive;
	QDF_STATUS status;

	status = cfg_p2p_get_go_link_monitor_period(mac->psoc,
						    max_inactive_time);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Failed to go monitor period");
		*max_inactive_time = WMA_LINK_MONITOR_DEFAULT_TIME_SECS;
	}
	status = cfg_p2p_get_go_keepalive_period(mac->psoc,
						 &keep_alive);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Failed to read go keep alive");
		keep_alive = WMA_KEEP_ALIVE_DEFAULT_TIME_SECS;
	}

	*max_unresponsive_time = *max_inactive_time + keep_alive;
}
#else
static inline void
wma_get_go_probe_timeout(struct mac_context *mac,
			 uint32_t *max_inactive_time,
			 uint32_t *max_unresponsive_time)
{
}
#endif

/**
 * wma_get_link_probe_timeout() - get link timeout based on sub type
 * @mac: UMAC handler
 * @sub_type: vdev syb type
 * @max_inactive_time: return max inactive time
 * @max_unresponsive_time: return max unresponsive time
 *
 * Return: none
 */
static inline void
wma_get_link_probe_timeout(struct mac_context *mac,
			   uint32_t sub_type,
			   uint32_t *max_inactive_time,
			   uint32_t *max_unresponsive_time)
{
	if (sub_type == WMI_UNIFIED_VDEV_SUBTYPE_P2P_GO) {
		wma_get_go_probe_timeout(mac, max_inactive_time,
					 max_unresponsive_time);
	} else {
		*max_inactive_time =
			mac->mlme_cfg->timeouts.ap_link_monitor_timeout;
		*max_unresponsive_time = *max_inactive_time +
			mac->mlme_cfg->timeouts.ap_keep_alive_timeout;
	}
}

/**
 * wma_verify_rate_code() - verify if rate code is valid.
 * @rate_code:     rate code
 * @band:     band information
 *
 * Return: verify result
 */
static bool wma_verify_rate_code(u_int32_t rate_code, enum cds_band_type band)
{
	uint8_t preamble, nss, rate;
	bool valid = true;

	preamble = (rate_code & 0xc0) >> 6;
	nss = (rate_code & 0x30) >> 4;
	rate = rate_code & 0xf;

	switch (preamble) {
	case WMI_RATE_PREAMBLE_CCK:
		if (nss != 0 || rate > 3 || band == CDS_BAND_5GHZ)
			valid = false;
		break;
	case WMI_RATE_PREAMBLE_OFDM:
		if (nss != 0 || rate > 7)
			valid = false;
		break;
	case WMI_RATE_PREAMBLE_HT:
		if (nss != 0 || rate > 7)
			valid = false;
		break;
	case WMI_RATE_PREAMBLE_VHT:
		if (nss != 0 || rate > 9)
			valid = false;
		break;
	default:
		break;
	}
	return valid;
}

#define TX_MGMT_RATE_2G_ENABLE_OFFSET 30
#define TX_MGMT_RATE_5G_ENABLE_OFFSET 31
#define TX_MGMT_RATE_2G_OFFSET 0
#define TX_MGMT_RATE_5G_OFFSET 12

/**
 * wma_set_mgmt_rate() - set vdev mgmt rate.
 * @wma:     wma handle
 * @vdev_id: vdev id
 *
 * Return: None
 */
void wma_set_vdev_mgmt_rate(tp_wma_handle wma, uint8_t vdev_id)
{
	uint32_t cfg_val;
	int ret;
	uint32_t per_band_mgmt_tx_rate = 0;
	enum cds_band_type band = 0;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Failed to get mac", __func__);
		return;
	}

	cfg_val = mac->mlme_cfg->sap_cfg.rate_tx_mgmt;
	band = CDS_BAND_ALL;
	if ((cfg_val == MLME_CFG_TX_MGMT_RATE_DEF) ||
	    !wma_verify_rate_code(cfg_val, band)) {
		WMA_LOGD("default WNI_CFG_RATE_FOR_TX_MGMT, ignore");
	} else {
		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					 WMI_VDEV_PARAM_MGMT_TX_RATE,
					 cfg_val);
		if (ret)
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_MGMT_TX_RATE");
	}

	cfg_val = mac->mlme_cfg->sap_cfg.rate_tx_mgmt_2g;
	band = CDS_BAND_2GHZ;
	if ((cfg_val == MLME_CFG_TX_MGMT_2G_RATE_DEF) ||
	    !wma_verify_rate_code(cfg_val, band)) {
		WMA_LOGD("use default 2G MGMT rate.");
		per_band_mgmt_tx_rate &=
		    ~(1 << TX_MGMT_RATE_2G_ENABLE_OFFSET);
	} else {
		per_band_mgmt_tx_rate |=
		    (1 << TX_MGMT_RATE_2G_ENABLE_OFFSET);
		per_band_mgmt_tx_rate |=
		    ((cfg_val & 0x7FF) << TX_MGMT_RATE_2G_OFFSET);
	}

	cfg_val = mac->mlme_cfg->sap_cfg.rate_tx_mgmt;
	band = CDS_BAND_5GHZ;
	if ((cfg_val == MLME_CFG_TX_MGMT_5G_RATE_DEF) ||
	    !wma_verify_rate_code(cfg_val, band)) {
		WMA_LOGD("use default 5G MGMT rate.");
		per_band_mgmt_tx_rate &=
		    ~(1 << TX_MGMT_RATE_5G_ENABLE_OFFSET);
	} else {
		per_band_mgmt_tx_rate |=
		    (1 << TX_MGMT_RATE_5G_ENABLE_OFFSET);
		per_band_mgmt_tx_rate |=
		    ((cfg_val & 0x7FF) << TX_MGMT_RATE_5G_OFFSET);
	}

	ret = wma_vdev_set_param(
		wma->wmi_handle,
		vdev_id,
		WMI_VDEV_PARAM_PER_BAND_MGMT_TX_RATE,
		per_band_mgmt_tx_rate);
	if (ret)
		WMA_LOGE("Failed to set WMI_VDEV_PARAM_PER_BAND_MGMT_TX_RATE");

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
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	QDF_STATUS status;

	if (!mac) {
		WMA_LOGE("%s: Failed to get mac", __func__);
		return;
	}

	wma_get_link_probe_timeout(mac, wma->interfaces[vdev_id].sub_type,
				   &max_inactive_time, &max_unresponsive_time);

	min_inactive_time = max_inactive_time / 2;

	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_AP_KEEPALIVE_MIN_IDLE_INACTIVE_TIME_SECS,
			min_inactive_time);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("Failed to Set AP MIN IDLE INACTIVE TIME");

	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_IDLE_INACTIVE_TIME_SECS,
			max_inactive_time);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("Failed to Set AP MAX IDLE INACTIVE TIME");

	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS,
			max_unresponsive_time);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("Failed to Set MAX UNRESPONSIVE TIME");

	WMA_LOGD("%s:vdev_id:%d min_inactive_time: %u max_inactive_time: %u max_unresponsive_time: %u",
		 __func__, vdev_id,
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
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint8_t max_retries;
	uint16_t retry_interval;

	WMA_LOGD(FL("Enter:"));

	if (!mac) {
		WMA_LOGE(FL("mac context is NULL"));
		return;
	}

	max_retries = mac->mlme_cfg->gen.pmf_sa_query_max_retries;
	retry_interval = mac->mlme_cfg->gen.pmf_sa_query_retry_interval;

	wmi_unified_set_sta_sa_query_param_cmd(wma->wmi_handle,
						vdev_id,
						max_retries,
						retry_interval);

	WMA_LOGD(FL("Exit :"));
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
	struct sta_keep_alive_params params = { 0 };

	WMA_LOGD("%s: Enter", __func__);

	if (!wma) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return;
	}

	if (timeperiod > cfg_max(CFG_INFRA_STA_KEEP_ALIVE_PERIOD)) {
		WMI_LOGE("Invalid period %d Max limit %d", timeperiod,
			 cfg_max(CFG_INFRA_STA_KEEP_ALIVE_PERIOD));
		return;
	}

	params.vdev_id = vdev_id;
	params.method = method;
	params.timeperiod = timeperiod;
	if (hostv4addr)
		qdf_mem_copy(params.hostv4addr, hostv4addr, QDF_IPV4_ADDR_SIZE);
	if (destv4addr)
		qdf_mem_copy(params.destv4addr, destv4addr, QDF_IPV4_ADDR_SIZE);
	if (destmac)
		qdf_mem_copy(params.destmac, destmac, QDF_MAC_ADDR_SIZE);

	wmi_unified_set_sta_keep_alive_cmd(wma->wmi_handle, &params);
	WMA_LOGD("%s: Exit", __func__);
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
	WMA_LOGD("%s: WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID", __func__);
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

#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)

/**
 * wma_unified_peer_state_update() - update peer state
 * @pdev: pdev handle
 * @sta_mac: pointer to sta mac addr
 * @bss_addr: bss address
 * @sta_type: sta entry type
 *
 *
 * Return: None
 */
static void
wma_unified_peer_state_update(
	struct cdp_pdev *pdev,
	uint8_t *sta_mac,
	uint8_t *bss_addr,
	uint8_t sta_type)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (STA_ENTRY_TDLS_PEER == sta_type)
		cdp_peer_state_update(soc, pdev, sta_mac,
					  OL_TXRX_PEER_STATE_AUTH);
	else
		cdp_peer_state_update(soc, pdev, bss_addr,
					  OL_TXRX_PEER_STATE_AUTH);
}
#else

static inline void
wma_unified_peer_state_update(
	struct cdp_pdev *pdev,
	uint8_t *sta_mac,
	uint8_t *bss_addr,
	uint8_t sta_type)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	cdp_peer_state_update(soc, pdev, bss_addr, OL_TXRX_PEER_STATE_AUTH);
}
#endif

#define CFG_CTRL_MASK              0xFF00
#define CFG_DATA_MASK              0x00FF

/**
 * wma_mask_tx_ht_rate() - mask tx ht rate based on config
 * @wma:     wma handle
 * @mcs_set  mcs set buffer
 *
 * Return: None
 */
static void wma_mask_tx_ht_rate(tp_wma_handle wma, uint8_t *mcs_set)
{
	uint32_t i, j;
	uint16_t mcs_limit;
	uint8_t *rate_pos = mcs_set;
	struct mac_context *mac = (struct mac_context *)wma->mac_context;

	/*
	 * Get MCS limit from ini configure, and map it to rate parameters
	 * This will limit HT rate upper bound. CFG_CTRL_MASK is used to
	 * check whether ini config is enabled and CFG_DATA_MASK to get the
	 * MCS value.
	 */
	mcs_limit = mac->mlme_cfg->rates.max_htmcs_txdata;

	if (mcs_limit & CFG_CTRL_MASK) {
		WMA_LOGD("%s: set mcs_limit %x", __func__, mcs_limit);

		mcs_limit &= CFG_DATA_MASK;
		for (i = 0, j = 0; i < MAX_SUPPORTED_RATES;) {
			if (j < mcs_limit / 8) {
				rate_pos[j] = 0xff;
				j++;
				i += 8;
			} else if (j < mcs_limit / 8 + 1) {
				if (i <= mcs_limit)
					rate_pos[i / 8] |= 1 << (i % 8);
				else
					rate_pos[i / 8] &= ~(1 << (i % 8));
				i++;

				if (i >= (j + 1) * 8)
					j++;
			} else {
				rate_pos[j++] = 0;
				i += 8;
			}
		}
	}
}

#if SUPPORT_11AX
/**
 * wma_fw_to_host_phymode_11ac() - convert fw to host phymode for 11ax phymodes
 * @wma:     wma handle
 * @phymode: phymode to convert
 *
 * Return: None
 */
static enum wlan_phymode wma_fw_to_host_phymode_11ac(WLAN_PHY_MODE phymode)
{
	switch (phymode) {
	default:
		return WLAN_PHYMODE_AUTO;
	case MODE_11AX_HE20:
		return WLAN_PHYMODE_11AC_VHT20;
	case MODE_11AX_HE40:
		return WLAN_PHYMODE_11AC_VHT40;
	case MODE_11AX_HE80:
		return WLAN_PHYMODE_11AC_VHT80;
	case MODE_11AX_HE80_80:
		return WLAN_PHYMODE_11AC_VHT80_80;
	case MODE_11AX_HE160:
		return WLAN_PHYMODE_11AC_VHT160;
	case MODE_11AX_HE20_2G:
		return WLAN_PHYMODE_11AC_VHT20;
	case MODE_11AX_HE40_2G:
		return WLAN_PHYMODE_11AC_VHT40;
	case MODE_11AX_HE80_2G:
		return WLAN_PHYMODE_11AC_VHT80;
	}
	return WLAN_PHYMODE_AUTO;
}
#else
static enum wlan_phymode wma_fw_to_host_phymode_11ac(WLAN_PHY_MODE phymode)
{
	return WLAN_PHYMODE_AUTO;
}
#endif

#ifdef CONFIG_160MHZ_SUPPORT
/**
 * wma_fw_to_host_phymode_160() - convert fw to host phymode for 160 mhz
 * phymodes
 * @wma:     wma handle
 * @phymode: phymode to convert
 *
 * Return: None
 */
static enum wlan_phymode wma_fw_to_host_phymode_160(WLAN_PHY_MODE phymode)
{
	switch (phymode) {
	default:
		return WLAN_PHYMODE_AUTO;
	case MODE_11AC_VHT80_80:
		return WLAN_PHYMODE_11AC_VHT80_80;
	case MODE_11AC_VHT160:
		return WLAN_PHYMODE_11AC_VHT160;
	}
}
#else
static enum wlan_phymode wma_fw_to_host_phymode_160(WLAN_PHY_MODE phymode)
{
	return WLAN_PHYMODE_AUTO;
}
#endif
/**
 * wma_fw_to_host_phymode() - convert fw to host phymode
 * @wma:     wma handle
 * @phymode: phymode to convert
 *
 * Return: None
 */
static enum wlan_phymode wma_fw_to_host_phymode(WLAN_PHY_MODE phymode)
{
	enum wlan_phymode host_phymode;
	switch (phymode) {
	default:
		host_phymode = wma_fw_to_host_phymode_160(phymode);
		if (host_phymode != WLAN_PHYMODE_AUTO)
			return host_phymode;
		return wma_fw_to_host_phymode_11ac(phymode);
	case MODE_11A:
		return WLAN_PHYMODE_11A;
	case MODE_11G:
		return WLAN_PHYMODE_11G;
	case MODE_11B:
		return WLAN_PHYMODE_11B;
	case MODE_11GONLY:
		return WLAN_PHYMODE_11G;
	case MODE_11NA_HT20:
		return WLAN_PHYMODE_11NA_HT20;
	case MODE_11NG_HT20:
		return WLAN_PHYMODE_11NG_HT20;
	case MODE_11NA_HT40:
		return WLAN_PHYMODE_11NA_HT40;
	case MODE_11NG_HT40:
		return WLAN_PHYMODE_11NG_HT40;
	case MODE_11AC_VHT20:
		return WLAN_PHYMODE_11AC_VHT20;
	case MODE_11AC_VHT40:
		return WLAN_PHYMODE_11AC_VHT40;
	case MODE_11AC_VHT80:
		return WLAN_PHYMODE_11AC_VHT80;
	case MODE_11AC_VHT20_2G:
		return WLAN_PHYMODE_11AC_VHT20;
	case MODE_11AC_VHT40_2G:
		return WLAN_PHYMODE_11AC_VHT40;
	case MODE_11AC_VHT80_2G:
		return WLAN_PHYMODE_11AC_VHT80;
	}
}

/**
 * wma_objmgr_set_peer_mlme_phymode() - set phymode to peer object
 * @wma:      wma handle
 * @mac_addr: mac addr of peer
 * @phymode:  phymode value to set
 *
 * Return: None
 */
static void wma_objmgr_set_peer_mlme_phymode(tp_wma_handle wma,
					     uint8_t *mac_addr,
					     WLAN_PHY_MODE phymode)
{
	uint8_t pdev_id;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_psoc *psoc = wma->psoc;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(wma->pdev);
	peer = wlan_objmgr_get_peer(psoc, pdev_id, mac_addr,
				    WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE(FL("peer object null"));
		return;
	}

	wlan_peer_obj_lock(peer);
	wlan_peer_set_phymode(peer, wma_fw_to_host_phymode(phymode));
	wlan_peer_obj_unlock(peer);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);
}

/**
 * wma_objmgr_set_peer_mlme_type() - set peer type to peer object
 * @wma:      wma handle
 * @mac_addr: mac addr of peer
 * @peer_type:  peer type value to set
 *
 * Return: None
 */
static void wma_objmgr_set_peer_mlme_type(tp_wma_handle wma,
					  uint8_t *mac_addr,
					  enum wlan_peer_type peer_type)
{
	uint8_t pdev_id;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_psoc *psoc = wma->psoc;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(wma->pdev);
	peer = wlan_objmgr_get_peer(psoc, pdev_id, mac_addr,
				    WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE(FL("peer object null"));
		return;
	}

	wlan_peer_obj_lock(peer);
	wlan_peer_set_peer_type(peer, peer_type);
	wlan_peer_obj_unlock(peer);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);
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
 * Return: QDF_STATUS
 */
QDF_STATUS wma_send_peer_assoc(tp_wma_handle wma,
				    tSirNwType nw_type,
				    tpAddStaParams params)
{
	struct cdp_pdev *pdev;
	struct peer_assoc_params *cmd;
	int32_t ret, max_rates, i;
	uint8_t *rate_pos;
	wmi_rate_set peer_legacy_rates, peer_ht_rates;
	uint32_t num_peer_11b_rates = 0;
	uint32_t num_peer_11a_rates = 0;
	uint32_t phymode;
	uint32_t peer_nss = 1;
	struct wma_txrx_node *intr = NULL;
	bool is_he;
	QDF_STATUS status;
	struct mac_context *mac = (struct mac_context *)wma->mac_context;

	cmd = qdf_mem_malloc(sizeof(struct peer_assoc_params));
	if (!cmd) {
		WMA_LOGE("Failed to allocate peer_assoc_params param");
		return QDF_STATUS_E_NOMEM;
	}

	intr = &wma->interfaces[params->smesessionId];

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		qdf_mem_free(cmd);
		return QDF_STATUS_E_INVAL;
	}

	wma_mask_tx_ht_rate(wma, params->supportedRates.supportedMCSSet);

	qdf_mem_zero(&peer_legacy_rates, sizeof(wmi_rate_set));
	qdf_mem_zero(&peer_ht_rates, sizeof(wmi_rate_set));
	qdf_mem_zero(cmd, sizeof(struct peer_assoc_params));

	is_he = wma_is_peer_he_capable(params);
	if ((params->ch_width > CH_WIDTH_40MHZ) &&
	    ((nw_type == eSIR_11G_NW_TYPE) ||
	     (nw_type == eSIR_11B_NW_TYPE))) {
		WMA_LOGE("ch_width %d sent in 11G, configure to 40MHz",
			 params->ch_width);
		params->ch_width = CH_WIDTH_40MHZ;
	}
	phymode = wma_peer_phymode(nw_type, params->staType,
				   params->htCapable, params->ch_width,
				   params->vhtCapable, is_he);

	wma_objmgr_set_peer_mlme_phymode(wma, params->staMac, phymode);

	if (!mac->mlme_cfg->rates.disable_abg_rate_txdata) {
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
	}

	if ((phymode == MODE_11A && num_peer_11a_rates == 0) ||
	    (phymode == MODE_11B && num_peer_11b_rates == 0)) {
		WMA_LOGW("%s: Invalid phy rates. phymode 0x%x, 11b_rates %d, 11a_rates %d",
			__func__, phymode, num_peer_11b_rates,
			num_peer_11a_rates);
		qdf_mem_free(cmd);
		return QDF_STATUS_E_INVAL;
	}

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
		qdf_mem_copy((uint8_t *) peer_ht_rates.rates, temp_ni_rates,
			     peer_ht_rates.num_rates);
	}

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
		qdf_mem_copy(cmd->peer_mac, params->staMac,
						sizeof(cmd->peer_mac));
	else
		qdf_mem_copy(cmd->peer_mac, params->bssId,
						sizeof(cmd->peer_mac));

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

		if (params->ch_width) {
			cmd->peer_flags |= WMI_PEER_40MHZ;
			cmd->peer_rate_caps |= WMI_RC_CW40_FLAG;
			if (params->fShortGI40Mhz)
				cmd->peer_rate_caps |= WMI_RC_SGI_FLAG;
		} else if (params->fShortGI20Mhz) {
			cmd->peer_rate_caps |= WMI_RC_SGI_FLAG;
		}
	}

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
	if (params->p2pCapableSta) {
		cmd->peer_flags |= WMI_PEER_IS_P2P_CAPABLE;
		wma_objmgr_set_peer_mlme_type(wma, params->staMac,
					      WLAN_PEER_P2P_CLI);
	}

	if (params->rmfEnabled)
		cmd->peer_flags |= WMI_PEER_PMF;

	if (params->stbc_capable)
		cmd->peer_flags |= WMI_PEER_STBC;

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

	wma_set_twt_peer_caps(params, cmd);
#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType)
		cmd->peer_flags |= WMI_PEER_AUTH;
#endif /* FEATURE_WLAN_TDLS */

	if (params->wpa_rsn
#ifdef FEATURE_WLAN_WAPI
	    || params->encryptType == eSIR_ED_WPI
#endif /* FEATURE_WLAN_WAPI */
	    ) {
		cmd->peer_flags |= WMI_PEER_NEED_PTK_4_WAY;
		WMA_LOGD("Acquire set key wake lock for %d ms",
			WMA_VDEV_SET_KEY_WAKELOCK_TIMEOUT);
		wma_acquire_wakelock(&intr->vdev_set_key_wakelock,
			WMA_VDEV_SET_KEY_WAKELOCK_TIMEOUT);
	}
	if (params->wpa_rsn >> 1)
		cmd->peer_flags |= WMI_PEER_NEED_GTK_2_WAY;

	wma_unified_peer_state_update(pdev, params->staMac,
				      params->bssId, params->staType);

#ifdef FEATURE_WLAN_WAPI
	if (params->encryptType == eSIR_ED_WPI) {
		ret = wma_vdev_set_param(wma->wmi_handle, params->smesessionId,
				      WMI_VDEV_PARAM_DROP_UNENCRY, false);
		if (ret) {
			WMA_LOGE
				("Set WMI_VDEV_PARAM_DROP_UNENCRY Param status:%d\n",
				ret);
			qdf_mem_free(cmd);
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
	cmd->peer_legacy_rates.num_rates = peer_legacy_rates.num_rates;
	qdf_mem_copy(cmd->peer_legacy_rates.rates, peer_legacy_rates.rates,
		     peer_legacy_rates.num_rates);

	/* Update peer HT rate information */
	cmd->peer_ht_rates.num_rates = peer_ht_rates.num_rates;
	qdf_mem_copy(cmd->peer_ht_rates.rates, peer_ht_rates.rates,
				 peer_ht_rates.num_rates);

	/* VHT Rates */

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

	cmd->vht_capable = params->vhtCapable;
	if (params->vhtCapable) {
#define VHT2x2MCSMASK 0xc
		cmd->rx_max_rate = params->supportedRates.vhtRxHighestDataRate;
		cmd->rx_mcs_set = params->supportedRates.vhtRxMCSMap;
		cmd->tx_max_rate = params->supportedRates.vhtTxHighestDataRate;
		cmd->tx_mcs_set = params->supportedRates.vhtTxMCSMap;

		if (params->vhtSupportedRxNss) {
			cmd->peer_nss = params->vhtSupportedRxNss;
		} else {
			cmd->peer_nss = ((cmd->rx_mcs_set & VHT2x2MCSMASK)
					 == VHT2x2MCSMASK) ? 1 : 2;
		}
	}

	WMA_LOGD(FL("rx_max_rate: %d, rx_mcs: %x, tx_max_rate: %d, tx_mcs: %x"),
		 cmd->rx_max_rate, cmd->rx_mcs_set, cmd->tx_max_rate,
		 cmd->tx_mcs_set);

	/*
	 * Limit nss to max number of rf chain supported by target
	 * Otherwise Fw will crash
	 */
	if (cmd->peer_nss > WMA_MAX_NSS)
		cmd->peer_nss = WMA_MAX_NSS;

	wma_populate_peer_he_cap(cmd, params);

	intr->nss = cmd->peer_nss;
	cmd->peer_phymode = phymode;
	WMA_LOGD("%s: vdev_id %d associd %d peer_flags %x rate_caps %x peer_caps %x",
		 __func__,  cmd->vdev_id, cmd->peer_associd, cmd->peer_flags,
		 cmd->peer_rate_caps, cmd->peer_caps);
	WMA_LOGD("%s:listen_intval %d ht_caps %x max_mpdu %d nss %d phymode %d",
		 __func__, cmd->peer_listen_intval, cmd->peer_ht_caps,
		 cmd->peer_max_mpdu, cmd->peer_nss, cmd->peer_phymode);
	WMA_LOGD("%s: peer_mpdu_density %d encr_type %d cmd->peer_vht_caps %x",
		 __func__, cmd->peer_mpdu_density, params->encryptType,
		 cmd->peer_vht_caps);

	status = wmi_unified_peer_assoc_send(wma->wmi_handle,
					 cmd);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGP(FL("Failed to send peer assoc command status = %d"),
			status);
	qdf_mem_free(cmd);

	return status;
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
QDF_STATUS wmi_unified_vdev_set_gtx_cfg_send(wmi_unified_t wmi_handle,
				  uint32_t if_id,
				  gtx_config_t *gtx_info)
{
	struct wmi_gtx_config params;

	params.gtx_rt_mask[0] = gtx_info->gtxRTMask[0];
	params.gtx_rt_mask[1] = gtx_info->gtxRTMask[1];
	params.gtx_usrcfg = gtx_info->gtxUsrcfg;
	params.gtx_threshold = gtx_info->gtxPERThreshold;
	params.gtx_margin = gtx_info->gtxPERMargin;
	params.gtx_tpcstep = gtx_info->gtxTPCstep;
	params.gtx_tpcmin = gtx_info->gtxTPCMin;
	params.gtx_bwmask = gtx_info->gtxBWMask;

	return wmi_unified_vdev_set_gtx_cfg_cmd(wmi_handle,
						if_id, &params);

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
	QDF_STATUS ret;
	enum ieee80211_protmode prot_mode;

	prot_mode = llbcoexist ? IEEE80211_PROT_CTSONLY : IEEE80211_PROT_NONE;

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_PROTECTION_MODE,
					      prot_mode);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to send wmi protection mode cmd");
	else
		WMA_LOGD("Updated protection mode %d to target", prot_mode);
}

void
wma_update_beacon_interval(tp_wma_handle wma, uint8_t vdev_id,
			   uint16_t beaconInterval)
{
	QDF_STATUS ret;

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_BEACON_INTERVAL,
					      beaconInterval);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to update beacon interval");
	else
		WMA_LOGI("Updated beacon interval %d for vdev %d",
			 beaconInterval, vdev_id);
}

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
/**
 * wma_update_bss_color() - update beacon bss color in fw
 * @wma: wma handle
 * @vdev_id: vdev id
 * @he_ops: HE operation, only the bss_color and bss_color_disabled fields
 * are updated.
 *
 * Return: none
 */
static void
wma_update_bss_color(tp_wma_handle wma, uint8_t vdev_id,
		     tUpdateBeaconParams *bcn_params)
{
	QDF_STATUS ret;
	uint32_t dword_he_ops = 0;

	WMI_HEOPS_COLOR_SET(dword_he_ops, bcn_params->bss_color);
	WMI_HEOPS_BSSCOLORDISABLE_SET(dword_he_ops,
				bcn_params->bss_color_disabled);
	WMA_LOGD("vdev: %d, update bss color, HE_OPS: 0x%x",
		vdev_id, dword_he_ops);
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
			      WMI_VDEV_PARAM_BSS_COLOR, dword_he_ops);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to update HE operations");
}
#else
static void wma_update_bss_color(tp_wma_handle wma, uint8_t vdev_id,
			   tUpdateBeaconParams *bcn_params)
{
}
#endif

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

	if (bcn_params->paramChangeBitmap & PARAM_BSS_COLOR_CHANGED)
		wma_update_bss_color(wma, bcn_params->smeSessionId,
				     bcn_params);
}

/**
 * wma_update_rts_params() - update cfg parameters to target
 * @wma: wma handle
 * @value: rts_threshold
 *
 * Return: none
 */
void wma_update_rts_params(tp_wma_handle wma, uint32_t value)
{
	uint8_t vdev_id;
	QDF_STATUS ret;

	for (vdev_id = 0; vdev_id < wma->max_bssid; vdev_id++) {
		if (wma->interfaces[vdev_id].handle != 0) {
			ret = wma_vdev_set_param(wma->wmi_handle,
						 vdev_id,
						 WMI_VDEV_PARAM_RTS_THRESHOLD,
						 value);
			if (QDF_IS_STATUS_ERROR(ret))
				WMA_LOGE("Update cfg param fail for vdevId %d",
					 vdev_id);
		}
	}
}

/**
 * wma_update_frag_params() - update cfg parameters to target
 * @wma: wma handle
 * @value: frag_threshold
 *
 * Return: none
 */
void wma_update_frag_params(tp_wma_handle wma, uint32_t value)
{
	uint8_t vdev_id;
	QDF_STATUS ret;

	for (vdev_id = 0; vdev_id < wma->max_bssid; vdev_id++) {
		if (wma->interfaces[vdev_id].handle != 0) {
			ret = wma_vdev_set_param(wma->wmi_handle,
					vdev_id,
					WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD,
					value);
			if (QDF_IS_STATUS_ERROR(ret))
				WMA_LOGE("Update cfg params failed for vdevId %d",
					vdev_id);
		}
	}
}

#ifndef CRYPTO_SET_KEY_CONVERGED
/**
 * wma_read_cfg_wepkey() - fill key_info for WEP key
 * @wma_handle: wma handle
 * @key_info: key_info ptr
 * @def_key_idx: default key index
 * @num_keys: number of keys
 * @vdev: vdev pointer
 *
 * This function reads WEP keys from cfg and fills
 * up key_info.
 *
 * Return: none
 */
static void wma_read_cfg_wepkey(tp_wma_handle wma_handle,
				tSirKeys *key_info, uint32_t *def_key_idx,
				uint8_t *num_keys,
				struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status;
	qdf_size_t val = SIR_MAC_KEY_LENGTH;
	uint8_t i, j;
	struct mac_context *mac_ctx = wma_handle->mac_context;

	WMA_LOGD("Reading WEP keys from cfg");

	/* NOTE:def_key_idx is initialized to 0 by the caller */
	*def_key_idx = mac_ctx->mlme_cfg->wep_params.wep_default_key_id;

	for (i = 0, j = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; i++) {
		status = mlme_get_wep_key(vdev, &mac_ctx->mlme_cfg->wep_params,
					  (MLME_WEP_DEFAULT_KEY_1 +
					  i), key_info[j].key, &val);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("WEP key is not configured at :%d", i);
		} else {
			key_info[j].keyId = i;
			key_info[j].keyLength = (uint16_t) val;
			j++;
		}
	}
	*num_keys = j;
}
#endif

#ifdef FEATURE_WLAN_WAPI
#define WPI_IV_LEN 16
#if defined(QCA_WIFI_QCA6290) || defined(QCA_WIFI_QCA6390)
/**
 * wma_fill_in_wapi_key_params() - update key parameters about wapi
 * @key_params: wma key parameters
 * @params: parameters pointer to be set
 * @mode: operation mode
 *
 * Return: None
 */
static inline void wma_fill_in_wapi_key_params(
		struct wma_set_key_params *key_params,
		struct set_key_params *params, uint8_t mode)
{
	/*
	 * Since MCL shares same FW with WIN for Napier/Hasting, FW WAPI logic
	 * is fit for WIN, change it to align with WIN.
	 */
	unsigned char iv_init_ap[16] = { 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
					 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
					 0x5c, 0x36, 0x5c, 0x37};
	unsigned char iv_init_sta[16] = { 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
					  0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
					  0x5c, 0x36, 0x5c, 0x36};

	if (mode == wlan_op_mode_ap) {
		qdf_mem_copy(params->rx_iv, iv_init_sta,
			     WPI_IV_LEN);
		qdf_mem_copy(params->tx_iv, iv_init_ap,
			     WPI_IV_LEN);
	} else {
		qdf_mem_copy(params->rx_iv, iv_init_ap,
			     WPI_IV_LEN);
		qdf_mem_copy(params->tx_iv, iv_init_sta,
			     WPI_IV_LEN);
	}

	params->key_txmic_len = WMA_TXMIC_LEN;
	params->key_rxmic_len = WMA_RXMIC_LEN;

	params->key_cipher = WMI_CIPHER_WAPI;
}
#else
static inline void wma_fill_in_wapi_key_params(
		struct wma_set_key_params *key_params,
		struct set_key_params *params, uint8_t mode)
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
		 * 0x5C365C365C365C365C365C365C365C36 for MCastkeyUpdate
		 */
		if (key_params->unicast)
			tx_iv[0] = 0x37;

		rx_iv[WPI_IV_LEN - 1] = 0x36;
	} else {
		if (!key_params->unicast)
			rx_iv[WPI_IV_LEN - 1] = 0x36;
	}

	params->key_txmic_len = WMA_TXMIC_LEN;
	params->key_rxmic_len = WMA_RXMIC_LEN;

	qdf_mem_copy(params->rx_iv, &rx_iv,
		     WPI_IV_LEN);
	qdf_mem_copy(params->tx_iv, &tx_iv,
		     WPI_IV_LEN);
	params->key_cipher = WMI_CIPHER_WAPI;
}
#endif
#endif

#ifndef CRYPTO_SET_KEY_CONVERGED
/**
 * wma_skip_bip_key_set() - skip the BIP key step or not
 * @wma_handle: wma handle
 * @iface: txrx node
 * @key_cipher: key cipher
 *
 * if target not support the BIP key cipher, skip the set key command.
 *
 * Return: true to skip set key to target, otherwise set key to target
 */
static bool
wma_skip_bip_key_set(tp_wma_handle wma_handle, uint32_t key_cipher)
{
	if ((key_cipher == WMI_CIPHER_AES_GMAC) &&
	    !wmi_service_enabled(wma_handle->wmi_handle,
				 wmi_service_gmac_offload_support))
		return true;
	return false;
}

static void wma_set_peer_unicast_cipher(tp_wma_handle wma,
					struct set_key_params *params)
{
	struct wlan_objmgr_peer *peer;

	peer = wlan_objmgr_get_peer(wma->psoc,
				    wlan_objmgr_pdev_get_pdev_id(wma->pdev),
				    params->peer_mac, WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE("Peer of peer_mac %pM not found", params->peer_mac);
		return;
	}

	wlan_peer_set_unicast_cipher(peer, params->key_cipher);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);
}

/**
 * wma_setup_install_key_cmd() - set key parameters
 * @wma_handle: wma handle
 * @key_params: key parameters
 * @mode: op mode
 *
 * This function fills structure from information
 * passed in key_params.
 *
 * Return: QDF_STATUS_SUCCESS - success
	QDF_STATUS_E_FAILURE - failure
	QDF_STATUS_E_NOMEM - invalid request
 */
static QDF_STATUS wma_setup_install_key_cmd(tp_wma_handle wma_handle,
					   struct wma_set_key_params
					   *key_params, uint8_t mode)
{
	struct set_key_params params;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wma_txrx_node *iface = NULL;
	enum cdp_sec_type sec_type = cdp_sec_type_none;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct cdp_pdev *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct cdp_vdev *txrx_vdev;
	uint32_t pn[4] = {0, 0, 0, 0};
	uint8_t peer_id;
	struct cdp_peer *peer;
	bool skip_set_key;

	if ((key_params->key_type == eSIR_ED_NONE &&
	     key_params->key_len) || (key_params->key_type != eSIR_ED_NONE &&
				      !key_params->key_len)) {
		WMA_LOGE("%s:Invalid set key request", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	if (!wma_handle) {
		WMA_LOGE(FL("Invalid wma_handle for vdev_id: %d"),
			key_params->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	if (!wma_is_vdev_valid(key_params->vdev_id)) {
		WMA_LOGE("%s: vdev id:%d is not active ", __func__,
			 key_params->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	txrx_vdev = wma_find_vdev_by_id(wma_handle,
					key_params->vdev_id);
	peer = cdp_peer_find_by_addr(soc, txrx_pdev,
				key_params->peer_mac, &peer_id);
	iface = &wma_handle->interfaces[key_params->vdev_id];

	params.vdev_id = key_params->vdev_id;
	params.key_idx = key_params->key_idx;
	qdf_mem_copy(params.peer_mac, key_params->peer_mac, QDF_MAC_ADDR_SIZE);

#ifdef FEATURE_WLAN_WAPI
	qdf_mem_zero(params.tx_iv, 16);
	qdf_mem_zero(params.rx_iv, 16);
#endif
	params.key_txmic_len = 0;
	params.key_rxmic_len = 0;
	qdf_mem_copy(&params.key_rsc_ctr,
		     &key_params->key_rsc[0], sizeof(uint64_t));
	params.key_flags = 0;
	if (key_params->unicast)
		params.key_flags |= PAIRWISE_USAGE;
	else
		params.key_flags |= GROUP_USAGE;

	switch (key_params->key_type) {
	case eSIR_ED_NONE:
		params.key_cipher = WMI_CIPHER_NONE;
		sec_type = cdp_sec_type_none;
		break;
	case eSIR_ED_WEP40:
	case eSIR_ED_WEP104:
		params.key_cipher = WMI_CIPHER_WEP;
		if (key_params->unicast &&
		    params.key_idx == key_params->def_key_idx) {
			WMA_LOGD("STA Mode: cmd->key_flags |= TX_USAGE");
			params.key_flags |= TX_USAGE;
		} else if ((mode == wlan_op_mode_ap) &&
			(params.key_idx == key_params->def_key_idx)) {
			WMA_LOGD("AP Mode: cmd->key_flags |= TX_USAGE");
			params.key_flags |= TX_USAGE;
		}
		sec_type = cdp_sec_type_wep104;
		break;
	case eSIR_ED_TKIP:
		params.key_txmic_len = WMA_TXMIC_LEN;
		params.key_rxmic_len = WMA_RXMIC_LEN;
		params.key_cipher = WMI_CIPHER_TKIP;
		sec_type = cdp_sec_type_tkip;
		break;
#ifdef FEATURE_WLAN_WAPI
	case eSIR_ED_WPI:
	{
		wma_fill_in_wapi_key_params(key_params, &params, mode);
		break;
	}
#endif /* FEATURE_WLAN_WAPI */
	case eSIR_ED_CCMP:
		params.key_cipher = WMI_CIPHER_AES_CCM;
		sec_type = cdp_sec_type_aes_ccmp;
		break;
#ifdef WLAN_FEATURE_11W
	case eSIR_ED_AES_128_CMAC:
		params.key_cipher = WMI_CIPHER_AES_CMAC;
		break;
	case eSIR_ED_AES_GMAC_128:
	case eSIR_ED_AES_GMAC_256:
		params.key_cipher = WMI_CIPHER_AES_GMAC;
		break;
#endif /* WLAN_FEATURE_11W */
	/* Firmware uses length to detect GCMP 128/256*/
	case eSIR_ED_GCMP:
	case eSIR_ED_GCMP_256:
		params.key_cipher = WMI_CIPHER_AES_GCM;
		break;
	default:
		/* TODO: MFP ? */
		WMA_LOGE("%s:Invalid encryption type:%d", __func__,
			 key_params->key_type);
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

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

		destp = (uint32_t *) params.key_data;
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
	qdf_mem_copy((void *)params.key_data,
		     (const void *)key_params->key_data, key_params->key_len);
#endif /* BIG_ENDIAN_HOST */
	params.key_len = key_params->key_len;

#ifdef WLAN_FEATURE_11W
	iface = &wma_handle->interfaces[key_params->vdev_id];

	if ((key_params->key_type == eSIR_ED_AES_128_CMAC) ||
	   (key_params->key_type == eSIR_ED_AES_GMAC_128) ||
	   (key_params->key_type == eSIR_ED_AES_GMAC_256)) {
		if (iface) {
			iface->key.key_length = key_params->key_len;
			iface->key.key_cipher = params.key_cipher;
			qdf_mem_copy(iface->key.key,
				     (const void *)key_params->key_data,
				     iface->key.key_length);
			if ((params.key_idx == WMA_IGTK_KEY_INDEX_4) ||
			    (params.key_idx == WMA_IGTK_KEY_INDEX_5))
				qdf_mem_zero(iface->key.key_id[params.key_idx -
						    WMA_IGTK_KEY_INDEX_4].ipn,
					     CMAC_IPN_LEN);
		}
	}
#endif /* WLAN_FEATURE_11W */

	if (key_params->unicast)
		wma_set_peer_unicast_cipher(wma_handle, &params);

	WMA_LOGD("Key setup : vdev_id %d key_idx %d key_type %d key_len %d",
		 key_params->vdev_id, key_params->key_idx,
		 key_params->key_type, key_params->key_len);
	WMA_LOGD("unicast %d peer_mac %pM def_key_idx %d",
		 key_params->unicast, key_params->peer_mac,
		 key_params->def_key_idx);
	WMA_LOGD("keyrsc param %llu", params.key_rsc_ctr);

	/*
	 * To prevent from any replay-attack, PN number provided by
	 * upper layer is used.
	 *
	 * Plumb the PN number to HW which will be used to evaluate whether
	 * incoming traffic is not replayed.
	 *
	 * supplicant would have some thing like following, example:
	 *
	 * num = 0x123456789ABCDEFF (64 bit number)
	 * uint8_t keyrsc[16] would look like following
	 *
	 * bit  0      7      15      23      31      39      47      55      63
	 *      +------+-------+-------+-------+-------+-------+-------+-------+
	 * byte |  0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
	 *      +------+-------+-------+-------+-------+-------+-------+-------+
	 * value| 0xFF |  0XDE | 0xBC  | 0x9A  | 0x78  | 0x56  | 0x34  | 0x12  |
	 *      +------+-------+-------+-------+-------+-------+-------+-------+
	 */
	qdf_mem_copy(&pn[0],
		     &key_params->key_rsc[0], sizeof(pn));
	wma_debug("key_type[%s] pn[%x:%x:%x:%x]",
		  (key_params->unicast) ? "unicast" : "group",
		  key_params->key_rsc[3], key_params->key_rsc[2],
		  key_params->key_rsc[1], key_params->key_rsc[0]);
	cdp_set_pn_check(soc, txrx_vdev, peer, sec_type, pn);
	cdp_set_key(soc, peer, key_params->unicast,
		    (uint32_t *)(key_params->key_data +
				WMA_IV_KEY_LEN +
				WMA_TXMIC_LEN));

	skip_set_key = wma_skip_bip_key_set(wma_handle, params.key_cipher);
	if (!skip_set_key)
		status = wmi_unified_setup_install_key_cmd(
				wma_handle->wmi_handle, &params);

	if (!key_params->unicast) {
		/* Its GTK release the wake lock */
		WMA_LOGD("Release set key wake lock");
		wma_release_wakelock(&iface->vdev_set_key_wakelock);
	}

	/* install key was requested */
	if (iface)
		iface->is_waiting_for_key = false;

end:
	qdf_mem_zero(&params, sizeof(struct set_key_params));
	return status;
}
#endif

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
	 * entry value : the heart time threshold value in seconds for
	 * detecting ibss peer departure
	 */
	static const uint16_t heart_beat_timer[MAX_PEERS] = {
		4, 4, 4, 4, 4, 4, 4, 4,
		8, 8, 8, 8, 8, 8, 8, 8,
		12, 12, 12, 12, 12, 12, 12, 12,
		16, 16, 16, 16, 16, 16, 16, 16
	};

	if (peer_num < 1 || peer_num > MAX_PEERS)
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
	struct cdp_vdev *vdev;
	int16_t new_peer_num;
	uint16_t new_timer_value_sec;
	uint32_t new_timer_value_ms;
	QDF_STATUS status;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (peer_num_delta != 1 && peer_num_delta != -1) {
		WMA_LOGE("Invalid peer_num_delta value %d", peer_num_delta);
		return;
	}

	vdev = wma_find_vdev_by_id(wma, vdev_id);
	if (!vdev) {
		WMA_LOGE("vdev not found : vdev_id %d", vdev_id);
		return;
	}

	/* adjust peer numbers */
	new_peer_num = cdp_peer_update_ibss_add_peer_num_of_vdev(soc, vdev,
								 peer_num_delta
								 );
	if (OL_TXRX_INVALID_NUM_PEERS == new_peer_num) {
		WMA_LOGE("new peer num %d out of valid boundary", new_peer_num);
		return;
	}

	/* reset timer value if all peers departed */
	if (new_peer_num == 0) {
		cdp_set_ibss_vdev_heart_beat_timer(soc, vdev, 0);
		return;
	}

	/* calculate new timer value */
	new_timer_value_sec = wma_calc_ibss_heart_beat_timer(new_peer_num);
	if (new_timer_value_sec == 0) {
		WMA_LOGE("timer value %d is invalid for peer number %d",
			 new_timer_value_sec, new_peer_num);
		return;
	}
	if (new_timer_value_sec ==
	    cdp_set_ibss_vdev_heart_beat_timer(soc, vdev,
					       new_timer_value_sec)) {
		WMA_LOGD("timer value %d stays same, no need to notify target",
			 new_timer_value_sec);
		return;
	}

	new_timer_value_ms = ((uint32_t)new_timer_value_sec) * 1000;

	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
				    WMI_VDEV_PARAM_IBSS_MAX_BCN_LOST_MS,
				    new_timer_value_ms);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Failed to set IBSS link monitoring timer value");
		return;
	}

	WMA_LOGD("Set IBSS link monitor timer: peer_num = %d timer_value = %d",
		 new_peer_num, new_timer_value_ms);
}
#endif /* QCA_IBSS_SUPPORT */

#ifndef CRYPTO_SET_KEY_CONVERGED
void wma_set_bsskey(tp_wma_handle wma_handle, tpSetBssKeyParams key_info)
{
	struct wma_set_key_params key_params;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i;
	uint32_t def_key_idx = 0;
	uint32_t wlan_opmode;
	struct cdp_vdev *txrx_vdev;
	uint8_t *mac_addr;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_objmgr_vdev *vdev;

	WMA_LOGD("BSS key setup");
	txrx_vdev = wma_find_vdev_by_id(wma_handle, key_info->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		goto out;
	}
	wlan_opmode = cdp_get_opmode(soc, txrx_vdev);

	/*
	 * For IBSS, WMI expects the BSS key to be set per peer key
	 * So cache the BSS key in the wma_handle and re-use it when the
	 * STA key is been setup for a peer
	 */
	if (wlan_op_mode_ibss == wlan_opmode) {
		key_info->status = QDF_STATUS_SUCCESS;
		if (wma_handle->ibss_started > 0)
			goto out;
		WMA_LOGD("Caching IBSS Key");
		qdf_mem_copy(&wma_handle->ibsskey_info, key_info,
			     sizeof(tSetBssKeyParams));
	}

	qdf_mem_zero(&key_params, sizeof(key_params));
	key_params.vdev_id = key_info->smesessionId;
	key_params.key_type = key_info->encType;
	key_params.singl_tid_rc = key_info->singleTidRc;
	key_params.unicast = false;
	if (wlan_opmode == wlan_op_mode_sta) {
		qdf_mem_copy(key_params.peer_mac,
			wma_handle->interfaces[key_info->smesessionId].bssid,
			QDF_MAC_ADDR_SIZE);
	} else {
		mac_addr = cdp_get_vdev_mac_addr(soc,
					txrx_vdev);
		if (!mac_addr) {
			WMA_LOGE("%s: mac_addr is NULL for vdev with id %d",
				 __func__, key_info->smesessionId);
			goto out;
		}
		/* vdev mac address will be passed for all other modes */
		qdf_mem_copy(key_params.peer_mac, mac_addr,
			     QDF_MAC_ADDR_SIZE);
		WMA_LOGD("BSS Key setup with vdev_mac %pM\n",
			 mac_addr);
	}

	if (key_info->numKeys == 0 &&
	    (key_info->encType == eSIR_ED_WEP40 ||
	     key_info->encType == eSIR_ED_WEP104)) {
		vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wma_handle->psoc,
						     key_info->smesessionId,
						     WLAN_LEGACY_WMA_ID);
		wma_read_cfg_wepkey(wma_handle, key_info->key,
				    &def_key_idx, &key_info->numKeys, vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_WMA_ID);
	} else if ((key_info->encType == eSIR_ED_WEP40) ||
		   (key_info->encType == eSIR_ED_WEP104)) {
		struct wma_txrx_node *intf =
			&wma_handle->interfaces[key_info->smesessionId];
		key_params.def_key_idx = intf->wep_default_key_idx;
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
		qdf_mem_copy(key_params.key_rsc,
				key_info->key[i].keyRsc,
				WLAN_CRYPTO_RSC_SIZE);
		if (key_info->encType == eSIR_ED_TKIP) {
			qdf_mem_copy(key_params.key_data,
				     key_info->key[i].key, 16);
			qdf_mem_copy(&key_params.key_data[16],
				     &key_info->key[i].key[24], 8);
			qdf_mem_copy(&key_params.key_data[24],
				     &key_info->key[i].key[16], 8);
		} else
			qdf_mem_copy((void *)key_params.key_data,
				     (const void *)key_info->key[i].key,
				     key_info->key[i].keyLength);

		WMA_LOGD("%s: bss key[%d] length %d", __func__, i,
			 key_info->key[i].keyLength);

		status = wma_setup_install_key_cmd(wma_handle, &key_params,
						   wlan_opmode);
		if (status == QDF_STATUS_E_NOMEM) {
			WMA_LOGE("%s:Failed to setup install key buf",
				 __func__);
			key_info->status = QDF_STATUS_E_NOMEM;
			goto out;
		} else if (status == QDF_STATUS_E_FAILURE) {
			WMA_LOGE("%s:Failed to send install key command",
				 __func__);
			key_info->status = QDF_STATUS_E_FAILURE;
			goto out;
		}
	}

	wma_handle->ibss_started++;
	/* TODO: Should we wait till we get HTT_T2H_MSG_TYPE_SEC_IND? */
	key_info->status = QDF_STATUS_SUCCESS;

	qdf_mem_zero(&key_params, sizeof(struct wma_set_key_params));

out:
	wma_send_msg_high_priority(wma_handle, WMA_SET_BSSKEY_RSP,
				   (void *)key_info, 0);
}

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
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i;
	uint32_t def_key_idx = 0;
	struct cdp_vdev *txrx_vdev;
	int opmode;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_objmgr_vdev *vdev;

	WMA_LOGD("BSS key setup for peer");
	txrx_vdev = wma_find_vdev_by_id(wma_handle, key_info->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		return;
	}

	qdf_mem_zero(&key_params, sizeof(key_params));
	opmode = cdp_get_opmode(soc, txrx_vdev);
	qdf_mem_zero(&key_params, sizeof(key_params));
	key_params.vdev_id = key_info->smesessionId;
	key_params.key_type = key_info->encType;
	key_params.singl_tid_rc = key_info->singleTidRc;
	key_params.unicast = false;
	ASSERT(wlan_op_mode_ibss == opmode);

	qdf_mem_copy(key_params.peer_mac, peer_macaddr.bytes,
			QDF_MAC_ADDR_SIZE);

	if (key_info->numKeys == 0 &&
	    (key_info->encType == eSIR_ED_WEP40 ||
	     key_info->encType == eSIR_ED_WEP104)) {
		vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wma_handle->psoc,
						     key_info->smesessionId,
						     WLAN_LEGACY_WMA_ID);
		wma_read_cfg_wepkey(wma_handle, key_info->key,
				    &def_key_idx, &key_info->numKeys, vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_WMA_ID);
	} else if ((key_info->encType == eSIR_ED_WEP40) ||
		(key_info->encType == eSIR_ED_WEP104)) {
		struct wma_txrx_node *intf =
			&wma_handle->interfaces[key_info->smesessionId];
		key_params.def_key_idx = intf->wep_default_key_idx;
	}

	for (i = 0; i < key_info->numKeys; i++) {
		if (key_params.key_type != eSIR_ED_NONE &&
		    !key_info->key[i].keyLength)
			continue;
		key_params.key_idx = key_info->key[i].keyId;
		key_params.key_len = key_info->key[i].keyLength;
		if (key_info->encType == eSIR_ED_TKIP) {
			qdf_mem_copy(key_params.key_data,
				     key_info->key[i].key, 16);
			qdf_mem_copy(&key_params.key_data[16],
				     &key_info->key[i].key[24], 8);
			qdf_mem_copy(&key_params.key_data[24],
				     &key_info->key[i].key[16], 8);
		} else
			qdf_mem_copy((void *)key_params.key_data,
				     (const void *)key_info->key[i].key,
				     key_info->key[i].keyLength);

		WMA_LOGD("%s: peer bcast key[%d] length %d", __func__, i,
			 key_info->key[i].keyLength);

		status = wma_setup_install_key_cmd(wma_handle, &key_params,
						   opmode);
		if (status == QDF_STATUS_E_NOMEM) {
			WMA_LOGE("%s:Failed to setup install key buf",
				 __func__);
			return;
		} else if (status == QDF_STATUS_E_FAILURE) {
			WMA_LOGE("%s:Failed to send install key command",
				 __func__);
		}
	}
}

void wma_set_stakey(tp_wma_handle wma_handle, tpSetStaKeyParams key_info)
{
	int32_t i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct cdp_pdev *txrx_pdev;
	struct cdp_vdev *txrx_vdev;
	void *peer;
	uint8_t num_keys = 0, peer_id;
	struct wma_set_key_params key_params;
	uint32_t def_key_idx = 0;
	int opmode;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_objmgr_vdev *vdev;

	WMA_LOGD("STA key setup");

	/* Get the txRx Pdev handle */
	txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!txrx_pdev) {
		WMA_LOGE("%s:Invalid txrx pdev handle", __func__);
		key_info->status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	peer = cdp_peer_find_by_addr(soc, txrx_pdev,
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
	opmode = cdp_get_opmode(soc, txrx_vdev);

	if (key_info->defWEPIdx == WMA_INVALID_KEY_IDX &&
	    (key_info->encType == eSIR_ED_WEP40 ||
	     key_info->encType == eSIR_ED_WEP104) &&
	    opmode != wlan_op_mode_ap) {
		vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wma_handle->psoc,
						     key_info->smesessionId,
						     WLAN_LEGACY_WMA_ID);
		wma_read_cfg_wepkey(wma_handle, key_info->key,
				    &def_key_idx, &num_keys, vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_WMA_ID);
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
	qdf_mem_zero(&key_params, sizeof(key_params));
	key_params.vdev_id = key_info->smesessionId;
	key_params.key_type = key_info->encType;
	key_params.singl_tid_rc = key_info->singleTidRc;
	key_params.unicast = true;
	key_params.def_key_idx = key_info->defWEPIdx;
	qdf_mem_copy((void *)key_params.peer_mac,
		     (const void *)key_info->peer_macaddr.bytes,
		     QDF_MAC_ADDR_SIZE);
	for (i = 0; i < num_keys; i++) {
		if (key_params.key_type != eSIR_ED_NONE &&
		    !key_info->key[i].keyLength)
			continue;
		if (key_info->encType == eSIR_ED_TKIP) {
			qdf_mem_copy(key_params.key_data,
				     key_info->key[i].key, 16);
			qdf_mem_copy(&key_params.key_data[16],
				     &key_info->key[i].key[24], 8);
			qdf_mem_copy(&key_params.key_data[24],
				     &key_info->key[i].key[16], 8);
		} else
			qdf_mem_copy(key_params.key_data, key_info->key[i].key,
				     key_info->key[i].keyLength);
		if (key_info->encType == eSIR_ED_WPI) {
			key_params.key_idx = key_info->key[i].keyId;
			key_params.def_key_idx = key_info->key[i].keyId;
		} else
			key_params.key_idx = i;

		key_params.key_len = key_info->key[i].keyLength;
		status = wma_setup_install_key_cmd(wma_handle, &key_params,
						   opmode);
		if (status == QDF_STATUS_E_NOMEM) {
			WMA_LOGE("%s:Failed to setup install key buf",
				 __func__);
			key_info->status = QDF_STATUS_E_NOMEM;
			goto out;
		}

		WMA_LOGD("%s: peer unicast key[%d] %d ", __func__, i,
			 key_info->key[i].keyLength);

		if (status == QDF_STATUS_E_FAILURE) {
			WMA_LOGE("%s:Failed to send install key command",
				 __func__);
			key_info->status = QDF_STATUS_E_FAILURE;
			goto out;
		}
	}

	/* In IBSS mode, set the BSS KEY for this peer
	 * BSS key is supposed to be cache into wma_handle
	 */
	if (wlan_op_mode_ibss == opmode) {
		wma_set_ibsskey_helper(wma_handle, &wma_handle->ibsskey_info,
				       key_info->peer_macaddr);
	}

	/* TODO: Should we wait till we get HTT_T2H_MSG_TYPE_SEC_IND? */
	key_info->status = QDF_STATUS_SUCCESS;
out:
	qdf_mem_zero(&key_params, sizeof(struct wma_set_key_params));
	if (key_info->sendRsp)
		wma_send_msg_high_priority(wma_handle, WMA_SET_STAKEY_RSP,
					   (void *)key_info, 0);
}
#endif

/**
 * wma_process_update_edca_param_req() - update EDCA params
 * @handle: wma handle
 * @edca_params: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: QDF Status
 */
QDF_STATUS wma_process_update_edca_param_req(WMA_HANDLE handle,
					     tEdcaParams *edca_params)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct wmi_host_wme_vparams wmm_param[QCA_WLAN_AC_ALL];
	tSirMacEdcaParamRecord *edca_record;
	int ac;
	struct cdp_pdev *pdev;
	struct ol_tx_wmm_param_t ol_tx_wmm_param;
	uint8_t vdev_id;
	QDF_STATUS status;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	vdev_id = edca_params->bssIdx;
	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: vdev id:%d is not active ", __func__, vdev_id);
		goto fail;
	}

	for (ac = 0; ac < QCA_WLAN_AC_ALL; ac++) {
		switch (ac) {
		case QCA_WLAN_AC_BE:
			edca_record = &edca_params->acbe;
			break;
		case QCA_WLAN_AC_BK:
			edca_record = &edca_params->acbk;
			break;
		case QCA_WLAN_AC_VI:
			edca_record = &edca_params->acvi;
			break;
		case QCA_WLAN_AC_VO:
			edca_record = &edca_params->acvo;
			break;
		default:
			goto fail;
		}

		wma_update_edca_params_for_ac(edca_record, &wmm_param[ac], ac,
				edca_params->mu_edca_params);

		ol_tx_wmm_param.ac[ac].aifs = wmm_param[ac].aifs;
		ol_tx_wmm_param.ac[ac].cwmin = wmm_param[ac].cwmin;
		ol_tx_wmm_param.ac[ac].cwmax = wmm_param[ac].cwmax;
	}

	status = wmi_unified_process_update_edca_param(wma_handle->wmi_handle,
						vdev_id,
						edca_params->mu_edca_params,
						wmm_param);
	if (status == QDF_STATUS_E_NOMEM)
		return status;
	else if (status == QDF_STATUS_E_FAILURE)
		goto fail;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (pdev)
		cdp_set_wmm_param(soc, (struct cdp_pdev *)pdev,
				 ol_tx_wmm_param);
	else
		QDF_ASSERT(0);

	return QDF_STATUS_SUCCESS;

fail:
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
	uint64_t adjusted_tsf_le;
	struct ieee80211_frame *wh;
	struct wmi_probe_resp_params params;

	WMA_LOGD(FL("Send probe response template for vdev %d"), vdev_id);

	/*
	 * Make the TSF offset negative so probe response in the same
	 * staggered batch have the same TSF.
	 */
	adjusted_tsf_le = cpu_to_le64(0ULL -
				      wma->interfaces[vdev_id].tsfadjust);
	/* Update the timstamp in the probe response buffer with adjusted TSF */
	wh = (struct ieee80211_frame *)probe_rsp_info->probeRespTemplate;
	A_MEMCPY(&wh[1], &adjusted_tsf_le, sizeof(adjusted_tsf_le));

	params.prb_rsp_template_len = probe_rsp_info->probeRespTemplateLen;
	params.prb_rsp_template_frm = probe_rsp_info->probeRespTemplate;

	return wmi_unified_probe_rsp_tmpl_send_cmd(wma->wmi_handle, vdev_id,
						   &params);
}

/**
 * wma_unified_bcn_tmpl_send() - send beacon template to fw
 * @wma:wma handle
 * @vdev_id: vdev id
 * @bcn_info: beacon info
 * @bytes_to_strip: bytes to strip
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS wma_unified_bcn_tmpl_send(tp_wma_handle wma,
				     uint8_t vdev_id,
				     const tpSendbeaconParams bcn_info,
				     uint8_t bytes_to_strip)
{
	struct beacon_tmpl_params params = {0};
	uint32_t tmpl_len, tmpl_len_aligned;
	uint8_t *frm;
	QDF_STATUS ret;
	uint8_t *p2p_ie;
	uint16_t p2p_ie_len = 0;
	uint64_t adjusted_tsf_le;
	struct ieee80211_frame *wh;

	if (!wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: vdev id:%d is not active ", __func__, vdev_id);
		return QDF_STATUS_E_INVAL;
	}

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
	if (p2p_ie_len)
		tmpl_len -= (uint32_t) p2p_ie_len;
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



	params.vdev_id = vdev_id;
	params.tim_ie_offset = bcn_info->timIeOffset - bytes_to_strip;
	params.tmpl_len = tmpl_len;
	params.frm = frm;
	params.tmpl_len_aligned = tmpl_len_aligned;
	if (bcn_info->csa_count_offset &&
	    (bcn_info->csa_count_offset > bytes_to_strip))
		params.csa_switch_count_offset =
			bcn_info->csa_count_offset - bytes_to_strip;
	if (bcn_info->ecsa_count_offset &&
	    (bcn_info->ecsa_count_offset > bytes_to_strip))
		params.ext_csa_switch_count_offset =
			bcn_info->ecsa_count_offset - bytes_to_strip;

	ret = wmi_unified_beacon_tmpl_send_cmd(wma->wmi_handle,
				 &params);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("%s: Failed to send bcn tmpl: %d", __func__, ret);

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
 * Return: QDF status
 */
static QDF_STATUS wma_store_bcn_tmpl(tp_wma_handle wma, uint8_t vdev_id,
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

	len = *(uint32_t *) &bcn_info->beacon[0];
	if (len > SIR_MAX_BEACON_SIZE - sizeof(uint32_t)) {
		WMA_LOGE("%s: Received beacon len %u exceeding max limit %lu",
			 __func__, len, (unsigned long)(
			 SIR_MAX_BEACON_SIZE - sizeof(uint32_t)));
		return QDF_STATUS_E_INVAL;
	}
	WMA_LOGD("%s: Storing received beacon template buf to local buffer",
		 __func__);
	qdf_spin_lock_bh(&bcn->lock);

	/*
	 * Copy received beacon template content in local buffer.
	 * this will be send to target on the reception of SWBA
	 * event from target.
	 */
	qdf_nbuf_trim_tail(bcn->buf, qdf_nbuf_len(bcn->buf));
	memcpy(qdf_nbuf_data(bcn->buf),
	       bcn_info->beacon + 4 /* Exclude beacon length field */,
	       len);
	if (bcn_info->timIeOffset > 3)
		bcn->tim_ie_offset = bcn_info->timIeOffset - 4;
	else
		bcn->tim_ie_offset = bcn_info->timIeOffset;

	if (bcn_info->p2pIeOffset > 3)
		bcn->p2p_ie_offset = bcn_info->p2pIeOffset - 4;
	else
		bcn->p2p_ie_offset = bcn_info->p2pIeOffset;

	bcn_payload = qdf_nbuf_data(bcn->buf);
	if (bcn->tim_ie_offset) {
		tim_ie = (struct beacon_tim_ie *)
				(&bcn_payload[bcn->tim_ie_offset]);
		/*
		 * Initial Value of bcn->dtim_count will be 0.
		 * But if the beacon gets updated then current dtim
		 * count will be restored
		 */
		tim_ie->dtim_count = bcn->dtim_count;
		tim_ie->tim_bitctl = 0;
	}

	qdf_nbuf_put_tail(bcn->buf, len);
	bcn->len = len;

	qdf_spin_unlock_bh(&bcn->lock);

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

	for (; (if_id < wma->max_bssid && vdev_map); vdev_map >>= 1, if_id++) {
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

		qdf_spin_lock_bh(&bcn->lock);
		qdf_mem_zero(&bcn_info, sizeof(bcn_info));
		qdf_mem_copy(bcn_info.beacon,
			     qdf_nbuf_data(bcn->buf), bcn->len);
		bcn_info.p2pIeOffset = bcn->p2p_ie_offset;
		bcn_info.beaconLength = bcn->len;
		bcn_info.timIeOffset = bcn->tim_ie_offset;
		qdf_spin_unlock_bh(&bcn->lock);

		/* Update beacon template in firmware */
		wma_unified_bcn_tmpl_send(wma, if_id, &bcn_info, 0);
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
	if (!wma_handle) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_p2p_go_set_beacon_ie_cmd(wma_handle->wmi_handle,
							vdev_id, p2pIe);
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
	struct cdp_vdev *vdev;
	uint8_t vdev_id;
	struct sAniProbeRspStruct *probe_rsp;

	if (!probe_rsp_info) {
		WMA_LOGE(FL("probe_rsp_info is NULL"));
		return;
	}

	probe_rsp = (struct sAniProbeRspStruct *)
			(probe_rsp_info->probeRespTemplate);
	if (!probe_rsp) {
		WMA_LOGE(FL("probe_rsp is NULL"));
		return;
	}

	vdev = wma_find_vdev_by_addr(wma, probe_rsp->macHdr.sa, &vdev_id);
	if (!vdev) {
		WMA_LOGE(FL("failed to get vdev handle"));
		return;
	}

	if (wmi_service_enabled(wma->wmi_handle,
				   wmi_service_beacon_offload)) {
		WMA_LOGD("Beacon Offload Enabled Sending Unified command");
		if (wmi_unified_probe_rsp_tmpl_send(wma, vdev_id,
						    probe_rsp_info) < 0) {
			WMA_LOGE(FL("wmi_unified_probe_rsp_tmpl_send Failed "));
			return;
		}
	}
}

#ifdef CONFIG_VDEV_SM
QDF_STATUS wma_set_ap_vdev_up(tp_wma_handle wma, uint8_t vdev_id)
{
	struct vdev_up_params param = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	param.vdev_id = vdev_id;
	param.assoc_id = 0;
	status = wma_send_vdev_up_to_fw(wma, &param,
					wma->interfaces[vdev_id].bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE(FL("failed to send vdev up"));
		policy_mgr_set_do_hw_mode_change_flag(
			wma->psoc, false);
		return status;
	}
	wma_set_sap_keepalive(wma, vdev_id);
	wma_set_vdev_mgmt_rate(wma, vdev_id);

	return status;
}

static inline
QDF_STATUS wma_ap_vdev_up(tp_wma_handle wma, uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS wma_set_ap_vdev_up(tp_wma_handle wma, uint8_t vdev_id)
{
	struct vdev_up_params param = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!wma_get_hidden_ssid_restart_in_progress(
	     &wma->interfaces[vdev_id]) ||
	    (wma->interfaces[vdev_id].is_channel_switch)) {
		if (wma_is_vdev_up(vdev_id))
			return status;
		param.vdev_id = vdev_id;
		param.assoc_id = 0;
		status = wma_send_vdev_up_to_fw(wma, &param,
						wma->interfaces[vdev_id].bssid);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE(FL("failed to send vdev up"));
			policy_mgr_set_do_hw_mode_change_flag(
				wma->psoc, false);
			return status;
		}
		wma_vdev_set_mlme_state_run(wma, vdev_id);
		wma_set_sap_keepalive(wma, vdev_id);
		wma_set_vdev_mgmt_rate(wma, vdev_id);
	}

	return status;
}

/**
 * wma_ap_vdev_up() - wrapper for vdev up req
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS wma_ap_vdev_up(tp_wma_handle wma, uint8_t vdev_id)
{
	return wma_set_ap_vdev_up(wma, vdev_id);
}
#endif

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
	struct cdp_vdev *vdev;
	uint8_t vdev_id;
	QDF_STATUS status;
	uint8_t *p2p_ie;
	struct sAniBeaconStruct *beacon;

	WMA_LOGD("Beacon update reason %d", bcn_info->reason);
	beacon = (struct sAniBeaconStruct *) (bcn_info->beacon);
	vdev = wma_find_vdev_by_addr(wma, beacon->macHdr.sa, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s : failed to get vdev handle", __func__);
		status = QDF_STATUS_E_INVAL;
		goto send_rsp;
	}

	if (wmi_service_enabled(wma->wmi_handle,
				   wmi_service_beacon_offload)) {
		WMA_LOGD("Beacon Offload Enabled Sending Unified command");
		status = wma_unified_bcn_tmpl_send(wma, vdev_id, bcn_info, 4);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("%s : wmi_unified_bcn_tmpl_send Failed ",
				 __func__);
			goto send_rsp;
		}

		if (bcn_info->p2pIeOffset) {
			p2p_ie = bcn_info->beacon + bcn_info->p2pIeOffset;
			WMA_LOGD("%s: p2pIe is present - vdev_id %hu, p2p_ie = %pK, p2p ie len = %hu",
				 __func__, vdev_id, p2p_ie, p2p_ie[1]);
			if (wma_p2p_go_set_beacon_ie(wma, vdev_id,
							 p2p_ie) < 0) {
				WMA_LOGE("%s : wmi_unified_bcn_tmpl_send Failed ",
					__func__);
				status = QDF_STATUS_E_INVAL;
				goto send_rsp;
			}
		}
	}
	status = wma_store_bcn_tmpl(wma, vdev_id, bcn_info);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s : wma_store_bcn_tmpl Failed", __func__);
		goto send_rsp;
	}
	wma_ap_vdev_up(wma, vdev_id);

send_rsp:
	bcn_info->status = status;
	wma_send_msg(wma, WMA_SEND_BCN_RSP, (void *)bcn_info, 0);
}

/**
 * wma_set_keepalive_req() - send keep alive request to fw
 * @wma: wma handle
 * @keepalive: keep alive parameters
 *
 * Return: none
 */
void wma_set_keepalive_req(tp_wma_handle wma,
			   struct keep_alive_req *keepalive)
{
	WMA_LOGD("KEEPALIVE:PacketType:%d", keepalive->packetType);
	wma_set_sta_keep_alive(wma, keepalive->sessionId,
			       keepalive->packetType,
			       keepalive->timePeriod,
			       keepalive->hostIpv4Addr,
			       keepalive->destIpv4Addr,
			       keepalive->dest_macaddr.bytes);

	qdf_mem_free(keepalive);
}

/**
 * wma_beacon_miss_handler() - beacon miss event handler
 * @wma: wma handle
 * @vdev_id: vdev id
 * @riis: rssi value
 *
 * This function send beacon miss indication to upper layers.
 *
 * Return: none
 */
void wma_beacon_miss_handler(tp_wma_handle wma, uint32_t vdev_id, int32_t rssi)
{
	struct missed_beacon_ind *beacon_miss_ind;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);

	beacon_miss_ind = qdf_mem_malloc(sizeof(*beacon_miss_ind));
	if (!beacon_miss_ind)
		return;

	if (mac && mac->sme.tx_queue_cb)
		mac->sme.tx_queue_cb(mac->hdd_handle, vdev_id,
				     WLAN_STOP_ALL_NETIF_QUEUE,
				     WLAN_CONTROL_PATH);
	beacon_miss_ind->messageType = WMA_MISSED_BEACON_IND;
	beacon_miss_ind->length = sizeof(*beacon_miss_ind);
	beacon_miss_ind->bssIdx = vdev_id;

	wma_send_msg(wma, WMA_MISSED_BEACON_IND, beacon_miss_ind, 0);
	if (!wmi_service_enabled(wma->wmi_handle,
				 wmi_service_hw_db2dbm_support))
		rssi += WMA_TGT_NOISE_FLOOR_DBM;
	wma_lost_link_info_handler(wma, vdev_id, rssi);
}

/**
 * wma_get_status_str() - get string of tx status from firmware
 * @status: tx status
 *
 * Return: converted string of tx status
 */
static const char *wma_get_status_str(uint32_t status)
{
	switch (status) {
	default:
		return "unknown";
	CASE_RETURN_STRING(WMI_MGMT_TX_COMP_TYPE_COMPLETE_OK);
	CASE_RETURN_STRING(WMI_MGMT_TX_COMP_TYPE_DISCARD);
	CASE_RETURN_STRING(WMI_MGMT_TX_COMP_TYPE_INSPECT);
	CASE_RETURN_STRING(WMI_MGMT_TX_COMP_TYPE_COMPLETE_NO_ACK);
	CASE_RETURN_STRING(WMI_MGMT_TX_COMP_TYPE_MAX);
	}
}

#ifdef CONFIG_HL_SUPPORT
static inline void wma_mgmt_unmap_buf(tp_wma_handle wma_handle, qdf_nbuf_t buf)
{
}
#else
static inline void wma_mgmt_unmap_buf(tp_wma_handle wma_handle, qdf_nbuf_t buf)
{
	qdf_nbuf_unmap_single(wma_handle->qdf_dev, buf, QDF_DMA_TO_DEVICE);
}
#endif
/**
 * wma_process_mgmt_tx_completion() - process mgmt completion
 * @wma_handle: wma handle
 * @desc_id: descriptor id
 * @status: status
 *
 * Return: 0 for success or error code
 */
static int wma_process_mgmt_tx_completion(tp_wma_handle wma_handle,
					  uint32_t desc_id, uint32_t status)
{
	struct wlan_objmgr_pdev *pdev;
	qdf_nbuf_t buf = NULL;
	QDF_STATUS ret;
#if !defined(REMOVE_PKT_LOG)
	uint8_t vdev_id = 0;
	struct cdp_vdev *txrx_vdev;
	ol_txrx_pktdump_cb packetdump_cb;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
#endif

	if (!wma_handle) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: status: %s wmi_desc_id: %d", __func__,
		wma_get_status_str(status), desc_id);

	pdev = wma_handle->pdev;
	if (!pdev) {
		WMA_LOGE("%s: psoc ptr is NULL", __func__);
		return -EINVAL;
	}

	buf = mgmt_txrx_get_nbuf(pdev, desc_id);


	if (buf)
		wma_mgmt_unmap_buf(wma_handle, buf);

#if !defined(REMOVE_PKT_LOG)
	vdev_id = mgmt_txrx_get_vdev_id(pdev, desc_id);
	txrx_vdev = wma_find_vdev_by_id(wma_handle, vdev_id);
	packetdump_cb = wma_handle->wma_mgmt_tx_packetdump_cb;
	if (packetdump_cb)
		packetdump_cb(soc, txrx_vdev,
			      buf, QDF_STATUS_SUCCESS, TX_MGMT_PKT);
#endif

	ret = mgmt_txrx_tx_completion_handler(pdev, desc_id, status, NULL);

	if (ret != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to process mgmt tx completion", __func__);
		return -EINVAL;
	}

	return 0;
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

	param_buf = (WMI_MGMT_TX_COMPLETION_EVENTID_param_tlvs *)
		cmpl_event_params;
	if (!param_buf || !wma_handle) {
		WMA_LOGE("%s: Invalid mgmt Tx completion event", __func__);
		return -EINVAL;
	}
	cmpl_params = param_buf->fixed_param;

	wma_process_mgmt_tx_completion(wma_handle,
		cmpl_params->desc_id, cmpl_params->status);

	return 0;
}

/**
 * wma_mgmt_tx_bundle_completion_handler() - mgmt bundle comp handler
 * @handle: wma handle
 * @buf: buffer
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_mgmt_tx_bundle_completion_handler(void *handle, uint8_t *buf,
				   uint32_t len)
{
	tp_wma_handle wma_handle = (tp_wma_handle)handle;
	WMI_MGMT_TX_BUNDLE_COMPLETION_EVENTID_param_tlvs *param_buf;
	wmi_mgmt_tx_compl_bundle_event_fixed_param	*cmpl_params;
	uint32_t num_reports;
	uint32_t *desc_ids;
	uint32_t *status;
	uint32_t i, buf_len;
	bool excess_data = false;

	param_buf = (WMI_MGMT_TX_BUNDLE_COMPLETION_EVENTID_param_tlvs *)buf;
	if (!param_buf || !wma_handle) {
		WMA_LOGE("%s: Invalid mgmt Tx completion event", __func__);
		return -EINVAL;
	}
	cmpl_params = param_buf->fixed_param;
	num_reports = cmpl_params->num_reports;
	desc_ids = (uint32_t *)(param_buf->desc_ids);
	status = (uint32_t *)(param_buf->status);

	/* buf contains num_reports * sizeof(uint32) len of desc_ids and
	 * num_reports * sizeof(uint32) status,
	 * so (2 x (num_reports * sizeof(uint32)) should not exceed MAX
	 */
	if (cmpl_params->num_reports > (WMI_SVC_MSG_MAX_SIZE /
	    (2 * sizeof(uint32_t))))
		excess_data = true;
	else
		buf_len = cmpl_params->num_reports * (2 * sizeof(uint32_t));

	if (excess_data || (sizeof(*cmpl_params) > (WMI_SVC_MSG_MAX_SIZE -
	    buf_len))) {
		WMA_LOGE("excess wmi buffer: num_reports %d",
			  cmpl_params->num_reports);
		return -EINVAL;
	}

	if ((cmpl_params->num_reports > param_buf->num_desc_ids) ||
	    (cmpl_params->num_reports > param_buf->num_status)) {
		WMA_LOGE("Invalid num_reports %d, num_desc_ids %d, num_status %d",
			 cmpl_params->num_reports, param_buf->num_desc_ids,
			 param_buf->num_status);
		return -EINVAL;
	}

	for (i = 0; i < num_reports; i++)
		wma_process_mgmt_tx_completion(wma_handle,
			desc_ids[i], status[i]);
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
	struct wma_txrx_node *iface;
	wmi_host_channel_width ch_width;

	iface = &wma_handle->interfaces[update_vht_opmode->smesessionId];
	ch_width = wmi_get_ch_width_from_phy_mode(wma_handle->wmi_handle,
						  iface->chanmode);
	if (ch_width < update_vht_opmode->opMode) {
		WMA_LOGE("%s: Invalid peer bw update %d, self bw %d",
				__func__, update_vht_opmode->opMode,
				ch_width);
		return;
	}
	WMA_LOGD("%s: phymode = %d", __func__, iface->chanmode);
	/* Always send phymode before BW to avoid any mismatch in FW */
	wma_set_peer_param(wma_handle, update_vht_opmode->peer_mac,
			   WMI_PEER_PHYMODE, iface->chanmode,
			   update_vht_opmode->smesessionId);
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
	struct target_psoc_info *tgt_hdl;
	struct wma_txrx_node *intr =
		&wma_handle->interfaces[update_rx_nss->smesessionId];
	int rx_nss = update_rx_nss->rxNss;
	int num_rf_chains;

	tgt_hdl = wlan_psoc_get_tgt_if_handle(wma_handle->psoc);
	if (!tgt_hdl) {
		WMA_LOGE("%s: target psoc info is NULL", __func__);
		return;
	}

	num_rf_chains = target_if_get_num_rf_chains(tgt_hdl);
	if (rx_nss > num_rf_chains || rx_nss > WMA_MAX_NSS)
		rx_nss = QDF_MIN(num_rf_chains, WMA_MAX_NSS);

	intr->nss = (uint8_t)rx_nss;
	update_rx_nss->rxNss = (uint32_t)rx_nss;

	WMA_LOGD("%s: Rx Nss = %d", __func__, update_rx_nss->rxNss);

	wma_set_peer_param(wma_handle, update_rx_nss->peer_mac,
			   WMI_PEER_NSS, update_rx_nss->rxNss,
			   update_rx_nss->smesessionId);
}

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

QDF_STATUS wma_set_cts2self_for_p2p_go(void *wma_handle,
				    uint32_t cts2self_for_p2p_go)
{
	int32_t ret;
	tp_wma_handle wma = (tp_wma_handle)wma_handle;
	struct pdev_params pdevparam;

	pdevparam.param_id = WMI_PDEV_PARAM_CTS2SELF_FOR_P2P_GO_CONFIG;
	pdevparam.param_value = cts2self_for_p2p_go;

	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
			&pdevparam,
			WMA_WILDCARD_PDEV_ID);
	if (ret) {
		WMA_LOGE("Fail to Set CTS2SELF for p2p GO %d",
			cts2self_for_p2p_go);
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Successfully Set CTS2SELF for p2p GO %d",
		cts2self_for_p2p_go);

	return QDF_STATUS_SUCCESS;
}


/**
 * wma_set_htconfig() - set ht config parameters to target
 * @vdev_id: vdev id
 * @ht_capab: ht capablity
 * @value: value of ht param
 *
 * Return: QDF status
 */
QDF_STATUS wma_set_htconfig(uint8_t vdev_id, uint16_t ht_capab, int value)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	if (!wma) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		return QDF_STATUS_E_INVAL;
	}

	switch (ht_capab) {
	case WNI_CFG_HT_CAP_INFO_ADVANCE_CODING:
		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_LDPC,
						      value);
		break;
	case WNI_CFG_HT_CAP_INFO_TX_STBC:
		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_TX_STBC,
						      value);
		break;
	case WNI_CFG_HT_CAP_INFO_RX_STBC:
		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_RX_STBC,
						      value);
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ:
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		WMA_LOGE("%s: ht_capab = %d, value = %d", __func__, ht_capab,
			 value);
		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						WMI_VDEV_PARAM_SGI, value);
		if (ret == QDF_STATUS_SUCCESS)
			wma->interfaces[vdev_id].config.shortgi = value;
		break;
	default:
		WMA_LOGE("%s:INVALID HT CONFIG", __func__);
	}

	return ret;
}

/**
 * wma_hidden_ssid_vdev_restart() - vdev restart for hidden ssid
 * @wma_handle: wma handle
 * @pReq: hidden ssid vdev restart request
 *
 * Return: none
 */
void wma_hidden_ssid_vdev_restart(tp_wma_handle wma,
				  tHalHiddenSsidVdevRestart *pReq)
{
	struct wma_txrx_node *intr = wma->interfaces;
	struct wma_target_req *msg;
	struct hidden_ssid_vdev_restart_params params;
	QDF_STATUS status;
	uint8_t vdev_id;

	vdev_id = pReq->sessionId;
	if ((vdev_id != intr[vdev_id].vdev_restart_params.vdev_id)
	    || !((intr[vdev_id].type == WMI_VDEV_TYPE_AP)
		 && (intr[vdev_id].sub_type == 0))) {
		WMA_LOGE(FL("invalid vdev_id %d"), vdev_id);
		return;
	}

	intr[vdev_id].vdev_restart_params.ssidHidden = pReq->ssidHidden;
	wma_set_hidden_ssid_restart_in_progress(&intr[vdev_id], 1);
	WMA_LOGD(FL("hidden ssid set using IOCTL for vdev %d ssid_hidden %d"),
		 vdev_id, pReq->ssidHidden);

	msg = wma_fill_vdev_req(wma, vdev_id,
			WMA_HIDDEN_SSID_VDEV_RESTART,
			WMA_TARGET_REQ_TYPE_VDEV_START,
			pReq,
			WMA_VDEV_START_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGE(FL("Failed to fill vdev request, vdev_id %d"),
			 vdev_id);
		wma_set_hidden_ssid_restart_in_progress(&intr[vdev_id], 0);
		qdf_mem_free(pReq);
		return;
	}

	params.vdev_id = vdev_id;
	params.ssid_len = intr[vdev_id].vdev_restart_params.ssid.ssid_len;
	qdf_mem_copy(params.ssid,
		     intr[vdev_id].vdev_restart_params.ssid.ssid,
		     params.ssid_len);
	params.flags = intr[vdev_id].vdev_restart_params.flags;
	if (intr[vdev_id].vdev_restart_params.ssidHidden)
		params.flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;
	else
		params.flags &= (0xFFFFFFFE);
	params.requestor_id = intr[vdev_id].vdev_restart_params.requestor_id;
	params.disable_hw_ack =
		intr[vdev_id].vdev_restart_params.disable_hw_ack;

	params.mhz = intr[vdev_id].vdev_restart_params.chan.mhz;
	params.band_center_freq1 =
		intr[vdev_id].vdev_restart_params.chan.band_center_freq1;
	params.band_center_freq2 =
		intr[vdev_id].vdev_restart_params.chan.band_center_freq2;
	params.info = intr[vdev_id].vdev_restart_params.chan.info;
	params.reg_info_1 = intr[vdev_id].vdev_restart_params.chan.reg_info_1;
	params.reg_info_2 = intr[vdev_id].vdev_restart_params.chan.reg_info_2;

	wma_vdev_set_mlme_state_stop(wma, vdev_id);
	status = wmi_unified_hidden_ssid_vdev_restart_send(wma->wmi_handle,
							   &params);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE(FL("Failed to send vdev restart command"));
		wma_set_hidden_ssid_restart_in_progress(&intr[vdev_id], 0);
		wma_remove_vdev_req(wma, vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);
		qdf_mem_free(pReq);
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
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	void *peer;
	uint8_t vdev_id, peer_id;
	uint8_t *last_pn_valid = NULL;
	uint64_t *last_pn = NULL, new_pn;
	uint32_t *rmf_pn_replays = NULL;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	bool ret = false;

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
	peer = cdp_peer_get_ref_by_addr(soc, pdev, wh->i_addr2, &peer_id,
					PEER_DEBUG_ID_WMA_CCMP_REPLAY_ATTACK);

	if (!peer) {
		WMA_LOGE("%s: Failed to find peer, Not able to validate PN",
			    __func__);
		return true;
	}

	new_pn = wma_extract_ccmp_pn(ccmp_ptr);

	cdp_get_pn_info(soc, peer, &last_pn_valid, &last_pn, &rmf_pn_replays);

	if (!last_pn_valid || !last_pn || !rmf_pn_replays) {
		WMA_LOGE("%s: PN validation seems not supported", __func__);
		goto rel_peer_ref;
	}

	if (*last_pn_valid) {
		if (new_pn > *last_pn) {
			*last_pn = new_pn;
			WMA_LOGE("%s: PN validation successful", __func__);
		} else {
			WMA_LOGE("%s: PN Replay attack detected", __func__);
			/* per 11W amendment, keeping track of replay attacks */
			*rmf_pn_replays += 1;
			ret = true;
		}
	} else {
		*last_pn_valid = 1;
		*last_pn = new_pn;
	}

rel_peer_ref:
	cdp_peer_release_ref(soc, peer, PEER_DEBUG_ID_WMA_CCMP_REPLAY_ATTACK);
	return ret;
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
	qdf_nbuf_t wbuf
)
{
	uint16_t mmie_size;
	uint16_t key_id;
	uint8_t *efrm;
	uint8_t *igtk;
	uint16_t key_len;

	efrm = qdf_nbuf_data(wbuf) + qdf_nbuf_len(wbuf);

	if (iface->key.key_cipher == WMI_CIPHER_AES_CMAC) {
		mmie_size = cds_get_mmie_size();
	} else if (iface->key.key_cipher == WMI_CIPHER_AES_GMAC) {
		mmie_size = cds_get_gmac_mmie_size();
	} else {
		WMA_LOGE(FL("Invalid key cipher %d"), iface->key.key_cipher);
		return -EINVAL;
	}

	/* Check if frame is invalid length */
	if (efrm - (uint8_t *)wh < sizeof(*wh) + mmie_size) {
		WMA_LOGE(FL("Invalid frame length"));
		return -EINVAL;
	}

	key_id = (uint16_t)*(efrm - mmie_size + 2);
	if (!((key_id == WMA_IGTK_KEY_INDEX_4)
	     || (key_id == WMA_IGTK_KEY_INDEX_5))) {
		WMA_LOGE(FL("Invalid KeyID(%d) dropping the frame"), key_id);
		return -EINVAL;
	}

	WMA_LOGD(FL("key_cipher %d key_id %d"), iface->key.key_cipher, key_id);

	igtk = wma_get_igtk(iface, &key_len);
	switch (iface->key.key_cipher) {
	case WMI_CIPHER_AES_CMAC:
		if (wmi_service_enabled(wma_handle->wmi_handle,
				wmi_service_sta_pmf_offload)) {
			/*
			 * if 11w offload is enabled then mmie validation is
			 * performed in firmware, host just need to trim the
			 * mmie.
			 */
			qdf_nbuf_trim_tail(wbuf, cds_get_mmie_size());
		} else {
			if (cds_is_mmie_valid(igtk, iface->key.key_id[
					      key_id -
					      WMA_IGTK_KEY_INDEX_4].ipn,
					      (uint8_t *)wh, efrm)) {
				WMA_LOGD(FL("Protected BC/MC frame MMIE validation successful"));
				/* Remove MMIE */
				qdf_nbuf_trim_tail(wbuf, cds_get_mmie_size());
			} else {
				WMA_LOGD(FL("BC/MC MIC error or MMIE not present, dropping the frame"));
				return -EINVAL;
			}
		}
		break;

	case WMI_CIPHER_AES_GMAC:
		if (wmi_service_enabled(wma_handle->wmi_handle,
				wmi_service_gmac_offload_support)) {
			/*
			 * if gmac offload is enabled then mmie validation is
			 * performed in firmware, host just need to trim the
			 * mmie.
			 */
			WMA_LOGD(FL("Trim GMAC MMIE"));
			qdf_nbuf_trim_tail(wbuf, cds_get_gmac_mmie_size());
		} else {
			if (cds_is_gmac_mmie_valid(igtk,
			   iface->key.key_id[key_id - WMA_IGTK_KEY_INDEX_4].ipn,
			   (uint8_t *) wh, efrm, key_len)) {
				WMA_LOGD(FL("Protected BC/MC frame GMAC MMIE validation successful"));
				/* Remove MMIE */
				qdf_nbuf_trim_tail(wbuf,
						   cds_get_gmac_mmie_size());
			} else {
				WMA_LOGD(FL("BC/MC GMAC MIC error or MMIE not present, dropping the frame"));
				return -EINVAL;
			}
		}
		break;

	default:
		WMA_LOGE(FL("Unsupported key cipher %d"),
			iface->key.key_cipher);
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
	qdf_nbuf_t wbuf)
{
	uint8_t *orig_hdr;
	uint8_t *ccmp;
	uint8_t mic_len, hdr_len, pdev_id;
	QDF_STATUS status;

	if ((wh)->i_fc[1] & IEEE80211_FC1_WEP) {
		if (QDF_IS_ADDR_BROADCAST(wh->i_addr1) ||
		    IEEE80211_IS_MULTICAST(wh->i_addr1)) {
			WMA_LOGE("Encrypted BC/MC frame dropping the frame");
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}

		pdev_id = wlan_objmgr_pdev_get_pdev_id(wma_handle->pdev);
		status = mlme_get_peer_mic_len(wma_handle->psoc, pdev_id,
					       wh->i_addr2, &mic_len,
					       &hdr_len);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("Failed to get mic hdr and length");
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}

		if (qdf_nbuf_len(wbuf) < (sizeof(*wh) + hdr_len + mic_len)) {
			WMA_LOGE("Buffer length less than expected %d",
				 (int)qdf_nbuf_len(wbuf));
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}

		orig_hdr = (uint8_t *) qdf_nbuf_data(wbuf);
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
		qdf_mem_move(orig_hdr +
			hdr_len, wh,
			sizeof(*wh));
		qdf_nbuf_pull_head(wbuf,
			hdr_len);
		qdf_nbuf_trim_tail(wbuf, mic_len);
		/*
		 * CCMP header has been pulled off
		 * reinitialize the start pointer of mac header
		 * to avoid accessing incorrect address
		 */
		wh = (struct ieee80211_frame *) qdf_nbuf_data(wbuf);
		rx_pkt->pkt_meta.mpdu_hdr_ptr =
				qdf_nbuf_data(wbuf);
		rx_pkt->pkt_meta.mpdu_len = qdf_nbuf_len(wbuf);
		rx_pkt->pkt_buf = wbuf;
		if (rx_pkt->pkt_meta.mpdu_len >=
			rx_pkt->pkt_meta.mpdu_hdr_len) {
			rx_pkt->pkt_meta.mpdu_data_len =
				rx_pkt->pkt_meta.mpdu_len -
				rx_pkt->pkt_meta.mpdu_hdr_len;
		} else {
			WMA_LOGE("mpdu len %d less than hdr %d, dropping frame",
				rx_pkt->pkt_meta.mpdu_len,
				rx_pkt->pkt_meta.mpdu_hdr_len);
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}

		if (rx_pkt->pkt_meta.mpdu_data_len > WMA_MAX_MGMT_MPDU_LEN) {
			WMA_LOGE("Data Len %d greater than max, dropping frame",
				rx_pkt->pkt_meta.mpdu_data_len);
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}
		rx_pkt->pkt_meta.mpdu_data_ptr =
		rx_pkt->pkt_meta.mpdu_hdr_ptr +
		rx_pkt->pkt_meta.mpdu_hdr_len;
		WMA_LOGD(FL("BSSID: "MAC_ADDRESS_STR" tsf_delta: %u"),
		    MAC_ADDR_ARRAY(wh->i_addr3), rx_pkt->pkt_meta.tsf_delta);
	} else {
		if (QDF_IS_ADDR_BROADCAST(wh->i_addr1) ||
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
#else
static inline int wma_process_rmf_frame(tp_wma_handle wma_handle,
	struct wma_txrx_node *iface,
	struct ieee80211_frame *wh,
	cds_pkt_t *rx_pkt,
	qdf_nbuf_t wbuf)
{
	return 0;
}

#endif

/**
 * wma_is_pkt_drop_candidate() - check if the mgmt frame should be droppped
 * @wma_handle: wma handle
 * @peer_addr: peer MAC address
 * @bssid: BSSID Address
 * @subtype: Management frame subtype
 *
 * This function is used to decide if a particular management frame should be
 * dropped to prevent DOS attack. Timestamp is used to decide the DOS attack.
 *
 * Return: true if the packet should be dropped and false oterwise
 */
static bool wma_is_pkt_drop_candidate(tp_wma_handle wma_handle,
				      uint8_t *peer_addr, uint8_t *bssid,
				      uint8_t subtype)
{
	bool should_drop = false;
	uint8_t nan_addr[] = {0x50, 0x6F, 0x9A, 0x01, 0x00, 0x00};

	/* Drop the beacons from NAN device */
	if ((subtype == MGMT_SUBTYPE_BEACON) &&
		(!qdf_mem_cmp(nan_addr, bssid, NAN_CLUSTER_ID_BYTES))) {
			should_drop = true;
			goto end;
	}
end:
	return should_drop;
}

#define RATE_LIMIT 16

int wma_form_rx_packet(qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params,
			cds_pkt_t *rx_pkt)
{
	struct wma_txrx_node *iface = NULL;
	uint8_t vdev_id = WMA_INVALID_VDEV_ID;
	struct ieee80211_frame *wh;
	uint8_t mgt_type, mgt_subtype;
	int status;
	tp_wma_handle wma_handle = (tp_wma_handle)
				cds_get_context(QDF_MODULE_ID_WMA);
#if !defined(REMOVE_PKT_LOG)
	ol_txrx_pktdump_cb packetdump_cb;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct cdp_vdev *txrx_vdev;
#endif
	static uint8_t limit_prints_invalid_len = RATE_LIMIT - 1;
	static uint8_t limit_prints_load_unload = RATE_LIMIT - 1;
	static uint8_t limit_prints_recovery = RATE_LIMIT - 1;

	if (!wma_handle) {
		WMA_LOGE(FL("wma handle is NULL"));
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	if (!mgmt_rx_params) {
		limit_prints_invalid_len++;
		if (limit_prints_invalid_len == RATE_LIMIT) {
			WMA_LOGD(FL("mgmt rx params is NULL"));
			limit_prints_invalid_len = 0;
		}
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	if (cds_is_load_or_unload_in_progress()) {
		limit_prints_load_unload++;
		if (limit_prints_load_unload == RATE_LIMIT) {
			WMA_LOGD(FL("Load/Unload in progress"));
			limit_prints_load_unload = 0;
		}
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	if (cds_is_driver_recovering()) {
		limit_prints_recovery++;
		if (limit_prints_recovery == RATE_LIMIT) {
			WMA_LOGD(FL("Recovery in progress"));
			limit_prints_recovery = 0;
		}
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	if (cds_is_driver_in_bad_state()) {
		limit_prints_recovery++;
		if (limit_prints_recovery == RATE_LIMIT) {
			WMA_LOGD(FL("Driver in bad state"));
			limit_prints_recovery = 0;
		}
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	/*
	 * Fill in meta information needed by pe/lim
	 * TODO: Try to maintain rx metainfo as part of skb->data.
	 */
	rx_pkt->pkt_meta.channel = mgmt_rx_params->channel;
	rx_pkt->pkt_meta.scan_src = mgmt_rx_params->flags;

	/*
	 * Get the rssi value from the current snr value
	 * using standard noise floor of -96.
	 */
	rx_pkt->pkt_meta.rssi = mgmt_rx_params->snr +
				WMA_NOISE_FLOOR_DBM_DEFAULT;
	rx_pkt->pkt_meta.snr = mgmt_rx_params->snr;

	/* If absolute rssi is available from firmware, use it */
	if (mgmt_rx_params->rssi != 0)
		rx_pkt->pkt_meta.rssi_raw = mgmt_rx_params->rssi;
	else
		rx_pkt->pkt_meta.rssi_raw = rx_pkt->pkt_meta.rssi;


	/*
	 * FIXME: Assigning the local timestamp as hw timestamp is not
	 * available. Need to see if pe/lim really uses this data.
	 */
	rx_pkt->pkt_meta.timestamp = (uint32_t) jiffies;
	rx_pkt->pkt_meta.mpdu_hdr_len = sizeof(struct ieee80211_frame);
	rx_pkt->pkt_meta.mpdu_len = mgmt_rx_params->buf_len;

	/*
	 * The buf_len should be at least 802.11 header len
	 */
	if (mgmt_rx_params->buf_len < rx_pkt->pkt_meta.mpdu_hdr_len) {
		WMA_LOGE("MPDU Len %d lesser than header len %d",
			 mgmt_rx_params->buf_len,
			 rx_pkt->pkt_meta.mpdu_hdr_len);
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	rx_pkt->pkt_meta.mpdu_data_len = mgmt_rx_params->buf_len -
					 rx_pkt->pkt_meta.mpdu_hdr_len;

	rx_pkt->pkt_meta.roamCandidateInd = 0;

	wh = (struct ieee80211_frame *)qdf_nbuf_data(buf);

	/*
	 * If the mpdu_data_len is greater than Max (2k), drop the frame
	 */
	if (rx_pkt->pkt_meta.mpdu_data_len > WMA_MAX_MGMT_MPDU_LEN) {
		WMA_LOGE("Data Len %d greater than max, dropping frame from "MAC_ADDRESS_STR,
			 rx_pkt->pkt_meta.mpdu_data_len,
			 MAC_ADDR_ARRAY(wh->i_addr3));
		qdf_nbuf_free(buf);
		qdf_mem_free(rx_pkt);
		return -EINVAL;
	}

	rx_pkt->pkt_meta.mpdu_hdr_ptr = qdf_nbuf_data(buf);
	rx_pkt->pkt_meta.mpdu_data_ptr = rx_pkt->pkt_meta.mpdu_hdr_ptr +
					 rx_pkt->pkt_meta.mpdu_hdr_len;
	rx_pkt->pkt_meta.tsf_delta = mgmt_rx_params->tsf_delta;
	rx_pkt->pkt_buf = buf;

	/* If it is a beacon/probe response, save it for future use */
	mgt_type = (wh)->i_fc[0] & IEEE80211_FC0_TYPE_MASK;
	mgt_subtype = (wh)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK;

	if (mgt_type == IEEE80211_FC0_TYPE_MGT &&
	    (mgt_subtype == MGMT_SUBTYPE_DISASSOC ||
	     mgt_subtype == MGMT_SUBTYPE_DEAUTH ||
	     mgt_subtype == MGMT_SUBTYPE_ACTION)) {
		if (wma_find_vdev_by_bssid(
			wma_handle, wh->i_addr3, &vdev_id)) {
			iface = &(wma_handle->interfaces[vdev_id]);
			if (iface->rmfEnabled) {
				status = wma_process_rmf_frame(wma_handle,
					iface, wh, rx_pkt, buf);
				if (status != 0)
					return status;
				/*
				 * CCMP header might have been pulled off
				 * reinitialize the start pointer of mac header
				 */
				wh = (struct ieee80211_frame *)
						qdf_nbuf_data(buf);
			}
		}
	}

	rx_pkt->pkt_meta.session_id =
		(vdev_id == WMA_INVALID_VDEV_ID ? 0 : vdev_id);

	if (mgt_type == IEEE80211_FC0_TYPE_MGT &&
	    (mgt_subtype == MGMT_SUBTYPE_BEACON ||
	     mgt_subtype == MGMT_SUBTYPE_PROBE_RESP)) {
		if (mgmt_rx_params->buf_len <=
			(sizeof(struct ieee80211_frame) +
			offsetof(struct wlan_bcn_frame, ie))) {
			WMA_LOGD("Dropping frame from "MAC_ADDRESS_STR,
				 MAC_ADDR_ARRAY(wh->i_addr3));
			cds_pkt_return_packet(rx_pkt);
			return -EINVAL;
		}
	}

	if (wma_is_pkt_drop_candidate(wma_handle, wh->i_addr2, wh->i_addr3,
					mgt_subtype)) {
		cds_pkt_return_packet(rx_pkt);
		return -EINVAL;
	}

#if !defined(REMOVE_PKT_LOG)
	packetdump_cb = wma_handle->wma_mgmt_rx_packetdump_cb;
	txrx_vdev = wma_find_vdev_by_id(wma_handle,
					rx_pkt->pkt_meta.session_id);
	if ((mgt_type == IEEE80211_FC0_TYPE_MGT &&
		mgt_subtype != MGMT_SUBTYPE_BEACON) &&
		packetdump_cb)
		packetdump_cb(soc, txrx_vdev, rx_pkt->pkt_buf,
			      QDF_STATUS_SUCCESS, RX_MGMT_PKT);
#endif
	return 0;
}

/**
 * wma_mem_endianness_based_copy() - does memory copy from src to dst
 * @dst: destination address
 * @src: source address
 * @size: size to be copied
 *
 * This function copies the memory of size passed from source
 * address to destination address.
 *
 * Return: Nothing
 */
#ifdef BIG_ENDIAN_HOST
static void wma_mem_endianness_based_copy(
			uint8_t *dst, uint8_t *src, uint32_t size)
{
	/*
	 * For big endian host, copy engine byte_swap is enabled
	 * But the rx mgmt frame buffer content is in network byte order
	 * Need to byte swap the mgmt frame buffer content - so when
	 * copy engine does byte_swap - host gets buffer content in the
	 * correct byte order.
	 */

	uint32_t i;
	uint32_t *destp, *srcp;

	destp = (uint32_t *) dst;
	srcp = (uint32_t *) src;
	for (i = 0; i < (roundup(size, sizeof(uint32_t)) / 4); i++) {
		*destp = cpu_to_le32(*srcp);
		destp++;
		srcp++;
	}
}
#else
static void wma_mem_endianness_based_copy(
			uint8_t *dst, uint8_t *src, uint32_t size)
{
	qdf_mem_copy(dst, src, size);
}
#endif

#define RESERVE_BYTES                   100
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
	struct mgmt_rx_event_params *mgmt_rx_params;
	struct wlan_objmgr_psoc *psoc;
	uint8_t *bufp;
	qdf_nbuf_t wbuf;
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE("%s: Failed to get WMA  context", __func__);
		return -EINVAL;
	}

	mgmt_rx_params = qdf_mem_malloc(sizeof(*mgmt_rx_params));
	if (!mgmt_rx_params) {
		WMA_LOGE("%s: memory allocation failed", __func__);
		return -ENOMEM;
	}

	if (wmi_extract_mgmt_rx_params(wma_handle->wmi_handle,
			data, mgmt_rx_params, &bufp) != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Extraction of mgmt rx params failed", __func__);
		qdf_mem_free(mgmt_rx_params);
		return -EINVAL;
	}

	if (mgmt_rx_params->buf_len > data_len) {
		WMA_LOGE("%s: Invalid rx mgmt packet, data_len %u, mgmt_rx_params->buf_len %u",
			__func__, data_len, mgmt_rx_params->buf_len);
		qdf_mem_free(mgmt_rx_params);
		return -EINVAL;
	}

	mgmt_rx_params->pdev_id = 0;
	mgmt_rx_params->rx_params = NULL;

	/*
	 * Allocate the memory for this rx packet, add extra 100 bytes for:-
	 *
	 * 1.  Filling the missing RSN capabilites by some APs, which fill the
	 *     RSN IE length as extra 2 bytes but dont fill the IE data with
	 *     capabilities, resulting in failure in unpack core due to length
	 *     mismatch. Check sir_validate_and_rectify_ies for more info.
	 *
	 * 2.  In the API wma_process_rmf_frame(), the driver trims the CCMP
	 *     header by overwriting the IEEE header to memory occupied by CCMP
	 *     header, but an overflow is possible if the memory allocated to
	 *     frame is less than the sizeof(struct ieee80211_frame) +CCMP
	 *     HEADER len, so allocating 100 bytes would solve this issue too.
	 *
	 * 3.  CCMP header is pointing to orig_hdr +
	 *     sizeof(struct ieee80211_frame) which could also result in OOB
	 *     access, if the data len is less than
	 *     sizeof(struct ieee80211_frame), allocating extra bytes would
	 *     result in solving this issue too.
	 */
	wbuf = qdf_nbuf_alloc(NULL, roundup(mgmt_rx_params->buf_len +
							RESERVE_BYTES,
							4), 0, 4, false);
	if (!wbuf) {
		qdf_mem_free(mgmt_rx_params);
		return -ENOMEM;
	}

	qdf_nbuf_put_tail(wbuf, mgmt_rx_params->buf_len);
	qdf_nbuf_set_protocol(wbuf, ETH_P_CONTROL);

	qdf_mem_zero(((uint8_t *)qdf_nbuf_data(wbuf) + mgmt_rx_params->buf_len),
		     (roundup(mgmt_rx_params->buf_len + RESERVE_BYTES, 4) -
		     mgmt_rx_params->buf_len));

	wma_mem_endianness_based_copy(qdf_nbuf_data(wbuf),
			bufp, mgmt_rx_params->buf_len);

	psoc = (struct wlan_objmgr_psoc *)
				wma_handle->psoc;
	if (!psoc) {
		WMA_LOGE("%s: psoc ctx is NULL", __func__);
		qdf_nbuf_free(wbuf);
		qdf_mem_free(mgmt_rx_params);
		return -EINVAL;
	}

	status = mgmt_txrx_rx_handler(psoc, wbuf, mgmt_rx_params);
	if (status != QDF_STATUS_SUCCESS) {
		wma_err_rl("Failed to process mgmt rx frame");
		qdf_mem_free(mgmt_rx_params);
		return -EINVAL;
	}

	qdf_mem_free(mgmt_rx_params);
	return 0;
}

/**
 * wma_de_register_mgmt_frm_client() - deregister management frame
 *
 * This function deregisters the event handler registered for
 * WMI_MGMT_RX_EVENTID.
 *
 * Return: QDF status
 */
QDF_STATUS wma_de_register_mgmt_frm_client(void)
{
	tp_wma_handle wma_handle = (tp_wma_handle)
				cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGE("%s: Failed to get WMA context", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

#ifdef QCA_WIFI_FTM
	if (cds_get_conparam() == QDF_GLOBAL_FTM_MODE)
		return QDF_STATUS_SUCCESS;
#endif

	if (wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
						 wmi_mgmt_rx_event_id) != 0) {
		WMA_LOGE("Failed to Unregister rx mgmt handler with wmi");
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wma_register_roaming_callbacks() - Register roaming callbacks
 * @csr_roam_synch_cb: CSR roam synch callback routine pointer
 * @pe_roam_synch_cb: PE roam synch callback routine pointer
 *
 * Register the SME and PE callback routines with WMA for
 * handling roaming
 *
 * Return: Success or Failure Status
 */
QDF_STATUS wma_register_roaming_callbacks(
	QDF_STATUS (*csr_roam_synch_cb)(struct mac_context *mac,
		struct roam_offload_synch_ind *roam_synch_data,
		struct bss_description *bss_desc_ptr,
		enum sir_roam_op_code reason),
	QDF_STATUS (*pe_roam_synch_cb)(struct mac_context *mac,
		struct roam_offload_synch_ind *roam_synch_data,
		struct bss_description *bss_desc_ptr,
		enum sir_roam_op_code reason))
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
#endif

/**
 * wma_register_mgmt_frm_client() - register management frame callback
 *
 * This function registers event handler for WMI_MGMT_RX_EVENTID.
 *
 * Return: QDF status
 */
QDF_STATUS wma_register_mgmt_frm_client(void)
{
	tp_wma_handle wma_handle = (tp_wma_handle)
				cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGE("%s: Failed to get WMA context", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (wmi_unified_register_event_handler(wma_handle->wmi_handle,
					       wmi_mgmt_rx_event_id,
					       wma_mgmt_rx_process,
					       WMA_RX_WORK_CTX) != 0) {
		WMA_LOGE("Failed to register rx mgmt handler with wmi");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_register_packetdump_callback() - stores tx and rx mgmt packet dump
 *   callback handler
 * @tx_cb: tx mgmt packetdump cb
 * @rx_cb: rx mgmt packetdump cb
 *
 * This function is used to store tx and rx mgmt. packet dump callback
 *
 * Return: None
 *
 */
void wma_register_packetdump_callback(
	ol_txrx_pktdump_cb tx_cb,
	ol_txrx_pktdump_cb rx_cb)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGE("wma handle is NULL");
		return;
	}

	wma_handle->wma_mgmt_tx_packetdump_cb = tx_cb;
	wma_handle->wma_mgmt_rx_packetdump_cb = rx_cb;
}

/**
 * wma_deregister_packetdump_callback() - removes tx and rx mgmt packet dump
 *   callback handler
 *
 * This function is used to remove tx and rx mgmt. packet dump callback
 *
 * Return: None
 *
 */
void wma_deregister_packetdump_callback(void)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGE("wma handle is NULL");
		return;
	}

	wma_handle->wma_mgmt_tx_packetdump_cb = NULL;
	wma_handle->wma_mgmt_rx_packetdump_cb = NULL;
}

QDF_STATUS wma_mgmt_unified_cmd_send(struct wlan_objmgr_vdev *vdev,
				qdf_nbuf_t buf, uint32_t desc_id,
				void *mgmt_tx_params)
{
	tp_wma_handle wma_handle;
	int ret;
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct wmi_mgmt_params *mgmt_params =
			(struct wmi_mgmt_params *)mgmt_tx_params;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct cdp_vdev *txrx_vdev;

	if (!mgmt_params) {
		WMA_LOGE("%s: mgmt_params ptr passed is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}
	mgmt_params->desc_id = desc_id;

	if (!vdev) {
		WMA_LOGE("%s: vdev ptr passed is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		WMA_LOGE("%s: wma handle is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	txrx_vdev = wma_handle->interfaces[mgmt_params->vdev_id].handle;

	if (wmi_service_enabled(wma_handle->wmi_handle,
				   wmi_service_mgmt_tx_wmi)) {
		status = wmi_mgmt_unified_cmd_send(wma_handle->wmi_handle,
						   mgmt_params);
	} else if (txrx_vdev) {
		QDF_NBUF_CB_MGMT_TXRX_DESC_ID(buf)
						= mgmt_params->desc_id;

		ret = cdp_mgmt_send_ext(soc, txrx_vdev, buf,
					mgmt_params->tx_type,
					mgmt_params->use_6mbps,
					mgmt_params->chanfreq);
		status = qdf_status_from_os_return(ret);
	}

	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: mgmt tx failed", __func__);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

#ifndef CONFIG_HL_SUPPORT
void wma_mgmt_nbuf_unmap_cb(struct wlan_objmgr_pdev *pdev,
			    qdf_nbuf_t buf)
{
	struct wlan_objmgr_psoc *psoc;
	qdf_device_t dev;

	if (!buf)
		return;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		WMA_LOGE("%s: Psoc handle NULL", __func__);
		return;
	}

	dev = wlan_psoc_get_qdf_dev(psoc);
	if (wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_SOC_CEXT_WMI_MGMT_REF))
		qdf_nbuf_unmap_single(dev, buf, QDF_DMA_TO_DEVICE);
}
#endif
