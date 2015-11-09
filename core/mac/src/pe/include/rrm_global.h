/*
 * Copyright (c) 2011-2012, 2014-2015 The Linux Foundation. All rights reserved.
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

#if !defined(__RRMGLOBAL_H)
#define __RRMGLOBAL_H

/**=========================================================================

   \file  rrm_global.h

   \brief Definitions for SME APIs

   ========================================================================*/

typedef enum eRrmRetStatus {
	eRRM_SUCCESS,
	eRRM_INCAPABLE,
	eRRM_REFUSED,
	eRRM_FAILURE
} tRrmRetStatus;

typedef enum eRrmMsgReqSource {
	eRRM_MSG_SOURCE_LEGACY_ESE = 1, /* legacy ese */
	eRRM_MSG_SOURCE_11K = 2,        /* 11k */
	eRRM_MSG_SOURCE_ESE_UPLOAD = 3, /* ese upload approach */
} tRrmMsgReqSource;

typedef struct sSirChannelInfo {
	uint8_t regulatoryClass;
	uint8_t channelNum;
} tSirChannelInfo, *tpSirChannelInfo;

typedef struct sSirBeaconReportReqInd {
	uint16_t messageType;   /* eWNI_SME_BEACON_REPORT_REQ_IND */
	uint16_t length;
	tSirMacAddr bssId;
	uint16_t measurementDuration[SIR_ESE_MAX_MEAS_IE_REQS]; /* ms */
	uint16_t randomizationInterval; /* ms */
	tSirChannelInfo channelInfo;
	/* 0: wildcard */
	tSirMacAddr macaddrBssid;
	/* 0:Passive, 1: Active, 2: table mode */
	uint8_t fMeasurementtype[SIR_ESE_MAX_MEAS_IE_REQS];
	tAniSSID ssId;          /* May be wilcard. */
	uint16_t uDialogToken;
	tSirChannelList channelList;    /* From AP channel report. */
	tRrmMsgReqSource msgSource;
} tSirBeaconReportReqInd, *tpSirBeaconReportReqInd;

typedef struct sSirBeaconReportXmitInd {
	uint16_t messageType;   /* eWNI_SME_BEACON_REPORT_RESP_XMIT_IND */
	uint16_t length;
	tSirMacAddr bssId;
	uint16_t uDialogToken;
	uint8_t fMeasureDone;
	uint16_t duration;
	uint8_t regClass;
	uint8_t numBssDesc;
	tpSirBssDescription pBssDescription[SIR_BCN_REPORT_MAX_BSS_DESC];
} tSirBeaconReportXmitInd, *tpSirBeaconReportXmitInd;

typedef struct sSirNeighborReportReqInd {
	/* eWNI_SME_NEIGHBOR_REPORT_REQ_IND */
	uint16_t messageType;
	uint16_t length;
	/* For the session. */
	tSirMacAddr bssId;
	/* true - dont include SSID in the request. */
	uint16_t noSSID;
	/* false  include the SSID. It may be null (wildcard) */
	tSirMacSSid ucSSID;
} tSirNeighborReportReqInd, *tpSirNeighborReportReqInd;

typedef struct sSirNeighborBssDescription {
	uint16_t length;
	tSirMacAddr bssId;
	uint8_t regClass;
	uint8_t channel;
	uint8_t phyType;
	union sSirNeighborBssidInfo {
		struct _rrmInfo {
			 /* see IEEE 802.11k Table 7-43a */
			uint32_t fApPreauthReachable:2;
			uint32_t fSameSecurityMode:1;
			uint32_t fSameAuthenticator:1;
			/* see IEEE 802.11k Table 7-95d */
			uint32_t fCapSpectrumMeasurement:1;
			uint32_t fCapQos:1;
			uint32_t fCapApsd:1;
			uint32_t fCapRadioMeasurement:1;
			uint32_t fCapDelayedBlockAck:1;
			uint32_t fCapImmediateBlockAck:1;
			uint32_t fMobilityDomain:1;
			uint32_t reserved:21;
		} rrmInfo;
		struct _eseInfo {
			uint32_t channelBand:8;
			uint32_t minRecvSigPower:8;
			uint32_t apTxPower:8;
			uint32_t roamHysteresis:8;
			uint32_t adaptScanThres:8;

			uint32_t transitionTime:8;
			uint32_t tsfOffset:16;

			uint32_t beaconInterval:16;
			uint32_t reserved:16;
		} eseInfo;
	} bssidInfo;

	/* Optional sub IEs....ignoring for now. */
} tSirNeighborBssDescription, *tpSirNeighborBssDescripton;

typedef struct sSirNeighborReportInd {
	uint16_t messageType;   /* eWNI_SME_NEIGHBOR_REPORT_IND */
	uint16_t length;
	uint8_t sessionId;
	uint16_t numNeighborReports;
	tSirMacAddr bssId;      /* For the session. */
	tSirNeighborBssDescription sNeighborBssDescription[1];
} tSirNeighborReportInd, *tpSirNeighborReportInd;

typedef struct sRRMBeaconReportRequestedIes {
	uint8_t num;
	uint8_t *pElementIds;
} tRRMBeaconReportRequestedIes, *tpRRMBeaconReportRequestedIes;

/* Reporting detail defines. */
/* Reference - IEEE Std 802.11k-2008 section 7.3.2.21.6 Table 7-29h */
#define BEACON_REPORTING_DETAIL_NO_FF_IE 0
#define BEACON_REPORTING_DETAIL_ALL_FF_REQ_IE 1
#define BEACON_REPORTING_DETAIL_ALL_FF_IE 2

typedef struct sRRMReq {
	uint8_t dialog_token;   /* In action frame; */
	uint8_t token;          /* Within individual request; */
	uint8_t type;
	union {
		struct {
			uint8_t reportingDetail;
			tRRMBeaconReportRequestedIes reqIes;
		} Beacon;
	} request;
	uint8_t sendEmptyBcnRpt;
} tRRMReq, *tpRRMReq;

typedef struct sRRMCaps {
	uint8_t LinkMeasurement:1;
	uint8_t NeighborRpt:1;
	uint8_t parallel:1;
	uint8_t repeated:1;
	uint8_t BeaconPassive:1;
	uint8_t BeaconActive:1;
	uint8_t BeaconTable:1;
	uint8_t BeaconRepCond:1;
	uint8_t FrameMeasurement:1;
	uint8_t ChannelLoad:1;
	uint8_t NoiseHistogram:1;
	uint8_t statistics:1;
	uint8_t LCIMeasurement:1;
	uint8_t LCIAzimuth:1;
	uint8_t TCMCapability:1;
	uint8_t triggeredTCM:1;
	uint8_t APChanReport:1;
	uint8_t RRMMIBEnabled:1;
	uint8_t operatingChanMax:3;
	uint8_t nonOperatingChanMax:3;
	uint8_t MeasurementPilot:3;
	uint8_t MeasurementPilotEnabled:1;
	uint8_t NeighborTSFOffset:1;
	uint8_t RCPIMeasurement:1;
	uint8_t RSNIMeasurement:1;
	uint8_t BssAvgAccessDelay:1;
	uint8_t BSSAvailAdmission:1;
	uint8_t AntennaInformation:1;
	uint8_t fine_time_meas_rpt:1;
	uint8_t lci_capability:1;
	uint8_t reserved:4;
} tRRMCaps, *tpRRMCaps;

typedef struct sRrmPEContext {
	uint8_t rrmEnable;
	/*
	 * Used during scan/measurement to store the start TSF.
	 * this is not used directly in beacon reports.
	 */
	uint32_t startTSF[2];
	/*
	 * This value is stored into bssdescription and beacon report
	 * gets it from bss decsription.
	 */
	tRRMCaps rrmEnabledCaps;
	tPowerdBm txMgmtPower;
	/* Dialog token for the request initiated from station. */
	uint8_t DialogToken;
	tpRRMReq pCurrentReq;
} tRrmPEContext, *tpRrmPEContext;

/* 2008 11k spec reference: 18.4.8.5 RCPI Measurement */
#define RCPI_LOW_RSSI_VALUE   (-110)
#define RCPI_MAX_VALUE        (220)
#define CALCULATE_RCPI(rssi)  (((rssi) + 110) * 2)

#endif /* #if defined __RRMGLOBAL_H */
