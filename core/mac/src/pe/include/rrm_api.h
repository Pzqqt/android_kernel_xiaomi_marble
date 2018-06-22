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

QDF_STATUS rrm_initialize(tpAniSirGlobal pMac);

QDF_STATUS rrm_cleanup(tpAniSirGlobal pMac);

QDF_STATUS rrm_process_link_measurement_request(tpAniSirGlobal pMac,
						uint8_t *pRxPacketInfo,
						tDot11fLinkMeasurementRequest
							  *pLinkReq,
						tpPESession
							  pSessionEntry);

QDF_STATUS rrm_process_radio_measurement_request(tpAniSirGlobal pMac,
						 tSirMacAddr peer,
						 tDot11fRadioMeasurementRequest
							   *pRRMReq,
						 tpPESession
							   pSessionEntry);

QDF_STATUS rrm_process_neighbor_report_response(tpAniSirGlobal pMac,
						tDot11fNeighborReportResponse
							  *pNeighborRep,
						tpPESession
							  pSessionEntry);

QDF_STATUS rrm_send_set_max_tx_power_req(tpAniSirGlobal pMac,
					 int8_t txPower,
					 tpPESession pSessionEntry);

int8_t rrm_get_mgmt_tx_power(tpAniSirGlobal pMac,
			     tpPESession pSessionEntry);

void rrm_cache_mgmt_tx_power(tpAniSirGlobal pMac,
			     int8_t txPower, tpPESession pSessionEntry);

tpRRMCaps rrm_get_capabilities(tpAniSirGlobal pMac,
			       tpPESession pSessionEntry);

void rrm_get_start_tsf(tpAniSirGlobal pMac, uint32_t *pStartTSF);

void rrm_update_start_tsf(tpAniSirGlobal pMac, uint32_t startTSF[2]);

QDF_STATUS rrm_set_max_tx_power_rsp(tpAniSirGlobal pMac,
				    struct scheduler_msg *limMsgQ);

QDF_STATUS
rrm_process_neighbor_report_req(tpAniSirGlobal pMac,
				tpSirNeighborReportReqInd pNeighborReq);

QDF_STATUS
rrm_process_beacon_report_xmit(tpAniSirGlobal pMac,
			       tpSirBeaconReportXmitInd pBcnReport);

void lim_update_rrm_capability(tpAniSirGlobal mac_ctx,
			       tpSirSmeJoinReq join_req);
#endif
