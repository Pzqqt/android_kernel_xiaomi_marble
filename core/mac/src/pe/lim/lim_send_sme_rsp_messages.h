/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/*
 * This file lim_send_sme_rsp_messages.h contains the definitions for
 * sending SME response/notification messages to applications above
 * MAC software.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __LIM_SEND_SME_RSP_H
#define __LIM_SEND_SME_RSP_H

#include "sir_common.h"
#include "sir_api.h"
#include "sir_mac_prot_def.h"

/* Functions for sending responses to Host */
void lim_send_sme_rsp(tpAniSirGlobal, uint16_t, tSirResultCodes, uint8_t,
		      uint16_t);
void lim_send_sme_roc_rsp(tpAniSirGlobal mac_ctx, uint16_t msg_type,
	 tSirResultCodes result_code, uint8_t sme_session_id,
	 uint32_t scan_id);
void lim_send_sme_start_bss_rsp(tpAniSirGlobal, uint16_t, tSirResultCodes,
				tpPESession, uint8_t, uint16_t);
void lim_send_sme_scan_rsp(tpAniSirGlobal, tSirResultCodes, uint8_t,
	uint16_t, uint32_t scan_id);
void lim_post_sme_scan_rsp_message(tpAniSirGlobal, tSirResultCodes,
				   uint8_t, uint16_t, uint32_t scan_id);

void lim_send_sme_join_reassoc_rsp(tpAniSirGlobal, uint16_t, tSirResultCodes,
				   uint16_t, tpPESession, uint8_t, uint16_t);
void lim_send_sme_disassoc_ntf(tpAniSirGlobal, tSirMacAddr, tSirResultCodes,
			       uint16_t, uint16_t, uint8_t, uint16_t, tpPESession);
void lim_send_sme_deauth_ntf(tpAniSirGlobal, tSirMacAddr, tSirResultCodes, uint16_t,
			     uint16_t, uint8_t, uint16_t);
void lim_send_sme_disassoc_ind(tpAniSirGlobal, tpDphHashNode, tpPESession);
void lim_send_sme_deauth_ind(tpAniSirGlobal, tpDphHashNode,
			     tpPESession psessionEntry);
void lim_send_sme_wm_status_change_ntf(tpAniSirGlobal, tSirSmeStatusChangeCode,
				       uint32_t *, uint16_t, uint8_t);
void lim_send_sme_set_context_rsp(tpAniSirGlobal, struct qdf_mac_addr, uint16_t,
				  tSirResultCodes, tpPESession, uint8_t, uint16_t);
void lim_send_sme_neighbor_bss_ind(tpAniSirGlobal, tLimScanResultNode *);
void lim_handle_delete_bss_rsp(tpAniSirGlobal pMac, tpSirMsgQ MsgQ);
void lim_handle_csa_offload_msg(tpAniSirGlobal mac_ctx, tpSirMsgQ msg);

void
lim_send_sme_aggr_qos_rsp(tpAniSirGlobal pMac, tpSirAggrQosRsp aggrQosRsp,
			  uint8_t smesessionId);

void lim_send_sme_addts_rsp(tpAniSirGlobal pMac, uint8_t rspReqd, uint32_t status,
			    tpPESession psessionEntry, tSirMacTspecIE tspec,
			    uint8_t smesessionId, uint16_t smetransactionId);
void lim_send_sme_delts_rsp(tpAniSirGlobal pMac, tpSirDeltsReq delts,
			    uint32_t status, tpPESession psessionEntry,
			    uint8_t smessionId, uint16_t smetransactionId);
void lim_send_sme_delts_ind(tpAniSirGlobal pMac, tpSirDeltsReqInfo delts,
			    uint16_t aid, tpPESession);
void lim_send_sme_stats_rsp(tpAniSirGlobal pMac, uint16_t msgtype, void *stats);

void lim_send_sme_pe_statistics_rsp(tpAniSirGlobal pMac, uint16_t msgtype,
				    void *stats);
#ifdef FEATURE_WLAN_ESE
void lim_send_sme_pe_ese_tsm_rsp(tpAniSirGlobal pMac, tAniGetTsmStatsRsp *pStats);
#endif

void lim_send_sme_ibss_peer_ind(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
				uint16_t staIndex, uint8_t ucastIdx,
				uint8_t bcastIdx, uint8_t *beacon,
				uint16_t beaconLen, uint16_t msgType,
				uint8_t sessionId);
void lim_send_sme_max_assoc_exceeded_ntf(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
					 uint8_t smesessionId);
#ifdef FEATURE_WLAN_TDLS
void lim_send_sme_tdls_link_establish_req_rsp(tpAniSirGlobal pMac, uint8_t sessionId,
					      struct qdf_mac_addr *peermac,
					      tDphHashNode *pStaDs, uint8_t status);
void lim_send_sme_tdls_event_notify(tpAniSirGlobal pMac, uint16_t msgType,
				    void *events);
#endif

void lim_send_sme_dfs_event_notify(tpAniSirGlobal pMac, uint16_t msgType,
				   void *event);
void lim_send_sme_ap_channel_switch_resp(tpAniSirGlobal pMac,
					 tpPESession psessionEntry,
					 tpSwitchChannelParams pChnlParams);
void
lim_process_beacon_tx_success_ind(tpAniSirGlobal pMac, uint16_t msgType,
				  void *event);

typedef enum {
	lim_csa_ie_present = 0x00000001,
	lim_xcsa_ie_present = 0x00000002,
	lim_wbw_ie_present = 0x00000004,
	lim_cswarp_ie_present = 0x00000008,
} lim_csa_event_ies_present_flag;

#endif /* __LIM_SEND_SME_RSP_H */
