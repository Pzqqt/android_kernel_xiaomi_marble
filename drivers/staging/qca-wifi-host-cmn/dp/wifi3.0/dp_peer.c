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

#include <qdf_types.h>
#include <qdf_lock.h>
#include <hal_hw_headers.h>
#include "dp_htt.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_peer.h"
#include "dp_rx_defrag.h"
#include "dp_rx.h"
#include <hal_api.h>
#include <hal_reo.h>
#include <cdp_txrx_handle.h>
#include <wlan_cfg.h>
#ifdef WIFI_MONITOR_SUPPORT
#include <dp_mon.h>
#endif
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#include <qdf_module.h>
#ifdef QCA_PEER_EXT_STATS
#include "dp_hist.h"
#endif

#ifdef REO_QDESC_HISTORY
#define REO_QDESC_HISTORY_SIZE 512
uint64_t reo_qdesc_history_idx;
struct reo_qdesc_event reo_qdesc_history[REO_QDESC_HISTORY_SIZE];
#endif

#ifdef FEATURE_WDS
static inline bool
dp_peer_ast_free_in_unmap_supported(struct dp_soc *soc,
				    struct dp_ast_entry *ast_entry)
{
	/* if peer map v2 is enabled we are not freeing ast entry
	 * here and it is supposed to be freed in unmap event (after
	 * we receive delete confirmation from target)
	 *
	 * if peer_id is invalid we did not get the peer map event
	 * for the peer free ast entry from here only in this case
	 */

	if ((ast_entry->type != CDP_TXRX_AST_TYPE_WDS_HM_SEC) &&
	    (ast_entry->type != CDP_TXRX_AST_TYPE_SELF))
		return true;

	return false;
}
#else
static inline bool
dp_peer_ast_free_in_unmap_supported(struct dp_soc *soc,
				    struct dp_ast_entry *ast_entry)
{
	return false;
}

void dp_soc_wds_attach(struct dp_soc *soc)
{
}

void dp_soc_wds_detach(struct dp_soc *soc)
{
}
#endif

#ifdef REO_QDESC_HISTORY
static inline void
dp_rx_reo_qdesc_history_add(struct reo_desc_list_node *free_desc,
			    enum reo_qdesc_event_type type)
{
	struct reo_qdesc_event *evt;
	struct dp_rx_tid *rx_tid = &free_desc->rx_tid;
	uint32_t idx;

	reo_qdesc_history_idx++;
	idx = (reo_qdesc_history_idx & (REO_QDESC_HISTORY_SIZE - 1));

	evt = &reo_qdesc_history[idx];

	qdf_mem_copy(evt->peer_mac, free_desc->peer_mac, QDF_MAC_ADDR_SIZE);
	evt->qdesc_addr = rx_tid->hw_qdesc_paddr;
	evt->ts = qdf_get_log_timestamp();
	evt->type = type;
}

#ifdef WLAN_DP_FEATURE_DEFERRED_REO_QDESC_DESTROY
static inline void
dp_rx_reo_qdesc_deferred_evt_add(struct reo_desc_deferred_freelist_node *desc,
				 enum reo_qdesc_event_type type)
{
	struct reo_qdesc_event *evt;
	uint32_t idx;

	reo_qdesc_history_idx++;
	idx = (reo_qdesc_history_idx & (REO_QDESC_HISTORY_SIZE - 1));

	evt = &reo_qdesc_history[idx];

	qdf_mem_copy(evt->peer_mac, desc->peer_mac, QDF_MAC_ADDR_SIZE);
	evt->qdesc_addr = desc->hw_qdesc_paddr;
	evt->ts = qdf_get_log_timestamp();
	evt->type = type;
}

#define DP_RX_REO_QDESC_DEFERRED_FREE_EVT(desc) \
	dp_rx_reo_qdesc_deferred_evt_add((desc), REO_QDESC_FREE)

#define DP_RX_REO_QDESC_DEFERRED_GET_MAC(desc, freedesc) \
	qdf_mem_copy(desc->peer_mac, freedesc->peer_mac, QDF_MAC_ADDR_SIZE)
#endif /* WLAN_DP_FEATURE_DEFERRED_REO_QDESC_DESTROY */

#define DP_RX_REO_QDESC_GET_MAC(freedesc, peer) \
	qdf_mem_copy(freedesc->peer_mac, peer->mac_addr.raw, QDF_MAC_ADDR_SIZE)

#define DP_RX_REO_QDESC_UPDATE_EVT(free_desc) \
	dp_rx_reo_qdesc_history_add((free_desc), REO_QDESC_UPDATE_CB)

#define DP_RX_REO_QDESC_FREE_EVT(free_desc) \
	dp_rx_reo_qdesc_history_add((free_desc), REO_QDESC_FREE)

#else
#define DP_RX_REO_QDESC_GET_MAC(freedesc, peer)

#define DP_RX_REO_QDESC_UPDATE_EVT(free_desc)

#define DP_RX_REO_QDESC_FREE_EVT(free_desc)

#define DP_RX_REO_QDESC_DEFERRED_FREE_EVT(desc)

#define DP_RX_REO_QDESC_DEFERRED_GET_MAC(desc, freedesc)
#endif

static inline void
dp_set_ssn_valid_flag(struct hal_reo_cmd_params *params,
					uint8_t valid)
{
	params->u.upd_queue_params.update_svld = 1;
	params->u.upd_queue_params.svld = valid;
	dp_peer_debug("Setting SSN valid bit to %d",
		      valid);
}

QDF_STATUS dp_peer_ast_table_attach(struct dp_soc *soc)
{
	uint32_t max_ast_index;

	max_ast_index = wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx);
	/* allocate ast_table for ast entry to ast_index map */
	dp_peer_info("\n%pK:<=== cfg max ast idx %d ====>", soc, max_ast_index);
	soc->ast_table = qdf_mem_malloc(max_ast_index *
					sizeof(struct dp_ast_entry *));
	if (!soc->ast_table) {
		dp_peer_err("%pK: ast_table memory allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS; /* success */
}

/*
 * dp_peer_find_map_attach() - allocate memory for peer_id_to_obj_map
 * @soc: soc handle
 *
 * return: QDF_STATUS
 */
static QDF_STATUS dp_peer_find_map_attach(struct dp_soc *soc)
{
	uint32_t max_peers, peer_map_size;

	max_peers = soc->max_peer_id;
	/* allocate the peer ID -> peer object map */
	dp_peer_info("\n%pK:<=== cfg max peer id %d ====>", soc, max_peers);
	peer_map_size = max_peers * sizeof(soc->peer_id_to_obj_map[0]);
	soc->peer_id_to_obj_map = qdf_mem_malloc(peer_map_size);
	if (!soc->peer_id_to_obj_map) {
		dp_peer_err("%pK: peer map memory allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}

	/*
	 * The peer_id_to_obj_map doesn't really need to be initialized,
	 * since elements are only used after they have been individually
	 * initialized.
	 * However, it is convenient for debugging to have all elements
	 * that are not in use set to 0.
	 */
	qdf_mem_zero(soc->peer_id_to_obj_map, peer_map_size);

	qdf_spinlock_create(&soc->peer_map_lock);
	return QDF_STATUS_SUCCESS; /* success */
}

#define DP_AST_HASH_LOAD_MULT  2
#define DP_AST_HASH_LOAD_SHIFT 0

static inline uint32_t
dp_peer_find_hash_index(struct dp_soc *soc,
			union dp_align_mac_addr *mac_addr)
{
	uint32_t index;

	index =
		mac_addr->align2.bytes_ab ^
		mac_addr->align2.bytes_cd ^
		mac_addr->align2.bytes_ef;

	index ^= index >> soc->peer_hash.idx_bits;
	index &= soc->peer_hash.mask;
	return index;
}

#ifdef WLAN_FEATURE_11BE_MLO
/*
 * dp_peer_find_hash_detach() - cleanup memory for peer_hash table
 * @soc: soc handle
 *
 * return: none
 */
static void dp_peer_find_hash_detach(struct dp_soc *soc)
{
	if (soc->peer_hash.bins) {
		qdf_mem_free(soc->peer_hash.bins);
		soc->peer_hash.bins = NULL;
		qdf_spinlock_destroy(&soc->peer_hash_lock);
	}

	if (soc->arch_ops.mlo_peer_find_hash_detach)
		soc->arch_ops.mlo_peer_find_hash_detach(soc);
}

/*
 * dp_peer_find_hash_attach() - allocate memory for peer_hash table
 * @soc: soc handle
 *
 * return: QDF_STATUS
 */
static QDF_STATUS dp_peer_find_hash_attach(struct dp_soc *soc)
{
	int i, hash_elems, log2;

	/* allocate the peer MAC address -> peer object hash table */
	hash_elems = soc->max_peers;
	hash_elems *= DP_PEER_HASH_LOAD_MULT;
	hash_elems >>= DP_PEER_HASH_LOAD_SHIFT;
	log2 = dp_log2_ceil(hash_elems);
	hash_elems = 1 << log2;

	soc->peer_hash.mask = hash_elems - 1;
	soc->peer_hash.idx_bits = log2;
	/* allocate an array of TAILQ peer object lists */
	soc->peer_hash.bins = qdf_mem_malloc(
		hash_elems * sizeof(TAILQ_HEAD(anonymous_tail_q, dp_peer)));
	if (!soc->peer_hash.bins)
		return QDF_STATUS_E_NOMEM;

	for (i = 0; i < hash_elems; i++)
		TAILQ_INIT(&soc->peer_hash.bins[i]);

	qdf_spinlock_create(&soc->peer_hash_lock);

	if (soc->arch_ops.mlo_peer_find_hash_attach &&
	    (soc->arch_ops.mlo_peer_find_hash_attach(soc) !=
			QDF_STATUS_SUCCESS)) {
		dp_peer_find_hash_detach(soc);
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_peer_find_hash_add() - add peer to peer_hash_table
 * @soc: soc handle
 * @peer: peer handle
 * @peer_type: link or mld peer
 *
 * return: none
 */
void dp_peer_find_hash_add(struct dp_soc *soc, struct dp_peer *peer)
{
	unsigned index;

	index = dp_peer_find_hash_index(soc, &peer->mac_addr);
	if (peer->peer_type == CDP_LINK_PEER_TYPE) {
		qdf_spin_lock_bh(&soc->peer_hash_lock);

		if (QDF_IS_STATUS_ERROR(dp_peer_get_ref(soc, peer,
							DP_MOD_ID_CONFIG))) {
			dp_err("fail to get peer ref:" QDF_MAC_ADDR_FMT,
			       QDF_MAC_ADDR_REF(peer->mac_addr.raw));
			qdf_spin_unlock_bh(&soc->peer_hash_lock);
			return;
		}

		/*
		 * It is important to add the new peer at the tail of
		 * peer list with the bin index. Together with having
		 * the hash_find function search from head to tail,
		 * this ensures that if two entries with the same MAC address
		 * are stored, the one added first will be found first.
		 */
		TAILQ_INSERT_TAIL(&soc->peer_hash.bins[index], peer,
				  hash_list_elem);

		qdf_spin_unlock_bh(&soc->peer_hash_lock);
	} else if (peer->peer_type == CDP_MLD_PEER_TYPE) {
		if (soc->arch_ops.mlo_peer_find_hash_add)
			soc->arch_ops.mlo_peer_find_hash_add(soc, peer);
	} else {
		dp_err("unknown peer type %d", peer->peer_type);
	}
}

/*
 * dp_peer_find_hash_find() - returns peer from peer_hash_table matching
 *                            vdev_id and mac_address
 * @soc: soc handle
 * @peer_mac_addr: peer mac address
 * @mac_addr_is_aligned: is mac addr alligned
 * @vdev_id: vdev_id
 * @mod_id: id of module requesting reference
 *
 * return: peer in sucsess
 *         NULL in failure
 */
struct dp_peer *dp_peer_find_hash_find(struct dp_soc *soc,
				       uint8_t *peer_mac_addr,
				       int mac_addr_is_aligned,
				       uint8_t vdev_id,
				       enum dp_mod_id mod_id)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	unsigned index;
	struct dp_peer *peer;

	if (!soc->peer_hash.bins)
		return NULL;

	if (mac_addr_is_aligned) {
		mac_addr = (union dp_align_mac_addr *)peer_mac_addr;
	} else {
		qdf_mem_copy(
			&local_mac_addr_aligned.raw[0],
			peer_mac_addr, QDF_MAC_ADDR_SIZE);
		mac_addr = &local_mac_addr_aligned;
	}
	/* search link peer table firstly */
	index = dp_peer_find_hash_index(soc, mac_addr);
	qdf_spin_lock_bh(&soc->peer_hash_lock);
	TAILQ_FOREACH(peer, &soc->peer_hash.bins[index], hash_list_elem) {
		if (dp_peer_find_mac_addr_cmp(mac_addr, &peer->mac_addr) == 0 &&
		    ((peer->vdev->vdev_id == vdev_id) ||
		     (vdev_id == DP_VDEV_ALL))) {
			/* take peer reference before returning */
			if (dp_peer_get_ref(soc, peer, mod_id) !=
						QDF_STATUS_SUCCESS)
				peer = NULL;

			qdf_spin_unlock_bh(&soc->peer_hash_lock);
			return peer;
		}
	}
	qdf_spin_unlock_bh(&soc->peer_hash_lock);

	if (soc->arch_ops.mlo_peer_find_hash_find)
		return soc->arch_ops.mlo_peer_find_hash_find(soc, peer_mac_addr,
							     mac_addr_is_aligned,
							     mod_id);
	return NULL;
}

qdf_export_symbol(dp_peer_find_hash_find);

/*
 * dp_peer_find_hash_remove() - remove peer from peer_hash_table
 * @soc: soc handle
 * @peer: peer handle
 *
 * return: none
 */
void dp_peer_find_hash_remove(struct dp_soc *soc, struct dp_peer *peer)
{
	unsigned index;
	struct dp_peer *tmppeer = NULL;
	int found = 0;

	index = dp_peer_find_hash_index(soc, &peer->mac_addr);

	if (peer->peer_type == CDP_LINK_PEER_TYPE) {
		/* Check if tail is not empty before delete*/
		QDF_ASSERT(!TAILQ_EMPTY(&soc->peer_hash.bins[index]));

		qdf_spin_lock_bh(&soc->peer_hash_lock);
		TAILQ_FOREACH(tmppeer, &soc->peer_hash.bins[index],
			      hash_list_elem) {
			if (tmppeer == peer) {
				found = 1;
				break;
			}
		}
		QDF_ASSERT(found);
		TAILQ_REMOVE(&soc->peer_hash.bins[index], peer,
			     hash_list_elem);

		dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
		qdf_spin_unlock_bh(&soc->peer_hash_lock);
	} else if (peer->peer_type == CDP_MLD_PEER_TYPE) {
		if (soc->arch_ops.mlo_peer_find_hash_remove)
			soc->arch_ops.mlo_peer_find_hash_remove(soc, peer);
	} else {
		dp_err("unknown peer type %d", peer->peer_type);
	}
}

/*
 * dp_peer_exist_on_pdev - check if peer with mac address exist on pdev
 *
 * @soc: Datapath SOC handle
 * @peer_mac_addr: peer mac address
 * @mac_addr_is_aligned: is mac address aligned
 * @pdev: Datapath PDEV handle
 *
 * Return: true if peer found else return false
 */
static bool dp_peer_exist_on_pdev(struct dp_soc *soc,
				  uint8_t *peer_mac_addr,
				  int mac_addr_is_aligned,
				  struct dp_pdev *pdev)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	unsigned int index;
	struct dp_peer *peer;
	bool found = false;

	if (mac_addr_is_aligned) {
		mac_addr = (union dp_align_mac_addr *)peer_mac_addr;
	} else {
		qdf_mem_copy(
			&local_mac_addr_aligned.raw[0],
			peer_mac_addr, QDF_MAC_ADDR_SIZE);
		mac_addr = &local_mac_addr_aligned;
	}
	index = dp_peer_find_hash_index(soc, mac_addr);
	qdf_spin_lock_bh(&soc->peer_hash_lock);
	TAILQ_FOREACH(peer, &soc->peer_hash.bins[index], hash_list_elem) {
		if (dp_peer_find_mac_addr_cmp(mac_addr, &peer->mac_addr) == 0 &&
		    (peer->vdev->pdev == pdev)) {
			found = true;
			break;
		}
	}
	qdf_spin_unlock_bh(&soc->peer_hash_lock);

	return found;
}
#else
static QDF_STATUS dp_peer_find_hash_attach(struct dp_soc *soc)
{
	int i, hash_elems, log2;

	/* allocate the peer MAC address -> peer object hash table */
	hash_elems = soc->max_peers;
	hash_elems *= DP_PEER_HASH_LOAD_MULT;
	hash_elems >>= DP_PEER_HASH_LOAD_SHIFT;
	log2 = dp_log2_ceil(hash_elems);
	hash_elems = 1 << log2;

	soc->peer_hash.mask = hash_elems - 1;
	soc->peer_hash.idx_bits = log2;
	/* allocate an array of TAILQ peer object lists */
	soc->peer_hash.bins = qdf_mem_malloc(
		hash_elems * sizeof(TAILQ_HEAD(anonymous_tail_q, dp_peer)));
	if (!soc->peer_hash.bins)
		return QDF_STATUS_E_NOMEM;

	for (i = 0; i < hash_elems; i++)
		TAILQ_INIT(&soc->peer_hash.bins[i]);

	qdf_spinlock_create(&soc->peer_hash_lock);
	return QDF_STATUS_SUCCESS;
}

static void dp_peer_find_hash_detach(struct dp_soc *soc)
{
	if (soc->peer_hash.bins) {
		qdf_mem_free(soc->peer_hash.bins);
		soc->peer_hash.bins = NULL;
		qdf_spinlock_destroy(&soc->peer_hash_lock);
	}
}

void dp_peer_find_hash_add(struct dp_soc *soc, struct dp_peer *peer)
{
	unsigned index;

	index = dp_peer_find_hash_index(soc, &peer->mac_addr);
	qdf_spin_lock_bh(&soc->peer_hash_lock);

	if (QDF_IS_STATUS_ERROR(dp_peer_get_ref(soc, peer, DP_MOD_ID_CONFIG))) {
		dp_err("unable to get peer ref at MAP mac: "QDF_MAC_ADDR_FMT,
		       QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		qdf_spin_unlock_bh(&soc->peer_hash_lock);
		return;
	}

	/*
	 * It is important to add the new peer at the tail of the peer list
	 * with the bin index.  Together with having the hash_find function
	 * search from head to tail, this ensures that if two entries with
	 * the same MAC address are stored, the one added first will be
	 * found first.
	 */
	TAILQ_INSERT_TAIL(&soc->peer_hash.bins[index], peer, hash_list_elem);

	qdf_spin_unlock_bh(&soc->peer_hash_lock);
}

struct dp_peer *dp_peer_find_hash_find(
				struct dp_soc *soc, uint8_t *peer_mac_addr,
				int mac_addr_is_aligned, uint8_t vdev_id,
				enum dp_mod_id mod_id)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	unsigned index;
	struct dp_peer *peer;

	if (!soc->peer_hash.bins)
		return NULL;

	if (mac_addr_is_aligned) {
		mac_addr = (union dp_align_mac_addr *)peer_mac_addr;
	} else {
		qdf_mem_copy(
			&local_mac_addr_aligned.raw[0],
			peer_mac_addr, QDF_MAC_ADDR_SIZE);
		mac_addr = &local_mac_addr_aligned;
	}
	index = dp_peer_find_hash_index(soc, mac_addr);
	qdf_spin_lock_bh(&soc->peer_hash_lock);
	TAILQ_FOREACH(peer, &soc->peer_hash.bins[index], hash_list_elem) {
		if (dp_peer_find_mac_addr_cmp(mac_addr, &peer->mac_addr) == 0 &&
		    ((peer->vdev->vdev_id == vdev_id) ||
		     (vdev_id == DP_VDEV_ALL))) {
			/* take peer reference before returning */
			if (dp_peer_get_ref(soc, peer, mod_id) !=
						QDF_STATUS_SUCCESS)
				peer = NULL;

			qdf_spin_unlock_bh(&soc->peer_hash_lock);
			return peer;
		}
	}
	qdf_spin_unlock_bh(&soc->peer_hash_lock);
	return NULL; /* failure */
}

qdf_export_symbol(dp_peer_find_hash_find);

void dp_peer_find_hash_remove(struct dp_soc *soc, struct dp_peer *peer)
{
	unsigned index;
	struct dp_peer *tmppeer = NULL;
	int found = 0;

	index = dp_peer_find_hash_index(soc, &peer->mac_addr);
	/* Check if tail is not empty before delete*/
	QDF_ASSERT(!TAILQ_EMPTY(&soc->peer_hash.bins[index]));

	qdf_spin_lock_bh(&soc->peer_hash_lock);
	TAILQ_FOREACH(tmppeer, &soc->peer_hash.bins[index], hash_list_elem) {
		if (tmppeer == peer) {
			found = 1;
			break;
		}
	}
	QDF_ASSERT(found);
	TAILQ_REMOVE(&soc->peer_hash.bins[index], peer, hash_list_elem);

	dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
	qdf_spin_unlock_bh(&soc->peer_hash_lock);
}

static bool dp_peer_exist_on_pdev(struct dp_soc *soc,
				  uint8_t *peer_mac_addr,
				  int mac_addr_is_aligned,
				  struct dp_pdev *pdev)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	unsigned int index;
	struct dp_peer *peer;
	bool found = false;

	if (mac_addr_is_aligned) {
		mac_addr = (union dp_align_mac_addr *)peer_mac_addr;
	} else {
		qdf_mem_copy(
			&local_mac_addr_aligned.raw[0],
			peer_mac_addr, QDF_MAC_ADDR_SIZE);
		mac_addr = &local_mac_addr_aligned;
	}
	index = dp_peer_find_hash_index(soc, mac_addr);
	qdf_spin_lock_bh(&soc->peer_hash_lock);
	TAILQ_FOREACH(peer, &soc->peer_hash.bins[index], hash_list_elem) {
		if (dp_peer_find_mac_addr_cmp(mac_addr, &peer->mac_addr) == 0 &&
		    (peer->vdev->pdev == pdev)) {
			found = true;
			break;
		}
	}
	qdf_spin_unlock_bh(&soc->peer_hash_lock);
	return found;
}
#endif/* WLAN_FEATURE_11BE_MLO */

/*
 * dp_peer_vdev_list_add() - add peer into vdev's peer list
 * @soc: soc handle
 * @vdev: vdev handle
 * @peer: peer handle
 *
 * return: none
 */
void dp_peer_vdev_list_add(struct dp_soc *soc, struct dp_vdev *vdev,
			   struct dp_peer *peer)
{
	/* only link peer will be added to vdev peer list */
	if (IS_MLO_DP_MLD_PEER(peer))
		return;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	if (QDF_IS_STATUS_ERROR(dp_peer_get_ref(soc, peer, DP_MOD_ID_CONFIG))) {
		dp_err("unable to get peer ref at MAP mac: "QDF_MAC_ADDR_FMT,
		       QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		return;
	}

	/* add this peer into the vdev's list */
	if (wlan_op_mode_sta == vdev->opmode)
		TAILQ_INSERT_HEAD(&vdev->peer_list, peer, peer_list_elem);
	else
		TAILQ_INSERT_TAIL(&vdev->peer_list, peer, peer_list_elem);

	vdev->num_peers++;
	qdf_spin_unlock_bh(&vdev->peer_list_lock);
}

/*
 * dp_peer_vdev_list_remove() - remove peer from vdev's peer list
 * @soc: SoC handle
 * @vdev: VDEV handle
 * @peer: peer handle
 *
 * Return: none
 */
void dp_peer_vdev_list_remove(struct dp_soc *soc, struct dp_vdev *vdev,
			      struct dp_peer *peer)
{
	uint8_t found = 0;
	struct dp_peer *tmppeer = NULL;

	/* only link peer will be added to vdev peer list */
	if (IS_MLO_DP_MLD_PEER(peer))
		return;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	TAILQ_FOREACH(tmppeer, &peer->vdev->peer_list, peer_list_elem) {
		if (tmppeer == peer) {
			found = 1;
			break;
		}
	}

	if (found) {
		TAILQ_REMOVE(&peer->vdev->peer_list, peer,
			     peer_list_elem);
		dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
		vdev->num_peers--;
	} else {
		/*Ignoring the remove operation as peer not found*/
		dp_peer_debug("%pK: peer:%pK not found in vdev:%pK peerlist:%pK"
			      , soc, peer, vdev, &peer->vdev->peer_list);
	}
	qdf_spin_unlock_bh(&vdev->peer_list_lock);
}

/*
 * dp_peer_find_id_to_obj_add() - Add peer into peer_id table
 * @soc: SoC handle
 * @peer: peer handle
 * @peer_id: peer_id
 *
 * Return: None
 */
void dp_peer_find_id_to_obj_add(struct dp_soc *soc,
				struct dp_peer *peer,
				uint16_t peer_id)
{
	QDF_ASSERT(peer_id <= soc->max_peer_id);

	qdf_spin_lock_bh(&soc->peer_map_lock);

	if (QDF_IS_STATUS_ERROR(dp_peer_get_ref(soc, peer, DP_MOD_ID_CONFIG))) {
		dp_err("unable to get peer ref at MAP mac: "QDF_MAC_ADDR_FMT" peer_id %u",
		       QDF_MAC_ADDR_REF(peer->mac_addr.raw), peer_id);
		qdf_spin_unlock_bh(&soc->peer_map_lock);
		return;
	}

	if (!soc->peer_id_to_obj_map[peer_id]) {
		soc->peer_id_to_obj_map[peer_id] = peer;
	} else {
		/* Peer map event came for peer_id which
		 * is already mapped, this is not expected
		 */
		QDF_ASSERT(0);
	}
	qdf_spin_unlock_bh(&soc->peer_map_lock);
}

/*
 * dp_peer_find_id_to_obj_remove() - remove peer from peer_id table
 * @soc: SoC handle
 * @peer_id: peer_id
 *
 * Return: None
 */
void dp_peer_find_id_to_obj_remove(struct dp_soc *soc,
				   uint16_t peer_id)
{
	struct dp_peer *peer = NULL;
	QDF_ASSERT(peer_id <= soc->max_peer_id);

	qdf_spin_lock_bh(&soc->peer_map_lock);
	peer = soc->peer_id_to_obj_map[peer_id];
	soc->peer_id_to_obj_map[peer_id] = NULL;
	dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
	qdf_spin_unlock_bh(&soc->peer_map_lock);
}

#ifdef FEATURE_MEC
/**
 * dp_peer_mec_hash_attach() - Allocate and initialize MEC Hash Table
 * @soc: SoC handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_peer_mec_hash_attach(struct dp_soc *soc)
{
	int log2, hash_elems, i;

	log2 = dp_log2_ceil(DP_PEER_MAX_MEC_IDX);
	hash_elems = 1 << log2;

	soc->mec_hash.mask = hash_elems - 1;
	soc->mec_hash.idx_bits = log2;

	dp_peer_info("%pK: max mec index: %d",
		     soc, DP_PEER_MAX_MEC_IDX);

	/* allocate an array of TAILQ mec object lists */
	soc->mec_hash.bins = qdf_mem_malloc(hash_elems *
					    sizeof(TAILQ_HEAD(anonymous_tail_q,
							      dp_mec_entry)));

	if (!soc->mec_hash.bins)
		return QDF_STATUS_E_NOMEM;

	for (i = 0; i < hash_elems; i++)
		TAILQ_INIT(&soc->mec_hash.bins[i]);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_peer_mec_hash_index() - Compute the MEC hash from MAC address
 * @soc: SoC handle
 *
 * Return: MEC hash
 */
static inline uint32_t dp_peer_mec_hash_index(struct dp_soc *soc,
					      union dp_align_mac_addr *mac_addr)
{
	uint32_t index;

	index =
		mac_addr->align2.bytes_ab ^
		mac_addr->align2.bytes_cd ^
		mac_addr->align2.bytes_ef;
	index ^= index >> soc->mec_hash.idx_bits;
	index &= soc->mec_hash.mask;
	return index;
}

struct dp_mec_entry *dp_peer_mec_hash_find_by_pdevid(struct dp_soc *soc,
						     uint8_t pdev_id,
						     uint8_t *mec_mac_addr)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	uint32_t index;
	struct dp_mec_entry *mecentry;

	qdf_mem_copy(&local_mac_addr_aligned.raw[0],
		     mec_mac_addr, QDF_MAC_ADDR_SIZE);
	mac_addr = &local_mac_addr_aligned;

	index = dp_peer_mec_hash_index(soc, mac_addr);
	TAILQ_FOREACH(mecentry, &soc->mec_hash.bins[index], hash_list_elem) {
		if ((pdev_id == mecentry->pdev_id) &&
		    !dp_peer_find_mac_addr_cmp(mac_addr, &mecentry->mac_addr))
			return mecentry;
	}

	return NULL;
}

/**
 * dp_peer_mec_hash_add() - Add MEC entry into hash table
 * @soc: SoC handle
 *
 * This function adds the MEC entry into SoC MEC hash table
 *
 * Return: None
 */
static inline void dp_peer_mec_hash_add(struct dp_soc *soc,
					struct dp_mec_entry *mecentry)
{
	uint32_t index;

	index = dp_peer_mec_hash_index(soc, &mecentry->mac_addr);
	qdf_spin_lock_bh(&soc->mec_lock);
	TAILQ_INSERT_TAIL(&soc->mec_hash.bins[index], mecentry, hash_list_elem);
	qdf_spin_unlock_bh(&soc->mec_lock);
}

QDF_STATUS dp_peer_mec_add_entry(struct dp_soc *soc,
				 struct dp_vdev *vdev,
				 uint8_t *mac_addr)
{
	struct dp_mec_entry *mecentry = NULL;
	struct dp_pdev *pdev = NULL;

	if (!vdev) {
		dp_peer_err("%pK: Peers vdev is NULL", soc);
		return QDF_STATUS_E_INVAL;
	}

	pdev = vdev->pdev;

	if (qdf_unlikely(qdf_atomic_read(&soc->mec_cnt) >=
					 DP_PEER_MAX_MEC_ENTRY)) {
		dp_peer_warn("%pK: max MEC entry limit reached mac_addr: "
			     QDF_MAC_ADDR_FMT, soc, QDF_MAC_ADDR_REF(mac_addr));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spin_lock_bh(&soc->mec_lock);
	mecentry = dp_peer_mec_hash_find_by_pdevid(soc, pdev->pdev_id,
						   mac_addr);
	if (qdf_likely(mecentry)) {
		mecentry->is_active = TRUE;
		qdf_spin_unlock_bh(&soc->mec_lock);
		return QDF_STATUS_E_ALREADY;
	}

	qdf_spin_unlock_bh(&soc->mec_lock);

	dp_peer_debug("%pK: pdevid: %u vdev: %u type: MEC mac_addr: "
		      QDF_MAC_ADDR_FMT,
		      soc, pdev->pdev_id, vdev->vdev_id,
		      QDF_MAC_ADDR_REF(mac_addr));

	mecentry = (struct dp_mec_entry *)
			qdf_mem_malloc(sizeof(struct dp_mec_entry));

	if (qdf_unlikely(!mecentry)) {
		dp_peer_err("%pK: fail to allocate mecentry", soc);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr((struct qdf_mac_addr *)&mecentry->mac_addr.raw[0],
			 (struct qdf_mac_addr *)mac_addr);
	mecentry->pdev_id = pdev->pdev_id;
	mecentry->vdev_id = vdev->vdev_id;
	mecentry->is_active = TRUE;
	dp_peer_mec_hash_add(soc, mecentry);

	qdf_atomic_inc(&soc->mec_cnt);
	DP_STATS_INC(soc, mec.added, 1);

	return QDF_STATUS_SUCCESS;
}

void dp_peer_mec_detach_entry(struct dp_soc *soc, struct dp_mec_entry *mecentry,
			      void *ptr)
{
	uint32_t index = dp_peer_mec_hash_index(soc, &mecentry->mac_addr);

	TAILQ_HEAD(, dp_mec_entry) * free_list = ptr;

	TAILQ_REMOVE(&soc->mec_hash.bins[index], mecentry,
		     hash_list_elem);
	TAILQ_INSERT_TAIL(free_list, mecentry, hash_list_elem);
}

void dp_peer_mec_free_list(struct dp_soc *soc, void *ptr)
{
	struct dp_mec_entry *mecentry, *mecentry_next;

	TAILQ_HEAD(, dp_mec_entry) * free_list = ptr;

	TAILQ_FOREACH_SAFE(mecentry, free_list, hash_list_elem,
			   mecentry_next) {
		dp_peer_debug("%pK: MEC delete for mac_addr " QDF_MAC_ADDR_FMT,
			      soc, QDF_MAC_ADDR_REF(&mecentry->mac_addr));
		qdf_mem_free(mecentry);
		qdf_atomic_dec(&soc->mec_cnt);
		DP_STATS_INC(soc, mec.deleted, 1);
	}
}

/**
 * dp_peer_mec_hash_detach() - Free MEC Hash table
 * @soc: SoC handle
 *
 * Return: None
 */
void dp_peer_mec_hash_detach(struct dp_soc *soc)
{
	dp_peer_mec_flush_entries(soc);
	qdf_mem_free(soc->mec_hash.bins);
	soc->mec_hash.bins = NULL;
}

void dp_peer_mec_spinlock_destroy(struct dp_soc *soc)
{
	qdf_spinlock_destroy(&soc->mec_lock);
}

void dp_peer_mec_spinlock_create(struct dp_soc *soc)
{
	qdf_spinlock_create(&soc->mec_lock);
}
#else
QDF_STATUS dp_peer_mec_hash_attach(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

void dp_peer_mec_hash_detach(struct dp_soc *soc)
{
}
#endif

#ifdef FEATURE_AST
/*
 * dp_peer_ast_hash_attach() - Allocate and initialize AST Hash Table
 * @soc: SoC handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_peer_ast_hash_attach(struct dp_soc *soc)
{
	int i, hash_elems, log2;
	unsigned int max_ast_idx = wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx);

	hash_elems = ((max_ast_idx * DP_AST_HASH_LOAD_MULT) >>
		DP_AST_HASH_LOAD_SHIFT);

	log2 = dp_log2_ceil(hash_elems);
	hash_elems = 1 << log2;

	soc->ast_hash.mask = hash_elems - 1;
	soc->ast_hash.idx_bits = log2;

	dp_peer_info("%pK: ast hash_elems: %d, max_ast_idx: %d",
		     soc, hash_elems, max_ast_idx);

	/* allocate an array of TAILQ peer object lists */
	soc->ast_hash.bins = qdf_mem_malloc(
		hash_elems * sizeof(TAILQ_HEAD(anonymous_tail_q,
				dp_ast_entry)));

	if (!soc->ast_hash.bins)
		return QDF_STATUS_E_NOMEM;

	for (i = 0; i < hash_elems; i++)
		TAILQ_INIT(&soc->ast_hash.bins[i]);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_peer_ast_cleanup() - cleanup the references
 * @soc: SoC handle
 * @ast: ast entry
 *
 * Return: None
 */
static inline void dp_peer_ast_cleanup(struct dp_soc *soc,
				       struct dp_ast_entry *ast)
{
	txrx_ast_free_cb cb = ast->callback;
	void *cookie = ast->cookie;

	dp_peer_debug("mac_addr: " QDF_MAC_ADDR_FMT ", cb: %pK, cookie: %pK",
		      QDF_MAC_ADDR_REF(ast->mac_addr.raw), cb, cookie);

	/* Call the callbacks to free up the cookie */
	if (cb) {
		ast->callback = NULL;
		ast->cookie = NULL;
		cb(soc->ctrl_psoc,
		   dp_soc_to_cdp_soc(soc),
		   cookie,
		   CDP_TXRX_AST_DELETE_IN_PROGRESS);
	}
}

/*
 * dp_peer_ast_hash_detach() - Free AST Hash table
 * @soc: SoC handle
 *
 * Return: None
 */
void dp_peer_ast_hash_detach(struct dp_soc *soc)
{
	unsigned int index;
	struct dp_ast_entry *ast, *ast_next;

	if (!soc->ast_hash.mask)
		return;

	if (!soc->ast_hash.bins)
		return;

	dp_peer_debug("%pK: num_ast_entries: %u", soc, soc->num_ast_entries);

	qdf_spin_lock_bh(&soc->ast_lock);
	for (index = 0; index <= soc->ast_hash.mask; index++) {
		if (!TAILQ_EMPTY(&soc->ast_hash.bins[index])) {
			TAILQ_FOREACH_SAFE(ast, &soc->ast_hash.bins[index],
					   hash_list_elem, ast_next) {
				TAILQ_REMOVE(&soc->ast_hash.bins[index], ast,
					     hash_list_elem);
				dp_peer_ast_cleanup(soc, ast);
				soc->num_ast_entries--;
				qdf_mem_free(ast);
			}
		}
	}
	qdf_spin_unlock_bh(&soc->ast_lock);

	qdf_mem_free(soc->ast_hash.bins);
	soc->ast_hash.bins = NULL;
}

/*
 * dp_peer_ast_hash_index() - Compute the AST hash from MAC address
 * @soc: SoC handle
 *
 * Return: AST hash
 */
static inline uint32_t dp_peer_ast_hash_index(struct dp_soc *soc,
	union dp_align_mac_addr *mac_addr)
{
	uint32_t index;

	index =
		mac_addr->align2.bytes_ab ^
		mac_addr->align2.bytes_cd ^
		mac_addr->align2.bytes_ef;
	index ^= index >> soc->ast_hash.idx_bits;
	index &= soc->ast_hash.mask;
	return index;
}

/*
 * dp_peer_ast_hash_add() - Add AST entry into hash table
 * @soc: SoC handle
 *
 * This function adds the AST entry into SoC AST hash table
 * It assumes caller has taken the ast lock to protect the access to this table
 *
 * Return: None
 */
static inline void dp_peer_ast_hash_add(struct dp_soc *soc,
		struct dp_ast_entry *ase)
{
	uint32_t index;

	index = dp_peer_ast_hash_index(soc, &ase->mac_addr);
	TAILQ_INSERT_TAIL(&soc->ast_hash.bins[index], ase, hash_list_elem);
}

/*
 * dp_peer_ast_hash_remove() - Look up and remove AST entry from hash table
 * @soc: SoC handle
 *
 * This function removes the AST entry from soc AST hash table
 * It assumes caller has taken the ast lock to protect the access to this table
 *
 * Return: None
 */
void dp_peer_ast_hash_remove(struct dp_soc *soc,
			     struct dp_ast_entry *ase)
{
	unsigned index;
	struct dp_ast_entry *tmpase;
	int found = 0;

	if (soc->ast_offload_support)
		return;

	index = dp_peer_ast_hash_index(soc, &ase->mac_addr);
	/* Check if tail is not empty before delete*/
	QDF_ASSERT(!TAILQ_EMPTY(&soc->ast_hash.bins[index]));

	dp_peer_debug("ID: %u idx: %u mac_addr: " QDF_MAC_ADDR_FMT,
		      ase->peer_id, index, QDF_MAC_ADDR_REF(ase->mac_addr.raw));

	TAILQ_FOREACH(tmpase, &soc->ast_hash.bins[index], hash_list_elem) {
		if (tmpase == ase) {
			found = 1;
			break;
		}
	}

	QDF_ASSERT(found);

	if (found)
		TAILQ_REMOVE(&soc->ast_hash.bins[index], ase, hash_list_elem);
}

/*
 * dp_peer_ast_hash_find_by_vdevid() - Find AST entry by MAC address
 * @soc: SoC handle
 *
 * It assumes caller has taken the ast lock to protect the access to
 * AST hash table
 *
 * Return: AST entry
 */
struct dp_ast_entry *dp_peer_ast_hash_find_by_vdevid(struct dp_soc *soc,
						     uint8_t *ast_mac_addr,
						     uint8_t vdev_id)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	uint32_t index;
	struct dp_ast_entry *ase;

	qdf_mem_copy(&local_mac_addr_aligned.raw[0],
		     ast_mac_addr, QDF_MAC_ADDR_SIZE);
	mac_addr = &local_mac_addr_aligned;

	index = dp_peer_ast_hash_index(soc, mac_addr);
	TAILQ_FOREACH(ase, &soc->ast_hash.bins[index], hash_list_elem) {
		if ((vdev_id == ase->vdev_id) &&
		    !dp_peer_find_mac_addr_cmp(mac_addr, &ase->mac_addr)) {
			return ase;
		}
	}

	return NULL;
}

/*
 * dp_peer_ast_hash_find_by_pdevid() - Find AST entry by MAC address
 * @soc: SoC handle
 *
 * It assumes caller has taken the ast lock to protect the access to
 * AST hash table
 *
 * Return: AST entry
 */
struct dp_ast_entry *dp_peer_ast_hash_find_by_pdevid(struct dp_soc *soc,
						     uint8_t *ast_mac_addr,
						     uint8_t pdev_id)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	uint32_t index;
	struct dp_ast_entry *ase;

	qdf_mem_copy(&local_mac_addr_aligned.raw[0],
		     ast_mac_addr, QDF_MAC_ADDR_SIZE);
	mac_addr = &local_mac_addr_aligned;

	index = dp_peer_ast_hash_index(soc, mac_addr);
	TAILQ_FOREACH(ase, &soc->ast_hash.bins[index], hash_list_elem) {
		if ((pdev_id == ase->pdev_id) &&
		    !dp_peer_find_mac_addr_cmp(mac_addr, &ase->mac_addr)) {
			return ase;
		}
	}

	return NULL;
}

/*
 * dp_peer_ast_hash_find_soc() - Find AST entry by MAC address
 * @soc: SoC handle
 *
 * It assumes caller has taken the ast lock to protect the access to
 * AST hash table
 *
 * Return: AST entry
 */
struct dp_ast_entry *dp_peer_ast_hash_find_soc(struct dp_soc *soc,
					       uint8_t *ast_mac_addr)
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	unsigned index;
	struct dp_ast_entry *ase;

	qdf_mem_copy(&local_mac_addr_aligned.raw[0],
			ast_mac_addr, QDF_MAC_ADDR_SIZE);
	mac_addr = &local_mac_addr_aligned;

	index = dp_peer_ast_hash_index(soc, mac_addr);
	TAILQ_FOREACH(ase, &soc->ast_hash.bins[index], hash_list_elem) {
		if (dp_peer_find_mac_addr_cmp(mac_addr, &ase->mac_addr) == 0) {
			return ase;
		}
	}

	return NULL;
}

/*
 * dp_peer_map_ast() - Map the ast entry with HW AST Index
 * @soc: SoC handle
 * @peer: peer to which ast node belongs
 * @mac_addr: MAC address of ast node
 * @hw_peer_id: HW AST Index returned by target in peer map event
 * @vdev_id: vdev id for VAP to which the peer belongs to
 * @ast_hash: ast hash value in HW
 * @is_wds: flag to indicate peer map event for WDS ast entry
 *
 * Return: QDF_STATUS code
 */
static inline QDF_STATUS dp_peer_map_ast(struct dp_soc *soc,
					 struct dp_peer *peer,
					 uint8_t *mac_addr,
					 uint16_t hw_peer_id,
					 uint8_t vdev_id,
					 uint16_t ast_hash,
					 uint8_t is_wds)
{
	struct dp_ast_entry *ast_entry = NULL;
	enum cdp_txrx_ast_entry_type peer_type = CDP_TXRX_AST_TYPE_STATIC;
	void *cookie = NULL;
	txrx_ast_free_cb cb = NULL;
	QDF_STATUS err = QDF_STATUS_SUCCESS;

	if (soc->ast_offload_support)
		return QDF_STATUS_SUCCESS;

	if (!peer) {
		return QDF_STATUS_E_INVAL;
	}

	dp_peer_err("%pK: peer %pK ID %d vid %d mac " QDF_MAC_ADDR_FMT,
		    soc, peer, hw_peer_id, vdev_id,
		    QDF_MAC_ADDR_REF(mac_addr));

	qdf_spin_lock_bh(&soc->ast_lock);

	ast_entry = dp_peer_ast_hash_find_by_vdevid(soc, mac_addr, vdev_id);

	if (is_wds) {
		/*
		 * In certain cases like Auth attack on a repeater
		 * can result in the number of ast_entries falling
		 * in the same hash bucket to exceed the max_skid
		 * length supported by HW in root AP. In these cases
		 * the FW will return the hw_peer_id (ast_index) as
		 * 0xffff indicating HW could not add the entry in
		 * its table. Host has to delete the entry from its
		 * table in these cases.
		 */
		if (hw_peer_id == HTT_INVALID_PEER) {
			DP_STATS_INC(soc, ast.map_err, 1);
			if (ast_entry) {
				if (ast_entry->is_mapped) {
					soc->ast_table[ast_entry->ast_idx] =
						NULL;
				}

				cb = ast_entry->callback;
				cookie = ast_entry->cookie;
				peer_type = ast_entry->type;

				dp_peer_unlink_ast_entry(soc, ast_entry, peer);
				dp_peer_free_ast_entry(soc, ast_entry);

				qdf_spin_unlock_bh(&soc->ast_lock);

				if (cb) {
					cb(soc->ctrl_psoc,
					   dp_soc_to_cdp_soc(soc),
					   cookie,
					   CDP_TXRX_AST_DELETED);
				}
			} else {
				qdf_spin_unlock_bh(&soc->ast_lock);
				dp_peer_alert("AST entry not found with peer %pK peer_id %u peer_mac " QDF_MAC_ADDR_FMT " mac_addr " QDF_MAC_ADDR_FMT " vdev_id %u next_hop %u",
					      peer, peer->peer_id,
					      QDF_MAC_ADDR_REF(peer->mac_addr.raw),
					      QDF_MAC_ADDR_REF(mac_addr),
					      vdev_id, is_wds);
			}
			err = QDF_STATUS_E_INVAL;

			dp_hmwds_ast_add_notify(peer, mac_addr,
						peer_type, err, true);

			return err;
		}
	}

	if (ast_entry) {
		ast_entry->ast_idx = hw_peer_id;
		soc->ast_table[hw_peer_id] = ast_entry;
		ast_entry->is_active = TRUE;
		peer_type = ast_entry->type;
		ast_entry->ast_hash_value = ast_hash;
		ast_entry->is_mapped = TRUE;
		qdf_assert_always(ast_entry->peer_id == HTT_INVALID_PEER);

		ast_entry->peer_id = peer->peer_id;
		TAILQ_INSERT_TAIL(&peer->ast_entry_list, ast_entry,
				  ase_list_elem);
	}

	if (ast_entry || (peer->vdev && peer->vdev->proxysta_vdev)) {
		if (soc->cdp_soc.ol_ops->peer_map_event) {
			soc->cdp_soc.ol_ops->peer_map_event(
			soc->ctrl_psoc, peer->peer_id,
			hw_peer_id, vdev_id,
			mac_addr, peer_type, ast_hash);
		}
	} else {
		dp_peer_err("%pK: AST entry not found", soc);
		err = QDF_STATUS_E_NOENT;
	}

	qdf_spin_unlock_bh(&soc->ast_lock);

	dp_hmwds_ast_add_notify(peer, mac_addr,
				peer_type, err, true);

	return err;
}

void dp_peer_free_hmwds_cb(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			   struct cdp_soc *dp_soc,
			   void *cookie,
			   enum cdp_ast_free_status status)
{
	struct dp_ast_free_cb_params *param =
		(struct dp_ast_free_cb_params *)cookie;
	struct dp_soc *soc = (struct dp_soc *)dp_soc;
	struct dp_peer *peer = NULL;
	QDF_STATUS err = QDF_STATUS_SUCCESS;

	if (status != CDP_TXRX_AST_DELETED) {
		qdf_mem_free(cookie);
		return;
	}

	peer = dp_peer_find_hash_find(soc, &param->peer_mac_addr.raw[0],
				      0, param->vdev_id, DP_MOD_ID_AST);
	if (peer) {
		err = dp_peer_add_ast(soc, peer,
				      &param->mac_addr.raw[0],
				      param->type,
				      param->flags);

		dp_hmwds_ast_add_notify(peer, &param->mac_addr.raw[0],
					param->type, err, false);

		dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	}
	qdf_mem_free(cookie);
}

/*
 * dp_peer_add_ast() - Allocate and add AST entry into peer list
 * @soc: SoC handle
 * @peer: peer to which ast node belongs
 * @mac_addr: MAC address of ast node
 * @is_self: Is this base AST entry with peer mac address
 *
 * This API is used by WDS source port learning function to
 * add a new AST entry into peer AST list
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS dp_peer_add_ast(struct dp_soc *soc,
			   struct dp_peer *peer,
			   uint8_t *mac_addr,
			   enum cdp_txrx_ast_entry_type type,
			   uint32_t flags)
{
	struct dp_ast_entry *ast_entry = NULL;
	struct dp_vdev *vdev = NULL;
	struct dp_pdev *pdev = NULL;
	uint8_t next_node_mac[6];
	txrx_ast_free_cb cb = NULL;
	void *cookie = NULL;
	struct dp_peer *vap_bss_peer = NULL;
	bool is_peer_found = false;

	if (soc->ast_offload_support)
		return QDF_STATUS_E_INVAL;

	vdev = peer->vdev;
	if (!vdev) {
		dp_peer_err("%pK: Peers vdev is NULL", soc);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	pdev = vdev->pdev;

	is_peer_found = dp_peer_exist_on_pdev(soc, mac_addr, 0, pdev);

	qdf_spin_lock_bh(&soc->ast_lock);

	if (!dp_peer_state_cmp(peer, DP_PEER_STATE_ACTIVE)) {
		if ((type != CDP_TXRX_AST_TYPE_STATIC) &&
		    (type != CDP_TXRX_AST_TYPE_SELF)) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			return QDF_STATUS_E_BUSY;
		}
	}

	dp_peer_debug("%pK: pdevid: %u vdev: %u  ast_entry->type: %d flags: 0x%x peer_mac: " QDF_MAC_ADDR_FMT " peer: %pK mac " QDF_MAC_ADDR_FMT,
		      soc, pdev->pdev_id, vdev->vdev_id, type, flags,
		      QDF_MAC_ADDR_REF(peer->mac_addr.raw), peer,
		      QDF_MAC_ADDR_REF(mac_addr));

	/* fw supports only 2 times the max_peers ast entries */
	if (soc->num_ast_entries >=
	    wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		dp_peer_err("%pK: Max ast entries reached", soc);
		return QDF_STATUS_E_RESOURCES;
	}

	/* If AST entry already exists , just return from here
	 * ast entry with same mac address can exist on different radios
	 * if ast_override support is enabled use search by pdev in this
	 * case
	 */
	if (soc->ast_override_support) {
		ast_entry = dp_peer_ast_hash_find_by_pdevid(soc, mac_addr,
							    pdev->pdev_id);
		if (ast_entry) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			return QDF_STATUS_E_ALREADY;
		}

		if (is_peer_found) {
			/* During WDS to static roaming, peer is added
			 * to the list before static AST entry create.
			 * So, allow AST entry for STATIC type
			 * even if peer is present
			 */
			if (type != CDP_TXRX_AST_TYPE_STATIC) {
				qdf_spin_unlock_bh(&soc->ast_lock);
				return QDF_STATUS_E_ALREADY;
			}
		}
	} else {
		/* For HWMWDS_SEC entries can be added for same mac address
		 * do not check for existing entry
		 */
		if (type == CDP_TXRX_AST_TYPE_WDS_HM_SEC)
			goto add_ast_entry;

		ast_entry = dp_peer_ast_hash_find_soc(soc, mac_addr);

		if (ast_entry) {
			if ((ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM) &&
			    !ast_entry->delete_in_progress) {
				qdf_spin_unlock_bh(&soc->ast_lock);
				return QDF_STATUS_E_ALREADY;
			}

			/* Add for HMWDS entry we cannot be ignored if there
			 * is AST entry with same mac address
			 *
			 * if ast entry exists with the requested mac address
			 * send a delete command and register callback which
			 * can take care of adding HMWDS ast enty on delete
			 * confirmation from target
			 */
			if (type == CDP_TXRX_AST_TYPE_WDS_HM) {
				struct dp_ast_free_cb_params *param = NULL;

				if (ast_entry->type ==
					CDP_TXRX_AST_TYPE_WDS_HM_SEC)
					goto add_ast_entry;

				/* save existing callback */
				if (ast_entry->callback) {
					cb = ast_entry->callback;
					cookie = ast_entry->cookie;
				}

				param = qdf_mem_malloc(sizeof(*param));
				if (!param) {
					QDF_TRACE(QDF_MODULE_ID_TXRX,
						  QDF_TRACE_LEVEL_ERROR,
						  "Allocation failed");
					qdf_spin_unlock_bh(&soc->ast_lock);
					return QDF_STATUS_E_NOMEM;
				}

				qdf_mem_copy(&param->mac_addr.raw[0], mac_addr,
					     QDF_MAC_ADDR_SIZE);
				qdf_mem_copy(&param->peer_mac_addr.raw[0],
					     &peer->mac_addr.raw[0],
					     QDF_MAC_ADDR_SIZE);
				param->type = type;
				param->flags = flags;
				param->vdev_id = vdev->vdev_id;
				ast_entry->callback = dp_peer_free_hmwds_cb;
				ast_entry->pdev_id = vdev->pdev->pdev_id;
				ast_entry->type = type;
				ast_entry->cookie = (void *)param;
				if (!ast_entry->delete_in_progress)
					dp_peer_del_ast(soc, ast_entry);

				qdf_spin_unlock_bh(&soc->ast_lock);

				/* Call the saved callback*/
				if (cb) {
					cb(soc->ctrl_psoc,
					   dp_soc_to_cdp_soc(soc),
					   cookie,
					   CDP_TXRX_AST_DELETE_IN_PROGRESS);
				}
				return QDF_STATUS_E_AGAIN;
			}

			qdf_spin_unlock_bh(&soc->ast_lock);
			return QDF_STATUS_E_ALREADY;
		}
	}

add_ast_entry:
	ast_entry = (struct dp_ast_entry *)
			qdf_mem_malloc(sizeof(struct dp_ast_entry));

	if (!ast_entry) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		dp_peer_err("%pK: fail to allocate ast_entry", soc);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(&ast_entry->mac_addr.raw[0], mac_addr, QDF_MAC_ADDR_SIZE);
	ast_entry->pdev_id = vdev->pdev->pdev_id;
	ast_entry->is_mapped = false;
	ast_entry->delete_in_progress = false;
	ast_entry->peer_id = HTT_INVALID_PEER;
	ast_entry->next_hop = 0;
	ast_entry->vdev_id = vdev->vdev_id;

	switch (type) {
	case CDP_TXRX_AST_TYPE_STATIC:
		peer->self_ast_entry = ast_entry;
		ast_entry->type = CDP_TXRX_AST_TYPE_STATIC;
		if (peer->vdev->opmode == wlan_op_mode_sta)
			ast_entry->type = CDP_TXRX_AST_TYPE_STA_BSS;
		break;
	case CDP_TXRX_AST_TYPE_SELF:
		peer->self_ast_entry = ast_entry;
		ast_entry->type = CDP_TXRX_AST_TYPE_SELF;
		break;
	case CDP_TXRX_AST_TYPE_WDS:
		ast_entry->next_hop = 1;
		ast_entry->type = CDP_TXRX_AST_TYPE_WDS;
		break;
	case CDP_TXRX_AST_TYPE_WDS_HM:
		ast_entry->next_hop = 1;
		ast_entry->type = CDP_TXRX_AST_TYPE_WDS_HM;
		break;
	case CDP_TXRX_AST_TYPE_WDS_HM_SEC:
		ast_entry->next_hop = 1;
		ast_entry->type = CDP_TXRX_AST_TYPE_WDS_HM_SEC;
		ast_entry->peer_id = peer->peer_id;
		TAILQ_INSERT_TAIL(&peer->ast_entry_list, ast_entry,
				  ase_list_elem);
		break;
	case CDP_TXRX_AST_TYPE_DA:
		vap_bss_peer = dp_vdev_bss_peer_ref_n_get(soc, vdev,
							  DP_MOD_ID_AST);
		if (!vap_bss_peer) {
			qdf_spin_unlock_bh(&soc->ast_lock);
			qdf_mem_free(ast_entry);
			return QDF_STATUS_E_FAILURE;
		}
		peer = vap_bss_peer;
		ast_entry->next_hop = 1;
		ast_entry->type = CDP_TXRX_AST_TYPE_DA;
		break;
	default:
		dp_peer_err("%pK: Incorrect AST entry type", soc);
	}

	ast_entry->is_active = TRUE;
	DP_STATS_INC(soc, ast.added, 1);
	soc->num_ast_entries++;
	dp_peer_ast_hash_add(soc, ast_entry);

	qdf_copy_macaddr((struct qdf_mac_addr *)next_node_mac,
			 (struct qdf_mac_addr *)peer->mac_addr.raw);

	if ((ast_entry->type != CDP_TXRX_AST_TYPE_STATIC) &&
	    (ast_entry->type != CDP_TXRX_AST_TYPE_SELF) &&
	    (ast_entry->type != CDP_TXRX_AST_TYPE_STA_BSS) &&
	    (ast_entry->type != CDP_TXRX_AST_TYPE_WDS_HM_SEC)) {
		if (QDF_STATUS_SUCCESS ==
				soc->cdp_soc.ol_ops->peer_add_wds_entry(
				soc->ctrl_psoc,
				peer->vdev->vdev_id,
				peer->mac_addr.raw,
				peer->peer_id,
				mac_addr,
				next_node_mac,
				flags,
				ast_entry->type)) {
			if (vap_bss_peer)
				dp_peer_unref_delete(vap_bss_peer,
						     DP_MOD_ID_AST);
			qdf_spin_unlock_bh(&soc->ast_lock);
			return QDF_STATUS_SUCCESS;
		}
	}

	if (vap_bss_peer)
		dp_peer_unref_delete(vap_bss_peer, DP_MOD_ID_AST);

	qdf_spin_unlock_bh(&soc->ast_lock);
	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(dp_peer_add_ast);

/*
 * dp_peer_free_ast_entry() - Free up the ast entry memory
 * @soc: SoC handle
 * @ast_entry: Address search entry
 *
 * This API is used to free up the memory associated with
 * AST entry.
 *
 * Return: None
 */
void dp_peer_free_ast_entry(struct dp_soc *soc,
			    struct dp_ast_entry *ast_entry)
{
	/*
	 * NOTE: Ensure that call to this API is done
	 * after soc->ast_lock is taken
	 */
	dp_peer_debug("type: %d ID: %u vid: %u mac_addr: " QDF_MAC_ADDR_FMT,
		      ast_entry->type, ast_entry->peer_id, ast_entry->vdev_id,
		      QDF_MAC_ADDR_REF(ast_entry->mac_addr.raw));

	ast_entry->callback = NULL;
	ast_entry->cookie = NULL;

	DP_STATS_INC(soc, ast.deleted, 1);
	dp_peer_ast_hash_remove(soc, ast_entry);
	dp_peer_ast_cleanup(soc, ast_entry);
	qdf_mem_free(ast_entry);
	soc->num_ast_entries--;
}

/*
 * dp_peer_unlink_ast_entry() - Free up the ast entry memory
 * @soc: SoC handle
 * @ast_entry: Address search entry
 * @peer: peer
 *
 * This API is used to remove/unlink AST entry from the peer list
 * and hash list.
 *
 * Return: None
 */
void dp_peer_unlink_ast_entry(struct dp_soc *soc,
			      struct dp_ast_entry *ast_entry,
			      struct dp_peer *peer)
{
	if (!peer) {
		dp_info_rl("NULL peer");
		return;
	}

	if (ast_entry->peer_id == HTT_INVALID_PEER) {
		dp_info_rl("Invalid peer id in AST entry mac addr:"QDF_MAC_ADDR_FMT" type:%d",
			  QDF_MAC_ADDR_REF(ast_entry->mac_addr.raw),
			  ast_entry->type);
		return;
	}
	/*
	 * NOTE: Ensure that call to this API is done
	 * after soc->ast_lock is taken
	 */

	qdf_assert_always(ast_entry->peer_id == peer->peer_id);
	TAILQ_REMOVE(&peer->ast_entry_list, ast_entry, ase_list_elem);

	if (ast_entry == peer->self_ast_entry)
		peer->self_ast_entry = NULL;

	/*
	 * release the reference only if it is mapped
	 * to ast_table
	 */
	if (ast_entry->is_mapped)
		soc->ast_table[ast_entry->ast_idx] = NULL;

	ast_entry->peer_id = HTT_INVALID_PEER;
}

/*
 * dp_peer_del_ast() - Delete and free AST entry
 * @soc: SoC handle
 * @ast_entry: AST entry of the node
 *
 * This function removes the AST entry from peer and soc tables
 * It assumes caller has taken the ast lock to protect the access to these
 * tables
 *
 * Return: None
 */
void dp_peer_del_ast(struct dp_soc *soc, struct dp_ast_entry *ast_entry)
{
	struct dp_peer *peer = NULL;

	if (soc->ast_offload_support)
		return;

	if (!ast_entry) {
		dp_info_rl("NULL AST entry");
		return;
	}

	if (ast_entry->delete_in_progress) {
		dp_info_rl("AST entry deletion in progress mac addr:"QDF_MAC_ADDR_FMT" type:%d",
			  QDF_MAC_ADDR_REF(ast_entry->mac_addr.raw),
			  ast_entry->type);
		return;
	}

	dp_peer_debug("call by %ps: ID: %u vid: %u mac_addr: " QDF_MAC_ADDR_FMT,
		      (void *)_RET_IP_, ast_entry->peer_id, ast_entry->vdev_id,
		      QDF_MAC_ADDR_REF(ast_entry->mac_addr.raw));

	ast_entry->delete_in_progress = true;

	/* In teardown del ast is called after setting logical delete state
	 * use __dp_peer_get_ref_by_id to get the reference irrespective of
	 * state
	 */
	peer = __dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
				       DP_MOD_ID_AST);

	dp_peer_ast_send_wds_del(soc, ast_entry, peer);

	/* Remove SELF and STATIC entries in teardown itself */
	if (!ast_entry->next_hop)
		dp_peer_unlink_ast_entry(soc, ast_entry, peer);

	if (ast_entry->is_mapped)
		soc->ast_table[ast_entry->ast_idx] = NULL;

	/* if peer map v2 is enabled we are not freeing ast entry
	 * here and it is supposed to be freed in unmap event (after
	 * we receive delete confirmation from target)
	 *
	 * if peer_id is invalid we did not get the peer map event
	 * for the peer free ast entry from here only in this case
	 */
	if (dp_peer_ast_free_in_unmap_supported(soc, ast_entry))
		goto end;

	/* for WDS secondary entry ast_entry->next_hop would be set so
	 * unlinking has to be done explicitly here.
	 * As this entry is not a mapped entry unmap notification from
	 * FW wil not come. Hence unlinkling is done right here.
	 */

	if (ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM_SEC)
		dp_peer_unlink_ast_entry(soc, ast_entry, peer);

	dp_peer_free_ast_entry(soc, ast_entry);

end:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_AST);
}

/*
 * dp_peer_update_ast() - Delete and free AST entry
 * @soc: SoC handle
 * @peer: peer to which ast node belongs
 * @ast_entry: AST entry of the node
 * @flags: wds or hmwds
 *
 * This function update the AST entry to the roamed peer and soc tables
 * It assumes caller has taken the ast lock to protect the access to these
 * tables
 *
 * Return: 0 if ast entry is updated successfully
 *         -1 failure
 */
int dp_peer_update_ast(struct dp_soc *soc, struct dp_peer *peer,
		       struct dp_ast_entry *ast_entry, uint32_t flags)
{
	int ret = -1;
	struct dp_peer *old_peer;

	if (soc->ast_offload_support)
		return QDF_STATUS_E_INVAL;

	dp_peer_debug("%pK: ast_entry->type: %d pdevid: %u vdevid: %u flags: 0x%x mac_addr: " QDF_MAC_ADDR_FMT " peer_mac: " QDF_MAC_ADDR_FMT "\n",
		      soc, ast_entry->type, peer->vdev->pdev->pdev_id,
		      peer->vdev->vdev_id, flags,
		      QDF_MAC_ADDR_REF(ast_entry->mac_addr.raw),
		      QDF_MAC_ADDR_REF(peer->mac_addr.raw));

	/* Do not send AST update in below cases
	 *  1) Ast entry delete has already triggered
	 *  2) Peer delete is already triggered
	 *  3) We did not get the HTT map for create event
	 */
	if (ast_entry->delete_in_progress ||
	    !dp_peer_state_cmp(peer, DP_PEER_STATE_ACTIVE) ||
	    !ast_entry->is_mapped)
		return ret;

	if ((ast_entry->type == CDP_TXRX_AST_TYPE_STATIC) ||
	    (ast_entry->type == CDP_TXRX_AST_TYPE_SELF) ||
	    (ast_entry->type == CDP_TXRX_AST_TYPE_STA_BSS) ||
	    (ast_entry->type == CDP_TXRX_AST_TYPE_WDS_HM_SEC))
		return 0;

	/*
	 * Avoids flood of WMI update messages sent to FW for same peer.
	 */
	if (qdf_unlikely(ast_entry->peer_id == peer->peer_id) &&
	    (ast_entry->type == CDP_TXRX_AST_TYPE_WDS) &&
	    (ast_entry->vdev_id == peer->vdev->vdev_id) &&
	    (ast_entry->is_active))
		return 0;

	old_peer = dp_peer_get_ref_by_id(soc, ast_entry->peer_id,
					 DP_MOD_ID_AST);
	if (!old_peer)
		return 0;

	TAILQ_REMOVE(&old_peer->ast_entry_list, ast_entry, ase_list_elem);

	dp_peer_unref_delete(old_peer, DP_MOD_ID_AST);

	ast_entry->peer_id = peer->peer_id;
	ast_entry->type = CDP_TXRX_AST_TYPE_WDS;
	ast_entry->pdev_id = peer->vdev->pdev->pdev_id;
	ast_entry->vdev_id = peer->vdev->vdev_id;
	ast_entry->is_active = TRUE;
	TAILQ_INSERT_TAIL(&peer->ast_entry_list, ast_entry, ase_list_elem);

	ret = soc->cdp_soc.ol_ops->peer_update_wds_entry(
				soc->ctrl_psoc,
				peer->vdev->vdev_id,
				ast_entry->mac_addr.raw,
				peer->mac_addr.raw,
				flags);

	return ret;
}

/*
 * dp_peer_ast_get_pdev_id() - get pdev_id from the ast entry
 * @soc: SoC handle
 * @ast_entry: AST entry of the node
 *
 * This function gets the pdev_id from the ast entry.
 *
 * Return: (uint8_t) pdev_id
 */
uint8_t dp_peer_ast_get_pdev_id(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry)
{
	return ast_entry->pdev_id;
}

/*
 * dp_peer_ast_get_next_hop() - get next_hop from the ast entry
 * @soc: SoC handle
 * @ast_entry: AST entry of the node
 *
 * This function gets the next hop from the ast entry.
 *
 * Return: (uint8_t) next_hop
 */
uint8_t dp_peer_ast_get_next_hop(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry)
{
	return ast_entry->next_hop;
}

/*
 * dp_peer_ast_set_type() - set type from the ast entry
 * @soc: SoC handle
 * @ast_entry: AST entry of the node
 *
 * This function sets the type in the ast entry.
 *
 * Return:
 */
void dp_peer_ast_set_type(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry,
				enum cdp_txrx_ast_entry_type type)
{
	ast_entry->type = type;
}

#else
QDF_STATUS dp_peer_add_ast(struct dp_soc *soc,
			   struct dp_peer *peer,
			   uint8_t *mac_addr,
			   enum cdp_txrx_ast_entry_type type,
			   uint32_t flags)
{
	return QDF_STATUS_E_FAILURE;
}

void dp_peer_del_ast(struct dp_soc *soc, struct dp_ast_entry *ast_entry)
{
}

int dp_peer_update_ast(struct dp_soc *soc, struct dp_peer *peer,
			struct dp_ast_entry *ast_entry, uint32_t flags)
{
	return 1;
}

struct dp_ast_entry *dp_peer_ast_hash_find_soc(struct dp_soc *soc,
					       uint8_t *ast_mac_addr)
{
	return NULL;
}

struct dp_ast_entry *dp_peer_ast_hash_find_by_pdevid(struct dp_soc *soc,
						     uint8_t *ast_mac_addr,
						     uint8_t pdev_id)
{
	return NULL;
}

QDF_STATUS dp_peer_ast_hash_attach(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_peer_map_ast(struct dp_soc *soc,
					 struct dp_peer *peer,
					 uint8_t *mac_addr,
					 uint16_t hw_peer_id,
					 uint8_t vdev_id,
					 uint16_t ast_hash,
					 uint8_t is_wds)
{
	return QDF_STATUS_SUCCESS;
}

void dp_peer_ast_hash_detach(struct dp_soc *soc)
{
}

void dp_peer_ast_set_type(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry,
				enum cdp_txrx_ast_entry_type type)
{
}

uint8_t dp_peer_ast_get_pdev_id(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry)
{
	return 0xff;
}

uint8_t dp_peer_ast_get_next_hop(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry)
{
	return 0xff;
}

int dp_peer_update_ast(struct dp_soc *soc, struct dp_peer *peer,
		       struct dp_ast_entry *ast_entry, uint32_t flags)
{
	return 1;
}

#endif

void dp_peer_ast_send_wds_del(struct dp_soc *soc,
			      struct dp_ast_entry *ast_entry,
			      struct dp_peer *peer)
{
	struct cdp_soc_t *cdp_soc = &soc->cdp_soc;
	bool delete_in_fw = false;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_TRACE,
		  "%s: ast_entry->type: %d pdevid: %u vdev: %u mac_addr: "QDF_MAC_ADDR_FMT" next_hop: %u peer_id: %uM\n",
		  __func__, ast_entry->type, ast_entry->pdev_id,
		  ast_entry->vdev_id,
		  QDF_MAC_ADDR_REF(ast_entry->mac_addr.raw),
		  ast_entry->next_hop, ast_entry->peer_id);

	/*
	 * If peer state is logical delete, the peer is about to get
	 * teared down with a peer delete command to firmware,
	 * which will cleanup all the wds ast entries.
	 * So, no need to send explicit wds ast delete to firmware.
	 */
	if (ast_entry->next_hop) {
		if (peer && dp_peer_state_cmp(peer,
					      DP_PEER_STATE_LOGICAL_DELETE))
			delete_in_fw = false;
		else
			delete_in_fw = true;

		cdp_soc->ol_ops->peer_del_wds_entry(soc->ctrl_psoc,
						    ast_entry->vdev_id,
						    ast_entry->mac_addr.raw,
						    ast_entry->type,
						    delete_in_fw);
	}

}

#ifdef FEATURE_WDS
/**
 * dp_peer_ast_free_wds_entries() - Free wds ast entries associated with peer
 * @soc: soc handle
 * @peer: peer handle
 *
 * Free all the wds ast entries associated with peer
 *
 * Return: Number of wds ast entries freed
 */
static uint32_t dp_peer_ast_free_wds_entries(struct dp_soc *soc,
					     struct dp_peer *peer)
{
	TAILQ_HEAD(, dp_ast_entry) ast_local_list = {0};
	struct dp_ast_entry *ast_entry, *temp_ast_entry;
	uint32_t num_ast = 0;

	TAILQ_INIT(&ast_local_list);
	qdf_spin_lock_bh(&soc->ast_lock);

	DP_PEER_ITERATE_ASE_LIST(peer, ast_entry, temp_ast_entry) {
		if (ast_entry->next_hop)
			num_ast++;

		if (ast_entry->is_mapped)
			soc->ast_table[ast_entry->ast_idx] = NULL;

		dp_peer_unlink_ast_entry(soc, ast_entry, peer);
		DP_STATS_INC(soc, ast.deleted, 1);
		dp_peer_ast_hash_remove(soc, ast_entry);
		TAILQ_INSERT_TAIL(&ast_local_list, ast_entry,
				  ase_list_elem);
		soc->num_ast_entries--;
	}

	qdf_spin_unlock_bh(&soc->ast_lock);

	TAILQ_FOREACH_SAFE(ast_entry, &ast_local_list, ase_list_elem,
			   temp_ast_entry) {
		if (ast_entry->callback)
			ast_entry->callback(soc->ctrl_psoc,
					    dp_soc_to_cdp_soc(soc),
					    ast_entry->cookie,
					    CDP_TXRX_AST_DELETED);

		qdf_mem_free(ast_entry);
	}

	return num_ast;
}
/**
 * dp_peer_clean_wds_entries() - Clean wds ast entries and compare
 * @soc: soc handle
 * @peer: peer handle
 * @free_wds_count - number of wds entries freed by FW with peer delete
 *
 * Free all the wds ast entries associated with peer and compare with
 * the value received from firmware
 *
 * Return: Number of wds ast entries freed
 */
static void
dp_peer_clean_wds_entries(struct dp_soc *soc, struct dp_peer *peer,
			  uint32_t free_wds_count)
{
	uint32_t wds_deleted = 0;

	if (soc->ast_offload_support)
		return;

	wds_deleted = dp_peer_ast_free_wds_entries(soc, peer);
	if ((DP_PEER_WDS_COUNT_INVALID != free_wds_count) &&
	    (free_wds_count != wds_deleted)) {
		DP_STATS_INC(soc, ast.ast_mismatch, 1);
		dp_alert("For peer %pK (mac: "QDF_MAC_ADDR_FMT")number of wds entries deleted by fw = %d during peer delete is not same as the numbers deleted by host = %d",
			 peer, peer->mac_addr.raw, free_wds_count,
			 wds_deleted);
	}
}

#else
static void
dp_peer_clean_wds_entries(struct dp_soc *soc, struct dp_peer *peer,
			  uint32_t free_wds_count)
{
	struct dp_ast_entry *ast_entry, *temp_ast_entry;

	qdf_spin_lock_bh(&soc->ast_lock);

	DP_PEER_ITERATE_ASE_LIST(peer, ast_entry, temp_ast_entry) {
		dp_peer_unlink_ast_entry(soc, ast_entry, peer);

		if (ast_entry->is_mapped)
			soc->ast_table[ast_entry->ast_idx] = NULL;

		dp_peer_free_ast_entry(soc, ast_entry);
	}

	peer->self_ast_entry = NULL;
	qdf_spin_unlock_bh(&soc->ast_lock);
}
#endif

/**
 * dp_peer_ast_free_entry_by_mac() - find ast entry by MAC address and delete
 * @soc: soc handle
 * @peer: peer handle
 * @vdev_id: vdev_id
 * @mac_addr: mac address of the AST entry to searc and delete
 *
 * find the ast entry from the peer list using the mac address and free
 * the entry.
 *
 * Return: SUCCESS or NOENT
 */
static int dp_peer_ast_free_entry_by_mac(struct dp_soc *soc,
					 struct dp_peer *peer,
					 uint8_t vdev_id,
					 uint8_t *mac_addr)
{
	struct dp_ast_entry *ast_entry;
	void *cookie = NULL;
	txrx_ast_free_cb cb = NULL;

	/*
	 * release the reference only if it is mapped
	 * to ast_table
	 */

	qdf_spin_lock_bh(&soc->ast_lock);

	ast_entry = dp_peer_ast_hash_find_by_vdevid(soc, mac_addr, vdev_id);
	if (!ast_entry) {
		qdf_spin_unlock_bh(&soc->ast_lock);
		return QDF_STATUS_E_NOENT;
	} else if (ast_entry->is_mapped) {
		soc->ast_table[ast_entry->ast_idx] = NULL;
	}

	cb = ast_entry->callback;
	cookie = ast_entry->cookie;


	dp_peer_unlink_ast_entry(soc, ast_entry, peer);

	dp_peer_free_ast_entry(soc, ast_entry);

	qdf_spin_unlock_bh(&soc->ast_lock);

	if (cb) {
		cb(soc->ctrl_psoc,
		   dp_soc_to_cdp_soc(soc),
		   cookie,
		   CDP_TXRX_AST_DELETED);
	}

	return QDF_STATUS_SUCCESS;
}

void dp_peer_find_hash_erase(struct dp_soc *soc)
{
	int i;

	/*
	 * Not really necessary to take peer_ref_mutex lock - by this point,
	 * it's known that the soc is no longer in use.
	 */
	for (i = 0; i <= soc->peer_hash.mask; i++) {
		if (!TAILQ_EMPTY(&soc->peer_hash.bins[i])) {
			struct dp_peer *peer, *peer_next;

			/*
			 * TAILQ_FOREACH_SAFE must be used here to avoid any
			 * memory access violation after peer is freed
			 */
			TAILQ_FOREACH_SAFE(peer, &soc->peer_hash.bins[i],
				hash_list_elem, peer_next) {
				/*
				 * Don't remove the peer from the hash table -
				 * that would modify the list we are currently
				 * traversing, and it's not necessary anyway.
				 */
				/*
				 * Artificially adjust the peer's ref count to
				 * 1, so it will get deleted by
				 * dp_peer_unref_delete.
				 */
				/* set to zero */
				qdf_atomic_init(&peer->ref_cnt);
				for (i = 0; i < DP_MOD_ID_MAX; i++)
					qdf_atomic_init(&peer->mod_refs[i]);
				/* incr to one */
				qdf_atomic_inc(&peer->ref_cnt);
				qdf_atomic_inc(&peer->mod_refs
						[DP_MOD_ID_CONFIG]);
				dp_peer_unref_delete(peer,
						     DP_MOD_ID_CONFIG);
			}
		}
	}
}

void dp_peer_ast_table_detach(struct dp_soc *soc)
{
	if (soc->ast_table) {
		qdf_mem_free(soc->ast_table);
		soc->ast_table = NULL;
	}
}

/*
 * dp_peer_find_map_detach() - cleanup memory for peer_id_to_obj_map
 * @soc: soc handle
 *
 * return: none
 */
void dp_peer_find_map_detach(struct dp_soc *soc)
{
	if (soc->peer_id_to_obj_map) {
		qdf_mem_free(soc->peer_id_to_obj_map);
		soc->peer_id_to_obj_map = NULL;
		qdf_spinlock_destroy(&soc->peer_map_lock);
	}
}

#ifndef AST_OFFLOAD_ENABLE
QDF_STATUS dp_peer_find_attach(struct dp_soc *soc)
{
	QDF_STATUS status;

	status = dp_peer_find_map_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	status = dp_peer_find_hash_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto map_detach;

	status = dp_peer_ast_table_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto hash_detach;

	status = dp_peer_ast_hash_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto ast_table_detach;

	status = dp_peer_mec_hash_attach(soc);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		dp_soc_wds_attach(soc);
		return status;
	}

	dp_peer_ast_hash_detach(soc);
ast_table_detach:
	dp_peer_ast_table_detach(soc);
hash_detach:
	dp_peer_find_hash_detach(soc);
map_detach:
	dp_peer_find_map_detach(soc);

	return status;
}
#else
QDF_STATUS dp_peer_find_attach(struct dp_soc *soc)
{
	QDF_STATUS status;

	status = dp_peer_find_map_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	status = dp_peer_find_hash_attach(soc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto map_detach;

	return status;
map_detach:
	dp_peer_find_map_detach(soc);

	return status;
}
#endif

void dp_rx_tid_stats_cb(struct dp_soc *soc, void *cb_ctxt,
	union hal_reo_status *reo_status)
{
	struct dp_rx_tid *rx_tid = (struct dp_rx_tid *)cb_ctxt;
	struct hal_reo_queue_status *queue_status = &(reo_status->queue_status);

	if (queue_status->header.status == HAL_REO_CMD_DRAIN)
		return;

	if (queue_status->header.status != HAL_REO_CMD_SUCCESS) {
		DP_PRINT_STATS("REO stats failure %d for TID %d\n",
			       queue_status->header.status, rx_tid->tid);
		return;
	}

	DP_PRINT_STATS("REO queue stats (TID: %d):\n"
		       "ssn: %d\n"
		       "curr_idx  : %d\n"
		       "pn_31_0   : %08x\n"
		       "pn_63_32  : %08x\n"
		       "pn_95_64  : %08x\n"
		       "pn_127_96 : %08x\n"
		       "last_rx_enq_tstamp : %08x\n"
		       "last_rx_deq_tstamp : %08x\n"
		       "rx_bitmap_31_0     : %08x\n"
		       "rx_bitmap_63_32    : %08x\n"
		       "rx_bitmap_95_64    : %08x\n"
		       "rx_bitmap_127_96   : %08x\n"
		       "rx_bitmap_159_128  : %08x\n"
		       "rx_bitmap_191_160  : %08x\n"
		       "rx_bitmap_223_192  : %08x\n"
		       "rx_bitmap_255_224  : %08x\n",
		       rx_tid->tid,
		       queue_status->ssn, queue_status->curr_idx,
		       queue_status->pn_31_0, queue_status->pn_63_32,
		       queue_status->pn_95_64, queue_status->pn_127_96,
		       queue_status->last_rx_enq_tstamp,
		       queue_status->last_rx_deq_tstamp,
		       queue_status->rx_bitmap_31_0,
		       queue_status->rx_bitmap_63_32,
		       queue_status->rx_bitmap_95_64,
		       queue_status->rx_bitmap_127_96,
		       queue_status->rx_bitmap_159_128,
		       queue_status->rx_bitmap_191_160,
		       queue_status->rx_bitmap_223_192,
		       queue_status->rx_bitmap_255_224);

	DP_PRINT_STATS(
		       "curr_mpdu_cnt      : %d\n"
		       "curr_msdu_cnt      : %d\n"
		       "fwd_timeout_cnt    : %d\n"
		       "fwd_bar_cnt        : %d\n"
		       "dup_cnt            : %d\n"
		       "frms_in_order_cnt  : %d\n"
		       "bar_rcvd_cnt       : %d\n"
		       "mpdu_frms_cnt      : %d\n"
		       "msdu_frms_cnt      : %d\n"
		       "total_byte_cnt     : %d\n"
		       "late_recv_mpdu_cnt : %d\n"
		       "win_jump_2k        : %d\n"
		       "hole_cnt           : %d\n",
		       queue_status->curr_mpdu_cnt,
		       queue_status->curr_msdu_cnt,
		       queue_status->fwd_timeout_cnt,
		       queue_status->fwd_bar_cnt,
		       queue_status->dup_cnt,
		       queue_status->frms_in_order_cnt,
		       queue_status->bar_rcvd_cnt,
		       queue_status->mpdu_frms_cnt,
		       queue_status->msdu_frms_cnt,
		       queue_status->total_cnt,
		       queue_status->late_recv_mpdu_cnt,
		       queue_status->win_jump_2k,
		       queue_status->hole_cnt);

	DP_PRINT_STATS("Addba Req          : %d\n"
			"Addba Resp         : %d\n"
			"Addba Resp success : %d\n"
			"Addba Resp failed  : %d\n"
			"Delba Req received : %d\n"
			"Delba Tx success   : %d\n"
			"Delba Tx Fail      : %d\n"
			"BA window size     : %d\n"
			"Pn size            : %d\n",
			rx_tid->num_of_addba_req,
			rx_tid->num_of_addba_resp,
			rx_tid->num_addba_rsp_success,
			rx_tid->num_addba_rsp_failed,
			rx_tid->num_of_delba_req,
			rx_tid->delba_tx_success_cnt,
			rx_tid->delba_tx_fail_cnt,
			rx_tid->ba_win_size,
			rx_tid->pn_size);
}

/*
 * dp_peer_find_add_id() - map peer_id with peer
 * @soc: soc handle
 * @peer_mac_addr: peer mac address
 * @peer_id: peer id to be mapped
 * @hw_peer_id: HW ast index
 * @vdev_id: vdev_id
 *
 * return: peer in success
 *         NULL in failure
 */
static inline struct dp_peer *dp_peer_find_add_id(struct dp_soc *soc,
	uint8_t *peer_mac_addr, uint16_t peer_id, uint16_t hw_peer_id,
	uint8_t vdev_id)
{
	struct dp_peer *peer;

	QDF_ASSERT(peer_id <= soc->max_peer_id);
	/* check if there's already a peer object with this MAC address */
	peer = dp_peer_find_hash_find(soc, peer_mac_addr,
		0 /* is aligned */, vdev_id, DP_MOD_ID_CONFIG);
	dp_peer_err("%pK: peer %pK ID %d vid %d mac " QDF_MAC_ADDR_FMT,
		    soc, peer, peer_id, vdev_id,
		    QDF_MAC_ADDR_REF(peer_mac_addr));

	if (peer) {
		/* peer's ref count was already incremented by
		 * peer_find_hash_find
		 */
		dp_peer_info("%pK: ref_cnt: %d", soc,
			     qdf_atomic_read(&peer->ref_cnt));

		/*
		 * if peer is in logical delete CP triggered delete before map
		 * is received ignore this event
		 */
		if (dp_peer_state_cmp(peer, DP_PEER_STATE_LOGICAL_DELETE)) {
			dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
			dp_alert("Peer %pK["QDF_MAC_ADDR_FMT"] logical delete state vid %d",
				 peer, QDF_MAC_ADDR_REF(peer_mac_addr),
				 vdev_id);
			return NULL;
		}
		dp_peer_find_id_to_obj_add(soc, peer, peer_id);
		dp_mlo_partner_chips_map(soc, peer, peer_id);
		if (peer->peer_id == HTT_INVALID_PEER) {
			peer->peer_id = peer_id;
			dp_monitor_peer_tid_peer_id_update(soc, peer,
							   peer->peer_id);
		} else {
			QDF_ASSERT(0);
		}

		dp_peer_update_state(soc, peer, DP_PEER_STATE_ACTIVE);
		return peer;
	}

	return NULL;
}

#ifdef WLAN_FEATURE_11BE_MLO
#ifdef DP_USE_REDUCED_PEER_ID_FIELD_WIDTH
static inline uint16_t dp_gen_ml_peer_id(struct dp_soc *soc,
					 uint16_t peer_id)
{
	return ((peer_id & soc->peer_id_mask) | (1 << soc->peer_id_shift));
}
#else
static inline uint16_t dp_gen_ml_peer_id(struct dp_soc *soc,
					 uint16_t peer_id)
{
	return (peer_id | (1 << HTT_RX_PEER_META_DATA_V1_ML_PEER_VALID_S));
}
#endif

QDF_STATUS
dp_rx_mlo_peer_map_handler(struct dp_soc *soc, uint16_t peer_id,
			   uint8_t *peer_mac_addr,
			   struct dp_mlo_flow_override_info *mlo_flow_info)
{
	struct dp_peer *peer = NULL;
	uint16_t hw_peer_id = mlo_flow_info[0].ast_idx;
	uint16_t ast_hash = mlo_flow_info[0].cache_set_num;
	uint8_t vdev_id = DP_VDEV_ALL;
	uint8_t is_wds = 0;
	uint16_t ml_peer_id = dp_gen_ml_peer_id(soc, peer_id);
	enum cdp_txrx_ast_entry_type type = CDP_TXRX_AST_TYPE_STATIC;
	QDF_STATUS err = QDF_STATUS_SUCCESS;

	dp_info("mlo_peer_map_event (soc:%pK): peer_id %d ml_peer_id %d, peer_mac "QDF_MAC_ADDR_FMT,
		soc, peer_id, ml_peer_id,
		QDF_MAC_ADDR_REF(peer_mac_addr));

	peer = dp_peer_find_add_id(soc, peer_mac_addr, ml_peer_id,
				   hw_peer_id, vdev_id);

	if (peer) {
		if (wlan_op_mode_sta == peer->vdev->opmode &&
		    qdf_mem_cmp(peer->mac_addr.raw,
				peer->vdev->mld_mac_addr.raw,
				QDF_MAC_ADDR_SIZE) != 0) {
			dp_peer_info("%pK: STA vdev bss_peer!!!!", soc);
			peer->bss_peer = 1;
		}

		if (peer->vdev->opmode == wlan_op_mode_sta) {
			peer->vdev->bss_ast_hash = ast_hash;
			peer->vdev->bss_ast_idx = hw_peer_id;
		}

		/* Add ast entry incase self ast entry is
		 * deleted due to DP CP sync issue
		 *
		 * self_ast_entry is modified in peer create
		 * and peer unmap path which cannot run in
		 * parllel with peer map, no lock need before
		 * referring it
		 */
		if (!peer->self_ast_entry) {
			dp_info("Add self ast from map "QDF_MAC_ADDR_FMT,
				QDF_MAC_ADDR_REF(peer_mac_addr));
			dp_peer_add_ast(soc, peer,
					peer_mac_addr,
					type, 0);
		}
	}

	err = dp_peer_map_ast(soc, peer, peer_mac_addr, hw_peer_id,
			      vdev_id, ast_hash, is_wds);

	return err;
}
#endif

#ifdef DP_RX_UDP_OVER_PEER_ROAM
void dp_rx_reset_roaming_peer(struct dp_soc *soc, uint8_t vdev_id,
			      uint8_t *peer_mac_addr)
{
	struct dp_vdev *vdev = NULL;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_HTT);
	if (vdev) {
		if (qdf_mem_cmp(vdev->roaming_peer_mac.raw, peer_mac_addr,
				QDF_MAC_ADDR_SIZE) == 0) {
			vdev->roaming_peer_status =
						WLAN_ROAM_PEER_AUTH_STATUS_NONE;
			qdf_mem_zero(vdev->roaming_peer_mac.raw,
				     QDF_MAC_ADDR_SIZE);
		}
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT);
	}
}
#endif

/**
 * dp_rx_peer_map_handler() - handle peer map event from firmware
 * @soc_handle - genereic soc handle
 * @peeri_id - peer_id from firmware
 * @hw_peer_id - ast index for this peer
 * @vdev_id - vdev ID
 * @peer_mac_addr - mac address of the peer
 * @ast_hash - ast hash value
 * @is_wds - flag to indicate peer map event for WDS ast entry
 *
 * associate the peer_id that firmware provided with peer entry
 * and update the ast table in the host with the hw_peer_id.
 *
 * Return: QDF_STATUS code
 */

QDF_STATUS
dp_rx_peer_map_handler(struct dp_soc *soc, uint16_t peer_id,
		       uint16_t hw_peer_id, uint8_t vdev_id,
		       uint8_t *peer_mac_addr, uint16_t ast_hash,
		       uint8_t is_wds)
{
	struct dp_peer *peer = NULL;
	struct dp_vdev *vdev = NULL;
	enum cdp_txrx_ast_entry_type type = CDP_TXRX_AST_TYPE_STATIC;
	QDF_STATUS err = QDF_STATUS_SUCCESS;

	dp_info("peer_map_event (soc:%pK): peer_id %d, hw_peer_id %d, peer_mac "QDF_MAC_ADDR_FMT", vdev_id %d",
		soc, peer_id, hw_peer_id,
		QDF_MAC_ADDR_REF(peer_mac_addr), vdev_id);

	/* Peer map event for WDS ast entry get the peer from
	 * obj map
	 */
	if (is_wds) {
		if (!soc->ast_offload_support) {
			peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_HTT);

			err = dp_peer_map_ast(soc, peer, peer_mac_addr,
					      hw_peer_id,
					      vdev_id, ast_hash, is_wds);
			if (peer)
				dp_peer_unref_delete(peer, DP_MOD_ID_HTT);
		}
	} else {
		/*
		 * It's the responsibility of the CP and FW to ensure
		 * that peer is created successfully. Ideally DP should
		 * not hit the below condition for directly assocaited
		 * peers.
		 */
		if ((!soc->ast_offload_support) && ((hw_peer_id < 0) ||
		    (hw_peer_id >=
		     wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx)))) {
			dp_peer_err("%pK: invalid hw_peer_id: %d", soc, hw_peer_id);
			qdf_assert_always(0);
		}

		peer = dp_peer_find_add_id(soc, peer_mac_addr, peer_id,
					   hw_peer_id, vdev_id);

		if (peer) {
			vdev = peer->vdev;
			/* Only check for STA Vdev and peer is not for TDLS */
			if (wlan_op_mode_sta == vdev->opmode &&
			    !peer->is_tdls_peer) {
				if (qdf_mem_cmp(peer->mac_addr.raw,
						vdev->mac_addr.raw,
						QDF_MAC_ADDR_SIZE) != 0) {
					dp_info("%pK: STA vdev bss_peer", soc);
					peer->bss_peer = 1;
				}

				dp_info("bss ast_hash 0x%x, ast_index 0x%x",
					ast_hash, hw_peer_id);
				vdev->bss_ast_hash = ast_hash;
				vdev->bss_ast_idx = hw_peer_id;
			}

			/* Add ast entry incase self ast entry is
			 * deleted due to DP CP sync issue
			 *
			 * self_ast_entry is modified in peer create
			 * and peer unmap path which cannot run in
			 * parllel with peer map, no lock need before
			 * referring it
			 */
			if (!soc->ast_offload_support &&
				!peer->self_ast_entry) {
				dp_info("Add self ast from map "QDF_MAC_ADDR_FMT,
					QDF_MAC_ADDR_REF(peer_mac_addr));
				dp_peer_add_ast(soc, peer,
						peer_mac_addr,
						type, 0);
			}
		}
		err = dp_peer_map_ast(soc, peer, peer_mac_addr, hw_peer_id,
				      vdev_id, ast_hash, is_wds);
	}

	dp_rx_reset_roaming_peer(soc, vdev_id, peer_mac_addr);

	return err;
}

/**
 * dp_rx_peer_unmap_handler() - handle peer unmap event from firmware
 * @soc_handle - genereic soc handle
 * @peeri_id - peer_id from firmware
 * @vdev_id - vdev ID
 * @mac_addr - mac address of the peer or wds entry
 * @is_wds - flag to indicate peer map event for WDS ast entry
 * @free_wds_count - number of wds entries freed by FW with peer delete
 *
 * Return: none
 */
void
dp_rx_peer_unmap_handler(struct dp_soc *soc, uint16_t peer_id,
			 uint8_t vdev_id, uint8_t *mac_addr,
			 uint8_t is_wds, uint32_t free_wds_count)
{
	struct dp_peer *peer;
	struct dp_vdev *vdev = NULL;

	if (soc->ast_offload_support && is_wds)
		return;

	peer = __dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_HTT);

	/*
	 * Currently peer IDs are assigned for vdevs as well as peers.
	 * If the peer ID is for a vdev, then the peer pointer stored
	 * in peer_id_to_obj_map will be NULL.
	 */
	if (!peer) {
		dp_err("Received unmap event for invalid peer_id %u",
		       peer_id);
		return;
	}

	/* If V2 Peer map messages are enabled AST entry has to be
	 * freed here
	 */
	if (is_wds) {
		if (!dp_peer_ast_free_entry_by_mac(soc, peer, vdev_id,
						   mac_addr)) {
			dp_peer_unref_delete(peer, DP_MOD_ID_HTT);
			return;
		}

		dp_alert("AST entry not found with peer %pK peer_id %u peer_mac "QDF_MAC_ADDR_FMT" mac_addr "QDF_MAC_ADDR_FMT" vdev_id %u next_hop %u",
			  peer, peer->peer_id,
			  QDF_MAC_ADDR_REF(peer->mac_addr.raw),
			  QDF_MAC_ADDR_REF(mac_addr), vdev_id,
			  is_wds);

		dp_peer_unref_delete(peer, DP_MOD_ID_HTT);
		return;
	}

	dp_peer_clean_wds_entries(soc, peer, free_wds_count);

	dp_info("peer_unmap_event (soc:%pK) peer_id %d peer %pK",
		soc, peer_id, peer);

	dp_peer_find_id_to_obj_remove(soc, peer_id);
	dp_mlo_partner_chips_unmap(soc, peer_id);
	peer->peer_id = HTT_INVALID_PEER;

	/*
	 *	 Reset ast flow mapping table
	 */
	if (!soc->ast_offload_support)
		dp_peer_reset_flowq_map(peer);

	if (soc->cdp_soc.ol_ops->peer_unmap_event) {
		soc->cdp_soc.ol_ops->peer_unmap_event(soc->ctrl_psoc,
				peer_id, vdev_id);
	}

	vdev = peer->vdev;
	DP_UPDATE_STATS(vdev, peer);

	dp_peer_update_state(soc, peer, DP_PEER_STATE_INACTIVE);
	dp_peer_unref_delete(peer, DP_MOD_ID_HTT);
	/*
	 * Remove a reference to the peer.
	 * If there are no more references, delete the peer object.
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_CONFIG);
}

#ifdef WLAN_FEATURE_11BE_MLO
void dp_rx_mlo_peer_unmap_handler(struct dp_soc *soc, uint16_t peer_id)
{
	uint16_t ml_peer_id = dp_gen_ml_peer_id(soc, peer_id);
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE] = {0};
	uint8_t vdev_id = DP_VDEV_ALL;
	uint8_t is_wds = 0;

	dp_info("MLO peer_unmap_event (soc:%pK) peer_id %d",
		soc, peer_id);

	dp_rx_peer_unmap_handler(soc, ml_peer_id, vdev_id,
				 mac_addr, is_wds,
				 DP_PEER_WDS_COUNT_INVALID);
}
#endif

#ifndef AST_OFFLOAD_ENABLE
void
dp_peer_find_detach(struct dp_soc *soc)
{
	dp_soc_wds_detach(soc);
	dp_peer_find_map_detach(soc);
	dp_peer_find_hash_detach(soc);
	dp_peer_ast_hash_detach(soc);
	dp_peer_ast_table_detach(soc);
	dp_peer_mec_hash_detach(soc);
}
#else
void
dp_peer_find_detach(struct dp_soc *soc)
{
	dp_peer_find_map_detach(soc);
	dp_peer_find_hash_detach(soc);
}
#endif

static void dp_rx_tid_update_cb(struct dp_soc *soc, void *cb_ctxt,
	union hal_reo_status *reo_status)
{
	struct dp_rx_tid *rx_tid = (struct dp_rx_tid *)cb_ctxt;

	if ((reo_status->rx_queue_status.header.status !=
		HAL_REO_CMD_SUCCESS) &&
		(reo_status->rx_queue_status.header.status !=
		HAL_REO_CMD_DRAIN)) {
		/* Should not happen normally. Just print error for now */
		dp_peer_err("%pK: Rx tid HW desc update failed(%d): tid %d",
			    soc, reo_status->rx_queue_status.header.status,
			    rx_tid->tid);
	}
}

static bool dp_get_peer_vdev_roaming_in_progress(struct dp_peer *peer)
{
	struct ol_if_ops *ol_ops = NULL;
	bool is_roaming = false;
	uint8_t vdev_id = -1;
	struct cdp_soc_t *soc;

	if (!peer) {
		dp_peer_info("Peer is NULL. No roaming possible");
		return false;
	}

	soc = dp_soc_to_cdp_soc_t(peer->vdev->pdev->soc);
	ol_ops = peer->vdev->pdev->soc->cdp_soc.ol_ops;

	if (ol_ops && ol_ops->is_roam_inprogress) {
		dp_get_vdevid(soc, peer->mac_addr.raw, &vdev_id);
		is_roaming = ol_ops->is_roam_inprogress(vdev_id);
	}

	dp_peer_info("peer: " QDF_MAC_ADDR_FMT ", vdev_id: %d, is_roaming: %d",
		     QDF_MAC_ADDR_REF(peer->mac_addr.raw), vdev_id, is_roaming);

	return is_roaming;
}

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * dp_rx_tid_setup_allow() - check if rx_tid and reo queue desc
			     setup is necessary
 * @peer: DP peer handle
 *
 * Return: true - allow, false - disallow
 */
static inline
bool dp_rx_tid_setup_allow(struct dp_peer *peer)
{
	if (IS_MLO_DP_LINK_PEER(peer) && !peer->first_link)
		return false;

	return true;
}

/**
 * dp_rx_tid_update_allow() - check if rx_tid update needed
 * @peer: DP peer handle
 *
 * Return: true - allow, false - disallow
 */
static inline
bool dp_rx_tid_update_allow(struct dp_peer *peer)
{
	/* not as expected for MLO connection link peer */
	if (IS_MLO_DP_LINK_PEER(peer)) {
		QDF_BUG(0);
		return false;
	}

	return true;
}

/**
 * dp_peer_rx_reorder_queue_setup() - Send reo queue setup wmi cmd to FW
				      per peer type
 * @soc: DP Soc handle
 * @peer: dp peer to operate on
 * @tid: TID
 * @ba_window_size: BlockAck window size
 *
 * Return: 0 - success, others - failure
 */
static QDF_STATUS dp_peer_rx_reorder_queue_setup(struct dp_soc *soc,
						 struct dp_peer *peer,
						 int tid,
						 uint32_t ba_window_size)
{
	uint8_t i;
	struct dp_mld_link_peers link_peers_info;
	struct dp_peer *link_peer;
	struct dp_rx_tid *rx_tid;
	struct dp_soc *link_peer_soc;

	rx_tid = &peer->rx_tid[tid];
	if (!rx_tid->hw_qdesc_paddr)
		return QDF_STATUS_E_INVAL;

	if (IS_MLO_DP_MLD_PEER(peer)) {
		/* get link peers with reference */
		dp_get_link_peers_ref_from_mld_peer(soc, peer,
						    &link_peers_info,
						    DP_MOD_ID_CDP);
		/* send WMI cmd to each link peers */
		for (i = 0; i < link_peers_info.num_links; i++) {
			link_peer = link_peers_info.link_peers[i];
			link_peer_soc = link_peer->vdev->pdev->soc;
			if (link_peer_soc->cdp_soc.ol_ops->
					peer_rx_reorder_queue_setup) {
				if (link_peer_soc->cdp_soc.ol_ops->
					peer_rx_reorder_queue_setup(
						link_peer_soc->ctrl_psoc,
						link_peer->vdev->pdev->pdev_id,
						link_peer->vdev->vdev_id,
						link_peer->mac_addr.raw,
						rx_tid->hw_qdesc_paddr,
						tid, tid,
						1, ba_window_size)) {
					dp_peer_err("%pK: Failed to send reo queue setup to FW - tid %d\n",
						    link_peer_soc, tid);
					return QDF_STATUS_E_FAILURE;
				}
			}
		}
		/* release link peers reference */
		dp_release_link_peers_ref(&link_peers_info, DP_MOD_ID_CDP);
	} else if (peer->peer_type == CDP_LINK_PEER_TYPE) {
			if (soc->cdp_soc.ol_ops->peer_rx_reorder_queue_setup) {
				if (soc->cdp_soc.ol_ops->
					peer_rx_reorder_queue_setup(
						soc->ctrl_psoc,
						peer->vdev->pdev->pdev_id,
						peer->vdev->vdev_id,
						peer->mac_addr.raw,
						rx_tid->hw_qdesc_paddr,
						tid, tid,
						1, ba_window_size)) {
					dp_peer_err("%pK: Failed to send reo queue setup to FW - tid %d\n",
						    soc, tid);
					return QDF_STATUS_E_FAILURE;
				}
			}
	} else {
		dp_peer_err("invalid peer type %d", peer->peer_type);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static inline
bool dp_rx_tid_setup_allow(struct dp_peer *peer)
{
	return true;
}

static inline
bool dp_rx_tid_update_allow(struct dp_peer *peer)
{
	return true;
}

static QDF_STATUS dp_peer_rx_reorder_queue_setup(struct dp_soc *soc,
						 struct dp_peer *peer,
						 int tid,
						 uint32_t ba_window_size)
{
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];

	if (!rx_tid->hw_qdesc_paddr)
		return QDF_STATUS_E_INVAL;

	if (soc->cdp_soc.ol_ops->peer_rx_reorder_queue_setup) {
		if (soc->cdp_soc.ol_ops->peer_rx_reorder_queue_setup(
		    soc->ctrl_psoc,
		    peer->vdev->pdev->pdev_id,
		    peer->vdev->vdev_id,
		    peer->mac_addr.raw, rx_tid->hw_qdesc_paddr, tid, tid,
		    1, ba_window_size)) {
			dp_peer_err("%pK: Failed to send reo queue setup to FW - tid %d\n",
				    soc, tid);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS dp_rx_tid_update_wifi3(struct dp_peer *peer, int tid, uint32_t
					 ba_window_size, uint32_t start_seq,
					 bool bar_update)
{
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];
	struct dp_soc *soc = peer->vdev->pdev->soc;
	struct hal_reo_cmd_params params;

	if (!dp_rx_tid_update_allow(peer)) {
		dp_peer_err("skip tid update for peer:" QDF_MAC_ADDR_FMT,
			    QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&params, sizeof(params));

	params.std.need_status = 1;
	params.std.addr_lo = rx_tid->hw_qdesc_paddr & 0xffffffff;
	params.std.addr_hi = (uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;
	params.u.upd_queue_params.update_ba_window_size = 1;
	params.u.upd_queue_params.ba_window_size = ba_window_size;

	if (start_seq < IEEE80211_SEQ_MAX) {
		params.u.upd_queue_params.update_ssn = 1;
		params.u.upd_queue_params.ssn = start_seq;
	} else {
	    dp_set_ssn_valid_flag(&params, 0);
	}

	if (dp_reo_send_cmd(soc, CMD_UPDATE_RX_REO_QUEUE, &params,
			    dp_rx_tid_update_cb, rx_tid)) {
		dp_err_log("failed to send reo cmd CMD_UPDATE_RX_REO_QUEUE");
		DP_STATS_INC(soc, rx.err.reo_cmd_send_fail, 1);
	}

	rx_tid->ba_win_size = ba_window_size;

	if (dp_get_peer_vdev_roaming_in_progress(peer))
		return QDF_STATUS_E_PERM;

	if (!bar_update)
		dp_peer_rx_reorder_queue_setup(soc, peer,
					       tid, ba_window_size);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_DP_FEATURE_DEFERRED_REO_QDESC_DESTROY
/*
 * dp_reo_desc_defer_free_enqueue() - enqueue REO QDESC to be freed into
 *                                    the deferred list
 * @soc: Datapath soc handle
 * @free_desc: REO DESC reference that needs to be freed
 *
 * Return: true if enqueued, else false
 */
static bool dp_reo_desc_defer_free_enqueue(struct dp_soc *soc,
					   struct reo_desc_list_node *freedesc)
{
	struct reo_desc_deferred_freelist_node *desc;

	if (!qdf_atomic_read(&soc->cmn_init_done))
		return false;

	desc = qdf_mem_malloc(sizeof(*desc));
	if (!desc)
		return false;

	desc->hw_qdesc_paddr = freedesc->rx_tid.hw_qdesc_paddr;
	desc->hw_qdesc_alloc_size = freedesc->rx_tid.hw_qdesc_alloc_size;
	desc->hw_qdesc_vaddr_unaligned =
			freedesc->rx_tid.hw_qdesc_vaddr_unaligned;
	desc->free_ts = qdf_get_system_timestamp();
	DP_RX_REO_QDESC_DEFERRED_GET_MAC(desc, freedesc);

	qdf_spin_lock_bh(&soc->reo_desc_deferred_freelist_lock);
	if (!soc->reo_desc_deferred_freelist_init) {
		qdf_mem_free(desc);
		qdf_spin_unlock_bh(&soc->reo_desc_deferred_freelist_lock);
		return false;
	}
	qdf_list_insert_back(&soc->reo_desc_deferred_freelist,
			     (qdf_list_node_t *)desc);
	qdf_spin_unlock_bh(&soc->reo_desc_deferred_freelist_lock);

	return true;
}

/*
 * dp_reo_desc_defer_free() - free the REO QDESC in the deferred list
 *                            based on time threshold
 * @soc: Datapath soc handle
 * @free_desc: REO DESC reference that needs to be freed
 *
 * Return: true if enqueued, else false
 */
static void dp_reo_desc_defer_free(struct dp_soc *soc)
{
	struct reo_desc_deferred_freelist_node *desc;
	unsigned long curr_ts = qdf_get_system_timestamp();

	qdf_spin_lock_bh(&soc->reo_desc_deferred_freelist_lock);

	while ((qdf_list_peek_front(&soc->reo_desc_deferred_freelist,
	       (qdf_list_node_t **)&desc) == QDF_STATUS_SUCCESS) &&
	       (curr_ts > (desc->free_ts + REO_DESC_DEFERRED_FREE_MS))) {
		qdf_list_remove_front(&soc->reo_desc_deferred_freelist,
				      (qdf_list_node_t **)&desc);

		DP_RX_REO_QDESC_DEFERRED_FREE_EVT(desc);

		qdf_mem_unmap_nbytes_single(soc->osdev,
					    desc->hw_qdesc_paddr,
					    QDF_DMA_BIDIRECTIONAL,
					    desc->hw_qdesc_alloc_size);
		qdf_mem_free(desc->hw_qdesc_vaddr_unaligned);
		qdf_mem_free(desc);

		curr_ts = qdf_get_system_timestamp();
	}

	qdf_spin_unlock_bh(&soc->reo_desc_deferred_freelist_lock);
}
#else
static inline bool
dp_reo_desc_defer_free_enqueue(struct dp_soc *soc,
			       struct reo_desc_list_node *freedesc)
{
	return false;
}

static void dp_reo_desc_defer_free(struct dp_soc *soc)
{
}
#endif /* !WLAN_DP_FEATURE_DEFERRED_REO_QDESC_DESTROY */

/*
 * dp_reo_desc_free() - Callback free reo descriptor memory after
 * HW cache flush
 *
 * @soc: DP SOC handle
 * @cb_ctxt: Callback context
 * @reo_status: REO command status
 */
static void dp_reo_desc_free(struct dp_soc *soc, void *cb_ctxt,
	union hal_reo_status *reo_status)
{
	struct reo_desc_list_node *freedesc =
		(struct reo_desc_list_node *)cb_ctxt;
	struct dp_rx_tid *rx_tid = &freedesc->rx_tid;
	unsigned long curr_ts = qdf_get_system_timestamp();

	if ((reo_status->fl_cache_status.header.status !=
		HAL_REO_CMD_SUCCESS) &&
		(reo_status->fl_cache_status.header.status !=
		HAL_REO_CMD_DRAIN)) {
		dp_peer_err("%pK: Rx tid HW desc flush failed(%d): tid %d",
			    soc, reo_status->rx_queue_status.header.status,
			    freedesc->rx_tid.tid);
	}
	dp_peer_info("%pK: %lu hw_qdesc_paddr: %pK, tid:%d", soc,
		     curr_ts, (void *)(rx_tid->hw_qdesc_paddr),
		     rx_tid->tid);

	/* REO desc is enqueued to be freed at a later point
	 * in time, just free the freedesc alone and return
	 */
	if (dp_reo_desc_defer_free_enqueue(soc, freedesc))
		goto out;

	DP_RX_REO_QDESC_FREE_EVT(freedesc);

	qdf_mem_unmap_nbytes_single(soc->osdev,
		rx_tid->hw_qdesc_paddr,
		QDF_DMA_BIDIRECTIONAL,
		rx_tid->hw_qdesc_alloc_size);
	qdf_mem_free(rx_tid->hw_qdesc_vaddr_unaligned);
out:
	qdf_mem_free(freedesc);
}

#if defined(CONFIG_WIFI_EMULATION_WIFI_3_0) && defined(BUILD_X86)
/* Hawkeye emulation requires bus address to be >= 0x50000000 */
static inline int dp_reo_desc_addr_chk(qdf_dma_addr_t dma_addr)
{
	if (dma_addr < 0x50000000)
		return QDF_STATUS_E_FAILURE;
	else
		return QDF_STATUS_SUCCESS;
}
#else
static inline int dp_reo_desc_addr_chk(qdf_dma_addr_t dma_addr)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * dp_rx_tid_setup_wifi3() – Setup receive TID state
 * @peer: Datapath peer handle
 * @tid: TID
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS dp_rx_tid_setup_wifi3(struct dp_peer *peer, int tid,
				 uint32_t ba_window_size, uint32_t start_seq)
{
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];
	struct dp_vdev *vdev = peer->vdev;
	struct dp_soc *soc = vdev->pdev->soc;
	uint32_t hw_qdesc_size;
	uint32_t hw_qdesc_align;
	int hal_pn_type;
	void *hw_qdesc_vaddr;
	uint32_t alloc_tries = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!qdf_atomic_read(&peer->is_default_route_set))
		return QDF_STATUS_E_FAILURE;

	if (!dp_rx_tid_setup_allow(peer)) {
		dp_peer_info("skip rx tid setup for peer" QDF_MAC_ADDR_FMT,
			     QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		goto send_wmi_reo_cmd;
	}

	rx_tid->ba_win_size = ba_window_size;
	if (rx_tid->hw_qdesc_vaddr_unaligned)
		return dp_rx_tid_update_wifi3(peer, tid, ba_window_size,
			start_seq, false);
	rx_tid->delba_tx_status = 0;
	rx_tid->ppdu_id_2k = 0;
	rx_tid->num_of_addba_req = 0;
	rx_tid->num_of_delba_req = 0;
	rx_tid->num_of_addba_resp = 0;
	rx_tid->num_addba_rsp_failed = 0;
	rx_tid->num_addba_rsp_success = 0;
	rx_tid->delba_tx_success_cnt = 0;
	rx_tid->delba_tx_fail_cnt = 0;
	rx_tid->statuscode = 0;

	/* TODO: Allocating HW queue descriptors based on max BA window size
	 * for all QOS TIDs so that same descriptor can be used later when
	 * ADDBA request is recevied. This should be changed to allocate HW
	 * queue descriptors based on BA window size being negotiated (0 for
	 * non BA cases), and reallocate when BA window size changes and also
	 * send WMI message to FW to change the REO queue descriptor in Rx
	 * peer entry as part of dp_rx_tid_update.
	 */
	if (tid != DP_NON_QOS_TID)
		hw_qdesc_size = hal_get_reo_qdesc_size(soc->hal_soc,
			HAL_RX_MAX_BA_WINDOW, tid);
	else
		hw_qdesc_size = hal_get_reo_qdesc_size(soc->hal_soc,
			ba_window_size, tid);

	hw_qdesc_align = hal_get_reo_qdesc_align(soc->hal_soc);
	/* To avoid unnecessary extra allocation for alignment, try allocating
	 * exact size and see if we already have aligned address.
	 */
	rx_tid->hw_qdesc_alloc_size = hw_qdesc_size;

try_desc_alloc:
	rx_tid->hw_qdesc_vaddr_unaligned =
		qdf_mem_malloc(rx_tid->hw_qdesc_alloc_size);

	if (!rx_tid->hw_qdesc_vaddr_unaligned) {
		dp_peer_err("%pK: Rx tid HW desc alloc failed: tid %d",
			    soc, tid);
		return QDF_STATUS_E_NOMEM;
	}

	if ((unsigned long)(rx_tid->hw_qdesc_vaddr_unaligned) %
		hw_qdesc_align) {
		/* Address allocated above is not alinged. Allocate extra
		 * memory for alignment
		 */
		qdf_mem_free(rx_tid->hw_qdesc_vaddr_unaligned);
		rx_tid->hw_qdesc_vaddr_unaligned =
			qdf_mem_malloc(rx_tid->hw_qdesc_alloc_size +
					hw_qdesc_align - 1);

		if (!rx_tid->hw_qdesc_vaddr_unaligned) {
			dp_peer_err("%pK: Rx tid HW desc alloc failed: tid %d",
				    soc, tid);
			return QDF_STATUS_E_NOMEM;
		}

		hw_qdesc_vaddr = (void *)qdf_align((unsigned long)
			rx_tid->hw_qdesc_vaddr_unaligned,
			hw_qdesc_align);

		dp_peer_debug("%pK: Total Size %d Aligned Addr %pK",
			      soc, rx_tid->hw_qdesc_alloc_size,
			      hw_qdesc_vaddr);

	} else {
		hw_qdesc_vaddr = rx_tid->hw_qdesc_vaddr_unaligned;
	}
	rx_tid->hw_qdesc_vaddr_aligned = hw_qdesc_vaddr;

	/* TODO: Ensure that sec_type is set before ADDBA is received.
	 * Currently this is set based on htt indication
	 * HTT_T2H_MSG_TYPE_SEC_IND from target
	 */
	switch (peer->security[dp_sec_ucast].sec_type) {
	case cdp_sec_type_tkip_nomic:
	case cdp_sec_type_aes_ccmp:
	case cdp_sec_type_aes_ccmp_256:
	case cdp_sec_type_aes_gcmp:
	case cdp_sec_type_aes_gcmp_256:
		hal_pn_type = HAL_PN_WPA;
		break;
	case cdp_sec_type_wapi:
		if (vdev->opmode == wlan_op_mode_ap)
			hal_pn_type = HAL_PN_WAPI_EVEN;
		else
			hal_pn_type = HAL_PN_WAPI_UNEVEN;
		break;
	default:
		hal_pn_type = HAL_PN_NONE;
		break;
	}

	hal_reo_qdesc_setup(soc->hal_soc, tid, ba_window_size, start_seq,
		hw_qdesc_vaddr, rx_tid->hw_qdesc_paddr, hal_pn_type,
		vdev->vdev_stats_id);

	qdf_mem_map_nbytes_single(soc->osdev, hw_qdesc_vaddr,
		QDF_DMA_BIDIRECTIONAL, rx_tid->hw_qdesc_alloc_size,
		&(rx_tid->hw_qdesc_paddr));

	if (dp_reo_desc_addr_chk(rx_tid->hw_qdesc_paddr) !=
			QDF_STATUS_SUCCESS) {
		if (alloc_tries++ < 10) {
			qdf_mem_free(rx_tid->hw_qdesc_vaddr_unaligned);
			rx_tid->hw_qdesc_vaddr_unaligned = NULL;
			goto try_desc_alloc;
		} else {
			dp_peer_err("%pK: Rx tid HW desc alloc failed (lowmem): tid %d",
				    soc, tid);
			status = QDF_STATUS_E_NOMEM;
			goto error;
		}
	}

send_wmi_reo_cmd:
	if (dp_get_peer_vdev_roaming_in_progress(peer)) {
		status = QDF_STATUS_E_PERM;
		goto error;
	}

	status = dp_peer_rx_reorder_queue_setup(soc, peer,
						tid, ba_window_size);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

error:
	if (rx_tid->hw_qdesc_vaddr_unaligned) {
		if (dp_reo_desc_addr_chk(rx_tid->hw_qdesc_paddr) ==
		    QDF_STATUS_SUCCESS)
			qdf_mem_unmap_nbytes_single(
				soc->osdev,
				rx_tid->hw_qdesc_paddr,
				QDF_DMA_BIDIRECTIONAL,
				rx_tid->hw_qdesc_alloc_size);
		qdf_mem_free(rx_tid->hw_qdesc_vaddr_unaligned);
		rx_tid->hw_qdesc_vaddr_unaligned = NULL;
		rx_tid->hw_qdesc_paddr = 0;
	}
	return status;
}

#ifdef REO_DESC_DEFER_FREE
/*
 * dp_reo_desc_clean_up() - If cmd to flush base desc fails add
 * desc back to freelist and defer the deletion
 *
 * @soc: DP SOC handle
 * @desc: Base descriptor to be freed
 * @reo_status: REO command status
 */
static void dp_reo_desc_clean_up(struct dp_soc *soc,
				 struct reo_desc_list_node *desc,
				 union hal_reo_status *reo_status)
{
	desc->free_ts = qdf_get_system_timestamp();
	DP_STATS_INC(soc, rx.err.reo_cmd_send_fail, 1);
	qdf_list_insert_back(&soc->reo_desc_freelist,
			     (qdf_list_node_t *)desc);
}

/*
 * dp_reo_limit_clean_batch_sz() - Limit number REO CMD queued to cmd
 * ring in aviod of REO hang
 *
 * @list_size: REO desc list size to be cleaned
 */
static inline void dp_reo_limit_clean_batch_sz(uint32_t *list_size)
{
	unsigned long curr_ts = qdf_get_system_timestamp();

	if ((*list_size) > REO_DESC_FREELIST_SIZE) {
		dp_err_log("%lu:freedesc number %d in freelist",
			   curr_ts, *list_size);
		/* limit the batch queue size */
		*list_size = REO_DESC_FREELIST_SIZE;
	}
}
#else
/*
 * dp_reo_desc_clean_up() - If send cmd to REO inorder to flush
 * cache fails free the base REO desc anyway
 *
 * @soc: DP SOC handle
 * @desc: Base descriptor to be freed
 * @reo_status: REO command status
 */
static void dp_reo_desc_clean_up(struct dp_soc *soc,
				 struct reo_desc_list_node *desc,
				 union hal_reo_status *reo_status)
{
	if (reo_status) {
		qdf_mem_zero(reo_status, sizeof(*reo_status));
		reo_status->fl_cache_status.header.status = 0;
		dp_reo_desc_free(soc, (void *)desc, reo_status);
	}
}

/*
 * dp_reo_limit_clean_batch_sz() - Limit number REO CMD queued to cmd
 * ring in aviod of REO hang
 *
 * @list_size: REO desc list size to be cleaned
 */
static inline void dp_reo_limit_clean_batch_sz(uint32_t *list_size)
{
}
#endif

/*
 * dp_resend_update_reo_cmd() - Resend the UPDATE_REO_QUEUE
 * cmd and re-insert desc into free list if send fails.
 *
 * @soc: DP SOC handle
 * @desc: desc with resend update cmd flag set
 * @rx_tid: Desc RX tid associated with update cmd for resetting
 * valid field to 0 in h/w
 *
 * Return: QDF status
 */
static QDF_STATUS
dp_resend_update_reo_cmd(struct dp_soc *soc,
			 struct reo_desc_list_node *desc,
			 struct dp_rx_tid *rx_tid)
{
	struct hal_reo_cmd_params params;

	qdf_mem_zero(&params, sizeof(params));
	params.std.need_status = 1;
	params.std.addr_lo =
		rx_tid->hw_qdesc_paddr & 0xffffffff;
	params.std.addr_hi =
		(uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;
	params.u.upd_queue_params.update_vld = 1;
	params.u.upd_queue_params.vld = 0;
	desc->resend_update_reo_cmd = false;
	/*
	 * If the cmd send fails then set resend_update_reo_cmd flag
	 * and insert the desc at the end of the free list to retry.
	 */
	if (dp_reo_send_cmd(soc,
			    CMD_UPDATE_RX_REO_QUEUE,
			    &params,
			    dp_rx_tid_delete_cb,
			    (void *)desc)
	    != QDF_STATUS_SUCCESS) {
		desc->resend_update_reo_cmd = true;
		desc->free_ts = qdf_get_system_timestamp();
		qdf_list_insert_back(&soc->reo_desc_freelist,
				     (qdf_list_node_t *)desc);
		dp_err_log("failed to send reo cmd CMD_UPDATE_RX_REO_QUEUE");
		DP_STATS_INC(soc, rx.err.reo_cmd_send_fail, 1);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_tid_delete_cb() - Callback to flush reo descriptor HW cache
 * after deleting the entries (ie., setting valid=0)
 *
 * @soc: DP SOC handle
 * @cb_ctxt: Callback context
 * @reo_status: REO command status
 */
void dp_rx_tid_delete_cb(struct dp_soc *soc, void *cb_ctxt,
			 union hal_reo_status *reo_status)
{
	struct reo_desc_list_node *freedesc =
		(struct reo_desc_list_node *)cb_ctxt;
	uint32_t list_size;
	struct reo_desc_list_node *desc;
	unsigned long curr_ts = qdf_get_system_timestamp();
	uint32_t desc_size, tot_desc_size;
	struct hal_reo_cmd_params params;
	bool flush_failure = false;

	DP_RX_REO_QDESC_UPDATE_EVT(freedesc);

	if (reo_status->rx_queue_status.header.status == HAL_REO_CMD_DRAIN) {
		qdf_mem_zero(reo_status, sizeof(*reo_status));
		reo_status->fl_cache_status.header.status = HAL_REO_CMD_DRAIN;
		dp_reo_desc_free(soc, (void *)freedesc, reo_status);
		DP_STATS_INC(soc, rx.err.reo_cmd_send_drain, 1);
		return;
	} else if (reo_status->rx_queue_status.header.status !=
		HAL_REO_CMD_SUCCESS) {
		/* Should not happen normally. Just print error for now */
		dp_info_rl("Rx tid HW desc deletion failed(%d): tid %d",
			   reo_status->rx_queue_status.header.status,
			   freedesc->rx_tid.tid);
	}

	dp_peer_info("%pK: rx_tid: %d status: %d",
		     soc, freedesc->rx_tid.tid,
		     reo_status->rx_queue_status.header.status);

	qdf_spin_lock_bh(&soc->reo_desc_freelist_lock);
	freedesc->free_ts = curr_ts;
	qdf_list_insert_back_size(&soc->reo_desc_freelist,
		(qdf_list_node_t *)freedesc, &list_size);

	/* MCL path add the desc back to reo_desc_freelist when REO FLUSH
	 * failed. it may cause the number of REO queue pending  in free
	 * list is even larger than REO_CMD_RING max size and lead REO CMD
	 * flood then cause REO HW in an unexpected condition. So it's
	 * needed to limit the number REO cmds in a batch operation.
	 */
	dp_reo_limit_clean_batch_sz(&list_size);

	while ((qdf_list_peek_front(&soc->reo_desc_freelist,
		(qdf_list_node_t **)&desc) == QDF_STATUS_SUCCESS) &&
		((list_size >= REO_DESC_FREELIST_SIZE) ||
		(curr_ts > (desc->free_ts + REO_DESC_FREE_DEFER_MS)) ||
		(desc->resend_update_reo_cmd && list_size))) {
		struct dp_rx_tid *rx_tid;

		qdf_list_remove_front(&soc->reo_desc_freelist,
				(qdf_list_node_t **)&desc);
		list_size--;
		rx_tid = &desc->rx_tid;

		/* First process descs with resend_update_reo_cmd set */
		if (desc->resend_update_reo_cmd) {
			if (dp_resend_update_reo_cmd(soc, desc, rx_tid) !=
			    QDF_STATUS_SUCCESS)
				break;
			else
				continue;
		}

		/* Flush and invalidate REO descriptor from HW cache: Base and
		 * extension descriptors should be flushed separately */
		if (desc->pending_ext_desc_size)
			tot_desc_size = desc->pending_ext_desc_size;
		else
			tot_desc_size = rx_tid->hw_qdesc_alloc_size;
		/* Get base descriptor size by passing non-qos TID */
		desc_size = hal_get_reo_qdesc_size(soc->hal_soc, 0,
						   DP_NON_QOS_TID);

		/* Flush reo extension descriptors */
		while ((tot_desc_size -= desc_size) > 0) {
			qdf_mem_zero(&params, sizeof(params));
			params.std.addr_lo =
				((uint64_t)(rx_tid->hw_qdesc_paddr) +
				tot_desc_size) & 0xffffffff;
			params.std.addr_hi =
				(uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;

			if (QDF_STATUS_SUCCESS != dp_reo_send_cmd(soc,
							CMD_FLUSH_CACHE,
							&params,
							NULL,
							NULL)) {
				dp_info_rl("fail to send CMD_CACHE_FLUSH:"
					   "tid %d desc %pK", rx_tid->tid,
					   (void *)(rx_tid->hw_qdesc_paddr));
				desc->pending_ext_desc_size = tot_desc_size +
								      desc_size;
				dp_reo_desc_clean_up(soc, desc, reo_status);
				flush_failure = true;
				break;
			}
		}

		if (flush_failure)
			break;
		else
			desc->pending_ext_desc_size = desc_size;

		/* Flush base descriptor */
		qdf_mem_zero(&params, sizeof(params));
		params.std.need_status = 1;
		params.std.addr_lo =
			(uint64_t)(rx_tid->hw_qdesc_paddr) & 0xffffffff;
		params.std.addr_hi = (uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;

		if (QDF_STATUS_SUCCESS != dp_reo_send_cmd(soc,
							  CMD_FLUSH_CACHE,
							  &params,
							  dp_reo_desc_free,
							  (void *)desc)) {
			union hal_reo_status reo_status;
			/*
			 * If dp_reo_send_cmd return failure, related TID queue desc
			 * should be unmapped. Also locally reo_desc, together with
			 * TID queue desc also need to be freed accordingly.
			 *
			 * Here invoke desc_free function directly to do clean up.
			 *
			 * In case of MCL path add the desc back to the free
			 * desc list and defer deletion.
			 */
			dp_info_rl("fail to send REO cmd to flush cache: tid %d",
				   rx_tid->tid);
			dp_reo_desc_clean_up(soc, desc, &reo_status);
			DP_STATS_INC(soc, rx.err.reo_cmd_send_fail, 1);
			break;
		}
	}
	qdf_spin_unlock_bh(&soc->reo_desc_freelist_lock);

	dp_reo_desc_defer_free(soc);
}

/*
 * dp_rx_tid_delete_wifi3() – Delete receive TID queue
 * @peer: Datapath peer handle
 * @tid: TID
 *
 * Return: 0 on success, error code on failure
 */
static int dp_rx_tid_delete_wifi3(struct dp_peer *peer, int tid)
{
	struct dp_rx_tid *rx_tid = &(peer->rx_tid[tid]);
	struct dp_soc *soc = peer->vdev->pdev->soc;
	struct hal_reo_cmd_params params;
	struct reo_desc_list_node *freedesc =
		qdf_mem_malloc(sizeof(*freedesc));

	if (!freedesc) {
		dp_peer_err("%pK: malloc failed for freedesc: tid %d",
			    soc, tid);
		return -ENOMEM;
	}

	freedesc->rx_tid = *rx_tid;
	freedesc->resend_update_reo_cmd = false;

	qdf_mem_zero(&params, sizeof(params));

	DP_RX_REO_QDESC_GET_MAC(freedesc, peer);

	params.std.need_status = 1;
	params.std.addr_lo = rx_tid->hw_qdesc_paddr & 0xffffffff;
	params.std.addr_hi = (uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;
	params.u.upd_queue_params.update_vld = 1;
	params.u.upd_queue_params.vld = 0;

	if (dp_reo_send_cmd(soc, CMD_UPDATE_RX_REO_QUEUE, &params,
			    dp_rx_tid_delete_cb, (void *)freedesc)
		!= QDF_STATUS_SUCCESS) {
		/* Defer the clean up to the call back context */
		qdf_spin_lock_bh(&soc->reo_desc_freelist_lock);
		freedesc->free_ts = qdf_get_system_timestamp();
		freedesc->resend_update_reo_cmd = true;
		qdf_list_insert_front(&soc->reo_desc_freelist,
				      (qdf_list_node_t *)freedesc);
		DP_STATS_INC(soc, rx.err.reo_cmd_send_fail, 1);
		qdf_spin_unlock_bh(&soc->reo_desc_freelist_lock);
		dp_info("Failed to send CMD_UPDATE_RX_REO_QUEUE");
	}

	rx_tid->hw_qdesc_vaddr_unaligned = NULL;
	rx_tid->hw_qdesc_alloc_size = 0;
	rx_tid->hw_qdesc_paddr = 0;

	return 0;
}

#ifdef DP_LFR
static void dp_peer_setup_remaining_tids(struct dp_peer *peer)
{
	int tid;

	for (tid = 1; tid < DP_MAX_TIDS-1; tid++) {
		dp_rx_tid_setup_wifi3(peer, tid, 1, 0);
		dp_peer_debug("Setting up TID %d for peer %pK peer->local_id %d",
			      tid, peer, peer->local_id);
	}
}
#else
static void dp_peer_setup_remaining_tids(struct dp_peer *peer) {};
#endif

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * dp_peer_rx_tids_init() - initialize each tids in peer
 * @peer: peer pointer
 *
 * Return: None
 */
static void dp_peer_rx_tids_init(struct dp_peer *peer)
{
	int tid;
	struct dp_rx_tid *rx_tid;

	/* if not first assoc link peer or MLD peer,
	 * not to initialize rx_tids again.
	 */
	if ((IS_MLO_DP_LINK_PEER(peer) && !peer->first_link) ||
	    IS_MLO_DP_MLD_PEER(peer))
		return;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		rx_tid = &peer->rx_tid[tid];
		rx_tid->array = &rx_tid->base;
		rx_tid->base.head = NULL;
		rx_tid->base.tail = NULL;
		rx_tid->tid = tid;
		rx_tid->defrag_timeout_ms = 0;
		rx_tid->ba_win_size = 0;
		rx_tid->ba_status = DP_RX_BA_INACTIVE;

		rx_tid->defrag_waitlist_elem.tqe_next = NULL;
		rx_tid->defrag_waitlist_elem.tqe_prev = NULL;
		rx_tid->defrag_peer =
			IS_MLO_DP_LINK_PEER(peer) ? peer->mld_peer : peer;
	}
}
#else
static void dp_peer_rx_tids_init(struct dp_peer *peer)
{
	int tid;
	struct dp_rx_tid *rx_tid;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		rx_tid = &peer->rx_tid[tid];
		rx_tid->array = &rx_tid->base;
		rx_tid->base.head = NULL;
		rx_tid->base.tail = NULL;
		rx_tid->tid = tid;
		rx_tid->defrag_timeout_ms = 0;
		rx_tid->ba_win_size = 0;
		rx_tid->ba_status = DP_RX_BA_INACTIVE;

		rx_tid->defrag_waitlist_elem.tqe_next = NULL;
		rx_tid->defrag_waitlist_elem.tqe_prev = NULL;
		rx_tid->defrag_peer = peer;
	}
}
#endif

/*
 * dp_peer_rx_init() – Initialize receive TID state
 * @pdev: Datapath pdev
 * @peer: Datapath peer
 *
 */
void dp_peer_rx_init(struct dp_pdev *pdev, struct dp_peer *peer)
{
	dp_peer_rx_tids_init(peer);

	peer->active_ba_session_cnt = 0;
	peer->hw_buffer_size = 0;
	peer->kill_256_sessions = 0;

	/* Setup default (non-qos) rx tid queue */
	dp_rx_tid_setup_wifi3(peer, DP_NON_QOS_TID, 1, 0);

	/* Setup rx tid queue for TID 0.
	 * Other queues will be setup on receiving first packet, which will cause
	 * NULL REO queue error
	 */
	dp_rx_tid_setup_wifi3(peer, 0, 1, 0);

	/*
	 * Setup the rest of TID's to handle LFR
	 */
	dp_peer_setup_remaining_tids(peer);

	/*
	 * Set security defaults: no PN check, no security. The target may
	 * send a HTT SEC_IND message to overwrite these defaults.
	 */
	peer->security[dp_sec_ucast].sec_type =
		peer->security[dp_sec_mcast].sec_type = cdp_sec_type_none;
}

/*
 * dp_peer_rx_cleanup() – Cleanup receive TID state
 * @vdev: Datapath vdev
 * @peer: Datapath peer
 *
 */
void dp_peer_rx_cleanup(struct dp_vdev *vdev, struct dp_peer *peer)
{
	int tid;
	uint32_t tid_delete_mask = 0;

	dp_info("Remove tids for peer: %pK", peer);
	if (IS_MLO_DP_LINK_PEER(peer))
		return;

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];

		qdf_spin_lock_bh(&rx_tid->tid_lock);
		if (!peer->bss_peer || peer->vdev->opmode == wlan_op_mode_sta) {
			/* Cleanup defrag related resource */
			dp_rx_defrag_waitlist_remove(peer, tid);
			dp_rx_reorder_flush_frag(peer, tid);
		}

		if (peer->rx_tid[tid].hw_qdesc_vaddr_unaligned) {
			dp_rx_tid_delete_wifi3(peer, tid);

			tid_delete_mask |= (1 << tid);
		}
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
	}
#ifdef notyet /* See if FW can remove queues as part of peer cleanup */
	if (soc->ol_ops->peer_rx_reorder_queue_remove) {
		soc->ol_ops->peer_rx_reorder_queue_remove(soc->ctrl_psoc,
			peer->vdev->pdev->pdev_id,
			peer->vdev->vdev_id, peer->mac_addr.raw,
			tid_delete_mask);
	}
#endif
}

/*
 * dp_peer_cleanup() – Cleanup peer information
 * @vdev: Datapath vdev
 * @peer: Datapath peer
 *
 */
void dp_peer_cleanup(struct dp_vdev *vdev, struct dp_peer *peer)
{
	enum wlan_op_mode vdev_opmode;
	uint8_t vdev_mac_addr[QDF_MAC_ADDR_SIZE];
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	/* save vdev related member in case vdev freed */
	vdev_opmode = vdev->opmode;

	dp_monitor_peer_tx_cleanup(vdev, peer);

	if (vdev_opmode != wlan_op_mode_monitor)
	/* cleanup the Rx reorder queues for this peer */
		dp_peer_rx_cleanup(vdev, peer);

	dp_peer_rx_tids_destroy(peer);

	if (IS_MLO_DP_LINK_PEER(peer))
		dp_link_peer_del_mld_peer(peer);
	if (IS_MLO_DP_MLD_PEER(peer))
		dp_mld_peer_deinit_link_peers_info(peer);

	qdf_mem_copy(vdev_mac_addr, vdev->mac_addr.raw,
		     QDF_MAC_ADDR_SIZE);

	if (soc->cdp_soc.ol_ops->peer_unref_delete)
		soc->cdp_soc.ol_ops->peer_unref_delete(
				soc->ctrl_psoc,
				vdev->pdev->pdev_id,
				peer->mac_addr.raw, vdev_mac_addr,
				vdev_opmode);
}

/* dp_teardown_256_ba_session() - Teardown sessions using 256
 *                                window size when a request with
 *                                64 window size is received.
 *                                This is done as a WAR since HW can
 *                                have only one setting per peer (64 or 256).
 *                                For HKv2, we use per tid buffersize setting
 *                                for 0 to per_tid_basize_max_tid. For tid
 *                                more than per_tid_basize_max_tid we use HKv1
 *                                method.
 * @peer: Datapath peer
 *
 * Return: void
 */
static void dp_teardown_256_ba_sessions(struct dp_peer *peer)
{
	uint8_t delba_rcode = 0;
	int tid;
	struct dp_rx_tid *rx_tid = NULL;

	tid = peer->vdev->pdev->soc->per_tid_basize_max_tid;
	for (; tid < DP_MAX_TIDS; tid++) {
		rx_tid = &peer->rx_tid[tid];
		qdf_spin_lock_bh(&rx_tid->tid_lock);

		if (rx_tid->ba_win_size <= 64) {
			qdf_spin_unlock_bh(&rx_tid->tid_lock);
			continue;
		} else {
			if (rx_tid->ba_status == DP_RX_BA_ACTIVE ||
			    rx_tid->ba_status == DP_RX_BA_IN_PROGRESS) {
				/* send delba */
				if (!rx_tid->delba_tx_status) {
					rx_tid->delba_tx_retry++;
					rx_tid->delba_tx_status = 1;
					rx_tid->delba_rcode =
					IEEE80211_REASON_QOS_SETUP_REQUIRED;
					delba_rcode = rx_tid->delba_rcode;

					qdf_spin_unlock_bh(&rx_tid->tid_lock);
					if (peer->vdev->pdev->soc->cdp_soc.ol_ops->send_delba)
						peer->vdev->pdev->soc->cdp_soc.ol_ops->send_delba(
							peer->vdev->pdev->soc->ctrl_psoc,
							peer->vdev->vdev_id,
							peer->mac_addr.raw,
							tid, delba_rcode,
							CDP_DELBA_REASON_NONE);
				} else {
					qdf_spin_unlock_bh(&rx_tid->tid_lock);
				}
			} else {
				qdf_spin_unlock_bh(&rx_tid->tid_lock);
			}
		}
	}
}

/*
* dp_rx_addba_resp_tx_completion_wifi3() – Update Rx Tid State
*
* @soc: Datapath soc handle
* @peer_mac: Datapath peer mac address
* @vdev_id: id of atapath vdev
* @tid: TID number
* @status: tx completion status
* Return: 0 on success, error code on failure
*/
int dp_addba_resp_tx_completion_wifi3(struct cdp_soc_t *cdp_soc,
				      uint8_t *peer_mac,
				      uint16_t vdev_id,
				      uint8_t tid, int status)
{
	struct dp_peer *peer = dp_peer_get_tgt_peer_hash_find(
					(struct dp_soc *)cdp_soc,
					peer_mac, 0, vdev_id,
					DP_MOD_ID_CDP);
	struct dp_rx_tid *rx_tid = NULL;

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", cdp_soc);
		goto fail;
	}
	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	if (status) {
		rx_tid->num_addba_rsp_failed++;
		dp_rx_tid_update_wifi3(peer, tid, 1,
				       IEEE80211_SEQ_MAX, false);
		rx_tid->ba_status = DP_RX_BA_INACTIVE;
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		dp_err("RxTid- %d addba rsp tx completion failed", tid);

		goto success;
	}

	rx_tid->num_addba_rsp_success++;
	if (rx_tid->ba_status == DP_RX_BA_INACTIVE) {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		dp_peer_err("%pK: Rx Tid- %d hw qdesc is not in IN_PROGRESS",
			    cdp_soc, tid);
		goto fail;
	}

	if (!qdf_atomic_read(&peer->is_default_route_set)) {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		dp_peer_debug("%pK: default route is not set for peer: " QDF_MAC_ADDR_FMT,
			      cdp_soc, QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		goto fail;
	}

	if (dp_rx_tid_update_wifi3(peer, tid,
				   rx_tid->ba_win_size,
				   rx_tid->startseqnum,
				   false)) {
		dp_err("Failed update REO SSN");
	}

	dp_info("tid %u window_size %u start_seq_num %u",
		tid, rx_tid->ba_win_size,
		rx_tid->startseqnum);

	/* First Session */
	if (peer->active_ba_session_cnt == 0) {
		if (rx_tid->ba_win_size > 64 && rx_tid->ba_win_size <= 256)
			peer->hw_buffer_size = 256;
		else
			peer->hw_buffer_size = 64;
	}

	rx_tid->ba_status = DP_RX_BA_ACTIVE;

	peer->active_ba_session_cnt++;

	qdf_spin_unlock_bh(&rx_tid->tid_lock);

	/* Kill any session having 256 buffer size
	 * when 64 buffer size request is received.
	 * Also, latch on to 64 as new buffer size.
	 */
	if (peer->kill_256_sessions) {
		dp_teardown_256_ba_sessions(peer);
		peer->kill_256_sessions = 0;
	}

success:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;

fail:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_E_FAILURE;
}

/*
* dp_rx_addba_responsesetup_wifi3() – Process ADDBA request from peer
*
* @soc: Datapath soc handle
* @peer_mac: Datapath peer mac address
* @vdev_id: id of atapath vdev
* @tid: TID number
* @dialogtoken: output dialogtoken
* @statuscode: output dialogtoken
* @buffersize: Output BA window size
* @batimeout: Output BA timeout
*/
QDF_STATUS
dp_addba_responsesetup_wifi3(struct cdp_soc_t *cdp_soc, uint8_t *peer_mac,
			     uint16_t vdev_id, uint8_t tid,
			     uint8_t *dialogtoken, uint16_t *statuscode,
			     uint16_t *buffersize, uint16_t *batimeout)
{
	struct dp_rx_tid *rx_tid = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)cdp_soc,
						       peer_mac, 0, vdev_id,
						       DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", cdp_soc);
		return QDF_STATUS_E_FAILURE;
	}
	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	rx_tid->num_of_addba_resp++;
	/* setup ADDBA response parameters */
	*dialogtoken = rx_tid->dialogtoken;
	*statuscode = rx_tid->statuscode;
	*buffersize = rx_tid->ba_win_size;
	*batimeout  = 0;
	qdf_spin_unlock_bh(&rx_tid->tid_lock);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}

/* dp_check_ba_buffersize() - Check buffer size in request
 *                            and latch onto this size based on
 *                            size used in first active session.
 * @peer: Datapath peer
 * @tid: Tid
 * @buffersize: Block ack window size
 *
 * Return: void
 */
static void dp_check_ba_buffersize(struct dp_peer *peer,
				   uint16_t tid,
				   uint16_t buffersize)
{
	struct dp_rx_tid *rx_tid = NULL;

	rx_tid = &peer->rx_tid[tid];
	if (peer->vdev->pdev->soc->per_tid_basize_max_tid &&
	    tid < peer->vdev->pdev->soc->per_tid_basize_max_tid) {
		rx_tid->ba_win_size = buffersize;
		return;
	} else {
		if (peer->active_ba_session_cnt == 0) {
			rx_tid->ba_win_size = buffersize;
		} else {
			if (peer->hw_buffer_size == 64) {
				if (buffersize <= 64)
					rx_tid->ba_win_size = buffersize;
				else
					rx_tid->ba_win_size = peer->hw_buffer_size;
			} else if (peer->hw_buffer_size == 256) {
				if (buffersize > 64) {
					rx_tid->ba_win_size = buffersize;
				} else {
					rx_tid->ba_win_size = buffersize;
					peer->hw_buffer_size = 64;
					peer->kill_256_sessions = 1;
				}
			}
		}
	}
}

QDF_STATUS dp_rx_tid_update_ba_win_size(struct cdp_soc_t *cdp_soc,
					uint8_t *peer_mac, uint16_t vdev_id,
					uint8_t tid, uint16_t buffersize)
{
	struct dp_rx_tid *rx_tid = NULL;
	struct dp_peer *peer;

	peer = dp_peer_get_tgt_peer_hash_find((struct dp_soc *)cdp_soc,
					      peer_mac, 0, vdev_id,
					      DP_MOD_ID_CDP);
	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", cdp_soc);
		return QDF_STATUS_E_FAILURE;
	}

	rx_tid = &peer->rx_tid[tid];

	qdf_spin_lock_bh(&rx_tid->tid_lock);
	rx_tid->ba_win_size = buffersize;
	qdf_spin_unlock_bh(&rx_tid->tid_lock);

	dp_info("peer "QDF_MAC_ADDR_FMT", tid %d, update BA win size to %d",
		QDF_MAC_ADDR_REF(peer->mac_addr.raw), tid, buffersize);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

#define DP_RX_BA_SESSION_DISABLE  1

/*
 * dp_addba_requestprocess_wifi3() - Process ADDBA request from peer
 *
 * @soc: Datapath soc handle
 * @peer_mac: Datapath peer mac address
 * @vdev_id: id of atapath vdev
 * @dialogtoken: dialogtoken from ADDBA frame
 * @tid: TID number
 * @batimeout: BA timeout
 * @buffersize: BA window size
 * @startseqnum: Start seq. number received in BA sequence control
 *
 * Return: 0 on success, error code on failure
 */
int dp_addba_requestprocess_wifi3(struct cdp_soc_t *cdp_soc,
				  uint8_t *peer_mac,
				  uint16_t vdev_id,
				  uint8_t dialogtoken,
				  uint16_t tid, uint16_t batimeout,
				  uint16_t buffersize,
				  uint16_t startseqnum)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_rx_tid *rx_tid = NULL;
	struct dp_peer *peer;

	peer = dp_peer_get_tgt_peer_hash_find((struct dp_soc *)cdp_soc,
					      peer_mac,
					      0, vdev_id,
					      DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", cdp_soc);
		return QDF_STATUS_E_FAILURE;
	}
	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	rx_tid->num_of_addba_req++;
	if ((rx_tid->ba_status == DP_RX_BA_ACTIVE &&
	     rx_tid->hw_qdesc_vaddr_unaligned)) {
		dp_rx_tid_update_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX, false);
		rx_tid->ba_status = DP_RX_BA_INACTIVE;
		peer->active_ba_session_cnt--;
		dp_peer_debug("%pK: Rx Tid- %d hw qdesc is already setup",
			      cdp_soc, tid);
	}

	if (rx_tid->ba_status == DP_RX_BA_IN_PROGRESS) {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		status = QDF_STATUS_E_FAILURE;
		goto fail;
	}

	if (rx_tid->rx_ba_win_size_override == DP_RX_BA_SESSION_DISABLE) {
		dp_peer_info("%pK: disable BA session",
			     cdp_soc);

		buffersize = 1;
	} else if (rx_tid->rx_ba_win_size_override) {
		dp_peer_info("%pK: override BA win to %d", cdp_soc,
			     rx_tid->rx_ba_win_size_override);

		buffersize = rx_tid->rx_ba_win_size_override;
	} else {
		dp_peer_info("%pK: restore BA win %d based on addba req", cdp_soc,
			     buffersize);
	}

	dp_check_ba_buffersize(peer, tid, buffersize);

	if (dp_rx_tid_setup_wifi3(peer, tid,
	    rx_tid->ba_win_size, startseqnum)) {
		rx_tid->ba_status = DP_RX_BA_INACTIVE;
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		status = QDF_STATUS_E_FAILURE;
		goto fail;
	}
	rx_tid->ba_status = DP_RX_BA_IN_PROGRESS;

	rx_tid->dialogtoken = dialogtoken;
	rx_tid->startseqnum = startseqnum;

	if (rx_tid->userstatuscode != IEEE80211_STATUS_SUCCESS)
		rx_tid->statuscode = rx_tid->userstatuscode;
	else
		rx_tid->statuscode = IEEE80211_STATUS_SUCCESS;

	if (rx_tid->rx_ba_win_size_override == DP_RX_BA_SESSION_DISABLE)
		rx_tid->statuscode = IEEE80211_STATUS_REFUSED;

	qdf_spin_unlock_bh(&rx_tid->tid_lock);

fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}

/*
* dp_set_addba_response() – Set a user defined ADDBA response status code
*
* @soc: Datapath soc handle
* @peer_mac: Datapath peer mac address
* @vdev_id: id of atapath vdev
* @tid: TID number
* @statuscode: response status code to be set
*/
QDF_STATUS
dp_set_addba_response(struct cdp_soc_t *cdp_soc, uint8_t *peer_mac,
		      uint16_t vdev_id, uint8_t tid, uint16_t statuscode)
{
	struct dp_peer *peer = dp_peer_get_tgt_peer_hash_find(
					(struct dp_soc *)cdp_soc,
					peer_mac, 0, vdev_id,
					DP_MOD_ID_CDP);
	struct dp_rx_tid *rx_tid;

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", cdp_soc);
		return QDF_STATUS_E_FAILURE;
	}

	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	rx_tid->userstatuscode = statuscode;
	qdf_spin_unlock_bh(&rx_tid->tid_lock);
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

/*
* dp_rx_delba_process_wifi3() – Process DELBA from peer
* @soc: Datapath soc handle
* @peer_mac: Datapath peer mac address
* @vdev_id: id of atapath vdev
* @tid: TID number
* @reasoncode: Reason code received in DELBA frame
*
* Return: 0 on success, error code on failure
*/
int dp_delba_process_wifi3(struct cdp_soc_t *cdp_soc, uint8_t *peer_mac,
			   uint16_t vdev_id, int tid, uint16_t reasoncode)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_rx_tid *rx_tid;
	struct dp_peer *peer = dp_peer_get_tgt_peer_hash_find(
					(struct dp_soc *)cdp_soc,
					peer_mac, 0, vdev_id,
					DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", cdp_soc);
		return QDF_STATUS_E_FAILURE;
	}
	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	if (rx_tid->ba_status == DP_RX_BA_INACTIVE ||
	    rx_tid->ba_status == DP_RX_BA_IN_PROGRESS) {
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
		status = QDF_STATUS_E_FAILURE;
		goto fail;
	}
	/* TODO: See if we can delete the existing REO queue descriptor and
	 * replace with a new one without queue extenstion descript to save
	 * memory
	 */
	rx_tid->delba_rcode = reasoncode;
	rx_tid->num_of_delba_req++;
	dp_rx_tid_update_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX, false);

	rx_tid->ba_status = DP_RX_BA_INACTIVE;
	peer->active_ba_session_cnt--;
	qdf_spin_unlock_bh(&rx_tid->tid_lock);
fail:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}

/*
 * dp_rx_delba_tx_completion_wifi3() – Send Delba Request
 *
 * @soc: Datapath soc handle
 * @peer_mac: Datapath peer mac address
 * @vdev_id: id of atapath vdev
 * @tid: TID number
 * @status: tx completion status
 * Return: 0 on success, error code on failure
 */

int dp_delba_tx_completion_wifi3(struct cdp_soc_t *cdp_soc, uint8_t *peer_mac,
				 uint16_t vdev_id,
				 uint8_t tid, int status)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	struct dp_rx_tid *rx_tid = NULL;
	struct dp_peer *peer = dp_peer_get_tgt_peer_hash_find(
					(struct dp_soc *)cdp_soc,
					peer_mac, 0, vdev_id,
					DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!", cdp_soc);
		return QDF_STATUS_E_FAILURE;
	}
	rx_tid = &peer->rx_tid[tid];
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	if (status) {
		rx_tid->delba_tx_fail_cnt++;
		if (rx_tid->delba_tx_retry >= DP_MAX_DELBA_RETRY) {
			rx_tid->delba_tx_retry = 0;
			rx_tid->delba_tx_status = 0;
			qdf_spin_unlock_bh(&rx_tid->tid_lock);
		} else {
			rx_tid->delba_tx_retry++;
			rx_tid->delba_tx_status = 1;
			qdf_spin_unlock_bh(&rx_tid->tid_lock);
			if (peer->vdev->pdev->soc->cdp_soc.ol_ops->send_delba)
				peer->vdev->pdev->soc->cdp_soc.ol_ops->send_delba(
					peer->vdev->pdev->soc->ctrl_psoc,
					peer->vdev->vdev_id,
					peer->mac_addr.raw, tid,
					rx_tid->delba_rcode,
					CDP_DELBA_REASON_NONE);
		}
		goto end;
	} else {
		rx_tid->delba_tx_success_cnt++;
		rx_tid->delba_tx_retry = 0;
		rx_tid->delba_tx_status = 0;
	}
	if (rx_tid->ba_status == DP_RX_BA_ACTIVE) {
		dp_rx_tid_update_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX, false);
		rx_tid->ba_status = DP_RX_BA_INACTIVE;
		peer->active_ba_session_cnt--;
	}
	if (rx_tid->ba_status == DP_RX_BA_IN_PROGRESS) {
		dp_rx_tid_update_wifi3(peer, tid, 1, IEEE80211_SEQ_MAX, false);
		rx_tid->ba_status = DP_RX_BA_INACTIVE;
	}
	qdf_spin_unlock_bh(&rx_tid->tid_lock);

end:
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return ret;
}

/**
 * dp_set_pn_check_wifi3() - enable PN check in REO for security
 * @soc: Datapath soc handle
 * @peer_mac: Datapath peer mac address
 * @vdev_id: id of atapath vdev
 * @vdev: Datapath vdev
 * @pdev - data path device instance
 * @sec_type - security type
 * @rx_pn - Receive pn starting number
 *
 */

QDF_STATUS
dp_set_pn_check_wifi3(struct cdp_soc_t *soc, uint8_t vdev_id,
		      uint8_t *peer_mac, enum cdp_sec_type sec_type,
		      uint32_t *rx_pn)
{
	struct dp_pdev *pdev;
	int i;
	uint8_t pn_size;
	struct hal_reo_cmd_params params;
	struct dp_peer *peer = NULL;
	struct dp_vdev *vdev = NULL;

	peer = dp_peer_find_hash_find((struct dp_soc *)soc,
				      peer_mac, 0, vdev_id,
				      DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", soc);
		return QDF_STATUS_E_FAILURE;
	}

	vdev = peer->vdev;

	if (!vdev) {
		dp_peer_debug("%pK: VDEV is NULL!\n", soc);
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAILURE;
	}

	pdev = vdev->pdev;
	qdf_mem_zero(&params, sizeof(params));

	params.std.need_status = 1;
	params.u.upd_queue_params.update_pn_valid = 1;
	params.u.upd_queue_params.update_pn_size = 1;
	params.u.upd_queue_params.update_pn = 1;
	params.u.upd_queue_params.update_pn_check_needed = 1;
	params.u.upd_queue_params.update_svld = 1;
	params.u.upd_queue_params.svld = 0;

	switch (sec_type) {
	case cdp_sec_type_tkip_nomic:
	case cdp_sec_type_aes_ccmp:
	case cdp_sec_type_aes_ccmp_256:
	case cdp_sec_type_aes_gcmp:
	case cdp_sec_type_aes_gcmp_256:
		params.u.upd_queue_params.pn_check_needed = 1;
		params.u.upd_queue_params.pn_size = 48;
		pn_size = 48;
		break;
	case cdp_sec_type_wapi:
		params.u.upd_queue_params.pn_check_needed = 1;
		params.u.upd_queue_params.pn_size = 128;
		pn_size = 128;
		if (vdev->opmode == wlan_op_mode_ap) {
			params.u.upd_queue_params.pn_even = 1;
			params.u.upd_queue_params.update_pn_even = 1;
		} else {
			params.u.upd_queue_params.pn_uneven = 1;
			params.u.upd_queue_params.update_pn_uneven = 1;
		}
		break;
	default:
		params.u.upd_queue_params.pn_check_needed = 0;
		pn_size = 0;
		break;
	}


	for (i = 0; i < DP_MAX_TIDS; i++) {
		struct dp_rx_tid *rx_tid = &peer->rx_tid[i];
		qdf_spin_lock_bh(&rx_tid->tid_lock);
		if (rx_tid->hw_qdesc_vaddr_unaligned) {
			params.std.addr_lo =
				rx_tid->hw_qdesc_paddr & 0xffffffff;
			params.std.addr_hi =
				(uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;

			if (pn_size) {
				dp_peer_info("%pK: PN set for TID:%d pn:%x:%x:%x:%x",
					     soc, i, rx_pn[3], rx_pn[2],
					     rx_pn[1], rx_pn[0]);
				params.u.upd_queue_params.update_pn_valid = 1;
				params.u.upd_queue_params.pn_31_0 = rx_pn[0];
				params.u.upd_queue_params.pn_63_32 = rx_pn[1];
				params.u.upd_queue_params.pn_95_64 = rx_pn[2];
				params.u.upd_queue_params.pn_127_96 = rx_pn[3];
			}
			rx_tid->pn_size = pn_size;
			if (dp_reo_send_cmd(cdp_soc_t_to_dp_soc(soc),
					    CMD_UPDATE_RX_REO_QUEUE,
					    &params, dp_rx_tid_update_cb,
					    rx_tid)) {
				dp_err_log("fail to send CMD_UPDATE_RX_REO_QUEUE"
					   "tid %d desc %pK", rx_tid->tid,
					   (void *)(rx_tid->hw_qdesc_paddr));
				DP_STATS_INC(cdp_soc_t_to_dp_soc(soc),
					     rx.err.reo_cmd_send_fail, 1);
			}
		} else {
			dp_peer_info("%pK: PN Check not setup for TID :%d ", soc, i);
		}
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}


/**
 * dp_set_key_sec_type_wifi3() - set security mode of key
 * @soc: Datapath soc handle
 * @peer_mac: Datapath peer mac address
 * @vdev_id: id of atapath vdev
 * @vdev: Datapath vdev
 * @pdev - data path device instance
 * @sec_type - security type
 * #is_unicast - key type
 *
 */

QDF_STATUS
dp_set_key_sec_type_wifi3(struct cdp_soc_t *soc, uint8_t vdev_id,
			  uint8_t *peer_mac, enum cdp_sec_type sec_type,
			  bool is_unicast)
{
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						       peer_mac, 0, vdev_id,
						       DP_MOD_ID_CDP);
	int sec_index;

	if (!peer) {
		dp_peer_debug("%pK: Peer is NULL!\n", soc);
		return QDF_STATUS_E_FAILURE;
	}

	dp_peer_info("%pK: key sec spec for peer %pK " QDF_MAC_ADDR_FMT ": %s key of type %d",
		     soc, peer, QDF_MAC_ADDR_REF(peer->mac_addr.raw),
		     is_unicast ? "ucast" : "mcast", sec_type);

	sec_index = is_unicast ? dp_sec_ucast : dp_sec_mcast;
	peer->security[sec_index].sec_type = sec_type;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

void
dp_rx_sec_ind_handler(struct dp_soc *soc, uint16_t peer_id,
		      enum cdp_sec_type sec_type, int is_unicast,
		      u_int32_t *michael_key,
		      u_int32_t *rx_pn)
{
	struct dp_peer *peer;
	int sec_index;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_HTT);
	if (!peer) {
		dp_peer_err("Couldn't find peer from ID %d - skipping security inits",
			    peer_id);
		return;
	}
	dp_peer_info("%pK: sec spec for peer %pK " QDF_MAC_ADDR_FMT ": %s key of type %d",
		     soc, peer, QDF_MAC_ADDR_REF(peer->mac_addr.raw),
			  is_unicast ? "ucast" : "mcast", sec_type);
	sec_index = is_unicast ? dp_sec_ucast : dp_sec_mcast;
	peer->security[sec_index].sec_type = sec_type;
#ifdef notyet /* TODO: See if this is required for defrag support */
	/* michael key only valid for TKIP, but for simplicity,
	 * copy it anyway
	 */
	qdf_mem_copy(
		&peer->security[sec_index].michael_key[0],
		michael_key,
		sizeof(peer->security[sec_index].michael_key));
#ifdef BIG_ENDIAN_HOST
	OL_IF_SWAPBO(peer->security[sec_index].michael_key[0],
				 sizeof(peer->security[sec_index].michael_key));
#endif /* BIG_ENDIAN_HOST */
#endif

#ifdef notyet /* TODO: Check if this is required for wifi3.0 */
	if (sec_type != cdp_sec_type_wapi) {
		qdf_mem_zero(peer->tids_last_pn_valid, _EXT_TIDS);
	} else {
		for (i = 0; i < DP_MAX_TIDS; i++) {
			/*
			 * Setting PN valid bit for WAPI sec_type,
			 * since WAPI PN has to be started with predefined value
			 */
			peer->tids_last_pn_valid[i] = 1;
			qdf_mem_copy(
				(u_int8_t *) &peer->tids_last_pn[i],
				(u_int8_t *) rx_pn, sizeof(union htt_rx_pn_t));
			peer->tids_last_pn[i].pn128[1] =
				qdf_cpu_to_le64(peer->tids_last_pn[i].pn128[1]);
			peer->tids_last_pn[i].pn128[0] =
				qdf_cpu_to_le64(peer->tids_last_pn[i].pn128[0]);
		}
	}
#endif
	/* TODO: Update HW TID queue with PN check parameters (pn type for
	 * all security types and last pn for WAPI) once REO command API
	 * is available
	 */

	dp_peer_unref_delete(peer, DP_MOD_ID_HTT);
}

#ifdef QCA_PEER_EXT_STATS
/*
 * dp_peer_ext_stats_ctx_alloc() - Allocate peer ext
 *                                 stats content
 * @soc: DP SoC context
 * @peer: DP peer context
 *
 * Allocate the peer extended stats context
 *
 * Return: QDF_STATUS_SUCCESS if allocation is
 *	   successful
 */
QDF_STATUS dp_peer_ext_stats_ctx_alloc(struct dp_soc *soc,
				       struct dp_peer *peer)
{
	uint8_t tid, ctx_id;

	if (!soc || !peer) {
		dp_warn("Null soc%pK or peer%pK", soc, peer);
		return QDF_STATUS_E_INVAL;
	}

	if (!wlan_cfg_is_peer_ext_stats_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	/*
	 * Allocate memory for peer extended stats.
	 */
	peer->pext_stats = qdf_mem_malloc(sizeof(struct cdp_peer_ext_stats));
	if (!peer->pext_stats) {
		dp_err("Peer extended stats obj alloc failed!!");
		return QDF_STATUS_E_NOMEM;
	}

	for (tid = 0; tid < CDP_MAX_DATA_TIDS; tid++) {
		for (ctx_id = 0; ctx_id < CDP_MAX_TXRX_CTX; ctx_id++) {
			struct cdp_delay_tx_stats *tx_delay =
			&peer->pext_stats->delay_stats[tid][ctx_id].tx_delay;
			struct cdp_delay_rx_stats *rx_delay =
			&peer->pext_stats->delay_stats[tid][ctx_id].rx_delay;

			dp_hist_init(&tx_delay->tx_swq_delay,
				     CDP_HIST_TYPE_SW_ENQEUE_DELAY);
			dp_hist_init(&tx_delay->hwtx_delay,
				     CDP_HIST_TYPE_HW_COMP_DELAY);
			dp_hist_init(&rx_delay->to_stack_delay,
				     CDP_HIST_TYPE_REAP_STACK);
		}
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_peer_ext_stats_ctx_dealloc() - Dealloc the peer context
 * @peer: DP peer context
 *
 * Free the peer extended stats context
 *
 * Return: Void
 */
void dp_peer_ext_stats_ctx_dealloc(struct dp_soc *soc, struct dp_peer *peer)
{
	if (!peer) {
		dp_warn("peer_ext dealloc failed due to NULL peer object");
		return;
	}

	if (!wlan_cfg_is_peer_ext_stats_enabled(soc->wlan_cfg_ctx))
		return;

	if (!peer->pext_stats)
		return;

	qdf_mem_free(peer->pext_stats);
	peer->pext_stats = NULL;
}
#endif

QDF_STATUS
dp_rx_delba_ind_handler(void *soc_handle, uint16_t peer_id,
			uint8_t tid, uint16_t win_sz)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_peer *peer;
	struct dp_rx_tid *rx_tid;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	peer = dp_peer_get_ref_by_id(soc, peer_id, DP_MOD_ID_HTT);

	if (!peer) {
		dp_peer_err("%pK: Couldn't find peer from ID %d",
			    soc, peer_id);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_assert_always(tid < DP_MAX_TIDS);

	rx_tid = &peer->rx_tid[tid];

	if (rx_tid->hw_qdesc_vaddr_unaligned) {
		if (!rx_tid->delba_tx_status) {
			dp_peer_info("%pK: PEER_ID: %d TID: %d, BA win: %d ",
				     soc, peer_id, tid, win_sz);

			qdf_spin_lock_bh(&rx_tid->tid_lock);

			rx_tid->delba_tx_status = 1;

			rx_tid->rx_ba_win_size_override =
			    qdf_min((uint16_t)63, win_sz);

			rx_tid->delba_rcode =
			    IEEE80211_REASON_QOS_SETUP_REQUIRED;

			qdf_spin_unlock_bh(&rx_tid->tid_lock);

			if (soc->cdp_soc.ol_ops->send_delba)
				soc->cdp_soc.ol_ops->send_delba(
					peer->vdev->pdev->soc->ctrl_psoc,
					peer->vdev->vdev_id,
					peer->mac_addr.raw,
					tid,
					rx_tid->delba_rcode,
					CDP_DELBA_REASON_NONE);
		}
	} else {
		dp_peer_err("%pK: BA session is not setup for TID:%d ", soc, tid);
		status = QDF_STATUS_E_FAILURE;
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_HTT);

	return status;
}

#ifdef DP_PEER_EXTENDED_API
/**
 * dp_peer_set_bw() - Set bandwidth and mpdu retry count threshold for peer
 * @soc: DP soc handle
 * @peer: DP peer handle
 * @set_bw: enum of bandwidth to be set for this peer connection
 *
 * Return: None
 */
static void dp_peer_set_bw(struct dp_soc *soc, struct dp_peer *peer,
			   enum cdp_peer_bw set_bw)
{
	peer->bw = set_bw;

	switch (set_bw) {
	case CDP_160_MHZ:
	case CDP_320_MHZ:
		peer->mpdu_retry_threshold =
				soc->wlan_cfg_ctx->mpdu_retry_threshold_2;
		break;
	case CDP_20_MHZ:
	case CDP_40_MHZ:
	case CDP_80_MHZ:
	default:
		peer->mpdu_retry_threshold =
				soc->wlan_cfg_ctx->mpdu_retry_threshold_1;
		break;
	}

	dp_info("Peer id: %u: BW: %u, mpdu retry threshold: %u",
		peer->peer_id, peer->bw, peer->mpdu_retry_threshold);
}

#ifdef WLAN_FEATURE_11BE_MLO
QDF_STATUS dp_register_peer(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			    struct ol_txrx_desc_type *sta_desc)
{
	struct dp_peer *peer;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	peer = dp_peer_find_hash_find(soc, sta_desc->peer_addr.bytes,
				      0, DP_VDEV_ALL, DP_MOD_ID_CDP);

	if (!peer)
		return QDF_STATUS_E_FAULT;

	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_CONN;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	dp_peer_set_bw(soc, peer, sta_desc->bw);

	/* For MLO connection, no RX packet to link peer */
	if (!IS_MLO_DP_LINK_PEER(peer))
		dp_rx_flush_rx_cached(peer, false);

	if (IS_MLO_DP_LINK_PEER(peer) && peer->first_link) {
		dp_peer_info("register for mld peer" QDF_MAC_ADDR_FMT,
			     QDF_MAC_ADDR_REF(peer->mld_peer->mac_addr.raw));
		qdf_spin_lock_bh(&peer->mld_peer->peer_info_lock);
		peer->mld_peer->state = peer->state;
		qdf_spin_unlock_bh(&peer->mld_peer->peer_info_lock);
		dp_rx_flush_rx_cached(peer->mld_peer, false);
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_peer_state_update(struct cdp_soc_t *soc_hdl, uint8_t *peer_mac,
				enum ol_txrx_peer_state state)
{
	struct dp_peer *peer;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	peer =  dp_peer_find_hash_find(soc, peer_mac, 0, DP_VDEV_ALL,
				       DP_MOD_ID_CDP);
	if (!peer) {
		dp_peer_err("%pK: Failed to find peer[" QDF_MAC_ADDR_FMT "]",
			    soc, QDF_MAC_ADDR_REF(peer_mac));
		return QDF_STATUS_E_FAILURE;
	}
	peer->state = state;
	peer->authorize = (state == OL_TXRX_PEER_STATE_AUTH) ? 1 : 0;

	dp_peer_info("peer" QDF_MAC_ADDR_FMT "state %d",
		     QDF_MAC_ADDR_REF(peer->mac_addr.raw),
		     peer->state);

	if (IS_MLO_DP_LINK_PEER(peer) && peer->first_link) {
		peer->mld_peer->state = peer->state;
		peer->mld_peer->authorize = peer->authorize;
		dp_peer_info("mld peer" QDF_MAC_ADDR_FMT "state %d",
			     QDF_MAC_ADDR_REF(peer->mld_peer->mac_addr.raw),
			     peer->mld_peer->state);
	}

	/* ref_cnt is incremented inside dp_peer_find_hash_find().
	 * Decrement it here.
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS dp_register_peer(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			    struct ol_txrx_desc_type *sta_desc)
{
	struct dp_peer *peer;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	peer = dp_peer_find_hash_find(soc, sta_desc->peer_addr.bytes,
				      0, DP_VDEV_ALL, DP_MOD_ID_CDP);

	if (!peer)
		return QDF_STATUS_E_FAULT;

	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_CONN;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	dp_peer_set_bw(soc, peer, sta_desc->bw);

	dp_rx_flush_rx_cached(peer, false);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_peer_state_update(struct cdp_soc_t *soc_hdl, uint8_t *peer_mac,
				enum ol_txrx_peer_state state)
{
	struct dp_peer *peer;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	peer =  dp_peer_find_hash_find(soc, peer_mac, 0, DP_VDEV_ALL,
				       DP_MOD_ID_CDP);
	if (!peer) {
		dp_peer_err("%pK: Failed to find peer for: [" QDF_MAC_ADDR_FMT "]",
			    soc, QDF_MAC_ADDR_REF(peer_mac));
		return QDF_STATUS_E_FAILURE;
	}
	peer->state = state;
	peer->authorize = (state == OL_TXRX_PEER_STATE_AUTH) ? 1 : 0;

	dp_info("peer %pK state %d", peer, peer->state);
	/* ref_cnt is incremented inside dp_peer_find_hash_find().
	 * Decrement it here.
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
dp_clear_peer(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
	      struct qdf_mac_addr peer_addr)
{
	struct dp_peer *peer;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);

	peer = dp_peer_find_hash_find(soc, peer_addr.bytes,
				      0, DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (!peer || !peer->valid)
		return QDF_STATUS_E_FAULT;

	dp_clear_peer_internal(soc, peer);
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_get_vdevid(struct cdp_soc_t *soc_hdl, uint8_t *peer_mac,
			 uint8_t *vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer =
		dp_peer_find_hash_find(soc, peer_mac, 0, DP_VDEV_ALL,
				       DP_MOD_ID_CDP);

	if (!peer)
		return QDF_STATUS_E_FAILURE;

	dp_info("peer %pK vdev %pK vdev id %d",
		peer, peer->vdev, peer->vdev->vdev_id);
	*vdev_id = peer->vdev->vdev_id;
	/* ref_cnt is incremented inside dp_peer_find_hash_find().
	 * Decrement it here.
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

struct cdp_vdev *
dp_get_vdev_by_peer_addr(struct cdp_pdev *pdev_handle,
			 struct qdf_mac_addr peer_addr)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_peer *peer = NULL;
	struct cdp_vdev *vdev = NULL;

	if (!pdev) {
		dp_peer_info("PDEV not found for peer_addr: " QDF_MAC_ADDR_FMT,
			     QDF_MAC_ADDR_REF(peer_addr.bytes));
		return NULL;
	}

	peer = dp_peer_find_hash_find(pdev->soc, peer_addr.bytes, 0,
				      DP_VDEV_ALL, DP_MOD_ID_CDP);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			  "PDEV not found for peer_addr: "QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(peer_addr.bytes));
		return NULL;
	}

	vdev = (struct cdp_vdev *)peer->vdev;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
	return vdev;
}

/**
 * dp_get_vdev_for_peer() - Get virtual interface instance which peer belongs
 * @peer - peer instance
 *
 * Get virtual interface instance which peer belongs
 *
 * Return: virtual interface instance pointer
 *         NULL in case cannot find
 */
struct cdp_vdev *dp_get_vdev_for_peer(void *peer_handle)
{
	struct dp_peer *peer = peer_handle;

	DP_TRACE(DEBUG, "peer %pK vdev %pK", peer, peer->vdev);
	return (struct cdp_vdev *)peer->vdev;
}

/**
 * dp_peer_get_peer_mac_addr() - Get peer mac address
 * @peer - peer instance
 *
 * Get peer mac address
 *
 * Return: peer mac address pointer
 *         NULL in case cannot find
 */
uint8_t *dp_peer_get_peer_mac_addr(void *peer_handle)
{
	struct dp_peer *peer = peer_handle;
	uint8_t *mac;

	mac = peer->mac_addr.raw;
	dp_info("peer %pK mac 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
		peer, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return peer->mac_addr.raw;
}

int dp_get_peer_state(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
		      uint8_t *peer_mac)
{
	enum ol_txrx_peer_state peer_state;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer =  dp_peer_find_hash_find(soc, peer_mac, 0,
						       vdev_id, DP_MOD_ID_CDP);

	if (!peer)
		return OL_TXRX_PEER_STATE_INVALID;

	DP_TRACE(DEBUG, "peer %pK stats %d", peer, peer->state);
	peer_state = peer->state;
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return peer_state;
}

/**
 * dp_local_peer_id_pool_init() - local peer id pool alloc for physical device
 * @pdev - data path device instance
 *
 * local peer id pool alloc for physical device
 *
 * Return: none
 */
void dp_local_peer_id_pool_init(struct dp_pdev *pdev)
{
	int i;

	/* point the freelist to the first ID */
	pdev->local_peer_ids.freelist = 0;

	/* link each ID to the next one */
	for (i = 0; i < OL_TXRX_NUM_LOCAL_PEER_IDS; i++) {
		pdev->local_peer_ids.pool[i] = i + 1;
		pdev->local_peer_ids.map[i] = NULL;
	}

	/* link the last ID to itself, to mark the end of the list */
	i = OL_TXRX_NUM_LOCAL_PEER_IDS;
	pdev->local_peer_ids.pool[i] = i;

	qdf_spinlock_create(&pdev->local_peer_ids.lock);
	DP_TRACE(INFO, "Peer pool init");
}

/**
 * dp_local_peer_id_alloc() - allocate local peer id
 * @pdev - data path device instance
 * @peer - new peer instance
 *
 * allocate local peer id
 *
 * Return: none
 */
void dp_local_peer_id_alloc(struct dp_pdev *pdev, struct dp_peer *peer)
{
	int i;

	qdf_spin_lock_bh(&pdev->local_peer_ids.lock);
	i = pdev->local_peer_ids.freelist;
	if (pdev->local_peer_ids.pool[i] == i) {
		/* the list is empty, except for the list-end marker */
		peer->local_id = OL_TXRX_INVALID_LOCAL_PEER_ID;
	} else {
		/* take the head ID and advance the freelist */
		peer->local_id = i;
		pdev->local_peer_ids.freelist = pdev->local_peer_ids.pool[i];
		pdev->local_peer_ids.map[i] = peer;
	}
	qdf_spin_unlock_bh(&pdev->local_peer_ids.lock);
	dp_info("peer %pK, local id %d", peer, peer->local_id);
}

/**
 * dp_local_peer_id_free() - remove local peer id
 * @pdev - data path device instance
 * @peer - peer instance should be removed
 *
 * remove local peer id
 *
 * Return: none
 */
void dp_local_peer_id_free(struct dp_pdev *pdev, struct dp_peer *peer)
{
	int i = peer->local_id;
	if ((i == OL_TXRX_INVALID_LOCAL_PEER_ID) ||
	    (i >= OL_TXRX_NUM_LOCAL_PEER_IDS)) {
		return;
	}

	/* put this ID on the head of the freelist */
	qdf_spin_lock_bh(&pdev->local_peer_ids.lock);
	pdev->local_peer_ids.pool[i] = pdev->local_peer_ids.freelist;
	pdev->local_peer_ids.freelist = i;
	pdev->local_peer_ids.map[i] = NULL;
	qdf_spin_unlock_bh(&pdev->local_peer_ids.lock);
}

bool dp_find_peer_exist_on_vdev(struct cdp_soc_t *soc_hdl,
				uint8_t vdev_id, uint8_t *peer_addr)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer = NULL;

	peer = dp_peer_find_hash_find(soc, peer_addr, 0, vdev_id,
				      DP_MOD_ID_CDP);
	if (!peer)
		return false;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return true;
}

bool dp_find_peer_exist_on_other_vdev(struct cdp_soc_t *soc_hdl,
				      uint8_t vdev_id, uint8_t *peer_addr,
				      uint16_t max_bssid)
{
	int i;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer = NULL;

	for (i = 0; i < max_bssid; i++) {
		/* Need to check vdevs other than the vdev_id */
		if (vdev_id == i)
			continue;
		peer = dp_peer_find_hash_find(soc, peer_addr, 0, i,
					      DP_MOD_ID_CDP);
		if (peer) {
			dp_err("Duplicate peer "QDF_MAC_ADDR_FMT" already exist on vdev %d",
			       QDF_MAC_ADDR_REF(peer_addr), i);
			dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
			return true;
		}
	}

	return false;
}

bool dp_find_peer_exist(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			uint8_t *peer_addr)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer = NULL;

	peer = dp_peer_find_hash_find(soc, peer_addr, 0, DP_VDEV_ALL,
				      DP_MOD_ID_CDP);
	if (peer) {
		dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
		return true;
	}

	return false;
}

void dp_set_peer_as_tdls_peer(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			      uint8_t *peer_mac, bool val)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer = NULL;

	peer = dp_peer_find_hash_find(soc, peer_mac, 0, vdev_id,
				      DP_MOD_ID_CDP);
	if (!peer) {
		dp_err("Failed to find peer for:" QDF_MAC_ADDR_FMT,
		       QDF_MAC_ADDR_REF(peer_mac));
		return;
	}

	dp_info("Set tdls flag %d for peer:" QDF_MAC_ADDR_FMT,
		val, QDF_MAC_ADDR_REF(peer_mac));
	peer->is_tdls_peer = val;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
}
#endif

/**
 * dp_peer_rxtid_stats: Retried Rx TID (REO queue) stats from HW
 * @peer: DP peer handle
 * @dp_stats_cmd_cb: REO command callback function
 * @cb_ctxt: Callback context
 *
 * Return: count of tid stats cmd send succeeded
 */
int dp_peer_rxtid_stats(struct dp_peer *peer,
			dp_rxtid_stats_cmd_cb dp_stats_cmd_cb,
			void *cb_ctxt)
{
	struct dp_soc *soc = peer->vdev->pdev->soc;
	struct hal_reo_cmd_params params;
	int i;
	int stats_cmd_sent_cnt = 0;
	QDF_STATUS status;

	if (!dp_stats_cmd_cb)
		return stats_cmd_sent_cnt;

	qdf_mem_zero(&params, sizeof(params));
	for (i = 0; i < DP_MAX_TIDS; i++) {
		struct dp_rx_tid *rx_tid = &peer->rx_tid[i];
		if (rx_tid->hw_qdesc_vaddr_unaligned) {
			params.std.need_status = 1;
			params.std.addr_lo =
				rx_tid->hw_qdesc_paddr & 0xffffffff;
			params.std.addr_hi =
				(uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;

			if (cb_ctxt) {
				status = dp_reo_send_cmd(
						soc, CMD_GET_QUEUE_STATS,
						&params, dp_stats_cmd_cb,
						cb_ctxt);
			} else {
				status = dp_reo_send_cmd(
						soc, CMD_GET_QUEUE_STATS,
						&params, dp_stats_cmd_cb,
						rx_tid);
			}

			if (QDF_IS_STATUS_SUCCESS(status))
				stats_cmd_sent_cnt++;

			/* Flush REO descriptor from HW cache to update stats
			 * in descriptor memory. This is to help debugging */
			qdf_mem_zero(&params, sizeof(params));
			params.std.need_status = 0;
			params.std.addr_lo =
				rx_tid->hw_qdesc_paddr & 0xffffffff;
			params.std.addr_hi =
				(uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;
			params.u.fl_cache_params.flush_no_inval = 1;
			dp_reo_send_cmd(soc, CMD_FLUSH_CACHE, &params, NULL,
				NULL);
		}
	}

	return stats_cmd_sent_cnt;
}

QDF_STATUS
dp_set_michael_key(struct cdp_soc_t *soc,
		   uint8_t vdev_id,
		   uint8_t *peer_mac,
		   bool is_unicast, uint32_t *key)
{
	uint8_t sec_index = is_unicast ? 1 : 0;
	struct dp_peer *peer = dp_peer_find_hash_find((struct dp_soc *)soc,
						      peer_mac, 0, vdev_id,
						      DP_MOD_ID_CDP);

	if (!peer) {
		dp_peer_err("%pK: peer not found ", soc);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(&peer->security[sec_index].michael_key[0],
		     key, IEEE80211_WEP_MICLEN);

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}


/**
 * dp_vdev_bss_peer_ref_n_get: Get bss peer of a vdev
 * @soc: DP soc
 * @vdev: vdev
 * @mod_id: id of module requesting reference
 *
 * Return: VDEV BSS peer
 */
struct dp_peer *dp_vdev_bss_peer_ref_n_get(struct dp_soc *soc,
					   struct dp_vdev *vdev,
					   enum dp_mod_id mod_id)
{
	struct dp_peer *peer = NULL;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (peer->bss_peer)
			break;
	}

	if (!peer) {
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		return NULL;
	}

	if (dp_peer_get_ref(soc, peer, mod_id) == QDF_STATUS_SUCCESS) {
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		return peer;
	}

	qdf_spin_unlock_bh(&vdev->peer_list_lock);
	return peer;
}

/**
 * dp_sta_vdev_self_peer_ref_n_get: Get self peer of sta vdev
 * @soc: DP soc
 * @vdev: vdev
 * @mod_id: id of module requesting reference
 *
 * Return: VDEV self peer
 */
struct dp_peer *dp_sta_vdev_self_peer_ref_n_get(struct dp_soc *soc,
						struct dp_vdev *vdev,
						enum dp_mod_id mod_id)
{
	struct dp_peer *peer;

	if (vdev->opmode != wlan_op_mode_sta)
		return NULL;

	qdf_spin_lock_bh(&vdev->peer_list_lock);
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (peer->sta_self_peer)
			break;
	}

	if (!peer) {
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		return NULL;
	}

	if (dp_peer_get_ref(soc, peer, mod_id) == QDF_STATUS_SUCCESS) {
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		return peer;
	}

	qdf_spin_unlock_bh(&vdev->peer_list_lock);
	return peer;
}

#ifdef DUMP_REO_QUEUE_INFO_IN_DDR
void dp_dump_rx_reo_queue_info(
	struct dp_soc *soc, void *cb_ctxt, union hal_reo_status *reo_status)
{
	struct dp_rx_tid *rx_tid = (struct dp_rx_tid *)cb_ctxt;

	if (!rx_tid)
		return;

	if (reo_status->fl_cache_status.header.status !=
		HAL_REO_CMD_SUCCESS) {
		dp_err_rl("Rx tid REO HW desc flush failed(%d)",
			  reo_status->rx_queue_status.header.status);
		return;
	}
	qdf_spin_lock_bh(&rx_tid->tid_lock);
	hal_dump_rx_reo_queue_desc(rx_tid->hw_qdesc_vaddr_aligned);
	qdf_spin_unlock_bh(&rx_tid->tid_lock);
}

void dp_send_cache_flush_for_rx_tid(
	struct dp_soc *soc, struct dp_peer *peer)
{
	int i;
	struct dp_rx_tid *rx_tid;
	struct hal_reo_cmd_params params;

	if (!peer) {
		dp_err_rl("Peer is NULL");
		return;
	}

	for (i = 0; i < DP_MAX_TIDS; i++) {
		rx_tid = &peer->rx_tid[i];
		if (!rx_tid)
			continue;
		qdf_spin_lock_bh(&rx_tid->tid_lock);
		if (rx_tid->hw_qdesc_vaddr_aligned) {
			qdf_mem_zero(&params, sizeof(params));
			params.std.need_status = 1;
			params.std.addr_lo =
				rx_tid->hw_qdesc_paddr & 0xffffffff;
			params.std.addr_hi =
				(uint64_t)(rx_tid->hw_qdesc_paddr) >> 32;
			params.u.fl_cache_params.flush_no_inval = 0;
			if (QDF_STATUS_SUCCESS !=
				dp_reo_send_cmd(
					soc, CMD_FLUSH_CACHE,
					&params, dp_dump_rx_reo_queue_info,
					(void *)rx_tid)) {
				dp_err_rl("cache flush send failed tid %d",
					  rx_tid->tid);
				qdf_spin_unlock_bh(&rx_tid->tid_lock);
				break;
			}
		}
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
	}
}

void dp_get_rx_reo_queue_info(
	struct cdp_soc_t *soc_hdl, uint8_t vdev_id)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_GENERIC_STATS);
	struct dp_peer *peer = NULL;

	if (!vdev) {
		dp_err_rl("vdev is null for vdev_id: %u", vdev_id);
		goto failed;
	}

	peer = dp_vdev_bss_peer_ref_n_get(soc, vdev, DP_MOD_ID_GENERIC_STATS);

	if (!peer) {
		dp_err_rl("Peer is NULL");
		goto failed;
	}
	dp_send_cache_flush_for_rx_tid(soc, peer);
failed:
	if (peer)
		dp_peer_unref_delete(peer, DP_MOD_ID_GENERIC_STATS);
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_GENERIC_STATS);
}
#endif /* DUMP_REO_QUEUE_INFO_IN_DDR */

void dp_peer_flush_frags(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			 uint8_t *peer_mac)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_peer *peer = dp_peer_find_hash_find(soc, peer_mac, 0,
						      vdev_id, DP_MOD_ID_CDP);
	struct dp_rx_tid *rx_tid;
	uint8_t tid;

	if (!peer)
		return;

	dp_info("Flushing fragments for peer " QDF_MAC_ADDR_FMT,
		QDF_MAC_ADDR_REF(peer->mac_addr.raw));

	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		rx_tid = &peer->rx_tid[tid];

		qdf_spin_lock_bh(&rx_tid->tid_lock);
		dp_rx_defrag_waitlist_remove(peer, tid);
		dp_rx_reorder_flush_frag(peer, tid);
		qdf_spin_unlock_bh(&rx_tid->tid_lock);
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);
}

/*
 * dp_peer_find_by_id_valid - check if peer exists for given id
 * @soc: core DP soc context
 * @peer_id: peer id from peer object can be retrieved
 *
 * Return: true if peer exists of false otherwise
 */
bool dp_peer_find_by_id_valid(struct dp_soc *soc, uint16_t peer_id)
{
	struct dp_peer *peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_HTT);

	if (peer) {
		/*
		 * Decrement the peer ref which is taken as part of
		 * dp_peer_get_ref_by_id if PEER_LOCK_REF_PROTECT is enabled
		 */
		dp_peer_unref_delete(peer, DP_MOD_ID_HTT);

		return true;
	}

	return false;
}

qdf_export_symbol(dp_peer_find_by_id_valid);
