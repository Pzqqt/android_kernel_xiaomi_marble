/*
 * Copyright (c) 2012-2015, 2020 The Linux Foundation. All rights reserved.
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

#include "osif_cm_req.h"
#include "wlan_cm_ucfg_api.h"
#include "wlan_nl_to_crypto_params.h"
#include <wlan_cfg80211.h>
#include "osif_cm_util.h"

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

static QDF_STATUS osif_cm_set_wep_key_params(
				struct wlan_cm_connect_req *connect_req,
				struct cfg80211_connect_params *req)
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

static void osif_cm_set_auth_type(struct wlan_cm_connect_req *connect_req,
				  struct cfg80211_connect_params *req)
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
	} else if (!req->crypto.wpa_versions) {
		crypto_auth_type = WLAN_CRYPTO_AUTH_OPEN;
	}

	QDF_SET_PARAM(connect_req->crypto.auth_type, crypto_auth_type);
}

static
QDF_STATUS osif_cm_set_crypto_params(struct wlan_cm_connect_req *connect_req,
				     struct cfg80211_connect_params *req)
{
	uint32_t i = 0;
	QDF_STATUS status;
	wlan_crypto_cipher_type cipher = WLAN_CRYPTO_CIPHER_NONE;
	wlan_crypto_key_mgmt akm;

	connect_req->crypto.wpa_versions = req->crypto.wpa_versions;

	osif_cm_set_auth_type(connect_req, req);

	if (req->crypto.cipher_group)
		cipher = osif_nl_to_crypto_cipher_type(cipher);

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

	/* Fill WEP Key information */
	status = osif_cm_set_wep_key_params(connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("set wep key params failed");

	return status;
}

#ifdef WLAN_FEATURE_FILS_SK
static bool osif_cm_is_fils_auth_type(enum nl80211_auth_type auth_type)
{
	switch (auth_type) {
	case NL80211_AUTHTYPE_FILS_SK:
	case NL80211_AUTHTYPE_FILS_SK_PFS:
	case NL80211_AUTHTYPE_FILS_PK:
		return true;
	default:
		return false;
	}
}

static bool osif_cm_is_akm_suite_fils(uint32_t key_mgmt)
{
	switch (key_mgmt) {
	case WLAN_AKM_SUITE_FILS_SHA256:
	case WLAN_AKM_SUITE_FILS_SHA384:
	case WLAN_AKM_SUITE_FT_FILS_SHA256:
	case WLAN_AKM_SUITE_FT_FILS_SHA384:
		return true;
	default:
		return false;
	}
}

static bool osif_cm_is_conn_type_fils(struct cfg80211_connect_params *req)
{
	int num_akm_suites = req->crypto.n_akm_suites;
	uint32_t key_mgmt = req->crypto.akm_suites[0];
	bool is_fils_auth_type =
		osif_cm_is_fils_auth_type(req->auth_type);

	if (num_akm_suites <= 0)
		return false;

	/*
	 * Auth type will be either be OPEN or FILS type for a FILS connection
	 */
	if (!is_fils_auth_type &&
	    req->auth_type != NL80211_AUTHTYPE_OPEN_SYSTEM)
		return false;

	if (!osif_cm_is_akm_suite_fils(key_mgmt))
		return false;

	osif_debug("Fils Auth %d AKM %d", req->auth_type, key_mgmt);

	return true;
}

static QDF_STATUS osif_cm_set_fils_info(struct wlan_cm_connect_req *connect_req,
					struct cfg80211_connect_params *req)
{
	connect_req->fils_info.is_fils_connection =
					osif_cm_is_conn_type_fils(req);
	connect_req->fils_info.username_len = req->fils_erp_username_len;

	if (connect_req->fils_info.username_len >
					WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH) {
		osif_err("Invalid fils username len");
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_zero(connect_req->fils_info.username,
		     WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH);
	qdf_mem_copy(connect_req->fils_info.username, req->fils_erp_username,
		     connect_req->fils_info.username_len);

	connect_req->fils_info.realm_len = req->fils_erp_username_len;

	if (connect_req->fils_info.realm_len > WLAN_CM_FILS_MAX_REALM_LEN) {
		osif_err("Invalid fils realm len");
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_zero(connect_req->fils_info.realm, WLAN_CM_FILS_MAX_REALM_LEN);
	qdf_mem_copy(connect_req->fils_info.realm, req->fils_erp_realm,
		     connect_req->fils_info.realm_len);

	connect_req->fils_info.next_seq_num = req->fils_erp_next_seq_num;

	connect_req->fils_info.rrk_len = req->fils_erp_rrk_len;

	if (connect_req->fils_info.rrk_len > WLAN_CM_FILS_MAX_RRK_LENGTH) {
		osif_err("Invalid fils rrk len");
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_zero(connect_req->fils_info.rrk, WLAN_CM_FILS_MAX_RRK_LENGTH);
	qdf_mem_copy(connect_req->fils_info.rrk, req->fils_erp_rrk,
		     connect_req->fils_info.rrk_len);

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS osif_cm_set_fils_info(struct wlan_cm_connect_req *connect_req,
				 struct cfg80211_connect_params *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline void
osif_cm_set_prev_bssid(struct wlan_cm_connect_req *connect_req,
		       struct cfg80211_connect_params *req)
{
	qdf_mem_copy(connect_req->prev_bssid.bytes, req->prev_bssid,
		     QDF_MAC_ADDR_SIZE);
}

static inline void osif_cm_dump_prev_bssid(struct cfg80211_connect_params *req)
{
	if (req->prev_bssid)
		osif_nofl_debug("prev BSSID "QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(req->prev_bssid));
}

#else
static inline void
osif_cm_set_prev_bssid(struct wlan_cm_connect_req *connect_req,
		       struct cfg80211_connect_params *req);

static inline void osif_cm_dump_prev_bssid(struct cfg80211_connect_params *req)
{
}

#endif

static inline void osif_cm_dump_connect_req(struct net_device *dev,
					    uint8_t vdev_id,
					    struct cfg80211_connect_params *req)
{
	uint32_t i;

	osif_nofl_debug("connect req for %s(vdevid-%d) freq %d SSID %.*s auth type %d WPA ver %d n_akm %d n_cipher %d grp_cipher %x mfp %d freq hint %d",
			dev->name, vdev_id,
			req->channel ? req->channel->center_freq : 0,
			(int)req->ssid_len, req->ssid, req->auth_type,
			req->crypto.wpa_versions,
			req->crypto.n_akm_suites,
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

	for (i = 0; i < req->crypto.n_akm_suites; i++)
		osif_nofl_debug("akm[%d] = %x", i, req->crypto.akm_suites[i]);

	for (i = 0; i < req->crypto.n_ciphers_pairwise; i++)
		osif_nofl_debug("cipher_pairwise[%d] = %x", i,
				req->crypto.ciphers_pairwise[i]);
}

int osif_cm_connect(struct net_device *dev, struct wlan_objmgr_vdev *vdev,
		    struct cfg80211_connect_params *req)
{
	struct wlan_cm_connect_req *connect_req;
	const u8 *bssid_hint = req->bssid_hint;
	uint8_t vdev_id = vdev->vdev_objmgr.vdev_id;
	QDF_STATUS status;

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
		qdf_mem_copy(connect_req->bssid.bytes, req->bssid_hint,
			     QDF_MAC_ADDR_SIZE);

	osif_cm_set_prev_bssid(connect_req, req);

	connect_req->ssid.length = req->ssid_len;
	if (connect_req->ssid.length > WLAN_SSID_MAX_LEN) {
		osif_err("Invalid ssid len %zu", req->ssid_len);
		return -EINVAL;
	}

	qdf_mem_copy(connect_req->ssid.ssid, req->ssid,
		     connect_req->ssid.length);

	qdf_mem_copy(connect_req->bssid_hint.bytes, bssid_hint,
		     QDF_MAC_ADDR_SIZE);

	if (req->channel)
		connect_req->chan_freq = req->channel->center_freq;
	else
		connect_req->chan_freq = 0;

	status = osif_cm_set_crypto_params(connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_start_fail;

	connect_req->ht_caps = req->ht_capa.cap_info;
	connect_req->ht_caps_mask = req->ht_capa_mask.cap_info;
	connect_req->vht_caps = req->vht_capa.vht_cap_info;
	connect_req->vht_caps_mask = req->vht_capa_mask.vht_cap_info;

	/* Copy complete ie */
	connect_req->assoc_ie.len = req->ie_len;
	connect_req->assoc_ie.ptr = qdf_mem_malloc(req->ie_len);
	if (!connect_req->assoc_ie.ptr) {
		connect_req->assoc_ie.len = 0;
		status = QDF_STATUS_E_NOMEM;
		goto connect_start_fail;
	}
	qdf_mem_copy(connect_req->assoc_ie.ptr, req->ie,
		     connect_req->assoc_ie.len);

	status = osif_cm_set_fils_info(connect_req, req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_start_fail;

	status = ucfg_cm_start_connect(vdev, connect_req);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("Connect failed with status %d", status);

connect_start_fail:
	if (connect_req->assoc_ie.ptr) {
		qdf_mem_free(connect_req->assoc_ie.ptr);
		connect_req->assoc_ie.ptr = NULL;
	}
	osif_cm_free_wep_key_params(connect_req);
	qdf_mem_free(connect_req);

	return qdf_status_to_os_return(status);
}

int osif_cm_disconnect(struct net_device *dev, struct wlan_objmgr_vdev *vdev,
		       uint16_t reason)
{
	struct wlan_cm_disconnect_req *req;
	uint8_t vdev_id = vdev->vdev_objmgr.vdev_id;
	QDF_STATUS status;

	osif_info("%s(vdevid-%d): Received Disconnect reason:%d %s",
		  dev->name, vdev_id, reason,
		  ucfg_cm_reason_code_to_str(reason));

	status = osif_cm_reset_id_and_src(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return -ENOMEM;

	req->vdev_id = vdev_id;
	req->source = CM_OSIF_DISCONNECT;
	req->reason_code = reason;
	status = ucfg_cm_start_disconnect(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("Disconnect failed with status %d", status);

	qdf_mem_free(req);

	return qdf_status_to_os_return(status);
}

