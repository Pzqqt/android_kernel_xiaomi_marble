/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <qdf_nbuf.h>
#include <wlan_cmn_ieee80211.h>
#include <wlan_cmn.h>
#include <wlan_objmgr_global_obj.h>

/* MAX MLO dev support */
#define WLAN_UMAC_MLO_MAX_VDEVS 2

/* MAX instances of ML devices */
#define WLAN_UMAC_MLO_MAX_DEV 2

/* Max PEER support */
#define MAX_MLO_PEER 512

struct mlo_mlme_ext_ops;
struct vdev_mlme_obj;

/* Max LINK PEER support */
#define MAX_MLO_LINK_PEERS WLAN_UMAC_MLO_MAX_VDEVS

#define MAX_MLO_PEER_ID 2048
#define MLO_INVALID_PEER_ID 0xFFFF

/* IE nomenclature */
#define ID_POS 0
#define TAG_LEN_POS 1
#define IDEXT_POS 2
#define MIN_IE_LEN 2
#define MULTI_LINK_CTRL_1 3
#define MULTI_LINK_CTRL_2 4
#define STA_CTRL_1 2
#define STA_CTRL_2 3
#define STA_PROFILE_SUB_ELEM_ID 0
#define PER_STA_PROF_MAC_ADDR_START 4

#ifdef WLAN_MLO_MULTI_CHIP
/*
 * enum MLO_LINK_STATE – MLO link state enums
 * @MLO_LINK_SETUP_INIT - MLO link SETUP exchange not yet done
 * @MLO_LINK_SETUP_DONE - MLO link SETUP exchange started
 * @MLO_LINK_READY - MLO link SETUP done and READY sent
 * @MLO_LINK_TEARDOWN - MLO teardown done.
 */
enum MLO_LINK_STATE {
	MLO_LINK_SETUP_INIT,
	MLO_LINK_SETUP_DONE,
	MLO_LINK_READY,
	MLO_LINK_TEARDOWN
};

/**
 * struct mlo_setup_info: MLO setup status per link
 * @ml_grp_id: Unique id for ML grouping of Pdevs/links
 * @tot_socs: Total number of soc participating in ML group
 * @num_soc: Number of soc ready or probed
 * @tot_links: Total links in ML group
 * @num_links: Number of links probed in ML group
 * @pdev_list[MAX_MLO_LINKS]: pdev pointers belonging to this group
 * @soc_list[MAX_MLO_CHIPS]: psoc pointers belonging to this group
 * @state[MAX_MLO_LINKS]: MLO link state
 * @state_lock: lock to protect access to link state
 */
#define MAX_MLO_LINKS 6
#define MAX_MLO_CHIPS 3
struct mlo_setup_info {
	uint8_t ml_grp_id;
	uint8_t tot_socs;
	uint8_t num_soc;
	uint8_t tot_links;
	uint8_t num_links;
	struct wlan_objmgr_pdev *pdev_list[MAX_MLO_LINKS];
	struct wlan_objmgr_psoc *soc_list[MAX_MLO_CHIPS];
	enum MLO_LINK_STATE state[MAX_MLO_LINKS];
	qdf_spinlock_t state_lock;
};

#define MAX_MLO_GROUP 1
#endif

/*
 * struct mlo_mgr_context - MLO manager context
 * @ml_dev_list_lock: ML DEV list lock
 * @aid_lock: AID global lock
 * @ml_peerid_lock: ML peer ID global lock
 * @context: Array of MLO device context
 * @mlo_peer_id_bmap: bitmap to allocate MLO Peer ID
 * @max_mlo_peer_id: Max MLO Peer ID
 * @info: MLO setup info
 * @mlme_ops: MLO MLME callback function pointers
 * @msgq_ctx: Context switch mgr
 * @mlo_is_force_primary_umac: Force Primary UMAC enable
 * @mlo_forced_primary_umac_id: Force Primary UMAC ID
 * @dp_handle: pointer to DP ML context
 */
struct mlo_mgr_context {
#ifdef WLAN_MLO_USE_SPINLOCK
	qdf_spinlock_t ml_dev_list_lock;
	qdf_spinlock_t aid_lock;
	qdf_spinlock_t ml_peerid_lock;
#else
	qdf_mutex_t ml_dev_list_lock;
	qdf_mutex_t aid_lock;
	qdf_mutex_t ml_peerid_lock;
#endif
	qdf_list_t ml_dev_list;
	qdf_bitmap(mlo_peer_id_bmap, MAX_MLO_PEER_ID);
	uint16_t max_mlo_peer_id;
#ifdef WLAN_MLO_MULTI_CHIP
	struct mlo_setup_info setup_info;
#endif
	struct mlo_mlme_ext_ops *mlme_ops;
	struct ctxt_switch_mgr *msgq_ctx;
	bool mlo_is_force_primary_umac;
	uint8_t mlo_forced_primary_umac_id;
	void *dp_handle;
};

/*
 * struct wlan_ml_vdev_aid_mgr – ML AID manager
 * @aid_bitmap: AID bitmap array
 * @start_aid: start of AID index
 * @max_aid: Max allowed AID
 * @aid_mgr[]:  Array of link vdev aid mgr
 */
struct wlan_ml_vdev_aid_mgr {
	qdf_bitmap(aid_bitmap, WLAN_UMAC_MAX_AID);
	uint16_t start_aid;
	uint16_t max_aid;
	struct wlan_vdev_aid_mgr *aid_mgr[WLAN_UMAC_MLO_MAX_VDEVS];
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
 * @orig_conn_req: original connect req
 * @assoc_rsp: Raw assoc response frame
 */
struct wlan_mlo_sta {
	qdf_bitmap(wlan_connect_req_links, WLAN_UMAC_MLO_MAX_VDEVS);
	qdf_bitmap(wlan_connected_links, WLAN_UMAC_MLO_MAX_VDEVS);
	struct wlan_mlo_key_mgmt key_mgmt[WLAN_UMAC_MLO_MAX_VDEVS - 1];
	struct wlan_cm_connect_req *connect_req;
	struct wlan_cm_connect_req *orig_conn_req;
	struct element_info assoc_rsp;
};

/*
 * struct wlan_mlo_ap - MLO AP related info
 * @num_ml_vdevs: number of vdevs to form MLD
 * @ml_aid_mgr: ML AID mgr
 * @mlo_vdev_quiet_bmap: Bitmap of vdevs for which quiet ie needs to enabled
 */
struct wlan_mlo_ap {
	uint8_t num_ml_vdevs;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;
	qdf_bitmap(mlo_vdev_quiet_bmap, WLAN_UMAC_MLO_MAX_VDEVS);
};

/*
 * struct wlan_mlo_peer_list - MLO peer list entry
 * @peer_hash: MLO peer hash code
 * @peer_list_lock: lock to access members of structure
 */
struct wlan_mlo_peer_list {
	qdf_list_t peer_hash[WLAN_PEER_HASHSIZE];
#ifdef WLAN_MLO_USE_SPINLOCK
	qdf_spinlock_t peer_list_lock;
#else
	qdf_mutex_t peer_list_lock;
#endif
};

/*
 * struct wlan_mlo_dev_context - MLO device context
 * @node: QDF list node member
 * @mld_id: MLD id
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
	struct wlan_mlo_peer_list mlo_peer_list;
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
 * @link_ix: Link index
 * @is_primary: sets true if the peer is primary UMAC’s peer
 * @hw_link_id: HW Link id of peer
 * @assoc_rsp_buf: Assoc resp buffer
 */
struct wlan_mlo_link_peer_entry {
	struct wlan_objmgr_peer *link_peer;
	struct qdf_mac_addr link_addr;
	uint8_t link_ix;
	bool is_primary;
	uint8_t hw_link_id;
	qdf_nbuf_t assoc_rsp_buf;
};

/*
 * enum mlo_peer_state – MLO peer state
 * @ML_PEER_CREATED:     Initial state
 * @ML_PEER_ASSOC_DONE:  ASSOC sent on assoc link
 * @ML_PEER_DISCONN_INITIATED: Disconnect initiated on one of the links
 */
enum mlo_peer_state {
	ML_PEER_CREATED,
	ML_PEER_ASSOC_DONE,
	ML_PEER_DISCONN_INITIATED,
};

/*
 * struct wlan_mlo_peer_context - MLO peer context
 *
 * @peer_node:     peer list node for ml_dev qdf list
 * @peer_list: list of peers on the MLO link
 * @link_peer_cnt: Number of link peers attached
 * @max_links: Max links for this ML peer
 * @mlo_peer_id: unique ID for the peer
 * @peer_mld_addr: MAC address of MLD link
 * @mlo_ie: MLO IE struct
 * @mlo_peer_lock: lock to access peer strucutre
 * @assoc_id: Assoc ID derived by MLO manager
 * @ref_cnt: Reference counter to avoid use after free
 * @ml_dev: MLO dev context
 * @mlpeer_state: MLO peer state
 * @avg_link_rssi: avg RSSI of ML peer
 */
struct wlan_mlo_peer_context {
	qdf_list_node_t peer_node;
	struct wlan_mlo_link_peer_entry peer_list[MAX_MLO_LINK_PEERS];
	uint8_t link_peer_cnt;
	uint8_t max_links;
	uint32_t mlo_peer_id;
	struct qdf_mac_addr peer_mld_addr;
	uint8_t *mlo_ie;
#ifdef WLAN_MLO_USE_SPINLOCK
	qdf_spinlock_t mlo_peer_lock;
#else
	qdf_mutex_t mlo_peer_lock;
#endif
	uint16_t assoc_id;
	uint8_t primary_umac_psoc_id;
	qdf_atomic_t ref_cnt;
	struct wlan_mlo_dev_context *ml_dev;
	enum mlo_peer_state mlpeer_state;
	int8_t avg_link_rssi;
};

/*
 * struct mlo_link_info – ML link info
 * @link_addr: link mac address
 * @link_id: link index
 * @chan_freq: Operating channel frequency
 */
struct mlo_link_info {
	struct qdf_mac_addr link_addr;
	uint8_t link_id;
	uint16_t chan_freq;
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

/*
 * struct mlo_tgt_link_info – ML target link info
 * @vdev_id: link peer vdev id
 * @hw_mld_link_id: HW link id
 */
struct mlo_tgt_link_info {
	uint8_t vdev_id;
	uint8_t hw_mld_link_id;
};

/*
 * struct mlo_tgt_partner_info – mlo target partner link info
 * @num_partner_links: no. of partner links
 * @link_info: per partner link info
 */
struct mlo_tgt_partner_info {
	uint8_t num_partner_links;
	struct mlo_tgt_link_info link_info[WLAN_UMAC_MLO_MAX_VDEVS];
};

/*
 * struct mlo_mlme_ext_ops - MLME callback functions
 * @mlo_mlme_ext_validate_conn_req: Callback to validate connect request
 * @mlo_mlme_ext_create_link_vdev: Callback to create link vdev for ML STA
 * @mlo_mlme_ext_peer_create: Callback to create link peer
 * @mlo_mlme_ext_peer_assoc: Callback to initiate peer assoc
 * @mlo_mlme_ext_peer_assoc_fail: Callback to notify peer assoc failure
 * @mlo_mlme_ext_peer_delete: Callback to initiate link peer delete
 * @mlo_mlme_ext_assoc_resp: Callback to initiate assoc resp
 * @mlo_mlme_get_link_assoc_req: Calback to get link assoc req buffer
 * @mlo_mlme_ext_deauth: Callback to initiate deauth
 */
struct mlo_mlme_ext_ops {
	QDF_STATUS (*mlo_mlme_ext_validate_conn_req)(
		    struct vdev_mlme_obj *vdev_mlme, void *ext_data);
	QDF_STATUS (*mlo_mlme_ext_create_link_vdev)(
		    struct vdev_mlme_obj *vdev_mlme, void *ext_data);
	QDF_STATUS (*mlo_mlme_ext_peer_create)(struct wlan_objmgr_vdev *vdev,
					struct wlan_mlo_peer_context *ml_peer,
					struct qdf_mac_addr *addr,
					qdf_nbuf_t frm_buf);
	void (*mlo_mlme_ext_peer_assoc)(struct wlan_objmgr_peer *peer);
	void (*mlo_mlme_ext_peer_assoc_fail)(struct wlan_objmgr_peer *peer);
	void (*mlo_mlme_ext_peer_delete)(struct wlan_objmgr_peer *peer);
	void (*mlo_mlme_ext_assoc_resp)(struct wlan_objmgr_peer *peer);
	qdf_nbuf_t (*mlo_mlme_get_link_assoc_req)(struct wlan_objmgr_peer *peer,
						  uint8_t link_ix);
	void (*mlo_mlme_ext_deauth)(struct wlan_objmgr_peer *peer);
};

#endif
