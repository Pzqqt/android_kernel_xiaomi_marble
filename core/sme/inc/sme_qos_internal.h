/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#if !defined(__SMEQOSINTERNAL_H)
#define __SMEQOSINTERNAL_H

/**
 * \file  sme_qos_internal.h
 *
 * \brief prototype for SME QoS APIs
 */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include "qdf_lock.h"
#include "qdf_trace.h"
#include "qdf_mem.h"
#include "qdf_types.h"
#include "ani_global.h"
#include "sir_api.h"
#include "sme_qos_api.h"
#include "sme_internal.h"

/*--------------------------------------------------------------------------
  Type declarations
  ------------------------------------------------------------------------*/
#define SME_QOS_AP_SUPPORTS_APSD         0x80

/*---------------------------------------------------------------------------
  Enumeration of the various EDCA Access Categories:
  Based on AC to ACI mapping in 802.11e spec (identical to WMM)
  ---------------------------------------------------------------------------*/
typedef enum {
	SME_QOS_EDCA_AC_BE = 0, /* Best effort access category */
	SME_QOS_EDCA_AC_BK = 1, /* Background access category  */
	SME_QOS_EDCA_AC_VI = 2, /* Video access category       */
	SME_QOS_EDCA_AC_VO = 3, /* Voice access category       */

	SME_QOS_EDCA_AC_MAX
} sme_QosEdcaAcType;

/*---------------------------------------------------------------------------
  Enumeration of the various CSR event indication types that would be reported
  by CSR
  ---------------------------------------------------------------------------*/
typedef enum {
	SME_QOS_CSR_JOIN_REQ = 0,
	SME_QOS_CSR_ASSOC_COMPLETE,
	SME_QOS_CSR_REASSOC_REQ,
	SME_QOS_CSR_REASSOC_COMPLETE,
	SME_QOS_CSR_REASSOC_FAILURE,
	SME_QOS_CSR_DISCONNECT_REQ,
	SME_QOS_CSR_DISCONNECT_IND,
	SME_QOS_CSR_HANDOFF_ASSOC_REQ,
	SME_QOS_CSR_HANDOFF_COMPLETE,
	SME_QOS_CSR_HANDOFF_FAILURE,
	SME_QOS_CSR_PREAUTH_SUCCESS_IND,
	SME_QOS_CSR_SET_KEY_SUCCESS_IND,
} sme_qos_csr_event_indType;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
typedef enum {
	SME_QOS_DIAG_ADDTS_REQ = 0,
	SME_QOS_DIAG_ADDTS_RSP,
	SME_QOS_DIAG_DELTS
} sme_QosDiagQosEventSubtype;

typedef enum {
	SME_QOS_DIAG_ADDTS_ADMISSION_ACCEPTED = 0,
	SME_QOS_DIAG_ADDTS_INVALID_PARAMS,
	SME_QOS_DIAG_ADDTS_RESERVED,
	SME_QOS_DIAG_ADDTS_REFUSED,
	SME_QOS_DIAG_USER_REQUESTED,
	SME_QOS_DIAG_DELTS_IND_FROM_AP,

} sme_QosDiagQosEventReasonCode;

#endif /* FEATURE_WLAN_DIAG_SUPPORT */
/*---------------------------------------------------------------------------
  The association information structure to be passed by CSR after assoc or
  reassoc is done
  ---------------------------------------------------------------------------*/
typedef struct {
	tSirBssDescription *pBssDesc;
	tCsrRoamProfile *pProfile;
} sme_QosAssocInfo;

/*--------------------------------------------------------------------------
  External APIs for CSR - Internal to SME
  ------------------------------------------------------------------------*/
QDF_STATUS sme_qos_open(tpAniSirGlobal pMac);
QDF_STATUS sme_qos_close(tpAniSirGlobal pMac);
QDF_STATUS sme_qos_msg_processor(tpAniSirGlobal pMac, uint16_t msg_type,
		void *pMsgBuf);

/*--------------------------------------------------------------------------
  Internal APIs for CSR
  ------------------------------------------------------------------------*/
QDF_STATUS sme_qos_validate_params(tpAniSirGlobal pMac,
		tSirBssDescription *pBssDesc);
QDF_STATUS sme_qos_csr_event_ind(tpAniSirGlobal pMac,
		uint8_t sessionId,
		sme_qos_csr_event_indType ind, void *pEvent_info);
uint8_t sme_qos_get_acm_mask(tpAniSirGlobal pMac,
		tSirBssDescription *pSirBssDesc, tDot11fBeaconIEs *pIes);
#ifdef FEATURE_WLAN_ESE
uint8_t sme_qos_ese_retrieve_tspec_info(tpAniSirGlobal pMac, uint8_t sessionId,
		tTspecInfo * pTspecInfo);
#endif

#endif /* #if !defined( __SMEQOSINTERNAL_H ) */
