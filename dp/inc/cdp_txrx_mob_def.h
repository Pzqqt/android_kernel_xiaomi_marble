/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#ifndef __CDP_TXRX_MOB_DEF_H
#define __CDP_TXRX_MOB_DEF_H
#include <sir_types.h>
#include <htt.h>

#define TX_WMM_AC_NUM	4
#define ENABLE_DP_HIST_STATS
#define DP_MEMORY_OPT
#ifndef CONFIG_BERYLLIUM
#define DP_USE_SINGLE_TCL
#endif

#define DP_RX_DISABLE_NDI_MDNS_FORWARDING

#define OL_TXQ_PAUSE_REASON_FW                (1 << 0)
#define OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED (1 << 1)
#define OL_TXQ_PAUSE_REASON_TX_ABORT          (1 << 2)
#define OL_TXQ_PAUSE_REASON_VDEV_STOP         (1 << 3)
#define OL_TXQ_PAUSE_REASON_THERMAL_MITIGATION (1 << 4)

#define OL_TXRX_INVALID_NUM_PEERS (-1)


/* Maximum number of station supported by data path, including BC. */
#define WLAN_MAX_STA_COUNT  (HAL_NUM_STA)

/* The symbolic station ID return to HDD to specify the packet is bc/mc */
#define WLAN_RX_BCMC_STA_ID (WLAN_MAX_STA_COUNT + 1)

/* The symbolic station ID return to HDD to specify the packet is
       to soft-AP itself */
#define WLAN_RX_SAP_SELF_STA_ID (WLAN_MAX_STA_COUNT + 2)

/* is 802.11 address multicast/broadcast? */
#define IEEE80211_IS_MULTICAST(_a)  (*(_a) & 0x01)

#define MAX_PEERS 32

/*
 * Bins used for reporting delay histogram:
 * bin 0:  0 - 10  ms delay
 * bin 1: 10 - 20  ms delay
 * bin 2: 20 - 40  ms delay
 * bin 3: 40 - 80  ms delay
 * bin 4: 80 - 160 ms delay
 * bin 5: > 160 ms delay
 */
#define QCA_TX_DELAY_HIST_REPORT_BINS 6

/* BA actions */
#define IEEE80211_ACTION_BA_ADDBA_REQUEST       0       /* ADDBA request */
#define IEEE80211_ACTION_BA_ADDBA_RESPONSE      1       /* ADDBA response */
#define IEEE80211_ACTION_BA_DELBA               2       /* DELBA */

#define IEEE80211_BA_POLICY_DELAYED      0
#define IEEE80211_BA_POLICY_IMMEDIATE    1
#define IEEE80211_BA_AMSDU_SUPPORTED     1

/**
 * enum netif_action_type - Type of actions on netif queues
 * @WLAN_STOP_ALL_NETIF_QUEUE: stop all netif queues
 * @WLAN_START_ALL_NETIF_QUEUE: start all netif queues
 * @WLAN_WAKE_ALL_NETIF_QUEUE: wake all netif queues
 * @WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER: stop all queues and off carrier
 * @WLAN_START_ALL_NETIF_QUEUE_N_CARRIER: start all queues and on carrier
 * @WLAN_NETIF_TX_DISABLE: disable tx
 * @WLAN_NETIF_TX_DISABLE_N_CARRIER: disable tx and off carrier
 * @WLAN_NETIF_CARRIER_ON: on carrier
 * @WLAN_NETIF_CARRIER_OFF: off carrier
 * @WLAN_NETIF_PRIORITY_QUEUE_ON: start priority netif queues
 * @WLAN_NETIF_PRIORITY_QUEUE_OFF: stop priority netif queues
 * @WLAN_WAKE_NON_PRIORITY_QUEUE: wake non priority netif queues
 * @WLAN_STOP_NON_PRIORITY_QUEUE: stop non priority netif queues
 */
enum netif_action_type {
	WLAN_NETIF_ACTION_TYPE_NONE = 0,
	WLAN_STOP_ALL_NETIF_QUEUE = 1,
	WLAN_START_ALL_NETIF_QUEUE = 2,
	WLAN_WAKE_ALL_NETIF_QUEUE = 3,
	WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER = 4,
	WLAN_START_ALL_NETIF_QUEUE_N_CARRIER = 5,
	WLAN_NETIF_TX_DISABLE = 6,
	WLAN_NETIF_TX_DISABLE_N_CARRIER = 7,
	WLAN_NETIF_CARRIER_ON = 8,
	WLAN_NETIF_CARRIER_OFF = 9,
	WLAN_NETIF_PRIORITY_QUEUE_ON = 10,
	WLAN_NETIF_PRIORITY_QUEUE_OFF = 11,
	WLAN_NETIF_VO_QUEUE_ON = 12,
	WLAN_NETIF_VO_QUEUE_OFF = 13,
	WLAN_NETIF_VI_QUEUE_ON = 14,
	WLAN_NETIF_VI_QUEUE_OFF = 15,
	WLAN_NETIF_BE_BK_QUEUE_OFF = 16,
	WLAN_NETIF_BE_BK_QUEUE_ON = 17,
	WLAN_WAKE_NON_PRIORITY_QUEUE = 18,
	WLAN_STOP_NON_PRIORITY_QUEUE = 19,
	WLAN_NETIF_ACTION_TYPE_MAX,
};

/**
 * enum netif_reason_type - reason for netif queue action
 * @WLAN_CONTROL_PATH: action from control path
 * @WLAN_DATA_FLOW_CONTROL: because of flow control
 * @WLAN_FW_PAUSE: because of firmware pause
 * @WLAN_TX_ABORT: because of tx abort
 * @WLAN_VDEV_STOP: because of vdev stop
 * @WLAN_PEER_UNAUTHORISED: because of peer is unauthorised
 * @WLAN_THERMAL_MITIGATION: because of thermal mitigation
 */
enum netif_reason_type {
	WLAN_CONTROL_PATH = 1,
	WLAN_DATA_FLOW_CONTROL,
	WLAN_FW_PAUSE,
	WLAN_TX_ABORT,
	WLAN_VDEV_STOP,
	WLAN_PEER_UNAUTHORISED,
	WLAN_THERMAL_MITIGATION,
	WLAN_DATA_FLOW_CONTROL_PRIORITY,
	WLAN_DATA_FLOW_CTRL_BE_BK,
	WLAN_DATA_FLOW_CTRL_VI,
	WLAN_DATA_FLOW_CTRL_VO,
	WLAN_DATA_FLOW_CTRL_PRI,
	WLAN_REASON_TYPE_MAX,
};

enum ol_rx_err_type {
	OL_RX_ERR_DEFRAG_MIC,
	OL_RX_ERR_PN,
	OL_RX_ERR_UNKNOWN_PEER,
	OL_RX_ERR_MALFORMED,
	OL_RX_ERR_TKIP_MIC,
	OL_RX_ERR_DECRYPT,
	OL_RX_ERR_MPDU_LENGTH,
	OL_RX_ERR_ENCRYPT_REQUIRED,
	OL_RX_ERR_DUP,
	OL_RX_ERR_UNKNOWN,
	OL_RX_ERR_FCS,
	OL_RX_ERR_PRIVACY,
	OL_RX_ERR_NONE_FRAG,
	OL_RX_ERR_NONE = 0xFF
};

enum throttle_level {
	THROTTLE_LEVEL_0,
	THROTTLE_LEVEL_1,
	THROTTLE_LEVEL_2,
	THROTTLE_LEVEL_3,
	/* Invalid */
	THROTTLE_LEVEL_MAX,
};

enum {
	OL_TX_WMM_AC_BE,
	OL_TX_WMM_AC_BK,
	OL_TX_WMM_AC_VI,
	OL_TX_WMM_AC_VO,
	OL_TX_NUM_WMM_AC
};

/**
 * @enum ol_tx_spec
 * @brief indicate what non-standard transmission actions to apply
 * @details
 *  Indicate one or more of the following:
 *    - The tx frame already has a complete 802.11 header.
 *      Thus, skip 802.3/native-WiFi to 802.11 header encapsulation and
 *      A-MSDU aggregation.
 *    - The tx frame should not be aggregated (A-MPDU or A-MSDU)
 *    - The tx frame is already encrypted - don't attempt encryption.
 *    - The tx frame is a segment of a TCP jumbo frame.
 *    - This tx frame should not be unmapped and freed by the txrx layer
 *      after transmission, but instead given to a registered tx completion
 *      callback.
 *  More than one of these specification can apply, though typically
 *  only a single specification is applied to a tx frame.
 *  A compound specification can be created, as a bit-OR of these
 *  specifications.
 */
enum ol_tx_spec {
	OL_TX_SPEC_STD = 0x0,   /* do regular processing */
	OL_TX_SPEC_RAW = 0x1,   /* skip encap + A-MSDU aggr */
	OL_TX_SPEC_NO_AGGR = 0x2,       /* skip encap + all aggr */
	OL_TX_SPEC_NO_ENCRYPT = 0x4,    /* skip encap + encrypt */
	OL_TX_SPEC_TSO = 0x8,   /* TCP segmented */
	OL_TX_SPEC_NWIFI_NO_ENCRYPT = 0x10,     /* skip encrypt for nwifi */
	OL_TX_SPEC_NO_FREE = 0x20,      /* give to cb rather than free */
};

/**
 * @enum peer_debug_id_type: debug ids to track peer get_ref and release_ref
 * @brief Unique peer debug IDs to track the callers. Each new usage can add to
 *        this enum list to create a new "PEER_DEBUG_ID_".
 * @PEER_DEBUG_ID_OL_INTERNAL: debug id for OL internal usage
 * @PEER_DEBUG_ID_WMA_PKT_DROP: debug id for wma_is_pkt_drop_candidate API
 * @PEER_DEBUG_ID_WMA_ADDBA_REQ: debug id for ADDBA request
 * @PEER_DEBUG_ID_WMA_DELBA_REQ: debug id for DELBA request
 * @PEER_DEBUG_ID_LIM_SEND_ADDBA_RESP: debug id for send ADDBA response
 * @PEER_DEBUG_ID_OL_RX_THREAD: debug id for rx thread
 * @PEER_DEBUG_ID_WMA_CCMP_REPLAY_ATTACK: debug id for CCMP replay
 * @PEER_DEBUG_ID_WMA_DEL_BSS:debug id for remove BSS
 * @PEER_DEBUG_ID_WMA_VDEV_STOP_RESP:debug id for vdev stop response handler
 * @PEER_DEBUG_ID_OL_PEER_MAP:debug id for peer map/unmap
 * @PEER_DEBUG_ID_OL_PEER_ATTACH: debug id for peer attach/detach
 * @PEER_DEBUG_ID_OL_TXQ_VDEV_FL: debug id for vdev flush
 * @PEER_DEBUG_ID_OL_HASH_ERS:debug id for peer find hash erase
 * @PEER_DEBUG_ID_MAX: debug id MAX
 */
enum peer_debug_id_type {
	PEER_DEBUG_ID_OL_INTERNAL,
	PEER_DEBUG_ID_WMA_PKT_DROP,
	PEER_DEBUG_ID_WMA_ADDBA_REQ,
	PEER_DEBUG_ID_WMA_DELBA_REQ,
	PEER_DEBUG_ID_LIM_SEND_ADDBA_RESP,
	PEER_DEBUG_ID_OL_RX_THREAD,
	PEER_DEBUG_ID_WMA_CCMP_REPLAY_ATTACK,
	PEER_DEBUG_ID_WMA_DEL_BSS,
	PEER_DEBUG_ID_WMA_VDEV_STOP_RESP,
	PEER_DEBUG_ID_OL_PEER_MAP,
	PEER_DEBUG_ID_OL_PEER_ATTACH,
	PEER_DEBUG_ID_OL_TXQ_VDEV_FL,
	PEER_DEBUG_ID_OL_HASH_ERS,
	PEER_DEBUG_ID_OL_UNMAP_TIMER_WORK,
	PEER_DEBUG_ID_MAX
};

/**
 * struct ol_txrx_desc_type - txrx descriptor type
 * @is_qos_enabled: is station qos enabled
 * @is_wapi_supported: is station wapi supported
 * @peer_addr: peer mac address
 */
struct ol_txrx_desc_type {
	uint8_t is_qos_enabled;
	uint8_t is_wapi_supported;
	struct qdf_mac_addr peer_addr;
};

/**
 * struct ol_tx_sched_wrr_ac_specs_t - the wrr ac specs params structure
 * @wrr_skip_weight: map to ol_tx_sched_wrr_adv_category_info_t.specs.
 *                            wrr_skip_weight
 * @credit_threshold: map to ol_tx_sched_wrr_adv_category_info_t.specs.
 *                            credit_threshold
 * @send_limit: map to ol_tx_sched_wrr_adv_category_info_t.specs.
 *                            send_limit
 * @credit_reserve: map to ol_tx_sched_wrr_adv_category_info_t.specs.
 *                            credit_reserve
 * @discard_weight: map to ol_tx_sched_wrr_adv_category_info_t.specs.
 *                            discard_weight
 *
 * This structure is for wrr ac specs params set from user, it will update
 * its content corresponding to the ol_tx_sched_wrr_adv_category_info_t.specs.
 */
struct ol_tx_sched_wrr_ac_specs_t {
	int wrr_skip_weight;
	uint32_t credit_threshold;
	uint16_t send_limit;
	int credit_reserve;
	int discard_weight;
};

/**
 * struct txrx_pdev_cfg_param_t - configuration information
 * passed to the data path
 */
struct txrx_pdev_cfg_param_t {
	uint8_t is_full_reorder_offload;
	/* IPA Micro controller data path offload enable flag */
	uint8_t is_uc_offload_enabled;
	/* IPA Micro controller data path offload TX buffer count */
	uint32_t uc_tx_buffer_count;
	/* IPA Micro controller data path offload TX buffer size */
	uint32_t uc_tx_buffer_size;
	/* IPA Micro controller data path offload RX indication ring count */
	uint32_t uc_rx_indication_ring_count;
	/* IPA Micro controller data path offload TX partition base */
	uint32_t uc_tx_partition_base;
	/* IP, TCP and UDP checksum offload */
	bool ip_tcp_udp_checksum_offload;
	/* IP, TCP and UDP checksum offload for NAN Mode */
	bool nan_ip_tcp_udp_checksum_offload;
	/* IP, TCP and UDP checksum offload for P2P Mode*/
	bool p2p_ip_tcp_udp_checksum_offload;
	/* Checksum offload override flag for Legcay modes */
	bool legacy_mode_csum_disable;
	/* Rx processing in thread from TXRX */
	bool enable_rxthread;
	/* CE classification enabled through INI */
	bool ce_classify_enabled;
#if defined(QCA_LL_TX_FLOW_CONTROL_V2) || defined(QCA_LL_PDEV_TX_FLOW_CONTROL)
	/* Threshold to stop queue in percentage */
	uint32_t tx_flow_stop_queue_th;
	/* Start queue offset in percentage */
	uint32_t tx_flow_start_queue_offset;
#endif

#ifdef QCA_SUPPORT_TXRX_DRIVER_TCP_DEL_ACK
	/* enable the tcp delay ack feature in the driver */
	bool  del_ack_enable;
	/* timeout if no more tcp ack frames, unit is ms */
	uint16_t del_ack_timer_value;
	/* the maximum number of replaced tcp ack frames */
	uint16_t del_ack_pkt_count;
#endif

	struct ol_tx_sched_wrr_ac_specs_t ac_specs[TX_WMM_AC_NUM];
	bool gro_enable;
	bool tso_enable;
	bool lro_enable;
	bool sg_enable;
	bool enable_data_stall_detection;
	bool enable_flow_steering;
	bool disable_intra_bss_fwd;

#ifdef WLAN_SUPPORT_TXRX_HL_BUNDLE
	uint16_t bundle_timer_value;
	uint16_t bundle_size;
#endif
	uint8_t pktlog_buffer_size;
};

#ifdef IPA_OFFLOAD
/**
 * ol_txrx_ipa_resources - Resources needed for IPA
 */
struct ol_txrx_ipa_resources {
	qdf_shared_mem_t *ce_sr;
	uint32_t ce_sr_ring_size;
	qdf_dma_addr_t ce_reg_paddr;

	qdf_shared_mem_t *tx_comp_ring;
	uint32_t tx_num_alloc_buffer;

	qdf_shared_mem_t *rx_rdy_ring;
	qdf_shared_mem_t *rx_proc_done_idx;

	qdf_shared_mem_t *rx2_rdy_ring;
	qdf_shared_mem_t *rx2_proc_done_idx;

	/* IPA UC doorbell registers paddr */
	qdf_dma_addr_t tx_comp_doorbell_dmaaddr;
	qdf_dma_addr_t rx_ready_doorbell_dmaaddr;

	uint32_t tx_pipe_handle;
	uint32_t rx_pipe_handle;
};
#endif

struct ol_txrx_ocb_chan_info {
	uint32_t chan_freq;
	uint16_t disable_rx_stats_hdr:1;
};

/**
 * ol_mic_error_info - carries the information associated with
 * a MIC error
 * @vdev_id: virtual device ID
 * @key_id: Key ID
 * @pn: packet number
 * @sa: source address
 * @da: destination address
 * @ta: transmitter address
 */
struct ol_mic_error_info {
	uint8_t vdev_id;
	uint32_t key_id;
	uint64_t pn;
	uint8_t sa[QDF_MAC_ADDR_SIZE];
	uint8_t da[QDF_MAC_ADDR_SIZE];
	uint8_t ta[QDF_MAC_ADDR_SIZE];
};

/**
 * ol_error_info - carries the information associated with an
 * error indicated by the firmware
 * @mic_err: MIC error information
 */
struct ol_error_info {
	union {
		struct ol_mic_error_info mic_err;
	} u;
};


/**
 * struct ol_txrx_ocb_set_chan - txrx OCB channel info
 * @ocb_channel_count: Channel count
 * @ocb_channel_info: OCB channel info
 */
struct ol_txrx_ocb_set_chan {
	uint32_t ocb_channel_count;
	struct ol_txrx_ocb_chan_info *ocb_channel_info;
};

/**
 * @brief Parameter type to pass WMM setting to ol_txrx_set_wmm_param
 * @details
 *   The struct is used to specify informaiton to update TX WMM scheduler.
 */
struct ol_tx_ac_param_t {
	uint32_t aifs;
	uint32_t cwmin;
	uint32_t cwmax;
};

struct ol_tx_wmm_param_t {
	struct ol_tx_ac_param_t ac[OL_TX_NUM_WMM_AC];
};

struct ieee80211_ba_parameterset {
#if _BYTE_ORDER == _BIG_ENDIAN
	uint16_t buffersize:10, /* B6-15  buffer size */
		 tid:4,       /* B2-5   TID */
		 bapolicy:1,  /* B1   block ack policy */
		 amsdusupported:1; /* B0   amsdu supported */
#else
	uint16_t amsdusupported:1,      /* B0   amsdu supported */
		 bapolicy:1,  /* B1   block ack policy */
		 tid:4,       /* B2-5   TID */
		 buffersize:10; /* B6-15  buffer size */
#endif
} __packed;

struct ieee80211_ba_seqctrl {
#if _BYTE_ORDER == _BIG_ENDIAN
	uint16_t startseqnum:12,        /* B4-15  starting sequence number */
		 fragnum:4;   /* B0-3  fragment number */
#else
	uint16_t fragnum:4,     /* B0-3  fragment number */
		 startseqnum:12; /* B4-15  starting sequence number */
#endif
} __packed;

struct ieee80211_delba_parameterset {
#if _BYTE_ORDER == _BIG_ENDIAN
	uint16_t tid:4,         /* B12-15  tid */
		 initiator:1, /* B11     initiator */
		 reserved0:11; /* B0-10   reserved */
#else
	uint16_t reserved0:11,  /* B0-10   reserved */
		 initiator:1, /* B11     initiator */
		 tid:4;       /* B12-15  tid */
#endif
} __packed;

/**
 * ol_txrx_vdev_peer_remove_cb - wma_remove_peer callback
 */
typedef void (*ol_txrx_vdev_peer_remove_cb)(void *handle, uint8_t *bssid,
		uint8_t vdev_id, void *peer);

/**
 * @typedef tx_pause_callback
 * @brief OSIF function registered with the data path
 */
typedef void (*tx_pause_callback)(uint8_t vdev_id,
		enum netif_action_type action,
		enum netif_reason_type reason);

typedef void (*ipa_uc_op_cb_type)(uint8_t *op_msg,
			void *osif_ctxt);

/**
 * struct ol_rx_inv_peer_params - rx invalid peer data parameters
 * @vdev_id: Virtual device ID
 * @ra: RX data receiver MAC address
 * @ta: RX data transmitter MAC address
 */
struct ol_rx_inv_peer_params {
	uint8_t vdev_id;
	uint8_t ra[QDF_MAC_ADDR_SIZE];
	uint8_t ta[QDF_MAC_ADDR_SIZE];
};

/**
 * cdp_txrx_ext_stats: dp extended stats
 * tx_msdu_enqueue: tx msdu queued to hw
 * tx_msdu_overflow: tx msdu overflow
 * rx_mpdu_received: rx mpdu processed by hw
 * rx_mpdu_delivered: rx mpdu received from hw
 * rx_mpdu_error: rx mpdu error count
 * rx_mpdu_missed: rx mpdu missed by hw
 */
struct cdp_txrx_ext_stats {
	uint32_t tx_msdu_enqueue;
	uint32_t tx_msdu_overflow;
	uint32_t rx_mpdu_received;
	uint32_t rx_mpdu_delivered;
	uint32_t rx_mpdu_error;
	uint32_t rx_mpdu_missed;
};

#endif /* __CDP_TXRX_MOB_DEF_H */
