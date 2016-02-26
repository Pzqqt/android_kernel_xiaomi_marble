/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_ASSOC_H__)
#define WLAN_HDD_ASSOC_H__

/**
 * DOC: wlan_hdd_assoc.h
 *
 */

/* Include files */
#include <wlan_hdd_mib.h>
#include <sme_api.h>
#include <wlan_defs.h>
#include "ol_txrx_ctrl_api.h"

/* Preprocessor Definitions and Constants */
#ifdef FEATURE_WLAN_TDLS
#define HDD_MAX_NUM_TDLS_STA          8
#define HDD_MAX_NUM_TDLS_STA_P_UAPSD_OFFCHAN  1
#define TDLS_STA_INDEX_VALID(staId) \
	(((staId) >= 1) && ((staId) < 0xFF))
#endif
#define TKIP_COUNTER_MEASURE_STARTED 1
#define TKIP_COUNTER_MEASURE_STOPED  0
/* Timeout (in ms) for Link to Up before Registering Station */
#define ASSOC_LINKUP_TIMEOUT 60

/* In pronto case, IBSS owns the first peer for bss peer.
   In Rome case, IBSS uses the 2nd peer as bss peer */
#define IBSS_BROADCAST_STAID 1

/* Type Declarations */
/**
 * typedef eConnectionState - Connection states
 * @eConnectionState_NotConnected: Not associated in Infra or participating
 *			in an IBSS / Ad-hoc network
 * @eConnectionState_Connecting: While connection in progress
 * @eConnectionState_Associated: Associated in an Infrastructure network
 * @eConnectionState_IbssDisconnected: Participating in an IBSS network though
 *			disconnected (no partner stations in the IBSS)
 * @eConnectionState_IbssConnected: Participating in an IBSS network with
 *			partner stations also present
 * eConnectionState_Disconnecting: Disconnecting in an Infrastructure network
 */
typedef enum {
	eConnectionState_NotConnected,
	eConnectionState_Connecting,
	eConnectionState_Associated,
	eConnectionState_IbssDisconnected,
	eConnectionState_IbssConnected,
	eConnectionState_Disconnecting
} eConnectionState;

/**
 * typedef ePeerStatus - Peer status
 * @ePeerConnected: peer connected
 * @ePeerDisconnected: peer disconnected
 */
typedef enum {
	ePeerConnected = 1,
	ePeerDisconnected
} ePeerStatus;

/**
 * typedef connection_info_t - structure to store connection information
 * @connState: connection state of the NIC
 * @connDot11DesiredBssType: BSS type of the current connection.
 *		Comes from the MIB at the time the connect request is issued
 *		in combination with the BssDescription from the
 *		associated entity
 * @bssId: BSSID
 * @SSID: SSID Info
 * @staId: Station ID
 * @peerMacAddress:Peer Mac Address of the IBSS Stations
 * @authType: Auth Type
 * @ucEncryptionType: Unicast Encryption Type
 * @mcEncryptionType: Multicast Encryption Type
 * @Keys: Keys
 * @operationChannel: Operation Channel
 * @uIsAuthenticated: Remembers authenticated state
 * @dot11Mode: dot11Mode
 * @proxyARPService: proxy arp service
 * @ptk_installed: ptk installed state
 * @gtk_installed: gtk installed state
 * @nss: number of spatial streams negotiated
 * @rate_flags: rate flags for current connection
 */
typedef struct connection_info_s {
	eConnectionState connState;
	eMib_dot11DesiredBssType connDot11DesiredBssType;
	struct cdf_mac_addr bssId;
	tCsrSSIDInfo SSID;
	uint8_t staId[MAX_IBSS_PEERS];
	struct cdf_mac_addr peerMacAddress[MAX_IBSS_PEERS];
	eCsrAuthType authType;
	eCsrEncryptionType ucEncryptionType;
	eCsrEncryptionType mcEncryptionType;
	tCsrKeys Keys;
	uint8_t operationChannel;
	uint8_t uIsAuthenticated;
	uint32_t dot11Mode;
	uint8_t proxyARPService;
	bool ptk_installed;
	bool gtk_installed;
	uint8_t nss;
	uint32_t rate_flags;
} connection_info_t;

/* Forward declarations */
typedef struct hdd_adapter_s hdd_adapter_t;
typedef struct hdd_context_s hdd_context_t;
typedef struct hdd_station_ctx hdd_station_ctx_t;
typedef struct hdd_ap_ctx_s hdd_ap_ctx_t;
typedef struct hdd_mon_ctx_s hdd_mon_ctx_t;

/**
 * hdd_is_connecting() - Function to check connection progress
 * @hdd_sta_ctx:    pointer to global HDD Station context
 *
 * Return: true if connecting, false otherwise
 */
bool hdd_is_connecting(hdd_station_ctx_t *hdd_sta_ctx);

/**
 * hdd_conn_is_connected() - Function to check connection status
 * @pHddStaCtx:    pointer to global HDD Station context
 *
 * Return: false if any errors encountered, true otherwise
 */
bool hdd_conn_is_connected(hdd_station_ctx_t *pHddStaCtx);

/**
 * hdd_conn_get_connected_band() - get current connection radio band
 * @pHddStaCtx:    pointer to global HDD Station context
 *
 * Return: eCSR_BAND_24 or eCSR_BAND_5G based on current AP connection
 *      eCSR_BAND_ALL if not connected
 */
eCsrBand hdd_conn_get_connected_band(hdd_station_ctx_t *pHddStaCtx);

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
CDF_STATUS hdd_sme_roam_callback(void *pContext, tCsrRoamInfo *pRoamInfo,
				 uint32_t roamId,
				 eRoamCmdStatus roamStatus,
				 eCsrRoamResult roamResult);

/**
 * hdd_conn_get_connected_bss_type() - get current bss type
 * @pHddStaCtx:         pointer to global HDD Station context
 * @pConnectedBssType:  pointer to connected bss type
 *
 * Return: false if any errors encountered, true otherwise
 */
bool
hdd_conn_get_connected_bss_type(hdd_station_ctx_t *pHddStaCtx,
				eMib_dot11DesiredBssType *pConnectedBssType);

/**
 * hdd_set_genie_to_csr() - set genie to csr
 * @pAdapter: pointer to adapter
 * @RSNAuthType: pointer to auth type
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_set_genie_to_csr(hdd_adapter_t *pAdapter, eCsrAuthType *RSNAuthType);

/**
 * hdd_set_csr_auth_type() - set csr auth type
 * @pAdapter: pointer to adapter
 * @RSNAuthType: auth type
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_set_csr_auth_type(hdd_adapter_t *pAdapter, eCsrAuthType RSNAuthType);

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
				     uint8_t ucastSig);

/**
 * hdd_perform_roam_set_key_complete() - perform set key complete
 * @pAdapter: pointer to adapter
 *
 * Return: none
 */
void hdd_perform_roam_set_key_complete(hdd_adapter_t *pAdapter);

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
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
					    const bool flag,
					    const uint8_t numBss);
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */

CDF_STATUS hdd_change_peer_state(hdd_adapter_t *pAdapter,
				 uint8_t sta_id,
				 enum ol_txrx_peer_state sta_state,
				 bool roam_synch_in_progress);
#endif
