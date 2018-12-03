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

uint8_t rrm_get_min_of_max_tx_power(tpAniSirGlobal mac, int8_t regMax,
				    int8_t apTxPower);

QDF_STATUS rrm_initialize(tpAniSirGlobal mac);

QDF_STATUS rrm_cleanup(tpAniSirGlobal mac);

QDF_STATUS rrm_process_link_measurement_request(tpAniSirGlobal mac,
						uint8_t *pRxPacketInfo,
						tDot11fLinkMeasurementRequest
							  *pLinkReq,
						struct pe_session *
							  pe_session);

QDF_STATUS rrm_process_radio_measurement_request(tpAniSirGlobal mac,
						 tSirMacAddr peer,
						 tDot11fRadioMeasurementRequest
							   *pRRMReq,
						 struct pe_session *
							   pe_session);

QDF_STATUS rrm_process_neighbor_report_response(tpAniSirGlobal mac,
						tDot11fNeighborReportResponse
							  *pNeighborRep,
						struct pe_session *
							  pe_session);

QDF_STATUS rrm_send_set_max_tx_power_req(tpAniSirGlobal mac,
					 int8_t txPower,
					 struct pe_session *pe_session);

int8_t rrm_get_mgmt_tx_power(tpAniSirGlobal mac,
			     struct pe_session *pe_session);

void rrm_cache_mgmt_tx_power(tpAniSirGlobal mac,
			     int8_t txPower, struct pe_session *pe_session);

tpRRMCaps rrm_get_capabilities(tpAniSirGlobal mac,
			       struct pe_session *pe_session);

void rrm_get_start_tsf(tpAniSirGlobal mac, uint32_t *pStartTSF);

void rrm_update_start_tsf(tpAniSirGlobal mac, uint32_t startTSF[2]);

QDF_STATUS rrm_set_max_tx_power_rsp(tpAniSirGlobal mac,
				    struct scheduler_msg *limMsgQ);

QDF_STATUS
rrm_process_neighbor_report_req(tpAniSirGlobal mac,
				tpSirNeighborReportReqInd pNeighborReq);

QDF_STATUS
rrm_process_beacon_report_xmit(tpAniSirGlobal mac,
			       tpSirBeaconReportXmitInd pBcnReport);

void lim_update_rrm_capability(tpAniSirGlobal mac_ctx,
			       tpSirSmeJoinReq join_req);
#endif
