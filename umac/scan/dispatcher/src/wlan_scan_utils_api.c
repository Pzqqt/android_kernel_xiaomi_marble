/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Defines scan utility functions
 */

#include <wlan_cmn.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_utils_api.h>
#include <../../core/src/wlan_scan_cache_db.h>
#include <../../core/src/wlan_scan_main.h>

const char*
util_scan_get_ev_type_name(enum scan_event_type type)
{
	static const char * const event_name[] = {
		[SCAN_EVENT_TYPE_STARTED] = "STARTED",
		[SCAN_EVENT_TYPE_COMPLETED] = "COMPLETED",
		[SCAN_EVENT_TYPE_BSS_CHANNEL] = "HOME_CHANNEL",
		[SCAN_EVENT_TYPE_FOREIGN_CHANNEL] = "FOREIGN_CHANNEL",
		[SCAN_EVENT_TYPE_DEQUEUED] = "DEQUEUED",
		[SCAN_EVENT_TYPE_PREEMPTED] = "PREEMPTED",
		[SCAN_EVENT_TYPE_START_FAILED] = "START_FAILED",
		[SCAN_EVENT_TYPE_RESTARTED] = "RESTARTED",
		[SCAN_EVENT_TYPE_FOREIGN_CHANNEL_EXIT] = "FOREIGN_CHANNEL_EXIT",
		[SCAN_EVENT_TYPE_SUSPENDED] = "SUSPENDED",
		[SCAN_EVENT_TYPE_RESUMED] = "RESUMED",
		[SCAN_EVENT_TYPE_NLO_COMPLETE] = "NLO_COMPLETE",
		[SCAN_EVENT_TYPE_NLO_MATCH] = "NLO_MATCH",
		[SCAN_EVENT_TYPE_INVALID] = "INVALID",
		[SCAN_EVENT_TYPE_GPIO_TIMEOUT] = "GPIO_TIMEOUT",
		[SCAN_EVENT_TYPE_RADIO_MEASUREMENT_START] =
			"RADIO_MEASUREMENT_START",
		[SCAN_EVENT_TYPE_RADIO_MEASUREMENT_END] =
			"RADIO_MEASUREMENT_END",
		[SCAN_EVENT_TYPE_BSSID_MATCH] = "BSSID_MATCH",
		[SCAN_EVENT_TYPE_FOREIGN_CHANNEL_GET_NF] =
			"FOREIGN_CHANNEL_GET_NF",
	};

	if (type >= SCAN_EVENT_TYPE_MAX)
		return "UNKNOWN";

	return event_name[type];
}


const char*
util_scan_get_ev_reason_name(enum scan_completion_reason reason)
{
	static const char * const reason_name[] = {
		[SCAN_REASON_NONE] = "NONE",
		[SCAN_REASON_COMPLETED] = "COMPLETED",
		[SCAN_REASON_CANCELLED] = "CANCELLED",
		[SCAN_REASON_PREEMPTED] = "PREEMPTED",
		[SCAN_REASON_TIMEDOUT] = "TIMEDOUT",
		[SCAN_REASON_INTERNAL_FAILURE] = "INTERNAL_FAILURE",
		[SCAN_REASON_SUSPENDED] = "SUSPENDED",
		[SCAN_REASON_RUN_FAILED] = "RUN_FAILED",
		[SCAN_REASON_TERMINATION_FUNCTION] = "TERMINATION_FUNCTION",
		[SCAN_REASON_MAX_OFFCHAN_RETRIES] = "MAX_OFFCHAN_RETRIES",
	};

	if (reason >= SCAN_REASON_MAX)
		return "UNKNOWN";

	return reason_name[reason];
}

qdf_time_t
util_get_last_scan_time(struct wlan_objmgr_vdev *vdev)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;

	if (!vdev) {
		scm_warn("null vdev");
		QDF_ASSERT(0);
		return 0;
	}
	pdev_id = wlan_scan_vdev_get_pdev_id(vdev);
	scan_obj = wlan_vdev_get_scan_obj(vdev);

	return scan_obj->pdev_info[pdev_id].last_scan_time;
}

enum wlan_band util_scan_scm_chan_to_band(uint32_t chan)
{
	if (WLAN_CHAN_IS_2GHZ(chan))
		return WLAN_BAND_2_4_GHZ;

	return WLAN_BAND_5_GHZ;
}

bool util_is_scan_entry_match(
	struct scan_cache_entry *entry1,
	struct scan_cache_entry *entry2)
{

	if (entry1->cap_info.wlan_caps.ess !=
	   entry2->cap_info.wlan_caps.ess)
		return false;

	if (entry1->cap_info.wlan_caps.ess &&
	   !qdf_mem_cmp(entry1->bssid.bytes,
	   entry2->bssid.bytes, QDF_MAC_ADDR_SIZE) &&
	   util_scan_scm_chan_to_band(
	   entry1->channel.chan_idx) ==
	   util_scan_scm_chan_to_band(entry2->channel.chan_idx)) {
		/* Check for BSS */
		if (util_is_ssid_match(
		   &entry1->ssid, &entry2->ssid))
			return true;
	} else if (entry1->cap_info.wlan_caps.ibss &&
	   (entry1->channel.chan_idx ==
	   entry2->channel.chan_idx)) {
		/*
		 * Same channel cannot have same SSID for
		 * different IBSS, so no need to check BSSID
		 */
		if (util_is_ssid_match(
		   &entry1->ssid, &entry2->ssid))
			return true;
	} else if (!entry1->cap_info.wlan_caps.ibss &&
	   !entry1->cap_info.wlan_caps.ess &&
	   !qdf_mem_cmp(entry1->bssid.bytes,
	   entry2->bssid.bytes, QDF_MAC_ADDR_SIZE)) {
		/* In case of P2P devices, ess and ibss will be set to zero */
		return true;
	}

	return false;
}

static bool util_is_pureg_rate(uint8_t *rates, uint8_t nrates)
{
	static const uint8_t g_rates[] = {12, 18, 24, 36, 48, 72, 96, 108};
	bool pureg = false;
	uint8_t i, j;

	for (i = 0; i < nrates; i++) {
		for (j = 0; j < QDF_ARRAY_SIZE(g_rates); j++) {
			if (WLAN_RV(rates[i]) == g_rates[j]) {
				pureg = true;
				break;
			}
		}
		if (pureg)
			break;
	}

	return pureg;
}
static enum wlan_phymode
util_scan_get_phymode_5g(struct scan_cache_entry *scan_params)
{
	enum wlan_phymode phymode = WLAN_PHYMODE_AUTO;
	uint16_t ht_cap = 0;
	struct htcap_cmn_ie *htcap;
	struct wlan_ie_htinfo_cmn *htinfo;
	struct wlan_ie_vhtop *vhtop;

	htcap = (struct htcap_cmn_ie *)
		util_scan_entry_htcap(scan_params);
	htinfo = (struct wlan_ie_htinfo_cmn *)
		util_scan_entry_htinfo(scan_params);
	vhtop = (struct wlan_ie_vhtop *)
		util_scan_entry_vhtop(scan_params);

	if (!(htcap && htinfo))
		return WLAN_PHYMODE_11A;

	if (htcap)
		ht_cap = le16toh(htcap->hc_cap);

	if (util_scan_entry_vhtcap(scan_params) && vhtop) {
		switch (vhtop->vht_op_chwidth) {
		case WLAN_VHTOP_CHWIDTH_2040:
			if ((ht_cap & WLAN_HTCAP_C_CHWIDTH40) &&
			   (htinfo->hi_extchoff ==
			   WLAN_HTINFO_EXTOFFSET_ABOVE))
				phymode = WLAN_PHYMODE_11AC_VHT40PLUS;
			else if ((ht_cap & WLAN_HTCAP_C_CHWIDTH40) &&
			   (htinfo->hi_extchoff ==
			   WLAN_HTINFO_EXTOFFSET_BELOW))
				phymode = WLAN_PHYMODE_11AC_VHT40MINUS;
			else
				phymode = WLAN_PHYMODE_11AC_VHT20;
			break;
		case WLAN_VHTOP_CHWIDTH_80:
			if (WLAN_IS_REVSIG_VHT80_80(vhtop))
				phymode = WLAN_PHYMODE_11AC_VHT80_80;
			else if (WLAN_IS_REVSIG_VHT160(vhtop))
				phymode = WLAN_PHYMODE_11AC_VHT160;
			else
				phymode = WLAN_PHYMODE_11AC_VHT80;
			break;
		case WLAN_VHTOP_CHWIDTH_160:
			phymode = WLAN_PHYMODE_11AC_VHT160;
			break;
		case WLAN_VHTOP_CHWIDTH_80_80:
			phymode = WLAN_PHYMODE_11AC_VHT80_80;
			break;
		default:
			scm_err("bad channel: %d",
					vhtop->vht_op_chwidth);
			break;
		}
	} else if ((ht_cap & WLAN_HTCAP_C_CHWIDTH40) &&
	   (htinfo->hi_extchoff == WLAN_HTINFO_EXTOFFSET_ABOVE))
		phymode = WLAN_PHYMODE_11NA_HT40PLUS;
	else if ((ht_cap & WLAN_HTCAP_C_CHWIDTH40) &&
	   (htinfo->hi_extchoff == WLAN_HTINFO_EXTOFFSET_BELOW))
		phymode = WLAN_PHYMODE_11NA_HT40MINUS;
	else
		phymode = WLAN_PHYMODE_11NA_HT20;

	return phymode;
}

static enum wlan_phymode
util_scan_get_phymode_2g(struct scan_cache_entry *scan_params)
{
	enum wlan_phymode phymode = WLAN_PHYMODE_AUTO;
	uint16_t ht_cap = 0;
	struct htcap_cmn_ie *htcap;
	struct wlan_ie_htinfo_cmn *htinfo;
	struct wlan_ie_vhtop *vhtop;

	htcap = (struct htcap_cmn_ie *)
		util_scan_entry_htcap(scan_params);
	htinfo = (struct wlan_ie_htinfo_cmn *)
		util_scan_entry_htinfo(scan_params);
	vhtop = (struct wlan_ie_vhtop *)
		util_scan_entry_vhtop(scan_params);

	if (htcap)
		ht_cap = le16toh(htcap->hc_cap);

	if (htcap && htinfo) {
		if ((ht_cap & WLAN_HTCAP_C_CHWIDTH40) &&
		   (htinfo->hi_extchoff == WLAN_HTINFO_EXTOFFSET_ABOVE))
			phymode = WLAN_PHYMODE_11NG_HT40PLUS;
		else if ((ht_cap & WLAN_HTCAP_C_CHWIDTH40) &&
		   (htinfo->hi_extchoff == WLAN_HTINFO_EXTOFFSET_BELOW))
			phymode = WLAN_PHYMODE_11NG_HT40MINUS;
		else
			phymode = WLAN_PHYMODE_11NG_HT20;
	} else if (util_scan_entry_xrates(scan_params)) {
		/* only 11G stations will have more than 8 rates */
		phymode = WLAN_PHYMODE_11G;
	} else {
		/* Some mischievous g-only APs do not set extended rates */
		if (util_scan_entry_rates(scan_params)) {
			if (util_is_pureg_rate(&scan_params->ie_list.rates[2],
			   scan_params->ie_list.rates[1]))
				phymode = WLAN_PHYMODE_11G;
			else
				phymode = WLAN_PHYMODE_11B;
		} else {
			phymode = WLAN_PHYMODE_11B;
		}
	}

	return phymode;
}

static QDF_STATUS
util_scan_parse_chan_switch_wrapper_ie(struct scan_cache_entry *scan_params,
	struct ie_header *sub_ie, qdf_size_t sub_ie_len)
{
	/* Walk through to check nothing is malformed */
	while (sub_ie_len >= sizeof(struct ie_header)) {
		/* At least one more header is present */
		sub_ie_len -= sizeof(struct ie_header);

		if (sub_ie->ie_len == 0) {
			sub_ie += 1;
			continue;
		}
		if (sub_ie_len < sub_ie->ie_len) {
			scm_err("Incomplete corrupted IE:%x",
				WLAN_ELEMID_CHAN_SWITCH_WRAP);
			return QDF_STATUS_E_INVAL;
		}
		switch (sub_ie->ie_id) {
		case WLAN_ELEMID_COUNTRY:
			scan_params->ie_list.country = (uint8_t *)sub_ie;
			break;
		case WLAN_ELEMID_WIDE_BAND_CHAN_SWITCH:
			scan_params->ie_list.widebw = (uint8_t *)sub_ie;
			break;
		case WLAN_ELEMID_VHT_TX_PWR_ENVLP:
			scan_params->ie_list.txpwrenvlp = (uint8_t *)sub_ie;
			break;
		}
		/* Consume sub info element */
		sub_ie_len -= sub_ie->ie_len;
		/* go to next Sub IE */
		sub_ie = (struct ie_header *)
			(((uint8_t *) sub_ie) +
			sizeof(struct ie_header) + sub_ie->ie_len);
	}

	return QDF_STATUS_SUCCESS;
}

bool
util_scan_is_hidden_ssid(struct ie_ssid *ssid)
{
	uint8_t i;

	/*
	 * We flag this as Hidden SSID if the Length is 0
	 * of the SSID only contains 0's
	 */
	if (!ssid || !ssid->ssid_len)
		return true;

	for (i = 0; i < ssid->ssid_len; i++)
		if (ssid->ssid[i] != 0)
			return false;

	/* All 0's */
	return true;
}

static QDF_STATUS
util_scan_parse_extn_ie(struct scan_cache_entry *scan_params,
	struct ie_header *ie)
{
	struct extn_ie_header *extn_ie = (struct extn_ie_header *) ie;

	switch (extn_ie->ie_extn_id) {
	case WLAN_EXTN_ELEMID_SRP:
		scan_params->ie_list.srp   = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HECAP:
		if ((extn_ie->ie_len < WLAN_HE_CAP_IE_MIN_LEN) ||
		    (extn_ie->ie_len > WLAN_HE_CAP_IE_MAX_LEN))
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.hecap = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HEOP:
		if ((extn_ie->ie_len < WLAN_HE_OP_IE_MIN_LEN) ||
		    (extn_ie->ie_len > WLAN_HE_OP_IE_MAX_LEN))
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.heop  = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_ESP:
		if (extn_ie->ie_len > WLAN_EXT_ESP_IE_MAX_LEN)
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.esp = (uint8_t *)ie;
		break;
	default:
		break;
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
util_scan_parse_vendor_ie(struct scan_cache_entry *scan_params,
	struct ie_header *ie)
{
	if (scan_params->ie_list.vendor == NULL)
		scan_params->ie_list.vendor = (uint8_t *)ie;

	if (is_wpa_oui((uint8_t *)ie)) {
		scan_params->ie_list.wpa = (uint8_t *)ie;
	} else if (is_wps_oui((uint8_t *)ie)) {
		scan_params->ie_list.wps = (uint8_t *)ie;
		/* WCN IE should be a subset of WPS IE */
		if (is_wcn_oui((uint8_t *)ie))
			scan_params->ie_list.wcn = (uint8_t *)ie;
	} else if (is_wme_param((uint8_t *)ie)) {
		scan_params->ie_list.wmeparam = (uint8_t *)ie;
	} else if (is_wme_info((uint8_t *)ie)) {
		scan_params->ie_list.wmeinfo = (uint8_t *)ie;
	} else if (is_atheros_oui((uint8_t *)ie)) {
		scan_params->ie_list.athcaps = (uint8_t *)ie;
	} else if (is_atheros_extcap_oui((uint8_t *)ie)) {
		scan_params->ie_list.athextcaps = (uint8_t *)ie;
	} else if (is_sfa_oui((uint8_t *)ie)) {
		scan_params->ie_list.sfa = (uint8_t *)ie;
	} else if (is_p2p_oui((uint8_t *)ie)) {
		scan_params->ie_list.p2p = (uint8_t *)ie;
	} else if (is_qca_son_oui((uint8_t *)ie,
				  QCA_OUI_WHC_AP_INFO_SUBTYPE)) {
		scan_params->ie_list.sonadv = (uint8_t *)ie;
	} else if (is_ht_cap((uint8_t *)ie)) {
		/* we only care if there isn't already an HT IE (ANA) */
		if (scan_params->ie_list.htcap == NULL) {
			if (ie->ie_len != (WLAN_VENDOR_HT_IE_OFFSET_LEN +
					   sizeof(struct htcap_cmn_ie)))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.htcap =
			 (uint8_t *)&(((struct wlan_vendor_ie_htcap *)ie)->ie);
		}
	} else if (is_ht_info((uint8_t *)ie)) {
		/* we only care if there isn't already an HT IE (ANA) */
		if (scan_params->ie_list.htinfo == NULL) {
			if (ie->ie_len != WLAN_VENDOR_HT_IE_OFFSET_LEN +
					  sizeof(struct wlan_ie_htinfo_cmn))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.htinfo =
			  (uint8_t *)&(((struct wlan_vendor_ie_htinfo *)
			  ie)->hi_ie);
		}
	} else if (is_interop_vht((uint8_t *)ie) &&
	    !(scan_params->ie_list.vhtop)) {
		uint8_t *vendor_ie = (uint8_t *)(ie);

		if (ie->ie_len < ((WLAN_VENDOR_VHTCAP_IE_OFFSET +
				 sizeof(struct wlan_ie_vhtcaps)) -
				 sizeof(struct ie_header)))
			return QDF_STATUS_E_INVAL;
		vendor_ie = ((uint8_t *)(ie)) + WLAN_VENDOR_VHTCAP_IE_OFFSET;
		if (vendor_ie[1] != (sizeof(struct wlan_ie_vhtcaps)) -
				      sizeof(struct ie_header))
			return QDF_STATUS_E_INVAL;
		/* location where Interop Vht Cap IE and VHT OP IE Present */
		scan_params->ie_list.vhtcap = (((uint8_t *)(ie)) +
						WLAN_VENDOR_VHTCAP_IE_OFFSET);
		if (ie->ie_len > ((WLAN_VENDOR_VHTCAP_IE_OFFSET +
				 sizeof(struct wlan_ie_vhtcaps)) -
				 sizeof(struct ie_header)) &&
		    ie->ie_len < ((WLAN_VENDOR_VHTOP_IE_OFFSET +
				  sizeof(struct wlan_ie_vhtop)) -
				  sizeof(struct ie_header)))
			return QDF_STATUS_E_INVAL;
		vendor_ie = ((uint8_t *)(ie)) + WLAN_VENDOR_VHTOP_IE_OFFSET;
		if (vendor_ie[1] != (sizeof(struct wlan_ie_vhtop) -
				     sizeof(struct ie_header)))
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.vhtop = (((uint8_t *)(ie)) +
						WLAN_VENDOR_VHTOP_IE_OFFSET);
	} else if (is_bwnss_oui((uint8_t *)ie)) {
		/*
		 * Bandwidth-NSS map has sub-type & version.
		 * hence copy data just after version byte
		 */
		scan_params->ie_list.bwnss_map = (((uint8_t *)ie) + 8);
	} else if (is_mbo_oce_oui((uint8_t *)ie)) {
		scan_params->ie_list.mbo_oce = (uint8_t *)ie;
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
util_scan_populate_bcn_ie_list(struct scan_cache_entry *scan_params)
{
	struct ie_header *ie, *sub_ie;
	uint32_t ie_len, sub_ie_len;
	QDF_STATUS status;

	ie_len = util_scan_entry_ie_len(scan_params);
	ie = (struct ie_header *)
		  util_scan_entry_ie_data(scan_params);

	while (ie_len >= sizeof(struct ie_header)) {
		ie_len -= sizeof(struct ie_header);

		if (!ie->ie_len) {
			ie += 1;
			continue;
		}

		if (ie_len < ie->ie_len) {
			scm_err("Incomplete corrupted IE:%x",
				ie->ie_id);
			return QDF_STATUS_E_INVAL;
		}

		switch (ie->ie_id) {
		case WLAN_ELEMID_SSID:
			if (ie->ie_len > (sizeof(struct ie_ssid) -
					  sizeof(struct ie_header)))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.ssid = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_RATES:
			if (ie->ie_len > WLAN_SUPPORTED_RATES_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.rates = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_DSPARMS:
			if (ie->ie_len != WLAN_DS_PARAM_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.ds_param = (uint8_t *)ie;
			scan_params->channel.chan_idx =
				((struct ds_ie *)ie)->cur_chan;
			break;
		case WLAN_ELEMID_TIM:
			if (ie->ie_len < WLAN_TIM_IE_MIN_LENGTH)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.tim = (uint8_t *)ie;
			scan_params->dtim_period =
				((struct wlan_tim_ie *)ie)->tim_period;
			break;
		case WLAN_ELEMID_COUNTRY:
			if (ie->ie_len < WLAN_COUNTRY_IE_MIN_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.country = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_QBSS_LOAD:
			if (ie->ie_len != sizeof(struct qbss_load_ie) -
					  sizeof(struct ie_header))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.qbssload = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_CHANSWITCHANN:
			if (ie->ie_len != WLAN_CSA_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.csa = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_IBSSDFS:
			if (ie->ie_len < WLAN_IBSSDFS_IE_MIN_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.ibssdfs = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_QUIET:
			if (ie->ie_len != WLAN_QUIET_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.quiet = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_ERP:
			if (ie->ie_len != (sizeof(struct erp_ie) -
					    sizeof(struct ie_header)))
				return QDF_STATUS_E_INVAL;
			scan_params->erp = ((struct erp_ie *)ie)->value;
			break;
		case WLAN_ELEMID_HTCAP_ANA:
			if (ie->ie_len != sizeof(struct htcap_cmn_ie))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.htcap =
				(uint8_t *)&(((struct htcap_ie *)ie)->ie);
			break;
		case WLAN_ELEMID_RSN:
			if (ie->ie_len < WLAN_RSN_IE_MIN_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.rsn = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_XRATES:
			scan_params->ie_list.xrates = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_EXTCHANSWITCHANN:
			if (ie->ie_len != WLAN_XCSA_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.xcsa = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_SECCHANOFFSET:
			if (ie->ie_len != WLAN_SECCHANOFF_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.secchanoff = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_HTINFO_ANA:
			if (ie->ie_len != sizeof(struct wlan_ie_htinfo_cmn))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.htinfo =
			  (uint8_t *)&(((struct wlan_ie_htinfo *) ie)->hi_ie);
			scan_params->channel.chan_idx =
			  ((struct wlan_ie_htinfo_cmn *)
			  (scan_params->ie_list.htinfo))->hi_ctrlchannel;
			break;
		case WLAN_ELEMID_WAPI:
			if (ie->ie_len < WLAN_WAPI_IE_MIN_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.wapi = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_XCAPS:
			if (ie->ie_len > WLAN_EXTCAP_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.extcaps = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_VHTCAP:
			if (ie->ie_len != (sizeof(struct wlan_ie_vhtcaps) -
					   sizeof(struct ie_header)))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.vhtcap = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_VHTOP:
			if (ie->ie_len != (sizeof(struct wlan_ie_vhtop) -
					   sizeof(struct ie_header)))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.vhtop = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_OP_MODE_NOTIFY:
			if (ie->ie_len != WLAN_OPMODE_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
				scan_params->ie_list.opmode = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_MOBILITY_DOMAIN:
			if (ie->ie_len != WLAN_MOBILITY_DOMAIN_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.mdie = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_VENDOR:
			status = util_scan_parse_vendor_ie(scan_params,
							   ie);
			if (QDF_IS_STATUS_ERROR(status))
				return status;
			break;
		case WLAN_ELEMID_CHAN_SWITCH_WRAP:
			scan_params->ie_list.cswrp = (uint8_t *)ie;
			/* Go to next sub IE */
			sub_ie = (struct ie_header *)
			(((uint8_t *)ie) + sizeof(struct ie_header));
			sub_ie_len = ie->ie_len;
			status =
				util_scan_parse_chan_switch_wrapper_ie(
					scan_params, sub_ie, sub_ie_len);
			if (QDF_IS_STATUS_ERROR(status)) {
				scm_err("failed to parse chan_switch_wrapper_ie");
				return status;
			}
			break;
		case WLAN_ELEMID_FILS_INDICATION:
			if (ie->ie_len < WLAN_FILS_INDICATION_IE_MIN_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.fils_indication = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_EXTN_ELEM:
			status = util_scan_parse_extn_ie(scan_params, ie);
			if (QDF_IS_STATUS_ERROR(status))
				return status;
			break;
		default:
			break;
		}

		/* Consume info element */
		ie_len -= ie->ie_len;
		/* Go to next IE */
		ie = (struct ie_header *)
			(((uint8_t *) ie) +
			sizeof(struct ie_header) +
			ie->ie_len);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * util_scan_update_esp_data: update ESP params from beacon/probe response
 * @esp_information: pointer to wlan_esp_information
 * @scan_entry: new received entry
 *
 * The Estimated Service Parameters element is
 * used by a AP to provide information to another STA which
 * can then use the information as input to an algorithm to
 * generate an estimate of throughput between the two STAs.
 * The ESP Information List field contains from 1 to 4 ESP
 * Information fields(each field 24 bits), each corresponding
 * to an access category for which estimated service parameters
 * information is provided.
 *
 * Return: None
 */
static void util_scan_update_esp_data(struct wlan_esp_ie *esp_information,
		struct scan_cache_entry *scan_entry)
{

	uint8_t *data;
	int i = 0;
	uint64_t total_elements;
	struct wlan_esp_info *esp_info;
	struct wlan_esp_ie *esp_ie;

	esp_ie = (struct wlan_esp_ie *)
		util_scan_entry_esp_info(scan_entry);

	total_elements  = esp_ie->esp_len;
	data = (uint8_t *)esp_ie + 3;
	do_div(total_elements, ESP_INFORMATION_LIST_LENGTH);

	if (total_elements > MAX_ESP_INFORMATION_FIELD) {
		scm_err("No of Air time fractions are greater than supported");
		return;
	}

	for (i = 0; i < total_elements; i++) {
		esp_info = (struct wlan_esp_info *)data;
		if (esp_info->access_category == ESP_AC_BK) {
			qdf_mem_copy(&esp_information->esp_info_AC_BK,
					data, 3);
			data = data + ESP_INFORMATION_LIST_LENGTH;
			continue;
		}
		if (esp_info->access_category == ESP_AC_BE) {
			qdf_mem_copy(&esp_information->esp_info_AC_BE,
					data, 3);
			data = data + ESP_INFORMATION_LIST_LENGTH;
			continue;
		}
		if (esp_info->access_category == ESP_AC_VI) {
			qdf_mem_copy(&esp_information->esp_info_AC_VI,
					data, 3);
			data = data + ESP_INFORMATION_LIST_LENGTH;
			continue;
		}
		if (esp_info->access_category == ESP_AC_VO) {
			qdf_mem_copy(&esp_information->esp_info_AC_VO,
					data, 3);
			data = data + ESP_INFORMATION_LIST_LENGTH;
			break;
		}
	}
}

/**
 * util_scan_scm_update_bss_with_esp_dataa: calculate estimated air time
 * fraction
 * @scan_entry: new received entry
 *
 * This function process all Access category ESP params and provide
 * best effort air time fraction.
 * If best effort is not available, it will choose VI, VO and BK in sequence
 *
 */
static void util_scan_scm_update_bss_with_esp_data(
		struct scan_cache_entry *scan_entry)
{
	uint8_t air_time_fraction = 0;
	struct wlan_esp_ie esp_information;

	if (!scan_entry->ie_list.esp)
		return;

	util_scan_update_esp_data(&esp_information, scan_entry);

	/*
	 * If the ESP metric is transmitting multiple airtime fractions, then
	 * follow the sequence AC_BE, AC_VI, AC_VO, AC_BK and pick whichever is
	 * the first one available
	 */
	if (esp_information.esp_info_AC_BE.access_category
			== ESP_AC_BE)
		air_time_fraction =
			esp_information.esp_info_AC_BE.
			estimated_air_fraction;
	else if (esp_information.esp_info_AC_VI.access_category
			== ESP_AC_VI)
		air_time_fraction =
			esp_information.esp_info_AC_VI.
			estimated_air_fraction;
	else if (esp_information.esp_info_AC_VO.access_category
			== ESP_AC_VO)
		air_time_fraction =
			esp_information.esp_info_AC_VO.
			estimated_air_fraction;
	else if (esp_information.esp_info_AC_BK.access_category
			== ESP_AC_BK)
		air_time_fraction =
			esp_information.esp_info_AC_BK.
				estimated_air_fraction;
	scan_entry->air_time_fraction = air_time_fraction;
}

/**
 * util_scan_scm_calc_nss_supported_by_ap() - finds out nss from AP
 * @scan_entry: new received entry
 *
 * Return: number of nss advertised by AP
 */
static int util_scan_scm_calc_nss_supported_by_ap(
		struct scan_cache_entry *scan_params)
{
	struct htcap_cmn_ie *htcap;
	struct wlan_ie_vhtcaps *vhtcaps;
	uint8_t rx_mcs_map;

	htcap = (struct htcap_cmn_ie *)
		util_scan_entry_htcap(scan_params);
	vhtcaps = (struct wlan_ie_vhtcaps *)
		util_scan_entry_vhtcap(scan_params);
	if (vhtcaps) {
		rx_mcs_map = vhtcaps->rx_mcs_map;
		if ((rx_mcs_map & 0xC0) != 0xC0)
			return 4;

		if ((rx_mcs_map & 0x30) != 0x30)
			return 3;

		if ((rx_mcs_map & 0x0C) != 0x0C)
			return 2;
	} else if (htcap) {
		if (htcap->mcsset[3])
			return 4;

		if (htcap->mcsset[2])
			return 3;

		if (htcap->mcsset[1])
			return 2;

	}
	return 1;
}

qdf_list_t *
util_scan_unpack_beacon_frame(uint8_t *frame,
	qdf_size_t frame_len, uint32_t frm_subtype,
	struct mgmt_rx_event_params *rx_param)
{
	struct wlan_frame_hdr *hdr;
	struct wlan_bcn_frame *bcn;
	QDF_STATUS status;
	struct ie_ssid *ssid;
	struct scan_cache_entry *scan_entry;
	struct qbss_load_ie *qbss_load;
	qdf_list_t *scan_list;
	struct scan_cache_node *scan_node;

	scan_list = qdf_mem_malloc(sizeof(*scan_list));
	if (!scan_list) {
		scm_err("failed to allocate scan_list");
		return NULL;
	}
	qdf_list_create(scan_list, MAX_SCAN_CACHE_SIZE);

	scan_entry = qdf_mem_malloc(sizeof(*scan_entry));
	if (!scan_entry) {
		scm_err("failed to allocate memory for scan_entry");
		qdf_mem_free(scan_list);
		return NULL;
	}
	scan_entry->raw_frame.ptr =
			qdf_mem_malloc(frame_len);
	if (!scan_entry->raw_frame.ptr) {
		scm_err("failed to allocate memory for frame");
		qdf_mem_free(scan_entry);
		qdf_mem_free(scan_list);
		return NULL;
	}

	bcn = (struct wlan_bcn_frame *)
			   (frame + sizeof(*hdr));
	hdr = (struct wlan_frame_hdr *)frame;

	/* update timestamp in nanoseconds needed by kernel layers */
	scan_entry->boottime_ns = qdf_get_monotonic_boottime_ns();

	scan_entry->frm_subtype = frm_subtype;
	qdf_mem_copy(scan_entry->bssid.bytes,
		hdr->i_addr3, QDF_MAC_ADDR_SIZE);
	/* Scr addr */
	qdf_mem_copy(scan_entry->mac_addr.bytes,
		hdr->i_addr2, QDF_MAC_ADDR_SIZE);
	scan_entry->seq_num =
		(le16toh(*(uint16_t *)hdr->i_seq) >> WLAN_SEQ_SEQ_SHIFT);

	scan_entry->rssi_raw = rx_param->rssi;
	scan_entry->avg_rssi = WLAN_RSSI_IN(scan_entry->rssi_raw);
	scan_entry->tsf_delta = rx_param->tsf_delta;

	/* Copy per chain rssi to scan entry */
	qdf_mem_copy(scan_entry->per_chain_snr, rx_param->rssi_ctl,
		     WLAN_MGMT_TXRX_HOST_MAX_ANTENNA);

	/* store jiffies */
	scan_entry->rrm_parent_tsf = (u_int32_t) qdf_system_ticks();

	scan_entry->bcn_int = le16toh(bcn->beacon_interval);

	/*
	 * In case if the beacon dosnt have
	 * valid beacon interval falback to def
	 */
	if (!scan_entry->bcn_int)
		scan_entry->bcn_int = 100;
	scan_entry->cap_info.value = le16toh(bcn->capability.value);
	qdf_mem_copy(scan_entry->tsf_info.data,
		bcn->timestamp, 8);
	scan_entry->erp = ERP_NON_ERP_PRESENT;

	scan_entry->scan_entry_time =
		qdf_mc_timer_get_system_time();

	scan_entry->raw_frame.len = frame_len;
	qdf_mem_copy(scan_entry->raw_frame.ptr,
		frame, frame_len);
	status = util_scan_populate_bcn_ie_list(scan_entry);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("failed to parse beacon IE");
		qdf_mem_free(scan_entry->raw_frame.ptr);
		qdf_mem_free(scan_entry);
		qdf_mem_free(scan_list);
		return NULL;
	}

	if (!scan_entry->ie_list.rates) {
		qdf_mem_free(scan_entry->raw_frame.ptr);
		qdf_mem_free(scan_entry);
		qdf_mem_free(scan_list);
		return NULL;
	}

	ssid = (struct ie_ssid *)
		scan_entry->ie_list.ssid;

	if (ssid && (ssid->ssid_len > WLAN_SSID_MAX_LEN)) {
		qdf_mem_free(scan_entry->raw_frame.ptr);
		qdf_mem_free(scan_entry);
		qdf_mem_free(scan_list);
		return NULL;
	}

	if (scan_entry->ie_list.p2p)
		scan_entry->is_p2p = true;

	/* If no channel info is present in beacon use meta channel */
	if (!scan_entry->channel.chan_idx) {
		scan_entry->channel.chan_idx =
				rx_param->channel;
	} else if (rx_param->channel !=
	   scan_entry->channel.chan_idx) {
		scan_entry->channel_mismatch = true;
	}

	if (util_scan_is_hidden_ssid(ssid)) {
		scan_entry->ie_list.ssid = NULL;
	} else {
		qdf_mem_copy(scan_entry->ssid.ssid,
				ssid->ssid, WLAN_SSID_MAX_LEN);
		scan_entry->ssid.length = ssid->ssid_len;
		scan_entry->hidden_ssid_timestamp =
			scan_entry->scan_entry_time;
	}

	if (WLAN_CHAN_IS_5GHZ(scan_entry->channel.chan_idx))
		scan_entry->phy_mode = util_scan_get_phymode_5g(scan_entry);
	else
		scan_entry->phy_mode = util_scan_get_phymode_2g(scan_entry);

	scan_entry->nss = util_scan_scm_calc_nss_supported_by_ap(scan_entry);
	util_scan_scm_update_bss_with_esp_data(scan_entry);
	qbss_load = (struct qbss_load_ie *)
			util_scan_entry_qbssload(scan_entry);
	if (qbss_load)
		scan_entry->qbss_chan_load = qbss_load->qbss_chan_load;

	scan_node = qdf_mem_malloc(sizeof(*scan_node));
	if (!scan_node) {
		qdf_mem_free(scan_entry);
		qdf_mem_free(scan_list);
		return NULL;
	}

	scan_node->entry = scan_entry;
	qdf_list_insert_front(scan_list, &scan_node->node);

	/* TODO calculate channel struct */
	return scan_list;
}

QDF_STATUS
util_scan_entry_update_mlme_info(struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *scan_entry)
{

	if (!pdev || !scan_entry) {
		scm_err("pdev 0x%pK, scan_entry: 0x%pK", pdev, scan_entry);
		return QDF_STATUS_E_INVAL;
	}

	return scm_update_scan_mlme_info(pdev, scan_entry);
}
