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
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/

QDF_STATUS oem_data_oem_data_req_open(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	do {
		/* initialize all the variables to null */
		cdf_mem_set(&(pMac->oemData), sizeof(tOemDataStruct), 0);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
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
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/

QDF_STATUS oem_data_oem_data_req_close(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	do {
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				"oem_data_oem_data_req_close: Failed in oem_data_oem_data_req_close at StopTimers");
			break;
		}

		/* initialize all the variables to null */
		cdf_mem_set(&(pMac->oemData), sizeof(tOemDataStruct), 0);

	} while (0);

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn oem_data_release_oem_data_req_command
    \brief This function removes the oemDataCommand from the active list and
    and frees up any memory occupied by this
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
void oem_data_release_oem_data_req_command(tpAniSirGlobal pMac,
					   tSmeCmd *pOemDataCmd,
					   eOemDataReqStatus oemDataReqStatus)
{

	/* First take this command out of the active list */
	if (csr_ll_remove_entry
		    (&pMac->sme.smeCmdActiveList, &pOemDataCmd->Link, LL_ACCESS_LOCK)) {
		if (pOemDataCmd->u.oemDataCmd.oemDataReq.data) {
			cdf_mem_free(
			    pOemDataCmd->u.oemDataCmd.oemDataReq.data);
			pOemDataCmd->u.oemDataCmd.oemDataReq.data =
			    NULL;
		}
		cdf_mem_zero(&(pOemDataCmd->u.oemDataCmd), sizeof(tOemDataCmd));

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
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS oem_data_oem_data_req(tHalHandle hHal,
				 uint8_t sessionId,
				 tOemDataReqConfig *oemDataReqConfig,
				 uint32_t *pOemDataReqID)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *pOemDataCmd = NULL;
	tOemDataReq *cmd_req;

	do {
		if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		pMac->oemData.oemDataReqID = *(pOemDataReqID);

		pMac->oemData.oemDataReqActive = false;

		pOemDataCmd = sme_get_command_buffer(pMac);

		/* fill up the command before posting it. */
		if (pOemDataCmd) {
			pOemDataCmd->command = eSmeCommandOemDataReq;
			pOemDataCmd->u.oemDataCmd.oemDataReqID =
				pMac->oemData.oemDataReqID;


			cmd_req = &(pOemDataCmd->u.oemDataCmd.oemDataReq);
			/* set the oem data request */
			cmd_req->sessionId = sessionId;
			cmd_req->data_len =  oemDataReqConfig->data_len;
			cmd_req->data = cdf_mem_malloc(cmd_req->data_len);

			if (!cmd_req->data) {
				sms_log(pMac, LOGE, FL("memory alloc failed"));
				status = QDF_STATUS_E_NOMEM;
				break;
			}

			cdf_mem_copy((void *)(cmd_req->data),
				     (void *)(oemDataReqConfig->data),
				     cmd_req->data_len);
		} else {
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		/* now queue this command in the sme command queue */
		/* Here since this is not interacting with the csr just push the command */
		/* into the sme queue. Also push this command with the normal priority */
		sme_push_command(pMac, pOemDataCmd, false);

	} while (0);

	if (!QDF_IS_STATUS_SUCCESS(status) && pOemDataCmd) {
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
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS oem_data_send_mb_oem_data_req(tpAniSirGlobal pMac,
					 tOemDataReq *pOemDataReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirOemDataReq *pMsg;
	tCsrRoamSession *pSession =
		CSR_GET_SESSION(pMac, pOemDataReq->sessionId);
	uint16_t msgLen;

	sms_log(pMac, LOGW, "OEM_DATA: entering Function %s", __func__);

	if (!pOemDataReq) {
		sms_log(pMac, LOGE, FL("oem data req is NULL"));
		return QDF_STATUS_E_INVAL;
	}

	pMsg = cdf_mem_malloc(sizeof(*pMsg));
	if (NULL == pMsg) {
		sms_log(pMac, LOGP, FL("cdf_mem_malloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	msgLen = (uint16_t) (sizeof(*pMsg) + pOemDataReq->data_len);
	pMsg->messageType = eWNI_SME_OEM_DATA_REQ;
	pMsg->messageLen = msgLen;
	cdf_copy_macaddr(&pMsg->selfMacAddr, &pSession->selfMacAddr);
	pMsg->data_len = pOemDataReq->data_len;
	pMsg->data = pOemDataReq->data;
	/* Incoming buffer ptr saved, set to null to avoid free by caller */
	pOemDataReq->data = NULL;
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
 * Return: QDF_STATUS
 */
QDF_STATUS oem_data_process_oem_data_req_command(tpAniSirGlobal pMac,
						 tSmeCmd *pOemDataReqCmd)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

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

	if (QDF_STATUS_SUCCESS == oem_data_is_oem_data_req_allowed(pMac)) {
		sms_log(pMac, LOG1,
			FL("OEM_DATA REQ allowed in the current mode"));
		pMac->oemData.oemDataReqActive = true;
		status = oem_data_send_mb_oem_data_req(pMac,
				&(pOemDataReqCmd->u.oemDataCmd.oemDataReq));
	} else {
		sms_log(pMac, LOG1,
			FL("OEM_DATA REQ not allowed in the current mode"));
		status = QDF_STATUS_E_FAILURE;
	}

	if (!QDF_IS_STATUS_SUCCESS(status)) {
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
 * Return: QDF_STATUS.
 */
QDF_STATUS sme_handle_oem_data_rsp(tHalHandle hHal, uint8_t *pMsg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	tSirOemDataRsp *pOemDataRsp = NULL;
	tOemDataReq *req;

	pMac = PMAC_STRUCT(hHal);

	sms_log(pMac, LOG1, "%s: OEM_DATA Entering", __func__);

	do {
		if (pMsg == NULL) {
			sms_log(pMac, LOGE, "in %s msg ptr is NULL", __func__);
			status = QDF_STATUS_E_FAILURE;
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
					req =
					   &(pCommand->u.oemDataCmd.oemDataReq);
					cdf_mem_free(req->data);
					sme_release_command(pMac, pCommand);
				}
			}
		}

		pOemDataRsp = (tSirOemDataRsp *) pMsg;

		/* Send to upper layer only if rsp is from target */
		if (pOemDataRsp->target_rsp) {
			sms_log(pMac, LOG1,
				FL("received target oem data resp"));
			if (pMac->oemData.oem_data_rsp_callback != NULL)
				 pMac->oemData.oem_data_rsp_callback(
					sizeof(tOemDataRsp),
					&pOemDataRsp->oemDataRsp[0]);
		} else {
			sms_log(pMac, LOG1,
				FL("received internal oem data resp"));
		}
	} while (0);

	return status;
}

/* ---------------------------------------------------------------------------
    \fn oem_data_is_oem_data_req_allowed
    \brief This function checks if OEM DATA REQs can be performed in the
    current driver state
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS oem_data_is_oem_data_req_allowed(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t sessionId;

	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			/* co-exist with IBSS mode is not supported */
			if (csr_is_conn_state_ibss(pMac, sessionId)) {
				sms_log(pMac, LOGW,
					"OEM DATA REQ is not allowed due to IBSS exist in session %d",
					sessionId);
				status = QDF_STATUS_CSR_WRONG_STATE;
				break;
			}
		}
	}

	sms_log(pMac, LOG1, "Exiting oem_data_is_oem_data_req_allowed with status %d",
		status);

	return status;
}

#endif /*FEATURE_OEM_DATA_SUPPORT */
