/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
#include <wlan_reg_services_api.h>

#define MAX_IE_LEN 1024

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
		[SCAN_REASON_DFS_VIOLATION] = "DFS_NOL_VIOLATION",
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

	if (scan_obj)
		return scan_obj->pdev_info[pdev_id].last_scan_time;
	else
		return 0;
}

enum wlan_band util_scan_scm_chan_to_band(uint32_t chan)
{
	if (WLAN_CHAN_IS_2GHZ(chan))
		return WLAN_BAND_2_4_GHZ;

	return WLAN_BAND_5_GHZ;
}

enum wlan_band util_scan_scm_freq_to_band(uint16_t freq)
{
	if (WLAN_REG_IS_24GHZ_CH_FREQ(freq))
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
	   entry2->bssid.bytes, QDF_MAC_ADDR_SIZE)) {
		/* Check for BSS */
		if (util_is_ssid_match(&entry1->ssid, &entry2->ssid) ||
		    util_scan_is_null_ssid(&entry1->ssid) ||
		    util_scan_is_null_ssid(&entry2->ssid))
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
	case WLAN_EXTN_ELEMID_MAX_CHAN_SWITCH_TIME:
		scan_params->ie_list.mcst  = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_SRP:
		scan_params->ie_list.srp   = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HECAP:
		scan_params->ie_list.hecap = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HEOP:
		scan_params->ie_list.heop  = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_ESP:
		scan_params->ie_list.esp = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_MUEDCA:
		scan_params->ie_list.muedca = (uint8_t *)ie;
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
	if (!scan_params->ie_list.vendor)
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
		if (!scan_params->ie_list.htcap) {
			if (ie->ie_len != (WLAN_VENDOR_HT_IE_OFFSET_LEN +
					   sizeof(struct htcap_cmn_ie)))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.htcap =
			 (uint8_t *)&(((struct wlan_vendor_ie_htcap *)ie)->ie);
		}
	} else if (is_ht_info((uint8_t *)ie)) {
		/* we only care if there isn't already an HT IE (ANA) */
		if (!scan_params->ie_list.htinfo) {
			if (ie->ie_len != WLAN_VENDOR_HT_IE_OFFSET_LEN +
					  sizeof(struct wlan_ie_htinfo_cmn))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.htinfo =
			  (uint8_t *)&(((struct wlan_vendor_ie_htinfo *)
			  ie)->hi_ie);
		}
	} else if (is_interop_vht((uint8_t *)ie) &&
	    !(scan_params->ie_list.vhtcap)) {
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
				 sizeof(struct ie_header))) {
			if (ie->ie_len < ((WLAN_VENDOR_VHTOP_IE_OFFSET +
					  sizeof(struct wlan_ie_vhtop)) -
					  sizeof(struct ie_header)))
				return QDF_STATUS_E_INVAL;
			vendor_ie = ((uint8_t *)(ie)) +
				    WLAN_VENDOR_VHTOP_IE_OFFSET;
			if (vendor_ie[1] != (sizeof(struct wlan_ie_vhtop) -
					     sizeof(struct ie_header)))
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.vhtop = (((uint8_t *)(ie)) +
						   WLAN_VENDOR_VHTOP_IE_OFFSET);
		}
	} else if (is_bwnss_oui((uint8_t *)ie)) {
		/*
		 * Bandwidth-NSS map has sub-type & version.
		 * hence copy data just after version byte
		 */
		scan_params->ie_list.bwnss_map = (((uint8_t *)ie) + 8);
	} else if (is_mbo_oce_oui((uint8_t *)ie)) {
		scan_params->ie_list.mbo_oce = (uint8_t *)ie;
	} else if (is_extender_oui((uint8_t *)ie)) {
		scan_params->ie_list.extender = (uint8_t *)ie;
	} else if (is_adaptive_11r_oui((uint8_t *)ie)) {
		if ((ie->ie_len < OUI_LENGTH) ||
		    (ie->ie_len > MAX_ADAPTIVE_11R_IE_LEN))
			return QDF_STATUS_E_INVAL;

		scan_params->ie_list.adaptive_11r = (uint8_t *)ie +
						sizeof(struct ie_header);
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
			scm_debug("Incomplete corrupted IE:%x",
				ie->ie_id);
			return QDF_STATUS_E_INVAL;
		}

		switch (ie->ie_id) {
		case WLAN_ELEMID_SSID:
			if (ie->ie_len > (sizeof(struct ie_ssid) -
					  sizeof(struct ie_header)))
				goto err;
			scan_params->ie_list.ssid = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_RATES:
			if (ie->ie_len > WLAN_SUPPORTED_RATES_IE_MAX_LEN)
				goto err;
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
				goto err;
			scan_params->ie_list.tim = (uint8_t *)ie;
			scan_params->dtim_period =
				((struct wlan_tim_ie *)ie)->tim_period;
			break;
		case WLAN_ELEMID_COUNTRY:
			if (ie->ie_len < WLAN_COUNTRY_IE_MIN_LEN)
				goto err;
			scan_params->ie_list.country = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_QBSS_LOAD:
			if (ie->ie_len != sizeof(struct qbss_load_ie) -
					  sizeof(struct ie_header)) {
				/*
				 * Expected QBSS IE length is 5Bytes; For some
				 * old cisco AP, QBSS IE length is 4Bytes, which
				 * doesn't match with latest spec, So ignore
				 * QBSS IE in such case.
				 */
				break;
			}
			scan_params->ie_list.qbssload = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_CHANSWITCHANN:
			if (ie->ie_len != WLAN_CSA_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.csa = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_IBSSDFS:
			if (ie->ie_len < WLAN_IBSSDFS_IE_MIN_LEN)
				goto err;
			scan_params->ie_list.ibssdfs = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_QUIET:
			if (ie->ie_len != WLAN_QUIET_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.quiet = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_ERP:
			if (ie->ie_len != (sizeof(struct erp_ie) -
					    sizeof(struct ie_header)))
				goto err;
			scan_params->erp = ((struct erp_ie *)ie)->value;
			break;
		case WLAN_ELEMID_HTCAP_ANA:
			if (ie->ie_len != sizeof(struct htcap_cmn_ie))
				goto err;
			scan_params->ie_list.htcap =
				(uint8_t *)&(((struct htcap_ie *)ie)->ie);
			break;
		case WLAN_ELEMID_RSN:
			if (ie->ie_len < WLAN_RSN_IE_MIN_LEN)
				goto err;
			scan_params->ie_list.rsn = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_XRATES:
			scan_params->ie_list.xrates = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_EXTCHANSWITCHANN:
			if (ie->ie_len != WLAN_XCSA_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.xcsa = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_SECCHANOFFSET:
			if (ie->ie_len != WLAN_SECCHANOFF_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.secchanoff = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_HTINFO_ANA:
			if (ie->ie_len != sizeof(struct wlan_ie_htinfo_cmn))
				goto err;
			scan_params->ie_list.htinfo =
			  (uint8_t *)&(((struct wlan_ie_htinfo *) ie)->hi_ie);
			scan_params->channel.chan_idx =
			  ((struct wlan_ie_htinfo_cmn *)
			  (scan_params->ie_list.htinfo))->hi_ctrlchannel;
			break;
		case WLAN_ELEMID_WAPI:
			if (ie->ie_len < WLAN_WAPI_IE_MIN_LEN)
				goto err;
			scan_params->ie_list.wapi = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_XCAPS:
			if (ie->ie_len > WLAN_EXTCAP_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.extcaps = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_VHTCAP:
			if (ie->ie_len != (sizeof(struct wlan_ie_vhtcaps) -
					   sizeof(struct ie_header)))
				goto err;
			scan_params->ie_list.vhtcap = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_VHTOP:
			if (ie->ie_len != (sizeof(struct wlan_ie_vhtop) -
					   sizeof(struct ie_header)))
				goto err;
			scan_params->ie_list.vhtop = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_OP_MODE_NOTIFY:
			if (ie->ie_len != WLAN_OPMODE_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.opmode = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_MOBILITY_DOMAIN:
			if (ie->ie_len != WLAN_MOBILITY_DOMAIN_IE_MAX_LEN)
				goto err;
			scan_params->ie_list.mdie = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_VENDOR:
			status = util_scan_parse_vendor_ie(scan_params,
							   ie);
			if (QDF_IS_STATUS_ERROR(status))
				goto err_status;
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
				goto err_status;
			}
			break;
		case WLAN_ELEMID_FILS_INDICATION:
			if (ie->ie_len < WLAN_FILS_INDICATION_IE_MIN_LEN)
				goto err;
			scan_params->ie_list.fils_indication = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_EXTN_ELEM:
			status = util_scan_parse_extn_ie(scan_params, ie);
			if (QDF_IS_STATUS_ERROR(status))
				goto err_status;
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

err:
	status = QDF_STATUS_E_INVAL;
err_status:
	scm_debug("failed to parse IE - id: %d, len: %d",
		  ie->ie_id, ie->ie_len);

	return status;
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

#ifdef WLAN_DFS_CHAN_HIDDEN_SSID
QDF_STATUS
util_scan_add_hidden_ssid(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t bcnbuf)
{
	struct wlan_frame_hdr *hdr;
	struct wlan_bcn_frame *bcn;
	struct wlan_scan_obj *scan_obj;
	struct wlan_ssid *conf_ssid;
	struct  ie_header *ie;
	uint32_t frame_len = qdf_nbuf_len(bcnbuf);
	uint16_t bcn_ie_offset, ssid_ie_start_offset, ssid_ie_end_offset;
	uint16_t tmplen, ie_length;
	uint8_t *pbeacon, *tmp;
	bool     set_ssid_flag = false;
	struct ie_ssid *ssid;
	uint8_t pdev_id;

	if (!pdev) {
		scm_warn("pdev: 0x%pK is NULL", pdev);
		return QDF_STATUS_E_NULL_VALUE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	scan_obj = wlan_pdev_get_scan_obj(pdev);
	if (!scan_obj) {
		scm_warn("null scan_obj");
		return QDF_STATUS_E_NULL_VALUE;
	}

	conf_ssid = &scan_obj->pdev_info[pdev_id].conf_ssid;

	hdr = (struct wlan_frame_hdr *)qdf_nbuf_data(bcnbuf);

	/* received bssid does not match configured bssid */
	if (qdf_mem_cmp(hdr->i_addr3, scan_obj->pdev_info[pdev_id].conf_bssid,
			QDF_MAC_ADDR_SIZE) ||
			conf_ssid->length == 0) {
		return QDF_STATUS_SUCCESS;
	}

	bcn = (struct wlan_bcn_frame *)(qdf_nbuf_data(bcnbuf) + sizeof(*hdr));
	pbeacon = (uint8_t *)bcn;

	ie = (struct ie_header *)(pbeacon +
				  offsetof(struct wlan_bcn_frame, ie));

	bcn_ie_offset = offsetof(struct wlan_bcn_frame, ie);
	ie_length = (uint16_t)(frame_len - sizeof(*hdr) -
			       bcn_ie_offset);

	while (ie_length >=  sizeof(struct ie_header)) {
		ie_length -= sizeof(struct ie_header);

		bcn_ie_offset += sizeof(struct ie_header);

		if (ie_length < ie->ie_len) {
			scm_debug("Incomplete corrupted IE:%x", ie->ie_id);
			return QDF_STATUS_E_INVAL;
		}
		if (ie->ie_id == WLAN_ELEMID_SSID) {
			if (ie->ie_len > (sizeof(struct ie_ssid) -
						 sizeof(struct ie_header))) {
				return QDF_STATUS_E_INVAL;
			}
			ssid = (struct ie_ssid *)ie;
			if (util_scan_is_hidden_ssid(ssid)) {
				set_ssid_flag  = true;
				ssid_ie_start_offset = bcn_ie_offset -
					sizeof(struct ie_header);
				ssid_ie_end_offset = bcn_ie_offset +
					ie->ie_len;
			}
		}
		if (ie->ie_len == 0) {
			ie += 1;    /* next IE */
			continue;
		}
		if (ie->ie_id == WLAN_ELEMID_VENDOR &&
		    is_wps_oui((uint8_t *)ie)) {
			set_ssid_flag = false;
			break;
		}
		/* Consume info element */
		ie_length -=  ie->ie_len;
		/* Go to next IE */
		ie = (struct ie_header *)(((uint8_t *)ie) +
				sizeof(struct ie_header) +
				ie->ie_len);
	}

	if (set_ssid_flag) {
		/* Hidden SSID if the Length is 0 */
		if (!ssid->ssid_len) {
			/* increase the taillength by length of ssid */
			if (qdf_nbuf_put_tail(bcnbuf,
					      conf_ssid->length) == NULL) {
				scm_debug("No enough tailroom");
				return  QDF_STATUS_E_NOMEM;
			}
			/* length of the buffer to be copied */
			tmplen = frame_len -
				sizeof(*hdr) - ssid_ie_end_offset;
			/*
			 * tmp memory to copy the beacon info
			 * after ssid ie.
			 */
			tmp = qdf_mem_malloc(tmplen * sizeof(u_int8_t));
			if (!tmp)
				return  QDF_STATUS_E_NOMEM;

			/* Copy beacon data after ssid ie to tmp */
			qdf_nbuf_copy_bits(bcnbuf, (sizeof(*hdr) +
					   ssid_ie_end_offset), tmplen, tmp);
			/* Add ssid length */
			*(pbeacon + (ssid_ie_start_offset + 1))
				= conf_ssid->length;
			/* Insert the  SSID string */
			qdf_mem_copy((pbeacon + ssid_ie_end_offset),
				     conf_ssid->ssid, conf_ssid->length);
			/* Copy rest of the beacon data */
			qdf_mem_copy((pbeacon + ssid_ie_end_offset +
				      conf_ssid->length), tmp, tmplen);
			qdf_mem_free(tmp);

			/* Hidden ssid with all 0's */
		} else if (ssid->ssid_len == conf_ssid->length) {
			/* Insert the  SSID string */
			qdf_mem_copy((pbeacon + ssid_ie_start_offset +
				      sizeof(struct ie_header)),
				      conf_ssid->ssid, conf_ssid->length);
		} else {
			scm_debug("mismatch in hidden ssid length");
			return QDF_STATUS_E_INVAL;
		}
	}
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_DFS_CHAN_HIDDEN_SSID */

#ifdef WLAN_ADAPTIVE_11R
/**
 * scm_fill_adaptive_11r_cap() - Check if the AP supports adaptive 11r
 * @scan_entry: Pointer to the scan entry
 *
 * Return: true if adaptive 11r is advertised else false
 */
static void scm_fill_adaptive_11r_cap(struct scan_cache_entry *scan_entry)
{
	uint8_t *ie;
	uint8_t data;
	bool adaptive_11r;

	ie = util_scan_entry_adaptive_11r(scan_entry);
	if (!ie)
		return;

	data = *(ie + OUI_LENGTH);
	adaptive_11r = (data & 0x1) ? true : false;

	scan_entry->adaptive_11r_ap = adaptive_11r;
}
#else
static void scm_fill_adaptive_11r_cap(struct scan_cache_entry *scan_entry)
{
	scan_entry->adaptive_11r_ap = false;
}
#endif

static QDF_STATUS
util_scan_gen_scan_entry(struct wlan_objmgr_pdev *pdev,
			 uint8_t *frame, qdf_size_t frame_len,
			 uint32_t frm_subtype,
			 struct mgmt_rx_event_params *rx_param,
			 struct scan_mbssid_info *mbssid_info,
			 qdf_list_t *scan_list)
{
	struct wlan_frame_hdr *hdr;
	struct wlan_bcn_frame *bcn;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct ie_ssid *ssid;
	struct scan_cache_entry *scan_entry;
	struct qbss_load_ie *qbss_load;
	struct scan_cache_node *scan_node;
	uint8_t i;

	scan_entry = qdf_mem_malloc_atomic(sizeof(*scan_entry));
	if (!scan_entry) {
		scm_err("failed to allocate memory for scan_entry");
		return QDF_STATUS_E_NOMEM;
	}
	scan_entry->raw_frame.ptr =
			qdf_mem_malloc_atomic(frame_len);
	if (!scan_entry->raw_frame.ptr) {
		scm_err("failed to allocate memory for frame");
		qdf_mem_free(scan_entry);
		return QDF_STATUS_E_NOMEM;
	}

	bcn = (struct wlan_bcn_frame *)
			   (frame + sizeof(*hdr));
	hdr = (struct wlan_frame_hdr *)frame;

	/* update timestamp in nanoseconds needed by kernel layers */
	scan_entry->boottime_ns = qdf_get_bootbased_boottime_ns();

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
	qdf_mem_copy(scan_entry->per_chain_rssi, rx_param->rssi_ctl,
		     WLAN_MGMT_TXRX_HOST_MAX_ANTENNA);

	if (!wlan_psoc_nif_fw_ext_cap_get(wlan_pdev_get_psoc(pdev),
					  WLAN_SOC_CEXT_HW_DB2DBM)) {
		for (i = 0; i < WLAN_MGMT_TXRX_HOST_MAX_ANTENNA; i++) {
			if (scan_entry->per_chain_rssi[i] !=
			    WLAN_INVALID_PER_CHAIN_SNR)
				scan_entry->per_chain_rssi[i] +=
						WLAN_NOISE_FLOOR_DBM_DEFAULT;
			else
				scan_entry->per_chain_rssi[i] =
						WLAN_INVALID_PER_CHAIN_RSSI;
		}
	}

	/* store jiffies */
	scan_entry->rrm_parent_tsf = (uint32_t)qdf_system_ticks();

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
		scm_debug("failed to parse beacon IE");
		qdf_mem_free(scan_entry->raw_frame.ptr);
		qdf_mem_free(scan_entry);
		return QDF_STATUS_E_FAILURE;
	}

	ssid = (struct ie_ssid *)
		scan_entry->ie_list.ssid;

	if (ssid && (ssid->ssid_len > WLAN_SSID_MAX_LEN)) {
		qdf_mem_free(scan_entry->raw_frame.ptr);
		qdf_mem_free(scan_entry);
		return QDF_STATUS_E_FAILURE;
	}

	if (scan_entry->ie_list.p2p)
		scan_entry->is_p2p = true;

	/* If no channel info is present in beacon use meta channel */
	if (!scan_entry->channel.chan_idx) {
		scan_entry->channel.chan_idx =
				rx_param->channel;
	} else if (rx_param->channel !=
	   scan_entry->channel.chan_idx) {
		if (!wlan_reg_chan_is_49ghz(pdev, scan_entry->channel.chan_idx))
			scan_entry->channel_mismatch = true;
	}

	if (util_scan_is_hidden_ssid(ssid)) {
		scan_entry->ie_list.ssid = NULL;
		scan_entry->is_hidden_ssid = true;
	} else {
		qdf_mem_copy(scan_entry->ssid.ssid,
				ssid->ssid, ssid->ssid_len);
		scan_entry->ssid.length = ssid->ssid_len;
		scan_entry->hidden_ssid_timestamp =
			scan_entry->scan_entry_time;
	}
	qdf_mem_copy(&scan_entry->mbssid_info, mbssid_info,
		     sizeof(scan_entry->mbssid_info));
	if (WLAN_CHAN_IS_5GHZ(scan_entry->channel.chan_idx))
		scan_entry->phy_mode = util_scan_get_phymode_5g(scan_entry);
	else
		scan_entry->phy_mode = util_scan_get_phymode_2g(scan_entry);

	scan_entry->nss = util_scan_scm_calc_nss_supported_by_ap(scan_entry);
	scm_fill_adaptive_11r_cap(scan_entry);

	util_scan_scm_update_bss_with_esp_data(scan_entry);
	qbss_load = (struct qbss_load_ie *)
			util_scan_entry_qbssload(scan_entry);
	if (qbss_load)
		scan_entry->qbss_chan_load = qbss_load->qbss_chan_load;

	scan_node = qdf_mem_malloc_atomic(sizeof(*scan_node));
	if (!scan_node) {
		qdf_mem_free(scan_entry->raw_frame.ptr);
		qdf_mem_free(scan_entry);
		return QDF_STATUS_E_FAILURE;
	}

	scan_node->entry = scan_entry;
	qdf_list_insert_front(scan_list, &scan_node->node);

	return status;
}

/**
 * util_scan_find_ie() - find information element
 * @eid: element id
 * @ies: pointer consisting of IEs
 * @len: IE length
 *
 * Return: NULL if the element ID is not found or
 * a pointer to the first byte of the requested
 * element
 */
static uint8_t *util_scan_find_ie(uint8_t eid, uint8_t *ies,
				  int32_t len)
{
	while (len >= 2 && len >= ies[1] + 2) {
		if (ies[0] == eid)
			return ies;
		len -= ies[1] + 2;
		ies += ies[1] + 2;
	}

	return NULL;
}

#ifdef WLAN_FEATURE_MBSSID
static void util_gen_new_bssid(uint8_t *bssid, uint8_t max_bssid,
			       uint8_t mbssid_index,
			       uint8_t *new_bssid_addr)
{
	uint64_t bssid_tmp = 0, new_bssid = 0;
	uint64_t lsb_n;
	int i;

	for (i = 0; i < QDF_MAC_ADDR_SIZE; i++)
		bssid_tmp = bssid_tmp << 8 | bssid[i];

	lsb_n = bssid_tmp & ((1 << max_bssid) - 1);
	new_bssid = bssid_tmp;
	new_bssid &= ~((1 << max_bssid) - 1);
	new_bssid |= (lsb_n + mbssid_index) % (1 << max_bssid);

	for (i = QDF_MAC_ADDR_SIZE - 1; i >= 0; i--) {
		new_bssid_addr[i] = new_bssid & 0xff;
		new_bssid = new_bssid >> 8;
	}
}

static uint32_t util_gen_new_ie(uint8_t *ie, uint32_t ielen,
				uint8_t *subelement,
				size_t subie_len, uint8_t *new_ie)
{
	uint8_t *pos, *tmp;
	const uint8_t *tmp_old, *tmp_new;
	uint8_t *sub_copy;

	/* copy subelement as we need to change its content to
	 * mark an ie after it is processed.
	 */
	sub_copy = qdf_mem_malloc(subie_len);
	if (!sub_copy)
		return 0;
	qdf_mem_copy(sub_copy, subelement, subie_len);

	pos = &new_ie[0];

	/* new ssid */
	tmp_new = util_scan_find_ie(WLAN_ELEMID_SSID, sub_copy, subie_len);
	if (tmp_new) {
		qdf_mem_copy(pos, tmp_new, tmp_new[1] + 2);
		pos += (tmp_new[1] + 2);
	}

	/* go through IEs in ie (skip SSID) and subelement,
	 * merge them into new_ie
	 */
	tmp_old = util_scan_find_ie(WLAN_ELEMID_SSID, ie, ielen);
	tmp_old = (tmp_old) ? tmp_old + tmp_old[1] + 2 : ie;

	while (tmp_old + tmp_old[1] + 2 - ie <= ielen) {
		if (tmp_old[0] == 0) {
			tmp_old++;
			continue;
		}

		tmp = (uint8_t *)util_scan_find_ie(tmp_old[0], sub_copy,
				subie_len);
		if (!tmp) {
			/* ie in old ie but not in subelement */
			if (tmp_old[0] != WLAN_ELEMID_MULTIPLE_BSSID) {
				qdf_mem_copy(pos, tmp_old, tmp_old[1] + 2);
				pos += tmp_old[1] + 2;
			}
		} else {
			/* ie in transmitting ie also in subelement,
			 * copy from subelement and flag the ie in subelement
			 * as copied (by setting eid field to 0xff). For
			 * vendor ie, compare OUI + type + subType to
			 * determine if they are the same ie.
			 */
			if (tmp_old[0] == WLAN_ELEMID_VENDOR) {
				if (!qdf_mem_cmp(tmp_old + 2, tmp + 2, 5)) {
					/* same vendor ie, copy from
					 * subelement
					 */
					qdf_mem_copy(pos, tmp, tmp[1] + 2);
					pos += tmp[1] + 2;
					tmp[0] = 0xff;
				} else {
					qdf_mem_copy(pos, tmp_old,
						     tmp_old[1] + 2);
					pos += tmp_old[1] + 2;
				}
			} else {
				/* copy ie from subelement into new ie */
				qdf_mem_copy(pos, tmp, tmp[1] + 2);
				pos += tmp[1] + 2;
				tmp[0] = 0xff;
			}
		}

		if (tmp_old + tmp_old[1] + 2 - ie == ielen)
			break;

		tmp_old += tmp_old[1] + 2;
	}

	/* go through subelement again to check if there is any ie not
	 * copied to new ie, skip ssid, capability, bssid-index ie
	 */
	tmp_new = sub_copy;
	while (tmp_new + tmp_new[1] + 2 - sub_copy <= subie_len) {
		if (!(tmp_new[0] == WLAN_ELEMID_NONTX_BSSID_CAP ||
		      tmp_new[0] == WLAN_ELEMID_SSID ||
		      tmp_new[0] == WLAN_ELEMID_MULTI_BSSID_IDX ||
		      tmp_new[0] == 0xff)) {
			qdf_mem_copy(pos, tmp_new, tmp_new[1] + 2);
			pos += tmp_new[1] + 2;
		}
		if (tmp_new + tmp_new[1] + 2 - sub_copy == subie_len)
			break;
		tmp_new += tmp_new[1] + 2;
	}

	qdf_mem_free(sub_copy);
	return pos - new_ie;
}

static QDF_STATUS util_scan_parse_mbssid(struct wlan_objmgr_pdev *pdev,
					 uint8_t *frame, qdf_size_t frame_len,
					 uint32_t frm_subtype,
					 struct mgmt_rx_event_params *rx_param,
					 qdf_list_t *scan_list)
{
	struct wlan_bcn_frame *bcn;
	struct wlan_frame_hdr *hdr;
	struct scan_mbssid_info mbssid_info;
	QDF_STATUS status;
	uint8_t *pos, *subelement, *mbssid_end_pos;
	uint8_t *tmp, *mbssid_index_ie;
	uint32_t subie_len, new_ie_len;
	uint8_t new_bssid[QDF_MAC_ADDR_SIZE], bssid[QDF_MAC_ADDR_SIZE];
	uint8_t *new_ie;
	uint8_t *ie, *new_frame = NULL;
	uint64_t ielen, new_frame_len;

	hdr = (struct wlan_frame_hdr *)frame;
	bcn = (struct wlan_bcn_frame *)(frame + sizeof(struct wlan_frame_hdr));
	ie = (uint8_t *)&bcn->ie;
	ielen = (uint16_t)(frame_len -
		sizeof(struct wlan_frame_hdr) -
		offsetof(struct wlan_bcn_frame, ie));
	qdf_mem_copy(bssid, hdr->i_addr3, QDF_MAC_ADDR_SIZE);

	if (!util_scan_find_ie(WLAN_ELEMID_MULTIPLE_BSSID, ie, ielen))
		return QDF_STATUS_E_FAILURE;

	pos = ie;

	new_ie = qdf_mem_malloc(MAX_IE_LEN);
	if (!new_ie)
		return QDF_STATUS_E_NOMEM;

	while (pos < ie + ielen + 2) {
		tmp = util_scan_find_ie(WLAN_ELEMID_MULTIPLE_BSSID, pos,
					ielen - (pos - ie));
		if (!tmp)
			break;

		mbssid_info.profile_count = 1 << tmp[2];
		mbssid_end_pos = tmp + tmp[1] + 2;
		/* Skip Element ID, Len, MaxBSSID Indicator */
		if (tmp[1] < 4)
			break;
		for (subelement = tmp + 3; subelement < mbssid_end_pos - 1;
		     subelement += 2 + subelement[1]) {
			subie_len = subelement[1];
			if (mbssid_end_pos - subelement < 2 + subie_len)
				break;
			if (subelement[0] != 0 || subelement[1] < 4) {
				/* not a valid BSS profile */
				continue;
			}

			if (subelement[2] != WLAN_ELEMID_NONTX_BSSID_CAP ||
			    subelement[3] != 2) {
				/* The first element within the Nontransmitted
				 * BSSID Profile is not the Nontransmitted
				 * BSSID Capability element.
				 */
				continue;
			}

			/* found a Nontransmitted BSSID Profile */
			mbssid_index_ie =
				util_scan_find_ie(WLAN_ELEMID_MULTI_BSSID_IDX,
						  subelement + 2, subie_len);
			if (!mbssid_index_ie || mbssid_index_ie[1] < 1 ||
			    mbssid_index_ie[2] == 0) {
				/* No valid Multiple BSSID-Index element */
				continue;
			}
			qdf_mem_copy(&mbssid_info.trans_bssid, bssid,
				     QDF_MAC_ADDR_SIZE);
			mbssid_info.profile_num = mbssid_index_ie[2];
			util_gen_new_bssid(bssid, tmp[2], mbssid_index_ie[2],
					   new_bssid);
			new_ie_len = util_gen_new_ie(ie, ielen, subelement + 2,
						     subie_len, new_ie);
			if (!new_ie_len)
				continue;

			new_frame_len = frame_len - ielen + new_ie_len;
			new_frame = qdf_mem_malloc(new_frame_len);
			if (!new_frame) {
				qdf_mem_free(new_ie);
				return QDF_STATUS_E_NOMEM;
			}

			/*
			 * Copy the header(24byte), timestamp(8 byte),
			 * beaconinterval(2byte) and capability(2byte)
			 */
			qdf_mem_copy(new_frame, frame, 36);
			/* Copy the new ie generated from MBSSID profile*/
			hdr = (struct wlan_frame_hdr *)new_frame;
			qdf_mem_copy(hdr->i_addr2, new_bssid,
				     QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(hdr->i_addr3, new_bssid,
				     QDF_MAC_ADDR_SIZE);
			/* Copy the new ie generated from MBSSID profile*/
			qdf_mem_copy(new_frame +
					offsetof(struct wlan_bcn_frame, ie) +
					sizeof(struct wlan_frame_hdr),
					new_ie, new_ie_len);
			status = util_scan_gen_scan_entry(pdev, new_frame,
							  new_frame_len,
							  frm_subtype,
							  rx_param,
							  &mbssid_info,
							  scan_list);
			if (QDF_IS_STATUS_ERROR(status)) {
				qdf_mem_free(new_frame);
				scm_err("failed to generate a scan entry");
				break;
			}
			/* scan entry makes its own copy so free the frame*/
			qdf_mem_free(new_frame);
		}

		pos = mbssid_end_pos;
	}
	qdf_mem_free(new_ie);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS util_scan_parse_mbssid(struct wlan_objmgr_pdev *pdev,
					 uint8_t *frame, qdf_size_t frame_len,
					 uint32_t frm_subtype,
					 struct mgmt_rx_event_params *rx_param,
					 qdf_list_t *scan_list)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS
util_scan_parse_beacon_frame(struct wlan_objmgr_pdev *pdev,
			     uint8_t *frame,
			     qdf_size_t frame_len,
			     uint32_t frm_subtype,
			     struct mgmt_rx_event_params *rx_param,
			     qdf_list_t *scan_list)
{
	struct wlan_bcn_frame *bcn;
	struct wlan_frame_hdr *hdr;
	uint8_t *mbssid_ie = NULL;
	uint32_t ie_len = 0;
	QDF_STATUS status;
	struct scan_mbssid_info mbssid_info = { 0 };

	hdr = (struct wlan_frame_hdr *)frame;
	bcn = (struct wlan_bcn_frame *)
			   (frame + sizeof(struct wlan_frame_hdr));
	ie_len = (uint16_t)(frame_len -
		sizeof(struct wlan_frame_hdr) -
		offsetof(struct wlan_bcn_frame, ie));

	mbssid_ie = util_scan_find_ie(WLAN_ELEMID_MULTIPLE_BSSID,
				      (uint8_t *)&bcn->ie, ie_len);
	if (mbssid_ie) {
		qdf_mem_copy(&mbssid_info.trans_bssid,
			     hdr->i_addr3, QDF_MAC_ADDR_SIZE);
		mbssid_info.profile_count = 1 << mbssid_ie[2];
	}

	status = util_scan_gen_scan_entry(pdev, frame, frame_len,
					  frm_subtype, rx_param,
					  &mbssid_info,
					  scan_list);

	/*
	 * IF MBSSID IE is present in the beacon then
	 * scan component will create a new entry for
	 * each BSSID found in the MBSSID
	 */
	if (mbssid_ie)
		status = util_scan_parse_mbssid(pdev, frame, frame_len,
						frm_subtype, rx_param,
						scan_list);

	if (QDF_IS_STATUS_ERROR(status))
		scm_debug_rl("Failed to create a scan entry");

	return status;
}

qdf_list_t *
util_scan_unpack_beacon_frame(struct wlan_objmgr_pdev *pdev, uint8_t *frame,
			      qdf_size_t frame_len, uint32_t frm_subtype,
			      struct mgmt_rx_event_params *rx_param)
{
	qdf_list_t *scan_list;
	QDF_STATUS status;

	scan_list = qdf_mem_malloc_atomic(sizeof(*scan_list));
	if (!scan_list) {
		scm_err("failed to allocate scan_list");
		return NULL;
	}
	qdf_list_create(scan_list, MAX_SCAN_CACHE_SIZE);

	status = util_scan_parse_beacon_frame(pdev, frame, frame_len,
					      frm_subtype, rx_param,
					      scan_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		ucfg_scan_purge_results(scan_list);
		return NULL;
	}

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

bool util_is_scan_completed(struct scan_event *event, bool *success)
{
	if ((event->type == SCAN_EVENT_TYPE_COMPLETED) ||
	    (event->type == SCAN_EVENT_TYPE_DEQUEUED) ||
	    (event->type == SCAN_EVENT_TYPE_START_FAILED)) {
		if ((event->type == SCAN_EVENT_TYPE_COMPLETED) &&
		    (event->reason == SCAN_REASON_COMPLETED))
			*success = true;
		else
			*success = false;

		return true;
	}

	*success = false;
	return false;
}

