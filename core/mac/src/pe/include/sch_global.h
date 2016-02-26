/*
 * Copyright (c) 2013-2014 The Linux Foundation. All rights reserved.
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
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __SCH_GLOBAL_H__
#define __SCH_GLOBAL_H__

#include "sir_mac_prop_exts.h"
#include "lim_global.h"

#include "parser_api.h"

#ifdef WLAN_SOFTAP_VSTA_FEATURE
#define TIM_IE_SIZE 0xB
#else
#define TIM_IE_SIZE 0x7
#endif

/* ----------------------- Beacon processing ------------------------ */

/* / Beacon structure */
#define tSchBeaconStruct tSirProbeRespBeacon
#define tpSchBeaconStruct struct sSirProbeRespBeacon *

/* ------------------------------------------------------------------- */

/* ****************** MISC defs ********************************* */

struct schMisc {
	uint16_t gSchBeaconInterval;

	/* / Current CFP count */
	uint8_t gSchCFPCount;

	/* / CFP Duration remaining */
	uint8_t gSchCFPDurRemaining;

	/* / CFP Maximum Duration */
	uint8_t gSchCFPMaxDuration;

	/* / Current DTIM count */
	uint8_t gSchDTIMCount;

	/* / Whether we have initiated a CFP or not */
	uint8_t gSchCFPInitiated;

	/* / Whether we have initiated a CFB or not */
	uint8_t gSchCFBInitiated;

	/* / CFP is enabled and AP is configured as HCF */
	uint8_t gSchCFPEnabled;

	/* / CFB is enabled and AP is configured as HCF */
	uint8_t gSchCFBEnabled;

	/* --------- STA ONLY state ----------- */

	/* / Indicates whether RR timer is running or not */
	uint8_t rrTimer[8];

	/* / Indicates the remaining RR timeout value if the RR timer is running */
	uint16_t rrTimeout[8];

	/* / Number of RRs transmitted */
	uint16_t numRR[8];
	uint16_t numRRtimeouts[8];

	/* / flag to indicate that beacon template has been updated */
	uint8_t fBeaconChanged;

	uint16_t p2pIeOffset;

};

/* ****************** MISC defs ********************************* */

typedef struct schStaWaitList {
	uint16_t staId;
	uint16_t count;
} tStaWaitList, *tpStaWaitList;

/* / Global SCH structure */
typedef struct sAniSirSch {
	/* / The scheduler object */
	struct schMisc schObject;

	/* schQoSClass unsolicited; */

	/* / Whether HCF is enabled or not */
	uint8_t gSchHcfEnabled;

	/* / Whether scan is requested by LIM or not */
	uint8_t gSchScanRequested;

	/* / Whether scan request is received by SCH or not */
	uint8_t gSchScanReqRcvd;

	/* / Debug flag to disable beacon generation */
	uint32_t gSchGenBeacon;

#define SCH_MAX_ARR 100
	uint32_t gSchBeaconsWritten;
	uint32_t gSchBeaconsSent;
	uint32_t gSchBBXportRcvCnt;
	uint32_t gSchRRRcvCnt, qosNullCnt;
	uint32_t gSchBcnRcvCnt;
	uint32_t gSchUnknownRcvCnt;

	uint32_t gSchBcnParseErrorCnt;
	uint32_t gSchBcnIgnored;

	uint32_t numPoll, numData, numCorrupt;
	uint32_t numBogusInt, numTxAct0;

#define SCH_MAX_NUM_SCH 21
	uint32_t lastBeaconLength;
	uint16_t rrTimeout;
	uint32_t pollPeriod;
	uint32_t multipleSched;
	uint32_t pollFeedbackHist[8];
	uint32_t dataFeedbackHist[8];
	uint32_t maxPollTimeouts;
	uint32_t checkCfbFlagStuck;

	/* / Sta Wait list */
	tpStaWaitList pStaWaitList;

	/* / Pointer to next available entry in sta wait list */
	uint16_t staWaitListIn;
	/* / Pointer to first waiting sta in sta wait list */
	uint16_t staWaitListOut;
	/* / Total number of waiting STAs in sta wait list */
	uint16_t staWaitListCount;
	/* / Total number of schedules to be waited */
	uint16_t staWaitListTotalWait;

	/* / Number of entries in DPH activity queue that were ignored */
	uint32_t ignoreDph;

} tAniSirSch, *tpAniSirSch;

#endif
