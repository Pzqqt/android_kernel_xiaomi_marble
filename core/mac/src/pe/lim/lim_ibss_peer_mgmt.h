/*
 * Copyright (c) 2011-2012, 2014-2017 The Linux Foundation. All rights reserved.
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
 *
 * This file lim_ibss_peer_mgmt.h contains prototypes for
 * the utility functions LIM uses to maintain peers in IBSS.
 * Author:        Chandra Modumudi
 * Date:          03/12/04
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "sir_common.h"
#include "lim_utils.h"

void lim_ibss_init(tpAniSirGlobal);
void lim_ibss_delete(tpAniSirGlobal, tpPESession psessionEntry);
tSirRetStatus lim_ibss_coalesce(tpAniSirGlobal, tpSirMacMgmtHdr,
				tpSchBeaconStruct, uint8_t *, uint32_t, uint16_t,
				tpPESession);
tSirRetStatus lim_ibss_sta_add(tpAniSirGlobal, void *, tpPESession);
tSirRetStatus lim_ibss_add_sta_rsp(tpAniSirGlobal, void *, tpPESession);

/**
 * lim_process_ibss_del_sta_rsp()- Handle ibss delete
 * peer resp from firmware
 *
 * @mac_ptr: Pointer to Global MAC structure
 * @lim_msg: delete sta response
 * @pe_session: pe session
 *
 * Return: None
 *
 */
void lim_process_ibss_del_sta_rsp(tpAniSirGlobal mac_ctx,
	struct scheduler_msg *lim_msg,
	tpPESession pe_session);
tLimIbssPeerNode *lim_ibss_peer_find(tpAniSirGlobal pMac, tSirMacAddr macAddr);
void lim_ibss_del_bss_rsp(tpAniSirGlobal, void *, tpPESession);
void lim_ibss_del_bss_rsp_when_coalescing(tpAniSirGlobal, void *, tpPESession);
void lim_ibss_add_bss_rsp_when_coalescing(tpAniSirGlobal pMac, void *msg,
					  tpPESession pSessionEntry);
void lim_ibss_decide_protection_on_delete(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
					  tpUpdateBeaconParams pBeaconParams,
					  tpPESession pSessionEntry);
void lim_ibss_heart_beat_handle(tpAniSirGlobal pMac, tpPESession psessionEntry);
void lim_process_ibss_peer_inactivity(tpAniSirGlobal pMac, void *buf);
