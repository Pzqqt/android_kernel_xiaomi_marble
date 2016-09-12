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

/**=========================================================================

   \file  rrm_api.h

   \brief RRM APIs

   ========================================================================*/

/* $Header$ */

#ifndef __RRM_API_H__
#define __RRM_API_H__

#define RRM_MIN_TX_PWR_CAP    13
#define RRM_MAX_TX_PWR_CAP    19

#define RRM_BCN_RPT_NO_BSS_INFO    0
#define RRM_BCN_RPT_MIN_RPT        1

uint8_t rrm_get_min_of_max_tx_power(tpAniSirGlobal pMac, int8_t regMax,
				    int8_t apTxPower);

extern tSirRetStatus rrm_initialize(tpAniSirGlobal pMac);

extern tSirRetStatus rrm_cleanup(tpAniSirGlobal pMac);

extern tSirRetStatus rrm_process_link_measurement_request(tpAniSirGlobal pMac,
							  uint8_t *pRxPacketInfo,
							  tDot11fLinkMeasurementRequest
							  *pLinkReq,
							  tpPESession
							  pSessionEntry);

extern tSirRetStatus rrm_process_radio_measurement_request(tpAniSirGlobal pMac,
							   tSirMacAddr peer,
							   tDot11fRadioMeasurementRequest
							   *pRRMReq,
							   tpPESession
							   pSessionEntry);

extern tSirRetStatus rrm_process_neighbor_report_response(tpAniSirGlobal pMac,
							  tDot11fNeighborReportResponse
							  *pNeighborRep,
							  tpPESession
							  pSessionEntry);

extern tSirRetStatus rrm_send_set_max_tx_power_req(tpAniSirGlobal pMac,
						   int8_t txPower,
						   tpPESession pSessionEntry);

extern int8_t rrm_get_mgmt_tx_power(tpAniSirGlobal pMac,
				       tpPESession pSessionEntry);

extern void rrm_cache_mgmt_tx_power(tpAniSirGlobal pMac,
				    int8_t txPower, tpPESession pSessionEntry);

extern tpRRMCaps rrm_get_capabilities(tpAniSirGlobal pMac,
				      tpPESession pSessionEntry);

extern void rrm_get_start_tsf(tpAniSirGlobal pMac, uint32_t *pStartTSF);

extern void rrm_update_start_tsf(tpAniSirGlobal pMac, uint32_t startTSF[2]);

extern tSirRetStatus rrm_set_max_tx_power_rsp(tpAniSirGlobal pMac,
					      tpSirMsgQ limMsgQ);

extern tSirRetStatus
rrm_process_neighbor_report_req(tpAniSirGlobal pMac,
				tpSirNeighborReportReqInd pNeighborReq);
extern tSirRetStatus
rrm_process_beacon_report_xmit(tpAniSirGlobal pMac,
			       tpSirBeaconReportXmitInd pBcnReport);
extern void lim_update_rrm_capability(tpAniSirGlobal mac_ctx,
				      tpSirSmeJoinReq join_req);
#endif
