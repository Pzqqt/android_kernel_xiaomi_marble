/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_qct_wma_legacy.c
 *
 * This software unit holds the implementation of the WLAN Device Adaptation
 * Layer for the legacy functionalities that were part of the old HAL.
 *
 * The functions externalized by this module are to be called ONLY by other
 * WLAN modules that properly register with the Transport Layer initially.
 *
 */

/* Standard include files */
/* Application Specific include files */
#include "lim_api.h"
#include "cfg_api.h"
#include "wma.h"
#include "sme_power_save_api.h"
/* Locally used Defines */

#define HAL_MMH_MB_MSG_TYPE_MASK    0xFF00

/**
 * wma_post_ctrl_msg() - Posts WMA messages to MC thread
 * @pMac: MAC parameters structure
 * @pMsg: pointer with message
 *
 * Return: Success or Failure
 */

tSirRetStatus wma_post_ctrl_msg(tpAniSirGlobal pMac, struct scheduler_msg *pMsg)
{
	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_msg(QDF_MODULE_ID_WMA,  pMsg))
		return eSIR_FAILURE;
	else
		return eSIR_SUCCESS;
}

/**
 * wma_post_cfg_msg() - Posts MNT messages to gSirMntMsgQ
 * @pMac: MAC parameters structure
 * @pMsg: A pointer to the msg
 *
 * Return: Success or Failure
 */

static tSirRetStatus wma_post_cfg_msg(tpAniSirGlobal pMac,
				      struct scheduler_msg *pMsg)
{
	tSirRetStatus rc = eSIR_SUCCESS;

	do {
		/*
		 *For Windows based MAC, instead of posting message to different
		 * queues we will call the handler routines directly
		 */

		cfg_process_mb_msg(pMac, (tSirMbMsg *) pMsg->bodyptr);
		rc = eSIR_SUCCESS;
	} while (0);

	return rc;
}

/**
 * u_mac_post_ctrl_msg() - post ctrl msg
 * @pMb: A pointer to the maibox message
 *
 * Forwards the completely received message to the respective
 * modules for further processing.
 *
 * NOTE:
 *  This function has been moved to the API file because for MAC running
 *  on Windows host, the host module will call this routine directly to
 *  send any mailbox messages. Making this function an API makes sure that
 *  outside world (any module outside MMH) only calls APIs to use MMH
 *  services and not an internal function.
 *
 * Return: success/error code
 */

tSirRetStatus u_mac_post_ctrl_msg(void *pSirGlobal, tSirMbMsg *pMb)
{
	struct scheduler_msg msg = {0};
	tSirRetStatus status = eSIR_SUCCESS;
	tpAniSirGlobal pMac = (tpAniSirGlobal) pSirGlobal;

	tSirMbMsg *pMbLocal;
	msg.type = pMb->type;
	msg.bodyval = 0;

	pMbLocal = qdf_mem_malloc(pMb->msgLen);
	if (!pMbLocal) {
		WMA_LOGE("Memory allocation failed! Can't send 0x%x\n",
			 msg.type);
		return eSIR_MEM_ALLOC_FAILED;
	}

	qdf_mem_copy((void *)pMbLocal, (void *)pMb, pMb->msgLen);
	msg.bodyptr = pMbLocal;

	switch (msg.type & HAL_MMH_MB_MSG_TYPE_MASK) {
	case WMA_MSG_TYPES_BEGIN:       /* Posts a message to the HAL MsgQ */
		status = wma_post_ctrl_msg(pMac, &msg);
		break;

	case SIR_LIM_MSG_TYPES_BEGIN:   /* Posts a message to the LIM MsgQ */
		status = lim_post_msg_api(pMac, &msg);
		break;

	case SIR_CFG_MSG_TYPES_BEGIN:   /* Posts a message to the CFG MsgQ */
		status = wma_post_cfg_msg(pMac, &msg);
		break;

	case SIR_PMM_MSG_TYPES_BEGIN:   /* Posts a message to the LIM MsgQ */
		status = sme_post_pe_message(pMac, &msg);
		break;

	case SIR_PTT_MSG_TYPES_BEGIN:
		qdf_mem_free(msg.bodyptr);
		break;

	default:
		WMA_LOGD("Unknown message type = 0x%X\n", msg.type);
		qdf_mem_free(msg.bodyptr);
		return eSIR_FAILURE;
	}

	if (status != eSIR_SUCCESS)
		qdf_mem_free(msg.bodyptr);

	return status;

} /* u_mac_post_ctrl_msg() */

/**
 * umac_send_mb_message_to_mac() - post a message to a message queue
 * @pBuf: Pointer to buffer allocated by caller
 *
 * Return: qdf status
 */
QDF_STATUS umac_send_mb_message_to_mac(void *pBuf)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	tSirRetStatus sirStatus;
	void *hHal;

	hHal = cds_get_context(QDF_MODULE_ID_SME);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid hHal", __func__);
	} else {
		sirStatus = u_mac_post_ctrl_msg(hHal, pBuf);
		if (eSIR_SUCCESS == sirStatus)
			qdf_ret_status = QDF_STATUS_SUCCESS;
	}

	qdf_mem_free(pBuf);

	return qdf_ret_status;
}
