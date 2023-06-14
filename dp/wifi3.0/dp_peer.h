/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifdef DUMP_REO_QUEUE_INFO_IN_DDR
#include "hal_reo.h"
#endif

#define DP_INVALID_PEER_ID 0xffff

#define DP_PEER_MAX_MEC_IDX 1024	/* maximum index for MEC table */
#define DP_PEER_MAX_MEC_ENTRY 4096	/* maximum MEC entries in MEC table */

#define DP_FW_PEER_STATS_CMP_TIMEOUT_MSEC 5000

#define DP_PEER_HASH_LOAD_MULT  2
#define DP_PEER_HASH_LOAD_SHIFT 0

#define dp_peer_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_PEER, params)
#define dp_peer_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_PEER, params)
#define dp_peer_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_PEER, params)
#define dp_peer_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_PEER, ## params)
#define dp_peer_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_PEER, params)

#ifdef REO_QDESC_HISTORY
enum reo_qdesc_event_type {
	REO_QDESC_UPDATE_CB = 0,
	REO_QDESC_FREE,
};

struct reo_qdesc_event {
	qdf_dma_addr_t qdesc_addr;
	uint64_t ts;
	enum reo_qdesc_event_type type;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
};
#endif

struct ast_del_ctxt {
	bool age;
	int del_count;
};

typedef void dp_peer_iter_func(struct dp_soc *soc, struct dp_peer *peer,
			       void *arg);
void dp_peer_unref_delete(struct dp_peer *peer, enum dp_mod_id id);
struct dp_peer *dp_peer_find_hash_find(struct dp_soc *soc,
				       uint8_t *peer_mac_addr,
				       int mac_addr_is_aligned,
				       uint8_t vdev_id,
				       enum dp_mod_id id);
bool dp_peer_find_by_id_valid(struct dp_soc *soc, uint16_t peer_id);

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
	peer = (peer_id >= soc->max_peer_id) ? NULL :
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
	peer = (peer_id >= soc->max_peer_id) ? NULL :
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
		qdf_spin_unlock_bh(&peer->peer_state_lock);
		dp_alert("Invalid peer state %u for peer "QDF_MAC_ADDR_FMT,
			 state, QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		return;
	}
	peer->peer_state = state;
	qdf_spin_unlock_bh(&peer->peer_state_lock);
	dp_info("Updating peer state from %u to %u mac "QDF_MAC_ADDR_FMT"\n",
		peer_state, state,
		QDF_MAC_ADDR_REF(peer->mac_addr.raw));
}

void dp_print_ast_stats(struct dp_soc *soc);
QDF_STATUS dp_rx_peer_map_handler(struct dp_soc *soc, uint16_t peer_id,
				  uint16_t hw_peer_id, uint8_t vdev_id,
				  uint8_t *peer_mac_addr, uint16_t ast_hash,
				  uint8_t is_wds);
void dp_rx_peer_unmap_handler(struct dp_soc *soc, uint16_t peer_id,
			      uint8_t vdev_id, uint8_t *peer_mac_addr,
			      uint8_t is_wds, uint32_t free_wds_count);

#ifdef DP_RX_UDP_OVER_PEER_ROAM
/**
 * dp_rx_reset_roaming_peer() - Reset the roamed peer in vdev
 * @soc - dp soc pointer
 * @vdev_id - vdev id
 * @peer_mac_addr - mac address of the peer
 *
 * This function resets the roamed peer auth status and mac address
 * after peer map indication of same peer is received from firmware.
 *
 * Return: None
 */
void dp_rx_reset_roaming_peer(struct dp_soc *soc, uint8_t vdev_id,
			      uint8_t *peer_mac_addr);
#else
static inline void dp_rx_reset_roaming_peer(struct dp_soc *soc, uint8_t vdev_id,
					    uint8_t *peer_mac_addr)
{
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * dp_rx_mlo_peer_map_handler() - handle MLO peer map event from firmware
 * @soc_handle - genereic soc handle
 * @peer_id - ML peer_id from firmware
 * @peer_mac_addr - mac address of the peer
 * @mlo_ast_flow_info: MLO AST flow info
 *
 * associate the ML peer_id that firmware provided with peer entry
 * and update the ast table in the host with the hw_peer_id.
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS
dp_rx_mlo_peer_map_handler(struct dp_soc *soc, uint16_t peer_id,
			   uint8_t *peer_mac_addr,
			   struct dp_mlo_flow_override_info *mlo_flow_info);

/**
 * dp_rx_mlo_peer_unmap_handler() - handle MLO peer unmap event from firmware
 * @soc_handle - genereic soc handle
 * @peeri_id - peer_id from firmware
 *
 * Return: none
 */
void dp_rx_mlo_peer_unmap_handler(struct dp_soc *soc, uint16_t peer_id);
#endif

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

/**
 * dp_peer_mec_detach_entry() - Detach the MEC entry
 * @soc: SoC handle
 * @mecentry: MEC entry of the node
 * @ptr: pointer to free list
 *
 * The MEC entry is detached from MEC table and added to free_list
 * to free the object outside lock
 *
 * Return: None
 */
void dp_peer_mec_detach_entry(struct dp_soc *soc, struct dp_mec_entry *mecentry,
			      void *ptr);

/**
 * dp_peer_mec_free_list() - free the MEC entry from free_list
 * @soc: SoC handle
 * @ptr: pointer to free list
 *
 * Return: None
 */
void dp_peer_mec_free_list(struct dp_soc *soc, void *ptr);

/**
 * dp_peer_mec_add_entry()
 * @soc: SoC handle
 * @vdev: vdev to which mec node belongs
 * @mac_addr: MAC address of mec node
 *
 * This function allocates and adds MEC entry to MEC table.
 * It assumes caller has taken the mec lock to protect the access to these
 * tables
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_peer_mec_add_entry(struct dp_soc *soc,
				 struct dp_vdev *vdev,
				 uint8_t *mac_addr);

/**
 * dp_peer_mec_hash_find_by_pdevid() - Find MEC entry by MAC address
 * within pdev
 * @soc: SoC handle
 *
 * It assumes caller has taken the mec_lock to protect the access to
 * MEC hash table
 *
 * Return: MEC entry
 */
struct dp_mec_entry *dp_peer_mec_hash_find_by_pdevid(struct dp_soc *soc,
						     uint8_t pdev_id,
						     uint8_t *mec_mac_addr);

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

void dp_peer_ast_table_detach(struct dp_soc *soc);
void dp_peer_find_map_detach(struct dp_soc *soc);
void dp_soc_wds_detach(struct dp_soc *soc);
QDF_STATUS dp_peer_ast_table_attach(struct dp_soc *soc);
QDF_STATUS dp_peer_ast_hash_attach(struct dp_soc *soc);
QDF_STATUS dp_peer_mec_hash_attach(struct dp_soc *soc);
void dp_soc_wds_attach(struct dp_soc *soc);
void dp_peer_mec_hash_detach(struct dp_soc *soc);
void dp_peer_ast_hash_detach(struct dp_soc *soc);

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

	dp_peer_debug("peer: %pK, self_ast: %pK", peer, peer->self_ast_entry);
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

#ifdef FEATURE_MEC
/**
 * dp_peer_mec_spinlock_create() - Create the MEC spinlock
 * @soc: SoC handle
 *
 * Return: none
 */
void dp_peer_mec_spinlock_create(struct dp_soc *soc);

/**
 * dp_peer_mec_spinlock_destroy() - Destroy the MEC spinlock
 * @soc: SoC handle
 *
 * Return: none
 */
void dp_peer_mec_spinlock_destroy(struct dp_soc *soc);

/**
 * dp_peer_mec_flush_entries() - Delete all mec entries in table
 * @soc: Datapath SOC
 *
 * Return: None
 */
void dp_peer_mec_flush_entries(struct dp_soc *soc);
#else
static inline void dp_peer_mec_spinlock_create(struct dp_soc *soc)
{
}

static inline void dp_peer_mec_spinlock_destroy(struct dp_soc *soc)
{
}

static inline void dp_peer_mec_flush_entries(struct dp_soc *soc)
{
}
#endif

#ifdef DUMP_REO_QUEUE_INFO_IN_DDR
/**
 * dp_send_cache_flush_for_rx_tid() - Send cache flush cmd to REO per tid
 * @soc : dp_soc handle
 * @peer: peer
 *
 * This function is used to send cache flush cmd to reo and
 * to register the callback to handle the dumping of the reo
 * queue stas from DDR
 *
 * Return: none
 */
void dp_send_cache_flush_for_rx_tid(
	struct dp_soc *soc, struct dp_peer *peer);

/**
 * dp_get_rx_reo_queue_info() - Handler to get rx tid info
 * @soc : cdp_soc_t handle
 * @vdev_id: vdev id
 *
 * Handler to get rx tid info from DDR after h/w cache is
 * invalidated first using the cache flush cmd.
 *
 * Return: none
 */
void dp_get_rx_reo_queue_info(
	struct cdp_soc_t *soc_hdl, uint8_t vdev_id);

/**
 * dp_dump_rx_reo_queue_info() - Callback function to dump reo queue stats
 * @soc : dp_soc handle
 * @cb_ctxt - callback context
 * @reo_status: vdev id
 *
 * This is the callback function registered after sending the reo cmd
 * to flush the h/w cache and invalidate it. In the callback the reo
 * queue desc info is dumped from DDR.
 *
 * Return: none
 */
void dp_dump_rx_reo_queue_info(
	struct dp_soc *soc, void *cb_ctxt, union hal_reo_status *reo_status);

#else /* DUMP_REO_QUEUE_INFO_IN_DDR */

static inline void dp_get_rx_reo_queue_info(
	struct cdp_soc_t *soc_hdl, uint8_t vdev_id)
{
}
#endif /* DUMP_REO_QUEUE_INFO_IN_DDR */

static inline int dp_peer_find_mac_addr_cmp(
	union dp_align_mac_addr *mac_addr1,
	union dp_align_mac_addr *mac_addr2)
{
		/*
		 * Intentionally use & rather than &&.
		 * because the operands are binary rather than generic boolean,
		 * the functionality is equivalent.
		 * Using && has the advantage of short-circuited evaluation,
		 * but using & has the advantage of no conditional branching,
		 * which is a more significant benefit.
		 */
	return !((mac_addr1->align4.bytes_abcd == mac_addr2->align4.bytes_abcd)
		 & (mac_addr1->align4.bytes_ef == mac_addr2->align4.bytes_ef));
}

/**
 * dp_peer_delete() - delete DP peer
 *
 * @soc: Datatpath soc
 * @peer: Datapath peer
 * @arg: argument to iter function
 *
 * Return: void
 */
void dp_peer_delete(struct dp_soc *soc,
		    struct dp_peer *peer,
		    void *arg);

#ifdef WLAN_FEATURE_11BE_MLO
/* set peer type */
#define DP_PEER_SET_TYPE(_peer, _type_val) \
	((_peer)->peer_type = (_type_val))
/* is MLO connection link peer */
#define IS_MLO_DP_LINK_PEER(_peer) \
	((_peer)->peer_type == CDP_LINK_PEER_TYPE && (_peer)->mld_peer)
/* is MLO connection mld peer */
#define IS_MLO_DP_MLD_PEER(_peer) \
	((_peer)->peer_type == CDP_MLD_PEER_TYPE)

#ifdef WLAN_MLO_MULTI_CHIP
uint8_t dp_mlo_get_chip_id(struct dp_soc *soc);

struct dp_peer *
dp_link_peer_hash_find_by_chip_id(struct dp_soc *soc,
				  uint8_t *peer_mac_addr,
				  int mac_addr_is_aligned,
				  uint8_t vdev_id,
				  uint8_t chip_id,
				  enum dp_mod_id mod_id);
#else
static inline uint8_t dp_mlo_get_chip_id(struct dp_soc *soc)
{
	return 0;
}

static inline struct dp_peer *
dp_link_peer_hash_find_by_chip_id(struct dp_soc *soc,
				  uint8_t *peer_mac_addr,
				  int mac_addr_is_aligned,
				  uint8_t vdev_id,
				  uint8_t chip_id,
				  enum dp_mod_id mod_id)
{
	return dp_peer_find_hash_find(soc, peer_mac_addr,
				      mac_addr_is_aligned,
				      vdev_id, mod_id);
}
#endif

/**
 * dp_link_peer_add_mld_peer() - add mld peer pointer to link peer,
				 increase mld peer ref_cnt
 * @link_peer: link peer pointer
 * @mld_peer: mld peer pointer
 *
 * Return: none
 */
static inline
void dp_link_peer_add_mld_peer(struct dp_peer *link_peer,
			       struct dp_peer *mld_peer)
{
	/* increase mld_peer ref_cnt */
	dp_peer_get_ref(NULL, mld_peer, DP_MOD_ID_CDP);
	link_peer->mld_peer = mld_peer;
}

/**
 * dp_link_peer_del_mld_peer() - delete mld peer pointer from link peer,
				 decrease mld peer ref_cnt
 * @link_peer: link peer pointer
 *
 * Return: None
 */
static inline
void dp_link_peer_del_mld_peer(struct dp_peer *link_peer)
{
	dp_peer_unref_delete(link_peer->mld_peer, DP_MOD_ID_CDP);
	link_peer->mld_peer = NULL;
}

/**
 * dp_mld_peer_init_link_peers_info() - init link peers info in mld peer
 * @mld_peer: mld peer pointer
 *
 * Return: None
 */
static inline
void dp_mld_peer_init_link_peers_info(struct dp_peer *mld_peer)
{
	int i;

	qdf_spinlock_create(&mld_peer->link_peers_info_lock);
	mld_peer->num_links = 0;
	for (i = 0; i < DP_MAX_MLO_LINKS; i++)
		mld_peer->link_peers[i].is_valid = false;
}

/**
 * dp_mld_peer_deinit_link_peers_info() - Deinit link peers info in mld peer
 * @mld_peer: mld peer pointer
 *
 * Return: None
 */
static inline
void dp_mld_peer_deinit_link_peers_info(struct dp_peer *mld_peer)
{
	qdf_spinlock_destroy(&mld_peer->link_peers_info_lock);
}

/**
 * dp_mld_peer_add_link_peer() - add link peer info to mld peer
 * @mld_peer: mld dp peer pointer
 * @link_peer: link dp peer pointer
 *
 * Return: None
 */
static inline
void dp_mld_peer_add_link_peer(struct dp_peer *mld_peer,
			       struct dp_peer *link_peer)
{
	int i;
	struct dp_peer_link_info *link_peer_info;

	qdf_spin_lock_bh(&mld_peer->link_peers_info_lock);
	for (i = 0; i < DP_MAX_MLO_LINKS; i++) {
		link_peer_info = &mld_peer->link_peers[i];
		if (!link_peer_info->is_valid) {
			qdf_mem_copy(link_peer_info->mac_addr.raw,
				     link_peer->mac_addr.raw,
				     QDF_MAC_ADDR_SIZE);
			link_peer_info->is_valid = true;
			link_peer_info->vdev_id = link_peer->vdev->vdev_id;
			link_peer_info->chip_id =
				dp_mlo_get_chip_id(link_peer->vdev->pdev->soc);
			mld_peer->num_links++;
			break;
		}
	}
	qdf_spin_unlock_bh(&mld_peer->link_peers_info_lock);

	if (i == DP_MAX_MLO_LINKS)
		dp_err("fail to add link peer" QDF_MAC_ADDR_FMT "to mld peer",
		       QDF_MAC_ADDR_REF(link_peer->mac_addr.raw));
}

/**
 * dp_mld_peer_del_link_peer() - Delete link peer info from MLD peer
 * @mld_peer: MLD dp peer pointer
 * @link_peer: link dp peer pointer
 *
 * Return: number of links left after deletion
 */
static inline
uint8_t dp_mld_peer_del_link_peer(struct dp_peer *mld_peer,
				  struct dp_peer *link_peer)
{
	int i;
	struct dp_peer_link_info *link_peer_info;
	uint8_t num_links;

	qdf_spin_lock_bh(&mld_peer->link_peers_info_lock);
	for (i = 0; i < DP_MAX_MLO_LINKS; i++) {
		link_peer_info = &mld_peer->link_peers[i];
		if (link_peer_info->is_valid &&
		    !dp_peer_find_mac_addr_cmp(&link_peer->mac_addr,
					&link_peer_info->mac_addr)) {
			link_peer_info->is_valid = false;
			mld_peer->num_links--;
			break;
		}
	}
	num_links = mld_peer->num_links;
	qdf_spin_unlock_bh(&mld_peer->link_peers_info_lock);

	if (i == DP_MAX_MLO_LINKS)
		dp_err("fail to del link peer" QDF_MAC_ADDR_FMT "to mld peer",
		       QDF_MAC_ADDR_REF(link_peer->mac_addr.raw));

	return num_links;
}

/**
 * dp_get_link_peers_ref_from_mld_peer() - get link peers pointer and
					   increase link peers ref_cnt
 * @soc: dp_soc handle
 * @mld_peer: dp mld peer pointer
 * @mld_link_peers: structure that hold links peers ponter array and number
 * @mod_id: id of module requesting reference
 *
 * Return: None
 */
static inline
void dp_get_link_peers_ref_from_mld_peer(
				struct dp_soc *soc,
				struct dp_peer *mld_peer,
				struct dp_mld_link_peers *mld_link_peers,
				enum dp_mod_id mod_id)
{
	struct dp_peer *peer;
	uint8_t i = 0, j = 0;
	struct dp_peer_link_info *link_peer_info;

	qdf_mem_zero(mld_link_peers, sizeof(*mld_link_peers));
	qdf_spin_lock_bh(&mld_peer->link_peers_info_lock);
	for (i = 0; i < DP_MAX_MLO_LINKS; i++)  {
		link_peer_info = &mld_peer->link_peers[i];
		if (link_peer_info->is_valid) {
			peer = dp_link_peer_hash_find_by_chip_id(
						soc,
						link_peer_info->mac_addr.raw,
						true,
						link_peer_info->vdev_id,
						link_peer_info->chip_id,
						mod_id);
			if (peer)
				mld_link_peers->link_peers[j++] = peer;
		}
	}
	qdf_spin_unlock_bh(&mld_peer->link_peers_info_lock);

	mld_link_peers->num_links = j;
}

/**
 * dp_release_link_peers_ref() - release all link peers reference
 * @mld_link_peers: structure that hold links peers ponter array and number
 * @mod_id: id of module requesting reference
 *
 * Return: None.
 */
static inline
void dp_release_link_peers_ref(
			struct dp_mld_link_peers *mld_link_peers,
			enum dp_mod_id mod_id)
{
	struct dp_peer *peer;
	uint8_t i;

	for (i = 0; i < mld_link_peers->num_links; i++) {
		peer = mld_link_peers->link_peers[i];
		if (peer)
			dp_peer_unref_delete(peer, mod_id);
		mld_link_peers->link_peers[i] = NULL;
	}

	 mld_link_peers->num_links = 0;
}

/**
 * dp_peer_get_tgt_peer_hash_find() - get MLD dp_peer handle
				   for processing
 * @soc: soc handle
 * @peer_mac_addr: peer mac address
 * @mac_addr_is_aligned: is mac addr alligned
 * @vdev_id: vdev_id
 * @mod_id: id of module requesting reference
 *
 * for MLO connection, get corresponding MLD peer,
 * otherwise get link peer for non-MLO case.
 *
 * return: peer in success
 *         NULL in failure
 */
static inline
struct dp_peer *dp_peer_get_tgt_peer_hash_find(struct dp_soc *soc,
					       uint8_t *peer_mac,
					       int mac_addr_is_aligned,
					       uint8_t vdev_id,
					       enum dp_mod_id mod_id)
{
	struct dp_peer *ta_peer = NULL;
	struct dp_peer *peer = dp_peer_find_hash_find(soc,
						      peer_mac, 0, vdev_id,
						      mod_id);

	if (peer) {
		/* mlo connection link peer, get mld peer with reference */
		if (IS_MLO_DP_LINK_PEER(peer)) {
			/* increase mld peer ref_cnt */
			if (QDF_STATUS_SUCCESS ==
			    dp_peer_get_ref(soc, peer->mld_peer, mod_id))
				ta_peer = peer->mld_peer;
			else
				ta_peer = NULL;

			/* relese peer reference that added by hash find */
			dp_peer_unref_delete(peer, mod_id);
		} else {
		/* mlo MLD peer or non-mlo link peer */
			ta_peer = peer;
		}
	}

	return ta_peer;
}

/**
 * dp_peer_get_tgt_peer_by_id() - Returns target peer object given the peer id
 * @soc		: core DP soc context
 * @peer_id	: peer id from peer object can be retrieved
 * @mod_id      : ID ot module requesting reference
 *
 * for MLO connection, get corresponding MLD peer,
 * otherwise get link peer for non-MLO case.
 *
 * return: peer in success
 *         NULL in failure
 */
static inline
struct dp_peer *dp_peer_get_tgt_peer_by_id(struct dp_soc *soc,
					   uint16_t peer_id,
					   enum dp_mod_id mod_id)
{
	struct dp_peer *ta_peer = NULL;
	struct dp_peer *peer = dp_peer_get_ref_by_id(soc, peer_id, mod_id);

	if (peer) {
		/* mlo connection link peer, get mld peer with reference */
		if (IS_MLO_DP_LINK_PEER(peer)) {
			/* increase mld peer ref_cnt */
			if (QDF_STATUS_SUCCESS ==
				dp_peer_get_ref(soc, peer->mld_peer, mod_id))
				ta_peer = peer->mld_peer;
			else
				ta_peer = NULL;

			/* relese peer reference that added by hash find */
			dp_peer_unref_delete(peer, mod_id);
		} else {
		/* mlo MLD peer or non-mlo link peer */
			ta_peer = peer;
		}
	}

	return ta_peer;
}

/**
 * dp_peer_mlo_delete() - peer MLO related delete operation
 * @soc: Soc handle
 * @peer: DP peer handle
 * Return: None
 */
static inline
void dp_peer_mlo_delete(struct dp_soc *soc,
			struct dp_peer *peer)
{
	/* MLO connection link peer */
	if (IS_MLO_DP_LINK_PEER(peer)) {
		/* if last link peer deletion, delete MLD peer */
		if (dp_mld_peer_del_link_peer(peer->mld_peer, peer) == 0)
			dp_peer_delete(soc, peer->mld_peer, NULL);
	}
}

/**
 * dp_peer_mlo_setup() - create MLD peer and MLO related initialization
 * @soc: Soc handle
 * @vdev_id: Vdev ID
 * @peer_setup_info: peer setup information for MLO
 */
QDF_STATUS dp_peer_mlo_setup(
			struct dp_soc *soc,
			struct dp_peer *peer,
			uint8_t vdev_id,
			struct cdp_peer_setup_info *setup_info);

#else
#define DP_PEER_SET_TYPE(_peer, _type_val) /* no op */
#define IS_MLO_DP_LINK_PEER(_peer) false
#define IS_MLO_DP_MLD_PEER(_peer) false

static inline
struct dp_peer *dp_peer_get_tgt_peer_hash_find(struct dp_soc *soc,
					       uint8_t *peer_mac,
					       int mac_addr_is_aligned,
					       uint8_t vdev_id,
					       enum dp_mod_id mod_id)
{
	return dp_peer_find_hash_find(soc, peer_mac,
				      mac_addr_is_aligned, vdev_id,
				      mod_id);
}

static inline
struct dp_peer *dp_peer_get_tgt_peer_by_id(struct dp_soc *soc,
					   uint16_t peer_id,
					   enum dp_mod_id mod_id)
{
	return dp_peer_get_ref_by_id(soc, peer_id, mod_id);
}

static inline
QDF_STATUS dp_peer_mlo_setup(
			struct dp_soc *soc,
			struct dp_peer *peer,
			uint8_t vdev_id,
			struct cdp_peer_setup_info *setup_info)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void dp_mld_peer_init_link_peers_info(struct dp_peer *mld_peer)
{
}

static inline
void dp_mld_peer_deinit_link_peers_info(struct dp_peer *mld_peer)
{
}

static inline
void dp_link_peer_del_mld_peer(struct dp_peer *link_peer)
{
}

static inline
void dp_peer_mlo_delete(struct dp_soc *soc,
			struct dp_peer *peer)
{
}

static inline
void dp_mlo_peer_authorize(struct dp_soc *soc,
			   struct dp_peer *link_peer)
{
}

static inline uint8_t dp_mlo_get_chip_id(struct dp_soc *soc)
{
	return 0;
}

static inline struct dp_peer *
dp_link_peer_hash_find_by_chip_id(struct dp_soc *soc,
				  uint8_t *peer_mac_addr,
				  int mac_addr_is_aligned,
				  uint8_t vdev_id,
				  uint8_t chip_id,
				  enum dp_mod_id mod_id)
{
	return dp_peer_find_hash_find(soc, peer_mac_addr,
				      mac_addr_is_aligned,
				      vdev_id, mod_id);
}
#endif /* WLAN_FEATURE_11BE_MLO */

#if defined(WLAN_FEATURE_11BE_MLO) && defined(WLAN_MLO_MULTI_CHIP)
/**
 * dp_mlo_partner_chips_map() - Map MLO peers to partner SOCs
 * @soc: Soc handle
 * @peer: DP peer handle for ML peer
 * @peer_id: peer_id
 * Return: None
 */
void dp_mlo_partner_chips_map(struct dp_soc *soc,
			      struct dp_peer *peer,
			      uint16_t peer_id);

/**
 * dp_mlo_partner_chips_unmap() - Unmap MLO peers to partner SOCs
 * @soc: Soc handle
 * @peer_id: peer_id
 * Return: None
 */
void dp_mlo_partner_chips_unmap(struct dp_soc *soc,
				uint16_t peer_id);
#else
static inline void dp_mlo_partner_chips_map(struct dp_soc *soc,
					    struct dp_peer *peer,
					    uint16_t peer_id)
{
}

static inline void dp_mlo_partner_chips_unmap(struct dp_soc *soc,
					      uint16_t peer_id)
{
}
#endif

static inline
QDF_STATUS dp_peer_rx_tids_create(struct dp_peer *peer)
{
	uint8_t i;

	if (IS_MLO_DP_MLD_PEER(peer)) {
		dp_peer_info("skip for mld peer");
		return QDF_STATUS_SUCCESS;
	}

	if (peer->rx_tid) {
		QDF_BUG(0);
		dp_peer_err("peer rx_tid mem already exist");
		return QDF_STATUS_E_FAILURE;
	}

	peer->rx_tid = qdf_mem_malloc(DP_MAX_TIDS *
				      sizeof(struct dp_rx_tid));

	if (!peer->rx_tid) {
		dp_err("fail to alloc tid for peer" QDF_MAC_ADDR_FMT,
		       QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(peer->rx_tid, DP_MAX_TIDS * sizeof(struct dp_rx_tid));
	for (i = 0; i < DP_MAX_TIDS; i++)
		qdf_spinlock_create(&peer->rx_tid[i].tid_lock);

	return QDF_STATUS_SUCCESS;
}

static inline
void dp_peer_rx_tids_destroy(struct dp_peer *peer)
{
	uint8_t i;

	if (!IS_MLO_DP_LINK_PEER(peer)) {
		for (i = 0; i < DP_MAX_TIDS; i++)
			qdf_spinlock_destroy(&peer->rx_tid[i].tid_lock);

		qdf_mem_free(peer->rx_tid);
	}

	peer->rx_tid = NULL;
}
#endif /* _DP_PEER_H_ */
