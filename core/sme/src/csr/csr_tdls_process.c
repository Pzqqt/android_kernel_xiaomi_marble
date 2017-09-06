/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * DOC: csr_tdls_process.c
 *
 * Implementation for the TDLS interface to PE.
 */
#ifdef FEATURE_WLAN_TDLS

#include "ani_global.h"          /* for tpAniSirGlobal */
#include "csr_inside_api.h"
#include "sme_inside.h"

#include "csr_support.h"

#include "host_diag_core_log.h"
#include "host_diag_core_event.h"
#include "csr_internal.h"

/*
 * TDLS Message processor, will be called after TDLS message recieved from
 * PE
 */
QDF_STATUS tdls_msg_processor(tpAniSirGlobal pMac, uint16_t msgType,
			      void *pMsgBuf)
{
	tCsrRoamInfo *roamInfo;
	tSirTdlsEventnotify *tevent = (tSirTdlsEventnotify *) pMsgBuf;

	roamInfo = qdf_mem_malloc(sizeof(*roamInfo));
	if (!roamInfo) {
		sme_err("failed to allocate memory");
		return QDF_STATUS_E_FAILURE;
	}
	switch (msgType) {
	case eWNI_SME_TDLS_SHOULD_DISCOVER:
		qdf_copy_macaddr(&roamInfo->peerMac, &tevent->peermac);
		roamInfo->reasonCode = tevent->peer_reason;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"%s: eWNI_SME_TDLS_SHOULD_DISCOVER for peer mac: "
				MAC_ADDRESS_STR " peer_reason: %d",
				__func__, MAC_ADDR_ARRAY(tevent->peermac.bytes),
				tevent->peer_reason);
		csr_roam_call_callback(pMac, tevent->sessionId, roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER);
		break;
	case eWNI_SME_TDLS_SHOULD_TEARDOWN:
		qdf_copy_macaddr(&roamInfo->peerMac, &tevent->peermac);
		roamInfo->reasonCode = tevent->peer_reason;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"%s: eWNI_SME_TDLS_SHOULD_TEARDOWN for peer mac: "
				MAC_ADDRESS_STR " peer_reason: %d",
				__func__, MAC_ADDR_ARRAY(tevent->peermac.bytes),
				tevent->peer_reason);
		csr_roam_call_callback(pMac, tevent->sessionId, roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN);
		break;
	case eWNI_SME_TDLS_PEER_DISCONNECTED:
		qdf_copy_macaddr(&roamInfo->peerMac, &tevent->peermac);
		roamInfo->reasonCode = tevent->peer_reason;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"%s: eWNI_SME_TDLS_PEER_DISCONNECTED for peer mac: "
				MAC_ADDRESS_STR " peer_reason: %d",
				__func__, MAC_ADDR_ARRAY(tevent->peermac.bytes),
				tevent->peer_reason);
		csr_roam_call_callback(pMac, tevent->sessionId, roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED);
		break;
	default:
		break;
	}
	qdf_mem_free(roamInfo);

	return QDF_STATUS_SUCCESS;
}
#endif
