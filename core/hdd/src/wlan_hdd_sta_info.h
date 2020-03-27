/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_sta_info.h
 *
 * Store and manage station info structure.
 *
 */
#if !defined(__WLAN_HDD_STA_INFO_H)
#define __WLAN_HDD_STA_INFO_H

#include "qdf_lock.h"
#include "qdf_types.h"
#include "qdf_hashtable.h"
#include "sap_api.h"
#include "cdp_txrx_cmn_struct.h"
#include "sir_mac_prot_def.h"
#include <linux/ieee80211.h>
#include <wlan_mlme_public_struct.h>

/* A bucket size of 2^4 = 16 */
#define WLAN_HDD_STA_INFO_SIZE 4

/* Opaque handle for abstraction */
#define hdd_sta_info_entry qdf_ht_entry

/**
 * struct dhcp_phase - Per Peer DHCP Phases
 * @DHCP_PHASE_ACK: upon receiving DHCP_ACK/NAK message in REQUEST phase or
 *         DHCP_DELINE message in OFFER phase
 * @DHCP_PHASE_DISCOVER: upon receiving DHCP_DISCOVER message in ACK phase
 * @DHCP_PHASE_OFFER: upon receiving DHCP_OFFER message in DISCOVER phase
 * @DHCP_PHASE_REQUEST: upon receiving DHCP_REQUEST message in OFFER phase or
 *         ACK phase (Renewal process)
 */
enum dhcp_phase {
	DHCP_PHASE_ACK,
	DHCP_PHASE_DISCOVER,
	DHCP_PHASE_OFFER,
	DHCP_PHASE_REQUEST
};

/**
 * struct dhcp_nego_status - Per Peer DHCP Negotiation Status
 * @DHCP_NEGO_STOP: when the peer is in ACK phase or client disassociated
 * @DHCP_NEGO_IN_PROGRESS: when the peer is in DISCOVER or REQUEST
 *         (Renewal process) phase
 */
enum dhcp_nego_status {
	DHCP_NEGO_STOP,
	DHCP_NEGO_IN_PROGRESS
};

/**
 * Pending frame type of EAP_FAILURE, bit number used in "pending_eap_frm_type"
 * of sta_info.
 */
#define PENDING_TYPE_EAP_FAILURE  0

/**
 * struct hdd_station_info - Per station structure kept in HDD for
 *                                     multiple station support for SoftAP
 * @in_use: Is the station entry in use?
 * @sta_id: Station ID reported back from HAL (through SAP).
 *           Broadcast uses station ID zero by default.
 * @sta_type: Type of station i.e. p2p client or infrastructure station
 * @sta_mac: MAC address of the station
 * @peer_state: Current Station state so HDD knows how to deal with packet
 *              queue. Most recent states used to change TLSHIM STA state.
 * @is_qos_enabled: Track QoS status of station
 * @is_deauth_in_progress: The station entry for which Deauth is in progress
 * @nss: Number of spatial streams supported
 * @rate_flags: Rate Flags for this connection
 * @ecsa_capable: Extended CSA capabilities
 * @max_phy_rate: Calcuated maximum phy rate based on mode, nss, mcs etc.
 * @tx_packets: Packets send to current station
 * @tx_bytes: Bytes send to current station
 * @rx_packets: Packets received from current station
 * @rx_bytes: Bytes received from current station
 * @last_tx_rx_ts: Last tx/rx timestamp with current station
 * @assoc_ts: Current station association timestamp
 * @tx_rate: Tx rate with current station reported from F/W
 * @rx_rate: Rx rate with current station reported from F/W
 * @ampdu: Ampdu enable or not of the station
 * @sgi_enable: Short GI enable or not of the station
 * @tx_stbc: Tx Space-time block coding enable/disable
 * @rx_stbc: Rx Space-time block coding enable/disable
 * @ch_width: Channel Width of the connection
 * @mode: Mode of the connection
 * @max_supp_idx: Max supported rate index of the station
 * @max_ext_idx: Max extended supported rate index of the station
 * @max_mcs_idx: Max supported mcs index of the station
 * @rx_mcs_map: VHT Rx mcs map
 * @tx_mcs_map: VHT Tx mcs map
 * @freq : Frequency of the current station
 * @dot11_mode: 802.11 Mode of the connection
 * @ht_present: HT caps present or not in the current station
 * @vht_present: VHT caps present or not in the current station
 * @ht_caps: HT capabilities of current station
 * @vht_caps: VHT capabilities of current station
 * @reason_code: Disconnection reason code for current station
 * @rssi: RSSI of the current station reported from F/W
 * @capability: Capability information of current station
 * @support_mode: Max supported mode of a station currently
 * connected to sap
 * @rx_retry_cnt: Number of rx retries received from current station
 *                Currently this feature is not supported from FW
 * @rx_mc_bc_cnt: Multicast broadcast packet count received from
 *                current station
 * MSB of rx_mc_bc_cnt indicates whether FW supports rx_mc_bc_cnt
 * feature or not, if first bit is 1 it indicates that FW supports this
 * feature, if it is 0 it indicates FW doesn't support this feature
 * @sta_info: The sta_info node for the station info list maintained in adapter
 * @assoc_req_ies: Assoc request IEs of the peer station
 * @ref_cnt: Reference count to synchronize sta_info access
 * @pending_eap_frm_type: EAP frame type in tx queue without tx completion
 */
struct hdd_station_info {
	bool in_use;
	uint8_t sta_id;
	eStationType sta_type;
	struct qdf_mac_addr sta_mac;
	enum ol_txrx_peer_state peer_state;
	bool is_qos_enabled;
	bool is_deauth_in_progress;
	uint8_t   nss;
	uint32_t  rate_flags;
	uint8_t   ecsa_capable;
	uint32_t max_phy_rate;
	uint32_t tx_packets;
	uint64_t tx_bytes;
	uint32_t rx_packets;
	uint64_t rx_bytes;
	qdf_time_t last_tx_rx_ts;
	qdf_time_t assoc_ts;
	qdf_time_t disassoc_ts;
	uint32_t tx_rate;
	uint32_t rx_rate;
	bool ampdu;
	bool sgi_enable;
	bool tx_stbc;
	bool rx_stbc;
	tSirMacHTChannelWidth ch_width;
	uint8_t mode;
	uint8_t max_supp_idx;
	uint8_t max_ext_idx;
	uint8_t max_mcs_idx;
	uint8_t rx_mcs_map;
	uint8_t tx_mcs_map;
	uint32_t freq;
	uint8_t dot11_mode;
	bool ht_present;
	bool vht_present;
	struct ieee80211_ht_cap ht_caps;
	struct ieee80211_vht_cap vht_caps;
	uint32_t reason_code;
	int8_t rssi;
	enum dhcp_phase dhcp_phase;
	enum dhcp_nego_status dhcp_nego_status;
	uint16_t capability;
	uint8_t support_mode;
	uint32_t rx_retry_cnt;
	uint32_t rx_mc_bc_cnt;
	struct qdf_ht_entry sta_node;
	struct wlan_ies assoc_req_ies;
	qdf_atomic_t ref_cnt;
	unsigned long pending_eap_frm_type;
};

/**
 * struct hdd_sta_info_obj - Station info container structure
 * @sta_obj: The sta info object that stores the sta_info
 * @sta_obj_lock: Lock to protect the sta_obj read/write access
 */
struct hdd_sta_info_obj {
	qdf_ht_declare(sta_obj, WLAN_HDD_STA_INFO_SIZE);
	qdf_spinlock_t sta_obj_lock;
};

/**
 * hdd_for_each_station - Iterate over each station stored in the sta info
 *                        container
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 * @sta_info: The station info structure that acts as the iterator object.
 * @index: The current index in which the current station is present.
 *
 * The sta_info will contain the structure that is fetched for that particular
 * iteration. The index of the current iterator object in the container
 * represents the bucket at which the given station info is stored.
 */
#define hdd_for_each_station(sta_info_container, sta_info, index) \
	  qdf_ht_for_each(sta_info_container.sta_obj, index, sta_info, sta_node)

/**
 * hdd_for_each_station_safe - Iterate over each station stored in the sta info
 *                           container being safe for removal of the sta info
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 * @sta_info: The station info structure that acts as the iterator object.
 * @index: The current index in which the current station is present.
 * @tmp: A &struct for temporary storage
 *
 * The sta_info will contain the structure that is fetched for that particular
 * iteration. The index of the current iterator object in the container
 * represents the bucket at which the given station info is stored.
 */
#define hdd_for_each_station_safe(sta_info_container, sta_info, index, tmp) \
	qdf_ht_for_each_safe(sta_info_container.sta_obj, index, tmp, \
			     sta_info, sta_node)

/**
 * wlan_sta_info_init() - Initialise the wlan hdd station info container obj
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 *
 * Return: QDF_STATUS_SUCCESS on success, failure code otherwise
 */
QDF_STATUS hdd_sta_info_init(struct hdd_sta_info_obj *sta_info_container);

/**
 * wlan_sta_info_deinit() - Deinit the wlan hdd station info container obj
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 *
 * Return: None
 */
void hdd_sta_info_deinit(struct hdd_sta_info_obj *sta_info_container);

/**
 * hdd_sta_info_detach() - Detach the station info structure from the list
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 * @sta_info: The station info structure that has to be detached from the
 *            container object.
 *
 * Return: None
 */
void hdd_sta_info_detach(struct hdd_sta_info_obj *sta_info_container,
			 struct hdd_station_info **sta_info);

/**
 * hdd_sta_info_attach() - Attach the station info structure into the list
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 * @sta_info: The station info structure that is to be attached to the
 *            container object.
 *
 * Return: QDF STATUS SUCCESS on successful attach, error code otherwise
 */
QDF_STATUS hdd_sta_info_attach(struct hdd_sta_info_obj *sta_info_container,
			       struct hdd_station_info *sta_info);

/**
 * hdd_get_sta_info_by_mac() - Find the sta_info structure by mac addr
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 * @mac_addr: The mac addr by which the sta_info has to be fetched.
 *
 * Return: Pointer to the hdd_station_info structure which contains the mac
 *         address passed
 */
struct hdd_station_info *hdd_get_sta_info_by_mac(
				struct hdd_sta_info_obj *sta_info_container,
				const uint8_t *mac_addr);

/**
 * hdd_put_sta_info() - Release sta_info for synchronization
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 * @sta_info: Station info structure to be released.
 *
 * Return: None
 */
void hdd_put_sta_info(struct hdd_sta_info_obj *sta_info_container,
		      struct hdd_station_info **sta_info, bool lock_required);

/**
 * hdd_clear_cached_sta_info() - Clear the cached sta info from the container
 * @sta_info_container: The station info container obj that stores and maintains
 *                      the sta_info obj.
 *
 * Return: None
 */
void hdd_clear_cached_sta_info(struct hdd_sta_info_obj *sta_info_container);

#endif /* __WLAN_HDD_STA_INFO_H */
