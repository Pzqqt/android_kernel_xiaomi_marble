/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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
 * DOC: csr_util.c
 *
 * Implementation supporting routines for CSR.
 */

#include "ani_global.h"

#include "csr_support.h"
#include "csr_inside_api.h"
#include "sme_qos_internal.h"
#include "wma_types.h"
#include "cds_utils.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_serialization_legacy_api.h"
#include "wlan_reg_services_api.h"
#include "wlan_crypto_global_api.h"


uint8_t csr_wpa_oui[][CSR_WPA_OUI_SIZE] = {
	{0x00, 0x50, 0xf2, 0x00}
	,
	{0x00, 0x50, 0xf2, 0x01}
	,
	{0x00, 0x50, 0xf2, 0x02}
	,
	{0x00, 0x50, 0xf2, 0x03}
	,
	{0x00, 0x50, 0xf2, 0x04}
	,
	{0x00, 0x50, 0xf2, 0x05}
	,
#ifdef FEATURE_WLAN_ESE
	{0x00, 0x40, 0x96, 0x00}
	,                       /* CCKM */
#endif /* FEATURE_WLAN_ESE */
};

/*
 * PLEASE DO NOT ADD THE #IFDEF IN BELOW TABLE,
 * IF STILL REQUIRE THEN PLEASE ADD NULL ENTRIES
 * OTHERWISE IT WILL BREAK OTHER LOWER
 * SECUIRTY MODES.
 */

uint8_t csr_rsn_oui[][CSR_RSN_OUI_SIZE] = {
	{0x00, 0x0F, 0xAC, 0x00}
	,                       /* group cipher */
	{0x00, 0x0F, 0xAC, 0x01}
	,                       /* WEP-40 or RSN */
	{0x00, 0x0F, 0xAC, 0x02}
	,                       /* TKIP or RSN-PSK */
	{0x00, 0x0F, 0xAC, 0x03}
	,                       /* Reserved */
	{0x00, 0x0F, 0xAC, 0x04}
	,                       /* AES-CCMP */
	{0x00, 0x0F, 0xAC, 0x05}
	,                       /* WEP-104 */
	{0x00, 0x40, 0x96, 0x00}
	,                       /* CCKM */
	{0x00, 0x0F, 0xAC, 0x06}
	,                       /* BIP (encryption type) or
				 * RSN-PSK-SHA256 (authentication type)
				 */
	/* RSN-8021X-SHA256 (authentication type) */
	{0x00, 0x0F, 0xAC, 0x05},
#ifdef WLAN_FEATURE_FILS_SK
#define ENUM_FILS_SHA256 9
	/* FILS SHA256 */
	{0x00, 0x0F, 0xAC, 0x0E},
#define ENUM_FILS_SHA384 10
	/* FILS SHA384 */
	{0x00, 0x0F, 0xAC, 0x0F},
#define ENUM_FT_FILS_SHA256 11
	/* FILS FT SHA256 */
	{0x00, 0x0F, 0xAC, 0x10},
#define ENUM_FT_FILS_SHA384 12
	/* FILS FT SHA384 */
	{0x00, 0x0F, 0xAC, 0x11},
#else
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
#endif
	/* AES GCMP */
	{0x00, 0x0F, 0xAC, 0x08},
	/* AES GCMP-256 */
	{0x00, 0x0F, 0xAC, 0x09},
#define ENUM_DPP_RSN 15
	/* DPP RSN */
	{0x50, 0x6F, 0x9A, 0x02},
#define ENUM_OWE 16
	/* OWE https://tools.ietf.org/html/rfc8110 */
	{0x00, 0x0F, 0xAC, 0x12},
#define ENUM_SUITEB_EAP256 17
	{0x00, 0x0F, 0xAC, 0x0B},
#define ENUM_SUITEB_EAP384 18
	{0x00, 0x0F, 0xAC, 0x0C},

#ifdef WLAN_FEATURE_SAE
#define ENUM_SAE 19
	/* SAE */
	{0x00, 0x0F, 0xAC, 0x08},
#define ENUM_FT_SAE 20
	/* FT SAE */
	{0x00, 0x0F, 0xAC, 0x09},
#else
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
#endif
#define ENUM_OSEN 21
	/* OSEN RSN */
	{0x50, 0x6F, 0x9A, 0x01},
#define ENUM_FT_SUITEB_SHA384 22
	/* FT Suite-B SHA384 */
	{0x00, 0x0F, 0xAC, 0x0D},

	/* define new oui here, update #define CSR_OUI_***_INDEX  */
};

#ifdef FEATURE_WLAN_WAPI
uint8_t csr_wapi_oui[][CSR_WAPI_OUI_SIZE] = {
	{0x00, 0x14, 0x72, 0x00}
	,                       /* Reserved */
	{0x00, 0x14, 0x72, 0x01}
	,                       /* WAI certificate or SMS4 */
	{0x00, 0x14, 0x72, 0x02} /* WAI PSK */
};
#endif /* FEATURE_WLAN_WAPI */

uint8_t csr_group_mgmt_oui[][CSR_RSN_OUI_SIZE] = {
#define ENUM_CMAC 0
	{0x00, 0x0F, 0xAC, 0x06},
#define ENUM_GMAC_128 1
	{0x00, 0x0F, 0xAC, 0x0B},
#define ENUM_GMAC_256 2
	{0x00, 0x0F, 0xAC, 0x0C},
};


/* ////////////////////////////////////////////////////////////////////// */

/**
 * \var g_phy_rates_suppt
 *
 * \brief Rate support lookup table
 *
 *
 * This is a  lookup table indexing rates &  configuration parameters to
 * support.  Given a rate (in  unites of 0.5Mpbs) & three bools (MIMO
 * Enabled, Channel  Bonding Enabled, & Concatenation  Enabled), one can
 * determine  whether  the given  rate  is  supported  by computing  two
 * indices.  The  first maps  the rate to  table row as  indicated below
 * (i.e. eHddSuppRate_6Mbps maps to  row zero, eHddSuppRate_9Mbps to row
 * 1, and so on).  Index two can be computed like so:
 *
 * \code
 *  idx2 = ( fEsf  ? 0x4 : 0x0 ) |
 *         ( fCb   ? 0x2 : 0x0 ) |
 *         ( fMimo ? 0x1 : 0x0 );
 * \endcode
 *
 *
 * Given that:
 *
 *  \code
 *  fSupported = g_phy_rates_suppt[idx1][idx2];
 *  \endcode
 *
 *
 * This table is based on  the document "PHY Supported Rates.doc".  This
 * table is  permissive in that a  rate is reflected  as being supported
 * even  when turning  off an  enabled feature  would be  required.  For
 * instance, "PHY Supported Rates"  lists 42Mpbs as unsupported when CB,
 * ESF, &  MIMO are all  on.  However,  if we turn  off either of  CB or
 * MIMO, it then becomes supported.   Therefore, we mark it as supported
 * even in index 7 of this table.
 *
 *
 */

static const bool g_phy_rates_suppt[24][8] = {

	/* SSF   SSF    SSF    SSF    ESF    ESF    ESF    ESF */
	/* SIMO  MIMO   SIMO   MIMO   SIMO   MIMO   SIMO   MIMO */
	/* No CB No CB  CB     CB     No CB  No CB  CB     CB */
	{true, true, true, true, true, true, true, true},       /* 6Mbps */
	{true, true, true, true, true, true, true, true},       /* 9Mbps */
	{true, true, true, true, true, true, true, true},       /* 12Mbps */
	{true, true, true, true, true, true, true, true},       /* 18Mbps */
	{false, false, true, true, false, false, true, true},   /* 20Mbps */
	{true, true, true, true, true, true, true, true},       /* 24Mbps */
	{true, true, true, true, true, true, true, true},       /* 36Mbps */
	{false, false, true, true, false, true, true, true},    /* 40Mbps */
	{false, false, true, true, false, true, true, true},    /* 42Mbps */
	{true, true, true, true, true, true, true, true},       /* 48Mbps */
	{true, true, true, true, true, true, true, true},       /* 54Mbps */
	{false, true, true, true, false, true, true, true},     /* 72Mbps */
	{false, false, true, true, false, true, true, true},    /* 80Mbps */
	{false, false, true, true, false, true, true, true},    /* 84Mbps */
	{false, true, true, true, false, true, true, true},     /* 96Mbps */
	{false, true, true, true, false, true, true, true},     /* 108Mbps */
	{false, false, true, true, false, true, true, true},    /* 120Mbps */
	{false, false, true, true, false, true, true, true},    /* 126Mbps */
	{false, false, false, true, false, false, false, true}, /* 144Mbps */
	{false, false, false, true, false, false, false, true}, /* 160Mbps */
	{false, false, false, true, false, false, false, true}, /* 168Mbps */
	{false, false, false, true, false, false, false, true}, /* 192Mbps */
	{false, false, false, true, false, false, false, true}, /* 216Mbps */
	{false, false, false, true, false, false, false, true}, /* 240Mbps */

};

#define CASE_RETURN_STR(n) {\
	case (n): return (# n);\
}

const char *get_e_roam_cmd_status_str(eRoamCmdStatus val)
{
	switch (val) {
		CASE_RETURN_STR(eCSR_ROAM_CANCELLED);
		CASE_RETURN_STR(eCSR_ROAM_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_ROAMING_START);
		CASE_RETURN_STR(eCSR_ROAM_ROAMING_COMPLETION);
		CASE_RETURN_STR(eCSR_ROAM_CONNECT_COMPLETION);
		CASE_RETURN_STR(eCSR_ROAM_ASSOCIATION_START);
		CASE_RETURN_STR(eCSR_ROAM_ASSOCIATION_COMPLETION);
		CASE_RETURN_STR(eCSR_ROAM_DISASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_ASSOCIATION_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_SHOULD_ROAM);
		CASE_RETURN_STR(eCSR_ROAM_SCAN_FOUND_NEW_BSS);
		CASE_RETURN_STR(eCSR_ROAM_LOSTLINK);
		CASE_RETURN_STR(eCSR_ROAM_LOSTLINK_DETECTED);
		CASE_RETURN_STR(eCSR_ROAM_MIC_ERROR_IND);
		CASE_RETURN_STR(eCSR_ROAM_IBSS_IND);
		CASE_RETURN_STR(eCSR_ROAM_CONNECT_STATUS_UPDATE);
		CASE_RETURN_STR(eCSR_ROAM_GEN_INFO);
		CASE_RETURN_STR(eCSR_ROAM_SET_KEY_COMPLETE);
		CASE_RETURN_STR(eCSR_ROAM_IBSS_LEAVE);
		CASE_RETURN_STR(eCSR_ROAM_INFRA_IND);
		CASE_RETURN_STR(eCSR_ROAM_WPS_PBC_PROBE_REQ_IND);
		CASE_RETURN_STR(eCSR_ROAM_FT_RESPONSE);
		CASE_RETURN_STR(eCSR_ROAM_FT_START);
		CASE_RETURN_STR(eCSR_ROAM_SESSION_OPENED);
		CASE_RETURN_STR(eCSR_ROAM_FT_REASSOC_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_PMK_NOTIFY);
#ifdef FEATURE_WLAN_LFR_METRICS
		CASE_RETURN_STR(eCSR_ROAM_PREAUTH_INIT_NOTIFY);
		CASE_RETURN_STR(eCSR_ROAM_PREAUTH_STATUS_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_PREAUTH_STATUS_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_HANDOVER_SUCCESS);
#endif
#ifdef FEATURE_WLAN_TDLS
		CASE_RETURN_STR(eCSR_ROAM_TDLS_STATUS_UPDATE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MGMT_TX_COMPLETE_IND);
#endif
		CASE_RETURN_STR(eCSR_ROAM_DISCONNECT_ALL_P2P_CLIENTS);
		CASE_RETURN_STR(eCSR_ROAM_SEND_P2P_STOP_BSS);
#ifdef WLAN_FEATURE_11W
		CASE_RETURN_STR(eCSR_ROAM_UNPROT_MGMT_FRAME_IND);
#endif
#ifdef WLAN_FEATURE_RMC
		CASE_RETURN_STR(eCSR_ROAM_IBSS_PEER_INFO_COMPLETE);
#endif
#ifdef FEATURE_WLAN_ESE
		CASE_RETURN_STR(eCSR_ROAM_TSM_IE_IND);
		CASE_RETURN_STR(eCSR_ROAM_CCKM_PREAUTH_NOTIFY);
		CASE_RETURN_STR(eCSR_ROAM_ESE_ADJ_AP_REPORT_IND);
		CASE_RETURN_STR(eCSR_ROAM_ESE_BCN_REPORT_IND);
#endif /* FEATURE_WLAN_ESE */
		CASE_RETURN_STR(eCSR_ROAM_DFS_RADAR_IND);
		CASE_RETURN_STR(eCSR_ROAM_SET_CHANNEL_RSP);
		CASE_RETURN_STR(eCSR_ROAM_DFS_CHAN_SW_NOTIFY);
		CASE_RETURN_STR(eCSR_ROAM_EXT_CHG_CHNL_IND);
		CASE_RETURN_STR(eCSR_ROAM_STA_CHANNEL_SWITCH);
		CASE_RETURN_STR(eCSR_ROAM_NDP_STATUS_UPDATE);
		CASE_RETURN_STR(eCSR_ROAM_UPDATE_SCAN_RESULT);
		CASE_RETURN_STR(eCSR_ROAM_START);
		CASE_RETURN_STR(eCSR_ROAM_ABORT);
		CASE_RETURN_STR(eCSR_ROAM_NAPI_OFF);
		CASE_RETURN_STR(eCSR_ROAM_CHANNEL_COMPLETE_IND);
		CASE_RETURN_STR(eCSR_ROAM_SAE_COMPUTE);
	default:
		return "unknown";
	}
}

const char *get_e_csr_roam_result_str(eCsrRoamResult val)
{
	switch (val) {
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NONE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_ASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NOT_ASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MIC_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_FORCED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DISASSOC_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DEAUTH_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CAP_CHANGED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_STARTED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_START_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_JOIN_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_CONNECT);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_INACTIVE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_NEW_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_COALESCED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_STOP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_LOSTLINK);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MIC_ERROR_UNICAST);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MIC_ERROR_GROUP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_AUTHENTICATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NEW_RSN_BSS);
 #ifdef FEATURE_WLAN_WAPI
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NEW_WAPI_BSS);
 #endif /* FEATURE_WLAN_WAPI */
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_STARTED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_START_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_STOPPED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_INFRA_DISASSOCIATED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_WPS_PBC_PROBE_REQ_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_SEND_ACTION_FAIL);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_MAX_ASSOC_EXCEEDED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_ADD_TDLS_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_UPDATE_TDLS_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DELETE_TDLS_PEER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_LINK_ESTABLISH_REQ_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED);
		CASE_RETURN_STR
			(eCSR_ROAM_RESULT_TDLS_CONNECTION_TRACKER_NOTIFICATION);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_PEER_INFO_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_IBSS_PEER_INFO_FAILED);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_CSA_RESTART_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS);
		CASE_RETURN_STR(eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDI_CREATE_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDI_DELETE_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_INITIATOR_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_NEW_PEER_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_CONFIRM_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_INDICATION);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_SCHED_UPDATE_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_RESPONDER_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_END_RSP);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_PEER_DEPARTED_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_NDP_END_IND);
		CASE_RETURN_STR(eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	default:
		return "unknown";
	}
}

const char *csr_phy_mode_str(eCsrPhyMode phy_mode)
{
	switch (phy_mode) {
	case eCSR_DOT11_MODE_abg:
		return "abg";
	case eCSR_DOT11_MODE_11a:
		return "11a";
	case eCSR_DOT11_MODE_11b:
		return "11b";
	case eCSR_DOT11_MODE_11g:
		return "11g";
	case eCSR_DOT11_MODE_11n:
		return "11n";
	case eCSR_DOT11_MODE_11g_ONLY:
		return "11g_only";
	case eCSR_DOT11_MODE_11n_ONLY:
		return "11n_only";
	case eCSR_DOT11_MODE_11b_ONLY:
		return "11b_only";
	case eCSR_DOT11_MODE_11ac:
		return "11ac";
	case eCSR_DOT11_MODE_11ac_ONLY:
		return "11ac_only";
	case eCSR_DOT11_MODE_AUTO:
		return "auto";
	case eCSR_DOT11_MODE_11ax:
		return "11ax";
	case eCSR_DOT11_MODE_11ax_ONLY:
		return "11ax_only";
	default:
		return "unknown";
	}
}

void csr_purge_vdev_pending_ser_cmd_list(struct mac_context *mac_ctx,
					 uint32_t vdev_id)
{
	wlan_serialization_purge_cmd_list_by_vdev_id(mac_ctx->psoc, vdev_id,
						     false, true, false,
						     true, false);
}

void csr_purge_vdev_all_scan_ser_cmd_list(struct mac_context *mac_ctx,
					  uint32_t vdev_id)
{
	wlan_serialization_purge_cmd_list_by_vdev_id(mac_ctx->psoc, vdev_id,
						     true, true, false,
						     false, false);
}

void csr_purge_pdev_all_ser_cmd_list(struct mac_context *mac_ctx)
{
	wlan_serialization_purge_cmd_list(mac_ctx->psoc, NULL, true, true,
					  true, true, true);
}

uint32_t csr_nonscan_active_ll_count(struct mac_context *mac_ctx)
{
	return wlan_serialization_get_active_list_count(mac_ctx->psoc, false);
}

uint32_t csr_nonscan_pending_ll_count(struct mac_context *mac_ctx)
{
	return wlan_serialization_get_pending_list_count(mac_ctx->psoc, false);
}

bool csr_nonscan_active_ll_is_list_empty(struct mac_context *mac_ctx,
					 bool inter_locked)
{
	return !wlan_serialization_get_active_list_count(mac_ctx->psoc, false);
}

bool csr_nonscan_pending_ll_is_list_empty(struct mac_context *mac_ctx,
					  bool inter_locked)
{
	return !wlan_serialization_get_pending_list_count(mac_ctx->psoc, false);
}

tListElem *csr_nonscan_active_ll_peek_head(struct mac_context *mac_ctx,
					   bool inter_locked)
{
	struct wlan_serialization_command *cmd;
	tSmeCmd *sme_cmd;

	cmd = wlan_serialization_peek_head_active_cmd_using_psoc(mac_ctx->psoc,
								 false);
	if (!cmd) {
		sme_err("No cmd found");
		return NULL;
	}
	sme_cmd = cmd->umac_cmd;

	return &sme_cmd->Link;
}

tListElem *csr_nonscan_pending_ll_peek_head(struct mac_context *mac_ctx,
					    bool inter_locked)
{
	struct wlan_serialization_command *cmd;
	tSmeCmd *sme_cmd;

	cmd = wlan_serialization_peek_head_pending_cmd_using_psoc(mac_ctx->psoc,
								  false);
	if (!cmd)
		return NULL;

	sme_cmd = cmd->umac_cmd;

	return &sme_cmd->Link;
}

bool csr_nonscan_active_ll_remove_entry(struct mac_context *mac_ctx,
					tListElem *entry, bool inter_locked)
{
	tListElem *head;

	head = csr_nonscan_active_ll_peek_head(mac_ctx, inter_locked);
	if (head == entry)
	return true;

	return false;
}

tListElem *csr_nonscan_active_ll_remove_head(struct mac_context *mac_ctx,
					     bool inter_locked)
{
	return csr_nonscan_active_ll_peek_head(mac_ctx, inter_locked);
}

tListElem *csr_nonscan_pending_ll_remove_head(struct mac_context *mac_ctx,
					      bool inter_locked)
{
	return csr_nonscan_pending_ll_peek_head(mac_ctx, inter_locked);
}

tListElem *csr_nonscan_pending_ll_next(struct mac_context *mac_ctx,
				       tListElem *entry, bool inter_locked)
{
	tSmeCmd *sme_cmd;
	struct wlan_serialization_command cmd, *tcmd;

	if (!entry)
		return NULL;
	sme_cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
	cmd.cmd_id = sme_cmd->cmd_id;
	cmd.cmd_type = csr_get_cmd_type(sme_cmd);
	cmd.vdev = wlan_objmgr_get_vdev_by_id_from_psoc_no_state(
				mac_ctx->psoc,
				sme_cmd->sessionId, WLAN_LEGACY_SME_ID);
	tcmd = wlan_serialization_get_pending_list_next_node_using_psoc(
				mac_ctx->psoc, &cmd, false);
	if (cmd.vdev)
		wlan_objmgr_vdev_release_ref(cmd.vdev, WLAN_LEGACY_SME_ID);
	if (!tcmd) {
		sme_err("No cmd found");
		return NULL;
	}
	sme_cmd = tcmd->umac_cmd;
	return &sme_cmd->Link;
}

bool csr_get_bss_id_bss_desc(tSirBssDescription *pSirBssDesc,
			     struct qdf_mac_addr *pBssId)
{
	qdf_mem_copy(pBssId, &pSirBssDesc->bssId[0],
			sizeof(struct qdf_mac_addr));
	return true;
}

bool csr_is_bss_id_equal(tSirBssDescription *pSirBssDesc1,
			 tSirBssDescription *pSirBssDesc2)
{
	bool fEqual = false;
	struct qdf_mac_addr bssId1;
	struct qdf_mac_addr bssId2;

	do {
		if (!pSirBssDesc1)
			break;
		if (!pSirBssDesc2)
			break;

		if (!csr_get_bss_id_bss_desc(pSirBssDesc1, &bssId1))
			break;
		if (!csr_get_bss_id_bss_desc(pSirBssDesc2, &bssId2))
			break;

		fEqual = qdf_is_macaddr_equal(&bssId1, &bssId2);
	} while (0);

	return fEqual;
}

static bool csr_is_conn_state(struct mac_context *mac_ctx, uint32_t session_id,
			      eCsrConnectState state)
{
	QDF_BUG(session_id < WLAN_MAX_VDEVS);
	if (session_id >= WLAN_MAX_VDEVS)
		return false;

	return mac_ctx->roam.roamSession[session_id].connectState == state;
}

bool csr_is_conn_state_connected_ibss(struct mac_context *mac_ctx,
				      uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_IBSS_CONNECTED);
}

bool csr_is_conn_state_disconnected_ibss(struct mac_context *mac_ctx,
					 uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED);
}

bool csr_is_conn_state_connected_infra(struct mac_context *mac_ctx,
				       uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED);
}

bool csr_is_conn_state_connected(struct mac_context *mac, uint32_t sessionId)
{
	return csr_is_conn_state_connected_ibss(mac, sessionId) ||
		csr_is_conn_state_connected_infra(mac, sessionId) ||
		csr_is_conn_state_connected_wds(mac, sessionId);
}

bool csr_is_conn_state_infra(struct mac_context *mac, uint32_t sessionId)
{
	return csr_is_conn_state_connected_infra(mac, sessionId);
}

bool csr_is_conn_state_ibss(struct mac_context *mac, uint32_t sessionId)
{
	return csr_is_conn_state_connected_ibss(mac, sessionId) ||
	       csr_is_conn_state_disconnected_ibss(mac, sessionId);
}

bool csr_is_conn_state_connected_wds(struct mac_context *mac_ctx,
				     uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_WDS_CONNECTED);
}

bool csr_is_conn_state_connected_infra_ap(struct mac_context *mac_ctx,
					  uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED) ||
		csr_is_conn_state(mac_ctx, session_id,
				  eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTED);
}

bool csr_is_conn_state_disconnected_wds(struct mac_context *mac_ctx,
					uint32_t session_id)
{
	return csr_is_conn_state(mac_ctx, session_id,
				 eCSR_ASSOC_STATE_TYPE_WDS_DISCONNECTED);
}

bool csr_is_conn_state_wds(struct mac_context *mac, uint32_t sessionId)
{
	return csr_is_conn_state_connected_wds(mac, sessionId) ||
	       csr_is_conn_state_disconnected_wds(mac, sessionId);
}

static bool csr_is_conn_state_ap(struct mac_context *mac, uint32_t sessionId)
{
	struct csr_roam_session *pSession;

	pSession = CSR_GET_SESSION(mac, sessionId);
	if (!pSession)
		return false;
	if (CSR_IS_INFRA_AP(&pSession->connectedProfile))
		return true;
	return false;
}

bool csr_is_any_session_in_connect_state(struct mac_context *mac)
{
	uint32_t i;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (CSR_IS_SESSION_VALID(mac, i) &&
		    (csr_is_conn_state_infra(mac, i) ||
		     csr_is_conn_state_ibss(mac, i) ||
		     csr_is_conn_state_ap(mac, i))) {
			return true;
		}
	}

	return false;
}

uint8_t csr_get_infra_operation_channel(struct mac_context *mac, uint8_t sessionId)
{
	uint8_t channel;

	if (CSR_IS_SESSION_VALID(mac, sessionId)) {
		channel =
			mac->roam.roamSession[sessionId].connectedProfile.
			operationChannel;
	} else {
		channel = 0;
	}
	return channel;
}

bool csr_is_session_client_and_connected(struct mac_context *mac, uint8_t sessionId)
{
	struct csr_roam_session *pSession = NULL;

	if (CSR_IS_SESSION_VALID(mac, sessionId)
	    && csr_is_conn_state_infra(mac, sessionId)) {
		pSession = CSR_GET_SESSION(mac, sessionId);
		if (pSession->pCurRoamProfile) {
			if ((pSession->pCurRoamProfile->csrPersona ==
			     QDF_STA_MODE)
			    || (pSession->pCurRoamProfile->csrPersona ==
				QDF_P2P_CLIENT_MODE))
				return true;
		}
	}
	return false;
}

uint8_t csr_get_concurrent_operation_channel(struct mac_context *mac_ctx)
{
	struct csr_roam_session *session = NULL;
	uint8_t i = 0;
	enum QDF_OPMODE persona;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (!CSR_IS_SESSION_VALID(mac_ctx, i))
			continue;
		session = CSR_GET_SESSION(mac_ctx, i);
		if (!session->pCurRoamProfile)
			continue;
		persona = session->pCurRoamProfile->csrPersona;
		if ((((persona == QDF_STA_MODE) ||
			(persona == QDF_P2P_CLIENT_MODE)) &&
			(session->connectState ==
				eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED)) ||
			(((persona == QDF_P2P_GO_MODE) ||
				(persona == QDF_SAP_MODE))
				 && (session->connectState !=
					 eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED)))
			return session->connectedProfile.operationChannel;

	}
	return 0;
}

uint8_t csr_get_beaconing_concurrent_channel(struct mac_context *mac_ctx,
					     uint8_t vdev_id_to_skip)
{
	struct csr_roam_session *session = NULL;
	uint8_t i = 0;
	enum QDF_OPMODE persona;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (i == vdev_id_to_skip)
			continue;
		if (!CSR_IS_SESSION_VALID(mac_ctx, i))
			continue;
		session = CSR_GET_SESSION(mac_ctx, i);
		if (!session->pCurRoamProfile)
			continue;
		persona = session->pCurRoamProfile->csrPersona;
		if (((persona == QDF_P2P_GO_MODE) ||
		     (persona == QDF_SAP_MODE)) &&
		     (session->connectState !=
		      eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED))
			return session->connectedProfile.operationChannel;
	}

	return 0;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH

#define HALF_BW_OF(eCSR_bw_val) ((eCSR_bw_val)/2)

/* calculation of center channel based on V/HT BW and WIFI channel bw=5MHz) */

#define CSR_GET_HT40_PLUS_CCH(och) ((och)+2)
#define CSR_GET_HT40_MINUS_CCH(och) ((och)-2)

#define CSR_GET_HT80_PLUS_LL_CCH(och) ((och)+6)
#define CSR_GET_HT80_PLUS_HL_CCH(och) ((och)+2)
#define CSR_GET_HT80_MINUS_LH_CCH(och) ((och)-2)
#define CSR_GET_HT80_MINUS_HH_CCH(och) ((och)-6)

/**
 * csr_get_ch_from_ht_profile() - to get channel from HT profile
 * @mac: pointer to Mac context
 * @htp: pointer to HT profile
 * @och: operating channel
 * @cfreq: channel frequency
 * @hbw: half bandwidth
 *
 * This function will fill half bandwidth and channel frequency based
 * on the HT profile
 *
 * Return: none
 */
static void csr_get_ch_from_ht_profile(struct mac_context *mac,
				       tCsrRoamHTProfile *htp,
				       uint16_t och, uint16_t *cfreq,
				       uint16_t *hbw)
{
	uint16_t cch, ch_bond;

	if (och > 14)
		ch_bond = mac->roam.configParam.channelBondingMode5GHz;
	else
		ch_bond = mac->roam.configParam.channelBondingMode24GHz;

	cch = och;
	*hbw = HALF_BW_OF(eCSR_BW_20MHz_VAL);

	if (!ch_bond)
		goto ret;

	sme_debug("HTC: %d scbw: %d rcbw: %d sco: %d VHTC: %d apc: %d apbw: %d",
			htp->htCapability, htp->htSupportedChannelWidthSet,
			htp->htRecommendedTxWidthSet,
			htp->htSecondaryChannelOffset,
			htp->vhtCapability, htp->apCenterChan, htp->apChanWidth
	       );

	if (htp->vhtCapability) {
		cch = htp->apCenterChan;
		if (htp->apChanWidth == WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ)
			*hbw = HALF_BW_OF(eCSR_BW_80MHz_VAL);
		else if (htp->apChanWidth == WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ)
			*hbw = HALF_BW_OF(eCSR_BW_160MHz_VAL);

		if (!*hbw && htp->htCapability) {
			if (htp->htSupportedChannelWidthSet ==
				eHT_CHANNEL_WIDTH_40MHZ)
				*hbw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
			else
				*hbw = HALF_BW_OF(eCSR_BW_20MHz_VAL);
		}
	} else if (htp->htCapability) {
		if (htp->htSupportedChannelWidthSet ==
					eHT_CHANNEL_WIDTH_40MHZ) {
			*hbw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
			if (htp->htSecondaryChannelOffset ==
					PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
				cch = CSR_GET_HT40_PLUS_CCH(och);
			else if (htp->htSecondaryChannelOffset ==
					PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
				cch = CSR_GET_HT40_MINUS_CCH(och);
		} else {
			cch = och;
			*hbw = HALF_BW_OF(eCSR_BW_20MHz_VAL);
		}
	}

ret:
	*cfreq = cds_chan_to_freq(cch);
}

/**
 * csr_calc_chb_for_sap_phymode() - to calc channel bandwidth for sap phymode
 * @mac_ctx: pointer to mac context
 * @sap_ch: SAP operating channel
 * @sap_phymode: SAP physical mode
 * @sap_cch: concurrency channel
 * @sap_hbw: SAP half bw
 * @chb: channel bandwidth
 *
 * This routine is called to calculate channel bandwidth
 *
 * Return: none
 */
static void csr_calc_chb_for_sap_phymode(struct mac_context *mac_ctx,
		uint16_t *sap_ch, eCsrPhyMode *sap_phymode,
		uint16_t *sap_cch, uint16_t *sap_hbw, uint8_t *chb)
{
	if (*sap_phymode == eCSR_DOT11_MODE_11n ||
			*sap_phymode == eCSR_DOT11_MODE_11n_ONLY) {

		*sap_hbw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
		if (*chb == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
			*sap_cch = CSR_GET_HT40_PLUS_CCH(*sap_ch);
		else if (*chb == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
			*sap_cch = CSR_GET_HT40_MINUS_CCH(*sap_ch);

	} else if (*sap_phymode == eCSR_DOT11_MODE_11ac ||
		   *sap_phymode == eCSR_DOT11_MODE_11ac_ONLY ||
		   *sap_phymode == eCSR_DOT11_MODE_11ax ||
		   *sap_phymode == eCSR_DOT11_MODE_11ax_ONLY) {
		/*11AC only 80/40/20 Mhz supported in Rome */
		if (mac_ctx->roam.configParam.nVhtChannelWidth ==
				(WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ + 1)) {
			*sap_hbw = HALF_BW_OF(eCSR_BW_80MHz_VAL);
			if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW - 1))
				*sap_cch = CSR_GET_HT80_PLUS_LL_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW
				     - 1))
				*sap_cch = CSR_GET_HT80_PLUS_HL_CCH(*sap_ch);
			else if (*chb ==
				 (PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT80_MINUS_LH_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT80_MINUS_HH_CCH(*sap_ch);
		} else {
			*sap_hbw = HALF_BW_OF(eCSR_BW_40MHz_VAL);
			if (*chb == (PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW
					- 1))
				*sap_cch = CSR_GET_HT40_PLUS_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW
				     - 1))
				*sap_cch = CSR_GET_HT40_MINUS_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT40_PLUS_CCH(*sap_ch);
			else if (*chb ==
				(PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH
				     - 1))
				*sap_cch = CSR_GET_HT40_MINUS_CCH(*sap_ch);
		}
	}
}

/**
 * csr_handle_conc_chnl_overlap_for_sap_go - To handle overlap for AP+AP
 * @mac_ctx: pointer to mac context
 * @session: Current session
 * @sap_ch: SAP/GO operating channel
 * @sap_hbw: SAP/GO half bw
 * @sap_cfreq: SAP/GO channel frequency
 * @intf_ch: concurrent SAP/GO operating channel
 * @intf_hbw: concurrent SAP/GO half bw
 * @intf_cfreq: concurrent SAP/GO channel frequency
 *
 * This routine is called to check if one SAP/GO channel is overlapping with
 * other SAP/GO channel
 *
 * Return: none
 */
static void csr_handle_conc_chnl_overlap_for_sap_go(struct mac_context *mac_ctx,
		struct csr_roam_session *session,
		uint16_t *sap_ch, uint16_t *sap_hbw, uint16_t *sap_cfreq,
		uint16_t *intf_ch, uint16_t *intf_hbw, uint16_t *intf_cfreq)
{
	/*
	 * if conc_custom_rule1 is defined then we don't
	 * want p2pgo to follow SAP's channel or SAP to
	 * follow P2PGO's channel.
	 */
	if (0 == mac_ctx->roam.configParam.conc_custom_rule1 &&
		0 == mac_ctx->roam.configParam.conc_custom_rule2) {
		if (*sap_ch == 0) {
			*sap_ch = session->connectedProfile.operationChannel;
			csr_get_ch_from_ht_profile(mac_ctx,
				&session->connectedProfile.ht_profile,
				*sap_ch, sap_cfreq, sap_hbw);
		} else if (*sap_ch !=
				session->connectedProfile.operationChannel) {
			*intf_ch = session->connectedProfile.operationChannel;
			csr_get_ch_from_ht_profile(mac_ctx,
					&session->connectedProfile.ht_profile,
					*intf_ch, intf_cfreq, intf_hbw);
		}
	} else if (*sap_ch == 0 &&
			(session->pCurRoamProfile->csrPersona ==
					QDF_SAP_MODE)) {
		*sap_ch = session->connectedProfile.operationChannel;
		csr_get_ch_from_ht_profile(mac_ctx,
				&session->connectedProfile.ht_profile,
				*sap_ch, sap_cfreq, sap_hbw);
	}
}


/**
 * csr_check_concurrent_channel_overlap() - To check concurrent overlap chnls
 * @mac_ctx: Pointer to mac context
 * @sap_ch: SAP channel
 * @sap_phymode: SAP phy mode
 * @cc_switch_mode: concurrent switch mode
 *
 * This routine will be called to check concurrent overlap channels
 *
 * Return: uint16_t
 */
uint16_t csr_check_concurrent_channel_overlap(struct mac_context *mac_ctx,
			uint16_t sap_ch, eCsrPhyMode sap_phymode,
			uint8_t cc_switch_mode)
{
	struct csr_roam_session *session = NULL;
	uint8_t i = 0, chb = PHY_SINGLE_CHANNEL_CENTERED;
	uint16_t intf_ch = 0, sap_hbw = 0, intf_hbw = 0, intf_cfreq = 0;
	uint16_t sap_cfreq = 0;
	uint16_t sap_lfreq, sap_hfreq, intf_lfreq, intf_hfreq, sap_cch = 0;
	QDF_STATUS status;

	sme_debug("sap_ch: %d sap_phymode: %d", sap_ch, sap_phymode);

	if (mac_ctx->roam.configParam.cc_switch_mode ==
			QDF_MCC_TO_SCC_SWITCH_DISABLE)
		return 0;

	if (sap_ch != 0) {
		sap_cch = sap_ch;
		sap_hbw = HALF_BW_OF(eCSR_BW_20MHz_VAL);

		if (sap_ch > 14)
			chb = mac_ctx->roam.configParam.channelBondingMode5GHz;
		else
			chb = mac_ctx->roam.configParam.channelBondingMode24GHz;

		if (chb)
			csr_calc_chb_for_sap_phymode(mac_ctx, &sap_ch,
					&sap_phymode, &sap_cch, &sap_hbw, &chb);
		sap_cfreq = cds_chan_to_freq(sap_cch);
	}

	sme_debug("sap_ch:%d sap_phymode:%d sap_cch:%d sap_hbw:%d chb:%d",
		sap_ch, sap_phymode, sap_cch, sap_hbw, chb);

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (!CSR_IS_SESSION_VALID(mac_ctx, i))
			continue;

		session = CSR_GET_SESSION(mac_ctx, i);
		if (!session->pCurRoamProfile)
			continue;
		if (((session->pCurRoamProfile->csrPersona == QDF_STA_MODE) ||
			(session->pCurRoamProfile->csrPersona ==
				QDF_P2P_CLIENT_MODE)) &&
			(session->connectState ==
				eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED)) {
			intf_ch = session->connectedProfile.operationChannel;
			csr_get_ch_from_ht_profile(mac_ctx,
				&session->connectedProfile.ht_profile,
				intf_ch, &intf_cfreq, &intf_hbw);
			sme_debug("%d: intf_ch:%d intf_cfreq:%d intf_hbw:%d",
				i, intf_ch, intf_cfreq, intf_hbw);
		} else if (((session->pCurRoamProfile->csrPersona ==
					QDF_P2P_GO_MODE) ||
				(session->pCurRoamProfile->csrPersona ==
					QDF_SAP_MODE)) &&
				(session->connectState !=
					eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED)) {
			if (session->ch_switch_in_progress)
				continue;

			csr_handle_conc_chnl_overlap_for_sap_go(mac_ctx,
					session, &sap_ch, &sap_hbw, &sap_cfreq,
					&intf_ch, &intf_hbw, &intf_cfreq);

			sme_debug("%d: sap_ch:%d sap_hbw:%d sap_cfreq:%d intf_ch:%d intf_hbw:%d, intf_cfreq:%d",
					i, sap_ch, sap_hbw, sap_cfreq,
					intf_ch, intf_hbw, intf_cfreq);
		}
		if (intf_ch && ((intf_ch > 14 && sap_ch > 14) ||
				(intf_ch <= 14 && sap_ch <= 14)))
			break;
	}

	sme_debug("intf_ch:%d sap_ch:%d cc_switch_mode:%d, dbs:%d",
			intf_ch, sap_ch, cc_switch_mode,
			policy_mgr_is_dbs_enable(mac_ctx->psoc));

	if (intf_ch && sap_ch != intf_ch &&
	    !policy_mgr_is_force_scc(mac_ctx->psoc)) {
		sap_lfreq = sap_cfreq - sap_hbw;
		sap_hfreq = sap_cfreq + sap_hbw;
		intf_lfreq = intf_cfreq - intf_hbw;
		intf_hfreq = intf_cfreq + intf_hbw;

		sme_err("SAP:  OCH: %03d OCF: %d CCH: %03d CF: %d BW: %d LF: %d HF: %d INTF: OCH: %03d OCF: %d CCH: %03d CF: %d BW: %d LF: %d HF: %d",
			sap_ch, cds_chan_to_freq(sap_ch),
			cds_freq_to_chan(sap_cfreq), sap_cfreq, sap_hbw * 2,
			sap_lfreq, sap_hfreq, intf_ch,
			cds_chan_to_freq(intf_ch), cds_freq_to_chan(intf_cfreq),
			intf_cfreq, intf_hbw * 2, intf_lfreq, intf_hfreq);

		if (!(((sap_lfreq > intf_lfreq && sap_lfreq < intf_hfreq) ||
			(sap_hfreq > intf_lfreq && sap_hfreq < intf_hfreq)) ||
			((intf_lfreq > sap_lfreq && intf_lfreq < sap_hfreq) ||
			(intf_hfreq > sap_lfreq && intf_hfreq < sap_hfreq))))
			intf_ch = 0;
	} else if (intf_ch && sap_ch != intf_ch &&
		  (policy_mgr_is_force_scc(mac_ctx->psoc))) {
		if (!((intf_ch <= 14 && sap_ch <= 14) ||
			(intf_ch > 14 && sap_ch > 14))) {
			if (policy_mgr_is_dbs_enable(mac_ctx->psoc) ||
			    cc_switch_mode ==
			    QDF_MCC_TO_SCC_WITH_PREFERRED_BAND)
				intf_ch = 0;
		} else if (cc_switch_mode ==
			QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL) {
			status =
				policy_mgr_get_sap_mandatory_channel(
				mac_ctx->psoc,
				(uint32_t *)&intf_ch);
			if (QDF_IS_STATUS_ERROR(status))
				sme_err("no mandatory channel");
		}
	} else if ((intf_ch == sap_ch) && (cc_switch_mode ==
				QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL)) {
		if (cds_chan_to_band(intf_ch) == CDS_BAND_2GHZ) {
			status =
				policy_mgr_get_sap_mandatory_channel(
					mac_ctx->psoc, (uint32_t *)&intf_ch);
			if (QDF_IS_STATUS_ERROR(status))
				sme_err("no mandatory channel");
		}
	}

	if (intf_ch == sap_ch)
		intf_ch = 0;

	sme_err("##Concurrent Channels %s Interfering",
		intf_ch == 0 ? "Not" : "Are");
	return intf_ch;
}
#endif

bool csr_is_all_session_disconnected(struct mac_context *mac)
{
	uint32_t i;
	bool fRc = true;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (CSR_IS_SESSION_VALID(mac, i)
		    && !csr_is_conn_state_disconnected(mac, i)) {
			fRc = false;
			break;
		}
	}

	return fRc;
}

uint8_t csr_get_connected_infra(struct mac_context *mac_ctx)
{
	uint32_t i;
	uint8_t connected_session = WLAN_UMAC_VDEV_ID_MAX;

	for (i = 0; i < WLAN_MAX_VDEVS; i++) {
		if (CSR_IS_SESSION_VALID(mac_ctx, i)
		    && csr_is_conn_state_connected_infra(mac_ctx, i)) {
			connected_session = i;
			break;
		}
	}

	return connected_session;
}


bool csr_is_concurrent_session_running(struct mac_context *mac)
{
	uint32_t sessionId, noOfCocurrentSession = 0;
	eCsrConnectState connectState;

	bool fRc = false;

	for (sessionId = 0; sessionId < WLAN_MAX_VDEVS; sessionId++) {
		if (CSR_IS_SESSION_VALID(mac, sessionId)) {
			connectState =
				mac->roam.roamSession[sessionId].connectState;
			if ((eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED ==
			     connectState)
			    || (eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED ==
				connectState)
			    || (eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTED ==
				connectState)) {
				++noOfCocurrentSession;
			}
		}
	}

	/* More than one session is Up and Running */
	if (noOfCocurrentSession > 1)
		fRc = true;
	return fRc;
}

bool csr_is_infra_ap_started(struct mac_context *mac)
{
	uint32_t sessionId;
	bool fRc = false;

	for (sessionId = 0; sessionId < WLAN_MAX_VDEVS; sessionId++) {
		if (CSR_IS_SESSION_VALID(mac, sessionId) &&
				(csr_is_conn_state_connected_infra_ap(mac,
					sessionId))) {
			fRc = true;
			break;
		}
	}

	return fRc;

}

bool csr_is_conn_state_disconnected(struct mac_context *mac, uint32_t sessionId)
{
	return eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED ==
	       mac->roam.roamSession[sessionId].connectState;
}

/**
 * csr_is_valid_mc_concurrent_session() - To check concurren session is valid
 * @mac_ctx: pointer to mac context
 * @session_id: session id
 * @bss_descr: bss description
 *
 * This function validates the concurrent session
 *
 * Return: true or false
 */
bool csr_is_valid_mc_concurrent_session(struct mac_context *mac_ctx,
		uint32_t session_id,
		tSirBssDescription *bss_descr)
{
	struct csr_roam_session *pSession = NULL;

	/* Check for MCC support */
	if (!mac_ctx->roam.configParam.fenableMCCMode)
		return false;
	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id))
		return false;
	/* Validate BeaconInterval */
	pSession = CSR_GET_SESSION(mac_ctx, session_id);
	if (!pSession->pCurRoamProfile)
		return false;
	if (QDF_STATUS_SUCCESS == csr_validate_mcc_beacon_interval(mac_ctx,
					bss_descr->channelId,
					&bss_descr->beaconInterval, session_id,
					pSession->pCurRoamProfile->csrPersona))
		return true;
	return false;
}

static tSirMacCapabilityInfo csr_get_bss_capabilities(tSirBssDescription *
						      pSirBssDesc)
{
	tSirMacCapabilityInfo dot11Caps;

	/* tSirMacCapabilityInfo is 16-bit */
	qdf_get_u16((uint8_t *) &pSirBssDesc->capabilityInfo,
		    (uint16_t *) &dot11Caps);

	return dot11Caps;
}

bool csr_is_infra_bss_desc(tSirBssDescription *pSirBssDesc)
{
	tSirMacCapabilityInfo dot11Caps = csr_get_bss_capabilities(pSirBssDesc);

	return (bool) dot11Caps.ess;
}

bool csr_is_ibss_bss_desc(tSirBssDescription *pSirBssDesc)
{
	tSirMacCapabilityInfo dot11Caps = csr_get_bss_capabilities(pSirBssDesc);

	return (bool) dot11Caps.ibss;
}

static bool csr_is_qos_bss_desc(tSirBssDescription *pSirBssDesc)
{
	tSirMacCapabilityInfo dot11Caps = csr_get_bss_capabilities(pSirBssDesc);

	return (bool) dot11Caps.qos;
}

bool csr_is_privacy(tSirBssDescription *pSirBssDesc)
{
	tSirMacCapabilityInfo dot11Caps = csr_get_bss_capabilities(pSirBssDesc);

	return (bool) dot11Caps.privacy;
}

bool csr_is11h_supported(struct mac_context *mac)
{
	return mac->mlme_cfg->gen.enabled_11h;
}

bool csr_is11e_supported(struct mac_context *mac)
{
	return mac->roam.configParam.Is11eSupportEnabled;
}

bool csr_is_wmm_supported(struct mac_context *mac)
{
	if (eCsrRoamWmmNoQos == mac->roam.configParam.WMMSupportMode)
		return false;
	else
		return true;
}

/* pIes is the IEs for pSirBssDesc2 */
bool csr_is_ssid_equal(struct mac_context *mac,
		       tSirBssDescription *pSirBssDesc1,
		       tSirBssDescription *pSirBssDesc2,
		       tDot11fBeaconIEs *pIes2)
{
	bool fEqual = false;
	tSirMacSSid Ssid1, Ssid2;
	tDot11fBeaconIEs *pIes1 = NULL;
	tDot11fBeaconIEs *pIesLocal = pIes2;

	do {
		if ((!pSirBssDesc1) || (!pSirBssDesc2))
			break;
		if (!pIesLocal
		    &&
		    !QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies
						   (mac, pSirBssDesc2,
						    &pIesLocal))) {
			sme_err("fail to parse IEs");
			break;
		}
		if (!QDF_IS_STATUS_SUCCESS
			(csr_get_parsed_bss_description_ies(mac,
				pSirBssDesc1, &pIes1))) {
			break;
		}
		if ((!pIes1->SSID.present) || (!pIesLocal->SSID.present))
			break;
		if (pIes1->SSID.num_ssid != pIesLocal->SSID.num_ssid)
			break;
		qdf_mem_copy(Ssid1.ssId, pIes1->SSID.ssid,
			     pIes1->SSID.num_ssid);
		qdf_mem_copy(Ssid2.ssId, pIesLocal->SSID.ssid,
			     pIesLocal->SSID.num_ssid);

		fEqual = (!qdf_mem_cmp(Ssid1.ssId, Ssid2.ssId,
					pIesLocal->SSID.num_ssid));

	} while (0);
	if (pIes1)
		qdf_mem_free(pIes1);
	if (pIesLocal && !pIes2)
		qdf_mem_free(pIesLocal);

	return fEqual;
}

/* pIes can be passed in as NULL if the caller doesn't have one prepared */
static bool csr_is_bss_description_wme(struct mac_context *mac,
				       tSirBssDescription *pSirBssDesc,
				       tDot11fBeaconIEs *pIes)
{
	/* Assume that WME is found... */
	bool fWme = true;
	tDot11fBeaconIEs *pIesTemp = pIes;

	do {
		if (!pIesTemp) {
			if (!QDF_IS_STATUS_SUCCESS
				    (csr_get_parsed_bss_description_ies
					    (mac, pSirBssDesc, &pIesTemp))) {
				fWme = false;
				break;
			}
		}
		/* if the Wme Info IE is found, then WME is supported... */
		if (CSR_IS_QOS_BSS(pIesTemp))
			break;
		/* if none of these are found, then WME is NOT supported... */
		fWme = false;
	} while (0);
	if (!csr_is_wmm_supported(mac) && fWme)
		if (!pIesTemp->HTCaps.present)
			fWme = false;

	if ((!pIes) && (pIesTemp))
		/* we allocate memory here so free it before returning */
		qdf_mem_free(pIesTemp);

	return fWme;
}

eCsrMediaAccessType csr_get_qos_from_bss_desc(struct mac_context *mac_ctx,
					      tSirBssDescription *pSirBssDesc,
					      tDot11fBeaconIEs *pIes)
{
	eCsrMediaAccessType qosType = eCSR_MEDIUM_ACCESS_DCF;

	if (!pIes) {
		QDF_ASSERT(pIes);
		return qosType;
	}

	do {
		/* If we find WMM in the Bss Description, then we let this
		 * override and use WMM.
		 */
		if (csr_is_bss_description_wme(mac_ctx, pSirBssDesc, pIes))
			qosType = eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP;
		else {
			/* If the QoS bit is on, then the AP is
			 * advertising 11E QoS.
			 */
			if (csr_is_qos_bss_desc(pSirBssDesc))
				qosType = eCSR_MEDIUM_ACCESS_11e_eDCF;
			else
				qosType = eCSR_MEDIUM_ACCESS_DCF;

			/* Scale back based on the types turned on
			 * for the adapter.
			 */
			if (eCSR_MEDIUM_ACCESS_11e_eDCF == qosType
			    && !csr_is11e_supported(mac_ctx))
				qosType = eCSR_MEDIUM_ACCESS_DCF;
		}

	} while (0);

	return qosType;
}

/* Caller allocates memory for pIEStruct */
QDF_STATUS csr_parse_bss_description_ies(struct mac_context *mac_ctx,
					 tSirBssDescription *pBssDesc,
					 tDot11fBeaconIEs *pIEStruct)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int ieLen =
		(int)(pBssDesc->length + sizeof(pBssDesc->length) -
		      GET_FIELD_OFFSET(tSirBssDescription, ieFields));

	if (ieLen > 0 && pIEStruct) {
		if (!DOT11F_FAILED(dot11f_unpack_beacon_i_es
				    (mac_ctx, (uint8_t *) pBssDesc->ieFields,
				    ieLen, pIEStruct, false)))
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

/* This function will allocate memory for the parsed IEs to the caller.
 * Caller must free the memory after it is done with the data only if
 * this function succeeds
 */
QDF_STATUS csr_get_parsed_bss_description_ies(struct mac_context *mac_ctx,
					      tSirBssDescription *pBssDesc,
					      tDot11fBeaconIEs **ppIEStruct)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (pBssDesc && ppIEStruct) {
		*ppIEStruct = qdf_mem_malloc(sizeof(tDot11fBeaconIEs));
		if ((*ppIEStruct) != NULL) {
			status = csr_parse_bss_description_ies(mac_ctx,
							       pBssDesc,
							       *ppIEStruct);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				qdf_mem_free(*ppIEStruct);
				*ppIEStruct = NULL;
			}
		} else {
			sme_err("failed to allocate memory");
			QDF_ASSERT(0);
			return QDF_STATUS_E_NOMEM;
		}
	}

	return status;
}

bool csr_is_nullssid(uint8_t *pBssSsid, uint8_t len)
{
	bool fNullSsid = false;

	uint32_t SsidLength;
	uint8_t *pSsidStr;

	do {
		if (0 == len) {
			fNullSsid = true;
			break;
		}
		/* Consider 0 or space for hidden SSID */
		if (0 == pBssSsid[0]) {
			fNullSsid = true;
			break;
		}

		SsidLength = len;
		pSsidStr = pBssSsid;

		while (SsidLength) {
			if (*pSsidStr)
				break;

			pSsidStr++;
			SsidLength--;
		}

		if (0 == SsidLength) {
			fNullSsid = true;
			break;
		}
	} while (0);

	return fNullSsid;
}

uint32_t csr_get_frag_thresh(struct mac_context *mac_ctx)
{
	return mac_ctx->mlme_cfg->threshold.frag_threshold;
}

uint32_t csr_get_rts_thresh(struct mac_context *mac_ctx)
{
	return mac_ctx->mlme_cfg->threshold.rts_threshold;
}

static eCsrPhyMode
csr_translate_to_phy_mode_from_bss_desc(struct mac_context *mac_ctx,
					tSirBssDescription *pSirBssDesc,
					tDot11fBeaconIEs *ies)
{
	eCsrPhyMode phyMode;
	uint8_t i;

	switch (pSirBssDesc->nwType) {
	case eSIR_11A_NW_TYPE:
		phyMode = eCSR_DOT11_MODE_11a;
		break;

	case eSIR_11B_NW_TYPE:
		phyMode = eCSR_DOT11_MODE_11b;
		break;

	case eSIR_11G_NW_TYPE:
		phyMode = eCSR_DOT11_MODE_11g_ONLY;

		/* Check if the BSS is in b/g mixed mode or g_only mode */
		if (!ies || !ies->SuppRates.present) {
			sme_debug("Unable to get rates, assume G only mode");
			break;
		}

		for (i = 0; i < ies->SuppRates.num_rates; i++) {
			if (csr_rates_is_dot11_rate11b_supported_rate(
			    ies->SuppRates.rates[i])) {
				sme_debug("One B rate is supported");
				phyMode = eCSR_DOT11_MODE_11g;
				break;
			}
		}
		break;
	case eSIR_11N_NW_TYPE:
		phyMode = eCSR_DOT11_MODE_11n;
		break;
	case eSIR_11AX_NW_TYPE:
		phyMode = eCSR_DOT11_MODE_11ax;
		break;
	case eSIR_11AC_NW_TYPE:
	default:
		phyMode = eCSR_DOT11_MODE_11ac;
		break;
	}
	return phyMode;
}

uint32_t csr_translate_to_wni_cfg_dot11_mode(struct mac_context *mac,
					     enum csr_cfgdot11mode csrDot11Mode)
{
	uint32_t ret;

	switch (csrDot11Mode) {
	case eCSR_CFG_DOT11_MODE_AUTO:
		sme_debug("eCSR_CFG_DOT11_MODE_AUTO");
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11A:
		ret = MLME_DOT11_MODE_11A;
		break;
	case eCSR_CFG_DOT11_MODE_11B:
		ret = MLME_DOT11_MODE_11B;
		break;
	case eCSR_CFG_DOT11_MODE_11G:
		ret = MLME_DOT11_MODE_11G;
		break;
	case eCSR_CFG_DOT11_MODE_11N:
		ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11G_ONLY:
		ret = MLME_DOT11_MODE_11G_ONLY;
		break;
	case eCSR_CFG_DOT11_MODE_11N_ONLY:
		ret = MLME_DOT11_MODE_11N_ONLY;
		break;
	case eCSR_CFG_DOT11_MODE_11AC_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC_ONLY;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AC:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AX_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	case eCSR_CFG_DOT11_MODE_11AX:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			ret = MLME_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			ret = MLME_DOT11_MODE_11AC;
		else
			ret = MLME_DOT11_MODE_11N;
		break;
	default:
		sme_warn("doesn't expect %d as csrDo11Mode", csrDot11Mode);
		if (BAND_2G == mac->mlme_cfg->gen.band)
			ret = MLME_DOT11_MODE_11G;
		else
			ret = MLME_DOT11_MODE_11A;
		break;
	}

	return ret;
}

/**
 * csr_get_phy_mode_from_bss() - Get Phy Mode
 * @mac:           Global MAC context
 * @pBSSDescription: BSS Descriptor
 * @pPhyMode:        Physical Mode
 * @pIes:            Pointer to the IE fields
 *
 * This function should only return the super set of supported modes
 * 11n implies 11b/g/a/n.
 *
 * Return: success
 **/
QDF_STATUS csr_get_phy_mode_from_bss(struct mac_context *mac,
		tSirBssDescription *pBSSDescription,
		eCsrPhyMode *pPhyMode, tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	eCsrPhyMode phyMode =
		csr_translate_to_phy_mode_from_bss_desc(mac, pBSSDescription,
							pIes);

	if (pIes) {
		if (pIes->HTCaps.present) {
			phyMode = eCSR_DOT11_MODE_11n;
			if (IS_BSS_VHT_CAPABLE(pIes->VHTCaps) ||
				IS_BSS_VHT_CAPABLE(pIes->vendor_vht_ie.VHTCaps))
				phyMode = eCSR_DOT11_MODE_11ac;
			if (pIes->he_cap.present)
				phyMode = eCSR_DOT11_MODE_11ax;
		}
		*pPhyMode = phyMode;
	}

	return status;
}

/**
 * csr_get_phy_mode_in_use() - to get phymode
 * @phyModeIn: physical mode
 * @bssPhyMode: physical mode in bss
 * @f5GhzBand: 5Ghz band
 * @pCfgDot11ModeToUse: dot11 mode in use
 *
 * This function returns the correct eCSR_CFG_DOT11_MODE is the two phyModes
 * matches. bssPhyMode is the mode derived from the BSS description
 * f5GhzBand is derived from the channel id of BSS description
 *
 * Return: true or false
 */
static bool csr_get_phy_mode_in_use(struct mac_context *mac_ctx,
				    eCsrPhyMode phyModeIn,
				    eCsrPhyMode bssPhyMode,
				    bool f5GhzBand,
				    enum csr_cfgdot11mode *pCfgDot11ModeToUse)
{
	bool fMatch = false;
	enum csr_cfgdot11mode cfgDot11Mode;

	cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
	switch (phyModeIn) {
	/* 11a or 11b or 11g */
	case eCSR_DOT11_MODE_abg:
		fMatch = true;
		if (f5GhzBand)
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
		else if (eCSR_DOT11_MODE_11b == bssPhyMode)
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
		break;

	case eCSR_DOT11_MODE_11a:
		if (f5GhzBand) {
			fMatch = true;
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
		}
		break;

	case eCSR_DOT11_MODE_11g:
		if (!f5GhzBand) {
			fMatch = true;
			if (eCSR_DOT11_MODE_11b == bssPhyMode)
				cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
			else
				cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
		}
		break;

	case eCSR_DOT11_MODE_11g_ONLY:
		if ((bssPhyMode == eCSR_DOT11_MODE_11g) ||
		    (bssPhyMode == eCSR_DOT11_MODE_11g_ONLY)) {
			fMatch = true;
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
		}
		break;

	case eCSR_DOT11_MODE_11b:
	case eCSR_DOT11_MODE_11b_ONLY:
		if (!f5GhzBand && (bssPhyMode != eCSR_DOT11_MODE_11g_ONLY)) {
			fMatch = true;
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
		}
		break;

	case eCSR_DOT11_MODE_11n:
		fMatch = true;
		switch (bssPhyMode) {
		case eCSR_DOT11_MODE_11g:
		case eCSR_DOT11_MODE_11g_ONLY:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
			break;
		case eCSR_DOT11_MODE_11b:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
			break;
		case eCSR_DOT11_MODE_11a:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
			break;
		case eCSR_DOT11_MODE_11n:
		case eCSR_DOT11_MODE_11ac:
		case eCSR_DOT11_MODE_11ax:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
			break;

		default:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
			break;
		}
		break;

	case eCSR_DOT11_MODE_11n_ONLY:
		if (eCSR_DOT11_MODE_11n == bssPhyMode) {
			fMatch = true;
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;

		}

		break;
	case eCSR_DOT11_MODE_11ac:
		fMatch = true;
		switch (bssPhyMode) {
		case eCSR_DOT11_MODE_11g:
		case eCSR_DOT11_MODE_11g_ONLY:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
			break;
		case eCSR_DOT11_MODE_11b:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
			break;
		case eCSR_DOT11_MODE_11a:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
			break;
		case eCSR_DOT11_MODE_11n:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
			break;
		case eCSR_DOT11_MODE_11ac:
		case eCSR_DOT11_MODE_11ax:
		default:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
			break;
		}
		break;

	case eCSR_DOT11_MODE_11ac_ONLY:
		if (eCSR_DOT11_MODE_11ac == bssPhyMode) {
			fMatch = true;
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		}
		break;
	case eCSR_DOT11_MODE_11ax:
		fMatch = true;
		switch (bssPhyMode) {
		case eCSR_DOT11_MODE_11g:
		case eCSR_DOT11_MODE_11g_ONLY:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
			break;
		case eCSR_DOT11_MODE_11b:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
			break;
		case eCSR_DOT11_MODE_11a:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
			break;
		case eCSR_DOT11_MODE_11n:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
			break;
		case eCSR_DOT11_MODE_11ac:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
			break;
		case eCSR_DOT11_MODE_11ax:
		default:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX;
			break;
		}
		break;

	case eCSR_DOT11_MODE_11ax_ONLY:
		if (eCSR_DOT11_MODE_11ax == bssPhyMode) {
			fMatch = true;
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX;
		}
		break;

	default:
		fMatch = true;
		switch (bssPhyMode) {
		case eCSR_DOT11_MODE_11g:
		case eCSR_DOT11_MODE_11g_ONLY:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
			break;
		case eCSR_DOT11_MODE_11b:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
			break;
		case eCSR_DOT11_MODE_11a:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
			break;
		case eCSR_DOT11_MODE_11n:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
			break;
		case eCSR_DOT11_MODE_11ac:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
			break;
		case eCSR_DOT11_MODE_11ax:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX;
			break;
		default:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_AUTO;
			break;
		}
		break;
	}

	if (fMatch && pCfgDot11ModeToUse) {
		if (cfgDot11Mode == eCSR_CFG_DOT11_MODE_11AX) {
			if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
				*pCfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11AX;
			else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
				*pCfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11AC;
			else
				*pCfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11N;
		} else {
			if (cfgDot11Mode == eCSR_CFG_DOT11_MODE_11AC
			    && (!IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)))
				*pCfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11N;
			else
				*pCfgDot11ModeToUse = cfgDot11Mode;
		}
	}
	return fMatch;
}

/**
 * csr_is_phy_mode_match() - to find if phy mode matches
 * @mac: pointer to mac context
 * @phyMode: physical mode
 * @pSirBssDesc: bss description
 * @pProfile: pointer to roam profile
 * @pReturnCfgDot11Mode: dot1 mode to return
 * @pIes: pointer to IEs
 *
 * This function decides whether the one of the bit of phyMode is matching the
 * mode in the BSS and allowed by the user setting
 *
 * Return: true or false based on mode that fits the criteria
 */
bool csr_is_phy_mode_match(struct mac_context *mac, uint32_t phyMode,
			   tSirBssDescription *pSirBssDesc,
			   struct csr_roam_profile *pProfile,
			   enum csr_cfgdot11mode *pReturnCfgDot11Mode,
			   tDot11fBeaconIEs *pIes)
{
	bool fMatch = false;
	eCsrPhyMode phyModeInBssDesc = eCSR_DOT11_MODE_AUTO, phyMode2;
	enum csr_cfgdot11mode cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_AUTO;
	uint32_t bitMask, loopCount;

	if (!QDF_IS_STATUS_SUCCESS(csr_get_phy_mode_from_bss(mac, pSirBssDesc,
					&phyModeInBssDesc, pIes)))
		return fMatch;

	if ((0 == phyMode) || (eCSR_DOT11_MODE_AUTO & phyMode)) {
		if (eCSR_CFG_DOT11_MODE_ABG ==
				mac->roam.configParam.uCfgDot11Mode) {
			phyMode = eCSR_DOT11_MODE_abg;
		} else if (eCSR_CFG_DOT11_MODE_AUTO ==
				mac->roam.configParam.uCfgDot11Mode) {
			if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
				phyMode = eCSR_DOT11_MODE_11ax;
			else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
				phyMode = eCSR_DOT11_MODE_11ac;
			else
				phyMode = eCSR_DOT11_MODE_11n;
		} else {
			/* user's pick */
			phyMode = mac->roam.configParam.phyMode;
		}
	}

	if ((0 == phyMode) || (eCSR_DOT11_MODE_AUTO & phyMode)) {
		if (0 != phyMode) {
			if (eCSR_DOT11_MODE_AUTO & phyMode) {
				phyMode2 =
					eCSR_DOT11_MODE_AUTO & phyMode;
			}
		} else {
			phyMode2 = phyMode;
		}
		fMatch = csr_get_phy_mode_in_use(mac, phyMode2,
						 phyModeInBssDesc,
						 WLAN_REG_IS_5GHZ_CH(
						 pSirBssDesc->channelId),
						 &cfgDot11ModeToUse);
	} else {
		bitMask = 1;
		loopCount = 0;
		while (loopCount < eCSR_NUM_PHY_MODE) {
			phyMode2 = (phyMode & (bitMask << loopCount++));
			if (0 != phyMode2 &&
			    csr_get_phy_mode_in_use(mac, phyMode2,
			    phyModeInBssDesc,
			    WLAN_REG_IS_5GHZ_CH(pSirBssDesc->channelId),
			    &cfgDot11ModeToUse)) {
				fMatch = true;
				break;
			}
		}
	}
	if (fMatch && pReturnCfgDot11Mode) {
		if (pProfile) {
			/*
			 * IEEE 11n spec (8.4.3): HT STA shall
			 * eliminate TKIP as a choice for the pairwise
			 * cipher suite if CCMP is advertised by the AP
			 * or if the AP included an HT capabilities
			 * element in its Beacons and Probe Response.
			 */
			if ((!CSR_IS_11n_ALLOWED(
					pProfile->negotiatedUCEncryptionType))
					&& ((eCSR_CFG_DOT11_MODE_11N ==
						cfgDot11ModeToUse) ||
					(eCSR_CFG_DOT11_MODE_11AC ==
						cfgDot11ModeToUse) ||
					(eCSR_CFG_DOT11_MODE_11AX ==
						cfgDot11ModeToUse))) {
				/* We cannot do 11n here */
				if (!WLAN_REG_IS_5GHZ_CH
						(pSirBssDesc->channelId)) {
					cfgDot11ModeToUse =
						eCSR_CFG_DOT11_MODE_11G;
				} else {
					cfgDot11ModeToUse =
						eCSR_CFG_DOT11_MODE_11A;
				}
			}
		}
		*pReturnCfgDot11Mode = cfgDot11ModeToUse;
	}

	return fMatch;
}

enum csr_cfgdot11mode csr_find_best_phy_mode(struct mac_context *mac,
			uint32_t phyMode)
{
	enum csr_cfgdot11mode cfgDot11ModeToUse;
	enum band_info band = mac->mlme_cfg->gen.band;

	if ((0 == phyMode) ||
	    (eCSR_DOT11_MODE_AUTO & phyMode) ||
	    (eCSR_DOT11_MODE_11ax & phyMode)) {
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX)) {
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11AX;
		} else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11AC;
		} else {
			/* Default to 11N mode if user has configured 11ac mode
			 * and FW doesn't supports 11ac mode .
			 */
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11N;
		}
	} else if (eCSR_DOT11_MODE_11ac & phyMode) {
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11AC;
		} else {
			/* Default to 11N mode if user has configured 11ac mode
			 * and FW doesn't supports 11ac mode .
			 */
		}	cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11N;
	} else {
		if ((eCSR_DOT11_MODE_11n | eCSR_DOT11_MODE_11n_ONLY) & phyMode)
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11N;
		else if (eCSR_DOT11_MODE_abg & phyMode) {
			if (BAND_2G != band)
				cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11A;
			else
				cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11G;
		} else if (eCSR_DOT11_MODE_11a & phyMode)
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11A;
		else if ((eCSR_DOT11_MODE_11g | eCSR_DOT11_MODE_11g_ONLY) &
			   phyMode)
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11G;
		else
			cfgDot11ModeToUse = eCSR_CFG_DOT11_MODE_11B;
	}

	return cfgDot11ModeToUse;
}

uint32_t csr_get11h_power_constraint(struct mac_context *mac_ctx,
				     tDot11fIEPowerConstraints *constraints)
{
	uint32_t localPowerConstraint = 0;

	/* check if .11h support is enabled, if not,
	 * the power constraint is 0.
	 */
	if (mac_ctx->mlme_cfg->gen.enabled_11h &&
	    constraints->present)
		localPowerConstraint = constraints->localPowerConstraints;

	return localPowerConstraint;
}

bool csr_is_profile_wpa(struct csr_roam_profile *pProfile)
{
	bool fWpaProfile = false;

	switch (pProfile->negotiatedAuthType) {
	case eCSR_AUTH_TYPE_WPA:
	case eCSR_AUTH_TYPE_WPA_PSK:
	case eCSR_AUTH_TYPE_WPA_NONE:
#ifdef FEATURE_WLAN_ESE
	case eCSR_AUTH_TYPE_CCKM_WPA:
#endif
		fWpaProfile = true;
		break;

	default:
		fWpaProfile = false;
		break;
	}

	if (fWpaProfile) {
		switch (pProfile->negotiatedUCEncryptionType) {
		case eCSR_ENCRYPT_TYPE_WEP40:
		case eCSR_ENCRYPT_TYPE_WEP104:
		case eCSR_ENCRYPT_TYPE_TKIP:
		case eCSR_ENCRYPT_TYPE_AES:
			fWpaProfile = true;
			break;

		default:
			fWpaProfile = false;
			break;
		}
	}
	return fWpaProfile;
}

bool csr_is_profile_rsn(struct csr_roam_profile *pProfile)
{
	bool fRSNProfile = false;

	switch (pProfile->negotiatedAuthType) {
	case eCSR_AUTH_TYPE_RSN:
	case eCSR_AUTH_TYPE_RSN_PSK:
	case eCSR_AUTH_TYPE_FT_RSN:
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
#ifdef FEATURE_WLAN_ESE
	case eCSR_AUTH_TYPE_CCKM_RSN:
#endif
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
#endif
	/* fallthrough */
	case eCSR_AUTH_TYPE_FILS_SHA256:
	case eCSR_AUTH_TYPE_FILS_SHA384:
	case eCSR_AUTH_TYPE_FT_FILS_SHA256:
	case eCSR_AUTH_TYPE_FT_FILS_SHA384:
	case eCSR_AUTH_TYPE_DPP_RSN:
	case eCSR_AUTH_TYPE_OSEN:
		fRSNProfile = true;
		break;

	case eCSR_AUTH_TYPE_OWE:
	case eCSR_AUTH_TYPE_SUITEB_EAP_SHA256:
	case eCSR_AUTH_TYPE_SUITEB_EAP_SHA384:
	case eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384:
		fRSNProfile = true;
		break;
	case eCSR_AUTH_TYPE_SAE:
	case eCSR_AUTH_TYPE_FT_SAE:
		fRSNProfile = true;
		break;

	default:
		fRSNProfile = false;
		break;
	}

	if (fRSNProfile) {
		switch (pProfile->negotiatedUCEncryptionType) {
		/* !!REVIEW - For WPA2, use of RSN IE mandates */
		/* use of AES as encryption. Here, we qualify */
		/* even if encryption type is WEP or TKIP */
		case eCSR_ENCRYPT_TYPE_WEP40:
		case eCSR_ENCRYPT_TYPE_WEP104:
		case eCSR_ENCRYPT_TYPE_TKIP:
		case eCSR_ENCRYPT_TYPE_AES:
		case eCSR_ENCRYPT_TYPE_AES_GCMP:
		case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
			fRSNProfile = true;
			break;

		default:
			fRSNProfile = false;
			break;
		}
	}
	return fRSNProfile;
}

/**
 * csr_update_mcc_p2p_beacon_interval() - update p2p beacon interval
 * @mac_ctx: pointer to mac context
 *
 * This function is to update the mcc p2p beacon interval
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS csr_update_mcc_p2p_beacon_interval(struct mac_context *mac_ctx)
{
	uint32_t session_id = 0;
	struct csr_roam_session *roam_session;

	/* If MCC is not supported just break and return SUCCESS */
	if (!mac_ctx->roam.configParam.fenableMCCMode)
		return QDF_STATUS_E_FAILURE;

	for (session_id = 0; session_id < WLAN_MAX_VDEVS; session_id++) {
		/*
		 * If GO in MCC support different beacon interval,
		 * change the BI of the P2P-GO
		 */
		roam_session = &mac_ctx->roam.roamSession[session_id];
		if (roam_session->bssParams.bssPersona != QDF_P2P_GO_MODE)
			continue;
		/*
		 * Handle different BI scneario based on the
		 * configuration set.If Config is set to 0x02 then
		 * Disconnect all the P2P clients associated. If config
		 * is set to 0x04 then update the BI without
		 * disconnecting all the clients
		 */
		if ((mac_ctx->roam.configParam.fAllowMCCGODiffBI == 0x04)
				&& (roam_session->bssParams.
					updatebeaconInterval)) {
			return csr_send_chng_mcc_beacon_interval(mac_ctx,
					session_id);
		} else if (roam_session->bssParams.updatebeaconInterval) {
			/*
			 * If the configuration of fAllowMCCGODiffBI is set to
			 * other than 0x04
			 */
			return csr_roam_call_callback(mac_ctx,
					session_id,
					NULL, 0,
					eCSR_ROAM_DISCONNECT_ALL_P2P_CLIENTS,
					eCSR_ROAM_RESULT_NONE);
		}
	}
	return QDF_STATUS_E_FAILURE;
}

static uint16_t csr_calculate_mcc_beacon_interval(struct mac_context *mac,
						  uint16_t sta_bi,
						  uint16_t go_gbi)
{
	uint8_t num_beacons = 0;
	uint8_t is_multiple = 0;
	uint16_t go_cbi = 0;
	uint16_t go_fbi = 0;
	uint16_t sta_cbi = 0;

	/* If GO's given beacon Interval is less than 100 */
	if (go_gbi < 100)
		go_cbi = 100;
	/* if GO's given beacon Interval is greater than or equal to 100 */
	else
		go_cbi = 100 + (go_gbi % 100);

	if (sta_bi == 0) {
		/* There is possibility to receive zero as value.
		 * Which will cause divide by zero. Hence initialise with 100
		 */
		sta_bi = 100;
		sme_warn("sta_bi 2nd parameter is zero, initialize to %d",
			sta_bi);
	}
	/* check, if either one is multiple of another */
	if (sta_bi > go_cbi)
		is_multiple = !(sta_bi % go_cbi);
	else
		is_multiple = !(go_cbi % sta_bi);

	/* if it is multiple, then accept GO's beacon interval
	 * range [100,199] as it is
	 */
	if (is_multiple)
		return go_cbi;

	/* else , if it is not multiple, then then check for number of beacons
	 * to be inserted based on sta BI
	 */
	num_beacons = sta_bi / 100;
	if (num_beacons) {
		/* GO's final beacon interval will be aligned to sta beacon
		 * interval, but in the range of [100, 199].
		 */
		sta_cbi = sta_bi / num_beacons;
		go_fbi = sta_cbi;
	} else
		/* if STA beacon interval is less than 100, use GO's change
		 * bacon interval instead of updating to STA's beacon interval.
		 */
		go_fbi = go_cbi;

	return go_fbi;
}

/**
 * csr_validate_p2pcli_bcn_intrvl() - to validate p2pcli beacon interval
 * @mac_ctx: pointer to mac context
 * @chnl_id: channel id variable
 * @bcn_interval: pointer to given beacon interval
 * @session_id: given session id
 * @status: fill the status in terms of QDF_STATUS to inform caller
 *
 * This API can provide the validation the beacon interval and re-calculate
 * in case concurrency
 *
 * Return: bool
 */
static bool csr_validate_p2pcli_bcn_intrvl(struct mac_context *mac_ctx,
		uint8_t chnl_id, uint16_t *bcn_interval, uint32_t session_id,
		QDF_STATUS *status)
{
	struct csr_roam_session *roamsession;

	roamsession = &mac_ctx->roam.roamSession[session_id];
	if (roamsession->pCurRoamProfile &&
		(roamsession->pCurRoamProfile->csrPersona ==
			 QDF_STA_MODE)) {
		/* check for P2P client mode */
		sme_debug("Ignore Beacon Interval Validation...");
	} else if (roamsession->bssParams.bssPersona == QDF_P2P_GO_MODE) {
		/* Check for P2P go scenario */
		if ((roamsession->bssParams.operationChn != chnl_id)
			&& (roamsession->bssParams.beaconInterval !=
				*bcn_interval)) {
			sme_err("BcnIntrvl is diff can't connect to P2P_GO network");
			*status = QDF_STATUS_E_FAILURE;
			return true;
		}
	}
	return false;
}

/**
 * csr_validate_p2pgo_bcn_intrvl() - to validate p2pgo beacon interval
 * @mac_ctx: pointer to mac context
 * @chnl_id: channel id variable
 * @bcn_interval: pointer to given beacon interval
 * @session_id: given session id
 * @status: fill the status in terms of QDF_STATUS to inform caller
 *
 * This API can provide the validation the beacon interval and re-calculate
 * in case concurrency
 *
 * Return: bool
 */
static bool csr_validate_p2pgo_bcn_intrvl(struct mac_context *mac_ctx,
		uint8_t chnl_id, uint16_t *bcn_interval,
		uint32_t session_id, QDF_STATUS *status)
{
	struct csr_roam_session *roamsession;
	struct csr_config *cfg_param;
	tCsrRoamConnectedProfile *conn_profile;
	uint16_t new_bcn_interval;

	roamsession = &mac_ctx->roam.roamSession[session_id];
	cfg_param = &mac_ctx->roam.configParam;
	conn_profile = &roamsession->connectedProfile;
	if (roamsession->pCurRoamProfile &&
		((roamsession->pCurRoamProfile->csrPersona ==
			  QDF_P2P_CLIENT_MODE) ||
		(roamsession->pCurRoamProfile->csrPersona ==
			  QDF_STA_MODE))) {
		/* check for P2P_client scenario */
		if ((conn_profile->operationChannel == 0) &&
			(conn_profile->beaconInterval == 0))
			return false;

		if (csr_is_conn_state_connected_infra(mac_ctx, session_id) &&
			(conn_profile->operationChannel != chnl_id) &&
			(conn_profile->beaconInterval != *bcn_interval)) {
			/*
			 * Updated beaconInterval should be used only when
			 * we are starting a new BSS not incase of
			 * client or STA case
			 */

			/* Calculate beacon Interval for P2P-GO incase of MCC */
			if (cfg_param->conc_custom_rule1 ||
					cfg_param->conc_custom_rule2) {
				new_bcn_interval = CSR_CUSTOM_CONC_GO_BI;
			} else {
				new_bcn_interval =
					csr_calculate_mcc_beacon_interval(
						mac_ctx,
						conn_profile->beaconInterval,
						*bcn_interval);
			}
			if (*bcn_interval != new_bcn_interval)
				*bcn_interval = new_bcn_interval;
			*status = QDF_STATUS_SUCCESS;
			return true;
		}
	}
	return false;
}

/**
 * csr_validate_sta_bcn_intrvl() - to validate sta beacon interval
 * @mac_ctx: pointer to mac context
 * @chnl_id: channel id variable
 * @bcn_interval: pointer to given beacon interval
 * @session_id: given session id
 * @status: fill the status in terms of QDF_STATUS to inform caller
 *
 * This API can provide the validation the beacon interval and re-calculate
 * in case concurrency
 *
 * Return: bool
 */
static bool csr_validate_sta_bcn_intrvl(struct mac_context *mac_ctx,
			uint8_t chnl_id, uint16_t *bcn_interval,
			uint32_t session_id, QDF_STATUS *status)
{
	struct csr_roam_session *roamsession;
	struct csr_config *cfg_param;
	uint16_t new_bcn_interval;

	roamsession = &mac_ctx->roam.roamSession[session_id];
	cfg_param = &mac_ctx->roam.configParam;

	if (roamsession->pCurRoamProfile &&
		(roamsession->pCurRoamProfile->csrPersona ==
				QDF_P2P_CLIENT_MODE)) {
		/* check for P2P client mode */
		sme_debug("Bcn Intrvl validation not require for STA/CLIENT");
		return false;
	}
	if ((roamsession->bssParams.bssPersona == QDF_SAP_MODE) &&
		   (roamsession->bssParams.operationChn != chnl_id)) {
		/*
		 * IF SAP has started and STA wants to connect
		 * on different channel MCC should
		 *  MCC should not be enabled so making it
		 * false to enforce on same channel
		 */
		sme_err("*** MCC with SAP+STA sessions ****");
		*status = QDF_STATUS_SUCCESS;
		return true;
	}
	/*
	 * Check for P2P go scenario
	 * if GO in MCC support different
	 * beacon interval,
	 * change the BI of the P2P-GO
	 */
	if ((roamsession->bssParams.bssPersona == QDF_P2P_GO_MODE) &&
		(roamsession->bssParams.operationChn != chnl_id) &&
		(roamsession->bssParams.beaconInterval != *bcn_interval)) {
		/* if GO in MCC support diff beacon interval, return success */
		if (cfg_param->fAllowMCCGODiffBI == 0x01) {
			*status = QDF_STATUS_SUCCESS;
			return true;
		}
		/*
		 * Send only Broadcast disassoc and update bcn_interval
		 * If configuration is set to 0x04 then dont
		 * disconnect all the station
		 */
		if ((cfg_param->fAllowMCCGODiffBI == 0x02)
			|| (cfg_param->fAllowMCCGODiffBI == 0x04)) {
			/* Check to pass the right beacon Interval */
			if (cfg_param->conc_custom_rule1 ||
				cfg_param->conc_custom_rule2) {
				new_bcn_interval = CSR_CUSTOM_CONC_GO_BI;
			} else {
				new_bcn_interval =
				csr_calculate_mcc_beacon_interval(
					mac_ctx, *bcn_interval,
					roamsession->bssParams.beaconInterval);
			}
			sme_debug("Peer AP BI : %d, new Beacon Interval: %d",
				*bcn_interval, new_bcn_interval);
			/* Update the becon Interval */
			if (new_bcn_interval !=
					roamsession->bssParams.beaconInterval) {
				/* Update the bcn_interval now */
				sme_err("Beacon Interval got changed config used: %d",
					cfg_param->fAllowMCCGODiffBI);

				roamsession->bssParams.beaconInterval =
					new_bcn_interval;
				roamsession->bssParams.updatebeaconInterval =
					true;
				*status = csr_update_mcc_p2p_beacon_interval(
					mac_ctx);
				return true;
			}
			*status = QDF_STATUS_SUCCESS;
			return true;
		}
		if (cfg_param->fAllowMCCGODiffBI
				== 0x03) {
			/* Disconnect the P2P session */
			roamsession->bssParams.updatebeaconInterval = false;
			*status = csr_roam_call_callback(mac_ctx,
					session_id, NULL, 0,
					eCSR_ROAM_SEND_P2P_STOP_BSS,
					eCSR_ROAM_RESULT_NONE);
			return true;
		}
		sme_err("BcnIntrvl is diff can't connect to preferred AP");
		*status = QDF_STATUS_E_FAILURE;
		return true;
	}
	return false;
}

QDF_STATUS csr_validate_mcc_beacon_interval(struct mac_context *mac_ctx,
					    uint8_t chnl_id,
					    uint16_t *bcn_interval,
					    uint32_t cur_session_id,
					    enum QDF_OPMODE cur_bss_persona)
{
	uint32_t session_id = 0;
	QDF_STATUS status;
	bool is_done;

	/* If MCC is not supported just break */
	if (!mac_ctx->roam.configParam.fenableMCCMode)
		return QDF_STATUS_E_FAILURE;

	for (session_id = 0; session_id < WLAN_MAX_VDEVS; session_id++) {
		if (cur_session_id == session_id)
			continue;

		if (!CSR_IS_SESSION_VALID(mac_ctx, session_id))
			continue;

		switch (cur_bss_persona) {
		case QDF_STA_MODE:
			is_done = csr_validate_sta_bcn_intrvl(mac_ctx, chnl_id,
					bcn_interval, session_id, &status);
			if (true == is_done)
				return status;
			break;

		case QDF_P2P_CLIENT_MODE:
			is_done = csr_validate_p2pcli_bcn_intrvl(mac_ctx,
					chnl_id, bcn_interval, session_id,
					&status);
			if (true == is_done)
				return status;
			break;

		case QDF_SAP_MODE:
		case QDF_IBSS_MODE:
			break;

		case QDF_P2P_GO_MODE:
			is_done = csr_validate_p2pgo_bcn_intrvl(mac_ctx,
					chnl_id, bcn_interval,
					session_id, &status);
			if (true == is_done)
				return status;
			break;

		default:
			sme_err("Persona not supported: %d", cur_bss_persona);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_is_auth_type11r() - Check if Authentication type is 11R
 * @mac: pointer to mac context
 * @auth_type: The authentication type that is used to make the connection
 * @mdie_present: Is MDIE IE present
 *
 * Return: true if is 11R auth type, false otherwise
 */
bool csr_is_auth_type11r(struct mac_context *mac, eCsrAuthType auth_type,
			uint8_t mdie_present)
{
	switch (auth_type) {
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		if (mdie_present &&
		    mac->mlme_cfg->lfr.enable_ftopen)
			return true;
		break;
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
	case eCSR_AUTH_TYPE_FT_RSN:
	case eCSR_AUTH_TYPE_FT_SAE:
	case eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384:
	case eCSR_AUTH_TYPE_FT_FILS_SHA256:
	case eCSR_AUTH_TYPE_FT_FILS_SHA384:
		return true;
	default:
		break;
	}
	return false;
}

/* Function to return true if the profile is 11r */
bool csr_is_profile11r(struct mac_context *mac,
			struct csr_roam_profile *pProfile)
{
	return csr_is_auth_type11r(mac, pProfile->negotiatedAuthType,
				   pProfile->mdid.mdie_present);
}

bool csr_is_auth_type_ese(eCsrAuthType AuthType)
{
	switch (AuthType) {
	case eCSR_AUTH_TYPE_CCKM_WPA:
	case eCSR_AUTH_TYPE_CCKM_RSN:
		return true;
	default:
		break;
	}
	return false;
}

#ifdef FEATURE_WLAN_ESE

/* Function to return true if the profile is ESE */
bool csr_is_profile_ese(struct csr_roam_profile *pProfile)
{
	return csr_is_auth_type_ese(pProfile->negotiatedAuthType);
}

#endif

#ifdef FEATURE_WLAN_WAPI
bool csr_is_profile_wapi(struct csr_roam_profile *pProfile)
{
	bool fWapiProfile = false;

	switch (pProfile->negotiatedAuthType) {
	case eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
	case eCSR_AUTH_TYPE_WAPI_WAI_PSK:
		fWapiProfile = true;
		break;

	default:
		fWapiProfile = false;
		break;
	}

	if (fWapiProfile) {
		switch (pProfile->negotiatedUCEncryptionType) {
		case eCSR_ENCRYPT_TYPE_WPI:
			fWapiProfile = true;
			break;

		default:
			fWapiProfile = false;
			break;
		}
	}
	return fWapiProfile;
}

static bool csr_is_wapi_oui_equal(struct mac_context *mac, uint8_t *Oui1,
				  uint8_t *Oui2)
{
	return !qdf_mem_cmp(Oui1, Oui2, CSR_WAPI_OUI_SIZE);
}

static bool csr_is_wapi_oui_match(struct mac_context *mac,
				  uint8_t AllCyphers[][CSR_WAPI_OUI_SIZE],
				  uint8_t cAllCyphers, uint8_t Cypher[],
				  uint8_t Oui[])
{
	bool fYes = false;
	uint8_t idx;

	for (idx = 0; idx < cAllCyphers; idx++) {
		if (csr_is_wapi_oui_equal(mac, AllCyphers[idx], Cypher)) {
			fYes = true;
			break;
		}
	}

	if (fYes && Oui)
		qdf_mem_copy(Oui, AllCyphers[idx], CSR_WAPI_OUI_SIZE);

	return fYes;
}
#endif /* FEATURE_WLAN_WAPI */

static bool csr_is_wpa_oui_equal(struct mac_context *mac, uint8_t *Oui1,
				 uint8_t *Oui2)
{
	return !qdf_mem_cmp(Oui1, Oui2, CSR_WPA_OUI_SIZE);
}

static bool csr_is_oui_match(struct mac_context *mac,
			     uint8_t AllCyphers[][CSR_WPA_OUI_SIZE],
			   uint8_t cAllCyphers, uint8_t Cypher[], uint8_t Oui[])
{
	bool fYes = false;
	uint8_t idx;

	for (idx = 0; idx < cAllCyphers; idx++) {
		if (csr_is_wpa_oui_equal(mac, AllCyphers[idx], Cypher)) {
			fYes = true;
			break;
		}
	}

	if (fYes && Oui)
		qdf_mem_copy(Oui, AllCyphers[idx], CSR_WPA_OUI_SIZE);

	return fYes;
}

static bool csr_match_rsnoui_index(struct mac_context *mac,
				   uint8_t AllCyphers[][CSR_RSN_OUI_SIZE],
				   uint8_t cAllCyphers, uint8_t ouiIndex,
				   uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllCyphers, cAllCyphers, csr_rsn_oui[ouiIndex], Oui);

}

#ifdef FEATURE_WLAN_WAPI
static bool csr_match_wapi_oui_index(struct mac_context *mac,
				     uint8_t AllCyphers[][CSR_WAPI_OUI_SIZE],
				     uint8_t cAllCyphers, uint8_t ouiIndex,
				     uint8_t Oui[])
{
	return csr_is_wapi_oui_match
		(mac, AllCyphers, cAllCyphers, csr_wapi_oui[ouiIndex], Oui);

}
#endif /* FEATURE_WLAN_WAPI */

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
static bool csr_match_wpaoui_index(struct mac_context *mac,
				   uint8_t AllCyphers[][CSR_RSN_OUI_SIZE],
				   uint8_t cAllCyphers, uint8_t ouiIndex,
				   uint8_t Oui[])
{
	if (ouiIndex < QDF_ARRAY_SIZE(csr_wpa_oui))
		return csr_is_oui_match
			(mac, AllCyphers, cAllCyphers,
			 csr_wpa_oui[ouiIndex], Oui);
	else
		return false;
}
#endif

#ifdef FEATURE_WLAN_WAPI
static bool csr_is_auth_wapi_cert(struct mac_context *mac,
				  uint8_t AllSuites[][CSR_WAPI_OUI_SIZE],
				  uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_wapi_oui_match
		(mac, AllSuites, cAllSuites, csr_wapi_oui[1], Oui);
}

static bool csr_is_auth_wapi_psk(struct mac_context *mac,
				 uint8_t AllSuites[][CSR_WAPI_OUI_SIZE],
				 uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_wapi_oui_match
		(mac, AllSuites, cAllSuites, csr_wapi_oui[2], Oui);
}
#endif /* FEATURE_WLAN_WAPI */


/*
 * Function for 11R FT Authentication. We match the FT Authentication Cipher
 * suite here. This matches for FT Auth with the 802.1X exchange.
 */
static bool csr_is_ft_auth_rsn(struct mac_context *mac,
			       uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
			       uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[03], Oui);
}

/*
 * Function for 11R FT Authentication. We match the FT Authentication Cipher
 * suite here. This matches for FT Auth with the PSK.
 */
static bool csr_is_ft_auth_rsn_psk(struct mac_context *mac,
				   uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
				   uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[04], Oui);
}


#ifdef FEATURE_WLAN_ESE

/*
 * Function for ESE CCKM AKM Authentication. We match the CCKM AKM
 * Authentication Key Management suite here. This matches for CCKM AKM Auth
 * with the 802.1X exchange.
 */
static bool csr_is_ese_cckm_auth_rsn(struct mac_context *mac,
				     uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
				     uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[06], Oui);
}

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
static bool csr_is_ese_cckm_auth_wpa(struct mac_context *mac,
				     uint8_t AllSuites[][CSR_WPA_OUI_SIZE],
				     uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_wpa_oui[06], Oui);
}
#endif

#endif

static bool csr_is_auth_rsn(struct mac_context *mac,
			    uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
			    uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[01], Oui);
}

static bool csr_is_auth_rsn_psk(struct mac_context *mac,
				uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
				uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[02], Oui);
}

#ifdef WLAN_FEATURE_11W
static bool csr_is_auth_rsn_psk_sha256(struct mac_context *mac,
				       uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
				       uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[07], Oui);
}
static bool csr_is_auth_rsn8021x_sha256(struct mac_context *mac,
					uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
					uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_rsn_oui[8], Oui);
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
/*
 * csr_is_auth_fils_sha256() - check whether oui is fils sha256
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is FILS SHA256, false otherwise
 */
static bool csr_is_auth_fils_sha256(struct mac_context *mac,
					uint8_t all_suites[][CSR_RSN_OUI_SIZE],
					uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_FILS_SHA256], oui);
}

/*
 * csr_is_auth_fils_sha384() - check whether oui is fils sha384
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is FILS SHA384, false otherwise
 */
static bool csr_is_auth_fils_sha384(struct mac_context *mac,
					uint8_t all_suites[][CSR_RSN_OUI_SIZE],
					uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_FILS_SHA384], oui);
}

/*
 * csr_is_auth_fils_ft_sha256() - check whether oui is fils ft sha256
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is FT FILS SHA256, false otherwise
 */
static bool csr_is_auth_fils_ft_sha256(struct mac_context *mac,
					uint8_t all_suites[][CSR_RSN_OUI_SIZE],
					uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_FT_FILS_SHA256], oui);
}

/*
 * csr_is_auth_fils_ft_sha384() - check whether oui is fils ft sha384
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is FT FILS SHA384, false otherwise
 */
static bool csr_is_auth_fils_ft_sha384(struct mac_context *mac,
					uint8_t all_suites[][CSR_RSN_OUI_SIZE],
					uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_FT_FILS_SHA384], oui);
}
#endif

/*
 * csr_is_auth_dpp_rsn() - check whether oui is dpp rsn
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is dpp rsn, false otherwise
 */
static bool csr_is_auth_dpp_rsn(struct mac_context *mac,
					uint8_t all_suites[][CSR_RSN_OUI_SIZE],
					uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_DPP_RSN], oui);
}

/*
 * csr_is_auth_wpa_owe() - check whether oui is OWE
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is OWE, false otherwise
 */
static bool csr_is_auth_wpa_owe(struct mac_context *mac,
			       uint8_t all_suites[][CSR_RSN_OUI_SIZE],
			       uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match
		(mac, all_suites, suite_count, csr_rsn_oui[ENUM_OWE], oui);
}

/*
 * csr_is_auth_suiteb_eap_256() - check whether oui is SuiteB EAP256
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is SuiteB EAP256, false otherwise
 */
static bool csr_is_auth_suiteb_eap_256(struct mac_context *mac,
			       uint8_t all_suites[][CSR_RSN_OUI_SIZE],
			       uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_SUITEB_EAP256], oui);
}

/*
 * csr_is_auth_suiteb_eap_384() - check whether oui is SuiteB EAP384
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is SuiteB EAP384, false otherwise
 */
static bool csr_is_auth_suiteb_eap_384(struct mac_context *mac,
			       uint8_t all_suites[][CSR_RSN_OUI_SIZE],
			       uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_SUITEB_EAP384], oui);
}

/*
 * csr_is_auth_ft_suiteb_eap_384() - check whether oui is SuiteB EAP384
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is FT SuiteB EAP384, false otherwise
 */
static
bool csr_is_auth_ft_suiteb_eap_384(struct mac_context *mac,
				   uint8_t all_suites[][CSR_RSN_OUI_SIZE],
				   uint8_t suite_count, uint8_t oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_FT_SUITEB_SHA384], oui);
}

#ifdef WLAN_FEATURE_SAE
/*
 * csr_is_auth_wpa_sae() - check whether oui is SAE
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is SAE, false otherwise
 */
static bool csr_is_auth_wpa_sae(struct mac_context *mac,
			       uint8_t all_suites[][CSR_RSN_OUI_SIZE],
			       uint8_t suite_count, uint8_t oui[])
{
	bool is_sae_auth;

	is_sae_auth = (csr_is_oui_match(mac, all_suites, suite_count,
					csr_rsn_oui[ENUM_SAE], oui) ||
		       csr_is_oui_match(mac, all_suites, suite_count,
					csr_rsn_oui[ENUM_FT_SAE], oui));
	return is_sae_auth;
}
#endif

/*
 * csr_is_auth_osen() - check whether oui is osen rsn
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is OSEN, false otherwise
 */
static bool csr_is_auth_osen(struct mac_context *mac,
			     u8 all_suites[][CSR_RSN_OUI_SIZE],
			     u8 suite_count, u8 oui[])
{
	return csr_is_oui_match(mac, all_suites, suite_count,
				csr_rsn_oui[ENUM_OSEN], oui);
}

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
static bool csr_is_auth_wpa(struct mac_context *mac,
			    uint8_t AllSuites[][CSR_WPA_OUI_SIZE],
			    uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_wpa_oui[01], Oui);
}

static bool csr_is_auth_wpa_psk(struct mac_context *mac,
				uint8_t AllSuites[][CSR_WPA_OUI_SIZE],
				uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match
		(mac, AllSuites, cAllSuites, csr_wpa_oui[02], Oui);
}
#endif

/*
 * csr_is_group_mgmt_gmac_128() - check whether oui is GMAC_128
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is GMAC_128, false otherwise
 */
static bool csr_is_group_mgmt_gmac_128(struct mac_context *mac,
				uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
				uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match(mac, AllSuites, cAllSuites,
				csr_group_mgmt_oui[ENUM_GMAC_128], Oui);
}

/*
 * csr_is_group_mgmt_gmac_256() - check whether oui is GMAC_256
 * @mac: Global MAC context
 * @all_suites: pointer to all supported akm suites
 * @suite_count: all supported akm suites count
 * @oui: Oui needs to be matched
 *
 * Return: True if OUI is GMAC_256, false otherwise
 */
static bool csr_is_group_mgmt_gmac_256(struct mac_context *mac,
				uint8_t AllSuites[][CSR_RSN_OUI_SIZE],
				uint8_t cAllSuites, uint8_t Oui[])
{
	return csr_is_oui_match(mac, AllSuites, cAllSuites,
				csr_group_mgmt_oui[ENUM_GMAC_256], Oui);
}

static uint8_t csr_get_oui_index_from_cipher(eCsrEncryptionType enType)
{
	uint8_t OUIIndex;

	switch (enType) {
	case eCSR_ENCRYPT_TYPE_WEP40:
	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
		OUIIndex = CSR_OUI_WEP40_OR_1X_INDEX;
		break;
	case eCSR_ENCRYPT_TYPE_WEP104:
	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
		OUIIndex = CSR_OUI_WEP104_INDEX;
		break;
	case eCSR_ENCRYPT_TYPE_TKIP:
		OUIIndex = CSR_OUI_TKIP_OR_PSK_INDEX;
		break;
	case eCSR_ENCRYPT_TYPE_AES:
		OUIIndex = CSR_OUI_AES_INDEX;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GCMP:
		OUIIndex = CSR_OUI_AES_GCMP_INDEX;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
		OUIIndex = CSR_OUI_AES_GCMP_256_INDEX;
		break;
	case eCSR_ENCRYPT_TYPE_NONE:
		OUIIndex = CSR_OUI_USE_GROUP_CIPHER_INDEX;
		break;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_ENCRYPT_TYPE_WPI:
		OUIIndex = CSR_OUI_WAPI_WAI_CERT_OR_SMS4_INDEX;
		break;
#endif /* FEATURE_WLAN_WAPI */
	default:                /* HOWTO handle this? */
		OUIIndex = CSR_OUI_RESERVED_INDEX;
		break;
	} /* switch */

	return OUIIndex;
}

#ifdef WLAN_FEATURE_FILS_SK
/**
 * csr_is_fils_auth() - update negotiated auth if matches to FILS auth type
 * @mac_ctx: pointer to mac context
 * @authsuites: auth suites
 * @c_auth_suites: auth suites count
 * @authentication: authentication
 * @auth_type: authentication type list
 * @index: current counter
 * @neg_authtype: pointer to negotiated auth
 *
 * Return: None
 */
static void csr_is_fils_auth(struct mac_context *mac_ctx,
	uint8_t authsuites[][CSR_RSN_OUI_SIZE], uint8_t c_auth_suites,
	uint8_t authentication[], tCsrAuthList *auth_type,
	uint8_t index, eCsrAuthType *neg_authtype)
{
	/*
	 * TODO Always try with highest security
	 * move this down once sha384 is validated
	 */
	if (csr_is_auth_fils_sha256(mac_ctx, authsuites,
				c_auth_suites, authentication)) {
		if (eCSR_AUTH_TYPE_FILS_SHA256 ==
				auth_type->authType[index])
			*neg_authtype = eCSR_AUTH_TYPE_FILS_SHA256;
	}
	if ((*neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
			csr_is_auth_fils_sha384(mac_ctx, authsuites,
				c_auth_suites, authentication)) {
		if (eCSR_AUTH_TYPE_FILS_SHA384 ==
				auth_type->authType[index])
			*neg_authtype = eCSR_AUTH_TYPE_FILS_SHA384;
	}
	if ((*neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
			csr_is_auth_fils_ft_sha256(mac_ctx, authsuites,
				c_auth_suites, authentication)) {
		if (eCSR_AUTH_TYPE_FT_FILS_SHA256 ==
				auth_type->authType[index])
			*neg_authtype = eCSR_AUTH_TYPE_FT_FILS_SHA256;
	}
	if ((*neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
			csr_is_auth_fils_ft_sha384(mac_ctx, authsuites,
				c_auth_suites, authentication)) {
		if (eCSR_AUTH_TYPE_FT_FILS_SHA384 ==
				auth_type->authType[index])
			*neg_authtype = eCSR_AUTH_TYPE_FT_FILS_SHA384;
	}
	sme_debug("negotiated auth type is %d", *neg_authtype);
}
#else
static void csr_is_fils_auth(struct mac_context *mac_ctx,
	uint8_t authsuites[][CSR_RSN_OUI_SIZE], uint8_t c_auth_suites,
	uint8_t authentication[], tCsrAuthList *auth_type,
	uint8_t index, eCsrAuthType *neg_authtype)
{
}
#endif

#ifdef WLAN_FEATURE_SAE
/**
 * csr_check_sae_auth() - update negotiated auth if matches to SAE auth type
 * @mac_ctx: pointer to mac context
 * @authsuites: auth suites
 * @c_auth_suites: auth suites count
 * @authentication: authentication
 * @auth_type: authentication type list
 * @index: current counter
 * @neg_authtype: pointer to negotiated auth
 *
 * Return: None
 */
static void csr_check_sae_auth(struct mac_context *mac_ctx,
	uint8_t authsuites[][CSR_RSN_OUI_SIZE], uint8_t c_auth_suites,
	uint8_t authentication[], tCsrAuthList *auth_type,
	uint8_t index, eCsrAuthType *neg_authtype)
{
	if ((*neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
	   csr_is_auth_wpa_sae(mac_ctx, authsuites,
	   c_auth_suites, authentication)) {
		if (eCSR_AUTH_TYPE_SAE == auth_type->authType[index])
			*neg_authtype = eCSR_AUTH_TYPE_SAE;
		else if (eCSR_AUTH_TYPE_FT_SAE == auth_type->authType[index])
			*neg_authtype = eCSR_AUTH_TYPE_FT_SAE;
	}
	sme_debug("negotiated auth type is %d", *neg_authtype);
}
#else
static void csr_check_sae_auth(struct mac_context *mac_ctx,
	uint8_t authsuites[][CSR_RSN_OUI_SIZE], uint8_t c_auth_suites,
	uint8_t authentication[], tCsrAuthList *auth_type,
	uint8_t index, eCsrAuthType *neg_authtype)
{
}
#endif

bool csr_is_pmkid_found_for_peer(struct mac_context *mac,
				 struct csr_roam_session *session,
				 tSirMacAddr peer_mac_addr,
				 uint8_t *pmkid,
				 uint16_t pmkid_count)
{
	uint32_t i, index;
	uint8_t *session_pmkid;
	tPmkidCacheInfo pmkid_cache;

	qdf_mem_zero(&pmkid_cache, sizeof(pmkid_cache));
	qdf_mem_copy(pmkid_cache.BSSID.bytes, peer_mac_addr,
		     QDF_MAC_ADDR_SIZE);

	if (!csr_lookup_pmkid_using_bssid(mac, session, &pmkid_cache, &index))
		return false;
	session_pmkid = &pmkid_cache.PMKID[0];
	for (i = 0; i < pmkid_count; i++) {
		if (!qdf_mem_cmp(pmkid + (i * PMKID_LEN),
				 session_pmkid, PMKID_LEN))
			return true;
	}

	sme_debug("PMKID in PmkidCacheInfo doesn't match with PMKIDs of peer");
	return false;
}

/**
 * csr_get_rsn_information() - to get RSN information
 * @mac_ctx: pointer to global MAC context
 * @auth_type: auth type
 * @encr_type: encryption type
 * @mc_encryption: multicast encryption type
 * @rsn_ie: pointer to RSN IE
 * @ucast_cipher: Unicast cipher
 * @mcast_cipher: Multicast cipher
 * @auth_suite: Authentication suite
 * @capabilities: RSN capabilities
 * @negotiated_authtype: Negotiated auth type
 * @negotiated_mccipher: negotiated multicast cipher
 * @gp_mgmt_cipher: group management cipher
 * @mgmt_encryption_type: group management encryption type
 *
 * This routine will get all RSN information
 *
 * Return: bool
 */
static bool csr_get_rsn_information(struct mac_context *mac_ctx,
				    tCsrAuthList *auth_type,
				    eCsrEncryptionType encr_type,
				    tCsrEncryptionList *mc_encryption,
				    tDot11fIERSN *rsn_ie, uint8_t *ucast_cipher,
				    uint8_t *mcast_cipher, uint8_t *auth_suite,
				    struct rsn_caps *capabilities,
				    eCsrAuthType *negotiated_authtype,
				    eCsrEncryptionType *negotiated_mccipher,
				    uint8_t *gp_mgmt_cipher,
				    tAniEdType *mgmt_encryption_type)
{
	bool acceptable_cipher = false;
	bool group_mgmt_acceptable_cipher = false;
	uint8_t c_ucast_cipher = 0;
	uint8_t c_mcast_cipher = 0;
	uint8_t c_group_mgmt_cipher = 0;
	uint8_t c_auth_suites = 0, i;
	uint8_t unicast[CSR_RSN_OUI_SIZE];
	uint8_t multicast[CSR_RSN_OUI_SIZE];
	uint8_t group_mgmt[CSR_RSN_OUI_SIZE];
	uint8_t authsuites[CSR_RSN_MAX_AUTH_SUITES][CSR_RSN_OUI_SIZE];
	uint8_t authentication[CSR_RSN_OUI_SIZE];
	uint8_t mccipher_arr[CSR_RSN_MAX_MULTICAST_CYPHERS][CSR_RSN_OUI_SIZE];
	uint8_t group_mgmt_arr[CSR_RSN_MAX_MULTICAST_CYPHERS][CSR_RSN_OUI_SIZE];
	eCsrAuthType neg_authtype = eCSR_AUTH_TYPE_UNKNOWN;

	if (!rsn_ie->present)
		goto end;
	c_mcast_cipher++;
	qdf_mem_copy(mccipher_arr, rsn_ie->gp_cipher_suite,
			CSR_RSN_OUI_SIZE);
	c_ucast_cipher =
		(uint8_t) (rsn_ie->pwise_cipher_suite_count);

	c_auth_suites = (uint8_t) (rsn_ie->akm_suite_cnt);
	for (i = 0; i < c_auth_suites && i < CSR_RSN_MAX_AUTH_SUITES; i++) {
		qdf_mem_copy((void *)&authsuites[i],
			(void *)&rsn_ie->akm_suite[i], CSR_RSN_OUI_SIZE);
	}

	/* Check - Is requested unicast Cipher supported by the BSS. */
	acceptable_cipher = csr_match_rsnoui_index(mac_ctx,
				rsn_ie->pwise_cipher_suites, c_ucast_cipher,
				csr_get_oui_index_from_cipher(encr_type),
				unicast);

	if (!acceptable_cipher)
		goto end;

	/* unicast is supported. Pick the first matching Group cipher, if any */
	for (i = 0; i < mc_encryption->numEntries; i++) {
		acceptable_cipher = csr_match_rsnoui_index(mac_ctx,
					mccipher_arr, c_mcast_cipher,
					csr_get_oui_index_from_cipher(
					    mc_encryption->encryptionType[i]),
					multicast);
		if (acceptable_cipher)
			break;
	}
	if (!acceptable_cipher)
		goto end;

	if (negotiated_mccipher)
		*negotiated_mccipher = mc_encryption->encryptionType[i];

	/* Group Management Cipher only for 11w */
	if (mgmt_encryption_type) {
		c_group_mgmt_cipher++;
		qdf_mem_copy(group_mgmt_arr, rsn_ie->gp_mgmt_cipher_suite,
						CSR_RSN_OUI_SIZE);
		if (csr_is_group_mgmt_gmac_128(mac_ctx, group_mgmt_arr,
			  c_group_mgmt_cipher, group_mgmt)) {
			group_mgmt_acceptable_cipher = true;
			*mgmt_encryption_type = eSIR_ED_AES_GMAC_128;
		} else if (csr_is_group_mgmt_gmac_256(mac_ctx, group_mgmt_arr,
			  c_group_mgmt_cipher, group_mgmt)) {
			group_mgmt_acceptable_cipher = true;
			*mgmt_encryption_type = eSIR_ED_AES_GMAC_256;
		} else {
			/* Default is CMAC */
			group_mgmt_acceptable_cipher = true;
			*mgmt_encryption_type = eSIR_ED_AES_128_CMAC;
			qdf_mem_copy(group_mgmt, csr_group_mgmt_oui[ENUM_CMAC],
						CSR_RSN_OUI_SIZE);
		}
	}

	/* Initializing with false as it has true value already */
	acceptable_cipher = false;
	for (i = 0; i < auth_type->numEntries; i++) {
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		/* Set FILS as first preference */
		csr_is_fils_auth(mac_ctx, authsuites, c_auth_suites,
			authentication, auth_type, i, &neg_authtype);
		/* Changed the AKM suites according to order of preference */
		csr_check_sae_auth(mac_ctx, authsuites, c_auth_suites,
			authentication, auth_type, i, &neg_authtype);

		if (neg_authtype == eCSR_AUTH_TYPE_UNKNOWN &&
		    csr_is_auth_osen(mac_ctx, authsuites,
				     c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_OSEN == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_OSEN;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
				csr_is_auth_dpp_rsn(mac_ctx, authsuites,
						    c_auth_suites,
						    authentication)) {
			if (eCSR_AUTH_TYPE_DPP_RSN == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_DPP_RSN;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
				csr_is_ft_auth_rsn(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_FT_RSN == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_FT_RSN;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN)
				&& csr_is_ft_auth_rsn_psk(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_FT_RSN_PSK ==
					auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_FT_RSN_PSK;
		}
#ifdef FEATURE_WLAN_ESE
		/* ESE only supports 802.1X.  No PSK. */
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
				csr_is_ese_cckm_auth_rsn(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_CCKM_RSN == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_CCKM_RSN;
		}
#endif
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN)
				&& csr_is_auth_rsn(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_RSN == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_RSN;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN)
				&& csr_is_auth_rsn_psk(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_RSN_PSK == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_RSN_PSK;
		}
#ifdef WLAN_FEATURE_11W
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN)
			&& csr_is_auth_rsn_psk_sha256(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_RSN_PSK_SHA256 ==
					auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_RSN_PSK_SHA256;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
				csr_is_auth_rsn8021x_sha256(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_RSN_8021X_SHA256 ==
					auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_RSN_8021X_SHA256;
		}
#endif
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
				csr_is_auth_wpa_owe(mac_ctx, authsuites,
					c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_OWE == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_OWE;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
		   csr_is_auth_suiteb_eap_256(mac_ctx, authsuites,
		   c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_SUITEB_EAP_SHA256 ==
						auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_SUITEB_EAP_SHA256;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
		   csr_is_auth_suiteb_eap_384(mac_ctx, authsuites,
		   c_auth_suites, authentication)) {
			if (eCSR_AUTH_TYPE_SUITEB_EAP_SHA384 ==
						auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_SUITEB_EAP_SHA384;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
		    csr_is_auth_ft_suiteb_eap_384(mac_ctx, authsuites,
						  c_auth_suites,
						  authentication)) {
			if (eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384 ==
						auth_type->authType[i])
				neg_authtype =
					eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384;
		}

		/*
		 * The 1st auth type in the APs RSN IE, to match stations
		 * connecting profiles auth type will cause us to exit this
		 * loop. This is added as some APs advertise multiple akms in
		 * the RSN IE
		 */
		if (eCSR_AUTH_TYPE_UNKNOWN != neg_authtype) {
			acceptable_cipher = true;
			break;
		}
	} /* for */
end:
	if (acceptable_cipher) {
		if (mcast_cipher)
			qdf_mem_copy(mcast_cipher, multicast,
					CSR_RSN_OUI_SIZE);

		if (ucast_cipher)
			qdf_mem_copy(ucast_cipher, unicast, CSR_RSN_OUI_SIZE);

		if (gp_mgmt_cipher && group_mgmt_acceptable_cipher)
			qdf_mem_copy(gp_mgmt_cipher, group_mgmt,
				     CSR_RSN_OUI_SIZE);

		if (auth_suite)
			qdf_mem_copy(auth_suite, authentication,
					CSR_RSN_OUI_SIZE);

		if (negotiated_authtype)
			*negotiated_authtype = neg_authtype;

		if (capabilities) {
			/* Bit 0 Preauthentication */
			capabilities->PreAuthSupported =
				(rsn_ie->RSN_Cap[0] >> 0) & 0x1;
			/* Bit 1 No Pairwise */
			capabilities->NoPairwise =
				(rsn_ie->RSN_Cap[0] >> 1) & 0x1;
			/* Bit 2, 3 PTKSA Replay Counter */
			capabilities->PTKSAReplayCounter =
				(rsn_ie->RSN_Cap[0] >> 2) & 0x3;
			/* Bit 4, 5 GTKSA Replay Counter */
			capabilities->GTKSAReplayCounter =
				(rsn_ie->RSN_Cap[0] >> 4) & 0x3;
#ifdef WLAN_FEATURE_11W
			/* Bit 6 MFPR */
			capabilities->MFPRequired =
				(rsn_ie->RSN_Cap[0] >> 6) & 0x1;
			/* Bit 7 MFPC */
			capabilities->MFPCapable =
				(rsn_ie->RSN_Cap[0] >> 7) & 0x1;
#else
			/* Bit 6 MFPR */
			capabilities->MFPRequired = 0;
			/* Bit 7 MFPC */
			capabilities->MFPCapable = 0;
#endif
			/* remaining reserved */
			capabilities->Reserved = rsn_ie->RSN_Cap[1] & 0xff;
		}
	}
	return acceptable_cipher;
}

#ifdef WLAN_FEATURE_11W
/**
 * csr_is_pmf_capabilities_in_rsn_match() - check for PMF capability
 * @mac:                   Global MAC Context
 * @pFilterMFPEnabled:     given by supplicant to us to specify what kind
 *                         of connection supplicant is expecting to make
 *                         if it is enabled then make PMF connection.
 *                         if it is disabled then make normal connection.
 * @pFilterMFPRequired:    given by supplicant based on our configuration
 *                         if it is 1 then we will require mandatory
 *                         PMF connection and if it is 0 then we PMF
 *                         connection is optional.
 * @pFilterMFPCapable:     given by supplicant based on our configuration
 *                         if it 1 then we are PMF capable and if it 0
 *                         then we are not PMF capable.
 * @pRSNIe:                RSNIe from Beacon/probe response of
 *                         neighbor AP against which we will compare
 *                         our capabilities.
 *
 * This function is to match our current capabilities with the AP
 * to which we are expecting make the connection.
 *
 * Return:   if our PMF capabilities matches with AP then we
 *           will return true to indicate that we are good
 *           to make connection with it. Else we will return false
 **/
static bool
csr_is_pmf_capabilities_in_rsn_match(struct mac_context *mac,
				     bool *pFilterMFPEnabled,
				     uint8_t *pFilterMFPRequired,
				     uint8_t *pFilterMFPCapable,
				     tDot11fIERSN *pRSNIe)
{
	uint8_t apProfileMFPCapable = 0;
	uint8_t apProfileMFPRequired = 0;

	if (pRSNIe && pFilterMFPEnabled && pFilterMFPCapable
	    && pFilterMFPRequired) {
		/* Extracting MFPCapable bit from RSN Ie */
		apProfileMFPCapable = csr_is_mfpc_capable(pRSNIe);
		apProfileMFPRequired = (pRSNIe->RSN_Cap[0] >> 6) & 0x1;

		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"pFilterMFPEnabled: %d pFilterMFPRequired: %d pFilterMFPCapable: %d apProfileMFPCapable: %d apProfileMFPRequired: %d",
			 *pFilterMFPEnabled, *pFilterMFPRequired,
			 *pFilterMFPCapable, apProfileMFPCapable,
			 apProfileMFPRequired);

		if (*pFilterMFPEnabled && *pFilterMFPCapable
		    && *pFilterMFPRequired && (apProfileMFPCapable == 0)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "AP is not capable to make PMF connection");
			return false;
		}  else if (!(*pFilterMFPCapable) &&
			   apProfileMFPCapable && apProfileMFPRequired) {

			/*
			 * In this case, AP with whom we trying to connect
			 * requires mandatory PMF connections and we are not
			 * capable so this AP is not good choice to connect
			 */
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "AP needs PMF connection and we are not capable of pmf connection");
			return false;
		}
	}
	return true;
}
#endif

static bool csr_is_rsn_match(struct mac_context *mac_ctx, tCsrAuthList *pAuthType,
			     eCsrEncryptionType enType,
			     tCsrEncryptionList *pEnMcType,
			     bool *pMFPEnabled, uint8_t *pMFPRequired,
			     uint8_t *pMFPCapable,
			     tDot11fBeaconIEs *pIes,
			     eCsrAuthType *pNegotiatedAuthType,
			     eCsrEncryptionType *pNegotiatedMCCipher)
{
	bool fRSNMatch = false;

	/* See if the cyphers in the Bss description match with the
	 * settings in the profile.
	 */
	fRSNMatch = csr_get_rsn_information(mac_ctx, pAuthType, enType,
					pEnMcType, &pIes->RSN,
					NULL, NULL, NULL, NULL,
					pNegotiatedAuthType,
					pNegotiatedMCCipher, NULL, NULL);
#ifdef WLAN_FEATURE_11W
	/* If all the filter matches then finally checks for PMF capabilities */
	if (fRSNMatch)
		fRSNMatch = csr_is_pmf_capabilities_in_rsn_match(mac_ctx,
								pMFPEnabled,
								 pMFPRequired,
								 pMFPCapable,
								 &pIes->RSN);
#endif
	return fRSNMatch;
}

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
/**
 * csr_lookup_pmkid_using_ssid() - lookup pmkid using ssid and cache_id
 * @mac: pointer to mac
 * @session: sme session pointer
 * @pmk_cache: pointer to pmk cache
 * @index: index value needs to be seached
 *
 * Return: true if pmkid is found else false
 */
static bool csr_lookup_pmkid_using_ssid(struct mac_context *mac,
					struct csr_roam_session *session,
					tPmkidCacheInfo *pmk_cache,
					uint32_t *index)
{
	uint32_t i;
	tPmkidCacheInfo *session_pmk;

	for (i = 0; i < session->NumPmkidCache; i++) {
		session_pmk = &session->PmkidCacheInfo[i];
		sme_debug("match PMKID ssid %*.*s cache id %x %x ssid_len %d to ssid %s cache_id %x %x",
			pmk_cache->ssid_len, pmk_cache->ssid_len,
			pmk_cache->ssid, pmk_cache->cache_id[0],
			pmk_cache->cache_id[1], pmk_cache->ssid_len,
			session_pmk->ssid,
			session_pmk->cache_id[0], session_pmk->cache_id[1]);

		if ((!qdf_mem_cmp(pmk_cache->ssid, session_pmk->ssid,
				  pmk_cache->ssid_len)) &&
		    (!qdf_mem_cmp(session_pmk->cache_id,
				  pmk_cache->cache_id, CACHE_ID_LEN))) {
			/* match found */
			*index = i;
			sme_debug("PMKID found at index %d", i);
			return true;
		}
	}

	return false;
}
#endif

#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
bool csr_lookup_pmkid_using_bssid(struct mac_context *mac,
				  struct csr_roam_session *session,
				  tPmkidCacheInfo *pmk_cache,
				  uint32_t *index)
{
	struct wlan_crypto_pmksa *pmksa;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, session->vdev_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("Invalid vdev");
		return false;
	}

	pmksa = wlan_crypto_get_pmksa(vdev, &pmk_cache->BSSID);
	if (!pmksa) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
		return false;
	}
	qdf_mem_copy(pmk_cache->PMKID, pmksa->pmkid, sizeof(pmk_cache->PMKID));
	qdf_mem_copy(pmk_cache->pmk, pmksa->pmk, pmksa->pmk_len);
	pmk_cache->pmk_len = pmksa->pmk_len;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return true;
}
#else
bool csr_lookup_pmkid_using_bssid(struct mac_context *mac,
					struct csr_roam_session *session,
					tPmkidCacheInfo *pmk_cache,
					uint32_t *index)
{
	uint32_t i;
	tPmkidCacheInfo *session_pmk;

	for (i = 0; i < session->NumPmkidCache; i++) {
		session_pmk = &session->PmkidCacheInfo[i];
		sme_debug("Matching BSSID: " MAC_ADDRESS_STR " to cached BSSID:"
			MAC_ADDRESS_STR, MAC_ADDR_ARRAY(pmk_cache->BSSID.bytes),
			MAC_ADDR_ARRAY(session_pmk->BSSID.bytes));
		if (qdf_is_macaddr_equal(&pmk_cache->BSSID,
					 &session_pmk->BSSID)) {
			/* match found */
			*index = i;
			sme_debug("PMKID found at index %d", i);
			qdf_mem_copy(pmk_cache, session_pmk,
				     sizeof(tPmkidCacheInfo));
			return true;
		}
	}

	return false;
}
#endif

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
/**
 * csr_lookup_pmkid() - lookup pmkid using bssid or ssid + cache_id
 * @mac: pointer to mac
 * @session: sme session pointer
 * @pmk_cache: pointer to pmk cache
 * @index: index value needs to be seached
 *
 * Return: true if pmkid is found else false
 */
static bool csr_lookup_pmkid(struct mac_context *mac, uint32_t sessionId,
				tPmkidCacheInfo *pmk_cache)
{
	bool fRC = false, fMatchFound = false;
	uint32_t Index;
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return false;
	}

	if (pmk_cache->ssid_len) {
		/* Try to find based on cache_id and ssid first */
		fMatchFound = csr_lookup_pmkid_using_ssid(mac, pSession,
							  pmk_cache, &Index);
	}

	/* If not able to find using cache id or ssid_len is not present */
	if (!fMatchFound)
		fMatchFound = csr_lookup_pmkid_using_bssid(mac,
						pSession, pmk_cache, &Index);

	if (!fMatchFound) {
		sme_debug("no pmkid match found NumPmkidCache = %d",
			pSession->NumPmkidCache);
		return false;
	}

	qdf_mem_copy(pmk_cache->PMKID,
		     pSession->PmkidCacheInfo[Index].PMKID,
		     PMKID_LEN);

	qdf_mem_copy(pmk_cache->pmk,
		     pSession->PmkidCacheInfo[Index].pmk,
		     pSession->PmkidCacheInfo[Index].pmk_len);
	pmk_cache->pmk_len = pSession->PmkidCacheInfo[Index].pmk_len;

	fRC = true;
	sme_debug("match = %d NumPmkidCache = %d",
		fRC, pSession->NumPmkidCache);

	return fRC;
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
/*
 * csr_update_pmksa_for_cache_id: update tPmkidCacheInfo to lookup using
 * ssid and cache id
 * @bss_desc: bss description
 * @profile: csr roam profile
 * @pmkid_cache: pmksa cache
 *
 * Return: true if cache identifier present else false
 */
static bool csr_update_pmksa_for_cache_id(tSirBssDescription *bss_desc,
				struct csr_roam_profile *profile,
				tPmkidCacheInfo *pmkid_cache)
{
	if (!bss_desc->fils_info_element.is_cache_id_present)
		return false;

	pmkid_cache->ssid_len =
		profile->SSIDs.SSIDList[0].SSID.length;
	qdf_mem_copy(pmkid_cache->ssid,
		profile->SSIDs.SSIDList[0].SSID.ssId,
		profile->SSIDs.SSIDList[0].SSID.length);
	qdf_mem_copy(pmkid_cache->cache_id,
		bss_desc->fils_info_element.cache_id,
		CACHE_ID_LEN);
	qdf_mem_copy(pmkid_cache->BSSID.bytes,
		bss_desc->bssId, QDF_MAC_ADDR_SIZE);

	return true;

}
#endif
/*
 * csr_update_pmksa_to_profile: update pmk and pmkid to profile which will be
 * used in case of fils session
 * @profile: profile
 * @pmkid_cache: pmksa cache
 *
 * Return: None
 */
static inline void csr_update_pmksa_to_profile(struct csr_roam_profile *profile,
					       tPmkidCacheInfo *pmkid_cache)
{
	if (!profile->fils_con_info)
		return;

	profile->fils_con_info->pmk_len = pmkid_cache->pmk_len;
	qdf_mem_copy(profile->fils_con_info->pmk,
			pmkid_cache->pmk, pmkid_cache->pmk_len);
	qdf_mem_copy(profile->fils_con_info->pmkid,
		pmkid_cache->PMKID, PMKID_LEN);

}
#else
static inline bool csr_update_pmksa_for_cache_id(tSirBssDescription *bss_desc,
				struct csr_roam_profile *profile,
				tPmkidCacheInfo *pmkid_cache)
{
	return false;
}

static inline void csr_update_pmksa_to_profile(struct csr_roam_profile *profile,
					       tPmkidCacheInfo *pmkid_cache)
{
}
#endif

#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
uint8_t csr_construct_rsn_ie(struct mac_context *mac, uint32_t sessionId,
			     struct csr_roam_profile *pProfile,
			     tSirBssDescription *pSirBssDesc,
			     tDot11fBeaconIEs *ap_ie, tCsrRSNIe *pRSNIe)
{
	struct wlan_objmgr_vdev *vdev;
	uint8_t *rsn_ie_end = NULL;
	uint8_t *rsn_ie = (uint8_t *)pRSNIe;
	uint8_t ie_len = 0;
	tDot11fBeaconIEs *local_ap_ie = ap_ie;
	uint16_t rsn_cap = 0;
	struct qdf_mac_addr bssid;

	if (!local_ap_ie &&
	    (!QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies
	     (mac, pSirBssDesc, &local_ap_ie))))
		return ie_len;

	/* get AP RSN cap */
	qdf_mem_copy(&rsn_cap, local_ap_ie->RSN.RSN_Cap, sizeof(rsn_cap));
	if (!ap_ie && local_ap_ie)
		/* locally allocated */
		qdf_mem_free(local_ap_ie);

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("Invalid vdev");
		return ie_len;
	}
	/*
	 * Use intersection of the RSN cap sent by user space and
	 * the AP, so that only common capability are enabled.
	 */
	rsn_cap &= (uint16_t)wlan_crypto_get_param(vdev,
						   WLAN_CRYPTO_PARAM_RSN_CAP);
	wlan_crypto_set_vdev_param(vdev, WLAN_CRYPTO_PARAM_RSN_CAP, rsn_cap);
	qdf_mem_copy(bssid.bytes, pSirBssDesc->bssId, QDF_MAC_ADDR_SIZE);
	rsn_ie_end = wlan_crypto_build_rsnie(vdev, rsn_ie, &bssid);

	if (rsn_ie_end)
		ie_len = rsn_ie_end - rsn_ie;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return ie_len;
}
#else
/**
 * csr_update_session_pmk() - Update the pmk len and pmk in the roam session
 * @session: pointer to the CSR Roam session
 * @pmkid_cache: pointer to the pmkid cache
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void csr_update_session_pmk(struct csr_roam_session *session,
				   tPmkidCacheInfo *pmkid_cache)
{
	session->pmk_len = pmkid_cache->pmk_len;
	qdf_mem_zero(session->psk_pmk, sizeof(session->psk_pmk));
	qdf_mem_copy(session->psk_pmk, pmkid_cache->pmk, session->pmk_len);
}
#else
static inline void csr_update_session_pmk(struct csr_roam_session *session,
					  tPmkidCacheInfo *pmkid_cache)
{
}
#endif

uint8_t csr_construct_rsn_ie(struct mac_context *mac, uint32_t sessionId,
			     struct csr_roam_profile *pProfile,
			     tSirBssDescription *pSirBssDesc,
			     tDot11fBeaconIEs *pIes, tCsrRSNIe *pRSNIe)
{
	uint32_t ret;
	bool fRSNMatch;
	uint8_t cbRSNIe = 0;
	uint8_t UnicastCypher[CSR_RSN_OUI_SIZE];
	uint8_t MulticastCypher[CSR_RSN_OUI_SIZE];
	uint8_t gp_mgmt_cipher_suite[CSR_RSN_OUI_SIZE];
	uint8_t AuthSuite[CSR_RSN_OUI_SIZE];
	tCsrRSNAuthIe *pAuthSuite;
	struct rsn_caps RSNCapabilities;
	tCsrRSNPMKIe *pPMK;
	tPmkidCacheInfo pmkid_cache;
#ifdef WLAN_FEATURE_11W
	uint8_t *pGroupMgmtCipherSuite;
#endif
	tDot11fBeaconIEs *pIesLocal = pIes;
	eCsrAuthType negAuthType = eCSR_AUTH_TYPE_UNKNOWN;
	tDot11fIERSN rsn_ie = {0};
	struct csr_roam_session *session = CSR_GET_SESSION(mac, sessionId);

	if (!CSR_IS_SESSION_VALID(mac, sessionId) || !session)
		return 0;
	qdf_mem_zero(&pmkid_cache, sizeof(pmkid_cache));
	qdf_mem_zero(&rsn_ie, sizeof(rsn_ie));

	do {
		if (!csr_is_profile_rsn(pProfile))
			break;

		if (!pIesLocal
		    &&
		    (!QDF_IS_STATUS_SUCCESS
			     (csr_get_parsed_bss_description_ies
				     (mac, pSirBssDesc, &pIesLocal)))) {
			break;
		}

		/*
		 * Use intersection of the RSN cap sent by user space and
		 * the AP, so that only common capability are enabled.
		 */
		if (pProfile->pRSNReqIE && pProfile->nRSNReqIELength) {
			ret = dot11f_unpack_ie_rsn(mac,
						   pProfile->pRSNReqIE + 2,
				  pProfile->nRSNReqIELength -2, &rsn_ie, false);
			if (!DOT11F_FAILED(ret)) {
				pIesLocal->RSN.RSN_Cap[0] =
						pIesLocal->RSN.RSN_Cap[0] &
						rsn_ie.RSN_Cap[0];
				pIesLocal->RSN.RSN_Cap[1] =
						pIesLocal->RSN.RSN_Cap[1] &
						rsn_ie.RSN_Cap[1];
			}
		}
		/* See if the cyphers in the Bss description match with the
		 * settings in the profile.
		 */
		fRSNMatch = csr_get_rsn_information(mac, &pProfile->AuthType,
					pProfile->negotiatedUCEncryptionType,
					&pProfile->mcEncryptionType,
					&pIesLocal->RSN, UnicastCypher,
					MulticastCypher, AuthSuite,
					&RSNCapabilities, &negAuthType, NULL,
					gp_mgmt_cipher_suite,
					&pProfile->mgmt_encryption_type);
		if (!fRSNMatch)
			break;

		pRSNIe->IeHeader.ElementID = SIR_MAC_RSN_EID;

		pRSNIe->Version = CSR_RSN_VERSION_SUPPORTED;

		qdf_mem_copy(pRSNIe->MulticastOui, MulticastCypher,
			     sizeof(MulticastCypher));

		pRSNIe->cUnicastCyphers = 1;

		qdf_mem_copy(&pRSNIe->UnicastOui[0], UnicastCypher,
			     sizeof(UnicastCypher));

		pAuthSuite =
			(tCsrRSNAuthIe *) (&pRSNIe->
					   UnicastOui[pRSNIe->cUnicastCyphers]);

		pAuthSuite->cAuthenticationSuites = 1;
		qdf_mem_copy(&pAuthSuite->AuthOui[0], AuthSuite,
			     sizeof(AuthSuite));

		/* PreAuthSupported is an AP only capability */
		RSNCapabilities.PreAuthSupported = 0;
		/*
		 * Use the Management Frame Protection values given by the
		 * supplicant, if AP and STA both are MFP capable.
		 */
#ifdef WLAN_FEATURE_11W
		if (RSNCapabilities.MFPCapable && pProfile->MFPCapable) {
			RSNCapabilities.MFPCapable = pProfile->MFPCapable;
			RSNCapabilities.MFPRequired = pProfile->MFPRequired;
		} else {
			RSNCapabilities.MFPCapable = 0;
			RSNCapabilities.MFPRequired = 0;
		}
#endif
		*(uint16_t *) (&pAuthSuite->AuthOui[1]) =
			*((uint16_t *) (&RSNCapabilities));

		pPMK = (tCsrRSNPMKIe *) (((uint8_t *) (&pAuthSuite->AuthOui[1]))
				+ sizeof(uint16_t));

		if (!csr_update_pmksa_for_cache_id(pSirBssDesc,
			pProfile, &pmkid_cache))
			qdf_mem_copy(pmkid_cache.BSSID.bytes,
				pSirBssDesc->bssId, QDF_MAC_ADDR_SIZE);
		/* Don't include the PMK SA IDs for CCKM associations. */
		if (
#ifdef FEATURE_WLAN_ESE
			(eCSR_AUTH_TYPE_CCKM_RSN != negAuthType) &&
#endif
			csr_lookup_pmkid(mac, sessionId, &pmkid_cache)) {
			pPMK->cPMKIDs = 1;

			qdf_trace_hex_dump(QDF_MODULE_ID_PE,
				   QDF_TRACE_LEVEL_INFO,
				   pmkid_cache.pmk, pmkid_cache.pmk_len);
			qdf_mem_copy(pPMK->PMKIDList[0].PMKID,
				     pmkid_cache.PMKID,
				     PMKID_LEN);

			/*
			 * If a PMK cache is found for the BSSID, then
			 * update the PMK in CSR session also as this
			 * will be sent to the FW during RSO.
			 */
			csr_update_session_pmk(session, &pmkid_cache);

			csr_update_pmksa_to_profile(pProfile, &pmkid_cache);
		} else {
			pPMK->cPMKIDs = 0;
		}
		session->rsn_caps = RSNCapabilities;

		qdf_mem_zero(&pmkid_cache, sizeof(pmkid_cache));

#ifdef WLAN_FEATURE_11W
		/* Advertise BIP in group cipher key management only if PMF is
		 * enabled and AP is capable.
		 */
		if (pProfile->MFPEnabled &&
			(RSNCapabilities.MFPCapable && pProfile->MFPCapable)) {
			pGroupMgmtCipherSuite =
				(uint8_t *) pPMK + sizeof(uint16_t) +
				(pPMK->cPMKIDs * PMKID_LEN);
			qdf_mem_copy(pGroupMgmtCipherSuite,
				     gp_mgmt_cipher_suite, CSR_RSN_OUI_SIZE);
		}
#endif
	host_log_rsn_info(UnicastCypher, MulticastCypher,
			  AuthSuite, gp_mgmt_cipher_suite);

		/* Add in the fixed fields plus 1 Unicast cypher, less the
		 * IE Header length Add in the size of the Auth suite (count
		 * plus a single OUI) Add in the RSN caps field.
		 * Add PMKID count and PMKID (if any)
		 * Add group management cipher suite
		 */
		pRSNIe->IeHeader.Length =
			(uint8_t) (sizeof(*pRSNIe) - sizeof(pRSNIe->IeHeader) +
				   sizeof(*pAuthSuite) +
				   sizeof(struct rsn_caps));
		if (pPMK->cPMKIDs)
			pRSNIe->IeHeader.Length += (uint8_t) (sizeof(uint16_t) +
							      (pPMK->cPMKIDs *
							PMKID_LEN));
#ifdef WLAN_FEATURE_11W
		if (pProfile->MFPEnabled &&
			(RSNCapabilities.MFPCapable && pProfile->MFPCapable)) {
			if (0 == pPMK->cPMKIDs)
				pRSNIe->IeHeader.Length += sizeof(uint16_t);
			pRSNIe->IeHeader.Length += CSR_WPA_OUI_SIZE;
		}
#endif

		/* return the size of the IE header (total) constructed... */
		cbRSNIe = pRSNIe->IeHeader.Length + sizeof(pRSNIe->IeHeader);

	} while (0);

	if (!pIes && pIesLocal)
		/* locally allocated */
		qdf_mem_free(pIesLocal);

	return cbRSNIe;
}
#endif

#ifdef FEATURE_WLAN_WAPI
/**
 * csr_get_wapi_information() - to get WAPI information
 * @mac_ctx: pointer to global MAC context
 * @auth_type: auth type
 * @encr_type: encryption type
 * @mc_encryption: multicast encryption type
 * @wapi_ie: pointer to WAPI IE
 * @ucast_cipher: Unicast cipher
 * @mcast_cipher: Multicast cipher
 * @auth_suite: Authentication suite
 * @negotiated_authtype: Negotiated auth type
 * @negotiated_mccipher: negotiated multicast cipher
 *
 * This routine will get all WAPI information
 *
 * Return: bool
 */
static bool csr_get_wapi_information(struct mac_context *mac_ctx,
				     tCsrAuthList *auth_type,
				     eCsrEncryptionType encr_type,
				     tCsrEncryptionList *mc_encryption,
				     tDot11fIEWAPI *wapi_ie,
				     uint8_t *ucast_cipher,
				     uint8_t *mcast_cipher, uint8_t *auth_suite,
				     eCsrAuthType *negotiated_authtype,
				     eCsrEncryptionType *negotiated_mccipher)
{
	bool acceptable_cipher = false;
	uint8_t c_ucast_cipher = 0;
	uint8_t c_mcast_cipher = 0;
	uint8_t c_auth_suites = 0, i;
	uint8_t unicast[CSR_WAPI_OUI_SIZE];
	uint8_t multicast[CSR_WAPI_OUI_SIZE];
	uint8_t authsuites[CSR_WAPI_MAX_AUTH_SUITES][CSR_WAPI_OUI_SIZE];
	uint8_t authentication[CSR_WAPI_OUI_SIZE];
	uint8_t mccipher_arr[CSR_WAPI_MAX_MULTICAST_CYPHERS][CSR_WAPI_OUI_SIZE];
	eCsrAuthType neg_authtype = eCSR_AUTH_TYPE_UNKNOWN;
	uint8_t wapioui_idx = 0;

	if (!wapi_ie->present)
		goto end;

	c_mcast_cipher++;
	qdf_mem_copy(mccipher_arr, wapi_ie->multicast_cipher_suite,
			CSR_WAPI_OUI_SIZE);
	c_ucast_cipher = (uint8_t) (wapi_ie->unicast_cipher_suite_count);
	c_auth_suites = (uint8_t) (wapi_ie->akm_suite_count);
	for (i = 0; i < c_auth_suites && i < CSR_WAPI_MAX_AUTH_SUITES; i++)
		qdf_mem_copy((void *)&authsuites[i],
			(void *)&wapi_ie->akm_suites[i], CSR_WAPI_OUI_SIZE);

	wapioui_idx = csr_get_oui_index_from_cipher(encr_type);
	if (wapioui_idx >= CSR_OUI_WAPI_WAI_MAX_INDEX) {
		sme_err("Wapi OUI index = %d out of limit",
			wapioui_idx);
		acceptable_cipher = false;
		goto end;
	}
	/* Check - Is requested unicast Cipher supported by the BSS. */
	acceptable_cipher = csr_match_wapi_oui_index(mac_ctx,
				wapi_ie->unicast_cipher_suites,
				c_ucast_cipher, wapioui_idx, unicast);
	if (!acceptable_cipher)
		goto end;

	/* unicast is supported. Pick the first matching Group cipher, if any */
	for (i = 0; i < mc_encryption->numEntries; i++) {
		wapioui_idx = csr_get_oui_index_from_cipher(
					mc_encryption->encryptionType[i]);
		if (wapioui_idx >= CSR_OUI_WAPI_WAI_MAX_INDEX) {
			sme_err("Wapi OUI index = %d out of limit",
				wapioui_idx);
			acceptable_cipher = false;
			break;
		}
		acceptable_cipher = csr_match_wapi_oui_index(mac_ctx,
						mccipher_arr, c_mcast_cipher,
						wapioui_idx, multicast);
		if (acceptable_cipher)
			break;
	}
	if (!acceptable_cipher)
		goto end;

	if (negotiated_mccipher)
		*negotiated_mccipher =
			mc_encryption->encryptionType[i];

	/*
	 * Ciphers are supported, Match authentication algorithm and
	 * pick first matching authtype
	 */
	if (csr_is_auth_wapi_cert
			(mac_ctx, authsuites, c_auth_suites, authentication)) {
		neg_authtype =
			eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	} else if (csr_is_auth_wapi_psk(mac_ctx, authsuites,
				c_auth_suites, authentication)) {
		neg_authtype = eCSR_AUTH_TYPE_WAPI_WAI_PSK;
	} else {
		acceptable_cipher = false;
		neg_authtype = eCSR_AUTH_TYPE_UNKNOWN;
	}

	/* Caller doesn't care about auth type, or BSS doesn't match */
	if ((0 == auth_type->numEntries) || (false == acceptable_cipher))
		goto end;

	acceptable_cipher = false;
	for (i = 0; i < auth_type->numEntries; i++) {
		if (auth_type->authType[i] == neg_authtype) {
			acceptable_cipher = true;
			break;
		}
	}

end:
	if (acceptable_cipher) {
		if (mcast_cipher)
			qdf_mem_copy(mcast_cipher, multicast,
					CSR_WAPI_OUI_SIZE);
		if (ucast_cipher)
			qdf_mem_copy(ucast_cipher, unicast, CSR_WAPI_OUI_SIZE);
		if (auth_suite)
			qdf_mem_copy(auth_suite, authentication,
					CSR_WAPI_OUI_SIZE);
		if (negotiated_authtype)
			*negotiated_authtype = neg_authtype;
	}
	return acceptable_cipher;
}

static bool csr_is_wapi_match(struct mac_context *mac_ctx, tCsrAuthList *pAuthType,
			      eCsrEncryptionType enType,
			      tCsrEncryptionList *pEnMcType,
			      tDot11fBeaconIEs *pIes,
			      eCsrAuthType *pNegotiatedAuthType,
			      eCsrEncryptionType *pNegotiatedMCCipher)
{
	bool fWapiMatch = false;

	/* See if the cyphers in the Bss description match with the
	 * settings in the profile.
	 */
	fWapiMatch =
		csr_get_wapi_information(mac_ctx, pAuthType, enType, pEnMcType,
					 &pIes->WAPI, NULL, NULL, NULL,
					 pNegotiatedAuthType,
					 pNegotiatedMCCipher);

	return fWapiMatch;
}

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
static bool csr_lookup_bkid(struct mac_context *mac, uint32_t sessionId,
			    uint8_t *pBSSId, uint8_t *pBKId)
{
	bool fRC = false, fMatchFound = false;
	uint32_t Index;
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return false;
	}

	do {
		for (Index = 0; Index < pSession->NumBkidCache; Index++) {
			sme_debug("match BKID " MAC_ADDRESS_STR " to ",
				MAC_ADDR_ARRAY(pBSSId));
			if (!qdf_mem_cmp
			    (pBSSId, pSession->BkidCacheInfo[Index].BSSID.bytes,
				    sizeof(struct qdf_mac_addr))) {
				/* match found */
				fMatchFound = true;
				break;
			}
		}

		if (!fMatchFound)
			break;

		qdf_mem_copy(pBKId, pSession->BkidCacheInfo[Index].BKID,
			     CSR_WAPI_BKID_SIZE);

		fRC = true;
	} while (0);
	sme_debug(
		"return match = %d mac->roam.NumBkidCache = %d",
		fRC, pSession->NumBkidCache);

	return fRC;
}
#endif

#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
uint8_t csr_construct_wapi_ie(struct mac_context *mac, uint32_t sessionId,
			      struct csr_roam_profile *pProfile,
			      tSirBssDescription *pSirBssDesc,
			      tDot11fBeaconIEs *pIes, tCsrWapiIe *pWapiIe)
{
	struct wlan_objmgr_vdev *vdev;
	uint8_t *wapi_ie_end = NULL;
	uint8_t *wapi_ie = (uint8_t *)pWapiIe;
	uint8_t ie_len = 0;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("Invalid vdev");
		return ie_len;
	}
	wapi_ie_end = wlan_crypto_build_wapiie(vdev, wapi_ie);
	if (wapi_ie_end)
		ie_len = wapi_ie_end - wapi_ie;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return ie_len;
}
#else
uint8_t csr_construct_wapi_ie(struct mac_context *mac, uint32_t sessionId,
			      struct csr_roam_profile *pProfile,
			      tSirBssDescription *pSirBssDesc,
			      tDot11fBeaconIEs *pIes, tCsrWapiIe *pWapiIe)
{
	bool fWapiMatch = false;
	uint8_t cbWapiIe = 0;
	uint8_t UnicastCypher[CSR_WAPI_OUI_SIZE];
	uint8_t MulticastCypher[CSR_WAPI_OUI_SIZE];
	uint8_t AuthSuite[CSR_WAPI_OUI_SIZE];
	uint8_t BKId[CSR_WAPI_BKID_SIZE];
	uint8_t *pWapi = NULL;
	bool fBKIDFound = false;
	tDot11fBeaconIEs *pIesLocal = pIes;

	do {
		if (!csr_is_profile_wapi(pProfile))
			break;

		if (!pIesLocal
		    &&
		    (!QDF_IS_STATUS_SUCCESS
			     (csr_get_parsed_bss_description_ies
				     (mac, pSirBssDesc, &pIesLocal)))) {
			break;
		}
		/* See if the cyphers in the Bss description match with the
		 * settings in the profile.
		 */
		fWapiMatch =
			csr_get_wapi_information(mac, &pProfile->AuthType,
					pProfile->negotiatedUCEncryptionType,
					&pProfile->mcEncryptionType,
					&pIesLocal->WAPI, UnicastCypher,
					MulticastCypher, AuthSuite, NULL,
						 NULL);
		if (!fWapiMatch)
			break;

		qdf_mem_zero(pWapiIe, sizeof(tCsrWapiIe));

		pWapiIe->IeHeader.ElementID = DOT11F_EID_WAPI;

		pWapiIe->Version = CSR_WAPI_VERSION_SUPPORTED;

		pWapiIe->cAuthenticationSuites = 1;
		qdf_mem_copy(&pWapiIe->AuthOui[0], AuthSuite,
			     sizeof(AuthSuite));

		pWapi = (uint8_t *) (&pWapiIe->AuthOui[1]);

		*pWapi = (uint16_t) 1;  /* cUnicastCyphers */
		pWapi += 2;
		qdf_mem_copy(pWapi, UnicastCypher, sizeof(UnicastCypher));
		pWapi += sizeof(UnicastCypher);

		qdf_mem_copy(pWapi, MulticastCypher, sizeof(MulticastCypher));
		pWapi += sizeof(MulticastCypher);

		/* WAPI capabilities follows the Auth Suite (two octects)
		 * we shouldn't EVER be sending out "pre-auth supported".
		 * It is an AP only capability & since we already did a memset
		 * pWapiIe to 0, skip these fields
		 */
		pWapi += 2;

		fBKIDFound =
			csr_lookup_bkid(mac, sessionId, pSirBssDesc->bssId,
					&(BKId[0]));

		if (fBKIDFound) {
			/* Do we need to change the endianness here */
			*pWapi = (uint16_t) 1;  /* cBKIDs */
			pWapi += 2;
			qdf_mem_copy(pWapi, BKId, CSR_WAPI_BKID_SIZE);
		} else {
			*pWapi = 0;
			pWapi += 1;
			*pWapi = 0;
			pWapi += 1;
		}

		/* Add in the IE fields except the IE header */
		/* Add BKID count and BKID (if any) */
		pWapiIe->IeHeader.Length =
			(uint8_t) (sizeof(*pWapiIe) -
				sizeof(pWapiIe->IeHeader));

		/*2 bytes for BKID Count field */
		pWapiIe->IeHeader.Length += sizeof(uint16_t);

		if (fBKIDFound)
			pWapiIe->IeHeader.Length += CSR_WAPI_BKID_SIZE;

		/* return the size of the IE header (total) constructed... */
		cbWapiIe = pWapiIe->IeHeader.Length + sizeof(pWapiIe->IeHeader);

	} while (0);

	if (!pIes && pIesLocal)
		/* locally allocated */
		qdf_mem_free(pIesLocal);

	return cbWapiIe;
}
#endif
#endif /* FEATURE_WLAN_WAPI */

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
/**
 * csr_get_wpa_cyphers() - to get WPA cipher info
 * @mac_ctx: pointer to mac context
 * @auth_type: auth type
 * @encr_type: encryption type
 * @mc_encryption: multicast encryption type
 * @wpa_ie: pointer to WPA IE
 * @ucast_cipher: Unicast cipher
 * @mcast_cipher: Multicast cipher
 * @auth_suite: Authentication suite
 * @negotiated_authtype: Negotiated auth type
 * @negotiated_mccipher: negotiated multicast cipher
 *
 * This routine will get all WPA information
 *
 * Return: bool
 */
static bool csr_get_wpa_cyphers(struct mac_context *mac_ctx, tCsrAuthList *auth_type,
				eCsrEncryptionType encr_type,
				tCsrEncryptionList *mc_encryption,
				tDot11fIEWPA *wpa_ie, uint8_t *ucast_cipher,
				uint8_t *mcast_cipher, uint8_t *auth_suite,
				eCsrAuthType *negotiated_authtype,
				eCsrEncryptionType *negotiated_mccipher)
{
	bool acceptable_cipher = false;
	uint8_t c_ucast_cipher = 0;
	uint8_t c_mcast_cipher = 0;
	uint8_t c_auth_suites = 0;
	uint8_t unicast[CSR_WPA_OUI_SIZE];
	uint8_t multicast[CSR_WPA_OUI_SIZE];
	uint8_t authentication[CSR_WPA_OUI_SIZE];
	uint8_t mccipher_arr[1][CSR_WPA_OUI_SIZE];
	uint8_t i;
	uint8_t index;
	eCsrAuthType neg_authtype = eCSR_AUTH_TYPE_UNKNOWN;

	if (!wpa_ie->present)
		goto end;
	c_mcast_cipher = 1;
	qdf_mem_copy(mccipher_arr, wpa_ie->multicast_cipher, CSR_WPA_OUI_SIZE);
	c_ucast_cipher = (uint8_t) (wpa_ie->unicast_cipher_count);
	c_auth_suites = (uint8_t) (wpa_ie->auth_suite_count);

	/*
	 * csr_match_wpaoui_index will provide the index of the
	 * array csr_wpa_oui to be read and determine if it is
	 * accepatable cipher or not. Below check ensures that
	 * the index will not be out of range of the array size.
	 */
	index = csr_get_oui_index_from_cipher(encr_type);
	if (!(index < (sizeof(csr_wpa_oui)/CSR_WPA_OUI_SIZE))) {
		sme_debug("Unacceptable index: %d", index);
		goto end;
	}

	sme_debug("kw_dbg: index: %d", index);
	/* Check - Is requested unicast Cipher supported by the BSS. */
	acceptable_cipher = csr_match_wpaoui_index(mac_ctx,
				wpa_ie->unicast_ciphers, c_ucast_cipher,
				index, unicast);
	if (!acceptable_cipher)
		goto end;
	/* unicast is supported. Pick the first matching Group cipher, if any */
	for (i = 0; i < mc_encryption->numEntries; i++) {
		index = csr_get_oui_index_from_cipher(
				mc_encryption->encryptionType[i]);
		sme_debug("kw_dbg: index: %d", index);
		if (!(index < (sizeof(csr_wpa_oui)/CSR_WPA_OUI_SIZE))) {
			sme_debug("Unacceptable MC index: %d", index);
			acceptable_cipher = false;
			continue;
		}
		acceptable_cipher = csr_match_wpaoui_index(mac_ctx,
					mccipher_arr, c_mcast_cipher,
					index, multicast);
		if (acceptable_cipher)
			break;
	}
	if (!acceptable_cipher)
		goto end;

	if (negotiated_mccipher)
		*negotiated_mccipher = mc_encryption->encryptionType[i];

	/* Initializing with false as it has true value already */
	acceptable_cipher = false;
	for (i = 0; i < auth_type->numEntries; i++) {
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype
		 */
		if (csr_is_auth_wpa(mac_ctx, wpa_ie->auth_suites, c_auth_suites,
			authentication)) {
			if (eCSR_AUTH_TYPE_WPA == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_WPA;
		}
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN) &&
			csr_is_auth_wpa_psk(mac_ctx,
				wpa_ie->auth_suites, c_auth_suites,
				authentication)) {
			if (eCSR_AUTH_TYPE_WPA_PSK == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_WPA_PSK;
		}
#ifdef FEATURE_WLAN_ESE
		if ((neg_authtype == eCSR_AUTH_TYPE_UNKNOWN)
			&& csr_is_ese_cckm_auth_wpa(mac_ctx,
				wpa_ie->auth_suites, c_auth_suites,
				authentication)) {
			if (eCSR_AUTH_TYPE_CCKM_WPA == auth_type->authType[i])
				neg_authtype = eCSR_AUTH_TYPE_CCKM_WPA;
		}
#endif /* FEATURE_WLAN_ESE */

		/*
		 * The 1st auth type in the APs WPA IE, to match stations
		 * connecting profiles auth type will cause us to exit this
		 * loop. This is added as some APs advertise multiple akms in
		 * the WPA IE
		 */
		if (eCSR_AUTH_TYPE_UNKNOWN != neg_authtype) {
			acceptable_cipher = true;
			break;
		}
	}

end:
	if (acceptable_cipher) {
		if (mcast_cipher)
			qdf_mem_copy((uint8_t **) mcast_cipher, multicast,
					CSR_WPA_OUI_SIZE);

		if (ucast_cipher)
			qdf_mem_copy((uint8_t **) ucast_cipher, unicast,
					CSR_WPA_OUI_SIZE);

		if (auth_suite)
			qdf_mem_copy((uint8_t **) auth_suite, authentication,
					CSR_WPA_OUI_SIZE);

		if (negotiated_authtype)
			*negotiated_authtype = neg_authtype;
	}

	return acceptable_cipher;
}

static bool csr_is_wpa_encryption_match(struct mac_context *mac,
					tCsrAuthList *pAuthType,
					eCsrEncryptionType enType,
					tCsrEncryptionList *pEnMcType,
					tDot11fBeaconIEs *pIes,
					eCsrAuthType *pNegotiatedAuthtype,
					eCsrEncryptionType *pNegotiatedMCCipher)
{
	bool fWpaMatch = false;

	/* See if the cyphers in the Bss description match with the
	 * settings in the profile.
	 */
	fWpaMatch =
		csr_get_wpa_cyphers(mac, pAuthType, enType, pEnMcType,
				&pIes->WPA,
				    NULL, NULL, NULL, pNegotiatedAuthtype,
				    pNegotiatedMCCipher);

	return fWpaMatch;
}
#endif

#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
uint8_t csr_construct_wpa_ie(struct mac_context *mac, uint8_t session_id,
			     struct csr_roam_profile *pProfile,
			     tSirBssDescription *pSirBssDesc,
			     tDot11fBeaconIEs *pIes, tCsrWpaIe *pWpaIe)
{
	struct wlan_objmgr_vdev *vdev;
	uint8_t *wpa_ie_end = NULL;
	uint8_t *wpa_ie = (uint8_t *)pWpaIe;
	uint8_t ie_len = 0;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mac->psoc, session_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("Invalid vdev");
		return ie_len;
	}
	wpa_ie_end = wlan_crypto_build_wpaie(vdev, wpa_ie);
	if (wpa_ie_end)
		ie_len = wpa_ie_end - wpa_ie;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return ie_len;
}
#else
uint8_t csr_construct_wpa_ie(struct mac_context *mac, uint8_t session_id,
			     struct csr_roam_profile *pProfile,
			     tSirBssDescription *pSirBssDesc,
			     tDot11fBeaconIEs *pIes, tCsrWpaIe *pWpaIe)
{
	bool fWpaMatch;
	uint8_t cbWpaIe = 0;
	uint8_t UnicastCypher[CSR_WPA_OUI_SIZE];
	uint8_t MulticastCypher[CSR_WPA_OUI_SIZE];
	uint8_t AuthSuite[CSR_WPA_OUI_SIZE];
	tCsrWpaAuthIe *pAuthSuite;
	tDot11fBeaconIEs *pIesLocal = pIes;

	do {
		if (!csr_is_profile_wpa(pProfile))
			break;

		if (!pIesLocal
		    &&
		    (!QDF_IS_STATUS_SUCCESS
			     (csr_get_parsed_bss_description_ies
				     (mac, pSirBssDesc, &pIesLocal))))
			break;
		/* See if the cyphers in the Bss description match with the
		 * settings in the profile.
		 */
		fWpaMatch =
			csr_get_wpa_cyphers(mac, &pProfile->AuthType,
					   pProfile->negotiatedUCEncryptionType,
					    &pProfile->mcEncryptionType,
					    &pIesLocal->WPA, UnicastCypher,
					MulticastCypher, AuthSuite, NULL, NULL);
		if (!fWpaMatch)
			break;

		pWpaIe->IeHeader.ElementID = SIR_MAC_WPA_EID;

		qdf_mem_copy(pWpaIe->Oui, csr_wpa_oui[01], sizeof(pWpaIe->Oui));

		pWpaIe->Version = CSR_WPA_VERSION_SUPPORTED;

		qdf_mem_copy(pWpaIe->MulticastOui, MulticastCypher,
			     sizeof(MulticastCypher));

		pWpaIe->cUnicastCyphers = 1;

		qdf_mem_copy(&pWpaIe->UnicastOui[0], UnicastCypher,
			     sizeof(UnicastCypher));

		pAuthSuite =
			(tCsrWpaAuthIe *) (&pWpaIe->
					   UnicastOui[pWpaIe->cUnicastCyphers]);

		pAuthSuite->cAuthenticationSuites = 1;
		qdf_mem_copy(&pAuthSuite->AuthOui[0], AuthSuite,
			     sizeof(AuthSuite));

		/* The WPA capabilities follows the Auth Suite (two octects)-
		 * this field is optional, and we always "send" zero, so just
		 * remove it.  This is consistent with our assumptions in the
		 * frames compiler; c.f. bug 15234:
		 * http://gold.woodsidenet.com/bugzilla/show_bug.cgi?id=15234
		 * Add in the fixed fields plus 1 Unicast cypher, less the IE
		 * Header length Add in the size of the Auth suite (count plus
		 * a single OUI)
		 */
		pWpaIe->IeHeader.Length =
			sizeof(*pWpaIe) - sizeof(pWpaIe->IeHeader) +
			sizeof(*pAuthSuite);

		/* return the size of the IE header (total) constructed... */
		cbWpaIe = pWpaIe->IeHeader.Length + sizeof(pWpaIe->IeHeader);

	} while (0);

	if (!pIes && pIesLocal)
		/* locally allocated */
		qdf_mem_free(pIesLocal);

	return cbWpaIe;
}
#endif

/* If a WPAIE exists in the profile, just use it. Or else construct
 * one from the BSS Caller allocated memory for pWpaIe and guarrantee
 * it can contain a max length WPA IE
 */
uint8_t csr_retrieve_wpa_ie(struct mac_context *mac, uint8_t session_id,
			    struct csr_roam_profile *pProfile,
			    tSirBssDescription *pSirBssDesc,
			    tDot11fBeaconIEs *pIes, tCsrWpaIe *pWpaIe)
{
	uint8_t cbWpaIe = 0;

	do {
		if (!csr_is_profile_wpa(pProfile))
			break;
		if (pProfile->nWPAReqIELength && pProfile->pWPAReqIE) {
			if (pProfile->nWPAReqIELength <=
					DOT11F_IE_RSN_MAX_LEN) {
				cbWpaIe = (uint8_t) pProfile->nWPAReqIELength;
				qdf_mem_copy(pWpaIe, pProfile->pWPAReqIE,
					     cbWpaIe);
			} else {
				sme_warn("Invalid WPA IE length %d",
					 pProfile->nWPAReqIELength);
			}
			break;
		}
		cbWpaIe = csr_construct_wpa_ie(mac, session_id, pProfile,
					       pSirBssDesc, pIes, pWpaIe);
	} while (0);

	return cbWpaIe;
}

#ifdef WLAN_FEATURE_11W
/**
 * csr_get_mc_mgmt_cipher(): Get mcast management cipher from profile rsn
 * @mac: mac ctx
 * @profile: connect profile
 * @bss: ap scan entry
 * @ap_ie: AP IE's
 *
 * Return: none
 */
static void csr_get_mc_mgmt_cipher(struct mac_context *mac,
				   struct csr_roam_profile *profile,
				   tSirBssDescription *bss,
				   tDot11fBeaconIEs *ap_ie)
{
	int ret;
	tDot11fIERSN rsn_ie = {0};
	uint8_t n_mgmt_cipher = 1;
	struct rsn_caps rsn_caps;
	tDot11fBeaconIEs *local_ap_ie = ap_ie;
	uint8_t grp_mgmt_arr[CSR_RSN_MAX_MULTICAST_CYPHERS][CSR_RSN_OUI_SIZE];

	if (!profile->MFPEnabled)
		return;

	if (!local_ap_ie &&
	    (!QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies
				    (mac, bss, &local_ap_ie))))
		return;

	qdf_mem_copy(&rsn_caps, local_ap_ie->RSN.RSN_Cap, sizeof(rsn_caps));

	if (!ap_ie && local_ap_ie)
		/* locally allocated */
		qdf_mem_free(local_ap_ie);

	/* if AP is not PMF capable return */
	if (!rsn_caps.MFPCapable)
		return;

	ret = dot11f_unpack_ie_rsn(mac, profile->pRSNReqIE + 2,
				   profile->nRSNReqIELength -2,
				   &rsn_ie, false);
	if (DOT11F_FAILED(ret))
		return;

	qdf_mem_copy(&rsn_caps, rsn_ie.RSN_Cap, sizeof(rsn_caps));

	/* if self cap is not PMF capable return */
	if (!rsn_caps.MFPCapable)
		return;

	qdf_mem_copy(grp_mgmt_arr, rsn_ie.gp_mgmt_cipher_suite,
		     CSR_RSN_OUI_SIZE);
	if (csr_is_group_mgmt_gmac_128(mac, grp_mgmt_arr, n_mgmt_cipher, NULL))
		profile->mgmt_encryption_type = eSIR_ED_AES_GMAC_128;
	else if (csr_is_group_mgmt_gmac_256(mac, grp_mgmt_arr,
		 n_mgmt_cipher, NULL))
		profile->mgmt_encryption_type = eSIR_ED_AES_GMAC_256;
	else
		/* Default is CMAC */
		profile->mgmt_encryption_type = eSIR_ED_AES_128_CMAC;
}
#else
static inline
void csr_get_mc_mgmt_cipher(struct mac_context *mac,
			    struct csr_roam_profile *profile,
			    tSirBssDescription *bss,
			    tDot11fBeaconIEs *ap_ie)
{
}
#endif
/* If a RSNIE exists in the profile, just use it. Or else construct
 * one from the BSS Caller allocated memory for pWpaIe and guarrantee
 * it can contain a max length WPA IE
 */
uint8_t csr_retrieve_rsn_ie(struct mac_context *mac, uint32_t sessionId,
			    struct csr_roam_profile *pProfile,
			    tSirBssDescription *pSirBssDesc,
			    tDot11fBeaconIEs *pIes, tCsrRSNIe *pRsnIe)
{
	uint8_t cbRsnIe = 0;

	do {
		if (!csr_is_profile_rsn(pProfile))
			break;
		/* copy RSNIE from user as it is if test mode is enabled */
		if (pProfile->force_rsne_override &&
		    pProfile->nRSNReqIELength && pProfile->pRSNReqIE) {
			sme_debug("force_rsne_override, copy RSN IE provided by user");
			if (pProfile->nRSNReqIELength <=
					DOT11F_IE_RSN_MAX_LEN) {
				cbRsnIe = (uint8_t) pProfile->nRSNReqIELength;
				qdf_mem_copy(pRsnIe, pProfile->pRSNReqIE,
					     cbRsnIe);
				csr_get_mc_mgmt_cipher(mac, pProfile,
						       pSirBssDesc, pIes);
			} else {
				sme_warn("Invalid RSN IE length: %d",
					 pProfile->nRSNReqIELength);
			}
			break;
		}
		cbRsnIe = csr_construct_rsn_ie(mac, sessionId, pProfile,
					       pSirBssDesc, pIes, pRsnIe);
	} while (0);

	return cbRsnIe;
}

#ifdef FEATURE_WLAN_WAPI
/* If a WAPI IE exists in the profile, just use it. Or else construct
 * one from the BSS Caller allocated memory for pWapiIe and guarrantee
 * it can contain a max length WAPI IE
 */
uint8_t csr_retrieve_wapi_ie(struct mac_context *mac, uint32_t sessionId,
			     struct csr_roam_profile *pProfile,
			     tSirBssDescription *pSirBssDesc,
			     tDot11fBeaconIEs *pIes, tCsrWapiIe *pWapiIe)
{
	uint8_t cbWapiIe = 0;

	do {
		if (!csr_is_profile_wapi(pProfile))
			break;
		if (pProfile->nWAPIReqIELength && pProfile->pWAPIReqIE) {
			if (DOT11F_IE_WAPI_MAX_LEN >=
			    pProfile->nWAPIReqIELength) {
				cbWapiIe = (uint8_t) pProfile->nWAPIReqIELength;
				qdf_mem_copy(pWapiIe, pProfile->pWAPIReqIE,
					     cbWapiIe);
			} else {
				sme_warn("Invalid WAPI IE length %d",
					 pProfile->nWAPIReqIELength);
			}
			break;
		}
		cbWapiIe = csr_construct_wapi_ie(mac, sessionId,
						 pProfile, pSirBssDesc,
						 pIes, pWapiIe);
	} while (0);

	return cbWapiIe;
}
#endif /* FEATURE_WLAN_WAPI */

bool csr_rates_is_dot11_rate11b_supported_rate(uint8_t dot11Rate)
{
	bool fSupported = false;
	uint16_t nonBasicRate =
		(uint16_t) (BITS_OFF(dot11Rate, CSR_DOT11_BASIC_RATE_MASK));

	switch (nonBasicRate) {
	case eCsrSuppRate_1Mbps:
	case eCsrSuppRate_2Mbps:
	case eCsrSuppRate_5_5Mbps:
	case eCsrSuppRate_11Mbps:
		fSupported = true;
		break;

	default:
		break;
	}

	return fSupported;
}

bool csr_rates_is_dot11_rate11a_supported_rate(uint8_t dot11Rate)
{
	bool fSupported = false;
	uint16_t nonBasicRate =
		(uint16_t) (BITS_OFF(dot11Rate, CSR_DOT11_BASIC_RATE_MASK));

	switch (nonBasicRate) {
	case eCsrSuppRate_6Mbps:
	case eCsrSuppRate_9Mbps:
	case eCsrSuppRate_12Mbps:
	case eCsrSuppRate_18Mbps:
	case eCsrSuppRate_24Mbps:
	case eCsrSuppRate_36Mbps:
	case eCsrSuppRate_48Mbps:
	case eCsrSuppRate_54Mbps:
		fSupported = true;
		break;

	default:
		break;
	}

	return fSupported;
}

tAniEdType csr_translate_encrypt_type_to_ed_type(eCsrEncryptionType EncryptType)
{
	tAniEdType edType;

	switch (EncryptType) {
	default:
	case eCSR_ENCRYPT_TYPE_NONE:
		edType = eSIR_ED_NONE;
		break;

	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP40:
		edType = eSIR_ED_WEP40;
		break;

	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP104:
		edType = eSIR_ED_WEP104;
		break;

	case eCSR_ENCRYPT_TYPE_TKIP:
		edType = eSIR_ED_TKIP;
		break;

	case eCSR_ENCRYPT_TYPE_AES:
		edType = eSIR_ED_CCMP;
		break;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_ENCRYPT_TYPE_WPI:
		edType = eSIR_ED_WPI;
		break;
#endif
#ifdef WLAN_FEATURE_11W
	/* 11w BIP */
	case eCSR_ENCRYPT_TYPE_AES_CMAC:
		edType = eSIR_ED_AES_128_CMAC;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GCMP:
		edType = eSIR_ED_GCMP;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
		edType = eSIR_ED_GCMP_256;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GMAC_128:
		edType = eSIR_ED_AES_GMAC_128;
		break;
	case eCSR_ENCRYPT_TYPE_AES_GMAC_256:
		edType = eSIR_ED_AES_GMAC_256;
		break;
#endif
	}

	return edType;
}

/**
 * csr_validate_wep() - to validate wep
 * @uc_encry_type: unicast encryption type
 * @auth_list: Auth list
 * @mc_encryption_list: multicast encryption type
 * @negotiated_authtype: negotiated auth type
 * @negotiated_mc_encry: negotiated mc encry type
 * @bss_descr: BSS description
 * @ie_ptr: IE pointer
 *
 * This function just checks whether HDD is giving correct values for
 * Multicast cipher and Auth
 *
 * Return: bool
 */
static bool csr_validate_wep(struct mac_context *mac_ctx,
			     eCsrEncryptionType uc_encry_type,
			     tCsrAuthList *auth_list,
			     tCsrEncryptionList *mc_encryption_list,
			     tSirBssDescription *bss_descr,
			     tDot11fBeaconIEs *ie_ptr)
{
	uint32_t idx;
	bool match = false;
	uint8_t oui_index;

	/* If privacy bit is not set, consider no match */
	if (!csr_is_privacy(bss_descr))
		return match;

	for (idx = 0; idx < mc_encryption_list->numEntries; idx++) {
		switch (mc_encryption_list->encryptionType[idx]) {
		case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
		case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
		case eCSR_ENCRYPT_TYPE_WEP40:
		case eCSR_ENCRYPT_TYPE_WEP104:
			/*
			 * Multicast list may contain WEP40/WEP104.
			 * Check whether it matches UC.
			 */
			if (uc_encry_type ==
				mc_encryption_list->encryptionType[idx]) {
				match = true;
			}
			break;
		default:
			match = false;
			break;
		}
		if (match)
			break;
	}

	if (!match)
		return match;

	for (idx = 0; idx < auth_list->numEntries; idx++) {
		switch (auth_list->authType[idx]) {
		case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		case eCSR_AUTH_TYPE_SHARED_KEY:
		case eCSR_AUTH_TYPE_AUTOSWITCH:
			match = true;
			break;
		default:
			match = false;
		}
		if (match)
			break;
	}

	if (!match)
		return match;

	if (!ie_ptr)
		return match;

	/*
	 * In case of WPA / WPA2, check whether it supports WEP as well.
	 * Prepare the encryption type for WPA/WPA2 functions
	 */
	if (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == uc_encry_type)
		uc_encry_type = eCSR_ENCRYPT_TYPE_WEP40;
	else if (eCSR_ENCRYPT_TYPE_WEP104 == uc_encry_type)
		uc_encry_type = eCSR_ENCRYPT_TYPE_WEP104;

	/* else we can use the encryption type directly */
	if (ie_ptr->WPA.present) {
		oui_index = csr_get_oui_index_from_cipher(uc_encry_type);
		if (oui_index < QDF_ARRAY_SIZE(csr_wpa_oui))
			match = (!qdf_mem_cmp(ie_ptr->WPA.multicast_cipher,
					csr_wpa_oui[oui_index],
					CSR_WPA_OUI_SIZE));
		if (match)
			return match;
	}
	if (ie_ptr->RSN.present) {
		match = (!qdf_mem_cmp(ie_ptr->RSN.gp_cipher_suite,
				csr_rsn_oui[csr_get_oui_index_from_cipher(
					uc_encry_type)],
				CSR_RSN_OUI_SIZE));
	}

	return match;
}

#ifndef WLAN_CONV_CRYPTO_IE_SUPPORT
/**
 * csr_validate_open_none() - Check if the security is matching
 * @bss_desc:          BSS Descriptor on which the check is done
 * @mc_enc_type:       Multicast encryption type
 * @mc_cipher:         Multicast Cipher
 * @auth_type:         Authentication type
 * @neg_auth_type:     Negotiated Auth type with the AP
 *
 * Return: Boolean value to tell if matched or not.
 */
static bool csr_validate_open_none(tSirBssDescription *bss_desc,
				   tCsrEncryptionList *mc_enc_type,
				   tCsrAuthList *auth_type)
{
	bool match;
	uint8_t idx;

	/*
	 * for NO encryption, if the Bss description has the
	 * Privacy bit turned on, then encryption is required
	 * so we have to reject this Bss.
	 */
	if (csr_is_privacy(bss_desc))
		match = false;
	else
		match = true;
	if (match) {
		match = false;
		/* Check MC cipher and Auth type requested. */
		for (idx = 0; idx < mc_enc_type->numEntries; idx++) {
			if (eCSR_ENCRYPT_TYPE_NONE ==
				mc_enc_type->encryptionType[idx]) {
				match = true;
				break;
			}
		}
		if (!match)
			return match;

		match = false;
		/* Check Auth list. It should contain AuthOpen. */
		for (idx = 0; idx < auth_type->numEntries; idx++) {
			if ((eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				auth_type->authType[idx]) ||
				(eCSR_AUTH_TYPE_AUTOSWITCH ==
				auth_type->authType[idx])) {
				match = true;
				break;
			}
		}
		if (!match)
			return match;

	}
	return match;
}
#endif
/**
 * csr_validate_any_default() - Check if the security is matching
 * @mac_ctx:           Global MAC context
 * @auth_type:         Authentication type
 * @mc_enc_type:       Multicast encryption type
 * @mfp_enabled:       Management frame protection feature
 * @mfp_required:      Management frame protection mandatory
 * @mfp_capable:       Device capable of MFP
 * @ies_ptr:           Pointer to the IE fields
 * @neg_auth_type:     Negotiated Auth type with the AP
 * @bss_desc:          BSS Descriptor
 * @neg_uc_cipher:     Negotiated unicast cipher suite
 * @neg_mc_cipher:     Negotiated multicast cipher
 *
 * Return: Boolean value to tell if matched or not.
 */
static bool csr_validate_any_default(struct mac_context *mac_ctx,
				     tCsrAuthList *auth_type,
				     tCsrEncryptionList *mc_enc_type,
				     bool *mfp_enabled,
				     uint8_t *mfp_required,
				     uint8_t *mfp_capable,
				     tDot11fBeaconIEs *ies_ptr,
				     eCsrAuthType *neg_auth_type,
				     tSirBssDescription *bss_desc,
				     eCsrEncryptionType *uc_cipher,
				     eCsrEncryptionType *mc_cipher)
{
	bool match_any = false;
	bool match = true;
	/* It is allowed to match anything. Try the more secured ones first. */
	if (ies_ptr) {
		/* Check GCMP-256 first */
		*uc_cipher = eCSR_ENCRYPT_TYPE_AES_GCMP_256;
		match_any = csr_is_rsn_match(mac_ctx, auth_type,
				*uc_cipher, mc_enc_type, mfp_enabled,
				mfp_required, mfp_capable, ies_ptr,
				neg_auth_type, mc_cipher);
		/* Check GCMP second */
		*uc_cipher = eCSR_ENCRYPT_TYPE_AES_GCMP;
		match_any = csr_is_rsn_match(mac_ctx, auth_type,
				*uc_cipher, mc_enc_type, mfp_enabled,
				mfp_required, mfp_capable, ies_ptr,
				neg_auth_type, mc_cipher);
		/* Check AES third */
		*uc_cipher = eCSR_ENCRYPT_TYPE_AES;
		match_any = csr_is_rsn_match(mac_ctx, auth_type,
				*uc_cipher, mc_enc_type, mfp_enabled,
				mfp_required, mfp_capable, ies_ptr,
				neg_auth_type, mc_cipher);
		if (!match_any) {
			/* Check TKIP */
			*uc_cipher = eCSR_ENCRYPT_TYPE_TKIP;
			match_any = csr_is_rsn_match(mac_ctx, auth_type,
					*uc_cipher,
					mc_enc_type, mfp_enabled, mfp_required,
					mfp_capable, ies_ptr, neg_auth_type,
					mc_cipher);
		}
#ifdef FEATURE_WLAN_WAPI
		if (!match_any) {
			/* Check WAPI */
			*uc_cipher = eCSR_ENCRYPT_TYPE_WPI;
			match_any = csr_is_wapi_match(mac_ctx, auth_type,
					*uc_cipher, mc_enc_type, ies_ptr,
					neg_auth_type, mc_cipher);
		}
#endif
	}
	if (match_any)
		return match;
	*uc_cipher = eCSR_ENCRYPT_TYPE_WEP104;
	if (csr_validate_wep(mac_ctx, *uc_cipher, auth_type, mc_enc_type,
			bss_desc, ies_ptr))
		return match;
	*uc_cipher = eCSR_ENCRYPT_TYPE_WEP40;
	if (csr_validate_wep(mac_ctx, *uc_cipher, auth_type, mc_enc_type,
			bss_desc, ies_ptr))
		return match;
	*uc_cipher = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
	if (csr_validate_wep(mac_ctx, *uc_cipher, auth_type, mc_enc_type,
			bss_desc, ies_ptr))
		return match;
	*uc_cipher = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
	if (csr_validate_wep(mac_ctx, *uc_cipher, auth_type, mc_enc_type,
			bss_desc, ies_ptr))
		return match;
	/* It must be open and no enc */
	if (csr_is_privacy(bss_desc)) {
		match = false;
		return match;
	}

	*neg_auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	*mc_cipher = eCSR_ENCRYPT_TYPE_NONE;
	*uc_cipher = eCSR_ENCRYPT_TYPE_NONE;
	return match;

}

/**
 * csr_is_security_match() - Check if the security is matching
 * @mac_ctx:           Global MAC context
 * @auth_type:         Authentication type
 * @uc_enc_type:       Unicast Encryption type
 * @mc_enc_type:       Multicast encryption type
 * @mfp_enabled:       Management frame protection feature
 * @mfp_required:      Management frame protection mandatory
 * @mfp_capable:       Device capable of MFP
 * @bss_desc:          BSS Descriptor
 * @ies_ptr:           Pointer to the IE fields
 * @session_id:        Session Id
 *
 * Return: Boolean value to tell if matched or not.
 */
bool csr_is_security_match(struct mac_context *mac_ctx, tCsrAuthList *auth_type,
			   tCsrEncryptionList *uc_enc_type,
			   tCsrEncryptionList *mc_enc_type, bool *mfp_enabled,
			   uint8_t *mfp_required, uint8_t *mfp_capable,
			   tSirBssDescription *bss_desc,
			   tDot11fBeaconIEs *ies_ptr, uint8_t session_id)
{
	bool match = false;
	uint8_t i;
	eCsrEncryptionType uc_cipher = eCSR_ENCRYPT_TYPE_UNKNOWN;
	uint16_t ie_len;

	ie_len = (bss_desc->length + sizeof(bss_desc->length) -
		  GET_FIELD_OFFSET(tSirBssDescription, ieFields));

	for (i = 0; ((i < uc_enc_type->numEntries) && (!match)); i++) {
		uc_cipher = uc_enc_type->encryptionType[i];
		/*
		 * If the Bss description shows the Privacy bit is on, then we
		 * must have some sort of encryption configured for the profile
		 * to work.  Don't attempt to join networks with Privacy bit
		 * set when profiles say NONE for encryption type.
		 */
		switch (uc_cipher) {
		case eCSR_ENCRYPT_TYPE_NONE:
#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
			if (csr_is_privacy(bss_desc))
				return false;

			match =	wlan_crypto_check_open_none(mac_ctx->psoc,
							    session_id);
#else
			match = csr_validate_open_none(bss_desc, mc_enc_type,
						       auth_type);
#endif
			break;

		case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
		case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
		case eCSR_ENCRYPT_TYPE_WEP40:
		case eCSR_ENCRYPT_TYPE_WEP104:
			/*
			 * !! might want to check for WEP keys set in the
			 * Profile.... ? !! don't need to have the privacy bit
			 * in the Bss description.  Many AP policies make
			 * legacy encryption 'optional' so we don't know if we
			 * can associate or not.  The AP will reject if
			 * encryption is not allowed without the Privacy bit
			 * turned on.
			 */
#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
			if (!csr_is_privacy(bss_desc))
				return false;

			match = wlan_crypto_check_wep(mac_ctx->psoc,
						      session_id);
#else
			match = csr_validate_wep(mac_ctx, uc_cipher, auth_type,
						 mc_enc_type, bss_desc,
						 ies_ptr);
#endif

			break;
		case eCSR_ENCRYPT_TYPE_TKIP:
		case eCSR_ENCRYPT_TYPE_AES:
		case eCSR_ENCRYPT_TYPE_AES_GCMP:
		case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
			if (!ies_ptr) {
				match = false;
				break;
			}
#ifdef WLAN_CONV_CRYPTO_IE_SUPPORT
			match = wlan_crypto_check_rsn_match(mac_ctx->psoc,
							    session_id,
							    (uint8_t *)
							    &bss_desc->ieFields,
							    ie_len);
			if (match)
				break;
			match = wlan_crypto_check_wpa_match(mac_ctx->psoc,
							    session_id,
							    (uint8_t *)
							    &bss_desc->ieFields,
							    ie_len);
#else
			/* First check if there is a RSN match */
			match = csr_is_rsn_match(mac_ctx, auth_type,
					uc_cipher, mc_enc_type,
					mfp_enabled, mfp_required,
					mfp_capable, ies_ptr,
					NULL, NULL);
			/* If not RSN, then check WPA match */
			if (!match)
				match = csr_is_wpa_encryption_match(
						mac_ctx, auth_type,
						uc_cipher, mc_enc_type,
						ies_ptr,
						NULL, NULL);
#endif
			break;
#ifdef FEATURE_WLAN_WAPI
		case eCSR_ENCRYPT_TYPE_WPI:     /* WAPI */
			if (ies_ptr)
				match = csr_is_wapi_match(mac_ctx, auth_type,
						uc_cipher, mc_enc_type, ies_ptr,
						NULL, NULL);
			else
				match = false;
			break;
#endif /* FEATURE_WLAN_WAPI */
		case eCSR_ENCRYPT_TYPE_ANY:
		default:
			match  = csr_validate_any_default(mac_ctx, auth_type,
					mc_enc_type, mfp_enabled, mfp_required,
					mfp_capable, ies_ptr,
					NULL, bss_desc,
					&uc_cipher, NULL);
			break;
		}

	}

	return match;
}

bool csr_is_ssid_match(struct mac_context *mac, uint8_t *ssid1, uint8_t ssid1Len,
		       uint8_t *bssSsid, uint8_t bssSsidLen, bool fSsidRequired)
{
	bool fMatch = false;

	do {
		/*
		 * Check for the specification of the Broadcast SSID at the
		 * beginning of the list. If specified, then all SSIDs are
		 * matches (broadcast SSID means accept all SSIDs).
		 */
		if (ssid1Len == 0) {
			fMatch = true;
			break;
		}

		/* There are a few special cases.  If the Bss description has
		 * a Broadcast SSID, then our Profile must have a single SSID
		 * without Wildcards so we can program the SSID.
		 *
		 * SSID could be suppressed in beacons. In that case SSID IE
		 * has valid length but the SSID value is all NULL characters.
		 * That condition is trated same as NULL SSID
		 */
		if (csr_is_nullssid(bssSsid, bssSsidLen)) {
			if (false == fSsidRequired) {
				fMatch = true;
				break;
			}
		}

		if (ssid1Len != bssSsidLen)
			break;
		if (!qdf_mem_cmp(bssSsid, ssid1, bssSsidLen)) {
			fMatch = true;
			break;
		}

	} while (0);

	return fMatch;
}

/* Null ssid means match */
bool csr_is_ssid_in_list(tSirMacSSid *pSsid, tCsrSSIDs *pSsidList)
{
	bool fMatch = false;
	uint32_t i;

	if (pSsidList && pSsid) {
		for (i = 0; i < pSsidList->numOfSSIDs; i++) {
			if (csr_is_nullssid
				    (pSsidList->SSIDList[i].SSID.ssId,
				    pSsidList->SSIDList[i].SSID.length)
			    ||
			    ((pSsidList->SSIDList[i].SSID.length ==
			      pSsid->length)
			     && (!qdf_mem_cmp(pSsid->ssId,
						pSsidList->SSIDList[i].SSID.
						ssId, pSsid->length)))) {
				fMatch = true;
				break;
			}
		}
	}

	return fMatch;
}

bool csr_is_bssid_match(struct qdf_mac_addr *pProfBssid,
			struct qdf_mac_addr *BssBssid)
{
	bool fMatch = false;
	struct qdf_mac_addr ProfileBssid;

	/* for efficiency of the MAC_ADDRESS functions, move the */
	/* Bssid's into MAC_ADDRESS structs. */
	qdf_mem_copy(&ProfileBssid, pProfBssid, sizeof(struct qdf_mac_addr));

	do {
		/* Give the profile the benefit of the doubt... accept
		 * either all 0 or the real broadcast Bssid (all 0xff)
		 * as broadcast Bssids (meaning to match any Bssids).
		 */
		if (qdf_is_macaddr_zero(&ProfileBssid) ||
		    qdf_is_macaddr_broadcast(&ProfileBssid)) {
			fMatch = true;
			break;
		}

		if (qdf_is_macaddr_equal(BssBssid, &ProfileBssid)) {
			fMatch = true;
			break;
		}

	} while (0);

	return fMatch;
}

bool csr_is_bss_type_ibss(eCsrRoamBssType bssType)
{
	return (bool)
		(eCSR_BSS_TYPE_START_IBSS == bssType
		 || eCSR_BSS_TYPE_IBSS == bssType);
}

/**
 * csr_is_aggregate_rate_supported() - to check if aggregate rate is supported
 * @mac_ctx: pointer to mac context
 * @rate: A rate in units of 500kbps
 *
 *
 * The rate encoding  is just as in 802.11  Information Elements, except
 * that the high bit is \em  not interpreted as indicating a Basic Rate,
 * and proprietary rates are allowed, too.
 *
 * Note  that if the  adapter's dot11Mode  is g,  we don't  restrict the
 * rates.  According to hwReadEepromParameters, this will happen when:
 * ... the  card is  configured for ALL  bands through  the property
 * page.  If this occurs, and the card is not an ABG card ,then this
 * code  is  setting the  dot11Mode  to  assume  the mode  that  the
 * hardware can support.   For example, if the card  is an 11BG card
 * and we  are configured to support  ALL bands, then  we change the
 * dot11Mode  to 11g  because  ALL in  this  case is  only what  the
 * hardware can support.
 *
 * Return: true if  the adapter is currently capable of supporting this rate
 */

static bool csr_is_aggregate_rate_supported(struct mac_context *mac_ctx,
			uint16_t rate)
{
	bool supported = false;
	uint16_t idx, new_rate;

	/* In case basic rate flag is set */
	new_rate = BITS_OFF(rate, CSR_DOT11_BASIC_RATE_MASK);
	if (eCSR_CFG_DOT11_MODE_11A ==
			mac_ctx->roam.configParam.uCfgDot11Mode) {
		switch (new_rate) {
		case eCsrSuppRate_6Mbps:
		case eCsrSuppRate_9Mbps:
		case eCsrSuppRate_12Mbps:
		case eCsrSuppRate_18Mbps:
		case eCsrSuppRate_24Mbps:
		case eCsrSuppRate_36Mbps:
		case eCsrSuppRate_48Mbps:
		case eCsrSuppRate_54Mbps:
			supported = true;
			break;
		default:
			supported = false;
			break;
		}

	} else if (eCSR_CFG_DOT11_MODE_11B ==
		   mac_ctx->roam.configParam.uCfgDot11Mode) {
		switch (new_rate) {
		case eCsrSuppRate_1Mbps:
		case eCsrSuppRate_2Mbps:
		case eCsrSuppRate_5_5Mbps:
		case eCsrSuppRate_11Mbps:
			supported = true;
			break;
		default:
			supported = false;
			break;
		}
	} else if (!mac_ctx->roam.configParam.ProprietaryRatesEnabled) {

		switch (new_rate) {
		case eCsrSuppRate_1Mbps:
		case eCsrSuppRate_2Mbps:
		case eCsrSuppRate_5_5Mbps:
		case eCsrSuppRate_6Mbps:
		case eCsrSuppRate_9Mbps:
		case eCsrSuppRate_11Mbps:
		case eCsrSuppRate_12Mbps:
		case eCsrSuppRate_18Mbps:
		case eCsrSuppRate_24Mbps:
		case eCsrSuppRate_36Mbps:
		case eCsrSuppRate_48Mbps:
		case eCsrSuppRate_54Mbps:
			supported = true;
			break;
		default:
			supported = false;
			break;
		}
	} else if (eCsrSuppRate_1Mbps == new_rate ||
			eCsrSuppRate_2Mbps == new_rate ||
			eCsrSuppRate_5_5Mbps == new_rate ||
			eCsrSuppRate_11Mbps == new_rate)
		supported = true;
	else {
		idx = 0x1;

		switch (new_rate) {
		case eCsrSuppRate_6Mbps:
			supported = g_phy_rates_suppt[0][idx];
			break;
		case eCsrSuppRate_9Mbps:
			supported = g_phy_rates_suppt[1][idx];
			break;
		case eCsrSuppRate_12Mbps:
			supported = g_phy_rates_suppt[2][idx];
			break;
		case eCsrSuppRate_18Mbps:
			supported = g_phy_rates_suppt[3][idx];
			break;
		case eCsrSuppRate_20Mbps:
			supported = g_phy_rates_suppt[4][idx];
			break;
		case eCsrSuppRate_24Mbps:
			supported = g_phy_rates_suppt[5][idx];
			break;
		case eCsrSuppRate_36Mbps:
			supported = g_phy_rates_suppt[6][idx];
			break;
		case eCsrSuppRate_40Mbps:
			supported = g_phy_rates_suppt[7][idx];
			break;
		case eCsrSuppRate_42Mbps:
			supported = g_phy_rates_suppt[8][idx];
			break;
		case eCsrSuppRate_48Mbps:
			supported = g_phy_rates_suppt[9][idx];
			break;
		case eCsrSuppRate_54Mbps:
			supported = g_phy_rates_suppt[10][idx];
			break;
		case eCsrSuppRate_72Mbps:
			supported = g_phy_rates_suppt[11][idx];
			break;
		case eCsrSuppRate_80Mbps:
			supported = g_phy_rates_suppt[12][idx];
			break;
		case eCsrSuppRate_84Mbps:
			supported = g_phy_rates_suppt[13][idx];
			break;
		case eCsrSuppRate_96Mbps:
			supported = g_phy_rates_suppt[14][idx];
			break;
		case eCsrSuppRate_108Mbps:
			supported = g_phy_rates_suppt[15][idx];
			break;
		case eCsrSuppRate_120Mbps:
			supported = g_phy_rates_suppt[16][idx];
			break;
		case eCsrSuppRate_126Mbps:
			supported = g_phy_rates_suppt[17][idx];
			break;
		case eCsrSuppRate_144Mbps:
			supported = g_phy_rates_suppt[18][idx];
			break;
		case eCsrSuppRate_160Mbps:
			supported = g_phy_rates_suppt[19][idx];
			break;
		case eCsrSuppRate_168Mbps:
			supported = g_phy_rates_suppt[20][idx];
			break;
		case eCsrSuppRate_192Mbps:
			supported = g_phy_rates_suppt[21][idx];
			break;
		case eCsrSuppRate_216Mbps:
			supported = g_phy_rates_suppt[22][idx];
			break;
		case eCsrSuppRate_240Mbps:
			supported = g_phy_rates_suppt[23][idx];
			break;
		default:
			supported = false;
			break;
		}
	}
	return supported;
}

void csr_add_rate_bitmap(uint8_t rate, uint16_t *pRateBitmap)
{
	uint16_t rateBitmap;
	uint16_t n = BITS_OFF(rate, CSR_DOT11_BASIC_RATE_MASK);

	rateBitmap = *pRateBitmap;
	switch (n) {
	case SIR_MAC_RATE_1:
		rateBitmap |= SIR_MAC_RATE_1_BITMAP;
		break;
	case SIR_MAC_RATE_2:
		rateBitmap |= SIR_MAC_RATE_2_BITMAP;
		break;
	case SIR_MAC_RATE_5_5:
		rateBitmap |= SIR_MAC_RATE_5_5_BITMAP;
		break;
	case SIR_MAC_RATE_11:
		rateBitmap |= SIR_MAC_RATE_11_BITMAP;
		break;
	case SIR_MAC_RATE_6:
		rateBitmap |= SIR_MAC_RATE_6_BITMAP;
		break;
	case SIR_MAC_RATE_9:
		rateBitmap |= SIR_MAC_RATE_9_BITMAP;
		break;
	case SIR_MAC_RATE_12:
		rateBitmap |= SIR_MAC_RATE_12_BITMAP;
		break;
	case SIR_MAC_RATE_18:
		rateBitmap |= SIR_MAC_RATE_18_BITMAP;
		break;
	case SIR_MAC_RATE_24:
		rateBitmap |= SIR_MAC_RATE_24_BITMAP;
		break;
	case SIR_MAC_RATE_36:
		rateBitmap |= SIR_MAC_RATE_36_BITMAP;
		break;
	case SIR_MAC_RATE_48:
		rateBitmap |= SIR_MAC_RATE_48_BITMAP;
		break;
	case SIR_MAC_RATE_54:
		rateBitmap |= SIR_MAC_RATE_54_BITMAP;
		break;
	}
	*pRateBitmap = rateBitmap;
}

bool csr_check_rate_bitmap(uint8_t rate, uint16_t rateBitmap)
{
	uint16_t n = BITS_OFF(rate, CSR_DOT11_BASIC_RATE_MASK);

	switch (n) {
	case SIR_MAC_RATE_1:
		rateBitmap &= SIR_MAC_RATE_1_BITMAP;
		break;
	case SIR_MAC_RATE_2:
		rateBitmap &= SIR_MAC_RATE_2_BITMAP;
		break;
	case SIR_MAC_RATE_5_5:
		rateBitmap &= SIR_MAC_RATE_5_5_BITMAP;
		break;
	case SIR_MAC_RATE_11:
		rateBitmap &= SIR_MAC_RATE_11_BITMAP;
		break;
	case SIR_MAC_RATE_6:
		rateBitmap &= SIR_MAC_RATE_6_BITMAP;
		break;
	case SIR_MAC_RATE_9:
		rateBitmap &= SIR_MAC_RATE_9_BITMAP;
		break;
	case SIR_MAC_RATE_12:
		rateBitmap &= SIR_MAC_RATE_12_BITMAP;
		break;
	case SIR_MAC_RATE_18:
		rateBitmap &= SIR_MAC_RATE_18_BITMAP;
		break;
	case SIR_MAC_RATE_24:
		rateBitmap &= SIR_MAC_RATE_24_BITMAP;
		break;
	case SIR_MAC_RATE_36:
		rateBitmap &= SIR_MAC_RATE_36_BITMAP;
		break;
	case SIR_MAC_RATE_48:
		rateBitmap &= SIR_MAC_RATE_48_BITMAP;
		break;
	case SIR_MAC_RATE_54:
		rateBitmap &= SIR_MAC_RATE_54_BITMAP;
		break;
	}
	return !!rateBitmap;
}

bool csr_rates_is_dot11_rate_supported(struct mac_context *mac_ctx, uint8_t rate)
{
	uint16_t n = BITS_OFF(rate, CSR_DOT11_BASIC_RATE_MASK);

	return csr_is_aggregate_rate_supported(mac_ctx, n);
}

#ifdef WLAN_FEATURE_FILS_SK
static inline void csr_free_fils_profile_info(struct csr_roam_profile *profile)
{
	if (profile->fils_con_info) {
		qdf_mem_free(profile->fils_con_info);
		profile->fils_con_info = NULL;
	}

	if (profile->hlp_ie) {
		qdf_mem_free(profile->hlp_ie);
		profile->hlp_ie = NULL;
		profile->hlp_ie_len = 0;
	}
}
#else
static inline void csr_free_fils_profile_info(struct csr_roam_profile *profile)
{ }
#endif

void csr_release_profile(struct mac_context *mac, struct csr_roam_profile *pProfile)
{
	if (pProfile) {
		if (pProfile->BSSIDs.bssid) {
			qdf_mem_free(pProfile->BSSIDs.bssid);
			pProfile->BSSIDs.bssid = NULL;
		}
		if (pProfile->SSIDs.SSIDList) {
			qdf_mem_free(pProfile->SSIDs.SSIDList);
			pProfile->SSIDs.SSIDList = NULL;
		}
		if (pProfile->pWPAReqIE) {
			qdf_mem_free(pProfile->pWPAReqIE);
			pProfile->pWPAReqIE = NULL;
		}
		if (pProfile->pRSNReqIE) {
			qdf_mem_free(pProfile->pRSNReqIE);
			pProfile->pRSNReqIE = NULL;
		}
#ifdef FEATURE_WLAN_WAPI
		if (pProfile->pWAPIReqIE) {
			qdf_mem_free(pProfile->pWAPIReqIE);
			pProfile->pWAPIReqIE = NULL;
		}
#endif /* FEATURE_WLAN_WAPI */

		if (pProfile->pAddIEScan) {
			qdf_mem_free(pProfile->pAddIEScan);
			pProfile->pAddIEScan = NULL;
		}

		if (pProfile->pAddIEAssoc) {
			qdf_mem_free(pProfile->pAddIEAssoc);
			pProfile->pAddIEAssoc = NULL;
		}
		if (pProfile->ChannelInfo.ChannelList) {
			qdf_mem_free(pProfile->ChannelInfo.ChannelList);
			pProfile->ChannelInfo.ChannelList = NULL;
		}
		csr_free_fils_profile_info(pProfile);
		qdf_mem_zero(pProfile, sizeof(struct csr_roam_profile));
	}
}

void csr_free_scan_filter(struct mac_context *mac, tCsrScanResultFilter
						*pScanFilter)
{
	if (pScanFilter->BSSIDs.bssid) {
		qdf_mem_free(pScanFilter->BSSIDs.bssid);
		pScanFilter->BSSIDs.bssid = NULL;
	}
	if (pScanFilter->ChannelInfo.ChannelList) {
		qdf_mem_free(pScanFilter->ChannelInfo.ChannelList);
		pScanFilter->ChannelInfo.ChannelList = NULL;
	}
	if (pScanFilter->SSIDs.SSIDList) {
		qdf_mem_free(pScanFilter->SSIDs.SSIDList);
		pScanFilter->SSIDs.SSIDList = NULL;
	}
}

void csr_free_roam_profile(struct mac_context *mac, uint32_t sessionId)
{
	struct csr_roam_session *pSession = &mac->roam.roamSession[sessionId];

	if (pSession->pCurRoamProfile) {
		csr_release_profile(mac, pSession->pCurRoamProfile);
		qdf_mem_free(pSession->pCurRoamProfile);
		pSession->pCurRoamProfile = NULL;
	}
}

void csr_free_connect_bss_desc(struct mac_context *mac, uint32_t sessionId)
{
	struct csr_roam_session *pSession = &mac->roam.roamSession[sessionId];

	if (pSession->pConnectBssDesc) {
		qdf_mem_free(pSession->pConnectBssDesc);
		pSession->pConnectBssDesc = NULL;
	}
}

tSirResultCodes csr_get_de_auth_rsp_status_code(struct deauth_rsp *pSmeRsp)
{
	uint8_t *pBuffer = (uint8_t *) pSmeRsp;
	uint32_t ret;

	pBuffer +=
		(sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t) +
		 sizeof(uint16_t));
	/* tSirResultCodes is an enum, assuming is 32bit */
	/* If we cannot make this assumption, use copymemory */
	qdf_get_u32(pBuffer, &ret);

	return (tSirResultCodes) ret;
}

tSirScanType csr_get_scan_type(struct mac_context *mac, uint8_t chnId)
{
	tSirScanType scanType = eSIR_PASSIVE_SCAN;
	enum channel_state channelEnabledType;

	channelEnabledType = wlan_reg_get_channel_state(mac->pdev, chnId);
	if (CHANNEL_STATE_ENABLE == channelEnabledType)
		scanType = eSIR_ACTIVE_SCAN;

	return scanType;
}

tSirBssType csr_translate_bsstype_to_mac_type(eCsrRoamBssType csrtype)
{
	tSirBssType ret;

	switch (csrtype) {
	case eCSR_BSS_TYPE_INFRASTRUCTURE:
		ret = eSIR_INFRASTRUCTURE_MODE;
		break;
	case eCSR_BSS_TYPE_IBSS:
	case eCSR_BSS_TYPE_START_IBSS:
		ret = eSIR_IBSS_MODE;
		break;
	case eCSR_BSS_TYPE_INFRA_AP:
		ret = eSIR_INFRA_AP_MODE;
		break;
	case eCSR_BSS_TYPE_NDI:
		ret = eSIR_NDI_MODE;
		break;
	case eCSR_BSS_TYPE_ANY:
	default:
		ret = eSIR_AUTO_MODE;
		break;
	}

	return ret;
}

/* This function use the parameters to decide the CFG value. */
/* CSR never sets MLME_DOT11_MODE_ALL to the CFG */
/* So PE should not see MLME_DOT11_MODE_ALL when it gets the CFG value */
enum csr_cfgdot11mode
csr_get_cfg_dot11_mode_from_csr_phy_mode(struct csr_roam_profile *pProfile,
					 eCsrPhyMode phyMode,
					 bool fProprietary)
{
	uint32_t cfgDot11Mode = eCSR_CFG_DOT11_MODE_ABG;

	switch (phyMode) {
	case eCSR_DOT11_MODE_11a:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
		break;
	case eCSR_DOT11_MODE_11b:
	case eCSR_DOT11_MODE_11b_ONLY:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
		break;
	case eCSR_DOT11_MODE_11g:
	case eCSR_DOT11_MODE_11g_ONLY:
		if (pProfile && (CSR_IS_INFRA_AP(pProfile))
		    && (phyMode == eCSR_DOT11_MODE_11g_ONLY))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G_ONLY;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
		break;
	case eCSR_DOT11_MODE_11n:
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11n_ONLY:
		if (pProfile && CSR_IS_INFRA_AP(pProfile))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N_ONLY;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_abg:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_ABG;
		break;
	case eCSR_DOT11_MODE_AUTO:
		cfgDot11Mode = eCSR_CFG_DOT11_MODE_AUTO;
		break;

	case eCSR_DOT11_MODE_11ac:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11ac_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC_ONLY;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11ax:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;
	case eCSR_DOT11_MODE_11ax_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AX_ONLY;
		else if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC))
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11AC;
		else
			cfgDot11Mode = eCSR_CFG_DOT11_MODE_11N;
		break;

	default:
		/* No need to assign anything here */
		break;
	}

	return cfgDot11Mode;
}

QDF_STATUS csr_get_regulatory_domain_for_country(struct mac_context *mac,
						 uint8_t *pCountry,
						 v_REGDOMAIN_t *pDomainId,
						 enum country_src source)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	QDF_STATUS qdf_status;
	uint8_t countryCode[CDS_COUNTRY_CODE_LEN + 1];
	v_REGDOMAIN_t domainId;

	if (pCountry) {
		countryCode[0] = pCountry[0];
		countryCode[1] = pCountry[1];
		qdf_status = wlan_reg_get_domain_from_country_code(&domainId,
								  countryCode,
								  source);

		if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
			if (pDomainId)
				*pDomainId = domainId;
			status = QDF_STATUS_SUCCESS;
		} else {
			sme_warn("Couldn't find domain for country code %c%c",
				pCountry[0], pCountry[1]);
			status = QDF_STATUS_E_INVAL;
		}
	}

	return status;
}

QDF_STATUS csr_get_modify_profile_fields(struct mac_context *mac,
					uint32_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields)
{
	if (!pModifyProfileFields)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_copy(pModifyProfileFields,
		     &mac->roam.roamSession[sessionId].connectedProfile.
		     modifyProfileFields, sizeof(tCsrRoamModifyProfileFields));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_set_modify_profile_fields(struct mac_context *mac,
					uint32_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields)
{
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);

	qdf_mem_copy(&pSession->connectedProfile.modifyProfileFields,
		     pModifyProfileFields, sizeof(tCsrRoamModifyProfileFields));

	return QDF_STATUS_SUCCESS;
}


bool csr_is_set_key_allowed(struct mac_context *mac, uint32_t sessionId)
{
	bool fRet = true;
	struct csr_roam_session *pSession;

	pSession = CSR_GET_SESSION(mac, sessionId);

	/*
	 * This condition is not working for infra state. When infra is in
	 * not-connected state the pSession->pCurRoamProfile is NULL, this
	 * function returns true, that is incorrect.
	 * Since SAP requires to set key without any BSS started, it needs
	 * this condition to be met. In other words, this function is useless.
	 * The current work-around is to process setcontext_rsp no matter
	 * what the state is.
	 */
	sme_debug("is not what it intends to. Must be revisit or removed");
	if ((!pSession)
	    || (csr_is_conn_state_disconnected(mac, sessionId)
		&& (pSession->pCurRoamProfile)
		&& (!(CSR_IS_INFRA_AP(pSession->pCurRoamProfile))))
	    ) {
		fRet = false;
	}

	return fRet;
}

/* no need to acquire lock for this basic function */
uint16_t sme_chn_to_freq(uint8_t chanNum)
{
	int i;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (WLAN_REG_CH_NUM(i) == chanNum)
			return WLAN_REG_CH_TO_FREQ(i);
	}

	return 0;
}

struct lim_channel_status *
csr_get_channel_status(struct mac_context *mac, uint32_t channel_id)
{
	uint8_t i;
	struct lim_scan_channel_status *channel_status;
	struct lim_channel_status *entry;

	if (!mac->sap.acs_with_more_param)
		return NULL;

	channel_status = &mac->lim.scan_channel_status;
	for (i = 0; i < channel_status->total_channel; i++) {
		entry = &channel_status->channel_status_list[i];
		if (entry->channel_id == channel_id)
			return entry;
	}
	sme_err("Channel %d status info not exist", channel_id);

	return NULL;
}

void csr_clear_channel_status(struct mac_context *mac)
{
	struct lim_scan_channel_status *channel_status;

	if (!mac->sap.acs_with_more_param)
		return;

	channel_status = &mac->lim.scan_channel_status;
	channel_status->total_channel = 0;

	return;
}

bool csr_is_channel_present_in_list(uint8_t *pChannelList,
				    int numChannels, uint8_t channel)
{
	int i = 0;

	/* Check for NULL pointer */
	if (!pChannelList || (numChannels == 0))
		return false;

	/* Look for the channel in the list */
	for (i = 0; (i < numChannels) &&
	     (i < CFG_VALID_CHANNEL_LIST_LEN); i++) {
		if (pChannelList[i] == channel)
			return true;
	}

	return false;
}

/**
 * sme_bsstype_to_string() - converts bss type to string.
 * @bss_type: bss type enum
 *
 * Return: printable string for bss type
 */
const char *sme_bss_type_to_string(const uint8_t bss_type)
{
	switch (bss_type) {
	CASE_RETURN_STRING(eCSR_BSS_TYPE_INFRASTRUCTURE);
	CASE_RETURN_STRING(eCSR_BSS_TYPE_INFRA_AP);
	CASE_RETURN_STRING(eCSR_BSS_TYPE_IBSS);
	CASE_RETURN_STRING(eCSR_BSS_TYPE_START_IBSS);
	CASE_RETURN_STRING(eCSR_BSS_TYPE_ANY);
	default:
		return "unknown bss type";
	}
}

QDF_STATUS csr_add_to_channel_list_front(uint8_t *pChannelList,
					 int numChannels, uint8_t channel)
{
	int i = 0;

	/* Check for NULL pointer */
	if (!pChannelList)
		return QDF_STATUS_E_NULL_VALUE;

	/* Make room for the addition.  (Start moving from the back.) */
	for (i = numChannels; i > 0; i--)
		pChannelList[i] = pChannelList[i - 1];

	/* Now add the NEW channel...at the front */
	pChannelList[0] = channel;

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_wait_for_connection_update() - Wait for hw mode update
 * @mac: Pointer to the MAC context
 * @do_release_reacquire_lock: Indicates whether release and
 * re-acquisition of SME global lock is required.
 *
 * Waits for CONNECTION_UPDATE_TIMEOUT time so that the
 * hw mode update can get processed.
 *
 * Return: True if the wait was successful, false otherwise
 */
bool csr_wait_for_connection_update(struct mac_context *mac,
		bool do_release_reacquire_lock)
{
	QDF_STATUS status, ret;

	if (do_release_reacquire_lock == true) {
		ret = sme_release_global_lock(&mac->sme);
		if (!QDF_IS_STATUS_SUCCESS(ret)) {
			cds_err("lock release fail %d", ret);
			return false;
		}
	}

	status = policy_mgr_wait_for_connection_update(mac->psoc);

	if (do_release_reacquire_lock == true) {
		ret = sme_acquire_global_lock(&mac->sme);
		if (!QDF_IS_STATUS_SUCCESS(ret))
			return false;
	}

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		cds_err("wait for event failed");
		return false;
	}

	return true;
}

/**
 * csr_get_session_persona() - get persona of a session
 * @pmac: pointer to global MAC context
 * @session_id: session id
 *
 * This function is to return the persona of a session
 *
 * Reture: enum QDF_OPMODE persona
 */
enum QDF_OPMODE csr_get_session_persona(struct mac_context *pmac,
					uint32_t session_id)
{
	struct csr_roam_session *session = NULL;

	session = CSR_GET_SESSION(pmac, session_id);
	if (!session || !session->pCurRoamProfile)
		return QDF_MAX_NO_OF_MODE;

	return session->pCurRoamProfile->csrPersona;
}

/**
 * csr_is_ndi_started() - function to check if NDI is started
 * @mac_ctx: handle to mac context
 * @session_id: session identifier
 *
 * returns: true if NDI is started, false otherwise
 */
bool csr_is_ndi_started(struct mac_context *mac_ctx, uint32_t session_id)
{
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session)
		return false;

	return eCSR_CONNECT_STATE_TYPE_NDI_STARTED == session->connectState;
}

bool csr_is_mcc_channel(struct mac_context *mac_ctx, uint8_t channel)
{
	struct csr_roam_session *session;
	enum QDF_OPMODE oper_mode;
	uint8_t oper_channel = 0;
	uint8_t session_id;

	if (channel == 0)
		return false;

	for (session_id = 0; session_id < WLAN_MAX_VDEVS; session_id++) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
			session = CSR_GET_SESSION(mac_ctx, session_id);
			if (!session->pCurRoamProfile)
				continue;
			oper_mode = session->pCurRoamProfile->csrPersona;
			if ((((oper_mode == QDF_STA_MODE) ||
			    (oper_mode == QDF_P2P_CLIENT_MODE)) &&
			    (session->connectState ==
			    eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED)) ||
			    (((oper_mode == QDF_P2P_GO_MODE) ||
			    (oper_mode == QDF_SAP_MODE))
			    && (session->connectState !=
			    eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED)))
				oper_channel = session->connectedProfile.
					operationChannel;

			if (oper_channel && channel != oper_channel &&
			    (!policy_mgr_is_hw_dbs_capable(mac_ctx->psoc) ||
			    WLAN_REG_IS_SAME_BAND_CHANNELS(channel,
						 oper_channel)))
				return true;
		}
	}

	return false;
}

