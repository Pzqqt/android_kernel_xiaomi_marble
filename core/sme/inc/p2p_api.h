/*
 * Copyright (c) 2011-2012, 2014-2016 The Linux Foundation. All rights reserved.
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
 *
 * Name:  p2p_api.h
 *
 * Description: P2P FSM defines.
 *
 *
 */

#ifndef __P2P_API_H__
#define __P2P_API_H__

#include "qdf_types.h"
#include "qdf_mc_timer.h"
#include "qdf_lock.h"

typedef struct sP2pPsConfig {
	uint8_t opp_ps;
	uint32_t ctWindow;
	uint8_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint8_t psSelection;
	uint8_t sessionid;
} tP2pPsConfig, *tpP2pPsConfig;

typedef QDF_STATUS (*remainOnChanCallback)(tHalHandle, void *context,
		QDF_STATUS status, uint32_t scan_id);

typedef struct sRemainOnChn {
	uint8_t chn;
	uint32_t duration;
	remainOnChanCallback callback;
	void *pCBContext;
} tRemainOnChn, tpRemainOnChn;

#define SIZE_OF_NOA_DESCRIPTOR 13
#define MAX_NOA_PERIOD_IN_MICROSECS 3000000

#define P2P_CLEAR_POWERSAVE 0
#define P2P_OPPORTUNISTIC_PS 1
#define P2P_PERIODIC_NOA 2
#define P2P_SINGLE_NOA 4

typedef struct sp2pContext {
	v_CONTEXT_t cds_context;
	tHalHandle hHal;
	uint8_t sessionId;      /* Session id corresponding to P2P */
	uint8_t SMEsessionId;
	uint8_t probeReqForwarding;
	uint8_t *probeRspIe;
	uint32_t probeRspIeLength;
} tp2pContext, *tPp2pContext;

QDF_STATUS sme_remain_on_chn_rsp(tpAniSirGlobal pMac, uint8_t *pMsg);
QDF_STATUS sme_remain_on_chn_ready(tHalHandle hHal, uint8_t *pMsg);
QDF_STATUS sme_remain_on_channel(tHalHandle hHal, uint8_t sessionId,
		uint8_t channel, uint32_t duration,
		remainOnChanCallback callback,
		void *pContext, uint8_t isP2PProbeReqAllowed,
		uint32_t *scan_id);
QDF_STATUS sme_report_probe_req(tHalHandle hHal, uint8_t flag);
QDF_STATUS sme_update_p2p_ie(tHalHandle hHal, void *p2pIe,
		uint32_t p2pIeLength);
QDF_STATUS sme_send_action(tHalHandle hHal, uint8_t sessionId,
		const uint8_t *pBuf, uint32_t len, uint16_t wait, bool noack,
		uint16_t channel_freq);
QDF_STATUS sme_cancel_remain_on_channel(tHalHandle hHal,
	uint8_t sessionId, uint32_t scan_id);
QDF_STATUS sme_p2p_open(tHalHandle hHal);
QDF_STATUS p2p_stop(tHalHandle hHal);
QDF_STATUS sme_p2p_close(tHalHandle hHal);
QDF_STATUS sme_p2p_set_ps(tHalHandle hHal, tP2pPsConfig *data);
QDF_STATUS p2p_remain_on_channel(tHalHandle hHal, uint8_t sessionId,
		uint8_t channel, uint32_t duration,
		remainOnChanCallback callback,
		void *pContext, uint8_t isP2PProbeReqAllowed,
		uint32_t scan_id);
QDF_STATUS p2p_send_action(tHalHandle hHal, uint8_t sessionId,
		const uint8_t *pBuf, uint32_t len, uint16_t wait, bool noack,
		uint16_t channel_freq);
QDF_STATUS p2p_cancel_remain_on_channel(tHalHandle hHal,
	uint8_t sessionId, uint32_t scan_id);
QDF_STATUS p2p_set_ps(tHalHandle hHal, tP2pPsConfig *pNoA);
tSirRFBand get_rf_band(uint8_t channel);
#endif /* __P2P_API_H__ */
