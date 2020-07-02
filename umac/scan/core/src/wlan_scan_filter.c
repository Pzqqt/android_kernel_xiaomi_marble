/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan cache filter logic
 */

#include <wlan_scan_utils_api.h>
#include "wlan_scan_main.h"
#include "wlan_scan_cache_db_i.h"

#ifdef WLAN_SCAN_SECURITY_FILTER_V1
#include "wlan_crypto_global_def.h"
#include "wlan_crypto_global_api.h"

/**
 * scm_check_open() - Check if scan entry support open authmode
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if open security else false
 */
static bool scm_check_open(struct scan_filter *filter,
			   struct scan_cache_entry *db_entry,
			   struct security_info *security)
{
	if (db_entry->cap_info.wlan_caps.privacy) {
		scm_debug("%pM : have privacy set",
			  db_entry->bssid.bytes);
		return false;
	}

	if (filter->ucastcipherset &&
	   !(QDF_HAS_PARAM(filter->ucastcipherset, WLAN_CRYPTO_CIPHER_NONE))) {
		scm_debug("%pM : Filter doesn't have CIPHER none in uc %x",
			  db_entry->bssid.bytes, filter->ucastcipherset);
		return false;
	}

	if (filter->mcastcipherset &&
	   !(QDF_HAS_PARAM(filter->mcastcipherset, WLAN_CRYPTO_CIPHER_NONE))) {
		scm_debug("%pM : Filter doesn't have CIPHER none in mc %x",
			  db_entry->bssid.bytes, filter->mcastcipherset);
		return false;
	}

	QDF_SET_PARAM(security->ucastcipherset, WLAN_CRYPTO_CIPHER_NONE);
	QDF_SET_PARAM(security->mcastcipherset, WLAN_CRYPTO_CIPHER_NONE);

	return true;
}

/**
 * scm_check_wep() - Check if scan entry support WEP authmode
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WEP security else false
 */
static bool scm_check_wep(struct scan_filter *filter,
			  struct scan_cache_entry *db_entry,
			  struct security_info *security)
{
	/* If privacy bit is not set, consider no match */
	if (!db_entry->cap_info.wlan_caps.privacy) {
		scm_debug("%pM : doesn't have privacy set",
			  db_entry->bssid.bytes);
		return false;
	}

	if (!(db_entry->security_type & SCAN_SECURITY_TYPE_WEP)) {
		scm_debug("%pM : doesn't support WEP", db_entry->bssid.bytes);
		return false;
	}

	if (!filter->ucastcipherset || !filter->mcastcipherset) {
		scm_debug("%pM : Filter uc %x or mc %x cipher are 0",
			  db_entry->bssid.bytes, filter->ucastcipherset,
			  filter->mcastcipherset);
		return false;
	}

	if (!(QDF_HAS_PARAM(filter->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP) ||
	     QDF_HAS_PARAM(filter->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_40) ||
	     QDF_HAS_PARAM(filter->ucastcipherset,
			   WLAN_CRYPTO_CIPHER_WEP_104))) {
		scm_debug("%pM : Filter doesn't have WEP cipher in uc %x",
			  db_entry->bssid.bytes, filter->ucastcipherset);
		return false;
	}

	if (!(QDF_HAS_PARAM(filter->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP) ||
	     QDF_HAS_PARAM(filter->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP_40) ||
	     QDF_HAS_PARAM(filter->mcastcipherset,
			   WLAN_CRYPTO_CIPHER_WEP_104))) {
		scm_debug("%pM : Filter doesn't have WEP cipher in mc %x",
			  db_entry->bssid.bytes, filter->mcastcipherset);
		return false;
	}

	if (QDF_HAS_PARAM(filter->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP))
		QDF_SET_PARAM(security->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP);

	if (QDF_HAS_PARAM(filter->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_40))
		QDF_SET_PARAM(security->ucastcipherset,
			      WLAN_CRYPTO_CIPHER_WEP_40);

	if (QDF_HAS_PARAM(filter->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_104))
		QDF_SET_PARAM(security->ucastcipherset,
			      WLAN_CRYPTO_CIPHER_WEP_104);

	if (QDF_HAS_PARAM(filter->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP))
		QDF_SET_PARAM(security->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP);

	if (QDF_HAS_PARAM(filter->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP_40))
		QDF_SET_PARAM(security->mcastcipherset,
			      WLAN_CRYPTO_CIPHER_WEP_40);

	if (QDF_HAS_PARAM(filter->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP_104))
		QDF_SET_PARAM(security->mcastcipherset,
			      WLAN_CRYPTO_CIPHER_WEP_104);

	return true;
}

/**
 * scm_chk_if_cipher_n_akm_match() - Check if akm and ciphers match
 * @filter: scan filter
 * @ap_crypto: aps crypto params
 *
 * Return: true if matches
 */
static bool scm_chk_if_cipher_n_akm_match(struct scan_filter *filter,
					  struct wlan_crypto_params *ap_crypto)
{
	/* Check AP's pairwise ciphers.*/
	if (!(filter->ucastcipherset & ap_crypto->ucastcipherset))
		return false;

	/* Check AP's group cipher match.*/
	if (!(filter->mcastcipherset & ap_crypto->mcastcipherset))
		return false;

	/* Check AP's AKM match with filter's AKM.*/
	if (!(filter->key_mgmt & ap_crypto->key_mgmt))
		return false;

	/* Check AP's mgmt cipher match if present.*/
	if ((filter->mgmtcipherset && ap_crypto->mgmtcipherset) &&
	    !(filter->mgmtcipherset & ap_crypto->mgmtcipherset))
		return false;

	if (filter->ignore_pmf_cap)
		return true;

	if (filter->pmf_cap == WLAN_PMF_REQUIRED &&
	    !(ap_crypto->rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED))
		return false;

	if (filter->pmf_cap == WLAN_PMF_DISABLED &&
	    (ap_crypto->rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED))
		return false;

	return true;
}

static bool scm_chk_crypto_params(struct scan_filter *filter,
				  struct wlan_crypto_params *ap_crypto,
				  bool is_adaptive_11r,
				  struct scan_cache_entry *db_entry,
				  struct security_info *security)
{
	if (!scm_chk_if_cipher_n_akm_match(filter, ap_crypto)) {
		scm_debug("%pM: fail. adaptive 11r %d Self: AKM %x CIPHER: mc %x uc %x mgmt %x pmf %d AP: AKM %x CIPHER: mc %x uc %x mgmt %x, RSN caps %x",
			  db_entry->bssid.bytes, is_adaptive_11r,
			  filter->key_mgmt, filter->mcastcipherset,
			  filter->ucastcipherset, filter->mgmtcipherset,
			  filter->pmf_cap, ap_crypto->key_mgmt,
			  ap_crypto->mcastcipherset, ap_crypto->ucastcipherset,
			  ap_crypto->mgmtcipherset, ap_crypto->rsn_caps);
		return false;
	}

	security->mcastcipherset =
		ap_crypto->mcastcipherset & filter->mcastcipherset;
	security->ucastcipherset =
		ap_crypto->ucastcipherset & filter->ucastcipherset;
	security->key_mgmt = ap_crypto->key_mgmt & filter->key_mgmt;

	return true;
}

/**
 * scm_check_rsn() - Check if scan entry support RSN security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if RSN security else false
 */
static bool scm_check_rsn(struct scan_filter *filter,
			  struct scan_cache_entry *db_entry,
			  struct security_info *security)
{
	bool is_adaptive_11r;
	QDF_STATUS status;
	struct wlan_crypto_params *ap_crypto;
	bool match;

	if (!util_scan_entry_rsn(db_entry)) {
		scm_debug("%pM : doesn't have RSN IE", db_entry->bssid.bytes);
		return false;
	}

	ap_crypto = qdf_mem_malloc(sizeof(*ap_crypto));
	if (!ap_crypto)
		return false;
	status = wlan_crypto_rsnie_check(ap_crypto,
					 util_scan_entry_rsn(db_entry));
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("%pM: failed to parse RSN IE, status %d",
			db_entry->bssid.bytes, status);
		qdf_mem_free(ap_crypto);
		return false;
	}

	is_adaptive_11r = db_entry->adaptive_11r_ap &&
				filter->enable_adaptive_11r;

	/* If adaptive 11r is enabled set the FT AKM for AP */
	if (is_adaptive_11r) {
		if (QDF_HAS_PARAM(ap_crypto->key_mgmt,
				  WLAN_CRYPTO_KEY_MGMT_IEEE8021X)) {
			QDF_SET_PARAM(ap_crypto->key_mgmt,
				      WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X);
		}
		if (QDF_HAS_PARAM(ap_crypto->key_mgmt,
				  WLAN_CRYPTO_KEY_MGMT_PSK)) {
			QDF_SET_PARAM(ap_crypto->key_mgmt,
				      WLAN_CRYPTO_KEY_MGMT_FT_PSK);
		}
		if (QDF_HAS_PARAM(ap_crypto->key_mgmt,
				  WLAN_CRYPTO_KEY_MGMT_PSK_SHA256)) {
			QDF_SET_PARAM(ap_crypto->key_mgmt,
				      WLAN_CRYPTO_KEY_MGMT_FT_PSK);
		}
		if (QDF_HAS_PARAM(ap_crypto->key_mgmt,
				  WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256)) {
			QDF_SET_PARAM(ap_crypto->key_mgmt,
				      WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X);
		}
	}

	match = scm_chk_crypto_params(filter, ap_crypto, is_adaptive_11r,
				      db_entry, security);
	qdf_mem_free(ap_crypto);

	return match;
}

/**
 * scm_check_wpa() - Check if scan entry support WPA security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WPA security else false
 */
static bool scm_check_wpa(struct scan_filter *filter,
			  struct scan_cache_entry *db_entry,
			  struct security_info *security)
{
	QDF_STATUS status;
	struct wlan_crypto_params *ap_crypto;
	bool match;

	if (!util_scan_entry_wpa(db_entry)) {
		scm_debug("%pM : doesn't have WPA IE",
			  db_entry->bssid.bytes);
		return false;
	}

	ap_crypto = qdf_mem_malloc(sizeof(*ap_crypto));
	if (!ap_crypto)
		return false;

	status = wlan_crypto_wpaie_check(ap_crypto,
					 util_scan_entry_wpa(db_entry));
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("%pM: failed to parse WPA IE, status %d",
			db_entry->bssid.bytes, status);
		qdf_mem_free(ap_crypto);
		return false;
	}

	match = scm_chk_crypto_params(filter, ap_crypto, false,
				      db_entry, security);
	qdf_mem_free(ap_crypto);

	return match;
}

/**
 * scm_check_wapi() - Check if scan entry support WAPI security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WAPI security else false
 */
static bool scm_check_wapi(struct scan_filter *filter,
			   struct scan_cache_entry *db_entry,
			   struct security_info *security)
{
	QDF_STATUS status;
	struct wlan_crypto_params *ap_crypto;

	if (!util_scan_entry_wapi(db_entry)) {
		scm_debug("%pM : doesn't have WAPI IE",
			  db_entry->bssid.bytes);
		return false;
	}

	ap_crypto = qdf_mem_malloc(sizeof(*ap_crypto));
	if (!ap_crypto)
		return false;

	status = wlan_crypto_wapiie_check(ap_crypto,
					  util_scan_entry_wapi(db_entry));
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("%pM: failed to parse WAPI IE, status %d",
			db_entry->bssid.bytes, status);
		qdf_mem_free(ap_crypto);
		return false;
	}

	if (!scm_chk_if_cipher_n_akm_match(filter, ap_crypto)) {
		scm_debug("%pM: fail. Self: AKM %x CIPHER: mc %x uc %x mgmt %x pmf %d AP: AKM %x CIPHER: mc %x uc %x mgmt %x, RSN caps %x",
			  db_entry->bssid.bytes, filter->key_mgmt,
			  filter->mcastcipherset, filter->ucastcipherset,
			  filter->mgmtcipherset, filter->pmf_cap,
			  ap_crypto->key_mgmt, ap_crypto->mcastcipherset,
			  ap_crypto->ucastcipherset, ap_crypto->mgmtcipherset,
			  ap_crypto->rsn_caps);
		qdf_mem_free(ap_crypto);

		return false;
	}

	security->mcastcipherset =
		ap_crypto->mcastcipherset & filter->mcastcipherset;
	security->ucastcipherset =
		ap_crypto->ucastcipherset & filter->ucastcipherset;
	security->key_mgmt = ap_crypto->key_mgmt & filter->key_mgmt;
	qdf_mem_free(ap_crypto);

	return true;
}

/**
 * scm_match_any_security() - Check if any security in filter match
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if any security else false
 */
static bool scm_match_any_security(struct scan_filter *filter,
				   struct scan_cache_entry *db_entry,
				   struct security_info *security)
{
	struct wlan_crypto_params *ap_crypto = {0};
	QDF_STATUS status;
	bool match = false;

	ap_crypto = qdf_mem_malloc(sizeof(*ap_crypto));
	if (!ap_crypto)
		return match;

	if (util_scan_entry_rsn(db_entry)) {
		status = wlan_crypto_rsnie_check(ap_crypto,
						 util_scan_entry_rsn(db_entry));
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("%pM: failed to parse RSN IE, status %d",
				db_entry->bssid.bytes, status);
			goto free;
		}
		security->mcastcipherset = ap_crypto->mcastcipherset;
		security->ucastcipherset = ap_crypto->ucastcipherset;
		security->key_mgmt = ap_crypto->key_mgmt;
		QDF_SET_PARAM(security->authmodeset, WLAN_CRYPTO_AUTH_RSNA);
		match = true;
		goto free;
	}

	if (util_scan_entry_wpa(db_entry)) {
		status = wlan_crypto_wpaie_check(ap_crypto,
						 util_scan_entry_wpa(db_entry));
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("%pM: failed to parse WPA IE, status %d",
				db_entry->bssid.bytes, status);
			goto free;
		}
		security->mcastcipherset = ap_crypto->mcastcipherset;
		security->ucastcipherset = ap_crypto->ucastcipherset;
		security->key_mgmt = ap_crypto->key_mgmt;
		QDF_SET_PARAM(security->authmodeset, WLAN_CRYPTO_AUTH_WPA);
		match = true;
		goto free;
	}

	if (util_scan_entry_wapi(db_entry)) {
		status = wlan_crypto_wapiie_check(ap_crypto,
						util_scan_entry_wapi(db_entry));
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("%pM: failed to parse WPA IE, status %d",
				db_entry->bssid.bytes, status);
			goto free;
		}
		security->mcastcipherset = ap_crypto->mcastcipherset;
		security->ucastcipherset = ap_crypto->ucastcipherset;
		security->key_mgmt = ap_crypto->key_mgmt;
		QDF_SET_PARAM(security->authmodeset, WLAN_CRYPTO_AUTH_WAPI);
		match = true;
		goto free;
	}

	if (db_entry->cap_info.wlan_caps.privacy) {
		QDF_SET_PARAM(security->ucastcipherset, WLAN_CRYPTO_CIPHER_WEP);
		QDF_SET_PARAM(security->mcastcipherset, WLAN_CRYPTO_CIPHER_WEP);
		QDF_SET_PARAM(security->authmodeset, WLAN_CRYPTO_AUTH_SHARED);
		match = true;
		goto free;
	}

	QDF_SET_PARAM(security->ucastcipherset, WLAN_CRYPTO_CIPHER_NONE);
	QDF_SET_PARAM(security->mcastcipherset, WLAN_CRYPTO_CIPHER_NONE);
	QDF_SET_PARAM(security->authmodeset, WLAN_CRYPTO_AUTH_OPEN);
	match = true;

free:
	qdf_mem_free(ap_crypto);

	return match;
}

/**
 * scm_is_security_match() - Check if security in filter match
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if security match else false
 */
static bool scm_is_security_match(struct scan_filter *filter,
				  struct scan_cache_entry *db_entry,
				  struct security_info *security)
{
	int i;
	bool match = false;

	if (!filter->authmodeset)
		return scm_match_any_security(filter, db_entry, security);

	for (i = 0; i <= WLAN_CRYPTO_AUTH_MAX && !match; i++) {
		if (!QDF_HAS_PARAM(filter->authmodeset, i))
			continue;

		security->authmodeset = 0;
		QDF_SET_PARAM(security->authmodeset, i);

		switch (i) {
		case WLAN_CRYPTO_AUTH_NONE:
		case WLAN_CRYPTO_AUTH_OPEN:
		case WLAN_CRYPTO_AUTH_AUTO:
			match = scm_check_open(filter, db_entry, security);
			if (match)
				break;
		/* If not OPEN, then check WEP match so fall through */
		case WLAN_CRYPTO_AUTH_SHARED:
			match = scm_check_wep(filter, db_entry, security);
			break;
		case WLAN_CRYPTO_AUTH_8021X:
		case WLAN_CRYPTO_AUTH_RSNA:
		case WLAN_CRYPTO_AUTH_CCKM:
		case WLAN_CRYPTO_AUTH_SAE:
		case WLAN_CRYPTO_AUTH_FILS_SK:
			/* First check if there is a RSN match */
			match = scm_check_rsn(filter, db_entry, security);
			break;
		case WLAN_CRYPTO_AUTH_WPA:
			match = scm_check_wpa(filter, db_entry, security);
			break;
		case WLAN_CRYPTO_AUTH_WAPI:/* WAPI */
			match = scm_check_wapi(filter, db_entry, security);
			break;
		default:
			break;
		}
	}

	return match;
}

static bool scm_ignore_ssid_check_for_owe(struct scan_filter *filter,
					  struct scan_cache_entry *db_entry)
{
	if (util_scan_entry_is_hidden_ap(db_entry) &&
	    QDF_HAS_PARAM(filter->key_mgmt, WLAN_CRYPTO_KEY_MGMT_OWE) &&
	    util_is_bssid_match(&filter->bssid_hint, &db_entry->bssid))
		return true;

	return false;
}

#else

/**
 * scm_is_open_security() - Check if scan entry support open security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if open security else false
 */
static bool scm_is_open_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	bool match = false;
	int i;

	if (db_entry->cap_info.wlan_caps.privacy)
		return false;

	/* Check MC cipher and Auth type requested. */
	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		if (WLAN_ENCRYPT_TYPE_NONE ==
			filter->mc_enc_type[i]) {
			security->mc_enc =
				filter->mc_enc_type[i];
			match = true;
			break;
		}
	}
	if (!match && filter->num_of_mc_enc_type)
		return match;

	match = false;
	/* Check Auth list. It should contain AuthOpen. */
	for (i = 0; i < filter->num_of_auth; i++) {
		if ((WLAN_AUTH_TYPE_OPEN_SYSTEM ==
			filter->auth_type[i]) ||
			(WLAN_AUTH_TYPE_AUTOSWITCH ==
			filter->auth_type[i])) {
			security->auth_type =
				WLAN_AUTH_TYPE_OPEN_SYSTEM;
			match = true;
			break;
		}
	}

	return match;
}

/**
 * scm_is_cipher_match() - Check if cipher match the cipher list
 * @cipher_list: cipher list to match
 * @num_cipher: number of cipher in cipher list
 * @cipher_to_match: cipher to found in cipher list
 *
 * Return: true if open security else false
 */
static bool scm_is_cipher_match(
	uint32_t *cipher_list,
	uint16_t num_cipher, uint32_t cipher_to_match)
{
	int i;
	bool match = false;

	for (i = 0; i < num_cipher ; i++) {
		match = (cipher_list[i] == cipher_to_match);
		if (match)
			break;
	}

	return match;
}

/**
 * scm_get_cipher_suite_type() - get cypher suite type from enc type
 * @enc: enc type
 *
 * Return: cypher suite type
 */
static uint8_t scm_get_cipher_suite_type(enum wlan_enc_type enc)
{
	uint8_t cipher_type;

	switch (enc) {
	case WLAN_ENCRYPT_TYPE_WEP40:
	case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		cipher_type = WLAN_CSE_WEP40;
		break;
	case WLAN_ENCRYPT_TYPE_WEP104:
	case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		cipher_type = WLAN_CSE_WEP104;
		break;
	case WLAN_ENCRYPT_TYPE_TKIP:
		cipher_type = WLAN_CSE_TKIP;
		break;
	case WLAN_ENCRYPT_TYPE_AES:
		cipher_type = WLAN_CSE_CCMP;
		break;
	case WLAN_ENCRYPT_TYPE_AES_GCMP:
		cipher_type = WLAN_CSE_GCMP_128;
		break;
	case WLAN_ENCRYPT_TYPE_AES_GCMP_256:
		cipher_type = WLAN_CSE_GCMP_256;
		break;
	case WLAN_ENCRYPT_TYPE_NONE:
		cipher_type = WLAN_CSE_NONE;
		break;
	case WLAN_ENCRYPT_TYPE_WPI:
		cipher_type = WLAN_WAI_CERT_OR_SMS4;
		break;
	default:
		cipher_type = WLAN_CSE_RESERVED;
		break;
	}

	return cipher_type;
}

/**
 * scm_is_wep_security() - Check if scan entry support WEP security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WEP security else false
 */
static bool scm_is_wep_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	QDF_STATUS status;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_AUTH_TYPE_OPEN_SYSTEM;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;

	if (!security)
		return false;

	/* If privacy bit is not set, consider no match */
	if (!db_entry->cap_info.wlan_caps.privacy)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		switch (filter->mc_enc_type[i]) {
		case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP40:
		case WLAN_ENCRYPT_TYPE_WEP104:
			/*
			 * Multicast list may contain WEP40/WEP104.
			 * Check whether it matches UC.
			 */
			if (security->uc_enc ==
			   filter->mc_enc_type[i]) {
				match = true;
				neg_mccipher =
				   filter->mc_enc_type[i];
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

	for (i = 0; i < filter->num_of_auth; i++) {
		switch (filter->auth_type[i]) {
		case WLAN_AUTH_TYPE_OPEN_SYSTEM:
		case WLAN_AUTH_TYPE_SHARED_KEY:
		case WLAN_AUTH_TYPE_AUTOSWITCH:
			match = true;
			neg_auth = filter->auth_type[i];
			break;
		default:
			match = false;
		}
		if (match)
			break;
	}

	if (!match)
		return match;

	/*
	 * In case of WPA / WPA2, check whether it supports WEP as well.
	 * Prepare the encryption type for WPA/WPA2 functions
	 */
	if (security->uc_enc == WLAN_ENCRYPT_TYPE_WEP40_STATICKEY)
		security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40;
	else if (security->uc_enc == WLAN_ENCRYPT_TYPE_WEP104)
		security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104;

	/* else we can use the encryption type directly */
	if (util_scan_entry_wpa(db_entry)) {
		struct wlan_wpa_ie wpa = {0};
		uint8_t cipher_type;

		cipher_type =
			scm_get_cipher_suite_type(security->uc_enc);
		status = wlan_parse_wpa_ie(util_scan_entry_wpa(db_entry), &wpa);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("failed to parse WPA IE, status %d", status);
			scm_hex_dump(QDF_TRACE_LEVEL_DEBUG,
				     util_scan_entry_wpa(db_entry),
				     util_scan_get_wpa_len(db_entry));
			return false;
		}

		match = scm_is_cipher_match(&wpa.mc_cipher,
				  1, WLAN_WPA_SEL(cipher_type));
	}
	if (!match && util_scan_entry_rsn(db_entry)) {
		struct wlan_rsn_ie rsn = {0};
		uint8_t cipher_type;

		cipher_type =
			scm_get_cipher_suite_type(security->uc_enc);
		status = wlan_parse_rsn_ie(util_scan_entry_rsn(db_entry), &rsn);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("failed to parse RSN IE, status %d", status);
			scm_hex_dump(QDF_TRACE_LEVEL_DEBUG,
				     util_scan_entry_rsn(db_entry),
				     util_scan_get_rsn_len(db_entry));
			return false;
		}
		match = scm_is_cipher_match(&rsn.gp_cipher_suite,
				  1, WLAN_RSN_SEL(cipher_type));
	}


	if (match) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_check_pmf_match() - Check PMF security of entry match filter
 * @filter: scan filter
 * @db_entry: ap entry
 * @rsn: rsn IE of the scan entry
 *
 * Return: true if PMF security match else false
 */
static bool
scm_check_pmf_match(struct scan_filter *filter,
		    struct scan_cache_entry *db_entry,
		    struct wlan_rsn_ie *rsn)
{
	enum wlan_pmf_cap ap_pmf_cap = WLAN_PMF_DISABLED;
	bool match = true;

	if (rsn->cap & RSN_CAP_MFP_CAPABLE)
		ap_pmf_cap = WLAN_PMF_CAPABLE;
	if (rsn->cap & RSN_CAP_MFP_REQUIRED)
		ap_pmf_cap = WLAN_PMF_REQUIRED;

	if ((filter->pmf_cap == WLAN_PMF_REQUIRED) &&
		(ap_pmf_cap == WLAN_PMF_DISABLED))
		match = false;
	else if ((filter->pmf_cap == WLAN_PMF_DISABLED) &&
		(ap_pmf_cap == WLAN_PMF_REQUIRED))
		match = false;

	if (!match)
		scm_debug("%pM : PMF cap didn't match (filter %d AP %d)",
			  db_entry->bssid.bytes, filter->pmf_cap,
			  ap_pmf_cap);

	return match;
}

/**
 * scm_is_rsn_mcast_cipher_match() - match the rsn mcast cipher type with AP's
 * mcast cipher
 * @rsn: AP's RSNE
 * @filter: scan filter
 * @neg_mccipher: negotiated mc cipher if matched.
 *
 * Return: true if mc cipher is negotiated
 */
static bool
scm_is_rsn_mcast_cipher_match(struct wlan_rsn_ie *rsn,
	struct scan_filter *filter, enum wlan_enc_type *neg_mccipher)
{
	int i;
	bool match;
	uint8_t cipher_type;

	if (!rsn || !neg_mccipher || !filter)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {

		if (filter->mc_enc_type[i] == WLAN_ENCRYPT_TYPE_ANY) {
			/* Try the more secured ones first. */
			/* Check GCMP_256 first */
			cipher_type = WLAN_CSE_GCMP_256;
			match = scm_is_cipher_match(&rsn->gp_cipher_suite, 1,
						    WLAN_RSN_SEL(cipher_type));
			if (match) {
				*neg_mccipher = WLAN_ENCRYPT_TYPE_AES_GCMP_256;
				return true;
			}
			/* Check GCMP */
			cipher_type = WLAN_CSE_GCMP_128;
			match = scm_is_cipher_match(&rsn->gp_cipher_suite, 1,
						    WLAN_RSN_SEL(cipher_type));
			if (match) {
				*neg_mccipher = WLAN_ENCRYPT_TYPE_AES_GCMP;
				return true;
			}
			/* Check AES */
			cipher_type = WLAN_CSE_CCMP;
			match = scm_is_cipher_match(&rsn->gp_cipher_suite, 1,
						    WLAN_RSN_SEL(cipher_type));
			if (match) {
				*neg_mccipher = WLAN_ENCRYPT_TYPE_AES;
				return true;
			}
			/* Check TKIP */
			cipher_type = WLAN_CSE_TKIP;
			match = scm_is_cipher_match(&rsn->gp_cipher_suite, 1,
						    WLAN_RSN_SEL(cipher_type));
			if (match) {
				*neg_mccipher = WLAN_ENCRYPT_TYPE_TKIP;
				return true;
			}
		} else {
			cipher_type =
			     scm_get_cipher_suite_type(filter->mc_enc_type[i]);
			match = scm_is_cipher_match(&rsn->gp_cipher_suite, 1,
						    WLAN_RSN_SEL(cipher_type));
			if (match) {
				*neg_mccipher = filter->mc_enc_type[i];
				return true;
			}
		}
	}

	return false;
}

/**
 * scm_is_rsn_security() - Check if scan entry support RSN security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if RSN security else false
 */
static bool scm_is_rsn_security(struct scan_filter *filter,
				struct scan_cache_entry *db_entry,
				struct security_info *security)
{
	int i;
	uint8_t cipher_type;
	bool match_any_akm, match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_auth_type filter_akm;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_rsn_ie rsn = {0};
	QDF_STATUS status;
	bool is_adaptive_11r;

	if (!security)
		return false;
	if (!util_scan_entry_rsn(db_entry)) {
		scm_debug("%pM : doesn't have RSN IE", db_entry->bssid.bytes);
		return false;
	}
	status = wlan_parse_rsn_ie(util_scan_entry_rsn(db_entry), &rsn);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("failed to parse RSN IE, status %d", status);
		scm_hex_dump(QDF_TRACE_LEVEL_DEBUG,
			     util_scan_entry_rsn(db_entry),
			     util_scan_get_rsn_len(db_entry));
		return false;
	}

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(rsn.pwise_cipher_suites,
		rsn.pwise_cipher_count, WLAN_RSN_SEL(cipher_type));
	if (!match) {
		scm_debug("%pM : pairwise cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	match = scm_is_rsn_mcast_cipher_match(&rsn, filter, &neg_mccipher);
	if (!match) {
		scm_debug("%pM : mcast cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	is_adaptive_11r = (db_entry->adaptive_11r_ap &&
			   filter->enable_adaptive_11r);

	/* Initializing with false as it has true value already */
	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {

		filter_akm = filter->auth_type[i];
		if (filter_akm == WLAN_AUTH_TYPE_ANY)
			match_any_akm = true;
		else
			match_any_akm = false;
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_FT_SHA384))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FT_FILS_SHA384)) {
				neg_auth = WLAN_AUTH_TYPE_FT_FILS_SHA384;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_FT_SHA256))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FT_FILS_SHA256)) {
				neg_auth = WLAN_AUTH_TYPE_FT_FILS_SHA256;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_SHA384))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FILS_SHA384)) {
				neg_auth = WLAN_AUTH_TYPE_FILS_SHA384;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FILS_SHA256))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FILS_SHA256)) {
				neg_auth = WLAN_AUTH_TYPE_FILS_SHA256;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites,
		    rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SAE))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_SAE)) {
				neg_auth = WLAN_AUTH_TYPE_SAE;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count, WLAN_RSN_DPP_AKM)) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_DPP_RSN)) {
				neg_auth = WLAN_AUTH_TYPE_DPP_RSN;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
					rsn.akm_suite_count,
					WLAN_RSN_OSEN_AKM)) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_OSEN)) {
				neg_auth = WLAN_AUTH_TYPE_OSEN;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_OWE))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_OWE)) {
				neg_auth = WLAN_AUTH_TYPE_OWE;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FT_IEEE8021X))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FT_RSN)) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FT_PSK))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FT_RSN_PSK)) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN_PSK;
				match = true;
				break;
			}
		}
		/* ESE only supports 802.1X.  No PSK. */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_CCKM_AKM)) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_CCKM_RSN)) {
				neg_auth = WLAN_AUTH_TYPE_CCKM_RSN;
				match = true;
				break;
			}
		}
		/* RSN */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_IEEE8021X))) {
			if (is_adaptive_11r &&
			    (filter_akm == WLAN_AUTH_TYPE_FT_RSN)) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN;
				match = true;
				break;
			}

			if (match_any_akm ||
			    (WLAN_AUTH_TYPE_RSN == filter_akm)) {
				neg_auth = WLAN_AUTH_TYPE_RSN;
				match = true;
				break;
			}
		}
		/* TKIP */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_PSK))) {
			if (is_adaptive_11r &&
			    (filter_akm == WLAN_AUTH_TYPE_FT_RSN_PSK)) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN_PSK;
				match = true;
				break;
			}

			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_RSN_PSK)) {
				neg_auth = WLAN_AUTH_TYPE_RSN_PSK;
				match = true;
				break;
			}
		}
		/* SHA256 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SHA256_PSK))) {
			if (is_adaptive_11r &&
			    (filter_akm == WLAN_AUTH_TYPE_FT_RSN_PSK)) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN_PSK;
				match = true;
				break;
			}

			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_RSN_PSK_SHA256)) {
				neg_auth = WLAN_AUTH_TYPE_RSN_PSK_SHA256;
				match = true;
				break;
			}
		}
		/* 8021X SHA256 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SHA256_IEEE8021X))) {
			if (is_adaptive_11r &&
			    (filter_akm == WLAN_AUTH_TYPE_FT_RSN)) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN;
				match = true;
				break;
			}

			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_RSN_8021X_SHA256)) {
				neg_auth = WLAN_AUTH_TYPE_RSN_8021X_SHA256;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SUITEB_EAP_SHA256))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_SUITEB_EAP_SHA256)) {
				neg_auth = WLAN_AUTH_TYPE_SUITEB_EAP_SHA256;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SUITEB_EAP_SHA384))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_SUITEB_EAP_SHA384)) {
				neg_auth = WLAN_AUTH_TYPE_SUITEB_EAP_SHA384;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites, rsn.akm_suite_count,
					WLAN_RSN_SEL(WLAN_AKM_FT_SAE))) {
			if (match_any_akm ||
			    (filter_akm == WLAN_AUTH_TYPE_FT_SAE)) {
				neg_auth = WLAN_AUTH_TYPE_FT_SAE;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites, rsn.akm_suite_count,
					WLAN_RSN_SEL(
					WLAN_AKM_FT_SUITEB_EAP_SHA384))) {
			if (match_any_akm ||
			    (filter_akm ==
			     WLAN_AUTH_TYPE_FT_SUITEB_EAP_SHA384)) {
				neg_auth = WLAN_AUTH_TYPE_FT_SUITEB_EAP_SHA384;
				match = true;
				break;
			}
		}
	}

	if (!match) {
		scm_debug("%pM : akm suites didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	if (!filter->ignore_pmf_cap)
		match = scm_check_pmf_match(filter, db_entry, &rsn);

	if (match) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_is_wpa_mcast_cipher_match() - match the wpa mcast cipher type with AP's
 * mcast cipher
 * @wpa: AP's WPA IE
 * @filter: scan filter
 * @neg_mccipher: negotiated mc cipher if matched.
 *
 * Return: true if mc cipher is negotiated
 */
static bool
scm_is_wpa_mcast_cipher_match(struct wlan_wpa_ie *wpa,
	struct scan_filter *filter, enum wlan_enc_type *neg_mccipher)
{
	int i;
	bool match;
	uint8_t cipher_type;

	if (!wpa || !neg_mccipher || !filter)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {

		if (filter->mc_enc_type[i] == WLAN_ENCRYPT_TYPE_ANY) {
			/* Try the more secured ones first. */

			/* Check AES */
			cipher_type = WLAN_CSE_CCMP;
			match = scm_is_cipher_match(&wpa->mc_cipher, 1,
						    WLAN_WPA_SEL(cipher_type));
			if (match) {
				*neg_mccipher = WLAN_ENCRYPT_TYPE_AES;
				return true;
			}
			/* Check TKIP */
			cipher_type = WLAN_CSE_TKIP;
			match = scm_is_cipher_match(&wpa->mc_cipher, 1,
						    WLAN_WPA_SEL(cipher_type));
			if (match) {
				*neg_mccipher = WLAN_ENCRYPT_TYPE_TKIP;
				return true;
			}
		} else {
			cipher_type =
			     scm_get_cipher_suite_type(filter->mc_enc_type[i]);
			match = scm_is_cipher_match(&wpa->mc_cipher, 1,
						    WLAN_WPA_SEL(cipher_type));
			if (match) {
				*neg_mccipher = filter->mc_enc_type[i];
				return true;
			}
		}
	}

	return false;
}

/**
 * scm_is_wpa_security() - Check if scan entry support WPA security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WPA security else false
 */
static bool scm_is_wpa_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	QDF_STATUS status;
	uint8_t cipher_type;
	bool match_any_akm, match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_wpa_ie wpa = {0};

	if (!security)
		return false;
	if (!util_scan_entry_wpa(db_entry)) {
		scm_debug("%pM : AP doesn't have WPA IE",
			  db_entry->bssid.bytes);
		return false;
	}

	status = wlan_parse_wpa_ie(util_scan_entry_wpa(db_entry), &wpa);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("failed to parse WPA IE, status %d", status);
		scm_hex_dump(QDF_TRACE_LEVEL_DEBUG,
			     util_scan_entry_wpa(db_entry),
			     util_scan_get_wpa_len(db_entry));
		return false;
	}

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(wpa.uc_ciphers,
		wpa.uc_cipher_count, WLAN_WPA_SEL(cipher_type));
	if (!match) {
		scm_debug("%pM : unicase cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	match = scm_is_wpa_mcast_cipher_match(&wpa, filter, &neg_mccipher);
	if (!match) {
		scm_debug("%pM : mcast cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	/* Initializing with false as it has true value already */
	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {

		if (filter->auth_type[i] == WLAN_AUTH_TYPE_ANY)
			match_any_akm = true;
		else
			match_any_akm = false;
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		/**/
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_SEL(WLAN_AKM_IEEE8021X))) {
			if (match_any_akm || (WLAN_AUTH_TYPE_WPA ==
			    filter->auth_type[i])) {
				neg_auth = WLAN_AUTH_TYPE_WPA;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_SEL(WLAN_AKM_PSK))) {
			if (match_any_akm || (WLAN_AUTH_TYPE_WPA_PSK ==
			    filter->auth_type[i])) {
				neg_auth = WLAN_AUTH_TYPE_WPA_PSK;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_CCKM_AKM)) {
			if (match_any_akm || (WLAN_AUTH_TYPE_CCKM_WPA ==
			    filter->auth_type[i])) {
				neg_auth = WLAN_AUTH_TYPE_CCKM_WPA;
				match = true;
				break;
			}
		}
	}

	if (!match)
		scm_debug("%pM : akm didn't match", db_entry->bssid.bytes);

	if (match) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_is_wapi_security() - Check if scan entry support WAPI security
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if WAPI security else false
 */
static bool scm_is_wapi_security(struct scan_filter *filter,
	struct scan_cache_entry *db_entry,
	struct security_info *security)
{
	int i;
	uint8_t cipher_type;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_wapi_ie wapi = {0};

	if (!security)
		return false;
	if (!util_scan_entry_wapi(db_entry)) {
		scm_debug("%pM : mcast cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	wlan_parse_wapi_ie(
		   util_scan_entry_wapi(db_entry), &wapi);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(wapi.uc_cipher_suites,
		wapi.uc_cipher_count, WLAN_WAPI_SEL(cipher_type));
	if (!match) {
		scm_debug("%pM : unicast cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
		  scm_get_cipher_suite_type(
		  filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&wapi.mc_cipher_suite,
				  1, WLAN_WAPI_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match) {
		scm_debug("%pM : mcast cipher didn't match",
			  db_entry->bssid.bytes);
		return false;
	}
	neg_mccipher = filter->mc_enc_type[i];

	if (scm_is_cipher_match(wapi.akm_suites,
	   wapi.akm_suite_count,
	   WLAN_WAPI_SEL(WLAN_WAI_CERT_OR_SMS4))) {
		neg_auth =
			WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	} else if (scm_is_cipher_match(wapi.akm_suites,
	   wapi.akm_suite_count, WLAN_WAPI_SEL(WLAN_WAI_PSK))) {
		neg_auth = WLAN_AUTH_TYPE_WAPI_WAI_PSK;
	} else {
		scm_debug("%pM : akm is not supported",
			  db_entry->bssid.bytes);
		return false;
	}

	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		if (filter->auth_type[i] == neg_auth) {
			match = true;
			break;
		}
	}

	if (!match)
		scm_debug("%pM : akm suite didn't match",
			  db_entry->bssid.bytes);
	if (match) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_is_def_security() - Check if any security in filter match
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if any security else false
 */
static bool scm_is_def_security(struct scan_filter *filter,
				struct scan_cache_entry *db_entry,
				struct security_info *security)
{

	/* It is allowed to match anything. Try the more secured ones first. */
	/* Check GCMP_256 first */
	security->uc_enc = WLAN_ENCRYPT_TYPE_AES_GCMP_256;
	if (scm_is_rsn_security(filter, db_entry, security))
		return true;

	/* Check GCMP */
	security->uc_enc = WLAN_ENCRYPT_TYPE_AES_GCMP;
	if (scm_is_rsn_security(filter, db_entry, security))
		return true;

	/* Check AES */
	security->uc_enc = WLAN_ENCRYPT_TYPE_AES;
	if (scm_is_rsn_security(filter, db_entry, security))
		return true;
	if (scm_is_wpa_security(filter, db_entry, security))
		return true;

	/* Check TKIP */
	security->uc_enc = WLAN_ENCRYPT_TYPE_TKIP;
	if (scm_is_rsn_security(filter, db_entry, security))
		return true;
	if (scm_is_wpa_security(filter, db_entry, security))
		return true;

	/* Check AES */
	security->uc_enc = WLAN_ENCRYPT_TYPE_AES;
	if (scm_is_wpa_security(filter, db_entry, security))
		return true;

	/* Check TKIP */
	security->uc_enc = WLAN_ENCRYPT_TYPE_TKIP;
	if (scm_is_wpa_security(filter, db_entry, security))
		return true;

	/* Check WAPI */
	security->uc_enc = WLAN_ENCRYPT_TYPE_WPI;
	if (scm_is_wapi_security(filter, db_entry, security))
		return true;

	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104;
	if (scm_is_wep_security(filter, db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40;
	if (scm_is_wep_security(filter, db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104_STATICKEY;
	if (scm_is_wep_security(filter, db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40_STATICKEY;
	if (scm_is_wep_security(filter, db_entry, security))
		return true;

	/* It must be open and no enc */
	if (db_entry->cap_info.wlan_caps.privacy)
		return false;

	security->auth_type = WLAN_AUTH_TYPE_OPEN_SYSTEM;
	security->mc_enc = WLAN_ENCRYPT_TYPE_NONE;
	security->uc_enc = WLAN_ENCRYPT_TYPE_NONE;

	return true;
}

/**
 * scm_is_security_match() - Check if security in filter match
 * @filter: scan filter
 * @db_entry: db entry
 * @security: matched security.
 *
 * Return: true if security match else false
 */
static bool scm_is_security_match(struct scan_filter *filter,
				  struct scan_cache_entry *db_entry,
				  struct security_info *security)
{
	int i;
	bool match = false;
	struct security_info local_security = {0};

	if (!filter->num_of_enc_type)
		return true;

	for (i = 0; (i < filter->num_of_enc_type) &&
	    !match; i++) {

		local_security.uc_enc =
			filter->enc_type[i];

		switch (filter->enc_type[i]) {
		case WLAN_ENCRYPT_TYPE_NONE:
			match = scm_is_open_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		case WLAN_ENCRYPT_TYPE_WEP40:
		case WLAN_ENCRYPT_TYPE_WEP104:
			match = scm_is_wep_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_TKIP:
		case WLAN_ENCRYPT_TYPE_AES:
		case WLAN_ENCRYPT_TYPE_AES_GCMP:
		case WLAN_ENCRYPT_TYPE_AES_GCMP_256:
			/* First check if there is a RSN match */
			match = scm_is_rsn_security(filter, db_entry,
						    &local_security);
			/* If not RSN, then check WPA match */
			if (!match)
				match = scm_is_wpa_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_WPI:/* WAPI */
			match = scm_is_wapi_security(filter,
				    db_entry, &local_security);
			break;
		case WLAN_ENCRYPT_TYPE_ANY:
		default:
			match  = scm_is_def_security(filter, db_entry,
						     &local_security);
			break;
		}
	}

	if (match && security)
		qdf_mem_copy(security, &local_security, sizeof(*security));

	return match;
}

static bool scm_ignore_ssid_check_for_owe(struct scan_filter *filter,
					  struct scan_cache_entry *db_entry)
{
	int i;

	if (util_scan_entry_is_hidden_ap(db_entry)) {
		for (i = 0; i < filter->num_of_auth; i++) {
			if (filter->auth_type[i] == WLAN_AUTH_TYPE_OWE &&
			    util_is_bssid_match(&filter->bssid_hint,
						&db_entry->bssid)) {
				return true;
			}
		}
	}

	return false;
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
/**
 * scm_is_fils_config_match() - Check if FILS config matches
 * @filter: scan filter
 * @db_entry: db entry
 *
 * Return: true if FILS config matches else false
 */
static bool scm_is_fils_config_match(struct scan_filter *filter,
				     struct scan_cache_entry *db_entry)
{
	int i;
	struct fils_indication_ie *indication_ie;
	uint8_t *data;

	if (!filter->fils_scan_filter.realm_check)
		return true;

	if (!db_entry->ie_list.fils_indication)
		return false;

	indication_ie =
		(struct fils_indication_ie *)db_entry->ie_list.fils_indication;

	data = indication_ie->variable_data;
	if (indication_ie->is_cache_id_present)
		data += CACHE_IDENTIFIER_LEN;

	if (indication_ie->is_hessid_present)
		data += HESSID_LEN;

	for (i = 1; i <= indication_ie->realm_identifiers_cnt; i++) {
		if (!qdf_mem_cmp(filter->fils_scan_filter.fils_realm,
				 data, REAM_HASH_LEN))
			return true;
		/* Max realm count reached */
		if (indication_ie->realm_identifiers_cnt == i)
			break;

		data = data + REAM_HASH_LEN;
	}

	return false;
}

#else

static inline bool scm_is_fils_config_match(struct scan_filter *filter,
					    struct scan_cache_entry *db_entry)
{
	return true;
}
#endif


bool scm_filter_match(struct wlan_objmgr_psoc *psoc,
		      struct scan_cache_entry *db_entry,
		      struct scan_filter *filter,
		      struct security_info *security)
{
	int i;
	bool match = false;
	struct scan_default_params *def_param;

	def_param = wlan_scan_psoc_get_def_params(psoc);
	if (!def_param)
		return false;

	if (filter->age_threshold && filter->age_threshold <
					util_scan_entry_age(db_entry))
		return false;

	if (db_entry->ssid.length) {
		for (i = 0; i < filter->num_of_ssid; i++) {
			if (util_is_ssid_match(&filter->ssid_list[i],
			   &db_entry->ssid)) {
				match = true;
				break;
			}
		}
	}
	/*
	 * In OWE transition mode, ssid is hidden. And supplicant does not issue
	 * scan with specific ssid prior to connect as in other hidden ssid
	 * cases. Add explicit check to allow OWE when ssid is hidden.
	 */
	if (!match)
		match = scm_ignore_ssid_check_for_owe(filter, db_entry);

	if (!match && filter->num_of_ssid)
		return false;

	match = false;
	/* TO do Fill p2p MAC*/
	for (i = 0; i < filter->num_of_bssid; i++) {
		if (util_is_bssid_match(&filter->bssid_list[i],
		   &db_entry->bssid)) {
			match = true;
			break;
		}
		/* TODO match p2p mac */
	}
	if (!match && filter->num_of_bssid)
		return false;

	match = false;
	for (i = 0; i < filter->num_of_channels; i++) {
		if (!filter->chan_freq_list[i] ||
		    filter->chan_freq_list[i] ==
		    db_entry->channel.chan_freq) {
			match = true;
			break;
		}
	}

	if (!match && filter->num_of_channels)
		return false;

	if (filter->rrm_measurement_filter)
		return true;

	if (!filter->ignore_auth_enc_type &&
	    !scm_is_security_match(filter, db_entry, security)) {
		scm_debug("%pM : Ignore as security profile didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	/* Match realm */
	if (!scm_is_fils_config_match(filter, db_entry)) {
		scm_debug("%pM :Ignore as fils config didn't match",
			  db_entry->bssid.bytes);
		return false;
	}

	if (!util_mdie_match(filter->mobility_domain,
	   (struct rsn_mdie *)db_entry->ie_list.mdie)) {
		scm_debug("%pM : Ignore as mdie didn't match",
			  db_entry->bssid.bytes);
		return false;
	}
	return true;
}
