/*
 * Copyright (c) 2011-2012, 2014-2018 The Linux Foundation. All rights reserved.
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
 *
 * Author:      Dinesh Upadhyay
 * Date:        10/24/06
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __LIM_ADMIT_CONTROL_H__
#define __LIM_ADMIT_CONTROL_H__

#include "sir_common.h"
#include "sir_mac_prot_def.h"

#include "ani_global.h"

QDF_STATUS
lim_tspec_find_by_assoc_id(tpAniSirGlobal, uint16_t, tSirMacTspecIE *,
			   tpLimTspecInfo, tpLimTspecInfo *);

/* Add TSPEC in lim local table */
QDF_STATUS lim_tspec_add(tpAniSirGlobal pMac,
			    uint8_t *pAddr,
			    uint16_t assocId,
			    tSirMacTspecIE *pTspec,
			    uint32_t interval, tpLimTspecInfo *ppInfo);

/* admit control interface */
extern QDF_STATUS lim_admit_control_add_ts(tpAniSirGlobal pMac,
				uint8_t *pAddr, tSirAddtsReqInfo *addts,
				tSirMacQosCapabilityStaIE *qos,
				uint16_t assocId, uint8_t alloc,
				tSirMacScheduleIE *pSch,
				/* index to the lim tspec table. */
				uint8_t *pTspecIdx,
				tpPESession psessionEntry);

static inline QDF_STATUS
lim_admit_control_add_sta(tpAniSirGlobal pMac, uint8_t *staAddr, uint8_t alloc)
{
	return QDF_STATUS_SUCCESS;
}

extern QDF_STATUS
lim_admit_control_delete_sta(tpAniSirGlobal pMac, uint16_t assocId);

extern QDF_STATUS
lim_admit_control_delete_ts(tpAniSirGlobal pMac,
			    uint16_t assocId,
			    tSirMacTSInfo *tsinfo,
			    uint8_t *tsStatus, uint8_t *tspecIdx);

extern QDF_STATUS lim_update_admit_policy(tpAniSirGlobal pMac);

QDF_STATUS lim_admit_control_init(tpAniSirGlobal pMac);
#ifdef FEATURE_WLAN_ESE
QDF_STATUS lim_send_hal_msg_add_ts(tpAniSirGlobal pMac,
				      uint16_t staIdx,
				      uint8_t tspecIdx,
				      tSirMacTspecIE tspecIE,
				      uint8_t sessionId, uint16_t tsm_interval);
#else
QDF_STATUS lim_send_hal_msg_add_ts(tpAniSirGlobal pMac,
				      uint16_t staIdx,
				      uint8_t tspecIdx,
				      tSirMacTspecIE tspecIE,
				      uint8_t sessionId);
#endif

QDF_STATUS lim_send_hal_msg_del_ts(tpAniSirGlobal pMac,
				      uint16_t staIdx,
				      uint8_t tspecIdx,
				      tSirDeltsReqInfo delts,
				      uint8_t sessionId, uint8_t *bssId);
void lim_process_hal_add_ts_rsp(tpAniSirGlobal pMac,
				struct scheduler_msg *limMsg);

#endif
