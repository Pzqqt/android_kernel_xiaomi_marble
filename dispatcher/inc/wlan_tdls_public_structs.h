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

/**
 * DOC: wlan_tdls_public_structs.h
 *
 * TDLS public structure definations
 */

#ifndef _WLAN_TDLS_STRUCTS_H_
#define _WLAN_TDLS_STRUCTS_H_
#include <qdf_timer.h>
#include <qdf_list.h>
#include <qdf_mc_timer.h>
#include <wlan_cmn.h>
#include <wlan_cmn_ieee80211.h>

#define WLAN_TDLS_STA_MAX_NUM                        8
#define WLAN_TDLS_STA_P_UAPSD_OFFCHAN_MAX_NUM        1
#define WLAN_TDLS_PEER_LIST_SIZE                     16
#define WLAN_TDLS_CT_TABLE_SIZE                      8
#define WLAN_TDLS_PEER_SUB_LIST_SIZE                 10
#define WLAN_MAC_MAX_EXTN_CAP                        8
#define WLAN_MAC_MAX_SUPP_CHANNELS                   100
#define WLAN_MAX_SUPP_OPER_CLASSES                   32
#define WLAN_MAC_MAX_SUPP_RATES                      32
#define WLAN_CHANNEL_14                              14
#define ENABLE_CHANSWITCH                            1
#define DISABLE_CHANSWITCH                           2
#define WLAN_TDLS_PREFERRED_OFF_CHANNEL_NUM_MIN      1
#define WLAN_TDLS_PREFERRED_OFF_CHANNEL_NUM_MAX      165
#define WLAN_TDLS_PREFERRED_OFF_CHANNEL_NUM_DEF      36

/** Maximum time(ms) to wait for tdls add sta to complete **/
#define WAIT_TIME_TDLS_ADD_STA      1500

/** Maximum time(ms) to wait for tdls del sta to complete **/
#define WAIT_TIME_TDLS_DEL_STA      1500

/** Maximum time(ms) to wait for Link Establish Req to complete **/
#define WAIT_TIME_TDLS_LINK_ESTABLISH_REQ      1500

/** Maximum time(ms) to wait for tdls mgmt to complete **/
#define WAIT_TIME_TDLS_MGMT         11000

#define TDLS_TEARDOWN_PEER_UNREACHABLE   25
#define TDLS_TEARDOWN_PEER_UNSPEC_REASON 26

#define TDLS_STA_INDEX_CHECK(sta_id) \
	(((sta_id) >= 1) && ((sta_id) < 0xFF))
/**
 * enum tdls_conc_cap - tdls concurrency support
 * @TDLS_SUPPORTED_ONLY_ON_STA: only support sta tdls
 * @TDLS_SUPPORTED_ONLY_ON_P2P_CLIENT: only support p2p client tdls
 */
enum tdls_conc_cap {
	TDLS_SUPPORTED_ONLY_ON_STA = 0,
	TDLS_SUPPORTED_ONLY_ON_P2P_CLIENT,
};

/**
 * enum tdls_peer_capab - tdls capability type
 * @TDLS_CAP_NOT_SUPPORTED: tdls not supported
 * @TDLS_CAP_UNKNOWN: unknown capability
 * @TDLS_CAP_SUPPORTED: tdls capability supported
 */
enum tdls_peer_capab {
	TDLS_CAP_NOT_SUPPORTED = -1,
	TDLS_CAP_UNKNOWN       = 0,
	TDLS_CAP_SUPPORTED     = 1,
};

/**
 * enum tdls_peer_state - tdls peer state
 * @TDLS_PEER_STATE_PEERING: tdls connection in progress
 * @TDLS_PEER_STATE_CONNCTED: tdls peer is connected
 * @TDLS_PEER_STATE_TEARDOWN: tdls peer is tear down
 * @TDLS_PEER_ADD_MAC_ADDR: add peer mac into connection table
 * @TDLS_PEER_REMOVE_MAC_ADDR: remove peer mac from connection table
 */
enum tdls_peer_state {
	TDLS_PEER_STATE_PEERING,
	TDLS_PEER_STATE_CONNCTED,
	TDLS_PEER_STATE_TEARDOWN,
	TDLS_PEER_ADD_MAC_ADDR,
	TDLS_PEER_REMOVE_MAC_ADDR
};

/**
 * enum tdls_link_state - tdls link state
 * @TDLS_LINK_IDLE: tdls link idle
 * @TDLS_LINK_DISCOVERING: tdls link discovering
 * @TDLS_LINK_DISCOVERED: tdls link discovered
 * @TDLS_LINK_CONNECTING: tdls link connecting
 * @TDLS_LINK_CONNECTED: tdls link connected
 * @TDLS_LINK_TEARING: tdls link tearing
 */
enum tdls_link_state {
	TDLS_LINK_IDLE = 0,
	TDLS_LINK_DISCOVERING,
	TDLS_LINK_DISCOVERED,
	TDLS_LINK_CONNECTING,
	TDLS_LINK_CONNECTED,
	TDLS_LINK_TEARING,
};

/**
 * enum tdls_link_state_reason - tdls link reason
 * @TDLS_LINK_SUCCESS: Success
 * @TDLS_LINK_UNSPECIFIED: Unspecified reason
 * @TDLS_LINK_NOT_SUPPORTED: Remote side doesn't support TDLS
 * @TDLS_LINK_UNSUPPORTED_BAND: Remote side doesn't support this band
 * @TDLS_LINK_NOT_BENEFICIAL: Going to AP is better than direct
 * @TDLS_LINK_DROPPED_BY_REMOTE: Remote side doesn't want it anymore
 */
enum tdls_link_state_reason {
	TDLS_LINK_SUCCESS,
	TDLS_LINK_UNSPECIFIED         = -1,
	TDLS_LINK_NOT_SUPPORTED       = -2,
	TDLS_LINK_UNSUPPORTED_BAND    = -3,
	TDLS_LINK_NOT_BENEFICIAL      = -4,
	TDLS_LINK_DROPPED_BY_REMOTE   = -5,
};

/**
 * enum tdls_feature_mode - TDLS support mode
 * @TDLS_SUPPORT_DISABLED: Disabled in ini or FW
 * @TDLS_SUPPORT_SUSPENDED: TDLS supported by ini and FW, but disabled
 *            temporarily due to off-channel operations or due to other reasons
 * @TDLS_SUPPORT_EXP_TRIG_ONLY: Explicit trigger mode
 * @TDLS_SUPPORT_IMP_MODE: Implicit mode
 * @TDLS_SUPPORT_EXT_CONTROL: External control mode
 */
enum tdls_feature_mode {
	TDLS_SUPPORT_DISABLED = 0,
	TDLS_SUPPORT_SUSPENDED,
	TDLS_SUPPORT_EXP_TRIG_ONLY,
	TDLS_SUPPORT_IMP_MODE,
	TDLS_SUPPORT_EXT_CONTROL,
};

/**
 * enum tdls_event_type - TDLS event type
 * @TDLS_EVENT_VDEV_STATE_CHANGE: umac connect/disconnect event
 * @TDLS_EVENT_MGMT_TX_ACK_CNF: tx tdls frame ack event
 * @TDLS_EVENT_RX_MGMT: rx discovery response frame
 * @TDLS_EVENT_ADD_PEER: add peer or update peer
 * @TDLS_EVENT_DEL_PEER: delete peer
 * @TDLS_EVENT_DISCOVERY_REQ: dicovery request
 * @TDLS_EVENT_TEARDOWN_REQ: teardown request
 * @TDLS_EVENT_SETUP_REQ: setup request
 */
enum tdls_event_type {
	TDLS_EVENT_VDEV_STATE_CHANGE = 0,
	TDLS_EVENT_MGMT_TX_ACK_CNF,
	TDLS_EVENT_RX_MGMT,
	TDLS_EVENT_ADD_PEER,
	TDLS_EVENT_DEL_PEER,
	TDLS_EVENT_DISCOVERY_REQ,
	TDLS_EVENT_TEARDOWN_REQ,
	TDLS_EVENT_SETUP_REQ,
};

/**
 * enum tdls_state_t - tdls state
 * @QCA_WIFI_HAL_TDLS_DISABLED: TDLS is not enabled, or is disabled now
 * @QCA_WIFI_HAL_TDLS_ENABLED: TDLS is enabled, but not yet tried
 * @QCA_WIFI_HAL_TDLS_ESTABLISHED: Direct link is established
 * @QCA_WIFI_HAL_TDLS_ESTABLISHED_OFF_CHANNEL: Direct link established using MCC
 * @QCA_WIFI_HAL_TDLS_DROPPED: Direct link was established, but is now dropped
 * @QCA_WIFI_HAL_TDLS_FAILED: Direct link failed
 */
enum tdls_state_t {
	QCA_WIFI_HAL_TDLS_S_DISABLED = 1,
	QCA_WIFI_HAL_TDLS_S_ENABLED,
	QCA_WIFI_HAL_TDLS_S_ESTABLISHED,
	QCA_WIFI_HAL_TDLS_S_ESTABLISHED_OFF_CHANNEL,
	QCA_WIFI_HAL_TDLS_S_DROPPED,
	QCA_WIFI_HAL_TDLS_S_FAILED,
};

/**
 * enum tdls_off_chan_mode - mode for WMI_TDLS_SET_OFFCHAN_MODE_CMDID
 * @TDLS_ENABLE_OFFCHANNEL: enable off channel
 * @TDLS_DISABLE_OFFCHANNEL: disable off channel
 */
enum tdls_off_chan_mode {
	TDLS_ENABLE_OFFCHANNEL,
	TDLS_DISABLE_OFFCHANNEL
};

/**
 * enum tdls_event_msg_type - TDLS event message type
 * @TDLS_SHOULD_DISCOVER: should do discover for peer (based on tx bytes per
 * second > tx_discover threshold)
 * @TDLS_SHOULD_TEARDOWN: recommend teardown the link for peer due to tx bytes
 * per second below tx_teardown_threshold
 * @TDLS_PEER_DISCONNECTED: tdls peer disconnected
 * @TDLS_CONNECTION_TRACKER_NOTIFY: TDLS/BT role change notification for
 * connection tracker
 */
enum tdls_event_msg_type {
	TDLS_SHOULD_DISCOVER = 0,
	TDLS_SHOULD_TEARDOWN,
	TDLS_PEER_DISCONNECTED,
	TDLS_CONNECTION_TRACKER_NOTIFY,
};

/**
 * enum tdls_event_reason - TDLS event reason
 * @TDLS_TEARDOWN_TX: tdls teardown recommended due to low transmits
 * @TDLS_TEARDOWN_RSSI: tdls link tear down recommended due to poor RSSI
 * @TDLS_TEARDOWN_SCAN: tdls link tear down recommended due to offchannel scan
 * @TDLS_TEARDOWN_PTR_TIMEOUT: tdls peer disconnected due to PTR timeout
 * @TDLS_TEARDOWN_BAD_PTR: tdls peer disconnected due wrong PTR format
 * @TDLS_TEARDOWN_NO_RSP: tdls peer not responding
 * @TDLS_DISCONNECTED_PEER_DELETE: tdls peer disconnected due to peer deletion
 * @TDLS_PEER_ENTER_BUF_STA: tdls entered buffer STA role, TDLS connection
 * tracker needs to handle this
 * @TDLS_PEER_EXIT_BUF_STA: tdls exited buffer STA role, TDLS connection tracker
 * needs to handle this
 * @TDLS_ENTER_BT_BUSY: BT entered busy mode, TDLS connection tracker needs to
 * handle this
 * @TDLS_EXIT_BT_BUSY: BT exited busy mode, TDLS connection tracker needs to
 * handle this
 * @DLS_SCAN_STARTED: TDLS module received a scan start event, TDLS connection
 * tracker needs to handle this
 * @TDLS_SCAN_COMPLETED: TDLS module received a scan complete event, TDLS
 * connection tracker needs to handle this
 */
enum tdls_event_reason {
	TDLS_TEARDOWN_TX,
	TDLS_TEARDOWN_RSSI,
	TDLS_TEARDOWN_SCAN,
	TDLS_TEARDOWN_PTR_TIMEOUT,
	TDLS_TEARDOWN_BAD_PTR,
	TDLS_TEARDOWN_NO_RSP,
	TDLS_DISCONNECTED_PEER_DELETE,
	TDLS_PEER_ENTER_BUF_STA,
	TDLS_PEER_EXIT_BUF_STA,
	TDLS_ENTER_BT_BUSY,
	TDLS_EXIT_BT_BUSY,
	TDLS_SCAN_STARTED,
	TDLS_SCAN_COMPLETED,
};

/**
 * struct tdls_osif_indication - tdls indication to os if layer
 * @vdev: vdev object
 * @reason: used with teardown indication
 * @peer_mac: MAC address of the TDLS peer
 */
struct tdls_osif_indication {
	struct wlan_objmgr_vdev *vdev;
	uint16_t reason;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	QDF_STATUS status;
};

/**
 * struct tx_frame - tx frame
 * @buf: frame buffer
 * @buf_len: buffer length
 * @tx_timer: tx send timer
 */
struct tx_frame {
	uint8_t *buf;
	size_t buf_len;
	qdf_timer_t tx_timer;
};

/**
 * enum tdls_feature_bit
 * @TDLS_FEATURE_OFF_CHANNEL: tdls off channel
 * @TDLS_FEATURE_WMM: tdls wmm
 * @TDLS_FEATURE_BUFFER_STA: tdls buffer sta
 * @TDLS_FEATURE_SLEEP_STA: tdls sleep sta feature
 * @TDLS_FEATURE_SCAN: tdls scan
 * @TDLS_FEATURE_ENABLE: tdls enabled
 * @TDLS_FEAUTRE_IMPLICIT_TRIGGER: tdls implicit trigger
 * @TDLS_FEATURE_EXTERNAL_CONTROL: tdls external control
 */
enum tdls_feature_bit {
	TDLS_FEATURE_OFF_CHANNEL,
	TDLS_FEATURE_WMM,
	TDLS_FEATURE_BUFFER_STA,
	TDLS_FEATURE_SLEEP_STA,
	TDLS_FEATURE_SCAN,
	TDLS_FEATURE_ENABLE,
	TDLS_FEAUTRE_IMPLICIT_TRIGGER,
	TDLS_FEATURE_EXTERNAL_CONTROL
};

#define TDLS_IS_OFF_CHANNEL_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_OFF_CHANNEL, flags)
#define TDLS_IS_WMM_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_WMM, flags)
#define TDLS_IS_BUFFER_STA_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_BUFFER_STA, flags)
#define TDLS_IS_SLEEP_STA_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_SLEEP_STA, flags)
#define TDLS_IS_SCAN_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_SCAN, flags)
#define TDLS_IS_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_ENABLE, flags)
#define TDLS_IS_IMPLICIT_TRIG_ENABLED(flags) \
	CHECK_BIT(TDLS_FEAUTRE_IMPLICIT_TRIGGER, flags)
#define TDLS_IS_EXTERNAL_CONTROL_ENABLED(flags) \
	CHECK_BIT(TDLS_FEATURE_EXTERNAL_CONTROL, flags)

/**
 * struct tdls_user_config - TDLS user configuration
 * @tdls_tx_states_period: tdls tx states period
 * @tdls_tx_pkt_threshold: tdls tx packets threshold
 * @tdls_rx_pkt_threshold: tdls rx packets threshold
 * @tdls_max_discovery_attempt: tdls discovery max times
 * @tdls_idle_timeout: tdls idle timeout
 * @tdls_idle_pkt_threshold: tdls idle packets threshold
 * @tdls_rssi_trigger_threshold: tdls rssi trigger threshold
 * @tdls_rssi_teardown_threshold: tdls rssi tear down threshold
 * @tdls_rssi_delta: tdls rssi delta
 * @tdls_uapsd_mask: tdls uapsd mask
 * @tdls_uapsd_inactivity_time: tdls uapsd inactivity time
 * @tdls_uapsd_pti_window: tdls peer traffic indication window
 * @tdls_uapsd_ptr_timeout: tdls peer response timeout
 * @tdls_feature_flags: tdls feature flags
 * @tdls_pre_off_chan_num: tdls off channel number
 * @tdls_pre_off_chan_bw: tdls off channel bandwidth
 * @tdls_peer_kickout_threshold: sta kickout threshold for tdls peer
 * @delayed_trig_framint: delayed trigger frame interval
 */
struct tdls_user_config {
	uint32_t tdls_tx_states_period;
	uint32_t tdls_tx_pkt_threshold;
	uint32_t tdls_rx_pkt_threshold;
	uint32_t tdls_max_discovery_attempt;
	uint32_t tdls_idle_timeout;
	uint32_t tdls_idle_pkt_threshold;
	uint32_t tdls_rssi_trigger_threshold;
	uint32_t tdls_rssi_teardown_threshold;
	uint32_t tdls_rssi_delta;
	uint32_t tdls_uapsd_mask;
	uint32_t tdls_uapsd_inactivity_time;
	uint32_t tdls_uapsd_pti_window;
	uint32_t tdls_uapsd_ptr_timeout;
	uint32_t tdls_feature_flags;
	uint32_t tdls_pre_off_chan_num;
	uint32_t tdls_pre_off_chan_bw;
	uint32_t tdls_peer_kickout_threshold;
	uint32_t delayed_trig_framint;
};

/**
 * struct tdls_config_params - tdls configure paramets
 * @tdls: tdls support mode
 * @tx_period_t: tdls tx stats period
 * @tx_packet_n: tdls tx packets number threshold
 * @discovery_tries_n: tdls max discovery attempt count
 * @idle_timeout_t: tdls idle time timeout
 * @idle_packet_n: tdls idle pkt threshold
 * @rssi_trigger_threshold: tdls rssi trigger threshold, checked before setup
 * @rssi_teardown_threshold: tdls rssi teardown threshold
 * @rssi_delta: rssi delta
 */
struct tdls_config_params {
	uint32_t tdls;
	uint32_t tx_period_t;
	uint32_t tx_packet_n;
	uint32_t discovery_tries_n;
	uint32_t idle_timeout_t;
	uint32_t idle_packet_n;
	int32_t rssi_trigger_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
};

/**
 * struct tdls_tx_cnf: tdls tx ack
 * @vdev_id: vdev id
 * @action_cookie: frame cookie
 * @buf: frame buf
 * @buf_len: buffer length
 * @status: tx send status
 */
struct tdls_tx_cnf {
	int vdev_id;
	uint64_t action_cookie;
	void *buf;
	size_t buf_len;
	int status;
};

/* This callback is used to report state change of peer to wpa_supplicant */
typedef int (*tdls_state_change_callback)(const uint8_t *mac,
					  uint32_t opclass,
					  uint32_t channel,
					  uint32_t state,
					  int32_t reason, void *ctx);

/* This callback is used to report events to os_if layer */
typedef void (*tdls_evt_callback) (void *data,
				   enum tdls_event_type ev_type,
				   struct tdls_osif_indication *event);

/* prototype of callback registered by hdd to receive the ack cnf */
typedef int (*tdls_tx_ack_cnf_callback)(void *user_data,
					struct tdls_tx_cnf *tx_ack_cnf_cb_data);

/* This callback is used to register TDLS peer with TL */
typedef QDF_STATUS (*tdls_register_tl_peer_callback)(void *userdata,
						     uint32_t vdev_id,
						     const uint8_t *mac,
						     uint16_t stat_id,
						     uint8_t ucastsig,
						     uint8_t qos);

/* This callback is used to deregister TDLS peer */
typedef QDF_STATUS (*tdls_deregister_tl_peer_callback)(void *userdata,
						       uint32_t vdev_id,
						       uint8_t sta_id);
/**
 * struct tdls_start_params - tdls start params
 * @config: tdls user config
 * @tdls_send_mgmt_req: pass eWNI_SME_TDLS_SEND_MGMT_REQ value
 * @tdls_add_sta_req: pass eWNI_SME_TDLS_ADD_STA_REQ value
 * @tdls_del_sta_req: pass eWNI_SME_TDLS_DEL_STA_REQ value
 * @tdls_update_peer_state: pass WMA_UPDATE_TDLS_PEER_STATE value
 * @tdls_event_cb: tdls event callback
 * @tdls_evt_cb_data: tdls event data
 * @ack_cnf_cb: tdls tx ack callback to indicate the tx status
 * @tx_ack_cnf_cb_data: tdls tx ack user data
 * @tdls_reg_tl_peer: tdls register tdls peer
 * @tdls_dereg_tl_peer: tdls deregister tdls peer
 * @tdls_tl_peer_data: userdata for register/deregister TDLS peer
 */
struct tdls_start_params {
	struct tdls_user_config config;
	uint16_t tdls_send_mgmt_req;
	uint16_t tdls_add_sta_req;
	uint16_t tdls_del_sta_req;
	uint16_t tdls_update_peer_state;
	tdls_evt_callback tdls_event_cb;
	void *tdls_evt_cb_data;
	tdls_tx_ack_cnf_callback ack_cnf_cb;
	void *tx_ack_cnf_cb_data;
	tdls_register_tl_peer_callback tdls_reg_tl_peer;
	tdls_deregister_tl_peer_callback tdls_dereg_tl_peer;
	void *tdls_tl_peer_data;
};

/**
 * struct tdls_add_peer_params - add peer request parameter
 * @peer_addr: peer mac addr
 * @peer_type: peer type
 * @vdev_id: vdev id
 */
struct tdls_add_peer_params {
	uint8_t peer_addr[QDF_MAC_ADDR_SIZE];
	uint32_t peer_type;
	uint32_t vdev_id;
};

/**
 * struct tdls_add_peer_request - peer add request
 * @vdev: vdev
 * @add_peer_req: add peer request parameters
 */
struct tdls_add_peer_request {
	struct wlan_objmgr_vdev *vdev;
	struct tdls_add_peer_params add_peer_req;
};

/**
 * struct tdls_del_peer_params - delete peer request parameter
 * @peer_addr: peer mac addr
 * @peer_type: peer type
 * @vdev_id: vdev id
 */
struct tdls_del_peer_params {
	const uint8_t *peer_addr;
	uint32_t peer_type;
	uint32_t vdev_id;
};

/**
 * struct tdls_del_peer_request - peer delete request
 * @vdev: vdev
 * @del_peer_req: delete peer request parameters
 */
struct tdls_del_peer_request {
	struct wlan_objmgr_vdev *vdev;
	struct tdls_del_peer_params del_peer_req;
};

/**
 * struct vhgmcsinfo - VHT MCS information
 * @rx_mcs_map: RX MCS map 2 bits for each stream, total 8 streams
 * @rx_highest: Indicates highest long GI VHT PPDU data rate
 *      STA can receive. Rate expressed in units of 1 Mbps.
 *      If this field is 0 this value should not be used to
 *      consider the highest RX data rate supported.
 * @tx_mcs_map: TX MCS map 2 bits for each stream, total 8 streams
 * @tx_highest: Indicates highest long GI VHT PPDU data rate
 *      STA can transmit. Rate expressed in units of 1 Mbps.
 *      If this field is 0 this value should not be used to
 *      consider the highest TX data rate supported.
 */
struct vhtmcsinfo {
	uint16_t rx_mcs_map;
	uint16_t rx_highest;
	uint16_t tx_mcs_map;
	uint16_t tx_highest;
};

/**
 * struct vhtcap - VHT capabilities
 *
 * This structure is the "VHT capabilities element" as
 * described in 802.11ac D3.0 8.4.2.160
 * @vht_cap_info: VHT capability info
 * @supp_mcs: VHT MCS supported rates
 */
struct vhtcap {
	uint32_t vht_capinfo;
	struct vhtmcsinfo supp_mcs;
};

struct tdls_update_peer_params {
	uint8_t peer_addr[QDF_MAC_ADDR_SIZE];
	uint32_t peer_type;
	uint32_t vdev_id;
	uint16_t capability;
	uint8_t extn_capability[WLAN_MAC_MAX_EXTN_CAP];
	uint8_t supported_rates_len;
	uint8_t supported_rates[WLAN_MAC_MAX_SUPP_RATES];
	uint8_t htcap_present;
	struct htcap_cmn_ie ht_cap;
	uint8_t vhtcap_present;
	struct vhtcap vht_cap;
	uint8_t uapsd_queues;
	uint8_t max_sp;
	uint8_t supported_channels_len;
	uint8_t supported_channels[WLAN_MAC_MAX_SUPP_CHANNELS];
	uint8_t supported_oper_classes_len;
	uint8_t supported_oper_classes[WLAN_MAX_SUPP_OPER_CLASSES];
	bool is_qos_wmm_sta;
};

struct tdls_update_peer_request {
	struct wlan_objmgr_vdev *vdev;
	struct tdls_update_peer_params update_peer_req;
};

/**
 * struct tdls_oper_request - tdls operation request
 * @vdev: vdev object
 * @peer_addr: MAC address of the TDLS peer
 */
struct tdls_oper_request {
	struct wlan_objmgr_vdev *vdev;
	uint8_t peer_addr[QDF_MAC_ADDR_SIZE];
};

/**
 * struct tdls_oper_config_force_peer_request - tdls enable force peer request
 * @vdev: vdev object
 * @peer_addr: MAC address of the TDLS peer
 * @chan: channel
 * @max_latency: maximum latency
 * @op_class: operation class
 * @min_bandwidth: minimal bandwidth
 * @callback: state change callback
 */
struct tdls_oper_config_force_peer_request {
	struct wlan_objmgr_vdev *vdev;
	uint8_t peer_addr[QDF_MAC_ADDR_SIZE];
	uint32_t chan;
	uint32_t max_latency;
	uint32_t op_class;
	uint32_t min_bandwidth;
	tdls_state_change_callback callback;
};

/**
 * struct tdls_info - tdls info
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
struct tdls_info {
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
};

/**
 * struct tdls_ch_params - channel parameters
 * @chan_id: ID of the channel
 * @pwr: power level
 * @dfs_set: is dfs supported or not
 * @half_rate: is the channel operating at 10MHz
 * @quarter_rate: is the channel operating at 5MHz
 */
struct tdls_ch_params {
	uint8_t chan_id;
	uint8_t pwr;
	bool dfs_set;
	bool half_rate;
	bool quarter_rate;
};

/**
 * struct tdls_peer_params - TDLS peer capablities parameters
 * @is_peer_responder: is peer responder or not
 * @peer_uapsd_queue: peer uapsd queue
 * @peer_max_sp: peer max SP value
 * @peer_buff_sta_support: peer buffer sta supported or not
 * @peer_off_chan_support: peer offchannel support
 * @peer_curr_operclass: peer current operating class
 * @self_curr_operclass: self current operating class
 * @peer_chanlen: peer channel length
 * @peer_chan: peer channel list
 * @peer_oper_classlen: peer operating class length
 * @peer_oper_class: peer operating class
 * @pref_off_channum: peer offchannel number
 * @pref_off_chan_bandwidth: peer offchannel bandwidth
 * @opclass_for_prefoffchan: operating class for offchannel
 */
struct tdls_peer_params {
	uint8_t is_peer_responder;
	uint8_t peer_uapsd_queue;
	uint8_t peer_max_sp;
	uint8_t peer_buff_sta_support;
	uint8_t peer_off_chan_support;
	uint8_t peer_curr_operclass;
	uint8_t self_curr_operclass;
	uint8_t peer_chanlen;
	struct tdls_ch_params peer_chan[WLAN_MAC_MAX_SUPP_CHANNELS];
	uint8_t peer_oper_classlen;
	uint8_t peer_oper_class[WLAN_MAX_SUPP_OPER_CLASSES];
	uint8_t pref_off_channum;
	uint8_t pref_off_chan_bandwidth;
	uint8_t opclass_for_prefoffchan;
};

/**
 * struct tdls_peer_update_state - TDLS peer state parameters
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @peer_cap: peer capabality
 * @resp_reqd: response needed
 */
struct tdls_peer_update_state {
	uint32_t vdev_id;
	uint8_t peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t peer_state;
	struct tdls_peer_params peer_cap;
	bool resp_reqd;
};

/**
 * struct tdls_channel_switch_params - channel switch parameter structure
 * @vdev_id: vdev ID
 * @peer_mac_addr: Peer mac address
 * @tdls_off_ch_bw_offset: Target off-channel bandwitdh offset
 * @tdls_off_ch: Target Off Channel
 * @oper_class: Operating class for target channel
 * @is_responder: Responder or initiator
 */
struct tdls_channel_switch_params {
	uint32_t    vdev_id;
	uint8_t     peer_mac_addr[QDF_MAC_ADDR_SIZE];
	uint16_t    tdls_off_ch_bw_offset;
	uint8_t     tdls_off_ch;
	uint8_t     tdls_sw_mode;
	uint8_t     oper_class;
	uint8_t     is_responder;
};

/**
 * struct tdls_event_info - firmware tdls event
 * @vdev_id: vdev id
 * @peermac: peer mac address
 * @message_type: message type
 * @peer_reason: reason
 */
struct tdls_event_info {
	uint8_t vdev_id;
	struct qdf_mac_addr peermac;
	uint16_t message_type;
	uint32_t peer_reason;
};

/**
 * struct tdls_event_notify - tdls event notify
 * @vdev: vdev object
 * @event: tdls event
 */
struct tdls_event_notify {
	struct wlan_objmgr_vdev *vdev;
	struct tdls_event_info event;
};
#endif
