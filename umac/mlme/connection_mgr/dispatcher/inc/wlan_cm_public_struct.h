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
#include "wlan_crypto_global_def.h"
#include "qdf_status.h"

#define CM_ID_INVALID 0xFFFFFFFF
typedef uint32_t wlan_cm_id;

/* Diconnect active timeout */
#define DISCONNECT_TIMEOUT   STOP_RESPONSE_TIMER + DELETE_RESPONSE_TIMER + 1000

/* Diconnect command wait timeout */
#define CM_DISCONNECT_CMD_TIMEOUT DISCONNECT_TIMEOUT + 2000

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

/**
 * struct wlan_cm_connect_crypto_info - Crypto settings
 * @wpa_versions: indicates which, if any, WPA versions are enabled
 *	(from enum nl80211_wpa_versions)
 * @auth_type: Auth mode type bitmask
 * @group_cipher: group key cipher suite bitmask
 * @ciphers_pairwise: unicast key cipher suites bitmask
 * @akm_suites: AKM suites bitmask
 * @wep_keys: static WEP keys, if not NULL points to an array of
 *	MAX_WEP_KEYS WEP keys
 * @rsn_caps: rsn caps
 * @mgmt_ciphers: mgmt cipher bitmask
 */
struct wlan_cm_connect_crypto_info {
	uint32_t wpa_versions;
	uint32_t auth_type;
	uint32_t group_cipher;
	uint32_t ciphers_pairwise;
	uint32_t akm_suites;
	struct wlan_cm_wep_key_params wep_keys;
	uint16_t rsn_caps;
	uint32_t mgmt_ciphers;
};

#ifdef WLAN_FEATURE_FILS_SK
#define WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH 253
#define WLAN_CM_FILS_MAX_REALM_LEN 255
#define WLAN_CM_FILS_MAX_RRK_LENGTH 64

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
 * @CM_OSIF_CONNECT: Connect req initiated by OSIF or north bound
 * @CM_ROAMING: Roaming request
 * @CM_OSIF_DISCONNECT: Disconnect req initiated by OSIF or north bound
 * @CM_PEER_DISCONNECT: Disconnect req initiated by peer sending deauth/disassoc
 * only for this localy generated will be false while indicating to kernel
 * @CM_SB_DISCONNECT: Disconnect initiated from firmware. eg HB failure,
 * sta kickout etc
 * @CM_MLME_DISCONNECT: Disconnect req initiated by mlme. eg VDEV mgr or any
 * other mlme component.
 * @CM_INTERNAL_DISCONNECT: Internal disconnect initiated by Connection manager
 * on receiving the back to back commands
 * @CM_ROAM_DISCONNECT: Disconnect req due to HO failure
 * @CM_SOURCE_MAX: max value of connection manager source
 * @CM_SOURCE_INVALID: Invalid connection manager req source
 */
enum wlan_cm_source {
	CM_OSIF_CONNECT,
	CM_ROAMING,
	CM_OSIF_DISCONNECT,
	CM_PEER_DISCONNECT,
	CM_SB_DISCONNECT,
	CM_MLME_DISCONNECT,
	CM_INTERNAL_DISCONNECT,
	CM_ROAM_DISCONNECT,
	CM_SOURCE_MAX,
	CM_SOURCE_INVALID = CM_SOURCE_MAX,
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
 * @assoc_ie:Additional assoc IE to be appended in assoc req
 *           (Include RSN/WPA/WAPI/WPS ies)
 * @scan_ie: Default scan ie to be used in the uncast probe req and connect scan
 * @force_rsne_override: force the arbitrary rsne received in connect req to be
 * used with out validation, used for the scenarios where the device is used
 * as a testbed device with special functionality and not recommended
 * for production.
 * @dot11mode_filter: dot11mode filter used to restrict connection to
 * 11n/11ac/11ax.
 * @sae_pwe: SAE mechanism for PWE derivation
 *           0 = hunting-and-pecking loop only
 *           1 = hash-to-element only
 *           2 = both hunting-and-pecking loop and hash-to-element enabled
 * @ht_caps: ht capability information bit mask
 * @ht_caps_mask: mask of valid ht caps
 * @vht_caps: vht capability information bit mask
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
	struct element_info assoc_ie;
	struct element_info scan_ie;
	bool force_rsne_override;
	enum dot11_mode_filter dot11mode_filter;
	uint8_t sae_pwe;
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
 * @force_rsne_override: force the arbitrary rsne received in connect req to be
 * used with out validation, used for the scenarios where the device is used
 * as a testbed device with special functionality and not recommended
 * for production.
 * @ht_caps: ht capability
 * @ht_caps_mask: mask of valid ht caps
 * @vht_caps: vht capability
 * @vht_caps_mask: mask of valid vht caps
 * @assoc_ie: assoc ie to be used in assoc req
 * @scan_ie: Default scan ie to be used in the uncast probe req
 * @bss: scan entry for the candidate
 * @fils_info: Fills related connect info
 */
struct wlan_cm_vdev_connect_req {
	uint8_t vdev_id;
	wlan_cm_id cm_id;
	bool force_rsne_override;
	uint16_t ht_caps;
	uint16_t ht_caps_mask;
	uint32_t vht_caps;
	uint32_t vht_caps_mask;
	struct element_info assoc_ie;
	struct element_info scan_ie;
	struct scan_cache_node *bss;
#ifdef WLAN_FEATURE_FILS_SK
	struct wlan_fils_con_info *fils_info;
#endif
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
	enum wlan_reason_code reason_code;
	struct qdf_mac_addr bssid;
};

/**
 * struct wlan_cm_vdev_discon_req - disconnect req from connection manager to
 * vdev mgr
 * @cm_id: connection manager ID
 * @req: disconnect req
 */
struct wlan_cm_vdev_discon_req {
	wlan_cm_id cm_id;
	struct wlan_cm_disconnect_req req;
};

/*
 * enum wlan_cm_connect_fail_reason: connection manager connect fail reason
 * @CM_NO_CANDIDATE_FOUND: No candidate found
 * @CM_ABORT_DUE_TO_NEW_REQ_RECVD: Aborted as new command is received and
 * @CM_BSS_SELECT_IND_FAILED: Failed BSS select indication
 * State machine is not able to handle as state has changed due to new command.
 * @CM_PEER_CREATE_FAILED: peer create failed
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
 * @CM_SER_TIMEOUT: Serialization cmd timeout
 * @CM_GENERIC_FAILURE: Generic failure apart from above
 */
enum wlan_cm_connect_fail_reason {
	CM_NO_CANDIDATE_FOUND,
	CM_ABORT_DUE_TO_NEW_REQ_RECVD,
	CM_BSS_SELECT_IND_FAILED,
	CM_PEER_CREATE_FAILED,
	CM_JOIN_FAILED,
	CM_JOIN_TIMEOUT,
	CM_AUTH_FAILED,
	CM_AUTH_TIMEOUT,
	CM_ASSOC_FAILED,
	CM_ASSOC_TIMEOUT,
	CM_HW_MODE_FAILURE,
	CM_SER_FAILURE,
	CM_SER_TIMEOUT,
	CM_GENERIC_FAILURE,
};

#ifdef WLAN_FEATURE_FILS_SK
#define CM_FILS_MAX_HLP_DATA_LEN 2048
#define MAX_KEK_LENGTH 64
#define MAX_TK_LENGTH 32
#define MAX_GTK_LENGTH 255

/**
 * struct fils_connect_rsp_params - fils related connect rsp params
 * @fils_pmk: fils pmk
 * @fils_pmk_len: fils pmk length
 * @fils_pmkid: fils pmkid
 * @kek: kek
 * @kek_len: kek length
 * @tk: tk
 * @tk_len: tk length
 * @gtk: gtk
 * @gtk_len: gtk length
 * @dst_mac: dst mac
 * @src_mac: src mac
 * @hlp_data: hlp data
 * @hlp_data_len: hlp data length
 * @fils_seq_num: FILS sequence number
 */
struct fils_connect_rsp_params {
	uint8_t *fils_pmk;
	uint8_t fils_pmk_len;
	uint8_t fils_pmkid[PMKID_LEN];
	uint8_t kek[MAX_KEK_LENGTH];
	uint8_t kek_len;
	uint8_t tk[MAX_TK_LENGTH];
	uint8_t tk_len;
	uint8_t gtk[MAX_GTK_LENGTH];
	uint8_t gtk_len;
	struct qdf_mac_addr dst_mac;
	struct qdf_mac_addr src_mac;
	uint8_t hlp_data[CM_FILS_MAX_HLP_DATA_LEN];
	uint16_t hlp_data_len;
	uint16_t fils_seq_num;
};
#endif

/**
 * struct connect_rsp_ies - connect rsp ies stored in vdev filled during connect
 * @bcn_probe_rsp: Raw beacon or probe rsp of connected AP
 * @assoc_req: assoc req IE pointer send during conenct
 * @assoc_rsq: assoc rsp IE received during connection
 * @ric_resp_ie: ric ie from assoc resp received during connection
 * @fills_ie: fills connection ie received during connection
 */
struct wlan_connect_rsp_ies {
	struct element_info bcn_probe_rsp;
	struct element_info assoc_req;
	struct element_info assoc_rsp;
	struct element_info ric_resp_ie;
#ifdef WLAN_FEATURE_FILS_SK
	struct fils_connect_rsp_params *fils_ie;
#endif
};

/**
 * struct wlan_cm_connect_rsp - connect resp from VDEV mgr and will be sent to
 * OSIF
 * @vdev_id: vdev id
 * @cm_id: Connect manager id
 * @bssid: BSSID of the ap
 * @ssid: SSID of the connection
 * @freq: Channel frequency
 * @connect_status: connect status success or failure
 * @reason: connect fail reason, valid only in case of failure
 * @status_code: protocol status code received in auth/assoc resp
 * @aid: aid
 * @connect_ies: connect related IE required by osif to send to kernel
 * @is_fils_connection: is fils connection
 */
struct wlan_cm_connect_resp {
	uint8_t vdev_id;
	wlan_cm_id cm_id;
	struct qdf_mac_addr bssid;
	struct wlan_ssid ssid;
	qdf_freq_t freq;
	QDF_STATUS connect_status;
	enum wlan_cm_connect_fail_reason reason;
	enum wlan_status_code status_code;
	uint8_t aid;
	struct wlan_connect_rsp_ies connect_ies;
#ifdef WLAN_FEATURE_FILS_SK
	bool is_fils_connection;
#endif
};


/**
 * struct wlan_cm_discon_rsp - disconnect resp from VDEV mgr and will be sent to
 * OSIF
 * @req: disconnect req sent to vdev mgr
 * @ap_discon_ie: disconnect IE sent by AP
 */
struct wlan_cm_discon_rsp {
	struct wlan_cm_vdev_discon_req req;
	struct element_info ap_discon_ie;
};

/*
 * enum wlan_cm_active_request_type: CM active req type
 * @CM_NONE: No active serialisation command
 * @CM_CONNECT_ACTIVE: Connect active in serialisation
 * @CM_DISCONNECT_ACTIVE: DicConnect active in serialisation
 */
enum wlan_cm_active_request_type {
	CM_NONE,
	CM_CONNECT_ACTIVE,
	CM_DISCONNECT_ACTIVE,
};

#endif /* FEATURE_CM_ENABLE */

#endif /* __WLAN_CM_PUBLIC_STRUCT_H__ */
