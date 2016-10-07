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
#include "qdf_trace.h"

/*
 * Cookie for SYS messages.  Note that anyone posting a SYS Message
 * has to write the COOKIE in the reserved field of the message.  The
 * SYS Module relies on this COOKIE
 */
#define SYS_MSG_COOKIE      0xFACE

/* SYS stop timeout 30 seconds */
#define SYS_STOP_TIMEOUT (30000)
static qdf_event_t g_stop_evt;

/**
 * sys_build_message_header() - to build the sys message header
 * @sysMsgId: message id
 * @pMsg: pointer to message context
 *
 * This API is used to build the sys message header.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sys_build_message_header(SYS_MSG_ID sysMsgId, cds_msg_t *pMsg)
{
	pMsg->type = sysMsgId;
	pMsg->reserved = SYS_MSG_COOKIE;

	return QDF_STATUS_SUCCESS;
}

/**
 * sys_stop_complete_cb() - a callback when system stop completes
 * @pUserData: pointer to user provided data context
 *
 * this callback is used once system stop is completed.
 *
 * Return: none
 */
#ifdef QDF_ENABLE_TRACING
static void sys_stop_complete_cb(void *pUserData)
{
	qdf_event_t *pStopEvt = (qdf_event_t *) pUserData;
	QDF_STATUS qdf_status = qdf_event_set(pStopEvt);

	QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));

}
#else
static void sys_stop_complete_cb(void *pUserData)
{
	return;
}
#endif

/**
 * sys_stop() - To post stop message to system module
 * @p_cds_context:  pointer to cds context
 *
 * This API is used post a stop message to system module
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sys_stop(v_CONTEXT_t p_cds_context)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	cds_msg_t sysMsg;

	/* Initialize the stop event */
	qdf_status = qdf_event_create(&g_stop_evt);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;

	/* post a message to SYS module in MC to stop SME and MAC */
	sys_build_message_header(SYS_MSG_ID_MC_STOP, &sysMsg);

	/* Save the user callback and user data */
	sysMsg.callback = sys_stop_complete_cb;
	sysMsg.bodyptr = (void *)&g_stop_evt;

	/* post the message.. */
	qdf_status = cds_mq_post_message(CDS_MQ_ID_SYS, &sysMsg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		qdf_status = QDF_STATUS_E_BADMSG;

	qdf_status = qdf_wait_single_event(&g_stop_evt, SYS_STOP_TIMEOUT);
	QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));

	qdf_status = qdf_event_destroy(&g_stop_evt);
	QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));

	return qdf_status;
}

/**
 * sys_mc_process_msg() - to process system mc thread messages
 * @p_cds_context: pointer to cds context
 * @pMsg: message pointer
 *
 * This API is used to process the message
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sys_mc_process_msg(v_CONTEXT_t p_cds_context, cds_msg_t *pMsg)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	qdf_mc_timer_callback_t timerCB;
	tpAniSirGlobal mac_ctx;
	void *hHal;

	if (NULL == pMsg) {
		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer to cds_msg_t", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * All 'new' SYS messages are identified by a cookie in the reserved
	 * field of the message as well as the message type.  This prevents
	 * the possibility of overlap in the message types defined for new
	 * SYS messages with the 'legacy' message types.  The legacy messages
	 * will not have this cookie in the reserved field
	 */
	if (SYS_MSG_COOKIE == pMsg->reserved) {
		/* Process all the new SYS messages.. */
		switch (pMsg->type) {
		case SYS_MSG_ID_MC_START:
			/*
			 * Handling for this message is not needed now so adding
			 * debug print and QDF_ASSERT
			 */
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
				"Rx SYS_MSG_ID_MC_START msgType= %d [0x%08x]",
				pMsg->type, pMsg->type);
			QDF_ASSERT(0);
			break;

		case SYS_MSG_ID_MC_STOP:
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_INFO,
				"Processing SYS MC STOP");

			/* get the HAL context... */
			hHal = cds_get_context(QDF_MODULE_ID_PE);
			if (NULL == hHal) {
				QDF_TRACE(QDF_MODULE_ID_SYS,
					QDF_TRACE_LEVEL_ERROR,
					"%s: Invalid hHal", __func__);
			} else {
				qdf_status = sme_stop(hHal,
						HAL_STOP_TYPE_SYS_DEEP_SLEEP);
				QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));
				qdf_status = mac_stop(hHal,
						HAL_STOP_TYPE_SYS_DEEP_SLEEP);
				QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));

				((sysResponseCback) pMsg->callback)(
						(void *)pMsg->bodyptr);

				qdf_status = QDF_STATUS_SUCCESS;
			}
			break;

		case SYS_MSG_ID_MC_THR_PROBE:
			/*
			 * Process MC thread probe.  Just callback to the
			 * function that is in the message.
			 */
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
				"Rx SYS_MSG_ID_MC_THR_PROBE msgType=%d[0x%08x]",
				pMsg->type, pMsg->type);
			break;

		case SYS_MSG_ID_MC_TIMER:
			timerCB = pMsg->callback;
			if (NULL != timerCB)
				timerCB(pMsg->bodyptr);
			break;

		case SYS_MSG_ID_FTM_RSP:
			hHal = cds_get_context(QDF_MODULE_ID_PE);
			if (NULL == hHal) {
				QDF_TRACE(QDF_MODULE_ID_SYS,
						QDF_TRACE_LEVEL_ERROR,
						FL("Invalid hal"));
				qdf_mem_free(pMsg->bodyptr);
				break;
			}
			mac_ctx = PMAC_STRUCT(hHal);
			if (NULL == mac_ctx) {
				QDF_TRACE(QDF_MODULE_ID_SYS,
						QDF_TRACE_LEVEL_ERROR,
						FL("Invalid mac context"));
				qdf_mem_free(pMsg->bodyptr);
				break;
			}
			if (NULL == mac_ctx->ftm_msg_processor_callback) {
				QDF_TRACE(QDF_MODULE_ID_SYS,
						QDF_TRACE_LEVEL_ERROR,
						FL("callback pointer is NULL"));
				qdf_mem_free(pMsg->bodyptr);
				break;
			}
			mac_ctx->ftm_msg_processor_callback(
					(void *)pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
			break;

		default:
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
				"Unknown message type msgType= %d [0x%08x]",
				pMsg->type, pMsg->type);
			break;

		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SYS,
				QDF_TRACE_LEVEL_ERROR,
				"Rx SYS unknown MC msgtype= %d [0x%08X]",
				pMsg->type, pMsg->type);
		QDF_ASSERT(0);
		qdf_status = QDF_STATUS_E_BADMSG;

		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
	}
	return qdf_status;
}

/**
 * sys_process_mmh_msg() - this api to process mmh message
 * @pMac: pointer to mac context
 * @pMsg: pointer to message
 *
 * This API is used to process mmh message
 *
 * Return: none
 */
void sys_process_mmh_msg(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
	CDS_MQ_ID targetMQ = CDS_MQ_ID_SYS;

	/*
	 * The body of this pMsg is a tSirMbMsg
	 * Contrary to previous generation, we cannot free it here!
	 * It is up to the callee to free it
	 */
	if (NULL == pMsg) {
		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
				"NULL Message Pointer");
		QDF_ASSERT(0);
		return;
	}

	switch (pMsg->type) {
	/*
	 * Following messages are routed to SYS
	 */
	case WNI_CFG_DNLD_REQ:
	case WNI_CFG_DNLD_CNF:
		/* Forward this message to the SYS module */
		targetMQ = CDS_MQ_ID_SYS;
		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
			"Handling for the Message ID %d is removed in SYS",
			pMsg->type);
		QDF_ASSERT(0);
		break;

		/*
		 * Following messages are routed to HAL
		 */
	case WNI_CFG_DNLD_RSP:
		/* Forward this message to the HAL module */
		targetMQ = CDS_MQ_ID_WMA;
		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
			"Handling for the Message ID %d is removed as no HAL",
			pMsg->type);

		QDF_ASSERT(0);
		break;

	case WNI_CFG_GET_REQ:
	case WNI_CFG_SET_REQ:
	case WNI_CFG_SET_REQ_NO_RSP:
	case eWNI_SME_SYS_READY_IND:
		/* Forward this message to the PE module */
		targetMQ = CDS_MQ_ID_PE;
		break;

	case WNI_CFG_GET_RSP:
	case WNI_CFG_SET_CNF:
		/* Forward this message to the SME module */
		targetMQ = CDS_MQ_ID_SME;
		break;

	default:
		if ((pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN)
				&& (pMsg->type <= eWNI_SME_MSG_TYPES_END)) {
			targetMQ = CDS_MQ_ID_SME;
			break;
		}

		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
			"Message of ID %d is not yet handled by SYS",
			pMsg->type);
		QDF_ASSERT(0);
	}

	/*
	 * Post now the message to the appropriate module for handling
	 */
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(targetMQ,
					(cds_msg_t *) pMsg)) {
		/*
		 * Caller doesn't allocate memory for the pMsg.
		 * It allocate memory for bodyptr free the mem and return
		 */
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
	}

}

/**
 * wlan_sys_probe() - API to post MC thread probe
 *
 * This API will be used send probe message
 *
 * Return: none
 */
void wlan_sys_probe(void)
{
	cds_msg_t cds_message;

	cds_message.reserved = SYS_MSG_COOKIE;
	cds_message.type = SYS_MSG_ID_MC_THR_PROBE;
	cds_message.bodyptr = NULL;
	cds_mq_post_message(CDS_MQ_ID_SYS, &cds_message);
}
