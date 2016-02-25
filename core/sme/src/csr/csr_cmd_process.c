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

/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *
    \file csr_cmd_process.c

    Implementation for processing various commands.
   ========================================================================== */

#include "ani_global.h"

#include "csr_inside_api.h"
#include "sme_inside.h"
#include "sms_debug.h"
#include "mac_trace.h"

/**
 * csr_msg_processor() - To process all csr msg
 * @mac_ctx: mac context
 * @msg_buf: message buffer
 *
 * This routine will handle all the message for csr to process
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_msg_processor(tpAniSirGlobal mac_ctx, void *msg_buf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeRsp *sme_rsp = (tSirSmeRsp *) msg_buf;
#ifdef FEATURE_WLAN_SCAN_PNO
	tSirMbMsg *msg = (tSirMbMsg *) msg_buf;
	tCsrRoamSession *session;
#endif
	uint8_t session_id = sme_rsp->sessionId;
	eCsrRoamState cur_state = mac_ctx->roam.curState[session_id];

	sms_log(mac_ctx, LOG2,
		FL("msg %d[0x%04X] recvd in curstate %s & substate %s id(%d)"),
		sme_rsp->messageType, sme_rsp->messageType,
		mac_trace_getcsr_roam_state(cur_state),
		mac_trace_getcsr_roam_sub_state(
			mac_ctx->roam.curSubState[session_id]),
		session_id);

#ifdef FEATURE_WLAN_SCAN_PNO
	/*
	 * PNO scan responses have to be handled irrespective of CSR roam state.
	 * Check if PNO has been started & only then process the PNO scan result
	 * Also note that normal scan isn't allowed when PNO scan is in progress
	 * and so the scan responses reaching here when PNO is started must be
	 * PNO responses. For normal scan, the PNO started flag will be false
	 * and it'll be processed as usual based on the current CSR roam state.
	 */
	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found, msgType : %d"),
			session_id, msg->type);
		return QDF_STATUS_E_FAILURE;
	}

	if (eWNI_SME_SCAN_RSP == msg->type) {
		status = csr_scanning_state_msg_processor(mac_ctx, msg_buf);
		if (QDF_STATUS_SUCCESS != status)
			sms_log(mac_ctx, LOGE,
				FL("handling PNO scan resp 0x%X CSR state %d"),
				sme_rsp->messageType, cur_state);
		return status;
	}
#endif

	/* Process the message based on the state of the roaming states... */
#if defined(ANI_RTT_DEBUG)
	if (!pAdapter->fRttModeEnabled) {
#endif
		switch (cur_state) {
		case eCSR_ROAMING_STATE_JOINED:
			/* are we in joined state */
			csr_roam_joined_state_msg_processor(mac_ctx, msg_buf);
			break;
		case eCSR_ROAMING_STATE_JOINING:
			/* are we in roaming states */
#if defined(ANI_EMUL_ASSOC)
			emulRoamingStateMsgProcessor(pAdapter, pMBBufHdr);
#endif
			csr_roaming_state_msg_processor(mac_ctx, msg_buf);
			break;

		default:
			/*
			 * For all other messages, we ignore it
			 * To work-around an issue where checking for set/remove
			 * key base on connection state is no longer workable
			 * due to failure or finding the condition meets both
			 * SAP and infra/IBSS requirement.
			 */
			if (eWNI_SME_SETCONTEXT_RSP == sme_rsp->messageType) {
				sms_log(mac_ctx, LOGW,
					FL("handling msg 0x%X CSR state is %d"),
					sme_rsp->messageType, cur_state);
				csr_roam_check_for_link_status_change(mac_ctx,
						sme_rsp);
			} else if (eWNI_SME_GET_RSSI_REQ ==
					sme_rsp->messageType) {
				tAniGetRssiReq *pGetRssiReq =
					(tAniGetRssiReq *) msg_buf;
				if (NULL == pGetRssiReq->rssiCallback) {
					sms_log(mac_ctx, LOGE,
						FL("rssiCallback is NULL"));
					return status;
				}
				sms_log(mac_ctx, LOGW,
						FL("msg eWNI_SME_GET_RSSI_REQ is not handled by CSR in state %d. calling RSSI callback"),
						cur_state);
				((tCsrRssiCallback)(pGetRssiReq->rssiCallback))(
						pGetRssiReq->lastRSSI,
						pGetRssiReq->staId,
						pGetRssiReq->pDevContext);
			} else {
				sms_log(mac_ctx, LOGE,
					FL("Message 0x%04X is not handled by CSR state is %d session Id %d"),
					sme_rsp->messageType, cur_state,
					session_id);

				if (eWNI_SME_FT_PRE_AUTH_RSP ==
						sme_rsp->messageType) {
					sms_log(mac_ctx, LOGE,
						FL("Dequeue eSmeCommandRoam command with reason eCsrPerformPreauth"));
					csr_dequeue_roam_command(mac_ctx,
						eCsrPerformPreauth);
				} else if (eWNI_SME_REASSOC_RSP ==
						sme_rsp->messageType) {
					sms_log(mac_ctx, LOGE,
						FL("Dequeue eSmeCommandRoam command with reason eCsrSmeIssuedFTReassoc"));
					csr_dequeue_roam_command(mac_ctx,
						eCsrSmeIssuedFTReassoc);
				}
			}
			break;
		} /* switch */
#if defined(ANI_RTT_DEBUG)
	}
#endif
	return status;
}

bool csr_check_ps_ready(void *pv)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(pv);

	if (pMac->roam.sPendingCommands < 0) {
		QDF_ASSERT(pMac->roam.sPendingCommands >= 0);
		return 0;
	}
	return pMac->roam.sPendingCommands == 0;
}

bool csr_check_ps_offload_ready(void *pv, uint32_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(pv);

	QDF_ASSERT(pMac->roam.sPendingCommands >= 0);
	return pMac->roam.sPendingCommands == 0;
}

