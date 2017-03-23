/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan cache operations
 */

#include <qdf_status.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_utils_api.h>
#include "wlan_scan_cache_db.h"
#include "wlan_scan_main.h"
#include "wlan_scan_cache_db_i.h"

/**
 * scm_get_altered_rssi() - Artificially increase/decrease RSSI
 * @params: scan params
 * @rssi: Actual RSSI of the AP.
 * @channel_id: Channel on which the AP is parked.
 * @bssid: BSSID of the AP to connect to.
 *
 * This routine will apply the boost and penalty parameters
 * if the channel_id is of 5G band and it will also apply
 * the preferred bssid score if there is a match between
 * the bssid and the global preferred bssid list.
 *
 * Return: The modified RSSI Value
 */
static int scm_get_altered_rssi(struct scan_default_params *params,
	int rssi, uint8_t channel_id, struct qdf_mac_addr *bssid)
{
	int modified_rssi;
	int boost_factor;
	int penalty_factor;
	int i;
	struct roam_filter_params *roam_params;

	roam_params = &params->roam_params;
	modified_rssi = rssi;

	/*
	 * If the 5G pref feature is enabled, apply the roaming
	 * parameters to boost or penalize the rssi.
	 * Boost Factor = boost_factor * (Actual RSSI - boost Threshold)
	 * Penalty Factor = penalty factor * (penalty threshold - Actual RSSI)
	 */
	if (roam_params->is_5g_pref_enabled &&
			WLAN_CHAN_IS_2GHZ(channel_id)) {
		if (rssi > roam_params->raise_rssi_thresh_5g) {
			/* Check and boost the threshold*/
			boost_factor = roam_params->raise_factor_5g *
				(rssi - roam_params->raise_rssi_thresh_5g);
			/* Check and penalize the threshold */
			modified_rssi += QDF_MIN(roam_params->max_raise_rssi_5g,
				boost_factor);
		} else if (rssi < roam_params->drop_rssi_thresh_5g) {
			penalty_factor = roam_params->drop_factor_5g *
				(roam_params->drop_rssi_thresh_5g - rssi);
			modified_rssi -= QDF_MIN(roam_params->max_drop_rssi_5g,
				penalty_factor);
		}
	}
	/*
	 * Check if there are preferred bssid and then apply the
	 * preferred score
	 */
	if (bssid && roam_params->num_bssid_favored &&
	   (roam_params->num_bssid_favored <= MAX_FAVORED_BSSID)) {
		for (i = 0; i < roam_params->num_bssid_favored; i++) {
			if (!qdf_is_macaddr_equal(
			   &roam_params->bssid_favored[i], bssid))
				continue;
			modified_rssi +=
				roam_params->bssid_favored_factor[i];
		}
	}

	return modified_rssi;
}

/**
 * scm_is_better_rssi() - Is bss1 better than bss2
 * @params: scan params
 * @bss1: Pointer to the first BSS.
 * @bss2: Pointer to the second BSS.
 *
 * This routine helps in determining the preference value
 * of a particular BSS in the scan result which is further
 * used in the sorting logic of the final candidate AP's.
 *
 * Return: true, if bss1 is better than bss2
 *         false, if bss2 is better than bss1.
 */
static bool scm_is_better_rssi(struct scan_default_params *params,
	struct scan_cache_entry *bss1, struct scan_cache_entry *bss2)
{
	bool ret;
	int rssi1, rssi2;
	struct qdf_mac_addr local_mac;

	rssi1 = bss1->rssi_raw;
	rssi2 = bss2->rssi_raw;
	/*
	 * Apply the boost and penlty logic and check
	 * which is the best RSSI
	 */
	qdf_mem_copy(local_mac.bytes,
		bss1->bssid.bytes, QDF_MAC_ADDR_SIZE);
	rssi1 = scm_get_altered_rssi(params, rssi1,
			bss1->channel.chan_idx,
			&local_mac);
	qdf_mem_copy(local_mac.bytes,
			bss2->bssid.bytes, QDF_MAC_ADDR_SIZE);
	rssi2 = scm_get_altered_rssi(params, rssi2,
			bss2->channel.chan_idx,
			&local_mac);
	if (rssi1 > rssi2)
		ret = true;
	else
		ret = false;

	return ret;
}

bool scm_is_better_bss(struct scan_default_params *params,
	struct scan_cache_entry *bss1,
	struct scan_cache_entry *bss2)
{
	bool ret;

	if (bss1->prefer_value > bss2->prefer_value)
		return true;

	if (bss1->prefer_value == bss2->prefer_value) {
		if (bss1->cap_val > bss2->cap_val)
			ret = true;
		else if (bss1->cap_val == bss2->cap_val) {
			if (scm_is_better_rssi(params, bss1, bss2))
				ret = true;
			else
				ret = false;
		} else {
			ret = false;
		}
	} else {
		ret = false;
	}

	return ret;
}

/**
 * scm_get_bss_prefer_value() - Get the preference value for BSS
 * @params: scan params
 * @entry: entry
 *
 * Each entry should be assigned a preference value ranging from
 * 14-0, which will be used as an RSSI bucket score while sorting the
 * scan results.
 *
 * Return: Preference value for the BSSID
 */
static uint32_t scm_get_bss_prefer_value(struct scan_default_params *params,
			struct scan_cache_entry *entry)
{
	uint32_t ret = 0;
	int modified_rssi;

	/*
	 * The RSSI does not get modified in case the 5G
	 * preference or preferred BSSID is not applicable
	 */
	modified_rssi = scm_get_altered_rssi(params,
		entry->rssi_raw, entry->channel.chan_idx,
		&entry->bssid);
	ret = scm_derive_prefer_value_from_rssi(params, modified_rssi);

	return ret;
}

/**
 * scm_get_bss_cap_value() - get bss capability value
 * @params: def scan params
 * @entry: scan entry entry
 *
 * Return: CapValue base on the capabilities of a BSS
 */
static uint32_t scm_get_bss_cap_value(struct scan_default_params *params,
	struct scan_cache_entry *entry)
{
	uint32_t ret = SCM_BSS_CAP_VALUE_NONE;

	if (params->prefer_5ghz ||
	   params->roam_params.is_5g_pref_enabled)
		if (WLAN_CHAN_IS_5GHZ(entry->channel.chan_idx))
			ret += SCM_BSS_CAP_VALUE_5GHZ;
	/*
	 * if strict select 5GHz is set then ignore
	 * the capability checking
	 */
	if (!params->select_5ghz_margin) {
		/* We only care about 11N capability */
		if (entry->ie_list.vhtcap)
			ret += SCM_BSS_CAP_VALUE_VHT;
		else if (entry->ie_list.htcap)
			ret += SCM_BSS_CAP_VALUE_HT;
		if (entry->ie_list.wmeinfo ||
		   entry->ie_list.wmeinfo) {
			ret += SCM_BSS_CAP_VALUE_WMM;
			/* TO do Give advantage to UAPSD */
		}
	}

	return ret;
}

/**
 * scm_calc_pref_val_by_pcl() - to calculate preferred value
 * @params: scan params
 * @filter: filter to find match from scan result
 * @entry: scan entry for which score needs to be calculated
 *
 * this routine calculates the new preferred value to be given to
 * provided bss if its channel falls under preferred channel list.
 * Thump rule is higer the RSSI better the boost.
 *
 * Return: success or failure
 */
static QDF_STATUS scm_calc_pref_val_by_pcl(struct scan_default_params *params,
	struct scan_filter *filter,
	struct scan_cache_entry *entry)
{
	int temp_rssi = 0, new_pref_val = 0;
	int orig_pref_val = 0;

	if (!entry)
		return QDF_STATUS_E_FAILURE;

	if (filter->num_of_bssid) {
		scm_info("filter has specific bssid, no point of boosting");
		return QDF_STATUS_SUCCESS;
	}

	if (is_channel_found_in_pcl(entry->channel.chan_idx, filter) &&
		(entry->rssi_raw > SCM_PCL_RSSI_THRESHOLD)) {
		orig_pref_val = scm_derive_prefer_value_from_rssi(params,
					entry->rssi_raw);
		temp_rssi = entry->rssi_raw +
				(SCM_PCL_ADVANTAGE/(SCM_NUM_RSSI_CAT -
							orig_pref_val));
		if (temp_rssi > 0)
			temp_rssi = 0;
		new_pref_val = scm_derive_prefer_value_from_rssi(params,
					temp_rssi);

		entry->prefer_value =
			QDF_MAX(new_pref_val, entry->prefer_value);
	}

	return QDF_STATUS_SUCCESS;
}

void scm_calculate_bss_score(struct scan_default_params *params,
	struct scan_filter *filter, struct scan_cache_entry *entry)
{
	entry->cap_val =
		scm_get_bss_cap_value(params, entry);

	entry->prefer_value =
		scm_get_bss_prefer_value(params, entry);

	if (filter->num_of_pcl_channels)
		scm_calc_pref_val_by_pcl(params, filter, entry);
}

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
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_AUTH_TYPE_OPEN_SYSTEM;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;

	/* If privacy bit is not set, consider no match */
	if (db_entry->cap_info.wlan_caps.privacy)
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
		wlan_parse_wpa_ie(
			util_scan_entry_wpa(db_entry), &wpa);

		match = scm_is_cipher_match(&wpa.mc_cipher,
				  1, WLAN_WPA_SEL(cipher_type));
	}
	if (!match && util_scan_entry_rsn(db_entry)) {
		struct wlan_rsn_ie rsn = {0};
		uint8_t cipher_type;

		cipher_type =
			scm_get_cipher_suite_type(security->uc_enc);

		wlan_parse_rsn_ie(
			   util_scan_entry_rsn(db_entry), &rsn);
		match = scm_is_cipher_match(&rsn.gp_cipher_suite,
				  1, WLAN_RSN_SEL(cipher_type));
	}


	if (match && security) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
}

/**
 * scm_check_pmf_match() - Check PMF security of entry match filter
 * @filter: scan filter
 * @rsn: rsn IE of the scan entry
 *
 * Return: true if PMF security match else false
 */
static bool
scm_check_pmf_match(struct scan_filter *filter,
	struct wlan_rsn_ie *rsn)
{
	enum wlan_pmf_cap ap_pmf_cap = WLAN_PMF_DISABLED;

	if (rsn->cap & RSN_CAP_MFP_CAPABLE)
		ap_pmf_cap = WLAN_PMF_CAPABLE;
	if (rsn->cap & RSN_CAP_MFP_CAPABLE)
		ap_pmf_cap = WLAN_PMF_REQUIRED;

	if ((filter->pmf_cap == WLAN_PMF_REQUIRED) &&
		(ap_pmf_cap == WLAN_PMF_DISABLED))
		return false;
	else if ((filter->pmf_cap == WLAN_PMF_DISABLED) &&
		(ap_pmf_cap == WLAN_PMF_REQUIRED))
		return false;

	return true;
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
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_rsn_ie rsn = {0};

	if (!util_scan_entry_rsn(db_entry))
		return false;

	wlan_parse_rsn_ie(
		   util_scan_entry_rsn(db_entry), &rsn);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(rsn.pwise_cipher_suites,
		rsn.pwise_cipher_count, WLAN_RSN_SEL(cipher_type));
	if (!match)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
			scm_get_cipher_suite_type(
				filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&rsn.gp_cipher_suite,
			   1, WLAN_RSN_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match)
		return false;

	neg_mccipher = filter->mc_enc_type[i];

	/* Initializing with false as it has true value already */
	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FT_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_FT_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN;
				match = true;
				break;
			}
		}

		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_FT_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_FT_RSN_PSK ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_FT_RSN_PSK;
				match = true;
				break;
			}
		}
		/* ESE only supports 802.1X.  No PSK. */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_CCKM_AKM)) {
			if (WLAN_AUTH_TYPE_CCKM_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_CCKM_RSN;
				match = true;
				break;
			}
		}
		/* RSN */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_RSN ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_RSN;
				match = true;
				break;
			}
		}
		/* TKIP */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_PSK))) {
			if (WLAN_AUTH_TYPE_RSN_PSK ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_RSN_PSK;
				match = true;
				break;
			}
		}
		/* SHA256 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SHA256_PSK))) {
			if (WLAN_AUTH_TYPE_RSN_PSK_SHA256 ==
			   filter->auth_type[i]) {
				neg_auth =
					WLAN_AUTH_TYPE_RSN_PSK_SHA256;
				match = true;
				break;
			}
		}
		/* 8021X SHA256 */
		if (scm_is_cipher_match(rsn.akm_suites,
		   rsn.akm_suite_count,
		   WLAN_RSN_SEL(WLAN_AKM_SHA256_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_RSN_8021X_SHA256 ==
			   filter->auth_type[i]) {
				neg_auth =
					WLAN_AUTH_TYPE_RSN_8021X_SHA256;
				match = true;
				break;
			}
		}
	}

	if (!match)
		return false;
	match = scm_check_pmf_match(filter, &rsn);

	if (match && security) {
		security->auth_type = neg_auth;
		security->mc_enc = neg_mccipher;
	}

	return match;
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
	uint8_t cipher_type;
	bool match = false;
	enum wlan_auth_type neg_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	enum wlan_enc_type neg_mccipher = WLAN_ENCRYPT_TYPE_NONE;
	struct wlan_wpa_ie wpa = {0};

	if (!util_scan_entry_wpa(db_entry))
		return false;

	wlan_parse_wpa_ie(util_scan_entry_wpa(db_entry), &wpa);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(wpa.uc_ciphers,
		wpa.uc_cipher_count, WLAN_WPA_SEL(cipher_type));
	if (!match)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
		  scm_get_cipher_suite_type(
		  filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&wpa.mc_cipher,
			   1, WLAN_WPA_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match)
		return false;
	neg_mccipher = filter->mc_enc_type[i];

	/* Initializing with false as it has true value already */
	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		/*
		 * Ciphers are supported, Match authentication algorithm and
		 * pick first matching authtype.
		 */
		/**/
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_SEL(WLAN_AKM_IEEE8021X))) {
			if (WLAN_AUTH_TYPE_WPA ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_WPA;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_SEL(WLAN_AKM_PSK))) {
			if (WLAN_AUTH_TYPE_WPA_PSK ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_WPA_PSK;
				match = true;
				break;
			}
		}
		if (scm_is_cipher_match(wpa.auth_suites,
		   wpa.auth_suite_count,
		   WLAN_WPA_CCKM_AKM)) {
			if (WLAN_AUTH_TYPE_CCKM_WPA ==
			   filter->auth_type[i]) {
				neg_auth = WLAN_AUTH_TYPE_CCKM_WPA;
				match = true;
				break;
			}
		}
	}

	if (match && security) {
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

	if (!util_scan_entry_wapi(db_entry))
		return false;

	wlan_parse_wapi_ie(
		   util_scan_entry_wapi(db_entry), &wapi);

	cipher_type =
		scm_get_cipher_suite_type(security->uc_enc);
	match = scm_is_cipher_match(wapi.uc_cipher_suites,
		wapi.uc_cipher_count, WLAN_WAPI_SEL(cipher_type));
	if (!match)
		return false;

	for (i = 0; i < filter->num_of_mc_enc_type; i++) {
		cipher_type =
		  scm_get_cipher_suite_type(
		  filter->mc_enc_type[i]);
		match = scm_is_cipher_match(&wapi.mc_cipher_suite,
				  1, WLAN_WAPI_SEL(cipher_type));
		if (match)
			break;
	}
	if (!match)
		return false;
	neg_mccipher = filter->mc_enc_type[i];

	if (scm_is_cipher_match(wapi.akm_suites,
	   wapi.akm_suite_count,
	   WLAN_WAPI_SEL(WLAN_WAI_CERT_OR_SMS4)))
		neg_auth =
			WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	else if (scm_is_cipher_match(wapi.akm_suites,
	   wapi.akm_suite_count, WLAN_WAPI_SEL(WLAN_WAI_PSK)))
		neg_auth = WLAN_AUTH_TYPE_WAPI_WAI_PSK;
	else
		return false;

	match = false;
	for (i = 0; i < filter->num_of_auth; i++) {
		if (filter->auth_type[i] == neg_auth) {
			match = true;
			break;
		}
	}

	if (match && security) {
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
	bool match_any = false;
	bool match = true;

	/* It is allowed to match anything. Try the more secured ones first. */
	/* Check AES first */
	security->uc_enc = WLAN_ENCRYPT_TYPE_AES;
	match_any = scm_is_rsn_security(filter,
			    db_entry, security);
	if (!match_any) {
		/* Check TKIP */
		security->uc_enc = WLAN_ENCRYPT_TYPE_TKIP;
		match_any = scm_is_rsn_security(filter,
			    db_entry, security);
	}

	if (!match_any) {
		/* Check WAPI */
		security->uc_enc = WLAN_ENCRYPT_TYPE_WPI;
		match_any = scm_is_wapi_security(filter,
			    db_entry, security);
	}

	if (match_any)
		return match;

	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP104_STATICKEY;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;
	security->uc_enc = WLAN_ENCRYPT_TYPE_WEP40_STATICKEY;
	if (scm_is_wep_security(filter,
	   db_entry, security))
		return true;

	/* It must be open and no enc */
	if (db_entry->cap_info.wlan_caps.privacy)
		return false;

	security->auth_type = WLAN_AUTH_TYPE_OPEN_SYSTEM;
	security->mc_enc = WLAN_ENCRYPT_TYPE_NONE;
	security->uc_enc = WLAN_ENCRYPT_TYPE_NONE;

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
			/* First check if there is a RSN match */
			match = scm_is_rsn_security(filter,
				    db_entry, &local_security);
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
			match  = scm_is_def_security(filter,
				    db_entry, &local_security);
			break;
		}
	}

	if (match && security)
		qdf_mem_copy(security,
			&local_security, sizeof(*security));

	return match;
}

bool scm_filter_match(struct wlan_objmgr_psoc *psoc,
	struct scan_cache_entry *db_entry,
	struct scan_filter *filter,
	struct security_info *security)
{
	int i;
	bool match = false;
	struct roam_filter_params *roam_params;
	struct scan_default_params *def_param;

	def_param = wlan_scan_psoc_get_def_params(psoc);
	roam_params = &def_param->roam_params;

	if (filter->p2p_results && !db_entry->is_p2p)
		return false;

	for (i = 0; i < roam_params->num_bssid_avoid_list; i++)
		if (qdf_is_macaddr_equal(&roam_params->bssid_avoid_list[i],
		   &db_entry->bssid))
			return false;

	match = false;
	if (db_entry->ssid.length) {
		for (i = 0; i < filter->num_of_ssid; i++) {
			if (util_is_ssid_match(&filter->ssid_list[i],
			   &db_entry->ssid)) {
				match = true;
				break;
			}
		}
	}
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
		if (!filter->channel_list[i] || (
		   (filter->channel_list[i] ==
		   db_entry->channel.chan_idx))) {
			match = true;
			break;
		}
	}

	if (!match && filter->num_of_channels)
		return false;

	if (filter->rrm_measurement_filter)
		return true;

	/* TODO match phyMode */

	if (!filter->ignore_auth_enc_type &&
	   !scm_is_security_match(filter,
	   db_entry, security))
		return false;

	if (!util_is_bss_type_match(filter->bss_type,
	   db_entry->cap_info))
		return false;

	/* TODO match rate set */

	if (filter->only_wmm_ap &&
	   !db_entry->ie_list.wmeinfo &&
	   !db_entry->ie_list.wmeparam)
		return false;

	if (!util_country_code_match(filter->country,
	   db_entry->ie_list.country))
		return false;

	if (!util_mdie_match(filter->mobility_domain,
	   (struct rsn_mdie *)db_entry->ie_list.mdie))
		return false;

	return true;
}
