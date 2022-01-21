
/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _WLAN_CFR_UTILS_API_H_
#define _WLAN_CFR_UTILS_API_H_

#include <wlan_objmgr_cmn.h>
#include <qdf_streamfs.h>
#ifdef WLAN_ENH_CFR_ENABLE
#include <qdf_timer.h>
#endif

#define cfr_alert(format, args...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_err(format, args...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_warn(format, args...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_info(format, args...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_CFR, format, ## args)

#define cfr_debug(format, args...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_CFR, format, ## args)

#define DBR_EVENT_TIMEOUT_IN_MS_CFR 1
#define DBR_NUM_RESP_PER_EVENT_CFR 1
#define MAX_CFR_ENABLED_CLIENTS 10
#define CFR_CAPTURE_HOST_MEM_REQ_ID 9
#define CFR_HOST_MEM_READ_INDEX_DEFAULT 8
#define CFR_VENDOR_ID 0x8cfdf0
#ifdef WLAN_ENH_CFR_ENABLE
#define MAX_CFR_MU_USERS 4
#define NUM_CHAN_CAPTURE_STATUS 4
#define NUM_CHAN_CAPTURE_REASON 6
#if defined(QCA_WIFI_QCA6750) || defined(QCA_WIFI_QCA6490)
#define MAX_TA_RA_ENTRIES 4
#define MAX_RESET_CFG_ENTRY 0xF
#else
#define MAX_TA_RA_ENTRIES 16
#define MAX_RESET_CFG_ENTRY 0xFFFF
#endif
#define CFR_INVALID_VDEV_ID 0xff
#define DEFAULT_SRNGID_CFR 0
#endif

#define MAX_CFR_PRD  (10 * 60 * 1000)   /* 10 minutes */
#define CFR_MOD_PRD  10                 /* CFR period to be multiples of 10ms */

#define MAX_AGC_GAIN 62

enum cfrmetaversion {
	CFR_META_VERSION_NONE,
	CFR_META_VERSION_1, /* initial version for legacy_cfr_metadata */
	CFR_META_VERSION_2, /* initial version for dbr_cfr_metadata */
	CFR_META_VERSION_3, /* initial version for enh_cfr_metadata */
	CFR_META_VERSION_4, /* agc gain, cfo, rx_start_ts in dbr_cfr_metadata */
	CFR_META_VERSION_5, /* agc gain, cfo, rx_start_ts in enh_cfr_metadata */
	CFR_META_VERSION_6, /* mcs, gi_type in dbr_cfr_metadata */
	CFR_META_VERSION_7, /* mcs, gi_type, sig_info in enh_cfr_metadata */
	CFR_META_VERSION_MAX = 0xFF,
};

enum cfrdataversion {
	CFR_DATA_VERSION_NONE,
	CFR_DATA_VERSION_1,
	CFR_DATA_VERSION_MAX = 0xFF,
};

enum cfrplatformtype {
	CFR_PLATFORM_TYPE_NONE,
	CFR_PLATFORM_TYPE_MIPS,
	CFR_PLATFORM_TYPE_ARM,
	CFR_PLATFFORM_TYPE_MAX = 0xFF,
};

enum cfrradiotype {
	CFR_CAPTURE_RADIO_NONE,
	CFR_CAPTURE_RADIO_OSPREY,
	CFR_CAPTURE_RADIO_PEAKCOCK,
	CFR_CAPTURE_RADIO_SCORPION,
	CFR_CAPTURE_RADIO_HONEYBEE,
	CFR_CAPTURE_RADIO_DRAGONFLY,
	CFR_CAPTURE_RADIO_JET,
	CFR_CAPTURE_RADIO_PEREGRINE = 17,
	CFR_CAPTURE_RADIO_SWIFT,
	CFR_CAPTURE_RADIO_BEELINER,
	CFR_CAPTURE_RADIO_CASCADE,
	CFR_CAPTURE_RADIO_DAKOTA,
	CFR_CAPTURE_RADIO_BESRA,
	CFR_CAPTURE_RADIO_HKV2,
	CFR_CAPTURE_RADIO_CYP,
	CFR_CAPTURE_RADIO_HSP,
	CFR_CAPTURE_RADIO_PINE,
	CFR_CAPTURE_RADIO_ADRASTEA,
	CFR_CAPTURE_RADIO_MAPLE,
	CFR_CAPTURE_RADIO_MOSELLE,
	CFR_CAPTURE_RADIO_SPRUCE,
	CFR_CAPTURE_RADIO_ALDER,
	CFR_CAPTURE_RADIO_WAIKIKI,
	CFR_CAPTURE_RADIO_KIWI,
	CFR_CAPTURE_RADIO_MAX = 0xFF,
};

enum ack_capture_mode {
	CFR_LEGACY_ACK     = 0,
	CFR_DUP_LEGACY_ACK = 1,
	CFR_HT_ACK         = 2,
	CFR_VHT_ACK        = 3,
	CFR_INVALID_ACK, /*Always keep this at last*/
};

/* Similar to WMI_PEER_CFR_CAPTURE_METHOD used in one-shot capture */
enum cfr_capture_type {
	CFR_TYPE_METHOD_NULL_FRAME = 0,
	CFR_TYPE_METHOD_NULL_FRAME_WITH_PHASE = 1,
	CFR_TYPE_METHOD_PROBE_RESP = 2,
	CFR_TYPE_METHOD_TM = 3,
	CFR_TYPE_METHOD_FTM = 4,
	CFR_TYPE_METHOD_ACK_RESP_TO_TM_FTM = 5,
	CFR_TYPE_METHOD_TA_RA_TYPE_FILTER = 6,
	CFR_TYPE_METHOD_NDPA_NDP = 7,
	CFR_TYPE_METHOD_ALL_PACKET = 8,
	/* Add new capture methods before this line */
	CFR_TYPE_METHOD_LAST_VALID,
	CFR_TYPE_METHOD_AUTO = 0xff,
	CFR_TYPE_METHOD_MAX,
};

/* ensure to add new members at the end of the structure only */
struct legacy_cfr_metadata {
	u_int8_t    peer_addr[QDF_MAC_ADDR_SIZE];
	u_int8_t    status;
	u_int8_t    capture_bw;
	u_int8_t    channel_bw;
	u_int8_t    phy_mode;
	u_int16_t   prim20_chan;
	u_int16_t   center_freq1;
	u_int16_t   center_freq2;
	u_int8_t    capture_mode;
	u_int8_t    capture_type;
	u_int8_t    sts_count;
	u_int8_t    num_rx_chain;
	u_int32_t   timestamp;
	u_int32_t   length;
} __attribute__ ((__packed__));

#define HOST_MAX_CHAINS 8

/* ensure to add new members at the end of the structure only */
struct dbr_cfr_metadata {
	u_int8_t    peer_addr[QDF_MAC_ADDR_SIZE];
	u_int8_t    status;
	u_int8_t    capture_bw;
	u_int8_t    channel_bw;
	u_int8_t    phy_mode;
	u_int16_t   prim20_chan;
	u_int16_t   center_freq1;
	u_int16_t   center_freq2;
	u_int8_t    capture_mode;
	u_int8_t    capture_type;
	u_int8_t    sts_count;
	u_int8_t    num_rx_chain;
	u_int32_t   timestamp;
	u_int32_t   length;
	u_int32_t   chain_rssi[HOST_MAX_CHAINS];
	u_int16_t   chain_phase[HOST_MAX_CHAINS];
	u_int32_t   rtt_cfo_measurement;
	u_int8_t    agc_gain[HOST_MAX_CHAINS];
	u_int32_t   rx_start_ts;
	u_int16_t   mcs_rate;
	u_int16_t   gi_type;
	u_int8_t    agc_gain_tbl_index[HOST_MAX_CHAINS];
} __attribute__ ((__packed__));

#ifdef WLAN_ENH_CFR_ENABLE
struct cfr_su_sig_info {
	u_int8_t coding;
	u_int8_t stbc;
	u_int8_t beamformed;
	u_int8_t dcm;
	u_int8_t ltf_size;
	u_int8_t sgi;
	u_int16_t reserved;
} __attribute__ ((__packed__));

/* ensure to add new members at the end of the structure only */
struct enh_cfr_metadata {
	u_int8_t    status;
	u_int8_t    capture_bw;
	u_int8_t    channel_bw;
	u_int8_t    phy_mode;
	u_int16_t   prim20_chan;
	u_int16_t   center_freq1;
	u_int16_t   center_freq2;
	u_int8_t    capture_mode; /* ack_capture_mode */
	u_int8_t    capture_type; /* cfr_capture_type */
	u_int8_t    sts_count;
	u_int8_t    num_rx_chain;
	u_int64_t   timestamp;
	u_int32_t   length;
	u_int8_t    is_mu_ppdu;
	u_int8_t    num_mu_users;
	union {
		u_int8_t    su_peer_addr[QDF_MAC_ADDR_SIZE];
		u_int8_t    mu_peer_addr[MAX_CFR_MU_USERS][QDF_MAC_ADDR_SIZE];
	} peer_addr;
	u_int32_t   chain_rssi[HOST_MAX_CHAINS];
	u_int16_t   chain_phase[HOST_MAX_CHAINS];
	u_int32_t   rtt_cfo_measurement;
	u_int8_t    agc_gain[HOST_MAX_CHAINS];
	u_int32_t   rx_start_ts;
	u_int16_t   mcs_rate;
	u_int16_t   gi_type;
	struct cfr_su_sig_info sig_info;
	u_int8_t    agc_gain_tbl_index[HOST_MAX_CHAINS];
} __attribute__ ((__packed__));
#endif

#define  CFR_META_DATA_LEN \
	(sizeof(struct csi_cfr_header) - sizeof(struct cfr_header_cmn))

struct cfr_header_cmn {
	u_int32_t   start_magic_num;
	u_int32_t   vendorid;
	u_int8_t    cfr_metadata_version;
	u_int8_t    cfr_data_version;
	u_int8_t    chip_type;
	u_int8_t    pltform_type;
	u_int32_t   cfr_metadata_len;
} __attribute__ ((__packed__));

struct csi_cfr_header {
	struct cfr_header_cmn cmn;
	union {
		struct legacy_cfr_metadata meta_legacy;
		struct dbr_cfr_metadata meta_dbr;
#ifdef WLAN_ENH_CFR_ENABLE
		struct enh_cfr_metadata meta_enh;
#endif
	} u;
} __attribute__ ((__packed__));

/**
 * struct cfr_capture_params - structure to store cfr config param
 * bandwidth: bandwitdh of capture
 * period: period of capture
 * method: enum of method being followed to capture cfr data. 0-QoS null data
 */
struct cfr_capture_params {
	u_int8_t   bandwidth;
	u_int32_t  period;
	u_int8_t   method;
#ifdef WLAN_FEATURE_11BE
	uint32_t   puncture_bitmap;
#endif
};

/**
 * struct psoc_cfr - private psoc object for cfr
 * psoc_obj: pointer to psoc object
 * is_cfr_capable: flag to determine if cfr is enabled or not
 * is_cap_interval_mode_sel_support: flag to determine if target supports both
 *				     capture_count and capture_duration modes
 *				     with a nob provided to configure
 * is_mo_marking_support: flag to determine if MO marking is supported or not
 */
struct psoc_cfr {
	struct wlan_objmgr_psoc *psoc_obj;
	uint8_t is_cfr_capable;
#ifdef WLAN_ENH_CFR_ENABLE
	uint8_t is_cap_interval_mode_sel_support;
	uint8_t is_mo_marking_support;
	uint8_t is_aoa_for_rcc_support;
#endif
};

/**
 * struct cfr_wmi_host_mem_chunk - wmi mem chunk related
 * vaddr: pointer to virtual address
 * paddr: physical address
 * len: len of the mem chunk allocated
 * req_id: reqid related to the mem chunk
 */
struct cfr_wmi_host_mem_chunk {
	uint32_t *vaddr;
	qdf_dma_addr_t paddr;
	uint32_t len;
	uint32_t req_id;
};

struct whal_cfir_dma_hdr {
	uint16_t
		// 'BA'
		tag                 : 8,
		// '02', length of header in 4 octet units
		length              : 6,
		// 00
		reserved            : 2;
	uint16_t
		// [16]
		upload_done         : 1,
		// [17:18], 0: invalid, 1: CFR, 2: CIR, 3: DebugH
		capture_type        : 3,
		// [19:20], 0: Legacy, 1: HT, 2: VHT, 3: HE
		preamble_type       : 2,
		// [21:23], 0: 1-stream, 1: 2-stream, ..., 7: 8-stream
		nss                 : 3,
		// [24:27], 0: invalid, 1: 1-chain, 2: 2-chain, etc.
		num_chains          : 3,
		// [28:30], 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 MHz
		upload_pkt_bw       : 3,    // [31]
		sw_peer_id_valid    : 1;
	uint16_t
		sw_peer_id          : 16;   // [15:0]
	uint16_t
		phy_ppdu_id         : 16;   // [15:0]
};

#define MAX_LUT_ENTRIES 140 /* For HKv2 136 is max */

/**
 * struct look_up_table - Placeholder for 2 asynchronous events (DBR and
 * TXRX event)
 * dbr_recv: Indicates whether WMI for DBR completion is received or not
 * tx_recv: Indicates whether WMI for TX completion (or) WDI event for RX
 * status is received or not
 * data: pointer to CFR data that ucode DMAs to host memory
 * data_len: length of CFR data DMAed by ucode
 * dbr_ppdu_id: PPDU id retrieved from DBR completion WMI event
 * tx_ppdu_id: PPDU id retrieved from WMI TX completion event (or) PPDU status
 * TLV
 * dbr_address: Physical address of the CFR data dump retrieved from DBR
 * completion WMI event
 * tx_address1: Physical address of the CFR data from TX/RX event
 * tx_address2: Physical address of the CFR data from TX/RX event
 * csi_cfr_header: CFR header constructed by host
 * whal_cfir_enhanced_hdr: CFR header constructed by ucode
 * tx_tstamp: Timestamp when TX/RX event was received
 * dbr_tstamp: Timestamp when DBR completion event was received
 * header_length: Length of header DMAed by ucode in words
 * payload_length: Length of CFR payload
 */
struct look_up_table {
	bool dbr_recv;
	bool tx_recv;
	uint8_t *data; /* capture payload */
	uint32_t data_len; /* capture len */
	uint16_t dbr_ppdu_id; /* ppdu id from dbr */
	uint16_t tx_ppdu_id; /* ppdu id from TX event */
	qdf_dma_addr_t dbr_address; /* capture len */
	uint32_t tx_address1; /* capture len */
	uint32_t tx_address2; /* capture len */
	struct csi_cfr_header header;
	struct whal_cfir_dma_hdr dma_hdr;
	uint64_t txrx_tstamp;
	uint64_t dbr_tstamp;
	uint32_t header_length;
	uint32_t payload_length;
};

struct unassoc_pool_entry {
	struct qdf_mac_addr mac;
	struct cfr_capture_params cfr_params;
	bool is_valid;
};

#ifdef WLAN_ENH_CFR_ENABLE
/**
 * struct ta_ra_cfr_cfg - structure to store configuration of 16 groups in
 * M_TA_RA mode
 * filter_group_id: Filter group number for which the below filters needs to be
 * applied
 * bw: CFR capture will be done for packets matching the bandwidths specified
 * within this bitmask
 * nss: CFR capture will be done for packets matching the Nss specified within
 * this bitmask
 * valid_ta: Ta_addr is valid if set
 * valid_ta_mask: Ta_addr_mask is valid if set
 * valid_ra: Ra_addr is valid if set
 * valid_ra_mask: Ra_addr_mask is valid if set
 * valid_bw_mask: Bandwidth is valid if set
 * valid_nss_mask: NSS is valid if set
 * valid_mgmt_subtype: Mgmt_subtype is valid if set
 * valid_ctrl_subtype: Ctrl_subtype is valid if set
 * valid_data_subtype: Data_subtype is valid if set
 * mgmt_subtype_filter: Managments Packets matching the subtype filter
 * categories will be filtered in by MAC for CFR capture.
 * ctrl_subtype_filter: Control Packets matching the subtype filter
 * categories will be filtered in by MAC for CFR capture.
 * data_subtype_filter: Data Packets matching the subtype filter
 * categories will be filtered in by MAC for CFR capture.
 * tx_addr: Packets whose transmitter address matches (tx_addr & tx_addr_mask)
 * will be filtered in by MAC
 * tx_addr_mask: Packets whose transmitter address matches (tx_addr &
 * tx_addr_mask) will be filtered in by MAC
 * rx_addr: Packets whose receiver address matches (rx_addr & rx_addr_mask)
 * will be filtered in by MAC
 * rx_addr_mask: Packets whose receiver address matches (rx_addr &
 * rx_addr_mask) will be filtered in by MAC
 */
struct ta_ra_cfr_cfg {
	uint8_t filter_group_id;
	uint16_t bw                          :6,
		 nss                         :8,
		 rsvd0                       :2;
	uint16_t valid_ta                    :1,
		 valid_ta_mask               :1,
		 valid_ra                    :1,
		 valid_ra_mask               :1,
		 valid_bw_mask               :1,
		 valid_nss_mask              :1,
		 valid_mgmt_subtype          :1,
		 valid_ctrl_subtype          :1,
		 valid_data_subtype          :1,
		 rsvd1                       :7;
	uint16_t mgmt_subtype_filter;
	uint16_t ctrl_subtype_filter;
	uint16_t data_subtype_filter;
	uint8_t tx_addr[QDF_MAC_ADDR_SIZE];
	uint8_t rx_addr[QDF_MAC_ADDR_SIZE];
	uint8_t tx_addr_mask[QDF_MAC_ADDR_SIZE];
	uint8_t rx_addr_mask[QDF_MAC_ADDR_SIZE];

} qdf_packed;

/**
 * struct cfr_rcc_param - structure to store cfr config param
 * pdev_id: pdev_id for identifying the MAC
 * vdev_id: vdev_id of current rcc configures
 * srng_id: srng id of current rcc configures
 * capture_duration: Capture Duration field for which CFR capture has to happen,
 * in microsecond units
 * capture_interval: Capture interval field which is time in between
 * consecutive CFR capture, in microsecond units
 * ul_mu_user_mask_lower: Bitfields indicates which of the users in the current
 * UL MU tranmission are enabled for CFR capture.
 * ul_mu_user_mask_upper: This is contiuation of the above lower mask.
 * freeze_tlv_delay_cnt_en: Enable Freeze TLV delay counter in MAC
 * freeze_tlv_delay_cnt_thr: Indicates the number of consecutive Rx packets to
 * be skipped before CFR capture is enabled again.
 * filter_group_bitmap: Bitfields set indicates which of the CFR group config is
 * enabled
 * m_directed_ftm: Filter Directed FTM ACK frames for CFR capture
 * m_all_ftm_ack: Filter All FTM ACK frames for CFR capture
 * m_ndpa_ndp_directed: Filter NDPA NDP Directed Frames for CFR capture
 * m_ndpa_ndp_all: Filter all NDPA NDP for CFR capture
 * m_ta_ra_filter: Filter Frames based on TA/RA/Subtype as provided in CFR Group
 * config
 * m_all_packet: Filter in All packets for CFR Capture
 * en_ta_ra_filter_in_as_fp: Filter in frames as FP/MO in m_ta_ra_filter mode
 * num_grp_tlvs: Indicates the number of groups in M_TA_RA mode, that have
 * changes in the current commit session, use to construct WMI group TLV(s)
 * curr: Placeholder for M_TA_RA group config in current commit session
 * modified_in_curr_session: Bitmap indicating number of groups in M_TA_RA mode
 * that have changed in current commit session.
 * capture_count: After capture_count+1 number of captures, MAC stops RCC  and
 * waits for capture_interval duration before enabling again
 * capture_intval_mode_sel: 0 indicates capture_duration mode, 1 indicates the
 * capture_count mode.
 */
struct cfr_rcc_param {
	uint8_t pdev_id;
	uint8_t vdev_id;
	uint8_t srng_id;
	uint32_t capture_duration;
	uint32_t capture_interval;
	uint32_t ul_mu_user_mask_lower;
	uint32_t ul_mu_user_mask_upper;
	uint16_t freeze_tlv_delay_cnt_en  :1,
		 freeze_tlv_delay_cnt_thr :8,
		 rsvd0 :7;
	uint16_t filter_group_bitmap;
	uint8_t m_directed_ftm           : 1,
		m_all_ftm_ack            : 1,
		m_ndpa_ndp_directed      : 1,
		m_ndpa_ndp_all           : 1,
		m_ta_ra_filter           : 1,
		m_all_packet             : 1,
		en_ta_ra_filter_in_as_fp : 1,
		rsvd1                    : 1;
	uint8_t num_grp_tlvs;

	struct ta_ra_cfr_cfg curr[MAX_TA_RA_ENTRIES];
	unsigned long modified_in_curr_session;
	uint32_t capture_count            :16,
		 capture_intval_mode_sel  :1,
		 rsvd2                    :15;
};
#endif /* WLAN_ENH_CFR_ENABLE */

/**
 * struct nl_event_cb - nl event cb for cfr data
 * vdev_id: vdev id
 * pid: PID to which data is sent via unicast nl evnet
 * cfr_nl_cb: callback to send nl evnet
 */
struct nl_event_cb {
	uint8_t vdev_id;
	uint32_t pid;
	void (*cfr_nl_cb)(uint8_t vdev_id, uint32_t pid,
			  const void *data, uint32_t data_len);
};

/**
 * struct pdev_cfr - private pdev object for cfr
 * pdev_obj: pointer to pdev object
 * is_cfr_capable: flag to determine if cfr is enabled or not
 * cfr_timer_enable: flag to enable/disable timer
 * chip_type: chip type which is defined in enum cfrradiotype
 * cfr_mem_chunk: Region of memory used for storing cfr data
 * cfr_max_sta_count: Maximum stations supported in one-shot capture mode
 * num_subbufs: No. of sub-buffers used in relayfs
 * subbuf_size: Size of sub-buffer used in relayfs
 * chan_ptr: Channel in relayfs
 * dir_ptr: Parent directory of relayfs file
 * lut: lookup table used to store asynchronous DBR and TX/RX events for
 * correlation
 * lut_num: Number of lut
 * dbr_buf_size: Size of DBR completion buffer
 * dbr_num_bufs: No. of DBR completions
 * tx_evt_cnt: No. of TX completion events till CFR stop was issued
 * total_tx_evt_cnt: No. of Tx completion events since wifi was up
 * dbr_evt_cnt: No. of WMI DBR completion events
 * release_cnt: No. of CFR data buffers relayed to userspace
 * tx_peer_status_cfr_fail: No. of tx events without tx status set to
 * PEER_CFR_CAPTURE_EVT_STATUS_MASK indicating CFR capture failure on a peer.
 * tx_evt_status_cfr_fail: No. of tx events without tx status set to
 * CFR_TX_EVT_STATUS_MASK indicating CFR capture status failure.
 * tx_dbr_cookie_lookup_fail: No. of dbr cookie lookup failures during tx event
 * process.
 * rcc_param: Structure to store CFR config for the current commit session
 * global: Structure to store accumulated CFR config
 * rx_tlv_evt_cnt: Number of CFR WDI events from datapath
 * lut_age_timer: Timer to flush pending TXRX/DBR events in lookup table
 * lut_timer_init: flag to determine if lut_age_timer is initialized or not
 * is_cfr_rcc_capable: Flag to determine if RCC is enabled or not.
 * flush_dbr_cnt: No. of un-correlated DBR completions flushed when a newer PPDU
 * is correlated successfully with newer DBR completion
 * invalid_dma_length_cnt: No. of buffers for which CFR DMA header length (or)
 * data length was invalid
 * flush_timeout_dbr_cnt: No. of DBR completion flushed out in ageout logic
 * clear_txrx_event: No. of PPDU status TLVs over-written in LUT
 * last_success_tstamp: DBR timestamp which indicates that both DBR and TX/RX
 * events have been received successfully.
 * cfr_dma_aborts: No. of CFR DMA aborts in ucode
 * is_cap_interval_mode_sel_support: flag to determine if target supports both
 * is_mo_marking_support: flag to determine if MO marking is supported or not
 * is_aoa_for_rcc_support: flag to determine if AoA is available for RCC or not
 * capture_count and capture_duration modes with a nob provided to configure.
 * unassoc_pool: Pool of un-associated clients used when capture method is
 * CFR_CAPTURE_METHOD_PROBE_RESPONSE
 * nl_cb: call back to register for nl event for cfr data
 * lut_lock: Lock to protect access to cfr lookup table
 * is_prevent_suspend: CFR wake lock acquired or not
 * wake_lock: wake lock for cfr
 * runtime_lock: runtime lock for cfr
 * freq: current operating freq for which AoA Phase delta values reported by FW
 * max_aoa_chains: Indicate the max number of chains to which target supports
 * AoA data.
 * phase_delta: per chain phase delta associated with 62 gain values reported by
 * FW via WMI_PDEV_AOA_PHASEDELTA_EVENTID.
 * ibf_cal_val: Per chain IBF cal value from FW.
 */
/*
 * To be extended if we get more capbality info
 * from FW's extended service ready event.
 */
struct pdev_cfr {
	struct wlan_objmgr_pdev *pdev_obj;
	uint8_t is_cfr_capable;
	uint8_t cfr_timer_enable;
	uint8_t chip_type;
	struct cfr_wmi_host_mem_chunk cfr_mem_chunk;
	uint16_t cfr_max_sta_count;
	uint16_t cfr_current_sta_count;
	uint32_t num_subbufs;
	uint32_t subbuf_size;
	qdf_streamfs_chan_t chan_ptr;
	qdf_dentry_t dir_ptr;
	struct look_up_table **lut;
	uint32_t lut_num;
	uint32_t dbr_buf_size;
	uint32_t dbr_num_bufs;
	uint32_t max_mu_users;
	uint64_t tx_evt_cnt;
	uint64_t total_tx_evt_cnt;
	uint64_t dbr_evt_cnt;
	uint64_t release_cnt;
	uint64_t tx_peer_status_cfr_fail;
	uint64_t tx_evt_status_cfr_fail;
	uint64_t tx_dbr_cookie_lookup_fail;
#ifdef WLAN_ENH_CFR_ENABLE
	struct cfr_rcc_param rcc_param;
	struct ta_ra_cfr_cfg global[MAX_TA_RA_ENTRIES];
	uint64_t rx_tlv_evt_cnt;
	qdf_timer_t lut_age_timer;
	uint8_t lut_timer_init;
	uint8_t is_cfr_rcc_capable;
	uint64_t flush_dbr_cnt;
	uint64_t invalid_dma_length_cnt;
	uint64_t flush_timeout_dbr_cnt;
	uint64_t clear_txrx_event;
	uint64_t last_success_tstamp;
	uint64_t cfr_dma_aborts;
	uint8_t is_cap_interval_mode_sel_support;
	uint8_t is_mo_marking_support;
	uint8_t is_aoa_for_rcc_support;
#endif
	struct unassoc_pool_entry unassoc_pool[MAX_CFR_ENABLED_CLIENTS];
	struct nl_event_cb nl_cb;
	qdf_spinlock_t lut_lock;
#ifdef WLAN_CFR_PM
	bool is_prevent_suspend;
	qdf_wake_lock_t wake_lock;
	qdf_runtime_lock_t runtime_lock;
#endif
#ifdef WLAN_ENH_CFR_ENABLE
	uint32_t freq;
	uint32_t max_aoa_chains;
	uint16_t phase_delta[HOST_MAX_CHAINS][MAX_AGC_GAIN];
	uint32_t ibf_cal_val[HOST_MAX_CHAINS];
#endif
};

/**
 * enum cfr_capt_status - CFR capture status
 */
enum cfr_capt_status {
	/* Capture not in progress */
	PEER_CFR_CAPTURE_DISABLE,
	/* Capture in progress */
	PEER_CFR_CAPTURE_ENABLE,
};

/**
 * struct peer_cfr - private peer object for cfr
 * peer_obj: pointer to peer_obj
 * request: Type of request (start/stop)
 * bandwidth: bandwitdth of capture for this peer
 * capture_method: enum determining type of cfr data capture.
 *                 0-Qos null data
 */
struct peer_cfr {
	struct wlan_objmgr_peer *peer_obj;
	u_int8_t   request;            /* start/stop */
	u_int8_t   bandwidth;
	u_int32_t  period;
	u_int8_t   capture_method;
#ifdef WLAN_FEATURE_11BE
	uint32_t   puncture_bitmap;
#endif
};

/**
 * cfr_initialize_pdev() - cfr initialize pdev
 * @pdev: Pointer to pdev_obj
 *
 * Return: status of cfr pdev init
 */
QDF_STATUS cfr_initialize_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * cfr_deinitialize_pdev() - cfr deinitialize pdev
 * @pdev: Pointer to pdev_obj
 *
 * Return: status of cfr pdev deinit
 */
QDF_STATUS cfr_deinitialize_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfr_init() - Global init for cfr.
 *
 * Return: status of global init pass/fail
 */
QDF_STATUS wlan_cfr_init(void);

/**
 * wlan_cfr_deinit() - Global de-init for cfr.
 *
 * Return: status of global de-init pass/fail
 */
QDF_STATUS wlan_cfr_deinit(void);

/**
 * wlan_cfr_pdev_open() - pdev_open function for cfr.
 * @pdev: pointer to pdev object
 *
 * Return: status of pdev_open pass/fail
 */
QDF_STATUS wlan_cfr_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cfr_pdev_close() - pdev_close function for cfr.
 * @pdev: pointer to pdev object
 *
 * Return: status of pdev_close pass/fail
 */
QDF_STATUS wlan_cfr_pdev_close(struct wlan_objmgr_pdev *pdev);

/**
 * count_set_bits() - function to count set bits in a bitmap
 * @value: input bitmap
 *
 * Return: No. of set bits
 */
uint8_t count_set_bits(unsigned long value);

/**
 * wlan_cfr_is_feature_disabled() - Check if cfr feature is disabled
 * @pdev - the physical device object.
 *
 * Return : true if cfr is disabled, else false.
 */
bool wlan_cfr_is_feature_disabled(struct wlan_objmgr_pdev *pdev);

#ifdef WLAN_ENH_CFR_ENABLE
/**
 * wlan_cfr_rx_tlv_process() - Process PPDU status TLVs and store info in
 * lookup table
 * @pdev_obj: PDEV object
 * @nbuf: ppdu info
 *
 * Return: none
 */
void wlan_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf);
#endif
#endif
