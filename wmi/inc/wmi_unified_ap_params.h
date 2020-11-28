/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* This file has WIN specific WMI structures and macros. */

#ifndef _WMI_UNIFIED_AP_PARAMS_H_
#define _WMI_UNIFIED_AP_PARAMS_H_

/* Country code string length*/
#define COUNTRY_CODE_LEN 2
/* Civic information size in bytes */
#define CIVIC_INFO_LEN 256

/**
 * wmi_wifi_pos_lcr_info - LCR info structure
 * @pdev_id: host pdev id
 * @req_id: LCR request id
 * @civic_len: Civic info length
 * @country_code: Country code string
 * @civic_info: Civic info
 */
struct wmi_wifi_pos_lcr_info {
	uint32_t pdev_id;
	uint16_t req_id;
	uint32_t civic_len;
	uint8_t  country_code[COUNTRY_CODE_LEN];
	uint8_t civic_info[CIVIC_INFO_LEN];
};

/**
 * lci_motion_pattern - LCI motion pattern
 * @LCI_MOTION_NOT_EXPECTED: Not expected to change location
 * @LCI_MOTION_EXPECTED: Expected to change location
 * @LCI_MOTION_UNKNOWN: Movement pattern unknown
 */
enum wifi_pos_motion_pattern {
	wifi_pos_MOTION_NOT_EXPECTED = 0,
	wifi_pos_MOTION_EXPECTED     = 1,
	wifi_pos_MOTION_UNKNOWN      = 2
};

/**
 * wifi_pos_lci_info - LCI info structure
 * @pdev_id: host pdev_id
 * @req_id: LCI request id
 * @latitude: Latitude value
 * @longitude: Longitude value
 * @altitude: Altitude value
 * @latitude_unc: Latitude uncertainty value
 * @longitude_unc: Longitude uncertainty value
 * @altitude_unc: Altitude uncertainty value
 * @motion_pattern: Motion pattern
 * @floor: Floor value
 * @height_above_floor: Height above floor
 * @height_unc: Height uncertainty value
 * @usage_rules: Usage rules
 */
struct wifi_pos_lci_info {
	uint32_t pdev_id;
	uint16_t req_id;
	int64_t latitude;
	int64_t longitude;
	int32_t altitude;
	uint8_t latitude_unc;
	uint8_t longitude_unc;
	uint8_t altitude_unc;
	enum wifi_pos_motion_pattern motion_pattern;
	int32_t floor;
	int32_t height_above_floor;
	int32_t height_unc;
	uint32_t usage_rules;
};

/**
 * struct peer_add_wds_entry_params - WDS peer entry add params
 * @dest_addr: Pointer to destination macaddr
 * @peer_addr: Pointer to peer mac addr
 * @flags: flags
 * @vdev_id: Vdev id
 */
struct peer_add_wds_entry_params {
	const uint8_t *dest_addr;
	uint8_t *peer_addr;
	uint32_t flags;
	uint32_t vdev_id;
};

/**
 * struct peer_del_wds_entry_params - WDS peer entry del params
 * @dest_addr: Pointer to destination macaddr
 * @vdev_id: Vdev id
 */
struct peer_del_wds_entry_params {
	uint8_t *dest_addr;
	uint32_t vdev_id;
};

/**
 * struct peer_update_wds_entry_params - WDS peer entry update params
 * @wds_macaddr: Pointer to destination macaddr
 * @peer_add: Pointer to peer mac addr
 * @flags: flags
 * @vdev_id: Vdev id
 */
struct peer_update_wds_entry_params {
	uint8_t *wds_macaddr;
	uint8_t *peer_macaddr;
	uint32_t flags;
	uint32_t vdev_id;
};

/**
 * struct ctl_table_params - Ctl table params
 * @ctl_array: pointer to ctl array
 * @ctl_cmd_len: ctl command length
 * @is_2g: is 2G
 * @target_type: target type
 * @ctl_band: ctl band
 * @pdev_id: pdev id
 */
struct ctl_table_params {
	uint8_t *ctl_array;
	uint16_t ctl_cmd_len;
	bool is_2g;
	uint32_t target_type;
	uint32_t ctl_band;
	uint32_t pdev_id;
};

/**
 * struct mimogain_table_params - MIMO gain table params
 * @array_gain: pointer to array gain table
 * @tbl_len: table length
 * @multichain_gain_bypass: bypass multichain gain
 */
struct mimogain_table_params {
	uint8_t *array_gain;
	uint16_t tbl_len;
	bool multichain_gain_bypass;
	uint32_t pdev_id;
};

/**
 * struct packet_power_info_params - packet power info params
 * @chainmask: chain mask
 * @chan_width: channel bandwidth
 * @rate_flags: rate flags
 * @su_mu_ofdma: su/mu/ofdma flags
 * @nss: number of spatial streams
 * @preamble: preamble
 * @hw_rate:
 */
struct packet_power_info_params {
	uint16_t chainmask;
	uint16_t chan_width;
	uint16_t rate_flags;
	uint16_t su_mu_ofdma;
	uint16_t nss;
	uint16_t preamble;
	uint16_t hw_rate;
	uint32_t pdev_id;
};

/* struct ht_ie_params - HT IE params
 * @ie_len: IE length
 * @ie_data: pointer to IE data
 * @tx_streams: Tx streams supported for this HT IE
 * @rx_streams: Rx streams supported for this HT IE
 */
struct ht_ie_params {
	uint32_t ie_len;
	uint8_t *ie_data;
	uint32_t tx_streams;
	uint32_t rx_streams;
};

/* struct vht_ie_params - VHT IE params
 * @ie_len: IE length
 * @ie_data: pointer to IE data
 * @tx_streams: Tx streams supported for this VHT IE
 * @rx_streams: Rx streams supported for this VHT IE
 */
struct vht_ie_params {
	uint32_t ie_len;
	uint8_t *ie_data;
	uint32_t tx_streams;
	uint32_t rx_streams;
};

/**
 * struct set_quiet_mode_params - Set quiet mode params
 * @enabled: Enabled
 * @period: Quite period
 * @intval: Quite interval
 * @duration: Quite duration
 * @offset: offset
 */
struct set_quiet_mode_params {
	uint8_t enabled;
	uint8_t period;
	uint16_t intval;
	uint16_t duration;
	uint16_t offset;
};

/**
 * struct set_bcn_offload_quiet_mode_params - Set quiet mode params
 * @vdev_id: Vdev ID
 * @period: Quite period
 * @duration: Quite duration
 * @next_start: Next quiet start
 * @flag: 0 - disable, 1 - enable and continuous, 3 - enable and single shot
 */
struct set_bcn_offload_quiet_mode_params {
	uint32_t vdev_id;
	uint32_t period;
	uint32_t duration;
	uint32_t next_start;
	uint32_t flag;
};

/**
 * struct bcn_offload_control - Beacon offload control params
 * @vdev_id: vdev identifer of VAP to control beacon tx
 * @bcn_ctrl_op: values from enum bcn_offload_control_param
 */
struct bcn_offload_control {
	uint32_t vdev_id;
	enum bcn_offload_control_param bcn_ctrl_op;
};

/**
 * struct wmi_host_tim_info - TIM info in SWBA event
 * @tim_len: TIM length
 * @tim_mcast:
 * @tim_bitmap: TIM bitmap
 * @tim_changed: TIM changed
 * @tim_num_ps_pending: TIM num PS sta pending
 * @vdev_id: Vdev id
 */
typedef struct {
	uint32_t tim_len;
	uint32_t tim_mcast;
	uint32_t tim_bitmap[WMI_HOST_TIM_BITMAP_ARRAY_SIZE];
	uint32_t tim_changed;
	uint32_t tim_num_ps_pending;
	uint32_t vdev_id;
} wmi_host_tim_info;

/* Maximum number of NOA Descriptors supported */
#define WMI_HOST_P2P_MAX_NOA_DESCRIPTORS 4
/**
 * struct wmi_host_p2p_noa_info - p2p noa information
 * @modified: NoA modified
 * @index: Index
 * @oppPS: Oppurtunstic ps
 * @ctwindow: CT window
 * @num_descriptors: number of descriptors
 * @noa_descriptors: noa descriptors
 * @vdev_id: Vdev id
 */
typedef struct {
	uint8_t modified;
	uint8_t index;
	uint8_t oppPS;
	uint8_t ctwindow;
	uint8_t num_descriptors;
	wmi_host_p2p_noa_descriptor
		noa_descriptors[WMI_HOST_P2P_MAX_NOA_DESCRIPTORS];
	uint32_t vdev_id;
} wmi_host_p2p_noa_info;

/**
 * struct wmi_host_quiet_info - Quiet info in SWBA event
 * @vdev_id: vdev_id for quiet info structure
 * @tbttcount: quiet start tbtt count
 * @period: Beacon interval between quiets
 * @duration: TUs of each quiet
 * @offset: TUs from TBTT to quiet start
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t tbttcount;
	uint32_t period;
	uint32_t duration;
	uint32_t offset;
} wmi_host_quiet_info;

/**
 * struct wmi_host_offchan_data_tx_compl_event - TX completion event
 * @desc_id: from tx_send_cmd
 * @status: VWMI_MGMT_TX_COMP_STATUS_TYPE
 * @pdev_id: pdev_id
 */
struct wmi_host_offchan_data_tx_compl_event {
	uint32_t desc_id;
	uint32_t status;
	uint32_t pdev_id;
};

/**
 * struct wmi_host_pdev_tpc_config_event - host pdev tpc config event
 * @pdev_id: pdev_id
 * @regDomain:
 * @chanFreq:
 * @phyMode:
 * @twiceAntennaReduction:
 * @twiceMaxRDPower:
 * @twiceAntennaGain:
 * @powerLimit:
 * @rateMax:
 * @numTxChain:
 * @ctl:
 * @flags:
 * @maxRegAllowedPower:
 * @maxRegAllowedPowerAGCDD:
 * @maxRegAllowedPowerAGSTBC:
 * @maxRegAllowedPowerAGTXBF:
 * @ratesArray:
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t regDomain;
	uint32_t chanFreq;
	uint32_t phyMode;
	uint32_t twiceAntennaReduction;
	uint32_t twiceMaxRDPower;
	int32_t  twiceAntennaGain;
	uint32_t powerLimit;
	uint32_t rateMax;
	uint32_t numTxChain;
	uint32_t ctl;
	uint32_t flags;
	int8_t  maxRegAllowedPower[WMI_HOST_TPC_TX_NUM_CHAIN];
	int8_t  maxRegAllowedPowerAGCDD[WMI_HOST_TPC_TX_NUM_CHAIN][WMI_HOST_TPC_TX_NUM_CHAIN];
	int8_t  maxRegAllowedPowerAGSTBC[WMI_HOST_TPC_TX_NUM_CHAIN][WMI_HOST_TPC_TX_NUM_CHAIN];
	int8_t  maxRegAllowedPowerAGTXBF[WMI_HOST_TPC_TX_NUM_CHAIN][WMI_HOST_TPC_TX_NUM_CHAIN];
	uint8_t ratesArray[WMI_HOST_TPC_RATE_MAX];
} wmi_host_pdev_tpc_config_event;

/**
 * struct wmi_host_peer_sta_kickout_event
 * @peer_macaddr: peer mac address
 * @reason: kickout reason
 * @rssi: rssi
 * @pdev_id: pdev_id
 */
typedef struct {
	uint8_t peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t reason;
	uint32_t rssi;
} wmi_host_peer_sta_kickout_event;

/**
 * struct wmi_host_peer_create_response_event - Peer Create response event param
 * @vdev_id: vdev id
 * @mac_address: Peer Mac Address
 * @status: Peer create status
 *
 */
struct wmi_host_peer_create_response_event {
	uint32_t vdev_id;
	struct qdf_mac_addr mac_address;
	uint32_t status;
};

/**
 * struct wmi_host_peer_delete_response_event - Peer Delete response event param
 * @vdev_id: vdev id
 * @mac_address: Peer Mac Address
 *
 */
struct wmi_host_peer_delete_response_event {
	uint32_t vdev_id;
	struct qdf_mac_addr mac_address;
};

/**
 * struct wmi_host_pdev_tpc_event - WMI host pdev TPC event
 * @pdev_id: pdev_id
 * @tpc:
 */
typedef struct {
	uint32_t pdev_id;
	int32_t tpc[WMI_HOST_TX_POWER_LEN];
} wmi_host_pdev_tpc_event;

/**
 * struct wmi_host_pdev_nfcal_power_all_channels_event - NF cal event data
 * @nfdbr:
 *   chan[0 ~ 7]: {NFCalPower_chain0, NFCalPower_chain1,
 *                 NFCalPower_chain2, NFCalPower_chain3,
 *                 NFCalPower_chain4, NFCalPower_chain5,
 *                 NFCalPower_chain6, NFCalPower_chain7},
 * @nfdbm:
 *   chan[0 ~ 7]: {NFCalPower_chain0, NFCalPower_chain1,
 *                 NFCalPower_chain2, NFCalPower_chain3,
 *                 NFCalPower_chain4, NFCalPower_chain5,
 *                 NFCalPower_chain6, NFCalPower_chain7},
 * @freqnum:
 *   chan[0 ~ 7]: frequency number
 * @pdev_id: pdev_id
 * @num_freq: number of valid frequency in freqnum
 * @num_nfdbr_dbm: number of valid entries in dbr/dbm array
 *
 */
typedef struct {
	int8_t nfdbr[WMI_HOST_RXG_CAL_CHAN_MAX * WMI_HOST_MAX_NUM_CHAINS];
	int8_t nfdbm[WMI_HOST_RXG_CAL_CHAN_MAX * WMI_HOST_MAX_NUM_CHAINS];
	uint32_t freqnum[WMI_HOST_RXG_CAL_CHAN_MAX];
	uint32_t pdev_id;
	uint16_t num_freq;
	uint16_t num_nfdbr_dbm;
} wmi_host_pdev_nfcal_power_all_channels_event;

/**
 * struct wds_addr_event - WDS addr event structure
 * @event_type: event type add/delete
 * @peer_mac: peer mac
 * @dest_mac: destination mac address
 * @vdev_id: vdev id
 */
typedef struct {
	uint32_t event_type[4];
	u_int8_t peer_mac[QDF_MAC_ADDR_SIZE];
	u_int8_t dest_mac[QDF_MAC_ADDR_SIZE];
	uint32_t vdev_id;
} wds_addr_event_t;

/**
 * struct wmi_host_peer_sta_ps_statechange_event - ST ps state change event
 * @peer_macaddr: peer mac address
 * @peer_ps_stats: peer PS state
 * @pdev_id: pdev_id
 */
typedef struct {
	uint8_t peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t peer_ps_state;
} wmi_host_peer_sta_ps_statechange_event;

/**
 * struct wmi_host_inst_stats_resp
 * @iRSSI: Instantaneous RSSI
 * @peer_macaddr: peer mac address
 * @pdev_id: pdev_id
 */
typedef struct {
	uint32_t iRSSI;
	wmi_host_mac_addr peer_macaddr;
	uint32_t pdev_id;
} wmi_host_inst_stats_resp;

typedef struct {
	uint32_t software_cal_version;
	uint32_t board_cal_version;
	/* board_mcn_detail:
	 * Provide a calibration message string for the host to display.
	 * Note: on a big-endian host, the 4 bytes within each uint32_t portion
	 * of a WMI message will be automatically byteswapped by the copy engine
	 * as the messages are transferred between host and target, to convert
	 * between the target's little-endianness and the host's big-endianness.
	 * Consequently, a big-endian host should manually unswap the bytes
	 * within the board_mcn_detail string buffer to get the bytes back into
	 * the desired natural order.
	 */
	uint8_t board_mcn_detail[WMI_HOST_BOARD_MCN_STRING_BUF_SIZE];
	uint32_t cal_ok; /* filled with CALIBRATION_STATUS enum value */
} wmi_host_pdev_check_cal_version_event;

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/**
 * enum wmi_pdev_pkt_routing_op_code_type - packet routing supported opcodes
 * @ADD_PKT_ROUTING: Add packet routing command
 * @DEL_PKT_ROUTING: Delete packet routing command
 *
 * Defines supported opcodes for packet routing/tagging
 */
enum wmi_pdev_pkt_routing_op_code_type {
	ADD_PKT_ROUTING,
	DEL_PKT_ROUTING,
};

/**
 * enum wmi_pdev_pkt_routing_pkt_type - supported packet types for
 * routing & tagging
 * @PDEV_PKT_TYPE_ARP_IPV4: Route/Tag for packet type ARP IPv4 (L3)
 * @PDEV_PKT_TYPE_NS_IPV6: Route/Tag for packet type NS IPv6 (L3)
 * @PDEV_PKT_TYPE_IGMP_IPV4: Route/Tag for packet type IGMP IPv4 (L3)
 * @PDEV_PKT_TYPE_MLD_IPV6: Route/Tag for packet type MLD IPv6 (L3)
 * @PDEV_PKT_TYPE_DHCP_IPV4: Route/Tag for packet type DHCP IPv4 (APP)
 * @PDEV_PKT_TYPE_DHCP_IPV6: Route/Tag for packet type DHCP IPv6 (APP)
 * @PDEV_PKT_TYPE_DNS_TCP_IPV4: Route/Tag for packet type TCP DNS IPv4 (APP)
 * @PDEV_PKT_TYPE_DNS_TCP_IPV6: Route/Tag for packet type TCP DNS IPv6 (APP)
 * @PDEV_PKT_TYPE_DNS_UDP_IPV4: Route/Tag for packet type UDP DNS IPv4 (APP)
 * @PDEV_PKT_TYPE_DNS_UDP_IPV6: Route/Tag for packet type UDP DNS IPv6 (APP)
 * @PDEV_PKT_TYPE_ICMP_IPV4: Route/Tag for packet type ICMP IPv4 (L3)
 * @PDEV_PKT_TYPE_ICMP_IPV6: Route/Tag for packet type ICMP IPv6 (L3)
 * @PDEV_PKT_TYPE_TCP_IPV4: Route/Tag for packet type TCP IPv4 (L4)
 * @PDEV_PKT_TYPE_TCP_IPV6: Route/Tag for packet type TCP IPv6 (L4)
 * @PDEV_PKT_TYPE_UDP_IPV4: Route/Tag for packet type UDP IPv4 (L4)
 * @PDEV_PKT_TYPE_UDP_IPV6: Route/Tag for packet type UDP IPv6 (L4)
 * @PDEV_PKT_TYPE_IPV4: Route/Tag for packet type IPv4 (L3)
 * @PDEV_PKT_TYPE_IPV6: Route/Tag for packet type IPv6 (L3)
 * @PDEV_PKT_TYPE_EAP: Route/Tag for packet type EAP (L2)
 *
 * Defines supported protocol types for routing/tagging
 */
enum wmi_pdev_pkt_routing_pkt_type {
	PDEV_PKT_TYPE_ARP_IPV4,
	PDEV_PKT_TYPE_NS_IPV6,
	PDEV_PKT_TYPE_IGMP_IPV4,
	PDEV_PKT_TYPE_MLD_IPV6,
	PDEV_PKT_TYPE_DHCP_IPV4,
	PDEV_PKT_TYPE_DHCP_IPV6,
	PDEV_PKT_TYPE_DNS_TCP_IPV4,
	PDEV_PKT_TYPE_DNS_TCP_IPV6,
	PDEV_PKT_TYPE_DNS_UDP_IPV4,
	PDEV_PKT_TYPE_DNS_UDP_IPV6,
	PDEV_PKT_TYPE_ICMP_IPV4,
	PDEV_PKT_TYPE_ICMP_IPV6,
	PDEV_PKT_TYPE_TCP_IPV4,
	PDEV_PKT_TYPE_TCP_IPV6,
	PDEV_PKT_TYPE_UDP_IPV4,
	PDEV_PKT_TYPE_UDP_IPV6,
	PDEV_PKT_TYPE_IPV4,
	PDEV_PKT_TYPE_IPV6,
	PDEV_PKT_TYPE_EAP,
	PDEV_PKT_TYPE_MAX
};

/**
 * enum wmi_pdev_dest_ring_handler_type - packet routing options post CCE
 * tagging
 * @PDEV_WIFIRXCCE_USE_CCE_E: Use REO destination ring from CCE
 * @PDEV_WIFIRXCCE_USE_ASPT_E: Use REO destination ring from ASPT
 * @PDEV_WIFIRXCCE_USE_FT_E: Use REO destination ring from FSE
 * @PDEV_WIFIRXCCE_USE_CCE2_E: Use REO destination ring from CCE2
 *
 * Defines various options for routing policy
 */
enum wmi_pdev_dest_ring_handler_type {
	PDEV_WIFIRXCCE_USE_CCE_E  = 0,
	PDEV_WIFIRXCCE_USE_ASPT_E = 1,
	PDEV_WIFIRXCCE_USE_FT_E   = 2,
	PDEV_WIFIRXCCE_USE_CCE2_E = 3,
};

/**
 * struct wmi_rx_pkt_protocol_routing_info - RX packet routing/tagging params
 * @pdev_id: pdev id
 * @op_code: Opcode option from wmi_pdev_pkt_routing_op_code_type enum
 * @routing_type_bitmap: Bitmap of protocol that is being configured. Only
 * one protocol can be configured in one command. Supported protocol list
 * from enum wmi_pdev_pkt_routing_pkt_type
 * @dest_ring_handler: Destination ring selection from enum
 * wmi_pdev_dest_ring_handler_type
 * @dest_ring: Destination ring number to use if dest ring handler is CCE
 * @meta_data: Metadata to tag with for given protocol
 */
struct wmi_rx_pkt_protocol_routing_info {
	uint32_t      pdev_id;
	enum wmi_pdev_pkt_routing_op_code_type op_code;
	uint32_t      routing_type_bitmap;
	uint32_t      dest_ring_handler;
	uint32_t      dest_ring;
	uint32_t      meta_data;
};
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

/**
 * struct peer_vlan_config_param - peer vlan config command
 * @tx_cmd: Tx command
 * @rx_cmd: Rx command
 * @tx_strip_insert: Strip or Insert vlan in Tx[0:Strip, 1: Insert]
 * @tx_strip_insert_inner: Enable tx_strip_insert operation for inner vlan tag.
 * @tx_strip_insert_outer: Enable tx_strip_insert operation for outer vlan tag.
 * @rx_strip_c_tag: Strip c_tag
 * @rx_strip_s_tag: Strip s_tag
 * @rx_insert_c_tag: Insert c_tag
 * @rx_insert_s_tag: Insert s_tag
 *
 * @insert_vlan_inner_tci: Vlan inner tci
 * @insert_vlan_inner_tci: Vlan outer tci
 *
 * @vdev_id: vdev id corresponding to peer.
 */
struct peer_vlan_config_param {
	uint16_t tx_cmd:1,
		rx_cmd:1,
		tx_strip_insert:1,
		tx_strip_insert_inner:1,
		tx_strip_insert_outer:1,
		rx_strip_c_tag:1,
		rx_strip_s_tag:1,
		rx_insert_c_tag:1,
		rx_insert_s_tag:1;
	uint16_t insert_vlan_inner_tci;
	uint16_t insert_vlan_outer_tci;
	uint8_t vdev_id;
};

/**
 * struct fd_params - FD cmd parameter
 * @vdev_id: vdev id
 * @frame_ctrl: frame control field
 * @wbuf: FD buffer
 */
struct fd_params {
	uint8_t vdev_id;
	uint16_t frame_ctrl;
	qdf_nbuf_t wbuf;
};

/**
 * struct set_qbosst_params - Set QBOOST params
 * @vdev_id: vdev id
 * @value: value
 */
struct set_qboost_params {
	uint8_t vdev_id;
	uint32_t value;
};

/**
 * struct mcast_group_update_param - Mcast group table update to target
 * @action: Addition/deletion
 * @wildcard: iwldcard table entry?
 * @mcast_ip_addr: mcast ip address to be updated
 * @mcast_ip_addr_bytes: mcast ip addr bytes
 * @nsrcs: number of entries in source list
 * @filter_mode: filter mode
 * @is_action_delete: is delete
 * @is_filter_mode_snoop: is filter mode snoop
 * @ucast_mac_addr: ucast peer mac subscribed to mcast ip
 * @srcs: source mac accpted
 * @mask: mask
 * @vap_id: vdev id
 * @is_mcast_addr_len: is mcast address length
 */
struct mcast_group_update_params {
	int action;
	int wildcard;
	uint8_t *mcast_ip_addr;
	int mcast_ip_addr_bytes;
	uint8_t nsrcs;
	uint8_t filter_mode;
	bool is_action_delete;
	bool is_filter_mode_snoop;
	uint8_t *ucast_mac_addr;
	uint8_t *srcs;
	uint8_t *mask;
	uint8_t vap_id;
	bool is_mcast_addr_len;
};

struct pdev_qvit_params {
	uint8_t *utf_payload;
	uint32_t len;
};

/**
 * struct wmi_host_wmeParams - WME params
 * @wmep_acm: ACM paramete
 * @wmep_aifsn:	AIFSN parameters
 * @wmep_logcwmin: cwmin in exponential form
 * @wmep_logcwmax: cwmax in exponential form
 * @wmep_txopLimit: txopLimit
 * @wmep_noackPolicy: No-Ack Policy: 0=ack, 1=no-ack
 */
struct wmi_host_wmeParams {
	u_int8_t	wmep_acm;
	u_int8_t	wmep_aifsn;
	u_int8_t	wmep_logcwmin;
	u_int8_t	wmep_logcwmax;
	u_int16_t   wmep_txopLimit;
	u_int8_t	wmep_noackPolicy;
};

/**
 * struct wmm_update_params - WMM update params
 * @wmep_array: WME params for each AC
 */
struct wmm_update_params {
	struct wmi_host_wmeParams *wmep_array;
};

/**
 * struct wmi_host_mgmt_tx_compl_event - TX completion event
 * @desc_id: from tx_send_cmd
 * @status: WMI_MGMT_TX_COMP_STATUS_TYPE
 * @pdev_id: pdev_id
 * @ppdu_id: ppdu_id
 * @retries_count: retries count
 * @tx_tsf: 64 bits completion timestamp
 */
typedef struct {
	uint32_t	desc_id;
	uint32_t	status;
	uint32_t	pdev_id;
	uint32_t        ppdu_id;
	uint32_t	retries_count;
	uint64_t	tx_tsf;
} wmi_host_mgmt_tx_compl_event;

/**
 * struct wmi_host_chan_info_event - Channel info WMI event
 * @pdev_id: pdev_id
 * @err_code: Error code
 * @freq: Channel freq
 * @cmd_flags: Read flags
 * @noise_floor: Noise Floor value
 * @rx_clear_count: rx clear count
 * @cycle_count: cycle count
 * @chan_tx_pwr_range: channel tx power per range
 * @chan_tx_pwr_tp: channel tx power per throughput
 * @rx_frame_count: rx frame count
 * @rx_11b_mode_data_duration: 11b mode data duration
 * @my_bss_rx_cycle_count: self BSS rx cycle count
 * @tx_frame_cnt: tx frame count
 * @mac_clk_mhz: mac clock
 * @vdev_id: unique id identifying the VDEV
 * @tx_frame_count: tx frame count
 * @rx_clear_ext20_count: ext20 frame count
 * @rx_clear_ext40_count: ext40 frame count
 * @rx_clear_ext80_count: ext80 frame count
 * @per_chain_noise_floor: Per chain NF value in dBm
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t err_code;
	uint32_t freq;
	uint32_t cmd_flags;
	uint32_t noise_floor;
	uint32_t rx_clear_count;
	uint32_t cycle_count;
	uint32_t chan_tx_pwr_range;
	uint32_t chan_tx_pwr_tp;
	uint32_t rx_frame_count;
	uint32_t rx_11b_mode_data_duration;
	uint32_t my_bss_rx_cycle_count;
	uint32_t tx_frame_cnt;
	uint32_t mac_clk_mhz;
	uint32_t vdev_id;
	uint32_t tx_frame_count;
	uint32_t rx_clear_ext20_count;
	uint32_t rx_clear_ext40_count;
	uint32_t rx_clear_ext80_count;
	uint32_t per_chain_noise_floor[WMI_HOST_MAX_CHAINS];
} wmi_host_chan_info_event;

/**
 * struct wmi_host_pdev_channel_hopping_event
 * @pdev_id: pdev_id
 * @noise_floor_report_iter: Noise threshold iterations with high values
 * @noise_floor_total_iter: Total noise threshold iterations
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t noise_floor_report_iter;
	uint32_t noise_floor_total_iter;
} wmi_host_pdev_channel_hopping_event;

/**
 * struct peer_chan_width_switch_params - Peer channel width capability wrapper
 * @num_peers: Total number of peers connected to AP
 * @max_peers_per_cmd: Peer limit per WMI command
 * @vdev_id: vdev id
 * @chan_width_peer_list: List of capabilities for all connected peers
 */

struct peer_chan_width_switch_params {
	uint32_t num_peers;
	uint32_t max_peers_per_cmd;
	uint32_t vdev_id;
	struct peer_chan_width_switch_info *chan_width_peer_list;
};

/**
 * struct wmi_pdev_enable_tx_mode_selection - fw tx mode selection
 * @pdev_id: radio id
 * @enable_tx_mode_selection: flag to enable tx mode selection
 */
struct wmi_pdev_enable_tx_mode_selection {
	uint32_t pdev_id;
	uint32_t enable_tx_mode_selection;
};

#ifdef WLAN_SUPPORT_MESH_LATENCY
/**
 * struct wmi_vdev_latency_info_params - vdev latency info params
 * @service_interval: service interval in miliseconds
 * @burst_size: burst size in bytes
 * @latency_tid: tid associated with this latency information
 * @ac: Access Category associated with this td
 * @ul_enable: Bit to indicate ul latency enable
 * @dl_enable: Bit to indicate dl latency enable
 */
struct wmi_vdev_latency_info_params {
	uint32_t service_interval;
	uint32_t burst_size;
	uint32_t latency_tid :8,
			 ac          :2,
			 ul_enable   :1,
			 dl_enable   :1,
			 reserved    :20;
};

/**
 * struct wmi_vdev_tid_latency_config_params - VDEV TID latency config params
 * @num_vdev: Number of vdevs in this message
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 * @latency_info: Vdev latency info
 */
struct wmi_vdev_tid_latency_config_params {
	uint8_t num_vdev;
	uint16_t vdev_id;
	uint16_t pdev_id;
	struct wmi_vdev_latency_info_params latency_info[1];
};

/**
 * struct wmi_peer_latency_info_params - peer latency info params
 * @peer_mac: peer mac address
 * @service_interval: service interval in miliseconds
 * @burst_size: burst size in bytes
 * @latency_tid: tid associated with this latency information
 * @ac: Access Category associated with this td
 * @ul_enable: Bit to indicate ul latency enable
 * @dl_enable: Bit to indicate dl latency enable
 * @flow_id: Flow id associated with tid
 * @add_or_sub: Bit to indicate add/delete of latency params
 */
struct wmi_peer_latency_info_params {
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	uint32_t service_interval;
	uint32_t burst_size;
	uint32_t latency_tid :8,
			ac          :2,
			ul_enable   :1,
			dl_enable   :1,
			flow_id     :4,
			add_or_sub  :2,
			reserved    :14;
};

/**
 * struct wmi_peer_latency_config_params - peer latency config params
 * @num_peer: Number of peers in this message
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 * @latency_info: Vdev latency info
 */
struct wmi_peer_latency_config_params {
	uint32_t num_peer;
	uint32_t pdev_id;
	struct wmi_peer_latency_info_params latency_info[1];
};
#endif
#endif
