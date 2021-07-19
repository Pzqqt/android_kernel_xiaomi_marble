/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/*
 * DOC: contains mlo manager structure definitions
 */
#ifndef __MLO_MGR_PUBLIC_STRUCTS_H
#define __MLO_MGR_PUBLIC_STRUCTS_H

#include <wlan_objmgr_cmn.h>
#include <qdf_list.h>
#include <qdf_atomic.h>
#include <wlan_cmn_ieee80211.h>
#include <wlan_cmn.h>

/* MAX MLO dev support */
#define WLAN_UMAC_MLO_MAX_VDEVS 3

/* MAX instances of ML devices */
#define WLAN_UMAC_MLO_MAX_DEV 2

/* Max PEER support */
#define MAX_MLO_PEER 512

/*
 * struct mlo_setup_info
 * To store the MLO setup related information
 */
struct mlo_setup_info {
};

/*
 * struct mlo_mgr_context - MLO manager context
 * @ml_dev_list_lock: ML device list lock
 * @context: Array of MLO device context
 * @info: MLO setup info
 */
struct mlo_mgr_context {
#ifdef WLAN_MLO_USE_SPINLOCK
	qdf_spinlock_t ml_dev_list_lock;
#else
	qdf_mutex_t ml_dev_list_lock;
#endif
	qdf_list_t ml_dev_list;
	struct mlo_setup_info info;
};

/*
 * struct wlan_mlo_key_mgmt - MLO key management
 * @link_mac_address: list of vdevs selected for connection with the MLAP
 * @ptk: Pairwise transition keys
 * @gtk: Group transition key
 */
struct wlan_mlo_key_mgmt {
	struct qdf_mac_addr link_mac_address;
	uint32_t ptk;
	uint32_t gtk;
};

/*
 * struct wlan_mlo_sta - MLO sta additional info
 * @wlan_connect_req_links: list of vdevs selected for connection with the MLAP
 * @wlan_connected_links: list of vdevs associated with this MLO connection
 * @connect req: connect params
 */
struct wlan_mlo_sta {
	qdf_bitmap(wlan_connect_req_links, WLAN_UMAC_MLO_MAX_VDEVS);
	qdf_bitmap(wlan_connected_links, WLAN_UMAC_MLO_MAX_VDEVS);
	struct wlan_mlo_key_mgmt key_mgmt[WLAN_UMAC_MLO_MAX_VDEVS - 1];
	struct wlan_cm_connect_req *connect_req;
};

/*
 * struct wlan_mlo_ap - MLO AP related info
 */
struct wlan_mlo_ap {
};

/*
 * struct wlan_mlo_peer_list - MLO peer list entry
 * @peer_hash: MLO peer hash code
 * @peer_list_lock: lock to access members of structure
 */
struct wlan_mlo_peer_list {
	qdf_list_t peer_hash[WLAN_PEER_HASHSIZE];
	qdf_spinlock_t peer_list_lock;
};

/*
 * struct wlan_mlo_dev_context - MLO device context
 *
 * @mld_addr: MLO device MAC address
 * @wlan_vdev_list: list of vdevs associated with this MLO connection
 * @wlan_vdev_count: number of elements in the vdev list
 * @mlo_peer: list peers in this MLO connection
 * @wlan_max_mlo_peer_count: peer count across the links of specific MLO
 * @mlo_dev_lock: lock to access struct
 * @ref_cnt: reference count
 * @ref_id_dbg: Reference count debug information
 * @sta_ctx: MLO STA related information
 * @ap_ctx: AP related information
 */
struct wlan_mlo_dev_context {
	qdf_list_node_t node;
	uint8_t mld_id;
	struct qdf_mac_addr mld_addr;
	struct wlan_objmgr_vdev *wlan_vdev_list[WLAN_UMAC_MLO_MAX_VDEVS];
	uint16_t wlan_vdev_count;
	struct wlan_mlo_peer_list mlo_peer;
	uint16_t wlan_max_mlo_peer_count;
#ifdef WLAN_MLO_USE_SPINLOCK
	qdf_spinlock_t mlo_dev_lock;
#else
	qdf_mutex_t mlo_dev_lock;
#endif
	qdf_atomic_t ref_cnt;
	qdf_atomic_t ref_id_dbg[WLAN_REF_ID_MAX];
	struct wlan_mlo_sta *sta_ctx;
	struct wlan_mlo_ap *ap_ctx;
};

/*
 * struct wlan_mlo_link_peer_entry – Link peer entry
 * @link_peer: Object manager peer
 * @link_addr: MAC address of link peer
 * @link_index: Link index
 * @is_primary: sets true if the peer is primary UMAC’s peer
 * @hw_link_id: HW Link id of peer
 */
struct wlan_mlo_link_peer_entry {
	struct wlan_objmgr_peer *link_peer;
	struct qdf_mac_addr link_addr;
	uint8_t link_index;
	bool is_primary;
	uint8_t hw_link_id;
};

/*
 * struct mlo_peer_context - MLO peer context
 *
 * @peer_list: list of peers on the MLO link
 * @link_peer_cnt: Number of link peers attached
 * @mlo_peer_id: unique ID for the peer
 * @peer_mld_addr: MAC address of MLD link
 * @mlo_ie: MLO IE struct
 * @mlo_peer_lock: lock to access peer structure
 * @assoc_id: Assoc ID derived by MLO manager
 * @primary_umac_psoc_id: Primary UMAC PSOC id
 * @ref_cnt: Reference counter to avoid use after free
 */
struct mlo_peer_context {
	struct wlan_mlo_link_peer_entry peer_list[MAX_MLO_PEER];
	uint8_t link_peer_cnt;
	uint32_t mlo_peer_id;
	struct qdf_mac_addr peer_mld_addr;
	uint8_t *mlo_ie;
	qdf_spinlock_t mlo_peer_lock;
	uint16_t assoc_id;
	uint8_t primary_umac_psoc_id;
	qdf_atomic_t ref_cnt;
};

/*
 * struct wlan_vdev_aid_mgr – AID manager
 * @aid_bitmap: AID bitmap array
 * @max_aid: Max allowed AID
 * @ref_cnt:  to share AID across VDEVs for MBSSID
 */
struct wlan_vdev_aid_mgr {
	uint32_t *aid_bitmap;
	uint16_t max_aid;
	qdf_atomic_t ref_cnt;
};

/*
 * struct mlo_link_info – ML link info
 * @link_addr: link mac address
 * @link_id: link index
 */
struct mlo_link_info {
	struct qdf_mac_addr link_addr;
	uint8_t link_id;
};

/*
 * struct mlo_partner_info – mlo partner link info
 * @num_partner_links: no. of partner links
 * @partner_link_info: per partner link info
 */
struct mlo_partner_info {
	uint8_t num_partner_links;
	struct mlo_link_info partner_link_info[WLAN_UMAC_MLO_MAX_VDEVS];
};
#endif
