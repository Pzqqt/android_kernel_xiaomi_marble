/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef __HDD_TDLS_H
#define __HDD_TDLS_H
/**
 * DOC: wlan_hdd_tdls.h
 * WLAN Host Device Driver TDLS include file
 */

#ifdef FEATURE_WLAN_TDLS

/*
 * Before UpdateTimer expires, we want to timeout discovery response
 * should not be more than 2000.
 */
#define TDLS_DISCOVERY_TIMEOUT_BEFORE_UPDATE     1000

#define TDLS_CTX_MAGIC 0x54444c53       /* "TDLS" */

#define TDLS_MAX_SCAN_SCHEDULE          10
#define TDLS_MAX_SCAN_REJECT            5
#define TDLS_DELAY_SCAN_PER_CONNECTION 100
#define TDLS_MAX_CONNECTED_PEERS_TO_ALLOW_SCAN   1

#define TDLS_IS_CONNECTED(peer)	 \
	((eTDLS_LINK_CONNECTED == (peer)->link_status) || \
	 (eTDLS_LINK_TEARING == (peer)->link_status))

/* Bit mask flag for tdls_option to FW */
#define ENA_TDLS_OFFCHAN      (1 << 0)  /* TDLS Off Channel support */
#define ENA_TDLS_BUFFER_STA   (1 << 1)  /* TDLS Buffer STA support */
#define ENA_TDLS_SLEEP_STA    (1 << 2)  /* TDLS Sleep STA support */
#define TDLS_SEC_OFFCHAN_OFFSET_0        0
#define TDLS_SEC_OFFCHAN_OFFSET_40PLUS   40
#define TDLS_SEC_OFFCHAN_OFFSET_40MINUS  (-40)
#define TDLS_SEC_OFFCHAN_OFFSET_80       80
#define TDLS_SEC_OFFCHAN_OFFSET_160      160

#define TDLS_PEER_LIST_SIZE   256

#define TDLS_CT_MAC_AGE_OUT_TIME (30*60*1000) /* Age out time is 30 mins */

#define EXTTDLS_EVENT_BUF_SIZE (4096)

#define TDLS_CT_MAC_MAX_TABLE_SIZE 8

/**
 * struct tdls_config_params_t - tdls config params
 *
 * @tdls: tdls
 * @tx_period_t: tx period
 * @tx_packet_n: tx packets number
 * @discovery_tries_n: discovery tries
 * @idle_timeout_t: idle traffic time out value
 * @idle_packet_n: idle packet number
 * @rssi_trigger_threshold: rssi trigger threshold
 * @rssi_teardown_threshold: rssi tear down threshold
 * @rssi_delta: rssi delta
 */
typedef struct {
	uint32_t tdls;
	uint32_t tx_period_t;
	uint32_t tx_packet_n;
	uint32_t discovery_tries_n;
	uint32_t idle_timeout_t;
	uint32_t idle_packet_n;
	int32_t rssi_trigger_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
} tdls_config_params_t;

/**
 * struct tdls_scan_context_t - tdls scan context
 *
 * @wiphy: pointer to wiphy structure
 * @dev: pointer to netdev
 * @scan_request: scan request
 * @magic: magic
 * @attempt: attempt
 * @reject: reject
 * @tdls_scan_work: delayed tdls scan work
 */
typedef struct {
	struct wiphy *wiphy;
	struct cfg80211_scan_request *scan_request;
	int magic;
	int attempt;
	int reject;
	struct delayed_work tdls_scan_work;
} tdls_scan_context_t;

/**
 * enum eTDLSSupportMode - tdls support mode
 *
 * @eTDLS_SUPPORT_NOT_ENABLED: tdls support not enabled
 * @eTDLS_SUPPORT_DISABLED: suppress implicit trigger and not
 *			respond to the peer
 * @eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY: suppress implicit trigger,
 *			but respond to the peer
 * @eTDLS_SUPPORT_ENABLED: implicit trigger
 * @eTDLS_SUPPORT_EXTERNAL_CONTROL: External control means implicit
 *     trigger but only to a peer mac configured by user space.
 */
typedef enum {
	eTDLS_SUPPORT_NOT_ENABLED = 0,
	eTDLS_SUPPORT_DISABLED,
	eTDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY,
	eTDLS_SUPPORT_ENABLED,
	eTDLS_SUPPORT_EXTERNAL_CONTROL,
} eTDLSSupportMode;

/**
 * enum tdls_spatial_streams - TDLS spatial streams
 * @TDLS_NSS_1x1_MODE: TDLS tx/rx spatial streams = 1
 * @TDLS_NSS_2x2_MODE: TDLS tx/rx spatial streams = 2
 */
enum tdls_spatial_streams {
	TDLS_NSS_1x1_MODE = 0,
	TDLS_NSS_2x2_MODE = 0xff,
};

/**
 * enum tdls_nss_transition_type - TDLS NSS transition states
 * @TDLS_NSS_TRANSITION_UNKNOWN: default state
 * @TDLS_NSS_TRANSITION_2x2_to_1x1: transition from 2x2 to 1x1 stream
 * @TDLS_NSS_TRANSITION_1x1_to_2x2: transition from 1x1 to 2x2 stream
 */
enum tdls_nss_transition_type {
	TDLS_NSS_TRANSITION_UNKNOWN = 0,
	TDLS_NSS_TRANSITION_2x2_to_1x1,
	TDLS_NSS_TRANSITION_1x1_to_2x2,
};

/**
 * enum tTDLSCapType - tdls capability type
 *
 * @eTDLS_CAP_NOT_SUPPORTED: tdls not supported
 * @eTDLS_CAP_UNKNOWN: unknown capability
 * @eTDLS_CAP_SUPPORTED: tdls capability supported
 */
typedef enum eTDLSCapType {
	eTDLS_CAP_NOT_SUPPORTED = -1,
	eTDLS_CAP_UNKNOWN = 0,
	eTDLS_CAP_SUPPORTED = 1,
} tTDLSCapType;

/**
 * enum tTDLSLinkStatus - tdls link status
 *
 * @eTDLS_LINK_IDLE: tdls link idle
 * @eTDLS_LINK_DISCOVERING: tdls link discovering
 * @eTDLS_LINK_DISCOVERED: tdls link discovered
 * @eTDLS_LINK_CONNECTING: tdls link connecting
 * @eTDLS_LINK_CONNECTED: tdls link connected
 * @eTDLS_LINK_TEARING: tdls link tearing
 */
typedef enum eTDLSLinkStatus {
	eTDLS_LINK_IDLE = 0,
	eTDLS_LINK_DISCOVERING,
	eTDLS_LINK_DISCOVERED,
	eTDLS_LINK_CONNECTING,
	eTDLS_LINK_CONNECTED,
	eTDLS_LINK_TEARING,
} tTDLSLinkStatus;

/**
 * enum tdls_teardown_reason - Reason for TDLS teardown
 * @eTDLS_TEARDOWN_EXT_CTRL: Reason ext ctrl.
 * @eTDLS_TEARDOWN_CONCURRENCY: Reason concurrency.
 * @eTDLS_TEARDOWN_RSSI_THRESHOLD: Reason rssi threshold.
 * @eTDLS_TEARDOWN_TXRX_THRESHOLD: Reason txrx threshold.
 * @eTDLS_TEARDOWN_BTCOEX: Reason BTCOEX.
 * @eTDLS_TEARDOWN_SCAN: Reason scan.
 * @eTDLS_TEARDOWN_BSS_DISCONNECT: Reason bss disconnected.
 * @eTDLS_TEARDOWN_ANTENNA_SWITCH: Disconnected due to antenna switch
 *
 * Reason to indicate in diag event of tdls teardown.
 */
enum tdls_teardown_reason {
	eTDLS_TEARDOWN_EXT_CTRL,
	eTDLS_TEARDOWN_CONCURRENCY,
	eTDLS_TEARDOWN_RSSI_THRESHOLD,
	eTDLS_TEARDOWN_TXRX_THRESHOLD,
	eTDLS_TEARDOWN_BTCOEX,
	eTDLS_TEARDOWN_SCAN,
	eTDLS_TEARDOWN_BSS_DISCONNECT,
	eTDLS_TEARDOWN_ANTENNA_SWITCH,
};

/**
 * enum tTDLSLinkReason - tdls link reason
 *
 * @eTDLS_LINK_SUCCESS: Success
 * @eTDLS_LINK_UNSPECIFIED: Unspecified reason
 * @eTDLS_LINK_NOT_SUPPORTED: Remote side doesn't support TDLS
 * @eTDLS_LINK_UNSUPPORTED_BAND: Remote side doesn't support this band
 * @eTDLS_LINK_NOT_BENEFICIAL: Going to AP is better than direct
 * @eTDLS_LINK_DROPPED_BY_REMOTE: Remote side doesn't want it anymore
 */
typedef enum {
	eTDLS_LINK_SUCCESS,
	eTDLS_LINK_UNSPECIFIED = -1,
	eTDLS_LINK_NOT_SUPPORTED = -2,
	eTDLS_LINK_UNSUPPORTED_BAND = -3,
	eTDLS_LINK_NOT_BENEFICIAL = -4,
	eTDLS_LINK_DROPPED_BY_REMOTE = -5
} tTDLSLinkReason;

/**
 * struct tdls_req_params_t - tdls request parameters
 *
 * @channel: channel hint, in channel number (NOT frequency)
 * @global_operating_class: operating class to use
 * @max_latency_ms: max latency that can be tolerated by apps
 * @min_bandwidth_kbps: bandwidth required by apps, in kilo bits per second
 */
typedef struct {
	int channel;
	int global_operating_class;
	int max_latency_ms;
	int min_bandwidth_kbps;
} tdls_req_params_t;

/**
 * enum tdls_state_t - tdls state
 *
 * @QCA_WIFI_HAL_TDLS_DISABLED: TDLS is not enabled, or is disabled now
 * @QCA_WIFI_HAL_TDLS_ENABLED: TDLS is enabled, but not yet tried
 * @QCA_WIFI_HAL_TDLS_ESTABLISHED: Direct link is established
 * @QCA_WIFI_HAL_TDLS_ESTABLISHED_OFF_CHANNEL: Direct link established using MCC
 * @QCA_WIFI_HAL_TDLS_DROPPED: Direct link was established, but is now dropped
 * @QCA_WIFI_HAL_TDLS_FAILED: Direct link failed
 */
typedef enum {
	QCA_WIFI_HAL_TDLS_DISABLED = 1,
	QCA_WIFI_HAL_TDLS_ENABLED,
	QCA_WIFI_HAL_TDLS_ESTABLISHED,
	QCA_WIFI_HAL_TDLS_ESTABLISHED_OFF_CHANNEL,
	QCA_WIFI_HAL_TDLS_DROPPED,
	QCA_WIFI_HAL_TDLS_FAILED
} tdls_state_t;

typedef int (*cfg80211_exttdls_callback)(const uint8_t *mac,
					 uint32_t opclass,
					 uint32_t channel,
					 uint32_t state,
					 int32_t reason, void *ctx);

/**
 * struct tdls_tx_tput_config_t - tdls tx throughput config
 *
 * @period: period
 * @bytes: bytes
 */
typedef struct {
	uint16_t period;
	uint16_t bytes;
} tdls_tx_tput_config_t;

/**
 * struct tdls_discovery_config_t - tdls discovery config
 *
 * @period: period
 * @tries: number of tries
 */
typedef struct {
	uint16_t period;
	uint16_t tries;
} tdls_discovery_config_t;

/**
 * struct tdls_rx_idle_config_t - tdls rx idle config
 *
 * @timeout: timeout
 */
typedef struct {
	uint16_t timeout;
} tdls_rx_idle_config_t;

/**
 * struct tdls_rssi_config_t - tdls rssi config
 *
 * @rssi_thres: rssi_thres
 */
typedef struct {
	uint16_t rssi_thres;
} tdls_rssi_config_t;

struct _hddTdlsPeer_t;

/**
 * struct tdls_ct_mac_table - connection tracker peer mac address table
 * @mac_address: peer mac address
 * @tx_packet_cnt: number of tx pkts
 * @rx_packet_cnt: number of rx pkts
 * @peer_timestamp_ms: time stamp of latest peer traffic
 */
struct tdls_ct_mac_table {
	struct qdf_mac_addr mac_address;
	uint32_t tx_packet_cnt;
	uint32_t rx_packet_cnt;
	uint32_t peer_timestamp_ms;
};

/**
 * struct tdls_set_state_db - set state command data base
 * @set_state_cnt: tdls set state count
 * @vdev_id: vdev id of last set state command
 */
struct tdls_set_state_info {
	uint8_t set_state_cnt;
	uint8_t vdev_id;
};

/**
 * struct tdlsCtx_t - tdls context
 *
 * @peer_list: peer list
 * @pAdapter: pointer to adapter
 * @peer_update_timer: connection tracker timer
 * @peerDiscoverTimer: peer discovery timer
 * @peerDiscoveryTimeoutTimer: peer discovery timeout timer
 * @threshold_config: threshold config
 * @discovery_peer_cnt: discovery peer count
 * @discovery_sent_cnt: discovery sent count
 * @ap_rssi: ap rssi
 * @curr_candidate: current candidate
 * @implicit_setup: implicit setup work queue
 * @ct_peer_mac_table: linear mac address table for counting the packets
 * @valid_mac_entries: number of valid mac entry in @ct_peer_mac_table
 * @magic: magic
 *
 */
typedef struct {
	struct list_head peer_list[TDLS_PEER_LIST_SIZE];
	hdd_adapter_t *pAdapter;
	qdf_mc_timer_t peer_update_timer;
	qdf_mc_timer_t peerDiscoveryTimeoutTimer;
	tdls_config_params_t threshold_config;
	int32_t discovery_peer_cnt;
	uint32_t discovery_sent_cnt;
	int8_t ap_rssi;
	struct _hddTdlsPeer_t *curr_candidate;
	struct work_struct implicit_setup;
	struct tdls_ct_mac_table ct_peer_mac_table[TDLS_CT_MAC_MAX_TABLE_SIZE];
	uint8_t valid_mac_entries;
	uint32_t magic;
} tdlsCtx_t;

/**
 * struct hddTdlsPeer_t - tdls peer data
 *
 * @node: node
 * @pHddTdlsCtx: pointer to tdls context
 * @peerMac: peer mac address
 * @staId: station identifier
 * @rssi: rssi
 * @tdls_support: tdls support
 * @link_status: tdls link status
 * @signature: signature
 * @is_responder: is responder
 * @discovery_processed: discovery processed flag
 * @discovery_attempt: discovery attempt
 * @tx_pkt: tx packet
 * @rx_pkt: rx packet
 * @uapsdQueues: uapsd queues
 * @maxSp: max sp
 * @isBufSta: is buffer sta
 * @isOffChannelSupported: is offchannel supported flag
 * @supported_channels_len: supported channels length
 * @supported_channels: supported channels
 * @supported_oper_classes_len: supported operation classes length
 * @supported_oper_classes: supported operation classes
 * @isForcedPeer: is forced peer
 * @op_class_for_pref_off_chan: op class for preferred off channel
 * @pref_off_chan_num: preferred off channel number
 * @op_class_for_pref_off_chan_is_set: op class for preferred off channel set
 * @peer_idle_timer: time to check idle traffic in tdls peers
 * @is_peer_idle_timer_initialised: Flag to check idle timer init
 * @spatial_streams: Number of TX/RX spatial streams for TDLS
 * @reason: reason
 * @state_change_notification: state change notification
 * @qos: QOS capability of TDLS link
 */
typedef struct _hddTdlsPeer_t {
	struct list_head node;
	tdlsCtx_t *pHddTdlsCtx;
	tSirMacAddr peerMac;
	uint16_t staId;
	int8_t rssi;
	tTDLSCapType tdls_support;
	tTDLSLinkStatus link_status;
	uint8_t signature;
	uint8_t is_responder;
	uint8_t discovery_processed;
	uint16_t discovery_attempt;
	uint16_t tx_pkt;
	uint16_t rx_pkt;
	uint8_t uapsdQueues;
	uint8_t maxSp;
	uint8_t isBufSta;
	uint8_t isOffChannelSupported;
	uint8_t supported_channels_len;
	uint8_t supported_channels[SIR_MAC_MAX_SUPP_CHANNELS];
	uint8_t supported_oper_classes_len;
	uint8_t supported_oper_classes[CDS_MAX_SUPP_OPER_CLASSES];
	bool isForcedPeer;
	uint8_t op_class_for_pref_off_chan;
	uint8_t pref_off_chan_num;
	uint8_t op_class_for_pref_off_chan_is_set;
	qdf_mc_timer_t peer_idle_timer;
	bool is_peer_idle_timer_initialised;
	uint8_t spatial_streams;
	tTDLSLinkReason reason;
	cfg80211_exttdls_callback state_change_notification;
	uint8_t qos;
} hddTdlsPeer_t;

/**
 * struct tdlsConnInfo_t - tdls connection info
 *
 * @sessionId: Session ID
 * @staId: TDLS peer station id
 * @peerMac: peer mac address
 */
typedef struct {
	uint8_t sessionId;
	uint8_t staId;
	struct qdf_mac_addr peerMac;
} tdlsConnInfo_t;

/**
 * struct tdlsInfo_t - tdls info
 *
 * @vdev_id: vdev id
 * @tdls_state: tdls state
 * @notification_interval_ms: notification interval in ms
 * @tx_discovery_threshold: tx discovery threshold
 * @tx_teardown_threshold: tx teardown threshold
 * @rssi_teardown_threshold: rx teardown threshold
 * @rssi_delta: rssi delta
 * @tdls_options: tdls options
 * @peer_traffic_ind_window: peer traffic indication window
 * @peer_traffic_response_timeout: peer traffic response timeout
 * @puapsd_mask: puapsd mask
 * @puapsd_inactivity_time: puapsd inactivity time
 * @puapsd_rx_frame_threshold: puapsd rx frame threshold
 * @teardown_notification_ms: tdls teardown notification interval
 * @tdls_peer_kickout_threshold: tdls packets threshold
 *    for peer kickout operation
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t tdls_state;
	uint32_t notification_interval_ms;
	uint32_t tx_discovery_threshold;
	uint32_t tx_teardown_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
	uint32_t tdls_options;
	uint32_t peer_traffic_ind_window;
	uint32_t peer_traffic_response_timeout;
	uint32_t puapsd_mask;
	uint32_t puapsd_inactivity_time;
	uint32_t puapsd_rx_frame_threshold;
	uint32_t teardown_notification_ms;
	uint32_t tdls_peer_kickout_threshold;
} tdlsInfo_t;

int wlan_hdd_tdls_init(hdd_adapter_t *pAdapter);

void wlan_hdd_tdls_exit(hdd_adapter_t *pAdapter);

void wlan_hdd_tdls_extract_da(struct sk_buff *skb, uint8_t *mac);

void wlan_hdd_tdls_extract_sa(struct sk_buff *skb, uint8_t *mac);

int wlan_hdd_tdls_increment_pkt_count(hdd_adapter_t *pAdapter,
				      const uint8_t *mac, uint8_t tx);

int wlan_hdd_tdls_set_sta_id(hdd_adapter_t *pAdapter, const uint8_t *mac,
			     uint8_t staId);

hddTdlsPeer_t *wlan_hdd_tdls_find_peer(hdd_adapter_t *pAdapter,
				       const uint8_t *mac, bool mutexLock);

hddTdlsPeer_t *wlan_hdd_tdls_find_all_peer(hdd_context_t *pHddCtx,
					   const uint8_t *mac);

int wlan_hdd_tdls_get_link_establish_params(hdd_adapter_t *pAdapter,
					    const uint8_t *mac,
					    tCsrTdlsLinkEstablishParams *
					    tdlsLinkEstablishParams);
hddTdlsPeer_t *wlan_hdd_tdls_get_peer(hdd_adapter_t *pAdapter,
				      const uint8_t *mac,
				      bool need_mutex_lock);

int wlan_hdd_tdls_set_cap(hdd_adapter_t *pAdapter, const uint8_t *mac,
			  tTDLSCapType cap);

void wlan_hdd_tdls_set_peer_link_status(hddTdlsPeer_t *curr_peer,
					tTDLSLinkStatus status,
					tTDLSLinkReason reason,
					bool lock_needed);
void wlan_hdd_tdls_set_link_status(hdd_adapter_t *pAdapter,
				   const uint8_t *mac,
				   tTDLSLinkStatus linkStatus,
				   tTDLSLinkReason reason);

int wlan_hdd_tdls_recv_discovery_resp(hdd_adapter_t *pAdapter,
				      const uint8_t *mac);

int wlan_hdd_tdls_set_peer_caps(hdd_adapter_t *pAdapter,
				const uint8_t *mac,
				tCsrStaParams *StaParams,
				bool isBufSta, bool isOffChannelSupported,
				bool is_qos_wmm_sta);

int wlan_hdd_tdls_set_rssi(hdd_adapter_t *pAdapter, const uint8_t *mac,
			   int8_t rxRssi);

int wlan_hdd_tdls_set_responder(hdd_adapter_t *pAdapter, const uint8_t *mac,
				uint8_t responder);

int wlan_hdd_tdls_set_signature(hdd_adapter_t *pAdapter, const uint8_t *mac,
				uint8_t uSignature);

int wlan_hdd_tdls_set_params(struct net_device *dev,
			     tdls_config_params_t *config);

int wlan_hdd_tdls_reset_peer(hdd_adapter_t *pAdapter, const uint8_t *mac);

uint16_t wlan_hdd_tdls_connected_peers(hdd_adapter_t *pAdapter);

int wlan_hdd_tdls_get_all_peers(hdd_adapter_t *pAdapter, char *buf,
				int buflen);

void wlan_hdd_tdls_connection_callback(hdd_adapter_t *pAdapter);

void wlan_hdd_tdls_disconnection_callback(hdd_adapter_t *pAdapter);

void wlan_hdd_tdls_mgmt_completion_callback(hdd_adapter_t *pAdapter,
					    uint32_t statusCode);

void wlan_hdd_tdls_tncrement_peer_count(hdd_adapter_t *pAdapter);

void wlan_hdd_tdls_decrement_peer_count(hdd_adapter_t *pAdapter);

hddTdlsPeer_t *wlan_hdd_tdls_is_progress(hdd_context_t *pHddCtx,
					 const uint8_t *mac, uint8_t skip_self,
					 bool need_lock);

int wlan_hdd_tdls_copy_scan_context(hdd_context_t *pHddCtx,
				    struct wiphy *wiphy,
				    struct cfg80211_scan_request *request);

int wlan_hdd_tdls_scan_callback(hdd_adapter_t *pAdapter, struct wiphy *wiphy,
				struct cfg80211_scan_request *request);

void wlan_hdd_tdls_scan_done_callback(hdd_adapter_t *pAdapter);

void wlan_hdd_tdls_timer_restart(hdd_adapter_t *pAdapter,
				 qdf_mc_timer_t *timer,
				 uint32_t expirationTime);
void wlan_hdd_tdls_indicate_teardown(hdd_adapter_t *pAdapter,
				     hddTdlsPeer_t *curr_peer,
				     uint16_t reason);

void wlan_hdd_tdls_pre_setup_init_work(tdlsCtx_t *pHddTdlsCtx,
				       hddTdlsPeer_t *curr_candidate);

int wlan_hdd_tdls_set_extctrl_param(hdd_adapter_t *pAdapter,
				    const uint8_t *mac,
				    uint32_t chan,
				    uint32_t max_latency,
				    uint32_t op_class, uint32_t min_bandwidth);
int wlan_hdd_tdls_set_force_peer(hdd_adapter_t *pAdapter, const uint8_t *mac,
				 bool forcePeer);

int wlan_hdd_tdls_update_peer_mac(hdd_adapter_t *adapter,
				const uint8_t *mac,
				uint32_t peer_state);

int wlan_hdd_tdls_extctrl_deconfig_peer(hdd_adapter_t *pAdapter,
					const uint8_t *peer);
int wlan_hdd_tdls_extctrl_config_peer(hdd_adapter_t *pAdapter,
				      const uint8_t *peer,
				      cfg80211_exttdls_callback callback,
				      uint32_t chan,
				      uint32_t max_latency,
				      uint32_t op_class,
				      uint32_t min_bandwidth);
int wlan_hdd_tdls_get_status(hdd_adapter_t *pAdapter,
			     const uint8_t *mac, uint32_t *opclass,
			     uint32_t *channel, uint32_t *state,
			     int32_t *reason);
void wlan_hdd_tdls_get_wifi_hal_state(hddTdlsPeer_t *curr_peer,
				      uint32_t *state, int32_t *reason);
int wlan_hdd_set_callback(hddTdlsPeer_t *curr_peer,
			  cfg80211_exttdls_callback callback);
void wlan_hdd_update_tdls_info(hdd_adapter_t *adapter, bool tdls_prohibited,
			       bool tdls_chan_swit_prohibited);

int wlan_hdd_tdls_add_station(struct wiphy *wiphy,
			      struct net_device *dev, const uint8_t *mac,
			      bool update, tCsrStaParams *StaParams);

int wlan_hdd_cfg80211_exttdls_enable(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len);

int wlan_hdd_cfg80211_exttdls_disable(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data,
				      int data_len);

int wlan_hdd_cfg80211_exttdls_get_status(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
				struct net_device *dev,
				const uint8_t *peer,
				enum nl80211_tdls_operation oper);
#else
int wlan_hdd_cfg80211_tdls_oper(struct wiphy *wiphy,
				struct net_device *dev,
				uint8_t *peer,
				enum nl80211_tdls_operation oper);
#endif

#ifdef TDLS_MGMT_VERSION2
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, u8 *peer,
				u8 action_code, u8 dialog_token,
				u16 status_code, u32 peer_capability,
				const u8 *buf, size_t len);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, const uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				bool initiator, const uint8_t *buf,
				size_t len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, const uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len);
#else
int wlan_hdd_cfg80211_tdls_mgmt(struct wiphy *wiphy,
				struct net_device *dev, uint8_t *peer,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, const uint8_t *buf,
				size_t len);
#endif
#endif

void hdd_tdls_notify_mode_change(hdd_adapter_t *adapter,
				hdd_context_t *hddctx);
void wlan_hdd_tdls_disable_offchan_and_teardown_links(hdd_context_t *hddctx);

hddTdlsPeer_t *wlan_hdd_tdls_find_first_connected_peer(hdd_adapter_t *adapter);
int hdd_set_tdls_offchannel(hdd_context_t *hdd_ctx, int offchannel);
int hdd_set_tdls_secoffchanneloffset(hdd_context_t *hdd_ctx, int offchanoffset);
int hdd_set_tdls_offchannelmode(hdd_adapter_t *adapter, int offchanmode);
void wlan_hdd_tdls_update_tx_pkt_cnt(hdd_adapter_t *adapter,
				     struct sk_buff *skb);
void wlan_hdd_tdls_update_rx_pkt_cnt(hdd_adapter_t *adapter,
				     struct sk_buff *skb);
int hdd_set_tdls_scan_type(hdd_context_t *hdd_ctx, int val);
void hdd_tdls_context_init(hdd_context_t *hdd_ctx);
void hdd_tdls_context_destroy(hdd_context_t *hdd_ctx);
int wlan_hdd_tdls_antenna_switch(hdd_context_t *hdd_ctx,
				 hdd_adapter_t *adapter,
				 uint32_t mode);
hdd_adapter_t *wlan_hdd_tdls_check_and_enable(hdd_context_t *hdd_ctx);


#else
static inline void hdd_tdls_notify_mode_change(hdd_adapter_t *adapter,
				hdd_context_t *hddctx)
{
}
static inline void
wlan_hdd_tdls_disable_offchan_and_teardown_links(hdd_context_t *hddctx)
{
}
static inline void wlan_hdd_tdls_exit(hdd_adapter_t *adapter)
{
}
static inline void wlan_hdd_tdls_update_tx_pkt_cnt(hdd_adapter_t *adapter,
						   struct sk_buff *skb)
{
}
static inline void wlan_hdd_tdls_update_rx_pkt_cnt(hdd_adapter_t *adapter,
						   struct sk_buff *skb)
{
}
static inline void hdd_tdls_context_init(hdd_context_t *hdd_ctx) { }
static inline void hdd_tdls_context_destroy(hdd_context_t *hdd_ctx) { }

static inline int wlan_hdd_tdls_antenna_switch(hdd_context_t *hdd_ctx,
					       hdd_adapter_t *adapter,
					       uint32_t mode)
{
	return 0;
}

static inline hdd_adapter_t *wlan_hdd_tdls_check_and_enable(
						hdd_context_t *hdd_ctx)
{
	return NULL;
}

static inline void wlan_hdd_update_tdls_info(hdd_adapter_t *adapter,
						bool tdls_prohibited,
						bool tdls_chan_swit_prohibited)
{
}

#endif /* End of FEATURE_WLAN_TDLS */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
void hdd_send_wlan_tdls_teardown_event(uint32_t reason,
					uint8_t *peer_mac);
void hdd_wlan_tdls_enable_link_event(const uint8_t *peer_mac,
	uint8_t is_off_chan_supported,
	uint8_t is_off_chan_configured,
	uint8_t is_off_chan_established);
void hdd_wlan_block_scan_by_tdls_event(void);
#else
static inline
void hdd_send_wlan_tdls_teardown_event(uint32_t reason,
					uint8_t *peer_mac) {}
static inline
void hdd_wlan_tdls_enable_link_event(const uint8_t *peer_mac,
	uint8_t is_off_chan_supported,
	uint8_t is_off_chan_configured,
	uint8_t is_off_chan_established) {}
static inline void hdd_wlan_block_scan_by_tdls_event(void) {}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

#endif /* __HDD_TDLS_H */
