/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
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
#include "ol_txrx.h"
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
#include "wlan_mlme_twt_ucfg_api.h"

#include "wlan_hdd_nud_tracking.h"
#include <wlan_cfg80211_crypto.h>
#include <wlan_crypto_global_api.h>
#include "wlan_blm_ucfg_api.h"
#include "wlan_hdd_sta_info.h"
#include "wlan_hdd_ftm_time_sync.h"
#include "wlan_hdd_periodic_sta_stats.h"
#include "wlan_cm_roam_api.h"

#include <ol_defines.h>
#include "wlan_pkt_capture_ucfg_api.h"
#include "wlan_if_mgr_ucfg_api.h"
#include "wlan_if_mgr_public_struct.h"
#include "wlan_cm_public_struct.h"
#include "osif_cm_util.h"
#include "wlan_hdd_cm_api.h"
#include "cm_utf.h"

#include "wlan_hdd_bootup_marker.h"
#include "wlan_roam_debug.h"

#include "wlan_hdd_twt.h"
#include "wlan_cm_roam_ucfg_api.h"

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

/* RSN-PSK-SHA256 */
uint8_t ccp_rsn_oui07[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x06 };

/* RSN-8021X-SHA256 */
uint8_t ccp_rsn_oui08[HDD_RSN_OUI_SIZE] = { 0x00, 0x0F, 0xAC, 0x05 };

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
#define ASSOC_RSP_IES_OFFSET 6  /* Capability(2) + AID(2) + Status Code(2) */
#define ASSOC_REQ_IES_OFFSET 4  /* Capability(2) + LI(2) */

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

/* HE operation BIT positins */
#if defined(WLAN_FEATURE_11AX)
#define HE_OPERATION_DFLT_PE_DURATION_POS 0
#define HE_OPERATION_TWT_REQUIRED_POS 3
#define HE_OPERATION_RTS_THRESHOLD_POS 4
#define HE_OPERATION_VHT_OPER_POS 14
#define HE_OPERATION_CO_LOCATED_BSS_POS 15
#define HE_OPERATION_ER_SU_DISABLE_POS 16
#define HE_OPERATION_OPER_INFO_6G_POS 17
#define HE_OPERATION_RESERVED_POS 18
#define HE_OPERATION_BSS_COLOR_POS 24
#define HE_OPERATION_PARTIAL_BSS_COLOR_POS 30
#define HE_OPERATION_BSS_COL_DISABLED_POS 31
#endif

#if defined(WLAN_FEATURE_SAE) && \
		(defined(CFG80211_EXTERNAL_AUTH_SUPPORT) || \
		LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0))
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
 * hdd_start_powersave_timer_on_associated() - Start auto powersave timer
 *  after associated
 * @adapter: pointer to the adapter
 *
 * This function will start auto powersave timer for STA/P2P Client.
 *
 * Return: none
 */
static void hdd_start_powersave_timer_on_associated(struct hdd_adapter *adapter)
{
	uint32_t timeout;
	uint32_t auto_bmps_timer_val;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE)
		return;
	ucfg_mlme_get_auto_bmps_timer_value(hdd_ctx->psoc,
					    &auto_bmps_timer_val);
	timeout = hdd_cm_is_vdev_roaming(adapter) ?
		AUTO_PS_ENTRY_TIMER_DEFAULT_VALUE :
		(auto_bmps_timer_val * 1000);
	sme_ps_enable_auto_ps_timer(hdd_ctx->mac_handle,
				    adapter->vdev_id,
				    timeout);
}

void hdd_conn_set_authenticated(struct hdd_adapter *adapter, uint8_t auth_state)
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
	if (auth_state &&
	    (sta_ctx->conn_info.ptk_installed ||
	     sta_ctx->conn_info.uc_encrypt_type == eCSR_ENCRYPT_TYPE_NONE))
		hdd_start_powersave_timer_on_associated(adapter);
}

void hdd_conn_set_connection_state(struct hdd_adapter *adapter,
				   eConnectionState conn_state)
{
	struct hdd_station_ctx *hdd_sta_ctx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* save the new connection state */
	if (conn_state == hdd_sta_ctx->conn_info.conn_state)
		return;

	hdd_nofl_debug("connection state changed %d --> %d for dev %s (vdev %d)",
		       hdd_sta_ctx->conn_info.conn_state, conn_state,
		       adapter->dev->name, adapter->vdev_id);

	hdd_sta_ctx->conn_info.conn_state = conn_state;
}

enum band_info hdd_conn_get_connected_band(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	uint32_t sta_freq = 0;

	if (hdd_cm_is_vdev_associated(adapter))
		sta_freq = sta_ctx->conn_info.chan_freq;

	if (wlan_reg_is_24ghz_ch_freq(sta_freq))
		return BAND_2G;
	else if (wlan_reg_is_5ghz_ch_freq(sta_freq) ||
		 wlan_reg_is_6ghz_chan_freq(sta_freq))
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
hdd_conn_get_connected_cipher_algo(struct hdd_adapter *adapter,
				   struct hdd_station_ctx *sta_ctx,
				   eCsrEncryptionType *pConnectedCipherAlgo)
{
	bool connected = false;

	connected = hdd_cm_is_vdev_associated(adapter);

	if (pConnectedCipherAlgo)
		*pConnectedCipherAlgo = sta_ctx->conn_info.uc_encrypt_type;

	return connected;
}

struct hdd_adapter *hdd_get_sta_connection_in_progress(
			struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter = NULL, *next_adapter = NULL;
	struct hdd_station_ctx *hdd_sta_ctx;
	wlan_net_dev_ref_dbgid dbgid =
				NET_DEV_HOLD_GET_STA_CONNECTION_IN_PROGRESS;

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return NULL;
	}

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if ((QDF_STA_MODE == adapter->device_mode) ||
		    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
		    (QDF_P2P_DEVICE_MODE == adapter->device_mode)) {
			if (hdd_cm_is_connecting(adapter)) {
				hdd_debug("vdev_id %d: Connection is in progress",
					  adapter->vdev_id);
				hdd_adapter_dev_put_debug(adapter, dbgid);
				if (next_adapter)
					hdd_adapter_dev_put_debug(next_adapter,
								  dbgid);
				return adapter;
			} else if (hdd_cm_is_vdev_associated(adapter) &&
				   sme_is_sta_key_exchange_in_progress(
							hdd_ctx->mac_handle,
							adapter->vdev_id)) {
				hdd_debug("vdev_id %d: Key exchange is in progress",
					  adapter->vdev_id);
				hdd_adapter_dev_put_debug(adapter, dbgid);
				if (next_adapter)
					hdd_adapter_dev_put_debug(next_adapter,
								  dbgid);
				return adapter;
			}
		}
		hdd_adapter_dev_put_debug(adapter, dbgid);
	}
	return NULL;
}

void hdd_abort_ongoing_sta_connection(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *sta_adapter;

	sta_adapter = hdd_get_sta_connection_in_progress(hdd_ctx);
	if (sta_adapter)
		wlan_hdd_cm_issue_disconnect(sta_adapter,
					     REASON_UNSPEC_FAILURE, false);
}

bool hdd_is_any_sta_connected(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter = NULL, *next_adapter = NULL;
	wlan_net_dev_ref_dbgid dbgid =
				NET_DEV_HOLD_IS_ANY_STA_CONNECTED;

	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
		return false;
	}

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
		if (QDF_STA_MODE == adapter->device_mode ||
		    QDF_P2P_CLIENT_MODE == adapter->device_mode) {
			if (hdd_cm_is_vdev_connected(adapter)) {
				hdd_adapter_dev_put_debug(adapter, dbgid);
				if (next_adapter)
					hdd_adapter_dev_put_debug(next_adapter,
								  dbgid);
				return true;
			}
		}
		hdd_adapter_dev_put_debug(adapter, dbgid);
	}
	return false;
}

/**
 * hdd_remove_beacon_filter() - remove beacon filter
 * @adapter: Pointer to the hdd adapter
 *
 * Return: 0 on success and errno on failure
 */
int hdd_remove_beacon_filter(struct hdd_adapter *adapter)
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

void hdd_copy_ht_operation(struct hdd_station_ctx *hdd_sta_ctx,
			   tDot11fIEHTInfo *ht_ops)
{
	struct ieee80211_ht_operation *hdd_ht_ops =
		&hdd_sta_ctx->conn_info.ht_operation;
	uint32_t i, temp_ht_ops;

	qdf_mem_zero(hdd_ht_ops, sizeof(struct ieee80211_ht_operation));

	hdd_ht_ops->primary_chan = ht_ops->primaryChannel;

	/* HT_PARAMS */
	temp_ht_ops = ht_ops->secondaryChannelOffset &
		IEEE80211_HT_PARAM_CHA_SEC_OFFSET;
	if (temp_ht_ops)
		hdd_ht_ops->ht_param |= temp_ht_ops;
	else
		hdd_ht_ops->ht_param = IEEE80211_HT_PARAM_CHA_SEC_NONE;
	if (ht_ops->recommendedTxWidthSet)
		hdd_ht_ops->ht_param |= IEEE80211_HT_PARAM_CHAN_WIDTH_ANY;
	if (ht_ops->rifsMode)
		hdd_ht_ops->ht_param |= IEEE80211_HT_PARAM_RIFS_MODE;
	if (ht_ops->controlledAccessOnly)
		hdd_ht_ops->ht_param |= HT_PARAM_CONTROLLED_ACCESS_ONLY;
	temp_ht_ops = ht_ops->serviceIntervalGranularity &
		(HT_PARAM_SERVICE_INT_GRAN >> HT_PARAM_SERVICE_INT_GRAN_SHIFT);
	if (temp_ht_ops)
		hdd_ht_ops->ht_param |= temp_ht_ops <<
			HT_PARAM_SERVICE_INT_GRAN_SHIFT;

	/* operation mode */
	temp_ht_ops = ht_ops->opMode &
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
	if (ht_ops->nonGFDevicesPresent)
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_NON_GF_STA_PRSNT;
	if (ht_ops->transmitBurstLimit)
		hdd_ht_ops->operation_mode |=
			HT_OP_MODE_TX_BURST_LIMIT;
	if (ht_ops->obssNonHTStaPresent)
		hdd_ht_ops->operation_mode |=
			IEEE80211_HT_OP_MODE_NON_HT_STA_PRSNT;

	/* stbc_param */
	temp_ht_ops = ht_ops->basicSTBCMCS &
			HT_STBC_PARAM_MCS;
	if (temp_ht_ops)
		hdd_ht_ops->stbc_param |= temp_ht_ops;
	if (ht_ops->dualCTSProtection)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_DUAL_CTS_PROT;
	if (ht_ops->secondaryBeacon)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_STBC_BEACON;
	if (ht_ops->lsigTXOPProtectionFullSupport)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_LSIG_TXOP_FULLPROT;
	if (ht_ops->pcoActive)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_PCO_ACTIVE;
	if (ht_ops->pcoPhase)
		hdd_ht_ops->stbc_param |=
			IEEE80211_HT_STBC_PARAM_PCO_PHASE;

	/* basic MCs set */
	for (i = 0; i < 16; ++i)
		hdd_ht_ops->basic_set[i] =
			ht_ops->basicMCSSet[i];
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

void hdd_copy_vht_operation(struct hdd_station_ctx *hdd_sta_ctx,
			    tDot11fIEVHTOperation *vht_ops)
{
	struct ieee80211_vht_operation *hdd_vht_ops =
		&hdd_sta_ctx->conn_info.vht_operation;

	qdf_mem_zero(hdd_vht_ops, sizeof(struct ieee80211_vht_operation));

	hdd_vht_ops->chan_width = vht_ops->chanWidth;
	hdd_copy_vht_center_freq(hdd_vht_ops, vht_ops);
	hdd_vht_ops->basic_mcs_set = vht_ops->basicMCSSet;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && \
     defined(WLAN_FEATURE_11AX)
void hdd_copy_he_operation(struct hdd_station_ctx *hdd_sta_ctx,
			   tDot11fIEhe_op *he_operation)
{
	struct ieee80211_he_operation *hdd_he_operation;
	uint32_t he_oper_params = 0;
	uint32_t len = 0, filled = 0;
	uint8_t he_oper_6g_params = 0;
	uint32_t he_oper_len;

	if (!he_operation->present)
		return;
	if (he_operation->vht_oper_present)
		len += 3;
	if (he_operation->co_located_bss)
		len += 1;
	if (he_operation->oper_info_6g_present)
		len += 5;

	he_oper_len = sizeof(struct ieee80211_he_operation) + len;

	hdd_he_operation = qdf_mem_malloc(he_oper_len);
	if (!hdd_he_operation)
		return;

	/* Fill he_oper_params */
	he_oper_params |= he_operation->default_pe <<
					HE_OPERATION_DFLT_PE_DURATION_POS;
	he_oper_params |= he_operation->twt_required <<
					HE_OPERATION_TWT_REQUIRED_POS;
	he_oper_params |= he_operation->txop_rts_threshold <<
					HE_OPERATION_RTS_THRESHOLD_POS;
	he_oper_params |= he_operation->vht_oper_present <<
					HE_OPERATION_VHT_OPER_POS;
	he_oper_params |= he_operation->co_located_bss <<
					HE_OPERATION_CO_LOCATED_BSS_POS;
	he_oper_params |= he_operation->er_su_disable <<
					HE_OPERATION_ER_SU_DISABLE_POS;
	he_oper_params |= he_operation->oper_info_6g_present <<
					HE_OPERATION_OPER_INFO_6G_POS;
	he_oper_params |= he_operation->reserved2 <<
					HE_OPERATION_RESERVED_POS;
	he_oper_params |= he_operation->bss_color <<
					HE_OPERATION_BSS_COLOR_POS;
	he_oper_params |= he_operation->partial_bss_col <<
					HE_OPERATION_PARTIAL_BSS_COLOR_POS;
	he_oper_params |= he_operation->bss_col_disabled <<
					HE_OPERATION_BSS_COL_DISABLED_POS;

	hdd_he_operation->he_oper_params = he_oper_params;

	/* Fill he_mcs_nss set */
	qdf_mem_copy(&hdd_he_operation->he_mcs_nss_set,
		     he_operation->basic_mcs_nss,
		     sizeof(hdd_he_operation->he_mcs_nss_set));

	/* Fill he_params_optional fields */

	if (he_operation->vht_oper_present) {
		hdd_he_operation->optional[filled++] =
			he_operation->vht_oper.info.chan_width;
		hdd_he_operation->optional[filled++] =
			he_operation->vht_oper.info.center_freq_seg0;
		hdd_he_operation->optional[filled++] =
			he_operation->vht_oper.info.center_freq_seg1;
	}
	if (he_operation->co_located_bss)
		hdd_he_operation->optional[filled++] =
				he_operation->maxbssid_ind.info.data;

	if (he_operation->oper_info_6g_present) {
		hdd_he_operation->optional[filled++] =
			he_operation->oper_info_6g.info.primary_ch;
		he_oper_6g_params |=
			he_operation->oper_info_6g.info.ch_width << 0;
		he_oper_6g_params |=
			he_operation->oper_info_6g.info.dup_bcon << 2;
		he_oper_6g_params |=
			he_operation->oper_info_6g.info.reserved << 3;

		hdd_he_operation->optional[filled++] = he_oper_6g_params;
		hdd_he_operation->optional[filled++] =
			he_operation->oper_info_6g.info.center_freq_seg0;
		hdd_he_operation->optional[filled++] =
			he_operation->oper_info_6g.info.center_freq_seg1;
		hdd_he_operation->optional[filled] =
				he_operation->oper_info_6g.info.min_rate;
	}

	if (hdd_sta_ctx->cache_conn_info.he_operation) {
		qdf_mem_free(hdd_sta_ctx->cache_conn_info.he_operation);
		hdd_sta_ctx->cache_conn_info.he_operation = NULL;
	}

	hdd_sta_ctx->cache_conn_info.he_oper_len = he_oper_len;

	hdd_sta_ctx->cache_conn_info.he_operation = hdd_he_operation;
}
#endif

bool hdd_is_roam_sync_in_progress(struct hdd_context *hdd_ctx, uint8_t vdev_id)
{
	return MLME_IS_ROAM_SYNCH_IN_PROGRESS(hdd_ctx->psoc, vdev_id);
}

void hdd_conn_remove_connect_info(struct hdd_station_ctx *sta_ctx)
{
	/* Remove bssid and peer_macaddr */
	qdf_mem_zero(&sta_ctx->conn_info.bssid, QDF_MAC_ADDR_SIZE);
	qdf_mem_zero(&sta_ctx->conn_info.peer_macaddr[0],
		     QDF_MAC_ADDR_SIZE);

	/* Clear all security settings */
	sta_ctx->conn_info.auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	sta_ctx->conn_info.uc_encrypt_type = eCSR_ENCRYPT_TYPE_NONE;

	sta_ctx->conn_info.proxy_arp_service = 0;

	qdf_mem_zero(&sta_ctx->conn_info.ssid, sizeof(tCsrSSIDInfo));

	/*
	 * Reset the ptk, gtk status flags to avoid using current connection
	 * status in further connections.
	 */
	sta_ctx->conn_info.gtk_installed = false;
	sta_ctx->conn_info.ptk_installed = false;
}

void hdd_clear_roam_profile_ie(struct hdd_adapter *adapter)
{
	hdd_enter();

#ifdef FEATURE_WLAN_WAPI
	adapter->wapi_info.wapi_auth_mode = WAPI_AUTH_MODE_OPEN;
	adapter->wapi_info.wapi_mode = false;
#endif

	hdd_exit();
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

	if (!hdd_ctx)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("Invalid vdev_id");
		return;
	}
	complete(&adapter->sta_authorized_event);
}

#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
static inline
void hdd_set_unpause_queue(void *soc, struct hdd_adapter *adapter)
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
void hdd_set_unpause_queue(void *soc, struct hdd_adapter *adapter)
{ }
#endif

#ifdef FEATURE_WDS
/**
 * hdd_config_wds_repeater_mode() - configures vdev for wds repeater mode
 * @adapter: pointer to adapter
 * @peer_addr: peer mac address
 *
 * Configure dp vdev to detect and drop multicast echo packets and enable
 * 4 address frame format in fw.
 *
 * Return: None
 */
static void
hdd_config_wds_repeater_mode(struct hdd_adapter *adapter, uint8_t *peer_addr)
{
	cdp_config_param_type vdev_param;
	ol_txrx_soc_handle soc = cds_get_context(QDF_MODULE_ID_SOC);

	vdev_param.cdp_vdev_param_mec = true;
	if (cdp_txrx_set_vdev_param(soc, adapter->vdev_id, CDP_ENABLE_MEC,
				    vdev_param))
		hdd_debug("Failed to set MEC param on DP vdev");

	hdd_nofl_info("Turn on 4 address for peer: " QDF_MAC_ADDR_FMT,
		      QDF_MAC_ADDR_REF(peer_addr));
	if (sme_set_peer_param(peer_addr, WMI_HOST_PEER_USE_4ADDR, true,
			       adapter->vdev_id))
		hdd_err("Failed to enable WDS on vdev");
}
#else
static inline void
hdd_config_wds_repeater_mode(struct hdd_adapter *adapter, uint8_t *peer_addr)
{
}
#endif

QDF_STATUS hdd_change_peer_state(struct hdd_adapter *adapter,
				 uint8_t *peer_mac,
				 enum ol_txrx_peer_state sta_state)
{
	QDF_STATUS err;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	err = cdp_peer_state_update(soc, peer_mac, sta_state);
	if (err != QDF_STATUS_SUCCESS) {
		hdd_err("peer state update failed");
		return QDF_STATUS_E_FAULT;
	}

	if (hdd_is_roam_sync_in_progress(hdd_ctx, adapter->vdev_id))
		return QDF_STATUS_SUCCESS;

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
			hdd_set_unpause_queue(soc, adapter);
		}

		if (adapter->device_mode == QDF_STA_MODE &&
		    (wlan_mlme_get_wds_mode(hdd_ctx->psoc) ==
		    WLAN_WDS_MODE_REPEATER))
			hdd_config_wds_repeater_mode(adapter, peer_mac);
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_update_dp_vdev_flags(void *cbk_data,
				    uint8_t vdev_id,
				    uint32_t vdev_param,
				    bool is_link_up)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct hdd_context *hdd_ctx;
	struct wlan_objmgr_psoc **psoc;
	cdp_config_param_type val;

	if (!cbk_data)
		return status;

	psoc = cbk_data;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx)
		return QDF_STATUS_E_INVAL;

	if (!hdd_ctx->tdls_nap_active)
		return status;

	if (vdev_id == WLAN_INVALID_VDEV_ID) {
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	val.cdp_vdev_param_tdls_flags = is_link_up;
	cdp_txrx_set_vdev_param(soc, vdev_id, vdev_param, val);

	return status;
}

#if defined(WLAN_SUPPORT_RX_FISA)
/**
 * hdd_rx_register_fisa_ops() - FISA callback functions
 * @txrx_ops: operations handle holding callback functions
 * @hdd_rx_fisa_cbk: callback for fisa aggregation handle function
 * @hdd_rx_fisa_flush: callback function to flush fisa aggregation
 *
 * Return: None
 */
static inline void
hdd_rx_register_fisa_ops(struct ol_txrx_ops *txrx_ops)
{
	txrx_ops->rx.osif_fisa_rx = hdd_rx_fisa_cbk;
	txrx_ops->rx.osif_fisa_flush = hdd_rx_fisa_flush_by_ctx_id;
}
#else
static inline void
hdd_rx_register_fisa_ops(struct ol_txrx_ops *txrx_ops)
{
}
#endif

QDF_STATUS hdd_roam_register_sta(struct hdd_adapter *adapter,
				 struct qdf_mac_addr *bssid,
				 bool is_auth_required)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type txrx_desc = {0};
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	/* Get the Station ID from the one saved during the association */
	if (!QDF_IS_ADDR_BROADCAST(bssid->bytes))
		WLAN_ADDR_COPY(txrx_desc.peer_addr.bytes,
			       bssid->bytes);
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
		adapter->rx_stack = hdd_rx_packet_cbk;
	} else {
		txrx_ops.rx.rx = hdd_rx_packet_cbk;
		txrx_ops.rx.rx_stack = NULL;
		txrx_ops.rx.rx_flush = NULL;
	}

	if (adapter->hdd_ctx->config->fisa_enable &&
		(adapter->device_mode != QDF_MONITOR_MODE)) {
		hdd_debug("FISA feature enabled");
		hdd_rx_register_fisa_ops(&txrx_ops);
	}

	txrx_ops.rx.stats_rx = hdd_tx_rx_collect_connectivity_stats_info;

	txrx_ops.tx.tx_comp = hdd_sta_notify_tx_comp_cb;
	txrx_ops.tx.tx = NULL;
	cdp_vdev_register(soc, adapter->vdev_id, (ol_osif_vdev_handle)adapter,
			  &txrx_ops);
	if (!txrx_ops.tx.tx) {
		hdd_err("vdev register fail");
		return QDF_STATUS_E_FAILURE;
	}

	adapter->tx_fn = txrx_ops.tx.tx;
	qdf_status = cdp_peer_register(soc, OL_TXRX_PDEV_ID, &txrx_desc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed Status: %d [0x%08X]",
			 qdf_status, qdf_status);
		return qdf_status;
	}

	hdd_cm_set_peer_authenticate(adapter, &txrx_desc.peer_addr,
				     is_auth_required);

	return qdf_status;
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
static int hdd_change_sta_state_authenticated(struct hdd_adapter *adapter,
					      struct csr_roam_info *roaminfo)
{
	uint8_t *mac_addr;
	struct hdd_station_ctx *hddstactx =
		WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	mac_addr = hddstactx->conn_info.bssid.bytes;

	if (ucfg_ipa_is_enabled() && !hddstactx->conn_info.is_authenticated &&
	    adapter->device_mode == QDF_STA_MODE &&
	    hddstactx->conn_info.auth_type != eCSR_AUTH_TYPE_NONE &&
	    hddstactx->conn_info.auth_type != eCSR_AUTH_TYPE_OPEN_SYSTEM &&
	    hddstactx->conn_info.auth_type != eCSR_AUTH_TYPE_SHARED_KEY)
		ucfg_ipa_wlan_evt(adapter->hdd_ctx->pdev, adapter->dev,
				  adapter->device_mode, adapter->vdev_id,
				  WLAN_IPA_STA_CONNECT, mac_addr,
				  WLAN_REG_IS_24GHZ_CH_FREQ(
					hddstactx->conn_info.chan_freq));

	hdd_cm_set_peer_authenticate(adapter, &hddstactx->conn_info.bssid,
				     false);

	return 0;
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
		  QDF_MAC_ADDR_FMT, roam_status, roam_result,
		  QDF_MAC_ADDR_REF(roam_info->peerMac.bytes));

	connected = hdd_conn_get_connected_cipher_algo(adapter, sta_ctx,
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

bool hdd_save_peer(struct hdd_station_ctx *sta_ctx,
		   struct qdf_mac_addr *peer_mac_addr)
{
	int idx;
	struct qdf_mac_addr *mac_addr;

	for (idx = 0; idx < MAX_PEERS; idx++) {
		mac_addr = &sta_ctx->conn_info.peer_macaddr[idx];
		if (qdf_is_macaddr_zero(mac_addr)) {
			hdd_debug("adding peer: "QDF_MAC_ADDR_FMT" at idx: %d",
				  QDF_MAC_ADDR_REF(peer_mac_addr->bytes), idx);
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

	for (i = 0; i < MAX_PEERS; i++) {
		mac_addr = &sta_ctx->conn_info.peer_macaddr[i];
		if (qdf_is_macaddr_equal(mac_addr, peer_mac_addr)) {
			qdf_zero_macaddr(mac_addr);
			return;
		}
	}
}

bool hdd_any_valid_peer_present(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	int i;
	struct qdf_mac_addr *mac_addr;

	for (i = 0; i < MAX_PEERS; i++) {
		mac_addr = &sta_ctx->conn_info.peer_macaddr[i];
		if (!qdf_is_macaddr_zero(mac_addr) &&
		    !qdf_is_macaddr_broadcast(mac_addr)) {
			hdd_debug("peer: index: %u " QDF_MAC_ADDR_FMT, i,
				  QDF_MAC_ADDR_REF(mac_addr->bytes));
			return true;
		}
	}

	return false;
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
	tSirMicFailureInfo *mic_failure_info;

	if (!hdd_cm_is_vdev_associated(adapter))
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

#ifdef FEATURE_WLAN_TDLS
QDF_STATUS hdd_roam_register_tdlssta(struct hdd_adapter *adapter,
				     const uint8_t *peerMac, uint8_t qos)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct ol_txrx_desc_type txrx_desc = { 0 };
	struct ol_txrx_ops txrx_ops;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

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
		adapter->rx_stack = hdd_rx_packet_cbk;
	} else {
		txrx_ops.rx.rx = hdd_rx_packet_cbk;
		txrx_ops.rx.rx_stack = NULL;
		txrx_ops.rx.rx_flush = NULL;
	}
	cdp_vdev_register(soc, adapter->vdev_id, (ol_osif_vdev_handle)adapter,
			  &txrx_ops);
	adapter->tx_fn = txrx_ops.tx.tx;
	txrx_ops.rx.stats_rx = hdd_tx_rx_collect_connectivity_stats_info;

	/* Register the Station with TL...  */
	qdf_status = cdp_peer_register(soc, OL_TXRX_PDEV_ID, &txrx_desc);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("cdp_peer_register() failed Status: %d [0x%08X]",
			qdf_status, qdf_status);
		return qdf_status;
	}

	return qdf_status;
}

#endif

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
	hdd_wext_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
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
	hdd_wext_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
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
	hdd_wext_send_event(adapter->dev, IWEVCUSTOM, &wrqu, buf);
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
			hdd_wext_send_event(adapter->dev, IWEVCUSTOM, &wrqu,
					    buf);
			lastSent += sendBss;
		}
	}
}

#endif /* FEATURE_WLAN_ESE */

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
	QDF_STATUS status;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	mac_handle_t mac_handle = hdd_adapter_get_mac_handle(adapter);
	struct hdd_station_ctx *sta_ctx;
	uint8_t connected_vdev;
	bool notify = true;

	/* Enable Roaming on STA interface which was disabled before CSA */
	if (adapter->device_mode == QDF_STA_MODE)
		sme_start_roaming(mac_handle, adapter->vdev_id,
				  REASON_DRIVER_ENABLED,
				  RSO_CHANNEL_SWITCH);

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (sta_ctx) {
		sta_ctx->conn_info.chan_freq = roam_info->chan_info.mhz;
		sta_ctx->conn_info.ch_width = roam_info->chan_info.ch_width;
	}

	chan_change.chan_freq = roam_info->chan_info.mhz;
	chan_change.chan_params.ch_width =
		roam_info->chan_info.ch_width;
	chan_change.chan_params.sec_ch_offset =
		roam_info->chan_info.sec_ch_offset;
	chan_change.chan_params.mhz_freq_seg0 =
		roam_info->chan_info.band_center_freq1;
	chan_change.chan_params.mhz_freq_seg1 =
		roam_info->chan_info.band_center_freq2;

	if ((adapter->device_mode == QDF_STA_MODE ||
	     adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		if (!wlan_get_connected_vdev_by_bssid(
				hdd_ctx->pdev, sta_ctx->conn_info.bssid.bytes,
				&connected_vdev))
			notify = false;
		else if (adapter->vdev_id != connected_vdev)
			notify = false;
	}
	if (notify) {
		status = hdd_chan_change_notify(adapter, adapter->dev,
						chan_change,
						roam_info->mode ==
						SIR_SME_PHY_MODE_LEGACY);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("channel change notification failed");
	} else {
		hdd_err("BSS "QDF_MAC_ADDR_FMT" no connected with vdev %d (%d)",
			QDF_MAC_ADDR_REF(sta_ctx->conn_info.bssid.bytes),
			adapter->vdev_id, connected_vdev);
	}
	status = policy_mgr_set_hw_mode_on_channel_switch(hdd_ctx->psoc,
		adapter->vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_debug("set hw mode change not done");

	policy_mgr_check_concurrent_intf_and_restart_sap(hdd_ctx->psoc);
	wlan_twt_concurrency_update(hdd_ctx);
}

#ifdef WLAN_FEATURE_HOST_ROAM
void wlan_hdd_ft_set_key_delay(struct wlan_objmgr_vdev *vdev)
{
	int errno = 0;

	if (ucfg_cm_ft_key_ready_for_install(vdev))
		errno =
		wlan_cfg80211_crypto_add_key(vdev,
					     WLAN_CRYPTO_KEY_TYPE_UNICAST,
					     0, false);
	if (errno)
		hdd_err("ft set key failed");
}
#endif

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
	struct hdd_context *hdd_ctx;

	hdd_debug("CSR Callback: status=%s (%d) result= %s (%d) roamID=%d",
		  get_e_roam_cmd_status_str(roam_status), roam_status,
		  get_e_csr_roam_result_str(roam_result), roam_result, roam_id);

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
	case eCSR_ROAM_MIC_ERROR_IND:
		hdd_roam_mic_error_indication_handler(adapter, roam_info);
		break;

	case eCSR_ROAM_SET_KEY_COMPLETE:
	{
		qdf_ret_status =
			hdd_roam_set_key_complete_handler(adapter, roam_info,
							  roam_id, roam_status,
							  roam_result);
		if (eCSR_ROAM_RESULT_AUTHENTICATED == roam_result)
			hdd_debug("set key complete, session: %d",
				  adapter->vdev_id);
	}
		break;
	case eCSR_ROAM_UNPROT_MGMT_FRAME_IND:
		if (roam_info)
			hdd_indicate_unprot_mgmt_frame(adapter,
					       roam_info->nFrameLength,
					       roam_info->pbFrames,
					       roam_info->frameType);
		break;
#ifdef FEATURE_WLAN_ESE
	case eCSR_ROAM_TSM_IE_IND:
		if (roam_info)
			hdd_indicate_tsm_ie(adapter, roam_info->tsm_ie.tsid,
				    roam_info->tsm_ie.state,
				    roam_info->tsm_ie.msmt_interval);
		break;
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

	case eCSR_ROAM_NDP_STATUS_UPDATE:
		hdd_ndp_event_handler(adapter, roam_info, roam_id, roam_status,
			roam_result);
		break;
	case eCSR_ROAM_SAE_COMPUTE:
		if (roam_info)
			wlan_hdd_sae_callback(adapter, roam_info);
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
	if (memcmp(auth_suite, ccp_rsn_oui07, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_RSN_PSK_SHA256;
	} else if (memcmp(auth_suite, ccp_rsn_oui08, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_RSN_8021X_SHA256;
	} else if (memcmp(auth_suite, ccp_rsn_oui_18, 4) == 0) {
		auth_type = eCSR_AUTH_TYPE_OWE;
	} else if (memcmp(auth_suite, ccp_rsn_oui_12, 4) == 0) {
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

	return cipher_type;
}

#ifdef WLAN_FEATURE_FILS_SK
bool hdd_is_fils_connection(struct hdd_context *hdd_ctx,
			    struct hdd_adapter *adapter)
{
	struct wlan_fils_connection_info *fils_info;

	fils_info = wlan_cm_get_fils_connection_info(hdd_ctx->psoc,
						     adapter->vdev_id);
	if (fils_info)
		return fils_info->is_fils_connection;

	return false;
}
#else
bool hdd_is_fils_connection(struct hdd_context *hdd_ctx,
			    struct hdd_adapter *adapter)
{
	return false;
}
#endif

void hdd_roam_profile_init(struct hdd_adapter *adapter)
{
	struct csr_roam_profile *roam_profile;
	struct hdd_station_ctx *sta_ctx;

	hdd_enter();

	roam_profile = hdd_roam_profile(adapter);
	qdf_mem_zero(roam_profile, sizeof(*roam_profile));

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

	/* Set the default scan mode */
	adapter->scan_info.scan_mode = eSIR_ACTIVE_SCAN;

	hdd_clear_roam_profile_ie(adapter);
	hdd_exit();
}

struct osif_cm_ops osif_ops = {
	.connect_complete_cb = hdd_cm_connect_complete,
	.disconnect_complete_cb = hdd_cm_disconnect_complete,
	.netif_queue_control_cb = hdd_cm_netif_queue_control,
	.napi_serialize_control_cb = hdd_cm_napi_serialize_control,
	.save_gtk_cb = hdd_cm_save_gtk,
#ifdef WLAN_FEATURE_FILS_SK
	.set_hlp_data_cb = hdd_cm_set_hlp_data,
#endif
#ifdef WLAN_FEATURE_PREAUTH_ENABLE
	.ft_preauth_complete_cb = hdd_cm_ft_preauth_complete,
#ifdef FEATURE_WLAN_ESE
	.cckm_preauth_complete_cb = hdd_cm_cckm_preauth_complete,
#endif
#endif
};

QDF_STATUS hdd_cm_register_cb(void)
{
	QDF_STATUS status;
	osif_cm_set_legacy_cb(&osif_ops);

	status = osif_cm_register_cb();
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	/* Overwrite with UTF cb if UTF enabled */
	return cm_utf_register_os_if_cb();
}

void hdd_cm_unregister_cb(void)
{
	osif_cm_reset_legacy_cb();
}
