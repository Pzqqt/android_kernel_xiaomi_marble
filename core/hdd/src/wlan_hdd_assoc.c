/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 *  DOC: wlan_hdd_assoc.c
 *
 *  WLAN Host Device Driver implementation
 *
 */

#include "wlan_hdd_includes.h"
#include <ani_global.h>
#include "dot11f.h"
#include "wlan_hdd_power.h"
#include "wlan_hdd_trace.h"
#include <linux/ieee80211.h>
#include <linux/wireless.h>
#include <linux/etherdevice.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#include "csr_inside_api.h"
#include "wlan_hdd_p2p.h"
#include "wlan_hdd_tdls.h"
#include "sme_api.h"
#include "wlan_hdd_hostapd.h"
#include <wlan_hdd_ipa.h>
#include "wlan_hdd_lpass.h"
#include <wlan_logging_sock_svc.h>
#include <cds_sched.h>
#include "wlan_policy_mgr_api.h"
#include <cds_utils.h>
#include "sme_power_save_api.h"
#include "wlan_hdd_napi.h"
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_flow_ctrl_legacy.h>
#include <cdp_txrx_peer_ops.h>
#include <cdp_txrx_misc.h>
#include <cdp_txrx_ctrl.h>
#include <wlan_logging_sock_svc.h>
#include <wlan_hdd_object_manager.h>
#include <cdp_txrx_handle.h>
#include "wlan_pmo_ucfg_api.h"

/* These are needed to recognize WPA and RSN suite types */
#define HDD_WPA_OUI_SIZE 4
#define HDD_RSN_OUI_SIZE 4
uint8_t ccp_wpa_oui00[HDD_WPA_OUI_SIZE] = { 0x00, 0x50, 0xf2, 0x00 };
uint8_t ccp_wpa_oui01[HDD_WPA_OUI_SIZE] = { 0x00, 0x50, 0xf2, 0x01 };
uint8_t ccp_wpa_oui02[HDD_WPA_OUI_SIZE] = { 0x00, 0x50, 0xf2, 0x02 };
uint8_t ccp_wpa_oui03[HDD_WPA_OUI_SIZE] = { 0x00, 0x50, 0xf2, 0x03 };
uint8_t ccp_wpa_oui04[HDD_WPA_OUI_SIZE] = { 0x00, 0x50, 0xf2, 0x04 };
uint8_t ccp_wpa_oui05[HDD_WPA_OUI_SIZE] = { 0x00, 0x50, 0xf2, 0x05 };

#ifdef FEATURE_WLAN_ESE
/* CCKM */
uint8_t ccp_wpa_oui06[HDD_WPA_OUI_SIZE] = { 0x00, 0x40, 0x96, 0x00 };
/* CCKM */
uint8_t ccp_rsn_oui06[HDD_RSN_OUI_SIZE] = { 0x00, 0x40, 0x96, 0x00 };
#endif /* FEATURE_WLAN_ESE */

/* group cipher */
uint8_t ccp_rsn_oui00[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x00 };

/* WEP-40 or RSN */
uint8_t ccp_rsn_oui01[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x01 };

/* TKIP or RSN-PSK */
uint8_t ccp_rsn_oui02[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x02 };

/* Reserved */
uint8_t ccp_rsn_oui03[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x03 };

/* AES-CCMP */
uint8_t ccp_rsn_oui04[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x04 };

/* WEP-104 */
uint8_t ccp_rsn_oui05[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x05 };

#ifdef WLAN_FEATURE_11W
/* RSN-PSK-SHA256 */
uint8_t ccp_rsn_oui07[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x06 };

/* RSN-8021X-SHA256 */
uint8_t ccp_rsn_oui08[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x05 };
#endif

/* Offset where the EID-Len-IE, start. */
#define FT_ASSOC_RSP_IES_OFFSET 6  /* Capability(2) + AID(2) + Status Code(2) */
#define FT_ASSOC_REQ_IES_OFFSET 4  /* Capability(2) + LI(2) */

#define BEACON_FRAME_IES_OFFSET 12
#define HDD_PEER_AUTHORIZE_WAIT 10

/**
 * beacon_filter_table - table of IEs used for beacon filtering
 */
static const int beacon_filter_table[] = {
	SIR_MAC_DS_PARAM_SET_EID,
	SIR_MAC_ERP_INFO_EID,
	SIR_MAC_EDCA_PARAM_SET_EID,
	SIR_MAC_QOS_CAPABILITY_EID,
	SIR_MAC_HT_INFO_EID,
	SIR_MAC_VHT_OPMODE_EID,
	SIR_MAC_VHT_OPERATION_EID,
#ifdef WLAN_FEATURE_11AX_BSS_COLOR
	/*
	 * EID: 221 vendor IE is being used temporarily by 11AX
	 * bss-color-change IE till it gets any fixed number. This
	 * vendor EID needs to be replaced with bss-color-change IE
	 * number.
	 */
	SIR_MAC_EID_VENDOR,
#endif
};

/**
 * hdd_conn_set_authenticated() - set authentication state
 * @pAdapter: pointer to the adapter
 * @authState: authentication state
 *
 * This function updates the global HDD station context
 * authentication state.
 *
 * Return: none
 */
static void
hdd_conn_set_authenticated(hdd_adapter_t *pAdapter, uint8_t authState)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	/* save the new connection state */
	hdd_debug("Authenticated state Changed from oldState:%d to State:%d",
		   pHddStaCtx->conn_info.uIsAuthenticated, authState);
	pHddStaCtx->conn_info.uIsAuthenticated = authState;

	/* Check is pending ROC request or not when auth state changed */
	schedule_delayed_work(&pHddCtx->roc_req_work, 0);
}

/**
 * hdd_conn_set_connection_state() - set connection state
 * @pAdapter: pointer to the adapter
 * @connState: connection state
 *
 * This function updates the global HDD station context connection state.
 *
 * Return: none
 */
void hdd_conn_set_connection_state(hdd_adapter_t *pAdapter,
				   eConnectionState connState)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	/* save the new connection state */
	hdd_debug("%pS Changed connectionState Changed from oldState:%d to State:%d",
		(void *)_RET_IP_, pHddStaCtx->conn_info.connState,
		connState);
	pHddStaCtx->conn_info.connState = connState;

	/* Check is pending ROC request or not when connection state changed */
	schedule_delayed_work(&pHddCtx->roc_req_work, 0);
}

/**
 * hdd_conn_get_connection_state() - get connection state
 * @pAdapter: pointer to the adapter
 * @pConnState: pointer to connection state
 *
 * This function updates the global HDD station context connection state.
 *
 * Return: true if (Infra Associated or IBSS Connected)
 *	and sets output parameter pConnState;
 *	false otherwise
 */
static inline bool hdd_conn_get_connection_state(hdd_station_ctx_t *pHddStaCtx,
						 eConnectionState *pConnState)
{
	bool fConnected = false;
	eConnectionState connState;

	/* get the connection state. */
	connState = pHddStaCtx->conn_info.connState;

	if (eConnectionState_Associated == connState ||
	    eConnectionState_IbssConnected == connState ||
	    eConnectionState_IbssDisconnected == connState) {
		fConnected = true;
	}

	if (pConnState)
		*pConnState = connState;

	return fConnected;
}

/**
 * hdd_is_connecting() - Function to check connection progress
 * @hdd_sta_ctx:    pointer to global HDD Station context
 *
 * Return: true if connecting, false otherwise
 */
bool hdd_is_connecting(hdd_station_ctx_t *hdd_sta_ctx)
{
	return hdd_sta_ctx->conn_info.connState ==
		eConnectionState_Connecting;
}

/**
 * hdd_conn_is_connected() - Function to check connection status
 * @pHddStaCtx:    pointer to global HDD Station context
 *
 * Return: false if any errors encountered, true otherwise
 */
bool hdd_conn_is_connected(hdd_station_ctx_t *pHddStaCtx)
{
	return hdd_conn_get_connection_state(pHddStaCtx, NULL);
}

/**
 * hdd_conn_get_connected_band() - get current connection radio band
 * @pHddStaCtx:    pointer to global HDD Station context
 *
 * Return: eCSR_BAND_24 or eCSR_BAND_5G based on current AP connection
 *	eCSR_BAND_ALL if not connected
 */
enum band_info hdd_conn_get_connected_band(hdd_station_ctx_t *pHddStaCtx)
{
	uint8_t staChannel = 0;

	if (eConnectionState_Associated == pHddStaCtx->conn_info.connState)
		staChannel = pHddStaCtx->conn_info.operationChannel;

	if (staChannel > 0 && staChannel < 14)
		return BAND_2G;
	else if (staChannel >= 36 && staChannel <= 184)
		return BAND_5G;
	else   /* If station is not connected return as eCSR_BAND_ALL */
		return BAND_ALL;
}

/**
 * hdd_conn_get_connected_cipher_algo() - get current connection cipher type
 * @pHddStaCtx: pointer to global HDD Station context
 * @pConnectedCipherAlgo: pointer to connected cipher algo
 *
 * Return: false if any errors encountered, true otherwise
 */
static inline bool
hdd_conn_get_connected_cipher_algo(hdd_station_ctx_t *pHddStaCtx,
				   eCsrEncryptionType *pConnectedCipherAlgo)
{
	bool fConnected = false;

	fConnected = hdd_conn_get_connection_state(pHddStaCtx, NULL);

	if (pConnectedCipherAlgo)
		*pConnectedCipherAlgo = pHddStaCtx->conn_info.ucEncryptionType;

	return fConnected;
}

/**
 * hdd_remove_beacon_filter() - remove beacon filter
 * @adapter: Pointer to the hdd adapter
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_remove_beacon_filter(hdd_adapter_t *adapter)
{
	QDF_STATUS status;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = sme_remove_beacon_filter(hdd_ctx->hHal,
				adapter->sessionId);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_remove_beacon_filter() failed");
		return -EFAULT;
	}

	return 0;
}

/**
 * hdd_add_beacon_filter() - add beacon filter
 * @adapter: Pointer to the hdd adapter
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_add_beacon_filter(hdd_adapter_t *adapter)
{
	int i;
	uint32_t ie_map[SIR_BCN_FLT_MAX_ELEMS_IE_LIST] = {0};
	QDF_STATUS status;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	for (i = 0; i < ARRAY_SIZE(beacon_filter_table); i++)
		qdf_set_bit((beacon_filter_table[i]),
				(unsigned long int *)ie_map);

	status = sme_add_beacon_filter(hdd_ctx->hHal,
				adapter->sessionId, ie_map);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_add_beacon_filter() failed");
		return -EFAULT;
	}
	return 0;
}

/**
 * hdd_copy_vht_caps()- copy vht caps info from roam info to
 *  hdd station context.
 * @hdd_sta_ctx: pointer to hdd station context
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_copy_ht_caps(hdd_station_ctx_t *hdd_sta_ctx,
				     tCsrRoamInfo *roam_info)
{
	tDot11fIEHTCaps *roam_ht_cap = &roam_info->ht_caps;
	struct ieee80211_ht_cap *hdd_ht_cap = &hdd_sta_ctx->conn_info.ht_caps;
	uint32_t i, temp_ht_cap;

	qdf_mem_zero(hdd_ht_cap, sizeof(struct ieee80211_ht_cap));

	if (roam_ht_cap->advCodingCap)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_LDPC_CODING;
	if (roam_ht_cap->supportedChannelWidthSet)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	temp_ht_cap = roam_ht_cap->mimoPowerSave &
	    (IEEE80211_HT_CAP_SM_PS >> IEEE80211_HT_CAP_SM_PS_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->cap_info |=
			temp_ht_cap << IEEE80211_HT_CAP_SM_PS_SHIFT;
	if (roam_ht_cap->greenField)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_GRN_FLD;
	if (roam_ht_cap->shortGI20MHz)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_SGI_20;
	if (roam_ht_cap->shortGI40MHz)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_SGI_40;
	if (roam_ht_cap->txSTBC)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_TX_STBC;
	temp_ht_cap = roam_ht_cap->rxSTBC & (IEEE80211_HT_CAP_RX_STBC >>
	    IEEE80211_HT_CAP_RX_STBC_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->cap_info |=
			temp_ht_cap << IEEE80211_HT_CAP_RX_STBC_SHIFT;
	if (roam_ht_cap->delayedBA)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_DELAY_BA;
	if (roam_ht_cap->maximalAMSDUsize)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_MAX_AMSDU;
	if (roam_ht_cap->dsssCckMode40MHz)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_DSSSCCK40;
	if (roam_ht_cap->psmp)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_RESERVED;
	if (roam_ht_cap->stbcControlFrame)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_40MHZ_INTOLERANT;
	if (roam_ht_cap->lsigTXOPProtection)
		hdd_ht_cap->cap_info |= IEEE80211_HT_CAP_LSIG_TXOP_PROT;

	/* 802.11n HT capability AMPDU settings (for ampdu_params_info) */
	if (roam_ht_cap->maxRxAMPDUFactor)
		hdd_ht_cap->ampdu_params_info |=
			IEEE80211_HT_AMPDU_PARM_FACTOR;
	temp_ht_cap = roam_ht_cap->mpduDensity &
	    (IEEE80211_HT_AMPDU_PARM_DENSITY >>
	     IEEE80211_HT_AMPDU_PARM_DENSITY_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->ampdu_params_info |=
		temp_ht_cap << IEEE80211_HT_AMPDU_PARM_DENSITY_SHIFT;

	/* 802.11n HT extended capabilities masks */
	if (roam_ht_cap->pco)
		hdd_ht_cap->extended_ht_cap_info |=
			IEEE80211_HT_EXT_CAP_PCO;
	temp_ht_cap = roam_ht_cap->transitionTime &
	    (IEEE80211_HT_EXT_CAP_PCO_TIME >>
	    IEEE80211_HT_EXT_CAP_PCO_TIME_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->extended_ht_cap_info |=
			temp_ht_cap << IEEE80211_HT_EXT_CAP_PCO_TIME_SHIFT;
	temp_ht_cap = roam_ht_cap->mcsFeedback &
	    (IEEE80211_HT_EXT_CAP_MCS_FB >> IEEE80211_HT_EXT_CAP_MCS_FB_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->extended_ht_cap_info |=
			temp_ht_cap << IEEE80211_HT_EXT_CAP_MCS_FB_SHIFT;

	/* tx_bf_cap_info capabilities */
	if (roam_ht_cap->txBF)
		hdd_ht_cap->tx_BF_cap_info |= TX_BF_CAP_INFO_TX_BF;
	if (roam_ht_cap->rxStaggeredSounding)
		hdd_ht_cap->tx_BF_cap_info |=
			TX_BF_CAP_INFO_RX_STAG_RED_SOUNDING;
	if (roam_ht_cap->txStaggeredSounding)
		hdd_ht_cap->tx_BF_cap_info |=
			TX_BF_CAP_INFO_TX_STAG_RED_SOUNDING;
	if (roam_ht_cap->rxZLF)
		hdd_ht_cap->tx_BF_cap_info |= TX_BF_CAP_INFO_RX_ZFL;
	if (roam_ht_cap->txZLF)
		hdd_ht_cap->tx_BF_cap_info |= TX_BF_CAP_INFO_TX_ZFL;
	if (roam_ht_cap->implicitTxBF)
		hdd_ht_cap->tx_BF_cap_info |= TX_BF_CAP_INFO_IMP_TX_BF;
	temp_ht_cap = roam_ht_cap->calibration &
	    (TX_BF_CAP_INFO_CALIBRATION >> TX_BF_CAP_INFO_CALIBRATION_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap << TX_BF_CAP_INFO_CALIBRATION_SHIFT;
	if (roam_ht_cap->explicitCSITxBF)
		hdd_ht_cap->tx_BF_cap_info |= TX_BF_CAP_INFO_EXP_CSIT_BF;
	if (roam_ht_cap->explicitUncompressedSteeringMatrix)
		hdd_ht_cap->tx_BF_cap_info |=
			TX_BF_CAP_INFO_EXP_UNCOMP_STEER_MAT;
	temp_ht_cap = roam_ht_cap->explicitBFCSIFeedback &
	    (TX_BF_CAP_INFO_EXP_BF_CSI_FB >>
	     TX_BF_CAP_INFO_EXP_BF_CSI_FB_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap << TX_BF_CAP_INFO_EXP_BF_CSI_FB_SHIFT;
	temp_ht_cap =
	    roam_ht_cap->explicitUncompressedSteeringMatrixFeedback &
	    (TX_BF_CAP_INFO_EXP_UNCMP_STEER_MAT >>
	     TX_BF_CAP_INFO_EXP_UNCMP_STEER_MAT_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap <<
			TX_BF_CAP_INFO_EXP_UNCMP_STEER_MAT_SHIFT;
	temp_ht_cap =
	    roam_ht_cap->explicitCompressedSteeringMatrixFeedback &
	    (TX_BF_CAP_INFO_EXP_CMP_STEER_MAT_FB >>
	     TX_BF_CAP_INFO_EXP_CMP_STEER_MAT_FB_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap <<
				TX_BF_CAP_INFO_EXP_CMP_STEER_MAT_FB_SHIFT;
	temp_ht_cap = roam_ht_cap->csiNumBFAntennae &
	    (TX_BF_CAP_INFO_CSI_NUM_BF_ANT >>
	     TX_BF_CAP_INFO_CSI_NUM_BF_ANT_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap << TX_BF_CAP_INFO_CSI_NUM_BF_ANT_SHIFT;
	temp_ht_cap = roam_ht_cap->uncompressedSteeringMatrixBFAntennae &
	    (TX_BF_CAP_INFO_UNCOMP_STEER_MAT_BF_ANT >>
	     TX_BF_CAP_INFO_UNCOMP_STEER_MAT_BF_ANT_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap <<
				TX_BF_CAP_INFO_UNCOMP_STEER_MAT_BF_ANT_SHIFT;
	temp_ht_cap = roam_ht_cap->compressedSteeringMatrixBFAntennae &
	    (TX_BF_CAP_INFO_COMP_STEER_MAT_BF_ANT >>
	     TX_BF_CAP_INFO_COMP_STEER_MAT_BF_ANT_SHIFT);
	if (temp_ht_cap)
		hdd_ht_cap->tx_BF_cap_info |=
			temp_ht_cap <<
				TX_BF_CAP_INFO_COMP_STEER_MAT_BF_ANT_SHIFT;

	/* antenna selection */
	if (roam_ht_cap->antennaSelection)
		hdd_ht_cap->antenna_selection_info |= ANTENNA_SEL_INFO;
	if (roam_ht_cap->explicitCSIFeedbackTx)
		hdd_ht_cap->antenna_selection_info |=
			ANTENNA_SEL_INFO_EXP_CSI_FB_TX;
	if (roam_ht_cap->antennaIndicesFeedbackTx)
		hdd_ht_cap->antenna_selection_info |=
			ANTENNA_SEL_INFO_ANT_ID_FB_TX;
	if (roam_ht_cap->explicitCSIFeedback)
		hdd_ht_cap->antenna_selection_info |=
			ANTENNA_SEL_INFO_EXP_CSI_FB;
	if (roam_ht_cap->antennaIndicesFeedback)
		hdd_ht_cap->antenna_selection_info |=
			ANTENNA_SEL_INFO_ANT_ID_FB;
	if (roam_ht_cap->rxAS)
		hdd_ht_cap->antenna_selection_info |=
			ANTENNA_SEL_INFO_RX_AS;
	if (roam_ht_cap->txSoundingPPDUs)
		hdd_ht_cap->antenna_selection_info |=
			ANTENNA_SEL_INFO_TX_SOUNDING_PPDU;

	/* mcs data rate */
	for (i = 0; i < IEEE80211_HT_MCS_MASK_LEN; ++i)
		hdd_ht_cap->mcs.rx_mask[i] =
			roam_ht_cap->supportedMCSSet[i];
		hdd_ht_cap->mcs.rx_highest =
			((short) (roam_ht_cap->supportedMCSSet[11]) << 8) |
			((short) (roam_ht_cap->supportedMCSSet[10]));
		hdd_ht_cap->mcs.tx_params =
			roam_ht_cap->supportedMCSSet[12];
}

#define VHT_CAP_MAX_MPDU_LENGTH_MASK 0x00000003
#define VHT_CAP_SUPP_CHAN_WIDTH_MASK_SHIFT 2
#define VHT_CAP_RXSTBC_MASK_SHIFT 8
#define VHT_CAP_BEAMFORMEE_STS_SHIFT 13
#define VHT_CAP_BEAMFORMEE_STS_MASK \
	(0x0000e000 >> VHT_CAP_BEAMFORMEE_STS_SHIFT)
#define VHT_CAP_SOUNDING_DIMENSIONS_SHIFT 16
#define VHT_CAP_SOUNDING_DIMENSIONS_MASK \
	(0x00070000 >> VHT_CAP_SOUNDING_DIMENSIONS_SHIFT)
#define VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK_SHIFT 23
#define VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK \
	(0x03800000 >> VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK_SHIFT)
#define VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB_SHIFT 26

/**
 * hdd_copy_ht_caps()- copy ht caps info from roam info to
 *  hdd station context.
 * @hdd_sta_ctx: pointer to hdd station context
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_copy_vht_caps(hdd_station_ctx_t *hdd_sta_ctx,
				     tCsrRoamInfo *roam_info)
{
	tDot11fIEVHTCaps *roam_vht_cap = &roam_info->vht_caps;
	struct ieee80211_vht_cap *hdd_vht_cap =
		&hdd_sta_ctx->conn_info.vht_caps;
	uint32_t temp_vht_cap;

	qdf_mem_zero(hdd_vht_cap, sizeof(struct ieee80211_vht_cap));

	temp_vht_cap = roam_vht_cap->maxMPDULen & VHT_CAP_MAX_MPDU_LENGTH_MASK;
	hdd_vht_cap->vht_cap_info |= temp_vht_cap;
	temp_vht_cap = roam_vht_cap->supportedChannelWidthSet &
		(IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK >>
			VHT_CAP_SUPP_CHAN_WIDTH_MASK_SHIFT);
	if (temp_vht_cap)
		if (roam_vht_cap->supportedChannelWidthSet &
		    (IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ >>
			VHT_CAP_SUPP_CHAN_WIDTH_MASK_SHIFT))
			hdd_vht_cap->vht_cap_info |=
				temp_vht_cap <<
				IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
		if (roam_vht_cap->supportedChannelWidthSet &
		    (IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ >>
			VHT_CAP_SUPP_CHAN_WIDTH_MASK_SHIFT))
			hdd_vht_cap->vht_cap_info |=
			temp_vht_cap <<
			IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ;
	if (roam_vht_cap->ldpcCodingCap)
		hdd_vht_cap->vht_cap_info |= IEEE80211_VHT_CAP_RXLDPC;
	if (roam_vht_cap->shortGI80MHz)
		hdd_vht_cap->vht_cap_info |= IEEE80211_VHT_CAP_SHORT_GI_80;
	if (roam_vht_cap->shortGI160and80plus80MHz)
		hdd_vht_cap->vht_cap_info |= IEEE80211_VHT_CAP_SHORT_GI_160;
	if (roam_vht_cap->txSTBC)
		hdd_vht_cap->vht_cap_info |= IEEE80211_VHT_CAP_TXSTBC;
	temp_vht_cap = roam_vht_cap->rxSTBC & (IEEE80211_VHT_CAP_RXSTBC_MASK >>
		VHT_CAP_RXSTBC_MASK_SHIFT);
	if (temp_vht_cap)
		hdd_vht_cap->vht_cap_info |=
			temp_vht_cap << VHT_CAP_RXSTBC_MASK_SHIFT;
	if (roam_vht_cap->suBeamFormerCap)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
	if (roam_vht_cap->suBeamformeeCap)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
	temp_vht_cap = roam_vht_cap->csnofBeamformerAntSup &
			(VHT_CAP_BEAMFORMEE_STS_MASK);
	if (temp_vht_cap)
		hdd_vht_cap->vht_cap_info |=
			temp_vht_cap << VHT_CAP_BEAMFORMEE_STS_SHIFT;
	temp_vht_cap = roam_vht_cap->numSoundingDim &
			(VHT_CAP_SOUNDING_DIMENSIONS_MASK);
	if (temp_vht_cap)
		hdd_vht_cap->vht_cap_info |=
			temp_vht_cap << VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
	if (roam_vht_cap->muBeamformerCap)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;
	if (roam_vht_cap->muBeamformeeCap)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
	if (roam_vht_cap->vhtTXOPPS)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_VHT_TXOP_PS;
	if (roam_vht_cap->htcVHTCap)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_HTC_VHT;
	temp_vht_cap = roam_vht_cap->maxAMPDULenExp &
			(VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK);
	if (temp_vht_cap)
		hdd_vht_cap->vht_cap_info |=
			temp_vht_cap <<
			VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK_SHIFT;
	temp_vht_cap = roam_vht_cap->vhtLinkAdaptCap &
		(IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB >>
		 VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB_SHIFT);
	if (temp_vht_cap)
		hdd_vht_cap->vht_cap_info |= temp_vht_cap <<
			VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB_SHIFT;
	if (roam_vht_cap->rxAntPattern)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN;
	if (roam_vht_cap->txAntPattern)
		hdd_vht_cap->vht_cap_info |=
			IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN;
	hdd_vht_cap->supp_mcs.rx_mcs_map = roam_vht_cap->rxMCSMap;
	hdd_vht_cap->supp_mcs.rx_highest =
		((uint16_t)roam_vht_cap->rxHighSupDataRate);
	hdd_vht_cap->supp_mcs.tx_mcs_map = roam_vht_cap->txMCSMap;
	hdd_vht_cap->supp_mcs.tx_highest =
		((uint16_t)roam_vht_cap->txSupDataRate);
}

/* ht param */
#define HT_PARAM_CONTROLLED_ACCESS_ONLY 0x10
#define HT_PARAM_SERVICE_INT_GRAN 0xe0
#define HT_PARAM_SERVICE_INT_GRAN_SHIFT 5

/* operatinon mode */
#define HT_OP_MODE_TX_BURST_LIMIT 0x0008

/* stbc_param */
#define HT_STBC_PARAM_MCS 0x007f

/**
 * hdd_copy_ht_operation()- copy HT operation element from roam info to
 *  hdd station context.
 * @hdd_sta_ctx: pointer to hdd station context
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_copy_ht_operation(hdd_station_ctx_t *hdd_sta_ctx,
					    tCsrRoamInfo *roam_info)
{
	tDot11fIEHTInfo *roam_ht_ops = &roam_info->ht_operation;
	struct ieee80211_ht_operation *hdd_ht_ops =
		&hdd_sta_ctx->conn_info.ht_operation;
	uint32_t i, temp_ht_ops;

	qdf_mem_zero(hdd_ht_ops, sizeof(struct ieee80211_ht_operation));

	hdd_ht_ops->primary_chan = roam_ht_ops->primaryChannel;

	/* HT_PARAMS */
	temp_ht_ops = roam_ht_ops->secondaryChannelOffset &
		IEEE80211_HT_PARAM_CHA_SEC_OFFSET;
	if (temp_ht_ops)
		hdd_ht_ops->ht_param |= temp_ht_ops;
	else
		hdd_ht_ops->ht_param = IEEE80211_HT_PARAM_CHA_SEC_NONE;
	if (roam_ht_ops->recommendedTxWidthSet)
		hdd_ht_ops->ht_param |= IEEE80211_HT_PARAM_CHAN_WIDTH_ANY;
	if (roam_ht_ops->rifsMode)
		hdd_ht_ops->ht_param |= IEEE80211_HT_PARAM_RIFS_MODE;
	if (roam_ht_ops->controlledAccessOnly)
		hdd_ht_ops->ht_param |= HT_PARAM_CONTROLLED_ACCESS_ONLY;
	temp_ht_ops = roam_ht_ops->serviceIntervalGranularity &
		(HT_PARAM_SERVICE_INT_GRAN >> HT_PARAM_SERVICE_INT_GRAN_SHIFT);
	if (temp_ht_ops)
		hdd_ht_ops->ht_param |= temp_ht_ops <<
			HT_PARAM_SERVICE_INT_GRAN_SHIFT;

	/* operation mode */
	temp_ht_ops = roam_ht_ops->opMode &
			IEEE80211_HT_OP_MODE_PROTECTION;
	switch (temp_ht_ops) {
	case IEEE80211_HT_OP_MODE_PROTECTION_NONMEMBER:
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_PROTECTION_NONMEMBER;
		break;
	case IEEE80211_HT_OP_MODE_PROTECTION_20MHZ:
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_PROTECTION_20MHZ;
		break;
	case IEEE80211_HT_OP_MODE_PROTECTION_NONHT_MIXED:
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_PROTECTION_NONHT_MIXED;
		break;
	case IEEE80211_HT_OP_MODE_PROTECTION_NONE:
	default:
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_PROTECTION_NONE;
	}
	if (roam_ht_ops->nonGFDevicesPresent)
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_NON_GF_STA_PRSNT;
	if (roam_ht_ops->transmitBurstLimit)
		hdd_ht_ops->operation_mode |=
			HT_OP_MODE_TX_BURST_LIMIT;
	if (roam_ht_ops->obssNonHTStaPresent)
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_NON_HT_STA_PRSNT;

	/* stbc_param */
	temp_ht_ops = roam_ht_ops->basicSTBCMCS &
			HT_STBC_PARAM_MCS;
	if (temp_ht_ops)
		hdd_ht_ops->stbc_param |= temp_ht_ops;
	if (roam_ht_ops->dualCTSProtection)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_DUAL_CTS_PROT;
	if (roam_ht_ops->secondaryBeacon)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_STBC_BEACON;
	if (roam_ht_ops->lsigTXOPProtectionFullSupport)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_LSIG_TXOP_FULLPROT;
	if (roam_ht_ops->pcoActive)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_PCO_ACTIVE;
	if (roam_ht_ops->pcoPhase)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_PCO_PHASE;

	/* basic MCs set */
	for (i = 0; i < 16; ++i)
		hdd_ht_ops->basic_set[i] =
			roam_ht_ops->basicMCSSet[i];
}

/**
 * hdd_copy_vht_operation()- copy VHT operations element from roam info to
 *  hdd station context.
 * @hdd_sta_ctx: pointer to hdd station context
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_copy_vht_operation(hdd_station_ctx_t *hdd_sta_ctx,
					      tCsrRoamInfo *roam_info)
{
	tDot11fIEVHTOperation *roam_vht_ops = &roam_info->vht_operation;
	struct ieee80211_vht_operation *hdd_vht_ops =
		&hdd_sta_ctx->conn_info.vht_operation;

	qdf_mem_zero(hdd_vht_ops, sizeof(struct ieee80211_vht_operation));

	hdd_vht_ops->chan_width = roam_vht_ops->chanWidth;
	hdd_vht_ops->center_freq_seg1_idx = roam_vht_ops->chanCenterFreqSeg1;
	hdd_vht_ops->center_freq_seg2_idx = roam_vht_ops->chanCenterFreqSeg2;
	hdd_vht_ops->basic_mcs_set = roam_vht_ops->basicMCSSet;
}


/**
 * hdd_save_bss_info() - save connection info in hdd sta ctx
 * @adapter: Pointer to adapter
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_save_bss_info(hdd_adapter_t *adapter,
						tCsrRoamInfo *roam_info)
{
	hdd_station_ctx_t *hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_sta_ctx->conn_info.freq = cds_chan_to_freq(
		hdd_sta_ctx->conn_info.operationChannel);
	if (roam_info->vht_caps.present) {
		hdd_sta_ctx->conn_info.conn_flag.vht_present = true;
		hdd_copy_vht_caps(hdd_sta_ctx, roam_info);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.vht_present = false;
	}
	if (roam_info->ht_caps.present) {
		hdd_sta_ctx->conn_info.conn_flag.ht_present = true;
		hdd_copy_ht_caps(hdd_sta_ctx, roam_info);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.ht_present = false;
	}
	if (roam_info->reassoc)
		hdd_sta_ctx->conn_info.roam_count++;
	if (roam_info->hs20vendor_ie.present) {
		hdd_sta_ctx->conn_info.conn_flag.hs20_present = true;
		qdf_mem_copy(&hdd_sta_ctx->conn_info.hs20vendor_ie,
			     &roam_info->hs20vendor_ie,
			     sizeof(roam_info->hs20vendor_ie));
	} else {
		hdd_sta_ctx->conn_info.conn_flag.hs20_present = false;
	}
	if (roam_info->ht_operation.present) {
		hdd_sta_ctx->conn_info.conn_flag.ht_op_present = true;
		hdd_copy_ht_operation(hdd_sta_ctx, roam_info);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.ht_op_present = false;
	}
	if (roam_info->vht_operation.present) {
		hdd_sta_ctx->conn_info.conn_flag.vht_op_present = true;
		hdd_copy_vht_operation(hdd_sta_ctx, roam_info);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.vht_op_present = false;
	}
}

/**
 * hdd_conn_save_connect_info() - save current connection information
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @eBssType: bss type
 *
 * Return: none
 */
static void
hdd_conn_save_connect_info(hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo,
			   eCsrRoamBssType eBssType)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	eCsrEncryptionType encryptType = eCSR_ENCRYPT_TYPE_NONE;

	QDF_ASSERT(pRoamInfo);

	if (pRoamInfo) {
		/* Save the BSSID for the connection */
		if (eCSR_BSS_TYPE_INFRASTRUCTURE == eBssType) {
			QDF_ASSERT(pRoamInfo->pBssDesc);
			qdf_copy_macaddr(&pHddStaCtx->conn_info.bssId,
					 &pRoamInfo->bssid);

			/*
			 * Save the Station ID for this station from
			 * the 'Roam Info'. For IBSS mode, staId is
			 * assigned in NEW_PEER_IND. For reassoc,
			 * the staID doesn't change and it may be invalid
			 * in this structure so no change here.
			 */
			if (!pRoamInfo->fReassocReq) {
				pHddStaCtx->conn_info.staId[0] =
					pRoamInfo->staId;
			}
		} else if (eCSR_BSS_TYPE_IBSS == eBssType) {
			qdf_copy_macaddr(&pHddStaCtx->conn_info.bssId,
					 &pRoamInfo->bssid);
		} else {
			/*
			 * can't happen. We need a valid IBSS or Infra setting
			 * in the BSSDescription or we can't function.
			 */
			QDF_ASSERT(0);
		}

		/* notify WMM */
		hdd_wmm_connect(pAdapter, pRoamInfo, eBssType);

		if (!pRoamInfo->u.pConnectedProfile) {
			QDF_ASSERT(pRoamInfo->u.pConnectedProfile);
		} else {
			/* Get Multicast Encryption Type */
			encryptType =
			    pRoamInfo->u.pConnectedProfile->mcEncryptionType;
			pHddStaCtx->conn_info.mcEncryptionType = encryptType;
			/* Get Unicast Encryption Type */
			encryptType =
				pRoamInfo->u.pConnectedProfile->EncryptionType;
			pHddStaCtx->conn_info.ucEncryptionType = encryptType;

			pHddStaCtx->conn_info.authType =
				pRoamInfo->u.pConnectedProfile->AuthType;

			pHddStaCtx->conn_info.operationChannel =
			    pRoamInfo->u.pConnectedProfile->operationChannel;

			/* Save the ssid for the connection */
			qdf_mem_copy(&pHddStaCtx->conn_info.SSID.SSID,
				     &pRoamInfo->u.pConnectedProfile->SSID,
				     sizeof(tSirMacSSid));

			/* Save dot11mode in which STA associated to AP */
			pHddStaCtx->conn_info.dot11Mode =
				pRoamInfo->u.pConnectedProfile->dot11Mode;

			pHddStaCtx->conn_info.proxyARPService =
				pRoamInfo->u.pConnectedProfile->proxyARPService;

			pHddStaCtx->conn_info.nss = pRoamInfo->chan_info.nss;

			pHddStaCtx->conn_info.rate_flags =
				pRoamInfo->chan_info.rate_flags;
		}
		hdd_save_bss_info(pAdapter, pRoamInfo);
	}
}

/**
 * hdd_send_ft_assoc_response() - send fast transition assoc response
 * @dev: pointer to net device
 * @pAdapter: pointer to adapter
 * @pCsrRoamInfo: pointer to roam info
 *
 * Send the 11R key information to the supplicant. Only then can the supplicant
 * generate the PMK-R1. (BTW, the ESE supplicant also needs the Assoc Resp IEs
 * for the same purpose.)
 *
 * Mainly the Assoc Rsp IEs are passed here. For the IMDA this contains the
 * R1KHID, R0KHID and the MDID. For FT, this consists of the Reassoc Rsp FTIEs.
 * This is the Assoc Response.
 *
 * Return: none
 */
static void
hdd_send_ft_assoc_response(struct net_device *dev,
			   hdd_adapter_t *pAdapter,
			   tCsrRoamInfo *pCsrRoamInfo)
{
	union iwreq_data wrqu;
	char *buff;
	unsigned int len = 0;
	u8 *pFTAssocRsp = NULL;

	if (pCsrRoamInfo->nAssocRspLength == 0) {
		hdd_debug("assoc rsp length is 0");
		return;
	}

	pFTAssocRsp =
		(u8 *) (pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength +
			pCsrRoamInfo->nAssocReqLength);
	if (pFTAssocRsp == NULL) {
		hdd_debug("AssocReq or AssocRsp is NULL");
		return;
	}
	/* pFTAssocRsp needs to point to the IEs */
	pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
	hdd_debug("AssocRsp is now at %02x%02x",
		   (unsigned int)pFTAssocRsp[0],
		   (unsigned int)pFTAssocRsp[1]);

	/* We need to send the IEs to the supplicant. */
	buff = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	if (buff == NULL) {
		hdd_err("unable to allocate memory");
		return;
	}
	/* Send the Assoc Resp, the supplicant needs this for initial Auth. */
	len = pCsrRoamInfo->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
	wrqu.data.length = len;
	memcpy(buff, pFTAssocRsp, len);
	wireless_send_event(dev, IWEVASSOCRESPIE, &wrqu, buff);

	qdf_mem_free(buff);
}

/**
 * hdd_send_ft_event() - send fast transition event
 * @pAdapter: pointer to adapter
 *
 * Send the FTIEs, RIC IEs during FT. This is eventually used to send the
 * FT events to the supplicant. At the reception of Auth2 we send the RIC
 * followed by the auth response IEs to the supplicant.
 * Once both are received in the supplicant, an FT event is generated
 * to the supplicant.
 *
 * Return: none
 */
static void hdd_send_ft_event(hdd_adapter_t *pAdapter)
{
	uint16_t auth_resp_len = 0;
	uint32_t ric_ies_length = 0;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

#if defined(KERNEL_SUPPORT_11R_CFG80211)
	struct cfg80211_ft_event_params ftEvent;
	uint8_t ftIe[DOT11F_IE_FTINFO_MAX_LEN];
	uint8_t ricIe[DOT11F_IE_RICDESCRIPTOR_MAX_LEN];
	struct net_device *dev = pAdapter->dev;
#else
	char *buff;
	union iwreq_data wrqu;
	uint16_t str_len;
#endif

#if defined(KERNEL_SUPPORT_11R_CFG80211)
	qdf_mem_zero(ftIe, DOT11F_IE_FTINFO_MAX_LEN);
	qdf_mem_zero(ricIe, DOT11F_IE_RICDESCRIPTOR_MAX_LEN);

	sme_get_rici_es(pHddCtx->hHal, pAdapter->sessionId, (u8 *) ricIe,
			DOT11F_IE_RICDESCRIPTOR_MAX_LEN, &ric_ies_length);
	if (ric_ies_length == 0)
		hdd_warn("Do not send RIC IEs as length is 0");

	ftEvent.ric_ies = ricIe;
	ftEvent.ric_ies_len = ric_ies_length;
	hdd_debug("RIC IEs is of length %d", (int)ric_ies_length);

	sme_get_ft_pre_auth_response(pHddCtx->hHal, pAdapter->sessionId,
				     (u8 *) ftIe, DOT11F_IE_FTINFO_MAX_LEN,
				     &auth_resp_len);

	if (auth_resp_len == 0) {
		hdd_debug("AuthRsp FTIES is of length 0");
		return;
	}

	sme_set_ft_pre_auth_state(pHddCtx->hHal, pAdapter->sessionId, true);

	ftEvent.target_ap = ftIe;

	ftEvent.ies = (u8 *) (ftIe + QDF_MAC_ADDR_SIZE);
	ftEvent.ies_len = auth_resp_len - QDF_MAC_ADDR_SIZE;

	hdd_debug("ftEvent.ies_len %zu", ftEvent.ies_len);
	hdd_debug("ftEvent.ric_ies_len %zu", ftEvent.ric_ies_len);
	hdd_debug("ftEvent.target_ap %2x-%2x-%2x-%2x-%2x-%2x",
	       ftEvent.target_ap[0], ftEvent.target_ap[1],
	       ftEvent.target_ap[2], ftEvent.target_ap[3], ftEvent.target_ap[4],
	       ftEvent.target_ap[5]);

	(void)cfg80211_ft_event(dev, &ftEvent);

#else
	/* We need to send the IEs to the supplicant */
	buff = qdf_mem_malloc(IW_CUSTOM_MAX);
	if (buff == NULL) {
		hdd_err("unable to allocate memory");
		return;
	}

	/* Sme needs to send the RIC IEs first */
	str_len = strlcpy(buff, "RIC=", IW_CUSTOM_MAX);
	sme_get_rici_es(pHddCtx->hHal, pAdapter->sessionId,
			(u8 *) &(buff[str_len]), (IW_CUSTOM_MAX - str_len),
			&ric_ies_length);
	if (ric_ies_length == 0) {
		hdd_warn("Do not send RIC IEs as length is 0");
	} else {
		wrqu.data.length = str_len + ric_ies_length;
		wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);
	}

	/* Sme needs to provide the Auth Resp */
	qdf_mem_zero(buff, IW_CUSTOM_MAX);
	str_len = strlcpy(buff, "AUTH=", IW_CUSTOM_MAX);
	sme_get_ft_pre_auth_response(pHddCtx->hHal, pAdapter->sessionId,
				     (u8 *) &buff[str_len],
				     (IW_CUSTOM_MAX - str_len), &auth_resp_len);

	if (auth_resp_len == 0) {
		qdf_mem_free(buff);
		hdd_debug("AuthRsp FTIES is of length 0");
		return;
	}

	wrqu.data.length = str_len + auth_resp_len;
	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);

	qdf_mem_free(buff);
#endif
}

#ifdef FEATURE_WLAN_ESE
/**
 * hdd_send_new_ap_channel_info() - send new ap channel info
 * @dev: pointer to net device
 * @pAdapter: pointer to adapter
 * @pCsrRoamInfo: pointer to roam info
 *
 * Send the ESE required "new AP Channel info" to the supplicant.
 * (This keeps the supplicant "up to date" on the current channel.)
 *
 * The current (new AP) channel information is passed in.
 *
 * Return: none
 */
static void
hdd_send_new_ap_channel_info(struct net_device *dev, hdd_adapter_t *pAdapter,
			     tCsrRoamInfo *pCsrRoamInfo)
{
	union iwreq_data wrqu;
	tSirBssDescription *descriptor = pCsrRoamInfo->pBssDesc;

	if (descriptor == NULL) {
		hdd_err("bss descriptor is null");
		return;
	}
	/*
	 * Send the Channel event, the supplicant needs this to generate
	 * the Adjacent AP report.
	 */
	hdd_debug("Sending up an SIOCGIWFREQ, channelId: %d",
		 descriptor->channelId);
	memset(&wrqu, '\0', sizeof(wrqu));
	wrqu.freq.m = descriptor->channelId;
	wrqu.freq.e = 0;
	wrqu.freq.i = 0;
	wireless_send_event(pAdapter->dev, SIOCGIWFREQ, &wrqu, NULL);
}

#endif /* FEATURE_WLAN_ESE */

/**
 * hdd_send_update_beacon_ies_event() - send update beacons ie event
 * @pAdapter: pointer to adapter
 * @pCsrRoamInfo: pointer to roam info
 *
 * Return: none
 */
static void
hdd_send_update_beacon_ies_event(hdd_adapter_t *pAdapter,
				  tCsrRoamInfo *pCsrRoamInfo)
{
	union iwreq_data wrqu;
	u8 *pBeaconIes;
	u8 currentLen = 0;
	char *buff;
	int totalIeLen = 0, currentOffset = 0, strLen;

	memset(&wrqu, '\0', sizeof(wrqu));

	if (0 == pCsrRoamInfo->nBeaconLength) {
		hdd_debug("beacon frame length is 0");
		return;
	}
	pBeaconIes = (u8 *) (pCsrRoamInfo->pbFrames + BEACON_FRAME_IES_OFFSET);
	if (pBeaconIes == NULL) {
		hdd_warn("Beacon IEs is NULL");
		return;
	}
	/* pBeaconIes needs to point to the IEs */
	hdd_debug("Beacon IEs is now at %02x%02x",
		   (unsigned int)pBeaconIes[0],
		   (unsigned int)pBeaconIes[1]);
	hdd_debug("Beacon IEs length = %d",
		   pCsrRoamInfo->nBeaconLength - BEACON_FRAME_IES_OFFSET);

	/* We need to send the IEs to the supplicant. */
	buff = qdf_mem_malloc(IW_CUSTOM_MAX);
	if (buff == NULL) {
		hdd_err("unable to allocate memory");
		return;
	}

	strLen = strlcpy(buff, "BEACONIEs=", IW_CUSTOM_MAX);
	currentLen = strLen + 1;

	totalIeLen = pCsrRoamInfo->nBeaconLength - BEACON_FRAME_IES_OFFSET;
	do {
		/*
		 * If the beacon size exceeds max CUSTOM event size, break it
		 * into chunks of CUSTOM event max size and send it to
		 * supplicant. Changes are done in supplicant to handle this.
		 */
		qdf_mem_zero(&buff[strLen + 1], IW_CUSTOM_MAX - (strLen + 1));
		currentLen =
			QDF_MIN(totalIeLen, IW_CUSTOM_MAX - (strLen + 1) - 1);
		qdf_mem_copy(&buff[strLen + 1], pBeaconIes + currentOffset,
			     currentLen);
		currentOffset += currentLen;
		totalIeLen -= currentLen;
		wrqu.data.length = strLen + 1 + currentLen;
		if (totalIeLen)
			buff[strLen] = 1; /* more chunks pending */
		else
			buff[strLen] = 0; /* last chunk */

		hdd_debug("Beacon IEs length to supplicant = %d",
			   currentLen);
		wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);
	} while (totalIeLen > 0);

	qdf_mem_free(buff);
}

/**
 * hdd_send_association_event() - send association event
 * @dev: pointer to net device
 * @pCsrRoamInfo: pointer to roam info
 *
 * Return: none
 */
static void hdd_send_association_event(struct net_device *dev,
				       tCsrRoamInfo *pCsrRoamInfo)
{
	int ret;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	union iwreq_data wrqu;
	int we_event;
	char *msg;
	struct qdf_mac_addr peerMacAddr;

	/* Added to find the auth type on the fly at run time */
	/* rather than with cfg to see if FT is enabled */
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);

	memset(&wrqu, '\0', sizeof(wrqu));
	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	we_event = SIOCGIWAP;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (NULL != pCsrRoamInfo)
		if (pCsrRoamInfo->roamSynchInProgress) {
			/* change logging before release */
			hdd_debug("LFR3:hdd_send_association_event");
			/* Update tdls module about the disconnection event */
			hdd_notify_sta_disconnect(pAdapter->sessionId,
						 true, pAdapter->hdd_vdev);
			wlan_hdd_tdls_notify_disconnect(pAdapter, true);
		}
#endif
	if (eConnectionState_Associated == pHddStaCtx->conn_info.connState) {
		tSirSmeChanInfo chan_info = {0};

		if (!pCsrRoamInfo) {
			hdd_warn("STA in associated state but pCsrRoamInfo is null");
			return;
		}

		if (!hdd_is_roam_sync_in_progress(pCsrRoamInfo))
			policy_mgr_incr_active_session(pHddCtx->hdd_psoc,
				pAdapter->device_mode, pAdapter->sessionId);
		memcpy(wrqu.ap_addr.sa_data, pCsrRoamInfo->pBssDesc->bssId,
		       sizeof(pCsrRoamInfo->pBssDesc->bssId));

#ifdef WLAN_FEATURE_P2P_DEBUG
		if (pAdapter->device_mode == QDF_P2P_CLIENT_MODE) {
			if (global_p2p_connection_status ==
			    P2P_CLIENT_CONNECTING_STATE_1) {
				global_p2p_connection_status =
					P2P_CLIENT_CONNECTED_STATE_1;
				hdd_debug("[P2P State] Changing state from Connecting state to Connected State for 8-way Handshake");
			} else if (global_p2p_connection_status ==
				   P2P_CLIENT_CONNECTING_STATE_2) {
				global_p2p_connection_status =
					P2P_CLIENT_COMPLETED_STATE;
				hdd_debug("[P2P State] Changing state from Connecting state to P2P Client Connection Completed");
			}
		}
#endif
		pr_info("wlan: " MAC_ADDRESS_STR " connected to "
			MAC_ADDRESS_STR "\n",
			MAC_ADDR_ARRAY(pAdapter->macAddressCurrent.bytes),
			MAC_ADDR_ARRAY(wrqu.ap_addr.sa_data));
		hdd_send_update_beacon_ies_event(pAdapter, pCsrRoamInfo);

		/*
		 * Send IWEVASSOCRESPIE Event if WLAN_FEATURE_CIQ_METRICS
		 * is Enabled Or Send IWEVASSOCRESPIE Event if
		 * fFTEnable is true.
		 * Send FT Keys to the supplicant when FT is enabled
		 */
		if ((pRoamProfile->AuthType.authType[0] ==
		     eCSR_AUTH_TYPE_FT_RSN_PSK)
		    || (pRoamProfile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_FT_RSN)
#ifdef FEATURE_WLAN_ESE
		    || (pRoamProfile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_CCKM_RSN)
		    || (pRoamProfile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_CCKM_WPA)
#endif
		    ) {
			hdd_send_ft_assoc_response(dev, pAdapter, pCsrRoamInfo);
		}
		qdf_copy_macaddr(&peerMacAddr,
				 &pHddStaCtx->conn_info.bssId);
		chan_info.chan_id = pCsrRoamInfo->chan_info.chan_id;
		chan_info.mhz = pCsrRoamInfo->chan_info.mhz;
		chan_info.info = pCsrRoamInfo->chan_info.info;
		chan_info.band_center_freq1 =
			pCsrRoamInfo->chan_info.band_center_freq1;
		chan_info.band_center_freq2 =
			pCsrRoamInfo->chan_info.band_center_freq2;
		chan_info.reg_info_1 =
			pCsrRoamInfo->chan_info.reg_info_1;
		chan_info.reg_info_2 =
			pCsrRoamInfo->chan_info.reg_info_2;

		ret = hdd_objmgr_add_peer_object(pAdapter->hdd_vdev,
						 pAdapter->device_mode,
						 peerMacAddr.bytes,
						 false);
		if (ret)
			hdd_err("Peer object "MAC_ADDRESS_STR" add fails!",
					MAC_ADDR_ARRAY(peerMacAddr.bytes));
		ret = hdd_objmgr_set_peer_mlme_state(pAdapter->hdd_vdev,
						     WLAN_ASSOC_STATE);
		if (ret)
			hdd_err("Peer object %pM fail to set associated state",
					peerMacAddr.bytes);

		/* send peer status indication to oem app */
		hdd_send_peer_status_ind_to_app(&peerMacAddr,
						ePeerConnected,
						pCsrRoamInfo->timingMeasCap,
						pAdapter->sessionId, &chan_info,
						pAdapter->device_mode);
		/* Update tdls module about connection event */
		hdd_notify_sta_connect(pAdapter->sessionId,
				       pCsrRoamInfo->tdls_chan_swit_prohibited,
				       pCsrRoamInfo->tdls_prohibited,
				       pAdapter->hdd_vdev);

		wlan_hdd_tdls_notify_connect(pAdapter, pCsrRoamInfo);


#ifdef MSM_PLATFORM
#if defined(CONFIG_ICNSS) || defined(CONFIG_CNSS)
		/* start timer in sta/p2p_cli */
		spin_lock_bh(&pHddCtx->bus_bw_lock);
		pAdapter->prev_tx_packets = pAdapter->stats.tx_packets;
		pAdapter->prev_rx_packets = pAdapter->stats.rx_packets;
		cdp_get_intra_bss_fwd_pkts_count(
			cds_get_context(QDF_MODULE_ID_SOC), pAdapter->sessionId,
			&pAdapter->prev_fwd_tx_packets,
			&pAdapter->prev_fwd_rx_packets);
		spin_unlock_bh(&pHddCtx->bus_bw_lock);
		hdd_bus_bw_compute_timer_start(pHddCtx);
#endif
#endif
	} else if (eConnectionState_IbssConnected ==    /* IBss Associated */
			pHddStaCtx->conn_info.connState) {
		policy_mgr_update_connection_info(pHddCtx->hdd_psoc,
				pAdapter->sessionId);
		memcpy(wrqu.ap_addr.sa_data, pHddStaCtx->conn_info.bssId.bytes,
				ETH_ALEN);
		hdd_info("wlan: new IBSS connection to " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(pHddStaCtx->conn_info.bssId.bytes));

		ret = hdd_objmgr_add_peer_object(pAdapter->hdd_vdev,
						 QDF_IBSS_MODE,
						 pCsrRoamInfo->bssid.bytes,
						 false);
		if (ret)
			hdd_err("Peer object "MAC_ADDRESS_STR" add fails!",
				MAC_ADDR_ARRAY(pCsrRoamInfo->bssid.bytes));
		ret = hdd_objmgr_set_peer_mlme_state(pAdapter->hdd_vdev,
						     WLAN_ASSOC_STATE);
		if (ret)
			hdd_err("Peer object %pM fail to set associated state",
					peerMacAddr.bytes);

	} else {                /* Not Associated */
		hdd_info("wlan: disconnected");
		memset(wrqu.ap_addr.sa_data, '\0', ETH_ALEN);
		policy_mgr_decr_session_set_pcl(pHddCtx->hdd_psoc,
				pAdapter->device_mode, pAdapter->sessionId);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif

		if ((pAdapter->device_mode == QDF_STA_MODE) ||
			(pAdapter->device_mode == QDF_P2P_CLIENT_MODE)) {
			qdf_copy_macaddr(&peerMacAddr,
					 &pHddStaCtx->conn_info.bssId);

			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_app(&peerMacAddr,
							ePeerDisconnected, 0,
							pAdapter->sessionId,
							NULL,
							pAdapter->device_mode);
		}

		ret = hdd_objmgr_remove_peer_object(pAdapter->hdd_vdev,
						    peerMacAddr.bytes);
		if (ret)
			hdd_err("Peer obj "MAC_ADDRESS_STR" delete fails",
					MAC_ADDR_ARRAY(peerMacAddr.bytes));

		hdd_lpass_notify_disconnect(pAdapter);
		/* Update tdls module about the disconnection event */
		hdd_notify_sta_disconnect(pAdapter->sessionId,
					  false,
					  pAdapter->hdd_vdev);
		wlan_hdd_tdls_notify_disconnect(pAdapter, false);

#ifdef MSM_PLATFORM
		/* stop timer in sta/p2p_cli */
		spin_lock_bh(&pHddCtx->bus_bw_lock);
		pAdapter->prev_tx_packets = 0;
		pAdapter->prev_rx_packets = 0;
		pAdapter->prev_fwd_tx_packets = 0;
		pAdapter->prev_fwd_rx_packets = 0;
		spin_unlock_bh(&pHddCtx->bus_bw_lock);
		hdd_bus_bw_compute_timer_try_stop(pHddCtx);
#endif
	}
	hdd_ipa_set_tx_flow_info();
	/* Send SCC/MCC Switching event to IPA */
	hdd_ipa_send_mcc_scc_msg(pHddCtx, pHddCtx->mcc_mode);

	msg = NULL;
	/* During the WLAN uninitialization,supplicant is stopped before the
	 * driver so not sending the status of the connection to supplicant
	 */
	if (cds_is_load_or_unload_in_progress()) {
		wireless_send_event(dev, we_event, &wrqu, msg);
#ifdef FEATURE_WLAN_ESE
		if (eConnectionState_Associated ==
			 pHddStaCtx->conn_info.connState) {
			if ((pRoamProfile->AuthType.authType[0] ==
			     eCSR_AUTH_TYPE_CCKM_RSN) ||
			    (pRoamProfile->AuthType.authType[0] ==
				eCSR_AUTH_TYPE_CCKM_WPA))
				hdd_send_new_ap_channel_info(dev, pAdapter,
							     pCsrRoamInfo);
		}
#endif
	}
}

/**
 * hdd_conn_remove_connect_info() - remove connection info
 * @pHddStaCtx: pointer to global HDD station context
 * @pCsrRoamInfo: pointer to roam info
 *
 * Return: none
 */
static void hdd_conn_remove_connect_info(hdd_station_ctx_t *pHddStaCtx)
{
	/* Remove staId, bssId and peerMacAddress */
	pHddStaCtx->conn_info.staId[0] = 0;
	qdf_mem_zero(&pHddStaCtx->conn_info.bssId, QDF_MAC_ADDR_SIZE);
	qdf_mem_zero(&pHddStaCtx->conn_info.peerMacAddress[0],
		     QDF_MAC_ADDR_SIZE);

	/* Clear all security settings */
	pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	pHddStaCtx->conn_info.mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
	pHddStaCtx->conn_info.ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;

	qdf_mem_zero(&pHddStaCtx->conn_info.Keys, sizeof(tCsrKeys));
	qdf_mem_zero(&pHddStaCtx->ibss_enc_key, sizeof(tCsrRoamSetKey));

	pHddStaCtx->conn_info.proxyARPService = 0;

	qdf_mem_zero(&pHddStaCtx->conn_info.SSID, sizeof(tCsrSSIDInfo));
}

/**
 * hdd_roam_deregister_sta() - deregister station
 * @pAdapter: pointer to adapter
 * @staId: station identifier
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_roam_deregister_sta(hdd_adapter_t *adapter, uint8_t staid)
{
	QDF_STATUS qdf_status;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int ret = 0;
	uint8_t *peer_mac = NULL;
	struct qdf_mac_addr broadcastMacAddr =
				QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	if (eConnectionState_IbssDisconnected ==
	    pHddStaCtx->conn_info.connState) {
		/*
		 * Do not set the carrier off when the last peer leaves.
		 * We will set the carrier off while stopping the IBSS.
		 */
	}

	qdf_status = cdp_clear_peer(cds_get_context(QDF_MODULE_ID_SOC),
			(struct cdp_pdev *)cds_get_context(QDF_MODULE_ID_TXRX),
			staid);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_clear_peer() failed for staid %d. Status(%d) [0x%08X]",
			staid, qdf_status, qdf_status);
	}

	if (adapter->device_mode == QDF_STA_MODE) {
		peer_mac = pHddStaCtx->conn_info.bssId.bytes;
	} else if (adapter->device_mode == QDF_IBSS_MODE) {
		if (pHddStaCtx->broadcast_staid == staid)
			peer_mac = broadcastMacAddr.bytes;
		else
			peer_mac = pHddStaCtx->conn_info.
					peerMacAddress[staid].bytes;
	}

	ret = hdd_objmgr_remove_peer_object(adapter->hdd_vdev, peer_mac);
	if (ret) {
		hdd_err("Peer obj %pM delete fails", peer_mac);
		return QDF_STATUS_E_FAILURE;
	}
	return qdf_status;
}

/**
 * hdd_print_bss_info() - print bss info
 * @hdd_sta_ctx: pointer to hdd station context
 *
 * Return: None
 */
static void hdd_print_bss_info(hdd_station_ctx_t *hdd_sta_ctx)
{
	uint32_t *cap_info;

	hdd_info("WIFI DATA LOGGER");
	hdd_info("channel: %d",
		 hdd_sta_ctx->conn_info.freq);
	hdd_info("dot11mode: %d",
		 hdd_sta_ctx->conn_info.dot11Mode);
	hdd_info("AKM: %d",
		 hdd_sta_ctx->conn_info.authType);
	hdd_info("ssid: %.*s",
		 hdd_sta_ctx->conn_info.SSID.SSID.length,
		 hdd_sta_ctx->conn_info.SSID.SSID.ssId);
	hdd_info("roam count: %d",
		 hdd_sta_ctx->conn_info.roam_count);
	hdd_info("ant_info: %d",
		 hdd_sta_ctx->conn_info.txrate.nss);
	hdd_info("datarate legacy %d",
		 hdd_sta_ctx->conn_info.txrate.legacy);
	hdd_info("datarate mcs: %d",
		 hdd_sta_ctx->conn_info.txrate.mcs);
	if (hdd_sta_ctx->conn_info.conn_flag.ht_present) {
		cap_info = (uint32_t *)&hdd_sta_ctx->conn_info.ht_caps;
		hdd_info("ht caps: %x", *cap_info);
	}
	if (hdd_sta_ctx->conn_info.conn_flag.vht_present) {
		cap_info = (uint32_t *)&hdd_sta_ctx->conn_info.vht_caps;
		hdd_info("vht caps: %x", *cap_info);
	}
	if (hdd_sta_ctx->conn_info.conn_flag.hs20_present)
		hdd_info("hs20 info: %x",
			 hdd_sta_ctx->conn_info.hs20vendor_ie.release_num);
	hdd_info("signal: %d",
		 hdd_sta_ctx->conn_info.signal);
	hdd_info("noise: %d",
		 hdd_sta_ctx->conn_info.noise);
}

/**
 * hdd_dis_connect_handler() - disconnect event handler
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam identifier
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * This function handles disconnect event:
 * 1. Disable transmit queues;
 * 2. Clean up internal connection states and data structures;
 * 3. Send disconnect indication to supplicant.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS hdd_dis_connect_handler(hdd_adapter_t *pAdapter,
					  tCsrRoamInfo *pRoamInfo,
					  uint32_t roamId,
					  eRoamCmdStatus roamStatus,
					  eCsrRoamResult roamResult)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS vstatus;
	struct net_device *dev = pAdapter->dev;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	uint8_t sta_id;
	bool sendDisconInd = true;

	if (dev == NULL) {
		hdd_err("net_dev is released return");
		return QDF_STATUS_E_FAILURE;
	}
	/* notify apps that we can't pass traffic anymore */
	hdd_notice("Disabling queues");
	wlan_hdd_netif_queue_control(pAdapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	if (hdd_ipa_is_enabled(pHddCtx))
		hdd_ipa_wlan_evt(pAdapter, pHddStaCtx->conn_info.staId[0],
				HDD_IPA_STA_DISCONNECT,
				pHddStaCtx->conn_info.bssId.bytes);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif

	DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
				pAdapter->sessionId,
				QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_DISASSOC));

	/* HDD has initiated disconnect, do not send disconnect indication
	 * to kernel. Sending disconnected event to kernel for userspace
	 * initiated disconnect will be handled by disconnect handler call
	 * to cfg80211_disconnected.
	 */
	if ((eConnectionState_Disconnecting ==
	    pHddStaCtx->conn_info.connState) ||
	    (eConnectionState_NotConnected ==
	    pHddStaCtx->conn_info.connState)) {
		hdd_debug("HDD has initiated a disconnect, no need to send disconnect indication to kernel");
		sendDisconInd = false;
	}

	if (pHddStaCtx->conn_info.connState != eConnectionState_Disconnecting) {
		INIT_COMPLETION(pAdapter->disconnect_comp_var);
		hdd_conn_set_connection_state(pAdapter,
					      eConnectionState_Disconnecting);
	}

	hdd_clear_roam_profile_ie(pAdapter);
	hdd_wmm_init(pAdapter);
	hdd_debug("Invoking packetdump deregistration API");
	wlan_deregister_txrx_packetdump();

	/* indicate 'disconnect' status to wpa_supplicant... */
	hdd_send_association_event(dev, pRoamInfo);
	/* indicate disconnected event to nl80211 */
	if (roamStatus != eCSR_ROAM_IBSS_LEAVE) {
		/*
		 * Only send indication to kernel if not initiated
		 * by kernel
		 */
		if (sendDisconInd) {
			/*
			 * To avoid wpa_supplicant sending "HANGED" CMD
			 * to ICS UI.
			 */
			if (eCSR_ROAM_LOSTLINK == roamStatus) {
				if (pRoamInfo->reasonCode ==
				    eSIR_MAC_PEER_STA_REQ_LEAVING_BSS_REASON)
					pr_info("wlan: disconnected due to poor signal, rssi is %d dB\n", pRoamInfo->rxRssi);
				wlan_hdd_cfg80211_indicate_disconnect(
							dev, false,
							pRoamInfo->reasonCode);
			} else {
				wlan_hdd_cfg80211_indicate_disconnect(
							dev, false,
							WLAN_REASON_UNSPECIFIED
							);
			}

			hdd_info("sent disconnected event to nl80211, reason code %d",
				(eCSR_ROAM_LOSTLINK == roamStatus) ?
				pRoamInfo->reasonCode :
				WLAN_REASON_UNSPECIFIED);
		}
		/*
		 * During the WLAN uninitialization,supplicant is stopped
		 * before the driver so not sending the status of the
		 * connection to supplicant.
		 */
		if (cds_is_load_or_unload_in_progress()) {
#ifdef WLAN_FEATURE_P2P_DEBUG
			if (pAdapter->device_mode == QDF_P2P_CLIENT_MODE) {
				if (global_p2p_connection_status ==
				    P2P_CLIENT_CONNECTED_STATE_1) {
					global_p2p_connection_status =
						P2P_CLIENT_DISCONNECTED_STATE;
					hdd_debug("[P2P State] 8 way Handshake completed and moved to disconnected state");
				} else if (global_p2p_connection_status ==
					   P2P_CLIENT_COMPLETED_STATE) {
					global_p2p_connection_status =
						P2P_NOT_ACTIVE;
					hdd_debug("[P2P State] P2P Client is removed and moved to inactive state");
				}
			}
#endif

		}
	}

	hdd_wmm_adapter_clear(pAdapter);
	sme_ft_reset(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId);
	if (hdd_remove_beacon_filter(pAdapter) != 0)
		hdd_err("hdd_remove_beacon_filter() failed");

	if (eCSR_ROAM_IBSS_LEAVE == roamStatus) {
		uint8_t i;

		sta_id = pHddStaCtx->broadcast_staid;
		vstatus = hdd_roam_deregister_sta(pAdapter, sta_id);
		if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
			hdd_err("hdd_roam_deregister_sta() failed for staID %d Status: %d [0x%x]",
					sta_id, status, status);
			status = QDF_STATUS_E_FAILURE;
		}
		pHddCtx->sta_to_adapter[sta_id] = NULL;
		/* Clear all the peer sta register with TL. */
		for (i = 0; i < MAX_PEERS; i++) {
			if (HDD_WLAN_INVALID_STA_ID ==
				pHddStaCtx->conn_info.staId[i])
				continue;
			sta_id = pHddStaCtx->conn_info.staId[i];
			hdd_debug("Deregister StaID %d", sta_id);
			vstatus = hdd_roam_deregister_sta(pAdapter, sta_id);
			if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
				hdd_err("hdd_roam_deregister_sta() failed to for staID %d Status: %d [0x%x]",
					sta_id, status, status);
				status = QDF_STATUS_E_FAILURE;
			}
			/* set the staid and peer mac as 0, all other
			 * reset are done in hdd_connRemoveConnectInfo.
			 */
			pHddStaCtx->conn_info.staId[i] = 0;
			qdf_mem_zero(&pHddStaCtx->conn_info.peerMacAddress[i],
				sizeof(struct qdf_mac_addr));
			if (sta_id < (WLAN_MAX_STA_COUNT + 3))
				pHddCtx->sta_to_adapter[sta_id] = NULL;
		}
	} else {
		sta_id = pHddStaCtx->conn_info.staId[0];
		hdd_debug("roamResult: %d", roamResult);

		/* clear scan cache for Link Lost */
		if (eCSR_ROAM_RESULT_DEAUTH_IND == roamResult ||
		    eCSR_ROAM_RESULT_DISASSOC_IND == roamResult ||
		    eCSR_ROAM_LOSTLINK == roamStatus) {
			wlan_hdd_cfg80211_update_bss_list(pAdapter,
				pHddStaCtx->conn_info.bssId.bytes);
			sme_remove_bssid_from_scan_list(pHddCtx->hHal,
			pHddStaCtx->conn_info.bssId.bytes);
		}
		pHddCtx->sta_to_adapter[sta_id] = NULL;
	}
	/* Clear saved connection information in HDD */
	hdd_conn_remove_connect_info(pHddStaCtx);
	hdd_conn_set_connection_state(pAdapter, eConnectionState_NotConnected);
	pmo_ucfg_flush_gtk_offload_req(pAdapter->hdd_vdev);

#ifdef FEATURE_WLAN_TDLS
	if (eCSR_ROAM_IBSS_LEAVE != roamStatus)
		wlan_hdd_tdls_disconnection_callback(pAdapter);
#endif

	if ((QDF_STA_MODE == pAdapter->device_mode) ||
			(QDF_P2P_CLIENT_MODE == pAdapter->device_mode)) {
		sme_ps_disable_auto_ps_timer(WLAN_HDD_GET_HAL_CTX
				(pAdapter),
				pAdapter->sessionId);
	}
	wlan_hdd_clear_link_layer_stats(pAdapter);
	/* Unblock anyone waiting for disconnect to complete */
	complete(&pAdapter->disconnect_comp_var);
	hdd_print_bss_info(pHddStaCtx);
	return status;
}

/**
 * hdd_set_peer_authorized_event() - set peer_authorized_event
 * @vdev_id: vdevid
 *
 * Return: None
 */
static void hdd_set_peer_authorized_event(uint32_t vdev_id)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	hdd_adapter_t *adapter = NULL;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (adapter == NULL) {
		hdd_err("Invalid vdev_id");
		return;
	}
	complete(&adapter->sta_authorized_event);
}

/**
 * hdd_change_peer_state() - change peer state
 * @pAdapter: HDD adapter
 * @sta_state: peer state
 * @roam_synch_in_progress: roam synch in progress
 *
 * Return: QDF status
 */
QDF_STATUS hdd_change_peer_state(hdd_adapter_t *pAdapter,
				 uint8_t sta_id,
				 enum ol_txrx_peer_state sta_state,
				 bool roam_synch_in_progress)
{
	QDF_STATUS err;
	uint8_t *peer_mac_addr;
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *peer;

	if (!pdev) {
		hdd_err("Failed to get txrx context");
		return QDF_STATUS_E_FAULT;
	}

	if (sta_id >= WLAN_MAX_STA_COUNT) {
		hdd_err("Invalid sta id: %d", sta_id);
		return QDF_STATUS_E_INVAL;
	}

	peer = cdp_peer_find_by_local_id(soc,
			(struct cdp_pdev *)pdev, sta_id);
	if (!peer)
		return QDF_STATUS_E_FAULT;

	peer_mac_addr = cdp_peer_get_peer_mac_addr(soc, peer);
	if (peer_mac_addr == NULL) {
		hdd_err("peer mac addr is NULL");
		return QDF_STATUS_E_FAULT;
	}

	err = cdp_peer_state_update(soc, pdev, peer_mac_addr, sta_state);
	if (err != QDF_STATUS_SUCCESS) {
		hdd_err("peer state update failed");
		return QDF_STATUS_E_FAULT;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (roam_synch_in_progress)
		return QDF_STATUS_SUCCESS;
#endif

	if (sta_state == OL_TXRX_PEER_STATE_AUTH) {
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
		/* make sure event is reset */
		INIT_COMPLETION(pAdapter->sta_authorized_event);
#endif

		err = sme_set_peer_authorized(peer_mac_addr,
				hdd_set_peer_authorized_event,
				pAdapter->sessionId);
		if (err != QDF_STATUS_SUCCESS) {
			hdd_err("Failed to set the peer state to authorized");
			return QDF_STATUS_E_FAULT;
		}

		if (pAdapter->device_mode == QDF_STA_MODE ||
		    pAdapter->device_mode == QDF_P2P_CLIENT_MODE) {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			void *vdev;
			unsigned long rc;

			/* wait for event from firmware to set the event */
			rc = wait_for_completion_timeout(
				&pAdapter->sta_authorized_event,
				msecs_to_jiffies(HDD_PEER_AUTHORIZE_WAIT));
			if (!rc)
				hdd_debug("timeout waiting for sta_authorized_event");

			vdev = (void *)cdp_peer_get_vdev(soc, peer);
			cdp_fc_vdev_unpause(soc, (struct cdp_vdev *)vdev,
					OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED);
#endif
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_update_dp_vdev_flags() - update datapath vdev flags
 * @cbk_data: callback data
 * @sta_id: station id
 * @vdev_param: vdev parameter
 * @is_link_up: link state up or down
 *
 * Return: QDF status
 */
QDF_STATUS hdd_update_dp_vdev_flags(void *cbk_data,
				    uint8_t sta_id,
				    uint32_t vdev_param,
				    bool is_link_up)
{
	struct cdp_vdev *data_vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	hdd_context_t *hdd_ctx;
	struct wlan_objmgr_psoc **psoc;

	if (!cbk_data)
		return status;

	psoc = cbk_data;
	hdd_ctx = container_of(psoc, hdd_context_t, hdd_psoc);

	if (!hdd_ctx->tdls_nap_active)
		return status;

	data_vdev = cdp_peer_get_vdev_by_sta_id(soc, sta_id);
	if (NULL == data_vdev) {
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	cdp_txrx_set_vdev_param(soc, data_vdev, vdev_param, is_link_up);

	return status;
}

/**
 * hdd_roam_register_sta() - register station
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @staId: station identifier
 * @pPeerMacAddress: peer MAC address
 * @pBssDesc: pointer to BSS description
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_roam_register_sta(hdd_adapter_t *pAdapter,
					tCsrRoamInfo *pRoamInfo,
					uint8_t staId,
					struct qdf_mac_addr *pPeerMacAddress,
					tSirBssDescription *pBssDesc)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type staDesc = { 0 };
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pBssDesc)
		return QDF_STATUS_E_FAILURE;

	/* Get the Station ID from the one saved during the association */
	staDesc.sta_id = staId;

	/* set the QoS field appropriately */
	if (hdd_wmm_is_active(pAdapter))
		staDesc.is_qos_enabled = 1;
	else
		staDesc.is_qos_enabled = 0;

#ifdef FEATURE_WLAN_WAPI
	hdd_notice("WAPI STA Registered: %d",
		   pAdapter->wapi_info.fIsWapiSta);
	if (pAdapter->wapi_info.fIsWapiSta)
		staDesc.is_wapi_supported = 1;
	else
		staDesc.is_wapi_supported = 0;
#endif /* FEATURE_WLAN_WAPI */

	/* Register the vdev transmit and receive functions */
	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	txrx_ops.rx.rx = hdd_rx_packet_cbk;

	pAdapter->txrx_vdev = (void *)cdp_get_vdev_from_vdev_id(soc,
				(struct cdp_pdev *)pdev,
				pAdapter->sessionId);
	if (!pAdapter->txrx_vdev) {
		hdd_err("%s find vdev fail", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	txrx_ops.tx.tx = NULL;
	cdp_vdev_register(soc,
		(struct cdp_vdev *)pAdapter->txrx_vdev, pAdapter, &txrx_ops);
	if (!txrx_ops.tx.tx) {
		hdd_err("%s vdev register fail", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pAdapter->tx_fn = txrx_ops.tx.tx;
	qdf_status = cdp_peer_register(soc,
			(struct cdp_pdev *)pdev, &staDesc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed Status: %d [0x%08X]",
			 qdf_status, qdf_status);
		return qdf_status;
	}

	if (!pRoamInfo->fAuthRequired) {
		/*
		 * Connections that do not need Upper layer auth, transition
		 * TLSHIM directly to 'Authenticated' state
		 */
		qdf_status =
			hdd_change_peer_state(pAdapter, staDesc.sta_id,
						OL_TXRX_PEER_STATE_AUTH,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);

		hdd_conn_set_authenticated(pAdapter, true);
		hdd_objmgr_set_peer_mlme_auth_state(pAdapter->hdd_vdev, true);
	} else {
		hdd_debug("ULA auth StaId= %d. Changing TL state to CONNECTED at Join time",
			 pHddStaCtx->conn_info.staId[0]);
		qdf_status =
			hdd_change_peer_state(pAdapter, staDesc.sta_id,
						OL_TXRX_PEER_STATE_CONN,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);
		hdd_conn_set_authenticated(pAdapter, false);
		hdd_objmgr_set_peer_mlme_auth_state(pAdapter->hdd_vdev, false);
	}
	return qdf_status;
}

/**
 * hdd_send_re_assoc_event() - send reassoc event
 * @dev: pointer to net device
 * @pAdapter: pointer to adapter
 * @pCsrRoamInfo: pointer to roam info
 * @reqRsnIe: pointer to RSN Information element
 * @reqRsnLength: length of RSN IE
 *
 * Return: none
 */
static void hdd_send_re_assoc_event(struct net_device *dev,
	hdd_adapter_t *pAdapter, tCsrRoamInfo *pCsrRoamInfo,
	uint8_t *reqRsnIe, uint32_t reqRsnLength)
{
	unsigned int len = 0;
	u8 *pFTAssocRsp = NULL;
	uint8_t *rspRsnIe = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	uint8_t *assoc_req_ies = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	uint32_t rspRsnLength = 0;
	struct ieee80211_channel *chan;
	uint8_t buf_ssid_ie[2 + SIR_MAC_SSID_EID_MAX]; /* 2 bytes-EID and len */
	uint8_t *buf_ptr, ssid_ie_len;
	struct cfg80211_bss *bss = NULL;
	uint8_t *final_req_ie = NULL;
	tCsrRoamConnectedProfile roam_profile;
	tHalHandle hal_handle = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	qdf_mem_zero(&roam_profile, sizeof(roam_profile));

	if (pAdapter->defer_disconnect) {
		hdd_debug("Do not send roam event as discon will be processed");
		goto done;
	}

	if (!rspRsnIe) {
		hdd_err("Unable to allocate RSN IE");
		goto done;
	}

	if (!assoc_req_ies) {
		hdd_err("Unable to allocate Assoc Req IE");
		goto done;
	}
	if (pCsrRoamInfo == NULL) {
		hdd_err("Invalid CSR roam info");
		goto done;
	}

	if (pCsrRoamInfo->nAssocRspLength == 0) {
		hdd_err("Assoc rsp length is 0");
		goto done;
	}

	pFTAssocRsp =
		(u8 *) (pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength +
			pCsrRoamInfo->nAssocReqLength);
	if (pFTAssocRsp == NULL)
		goto done;

	/* pFTAssocRsp needs to point to the IEs */
	pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
	hdd_debug("AssocRsp is now at %02x%02x",
		 (unsigned int)pFTAssocRsp[0], (unsigned int)pFTAssocRsp[1]);

	/*
	 * Active session count is decremented upon disconnection, but during
	 * roaming, there is no disconnect indication and hence active session
	 * count is not decremented.
	 * After roaming is completed, active session count is incremented
	 * as a part of connect indication but effectively after roaming the
	 * active session count should still be the same and hence upon
	 * successful reassoc decrement the active session count here.
	 */
	if (!hdd_is_roam_sync_in_progress(pCsrRoamInfo))
		policy_mgr_decr_session_set_pcl(hdd_ctx->hdd_psoc,
				pAdapter->device_mode, pAdapter->sessionId);

	/* Send the Assoc Resp, the supplicant needs this for initial Auth */
	len = pCsrRoamInfo->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
	rspRsnLength = len;
	qdf_mem_copy(rspRsnIe, pFTAssocRsp, len);
	qdf_mem_zero(rspRsnIe + len, IW_GENERIC_IE_MAX - len);

	chan = ieee80211_get_channel(pAdapter->wdev.wiphy,
			(int)pCsrRoamInfo->pBssDesc->channelId);
	sme_roam_get_connect_profile(hal_handle, pAdapter->sessionId,
		&roam_profile);

	bss = hdd_cfg80211_get_bss(pAdapter->wdev.wiphy,
			chan, pCsrRoamInfo->bssid.bytes,
			&roam_profile.SSID.ssId[0],
			roam_profile.SSID.length);

	if (bss == NULL)
		hdd_warn("Get BSS returned NULL");
	buf_ptr = buf_ssid_ie;
	*buf_ptr = SIR_MAC_SSID_EID;
	buf_ptr++;
	*buf_ptr = roam_profile.SSID.length; /*len of ssid*/
	buf_ptr++;
	qdf_mem_copy(buf_ptr, &roam_profile.SSID.ssId[0],
			roam_profile.SSID.length);
	ssid_ie_len = 2 + roam_profile.SSID.length;
	hdd_debug("SSIDIE:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   buf_ssid_ie, ssid_ie_len);
	final_req_ie = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	if (final_req_ie == NULL)
		goto done;
	buf_ptr = final_req_ie;
	qdf_mem_copy(buf_ptr, buf_ssid_ie, ssid_ie_len);
	buf_ptr += ssid_ie_len;
	qdf_mem_copy(buf_ptr, reqRsnIe, reqRsnLength);
	qdf_mem_copy(rspRsnIe, pFTAssocRsp, len);
	qdf_mem_zero(final_req_ie + (ssid_ie_len + reqRsnLength),
		IW_GENERIC_IE_MAX - (ssid_ie_len + reqRsnLength));
	hdd_debug("Req RSN IE:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   final_req_ie, (ssid_ie_len + reqRsnLength));
	cfg80211_roamed_bss(dev, bss,
			final_req_ie, (ssid_ie_len + reqRsnLength),
			rspRsnIe, rspRsnLength, GFP_KERNEL);

	qdf_mem_copy(assoc_req_ies,
		(u8 *)pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength,
		pCsrRoamInfo->nAssocReqLength);

	hdd_debug("ReAssoc Req IE dump");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
		assoc_req_ies, pCsrRoamInfo->nAssocReqLength);

	wlan_hdd_send_roam_auth_event(pAdapter, pCsrRoamInfo->bssid.bytes,
			assoc_req_ies, pCsrRoamInfo->nAssocReqLength,
			rspRsnIe, rspRsnLength,
			pCsrRoamInfo);
done:
	sme_roam_free_connect_profile(&roam_profile);
	if (final_req_ie)
		qdf_mem_free(final_req_ie);
	qdf_mem_free(rspRsnIe);
	qdf_mem_free(assoc_req_ies);
}

/**
 * hdd_is_roam_sync_in_progress()- Check if roam offloaded
 * @roaminfo - Roaming Information
 *
 * Return: roam sync status if roaming offloaded else false
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
bool hdd_is_roam_sync_in_progress(tCsrRoamInfo *roaminfo)
{
	if (roaminfo)
		return roaminfo->roamSynchInProgress;
	else
		return false;
}
#endif

/**
 * hdd_get_ibss_peer_staid() - get sta id for IBSS peer
 * @hddstactx: pointer to HDD sta context
 * @roaminfo: pointer to roaminfo structure
 *
 * This function returns staid for IBSS peer. If peer is broadcast
 * MAC address return self staid(0) else find the peer sta id of
 * the peer.
 *
 * Return: sta_id (HDD_WLAN_INVALID_STA_ID if peer not found).
 */
static uint8_t hdd_get_ibss_peer_staid(hdd_station_ctx_t *hddstactx,
				       tCsrRoamInfo *roaminfo)
{
	uint8_t staid = HDD_WLAN_INVALID_STA_ID;
	QDF_STATUS status;

	if (qdf_is_macaddr_broadcast(&roaminfo->peerMac)) {
		staid = 0;
	} else {
		status = hdd_get_peer_sta_id(hddstactx,
				&roaminfo->peerMac, &staid);
		if (status != QDF_STATUS_SUCCESS) {
			hdd_err("Unable to find staid for " MAC_ADDRESS_STR,
			    MAC_ADDR_ARRAY(roaminfo->peerMac.bytes));
		}
	}

	return staid;
}

/**
 * hdd_change_sta_state_authenticated()-
 * This function changes STA state to authenticated
 * @adapter:  pointer to the adapter structure.
 * @roaminfo: pointer to the RoamInfo structure.
 *
 * This is called from hdd_RoamSetKeyCompleteHandler
 * in context to eCSR_ROAM_SET_KEY_COMPLETE event from fw.
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_change_sta_state_authenticated(hdd_adapter_t *adapter,
						 tCsrRoamInfo *roaminfo)
{
	QDF_STATUS status;
	uint32_t timeout;
	uint8_t staid = HDD_WLAN_INVALID_STA_ID;
	hdd_station_ctx_t *hddstactx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	timeout = hddstactx->hdd_ReassocScenario ?
		AUTO_PS_ENTRY_TIMER_DEFAULT_VALUE :
		hdd_ctx->config->auto_bmps_timer_val * 1000;

	if (QDF_IBSS_MODE == adapter->device_mode)
		staid = hdd_get_ibss_peer_staid(hddstactx, roaminfo);
	else
		staid = hddstactx->conn_info.staId[0];

	hdd_debug("Changing Peer state to AUTHENTICATED for StaId = %d", staid);

	/* Connections that do not need Upper layer authentication,
	 * transition TL to 'Authenticated' state after the keys are set
	 */
	status = hdd_change_peer_state(adapter, staid, OL_TXRX_PEER_STATE_AUTH,
			hdd_is_roam_sync_in_progress(roaminfo));
	hdd_conn_set_authenticated(adapter, true);
	hdd_objmgr_set_peer_mlme_auth_state(adapter->hdd_vdev, true);

	if ((QDF_STA_MODE == adapter->device_mode) ||
		(QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
		sme_ps_enable_auto_ps_timer(
			WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId,
			timeout, false);
	}

	return qdf_status_to_os_return(status);
}

/**
 * hdd_is_key_install_required_for_ibss() - check encryption type to identify
 *                                          if key installation is required
 * @encr_type: encryption type
 *
 * Return: true if key installation is required and false otherwise.
 */
static inline bool hdd_is_key_install_required_for_ibss(
				eCsrEncryptionType encr_type)
{
	if (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == encr_type ||
	    eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == encr_type ||
	    eCSR_ENCRYPT_TYPE_TKIP == encr_type ||
	    eCSR_ENCRYPT_TYPE_AES == encr_type)
		return true;
	else
		return false;
}

/**
 * hdd_change_peer_state_after_set_key() - change the peer state on set key
 *                                         complete
 * @adapter: pointer to HDD adapter
 * @roaminfo: pointer to roam info
 * @roam_result: roam result
 *
 * Peer state will be OL_TXRX_PEER_STATE_CONN until set key is complete.
 * This function checks for the successful set key completion and update
 * the peer state to OL_TXRX_PEER_STATE_AUTH.
 *
 * Return: None
 */
static void hdd_change_peer_state_after_set_key(hdd_adapter_t *adapter,
			tCsrRoamInfo *roaminfo, eCsrRoamResult roam_result)
{
	hdd_station_ctx_t *hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	eCsrEncryptionType encr_type = hdd_sta_ctx->conn_info.ucEncryptionType;

	/*
	 * If the security mode is one of the following, IBSS peer will be
	 * waiting in CONN state and we will move the peer state to AUTH
	 * here. For non-secure connection, no need to wait for set-key complete
	 * peer will be moved to AUTH in hdd_roam_register_sta.
	 */
	if (QDF_IBSS_MODE == adapter->device_mode) {
		if (hdd_is_key_install_required_for_ibss(encr_type))
			hdd_change_sta_state_authenticated(adapter, roaminfo);

		return;
	}

	if (eCSR_ROAM_RESULT_AUTHENTICATED == roam_result) {
		hdd_sta_ctx->conn_info.gtk_installed = true;
		/*
		 * PTK exchange happens in preauthentication itself if key_mgmt
		 * is FT-PSK, ptk_installed was false as there is no set PTK
		 * after roaming. STA TL state moves to authenticated only if
		 * ptk_installed is true. So, make ptk_installed to true in
		 * case of 11R roaming.
		 */
		if (sme_neighbor_roam_is11r_assoc(WLAN_HDD_GET_HAL_CTX(adapter),
						  adapter->sessionId))
			hdd_sta_ctx->conn_info.ptk_installed = true;
	} else {
		hdd_sta_ctx->conn_info.ptk_installed = true;
	}

	/* In WPA case move STA to authenticated when ptk is installed. Earlier
	 * in WEP case STA was moved to AUTHENTICATED prior to setting the
	 * unicast key and it was resulting in sending few un-encrypted packet.
	 * Now in WEP case STA state will be moved to AUTHENTICATED after we
	 * set the unicast and broadcast key.
	 */
	if ((encr_type == eCSR_ENCRYPT_TYPE_WEP40) ||
	    (encr_type == eCSR_ENCRYPT_TYPE_WEP104) ||
	    (encr_type == eCSR_ENCRYPT_TYPE_WEP40_STATICKEY) ||
	    (encr_type == eCSR_ENCRYPT_TYPE_WEP104_STATICKEY)) {
		if (hdd_sta_ctx->conn_info.gtk_installed &&
		    hdd_sta_ctx->conn_info.ptk_installed)
			hdd_change_sta_state_authenticated(adapter, roaminfo);
	} else if (hdd_sta_ctx->conn_info.ptk_installed) {
		hdd_change_sta_state_authenticated(adapter, roaminfo);
	}

	if (hdd_sta_ctx->conn_info.gtk_installed &&
		hdd_sta_ctx->conn_info.ptk_installed) {
		hdd_sta_ctx->conn_info.gtk_installed = false;
		hdd_sta_ctx->conn_info.ptk_installed = false;
	}

	hdd_sta_ctx->roam_info.roamingState = HDD_ROAM_STATE_NONE;

	return;
}

/**
 * hdd_roam_set_key_complete_handler() - Update the security parameters
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS hdd_roam_set_key_complete_handler(hdd_adapter_t *pAdapter,
						    tCsrRoamInfo *pRoamInfo,
						    uint32_t roamId,
						    eRoamCmdStatus roamStatus,
						    eCsrRoamResult roamResult)
{
	eCsrEncryptionType connectedCipherAlgo;
	bool fConnected = false;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	ENTER();

	if (NULL == pRoamInfo) {
		hdd_err("pRoamInfo is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * if (WPA), tell TL to go to 'authenticated' after the keys are set.
	 * then go to 'authenticated'.  For all other authentication types
	 * (those that do not require upper layer authentication) we can put TL
	 * directly into 'authenticated' state.
	 */
	hdd_info("Set Key completion roamStatus =%d roamResult=%d "
		  MAC_ADDRESS_STR, roamStatus, roamResult,
		  MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes));

	fConnected = hdd_conn_get_connected_cipher_algo(pHddStaCtx,
						   &connectedCipherAlgo);
	if (fConnected) {
		hdd_change_peer_state_after_set_key(pAdapter, pRoamInfo,
						    roamResult);
	} else {
		/*
		 * possible disassoc after issuing set key and waiting
		 * set key complete.
		 */
		pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
	}

	EXIT();
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_perform_roam_set_key_complete() - perform set key complete
 * @pAdapter: pointer to adapter
 *
 * Return: none
 */
void hdd_perform_roam_set_key_complete(hdd_adapter_t *pAdapter)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	tCsrRoamInfo roamInfo;

	roamInfo.fAuthRequired = false;
	qdf_mem_copy(roamInfo.bssid.bytes,
		     pHddStaCtx->roam_info.bssid, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(roamInfo.peerMac.bytes,
		     pHddStaCtx->roam_info.peerMac, QDF_MAC_ADDR_SIZE);

	qdf_ret_status =
			hdd_roam_set_key_complete_handler(pAdapter,
					   &roamInfo,
					   pHddStaCtx->roam_info.roamId,
					   pHddStaCtx->roam_info.roamStatus,
					   eCSR_ROAM_RESULT_AUTHENTICATED);
	if (qdf_ret_status != QDF_STATUS_SUCCESS)
		hdd_err("Set Key complete failure");

	pHddStaCtx->roam_info.deferKeyComplete = false;
}

/**
 * hdd_association_completion_handler() - association completion handler
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS hdd_association_completion_handler(hdd_adapter_t *pAdapter,
						     tCsrRoamInfo *pRoamInfo,
						     uint32_t roamId,
						     eRoamCmdStatus roamStatus,
						     eCsrRoamResult roamResult)
{
	struct net_device *dev = pAdapter->dev;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint8_t reqRsnIe[DOT11F_IE_RSN_MAX_LEN];
	uint32_t reqRsnLength = DOT11F_IE_RSN_MAX_LEN, ie_len;
	int ft_carrier_on = false;
	bool hddDisconInProgress = false;
	unsigned long rc;
	tSirResultCodes timeout_reason = 0;
	bool ok;

	if (!pHddCtx) {
		hdd_err("HDD context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* validate config */
	if (!pHddCtx->config) {
		hdd_err("config is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}
	/* HDD has initiated disconnect, do not send connect result indication
	 * to kernel as it will be handled by __cfg80211_disconnect.
	 */
	if (((eConnectionState_Disconnecting ==
	    pHddStaCtx->conn_info.connState) ||
	    (eConnectionState_NotConnected ==
	    pHddStaCtx->conn_info.connState)) &&
	    ((eCSR_ROAM_RESULT_ASSOCIATED == roamResult) ||
	    (eCSR_ROAM_ASSOCIATION_FAILURE == roamStatus))) {
		hdd_info("Disconnect from HDD in progress");
		hddDisconInProgress = true;
	}

	if (eCSR_ROAM_RESULT_ASSOCIATED == roamResult) {
		if (NULL == pRoamInfo) {
			hdd_err("pRoamInfo is NULL");
			return QDF_STATUS_E_FAILURE;
		}
		if (!hddDisconInProgress) {
			hdd_conn_set_connection_state(pAdapter,
						   eConnectionState_Associated);
		}

		/* Save the connection info from CSR... */
		hdd_conn_save_connect_info(pAdapter, pRoamInfo,
					   eCSR_BSS_TYPE_INFRASTRUCTURE);

		if (hdd_add_beacon_filter(pAdapter) != 0)
			hdd_err("hdd_add_beacon_filter() failed");
#ifdef FEATURE_WLAN_WAPI
		if (pRoamInfo->u.pConnectedProfile->AuthType ==
		    eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE
		    || pRoamInfo->u.pConnectedProfile->AuthType ==
		    eCSR_AUTH_TYPE_WAPI_WAI_PSK) {
			pAdapter->wapi_info.fIsWapiSta = 1;
		} else {
			pAdapter->wapi_info.fIsWapiSta = 0;
		}
#endif /* FEATURE_WLAN_WAPI */
		hdd_debug("bss_descr[%d] devicemode[%d]", !!pRoamInfo->pBssDesc,
				pAdapter->device_mode);
		if ((QDF_STA_MODE == pAdapter->device_mode) &&
						pRoamInfo->pBssDesc) {
			ie_len = GET_IE_LEN_IN_BSS(pRoamInfo->pBssDesc->length);
			pHddStaCtx->ap_supports_immediate_power_save =
				wlan_hdd_is_ap_supports_immediate_power_save(
				     (uint8_t *) pRoamInfo->pBssDesc->ieFields,
				     ie_len);
			hdd_debug("ap_supports_immediate_power_save flag [%d]",
				  pHddStaCtx->ap_supports_immediate_power_save);
		}

		/* Indicate 'connect' status to user space */
		hdd_send_association_event(dev, pRoamInfo);

		if (policy_mgr_is_mcc_in_24G(pHddCtx->hdd_psoc)) {
			if (pHddCtx->miracast_value)
				wlan_hdd_set_mas(pAdapter,
					pHddCtx->miracast_value);
		}

		/* Initialize the Linkup event completion variable */
		INIT_COMPLETION(pAdapter->linkup_event_var);

		/*
		 * Sometimes Switching ON the Carrier is taking time to activate
		 * the device properly. Before allowing any packet to go up to
		 * the application, device activation has to be ensured for
		 * proper queue mapping by the kernel. we have registered net
		 * device notifier for device change notification. With this we
		 * will come to know that the device is getting
		 * activated properly.
		 */
		if (pHddStaCtx->ft_carrier_on == false) {
			/*
			 * Enable Linkup Event Servicing which allows the net
			 * device notifier to set the linkup event variable.
			 */
			pAdapter->isLinkUpSvcNeeded = true;

			/* Switch on the Carrier to activate the device */
			wlan_hdd_netif_queue_control(pAdapter,
						WLAN_NETIF_CARRIER_ON,
						WLAN_CONTROL_PATH);

			/*
			 * Wait for the Link to up to ensure all the queues
			 * are set properly by the kernel.
			 */
			rc = wait_for_completion_timeout(
					&pAdapter->linkup_event_var,
					msecs_to_jiffies(ASSOC_LINKUP_TIMEOUT)
					);
			if (!rc)
				hdd_warn("Warning:ASSOC_LINKUP_TIMEOUT");

			/*
			 * Disable Linkup Event Servicing - no more service
			 * required from the net device notifier call.
			 */
			pAdapter->isLinkUpSvcNeeded = false;
		} else {
			pHddStaCtx->ft_carrier_on = false;
			ft_carrier_on = true;
		}
		if ((WLAN_MAX_STA_COUNT + 3) > pRoamInfo->staId)
			pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;
		else
			hdd_err("Wrong Staid: %d", pRoamInfo->staId);

		pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;

		if (hdd_ipa_is_enabled(pHddCtx))
			hdd_ipa_wlan_evt(pAdapter, pRoamInfo->staId,
					 HDD_IPA_STA_CONNECT,
					 pRoamInfo->bssid.bytes);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, false);
#endif

		hdd_debug("check if STA chan ok for DNBS");
		if (policy_mgr_is_chan_ok_for_dnbs(pHddCtx->hdd_psoc,
					pHddStaCtx->conn_info.operationChannel,
					&ok)) {
			hdd_err("Unable to check DNBS eligibility for chan:%d",
					pHddStaCtx->conn_info.operationChannel);
			return QDF_STATUS_E_FAILURE;
		}

		if (!ok) {
			hdd_err("Chan:%d not suitable for DNBS",
				pHddStaCtx->conn_info.operationChannel);
			wlan_hdd_netif_queue_control(pAdapter,
				WLAN_NETIF_CARRIER_OFF,
				WLAN_CONTROL_PATH);
			if (!hddDisconInProgress) {
				hdd_err("Disconnecting...");
				sme_roam_disconnect(
					WLAN_HDD_GET_HAL_CTX(pAdapter),
					pAdapter->sessionId,
					eCSR_DISCONNECT_REASON_UNSPECIFIED);
			}
			return QDF_STATUS_E_FAILURE;
		}

		hdd_debug("check for SAP restart");
		policy_mgr_check_concurrent_intf_and_restart_sap(
			pHddCtx->hdd_psoc);

		DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
			pAdapter->sessionId,
			QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_ASSOC));

		/*
		 * For reassoc, the station is already registered, all we need
		 * is to change the state of the STA in TL.
		 * If authentication is required (WPA/WPA2/DWEP), change TL to
		 * CONNECTED instead of AUTHENTICATED.
		 */
		if (!pRoamInfo->fReassocReq) {
			struct cfg80211_bss *bss;
			u8 *pFTAssocRsp = NULL;
			unsigned int assocRsplen = 0;
			u8 *pFTAssocReq = NULL;
			unsigned int assocReqlen = 0;
			struct ieee80211_channel *chan;
			uint8_t rspRsnIe[DOT11F_IE_RSN_MAX_LEN];
			uint32_t rspRsnLength = DOT11F_IE_RSN_MAX_LEN;

			/* add bss_id to cfg80211 data base */
			bss =
				wlan_hdd_cfg80211_update_bss_db(pAdapter,
								pRoamInfo);
			if (NULL == bss) {
				hdd_err("wlan: Not able to create BSS entry");
				wlan_hdd_netif_queue_control(pAdapter,
					WLAN_NETIF_CARRIER_OFF,
					WLAN_CONTROL_PATH);
				if (!hddDisconInProgress) {
					/*
					 * Here driver was not able to add bss
					 * in cfg80211 database this can happen
					 * if connected channel is not valid,
					 * i.e reg domain was changed during
					 * connection. Queue disconnect for the
					 * session if disconnect is not in
					 * progress.
					 */
					hdd_debug("Disconnecting...");
					sme_roam_disconnect(
					   WLAN_HDD_GET_HAL_CTX(pAdapter),
					   pAdapter->sessionId,
					   eCSR_DISCONNECT_REASON_UNSPECIFIED);
				}
				return QDF_STATUS_E_FAILURE;
			}
			if (pRoamInfo->u.pConnectedProfile->AuthType ==
			    eCSR_AUTH_TYPE_FT_RSN
			    || pRoamInfo->u.pConnectedProfile->AuthType ==
			    eCSR_AUTH_TYPE_FT_RSN_PSK) {

				/* Association Response */
				pFTAssocRsp =
					(u8 *) (pRoamInfo->pbFrames +
						pRoamInfo->nBeaconLength +
						pRoamInfo->nAssocReqLength);
				if (pFTAssocRsp != NULL) {
					/*
					 * pFTAssocRsp needs to point to the IEs
					 */
					pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
					hdd_debug("AssocRsp is now at %02x%02x",
						 (unsigned int)pFTAssocRsp[0],
						 (unsigned int)pFTAssocRsp[1]);
					assocRsplen =
						pRoamInfo->nAssocRspLength -
						FT_ASSOC_RSP_IES_OFFSET;
				} else {
					hdd_debug("AssocRsp is NULL");
					assocRsplen = 0;
				}

				/* Association Request */
				pFTAssocReq = (u8 *) (pRoamInfo->pbFrames +
						      pRoamInfo->nBeaconLength);
				if (pFTAssocReq != NULL) {
					if (!ft_carrier_on) {
						/*
						 * pFTAssocReq needs to point to
						 * the IEs
						 */
						pFTAssocReq +=
							FT_ASSOC_REQ_IES_OFFSET;
						hdd_debug("pFTAssocReq is now at %02x%02x",
							 (unsigned int)
							 pFTAssocReq[0],
							 (unsigned int)
							 pFTAssocReq[1]);
						assocReqlen =
						    pRoamInfo->nAssocReqLength -
							FT_ASSOC_REQ_IES_OFFSET;
					} else {
						/*
						 * This should contain only the
						 * FTIEs
						 */
						assocReqlen =
						    pRoamInfo->nAssocReqLength;
					}
				} else {
					hdd_debug("AssocReq is NULL");
					assocReqlen = 0;
				}

				if (ft_carrier_on) {
					if (!hddDisconInProgress) {
						struct cfg80211_bss *roam_bss;

						/*
						 * After roaming is completed,
						 * active session count is
						 * incremented as a part of
						 * connect indication but
						 * effectively the active
						 * session count should still
						 * be the same and hence upon
						 * successful reassoc
						 * decrement the active session
						 * count here.
						 */
						if (!hdd_is_roam_sync_in_progress
								(pRoamInfo))
						policy_mgr_decr_session_set_pcl(
							pHddCtx->hdd_psoc,
							pAdapter->device_mode,
							pAdapter->sessionId);
						hdd_debug("ft_carrier_on is %d, sending roamed indication",
							 ft_carrier_on);
						chan =
							ieee80211_get_channel
								(pAdapter->wdev.wiphy,
								(int)pRoamInfo->pBssDesc->
								channelId);
						hdd_debug(
							"assocReqlen %d assocRsplen %d",
							assocReqlen,
							assocRsplen);

						hdd_debug(
							"Reassoc Req IE dump");
						QDF_TRACE_HEX_DUMP(
							QDF_MODULE_ID_HDD,
							QDF_TRACE_LEVEL_DEBUG,
							pFTAssocReq,
							assocReqlen);
						if (!pAdapter->defer_disconnect) {
							roam_bss = hdd_cfg80211_get_bss(
								pAdapter->wdev.wiphy,
								chan,
								pRoamInfo->bssid.bytes,
								pRoamInfo->u.
								pConnectedProfile->SSID.ssId,
								pRoamInfo->u.
								pConnectedProfile->SSID.length);
							cfg80211_roamed_bss(dev,
								roam_bss,
								pFTAssocReq,
								assocReqlen,
								pFTAssocRsp,
								assocRsplen,
								GFP_KERNEL);
							wlan_hdd_send_roam_auth_event(
								pAdapter,
								pRoamInfo->bssid.bytes,
								pFTAssocReq,
								assocReqlen,
								pFTAssocRsp,
								assocRsplen,
								pRoamInfo);
						}
					}
					if (sme_get_ftptk_state
						    (WLAN_HDD_GET_HAL_CTX(pAdapter),
						    pAdapter->sessionId)) {
						sme_set_ftptk_state
							(WLAN_HDD_GET_HAL_CTX
								(pAdapter),
							pAdapter->sessionId,
							false);
						pRoamInfo->fAuthRequired =
							false;

						qdf_mem_copy(pHddStaCtx->
							     roam_info.bssid,
							     pRoamInfo->bssid.bytes,
							     QDF_MAC_ADDR_SIZE);
						qdf_mem_copy(pHddStaCtx->
							     roam_info.peerMac,
							     pRoamInfo->peerMac.bytes,
							     QDF_MAC_ADDR_SIZE);
						pHddStaCtx->roam_info.roamId =
							roamId;
						pHddStaCtx->roam_info.
						roamStatus = roamStatus;
						pHddStaCtx->roam_info.
						deferKeyComplete = true;
					}
				} else if (!hddDisconInProgress) {
					hdd_debug("ft_carrier_on is %d, sending connect indication",
						 ft_carrier_on);
					hdd_connect_result(dev,
							   pRoamInfo->
							   bssid.bytes,
							   pRoamInfo,
							   pFTAssocReq,
							   assocReqlen,
							   pFTAssocRsp,
							   assocRsplen,
							   WLAN_STATUS_SUCCESS,
							   GFP_KERNEL,
							   false,
							   pRoamInfo->statusCode);
				}
			} else {
				/*
				 * wpa supplicant expecting WPA/RSN IE in
				 * connect result.
				 */
				csr_roam_get_wpa_rsn_req_ie(WLAN_HDD_GET_HAL_CTX
								    (pAdapter),
							    pAdapter->sessionId,
							    &reqRsnLength,
							    reqRsnIe);

				csr_roam_get_wpa_rsn_rsp_ie(WLAN_HDD_GET_HAL_CTX
								    (pAdapter),
							    pAdapter->sessionId,
							    &rspRsnLength,
							    rspRsnIe);
				if (!hddDisconInProgress) {
					if (ft_carrier_on)
						hdd_send_re_assoc_event(dev,
									pAdapter,
									pRoamInfo,
									reqRsnIe,
									reqRsnLength);
					else {
						hdd_debug("sending connect indication to nl80211:for bssid "
							 MAC_ADDRESS_STR
							 " result:%d and Status:%d",
							 MAC_ADDR_ARRAY
							 (pRoamInfo->bssid.bytes),
							 roamResult, roamStatus);

						/* inform connect result to nl80211 */
						hdd_connect_result(dev,
								   pRoamInfo->
								   bssid.bytes,
								   pRoamInfo,
								   reqRsnIe,
								   reqRsnLength,
								   rspRsnIe,
								   rspRsnLength,
								   WLAN_STATUS_SUCCESS,
								   GFP_KERNEL,
								   false,
								   pRoamInfo->statusCode);
					}
				}
			}
			if (!hddDisconInProgress) {
				cfg80211_put_bss(
					pHddCtx->wiphy,
					bss);

				/*
				 * Perform any WMM-related association
				 * processing.
				 */
				hdd_wmm_assoc(pAdapter, pRoamInfo,
					      eCSR_BSS_TYPE_INFRASTRUCTURE);

				/*
				 * Register the Station with DP after associated
				 */
				qdf_status = hdd_roam_register_sta(pAdapter,
						pRoamInfo,
						pHddStaCtx->conn_info.staId[0],
						NULL, pRoamInfo->pBssDesc);
				hdd_info("Enabling queues");
				wlan_hdd_netif_queue_control(pAdapter,
						WLAN_WAKE_ALL_NETIF_QUEUE,
						WLAN_CONTROL_PATH);

			}
		} else {
			/*
			 * wpa supplicant expecting WPA/RSN IE in connect result
			 * in case of reassociation also need to indicate it to
			 * supplicant.
			 */
			csr_roam_get_wpa_rsn_req_ie(
						WLAN_HDD_GET_HAL_CTX(pAdapter),
						pAdapter->sessionId,
						&reqRsnLength, reqRsnIe);

			hdd_send_re_assoc_event(dev, pAdapter, pRoamInfo,
						reqRsnIe, reqRsnLength);
			/* Reassoc successfully */
			if (pRoamInfo->fAuthRequired) {
				qdf_status =
					hdd_change_peer_state(pAdapter,
						pHddStaCtx->conn_info.staId[0],
						OL_TXRX_PEER_STATE_CONN,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);
				hdd_conn_set_authenticated(pAdapter, false);
				hdd_objmgr_set_peer_mlme_auth_state(
							pAdapter->hdd_vdev,
							false);
			} else {
				hdd_debug("staId: %d Changing TL state to AUTHENTICATED",
					 pHddStaCtx->conn_info.staId[0]);
				qdf_status =
					hdd_change_peer_state(pAdapter,
						pHddStaCtx->conn_info.staId[0],
						OL_TXRX_PEER_STATE_AUTH,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);
				hdd_conn_set_authenticated(pAdapter, true);
				hdd_objmgr_set_peer_mlme_auth_state(
							pAdapter->hdd_vdev,
							true);
			}

			if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
				/*
				 * Perform any WMM-related association
				 * processing
				 */
				hdd_wmm_assoc(pAdapter, pRoamInfo,
					      eCSR_BSS_TYPE_INFRASTRUCTURE);
			}

			/* Start the tx queues */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
			if (pRoamInfo->roamSynchInProgress)
				hdd_debug("LFR3:netif_tx_wake_all_queues");
#endif
			hdd_info("Enabling queues");
			wlan_hdd_netif_queue_control(pAdapter,
						   WLAN_WAKE_ALL_NETIF_QUEUE,
						   WLAN_CONTROL_PATH);
		}

#ifdef FEATURE_WLAN_TDLS
		wlan_hdd_tdls_connection_callback(pAdapter);
#endif

		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("STA register with TL failed status: %d [%08X]",
				qdf_status, qdf_status);
		}
#ifdef WLAN_FEATURE_11W
		qdf_mem_zero(&pAdapter->hdd_stats.hddPmfStats,
			     sizeof(pAdapter->hdd_stats.hddPmfStats));
#endif
	} else {
		bool connect_timeout = false;
		hdd_wext_state_t *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
		if (pRoamInfo)
			hdd_err("wlan: connection failed with " MAC_ADDRESS_STR
				 " result: %d and Status: %d",
				 MAC_ADDR_ARRAY(pRoamInfo->bssid.bytes),
				 roamResult, roamStatus);
		else
			hdd_err("wlan: connection failed with " MAC_ADDRESS_STR
				 " result: %d and Status: %d",
				 MAC_ADDR_ARRAY(pWextState->req_bssId.bytes),
				 roamResult, roamStatus);

		if ((eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE == roamResult) ||
		   (pRoamInfo &&
		   ((eSIR_SME_JOIN_TIMEOUT_RESULT_CODE ==
					pRoamInfo->statusCode) ||
		   (eSIR_SME_AUTH_TIMEOUT_RESULT_CODE ==
					pRoamInfo->statusCode) ||
		   (eSIR_SME_ASSOC_TIMEOUT_RESULT_CODE ==
					pRoamInfo->statusCode)))) {
			wlan_hdd_cfg80211_update_bss_list(pAdapter,
				pRoamInfo ?
				pRoamInfo->bssid.bytes :
				pWextState->req_bssId.bytes);
			sme_remove_bssid_from_scan_list(pHddCtx->hHal,
				pRoamInfo ?
				pRoamInfo->bssid.bytes :
				pWextState->req_bssId.bytes);
			connect_timeout = true;
		}

		/*
		 * CR465478: Only send up a connection failure result when CSR
		 * has completed operation - with a ASSOCIATION_FAILURE status.
		 */
		if (eCSR_ROAM_ASSOCIATION_FAILURE == roamStatus
		    && !hddDisconInProgress) {
			if (pRoamInfo) {
				hdd_err("send connect failure to nl80211: for bssid "
					MAC_ADDRESS_STR
					" result: %d and Status: %d reasoncode: %d",
					MAC_ADDR_ARRAY(pRoamInfo->bssid.bytes),
					roamResult, roamStatus,
					pRoamInfo->reasonCode);
				pHddStaCtx->conn_info.assoc_status_code =
					pRoamInfo->statusCode;
			} else {
				hdd_err("connect failed: for bssid "
				       MAC_ADDRESS_STR
				       " result: %d and status: %d ",
				       MAC_ADDR_ARRAY(pWextState->req_bssId.bytes),
				       roamResult, roamStatus);
			}
			hdd_debug("Invoking packetdump deregistration API");
			wlan_deregister_txrx_packetdump();

			/* inform association failure event to nl80211 */
			if (eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL ==
			    roamResult) {
				if (pRoamInfo)
					hdd_connect_result(dev,
						pRoamInfo->bssid.bytes,
						NULL, NULL, 0, NULL, 0,
						WLAN_STATUS_ASSOC_DENIED_UNSPEC,
						GFP_KERNEL,
						connect_timeout,
						pRoamInfo->statusCode);
				else
					hdd_connect_result(dev,
						pWextState->req_bssId.bytes,
						NULL, NULL, 0, NULL, 0,
						WLAN_STATUS_ASSOC_DENIED_UNSPEC,
						GFP_KERNEL,
						connect_timeout,
						timeout_reason);
			} else {
				if (pRoamInfo)
					hdd_connect_result(dev,
						pRoamInfo->bssid.bytes,
						NULL, NULL, 0, NULL, 0,
						pRoamInfo->reasonCode ?
						pRoamInfo->reasonCode :
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL,
						connect_timeout,
						pRoamInfo->statusCode);
				else
					hdd_connect_result(dev,
						pWextState->req_bssId.bytes,
						NULL, NULL, 0, NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL,
						connect_timeout,
						timeout_reason);
			}
			hdd_clear_roam_profile_ie(pAdapter);
		} else  if ((eCSR_ROAM_CANCELLED == roamStatus
		    && !hddDisconInProgress)) {
				hdd_connect_result(dev,
						pWextState->req_bssId.bytes,
						NULL, NULL, 0, NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL,
						connect_timeout,
						timeout_reason);
		}

		/*
		 * Set connection state to eConnectionState_NotConnected only
		 * when CSR has completed operation - with a
		 * ASSOCIATION_FAILURE or eCSR_ROAM_CANCELLED status.
		 */
		if (((eCSR_ROAM_ASSOCIATION_FAILURE == roamStatus) ||
			(eCSR_ROAM_CANCELLED == roamStatus))
		    && !hddDisconInProgress) {
			hdd_conn_set_connection_state(pAdapter,
					eConnectionState_NotConnected);
		}
		hdd_wmm_init(pAdapter);

		hdd_info("Disabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					   WLAN_CONTROL_PATH);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_roam_ibss_indication_handler() - update the status of the IBSS
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * Here we update the status of the Ibss when we receive information that we
 * have started/joined an ibss session.
 *
 * Return: none
 */
static void hdd_roam_ibss_indication_handler(hdd_adapter_t *pAdapter,
					     tCsrRoamInfo *pRoamInfo,
					     uint32_t roamId,
					     eRoamCmdStatus roamStatus,
					     eCsrRoamResult roamResult)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	hdd_debug("%s: id %d, status %d, result %d",
		 pAdapter->dev->name, roamId,
		 roamStatus, roamResult);

	switch (roamResult) {
	/* both IBSS Started and IBSS Join should come in here. */
	case eCSR_ROAM_RESULT_IBSS_STARTED:
	case eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS:
	case eCSR_ROAM_RESULT_IBSS_COALESCED:
	{
		hdd_context_t *pHddCtx =
			(hdd_context_t *) pAdapter->pHddCtx;
		hdd_station_ctx_t *hdd_sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
		struct qdf_mac_addr broadcastMacAddr =
			QDF_MAC_ADDR_BROADCAST_INITIALIZER;

		if (NULL == pRoamInfo) {
			QDF_ASSERT(0);
			return;
		}

		/* When IBSS Started comes from CSR, we need to move
		 * connection state to IBSS Disconnected (meaning no peers
		 * are in the IBSS).
		 */
		hdd_conn_set_connection_state(pAdapter,
				      eConnectionState_IbssDisconnected);
		/* notify wmm */
		hdd_wmm_connect(pAdapter, pRoamInfo,
				eCSR_BSS_TYPE_IBSS);

		hdd_sta_ctx->broadcast_staid = pRoamInfo->staId;

		pHddCtx->sta_to_adapter[pRoamInfo->staId] =
			pAdapter;
		hdd_roam_register_sta(pAdapter, pRoamInfo,
				      pRoamInfo->staId,
				      &broadcastMacAddr,
				      pRoamInfo->pBssDesc);

		if (pRoamInfo->pBssDesc) {
			struct cfg80211_bss *bss;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
			struct ieee80211_channel *chan;
			int chan_no;
			unsigned int freq;
#endif
			/* we created the IBSS, notify supplicant */
			hdd_info("%s: created ibss " MAC_ADDRESS_STR,
				pAdapter->dev->name,
				MAC_ADDR_ARRAY(
					pRoamInfo->pBssDesc->bssId));

			/* we must first give cfg80211 the BSS information */
			bss = wlan_hdd_cfg80211_update_bss_db(pAdapter,
								pRoamInfo);
			if (NULL == bss) {
				hdd_err("%s: unable to create IBSS entry",
					pAdapter->dev->name);
				return;
			}
			hdd_info("Enabling queues");
			wlan_hdd_netif_queue_control(pAdapter,
					WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
			chan_no = pRoamInfo->pBssDesc->channelId;

			if (chan_no <= 14)
				freq = ieee80211_channel_to_frequency(chan_no,
					  NL80211_BAND_2GHZ);
			else
				freq = ieee80211_channel_to_frequency(chan_no,
					  NL80211_BAND_5GHZ);

			chan = ieee80211_get_channel(pAdapter->wdev.wiphy, freq);

			if (chan)
				cfg80211_ibss_joined(pAdapter->dev,
						     bss->bssid, chan,
						     GFP_KERNEL);
			else
				hdd_warn("%s: chanId: %d, can't find channel",
				pAdapter->dev->name,
				(int)pRoamInfo->pBssDesc->channelId);
#else
			cfg80211_ibss_joined(pAdapter->dev, bss->bssid,
					     GFP_KERNEL);
#endif
			cfg80211_put_bss(
				pHddCtx->wiphy,
				bss);
		}
		if (eCSR_ROAM_RESULT_IBSS_STARTED == roamResult) {
			policy_mgr_incr_active_session(hdd_ctx->hdd_psoc,
				pAdapter->device_mode, pAdapter->sessionId);
		} else if (eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS == roamResult ||
				eCSR_ROAM_RESULT_IBSS_COALESCED == roamResult) {
			policy_mgr_update_connection_info(hdd_ctx->hdd_psoc,
					pAdapter->sessionId);
		}
		break;
	}

	case eCSR_ROAM_RESULT_IBSS_START_FAILED:
	{
		hdd_err("%s: unable to create IBSS", pAdapter->dev->name);
		break;
	}

	default:
		hdd_err("%s: unexpected result %d",
			pAdapter->dev->name, (int)roamResult);
		break;
	}
}

/**
 * hdd_save_peer() - Save peer MAC address in adapter peer table.
 * @sta_ctx: pointer to hdd station context
 * @sta_id: station ID
 * @peer_mac_addr: mac address of new peer
 *
 * This information is passed to iwconfig later. The peer that joined
 * last is passed as information to iwconfig.

 * Return: true if success, false otherwise
 */
bool hdd_save_peer(hdd_station_ctx_t *sta_ctx, uint8_t sta_id,
		   struct qdf_mac_addr *peer_mac_addr)
{
	int idx;

	for (idx = 0; idx < SIR_MAX_NUM_STA_IN_IBSS; idx++) {
		if (0 == sta_ctx->conn_info.staId[idx]) {
			hdd_debug("adding peer: %pM, sta_id: %d, at idx: %d",
				 peer_mac_addr, sta_id, idx);
			sta_ctx->conn_info.staId[idx] = sta_id;
			qdf_copy_macaddr(
				&sta_ctx->conn_info.peerMacAddress[idx],
				peer_mac_addr);
			return true;
		}
	}
	return false;
}

/**
 * hdd_delete_peer() - removes peer from hdd station context peer table
 * @sta_ctx: pointer to hdd station context
 * @sta_id: station ID
 *
 * Return: None
 */
void hdd_delete_peer(hdd_station_ctx_t *sta_ctx, uint8_t sta_id)
{
	int i;

	for (i = 0; i < SIR_MAX_NUM_STA_IN_IBSS; i++) {
		if (sta_id == sta_ctx->conn_info.staId[i]) {
			sta_ctx->conn_info.staId[i] = 0;
			return;
		}
	}
}

/**
 * roam_remove_ibss_station() - Remove the IBSS peer MAC address in the adapter
 * @pAdapter: pointer to adapter
 * @staId: station id
 *
 * Return:
 *	true if we remove MAX_PEERS or less STA
 *	false otherwise.
 */
static bool roam_remove_ibss_station(hdd_adapter_t *pAdapter, uint8_t staId)
{
	bool fSuccess = false;
	int idx = 0;
	uint8_t valid_idx = 0;
	uint8_t del_idx = 0;
	uint8_t empty_slots = 0;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	for (idx = 0; idx < MAX_PEERS; idx++) {
		if (staId == pHddStaCtx->conn_info.staId[idx]) {
			pHddStaCtx->conn_info.staId[idx] = 0;

			qdf_zero_macaddr(&pHddStaCtx->conn_info.
					 peerMacAddress[idx]);

			fSuccess = true;

			/*
			 * Note the deleted Index, if its 0 we need special
			 * handling.
			 */
			del_idx = idx;

			empty_slots++;
		} else {
			if (pHddStaCtx->conn_info.staId[idx] !=
					HDD_WLAN_INVALID_STA_ID) {
				valid_idx = idx;
			} else {
				/* Found an empty slot */
				empty_slots++;
			}
		}
	}

	if (MAX_PEERS == empty_slots) {
		/* Last peer departed, set the IBSS state appropriately */
		pHddStaCtx->conn_info.connState =
			eConnectionState_IbssDisconnected;
		hdd_debug("Last IBSS Peer Departed!!!");
	}
	/* Find next active staId, to have a valid sta trigger for TL. */
	if (fSuccess == true) {
		if (del_idx == 0) {
			if (pHddStaCtx->conn_info.staId[valid_idx] !=
					HDD_WLAN_INVALID_STA_ID) {
				pHddStaCtx->conn_info.staId[0] =
					pHddStaCtx->conn_info.staId[valid_idx];
				qdf_copy_macaddr(&pHddStaCtx->conn_info.
						 peerMacAddress[0],
						 &pHddStaCtx->conn_info.
						 peerMacAddress[valid_idx]);

				pHddStaCtx->conn_info.staId[valid_idx] = 0;
				qdf_zero_macaddr(&pHddStaCtx->conn_info.
						 peerMacAddress[valid_idx]);
			}
		}
	}
	return fSuccess;
}

/**
 * roam_ibss_connect_handler() - IBSS connection handler
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 *
 * We update the status of the IBSS to connected in this function.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS roam_ibss_connect_handler(hdd_adapter_t *pAdapter,
					    tCsrRoamInfo *pRoamInfo)
{
	struct cfg80211_bss *bss;
	/*
	 * Set the internal connection state to show 'IBSS Connected' (IBSS with
	 * a partner stations).
	 */
	hdd_conn_set_connection_state(pAdapter, eConnectionState_IbssConnected);

	/* Save the connection info from CSR... */
	hdd_conn_save_connect_info(pAdapter, pRoamInfo, eCSR_BSS_TYPE_IBSS);

	/* Send the bssid address to the wext. */
	hdd_send_association_event(pAdapter->dev, pRoamInfo);
	/* add bss_id to cfg80211 data base */
	bss = wlan_hdd_cfg80211_update_bss_db(pAdapter, pRoamInfo);
	if (NULL == bss) {
		hdd_err("%s: unable to create IBSS entry",
		       pAdapter->dev->name);
		return QDF_STATUS_E_FAILURE;
	}
	cfg80211_put_bss(
		WLAN_HDD_GET_CTX(pAdapter)->wiphy,
		bss);

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_roam_mic_error_indication_handler() - MIC error indication handler
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * This function indicates the Mic failure to the supplicant
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
hdd_roam_mic_error_indication_handler(hdd_adapter_t *pAdapter,
				      tCsrRoamInfo *pRoamInfo,
				      uint32_t roamId,
				      eRoamCmdStatus roamStatus,
				      eCsrRoamResult roamResult)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	if (eConnectionState_Associated == pHddStaCtx->conn_info.connState &&
	    TKIP_COUNTER_MEASURE_STOPED ==
	    pHddStaCtx->WextState.mTKIPCounterMeasures) {
		struct iw_michaelmicfailure msg;
		union iwreq_data wreq;

		memset(&msg, '\0', sizeof(msg));
		msg.src_addr.sa_family = ARPHRD_ETHER;
		memcpy(msg.src_addr.sa_data,
		       pRoamInfo->u.pMICFailureInfo->taMacAddr,
		       sizeof(pRoamInfo->u.pMICFailureInfo->taMacAddr));
		hdd_debug("MIC MAC " MAC_ADDRESS_STR,
			 MAC_ADDR_ARRAY(msg.src_addr.sa_data));

		if (pRoamInfo->u.pMICFailureInfo->multicast == eSIR_TRUE)
			msg.flags = IW_MICFAILURE_GROUP;
		else
			msg.flags = IW_MICFAILURE_PAIRWISE;
		memset(&wreq, 0, sizeof(wreq));
		wreq.data.length = sizeof(msg);
		wireless_send_event(pAdapter->dev, IWEVMICHAELMICFAILURE, &wreq,
				    (char *)&msg);
		/* inform mic failure to nl80211 */
		cfg80211_michael_mic_failure(pAdapter->dev,
					     pRoamInfo->u.pMICFailureInfo->
					     taMacAddr,
					     ((pRoamInfo->u.pMICFailureInfo->
					       multicast ==
					       eSIR_TRUE) ?
					      NL80211_KEYTYPE_GROUP :
					      NL80211_KEYTYPE_PAIRWISE),
					     pRoamInfo->u.pMICFailureInfo->
					     keyId,
					     pRoamInfo->u.pMICFailureInfo->TSC,
					     GFP_KERNEL);

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * roam_roam_connect_status_update_handler() - IBSS connect status update
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * The Ibss connection status is updated regularly here in this function.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
roam_roam_connect_status_update_handler(hdd_adapter_t *pAdapter,
					tCsrRoamInfo *pRoamInfo,
					uint32_t roamId,
					eRoamCmdStatus roamStatus,
					eCsrRoamResult roamResult)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	QDF_STATUS qdf_status;

	switch (roamResult) {
	case eCSR_ROAM_RESULT_IBSS_NEW_PEER:
	{
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
		struct station_info *stainfo;
		eCsrEncryptionType encr_type = pHddStaCtx->ibss_enc_key.encType;

		hdd_info("IBSS New Peer indication from SME "
			 "with peerMac " MAC_ADDRESS_STR " BSSID: "
			 MAC_ADDRESS_STR " and stationID= %d",
			 MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes),
			 MAC_ADDR_ARRAY(pHddStaCtx->conn_info.bssId.bytes),
			 pRoamInfo->staId);

		if (!hdd_save_peer
			    (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter),
			    pRoamInfo->staId,
			    &pRoamInfo->peerMac)) {
			hdd_warn("Max reached: Can't register new IBSS peer");
			break;
		}

		pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;

		if (hdd_is_key_install_required_for_ibss(encr_type))
			pRoamInfo->fAuthRequired = true;

		/* Register the Station with TL for the new peer. */
		qdf_status = hdd_roam_register_sta(pAdapter,
						   pRoamInfo,
						   pRoamInfo->staId,
						   &pRoamInfo->peerMac,
						   pRoamInfo->pBssDesc);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("Cannot register STA with TL for IBSS. qdf_status: %d [%08X]",
				qdf_status, qdf_status);
		}
		pHddStaCtx->ibss_sta_generation++;
		stainfo = qdf_mem_malloc(sizeof(*stainfo));
		if (stainfo == NULL) {
			hdd_err("memory allocation for station_info failed");
			return QDF_STATUS_E_NOMEM;
		}
		stainfo->filled = 0;
		stainfo->generation = pHddStaCtx->ibss_sta_generation;

		cfg80211_new_sta(pAdapter->dev,
				 (const u8 *)pRoamInfo->peerMac.bytes,
				 stainfo, GFP_KERNEL);
		qdf_mem_free(stainfo);

		if (hdd_is_key_install_required_for_ibss(encr_type)) {
			pHddStaCtx->ibss_enc_key.keyDirection =
				eSIR_TX_RX;
			qdf_copy_macaddr(&pHddStaCtx->ibss_enc_key.peerMac,
					 &pRoamInfo->peerMac);

			hdd_info("New peer joined set PTK encType=%d",
				 encr_type);

			qdf_status =
				sme_roam_set_key(WLAN_HDD_GET_HAL_CTX
							 (pAdapter),
						 pAdapter->sessionId,
						 &pHddStaCtx->ibss_enc_key,
						 &roamId);

			if (QDF_STATUS_SUCCESS != qdf_status) {
				hdd_err("sme_roam_set_key failed, status: %d",
					qdf_status);
				return QDF_STATUS_E_FAILURE;
			}
		}
		hdd_info("Enabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					   WLAN_CONTROL_PATH);
		break;
	}

	case eCSR_ROAM_RESULT_IBSS_CONNECT:
	{

		roam_ibss_connect_handler(pAdapter, pRoamInfo);

		break;
	}
	case eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED:
	{
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

		if (!roam_remove_ibss_station(pAdapter, pRoamInfo->staId))
			hdd_warn("IBSS peer departed by cannot find peer in our registration table with TL");

		hdd_info("IBSS Peer Departed from SME "
			 "with peerMac " MAC_ADDRESS_STR " BSSID: "
			 MAC_ADDRESS_STR " and stationID= %d",
			 MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes),
			 MAC_ADDR_ARRAY(pHddStaCtx->conn_info.bssId.bytes),
			 pRoamInfo->staId);

		hdd_roam_deregister_sta(pAdapter, pRoamInfo->staId);

		pHddCtx->sta_to_adapter[pRoamInfo->staId] = NULL;
		pHddStaCtx->ibss_sta_generation++;

		cfg80211_del_sta(pAdapter->dev,
				 (const u8 *)&pRoamInfo->peerMac.bytes,
				 GFP_KERNEL);
		break;
	}
	case eCSR_ROAM_RESULT_IBSS_INACTIVE:
	{
		hdd_debug("Received eCSR_ROAM_RESULT_IBSS_INACTIVE from SME");
		/* Stop only when we are inactive */
		hdd_info("Disabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					   WLAN_CONTROL_PATH);
		hdd_conn_set_connection_state(pAdapter,
					      eConnectionState_NotConnected);

		/* Send the bssid address to the wext. */
		hdd_send_association_event(pAdapter->dev, pRoamInfo);
		break;
	}
	default:
		break;

	}

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_TDLS
/**
 * hdd_roam_register_tdlssta() - register new TDLS station
 * @pAdapter: pointer to adapter
 * @peerMac: pointer to peer MAC address
 * @staId: station identifier
 * @ucastSig: unicast signature
 * @qos: QOS capability of TDLS station/link
 *
 * Construct the staDesc and register with TL the new STA.
 * This is called as part of ADD_STA in the TDLS setup.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_roam_register_tdlssta(hdd_adapter_t *pAdapter,
				     const uint8_t *peerMac, uint16_t staId,
				     uint8_t ucastSig, uint8_t qos)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type staDesc = { 0 };
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	/*
	 * TDLS sta in BSS should be set as STA type TDLS and STA MAC should
	 * be peer MAC, here we are working on direct Link
	 */
	staDesc.sta_id = staId;

	/* set the QoS field appropriately .. */
	staDesc.is_qos_enabled = qos;

	/* Register the vdev transmit and receive functions */
	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	txrx_ops.rx.rx = hdd_rx_packet_cbk;
	cdp_vdev_register(soc,
		 (struct cdp_vdev *)cdp_get_vdev_from_vdev_id(soc,
		 (struct cdp_pdev *)pdev, pAdapter->sessionId),
		 pAdapter, &txrx_ops);
	pAdapter->tx_fn = txrx_ops.tx.tx;

	/* Register the Station with TL...  */
	qdf_status = cdp_peer_register(soc,
			(struct cdp_pdev *)pdev, &staDesc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed Status: %d [0x%08X]",
			qdf_status, qdf_status);
		return qdf_status;
	}

	return qdf_status;
}

/**
 * hdd_roam_deregister_tdlssta() - deregister new TDLS station
 * @pAdapter: pointer to adapter
 * @staId: station identifier
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_roam_deregister_tdlssta(hdd_adapter_t *pAdapter, uint8_t staId)
{
	QDF_STATUS qdf_status;
	qdf_status = cdp_clear_peer(cds_get_context(QDF_MODULE_ID_SOC),
			(struct cdp_pdev *)cds_get_context(QDF_MODULE_ID_TXRX),
			staId);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_clear_peer() failed for staID: %d Status: %d [0x%08X]",
			staId, qdf_status, qdf_status);
	}
	return qdf_status;
}

/**
 * hdd_tdls_connection_tracker_update() - update connection tracker state
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @hdd_tdls_ctx: tdls context
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS hdd_tdls_connection_tracker_update(hdd_adapter_t *adapter,
						     tCsrRoamInfo *roam_info,
						     tdlsCtx_t *hdd_tdls_ctx)
{
	hddTdlsPeer_t *curr_peer;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	curr_peer = wlan_hdd_tdls_find_peer(adapter,
					    roam_info->peerMac.bytes);

	if (!curr_peer) {
		hdd_err("curr_peer is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (eTDLS_LINK_CONNECTED ==
	    curr_peer->link_status) {
		hdd_debug("Received CONNECTION_TRACKER_NOTIFICATION "
			MAC_ADDRESS_STR
			" staId: %d, reason: %d",
			MAC_ADDR_ARRAY(roam_info->peerMac.bytes),
			roam_info->staId,
			roam_info->reasonCode);

		if (roam_info->reasonCode ==
		    eWNI_TDLS_PEER_ENTER_BUF_STA ||
		    roam_info->reasonCode ==
		    eWNI_TDLS_ENTER_BT_BUSY_MODE ||
		    roam_info->reasonCode ==
		    eWMI_TDLS_SCAN_STARTED_EVENT)
			hdd_ctx->enable_tdls_connection_tracker = false;
		else if (roam_info->reasonCode ==
			  eWNI_TDLS_PEER_EXIT_BUF_STA ||
			  roam_info->reasonCode ==
			  eWNI_TDLS_EXIT_BT_BUSY_MODE ||
			  roam_info->reasonCode ==
			  eWMI_TDLS_SCAN_COMPLETED_EVENT)
			hdd_ctx->enable_tdls_connection_tracker = true;
		hdd_debug("hdd_ctx->enable_tdls_connection_tracker %d",
			hdd_ctx->enable_tdls_connection_tracker);
	} else {
		hdd_debug("TDLS not connected, ignore notification, reason: %d",
			roam_info->reasonCode);
	}

	return QDF_STATUS_SUCCESS;
}




/**
 * hdd_roam_tdls_status_update_handler() - TDLS status update handler
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * HDD interface between SME and TL to ensure TDLS client registration with
 * TL in case of new TDLS client is added and deregistration at the time
 * TDLS client is deleted.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
hdd_roam_tdls_status_update_handler(hdd_adapter_t *pAdapter,
				    tCsrRoamInfo *pRoamInfo,
				    uint32_t roamId,
				    eRoamCmdStatus roamStatus,
				    eCsrRoamResult roamResult)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	tdlsCtx_t *pHddTdlsCtx;
	tSmeTdlsPeerStateParams smeTdlsPeerStateParams;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint8_t staIdx;
	hddTdlsPeer_t *curr_peer;
	uint32_t reason;

	hdd_debug("hdd_tdlsStatusUpdate: %s staIdx %d " MAC_ADDRESS_STR,
		  roamResult ==
		  eCSR_ROAM_RESULT_ADD_TDLS_PEER ? "ADD_TDLS_PEER" : roamResult
		  ==
		  eCSR_ROAM_RESULT_DELETE_TDLS_PEER ? "DEL_TDLS_PEER" :
		  roamResult ==
		  eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND ? "DEL_TDLS_PEER_IND"
		  : roamResult ==
		  eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND ?
		  "DEL_ALL_TDLS_PEER_IND" : roamResult ==
		  eCSR_ROAM_RESULT_UPDATE_TDLS_PEER ? "UPDATE_TDLS_PEER" :
		  roamResult ==
		  eCSR_ROAM_RESULT_LINK_ESTABLISH_REQ_RSP ?
		  "LINK_ESTABLISH_REQ_RSP" : roamResult ==
		  eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER ? "TDLS_SHOULD_DISCOVER"
		  : roamResult ==
		  eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN ? "TDLS_SHOULD_TEARDOWN"
		  : roamResult ==
		  eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED ?
		  "TDLS_SHOULD_PEER_DISCONNECTED" : "UNKNOWN", pRoamInfo->staId,
		  MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes));

	switch (roamResult) {
	case eCSR_ROAM_RESULT_ADD_TDLS_PEER:
	{
		if (eSIR_SME_SUCCESS != pRoamInfo->statusCode) {
			hdd_err("Add Sta failed. status code: %d",
				pRoamInfo->statusCode);
			pAdapter->tdlsAddStaStatus = QDF_STATUS_E_FAILURE;
		} else {
			/*
			 * Check if there is available index for this new TDLS
			 * STA.
			 */
			for (staIdx = 0;
			     staIdx < pHddCtx->max_num_tdls_sta;
			     staIdx++) {
				if (0 ==
				    pHddCtx->tdlsConnInfo[staIdx].
				    staId) {
					pHddCtx->tdlsConnInfo[staIdx].
					sessionId =
						pRoamInfo->sessionId;
					pHddCtx->tdlsConnInfo[staIdx].
					staId = pRoamInfo->staId;

					hdd_debug("TDLS: STA IDX at %d is %d "
						   "of mac "
						   MAC_ADDRESS_STR,
						  staIdx,
						  pHddCtx->
						  tdlsConnInfo[staIdx].
						  staId,
						  MAC_ADDR_ARRAY
							  (pRoamInfo->peerMac.bytes));

					qdf_copy_macaddr(&pHddCtx->
							 tdlsConnInfo
							 [staIdx].
							 peerMac,
							 &pRoamInfo->
							 peerMac);
					status = QDF_STATUS_SUCCESS;
					break;
				}
			}
			if (staIdx < pHddCtx->max_num_tdls_sta) {
				if (-1 ==
				    wlan_hdd_tdls_set_sta_id(pAdapter,
							     pRoamInfo->
							     peerMac.bytes,
							     pRoamInfo->
							     staId)) {
					hdd_err("wlan_hdd_tdls_set_sta_id() failed");
					return QDF_STATUS_E_FAILURE;
				}

				(WLAN_HDD_GET_CTX(pAdapter))->
				sta_to_adapter[pRoamInfo->staId] =
					pAdapter;
				/*
				 * store the ucast signature,
				 * if required for further reference.
				 */

				wlan_hdd_tdls_set_signature(pAdapter,
							    pRoamInfo->
							    peerMac.bytes,
							    pRoamInfo->
							    ucastSig);
			} else {
				status = QDF_STATUS_E_FAILURE;
				hdd_debug("no available slot in conn_info. staId: %d cannot be stored",
					pRoamInfo->staId);
			}
			pAdapter->tdlsAddStaStatus = status;
		}
		complete(&pAdapter->tdls_add_station_comp);
		break;
	}
	case eCSR_ROAM_RESULT_UPDATE_TDLS_PEER:
	{
		if (eSIR_SME_SUCCESS != pRoamInfo->statusCode) {
			hdd_err("Add Sta failed. status code: %d",
				pRoamInfo->statusCode);
		}
		/* store the ucast signature which will be used later when
		 * registering to TL
		 */
		pAdapter->tdlsAddStaStatus = pRoamInfo->statusCode;
		complete(&pAdapter->tdls_add_station_comp);
		break;
	}
	case eCSR_ROAM_RESULT_LINK_ESTABLISH_REQ_RSP:
	{
		if (eSIR_SME_SUCCESS != pRoamInfo->statusCode) {
			hdd_err("Link Establish Request failed. status: %d",
				pRoamInfo->statusCode);
		}
		complete(&pAdapter->tdls_link_establish_req_comp);
		break;
	}
	case eCSR_ROAM_RESULT_DELETE_TDLS_PEER:
	{
		for (staIdx = 0; staIdx < pHddCtx->max_num_tdls_sta;
		     staIdx++) {
			if ((pHddCtx->tdlsConnInfo[staIdx].sessionId ==
			     pRoamInfo->sessionId)
			    && pRoamInfo->staId ==
			    pHddCtx->tdlsConnInfo[staIdx].staId) {
				hdd_debug("HDD: del STA IDX = %x",
					 pRoamInfo->staId);
				mutex_lock(&pHddCtx->tdls_lock);
				curr_peer =
					wlan_hdd_tdls_find_peer(pAdapter,
								pRoamInfo->
								peerMac.bytes);
				if (NULL != curr_peer) {
				    hdd_debug("Current status for peer " MAC_ADDRESS_STR " is %d",
				    MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes),
					    curr_peer->link_status);
				    if (TDLS_IS_CONNECTED(curr_peer)) {
					mutex_unlock(&pHddCtx->tdls_lock);
					hdd_roam_deregister_tdlssta
						(pAdapter,
						pRoamInfo->staId);
					wlan_hdd_tdls_decrement_peer_count
						(pAdapter);
				    } else if (eTDLS_LINK_CONNECTING ==
					    curr_peer->link_status) {
					mutex_unlock(&pHddCtx->tdls_lock);
					hdd_roam_deregister_tdlssta
						(pAdapter,
						pRoamInfo->staId);
				    } else
					mutex_unlock(&pHddCtx->tdls_lock);
				} else
				    mutex_unlock(&pHddCtx->tdls_lock);

				mutex_lock(&pHddCtx->tdls_lock);
				wlan_hdd_tdls_reset_peer(pAdapter,
							 pRoamInfo->
							 peerMac.bytes);
				mutex_unlock(&pHddCtx->tdls_lock);

				pHddCtx->tdlsConnInfo[staIdx].staId = 0;
				pHddCtx->tdlsConnInfo[staIdx].
				sessionId = 255;
				qdf_mem_zero(&pHddCtx->
					     tdlsConnInfo[staIdx].
					     peerMac,
					     QDF_MAC_ADDR_SIZE);
				status = QDF_STATUS_SUCCESS;
				break;
			}
		}
		complete(&pAdapter->tdls_del_station_comp);
	}
	break;
	case eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND:
	{
		hdd_debug("Sending teardown to supplicant with reason code %u",
			pRoamInfo->reasonCode);

		mutex_lock(&pHddCtx->tdls_lock);
		curr_peer =
			wlan_hdd_tdls_find_peer(pAdapter,
						pRoamInfo->peerMac.bytes);

		if (!curr_peer) {
			mutex_unlock(&pHddCtx->tdls_lock);
			hdd_debug("peer doesn't exists");
			status = QDF_STATUS_SUCCESS;
			break;
		}

		wlan_hdd_tdls_indicate_teardown(pAdapter, curr_peer,
						pRoamInfo->reasonCode);
		hdd_send_wlan_tdls_teardown_event(eTDLS_TEARDOWN_BSS_DISCONNECT,
						curr_peer->peerMac);
		mutex_unlock(&pHddCtx->tdls_lock);
		status = QDF_STATUS_SUCCESS;
		break;
	}
	case eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND:
	{
		/* 0 staIdx is assigned to AP we dont want to touch that */
		for (staIdx = 0; staIdx < pHddCtx->max_num_tdls_sta;
		     staIdx++) {
			if ((pHddCtx->tdlsConnInfo[staIdx].sessionId ==
			     pRoamInfo->sessionId)
			    && pHddCtx->tdlsConnInfo[staIdx].staId) {
				hdd_debug("hdd_tdlsStatusUpdate: staIdx %d "
					 MAC_ADDRESS_STR,
					pHddCtx->tdlsConnInfo[staIdx].
					staId,
					MAC_ADDR_ARRAY(pHddCtx->
						       tdlsConnInfo
						       [staIdx].
						       peerMac.
						       bytes));
				mutex_lock(&pHddCtx->tdls_lock);
				wlan_hdd_tdls_reset_peer(pAdapter,
							 pHddCtx->
							 tdlsConnInfo
							 [staIdx].
							 peerMac.bytes);
				mutex_unlock(&pHddCtx->tdls_lock);
				hdd_roam_deregister_tdlssta(pAdapter,
							    pHddCtx->
							    tdlsConnInfo
							    [staIdx].
							    staId);
				qdf_mem_zero(&smeTdlsPeerStateParams,
					     sizeof
					     (smeTdlsPeerStateParams));
				smeTdlsPeerStateParams.vdevId =
					pHddCtx->tdlsConnInfo[staIdx].
					sessionId;
				qdf_mem_copy(&smeTdlsPeerStateParams.
					     peerMacAddr,
					     &pHddCtx->
					     tdlsConnInfo[staIdx].
					     peerMac.bytes,
					     QDF_MAC_ADDR_SIZE);
				smeTdlsPeerStateParams.peerState =
					eSME_TDLS_PEER_STATE_TEARDOWN;

				hdd_debug("calling sme_update_tdls_peer_state for staIdx %d "
					 MAC_ADDRESS_STR,
					 pHddCtx->tdlsConnInfo[staIdx].
					 staId,
					 MAC_ADDR_ARRAY(pHddCtx->
							 tdlsConnInfo
							 [staIdx].
							 peerMac.
							 bytes));
				status =
					sme_update_tdls_peer_state(
						pHddCtx->hHal,
						&smeTdlsPeerStateParams);
				if (QDF_STATUS_SUCCESS != status) {
					hdd_err("sme_update_tdls_peer_state failed for "
						MAC_ADDRESS_STR,
						MAC_ADDR_ARRAY
						(pHddCtx->
						 tdlsConnInfo[staIdx].
						 peerMac.bytes));
				}
				wlan_hdd_tdls_decrement_peer_count
					(pAdapter);

				qdf_mem_zero(&pHddCtx->
					     tdlsConnInfo[staIdx].
					     peerMac,
					     QDF_MAC_ADDR_SIZE);
				pHddCtx->tdlsConnInfo[staIdx].staId = 0;
				pHddCtx->tdlsConnInfo[staIdx].
				sessionId = 255;

				status = QDF_STATUS_SUCCESS;
			}
		}
		break;
	}
	case eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER:
	{
		/* ignore TDLS_SHOULD_DISCOVER if any concurrency detected */
		if (!cds_check_is_tdls_allowed(pAdapter->device_mode)) {
			hdd_err("TDLS not allowed, ignore SHOULD_DISCOVER");
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		if (pHddCtx->tdls_nss_switch_in_progress) {
			hdd_debug("TDLS antenna switch is in progress, ignore SHOULD_DISCOVER");
			status = QDF_STATUS_SUCCESS;
			break;
		}

		mutex_lock(&pHddCtx->tdls_lock);
		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (!pHddTdlsCtx) {
			mutex_unlock(&pHddCtx->tdls_lock);
			hdd_debug("TDLS ctx is null, ignore roamResult (%d)",
				 roamResult);
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		curr_peer =
			wlan_hdd_tdls_get_peer(pAdapter,
					       pRoamInfo->peerMac.bytes);
		if (!curr_peer) {
			hdd_debug("curr_peer is null");
			status = QDF_STATUS_E_FAILURE;
		} else {
			if (eTDLS_LINK_CONNECTED ==
			    curr_peer->link_status) {
				hdd_debug("TDLS link status is connected, ignore SHOULD_DISCOVER");
			} else {
				/*
				 * If external control is enabled then initiate
				 * TDLS only if forced peer is set otherwise
				 * ignore should Discover trigger from fw.
				 */
				if (pHddCtx->config->
				    fTDLSExternalControl
				    && (false ==
					curr_peer->isForcedPeer)) {
					hdd_debug("TDLS ExternalControl enabled but curr_peer is not forced, ignore SHOULD_DISCOVER");
					status = QDF_STATUS_SUCCESS;
					break;
				}
				hdd_debug("initiate TDLS setup on SHOULD_DISCOVER, fTDLSExternalControl: %d, curr_peer->isForcedPeer: %d, reason: %d",
					 pHddCtx->config->
					 fTDLSExternalControl,
					 curr_peer->isForcedPeer,
					 pRoamInfo->reasonCode);
				pHddTdlsCtx->curr_candidate = curr_peer;
				wlan_hdd_tdls_implicit_send_discovery_request(
								pHddTdlsCtx);
			}
			status = QDF_STATUS_SUCCESS;
		}
		mutex_unlock(&pHddCtx->tdls_lock);
		break;
	}

	case eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN:
	{
		mutex_lock(&pHddCtx->tdls_lock);
		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (!pHddTdlsCtx) {
			mutex_unlock(&pHddCtx->tdls_lock);
			hdd_debug("TDLS ctx is null, ignore roamResult (%d)",
				 roamResult);
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		curr_peer =
			wlan_hdd_tdls_find_peer(pAdapter,
						pRoamInfo->peerMac.bytes);
		if (!curr_peer) {
			hdd_debug("curr_peer is null");
			status = QDF_STATUS_E_FAILURE;
		} else {
			if (eTDLS_LINK_CONNECTED ==
			    curr_peer->link_status) {
				hdd_debug("Received SHOULD_TEARDOWN for peer "
					MAC_ADDRESS_STR
					" staId: %d, reason: %d",
					MAC_ADDR_ARRAY(pRoamInfo->
						       peerMac.bytes),
					pRoamInfo->staId,
					pRoamInfo->reasonCode);

				if (pRoamInfo->reasonCode ==
				    eWNI_TDLS_TEARDOWN_REASON_RSSI ||
				    pRoamInfo->reasonCode ==
				    eWNI_TDLS_DISCONNECTED_REASON_PEER_DELETE ||
				    pRoamInfo->reasonCode ==
				    eWNI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT ||
				    pRoamInfo->reasonCode ==
				    eWNI_TDLS_TEARDOWN_REASON_NO_RESPONSE) {
					reason =
						eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE;
				} else
					reason =
						eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON;

				wlan_hdd_tdls_indicate_teardown
					(pHddTdlsCtx->pAdapter, curr_peer,
					reason);
				hdd_send_wlan_tdls_teardown_event(
					eTDLS_TEARDOWN_BSS_DISCONNECT,
					curr_peer->peerMac);
			} else {
				hdd_debug("TDLS link is not connected, ignore SHOULD_TEARDOWN, reason: %d",
					pRoamInfo->reasonCode);
			}
			status = QDF_STATUS_SUCCESS;
		}
		mutex_unlock(&pHddCtx->tdls_lock);
		break;
	}

	case eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED:
	{
		mutex_lock(&pHddCtx->tdls_lock);
		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (!pHddTdlsCtx) {
			mutex_unlock(&pHddCtx->tdls_lock);
			hdd_debug("TDLS ctx is null, ignore roamResult (%d)",
				 roamResult);
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		curr_peer =
			wlan_hdd_tdls_find_peer(pAdapter,
						pRoamInfo->peerMac.bytes);
		if (!curr_peer) {
			hdd_debug("curr_peer is null");
			status = QDF_STATUS_E_FAILURE;
		} else {
			if (eTDLS_LINK_CONNECTED ==
			    curr_peer->link_status) {
				hdd_debug("Received SHOULD_PEER_DISCONNECTED for peer "
					MAC_ADDRESS_STR
					" staId: %d reason: %d",
					MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes),
					pRoamInfo->staId,
					pRoamInfo->reasonCode);

				if (pRoamInfo->reasonCode ==
				    eWNI_TDLS_TEARDOWN_REASON_RSSI ||
				    pRoamInfo->reasonCode ==
				    eWNI_TDLS_DISCONNECTED_REASON_PEER_DELETE ||
				     pRoamInfo->reasonCode ==
					eWNI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT ||
				    pRoamInfo->reasonCode ==
					eWNI_TDLS_TEARDOWN_REASON_NO_RESPONSE) {
					reason =
						eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE;
				} else
					reason =
						eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON;

				wlan_hdd_tdls_indicate_teardown
					(pHddTdlsCtx->pAdapter, curr_peer,
					reason);
				hdd_send_wlan_tdls_teardown_event(
					eTDLS_TEARDOWN_BSS_DISCONNECT,
					curr_peer->peerMac);
			} else {
				hdd_debug("TDLS link is not connected, ignore SHOULD_PEER_DISCONNECTED, reason: %d",
					pRoamInfo->reasonCode);
			}
			status = QDF_STATUS_SUCCESS;
		}
		mutex_unlock(&pHddCtx->tdls_lock);
		break;
	}

	case eCSR_ROAM_RESULT_TDLS_CONNECTION_TRACKER_NOTIFICATION:
		mutex_lock(&pHddCtx->tdls_lock);
		pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
		if (!pHddTdlsCtx) {
			mutex_unlock(&pHddCtx->tdls_lock);
			hdd_debug("TDLS ctx is null, ignore roamResult (%d)",
				 roamResult);
			status = QDF_STATUS_E_FAILURE;
			break;
		}

		status = hdd_tdls_connection_tracker_update(pAdapter,
							    pRoamInfo,
							    pHddTdlsCtx);
		mutex_unlock(&pHddCtx->tdls_lock);
		break;

	default:
	{
		break;
	}
	}

	return status;
}
#else

inline QDF_STATUS hdd_roam_deregister_tdlssta(hdd_adapter_t *pAdapter,
					      uint8_t staId)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
hdd_roam_tdls_status_update_handler(hdd_adapter_t *pAdapter,
				    tCsrRoamInfo *pRoamInfo,
				    uint32_t roamId,
				    eRoamCmdStatus roamStatus,
				    eCsrRoamResult roamResult)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_11W
/**
 * hdd_indicate_unprot_mgmt_frame() - indicate unprotected management frame
 * @pAdapter:     pointer to the adapter
 * @nFrameLength: Length of the unprotected frame being passed
 * @pbFrames:     Pointer to the frame buffer
 * @frameType:    802.11 frame type
 *
 * This function forwards the unprotected management frame to the supplicant.
 *
 * Return: nothing
 */
static void
hdd_indicate_unprot_mgmt_frame(hdd_adapter_t *pAdapter, uint32_t nFrameLength,
			       uint8_t *pbFrames, uint8_t frameType)
{
	uint8_t type = 0;
	uint8_t subType = 0;

	hdd_debug("Frame Type = %d Frame Length = %d",
		 frameType, nFrameLength);

	/* Sanity Checks */
	if (NULL == pAdapter) {
		hdd_err("pAdapter is NULL");
		return;
	}

	if (NULL == pAdapter->dev) {
		hdd_err("pAdapter->dev is NULL");
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) {
		hdd_err("pAdapter has invalid magic");
		return;
	}

	if (!nFrameLength) {
		hdd_err("Frame Length is Invalid ZERO");
		return;
	}

	if (NULL == pbFrames) {
		hdd_err("pbFrames is NULL");
		return;
	}

	type = WLAN_HDD_GET_TYPE_FRM_FC(pbFrames[0]);
	subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(pbFrames[0]);

	/* Get pAdapter from Destination mac address of the frame */
	if (type == SIR_MAC_MGMT_FRAME && subType == SIR_MAC_MGMT_DISASSOC) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
		cfg80211_rx_unprot_mlme_mgmt(pAdapter->dev, pbFrames,
					     nFrameLength);
#else
		cfg80211_send_unprot_disassoc(pAdapter->dev, pbFrames,
					      nFrameLength);
#endif
		pAdapter->hdd_stats.hddPmfStats.numUnprotDisassocRx++;
	} else if (type == SIR_MAC_MGMT_FRAME &&
		   subType == SIR_MAC_MGMT_DEAUTH) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
		cfg80211_rx_unprot_mlme_mgmt(pAdapter->dev, pbFrames,
					     nFrameLength);
#else
		cfg80211_send_unprot_deauth(pAdapter->dev, pbFrames,
					    nFrameLength);
#endif
		pAdapter->hdd_stats.hddPmfStats.numUnprotDeauthRx++;
	} else {
		hdd_warn("Frame type %d and subtype %d are not valid",
			type, subType);
		return;
	}
}
#endif

#ifdef FEATURE_WLAN_ESE
/**
 * hdd_indicate_tsm_ie() - send traffic stream metrics ie
 * @pAdapter: pointer to adapter
 * @tid: traffic identifier
 * @state: state
 * @measInterval: measurement interval
 *
 * This function sends traffic stream metrics IE information to
 * the supplicant via wireless event.
 *
 * Return: none
 */
static void
hdd_indicate_tsm_ie(hdd_adapter_t *pAdapter, uint8_t tid,
		    uint8_t state, uint16_t measInterval)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	int nBytes = 0;

	if (NULL == pAdapter)
		return;

	/* create the event */
	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	hdd_debug("TSM Ind tid(%d) state(%d) MeasInt(%d)",
		 tid, state, measInterval);

	nBytes =
		snprintf(buf, IW_CUSTOM_MAX, "TSMIE=%d:%d:%d", tid, state,
			 measInterval);

	wrqu.data.pointer = buf;
	wrqu.data.length = nBytes;
	/* send the event */
	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_cckm_pre_auth() - send cckm preauth indication
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 *
 * This function sends cckm preauth indication to the supplicant
 * via wireless custom event.
 *
 * Return: none
 */
static void
hdd_indicate_cckm_pre_auth(hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	char *pos = buf;
	int nBytes = 0, freeBytes = IW_CUSTOM_MAX;

	if ((NULL == pAdapter) || (NULL == pRoamInfo))
		return;

	/* create the event */
	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	/* Timestamp0 is lower 32 bits and Timestamp1 is upper 32 bits */
	hdd_debug("CCXPREAUTHNOTIFY=" MAC_ADDRESS_STR " %d:%d",
		 MAC_ADDR_ARRAY(pRoamInfo->bssid.bytes),
		 pRoamInfo->timestamp[0], pRoamInfo->timestamp[1]);

	nBytes = snprintf(pos, freeBytes, "CCXPREAUTHNOTIFY=");
	pos += nBytes;
	freeBytes -= nBytes;

	qdf_mem_copy(pos, pRoamInfo->bssid.bytes, QDF_MAC_ADDR_SIZE);
	pos += QDF_MAC_ADDR_SIZE;
	freeBytes -= QDF_MAC_ADDR_SIZE;

	nBytes = snprintf(pos, freeBytes, " %u:%u",
			  pRoamInfo->timestamp[0], pRoamInfo->timestamp[1]);
	freeBytes -= nBytes;

	wrqu.data.pointer = buf;
	wrqu.data.length = (IW_CUSTOM_MAX - freeBytes);

	/* send the event */
	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_ese_adj_ap_rep_ind() - send adjacent AP report indication
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 *
 * Return: none
 */
static void
hdd_indicate_ese_adj_ap_rep_ind(hdd_adapter_t *pAdapter,
				tCsrRoamInfo *pRoamInfo)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	int nBytes = 0;

	if ((NULL == pAdapter) || (NULL == pRoamInfo))
		return;

	/* create the event */
	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	hdd_debug("CCXADJAPREP=%u", pRoamInfo->tsmRoamDelay);

	nBytes =
		snprintf(buf, IW_CUSTOM_MAX, "CCXADJAPREP=%u",
			 pRoamInfo->tsmRoamDelay);

	wrqu.data.pointer = buf;
	wrqu.data.length = nBytes;

	/* send the event */
	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_ese_bcn_report_no_results() - beacon report no scan results
 * @pAdapter: pointer to adapter
 * @measurementToken: measurement token
 * @flag: flag
 * @numBss: number of bss
 *
 * If the measurement is none and no scan results found,
 * indicate the supplicant about measurement done.
 *
 * Return: none
 */
void
hdd_indicate_ese_bcn_report_no_results(const hdd_adapter_t *pAdapter,
				       const uint16_t measurementToken,
				       const bool flag, const uint8_t numBss)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX];
	char *pos = buf;
	int nBytes = 0, freeBytes = IW_CUSTOM_MAX;

	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	hdd_debug("CCXBCNREP=%d %d %d", measurementToken,
		 flag, numBss);

	nBytes =
		snprintf(pos, freeBytes, "CCXBCNREP=%d %d %d", measurementToken,
			 flag, numBss);

	wrqu.data.pointer = buf;
	wrqu.data.length = nBytes;
	/* send the event */
	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_ese_bcn_report_ind() - send beacon report indication
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 *
 * If the measurement is none and no scan results found,
 * indicate the supplicant about measurement done.
 *
 * Return: none
 */
static void
hdd_indicate_ese_bcn_report_ind(const hdd_adapter_t *pAdapter,
				const tCsrRoamInfo *pRoamInfo)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX];
	char *pos = buf;
	int nBytes = 0, freeBytes = IW_CUSTOM_MAX;
	uint8_t i = 0, len = 0;
	uint8_t tot_bcn_ieLen = 0;  /* total size of the beacon report data */
	uint8_t lastSent = 0, sendBss = 0;
	int bcnRepFieldSize =
		sizeof(pRoamInfo->pEseBcnReportRsp->bcnRepBssInfo[0].
		       bcnReportFields);
	uint8_t ieLenByte = 1;
	/*
	 * CCXBCNREP=meas_tok<sp>flag<sp>no_of_bss<sp>tot_bcn_ie_len = 18 bytes
	 */
#define ESEBCNREPHEADER_LEN  (18)

	if ((NULL == pAdapter) || (NULL == pRoamInfo))
		return;

	/*
	 * Custom event can pass maximum of 256 bytes of data,
	 * based on the IE len we need to identify how many BSS info can
	 * be filled in to custom event data.
	 */
	/*
	 * meas_tok<sp>flag<sp>no_of_bss<sp>tot_bcn_ie_len bcn_rep_data
	 * bcn_rep_data will have bcn_rep_fields,ie_len,ie without any spaces
	 * CCXBCNREP=meas_tok<sp>flag<sp>no_of_bss<sp>tot_bcn_ie_len = 18 bytes
	 */

	if ((pRoamInfo->pEseBcnReportRsp->flag >> 1)
	    && (!pRoamInfo->pEseBcnReportRsp->numBss)) {
		hdd_debug("Measurement Done but no scan results");
		/* If the measurement is none and no scan results found,
		 * indicate the supplicant about measurement done
		 */
		hdd_indicate_ese_bcn_report_no_results(
				pAdapter,
				pRoamInfo->pEseBcnReportRsp->
				measurementToken,
				pRoamInfo->pEseBcnReportRsp->flag,
				pRoamInfo->pEseBcnReportRsp->numBss);
	} else {
		while (lastSent < pRoamInfo->pEseBcnReportRsp->numBss) {
			memset(&wrqu, '\0', sizeof(wrqu));
			memset(buf, '\0', sizeof(buf));
			tot_bcn_ieLen = 0;
			sendBss = 0;
			pos = buf;
			freeBytes = IW_CUSTOM_MAX;

			for (i = lastSent;
			     i < pRoamInfo->pEseBcnReportRsp->numBss; i++) {
				len =
					bcnRepFieldSize + ieLenByte +
					pRoamInfo->pEseBcnReportRsp->
					bcnRepBssInfo[i].ieLen;
				if ((len + tot_bcn_ieLen) >
				    (IW_CUSTOM_MAX - ESEBCNREPHEADER_LEN)) {
					break;
				}
				tot_bcn_ieLen += len;
				sendBss++;
				hdd_debug("i(%d) sizeof bcnReportFields(%d) IeLength(%d) Length of Ie(%d) totLen(%d)",
					 i, bcnRepFieldSize, 1,
					 pRoamInfo->pEseBcnReportRsp->
					 bcnRepBssInfo[i].ieLen, tot_bcn_ieLen);
			}

			hdd_debug("Sending %d BSS Info", sendBss);
			hdd_debug("CCXBCNREP=%d %d %d %d",
				 pRoamInfo->pEseBcnReportRsp->measurementToken,
				 pRoamInfo->pEseBcnReportRsp->flag, sendBss,
				 tot_bcn_ieLen);

			nBytes = snprintf(pos, freeBytes, "CCXBCNREP=%d %d %d ",
					  pRoamInfo->pEseBcnReportRsp->
					  measurementToken,
					  pRoamInfo->pEseBcnReportRsp->flag,
					  sendBss);
			pos += nBytes;
			freeBytes -= nBytes;

			/* Copy total Beacon report data length */
			qdf_mem_copy(pos, (char *)&tot_bcn_ieLen,
				     sizeof(tot_bcn_ieLen));
			pos += sizeof(tot_bcn_ieLen);
			freeBytes -= sizeof(tot_bcn_ieLen);

			for (i = 0; i < sendBss; i++) {
				hdd_debug("ChanNum(%d) Spare(%d) MeasDuration(%d)"
				       " PhyType(%d) RecvSigPower(%d) ParentTSF(%u)"
				       " TargetTSF[0](%u) TargetTSF[1](%u) BeaconInterval(%u)"
				       " CapabilityInfo(%d) BSSID(%02X:%02X:%02X:%02X:%02X:%02X)",
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       ChanNum,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Spare,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       MeasDuration,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       PhyType,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       RecvSigPower,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       ParentTsf,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       TargetTsf[0],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       TargetTsf[1],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       BcnInterval,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       CapabilityInfo,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[0],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[1],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[2],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[3],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[4],
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[5]);

				/* bcn report fields are copied */
				len =
					sizeof(pRoamInfo->pEseBcnReportRsp->
					       bcnRepBssInfo[i +
							     lastSent].
					       bcnReportFields);
				qdf_mem_copy(pos,
					     (char *)&pRoamInfo->
					     pEseBcnReportRsp->bcnRepBssInfo[i +
									     lastSent].
					     bcnReportFields, len);
				pos += len;
				freeBytes -= len;

				/* Add 1 byte of ie len */
				len =
					pRoamInfo->pEseBcnReportRsp->
					bcnRepBssInfo[i + lastSent].ieLen;
				qdf_mem_copy(pos, (char *)&len, sizeof(len));
				pos += sizeof(len);
				freeBytes -= sizeof(len);

				/* copy IE from scan results */
				qdf_mem_copy(pos,
					     (char *)pRoamInfo->
					     pEseBcnReportRsp->bcnRepBssInfo[i +
									     lastSent].
					     pBuf, len);
				pos += len;
				freeBytes -= len;
			}

			wrqu.data.pointer = buf;
			wrqu.data.length = IW_CUSTOM_MAX - freeBytes;

			/* send the event */
			wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu,
					    buf);
			lastSent += sendBss;
		}
	}
}

#endif /* FEATURE_WLAN_ESE */

/**
 * hdd_is_8021x_sha256_auth_type() - check authentication type to 8021x_sha256
 * @pHddStaCtx:	Station Context
 *
 * API to check if the connection authentication type is 8021x_sha256.
 *
 * Return: bool
 */
#ifdef WLAN_FEATURE_11W
static inline bool hdd_is_8021x_sha256_auth_type(hdd_station_ctx_t *pHddStaCtx)
{
	return eCSR_AUTH_TYPE_RSN_8021X_SHA256 ==
				pHddStaCtx->conn_info.authType;
}
#else
static inline bool hdd_is_8021x_sha256_auth_type(hdd_station_ctx_t *pHddStaCtx)
{
	return false;
}
#endif

/*
 * hdd_roam_channel_switch_handler() - hdd channel switch handler
 * @adapter: Pointer to adapter context
 * @roam_info: Pointer to roam info
 *
 * Return: None
 */
static void hdd_roam_channel_switch_handler(hdd_adapter_t *adapter,
				tCsrRoamInfo *roam_info)
{
	struct hdd_chan_change_params chan_change;
	struct cfg80211_bss *bss;
	struct net_device *dev = adapter->dev;
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct wiphy *wiphy = wdev->wiphy;
	QDF_STATUS status;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_debug("channel switch for session:%d to channel:%d",
		adapter->sessionId, roam_info->chan_info.chan_id);

	chan_change.chan = roam_info->chan_info.chan_id;
	chan_change.chan_params.ch_width =
		roam_info->chan_info.ch_width;
	chan_change.chan_params.sec_ch_offset =
		roam_info->chan_info.sec_ch_offset;
	chan_change.chan_params.center_freq_seg0 =
		roam_info->chan_info.band_center_freq1;
	chan_change.chan_params.center_freq_seg1 =
		roam_info->chan_info.band_center_freq2;

	bss = wlan_hdd_cfg80211_update_bss_db(adapter, roam_info);
	if (NULL == bss)
		hdd_err("%s: unable to create BSS entry", adapter->dev->name);
	else
		cfg80211_put_bss(wiphy, bss);

	status = hdd_chan_change_notify(adapter, adapter->dev, chan_change);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("channel change notification failed");

	status = policy_mgr_set_hw_mode_on_channel_switch(hdd_ctx->hdd_psoc,
		adapter->sessionId);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_debug("set hw mode change not done");
}

/**
 * hdd_sme_roam_callback() - hdd sme roam callback
 * @pContext: pointer to adapter context
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS
hdd_sme_roam_callback(void *pContext, tCsrRoamInfo *pRoamInfo, uint32_t roamId,
		      eRoamCmdStatus roamStatus, eCsrRoamResult roamResult)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) pContext;
	hdd_wext_state_t *pWextState = NULL;
	hdd_station_ctx_t *pHddStaCtx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct cfg80211_bss *bss_status;
	hdd_context_t *hdd_ctx;

	hdd_debug("CSR Callback: status= %d result= %d roamID=%d",
		 roamStatus, roamResult, roamId);

	/* Sanity check */
	if ((NULL == pAdapter) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return QDF_STATUS_E_FAILURE;
	}

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	/* Omitting eCSR_ROAM_UPDATE_SCAN_RESULT as this is too frequent */
	if (eCSR_ROAM_UPDATE_SCAN_RESULT != roamStatus)
		MTRACE(qdf_trace(QDF_MODULE_ID_HDD, TRACE_CODE_HDD_RX_SME_MSG,
				 pAdapter->sessionId, roamStatus));

	switch (roamStatus) {
	case eCSR_ROAM_SESSION_OPENED:
		set_bit(SME_SESSION_OPENED, &pAdapter->event_flags);
		complete(&pAdapter->session_open_comp_var);
		hdd_debug("session %d opened", pAdapter->sessionId);
		break;

	/*
	 * We did pre-auth,then we attempted a 11r or ese reassoc.
	 * reassoc failed due to failure, timeout, reject from ap
	 * in any case tell the OS, our carrier is off and mark
	 * interface down.
	 */
	case eCSR_ROAM_FT_REASSOC_FAILED:
		hdd_err("Reassoc Failed with roamStatus: %d roamResult: %d SessionID: %d",
			 roamStatus, roamResult, pAdapter->sessionId);
		qdf_ret_status =
			hdd_dis_connect_handler(pAdapter, pRoamInfo, roamId,
						roamStatus, roamResult);
		pHddStaCtx->ft_carrier_on = false;
		pHddStaCtx->hdd_ReassocScenario = false;
		hdd_debug("hdd_ReassocScenario set to: %d, ReAssoc Failed, session: %d",
			 pHddStaCtx->hdd_ReassocScenario, pAdapter->sessionId);
		break;

	case eCSR_ROAM_FT_START:
		/*
		 * When we roam for ESE and 11r, we dont want the OS to be
		 * informed that the link is down. So mark the link ready for
		 * ft_start. After this the eCSR_ROAM_SHOULD_ROAM will
		 * be received. Where in we will not mark the link down
		 * Also we want to stop tx at this point when we will be
		 * doing disassoc at this time. This saves 30-60 msec
		 * after reassoc.
		 */
		hdd_info("Disabling queues");
		hdd_debug("Roam Synch Ind: NAPI Serialize ON");
		hdd_napi_serialize(1);
		wlan_hdd_netif_queue_control(pAdapter,
				WLAN_STOP_ALL_NETIF_QUEUE,
				WLAN_CONTROL_PATH);
		status = hdd_roam_deregister_sta(pAdapter,
					pHddStaCtx->conn_info.staId[0]);
		if (!QDF_IS_STATUS_SUCCESS(status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		pHddStaCtx->ft_carrier_on = true;
		pHddStaCtx->hdd_ReassocScenario = true;
		hdd_info("hdd_ReassocScenario set to: %d, due to eCSR_ROAM_FT_START, session: %d",
			 pHddStaCtx->hdd_ReassocScenario, pAdapter->sessionId);
		break;
	case eCSR_ROAM_NAPI_OFF:
		hdd_debug("After Roam Synch Comp: NAPI Serialize OFF");
		hdd_napi_serialize(0);
		hdd_set_roaming_in_progress(false);
		if (pAdapter->defer_disconnect)
			hdd_process_defer_disconnect(pAdapter);
		break;
	case eCSR_ROAM_SHOULD_ROAM:
		/* notify apps that we can't pass traffic anymore */
		hdd_debug("Disabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_STOP_ALL_NETIF_QUEUE,
					   WLAN_CONTROL_PATH);
		if (pHddStaCtx->ft_carrier_on == false) {
			wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_NETIF_CARRIER_OFF,
					   WLAN_CONTROL_PATH);
		}
		break;
	case eCSR_ROAM_LOSTLINK:
		if (roamResult == eCSR_ROAM_RESULT_LOSTLINK) {
			hdd_debug("Roaming started due to connection lost");
			hdd_info("Disabling queues");
			wlan_hdd_netif_queue_control(pAdapter,
					WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);
			break;
		}
	case eCSR_ROAM_DISASSOCIATED:
	{
		hdd_debug("****eCSR_ROAM_DISASSOCIATED****");
		hdd_napi_serialize(0);
		hdd_set_connection_in_progress(false);
		hdd_set_roaming_in_progress(false);
		pAdapter->defer_disconnect = 0;

		/* Call to clear any MC Addr List filter applied after
		 * successful connection.
		 */
		hdd_disable_and_flush_mc_addr_list(pAdapter,
			pmo_peer_disconnect);
		qdf_ret_status =
			hdd_dis_connect_handler(pAdapter, pRoamInfo, roamId,
						roamStatus, roamResult);
	}
	break;
	case eCSR_ROAM_IBSS_LEAVE:
		hdd_debug("****eCSR_ROAM_IBSS_LEAVE****");
		qdf_ret_status =
			hdd_dis_connect_handler(pAdapter, pRoamInfo, roamId,
						roamStatus, roamResult);
		break;
	case eCSR_ROAM_ASSOCIATION_COMPLETION:
		hdd_debug("****eCSR_ROAM_ASSOCIATION_COMPLETION****");
		/*
		 * To Do - address probable memory leak with WEP encryption upon
		 * successful association.
		 */
		if (eCSR_ROAM_RESULT_ASSOCIATED != roamResult) {
			/* Clear saved connection information in HDD */
			hdd_conn_remove_connect_info(
				WLAN_HDD_GET_STATION_CTX_PTR(pAdapter));
		}
		qdf_ret_status =
			hdd_association_completion_handler(pAdapter, pRoamInfo,
							   roamId, roamStatus,
							   roamResult);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (pRoamInfo)
			pRoamInfo->roamSynchInProgress = false;
#endif
		break;
	case eCSR_ROAM_CANCELLED:
		hdd_debug("****eCSR_ROAM_CANCELLED****");
	case eCSR_ROAM_ASSOCIATION_FAILURE:
		qdf_ret_status = hdd_association_completion_handler(pAdapter,
								    pRoamInfo,
								    roamId,
								    roamStatus,
								    roamResult);
		break;
	case eCSR_ROAM_IBSS_IND:
		hdd_roam_ibss_indication_handler(pAdapter, pRoamInfo, roamId,
						 roamStatus, roamResult);
		break;

	case eCSR_ROAM_CONNECT_STATUS_UPDATE:
		qdf_ret_status =
			roam_roam_connect_status_update_handler(pAdapter,
								pRoamInfo,
								roamId,
								roamStatus,
								roamResult);
		break;

	case eCSR_ROAM_MIC_ERROR_IND:
		qdf_ret_status =
			hdd_roam_mic_error_indication_handler(pAdapter,
							      pRoamInfo,
							      roamId,
							      roamStatus,
							      roamResult);
		break;

	case eCSR_ROAM_SET_KEY_COMPLETE:
	{
		qdf_ret_status =
			hdd_roam_set_key_complete_handler(pAdapter, pRoamInfo,
							  roamId, roamStatus,
							  roamResult);
		if (eCSR_ROAM_RESULT_AUTHENTICATED == roamResult) {
			pHddStaCtx->hdd_ReassocScenario = false;
			hdd_debug("hdd_ReassocScenario set to: %d, set key complete, session: %d",
			       pHddStaCtx->hdd_ReassocScenario,
			       pAdapter->sessionId);
		}
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (pRoamInfo != NULL)
			pRoamInfo->roamSynchInProgress = false;
#endif
		break;

	case eCSR_ROAM_FT_RESPONSE:
		hdd_send_ft_event(pAdapter);
		break;

	case eCSR_ROAM_PMK_NOTIFY:
		if (eCSR_AUTH_TYPE_RSN == pHddStaCtx->conn_info.authType
				|| hdd_is_8021x_sha256_auth_type(pHddStaCtx)) {
			/* notify the supplicant of a new candidate */
			qdf_ret_status =
				wlan_hdd_cfg80211_pmksa_candidate_notify(
						pAdapter, pRoamInfo, 1, false);
		}
		break;

#ifdef FEATURE_WLAN_LFR_METRICS
	case eCSR_ROAM_PREAUTH_INIT_NOTIFY:
		/* This event is to notify pre-auth initiation */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth(pAdapter,
							   pRoamInfo)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_PREAUTH_STATUS_SUCCESS:
		/*
		 * This event will notify pre-auth completion in case of success
		 */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth_status(pAdapter,
							 pRoamInfo, 1)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_PREAUTH_STATUS_FAILURE:
		/*
		 * This event will notify pre-auth completion incase of failure.
		 */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth_status(pAdapter,
								pRoamInfo, 0)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_HANDOVER_SUCCESS:
		/* This event is to notify handover success.
		 * It will be only invoked on success
		 */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_handover(pAdapter,
							    pRoamInfo)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
#endif
	case eCSR_ROAM_REMAIN_CHAN_READY:
		hdd_remain_chan_ready_handler(pAdapter, pRoamInfo->roc_scan_id);
		break;
#ifdef FEATURE_WLAN_TDLS
	case eCSR_ROAM_TDLS_STATUS_UPDATE:
		qdf_ret_status =
			hdd_roam_tdls_status_update_handler(pAdapter, pRoamInfo,
							    roamId,
							    roamStatus,
							    roamResult);
		break;
	case eCSR_ROAM_RESULT_MGMT_TX_COMPLETE_IND:
		wlan_hdd_tdls_mgmt_completion_callback(pAdapter,
						       pRoamInfo->reasonCode);
		break;
#endif
#ifdef WLAN_FEATURE_11W
	case eCSR_ROAM_UNPROT_MGMT_FRAME_IND:
		hdd_indicate_unprot_mgmt_frame(pAdapter,
					       pRoamInfo->nFrameLength,
					       pRoamInfo->pbFrames,
					       pRoamInfo->frameType);
		break;
#endif
#ifdef FEATURE_WLAN_ESE
	case eCSR_ROAM_TSM_IE_IND:
		hdd_indicate_tsm_ie(pAdapter, pRoamInfo->tsmIe.tsid,
				    pRoamInfo->tsmIe.state,
				    pRoamInfo->tsmIe.msmt_interval);
		break;

	case eCSR_ROAM_CCKM_PREAUTH_NOTIFY:
	{
		if (eCSR_AUTH_TYPE_CCKM_WPA ==
		    pHddStaCtx->conn_info.authType
		    || eCSR_AUTH_TYPE_CCKM_RSN ==
		    pHddStaCtx->conn_info.authType) {
			hdd_indicate_cckm_pre_auth(pAdapter, pRoamInfo);
		}
		break;
	}

	case eCSR_ROAM_ESE_ADJ_AP_REPORT_IND:
	{
		hdd_indicate_ese_adj_ap_rep_ind(pAdapter, pRoamInfo);
		break;
	}

	case eCSR_ROAM_ESE_BCN_REPORT_IND:
	{
		hdd_indicate_ese_bcn_report_ind(pAdapter, pRoamInfo);
		break;
	}
#endif /* FEATURE_WLAN_ESE */
	case eCSR_ROAM_STA_CHANNEL_SWITCH:
		hdd_roam_channel_switch_handler(pAdapter, pRoamInfo);
		break;

	case eCSR_ROAM_UPDATE_SCAN_RESULT:
		if ((NULL != pRoamInfo) && (NULL != pRoamInfo->pBssDesc)) {
			bss_status = wlan_hdd_cfg80211_inform_bss_frame(
					pAdapter, pRoamInfo->pBssDesc);
			if (NULL == bss_status)
				hdd_debug("UPDATE_SCAN_RESULT returned NULL");
			else
				cfg80211_put_bss(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)) || defined(WITH_BACKPORTS)
					(WLAN_HDD_GET_CTX(pAdapter))->wiphy,
#endif
					bss_status);
		}
		break;
	case eCSR_ROAM_NDP_STATUS_UPDATE:
		hdd_ndp_event_handler(pAdapter, pRoamInfo, roamId, roamStatus,
			roamResult);
		break;
	case eCSR_ROAM_START:
		hdd_debug("Process ROAM_START from firmware");
		wlan_hdd_netif_queue_control(pAdapter,
				WLAN_STOP_ALL_NETIF_QUEUE,
				WLAN_CONTROL_PATH);
		hdd_napi_serialize(1);
		hdd_set_connection_in_progress(true);
		hdd_set_roaming_in_progress(true);
		policy_mgr_restart_opportunistic_timer(hdd_ctx->hdd_psoc, true);
		break;
	case eCSR_ROAM_ABORT:
		hdd_debug("Firmware aborted roaming operation, previous connection is still valid");
		hdd_napi_serialize(0);
		wlan_hdd_netif_queue_control(pAdapter,
				WLAN_WAKE_ALL_NETIF_QUEUE,
				WLAN_CONTROL_PATH);
		hdd_set_connection_in_progress(false);
		hdd_set_roaming_in_progress(false);
		/*
		 * If disconnect operation is in deferred state, do it now.
		 */
		if (pAdapter->defer_disconnect)
			hdd_process_defer_disconnect(pAdapter);
		break;

	default:
		break;
	}
	return qdf_ret_status;
}

/**
 * hdd_translate_rsn_to_csr_auth_type() - Translate RSN to CSR auth type
 * @auth_suite: auth suite
 *
 * Return: eCsrAuthType enumeration
 */
eCsrAuthType hdd_translate_rsn_to_csr_auth_type(uint8_t auth_suite[4])
{
	eCsrAuthType auth_type;
	/* is the auth type supported? */
	if (memcmp(auth_suite, ccp_rsn_oui01, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_RSN;
	} else if (memcmp(auth_suite, ccp_rsn_oui02, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_RSN_PSK;
	} else if (memcmp(auth_suite, ccp_rsn_oui04, 4) == 0) {
		/* Check for 11r FT Authentication with PSK */
		auth_type = eCSR_AUTH_TYPE_FT_RSN_PSK;
	} else if (memcmp(auth_suite, ccp_rsn_oui03, 4) == 0) {
		/* Check for 11R FT Authentication with 802.1X */
		auth_type = eCSR_AUTH_TYPE_FT_RSN;
	} else
#ifdef FEATURE_WLAN_ESE
	if (memcmp(auth_suite, ccp_rsn_oui06, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_CCKM_RSN;
	} else
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_11W
	if (memcmp(auth_suite, ccp_rsn_oui07, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_RSN_PSK_SHA256;
	} else if (memcmp(auth_suite, ccp_rsn_oui08, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_RSN_8021X_SHA256;
	} else
#endif
	{
		auth_type = eCSR_AUTH_TYPE_UNKNOWN;
	}
	return auth_type;
}

/**
 * hdd_translate_wpa_to_csr_auth_type() - Translate WPA to CSR auth type
 * @auth_suite: auth suite
 *
 * Return: eCsrAuthType enumeration
 */
eCsrAuthType hdd_translate_wpa_to_csr_auth_type(uint8_t auth_suite[4])
{
	eCsrAuthType auth_type;
	/* is the auth type supported? */
	if (memcmp(auth_suite, ccp_wpa_oui01, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_WPA;
	} else if (memcmp(auth_suite, ccp_wpa_oui02, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_WPA_PSK;
	} else
#ifdef FEATURE_WLAN_ESE
	if (memcmp(auth_suite, ccp_wpa_oui06, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_CCKM_WPA;
	} else
#endif /* FEATURE_WLAN_ESE */
	{
		auth_type = eCSR_AUTH_TYPE_UNKNOWN;
	}
	hdd_debug("auth_type: %d", auth_type);
	return auth_type;
}

/**
 * hdd_translate_rsn_to_csr_encryption_type() -
 *	Translate RSN to CSR encryption type
 * @cipher_suite: cipher suite
 *
 * Return: eCsrEncryptionType enumeration
 */
eCsrEncryptionType
hdd_translate_rsn_to_csr_encryption_type(uint8_t cipher_suite[4])
{
	eCsrEncryptionType cipher_type;

	if (memcmp(cipher_suite, ccp_rsn_oui04, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_AES;
	else if (memcmp(cipher_suite, ccp_rsn_oui02, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
	else if (memcmp(cipher_suite, ccp_rsn_oui00, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_NONE;
	else if (memcmp(cipher_suite, ccp_rsn_oui01, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
	else if (memcmp(cipher_suite, ccp_rsn_oui05, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
	else
		cipher_type = eCSR_ENCRYPT_TYPE_FAILED;

	hdd_debug("cipher_type: %d", cipher_type);
	return cipher_type;
}

/**
 * hdd_translate_wpa_to_csr_encryption_type() -
 *	Translate WPA to CSR encryption type
 * @cipher_suite: cipher suite
 *
 * Return: eCsrEncryptionType enumeration
 */
eCsrEncryptionType
hdd_translate_wpa_to_csr_encryption_type(uint8_t cipher_suite[4])
{
	eCsrEncryptionType cipher_type;

	if (memcmp(cipher_suite, ccp_wpa_oui04, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_AES;
	else if (memcmp(cipher_suite, ccp_wpa_oui02, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
	else if (memcmp(cipher_suite, ccp_wpa_oui00, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_NONE;
	else if (memcmp(cipher_suite, ccp_wpa_oui01, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
	else if (memcmp(cipher_suite, ccp_wpa_oui05, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
	else
		cipher_type = eCSR_ENCRYPT_TYPE_FAILED;

	hdd_debug("cipher_type: %d", cipher_type);
	return cipher_type;
}

/**
 * hdd_process_genie() - process gen ie
 * @pAdapter: pointer to adapter
 * @bssid: pointer to mac address
 * @pEncryptType: pointer to encryption type
 * @mcEncryptType: pointer to multicast encryption type
 * @pAuthType: pointer to auth type
 *
 * Return: 0 on success, error number otherwise
 */
static int32_t hdd_process_genie(hdd_adapter_t *pAdapter,
				 u8 *bssid,
				 eCsrEncryptionType *pEncryptType,
				 eCsrEncryptionType *mcEncryptType,
				 eCsrAuthType *pAuthType,
#ifdef WLAN_FEATURE_11W
				 uint8_t *pMfpRequired, uint8_t *pMfpCapable,
#endif
				 uint16_t gen_ie_len, uint8_t *gen_ie)
{
	tHalHandle halHandle = WLAN_HDD_GET_HAL_CTX(pAdapter);
	QDF_STATUS result;
	tDot11fIERSN dot11RSNIE;
	tDot11fIEWPA dot11WPAIE;
	uint32_t i;
	uint8_t *pRsnIe;
	uint16_t RSNIeLen;
	tPmkidCacheInfo PMKIDCache[4];  /* Local transfer memory */
	bool updatePMKCache = false;

	/*
	 * Clear struct of tDot11fIERSN and tDot11fIEWPA specifically
	 * setting present flag to 0.
	 */
	memset(&dot11WPAIE, 0, sizeof(tDot11fIEWPA));
	memset(&dot11RSNIE, 0, sizeof(tDot11fIERSN));

	/* Type check */
	if (gen_ie[0] == DOT11F_EID_RSN) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_RSN_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_RSN_MAX_LEN)) {
			hdd_err("Invalid DOT11F RSN IE length: %d",
				gen_ie_len);
			return -EINVAL;
		}
		/* Skip past the EID byte and length byte */
		pRsnIe = gen_ie + 2;
		RSNIeLen = gen_ie_len - 2;
		/* Unpack the RSN IE */
		dot11f_unpack_ie_rsn((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11RSNIE, false);
		/* Copy out the encryption and authentication types */
		hdd_debug("pairwise cipher suite count: %d",
			 dot11RSNIE.pwise_cipher_suite_count);
		hdd_debug("authentication suite count: %d",
			 dot11RSNIE.akm_suite_count);
		/* dot11RSNIE.akm_suite_count */
		/* Just translate the FIRST one */
		*pAuthType =
			hdd_translate_rsn_to_csr_auth_type(
					dot11RSNIE.akm_suites[0]);
		/* dot11RSNIE.pwise_cipher_suite_count */
		*pEncryptType =
			hdd_translate_rsn_to_csr_encryption_type(
					dot11RSNIE.pwise_cipher_suites[0]);
		/* dot11RSNIE.gp_cipher_suite_count */
		*mcEncryptType =
			hdd_translate_rsn_to_csr_encryption_type(
					dot11RSNIE.gp_cipher_suite);
#ifdef WLAN_FEATURE_11W
		*pMfpRequired = (dot11RSNIE.RSN_Cap[0] >> 6) & 0x1;
		*pMfpCapable = (dot11RSNIE.RSN_Cap[0] >> 7) & 0x1;
#endif
		/* Set the PMKSA ID Cache for this interface */
		for (i = 0; i < dot11RSNIE.pmkid_count; i++) {
			if (is_zero_ether_addr(bssid)) {
				hdd_warn("MAC address is all zeroes");
				break;
			}
			updatePMKCache = true;
			/*
			 * For right now, I assume setASSOCIATE() has passed
			 * in the bssid.
			 */
			qdf_mem_copy(PMKIDCache[i].BSSID.bytes,
				     bssid, QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(PMKIDCache[i].PMKID,
				     dot11RSNIE.pmkid[i], CSR_RSN_PMKID_SIZE);
		}

		if (updatePMKCache) {
			/*
			 * Calling csr_roam_set_pmkid_cache to configure the
			 * PMKIDs into the cache.
			 */
			hdd_debug("Calling sme_roam_set_pmkid_cache with cache entry %d.",
				 i);
			/* Finally set the PMKSA ID Cache in CSR */
			result =
				sme_roam_set_pmkid_cache(halHandle,
							 pAdapter->sessionId,
							 PMKIDCache,
							 dot11RSNIE.pmkid_count,
							 false);
		}
	} else if (gen_ie[0] == DOT11F_EID_WPA) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_WPA_MAX_LEN)) {
			hdd_err("Invalid DOT11F WPA IE length: %d",
				gen_ie_len);
			return -EINVAL;
		}
		/* Skip past the EID and length byte - and four byte WiFi OUI */
		pRsnIe = gen_ie + 2 + 4;
		RSNIeLen = gen_ie_len - (2 + 4);
		/* Unpack the WPA IE */
		dot11f_unpack_ie_wpa((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11WPAIE, false);
		/* Copy out the encryption and authentication types */
		hdd_debug("WPA unicast cipher suite count: %d",
			 dot11WPAIE.unicast_cipher_count);
		hdd_debug("WPA authentication suite count: %d",
			 dot11WPAIE.auth_suite_count);
		/* dot11WPAIE.auth_suite_count */
		/* Just translate the FIRST one */
		*pAuthType =
			hdd_translate_wpa_to_csr_auth_type(
					dot11WPAIE.auth_suites[0]);
		/* dot11WPAIE.unicast_cipher_count */
		*pEncryptType =
			hdd_translate_wpa_to_csr_encryption_type(
					dot11WPAIE.unicast_ciphers[0]);
		/* dot11WPAIE.unicast_cipher_count */
		*mcEncryptType =
			hdd_translate_wpa_to_csr_encryption_type(
					dot11WPAIE.multicast_cipher);
	} else {
		hdd_warn("gen_ie[0]: %d", gen_ie[0]);
		return -EINVAL;
	}
	return 0;
}

/**
 * hdd_set_genie_to_csr() - set genie to csr
 * @pAdapter: pointer to adapter
 * @RSNAuthType: pointer to auth type
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_set_genie_to_csr(hdd_adapter_t *pAdapter, eCsrAuthType *RSNAuthType)
{
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	uint32_t status = 0;
	eCsrEncryptionType RSNEncryptType;
	eCsrEncryptionType mcRSNEncryptType;
#ifdef WLAN_FEATURE_11W
	uint8_t RSNMfpRequired = 0;
	uint8_t RSNMfpCapable = 0;
#endif
	u8 bssid[ETH_ALEN];        /* MAC address of assoc peer */
	/* MAC address of assoc peer */
	/* But, this routine is only called when we are NOT associated. */
	qdf_mem_copy(bssid,
		     pWextState->roamProfile.BSSIDs.bssid,
		     sizeof(bssid));
	if (pWextState->WPARSNIE[0] == DOT11F_EID_RSN
	    || pWextState->WPARSNIE[0] == DOT11F_EID_WPA) {
		/* continue */
	} else {
		return 0;
	}
	/* The actual processing may eventually be more extensive than this. */
	/* Right now, just consume any PMKIDs that are  sent in by the app. */
	status = hdd_process_genie(pAdapter, bssid,
				   &RSNEncryptType,
				   &mcRSNEncryptType, RSNAuthType,
#ifdef WLAN_FEATURE_11W
				   &RSNMfpRequired, &RSNMfpCapable,
#endif
				   pWextState->WPARSNIE[1] + 2,
				   pWextState->WPARSNIE);
	if (status == 0) {
		/*
		 * Now copy over all the security attributes
		 * you have parsed out.
		 */
		pWextState->roamProfile.EncryptionType.numEntries = 1;
		pWextState->roamProfile.mcEncryptionType.numEntries = 1;

		pWextState->roamProfile.EncryptionType.encryptionType[0] = RSNEncryptType;      /* Use the cipher type in the RSN IE */
		pWextState->roamProfile.mcEncryptionType.encryptionType[0] =
			mcRSNEncryptType;

		if ((QDF_IBSS_MODE == pAdapter->device_mode) &&
		    ((eCSR_ENCRYPT_TYPE_AES == mcRSNEncryptType) ||
		     (eCSR_ENCRYPT_TYPE_TKIP == mcRSNEncryptType))) {
			/*
			 * For wpa none supplicant sends the WPA IE with unicast
			 * cipher as eCSR_ENCRYPT_TYPE_NONE ,where as the
			 * multicast cipher as either AES/TKIP based on group
			 * cipher configuration mentioned in the
			 * wpa_supplicant.conf.
			 */

			/* Set the unicast cipher same as multicast cipher */
			pWextState->roamProfile.EncryptionType.encryptionType[0]
				= mcRSNEncryptType;
		}
#ifdef WLAN_FEATURE_11W
		hdd_debug("RSNMfpRequired = %d, RSNMfpCapable = %d",
			 RSNMfpRequired, RSNMfpCapable);
		pWextState->roamProfile.MFPRequired = RSNMfpRequired;
		pWextState->roamProfile.MFPCapable = RSNMfpCapable;
#endif
		hdd_debug("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d",
			 *RSNAuthType, RSNEncryptType, mcRSNEncryptType);
	}
	return 0;
}

/**
 * hdd_set_csr_auth_type() - set csr auth type
 * @pAdapter: pointer to adapter
 * @RSNAuthType: auth type
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_set_csr_auth_type(hdd_adapter_t *pAdapter, eCsrAuthType RSNAuthType)
{
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	pRoamProfile->AuthType.numEntries = 1;
	hdd_debug("pHddStaCtx->conn_info.authType = %d",
		 pHddStaCtx->conn_info.authType);

	switch (pHddStaCtx->conn_info.authType) {
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
#ifdef FEATURE_WLAN_ESE
	case eCSR_AUTH_TYPE_CCKM_WPA:
	case eCSR_AUTH_TYPE_CCKM_RSN:
#endif
		if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_DISABLED) {

			pRoamProfile->AuthType.authType[0] =
				eCSR_AUTH_TYPE_OPEN_SYSTEM;
		} else if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_WPA) {

#ifdef FEATURE_WLAN_ESE
			if ((RSNAuthType == eCSR_AUTH_TYPE_CCKM_WPA) &&
			    ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)
			     == IW_AUTH_KEY_MGMT_802_1X)) {
				hdd_debug("set authType to CCKM WPA. AKM also 802.1X.");
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_WPA;
			} else if (RSNAuthType == eCSR_AUTH_TYPE_CCKM_WPA) {
				hdd_debug("Last chance to set authType to CCKM WPA.");
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_WPA;
			} else
#endif
			if ((pWextState->
			     authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)
			    == IW_AUTH_KEY_MGMT_802_1X) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WPA;
			} else
			if ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_PSK)
			    == IW_AUTH_KEY_MGMT_PSK) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WPA_PSK;
			} else {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WPA_NONE;
			}
		}
		if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_WPA2) {
#ifdef FEATURE_WLAN_ESE
			if ((RSNAuthType == eCSR_AUTH_TYPE_CCKM_RSN) &&
			    ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)
			     == IW_AUTH_KEY_MGMT_802_1X)) {
				hdd_debug("set authType to CCKM RSN. AKM also 802.1X.");
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_RSN;
			} else if (RSNAuthType == eCSR_AUTH_TYPE_CCKM_RSN) {
				hdd_debug("Last chance to set authType to CCKM RSN.");
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_RSN;
			} else
#endif

			if ((RSNAuthType == eCSR_AUTH_TYPE_FT_RSN) &&
			    ((pWextState->
			      authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)
			     == IW_AUTH_KEY_MGMT_802_1X)) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_FT_RSN;
			} else if ((RSNAuthType == eCSR_AUTH_TYPE_FT_RSN_PSK)
				   &&
				   ((pWextState->
				     authKeyMgmt & IW_AUTH_KEY_MGMT_PSK)
				    == IW_AUTH_KEY_MGMT_PSK)) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_FT_RSN_PSK;
			} else

#ifdef WLAN_FEATURE_11W
			if (RSNAuthType == eCSR_AUTH_TYPE_RSN_PSK_SHA256) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN_PSK_SHA256;
			} else if (RSNAuthType ==
				   eCSR_AUTH_TYPE_RSN_8021X_SHA256) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN_8021X_SHA256;
			} else
#endif

			if ((pWextState->
			     authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)
			    == IW_AUTH_KEY_MGMT_802_1X) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN;
			} else
			if ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_PSK)
			    == IW_AUTH_KEY_MGMT_PSK) {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN_PSK;
			} else {
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_UNKNOWN;
			}
		}
		break;

	case eCSR_AUTH_TYPE_SHARED_KEY:

		pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;
		break;
	default:

#ifdef FEATURE_WLAN_ESE
		hdd_debug("In default, unknown auth type.");
#endif /* FEATURE_WLAN_ESE */
		pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
		break;
	}

	hdd_debug("Set roam Authtype to %d",
		 pWextState->roamProfile.AuthType.authType[0]);

	return 0;
}

/**
 * __iw_set_essid() - This function sets the ssid received from wpa_supplicant
 *		    to the CSR roam profile.
 *
 * @dev:	 Pointer to the net device.
 * @info:	 Pointer to the iw_request_info.
 * @wrqu:	 Pointer to the iwreq_data.
 * @extra:	 Pointer to the data.
 *
 * Return: 0 for success, error number on failure
 */
static int __iw_set_essid(struct net_device *dev,
		 struct iw_request_info *info,
		 union iwreq_data *wrqu, char *extra)
{
	uint32_t status = 0;
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	uint32_t roamId;
	tCsrRoamProfile *pRoamProfile;
	eCsrAuthType RSNAuthType;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (pAdapter->device_mode != QDF_STA_MODE &&
	    pAdapter->device_mode != QDF_IBSS_MODE &&
	    pAdapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_warn("device mode %s(%d) is not allowed",
			 hdd_device_mode_to_string(pAdapter->device_mode),
			 pAdapter->device_mode);
		return -EINVAL;
	}

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	if (pWextState->mTKIPCounterMeasures == TKIP_COUNTER_MEASURE_STARTED) {
		hdd_warn("Counter measure is in progress");
		return -EBUSY;
	}
	if (SIR_MAC_MAX_SSID_LENGTH < wrqu->essid.length)
		return -EINVAL;

	pRoamProfile = &pWextState->roamProfile;
	/*Try disconnecting if already in connected state*/
	status = wlan_hdd_try_disconnect(pAdapter);
	if (0 > status) {
	    hdd_err("Failed to disconnect the existing connection");
	    return -EALREADY;
	}

	/*
	 * when cfg80211 defined, wpa_supplicant wext driver uses
	 * zero-length, null-string ssid for force disconnection.
	 * after disconnection (if previously connected) and cleaning ssid,
	 * driver MUST return success.
	 */
	if (0 == wrqu->essid.length)
		return 0;

	status = hdd_wmm_get_uapsd_mask(pAdapter,
					&pWextState->roamProfile.uapsd_mask);
	if (QDF_STATUS_SUCCESS != status)
		pWextState->roamProfile.uapsd_mask = 0;

	pWextState->roamProfile.SSIDs.numOfSSIDs = 1;

	pWextState->roamProfile.SSIDs.SSIDList->SSID.length =
		wrqu->essid.length;

	qdf_mem_zero(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId,
		     sizeof(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId));
	qdf_mem_copy((void *)(pWextState->roamProfile.SSIDs.SSIDList->SSID.
			      ssId), extra, wrqu->essid.length);
	if (IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion
	    || IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion) {

		/* set gen ie */
		hdd_set_genie_to_csr(pAdapter, &RSNAuthType);

		/* set auth */
		hdd_set_csr_auth_type(pAdapter, RSNAuthType);
	}
#ifdef FEATURE_WLAN_WAPI
	hdd_debug("Setting WAPI AUTH Type and Encryption Mode values");
	if (pAdapter->wapi_info.nWapiMode) {
		switch (pAdapter->wapi_info.wapiAuthMode) {
		case WAPI_AUTH_MODE_PSK:
		{
			hdd_debug("WAPI AUTH TYPE: PSK: %d",
				   pAdapter->wapi_info.wapiAuthMode);
			pRoamProfile->AuthType.numEntries = 1;
			pRoamProfile->AuthType.authType[0] =
				eCSR_AUTH_TYPE_WAPI_WAI_PSK;
			break;
		}
		case WAPI_AUTH_MODE_CERT:
		{
			hdd_debug("WAPI AUTH TYPE: CERT: %d",
				   pAdapter->wapi_info.wapiAuthMode);
			pRoamProfile->AuthType.numEntries = 1;
			pRoamProfile->AuthType.authType[0] =
				eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
			break;
		}
		} /* End of switch */
		if (pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_PSK ||
		    pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_CERT) {
			hdd_debug("WAPI PAIRWISE/GROUP ENCRYPTION: WPI");
			pRoamProfile->EncryptionType.numEntries = 1;
			pRoamProfile->EncryptionType.encryptionType[0] =
				eCSR_ENCRYPT_TYPE_WPI;
			pRoamProfile->mcEncryptionType.numEntries = 1;
			pRoamProfile->mcEncryptionType.encryptionType[0] =
				eCSR_ENCRYPT_TYPE_WPI;
		}
	}
#endif /* FEATURE_WLAN_WAPI */
	/* if previous genIE is not NULL, update AssocIE */
	if (0 != pWextState->genIE.length) {
		memset(&pWextState->assocAddIE, 0,
		       sizeof(pWextState->assocAddIE));
		memcpy(pWextState->assocAddIE.addIEdata,
		       pWextState->genIE.addIEdata, pWextState->genIE.length);
		pWextState->assocAddIE.length = pWextState->genIE.length;
		pWextState->roamProfile.pAddIEAssoc =
			pWextState->assocAddIE.addIEdata;
		pWextState->roamProfile.nAddIEAssocLength =
			pWextState->assocAddIE.length;

		/* clear previous genIE after use it */
		memset(&pWextState->genIE, 0, sizeof(pWextState->genIE));
	}

	/*
	 * Assumes it is not WPS Association by default, except when
	 * pAddIEAssoc has WPS IE.
	 */
	pWextState->roamProfile.bWPSAssociation = false;

	if (NULL != wlan_hdd_get_wps_ie_ptr(pWextState->roamProfile.pAddIEAssoc,
					    pWextState->roamProfile.
					    nAddIEAssocLength))
		pWextState->roamProfile.bWPSAssociation = true;

	/* Disable auto BMPS entry by PMC until DHCP is done */
	sme_set_dhcp_till_power_active_flag(WLAN_HDD_GET_HAL_CTX(pAdapter),
						 true);

	pWextState->roamProfile.csrPersona = pAdapter->device_mode;

	if (eCSR_BSS_TYPE_START_IBSS == pRoamProfile->BSSType) {
		pRoamProfile->ch_params.ch_width = 0;
		hdd_select_cbmode(pAdapter,
			(WLAN_HDD_GET_CTX(pAdapter))->config->AdHocChannel5G,
			&pRoamProfile->ch_params);
	}

	/*
	 * Change conn_state to connecting before sme_roam_connect(),
	 * because sme_roam_connect() has a direct path to call
	 * hdd_sme_roam_callback(), which will change the conn_state
	 * If direct path, conn_state will be accordingly changed to
	 * NotConnected or Associated by either
	 * hdd_association_completion_handler() or hdd_dis_connect_handler()
	 * in sme_RoamCallback()if sme_RomConnect is to be queued,
	 * Connecting state will remain until it is completed.
	 *
	 * If connection state is not changed,
	 * connection state will remain in eConnectionState_NotConnected state.
	 * In hdd_association_completion_handler, "hddDisconInProgress" is
	 * set to true if conn state is eConnectionState_NotConnected.
	 * If "hddDisconInProgress" is set to true then cfg80211 layer is not
	 * informed of connect result indication which is an issue.
	 */
	if (QDF_STA_MODE == pAdapter->device_mode ||
			QDF_P2P_CLIENT_MODE == pAdapter->device_mode)
		hdd_conn_set_connection_state(pAdapter,
				eConnectionState_Connecting);

	status = sme_roam_connect(hHal, pAdapter->sessionId,
				  &(pWextState->roamProfile), &roamId);
	if ((QDF_STATUS_SUCCESS != status) &&
		(QDF_STA_MODE == pAdapter->device_mode ||
		 QDF_P2P_CLIENT_MODE == pAdapter->device_mode)) {
		hdd_err("sme_roam_connect failed session_id: %d status %d -> NotConnected",
			pAdapter->sessionId, status);
		/* change back to NotAssociated */
		hdd_conn_set_connection_state(pAdapter,
			eConnectionState_NotConnected);
	}
	pRoamProfile->ChannelInfo.ChannelList = NULL;
	pRoamProfile->ChannelInfo.numOfChannels = 0;

	EXIT();
	return status;
}

/**
 * iw_set_essid() - set essid handler function
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 for success, error number on failure
 */
int iw_set_essid(struct net_device *dev,
		 struct iw_request_info *info,
		 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_essid(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_essid() - This function returns the essid to the wpa_supplicant
 * @dev: pointer to the net device
 * @info: pointer to the iw request info
 * @dwrq: pointer to iw_point
 * @extra: pointer to the data
 *
 * Return: 0 on success, error number otherwise
 */
static int __iw_get_essid(struct net_device *dev,
		 struct iw_request_info *info,
		 struct iw_point *dwrq, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	hdd_wext_state_t *wextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if ((pHddStaCtx->conn_info.connState == eConnectionState_Associated &&
	     wextBuf->roamProfile.SSIDs.SSIDList->SSID.length > 0) ||
	    ((pHddStaCtx->conn_info.connState == eConnectionState_IbssConnected
	      || pHddStaCtx->conn_info.connState ==
	      eConnectionState_IbssDisconnected)
	     && wextBuf->roamProfile.SSIDs.SSIDList->SSID.length > 0)) {
		dwrq->length = pHddStaCtx->conn_info.SSID.SSID.length;
		memcpy(extra, pHddStaCtx->conn_info.SSID.SSID.ssId,
		       dwrq->length);
		dwrq->flags = 1;
	} else {
		memset(extra, 0, dwrq->length);
		dwrq->length = 0;
		dwrq->flags = 0;
	}
	EXIT();
	return 0;
}

/**
 * iw_get_essid() - get essid handler function
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 for success, error number on failure
 */
int iw_get_essid(struct net_device *dev,
		 struct iw_request_info *info,
		 struct iw_point *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_essid(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_auth() -
 *	This function sets the auth type received from the wpa_supplicant
 * @dev: pointer to the net device
 * @info: pointer to the iw request info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to the data
 *
 * Return: 0 on success, error number otherwise
 */
static int __iw_set_auth(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
	eCsrEncryptionType mcEncryptionType;
	eCsrEncryptionType ucEncryptionType;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (wrqu->param.flags & IW_AUTH_INDEX) {
	case IW_AUTH_WPA_VERSION:
		pWextState->wpaVersion = wrqu->param.value;
		break;

	case IW_AUTH_CIPHER_PAIRWISE:
	{
		if (wrqu->param.value & IW_AUTH_CIPHER_NONE) {
			ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_TKIP) {
			ucEncryptionType = eCSR_ENCRYPT_TYPE_TKIP;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_CCMP) {
			ucEncryptionType = eCSR_ENCRYPT_TYPE_AES;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_WEP40) {
			if ((IW_AUTH_KEY_MGMT_802_1X
			     ==
			     (pWextState->
			      authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X))
			    && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				pHddStaCtx->conn_info.authType))
				/*Dynamic WEP key */
				ucEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP40;
			else
				/*Static WEP key */
				ucEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_WEP104) {
			if ((IW_AUTH_KEY_MGMT_802_1X
			     ==
			     (pWextState->
			      authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X))
			    && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				pHddStaCtx->conn_info.authType))
				/*Dynamic WEP key */
				ucEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP104;
			else
				/*Static WEP key */
				ucEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
		} else {
			hdd_warn("value %d UNKNOWN IW_AUTH_CIPHER",
				 wrqu->param.value);
			return -EINVAL;
		}

		pRoamProfile->EncryptionType.numEntries = 1;
		pRoamProfile->EncryptionType.encryptionType[0] =
			ucEncryptionType;
	}
	break;
	case IW_AUTH_CIPHER_GROUP:
	{
		if (wrqu->param.value & IW_AUTH_CIPHER_NONE) {
			mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_TKIP) {
			mcEncryptionType = eCSR_ENCRYPT_TYPE_TKIP;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_CCMP) {
			mcEncryptionType = eCSR_ENCRYPT_TYPE_AES;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_WEP40) {
			if ((IW_AUTH_KEY_MGMT_802_1X
			     ==
			     (pWextState->
			      authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X))
			    && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				pHddStaCtx->conn_info.authType))
				mcEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP40;
			else
				mcEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
		} else if (wrqu->param.value & IW_AUTH_CIPHER_WEP104) {
			/* Dynamic WEP keys won't work with shared keys */
			if ((IW_AUTH_KEY_MGMT_802_1X
			     ==
			     (pWextState->
			      authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X))
			    && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				pHddStaCtx->conn_info.authType)) {
				mcEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP104;
			} else {
				mcEncryptionType =
					eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
			}
		} else {
			hdd_warn("value %d UNKNOWN IW_AUTH_CIPHER",
				wrqu->param.value);
			return -EINVAL;
		}

		pRoamProfile->mcEncryptionType.numEntries = 1;
		pRoamProfile->mcEncryptionType.encryptionType[0] =
			mcEncryptionType;
	}
	break;

	case IW_AUTH_80211_AUTH_ALG:
	{
		/* Save the auth algo here and set auth type to SME
		 * Roam profile in the iw_set_ap_address
		 */
		if (wrqu->param.value & IW_AUTH_ALG_OPEN_SYSTEM)
			pHddStaCtx->conn_info.authType =
				eCSR_AUTH_TYPE_OPEN_SYSTEM;

		else if (wrqu->param.value & IW_AUTH_ALG_SHARED_KEY)
			pHddStaCtx->conn_info.authType =
				eCSR_AUTH_TYPE_SHARED_KEY;

		else if (wrqu->param.value & IW_AUTH_ALG_LEAP)
			/*Not supported */
			pHddStaCtx->conn_info.authType =
				eCSR_AUTH_TYPE_OPEN_SYSTEM;
		pWextState->roamProfile.AuthType.authType[0] =
			pHddStaCtx->conn_info.authType;
	}
	break;

	case IW_AUTH_KEY_MGMT:
	{
#ifdef FEATURE_WLAN_ESE
#define IW_AUTH_KEY_MGMT_CCKM       8   /* Should be in linux/wireless.h */
		/*Check for CCKM AKM type */
		if (wrqu->param.value & IW_AUTH_KEY_MGMT_CCKM) {
			hdd_debug("CCKM AKM Set %d", wrqu->param.value);
			/* Set the CCKM bit in authKeyMgmt */
			/*
			 * Right now, this breaks all ref to authKeyMgmt because
			 * our code doesn't realize it is a "bitfield"
			 */
			pWextState->authKeyMgmt |=
				IW_AUTH_KEY_MGMT_CCKM;
			/* Set the key management to 802.1X */
			/* pWextState->authKeyMgmt = IW_AUTH_KEY_MGMT_802_1X; */
			pWextState->isESEConnection = true;
		} else if (wrqu->param.value & IW_AUTH_KEY_MGMT_PSK) {
			/* Save the key management */
			pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_PSK;
		} else if (!(wrqu->param.value & IW_AUTH_KEY_MGMT_802_1X)) {
			/* Save the key management anyway */
			pWextState->authKeyMgmt = wrqu->param.value;
		} else {                /* It must be IW_AUTH_KEY_MGMT_802_1X */
			/* Save the key management */
			pWextState->authKeyMgmt |=
				IW_AUTH_KEY_MGMT_802_1X;
		}
#else
		/* Save the key management */
		pWextState->authKeyMgmt = wrqu->param.value;
#endif /* FEATURE_WLAN_ESE */
	}
	break;

	case IW_AUTH_TKIP_COUNTERMEASURES:
	{
		if (wrqu->param.value) {
			hdd_debug("Counter Measure started %d",
			       wrqu->param.value);
			pWextState->mTKIPCounterMeasures =
				TKIP_COUNTER_MEASURE_STARTED;
		} else {
			hdd_debug("Counter Measure stopped=%d",
			       wrqu->param.value);
			pWextState->mTKIPCounterMeasures =
				TKIP_COUNTER_MEASURE_STOPED;
		}
	}
	break;
	case IW_AUTH_DROP_UNENCRYPTED:
	case IW_AUTH_WPA_ENABLED:
	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
	case IW_AUTH_ROAMING_CONTROL:
	case IW_AUTH_PRIVACY_INVOKED:

	default:

		hdd_warn("called with unsupported auth type %d",
			 wrqu->param.flags & IW_AUTH_INDEX);
		break;
	}

	EXIT();
	return 0;
}

/**
 * iw_set_auth() - set auth callback function
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 for success, error number on failure.
 */
int iw_set_auth(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_auth(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
/**
 * __iw_get_auth() -
 *	This function returns the auth type to the wpa_supplicant
 * @dev: pointer to the net device
 * @info: pointer to the iw request info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to the data
 *
 * Return: 0 on success, error number otherwise
 */
static int __iw_get_auth(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	switch (pRoamProfile->negotiatedAuthType) {
	case eCSR_AUTH_TYPE_WPA_NONE:
		wrqu->param.flags = IW_AUTH_WPA_VERSION;
		wrqu->param.value = IW_AUTH_WPA_VERSION_DISABLED;
		break;
	case eCSR_AUTH_TYPE_WPA:
		wrqu->param.flags = IW_AUTH_WPA_VERSION;
		wrqu->param.value = IW_AUTH_WPA_VERSION_WPA;
		break;

	case eCSR_AUTH_TYPE_FT_RSN:
	case eCSR_AUTH_TYPE_RSN:
		wrqu->param.flags = IW_AUTH_WPA_VERSION;
		wrqu->param.value = IW_AUTH_WPA_VERSION_WPA2;
		break;
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		wrqu->param.value = IW_AUTH_ALG_SHARED_KEY;
		break;
	case eCSR_AUTH_TYPE_UNKNOWN:
		hdd_debug("called with unknown auth type");
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_WPA_PSK:
		hdd_debug("called with WPA PSK auth type");
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		return -EIO;

	case eCSR_AUTH_TYPE_FT_RSN_PSK:
	case eCSR_AUTH_TYPE_RSN_PSK:
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
#endif
		hdd_debug("called with RSN PSK auth type");
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		return -EIO;
	default:
		hdd_warn("Unknown auth type: %d",
			pRoamProfile->negotiatedAuthType);
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		return -EIO;
	}
	if (((wrqu->param.flags & IW_AUTH_INDEX) == IW_AUTH_CIPHER_PAIRWISE)) {
		switch (pRoamProfile->negotiatedUCEncryptionType) {
		case eCSR_ENCRYPT_TYPE_NONE:
			wrqu->param.value = IW_AUTH_CIPHER_NONE;
			break;
		case eCSR_ENCRYPT_TYPE_WEP40:
		case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
			wrqu->param.value = IW_AUTH_CIPHER_WEP40;
			break;
		case eCSR_ENCRYPT_TYPE_TKIP:
			wrqu->param.value = IW_AUTH_CIPHER_TKIP;
			break;
		case eCSR_ENCRYPT_TYPE_WEP104:
		case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
			wrqu->param.value = IW_AUTH_CIPHER_WEP104;
			break;
		case eCSR_ENCRYPT_TYPE_AES:
			wrqu->param.value = IW_AUTH_CIPHER_CCMP;
			break;
		default:
			hdd_warn("called with unknown auth type %d",
				   pRoamProfile->negotiatedUCEncryptionType);
			return -EIO;
		}
	}

	if (((wrqu->param.flags & IW_AUTH_INDEX) == IW_AUTH_CIPHER_GROUP)) {
		switch (pRoamProfile->negotiatedMCEncryptionType) {
		case eCSR_ENCRYPT_TYPE_NONE:
			wrqu->param.value = IW_AUTH_CIPHER_NONE;
			break;
		case eCSR_ENCRYPT_TYPE_WEP40:
		case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
			wrqu->param.value = IW_AUTH_CIPHER_WEP40;
			break;
		case eCSR_ENCRYPT_TYPE_TKIP:
			wrqu->param.value = IW_AUTH_CIPHER_TKIP;
			break;
		case eCSR_ENCRYPT_TYPE_WEP104:
		case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
			wrqu->param.value = IW_AUTH_CIPHER_WEP104;
			break;
		case eCSR_ENCRYPT_TYPE_AES:
			wrqu->param.value = IW_AUTH_CIPHER_CCMP;
			break;
		default:
			hdd_err("called with unknown auth type %d",
				 pRoamProfile->negotiatedMCEncryptionType);
			return -EIO;
		}
	}

	hdd_debug("called with auth type %d",
		 pRoamProfile->AuthType.authType[0]);
	EXIT();
	return 0;
}

/**
 * iw_get_auth() - get auth callback function
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 for success, error number on failure.
 */
int iw_get_auth(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_auth(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_ap_address() - set ap address
 * @dev: pointer to the net device
 * @info: pointer to the iw request info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to the data
 *
 * This function updates the HDD global station context connection info
 * BSSID with the MAC address received from the wpa_supplicant.
 *
 * Return: 0 on success, error number otherwise
 */
static int __iw_set_ap_address(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{

	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	hdd_station_ctx_t *pHddStaCtx =
		WLAN_HDD_GET_STATION_CTX_PTR(WLAN_HDD_GET_PRIV_PTR(dev));
	uint8_t *pMacAddress = NULL;
	int ret;

	ENTER_DEV(dev);

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	pMacAddress = (uint8_t *) wrqu->ap_addr.sa_data;
	hdd_debug(" " MAC_ADDRESS_STR, MAC_ADDR_ARRAY(pMacAddress));
	qdf_mem_copy(pHddStaCtx->conn_info.bssId.bytes, pMacAddress,
		     sizeof(struct qdf_mac_addr));
	EXIT();

	return 0;
}

/**
 * iw_set_ap_address() - set ap addresses callback function
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 for success, error number on failure.
 */
int iw_set_ap_address(struct net_device *dev, struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_ap_address(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_ap_address() - get ap address
 * @dev: pointer to the net device
 * @info: pointer to the iw request info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to the data
 *
 * This function returns currently associated BSSID.
 *
 * Return: 0 on success, error number otherwise
 */
static int __iw_get_ap_address(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	hdd_station_ctx_t *pHddStaCtx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	if (pHddStaCtx->conn_info.connState == eConnectionState_Associated ||
	    eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState) {
		qdf_mem_copy(wrqu->ap_addr.sa_data,
				pHddStaCtx->conn_info.bssId.bytes,
				QDF_MAC_ADDR_SIZE);
	} else {
		memset(wrqu->ap_addr.sa_data, 0, sizeof(wrqu->ap_addr.sa_data));
	}
	EXIT();
	return 0;
}

/**
 * iw_get_ap_address() - get ap addresses callback function
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 for success, error number on failure.
 */
int iw_get_ap_address(struct net_device *dev, struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_ap_address(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
