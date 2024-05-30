/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * struct mlo_sta_csa _params - CSA request parameters in mlo mgr
 * @csa_param: csa parameters
 * @link_id: the link index of AP which triggers CSA
 * @mlo_csa_synced: Before vdev is up, csa information is only saved but not
 *                  handled, and this value is false. Once vdev is up, the saved
 *                  csa information is handled, and this value is changed to
 *                  true. Note this value will be true if the vdev is doing
 *                  restart.
 * @csa_offload_event_recvd: True if WMI_CSA_HANDLING_EVENTID is already
 *                           received. False if this is the first
 *                           WMI_CSA_HANDLING_EVENTID.
 * @valid_csa_param: True once csa_param is filled.
 */
struct mlo_sta_csa_params {
	struct csa_offload_params csa_param;
	uint8_t link_id;
	bool mlo_csa_synced;
	bool csa_offload_event_recvd;
	bool valid_csa_param;
};

/*
 * struct mlo_sta_quiet_status - MLO sta quiet status
 * @link_id: link id
 * @quiet_status: true if corresponding ap in quiet status
 * @valid_status: true if mlo_sta_quiet_status is filled
 */
struct mlo_sta_quiet_status {
	uint8_t link_id;
	bool quiet_status;
	bool valid_status;
};

/*
 * struct wlan_mlo_sta - MLO sta additional info
 * @wlan_connect_req_links: list of vdevs selected for connection with the MLAP
 * @wlan_connected_links: list of vdevs associated with this MLO connection
 * @connect req: connect params
 * @copied_conn_req: original connect req
 * @copied_conn_req_lock: lock for the original connect request
 * @assoc_rsp: Raw assoc response frame
 * @mlo_csa_param: CSA request parameters for mlo sta
 */
struct wlan_mlo_sta {
	qdf_bitmap(wlan_connect_req_links, WLAN_UMAC_MLO_MAX_VDEVS);
	qdf_bitmap(wlan_connected_links, WLAN_UMAC_MLO_MAX_VDEVS);
	struct wlan_mlo_key_mgmt key_mgmt[WLAN_UMAC_MLO_MAX_VDEVS - 1];
	struct wlan_cm_connect_req *connect_req;
	struct wlan_cm_connect_req *copied_conn_req;
#ifdef WLAN_MLO_USE_SPINLOCK
	qdf_spinlock_t copied_conn_req_lock;
#else
	qdf_mutex_t copied_conn_req_lock;
#endif
	struct element_info assoc_rsp;
	struct mlo_sta_quiet_status mlo_quiet_status[WLAN_UMAC_MLO_MAX_VDEVS];
	struct mlo_sta_csa_params mlo_csa_param[WLAN_UMAC_MLO_MAX_VDEVS];
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
 * @vdev_id: VDEV ID
 */
struct mlo_link_info {
	struct qdf_mac_addr link_addr;
	uint8_t link_id;
	uint16_t chan_freq;
	uint8_t vdev_id;
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
 * @mlo_mlme_ext_clone_security_param: Callback to clone mlo security params
 * @mlo_mlme_ext_handle_sta_csa_param: Callback to handle sta csa param
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
	QDF_STATUS (*mlo_mlme_ext_clone_security_param)(
		    struct vdev_mlme_obj *vdev_mlme,
		    struct wlan_cm_connect_req *req);
	void (*mlo_mlme_ext_handle_sta_csa_param)(
				struct wlan_objmgr_vdev *vdev,
				struct csa_offload_params *csa_param);
};

/* maximum size of vdev bitmap array for MLO link set active command */
#define MLO_VDEV_BITMAP_SZ 2

/* maximum size of link number param array for MLO link set active command */
#define MLO_LINK_NUM_SZ 2

/**
 * enum mlo_link_force_mode: MLO link force modes
 * @MLO_LINK_FORCE_MODE_ACTIVE:
 *  Force specific links active
 * @MLO_LINK_FORCE_MODE_INACTIVE:
 *  Force specific links inactive
 * @MLO_LINK_FORCE_MODE_ACTIVE_NUM:
 *  Force active a number of links, firmware to decide which links to inactive
 * @MLO_LINK_FORCE_MODE_INACTIVE_NUM:
 *  Force inactive a number of links, firmware to decide which links to inactive
 * @MLO_LINK_FORCE_MODE_NO_FORCE:
 *  Cancel the force operation of specific links, allow firmware to decide
 */
enum mlo_link_force_mode {
	MLO_LINK_FORCE_MODE_ACTIVE       = 1,
	MLO_LINK_FORCE_MODE_INACTIVE     = 2,
	MLO_LINK_FORCE_MODE_ACTIVE_NUM   = 3,
	MLO_LINK_FORCE_MODE_INACTIVE_NUM = 4,
	MLO_LINK_FORCE_MODE_NO_FORCE     = 5,
};

/**
 * enum mlo_link_force_reason: MLO link force reasons
 * @MLO_LINK_FORCE_REASON_CONNECT:
 *  Set force specific links because of new connection
 * @MLO_LINK_FORCE_REASON_DISCONNECT:
 *  Set force specific links because of new dis-connection
 */
enum mlo_link_force_reason {
	MLO_LINK_FORCE_REASON_CONNECT    = 1,
	MLO_LINK_FORCE_REASON_DISCONNECT = 2,
};

/**
 * struct mlo_link_set_active_resp: MLO link set active response structure
 * @status: Return status, 0 for success, non-zero otherwise
 * @active_sz: size of current active vdev bitmap array
 * @active: current active vdev bitmap array
 * @inactive_sz: size of current inactive vdev bitmap array
 * @inactive: current inactive vdev bitmap array
 */
struct mlo_link_set_active_resp {
	uint32_t status;
	uint32_t active_sz;
	uint32_t active[MLO_VDEV_BITMAP_SZ];
	uint32_t inactive_sz;
	uint32_t inactive[MLO_VDEV_BITMAP_SZ];
};

/**
 * struct mlo_link_num_param: MLO link set active number params
 * @num_of_link: number of links to active/inactive
 * @vdev_type: type of vdev
 * @vdev_subtype: subtype of vdev
 * @home_freq: home frequency of the link
 */
struct mlo_link_num_param {
	uint32_t num_of_link;
	uint32_t vdev_type;
	uint32_t vdev_subtype;
	uint32_t home_freq;
};

/**
 * struct mlo_link_set_active_param: MLO link set active params
 * @force_mode: operation to take (enum mlo_link_force_mode)
 * @reason: reason for the operation (enum mlo_link_force_reason)
 * @num_link_entry: number of the valid entries for link_num
 * @num_vdev_bitmap: number of the valid entries for vdev_bitmap
 * @link_num: link number param array
 *  It's present only when force_mode is MLO_LINK_FORCE_MODE_ACTIVE_NUM or
 *  MLO_LINK_FORCE_MODE_INACTIVE_NUM
 * @vdev_bitmap: active/inactive vdev bitmap array
 *  It will be present when force_mode is MLO_LINK_FORCE_MODE_ACTIVE,
 *  MLO_LINK_FORCE_MODE_INACTIVE, MLO_LINK_FORCE_MODE_NO_FORCE,
 *  MLO_LINK_FORCE_MODE_ACTIVE_NUM or MLO_LINK_FORCE_MODE_INACTIVE_NUM
 */
struct mlo_link_set_active_param {
	uint32_t force_mode;
	uint32_t reason;
	uint32_t num_link_entry;
	uint32_t num_vdev_bitmap;
	struct mlo_link_num_param link_num[MLO_LINK_NUM_SZ];
	uint32_t vdev_bitmap[MLO_VDEV_BITMAP_SZ];
};

/*
 * struct mlo_link_set_active_ctx - Context for MLO link set active request
 * @vdev: pointer to vdev on which the request issued
 * @cb: callback function for MLO link set active request
 * @cb_arg: callback context
 */
struct mlo_link_set_active_ctx {
	struct wlan_objmgr_vdev *vdev;
	void (*set_mlo_link_cb)(struct wlan_objmgr_vdev *vdev, void *arg,
				struct mlo_link_set_active_resp *evt);
	void *cb_arg;
};

/*
 * struct mlo_link_set_active_req - MLO link set active request
 * @ctx: context for MLO link set active request
 * @param: MLO link set active params
 */
struct mlo_link_set_active_req {
	struct mlo_link_set_active_ctx ctx;
	struct mlo_link_set_active_param param;
};
#endif
