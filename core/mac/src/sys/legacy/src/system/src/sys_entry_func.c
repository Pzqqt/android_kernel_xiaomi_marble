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
 * sys_entry_func.cc - This file has all the system level entry functions
 *                   for all the defined threads at system level.
 * Author:    V. K. Kandarpa
 * Date:      01/16/2002
 * History:-
 * Date       Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */
/* Standard include files */

/* Application Specific include files */
#include "sir_common.h"
#include "ani_global.h"

#include "lim_api.h"
#include "sch_api.h"
#include "utils_api.h"

#include "sys_debug.h"
#include "sys_def.h"
#include "sys_entry_func.h"
#include "sys_startup.h"
#include "lim_trace.h"
#include "wma_types.h"

tSirRetStatus postPTTMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg);

#include "qdf_types.h"
#include "cds_packet.h"

#define MAX_DEAUTH_ALLOWED 5
/* --------------------------------------------------------------------------- */
/**
 * sys_init_globals
 *
 * FUNCTION:
 *    Initializes system level global parameters
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param tpAniSirGlobal Sirius software parameter struct pointer
 * @return None
 */

tSirRetStatus sys_init_globals(tpAniSirGlobal pMac)
{

	qdf_mem_set((uint8_t *) &pMac->sys, sizeof(pMac->sys), 0);

	pMac->sys.gSysEnableScanMode = 1;
	pMac->sys.gSysEnableLinkMonitorMode = 0;
	sch_init_globals(pMac);

	return eSIR_SUCCESS;
}

/**
 * sys_bbt_process_message_core() - to process BBT messages
 * @mac_ctx: pointer to mac context
 * @msg: message pointer
 * @type: type of persona
 * @subtype: subtype of persona
 *
 * This routine is to process some bbt messages
 *
 * Return: None
 */
tSirRetStatus
sys_bbt_process_message_core(tpAniSirGlobal mac_ctx, tpSirMsgQ msg,
		uint32_t type, uint32_t subtype)
{
	uint32_t framecount;
	tSirRetStatus ret;
	void *bd_ptr;
	tMgmtFrmDropReason dropreason;
	cds_pkt_t *vos_pkt = (cds_pkt_t *) msg->bodyptr;
	QDF_STATUS qdf_status =
		wma_ds_peek_rx_packet_info(vos_pkt, &bd_ptr, false);

	mac_ctx->sys.gSysBbtReceived++;

	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		goto fail;

	sys_log(mac_ctx, LOG3, FL("Rx Mgmt Frame Subtype: %d\n"), subtype);
	sir_dump_buf(mac_ctx, SIR_SYS_MODULE_ID, LOG3,
		(uint8_t *) WMA_GET_RX_MAC_HEADER(bd_ptr),
		WMA_GET_RX_MPDU_LEN(bd_ptr));
	sir_dump_buf(mac_ctx, SIR_SYS_MODULE_ID, LOG3,
		WMA_GET_RX_MPDU_DATA(bd_ptr),
		WMA_GET_RX_PAYLOAD_LEN(bd_ptr));

	mac_ctx->sys.gSysFrameCount[type][subtype]++;
	framecount = mac_ctx->sys.gSysFrameCount[type][subtype];

	if (type == SIR_MAC_MGMT_FRAME) {
		tpSirMacMgmtHdr mac_hdr;

		/*
		 * Drop beacon frames in deferred state to avoid VOSS run out of
		 * message wrappers.
		 */
		if ((subtype == SIR_MAC_MGMT_BEACON) &&
			(!lim_is_system_in_scan_state(mac_ctx)) &&
			(GET_LIM_PROCESS_DEFD_MESGS(mac_ctx) != true) &&
			!mac_ctx->lim.gLimSystemInScanLearnMode) {
			sys_log(mac_ctx, LOG1,
				FL("dropping received beacon in deffered state"));
			goto fail;
		}

		dropreason = lim_is_pkt_candidate_for_drop(mac_ctx, bd_ptr,
				subtype);
		if (eMGMT_DROP_NO_DROP != dropreason) {
			sys_log(mac_ctx, LOG1,
				FL("Mgmt Frame %d being dropped, reason: %d\n"),
				subtype, dropreason);
				MTRACE(mac_trace(mac_ctx,
					TRACE_CODE_RX_MGMT_DROP, NO_SESSION,
					dropreason);)
			goto fail;
		}

		mac_hdr = WMA_GET_RX_MAC_HEADER(bd_ptr);
		if (subtype == SIR_MAC_MGMT_ASSOC_REQ) {
			sys_log(mac_ctx, LOG1,
				FL("ASSOC REQ frame allowed: da: " MAC_ADDRESS_STR ", sa: " MAC_ADDRESS_STR ", bssid: " MAC_ADDRESS_STR ", Assoc Req count so far: %d\n"),
				MAC_ADDR_ARRAY(mac_hdr->da),
				MAC_ADDR_ARRAY(mac_hdr->sa),
				MAC_ADDR_ARRAY(mac_hdr->bssId),
				mac_ctx->sys.gSysFrameCount[type][subtype]);
		}
		if (subtype == SIR_MAC_MGMT_DEAUTH) {
			sys_log(mac_ctx, LOG1,
				FL("DEAUTH frame allowed: da: " MAC_ADDRESS_STR ", sa: " MAC_ADDRESS_STR ", bssid: " MAC_ADDRESS_STR ", DEAUTH count so far: %d\n"),
				MAC_ADDR_ARRAY(mac_hdr->da),
				MAC_ADDR_ARRAY(mac_hdr->sa),
				MAC_ADDR_ARRAY(mac_hdr->bssId),
				mac_ctx->sys.gSysFrameCount[type][subtype]);
		}
		if (subtype == SIR_MAC_MGMT_DISASSOC) {
			sys_log(mac_ctx, LOG1,
				FL("DISASSOC frame allowed: da: " MAC_ADDRESS_STR ", sa: " MAC_ADDRESS_STR ", bssid: " MAC_ADDRESS_STR ", DISASSOC count so far: %d\n"),
				MAC_ADDR_ARRAY(mac_hdr->da),
				MAC_ADDR_ARRAY(mac_hdr->sa),
				MAC_ADDR_ARRAY(mac_hdr->bssId),
				mac_ctx->sys.gSysFrameCount[type][subtype]);
		}

		/* Post the message to PE Queue */
		ret = (tSirRetStatus) lim_post_msg_api(mac_ctx, msg);
		if (ret != eSIR_SUCCESS) {
			sys_log(mac_ctx, LOGE,
				FL("posting to LIM2 failed, ret %d\n"), ret);
				goto fail;
		}
		mac_ctx->sys.gSysBbtPostedToLim++;
	} else if (type == SIR_MAC_DATA_FRAME) {
#ifdef FEATURE_WLAN_ESE
		PELOGW(sys_log(mac_ctx, LOGW, FL("IAPP Frame...\n")););
		/* Post the message to PE Queue */
		ret = (tSirRetStatus) lim_post_msg_api(mac_ctx, msg);
		if (ret != eSIR_SUCCESS) {
			sys_log(mac_ctx, LOGE,
				FL("posting to LIM2 failed, ret %d\n"), ret);
			goto fail;
		}
		mac_ctx->sys.gSysBbtPostedToLim++;
#endif
	} else {
		sys_log(mac_ctx, LOG3,
			"BBT received Invalid type %d subtype %d "
			"LIM state %X. BD dump is:\n", type, subtype,
			lim_get_sme_state(mac_ctx));
		goto fail;
	}
	return eSIR_SUCCESS;
fail:
	mac_ctx->sys.gSysBbtDropped++;
	return eSIR_FAILURE;
}

void sys_log(tpAniSirGlobal pMac, uint32_t loglevel, const char *pString, ...)
{
	/* Verify against current log level */
	if (loglevel >
	    pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE(SIR_SYS_MODULE_ID)])
		return;
	else {
		va_list marker;

		va_start(marker, pString);      /* Initialize variable arguments. */

		log_debug(pMac, SIR_SYS_MODULE_ID, loglevel, pString, marker);

		va_end(marker); /* Reset variable arguments.      */
	}
}
