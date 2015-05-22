/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
#include <linux/ieee80211.h>
#include <linux/wireless.h>
#include <linux/etherdevice.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#include "csr_inside_api.h"
#include "wlan_hdd_p2p.h"
#ifdef FEATURE_WLAN_TDLS
#include "wlan_hdd_tdls.h"
#endif
#include "sme_api.h"
#include "wlan_hdd_hostapd.h"
#include <wlan_hdd_ipa.h>
#include <cds_sched.h>
#include "cds_concurrency.h"
#include "sme_power_save_api.h"
#include "ol_txrx_ctrl_api.h"
#include "ol_txrx_types.h"

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

#if defined(WLAN_FEATURE_VOWIFI_11R)
/* Offset where the EID-Len-IE, start. */
#define FT_ASSOC_RSP_IES_OFFSET 6  /* Capability(2) + AID(2) + Status Code(2) */
#define FT_ASSOC_REQ_IES_OFFSET 4  /* Capability(2) + LI(2) */
#endif

#define BEACON_FRAME_IES_OFFSET 12
#define HDD_PEER_AUTHORIZE_WAIT 10

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
	hddLog(LOG1,
	       FL("Authenticated state Changed from oldState:%d to State:%d"),
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
	hddLog(LOG1, FL("ConnectionState Changed from oldState:%d to State:%d"),
	       pHddStaCtx->conn_info.connState, connState);
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
eCsrBand hdd_conn_get_connected_band(hdd_station_ctx_t *pHddStaCtx)
{
	uint8_t staChannel = 0;

	if (eConnectionState_Associated == pHddStaCtx->conn_info.connState)
		staChannel = pHddStaCtx->conn_info.operationChannel;

	if (staChannel > 0 && staChannel < 14)
		return eCSR_BAND_24;
	else if (staChannel >= 36 && staChannel <= 184)
		return eCSR_BAND_5G;
	else   /* If station is not connected return as eCSR_BAND_ALL */
		return eCSR_BAND_ALL;
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
 * hdd_conn_get_connected_bss_type() - get current bss type
 * @pHddStaCtx: pointer to global HDD Station context
 * @pConnectedBssType: pointer to connected bss type
 *
 * Return: false if any errors encountered, true otherwise
 */
inline bool
hdd_conn_get_connected_bss_type(hdd_station_ctx_t *pHddStaCtx,
				eMib_dot11DesiredBssType *pConnectedBssType)
{
	bool fConnected = false;

	fConnected = hdd_conn_get_connection_state(pHddStaCtx, NULL);

	if (pConnectedBssType) {
		*pConnectedBssType =
			pHddStaCtx->conn_info.connDot11DesiredBssType;
	}

	return fConnected;
}

/**
 * hdd_conn_save_connected_bss_type() - set connected bss type
 * @pHddStaCtx: pointer to global HDD Station context
 * @csr_roamBssType: bss type
 *
 * Return: none
 */
static inline void
hdd_conn_save_connected_bss_type(hdd_station_ctx_t *pHddStaCtx,
				 eCsrRoamBssType csr_roamBssType)
{
	switch (csr_roamBssType) {
	case eCSR_BSS_TYPE_INFRASTRUCTURE:
		pHddStaCtx->conn_info.connDot11DesiredBssType =
			eMib_dot11DesiredBssType_infrastructure;
		break;

	case eCSR_BSS_TYPE_IBSS:
	case eCSR_BSS_TYPE_START_IBSS:
		pHddStaCtx->conn_info.connDot11DesiredBssType =
			eMib_dot11DesiredBssType_independent;
		break;

	/** We will never set the BssType to 'any' when attempting a connection
	      so CSR should never send this back to us.*/
	case eCSR_BSS_TYPE_ANY:
	default:
		CDF_ASSERT(0);
		break;
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

	CDF_ASSERT(pRoamInfo);

	if (pRoamInfo) {
		/* Save the BSSID for the connection */
		if (eCSR_BSS_TYPE_INFRASTRUCTURE == eBssType) {
			CDF_ASSERT(pRoamInfo->pBssDesc);
			cdf_copy_macaddr(&pHddStaCtx->conn_info.bssId,
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
			cdf_copy_macaddr(&pHddStaCtx->conn_info.bssId,
					 &pRoamInfo->bssid);
		} else {
			/*
			 * can't happen. We need a valid IBSS or Infra setting
			 * in the BSSDescription or we can't function.
			 */
			CDF_ASSERT(0);
		}

		/* notify WMM */
		hdd_wmm_connect(pAdapter, pRoamInfo, eBssType);

		if (!pRoamInfo->u.pConnectedProfile) {
			CDF_ASSERT(pRoamInfo->u.pConnectedProfile);
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
			cdf_mem_copy(&pHddStaCtx->conn_info.SSID.SSID,
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
	}
	/* save the connected BssType */
	hdd_conn_save_connected_bss_type(pHddStaCtx, eBssType);
}

#if defined(WLAN_FEATURE_VOWIFI_11R)
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
		hddLog(LOGE,
		       FL("pCsrRoamInfo->nAssocRspLength=%d"),
		       (int)pCsrRoamInfo->nAssocRspLength);
		return;
	}

	pFTAssocRsp =
		(u8 *) (pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength +
			pCsrRoamInfo->nAssocReqLength);
	if (pFTAssocRsp == NULL) {
		hddLog(LOGE, FL("AssocReq or AssocRsp is NULL"));
		return;
	}
	/* pFTAssocRsp needs to point to the IEs */
	pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
	hddLog(LOG1, FL("AssocRsp is now at %02x%02x"),
	       (unsigned int)pFTAssocRsp[0], (unsigned int)pFTAssocRsp[1]);

	/* We need to send the IEs to the supplicant. */
	buff = kmalloc(IW_GENERIC_IE_MAX, GFP_ATOMIC);
	if (buff == NULL) {
		hddLog(LOGE, FL("kmalloc unable to allocate memory"));
		return;
	}
	/* Send the Assoc Resp, the supplicant needs this for initial Auth. */
	len = pCsrRoamInfo->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
	wrqu.data.length = len;
	memset(buff, 0, IW_GENERIC_IE_MAX);
	memcpy(buff, pFTAssocRsp, len);
	wireless_send_event(dev, IWEVASSOCRESPIE, &wrqu, buff);

	kfree(buff);
}
#endif /* WLAN_FEATURE_VOWIFI_11R */

#ifdef WLAN_FEATURE_VOWIFI_11R
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
	cdf_mem_zero(ftIe, DOT11F_IE_FTINFO_MAX_LEN);
	cdf_mem_zero(ricIe, DOT11F_IE_RICDESCRIPTOR_MAX_LEN);

	sme_get_rici_es(pHddCtx->hHal, pAdapter->sessionId, (u8 *) ricIe,
			DOT11F_IE_RICDESCRIPTOR_MAX_LEN, &ric_ies_length);
	if (ric_ies_length == 0) {
		hddLog(LOGW,
		       FL("RIC IEs is of length 0 not sending RIC Information for now"));
	}

	ftEvent.ric_ies = ricIe;
	ftEvent.ric_ies_len = ric_ies_length;
	hddLog(LOG1, FL("RIC IEs is of length %d"), (int)ric_ies_length);

	sme_get_ft_pre_auth_response(pHddCtx->hHal, pAdapter->sessionId,
				     (u8 *) ftIe, DOT11F_IE_FTINFO_MAX_LEN,
				     &auth_resp_len);

	if (auth_resp_len == 0) {
		hddLog(LOGE, FL("AuthRsp FTIES is of length 0"));
		return;
	}

	sme_set_ft_pre_auth_state(pHddCtx->hHal, pAdapter->sessionId, true);

	ftEvent.target_ap = ftIe;

	ftEvent.ies = (u8 *) (ftIe + CDF_MAC_ADDR_SIZE);
	ftEvent.ies_len = auth_resp_len - CDF_MAC_ADDR_SIZE;

	hddLog(LOG1, FL("ftEvent.ies_len %zu"), ftEvent.ies_len);
	hddLog(LOG1, FL("ftEvent.ric_ies_len %zu"), ftEvent.ric_ies_len);
	hddLog(LOG1, FL("ftEvent.target_ap %2x-%2x-%2x-%2x-%2x-%2x"),
	       ftEvent.target_ap[0], ftEvent.target_ap[1],
	       ftEvent.target_ap[2], ftEvent.target_ap[3], ftEvent.target_ap[4],
	       ftEvent.target_ap[5]);

	(void)cfg80211_ft_event(dev, &ftEvent);

#else
	/* We need to send the IEs to the supplicant */
	buff = kmalloc(IW_CUSTOM_MAX, GFP_ATOMIC);
	if (buff == NULL) {
		hddLog(LOGE, FL("kmalloc unable to allocate memory"));
		return;
	}
	cdf_mem_zero(buff, IW_CUSTOM_MAX);

	/* Sme needs to send the RIC IEs first */
	str_len = strlcpy(buff, "RIC=", IW_CUSTOM_MAX);
	sme_get_rici_es(pHddCtx->hHal, pAdapter->sessionId,
			(u8 *) &(buff[str_len]), (IW_CUSTOM_MAX - str_len),
			&ric_ies_length);
	if (ric_ies_length == 0) {
		hddLog(LOGW,
		       FL("RIC IEs is of length 0 not sending RIC Information for now"));
	} else {
		wrqu.data.length = str_len + ric_ies_length;
		wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);
	}

	/* Sme needs to provide the Auth Resp */
	cdf_mem_zero(buff, IW_CUSTOM_MAX);
	str_len = strlcpy(buff, "AUTH=", IW_CUSTOM_MAX);
	sme_get_ft_pre_auth_response(pHddCtx->hHal, pAdapter->sessionId,
				     (u8 *) &buff[str_len],
				     (IW_CUSTOM_MAX - str_len), &auth_resp_len);

	if (auth_resp_len == 0) {
		kfree(buff);
		hddLog(LOGE, FL("AuthRsp FTIES is of length 0"));
		return;
	}

	wrqu.data.length = str_len + auth_resp_len;
	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);

	kfree(buff);
#endif
}

#endif /* WLAN_FEATURE_VOWIFI_11R */

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
		hddLog(LOGE, FL("pCsrRoamInfo->pBssDesc(%p)"), descriptor);
		return;
	}
	/*
	 * Send the Channel event, the supplicant needs this to generate
	 * the Adjacent AP report.
	 */
	hddLog(LOGW, FL("Sending up an SIOCGIWFREQ, channelId(%d)"),
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
		hddLog(LOGW, FL("pCsrRoamInfo->nBeaconFrameLength = 0"));
		return;
	}
	pBeaconIes = (u8 *) (pCsrRoamInfo->pbFrames + BEACON_FRAME_IES_OFFSET);
	if (pBeaconIes == NULL) {
		hddLog(LOGW, FL("Beacon IEs is NULL"));
		return;
	}
	/* pBeaconIes needs to point to the IEs */
	hddLog(LOG1, FL("Beacon IEs is now at %02x%02x"),
	       (unsigned int)pBeaconIes[0], (unsigned int)pBeaconIes[1]);
	hddLog(LOG1, FL("Beacon IEs length = %d"),
	       pCsrRoamInfo->nBeaconLength - BEACON_FRAME_IES_OFFSET);

	/* We need to send the IEs to the supplicant. */
	buff = kmalloc(IW_CUSTOM_MAX, GFP_ATOMIC);
	if (buff == NULL) {
		hddLog(LOGE, FL("kmalloc unable to allocate memory"));
		return;
	}
	cdf_mem_zero(buff, IW_CUSTOM_MAX);

	strLen = strlcpy(buff, "BEACONIEs=", IW_CUSTOM_MAX);
	currentLen = strLen + 1;

	totalIeLen = pCsrRoamInfo->nBeaconLength - BEACON_FRAME_IES_OFFSET;
	do {
		/*
		 * If the beacon size exceeds max CUSTOM event size, break it
		 * into chunks of CUSTOM event max size and send it to
		 * supplicant. Changes are done in supplicant to handle this.
		 */
		cdf_mem_zero(&buff[strLen + 1], IW_CUSTOM_MAX - (strLen + 1));
		currentLen =
			CDF_MIN(totalIeLen, IW_CUSTOM_MAX - (strLen + 1) - 1);
		cdf_mem_copy(&buff[strLen + 1], pBeaconIes + currentOffset,
			     currentLen);
		currentOffset += currentLen;
		totalIeLen -= currentLen;
		wrqu.data.length = strLen + 1 + currentLen;
		if (totalIeLen)
			buff[strLen] = 1; /* more chunks pending */
		else
			buff[strLen] = 0; /* last chunk */

		hddLog(LOG1, FL("Beacon IEs length to supplicant = %d"),
		       currentLen);
		wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);
	} while (totalIeLen > 0);

	kfree(buff);
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	union iwreq_data wrqu;
	int we_event;
	char *msg;
	struct cdf_mac_addr peerMacAddr;

#ifdef WLAN_FEATURE_VOWIFI_11R
	/* Added to find the auth type on the fly at run time */
	/* rather than with cfg to see if FT is enabled */
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
#endif

	memset(&wrqu, '\0', sizeof(wrqu));
	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	we_event = SIOCGIWAP;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (NULL != pCsrRoamInfo)
		if (pCsrRoamInfo->roamSynchInProgress)
			/* change logging before release */
			hddLog(LOG4, "LFR3:hdd_send_association_event");
#endif
	if (eConnectionState_Associated == pHddStaCtx->conn_info.connState) {
		if (!pCsrRoamInfo) {
			hddLog(LOGE, FL("STA in associated state but pCsrRoamInfo is null"));
			return;
		}

		cds_incr_active_session(pHddCtx, pAdapter->device_mode,
						pAdapter->sessionId);
		memcpy(wrqu.ap_addr.sa_data, pCsrRoamInfo->pBssDesc->bssId,
		       sizeof(pCsrRoamInfo->pBssDesc->bssId));

#ifdef WLAN_FEATURE_P2P_DEBUG
		if (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) {
			if (global_p2p_connection_status ==
			    P2P_CLIENT_CONNECTING_STATE_1) {
				global_p2p_connection_status =
					P2P_CLIENT_CONNECTED_STATE_1;
				hddLog(LOGE,
				       "[P2P State] Changing state from Connecting state to Connected State for 8-way Handshake");
			} else if (global_p2p_connection_status ==
				   P2P_CLIENT_CONNECTING_STATE_2) {
				global_p2p_connection_status =
					P2P_CLIENT_COMPLETED_STATE;
				hddLog(LOGE,
				       "[P2P State] Changing state from Connecting state to P2P Client Connection Completed");
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
		 * WLAN_FEATURE_VOWIFI_11R is Enabled and fFTEnable is true.
		 */
#ifdef WLAN_FEATURE_VOWIFI_11R
		/* Send FT Keys to the supplicant when FT is enabled */
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
#endif
		if (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) {
			tSirSmeChanInfo chan_info;
			cdf_copy_macaddr(&peerMacAddr,
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

			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_oem_app(&peerMacAddr,
							    ePeerConnected,
							    pCsrRoamInfo->
							    timingMeasCap,
							    pAdapter->sessionId,
							    &chan_info);
		}
#ifdef MSM_PLATFORM
#ifdef CONFIG_CNSS
		/* start timer in sta/p2p_cli */
		spin_lock_bh(&pHddCtx->bus_bw_lock);
		pAdapter->prev_tx_packets = pAdapter->stats.tx_packets;
		pAdapter->prev_rx_packets = pAdapter->stats.rx_packets;
		spin_unlock_bh(&pHddCtx->bus_bw_lock);
		hdd_start_bus_bw_compute_timer(pAdapter);
#endif
#endif
	} else if (eConnectionState_IbssConnected ==    /* IBss Associated */
			pHddStaCtx->conn_info.connState) {
		cds_incr_active_session(pHddCtx, pAdapter->device_mode,
						pAdapter->sessionId);
		memcpy(wrqu.ap_addr.sa_data, pHddStaCtx->conn_info.bssId.bytes,
		       ETH_ALEN);
		pr_info("wlan: new IBSS connection to " MAC_ADDRESS_STR "\n",
			MAC_ADDR_ARRAY(pHddStaCtx->conn_info.bssId.bytes));
	} else {                /* Not Associated */

		pr_info("wlan: disconnected\n");
		memset(wrqu.ap_addr.sa_data, '\0', ETH_ALEN);
		cds_decr_session_set_pcl(
						pHddCtx, pAdapter->device_mode,
						pAdapter->sessionId);
#if defined(FEATURE_WLAN_LFR)
		wlan_hdd_enable_roaming(pAdapter);
#endif

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif

		if (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) {
			cdf_copy_macaddr(&peerMacAddr,
					 &pHddStaCtx->conn_info.bssId);

			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_oem_app(&peerMacAddr,
							   ePeerDisconnected, 0,
							   pAdapter->sessionId,
							   NULL);
		}
#ifdef WLAN_FEATURE_LPSS
		pAdapter->rssi_send = false;
		wlan_hdd_send_status_pkg(pAdapter, pHddStaCtx, 1, 0);
#endif
#ifdef FEATURE_WLAN_TDLS
		if ((pAdapter->device_mode == WLAN_HDD_INFRA_STATION) &&
		    (pCsrRoamInfo)) {
			hddLog(LOG4,
				FL("tdls_prohibited: %d, tdls_chan_swit_prohibited: %d"),
				pCsrRoamInfo->tdls_prohibited,
				pCsrRoamInfo->tdls_chan_swit_prohibited);

			wlan_hdd_update_tdls_info(pAdapter,
				pCsrRoamInfo->tdls_prohibited,
				pCsrRoamInfo->tdls_chan_swit_prohibited);
		}
#endif
#ifdef MSM_PLATFORM
		/* stop timer in sta/p2p_cli */
		spin_lock_bh(&pHddCtx->bus_bw_lock);
		pAdapter->prev_tx_packets = 0;
		pAdapter->prev_rx_packets = 0;
		spin_unlock_bh(&pHddCtx->bus_bw_lock);
		hdd_stop_bus_bw_compute_timer(pAdapter);
#endif
	}
	cds_dump_concurrency_info(pHddCtx);
	/* Send SCC/MCC Switching event to IPA */
	hdd_ipa_send_mcc_scc_msg(pHddCtx, pHddCtx->mcc_mode);

	msg = NULL;
	/*During the WLAN uninitialization,supplicant is stopped before the
	   driver so not sending the status of the connection to supplicant */
	if ((pHddCtx->isLoadInProgress != true) &&
	    (pHddCtx->isUnloadInProgress != true)) {
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
	cdf_mem_zero(&pHddStaCtx->conn_info.bssId, CDF_MAC_ADDR_SIZE);
	cdf_mem_zero(&pHddStaCtx->conn_info.peerMacAddress[0],
		     CDF_MAC_ADDR_SIZE);

	/* Clear all security settings */
	pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	pHddStaCtx->conn_info.mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
	pHddStaCtx->conn_info.ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;

	cdf_mem_zero(&pHddStaCtx->conn_info.Keys, sizeof(tCsrKeys));
	cdf_mem_zero(&pHddStaCtx->ibss_enc_key, sizeof(tCsrRoamSetKey));

	/* Set not-connected state */
	pHddStaCtx->conn_info.connDot11DesiredBssType = eCSR_BSS_TYPE_ANY;
	pHddStaCtx->conn_info.proxyARPService = 0;

	cdf_mem_zero(&pHddStaCtx->conn_info.SSID, sizeof(tCsrSSIDInfo));
}

/**
 * hdd_roam_deregister_sta() - deregister station
 * @pAdapter: pointer to adapter
 * @staId: station identifier
 *
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS
hdd_roam_deregister_sta(hdd_adapter_t *pAdapter, uint8_t staId)
{
	CDF_STATUS cdf_status;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	if (eConnectionState_IbssDisconnected ==
	    pHddStaCtx->conn_info.connState) {
		/*
		 * Do not set the carrier off when the last peer leaves.
		 * We will set the carrier off while stopping the IBSS.
		 */
	}

	cdf_status = ol_txrx_clear_peer(staId);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		hddLog(LOGE,
			FL("ol_txrx_clear_peer() failed for staID %d. Status(%d) [0x%08X]"),
			staId, cdf_status, cdf_status);
	}
	return cdf_status;
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
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS hdd_dis_connect_handler(hdd_adapter_t *pAdapter,
					  tCsrRoamInfo *pRoamInfo,
					  uint32_t roamId,
					  eRoamCmdStatus roamStatus,
					  eCsrRoamResult roamResult)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	CDF_STATUS vstatus;
	struct net_device *dev = pAdapter->dev;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	uint8_t sta_id;
	bool sendDisconInd = true;

	if (dev == NULL) {
		hddLog(LOGE, FL("net_dev is released return"));
		return CDF_STATUS_E_FAILURE;
	}
	/* notify apps that we can't pass traffic anymore */
	hddLog(LOG1, FL("Disabling queues"));
	wlan_hdd_netif_queue_control(pAdapter, WLAN_NETIF_TX_DISABLE_N_CARRIER,
				   WLAN_CONTROL_PATH);

	if (hdd_ipa_is_enabled(pHddCtx))
		hdd_ipa_wlan_evt(pAdapter, pHddStaCtx->conn_info.staId[0],
				 WLAN_STA_DISCONNECT,
				 pHddStaCtx->conn_info.bssId.bytes);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	wlan_hdd_auto_shutdown_enable(pHddCtx, true);
#endif

#ifdef QCA_PKT_PROTO_TRACE
	/* STA disconnected, update into trace buffer */
	if (pHddCtx->config->gEnableDebugLog)
		cds_pkt_trace_buf_update("ST:DISASC");
#endif /* QCA_PKT_PROTO_TRACE */

	/* HDD has initiated disconnect, do not send disconnect indication
	 * to kernel. Sending disconnected event to kernel for userspace
	 * initiated disconnect will be handled by hdd_DisConnectHandler call
	 * to cfg80211_disconnected.
	 */
	if ((eConnectionState_Disconnecting ==
	    pHddStaCtx->conn_info.connState) ||
	    (eConnectionState_NotConnected ==
	    pHddStaCtx->conn_info.connState)) {
		hddLog(LOG1,
			FL("HDD has initiated a disconnect, no need to send disconnect indication to kernel"));
		sendDisconInd = false;
	}

	if (pHddStaCtx->conn_info.connState != eConnectionState_Disconnecting) {
		INIT_COMPLETION(pAdapter->disconnect_comp_var);
		hddLog(LOG1,
			FL("Set HDD connState to eConnectionState_Disconnecting"));
		hdd_conn_set_connection_state(pAdapter,
					      eConnectionState_Disconnecting);
	}

	hdd_clear_roam_profile_ie(pAdapter);
	hdd_wmm_init(pAdapter);

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
			if (eCSR_ROAM_LOSTLINK == roamStatus)
				cfg80211_disconnected(dev, pRoamInfo->
						      reasonCode, NULL,
						      0, GFP_KERNEL);
			else
				cfg80211_disconnected(dev,
					      WLAN_REASON_UNSPECIFIED,
					      NULL, 0,
					      GFP_KERNEL);

			hdd_info("sent disconnected event to nl80211, rssi: %d",
				pAdapter->rssi);
		}
		/*
		 * During the WLAN uninitialization,supplicant is stopped
		 * before the driver so not sending the status of the
		 * connection to supplicant.
		 */
		if ((pHddCtx->isLoadInProgress != true) &&
		    (pHddCtx->isUnloadInProgress != true)) {
#ifdef WLAN_FEATURE_P2P_DEBUG
			if (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) {
				if (global_p2p_connection_status ==
				    P2P_CLIENT_CONNECTED_STATE_1) {
					global_p2p_connection_status =
						P2P_CLIENT_DISCONNECTED_STATE;
					hddLog(LOGE,
					       "[P2P State] 8 way Handshake completed and moved to disconnected state");
				} else if (global_p2p_connection_status ==
					   P2P_CLIENT_COMPLETED_STATE) {
					global_p2p_connection_status =
						P2P_NOT_ACTIVE;
					hddLog(LOGE,
					       "[P2P State] P2P Client is removed and moved to inactive state");
				}
			}
#endif

		}
	}

	hdd_wmm_adapter_clear(pAdapter);
#if defined(WLAN_FEATURE_VOWIFI_11R)
	sme_ft_reset(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId);
#endif
	if (eCSR_ROAM_IBSS_LEAVE == roamStatus) {
		uint8_t i;
		sta_id = IBSS_BROADCAST_STAID;
		vstatus = hdd_roam_deregister_sta(pAdapter, sta_id);
		if (!CDF_IS_STATUS_SUCCESS(vstatus)) {
			hddLog(LOGE,
				FL("hdd_roam_deregister_sta() failed for staID %d Status=%d [0x%x]"),
				sta_id, status, status);
			status = CDF_STATUS_E_FAILURE;
		}
		pHddCtx->sta_to_adapter[sta_id] = NULL;
		/* Clear all the peer sta register with TL. */
		for (i = 0; i < MAX_IBSS_PEERS; i++) {
			if (0 == pHddStaCtx->conn_info.staId[i])
				continue;
			sta_id = pHddStaCtx->conn_info.staId[i];
			hddLog(LOG1, FL("Deregister StaID %d"), sta_id);
			vstatus = hdd_roam_deregister_sta(pAdapter, sta_id);
			if (!CDF_IS_STATUS_SUCCESS(vstatus)) {
				hddLog(LOGE,
					FL("hdd_roamDeregisterSTA() failed to for staID %d. Status= %d [0x%x]"),
					sta_id, status, status);
				status = CDF_STATUS_E_FAILURE;
			}
			/* set the staid and peer mac as 0, all other
			 * reset are done in hdd_connRemoveConnectInfo.
			 */
			pHddStaCtx->conn_info.staId[i] = 0;
			cdf_mem_zero(&pHddStaCtx->conn_info.peerMacAddress[i],
				sizeof(struct cdf_mac_addr));
			if (sta_id < (WLAN_MAX_STA_COUNT + 3))
				pHddCtx->sta_to_adapter[sta_id] = NULL;
		}
	} else {
		sta_id = pHddStaCtx->conn_info.staId[0];
		/* We should clear all sta register with TL,
		 * for now, only one.
		 */
		vstatus = hdd_roam_deregister_sta(pAdapter, sta_id);
		if (!CDF_IS_STATUS_SUCCESS(vstatus)) {
			hddLog(LOGE,
				FL("hdd_roam_deregister_sta() failed to for staID %d. Status= %d [0x%x]"),
				sta_id, status, status);
			status = CDF_STATUS_E_FAILURE;
		}
		pHddCtx->sta_to_adapter[sta_id] = NULL;
	}
	/* Clear saved connection information in HDD */
	hdd_conn_remove_connect_info(pHddStaCtx);
	hddLog(LOG1, FL("Set HDD connState to eConnectionState_NotConnected"));
	hdd_conn_set_connection_state(pAdapter, eConnectionState_NotConnected);
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	if ((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
	    (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode)) {
		memset(&pHddStaCtx->gtkOffloadReqParams, 0,
		       sizeof(tSirGtkOffloadParams));
		pHddStaCtx->gtkOffloadReqParams.ulFlags = GTK_OFFLOAD_DISABLE;
	}
#endif

#ifdef FEATURE_WLAN_TDLS
	if (eCSR_ROAM_IBSS_LEAVE != roamStatus)
		wlan_hdd_tdls_disconnection_callback(pAdapter);
#endif

	if ((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
			(WLAN_HDD_P2P_CLIENT == pAdapter->device_mode)) {
		sme_ps_disable_auto_ps_timer(WLAN_HDD_GET_HAL_CTX
				(pAdapter),
				pAdapter->sessionId);
	}
	/* Unblock anyone waiting for disconnect to complete */
	complete(&pAdapter->disconnect_comp_var);
	return status;
}

/**
 * hdd_set_peer_authorized_event() - set peer_authorized_event
 * @vdev_id: vdevid
 *
 * Return: None
 */
void hdd_set_peer_authorized_event(uint32_t vdev_id)
{
	hdd_context_t *hdd_ctx = cds_get_context(CDF_MODULE_ID_HDD);
	hdd_adapter_t *adapter = NULL;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (adapter == NULL) {
		hddLog(LOGE,
			"%s: Invalid vdev_id", __func__);
	}
	complete(&adapter->sta_authorized_event);
}

/**
 * hdd_change_peer_state() - change peer state
 * @pAdapter: HDD adapter
 * @sta_state: peer state
 * @roam_synch_in_progress: roam synch in progress
 *
 * Return: CDF status
 */
CDF_STATUS hdd_change_peer_state(hdd_adapter_t *pAdapter,
				 uint8_t sta_id,
				 enum ol_txrx_peer_state sta_state,
				 bool roam_synch_in_progress)
{
	struct ol_txrx_peer_t *peer;
	CDF_STATUS err;
	struct ol_txrx_pdev_t *pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (!pdev) {
		hdd_err("Failed to get txrx context");
		return CDF_STATUS_E_FAULT;
	}

	if (sta_id >= WLAN_MAX_STA_COUNT) {
		hddLog(LOGE, "Invalid sta id :%d", sta_id);
		return CDF_STATUS_E_INVAL;
	}

	peer = ol_txrx_peer_find_by_local_id(pdev, sta_id);
	if (!peer)
		return CDF_STATUS_E_FAULT;

	err = ol_txrx_peer_state_update(pdev,
			(u_int8_t *) peer->mac_addr.raw, sta_state);
	if (err != CDF_STATUS_SUCCESS) {
		hddLog(LOGE, "peer state update failed");
		return CDF_STATUS_E_FAULT;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (roam_synch_in_progress)
		return CDF_STATUS_SUCCESS;
#endif

	if (sta_state == ol_txrx_peer_state_auth) {
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
		/* make sure event is reset */
		INIT_COMPLETION(pAdapter->sta_authorized_event);
#endif

		err = sme_set_peer_authorized(peer->mac_addr.raw,
				hdd_set_peer_authorized_event,
				pAdapter->sessionId);
		if (err != CDF_STATUS_SUCCESS) {
			hddLog(LOGE, "Failed to set the peer state to authorized");
			return CDF_STATUS_E_FAULT;
		}

		if (pAdapter->device_mode == WLAN_HDD_INFRA_STATION ||
		    pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			unsigned long rc;

			/* wait for event from firmware to set the event */
			rc = wait_for_completion_timeout(
				&pAdapter->sta_authorized_event,
				msecs_to_jiffies(HDD_PEER_AUTHORIZE_WAIT));
			if (!rc) {
				hddLog(LOG1, "%s: timeout waiting for sta_authorized_event",
					__func__);
			}
			ol_txrx_vdev_unpause(peer->vdev,
				OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED);
#endif
		}
	}
	return CDF_STATUS_SUCCESS;
}

/**
 * hdd_roam_register_sta() - register station
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @staId: station identifier
 * @pPeerMacAddress: peer MAC address
 * @pBssDesc: pointer to BSS description
 *
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS hdd_roam_register_sta(hdd_adapter_t *pAdapter,
					tCsrRoamInfo *pRoamInfo,
					uint8_t staId,
					struct cdf_mac_addr *pPeerMacAddress,
					tSirBssDescription *pBssDesc)
{
	CDF_STATUS cdf_status = CDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type staDesc = { 0 };
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	if (NULL == pBssDesc)
		return CDF_STATUS_E_FAILURE;

	/* Get the Station ID from the one saved during the association */
	staDesc.sta_id = staId;

	/* set the QoS field appropriately */
	if (hdd_wmm_is_active(pAdapter))
		staDesc.is_qos_enabled = 1;
	else
		staDesc.is_qos_enabled = 0;

#ifdef FEATURE_WLAN_WAPI
	hddLog(LOG1, FL("WAPI STA Registered: %d"),
	       pAdapter->wapi_info.fIsWapiSta);
	if (pAdapter->wapi_info.fIsWapiSta)
		staDesc.is_wapi_supported = 1;
	else
		staDesc.is_wapi_supported = 0;
#endif /* FEATURE_WLAN_WAPI */

	cdf_status = ol_txrx_register_peer(hdd_rx_packet_cbk,
						&staDesc);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		hddLog(LOGW,
			"ol_txrx_register_peer() failed to register. Status=%d [0x%08X]",
			cdf_status, cdf_status);
		return cdf_status;
	}

	if (!pRoamInfo->fAuthRequired) {
		/*
		 * Connections that do not need Upper layer auth, transition
		 * TLSHIM directly to 'Authenticated' state
		 */
		cdf_status =
			hdd_change_peer_state(pAdapter, staDesc.sta_id,
						ol_txrx_peer_state_auth,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);

		hdd_conn_set_authenticated(pAdapter, true);
	} else {
		hddLog(LOG3,
			"ULA auth StaId= %d. Changing TL state to CONNECTED at Join time",
			 pHddStaCtx->conn_info.staId[0]);
		cdf_status =
			hdd_change_peer_state(pAdapter, staDesc.sta_id,
						ol_txrx_peer_state_conn,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);
		hdd_conn_set_authenticated(pAdapter, false);
	}
	return cdf_status;
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
	uint8_t *rspRsnIe = kmalloc(IW_GENERIC_IE_MAX, GFP_KERNEL);
	uint32_t rspRsnLength = 0;
	struct ieee80211_channel *chan;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	uint8_t buf_ssid_ie[2 + SIR_MAC_SSID_EID_MAX]; /* 2 bytes-EID and len */
	uint8_t *buf_ptr, ssid_ie_len;
	struct cfg80211_bss *bss = NULL;
	uint8_t *final_req_ie = NULL;
	tCsrRoamConnectedProfile roam_profile;
	tHalHandle hal_handle = WLAN_HDD_GET_HAL_CTX(pAdapter);

	if (!rspRsnIe) {
		hddLog(LOGE, FL("Unable to allocate RSN IE"));
		return;
	}

	if (pCsrRoamInfo == NULL) {
		hddLog(LOGE, FL("Invalid CSR roam info"));
		goto done;
	}

	if (pCsrRoamInfo->nAssocRspLength == 0) {
		hddLog(LOGE, FL("Invalid assoc response length"));
		goto done;
	}

	pFTAssocRsp =
		(u8 *) (pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength +
			pCsrRoamInfo->nAssocReqLength);
	if (pFTAssocRsp == NULL)
		goto done;

	/* pFTAssocRsp needs to point to the IEs */
	pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
	hddLog(LOG1, FL("AssocRsp is now at %02x%02x"),
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
	cds_decr_session_set_pcl(
					pHddCtx, pAdapter->device_mode,
					pAdapter->sessionId);

	/* Send the Assoc Resp, the supplicant needs this for initial Auth */
	len = pCsrRoamInfo->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
	rspRsnLength = len;
	cdf_mem_copy(rspRsnIe, pFTAssocRsp, len);
	cdf_mem_zero(rspRsnIe + len, IW_GENERIC_IE_MAX - len);

	chan = ieee80211_get_channel(pAdapter->wdev.wiphy,
				     (int)pCsrRoamInfo->pBssDesc->channelId);
	cdf_mem_zero(&roam_profile, sizeof(tCsrRoamConnectedProfile));
	sme_roam_get_connect_profile(hal_handle, pAdapter->sessionId,
		&roam_profile);
	bss = cfg80211_get_bss(pAdapter->wdev.wiphy, chan,
			pCsrRoamInfo->bssid.bytes,
			&roam_profile.SSID.ssId[0], roam_profile.SSID.length,
			WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

	if (bss == NULL)
		hddLog(LOGE, FL("Get BSS returned NULL"));
	buf_ptr = buf_ssid_ie;
	*buf_ptr = SIR_MAC_SSID_EID;
	buf_ptr++;
	*buf_ptr = roam_profile.SSID.length; /*len of ssid*/
	buf_ptr++;
	cdf_mem_copy(buf_ptr, &roam_profile.SSID.ssId[0],
			roam_profile.SSID.length);
	ssid_ie_len = 2 + roam_profile.SSID.length;
	hddLog(LOG2, FL("SSIDIE:"));
	hddLog(CDF_TRACE_LEVEL_DEBUG, buf_ssid_ie, ssid_ie_len);
	final_req_ie = kmalloc(IW_GENERIC_IE_MAX, GFP_KERNEL);
	if (final_req_ie == NULL)
		goto done;
	buf_ptr = final_req_ie;
	cdf_mem_copy(buf_ptr, buf_ssid_ie, ssid_ie_len);
	buf_ptr += ssid_ie_len;
	cdf_mem_copy(buf_ptr, reqRsnIe, reqRsnLength);
	cdf_mem_copy(rspRsnIe, pFTAssocRsp, len);
	cdf_mem_zero(final_req_ie + (ssid_ie_len + reqRsnLength),
		IW_GENERIC_IE_MAX - (ssid_ie_len + reqRsnLength));
	hddLog(LOG2, FL("Req RSN IE:"));
	hddLog(CDF_TRACE_LEVEL_DEBUG, final_req_ie,
		(ssid_ie_len + reqRsnLength));
	cfg80211_roamed_bss(dev, bss,
			final_req_ie, (ssid_ie_len + reqRsnLength),
			rspRsnIe, rspRsnLength, GFP_KERNEL);

	wlan_hdd_send_roam_auth_event(pHddCtx, pCsrRoamInfo->bssid.bytes,
			reqRsnIe, reqRsnLength, rspRsnIe, rspRsnLength,
			pCsrRoamInfo);
done:
	sme_roam_free_connect_profile(hal_handle, &roam_profile);
	if (final_req_ie)
		kfree(final_req_ie);
	kfree(rspRsnIe);
}

/**
 * hdd_is_roam_sync_in_progress()- Check if roam offloaded
 *
 * Return: roam sync status if roaming offloaded else false
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static inline bool hdd_is_roam_sync_in_progress(tCsrRoamInfo *roaminfo)
{
	return roaminfo->roamSynchInProgress;
}
#else
static inline bool hdd_is_roam_sync_in_progress(tCsrRoamInfo *roaminfo)
{
	return false;
}
#endif


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
	int ret;
	hdd_station_ctx_t *hddstactx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hddLog(LOG1,
		"Changing TL state to AUTHENTICATED for StaId= %d",
		hddstactx->conn_info.staId[0]);

	/* Connections that do not need Upper layer authentication,
	 * transition TL to 'Authenticated' state after the keys are set
	 */
	ret = hdd_change_peer_state(adapter,
			hddstactx->conn_info.staId[0],
			ol_txrx_peer_state_auth,
			hdd_is_roam_sync_in_progress(roaminfo));
	hdd_conn_set_authenticated(adapter, true);
	if ((WLAN_HDD_INFRA_STATION == adapter->device_mode) ||
		(WLAN_HDD_P2P_CLIENT == adapter->device_mode)) {
		sme_ps_enable_auto_ps_timer(
			WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId,
			hddstactx->hdd_ReassocScenario);
	}

	return ret;
}

/**
 * hdd_roam_set_key_complete_handler() - Update the security parameters
 * @pAdapter: pointer to adapter
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS hdd_roam_set_key_complete_handler(hdd_adapter_t *pAdapter,
						    tCsrRoamInfo *pRoamInfo,
						    uint32_t roamId,
						    eRoamCmdStatus roamStatus,
						    eCsrRoamResult roamResult)
{
	eCsrEncryptionType connectedCipherAlgo;
	bool fConnected = false;
	CDF_STATUS cdf_status = CDF_STATUS_E_FAILURE;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	ENTER();

	if (NULL == pRoamInfo) {
		hddLog(LOG2, FL("pRoamInfo is NULL"));
		return CDF_STATUS_E_FAILURE;
	}
	/*
	 * if (WPA), tell TL to go to 'authenticated' after the keys are set.
	 * then go to 'authenticated'.  For all other authentication types
	 * (those that do not require upper layer authentication) we can put TL
	 * directly into 'authenticated' state.
	 */
	hddLog(LOG2, "Set Key completion roamStatus =%d roamResult=%d "
		  MAC_ADDRESS_STR, roamStatus, roamResult,
		  MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes));

	fConnected = hdd_conn_get_connected_cipher_algo(pHddStaCtx,
						   &connectedCipherAlgo);
	if (fConnected) {
		if (WLAN_HDD_IBSS == pAdapter->device_mode) {
			uint8_t staId;

			if (cdf_is_macaddr_broadcast(&pRoamInfo->peerMac)) {
				pHddStaCtx->roam_info.roamingState =
					HDD_ROAM_STATE_NONE;
			} else {
				cdf_status = hdd_ibss_get_sta_id(
							pHddStaCtx,
							&pRoamInfo->peerMac,
							&staId);
				if (CDF_STATUS_SUCCESS == cdf_status) {
					hddLog(LOG2,
						"WLAN TL STA Ptk Installed for STAID=%d",
						staId);
					pHddStaCtx->roam_info.roamingState =
						HDD_ROAM_STATE_NONE;
				}
			}
		} else {
			/*
			 * TODO: Considering getting a state machine in
			 * HDD later.This routine is invoked twice.
			 * 1)set PTK 2)set GTK.The following if
			 * statement will be TRUE when setting GTK.
			 * At this time we don't handle the state in detail.
			 * Related CR: 174048 - TL not in authenticated state
			*/
			if (eCSR_ROAM_RESULT_AUTHENTICATED == roamResult)
				pHddStaCtx->conn_info.gtk_installed = true;
			else
				pHddStaCtx->conn_info.ptk_installed = true;

			/* In WPA case move STA to authenticated when
			 * ptk is installed.Earlier in WEP case STA
			 * was moved to AUTHENTICATED prior to setting
			 * the unicast key and it was resulting in sending
			 * few un-encrypted packet. Now in WEP case
			 * STA state will be moved to AUTHENTICATED
			 * after we set the unicast and broadcast key.
			 */
			if ((pHddStaCtx->conn_info.ucEncryptionType ==
			  eCSR_ENCRYPT_TYPE_WEP40) ||
			  (pHddStaCtx->conn_info.ucEncryptionType ==
			  eCSR_ENCRYPT_TYPE_WEP104) ||
			  (pHddStaCtx->conn_info.ucEncryptionType ==
			  eCSR_ENCRYPT_TYPE_WEP40_STATICKEY) ||
			  (pHddStaCtx->conn_info.ucEncryptionType ==
			  eCSR_ENCRYPT_TYPE_WEP104_STATICKEY)) {
				if (pHddStaCtx->conn_info.gtk_installed &&
					pHddStaCtx->conn_info.ptk_installed)
					cdf_status =
					    hdd_change_sta_state_authenticated(pAdapter,
						pRoamInfo);
			} else if (pHddStaCtx->conn_info.ptk_installed) {
				cdf_status =
				    hdd_change_sta_state_authenticated(pAdapter,
					pRoamInfo);
			}

			if (pHddStaCtx->conn_info.gtk_installed &&
				pHddStaCtx->conn_info.ptk_installed) {
				pHddStaCtx->conn_info.gtk_installed = false;
				pHddStaCtx->conn_info.ptk_installed = false;
			}

			pHddStaCtx->roam_info.roamingState =
						HDD_ROAM_STATE_NONE;
		}
	} else {
		/*
		 * possible disassoc after issuing set key and waiting
		 * set key complete.
		 */
		pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
	}

	EXIT();
	return CDF_STATUS_SUCCESS;
}

/**
 * hdd_perform_roam_set_key_complete() - perform set key complete
 * @pAdapter: pointer to adapter
 *
 * Return: none
 */
void hdd_perform_roam_set_key_complete(hdd_adapter_t *pAdapter)
{
	CDF_STATUS cdf_ret_status = CDF_STATUS_SUCCESS;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	tCsrRoamInfo roamInfo;
	roamInfo.fAuthRequired = false;
	cdf_mem_copy(roamInfo.bssid.bytes,
		     pHddStaCtx->roam_info.bssid, CDF_MAC_ADDR_SIZE);
	cdf_mem_copy(roamInfo.peerMac.bytes,
		     pHddStaCtx->roam_info.peerMac, CDF_MAC_ADDR_SIZE);

	cdf_ret_status =
			hdd_roam_set_key_complete_handler(pAdapter,
					   &roamInfo,
					   pHddStaCtx->roam_info.roamId,
					   pHddStaCtx->roam_info.roamStatus,
					   eCSR_ROAM_RESULT_AUTHENTICATED);
	if (cdf_ret_status != CDF_STATUS_SUCCESS)
		hddLog(LOGE, FL("Set Key complete failure"));

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
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS hdd_association_completion_handler(hdd_adapter_t *pAdapter,
						     tCsrRoamInfo *pRoamInfo,
						     uint32_t roamId,
						     eRoamCmdStatus roamStatus,
						     eCsrRoamResult roamResult)
{
	struct net_device *dev = pAdapter->dev;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	CDF_STATUS cdf_status = CDF_STATUS_E_FAILURE;
	uint8_t reqRsnIe[DOT11F_IE_RSN_MAX_LEN];
	uint32_t reqRsnLength = DOT11F_IE_RSN_MAX_LEN;
#if  defined(FEATURE_WLAN_ESE) || defined(FEATURE_WLAN_LFR) || \
defined(WLAN_FEATURE_VOWIFI_11R)
	int ft_carrier_on = false;
#endif
	bool hddDisconInProgress = false;
	unsigned long rc;

	if (!pHddCtx) {
		hdd_err("HDD context is NULL");
		return CDF_STATUS_E_FAILURE;
	}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (pRoamInfo && pRoamInfo->roamSynchInProgress) {
		/* change logging before release */
		hddLog(LOG3, "LFR3:hdd_association_completion_handler");
	}
#endif

	/* HDD has initiated disconnect, do not send connect result indication
	 * to kernel as it will be handled by __cfg80211_disconnect.
	 */
	if ((eConnectionState_Disconnecting == pHddStaCtx->conn_info.connState)
	    && ((eCSR_ROAM_RESULT_ASSOCIATED == roamResult)
		|| (eCSR_ROAM_ASSOCIATION_FAILURE == roamStatus))) {
		hddLog(LOG1, FL("Disconnect from HDD in progress"));
		hddDisconInProgress = true;
	}

	if (eCSR_ROAM_RESULT_ASSOCIATED == roamResult) {
		if (NULL == pRoamInfo) {
			hddLog(LOGE, FL("pRoamInfo is NULL"));
			return CDF_STATUS_E_FAILURE;
		}
		if (!hddDisconInProgress) {
			hddLog(LOG1, FL("Set HDD connState to eConnectionState_Associated"));
			hdd_conn_set_connection_state(pAdapter,
						   eConnectionState_Associated);
		}
		/* Save the connection info from CSR... */
		hdd_conn_save_connect_info(pAdapter, pRoamInfo,
					   eCSR_BSS_TYPE_INFRASTRUCTURE);
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

		/* Indicate 'connect' status to user space */
		hdd_send_association_event(dev, pRoamInfo);

		if (cds_is_mcc_in_24G(pHddCtx)) {
			if (pHddCtx->miracast_value)
				cds_set_mas(pAdapter, pHddCtx->miracast_value);
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
#if  defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
defined(FEATURE_WLAN_LFR)
		if (pHddStaCtx->ft_carrier_on == false) {
#endif
		/*
		 * Enable Linkup Event Servicing which allows the net device
		 * notifier to set the linkup event variable.
		 */
		pAdapter->isLinkUpSvcNeeded = true;

		/*
		 * Enable Linkup Event Servicing which allows the net device
		 * notifier to set the linkup event variable.
		 */
		pAdapter->isLinkUpSvcNeeded = true;

		/* Switch on the Carrier to activate the device */
		wlan_hdd_netif_queue_control(pAdapter, WLAN_NETIF_CARRIER_ON,
					   WLAN_CONTROL_PATH);

		/*
		 * Wait for the Link to up to ensure all the queues are set
		 * properly by the kernel.
		 */
		rc = wait_for_completion_timeout(&pAdapter->
						 linkup_event_var,
						 msecs_to_jiffies
						 (ASSOC_LINKUP_TIMEOUT));
		if (!rc)
			hddLog(LOGW, FL("Warning:ASSOC_LINKUP_TIMEOUT"));

		/*
		 * Disable Linkup Event Servicing - no more service required
		 * from the net device notifier call.
		 */
		pAdapter->isLinkUpSvcNeeded = false;
#if  defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
defined(FEATURE_WLAN_LFR)
	} else {
		pHddStaCtx->ft_carrier_on = false;
		ft_carrier_on = true;
	}
#endif
		if ((WLAN_MAX_STA_COUNT + 3) > pRoamInfo->staId)
			pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;
		else
			hddLog(LOGE, "%s: Wrong Staid: %d", __func__,
						pRoamInfo->staId);

		pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;

		if (hdd_ipa_is_enabled(pHddCtx))
			hdd_ipa_wlan_evt(pAdapter, pRoamInfo->staId,
					 WLAN_STA_CONNECT,
					 pRoamInfo->bssid.bytes);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(pHddCtx, false);
#endif

		cds_check_concurrent_intf_and_restart_sap(pHddCtx,
							  pHddStaCtx,
							  pAdapter);

#ifdef FEATURE_WLAN_TDLS
		wlan_hdd_tdls_connection_callback(pAdapter);
#endif

#ifdef QCA_PKT_PROTO_TRACE
		/* STA Associated, update into trace buffer */
		if (pHddCtx->config->gEnableDebugLog)
			cds_pkt_trace_buf_update("ST:ASSOC");
#endif /* QCA_PKT_PROTO_TRACE */
		/*
		 * For reassoc, the station is already registered, all we need
		 * is to change the state of the STA in TL.
		 * If authentication is required (WPA/WPA2/DWEP), change TL to
		 * CONNECTED instead of AUTHENTICATED.
		 */
		if (!pRoamInfo->fReassocReq) {
			struct cfg80211_bss *bss;
#ifdef WLAN_FEATURE_VOWIFI_11R
			u8 *pFTAssocRsp = NULL;
			unsigned int assocRsplen = 0;
			u8 *pFTAssocReq = NULL;
			unsigned int assocReqlen = 0;
			struct ieee80211_channel *chan;
#endif
			uint8_t rspRsnIe[DOT11F_IE_RSN_MAX_LEN];
			uint32_t rspRsnLength = DOT11F_IE_RSN_MAX_LEN;

			/* add bss_id to cfg80211 data base */
			bss =
				wlan_hdd_cfg80211_update_bss_db(pAdapter,
								pRoamInfo);
			if (NULL == bss) {
				pr_err("wlan: Not able to create BSS entry\n");
				wlan_hdd_netif_queue_control(pAdapter,
					WLAN_NETIF_CARRIER_OFF,
					WLAN_CONTROL_PATH);
				return CDF_STATUS_E_FAILURE;
			}
#ifdef WLAN_FEATURE_VOWIFI_11R
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
					hddLog(LOG1,
					       FL("AssocRsp is now at %02x%02x"),
					       (unsigned int)pFTAssocRsp[0],
					       (unsigned int)pFTAssocRsp[1]);
					assocRsplen =
						pRoamInfo->nAssocRspLength -
						FT_ASSOC_RSP_IES_OFFSET;
				} else {
					hddLog(LOGE, FL("AssocRsp is NULL"));
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
						hddLog(LOG1,
							FL("pFTAssocReq is now at %02x%02x"),
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
					hddLog(LOGE, FL("AssocReq is NULL"));
					assocReqlen = 0;
				}

				if (ft_carrier_on) {
					if (!hddDisconInProgress) {
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
						cds_decr_session_set_pcl
							(pHddCtx,
							pAdapter->device_mode,
							pAdapter->sessionId);
						hddLog(LOG1,
						       FL("ft_carrier_on is %d, sending roamed indication"),
						       ft_carrier_on);
						chan =
							ieee80211_get_channel
								(pAdapter->wdev.wiphy,
								(int)pRoamInfo->pBssDesc->
								channelId);
						hddLog(LOG1,
						       "assocReqlen %d assocRsplen %d",
						       assocReqlen,
						       assocRsplen);
						cfg80211_roamed(dev, chan,
								pRoamInfo->
								bssid.bytes,
								pFTAssocReq,
								assocReqlen,
								pFTAssocRsp,
								assocRsplen,
								GFP_KERNEL);
						wlan_hdd_send_roam_auth_event(
								pHddCtx,
								pRoamInfo->bssid.bytes,
								pFTAssocReq,
								assocReqlen,
								pFTAssocRsp,
								assocRsplen,
								pRoamInfo);
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

						cdf_mem_copy(pHddStaCtx->
							     roam_info.bssid,
							     pRoamInfo->bssid.bytes,
							     CDF_MAC_ADDR_SIZE);
						cdf_mem_copy(pHddStaCtx->
							     roam_info.peerMac,
							     pRoamInfo->peerMac.bytes,
							     CDF_MAC_ADDR_SIZE);
						pHddStaCtx->roam_info.roamId =
							roamId;
						pHddStaCtx->roam_info.
						roamStatus = roamStatus;
						pHddStaCtx->roam_info.
						deferKeyComplete = true;
					}
				} else if (!hddDisconInProgress) {
					hddLog(LOG1,
					       FL("ft_carrier_on is %d, sending connect indication"),
					       ft_carrier_on);
					cfg80211_connect_result(dev,
								pRoamInfo->
								bssid.bytes,
								pFTAssocReq,
								assocReqlen,
								pFTAssocRsp,
								assocRsplen,
								WLAN_STATUS_SUCCESS,
								GFP_KERNEL);
				}
			} else
#endif
			{
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
#if  defined(FEATURE_WLAN_ESE) || defined(FEATURE_WLAN_LFR)
					if (ft_carrier_on)
						hdd_send_re_assoc_event(dev,
									pAdapter,
									pRoamInfo,
									reqRsnIe,
									reqRsnLength);
					else
#endif /* FEATURE_WLAN_ESE */

					{
						hddLog(LOG1,
						       FL("sending connect indication to nl80211:for bssid "
						       MAC_ADDRESS_STR
						       " reason:%d and Status:%d"),
						       MAC_ADDR_ARRAY
							       (pRoamInfo->bssid.bytes),
						       roamResult, roamStatus);

						/* inform connect result to nl80211 */
						cfg80211_connect_result(dev,
									pRoamInfo->
									bssid.bytes,
									reqRsnIe,
									reqRsnLength,
									rspRsnIe,
									rspRsnLength,
									WLAN_STATUS_SUCCESS,
									GFP_KERNEL);
					}
				}
			}
			if (!hddDisconInProgress) {
				cfg80211_put_bss(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
					pHddCtx->wiphy,
#endif
					bss);

				/*
				 * Perform any WMM-related association
				 * processing.
				 */
				hdd_wmm_assoc(pAdapter, pRoamInfo,
					      eCSR_BSS_TYPE_INFRASTRUCTURE);

				/*
				 * Start the Queue - Start tx queues before
				 * hdd_roam_register_sta, since
				 * hdd_roam_register_sta will flush any cached
				 * data frames immediately.
				 */
				hddLog(LOG1, FL("Enabling queues"));
				wlan_hdd_netif_queue_control(pAdapter,
						WLAN_WAKE_ALL_NETIF_QUEUE,
						WLAN_CONTROL_PATH);

				/*
				 * Register the Station with TL after associated
				 */
				cdf_status = hdd_roam_register_sta(pAdapter,
								   pRoamInfo,
								   pHddStaCtx->
								   conn_info.
								   staId[0],
								   NULL,
								   pRoamInfo->
								   pBssDesc);
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
				cdf_status =
					hdd_change_peer_state(pAdapter,
						pHddStaCtx->conn_info.staId[0],
						ol_txrx_peer_state_conn,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);
				hdd_conn_set_authenticated(pAdapter, false);
			} else {
				hddLog(LOG2,
					FL("staId: %d Changing TL state to AUTHENTICATED"),
					  pHddStaCtx->conn_info.staId[0]);
				cdf_status =
					hdd_change_peer_state(pAdapter,
						pHddStaCtx->conn_info.staId[0],
						ol_txrx_peer_state_auth,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						pRoamInfo->roamSynchInProgress
#else
						false
#endif
						);
				hdd_conn_set_authenticated(pAdapter, true);
			}

			if (CDF_IS_STATUS_SUCCESS(cdf_status)) {
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
				hddLog(LOG3, "LFR3:netif_tx_wake_all_queues");
#endif
			hddLog(LOG1, FL("Enabling queues"));
			wlan_hdd_netif_queue_control(pAdapter,
						   WLAN_WAKE_ALL_NETIF_QUEUE,
						   WLAN_CONTROL_PATH);
		}

		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			hddLog(LOGE,
				"STA register with TL failed. status(=%d) [%08X]",
				cdf_status, cdf_status);
		}
#ifdef WLAN_FEATURE_11W
		cdf_mem_zero(&pAdapter->hdd_stats.hddPmfStats,
			     sizeof(pAdapter->hdd_stats.hddPmfStats));
#endif
	} else {
		hdd_wext_state_t *pWextState =
			WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
		if (pRoamInfo)
			pr_info("wlan: connection failed with " MAC_ADDRESS_STR
				" reason:%d and Status:%d\n",
				MAC_ADDR_ARRAY(pRoamInfo->bssid.bytes),
				roamResult, roamStatus);
		else
			pr_info("wlan: connection failed with " MAC_ADDRESS_STR
				" reason:%d and Status:%d\n",
				MAC_ADDR_ARRAY(pWextState->req_bssId.bytes),
				roamResult, roamStatus);

		/*
		 * CR465478: Only send up a connection failure result when CSR
		 * has completed operation - with a ASSOCIATION_FAILURE status.
		 */
		if (eCSR_ROAM_ASSOCIATION_FAILURE == roamStatus
		    && !hddDisconInProgress) {
			if (pRoamInfo)
				hddLog(LOGE,
				       FL("send connect failure to nl80211: for bssid "
				       MAC_ADDRESS_STR
				       " reason:%d and Status:%d "),
				       MAC_ADDR_ARRAY(pRoamInfo->bssid.bytes),
				       roamResult, roamStatus);
			else
				hddLog(LOGE,
				       FL("connect failed: for bssid "
				       MAC_ADDRESS_STR
				       " reason:%d and Status:%d "),
				       MAC_ADDR_ARRAY(pWextState->req_bssId.bytes),
				       roamResult, roamStatus);

			hdd_clear_roam_profile_ie(pAdapter);

			/* inform association failure event to nl80211 */
			if (eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL ==
			    roamResult) {
				if (pRoamInfo)
					cfg80211_connect_result(dev,
						pRoamInfo->bssid.bytes,
						NULL, 0, NULL, 0,
						WLAN_STATUS_ASSOC_DENIED_UNSPEC,
						GFP_KERNEL);
				else
					cfg80211_connect_result(dev,
						pWextState->req_bssId.bytes,
						NULL, 0, NULL, 0,
						WLAN_STATUS_ASSOC_DENIED_UNSPEC,
						GFP_KERNEL);
			} else {
				if (pRoamInfo) {
					eCsrAuthType authType =
						pWextState->roamProfile.AuthType.
						authType[0];
					bool isWep =
						(authType ==
						 eCSR_AUTH_TYPE_OPEN_SYSTEM)
						|| (authType ==
						    eCSR_AUTH_TYPE_SHARED_KEY);

					/*
					 * In case of OPEN-WEP or SHARED-WEP
					 * authentication, send exact protocol
					 * reason code. This enables user
					 * applications to reconnect the station
					 * with correct configuration.
					 */
					cfg80211_connect_result(dev,
						pRoamInfo->bssid.bytes, NULL, 0,
						NULL, 0,
						isWep ? pRoamInfo->reasonCode :
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL);
				} else
					cfg80211_connect_result(dev,
						pWextState->req_bssId.bytes,
						NULL, 0, NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL);
			}
		}

		if (pRoamInfo) {
			if ((eSIR_SME_JOIN_TIMEOUT_RESULT_CODE ==
			     pRoamInfo->statusCode)
			    || (eSIR_SME_AUTH_TIMEOUT_RESULT_CODE ==
				pRoamInfo->statusCode)
			    || (eSIR_SME_ASSOC_TIMEOUT_RESULT_CODE ==
				pRoamInfo->statusCode)) {
				wlan_hdd_cfg80211_update_bss_list(pAdapter,
								  pRoamInfo);
			}
		}

		/*
		 * Set connection state to eConnectionState_NotConnected only
		 * when CSR has completed operation - with a
		 * ASSOCIATION_FAILURE status.
		 */
		if (eCSR_ROAM_ASSOCIATION_FAILURE == roamStatus
		    && !hddDisconInProgress) {
			hddLog(LOG1,
				FL("state to eConnectionState_NotConnected"));
			hdd_conn_set_connection_state(pAdapter,
					eConnectionState_NotConnected);
		}
		hdd_wmm_init(pAdapter);

		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_NETIF_TX_DISABLE_N_CARRIER,
					   WLAN_CONTROL_PATH);
	}

	if (CDF_STATUS_SUCCESS != cds_check_and_restart_sap(pHddCtx,
					roamResult, pHddStaCtx))
		return CDF_STATUS_E_FAILURE;

	cds_force_sap_on_scc(pHddCtx, roamResult);

	return CDF_STATUS_SUCCESS;
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
	hddLog(LOG1, "%s: id %d, status %d, result %d",
	       pAdapter->dev->name, roamId, roamStatus, roamResult);

	switch (roamResult) {
	/* both IBSS Started and IBSS Join should come in here. */
	case eCSR_ROAM_RESULT_IBSS_STARTED:
	case eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS:
	case eCSR_ROAM_RESULT_IBSS_COALESCED:
	{
		hdd_context_t *pHddCtx =
			(hdd_context_t *) pAdapter->pHddCtx;
		struct cdf_mac_addr broadcastMacAddr =
			CDF_MAC_ADDR_BROADCAST_INITIALIZER;

		if (NULL == pRoamInfo) {
			CDF_ASSERT(0);
			return;
		}

		/* When IBSS Started comes from CSR, we need to move
		 * connection state to IBSS Disconnected (meaning no peers
		 * are in the IBSS).
		 */
		hddLog(LOG1,
			FL("Set HDD connState to eConnectionState_IbssDisconnected"));
		hdd_conn_set_connection_state(pAdapter,
				      eConnectionState_IbssDisconnected);
		/* notify wmm */
		hdd_wmm_connect(pAdapter, pRoamInfo,
				eCSR_BSS_TYPE_IBSS);
		pHddCtx->sta_to_adapter[IBSS_BROADCAST_STAID] =
			pAdapter;
		hdd_roam_register_sta(pAdapter, pRoamInfo,
				      IBSS_BROADCAST_STAID,
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
			hddLog(LOG1,
			       FL("%s: created ibss " MAC_ADDRESS_STR),
			       pAdapter->dev->name,
			       MAC_ADDR_ARRAY(pRoamInfo->pBssDesc->bssId));

			/* we must first give cfg80211 the BSS information */
			bss = wlan_hdd_cfg80211_update_bss_db(pAdapter,
								pRoamInfo);
			if (NULL == bss) {
				hddLog(LOGE,
				       FL("%s: unable to create IBSS entry"),
				       pAdapter->dev->name);
				return;
			}
			hddLog(LOG1, FL("Enabling queues"));
			wlan_hdd_netif_queue_control(pAdapter,
					WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
			chan_no = pRoamInfo->pBssDesc->channelId;

			if (chan_no <= 14)
				freq = ieee80211_channel_to_frequency(chan_no,
					  IEEE80211_BAND_2GHZ);
			else
				freq = ieee80211_channel_to_frequency(chan_no,
					  IEEE80211_BAND_5GHZ);

			chan = ieee80211_get_channel(pAdapter->wdev.wiphy, freq);

			if (chan)
				cfg80211_ibss_joined(pAdapter->dev,
						     bss->bssid, chan,
						     GFP_KERNEL);
			else
				hddLog(LOGE, FL("%s: chanId: %d, can't find channel"),
					   pAdapter->dev->name,
					   (int)pRoamInfo->pBssDesc->channelId);
#else
			cfg80211_ibss_joined(pAdapter->dev, bss->bssid,
					     GFP_KERNEL);
#endif
			cfg80211_put_bss(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
				pHddCtx->wiphy,
#endif
				bss);
		}

		break;
	}

	case eCSR_ROAM_RESULT_IBSS_START_FAILED:
	{
		hddLog(LOGE,
		       FL("%s: unable to create IBSS"), pAdapter->dev->name);
		break;
	}

	default:
		hddLog(LOGE, FL("%s: unexpected result %d"),
		       pAdapter->dev->name, (int)roamResult);
		break;
	}

	return;
}

/**
 * roam_save_ibss_station() - Save the IBSS peer MAC address in the adapter
 * @pHddStaCtx: pointer to global HDD station context
 * @staId: station id
 * @peerMacAddress: pointer to peer MAC address
 *
 * This information is passed to iwconfig later. The peer that joined
 * last is passed as information to iwconfig.
 *
 * Return:
 *	true if we add MAX_IBSS_PEERS or less STA
 *	false otherwise.
 */
static bool roam_save_ibss_station(hdd_station_ctx_t *pHddStaCtx, uint8_t staId,
				  struct cdf_mac_addr *peerMacAddress)
{
	bool fSuccess = false;
	int idx = 0;

	for (idx = 0; idx < MAX_IBSS_PEERS; idx++) {
		if (0 == pHddStaCtx->conn_info.staId[idx]) {
			pHddStaCtx->conn_info.staId[idx] = staId;

			cdf_copy_macaddr(&pHddStaCtx->conn_info.
					 peerMacAddress[idx], peerMacAddress);

			fSuccess = true;
			break;
		}
	}

	return fSuccess;
}

/**
 * roam_remove_ibss_station() - Remove the IBSS peer MAC address in the adapter
 * @pAdapter: pointer to adapter
 * @staId: station id
 *
 * Return:
 *	true if we remove MAX_IBSS_PEERS or less STA
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

	for (idx = 0; idx < MAX_IBSS_PEERS; idx++) {
		if (staId == pHddStaCtx->conn_info.staId[idx]) {
			pHddStaCtx->conn_info.staId[idx] = 0;

			cdf_zero_macaddr(&pHddStaCtx->conn_info.
					 peerMacAddress[idx]);

			fSuccess = true;

			/*
			 * Note the deleted Index, if its 0 we need special
			 * handling.
			 */
			del_idx = idx;

			empty_slots++;
		} else {
			if (pHddStaCtx->conn_info.staId[idx] != 0) {
				valid_idx = idx;
			} else {
				/* Found an empty slot */
				empty_slots++;
			}
		}
	}

	if (MAX_IBSS_PEERS == empty_slots) {
		/* Last peer departed, set the IBSS state appropriately */
		pHddStaCtx->conn_info.connState =
			eConnectionState_IbssDisconnected;
		hddLog(LOGE, "Last IBSS Peer Departed!!!");
	}
	/* Find next active staId, to have a valid sta trigger for TL. */
	if (fSuccess == true) {
		if (del_idx == 0) {
			if (pHddStaCtx->conn_info.staId[valid_idx] != 0) {
				pHddStaCtx->conn_info.staId[0] =
					pHddStaCtx->conn_info.staId[valid_idx];
				cdf_copy_macaddr(&pHddStaCtx->conn_info.
						 peerMacAddress[0],
						 &pHddStaCtx->conn_info.
						 peerMacAddress[valid_idx]);

				pHddStaCtx->conn_info.staId[valid_idx] = 0;
				cdf_zero_macaddr(&pHddStaCtx->conn_info.
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
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS roam_ibss_connect_handler(hdd_adapter_t *pAdapter,
					    tCsrRoamInfo *pRoamInfo)
{
	struct cfg80211_bss *bss;
	hddLog(LOG1, FL("IBSS Connect Indication from SME. Set HDD connState to eConnectionState_IbssConnected"));
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
		hddLog(LOGE,
		       FL("%s: unable to create IBSS entry"),
		       pAdapter->dev->name);
		return CDF_STATUS_E_FAILURE;
	}
	cfg80211_put_bss(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
		WLAN_HDD_GET_CTX(pAdapter)->wiphy,
#endif
		bss);

	return CDF_STATUS_SUCCESS;
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
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS
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
		hddLog(LOG1, "MIC MAC " MAC_ADDRESS_STR,
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

	return CDF_STATUS_SUCCESS;
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
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS
roam_roam_connect_status_update_handler(hdd_adapter_t *pAdapter,
					tCsrRoamInfo *pRoamInfo,
					uint32_t roamId,
					eRoamCmdStatus roamStatus,
					eCsrRoamResult roamResult)
{
	CDF_STATUS cdf_status;

	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	switch (roamResult) {
	case eCSR_ROAM_RESULT_IBSS_NEW_PEER:
	{
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
		struct station_info staInfo;

		pr_info("IBSS New Peer indication from SME "
			"with peerMac " MAC_ADDRESS_STR " BSSID: "
			MAC_ADDRESS_STR " and stationID= %d",
			MAC_ADDR_ARRAY(pRoamInfo->peerMac.bytes),
			MAC_ADDR_ARRAY(pHddStaCtx->conn_info.bssId.bytes),
			pRoamInfo->staId);

		if (!roam_save_ibss_station
			    (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter),
			    pRoamInfo->staId,
			    &pRoamInfo->peerMac)) {
			hddLog(LOGW, "Max reached: Can't register new IBSS peer");
			break;
		}

		pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;

		pHddCtx->sta_to_adapter[IBSS_BROADCAST_STAID] =
			pAdapter;

		/* Register the Station with TL for the new peer. */
		cdf_status = hdd_roam_register_sta(pAdapter,
						   pRoamInfo,
						   pRoamInfo->staId,
						   &pRoamInfo->peerMac,
						   pRoamInfo->pBssDesc);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			hddLog(LOGE,
				"Cannot register STA with TL for IBSS. Failed with cdf_status = %d [%08X]",
				cdf_status, cdf_status);
		}
		pHddStaCtx->ibss_sta_generation++;
		memset(&staInfo, 0, sizeof(staInfo));
		staInfo.filled = 0;
		staInfo.generation = pHddStaCtx->ibss_sta_generation;

		cfg80211_new_sta(pAdapter->dev,
				 (const u8 *)pRoamInfo->peerMac.bytes,
				 &staInfo, GFP_KERNEL);

		if (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY ==
		    pHddStaCtx->ibss_enc_key.encType
		    || eCSR_ENCRYPT_TYPE_WEP104_STATICKEY ==
		    pHddStaCtx->ibss_enc_key.encType
		    || eCSR_ENCRYPT_TYPE_TKIP ==
		    pHddStaCtx->ibss_enc_key.encType
		    || eCSR_ENCRYPT_TYPE_AES ==
		    pHddStaCtx->ibss_enc_key.encType) {
			pHddStaCtx->ibss_enc_key.keyDirection =
				eSIR_TX_RX;
			cdf_copy_macaddr(&pHddStaCtx->ibss_enc_key.peerMac,
					 &pRoamInfo->peerMac);

			hddLog(LOG2, "New peer joined set PTK encType=%d",
				  pHddStaCtx->ibss_enc_key.encType);

			cdf_status =
				sme_roam_set_key(WLAN_HDD_GET_HAL_CTX
							 (pAdapter),
						 pAdapter->sessionId,
						 &pHddStaCtx->ibss_enc_key,
						 &roamId);

			if (CDF_STATUS_SUCCESS != cdf_status) {
				hddLog(LOGE,
				       FL("sme_roam_set_key failed, status=%d"),
				       cdf_status);
				return CDF_STATUS_E_FAILURE;
			}
		}
		hddLog(LOG1, FL("Enabling queues"));
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
			hddLog(LOGW,
				"IBSS peer departed by cannot find peer in our registration table with TL");

		pr_info("IBSS Peer Departed from SME "
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
		hddLog(LOG3,
			  "Received eCSR_ROAM_RESULT_IBSS_INACTIVE from SME");
		/* Stop only when we are inactive */
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_NETIF_TX_DISABLE_N_CARRIER,
					   WLAN_CONTROL_PATH);
		hddLog(LOG1,
			FL("Set HDD connState to eConnectionState_NotConnected"));
		hdd_conn_set_connection_state(pAdapter,
					      eConnectionState_NotConnected);

		/* Send the bssid address to the wext. */
		hdd_send_association_event(pAdapter->dev, pRoamInfo);
		break;
	}
	default:
		break;

	}

	return CDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_TDLS
/**
 * hdd_roam_register_tdlssta() - register new TDLS station
 * @pAdapter: pointer to adapter
 * @peerMac: pointer to peer MAC address
 * @staId: station identifier
 * @ucastSig: unicast signature
 *
 * Construct the staDesc and register with TL the new STA.
 * This is called as part of ADD_STA in the TDLS setup.
 *
 * Return: CDF_STATUS enumeration
 */
CDF_STATUS hdd_roam_register_tdlssta(hdd_adapter_t *pAdapter,
				     const uint8_t *peerMac, uint16_t staId,
				     uint8_t ucastSig)
{
	CDF_STATUS cdf_status = CDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type staDesc = { 0 };

	/*
	 * TDLS sta in BSS should be set as STA type TDLS and STA MAC should
	 * be peer MAC, here we are working on direct Link
	 */
	staDesc.sta_id = staId;

	/* set the QoS field appropriately .. */
	(hdd_wmm_is_active(pAdapter)) ? (staDesc.is_qos_enabled = 1)
	: (staDesc.is_qos_enabled = 0);


	/* Register the Station with TL...  */
	cdf_status = ol_txrx_register_peer(hdd_rx_packet_cbk,
						&staDesc);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		hddLog(LOGE, FL("ol_txrx_register_peer() failed to register. Status=%d [0x%08X]"),
			cdf_status, cdf_status);
		return cdf_status;
	}

	return cdf_status;
}

/**
 * hdd_roam_deregister_tdlssta() - deregister new TDLS station
 * @pAdapter: pointer to adapter
 * @staId: station identifier
 *
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS hdd_roam_deregister_tdlssta(hdd_adapter_t *pAdapter,
					      uint8_t staId)
{
	CDF_STATUS cdf_status;
	cdf_status = ol_txrx_clear_peer(staId);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		hddLog(LOGW, FL("ol_txrx_clear_peer() failed for staID %d. Status=%d [0x%08X]"),
			staId, cdf_status, cdf_status);
	}
	return cdf_status;
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
 * Return: CDF_STATUS enumeration
 */
static CDF_STATUS
hdd_roam_tdls_status_update_handler(hdd_adapter_t *pAdapter,
				    tCsrRoamInfo *pRoamInfo,
				    uint32_t roamId,
				    eRoamCmdStatus roamStatus,
				    eCsrRoamResult roamResult)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	tdlsCtx_t *pHddTdlsCtx = WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter);
	tSmeTdlsPeerStateParams smeTdlsPeerStateParams;
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	uint8_t staIdx;
	hddTdlsPeer_t *curr_peer;
	uint32_t reason;

	hddLog(LOG2,
		  ("hdd_tdlsStatusUpdate: %s staIdx %d " MAC_ADDRESS_STR),
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

	if (!pHddTdlsCtx) {
		hddLog(LOG1,
			FL("TDLS ctx is null, ignore roamResult (%d)"),
			roamResult);
		return status;
	}

	switch (roamResult) {
	case eCSR_ROAM_RESULT_ADD_TDLS_PEER:
	{
		if (eSIR_SME_SUCCESS != pRoamInfo->statusCode) {
			hddLog(LOGE, FL("Add Sta failed. status code(=%d)"),
				pRoamInfo->statusCode);
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

					hddLog(LOGW,
						  ("TDLS: STA IDX at %d is %d "
						   "of mac "
						   MAC_ADDRESS_STR),
						  staIdx,
						  pHddCtx->
						  tdlsConnInfo[staIdx].
						  staId,
						  MAC_ADDR_ARRAY
							  (pRoamInfo->peerMac.bytes));

					cdf_copy_macaddr(&pHddCtx->
							 tdlsConnInfo
							 [staIdx].
							 peerMac,
							 &pRoamInfo->
							 peerMac);
					status = CDF_STATUS_SUCCESS;
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
					hddLog(LOGE,
						"wlan_hdd_tdls_set_sta_id() failed");
					return CDF_STATUS_E_FAILURE;
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
				status = CDF_STATUS_E_FAILURE;
				hddLog(LOGE,
					FL("no available slot in conn_info. staId %d cannot be stored"),
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
			hddLog(LOGE,
				FL("Add Sta failed. status code(=%d)"),
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
			hddLog(LOGE,
				FL("Link Establish Request failed. status(=%d)"),
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
				hddLog(LOGW,
					  ("HDD: del STA IDX = %x"),
					  pRoamInfo->staId);

				curr_peer =
					wlan_hdd_tdls_find_peer(pAdapter,
								pRoamInfo->
								peerMac.bytes,
								true);
				if (NULL != curr_peer
				    && TDLS_IS_CONNECTED(curr_peer)) {
					hdd_roam_deregister_tdlssta
						(pAdapter,
						pRoamInfo->staId);
					wlan_hdd_tdls_decrement_peer_count
						(pAdapter);
				}
				wlan_hdd_tdls_reset_peer(pAdapter,
							 pRoamInfo->
							 peerMac.bytes);

				pHddCtx->tdlsConnInfo[staIdx].staId = 0;
				pHddCtx->tdlsConnInfo[staIdx].
				sessionId = 255;
				cdf_mem_zero(&pHddCtx->
					     tdlsConnInfo[staIdx].
					     peerMac,
					     CDF_MAC_ADDR_SIZE);
				status = CDF_STATUS_SUCCESS;
				break;
			}
		}
		complete(&pAdapter->tdls_del_station_comp);
	}
	break;
	case eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND:
	{
		hddLog(LOGE,
			FL("Sending teardown to supplicant with reason code %u"),
			pRoamInfo->reasonCode);

		curr_peer =
			wlan_hdd_tdls_find_peer(pAdapter,
						pRoamInfo->peerMac.bytes, true);
		wlan_hdd_tdls_indicate_teardown(pAdapter, curr_peer,
						pRoamInfo->reasonCode);
		status = CDF_STATUS_SUCCESS;
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
				hddLog(LOGW,
					  ("hdd_tdlsStatusUpdate: staIdx %d "
					   MAC_ADDRESS_STR),
					  pHddCtx->tdlsConnInfo[staIdx].
					  staId,
					  MAC_ADDR_ARRAY(pHddCtx->
							 tdlsConnInfo
							 [staIdx].
							 peerMac.
							 bytes));
				wlan_hdd_tdls_reset_peer(pAdapter,
							 pHddCtx->
							 tdlsConnInfo
							 [staIdx].
							 peerMac.bytes);
				hdd_roam_deregister_tdlssta(pAdapter,
							    pHddCtx->
							    tdlsConnInfo
							    [staIdx].
							    staId);
				cdf_mem_zero(&smeTdlsPeerStateParams,
					     sizeof
					     (smeTdlsPeerStateParams));
				smeTdlsPeerStateParams.vdevId =
					pHddCtx->tdlsConnInfo[staIdx].
					sessionId;
				cdf_mem_copy(&smeTdlsPeerStateParams.
					     peerMacAddr,
					     &pHddCtx->
					     tdlsConnInfo[staIdx].
					     peerMac.bytes,
					     CDF_MAC_ADDR_SIZE);
				smeTdlsPeerStateParams.peerState =
					eSME_TDLS_PEER_STATE_TEARDOWN;

				hddLog(LOG1,
					FL("calling sme_update_tdls_peer_state for staIdx %d "
					MAC_ADDRESS_STR),
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
				if (CDF_STATUS_SUCCESS != status) {
					hddLog(LOGE,
						  FL("sme_update_tdls_peer_state failed for "
						  MAC_ADDRESS_STR),
						  MAC_ADDR_ARRAY
							  (pHddCtx->
							  tdlsConnInfo[staIdx].
							  peerMac.bytes));
				}
				wlan_hdd_tdls_decrement_peer_count
					(pAdapter);

				cdf_mem_zero(&pHddCtx->
					     tdlsConnInfo[staIdx].
					     peerMac,
					     CDF_MAC_ADDR_SIZE);
				pHddCtx->tdlsConnInfo[staIdx].staId = 0;
				pHddCtx->tdlsConnInfo[staIdx].
				sessionId = 255;

				status = CDF_STATUS_SUCCESS;
			}
		}
		break;
	}
	case eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER:
	{
		/* ignore TDLS_SHOULD_DISCOVER if any concurrency detected */
		if (((1 << CDF_STA_MODE) != pHddCtx->concurrency_mode) ||
		    (pHddCtx->no_of_active_sessions[CDF_STA_MODE] > 1)) {
			hddLog(LOG2,
				FL("concurrency detected. ignore SHOULD_DISCOVER concurrency_mode: 0x%x, active_sessions: %d"),
				 pHddCtx->concurrency_mode,
				 pHddCtx->no_of_active_sessions[CDF_STA_MODE]);
			status = CDF_STATUS_E_FAILURE;
			break;
		}

		curr_peer =
			wlan_hdd_tdls_get_peer(pAdapter,
					       pRoamInfo->peerMac.bytes);
		if (!curr_peer) {
			hddLog(LOGE, FL("curr_peer is null"));
			status = CDF_STATUS_E_FAILURE;
		} else {
			if (eTDLS_LINK_CONNECTED ==
			    curr_peer->link_status) {
				hddLog(LOGE,
					FL("TDLS link status is connected, ignore SHOULD_DISCOVER"));
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
					hddLog(LOG2,
						FL
						("TDLS ExternalControl enabled but curr_peer is not forced, ignore SHOULD_DISCOVER"));
					status = CDF_STATUS_SUCCESS;
					break;
				} else {
					hddLog(LOG2,
						FL
						("initiate TDLS setup on SHOULD_DISCOVER, fTDLSExternalControl: %d, curr_peer->isForcedPeer: %d, reason: %d"),
						pHddCtx->config->
						fTDLSExternalControl,
						curr_peer->isForcedPeer,
						pRoamInfo->reasonCode);
				}
				wlan_hdd_tdls_pre_setup_init_work
					(pHddTdlsCtx, curr_peer);
			}
			status = CDF_STATUS_SUCCESS;
		}
		break;
	}

	case eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN:
	{
		curr_peer =
			wlan_hdd_tdls_find_peer(pAdapter,
						pRoamInfo->peerMac.bytes, true);
		if (!curr_peer) {
			hddLog(LOGE, FL("curr_peer is null"));
			status = CDF_STATUS_E_FAILURE;
		} else {
			if (eTDLS_LINK_CONNECTED ==
			    curr_peer->link_status) {
				hddLog(LOGE,
					  FL
					  ("Received SHOULD_TEARDOWN for peer "
					  MAC_ADDRESS_STR
					  " staId: %d, reason: %d"),
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
			} else {
				hddLog(LOGE,
					  FL
					  ("TDLS link is not connected, ignore SHOULD_TEARDOWN, reason: %d"),
					  pRoamInfo->reasonCode);
			}
			status = CDF_STATUS_SUCCESS;
		}
		break;
	}

	case eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED:
	{
		curr_peer =
			wlan_hdd_tdls_find_peer(pAdapter,
						pRoamInfo->peerMac.bytes, true);
		if (!curr_peer) {
			hddLog(LOGE, FL("curr_peer is null"));
			status = CDF_STATUS_E_FAILURE;
		} else {
			if (eTDLS_LINK_CONNECTED ==
			    curr_peer->link_status) {
				hddLog(LOGE,
					  FL
					  ("Received SHOULD_PEER_DISCONNECTED for peer "
					  MAC_ADDRESS_STR
					  " staId: %d, reason: %d"),
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
			} else {
				hddLog(LOGE,
					  FL
					  ("TDLS link is not connected, ignore SHOULD_PEER_DISCONNECTED, reason: %d"),
					  pRoamInfo->reasonCode);
			}
			status = CDF_STATUS_SUCCESS;
		}
		break;
	}
	default:
	{
		break;
	}
	}

	return status;
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

	hddLog(LOG1, FL("Frame Type = %d Frame Length = %d"),
	       frameType, nFrameLength);

	/* Sanity Checks */
	if (NULL == pAdapter) {
		hddLog(LOGE, FL("pAdapter is NULL"));
		return;
	}

	if (NULL == pAdapter->dev) {
		hddLog(LOGE, FL("pAdapter->dev is NULL"));
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) {
		hddLog(LOGE, FL("pAdapter has invalid magic"));
		return;
	}

	if (!nFrameLength) {
		hddLog(LOGE, FL("Frame Length is Invalid ZERO"));
		return;
	}

	if (NULL == pbFrames) {
		hddLog(LOGE, FL("pbFrames is NULL"));
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
		hddLog(LOGE, FL("Frame type %d and subtype %d are not valid"),
		       type, subType);
		return;
	}
}
#endif

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
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

	hddLog(LOG1, "TSM Ind tid(%d) state(%d) MeasInt(%d)",
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
	hddLog(LOG1,
	       "CCXPREAUTHNOTIFY=" MAC_ADDRESS_STR " %d:%d",
	       MAC_ADDR_ARRAY(pRoamInfo->bssid.bytes),
	       pRoamInfo->timestamp[0], pRoamInfo->timestamp[1]);

	nBytes = snprintf(pos, freeBytes, "CCXPREAUTHNOTIFY=");
	pos += nBytes;
	freeBytes -= nBytes;

	cdf_mem_copy(pos, pRoamInfo->bssid.bytes, CDF_MAC_ADDR_SIZE);
	pos += CDF_MAC_ADDR_SIZE;
	freeBytes -= CDF_MAC_ADDR_SIZE;

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

	hddLog(LOG1, "CCXADJAPREP=%u", pRoamInfo->tsmRoamDelay);

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

	hddLog(LOG1, FL("CCXBCNREP=%d %d %d"), measurementToken,
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
		hddLog(LOG1,
		       "Measurement Done but no scan results");
		/* If the measurement is none and no scan results found,
		   indicate the supplicant about measurement done */
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
				hddLog(LOG1, "i(%d) sizeof bcnReportFields(%d) IeLength(%d) Length of Ie(%d) totLen(%d)",
				       i, bcnRepFieldSize, 1,
				       pRoamInfo->pEseBcnReportRsp->
				       bcnRepBssInfo[i].ieLen, tot_bcn_ieLen);
			}

			hddLog(LOG1, "Sending %d BSS Info",
			       sendBss);
			hddLog(LOG1, "CCXBCNREP=%d %d %d %d",
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
			cdf_mem_copy(pos, (char *)&tot_bcn_ieLen,
				     sizeof(tot_bcn_ieLen));
			pos += sizeof(tot_bcn_ieLen);
			freeBytes -= sizeof(tot_bcn_ieLen);

			for (i = 0; i < sendBss; i++) {
				hddLog(LOG1,
				       "ChanNum(%d) Spare(%d) MeasDuration(%d)"
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
				cdf_mem_copy(pos,
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
				cdf_mem_copy(pos, (char *)&len, sizeof(len));
				pos += sizeof(len);
				freeBytes -= sizeof(len);

				/* copy IE from scan results */
				cdf_mem_copy(pos,
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

#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */

/**
 * hdd_sme_roam_callback() - hdd sme roam callback
 * @pContext: pointer to adapter context
 * @pRoamInfo: pointer to roam info
 * @roamId: roam id
 * @roamStatus: roam status
 * @roamResult: roam result
 *
 * Return: CDF_STATUS enumeration
 */
CDF_STATUS
hdd_sme_roam_callback(void *pContext, tCsrRoamInfo *pRoamInfo, uint32_t roamId,
		      eRoamCmdStatus roamStatus, eCsrRoamResult roamResult)
{
	CDF_STATUS cdf_ret_status = CDF_STATUS_SUCCESS;
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) pContext;
	hdd_wext_state_t *pWextState = NULL;
	hdd_station_ctx_t *pHddStaCtx = NULL;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	hdd_context_t *pHddCtx = NULL;

	hddLog(LOG2,
		  "CSR Callback: status= %d result= %d roamID=%d",
		  roamStatus, roamResult, roamId);

	/* Sanity check */
	if ((NULL == pAdapter) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		hddLog(LOGP, "invalid adapter or adapter has invalid magic");
		return CDF_STATUS_E_FAILURE;
	}

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	switch (roamStatus) {
	case eCSR_ROAM_SESSION_OPENED:
		set_bit(SME_SESSION_OPENED, &pAdapter->event_flags);
		complete(&pAdapter->session_open_comp_var);
		break;

#if  defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
defined(FEATURE_WLAN_LFR)
	/*
	 * We did pre-auth,then we attempted a 11r or ese reassoc.
	 * reassoc failed due to failure, timeout, reject from ap
	 * in any case tell the OS, our carrier is off and mark
	 * interface down.
	 */
	case eCSR_ROAM_FT_REASSOC_FAILED:
		hddLog(LOGE,
		       FL
		       ("Reassoc Failed with roamStatus: %d roamResult: %d SessionID: %d"),
		       roamStatus, roamResult, pAdapter->sessionId);
		cdf_ret_status =
			hdd_dis_connect_handler(pAdapter, pRoamInfo, roamId,
						roamStatus, roamResult);
		/*
		 * Check if Mcast/Bcast Filters are set, if yes
		 * clear the filters here.
		 */
		if ((WLAN_HDD_GET_CTX(pAdapter))->hdd_mcastbcast_filter_set ==
		    true) {
			(WLAN_HDD_GET_CTX(pAdapter))->
			hdd_mcastbcast_filter_set = false;
		}
		pHddStaCtx->ft_carrier_on = false;
		pHddStaCtx->hdd_ReassocScenario = false;
		hddLog(LOG1,
		       FL("hdd_ReassocScenario set to: %d, ReAssoc Failed, session: %d"),
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
	{
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(pAdapter, WLAN_NETIF_TX_DISABLE,
					   WLAN_CONTROL_PATH);
		status =
			hdd_roam_deregister_sta(pAdapter,
						pHddStaCtx->conn_info.
						staId[0]);
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			hddLog(LOGW,
				FL
				("hdd_roam_deregister_sta() failed to for staID %d. Status=%d [0x%x]"),
				pHddStaCtx->conn_info.staId[0],
				status, status);
			cdf_ret_status = CDF_STATUS_E_FAILURE;
		}
	}
		pHddStaCtx->ft_carrier_on = true;
		pHddStaCtx->hdd_ReassocScenario = true;
		hddLog(LOG1,
		       FL("hdd_ReassocScenario set to: %d, due to eCSR_ROAM_FT_START, session: %d"),
		       pHddStaCtx->hdd_ReassocScenario, pAdapter->sessionId);
		break;
#endif

	case eCSR_ROAM_SHOULD_ROAM:
		/* Dont need to do anything */
	{
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
		/* notify apps that we can't pass traffic anymore */
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_NETIF_TX_DISABLE,
					   WLAN_CONTROL_PATH);
#if  defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
defined(FEATURE_WLAN_LFR)
		if (pHddStaCtx->ft_carrier_on == false) {
#endif
			wlan_hdd_netif_queue_control(pAdapter,
					   WLAN_NETIF_CARRIER_OFF,
					   WLAN_CONTROL_PATH);
#if  defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
defined(FEATURE_WLAN_LFR)
	}
#endif

#if  !(defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
defined(FEATURE_WLAN_LFR))
		/*
		 * We should clear all sta register with TL, for now, only one.
		 */
		status =
			hdd_roam_deregister_sta(pAdapter,
						pHddStaCtx->conn_info.
						staId[0]);
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			hddLog(LOGW,
				FL
				("hdd_roam_deregister_sta() failed to for staID %d. Status=%d [0x%x]"),
				pHddStaCtx->conn_info.staId[0],
				status, status);
			cdf_ret_status = CDF_STATUS_E_FAILURE;
		}
#endif
	}
		break;
	case eCSR_ROAM_LOSTLINK:
		if (roamResult == eCSR_ROAM_RESULT_LOSTLINK) {
			hddLog(LOG2, "Roaming started due to connection lost");
			hddLog(LOG1, FL("Disabling queues"));
			wlan_hdd_netif_queue_control(pAdapter,
					WLAN_NETIF_TX_DISABLE_N_CARRIER,
					WLAN_CONTROL_PATH);
			break;
		}
	case eCSR_ROAM_DISASSOCIATED:
	{
		hddLog(LOG1, "****eCSR_ROAM_DISASSOCIATED****");
		cdf_ret_status =
			hdd_dis_connect_handler(pAdapter, pRoamInfo, roamId,
						roamStatus, roamResult);
		/* Check if Mcast/Bcast Filters are set, if yes clear the filters here */
		pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
		if (pHddCtx->hdd_mcastbcast_filter_set == true) {
			hdd_conf_mcastbcast_filter(pHddCtx, false);

			if (true ==
			    pHddCtx->sus_res_mcastbcast_filter_valid) {
				pHddCtx->configuredMcastBcastFilter =
					pHddCtx->sus_res_mcastbcast_filter;
				pHddCtx->
				sus_res_mcastbcast_filter_valid =
					false;
			}

			hddLog(LOG1,
			       "offload: disassociation happening, restoring configuredMcastBcastFilter");
			hddLog(LOG1,
			       "McastBcastFilter = %d",
			       pHddCtx->configuredMcastBcastFilter);
			hddLog(LOG1,
			       "offload: already called mcastbcast filter");
			(WLAN_HDD_GET_CTX(pAdapter))->
			hdd_mcastbcast_filter_set = false;
		}
		/* Call to clear any MC Addr List filter applied after
		 * successful connection.
		 */
		wlan_hdd_set_mc_addr_list(pAdapter, false);
	}
	break;
	case eCSR_ROAM_IBSS_LEAVE:
		hddLog(LOG1, "****eCSR_ROAM_IBSS_LEAVE****");
		cdf_ret_status =
			hdd_dis_connect_handler(pAdapter, pRoamInfo, roamId,
						roamStatus, roamResult);
		break;
	case eCSR_ROAM_ASSOCIATION_COMPLETION:
		hddLog(LOG1, "****eCSR_ROAM_ASSOCIATION_COMPLETION****");
		/*
		 * To Do - address probable memory leak with WEP encryption upon
		 * successful association.
		 */
		if (eCSR_ROAM_RESULT_ASSOCIATED != roamResult) {
			/* Clear saved connection information in HDD */
			hdd_conn_remove_connect_info(
				WLAN_HDD_GET_STATION_CTX_PTR(pAdapter));
		}
		cdf_ret_status =
			hdd_association_completion_handler(pAdapter, pRoamInfo,
							   roamId, roamStatus,
							   roamResult);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (pRoamInfo)
			pRoamInfo->roamSynchInProgress = false;
#endif
		break;
	case eCSR_ROAM_ASSOCIATION_FAILURE:
		cdf_ret_status = hdd_association_completion_handler(pAdapter,
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
		cdf_ret_status =
			roam_roam_connect_status_update_handler(pAdapter,
								pRoamInfo,
								roamId,
								roamStatus,
								roamResult);
		break;

	case eCSR_ROAM_MIC_ERROR_IND:
		cdf_ret_status =
			hdd_roam_mic_error_indication_handler(pAdapter,
							      pRoamInfo,
							      roamId,
							      roamStatus,
							      roamResult);
		break;

	case eCSR_ROAM_SET_KEY_COMPLETE:
	{
		cdf_ret_status =
			hdd_roam_set_key_complete_handler(pAdapter, pRoamInfo,
							  roamId, roamStatus,
							  roamResult);
		if (eCSR_ROAM_RESULT_AUTHENTICATED == roamResult) {
			pHddStaCtx->hdd_ReassocScenario = false;
			hddLog(LOG1,
			       FL("hdd_ReassocScenario set to: %d, set key complete, session: %d"),
			       pHddStaCtx->hdd_ReassocScenario,
			       pAdapter->sessionId);
		}
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (pRoamInfo != NULL)
			pRoamInfo->roamSynchInProgress = false;
#endif
		break;
#ifdef WLAN_FEATURE_VOWIFI_11R
	case eCSR_ROAM_FT_RESPONSE:
		hdd_send_ft_event(pAdapter);
		break;
#endif
#if defined(FEATURE_WLAN_LFR) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
	case eCSR_ROAM_PMK_NOTIFY:
		if (eCSR_AUTH_TYPE_RSN == pHddStaCtx->conn_info.authType ||
			eCSR_AUTH_TYPE_RSN_8021X_SHA256 ==
					pHddStaCtx->conn_info.authType) {
			/* notify the supplicant of a new candidate */
			cdf_ret_status =
				wlan_hdd_cfg80211_pmksa_candidate_notify(
						pAdapter, pRoamInfo, 1, false);
		}
		break;
#endif

#ifdef FEATURE_WLAN_LFR_METRICS
	case eCSR_ROAM_PREAUTH_INIT_NOTIFY:
		/* This event is to notify pre-auth initiation */
		if (CDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth(pAdapter,
							   pRoamInfo)) {
			cdf_ret_status = CDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_PREAUTH_STATUS_SUCCESS:
		/*
		 * This event will notify pre-auth completion in case of success
		 */
		if (CDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth_status(pAdapter,
							 pRoamInfo, 1)) {
			cdf_ret_status = CDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_PREAUTH_STATUS_FAILURE:
		/*
		 * This event will notify pre-auth completion incase of failure.
		 */
		if (CDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth_status(pAdapter,
								pRoamInfo, 0)) {
			cdf_ret_status = CDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_HANDOVER_SUCCESS:
		/* This event is to notify handover success.
		   It will be only invoked on success */
		if (CDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_handover(pAdapter,
							    pRoamInfo)) {
			cdf_ret_status = CDF_STATUS_E_FAILURE;
		}
		break;
#endif

	case eCSR_ROAM_INDICATE_MGMT_FRAME:
		hdd_indicate_mgmt_frame(pAdapter,
					pRoamInfo->nFrameLength,
					pRoamInfo->pbFrames,
					pRoamInfo->frameType,
					pRoamInfo->rxChan, pRoamInfo->rxRssi);
		break;
	case eCSR_ROAM_REMAIN_CHAN_READY:
		hdd_remain_chan_ready_handler(pAdapter, pRoamInfo->roc_scan_id);
		break;
	case eCSR_ROAM_SEND_ACTION_CNF:
		hdd_send_action_cnf(pAdapter,
				    (roamResult ==
				     eCSR_ROAM_RESULT_NONE) ? true : false);
		break;
#ifdef FEATURE_WLAN_TDLS
	case eCSR_ROAM_TDLS_STATUS_UPDATE:
		cdf_ret_status =
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
#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
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
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */
	default:
		break;
	}
	return cdf_ret_status;
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
	} else
#ifdef WLAN_FEATURE_VOWIFI_11R
	if (memcmp(auth_suite, ccp_rsn_oui04, 4) == 0) {
		/* Check for 11r FT Authentication with PSK */
		auth_type = eCSR_AUTH_TYPE_FT_RSN_PSK;
	} else if (memcmp(auth_suite, ccp_rsn_oui03, 4) == 0) {
		/* Check for 11R FT Authentication with 802.1X */
		auth_type = eCSR_AUTH_TYPE_FT_RSN;
	} else
#endif
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
	hddLog(LOG1, FL("auth_type: %d"), auth_type);
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

	hddLog(LOG1, FL("cipher_type: %d"), cipher_type);
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

	hddLog(LOG1, FL("cipher_type: %d"), cipher_type);
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
	CDF_STATUS result;
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
			hddLog(LOGE, FL("Invalid DOT11F RSN IE length :%d"),
			       gen_ie_len);
			return -EINVAL;
		}
		/* Skip past the EID byte and length byte */
		pRsnIe = gen_ie + 2;
		RSNIeLen = gen_ie_len - 2;
		/* Unpack the RSN IE */
		dot11f_unpack_ie_rsn((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11RSNIE);
		/* Copy out the encryption and authentication types */
		hddLog(LOG1, FL("pairwise cipher suite count: %d"),
		       dot11RSNIE.pwise_cipher_suite_count);
		hddLog(LOG1, FL("authentication suite count: %d"),
		       dot11RSNIE.akm_suite_count);
		/*Here we have followed the apple base code,
		   but probably I suspect we can do something different */
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
				hddLog(LOGE, FL("MAC address is all zeroes"));
				break;
			}
			updatePMKCache = true;
			/*
			 * For right now, I assume setASSOCIATE() has passed
			 * in the bssid.
			 */
			cdf_mem_copy(PMKIDCache[i].BSSID.bytes,
				     bssid, ETHER_ADDR_LEN);
			cdf_mem_copy(PMKIDCache[i].PMKID,
				     dot11RSNIE.pmkid[i], CSR_RSN_PMKID_SIZE);
		}

		if (updatePMKCache) {
			/*
			 * Calling csr_roam_set_pmkid_cache to configure the
			 * PMKIDs into the cache.
			 */
			hddLog(LOG1,
				FL("Calling sme_roam_set_pmkid_cache with cache entry %d."),
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
			hddLog(LOGE, FL("Invalid DOT11F WPA IE length :%d"),
			       gen_ie_len);
			return -EINVAL;
		}
		/* Skip past the EID and length byte - and four byte WiFi OUI */
		pRsnIe = gen_ie + 2 + 4;
		RSNIeLen = gen_ie_len - (2 + 4);
		/* Unpack the WPA IE */
		dot11f_unpack_ie_wpa((tpAniSirGlobal) halHandle,
				     pRsnIe, RSNIeLen, &dot11WPAIE);
		/* Copy out the encryption and authentication types */
		hddLog(LOG1, FL("WPA unicast cipher suite count: %d"),
		       dot11WPAIE.unicast_cipher_count);
		hddLog(LOG1, FL("WPA authentication suite count: %d"),
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
		hddLog(LOGW, FL("gen_ie[0]: %d"), gen_ie[0]);
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
	cdf_mem_copy(bssid,
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

		if ((WLAN_HDD_IBSS == pAdapter->device_mode) &&
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
		hddLog(LOG1, FL("RSNMfpRequired = %d, RSNMfpCapable = %d"),
						RSNMfpRequired, RSNMfpCapable);
		pWextState->roamProfile.MFPRequired = RSNMfpRequired;
		pWextState->roamProfile.MFPCapable = RSNMfpCapable;
#endif
		hddLog(LOG1,
		       FL("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d"),
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
	ENTER();

	pRoamProfile->AuthType.numEntries = 1;
	hddLog(LOG1, FL("pHddStaCtx->conn_info.authType = %d"),
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
				hddLog(LOG1,
				       FL("set authType to CCKM WPA. AKM also 802.1X."));
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_WPA;
			} else if (RSNAuthType == eCSR_AUTH_TYPE_CCKM_WPA) {
				hddLog(LOG1,
				       FL("Last chance to set authType to CCKM WPA."));
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
				hddLog(LOG1,
				       FL("set authType to CCKM RSN. AKM also 802.1X."));
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_RSN;
			} else if (RSNAuthType == eCSR_AUTH_TYPE_CCKM_RSN) {
				hddLog(LOG1,
				       FL("Last chance to set authType to CCKM RSN."));
				pRoamProfile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_RSN;
			} else
#endif

#ifdef WLAN_FEATURE_VOWIFI_11R
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
#endif

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
		hddLog(LOG1, FL("In default, unknown auth type."));
#endif /* FEATURE_WLAN_ESE */
		pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
		break;
	}

	hddLog(LOG1, FL("Set roam Authtype to %d"),
	       pWextState->roamProfile.AuthType.authType[0]);

	EXIT();
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
	unsigned long rc;
	uint32_t status = 0;
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	uint32_t roamId;
	tCsrRoamProfile *pRoamProfile;
	eMib_dot11DesiredBssType connectedBssType;
	eCsrAuthType RSNAuthType;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (pAdapter->device_mode != WLAN_HDD_INFRA_STATION &&
		pAdapter->device_mode != WLAN_HDD_P2P_CLIENT) {
		hddLog(LOGW, FL("device mode %s(%d) is not allowed"),
			hdd_device_mode_to_string(pAdapter->device_mode),
			pAdapter->device_mode);
		return -EINVAL;
	}

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	if (pWextState->mTKIPCounterMeasures == TKIP_COUNTER_MEASURE_STARTED) {
		hddLog(LOG2, FL("Counter measure is in progress"));
		return -EBUSY;
	}
	if (SIR_MAC_MAX_SSID_LENGTH < wrqu->essid.length)
		return -EINVAL;

	pRoamProfile = &pWextState->roamProfile;
	if (hdd_conn_get_connected_bss_type(pHddStaCtx, &connectedBssType) ||
	    (eMib_dot11DesiredBssType_independent ==
	     pHddStaCtx->conn_info.connDot11DesiredBssType)) {
		CDF_STATUS cdf_status;

		/* Need to issue a disconnect to CSR. */
		INIT_COMPLETION(pAdapter->disconnect_comp_var);
		cdf_status = sme_roam_disconnect(hHal, pAdapter->sessionId,
					 eCSR_DISCONNECT_REASON_UNSPECIFIED);

		if (CDF_STATUS_SUCCESS == cdf_status) {
			rc = wait_for_completion_timeout(&pAdapter->
						 disconnect_comp_var,
						 msecs_to_jiffies
						 (WLAN_WAIT_TIME_DISCONNECT));
			if (!rc)
				hddLog(LOGE, FL("Disconnect event timed out"));
		}
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
	if (CDF_STATUS_SUCCESS != status)
		pWextState->roamProfile.uapsd_mask = 0;

	pWextState->roamProfile.SSIDs.numOfSSIDs = 1;

	pWextState->roamProfile.SSIDs.SSIDList->SSID.length =
		wrqu->essid.length;

	cdf_mem_zero(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId,
		     sizeof(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId));
	cdf_mem_copy((void *)(pWextState->roamProfile.SSIDs.SSIDList->SSID.
			      ssId), extra, wrqu->essid.length);
	if (IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion
	    || IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion) {

		/* set gen ie */
		hdd_set_genie_to_csr(pAdapter, &RSNAuthType);

		/* set auth */
		hdd_set_csr_auth_type(pAdapter, RSNAuthType);
	}
#ifdef FEATURE_WLAN_WAPI
	hddLog(LOG1, FL("Setting WAPI AUTH Type and Encryption Mode values"));
	if (pAdapter->wapi_info.nWapiMode) {
		switch (pAdapter->wapi_info.wapiAuthMode) {
		case WAPI_AUTH_MODE_PSK:
		{
			hddLog(LOG1, FL("WAPI AUTH TYPE: PSK: %d"),
			       pAdapter->wapi_info.wapiAuthMode);
			pRoamProfile->AuthType.numEntries = 1;
			pRoamProfile->AuthType.authType[0] =
				eCSR_AUTH_TYPE_WAPI_WAI_PSK;
			break;
		}
		case WAPI_AUTH_MODE_CERT:
		{
			hddLog(LOG1, FL("WAPI AUTH TYPE: CERT: %d"),
			       pAdapter->wapi_info.wapiAuthMode);
			pRoamProfile->AuthType.numEntries = 1;
			pRoamProfile->AuthType.authType[0] =
				eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
			break;
		}
		} /* End of switch */
		if (pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_PSK ||
		    pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_CERT) {
			hddLog(LOG1, FL("WAPI PAIRWISE/GROUP ENCRYPTION: WPI"));
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
		hdd_select_cbmode(pAdapter,
				  (WLAN_HDD_GET_CTX(pAdapter))->config->
				  AdHocChannel5G);
	}
	status = sme_roam_connect(hHal, pAdapter->sessionId,
				  &(pWextState->roamProfile), &roamId);
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

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
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

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
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
			hddLog(LOGW, FL("value %d UNKNOWN IW_AUTH_CIPHER"),
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
			hddLog(LOGW, FL("value %d UNKNOWN IW_AUTH_CIPHER"),
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
		/* Save the auth algo here and set auth type to SME Roam profile
		   in the iw_set_ap_address */
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
			hddLog(LOG1, FL("CCKM AKM Set %d"), wrqu->param.value);
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
			/*
			 * This is test code. I need to actually KNOW whether
			 * this is an RSN Assoc or WPA.
			 */
			pWextState->collectedAuthType =
				eCSR_AUTH_TYPE_CCKM_RSN;
		} else if (wrqu->param.value & IW_AUTH_KEY_MGMT_PSK) {
			/* Save the key management */
			pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_PSK;
			pWextState->collectedAuthType =
				eCSR_AUTH_TYPE_RSN;
		} else
		if (!(wrqu->param.value & IW_AUTH_KEY_MGMT_802_1X)) {
			pWextState->collectedAuthType = eCSR_AUTH_TYPE_NONE;
			/* Save the key management anyway */
			pWextState->authKeyMgmt = wrqu->param.value;
		} else {                /* It must be IW_AUTH_KEY_MGMT_802_1X */
			/* Save the key management */
			pWextState->authKeyMgmt |=
				IW_AUTH_KEY_MGMT_802_1X;
			pWextState->collectedAuthType =
				eCSR_AUTH_TYPE_RSN;
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
			hddLog(LOG2,
			       "Counter Measure started %d",
			       wrqu->param.value);
			pWextState->mTKIPCounterMeasures =
				TKIP_COUNTER_MEASURE_STARTED;
		} else {
			hddLog(LOG2,
			       "Counter Measure stopped=%d",
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

		hddLog(LOGW, FL("called with unsupported auth type %d"),
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

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
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
#ifdef WLAN_FEATURE_VOWIFI_11R
	case eCSR_AUTH_TYPE_FT_RSN:
#endif
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
		hddLog(LOG1, FL("called with unknown auth type"));
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_WPA_PSK:
		hddLog(LOG1, FL("called with WPA PSK auth type"));
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		return -EIO;
#ifdef WLAN_FEATURE_VOWIFI_11R
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
#endif
	case eCSR_AUTH_TYPE_RSN_PSK:
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
#endif
		hddLog(LOG1, FL("called with RSN PSK auth type"));
		wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
		return -EIO;
	default:
		hddLog(LOGE, FL("called with unknown auth type"));
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
			hddLog(LOG1, FL("called with unknown auth type %d"),
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
			hddLog(LOG1, FL("called with unknown auth type %d"),
			       pRoamProfile->negotiatedMCEncryptionType);
			return -EIO;
		}
	}

	hddLog(LOG1, FL("called with auth type %d"),
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

	ENTER();

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pMacAddress = (uint8_t *) wrqu->ap_addr.sa_data;
	hddLog(LOG1, FL(" " MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pMacAddress));
	cdf_mem_copy(pHddStaCtx->conn_info.bssId.bytes, pMacAddress,
		     sizeof(struct cdf_mac_addr));
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

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (pHddStaCtx->conn_info.connState == eConnectionState_Associated ||
	    eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState) {
		cdf_mem_copy(wrqu->ap_addr.sa_data,
				pHddStaCtx->conn_info.bssId.bytes,
				CDF_MAC_ADDR_SIZE);
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
