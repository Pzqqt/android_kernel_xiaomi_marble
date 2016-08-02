/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#include "dp_htt.h"
#include "dp_types.h"
#include "dp_internal.h"
#include <hal_api.h>

/* Temporary definitions to be moved to wlan_cfg */
static inline uint32_t wlan_cfg_max_peer_id(void *wlan_cfg_ctx)
{
	/* TODO: This should be calculated based on target capabilities */
	return 2048;
}

static inline int dp_peer_find_mac_addr_cmp(
	union dp_align_mac_addr *mac_addr1,
	union dp_align_mac_addr *mac_addr2)
{
	return !((mac_addr1->align4.bytes_abcd == mac_addr2->align4.bytes_abcd)
		/*
		 * Intentionally use & rather than &&.
		 * because the operands are binary rather than generic boolean,
		 * the functionality is equivalent.
		 * Using && has the advantage of short-circuited evaluation,
		 * but using & has the advantage of no conditional branching,
		 * which is a more significant benefit.
		 */
		&
		(mac_addr1->align4.bytes_ef == mac_addr2->align4.bytes_ef));
}

static inline struct dp_peer *dp_peer_find_by_id(
	struct dp_soc *soc, uint16_t peer_id)
{
	struct dp_peer *peer;
	peer = (peer_id == HTT_INVALID_PEER) ? NULL :
		soc->peer_id_to_obj_map[peer_id];
	/*
	 * Currently, peer IDs are assigned to vdevs as well as peers.
	 * If the peer ID is for a vdev, the peer_id_to_obj_map entry
	 * will hold NULL rather than a valid peer pointer.
	 */
	return peer;
}

static int dp_peer_find_map_attach(struct dp_soc *soc)
{
	uint32_t max_peers, peer_map_size;

	/* allocate the peer ID -> peer object map */
	max_peers = wlan_cfg_max_peer_id(soc->wlan_cfg_ctx) + 1;
	soc->max_peers = max_peers;
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"\n<=== cfg max peer id %d ====>\n", max_peers);
	peer_map_size = max_peers * sizeof(soc->peer_id_to_obj_map[0]);
	soc->peer_id_to_obj_map = qdf_mem_malloc(peer_map_size);
	if (!soc->peer_id_to_obj_map) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: peer map memory allocation failed\n", __func__);
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
#ifdef notyet /* ATH_BAND_STEERING */
		OS_INIT_TIMER(soc->osdev, &(soc->bs_inact_timer),
			dp_peer_find_inact_timeout_handler, (void *)soc,
			QDF_TIMER_TYPE_WAKE_APPS);
#endif
	return 0; /* success */
}

static int dp_log2_ceil(unsigned value)
{
	unsigned tmp = value;
	int log2 = -1;

	while (tmp) {
		log2++;
		tmp >>= 1;
	}
	if (1 << log2 != value)
		log2++;
	return log2;
}

static int dp_peer_find_add_id_to_obj(
	struct dp_peer *peer,
	uint16_t peer_id)
{
	int i;

	for (i = 0; i < MAX_NUM_PEER_ID_PER_PEER; i++) {
		if (peer->peer_ids[i] == HTT_INVALID_PEER) {
			peer->peer_ids[i] = peer_id;
			return 0; /* success */
		}
	}
	return QDF_STATUS_E_FAILURE; /* failure */
}

#define DP_PEER_HASH_LOAD_MULT  2
#define DP_PEER_HASH_LOAD_SHIFT 0

static int dp_peer_find_hash_attach(struct dp_soc *soc)
{
	int i, hash_elems, log2;

	/* allocate the peer MAC address -> peer object hash table */
	hash_elems = wlan_cfg_max_peer_id(soc->wlan_cfg_ctx) + 1;
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

	return 0;
}

static void dp_peer_find_hash_detach(struct dp_soc *soc)
{
	qdf_mem_free(soc->peer_hash.bins);
}

static inline unsigned dp_peer_find_hash_index(struct dp_soc *soc,
	union dp_align_mac_addr *mac_addr)
{
	unsigned index;

	index =
		mac_addr->align2.bytes_ab ^
		mac_addr->align2.bytes_cd ^
		mac_addr->align2.bytes_ef;
	index ^= index >> soc->peer_hash.idx_bits;
	index &= soc->peer_hash.mask;
	return index;
}


void dp_peer_find_hash_add(struct dp_soc *soc, struct dp_peer *peer)
{
	unsigned index;

	index = dp_peer_find_hash_index(soc, &peer->mac_addr);
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	/*
	 * It is important to add the new peer at the tail of the peer list
	 * with the bin index.  Together with having the hash_find function
	 * search from head to tail, this ensures that if two entries with
	 * the same MAC address are stored, the one added first will be
	 * found first.
	 */
	TAILQ_INSERT_TAIL(&soc->peer_hash.bins[index], peer, hash_list_elem);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);
}

#if ATH_SUPPORT_WRAP
struct dp_peer *dp_peer_find_hash_find(struct dp_soc *soc,
	uint8_t *peer_mac_addr, int mac_addr_is_aligned, uint8_t vdev_id)
#else
struct dp_peer *dp_peer_find_hash_find(struct dp_soc *soc,
	uint8_t *peer_mac_addr, int mac_addr_is_aligned)
#endif
{
	union dp_align_mac_addr local_mac_addr_aligned, *mac_addr;
	unsigned index;
	struct dp_peer *peer;

	if (mac_addr_is_aligned) {
		mac_addr = (union dp_align_mac_addr *) peer_mac_addr;
	} else {
		qdf_mem_copy(
			&local_mac_addr_aligned.raw[0],
			peer_mac_addr, DP_MAC_ADDR_LEN);
		mac_addr = &local_mac_addr_aligned;
	}
	index = dp_peer_find_hash_index(soc, mac_addr);
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	TAILQ_FOREACH(peer, &soc->peer_hash.bins[index], hash_list_elem) {
#if ATH_SUPPORT_WRAP
		/* ProxySTA may have multiple BSS peer with same MAC address,
		 * modified find will take care of finding the correct BSS peer.
		 */
		if (dp_peer_find_mac_addr_cmp(mac_addr, &peer->mac_addr) == 0 &&
			(peer->vdev->vdev_id == vdev_id)) {
#else
		if (dp_peer_find_mac_addr_cmp(mac_addr, &peer->mac_addr) == 0) {
#endif
			/* found it - increment the ref count before releasing
			 * the lock
			 */
			qdf_atomic_inc(&peer->ref_cnt);
			qdf_spin_unlock_bh(&soc->peer_ref_mutex);
			return peer;
		}
	}
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);
	return NULL; /* failure */
}

void dp_peer_find_hash_remove(struct dp_soc *soc, struct dp_peer *peer)
{
	unsigned index;
	struct dp_peer *tmppeer = NULL;
	int found = 0;

	index = dp_peer_find_hash_index(soc, &peer->mac_addr);
	/* Check if tail is not empty before delete*/
	QDF_ASSERT(!TAILQ_EMPTY(&soc->peer_hash.bins[index]));
	/*
	 * DO NOT take the peer_ref_mutex lock here - it needs to be taken
	 * by the caller.
	 * The caller needs to hold the lock from the time the peer object's
	 * reference count is decremented and tested up through the time the
	 * reference to the peer object is removed from the hash table, by
	 * this function.
	 * Holding the lock only while removing the peer object reference
	 * from the hash table keeps the hash table consistent, but does not
	 * protect against a new HL tx context starting to use the peer object
	 * if it looks up the peer object from its MAC address just after the
	 * peer ref count is decremented to zero, but just before the peer
	 * object reference is removed from the hash table.
	 */
	 TAILQ_FOREACH(tmppeer, &soc->peer_hash.bins[index], hash_list_elem) {
		if (tmppeer == peer) {
			found = 1;
			break;
		}
	}
	QDF_ASSERT(found);
	TAILQ_REMOVE(&soc->peer_hash.bins[index], peer, hash_list_elem);
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
				/* incr to one */
				qdf_atomic_inc(&peer->ref_cnt);
				dp_peer_unref_delete(peer);
			}
		}
	}
}

static void dp_peer_find_map_detach(struct dp_soc *soc)
{
#ifdef notyet /* ATH_BAND_STEERING */
	OS_FREE_TIMER(&(soc->bs_inact_timer));
#endif
	qdf_mem_free(soc->peer_id_to_obj_map);
}

int dp_peer_find_attach(struct dp_soc *soc)
{
	if (dp_peer_find_map_attach(soc))
		return 1;

	if (dp_peer_find_hash_attach(soc)) {
		dp_peer_find_map_detach(soc);
		return 1;
	}
	return 0; /* success */
}

static inline void dp_peer_find_add_id(struct dp_soc *soc,
	uint8_t *peer_mac_addr, uint16_t peer_id, uint8_t vdev_id)
{
	struct dp_peer *peer;

	QDF_ASSERT(peer_id <= wlan_cfg_max_peer_id(soc->wlan_cfg_ctx) + 1);
	/* check if there's already a peer object with this MAC address */
#if ATH_SUPPORT_WRAP
	peer = dp_peer_find_hash_find(soc, peer_mac_addr,
		0 /* is aligned */, vdev_id);
#else
	peer = dp_peer_find_hash_find(soc, peer_mac_addr, 0 /* is aligned */);
#endif
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		"%s: peer %p ID %d vid %d mac %02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, peer, peer_id, vdev_id, peer_mac_addr[0],
		peer_mac_addr[1], peer_mac_addr[2], peer_mac_addr[3],
		peer_mac_addr[4], peer_mac_addr[5]);

	if (peer) {
		/* peer's ref count was already incremented by
		 * peer_find_hash_find
		 */
		soc->peer_id_to_obj_map[peer_id] = peer;

		if (dp_peer_find_add_id_to_obj(peer, peer_id)) {
			/* TBDXXX: assert for now */
			QDF_ASSERT(0);
		}

		return;
	}
}

void
dp_rx_peer_map_handler(void *soc_handle, uint16_t peer_id, uint8_t vdev_id,
	uint8_t *peer_mac_addr)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"peer_map_event (soc:%p): peer_id %d, peer_mac "
		"%02x:%02x:%02x:%02x:%02x:%02x, vdev_id %d\n", soc, peer_id,
		peer_mac_addr[0], peer_mac_addr[1], peer_mac_addr[2],
		peer_mac_addr[3], peer_mac_addr[4], peer_mac_addr[5], vdev_id);

	dp_peer_find_add_id(soc, peer_mac_addr, peer_id, vdev_id);
}

void
dp_rx_peer_unmap_handler(void *soc_handle, uint16_t peer_id)
{
	struct dp_peer *peer;
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	uint8_t i;
	peer = dp_peer_find_by_id(soc, peer_id);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"peer_unmap_event (soc:%p) peer_id %d peer %p\n",
		soc, peer_id, peer);

	/*
	 * Currently peer IDs are assigned for vdevs as well as peers.
	 * If the peer ID is for a vdev, then the peer pointer stored
	 * in peer_id_to_obj_map will be NULL.
	 */
	if (!peer)
		return;

	soc->peer_id_to_obj_map[peer_id] = NULL;
	for (i = 0; i < MAX_NUM_PEER_ID_PER_PEER; i++) {
		if (peer->peer_ids[i] == peer_id) {
			peer->peer_ids[i] = HTT_INVALID_PEER;
			break;
		}
	}

	/*
	 * Remove a reference to the peer.
	 * If there are no more references, delete the peer object.
	 */
	dp_peer_unref_delete(peer);
}

void
dp_peer_find_detach(struct dp_soc *soc)
{
	dp_peer_find_map_detach(soc);
	dp_peer_find_hash_detach(soc);
}

/*
 * dp_find_peer_by_addr - find peer instance by mac address
 * @dev: physical device instance
 * @peer_mac_addr: peer mac address
 * @local_id: local id for the peer
 *
 * Return: peer instance pointer
 */
void *dp_find_peer_by_addr(void *dev, uint8_t *peer_mac_addr,
		uint8_t *local_id)
{
	struct dp_pdev *pdev = dev;
	struct dp_peer *peer;

#if ATH_SUPPORT_WRAP
	peer = dp_peer_find_hash_find(pdev->soc, peer_mac_addr, 0, 0);
	/* WAR, VDEV ID? TEMP 0 */
#else
	peer = dp_peer_find_hash_find(pdev->soc, peer_mac_addr, 0);
#endif
	if (!peer)
		return NULL;

	/* Multiple peer ids? How can know peer id? */
	*local_id = peer->local_id;
	DP_TRACE(INFO, "%s: peer %p id %d", __func__, peer, *local_id);
	return peer;
}

/*
 * dp_rx_tid_update_wifi3() – Update receive TID state
 * @peer: Datapath peer handle
 * @tid: TID
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 *
 * Return: 0 on success, error code on failure
 */
int dp_rx_tid_update_wifi3(struct dp_peer *peer, int tid, uint32_t
ba_window_size, uint32_t start_seq)
{
	/* TODO: Implement this once REO command API is available */
	return 0;
}

/*
 * dp_rx_tid_setup_wifi3() – Setup receive TID state
 * @peer: Datapath peer handle
 * @tid: TID
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 *
 * Return: 0 on success, error code on failure
 */
int dp_rx_tid_setup_wifi3(struct dp_peer *peer, int tid,
	uint32_t ba_window_size, uint32_t start_seq)
{
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];
	struct dp_vdev *vdev = peer->vdev;
	struct dp_soc *soc = vdev->pdev->soc;
	uint32_t hw_qdesc_size;
	uint32_t hw_qdesc_align;
	int hal_pn_type;
	void *hw_qdesc_vaddr;

	if (rx_tid->hw_qdesc_vaddr_unaligned != NULL)
		return dp_rx_tid_update_wifi3(peer, tid, ba_window_size,
			start_seq);

#ifdef notyet
	hw_qdesc_size = hal_get_reo_qdesc_size(soc->hal_soc, ba_window_size);
#else
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
			HAL_RX_MAX_BA_WINDOW);
	else
		hw_qdesc_size = hal_get_reo_qdesc_size(soc->hal_soc,
			ba_window_size);
#endif
	hw_qdesc_align = hal_get_reo_qdesc_align(soc->hal_soc);
	/* To avoid unnecessary extra allocation for alignment, try allocating
	 * exact size and see if we already have aligned address.
	 */
	rx_tid->hw_qdesc_alloc_size = hw_qdesc_size;
	rx_tid->hw_qdesc_vaddr_unaligned = qdf_mem_alloc_consistent(
		soc->osdev, soc->osdev->dev, rx_tid->hw_qdesc_alloc_size,
		&(rx_tid->hw_qdesc_paddr_unaligned));

	if (!rx_tid->hw_qdesc_vaddr_unaligned) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Rx tid HW desc alloc failed: tid %d\n",
			__func__, tid);
		return QDF_STATUS_E_NOMEM;
	}

	if ((unsigned long)(rx_tid->hw_qdesc_vaddr_unaligned) %
		hw_qdesc_align) {
		/* Address allocated above is not alinged. Allocate extra
		 * memory for alignment
		 */
		qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				rx_tid->hw_qdesc_alloc_size,
				rx_tid->hw_qdesc_vaddr_unaligned,
				rx_tid->hw_qdesc_paddr_unaligned, 0);

		rx_tid->hw_qdesc_alloc_size =
			hw_qdesc_size + hw_qdesc_align - 1;
		rx_tid->hw_qdesc_vaddr_unaligned = qdf_mem_alloc_consistent(
			soc->osdev, soc->osdev->dev, rx_tid->hw_qdesc_alloc_size,
			&(rx_tid->hw_qdesc_paddr_unaligned));

		if (!rx_tid->hw_qdesc_vaddr_unaligned) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Rx tid HW desc alloc failed: tid %d\n",
				__func__, tid);
			return QDF_STATUS_E_NOMEM;
		}

		hw_qdesc_vaddr = rx_tid->hw_qdesc_vaddr_unaligned +
			((unsigned long)(rx_tid->hw_qdesc_vaddr_unaligned) %
			hw_qdesc_align);

		rx_tid->hw_qdesc_paddr = rx_tid->hw_qdesc_paddr_unaligned +
			((unsigned long)hw_qdesc_vaddr -
			(unsigned long)(rx_tid->hw_qdesc_vaddr_unaligned));
	} else {
		hw_qdesc_vaddr = rx_tid->hw_qdesc_vaddr_unaligned;
		rx_tid->hw_qdesc_paddr = rx_tid->hw_qdesc_paddr_unaligned;
	}

	/* TODO: Ensure that sec_type is set before ADDBA is received.
	 * Currently this is set based on htt indication
	 * HTT_T2H_MSG_TYPE_SEC_IND from target
	 */
	switch (peer->security[dp_sec_ucast].sec_type) {
	case htt_sec_type_tkip_nomic:
	case htt_sec_type_aes_ccmp:
	case htt_sec_type_aes_ccmp_256:
	case htt_sec_type_aes_gcmp:
	case htt_sec_type_aes_gcmp_256:
		hal_pn_type = HAL_PN_WPA;
		break;
	case htt_sec_type_wapi:
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
		hw_qdesc_vaddr, rx_tid->hw_qdesc_paddr, hal_pn_type);

	if (soc->cdp_soc.ol_ops->peer_rx_reorder_queue_setup) {
		soc->cdp_soc.ol_ops->peer_rx_reorder_queue_setup(soc->osif_soc,
			peer->vdev->vdev_id, peer->mac_addr.raw,
			rx_tid->hw_qdesc_paddr, tid, tid);

		if (tid == DP_NON_QOS_TID) {
			/* TODO: Setting up default queue - currently using
			 * same queue for BE and non-qos traffic till BA
			 * session is setup. Check if there are any HW
			 * restrictions and also if this can be done for
			 * all other TIDs
			 */
			soc->cdp_soc.ol_ops->
				peer_rx_reorder_queue_setup(soc->osif_soc,
					peer->vdev->vdev_id, peer->mac_addr.raw,
					rx_tid->hw_qdesc_paddr, 0, tid);
		}
	}
	return 0;
}

/*
 * Rx TID deletion callback to free memory allocated for HW queue descriptor
 */
void dp_rx_tid_delete_cb(struct dp_pdev *pdev, void *cb_ctxt, int status)
{
	struct dp_soc *soc = pdev->soc;
	struct dp_rx_tid *rx_tid = (struct dp_rx_tid *)cb_ctxt;

	if (status) {
		/* Should not happen normally. Just print error for now */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Rx tid HW desc deletion failed: tid %d\n",
				__func__, rx_tid->tid);
	}

	qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				rx_tid->hw_qdesc_alloc_size,
				rx_tid->hw_qdesc_vaddr_unaligned,
				rx_tid->hw_qdesc_paddr_unaligned, 0);

	rx_tid->hw_qdesc_vaddr_unaligned = NULL;
	rx_tid->hw_qdesc_alloc_size = 0;
}

/*
 * dp_rx_tid_delete_wifi3() – Delete receive TID queue
 * @peer: Datapath peer handle
 * @tid: TID
 *
 * Return: 0 on success, error code on failure
 */
int dp_rx_tid_delete_wifi3(struct dp_peer *peer, int tid)
{
#ifdef notyet /* TBD: Enable this once REO command interface is available */
	struct dp_rx_tid *rx_tid = peer->rx_tid[tid];
	dp_rx_tid_hw_update_valid(rx_tid->hw_qdesc_paddr, 0,
		dp_rx_tid_delete_cb, (void *)rx_tid);
#endif
	return 0;
}

/*
 * dp_peer_rx_init() – Initialize receive TID state
 * @pdev: Datapath pdev
 * @peer: Datapath peer
 *
 */
void dp_peer_rx_init(struct dp_pdev *pdev, struct dp_peer *peer)
{
	int tid;
	struct dp_rx_tid *rx_tid;
	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		rx_tid = &peer->rx_tid[tid];
		rx_tid->array = &rx_tid->base;
		rx_tid->base.head = rx_tid->base.tail = NULL;
		rx_tid->tid = tid;
		rx_tid->defrag_timeout_ms = 0;
		rx_tid->ba_win_size = 0;
		rx_tid->ba_status = DP_RX_BA_INACTIVE;

		rx_tid->defrag_waitlist_elem.tqe_next = NULL;
		rx_tid->defrag_waitlist_elem.tqe_prev = NULL;

#ifdef notyet /* TODO: See if this is required for exception handling */
		/* invalid sequence number */
		peer->tids_last_seq[tid] = 0xffff;
#endif
	}

	/* Setup default (non-qos) rx tid queue */
	dp_rx_tid_setup_wifi3(peer, DP_NON_QOS_TID, 1, 0);

	/*
	 * Set security defaults: no PN check, no security. The target may
	 * send a HTT SEC_IND message to overwrite these defaults.
	 */
	peer->security[dp_sec_ucast].sec_type =
		peer->security[dp_sec_mcast].sec_type = htt_sec_type_none;
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
	struct dp_rx_tid *rx_tid;
	uint32_t tid_delete_mask = 0;
	for (tid = 0; tid < DP_MAX_TIDS; tid++) {
		if (rx_tid->hw_qdesc_vaddr_unaligned != NULL) {
			dp_rx_tid_delete_wifi3(peer, tid);
			tid_delete_mask |= (1 << tid);
		}
	}
#ifdef notyet /* See if FW can remove queues as part of peer cleanup */
	if (soc->ol_ops->peer_rx_reorder_queue_remove) {
		soc->ol_ops->peer_rx_reorder_queue_remove(soc->osif_soc,
			peer->vdev->vdev_id, peer->mac_addr.raw,
			tid_delete_mask);
	}
#endif
}

/*
* dp_rx_addba_requestprocess_wifi3() – Process ADDBA request from peer
*
* @peer: Datapath peer handle
* @dialogtoken: dialogtoken from ADDBA frame
* @tid: TID number
* @startseqnum: Start seq. number received in BA sequence control
* in ADDBA frame
*
* Return: 0 on success, error code on failure
*/
int dp_addba_requestprocess_wifi3(void *peer_handle,
	uint8_t dialogtoken, uint16_t tid, uint16_t batimeout,
	uint16_t buffersize, uint16_t startseqnum)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];

	if ((rx_tid->ba_status == DP_RX_BA_ACTIVE) &&
			(rx_tid->hw_qdesc_vaddr_unaligned != NULL))
		rx_tid->ba_status = DP_RX_BA_INACTIVE;

	if (dp_rx_tid_setup_wifi3(peer, tid, buffersize,
		startseqnum)) {
		/* TODO: Should we send addba reject in this case */
		return QDF_STATUS_E_FAILURE;
	}

	rx_tid->ba_win_size = buffersize;
	rx_tid->dialogtoken = dialogtoken;
	rx_tid->statuscode = QDF_STATUS_SUCCESS;
	rx_tid->ba_status = DP_RX_BA_ACTIVE;

	return 0;
}

/*
* dp_rx_addba_responsesetup_wifi3() – Process ADDBA request from peer
*
* @peer: Datapath peer handle
* @tid: TID number
* @dialogtoken: output dialogtoken
* @statuscode: output dialogtoken
* @buffersize: Ouput BA window sizze
* @batimeout: Ouput BA timeout
*/
void dp_addba_responsesetup_wifi3(void *peer_handle, uint8_t tid,
	uint8_t *dialogtoken, uint16_t *statuscode,
	uint16_t *buffersize, uint16_t *batimeout)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];

	/* setup ADDBA response paramters */
	*dialogtoken = rx_tid->dialogtoken;
	*statuscode = rx_tid->statuscode;
	*buffersize = rx_tid->ba_win_size;
	*batimeout  = 0;
}

/*
* dp_rx_delba_process_wifi3() – Process DELBA from peer
* @peer: Datapath peer handle
* @tid: TID number
* @reasoncode: Reason code received in DELBA frame
*
* Return: 0 on success, error code on failure
*/
int dp_delba_process_wifi3(void *peer_handle,
	int tid, uint16_t reasoncode)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];

	if (rx_tid->ba_status != DP_RX_BA_ACTIVE)
		return QDF_STATUS_E_FAILURE;

	/* TODO: See if we can delete the existing REO queue descriptor and
	 * replace with a new one without queue extenstion descript to save
	 * memory
	 */
	dp_rx_tid_update_wifi3(peer, tid, 0, 0);

	rx_tid->ba_status = DP_RX_BA_INACTIVE;

	return 0;
}

void dp_rx_discard(struct dp_vdev *vdev, struct dp_peer *peer, unsigned tid,
	qdf_nbuf_t msdu_list)
{
	while (msdu_list) {
		qdf_nbuf_t msdu = msdu_list;

		msdu_list = qdf_nbuf_next(msdu_list);
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			"discard rx %p from partly-deleted peer %p "
			"(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			msdu, peer,
			peer->mac_addr.raw[0], peer->mac_addr.raw[1],
			peer->mac_addr.raw[2], peer->mac_addr.raw[3],
			peer->mac_addr.raw[4], peer->mac_addr.raw[5]);
		qdf_nbuf_free(msdu);
	}
}

void
dp_rx_sec_ind_handler(void *soc_handle, uint16_t peer_id,
	enum htt_sec_type sec_type, int is_unicast, u_int32_t *michael_key,
	u_int32_t *rx_pn)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_peer *peer;
	int sec_index;

	peer = dp_peer_find_by_id(soc, peer_id);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Couldn't find peer from ID %d - skipping security inits\n",
			peer_id);
		return;
	}
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"sec spec for peer %p (%02x:%02x:%02x:%02x:%02x:%02x): "
		"%s key of type %d\n",
		peer,
		peer->mac_addr.raw[0], peer->mac_addr.raw[1],
		peer->mac_addr.raw[2], peer->mac_addr.raw[3],
		peer->mac_addr.raw[4], peer->mac_addr.raw[5],
		is_unicast ? "ucast" : "mcast",
		sec_type);
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
	if (sec_type != htt_sec_type_wapi) {
		qdf_mem_set(peer->tids_last_pn_valid, _EXT_TIDS, 0x00);
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
}

#ifndef CONFIG_WIN
/**
 * dp_register_peer() - Register peer into physical device
 * @pdev - data path device instance
 * @sta_desc - peer description
 *
 * Register peer into physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_FAULT peer not found
 */
QDF_STATUS dp_register_peer(void *pdev_handle,
		struct ol_txrx_desc_type *sta_desc)
{
	struct dp_peer *peer;
	struct dp_pdev *pdev = pdev_handle;

	peer = dp_peer_find_by_local_id(pdev, sta_desc->sta_id);
	if (!peer)
		return QDF_STATUS_E_FAULT;

	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_CONN;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_clear_peer() - remove peer from physical device
 * @pdev - data path device instance
 * @sta_id - local peer id
 *
 * remove peer from physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_FAULT peer not found
 */
QDF_STATUS dp_clear_peer(void *pdev_handle, uint8_t local_id)
{
	struct dp_peer *peer;
	struct dp_pdev *pdev = pdev_handle;

	peer = dp_peer_find_by_local_id(pdev, local_id);
	if (!peer)
		return QDF_STATUS_E_FAULT;

	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_DISC;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_find_peer_by_addr_and_vdev() - Find peer by peer mac address within vdev
 * @pdev - data path device instance
 * @vdev - virtual interface instance
 * @peer_addr - peer mac address
 * @peer_id - local peer id with target mac address
 *
 * Find peer by peer mac address within vdev
 *
 * Return: peer instance void pointer
 *         NULL cannot find target peer
 */
void *dp_find_peer_by_addr_and_vdev(void *pdev_handle, void *vdev,
		uint8_t *peer_addr, uint8_t *local_id)
{
	struct dp_pdev *pdev = pdev_handle;
	struct dp_peer *peer;

	DP_TRACE(INFO, "vdev %p peer_addr %p", vdev, peer_addr);
	peer = dp_peer_find_hash_find(pdev->soc, peer_addr, 0);
	DP_TRACE(INFO, "peer %p vdev %p", peer, vdev);

	if (!peer)
		return NULL;

	if (peer->vdev != vdev)
		return NULL;

	*local_id = peer->local_id;

	DP_TRACE(INFO, "peer %p vdev %p lcoal id %d",
			peer, vdev, *local_id);

	return peer;
}

/**
 * dp_local_peer_id() - Find local peer id within peer instance
 * @peer - peer instance
 *
 * Find local peer id within peer instance
 *
 * Return: local peer id
 */
uint16_t dp_local_peer_id(void *peer)
{
	return ((struct dp_peer *)peer)->local_id;
}

/**
 * dp_peer_find_by_local_id() - Find peer by local peer id
 * @pdev - data path device instance
 * @local_peer_id - local peer id want to find
 *
 * Find peer by local peer id within physical device
 *
 * Return: peer instance void pointer
 *         NULL cannot find target peer
 */
void *dp_peer_find_by_local_id(void *pdev_handle, uint8_t local_id)
{
	struct dp_peer *peer;
	struct dp_pdev *pdev = pdev_handle;

	qdf_spin_lock_bh(&pdev->local_peer_ids.lock);
	peer = pdev->local_peer_ids.map[local_id];
	qdf_spin_unlock_bh(&pdev->local_peer_ids.lock);
	DP_TRACE(INFO, "peer %p lcoal id %d",
			peer, local_id);
	return peer;
}

/**
 * dp_peer_state_update() - update peer local state
 * @pdev - data path device instance
 * @peer_addr - peer mac address
 * @state - new peer local state
 *
 * update peer local state
 *
 * Return: QDF_STATUS_SUCCESS registration success
 */
QDF_STATUS dp_peer_state_update(void *pdev_handle, uint8_t *peer_mac,
		enum ol_txrx_peer_state state)
{
	struct dp_peer *peer;
	struct dp_pdev *pdev = pdev_handle;

	peer =  dp_peer_find_hash_find(pdev->soc, peer_mac, 0);
	peer->state = state;
	DP_TRACE(INFO, "peer %p state %d",
			peer, peer->state);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_get_vdevid() - Get virtaul interface id which peer registered
 * @peer - peer instance
 * @vdev_id - virtaul interface id which peer registered
 *
 * Get virtaul interface id which peer registered
 *
 * Return: QDF_STATUS_SUCCESS registration success
 */
QDF_STATUS dp_get_vdevid(void *peer_handle, uint8_t *vdev_id)
{
	struct dp_peer *peer = peer_handle;

	DP_TRACE(INFO, "peer %p vdev %p vdev id %d",
			peer, peer->vdev, peer->vdev->vdev_id);
	*vdev_id = peer->vdev->vdev_id;
	return QDF_STATUS_SUCCESS;
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
void *dp_get_vdev_for_peer(void *peer_handle)
{
	struct dp_peer *peer = peer_handle;

	DP_TRACE(INFO, "peer %p vdev %p", peer, peer->vdev);
	return (void *)peer->vdev;
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
	DP_TRACE(INFO, "peer %p mac 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
		peer, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return peer->mac_addr.raw;
}

/**
 * dp_get_peer_state() - Get local peer state
 * @peer - peer instance
 *
 * Get local peer state
 *
 * Return: peer status
 */
int dp_get_peer_state(void *peer_handle)
{
	struct dp_peer *peer = peer_handle;

	DP_TRACE(INFO, "peer %p stats %d", peer, peer->state);
	return peer->state;
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
	DP_TRACE(INFO, "peer %p, local id %d", peer, peer->local_id);
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
#endif
