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
 * DOC: contains scan public utility functions
 */

#ifndef _WLAN_SCAN_UTILS_H_
#define _WLAN_SCAN_UTILS_H_

#include <wlan_objmgr_cmn.h>
#include <qdf_mc_timer.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>
#include<wlan_mgmt_txrx_utils_api.h>
#include <wlan_reg_services_api.h>

#define ASCII_SPACE_CHARACTER 32

/**
 * util_is_scan_entry_match() - func to check if both scan entry
 * are from same AP
 * @entry1: scan entry 1
 * @entry2: scan entry 2
 *
 * match the two scan entries
 *
 * Return: true if entry match else false.
 */
bool util_is_scan_entry_match(
	struct scan_cache_entry *entry1,
	struct scan_cache_entry *entry2);

/**
 * util_scan_unpack_beacon_frame() - func to unpack beacon frame to scan entry
 * @pdev: pdev pointer
 * @frame: beacon/probe frame
 * @frame_len: beacon frame len
 * @frm_subtype: beacon or probe
 * @rx_param: rx meta data
 *
 * get the defaults scan params
 *
 * Return: unpacked list of scan entries.
 */
qdf_list_t *util_scan_unpack_beacon_frame(
	struct wlan_objmgr_pdev *pdev,
	uint8_t *frame, qdf_size_t frame_len, uint32_t frm_subtype,
	struct mgmt_rx_event_params *rx_param);

/**
 * util_scan_add_hidden_ssid() - func to add hidden ssid
 * @pdev: pdev pointer
 * @frame: beacon buf
 *
 * Return:
 */
#ifdef WLAN_DFS_CHAN_HIDDEN_SSID
QDF_STATUS
util_scan_add_hidden_ssid(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t bcnbuf);
#else
static inline QDF_STATUS
util_scan_add_hidden_ssid(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t bcnbuf)
{
	return  QDF_STATUS_SUCCESS;
}
#endif /* WLAN_DFS_CHAN_HIDDEN_SSID */

/**
 * util_scan_get_ev_type_name() - converts enum event to printable string
 * @event:      event of type scan_event_type
 *
 * API, converts enum event to printable character string
 *
 * Return:      pointer to printable string
 */
const char *util_scan_get_ev_type_name(enum scan_event_type event);

/**
 * util_scan_get_ev_reason_name() - converts enum reason to printable string
 * @reason      enum of scan completion reason
 *
 * API, converts enum event to printable character string
 *
 * Return:      pointer to printable string
 */
const char *util_scan_get_ev_reason_name(enum scan_completion_reason reason);

/**
 * util_scan_entry_macaddr() - function to read transmitter address
 * @scan_entry: scan entry
 *
 * API, function to read transmitter address of scan entry
 *
 * Return:      pointer to mac address
 */
static inline uint8_t*
util_scan_entry_macaddr(struct scan_cache_entry *scan_entry)
{
	return &(scan_entry->mac_addr.bytes[0]);
}

/**
 * util_scan_entry_bssid() - function to read bssid
 * @scan_entry: scan entry
 *
 * API, function to read bssid of scan entry
 *
 * Return: pointer to mac address
 */
static inline uint8_t*
util_scan_entry_bssid(struct scan_cache_entry *scan_entry)
{
	return &(scan_entry->bssid.bytes[0]);
}

/**
 * util_scan_entry_capinfo() - function to read capibility info
 * @scan_entry: scan entry
 *
 * API, function to read capibility info of scan entry
 *
 * Return: capability info
 */
static inline union wlan_capability
util_scan_entry_capinfo(struct scan_cache_entry *scan_entry)
{
	return scan_entry->cap_info;
}

/**
 * util_scan_entry_beacon_interval() - function to read beacon interval
 * @scan_entry: scan entry
 *
 * API, function to read beacon interval of scan entry
 *
 * Return: beacon interval
 */
static inline uint16_t
util_scan_entry_beacon_interval(struct scan_cache_entry *scan_entry)
{
	return scan_entry->bcn_int;
}

/**
 * util_scan_entry_sequence_number() - function to read sequence number
 * @scan_entry: scan entry
 *
 * API, function to read sequence number of scan entry
 *
 * Return: sequence number
 */
static inline uint16_t
util_scan_entry_sequence_number(struct scan_cache_entry *scan_entry)
{
	return scan_entry->seq_num;
}

/**
 * util_scan_entry_tsf() - function to read tsf
 * @scan_entry: scan entry
 *
 * API, function to read tsf of scan entry
 *
 * Return: tsf
 */
static inline uint8_t*
util_scan_entry_tsf(struct scan_cache_entry *scan_entry)
{
	return scan_entry->tsf_info.data;
}

/**
 * util_scan_entry_reset_timestamp() - function to reset bcn receive timestamp
 * @scan_entry: scan entry
 *
 * API, function to reset bcn receive timestamp of scan entry
 *
 * Return: void
 */
static inline void
util_scan_entry_reset_timestamp(struct scan_cache_entry *scan_entry)
{
	scan_entry->scan_entry_time = 0;
}

/*
 * Macros used for RSSI calculation.
 */
#define WLAN_RSSI_AVERAGING_TIME (5 * 1000) /* 5 seconds */

#define WLAN_RSSI_EP_MULTIPLIER (1<<7)  /* pow2 to optimize out * and / */

#define WLAN_RSSI_LPF_LEN       0
#define WLAN_RSSI_DUMMY_MARKER  0x127

#define WLAN_EP_MUL(x, mul) ((x) * (mul))

#define WLAN_EP_RND(x, mul) ((((x)%(mul)) >= ((mul)/2)) ?\
	((x) + ((mul) - 1)) / (mul) : (x)/(mul))

#define WLAN_RSSI_GET(x) WLAN_EP_RND(x, WLAN_RSSI_EP_MULTIPLIER)

#define RSSI_LPF_THRESHOLD      -20


#define WLAN_RSSI_OUT(x) (((x) != WLAN_RSSI_DUMMY_MARKER) ?     \
	(WLAN_EP_RND((x), WLAN_RSSI_EP_MULTIPLIER)) :  WLAN_RSSI_DUMMY_MARKER)


#define WLAN_RSSI_IN(x)         (WLAN_EP_MUL((x), WLAN_RSSI_EP_MULTIPLIER))

#define WLAN_LPF_RSSI(x, y, len) \
	((x != WLAN_RSSI_DUMMY_MARKER) ? ((((x) << 3) + (y) - (x)) >> 3) : (y))

#define WLAN_RSSI_LPF(x, y) do { \
	if ((y) < RSSI_LPF_THRESHOLD) \
		x = WLAN_LPF_RSSI((x), WLAN_RSSI_IN((y)), WLAN_RSSI_LPF_LEN); \
	} while (0)

#define WLAN_ABS_RSSI_LPF(x, y) do { \
	if ((y) >= (RSSI_LPF_THRESHOLD + WLAN_DEFAULT_NOISE_FLOOR)) \
		x = WLAN_LPF_RSSI((x), WLAN_RSSI_IN((y)), WLAN_RSSI_LPF_LEN); \
	} while (0)

#define WLAN_SNR_EP_MULTIPLIER BIT(7) /* pow2 to optimize out * and / */
#define WLAN_SNR_DUMMY_MARKER  0x127
#define SNR_LPF_THRESHOLD      0
#define WLAN_SNR_LPF_LEN       10

#define WLAN_SNR_OUT(x) (((x) != WLAN_SNR_DUMMY_MARKER) ?     \
	(WLAN_EP_RND((x), WLAN_SNR_EP_MULTIPLIER)) :  WLAN_SNR_DUMMY_MARKER)

#define WLAN_SNR_IN(x)         (WLAN_EP_MUL((x), WLAN_SNR_EP_MULTIPLIER))

#define WLAN_LPF_SNR(x, y, len) \
	((x != WLAN_SNR_DUMMY_MARKER) ? ((((x) << 3) + (y) - (x)) >> 3) : (y))

#define WLAN_SNR_LPF(x, y) do { \
	if ((y) > SNR_LPF_THRESHOLD) \
		x = WLAN_LPF_SNR((x), WLAN_SNR_IN((y)), WLAN_SNR_LPF_LEN); \
	} while (0)

/**
 * util_scan_entry_rssi() - function to read rssi of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read rssi value of scan entry
 *
 * Return: rssi
 */
static inline int32_t
util_scan_entry_rssi(struct scan_cache_entry *scan_entry)
{
	return WLAN_RSSI_OUT(scan_entry->avg_rssi);
}

/**
 * util_scan_entry_snr() - function to read snr of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read snr value of scan entry
 *
 * Return: snr
 */
static inline uint8_t
util_scan_entry_snr(struct scan_cache_entry *scan_entry)
{
	uint32_t snr = WLAN_SNR_OUT(scan_entry->avg_snr);
	/*
	 * An entry is in the BSS list means we've received at least one beacon
	 * from the corresponding AP, so the snr must be initialized.
	 *
	 * If the SNR is not initialized, return 0 (i.e. SNR == Noise Floor).
	 * Once se_avgsnr field has been initialized, ATH_SNR_OUT always
	 * returns values that fit in an 8-bit variable.
	 */
	return (snr >= WLAN_SNR_DUMMY_MARKER) ? 0 : (uint8_t)snr;
}

/**
 * util_scan_entry_phymode() - function to read phymode of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read phymode of scan entry
 *
 * Return: phymode
 */
static inline enum wlan_phymode
util_scan_entry_phymode(struct scan_cache_entry *scan_entry)
{
	return scan_entry->phy_mode;
}

/**
 * util_is_ssid_match() - to check if ssid match
 * @ssid1: ssid 1
 * @ssid2: ssid 2
 *
 * Return: true if ssid match
 */
static inline bool
util_is_ssid_match(struct wlan_ssid *ssid1,
		struct wlan_ssid *ssid2)
{
	if (ssid1->length != ssid2->length)
		return false;

	if (!qdf_mem_cmp(ssid1->ssid,
	   ssid2->ssid, ssid1->length))
		return true;

	return false;
}

/**
 * util_is_bssid_match() - to check if bssid match
 * @bssid1: bssid 1
 * @bssid2: bssid 2
 *
 * Return: true if bssid match
 */
static inline bool util_is_bssid_match(struct qdf_mac_addr *bssid1,
	struct qdf_mac_addr *bssid2)
{

	if (qdf_is_macaddr_zero(bssid1) ||
	   qdf_is_macaddr_broadcast(bssid1))
		return true;

	if (qdf_is_macaddr_equal(bssid1, bssid2))
		return true;

	return false;
}

/**
 * util_is_bss_type_match() - to check if bss type
 * @bss_type: bss type
 * @cap: capability
 *
 * Return: true if bss type match
 */
static inline bool util_is_bss_type_match(enum wlan_bss_type bss_type,
	union wlan_capability cap)
{
	bool match = true;

	switch (bss_type) {
	case WLAN_TYPE_ANY:
		break;
	case WLAN_TYPE_IBSS:
		if (!cap.wlan_caps.ibss)
			match = false;
		break;
	case WLAN_TYPE_BSS:
		if (!cap.wlan_caps.ess)
			match = false;
		break;
	default:
		match = false;
	}

	return match;
}

/**
 * util_country_code_match() - to check if country match
 * @country: country code pointer
 * @country_ie: country IE in beacon
 *
 * Return: true if country match
 */
static inline bool util_country_code_match(uint8_t *country,
					   struct wlan_country_ie *cc)
{
	if (!country || !country[0])
		return true;

	if (!cc)
		return false;

	if (cc->cc[0] == country[0] &&
	    cc->cc[1] == country[1])
		return true;

	return false;
}

/**
 * util_mdie_match() - to check if mdie match
 * @mobility_domain: mobility domain
 * @mdie: mobility domain ie
 *
 * Return: true if country match
 */
static inline bool util_mdie_match(uint16_t mobility_domain,
	struct rsn_mdie *mdie)
{
	uint16_t md;

	if (!mobility_domain)
		return true;

	if (!mdie)
		return false;

	md =
	  (mdie->mobility_domain[1] << 8) |
	   mdie->mobility_domain[0];

	if (md == mobility_domain)
		return true;

	return false;
}

/**
 * util_scan_entry_ssid() - function to read ssid of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read ssid of scan entry
 *
 * Return: ssid
 */
static inline struct wlan_ssid*
util_scan_entry_ssid(struct scan_cache_entry *scan_entry)
{
	return &(scan_entry->ssid);
}

/**
 * util_scan_entry_dtimperiod() - function to read dtim period of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read dtim period of scan entry
 *
 * Return: dtim period
 */
static inline uint8_t
util_scan_entry_dtimperiod(struct scan_cache_entry *scan_entry)
{
	return scan_entry->dtim_period;
}

/**
 * util_scan_entry_tim() - function to read tim ie of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read tim ie of scan entry
 *
 * Return: timie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_tim(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.tim;
}

/**
 * util_scan_entry_beacon_frame() - function to read full beacon or
 * probe resp frame
 * @scan_entry: scan entry
 *
 * API, function to read full beacon or probe resp frame including frame header
 *
 * Return: beacon/probe resp frame
 */
static inline struct element_info
util_scan_entry_beacon_frame(struct scan_cache_entry *scan_entry)
{
	/* util_scan_entry_beacon_data */
	return scan_entry->raw_frame;
}

/**
 * util_scan_entry_ie_data() - function to read tagged IEs
 * @scan_entry: scan entry
 *
 * API, function to read beacon/probe response frames starting from tagged IEs
 * (excluding frame header and fixed parameters)
 *
 * Return: tagged IES of beacon/probe resp frame
 */
static inline uint8_t*
util_scan_entry_ie_data(struct scan_cache_entry *scan_entry)
{
	struct element_info bcn_frm;
	uint8_t *ie_data = NULL;

	bcn_frm = util_scan_entry_beacon_frame(scan_entry);
	ie_data = (uint8_t *) (bcn_frm.ptr +
		 sizeof(struct wlan_frame_hdr) +
		 offsetof(struct wlan_bcn_frame, ie));
	return ie_data;
}

/**
 * util_scan_entry_ie_len() - function to read length of all tagged IEs
 * @scan_entry: scan entry
 *
 * API, function to read length of all tagged IEs
 *
 * Return: length of all tagged IEs
 */
static inline uint16_t
util_scan_entry_ie_len(struct scan_cache_entry *scan_entry)
{
	struct element_info bcn_frm;
	uint16_t ie_len = 0;

	bcn_frm = util_scan_entry_beacon_frame(scan_entry);
	ie_len = (uint16_t) (bcn_frm.len -
		sizeof(struct wlan_frame_hdr) -
		offsetof(struct wlan_bcn_frame, ie));
	return ie_len;
}

/**
 * util_scan_entry_frame_len() - function to frame length
 * @scan_entry: scan entry
 *
 * API, function to read frame length
 *
 * Return: frame length
 */
static inline uint32_t
util_scan_entry_frame_len(struct scan_cache_entry *scan_entry)
{
	return scan_entry->raw_frame.len;
}

/**
 * util_scan_entry_frame_ptr() - function to get frame ptr
 * @scan_entry: scan entry
 *
 * API, function to read frame ptr
 *
 * Return: frame ptr
 */
static inline uint8_t*
util_scan_entry_frame_ptr(struct scan_cache_entry *scan_entry)
{
	return scan_entry->raw_frame.ptr;
}

/**
 * util_scan_entry_copy_ie_data() - function to get a copy of all tagged IEs
 * @scan_entry: scan entry
 *
 * API, function to get a copy of all tagged IEs in passed memory
 *
 * Return: QDF_STATUS_SUCCESS if tagged IEs copied successfully
 *         QDF_STATUS_E_NOMEM if passed memory/length can't hold all tagged IEs
 */
static inline QDF_STATUS
util_scan_entry_copy_ie_data(struct scan_cache_entry *scan_entry,
	uint8_t *iebuf, uint16_t *ie_len)
{
	u_int8_t     *buff;
	u_int16_t    buff_len;

	/* iebuf can be NULL, ie_len must be a valid pointer. */
	QDF_ASSERT(ie_len);
	if (!ie_len)
		return QDF_STATUS_E_NULL_VALUE;

	buff = util_scan_entry_ie_data(scan_entry);
	buff_len = util_scan_entry_ie_len(scan_entry);
	/*
	 * If caller passed a buffer, check the length to make sure
	 * it's large enough.
	 * If no buffer is passed, just return the length of the IE blob.
	 */
	if (iebuf) {
		if (*ie_len >= buff_len) {
			qdf_mem_copy(iebuf, buff, buff_len);
			*ie_len = buff_len;
			return QDF_STATUS_SUCCESS;
		}
	}

	*ie_len = buff_len;
	return QDF_STATUS_E_NOMEM;
}

/**
 * util_scan_free_cache_entry() - function to free scan
 * cache entry
 * @scan_entry: scan entry
 *
 * API, function to free scan cache entry
 *
 * Return: void
 */
static inline void
util_scan_free_cache_entry(struct scan_cache_entry *scan_entry)
{
	if (!scan_entry)
		return;
	if (scan_entry->alt_wcn_ie.ptr)
		qdf_mem_free(scan_entry->alt_wcn_ie.ptr);
	if (scan_entry->raw_frame.ptr)
		qdf_mem_free(scan_entry->raw_frame.ptr);
	qdf_mem_free(scan_entry);
}

#define conv_ptr(_address, _base1, _base2) \
	((_address) ? (((u_int8_t *) (_address) - \
	(u_int8_t *) (_base1)) + (u_int8_t *) (_base2)) : NULL)

/**
 * util_scan_copy_beacon_data() - copy beacon and update ie ptrs
 * cache entry
 * @new_entry: new scan entry
 * @scan_entry: entry from where data is copied
 *
 * API, function to copy beacon and update ie ptrs
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
util_scan_copy_beacon_data(struct scan_cache_entry *new_entry,
	struct scan_cache_entry *scan_entry)
{
	u_int8_t *new_ptr, *old_ptr;
	struct ie_list *ie_lst;

	new_entry->raw_frame.ptr =
		qdf_mem_malloc_atomic(scan_entry->raw_frame.len);
	if (!new_entry->raw_frame.ptr)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(new_entry->raw_frame.ptr,
		scan_entry->raw_frame.ptr,
		scan_entry->raw_frame.len);
	new_entry->raw_frame.len = scan_entry->raw_frame.len;
	new_ptr = new_entry->raw_frame.ptr;
	old_ptr = scan_entry->raw_frame.ptr;

	new_entry->ie_list = scan_entry->ie_list;

	ie_lst = &new_entry->ie_list;

	/* New info_element needs also be added in ieee80211_parse_beacon */
	ie_lst->tim = conv_ptr(ie_lst->tim, old_ptr, new_ptr);
	ie_lst->country = conv_ptr(ie_lst->country, old_ptr, new_ptr);
	ie_lst->ssid = conv_ptr(ie_lst->ssid, old_ptr, new_ptr);
	ie_lst->rates = conv_ptr(ie_lst->rates, old_ptr, new_ptr);
	ie_lst->xrates = conv_ptr(ie_lst->xrates, old_ptr, new_ptr);
	ie_lst->ds_param = conv_ptr(ie_lst->ds_param, old_ptr, new_ptr);
	ie_lst->csa = conv_ptr(ie_lst->csa, old_ptr, new_ptr);
	ie_lst->xcsa = conv_ptr(ie_lst->xcsa, old_ptr, new_ptr);
	ie_lst->mcst = conv_ptr(ie_lst->mcst, old_ptr, new_ptr);
	ie_lst->secchanoff = conv_ptr(ie_lst->secchanoff, old_ptr, new_ptr);
	ie_lst->wpa = conv_ptr(ie_lst->wpa, old_ptr, new_ptr);
	ie_lst->wcn = conv_ptr(ie_lst->wcn, old_ptr, new_ptr);
	ie_lst->rsn = conv_ptr(ie_lst->rsn, old_ptr, new_ptr);
	ie_lst->wps = conv_ptr(ie_lst->wps, old_ptr, new_ptr);
	ie_lst->wmeinfo = conv_ptr(ie_lst->wmeinfo, old_ptr, new_ptr);
	ie_lst->wmeparam = conv_ptr(ie_lst->wmeparam, old_ptr, new_ptr);
	ie_lst->quiet = conv_ptr(ie_lst->quiet, old_ptr, new_ptr);
	ie_lst->htcap = conv_ptr(ie_lst->htcap, old_ptr, new_ptr);
	ie_lst->htinfo = conv_ptr(ie_lst->htinfo, old_ptr, new_ptr);
	ie_lst->athcaps = conv_ptr(ie_lst->athcaps, old_ptr, new_ptr);
	ie_lst->athextcaps = conv_ptr(ie_lst->athextcaps, old_ptr, new_ptr);
	ie_lst->sfa = conv_ptr(ie_lst->sfa, old_ptr, new_ptr);
	ie_lst->vendor = conv_ptr(ie_lst->vendor, old_ptr, new_ptr);
	ie_lst->qbssload = conv_ptr(ie_lst->qbssload, old_ptr, new_ptr);
	ie_lst->wapi = conv_ptr(ie_lst->wapi, old_ptr, new_ptr);
	ie_lst->p2p = conv_ptr(ie_lst->p2p, old_ptr, new_ptr);
	ie_lst->alt_wcn = conv_ptr(ie_lst->alt_wcn, old_ptr, new_ptr);
	ie_lst->extcaps = conv_ptr(ie_lst->extcaps, old_ptr, new_ptr);
	ie_lst->ibssdfs = conv_ptr(ie_lst->ibssdfs, old_ptr, new_ptr);
	ie_lst->sonadv = conv_ptr(ie_lst->sonadv, old_ptr, new_ptr);
	ie_lst->vhtcap = conv_ptr(ie_lst->vhtcap, old_ptr, new_ptr);
	ie_lst->vhtop = conv_ptr(ie_lst->vhtop, old_ptr, new_ptr);
	ie_lst->opmode = conv_ptr(ie_lst->opmode, old_ptr, new_ptr);
	ie_lst->cswrp = conv_ptr(ie_lst->cswrp, old_ptr, new_ptr);
	ie_lst->widebw = conv_ptr(ie_lst->widebw, old_ptr, new_ptr);
	ie_lst->txpwrenvlp = conv_ptr(ie_lst->txpwrenvlp, old_ptr, new_ptr);
	ie_lst->bwnss_map = conv_ptr(ie_lst->bwnss_map, old_ptr, new_ptr);
	ie_lst->mdie = conv_ptr(ie_lst->mdie, old_ptr, new_ptr);
	ie_lst->hecap = conv_ptr(ie_lst->hecap, old_ptr, new_ptr);
	ie_lst->heop = conv_ptr(ie_lst->heop, old_ptr, new_ptr);
	ie_lst->fils_indication = conv_ptr(ie_lst->fils_indication,
					   old_ptr, new_ptr);
	ie_lst->esp = conv_ptr(ie_lst->esp, old_ptr, new_ptr);
	ie_lst->mbo_oce = conv_ptr(ie_lst->mbo_oce, old_ptr, new_ptr);
	ie_lst->muedca = conv_ptr(ie_lst->muedca, old_ptr, new_ptr);
	ie_lst->rnrie = conv_ptr(ie_lst->rnrie, old_ptr, new_ptr);
	ie_lst->extender = conv_ptr(ie_lst->extender, old_ptr, new_ptr);
	ie_lst->adaptive_11r = conv_ptr(ie_lst->adaptive_11r, old_ptr, new_ptr);
	ie_lst->single_pmk = conv_ptr(ie_lst->single_pmk, old_ptr, new_ptr);

	return QDF_STATUS_SUCCESS;
}
/**
 * util_scan_copy_cache_entry() - function to create a copy
 * of scan cache entry
 * @scan_entry: scan entry
 *
 * API, function to create a copy of scan cache entry
 *
 * Return: copy of scan_entry
 */
static inline struct scan_cache_entry *
util_scan_copy_cache_entry(struct scan_cache_entry *scan_entry)
{
	struct scan_cache_entry *new_entry;
	QDF_STATUS status;

	if (!scan_entry)
		return NULL;

	new_entry =
	   qdf_mem_malloc_atomic(sizeof(*scan_entry));
	if (!new_entry)
		return NULL;

	qdf_mem_copy(new_entry,
		scan_entry, sizeof(*scan_entry));

	if (scan_entry->alt_wcn_ie.ptr) {
		new_entry->alt_wcn_ie.ptr =
		    qdf_mem_malloc_atomic(scan_entry->alt_wcn_ie.len);
		if (!new_entry->alt_wcn_ie.ptr) {
			qdf_mem_free(new_entry);
			return NULL;
		}
		qdf_mem_copy(new_entry->alt_wcn_ie.ptr,
		   scan_entry->alt_wcn_ie.ptr,
		   scan_entry->alt_wcn_ie.len);
		new_entry->alt_wcn_ie.len =
			scan_entry->alt_wcn_ie.len;
	}

	status = util_scan_copy_beacon_data(new_entry, scan_entry);
	if (QDF_IS_STATUS_ERROR(status)) {
		util_scan_free_cache_entry(new_entry);
		return NULL;
	}

	return new_entry;
}

/**
 * util_scan_entry_channel() - function to read channel info
 * @scan_entry: scan entry
 *
 * API, function to read channel info
 *
 * Return: channel info
 */
static inline struct channel_info*
util_scan_entry_channel(struct scan_cache_entry *scan_entry)
{
	return &(scan_entry->channel);
}

/**
 * util_scan_entry_channel_frequency() - function to read channel number
 * @scan_entry: scan entry
 *
 * API, function to read channel number
 *
 * Return: channel number
 */
static inline uint32_t
util_scan_entry_channel_frequency(struct scan_cache_entry *scan_entry)
{
	return scan_entry->channel.chan_freq;
}

/**
 * util_scan_entry_erpinfo() - function to read erp info
 * @scan_entry: scan entry
 *
 * API, function to read erp info
 *
 * Return: erp info
 */
static inline uint8_t
util_scan_entry_erpinfo(struct scan_cache_entry *scan_entry)
{
	return scan_entry->erp;
}

/**
 * util_scan_entry_rates() - function to read supported rates IE
 * @scan_entry: scan entry
 *
 * API, function to read supported rates IE
 *
 * Return: basic ratesie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_rates(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.rates;
}

/**
 * util_scan_entry_xrates()- function to read extended supported rates IE
 * @scan_entry: scan entry
 *
 * API, function to read extended supported rates IE
 *
 * Return: extended supported ratesie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_xrates(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.xrates;
}

/**
 * util_scan_entry_rsn()- function to read rsn IE
 * @scan_entry: scan entry
 *
 * API, function to read rsn IE
 *
 * Return: rsnie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_rsn(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.rsn;
}

/**
 * util_scan_entry_adaptive_11r()- function to read adaptive 11r Vendor IE
 * @scan_entry: scan entry
 *
 * API, function to read adaptive 11r IE
 *
 * Return:  apaptive 11r ie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_adaptive_11r(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.adaptive_11r;
}

/**
 * util_scan_entry_single_pmk()- function to read single pmk Vendor IE
 * @scan_entry: scan entry
 *
 * API, function to read sae single pmk IE
 *
 * Return: true if single_pmk ie is present or false if ie is not present
 */
static inline bool
util_scan_entry_single_pmk(struct scan_cache_entry *scan_entry)
{
	if (scan_entry->ie_list.single_pmk)
		return true;

	return false;
}

/**
 * util_scan_get_rsn_len()- function to read rsn IE length if present
 * @scan_entry: scan entry
 *
 * API, function to read rsn length if present
 *
 * Return: rsnie length
 */
static inline uint8_t
util_scan_get_rsn_len(struct scan_cache_entry *scan_entry)
{
	if (scan_entry && scan_entry->ie_list.rsn)
		return scan_entry->ie_list.rsn[1] + 2;
	else
		return 0;
}


/**
 * util_scan_entry_wpa() - function to read wpa IE
 * @scan_entry: scan entry
 *
 * API, function to read wpa IE
 *
 * Return: wpaie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_wpa(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.wpa;
}

/**
 * util_scan_get_wpa_len()- function to read wpa IE length if present
 * @scan_entry: scan entry
 *
 * API, function to read wpa ie length if present
 *
 * Return: wpa ie length
 */
static inline uint8_t
util_scan_get_wpa_len(struct scan_cache_entry *scan_entry)
{
	if (scan_entry && scan_entry->ie_list.wpa)
		return scan_entry->ie_list.wpa[1] + 2;
	else
		return 0;
}


/**
 * util_scan_entry_wapi() - function to read wapi IE
 * @scan_entry: scan entry
 *
 * API, function to read wapi IE
 *
 * Return: wapiie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_wapi(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.wapi;
}

/**
 * util_scan_entry_wps() - function to read wps IE
 * @scan_entry: scan entry
 *
 * API, function to read wps IE
 *
 * Return: wpsie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_wps(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.wps;
}

/**
 * util_scan_entry_sfa() - function to read sfa IE
 * @scan_entry: scan entry
 *
 * API, function to read sfa IE
 *
 * Return: sfaie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_sfa(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.sfa;
}

/**
 * util_scan_entry_ds_param() - function to read ds params
 * @scan_entry: scan entry
 *
 * API, function to read ds params
 *
 * Return: ds params or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_ds_param(struct scan_cache_entry *scan_entry)
{
	if (scan_entry)
		return scan_entry->ie_list.ds_param;
	else
		return NULL;
}

/**
 * util_scan_entry_csa() - function to read csa IE
 * @scan_entry: scan entry
 *
 * API, function to read csa IE
 *
 * Return: csaie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_csa(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.csa;
}

/**
 * util_scan_entry_xcsa() - function to read extended csa IE
 * @scan_entry: scan entry
 *
 * API, function to read extended csa IE
 *
 * Return: extended csaie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_xcsa(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.xcsa;
}

/**
 * util_scan_entry_htinfo() - function to read htinfo IE
 * @scan_entry: scan entry
 *
 * API, function to read htinfo IE
 *
 * Return: htinfoie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_htinfo(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.htinfo;
}


/**
 * util_scan_entry_htcap() - function to read htcap IE
 * @scan_entry: scan entry
 *
 * API, function to read htcap IE
 *
 * Return: htcapie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_htcap(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.htcap;
}

/**
 * util_scan_entry_vhtcap() - function to read vhtcap IE
 * @scan_entry: scan entry
 *
 * API, function to read vhtcap IE
 *
 * Return: vhtcapie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_vhtcap(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.vhtcap;
}

/**
 * util_scan_entry_vhtop() - function to read vhtop IE
 * @scan_entry: scan entry
 *
 * API, function to read vhtop IE
 *
 * Return: vhtopie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_vhtop(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.vhtop;
}

/**
 * util_scan_entry_quiet() - function to read quiet IE
 * @scan_entry: scan entry
 *
 * API, function to read quiet IE
 *
 * Return: quietie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_quiet(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.quiet;
}

/**
 * util_scan_entry_qbssload() - function to read qbss load IE
 * @scan_entry: scan entry
 *
 * API, function to read qbss load IE
 *
 * Return: qbss loadie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_qbssload(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.qbssload;
}

/**
 * util_scan_entry_vendor() - function to read vendor IE
 * @scan_entry: scan entry
 *
 * API, function to read vendor IE
 *
 * Return: vendorie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_vendor(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.vendor;
}

/**
 * util_scan_entry_country() - function to read country IE
 * @scan_entry: scan entry
 *
 * API, function to read country IE
 *
 * Return: countryie or NULL if ie is not present
 */
static inline struct wlan_country_ie*
util_scan_entry_country(struct scan_cache_entry *scan_entry)
{
	return (struct wlan_country_ie *)scan_entry->ie_list.country;
}

/**
 * util_scan_entry_copy_country() - function to copy country name
 * @scan_entry: scan entry
 * @cntry:      out buffer
 *
 * API, function to copy country name code string in given memory @centry
 *
 * Return: QDF_STATUS_SUCCESS if successfully copied country name
 *         QDF_STATUS_E_INVAL if passed buffer is null
 *         QDF_STATUS_E_NOMEM if scan entry dont have country IE
 */
static inline QDF_STATUS
util_scan_entry_copy_country(struct scan_cache_entry *scan_entry,
	uint8_t *cntry)
{
	struct wlan_country_ie *country_ie;

	if (!cntry)
		return QDF_STATUS_E_INVAL;

	country_ie = util_scan_entry_country(scan_entry);

	if (!country_ie)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(cntry, country_ie->cc, 3);

	return QDF_STATUS_SUCCESS;
}

/**
 * util_scan_entry_wmeinfo() - function to read wme info ie
 * @scan_entry: scan entry
 *
 * API, function to read wme info ie
 *
 * Return: wme infoie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_wmeinfo(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.wmeinfo;
}

/**
 * util_scan_entry_wmeparam() - function to read wme param ie
 * @scan_entry: scan entry
 *
 * API, function to read wme param ie
 *
 * Return: wme paramie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_wmeparam(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.wmeparam;
}

/**
 * util_scan_entry_age() - function to read age of scan entry
 * @scan_entry: scan entry
 *
 * API, function to read age of scan entry
 *
 * Return: age in ms
 */
static inline qdf_time_t
util_scan_entry_age(struct scan_cache_entry *scan_entry)
{
	qdf_time_t ts = scan_entry->scan_entry_time;

	return qdf_mc_timer_get_system_time() - ts;
}

/**
 * util_scan_mlme_info() - function to read mlme info struct
 * @scan_entry: scan entry
 *
 * API, function to read mlme info struct
 *
 * Return: mlme info
 */
static inline struct mlme_info*
util_scan_mlme_info(struct scan_cache_entry *scan_entry)
{
	return &scan_entry->mlme_info;
}

/**
 * util_scan_entry_bss_type() - function to read bss type
 * @scan_entry: scan entry
 *
 * API, function to read bss type
 *
 * Return: bss type
 */
static inline enum wlan_bss_type
util_scan_entry_bss_type(struct scan_cache_entry *scan_entry)
{
	if (scan_entry->cap_info.value & WLAN_CAPINFO_ESS)
		return WLAN_TYPE_BSS;
	else if (scan_entry->cap_info.value & WLAN_CAPINFO_IBSS)
		return WLAN_TYPE_IBSS;
	else
		return WLAN_TYPE_ANY;
}

/**
 * util_scan_entry_privacy() - function to check if privacy is enebled
 * @scan_entry: scan entry
 *
 * API, function to check if privacy is enebled
 *
 * Return: true if privacy is enabled, false other wise
 */
static inline bool
util_scan_entry_privacy(struct scan_cache_entry *scan_entry)
{
	return (scan_entry->cap_info.value &
		WLAN_CAPINFO_PRIVACY) ? true : false;
}

/**
 * util_scan_entry_athcaps() - function to read ath caps vendor ie
 * @scan_entry: scan entry
 *
 * API, function to read ath caps vendor ie
 *
 * Return: ath caps vendorie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_athcaps(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.athcaps;
}

/**
 * util_scan_entry_athextcaps() - function to read ath extcaps vendor ie
 * @scan_entry: scan entry
 *
 * API, function to read ath extcaps vendor ie
 *
 * Return: ath extcaps vendorie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_athextcaps(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.athextcaps;
}

/**
 * util_scan_entry_bwnss_map() - function to read bwnss_map ie
 * @scan_entry: scan entry
 *
 * API, function to read bwnss_map ie
 *
 * Return: bwnss_map ie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_bwnss_map(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.bwnss_map;
}

/**
 * util_scan_entry_sonie() - function to read son ie
 * @scan_entry: scan entry
 *
 * API, function to read son ie
 *
 * Return: son ie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_sonie(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.sonadv;
}

/**
 * util_scan_entry_widebw() - function to read wide band chan switch sub elem ie
 * @scan_entry: scan entry
 *
 * API, function to read wide band chan switch sub elem ie
 *
 * Return: wide band chan switch sub elem or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_widebw(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.widebw;
}

/**
 * util_scan_entry_secchanoff() - function to read secondary channel offset ie
 * @scan_entry: scan entry
 *
 * API, function to read secondary channel offset ie
 *
 * Return: secondary channel offset element or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_secchanoff(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.secchanoff;
}

/**
 * util_scan_entry_cswrp() - function to read channel switch wrapper ie
 * @scan_entry: scan entry
 *
 * API, function to read channel switch wrapper ie
 *
 * Return: channel switch wrapper element or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_cswrp(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.cswrp;
}

/**
 * util_scan_entry_omn() - function to read operating mode notification ie
 * @scan_entry: scan entry
 *
 * API, function to read operating mode notification
 *
 * Return: operating mode notification element or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_omn(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.opmode;
}

/**
 * util_scan_entry_extcaps() - function to read extcap ie
 * @scan_entry: scan entry
 *
 * API, function to read extcap ie
 *
 * Return: extcap element or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_extcaps(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.extcaps;
}

/**
 * util_scan_entry_get_extcap() - function to read extended capability field ie
 * @scan_entry: scan entry
 * @extcap_bit_field: extended capability bit field
 * @extcap_value: pointer to fill extended capability field value
 *
 * API, function to read extended capability field
 *
 * Return: QDF_STATUS_SUCCESS if extended capability field is found
 *         QDF_STATUS_E_NOMEM if extended capability field is not found
 */
static inline QDF_STATUS
util_scan_entry_get_extcap(struct scan_cache_entry *scan_entry,
			   enum ext_cap_bit_field extcap_bit_field,
			   uint8_t *extcap_value)
{
	struct wlan_ext_cap_ie *ext_cap =
		(struct wlan_ext_cap_ie *)util_scan_entry_extcaps(scan_entry);

	uint8_t ext_caps_byte = (extcap_bit_field >> 3);
	uint8_t ext_caps_bit_pos = extcap_bit_field & 0x7;

	*extcap_value = 0;

	if (!ext_cap)
		return QDF_STATUS_E_NULL_VALUE;

	if (ext_cap->ext_cap_len < ext_caps_byte)
		return QDF_STATUS_E_NULL_VALUE;

	*extcap_value =
		((ext_cap->ext_caps[ext_caps_byte] >> ext_caps_bit_pos) & 0x1);

	return QDF_STATUS_SUCCESS;
}

/**
 * util_scan_entry_athcaps() - function to read ath caps vendor ie
 * @scan_entry: scan entry
 *
 * API, function to read ath caps vendor ie
 *
 * Return: ath caps vendorie or NULL if ie is not present
 */
static inline struct mlme_info*
util_scan_entry_mlme_info(struct scan_cache_entry *scan_entry)
{
	return &(scan_entry->mlme_info);
}

/**
* util_scan_entry_mcst() - function to read mcst IE
* @scan_entry:scan entry
*
* API, function to read mcst IE
*
* Return: mcst or NULL if ie is not present
*/
static inline uint8_t*
util_scan_entry_mcst(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.mcst;
}

/**
 * util_scan_entry_hecap() - function to read he caps vendor ie
 * @scan_entry: scan entry
 *
 * API, function to read he caps vendor ie
 *
 * Return: he caps vendorie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_hecap(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.hecap;
}

/**
 * util_scan_entry_he_6g_cap() - function to read  he 6GHz caps vendor ie
 * @scan_entry: scan entry
 *
 * API, function to read he 6GHz caps vendor ie
 *
 * Return: he caps vendorie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_he_6g_cap(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.hecap_6g;
}

/**
 * util_scan_entry_heop() - function to read heop vendor ie
 * @scan_entry: scan entry
 *
 * API, function to read heop vendor ie
 *
 * Return, heop vendorie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_heop(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.heop;
}

/**
 * util_scan_entry_muedca() - function to read MU-EDCA IE
 * @scan_entry: scan entry
 *
 * API, function to read MU-EDCA IE
 *
 * Return, MUEDCA IE or NULL if IE is not present
 */
static inline uint8_t*
util_scan_entry_muedca(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.muedca;
}

/**
 * util_scan_entry_spatial_reuse_parameter() - function to read spatial reuse
 *                                             parameter ie
 * @scan_entry: scan entry
 *
 * API, function to read scan_entry reuse parameter ie
 *
 * Return, spatial reuse parameter ie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_spatial_reuse_parameter(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.srp;
}

/**
 * util_scan_entry_fils_indication() - function to read FILS indication ie
 * @scan_entry: scan entry
 *
 * API, function to read FILS indication ie
 *
 * Return, FILS indication ie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_fils_indication(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.fils_indication;
}

/**
 * util_get_last_scan_time() - function to get last scan time on this pdev
 * @vdev: vdev object
 *
 * API, function to read last scan time on this pdev
 *
 * Return: qdf_time_t
 */
qdf_time_t
util_get_last_scan_time(struct wlan_objmgr_vdev *vdev);

/**
 * util_scan_entry_update_mlme_info() - function to update mlme info
 * @scan_entry: scan entry object
 *
 * API, function to update mlme info in scan DB
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
util_scan_entry_update_mlme_info(struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *scan_entry);

/**
 * util_scan_is_hidden_ssid() - function to check if ssid is hidden
 * @ssid: struct ie_ssid object
 *
 * API, function to check if ssid is hidden
 *
 * Return: true if ap is hidden, false otherwise
 */
bool
util_scan_is_hidden_ssid(struct ie_ssid *ssid);

/**
 * util_scan_entry_is_hidden_ap() - function to check if ap is hidden
 * @scan_entry: scan entry
 *
 * API, function to check if ap is hidden
 *
 * Return: true if ap is hidden, false otherwise
 */
static inline bool
util_scan_entry_is_hidden_ap(struct scan_cache_entry *scan_entry)
{
    return util_scan_is_hidden_ssid(
			(struct ie_ssid *)scan_entry->ie_list.ssid);
}

/**
 * util_scan_entry_espinfo() - function to read ESP info
 * @scan_entry: scan entry
 *
 * API, function to read ESP info
 *
 * Return: erp info
 */
static inline uint8_t *
util_scan_entry_esp_info(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.esp;
}

/**
 * util_scan_entry_mbo_oce() - function to read MBO/OCE ie
 * @scan_entry: scan entry
 *
 * API, function to read MBO/OCE ie
 *
 * Return: MBO/OCE ie
 */
static inline uint8_t *
util_scan_entry_mbo_oce(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.mbo_oce;
}

/**
 * util_scan_scm_chan_to_band() - function to tell band for channel number
 * @chan: Channel number
 *
 * Return: Band information as per channel
 */
enum wlan_band util_scan_scm_chan_to_band(uint32_t chan);

/**
 * util_scan_scm_freq_to_band() - API to get band from frequency
 * @freq: Channel frequency
 *
 * Return: Band information as per frequency
 */
enum wlan_band util_scan_scm_freq_to_band(uint16_t freq);

/**
 * util_is_scan_completed() - function to get scan complete status
 * @event: scan event
 * @success: true if scan complete success, false otherwise
 *
 * API, function to get the scan result
 *
 * Return: true if scan complete, false otherwise
 */
bool util_is_scan_completed(struct scan_event *event, bool *success);

/**
 * util_scan_entry_extenderie() - function to read extender IE
 * @scan_entry: scan entry
 *
 * API, function to read extender IE
 *
 * Return: extenderie or NULL if ie is not present
 */
static inline uint8_t*
util_scan_entry_extenderie(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.extender;
}

/**
 * util_scan_entry_mdie() - function to read Mobility Domain IE
 * @scan_entry: scan entry
 *
 * API, function to read Mobility Domain IE
 *
 * Return: MDIE or NULL if IE is not present
 */
static inline uint8_t*
util_scan_entry_mdie(struct scan_cache_entry *scan_entry)
{
	return scan_entry->ie_list.mdie;
}

/**
 * util_scan_is_null_ssid() - to check for NULL ssid
 * @ssid: ssid
 *
 * Return: true if NULL ssid else false
 */
static inline bool util_scan_is_null_ssid(struct wlan_ssid *ssid)
{
	uint32_t ssid_length;
	uint8_t *ssid_str;

	if (ssid->length == 0)
		return true;

	/* Consider 0 or space for hidden SSID */
	if (0 == ssid->ssid[0])
		return true;

	ssid_length = ssid->length;
	ssid_str = ssid->ssid;

	while (ssid_length) {
		if (*ssid_str != ASCII_SPACE_CHARACTER &&
		    *ssid_str)
			break;
		ssid_str++;
		ssid_length--;
	}

	if (ssid_length == 0)
		return true;

	return false;
}

#endif
