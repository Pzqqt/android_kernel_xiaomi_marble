/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
#include <wlan_hdd_green_ap.h>
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
#include "wlan_hdd_tsf.h"
#include "wlan_utility.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_ipa_ucfg_api.h"
#include "wlan_hdd_stats.h"
#include "wlan_hdd_scan.h"
#include "wlan_crypto_global_api.h"
#include "wlan_hdd_bcn_recv.h"

#include "wlan_hdd_nud_tracking.h"
#include <wlan_cfg80211_crypto.h>
#include <wlan_crypto_global_api.h>
#include "wlan_blm_ucfg_api.h"
#include "wlan_hdd_sta_info.h"

#include <ol_defines.h>

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

/* AES-GCMP-128 */
uint8_t ccp_rsn_oui09[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x08 };

/* AES-GCMP-256 */
uint8_t ccp_rsn_oui0a[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x09 };
#ifdef WLAN_FEATURE_FILS_SK
uint8_t ccp_rsn_oui_0e[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x0E};
uint8_t ccp_rsn_oui_0f[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x0F};
uint8_t ccp_rsn_oui_10[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x10};
uint8_t ccp_rsn_oui_11[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x11};
#endif
uint8_t ccp_rsn_oui_12[HDD_RSN_OUI_SIZE] = {0x50, 0x6F, 0x9A, 0x02};
uint8_t ccp_rsn_oui_0b[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x0B};
uint8_t ccp_rsn_oui_0c[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x0C};
/* FT-SUITE-B AKM */
uint8_t ccp_rsn_oui_0d[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x0D};

/* OWE https://tools.ietf.org/html/rfc8110 */
uint8_t ccp_rsn_oui_18[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x12};

#ifdef WLAN_FEATURE_SAE
/* SAE AKM */
uint8_t ccp_rsn_oui_80[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x08};
/* FT SAE AKM */
uint8_t ccp_rsn_oui_90[HDD_RSN_OUI_SIZE] = {0x00, 0x0F, 0xAC, 0x09};
#endif
static const
u8 ccp_rsn_oui_13[HDD_RSN_OUI_SIZE] = {0x50, 0x6F, 0x9A, 0x01};

/* Offset where the EID-Len-IE, start. */
#define FT_ASSOC_RSP_IES_OFFSET 6  /* Capability(2) + AID(2) + Status Code(2) */
#define FT_ASSOC_REQ_IES_OFFSET 4  /* Capability(2) + LI(2) */

#define HDD_PEER_AUTHORIZE_WAIT 10

/**
 * beacon_filter_table - table of IEs used for beacon filtering
 */
static const int beacon_filter_table[] = {
	WLAN_ELEMID_DSPARMS,
	WLAN_ELEMID_ERP,
	WLAN_ELEMID_EDCAPARMS,
	WLAN_ELEMID_QOS_CAPABILITY,
	WLAN_ELEMID_HTINFO_ANA,
	WLAN_ELEMID_OP_MODE_NOTIFY,
	WLAN_ELEMID_VHTOP,
#ifdef WLAN_FEATURE_11AX_BSS_COLOR
	/*
	 * EID: 221 vendor IE is being used temporarily by 11AX
	 * bss-color-change IE till it gets any fixed number. This
	 * vendor EID needs to be replaced with bss-color-change IE
	 * number.
	 */
	WLAN_ELEMID_VENDOR,
#endif
};

#if defined(WLAN_FEATURE_SAE) && \
		defined(CFG80211_EXTERNAL_AUTH_SUPPORT)
/**
 * wlan_hdd_sae_callback() - Sends SAE info to supplicant
 * @adapter: pointer adapter context
 * @roam_info: pointer to roam info
 *
 * This API is used to send required SAE info to trigger SAE in supplicant.
 *
 * Return: None
 */
static void wlan_hdd_sae_callback(struct hdd_adapter *adapter,
				  struct csr_roam_info *roam_info)
{
	struct hdd_context *hdd_ctx = adapter->hdd_ctx;
	int flags;
	struct sir_sae_info *sae_info = roam_info->sae_info;
	struct cfg80211_external_auth_params params = {0};

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!sae_info) {
		hdd_err("SAE info in NULL");
		return;
	}

	flags = cds_get_gfp_flags();

	params.key_mgmt_suite = 0x00;
	params.key_mgmt_suite |= 0x0F << 8;
	params.key_mgmt_suite |= 0xAC << 16;
	params.key_mgmt_suite |= 0x8 << 24;

	params.action = NL80211_EXTERNAL_AUTH_START;
	qdf_mem_copy(params.bssid, sae_info->peer_mac_addr.bytes,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(params.ssid.ssid, sae_info->ssid.ssId,
		     sae_info->ssid.length);
	params.ssid.ssid_len = sae_info->ssid.length;

	cfg80211_external_auth_request(adapter->dev, &params, flags);
	hdd_debug("SAE: sent cmd");
}
#else
static inline void wlan_hdd_sae_callback(struct hdd_adapter *adapter,
					 struct csr_roam_info *roam_info)
{ }
#endif

/**
 * hdd_conn_set_authenticated() - set authentication state
 * @adapter: pointer to the adapter
 * @auth_state: authentication state
 *
 * This function updates the global HDD station context
 * authentication state.
 *
 * Return: none
 */
static void
hdd_conn_set_authenticated(struct hdd_adapter *adapter, uint8_t auth_state)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	char *auth_time;
	uint32_t time_buffer_size;

	/* save the new connection state */
	hdd_debug("Authenticated state Changed from oldState:%d to State:%d",
		   sta_ctx->conn_info.is_authenticated, auth_state);
	sta_ctx->conn_info.is_authenticated = auth_state;

	auth_time = sta_ctx->conn_info.auth_time;
	time_buffer_size = sizeof(sta_ctx->conn_info.auth_time);

	if (auth_state)
		qdf_get_time_of_the_day_in_hr_min_sec_usec(auth_time,
							   time_buffer_size);
	else
		qdf_mem_zero(auth_time, time_buffer_size);

}

void hdd_conn_set_connection_state(struct hdd_adapter *adapter,
				   eConnectionState conn_state)
{
	struct hdd_station_ctx *hdd_sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	char *connect_time;
	uint32_t time_buffer_size;

	/* save the new connection state */
	hdd_debug("Changed conn state from old:%d to new:%d for dev %s",
		  hdd_sta_ctx->conn_info.conn_state, conn_state,
		  adapter->dev->name);

	hdd_tsf_notify_wlan_state_change(adapter,
					 hdd_sta_ctx->conn_info.conn_state,
					 conn_state);
	hdd_sta_ctx->conn_info.conn_state = conn_state;

	connect_time = hdd_sta_ctx->conn_info.connect_time;
	time_buffer_size = sizeof(hdd_sta_ctx->conn_info.connect_time);
	if (conn_state == eConnectionState_Associated)
		qdf_get_time_of_the_day_in_hr_min_sec_usec(connect_time,
							   time_buffer_size);
	else
		qdf_mem_zero(connect_time, time_buffer_size);

}

/**
 * hdd_conn_get_connection_state() - get connection state
 * @adapter: pointer to the adapter
 * @pConnState: pointer to connection state
 *
 * This function updates the global HDD station context connection state.
 *
 * Return: true if (Infra Associated or IBSS Connected)
 *	and sets output parameter pConnState;
 *	false otherwise
 */
static inline bool
hdd_conn_get_connection_state(struct hdd_station_ctx *sta_ctx,
			      eConnectionState *out_state)
{
	eConnectionState state = sta_ctx->conn_info.conn_state;

	if (out_state)
		*out_state = state;

	switch (state) {
	case eConnectionState_Associated:
	case eConnectionState_IbssConnected:
	case eConnectionState_IbssDisconnected:
	case eConnectionState_NdiConnected:
		return true;
	default:
		return false;
	}
}

bool hdd_is_connecting(struct hdd_station_ctx *hdd_sta_ctx)
{
	return hdd_sta_ctx->conn_info.conn_state ==
		eConnectionState_Connecting;
}

bool hdd_conn_is_connected(struct hdd_station_ctx *sta_ctx)
{
	return hdd_conn_get_connection_state(sta_ctx, NULL);
}

bool hdd_adapter_is_connected_sta(struct hdd_adapter *adapter)
{
	switch (adapter->device_mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_NDI_MODE:
		return hdd_conn_is_connected(&adapter->session.station);
	default:
		return false;
	}
}

enum band_info hdd_conn_get_connected_band(struct hdd_station_ctx *sta_ctx)
{
	uint32_t sta_freq = 0;

	if (eConnectionState_Associated == sta_ctx->conn_info.conn_state)
		sta_freq = sta_ctx->conn_info.chan_freq;

	if (wlan_reg_is_24ghz_ch_freq(sta_freq))
		return BAND_2G;
	else if (wlan_reg_is_5ghz_ch_freq(sta_freq))
		return BAND_5G;
	else   /* If station is not connected return as BAND_ALL */
		return BAND_ALL;
}

/**
 * hdd_conn_get_connected_cipher_algo() - get current connection cipher type
 * @sta_ctx: pointer to global HDD Station context
 * @pConnectedCipherAlgo: pointer to connected cipher algo
 *
 * Return: false if any errors encountered, true otherwise
 */
static inline bool
hdd_conn_get_connected_cipher_algo(struct hdd_station_ctx *sta_ctx,
				   eCsrEncryptionType *pConnectedCipherAlgo)
{
	bool connected = false;

	connected = hdd_conn_get_connection_state(sta_ctx, NULL);

	if (pConnectedCipherAlgo)
		*pConnectedCipherAlgo = sta_ctx->conn_info.uc_encrypt_type;

	return connected;
}

struct hdd_adapter *hdd_get_sta_connection_in_progress(
			struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter = NULL;
	struct hdd_station_ctx *hdd_sta_ctx;

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return NULL;
	}

	hdd_for_each_adapter(hdd_ctx, adapter) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if ((QDF_STA_MODE == adapter->device_mode) ||
		    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
		    (QDF_P2P_DEVICE_MODE == adapter->device_mode)) {
			if (eConnectionState_Connecting ==
			    hdd_sta_ctx->conn_info.conn_state) {
				hdd_debug("vdev_id %d: Connection is in progress",
					  adapter->vdev_id);
				return adapter;
			} else if ((eConnectionState_Associated ==
				   hdd_sta_ctx->conn_info.conn_state) &&
				   sme_is_sta_key_exchange_in_progress(
							hdd_ctx->mac_handle,
							adapter->vdev_id)) {
				hdd_debug("vdev_id %d: Key exchange is in progress",
					  adapter->vdev_id);
				return adapter;
			}
		}
	}
	return NULL;
}

void hdd_abort_ongoing_sta_connection(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *sta_adapter;
	QDF_STATUS status;

	sta_adapter = hdd_get_sta_connection_in_progress(hdd_ctx);
	if (sta_adapter) {
		hdd_debug("Disconnecting STA on vdev: %d",
			  sta_adapter->vdev_id);
		status = wlan_hdd_disconnect(sta_adapter,
					     eCSR_DISCONNECT_REASON_DEAUTH);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("wlan_hdd_disconnect failed, status: %d",
				status);
		}
	}
}

/**
 * hdd_remove_beacon_filter() - remove beacon filter
 * @adapter: Pointer to the hdd adapter
 *
 * Return: 0 on success and errno on failure
 */
static int hdd_remove_beacon_filter(struct hdd_adapter *adapter)
{
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	status = sme_remove_beacon_filter(hdd_ctx->mac_handle,
					  adapter->vdev_id);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_remove_beacon_filter() failed");
		return -EFAULT;
	}

	return 0;
}

int hdd_add_beacon_filter(struct hdd_adapter *adapter)
{
	int i;
	uint32_t ie_map[SIR_BCN_FLT_MAX_ELEMS_IE_LIST] = {0};
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	for (i = 0; i < ARRAY_SIZE(beacon_filter_table); i++)
		qdf_set_bit((beacon_filter_table[i]),
				(unsigned long int *)ie_map);

	status = sme_add_beacon_filter(hdd_ctx->mac_handle,
				       adapter->vdev_id, ie_map);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_add_beacon_filter() failed");
		return -EFAULT;
	}
	return 0;
}

void hdd_copy_ht_caps(struct ieee80211_ht_cap *hdd_ht_cap,
		      tDot11fIEHTCaps *roam_ht_cap)

{
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

void hdd_copy_vht_caps(struct ieee80211_vht_cap *hdd_vht_cap,
		       tDot11fIEVHTCaps *roam_vht_cap)
{
	uint32_t temp_vht_cap;

	qdf_mem_zero(hdd_vht_cap, sizeof(struct ieee80211_vht_cap));

	temp_vht_cap = roam_vht_cap->maxMPDULen & VHT_CAP_MAX_MPDU_LENGTH_MASK;
	hdd_vht_cap->vht_cap_info |= temp_vht_cap;
	temp_vht_cap = roam_vht_cap->supportedChannelWidthSet &
		(IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK >>
			VHT_CAP_SUPP_CHAN_WIDTH_MASK_SHIFT);
	if (temp_vht_cap) {
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
	}
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
static void hdd_copy_ht_operation(struct hdd_station_ctx *hdd_sta_ctx,
					    struct csr_roam_info *roam_info)
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
static void hdd_copy_vht_center_freq(struct ieee80211_vht_operation *ieee_ops,
				     tDot11fIEVHTOperation *roam_ops)
{
	ieee_ops->center_freq_seg0_idx = roam_ops->chan_center_freq_seg0;
	ieee_ops->center_freq_seg1_idx = roam_ops->chan_center_freq_seg1;
}
#else
static void hdd_copy_vht_center_freq(struct ieee80211_vht_operation *ieee_ops,
				     tDot11fIEVHTOperation *roam_ops)
{
	ieee_ops->center_freq_seg1_idx = roam_ops->chan_center_freq_seg0;
	ieee_ops->center_freq_seg2_idx = roam_ops->chan_center_freq_seg1;
}
#endif /* KERNEL_VERSION(4, 12, 0) */

/**
 * hdd_copy_vht_operation()- copy VHT operations element from roam info to
 *  hdd station context.
 * @hdd_sta_ctx: pointer to hdd station context
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_copy_vht_operation(struct hdd_station_ctx *hdd_sta_ctx,
					      struct csr_roam_info *roam_info)
{
	tDot11fIEVHTOperation *roam_vht_ops = &roam_info->vht_operation;
	struct ieee80211_vht_operation *hdd_vht_ops =
		&hdd_sta_ctx->conn_info.vht_operation;

	qdf_mem_zero(hdd_vht_ops, sizeof(struct ieee80211_vht_operation));

	hdd_vht_ops->chan_width = roam_vht_ops->chanWidth;
	hdd_copy_vht_center_freq(hdd_vht_ops, roam_vht_ops);
	hdd_vht_ops->basic_mcs_set = roam_vht_ops->basicMCSSet;
}


/**
 * hdd_save_bss_info() - save connection info in hdd sta ctx
 * @adapter: Pointer to adapter
 * @roam_info: pointer to roam info
 *
 * Return: None
 */
static void hdd_save_bss_info(struct hdd_adapter *adapter,
						struct csr_roam_info *roam_info)
{
	struct hdd_station_ctx *hdd_sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (roam_info->vht_caps.present) {
		hdd_sta_ctx->conn_info.conn_flag.vht_present = true;
		hdd_copy_vht_caps(&hdd_sta_ctx->conn_info.vht_caps,
				  &roam_info->vht_caps);
	} else {
		hdd_sta_ctx->conn_info.conn_flag.vht_present = false;
	}
	if (roam_info->ht_caps.present) {
		hdd_sta_ctx->conn_info.conn_flag.ht_present = true;
		hdd_copy_ht_caps(&hdd_sta_ctx->conn_info.ht_caps,
				 &roam_info->ht_caps);
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
	/* Cache last connection info */
	qdf_mem_copy(&hdd_sta_ctx->cache_conn_info, &hdd_sta_ctx->conn_info,
		     sizeof(hdd_sta_ctx->cache_conn_info));
}

/**
 * hdd_conn_save_connect_info() - save current connection information
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @bss_type: bss type
 *
 * Return: none
 */
static void
hdd_conn_save_connect_info(struct hdd_adapter *adapter,
			   struct csr_roam_info *roam_info,
			   eCsrRoamBssType bss_type)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	eCsrEncryptionType encrypt_type = eCSR_ENCRYPT_TYPE_NONE;

	QDF_ASSERT(roam_info);

	if (roam_info) {
		/* Save the BSSID for the connection */
		if (eCSR_BSS_TYPE_INFRASTRUCTURE == bss_type) {
			QDF_ASSERT(roam_info->bss_desc);
			qdf_copy_macaddr(&sta_ctx->conn_info.bssid,
					 &roam_info->bssid);

		} else if (eCSR_BSS_TYPE_IBSS == bss_type) {
			qdf_copy_macaddr(&sta_ctx->conn_info.bssid,
					 &roam_info->bssid);
		} else {
			/*
			 * can't happen. We need a valid IBSS or Infra setting
			 * in the BSSDescription or we can't function.
			 */
			QDF_ASSERT(0);
		}

		/* notify WMM */
		hdd_wmm_connect(adapter, roam_info, bss_type);

		if (!roam_info->u.pConnectedProfile) {
			QDF_ASSERT(roam_info->u.pConnectedProfile);
		} else {
			/* Get Multicast Encryption Type */
			encrypt_type =
			    roam_info->u.pConnectedProfile->mcEncryptionType;
			sta_ctx->conn_info.mc_encrypt_type = encrypt_type;
			/* Get Unicast Encryption Type */
			encrypt_type =
				roam_info->u.pConnectedProfile->EncryptionType;
			sta_ctx->conn_info.uc_encrypt_type = encrypt_type;

			sta_ctx->conn_info.auth_type =
				roam_info->u.pConnectedProfile->AuthType;
			sta_ctx->conn_info.last_auth_type =
				sta_ctx->conn_info.auth_type;

			sta_ctx->conn_info.chan_freq =
				roam_info->u.pConnectedProfile->op_freq;

			/* Save the ssid for the connection */
			qdf_mem_copy(&sta_ctx->conn_info.ssid.SSID,
				     &roam_info->u.pConnectedProfile->SSID,
				     sizeof(tSirMacSSid));
			qdf_mem_copy(&sta_ctx->conn_info.last_ssid.SSID,
				     &roam_info->u.pConnectedProfile->SSID,
				     sizeof(tSirMacSSid));

			/* Save dot11mode in which STA associated to AP */
			sta_ctx->conn_info.dot11mode =
				roam_info->u.pConnectedProfile->dot11Mode;

			sta_ctx->conn_info.proxy_arp_service =
				roam_info->u.pConnectedProfile->proxy_arp_service;

			sta_ctx->conn_info.nss = roam_info->chan_info.nss;

			sta_ctx->conn_info.rate_flags =
				roam_info->chan_info.rate_flags;

			sta_ctx->conn_info.ch_width =
				roam_info->chan_info.ch_width;
		}
		hdd_save_bss_info(adapter, roam_info);
	}
}

/**
 * hdd_send_ft_assoc_response() - send fast transition assoc response
 * @dev: pointer to net device
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
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
			   struct hdd_adapter *adapter,
			   struct csr_roam_info *roam_info)
{
	union iwreq_data wrqu;
	char *buff;
	unsigned int len = 0;
	u8 *assoc_rsp = NULL;

	if (roam_info->nAssocRspLength < FT_ASSOC_RSP_IES_OFFSET) {
		hdd_debug("Invalid assoc rsp length %d",
			  roam_info->nAssocRspLength);
		return;
	}

	assoc_rsp =
		(u8 *) (roam_info->pbFrames + roam_info->nBeaconLength +
			roam_info->nAssocReqLength);
	if (!assoc_rsp) {
		hdd_debug("AssocReq or AssocRsp is NULL");
		return;
	}
	/* assoc_rsp needs to point to the IEs */
	assoc_rsp += FT_ASSOC_RSP_IES_OFFSET;

	/* Send the Assoc Resp, the supplicant needs this for initial Auth. */
	len = roam_info->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
	if (len > IW_GENERIC_IE_MAX) {
		hdd_err("Invalid Assoc resp length %d", len);
		return;
	}
	wrqu.data.length = len;

	/* We need to send the IEs to the supplicant. */
	buff = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	if (!buff)
		return;

	memcpy(buff, assoc_rsp, len);
	wireless_send_event(dev, IWEVASSOCRESPIE, &wrqu, buff);

	qdf_mem_free(buff);
}

/**
 * hdd_send_ft_event() - send fast transition event
 * @adapter: pointer to adapter
 *
 * Send the FTIEs, RIC IEs during FT. This is eventually used to send the
 * FT events to the supplicant. At the reception of Auth2 we send the RIC
 * followed by the auth response IEs to the supplicant.
 * Once both are received in the supplicant, an FT event is generated
 * to the supplicant.
 *
 * Return: none
 */
static void hdd_send_ft_event(struct hdd_adapter *adapter)
{
	uint16_t auth_resp_len = 0;
	uint32_t ric_ies_length = 0;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	mac_handle_t mac_handle;

#if defined(KERNEL_SUPPORT_11R_CFG80211)
	struct cfg80211_ft_event_params ftEvent;
	uint8_t ftIe[DOT11F_IE_FTINFO_MAX_LEN];
	uint8_t ricIe[DOT11F_IE_RICDESCRIPTOR_MAX_LEN];
	struct net_device *dev = adapter->dev;
#else
	char *buff;
	union iwreq_data wrqu;
	uint16_t str_len;
#endif

	mac_handle = hdd_ctx->mac_handle;
#if defined(KERNEL_SUPPORT_11R_CFG80211)
	qdf_mem_zero(ftIe, DOT11F_IE_FTINFO_MAX_LEN);
	qdf_mem_zero(ricIe, DOT11F_IE_RICDESCRIPTOR_MAX_LEN);

	sme_get_rici_es(mac_handle, adapter->vdev_id, (u8 *) ricIe,
			DOT11F_IE_RICDESCRIPTOR_MAX_LEN, &ric_ies_length);
	if (ric_ies_length == 0)
		hdd_warn("Do not send RIC IEs as length is 0");

	ftEvent.ric_ies = ricIe;
	ftEvent.ric_ies_len = ric_ies_length;
	hdd_debug("RIC IEs is of length %d", (int)ric_ies_length);

	sme_get_ft_pre_auth_response(mac_handle, adapter->vdev_id,
				     (u8 *) ftIe, DOT11F_IE_FTINFO_MAX_LEN,
				     &auth_resp_len);

	if (auth_resp_len == 0) {
		hdd_debug("AuthRsp FTIES is of length 0");
		return;
	}

	sme_set_ft_pre_auth_state(mac_handle, adapter->vdev_id, true);

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
	if (!buff)
		return;

	/* Sme needs to send the RIC IEs first */
	str_len = strlcpy(buff, "RIC=", IW_CUSTOM_MAX);
	sme_get_rici_es(mac_handle, adapter->vdev_id,
			(u8 *) &(buff[str_len]), (IW_CUSTOM_MAX - str_len),
			&ric_ies_length);
	if (ric_ies_length == 0) {
		hdd_warn("Do not send RIC IEs as length is 0");
	} else {
		wrqu.data.length = str_len + ric_ies_length;
		wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buff);
	}

	/* Sme needs to provide the Auth Resp */
	qdf_mem_zero(buff, IW_CUSTOM_MAX);
	str_len = strlcpy(buff, "AUTH=", IW_CUSTOM_MAX);
	sme_get_ft_pre_auth_response(mac_handle, adapter->vdev_id,
				     (u8 *) &buff[str_len],
				     (IW_CUSTOM_MAX - str_len), &auth_resp_len);

	if (auth_resp_len == 0) {
		qdf_mem_free(buff);
		hdd_debug("AuthRsp FTIES is of length 0");
		return;
	}

	wrqu.data.length = str_len + auth_resp_len;
	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buff);

	qdf_mem_free(buff);
#endif
}

#ifdef FEATURE_WLAN_ESE
/**
 * hdd_send_new_ap_channel_info() - send new ap channel info
 * @dev: pointer to net device
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 *
 * Send the ESE required "new AP Channel info" to the supplicant.
 * (This keeps the supplicant "up to date" on the current channel.)
 *
 * The current (new AP) channel information is passed in.
 *
 * Return: none
 */
static void
hdd_send_new_ap_channel_info(struct net_device *dev,
			     struct hdd_adapter *adapter,
			     struct csr_roam_info *roam_info)
{
	union iwreq_data wrqu;
	struct bss_description *descriptor = roam_info->bss_desc;
	mac_handle_t mac_hdl;
	struct wlan_objmgr_pdev *pdev;

	if (!descriptor) {
		hdd_err("bss descriptor is null");
		return;
	}
	/*
	 * Send the Channel event, the supplicant needs this to generate
	 * the Adjacent AP report.
	 */
	hdd_debug("Sending up an SIOCGIWFREQ, channel freq: %d",
		  descriptor->chan_freq);
	memset(&wrqu, '\0', sizeof(wrqu));
	mac_hdl = hdd_adapter_get_mac_handle(adapter);
	if (!mac_hdl) {
		hdd_err("MAC handle invalid, falling back!");
		return;
	}
	pdev = MAC_CONTEXT(mac_hdl)->pdev;
	if (!pdev) {
		hdd_err("pdev invalid in MAC context, falling back!");
		return;
	}
	wrqu.freq.m = wlan_reg_freq_to_chan(pdev, descriptor->chan_freq);
	wrqu.freq.e = 0;
	wrqu.freq.i = 0;
	wireless_send_event(adapter->dev, SIOCGIWFREQ, &wrqu, NULL);
}

#endif /* FEATURE_WLAN_ESE */

/**
 * hdd_send_update_beacon_ies_event() - send update beacons ie event
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 *
 * Return: none
 */
static void
hdd_send_update_beacon_ies_event(struct hdd_adapter *adapter,
				 struct csr_roam_info *roam_info)
{
	union iwreq_data wrqu;
	u8 *beacon_ies;
	u8 currentLen = 0;
	char *buff;
	int totalIeLen = 0, currentOffset = 0, strLen;

	memset(&wrqu, '\0', sizeof(wrqu));

	if (0 == roam_info->nBeaconLength) {
		hdd_debug("beacon frame length is 0");
		return;
	}
	beacon_ies = (u8 *) (roam_info->pbFrames + BEACON_FRAME_IES_OFFSET);
	if (!beacon_ies) {
		hdd_warn("Beacon IEs is NULL");
		return;
	}
	/* beacon_ies needs to point to the IEs */
	hdd_debug("Beacon IEs is now at %02x%02x",
		   (unsigned int)beacon_ies[0],
		   (unsigned int)beacon_ies[1]);
	hdd_debug("Beacon IEs length = %d",
		   roam_info->nBeaconLength - BEACON_FRAME_IES_OFFSET);

	/* We need to send the IEs to the supplicant. */
	buff = qdf_mem_malloc(IW_CUSTOM_MAX);
	if (!buff)
		return;

	strLen = strlcpy(buff, "BEACONIEs=", IW_CUSTOM_MAX);
	currentLen = strLen + 1;

	totalIeLen = roam_info->nBeaconLength - BEACON_FRAME_IES_OFFSET;
	do {
		/*
		 * If the beacon size exceeds max CUSTOM event size, break it
		 * into chunks of CUSTOM event max size and send it to
		 * supplicant. Changes are done in supplicant to handle this.
		 */
		qdf_mem_zero(&buff[strLen + 1], IW_CUSTOM_MAX - (strLen + 1));
		currentLen =
			QDF_MIN(totalIeLen, IW_CUSTOM_MAX - (strLen + 1) - 1);
		qdf_mem_copy(&buff[strLen + 1], beacon_ies + currentOffset,
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
		wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buff);
	} while (totalIeLen > 0);

	qdf_mem_free(buff);
}

/**
 * hdd_send_association_event() - send association event
 * @dev: pointer to net device
 * @roam_info: pointer to roam info
 *
 * Return: none
 */
static void hdd_send_association_event(struct net_device *dev,
				       struct csr_roam_info *roam_info)
{
	int ret;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	union iwreq_data wrqu;
	int we_event;
	char *msg;
	struct qdf_mac_addr peer_macaddr;
	struct csr_roam_profile *roam_profile;

	roam_profile = hdd_roam_profile(adapter);
	memset(&wrqu, '\0', sizeof(wrqu));
	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	we_event = SIOCGIWAP;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (roam_info)
		if (roam_info->roamSynchInProgress) {
			/* Update tdls module about the disconnection event */
			hdd_notify_sta_disconnect(adapter->vdev_id,
						 true, false,
						 adapter->vdev);
		}
#endif
	if (eConnectionState_Associated == sta_ctx->conn_info.conn_state) {
		struct oem_channel_info chan_info = {0};

		if (!roam_info || !roam_info->bss_desc) {
			hdd_warn("STA in associated state but roam_info is null");
			return;
		}

		if (!hdd_is_roam_sync_in_progress(roam_info)) {
			policy_mgr_incr_active_session(hdd_ctx->psoc,
				adapter->device_mode, adapter->vdev_id);
			hdd_green_ap_start_state_mc(hdd_ctx,
						    adapter->device_mode, true);
		}
		memcpy(wrqu.ap_addr.sa_data, roam_info->bss_desc->bssId,
		       sizeof(roam_info->bss_desc->bssId));

		ucfg_p2p_status_connect(adapter->vdev);

		hdd_info("%s(vdevid-%d): " QDF_MAC_ADDR_STR " connected to "
			QDF_MAC_ADDR_STR, dev->name, adapter->vdev_id,
			QDF_MAC_ADDR_ARRAY(adapter->mac_addr.bytes),
			QDF_MAC_ADDR_ARRAY(wrqu.ap_addr.sa_data));
		hdd_send_update_beacon_ies_event(adapter, roam_info);

		/*
		 * Send IWEVASSOCRESPIE Event if WLAN_FEATURE_CIQ_METRICS
		 * is Enabled Or Send IWEVASSOCRESPIE Event if
		 * fFTEnable is true.
		 * Send FT Keys to the supplicant when FT is enabled
		 */
		if ((roam_profile->AuthType.authType[0] ==
		     eCSR_AUTH_TYPE_FT_RSN_PSK)
		    || (roam_profile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_FT_RSN)
		    || (roam_profile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_FT_SAE)
		    || (roam_profile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384)
#ifdef FEATURE_WLAN_ESE
		    || (roam_profile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_CCKM_RSN)
		    || (roam_profile->AuthType.authType[0] ==
			eCSR_AUTH_TYPE_CCKM_WPA)
#endif
		    ) {
			hdd_send_ft_assoc_response(dev, adapter, roam_info);
		}
		qdf_copy_macaddr(&peer_macaddr,
				 &sta_ctx->conn_info.bssid);
		chan_info.mhz = roam_info->chan_info.mhz;
		chan_info.info = roam_info->chan_info.info;
		chan_info.band_center_freq1 =
			roam_info->chan_info.band_center_freq1;
		chan_info.band_center_freq2 =
			roam_info->chan_info.band_center_freq2;
		chan_info.reg_info_1 =
			roam_info->chan_info.reg_info_1;
		chan_info.reg_info_2 =
			roam_info->chan_info.reg_info_2;

		ret = hdd_objmgr_set_peer_mlme_state(adapter->vdev,
						     WLAN_ASSOC_STATE);
		if (ret)
			hdd_err("Peer object %pM fail to set associated state",
					peer_macaddr.bytes);

		/* send peer status indication to oem app */
		hdd_send_peer_status_ind_to_app(&peer_macaddr,
						ePeerConnected,
						roam_info->timingMeasCap,
						adapter->vdev_id, &chan_info,
						adapter->device_mode);

#ifdef FEATURE_WLAN_TDLS
		/* Update tdls module about connection event */
		hdd_notify_sta_connect(adapter->vdev_id,
				       roam_info->tdls_chan_swit_prohibited,
				       roam_info->tdls_prohibited,
				       adapter->vdev);
#endif
		/* start timer in sta/p2p_cli */
		hdd_bus_bw_compute_prev_txrx_stats(adapter);
		hdd_bus_bw_compute_timer_start(hdd_ctx);
	} else if (eConnectionState_IbssConnected ==    /* IBss Associated */
			sta_ctx->conn_info.conn_state) {
		policy_mgr_update_connection_info(hdd_ctx->psoc,
				adapter->vdev_id);
		memcpy(wrqu.ap_addr.sa_data, sta_ctx->conn_info.bssid.bytes,
				ETH_ALEN);
		hdd_debug("%s(vdevid-%d): new IBSS peer connection to BSSID " QDF_MAC_ADDR_STR,
			  dev->name, adapter->vdev_id,
			  QDF_MAC_ADDR_ARRAY(sta_ctx->conn_info.bssid.bytes));
	} else {                /* Not Associated */
		hdd_info("%s(vdevid-%d): disconnected", dev->name,
			 adapter->vdev_id);
		memset(wrqu.ap_addr.sa_data, '\0', ETH_ALEN);
		policy_mgr_decr_session_set_pcl(hdd_ctx->psoc,
				adapter->device_mode, adapter->vdev_id);
		hdd_green_ap_start_state_mc(hdd_ctx, adapter->device_mode,
					    false);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(hdd_ctx, true);
#endif

		if ((adapter->device_mode == QDF_STA_MODE) ||
		    (adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
			qdf_copy_macaddr(&peer_macaddr,
					 &sta_ctx->conn_info.bssid);

			/* send peer status indication to oem app */
			hdd_send_peer_status_ind_to_app(&peer_macaddr,
							ePeerDisconnected, 0,
							adapter->vdev_id,
							NULL,
							adapter->device_mode);
		}

		hdd_lpass_notify_disconnect(adapter);
		/* Update tdls module about the disconnection event */
		hdd_notify_sta_disconnect(adapter->vdev_id,
					  false,
					  false,
					  adapter->vdev);

		/* stop timer in sta/p2p_cli */
		hdd_bus_bw_compute_reset_prev_txrx_stats(adapter);
		hdd_bus_bw_compute_timer_try_stop(hdd_ctx);
	}
	hdd_ipa_set_tx_flow_info();

	msg = NULL;
	/* During the WLAN uninitialization,supplicant is stopped before the
	 * driver so not sending the status of the connection to supplicant
	 */
	if (cds_is_load_or_unload_in_progress()) {
		wireless_send_event(dev, we_event, &wrqu, msg);
#ifdef FEATURE_WLAN_ESE
		if (eConnectionState_Associated ==
			 sta_ctx->conn_info.conn_state) {
			if ((roam_profile->AuthType.authType[0] ==
			     eCSR_AUTH_TYPE_CCKM_RSN) ||
			    (roam_profile->AuthType.authType[0] ==
				eCSR_AUTH_TYPE_CCKM_WPA))
				hdd_send_new_ap_channel_info(dev, adapter,
							     roam_info);
		}
#endif
	}
}

/**
 * hdd_conn_remove_connect_info() - remove connection info
 * @sta_ctx: pointer to global HDD station context
 * @roam_info: pointer to roam info
 *
 * Return: none
 */
static void hdd_conn_remove_connect_info(struct hdd_station_ctx *sta_ctx)
{
	/* Remove bssid and peer_macaddr */
	qdf_mem_zero(&sta_ctx->conn_info.bssid, QDF_MAC_ADDR_SIZE);
	qdf_mem_zero(&sta_ctx->conn_info.peer_macaddr[0],
		     QDF_MAC_ADDR_SIZE);

	/* Clear all security settings */
	sta_ctx->conn_info.auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	sta_ctx->conn_info.mc_encrypt_type = eCSR_ENCRYPT_TYPE_NONE;
	sta_ctx->conn_info.uc_encrypt_type = eCSR_ENCRYPT_TYPE_NONE;

	qdf_mem_zero(&sta_ctx->ibss_enc_key, sizeof(tCsrRoamSetKey));

	sta_ctx->conn_info.proxy_arp_service = 0;

	qdf_mem_zero(&sta_ctx->conn_info.ssid, sizeof(tCsrSSIDInfo));

	/*
	 * Reset the ptk, gtk status flags to avoid using current connection
	 * status in further connections.
	 */
	sta_ctx->conn_info.gtk_installed = false;
	sta_ctx->conn_info.ptk_installed = false;
}

/**
 * hdd_clear_roam_profile_ie() - Clear Roam Profile IEs
 * @adapter: adapter who's IEs are to be cleared
 *
 * Return: None
 */
static void hdd_clear_roam_profile_ie(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx;
	struct csr_roam_profile *roam_profile;

	hdd_enter();

	/* clear WPA/RSN/WSC IE information in the profile */
	roam_profile = hdd_roam_profile(adapter);

	roam_profile->nWPAReqIELength = 0;
	roam_profile->pWPAReqIE = NULL;
	roam_profile->nRSNReqIELength = 0;
	roam_profile->pRSNReqIE = NULL;

#ifdef FEATURE_WLAN_WAPI
	roam_profile->nWAPIReqIELength = 0;
	roam_profile->pWAPIReqIE = NULL;
#endif

	roam_profile->bWPSAssociation = false;
	roam_profile->bOSENAssociation = false;
	roam_profile->pAddIEScan = NULL;
	roam_profile->nAddIEScanLength = 0;
	roam_profile->pAddIEAssoc = NULL;
	roam_profile->nAddIEAssocLength = 0;

	roam_profile->EncryptionType.numEntries = 1;
	roam_profile->EncryptionType.encryptionType[0] =
		eCSR_ENCRYPT_TYPE_NONE;

	roam_profile->mcEncryptionType.numEntries = 1;
	roam_profile->mcEncryptionType.encryptionType[0] =
		eCSR_ENCRYPT_TYPE_NONE;

	roam_profile->AuthType.numEntries = 1;
	roam_profile->AuthType.authType[0] =
		eCSR_AUTH_TYPE_OPEN_SYSTEM;

	qdf_mem_zero(roam_profile->bssid_hint.bytes, QDF_MAC_ADDR_SIZE);

#ifdef WLAN_FEATURE_11W
	roam_profile->MFPEnabled = false;
	roam_profile->MFPRequired = 0;
	roam_profile->MFPCapable = 0;
#endif

	qdf_mem_zero(roam_profile->Keys.KeyLength, CSR_MAX_NUM_KEY);
	qdf_mem_zero(roam_profile->Keys.KeyMaterial,
		     sizeof(roam_profile->Keys.KeyMaterial));
#ifdef FEATURE_WLAN_WAPI
	adapter->wapi_info.wapi_auth_mode = WAPI_AUTH_MODE_OPEN;
	adapter->wapi_info.wapi_mode = false;
#endif

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	sta_ctx->auth_key_mgmt = 0;
	qdf_zero_macaddr(&sta_ctx->requested_bssid);
	hdd_clear_fils_connection_info(adapter);
	hdd_exit();
}

QDF_STATUS hdd_roam_deregister_sta(struct hdd_adapter *adapter,
				   struct qdf_mac_addr mac_addr)
{
	QDF_STATUS qdf_status;

	qdf_status = cdp_clear_peer(cds_get_context(QDF_MODULE_ID_SOC),
			(struct cdp_pdev *)cds_get_context(QDF_MODULE_ID_TXRX),
			mac_addr);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_clear_peer() failed for sta mac: "
			QDF_MAC_ADDR_STR ". Status(%d) [0x%08X]",
			QDF_MAC_ADDR_ARRAY(mac_addr.bytes),
			qdf_status, qdf_status);
	}

	return qdf_status;
}

/**
 * hdd_print_bss_info() - print bss info
 * @hdd_sta_ctx: pointer to hdd station context
 *
 * Return: None
 */
static void hdd_print_bss_info(struct hdd_station_ctx *hdd_sta_ctx)
{
	uint32_t *cap_info;

	hdd_debug("WIFI DATA LOGGER");
	hdd_debug("chan_freq: %d",
		  hdd_sta_ctx->conn_info.chan_freq);
	hdd_debug("dot11mode: %d",
		 hdd_sta_ctx->conn_info.dot11mode);
	hdd_debug("AKM: %d",
		  hdd_sta_ctx->conn_info.last_auth_type);
	hdd_debug("ssid: %.*s",
		 hdd_sta_ctx->conn_info.last_ssid.SSID.length,
		 hdd_sta_ctx->conn_info.last_ssid.SSID.ssId);
	hdd_debug("roam count: %d",
		 hdd_sta_ctx->conn_info.roam_count);
	hdd_debug("ant_info: %d",
		 hdd_sta_ctx->conn_info.txrate.nss);
	hdd_debug("datarate legacy %d",
		 hdd_sta_ctx->conn_info.txrate.legacy);
	hdd_debug("datarate mcs: %d",
		 hdd_sta_ctx->conn_info.txrate.mcs);
	if (hdd_sta_ctx->conn_info.conn_flag.ht_present) {
		cap_info = (uint32_t *)&hdd_sta_ctx->conn_info.ht_caps;
		hdd_debug("ht caps: %x", *cap_info);
	}
	if (hdd_sta_ctx->conn_info.conn_flag.vht_present) {
		cap_info = (uint32_t *)&hdd_sta_ctx->conn_info.vht_caps;
		hdd_debug("vht caps: %x", *cap_info);
	}
	if (hdd_sta_ctx->conn_info.conn_flag.hs20_present)
		hdd_debug("hs20 info: %x",
			 hdd_sta_ctx->conn_info.hs20vendor_ie.release_num);
	hdd_debug("signal: %d",
		 hdd_sta_ctx->conn_info.signal);
	hdd_debug("noise: %d",
		 hdd_sta_ctx->conn_info.noise);
}

/**
 * hdd_dis_connect_handler() - disconnect event handler
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam identifier
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * This function handles disconnect event:
 * 1. Disable transmit queues;
 * 2. Clean up internal connection states and data structures;
 * 3. Send disconnect indication to supplicant.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS hdd_dis_connect_handler(struct hdd_adapter *adapter,
					  struct csr_roam_info *roam_info,
					  uint32_t roam_id,
					  eRoamCmdStatus roam_status,
					  eCsrRoamResult roam_result)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS vstatus;
	struct net_device *dev = adapter->dev;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	bool sendDisconInd = true;
	mac_handle_t mac_handle;
	struct wlan_ies disconnect_ies = {0};

	if (!dev) {
		hdd_err("net_dev is released return");
		return QDF_STATUS_E_FAILURE;
	}
	/* notify apps that we can't pass traffic anymore */
	hdd_debug("Disabling queues");
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);

	if (ucfg_ipa_is_enabled() &&
	    (hdd_is_valid_mac_address(sta_ctx->conn_info.bssid.bytes)))
		ucfg_ipa_wlan_evt(hdd_ctx->pdev, adapter->dev,
				  adapter->device_mode,
				  adapter->vdev_id,
				  WLAN_IPA_STA_DISCONNECT,
				  sta_ctx->conn_info.bssid.bytes);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	wlan_hdd_auto_shutdown_enable(hdd_ctx, true);
#endif

	DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
				adapter->vdev_id,
				QDF_TRACE_DEFAULT_PDEV_ID,
				QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_DISASSOC));

	/* HDD has initiated disconnect, do not send disconnect indication
	 * to kernel. Sending disconnected event to kernel for userspace
	 * initiated disconnect will be handled by disconnect handler call
	 * to cfg80211_disconnected.
	 */
	if ((eConnectionState_Disconnecting ==
	    sta_ctx->conn_info.conn_state) ||
	    (eConnectionState_NotConnected ==
	    sta_ctx->conn_info.conn_state) ||
	    (eConnectionState_Connecting ==
	    sta_ctx->conn_info.conn_state)) {
		hdd_debug("HDD has initiated a disconnect, no need to send disconnect indication to kernel");
		sendDisconInd = false;
	} else {
		INIT_COMPLETION(adapter->disconnect_comp_var);
		hdd_conn_set_connection_state(adapter,
					      eConnectionState_Disconnecting);
	}

	hdd_clear_roam_profile_ie(adapter);
	hdd_wmm_init(adapter);
	hdd_debug("Invoking packetdump deregistration API");
	wlan_deregister_txrx_packetdump(OL_TXRX_PDEV_ID);

	/* indicate 'disconnect' status to wpa_supplicant... */
	hdd_send_association_event(dev, roam_info);
	/* indicate disconnected event to nl80211 */
	if (roam_status != eCSR_ROAM_IBSS_LEAVE) {
		/*
		 * Only send indication to kernel if not initiated
		 * by kernel
		 */
		if (sendDisconInd) {
			int reason = WLAN_REASON_UNSPECIFIED;

			if (roam_info && roam_info->disconnect_ies) {
				disconnect_ies.data =
					roam_info->disconnect_ies->data;
				disconnect_ies.len =
					roam_info->disconnect_ies->len;
			}
			/*
			 * To avoid wpa_supplicant sending "HANGED" CMD
			 * to ICS UI.
			 */
			if (roam_info && eCSR_ROAM_LOSTLINK == roam_status) {
				reason = roam_info->reasonCode;
				if (reason ==
				    eSIR_MAC_PEER_STA_REQ_LEAVING_BSS_REASON)
					pr_info("wlan: disconnected due to poor signal, rssi is %d dB\n",
						roam_info->rxRssi);
			}
			wlan_hdd_cfg80211_indicate_disconnect(
							dev, false,
							reason,
							disconnect_ies.data,
							disconnect_ies.len);

			hdd_debug("sent disconnected event to nl80211, reason code %d",
				  reason);
		}

		/* update P2P connection status */
		ucfg_p2p_status_disconnect(adapter->vdev);
	}

	/* Inform BLM about the disconnection with the AP */
	if (adapter->device_mode == QDF_STA_MODE)
		ucfg_blm_update_bssid_connect_params(hdd_ctx->pdev,
						     sta_ctx->conn_info.bssid,
						     BLM_AP_DISCONNECTED);

	hdd_wmm_adapter_clear(adapter);
	mac_handle = hdd_ctx->mac_handle;
	sme_ft_reset(mac_handle, adapter->vdev_id);
	sme_reset_key(mac_handle, adapter->vdev_id);
	if (!hdd_remove_beacon_filter(adapter))
		hdd_err("hdd_remove_beacon_filter() failed");

	if (sme_is_beacon_report_started(mac_handle, adapter->vdev_id)) {
		hdd_debug("Sending beacon pause indication to userspace");
		hdd_beacon_recv_pause_indication((hdd_handle_t)hdd_ctx,
						 adapter->vdev_id,
						 SCAN_EVENT_TYPE_MAX, true);
	}
	if (eCSR_ROAM_IBSS_LEAVE == roam_status) {
		uint8_t i;

		vstatus = hdd_roam_deregister_sta(adapter,
						  sta_ctx->conn_info.bssid);
		if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
			hdd_err("hdd_roam_deregister_sta() failed for sta"
				QDF_MAC_ADDR_STR " Status: %d [0x%x]",
				QDF_MAC_ADDR_ARRAY(
				sta_ctx->conn_info.bssid.bytes),
				status, status);
			status = QDF_STATUS_E_FAILURE;
		}

		/* Clear all the peer sta register with TL. */
		for (i = 0; i < MAX_PEERS; i++) {
			struct qdf_mac_addr mac_addr =
				sta_ctx->conn_info.peer_macaddr[i];

			if (!hdd_is_valid_mac_address(mac_addr.bytes))
				continue;

			hdd_debug("Deregister Sta " QDF_MAC_ADDR_STR,
				  QDF_MAC_ADDR_ARRAY(mac_addr.bytes));

			vstatus = hdd_roam_deregister_sta(adapter, mac_addr);

			if (!QDF_IS_STATUS_SUCCESS(vstatus)) {
				hdd_err("hdd_roam_deregister_sta() failed sta "
					QDF_MAC_ADDR_STR " Status: %d [0x%x]",
					QDF_MAC_ADDR_ARRAY(mac_addr.bytes),
					status, status);
				status = QDF_STATUS_E_FAILURE;
			}
			/* set the peer mac as 0, all other
			 * reset are done in hdd_conn_remove_connect_info.
			 */
			qdf_mem_zero(&sta_ctx->conn_info.peer_macaddr[i],
				sizeof(struct qdf_mac_addr));

		}
	} else {
		hdd_debug("roam_result: %d", roam_result);

		/* clear scan cache for Link Lost */
		if (eCSR_ROAM_LOSTLINK == roam_status) {
			wlan_hdd_cfg80211_unlink_bss(adapter,
				sta_ctx->conn_info.bssid.bytes,
				sta_ctx->conn_info.ssid.SSID.ssId,
				sta_ctx->conn_info.ssid.SSID.length);
			sme_remove_bssid_from_scan_list(mac_handle,
			sta_ctx->conn_info.bssid.bytes);
		}

	}
	/* Clear saved connection information in HDD */
	hdd_conn_remove_connect_info(sta_ctx);
	/*
	* eConnectionState_Connecting state mean that connection is in
	* progress so no need to set state to eConnectionState_NotConnected
	*/
	if ((eConnectionState_Connecting != sta_ctx->conn_info.conn_state)) {
		hdd_conn_set_connection_state(adapter,
					       eConnectionState_NotConnected);
		 hdd_set_roaming_in_progress(false);
	}
	ucfg_pmo_flush_gtk_offload_req(adapter->vdev);

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
		sme_ps_disable_auto_ps_timer(mac_handle,
					     adapter->vdev_id);
		adapter->send_mode_change = true;
	}
	wlan_hdd_clear_link_layer_stats(adapter);

	hdd_debug("check for SAP restart");
	policy_mgr_check_concurrent_intf_and_restart_sap(hdd_ctx->psoc);
	adapter->hdd_stats.tx_rx_stats.cont_txtimeout_cnt = 0;

	/* Unblock anyone waiting for disconnect to complete */
	complete(&adapter->disconnect_comp_var);

	hdd_nud_reset_tracking(adapter);

	hdd_set_disconnect_status(adapter, false);

	hdd_reset_limit_off_chan(adapter);

	hdd_print_bss_info(sta_ctx);

	if (policy_mgr_is_sta_active_connection_exists(hdd_ctx->psoc))
		sme_enable_roaming_on_connected_sta(mac_handle,
						    adapter->vdev_id);

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
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter = NULL;

	if (!hdd_ctx) {
		hdd_err("Invalid hdd context");
		return;
	}
	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("Invalid vdev_id");
		return;
	}
	complete(&adapter->sta_authorized_event);
}

#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
static inline
void hdd_set_unpause_queue(void *soc, struct hdd_adapter *adapter, void *peer)
{
	unsigned long rc;
	/* wait for event from firmware to set the event */
	rc = wait_for_completion_timeout(
			&adapter->sta_authorized_event,
			msecs_to_jiffies(HDD_PEER_AUTHORIZE_WAIT));
	if (!rc)
		hdd_debug("timeout waiting for sta_authorized_event");

	cdp_fc_vdev_unpause(soc, adapter->vdev_id,
			    OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED,
			    0);
}
#else
static inline
void hdd_set_unpause_queue(void *soc, struct hdd_adapter *adapter, void *peer)
{ }
#endif

QDF_STATUS hdd_change_peer_state(struct hdd_adapter *adapter,
				 uint8_t *peer_mac,
				 enum ol_txrx_peer_state sta_state,
				 bool roam_synch_in_progress)
{
	QDF_STATUS err;
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *peer;

	if (!pdev) {
		hdd_err("Failed to get txrx context");
		return QDF_STATUS_E_FAULT;
	}

	peer = cdp_peer_find_by_addr(soc, (struct cdp_pdev *)pdev, peer_mac);
	if (!peer)
		return QDF_STATUS_E_FAULT;

	err = cdp_peer_state_update(soc, pdev, peer_mac, sta_state);
	if (err != QDF_STATUS_SUCCESS) {
		hdd_err("peer state update failed");
		return QDF_STATUS_E_FAULT;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (roam_synch_in_progress)
		return QDF_STATUS_SUCCESS;
#endif

	if (sta_state == OL_TXRX_PEER_STATE_AUTH) {
		/* Reset scan reject params on successful set key */
		hdd_debug("Reset scan reject params");
		hdd_init_scan_reject_params(adapter->hdd_ctx);
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
		/* make sure event is reset */
		INIT_COMPLETION(adapter->sta_authorized_event);
#endif

		err = sme_set_peer_authorized(
				peer_mac,
				hdd_set_peer_authorized_event,
				adapter->vdev_id);
		if (err != QDF_STATUS_SUCCESS) {
			hdd_err("Failed to set the peer state to authorized");
			return QDF_STATUS_E_FAULT;
		}

		if (adapter->device_mode == QDF_STA_MODE ||
		    adapter->device_mode == QDF_P2P_CLIENT_MODE) {
			hdd_set_unpause_queue(soc, adapter, peer);
		}
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_update_dp_vdev_flags(void *cbk_data,
				    struct qdf_mac_addr *mac_addr,
				    uint32_t vdev_param,
				    bool is_link_up)
{
	struct cdp_vdev *data_vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct hdd_context *hdd_ctx;
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct wlan_objmgr_psoc **psoc;

	if (!cbk_data)
		return status;

	psoc = cbk_data;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("Invalid HDD Context");
		return QDF_STATUS_E_INVAL;
	}

	if (!hdd_ctx->tdls_nap_active)
		return status;

	data_vdev = cdp_peer_get_vdev_by_peer_addr(soc, pdev, *mac_addr);
	if (!data_vdev) {
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	cdp_txrx_set_vdev_param(soc, data_vdev, vdev_param, is_link_up);

	return status;
}

/**
 * hdd_conn_change_peer_state() - Change the state of the peer
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @mac_addr: peer mac address
 * @sta_state: peer state
 *
 * Return: QDF_STATUS enumeration
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static QDF_STATUS hdd_conn_change_peer_state(struct hdd_adapter *adapter,
					     struct csr_roam_info *roam_info,
					     uint8_t *mac_addr,
					     enum ol_txrx_peer_state sta_state)
{
	return hdd_change_peer_state(adapter, mac_addr, sta_state,
				     roam_info->roamSynchInProgress);
}
#else
static QDF_STATUS hdd_conn_change_peer_state(struct hdd_adapter *adapter,
					     struct csr_roam_info *roam_info,
					     uint8_t *mac_addr,
					     enum ol_txrx_peer_state sta_state)
{
	return hdd_change_peer_state(adapter, mac_addr, sta_state,
				     false);
}
#endif

/**
 * hdd_roam_register_sta() - register station
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @sta_id: station identifier
 * @bss_desc: pointer to BSS description
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_roam_register_sta(struct hdd_adapter *adapter,
					struct csr_roam_info *roam_info,
					struct bss_description *bss_desc)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type txrx_desc = {0};
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!bss_desc)
		return QDF_STATUS_E_FAILURE;

	/* Get the Station ID from the one saved during the association */
	if (!QDF_IS_ADDR_BROADCAST(roam_info->bssid.bytes))
		WLAN_ADDR_COPY(txrx_desc.peer_addr.bytes,
			       roam_info->bssid.bytes);
	else
		WLAN_ADDR_COPY(txrx_desc.peer_addr.bytes,
			       adapter->mac_addr.bytes);

	/* set the QoS field appropriately */
	if (hdd_wmm_is_active(adapter))
		txrx_desc.is_qos_enabled = 1;
	else
		txrx_desc.is_qos_enabled = 0;

#ifdef FEATURE_WLAN_WAPI
	hdd_debug("WAPI STA Registered: %d",
		   adapter->wapi_info.is_wapi_sta);
	if (adapter->wapi_info.is_wapi_sta)
		txrx_desc.is_wapi_supported = 1;
	else
		txrx_desc.is_wapi_supported = 0;
#endif /* FEATURE_WLAN_WAPI */

	/* Register the vdev transmit and receive functions */
	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));

	if (adapter->hdd_ctx->enable_dp_rx_threads) {
		txrx_ops.rx.rx = hdd_rx_pkt_thread_enqueue_cbk;
		txrx_ops.rx.rx_stack = hdd_rx_packet_cbk;
		txrx_ops.rx.rx_flush = hdd_rx_flush_packet_cbk;
		txrx_ops.rx.rx_gro_flush = hdd_rx_thread_gro_flush_ind_cbk;
	} else {
		txrx_ops.rx.rx = hdd_rx_packet_cbk;
		txrx_ops.rx.rx_stack = NULL;
		txrx_ops.rx.rx_flush = NULL;
	}

	txrx_ops.rx.stats_rx = hdd_tx_rx_collect_connectivity_stats_info;

	adapter->txrx_vdev = cdp_get_vdev_from_vdev_id(soc,
				(struct cdp_pdev *)pdev,
				adapter->vdev_id);
	if (!adapter->txrx_vdev) {
		return QDF_STATUS_E_FAILURE;
	}

	txrx_ops.tx.tx = NULL;
	cdp_vdev_register(soc, adapter->txrx_vdev, adapter, &txrx_ops);
	if (!txrx_ops.tx.tx) {
		hdd_err("%s vdev register fail", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	adapter->tx_fn = txrx_ops.tx.tx;
	qdf_status = cdp_peer_register(soc,
			(struct cdp_pdev *)pdev, &txrx_desc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed Status: %d [0x%08X]",
			 qdf_status, qdf_status);
		return qdf_status;
	}

	if (!roam_info->fAuthRequired) {
		/*
		 * Connections that do not need Upper layer auth, transition
		 * TLSHIM directly to 'Authenticated' state
		 */
		qdf_status = hdd_conn_change_peer_state(
						adapter, roam_info,
						txrx_desc.peer_addr.bytes,
						OL_TXRX_PEER_STATE_AUTH);

		hdd_conn_set_authenticated(adapter, true);
		hdd_objmgr_set_peer_mlme_auth_state(adapter->vdev, true);
	} else {
		hdd_debug("ULA auth Sta: " QDF_MAC_ADDR_STR
			  " Changing TL state to CONNECTED at Join time",
			  QDF_MAC_ADDR_ARRAY(txrx_desc.peer_addr.bytes));

		qdf_status = hdd_conn_change_peer_state(
						adapter, roam_info,
						txrx_desc.peer_addr.bytes,
						OL_TXRX_PEER_STATE_CONN);

		hdd_conn_set_authenticated(adapter, false);
		hdd_objmgr_set_peer_mlme_auth_state(adapter->vdev, false);
	}
	return qdf_status;
}

/**
 * hdd_send_roamed_ind() - send roamed indication to cfg80211
 * @dev: network device
 * @bss: cfg80211 roamed bss pointer
 * @req_ie: IEs used in reassociation request
 * @req_ie_len: Length of the @req_ie
 * @resp_ie: IEs received in successful reassociation response
 * @resp_ie_len: Length of @resp_ie
 *
 * Return: none
 */
#if defined CFG80211_ROAMED_API_UNIFIED || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
static void hdd_send_roamed_ind(struct net_device *dev,
				struct cfg80211_bss *bss, const uint8_t *req_ie,
				size_t req_ie_len, const uint8_t *resp_ie,
				size_t resp_ie_len)
{
	struct cfg80211_roam_info info = {0};

	info.bss = bss;
	info.req_ie = req_ie;
	info.req_ie_len = req_ie_len;
	info.resp_ie = resp_ie;
	info.resp_ie_len = resp_ie_len;
	cfg80211_roamed(dev, &info, GFP_KERNEL);
}
#else
static inline void hdd_send_roamed_ind(struct net_device *dev,
				       struct cfg80211_bss *bss,
				       const uint8_t *req_ie, size_t req_ie_len,
				       const uint8_t *resp_ie,
				       size_t resp_ie_len)
{
	cfg80211_roamed_bss(dev, bss, req_ie, req_ie_len, resp_ie, resp_ie_len,
			    GFP_KERNEL);
}
#endif

#if defined(WLAN_FEATURE_ROAM_OFFLOAD)
#if defined(WLAN_FEATURE_FILS_SK)
void hdd_save_gtk_params(struct hdd_adapter *adapter,
			 struct csr_roam_info *csr_roam_info, bool is_reassoc)
{
	uint8_t *kek;
	uint32_t kek_len;

	if (is_reassoc) {
		kek = csr_roam_info->kek;
		kek_len = csr_roam_info->kek_len;
	} else {
		/*
		 * This should come for FILS case only.
		 * Caller should make sure fils_join_rsp is
		 * not NULL, if there is need to use else where.
		 */
		kek = csr_roam_info->fils_join_rsp->kek;
		kek_len = csr_roam_info->fils_join_rsp->kek_len;
	}

	wlan_hdd_save_gtk_offload_params(adapter, NULL, 0, kek, kek_len,
					 csr_roam_info->replay_ctr, true);

	hdd_debug("Kek len %d", kek_len);
}
#else
void hdd_save_gtk_params(struct hdd_adapter *adapter,
			 struct csr_roam_info *csr_roam_info, bool is_reassoc)
{
	uint8_t *kek;
	uint32_t kek_len;

	/*
	 * is_reassoc is set to true always for Legacy GTK offload
	 * case, It is false only for FILS case
	 */
	kek = csr_roam_info->kek;
	kek_len = csr_roam_info->kek_len;

	wlan_hdd_save_gtk_offload_params(adapter, NULL, 0, kek, kek_len,
					 csr_roam_info->replay_ctr, true);

	hdd_debug("Kek len %d", kek_len);
}
#endif
#endif

static void hdd_roam_decr_conn_count(struct hdd_adapter *adapter,
				     struct hdd_context *hdd_ctx)
{
	/* In case of LFR2.0, the number of connection count is
	 * already decrement in hdd_sme_roam_callback. Hence
	 * here we should not decrement again.
	 */
	if (roaming_offload_enabled(hdd_ctx))
		policy_mgr_decr_session_set_pcl(hdd_ctx->psoc,
						adapter->device_mode,
						adapter->vdev_id);
}
/**
 * hdd_send_re_assoc_event() - send reassoc event
 * @dev: pointer to net device
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @reqRsnIe: pointer to RSN Information element
 * @reqRsnLength: length of RSN IE
 *
 * Return: none
 */
static void hdd_send_re_assoc_event(struct net_device *dev,
	struct hdd_adapter *adapter, struct csr_roam_info *roam_info,
	uint8_t *reqRsnIe, uint32_t reqRsnLength)
{
	unsigned int len = 0;
	u8 *assoc_rsp = NULL;
	uint8_t *rsp_rsn_ie = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	uint8_t *assoc_req_ies = qdf_mem_malloc(IW_GENERIC_IE_MAX);
	uint32_t rsp_rsn_lemgth = 0;
	struct ieee80211_channel *chan;
	uint8_t buf_ssid_ie[2 + WLAN_SSID_MAX_LEN]; /* 2 bytes-EID and len */
	uint8_t *buf_ptr, ssid_ie_len;
	struct cfg80211_bss *bss = NULL;
	uint8_t *final_req_ie = NULL;
	tCsrRoamConnectedProfile roam_profile;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	qdf_mem_zero(&roam_profile, sizeof(roam_profile));

	if (!rsp_rsn_ie) {
		hdd_err("Unable to allocate RSN IE");
		goto done;
	}

	if (!assoc_req_ies) {
		hdd_err("Unable to allocate Assoc Req IE");
		goto done;
	}

	if (!roam_info || !roam_info->bss_desc) {
		hdd_err("Invalid CSR roam info");
		goto done;
	}

	if (roam_info->nAssocRspLength == 0) {
		hdd_err("Assoc rsp length is 0");
		goto done;
	}

	assoc_rsp =
		(u8 *) (roam_info->pbFrames + roam_info->nBeaconLength +
			roam_info->nAssocReqLength);
	if (!assoc_rsp)
		goto done;

	/* assoc_rsp needs to point to the IEs */
	assoc_rsp += FT_ASSOC_RSP_IES_OFFSET;

	/*
	 * Active session count is decremented upon disconnection, but during
	 * roaming, there is no disconnect indication and hence active session
	 * count is not decremented.
	 * After roaming is completed, active session count is incremented
	 * as a part of connect indication but effectively after roaming the
	 * active session count should still be the same and hence upon
	 * successful reassoc decrement the active session count here.
	 */
	if (!hdd_is_roam_sync_in_progress(roam_info)) {
		hdd_roam_decr_conn_count(adapter, hdd_ctx);
		hdd_green_ap_start_state_mc(hdd_ctx, adapter->device_mode,
					    false);
	}

	/* Send the Assoc Resp, the supplicant needs this for initial Auth */
	len = roam_info->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
	rsp_rsn_lemgth = len;
	qdf_mem_copy(rsp_rsn_ie, assoc_rsp, len);
	qdf_mem_zero(rsp_rsn_ie + len, IW_GENERIC_IE_MAX - len);

	chan = ieee80211_get_channel(adapter->wdev.wiphy,
				     roam_info->bss_desc->chan_freq);

	sme_roam_get_connect_profile(hdd_ctx->mac_handle, adapter->vdev_id,
				     &roam_profile);

	bss = wlan_cfg80211_get_bss(adapter->wdev.wiphy,
				    chan, roam_info->bssid.bytes,
				    &roam_profile.SSID.ssId[0],
				    roam_profile.SSID.length);

	if (!bss)
		hdd_warn("Get BSS returned NULL");
	buf_ptr = buf_ssid_ie;
	*buf_ptr = WLAN_ELEMID_SSID;
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
	if (!final_req_ie) {
		if (bss)
			cfg80211_put_bss(adapter->wdev.wiphy, bss);
		goto done;
	}
	buf_ptr = final_req_ie;
	qdf_mem_copy(buf_ptr, buf_ssid_ie, ssid_ie_len);
	buf_ptr += ssid_ie_len;
	qdf_mem_copy(buf_ptr, reqRsnIe, reqRsnLength);
	qdf_mem_copy(rsp_rsn_ie, assoc_rsp, len);
	qdf_mem_zero(final_req_ie + (ssid_ie_len + reqRsnLength),
		IW_GENERIC_IE_MAX - (ssid_ie_len + reqRsnLength));
	hdd_debug("Req RSN IE:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
			   final_req_ie, (ssid_ie_len + reqRsnLength));
	hdd_send_roamed_ind(dev, bss, final_req_ie,
			    (ssid_ie_len + reqRsnLength), rsp_rsn_ie,
			    rsp_rsn_lemgth);

	qdf_mem_copy(assoc_req_ies,
		(u8 *)roam_info->pbFrames + roam_info->nBeaconLength,
		roam_info->nAssocReqLength);

	hdd_save_gtk_params(adapter, roam_info, true);

	hdd_debug("ReAssoc Req IE dump");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
		assoc_req_ies, roam_info->nAssocReqLength);

	wlan_hdd_send_roam_auth_event(adapter, roam_info->bssid.bytes,
			assoc_req_ies, roam_info->nAssocReqLength,
			rsp_rsn_ie, rsp_rsn_lemgth,
			roam_info);

	hdd_update_hlp_info(dev, roam_info);

done:
	sme_roam_free_connect_profile(&roam_profile);
	if (final_req_ie)
		qdf_mem_free(final_req_ie);
	qdf_mem_free(rsp_rsn_ie);
	qdf_mem_free(assoc_req_ies);
}

/**
 * hdd_is_roam_sync_in_progress()- Check if roam offloaded
 * @roaminfo - Roaming Information
 *
 * Return: roam sync status if roaming offloaded else false
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
bool hdd_is_roam_sync_in_progress(struct csr_roam_info *roaminfo)
{
	if (roaminfo)
		return roaminfo->roamSynchInProgress;
	else
		return false;
}
#endif

#ifdef QCA_IBSS_SUPPORT
/**
 * hdd_roam_ibss_indication_handler() - update the status of the IBSS
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * Here we update the status of the Ibss when we receive information that we
 * have started/joined an ibss session.
 *
 * Return: none
 */
static void hdd_roam_ibss_indication_handler(struct hdd_adapter *adapter,
					     struct csr_roam_info *roam_info,
					     uint32_t roam_id,
					     eRoamCmdStatus roam_status,
					     eCsrRoamResult roam_result)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_debug("%s: id %d, status %d, result %d",
		 adapter->dev->name, roam_id,
		 roam_status, roam_result);

	switch (roam_result) {
	/* both IBSS Started and IBSS Join should come in here. */
	case eCSR_ROAM_RESULT_IBSS_STARTED:
	case eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS:
	case eCSR_ROAM_RESULT_IBSS_COALESCED:
	{
		if (!roam_info) {
			QDF_ASSERT(0);
			return;
		}

		/* When IBSS Started comes from CSR, we need to move
		 * connection state to IBSS Disconnected (meaning no peers
		 * are in the IBSS).
		 */
		hdd_conn_set_connection_state(adapter,
				      eConnectionState_IbssDisconnected);
		/* notify wmm */
		hdd_wmm_connect(adapter, roam_info,
				eCSR_BSS_TYPE_IBSS);

		hdd_roam_register_sta(adapter, roam_info, roam_info->bss_desc);

		if (roam_info->bss_desc) {
			struct cfg80211_bss *bss;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
			struct ieee80211_channel *chan;
#endif
			/* we created the IBSS, notify supplicant */
			hdd_debug("%s: created ibss " QDF_MAC_ADDR_STR,
				adapter->dev->name,
				QDF_MAC_ADDR_ARRAY(
					roam_info->bss_desc->bssId));

			/* we must first give cfg80211 the BSS information */
			bss = wlan_hdd_cfg80211_update_bss_db(adapter,
								roam_info);
			if (!bss) {
				hdd_err("%s: unable to create IBSS entry",
					adapter->dev->name);
				return;
			}
			hdd_debug("Enabling queues");
			wlan_hdd_netif_queue_control(adapter,
					WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					WLAN_CONTROL_PATH);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
			chan = ieee80211_get_channel(
				adapter->wdev.wiphy,
				roam_info->bss_desc->chan_freq);

			if (chan)
				cfg80211_ibss_joined(adapter->dev,
						     bss->bssid, chan,
						     GFP_KERNEL);
			else
				hdd_warn("%s: freq: %d, can't find channel",
					 adapter->dev->name,
					 roam_info->bss_desc->chan_freq);
#else
			cfg80211_ibss_joined(adapter->dev, bss->bssid,
					     GFP_KERNEL);
#endif
			cfg80211_put_bss(
				hdd_ctx->wiphy,
				bss);
		}
		if (eCSR_ROAM_RESULT_IBSS_STARTED == roam_result) {
			policy_mgr_incr_active_session(hdd_ctx->psoc,
				adapter->device_mode, adapter->vdev_id);
			hdd_green_ap_start_state_mc(hdd_ctx,
						    adapter->device_mode, true);
		} else if (eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS == roam_result ||
				eCSR_ROAM_RESULT_IBSS_COALESCED == roam_result) {
			policy_mgr_update_connection_info(hdd_ctx->psoc,
					adapter->vdev_id);
		}
		break;
	}

	case eCSR_ROAM_RESULT_IBSS_START_FAILED:
	{
		hdd_err("%s: unable to create IBSS", adapter->dev->name);
		break;
	}

	default:
		hdd_err("%s: unexpected result %d",
			adapter->dev->name, (int)roam_result);
		break;
	}
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
	    eCSR_ENCRYPT_TYPE_AES_GCMP == encr_type ||
	    eCSR_ENCRYPT_TYPE_AES_GCMP_256 == encr_type ||
	    eCSR_ENCRYPT_TYPE_AES == encr_type)
		return true;
	else
		return false;
}
#else
/**
 * hdd_roam_ibss_indication_handler() - update the status of the IBSS
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * This function is dummy
 *
 * Return: none
 */
static inline void
hdd_roam_ibss_indication_handler(struct hdd_adapter *adapter,
				 struct csr_roam_info *roam_info,
				 uint32_t roam_id,
				 eRoamCmdStatus roam_status,
				 eCsrRoamResult roam_result)
{
}

/**
 * hdd_get_ibss_peer_sta_id() - get sta id for IBSS peer
 * @hddstactx: pointer to HDD sta context
 * @roaminfo: pointer to roaminfo structure
 *
 * This function is dummy
 *
 * Return: WLAN_MAX_STA_COUNT if peer not found.
 */
static inline uint8_t
hdd_get_ibss_peer_sta_id(struct hdd_station_ctx *hddstactx,
			 struct csr_roam_info *roaminfo)
{
	return WLAN_MAX_STA_COUNT;
}

/**
 * hdd_is_key_install_required_for_ibss() - check encryption type to identify
 * if key installation is required
 * @encr_type: encryption type
 *
 * This function is dummy
 *
 * Return: true.
 */
static inline bool
hdd_is_key_install_required_for_ibss(eCsrEncryptionType encr_type)
{
	return true;
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
static int hdd_change_sta_state_authenticated(struct hdd_adapter *adapter,
						 struct csr_roam_info *roaminfo)
{
	QDF_STATUS status;
	uint8_t *mac_addr;
	uint32_t timeout, auto_bmps_timer_val;
	struct hdd_station_ctx *hddstactx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	ucfg_mlme_get_auto_bmps_timer_value(hdd_ctx->psoc,
					    &auto_bmps_timer_val);
	timeout = hddstactx->hdd_reassoc_scenario ?
		AUTO_PS_ENTRY_TIMER_DEFAULT_VALUE :
		(auto_bmps_timer_val * 1000);

	if (QDF_IBSS_MODE == adapter->device_mode)
		mac_addr = roaminfo->peerMac.bytes;
	else
		mac_addr = hddstactx->conn_info.bssid.bytes;

	hdd_debug("Changing Peer state to AUTHENTICATED for Sta = "
		  QDF_MAC_ADDR_STR, QDF_MAC_ADDR_ARRAY(mac_addr));

	/* Connections that do not need Upper layer authentication,
	 * transition TL to 'Authenticated' state after the keys are set
	 */

	status = hdd_change_peer_state(adapter, mac_addr,
				       OL_TXRX_PEER_STATE_AUTH,
				       hdd_is_roam_sync_in_progress(roaminfo));
	hdd_conn_set_authenticated(adapter, true);
	hdd_objmgr_set_peer_mlme_auth_state(adapter->vdev, true);

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
		sme_ps_enable_auto_ps_timer(hdd_ctx->mac_handle,
					    adapter->vdev_id,
					    timeout);
	}

	return qdf_status_to_os_return(status);
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
static void hdd_change_peer_state_after_set_key(struct hdd_adapter *adapter,
						struct csr_roam_info *roaminfo,
						eCsrRoamResult roam_result)
{
	struct hdd_station_ctx *hdd_sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	eCsrEncryptionType encr_type = hdd_sta_ctx->conn_info.uc_encrypt_type;

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
		if (sme_neighbor_roam_is11r_assoc(adapter->hdd_ctx->mac_handle,
						  adapter->vdev_id))
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
}

/**
 * hdd_roam_set_key_complete_handler() - Update the security parameters
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
hdd_roam_set_key_complete_handler(struct hdd_adapter *adapter,
				  struct csr_roam_info *roam_info,
				  uint32_t roam_id,
				  eRoamCmdStatus roam_status,
				  eCsrRoamResult roam_result)
{
	eCsrEncryptionType algorithm;
	bool connected = false;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_enter();

	if (!roam_info) {
		hdd_err("roam_info is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * if (WPA), tell TL to go to 'authenticated' after the keys are set.
	 * then go to 'authenticated'.  For all other authentication types
	 * (those that do not require upper layer authentication) we can put TL
	 * directly into 'authenticated' state.
	 */
	hdd_debug("Set Key completion roam_status =%d roam_result=%d "
		  QDF_MAC_ADDR_STR, roam_status, roam_result,
		  QDF_MAC_ADDR_ARRAY(roam_info->peerMac.bytes));

	connected = hdd_conn_get_connected_cipher_algo(sta_ctx,
						       &algorithm);
	if (connected) {
		hdd_change_peer_state_after_set_key(adapter, roam_info,
						    roam_result);
	}

	if (!adapter->hdd_ctx || !adapter->hdd_ctx->psoc) {
		hdd_err("hdd_ctx or psoc is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	policy_mgr_restart_opportunistic_timer(adapter->hdd_ctx->psoc, false);

	hdd_exit();
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_perform_roam_set_key_complete() - perform set key complete
 * @adapter: pointer to adapter
 *
 * Return: none
 */
void hdd_perform_roam_set_key_complete(struct hdd_adapter *adapter)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct csr_roam_info *roam_info;

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return;
	roam_info->fAuthRequired = false;
	qdf_mem_copy(roam_info->bssid.bytes,
		     sta_ctx->roam_info.bssid, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(roam_info->peerMac.bytes,
		     sta_ctx->roam_info.peer_mac, QDF_MAC_ADDR_SIZE);

	qdf_ret_status =
			hdd_roam_set_key_complete_handler(adapter,
					   roam_info,
					   sta_ctx->roam_info.roam_id,
					   sta_ctx->roam_info.roam_status,
					   eCSR_ROAM_RESULT_AUTHENTICATED);
	if (qdf_ret_status != QDF_STATUS_SUCCESS)
		hdd_err("Set Key complete failure");

	sta_ctx->roam_info.defer_key_complete = false;
	qdf_mem_free(roam_info);
}

#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
void hdd_clear_fils_connection_info(struct hdd_adapter *adapter)
{
	struct csr_roam_profile *roam_profile;

	if ((adapter->device_mode == QDF_SAP_MODE) ||
	    (adapter->device_mode == QDF_P2P_GO_MODE))
		return;

	roam_profile = hdd_roam_profile(adapter);
	if (roam_profile->fils_con_info) {
		qdf_mem_free(roam_profile->fils_con_info);
		roam_profile->fils_con_info = NULL;
	}

	if (roam_profile->hlp_ie) {
		qdf_mem_free(roam_profile->hlp_ie);
		roam_profile->hlp_ie = NULL;
		roam_profile->hlp_ie_len = 0;
	}
}
#endif

/**
 * hdd_association_completion_handler() - association completion handler
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
hdd_association_completion_handler(struct hdd_adapter *adapter,
				   struct csr_roam_info *roam_info,
				   uint32_t roam_id,
				   eRoamCmdStatus roam_status,
				   eCsrRoamResult roam_result)
{
	struct net_device *dev = adapter->dev;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint8_t *reqRsnIe;
	uint32_t reqRsnLength = DOT11F_IE_RSN_MAX_LEN, ie_len;
	int ft_carrier_on = false;
	bool hddDisconInProgress = false;
	unsigned long rc;
	tSirResultCodes timeout_reason = 0;
	bool ok;
	mac_handle_t mac_handle;
	uint32_t conn_info_freq;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* validate config */
	if (!hdd_ctx->config) {
		hdd_err("config is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/*
	 * reset scan reject params if connection is success or we received
	 * final failure from CSR after trying with all APs.
	 */
	hdd_reset_scan_reject_params(hdd_ctx, roam_status, roam_result);

	/* HDD has initiated disconnect, do not send connect result indication
	 * to kernel as it will be handled by __cfg80211_disconnect.
	 */
	if (((eConnectionState_Disconnecting ==
	    sta_ctx->conn_info.conn_state) ||
	    (eConnectionState_NotConnected ==
	    sta_ctx->conn_info.conn_state)) &&
	    ((eCSR_ROAM_RESULT_ASSOCIATED == roam_result) ||
	    (eCSR_ROAM_ASSOCIATION_FAILURE == roam_status))) {
		hdd_info("hddDisconInProgress state=%d, result=%d, status=%d",
				sta_ctx->conn_info.conn_state,
				roam_result, roam_status);
		hddDisconInProgress = true;
	}

	mac_handle = hdd_ctx->mac_handle;

	if (eCSR_ROAM_RESULT_ASSOCIATED == roam_result) {
		if (!roam_info) {
			hdd_err("roam_info is NULL");
			return QDF_STATUS_E_FAILURE;
		}
		if (!hddDisconInProgress) {
			hdd_conn_set_connection_state(adapter,
						   eConnectionState_Associated);
		}

		/*
		 * Enable roaming on other STA iface except this one.
		 * Firmware dosent support connection on one STA iface while
		 * roaming on other STA iface
		 */
		wlan_hdd_enable_roaming(adapter, RSO_CONNECT_START);

		/* Save the connection info from CSR... */
		hdd_conn_save_connect_info(adapter, roam_info,
					   eCSR_BSS_TYPE_INFRASTRUCTURE);

		if (hdd_add_beacon_filter(adapter) != 0)
			hdd_err("hdd_add_beacon_filter() failed");
#ifdef FEATURE_WLAN_WAPI
		if (roam_info->u.pConnectedProfile->AuthType ==
		    eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE
		    || roam_info->u.pConnectedProfile->AuthType ==
		    eCSR_AUTH_TYPE_WAPI_WAI_PSK) {
			adapter->wapi_info.is_wapi_sta = true;
		} else {
			adapter->wapi_info.is_wapi_sta = false;
		}
#endif /* FEATURE_WLAN_WAPI */
		hdd_debug("bss_descr[%d] devicemode[%d]", !!roam_info->bss_desc,
			  adapter->device_mode);
		if ((QDF_STA_MODE == adapter->device_mode) &&
						roam_info->bss_desc) {
			ie_len = GET_IE_LEN_IN_BSS(roam_info->bss_desc->length);
			sta_ctx->ap_supports_immediate_power_save =
				wlan_hdd_is_ap_supports_immediate_power_save(
				     (uint8_t *)roam_info->bss_desc->ieFields,
				     ie_len);
			hdd_debug("ap_supports_immediate_power_save flag [%d]",
				  sta_ctx->ap_supports_immediate_power_save);
		}

		/* Indicate 'connect' status to user space */
		hdd_send_association_event(dev, roam_info);

		if (policy_mgr_is_mcc_in_24G(hdd_ctx->psoc)) {
			if (hdd_ctx->miracast_value)
				wlan_hdd_set_mas(adapter,
					hdd_ctx->miracast_value);
		}

		/* Initialize the Linkup event completion variable */
		INIT_COMPLETION(adapter->linkup_event_var);

		/*
		 * Sometimes Switching ON the Carrier is taking time to activate
		 * the device properly. Before allowing any packet to go up to
		 * the application, device activation has to be ensured for
		 * proper queue mapping by the kernel. we have registered net
		 * device notifier for device change notification. With this we
		 * will come to know that the device is getting
		 * activated properly.
		 */
		if (sta_ctx->ft_carrier_on == false) {
			/*
			 * Enable Linkup Event Servicing which allows the net
			 * device notifier to set the linkup event variable.
			 */
			adapter->is_link_up_service_needed = true;

			/* Switch on the Carrier to activate the device */
			wlan_hdd_netif_queue_control(adapter,
						WLAN_NETIF_CARRIER_ON,
						WLAN_CONTROL_PATH);

			/*
			 * Wait for the Link to up to ensure all the queues
			 * are set properly by the kernel.
			 */
			rc = wait_for_completion_timeout(
					&adapter->linkup_event_var,
					msecs_to_jiffies(ASSOC_LINKUP_TIMEOUT)
					);
			if (!rc)
				hdd_warn("Warning:ASSOC_LINKUP_TIMEOUT");

			/*
			 * Disable Linkup Event Servicing - no more service
			 * required from the net device notifier call.
			 */
			adapter->is_link_up_service_needed = false;
		} else {
			sta_ctx->ft_carrier_on = false;
			ft_carrier_on = true;
		}

		if (ucfg_ipa_is_enabled())
			ucfg_ipa_wlan_evt(hdd_ctx->pdev, adapter->dev,
					  adapter->device_mode,
					  adapter->vdev_id,
					  WLAN_IPA_STA_CONNECT,
					  roam_info->bssid.bytes);

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
		wlan_hdd_auto_shutdown_enable(hdd_ctx, false);
#endif

		conn_info_freq = sta_ctx->conn_info.chan_freq;

		hdd_debug("check if STA chan ok for DNBS");
		if (policy_mgr_is_chan_ok_for_dnbs(hdd_ctx->psoc,
						   conn_info_freq, &ok)) {
			hdd_err("Unable to check DNBS eligibility for chan(freq):%u",
				sta_ctx->conn_info.chan_freq);
			return QDF_STATUS_E_FAILURE;
		}

		if (!ok) {
			hdd_err("Chan(freq):%u not suitable for DNBS",
				sta_ctx->conn_info.chan_freq);
			wlan_hdd_netif_queue_control(adapter,
				WLAN_NETIF_CARRIER_OFF,
				WLAN_CONTROL_PATH);
			if (!hddDisconInProgress) {
				hdd_err("Disconnecting...");
				sme_roam_disconnect(
					mac_handle,
					adapter->vdev_id,
					eCSR_DISCONNECT_REASON_UNSPECIFIED);
			}
			return QDF_STATUS_E_FAILURE;
		}

		DPTRACE(qdf_dp_trace_mgmt_pkt(QDF_DP_TRACE_MGMT_PACKET_RECORD,
			adapter->vdev_id,
			QDF_TRACE_DEFAULT_PDEV_ID,
			QDF_PROTO_TYPE_MGMT, QDF_PROTO_MGMT_ASSOC));

		reqRsnIe = qdf_mem_malloc(sizeof(uint8_t) *
					  DOT11F_IE_RSN_MAX_LEN);
		if (!reqRsnIe)
			return QDF_STATUS_E_NOMEM;

		/*
		 * For reassoc, the station is already registered, all we need
		 * is to change the state of the STA in TL.
		 * If authentication is required (WPA/WPA2/DWEP), change TL to
		 * CONNECTED instead of AUTHENTICATED.
		 */
		if (!roam_info->fReassocReq) {
			struct cfg80211_bss *bss;
			u8 *assoc_rsp = NULL;
			unsigned int assoc_rsp_len = 0;
			u8 *assoc_req = NULL;
			unsigned int assoc_req_len = 0;
			struct ieee80211_channel *chan;
			uint32_t rsp_rsn_lemgth = DOT11F_IE_RSN_MAX_LEN;
			uint8_t *rsp_rsn_ie =
				qdf_mem_malloc(sizeof(uint8_t) *
					       DOT11F_IE_RSN_MAX_LEN);
			if (!rsp_rsn_ie) {
				qdf_mem_free(reqRsnIe);
				return QDF_STATUS_E_NOMEM;
			}


			/* add bss_id to cfg80211 data base */
			bss =
				wlan_hdd_cfg80211_update_bss_db(adapter,
								roam_info);
			if (!bss) {
				hdd_err("wlan: Not able to create BSS entry");
				wlan_hdd_netif_queue_control(adapter,
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
					   mac_handle,
					   adapter->vdev_id,
					   eCSR_DISCONNECT_REASON_UNSPECIFIED);
				}
				qdf_mem_free(reqRsnIe);
				qdf_mem_free(rsp_rsn_ie);
				return QDF_STATUS_E_FAILURE;
			}

			cfg80211_put_bss(hdd_ctx->wiphy, bss);

			/* Association Response */
			assoc_rsp =
				(u8 *) (roam_info->pbFrames +
					roam_info->nBeaconLength +
					roam_info->nAssocReqLength);
			if (assoc_rsp) {
				/*
				 * assoc_rsp needs to point to the IEs
				 */
				assoc_rsp += FT_ASSOC_RSP_IES_OFFSET;
				assoc_rsp_len =
					roam_info->nAssocRspLength -
					FT_ASSOC_RSP_IES_OFFSET;

				hdd_debug("assoc_rsp_len %d", assoc_rsp_len);
				hdd_debug("Assoc Rsp IE dump");
				QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD,
						   QDF_TRACE_LEVEL_DEBUG,
						   assoc_rsp,
						   assoc_rsp_len);
			} else {
				hdd_debug("AssocRsp is NULL");
				assoc_rsp_len = 0;
			}

			/* Association Request */
			assoc_req = (u8 *) (roam_info->pbFrames +
					      roam_info->nBeaconLength);
			if (assoc_req) {
				if (!ft_carrier_on) {
					/*
					 * assoc_req needs to point to
					 * the IEs
					 */
					assoc_req +=
						FT_ASSOC_REQ_IES_OFFSET;
					hdd_debug("assoc_req is now at %02x%02x",
						 (unsigned int)
						 assoc_req[0],
						 (unsigned int)
						 assoc_req[1]);
					assoc_req_len =
					    roam_info->nAssocReqLength -
						FT_ASSOC_REQ_IES_OFFSET;
				} else {
					/*
					 * This should contain only the
					 * FTIEs
					 */
					assoc_req_len =
					    roam_info->nAssocReqLength;
				}

				hdd_debug("assoc_req_len %d", assoc_req_len);
				hdd_debug("Assoc/Reassoc Req IE dump");
				QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD,
						   QDF_TRACE_LEVEL_DEBUG,
						   assoc_req,
						   assoc_req_len);
			} else {
				hdd_debug("AssocReq is NULL");
				assoc_req_len = 0;
			}

			if ((roam_info->u.pConnectedProfile->AuthType ==
			     eCSR_AUTH_TYPE_FT_RSN) ||
			    (roam_info->u.pConnectedProfile->AuthType ==
			     eCSR_AUTH_TYPE_FT_RSN_PSK) ||
			    (roam_info->u.pConnectedProfile->AuthType ==
			     eCSR_AUTH_TYPE_FT_SAE) ||
			    (roam_info->u.pConnectedProfile->AuthType ==
			     eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384)) {
				if (ft_carrier_on) {
					if (!hddDisconInProgress &&
						roam_info->bss_desc) {
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
								(roam_info)) {
						hdd_roam_decr_conn_count(
							 adapter, hdd_ctx);

						hdd_green_ap_start_state_mc(
							hdd_ctx,
							adapter->device_mode,
							false);
						}
						hdd_debug("ft_carrier_on is %d, sending roamed indication",
							 ft_carrier_on);
						chan = ieee80211_get_channel(
							adapter->wdev.wiphy,
							roam_info->bss_desc->chan_freq);

						roam_bss =
							wlan_cfg80211_get_bss(
							adapter->wdev.wiphy,
							chan,
							roam_info->bssid.bytes,
							roam_info->u.
							pConnectedProfile->SSID.ssId,
							roam_info->u.
							pConnectedProfile->SSID.length);

						cdp_hl_fc_set_td_limit(soc,
							adapter->vdev_id,
							conn_info_freq);

						hdd_send_roamed_ind(
								dev,
								roam_bss,
								assoc_req,
								assoc_req_len,
								assoc_rsp,
								assoc_rsp_len);
						wlan_hdd_send_roam_auth_event(
							adapter,
							roam_info->bssid.bytes,
							assoc_req,
							assoc_req_len,
							assoc_rsp,
							assoc_rsp_len,
							roam_info);
					}
					if (sme_get_ftptk_state
						    (mac_handle,
						    adapter->vdev_id)) {
						sme_set_ftptk_state
							(mac_handle,
							adapter->vdev_id,
							false);
						roam_info->fAuthRequired =
							false;

						qdf_mem_copy(sta_ctx->
							     roam_info.bssid,
							     roam_info->bssid.bytes,
							     QDF_MAC_ADDR_SIZE);
						qdf_mem_copy(sta_ctx->
							     roam_info.peer_mac,
							     roam_info->peerMac.bytes,
							     QDF_MAC_ADDR_SIZE);
						sta_ctx->roam_info.roam_id =
							roam_id;
						sta_ctx->roam_info.roam_status =
							roam_status;
						sta_ctx->roam_info.
						defer_key_complete = true;
					}
				} else if (!hddDisconInProgress) {
					hdd_debug("ft_carrier_on is %d, sending connect indication",
						 ft_carrier_on);
					cdp_hl_fc_set_td_limit(soc,
							       adapter->vdev_id,
							       conn_info_freq);
					hdd_connect_result(dev,
							   roam_info->
							   bssid.bytes,
							   roam_info,
							   assoc_req,
							   assoc_req_len,
							   assoc_rsp,
							   assoc_rsp_len,
							   WLAN_STATUS_SUCCESS,
							   GFP_KERNEL, false,
							   roam_info->status_code);
				}
			} else {
				/*
				 * wpa supplicant expecting WPA/RSN IE in
				 * connect result.
				 */

				sme_roam_get_wpa_rsn_req_ie(mac_handle,
							    adapter->vdev_id,
							    &reqRsnLength,
							    reqRsnIe);

				sme_roam_get_wpa_rsn_rsp_ie(mac_handle,
							    adapter->vdev_id,
							    &rsp_rsn_lemgth,
							    rsp_rsn_ie);
				if (!hddDisconInProgress) {
					if (ft_carrier_on)
						hdd_send_re_assoc_event(dev,
									adapter,
									roam_info,
									reqRsnIe,
									reqRsnLength);
					else {
						hdd_debug("sending connect indication to nl80211:for bssid "
							 QDF_MAC_ADDR_STR
							 " result:%d and Status:%d",
							 QDF_MAC_ADDR_ARRAY
							 (roam_info->bssid.bytes),
							 roam_result, roam_status);

						/* inform connect result to nl80211 */
						hdd_connect_result(dev,
								   roam_info->
								   bssid.bytes,
								   roam_info,
								   assoc_req,
								   assoc_req_len,
								   assoc_rsp,
								   assoc_rsp_len,
								   WLAN_STATUS_SUCCESS,
								   GFP_KERNEL,
								   false,
								   roam_info->status_code);
					}
					cdp_hl_fc_set_td_limit(soc,
							       adapter->vdev_id,
							       conn_info_freq);
				}
			}
			if (!hddDisconInProgress) {
				/*
				 * Perform any WMM-related association
				 * processing.
				 */
				hdd_wmm_assoc(adapter, roam_info,
					      eCSR_BSS_TYPE_INFRASTRUCTURE);

				/*
				 * Register the Station with DP after associated
				 */
				qdf_status = hdd_roam_register_sta(adapter,
						roam_info,
						roam_info->bss_desc);
				hdd_debug("Enabling queues");
				wlan_hdd_netif_queue_control(adapter,
						WLAN_WAKE_ALL_NETIF_QUEUE,
						WLAN_CONTROL_PATH);

			}
			qdf_mem_free(rsp_rsn_ie);
		} else {
			/*
			 * wpa supplicant expecting WPA/RSN IE in connect result
			 * in case of reassociation also need to indicate it to
			 * supplicant.
			 */
			sme_roam_get_wpa_rsn_req_ie(
						mac_handle,
						adapter->vdev_id,
						&reqRsnLength, reqRsnIe);

			cdp_hl_fc_set_td_limit(soc, adapter->vdev_id,
					       conn_info_freq);
			hdd_send_re_assoc_event(dev, adapter, roam_info,
						reqRsnIe, reqRsnLength);
			/* Reassoc successfully */
			if (roam_info->fAuthRequired) {
				qdf_status =
					hdd_change_peer_state(adapter,
						roam_info->bssid.bytes,
						OL_TXRX_PEER_STATE_CONN,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						roam_info->roamSynchInProgress
#else
						false
#endif
						);
				hdd_conn_set_authenticated(adapter, false);
				hdd_objmgr_set_peer_mlme_auth_state(
							adapter->vdev,
							false);
			} else {
				hdd_debug("sta: " QDF_MAC_ADDR_STR
					  "Changing TL state to AUTHENTICATED",
					  QDF_MAC_ADDR_ARRAY(
					  roam_info->bssid.bytes));
				qdf_status =
					hdd_change_peer_state(adapter,
						roam_info->bssid.bytes,
						OL_TXRX_PEER_STATE_AUTH,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
						roam_info->roamSynchInProgress
#else
						false
#endif
						);
				hdd_conn_set_authenticated(adapter, true);
				hdd_objmgr_set_peer_mlme_auth_state(
							adapter->vdev,
							true);
			}

			if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
				/*
				 * Perform any WMM-related association
				 * processing
				 */
				hdd_wmm_assoc(adapter, roam_info,
					      eCSR_BSS_TYPE_INFRASTRUCTURE);
			}

			/* Start the tx queues */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
			if (roam_info->roamSynchInProgress)
				hdd_debug("LFR3:netif_tx_wake_all_queues");
#endif
			hdd_debug("Enabling queues");
			wlan_hdd_netif_queue_control(adapter,
						   WLAN_WAKE_ALL_NETIF_QUEUE,
						   WLAN_CONTROL_PATH);
		}
		qdf_mem_free(reqRsnIe);

		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("STA register with TL failed status: %d [%08X]",
				qdf_status, qdf_status);
		}
#ifdef WLAN_FEATURE_11W
		qdf_mem_zero(&adapter->hdd_stats.hdd_pmf_stats,
			     sizeof(adapter->hdd_stats.hdd_pmf_stats));
#endif
		if (adapter->device_mode == QDF_STA_MODE)
			ucfg_blm_update_bssid_connect_params(hdd_ctx->pdev,
							     roam_info->bssid,
							     BLM_AP_CONNECTED);

		policy_mgr_check_n_start_opportunistic_timer(hdd_ctx->psoc);
		hdd_debug("check for SAP restart");
		policy_mgr_check_concurrent_intf_and_restart_sap(
			hdd_ctx->psoc);
	} else {
		bool connect_timeout = false;
		if (roam_info && roam_info->is_fils_connection &&
		    eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE == roam_result)
			qdf_copy_macaddr(&roam_info->bssid,
					 &sta_ctx->requested_bssid);
		if (roam_info)
			hdd_err("%s(vdevid-%d): connection failed with " QDF_MAC_ADDR_STR
				 " result: %d and Status: %d", dev->name,
				 adapter->vdev_id,
				 QDF_MAC_ADDR_ARRAY(roam_info->bssid.bytes),
				 roam_result, roam_status);
		else
			hdd_err("%s(vdevid-%d): connection failed with " QDF_MAC_ADDR_STR
				 " result: %d and Status: %d", dev->name,
				 adapter->vdev_id,
				 QDF_MAC_ADDR_ARRAY(sta_ctx->requested_bssid.bytes),
				 roam_result, roam_status);

		if ((eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE == roam_result) ||
		   (roam_info &&
		   ((eSIR_SME_JOIN_TIMEOUT_RESULT_CODE ==
					roam_info->status_code) ||
		   (eSIR_SME_AUTH_TIMEOUT_RESULT_CODE ==
					roam_info->status_code) ||
		   (eSIR_SME_ASSOC_TIMEOUT_RESULT_CODE ==
					roam_info->status_code)))) {
			uint8_t *ssid;
			uint8_t ssid_len;

			if (roam_info && roam_info->pProfile &&
			    roam_info->pProfile->SSIDs.SSIDList) {
				ssid_len =
					roam_info->pProfile->SSIDs.
						SSIDList[0].SSID.length;
				ssid = roam_info->pProfile->SSIDs.
						SSIDList[0].SSID.ssId;
			} else {
				ssid_len = sta_ctx->conn_info.ssid.SSID.length;
				ssid = sta_ctx->conn_info.ssid.SSID.ssId;
			}
			wlan_hdd_cfg80211_unlink_bss(adapter,
				roam_info ?
				roam_info->bssid.bytes :
				sta_ctx->requested_bssid.bytes, ssid, ssid_len);
			sme_remove_bssid_from_scan_list(mac_handle,
				roam_info ?
				roam_info->bssid.bytes :
				sta_ctx->requested_bssid.bytes);
			if (roam_result !=
			    eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE)
				connect_timeout = true;
		}

		/*
		 * CR465478: Only send up a connection failure result when CSR
		 * has completed operation - with a ASSOCIATION_FAILURE status.
		 */
		if (eCSR_ROAM_ASSOCIATION_FAILURE == roam_status
		    && !hddDisconInProgress) {
			u8 *assoc_rsp = NULL;
			u8 *assoc_req = NULL;

			if (roam_info) {
				if (roam_info->pbFrames) {
				/* Association Request */
					assoc_req =
						(u8 *)(roam_info->pbFrames +
						      roam_info->nBeaconLength);
					/* Association Response */
					assoc_rsp =
						(u8 *)(roam_info->pbFrames +
						      roam_info->nBeaconLength +
						    roam_info->nAssocReqLength);
				}
				hdd_err("send connect failure to nl80211: for bssid "
					QDF_MAC_ADDR_STR
					" result: %d and Status: %d reasoncode: %d",
					QDF_MAC_ADDR_ARRAY(roam_info->bssid.bytes),
					roam_result, roam_status,
					roam_info->reasonCode);
				sta_ctx->conn_info.assoc_status_code =
					roam_info->status_code;
			} else {
				hdd_err("connect failed: for bssid "
				       QDF_MAC_ADDR_STR
				       " result: %d and status: %d ",
				       QDF_MAC_ADDR_ARRAY(sta_ctx->requested_bssid.bytes),
				       roam_result, roam_status);
			}
			hdd_debug("Invoking packetdump deregistration API");
			wlan_deregister_txrx_packetdump(OL_TXRX_PDEV_ID);

			/* inform association failure event to nl80211 */
			if (eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL ==
			    roam_result) {
				if (roam_info)
					hdd_connect_result(dev,
						roam_info->bssid.bytes,
						roam_info, assoc_req,
						roam_info->nAssocReqLength,
						assoc_rsp,
						roam_info->nAssocRspLength,
						WLAN_STATUS_ASSOC_DENIED_UNSPEC,
						GFP_KERNEL,
						connect_timeout,
						roam_info->status_code);
				else
					hdd_connect_result(dev,
						sta_ctx->requested_bssid.bytes,
						NULL, NULL, 0, NULL, 0,
						WLAN_STATUS_ASSOC_DENIED_UNSPEC,
						GFP_KERNEL,
						connect_timeout,
						timeout_reason);
			} else {
				if (roam_info)
					hdd_connect_result(dev,
						roam_info->bssid.bytes,
						roam_info, assoc_req,
						roam_info->nAssocReqLength,
						assoc_rsp,
						roam_info->nAssocRspLength,
						roam_info->reasonCode ?
						roam_info->reasonCode :
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL,
						connect_timeout,
						roam_info->status_code);
				else
					hdd_connect_result(dev,
						sta_ctx->requested_bssid.bytes,
						NULL, NULL, 0, NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL,
						connect_timeout,
						timeout_reason);
			}
			hdd_clear_roam_profile_ie(adapter);
			sme_reset_key(hdd_ctx->mac_handle,
				      adapter->vdev_id);
		} else  if ((eCSR_ROAM_CANCELLED == roam_status
		    && !hddDisconInProgress)) {
			hdd_connect_result(dev,
					   sta_ctx->requested_bssid.bytes,
					   NULL, NULL, 0, NULL, 0,
					   WLAN_STATUS_UNSPECIFIED_FAILURE,
					   GFP_KERNEL,
					   connect_timeout,
					   timeout_reason);
		}

		if (roam_status == eCSR_ROAM_ASSOCIATION_FAILURE ||
		    roam_status == eCSR_ROAM_CANCELLED) {
			/* notify connect faiilure on final failure */
			ucfg_tdls_notify_connect_failure(hdd_ctx->psoc);
			/* do we need to change the HW mode on final failure */
			policy_mgr_check_n_start_opportunistic_timer(
								hdd_ctx->psoc);

			/*
			 * Enable roaming on other STA iface except this one.
			 * Firmware dosent support connection on one STA iface
			 * while roaming on other STA iface
			 */
			wlan_hdd_enable_roaming(adapter, RSO_CONNECT_START);
		}

		/*
		 * Set connection state to eConnectionState_NotConnected only
		 * when CSR has completed operation - with a
		 * ASSOCIATION_FAILURE or eCSR_ROAM_CANCELLED status.
		 */
		if (((eCSR_ROAM_ASSOCIATION_FAILURE == roam_status) ||
			(eCSR_ROAM_CANCELLED == roam_status))
		    && !hddDisconInProgress) {
			hdd_conn_set_connection_state(adapter,
					eConnectionState_NotConnected);
		}
		hdd_wmm_init(adapter);

		hdd_debug("Disabling queues");
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					   WLAN_CONTROL_PATH);
		/*
		 * if hddDisconInProgress is set and roam_result is
		 * eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE that mean HDD is
		 * waiting on disconnect_comp_var so unblock anyone waiting for
		 * disconnect to complete.
		 */
		/*
		 * Also add eCSR_ROAM_ASSOCIATION_FAILURE and
		 * eCSR_ROAM_CANCELLED here to handle the following scenarios:
		 *
		 * 1. Connection is in progress, but completes with failure
		 *    before we check the CSR state.
		 * 2. Connection is in progress. But the connect command is in
		 *    pending queue and is removed from pending queue as part
		 *    of csr_roam_disconnect.
		 */
		if ((roam_result == eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE ||
		     roam_status == eCSR_ROAM_ASSOCIATION_FAILURE ||
		     roam_status == eCSR_ROAM_CANCELLED) &&
		    hddDisconInProgress)
			complete(&adapter->disconnect_comp_var);
	}

	return QDF_STATUS_SUCCESS;
}

bool hdd_save_peer(struct hdd_station_ctx *sta_ctx,
		   struct qdf_mac_addr *peer_mac_addr)
{
	int idx;
	struct qdf_mac_addr *mac_addr;

	for (idx = 0; idx < SIR_MAX_NUM_STA_IN_IBSS; idx++) {
		mac_addr = &sta_ctx->conn_info.peer_macaddr[idx];
		if (qdf_is_macaddr_zero(mac_addr)) {
			hdd_debug("adding peer: %pM at idx: %d",
				  peer_mac_addr, idx);
			qdf_copy_macaddr(mac_addr, peer_mac_addr);
			return true;
		}
	}

	return false;
}

void hdd_delete_peer(struct hdd_station_ctx *sta_ctx,
		     struct qdf_mac_addr *peer_mac_addr)
{
	int i;
	struct qdf_mac_addr *mac_addr;

	for (i = 0; i < SIR_MAX_NUM_STA_IN_IBSS; i++) {
		mac_addr = &sta_ctx->conn_info.peer_macaddr[i];
		if (qdf_is_macaddr_equal(mac_addr, peer_mac_addr)) {
			qdf_zero_macaddr(mac_addr);
			return;
		}
	}
}

/**
 * roam_remove_ibss_station() - Remove the IBSS peer MAC address in the adapter
 * @adapter: pointer to adapter
 * @sta_id: station id
 *
 * Return:
 *	true if we remove MAX_PEERS or less STA
 *	false otherwise.
 */
static bool roam_remove_ibss_station(struct hdd_adapter *adapter,
				     uint8_t *mac_addr)
{
	bool successful = false;
	int idx = 0;
	uint8_t valid_idx = 0;
	uint8_t del_idx = 0;
	uint8_t empty_slots = 0;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	for (idx = 0; idx < MAX_PEERS; idx++) {
		if (WLAN_ADDR_EQ(mac_addr,
				 sta_ctx->conn_info.peer_macaddr[idx].bytes)) {
			qdf_zero_macaddr(&sta_ctx->conn_info.peer_macaddr[idx]);

			successful = true;

			/*
			 * Note the deleted Index, if its 0 we need special
			 * handling.
			 */
			del_idx = idx;

			empty_slots++;
		} else {
			if (hdd_is_valid_mac_address(
			    sta_ctx->conn_info.peer_macaddr[idx].bytes))
				valid_idx = idx;
			else
				empty_slots++;
		}
	}

	if (MAX_PEERS == empty_slots) {
		/* Last peer departed, set the IBSS state appropriately */
		hdd_conn_set_connection_state(adapter,
				eConnectionState_IbssDisconnected);
		hdd_debug("Last IBSS Peer Departed!!!");
	}
	/* Find next active sta_id, to have a valid sta trigger for TL. */
	if (successful) {
		if (del_idx == 0) {
			if (hdd_is_valid_mac_address(
			    sta_ctx->conn_info.peer_macaddr[idx].bytes)) {
				qdf_copy_macaddr(&sta_ctx->conn_info.
						 peer_macaddr[0],
						 &sta_ctx->conn_info.
						 peer_macaddr[valid_idx]);

				qdf_zero_macaddr(&sta_ctx->conn_info.
						 peer_macaddr[valid_idx]);
			}
		}
	}
	return successful;
}

/**
 * roam_ibss_connect_handler() - IBSS connection handler
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 *
 * We update the status of the IBSS to connected in this function.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS roam_ibss_connect_handler(struct hdd_adapter *adapter,
					    struct csr_roam_info *roam_info)
{
	struct cfg80211_bss *bss;
	/*
	 * Set the internal connection state to show 'IBSS Connected' (IBSS with
	 * a partner stations).
	 */
	hdd_conn_set_connection_state(adapter, eConnectionState_IbssConnected);

	/* Save the connection info from CSR... */
	hdd_conn_save_connect_info(adapter, roam_info, eCSR_BSS_TYPE_IBSS);

	/* Send the bssid address to the wext. */
	hdd_send_association_event(adapter->dev, roam_info);
	/* add bss_id to cfg80211 data base */
	bss = wlan_hdd_cfg80211_update_bss_db(adapter, roam_info);
	if (!bss) {
		hdd_err("%s: unable to create IBSS entry",
		       adapter->dev->name);
		return QDF_STATUS_E_FAILURE;
	}
	cfg80211_put_bss(
		WLAN_HDD_GET_CTX(adapter)->wiphy,
		bss);

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_roam_mic_error_indication_handler() - MIC error indication handler
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * This function indicates the Mic failure to the supplicant
 *
 * Return: None
 */
static void
hdd_roam_mic_error_indication_handler(struct hdd_adapter *adapter,
				      struct csr_roam_info *roam_info)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	tSirMicFailureInfo *mic_failure_info;

	if (eConnectionState_Associated != sta_ctx->conn_info.conn_state)
		return;

	mic_failure_info = roam_info->u.pMICFailureInfo;
	cfg80211_michael_mic_failure(adapter->dev,
				     mic_failure_info->taMacAddr,
				     mic_failure_info->multicast ?
					NL80211_KEYTYPE_GROUP :
					NL80211_KEYTYPE_PAIRWISE,
				     mic_failure_info->keyId,
				     mic_failure_info->TSC,
				     GFP_KERNEL);
}

#ifdef CRYPTO_SET_KEY_CONVERGED
static QDF_STATUS wlan_hdd_set_key_helper(struct hdd_adapter *adapter,
					  uint32_t *roam_id)
{
	int ret;
	struct wlan_objmgr_vdev *vdev;

	vdev = hdd_objmgr_get_vdev(adapter);
	if (!vdev)
		return QDF_STATUS_E_FAILURE;
	ret = wlan_cfg80211_crypto_add_key(vdev,
					   WLAN_CRYPTO_KEY_TYPE_UNICAST, 0);
	hdd_objmgr_put_vdev(vdev);
	if (ret != 0) {
		hdd_err("crypto add key fail, status: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS wlan_hdd_set_key_helper(struct hdd_adapter *adapter,
					  uint32_t *roam_id)
{
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	return sme_roam_set_key(hdd_ctx->mac_handle,
				adapter->vdev_id,
				&sta_ctx->ibss_enc_key,
				roam_id);
}
#endif

/**
 * roam_roam_connect_status_update_handler() - IBSS connect status update
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * The Ibss connection status is updated regularly here in this function.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
roam_roam_connect_status_update_handler(struct hdd_adapter *adapter,
					struct csr_roam_info *roam_info,
					uint32_t roam_id,
					eRoamCmdStatus roam_status,
					eCsrRoamResult roam_result)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS qdf_status;

	switch (roam_result) {
	case eCSR_ROAM_RESULT_IBSS_NEW_PEER:
	{
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		struct station_info *stainfo;
		eCsrEncryptionType encr_type = sta_ctx->ibss_enc_key.encType;

		hdd_debug("IBSS New Peer indication from SME "
			 "with peerMac " QDF_MAC_ADDR_STR " BSSID: "
			 QDF_MAC_ADDR_STR " and stationID= %d",
			 QDF_MAC_ADDR_ARRAY(roam_info->peerMac.bytes),
			 QDF_MAC_ADDR_ARRAY(sta_ctx->conn_info.bssid.bytes),
			 roam_info->staId);

		if (!hdd_save_peer
			    (WLAN_HDD_GET_STATION_CTX_PTR(adapter),
			    &roam_info->peerMac)) {
			hdd_warn("Max reached: Can't register new IBSS peer");
			break;
		}

		if (hdd_is_key_install_required_for_ibss(encr_type))
			roam_info->fAuthRequired = true;

		/* Register the Station with datapath for the new peer. */
		qdf_status = hdd_roam_register_sta(adapter,
						   roam_info,
						   roam_info->bss_desc);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hdd_err("Cannot register STA for IBSS. qdf_status: %d [%08X]",
				qdf_status, qdf_status);
		}
		sta_ctx->ibss_sta_generation++;
		stainfo = qdf_mem_malloc(sizeof(*stainfo));
		if (!stainfo)
			return QDF_STATUS_E_NOMEM;

		stainfo->filled = 0;
		stainfo->generation = sta_ctx->ibss_sta_generation;

		cfg80211_new_sta(adapter->dev,
				 (const u8 *)roam_info->peerMac.bytes,
				 stainfo, GFP_KERNEL);
		qdf_mem_free(stainfo);

		if (hdd_is_key_install_required_for_ibss(encr_type)) {
			sta_ctx->ibss_enc_key.keyDirection =
				eSIR_TX_RX;
			qdf_copy_macaddr(&sta_ctx->ibss_enc_key.peerMac,
					 &roam_info->peerMac);
			vdev = hdd_objmgr_get_vdev(adapter);
			if (!vdev)
				return QDF_STATUS_E_FAILURE;
			wlan_crypto_update_set_key_peer(vdev, true, 0,
							&roam_info->peerMac);
			hdd_objmgr_put_vdev(vdev);

			hdd_debug("New peer joined set PTK encType=%d",
				 encr_type);
			qdf_status = wlan_hdd_set_key_helper(adapter, &roam_id);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				hdd_err("sme set_key fail status: %d",
					qdf_status);
				return QDF_STATUS_E_FAILURE;
			}
		}
		hdd_debug("Enabling queues");
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
					   WLAN_CONTROL_PATH);
		break;
	}

	case eCSR_ROAM_RESULT_IBSS_CONNECT:
	{

		roam_ibss_connect_handler(adapter, roam_info);

		break;
	}
	case eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED:
	{
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		if (!roam_remove_ibss_station(adapter,
					      roam_info->peerMac.bytes))
			hdd_warn("IBSS peer departed by cannot find peer in our registration table with TL");

		hdd_debug("IBSS Peer Departed from SME "
			 "with peerMac " QDF_MAC_ADDR_STR " BSSID: "
			 QDF_MAC_ADDR_STR " and stationID= %d",
			 QDF_MAC_ADDR_ARRAY(roam_info->peerMac.bytes),
			 QDF_MAC_ADDR_ARRAY(sta_ctx->conn_info.bssid.bytes),
			 roam_info->staId);

		hdd_roam_deregister_sta(adapter, roam_info->peerMac);

		sta_ctx->ibss_sta_generation++;

		cfg80211_del_sta(adapter->dev,
				 (const u8 *)&roam_info->peerMac.bytes,
				 GFP_KERNEL);
		break;
	}
	case eCSR_ROAM_RESULT_IBSS_INACTIVE:
	{
		hdd_debug("Received eCSR_ROAM_RESULT_IBSS_INACTIVE from SME");
		/* Stop only when we are inactive */
		hdd_debug("Disabling queues");
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
					   WLAN_CONTROL_PATH);
		hdd_conn_set_connection_state(adapter,
					      eConnectionState_NotConnected);

		/* Send the bssid address to the wext. */
		hdd_send_association_event(adapter->dev, roam_info);
		break;
	}
	default:
		break;

	}

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_TDLS
QDF_STATUS hdd_roam_register_tdlssta(struct hdd_adapter *adapter,
				     const uint8_t *peerMac, uint8_t qos)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type txrx_desc = { 0 };
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	void *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct cdp_vdev *txrx_vdev;

	/*
	 * TDLS sta in BSS should be set as STA type TDLS and STA MAC should
	 * be peer MAC, here we are working on direct Link
	 */
	WLAN_ADDR_COPY(txrx_desc.peer_addr.bytes, peerMac);

	/* set the QoS field appropriately .. */
	txrx_desc.is_qos_enabled = qos;

	/* Register the vdev transmit and receive functions */
	qdf_mem_zero(&txrx_ops, sizeof(txrx_ops));
	if (adapter->hdd_ctx->enable_dp_rx_threads) {
		txrx_ops.rx.rx = hdd_rx_pkt_thread_enqueue_cbk;
		txrx_ops.rx.rx_stack = hdd_rx_packet_cbk;
		txrx_ops.rx.rx_flush = hdd_rx_flush_packet_cbk;
		txrx_ops.rx.rx_gro_flush = hdd_rx_thread_gro_flush_ind_cbk;
	} else {
		txrx_ops.rx.rx = hdd_rx_packet_cbk;
		txrx_ops.rx.rx_stack = NULL;
		txrx_ops.rx.rx_flush = NULL;
	}
	txrx_vdev = cdp_get_vdev_from_vdev_id(soc,
					      (struct cdp_pdev *)pdev,
					      adapter->vdev_id);
	if (!txrx_vdev)
		return QDF_STATUS_E_FAILURE;

	cdp_vdev_register(soc, txrx_vdev,
			  adapter,
			  &txrx_ops);
	adapter->tx_fn = txrx_ops.tx.tx;
	txrx_ops.rx.stats_rx = hdd_tx_rx_collect_connectivity_stats_info;

	/* Register the Station with TL...  */
	qdf_status = cdp_peer_register(soc,
			(struct cdp_pdev *)pdev, &txrx_desc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed Status: %d [0x%08X]",
			qdf_status, qdf_status);
		return qdf_status;
	}

	return qdf_status;
}

QDF_STATUS hdd_roam_deregister_tdlssta(struct hdd_adapter *adapter,
				       struct qdf_mac_addr *peer_mac)
{
	QDF_STATUS qdf_status;

	qdf_status = cdp_clear_peer(cds_get_context(QDF_MODULE_ID_SOC),
			(struct cdp_pdev *)cds_get_context(QDF_MODULE_ID_TXRX),
			*peer_mac);

	return qdf_status;
}
#endif

#ifdef WLAN_FEATURE_11W

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))

static void hdd_rx_unprot_disassoc(struct net_device *dev,
				   const u8 *buf, size_t len)
{
	cfg80211_rx_unprot_mlme_mgmt(dev, buf, len);
}

static void hdd_rx_unprot_deauth(struct net_device *dev,
				 const u8 *buf, size_t len)
{
	cfg80211_rx_unprot_mlme_mgmt(dev, buf, len);
}

#else

static void hdd_rx_unprot_disassoc(struct net_device *dev,
				   const u8 *buf, size_t len)
{
	cfg80211_send_unprot_disassoc(dev, buf, len);
}

static void hdd_rx_unprot_deauth(struct net_device *dev,
				 const u8 *buf, size_t len)
{
	cfg80211_send_unprot_deauth(dev, buf, len);
}

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)) */

/**
 * hdd_indicate_unprot_mgmt_frame() - indicate unprotected management frame
 * @adapter: pointer to the adapter
 * @frame_length: Length of the unprotected frame being passed
 * @frame: Pointer to the frame buffer
 * @frame_type: 802.11 frame type
 *
 * This function forwards the unprotected management frame to the supplicant.
 *
 * Return: nothing
 */
static void
hdd_indicate_unprot_mgmt_frame(struct hdd_adapter *adapter,
			       uint32_t frame_length,
			       uint8_t *frame, uint8_t frame_type)
{
	uint8_t type, subtype;

	hdd_debug("Frame Type = %d Frame Length = %d",
		  frame_type, frame_length);

	if (hdd_validate_adapter(adapter))
		return;

	if (!frame_length) {
		hdd_err("Frame Length is Invalid ZERO");
		return;
	}

	if (!frame) {
		hdd_err("frame is NULL");
		return;
	}

	type = WLAN_HDD_GET_TYPE_FRM_FC(frame[0]);
	if (type != SIR_MAC_MGMT_FRAME) {
		hdd_warn("Unexpected frame type %d", type);
		return;
	}

	subtype = WLAN_HDD_GET_SUBTYPE_FRM_FC(frame[0]);
	switch (subtype) {
	case SIR_MAC_MGMT_DISASSOC:
		hdd_rx_unprot_disassoc(adapter->dev, frame, frame_length);
		adapter->hdd_stats.hdd_pmf_stats.num_unprot_disassoc_rx++;
		break;
	case SIR_MAC_MGMT_DEAUTH:
		hdd_rx_unprot_deauth(adapter->dev, frame, frame_length);
		adapter->hdd_stats.hdd_pmf_stats.num_unprot_deauth_rx++;
		break;
	default:
		hdd_warn("Unexpected frame subtype %d", subtype);
		break;
	}
}
#endif /* WLAN_FEATURE_11W */

#ifdef FEATURE_WLAN_ESE
/**
 * hdd_indicate_tsm_ie() - send traffic stream metrics ie
 * @adapter: pointer to adapter
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
hdd_indicate_tsm_ie(struct hdd_adapter *adapter, uint8_t tid,
		    uint8_t state, uint16_t measInterval)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	int nBytes = 0;

	if (!adapter)
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
	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_cckm_pre_auth() - send cckm preauth indication
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 *
 * This function sends cckm preauth indication to the supplicant
 * via wireless custom event.
 *
 * Return: none
 */
static void
hdd_indicate_cckm_pre_auth(struct hdd_adapter *adapter,
			   struct csr_roam_info *roam_info)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	char *pos = buf;
	int nBytes = 0, freeBytes = IW_CUSTOM_MAX;

	if ((!adapter) || (!roam_info))
		return;

	/* create the event */
	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	/* Timestamp0 is lower 32 bits and Timestamp1 is upper 32 bits */
	hdd_debug("CCXPREAUTHNOTIFY=" QDF_MAC_ADDR_STR " %d:%d",
		 QDF_MAC_ADDR_ARRAY(roam_info->bssid.bytes),
		 roam_info->timestamp[0], roam_info->timestamp[1]);

	nBytes = snprintf(pos, freeBytes, "CCXPREAUTHNOTIFY=");
	pos += nBytes;
	freeBytes -= nBytes;

	qdf_mem_copy(pos, roam_info->bssid.bytes, QDF_MAC_ADDR_SIZE);
	pos += QDF_MAC_ADDR_SIZE;
	freeBytes -= QDF_MAC_ADDR_SIZE;

	nBytes = snprintf(pos, freeBytes, " %u:%u",
			  roam_info->timestamp[0], roam_info->timestamp[1]);
	freeBytes -= nBytes;

	wrqu.data.pointer = buf;
	wrqu.data.length = (IW_CUSTOM_MAX - freeBytes);

	/* send the event */
	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_ese_adj_ap_rep_ind() - send adjacent AP report indication
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 *
 * Return: none
 */
static void
hdd_indicate_ese_adj_ap_rep_ind(struct hdd_adapter *adapter,
				struct csr_roam_info *roam_info)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX + 1];
	int nBytes = 0;

	if ((!adapter) || (!roam_info))
		return;

	/* create the event */
	memset(&wrqu, '\0', sizeof(wrqu));
	memset(buf, '\0', sizeof(buf));

	hdd_debug("CCXADJAPREP=%u", roam_info->tsmRoamDelay);

	nBytes =
		snprintf(buf, IW_CUSTOM_MAX, "CCXADJAPREP=%u",
			 roam_info->tsmRoamDelay);

	wrqu.data.pointer = buf;
	wrqu.data.length = nBytes;

	/* send the event */
	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_ese_bcn_report_no_results() - beacon report no scan results
 * @adapter: pointer to adapter
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
hdd_indicate_ese_bcn_report_no_results(const struct hdd_adapter *adapter,
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
	wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
}

/**
 * hdd_indicate_ese_bcn_report_ind() - send beacon report indication
 * @adapter: pointer to adapter
 * @roam_info: pointer to roam info
 *
 * If the measurement is none and no scan results found,
 * indicate the supplicant about measurement done.
 *
 * Return: none
 */
static void
hdd_indicate_ese_bcn_report_ind(const struct hdd_adapter *adapter,
				const struct csr_roam_info *roam_info)
{
	union iwreq_data wrqu;
	char buf[IW_CUSTOM_MAX];
	char *pos = buf;
	int nBytes = 0, freeBytes = IW_CUSTOM_MAX;
	uint8_t i = 0, len = 0;
	uint8_t tot_bcn_ieLen = 0;  /* total size of the beacon report data */
	uint8_t lastSent = 0, sendBss = 0;
	int bcnRepFieldSize =
		sizeof(roam_info->pEseBcnReportRsp->bcnRepBssInfo[0].
		       bcnReportFields);
	uint8_t ieLenByte = 1;
	/*
	 * CCXBCNREP=meas_tok<sp>flag<sp>no_of_bss<sp>tot_bcn_ie_len = 18 bytes
	 */
#define ESEBCNREPHEADER_LEN  (18)

	if ((!adapter) || (!roam_info))
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

	if ((roam_info->pEseBcnReportRsp->flag >> 1)
	    && (!roam_info->pEseBcnReportRsp->numBss)) {
		hdd_debug("Measurement Done but no scan results");
		/* If the measurement is none and no scan results found,
		 * indicate the supplicant about measurement done
		 */
		hdd_indicate_ese_bcn_report_no_results(
				adapter,
				roam_info->pEseBcnReportRsp->
				measurementToken,
				roam_info->pEseBcnReportRsp->flag,
				roam_info->pEseBcnReportRsp->numBss);
	} else {
		while (lastSent < roam_info->pEseBcnReportRsp->numBss) {
			memset(&wrqu, '\0', sizeof(wrqu));
			memset(buf, '\0', sizeof(buf));
			tot_bcn_ieLen = 0;
			sendBss = 0;
			pos = buf;
			freeBytes = IW_CUSTOM_MAX;

			for (i = lastSent;
			     i < roam_info->pEseBcnReportRsp->numBss; i++) {
				len =
					bcnRepFieldSize + ieLenByte +
					roam_info->pEseBcnReportRsp->
					bcnRepBssInfo[i].ieLen;
				if ((len + tot_bcn_ieLen) >
				    (IW_CUSTOM_MAX - ESEBCNREPHEADER_LEN)) {
					break;
				}
				tot_bcn_ieLen += len;
				sendBss++;
				hdd_debug("i(%d) sizeof bcnReportFields(%d) IeLength(%d) Length of Ie(%d) totLen(%d)",
					 i, bcnRepFieldSize, 1,
					 roam_info->pEseBcnReportRsp->
					 bcnRepBssInfo[i].ieLen, tot_bcn_ieLen);
			}

			hdd_debug("Sending %d BSS Info", sendBss);
			hdd_debug("CCXBCNREP=%d %d %d %d",
				 roam_info->pEseBcnReportRsp->measurementToken,
				 roam_info->pEseBcnReportRsp->flag, sendBss,
				 tot_bcn_ieLen);

			nBytes = snprintf(pos, freeBytes, "CCXBCNREP=%d %d %d ",
					  roam_info->pEseBcnReportRsp->
					  measurementToken,
					  roam_info->pEseBcnReportRsp->flag,
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
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       ChanNum,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Spare,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       MeasDuration,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       PhyType,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       RecvSigPower,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       ParentTsf,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       TargetTsf[0],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       TargetTsf[1],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       BcnInterval,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       CapabilityInfo,
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[0],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[1],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[2],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[3],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[4],
				       roam_info->pEseBcnReportRsp->
				       bcnRepBssInfo[i +
						     lastSent].bcnReportFields.
				       Bssid[5]);

				/* bcn report fields are copied */
				len =
					sizeof(roam_info->pEseBcnReportRsp->
					       bcnRepBssInfo[i +
							     lastSent].
					       bcnReportFields);
				qdf_mem_copy(pos,
					     (char *)&roam_info->
					     pEseBcnReportRsp->bcnRepBssInfo[i +
									     lastSent].
					     bcnReportFields, len);
				pos += len;
				freeBytes -= len;

				/* Add 1 byte of ie len */
				len =
					roam_info->pEseBcnReportRsp->
					bcnRepBssInfo[i + lastSent].ieLen;
				qdf_mem_copy(pos, (char *)&len, sizeof(len));
				pos += sizeof(len);
				freeBytes -= sizeof(len);

				/* copy IE from scan results */
				qdf_mem_copy(pos,
					     (char *)roam_info->
					     pEseBcnReportRsp->bcnRepBssInfo[i +
									     lastSent].
					     pBuf, len);
				pos += len;
				freeBytes -= len;
			}

			wrqu.data.pointer = buf;
			wrqu.data.length = IW_CUSTOM_MAX - freeBytes;

			/* send the event */
			wireless_send_event(adapter->dev, IWEVCUSTOM, &wrqu,
					    buf);
			lastSent += sendBss;
		}
	}
}

#endif /* FEATURE_WLAN_ESE */

/**
 * hdd_is_8021x_sha256_auth_type() - check authentication type to 8021x_sha256
 * @sta_ctx:	Station Context
 *
 * API to check if the connection authentication type is 8021x_sha256.
 *
 * Return: bool
 */
#ifdef WLAN_FEATURE_11W
static inline bool
hdd_is_8021x_sha256_auth_type(struct hdd_station_ctx *sta_ctx)
{
	return eCSR_AUTH_TYPE_RSN_8021X_SHA256 ==
				sta_ctx->conn_info.auth_type;
}
#else
static inline bool
hdd_is_8021x_sha256_auth_type(struct hdd_station_ctx *sta_ctx)
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
static void hdd_roam_channel_switch_handler(struct hdd_adapter *adapter,
					    struct csr_roam_info *roam_info)
{
	struct hdd_chan_change_params chan_change;
	struct cfg80211_bss *bss;
	struct net_device *dev = adapter->dev;
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct wiphy *wiphy = wdev->wiphy;
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);

	hdd_debug("channel switch for session:%d to channel freq:%d",
		adapter->vdev_id, roam_info->chan_info.mhz);

	/* Enable Roaming on STA interface which was disabled before CSA */
	if (adapter->device_mode == QDF_STA_MODE)
		sme_start_roaming(mac_handle, adapter->vdev_id,
				  REASON_DRIVER_ENABLED,
				  RSO_CHANNEL_SWITCH);

	chan_change.chan_freq = roam_info->chan_info.mhz;
	chan_change.chan_params.ch_width =
		roam_info->chan_info.ch_width;
	chan_change.chan_params.sec_ch_offset =
		roam_info->chan_info.sec_ch_offset;
	chan_change.chan_params.center_freq_seg0 =
		roam_info->chan_info.band_center_freq1;
	chan_change.chan_params.center_freq_seg1 =
		roam_info->chan_info.band_center_freq2;

	bss = wlan_hdd_cfg80211_update_bss_db(adapter, roam_info);
	if (!bss)
		hdd_err("%s: unable to create BSS entry", adapter->dev->name);
	else
		cfg80211_put_bss(wiphy, bss);

	status = hdd_chan_change_notify(adapter, adapter->dev, chan_change,
				roam_info->mode == SIR_SME_PHY_MODE_LEGACY);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("channel change notification failed");

	status = policy_mgr_set_hw_mode_on_channel_switch(hdd_ctx->psoc,
		adapter->vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_debug("set hw mode change not done");

	hdd_debug("check for SAP restart");
	policy_mgr_check_concurrent_intf_and_restart_sap(hdd_ctx->psoc);
}

/**
 * hdd_sme_roam_callback() - hdd sme roam callback
 * @context: pointer to adapter context
 * @roam_info: pointer to roam info
 * @roam_id: roam id
 * @roam_status: roam status
 * @roam_result: roam result
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS
hdd_sme_roam_callback(void *context, struct csr_roam_info *roam_info,
		      uint32_t roam_id,
		      eRoamCmdStatus roam_status, eCsrRoamResult roam_result)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	struct hdd_adapter *adapter = context;
	struct hdd_station_ctx *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct cfg80211_bss *bss_status;
	struct hdd_context *hdd_ctx;

	hdd_debug("CSR Callback: status=%d result=%d roamID=%d",
		  roam_status, roam_result, roam_id);

	/* Sanity check */
	if ((!adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return QDF_STATUS_E_FAILURE;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD, TRACE_CODE_HDD_RX_SME_MSG,
				 adapter->vdev_id, roam_status));

	switch (roam_status) {
	/*
	 * We did pre-auth,then we attempted a 11r or ese reassoc.
	 * reassoc failed due to failure, timeout, reject from ap
	 * in any case tell the OS, our carrier is off and mark
	 * interface down.
	 */
	case eCSR_ROAM_FT_REASSOC_FAILED:
		hdd_err("Reassoc Failed with roam_status: %d roam_result: %d SessionID: %d",
			 roam_status, roam_result, adapter->vdev_id);
		qdf_ret_status =
			hdd_dis_connect_handler(adapter, roam_info, roam_id,
						roam_status, roam_result);
		sta_ctx->ft_carrier_on = false;
		sta_ctx->hdd_reassoc_scenario = false;
		hdd_debug("hdd_reassoc_scenario set to: %d, ReAssoc Failed, session: %d",
			  sta_ctx->hdd_reassoc_scenario, adapter->vdev_id);
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
		hdd_debug("Disabling queues");
		hdd_debug("Roam Synch Ind: NAPI Serialize ON");
		hdd_napi_serialize(1);
		wlan_hdd_netif_queue_control(adapter,
				WLAN_STOP_ALL_NETIF_QUEUE,
				WLAN_CONTROL_PATH);
		status = hdd_roam_deregister_sta(
					adapter,
					sta_ctx->conn_info.peer_macaddr[0]);
		if (!QDF_IS_STATUS_SUCCESS(status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		sta_ctx->ft_carrier_on = true;
		sta_ctx->hdd_reassoc_scenario = true;
		hdd_debug("hdd_reassoc_scenario set to: %d, due to eCSR_ROAM_FT_START, session: %d",
			  sta_ctx->hdd_reassoc_scenario, adapter->vdev_id);
		break;
	case eCSR_ROAM_NAPI_OFF:
		hdd_debug("After Roam Synch Comp: NAPI Serialize OFF");
		hdd_napi_serialize(0);
		if (roam_result == eCSR_ROAM_RESULT_FAILURE) {
			adapter->roam_ho_fail = true;
			hdd_set_roaming_in_progress(false);
		} else {
			adapter->roam_ho_fail = false;
		}
		complete(&adapter->roaming_comp_var);
		break;
	case eCSR_ROAM_SYNCH_COMPLETE:
		hdd_debug("LFR3: Roam synch complete");
		hdd_set_roaming_in_progress(false);
		break;
	case eCSR_ROAM_SHOULD_ROAM:
		/* notify apps that we can't pass traffic anymore */
		hdd_debug("Disabling queues");
		wlan_hdd_netif_queue_control(adapter,
					   WLAN_STOP_ALL_NETIF_QUEUE,
					   WLAN_CONTROL_PATH);
		if (sta_ctx->ft_carrier_on == false) {
			wlan_hdd_netif_queue_control(adapter,
					   WLAN_NETIF_CARRIER_OFF,
					   WLAN_CONTROL_PATH);
		}
		break;
	case eCSR_ROAM_LOSTLINK:
		if (roam_result == eCSR_ROAM_RESULT_LOSTLINK) {
			hdd_debug("Roaming started due to connection lost");
			hdd_debug("Disabling queues");
			wlan_hdd_netif_queue_control(adapter,
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
		adapter->roam_ho_fail = false;
		complete(&adapter->roaming_comp_var);

		/* Call to clear any MC Addr List filter applied after
		 * successful connection.
		 */
		hdd_disable_and_flush_mc_addr_list(adapter,
			pmo_peer_disconnect);
		qdf_ret_status =
			hdd_dis_connect_handler(adapter, roam_info, roam_id,
						roam_status, roam_result);
	}
	break;
	case eCSR_ROAM_IBSS_LEAVE:
		hdd_debug("****eCSR_ROAM_IBSS_LEAVE****");
		qdf_ret_status =
			hdd_dis_connect_handler(adapter, roam_info, roam_id,
						roam_status, roam_result);
		break;
	case eCSR_ROAM_ASSOCIATION_COMPLETION:
		hdd_debug("****eCSR_ROAM_ASSOCIATION_COMPLETION****");
		/*
		 * To Do - address probable memory leak with WEP encryption upon
		 * successful association.
		 */
		if (eCSR_ROAM_RESULT_ASSOCIATED != roam_result) {
			/* Clear saved connection information in HDD */
			hdd_conn_remove_connect_info(
				WLAN_HDD_GET_STATION_CTX_PTR(adapter));
		}
		qdf_ret_status =
			hdd_association_completion_handler(adapter, roam_info,
							   roam_id, roam_status,
							   roam_result);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (roam_info)
			roam_info->roamSynchInProgress = false;
#endif
		break;
	case eCSR_ROAM_CANCELLED:
		hdd_debug("****eCSR_ROAM_CANCELLED****");
		/* fallthrough */
	case eCSR_ROAM_ASSOCIATION_FAILURE:
		qdf_ret_status = hdd_association_completion_handler(adapter,
								    roam_info,
								    roam_id,
								    roam_status,
								    roam_result);
		break;
	case eCSR_ROAM_IBSS_IND:
		hdd_roam_ibss_indication_handler(adapter, roam_info, roam_id,
						 roam_status, roam_result);
		break;

	case eCSR_ROAM_CONNECT_STATUS_UPDATE:
		qdf_ret_status =
			roam_roam_connect_status_update_handler(adapter,
								roam_info,
								roam_id,
								roam_status,
								roam_result);
		break;

	case eCSR_ROAM_MIC_ERROR_IND:
		hdd_roam_mic_error_indication_handler(adapter, roam_info);
		break;

	case eCSR_ROAM_SET_KEY_COMPLETE:
	{
		qdf_ret_status =
			hdd_roam_set_key_complete_handler(adapter, roam_info,
							  roam_id, roam_status,
							  roam_result);
		if (eCSR_ROAM_RESULT_AUTHENTICATED == roam_result) {
			sta_ctx->hdd_reassoc_scenario = false;
			hdd_debug("hdd_reassoc_scenario set to: %d, set key complete, session: %d",
				  sta_ctx->hdd_reassoc_scenario,
				  adapter->vdev_id);
		}
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (roam_info)
			roam_info->roamSynchInProgress = false;
#endif
		break;

	case eCSR_ROAM_FT_RESPONSE:
		hdd_send_ft_event(adapter);
		break;

	case eCSR_ROAM_PMK_NOTIFY:
		if (eCSR_AUTH_TYPE_RSN == sta_ctx->conn_info.auth_type
				|| hdd_is_8021x_sha256_auth_type(sta_ctx)) {
			/* notify the supplicant of a new candidate */
			qdf_ret_status =
				wlan_hdd_cfg80211_pmksa_candidate_notify(
						adapter, roam_info, 1, false);
		}
		break;

#ifdef FEATURE_WLAN_LFR_METRICS
	case eCSR_ROAM_PREAUTH_INIT_NOTIFY:
		/* This event is to notify pre-auth initiation */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth(adapter,
							   roam_info)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_PREAUTH_STATUS_SUCCESS:
		/*
		 * This event will notify pre-auth completion in case of success
		 */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth_status(adapter,
							 roam_info, 1)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_PREAUTH_STATUS_FAILURE:
		/*
		 * This event will notify pre-auth completion incase of failure.
		 */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_preauth_status(adapter,
								roam_info, 0)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_HANDOVER_SUCCESS:
		/* This event is to notify handover success.
		 * It will be only invoked on success
		 */
		if (QDF_STATUS_SUCCESS !=
		    wlan_hdd_cfg80211_roam_metrics_handover(adapter,
							    roam_info)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
#endif
#ifdef WLAN_FEATURE_11W
	case eCSR_ROAM_UNPROT_MGMT_FRAME_IND:
		if (roam_info)
			hdd_indicate_unprot_mgmt_frame(adapter,
					       roam_info->nFrameLength,
					       roam_info->pbFrames,
					       roam_info->frameType);
		break;
#endif
#ifdef FEATURE_WLAN_ESE
	case eCSR_ROAM_TSM_IE_IND:
		if (roam_info)
			hdd_indicate_tsm_ie(adapter, roam_info->tsm_ie.tsid,
				    roam_info->tsm_ie.state,
				    roam_info->tsm_ie.msmt_interval);
		break;

	case eCSR_ROAM_CCKM_PREAUTH_NOTIFY:
	{
		if (eCSR_AUTH_TYPE_CCKM_WPA ==
		    sta_ctx->conn_info.auth_type
		    || eCSR_AUTH_TYPE_CCKM_RSN ==
		    sta_ctx->conn_info.auth_type) {
			hdd_indicate_cckm_pre_auth(adapter, roam_info);
		}
		break;
	}

	case eCSR_ROAM_ESE_ADJ_AP_REPORT_IND:
	{
		hdd_indicate_ese_adj_ap_rep_ind(adapter, roam_info);
		break;
	}

	case eCSR_ROAM_ESE_BCN_REPORT_IND:
	{
		hdd_indicate_ese_bcn_report_ind(adapter, roam_info);
		break;
	}
#endif /* FEATURE_WLAN_ESE */
	case eCSR_ROAM_STA_CHANNEL_SWITCH:
		hdd_roam_channel_switch_handler(adapter, roam_info);
		break;

	case eCSR_ROAM_UPDATE_SCAN_RESULT:
		if ((roam_info) && (roam_info->bss_desc)) {
			bss_status = wlan_hdd_inform_bss_frame(adapter,
							roam_info->bss_desc);
			if (!bss_status)
				hdd_debug("UPDATE_SCAN_RESULT returned NULL");
			else
				cfg80211_put_bss(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)) || defined(WITH_BACKPORTS)
					(WLAN_HDD_GET_CTX(adapter))->wiphy,
#endif
					bss_status);
		}
		break;
	case eCSR_ROAM_NDP_STATUS_UPDATE:
		hdd_ndp_event_handler(adapter, roam_info, roam_id, roam_status,
			roam_result);
		break;
	case eCSR_ROAM_START:
		hdd_debug("Process ROAM_START from firmware");
		wlan_hdd_netif_queue_control(adapter,
				WLAN_STOP_ALL_NETIF_QUEUE,
				WLAN_CONTROL_PATH);
		hdd_set_connection_in_progress(true);
		hdd_set_roaming_in_progress(true);
		policy_mgr_restart_opportunistic_timer(hdd_ctx->psoc, true);
		break;
	case eCSR_ROAM_ABORT:
		hdd_debug("Firmware aborted roaming operation, previous connection is still valid");
		hdd_napi_serialize(0);
		wlan_hdd_netif_queue_control(adapter,
				WLAN_WAKE_ALL_NETIF_QUEUE,
				WLAN_CONTROL_PATH);
		hdd_set_connection_in_progress(false);
		hdd_set_roaming_in_progress(false);
		adapter->roam_ho_fail = false;
		sta_ctx->ft_carrier_on = false;
		complete(&adapter->roaming_comp_var);
		break;

	case eCSR_ROAM_SAE_COMPUTE:
		if (roam_info)
			wlan_hdd_sae_callback(adapter, roam_info);
		break;

	case eCSR_ROAM_ROAMING_START:
		/*
		 * For LFR2, Handle roaming start to remove disassociated
		 * session
		 */
		if (roaming_offload_enabled(hdd_ctx))
			break;
		if (roam_result == eCSR_ROAM_RESULT_NOT_ASSOCIATED) {
			hdd_debug("Decrement session of disassociated AP device_mode %d sessionId %d",
				  adapter->device_mode,
				  adapter->vdev_id);
			policy_mgr_decr_session_set_pcl(hdd_ctx->psoc,
							adapter->device_mode,
							adapter->vdev_id);
		}
		break;

	default:
		break;
	}
	return qdf_ret_status;
}

#ifdef WLAN_FEATURE_FILS_SK
/**
 * hdd_translate_fils_rsn_to_csr_auth() - Translate FILS RSN to CSR auth type
 * @auth_suite: auth suite
 * @auth_type: pointer to enum csr_akm_type
 *
 * Return: None
 */
static void hdd_translate_fils_rsn_to_csr_auth(int8_t auth_suite[4],
					enum csr_akm_type *auth_type)
{
	if (!memcmp(auth_suite, ccp_rsn_oui_0e, 4))
		*auth_type = eCSR_AUTH_TYPE_FILS_SHA256;
	else if (!memcmp(auth_suite, ccp_rsn_oui_0f, 4))
		*auth_type = eCSR_AUTH_TYPE_FILS_SHA384;
	else if (!memcmp(auth_suite, ccp_rsn_oui_10, 4))
		*auth_type = eCSR_AUTH_TYPE_FT_FILS_SHA256;
	else if (!memcmp(auth_suite, ccp_rsn_oui_11, 4))
		*auth_type = eCSR_AUTH_TYPE_FT_FILS_SHA384;
}
#else
static inline void hdd_translate_fils_rsn_to_csr_auth(int8_t auth_suite[4],
					enum csr_akm_type *auth_type)
{
}
#endif

#ifdef WLAN_FEATURE_SAE
/**
 * hdd_translate_sae_rsn_to_csr_auth() - Translate SAE RSN to CSR auth type
 * @auth_suite: auth suite
 * @auth_type: pointer to enum csr_akm_type
 *
 * Return: None
 */
static void hdd_translate_sae_rsn_to_csr_auth(int8_t auth_suite[4],
					enum csr_akm_type *auth_type)
{
	if (qdf_mem_cmp(auth_suite, ccp_rsn_oui_80, 4) == 0)
		*auth_type = eCSR_AUTH_TYPE_SAE;
	else if (qdf_mem_cmp(auth_suite, ccp_rsn_oui_90, 4) == 0)
		*auth_type = eCSR_AUTH_TYPE_FT_SAE;

}
#else
static inline void hdd_translate_sae_rsn_to_csr_auth(int8_t auth_suite[4],
					enum csr_akm_type *auth_type)
{
}
#endif

/**
 * hdd_translate_rsn_to_csr_auth_type() - Translate RSN to CSR auth type
 * @auth_suite: auth suite
 *
 * Return: enum csr_akm_type enumeration
 */
enum csr_akm_type hdd_translate_rsn_to_csr_auth_type(uint8_t auth_suite[4])
{
	enum csr_akm_type auth_type = eCSR_AUTH_TYPE_UNKNOWN;
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
	} else if (memcmp(auth_suite, ccp_rsn_oui_18, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_OWE;
	} else
#endif
	if (memcmp(auth_suite, ccp_rsn_oui_12, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_DPP_RSN;
	} else if (memcmp(auth_suite, ccp_rsn_oui_0b, 4) == 0) {
		/* Check for Suite B EAP 256 */
		auth_type = eCSR_AUTH_TYPE_SUITEB_EAP_SHA256;
	} else if (memcmp(auth_suite, ccp_rsn_oui_0c, 4) == 0) {
		/* Check for Suite B EAP 384 */
		auth_type = eCSR_AUTH_TYPE_SUITEB_EAP_SHA384;
	} else if (memcmp(auth_suite, ccp_rsn_oui_0d, 4) == 0) {
		/* Check for FT Suite B EAP 384 */
		auth_type = eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384;
	} else if (memcmp(auth_suite, ccp_rsn_oui_13, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_OSEN;
	} else {
		hdd_translate_fils_rsn_to_csr_auth(auth_suite, &auth_type);
		hdd_translate_sae_rsn_to_csr_auth(auth_suite, &auth_type);
	}
	hdd_debug("auth_type: %d", auth_type);
	return auth_type;
}

/**
 * hdd_translate_wpa_to_csr_auth_type() - Translate WPA to CSR auth type
 * @auth_suite: auth suite
 *
 * Return: enum csr_akm_type enumeration
 */
enum csr_akm_type hdd_translate_wpa_to_csr_auth_type(uint8_t auth_suite[4])
{
	enum csr_akm_type auth_type = eCSR_AUTH_TYPE_UNKNOWN;
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
		hdd_translate_fils_rsn_to_csr_auth(auth_suite, &auth_type);
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
	else if (memcmp(cipher_suite, ccp_rsn_oui09, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_AES_GCMP;
	else if (memcmp(cipher_suite, ccp_rsn_oui0a, 4) == 0)
		cipher_type = eCSR_ENCRYPT_TYPE_AES_GCMP_256;
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

#ifdef WLAN_FEATURE_FILS_SK
bool hdd_is_fils_connection(struct hdd_adapter *adapter)
{
	struct csr_roam_profile *roam_profile;

	roam_profile = hdd_roam_profile(adapter);
	if (roam_profile->fils_con_info)
		return roam_profile->fils_con_info->is_fils_connection;

	return false;
}
#else
bool hdd_is_fils_connection(struct hdd_adapter *adapter)
{
	return false;
}
#endif

/**
 * hdd_process_genie() - process gen ie
 * @adapter: pointer to adapter
 * @bssid: pointer to mac address
 * @encrypt_type: pointer to encryption type
 * @mc_encrypt_type: pointer to multicast encryption type
 * @auth_type: pointer to auth type
 *
 * Return: 0 on success, error number otherwise
 */
static int32_t hdd_process_genie(struct hdd_adapter *adapter,
				 u8 *bssid,
				 eCsrEncryptionType *encrypt_type,
				 eCsrEncryptionType *mc_encrypt_type,
				 enum csr_akm_type *auth_type,
#ifdef WLAN_FEATURE_11W
				 uint8_t *mfp_required, uint8_t *mfp_capable,
#endif
				 uint16_t gen_ie_len, uint8_t *gen_ie)
{
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);
	tDot11fIERSN dot11_rsn_ie = {0};
	tDot11fIEWPA dot11_wpa_ie = {0};
	uint8_t *rsn_ie;
	uint16_t rsn_ie_len;
	uint32_t parse_status;
#ifdef WLAN_CONV_CRYPTO_SUPPORTED
	uint16_t rsn_cap = 0;
#endif

	/*
	 * Clear struct of tDot11fIERSN and tDot11fIEWPA specifically
	 * setting present flag to 0.
	 */
	memset(&dot11_wpa_ie, 0, sizeof(tDot11fIEWPA));
	memset(&dot11_rsn_ie, 0, sizeof(tDot11fIERSN));

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
		rsn_ie = gen_ie + 2;
		rsn_ie_len = gen_ie_len - 2;
		/* Unpack the RSN IE */
		parse_status = sme_unpack_rsn_ie(mac_handle, rsn_ie, rsn_ie_len,
						 &dot11_rsn_ie, false);
		if (!DOT11F_SUCCEEDED(parse_status)) {
			hdd_err("Invalid RSN IE: parse status %d",
				parse_status);
			return -EINVAL;
		}
		hdd_debug("gp_cipher_suite_present: %d",
			 dot11_rsn_ie.gp_cipher_suite_present);
		/* Copy out the encryption and authentication types */
		hdd_debug("pairwise cipher suite count: %d",
			 dot11_rsn_ie.pwise_cipher_suite_count);
		hdd_debug("authentication suite count: %d",
			 dot11_rsn_ie.akm_suite_cnt);
		/* dot11_rsn_ie.akm_suite_cnt */
		/* Just translate the FIRST one */
		*auth_type =
			hdd_translate_rsn_to_csr_auth_type(
					dot11_rsn_ie.akm_suite[0]);
		/* dot11_rsn_ie.pwise_cipher_suite_count */
		*encrypt_type =
			hdd_translate_rsn_to_csr_encryption_type(
					dot11_rsn_ie.pwise_cipher_suites[0]);
		/* dot11_rsn_ie.gp_cipher_suite_count */
		*mc_encrypt_type =
			hdd_translate_rsn_to_csr_encryption_type(
					dot11_rsn_ie.gp_cipher_suite);
#ifdef WLAN_FEATURE_11W
		*mfp_required = (dot11_rsn_ie.RSN_Cap[0] >> 6) & 0x1;
		*mfp_capable = csr_is_mfpc_capable(&dot11_rsn_ie);
#endif
#ifdef WLAN_CONV_CRYPTO_SUPPORTED
		qdf_mem_copy(&rsn_cap, dot11_rsn_ie.RSN_Cap, sizeof(rsn_cap));
		wlan_crypto_set_vdev_param(adapter->vdev,
					   WLAN_CRYPTO_PARAM_RSN_CAP, rsn_cap);
#endif
	} else if (gen_ie[0] == DOT11F_EID_WPA) {
		/* Validity checks */
		if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN) ||
		    (gen_ie_len > DOT11F_IE_WPA_MAX_LEN)) {
			hdd_err("Invalid DOT11F WPA IE length: %d",
				gen_ie_len);
			return -EINVAL;
		}
		/* Skip past the EID and length byte - and four byte WiFi OUI */
		rsn_ie = gen_ie + 2 + 4;
		rsn_ie_len = gen_ie_len - (2 + 4);
		/* Unpack the WPA IE */
		parse_status = dot11f_unpack_ie_wpa(MAC_CONTEXT(mac_handle),
						    rsn_ie, rsn_ie_len,
						    &dot11_wpa_ie, false);
		if (!DOT11F_SUCCEEDED(parse_status)) {
			hdd_err("Invalid WPA IE: parse status %d",
				parse_status);
			return -EINVAL;
		}
		/* Copy out the encryption and authentication types */
		hdd_debug("WPA unicast cipher suite count: %d",
			 dot11_wpa_ie.unicast_cipher_count);
		hdd_debug("WPA authentication suite count: %d",
			 dot11_wpa_ie.auth_suite_count);
		/* dot11_wpa_ie.auth_suite_count */
		/* Just translate the FIRST one */
		*auth_type =
			hdd_translate_wpa_to_csr_auth_type(
					dot11_wpa_ie.auth_suites[0]);
		/* dot11_wpa_ie.unicast_cipher_count */
		*encrypt_type =
			hdd_translate_wpa_to_csr_encryption_type(
					dot11_wpa_ie.unicast_ciphers[0]);
		/* dot11_wpa_ie.unicast_cipher_count */
		*mc_encrypt_type =
			hdd_translate_wpa_to_csr_encryption_type(
					dot11_wpa_ie.multicast_cipher);
	} else {
		hdd_warn("gen_ie[0]: %d", gen_ie[0]);
		return -EINVAL;
	}
	return 0;
}

#ifdef WLAN_FEATURE_11W
/**
 * hdd_set_def_mfp_cap: Set default value for MFPCapable
 * @profile: Source profile
 *
 * Return: None
 */
static void hdd_set_def_mfp_cap(
	struct csr_roam_profile *roam_profile)
{
	roam_profile->MFPCapable = roam_profile->MFPEnabled;
}
#else
static void hdd_set_def_mfp_cap(
	struct csr_roam_profile *roam_profile)
{
}
#endif

/**
 * hdd_set_def_rsne_override() - set default encryption type and auth type
 * in profile.
 * @roam_profile: pointer to adapter
 * @auth_type: pointer to auth type
 *
 * Set default value of encryption type and auth type in profile to
 * search the AP using filter, as in force_rsne_override the RSNIE can be
 * currupt and we might not get the proper encryption type and auth type
 * while parsing the RSNIE.
 *
 * Return: void
 */
static void hdd_set_def_rsne_override(
	struct csr_roam_profile *roam_profile, enum csr_akm_type *auth_type)
{
	hdd_debug("Set def values in roam profile");
	hdd_set_def_mfp_cap(roam_profile);
	roam_profile->EncryptionType.numEntries = 2;
	roam_profile->mcEncryptionType.numEntries = 2;
		/* Use the cipher type in the RSN IE */
	roam_profile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_AES;
	roam_profile->EncryptionType.encryptionType[1] = eCSR_ENCRYPT_TYPE_TKIP;
	roam_profile->mcEncryptionType.encryptionType[0] =
		eCSR_ENCRYPT_TYPE_AES;
	roam_profile->mcEncryptionType.encryptionType[1] =
		eCSR_ENCRYPT_TYPE_TKIP;
	*auth_type = eCSR_AUTH_TYPE_RSN_PSK;
}

#ifdef WLAN_FEATURE_11W
/**
 * hdd_update_values_mfp_cap: Update values for MFPCapable,MFPEnabled
 * @profile: Source profile
 *
 * Return: None
 */
static void hdd_update_values_mfp_cap(
	struct csr_roam_profile *roam_profile,
	uint8_t mfp_required, uint8_t mfp_capable )
{
	hdd_debug("mfp_required = %d, mfp_capable = %d",
		 mfp_required, mfp_capable);
	roam_profile->MFPRequired = mfp_required;
	roam_profile->MFPCapable = mfp_capable;
}
#else
static void hdd_update_values_mfp_cap(
	struct csr_roam_profile *roam_profile,
	uint8_t mfp_required, uint8_t mfp_capable )
{
}
#endif

#ifdef WLAN_FEATURE_11W
/**
 * hdd_set_mfp_enable: Set value for MFPEnabled
 * @profile: Source profile
 *
 * Return: None
 */
static void hdd_set_mfp_enable(struct csr_roam_profile *roam_profile)
{
	hdd_debug("MFPEnabled %d", roam_profile->MFPEnabled);
	/*
	 * Reset MFPEnabled if testmode RSNE passed doesn't have MFPR
	 * or MFPC bit set
	 */
	if (roam_profile->MFPEnabled &&
		!(roam_profile->MFPRequired ||
		  roam_profile->MFPCapable)) {
		hdd_debug("Reset MFPEnabled");
		roam_profile->MFPEnabled = 0;
	}
}
#else
static void hdd_set_mfp_enable(struct csr_roam_profile *roam_profile)
{
}
#endif

/**
 * hdd_set_genie_to_csr() - set genie to csr
 * @adapter: pointer to adapter
 * @rsn_auth_type: pointer to auth type
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_set_genie_to_csr(struct hdd_adapter *adapter,
			 enum csr_akm_type *rsn_auth_type)
{
	struct csr_roam_profile *roam_profile;
	uint8_t *security_ie;
	uint32_t status = 0;
	eCsrEncryptionType rsn_encrypt_type;
	eCsrEncryptionType mc_rsn_encrypt_type;
	struct hdd_context *hdd_ctx;
	uint8_t mfp_required = 0;
	uint8_t mfp_capable = 0;
	u8 bssid[ETH_ALEN];        /* MAC address of assoc peer */

	roam_profile = hdd_roam_profile(adapter);
	security_ie = hdd_security_ie(adapter);

	/* MAC address of assoc peer */
	/* But, this routine is only called when we are NOT associated. */
	qdf_mem_copy(bssid, roam_profile->BSSIDs.bssid, sizeof(bssid));
	if (security_ie[0] == DOT11F_EID_RSN ||
	    security_ie[0] == DOT11F_EID_WPA) {
		/* continue */
	} else {
		return 0;
	}

	/* The actual processing may eventually be more extensive than this. */
	/* Right now, just consume any PMKIDs that are  sent in by the app. */
	status = hdd_process_genie(adapter, bssid,
				   &rsn_encrypt_type,
				   &mc_rsn_encrypt_type, rsn_auth_type,
				   &mfp_required, &mfp_capable,
				   security_ie[1] + 2,
				   security_ie);

	if (status == 0) {
		/*
		 * Now copy over all the security attributes
		 * you have parsed out.
		 */
		roam_profile->EncryptionType.numEntries = 1;
		roam_profile->mcEncryptionType.numEntries = 1;

		/* Use the cipher type in the RSN IE */
		roam_profile->EncryptionType.encryptionType[0] =
			rsn_encrypt_type;
		roam_profile->mcEncryptionType.encryptionType[0] =
			mc_rsn_encrypt_type;

		if ((QDF_IBSS_MODE == adapter->device_mode) &&
		    ((eCSR_ENCRYPT_TYPE_AES == mc_rsn_encrypt_type) ||
		     (eCSR_ENCRYPT_TYPE_AES_GCMP == mc_rsn_encrypt_type) ||
		     (eCSR_ENCRYPT_TYPE_AES_GCMP_256 == mc_rsn_encrypt_type) ||
		     (eCSR_ENCRYPT_TYPE_TKIP == mc_rsn_encrypt_type))) {
			/*
			 * For wpa none supplicant sends the WPA IE with unicast
			 * cipher as eCSR_ENCRYPT_TYPE_NONE ,where as the
			 * multicast cipher as either AES/TKIP based on group
			 * cipher configuration mentioned in the
			 * wpa_supplicant.conf.
			 */

			/* Set the unicast cipher same as multicast cipher */
			roam_profile->EncryptionType.encryptionType[0]
				= mc_rsn_encrypt_type;
		}

		hdd_update_values_mfp_cap(roam_profile, mfp_required,
					  mfp_capable);

		hdd_debug("CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d",
			 *rsn_auth_type, rsn_encrypt_type, mc_rsn_encrypt_type);
	}
#ifdef WLAN_CONV_CRYPTO_SUPPORTED
	if (QDF_STATUS_SUCCESS != wlan_set_vdev_crypto_prarams_from_ie(
				  adapter->vdev, security_ie,
				  (security_ie[1] + 2)))
		hdd_err("Failed to set the crypto params from IE");
#endif

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx->force_rsne_override &&
	    (security_ie[0] == DOT11F_EID_RSN)) {
		hdd_warn("Test mode enabled set def Auth and enc type. RSN IE passed in connect req: ");
		qdf_trace_hex_dump(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
				   roam_profile->pRSNReqIE,
				   roam_profile->nRSNReqIELength);

		roam_profile->force_rsne_override = true;

		hdd_set_mfp_enable(roam_profile);

		/* If parsing failed set the def value for the roam profile */
		if (status)
			hdd_set_def_rsne_override(roam_profile, rsn_auth_type);
		return 0;
	}
	return status;
}

#ifdef WLAN_FEATURE_FILS_SK
/**
 * hdd_check_fils_rsn_n_set_auth_type() - This API checks whether a give
 * auth type is fils if yes, sets it in profile.
 * @rsn_auth_type: auth type
 *
 * Return: true if FILS auth else false
 */
static
bool hdd_check_fils_rsn_n_set_auth_type(struct csr_roam_profile *roam_profile,
					enum csr_akm_type rsn_auth_type)
{
	bool is_fils_rsn = false;

	if (!roam_profile->fils_con_info)
		return false;

	if ((rsn_auth_type == eCSR_AUTH_TYPE_FILS_SHA256) ||
	   (rsn_auth_type == eCSR_AUTH_TYPE_FILS_SHA384) ||
	   (rsn_auth_type == eCSR_AUTH_TYPE_FT_FILS_SHA256) ||
	   (rsn_auth_type == eCSR_AUTH_TYPE_FT_FILS_SHA384))
		is_fils_rsn = true;
	if (is_fils_rsn)
		roam_profile->fils_con_info->akm_type = rsn_auth_type;

	return is_fils_rsn;
}
#else
static
bool hdd_check_fils_rsn_n_set_auth_type(struct csr_roam_profile *roam_profile,
					enum csr_akm_type rsn_auth_type)
{
	return false;
}
#endif

/**
 * hdd_set_csr_auth_type() - set csr auth type
 * @adapter: pointer to adapter
 * @rsn_auth_type: auth type
 *
 * Return: 0 on success, error number otherwise
 */
int hdd_set_csr_auth_type(struct hdd_adapter *adapter,
			  enum csr_akm_type rsn_auth_type)
{
	struct csr_roam_profile *roam_profile;
	struct hdd_station_ctx *sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	enum hdd_auth_key_mgmt key_mgmt = sta_ctx->auth_key_mgmt;

	roam_profile = hdd_roam_profile(adapter);
	roam_profile->AuthType.numEntries = 1;
	hdd_debug("auth_type = %d rsn_auth_type %d wpa_versions %d key_mgmt : 0x%x",
		  sta_ctx->conn_info.auth_type, rsn_auth_type,
		  sta_ctx->wpa_versions, key_mgmt);

	switch (sta_ctx->conn_info.auth_type) {
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
	case eCSR_AUTH_TYPE_AUTOSWITCH:
#ifdef FEATURE_WLAN_ESE
	case eCSR_AUTH_TYPE_CCKM_WPA:
	case eCSR_AUTH_TYPE_CCKM_RSN:
#endif
		if (!sta_ctx->wpa_versions) {

			roam_profile->AuthType.authType[0] =
				eCSR_AUTH_TYPE_OPEN_SYSTEM;
		} else if (sta_ctx->wpa_versions & NL80211_WPA_VERSION_1) {

#ifdef FEATURE_WLAN_ESE
			if ((rsn_auth_type == eCSR_AUTH_TYPE_CCKM_WPA) &&
			    ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
			     == HDD_AUTH_KEY_MGMT_802_1X)) {
				hdd_debug("set authType to CCKM WPA. AKM also 802.1X.");
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_WPA;
			} else if (rsn_auth_type == eCSR_AUTH_TYPE_CCKM_WPA) {
				hdd_debug("Last chance to set authType to CCKM WPA.");
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_WPA;
			} else
#endif
			if ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
			    == HDD_AUTH_KEY_MGMT_802_1X) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WPA;
			} else
			if ((key_mgmt & HDD_AUTH_KEY_MGMT_PSK)
			    == HDD_AUTH_KEY_MGMT_PSK) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WPA_PSK;
			} else {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_WPA_NONE;
			}
		}
		if (sta_ctx->wpa_versions & NL80211_WPA_VERSION_2) {
#ifdef FEATURE_WLAN_ESE
			if ((rsn_auth_type == eCSR_AUTH_TYPE_CCKM_RSN) &&
			    ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
			     == HDD_AUTH_KEY_MGMT_802_1X)) {
				hdd_debug("set authType to CCKM RSN. AKM also 802.1X.");
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_RSN;
			} else if (rsn_auth_type == eCSR_AUTH_TYPE_CCKM_RSN) {
				hdd_debug("Last chance to set authType to CCKM RSN.");
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_CCKM_RSN;
			} else
#endif
			if (rsn_auth_type == eCSR_AUTH_TYPE_DPP_RSN) {
				roam_profile->AuthType.authType[0] =
							eCSR_AUTH_TYPE_DPP_RSN;
			} else if (rsn_auth_type == eCSR_AUTH_TYPE_OSEN) {
				roam_profile->AuthType.authType[0] =
							eCSR_AUTH_TYPE_OSEN;
			} else if ((rsn_auth_type == eCSR_AUTH_TYPE_FT_RSN) &&
			    ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
			     == HDD_AUTH_KEY_MGMT_802_1X)) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_FT_RSN;
			} else if ((rsn_auth_type == eCSR_AUTH_TYPE_FT_RSN_PSK)
				   &&
				   ((key_mgmt & HDD_AUTH_KEY_MGMT_PSK)
				    == HDD_AUTH_KEY_MGMT_PSK)) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_FT_RSN_PSK;
			} else

#ifdef WLAN_FEATURE_11W
			if (rsn_auth_type == eCSR_AUTH_TYPE_RSN_PSK_SHA256) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN_PSK_SHA256;
			} else if (rsn_auth_type ==
				   eCSR_AUTH_TYPE_RSN_8021X_SHA256) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN_8021X_SHA256;
			} else
#endif
			if (hdd_check_fils_rsn_n_set_auth_type(roam_profile,
				rsn_auth_type)) {
				roam_profile->AuthType.authType[0] =
					rsn_auth_type;
				hdd_debug("updated profile authtype as %d",
					rsn_auth_type);

			} else if ((rsn_auth_type == eCSR_AUTH_TYPE_OWE) &&
				  ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
				  == HDD_AUTH_KEY_MGMT_802_1X)) {
				/* OWE case */
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_OWE;
			} else if (rsn_auth_type == eCSR_AUTH_TYPE_SAE) {
				/* SAE with open authentication case */
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_SAE;
			} else if ((rsn_auth_type ==
				  eCSR_AUTH_TYPE_SUITEB_EAP_SHA256) &&
				  ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
				  == HDD_AUTH_KEY_MGMT_802_1X)) {
				/* Suite B EAP SHA 256 */
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_SUITEB_EAP_SHA256;
			} else if ((rsn_auth_type ==
				  eCSR_AUTH_TYPE_SUITEB_EAP_SHA384) &&
				  ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
				  == HDD_AUTH_KEY_MGMT_802_1X)) {
				/* Suite B EAP SHA 384 */
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_SUITEB_EAP_SHA384;
			} else if ((rsn_auth_type == eCSR_AUTH_TYPE_FT_SAE) &&
				   ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X) ==
				    HDD_AUTH_KEY_MGMT_802_1X)) {
				roam_profile->AuthType.authType[0] =
						eCSR_AUTH_TYPE_FT_SAE;
			} else if ((rsn_auth_type ==
				  eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384) &&
				  ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
				  == HDD_AUTH_KEY_MGMT_802_1X)) {
				/* FT Suite-B EAP SHA 384 */
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384;

			} else if ((key_mgmt & HDD_AUTH_KEY_MGMT_802_1X)
				    == HDD_AUTH_KEY_MGMT_802_1X) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN;
			} else
			if ((key_mgmt & HDD_AUTH_KEY_MGMT_PSK)
			    == HDD_AUTH_KEY_MGMT_PSK) {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_RSN_PSK;
			} else {
				roam_profile->AuthType.authType[0] =
					eCSR_AUTH_TYPE_UNKNOWN;
			}
		}
		break;

	case eCSR_AUTH_TYPE_SHARED_KEY:

		roam_profile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;
		break;

	case eCSR_AUTH_TYPE_SAE:

		if (rsn_auth_type == eCSR_AUTH_TYPE_FT_SAE)
			roam_profile->AuthType.authType[0] =
						eCSR_AUTH_TYPE_FT_SAE;
		else
			roam_profile->AuthType.authType[0] = eCSR_AUTH_TYPE_SAE;
		break;

	default:

#ifdef FEATURE_WLAN_ESE
		hdd_debug("In default, unknown auth type.");
#endif /* FEATURE_WLAN_ESE */
		roam_profile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
		break;
	}

	hdd_debug("Set roam Authtype to %d",
		 roam_profile->AuthType.authType[0]);

	return 0;
}

#ifdef WLAN_FEATURE_FILS_SK
static void hdd_initialize_fils_info(struct hdd_adapter *adapter)
{
	struct csr_roam_profile *roam_profile;

	roam_profile = hdd_roam_profile(adapter);
	roam_profile->fils_con_info = NULL;
	roam_profile->hlp_ie = NULL;
	roam_profile->hlp_ie_len = 0;
}
#else
static void hdd_initialize_fils_info(struct hdd_adapter *adapter)
{ }
#endif

void hdd_roam_profile_init(struct hdd_adapter *adapter)
{
	struct csr_roam_profile *roam_profile;
	uint8_t *security_ie;
	tSirAddie *assoc_additional_ie;
	struct hdd_station_ctx *sta_ctx;

	hdd_enter();

	roam_profile = hdd_roam_profile(adapter);
	qdf_mem_zero(roam_profile, sizeof(*roam_profile));

	security_ie = hdd_security_ie(adapter);
	qdf_mem_zero(security_ie, WLAN_MAX_IE_LEN);

	assoc_additional_ie = hdd_assoc_additional_ie(adapter);
	qdf_mem_zero(assoc_additional_ie, sizeof(*assoc_additional_ie));

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* Configure the roaming profile links to SSID and bssid. */
	roam_profile->SSIDs.numOfSSIDs = 0;
	roam_profile->SSIDs.SSIDList = &sta_ctx->conn_info.ssid;

	roam_profile->BSSIDs.numOfBSSIDs = 0;
	roam_profile->BSSIDs.bssid = &sta_ctx->conn_info.bssid;

	/* Set the numOfChannels to zero to scan all the channels */
	roam_profile->ChannelInfo.numOfChannels = 0;
	roam_profile->ChannelInfo.freq_list = NULL;

	roam_profile->BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;

	roam_profile->phyMode = eCSR_DOT11_MODE_AUTO;
	sta_ctx->wpa_versions = 0;

	/* Set the default scan mode */
	adapter->scan_info.scan_mode = eSIR_ACTIVE_SCAN;

	hdd_clear_roam_profile_ie(adapter);

	hdd_initialize_fils_info(adapter);

	hdd_exit();
}
