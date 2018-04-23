/*
 * Copyright (c) 2012, 2014-2017 The Linux Foundation. All rights reserved.
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
 * This file contains the source code for composing and sending messages
 * to host.
 *
 * Author:      Kevin Nguyen
 * Date:        04/09/02
 * History:-
 * 04/09/02     Created.
 * --------------------------------------------------------------------
 */
#include "cds_api.h"
#include "cfg_priv.h"
#include "lim_trace.h"

/*--------------------------------------------------------------------*/
/* ATTENTION:  The functions contained in this module are to be used  */
/*             by CFG module ONLY.                                    */
/*--------------------------------------------------------------------*/

/**---------------------------------------------------------------------
 * cfg_send_host_msg()
 *
 * FUNCTION:
 * Send CNF/RSP to host.
 *
 * LOGIC:
 * Please see Configuration & Statistic Collection Micro-Architecture
 * specification for details.
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param msgType:     message type
 * @param msgLen:      message length
 * @param paramNum:    number of parameters
 * @param pParamList:  pointer to parameter list
 * @param dataLen:     data length
 * @param pData:       pointer to additional data
 *
 * @return None.
 *
 */
void
cfg_send_host_msg(tpAniSirGlobal pMac, uint16_t msgType, uint32_t msgLen,
		  uint32_t paramNum, uint32_t *pParamList, uint32_t dataLen,
		  uint32_t *pData)
{
	uint32_t *pMsg, *pEnd;
	struct scheduler_msg mmhMsg = {0};

	if ((paramNum > 0) && (NULL == pParamList)) {
		pe_err("pParamList NULL when paramNum greater than 0!");
		return;
	}
	if ((dataLen > 0) && (NULL == pData)) {
		pe_err("pData NULL when dataLen greater than 0!");
		return;
	}
	pMsg = qdf_mem_malloc(msgLen);
	if (NULL == pMsg) {
		pe_err("Memory allocation failure!");
		return;
	}
	/* Fill in message details */
	mmhMsg.type = msgType;
	mmhMsg.bodyptr = pMsg;
	mmhMsg.bodyval = 0;
	((tSirMbMsg *) pMsg)->type = msgType;
	((tSirMbMsg *) pMsg)->msgLen = (uint16_t) msgLen;

	switch (msgType) {
	case WNI_CFG_GET_RSP:
	case WNI_CFG_PARAM_UPDATE_IND:
	case WNI_CFG_DNLD_REQ:
	case WNI_CFG_DNLD_CNF:
	case WNI_CFG_SET_CNF:
		/* Fill in parameters */
		pMsg++;
		if (NULL != pParamList) {
			pEnd = pMsg + paramNum;
			while (pMsg < pEnd) {
				*pMsg++ = *pParamList++;
			}
		}
		/* Copy data if there is any */
		if (NULL != pData) {
			pEnd = pMsg + (dataLen >> 2);
			while (pMsg < pEnd) {
				*pMsg++ = *pData++;
			}
		}
		break;

	default:
		pe_warn("Unknown msg: %d!", (int)msgType);
		qdf_mem_free(pMsg);
		return;
	}

	/* Ship it */
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, mmhMsg.type));
	sys_process_mmh_msg(pMac, &mmhMsg);

} /*** end cfg_send_host_msg() ***/
