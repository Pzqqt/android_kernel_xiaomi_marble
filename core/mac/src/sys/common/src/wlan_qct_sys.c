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

#include <wlan_qct_sys.h>
#include <cds_api.h>
#include <sir_types.h>           /* needed for tSirRetStatus */
#include <sir_params.h>          /* needed for tSirMbMsg */
#include <sir_api.h>             /* needed for SIR_... message types */
#include <wni_api.h>             /* needed for WNI_... message types */
#include "ani_global.h"
#include "wma_types.h"
#include "sme_api.h"
#include "mac_init_api.h"

/* Cookie for SYS messages.  Note that anyone posting a SYS Message
 * has to write the COOKIE in the reserved field of the message.  The
 * SYS Module relies on this COOKIE
 */
#define SYS_MSG_COOKIE      0xFACE

/* SYS stop timeout 30 seconds */
#define SYS_STOP_TIMEOUT (30000)

static cdf_event_t g_stop_evt;

CDF_STATUS sys_build_message_header(SYS_MSG_ID sysMsgId, cds_msg_t *pMsg)
{
	pMsg->type = sysMsgId;
	pMsg->reserved = SYS_MSG_COOKIE;

	return (CDF_STATUS_SUCCESS);
}

void sys_stop_complete_cb(void *pUserData) {
	cdf_event_t *pStopEvt = (cdf_event_t *) pUserData;
	CDF_STATUS cdf_status;
/*-------------------------------------------------------------------------*/

	cdf_status = cdf_event_set(pStopEvt);
	CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));

} /* cds_sys_stop_complete_cback() */

CDF_STATUS sys_stop(v_CONTEXT_t p_cds_context)
{
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	cds_msg_t sysMsg;

	/* Initialize the stop event */
	cdf_status = cdf_event_init(&g_stop_evt);

	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		return cdf_status;
	}

	/* post a message to SYS module in MC to stop SME and MAC */
	sys_build_message_header(SYS_MSG_ID_MC_STOP, &sysMsg);

	/* Save the user callback and user data */
	/* finished. */
	sysMsg.callback = sys_stop_complete_cb;
	sysMsg.bodyptr = (void *)&g_stop_evt;

	/* post the message.. */
	cdf_status = cds_mq_post_message(CDS_MQ_ID_SYS, &sysMsg);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		cdf_status = CDF_STATUS_E_BADMSG;
	}

	cdf_status = cdf_wait_single_event(&g_stop_evt, SYS_STOP_TIMEOUT);
	CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));

	cdf_status = cdf_event_destroy(&g_stop_evt);
	CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));

	return (cdf_status);
}

CDF_STATUS sys_mc_process_msg(v_CONTEXT_t p_cds_context, cds_msg_t *pMsg)
{
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	void *hHal;

	if (NULL == pMsg) {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer to cds_msg_t", __func__);
		return CDF_STATUS_E_INVAL;
	}

	/* All 'new' SYS messages are identified by a cookie in the reserved
	 * field of the message as well as the message type.  This prevents
	 * the possibility of overlap in the message types defined for new
	 * SYS messages with the 'legacy' message types.  The legacy messages
	 * will not have this cookie in the reserved field
	 */
	if (SYS_MSG_COOKIE == pMsg->reserved) {
		/* Process all the new SYS messages.. */
		switch (pMsg->type) {
		case SYS_MSG_ID_MC_START:
		{
			/* Handling for this message is not needed now so adding
			 *debug print and CDF_ASSERT*/
			CDF_TRACE(CDF_MODULE_ID_SYS,
				  CDF_TRACE_LEVEL_ERROR,
				  " Received SYS_MSG_ID_MC_START message msgType= %d [0x%08x]",
				  pMsg->type, pMsg->type);
			CDF_ASSERT(0);
			break;
		}

		case SYS_MSG_ID_MC_STOP:
		{
			CDF_TRACE(CDF_MODULE_ID_SYS,
				  CDF_TRACE_LEVEL_INFO,
				  "Processing SYS MC STOP");

			/* get the HAL context... */
			hHal = cds_get_context(CDF_MODULE_ID_PE);
			if (NULL == hHal) {
				CDF_TRACE(CDF_MODULE_ID_SYS,
					  CDF_TRACE_LEVEL_ERROR,
					  "%s: Invalid hHal", __func__);
			} else {
				cdf_status =
					sme_stop(hHal,
						 HAL_STOP_TYPE_SYS_DEEP_SLEEP);
				CDF_ASSERT(CDF_IS_STATUS_SUCCESS
						   (cdf_status));

				cdf_status =
					mac_stop(hHal,
						 HAL_STOP_TYPE_SYS_DEEP_SLEEP);
				CDF_ASSERT(CDF_IS_STATUS_SUCCESS
						   (cdf_status));

				((sysResponseCback) pMsg->
				 callback)((void *)pMsg->bodyptr);

				cdf_status = CDF_STATUS_SUCCESS;
			}
			break;
		}

		/* Process MC thread probe.  Just callback to the */
		/* function that is in the message. */
		case SYS_MSG_ID_MC_THR_PROBE:
		{
			CDF_TRACE(CDF_MODULE_ID_SYS,
				  CDF_TRACE_LEVEL_ERROR,
				  " Received SYS_MSG_ID_MC_THR_PROBE message msgType = %d [0x%08x]",
				  pMsg->type, pMsg->type);
			break;
		}

		case SYS_MSG_ID_MC_TIMER:
		{
			cdf_mc_timer_callback_t timerCB =
				pMsg->callback;

			if (NULL != timerCB) {
				timerCB(pMsg->bodyptr);
			}
			break;
		}
		case SYS_MSG_ID_FTM_RSP:
		{
			tpAniSirGlobal mac_ctx = NULL;
			hHal = cds_get_context(CDF_MODULE_ID_PE);
			if (NULL == hHal) {
				CDF_TRACE(CDF_MODULE_ID_SYS,
					  CDF_TRACE_LEVEL_ERROR,
					  FL("Invalid hal"));
				cdf_mem_free(pMsg->bodyptr);
				break;
			}
			mac_ctx = PMAC_STRUCT(hHal);
			if (NULL == mac_ctx) {
				CDF_TRACE(CDF_MODULE_ID_SYS,
					  CDF_TRACE_LEVEL_ERROR,
					  FL("Invalid mac context"));
				cdf_mem_free(pMsg->bodyptr);
				break;
			}
			if (NULL == mac_ctx->ftm_msg_processor_callback) {
				CDF_TRACE(CDF_MODULE_ID_SYS,
					  CDF_TRACE_LEVEL_ERROR,
					  FL("callback pointer is NULL"));
				cdf_mem_free(pMsg->bodyptr);
				break;
			}
			mac_ctx->ftm_msg_processor_callback(
							(void *)pMsg->bodyptr);
			cdf_mem_free(pMsg->bodyptr);
			break;
		}

		default:
		{
			CDF_TRACE(CDF_MODULE_ID_SYS,
				  CDF_TRACE_LEVEL_ERROR,
				  "Unknown message type in sys_mc_process_msg() msgType= %d [0x%08x]",
				  pMsg->type, pMsg->type);
			break;
		}

		} /* end switch on message type */

	} /* end if cookie set */
	else {
		/* Process all 'legacy' messages */
		switch (pMsg->type) {

		default:
		{
			CDF_ASSERT(0);

			CDF_TRACE(CDF_MODULE_ID_SYS,
				  CDF_TRACE_LEVEL_ERROR,
				  "Received SYS message cookie with unidentified "
				  "MC message type= %d [0x%08X]",
				  pMsg->type, pMsg->type);

			cdf_status = CDF_STATUS_E_BADMSG;
			if (pMsg->bodyptr)
				cdf_mem_free(pMsg->bodyptr);
			break;
		}
		} /* end switch on pMsg->type */
	} /* end else */

	return (cdf_status);
}


void sys_process_mmh_msg(tpAniSirGlobal pMac, tSirMsgQ *pMsg) {
	CDS_MQ_ID targetMQ = CDS_MQ_ID_SYS;
/*-------------------------------------------------------------------------*/
	/*
	** The body of this pMsg is a tSirMbMsg
	** Contrary to Gen4, we cannot free it here!
	** It is up to the callee to free it
	*/

	if (NULL == pMsg) {
		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "NULL Message Pointer");
		CDF_ASSERT(0);
		return;
	}

	switch (pMsg->type) {
	/*
	** Following messages are routed to SYS
	*/
	case WNI_CFG_DNLD_REQ:
	case WNI_CFG_DNLD_CNF:
	{
		/* Forward this message to the SYS module */
		targetMQ = CDS_MQ_ID_SYS;

		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "Handling for the Message ID %d is removed in SYS\r\n",
			  pMsg->type);

		CDF_ASSERT(0);
		break;
	}

	/*
	** Following messages are routed to HAL
	*/
	case WNI_CFG_DNLD_RSP:
	{
		/* Forward this message to the HAL module */
		targetMQ = CDS_MQ_ID_WMA;

		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "Handling for the Message ID %d is removed as there is no HAL \r\n",
			  pMsg->type);

		CDF_ASSERT(0);
		break;
	}

	case WNI_CFG_GET_REQ:
	case WNI_CFG_SET_REQ:
	case WNI_CFG_SET_REQ_NO_RSP:
	case eWNI_SME_SYS_READY_IND:
	{
		/* Forward this message to the PE module */
		targetMQ = CDS_MQ_ID_PE;
		break;
	}

	case WNI_CFG_GET_RSP:
	case WNI_CFG_SET_CNF:
	{
		/* Forward this message to the SME module */
		targetMQ = CDS_MQ_ID_SME;
		break;
	}

	default:
	{

		if ((pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN)
		    && (pMsg->type <= eWNI_SME_MSG_TYPES_END)) {
			targetMQ = CDS_MQ_ID_SME;
			break;
		}

		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "Message of ID %d is not yet handled by SYS\r\n",
			  pMsg->type);

		CDF_ASSERT(0);
	}

	}

	/*
	** Post now the message to the appropriate module for handling
	*/
	if (CDF_STATUS_SUCCESS !=
	    cds_mq_post_message(targetMQ, (cds_msg_t *) pMsg)) {
		/* Caller doesn't allocate memory for the pMsg. It allocate memory for bodyptr */
		/* free the mem and return */
		if (pMsg->bodyptr) {
			cdf_mem_free(pMsg->bodyptr);
		}
	}

} /* sys_process_mmh_msg() */

void wlan_sys_probe(void)
{
	cds_msg_t cds_message;

	cds_message.reserved = SYS_MSG_COOKIE;
	cds_message.type = SYS_MSG_ID_MC_THR_PROBE;
	cds_message.bodyptr = NULL;

	cds_mq_post_message(CDS_MQ_ID_SYS, &cds_message);

	return;
}
