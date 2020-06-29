/*
 * Copyright (c) 2012-2015, 2020, The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_cm_public_struct.h
 *
 * This header file maintain public structures required for connection mgr
 */

#ifndef __WLAN_CM_PUBLIC_STRUCT_H__
#define __WLAN_CM_PUBLIC_STRUCT_H__

#ifdef FEATURE_CM_ENABLE
#include <wlan_scan_public_structs.h>

/**
 * struct wlan_cm_wep_key_params - store wep key info
 * @key: key info
 * @seq: seq info
 * @key_len: key length
 * @seq_len: seq len
 * @key_idx: key index
 */
struct wlan_cm_wep_key_params {
	uint8_t *key;
	uint8_t *seq;
	uint8_t key_len;
	uint8_t seq_len;
	uint8_t key_idx;
};

#define WLAN_CM_MAX_NR_CIPHER_SUITES 5
#define WLAN_CM_MAX_NR_AKM_SUITES 2

/**
 * struct wlan_cm_connect_crypto_info - Crypto settings
 * @wpa_versions: indicates which, if any, WPA versions are enabled
 *	(from enum nl80211_wpa_versions)
 * @group_cipher: group key cipher suite (or 0 if unset)
 * @n_ciphers_pairwise: number of AP supported unicast ciphers
 * @ciphers_pairwise: unicast key cipher suites
 * @n_akm_suites: number of AKM suites
 * @akm_suites: AKM suites
 * @wep_keys: static WEP keys, if not NULL points to an array of
 *	MAX_WEP_KEYS WEP keys
 * @pmf_cap: Pmf capability
 */
struct wlan_cm_connect_crypto_info {
	uint32_t wpa_versions;
	wlan_crypto_auth_mode auth_type;
	wlan_crypto_cipher_type group_cipher;
	uint32_t n_ciphers_pairwise;
	wlan_crypto_cipher_type ciphers_pairwise[WLAN_CM_MAX_NR_CIPHER_SUITES];
	uint32_t n_akm_suites;
	wlan_crypto_key_mgmt akm_suites[WLAN_CM_MAX_NR_AKM_SUITES];
	struct wlan_cm_wep_key_params wep_keys;
	enum wlan_pmf_cap pmf_cap;
	uint32_t rsn_ie_len;
	uint8_t *rsn_ie;
};

#ifdef WLAN_FEATURE_FILS_SK
#define WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH 253
#define WLAN_CM_FILS_MAX_REALM_LEN 255
#define WLAN_CM_FILS_MAX_RRK_LENGTH 64
#define WLAN_CM_FILS_MAX_RIK_LENGTH CM_FILS_MAX_RRK_LENGTH

/**
 * struct wlan_fils_con_info - fils connect req info
 * @is_fils_connection: is fils connection
 * @username_len: username length
 * @username: username
 * @realm_len: realm length
 * @realm: realm
 * @next_seq_num: next seq number
 * @rrk_len: rrk length
 * @rrk: rrk
 */
struct wlan_fils_con_info {
	bool is_fils_connection;
	uint32_t username_len;
	uint8_t username[WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH];
	uint32_t realm_len;
	uint8_t realm[WLAN_CM_FILS_MAX_REALM_LEN];
	uint16_t next_seq_num;
	uint32_t rrk_len;
	uint8_t rrk[WLAN_CM_FILS_MAX_RRK_LENGTH];
};
#endif

/**
 * enum wlan_cm_source - connection manager req source
 * @CM_OSIF_CONENCT_REQ: Connect req initiated by OSIF or north bound
 * @CM_ROAMING: Roaming request
 * @CM_OSIF_DISCONNECT: Disconnect req initiated by OSIF or north bound
 * @CM_PEER_DISCONNECT: Disconnect req initiated by peer sending deauth/disassoc
 * only for this localy generated will be false while indicating to kernel
 * @CM_SB_DISCONNECT: Disconnect req initiated by South bound/VDEV mgr/Peer mgr
 * @CM_INTERNAL_DISCONNECT: Internal disconnect initiated by Connection manager
 * on receiving the back to back commands
 * @CM_ROAM_DISCONNECT: Disconnect req due to HO failure
 */
enum wlan_cm_source {
	CM_OSIF_CONENCT_REQ,
	CM_ROAMING,
	CM_OSIF_DISCONNECT,
	CM_PEER_DISCONNECT,
	CM_SB_DISCONNECT,
	CM_INTERNAL_DISCONNECT,
	CM_ROAM_DISCONNECT,
};

/**
 * struct wlan_cm_connect_req - connect req from requester
 * @vdev_id: vdev id
 * @source: source of the req
 * @bssid: bssid given
 * @prev_bssid: prev AP bssid, given in case supplican want to roam to new BSSID
 * @ssid: profile SSID
 * @bssid_hint: bssid hint to connect
 * @chan_freq: channel of the AP
 * @crypto: crypto related info
 * @connect_ie: connect IE additional assoc IE
 * @ht_caps: ht capability
 * @ht_caps_mask: mask of valid ht caps
 * @vht_caps: vht capability
 * @vht_caps_mask: mask of valid vht caps
 * @fils_info: Fills related connect info
 */
struct wlan_cm_connect_req {
	uint8_t vdev_id;
	enum wlan_cm_source source;
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr prev_bssid;
	struct wlan_ssid ssid;
	struct qdf_mac_addr bssid_hint;
	uint32_t chan_freq;
	struct wlan_cm_connect_crypto_info crypto;
	struct element_info connect_ie;
	uint16_t ht_caps;
	uint16_t ht_caps_mask;
	uint32_t vht_caps;
	uint32_t vht_caps_mask;
#ifdef WLAN_FEATURE_FILS_SK
	struct wlan_fils_con_info fils_info;
#endif
};

/**
 * struct wlan_cm_vdev_connect_req - connect req from connection manager to
 * vdev mgr
 * @vdev_id: vdev id
 * @cm_id: Connect manager id
 * @bss: scan entry for the candidate
 */
struct wlan_cm_vdev_connect_req {
	uint8_t vdev_id;
	uint8_t cm_id;
	struct scan_cache_node *bss;
};

/**
 * struct wlan_cm_disconnect_req - disconnect req from requester
 * @vdev_id: vdev id
 * @source: source of disconnect
 * @reason_code: protocol/propitiatory reason code of the disconnect.
 * propitiatory will be used to send in
 * QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_DRIVER_DISCONNECT_REASON
 * @bssid: bssid of AP
 */
struct wlan_cm_disconnect_req {
	uint8_t vdev_id;
	enum wlan_cm_source source;
	uint16_t reason_code;
	struct qdf_mac_addr bssid;
};

/**
 * struct wlan_cm_vdev_discon_req - disconnect req from connection manager to
 * vdev mgr
 * @cm_id: connection manager ID
 * @req: disconnect req
 */
struct wlan_cm_vdev_discon_req {
	uint8_t cm_id;
	struct wlan_cm_disconnect_req req;
};

/*
 * enum wlan_cm_connect_fail_reason: connection manager connect fail reason
 * @CM_NO_CANDIDATE_FOUND: No candidate found
 * @CM_JOIN_FAILED: Failed in joining state
 * (BSS peer creation or other handling)
 * @CM_JOIN_TIMEOUT: Did not receive beacon or probe response after unicast
 * probe request
 * @CM_AUTH_FAILED: Auth rejected by AP
 * @CM_AUTH_TIMEOUT: No Auth resp from AP
 * @CM_ASSOC_FAILED: Assoc rejected by AP
 * @CM_ASSOC_TIMEOUT: No Assoc resp from AP
 * @CM_HW_MODE_FAILURE: failed to change HW mode
 * @CM_SER_FAILURE: Failed to serialize command
 * @CM_GENERIC_FAILURE: Generic failure apart from above
 */
enum wlan_cm_connect_fail_reason {
	CM_NO_CANDIDATE_FOUND,
	CM_JOIN_FAILED,
	CM_JOIN_TIMEOUT,
	CM_AUTH_FAILED,
	CM_AUTH_TIMEOUT,
	CM_ASSOC_FAILED,
	CM_ASSOC_TIMEOUT,
	CM_HW_MODE_FAILURE,
	CM_SER_FAILURE,
	CM_GENERIC_FAILURE,
};

/**
 * struct wlan_cm_connect_rsp - connect resp from VDEV mgr and will be sent to
 * OSIF
 * @vdev_id: vdev id
 * @cm_id: Connect manager id
 * @connect_status: connect status success or failure
 * @reason: connect fail reason
 * @reason_code: protocol reason code of the connect failure
 * @peer_macaddr: bssid of AP
 */
struct wlan_cm_connect_rsp {
	uint8_t vdev_id;
	uint8_t cm_id;
	uint8_t connect_status;
	enum wlan_cm_connect_fail_reason reason;
	uint8_t failure_code;
	uint8_t aid;
};

/**
 * struct wlan_cm_discon_rsp - disconnect resp from VDEV mgr and will be sent to
 * OSIF
 * @req: disconnect req sent to vdev mgr
 */
struct wlan_cm_discon_rsp {
	struct wlan_cm_vdev_discon_req req;
};

#endif /* FEATURE_CM_ENABLE */

#endif /* __WLAN_CM_PUBLIC_STRUCT_H__ */
