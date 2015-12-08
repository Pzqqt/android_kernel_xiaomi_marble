/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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

#ifdef FEATURE_OEM_DATA_SUPPORT
/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *

    \file oem_data_api.c

    Implementation for the OEM DATA REQ/RSP interfaces.
   ========================================================================== */
#include "ani_global.h"
#include "oem_data_api.h"
#include "cds_mq.h"
#include "sme_inside.h"
#include "sms_debug.h"
#include "cdf_util.h"

#include "csr_support.h"

#include "host_diag_core_log.h"
#include "host_diag_core_event.h"

/* ---------------------------------------------------------------------------
    \fn oem_data_oem_data_req_open
    \brief This function must be called before any API call to (OEM DATA REQ/RSP module)
    \return CDF_STATUS
   -------------------------------------------------------------------------------*/

CDF_STATUS oem_data_oem_data_req_open(tHalHandle hHal)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	do {
		/* initialize all the variables to null */
		cdf_mem_set(&(pMac->oemData), sizeof(tOemDataStruct), 0);
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				"oem_data_oem_data_req_open: Cannot allocate memory for the timer function");
			break;
		}
	} while (0);

	return status;
}

/* ---------------------------------------------------------------------------
    \fn oem_data_oem_data_req_close
    \brief This function must be called before closing the csr module
    \return CDF_STATUS
   -------------------------------------------------------------------------------*/

CDF_STATUS oem_data_oem_data_req_close(tHalHandle hHal)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	do {
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				"oem_data_oem_data_req_close: Failed in oem_data_oem_data_req_close at StopTimers");
			break;
		}

		/* initialize all the variables to null */
		cdf_mem_set(&(pMac->oemData), sizeof(tOemDataStruct), 0);
	} while (0);

	return CDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn oem_data_release_oem_data_req_command
    \brief This function removes the oemDataCommand from the active list and
           and frees up any memory occupied by this
    \return CDF_STATUS
   -------------------------------------------------------------------------------*/
void oem_data_release_oem_data_req_command(tpAniSirGlobal pMac,
					   tSmeCmd *pOemDataCmd,
					   eOemDataReqStatus oemDataReqStatus)
{

	/* First take this command out of the active list */
	if (csr_ll_remove_entry
		    (&pMac->sme.smeCmdActiveList, &pOemDataCmd->Link, LL_ACCESS_LOCK)) {
		cdf_mem_set(&(pOemDataCmd->u.oemDataCmd), sizeof(tOemDataCmd),
			    0);

		/* Now put this command back on the avilable command list */
		sme_release_command(pMac, pOemDataCmd);
	} else {
		sms_log(pMac, LOGE,
			"OEM_DATA: **************** oem_data_release_oem_data_req_command cannot release the command");
	}
}

/* ---------------------------------------------------------------------------
    \fn oem_data_oem_data_req
    \brief Request an OEM DATA RSP
    \param sessionId - Id of session to be used
    \param pOemDataReqID - pointer to an object to get back the request ID
    \param callback - a callback function that is called upon finish
    \param pContext - a pointer passed in for the callback
    \return CDF_STATUS
   -------------------------------------------------------------------------------*/
CDF_STATUS oem_data_oem_data_req(tHalHandle hHal,
				 uint8_t sessionId,
				 tOemDataReqConfig *oemDataReqConfig,
				 uint32_t *pOemDataReqID,
				 oem_data_oem_data_reqCompleteCallback callback,
				 void *pContext)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *pOemDataCmd = NULL;

	do {
		if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = CDF_STATUS_E_FAILURE;
			break;
		}

		pMac->oemData.oemDataReqConfig.sessionId = sessionId;
		pMac->oemData.callback = callback;
		pMac->oemData.pContext = pContext;
		pMac->oemData.oemDataReqID = *(pOemDataReqID);

		cdf_mem_copy((void *)(pMac->oemData.oemDataReqConfig.
				      oemDataReq),
			     (void *)(oemDataReqConfig->oemDataReq),
			     OEM_DATA_REQ_SIZE);

		pMac->oemData.oemDataReqActive = false;

		pOemDataCmd = sme_get_command_buffer(pMac);

		/* fill up the command before posting it. */
		if (pOemDataCmd) {
			pOemDataCmd->command = eSmeCommandOemDataReq;
			pOemDataCmd->u.oemDataCmd.callback = callback;
			pOemDataCmd->u.oemDataCmd.pContext = pContext;
			pOemDataCmd->u.oemDataCmd.oemDataReqID =
				pMac->oemData.oemDataReqID;

			/* set the oem data request */
			pOemDataCmd->u.oemDataCmd.oemDataReq.sessionId =
				pMac->oemData.oemDataReqConfig.sessionId;
			cdf_mem_copy((void *)(pOemDataCmd->u.oemDataCmd.
					      oemDataReq.oemDataReq),
				     (void *)(pMac->oemData.oemDataReqConfig.
					      oemDataReq), OEM_DATA_REQ_SIZE);
		} else {
			status = CDF_STATUS_E_FAILURE;
			break;
		}

		/* now queue this command in the sme command queue */
		/* Here since this is not interacting with the csr just push the command */
		/* into the sme queue. Also push this command with the normal priority */
		sme_push_command(pMac, pOemDataCmd, false);

	} while (0);

	if (!CDF_IS_STATUS_SUCCESS(status) && pOemDataCmd) {
		oem_data_release_oem_data_req_command(pMac, pOemDataCmd,
						      eOEM_DATA_REQ_FAILURE);
		pMac->oemData.oemDataReqActive = false;
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn oem_data_send_mb_oem_data_req
    \brief Request an OEM DATA REQ to be passed down to PE
    \param pMac:
    \param pOemDataReq: Pointer to the oem data request
    \return CDF_STATUS
   -------------------------------------------------------------------------------*/
CDF_STATUS oem_data_send_mb_oem_data_req(tpAniSirGlobal pMac,
					 tOemDataReq *pOemDataReq)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	tSirOemDataReq *pMsg;
	uint16_t msgLen;
	tCsrRoamSession *pSession =
		CSR_GET_SESSION(pMac, pOemDataReq->sessionId);

	sms_log(pMac, LOGW, "OEM_DATA: entering Function %s", __func__);

	msgLen = (uint16_t) (sizeof(tSirOemDataReq));

	pMsg = cdf_mem_malloc(msgLen);
	if (NULL == pMsg) {
		sms_log(pMac, LOGP, FL("cdf_mem_malloc failed"));
		return CDF_STATUS_E_NOMEM;
	}
	cdf_mem_set(pMsg, msgLen, 0);
	pMsg->messageType = eWNI_SME_OEM_DATA_REQ;
	pMsg->messageLen = msgLen;
	cdf_copy_macaddr(&pMsg->selfMacAddr, &pSession->selfMacAddr);
	cdf_mem_copy(pMsg->oemDataReq, pOemDataReq->oemDataReq,
		     OEM_DATA_REQ_SIZE);
	sms_log(pMac, LOGW, "OEM_DATA: sending message to pe%s", __func__);
	status = cds_send_mb_message_to_mac(pMsg);

	sms_log(pMac, LOGW, "OEM_DATA: exiting Function %s", __func__);

	return status;
}

/**
 * oem_data_process_oem_data_req_command() - process oem data request command
 * @pMac: Mac context
 * @pOemDataReqCmd: pointer to oem data request command
 *
 * This function is called by the sme_process_command when the case hits
 * eSmeCommandOemDataReq.
 *
 * Return: CDF_STATUS
 */
CDF_STATUS oem_data_process_oem_data_req_command(tpAniSirGlobal pMac,
						 tSmeCmd *pOemDataReqCmd)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;

	/* check if the system is in proper mode of operation for
	 * oem data req/rsp to be functional. Currently, concurrency is not
	 * supported and the driver must be operational only as
	 * STA for oem data req/rsp to be functional. We return an invalid
	 * mode flag if it is operational as any one of the following
	 * in any of the active sessions
	 * 1. AP Mode
	 * 2. IBSS Mode
	 * 3. BTAMP Mode
	 */

	if (CDF_STATUS_SUCCESS == oem_data_is_oem_data_req_allowed(pMac)) {
		sms_log(pMac, LOG1,
			FL("OEM_DATA REQ allowed in the current mode"));
		pMac->oemData.oemDataReqActive = true;
		status = oem_data_send_mb_oem_data_req(pMac,
				&(pOemDataReqCmd->u.oemDataCmd.oemDataReq));
	} else {
		sms_log(pMac, LOG1,
			FL("OEM_DATA REQ not allowed in the current mode"));
		status = CDF_STATUS_E_FAILURE;
	}

	if (!CDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOG1, FL("OEM_DATA Failure, Release command"));
		oem_data_release_oem_data_req_command(pMac, pOemDataReqCmd,
				eOEM_DATA_REQ_INVALID_MODE);
		pMac->oemData.oemDataReqActive = false;
	}

	return status;
}

/**
 * sme_handle_oem_data_rsp() - processes the oem data response
 * @hHal:                      Handle returned by mac_open.
 * @pMsg:                      Pointer to the pSirOemDataRsp
 *
 * This function processes the oem data response obtained from the PE
 *
 * Return: CDF_STATUS.
 */
CDF_STATUS sme_handle_oem_data_rsp(tHalHandle hHal, uint8_t *pMsg)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	tSirOemDataRsp *pOemDataRsp = NULL;
	uint32_t *msgSubType;

	pMac = PMAC_STRUCT(hHal);

	sms_log(pMac, LOG1, "%s: OEM_DATA Entering", __func__);

	do {
		if (pMsg == NULL) {
			sms_log(pMac, LOGE, "in %s msg ptr is NULL", __func__);
			status = CDF_STATUS_E_FAILURE;
			break;
		}

		/* In this case, there can be multiple OEM Data Responses for one
		 * OEM Data request, SME does not peek into data response so SME
		 * can not know which response is the last one. So SME clears active
		 * request command on receiving first response and thereafter SME
		 * passes each sunsequent response to upper user layer.
		 */
		pEntry =
			csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
		if (pEntry) {
			pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			if (eSmeCommandOemDataReq == pCommand->command) {
				if (csr_ll_remove_entry
					    (&pMac->sme.smeCmdActiveList,
					    &pCommand->Link, LL_ACCESS_LOCK)) {
					cdf_mem_set(&(pCommand->u.oemDataCmd),
						    sizeof(tOemDataCmd), 0);
					sme_release_command(pMac, pCommand);
				}
			}
		}

		pOemDataRsp = (tSirOemDataRsp *) pMsg;

		/* check if message is to be forwarded to oem application or not */
		msgSubType = (uint32_t *) (&pOemDataRsp->oemDataRsp[0]);
		if (*msgSubType != OEM_MESSAGE_SUBTYPE_INTERNAL) {
			CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
				  "%s: calling send_oem_data_rsp_msg, msgSubType(0x%x)",
				  __func__, *msgSubType);
			if (pMac->oemData.oem_data_rsp_callback != NULL) {
				pMac->oemData.oem_data_rsp_callback(
						sizeof(tOemDataRsp),
						&pOemDataRsp->oemDataRsp[0]);
			}
		} else
			CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
				  "%s: received internal oem data resp, msgSubType (0x%x)",
				  __func__, *msgSubType);
	} while (0);

	return status;
}

/* ---------------------------------------------------------------------------
    \fn oem_data_is_oem_data_req_allowed
    \brief This function checks if OEM DATA REQs can be performed in the
           current driver state
    \return CDF_STATUS
   -------------------------------------------------------------------------------*/
CDF_STATUS oem_data_is_oem_data_req_allowed(tHalHandle hHal)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	uint32_t sessionId;

	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			/* co-exist with IBSS mode is not supported */
			if (csr_is_conn_state_ibss(pMac, sessionId)) {
				sms_log(pMac, LOGW,
					"OEM DATA REQ is not allowed due to IBSS exist in session %d",
					sessionId);
				status = CDF_STATUS_CSR_WRONG_STATE;
				break;
			}
		}
	}

	sms_log(pMac, LOG1, "Exiting oem_data_is_oem_data_req_allowed with status %d",
		status);

	return (status);
}

#endif /*FEATURE_OEM_DATA_SUPPORT */
