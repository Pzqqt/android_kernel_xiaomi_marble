/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#ifndef _DP_PEER_H_
#define _DP_PEER_H_

#include <qdf_types.h>
#include <qdf_lock.h>
#include "dp_types.h"

#define DP_INVALID_PEER_ID 0xffff

#define DP_FW_PEER_STATS_CMP_TIMEOUT_MSEC 5000

typedef void dp_peer_iter_func(struct dp_soc *soc, struct dp_peer *peer,
			       void *arg);
void dp_peer_unref_delete(struct dp_peer *peer, enum dp_mod_id id);

/**
 * dp_peer_get_ref() - Returns peer object given the peer id
 *
 * @soc		: core DP soc context
 * @peer	: DP peer
 * @mod_id	: id of module requesting the reference
 *
 * Return:	QDF_STATUS_SUCCESS if reference held successfully
 *		else QDF_STATUS_E_INVAL
 */
static inline
QDF_STATUS dp_peer_get_ref(struct dp_soc *soc,
			   struct dp_peer *peer,
			   enum dp_mod_id mod_id)
{
	if (!qdf_atomic_inc_not_zero(&peer->ref_cnt))
		return QDF_STATUS_E_INVAL;

	if (mod_id > DP_MOD_ID_RX)
		qdf_atomic_inc(&peer->mod_refs[mod_id]);

	return QDF_STATUS_SUCCESS;
}

/**
 * __dp_peer_get_ref_by_id() - Returns peer object given the peer id
 *
 * @soc		: core DP soc context
 * @peer_id	: peer id from peer object can be retrieved
 * @mod_id	: module id
 *
 * Return: struct dp_peer*: Pointer to DP peer object
 */
static inline struct dp_peer *
__dp_peer_get_ref_by_id(struct dp_soc *soc,
			uint16_t peer_id,
			enum dp_mod_id mod_id)

{
	struct dp_peer *peer;

	qdf_spin_lock_bh(&soc->peer_map_lock);
	peer = (peer_id >= soc->max_peers) ? NULL :
				soc->peer_id_to_obj_map[peer_id];
	if (!peer ||
	    (dp_peer_get_ref(soc, peer, mod_id) != QDF_STATUS_SUCCESS)) {
		qdf_spin_unlock_bh(&soc->peer_map_lock);
		return NULL;
	}

	qdf_spin_unlock_bh(&soc->peer_map_lock);
	return peer;
}

/**
 * dp_peer_get_ref_by_id() - Returns peer object given the peer id
 *                        if peer state is active
 *
 * @soc		: core DP soc context
 * @peer_id	: peer id from peer object can be retrieved
 * @mod_id      : ID ot module requesting reference
 *
 * Return: struct dp_peer*: Pointer to DP peer object
 */
static inline
struct dp_peer *dp_peer_get_ref_by_id(struct dp_soc *soc,
				      uint16_t peer_id,
				      enum dp_mod_id mod_id)
{
	struct dp_peer *peer;

	qdf_spin_lock_bh(&soc->peer_map_lock);
	peer = (peer_id >= soc->max_peers) ? NULL :
				soc->peer_id_to_obj_map[peer_id];

	if (!peer || peer->peer_state >= DP_PEER_STATE_LOGICAL_DELETE ||
	    (dp_peer_get_ref(soc, peer, mod_id) != QDF_STATUS_SUCCESS)) {
		qdf_spin_unlock_bh(&soc->peer_map_lock);
		return NULL;
	}

	qdf_spin_unlock_bh(&soc->peer_map_lock);

	return peer;
}

#ifdef PEER_CACHE_RX_PKTS
/**
 * dp_rx_flush_rx_cached() - flush cached rx frames
 * @peer: peer
 * @drop: set flag to drop frames
 *
 * Return: None
 */
void dp_rx_flush_rx_cached(struct dp_peer *peer, bool drop);
#else
static inline void dp_rx_flush_rx_cached(struct dp_peer *peer, bool drop)
{
}
#endif

static inline void
dp_clear_peer_internal(struct dp_soc *soc, struct dp_peer *peer)
{
	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_DISC;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	dp_rx_flush_rx_cached(peer, true);
}

/**
 * dp_vdev_iterate_peer() - API to iterate through vdev peer list
 *
 * @vdev	: DP vdev context
 * @func	: function to be called for each peer
 * @arg		: argument need to be passed to func
 * @mod_id	: module_id
 *
 * Return: void
 */
static inline void
dp_vdev_iterate_peer(struct dp_vdev *vdev, dp_peer_iter_func *func, void *arg,
		     enum dp_mod_id mod_id)
{
	struct dp_peer *peer;
	struct dp_peer *tmp_peer;
	struct dp_soc *soc = NULL;

	if (!vdev || !vdev->pdev || !vdev->pdev->soc)
		return;

	soc = vdev->pdev->soc;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	TAILQ_FOREACH_SAFE(peer, &vdev->peer_list,
			   peer_list_elem,
			   tmp_peer) {
		if (dp_peer_get_ref(soc, peer, mod_id) ==
					QDF_STATUS_SUCCESS) {
			(*func)(soc, peer, arg);
			dp_peer_unref_delete(peer, mod_id);
		}
	}
	qdf_spin_unlock_bh(&vdev->peer_list_lock);
}

/**
 * dp_pdev_iterate_peer() - API to iterate through all peers of pdev
 *
 * @pdev	: DP pdev context
 * @func	: function to be called for each peer
 * @arg		: argument need to be passed to func
 * @mod_id	: module_id
 *
 * Return: void
 */
static inline void
dp_pdev_iterate_peer(struct dp_pdev *pdev, dp_peer_iter_func *func, void *arg,
		     enum dp_mod_id mod_id)
{
	struct dp_vdev *vdev;

	if (!pdev)
		return;

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev)
		dp_vdev_iterate_peer(vdev, func, arg, mod_id);
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);
}

/**
 * dp_soc_iterate_peer() - API to iterate through all peers of soc
 *
 * @soc		: DP soc context
 * @func	: function to be called for each peer
 * @arg		: argument need to be passed to func
 * @mod_id	: module_id
 *
 * Return: void
 */
static inline void
dp_soc_iterate_peer(struct dp_soc *soc, dp_peer_iter_func *func, void *arg,
		    enum dp_mod_id mod_id)
{
	struct dp_pdev *pdev;
	int i;

	if (!soc)
		return;

	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		dp_pdev_iterate_peer(pdev, func, arg, mod_id);
	}
}

/**
 * dp_vdev_iterate_peer_lock_safe() - API to iterate through vdev list
 *
 * This API will cache the peers in local allocated memory and calls
 * iterate function outside the lock.
 *
 * As this API is allocating new memory it is suggested to use this
 * only when lock cannot be held
 *
 * @vdev	: DP vdev context
 * @func	: function to be called for each peer
 * @arg		: argument need to be passed to func
 * @mod_id	: module_id
 *
 * Return: void
 */
static inline void
dp_vdev_iterate_peer_lock_safe(struct dp_vdev *vdev,
			       dp_peer_iter_func *func,
			       void *arg,
			       enum dp_mod_id mod_id)
{
	struct dp_peer *peer;
	struct dp_peer *tmp_peer;
	struct dp_soc *soc = NULL;
	struct dp_peer **peer_array = NULL;
	int i = 0;
	uint32_t num_peers = 0;

	if (!vdev || !vdev->pdev || !vdev->pdev->soc)
		return;

	num_peers = vdev->num_peers;

	soc = vdev->pdev->soc;

	peer_array = qdf_mem_malloc(num_peers * sizeof(struct dp_peer *));
	if (!peer_array)
		return;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	TAILQ_FOREACH_SAFE(peer, &vdev->peer_list,
			   peer_list_elem,
			   tmp_peer) {
		if (i >= num_peers)
			break;

		if (dp_peer_get_ref(soc, peer, mod_id) == QDF_STATUS_SUCCESS) {
			peer_array[i] = peer;
			i = (i + 1);
		}
	}
	qdf_spin_unlock_bh(&vdev->peer_list_lock);

	for (i = 0; i < num_peers; i++) {
		peer = peer_array[i];

		if (!peer)
			continue;

		(*func)(soc, peer, arg);
		dp_peer_unref_delete(peer, mod_id);
	}

	qdf_mem_free(peer_array);
}

/**
 * dp_pdev_iterate_peer_lock_safe() - API to iterate through all peers of pdev
 *
 * This API will cache the peers in local allocated memory and calls
 * iterate function outside the lock.
 *
 * As this API is allocating new memory it is suggested to use this
 * only when lock cannot be held
 *
 * @pdev	: DP pdev context
 * @func	: function to be called for each peer
 * @arg		: argument need to be passed to func
 * @mod_id	: module_id
 *
 * Return: void
 */
static inline void
dp_pdev_iterate_peer_lock_safe(struct dp_pdev *pdev,
			       dp_peer_iter_func *func,
			       void *arg,
			       enum dp_mod_id mod_id)
{
	struct dp_peer *peer;
	struct dp_peer *tmp_peer;
	struct dp_soc *soc = NULL;
	struct dp_vdev *vdev = NULL;
	struct dp_peer **peer_array[DP_PDEV_MAX_VDEVS] = {0};
	int i = 0;
	int j = 0;
	uint32_t num_peers[DP_PDEV_MAX_VDEVS] = {0};

	if (!pdev || !pdev->soc)
		return;

	soc = pdev->soc;

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
		num_peers[i] = vdev->num_peers;
		peer_array[i] = qdf_mem_malloc(num_peers[i] *
					       sizeof(struct dp_peer *));
		if (!peer_array[i])
			break;

		qdf_spin_lock_bh(&vdev->peer_list_lock);
		TAILQ_FOREACH_SAFE(peer, &vdev->peer_list,
				   peer_list_elem,
				   tmp_peer) {
			if (j >= num_peers[i])
				break;

			if (dp_peer_get_ref(soc, peer, mod_id) ==
					QDF_STATUS_SUCCESS) {
				peer_array[i][j] = peer;

				j = (j + 1);
			}
		}
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		i = (i + 1);
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);

	for (i = 0; i < DP_PDEV_MAX_VDEVS; i++) {
		if (!peer_array[i])
			break;

		for (j = 0; j < num_peers[i]; j++) {
			peer = peer_array[i][j];

			if (!peer)
				continue;

			(*func)(soc, peer, arg);
			dp_peer_unref_delete(peer, mod_id);
		}

		qdf_mem_free(peer_array[i]);
	}
}

/**
 * dp_soc_iterate_peer_lock_safe() - API to iterate through all peers of soc
 *
 * This API will cache the peers in local allocated memory and calls
 * iterate function outside the lock.
 *
 * As this API is allocating new memory it is suggested to use this
 * only when lock cannot be held
 *
 * @soc		: DP soc context
 * @func	: function to be called for each peer
 * @arg		: argument need to be passed to func
 * @mod_id	: module_id
 *
 * Return: void
 */
static inline void
dp_soc_iterate_peer_lock_safe(struct dp_soc *soc,
			      dp_peer_iter_func *func,
			      void *arg,
			      enum dp_mod_id mod_id)
{
	struct dp_pdev *pdev;
	int i;

	if (!soc)
		return;

	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		dp_pdev_iterate_peer_lock_safe(pdev, func, arg, mod_id);
	}
}

#ifdef DP_PEER_STATE_DEBUG
#define DP_PEER_STATE_ASSERT(_peer, _new_state, _condition) \
	do {  \
		if (!(_condition)) { \
			dp_alert("Invalid state shift from %u to %u peer " \
				 QDF_MAC_ADDR_FMT, \
				 (_peer)->peer_state, (_new_state), \
				 QDF_MAC_ADDR_REF((_peer)->mac_addr.raw)); \
			QDF_ASSERT(0); \
		} \
	} while (0)

#else
#define DP_PEER_STATE_ASSERT(_peer, _new_state, _condition) \
	do {  \
		if (!(_condition)) { \
			dp_alert("Invalid state shift from %u to %u peer " \
				 QDF_MAC_ADDR_FMT, \
				 (_peer)->peer_state, (_new_state), \
				 QDF_MAC_ADDR_REF((_peer)->mac_addr.raw)); \
		} \
	} while (0)
#endif

/**
 * dp_peer_state_cmp() - compare dp peer state
 *
 * @peer	: DP peer
 * @state	: state
 *
 * Return: true if state matches with peer state
 *	   false if it does not match
 */
static inline bool
dp_peer_state_cmp(struct dp_peer *peer,
		  enum dp_peer_state state)
{
	bool is_status_equal = false;

	qdf_spin_lock_bh(&peer->peer_state_lock);
	is_status_equal = (peer->peer_state == state);
	qdf_spin_unlock_bh(&peer->peer_state_lock);

	return is_status_equal;
}

/**
 * dp_peer_update_state() - update dp peer state
 *
 * @soc		: core DP soc context
 * @peer	: DP peer
 * @state	: new state
 *
 * Return: None
 */
static inline void
dp_peer_update_state(struct dp_soc *soc,
		     struct dp_peer *peer,
		     enum dp_peer_state state)
{
	uint8_t peer_state;

	qdf_spin_lock_bh(&peer->peer_state_lock);
	peer_state = peer->peer_state;

	switch (state) {
	case DP_PEER_STATE_INIT:
		DP_PEER_STATE_ASSERT
			(peer, state, (peer_state != DP_PEER_STATE_ACTIVE) ||
			 (peer_state != DP_PEER_STATE_LOGICAL_DELETE));
		break;

	case DP_PEER_STATE_ACTIVE:
		DP_PEER_STATE_ASSERT(peer, state,
				     (peer_state == DP_PEER_STATE_INIT));
		break;

	case DP_PEER_STATE_LOGICAL_DELETE:
		DP_PEER_STATE_ASSERT(peer, state,
				     (peer_state == DP_PEER_STATE_ACTIVE) ||
				     (peer_state == DP_PEER_STATE_INIT));
		break;

	case DP_PEER_STATE_INACTIVE:
		DP_PEER_STATE_ASSERT
			(peer, state,
			 (peer_state == DP_PEER_STATE_LOGICAL_DELETE));
		break;

	case DP_PEER_STATE_FREED:
		if (peer->sta_self_peer)
			DP_PEER_STATE_ASSERT
			(peer, state, (peer_state == DP_PEER_STATE_INIT));
		else
			DP_PEER_STATE_ASSERT
				(peer, state,
				 (peer_state == DP_PEER_STATE_INACTIVE) ||
				 (peer_state == DP_PEER_STATE_LOGICAL_DELETE));
		break;

	default:
		dp_alert("Invalid peer state %u for peer "QDF_MAC_ADDR_FMT,
			 state, QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		qdf_spin_unlock_bh(&peer->peer_state_lock);
		return;
	}
	dp_info("Updating peer state from %u to %u mac "QDF_MAC_ADDR_FMT"\n",
		peer_state, state,
		QDF_MAC_ADDR_REF(peer->mac_addr.raw));
	peer->peer_state = state;
	qdf_spin_unlock_bh(&peer->peer_state_lock);
}

void dp_print_ast_stats(struct dp_soc *soc);
QDF_STATUS dp_rx_peer_map_handler(struct dp_soc *soc, uint16_t peer_id,
				  uint16_t hw_peer_id, uint8_t vdev_id,
				  uint8_t *peer_mac_addr, uint16_t ast_hash,
				  uint8_t is_wds);
void dp_rx_peer_unmap_handler(struct dp_soc *soc, uint16_t peer_id,
			      uint8_t vdev_id, uint8_t *peer_mac_addr,
			      uint8_t is_wds, uint32_t free_wds_count);
void dp_rx_sec_ind_handler(struct dp_soc *soc, uint16_t peer_id,
			   enum cdp_sec_type sec_type, int is_unicast,
			   u_int32_t *michael_key, u_int32_t *rx_pn);

QDF_STATUS dp_rx_delba_ind_handler(void *soc_handle, uint16_t peer_id,
				   uint8_t tid, uint16_t win_sz);

uint8_t dp_get_peer_mac_addr_frm_id(struct cdp_soc_t *soc_handle,
		uint16_t peer_id, uint8_t *peer_mac);

QDF_STATUS dp_peer_add_ast(struct dp_soc *soc, struct dp_peer *peer,
			   uint8_t *mac_addr, enum cdp_txrx_ast_entry_type type,
			   uint32_t flags);

void dp_peer_del_ast(struct dp_soc *soc, struct dp_ast_entry *ast_entry);

void dp_peer_ast_unmap_handler(struct dp_soc *soc,
			       struct dp_ast_entry *ast_entry);

int dp_peer_update_ast(struct dp_soc *soc, struct dp_peer *peer,
			struct dp_ast_entry *ast_entry,	uint32_t flags);

struct dp_ast_entry *dp_peer_ast_hash_find_by_pdevid(struct dp_soc *soc,
						     uint8_t *ast_mac_addr,
						     uint8_t pdev_id);

struct dp_ast_entry *dp_peer_ast_hash_find_by_vdevid(struct dp_soc *soc,
						     uint8_t *ast_mac_addr,
						     uint8_t vdev_id);

struct dp_ast_entry *dp_peer_ast_hash_find_soc(struct dp_soc *soc,
					       uint8_t *ast_mac_addr);

uint8_t dp_peer_ast_get_pdev_id(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry);


uint8_t dp_peer_ast_get_next_hop(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry);

void dp_peer_ast_set_type(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry,
				enum cdp_txrx_ast_entry_type type);

void dp_peer_ast_send_wds_del(struct dp_soc *soc,
			      struct dp_ast_entry *ast_entry,
			      struct dp_peer *peer);

void dp_peer_free_hmwds_cb(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			   struct cdp_soc *dp_soc,
			   void *cookie,
			   enum cdp_ast_free_status status);

void dp_peer_ast_hash_remove(struct dp_soc *soc,
			     struct dp_ast_entry *ase);

void dp_peer_free_ast_entry(struct dp_soc *soc,
			    struct dp_ast_entry *ast_entry);

void dp_peer_unlink_ast_entry(struct dp_soc *soc,
			      struct dp_ast_entry *ast_entry,
			      struct dp_peer *peer);

#define DP_AST_ASSERT(_condition) \
	do { \
		if (!(_condition)) { \
			dp_print_ast_stats(soc);\
			QDF_BUG(_condition); \
		} \
	} while (0)

/**
 * dp_peer_update_inactive_time - Update inactive time for peer
 * @pdev: pdev object
 * @tag_type: htt_tlv_tag type
 * #tag_buf: buf message
 */
void
dp_peer_update_inactive_time(struct dp_pdev *pdev, uint32_t tag_type,
			     uint32_t *tag_buf);

#ifndef QCA_MULTIPASS_SUPPORT
/**
 * dp_peer_set_vlan_id: set vlan_id for this peer
 * @cdp_soc: soc handle
 * @vdev_id: id of vdev object
 * @peer_mac: mac address
 * @vlan_id: vlan id for peer
 *
 * return: void
 */
static inline
void dp_peer_set_vlan_id(struct cdp_soc_t *cdp_soc,
			 uint8_t vdev_id, uint8_t *peer_mac,
			 uint16_t vlan_id)
{
}

/**
 * dp_set_vlan_groupkey: set vlan map for vdev
 * @soc: pointer to soc
 * @vdev_id: id of vdev handle
 * @vlan_id: vlan_id
 * @group_key: group key for vlan
 *
 * return: set success/failure
 */
static inline
QDF_STATUS dp_set_vlan_groupkey(struct cdp_soc_t *soc, uint8_t vdev_id,
				uint16_t vlan_id, uint16_t group_key)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_peer_multipass_list_init: initialize multipass peer list
 * @vdev: pointer to vdev
 *
 * return: void
 */
static inline
void dp_peer_multipass_list_init(struct dp_vdev *vdev)
{
}

/**
 * dp_peer_multipass_list_remove: remove peer from special peer list
 * @peer: peer handle
 *
 * return: void
 */
static inline
void dp_peer_multipass_list_remove(struct dp_peer *peer)
{
}
#else
void dp_peer_set_vlan_id(struct cdp_soc_t *cdp_soc,
			 uint8_t vdev_id, uint8_t *peer_mac,
			 uint16_t vlan_id);
QDF_STATUS dp_set_vlan_groupkey(struct cdp_soc_t *soc, uint8_t vdev_id,
				uint16_t vlan_id, uint16_t group_key);
void dp_peer_multipass_list_init(struct dp_vdev *vdev);
void dp_peer_multipass_list_remove(struct dp_peer *peer);
#endif


#ifndef QCA_PEER_MULTIQ_SUPPORT
/**
 * dp_peer_reset_flowq_map() - reset peer flowq map table
 * @peer - dp peer handle
 *
 * Return: none
 */
static inline
void dp_peer_reset_flowq_map(struct dp_peer *peer)
{
}

/**
 * dp_peer_ast_index_flow_queue_map_create() - create ast index flow queue map
 * @soc - genereic soc handle
 * @is_wds - flag to indicate if peer is wds
 * @peer_id - peer_id from htt peer map message
 * @peer_mac_addr - mac address of the peer
 * @ast_info - ast flow override information from peer map
 *
 * Return: none
 */
static inline
void dp_peer_ast_index_flow_queue_map_create(void *soc_hdl,
		    bool is_wds, uint16_t peer_id, uint8_t *peer_mac_addr,
		    struct dp_ast_flow_override_info *ast_info)
{
}
#else
void dp_peer_reset_flowq_map(struct dp_peer *peer);
void dp_peer_ast_index_flow_queue_map_create(void *soc_hdl,
		    bool is_wds, uint16_t peer_id, uint8_t *peer_mac_addr,
		    struct dp_ast_flow_override_info *ast_info);
#endif

/**
 * dp_peer_update_pkt_capture_params: Set Rx & Tx Capture flags for a peer
 * @soc: DP SOC handle
 * @pdev_id: id of DP pdev handle
 * @is_rx_pkt_cap_enable: enable/disable Rx packet capture in monitor mode
 * @is_tx_pkt_cap_enable: enable/disable/delete/print
 * Tx packet capture in monitor mode
 * Tx packet capture in monitor mode
 * @peer_mac: MAC address for which the above need to be enabled/disabled
 *
 * Return: Success if Rx & Tx capture is enabled for peer, false otherwise
 */
QDF_STATUS
dp_peer_update_pkt_capture_params(ol_txrx_soc_handle soc,
				  uint8_t pdev_id,
				  bool is_rx_pkt_cap_enable,
				  uint8_t is_tx_pkt_cap_enable,
				  uint8_t *peer_mac);

/*
 * dp_rx_tid_delete_cb() - Callback to flush reo descriptor HW cache
 * after deleting the entries (ie., setting valid=0)
 *
 * @soc: DP SOC handle
 * @cb_ctxt: Callback context
 * @reo_status: REO command status
 */
void dp_rx_tid_delete_cb(struct dp_soc *soc,
			 void *cb_ctxt,
			 union hal_reo_status *reo_status);

#ifndef WLAN_TX_PKT_CAPTURE_ENH
/**
 * dp_peer_tid_queue_init() – Initialize ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
static inline void dp_peer_tid_queue_init(struct dp_peer *peer)
{
}

/**
 * dp_peer_tid_peer_id_update() – update peer_id to tid structure
 * @peer: Datapath peer
 * @peer_id: peer_id
 *
 */
static inline
void dp_peer_tid_peer_id_update(struct dp_peer *peer, uint16_t peer_id)
{
}

/**
 * dp_peer_tid_queue_cleanup() – remove ppdu stats queue per TID
 * @peer: Datapath peer
 *
 */
static inline void dp_peer_tid_queue_cleanup(struct dp_peer *peer)
{
}

/**
 * dp_peer_update_80211_hdr() – dp peer update 80211 hdr
 * @vdev: Datapath vdev
 * @peer: Datapath peer
 *
 */
static inline void
dp_peer_update_80211_hdr(struct dp_vdev *vdev, struct dp_peer *peer)
{
}
#endif

#ifdef QCA_PEER_EXT_STATS
QDF_STATUS dp_peer_ext_stats_ctx_alloc(struct dp_soc *soc,
				       struct dp_peer *peer);
void dp_peer_ext_stats_ctx_dealloc(struct dp_soc *soc,
				   struct dp_peer *peer);
#else
static inline QDF_STATUS dp_peer_ext_stats_ctx_alloc(struct dp_soc *soc,
						     struct dp_peer *peer)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_peer_ext_stats_ctx_dealloc(struct dp_soc *soc,
						 struct dp_peer *peer)
{
}
#endif

struct dp_peer *dp_vdev_bss_peer_ref_n_get(struct dp_soc *soc,
					   struct dp_vdev *vdev,
					   enum dp_mod_id mod_id);
struct dp_peer *dp_sta_vdev_self_peer_ref_n_get(struct dp_soc *soc,
						struct dp_vdev *vdev,
						enum dp_mod_id mod_id);

#ifdef FEATURE_AST
/*
 * dp_peer_delete_ast_entries(): Delete all AST entries for a peer
 * @soc - datapath soc handle
 * @peer - datapath peer handle
 *
 * Delete the AST entries belonging to a peer
 */
static inline void dp_peer_delete_ast_entries(struct dp_soc *soc,
					      struct dp_peer *peer)
{
	struct dp_ast_entry *ast_entry, *temp_ast_entry;

	/*
	 * Delete peer self ast entry. This is done to handle scenarios
	 * where peer is freed before peer map is received(for ex in case
	 * of auth disallow due to ACL) in such cases self ast is not added
	 * to peer->ast_list.
	 */
	if (peer->self_ast_entry) {
		dp_peer_del_ast(soc, peer->self_ast_entry);
		peer->self_ast_entry = NULL;
	}

	DP_PEER_ITERATE_ASE_LIST(peer, ast_entry, temp_ast_entry)
		dp_peer_del_ast(soc, ast_entry);
}
#else
static inline void dp_peer_delete_ast_entries(struct dp_soc *soc,
					      struct dp_peer *peer)
{
}
#endif
#endif /* _DP_PEER_H_ */
