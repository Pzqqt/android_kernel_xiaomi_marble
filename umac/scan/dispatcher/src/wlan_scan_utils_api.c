/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
#include <wlan_mlme_api.h>
#endif

#define MAX_IE_LEN 1024
#define SHORT_SSID_LEN 4
#define NEIGHBOR_AP_LEN 1
#define BSS_PARAMS_LEN 1

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
	   (entry1->channel.chan_freq ==
	   entry2->channel.chan_freq)) {
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

#ifdef WLAN_FEATURE_11BE
static enum wlan_phymode
util_scan_get_phymode_11be(struct wlan_objmgr_pdev *pdev,
			   struct scan_cache_entry *scan_params,
			   enum wlan_phymode phymode,
			   uint8_t band_mask)
{
	struct wlan_ie_ehtops *eht_ops;

	eht_ops = (struct wlan_ie_ehtops *)util_scan_entry_ehtop(scan_params);
	if (!util_scan_entry_ehtcap(scan_params) || !eht_ops)
		return phymode;

	switch (eht_ops->width) {
	case WLAN_EHT_CHWIDTH_20:
		phymode = WLAN_PHYMODE_11BEA_EHT20;
		break;
	case WLAN_EHT_CHWIDTH_40:
		phymode = WLAN_PHYMODE_11BEA_EHT40;
		break;
	case WLAN_EHT_CHWIDTH_80:
		phymode = WLAN_PHYMODE_11BEA_EHT80;
		break;
	case WLAN_EHT_CHWIDTH_160:
		phymode = WLAN_PHYMODE_11BEA_EHT160;
		break;
	case WLAN_EHT_CHWIDTH_320:
		phymode = WLAN_PHYMODE_11BEA_EHT320;
		break;
	default:
		scm_err("Invalid eht_ops width: %d", eht_ops->width);
		phymode = WLAN_PHYMODE_11BEA_EHT20;
		break;
	}

	scan_params->channel.cfreq0 =
		wlan_reg_chan_band_to_freq(pdev,
					   eht_ops->chan_freq_seg0,
					   band_mask);
	scan_params->channel.cfreq1 =
		wlan_reg_chan_band_to_freq(pdev,
					   eht_ops->chan_freq_seg1,
					   band_mask);
	scan_params->channel.puncture_bitmap = eht_ops->puncture_pattern;
	return phymode;
}
#else
static enum wlan_phymode
util_scan_get_phymode_11be(struct wlan_objmgr_pdev *pdev,
			   struct scan_cache_entry *scan_params,
			   enum wlan_phymode phymode,
			   uint8_t band_mask)
{
	return phymode;
}
#endif

#ifdef CONFIG_BAND_6GHZ
static struct he_oper_6g_param *util_scan_get_he_6g_params(uint8_t *he_ops)
{
	uint8_t len;
	uint32_t he_oper_params;

	if (!he_ops)
		return NULL;

	len = he_ops[1];
	he_ops += sizeof(struct ie_header);

	if (len < WLAN_HEOP_FIXED_PARAM_LENGTH)
		return NULL;

	/* element id extension */
	he_ops++;
	len--;

	he_oper_params = LE_READ_4(he_ops);
	if (!(he_oper_params & WLAN_HEOP_6GHZ_INFO_PRESENT_MASK))
		return NULL;

	/* fixed params - element id extension */
	he_ops += WLAN_HEOP_FIXED_PARAM_LENGTH - 1;
	len -= WLAN_HEOP_FIXED_PARAM_LENGTH - 1;

	if (!len)
		return NULL;

	/* vht oper params */
	if (he_oper_params & WLAN_HEOP_VHTOP_PRESENT_MASK) {
		if (len < WLAN_HEOP_VHTOP_LENGTH)
			return NULL;
		he_ops += WLAN_HEOP_VHTOP_LENGTH;
		len -= WLAN_HEOP_VHTOP_LENGTH;
	}

	if (!len)
		return NULL;

	if (he_oper_params & WLAN_HEOP_CO_LOCATED_BSS_MASK) {
		he_ops += WLAN_HEOP_CO_LOCATED_BSS_LENGTH;
		len -= WLAN_HEOP_CO_LOCATED_BSS_LENGTH;
	}

	if (len < sizeof(struct he_oper_6g_param))
		return NULL;

	return (struct he_oper_6g_param *)he_ops;
}

static QDF_STATUS
util_scan_get_chan_from_he_6g_params(struct wlan_objmgr_pdev *pdev,
				     struct scan_cache_entry *scan_params,
				     qdf_freq_t *chan_freq,
				     bool *he_6g_dup_bcon, uint8_t band_mask)
{
	struct he_oper_6g_param *he_6g_params;
	uint8_t *he_ops;
	struct wlan_scan_obj *scan_obj;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	*he_6g_dup_bcon = false;

	he_ops = util_scan_entry_heop(scan_params);
	if (!util_scan_entry_hecap(scan_params) || !he_ops)
		return QDF_STATUS_SUCCESS;

	he_6g_params = util_scan_get_he_6g_params(he_ops);
	if (!he_6g_params)
		return QDF_STATUS_SUCCESS;

	*chan_freq = wlan_reg_chan_band_to_freq(pdev,
						he_6g_params->primary_channel,
						band_mask);
	if (scan_obj->drop_bcn_on_invalid_freq &&
	    wlan_reg_is_disable_for_freq(pdev, *chan_freq)) {
		scm_debug_rl(QDF_MAC_ADDR_FMT": Drop as invalid channel %d freq %d in HE 6Ghz params",
			     QDF_MAC_ADDR_REF(scan_params->bssid.bytes),
			     he_6g_params->primary_channel, *chan_freq);
		return QDF_STATUS_E_INVAL;
	}
	*he_6g_dup_bcon = he_6g_params->duplicate_beacon ? true : false;

	return QDF_STATUS_SUCCESS;
}

static enum wlan_phymode
util_scan_get_phymode_6g(struct wlan_objmgr_pdev *pdev,
			 struct scan_cache_entry *scan_params)
{
	struct he_oper_6g_param *he_6g_params;
	enum wlan_phymode phymode = WLAN_PHYMODE_11AXA_HE20;
	uint8_t *he_ops;
	uint8_t band_mask = BIT(REG_BAND_6G);

	he_ops = util_scan_entry_heop(scan_params);
	if (!util_scan_entry_hecap(scan_params) || !he_ops)
		return phymode;

	he_6g_params = util_scan_get_he_6g_params(he_ops);
	if (!he_6g_params)
		return phymode;

	switch (he_6g_params->width) {
	case WLAN_HE_6GHZ_CHWIDTH_20:
		phymode = WLAN_PHYMODE_11AXA_HE20;
		break;
	case WLAN_HE_6GHZ_CHWIDTH_40:
		phymode = WLAN_PHYMODE_11AXA_HE40;
		break;
	case WLAN_HE_6GHZ_CHWIDTH_80:
		phymode = WLAN_PHYMODE_11AXA_HE80;
		break;
	case WLAN_HE_6GHZ_CHWIDTH_160_80_80:
		if (WLAN_IS_HE80_80(he_6g_params))
			phymode = WLAN_PHYMODE_11AXA_HE80_80;
		else if (WLAN_IS_HE160(he_6g_params))
			phymode = WLAN_PHYMODE_11AXA_HE160;
		else
			phymode = WLAN_PHYMODE_11AXA_HE80;
		break;
	default:
		scm_err("Invalid he_6g_params width: %d", he_6g_params->width);
		phymode = WLAN_PHYMODE_11AXA_HE20;
		break;
	}

	if (he_6g_params->chan_freq_seg0)
		scan_params->channel.cfreq0 =
			wlan_reg_chan_band_to_freq(pdev,
					he_6g_params->chan_freq_seg0,
					band_mask);
	if (he_6g_params->chan_freq_seg1)
		scan_params->channel.cfreq1 =
			wlan_reg_chan_band_to_freq(pdev,
					he_6g_params->chan_freq_seg1,
					band_mask);

	phymode = util_scan_get_phymode_11be(pdev, scan_params,
					     phymode, band_mask);

	return phymode;
}

uint8_t
util_scan_get_6g_oper_channel(uint8_t *he_op_ie)
{
	struct he_oper_6g_param *he_6g_params;

	he_6g_params = util_scan_get_he_6g_params(he_op_ie);
	if (!he_6g_params)
		return 0;

	return he_6g_params->primary_channel;
}

#else
static QDF_STATUS
util_scan_get_chan_from_he_6g_params(struct wlan_objmgr_pdev *pdev,
				     struct scan_cache_entry *scan_params,
				     qdf_freq_t *chan_freq,
				     bool *he_6g_dup_bcon,
				     uint8_t band_mask)
{
	return QDF_STATUS_SUCCESS;
}
static inline enum wlan_phymode
util_scan_get_phymode_6g(struct wlan_objmgr_pdev *pdev,
			 struct scan_cache_entry *scan_params)
{
	return WLAN_PHYMODE_AUTO;
}
#endif

static inline
uint32_t util_scan_sec_chan_freq_from_htinfo(struct wlan_ie_htinfo_cmn *htinfo,
					     uint32_t primary_chan_freq)
{
	if (htinfo->hi_extchoff == WLAN_HTINFO_EXTOFFSET_ABOVE)
		return primary_chan_freq + WLAN_CHAN_SPACING_20MHZ;
	else if (htinfo->hi_extchoff == WLAN_HTINFO_EXTOFFSET_BELOW)
		return primary_chan_freq - WLAN_CHAN_SPACING_20MHZ;

	return 0;
}

static enum wlan_phymode
util_scan_get_phymode_5g(struct wlan_objmgr_pdev *pdev,
			 struct scan_cache_entry *scan_params)
{
	enum wlan_phymode phymode = WLAN_PHYMODE_AUTO;
	uint16_t ht_cap = 0;
	struct htcap_cmn_ie *htcap;
	struct wlan_ie_htinfo_cmn *htinfo;
	struct wlan_ie_vhtop *vhtop;
	uint8_t band_mask = BIT(REG_BAND_5G);

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

	if (ht_cap & WLAN_HTCAP_C_CHWIDTH40)
		phymode = WLAN_PHYMODE_11NA_HT40;
	else
		phymode = WLAN_PHYMODE_11NA_HT20;

	scan_params->channel.cfreq0 =
		util_scan_sec_chan_freq_from_htinfo(htinfo,
						scan_params->channel.chan_freq);

	if (util_scan_entry_vhtcap(scan_params) && vhtop) {
		switch (vhtop->vht_op_chwidth) {
		case WLAN_VHTOP_CHWIDTH_2040:
			if (ht_cap & WLAN_HTCAP_C_CHWIDTH40)
				phymode = WLAN_PHYMODE_11AC_VHT40;
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
			phymode = WLAN_PHYMODE_11AC_VHT20;
			break;
		}
		if (vhtop->vht_op_ch_freq_seg1)
			scan_params->channel.cfreq0 =
				wlan_reg_chan_band_to_freq(pdev,
						vhtop->vht_op_ch_freq_seg1,
						band_mask);
		if (vhtop->vht_op_ch_freq_seg2)
			scan_params->channel.cfreq1 =
				wlan_reg_chan_band_to_freq(pdev,
						vhtop->vht_op_ch_freq_seg2,
						band_mask);
	}

	if (!util_scan_entry_hecap(scan_params))
		return phymode;

	/* for 5Ghz Check for HE, only if VHT cap and HE cap are present */
	if (!IS_WLAN_PHYMODE_VHT(phymode))
		return phymode;

	switch (phymode) {
	case WLAN_PHYMODE_11AC_VHT20:
		phymode = WLAN_PHYMODE_11AXA_HE20;
		break;
	case WLAN_PHYMODE_11AC_VHT40:
		phymode = WLAN_PHYMODE_11AXA_HE40;
		break;
	case WLAN_PHYMODE_11AC_VHT80:
		phymode = WLAN_PHYMODE_11AXA_HE80;
		break;
	case WLAN_PHYMODE_11AC_VHT160:
		phymode = WLAN_PHYMODE_11AXA_HE160;
		break;
	case WLAN_PHYMODE_11AC_VHT80_80:
		phymode = WLAN_PHYMODE_11AXA_HE80_80;
		break;
	default:
		phymode = WLAN_PHYMODE_11AXA_HE20;
		break;
	}

	phymode = util_scan_get_phymode_11be(pdev, scan_params,
					     phymode, band_mask);

	return phymode;
}

#ifdef WLAN_FEATURE_11BE
static enum wlan_phymode
util_scan_get_phymode_2g_11be(struct scan_cache_entry *scan_params,
			      enum wlan_phymode  phymode)
{
	if (!util_scan_entry_ehtcap(scan_params))
		return phymode;

	if (phymode == WLAN_PHYMODE_11AXG_HE40PLUS)
		phymode = WLAN_PHYMODE_11BEG_EHT40PLUS;
	else if (phymode == WLAN_PHYMODE_11AXG_HE40MINUS)
		phymode = WLAN_PHYMODE_11BEG_EHT40MINUS;
	else
		phymode = WLAN_PHYMODE_11BEG_EHT20;

	return phymode;
}
#else
static enum wlan_phymode
util_scan_get_phymode_2g_11be(struct scan_cache_entry *scan_params,
			      enum wlan_phymode  phymode)
{
	return phymode;
}
#endif

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

	/* Check for VHT only if HT cap is present */
	if (!IS_WLAN_PHYMODE_HT(phymode))
		return phymode;

	scan_params->channel.cfreq0 =
		util_scan_sec_chan_freq_from_htinfo(htinfo,
						scan_params->channel.chan_freq);

	if (util_scan_entry_vhtcap(scan_params) && vhtop) {
		switch (vhtop->vht_op_chwidth) {
		case WLAN_VHTOP_CHWIDTH_2040:
			if (phymode == WLAN_PHYMODE_11NG_HT40PLUS)
				phymode = WLAN_PHYMODE_11AC_VHT40PLUS_2G;
			else if (phymode == WLAN_PHYMODE_11NG_HT40MINUS)
				phymode = WLAN_PHYMODE_11AC_VHT40MINUS_2G;
			else
				phymode = WLAN_PHYMODE_11AC_VHT20_2G;

			break;
		default:
			scm_info("bad vht_op_chwidth: %d",
				 vhtop->vht_op_chwidth);
			phymode = WLAN_PHYMODE_11AC_VHT20_2G;
			break;
		}
	}

	if (!util_scan_entry_hecap(scan_params))
		return phymode;

	if (phymode == WLAN_PHYMODE_11AC_VHT40PLUS_2G ||
	    phymode == WLAN_PHYMODE_11NG_HT40PLUS)
		phymode = WLAN_PHYMODE_11AXG_HE40PLUS;
	else if (phymode == WLAN_PHYMODE_11AC_VHT40MINUS_2G ||
		 phymode == WLAN_PHYMODE_11NG_HT40MINUS)
		phymode = WLAN_PHYMODE_11AXG_HE40MINUS;
	else
		phymode = WLAN_PHYMODE_11AXG_HE20;

	phymode = util_scan_get_phymode_2g_11be(scan_params, phymode);

	return phymode;
}

static enum wlan_phymode
util_scan_get_phymode(struct wlan_objmgr_pdev *pdev,
		      struct scan_cache_entry *scan_params)
{
	if (WLAN_REG_IS_24GHZ_CH_FREQ(scan_params->channel.chan_freq))
		return util_scan_get_phymode_2g(scan_params);
	else if (WLAN_REG_IS_6GHZ_CHAN_FREQ(scan_params->channel.chan_freq))
		return util_scan_get_phymode_6g(pdev, scan_params);
	else
		return util_scan_get_phymode_5g(pdev, scan_params);
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
			scm_debug_rl(QDF_MAC_ADDR_FMT": Incomplete corrupted IE:%x",
				     QDF_MAC_ADDR_REF(scan_params->bssid.bytes),
				     WLAN_ELEMID_CHAN_SWITCH_WRAP);
			return QDF_STATUS_E_INVAL;
		}
		switch (sub_ie->ie_id) {
		case WLAN_ELEMID_COUNTRY:
			if (sub_ie->ie_len < WLAN_COUNTRY_IE_MIN_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.country = (uint8_t *)sub_ie;
			break;
		case WLAN_ELEMID_WIDE_BAND_CHAN_SWITCH:
			if (sub_ie->ie_len < WLAN_WIDE_BW_CHAN_SWITCH_IE_LEN)
				return QDF_STATUS_E_INVAL;
			scan_params->ie_list.widebw = (uint8_t *)sub_ie;
			break;
		case WLAN_ELEMID_VHT_TX_PWR_ENVLP:
			if (sub_ie->ie_len > WLAN_TPE_IE_MAX_LEN)
				return QDF_STATUS_E_INVAL;
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

#ifdef WLAN_FEATURE_11BE_MLO
static void
util_scan_update_rnr_mld(struct rnr_bss_info *rnr,
			 struct neighbor_ap_info_field *ap_info, uint8_t *data)
{
	bool mld_info_present = false;

	switch (ap_info->tbtt_header.tbtt_info_length) {
	case TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM_20MHZ_PSD_MLD_PARAM:
		qdf_mem_copy(&rnr->mld_info, &data[13],
			     sizeof(struct rnr_mld_info));
		mld_info_present = true;
		break;
	};
}
#else
static void
util_scan_update_rnr_mld(struct rnr_bss_info *rnr,
			 struct neighbor_ap_info_field *ap_info, uint8_t *data)
{
}
#endif

static QDF_STATUS
util_scan_update_rnr(struct rnr_bss_info *rnr,
		     struct neighbor_ap_info_field *ap_info,
		     uint8_t *data)
{
	uint8_t tbtt_info_length;

	tbtt_info_length = ap_info->tbtt_header.tbtt_info_length;

	switch (tbtt_info_length) {
	case TBTT_NEIGHBOR_AP_OFFSET_ONLY:
		/* Dont store it skip*/
		break;

	case TBTT_NEIGHBOR_AP_BSS_PARAM:
		/* Dont store it skip*/
		break;

	case TBTT_NEIGHBOR_AP_S_SSID_BSS_PARAM:
		rnr->bss_params = data[5];
		fallthrough;
	case TBTT_NEIGHBOR_AP_SHORTSSID:
		rnr->channel_number = ap_info->channel_number;
		rnr->operating_class = ap_info->operting_class;
		qdf_mem_copy(&rnr->short_ssid, &data[1], SHORT_SSID_LEN);
		break;

	case TBTT_NEIGHBOR_AP_BSSID_BSS_PARAM_20MHZ_PSD:
		rnr->psd_20mhz = data[8];
		fallthrough;
	case TBTT_NEIGHBOR_AP_BSSID_BSS_PARAM:
		rnr->bss_params = data[7];
		fallthrough;
	case TBTT_NEIGHBOR_AP_BSSID:
		rnr->channel_number = ap_info->channel_number;
		rnr->operating_class = ap_info->operting_class;
		qdf_mem_copy(&rnr->bssid, &data[1], QDF_MAC_ADDR_SIZE);
		break;

	case TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM_20MHZ_PSD_MLD_PARAM:
		util_scan_update_rnr_mld(rnr, ap_info, data);
		fallthrough;
	case TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM_20MHZ_PSD:
		rnr->psd_20mhz = data[12];
		fallthrough;
	case TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM:
		rnr->bss_params = data[11];
		fallthrough;
	case TBTT_NEIGHBOR_AP_BSSSID_S_SSID:
		rnr->channel_number = ap_info->channel_number;
		rnr->operating_class = ap_info->operting_class;
		qdf_mem_copy(&rnr->bssid, &data[1], QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(&rnr->short_ssid, &data[7], SHORT_SSID_LEN);
		break;

	default:
		scm_debug("Wrong fieldtype");
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
util_scan_parse_rnr_ie(struct scan_cache_entry *scan_entry,
		       struct ie_header *ie)
{
	uint32_t rnr_ie_len;
	uint16_t tbtt_count, tbtt_length, i, fieldtype;
	uint8_t *data;
	struct neighbor_ap_info_field *neighbor_ap_info;

	rnr_ie_len = ie->ie_len;
	data = (uint8_t *)ie + sizeof(struct ie_header);

	while ((data + sizeof(struct neighbor_ap_info_field)) <=
					((uint8_t *)ie + rnr_ie_len + 2)) {
		neighbor_ap_info = (struct neighbor_ap_info_field *)data;
		tbtt_count = neighbor_ap_info->tbtt_header.tbtt_info_count;
		tbtt_length = neighbor_ap_info->tbtt_header.tbtt_info_length;
		fieldtype = neighbor_ap_info->tbtt_header.tbbt_info_fieldtype;
		scm_debug("channel number %d, op class %d",
			  neighbor_ap_info->channel_number,
			  neighbor_ap_info->operting_class);
		scm_debug("tbtt_count %d, tbtt_length %d, fieldtype %d",
			  tbtt_count, tbtt_length, fieldtype);
		data += sizeof(struct neighbor_ap_info_field);

		if (tbtt_count > TBTT_INFO_COUNT)
			break;

		for (i = 0; i < (tbtt_count + 1) &&
		     (data + tbtt_length) <=
				((uint8_t *)ie + rnr_ie_len + 2); i++) {
			if (i < MAX_RNR_BSS)
				util_scan_update_rnr(
					&scan_entry->rnr.bss_info[i],
					neighbor_ap_info,
					data);
			data += tbtt_length;
		}
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
static void util_scan_parse_eht_ie(struct scan_cache_entry *scan_params,
				   struct extn_ie_header *extn_ie)
{
	switch (extn_ie->ie_extn_id) {
	case WLAN_EXTN_ELEMID_MULTI_LINK:
		scan_params->ie_list.multi_link = (uint8_t *)extn_ie;
		break;
	default:
		break;
	}
}
#else
static void util_scan_parse_eht_ie(struct scan_cache_entry *scan_params,
				   struct extn_ie_header *extn_ie)
{
}
#endif

static QDF_STATUS
util_scan_parse_extn_ie(struct scan_cache_entry *scan_params,
	struct ie_header *ie)
{
	struct extn_ie_header *extn_ie = (struct extn_ie_header *) ie;

	switch (extn_ie->ie_extn_id) {
	case WLAN_EXTN_ELEMID_MAX_CHAN_SWITCH_TIME:
		if (extn_ie->ie_len != WLAN_MAX_CHAN_SWITCH_TIME_IE_LEN)
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.mcst  = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_SRP:
		if (extn_ie->ie_len > WLAN_MAX_SRP_IE_LEN)
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.srp   = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HECAP:
		if ((extn_ie->ie_len < WLAN_MIN_HECAP_IE_LEN) ||
		    (extn_ie->ie_len > WLAN_MAX_HECAP_IE_LEN))
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.hecap = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HEOP:
		if (extn_ie->ie_len > WLAN_MAX_HEOP_IE_LEN)
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.heop  = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_ESP:
		scan_params->ie_list.esp = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_MUEDCA:
		if (extn_ie->ie_len > WLAN_MAX_MUEDCA_IE_LEN)
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.muedca = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_HE_6G_CAP:
		if (extn_ie->ie_len > WLAN_MAX_HE_6G_CAP_IE_LEN)
			return QDF_STATUS_E_INVAL;
		scan_params->ie_list.hecap_6g = (uint8_t *)ie;
		break;
#ifdef WLAN_FEATURE_11BE
	case WLAN_EXTN_ELEMID_EHTCAP:
		scan_params->ie_list.ehtcap = (uint8_t *)ie;
		break;
	case WLAN_EXTN_ELEMID_EHTOP:
		scan_params->ie_list.ehtop  = (uint8_t *)ie;
		break;
#endif
	default:
		break;
	}
	util_scan_parse_eht_ie(scan_params, extn_ie);

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
		if (ie->ie_len > WLAN_VENDOR_WME_IE_LEN)
			return QDF_STATUS_E_INVAL;

		scan_params->ie_list.wmeparam = (uint8_t *)ie;
	} else if (is_wme_info((uint8_t *)ie)) {
		scan_params->ie_list.wmeinfo = (uint8_t *)ie;
	} else if (is_atheros_oui((uint8_t *)ie)) {
		if (ie->ie_len > WLAN_VENDOR_ATHCAPS_IE_LEN)
			return QDF_STATUS_E_INVAL;

		scan_params->ie_list.athcaps = (uint8_t *)ie;
	} else if (is_atheros_extcap_oui((uint8_t *)ie)) {
		if (ie->ie_len > WLAN_VENDOR_ATH_EXTCAP_IE_LEN)
			return QDF_STATUS_E_INVAL;

		scan_params->ie_list.athextcaps = (uint8_t *)ie;
	} else if (is_sfa_oui((uint8_t *)ie)) {
		if (ie->ie_len > WLAN_VENDOR_SFA_IE_LEN)
			return QDF_STATUS_E_INVAL;

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
		if (ie->ie_len > WLAN_BWNSS_MAP_OFFSET)
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
	} else if (is_sae_single_pmk_oui((uint8_t *)ie)) {
		if ((ie->ie_len < OUI_LENGTH) ||
		    (ie->ie_len > MAX_SAE_SINGLE_PMK_IE_LEN)) {
			scm_debug("Invalid sae single pmk OUI");
			return QDF_STATUS_E_INVAL;
		}
		scan_params->ie_list.single_pmk = (uint8_t *)ie +
						sizeof(struct ie_header);
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
util_scan_populate_bcn_ie_list(struct wlan_objmgr_pdev *pdev,
			       struct scan_cache_entry *scan_params,
			       qdf_freq_t *chan_freq, uint8_t band_mask)
{
	struct ie_header *ie, *sub_ie;
	uint32_t ie_len, sub_ie_len;
	QDF_STATUS status;
	uint8_t chan_idx;
	struct wlan_scan_obj *scan_obj;
	struct wlan_objmgr_psoc *psoc;
	uint8_t tpe_idx = 0;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

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
			if (scan_obj->allow_bss_with_incomplete_ie) {
				scm_debug(QDF_MAC_ADDR_FMT": Scan allowed with incomplete corrupted IE:%x, ie_len: %d, ie->ie_len: %d, stop processing further",
					  QDF_MAC_ADDR_REF(scan_params->bssid.bytes),
					  ie->ie_id, ie_len, ie->ie_len);
				break;
			}
			scm_debug(QDF_MAC_ADDR_FMT": Scan not allowed with incomplete corrupted IE:%x, ie_len: %d, ie->ie_len: %d, stop processing further",
				  QDF_MAC_ADDR_REF(scan_params->bssid.bytes),
				  ie->ie_id, ie_len, ie->ie_len);
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
			chan_idx = ((struct ds_ie *)ie)->cur_chan;
			*chan_freq = wlan_reg_chan_band_to_freq(pdev, chan_idx,
								band_mask);
			/* Drop if invalid freq */
			if (scan_obj->drop_bcn_on_invalid_freq &&
			    !wlan_reg_is_freq_present_in_cur_chan_list(pdev,
								*chan_freq)) {
				scm_debug(QDF_MAC_ADDR_FMT": Drop as invalid chan %d in DS IE, freq %d, band_mask %d",
					  QDF_MAC_ADDR_REF(
						  scan_params->bssid.bytes),
					  chan_idx, *chan_freq, band_mask);
				return QDF_STATUS_E_INVAL;
			}
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
			if (ie->ie_len == sizeof(struct htcap_cmn_ie)) {
				scan_params->ie_list.htcap =
				(uint8_t *)&(((struct htcap_ie *)ie)->ie);
			}
			break;
		case WLAN_ELEMID_RSN:
			/*
			 * For security cert TC, RSNIE length can be 1 but if
			 * beacon is dropped, old entry will remain in scan
			 * cache and cause cert TC failure as connection with
			 * old entry with valid RSN IE will pass.
			 * So instead of dropping the frame, do not store the
			 * RSN pointer so that old entry is overwritten.
			 */
			if (ie->ie_len >= WLAN_RSN_IE_MIN_LEN)
				scan_params->ie_list.rsn = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_XRATES:
			if (ie->ie_len > WLAN_EXT_SUPPORTED_RATES_IE_MAX_LEN)
				goto err;
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
			chan_idx = ((struct wlan_ie_htinfo_cmn *)
				 (scan_params->ie_list.htinfo))->hi_ctrlchannel;
			*chan_freq = wlan_reg_chan_band_to_freq(pdev, chan_idx,
								band_mask);
			/* Drop if invalid freq */
			if (scan_obj->drop_bcn_on_invalid_freq &&
			    wlan_reg_is_disable_for_freq(pdev, *chan_freq)) {
				scm_debug_rl(QDF_MAC_ADDR_FMT": Drop as invalid channel %d freq %d in HT_INFO IE",
					     QDF_MAC_ADDR_REF(scan_params->bssid.bytes),
					     chan_idx, *chan_freq);
				return QDF_STATUS_E_INVAL;
			}
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
		case WLAN_ELEMID_VHT_TX_PWR_ENVLP:
			if (ie->ie_len < WLAN_TPE_IE_MIN_LEN)
				goto err;
			if (tpe_idx >= WLAN_MAX_NUM_TPE_IE)
				goto err;
			scan_params->ie_list.tpe[tpe_idx++] = (uint8_t *)ie;
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
		case WLAN_ELEMID_RSNXE:
			if (!ie->ie_len)
				goto err;
			scan_params->ie_list.rsnxe = (uint8_t *)ie;
			break;
		case WLAN_ELEMID_EXTN_ELEM:
			status = util_scan_parse_extn_ie(scan_params, ie);
			if (QDF_IS_STATUS_ERROR(status))
				goto err_status;
			break;
		case WLAN_ELEMID_REDUCED_NEIGHBOR_REPORT:
			if (ie->ie_len < WLAN_RNR_IE_MIN_LEN)
				goto err;
			scan_params->ie_list.rnrie = (uint8_t *)ie;
			status = util_scan_parse_rnr_ie(scan_params, ie);
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

	// Ignore ESP_ID_EXTN element
	total_elements  = esp_ie->esp_len - 1;
	data = (uint8_t *)esp_ie + 3;
	do_div(total_elements, ESP_INFORMATION_LIST_LENGTH);

	if (total_elements > MAX_ESP_INFORMATION_FIELD) {
		scm_err("No of Air time fractions are greater than supported");
		return;
	}

	for (i = 0; i < total_elements &&
	     data < ((uint8_t *)esp_ie + esp_ie->esp_len); i++) {
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
 * util_scan_scm_update_bss_with_esp_data: calculate estimated air time
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
 * @scan_params: new received entry
 *
 * Return: number of nss advertised by AP
 */
static int util_scan_scm_calc_nss_supported_by_ap(
		struct scan_cache_entry *scan_params)
{
	struct htcap_cmn_ie *htcap;
	struct wlan_ie_vhtcaps *vhtcaps;
	uint8_t *he_cap;
	uint8_t *end_ptr = NULL;
	uint16_t rx_mcs_map = 0;
	uint8_t *mcs_map_offset;

	htcap = (struct htcap_cmn_ie *)
		util_scan_entry_htcap(scan_params);
	vhtcaps = (struct wlan_ie_vhtcaps *)
		util_scan_entry_vhtcap(scan_params);
	he_cap = util_scan_entry_hecap(scan_params);

	if (he_cap) {
		/* Using rx mcs map related to 80MHz or lower as in some
		 * cases higher mcs may support lesser NSS than that
		 * of lowe mcs. Thus giving max NSS capability.
		 */
		end_ptr = he_cap + he_cap[1] + sizeof(struct ie_header);
		mcs_map_offset = (he_cap + sizeof(struct extn_ie_header) +
				  WLAN_HE_MACCAP_LEN + WLAN_HE_PHYCAP_LEN);
		if ((mcs_map_offset + WLAN_HE_MCS_MAP_LEN) <= end_ptr) {
			rx_mcs_map = *(uint16_t *)mcs_map_offset;
		} else {
			rx_mcs_map = WLAN_INVALID_RX_MCS_MAP;
			scm_debug("mcs_map_offset exceeds he cap len");
		}
	} else if (vhtcaps) {
		rx_mcs_map = vhtcaps->rx_mcs_map;
	}

	if (he_cap || vhtcaps) {
		if ((rx_mcs_map & 0xC000) != 0xC000)
			return 8;

		if ((rx_mcs_map & 0x3000) != 0x3000)
			return 7;

		if ((rx_mcs_map & 0x0C00) != 0x0C00)
			return 6;

		if ((rx_mcs_map & 0x0300) != 0x0300)
			return 5;

		if ((rx_mcs_map & 0x00C0) != 0x00C0)
			return 4;

		if ((rx_mcs_map & 0x0030) != 0x0030)
			return 3;

		if ((rx_mcs_map & 0x000C) != 0x000C)
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
	struct ie_ssid ssid = {0};
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
			ssid.ssid_id = ie->ie_id;
			ssid.ssid_len = ie->ie_len;

			if (ssid.ssid_len)
				qdf_mem_copy(ssid.ssid,
					     ie + sizeof(struct ie_header),
					     ssid.ssid_len);

			if (util_scan_is_hidden_ssid(&ssid)) {
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
		if (!ssid.ssid_len) {
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
		} else if (ssid.ssid_len == conf_ssid->length) {
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

static void util_scan_set_security(struct scan_cache_entry *scan_params)
{
	if (util_scan_entry_wpa(scan_params))
		scan_params->security_type |= SCAN_SECURITY_TYPE_WPA;

	if (util_scan_entry_rsn(scan_params))
		scan_params->security_type |= SCAN_SECURITY_TYPE_RSN;
	if (util_scan_entry_wapi(scan_params))
		scan_params->security_type |= SCAN_SECURITY_TYPE_WAPI;

	if (!scan_params->security_type &&
	    scan_params->cap_info.wlan_caps.privacy)
		scan_params->security_type |= SCAN_SECURITY_TYPE_WEP;
}

#ifdef WLAN_FEATURE_11BE_MLO
/*
 * Multi link IE field offsets
 *  ------------------------------------------------------------------------
 * | EID(1) | Len (1) | EID_EXT (1) | ML_CONTROL (2) | CMN_INFO (var) | ... |
 *  ------------------------------------------------------------------------
 */
#define ML_CONTROL_OFFSET 3
#define ML_CMN_INFO_OFFSET ML_CONTROL_OFFSET + 2

#define CMN_INFO_MLD_ADDR_PRESENT_BIT     BIT(4)
#define CMN_INFO_LINK_ID_PRESENT_BIT      BIT(5)
#define LINK_INFO_MAC_ADDR_PRESENT_BIT    BIT(5)

static uint8_t util_get_link_info_offset(uint8_t *ml_ie)
{
	uint8_t offset = ML_CMN_INFO_OFFSET;
	uint8_t ml_ie_len = ml_ie[1];
	uint16_t multi_link_ctrl = *(uint16_t *)(ml_ie + ML_CONTROL_OFFSET);

	offset += (BIT(4) & multi_link_ctrl) * 6 +
		  (BIT(5) & multi_link_ctrl) * 1 +
		  (BIT(6) & multi_link_ctrl) * 1 +
		  (BIT(7) & multi_link_ctrl) * 2 +
		  (BIT(8) & multi_link_ctrl) * 2 +
		  (BIT(9) & multi_link_ctrl) * 2;

	if (offset < ml_ie_len)
		return offset;

	return 0;
}

static void util_get_partner_link_info(struct scan_cache_entry *scan_entry)
{
	uint8_t *ml_ie = scan_entry->ie_list.multi_link;
	uint8_t offset = util_get_link_info_offset(ml_ie);
	uint16_t sta_ctrl;
	qdf_size_t ml_ie_len = ml_ie[TAG_LEN_POS] + sizeof(struct ie_header);

	/* Update partner info  from RNR IE */
	qdf_mem_copy(&scan_entry->ml_info.link_info[0].link_addr,
		     &scan_entry->rnr.bss_info[0].bssid, 6);

	scan_entry->ml_info.link_info[0].link_id =
				scan_entry->rnr.bss_info[0].mld_info.link_id;

	if (!offset ||
	    (offset + sizeof(struct wlan_ml_bv_linfo_perstaprof) >= ml_ie_len)) {
		scm_err_rl("incorrect offset value %d", offset);
		return;
	}

	/* TODO: loop through all the STA info fields */

	/* Sub element ID 0 represents Per-STA Profile */
	if (ml_ie[offset] == 0) {
		/* Skip sub element ID and length fields */
		offset += 2;
		sta_ctrl = *(uint16_t *)(ml_ie + offset);
		/* Skip STA control field */
		offset += 2;

		scan_entry->ml_info.link_info[0].link_id = sta_ctrl & 0xF;
		if (sta_ctrl & LINK_INFO_MAC_ADDR_PRESENT_BIT) {
			qdf_mem_copy(
				&scan_entry->ml_info.link_info[0].link_addr,
				ml_ie + offset, 6);
			scm_debug("Found partner info in ML IE");
			return;
		}
	}
}

static void util_scan_update_ml_info(struct scan_cache_entry *scan_entry)
{
	uint8_t *ml_ie = scan_entry->ie_list.multi_link;
	uint16_t multi_link_ctrl;
	uint8_t offset;
	uint8_t *end_ptr = NULL;

	if (!scan_entry->ie_list.multi_link) {
		return;
	}

	end_ptr = ml_ie + ml_ie[TAG_LEN_POS] + sizeof(struct ie_header);

	multi_link_ctrl = *(uint16_t *)(ml_ie + ML_CONTROL_OFFSET);

	/* TODO: update ml_info based on ML IE */

	offset = ML_CMN_INFO_OFFSET;
	/* TODO: Add proper parsing based on presence bitmap */
	if (multi_link_ctrl & CMN_INFO_MLD_ADDR_PRESENT_BIT) {
		if ((ml_ie + offset + QDF_MAC_ADDR_SIZE) <= end_ptr) {
			qdf_mem_copy(&scan_entry->ml_info.mld_mac_addr,
				     ml_ie + offset, QDF_MAC_ADDR_SIZE);
			offset += QDF_MAC_ADDR_SIZE;
		}
	}

	/* TODO: Decode it from ML IE */
	scan_entry->ml_info.num_links = 2;

	/**
	 * Copy Link ID & MAC address of the scan cache entry as first entry
	 * in the partner info list
	 */
	if (multi_link_ctrl & CMN_INFO_LINK_ID_PRESENT_BIT) {
		if (&ml_ie[offset] < end_ptr)
			scan_entry->ml_info.self_link_id = ml_ie[offset] & 0x0F;
	}

	util_get_partner_link_info(scan_entry);
}
#else
static void util_scan_update_ml_info(struct scan_cache_entry *scan_entry)
{
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
	qdf_freq_t chan_freq = 0;
	bool he_6g_dup_bcon = false;
	uint8_t band_mask;

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

	scan_entry->snr = rx_param->snr;
	scan_entry->avg_snr = WLAN_SNR_IN(scan_entry->snr);
	scan_entry->rssi_raw = rx_param->rssi;
	scan_entry->avg_rssi = WLAN_RSSI_IN(scan_entry->rssi_raw);
	scan_entry->tsf_delta = rx_param->tsf_delta;
	scan_entry->pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	/* Copy per chain rssi to scan entry */
	qdf_mem_copy(scan_entry->per_chain_rssi, rx_param->rssi_ctl,
		     WLAN_MGMT_TXRX_HOST_MAX_ANTENNA);
	band_mask = BIT(wlan_reg_freq_to_band(rx_param->chan_freq));

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
	 * In case if the beacon doesn't have
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
	status = util_scan_populate_bcn_ie_list(pdev, scan_entry, &chan_freq,
						band_mask);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_debug(QDF_MAC_ADDR_FMT": failed to parse beacon IE",
			  QDF_MAC_ADDR_REF(scan_entry->bssid.bytes));
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

	if (!chan_freq && util_scan_entry_hecap(scan_entry)) {
		status = util_scan_get_chan_from_he_6g_params(pdev, scan_entry,
							      &chan_freq,
							      &he_6g_dup_bcon,
							      band_mask);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_mem_free(scan_entry->raw_frame.ptr);
			qdf_mem_free(scan_entry);
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (chan_freq)
		scan_entry->channel.chan_freq = chan_freq;

	/* If no channel info is present in beacon use meta channel */
	if (!scan_entry->channel.chan_freq) {
		scan_entry->channel.chan_freq = rx_param->chan_freq;
	} else if (rx_param->chan_freq !=
	   scan_entry->channel.chan_freq) {
		if (!wlan_reg_is_49ghz_freq(scan_entry->channel.chan_freq) &&
		    !he_6g_dup_bcon)
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

	scan_entry->phy_mode = util_scan_get_phymode(pdev, scan_entry);

	scan_entry->nss = util_scan_scm_calc_nss_supported_by_ap(scan_entry);
	scm_fill_adaptive_11r_cap(scan_entry);
	util_scan_set_security(scan_entry);

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

	util_scan_update_ml_info(scan_entry);

	scan_node->entry = scan_entry;
	qdf_list_insert_front(scan_list, &scan_node->node);

	return status;
}

#ifdef WLAN_FEATURE_MBSSID
/*
 * util_is_noninh_ie() - find the noninhertance information element
 * in the received frame's IE list, so that we can stop inheriting that IE
 * in the caller function.
 *
 * @elem_id: Element ID in the received frame's IE, which is being processed.
 * @non_inh_list: pointer to the non inherited list of element IDs or
 *                list of extension element IDs.
 * @len: Length of non inheritance IE list
 *
 * Return: False if the element ID is not found or else return true
 */
static bool util_is_noninh_ie(uint8_t elem_id,
			      uint8_t *non_inh_list,
			      int8_t len)
{
	int count;

	for (count = 0; count < len; count++) {
		if (elem_id == non_inh_list[count])
			return true;
	}

	return false;
}

/*
 * util_scan_find_noninheritance_ie() - find noninheritance information element
 * This block of code is to identify if there is any non-inheritance element
 * present as part of the nontransmitted BSSID profile.
 * @elem_id: element id
 * @ies: pointer consisting of IEs
 * @len: IE length
 *
 * Return: NULL if the element ID is not found or if IE pointer is NULL else
 * pointer to the first byte of the requested element
 */
static uint8_t
*util_scan_find_noninheritance_ie(uint8_t elem_id, uint8_t *ies,
				  int32_t len)
{
	if (!ies)
		return NULL;

	while ((len >= MIN_IE_LEN + 1) && len >= ies[TAG_LEN_POS] + MIN_IE_LEN)
	{
		if ((ies[ID_POS] == elem_id) &&
		    (ies[ELEM_ID_EXTN_POS] ==
		     WLAN_EXTN_ELEMID_NONINHERITANCE)) {
			return ies;
		}
		len -= ies[TAG_LEN_POS] + MIN_IE_LEN;
		ies += ies[TAG_LEN_POS] + MIN_IE_LEN;
	}

	return NULL;
}
#endif

/*
 * util_scan_find_ie() - find information element
 * @eid: element id
 * @ies: pointer consisting of IEs
 * @len: IE length
 *
 * Return: NULL if the element ID is not found or if IE pointer is NULL else
 * pointer to the first byte of the requested element
 */
static uint8_t *util_scan_find_ie(uint8_t eid, uint8_t *ies,
				  int32_t len)
{
	if (!ies)
		return NULL;

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
	uint8_t lsb_n;
	int i;

	for (i = 0; i < QDF_MAC_ADDR_SIZE; i++)
		new_bssid_addr[i] = bssid[i];

	lsb_n = new_bssid_addr[5] & ((1 << max_bssid) - 1);

	new_bssid_addr[5] &= ~((1 << max_bssid) - 1);
	new_bssid_addr[5] |= (lsb_n + mbssid_index) % (1 << max_bssid);
}

/*
 * util_parse_noninheritance_list() - This block of code will be executed only
 * if there is a valid non inheritance IE present in the nontx profile.
 * Host need not inherit those list of element IDs and list of element ID
 * extensions from the transmitted BSSID profile.
 * Since non-inheritance element is an element ID extension, it should
 * be part of extension element. So first we need to find if there are
 * any extension element present in the nontransmitted BSSID profile.
 * @extn_elem: If valid, it points to the element ID field of
 * extension element tag in the nontransmitted BSSID profile.
 * It may or may not have non inheritance tag present.
 *      _____________________________________________
 *     |         |       |       |List of|List of    |
 *     | Element |Length |Element|Element|Element ID |
 *     |  ID     |       |ID extn| IDs   |Extension  |
 *     |_________|_______|_______|_______|___________|
 * List of Element IDs:
 *      __________________
 *     |         |        |
 *     |  Length |Element |
 *     |         |ID List |
 *     |_________|________|
 * List of Element ID Extensions:
 *      __________________________
 *     |         |                |
 *     |  Length |Element ID      |
 *     |         |extension List  |
 *     |_________|________________|
 * @elem_list: Element ID list
 * @extn_elem_list: Element ID exiension list
 * @non_inheritance_ie: Non inheritance IE information
 */

static void util_parse_noninheritance_list(uint8_t *extn_elem,
					   uint8_t **elem_list,
					   uint8_t **extn_elem_list,
					   struct non_inheritance_ie *ninh)
{
	int8_t extn_rem_len = 0;

	if (extn_elem[ELEM_ID_LIST_LEN_POS] < extn_elem[TAG_LEN_POS]) {
		/*
		 * extn_rem_len represents the number of bytes after
		 * the length subfield of list of Element IDs.
		 * So here, extn_rem_len should be equal to
		 * Element ID list + Length subfield of Element ID
		 * extension list + Element ID extension list.
		 *
		 * Here we have taken two pointers pointing to the
		 * element ID list and element ID extension list
		 * which we will use to detect the same elements
		 * in the transmitted BSSID profile and choose not
		 * to inherit those elements while constructing the
		 * frame for nontransmitted BSSID profile.
		 */
		extn_rem_len = extn_elem[TAG_LEN_POS] - MIN_IE_LEN;
		ninh->non_inherit = true;

		if (extn_rem_len && extn_elem[ELEM_ID_LIST_LEN_POS]) {
			if (extn_rem_len >= extn_elem[ELEM_ID_LIST_LEN_POS]) {
				ninh->list_len =
					extn_elem[ELEM_ID_LIST_LEN_POS];
				*elem_list = extn_elem + ELEM_ID_LIST_POS;
				extn_rem_len -= ninh->list_len;
			} else {
				/*
				 * Corrupt frame. length subfield of
				 * element ID list is greater than
				 * what it should be. Go ahead with
				 * frame generation but do not honour
				 * the non inheritance part. Also, mark
				 * the element ID in subcopy as 0, so
				 * that this element info will not
				 * be copied.
				 */
				ninh->non_inherit = false;
				extn_elem[0] = 0;
			}
		}

		extn_rem_len--;
		if (extn_rem_len > 0) {
			if (!ninh->list_len) {
				ninh->extn_len =
					extn_elem[ELEM_ID_LIST_LEN_POS + 1];
			} else {
				ninh->extn_len =
					extn_elem[ELEM_ID_LIST_POS +
					ninh->list_len];
			}

			if (extn_rem_len != ninh->extn_len) {
				/*
				 * Corrupt frame. length subfield of
				 * element ID extn list is not
				 * what it should be. Go ahead with
				 * frame generation but do not honour
				 * the non inheritance part. Also, mark
				 * the element ID in subcopy as 0, so
				 * that this element info will not
				 * be copied.
				 */
				ninh->non_inherit = false;
				extn_elem[0] = 0;
			}

			if (ninh->extn_len) {
				*extn_elem_list =
					(extn_elem + ninh->list_len +
					 ELEM_ID_LIST_POS + 1);
			}
		}
	}
}

#ifdef WLAN_FEATURE_11BE_MLO
static bool util_is_ml_ie(uint8_t *pos)
{
	if (pos[PAYLOAD_START_POS] == WLAN_EXTN_ELEMID_MULTI_LINK)
		return true;

	return false;
}

/**
 * util_handle_rnr_ie_for_mbssid() - parse and modify RNR IE for MBSSID feature
 * @rnr: The pointer to RNR IE
 * @bssid_index: BSSID index from MBSSID index IE
 * @pos: The buffer pointer to save the transformed RNR IE, caller is expected
 *       to supply a buffer that is at least as big as @rnr
 *
 * Per the description about Neighbor AP Information field about MLD
 * parameters subfield in section 9.4.2.170.2 of Draft P802.11be_D1.4.
 * If the reported AP is affiliated with the same MLD of the reporting AP,
 * the TBTT information is skipped; If the reported AP is affiliated with
 * the same MLD of the nontransmitted BSSID, the TBTT information is
 * copied and the MLD ID is changed to 0.
 *
 * Return: Length of the element written to @pos
 */
static int util_handle_rnr_ie_for_mbssid(const uint8_t *rnr,
					 uint8_t bssid_index, uint8_t *pos)
{
	size_t rnr_len;
	const uint8_t *data, *rnr_end;
	uint8_t *rnr_new;
	struct neighbor_ap_info_field *neighbor_ap_info;
	struct rnr_mld_info *mld_param;
	uint8_t tbtt_type, tbtt_len, tbtt_count;
	uint8_t mld_pos, mld_id;
	int32_t i, copy_len;
	/* The count of TBTT info field whose MLD ID equals to 0 in a neighbor
	 * AP information field.
	 */
	uint32_t tbtt_info_field_count;
	/* The total bytes of TBTT info fields whose MLD ID equals to 0 in
	 * current RNR IE.
	 */
	uint32_t tbtt_info_field_len = 0;
	uint8_t nbr_ap_info_len = sizeof(struct neighbor_ap_info_field);

	rnr_len = rnr[TAG_LEN_POS];
	rnr_end = rnr + rnr_len + MIN_IE_LEN;
	rnr_new = pos;
	qdf_mem_copy(pos, rnr, MIN_IE_LEN);
	pos += MIN_IE_LEN;

	data = rnr + PAYLOAD_START_POS;
	while (data + sizeof(struct neighbor_ap_info_field) <= rnr_end) {
		neighbor_ap_info = (struct neighbor_ap_info_field *)data;
		tbtt_count = neighbor_ap_info->tbtt_header.tbtt_info_count;
		tbtt_len = neighbor_ap_info->tbtt_header.tbtt_info_length;
		tbtt_type = neighbor_ap_info->tbtt_header.tbbt_info_fieldtype;
		scm_debug("channel number %d, op class %d, bssid_index %d",
			  neighbor_ap_info->channel_number,
			  neighbor_ap_info->operting_class, bssid_index);
		scm_debug("tbtt_count %d, tbtt_length %d, tbtt_type %d",
			  tbtt_count, tbtt_len, tbtt_type);

		copy_len = tbtt_len * (tbtt_count + 1) +
			   nbr_ap_info_len;
		if (data + copy_len > rnr_end)
			return 0;

		if (tbtt_len >=
		    TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM_20MHZ_PSD_MLD_PARAM)
			mld_pos =
			     TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM_20MHZ_PSD;
		else
			mld_pos = 0;

		/* If MLD params do not exist, copy this neighbor AP
		 * information field.
		 * Per Draft P802.11be_D1.4, tbtt_type value 1, 2 and 3
		 * are reserved,
		 */
		if (mld_pos == 0 || tbtt_type != 0) {
			scm_debug("no MLD params, tbtt_type %d", tbtt_type);
			qdf_mem_copy(pos, data, copy_len);
			pos += copy_len;
			data += copy_len;
			continue;
		}

		qdf_mem_copy(pos, data, nbr_ap_info_len);
		neighbor_ap_info = (struct neighbor_ap_info_field *)pos;
		pos += nbr_ap_info_len;
		data += nbr_ap_info_len;

		tbtt_info_field_count = 0;
		for (i = 0; i < tbtt_count + 1; i++) {
			mld_param = (struct rnr_mld_info *)&data[mld_pos];
			mld_id = mld_param->mld_id;

			/* Refer to Draft P802.11be_D1.4
			 * 9.4.2.170.2 Neighbor AP Information field about
			 * MLD parameters subfield
			 */
			if (mld_id == 0) {
				/* Skip this TBTT information since this
				 * reported AP is affiliated with the same MLD
				 * of the reporting AP who sending the frame
				 * carrying this element.
				 */
				tbtt_info_field_len += tbtt_len;
				data += tbtt_len;
				tbtt_info_field_count++;
			} else if (mld_id == bssid_index) {
				/* Copy this TBTT information and change MLD
				 * to 0 as this reported AP is affiliated with
				 * the same MLD of the nontransmitted BSSID.
				 */
				qdf_mem_copy(pos, data, tbtt_len);
				mld_param =
					(struct rnr_mld_info *)&pos[mld_pos];
				scm_debug("change MLD ID from %d to 0",
					  mld_param->mld_id);
				mld_param->mld_id = 0;
				data += tbtt_len;
				pos += tbtt_len;
			} else {
				qdf_mem_copy(pos, data, tbtt_len);
				data += tbtt_len;
				pos += tbtt_len;
			}
		}

		scm_debug("skip %d neighbor info", tbtt_info_field_count);
		if (tbtt_info_field_count == (tbtt_count + 1)) {
			/* If all the TBTT information are skipped, then also
			 * revert the neighbor AP info which has been copied.
			 */
			pos -= nbr_ap_info_len;
			tbtt_info_field_len += nbr_ap_info_len;
		} else {
			neighbor_ap_info->tbtt_header.tbtt_info_count -=
							tbtt_info_field_count;
		}
	}

	rnr_new[TAG_LEN_POS] = rnr_len - tbtt_info_field_len;
	if (rnr_new[TAG_LEN_POS] > 0)
		rnr_len = rnr_new[TAG_LEN_POS] + MIN_IE_LEN;
	else
		rnr_len = 0;

	return rnr_len;
}
#else
static bool util_is_ml_ie(uint8_t *pos)
{
	return false;
}

static int util_handle_rnr_ie_for_mbssid(const uint8_t *rnr,
					 uint8_t bssid_index, uint8_t *pos)
{
	return 0;
}
#endif

static size_t util_oui_header_len(uint8_t *ie)
{
	/* Cisco Vendor Specific IEs doesn't have subtype in
	 * their VSIE header, therefore skip subtype
	 */
	if (ie[0] == 0x00 && ie[1] == 0x40 && ie[2] == 0x96)
		return OUI_LEN - 1;
	return OUI_LEN;
}

static uint32_t util_gen_new_ie(uint8_t *ie, uint32_t ielen,
				uint8_t *subelement,
				size_t subie_len, uint8_t *new_ie,
				uint8_t bssid_index)
{
	uint8_t *pos, *tmp;
	const uint8_t *tmp_old, *tmp_new;
	uint8_t *sub_copy, *extn_elem = NULL;
	struct non_inheritance_ie ninh = {0};
	uint8_t *elem_list = NULL, *extn_elem_list = NULL;
	size_t tmp_rem_len;

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
		scm_debug(" SSID %.*s", tmp_new[1],
			  &tmp_new[PAYLOAD_START_POS]);
		if ((pos + tmp_new[1] + MIN_IE_LEN) <=
		    (new_ie + ielen)) {
			qdf_mem_copy(pos, tmp_new,
				     (tmp_new[1] + MIN_IE_LEN));
			pos += (tmp_new[1] + MIN_IE_LEN);
		}
	}

	extn_elem = util_scan_find_noninheritance_ie(WLAN_ELEMID_EXTN_ELEM,
						     sub_copy, subie_len);

	if (extn_elem && extn_elem[TAG_LEN_POS] >= VALID_ELEM_LEAST_LEN) {
		if (((extn_elem + extn_elem[1] + MIN_IE_LEN) - sub_copy)
		    <= subie_len)
			util_parse_noninheritance_list(extn_elem, &elem_list,
						       &extn_elem_list, &ninh);
	}

	/* go through IEs in ie (skip SSID) and subelement,
	 * merge them into new_ie
	 */
	tmp_old = util_scan_find_ie(WLAN_ELEMID_SSID, ie, ielen);
	tmp_old = (tmp_old) ? tmp_old + tmp_old[1] + MIN_IE_LEN : ie;

	if (((tmp_old + MIN_IE_LEN) - ie) >= ielen) {
		qdf_mem_free(sub_copy);
		return 0;
	}

	while (((tmp_old + tmp_old[1] + MIN_IE_LEN) - ie) <= ielen) {
		ninh.non_inh_ie_found = 0;
		if (ninh.non_inherit) {
			if (ninh.list_len) {
				ninh.non_inh_ie_found =
					util_is_noninh_ie(tmp_old[0],
							  elem_list,
							  ninh.list_len);
			}

			if (!ninh.non_inh_ie_found &&
			    ninh.extn_len &&
			    (tmp_old[0] == WLAN_ELEMID_EXTN_ELEM)) {
				ninh.non_inh_ie_found =
					util_is_noninh_ie(tmp_old[2],
							  extn_elem_list,
							  ninh.extn_len);
			}
		}

		if (ninh.non_inh_ie_found || (tmp_old[0] == 0)) {
			if (((tmp_old + tmp_old[1] + MIN_IE_LEN) - ie) >=
			    (ielen - MIN_IE_LEN))
				break;
			tmp_old += tmp_old[1] + MIN_IE_LEN;
			continue;
		}

		tmp = (uint8_t *)util_scan_find_ie(tmp_old[0], sub_copy,
						   subie_len);
		if (!tmp) {
			/* ie in old ie but not in subelement */
			if (tmp_old[0] == WLAN_ELEMID_REDUCED_NEIGHBOR_REPORT) {
				/* handle rnr ie for mbssid*/
				pos +=
				    util_handle_rnr_ie_for_mbssid(tmp_old,
								  bssid_index,
								  pos);
			} else if (tmp_old[0] != WLAN_ELEMID_MULTIPLE_BSSID) {
				if ((pos + tmp_old[1] + MIN_IE_LEN) <=
				    (new_ie + ielen)) {
					qdf_mem_copy(pos, tmp_old,
						     (tmp_old[1] +
						      MIN_IE_LEN));
					pos += tmp_old[1] + MIN_IE_LEN;
				}
			}
		} else {
			/* ie in transmitting ie also in subelement,
			 * copy from subelement and flag the ie in subelement
			 * as copied (by setting eid field to 0xff).
			 * To determine if the vendor ies are same:
			 * 1. For Cisco OUI, compare only OUI + type
			 * 2. For other OUI, compare OUI + type + subType
			 */
			tmp_rem_len = subie_len - (tmp - sub_copy);
			if (tmp_old[0] == WLAN_ELEMID_VENDOR &&
			    tmp_rem_len >= MIN_VENDOR_TAG_LEN) {
				if (!qdf_mem_cmp(tmp_old + PAYLOAD_START_POS,
						 tmp + PAYLOAD_START_POS,
						 util_oui_header_len(tmp +
								     PAYLOAD_START_POS))) {
					/* same vendor ie, copy from
					 * subelement
					 */
					if ((pos + tmp[1] + MIN_IE_LEN) <=
					    (new_ie + ielen)) {
						qdf_mem_copy(pos, tmp,
							     tmp[1] +
							     MIN_IE_LEN);
						pos += tmp[1] + MIN_IE_LEN;
						tmp[0] = 0;
					}
				} else {
					if ((pos + tmp_old[1] +
					     MIN_IE_LEN) <=
					    (new_ie + ielen)) {
						qdf_mem_copy(pos, tmp_old,
							     tmp_old[1] +
							     MIN_IE_LEN);
						pos += tmp_old[1] +
							MIN_IE_LEN;
					}
				}
			} else if (tmp_old[0] == WLAN_ELEMID_EXTN_ELEM &&
				   tmp_rem_len >= (MIN_IE_LEN + 1)) {
				if (tmp_old[PAYLOAD_START_POS] ==
				    tmp[PAYLOAD_START_POS] &&
				    !util_is_ml_ie(tmp)) {
					/* same ie, copy from subelement
					 * but multi link IE is exception,
					 * it needs to copy from main frame
					 * for full info.
					 */
					if ((pos + tmp[1] + MIN_IE_LEN) <=
					    (new_ie + ielen)) {
						qdf_mem_copy(pos, tmp,
							     tmp[1] +
							     MIN_IE_LEN);
						pos += tmp[1] + MIN_IE_LEN;
						tmp[0] = 0;
					}
				} else {
					if ((pos + tmp_old[1] + MIN_IE_LEN) <=
					    (new_ie + ielen)) {
						qdf_mem_copy(pos, tmp_old,
							     tmp_old[1] +
							     MIN_IE_LEN);
						pos += tmp_old[1] +
							MIN_IE_LEN;
					}
				}

			} else {
				/* copy ie from subelement into new ie */
				if ((pos + tmp[1] + MIN_IE_LEN) <=
				    (new_ie + ielen)) {
					qdf_mem_copy(pos, tmp,
						     tmp[1] + MIN_IE_LEN);
					pos += tmp[1] + MIN_IE_LEN;
					tmp[0] = 0;
				}
			}
		}

		if (((tmp_old + tmp_old[1] + MIN_IE_LEN) - ie) >=
		    (ielen - MIN_IE_LEN))
			break;

		tmp_old += tmp_old[1] + MIN_IE_LEN;
	}

	/* go through subelement again to check if there is any ie not
	 * copied to new ie, skip ssid, capability, bssid-index ie
	 */
	tmp_new = sub_copy;
	while ((subie_len > 0) &&
	       (((tmp_new + tmp_new[1] + MIN_IE_LEN) - sub_copy) <=
		subie_len)) {
		if (!(tmp_new[0] == WLAN_ELEMID_NONTX_BSSID_CAP ||
		      tmp_new[0] == WLAN_ELEMID_SSID ||
		      tmp_new[0] == WLAN_ELEMID_MULTI_BSSID_IDX ||
		      ((tmp_new[0] == WLAN_ELEMID_EXTN_ELEM) &&
		       (tmp_new[2] == WLAN_EXTN_ELEMID_NONINHERITANCE)))) {
			if ((pos + tmp_new[1] + MIN_IE_LEN) <=
			    (new_ie + ielen)) {
				qdf_mem_copy(pos, tmp_new,
					     tmp_new[1] + MIN_IE_LEN);
				pos += tmp_new[1] + MIN_IE_LEN;
			}
		}
		if (((tmp_new + tmp_new[1] + MIN_IE_LEN) - sub_copy) >=
		    (subie_len - 1))
			break;
		tmp_new += tmp_new[1] + MIN_IE_LEN;
	}

	qdf_mem_free(sub_copy);

	if (pos > new_ie)
		return pos - new_ie;
	else
		return 0;
}

static enum nontx_profile_reasoncode
util_handle_nontx_prof(uint8_t *mbssid_elem, uint8_t *subelement,
		       uint8_t *next_subelement,
		       struct scan_mbssid_info *mbssid_info,
		       char *bssid, char *new_bssid)
{
	uint8_t *mbssid_index_ie;
	uint32_t prof_len;

	prof_len = subelement[TAG_LEN_POS];
	/*
	 * If we are executing the split portion of the nontx
	 * profile present in the subsequent MBSSID, then there
	 * is no need of any sanity check for valid BSS profile
	 */

	if (mbssid_info->split_prof_continue) {
		if ((subelement[ID_POS] != 0) ||
		    (subelement[TAG_LEN_POS] < SPLIT_PROF_DATA_LEAST_LEN)) {
			return INVALID_SPLIT_PROF;
		}
	} else {
		if ((subelement[ID_POS] != 0) ||
		    (subelement[TAG_LEN_POS] < VALID_ELEM_LEAST_LEN)) {
			/* not a valid BSS profile */
			return INVALID_NONTX_PROF;
		}
	}

	if (mbssid_info->split_profile) {
		if (next_subelement[PAYLOAD_START_POS] !=
		    WLAN_ELEMID_NONTX_BSSID_CAP) {
			mbssid_info->prof_residue = true;
		}
	}

	if (!mbssid_info->split_prof_continue &&
	    ((subelement[PAYLOAD_START_POS] != WLAN_ELEMID_NONTX_BSSID_CAP) ||
	     (subelement[NONTX_BSSID_CAP_TAG_LEN_POS] != CAP_INFO_LEN))) {
		/* The first element within the Nontransmitted
		 * BSSID Profile is not the Nontransmitted
		 * BSSID Capability element.
		 */
		return INVALID_NONTX_PROF;
	}

	/* found a Nontransmitted BSSID Profile */
	mbssid_index_ie =
		util_scan_find_ie(WLAN_ELEMID_MULTI_BSSID_IDX,
				  (subelement + PAYLOAD_START_POS), prof_len);

	if (!mbssid_index_ie) {
		if (!mbssid_info->prof_residue)
			return INVALID_NONTX_PROF;

		mbssid_info->skip_bssid_copy = true;
	} else if ((mbssid_index_ie[TAG_LEN_POS] < 1) ||
		   (mbssid_index_ie[BSS_INDEX_POS] == 0)) {
		/* No valid Multiple BSSID-Index element */
		return INVALID_NONTX_PROF;
	}

	if (!mbssid_info->skip_bssid_copy) {
		qdf_mem_copy(mbssid_info->trans_bssid,
			     bssid, QDF_MAC_ADDR_SIZE);
		mbssid_info->profile_num =
			mbssid_index_ie[BSS_INDEX_POS];
		util_gen_new_bssid(bssid,
				   mbssid_elem[MBSSID_INDICATOR_POS],
				   mbssid_index_ie[BSS_INDEX_POS],
				   new_bssid);
	}
	/* In single MBSS IE, there could be subelement holding
	 * remaining vendor IEs of non tx profile from last MBSS IE
	 * [split profile] and new non tx profile, hence reset
	 * skip_bssid_copy flag after each subelement processing
	 */
	mbssid_info->skip_bssid_copy = false;
	return VALID_NONTX_PROF;
}

/*
 * What's split profile:
 *  If any nontransmitted BSSID profile is fragmented across
 * multiple MBSSID elements, then it is called split profile.
 * For a split profile to exist we need to have at least two
 * MBSSID elements as part of the RX beacon or probe response
 * Hence, first we need to identify the next MBSSID element
 * and check for the 5th bit from the starting of the next
 * MBSSID IE and if it does not have Nontransmitted BSSID
 * capability element, then it's a split profile case.
 */
static bool util_scan_is_split_prof_found(uint8_t *next_elem,
					  uint8_t *ie, uint32_t ielen)
{
	uint8_t *next_mbssid_elem;

	if ((next_elem + MIN_IE_LEN + VALID_ELEM_LEAST_LEN) > (ie + ielen))
		return false;

	if (next_elem[0] == WLAN_ELEMID_MULTIPLE_BSSID) {
		if ((next_elem[TAG_LEN_POS] >= VALID_ELEM_LEAST_LEN) &&
		    (next_elem[SUBELEM_DATA_POS_FROM_MBSSID] !=
		     WLAN_ELEMID_NONTX_BSSID_CAP)) {
			return true;
		}
	} else {
		next_mbssid_elem =
			util_scan_find_ie(WLAN_ELEMID_MULTIPLE_BSSID,
					  next_elem,
					  ielen - (next_elem - ie));
		if (!next_mbssid_elem)
			return false;

		if ((next_mbssid_elem[TAG_LEN_POS] >= VALID_ELEM_LEAST_LEN) &&
		    (next_mbssid_elem[SUBELEM_DATA_POS_FROM_MBSSID] !=
		     WLAN_ELEMID_NONTX_BSSID_CAP)) {
			return true;
		}
	}

	return false;
}

static QDF_STATUS util_scan_parse_mbssid(struct wlan_objmgr_pdev *pdev,
					 uint8_t *frame, qdf_size_t frame_len,
					 uint32_t frm_subtype,
					 struct mgmt_rx_event_params *rx_param,
					 qdf_list_t *scan_list)
{
	struct wlan_bcn_frame *bcn;
	struct wlan_frame_hdr *hdr;
	struct scan_mbssid_info mbssid_info = {0};
	QDF_STATUS status;
	uint8_t *pos, *subelement, *next_elem;
	uint8_t *mbssid_elem;
	uint32_t subie_len, new_ie_len, ielen;
	uint8_t *next_subelement = NULL;
	uint8_t new_bssid[QDF_MAC_ADDR_SIZE], bssid[QDF_MAC_ADDR_SIZE];
	uint8_t *new_ie, *split_prof_start = NULL, *split_prof_end = NULL;
	uint8_t *ie, *new_frame = NULL;
	int new_frame_len = 0, split_prof_len = 0;
	enum nontx_profile_reasoncode retval;
	uint8_t *nontx_profile = NULL;

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

	new_ie = qdf_mem_malloc(ielen);
	if (!new_ie)
		return QDF_STATUS_E_NOMEM;

	while (pos < (ie + ielen + MIN_IE_LEN)) {
		mbssid_elem =
			util_scan_find_ie(WLAN_ELEMID_MULTIPLE_BSSID, pos,
					  ielen - (pos - ie));
		if (!mbssid_elem)
			break;

		/*
		 * The max_bssid_indicator field is mandatory, therefore the
		 * length of the MBSSID element should atleast be 1.
		 */
		if (!mbssid_elem[TAG_LEN_POS]) {
			scm_debug_rl("MBSSID IE is of length zero");
			break;
		}

		mbssid_info.profile_count =
			(1 << mbssid_elem[MBSSID_INDICATOR_POS]);

		next_elem =
			mbssid_elem + mbssid_elem[TAG_LEN_POS] + MIN_IE_LEN;

		/* Skip Element ID, Len, MaxBSSID Indicator */
		if (!mbssid_info.split_profile &&
		    (mbssid_elem[TAG_LEN_POS] < VALID_ELEM_LEAST_LEN)) {
			break;
		}

		/*
		 * Find if the next IE is MBSSID, if not, then scan through
		 * the IE list and find the next MBSSID tag, if present.
		 * Once we find the MBSSID tag, check if this MBSSID tag has
		 * the other fragmented part of the non Tx profile.
		 */

		mbssid_info.split_profile =
			util_scan_is_split_prof_found(next_elem, ie, ielen);

		for (subelement = mbssid_elem + SUBELEMENT_START_POS;
		     subelement < (next_elem - 1);
		     subelement += MIN_IE_LEN + subelement[TAG_LEN_POS]) {
			subie_len = subelement[TAG_LEN_POS];

			/*
			 * if prof_residue is true, that means we are
			 * in the continuation of the fragmented profile part,
			 * present in the next MBSSD IE else this profile
			 * is a non fragmented non tx BSSID profile.
			 */

			if (mbssid_info.prof_residue)
				mbssid_info.split_prof_continue = true;
			else
				mbssid_info.split_prof_continue = false;

			if (subie_len > MAX_SUBELEM_LEN) {
				scm_err_rl("Corrupt frame with subie_len: %d\n"
					   "split_prof_continue: %d\n"
					   "prof_residue: %d\n",
					   subie_len,
					   mbssid_info.split_prof_continue,
					   mbssid_info.prof_residue);
				if (mbssid_info.split_prof_continue) {
					qdf_mem_free(split_prof_start);
					split_prof_start = NULL;
				}

				qdf_mem_free(new_ie);
				return QDF_STATUS_E_INVAL;
			}

			if ((next_elem - subelement) <
			    (MIN_IE_LEN + subie_len))
				break;

			next_subelement = subelement + subie_len + MIN_IE_LEN;
			retval = util_handle_nontx_prof(mbssid_elem, subelement,
							next_subelement,
							&mbssid_info,
							bssid, new_bssid);

			if (retval == INVALID_SPLIT_PROF) {
				scm_err_rl("Corrupt frame with ID_POS: %d\n"
					   "TAG_LEN_POS: %d\n",
					   subelement[ID_POS],
					   subelement[TAG_LEN_POS]);
				qdf_mem_free(split_prof_start);
				split_prof_start = NULL;
				qdf_mem_free(new_ie);
				return QDF_STATUS_E_INVAL;
			} else if (retval == INVALID_NONTX_PROF) {
				continue;
			}

			/*
			 * Merging parts of nontx profile-
			 * Just for understanding, let's make an assumption
			 * that nontx profile is fragmented across MBSSIE1
			 * and MBSSIE2.
			 * mbssid_info.prof_residue being set indicates
			 * that the ongoing nontx profile is part of split
			 * profile, whose other fragmented part is present
			 * in MBSSIE2.
			 * So once prof_residue is set, we need to
			 * identify whether we are accessing the split
			 * profile in MBSSIE1 or MBSSIE2.
			 * If we are in MBSSIE1, then copy the part of split
			 * profile from MBSSIE1 into a new buffer and then
			 * move to the next part of the split profile which
			 * is present in MBSSIE2 and append that part into
			 * the new buffer.
			 * Once the full profile is accumulated, go ahead with
			 * the ie generation and length calculation of the
			 * new frame.
			 */

			if (mbssid_info.prof_residue) {
				if (!mbssid_info.split_prof_continue) {
					split_prof_start =
						qdf_mem_malloc(ielen);
					if (!split_prof_start) {
						scm_err_rl("Malloc failed");
						qdf_mem_free(new_ie);
						return QDF_STATUS_E_NOMEM;
					}

					qdf_mem_copy(split_prof_start,
						     subelement,
						     (subie_len +
						      MIN_IE_LEN));
					split_prof_end = (split_prof_start +
							  subie_len +
							  MIN_IE_LEN);
					break;
				}

				/*
				 * Currently we are accessing other part of the
				 * split profile present in the subsequent
				 * MBSSIE. There is a possibility that one
				 * non tx profile is spread across more than
				 * two MBSSID tag as well. This code will
				 * handle such scenario.
				 */

				qdf_mem_copy(split_prof_end,
					     (subelement + MIN_IE_LEN),
					     subie_len);
				split_prof_end =
					(split_prof_end + subie_len);

				/*
				 * When to stop the process of accumulating
				 * parts of split profile, is decided by
				 * mbssid_info.prof_residue. prof_residue
				 * could be made false if there is not any
				 * continuation of the split profile.
				 * which could be identified by two factors
				 * 1. By checking if the next MBSSIE's first
				 * non tx profile is not a fragmented one or
				 * 2. there is a probability that first
				 * subelement of MBSSIE2 is end if split
				 * profile and the next subelement of MBSSIE2
				 * is a non split one.
				 */

				if (!mbssid_info.split_profile ||
				    (next_subelement[PAYLOAD_START_POS] ==
				     WLAN_ELEMID_NONTX_BSSID_CAP)) {
					mbssid_info.prof_residue = false;
				}

				/*
				 * Until above mentioned conditions are met,
				 * we need to iterate and keep accumulating
				 * the split profile contents.
				 */

				if (mbssid_info.prof_residue)
					break;

				split_prof_len =
					(split_prof_end -
					 split_prof_start - MIN_IE_LEN);
			}

			if (mbssid_info.split_prof_continue) {
				if (!split_prof_start)
					break;
				nontx_profile = split_prof_start;
				subie_len = split_prof_len;
			} else {
				nontx_profile = subelement;
			}

			new_ie_len =
				util_gen_new_ie(ie, ielen,
						(nontx_profile +
						 PAYLOAD_START_POS),
						subie_len, new_ie,
						mbssid_info.profile_num);

			if (!new_ie_len) {
				if (mbssid_info.split_prof_continue) {
					qdf_mem_free(split_prof_start);
					split_prof_start = NULL;
					split_prof_end = NULL;
					split_prof_len = 0;
				}
				continue;
			}

			new_frame_len = frame_len - ielen + new_ie_len;

			if (new_frame_len < 0) {
				if (mbssid_info.split_prof_continue) {
					qdf_mem_free(split_prof_start);
					split_prof_start = NULL;
				}
				qdf_mem_free(new_ie);
				scm_err("Invalid frame:Stop MBSSIE parsing");
				scm_err("Frame_len: %zu,ielen:%u,new_ie_len:%u",
					frame_len, ielen, new_ie_len);
				return QDF_STATUS_E_INVAL;
			}

			new_frame = qdf_mem_malloc(new_frame_len);
			if (!new_frame) {
				if (mbssid_info.split_prof_continue) {
					qdf_mem_free(split_prof_start);
					split_prof_start = NULL;
				}
				qdf_mem_free(new_ie);
				scm_err_rl("Malloc for new_frame failed");
				scm_err_rl("split_prof_continue: %d",
					   mbssid_info.split_prof_continue);
				return QDF_STATUS_E_NOMEM;
			}

			/*
			 * Copy the header(24byte), timestamp(8 byte),
			 * beaconinterval(2byte) and capability(2byte)
			 */
			qdf_mem_copy(new_frame, frame, FIXED_LENGTH);
			/* Copy the new ie generated from MBSSID profile*/
			hdr = (struct wlan_frame_hdr *)new_frame;
			qdf_mem_copy(hdr->i_addr2, new_bssid,
				     QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(hdr->i_addr3, new_bssid,
				     QDF_MAC_ADDR_SIZE);
			bcn = (struct wlan_bcn_frame *)
				(new_frame + sizeof(struct wlan_frame_hdr));
			/* update the non-tx capability */
			qdf_mem_copy(&bcn->capability,
				     nontx_profile + CAP_INFO_POS,
				     CAP_INFO_LEN);

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
				if (mbssid_info.split_prof_continue) {
					qdf_mem_free(split_prof_start);
					split_prof_start = NULL;
					split_prof_end = NULL;
					split_prof_len = 0;
					qdf_mem_zero(&mbssid_info,
						     sizeof(mbssid_info));
				}
				qdf_mem_free(new_frame);
				scm_err_rl("failed to generate a scan entry");
				scm_err_rl("split_prof_continue: %d",
					   mbssid_info.split_prof_continue);
				break;
			}
			/* scan entry makes its own copy so free the frame*/
			if (mbssid_info.split_prof_continue) {
				qdf_mem_free(split_prof_start);
				split_prof_start = NULL;
				split_prof_end = NULL;
				split_prof_len = 0;
			}
			qdf_mem_free(new_frame);
		}

		pos = next_elem;
	}
	qdf_mem_free(new_ie);

	if (split_prof_start)
		qdf_mem_free(split_prof_start);

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
	uint8_t *mbssid_ie = NULL, *extcap_ie;
	uint32_t ie_len = 0;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct scan_mbssid_info mbssid_info = { 0 };

	hdr = (struct wlan_frame_hdr *)frame;
	bcn = (struct wlan_bcn_frame *)
			   (frame + sizeof(struct wlan_frame_hdr));
	ie_len = (uint16_t)(frame_len -
		sizeof(struct wlan_frame_hdr) -
		offsetof(struct wlan_bcn_frame, ie));

	extcap_ie = util_scan_find_ie(WLAN_ELEMID_XCAPS,
				      (uint8_t *)&bcn->ie, ie_len);
	/* Process MBSSID when Multiple BSSID (Bit 22) is set in Ext Caps */
	if (extcap_ie &&
	    extcap_ie[1] >= 3 && extcap_ie[1] <= WLAN_EXTCAP_IE_MAX_LEN &&
	    (extcap_ie[4] & 0x40)) {
		mbssid_ie = util_scan_find_ie(WLAN_ELEMID_MULTIPLE_BSSID,
					      (uint8_t *)&bcn->ie, ie_len);
		if (mbssid_ie) {
			/* some APs announce the MBSSID ie_len as 1 */
			if (mbssid_ie[TAG_LEN_POS] < 1) {
				scm_debug("MBSSID IE length is wrong %d",
					  mbssid_ie[TAG_LEN_POS]);
				return status;
			}
			qdf_mem_copy(&mbssid_info.trans_bssid,
				     hdr->i_addr3, QDF_MAC_ADDR_SIZE);
			mbssid_info.profile_count = 1 << mbssid_ie[2];
		}
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

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
bool
util_scan_entry_single_pmk(struct wlan_objmgr_psoc *psoc,
			   struct scan_cache_entry *scan_entry)
{
	if (scan_entry->ie_list.single_pmk &&
	    wlan_mlme_is_sae_single_pmk_enabled(psoc))
		return true;

	return false;
}
#endif
