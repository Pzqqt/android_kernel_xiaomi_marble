/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 * \file lim_send_management_frames.c
 *
 * \brief Code for preparing and sending 802.11 Management frames
 *
 *
 */

#include "sir_api.h"
#include "ani_global.h"
#include "sir_mac_prot_def.h"
#include "cfg_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_security_utils.h"
#include "lim_prop_exts_utils.h"
#include "dot11f.h"
#include "lim_sta_hash_api.h"
#include "sch_api.h"
#include "lim_send_messages.h"
#include "lim_assoc_utils.h"
#include "lim_ft.h"
#ifdef WLAN_FEATURE_11W
#include "wni_cfg.h"
#endif

#include "lim_ft_defs.h"
#include "lim_session.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "cds_utils.h"
#include "sme_trace.h"
#include "rrm_api.h"

#include "wma_types.h"

/**
 *
 * \brief This function is called to add the sequence number to the
 * management frames
 *
 * \param  pMac Pointer to Global MAC structure
 *
 * \param  pMacHdr Pointer to MAC management header
 *
 * The pMacHdr argument points to the MAC management header. The
 * sequence number stored in the pMac structure will be incremented
 * and updated to the MAC management header. The start sequence
 * number is WLAN_HOST_SEQ_NUM_MIN and the end value is
 * WLAN_HOST_SEQ_NUM_MAX. After reaching the MAX value, the sequence
 * number will roll over.
 *
 */
void lim_add_mgmt_seq_num(tpAniSirGlobal pMac, tpSirMacMgmtHdr pMacHdr)
{
	if (pMac->mgmtSeqNum >= WLAN_HOST_SEQ_NUM_MAX) {
		pMac->mgmtSeqNum = WLAN_HOST_SEQ_NUM_MIN - 1;
	}

	pMac->mgmtSeqNum++;

	pMacHdr->seqControl.seqNumLo = (pMac->mgmtSeqNum & LOW_SEQ_NUM_MASK);
	pMacHdr->seqControl.seqNumHi =
		((pMac->mgmtSeqNum & HIGH_SEQ_NUM_MASK) >> HIGH_SEQ_NUM_OFFSET);
}

/**
 *
 * \brief This function is called before sending a p2p action frame
 * inorder to add sequence numbers to action packets
 *
 * \param  pMac Pointer to Global MAC structure
 *
 * \param pBD Pointer to the frame buffer that needs to be populate
 *
 * The pMacHdr argument points to the MAC management header. The
 * sequence number stored in the pMac structure will be incremented
 * and updated to the MAC management header. The start sequence
 * number is WLAN_HOST_SEQ_NUM_MIN and the end value is
 * WLAN_HOST_SEQ_NUM_MAX. After reaching the MAX value, the sequence
 * number will roll over.
 *
 */
void lim_populate_p2p_mac_header(tpAniSirGlobal pMac, uint8_t *pBD)
{
	tpSirMacMgmtHdr pMacHdr;

	/* / Prepare MAC management header */
	pMacHdr = (tpSirMacMgmtHdr) (pBD);

	/* Prepare sequence number */
	lim_add_mgmt_seq_num(pMac, pMacHdr);
	lim_log(pMac, LOG1, "seqNumLo=%d, seqNumHi=%d, mgmtSeqNum=%d",
		pMacHdr->seqControl.seqNumLo,
		pMacHdr->seqControl.seqNumHi, pMac->mgmtSeqNum);
}

/**
 * lim_populate_mac_header() - Fill in 802.11 header of frame
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @buf: Pointer to the frame buffer that needs to be populate
 * @type: 802.11 Type of the frame
 * @sub_type: 802.11 Subtype of the frame
 * @peer_addr: dst address
 * @self_mac_addr: local mac address
 *
 * This function is called by various LIM modules to prepare the
 * 802.11 frame MAC header
 *
 * The buf argument points to the beginning of the frame buffer to
 * which - a) The 802.11 MAC header is set b) Following this MAC header
 * will be the MGMT frame payload The payload itself is populated by the
 * caller API
 *
 * Return: None
 */

void lim_populate_mac_header(tpAniSirGlobal mac_ctx, uint8_t *buf,
		uint8_t type, uint8_t sub_type, tSirMacAddr peer_addr,
		tSirMacAddr self_mac_addr)
{
	tpSirMacMgmtHdr mac_hdr;

	/* Prepare MAC management header */
	mac_hdr = (tpSirMacMgmtHdr) (buf);

	/* Prepare FC */
	mac_hdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
	mac_hdr->fc.type = type;
	mac_hdr->fc.subType = sub_type;

	/* Prepare Address 1 */
	qdf_mem_copy((uint8_t *) mac_hdr->da,
		     (uint8_t *) peer_addr, sizeof(tSirMacAddr));

	/* Prepare Address 2 */
	sir_copy_mac_addr(mac_hdr->sa, self_mac_addr);

	/* Prepare Address 3 */
	qdf_mem_copy((uint8_t *) mac_hdr->bssId,
		     (uint8_t *) peer_addr, sizeof(tSirMacAddr));

	/* Prepare sequence number */
	lim_add_mgmt_seq_num(mac_ctx, mac_hdr);
	lim_log(mac_ctx, LOG1, "seqNumLo=%d, seqNumHi=%d, mgmtSeqNum=%d",
		mac_hdr->seqControl.seqNumLo,
		mac_hdr->seqControl.seqNumHi, mac_ctx->mgmtSeqNum);
}

/**
 * lim_send_probe_req_mgmt_frame() - send probe request management frame
 * @mac_ctx: Pointer to Global MAC structure
 * @ssid: SSID to be sent in Probe Request frame
 * @bssid: BSSID to be sent in Probe Request frame
 * @channel: Channel # on which the Probe Request is going out
 * @self_macaddr: self MAC address
 * @dot11mode: self dotllmode
 * @additional_ielen: if non-zero, include additional_ie in the Probe Request
 *                   frame
 * @additional_ie: if additional_ielen is non zero, include this field in the
 *                Probe Request frame
 *
 * This function is called by various LIM modules to send Probe Request frame
 * during active scan/learn phase.
 * Probe request is sent out in the following scenarios:
 * --heartbeat failure:  session needed
 * --join req:           session needed
 * --foreground scan:    no session
 * --background scan:    no session
 * --sch_beacon_processing:  to get EDCA parameters:  session needed
 *
 * Return: tSirRetStatus (eSIR_SUCCESS on success and error codes otherwise)
 */
tSirRetStatus
lim_send_probe_req_mgmt_frame(tpAniSirGlobal mac_ctx,
			      tSirMacSSid *ssid,
			      tSirMacAddr bssid,
			      uint8_t channel,
			      tSirMacAddr self_macaddr,
			      uint32_t dot11mode,
			      uint32_t additional_ielen, uint8_t *additional_ie)
{
	tDot11fProbeRequest pr;
	uint32_t status, bytes, payload;
	uint8_t *frame;
	void *packet;
	QDF_STATUS qdf_status;
	tpPESession pesession;
	uint8_t sessionid;
	uint8_t *p2pie = NULL;
	uint8_t txflag = 0;
	uint8_t sme_sessionid = 0;
	bool is_vht_enabled = false;
	uint8_t txPower;
	uint16_t addn_ielen = additional_ielen;
	bool extracted_ext_cap_flag = false;
	tDot11fIEExtCap extracted_ext_cap;
	tSirRetStatus sir_status;

	/* The probe req should not send 11ac capabilieties if band is 2.4GHz,
	 * unless enableVhtFor24GHz is enabled in INI. So if enableVhtFor24GHz
	 * is false and dot11mode is 11ac set it to 11n.
	 */
	if (channel <= SIR_11B_CHANNEL_END &&
	    (false == mac_ctx->roam.configParam.enableVhtFor24GHz) &&
	    (WNI_CFG_DOT11_MODE_11AC == dot11mode ||
	     WNI_CFG_DOT11_MODE_11AC_ONLY == dot11mode))
		dot11mode = WNI_CFG_DOT11_MODE_11N;
	/*
	 * session context may or may not be present, when probe request needs
	 * to be sent out. Following cases exist:
	 * --heartbeat failure:  session needed
	 * --join req:           session needed
	 * --foreground scan:    no session
	 * --background scan:    no session
	 * --sch_beacon_processing:  to get EDCA parameters:  session needed
	 * If session context does not exist, some IEs will be populated from
	 * CFGs, e.g. Supported and Extended rate set IEs
	 */
	pesession = pe_find_session_by_bssid(mac_ctx, bssid, &sessionid);

	if (pesession != NULL)
		sme_sessionid = pesession->smeSessionId;

	/* The scheme here is to fill out a 'tDot11fProbeRequest' structure */
	/* and then hand it off to 'dot11f_pack_probe_request' (for */
	/* serialization).  We start by zero-initializing the structure: */
	qdf_mem_set((uint8_t *) &pr, sizeof(pr), 0);

	/* & delegating to assorted helpers: */
	populate_dot11f_ssid(mac_ctx, ssid, &pr.SSID);

	if (addn_ielen && additional_ie)
		p2pie = limGetP2pIEPtr(mac_ctx, additional_ie, addn_ielen);

	/*
	 * Don't include 11b rate if it is a P2P serach or probe request is
	 * sent by P2P Client
	 */
	if ((WNI_CFG_DOT11_MODE_11B != dot11mode) && (p2pie != NULL) &&
	    (((mac_ctx->lim.gpLimMlmScanReq != NULL) &&
	      mac_ctx->lim.gpLimMlmScanReq->p2pSearch) ||
	     ((pesession != NULL) &&
	      (QDF_P2P_CLIENT_MODE == pesession->pePersona))
	    )
	   ) {
		/*
		 * In the below API pass channel number > 14, do that it fills
		 * only 11a rates in supported rates
		 */
		populate_dot11f_supp_rates(mac_ctx, 15, &pr.SuppRates,
					   pesession);
	} else {
		populate_dot11f_supp_rates(mac_ctx, channel,
					   &pr.SuppRates, pesession);

		if (WNI_CFG_DOT11_MODE_11B != dot11mode) {
			populate_dot11f_ext_supp_rates1(mac_ctx, channel,
							&pr.ExtSuppRates);
		}
	}

	/*
	 * Table 7-14 in IEEE Std. 802.11k-2008 says
	 * DS params "can" be present in RRM is disabled and "is" present if
	 * RRM is enabled. It should be ok even if we add it into probe req when
	 * RRM is not enabled.
	 */
	populate_dot11f_ds_params(mac_ctx, &pr.DSParams, channel);
	/* Call RRM module to get the tx power for management used. */
	txPower = (uint8_t) rrm_get_mgmt_tx_power(mac_ctx, pesession);
	populate_dot11f_wfatpc(mac_ctx, &pr.WFATPC, txPower, 0);


	if (pesession != NULL) {
		pesession->htCapability = IS_DOT11_MODE_HT(dot11mode);
		/* Include HT Capability IE */
		if (pesession->htCapability)
			populate_dot11f_ht_caps(mac_ctx, pesession, &pr.HTCaps);
	} else {                /* pesession == NULL */
		if (IS_DOT11_MODE_HT(dot11mode))
			populate_dot11f_ht_caps(mac_ctx, NULL, &pr.HTCaps);
	}

	/*
	 * Set channelbonding information as "disabled" when tunned to a
	 * 2.4 GHz channel
	 */
	if (channel <= SIR_11B_CHANNEL_END) {
		if (mac_ctx->roam.configParam.channelBondingMode24GHz
		    == PHY_SINGLE_CHANNEL_CENTERED) {
			pr.HTCaps.supportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_20MHZ;
			pr.HTCaps.shortGI40MHz = 0;
		} else {
			pr.HTCaps.supportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_40MHZ;
		}
	}
	if (pesession != NULL) {
		pesession->vhtCapability = IS_DOT11_MODE_VHT(dot11mode);
		/* Include VHT Capability IE */
		if (pesession->vhtCapability) {
			populate_dot11f_vht_caps(mac_ctx, pesession,
						 &pr.VHTCaps);
			is_vht_enabled = true;
		}
	} else {
		if (IS_DOT11_MODE_VHT(dot11mode)) {
			populate_dot11f_vht_caps(mac_ctx, pesession,
						 &pr.VHTCaps);
			is_vht_enabled = true;
		}
	}
	if (pesession != NULL)
		populate_dot11f_ext_cap(mac_ctx, is_vht_enabled, &pr.ExtCap,
			pesession);

	/* That's it-- now we pack it.  First, how much space are we going to */
	status = dot11f_get_packed_probe_request_size(mac_ctx, &pr, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGP, FL("Failed to calculate the packed size for a Probe Request (0x%08x)."), status);
		/* We'll fall back on the worst case scenario: */
		payload = sizeof(tDot11fProbeRequest);
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("There were warnings while calculating the packed size for a Probe Request (0x%08x)."), status);
	}

	if (addn_ielen) {
		qdf_mem_zero((uint8_t *)&extracted_ext_cap,
			sizeof(tDot11fIEExtCap));
		sir_status = lim_strip_extcap_update_struct(mac_ctx,
					additional_ie,
					&addn_ielen,
					&extracted_ext_cap);
		if (eSIR_SUCCESS != sir_status) {
			lim_log(mac_ctx, LOG1,
			FL("Unable to Stripoff ExtCap IE from Probe Req"));
		} else {
			struct s_ext_cap *p_ext_cap =
				(struct s_ext_cap *)
					extracted_ext_cap.bytes;
			if (p_ext_cap->interworking_service)
				p_ext_cap->qos_map = 1;
			extracted_ext_cap_flag =
				lim_is_ext_cap_ie_present(p_ext_cap);
		}
	}

	bytes = payload + sizeof(tSirMacMgmtHdr) + addn_ielen;

	/* Ok-- try to allocate some memory: */
	qdf_status = cds_packet_alloc((uint16_t) bytes, (void **)&frame,
				      (void **)&packet);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGP, FL("Failed to allocate %d bytes for a Probe Request."), bytes);
		return eSIR_MEM_ALLOC_FAILED;
	}
	/* Paranoia: */
	qdf_mem_set(frame, bytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(mac_ctx, frame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_PROBE_REQ, bssid, self_macaddr);

	/* merge the ExtCap struct*/
	if (extracted_ext_cap_flag)
		lim_merge_extcap_struct(&pr.ExtCap, &extracted_ext_cap);

	/* That done, pack the Probe Request: */
	status = dot11f_pack_probe_request(mac_ctx, &pr, frame +
					    sizeof(tSirMacMgmtHdr),
					    payload, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Failed to pack a Probe Request (0x%08x)."), status);
		cds_packet_free((void *)packet);
		return eSIR_FAILURE;    /* allocated! */
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW, FL("There were warnings while packing a Probe Request (0x%08x)."), status);
	}
	/* Append any AddIE if present. */
	if (addn_ielen) {
		qdf_mem_copy(frame + sizeof(tSirMacMgmtHdr) + payload,
			     additional_ie, addn_ielen);
		payload += addn_ielen;
	}

	/* If this probe request is sent during P2P Search State, then we need
	 * to send it at OFDM rate.
	 */
	if ((SIR_BAND_5_GHZ == lim_get_rf_band(channel))
	    || ((mac_ctx->lim.gpLimMlmScanReq != NULL) &&
		mac_ctx->lim.gpLimMlmScanReq->p2pSearch)
		/*
		 * For unicast probe req mgmt from Join function we don't set
		 * above variables. So we need to add one more check whether it
		 * is pePersona is P2P_CLIENT or not
		 */
	    || ((pesession != NULL) &&
		(QDF_P2P_CLIENT_MODE == pesession->pePersona))
	    ) {
		txflag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	qdf_status =
		wma_tx_frame(mac_ctx, packet,
			   (uint16_t) sizeof(tSirMacMgmtHdr) + payload,
			   TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS, 7,
			   lim_tx_complete, frame, txflag, sme_sessionid,
			   0);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGE,
			FL("could not send Probe Request frame!"));
		/* Pkt will be freed up by the callback */
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
} /* End lim_send_probe_req_mgmt_frame. */

tSirRetStatus lim_get_addn_ie_for_probe_resp(tpAniSirGlobal pMac,
					     uint8_t *addIE, uint16_t *addnIELen,
					     uint8_t probeReqP2pIe)
{
	/* If Probe request doesn't have P2P IE, then take out P2P IE
	   from additional IE */
	if (!probeReqP2pIe) {
		uint8_t *tempbuf = NULL;
		uint16_t tempLen = 0;
		int left = *addnIELen;
		uint8_t *ptr = addIE;
		uint8_t elem_id, elem_len;

		if (NULL == addIE) {
			PELOGE(lim_log(pMac, LOGE, FL(" NULL addIE pointer"));)
			return eSIR_FAILURE;
		}

		tempbuf = qdf_mem_malloc(left);
		if (NULL == tempbuf) {
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("Unable to allocate memory to store addn IE"));
			       )
			return eSIR_MEM_ALLOC_FAILED;
		}

		while (left >= 2) {
			elem_id = ptr[0];
			elem_len = ptr[1];
			left -= 2;
			if (elem_len > left) {
				lim_log(pMac, LOGE,
					FL
						("****Invalid IEs eid = %d elem_len=%d left=%d*****"),
					elem_id, elem_len, left);
				qdf_mem_free(tempbuf);
				return eSIR_FAILURE;
			}
			if (!((SIR_MAC_EID_VENDOR == elem_id) &&
			      (memcmp
				       (&ptr[2], SIR_MAC_P2P_OUI,
				       SIR_MAC_P2P_OUI_SIZE) == 0))) {
				qdf_mem_copy(tempbuf + tempLen, &ptr[0],
					     elem_len + 2);
				tempLen += (elem_len + 2);
			}
			left -= elem_len;
			ptr += (elem_len + 2);
		}
		qdf_mem_copy(addIE, tempbuf, tempLen);
		*addnIELen = tempLen;
		qdf_mem_free(tempbuf);
	}
	return eSIR_SUCCESS;
}

/**
 * lim_send_probe_rsp_mgmt_frame() - Send probe response
 *
 * @mac_ctx: Handle for mac context
 * @peer_macaddr: Mac address of requesting peer
 * @ssid: SSID for response
 * @n_staid: Station ID, currently unused.
 * @pe_session: PE session id
 * @keepalive: Keep alive flag. Currently unused.
 * @preq_p2pie: P2P IE in incoming probe request
 *
 * Builds and sends probe response frame to the requesting peer
 *
 * Return: void
 */

void
lim_send_probe_rsp_mgmt_frame(tpAniSirGlobal mac_ctx,
			      tSirMacAddr peer_macaddr,
			      tpAniSSID ssid,
			      short n_staid,
			      uint8_t keepalive,
			      tpPESession pe_session, uint8_t preq_p2pie)
{
	tDot11fProbeResponse *frm;
	tSirRetStatus sir_status;
	uint32_t cfg, payload, bytes, status;
	tpSirMacMgmtHdr mac_hdr;
	uint8_t *frame;
	void *packet = NULL;
	QDF_STATUS qdf_status;
	uint32_t addn_ie_present = false;

	uint16_t addn_ie_len = 0;
	uint32_t wps_ap = 0, tmp;
	uint8_t tx_flag = 0;
	uint8_t *add_ie = NULL;
	uint8_t *p2p_ie = NULL;
	uint8_t noalen = 0;
	uint8_t total_noalen = 0;
	uint8_t noa_stream[SIR_MAX_NOA_ATTR_LEN + SIR_P2P_IE_HEADER_LEN];
	uint8_t noa_ie[SIR_MAX_NOA_ATTR_LEN + SIR_P2P_IE_HEADER_LEN];
	uint8_t sme_sessionid = 0;
	bool is_vht_enabled = false;
	tDot11fIEExtCap extracted_ext_cap;
	bool extracted_ext_cap_flag = false;

	/* We don't answer requests in this case*/
	if (ANI_DRIVER_TYPE(mac_ctx) == eDRIVER_TYPE_MFG)
		return;

	if (NULL == pe_session)
		return;

	/*
	 * In case when cac timer is running for this SAP session then
	 * avoid sending probe rsp out. It is violation of dfs specification.
	 */
	if (((pe_session->pePersona == QDF_SAP_MODE) ||
	    (pe_session->pePersona == QDF_P2P_GO_MODE)) &&
	    (true == mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("CAC timer is running, probe response dropped"));
		return;
	}
	sme_sessionid = pe_session->smeSessionId;
	frm = qdf_mem_malloc(sizeof(tDot11fProbeResponse));
	if (NULL == frm) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to allocate memory"));
		return;
	}

	qdf_mem_zero(&extracted_ext_cap, sizeof(extracted_ext_cap));

	/*
	 * Fill out 'frm', after which we'll just hand the struct off to
	 * 'dot11f_pack_probe_response'.
	 */
	qdf_mem_set((uint8_t *) frm, sizeof(tDot11fProbeResponse), 0);

	/*
	 * Timestamp to be updated by TFP, below.
	 *
	 * Beacon Interval:
	 */
	if (LIM_IS_AP_ROLE(pe_session)) {
		frm->BeaconInterval.interval =
			mac_ctx->sch.schObject.gSchBeaconInterval;
	} else {
		sir_status = wlan_cfg_get_int(mac_ctx,
				WNI_CFG_BEACON_INTERVAL, &cfg);
		if (eSIR_SUCCESS != sir_status) {
			lim_log(mac_ctx, LOGP,
				FL("Failed to get WNI_CFG_BEACON_INTERVAL (%d)."),
				sir_status);
			goto err_ret;
		}
		frm->BeaconInterval.interval = (uint16_t) cfg;
	}

	populate_dot11f_capabilities(mac_ctx, &frm->Capabilities, pe_session);
	populate_dot11f_ssid(mac_ctx, (tSirMacSSid *) ssid, &frm->SSID);
	populate_dot11f_supp_rates(mac_ctx, POPULATE_DOT11F_RATES_OPERATIONAL,
		&frm->SuppRates, pe_session);

	populate_dot11f_ds_params(mac_ctx, &frm->DSParams,
		pe_session->currentOperChannel);
	populate_dot11f_ibss_params(mac_ctx, &frm->IBSSParams, pe_session);

	if (LIM_IS_AP_ROLE(pe_session)) {
		if (pe_session->wps_state != SAP_WPS_DISABLED)
			populate_dot11f_probe_res_wpsi_es(mac_ctx,
				&frm->WscProbeRes,
				pe_session);
	} else {
		if (wlan_cfg_get_int(mac_ctx, (uint16_t) WNI_CFG_WPS_ENABLE,
			&tmp) != eSIR_SUCCESS)
			lim_log(mac_ctx, LOGP, "Failed to cfg get id %d",
				WNI_CFG_WPS_ENABLE);

		wps_ap = tmp & WNI_CFG_WPS_ENABLE_AP;

		if (wps_ap)
			populate_dot11f_wsc_in_probe_res(mac_ctx,
				&frm->WscProbeRes);

		if (mac_ctx->lim.wscIeInfo.probeRespWscEnrollmentState ==
		    eLIM_WSC_ENROLL_BEGIN) {
			populate_dot11f_wsc_registrar_info_in_probe_res(mac_ctx,
				&frm->WscProbeRes);
			mac_ctx->lim.wscIeInfo.probeRespWscEnrollmentState =
				eLIM_WSC_ENROLL_IN_PROGRESS;
		}

		if (mac_ctx->lim.wscIeInfo.wscEnrollmentState ==
		    eLIM_WSC_ENROLL_END) {
			de_populate_dot11f_wsc_registrar_info_in_probe_res(
				mac_ctx, &frm->WscProbeRes);
			mac_ctx->lim.wscIeInfo.probeRespWscEnrollmentState =
				eLIM_WSC_ENROLL_NOOP;
		}
	}

	populate_dot11f_country(mac_ctx, &frm->Country, pe_session);
	populate_dot11f_edca_param_set(mac_ctx, &frm->EDCAParamSet, pe_session);

	if (pe_session->dot11mode != WNI_CFG_DOT11_MODE_11B)
		populate_dot11f_erp_info(mac_ctx, &frm->ERPInfo, pe_session);

	populate_dot11f_ext_supp_rates(mac_ctx,
		POPULATE_DOT11F_RATES_OPERATIONAL,
		&frm->ExtSuppRates, pe_session);

	/* Populate HT IEs, when operating in 11n */
	if (pe_session->htCapability) {
		populate_dot11f_ht_caps(mac_ctx, pe_session, &frm->HTCaps);
		populate_dot11f_ht_info(mac_ctx, &frm->HTInfo, pe_session);
	}
	if (pe_session->vhtCapability) {
		lim_log(mac_ctx, LOG1, FL("Populate VHT IE in Probe Response"));
		populate_dot11f_vht_caps(mac_ctx, pe_session, &frm->VHTCaps);
		populate_dot11f_vht_operation(mac_ctx, pe_session,
			&frm->VHTOperation);
		/*
		 * we do not support multi users yet.
		 * populate_dot11f_vht_ext_bss_load( mac_ctx,
		 *         &frm.VHTExtBssLoad );
		 */
		is_vht_enabled = true;
	}

	populate_dot11f_ext_cap(mac_ctx, is_vht_enabled, &frm->ExtCap,
		pe_session);

	if (pe_session->pLimStartBssReq) {
		populate_dot11f_wpa(mac_ctx,
			&(pe_session->pLimStartBssReq->rsnIE),
			&frm->WPA);
		populate_dot11f_rsn_opaque(mac_ctx,
			&(pe_session->pLimStartBssReq->rsnIE),
			&frm->RSNOpaque);
	}

	populate_dot11f_wmm(mac_ctx, &frm->WMMInfoAp, &frm->WMMParams,
		&frm->WMMCaps, pe_session);

#if defined(FEATURE_WLAN_WAPI)
	if (pe_session->pLimStartBssReq)
		populate_dot11f_wapi(mac_ctx,
			&(pe_session->pLimStartBssReq->rsnIE),
			&frm->WAPI);
#endif /* defined(FEATURE_WLAN_WAPI) */

	status = dot11f_get_packed_probe_response_size(mac_ctx, frm, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGP,
			FL("Probe Response size error (0x%08x)."),
			status);
		/* We'll fall back on the worst case scenario: */
		payload = sizeof(tDot11fProbeResponse);
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("Probe Response size warning (0x%08x)."),
			status);
	}

	bytes = payload + sizeof(tSirMacMgmtHdr);

	if (mac_ctx->lim.gpLimRemainOnChanReq)
		bytes += (mac_ctx->lim.gpLimRemainOnChanReq->length -
			 sizeof(tSirRemainOnChnReq));
	else
		/*
		 * Only use CFG for non-listen mode. This CFG is not working for
		 * concurrency. In listening mode, probe rsp IEs is passed in
		 * the message from SME to PE.
		 */
		addn_ie_present =
			(pe_session->addIeParams.probeRespDataLen != 0);

	if (addn_ie_present) {

		add_ie = qdf_mem_malloc(
				pe_session->addIeParams.probeRespDataLen);
		if (NULL == add_ie) {
			lim_log(mac_ctx, LOGE,
				FL("add_ie allocation failed"));
			goto err_ret;
		}

		qdf_mem_copy(add_ie,
			     pe_session->addIeParams.probeRespData_buff,
			     pe_session->addIeParams.probeRespDataLen);
		addn_ie_len = pe_session->addIeParams.probeRespDataLen;

		if (eSIR_SUCCESS != lim_get_addn_ie_for_probe_resp(mac_ctx,
					add_ie, &addn_ie_len, preq_p2pie)) {
			lim_log(mac_ctx, LOGP,
				FL("Unable to get addn_ie"));
			goto err_ret;
		}

		sir_status = lim_strip_extcap_update_struct(mac_ctx,
					add_ie, &addn_ie_len,
					&extracted_ext_cap);
		if (eSIR_SUCCESS != sir_status) {
			lim_log(mac_ctx, LOG1,
				FL("Unable to strip off ExtCap IE"));
		} else {
			extracted_ext_cap_flag = true;
		}

		bytes = bytes + addn_ie_len;

		if (preq_p2pie)
			p2p_ie = limGetP2pIEPtr(mac_ctx, &add_ie[0],
					addn_ie_len);

		if (p2p_ie != NULL) {
			/* get NoA attribute stream P2P IE */
			noalen = lim_get_noa_attr_stream(mac_ctx,
					noa_stream, pe_session);
			if (noalen != 0) {
				total_noalen =
					lim_build_p2p_ie(mac_ctx, &noa_ie[0],
						&noa_stream[0], noalen);
				bytes = bytes + total_noalen;
			}
		}
	}

	qdf_status = cds_packet_alloc((uint16_t) bytes, (void **)&frame,
				      (void **)&packet);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGP, FL("Probe Response allocation failed"));
		goto err_ret;
	}
	/* Paranoia: */
	qdf_mem_set(frame, bytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(mac_ctx, frame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_PROBE_RSP, peer_macaddr,
		pe_session->selfMacAddr);

	mac_hdr = (tpSirMacMgmtHdr) frame;

	sir_copy_mac_addr(mac_hdr->bssId, pe_session->bssId);

	/* merge ExtCap IE */
	if (extracted_ext_cap_flag)
		lim_merge_extcap_struct(&frm->ExtCap, &extracted_ext_cap);

	/* That done, pack the Probe Response: */
	status =
		dot11f_pack_probe_response(mac_ctx, frm,
			frame + sizeof(tSirMacMgmtHdr),
			payload, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Probe Response pack failure (0x%08x)."),
			status);
			goto err_ret;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("Probe Response pack warning (0x%08x)."),
			status);
	}

	lim_log(mac_ctx, LOG3, FL("Sending Probe Response frame to "));
	lim_print_mac_addr(mac_ctx, peer_macaddr, LOG3);

	mac_ctx->sys.probeRespond++;

	if (mac_ctx->lim.gpLimRemainOnChanReq)
		qdf_mem_copy(frame + sizeof(tSirMacMgmtHdr) + payload,
			     mac_ctx->lim.gpLimRemainOnChanReq->probeRspIe,
			     (mac_ctx->lim.gpLimRemainOnChanReq->length -
			      sizeof(tSirRemainOnChnReq)));

	if (addn_ie_present)
		qdf_mem_copy(frame + sizeof(tSirMacMgmtHdr) + payload,
			     &add_ie[0], addn_ie_len);

	if (noalen != 0) {
		if (total_noalen >
		    (SIR_MAX_NOA_ATTR_LEN + SIR_P2P_IE_HEADER_LEN)) {
			lim_log(mac_ctx, LOGE,
				FL("Not able to insert NoA, total len=%d"),
				total_noalen);
			goto err_ret;
		} else {
			qdf_mem_copy(&frame[bytes - (total_noalen)],
				     &noa_ie[0], total_noalen);
		}
	}

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(pe_session->currentOperChannel))
	    || (pe_session->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (pe_session->pePersona == QDF_P2P_GO_MODE)
	    )
		tx_flag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;

	/* Queue Probe Response frame in high priority WQ */
	qdf_status = wma_tx_frame((tHalHandle) mac_ctx, packet,
				(uint16_t) bytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, frame, tx_flag,
				sme_sessionid, 0);

	/* Pkt will be freed up by the callback */
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		lim_log(mac_ctx, LOGE, FL("Could not send Probe Response."));

	if (add_ie != NULL)
		qdf_mem_free(add_ie);

	qdf_mem_free(frm);
	return;

err_ret:
	if (add_ie != NULL)
		qdf_mem_free(add_ie);
	if (frm != NULL)
		qdf_mem_free(frm);
	if (packet != NULL)
		cds_packet_free((void *)packet);
	return;

} /* End lim_send_probe_rsp_mgmt_frame. */

void
lim_send_addts_req_action_frame(tpAniSirGlobal pMac,
				tSirMacAddr peerMacAddr,
				tSirAddtsReqInfo *pAddTS, tpPESession psessionEntry)
{
	uint16_t i;
	uint8_t *pFrame;
	tDot11fAddTSRequest AddTSReq;
	tDot11fWMMAddTSRequest WMMAddTSReq;
	uint32_t nPayload, nBytes, nStatus;
	tpSirMacMgmtHdr pMacHdr;
	void *pPacket;
#ifdef FEATURE_WLAN_ESE
	uint32_t phyMode;
#endif
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	if (NULL == psessionEntry) {
		return;
	}

	smeSessionId = psessionEntry->smeSessionId;

	if (!pAddTS->wmeTspecPresent) {
		qdf_mem_set((uint8_t *) &AddTSReq, sizeof(AddTSReq), 0);

		AddTSReq.Action.action = SIR_MAC_QOS_ADD_TS_REQ;
		AddTSReq.DialogToken.token = pAddTS->dialogToken;
		AddTSReq.Category.category = SIR_MAC_ACTION_QOS_MGMT;
		if (pAddTS->lleTspecPresent) {
			populate_dot11f_tspec(&pAddTS->tspec, &AddTSReq.TSPEC);
		} else {
			populate_dot11f_wmmtspec(&pAddTS->tspec,
						 &AddTSReq.WMMTSPEC);
		}

		if (pAddTS->lleTspecPresent) {
			AddTSReq.num_WMMTCLAS = 0;
			AddTSReq.num_TCLAS = pAddTS->numTclas;
			for (i = 0; i < pAddTS->numTclas; ++i) {
				populate_dot11f_tclas(pMac, &pAddTS->tclasInfo[i],
						      &AddTSReq.TCLAS[i]);
			}
		} else {
			AddTSReq.num_TCLAS = 0;
			AddTSReq.num_WMMTCLAS = pAddTS->numTclas;
			for (i = 0; i < pAddTS->numTclas; ++i) {
				populate_dot11f_wmmtclas(pMac,
							 &pAddTS->tclasInfo[i],
							 &AddTSReq.WMMTCLAS[i]);
			}
		}

		if (pAddTS->tclasProcPresent) {
			if (pAddTS->lleTspecPresent) {
				AddTSReq.TCLASSPROC.processing =
					pAddTS->tclasProc;
				AddTSReq.TCLASSPROC.present = 1;
			} else {
				AddTSReq.WMMTCLASPROC.version = 1;
				AddTSReq.WMMTCLASPROC.processing =
					pAddTS->tclasProc;
				AddTSReq.WMMTCLASPROC.present = 1;
			}
		}

		nStatus =
			dot11f_get_packed_add_ts_request_size(pMac, &AddTSReq, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGP,
				FL("Failed to calculate the packed size f"
				   "or an Add TS Request (0x%08x)."), nStatus);
			/* We'll fall back on the worst case scenario: */
			nPayload = sizeof(tDot11fAddTSRequest);
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while calculating"
				   "the packed size for an Add TS Request"
				   " (0x%08x)."), nStatus);
		}
	} else {
		qdf_mem_set((uint8_t *) &WMMAddTSReq, sizeof(WMMAddTSReq), 0);

		WMMAddTSReq.Action.action = SIR_MAC_QOS_ADD_TS_REQ;
		WMMAddTSReq.DialogToken.token = pAddTS->dialogToken;
		WMMAddTSReq.Category.category = SIR_MAC_ACTION_WME;

		/* WMM spec 2.2.10 - status code is only filled in for ADDTS response */
		WMMAddTSReq.StatusCode.statusCode = 0;

		populate_dot11f_wmmtspec(&pAddTS->tspec, &WMMAddTSReq.WMMTSPEC);
#ifdef FEATURE_WLAN_ESE
		lim_get_phy_mode(pMac, &phyMode, psessionEntry);

		if (phyMode == WNI_CFG_PHY_MODE_11G
		    || phyMode == WNI_CFG_PHY_MODE_11A) {
			pAddTS->tsrsIE.rates[0] = TSRS_11AG_RATE_6MBPS;
		} else {
			pAddTS->tsrsIE.rates[0] = TSRS_11B_RATE_5_5MBPS;
		}
		populate_dot11_tsrsie(pMac, &pAddTS->tsrsIE,
				      &WMMAddTSReq.ESETrafStrmRateSet,
				      sizeof(uint8_t));
#endif
		/* fillWmeTspecIE */

		nStatus =
			dot11f_get_packed_wmm_add_ts_request_size(pMac, &WMMAddTSReq,
								  &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGP,
				FL("Failed to calculate the packed size f"
				   "or a WMM Add TS Request (0x%08x)."),
				nStatus);
			/* We'll fall back on the worst case scenario: */
			nPayload = sizeof(tDot11fAddTSRequest);
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while calculating"
				   "the packed size for a WMM Add TS Requ"
				   "est (0x%08x)."), nStatus);
		}
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				      (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for an Ad"
				       "d TS Request."), nBytes);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peerMacAddr, psessionEntry->selfMacAddr);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peerMacAddr, pMacHdr);
#endif

	/* That done, pack the struct: */
	if (!pAddTS->wmeTspecPresent) {
		nStatus = dot11f_pack_add_ts_request(pMac, &AddTSReq,
						     pFrame +
						     sizeof(tSirMacMgmtHdr),
						     nPayload, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGE,
				FL("Failed to pack an Add TS Request "
				   "(0x%08x)."), nStatus);
			cds_packet_free((void *)pPacket);
			return; /* allocated! */
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while packing "
				   "an Add TS Request (0x%08x)."), nStatus);
		}
	} else {
		nStatus = dot11f_pack_wmm_add_ts_request(pMac, &WMMAddTSReq,
							 pFrame +
							 sizeof(tSirMacMgmtHdr),
							 nPayload, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGE,
				FL("Failed to pack a WMM Add TS Reque"
				   "st (0x%08x)."), nStatus);
			cds_packet_free((void *)pPacket);
			return; /* allocated! */
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while packing "
				   "a WMM Add TS Request (0x%08x)."), nStatus);
		}
	}

	PELOG3(lim_log(pMac, LOG3, FL("Sending an Add TS Request frame to "));
	       lim_print_mac_addr(pMac, peerMacAddr, LOG3);
	       )

	if ((SIR_BAND_5_GHZ ==
	     lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE)
	    || (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));

	/* Queue Addts Response frame in high priority WQ */
	qdf_status = wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGE, FL("*** Could not send an Add TS Request"
				       " (%X) ***"), qdf_status);
		/* Pkt will be freed up by the callback */
	}

} /* End lim_send_addts_req_action_frame. */

/**
 * lim_send_assoc_rsp_mgmt_frame() - Send assoc response
 * @mac_ctx: Handle for mac context
 * @status_code: Status code for assoc response frame
 * @aid: Association ID
 * @peer_addr: Mac address of requesting peer
 * @subtype: Assoc/Reassoc
 * @sta: Pointer to station node
 * @pe_session: PE session id.
 *
 * Builds and sends association response frame to the requesting peer.
 *
 * Return: void
 */

void
lim_send_assoc_rsp_mgmt_frame(tpAniSirGlobal mac_ctx,
	uint16_t status_code, uint16_t aid, tSirMacAddr peer_addr,
	uint8_t subtype, tpDphHashNode sta, tpPESession pe_session)
{
	static tDot11fAssocResponse frm;
	uint8_t *frame;
	tpSirMacMgmtHdr mac_hdr;
	tSirRetStatus sir_status;
	uint8_t lle_mode = 0, addts;
	tHalBitVal qos_mode, wme_mode;
	uint32_t payload, bytes, status;
	void *packet;
	QDF_STATUS qdf_status;
	tUpdateBeaconParams beacon_params;
	uint8_t tx_flag = 0;
	uint32_t addn_ie_len = 0;
	uint8_t add_ie[WNI_CFG_ASSOC_RSP_ADDNIE_DATA_LEN];
	tpSirAssocReq assoc_req = NULL;
	uint8_t sme_session = 0;
	bool is_vht = false;
	uint16_t stripoff_len = 0;
	tDot11fIEExtCap extracted_ext_cap;
	bool extracted_flag = false;
#ifdef WLAN_FEATURE_11W
	uint32_t retry_int;
	uint32_t max_retries;
#endif

	if (NULL == pe_session) {
		lim_log(mac_ctx, LOGE, FL("pe_session is NULL"));
		return;
	}

	sme_session = pe_session->smeSessionId;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	limGetQosMode(pe_session, &qos_mode);
	limGetWmeMode(pe_session, &wme_mode);

	/*
	 * An Add TS IE is added only if the AP supports it and
	 * the requesting STA sent a traffic spec.
	 */
	addts = (qos_mode && sta && sta->qos.addtsPresent) ? 1 : 0;

	frm.Status.status = status_code;

	frm.AID.associd = aid | LIM_AID_MASK;

	if (NULL == sta) {
		populate_dot11f_supp_rates(mac_ctx,
			POPULATE_DOT11F_RATES_OPERATIONAL,
			&frm.SuppRates, pe_session);
		populate_dot11f_ext_supp_rates(mac_ctx,
			POPULATE_DOT11F_RATES_OPERATIONAL,
			&frm.ExtSuppRates, pe_session);
	} else {
		populate_dot11f_assoc_rsp_rates(mac_ctx, &frm.SuppRates,
			&frm.ExtSuppRates,
			sta->supportedRates.llbRates,
			sta->supportedRates.llaRates);
	}

	if (LIM_IS_AP_ROLE(pe_session) && sta != NULL &&
	    eSIR_SUCCESS == status_code) {
		assoc_req = (tpSirAssocReq)
			pe_session->parsedAssocReq[sta->assocId];
		/*
		 * populate P2P IE in AssocRsp when assocReq from the peer
		 * includes P2P IE
		 */
		if (assoc_req != NULL && assoc_req->addIEPresent)
			populate_dot11_assoc_res_p2p_ie(mac_ctx,
				&frm.P2PAssocRes,
				assoc_req);
	}

	if (NULL != sta) {
		if (eHAL_SET == qos_mode) {
			if (sta->lleEnabled) {
				lle_mode = 1;
				populate_dot11f_edca_param_set(mac_ctx,
					&frm.EDCAParamSet, pe_session);
			}
		}

		if ((!lle_mode) && (eHAL_SET == wme_mode) && sta->wmeEnabled) {
			populate_dot11f_wmm_params(mac_ctx, &frm.WMMParams,
				pe_session);

			if (sta->wsmEnabled)
				populate_dot11f_wmm_caps(&frm.WMMCaps);
		}

		if (sta->mlmStaContext.htCapability &&
		    pe_session->htCapability) {
			lim_log(mac_ctx, LOG1,
				FL("Populate HT IEs in Assoc Response"));
			populate_dot11f_ht_caps(mac_ctx, pe_session,
				&frm.HTCaps);
			/*
			 * Check the STA capability and
			 * update the HTCaps accordingly
			 */
			frm.HTCaps.supportedChannelWidthSet = (
				sta->htSupportedChannelWidthSet <
				     pe_session->htSupportedChannelWidthSet) ?
				      sta->htSupportedChannelWidthSet :
				       pe_session->htSupportedChannelWidthSet;
			if (!frm.HTCaps.supportedChannelWidthSet)
				frm.HTCaps.shortGI40MHz = 0;

			populate_dot11f_ht_info(mac_ctx, &frm.HTInfo,
				pe_session);
		}
		lim_log(mac_ctx, LOG1,
			FL("SupportedChnlWidth: %d, mimoPS: %d, GF: %d, short GI20:%d, shortGI40: %d, dsssCck: %d, AMPDU Param: %x"),
			frm.HTCaps.supportedChannelWidthSet,
			frm.HTCaps.mimoPowerSave,
			frm.HTCaps.greenField, frm.HTCaps.shortGI20MHz,
			frm.HTCaps.shortGI40MHz,
			frm.HTCaps.dsssCckMode40MHz,
			frm.HTCaps.maxRxAMPDUFactor);

		if (sta->mlmStaContext.vhtCapability &&
		    pe_session->vhtCapability) {
			lim_log(mac_ctx, LOG1,
				FL("Populate VHT IEs in Assoc Response"));
			populate_dot11f_vht_caps(mac_ctx, pe_session,
				&frm.VHTCaps);
			populate_dot11f_vht_operation(mac_ctx, pe_session,
				&frm.VHTOperation);
			is_vht = true;
		}
		populate_dot11f_ext_cap(mac_ctx, is_vht, &frm.ExtCap,
			pe_session);

#ifdef WLAN_FEATURE_11W
		if (eSIR_MAC_TRY_AGAIN_LATER == status_code) {
			if (wlan_cfg_get_int
				    (mac_ctx, WNI_CFG_PMF_SA_QUERY_MAX_RETRIES,
				    &max_retries) != eSIR_SUCCESS)
				lim_log(mac_ctx, LOGE,
					FL("get WNI_CFG_PMF_SA_QUERY_MAX_RETRIES failure"));
			else if (wlan_cfg_get_int
					 (mac_ctx,
					 WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL,
					 &retry_int) != eSIR_SUCCESS)
				lim_log(mac_ctx, LOGE,
					FL("get WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL failure"));
			else
				populate_dot11f_timeout_interval(mac_ctx,
					&frm.TimeoutInterval,
					SIR_MAC_TI_TYPE_ASSOC_COMEBACK,
					(max_retries -
						sta->pmfSaQueryRetryCount)
						* retry_int);
		}
#endif
	}

	qdf_mem_set((uint8_t *) &beacon_params, sizeof(beacon_params), 0);

	if (LIM_IS_AP_ROLE(pe_session) &&
		(pe_session->gLimProtectionControl !=
		    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE))
			lim_decide_ap_protection(mac_ctx, peer_addr,
				&beacon_params, pe_session);

	lim_update_short_preamble(mac_ctx, peer_addr, &beacon_params,
		pe_session);
	lim_update_short_slot_time(mac_ctx, peer_addr, &beacon_params,
		pe_session);

	/*
	 * Populate Do11capabilities after updating session with
	 * Assos req details
	 */
	populate_dot11f_capabilities(mac_ctx, &frm.Capabilities, pe_session);

	beacon_params.bssIdx = pe_session->bssIdx;

	/* Send message to HAL about beacon parameter change. */
	if ((false == mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running)
	    && beacon_params.paramChangeBitmap) {
		sch_set_fixed_beacon_fields(mac_ctx, pe_session);
		lim_send_beacon_params(mac_ctx, &beacon_params, pe_session);
	}
	/* Allocate a buffer for this frame: */
	status = dot11f_get_packed_assoc_response_size(mac_ctx, &frm, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("get Association Response size failure (0x%08x)."),
			status);
		return;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("get Association Response size warning (0x%08x)."),
			status);
	}

	bytes = sizeof(tSirMacMgmtHdr) + payload;

	if (assoc_req != NULL) {
		addn_ie_len = pe_session->addIeParams.assocRespDataLen;

		/* Nonzero length indicates Assoc rsp IE available */
		if (addn_ie_len > 0 &&
		    addn_ie_len <= WNI_CFG_ASSOC_RSP_ADDNIE_DATA_LEN &&
		    (bytes + addn_ie_len) <= SIR_MAX_PACKET_SIZE) {
			qdf_mem_copy(add_ie,
				pe_session->addIeParams.assocRespData_buff,
				pe_session->addIeParams.assocRespDataLen);

			qdf_mem_set((uint8_t *) &extracted_ext_cap,
				    sizeof(extracted_ext_cap), 0);

			stripoff_len = addn_ie_len;
			sir_status =
				lim_strip_extcap_update_struct
					(mac_ctx, &add_ie[0], &stripoff_len,
					&extracted_ext_cap);
			if (eSIR_SUCCESS != sir_status) {
				lim_log(mac_ctx, LOG1,
					FL("strip off extcap IE failed"));
			} else {
				addn_ie_len = stripoff_len;
				extracted_flag = true;
			}
			bytes = bytes + addn_ie_len;
		}
		lim_log(mac_ctx, LOG1,
			FL("addn_ie_len = %d for Assoc Resp : %d"),
			addn_ie_len, assoc_req->addIEPresent);
	}
	qdf_status = cds_packet_alloc((uint16_t) bytes, (void **)&frame,
				      (void **)&packet);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGP, FL("cds_packet_alloc failed."));
		return;
	}
	/* Paranoia: */
	qdf_mem_set(frame, bytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(mac_ctx, frame, SIR_MAC_MGMT_FRAME,
		(LIM_ASSOC == subtype) ?
			SIR_MAC_MGMT_ASSOC_RSP : SIR_MAC_MGMT_REASSOC_RSP,
			peer_addr,
			pe_session->selfMacAddr);
	mac_hdr = (tpSirMacMgmtHdr) frame;

	sir_copy_mac_addr(mac_hdr->bssId, pe_session->bssId);

	/* merge the ExtCap struct */
	if (extracted_flag)
		lim_merge_extcap_struct(&(frm.ExtCap), &extracted_ext_cap);
	status = dot11f_pack_assoc_response(mac_ctx, &frm,
					     frame + sizeof(tSirMacMgmtHdr),
					     payload, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Association Response pack failure(0x%08x)."),
			status);
		cds_packet_free((void *)packet);
		return;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("Association Response pack warning (0x%08x)."),
			status);
	}

	if (subtype == LIM_ASSOC)
		lim_log(mac_ctx, LOG1,
			FL("*** Sending Assoc Resp status %d aid %d to "),
			status_code, aid);
	else
		lim_log(mac_ctx, LOG1,
			FL("*** Sending ReAssoc Resp status %d aid %d to "),
			status_code, aid);

	lim_print_mac_addr(mac_ctx, mac_hdr->da, LOG1);

	if (addn_ie_len && addn_ie_len <= WNI_CFG_ASSOC_RSP_ADDNIE_DATA_LEN)
		qdf_mem_copy(frame + sizeof(tSirMacMgmtHdr) + payload,
			     &add_ie[0], addn_ie_len);

	if ((SIR_BAND_5_GHZ ==
		lim_get_rf_band(pe_session->currentOperChannel)) ||
			(pe_session->pePersona == QDF_P2P_CLIENT_MODE) ||
			(pe_session->pePersona == QDF_P2P_GO_MODE))
		tx_flag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 pe_session->peSessionId, mac_hdr->fc.subType));
	/* Queue Association Response frame in high priority WQ */
	qdf_status = wma_tx_frame(mac_ctx, packet, (uint16_t) bytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, frame, tx_flag,
				sme_session, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 pe_session->peSessionId, qdf_status));

	/* Pkt will be freed up by the callback */
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		lim_log(mac_ctx, LOGE,
			FL("*** Could not Send Re/AssocRsp, retCode=%X ***"),
			qdf_status);

	/*
	 * update the ANI peer station count.
	 * FIXME_PROTECTION : take care of different type of station
	 * counter inside this function.
	 */
	lim_util_count_sta_add(mac_ctx, sta, pe_session);

}

void
lim_send_delts_req_action_frame(tpAniSirGlobal pMac,
				tSirMacAddr peer,
				uint8_t wmmTspecPresent,
				tSirMacTSInfo *pTsinfo,
				tSirMacTspecIE *pTspecIe, tpPESession psessionEntry)
{
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	tDot11fDelTS DelTS;
	tDot11fWMMDelTS WMMDelTS;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	if (NULL == psessionEntry) {
		return;
	}

	smeSessionId = psessionEntry->smeSessionId;

	if (!wmmTspecPresent) {
		qdf_mem_set((uint8_t *) &DelTS, sizeof(DelTS), 0);

		DelTS.Category.category = SIR_MAC_ACTION_QOS_MGMT;
		DelTS.Action.action = SIR_MAC_QOS_DEL_TS_REQ;
		populate_dot11f_ts_info(pTsinfo, &DelTS.TSInfo);

		nStatus = dot11f_get_packed_del_ts_size(pMac, &DelTS, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGP,
				FL("Failed to calculate the packed si"
				   "ze for a Del TS (0x%08x)."), nStatus);
			/* We'll fall back on the worst case scenario: */
			nPayload = sizeof(tDot11fDelTS);
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while calcula"
				   "ting the packed size for a Del TS"
				   " (0x%08x)."), nStatus);
		}
	} else {
		qdf_mem_set((uint8_t *) &WMMDelTS, sizeof(WMMDelTS), 0);

		WMMDelTS.Category.category = SIR_MAC_ACTION_WME;
		WMMDelTS.Action.action = SIR_MAC_QOS_DEL_TS_REQ;
		WMMDelTS.DialogToken.token = 0;
		WMMDelTS.StatusCode.statusCode = 0;
		populate_dot11f_wmmtspec(pTspecIe, &WMMDelTS.WMMTSPEC);
		nStatus =
			dot11f_get_packed_wmm_del_ts_size(pMac, &WMMDelTS, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGP,
				FL("Failed to calculate the packed si"
				   "ze for a WMM Del TS (0x%08x)."), nStatus);
			/* We'll fall back on the worst case scenario: */
			nPayload = sizeof(tDot11fDelTS);
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while calcula"
				   "ting the packed size for a WMM De"
				   "l TS (0x%08x)."), nStatus);
		}
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for an Ad"
				       "d TS Response."), nBytes);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, psessionEntry->selfMacAddr);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	/* That done, pack the struct: */
	if (!wmmTspecPresent) {
		nStatus = dot11f_pack_del_ts(pMac, &DelTS,
					     pFrame + sizeof(tSirMacMgmtHdr),
					     nPayload, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGE,
				FL("Failed to pack a Del TS frame (0x%08x)."),
				nStatus);
			cds_packet_free((void *)pPacket);
			return; /* allocated! */
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while packing "
				   "a Del TS frame (0x%08x)."), nStatus);
		}
	} else {
		nStatus = dot11f_pack_wmm_del_ts(pMac, &WMMDelTS,
						 pFrame + sizeof(tSirMacMgmtHdr),
						 nPayload, &nPayload);
		if (DOT11F_FAILED(nStatus)) {
			lim_log(pMac, LOGE,
				FL
					("Failed to pack a WMM Del TS frame (0x%08x)."),
				nStatus);
			cds_packet_free((void *)pPacket);
			return; /* allocated! */
		} else if (DOT11F_WARNED(nStatus)) {
			lim_log(pMac, LOGW,
				FL("There were warnings while packing "
				   "a WMM Del TS frame (0x%08x)."), nStatus);
		}
	}

	PELOG1(lim_log
		       (pMac, LOG1, FL("Sending DELTS REQ (size %d) to "), nBytes);
	       lim_print_mac_addr(pMac, pMacHdr->da, LOG1);
	       )

	if ((SIR_BAND_5_GHZ ==
	     lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE)
	    || (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	/* Pkt will be freed up by the callback */
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		lim_log(pMac, LOGE, FL("Failed to send Del TS (%X)!"),
			qdf_status);

} /* End lim_send_delts_req_action_frame. */

/**
 * lim_send_assoc_req_mgmt_frame() - Send association request
 * @mac_ctx: Handle to MAC context
 * @mlm_assoc_req: Association request information
 * @pe_session: PE session information
 *
 * Builds and transmits association request frame to AP.
 *
 * Return: Void
 */

void
lim_send_assoc_req_mgmt_frame(tpAniSirGlobal mac_ctx,
			      tLimMlmAssocReq *mlm_assoc_req,
			      tpPESession pe_session)
{
	tDot11fAssocRequest *frm;
	uint16_t caps;
	uint8_t *frame;
	tSirRetStatus sir_status;
	tLimMlmAssocCnf assoc_cnf;
	uint32_t bytes, payload, status;
	uint8_t qos_enabled, wme_enabled, wsm_enabled;
	void *packet;
	QDF_STATUS qdf_status;
	uint16_t add_ie_len;
	uint8_t *add_ie;
	uint8_t *wps_ie = NULL;
	uint8_t power_caps = false;
	uint8_t tx_flag = 0;
	uint8_t sme_sessionid = 0;
	bool vht_enabled = false;
	tDot11fIEExtCap extr_ext_cap;
	bool extr_ext_flag = true;
	tpSirMacMgmtHdr mac_hdr;

	if (NULL == pe_session) {
		lim_log(mac_ctx, LOGE, FL("pe_session is NULL"));
		return;
	}

	sme_sessionid = pe_session->smeSessionId;

	/* check this early to avoid unncessary operation */
	if (NULL == pe_session->pLimJoinReq) {
		lim_log(mac_ctx, LOGE, FL("pe_session->pLimJoinReq is NULL"));
		return;
	}
	add_ie_len = pe_session->pLimJoinReq->addIEAssoc.length;
	add_ie = pe_session->pLimJoinReq->addIEAssoc.addIEdata;

	frm = qdf_mem_malloc(sizeof(tDot11fAssocRequest));
	if (NULL == frm) {
		lim_log(mac_ctx, LOGE, FL("Unable to allocate memory"));
		return;
	}

	qdf_mem_set((uint8_t *) frm, sizeof(tDot11fAssocRequest), 0);

	if (add_ie_len && pe_session->is_ext_caps_present) {
		qdf_mem_set((uint8_t *) &extr_ext_cap, sizeof(tDot11fIEExtCap),
			    0);
		sir_status = lim_strip_extcap_update_struct(mac_ctx,
					add_ie, &add_ie_len, &extr_ext_cap);
		if (eSIR_SUCCESS != sir_status) {
			extr_ext_flag = false;
			lim_log(mac_ctx, LOG1,
			    FL("Unable to Stripoff ExtCap IE from Assoc Req"));
		} else {
			struct s_ext_cap *p_ext_cap = (struct s_ext_cap *)
							extr_ext_cap.bytes;

			if (p_ext_cap->interworking_service)
				p_ext_cap->qos_map = 1;
			extr_ext_flag = lim_is_ext_cap_ie_present(p_ext_cap);
		}
	} else {
		lim_log(mac_ctx, LOG1,
		FL("No addn IE or peer dosen't support addnIE for Assoc Req"));
		extr_ext_flag = false;
	}

	caps = mlm_assoc_req->capabilityInfo;
#if defined(FEATURE_WLAN_WAPI)
	/*
	 * According to WAPI standard:
	 * 7.3.1.4 Capability Information field
	 * In WAPI, non-AP STAs within an ESS set the Privacy subfield to 0
	 * in transmitted Association or Reassociation management frames.
	 * APs ignore the Privacy subfield within received Association and
	 * Reassociation management frames.
	 */
	if (pe_session->encryptType == eSIR_ED_WPI)
		((tSirMacCapabilityInfo *) &caps)->privacy = 0;
#endif
	swap_bit_field16(caps, (uint16_t *) &frm->Capabilities);

	frm->ListenInterval.interval = mlm_assoc_req->listenInterval;
	populate_dot11f_ssid2(mac_ctx, &frm->SSID);
	populate_dot11f_supp_rates(mac_ctx, POPULATE_DOT11F_RATES_OPERATIONAL,
		&frm->SuppRates, pe_session);

	qos_enabled = (pe_session->limQosEnabled) &&
		      SIR_MAC_GET_QOS(pe_session->limCurrentBssCaps);

	wme_enabled = (pe_session->limWmeEnabled) &&
		      LIM_BSS_CAPS_GET(WME, pe_session->limCurrentBssQosCaps);

	/* We prefer .11e asociations: */
	if (qos_enabled)
		wme_enabled = false;

	wsm_enabled = (pe_session->limWsmEnabled) && wme_enabled &&
		      LIM_BSS_CAPS_GET(WSM, pe_session->limCurrentBssQosCaps);

	if (pe_session->lim11hEnable &&
	    pe_session->pLimJoinReq->spectrumMgtIndicator == eSIR_TRUE) {
		power_caps = true;

		populate_dot11f_power_caps(mac_ctx, &frm->PowerCaps,
			LIM_ASSOC, pe_session);
		populate_dot11f_supp_channels(mac_ctx, &frm->SuppChannels,
			LIM_ASSOC, pe_session);

	}
	if (mac_ctx->rrm.rrmPEContext.rrmEnable &&
	    SIR_MAC_GET_RRM(pe_session->limCurrentBssCaps)) {
		if (power_caps == false) {
			power_caps = true;
			populate_dot11f_power_caps(mac_ctx, &frm->PowerCaps,
				LIM_ASSOC, pe_session);
		}
	}
	if (qos_enabled)
		populate_dot11f_qos_caps_station(mac_ctx, &frm->QOSCapsStation);

	populate_dot11f_ext_supp_rates(mac_ctx,
		POPULATE_DOT11F_RATES_OPERATIONAL, &frm->ExtSuppRates,
		pe_session);

	if (mac_ctx->rrm.rrmPEContext.rrmEnable &&
	    SIR_MAC_GET_RRM(pe_session->limCurrentBssCaps))
		populate_dot11f_rrm_ie(mac_ctx, &frm->RRMEnabledCap,
			pe_session);

	/*
	 * The join request *should* contain zero or one of the WPA and RSN
	 * IEs.  The payload send along with the request is a
	 * 'tSirSmeJoinReq'; the IE portion is held inside a 'tSirRSNie':
	 *     typedef struct sSirRSNie
	 *     {
	 *         uint16_t       length;
	 *         uint8_t        rsnIEdata[SIR_MAC_MAX_IE_LENGTH+2];
	 *     } tSirRSNie, *tpSirRSNie;
	 * So, we should be able to make the following two calls harmlessly,
	 * since they do nothing if they don't find the given IE in the
	 * bytestream with which they're provided.
	 * The net effect of this will be to faithfully transmit whatever
	 * security IE is in the join request.
	 * However, if we're associating for the purpose of WPS
	 * enrollment, and we've been configured to indicate that by
	 * eliding the WPA or RSN IE, we just skip this:
	 */
	if (add_ie_len && add_ie)
		wps_ie = limGetWscIEPtr(mac_ctx, add_ie, add_ie_len);

	if (NULL == wps_ie) {
		populate_dot11f_rsn_opaque(mac_ctx,
			&(pe_session->pLimJoinReq->rsnIE),
			&frm->RSNOpaque);
		populate_dot11f_wpa_opaque(mac_ctx,
			&(pe_session->pLimJoinReq->rsnIE),
			&frm->WPAOpaque);
#if defined(FEATURE_WLAN_WAPI)
		populate_dot11f_wapi_opaque(mac_ctx,
			&(pe_session->pLimJoinReq->rsnIE),
			&frm->WAPIOpaque);
#endif /* defined(FEATURE_WLAN_WAPI) */
	}
	/* include WME EDCA IE as well */
	if (wme_enabled) {
		populate_dot11f_wmm_info_station_per_session(mac_ctx,
			pe_session, &frm->WMMInfoStation);

		if (wsm_enabled)
			populate_dot11f_wmm_caps(&frm->WMMCaps);
	}

	/*
	 * Populate HT IEs, when operating in 11n and
	 * when AP is also operating in 11n mode
	 */
	if (pe_session->htCapability &&
	    mac_ctx->lim.htCapabilityPresentInBeacon) {
		lim_log(mac_ctx, LOG1, FL("Populate HT Caps in Assoc Request"));
		populate_dot11f_ht_caps(mac_ctx, pe_session, &frm->HTCaps);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
				   &frm->HTCaps, sizeof(frm->HTCaps));
	}
	lim_log(mac_ctx, LOG1,
		FL("SupportedChnlWidth: %d, mimoPS: %d, GF: %d, short GI20:%d, shortGI40: %d, dsssCck: %d, AMPDU Param: %x"),
		frm->HTCaps.supportedChannelWidthSet,
		frm->HTCaps.mimoPowerSave,
		frm->HTCaps.greenField, frm->HTCaps.shortGI20MHz,
		frm->HTCaps.shortGI40MHz,
		frm->HTCaps.dsssCckMode40MHz,
		frm->HTCaps.maxRxAMPDUFactor);

	if (pe_session->vhtCapability &&
	    pe_session->vhtCapabilityPresentInBeacon) {
		lim_log(mac_ctx, LOG1, FL("Populate VHT IEs in Assoc Request"));
		populate_dot11f_vht_caps(mac_ctx, pe_session, &frm->VHTCaps);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_DEBUG,
				   &frm->VHTCaps, sizeof(frm->VHTCaps));
		vht_enabled = true;
		if (pe_session->enableHtSmps &&
				!pe_session->supported_nss_1x1) {
			lim_log(mac_ctx, LOGE, FL(
					"VHT OP mode IE in Assoc Req"));
			populate_dot11f_operating_mode(mac_ctx,
					&frm->OperatingMode, pe_session);
		}
	}
	if (!vht_enabled &&
			pe_session->is_vendor_specific_vhtcaps) {
		lim_log(mac_ctx, LOG1,
			FL("Populate Vendor VHT IEs in Assoc Request"));
		frm->vendor2_ie.present = 1;
		frm->vendor2_ie.type =
			pe_session->vendor_specific_vht_ie_type;
		frm->vendor2_ie.sub_type =
			pe_session->vendor_specific_vht_ie_sub_type;

		frm->vendor2_ie.VHTCaps.present = 1;
		populate_dot11f_vht_caps(mac_ctx, pe_session,
				&frm->vendor2_ie.VHTCaps);
		vht_enabled = true;
	}
	if (pe_session->is_ext_caps_present)
		populate_dot11f_ext_cap(mac_ctx, vht_enabled,
				&frm->ExtCap, pe_session);

	if (pe_session->pLimJoinReq->is11Rconnection) {
		tSirBssDescription *bssdescr;

		bssdescr = &pe_session->pLimJoinReq->bssDescription;
		lim_log(mac_ctx, LOG1, FL("mdie = %02x %02x %02x"),
			(unsigned int) bssdescr->mdie[0],
			(unsigned int) bssdescr->mdie[1],
			(unsigned int) bssdescr->mdie[2]);
		populate_mdie(mac_ctx, &frm->MobilityDomain,
			pe_session->pLimJoinReq->bssDescription.mdie);
	} else {
		/* No 11r IEs dont send any MDIE */
		lim_log(mac_ctx, LOG1, FL("MDIE not present"));
	}

#ifdef FEATURE_WLAN_ESE
	/*
	 * ESE Version IE will be included in association request
	 * when ESE is enabled on DUT through ini and it is also
	 * advertised by the peer AP to which we are trying to
	 * associate to.
	 */
	if (pe_session->is_ese_version_ie_present &&
		mac_ctx->roam.configParam.isEseIniFeatureEnabled)
		populate_dot11f_ese_version(&frm->ESEVersion);
	/* For ESE Associations fill the ESE IEs */
	if (pe_session->isESEconnection &&
	    pe_session->pLimJoinReq->isESEFeatureIniEnabled) {
#ifndef FEATURE_DISABLE_RM
		populate_dot11f_ese_rad_mgmt_cap(&frm->ESERadMgmtCap);
#endif
	}
#endif

	status = dot11f_get_packed_assoc_request_size(mac_ctx, frm, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGP,
			FL("Association Request packet size failure(0x%08x)."),
			status);
		/* We'll fall back on the worst case scenario: */
		payload = sizeof(tDot11fAssocRequest);
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("Association request packet size warning (0x%08x)."),
			status);
	}

	bytes = payload + sizeof(tSirMacMgmtHdr) + add_ie_len;

	qdf_status = cds_packet_alloc((uint16_t) bytes, (void **)&frame,
				(void **)&packet);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGP, FL("Failed to allocate %d bytes."),
			bytes);

		pe_session->limMlmState = pe_session->limPrevMlmState;
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_MLM_STATE,
				 pe_session->peSessionId,
				 pe_session->limMlmState));

		/* Update PE session id */
		assoc_cnf.sessionId = pe_session->peSessionId;

		assoc_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;

		cds_packet_free((void *)packet);

		lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
			(uint32_t *) &assoc_cnf);

		qdf_mem_free(frm);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(frame, bytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(mac_ctx, frame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ASSOC_REQ, pe_session->bssId,
		pe_session->selfMacAddr);
	/* merge the ExtCap struct */
	if (extr_ext_flag)
		lim_merge_extcap_struct(&frm->ExtCap, &extr_ext_cap);
	/* That done, pack the Assoc Request: */
	status = dot11f_pack_assoc_request(mac_ctx, frm,
			frame + sizeof(tSirMacMgmtHdr), payload, &payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			FL("Assoc request pack failure (0x%08x)"), status);
		cds_packet_free((void *)packet);
		qdf_mem_free(frm);
		return;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
			FL("Assoc request pack warning (0x%08x)"), status);
	}

	lim_log(mac_ctx, LOG1,
		FL("*** Sending Association Request length %d to "), bytes);
	if (pe_session->assocReq != NULL) {
		qdf_mem_free(pe_session->assocReq);
		pe_session->assocReq = NULL;
		pe_session->assocReqLen = 0;
	}

	if (add_ie_len) {
		qdf_mem_copy(frame + sizeof(tSirMacMgmtHdr) + payload,
			     add_ie, add_ie_len);
		payload += add_ie_len;
	}

	pe_session->assocReq = qdf_mem_malloc(payload);
	if (NULL == pe_session->assocReq) {
		lim_log(mac_ctx, LOGE, FL("Unable to allocate memory"));
	} else {
		/*
		 * Store the Assoc request. This is sent to csr/hdd in
		 * join cnf response.
		 */
		qdf_mem_copy(pe_session->assocReq,
			     frame + sizeof(tSirMacMgmtHdr), payload);
		pe_session->assocReqLen = payload;
	}

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(pe_session->currentOperChannel))
	    || (pe_session->pePersona == QDF_P2P_CLIENT_MODE)
	    || (pe_session->pePersona == QDF_P2P_GO_MODE)
	    )
		tx_flag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;

	if (pe_session->pePersona == QDF_P2P_CLIENT_MODE ||
		pe_session->pePersona == QDF_STA_MODE)
		tx_flag |= HAL_USE_PEER_STA_REQUESTED_MASK;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_ASSOC_START_EVENT,
			      pe_session, eSIR_SUCCESS, eSIR_SUCCESS);
#endif
	mac_hdr = (tpSirMacMgmtHdr) frame;
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 pe_session->peSessionId, mac_hdr->fc.subType));
	qdf_status =
		wma_tx_frame(mac_ctx, packet,
			   (uint16_t) (sizeof(tSirMacMgmtHdr) + payload),
			   TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS, 7,
			   lim_tx_complete, frame, tx_flag, sme_sessionid, 0);
	MTRACE(qdf_trace
		       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
		       pe_session->peSessionId, qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGE,
			FL("Failed to send Association Request (%X)!"),
			qdf_status);
		/* Pkt will be freed up by the callback */
		qdf_mem_free(frm);
		return;
	}
	/* Free up buffer allocated for mlm_assoc_req */
	qdf_mem_free(mlm_assoc_req);
	mlm_assoc_req = NULL;
	qdf_mem_free(frm);
	return;
}

/**
 * lim_auth_tx_complete_cnf()- Confirmation for auth sent over the air
 *
 * @mac_ctx: pointer to global mac
 * @tx_complete : Sent status
 *
 * Return: This returns QDF_STATUS
 */

QDF_STATUS lim_auth_tx_complete_cnf(tpAniSirGlobal mac_ctx,
					uint32_t tx_complete)
{
	lim_log(mac_ctx, LOG1,
		 FL("tx_complete= %d"), tx_complete);
	if (tx_complete) {
		mac_ctx->auth_ack_status = LIM_AUTH_ACK_RCD_SUCCESS;
		/* 'Change' timer for future activations */
		lim_deactivate_and_change_timer(mac_ctx, eLIM_AUTH_RETRY_TIMER);
	} else {
		mac_ctx->auth_ack_status = LIM_AUTH_ACK_RCD_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_send_auth_mgmt_frame() - Send an Authentication frame
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @auth_frame: Pointer to Authentication frame structure
 * @peer_addr: MAC address of destination peer
 * @wep_bit: wep bit in frame control for Authentication frame3
 * @session: PE session information
 *
 * This function is called by lim_process_mlm_messages(). Authentication frame
 * is formatted and sent when this function is called.
 *
 * Return: void
 */

void
lim_send_auth_mgmt_frame(tpAniSirGlobal mac_ctx,
			 tpSirMacAuthFrameBody auth_frame,
			 tSirMacAddr peer_addr,
			 uint8_t wep_bit,
			 tpPESession session, bool wait_for_ack)
{
	uint8_t *frame, *body;
	uint32_t frame_len = 0, body_len = 0;
	tpSirMacMgmtHdr mac_hdr;
	void *packet;
	QDF_STATUS qdf_status;
	uint8_t tx_flag = 0;
	uint8_t sme_sessionid = 0;
	uint16_t ft_ies_length = 0;

	if (NULL == session) {
		lim_log(mac_ctx, LOGE, FL("Error: psession Entry is NULL"));
		return;
	}

	sme_sessionid = session->smeSessionId;

	if (wep_bit == LIM_WEP_IN_FC) {
		/*
		 * Auth frame3 to be sent with encrypted framebody
		 *
		 * Allocate buffer for Authenticaton frame of size
		 * equal to management frame header length plus 2 bytes
		 * each for auth algorithm number, transaction number,
		 * status code, 128 bytes for challenge text and
		 * 4 bytes each for IV & ICV.
		 */
		lim_log(mac_ctx, LOG1,
			FL("Sending encrypted auth frame to " MAC_ADDRESS_STR
			   "- wait_for_ack %d"), MAC_ADDR_ARRAY(peer_addr),
			   wait_for_ack);

		frame_len = sizeof(tSirMacMgmtHdr) + LIM_ENCR_AUTH_BODY_LEN;
		body_len = LIM_ENCR_AUTH_BODY_LEN;

		goto alloc_packet;
	}

	lim_log(mac_ctx, LOG1,
		FL("Sending Auth seq# %d status %d (%d) wait_for_ack %d to "
		MAC_ADDRESS_STR),
		auth_frame->authTransactionSeqNumber,
		auth_frame->authStatusCode,
		(auth_frame->authStatusCode == eSIR_MAC_SUCCESS_STATUS),
		wait_for_ack,
		MAC_ADDR_ARRAY(peer_addr));

	switch (auth_frame->authTransactionSeqNumber) {
	case SIR_MAC_AUTH_FRAME_1:
		/*
		 * Allocate buffer for Authenticaton frame of size
		 * equal to management frame header length plus 2 bytes
		 * each for auth algorithm number, transaction number
		 * and status code.
		 */

		frame_len = sizeof(tSirMacMgmtHdr) +
			   SIR_MAC_AUTH_CHALLENGE_OFFSET;
		body_len = SIR_MAC_AUTH_CHALLENGE_OFFSET;

		if (auth_frame->authAlgoNumber == eSIR_FT_AUTH) {
			if (NULL != session->ftPEContext.pFTPreAuthReq &&
			    0 != session->ftPEContext.pFTPreAuthReq->
				ft_ies_length) {
				ft_ies_length = session->ftPEContext.
					pFTPreAuthReq->ft_ies_length;
				frame_len += ft_ies_length;
				lim_log(mac_ctx, LOG3,
					FL("Auth frame, FTIES length added=%d"),
					ft_ies_length);
			} else {
				lim_log(mac_ctx, LOG3,
					FL("Auth frame, Does not contain FTIES!!!"));
				frame_len += (2 + SIR_MDIE_SIZE);
			}
		}
		break;

	case SIR_MAC_AUTH_FRAME_2:
		if ((auth_frame->authAlgoNumber == eSIR_OPEN_SYSTEM) ||
		    ((auth_frame->authAlgoNumber == eSIR_SHARED_KEY) &&
			(auth_frame->authStatusCode !=
			 eSIR_MAC_SUCCESS_STATUS))) {
			/*
			 * Allocate buffer for Authenticaton frame of size
			 * equal to management frame header length plus
			 * 2 bytes each for auth algorithm number,
			 * transaction number and status code.
			 */

			frame_len = sizeof(tSirMacMgmtHdr) +
				   SIR_MAC_AUTH_CHALLENGE_OFFSET;
			body_len = SIR_MAC_AUTH_CHALLENGE_OFFSET;
		} else {
			/*
			 * Shared Key algorithm with challenge text
			 * to be sent.
			 *
			 * Allocate buffer for Authenticaton frame of size
			 * equal to management frame header length plus
			 * 2 bytes each for auth algorithm number,
			 * transaction number, status code and 128 bytes
			 * for challenge text.
			 */

			frame_len = sizeof(tSirMacMgmtHdr) +
				   sizeof(tSirMacAuthFrame);
			body_len = sizeof(tSirMacAuthFrameBody);
		}
		break;

	case SIR_MAC_AUTH_FRAME_3:
		/*
		 * Auth frame3 to be sent without encrypted framebody
		 *
		 * Allocate buffer for Authenticaton frame of size equal
		 * to management frame header length plus 2 bytes each
		 * for auth algorithm number, transaction number and
		 * status code.
		 */

		frame_len = sizeof(tSirMacMgmtHdr) +
			   SIR_MAC_AUTH_CHALLENGE_OFFSET;
		body_len = SIR_MAC_AUTH_CHALLENGE_OFFSET;
		break;

	case SIR_MAC_AUTH_FRAME_4:
		/*
		 * Allocate buffer for Authenticaton frame of size equal
		 * to management frame header length plus 2 bytes each
		 * for auth algorithm number, transaction number and
		 * status code.
		 */

		frame_len = sizeof(tSirMacMgmtHdr) +
			   SIR_MAC_AUTH_CHALLENGE_OFFSET;
		body_len = SIR_MAC_AUTH_CHALLENGE_OFFSET;

		break;
	default:
		lim_log(mac_ctx, LOGE, FL("Invalid auth transaction seq num."));
		return;
	} /* switch (auth_frame->authTransactionSeqNumber) */

alloc_packet:
	qdf_status = cds_packet_alloc((uint16_t) frame_len, (void **)&frame,
				 (void **)&packet);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGP,
			FL("call to bufAlloc failed for AUTH frame"));
		return;
	}

	qdf_mem_zero(frame, frame_len);

	/* Prepare BD */
	lim_populate_mac_header(mac_ctx, frame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_AUTH, peer_addr, session->selfMacAddr);
	mac_hdr = (tpSirMacMgmtHdr) frame;
	mac_hdr->fc.wep = wep_bit;

	/* Prepare BSSId */
	if (LIM_IS_AP_ROLE(session))
		qdf_mem_copy((uint8_t *) mac_hdr->bssId,
			     (uint8_t *) session->bssId,
			     sizeof(tSirMacAddr));

	/* Prepare Authentication frame body */
	body = frame + sizeof(tSirMacMgmtHdr);

	if (wep_bit == LIM_WEP_IN_FC) {
		qdf_mem_copy(body, (uint8_t *) auth_frame, body_len);

		lim_log(mac_ctx, LOG1,
			FL("Sending Auth seq# 3 to " MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(mac_hdr->da));

	} else {
		*((uint16_t *) (body)) =
			sir_swap_u16if_needed(auth_frame->authAlgoNumber);
		body += sizeof(uint16_t);
		body_len -= sizeof(uint16_t);

		*((uint16_t *) (body)) =
			sir_swap_u16if_needed(
				auth_frame->authTransactionSeqNumber);
		body += sizeof(uint16_t);
		body_len -= sizeof(uint16_t);

		*((uint16_t *) (body)) =
			sir_swap_u16if_needed(auth_frame->authStatusCode);
		body += sizeof(uint16_t);
		body_len -= sizeof(uint16_t);
		if (body_len <= (sizeof(auth_frame->type) +
				sizeof(auth_frame->length) +
				sizeof(auth_frame->challengeText)))
			qdf_mem_copy(body, (uint8_t *) &auth_frame->type,
				     body_len);

		if ((auth_frame->authAlgoNumber == eSIR_FT_AUTH) &&
		    (auth_frame->authTransactionSeqNumber ==
		     SIR_MAC_AUTH_FRAME_1) &&
		     (session->ftPEContext.pFTPreAuthReq != NULL)) {

			if (ft_ies_length > 0) {
				qdf_mem_copy(body,
					session->ftPEContext.
						pFTPreAuthReq->ft_ies,
					ft_ies_length);
				lim_log(mac_ctx, LOG2,
					FL("Auth1 Frame FTIE is: "));
				sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG2,
					(uint8_t *) body,
					ft_ies_length);
			} else if (NULL != session->ftPEContext.
					pFTPreAuthReq->pbssDescription) {
				/* MDID attr is 54 */
				*body = SIR_MDIE_ELEMENT_ID;
				body++;
				*body = SIR_MDIE_SIZE;
				body++;
				qdf_mem_copy(body,
					&session->ftPEContext.pFTPreAuthReq->
						pbssDescription->mdie[0],
					SIR_MDIE_SIZE);
			}
		}

		lim_log(mac_ctx, LOG1,
			FL("*** Sending Auth seq# %d status %d (%d) to "
				MAC_ADDRESS_STR),
			auth_frame->authTransactionSeqNumber,
			auth_frame->authStatusCode,
			(auth_frame->authStatusCode ==
				eSIR_MAC_SUCCESS_STATUS),
			MAC_ADDR_ARRAY(mac_hdr->da));
	}
	sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG2, frame, frame_len);

	if ((NULL != session->ftPEContext.pFTPreAuthReq) &&
	    (SIR_BAND_5_GHZ == lim_get_rf_band(
	     session->ftPEContext.pFTPreAuthReq->preAuthchannelNum)))
		tx_flag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	else if ((SIR_BAND_5_GHZ ==
		  lim_get_rf_band(session->currentOperChannel))
		  || (session->pePersona == QDF_P2P_CLIENT_MODE)
		  || (session->pePersona == QDF_P2P_GO_MODE))
		tx_flag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;

	if (session->pePersona == QDF_P2P_CLIENT_MODE ||
		session->pePersona == QDF_STA_MODE)
		tx_flag |= HAL_USE_PEER_STA_REQUESTED_MASK;

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 session->peSessionId, mac_hdr->fc.subType));

	if (wait_for_ack) {
		mac_ctx->auth_ack_status = LIM_AUTH_ACK_NOT_RCD;
		qdf_status = wma_tx_frameWithTxComplete(mac_ctx, packet,
					 (uint16_t)frame_len,
					 TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS,
					 7, lim_tx_complete, frame,
					 lim_auth_tx_complete_cnf,
					 tx_flag, sme_sessionid, false, 0);
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			session->peSessionId, qdf_status));
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			lim_log(mac_ctx, LOGE,
			  FL("*** Could not send Auth frame, retCode=%X ***"),
			  qdf_status);
			mac_ctx->auth_ack_status = LIM_AUTH_ACK_RCD_FAILURE;
		/* Pkt will be freed up by the callback */
		}
	} else {
		/* Queue Authentication frame in high priority WQ */
		qdf_status = wma_tx_frame(mac_ctx, packet, (uint16_t) frame_len,
					TXRX_FRM_802_11_MGMT,
					ANI_TXDIR_TODS, 7, lim_tx_complete,
					frame, tx_flag, sme_sessionid, 0);
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
				 session->peSessionId, qdf_status));
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			lim_log(mac_ctx, LOGE,
			  FL("*** Could not send Auth frame, retCode=%X ***"),
			  qdf_status);
	}
	return;
}

QDF_STATUS lim_send_deauth_cnf(tpAniSirGlobal pMac)
{
	uint16_t aid;
	tpDphHashNode pStaDs;
	tLimMlmDeauthReq *pMlmDeauthReq;
	tLimMlmDeauthCnf mlmDeauthCnf;
	tpPESession psessionEntry;

	pMlmDeauthReq = pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq;
	if (pMlmDeauthReq) {
		if (tx_timer_running(&pMac->lim.limTimers.gLimDeauthAckTimer)) {
			lim_deactivate_and_change_timer(pMac,
							eLIM_DEAUTH_ACK_TIMER);
		}

		psessionEntry = pe_find_session_by_session_id(pMac,
				pMlmDeauthReq->sessionId);
		if (psessionEntry == NULL) {
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("session does not exist for given sessionId"));
			       )
			mlmDeauthCnf.resultCode =
				eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}

		pStaDs =
			dph_lookup_hash_entry(pMac,
					      pMlmDeauthReq->peer_macaddr.bytes,
					      &aid,
					      &psessionEntry->dph.dphHashTable);
		if (pStaDs == NULL) {
			mlmDeauthCnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}

		/* / Receive path cleanup with dummy packet */
		lim_ft_cleanup_pre_auth_info(pMac, psessionEntry);
		lim_cleanup_rx_path(pMac, pStaDs, psessionEntry);
		if ((psessionEntry->limSystemRole == eLIM_STA_ROLE) &&
				(
#ifdef FEATURE_WLAN_ESE
				 (psessionEntry->isESEconnection) ||
#endif
				 (psessionEntry->isFastRoamIniFeatureEnabled) ||
				 (psessionEntry->is11Rconnection))) {
			lim_log(pMac, LOGE,
				FL("FT Preauth (%p,%d) Deauth rc %d src = %d"),
					psessionEntry,
					psessionEntry->peSessionId,
					pMlmDeauthReq->reasonCode,
					pMlmDeauthReq->deauthTrigger);
			lim_ft_cleanup(pMac, psessionEntry);
		} else {
			lim_log(pMac, LOGE,
				FL("No FT Preauth Session Cleanup in role %d"
#ifdef FEATURE_WLAN_ESE
				" isESE %d"
#endif
				" isLFR %d"
				" is11r %d, Deauth reason %d Trigger = %d"),
				psessionEntry->limSystemRole,
#ifdef FEATURE_WLAN_ESE
				psessionEntry->isESEconnection,
#endif
				psessionEntry->isFastRoamIniFeatureEnabled,
				psessionEntry->is11Rconnection,
				pMlmDeauthReq->reasonCode,
				pMlmDeauthReq->deauthTrigger);
		}
		/* Free up buffer allocated for mlmDeauthReq */
		qdf_mem_free(pMlmDeauthReq);
		pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq = NULL;
	}
	return QDF_STATUS_SUCCESS;
end:
	qdf_copy_macaddr(&mlmDeauthCnf.peer_macaddr,
			 &pMlmDeauthReq->peer_macaddr);
	mlmDeauthCnf.deauthTrigger = pMlmDeauthReq->deauthTrigger;
	mlmDeauthCnf.aid = pMlmDeauthReq->aid;
	mlmDeauthCnf.sessionId = pMlmDeauthReq->sessionId;

	/* Free up buffer allocated */
	/* for mlmDeauthReq */
	qdf_mem_free(pMlmDeauthReq);

	lim_post_sme_message(pMac,
			     LIM_MLM_DEAUTH_CNF, (uint32_t *) &mlmDeauthCnf);
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_send_disassoc_cnf() - Send disassoc confirmation to SME
 *
 * @mac_ctx: Handle to MAC context
 *
 * Sends disassoc confirmation to SME. Removes disassoc request stored
 * in lim.
 *
 * Return: QDF_STATUS_SUCCESS
 */

QDF_STATUS lim_send_disassoc_cnf(tpAniSirGlobal mac_ctx)
{
	uint16_t aid;
	tpDphHashNode sta_ds;
	tLimMlmDisassocCnf disassoc_cnf;
	tpPESession pe_session;
	tLimMlmDisassocReq *disassoc_req;

	disassoc_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq;
	if (disassoc_req) {
		if (tx_timer_running(
			&mac_ctx->lim.limTimers.gLimDisassocAckTimer))
			lim_deactivate_and_change_timer(mac_ctx,
				eLIM_DISASSOC_ACK_TIMER);

		pe_session = pe_find_session_by_session_id(
					mac_ctx, disassoc_req->sessionId);
		if (pe_session == NULL) {
			lim_log(mac_ctx, LOGE,
				       FL("No session for given sessionId"));
			disassoc_cnf.resultCode =
				eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}

		sta_ds = dph_lookup_hash_entry(mac_ctx,
				disassoc_req->peer_macaddr.bytes, &aid,
				&pe_session->dph.dphHashTable);
		if (sta_ds == NULL) {
			lim_log(mac_ctx, LOGE, FL("StaDs Null"));
			disassoc_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}
		/* Receive path cleanup with dummy packet */
		if (eSIR_SUCCESS !=
		    lim_cleanup_rx_path(mac_ctx, sta_ds, pe_session)) {
			disassoc_cnf.resultCode =
				eSIR_SME_RESOURCES_UNAVAILABLE;
			lim_log(mac_ctx, LOGE, FL("cleanup_rx_path error"));
			goto end;
		}
		if (LIM_IS_STA_ROLE(pe_session) && (
#ifdef FEATURE_WLAN_ESE
			    (pe_session->isESEconnection) ||
#endif
			    (pe_session->isFastRoamIniFeatureEnabled) ||
			    (pe_session->is11Rconnection)) &&
			    (disassoc_req->reasonCode !=
				eSIR_MAC_DISASSOC_DUE_TO_FTHANDOFF_REASON)) {
			lim_log(mac_ctx, LOG1,
				FL("FT Preauth Session (%p,%d) Clean up"),
				pe_session, pe_session->peSessionId);

			/* Delete FT session if there exists one */
			lim_ft_cleanup_pre_auth_info(mac_ctx, pe_session);
		} else {
			lim_log(mac_ctx, LOGE,
				FL("No FT Preauth Session Clean up in role %d"
#ifdef FEATURE_WLAN_ESE
				" isESE %d"
#endif
				" isLFR %d"
				" is11r %d reason %d"),
				GET_LIM_SYSTEM_ROLE(pe_session),
#ifdef FEATURE_WLAN_ESE
				pe_session->isESEconnection,
#endif
				pe_session->isFastRoamIniFeatureEnabled,
				pe_session->is11Rconnection,
				disassoc_req->reasonCode);
		}
		/* Free up buffer allocated for mlmDisassocReq */
		qdf_mem_free(disassoc_req);
		mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq = NULL;
		return QDF_STATUS_SUCCESS;
	} else {
		return QDF_STATUS_SUCCESS;
	}
end:
	qdf_mem_copy((uint8_t *) &disassoc_cnf.peerMacAddr,
		     (uint8_t *) disassoc_req->peer_macaddr.bytes,
		     QDF_MAC_ADDR_SIZE);
	disassoc_cnf.aid = disassoc_req->aid;
	disassoc_cnf.disassocTrigger = disassoc_req->disassocTrigger;

	/* Update PE session ID */
	disassoc_cnf.sessionId = disassoc_req->sessionId;

	if (disassoc_req != NULL) {
		/* / Free up buffer allocated for mlmDisassocReq */
		qdf_mem_free(disassoc_req);
		mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq = NULL;
	}

	lim_post_sme_message(mac_ctx, LIM_MLM_DISASSOC_CNF,
		(uint32_t *) &disassoc_cnf);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS lim_disassoc_tx_complete_cnf(tpAniSirGlobal pMac,
					uint32_t txCompleteSuccess)
{
	lim_log(pMac, LOG1,
		FL("txCompleteSuccess: %d"), txCompleteSuccess);
	return lim_send_disassoc_cnf(pMac);
}

QDF_STATUS lim_deauth_tx_complete_cnf(tpAniSirGlobal pMac,
				      uint32_t txCompleteSuccess)
{
	lim_log(pMac, LOG1,
		FL("txCompleteSuccess: %d"), txCompleteSuccess);
	return lim_send_deauth_cnf(pMac);
}

/**
 * \brief This function is called to send Disassociate frame.
 *
 *
 * \param pMac Pointer to Global MAC structure
 *
 * \param nReason Indicates the reason that need to be sent in
 * Disassociation frame
 *
 * \param peerMacAddr MAC address of the STA to which Disassociation frame is
 * sent
 *
 *
 */

void
lim_send_disassoc_mgmt_frame(tpAniSirGlobal pMac,
			     uint16_t nReason,
			     tSirMacAddr peer,
			     tpPESession psessionEntry, bool waitForAck)
{
	tDot11fDisassociation frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint32_t val = 0;
	uint8_t smeSessionId = 0;
	if (NULL == psessionEntry) {
		return;
	}

	/*
	 * In case when cac timer is running for this SAP session then
	 * avoid sending disassoc out. It is violation of dfs specification.
	 */
	if (((psessionEntry->pePersona == QDF_SAP_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)) &&
	    (true == pMac->sap.SapDfsInfo.is_dfs_cac_timer_running)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL
				  ("CAC timer is running, drop disassoc from going out"));
		return;
	}
	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Reason.code = nReason;

	nStatus = dot11f_get_packed_disassociation_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Disassociation (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fDisassociation);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Disassociation "
				       "(0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				      (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a Dis"
				       "association."), nBytes);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_DISASSOC, peer, psessionEntry->selfMacAddr);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	/* Prepare the BSSID */
	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	nStatus = dot11f_pack_disassociation(pMac, &frm, pFrame +
					     sizeof(tSirMacMgmtHdr),
					     nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack a Disassociation (0x%08x)."),
			nStatus);
		cds_packet_free((void *)pPacket);
		return;         /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while packing a D"
				       "isassociation (0x%08x)."), nStatus);
	}

	lim_log(pMac, LOG1,
		FL("***Sessionid %d Sending Disassociation frame with "
		   "reason %u and waitForAck %d to " MAC_ADDRESS_STR " ,From "
		   MAC_ADDRESS_STR), psessionEntry->peSessionId, nReason,
		waitForAck, MAC_ADDR_ARRAY(pMacHdr->da),
		MAC_ADDR_ARRAY(psessionEntry->selfMacAddr));

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	txFlag |= HAL_USE_PEER_STA_REQUESTED_MASK;

	if (waitForAck) {
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
				 psessionEntry->peSessionId,
				 pMacHdr->fc.subType));
		/* Queue Disassociation frame in high priority WQ */
		/* get the duration from the request */
		qdf_status =
			wma_tx_frameWithTxComplete(pMac, pPacket, (uint16_t) nBytes,
					 TXRX_FRM_802_11_MGMT,
					 ANI_TXDIR_TODS, 7, lim_tx_complete,
					 pFrame, lim_disassoc_tx_complete_cnf,
					 txFlag, smeSessionId, false, 0);
		MTRACE(qdf_trace
			       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			       psessionEntry->peSessionId, qdf_status));

		val = SYS_MS_TO_TICKS(LIM_DISASSOC_DEAUTH_ACK_TIMEOUT);

		if (tx_timer_change
			    (&pMac->lim.limTimers.gLimDisassocAckTimer, val, 0)
		    != TX_SUCCESS) {
			lim_log(pMac, LOGP,
				FL("Unable to change Disassoc ack Timer val"));
			return;
		} else if (TX_SUCCESS !=
			   tx_timer_activate(&pMac->lim.limTimers.
					     gLimDisassocAckTimer)) {
			lim_log(pMac, LOGP,
				FL("Unable to activate Disassoc ack Timer"));
			lim_deactivate_and_change_timer(pMac,
							eLIM_DISASSOC_ACK_TIMER);
			return;
		}
	} else {
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
				 psessionEntry->peSessionId,
				 pMacHdr->fc.subType));
		/* Queue Disassociation frame in high priority WQ */
		qdf_status = wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
					TXRX_FRM_802_11_MGMT,
					ANI_TXDIR_TODS,
					7,
					lim_tx_complete, pFrame, txFlag,
					smeSessionId, 0);
		MTRACE(qdf_trace
			       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			       psessionEntry->peSessionId, qdf_status));
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			lim_log(pMac, LOGE,
				FL("Failed to send Disassociation (%X)!"),
				qdf_status);
			/* Pkt will be freed up by the callback */
		}
	}
} /* End lim_send_disassoc_mgmt_frame. */

/**
 * \brief This function is called to send a Deauthenticate frame
 *
 *
 * \param pMac Pointer to global MAC structure
 *
 * \param nReason Indicates the reason that need to be sent in the
 * Deauthenticate frame
 *
 * \param peeer address of the STA to which the frame is to be sent
 *
 *
 */

void
lim_send_deauth_mgmt_frame(tpAniSirGlobal pMac,
			   uint16_t nReason,
			   tSirMacAddr peer,
			   tpPESession psessionEntry, bool waitForAck)
{
	tDot11fDeAuth frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint32_t val = 0;
#ifdef FEATURE_WLAN_TDLS
	uint16_t aid;
	tpDphHashNode pStaDs;
#endif
	uint8_t smeSessionId = 0;

	if (NULL == psessionEntry) {
		return;
	}

	/*
	 * In case when cac timer is running for this SAP session then
	 * avoid deauth frame out. It is violation of dfs specification.
	 */
	if (((psessionEntry->pePersona == QDF_SAP_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)) &&
	    (true == pMac->sap.SapDfsInfo.is_dfs_cac_timer_running)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL
				  ("CAC timer is running, drop the deauth from going out"));
		return;
	}
	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Reason.code = nReason;

	nStatus = dot11f_get_packed_de_auth_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a De-Authentication (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fDeAuth);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a De-Authentication "
				       "(0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				      (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a De-"
				       "Authentication."), nBytes);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_DEAUTH, peer, psessionEntry->selfMacAddr);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	/* Prepare the BSSID */
	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	nStatus = dot11f_pack_de_auth(pMac, &frm, pFrame +
				      sizeof(tSirMacMgmtHdr), nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack a DeAuthentication (0x%08x)."),
			nStatus);
		cds_packet_free((void *)pPacket);
		return;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while packing a D"
				       "e-Authentication (0x%08x)."), nStatus);
	}
	lim_log(pMac, LOG1, FL("***Sessionid %d Sending Deauth frame with "
			       "reason %u and waitForAck %d to " MAC_ADDRESS_STR
			       " ,From " MAC_ADDRESS_STR),
		psessionEntry->peSessionId, nReason, waitForAck,
		MAC_ADDR_ARRAY(pMacHdr->da),
		MAC_ADDR_ARRAY(psessionEntry->selfMacAddr));

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	txFlag |= HAL_USE_PEER_STA_REQUESTED_MASK;
#ifdef FEATURE_WLAN_TDLS
	pStaDs =
		dph_lookup_hash_entry(pMac, peer, &aid,
				      &psessionEntry->dph.dphHashTable);
#endif

	if (waitForAck) {
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
				 psessionEntry->peSessionId,
				 pMacHdr->fc.subType));
		/* Queue Disassociation frame in high priority WQ */
		qdf_status =
			wma_tx_frameWithTxComplete(pMac, pPacket, (uint16_t) nBytes,
					 TXRX_FRM_802_11_MGMT,
					 ANI_TXDIR_TODS, 7, lim_tx_complete,
					 pFrame, lim_deauth_tx_complete_cnf,
					 txFlag, smeSessionId, false, 0);
		MTRACE(qdf_trace
			       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			       psessionEntry->peSessionId, qdf_status));
		/* Pkt will be freed up by the callback lim_tx_complete */
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			lim_log(pMac, LOGE,
				FL("Failed to send De-Authentication (%X)!"),
				qdf_status);

			/* Call lim_process_deauth_ack_timeout which will send
			 * DeauthCnf for this frame
			 */
			lim_process_deauth_ack_timeout(pMac);
			return;
		}

		val = SYS_MS_TO_TICKS(LIM_DISASSOC_DEAUTH_ACK_TIMEOUT);

		if (tx_timer_change
			    (&pMac->lim.limTimers.gLimDeauthAckTimer, val, 0)
		    != TX_SUCCESS) {
			lim_log(pMac, LOGP,
				FL("Unable to change Deauth ack Timer val"));
			return;
		} else if (TX_SUCCESS !=
			   tx_timer_activate(&pMac->lim.limTimers.
					     gLimDeauthAckTimer)) {
			lim_log(pMac, LOGP,
				FL("Unable to activate Deauth ack Timer"));
			lim_deactivate_and_change_timer(pMac,
							eLIM_DEAUTH_ACK_TIMER);
			return;
		}
	} else {
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
				 psessionEntry->peSessionId,
				 pMacHdr->fc.subType));
#ifdef FEATURE_WLAN_TDLS
		if ((NULL != pStaDs)
		    && (STA_ENTRY_TDLS_PEER == pStaDs->staType)) {
			/* Queue Disassociation frame in high priority WQ */
			qdf_status =
				wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
					   TXRX_FRM_802_11_MGMT, ANI_TXDIR_IBSS,
					   7, lim_tx_complete, pFrame, txFlag,
					   smeSessionId, 0);
		} else {
#endif
		/* Queue Disassociation frame in high priority WQ */
		qdf_status =
			wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
				   TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS,
				   7, lim_tx_complete, pFrame, txFlag,
				   smeSessionId, 0);
#ifdef FEATURE_WLAN_TDLS
	}
#endif
		MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
				 psessionEntry->peSessionId, qdf_status));
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			lim_log(pMac, LOGE,
				FL("Failed to send De-Authentication (%X)!"),
				qdf_status);
			/* Pkt will be freed up by the callback */
		}
	}

} /* End lim_send_deauth_mgmt_frame. */

#ifdef ANI_SUPPORT_11H
/**
 * \brief Send a Measurement Report Action frame
 *
 *
 * \param pMac Pointer to the global MAC structure
 *
 * \param pMeasReqFrame Address of a tSirMacMeasReqActionFrame
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
lim_send_meas_report_frame(tpAniSirGlobal pMac,
			   tpSirMacMeasReqActionFrame pMeasReqFrame,
			   tSirMacAddr peer, tpPESession psessionEntry)
{
	tDot11fMeasurementReport frm;
	uint8_t *pFrame;
	tSirRetStatus nSirStatus;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_SPECTRUM_MGMT;
	frm.Action.action = SIR_MAC_ACTION_MEASURE_REPORT_ID;
	frm.DialogToken.token = pMeasReqFrame->actionHeader.dialogToken;

	switch (pMeasReqFrame->measReqIE.measType) {
	case SIR_MAC_BASIC_MEASUREMENT_TYPE:
		nSirStatus =
			populate_dot11f_measurement_report0(pMac, pMeasReqFrame,
							    &frm.MeasurementReport);
		break;
	case SIR_MAC_CCA_MEASUREMENT_TYPE:
		nSirStatus =
			populate_dot11f_measurement_report1(pMac, pMeasReqFrame,
							    &frm.MeasurementReport);
		break;
	case SIR_MAC_RPI_MEASUREMENT_TYPE:
		nSirStatus =
			populate_dot11f_measurement_report2(pMac, pMeasReqFrame,
							    &frm.MeasurementReport);
		break;
	default:
		lim_log(pMac, LOGE, FL("Unknown measurement type %d in limSen"
				       "dMeasReportFrame."),
			pMeasReqFrame->measReqIE.measType);
		return eSIR_FAILURE;
	}

	if (eSIR_SUCCESS != nSirStatus)
		return eSIR_FAILURE;

	nStatus = dot11f_get_packed_measurement_report_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Measurement Report (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fMeasurementReport);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Measurement Rep"
				       "ort (0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc(pMac->hHdd, TXRX_FRM_802_11_MGMT,
				 (uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a De-"
				       "Authentication."), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	qdf_mem_copy(pMacHdr->bssId, psessionEntry->bssId, sizeof(tSirMacAddr));

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	nStatus = dot11f_pack_measurement_report(pMac, &frm, pFrame +
						 sizeof(tSirMacMgmtHdr),
						 nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack a Measurement Report (0x%08x)."),
			nStatus);
		cds_packet_free(pMac->hHdd, TXRX_FRM_802_11_MGMT,
				(void *)pFrame, (void *)pPacket);
		return eSIR_FAILURE;    /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while packing a M"
				       "easurement Report (0x%08x)."), nStatus);
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 ((psessionEntry) ? psessionEntry->
			  peSessionId : NO_SESSION), pMacHdr->fc.subType));
	qdf_status =
		wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
			   TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS, 7,
			   lim_tx_complete, pFrame, 0, 0);
	MTRACE(qdf_trace
		       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
		       ((psessionEntry) ? psessionEntry->peSessionId : NO_SESSION),
		       qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGE,
			FL("Failed to send a Measurement Report  (%X)!"),
			qdf_status);
		/* Pkt will be freed up by the callback */
		return eSIR_FAILURE;    /* just allocated... */
	}

	return eSIR_SUCCESS;

} /* End lim_send_meas_report_frame. */

/**
 * \brief Send a TPC Request Action frame
 *
 *
 * \param pMac Pointer to the global MAC datastructure
 *
 * \param peer MAC address to which the frame should be sent
 *
 *
 */

void
lim_send_tpc_request_frame(tpAniSirGlobal pMac,
			   tSirMacAddr peer, tpPESession psessionEntry)
{
	tDot11fTPCRequest frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_SPECTRUM_MGMT;
	frm.Action.action = SIR_MAC_ACTION_TPC_REQUEST_ID;
	frm.DialogToken.token = 1;
	frm.TPCRequest.present = 1;

	nStatus = dot11f_get_packed_tpc_request_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a TPC Request (0x%08x)."), nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fTPCRequest);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a TPC Request (0x"
				       "%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc(pMac->hHdd, TXRX_FRM_802_11_MGMT,
				 (uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a TPC"
				       " Request."), nBytes);
		return;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	qdf_mem_copy(pMacHdr->bssId, psessionEntry->bssId, sizeof(tSirMacAddr));

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	nStatus = dot11f_pack_tpc_request(pMac, &frm, pFrame +
					  sizeof(tSirMacMgmtHdr),
					  nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE, FL("Failed to pack a TPC Request (0x%08x)."),
			nStatus);
		cds_packet_free(pMac->hHdd, TXRX_FRM_802_11_MGMT,
				(void *)pFrame, (void *)pPacket);
		return;         /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while packing a T"
				       "PC Request (0x%08x)."), nStatus);
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 ((psessionEntry) ? psessionEntry->
			  peSessionId : NO_SESSION), pMacHdr->fc.subType));
	qdf_status =
		wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
			   TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS, 7,
			   lim_tx_complete, pFrame, 0, 0);
	MTRACE(qdf_trace
		       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
		       ((psessionEntry) ? psessionEntry->peSessionId : NO_SESSION),
		       qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGE,
			FL("Failed to send a TPC Request (%X)!"),
			qdf_status);
		/* Pkt will be freed up by the callback */
	}

} /* End lim_send_tpc_request_frame. */

/**
 * \brief Send a TPC Report Action frame
 *
 *
 * \param pMac Pointer to the global MAC datastructure
 *
 * \param pTpcReqFrame Pointer to the received TPC Request
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
lim_send_tpc_report_frame(tpAniSirGlobal pMac,
			  tpSirMacTpcReqActionFrame pTpcReqFrame,
			  tSirMacAddr peer, tpPESession psessionEntry)
{
	tDot11fTPCReport frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_SPECTRUM_MGMT;
	frm.Action.action = SIR_MAC_ACTION_TPC_REPORT_ID;
	frm.DialogToken.token = pTpcReqFrame->actionHeader.dialogToken;

	frm.TPCReport.tx_power = 0;
	frm.TPCReport.link_margin = 0;
	frm.TPCReport.present = 1;

	nStatus = dot11f_get_packed_tpc_report_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a TPC Report (0x%08x)."), nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fTPCReport);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a TPC Report (0x"
				       "%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc(pMac->hHdd, TXRX_FRM_802_11_MGMT,
				 (uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a TPC"
				       " Report."), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer);

	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	qdf_mem_copy(pMacHdr->bssId, psessionEntry->bssId, sizeof(tSirMacAddr));

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	nStatus = dot11f_pack_tpc_report(pMac, &frm, pFrame +
					 sizeof(tSirMacMgmtHdr),
					 nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE, FL("Failed to pack a TPC Report (0x%08x)."),
			nStatus);
		cds_packet_free(pMac->hHdd, TXRX_FRM_802_11_MGMT,
				(void *)pFrame, (void *)pPacket);
		return eSIR_FAILURE;    /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while packing a T"
				       "PC Report (0x%08x)."), nStatus);
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 ((psessionEntry) ? psessionEntry->
			  peSessionId : NO_SESSION), pMacHdr->fc.subType));
	qdf_status =
		wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
			   TXRX_FRM_802_11_MGMT, ANI_TXDIR_TODS, 7,
			   lim_tx_complete, pFrame, 0, 0);
	MTRACE(qdf_trace
		       (QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
		       ((psessionEntry) ? psessionEntry->peSessionId : NO_SESSION),
		       qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGE,
			FL("Failed to send a TPC Report (%X)!"),
			qdf_status);
		/* Pkt will be freed up by the callback */
		return eSIR_FAILURE;    /* just allocated... */
	}

	return eSIR_SUCCESS;

} /* End lim_send_tpc_report_frame. */
#endif /* ANI_SUPPORT_11H */

/**
 * \brief Send a Channel Switch Announcement
 *
 *
 * \param pMac Pointer to the global MAC datastructure
 *
 * \param peer MAC address to which this frame will be sent
 *
 * \param nMode
 *
 * \param nNewChannel
 *
 * \param nCount
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
lim_send_channel_switch_mgmt_frame(tpAniSirGlobal pMac,
				   tSirMacAddr peer,
				   uint8_t nMode,
				   uint8_t nNewChannel,
				   uint8_t nCount, tpPESession psessionEntry)
{
	tDot11fChannelSwitch frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;     /* , nCfg; */
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;

	uint8_t smeSessionId = 0;

	if (psessionEntry == NULL) {
		PELOGE(lim_log(pMac, LOGE, FL("Session entry is NULL!!!"));)
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_SPECTRUM_MGMT;
	frm.Action.action = SIR_MAC_ACTION_CHANNEL_SWITCH_ID;
	frm.ChanSwitchAnn.switchMode = nMode;
	frm.ChanSwitchAnn.newChannel = nNewChannel;
	frm.ChanSwitchAnn.switchCount = nCount;
	frm.ChanSwitchAnn.present = 1;

	nStatus = dot11f_get_packed_channel_switch_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Channel Switch (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fChannelSwitch);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Channel Switch (0x"
				       "%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a TPC"
				       " Report."), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer,
		psessionEntry->selfMacAddr);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;
	qdf_mem_copy((uint8_t *) pMacHdr->bssId,
		     (uint8_t *) psessionEntry->bssId, sizeof(tSirMacAddr));

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	nStatus = dot11f_pack_channel_switch(pMac, &frm, pFrame +
					     sizeof(tSirMacMgmtHdr),
					     nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack a Channel Switch (0x%08x)."),
			nStatus);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;    /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while packing a C"
				       "hannel Switch (0x%08x)."), nStatus);
	}

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGE,
			FL("Failed to send a Channel Switch (%X)!"),
			qdf_status);
		/* Pkt will be freed up by the callback */
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;

} /* End lim_send_channel_switch_mgmt_frame. */

/**
 * lim_send_extended_chan_switch_action_frame()- function to send ECSA
 * action frame over the air .
 * @mac_ctx: pointer to global mac structure
 * @peer: Destination mac.
 * @mode: channel switch mode
 * @new_op_class: new op class
 * @new_channel: new channel to switch
 * @count: channel switch count
 *
 * This function is called to send ECSA frame.
 *
 * Return: success if frame is sent else return failure
 */

tSirRetStatus
lim_send_extended_chan_switch_action_frame(tpAniSirGlobal mac_ctx,
		tSirMacAddr peer, uint8_t mode, uint8_t new_op_class,
		uint8_t new_channel, uint8_t count, tpPESession session_entry)
{
	tDot11fext_channel_switch_action_frame frm;
	uint8_t                  *frame;
	tpSirMacMgmtHdr          mac_hdr;
	uint32_t                 num_bytes, n_payload, status;
	void                     *packet;
	QDF_STATUS               qdf_status;
	uint8_t                  txFlag = 0;
	uint8_t                  sme_session_id = 0;

	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE, FL("Session entry is NULL!!!"));
		return eSIR_FAILURE;
	}

	sme_session_id = session_entry->smeSessionId;

	qdf_mem_set(&frm, sizeof(frm), 0);

	frm.Category.category     = SIR_MAC_ACTION_PUBLIC_USAGE;
	frm.Action.action         = SIR_MAC_ACTION_EXT_CHANNEL_SWITCH_ID;

	frm.ext_chan_switch_ann_action.switch_mode = mode;
	frm.ext_chan_switch_ann_action.op_class = new_op_class;
	frm.ext_chan_switch_ann_action.new_channel = new_channel;
	frm.ext_chan_switch_ann_action.switch_count = count;


	status = dot11f_get_packed_ext_channel_switch_action_frame_size(mac_ctx,
							    &frm, &n_payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGP,
		 FL("Failed to get packed size for Channel Switch 0x%08x."),
				 status);
		/* We'll fall back on the worst case scenario*/
		n_payload = sizeof(tDot11fext_channel_switch_action_frame);
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
		 FL("There were warnings while calculating the packed size for a Ext Channel Switch (0x%08x)."),
		 status);
	}

	num_bytes = n_payload + sizeof(tSirMacMgmtHdr);

	qdf_status = cds_packet_alloc((uint16_t)num_bytes,
				(void **) &frame, (void **) &packet);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGP,
		 FL("Failed to allocate %d bytes for a Ext Channel Switch."),
								 num_bytes);
		return eSIR_FAILURE;
	}

	/* Paranoia*/
	qdf_mem_set(frame, num_bytes, 0);

	/* Next, we fill out the buffer descriptor */
	lim_populate_mac_header(mac_ctx, frame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, session_entry->selfMacAddr);
	mac_hdr = (tpSirMacMgmtHdr) frame;
	qdf_mem_copy((uint8_t *) mac_hdr->bssId,
				   (uint8_t *) session_entry->bssId,
				   sizeof(tSirMacAddr));

	status = dot11f_pack_ext_channel_switch_action_frame(mac_ctx, &frm,
		frame + sizeof(tSirMacMgmtHdr), n_payload, &n_payload);
	if (DOT11F_FAILED(status)) {
		lim_log(mac_ctx, LOGE,
			 FL("Failed to pack a Channel Switch 0x%08x."),
								 status);
		cds_packet_free((void *)packet);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		lim_log(mac_ctx, LOGW,
		 FL("There were warnings while packing a Channel Switch 0x%08x."),
		 status);
	}

	if ((SIR_BAND_5_GHZ ==
		lim_get_rf_band(session_entry->currentOperChannel)) ||
		(session_entry->pePersona == QDF_P2P_CLIENT_MODE) ||
		(session_entry->pePersona == QDF_P2P_GO_MODE)) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	lim_log(mac_ctx, LOG1,
	 FL("Send Ext channel Switch to :"MAC_ADDRESS_STR" with swcount %d, swmode %d , newchannel %d newops %d"),
		MAC_ADDR_ARRAY(mac_hdr->da),
		frm.ext_chan_switch_ann_action.switch_count,
		frm.ext_chan_switch_ann_action.switch_mode,
		frm.ext_chan_switch_ann_action.new_channel,
			 frm.ext_chan_switch_ann_action.op_class);

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			session_entry->peSessionId, mac_hdr->fc.subType));
	qdf_status = wma_tx_frame(mac_ctx, packet, (uint16_t) num_bytes,
						 TXRX_FRM_802_11_MGMT,
						 ANI_TXDIR_TODS,
						 7,
						 lim_tx_complete, frame,
						 txFlag, sme_session_id, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			session_entry->peSessionId, qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(mac_ctx, LOGE,
		  FL("Failed to send a Ext Channel Switch %X!"),
							 qdf_status);
		/* Pkt will be freed up by the callback */
		return eSIR_FAILURE;
	}
	return eSIR_SUCCESS;
} /* End lim_send_extended_chan_switch_action_frame */

tSirRetStatus
lim_send_vht_opmode_notification_frame(tpAniSirGlobal pMac,
				       tSirMacAddr peer,
				       uint8_t nMode, tpPESession psessionEntry)
{
	tDot11fOperatingMode frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload = 0, nStatus; /* , nCfg; */
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;

	uint8_t smeSessionId = 0;

	if (psessionEntry == NULL) {
		PELOGE(lim_log(pMac, LOGE, FL("Session entry is NULL!!!"));)
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_VHT;
	frm.Action.action = SIR_MAC_VHT_OPMODE_NOTIFICATION;
	frm.OperatingMode.chanWidth = nMode;
	frm.OperatingMode.rxNSS = 0;
	frm.OperatingMode.rxNSSType = 0;

	nStatus = dot11f_get_packed_operating_mode_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Operating Mode (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fOperatingMode);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Operating Mode (0x"
				       "%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP,
			FL("Failed to allocate %d bytes for a Operating Mode"
			   " Report."), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Next, we fill out the buffer descriptor: */
	if (psessionEntry->pePersona == QDF_SAP_MODE)
		lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
			SIR_MAC_MGMT_ACTION, peer,
			psessionEntry->selfMacAddr);
	else
		lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
			SIR_MAC_MGMT_ACTION, psessionEntry->bssId,
			psessionEntry->selfMacAddr);
	pMacHdr = (tpSirMacMgmtHdr) pFrame;
	qdf_mem_copy((uint8_t *) pMacHdr->bssId,
		     (uint8_t *) psessionEntry->bssId, sizeof(tSirMacAddr));
	nStatus = dot11f_pack_operating_mode(pMac, &frm, pFrame +
					     sizeof(tSirMacMgmtHdr),
					     nPayload, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack a Operating Mode (0x%08x)."),
			nStatus);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;    /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL("There were warnings while packing a Operating Mode"
			   " (0x%08x)."), nStatus);
	}
	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac, pPacket, (uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGE,
			FL("Failed to send a Channel Switch (%X)!"),
			qdf_status);
		/* Pkt will be freed up by the callback */
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}

/**
 * \brief Send a Neighbor Report Request Action frame
 *
 *
 * \param pMac Pointer to the global MAC structure
 *
 * \param pNeighborReq Address of a tSirMacNeighborReportReq
 *
 * \param peer mac address of peer station.
 *
 * \param psessionEntry address of session entry.
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
lim_send_neighbor_report_request_frame(tpAniSirGlobal pMac,
				       tpSirMacNeighborReportReq pNeighborReq,
				       tSirMacAddr peer, tpPESession psessionEntry)
{
	tSirRetStatus statusCode = eSIR_SUCCESS;
	tDot11fNeighborReportRequest frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL
				("(psession == NULL) in Request to send Neighbor Report request action frame"));
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;
	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_RRM;
	frm.Action.action = SIR_MAC_RRM_NEIGHBOR_REQ;
	frm.DialogToken.token = pNeighborReq->dialogToken;

	if (pNeighborReq->ssid_present) {
		populate_dot11f_ssid(pMac, &pNeighborReq->ssid, &frm.SSID);
	}

	nStatus =
		dot11f_get_packed_neighbor_report_request_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Neighbor Report Request(0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fNeighborReportRequest);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Neighbor Rep"
				       "ort Request(0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP,
			FL("Failed to allocate %d bytes for a Neighbor "
			   "Report Request."), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Copy necessary info to BD */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, psessionEntry->selfMacAddr);

	/* Update A3 with the BSSID */
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	/* Now, we're ready to "pack" the frames */
	nStatus = dot11f_pack_neighbor_report_request(pMac,
						      &frm,
						      pFrame +
						      sizeof(tSirMacMgmtHdr),
						      nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL
				("Failed to pack an Neighbor Report Request (0x%08x)."),
			nStatus);

		/* FIXME - Need to convert to tSirRetStatus */
		statusCode = eSIR_FAILURE;
		goto returnAfterError;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL("There were warnings while packing Neighbor Report "
			   "Request (0x%08x)."), nStatus);
	}

	lim_log(pMac, LOGW, FL("Sending a Neighbor Report Request to "));
	lim_print_mac_addr(pMac, peer, LOGW);

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac,
				pPacket,
				(uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	if (QDF_STATUS_SUCCESS != qdf_status) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("wma_tx_frame FAILED! Status [%d]"),
			       qdf_status);
		       )
		statusCode = eSIR_FAILURE;
		/* Pkt will be freed up by the callback */
		return statusCode;
	} else
		return eSIR_SUCCESS;

returnAfterError:
	cds_packet_free((void *)pPacket);

	return statusCode;
} /* End lim_send_neighbor_report_request_frame. */

/**
 * \brief Send a Link Report Action frame
 *
 *
 * \param pMac Pointer to the global MAC structure
 *
 * \param pLinkReport Address of a tSirMacLinkReport
 *
 * \param peer mac address of peer station.
 *
 * \param psessionEntry address of session entry.
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
lim_send_link_report_action_frame(tpAniSirGlobal pMac,
				  tpSirMacLinkReport pLinkReport,
				  tSirMacAddr peer, tpPESession psessionEntry)
{
	tSirRetStatus statusCode = eSIR_SUCCESS;
	tDot11fLinkMeasurementReport frm;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL
				("(psession == NULL) in Request to send Link Report action frame"));
		return eSIR_FAILURE;
	}

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);

	frm.Category.category = SIR_MAC_ACTION_RRM;
	frm.Action.action = SIR_MAC_RRM_LINK_MEASUREMENT_RPT;
	frm.DialogToken.token = pLinkReport->dialogToken;

	/* IEEE Std. 802.11 7.3.2.18. for the report element. */
	/* Even though TPC report an IE, it is represented using fixed fields since it is positioned */
	/* in the middle of other fixed fields in the link report frame(IEEE Std. 802.11k section7.4.6.4 */
	/* and frame parser always expects IEs to come after all fixed fields. It is easier to handle */
	/* such case this way than changing the frame parser. */
	frm.TPCEleID.TPCId = SIR_MAC_TPC_RPT_EID;
	frm.TPCEleLen.TPCLen = 2;
	frm.TxPower.txPower = pLinkReport->txPower;
	frm.LinkMargin.linkMargin = 0;

	frm.RxAntennaId.antennaId = pLinkReport->rxAntenna;
	frm.TxAntennaId.antennaId = pLinkReport->txAntenna;
	frm.RCPI.rcpi = pLinkReport->rcpi;
	frm.RSNI.rsni = pLinkReport->rsni;

	nStatus =
		dot11f_get_packed_link_measurement_report_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Link Report (0x%08x)."), nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fLinkMeasurementReport);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Link Rep"
				       "ort (0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP, FL("Failed to allocate %d bytes for a Link "
				       "Report."), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Copy necessary info to BD */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, psessionEntry->selfMacAddr);

	/* Update A3 with the BSSID */
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	/* Now, we're ready to "pack" the frames */
	nStatus = dot11f_pack_link_measurement_report(pMac,
						      &frm,
						      pFrame +
						      sizeof(tSirMacMgmtHdr),
						      nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack an Link Report (0x%08x)."), nStatus);

		/* FIXME - Need to convert to tSirRetStatus */
		statusCode = eSIR_FAILURE;
		goto returnAfterError;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL
				("There were warnings while packing Link Report (0x%08x)."),
			nStatus);
	}

	lim_log(pMac, LOGW, FL("Sending a Link Report to "));
	lim_print_mac_addr(pMac, peer, LOGW);

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac,
				pPacket,
				(uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	if (QDF_STATUS_SUCCESS != qdf_status) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("wma_tx_frame FAILED! Status [%d]"),
			       qdf_status);
		       )
		statusCode = eSIR_FAILURE;
		/* Pkt will be freed up by the callback */
		return statusCode;
	} else
		return eSIR_SUCCESS;

returnAfterError:
	cds_packet_free((void *)pPacket);

	return statusCode;
} /* End lim_send_link_report_action_frame. */

/**
 * \brief Send a Beacon Report Action frame
 *
 *
 * \param pMac Pointer to the global MAC structure
 *
 * \param dialog_token dialog token to be used in the action frame.
 *
 * \param num_report number of reports in pRRMReport.
 *
 * \param pRRMReport Address of a tSirMacRadioMeasureReport.
 *
 * \param peer mac address of peer station.
 *
 * \param psessionEntry address of session entry.
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
lim_send_radio_measure_report_action_frame(tpAniSirGlobal pMac,
					   uint8_t dialog_token,
					   uint8_t num_report,
					   tpSirMacRadioMeasureReport pRRMReport,
					   tSirMacAddr peer,
					   tpPESession psessionEntry)
{
	tSirRetStatus statusCode = eSIR_SUCCESS;
	uint8_t *pFrame;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t i;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	tDot11fRadioMeasurementReport *frm =
		qdf_mem_malloc(sizeof(tDot11fRadioMeasurementReport));
	if (!frm) {
		lim_log(pMac, LOGE,
			FL
				("Not enough memory to allocate tDot11fRadioMeasurementReport"));
		return eSIR_MEM_ALLOC_FAILED;
	}

	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL
				("(psession == NULL) in Request to send Beacon Report action frame"));
		qdf_mem_free(frm);
		return eSIR_FAILURE;
	}
	qdf_mem_set((uint8_t *) frm, sizeof(*frm), 0);

	frm->Category.category = SIR_MAC_ACTION_RRM;
	frm->Action.action = SIR_MAC_RRM_RADIO_MEASURE_RPT;
	frm->DialogToken.token = dialog_token;

	frm->num_MeasurementReport =
		(num_report >
		 RADIO_REPORTS_MAX_IN_A_FRAME) ? RADIO_REPORTS_MAX_IN_A_FRAME :
		num_report;

	for (i = 0; i < frm->num_MeasurementReport; i++) {
		frm->MeasurementReport[i].type = pRRMReport[i].type;
		frm->MeasurementReport[i].token = pRRMReport[i].token;
		frm->MeasurementReport[i].late = 0;     /* IEEE 802.11k section 7.3.22. (always zero in rrm) */
		switch (pRRMReport[i].type) {
		case SIR_MAC_RRM_BEACON_TYPE:
			populate_dot11f_beacon_report(pMac,
						      &frm->MeasurementReport[i],
						      &pRRMReport[i].report.
						      beaconReport);
			frm->MeasurementReport[i].incapable =
				pRRMReport[i].incapable;
			frm->MeasurementReport[i].refused =
				pRRMReport[i].refused;
			frm->MeasurementReport[i].present = 1;
			break;
		default:
			frm->MeasurementReport[i].incapable =
				pRRMReport[i].incapable;
			frm->MeasurementReport[i].refused =
				pRRMReport[i].refused;
			frm->MeasurementReport[i].present = 1;
			break;
		}
	}

	nStatus =
		dot11f_get_packed_radio_measurement_report_size(pMac, frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a Radio Measure Report (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fLinkMeasurementReport);
		qdf_mem_free(frm);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for a Radio Measure Rep"
				       "ort (0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);

	qdf_status =
		cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				 (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP,
			FL("Failed to allocate %d bytes for a Radio Measure "
			   "Report."), nBytes);
		qdf_mem_free(frm);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Copy necessary info to BD */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, psessionEntry->selfMacAddr);

	/* Update A3 with the BSSID */
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

#ifdef WLAN_FEATURE_11W
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);
#endif

	/* Now, we're ready to "pack" the frames */
	nStatus = dot11f_pack_radio_measurement_report(pMac,
						       frm,
						       pFrame +
						       sizeof(tSirMacMgmtHdr),
						       nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack an Radio Measure Report (0x%08x)."),
			nStatus);

		/* FIXME - Need to convert to tSirRetStatus */
		statusCode = eSIR_FAILURE;
		goto returnAfterError;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL("There were warnings while packing Radio "
			   "Measure Report (0x%08x)."), nStatus);
	}

	lim_log(pMac, LOGW, FL("Sending a Radio Measure Report to "));
	lim_print_mac_addr(pMac, peer, LOGW);

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac,
				pPacket,
				(uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	if (QDF_STATUS_SUCCESS != qdf_status) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("wma_tx_frame FAILED! Status [%d]"),
			       qdf_status);
		       )
		statusCode = eSIR_FAILURE;
		/* Pkt will be freed up by the callback */
		qdf_mem_free(frm);
		return statusCode;
	} else {
		qdf_mem_free(frm);
		return eSIR_SUCCESS;
	}

returnAfterError:
	qdf_mem_free(frm);
	cds_packet_free((void *)pPacket);
	return statusCode;
}

#ifdef WLAN_FEATURE_11W
/**
 * \brief Send SA query request action frame to peer
 *
 * \sa lim_send_sa_query_request_frame
 *
 *
 * \param pMac    The global tpAniSirGlobal object
 *
 * \param transId Transaction identifier
 *
 * \param peer    The Mac address of the station to which this action frame is addressed
 *
 * \param psessionEntry The PE session entry
 *
 * \return eSIR_SUCCESS if setup completes successfully
 *         eSIR_FAILURE is some problem is encountered
 */

tSirRetStatus lim_send_sa_query_request_frame(tpAniSirGlobal pMac, uint8_t *transId,
					      tSirMacAddr peer,
					      tpPESession psessionEntry)
{

	tDot11fSaQueryReq frm;  /* SA query request action frame */
	uint8_t *pFrame;
	tSirRetStatus nSirStatus;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);
	frm.Category.category = SIR_MAC_ACTION_SA_QUERY;
	/* 11w action  field is :
	   action: 0 --> SA Query Request action frame
	   action: 1 --> SA Query Response action frame */
	frm.Action.action = SIR_MAC_SA_QUERY_REQ;
	/* 11w SA Query Request transId */
	qdf_mem_copy(&frm.TransactionId.transId[0], &transId[0], 2);

	nStatus = dot11f_get_packed_sa_query_req_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size "
				       "for an SA Query Request (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fSaQueryReq);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for an SA Query Request"
				       " (0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);
	qdf_status =
		cds_packet_alloc(nBytes, (void **)&pFrame, (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP,
			FL("Failed to allocate %d bytes for a SA Query Request "
			   "action frame"), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Copy necessary info to BD */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, psessionEntry->selfMacAddr);

	/* Update A3 with the BSSID */
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

	/* Since this is a SA Query Request, set the "protect" (aka WEP) bit */
	/* in the FC */
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);

	/* Pack 11w SA Query Request frame */
	nStatus = dot11f_pack_sa_query_req(pMac,
					   &frm,
					   pFrame + sizeof(tSirMacMgmtHdr),
					   nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack an SA Query Request (0x%08x)."),
			nStatus);
		/* FIXME - Need to convert to tSirRetStatus */
		nSirStatus = eSIR_FAILURE;
		goto returnAfterError;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL
				("There were warnings while packing SA Query Request (0x%08x)."),
			nStatus);
	}

	lim_log(pMac, LOG1, FL("Sending an SA Query Request to "));
	lim_print_mac_addr(pMac, peer, LOG1);
	lim_log(pMac, LOG1, FL("Sending an SA Query Request from "));
	lim_print_mac_addr(pMac, psessionEntry->selfMacAddr, LOG1);

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
#ifdef WLAN_FEATURE_P2P
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
#endif
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}
	smeSessionId = psessionEntry->smeSessionId;

	qdf_status = wma_tx_frame(pMac,
				pPacket,
				(uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("wma_tx_frame FAILED! Status [%d]"),
			       qdf_status);
		       )
		nSirStatus = eSIR_FAILURE;
		/* Pkt will be freed up by the callback */
		return nSirStatus;
	} else {
		return eSIR_SUCCESS;
	}

returnAfterError:
	cds_packet_free((void *)pPacket);
	return nSirStatus;
} /* End lim_send_sa_query_request_frame */

/**
 * \brief Send SA query response action frame to peer
 *
 * \sa lim_send_sa_query_response_frame
 *
 *
 * \param pMac    The global tpAniSirGlobal object
 *
 * \param transId Transaction identifier received in SA query request action frame
 *
 * \param peer    The Mac address of the AP to which this action frame is addressed
 *
 * \param psessionEntry The PE session entry
 *
 * \return eSIR_SUCCESS if setup completes successfully
 *         eSIR_FAILURE is some problem is encountered
 */

tSirRetStatus lim_send_sa_query_response_frame(tpAniSirGlobal pMac,
					       uint8_t *transId, tSirMacAddr peer,
					       tpPESession psessionEntry)
{

	tDot11fSaQueryRsp frm;  /* SA query reponse action frame */
	uint8_t *pFrame;
	tSirRetStatus nSirStatus;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t nBytes, nPayload, nStatus;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint8_t txFlag = 0;
	uint8_t smeSessionId = 0;

	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &frm, sizeof(frm), 0);
	frm.Category.category = SIR_MAC_ACTION_SA_QUERY;
	/*11w action  field is :
	   action: 0 --> SA query request action frame
	   action: 1 --> SA query response action frame */
	frm.Action.action = SIR_MAC_SA_QUERY_RSP;
	/*11w SA query response transId is same as
	   SA query request transId */
	qdf_mem_copy(&frm.TransactionId.transId[0], &transId[0], 2);

	nStatus = dot11f_get_packed_sa_query_rsp_size(pMac, &frm, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGP, FL("Failed to calculate the packed size f"
				       "or a SA Query Response (0x%08x)."),
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fSaQueryRsp);
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW, FL("There were warnings while calculating "
				       "the packed size for an SA Query Response"
				       " (0x%08x)."), nStatus);
	}

	nBytes = nPayload + sizeof(tSirMacMgmtHdr);
	qdf_status =
		cds_packet_alloc(nBytes, (void **)&pFrame, (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		lim_log(pMac, LOGP,
			FL("Failed to allocate %d bytes for a SA query response"
			   " action frame"), nBytes);
		return eSIR_FAILURE;
	}
	/* Paranoia: */
	qdf_mem_set(pFrame, nBytes, 0);

	/* Copy necessary info to BD */
	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_ACTION, peer, psessionEntry->selfMacAddr);

	/* Update A3 with the BSSID */
	pMacHdr = (tpSirMacMgmtHdr) pFrame;

	sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);

	/* Since this is a SA Query Response, set the "protect" (aka WEP) bit */
	/* in the FC */
	lim_set_protected_bit(pMac, psessionEntry, peer, pMacHdr);

	/* Pack 11w SA query response frame */
	nStatus = dot11f_pack_sa_query_rsp(pMac,
					   &frm,
					   pFrame + sizeof(tSirMacMgmtHdr),
					   nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		lim_log(pMac, LOGE,
			FL("Failed to pack an SA Query Response (0x%08x)."),
			nStatus);
		/* FIXME - Need to convert to tSirRetStatus */
		nSirStatus = eSIR_FAILURE;
		goto returnAfterError;
	} else if (DOT11F_WARNED(nStatus)) {
		lim_log(pMac, LOGW,
			FL
				("There were warnings while packing SA Query Response (0x%08x)."),
			nStatus);
	}

	lim_log(pMac, LOG1, FL("Sending a SA Query Response to "));
	lim_print_mac_addr(pMac, peer, LOGW);

	if ((SIR_BAND_5_GHZ == lim_get_rf_band(psessionEntry->currentOperChannel))
#ifdef WLAN_FEATURE_P2P
	    || (psessionEntry->pePersona == QDF_P2P_CLIENT_MODE) ||
	    (psessionEntry->pePersona == QDF_P2P_GO_MODE)
#endif
	    ) {
		txFlag |= HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_MGMT,
			 psessionEntry->peSessionId, pMacHdr->fc.subType));
	qdf_status = wma_tx_frame(pMac,
				pPacket,
				(uint16_t) nBytes,
				TXRX_FRM_802_11_MGMT,
				ANI_TXDIR_TODS,
				7, lim_tx_complete, pFrame, txFlag,
				smeSessionId, 0);
	MTRACE(qdf_trace(QDF_MODULE_ID_PE, TRACE_CODE_TX_COMPLETE,
			 psessionEntry->peSessionId, qdf_status));
	if (QDF_STATUS_SUCCESS != qdf_status) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("wma_tx_frame FAILED! Status [%d]"),
			       qdf_status);
		       )
		nSirStatus = eSIR_FAILURE;
		/* Pkt will be freed up by the callback */
		return nSirStatus;
	} else {
		return eSIR_SUCCESS;
	}

returnAfterError:
	cds_packet_free((void *)pPacket);
	return nSirStatus;
} /* End lim_send_sa_query_response_frame */
#endif
