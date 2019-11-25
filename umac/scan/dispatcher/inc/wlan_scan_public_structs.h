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
 * DOC: contains scan structure definations
 */

#ifndef _WLAN_SCAN_STRUCTS_H_
#define _WLAN_SCAN_STRUCTS_H_
#include <wlan_cmn.h>
#include <qdf_time.h>
#include <qdf_list.h>
#include <qdf_atomic.h>
#include <wlan_cmn_ieee80211.h>
#include <wlan_mgmt_txrx_utils_api.h>
#include <reg_services_public_struct.h>

typedef uint16_t wlan_scan_requester;
typedef uint32_t wlan_scan_id;

#define WLAN_SCAN_MAX_HINT_S_SSID        10
#define WLAN_SCAN_MAX_HINT_BSSID         10
#define MAX_RNR_BSS                      5
#define WLAN_SCAN_MAX_NUM_SSID          16
#define WLAN_SCAN_MAX_NUM_BSSID         4

#define SCM_CANCEL_SCAN_WAIT_TIME 50
#define SCM_CANCEL_SCAN_WAIT_ITERATION 600

#define INVAL_SCAN_ID        0xFFFFFFFF
#define INVAL_VDEV_ID        0xFFFFFFFF
#define INVAL_PDEV_ID        0xFFFFFFFF

#define USER_SCAN_REQUESTOR_ID  0xA0000
#define PREAUTH_REQUESTOR_ID    0xC0000

#define BURST_SCAN_MAX_NUM_OFFCHANNELS 3
#define P2P_SCAN_MAX_BURST_DURATION 180
/* Increase dwell time for P2P search in ms */
#define P2P_SEARCH_DWELL_TIME_INC 20

#define PROBE_REQ_BITMAP_LEN 8
#define MAX_PROBE_REQ_OUIS 16

#define RSSI_WEIGHTAGE 20
#define HT_CAPABILITY_WEIGHTAGE 2
#define VHT_CAP_WEIGHTAGE 1
#define HE_CAP_WEIGHTAGE 2
#define CHAN_WIDTH_WEIGHTAGE 17
#define CHAN_BAND_WEIGHTAGE 2
#define NSS_WEIGHTAGE 16
#define BEAMFORMING_CAP_WEIGHTAGE 2
#define PCL_WEIGHT 10
#define CHANNEL_CONGESTION_WEIGHTAGE 5
#define OCE_WAN_WEIGHTAGE 0
#define BEST_CANDIDATE_MAX_WEIGHT 100
#define MAX_INDEX_SCORE 100
#define MAX_INDEX_PER_INI 4

#define WLAN_GET_BITS(_val, _index, _num_bits) \
	(((_val) >> (_index)) & ((1 << (_num_bits)) - 1))

#define WLAN_SET_BITS(_var, _index, _num_bits, _val) do { \
	(_var) &= ~(((1 << (_num_bits)) - 1) << (_index)); \
	(_var) |= (((_val) & ((1 << (_num_bits)) - 1)) << (_index)); \
	} while (0)

#define WLAN_GET_SCORE_PERCENTAGE(value32, bw_index) \
	WLAN_GET_BITS(value32, (8 * (bw_index)), 8)
#define WLAN_SET_SCORE_PERCENTAGE(value32, score_pcnt, bw_index) \
	WLAN_SET_BITS(value32, (8 * (bw_index)), 8, score_pcnt)

/* forward declaration */
struct wlan_objmgr_vdev;
struct wlan_objmgr_pdev;
struct wlan_objmgr_psoc;

/**
 * struct channel_info - BSS channel information
 * @chan_freq: channel frequency
 * @cfreq0: channel frequency index0
 * @cfreq1: channel frequency index1
 * @priv: channel private information
 */
struct channel_info {
	uint32_t chan_freq;
	uint32_t cfreq0;
	uint32_t cfreq1;
	void *priv;
};

/**
 * struct element_info - defines length of a memory block and memory block
 * @len: length of memory block
 * @ptr: memory block pointer
 */
struct element_info {
	uint32_t len;
	uint8_t *ptr;
};

/**
 * struct ie_list - pointers to various IEs
 * @tim:        pointer to tim ie
 * @country:    pointer to country ie
 * @ssid:       pointer to ssid ie
 * @rates:      pointer to supported rates ie
 * @xrates:     pointer to extended supported rate ie
 * @ds_param:   pointer to ds params
 * @csa:        pointer to csa ie
 * @xcsa:       pointer to extended csa ie
 * @mcst:       pointer to maximum channel switch time ie
 * @wpa:        pointer to wpa ie
 * @wcn:        pointer to wcn ie
 * @rsn:        pointer to rsn ie
 * @wps:        pointer to wps ie
 * @wmeinfo:    pointer to wmeinfo ie
 * @wmeparam:   pointer to wmeparam ie
 * @quiet:      pointer to quiet ie
 * @htcap:      pointer to htcap ie
 * @htinfo:     pointer to htinfo ie
 * @athcaps:    pointer to athcaps ie
 * @athextcaps: pointer to extended athcaps ie
 * @sfa:        pointer to sfa ie
 * @vendor:     pointer to vendor ie
 * @qbssload:   pointer to qbssload ie
 * @wapi:       pointer to wapi ie
 * @p2p:        pointer to p2p ie
 * @alt_wcn:    pointer to alternate wcn ie
 * @extcaps:    pointer to extended caps ie
 * @ibssdfs:    pointer to ibssdfs ie
 * @sonadv:     pointer to wifi son ie
 * @vhtcap:     pointer to vhtcap ie
 * @vhtop:      pointer to vhtop ie
 * @opmode:     pointer to opmode ie
 * @cswrp:      pointer to channel switch announcement wrapper ie
 * @widebw:     pointer to wide band channel switch sub ie
 * @txpwrenvlp: pointer to tx power envelop sub ie
 * @hecap:      pointer to hecap ie
 * @hecap_6g:   pointer to he 6ghz cap ie
 * @srp: pointer to spatial reuse parameter sub extended ie
 * @fils_indication: pointer to FILS indication ie
 * @esp: pointer to ESP indication ie
 * @mbo_oce: pointer to mbo/oce indication ie
 * @rnrie: reduced neighbor report IE
 * @adaptive_11r: pointer to adaptive 11r IE
 */
struct ie_list {
	uint8_t *tim;
	uint8_t *country;
	uint8_t *ssid;
	uint8_t *rates;
	uint8_t *xrates;
	uint8_t *ds_param;
	uint8_t *csa;
	uint8_t *xcsa;
	uint8_t *mcst;
	uint8_t *wpa;
	uint8_t *wcn;
	uint8_t *rsn;
	uint8_t *wps;
	uint8_t *wmeinfo;
	uint8_t *wmeparam;
	uint8_t *quiet;
	uint8_t *htcap;
	uint8_t *htinfo;
	uint8_t *athcaps;
	uint8_t *athextcaps;
	uint8_t *sfa;
	uint8_t *vendor;
	uint8_t *qbssload;
	uint8_t *wapi;
	uint8_t *p2p;
	uint8_t *alt_wcn;
	uint8_t *extcaps;
	uint8_t *ibssdfs;
	uint8_t *sonadv;
	uint8_t *vhtcap;
	uint8_t *vhtop;
	uint8_t *opmode;
	uint8_t *cswrp;
	uint8_t *widebw;
	uint8_t *txpwrenvlp;
	uint8_t *bwnss_map;
	uint8_t *secchanoff;
	uint8_t *mdie;
	uint8_t *hecap;
	uint8_t *hecap_6g;
	uint8_t *heop;
	uint8_t *srp;
	uint8_t *fils_indication;
	uint8_t *esp;
	uint8_t *mbo_oce;
	uint8_t *muedca;
	uint8_t *rnrie;
	uint8_t *extender;
	uint8_t *adaptive_11r;
};

enum scan_entry_connection_state {
	SCAN_ENTRY_CON_STATE_NONE,
	SCAN_ENTRY_CON_STATE_AUTH,
	SCAN_ENTRY_CON_STATE_ASSOC
};

/**
 * struct mlme_info - mlme specific info
 * temporarily maintained in scan cache for backward compatibility.
 * must be removed as part of umac convergence.
 * @bad_ap_time: time when this ap was marked bad
 * @status: status
 * @rank: rank
 * @utility: utility
 * @assoc_state: association state
 * @chanload: channel load
 */
struct mlme_info {
	qdf_time_t bad_ap_time;
	uint32_t status;
	uint32_t rank;
	uint32_t utility;
	uint32_t assoc_state;
	uint32_t chanload;
};

/**
 * struct bss_info - information required to uniquely define a bss
 * @freq: freq of operating primary channel
 * @ssid: ssid of bss
 * @bssid: bssid of bss
 */
struct bss_info {
	uint32_t freq;
	struct wlan_ssid ssid;
	struct qdf_mac_addr bssid;
};

#define SCAN_NODE_ACTIVE_COOKIE 0x1248F842
/**
 * struct scan_cache_node - Scan cache entry node
 * @node: node pointers
 * @ref_cnt: ref count if in use
 * @cookie: cookie to check if entry is logically active
 * @entry: scan entry pointer
 */
struct scan_cache_node {
	qdf_list_node_t node;
	qdf_atomic_t ref_cnt;
	uint32_t cookie;
	struct scan_cache_entry *entry;
};

struct security_info {
	enum wlan_enc_type uc_enc;
	enum wlan_enc_type mc_enc;
	enum wlan_auth_type auth_type;
};

/**
 * struct scan_mbssid_info - Scan mbssid information
 * @profile_num: profile number
 * @profile_count: total profile count
 * @trans_bssid: TX BSSID address
 */
struct scan_mbssid_info {
	uint8_t profile_num;
	uint8_t profile_count;
	uint8_t trans_bssid[QDF_MAC_ADDR_SIZE];
};

/**
 * struct rnr_bss_info - Reduced Neighbor Report BSS information
 * @neighbor_ap_tbtt_offset: Neighbor AP TBTT offset
 * @channel_number: channel number
 * @operating_class: operting class
 * @bssid: BSS MAC address
 * @short_ssid: short ssid
 * @bss_params: BSS parameters
 */
struct rnr_bss_info {
	uint8_t neighbor_ap_tbtt_offset;
	uint32_t channel_number;
	uint32_t operating_class;
	struct qdf_mac_addr bssid;
	uint32_t short_ssid;
	uint8_t bss_params;
};

/**
 * struct tbtt_information_header - TBTT information header
 * @tbbt_info_fieldtype: TBTT information field type
 * @filter_neighbor_ap: filtered neighbor ap
 * @tbbt_info_count: TBTT information count
 * @tbtt_info_length: TBTT informaiton length
 */
struct tbtt_information_header {
	uint16_t tbbt_info_fieldtype:2;
	uint16_t filtered_neighbor_ap:1;
	uint16_t reserved:1;
	uint16_t tbtt_info_count:4;
	uint16_t tbtt_info_length:8;
};

/**
 * struct neighbor_ap_info_field - Neighbor information field
 * @tbtt_info_header: TBTT information header
 * @operting_class: operating class
 * @channel_number: channel number
 */
struct neighbor_ap_info_field {
	struct tbtt_information_header tbtt_header;
	uint8_t operting_class;
	uint8_t channel_number;
};

/**
 * enum tbtt_information_field - TBTT information field
 * @TBTT_NEIGHBOR_AP_OFFSET_ONLY: TBTT information field type
 * @TBTT_NEIGHBOR_AP_BSS_PARAM: neighbor AP and bss param
 * @TBTT_NEIGHBOR_AP_SHORTSSID: neighbor AP and Short ssid
 * @TBTT_NEIGHBOR_AP_S_SSID_BSS_PARAM: neighbor AP, short ssid and bss param
 * @TBTT_NEIGHBOR_AP_BSSID: neighbor AP and bssid
 * @TBTT_NEIGHBOR_AP_BSSID_BSS_PARAM: neighbor AP, bssid and bss param
 * @TBTT_NEIGHBOR_AP_BSSSID_S_SSID: neighbor AP, bssid and short ssid
 * @TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM: neighbor AP, bssid, short ssid
 * and bss params
 */
enum tbtt_information_field {
	TBTT_NEIGHBOR_AP_OFFSET_ONLY = 1,
	TBTT_NEIGHBOR_AP_BSS_PARAM = 2,
	TBTT_NEIGHBOR_AP_SHORTSSID = 5,
	TBTT_NEIGHBOR_AP_S_SSID_BSS_PARAM = 6,
	TBTT_NEIGHBOR_AP_BSSID = 7,
	TBTT_NEIGHBOR_AP_BSSID_BSS_PARAM = 8,
	TBTT_NEIGHBOR_AP_BSSSID_S_SSID = 11,
	TBTT_NEIGHBOR_AP_BSSID_S_SSID_BSS_PARAM = 12
};

/**
 * struct reduced_neighbor_report - Reduced Neighbor Report
 * @bss_info: RNR BSS Information
 */
struct reduced_neighbor_report {
	struct rnr_bss_info bss_info[MAX_RNR_BSS];
};

/**
 * struct scan_cache_entry: structure containing scan entry
 * @frm_subtype: updated from beacon/probe
 * @bssid: bssid
 * @mac_addr: mac address
 * @ssid: ssid
 * @is_hidden_ssid: is AP having hidden ssid.
 * @seq_num: sequence number
 * @phy_mode: Phy mode of the AP
 * @avg_rssi: Average RSSI of the AP
 * @rssi_raw: The rssi of the last beacon/probe received
 * @snr: The snr of the last beacon/probe received
 * @avg_snr: Average SNR of the AP
 * @bcn_int: Beacon interval of the AP
 * @cap_info: Capability of the AP
 * @tsf_info: TSF info
 * @erp: erp info
 * @dtim_period: dtime period
 * @air_time_fraction: Air time fraction from ESP param
 * @qbss_chan_load: Qbss channel load
 * @nss: supported NSS information
 * @is_p2p_ssid: is P2P entry
 * @adaptive_11r_ap: flag to check if AP supports adaptive 11r
 * @scan_entry_time: boottime in microsec when last beacon/probe is received
 * @rssi_timestamp: boottime in microsec when RSSI was updated
 * @hidden_ssid_timestamp: boottime in microsec when hidden
 *                         ssid was received
 * @mbssid_info: Multi bssid information
 * @rnr: Reduced neighbor report information
 * @channel: channel info on which AP is present
 * @channel_mismatch: if channel received in metadata
 *                    doesnot match the one in beacon
 * @tsf_delta: TSF delta
 * @bss_score: bss score calculated on basis of RSSI/caps etc.
 * @neg_sec_info: negotiated security info
 * @per_chain_rssi: per chain RSSI value received.
 * boottime_ns: boottime in ns.
 * @rrm_parent_tsf: RRM parent tsf
 * @mlme_info: Mlme info, this will be updated by MLME for the scan entry
 * @alt_wcn_ie: alternate WCN IE
 * @ie_list: IE list pointers
 * @raw_frame: contain raw frame and the length of the raw frame
 * @pdev_id: pdev id
 */
struct scan_cache_entry {
	uint8_t frm_subtype;
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr mac_addr;
	struct wlan_ssid ssid;
	bool is_hidden_ssid;
	uint16_t seq_num;
	enum wlan_phymode phy_mode;
	int32_t avg_rssi;
	int8_t rssi_raw;
	uint8_t snr;
	uint32_t avg_snr;
	uint16_t bcn_int;
	union wlan_capability cap_info;
	union {
		uint8_t data[8];
		uint64_t tsf;
	} tsf_info;
	uint8_t erp;
	uint8_t dtim_period;
	uint8_t air_time_fraction;
	uint8_t qbss_chan_load;
	uint8_t nss;
	bool is_p2p;
	bool adaptive_11r_ap;
	qdf_time_t scan_entry_time;
	qdf_time_t rssi_timestamp;
	qdf_time_t hidden_ssid_timestamp;
	struct scan_mbssid_info mbssid_info;
	struct reduced_neighbor_report rnr;
	struct channel_info channel;
	bool channel_mismatch;
	struct mlme_info mlme_info;
	uint32_t tsf_delta;
	uint32_t bss_score;
	struct security_info neg_sec_info;
	uint8_t per_chain_rssi[WLAN_MGMT_TXRX_HOST_MAX_ANTENNA];
	uint64_t boottime_ns;
	uint32_t rrm_parent_tsf;
	struct element_info alt_wcn_ie;
	struct ie_list ie_list;
	struct element_info raw_frame;
	/*
	 * This is added temporarily for 6GHz channel to freq conversion
	 * to get pdev wherever it requores to convert frequency to
	 * channel as regulatory apis requires pdev as argument
	 */
	uint8_t pdev_id;
};

#define MAX_FAVORED_BSSID 16
#define MAX_ALLOWED_SSID_LIST 4

/**
 * struct weight_config - weight params to calculate best candidate
 * @rssi_weightage: RSSI weightage
 * @ht_caps_weightage: HT caps weightage
 * @vht_caps_weightage: VHT caps weightage
 * @he_caps_weightage: HE caps weightage
 * @chan_width_weightage: Channel width weightage
 * @chan_band_weightage: Channel band weightage
 * @nss_weightage: NSS weightage
 * @beamforming_cap_weightage: Beamforming caps weightage
 * @pcl_weightage: PCL weightage
 * @channel_congestion_weightage: channel congestion weightage
 * @oce_wan_weightage: OCE WAN metrics weightage
 */
struct  weight_config {
	uint8_t rssi_weightage;
	uint8_t ht_caps_weightage;
	uint8_t vht_caps_weightage;
	uint8_t he_caps_weightage;
	uint8_t chan_width_weightage;
	uint8_t chan_band_weightage;
	uint8_t nss_weightage;
	uint8_t beamforming_cap_weightage;
	uint8_t pcl_weightage;
	uint8_t channel_congestion_weightage;
	uint8_t oce_wan_weightage;
};

/**
 * struct rssi_cfg_score - rssi related params for scoring logic
 * @best_rssi_threshold: RSSI weightage
 * @good_rssi_threshold: HT caps weightage
 * @bad_rssi_threshold: VHT caps weightage
 * @good_rssi_pcnt: HE caps weightage
 * @bad_rssi_pcnt: Channel width weightage
 * @good_rssi_bucket_size: Channel band weightage
 * @bad_rssi_bucket_size: NSS weightage
 * @rssi_pref_5g_rssi_thresh: Beamforming caps weightage
 */
struct rssi_cfg_score  {
	uint32_t best_rssi_threshold;
	uint32_t good_rssi_threshold;
	uint32_t bad_rssi_threshold;
	uint32_t good_rssi_pcnt;
	uint32_t bad_rssi_pcnt;
	uint32_t good_rssi_bucket_size;
	uint32_t bad_rssi_bucket_size;
	uint32_t rssi_pref_5g_rssi_thresh;
};

/**
 * struct per_slot_scoring - define % score for differents slots for a
 *                               scoring param.
 * num_slot: number of slots in which the param will be divided.
 *           Max 15. index 0 is used for 'not_present. Num_slot will
 *           equally divide 100. e.g, if num_slot = 4 slot 0 = 0-25%, slot
 *           1 = 26-50% slot 2 = 51-75%, slot 3 = 76-100%
 * score_pcnt3_to_0: Conatins score percentage for slot 0-3
 *             BITS 0-7   :- the scoring pcnt when not present
 *             BITS 8-15  :- SLOT_1
 *             BITS 16-23 :- SLOT_2
 *             BITS 24-31 :- SLOT_3
 * score_pcnt7_to_4: Conatins score percentage for slot 4-7
 *             BITS 0-7   :- SLOT_4
 *             BITS 8-15  :- SLOT_5
 *             BITS 16-23 :- SLOT_6
 *             BITS 24-31 :- SLOT_7
 * score_pcnt11_to_8: Conatins score percentage for slot 8-11
 *             BITS 0-7   :- SLOT_8
 *             BITS 8-15  :- SLOT_9
 *             BITS 16-23 :- SLOT_10
 *             BITS 24-31 :- SLOT_11
 * score_pcnt15_to_12: Conatins score percentage for slot 12-15
 *             BITS 0-7   :- SLOT_12
 *             BITS 8-15  :- SLOT_13
 *             BITS 16-23 :- SLOT_14
 *             BITS 24-31 :- SLOT_15
 */
struct per_slot_scoring {
	uint32_t num_slot;
	uint32_t score_pcnt3_to_0;
	uint32_t score_pcnt7_to_4;
	uint32_t score_pcnt11_to_8;
	uint32_t score_pcnt15_to_12;
};

/**
 * struct scoring_config - Scoring related configuration
 * @weight_cfg: weigtage config for config
 * @rssi_score: Rssi related config for scoring config
 * @esp_qbss_scoring: esp and qbss related scoring config
 * @oce_wan_scoring: oce related scoring config
 * @bandwidth_weight_per_index: BW wight per index
 * @nss_weight_per_index: nss weight per index
 * @band_weight_per_index: band weight per index
 * @cb_mode_24G: cb mode supprted for 2.4Ghz
 * @cb_mode_5G: cb mode supprted for 5Ghz
 * @nss: Number of NSS the device support
 * @ht_cap: If dev is configured as HT capable
 * @vht_cap:If dev is configured as VHT capable
 * @he_cap: If dev is configured as HE capable
 * @vht_24G_cap:If dev is configured as VHT capable for 2.4Ghz
 * @beamformee_cap:If dev is configured as BF capable
 */
struct scoring_config {
	struct weight_config weight_cfg;
	struct rssi_cfg_score rssi_score;
	struct per_slot_scoring esp_qbss_scoring;
	struct per_slot_scoring oce_wan_scoring;
	uint32_t bandwidth_weight_per_index;
	uint32_t nss_weight_per_index;
	uint32_t band_weight_per_index;
	uint8_t cb_mode_24G;
	uint8_t cb_mode_5G;
	uint8_t vdev_nss_24g;
	uint8_t vdev_nss_5g;
	uint8_t ht_cap:1,
		vht_cap:1,
		he_cap:1,
		vht_24G_cap:1,
		beamformee_cap:1;
};

#define WLAN_SCAN_FILTER_NUM_SSID 5
#define WLAN_SCAN_FILTER_NUM_BSSID 5

#define REAM_HASH_LEN 2
#define CACHE_IDENTIFIER_LEN 2
#define HESSID_LEN 6

/**
 * struct fils_filter_info: FILS info present in scan filter
 * @realm_check: whether realm check is required
 * @fils_realm: realm hash value
 * @security_type: type of security supported
 */
struct fils_filter_info {
	bool realm_check;
	uint8_t fils_realm[REAM_HASH_LEN];
	uint8_t security_type;
};

/**
 * @bss_scoring_required :- flag to bypass scoring filtered results
 * @enable_adaptive_11r:    flag to check if adaptive 11r ini is enabled
 * @age_threshold: If set return entry which are newer than the age_threshold
 * @p2p_results: If only p2p entries is required
 * @rrm_measurement_filter: For measurement reports.if set, only SSID, BSSID
 *                          and channel is considered for filtering.
 * @num_of_bssid: number of bssid passed
 * @num_of_ssid: number of ssid
 * @num_of_channels: number of  channels
 * @num_of_auth: number of auth types
 * @num_of_enc_type: number of unicast enc type
 * @num_of_mc_enc_type: number of multicast enc type
 * @pmf_cap: Pmf capability
 * @ignore_pmf_cap: Ignore pmf capability match
 * @num_of_pcl_channels: number of pcl channels
 * @bss_type: bss type BSS/IBSS etc
 * @dot11_mode: operating modes 0 mean any
 *              11a , 11g, 11n , 11ac , 11b etc
 * @band: to get specific band 2.4G, 5G or 4.9 G
 * @rssi_threshold: AP having RSSI greater than
 *                  rssi threasholed (ignored if set 0)
 * @only_wmm_ap: If only Qos AP is needed
 * @ignore_auth_enc_type: Ignore enc type if
 *                        this is set (For WPS/OSEN connection)
 * @mobility_domain: Mobility domain for 11r
 * @country[3]: Ap with specific country code
 * @bssid_list: bssid list
 * @ssid_list: ssid list
 * @chan_freq_list: channel frequency list, frequency unit: MHz
 * @auth_type: auth type list
 * @enc_type: unicast enc type list
 * @mc_enc_type: multicast cast enc type list
 * @pcl_freq_list: PCL channel frequency list, frequency unit: MHz
 * @fils_scan_filter: FILS info
 * @pcl_weight_list: PCL Weight list
 * @bssid_hint: Mac address of bssid_hint
 */
struct scan_filter {
	bool bss_scoring_required;
	bool enable_adaptive_11r;
	qdf_time_t age_threshold;
	uint32_t p2p_results;
	uint32_t rrm_measurement_filter;
	uint32_t num_of_bssid;
	uint32_t num_of_ssid;
	uint32_t num_of_channels;
	uint32_t num_of_auth;
	uint32_t num_of_enc_type;
	uint32_t num_of_mc_enc_type;
	enum wlan_pmf_cap pmf_cap;
	bool ignore_pmf_cap;
	uint32_t num_of_pcl_channels;
	enum wlan_bss_type bss_type;
	enum wlan_phymode dot11_mode;
	enum wlan_band band;
	uint32_t rssi_threshold;
	uint32_t only_wmm_ap;
	uint32_t ignore_auth_enc_type;
	uint32_t mobility_domain;
	/* Variable params list */
	uint8_t country[3];
	struct qdf_mac_addr bssid_list[WLAN_SCAN_FILTER_NUM_BSSID];
	struct wlan_ssid ssid_list[WLAN_SCAN_FILTER_NUM_SSID];
	uint32_t chan_freq_list[QDF_MAX_NUM_CHAN];
	enum wlan_auth_type auth_type[WLAN_NUM_OF_SUPPORT_AUTH_TYPE];
	enum wlan_enc_type enc_type[WLAN_NUM_OF_ENCRYPT_TYPE];
	enum wlan_enc_type mc_enc_type[WLAN_NUM_OF_ENCRYPT_TYPE];
	uint32_t pcl_freq_list[QDF_MAX_NUM_CHAN];
	struct fils_filter_info fils_scan_filter;
	uint8_t pcl_weight_list[QDF_MAX_NUM_CHAN];
	struct qdf_mac_addr bssid_hint;
};

/**
 * enum scan_disable_reason - scan enable/disable reason
 * @REASON_SUSPEND: reason is suspend
 * @REASON_SYSTEM_DOWN: reason is system going down
 * @REASON_USER_SPACE: reason is user space initiated
 * @REASON_VDEV_DOWN: reason is vdev going down
 */
enum scan_disable_reason {
	REASON_SUSPEND  = 0x1,
	REASON_SYSTEM_DOWN = 0x2,
	REASON_USER_SPACE = 0x4,
	REASON_VDEV_DOWN = 0x8,
};

/**
 * enum scan_priority - scan priority definitions
 * @SCAN_PRIORITY_VERY_LOW: very low priority
 * @SCAN_PRIORITY_LOW: low scan priority
 * @SCAN_PRIORITY_MEDIUM: medium priority
 * @SCAN_PRIORITY_HIGH: high priority
 * @SCAN_PRIORITY_VERY_HIGH: very high priority
 * @SCAN_PRIORITY_COUNT: number of priorities supported
 */
enum scan_priority {
	SCAN_PRIORITY_VERY_LOW,
	SCAN_PRIORITY_LOW,
	SCAN_PRIORITY_MEDIUM,
	SCAN_PRIORITY_HIGH,
	SCAN_PRIORITY_VERY_HIGH,
	SCAN_PRIORITY_COUNT,
};

/**
 * enum scan_phy_mode - phymode used for scan
 * @SCAN_PHY_MODE_11A: 11a mode
 * @SCAN_PHY_MODE_11G: 11g mode
 * @SCAN_PHY_MODE_11B: 11b mode
 * @SCAN_PHY_MODE_11GONLY: 11g only mode
 * @SCAN_PHY_MODE_11NA_HT20: 11na ht20 mode
 * @SCAN_PHY_MODE_11NG_HT20: 11ng ht20 mode
 * @SCAN_PHY_MODE_11NA_HT40: 11na ht40 mode
 * @SCAN_PHY_MODE_11NG_HT40: 11ng ht40 mode
 * @SCAN_PHY_MODE_11AC_VHT20: 11ac vht20 mode
 * @SCAN_PHY_MODE_11AC_VHT40: 11ac vht40 mode
 * @SCAN_PHY_MODE_11AC_VHT80: 11ac vht80 mode
 * @SCAN_PHY_MODE_11AC_VHT20_2G: 2GHz 11ac vht20 mode
 * @SCAN_PHY_MODE_11AC_VHT40_2G: 2GHz 11ac vht40 mode
 * @SCAN_PHY_MODE_11AC_VHT80_2G: 2GHz 11ac vht80 mode
 * @SCAN_PHY_MODE_11AC_VHT80_80: 11ac vht 80+80 mode
 * @SCAN_PHY_MODE_11AC_VHT160: 11ac vht160 mode
 * @SCAN_PHY_MODE_11AX_HE20: 11ax he20 mode
 * @SCAN_PHY_MODE_11AX_HE40: 11ax he40 mode
 * @SCAN_PHY_MODE_11AX_HE80: 11ax he80 mode
 * @SCAN_PHY_MODE_11AX_HE80_80: 11ax he80+80 mode
 * @SCAN_PHY_MODE_11AX_HE160: 11ax he160 mode
 * @SCAN_PHY_MODE_11AX_HE20_2G: 2GHz 11ax he20 mode
 * @SCAN_PHY_MODE_11AX_HE40_2G: 2GHz 11ax he40 mode
 * @SCAN_PHY_MODE_11AX_HE80_2G: 2GHz 11ax he80 mode
 * @SCAN_PHY_MODE_UNKNOWN: unknown phy mode
 * @SCAN_PHY_MODE_MAX: max valid phymode
 */
enum scan_phy_mode {
	SCAN_PHY_MODE_11A = 0,
	SCAN_PHY_MODE_11G = 1,
	SCAN_PHY_MODE_11B = 2,
	SCAN_PHY_MODE_11GONLY = 3,
	SCAN_PHY_MODE_11NA_HT20 = 4,
	SCAN_PHY_MODE_11NG_HT20 = 5,
	SCAN_PHY_MODE_11NA_HT40 = 6,
	SCAN_PHY_MODE_11NG_HT40 = 7,
	SCAN_PHY_MODE_11AC_VHT20 = 8,
	SCAN_PHY_MODE_11AC_VHT40 = 9,
	SCAN_PHY_MODE_11AC_VHT80 = 10,
	SCAN_PHY_MODE_11AC_VHT20_2G = 11,
	SCAN_PHY_MODE_11AC_VHT40_2G = 12,
	SCAN_PHY_MODE_11AC_VHT80_2G = 13,
	SCAN_PHY_MODE_11AC_VHT80_80 = 14,
	SCAN_PHY_MODE_11AC_VHT160 = 15,
	SCAN_PHY_MODE_11AX_HE20 = 16,
	SCAN_PHY_MODE_11AX_HE40 = 17,
	SCAN_PHY_MODE_11AX_HE80 = 18,
	SCAN_PHY_MODE_11AX_HE80_80 = 19,
	SCAN_PHY_MODE_11AX_HE160 = 20,
	SCAN_PHY_MODE_11AX_HE20_2G = 21,
	SCAN_PHY_MODE_11AX_HE40_2G = 22,
	SCAN_PHY_MODE_11AX_HE80_2G = 23,
	SCAN_PHY_MODE_UNKNOWN = 24,
	SCAN_PHY_MODE_MAX = 24
};

/**
 * enum scan_dwelltime_adaptive_mode: dwelltime_mode
 * @SCAN_DWELL_MODE_DEFAULT: Use firmware default mode
 * @SCAN_DWELL_MODE_CONSERVATIVE: Conservative adaptive mode
 * @SCAN_DWELL_MODE_MODERATE: Moderate adaptive mode
 * @SCAN_DWELL_MODE_AGGRESSIVE: Aggressive adaptive mode
 * @SCAN_DWELL_MODE_STATIC: static adaptive mode
 */
enum scan_dwelltime_adaptive_mode {
	SCAN_DWELL_MODE_DEFAULT = 0,
	SCAN_DWELL_MODE_CONSERVATIVE = 1,
	SCAN_DWELL_MODE_MODERATE = 2,
	SCAN_DWELL_MODE_AGGRESSIVE = 3,
	SCAN_DWELL_MODE_STATIC = 4
};

/**
 * struct scan_random_attr - holds scan randomization attrs
 * @randomize: set to true for scan randomization
 * @mac_addr: mac addr to be randomized
 * @mac_mask: used to represent bits in mac_addr for randomization
 */
struct scan_random_attr {
	bool randomize;
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
	uint8_t mac_mask[QDF_MAC_ADDR_SIZE];
};

/**
 * struct probe_req_whitelist_attr - holds probe req ie whitelist attrs
 * @white_list: enable/disable whitelist
 * @ie_bitmap: bitmap of IEs to be enabled
 * @num_vendor_oui: number of vendor OUIs
 * @voui: vendor oui buffer
 */
struct probe_req_whitelist_attr {
	bool white_list;
	uint32_t ie_bitmap[PROBE_REQ_BITMAP_LEN];
	uint32_t num_vendor_oui;
	uint32_t voui[MAX_PROBE_REQ_OUIS];
};

/**
 * struct chan_info - channel information
 * @freq: frequency to scan
 * @phymode: phymode in which @frequency should be scanned
 */
struct chan_info {
	qdf_freq_t freq;
	uint32_t phymode;
};

/**
 * struct chan_list - list of frequencies to be scanned
 *  and their phymode
 * @num_chan: number of channels to scan
 * @chan: channel parameters used for this scan
 */
struct chan_list {
	uint8_t num_chan;
	struct chan_info chan[NUM_CHANNELS];
};

/**
 * struct hint_short_ssid - short SSID hint
 *  and their phymode
 * @freq_flags: freq unit: MHz (upper 16bits)
 *              flags (lower 16bits)
 * @short_ssid: short SSID
 */
struct hint_short_ssid {
	uint32_t freq_flags;
	uint32_t short_ssid;
};

/**
 * struct hint_bssid - BSSID hint
 *  and their phymode
 * @freq_flags: freq unit: MHz (upper 16bits)
 *              flags (lower 16bits)
 * @bssid: BSSID
 */
struct hint_bssid {
	uint32_t freq_flags;
	struct qdf_mac_addr bssid;
};

/**
 * enum scan_request_type: scan type
 * @SCAN_TYPE_DEFAULT: Def scan
 * @SCAN_TYPE_P2P_SEARCH: P2P Search
 * @SCAN_TYPE_P2P_LISTEN: P2P listed
 * @SCAN_TYPE_RRM: RRM scan request
 */
enum scan_request_type {
	SCAN_TYPE_DEFAULT = 0,
	SCAN_TYPE_P2P_SEARCH = 1,
	SCAN_TYPE_P2P_LISTEN = 2,
	SCAN_TYPE_RRM = 3
};

/**
 * struct scan_req_params - start scan request parameter
 * @scan_id: scan id
 * @scan_req_id: scan requester id
 * @vdev_id: vdev id where scan was originated
 * @pdev_id: pdev id of parent pdev
 * @scan_priority: scan priority
 * @scan_ev_started: notify scan started event
 * @scan_ev_completed: notify scan completed event
 * @scan_ev_bss_chan: notify bss chan event
 * @scan_ev_foreign_chan: notify foreign chan event
 * @scan_ev_dequeued: notify scan request dequed event
 * @scan_ev_preempted: notify scan preempted event
 * @scan_ev_start_failed: notify scan start failed event
 * @scan_ev_restarted: notify scan restarted event
 * @scan_ev_foreign_chn_exit: notify foreign chan exit event
 * @scan_ev_invalid: notify invalid scan request event
 * @scan_ev_gpio_timeout: notify gpio timeout event
 * @scan_ev_suspended: notify scan suspend event
 * @scan_ev_resumed: notify scan resumed event
 * @scan_events: variable to read and set scan_ev_* flags in one shot
 *               can be used to dump all scan_ev_* flags for debug
 * @dwell_time_active: active dwell time
 * @dwell_time_active_2g: active dwell time for 2G channels, if it's not zero
 * @dwell_time_passive: passive dwell time
 * @dwell_time_active_6g: 6Ghz active dwell time
 * @dwell_time_passive_6g: 6Ghz passive dwell time
 * @min_rest_time: min rest time
 * @max_rest_time: max rest time
 * @repeat_probe_time: repeat probe time
 * @probe_spacing_time: probe spacing time
 * @idle_time: idle time
 * @max_scan_time: max scan time
 * @probe_delay: probe delay
 * @scan_offset_time: Support split scanning on the
 *                    same channel for CBS feature.
 * @scan_f_passive: passively scan all channels including active channels
 * @scan_f_bcast_probe: add wild card ssid prbreq even if ssid_list is specified
 * @scan_f_cck_rates: add cck rates to rates/xrates ie in prb req
 * @scan_f_ofdm_rates: add ofdm rates to rates/xrates ie in prb req
 * @scan_f_chan_stat_evnt: enable indication of chan load and noise floor
 * @scan_f_filter_prb_req: filter Probe request frames
 * @scan_f_bypass_dfs_chn: when set, do not scan DFS channels
 * @scan_f_continue_on_err:continue scan even if few certain erros have occurred
 * @scan_f_offchan_mgmt_tx: allow mgmt transmission during off channel scan
 * @scan_f_offchan_data_tx: allow data transmission during off channel scan
 * @scan_f_promisc_mode: scan with promiscuous mode
 * @scan_f_capture_phy_err: enable capture ppdu with phy errrors
 * @scan_f_strict_passive_pch: do passive scan on passive channels
 * @scan_f_half_rate: enable HALF (10MHz) rate support
 * @scan_f_quarter_rate: set Quarter (5MHz) rate support
 * @scan_f_force_active_dfs_chn: allow to send probe req on DFS channel
 * @scan_f_add_tpc_ie_in_probe: add TPC ie in probe req frame
 * @scan_f_add_ds_ie_in_probe: add DS ie in probe req frame
 * @scan_f_add_spoofed_mac_in_probe: use random mac address for TA in probe
 * @scan_f_add_rand_seq_in_probe: use random sequence number in probe
 * @scan_f_en_ie_whitelist_in_probe: enable ie whitelist in probe
 * @scan_f_forced: force scan even in presence of data traffic
 * @scan_f_2ghz: scan 2.4 GHz channels
 * @scan_f_5ghz: scan 5 GHz channels
 * @scan_f_wide_band: scan in 40 MHz or higher bandwidth
 * @scan_flags: variable to read and set scan_f_* flags in one shot
 *              can be used to dump all scan_f_* flags for debug
 * @burst_duration: burst duration
 * @num_bssid: no of bssid
 * @num_ssids: no of ssid
 * @n_probes: no of probe
 * @chan_list: channel list
 * @ssid: ssid list
 * @bssid_list: Lisst of bssid to scan
 * @scan_random: scan randomization params
 * @ie_whitelist: probe req IE whitelist attrs
 * @extraie: list of optional/vendor specific ie's to be added in probe requests
 * @htcap: htcap ie
 * @vhtcap: vhtcap ie
 * @scan_ctrl_flags_ext: scan control flag extended
 * @num_hint_s_ssid: number of short SSID hints
 * @num_hint_bssid: number of BSSID hints
 * @hint_s_ssid: short SSID hints
 * @hint_bssid: BSSID hints
 */

struct scan_req_params {
	uint32_t scan_id;
	uint32_t scan_req_id;
	uint32_t vdev_id;
	uint32_t pdev_id;
	enum scan_priority scan_priority;
	enum scan_request_type scan_type;
	union {
		struct {
			uint32_t scan_ev_started:1,
				 scan_ev_completed:1,
				 scan_ev_bss_chan:1,
				 scan_ev_foreign_chan:1,
				 scan_ev_dequeued:1,
				 scan_ev_preempted:1,
				 scan_ev_start_failed:1,
				 scan_ev_restarted:1,
				 scan_ev_foreign_chn_exit:1,
				 scan_ev_invalid:1,
				 scan_ev_gpio_timeout:1,
				 scan_ev_suspended:1,
				 scan_ev_resumed:1;
		};
		uint32_t scan_events;
	};
	uint32_t dwell_time_active;
	uint32_t dwell_time_active_2g;
	uint32_t dwell_time_passive;
	uint32_t dwell_time_active_6g;
	uint32_t dwell_time_passive_6g;
	uint32_t min_rest_time;
	uint32_t max_rest_time;
	uint32_t repeat_probe_time;
	uint32_t probe_spacing_time;
	uint32_t idle_time;
	uint32_t max_scan_time;
	uint32_t probe_delay;
	uint32_t scan_offset_time;
	union {
		struct {
			uint32_t scan_f_passive:1,
				 scan_f_bcast_probe:1,
				 scan_f_cck_rates:1,
				 scan_f_ofdm_rates:1,
				 scan_f_chan_stat_evnt:1,
				 scan_f_filter_prb_req:1,
				 scan_f_bypass_dfs_chn:1,
				 scan_f_continue_on_err:1,
				 scan_f_offchan_mgmt_tx:1,
				 scan_f_offchan_data_tx:1,
				 scan_f_promisc_mode:1,
				 scan_f_capture_phy_err:1,
				 scan_f_strict_passive_pch:1,
				 scan_f_half_rate:1,
				 scan_f_quarter_rate:1,
				 scan_f_force_active_dfs_chn:1,
				 scan_f_add_tpc_ie_in_probe:1,
				 scan_f_add_ds_ie_in_probe:1,
				 scan_f_add_spoofed_mac_in_probe:1,
				 scan_f_add_rand_seq_in_probe:1,
				 scan_f_en_ie_whitelist_in_probe:1,
				 scan_f_forced:1,
				 scan_f_2ghz:1,
				 scan_f_5ghz:1,
				 scan_f_wide_band:1;
		};
		uint32_t scan_flags;
	};
	union {
		struct {
			uint32_t scan_policy_high_accuracy:1,
				 scan_policy_low_span:1,
				 scan_policy_low_power:1;
		};
		uint32_t scan_policy_type;
	};

	enum scan_dwelltime_adaptive_mode adaptive_dwell_time_mode;
	uint32_t burst_duration;
	uint32_t num_bssid;
	uint32_t num_ssids;
	uint32_t n_probes;
	struct chan_list chan_list;
	struct wlan_ssid ssid[WLAN_SCAN_MAX_NUM_SSID];
	struct qdf_mac_addr bssid_list[WLAN_SCAN_MAX_NUM_BSSID];
	struct scan_random_attr scan_random;
	struct probe_req_whitelist_attr ie_whitelist;
	struct element_info extraie;
	struct element_info htcap;
	struct element_info vhtcap;
	uint32_t scan_ctrl_flags_ext;
	uint32_t num_hint_s_ssid;
	uint32_t num_hint_bssid;
	struct hint_short_ssid hint_s_ssid[WLAN_SCAN_MAX_HINT_S_SSID];
	struct hint_bssid hint_bssid[WLAN_SCAN_MAX_HINT_BSSID];
};

/**
 * struct scan_start_request - scan request config
 * @vdev: vdev
 * @scan_req: common scan start request parameters
 */
struct scan_start_request {
	struct wlan_objmgr_vdev *vdev;
	struct scan_req_params scan_req;
};

/**
 * enum scan_cancel_type - type specifiers for cancel scan request
 * @WLAN_SCAN_CANCEL_SINGLE: cancel particular scan specified by scan_id
 * @WLAN_SCAN_CANCEL_VAP_ALL: cancel all scans running on a particular vdevid
 * WLAN_SCAN_CANCEL_PDEV_ALL: cancel all scans running on parent pdev of vdevid
 */
enum scan_cancel_req_type {
	WLAN_SCAN_CANCEL_SINGLE = 1,
	WLAN_SCAN_CANCEL_VDEV_ALL,
	WLAN_SCAN_CANCEL_PDEV_ALL,
};

/**
 * struct scan_cancel_param - stop scan cmd parameter
 * @requester: scan requester
 * @scan_id: scan id
 * @req_type: scan request type
 * @vdev_id: vdev id
 * @pdev_id: pdev id of parent pdev
 */
struct scan_cancel_param {
	uint32_t requester;
	uint32_t scan_id;
	enum scan_cancel_req_type req_type;
	uint32_t vdev_id;
	uint32_t pdev_id;
};

/**
 * struct scan_cancel_request - stop scan cmd
 * @vdev: vdev object
 * @wait_tgt_cancel: wait for target to cancel scan
 * @cancel_req: stop scan cmd parameter
 */
struct scan_cancel_request {
	/* Extra parameters consumed by scan module or serialization */
	struct wlan_objmgr_vdev *vdev;
	bool wait_tgt_cancel;
	/* Actual scan cancel request parameters */
	struct scan_cancel_param cancel_req;
};

/**
 * enum scan_event_type - scan event types
 * @SCAN_EVENT_TYPE_STARTED: scan started
 * @SCAN_EVENT_TYPE_COMPLETED: scan completed
 * @SCAN_EVENT_TYPE_BSS_CHANNEL: HW came back to home channel
 * @SCAN_EVENT_TYPE_FOREIGN_CHANNEL: HW moved to foreign channel
 * @SCAN_EVENT_TYPE_DEQUEUED: scan request dequeued
 * @SCAN_EVENT_TYPE_PREEMPTED: scan got preempted
 * @SCAN_EVENT_TYPE_START_FAILED: couldn't start scan
 * @SCAN_EVENT_TYPE_RESTARTED: scan restarted
 * @SCAN_EVENT_TYPE_FOREIGN_CHANNEL_EXIT: HW exited foreign channel
 * @SCAN_EVENT_TYPE_SUSPENDED: scan got suspended
 * @SCAN_EVENT_TYPE_RESUMED: scan resumed
 * @SCAN_EVENT_TYPE_NLO_COMPLETE: NLO completed
 * @SCAN_EVENT_TYPE_NLO_MATCH: NLO match event
 * @SCAN_EVENT_TYPE_INVALID: invalid request
 * @SCAN_EVENT_TYPE_GPIO_TIMEOUT: gpio timeout
 * @SCAN_EVENT_TYPE_RADIO_MEASUREMENT_START: radio measurement start
 * @SCAN_EVENT_TYPE_RADIO_MEASUREMENT_END: radio measurement end
 * @SCAN_EVENT_TYPE_BSSID_MATCH: bssid match found
 * @SCAN_EVENT_TYPE_FOREIGN_CHANNEL_GET_NF: foreign channel noise floor
 * @SCAN_EVENT_TYPE_MAX: marker for invalid event
 */
enum scan_event_type {
	SCAN_EVENT_TYPE_STARTED,
	SCAN_EVENT_TYPE_COMPLETED,
	SCAN_EVENT_TYPE_BSS_CHANNEL,
	SCAN_EVENT_TYPE_FOREIGN_CHANNEL,
	SCAN_EVENT_TYPE_DEQUEUED,
	SCAN_EVENT_TYPE_PREEMPTED,
	SCAN_EVENT_TYPE_START_FAILED,
	SCAN_EVENT_TYPE_RESTARTED,
	SCAN_EVENT_TYPE_FOREIGN_CHANNEL_EXIT,
	SCAN_EVENT_TYPE_SUSPENDED,
	SCAN_EVENT_TYPE_RESUMED,
	SCAN_EVENT_TYPE_NLO_COMPLETE,
	SCAN_EVENT_TYPE_NLO_MATCH,
	SCAN_EVENT_TYPE_INVALID,
	SCAN_EVENT_TYPE_GPIO_TIMEOUT,
	SCAN_EVENT_TYPE_RADIO_MEASUREMENT_START,
	SCAN_EVENT_TYPE_RADIO_MEASUREMENT_END,
	SCAN_EVENT_TYPE_BSSID_MATCH,
	SCAN_EVENT_TYPE_FOREIGN_CHANNEL_GET_NF,
	SCAN_EVENT_TYPE_MAX,
};

/**
 * enum scan_completion_reason - scan completion reason
 * @SCAN_REASON_NONE: un specified reason
 * @SCAN_REASON_COMPLETED: scan successfully completed
 * @SCAN_REASON_CANCELLED: scan got cancelled
 * @SCAN_REASON_PREEMPTED: scan got preempted
 * @SCAN_REASON_TIMEDOUT: couldnt complete within specified time
 * @SCAN_REASON_INTERNAL_FAILURE: cancelled because of some failure
 * @SCAN_REASON_SUSPENDED: scan suspended
 * @SCAN_REASON_RUN_FAILED: run failed
 * @SCAN_REASON_TERMINATION_FUNCTION: termination function
 * @SCAN_REASON_MAX_OFFCHAN_RETRIES: max retries exceeded thresold
 * @SCAN_REASON_DFS_VIOLATION: Scan start failure due to DFS violation.
 * @SCAN_REASON_MAX: invalid completion reason marker
 */
enum scan_completion_reason {
	SCAN_REASON_NONE,
	SCAN_REASON_COMPLETED,
	SCAN_REASON_CANCELLED,
	SCAN_REASON_PREEMPTED,
	SCAN_REASON_TIMEDOUT,
	SCAN_REASON_INTERNAL_FAILURE,
	SCAN_REASON_SUSPENDED,
	SCAN_REASON_RUN_FAILED,
	SCAN_REASON_TERMINATION_FUNCTION,
	SCAN_REASON_MAX_OFFCHAN_RETRIES,
	SCAN_REASON_DFS_VIOLATION,
	SCAN_REASON_MAX,
};

/**
 * struct scan_event - scan event definition
 * @vdev_id: vdev where scan was run
 * @type: type of scan event
 * @reason: completion reason
 * @chan_freq: channel centre frequency
 * @requester: requester id
 * @scan_id: scan id
 * @timestamp: timestamp in microsec recorded by target for the scan event
 * @scan_start_req: scan request object used to start this scan
 */
struct scan_event {
	uint32_t vdev_id;
	enum scan_event_type type;
	enum scan_completion_reason reason;
	uint32_t chan_freq;
	uint32_t requester;
	uint32_t scan_id;
	uint32_t timestamp;
	struct scan_start_request *scan_start_req;
};

/**
 * struct scan_event_info - scan event information
 * @vdev: vdev object
 * @event: scan event
 */
struct scan_event_info {
	struct wlan_objmgr_vdev *vdev;
	struct scan_event event;
};

/**
 * enum scm_scan_status - scan status
 * @SCAN_NOT_IN_PROGRESS: Neither active nor pending scan in progress
 * @SCAN_IS_ACTIVE: scan request is present only in active list
 * @SCAN_IS_PENDING: scan request is present only in pending list
 * @SCAN_IS_ACTIVE_AND_PENDING: scan request is present in active
 *                               and pending both lists
 */
enum scm_scan_status {
	SCAN_NOT_IN_PROGRESS = 0, /* Must be 0 */
	SCAN_IS_ACTIVE,
	SCAN_IS_PENDING,
	SCAN_IS_ACTIVE_AND_PENDING,
};

/**
 * scan_event_handler() - function prototype of scan event handlers
 * @vdev: vdev object
 * @event: scan event
 * @arg: argument
 *
 * PROTO TYPE, scan event handler call back function prototype
 *
 * @Return: void
 */
typedef void (*scan_event_handler) (struct wlan_objmgr_vdev *vdev,
	struct scan_event *event, void *arg);

/**
 * enum scan_cb_type - update beacon cb type
 * @SCAN_CB_TYPE_INFORM_BCN: Calback to indicate beacon to OS
 * @SCAN_CB_TYPE_UPDATE_BCN: Calback to indicate beacon
 * @SCAN_CB_TYPE_UNLINK_BSS: cb to unlink bss entry
 *                    to MLME and update MLME info
 *
 */
enum scan_cb_type {
	SCAN_CB_TYPE_INFORM_BCN,
	SCAN_CB_TYPE_UPDATE_BCN,
	SCAN_CB_TYPE_UNLINK_BSS,
};

/* Set PNO */
#define SCAN_PNO_MAX_PLAN_REQUEST   2
#define SCAN_PNO_MAX_NETW_CHANNELS_EX  60
#define SCAN_PNO_MAX_SUPP_NETWORKS  16
#define SCAN_PNO_DEF_SLOW_SCAN_MULTIPLIER 6
#define SCAN_PNO_DEF_SCAN_TIMER_REPEAT 20
#define SCAN_PNO_MATCH_WAKE_LOCK_TIMEOUT         (5 * 1000)     /* in msec */
#define SCAN_MAX_IE_LENGTH 255
#ifdef CONFIG_SLUB_DEBUG_ON
#define SCAN_PNO_SCAN_COMPLETE_WAKE_LOCK_TIMEOUT (2 * 1000)     /* in msec */
#else
#define SCAN_PNO_SCAN_COMPLETE_WAKE_LOCK_TIMEOUT (1 * 1000)     /* in msec */
#endif /* CONFIG_SLUB_DEBUG_ON */

/**
 * enum ssid_bc_type - SSID broadcast type
 * @SSID_BC_TYPE_UNKNOWN: Broadcast unknown
 * @SSID_BC_TYPE_NORMAL: Broadcast normal
 * @SSID_BC_TYPE_HIDDEN: Broadcast hidden
 */
enum ssid_bc_type {
	SSID_BC_TYPE_UNKNOWN = 0,
	SSID_BC_TYPE_NORMAL = 1,
	SSID_BC_TYPE_HIDDEN = 2,
};

/**
 * struct pno_nw_type - pno nw type
 * @ssid: ssid
 * @authentication: authentication type
 * @encryption: encryption type
 * @bcastNetwType: broadcast nw type
 * @ucChannelCount: uc channel count
 * @aChannels: pno channel
 * @rssiThreshold: rssi threshold
 */
struct pno_nw_type {
	struct wlan_ssid ssid;
	uint32_t authentication;
	uint32_t encryption;
	uint32_t bc_new_type;
	uint8_t channel_cnt;
	uint32_t channels[SCAN_PNO_MAX_NETW_CHANNELS_EX];
	int32_t rssi_thresh;
};

/**
 * struct connected_pno_band_rssi_pref - BSS preference based on band
 * and RSSI
 * @band: band preference
 * @rssi_pref: RSSI preference
 */
struct cpno_band_rssi_pref {
	int8_t band;
	int8_t rssi;
};

/**
 * struct nlo_mawc_params - Motion Aided Wireless Connectivity based
 *                          Network List Offload configuration
 * @vdev_id: VDEV ID on which the configuration needs to be applied
 * @enable: flag to enable or disable
 * @exp_backoff_ratio: ratio of exponential backoff
 * @init_scan_interval: initial scan interval(msec)
 * @max_scan_interval:  max scan interval(msec)
 */
struct nlo_mawc_params {
	uint8_t vdev_id;
	bool enable;
	uint32_t exp_backoff_ratio;
	uint32_t init_scan_interval;
	uint32_t max_scan_interval;
};

/**
 * struct pno_scan_req_params - PNO Scan request structure
 * @networks_cnt: Number of networks
 * @do_passive_scan: Flag to request passive scan to fw
 * @vdev_id: vdev id
 * @fast_scan_period: Fast Scan period
 * @slow_scan_period: Slow scan period
 * @delay_start_time: delay in seconds to use before starting the first scan
 * @fast_scan_max_cycles: Fast scan max cycles
 * @scan_backoff_multiplier: multiply fast scan period by this after max cycles
 * @pno_channel_prediction: PNO channel prediction feature status
 * @uint32_t active_dwell_time: active dwell time
 * @uint32_t passive_dwell_time: passive dwell time
 * @top_k_num_of_channels: top K number of channels are used for tanimoto
 * distance calculation.
 * @stationary_thresh: threshold value to determine that the STA is stationary.
 * @adaptive_dwell_mode: adaptive dwelltime mode for pno scan
 * @channel_prediction_full_scan: periodic timer upon which a full scan needs
 * to be triggered.
 * @networks_list: Preferred network list
 * @scan_random: scan randomization params
 * @ie_whitelist: probe req IE whitelist attrs
 * @relative_rssi_set: Flag to check whether realtive_rssi is set or not
 * @relative_rssi: Relative rssi threshold, used for connected pno
 * @band_rssi_pref: Band and RSSI preference that can be given to one BSS
 *     over the other BSS
 *
 * E.g.
 *	{ fast_scan_period=120, fast_scan_max_cycles=2,
 *	  slow_scan_period=1800, scan_backoff_multiplier=2 }
 *	Result: 120s x2, 240s x2, 480s x2, 960s x2, 1800s xN
 * @mawc_params: Configuration parameters for NLO MAWC.
 */
struct pno_scan_req_params {
	uint32_t networks_cnt;
	bool     do_passive_scan;
	uint32_t vdev_id;
	uint32_t fast_scan_period;
	uint32_t slow_scan_period;
	uint32_t delay_start_time;
	uint32_t fast_scan_max_cycles;
	uint8_t scan_backoff_multiplier;
	uint32_t active_dwell_time;
	uint32_t passive_dwell_time;
	uint32_t pno_channel_prediction;
	uint32_t top_k_num_of_channels;
	uint32_t stationary_thresh;
	enum scan_dwelltime_adaptive_mode adaptive_dwell_mode;
	uint32_t channel_prediction_full_scan;
	struct pno_nw_type networks_list[SCAN_PNO_MAX_SUPP_NETWORKS];
	struct scan_random_attr scan_random;
	struct probe_req_whitelist_attr ie_whitelist;
	bool relative_rssi_set;
	int8_t relative_rssi;
	struct cpno_band_rssi_pref band_rssi_pref;
	struct nlo_mawc_params mawc_params;
};

/**
 * struct scan_user_cfg - user configuration required for for scan
 * @ie_whitelist: probe req IE whitelist attrs
 * @sta_miracast_mcc_rest_time: sta miracast mcc rest time
 * @score_config: scoring logic configuration
 */
struct scan_user_cfg {
	struct probe_req_whitelist_attr ie_whitelist;
	uint32_t sta_miracast_mcc_rest_time;
	struct scoring_config score_config;
};

/**
 * update_beacon_cb() - cb to inform/update beacon
 * @psoc: psoc pointer
 * @scan_params:  scan entry to inform/update
 *
 * @Return: void
 */
typedef void (*update_beacon_cb) (struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *scan_entry);

/**
 * scan_iterator_func() - function prototype of scan iterator function
 * @scan_entry: scan entry object
 * @arg: extra argument
 *
 * PROTO TYPE, scan iterator function prototype
 *
 * @Return: QDF_STATUS
 */
typedef QDF_STATUS (*scan_iterator_func) (void *arg,
	struct scan_cache_entry *scan_entry);

/**
 * enum scan_priority - scan priority definitions
 * @SCAN_CFG_DISABLE_SCAN_COMMAND_TIMEOUT: disable scan command timeout
 * @SCAN_CFG_DROP_BCN_ON_CHANNEL_MISMATCH: config to drop beacon/probe
 *  response frames if received channel and IE channels do not match
 */
enum scan_config {
	SCAN_CFG_DISABLE_SCAN_COMMAND_TIMEOUT,
	SCAN_CFG_DROP_BCN_ON_CHANNEL_MISMATCH,
};

/**
 * enum ext_cap_bit_field - Extended capabilities bit field
 * @BSS_2040_COEX_MGMT_SUPPORT: 20/40 BSS Coexistence Management Support field
 * @OBSS_NARROW_BW_RU_IN_ULOFDMA_TOLERENT_SUPPORT: OBSS Narrow  Bandwidth RU
 *     in UL OFDMA  Tolerance Support
 */
enum ext_cap_bit_field {
	BSS_2040_COEX_MGMT_SUPPORT = 0,
	OBSS_NARROW_BW_RU_IN_ULOFDMA_TOLERENT_SUPPORT = 79,
};

/**
 * scan_rnr_info - RNR information
 * @timestamp: time stamp of beacon/probe
 * @short_ssid: Short SSID
 * @bssid: BSSID
 */
struct scan_rnr_info {
	qdf_time_t timestamp;
	uint32_t short_ssid;
	struct qdf_mac_addr bssid;
};

/**
 * struct scan_rnr_node - Scan RNR entry node
 * @node: node pointers
 * @entry: scan RNR entry pointer
 */
struct scan_rnr_node {
	qdf_list_node_t node;
	struct scan_rnr_info entry;
};

/**
 * meta_rnr_channel - Channel information for scan priority algorithm
 * @chan_freq: channel frequency
 * @bss_beacon_probe_count: Beacon and probe request count
 * @saved_profile_count: Saved profile count
 * @beacon_probe_last_time_found: Timestamp of beacon/probe observed
 * @rnr_list: RNR list to store RNR IE information
 */
struct meta_rnr_channel {
	uint32_t chan_freq;
	uint32_t bss_beacon_probe_count;
	uint32_t saved_profile_count;
	qdf_time_t beacon_probe_last_time_found;
	qdf_list_t rnr_list;
};

/**
 * channel_list_db - Database for channel information
 * @channel: channel meta information
 */
struct channel_list_db {
	struct meta_rnr_channel channel[NUM_6GHZ_CHANNELS];
};

/**
 * rnr_chan_weight - RNR channel weightage
 * @chan_freq: channel frequency
 * @weight: weightage of the channel
 */
struct rnr_chan_weight {
	uint32_t chan_freq;
	uint32_t weight;
};
#endif
