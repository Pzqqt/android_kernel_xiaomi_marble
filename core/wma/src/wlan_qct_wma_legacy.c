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

tSirRetStatus wma_post_ctrl_msg(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
	if (QDF_STATUS_SUCCESS !=
	    cds_mq_post_message(CDS_MQ_ID_WMA, (cds_msg_t *) pMsg))
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

static tSirRetStatus wma_post_cfg_msg(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
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
	tSirMsgQ msg;
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
		wma_post_ctrl_msg(pMac, &msg);
		break;

	case SIR_LIM_MSG_TYPES_BEGIN:   /* Posts a message to the LIM MsgQ */
		lim_post_msg_api(pMac, &msg);
		break;

	case SIR_CFG_MSG_TYPES_BEGIN:   /* Posts a message to the CFG MsgQ */
		wma_post_cfg_msg(pMac, &msg);
		break;

	case SIR_PMM_MSG_TYPES_BEGIN:   /* Posts a message to the LIM MsgQ */
		sme_post_pe_message(pMac, &msg);
		break;

	case SIR_PTT_MSG_TYPES_BEGIN:
		qdf_mem_free(msg.bodyptr);
		break;

	default:
		WMA_LOGD("Unknown message type = 0x%X\n", msg.type);
		qdf_mem_free(msg.bodyptr);
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;

} /* u_mac_post_ctrl_msg() */

