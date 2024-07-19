/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: osif_cm_req.c
 *
 * This file maintains definitaions of connect, disconnect, roam
 * request apis.
 */

#include "wlan_osif_priv.h"
#include "osif_cm_req.h"
#include "wlan_cm_ucfg_api.h"
#include "wlan_nl_to_crypto_params.h"
#include <wlan_cfg80211.h>
#include "osif_cm_util.h"
#ifdef WLAN_FEATURE_FILS_SK
#include <wlan_mlme_ucfg_api.h>
#endif
#include <wlan_mlo_mgr_sta.h>
#include <utils_mlo.h>

static void osif_cm_free_wep_key_params(struct wlan_cm_connect_req *connect_req)
{
	if (connect_req->crypto.wep_keys.key) {
		qdf_mem_zero(connect_req->crypto.wep_keys.key,
			     connect_req->crypto.wep_keys.key_len);
		qdf_mem_free(connect_req->crypto.wep_keys.key);
		connect_req->crypto.wep_keys.key = NULL;
	}
	if (connect_req->crypto.wep_keys.seq) {
		qdf_mem_zero(connect_req->crypto.wep_keys.seq,
			     connect_req->crypto.wep_keys.seq_len);
		qdf_mem_free(connect_req->crypto.wep_keys.seq);
		connect_req->crypto.wep_keys.seq = NULL;
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0) && \
LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0) && \
!defined(CFG80211_CRYPTO_WEP_KEYS_REMOVED)
static QDF_STATUS
osif_cm_update_wep_seq_info(struct wlan_cm_connect_req *connect_req,
			    const struct cfg80211_connect_params *req)
{
	if (req->crypto.wep_keys->seq_len) {
		connect_req->crypto.wep_keys.seq_len =
						req->crypto.wep_keys->seq_len;
		connect_req->crypto.wep_keys.seq =
			qdf_mem_malloc(connect_req->crypto.wep_keys.seq_len);
		if (!connect_req->crypto.wep_keys.seq) {
			osif_cm_free_wep_key_params(connect_req);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(connect_req->crypto.wep_keys.seq,
			     req->crypto.wep_keys->seq,
			     connect_req->crypto.wep_keys.seq_len);
	}
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
osif_cm_update_wep_seq_info(struct wlan_cm_connect_req *connect_req,
			    const struct cfg80211_connect_params *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if !defined(CFG80211_CRYPTO_WEP_KEYS_REMOVED)
static QDF_STATUS
osif_cm_set_wep_key_params(struct wlan_cm_connect_req *connect_req,
			   const struct cfg80211_connect_params *req)
{
	if (!req->key_len)
		return QDF_STATUS_SUCCESS;

	connect_req->crypto.wep_keys.key_len = req->key_len;
	connect_req->crypto.wep_keys.key_idx = req->key_idx;

	connect_req->crypto.wep_keys.key =
			qdf_mem_malloc(connect_req->crypto.wep_keys.key_len);
	if (!connect_req->crypto.wep_keys.key)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(connect_req->crypto.wep_keys.key, req->key,
		     connect_req->crypto.wep_keys.key_len);

	return osif_cm_update_wep_seq_info(connect_req, req);
}
#else
static QDF_STATUS
osif_cm_set_wep_key_params(struct wlan_cm_connect_req *connect_req,
			   const struct cfg80211_connect_params *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static void osif_cm_set_auth_type(struct wlan_cm_connect_req *connect_req,
				  const struct cfg80211_connect_params *req)
{
	wlan_crypto_auth_mode crypto_auth_type =
			osif_nl_to_crypto_auth_type(req->auth_type);

	/* For auto check wpa version to decide WPA or RSNA */
	if (crypto_auth_type == WLAN_CRYPTO_AUTH_AUTO &&
	    req->crypto.wpa_versions) {
		if (req->crypto.wpa_versions & NL80211_WPA_VERSION_1)
			crypto_auth_type = WLAN_CRYPTO_AUTH_WPA;
		else
			crypto_auth_type = WLAN_CRYPTO_AUTH_RSNA;
	} else if (!req->crypto.n_ciphers_pairwise) {
		crypto_auth_type = WLAN_CRYPTO_AUTH_OPEN;
	}

	QDF_SET_PARAM(connect_req->crypto.auth_type, crypto_auth_type);
}

#ifdef CFG80211_MULTI_AKM_CONNECT_SUPPORT
static void
osif_cm_set_akm_params(struct wlan_cm_connect_req *connect_req,
		       const struct cfg80211_connect_params *req)
{
	uint32_t i = 0;
	wlan_crypto_key_mgmt akm;

	/* Fill AKM suites */
	if (req->crypto.n_connect_akm_suites) {
		for (i = 0; i < req->crypto.n_connect_akm_suites &&
		     i < WLAN_CM_MAX_CONNECT_AKMS; i++) {
			akm = osif_nl_to_crypto_akm_type(
					req->crypto.connect_akm_suites[i]);
			QDF_SET_PARAM(connect_req->crypto.akm_suites, akm);
		}
	} else {
		QDF_SET_PARAM(connect_req->crypto.akm_suites,
			      WLAN_CRYPTO_KEY_MGMT_NONE);
	}
}

static int
osif_cm_get_num_akm_suites(const struct cfg80211_connect_params *req)
{
	return req->crypto.n_connect_akm_suites;
}

static uint32_t*
osif_cm_get_akm_suites(const struct cfg80211_connect_params *req)
{
	return (uint32_t *)req->crypto.connect_akm_suites;
}
#else
static void
osif_cm_set_akm_params(struct wlan_cm_connect_req *connect_req,
		       const struct cfg80211_connect_params *req)
{
	uint32_t i = 0;
	wlan_crypto_key_mgmt akm;

	/* Fill AKM suites */
	if (req->crypto.n_akm_suites) {
		for (i = 0; i < req->crypto.n_akm_suites &&
		     i < NL80211_MAX_NR_AKM_SUITES; i++) {
			akm = osif_nl_to_crypto_akm_type(
					req->crypto.akm_suites[i]);
			QDF_SET_PARAM(connect_req->crypto.akm_suites, akm);
		}
	} else {
		QDF_SET_PARAM(connect_req->crypto.akm_suites,
			      WLAN_CRYPTO_KEY_MGMT_NONE);
	}
}

static int
osif_cm_get_num_akm_suites(const struct cfg80211_connect_params *req)
{
	return req->crypto.n_akm_suites;
}

static uint32_t*
osif_cm_get_akm_suites(const struct cfg80211_connect_params *req)
{
	return (uint32_t *)req->crypto.akm_suites;
}
#endif

static inline
uint8_t osif_cm_get_rsn_cap_mfp(enum nl80211_mfp mfp_state)
{
	switch (mfp_state) {
	case NL80211_MFP_REQUIRED:
		return RSN_CAP_MFP_REQUIRED;
	case NL80211_MFP_OPTIONAL:
		return RSN_CAP_MFP_CAPABLE;
	default:
		return RSN_CAP_MFP_DISABLED;
	}
}

static
QDF_STATUS osif_cm_set_crypto_params(struct wlan_cm_connect_req *connect_req,
				     const struct cfg80211_connect_params *req)
{
	uint32_t i = 0;
	QDF_STATUS status;
	wlan_crypto_cipher_type cipher = WLAN_CRYPTO_CIPHER_NONE;

	connect_req->crypto.wpa_versions = req->crypto.wpa_versions;

	osif_cm_set_auth_type(connect_req, req);

	if (req->crypto.cipher_group)
		cipher =
			osif_nl_to_crypto_cipher_type(req->crypto.cipher_group);

	QDF_SET_PARAM(connect_req->crypto.group_cipher, cipher);

	/* Fill Pairwise ciphers */
	if (req->crypto.n_ciphers_pairwise) {
		for (i = 0; i < req->crypto.n_ciphers_pairwise &&
		     i < NL80211_MAX_NR_CIPHER_SUITES; i++) {
			cipher = osif_nl_to_crypto_cipher_type(
					req->crypto.ciphers_pairwise[i]);
			QDF_SET_PARAM(connect_req->crypto.ciphers_pairwise,
				      cipher);
		}
	} else {
		QDF_SET_PARAM(connect_req->crypto.ciphers_pairwise,
			      WLAN_CRYPTO_CIPHER_NONE);
	}

	/* Fill AKM suites */
	osif_cm_set_akm_params(connect_req, req);

	/* Fill WEP Key information */
	status = osif_cm_set_wep_key_params(connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("set wep key params failed");

	/* Copy user configured MFP capability */
	connect_req->crypto.user_mfp = osif_cm_get_rsn_cap_mfp(req->mfp);

	return status;
}

#ifdef WLAN_FEATURE_FILS_SK
static bool osif_cm_is_akm_suite_fils(uint32_t key_mgmt)
{
	switch (key_mgmt) {
	case WLAN_AKM_SUITE_FILS_SHA256:
	case WLAN_AKM_SUITE_FILS_SHA384:
	case WLAN_AKM_SUITE_FT_FILS_SHA256:
	case WLAN_AKM_SUITE_FT_FILS_SHA384:
		osif_debug("Fils AKM : %x", key_mgmt);
		return true;
	default:
		return false;
	}
}

static bool osif_cm_is_conn_type_fils(struct wlan_cm_connect_req *connect_req,
				      const struct cfg80211_connect_params *req)
{
	int num_akm_suites;
	uint32_t *akm_suites;
	uint8_t i;

	num_akm_suites = osif_cm_get_num_akm_suites(req);
	akm_suites = osif_cm_get_akm_suites(req);

	if (num_akm_suites <= 0)
		return false;

	/*
	 * Auth type will be either be OPEN or FILS type for a FILS connection
	 */
	if (connect_req->fils_info.auth_type == FILS_PK_MAX &&
	    req->auth_type != NL80211_AUTHTYPE_OPEN_SYSTEM)
		return false;

	for (i = 0; i < num_akm_suites; i++) {
		if (!osif_cm_is_akm_suite_fils(akm_suites[i]))
			continue;
		return true;
	}


	return false;
}

enum wlan_fils_auth_type
osif_cm_get_fils_auth_type(enum nl80211_auth_type auth)
{
	switch (auth) {
	case NL80211_AUTHTYPE_FILS_SK:
		return FILS_SK_WITHOUT_PFS;
	case NL80211_AUTHTYPE_FILS_SK_PFS:
		return FILS_SK_WITH_PFS;
	case NL80211_AUTHTYPE_FILS_PK:
		return FILS_PK_AUTH;
	default:
		return FILS_PK_MAX;
	}
}

static QDF_STATUS
osif_cm_set_fils_info(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_connect_req *connect_req,
		      const struct cfg80211_connect_params *req)
{
	bool value = 0;
	QDF_STATUS status;
	uint8_t *buf;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -QDF_STATUS_E_INVAL;

	connect_req->fils_info.auth_type =
		osif_cm_get_fils_auth_type(req->auth_type);
	connect_req->fils_info.is_fils_connection =
					osif_cm_is_conn_type_fils(connect_req,
								  req);
	osif_debug("auth type %d is fils %d",
		   connect_req->fils_info.auth_type,
		   connect_req->fils_info.is_fils_connection);
	if (!connect_req->fils_info.is_fils_connection)
		return QDF_STATUS_SUCCESS;

	status = ucfg_mlme_get_fils_enabled_info(psoc, &value);
	if (QDF_IS_STATUS_ERROR(status) || !value) {
		osif_err("get_fils_enabled status: %d fils_enabled: %d",
			 status, value);
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * The initial connection for FILS may happen with an OPEN
	 * auth type. Hence we need to allow the connection to go
	 * through in that case as well.
	 */
	if (req->auth_type != NL80211_AUTHTYPE_FILS_SK) {
		osif_debug("set is fils false for initial connection");
		connect_req->fils_info.is_fils_connection = false;
		return QDF_STATUS_SUCCESS;
	}

	connect_req->fils_info.realm_len = req->fils_erp_realm_len;

	if (connect_req->fils_info.realm_len > WLAN_CM_FILS_MAX_REALM_LEN) {
		osif_err("Invalid fils realm len %d",
			 connect_req->fils_info.realm_len);
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_zero(connect_req->fils_info.realm, WLAN_CM_FILS_MAX_REALM_LEN);
	qdf_mem_copy(connect_req->fils_info.realm, req->fils_erp_realm,
		     connect_req->fils_info.realm_len);

	connect_req->fils_info.next_seq_num = req->fils_erp_next_seq_num + 1;

	connect_req->fils_info.rrk_len = req->fils_erp_rrk_len;

	if (connect_req->fils_info.rrk_len > WLAN_CM_FILS_MAX_RRK_LENGTH) {
		osif_err("Invalid fils rrk len %d",
			 connect_req->fils_info.rrk_len);
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_zero(connect_req->fils_info.rrk, WLAN_CM_FILS_MAX_RRK_LENGTH);
	qdf_mem_copy(connect_req->fils_info.rrk, req->fils_erp_rrk,
		     connect_req->fils_info.rrk_len);

	connect_req->fils_info.username_len = req->fils_erp_username_len +
					sizeof(char) + req->fils_erp_realm_len;
	osif_debug("usrname len %d = usrname recv len %zu + realm len %d + %zu",
		   connect_req->fils_info.username_len,
		   req->fils_erp_username_len,
		   connect_req->fils_info.realm_len, sizeof(char));

	if (connect_req->fils_info.username_len >
					WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH) {
		osif_err("Invalid fils username len %d",
			 connect_req->fils_info.username_len);
		return QDF_STATUS_E_INVAL;
	}
	if (!req->fils_erp_username_len) {
		osif_info("FILS_PMKSA: No ERP username, return success");
		return QDF_STATUS_SUCCESS;
	}
	buf = connect_req->fils_info.username;
	qdf_mem_zero(connect_req->fils_info.username,
		     WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH);
	qdf_mem_copy(buf, req->fils_erp_username, req->fils_erp_username_len);
	buf += req->fils_erp_username_len;
	*buf++ = '@';
	qdf_mem_copy(buf, req->fils_erp_realm, req->fils_erp_realm_len);

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS osif_cm_set_fils_info(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_req *connect_req,
				 const struct cfg80211_connect_params *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline void
osif_cm_set_prev_bssid(struct wlan_cm_connect_req *connect_req,
		       const struct cfg80211_connect_params *req)
{
	if (req->prev_bssid)
		qdf_mem_copy(connect_req->prev_bssid.bytes, req->prev_bssid,
			     QDF_MAC_ADDR_SIZE);
}

static inline
void osif_cm_dump_prev_bssid(const struct cfg80211_connect_params *req)
{
	if (req->prev_bssid)
		osif_nofl_debug("prev BSSID "QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(req->prev_bssid));
}

#else
static inline void
osif_cm_set_prev_bssid(struct wlan_cm_connect_req *connect_req,
		       const struct cfg80211_connect_params *req)
{
}

static inline
void osif_cm_dump_prev_bssid(const struct cfg80211_connect_params *req)
{
}

#endif

static inline void
osif_cm_dump_connect_req(struct net_device *dev, uint8_t vdev_id,
			 const struct cfg80211_connect_params *req)
{
	uint32_t i;
	uint32_t num_akm_suites;
	uint32_t *akm_suites;

	num_akm_suites = osif_cm_get_num_akm_suites(req);
	akm_suites = osif_cm_get_akm_suites(req);

	osif_nofl_debug("connect req for %s(vdevid-%d) freq %d SSID %.*s auth type %d WPA ver %d n_akm %d n_cipher %d grp_cipher %x mfp %d freq hint %d",
			dev->name, vdev_id,
			req->channel ? req->channel->center_freq : 0,
			(int)req->ssid_len, req->ssid, req->auth_type,
			req->crypto.wpa_versions,
			num_akm_suites,
			req->crypto.n_ciphers_pairwise,
			req->crypto.cipher_group, req->mfp,
			req->channel_hint ? req->channel_hint->center_freq : 0);
	if (req->bssid)
		osif_nofl_debug("BSSID "QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(req->bssid));
	if (req->bssid_hint)
		osif_nofl_debug("BSSID hint "QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(req->bssid_hint));
	osif_cm_dump_prev_bssid(req);

	for (i = 0; i < num_akm_suites; i++)
		osif_nofl_debug("akm[%d] = %x", i, akm_suites[i]);

	for (i = 0; i < req->crypto.n_ciphers_pairwise; i++)
		osif_nofl_debug("cipher_pairwise[%d] = %x", i,
				req->crypto.ciphers_pairwise[i]);
}

static void
osif_cm_fill_connect_params(struct wlan_cm_connect_req *req,
			    const struct osif_connect_params *params)
{
	if (!params)
		return;

	if (params->scan_ie.len) {
		req->scan_ie.ptr = qdf_mem_malloc(params->scan_ie.len);
		if (req->scan_ie.ptr) {
			qdf_mem_copy(req->scan_ie.ptr, params->scan_ie.ptr,
				     params->scan_ie.len);
			req->scan_ie.len = params->scan_ie.len;
		}
	}
	req->dot11mode_filter = params->dot11mode_filter;
	req->force_rsne_override = params->force_rsne_override;
	req->sae_pwe = params->sae_pwe;

	if (!qdf_is_macaddr_zero((struct qdf_mac_addr *)&params->prev_bssid))
		qdf_copy_macaddr(&req->prev_bssid,
				 (struct qdf_mac_addr *)&params->prev_bssid);
}

static void osif_cm_free_connect_req(struct wlan_cm_connect_req *connect_req)
{
	if (connect_req->scan_ie.ptr) {
		qdf_mem_free(connect_req->scan_ie.ptr);
		connect_req->scan_ie.ptr = NULL;
	}

	if (connect_req->assoc_ie.ptr) {
		qdf_mem_free(connect_req->assoc_ie.ptr);
		connect_req->assoc_ie.ptr = NULL;
	}

	osif_cm_free_wep_key_params(connect_req);
	qdf_mem_free(connect_req);
}

#ifdef WLAN_FEATURE_11BE_MLO
#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static inline
QDF_STATUS osif_update_mlo_partner_info(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_req *connect_req,
			const struct cfg80211_connect_params *req)
{
	return QDF_STATUS_SUCCESS;
}
#else
static inline
void osif_update_partner_vdev_info(struct wlan_objmgr_vdev *vdev,
				   struct mlo_partner_info partner_info)
{
	struct wlan_objmgr_vdev *tmp_vdev;
	uint8_t i = 0;

	if (!vdev)
		return;

	for (i = 0; i < partner_info.num_partner_links; i++) {
		tmp_vdev = mlo_get_ml_vdev_by_mac(
				vdev,
				&partner_info.partner_link_info[i].link_addr);
		if (tmp_vdev) {
			mlo_update_connect_req_links(tmp_vdev, 1);
			wlan_vdev_mlme_feat_ext2_cap_set(
					tmp_vdev, WLAN_VDEV_FEXT2_MLO);
			wlan_vdev_mlme_feat_ext2_cap_set(
					tmp_vdev, WLAN_VDEV_FEXT2_MLO_STA_LINK);
			wlan_vdev_set_link_id(
				tmp_vdev,
				partner_info.partner_link_info[i].link_id);
			osif_debug("link id %d",
				   tmp_vdev->vdev_mlme.mlo_link_id);
		}
	}
}

static inline
QDF_STATUS osif_update_mlo_partner_info(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_req *connect_req,
			const struct cfg80211_connect_params *req)
{
	/* Update ml partner info from connect req*/
	uint8_t *ptr = NULL;
	uint8_t *ml_ie = NULL;
	qdf_size_t ml_ie_len = 0;
	struct mlo_partner_info partner_info = {0};
	bool ml_ie_found = false, linkidfound = false;
	uint8_t linkid = 0;
	enum wlan_ml_variant variant;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!vdev || !connect_req || !req)
		return status;

	osif_debug("ML IE search start");
	if (req->ie_len) {
		ptr = (uint8_t *)req->ie;
		status = util_find_mlie(ptr, req->ie_len, &ml_ie, &ml_ie_len);
		if (QDF_IS_STATUS_ERROR(status) || !ml_ie) {
			osif_debug("ML IE not found");
			/* Return success since ML is not mandatory for a
			 * connect request
			 */
			return QDF_STATUS_SUCCESS;
		}

		osif_debug("ML IE found length %d", (int)ml_ie_len);
		qdf_trace_hex_dump(QDF_MODULE_ID_OS_IF, QDF_TRACE_LEVEL_DEBUG,
				   ml_ie, (int)ml_ie_len);
		ml_ie_found = true;

		status = util_get_mlie_variant(ml_ie, ml_ie_len,
					       (int *)&variant);
		if (status != QDF_STATUS_SUCCESS) {
			osif_err("Unable to get Multi-Link element variant");
			return status;
		}

		if (variant != WLAN_ML_VARIANT_BASIC) {
			osif_err("Invalid Multi-Link element variant %u",
				 variant);
			return status;
		}

		status = util_get_bvmlie_primary_linkid(ml_ie, ml_ie_len,
							&linkidfound, &linkid);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("Unable to find primary link ID in ML IE");
			return status;
		}

		status = util_get_bvmlie_persta_partner_info(ml_ie, ml_ie_len,
							     &partner_info);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("Unable to find per-sta profile in ML IE");
			return status;
		}

		wlan_vdev_set_link_id(vdev, linkid);
		wlan_vdev_mlme_feat_ext2_cap_set(vdev, WLAN_VDEV_FEXT2_MLO);
	}

	qdf_mem_copy(&connect_req->ml_parnter_info,
		     &partner_info, sizeof(struct mlo_partner_info));

	if (ml_ie_found) {
		mlo_clear_connect_req_links_bmap(vdev);
		mlo_update_connect_req_links(vdev, 1);
		osif_update_partner_vdev_info(vdev, partner_info);
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_11BE_MLO_ADV_FEATURE */
#else
static inline
QDF_STATUS osif_update_mlo_partner_info(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_req *connect_req,
			const struct cfg80211_connect_params *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

int osif_cm_connect(struct net_device *dev, struct wlan_objmgr_vdev *vdev,
		    const struct cfg80211_connect_params *req,
		    const struct osif_connect_params *params)
{
	struct wlan_cm_connect_req *connect_req;
	const u8 *bssid_hint = req->bssid_hint;
	uint8_t vdev_id = vdev->vdev_objmgr.vdev_id;
	QDF_STATUS status;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_BCAST_INIT;
	struct wlan_objmgr_vdev *temp_vdev;

	if (req->bssid)
		qdf_mem_copy(bssid.bytes, req->bssid,
			     QDF_MAC_ADDR_SIZE);
	else if (bssid_hint)
		qdf_mem_copy(bssid.bytes, req->bssid_hint,
			     QDF_MAC_ADDR_SIZE);

	temp_vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(
						wlan_vdev_get_pdev(vdev),
						bssid.bytes,
						WLAN_OSIF_CM_ID);

	if (temp_vdev) {
		osif_err("vdev %d already exist with same mac address"
			 QDF_MAC_ADDR_FMT, wlan_vdev_get_id(temp_vdev),
			 QDF_MAC_ADDR_REF(bssid.bytes));
		wlan_objmgr_vdev_release_ref(temp_vdev, WLAN_OSIF_CM_ID);
		return -EINVAL;
	}
	osif_cm_dump_connect_req(dev, vdev_id, req);

	status = osif_cm_reset_id_and_src(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	connect_req = qdf_mem_malloc(sizeof(*connect_req));
	if (!connect_req)
		return -ENOMEM;

	connect_req->vdev_id = vdev_id;
	connect_req->source = CM_OSIF_CONNECT;
	if (req->bssid)
		qdf_mem_copy(connect_req->bssid.bytes, req->bssid,
			     QDF_MAC_ADDR_SIZE);
	else if (bssid_hint)
		qdf_mem_copy(connect_req->bssid_hint.bytes, req->bssid_hint,
			     QDF_MAC_ADDR_SIZE);

	osif_cm_set_prev_bssid(connect_req, req);

	connect_req->ssid.length = req->ssid_len;
	if (connect_req->ssid.length > WLAN_SSID_MAX_LEN) {
		osif_err("Invalid ssid len %zu", req->ssid_len);
		osif_cm_free_connect_req(connect_req);
		return -EINVAL;
	}

	qdf_mem_copy(connect_req->ssid.ssid, req->ssid,
		     connect_req->ssid.length);

	if (req->channel)
		connect_req->chan_freq = req->channel->center_freq;

	if (req->channel_hint)
		connect_req->chan_freq_hint = req->channel_hint->center_freq;

	status = osif_cm_set_crypto_params(connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_start_fail;

	connect_req->ht_caps = req->ht_capa.cap_info;
	connect_req->ht_caps_mask = req->ht_capa_mask.cap_info;
	connect_req->vht_caps = req->vht_capa.vht_cap_info;
	connect_req->vht_caps_mask = req->vht_capa_mask.vht_cap_info;

	/* Copy complete ie */
	if (req->ie_len) {
		connect_req->assoc_ie.len = req->ie_len;
		connect_req->assoc_ie.ptr = qdf_mem_malloc(req->ie_len);
		if (!connect_req->assoc_ie.ptr) {
			connect_req->assoc_ie.len = 0;
			status = QDF_STATUS_E_NOMEM;
				goto connect_start_fail;
		}
		qdf_mem_copy(connect_req->assoc_ie.ptr, req->ie,
			     connect_req->assoc_ie.len);
	}

	status = osif_cm_set_fils_info(vdev, connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_start_fail;

	osif_cm_fill_connect_params(connect_req, params);

	status = osif_update_mlo_partner_info(vdev, connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_start_fail;

	status = mlo_connect(vdev, connect_req);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("Connect failed with status %d", status);

connect_start_fail:
	osif_cm_free_connect_req(connect_req);

	return qdf_status_to_os_return(status);
}

int osif_cm_disconnect(struct net_device *dev, struct wlan_objmgr_vdev *vdev,
		       uint16_t reason)
{
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	QDF_STATUS status;

	osif_info("%s(vdevid-%d): Received Disconnect reason:%d %s",
		  dev->name, vdev_id, reason,
		  ucfg_cm_reason_code_to_str(reason));

	status = mlo_disconnect(vdev, CM_OSIF_DISCONNECT, reason, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("Disconnect failed with status %d", status);

	return qdf_status_to_os_return(status);
}

int osif_cm_disconnect_sync(struct wlan_objmgr_vdev *vdev, uint16_t reason)
{
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	QDF_STATUS status;

	osif_info("vdevid-%d: Received Disconnect reason:%d %s",
		  vdev_id, reason, ucfg_cm_reason_code_to_str(reason));

	status = mlo_sync_disconnect(vdev, CM_OSIF_DISCONNECT, reason, NULL);

	return qdf_status_to_os_return(status);
}
