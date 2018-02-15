/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*=== includes ===*/
/* header files for OS primitives */
#include <osdep.h>              /* uint32_t, etc. */
#include <qdf_mem.h>         /* qdf_mem_malloc,free */
#include <qdf_types.h>          /* qdf_device_t, qdf_print */
#include <qdf_lock.h>           /* qdf_spinlock */
#include <qdf_atomic.h>         /* qdf_atomic_read */

#if defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB)
/* Required for WLAN_FEATURE_FASTPATH */
#include <ce_api.h>
#endif
/* header files for utilities */
#include <cds_queue.h>          /* TAILQ */

/* header files for configuration API */
#include <ol_cfg.h>             /* ol_cfg_is_high_latency */
#include <ol_if_athvar.h>

/* header files for HTT API */
#include <ol_htt_api.h>
#include <ol_htt_tx_api.h>

/* header files for our own APIs */
#include <ol_txrx_api.h>
#include <ol_txrx_dbg.h>
#include <cdp_txrx_ocb.h>
#include <ol_txrx_ctrl_api.h>
#include <cdp_txrx_stats.h>
#include <ol_txrx_osif_api.h>
/* header files for our internal definitions */
#include <ol_txrx_internal.h>   /* TXRX_ASSERT, etc. */
#include <wdi_event.h>          /* WDI events */
#include <ol_tx.h>              /* ol_tx_ll */
#include <ol_rx.h>              /* ol_rx_deliver */
#include <ol_txrx_peer_find.h>  /* ol_txrx_peer_find_attach, etc. */
#include <ol_rx_pn.h>           /* ol_rx_pn_check, etc. */
#include <ol_rx_fwd.h>          /* ol_rx_fwd_check, etc. */
#include <ol_rx_reorder_timeout.h>      /* OL_RX_REORDER_TIMEOUT_INIT, etc. */
#include <ol_rx_reorder.h>
#include <ol_tx_send.h>         /* ol_tx_discard_target_frms */
#include <ol_tx_desc.h>         /* ol_tx_desc_frame_free */
#include <ol_tx_queue.h>
#include <ol_tx_sched.h>           /* ol_tx_sched_attach, etc. */
#include <ol_txrx.h>
#include <ol_txrx_types.h>
#include <cdp_txrx_flow_ctrl_legacy.h>
#include <cdp_txrx_bus.h>
#include <cdp_txrx_ipa.h>
#include <cdp_txrx_pmf.h>
#include "wma.h"
#include "hif.h"
#include "hif_main.h"
#include <cdp_txrx_peer_ops.h>
#ifndef REMOVE_PKT_LOG
#include "pktlog_ac.h"
#endif
#include <wlan_policy_mgr_api.h>
#include "epping_main.h"
#include <a_types.h>
#include <cdp_txrx_handle.h>
#include "wlan_qct_sys.h"

#include <htt_internal.h>
#include <ol_txrx_ipa.h>
#include "wlan_roam_debug.h"

#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
ol_txrx_peer_handle
ol_txrx_peer_find_by_local_id(struct cdp_pdev *pdev,
			      uint8_t local_peer_id);
ol_txrx_peer_handle
ol_txrx_peer_get_ref_by_local_id(struct cdp_pdev *ppdev,
			      uint8_t local_peer_id,
			      enum peer_debug_id_type dbg_id);
#endif /* QCA_SUPPORT_TXRX_LOCAL_PEER_ID */
QDF_STATUS ol_txrx_peer_state_update(struct cdp_pdev *pdev,
				     uint8_t *peer_mac,
				     enum ol_txrx_peer_state state);
static void ol_vdev_rx_set_intrabss_fwd(struct cdp_vdev *vdev,
		bool val);
int ol_txrx_get_tx_pending(struct cdp_pdev *pdev_handle);
extern void
ol_txrx_set_wmm_param(struct cdp_pdev *data_pdev,
		      struct ol_tx_wmm_param_t wmm_param);

extern void ol_txrx_get_pn_info(void *ppeer, uint8_t **last_pn_valid,
		    uint64_t **last_pn, uint32_t **rmf_pn_replays);

/* thresh for peer's cached buf queue beyond which the elements are dropped */
#define OL_TXRX_CACHED_BUFQ_THRESH 128

#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)

/**
 * ol_txrx_copy_mac_addr_raw() - copy raw mac addr
 * @vdev: the data virtual device
 * @bss_addr: bss address
 *
 * Return: None
 */
static void
ol_txrx_copy_mac_addr_raw(struct cdp_vdev *pvdev, uint8_t *bss_addr)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t  *)pvdev;

	qdf_spin_lock_bh(&vdev->pdev->last_real_peer_mutex);
	if (bss_addr && vdev->last_real_peer &&
	    !qdf_mem_cmp((u8 *)bss_addr,
			     vdev->last_real_peer->mac_addr.raw,
			     IEEE80211_ADDR_LEN))
		qdf_mem_copy(vdev->hl_tdls_ap_mac_addr.raw,
			     vdev->last_real_peer->mac_addr.raw,
			     OL_TXRX_MAC_ADDR_LEN);
	qdf_spin_unlock_bh(&vdev->pdev->last_real_peer_mutex);
}

/**
 * ol_txrx_add_last_real_peer() - add last peer
 * @pdev: the data physical device
 * @vdev: virtual device
 * @peer_id: peer id
 *
 * Return: None
 */
static void
ol_txrx_add_last_real_peer(struct cdp_pdev *ppdev,
			   struct cdp_vdev *pvdev, uint8_t *peer_id)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	ol_txrx_peer_handle peer;

	peer = ol_txrx_find_peer_by_addr(
		(struct cdp_pdev *)pdev,
		vdev->hl_tdls_ap_mac_addr.raw,
		peer_id);

	qdf_spin_lock_bh(&pdev->last_real_peer_mutex);
	if (!vdev->last_real_peer && peer &&
	    (peer->peer_ids[0] != HTT_INVALID_PEER_ID))
		vdev->last_real_peer = peer;
	qdf_spin_unlock_bh(&pdev->last_real_peer_mutex);
}

/**
 * is_vdev_restore_last_peer() - check for vdev last peer
 * @peer: peer object
 *
 * Return: true if last peer is not null
 */
static bool
is_vdev_restore_last_peer(void *ppeer)
{
	struct ol_txrx_peer_t *peer = ppeer;
	struct ol_txrx_vdev_t *vdev;

	vdev = peer->vdev;
	return vdev->last_real_peer && (vdev->last_real_peer == peer);
}

/**
 * ol_txrx_update_last_real_peer() - check for vdev last peer
 * @pdev: the data physical device
 * @peer: peer device
 * @peer_id: peer id
 * @restore_last_peer: restore last peer flag
 *
 * Return: None
 */
static void
ol_txrx_update_last_real_peer(struct cdp_pdev *ppdev, void *ppeer,
	uint8_t *peer_id, bool restore_last_peer)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_peer_t *peer = ppeer;
	struct ol_txrx_vdev_t *vdev;

	if (!restore_last_peer)
		return;

	vdev = peer->vdev;
	peer = ol_txrx_find_peer_by_addr((struct cdp_pdev *)pdev,
				vdev->hl_tdls_ap_mac_addr.raw, peer_id);

	qdf_spin_lock_bh(&pdev->last_real_peer_mutex);
	if (!vdev->last_real_peer && peer &&
	    (peer->peer_ids[0] != HTT_INVALID_PEER_ID))
		vdev->last_real_peer = peer;
	qdf_spin_unlock_bh(&pdev->last_real_peer_mutex);
}
#endif

/**
 * ol_tx_mark_first_wakeup_packet() - set flag to indicate that
 *    fw is compatible for marking first packet after wow wakeup
 * @value: 1 for enabled/ 0 for disabled
 *
 * Return: None
 */
static void ol_tx_mark_first_wakeup_packet(uint8_t value)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		ol_txrx_err(
			"%s: pdev is NULL\n", __func__);
		return;
	}

	htt_mark_first_wakeup_packet(pdev->htt_pdev, value);
}

u_int16_t
ol_tx_desc_pool_size_hl(struct cdp_cfg *ctrl_pdev)
{
	u_int16_t desc_pool_size;
	u_int16_t steady_state_tx_lifetime_ms;
	u_int16_t safety_factor;

	/*
	 * Steady-state tx latency:
	 *     roughly 1-2 ms flight time
	 *   + roughly 1-2 ms prep time,
	 *   + roughly 1-2 ms target->host notification time.
	 * = roughly 6 ms total
	 * Thus, steady state number of frames =
	 * steady state max throughput / frame size * tx latency, e.g.
	 * 1 Gbps / 1500 bytes * 6 ms = 500
	 *
	 */
	steady_state_tx_lifetime_ms = 6;

	safety_factor = 8;

	desc_pool_size =
		ol_cfg_max_thruput_mbps(ctrl_pdev) *
		1000 /* 1e6 bps/mbps / 1e3 ms per sec = 1000 */ /
		(8 * OL_TX_AVG_FRM_BYTES) *
		steady_state_tx_lifetime_ms *
		safety_factor;

	/* minimum */
	if (desc_pool_size < OL_TX_DESC_POOL_SIZE_MIN_HL)
		desc_pool_size = OL_TX_DESC_POOL_SIZE_MIN_HL;

	/* maximum */
	if (desc_pool_size > OL_TX_DESC_POOL_SIZE_MAX_HL)
		desc_pool_size = OL_TX_DESC_POOL_SIZE_MAX_HL;

	return desc_pool_size;
}

/*=== function definitions ===*/

/**
 * ol_tx_set_is_mgmt_over_wmi_enabled() - set flag to indicate that mgmt over
 *                                        wmi is enabled or not.
 * @value: 1 for enabled/ 0 for disable
 *
 * Return: None
 */
void ol_tx_set_is_mgmt_over_wmi_enabled(uint8_t value)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		qdf_print("%s: pdev is NULL\n", __func__);
		return;
	}
	pdev->is_mgmt_over_wmi_enabled = value;
}

/**
 * ol_tx_get_is_mgmt_over_wmi_enabled() - get value of is_mgmt_over_wmi_enabled
 *
 * Return: is_mgmt_over_wmi_enabled
 */
uint8_t ol_tx_get_is_mgmt_over_wmi_enabled(void)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		qdf_print("%s: pdev is NULL\n", __func__);
		return 0;
	}
	return pdev->is_mgmt_over_wmi_enabled;
}


#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
static void *
ol_txrx_find_peer_by_addr_and_vdev(struct cdp_pdev *ppdev,
	struct cdp_vdev *pvdev, uint8_t *peer_addr, uint8_t *peer_id)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	struct ol_txrx_peer_t *peer;

	peer = ol_txrx_peer_vdev_find_hash(pdev, vdev, peer_addr, 0, 1);
	if (!peer)
		return NULL;
	*peer_id = peer->local_id;
	ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);
	return peer;
}

static QDF_STATUS ol_txrx_get_vdevid(void *ppeer, uint8_t *vdev_id)
{
	struct ol_txrx_peer_t *peer = ppeer;

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "peer argument is null!!");
		return QDF_STATUS_E_FAILURE;
	}

	*vdev_id = peer->vdev->vdev_id;
	return QDF_STATUS_SUCCESS;
}

static struct cdp_vdev *ol_txrx_get_vdev_by_sta_id(struct cdp_pdev *ppdev,
						   uint8_t sta_id)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_peer_t *peer = NULL;
	ol_txrx_vdev_handle vdev;

	if (sta_id >= WLAN_MAX_STA_COUNT) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			  "Invalid sta id passed");
		return NULL;
	}

	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			  "PDEV not found for sta_id [%d]", sta_id);
		return NULL;
	}

	peer = ol_txrx_peer_get_ref_by_local_id((struct cdp_pdev *)pdev, sta_id,
						PEER_DEBUG_ID_OL_INTERNAL);
	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			  "PEER [%d] not found", sta_id);
		return NULL;
	}

	vdev = peer->vdev;
	ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);

	return (struct cdp_vdev *)vdev;
}

/**
 * ol_txrx_find_peer_by_addr() - find peer via peer mac addr and peer_id
 * @ppdev: pointer of type cdp_pdev
 * @peer_addr: peer mac addr
 * @peer_id: pointer to fill in the value of peer->local_id for caller
 *
 * This function finds a peer with given mac address and returns its peer_id.
 * Note that this function does not increment the peer->ref_cnt.
 * This means that the peer may be deleted in some other parallel context after
 * its been found.
 *
 * Return: peer handle if peer is found, NULL if peer is not found.
 */
void *ol_txrx_find_peer_by_addr(struct cdp_pdev *ppdev,
				uint8_t *peer_addr,
				uint8_t *peer_id)
{
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	peer = ol_txrx_peer_find_hash_find_get_ref(pdev, peer_addr, 0, 1,
						   PEER_DEBUG_ID_OL_INTERNAL);
	if (!peer)
		return NULL;
	*peer_id = peer->local_id;
	ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);
	return peer;
}

/**
 * ol_txrx_peer_get_ref_by_addr() - get peer ref via peer mac addr and peer_id
 * @pdev: pointer of type ol_txrx_pdev_handle
 * @peer_addr: peer mac addr
 * @peer_id: pointer to fill in the value of peer->local_id for caller
 *
 * This function finds the peer with given mac address and returns its peer_id.
 * Note that this function increments the peer->ref_cnt.
 * This makes sure that peer will be valid. This also means the caller needs to
 * call the corresponding API - ol_txrx_peer_release_ref to delete the peer
 * reference.
 * Sample usage:
 *    {
 *      //the API call below increments the peer->ref_cnt
 *      peer = ol_txrx_peer_get_ref_by_addr(pdev, peer_addr, peer_id, dbg_id);
 *
 *      // Once peer usage is done
 *
 *      //the API call below decrements the peer->ref_cnt
 *       ol_txrx_peer_release_ref(peer, dbg_id);
 *    }
 *
 * Return: peer handle if the peer is found, NULL if peer is not found.
 */
ol_txrx_peer_handle ol_txrx_peer_get_ref_by_addr(ol_txrx_pdev_handle pdev,
						 u8 *peer_addr,
						 u8 *peer_id,
						 enum peer_debug_id_type dbg_id)
{
	struct ol_txrx_peer_t *peer;

	peer = ol_txrx_peer_find_hash_find_get_ref(pdev, peer_addr, 0, 1,
						   dbg_id);
	if (!peer)
		return NULL;
	*peer_id = peer->local_id;
	return peer;
}

static uint16_t ol_txrx_local_peer_id(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	return peer->local_id;
}

/**
 * @brief Find a txrx peer handle from a peer's local ID
 * @details
 *  The control SW typically uses the txrx peer handle to refer to the peer.
 *  In unusual circumstances, if it is infeasible for the control SW maintain
 *  the txrx peer handle but it can maintain a small integer local peer ID,
 *  this function allows the peer handled to be retrieved, based on the local
 *  peer ID.
 *
 * @param pdev - the data physical device object
 * @param local_peer_id - the ID txrx assigned locally to the peer in question
 * @return handle to the txrx peer object
 */
ol_txrx_peer_handle
ol_txrx_peer_find_by_local_id(struct cdp_pdev *ppdev,
			      uint8_t local_peer_id)
{
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	if ((local_peer_id == OL_TXRX_INVALID_LOCAL_PEER_ID) ||
	    (local_peer_id >= OL_TXRX_NUM_LOCAL_PEER_IDS)) {
		return NULL;
	}

	qdf_spin_lock_bh(&pdev->local_peer_ids.lock);
	peer = pdev->local_peer_ids.map[local_peer_id];
	qdf_spin_unlock_bh(&pdev->local_peer_ids.lock);
	return peer;
}

/**
 * @brief Find a txrx peer handle from a peer's local ID
 * @param pdev - the data physical device object
 * @param local_peer_id - the ID txrx assigned locally to the peer in question
 * @dbg_id - debug_id to track caller
 * @return handle to the txrx peer object
 * @details
 *  The control SW typically uses the txrx peer handle to refer to the peer.
 *  In unusual circumstances, if it is infeasible for the control SW maintain
 *  the txrx peer handle but it can maintain a small integer local peer ID,
 *  this function allows the peer handled to be retrieved, based on the local
 *  peer ID.
 *
 * Note that this function increments the peer->ref_cnt.
 * This makes sure that peer will be valid. This also means the caller needs to
 * call the corresponding API -
 *          ol_txrx_peer_release_ref
 *
 * reference.
 * Sample usage:
 *    {
 *      //the API call below increments the peer->ref_cnt
 *      peer = ol_txrx_peer_get_ref_by_local_id(pdev,local_peer_id, dbg_id);
 *
 *      // Once peer usage is done
 *
 *      //the API call below decrements the peer->ref_cnt
 *      ol_txrx_peer_release_ref(peer, dbg_id);
 *    }
 *
 * Return: peer handle if the peer is found, NULL if peer is not found.
 */
ol_txrx_peer_handle
ol_txrx_peer_get_ref_by_local_id(struct cdp_pdev *ppdev,
			      uint8_t local_peer_id,
			      enum peer_debug_id_type dbg_id)
{
	struct ol_txrx_peer_t *peer = NULL;
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	if ((local_peer_id == OL_TXRX_INVALID_LOCAL_PEER_ID) ||
	    (local_peer_id >= OL_TXRX_NUM_LOCAL_PEER_IDS)) {
		return NULL;
	}

	qdf_spin_lock_bh(&pdev->peer_ref_mutex);
	qdf_spin_lock_bh(&pdev->local_peer_ids.lock);
	peer = pdev->local_peer_ids.map[local_peer_id];
	qdf_spin_unlock_bh(&pdev->local_peer_ids.lock);
	if (peer && peer->valid)
		ol_txrx_peer_get_ref(peer, dbg_id);
	else
		peer = NULL;
	qdf_spin_unlock_bh(&pdev->peer_ref_mutex);

	return peer;
}

static void ol_txrx_local_peer_id_pool_init(struct ol_txrx_pdev_t *pdev)
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
}

static void
ol_txrx_local_peer_id_alloc(struct ol_txrx_pdev_t *pdev,
			    struct ol_txrx_peer_t *peer)
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
}

static void
ol_txrx_local_peer_id_free(struct ol_txrx_pdev_t *pdev,
			   struct ol_txrx_peer_t *peer)
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

static void ol_txrx_local_peer_id_cleanup(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_destroy(&pdev->local_peer_ids.lock);
}

#else
#define ol_txrx_local_peer_id_pool_init(pdev)   /* no-op */
#define ol_txrx_local_peer_id_alloc(pdev, peer) /* no-op */
#define ol_txrx_local_peer_id_free(pdev, peer)  /* no-op */
#define ol_txrx_local_peer_id_cleanup(pdev)     /* no-op */
#endif

#ifdef FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL

/**
 * ol_txrx_update_group_credit() - update group credit for tx queue
 * @group: for which credit needs to be updated
 * @credit: credits
 * @absolute: TXQ group absolute
 *
 * Return: allocated pool size
 */
void ol_txrx_update_group_credit(
		struct ol_tx_queue_group_t *group,
		int32_t credit,
		u_int8_t absolute)
{
	if (absolute)
		qdf_atomic_set(&group->credit, credit);
	else
		qdf_atomic_add(credit, &group->credit);
}

/**
 * ol_txrx_update_tx_queue_groups() - update vdev tx queue group if
 *				      vdev id mask and ac mask is not matching
 * @pdev: the data physical device
 * @group_id: TXQ group id
 * @credit: TXQ group credit count
 * @absolute: TXQ group absolute
 * @vdev_id_mask: TXQ vdev group id mask
 * @ac_mask: TQX access category mask
 *
 * Return: None
 */
void ol_txrx_update_tx_queue_groups(
		ol_txrx_pdev_handle pdev,
		u_int8_t group_id,
		int32_t credit,
		u_int8_t absolute,
		u_int32_t vdev_id_mask,
		u_int32_t ac_mask
		)
{
	struct ol_tx_queue_group_t *group;
	u_int32_t group_vdev_bit_mask, vdev_bit_mask, group_vdev_id_mask;
	u_int32_t membership;
	struct ol_txrx_vdev_t *vdev;

	if (group_id >= OL_TX_MAX_TXQ_GROUPS) {
		ol_txrx_warn("%s: invalid group_id=%u, ignore update.\n",
			__func__,
			group_id);
		return;
	}

	group = &pdev->txq_grps[group_id];

	membership = OL_TXQ_GROUP_MEMBERSHIP_GET(vdev_id_mask, ac_mask);

	qdf_spin_lock_bh(&pdev->tx_queue_spinlock);
	/*
	 * if the membership (vdev id mask and ac mask)
	 * matches then no need to update tx qeue groups.
	 */
	if (group->membership == membership)
		/* Update Credit Only */
		goto credit_update;


	/*
	 * membership (vdev id mask and ac mask) is not matching
	 * TODO: ignoring ac mask for now
	 */
	group_vdev_id_mask =
		OL_TXQ_GROUP_VDEV_ID_MASK_GET(group->membership);

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		group_vdev_bit_mask =
			OL_TXQ_GROUP_VDEV_ID_BIT_MASK_GET(
					group_vdev_id_mask, vdev->vdev_id);
		vdev_bit_mask =
			OL_TXQ_GROUP_VDEV_ID_BIT_MASK_GET(
					vdev_id_mask, vdev->vdev_id);

		if (group_vdev_bit_mask != vdev_bit_mask) {
			/*
			 * Change in vdev tx queue group
			 */
			if (!vdev_bit_mask) {
				/* Set Group Pointer (vdev and peer) to NULL */
				ol_tx_set_vdev_group_ptr(
						pdev, vdev->vdev_id, NULL);
			} else {
				/* Set Group Pointer (vdev and peer) */
				ol_tx_set_vdev_group_ptr(
						pdev, vdev->vdev_id, group);
			}
		}
	}
	/* Update membership */
	group->membership = membership;
credit_update:
	/* Update Credit */
	ol_txrx_update_group_credit(group, credit, absolute);
	qdf_spin_unlock_bh(&pdev->tx_queue_spinlock);
}
#endif

#ifdef WLAN_FEATURE_FASTPATH
/**
 * setup_fastpath_ce_handles() Update pdev with ce_handle for fastpath use.
 *
 * @osc: pointer to HIF context
 * @pdev: pointer to ol pdev
 *
 * Return: void
 */
static inline void setup_fastpath_ce_handles(struct hif_opaque_softc *osc,
						struct ol_txrx_pdev_t *pdev)
{
	/*
	 * Before the HTT attach, set up the CE handles
	 * CE handles are (struct CE_state *)
	 * This is only required in the fast path
	 */
	pdev->ce_tx_hdl = hif_get_ce_handle(osc, CE_HTT_H2T_MSG);

}

#else  /* not WLAN_FEATURE_FASTPATH */
static inline void setup_fastpath_ce_handles(struct hif_opaque_softc *osc,
						struct ol_txrx_pdev_t *pdev)
{
}
#endif /* WLAN_FEATURE_FASTPATH */

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * ol_tx_set_desc_global_pool_size() - set global pool size
 * @num_msdu_desc: total number of descriptors
 *
 * Return: none
 */
static void ol_tx_set_desc_global_pool_size(uint32_t num_msdu_desc)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		qdf_print("%s: pdev is NULL\n", __func__);
		return;
	}
	pdev->num_msdu_desc = num_msdu_desc;
	if (!ol_tx_get_is_mgmt_over_wmi_enabled())
		pdev->num_msdu_desc += TX_FLOW_MGMT_POOL_SIZE;
	ol_txrx_info_high("Global pool size: %d\n",
		pdev->num_msdu_desc);
}

/**
 * ol_tx_get_desc_global_pool_size() - get global pool size
 * @pdev: pdev handle
 *
 * Return: global pool size
 */
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return pdev->num_msdu_desc;
}

/**
 * ol_tx_get_total_free_desc() - get total free descriptors
 * @pdev: pdev handle
 *
 * Return: total free descriptors
 */
static inline
uint32_t ol_tx_get_total_free_desc(struct ol_txrx_pdev_t *pdev)
{
	struct ol_tx_flow_pool_t *pool = NULL;
	uint32_t free_desc;

	free_desc = pdev->tx_desc.num_free;
	qdf_spin_lock_bh(&pdev->tx_desc.flow_pool_list_lock);
	TAILQ_FOREACH(pool, &pdev->tx_desc.flow_pool_list,
					 flow_pool_list_elem) {
		qdf_spin_lock_bh(&pool->flow_pool_lock);
		free_desc += pool->avail_desc;
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
	}
	qdf_spin_unlock_bh(&pdev->tx_desc.flow_pool_list_lock);

	return free_desc;
}

#else
/**
 * ol_tx_get_desc_global_pool_size() - get global pool size
 * @pdev: pdev handle
 *
 * Return: global pool size
 */
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return ol_cfg_target_tx_credit(pdev->ctrl_pdev);
}

/**
 * ol_tx_get_total_free_desc() - get total free descriptors
 * @pdev: pdev handle
 *
 * Return: total free descriptors
 */
static inline
uint32_t ol_tx_get_total_free_desc(struct ol_txrx_pdev_t *pdev)
{
	return pdev->tx_desc.num_free;
}

#endif

#if defined(CONFIG_HL_SUPPORT) && defined(CONFIG_PER_VDEV_TX_DESC_POOL)

/**
 * ol_txrx_rsrc_threshold_lo() - set threshold low - when to start tx desc
 *				 margin replenishment
 * @desc_pool_size: tx desc pool size
 *
 * Return: threshold low
 */
static inline uint16_t
ol_txrx_rsrc_threshold_lo(int desc_pool_size)
{
	int threshold_low;

	/*
	 * 5% margin of unallocated desc is too much for per
	 * vdev mechanism.
	 * Define the value seperately.
	 */
	threshold_low = TXRX_HL_TX_FLOW_CTRL_MGMT_RESERVED;

	return threshold_low;
}

/**
 * ol_txrx_rsrc_threshold_hi() - set threshold high - where to stop
 *				 during tx desc margin replenishment
 * @desc_pool_size: tx desc pool size
 *
 * Return: threshold high
 */
static inline uint16_t
ol_txrx_rsrc_threshold_hi(int desc_pool_size)
{
	int threshold_high;
	/* when freeing up descriptors,
	 * keep going until there's a 7.5% margin
	 */
	threshold_high = ((15 * desc_pool_size)/100)/2;

	return threshold_high;
}
#else

static inline uint16_t
ol_txrx_rsrc_threshold_lo(int desc_pool_size)
{
	int threshold_low;
	/* always maintain a 5% margin of unallocated descriptors */
	threshold_low = (5 * desc_pool_size)/100;

	return threshold_low;
}

static inline uint16_t
ol_txrx_rsrc_threshold_hi(int desc_pool_size)
{
	int threshold_high;
	/* when freeing up descriptors, keep going until
	 * there's a 15% margin
	 */
	threshold_high = (15 * desc_pool_size)/100;

	return threshold_high;
}
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(DEBUG_HL_LOGGING)

/**
 * ol_txrx_pdev_txq_log_init() - initialise pdev txq logs
 * @pdev: the physical device object
 *
 * Return: None
 */
static void
ol_txrx_pdev_txq_log_init(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_create(&pdev->txq_log_spinlock);
	pdev->txq_log.size = OL_TXQ_LOG_SIZE;
	pdev->txq_log.oldest_record_offset = 0;
	pdev->txq_log.offset = 0;
	pdev->txq_log.allow_wrap = 1;
	pdev->txq_log.wrapped = 0;
}

/**
 * ol_txrx_pdev_txq_log_destroy() - remove txq log spinlock for pdev
 * @pdev: the physical device object
 *
 * Return: None
 */
static inline void
ol_txrx_pdev_txq_log_destroy(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_destroy(&pdev->txq_log_spinlock);
}

#else

static inline void
ol_txrx_pdev_txq_log_init(struct ol_txrx_pdev_t *pdev)
{
}

static inline void
ol_txrx_pdev_txq_log_destroy(struct ol_txrx_pdev_t *pdev)
{
}


#endif

#if defined(DEBUG_HL_LOGGING)

/**
 * ol_txrx_pdev_grp_stats_init() - initialise group stat spinlock for pdev
 * @pdev: the physical device object
 *
 * Return: None
 */
static inline void
ol_txrx_pdev_grp_stats_init(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_create(&pdev->grp_stat_spinlock);
	pdev->grp_stats.last_valid_index = -1;
	pdev->grp_stats.wrap_around = 0;
}

/**
 * ol_txrx_pdev_grp_stat_destroy() - destroy group stat spinlock for pdev
 * @pdev: the physical device object
 *
 * Return: None
 */
static inline void
ol_txrx_pdev_grp_stat_destroy(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_destroy(&pdev->grp_stat_spinlock);
}
#else

static inline void
ol_txrx_pdev_grp_stats_init(struct ol_txrx_pdev_t *pdev)
{
}

static inline void
ol_txrx_pdev_grp_stat_destroy(struct ol_txrx_pdev_t *pdev)
{
}
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)

/**
 * ol_txrx_hl_tdls_flag_reset() - reset tdls flag for vdev
 * @vdev: the virtual device object
 * @flag: flag
 *
 * Return: None
 */
void
ol_txrx_hl_tdls_flag_reset(struct cdp_vdev *pvdev, bool flag)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	vdev->hlTdlsFlag = flag;
}
#endif

#if defined(CONFIG_HL_SUPPORT)

/**
 * ol_txrx_vdev_txqs_init() - initialise vdev tx queues
 * @vdev: the virtual device object
 *
 * Return: None
 */
static void
ol_txrx_vdev_txqs_init(struct ol_txrx_vdev_t *vdev)
{
	u_int8_t i;

	for (i = 0; i < OL_TX_VDEV_NUM_QUEUES; i++) {
		TAILQ_INIT(&vdev->txqs[i].head);
		vdev->txqs[i].paused_count.total = 0;
		vdev->txqs[i].frms = 0;
		vdev->txqs[i].bytes = 0;
		vdev->txqs[i].ext_tid = OL_TX_NUM_TIDS + i;
		vdev->txqs[i].flag = ol_tx_queue_empty;
		/* aggregation is not applicable for vdev tx queues */
		vdev->txqs[i].aggr_state = ol_tx_aggr_disabled;
		ol_tx_txq_set_group_ptr(&vdev->txqs[i], NULL);
		ol_txrx_set_txq_peer(&vdev->txqs[i], NULL);
	}
}

/**
 * ol_txrx_vdev_tx_queue_free() - free vdev tx queues
 * @vdev: the virtual device object
 *
 * Return: None
 */
static void
ol_txrx_vdev_tx_queue_free(struct ol_txrx_vdev_t *vdev)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	struct ol_tx_frms_queue_t *txq;
	int i;

	for (i = 0; i < OL_TX_VDEV_NUM_QUEUES; i++) {
		txq = &vdev->txqs[i];
		ol_tx_queue_free(pdev, txq, (i + OL_TX_NUM_TIDS), false);
	}
}

/**
 * ol_txrx_peer_txqs_init() - initialise peer tx queues
 * @pdev: the physical device object
 * @peer: peer object
 *
 * Return: None
 */
static void
ol_txrx_peer_txqs_init(struct ol_txrx_pdev_t *pdev,
		       struct ol_txrx_peer_t *peer)
{
	uint8_t i;
	struct ol_txrx_vdev_t *vdev = peer->vdev;

	qdf_spin_lock_bh(&pdev->tx_queue_spinlock);
	for (i = 0; i < OL_TX_NUM_TIDS; i++) {
		TAILQ_INIT(&peer->txqs[i].head);
		peer->txqs[i].paused_count.total = 0;
		peer->txqs[i].frms = 0;
		peer->txqs[i].bytes = 0;
		peer->txqs[i].ext_tid = i;
		peer->txqs[i].flag = ol_tx_queue_empty;
		peer->txqs[i].aggr_state = ol_tx_aggr_untried;
		ol_tx_set_peer_group_ptr(pdev, peer, vdev->vdev_id, i);
		ol_txrx_set_txq_peer(&peer->txqs[i], peer);
	}
	qdf_spin_unlock_bh(&pdev->tx_queue_spinlock);

	/* aggregation is not applicable for mgmt and non-QoS tx queues */
	for (i = OL_TX_NUM_QOS_TIDS; i < OL_TX_NUM_TIDS; i++)
		peer->txqs[i].aggr_state = ol_tx_aggr_disabled;

	ol_txrx_peer_pause(peer);
}

/**
 * ol_txrx_peer_tx_queue_free() - free peer tx queues
 * @pdev: the physical device object
 * @peer: peer object
 *
 * Return: None
 */
static void
ol_txrx_peer_tx_queue_free(struct ol_txrx_pdev_t *pdev,
			   struct ol_txrx_peer_t *peer)
{
	struct ol_tx_frms_queue_t *txq;
	uint8_t i;

	for (i = 0; i < OL_TX_NUM_TIDS; i++) {
		txq = &peer->txqs[i];
		ol_tx_queue_free(pdev, txq, i, true);
	}
}
#else

static inline void
ol_txrx_vdev_txqs_init(struct ol_txrx_vdev_t *vdev)
{
}

static inline void
ol_txrx_vdev_tx_queue_free(struct ol_txrx_vdev_t *vdev)
{
}

static inline void
ol_txrx_peer_txqs_init(struct ol_txrx_pdev_t *pdev,
		       struct ol_txrx_peer_t *peer)
{
}

static inline void
ol_txrx_peer_tx_queue_free(struct ol_txrx_pdev_t *pdev,
			   struct ol_txrx_peer_t *peer)
{
}
#endif

#if defined(FEATURE_TSO) && defined(FEATURE_TSO_DEBUG)
static void ol_txrx_tso_stats_init(ol_txrx_pdev_handle pdev)
{
	qdf_spinlock_create(&pdev->stats.pub.tx.tso.tso_stats_lock);
}

static void ol_txrx_tso_stats_deinit(ol_txrx_pdev_handle pdev)
{
	qdf_spinlock_destroy(&pdev->stats.pub.tx.tso.tso_stats_lock);
}

static void ol_txrx_stats_display_tso(ol_txrx_pdev_handle pdev)
{
	int msdu_idx;
	int seg_idx;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "TSO Statistics:");
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "TSO pkts %lld, bytes %lld\n",
		 pdev->stats.pub.tx.tso.tso_pkts.pkts,
		 pdev->stats.pub.tx.tso.tso_pkts.bytes);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "TSO Histogram for numbers of segments:\n"
		 "Single segment	%d\n"
		 "  2-5 segments	%d\n"
		 " 6-10 segments	%d\n"
		 "11-15 segments	%d\n"
		 "16-20 segments	%d\n"
		 "  20+ segments	%d\n",
		 pdev->stats.pub.tx.tso.tso_hist.pkts_1,
		 pdev->stats.pub.tx.tso.tso_hist.pkts_2_5,
		 pdev->stats.pub.tx.tso.tso_hist.pkts_6_10,
		 pdev->stats.pub.tx.tso.tso_hist.pkts_11_15,
		 pdev->stats.pub.tx.tso.tso_hist.pkts_16_20,
		 pdev->stats.pub.tx.tso.tso_hist.pkts_20_plus);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "TSO History Buffer: Total size %d, current_index %d",
		 NUM_MAX_TSO_MSDUS,
		 TXRX_STATS_TSO_MSDU_IDX(pdev));

	for (msdu_idx = 0; msdu_idx < NUM_MAX_TSO_MSDUS; msdu_idx++) {
		if (TXRX_STATS_TSO_MSDU_TOTAL_LEN(pdev, msdu_idx) == 0)
			continue;
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
			  "jumbo pkt idx: %d num segs %d gso_len %d total_len %d nr_frags %d",
			 msdu_idx,
			 TXRX_STATS_TSO_MSDU_NUM_SEG(pdev, msdu_idx),
			 TXRX_STATS_TSO_MSDU_GSO_SIZE(pdev, msdu_idx),
			 TXRX_STATS_TSO_MSDU_TOTAL_LEN(pdev, msdu_idx),
			 TXRX_STATS_TSO_MSDU_NR_FRAGS(pdev, msdu_idx));

		for (seg_idx = 0;
			 ((seg_idx < TXRX_STATS_TSO_MSDU_NUM_SEG(pdev,
			   msdu_idx)) && (seg_idx < NUM_MAX_TSO_SEGS));
			 seg_idx++) {
			struct qdf_tso_seg_t tso_seg =
				 TXRX_STATS_TSO_SEG(pdev, msdu_idx, seg_idx);

			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
				  "seg idx: %d", seg_idx);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
				  "tso_enable: %d",
				  tso_seg.tso_flags.tso_enable);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
				  "fin %d syn %d rst %d psh %d ack %d urg %d ece %d cwr %d ns %d",
				  tso_seg.tso_flags.fin, tso_seg.tso_flags.syn,
				  tso_seg.tso_flags.rst, tso_seg.tso_flags.psh,
				  tso_seg.tso_flags.ack, tso_seg.tso_flags.urg,
				  tso_seg.tso_flags.ece, tso_seg.tso_flags.cwr,
				  tso_seg.tso_flags.ns);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
				  "tcp_seq_num: 0x%x ip_id: %d",
				  tso_seg.tso_flags.tcp_seq_num,
				  tso_seg.tso_flags.ip_id);
		}
	}
}

static void ol_txrx_tso_stats_clear(ol_txrx_pdev_handle pdev)
{
	qdf_mem_zero(&pdev->stats.pub.tx.tso.tso_pkts,
		     sizeof(struct ol_txrx_stats_elem));
#if defined(FEATURE_TSO)
	qdf_mem_zero(&pdev->stats.pub.tx.tso.tso_info,
		     sizeof(struct ol_txrx_stats_tso_info));
	qdf_mem_zero(&pdev->stats.pub.tx.tso.tso_hist,
		     sizeof(struct ol_txrx_tso_histogram));
#endif
}

#else

static void ol_txrx_stats_display_tso(ol_txrx_pdev_handle pdev)
{
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
	 "TSO is not supported\n");
}

static void ol_txrx_tso_stats_init(ol_txrx_pdev_handle pdev)
{
	/*
	 * keeping the body empty and not keeping an error print as print will
	 * will show up everytime during driver load if TSO is not enabled.
	 */
}

static void ol_txrx_tso_stats_deinit(ol_txrx_pdev_handle pdev)
{
	/*
	 * keeping the body empty and not keeping an error print as print will
	 * will show up everytime during driver unload if TSO is not enabled.
	 */
}

static void ol_txrx_tso_stats_clear(ol_txrx_pdev_handle pdev)
{
	/*
	 * keeping the body empty and not keeping an error print as print will
	 * will show up everytime during driver unload if TSO is not enabled.
	 */
}
#endif /* defined(FEATURE_TSO) && defined(FEATURE_TSO_DEBUG) */

/**
 * ol_txrx_pdev_attach() - allocate txrx pdev
 * @ctrl_pdev: cfg pdev
 * @htc_pdev: HTC pdev
 * @osdev: os dev
 *
 * Return: txrx pdev handle
 *		  NULL for failure
 */
static struct cdp_pdev *
ol_txrx_pdev_attach(ol_txrx_soc_handle soc, struct cdp_cfg *ctrl_pdev,
		    HTC_HANDLE htc_pdev, qdf_device_t osdev, uint8_t pdev_id)
{
	struct ol_txrx_pdev_t *pdev;
	int i, tid;

	pdev = qdf_mem_malloc(sizeof(*pdev));
	if (!pdev)
		goto fail0;

	/* init LL/HL cfg here */
	pdev->cfg.is_high_latency = ol_cfg_is_high_latency(ctrl_pdev);
	pdev->cfg.default_tx_comp_req = !ol_cfg_tx_free_at_download(ctrl_pdev);

	/* store provided params */
	pdev->ctrl_pdev = ctrl_pdev;
	pdev->osdev = osdev;

	for (i = 0; i < htt_num_sec_types; i++)
		pdev->sec_types[i] = (enum ol_sec_type)i;

	TXRX_STATS_INIT(pdev);
	ol_txrx_tso_stats_init(pdev);

	TAILQ_INIT(&pdev->vdev_list);

	TAILQ_INIT(&pdev->req_list);
	pdev->req_list_depth = 0;
	qdf_spinlock_create(&pdev->req_list_spinlock);

	/* do initial set up of the peer ID -> peer object lookup map */
	if (ol_txrx_peer_find_attach(pdev))
		goto fail1;

	/* initialize the counter of the target's tx buffer availability */
	qdf_atomic_init(&pdev->target_tx_credit);
	qdf_atomic_init(&pdev->orig_target_tx_credit);

	if (ol_cfg_is_high_latency(ctrl_pdev)) {
		qdf_spinlock_create(&pdev->tx_queue_spinlock);
		pdev->tx_sched.scheduler = ol_tx_sched_attach(pdev);
		if (pdev->tx_sched.scheduler == NULL)
			goto fail2;
	}
	ol_txrx_pdev_txq_log_init(pdev);
	ol_txrx_pdev_grp_stats_init(pdev);

	pdev->htt_pdev =
		htt_pdev_alloc(pdev, ctrl_pdev, htc_pdev, osdev);
	if (!pdev->htt_pdev)
		goto fail3;

	htt_register_rx_pkt_dump_callback(pdev->htt_pdev,
			ol_rx_pkt_dump_call);

	/*
	 * Init the tid --> category table.
	 * Regular tids (0-15) map to their AC.
	 * Extension tids get their own categories.
	 */
	for (tid = 0; tid < OL_TX_NUM_QOS_TIDS; tid++) {
		int ac = TXRX_TID_TO_WMM_AC(tid);

		pdev->tid_to_ac[tid] = ac;
	}
	pdev->tid_to_ac[OL_TX_NON_QOS_TID] =
		OL_TX_SCHED_WRR_ADV_CAT_NON_QOS_DATA;
	pdev->tid_to_ac[OL_TX_MGMT_TID] =
		OL_TX_SCHED_WRR_ADV_CAT_UCAST_MGMT;
	pdev->tid_to_ac[OL_TX_NUM_TIDS + OL_TX_VDEV_MCAST_BCAST] =
		OL_TX_SCHED_WRR_ADV_CAT_MCAST_DATA;
	pdev->tid_to_ac[OL_TX_NUM_TIDS + OL_TX_VDEV_DEFAULT_MGMT] =
		OL_TX_SCHED_WRR_ADV_CAT_MCAST_MGMT;

	return (struct cdp_pdev *)pdev;

fail3:
	ol_txrx_peer_find_detach(pdev);

fail2:
	if (ol_cfg_is_high_latency(ctrl_pdev))
		qdf_spinlock_destroy(&pdev->tx_queue_spinlock);

fail1:
	ol_txrx_tso_stats_deinit(pdev);
	qdf_mem_free(pdev);

fail0:
	return NULL;
}

#if !defined(REMOVE_PKT_LOG) && !defined(QVIT)
/**
 * htt_pkt_log_init() - API to initialize packet log
 * @handle: pdev handle
 * @scn: HIF context
 *
 * Return: void
 */
void htt_pkt_log_init(struct cdp_pdev *ppdev, void *scn)
{
	struct ol_txrx_pdev_t *handle = (struct ol_txrx_pdev_t *)ppdev;

	if (handle->pkt_log_init)
		return;

	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE &&
			!QDF_IS_EPPING_ENABLED(cds_get_conparam())) {
		pktlog_sethandle(&handle->pl_dev, scn);
		pktlog_set_callback_regtype(PKTLOG_DEFAULT_CALLBACK_REGISTRATION);
		if (pktlogmod_init(scn))
			qdf_print("%s: pktlogmod_init failed", __func__);
		else
			handle->pkt_log_init = true;
	}
}

/**
 * htt_pktlogmod_exit() - API to cleanup pktlog info
 * @handle: Pdev handle
 * @scn: HIF Context
 *
 * Return: void
 */
static void htt_pktlogmod_exit(struct ol_txrx_pdev_t *handle)
{
	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE &&
		!QDF_IS_EPPING_ENABLED(cds_get_conparam()) &&
			handle->pkt_log_init) {
		pktlogmod_exit(handle);
		handle->pkt_log_init = false;
	}
}

#else
void htt_pkt_log_init(struct cdp_pdev *pdev_handle, void *ol_sc) { }
static void htt_pktlogmod_exit(ol_txrx_pdev_handle handle)  { }
#endif

/**
 * ol_txrx_pdev_post_attach() - attach txrx pdev
 * @pdev: txrx pdev
 *
 * Return: 0 for success
 */
int
ol_txrx_pdev_post_attach(struct cdp_pdev *ppdev)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	uint16_t i;
	uint16_t fail_idx = 0;
	int ret = 0;
	uint16_t desc_pool_size;
	struct hif_opaque_softc *osc =  cds_get_context(QDF_MODULE_ID_HIF);

	uint16_t desc_element_size = sizeof(union ol_tx_desc_list_elem_t);
	union ol_tx_desc_list_elem_t *c_element;
	unsigned int sig_bit;
	uint16_t desc_per_page;

	if (!osc) {
		ret = -EINVAL;
		goto ol_attach_fail;
	}

	/*
	 * For LL, limit the number of host's tx descriptors to match
	 * the number of target FW tx descriptors.
	 * This simplifies the FW, by ensuring the host will never
	 * download more tx descriptors than the target has space for.
	 * The FW will drop/free low-priority tx descriptors when it
	 * starts to run low, so that in theory the host should never
	 * run out of tx descriptors.
	 */

	/*
	 * LL - initialize the target credit outselves.
	 * HL - wait for a HTT target credit initialization
	 * during htt_attach.
	 */
	if (pdev->cfg.is_high_latency) {
		desc_pool_size = ol_tx_desc_pool_size_hl(pdev->ctrl_pdev);

		qdf_atomic_init(&pdev->tx_queue.rsrc_cnt);
		qdf_atomic_add(desc_pool_size, &pdev->tx_queue.rsrc_cnt);

		pdev->tx_queue.rsrc_threshold_lo =
			ol_txrx_rsrc_threshold_lo(desc_pool_size);
		pdev->tx_queue.rsrc_threshold_hi =
			ol_txrx_rsrc_threshold_hi(desc_pool_size);

		for (i = 0 ; i < OL_TX_MAX_TXQ_GROUPS; i++)
			qdf_atomic_init(&pdev->txq_grps[i].credit);

		ol_tx_target_credit_init(pdev, desc_pool_size);
	} else {
		qdf_atomic_add(ol_cfg_target_tx_credit(pdev->ctrl_pdev),
			       &pdev->target_tx_credit);
		desc_pool_size = ol_tx_get_desc_global_pool_size(pdev);
	}

	ol_tx_desc_dup_detect_init(pdev, desc_pool_size);

	setup_fastpath_ce_handles(osc, pdev);

	ret = htt_attach(pdev->htt_pdev, desc_pool_size);
	if (ret)
		goto htt_attach_fail;

	/* Attach micro controller data path offload resource */
	if (ol_cfg_ipa_uc_offload_enabled(pdev->ctrl_pdev)) {
		ret = htt_ipa_uc_attach(pdev->htt_pdev);
		if (ret)
			goto uc_attach_fail;
	}

	/* Calculate single element reserved size power of 2 */
	pdev->tx_desc.desc_reserved_size = qdf_get_pwr2(desc_element_size);
	qdf_mem_multi_pages_alloc(pdev->osdev, &pdev->tx_desc.desc_pages,
		pdev->tx_desc.desc_reserved_size, desc_pool_size, 0, true);
	if ((0 == pdev->tx_desc.desc_pages.num_pages) ||
		(NULL == pdev->tx_desc.desc_pages.cacheable_pages)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Page alloc fail");
		ret = -ENOMEM;
		goto page_alloc_fail;
	}
	desc_per_page = pdev->tx_desc.desc_pages.num_element_per_page;
	pdev->tx_desc.offset_filter = desc_per_page - 1;
	/* Calculate page divider to find page number */
	sig_bit = 0;
	while (desc_per_page) {
		sig_bit++;
		desc_per_page = desc_per_page >> 1;
	}
	pdev->tx_desc.page_divider = (sig_bit - 1);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		"page_divider 0x%x, offset_filter 0x%x num elem %d, ol desc num page %d, ol desc per page %d",
		pdev->tx_desc.page_divider, pdev->tx_desc.offset_filter,
		desc_pool_size, pdev->tx_desc.desc_pages.num_pages,
		pdev->tx_desc.desc_pages.num_element_per_page);

	/*
	 * Each SW tx desc (used only within the tx datapath SW) has a
	 * matching HTT tx desc (used for downloading tx meta-data to FW/HW).
	 * Go ahead and allocate the HTT tx desc and link it with the SW tx
	 * desc now, to avoid doing it during time-critical transmit.
	 */
	pdev->tx_desc.pool_size = desc_pool_size;
	pdev->tx_desc.freelist =
		(union ol_tx_desc_list_elem_t *)
		(*pdev->tx_desc.desc_pages.cacheable_pages);
	c_element = pdev->tx_desc.freelist;
	for (i = 0; i < desc_pool_size; i++) {
		void *htt_tx_desc;
		void *htt_frag_desc = NULL;
		qdf_dma_addr_t frag_paddr = 0;
		qdf_dma_addr_t paddr;

		if (i == (desc_pool_size - 1))
			c_element->next = NULL;
		else
			c_element->next = (union ol_tx_desc_list_elem_t *)
				ol_tx_desc_find(pdev, i + 1);

		htt_tx_desc = htt_tx_desc_alloc(pdev->htt_pdev, &paddr, i);
		if (!htt_tx_desc) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
				  "%s: failed to alloc HTT tx desc (%d of %d)",
				__func__, i, desc_pool_size);
			fail_idx = i;
			ret = -ENOMEM;
			goto desc_alloc_fail;
		}

		c_element->tx_desc.htt_tx_desc = htt_tx_desc;
		c_element->tx_desc.htt_tx_desc_paddr = paddr;
		ret = htt_tx_frag_alloc(pdev->htt_pdev,
					i, &frag_paddr, &htt_frag_desc);
		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: failed to alloc HTT frag dsc (%d/%d)",
				__func__, i, desc_pool_size);
			/* Is there a leak here, is this handling correct? */
			fail_idx = i;
			goto desc_alloc_fail;
		}
		if (!ret && htt_frag_desc) {
			/*
			 * Initialize the first 6 words (TSO flags)
			 * of the frag descriptor
			 */
			memset(htt_frag_desc, 0, 6 * sizeof(uint32_t));
			c_element->tx_desc.htt_frag_desc = htt_frag_desc;
			c_element->tx_desc.htt_frag_desc_paddr = frag_paddr;
		}
#ifdef QCA_SUPPORT_TXDESC_SANITY_CHECKS
		c_element->tx_desc.pkt_type = 0xff;
#ifdef QCA_COMPUTE_TX_DELAY
		c_element->tx_desc.entry_timestamp_ticks =
			0xffffffff;
#endif
#endif
		c_element->tx_desc.id = i;
		qdf_atomic_init(&c_element->tx_desc.ref_cnt);
		c_element = c_element->next;
		fail_idx = i;
	}

	/* link SW tx descs into a freelist */
	pdev->tx_desc.num_free = desc_pool_size;
	ol_txrx_dbg(
		   "%s first tx_desc:0x%pK Last tx desc:0x%pK\n", __func__,
		   (uint32_t *) pdev->tx_desc.freelist,
		   (uint32_t *) (pdev->tx_desc.freelist + desc_pool_size));

	/* check what format of frames are expected to be delivered by the OS */
	pdev->frame_format = ol_cfg_frame_type(pdev->ctrl_pdev);
	if (pdev->frame_format == wlan_frm_fmt_native_wifi)
		pdev->htt_pkt_type = htt_pkt_type_native_wifi;
	else if (pdev->frame_format == wlan_frm_fmt_802_3) {
		if (ol_cfg_is_ce_classify_enabled(pdev->ctrl_pdev))
			pdev->htt_pkt_type = htt_pkt_type_eth2;
		else
			pdev->htt_pkt_type = htt_pkt_type_ethernet;
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s Invalid standard frame type: %d",
			  __func__, pdev->frame_format);
		ret = -EINVAL;
		goto control_init_fail;
	}

	/* setup the global rx defrag waitlist */
	TAILQ_INIT(&pdev->rx.defrag.waitlist);

	/* configure where defrag timeout and duplicate detection is handled */
	pdev->rx.flags.defrag_timeout_check =
		pdev->rx.flags.dup_check =
		ol_cfg_rx_host_defrag_timeout_duplicate_check(pdev->ctrl_pdev);

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
	/* Need to revisit this part. Currently,hardcode to riva's caps */
	pdev->target_tx_tran_caps = wlan_frm_tran_cap_raw;
	pdev->target_rx_tran_caps = wlan_frm_tran_cap_raw;
	/*
	 * The Riva HW de-aggregate doesn't have capability to generate 802.11
	 * header for non-first subframe of A-MSDU.
	 */
	pdev->sw_subfrm_hdr_recovery_enable = 1;
	/*
	 * The Riva HW doesn't have the capability to set Protected Frame bit
	 * in the MAC header for encrypted data frame.
	 */
	pdev->sw_pf_proc_enable = 1;

	if (pdev->frame_format == wlan_frm_fmt_802_3) {
		/*
		 * sw llc process is only needed in
		 * 802.3 to 802.11 transform case
		 */
		pdev->sw_tx_llc_proc_enable = 1;
		pdev->sw_rx_llc_proc_enable = 1;
	} else {
		pdev->sw_tx_llc_proc_enable = 0;
		pdev->sw_rx_llc_proc_enable = 0;
	}

	switch (pdev->frame_format) {
	case wlan_frm_fmt_raw:
		pdev->sw_tx_encap =
			pdev->target_tx_tran_caps & wlan_frm_tran_cap_raw
			? 0 : 1;
		pdev->sw_rx_decap =
			pdev->target_rx_tran_caps & wlan_frm_tran_cap_raw
			? 0 : 1;
		break;
	case wlan_frm_fmt_native_wifi:
		pdev->sw_tx_encap =
			pdev->
			target_tx_tran_caps & wlan_frm_tran_cap_native_wifi
			? 0 : 1;
		pdev->sw_rx_decap =
			pdev->
			target_rx_tran_caps & wlan_frm_tran_cap_native_wifi
			? 0 : 1;
		break;
	case wlan_frm_fmt_802_3:
		pdev->sw_tx_encap =
			pdev->target_tx_tran_caps & wlan_frm_tran_cap_8023
			? 0 : 1;
		pdev->sw_rx_decap =
			pdev->target_rx_tran_caps & wlan_frm_tran_cap_8023
			? 0 : 1;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Invalid std frame type; [en/de]cap: f:%x t:%x r:%x",
			  pdev->frame_format,
			  pdev->target_tx_tran_caps, pdev->target_rx_tran_caps);
		ret = -EINVAL;
		goto control_init_fail;
	}
#endif

	/*
	 * Determine what rx processing steps are done within the host.
	 * Possibilities:
	 * 1.  Nothing - rx->tx forwarding and rx PN entirely within target.
	 *     (This is unlikely; even if the target is doing rx->tx forwarding,
	 *     the host should be doing rx->tx forwarding too, as a back up for
	 *     the target's rx->tx forwarding, in case the target runs short on
	 *     memory, and can't store rx->tx frames that are waiting for
	 *     missing prior rx frames to arrive.)
	 * 2.  Just rx -> tx forwarding.
	 *     This is the typical configuration for HL, and a likely
	 *     configuration for LL STA or small APs (e.g. retail APs).
	 * 3.  Both PN check and rx -> tx forwarding.
	 *     This is the typical configuration for large LL APs.
	 * Host-side PN check without rx->tx forwarding is not a valid
	 * configuration, since the PN check needs to be done prior to
	 * the rx->tx forwarding.
	 */
	if (ol_cfg_is_full_reorder_offload(pdev->ctrl_pdev)) {
		/*
		 * PN check, rx-tx forwarding and rx reorder is done by
		 * the target
		 */
		if (ol_cfg_rx_fwd_disabled(pdev->ctrl_pdev))
			pdev->rx_opt_proc = ol_rx_in_order_deliver;
		else
			pdev->rx_opt_proc = ol_rx_fwd_check;
	} else {
		if (ol_cfg_rx_pn_check(pdev->ctrl_pdev)) {
			if (ol_cfg_rx_fwd_disabled(pdev->ctrl_pdev)) {
				/*
				 * PN check done on host,
				 * rx->tx forwarding not done at all.
				 */
				pdev->rx_opt_proc = ol_rx_pn_check_only;
			} else if (ol_cfg_rx_fwd_check(pdev->ctrl_pdev)) {
				/*
				 * Both PN check and rx->tx forwarding done
				 * on host.
				 */
				pdev->rx_opt_proc = ol_rx_pn_check;
			} else {
#define TRACESTR01 "invalid config: if rx PN check is on the host,"\
"rx->tx forwarding check needs to also be on the host"
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: %s", __func__, TRACESTR01);
#undef TRACESTR01
				ret = -EINVAL;
				goto control_init_fail;
			}
		} else {
			/* PN check done on target */
			if ((!ol_cfg_rx_fwd_disabled(pdev->ctrl_pdev)) &&
			    ol_cfg_rx_fwd_check(pdev->ctrl_pdev)) {
				/*
				 * rx->tx forwarding done on host (possibly as
				 * back-up for target-side primary rx->tx
				 * forwarding)
				 */
				pdev->rx_opt_proc = ol_rx_fwd_check;
			} else {
				/*
				 * rx->tx forwarding either done in target,
				 * or not done at all
				 */
				pdev->rx_opt_proc = ol_rx_deliver;
			}
		}
	}

	/* initialize mutexes for tx desc alloc and peer lookup */
	qdf_spinlock_create(&pdev->tx_mutex);
	qdf_spinlock_create(&pdev->peer_ref_mutex);
	qdf_spinlock_create(&pdev->rx.mutex);
	qdf_spinlock_create(&pdev->last_real_peer_mutex);
	qdf_spinlock_create(&pdev->peer_map_unmap_lock);
	OL_TXRX_PEER_STATS_MUTEX_INIT(pdev);

	if (OL_RX_REORDER_TRACE_ATTACH(pdev) != A_OK) {
		ret = -ENOMEM;
		goto reorder_trace_attach_fail;
	}

	if (OL_RX_PN_TRACE_ATTACH(pdev) != A_OK) {
		ret = -ENOMEM;
		goto pn_trace_attach_fail;
	}

#ifdef PERE_IP_HDR_ALIGNMENT_WAR
	pdev->host_80211_enable = ol_scn_host_80211_enable_get(pdev->ctrl_pdev);
#endif

	/*
	 * WDI event attach
	 */
	wdi_event_attach(pdev);

	/*
	 * Initialize rx PN check characteristics for different security types.
	 */
	qdf_mem_set(&pdev->rx_pn[0], sizeof(pdev->rx_pn), 0);

	/* TKIP: 48-bit TSC, CCMP: 48-bit PN */
	pdev->rx_pn[htt_sec_type_tkip].len =
		pdev->rx_pn[htt_sec_type_tkip_nomic].len =
			pdev->rx_pn[htt_sec_type_aes_ccmp].len = 48;
	pdev->rx_pn[htt_sec_type_tkip].cmp =
		pdev->rx_pn[htt_sec_type_tkip_nomic].cmp =
			pdev->rx_pn[htt_sec_type_aes_ccmp].cmp = ol_rx_pn_cmp48;

	/* WAPI: 128-bit PN */
	pdev->rx_pn[htt_sec_type_wapi].len = 128;
	pdev->rx_pn[htt_sec_type_wapi].cmp = ol_rx_pn_wapi_cmp;

	OL_RX_REORDER_TIMEOUT_INIT(pdev);

	ol_txrx_dbg("Created pdev %pK\n", pdev);

	pdev->cfg.host_addba = ol_cfg_host_addba(pdev->ctrl_pdev);

#ifdef QCA_SUPPORT_PEER_DATA_RX_RSSI
#define OL_TXRX_RSSI_UPDATE_SHIFT_DEFAULT 3

/* #if 1 -- TODO: clean this up */
#define OL_TXRX_RSSI_NEW_WEIGHT_DEFAULT	\
	/* avg = 100% * new + 0% * old */ \
	(1 << OL_TXRX_RSSI_UPDATE_SHIFT_DEFAULT)
/*
 * #else
 * #define OL_TXRX_RSSI_NEW_WEIGHT_DEFAULT
 *	//avg = 25% * new + 25% * old
 *	(1 << (OL_TXRX_RSSI_UPDATE_SHIFT_DEFAULT-2))
 * #endif
 */
	pdev->rssi_update_shift = OL_TXRX_RSSI_UPDATE_SHIFT_DEFAULT;
	pdev->rssi_new_weight = OL_TXRX_RSSI_NEW_WEIGHT_DEFAULT;
#endif

	ol_txrx_local_peer_id_pool_init(pdev);

	pdev->cfg.ll_pause_txq_limit =
		ol_tx_cfg_max_tx_queue_depth_ll(pdev->ctrl_pdev);

	/* TX flow control for peer who is in very bad link status */
	ol_tx_badpeer_flow_cl_init(pdev);

#ifdef QCA_COMPUTE_TX_DELAY
	qdf_mem_zero(&pdev->tx_delay, sizeof(pdev->tx_delay));
	qdf_spinlock_create(&pdev->tx_delay.mutex);

	/* initialize compute interval with 5 seconds (ESE default) */
	pdev->tx_delay.avg_period_ticks = qdf_system_msecs_to_ticks(5000);
	{
		uint32_t bin_width_1000ticks;

		bin_width_1000ticks =
			qdf_system_msecs_to_ticks
				(QCA_TX_DELAY_HIST_INTERNAL_BIN_WIDTH_MS
				 * 1000);
		/*
		 * Compute a factor and shift that together are equal to the
		 * inverse of the bin_width time, so that rather than dividing
		 * by the bin width time, approximately the same result can be
		 * obtained much more efficiently by a multiply + shift.
		 * multiply_factor >> shift = 1 / bin_width_time, so
		 * multiply_factor = (1 << shift) / bin_width_time.
		 *
		 * Pick the shift semi-arbitrarily.
		 * If we knew statically what the bin_width would be, we could
		 * choose a shift that minimizes the error.
		 * Since the bin_width is determined dynamically, simply use a
		 * shift that is about half of the uint32_t size.  This should
		 * result in a relatively large multiplier value, which
		 * minimizes error from rounding the multiplier to an integer.
		 * The rounding error only becomes significant if the tick units
		 * are on the order of 1 microsecond.  In most systems, it is
		 * expected that the tick units will be relatively low-res,
		 * on the order of 1 millisecond.  In such systems the rounding
		 * error is negligible.
		 * It would be more accurate to dynamically try out different
		 * shifts and choose the one that results in the smallest
		 * rounding error, but that extra level of fidelity is
		 * not needed.
		 */
		pdev->tx_delay.hist_internal_bin_width_shift = 16;
		pdev->tx_delay.hist_internal_bin_width_mult =
			((1 << pdev->tx_delay.hist_internal_bin_width_shift) *
			 1000 + (bin_width_1000ticks >> 1)) /
			bin_width_1000ticks;
	}
#endif /* QCA_COMPUTE_TX_DELAY */

	/* Thermal Mitigation */
	ol_tx_throttle_init(pdev);

	ol_tso_seg_list_init(pdev, desc_pool_size);

	ol_tso_num_seg_list_init(pdev, desc_pool_size);

	ol_tx_register_flow_control(pdev);

	return 0;            /* success */

pn_trace_attach_fail:
	OL_RX_REORDER_TRACE_DETACH(pdev);

reorder_trace_attach_fail:
	qdf_spinlock_destroy(&pdev->tx_mutex);
	qdf_spinlock_destroy(&pdev->peer_ref_mutex);
	qdf_spinlock_destroy(&pdev->rx.mutex);
	qdf_spinlock_destroy(&pdev->last_real_peer_mutex);
	qdf_spinlock_destroy(&pdev->peer_map_unmap_lock);
	OL_TXRX_PEER_STATS_MUTEX_DESTROY(pdev);

control_init_fail:
desc_alloc_fail:
	for (i = 0; i < fail_idx; i++)
		htt_tx_desc_free(pdev->htt_pdev,
			(ol_tx_desc_find(pdev, i))->htt_tx_desc);

	qdf_mem_multi_pages_free(pdev->osdev,
		&pdev->tx_desc.desc_pages, 0, true);

page_alloc_fail:
	if (ol_cfg_ipa_uc_offload_enabled(pdev->ctrl_pdev))
		htt_ipa_uc_detach(pdev->htt_pdev);
uc_attach_fail:
	htt_detach(pdev->htt_pdev);
htt_attach_fail:
	ol_tx_desc_dup_detect_deinit(pdev);
ol_attach_fail:
	return ret;            /* fail */
}

/**
 * ol_txrx_pdev_attach_target() - send target configuration
 *
 * @pdev - the physical device being initialized
 *
 * The majority of the data SW setup are done by the pdev_attach
 * functions, but this function completes the data SW setup by
 * sending datapath configuration messages to the target.
 *
 * Return: 0 - success 1 - failure
 */
static A_STATUS ol_txrx_pdev_attach_target(struct cdp_pdev *ppdev)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	return htt_attach_target(pdev->htt_pdev) == QDF_STATUS_SUCCESS ? 0:1;
}

/**
 * ol_tx_free_descs_inuse - free tx descriptors which are in use
 * @pdev - the physical device for which tx descs need to be freed
 *
 * Cycle through the list of TX descriptors (for a pdev) which are in use,
 * for which TX completion has not been received and free them. Should be
 * called only when the interrupts are off and all lower layer RX is stopped.
 * Otherwise there may be a race condition with TX completions.
 *
 * Return: None
 */
static void ol_tx_free_descs_inuse(ol_txrx_pdev_handle pdev)
{
	int i;
	void *htt_tx_desc;
	struct ol_tx_desc_t *tx_desc;
	int num_freed_tx_desc = 0;

	for (i = 0; i < pdev->tx_desc.pool_size; i++) {
		tx_desc = ol_tx_desc_find(pdev, i);
		/*
		 * Confirm that each tx descriptor is "empty", i.e. it has
		 * no tx frame attached.
		 * In particular, check that there are no frames that have
		 * been given to the target to transmit, for which the
		 * target has never provided a response.
		 */
		if (qdf_atomic_read(&tx_desc->ref_cnt)) {
			ol_txrx_dbg("Warning: freeing tx frame (no compltn)");
			ol_tx_desc_frame_free_nonstd(pdev,
						     tx_desc, 1);
			num_freed_tx_desc++;
		}
		htt_tx_desc = tx_desc->htt_tx_desc;
		htt_tx_desc_free(pdev->htt_pdev, htt_tx_desc);
	}

	if (num_freed_tx_desc)
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"freed %d tx frames for which no resp from target",
		num_freed_tx_desc);

}

/**
 * ol_txrx_pdev_pre_detach() - detach the data SW state
 * @pdev - the data physical device object being removed
 * @force - delete the pdev (and its vdevs and peers) even if
 * there are outstanding references by the target to the vdevs
 * and peers within the pdev
 *
 * This function is used when the WLAN driver is being removed to
 * detach the host data component within the driver.
 *
 * Return: None
 */
static void ol_txrx_pdev_pre_detach(struct cdp_pdev *ppdev, int force)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	/* preconditions */
	TXRX_ASSERT2(pdev);

	/* check that the pdev has no vdevs allocated */
	TXRX_ASSERT1(TAILQ_EMPTY(&pdev->vdev_list));

#ifdef QCA_SUPPORT_TX_THROTTLE
	/* Thermal Mitigation */
	qdf_timer_stop(&pdev->tx_throttle.phase_timer);
	qdf_timer_free(&pdev->tx_throttle.phase_timer);
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	qdf_timer_stop(&pdev->tx_throttle.tx_timer);
	qdf_timer_free(&pdev->tx_throttle.tx_timer);
#endif
#endif

	if (force) {
		/*
		 * The assertion above confirms that all vdevs within this pdev
		 * were detached.  However, they may not have actually been
		 * deleted.
		 * If the vdev had peers which never received a PEER_UNMAP msg
		 * from the target, then there are still zombie peer objects,
		 * and the vdev parents of the zombie peers are also zombies,
		 * hanging around until their final peer gets deleted.
		 * Go through the peer hash table and delete any peers left.
		 * As a side effect, this will complete the deletion of any
		 * vdevs that are waiting for their peers to finish deletion.
		 */
		ol_txrx_dbg("Force delete for pdev %pK\n",
			   pdev);
		ol_txrx_peer_find_hash_erase(pdev);
	}

	/* to get flow pool status before freeing descs */
	ol_tx_dump_flow_pool_info((void *)pdev);
	ol_tx_free_descs_inuse(pdev);
	ol_tx_deregister_flow_control(pdev);

	/*
	 * ol_tso_seg_list_deinit should happen after
	 * ol_tx_deinit_tx_desc_inuse as it tries to access the tso seg freelist
	 * which is being de-initilized in ol_tso_seg_list_deinit
	 */
	ol_tso_seg_list_deinit(pdev);
	ol_tso_num_seg_list_deinit(pdev);

	/* Stop the communication between HTT and target at first */
	htt_detach_target(pdev->htt_pdev);

	qdf_mem_multi_pages_free(pdev->osdev,
		&pdev->tx_desc.desc_pages, 0, true);
	pdev->tx_desc.freelist = NULL;

	/* Detach micro controller data path offload resource */
	if (ol_cfg_ipa_uc_offload_enabled(pdev->ctrl_pdev))
		htt_ipa_uc_detach(pdev->htt_pdev);

	htt_detach(pdev->htt_pdev);
	ol_tx_desc_dup_detect_deinit(pdev);

	qdf_spinlock_destroy(&pdev->tx_mutex);
	qdf_spinlock_destroy(&pdev->peer_ref_mutex);
	qdf_spinlock_destroy(&pdev->last_real_peer_mutex);
	qdf_spinlock_destroy(&pdev->rx.mutex);
	qdf_spinlock_destroy(&pdev->peer_map_unmap_lock);
#ifdef QCA_SUPPORT_TX_THROTTLE
	/* Thermal Mitigation */
	qdf_spinlock_destroy(&pdev->tx_throttle.mutex);
#endif

	/* TX flow control for peer who is in very bad link status */
	ol_tx_badpeer_flow_cl_deinit(pdev);

	OL_TXRX_PEER_STATS_MUTEX_DESTROY(pdev);

	OL_RX_REORDER_TRACE_DETACH(pdev);
	OL_RX_PN_TRACE_DETACH(pdev);

	/*
	 * WDI event detach
	 */
	wdi_event_detach(pdev);

	ol_txrx_local_peer_id_cleanup(pdev);

#ifdef QCA_COMPUTE_TX_DELAY
	qdf_spinlock_destroy(&pdev->tx_delay.mutex);
#endif
	qdf_mem_free(ppdev);
}

/**
 * ol_txrx_pdev_detach() - delete the data SW state
 * @ppdev - the data physical device object being removed
 * @force - delete the pdev (and its vdevs and peers) even if
 * there are outstanding references by the target to the vdevs
 * and peers within the pdev
 *
 * This function is used when the WLAN driver is being removed to
 * remove the host data component within the driver.
 * All virtual devices within the physical device need to be deleted
 * (ol_txrx_vdev_detach) before the physical device itself is deleted.
 *
 * Return: None
 */
static void ol_txrx_pdev_detach(struct cdp_pdev *ppdev, int force)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_stats_req_internal *req;
	int i = 0;

	/*checking to ensure txrx pdev structure is not NULL */
	if (!pdev) {
		ol_txrx_err(
			   "NULL pdev passed to %s\n", __func__);
		return;
	}

	htt_pktlogmod_exit(pdev);

	qdf_spin_lock_bh(&pdev->req_list_spinlock);
	if (pdev->req_list_depth > 0)
		ol_txrx_err(
			"Warning: the txrx req list is not empty, depth=%d\n",
			pdev->req_list_depth
			);
	TAILQ_FOREACH(req, &pdev->req_list, req_list_elem) {
		TAILQ_REMOVE(&pdev->req_list, req, req_list_elem);
		pdev->req_list_depth--;
		ol_txrx_err(
			"%d: %pK,verbose(%d), concise(%d), up_m(0x%x), reset_m(0x%x)\n",
			i++,
			req,
			req->base.print.verbose,
			req->base.print.concise,
			req->base.stats_type_upload_mask,
			req->base.stats_type_reset_mask
			);
		qdf_mem_free(req);
	}
	qdf_spin_unlock_bh(&pdev->req_list_spinlock);

	qdf_spinlock_destroy(&pdev->req_list_spinlock);

	OL_RX_REORDER_TIMEOUT_CLEANUP(pdev);

	if (pdev->cfg.is_high_latency)
		ol_tx_sched_detach(pdev);

	htt_deregister_rx_pkt_dump_callback(pdev->htt_pdev);

	htt_pdev_free(pdev->htt_pdev);
	ol_txrx_peer_find_detach(pdev);
	ol_txrx_tso_stats_deinit(pdev);

	ol_txrx_pdev_txq_log_destroy(pdev);
	ol_txrx_pdev_grp_stat_destroy(pdev);
}

#if defined(CONFIG_PER_VDEV_TX_DESC_POOL)

/**
 * ol_txrx_vdev_tx_desc_cnt_init() - initialise tx descriptor count for vdev
 * @vdev: the virtual device object
 *
 * Return: None
 */
static inline void
ol_txrx_vdev_tx_desc_cnt_init(struct ol_txrx_vdev_t *vdev)
{
	qdf_atomic_init(&vdev->tx_desc_count);
}
#else

static inline void
ol_txrx_vdev_tx_desc_cnt_init(struct ol_txrx_vdev_t *vdev)
{
}
#endif

/**
 * ol_txrx_vdev_attach - Allocate and initialize the data object
 * for a new virtual device.
 *
 * @data_pdev - the physical device the virtual device belongs to
 * @vdev_mac_addr - the MAC address of the virtual device
 * @vdev_id - the ID used to identify the virtual device to the target
 * @op_mode - whether this virtual device is operating as an AP,
 * an IBSS, or a STA
 *
 * Return: success: handle to new data vdev object, failure: NULL
 */
static struct cdp_vdev *
ol_txrx_vdev_attach(struct cdp_pdev *ppdev,
		    uint8_t *vdev_mac_addr,
		    uint8_t vdev_id, enum wlan_op_mode op_mode)
{
	struct ol_txrx_pdev_t  *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_vdev_t *vdev;
	QDF_STATUS qdf_status;

	/* preconditions */
	TXRX_ASSERT2(pdev);
	TXRX_ASSERT2(vdev_mac_addr);

	vdev = qdf_mem_malloc(sizeof(*vdev));
	if (!vdev)
		return NULL;    /* failure */

	/* store provided params */
	vdev->pdev = pdev;
	vdev->vdev_id = vdev_id;
	vdev->opmode = op_mode;

	vdev->delete.pending = 0;
	vdev->safemode = 0;
	vdev->drop_unenc = 1;
	vdev->num_filters = 0;
	vdev->fwd_tx_packets = 0;
	vdev->fwd_rx_packets = 0;

	ol_txrx_vdev_tx_desc_cnt_init(vdev);

	qdf_mem_copy(&vdev->mac_addr.raw[0], vdev_mac_addr,
		     OL_TXRX_MAC_ADDR_LEN);

	TAILQ_INIT(&vdev->peer_list);
	vdev->last_real_peer = NULL;

	ol_txrx_hl_tdls_flag_reset((struct cdp_vdev *)vdev, false);

#ifdef QCA_IBSS_SUPPORT
	vdev->ibss_peer_num = 0;
	vdev->ibss_peer_heart_beat_timer = 0;
#endif

	ol_txrx_vdev_txqs_init(vdev);

	qdf_spinlock_create(&vdev->ll_pause.mutex);
	vdev->ll_pause.paused_reason = 0;
	vdev->ll_pause.txq.head = vdev->ll_pause.txq.tail = NULL;
	vdev->ll_pause.txq.depth = 0;
	qdf_atomic_init(&vdev->delete.detaching);
	qdf_timer_init(pdev->osdev,
			       &vdev->ll_pause.timer,
			       ol_tx_vdev_ll_pause_queue_send, vdev,
			       QDF_TIMER_TYPE_SW);
	qdf_atomic_init(&vdev->os_q_paused);
	qdf_atomic_set(&vdev->os_q_paused, 0);
	vdev->tx_fl_lwm = 0;
	vdev->tx_fl_hwm = 0;
	vdev->rx = NULL;
	vdev->wait_on_peer_id = OL_TXRX_INVALID_LOCAL_PEER_ID;
	qdf_mem_zero(&vdev->last_peer_mac_addr,
			sizeof(union ol_txrx_align_mac_addr_t));
	qdf_spinlock_create(&vdev->flow_control_lock);
	vdev->osif_flow_control_cb = NULL;
	vdev->osif_flow_control_is_pause = NULL;
	vdev->osif_fc_ctx = NULL;

	/* Default MAX Q depth for every VDEV */
	vdev->ll_pause.max_q_depth =
		ol_tx_cfg_max_tx_queue_depth_ll(vdev->pdev->ctrl_pdev);
	qdf_status = qdf_event_create(&vdev->wait_delete_comp);
	/* add this vdev into the pdev's list */
	TAILQ_INSERT_TAIL(&pdev->vdev_list, vdev, vdev_list_elem);

	ol_txrx_dbg(
		   "Created vdev %pK (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		   vdev,
		   vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
		   vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
		   vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);

	/*
	 * We've verified that htt_op_mode == wlan_op_mode,
	 * so no translation is needed.
	 */
	htt_vdev_attach(pdev->htt_pdev, vdev_id, op_mode);

	return (struct cdp_vdev *)vdev;
}

/**
 *ol_txrx_vdev_register - Link a vdev's data object with the
 * matching OS shim vdev object.
 *
 * @txrx_vdev: the virtual device's data object
 * @osif_vdev: the virtual device's OS shim object
 * @txrx_ops: (pointers to)functions used for tx and rx data xfer
 *
 *  The data object for a virtual device is created by the
 *  function ol_txrx_vdev_attach.  However, rather than fully
 *  linking the data vdev object with the vdev objects from the
 *  other subsystems that the data vdev object interacts with,
 *  the txrx_vdev_attach function focuses primarily on creating
 *  the data vdev object. After the creation of both the data
 *  vdev object and the OS shim vdev object, this
 *  txrx_osif_vdev_attach function is used to connect the two
 *  vdev objects, so the data SW can use the OS shim vdev handle
 *  when passing rx data received by a vdev up to the OS shim.
 */
static void ol_txrx_vdev_register(struct cdp_vdev *pvdev,
				void *osif_vdev,
				struct ol_txrx_ops *txrx_ops)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	if (qdf_unlikely(!vdev) || qdf_unlikely(!txrx_ops)) {
		qdf_print("%s: vdev/txrx_ops is NULL!\n", __func__);
		qdf_assert(0);
		return;
	}

	vdev->osif_dev = osif_vdev;
	vdev->rx = txrx_ops->rx.rx;
	txrx_ops->tx.tx = ol_tx_data;
}

#ifdef currently_unused
/**
 * ol_txrx_set_curchan - Setup the current operating channel of
 * the device
 * @pdev - the data physical device object
 * @chan_mhz - the channel frequency (mhz) packets on
 *
 * Mainly used when populating monitor mode status that requires
 * the current operating channel
 *
 */
void ol_txrx_set_curchan(ol_txrx_pdev_handle pdev, uint32_t chan_mhz)
{
}
#endif

void ol_txrx_set_safemode(ol_txrx_vdev_handle vdev, uint32_t val)
{
	vdev->safemode = val;
}

/**
 * ol_txrx_set_privacy_filters - set the privacy filter
 * @vdev - the data virtual device object
 * @filter - filters to be set
 * @num - the number of filters
 *
 * Rx related. Set the privacy filters. When rx packets, check
 * the ether type, filter type and packet type to decide whether
 * discard these packets.
 */
static void
ol_txrx_set_privacy_filters(ol_txrx_vdev_handle vdev,
			    void *filters, uint32_t num)
{
	qdf_mem_copy(vdev->privacy_filters, filters,
		     num * sizeof(struct privacy_exemption));
	vdev->num_filters = num;
}

void ol_txrx_set_drop_unenc(ol_txrx_vdev_handle vdev, uint32_t val)
{
	vdev->drop_unenc = val;
}

#if defined(CONFIG_HL_SUPPORT)

static void
ol_txrx_tx_desc_reset_vdev(ol_txrx_vdev_handle vdev)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	int i;
	struct ol_tx_desc_t *tx_desc;

	qdf_spin_lock_bh(&pdev->tx_mutex);
	for (i = 0; i < pdev->tx_desc.pool_size; i++) {
		tx_desc = ol_tx_desc_find(pdev, i);
		if (tx_desc->vdev == vdev)
			tx_desc->vdev = NULL;
	}
	qdf_spin_unlock_bh(&pdev->tx_mutex);
}

#else

static void
ol_txrx_tx_desc_reset_vdev(ol_txrx_vdev_handle vdev)
{

}

#endif

/**
 * ol_txrx_vdev_detach - Deallocate the specified data virtual
 * device object.
 * @data_vdev: data object for the virtual device in question
 * @callback: function to call (if non-NULL) once the vdev has
 * been wholly deleted
 * @callback_context: context to provide in the callback
 *
 * All peers associated with the virtual device need to be deleted
 * (ol_txrx_peer_detach) before the virtual device itself is deleted.
 * However, for the peers to be fully deleted, the peer deletion has to
 * percolate through the target data FW and back up to the host data SW.
 * Thus, even though the host control SW may have issued a peer_detach
 * call for each of the vdev's peers, the peer objects may still be
 * allocated, pending removal of all references to them by the target FW.
 * In this case, though the vdev_detach function call will still return
 * immediately, the vdev itself won't actually be deleted, until the
 * deletions of all its peers complete.
 * The caller can provide a callback function pointer to be notified when
 * the vdev deletion actually happens - whether it's directly within the
 * vdev_detach call, or if it's deferred until all in-progress peer
 * deletions have completed.
 */
static void
ol_txrx_vdev_detach(struct cdp_vdev *pvdev,
		    ol_txrx_vdev_delete_cb callback, void *context)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	struct ol_txrx_pdev_t *pdev;

	/* preconditions */
	TXRX_ASSERT2(vdev);
	pdev = vdev->pdev;

	/* prevent anyone from restarting the ll_pause timer again */
	qdf_atomic_set(&vdev->delete.detaching, 1);

	ol_txrx_vdev_tx_queue_free(vdev);

	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	qdf_timer_stop(&vdev->ll_pause.timer);
	vdev->ll_pause.is_q_timer_on = false;
	while (vdev->ll_pause.txq.head) {
		qdf_nbuf_t next = qdf_nbuf_next(vdev->ll_pause.txq.head);

		qdf_nbuf_set_next(vdev->ll_pause.txq.head, NULL);
		qdf_nbuf_unmap(pdev->osdev, vdev->ll_pause.txq.head,
			       QDF_DMA_TO_DEVICE);
		qdf_nbuf_tx_free(vdev->ll_pause.txq.head, QDF_NBUF_PKT_ERROR);
		vdev->ll_pause.txq.head = next;
	}
	qdf_spin_unlock_bh(&vdev->ll_pause.mutex);

	/* ll_pause timer should be deleted without any locks held, and
	 * no timer function should be executed after this point because
	 * qdf_timer_free is deleting the timer synchronously.
	 */
	qdf_timer_free(&vdev->ll_pause.timer);
	qdf_spinlock_destroy(&vdev->ll_pause.mutex);

	qdf_spin_lock_bh(&vdev->flow_control_lock);
	vdev->osif_flow_control_cb = NULL;
	vdev->osif_flow_control_is_pause = NULL;
	vdev->osif_fc_ctx = NULL;
	qdf_spin_unlock_bh(&vdev->flow_control_lock);
	qdf_spinlock_destroy(&vdev->flow_control_lock);

	/* remove the vdev from its parent pdev's list */
	TAILQ_REMOVE(&pdev->vdev_list, vdev, vdev_list_elem);

	/*
	 * Use peer_ref_mutex while accessing peer_list, in case
	 * a peer is in the process of being removed from the list.
	 */
	qdf_spin_lock_bh(&pdev->peer_ref_mutex);
	/* check that the vdev has no peers allocated */
	if (!TAILQ_EMPTY(&vdev->peer_list)) {
		/* debug print - will be removed later */
		ol_txrx_dbg(
			   "%s: not deleting vdev object %pK (%02x:%02x:%02x:%02x:%02x:%02x) until deletion finishes for all its peers\n",
			   __func__, vdev,
			   vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
			   vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
			   vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);
		/* indicate that the vdev needs to be deleted */
		vdev->delete.pending = 1;
		vdev->delete.callback = callback;
		vdev->delete.context = context;
		qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
		return;
	}
	qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
	qdf_event_destroy(&vdev->wait_delete_comp);

	ol_txrx_dbg(
		   "%s: deleting vdev obj %pK (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		   __func__, vdev,
		   vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
		   vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
		   vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);

	htt_vdev_detach(pdev->htt_pdev, vdev->vdev_id);

	/*
	 * The ol_tx_desc_free might access the invalid content of vdev referred
	 * by tx desc, since this vdev might be detached in another thread
	 * asynchronous.
	 *
	 * Go through tx desc pool to set corresponding tx desc's vdev to NULL
	 * when detach this vdev, and add vdev checking in the ol_tx_desc_free
	 * to avoid crash.
	 *
	 */
	ol_txrx_tx_desc_reset_vdev(vdev);

	/*
	 * Doesn't matter if there are outstanding tx frames -
	 * they will be freed once the target sends a tx completion
	 * message for them.
	 */
	qdf_mem_free(vdev);
	if (callback)
		callback(context);
}

/**
 * ol_txrx_flush_rx_frames() - flush cached rx frames
 * @peer: peer
 * @drop: set flag to drop frames
 *
 * Return: None
 */
void ol_txrx_flush_rx_frames(struct ol_txrx_peer_t *peer,
			     bool drop)
{
	struct ol_txrx_cached_bufq_t *bufqi;
	struct ol_rx_cached_buf *cache_buf;
	QDF_STATUS ret;
	ol_txrx_rx_fp data_rx = NULL;

	if (qdf_atomic_inc_return(&peer->flush_in_progress) > 1) {
		qdf_atomic_dec(&peer->flush_in_progress);
		return;
	}

	qdf_assert(peer->vdev);
	qdf_spin_lock_bh(&peer->peer_info_lock);
	bufqi = &peer->bufq_info;

	if (peer->state >= OL_TXRX_PEER_STATE_CONN && peer->vdev->rx)
		data_rx = peer->vdev->rx;
	else
		drop = true;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	cache_buf = list_entry((&bufqi->cached_bufq)->next,
				typeof(*cache_buf), list);
	while (!list_empty(&bufqi->cached_bufq)) {
		list_del(&cache_buf->list);
		bufqi->curr--;
		qdf_assert(bufqi->curr >= 0);
		qdf_spin_unlock_bh(&bufqi->bufq_lock);
		if (drop) {
			qdf_nbuf_free(cache_buf->buf);
		} else {
			/* Flush the cached frames to HDD */
			ret = data_rx(peer->vdev->osif_dev, cache_buf->buf);
			if (ret != QDF_STATUS_SUCCESS)
				qdf_nbuf_free(cache_buf->buf);
		}
		qdf_mem_free(cache_buf);
		qdf_spin_lock_bh(&bufqi->bufq_lock);
		cache_buf = list_entry((&bufqi->cached_bufq)->next,
				typeof(*cache_buf), list);
	}
	bufqi->qdepth_no_thresh = bufqi->curr;
	qdf_spin_unlock_bh(&bufqi->bufq_lock);
	qdf_atomic_dec(&peer->flush_in_progress);
}

static void ol_txrx_flush_cache_rx_queue(void)
{
	uint8_t sta_id;
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_pdev_t *pdev;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev)
		return;

	for (sta_id = 0; sta_id < WLAN_MAX_STA_COUNT; sta_id++) {
		peer = ol_txrx_peer_find_by_local_id((struct cdp_pdev *)pdev,
						    sta_id);
		if (!peer)
			continue;
		ol_txrx_flush_rx_frames(peer, 1);
	}
}

/* Define short name to use in cds_trigger_recovery */
#define PEER_DEL_TIMEOUT QDF_PEER_DELETION_TIMEDOUT

/**
 * ol_txrx_dump_peer_access_list() - dump peer access list
 * @peer: peer handle
 *
 * This function will dump if any peer debug ids are still accessing peer
 *
 * Return: None
 */
static void ol_txrx_dump_peer_access_list(ol_txrx_peer_handle peer)
{
	u32 i;
	u32 pending_ref;

	for (i = 0; i < PEER_DEBUG_ID_MAX; i++) {
		pending_ref = qdf_atomic_read(&peer->access_list[i]);
		if (pending_ref)
			ol_txrx_info_high("id %d pending refs %d",
					  i, pending_ref);
	}
}

/**
 * ol_txrx_peer_attach - Allocate and set up references for a
 * data peer object.
 * @data_pdev: data physical device object that will indirectly
 * own the data_peer object
 * @data_vdev - data virtual device object that will directly
 * own the data_peer object
 * @peer_mac_addr - MAC address of the new peer
 *
 * When an association with a peer starts, the host's control SW
 * uses this function to inform the host data SW.
 * The host data SW allocates its own peer object, and stores a
 * reference to the control peer object within the data peer object.
 * The host data SW also stores a reference to the virtual device
 * that the peer is associated with.  This virtual device handle is
 * used when the data SW delivers rx data frames to the OS shim layer.
 * The host data SW returns a handle to the new peer data object,
 * so a reference within the control peer object can be set to the
 * data peer object.
 *
 * Return: handle to new data peer object, or NULL if the attach
 * fails
 */
static void *
ol_txrx_peer_attach(struct cdp_vdev *pvdev, uint8_t *peer_mac_addr)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_peer_t *temp_peer;
	uint8_t i;
	bool wait_on_deletion = false;
	unsigned long rc;
	struct ol_txrx_pdev_t *pdev;
	bool cmp_wait_mac = false;
	uint8_t zero_mac_addr[QDF_MAC_ADDR_SIZE] = { 0, 0, 0, 0, 0, 0 };

	/* preconditions */
	TXRX_ASSERT2(vdev);
	TXRX_ASSERT2(peer_mac_addr);

	pdev = vdev->pdev;
	TXRX_ASSERT2(pdev);

	if (qdf_mem_cmp(&zero_mac_addr, &vdev->last_peer_mac_addr,
				QDF_MAC_ADDR_SIZE))
		cmp_wait_mac = true;

	qdf_spin_lock_bh(&pdev->peer_ref_mutex);
	/* check for duplicate existing peer */
	TAILQ_FOREACH(temp_peer, &vdev->peer_list, peer_list_elem) {
		if (!ol_txrx_peer_find_mac_addr_cmp(&temp_peer->mac_addr,
			(union ol_txrx_align_mac_addr_t *)peer_mac_addr)) {
			ol_txrx_info_high(
				"vdev_id %d (%02x:%02x:%02x:%02x:%02x:%02x) already exists.\n",
				vdev->vdev_id,
				peer_mac_addr[0], peer_mac_addr[1],
				peer_mac_addr[2], peer_mac_addr[3],
				peer_mac_addr[4], peer_mac_addr[5]);
			if (qdf_atomic_read(&temp_peer->delete_in_progress)) {
				vdev->wait_on_peer_id = temp_peer->local_id;
				qdf_event_reset(&vdev->wait_delete_comp);
				wait_on_deletion = true;
				break;
			} else {
				qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
				return NULL;
			}
		}
		if (cmp_wait_mac && !ol_txrx_peer_find_mac_addr_cmp(
					&temp_peer->mac_addr,
					&vdev->last_peer_mac_addr)) {
			ol_txrx_info_high(
				"vdev_id %d (%02x:%02x:%02x:%02x:%02x:%02x) old peer exists.\n",
				vdev->vdev_id,
				vdev->last_peer_mac_addr.raw[0],
				vdev->last_peer_mac_addr.raw[1],
				vdev->last_peer_mac_addr.raw[2],
				vdev->last_peer_mac_addr.raw[3],
				vdev->last_peer_mac_addr.raw[4],
				vdev->last_peer_mac_addr.raw[5]);
			if (qdf_atomic_read(&temp_peer->delete_in_progress)) {
				vdev->wait_on_peer_id = temp_peer->local_id;
				qdf_event_reset(&vdev->wait_delete_comp);
				wait_on_deletion = true;
				break;
			} else {
				qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
				ol_txrx_err("peer not found");
				return NULL;
			}
		}
	}
	qdf_spin_unlock_bh(&pdev->peer_ref_mutex);

	qdf_mem_zero(&vdev->last_peer_mac_addr,
			sizeof(union ol_txrx_align_mac_addr_t));
	if (wait_on_deletion) {
		/* wait for peer deletion */
		rc = qdf_wait_for_event_completion(&vdev->wait_delete_comp,
					   PEER_DELETION_TIMEOUT);
		if (QDF_STATUS_SUCCESS != rc) {
			ol_txrx_err("error waiting for peer_id(%d) deletion, status %d\n",
				    vdev->wait_on_peer_id, (int) rc);
			/* Added for debugging only */
			ol_txrx_dump_peer_access_list(temp_peer);
			wlan_roam_debug_dump_table();
			vdev->wait_on_peer_id = OL_TXRX_INVALID_LOCAL_PEER_ID;

			return NULL;
		}
	}

	peer = qdf_mem_malloc(sizeof(*peer));
	if (!peer)
		return NULL;    /* failure */

	/* store provided params */
	peer->vdev = vdev;
	qdf_mem_copy(&peer->mac_addr.raw[0], peer_mac_addr,
		     OL_TXRX_MAC_ADDR_LEN);

	ol_txrx_peer_txqs_init(pdev, peer);

	INIT_LIST_HEAD(&peer->bufq_info.cached_bufq);
	qdf_spin_lock_bh(&pdev->peer_ref_mutex);
	/* add this peer into the vdev's list */
	TAILQ_INSERT_TAIL(&vdev->peer_list, peer, peer_list_elem);
	qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
	/* check whether this is a real peer (peer mac addr != vdev mac addr) */
	if (ol_txrx_peer_find_mac_addr_cmp(&vdev->mac_addr, &peer->mac_addr)) {
		qdf_spin_lock_bh(&pdev->last_real_peer_mutex);
		vdev->last_real_peer = peer;
		qdf_spin_unlock_bh(&pdev->last_real_peer_mutex);
	}

	peer->rx_opt_proc = pdev->rx_opt_proc;

	ol_rx_peer_init(pdev, peer);

	/* initialize the peer_id */
	for (i = 0; i < MAX_NUM_PEER_ID_PER_PEER; i++)
		peer->peer_ids[i] = HTT_INVALID_PEER;

	qdf_spinlock_create(&peer->peer_info_lock);
	qdf_spinlock_create(&peer->bufq_info.bufq_lock);

	peer->bufq_info.thresh = OL_TXRX_CACHED_BUFQ_THRESH;

	qdf_atomic_init(&peer->delete_in_progress);
	qdf_atomic_init(&peer->flush_in_progress);
	qdf_atomic_init(&peer->ref_cnt);

	for (i = 0; i < PEER_DEBUG_ID_MAX; i++)
		qdf_atomic_init(&peer->access_list[i]);

	/* keep one reference for attach */
	ol_txrx_peer_get_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);

	/* Set a flag to indicate peer create is pending in firmware */
	qdf_atomic_init(&peer->fw_create_pending);
	qdf_atomic_set(&peer->fw_create_pending, 1);

	peer->valid = 1;
	qdf_timer_init(pdev->osdev, &peer->peer_unmap_timer,
		       peer_unmap_timer_handler, peer, QDF_TIMER_TYPE_SW);

	ol_txrx_peer_find_hash_add(pdev, peer);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		   "vdev %pK created peer %pK ref_cnt %d (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		   vdev, peer, qdf_atomic_read(&peer->ref_cnt),
		   peer->mac_addr.raw[0], peer->mac_addr.raw[1],
		   peer->mac_addr.raw[2], peer->mac_addr.raw[3],
		   peer->mac_addr.raw[4], peer->mac_addr.raw[5]);
	/*
	 * For every peer MAp message search and set if bss_peer
	 */
	if (qdf_mem_cmp(peer->mac_addr.raw, vdev->mac_addr.raw,
				OL_TXRX_MAC_ADDR_LEN))
		peer->bss_peer = 1;

	/*
	 * The peer starts in the "disc" state while association is in progress.
	 * Once association completes, the peer will get updated to "auth" state
	 * by a call to ol_txrx_peer_state_update if the peer is in open mode,
	 * or else to the "conn" state. For non-open mode, the peer will
	 * progress to "auth" state once the authentication completes.
	 */
	peer->state = OL_TXRX_PEER_STATE_INVALID;
	ol_txrx_peer_state_update((struct cdp_pdev *)pdev, peer->mac_addr.raw,
				  OL_TXRX_PEER_STATE_DISC);

#ifdef QCA_SUPPORT_PEER_DATA_RX_RSSI
	peer->rssi_dbm = HTT_RSSI_INVALID;
#endif
	if ((QDF_GLOBAL_MONITOR_MODE == cds_get_conparam()) &&
	    !pdev->self_peer) {
		pdev->self_peer = peer;
		/*
		 * No Tx in monitor mode, otherwise results in target assert.
		 * Setting disable_intrabss_fwd to true
		 */
		ol_vdev_rx_set_intrabss_fwd((struct cdp_vdev *)vdev, true);
	}

	ol_txrx_local_peer_id_alloc(pdev, peer);

	return (void *)peer;
}

#undef PEER_DEL_TIMEOUT

/*
 * Discarding tx filter - removes all data frames (disconnected state)
 */
static A_STATUS ol_tx_filter_discard(struct ol_txrx_msdu_info_t *tx_msdu_info)
{
	return A_ERROR;
}

/*
 * Non-autentication tx filter - filters out data frames that are not
 * related to authentication, but allows EAPOL (PAE) or WAPI (WAI)
 * data frames (connected state)
 */
static A_STATUS ol_tx_filter_non_auth(struct ol_txrx_msdu_info_t *tx_msdu_info)
{
	return
		(tx_msdu_info->htt.info.ethertype == ETHERTYPE_PAE ||
		 tx_msdu_info->htt.info.ethertype ==
		 ETHERTYPE_WAI) ? A_OK : A_ERROR;
}

/*
 * Pass-through tx filter - lets all data frames through (authenticated state)
 */
static A_STATUS ol_tx_filter_pass_thru(struct ol_txrx_msdu_info_t *tx_msdu_info)
{
	return A_OK;
}

/**
 * ol_txrx_peer_get_peer_mac_addr() - return mac_addr from peer handle.
 * @peer: handle to peer
 *
 * returns mac addrs for module which do not know peer type
 *
 * Return: the mac_addr from peer
 */
static uint8_t *
ol_txrx_peer_get_peer_mac_addr(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	if (!peer)
		return NULL;

	return peer->mac_addr.raw;
}

#ifdef WLAN_FEATURE_11W
/**
 * ol_txrx_get_pn_info() - Returns pn info from peer
 * @peer: handle to peer
 * @last_pn_valid: return last_rmf_pn_valid value from peer.
 * @last_pn: return last_rmf_pn value from peer.
 * @rmf_pn_replays: return rmf_pn_replays value from peer.
 *
 * Return: NONE
 */
void
ol_txrx_get_pn_info(void *ppeer, uint8_t **last_pn_valid,
		    uint64_t **last_pn, uint32_t **rmf_pn_replays)
{
	ol_txrx_peer_handle peer = ppeer;
	*last_pn_valid = &peer->last_rmf_pn_valid;
	*last_pn = &peer->last_rmf_pn;
	*rmf_pn_replays = &peer->rmf_pn_replays;
}
#else
void
ol_txrx_get_pn_info(void *ppeer, uint8_t **last_pn_valid,
		uint64_t **last_pn, uint32_t **rmf_pn_replays)
{
}
#endif

/**
 * ol_txrx_get_opmode() - Return operation mode of vdev
 * @vdev: vdev handle
 *
 * Return: operation mode.
 */
static int ol_txrx_get_opmode(struct cdp_vdev *pvdev)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	return vdev->opmode;
}

/**
 * ol_txrx_get_peer_state() - Return peer state of peer
 * @peer: peer handle
 *
 * Return: return peer state
 */
static int ol_txrx_get_peer_state(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	return peer->state;
}

/**
 * ol_txrx_get_vdev_for_peer() - Return vdev from peer handle
 * @peer: peer handle
 *
 * Return: vdev handle from peer
 */
static struct cdp_vdev *ol_txrx_get_vdev_for_peer(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	return (struct cdp_vdev *)peer->vdev;
}

/**
 * ol_txrx_get_vdev_mac_addr() - Return mac addr of vdev
 * @vdev: vdev handle
 *
 * Return: vdev mac address
 */
static uint8_t *
ol_txrx_get_vdev_mac_addr(struct cdp_vdev *pvdev)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	if (!vdev)
		return NULL;

	return vdev->mac_addr.raw;
}

#ifdef currently_unused
/**
 * ol_txrx_get_vdev_struct_mac_addr() - Return handle to struct qdf_mac_addr of
 * vdev
 * @vdev: vdev handle
 *
 * Return: Handle to struct qdf_mac_addr
 */
struct qdf_mac_addr *
ol_txrx_get_vdev_struct_mac_addr(ol_txrx_vdev_handle vdev)
{
	return (struct qdf_mac_addr *)&(vdev->mac_addr);
}
#endif

#ifdef currently_unused
/**
 * ol_txrx_get_pdev_from_vdev() - Return handle to pdev of vdev
 * @vdev: vdev handle
 *
 * Return: Handle to pdev
 */
ol_txrx_pdev_handle ol_txrx_get_pdev_from_vdev(ol_txrx_vdev_handle vdev)
{
	return vdev->pdev;
}
#endif

/**
 * ol_txrx_get_ctrl_pdev_from_vdev() - Return control pdev of vdev
 * @vdev: vdev handle
 *
 * Return: Handle to control pdev
 */
static struct cdp_cfg *
ol_txrx_get_ctrl_pdev_from_vdev(struct cdp_vdev *pvdev)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	return vdev->pdev->ctrl_pdev;
}

/**
 * ol_txrx_is_rx_fwd_disabled() - returns the rx_fwd_disabled status on vdev
 * @vdev: vdev handle
 *
 * Return: Rx Fwd disabled status
 */
static uint8_t
ol_txrx_is_rx_fwd_disabled(struct cdp_vdev *pvdev)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)
					vdev->pdev->ctrl_pdev;
	return cfg->rx_fwd_disabled;
}

#ifdef QCA_IBSS_SUPPORT
/**
 * ol_txrx_update_ibss_add_peer_num_of_vdev() - update and return peer num
 * @vdev: vdev handle
 * @peer_num_delta: peer nums to be adjusted
 *
 * Return: -1 for failure or total peer nums after adjustment.
 */
static int16_t
ol_txrx_update_ibss_add_peer_num_of_vdev(struct cdp_vdev *pvdev,
					 int16_t peer_num_delta)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	int16_t new_peer_num;

	new_peer_num = vdev->ibss_peer_num + peer_num_delta;
	if (new_peer_num > MAX_PEERS || new_peer_num < 0)
		return OL_TXRX_INVALID_NUM_PEERS;

	vdev->ibss_peer_num = new_peer_num;

	return new_peer_num;
}

/**
 * ol_txrx_set_ibss_vdev_heart_beat_timer() - Update ibss vdev heart
 * beat timer
 * @vdev: vdev handle
 * @timer_value_sec: new heart beat timer value
 *
 * Return: Old timer value set in vdev.
 */
static uint16_t ol_txrx_set_ibss_vdev_heart_beat_timer(struct cdp_vdev *pvdev,
						uint16_t timer_value_sec)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	uint16_t old_timer_value = vdev->ibss_peer_heart_beat_timer;

	vdev->ibss_peer_heart_beat_timer = timer_value_sec;

	return old_timer_value;
}
#endif

/**
 * ol_txrx_remove_peers_for_vdev() - remove all vdev peers with lock held
 * @vdev: vdev handle
 * @callback: callback function to remove the peer.
 * @callback_context: handle for callback function
 * @remove_last_peer: Does it required to last peer.
 *
 * Return: NONE
 */
static void
ol_txrx_remove_peers_for_vdev(struct cdp_vdev *pvdev,
			      ol_txrx_vdev_peer_remove_cb callback,
			      void *callback_context, bool remove_last_peer)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	ol_txrx_peer_handle peer, temp;
	int self_removed = 0;
	/* remove all remote peers for vdev */
	qdf_spin_lock_bh(&vdev->pdev->peer_ref_mutex);

	temp = NULL;
	TAILQ_FOREACH_REVERSE(peer, &vdev->peer_list, peer_list_t,
			      peer_list_elem) {
		if (qdf_atomic_read(&peer->delete_in_progress))
			continue;
		if (temp) {
			qdf_spin_unlock_bh(&vdev->pdev->peer_ref_mutex);
			callback(callback_context, temp->mac_addr.raw,
				vdev->vdev_id, temp);
			qdf_spin_lock_bh(&vdev->pdev->peer_ref_mutex);
		}
		/* self peer is deleted last */
		if (peer == TAILQ_FIRST(&vdev->peer_list)) {
			self_removed = 1;
			break;
		}
		temp = peer;
	}

	qdf_spin_unlock_bh(&vdev->pdev->peer_ref_mutex);

	if (self_removed)
		ol_txrx_info("%s: self peer removed by caller ",
				   __func__);

	if (remove_last_peer) {
		/* remove IBSS bss peer last */
		peer = TAILQ_FIRST(&vdev->peer_list);
		callback(callback_context, (uint8_t *) &vdev->mac_addr,
			 vdev->vdev_id, peer);
	}
}

/**
 * ol_txrx_remove_peers_for_vdev_no_lock() - remove vdev peers with no lock.
 * @vdev: vdev handle
 * @callback: callback function to remove the peer.
 * @callback_context: handle for callback function
 *
 * Return: NONE
 */
static void
ol_txrx_remove_peers_for_vdev_no_lock(struct cdp_vdev *pvdev,
			      ol_txrx_vdev_peer_remove_cb callback,
			      void *callback_context)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	ol_txrx_peer_handle peer = NULL;
	ol_txrx_peer_handle tmp_peer = NULL;

	TAILQ_FOREACH_SAFE(peer, &vdev->peer_list, peer_list_elem, tmp_peer) {
		ol_txrx_info_high(
			   "%s: peer found for vdev id %d. deleting the peer",
			   __func__, vdev->vdev_id);
		callback(callback_context, (uint8_t *)&vdev->mac_addr,
				vdev->vdev_id, peer);
	}
}

/**
 * ol_txrx_set_ocb_chan_info() - set OCB channel info to vdev.
 * @vdev: vdev handle
 * @ocb_set_chan: OCB channel information to be set in vdev.
 *
 * Return: NONE
 */
static void ol_txrx_set_ocb_chan_info(struct cdp_vdev *pvdev,
			  struct ol_txrx_ocb_set_chan ocb_set_chan)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	vdev->ocb_channel_info = ocb_set_chan.ocb_channel_info;
	vdev->ocb_channel_count = ocb_set_chan.ocb_channel_count;
}

/**
 * ol_txrx_get_ocb_chan_info() - return handle to vdev ocb_channel_info
 * @vdev: vdev handle
 *
 * Return: handle to struct ol_txrx_ocb_chan_info
 */
static struct ol_txrx_ocb_chan_info *
ol_txrx_get_ocb_chan_info(struct cdp_vdev *pvdev)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	return vdev->ocb_channel_info;
}

/**
 * @brief specify the peer's authentication state
 * @details
 *  Specify the peer's authentication state (none, connected, authenticated)
 *  to allow the data SW to determine whether to filter out invalid data frames.
 *  (In the "connected" state, where security is enabled, but authentication
 *  has not completed, tx and rx data frames other than EAPOL or WAPI should
 *  be discarded.)
 *  This function is only relevant for systems in which the tx and rx filtering
 *  are done in the host rather than in the target.
 *
 * @param data_peer - which peer has changed its state
 * @param state - the new state of the peer
 *
 * Return: QDF Status
 */
QDF_STATUS ol_txrx_peer_state_update(struct cdp_pdev *ppdev,
				     uint8_t *peer_mac,
				     enum ol_txrx_peer_state state)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_peer_t *peer;
	int    peer_ref_cnt;

	if (qdf_unlikely(!pdev)) {
		ol_txrx_err("Pdev is NULL");
		qdf_assert(0);
		return QDF_STATUS_E_INVAL;
	}

	peer =  ol_txrx_peer_find_hash_find_get_ref(pdev, peer_mac, 0, 1,
						    PEER_DEBUG_ID_OL_INTERNAL);
	if (NULL == peer) {
		ol_txrx_err(
			   "%s: peer is null for peer_mac 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			   __func__,
			   peer_mac[0], peer_mac[1], peer_mac[2], peer_mac[3],
			   peer_mac[4], peer_mac[5]);
		return QDF_STATUS_E_INVAL;
	}

	/* TODO: Should we send WMI command of the connection state? */
	/* avoid multiple auth state change. */
	if (peer->state == state) {
#ifdef TXRX_PRINT_VERBOSE_ENABLE
		ol_txrx_dbg(
			   "%s: no state change, returns directly\n",
			   __func__);
#endif
		peer_ref_cnt = ol_txrx_peer_release_ref
						(peer,
						 PEER_DEBUG_ID_OL_INTERNAL);
		return QDF_STATUS_SUCCESS;
	}

	ol_txrx_dbg("%s: change from %d to %d\n",
		   __func__, peer->state, state);

	peer->tx_filter = (state == OL_TXRX_PEER_STATE_AUTH)
		? ol_tx_filter_pass_thru
		: ((state == OL_TXRX_PEER_STATE_CONN)
		   ? ol_tx_filter_non_auth
		   : ol_tx_filter_discard);

	if (peer->vdev->pdev->cfg.host_addba) {
		if (state == OL_TXRX_PEER_STATE_AUTH) {
			int tid;
			/*
			 * Pause all regular (non-extended) TID tx queues until
			 * data arrives and ADDBA negotiation has completed.
			 */
			ol_txrx_dbg(
				   "%s: pause peer and unpause mgmt/non-qos\n",
				   __func__);
			ol_txrx_peer_pause(peer); /* pause all tx queues */
			/* unpause mgmt and non-QoS tx queues */
			for (tid = OL_TX_NUM_QOS_TIDS;
			     tid < OL_TX_NUM_TIDS; tid++)
				ol_txrx_peer_tid_unpause(peer, tid);
		}
	}
	peer_ref_cnt = ol_txrx_peer_release_ref(peer,
						PEER_DEBUG_ID_OL_INTERNAL);
	/*
	 * after ol_txrx_peer_release_ref, peer object cannot be accessed
	 * if the return code was 0
	 */
	if (peer_ref_cnt > 0)
		/*
		 * Set the state after the Pause to avoid the race condiction
		 * with ADDBA check in tx path
		 */
		peer->state = state;
	return QDF_STATUS_SUCCESS;
}

void
ol_txrx_peer_keyinstalled_state_update(struct ol_txrx_peer_t *peer, uint8_t val)
{
	peer->keyinstalled = val;
}

void
ol_txrx_peer_update(ol_txrx_vdev_handle vdev,
		    uint8_t *peer_mac,
		    union ol_txrx_peer_update_param_t *param,
		    enum ol_txrx_peer_update_select_t select)
{
	struct ol_txrx_peer_t *peer;

	peer = ol_txrx_peer_find_hash_find_get_ref(vdev->pdev, peer_mac, 0, 1,
						   PEER_DEBUG_ID_OL_INTERNAL);
	if (!peer) {
		ol_txrx_dbg("%s: peer is null",
			   __func__);
		return;
	}

	switch (select) {
	case ol_txrx_peer_update_qos_capable:
	{
		/* save qos_capable here txrx peer,
		 * when HTT_ISOC_T2H_MSG_TYPE_PEER_INFO comes then save.
		 */
		peer->qos_capable = param->qos_capable;
		/*
		 * The following function call assumes that the peer has a
		 * single ID. This is currently true, and
		 * is expected to remain true.
		 */
		htt_peer_qos_update(peer->vdev->pdev->htt_pdev,
				    peer->peer_ids[0],
				    peer->qos_capable);
		break;
	}
	case ol_txrx_peer_update_uapsdMask:
	{
		peer->uapsd_mask = param->uapsd_mask;
		htt_peer_uapsdmask_update(peer->vdev->pdev->htt_pdev,
					  peer->peer_ids[0],
					  peer->uapsd_mask);
		break;
	}
	case ol_txrx_peer_update_peer_security:
	{
		enum ol_sec_type sec_type = param->sec_type;
		enum htt_sec_type peer_sec_type = htt_sec_type_none;

		switch (sec_type) {
		case ol_sec_type_none:
			peer_sec_type = htt_sec_type_none;
			break;
		case ol_sec_type_wep128:
			peer_sec_type = htt_sec_type_wep128;
			break;
		case ol_sec_type_wep104:
			peer_sec_type = htt_sec_type_wep104;
			break;
		case ol_sec_type_wep40:
			peer_sec_type = htt_sec_type_wep40;
			break;
		case ol_sec_type_tkip:
			peer_sec_type = htt_sec_type_tkip;
			break;
		case ol_sec_type_tkip_nomic:
			peer_sec_type = htt_sec_type_tkip_nomic;
			break;
		case ol_sec_type_aes_ccmp:
			peer_sec_type = htt_sec_type_aes_ccmp;
			break;
		case ol_sec_type_wapi:
			peer_sec_type = htt_sec_type_wapi;
			break;
		default:
			peer_sec_type = htt_sec_type_none;
			break;
		}

		peer->security[txrx_sec_ucast].sec_type =
			peer->security[txrx_sec_mcast].sec_type =
				peer_sec_type;

		break;
	}
	default:
	{
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "ERROR: unknown param %d in %s", select,
			  __func__);
		break;
	}
	} /* switch */
	ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);
}

uint8_t
ol_txrx_peer_uapsdmask_get(struct ol_txrx_pdev_t *txrx_pdev, uint16_t peer_id)
{

	struct ol_txrx_peer_t *peer;

	peer = ol_txrx_peer_find_by_id(txrx_pdev, peer_id);
	if (peer)
		return peer->uapsd_mask;
	return 0;
}

uint8_t
ol_txrx_peer_qoscapable_get(struct ol_txrx_pdev_t *txrx_pdev, uint16_t peer_id)
{

	struct ol_txrx_peer_t *peer_t =
		ol_txrx_peer_find_by_id(txrx_pdev, peer_id);
	if (peer_t != NULL)
		return peer_t->qos_capable;
	return 0;
}

/**
 * ol_txrx_peer_free_tids() - free tids for the peer
 * @peer: peer handle
 *
 * Return: None
 */
static inline void ol_txrx_peer_free_tids(ol_txrx_peer_handle peer)
{
	int i = 0;
	/*
	 * 'array' is allocated in addba handler and is supposed to be
	 * freed in delba handler. There is the case (for example, in
	 * SSR) where delba handler is not called. Because array points
	 * to address of 'base' by default and is reallocated in addba
	 * handler later, only free the memory when the array does not
	 * point to base.
	 */
	for (i = 0; i < OL_TXRX_NUM_EXT_TIDS; i++) {
		if (peer->tids_rx_reorder[i].array !=
		    &peer->tids_rx_reorder[i].base) {
			ol_txrx_dbg(
				   "%s, delete reorder arr, tid:%d\n",
				   __func__, i);
			qdf_mem_free(peer->tids_rx_reorder[i].array);
			ol_rx_reorder_init(&peer->tids_rx_reorder[i],
					   (uint8_t)i);
		}
	}
}

/**
 * ol_txrx_peer_release_ref() - release peer reference
 * @peer: peer handle
 *
 * Release peer reference and delete peer if refcount is 0
 *
 * Return: Resulting peer ref_cnt after this function is invoked
 */
int ol_txrx_peer_release_ref(ol_txrx_peer_handle peer,
			     enum peer_debug_id_type debug_id)
{
	int    rc;
	struct ol_txrx_vdev_t *vdev;
	struct ol_txrx_pdev_t *pdev;
	bool ref_silent = false;
	int access_list = 0;

	/* preconditions */
	TXRX_ASSERT2(peer);

	vdev = peer->vdev;
	if (NULL == vdev) {
		ol_txrx_dbg(
			   "The vdev is not present anymore\n");
		return -EINVAL;
	}

	pdev = vdev->pdev;
	if (NULL == pdev) {
		ol_txrx_dbg(
			   "The pdev is not present anymore\n");
		return -EINVAL;
	}

	if (debug_id >= PEER_DEBUG_ID_MAX || debug_id < 0) {
		ol_txrx_err("incorrect debug_id %d ", debug_id);
		return -EINVAL;
	}

	if (debug_id == PEER_DEBUG_ID_OL_RX_THREAD)
		ref_silent = true;

	if (!ref_silent)
		wlan_roam_debug_log(vdev->vdev_id, DEBUG_PEER_UNREF_DELETE,
				    DEBUG_INVALID_PEER_ID, &peer->mac_addr.raw,
				    peer, 0,
				    qdf_atomic_read(&peer->ref_cnt));


	/*
	 * Hold the lock all the way from checking if the peer ref count
	 * is zero until the peer references are removed from the hash
	 * table and vdev list (if the peer ref count is zero).
	 * This protects against a new HL tx operation starting to use the
	 * peer object just after this function concludes it's done being used.
	 * Furthermore, the lock needs to be held while checking whether the
	 * vdev's list of peers is empty, to make sure that list is not modified
	 * concurrently with the empty check.
	 */
	qdf_spin_lock_bh(&pdev->peer_ref_mutex);

	/*
	 * Check for the reference count before deleting the peer
	 * as we noticed that sometimes we are re-entering this
	 * function again which is leading to dead-lock.
	 * (A double-free should never happen, so assert if it does.)
	 */
	rc = qdf_atomic_read(&(peer->ref_cnt));

	if (rc == 0) {
		qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
		ol_txrx_err("The Peer is not present anymore\n");
		qdf_assert(0);
		return -EACCES;
	}
	/*
	 * now decrement rc; this will be the return code.
	 * 0 : peer deleted
	 * >0: peer ref removed, but still has other references
	 * <0: sanity failed - no changes to the state of the peer
	 */
	rc--;

	if (!qdf_atomic_read(&peer->access_list[debug_id])) {
		qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
		ol_txrx_err("peer %p ref was not taken by %d",
			    peer, debug_id);
		ol_txrx_dump_peer_access_list(peer);
		QDF_BUG(0);
		return -EACCES;
	}
	qdf_atomic_dec(&peer->access_list[debug_id]);

	if (qdf_atomic_dec_and_test(&peer->ref_cnt)) {
		u16 peer_id;
		wlan_roam_debug_log(vdev->vdev_id,
				    DEBUG_DELETING_PEER_OBJ,
				    DEBUG_INVALID_PEER_ID,
				    &peer->mac_addr.raw, peer, 0,
				    qdf_atomic_read(&peer->ref_cnt));
		peer_id = peer->local_id;
		/* remove the reference to the peer from the hash table */
		ol_txrx_peer_find_hash_remove(pdev, peer);

		/* remove the peer from its parent vdev's list */
		TAILQ_REMOVE(&peer->vdev->peer_list, peer, peer_list_elem);

		/* cleanup the Rx reorder queues for this peer */
		ol_rx_peer_cleanup(vdev, peer);

		qdf_spinlock_destroy(&peer->peer_info_lock);
		qdf_spinlock_destroy(&peer->bufq_info.bufq_lock);

		/* peer is removed from peer_list */
		qdf_atomic_set(&peer->delete_in_progress, 0);

		/*
		 * Set wait_delete_comp event if the current peer id matches
		 * with registered peer id.
		 */
		if (peer_id == vdev->wait_on_peer_id) {
			qdf_event_set(&vdev->wait_delete_comp);
			vdev->wait_on_peer_id = OL_TXRX_INVALID_LOCAL_PEER_ID;
		}

		qdf_timer_sync_cancel(&peer->peer_unmap_timer);
		qdf_timer_free(&peer->peer_unmap_timer);

		/* check whether the parent vdev has no peers left */
		if (TAILQ_EMPTY(&vdev->peer_list)) {
			/*
			 * Check if the parent vdev was waiting for its peers
			 * to be deleted, in order for it to be deleted too.
			 */
			if (vdev->delete.pending) {
				ol_txrx_vdev_delete_cb vdev_delete_cb =
					vdev->delete.callback;
				void *vdev_delete_context =
					vdev->delete.context;
				/*
				 * Now that there are no references to the peer,
				 * we can release the peer reference lock.
				 */
				qdf_spin_unlock_bh(&pdev->peer_ref_mutex);

				/*
				 * The ol_tx_desc_free might access the invalid
				 * content of vdev referred by tx desc, since
				 * this vdev might be detached in another thread
				 * asynchronous.
				 *
				 * Go through tx desc pool to set corresponding
				 * tx desc's vdev to NULL when detach this vdev,
				 * and add vdev checking in the ol_tx_desc_free
				 * to avoid crash.
				 */
				ol_txrx_tx_desc_reset_vdev(vdev);
				ol_txrx_dbg(
					"%s: deleting vdev object %pK (%02x:%02x:%02x:%02x:%02x:%02x) - its last peer is done",
					__func__, vdev,
					vdev->mac_addr.raw[0],
					vdev->mac_addr.raw[1],
					vdev->mac_addr.raw[2],
					vdev->mac_addr.raw[3],
					vdev->mac_addr.raw[4],
					vdev->mac_addr.raw[5]);
				/* all peers are gone, go ahead and delete it */
				qdf_mem_free(vdev);
				if (vdev_delete_cb)
					vdev_delete_cb(vdev_delete_context);
			} else {
				qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
			}
		} else {
			qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
		}

		ol_txrx_info_high("[%d][%d]: Deleting peer %p ref_cnt -> %d %s",
				  debug_id,
				  qdf_atomic_read(&peer->access_list[debug_id]),
				  peer, rc,
				  qdf_atomic_read(&peer->fw_create_pending)
									== 1 ?
				  "(No Maps received)" : "");

		ol_txrx_peer_tx_queue_free(pdev, peer);

		/* Remove mappings from peer_id to peer object */
		ol_txrx_peer_clear_map_peer(pdev, peer);

		/* Remove peer pointer from local peer ID map */
		ol_txrx_local_peer_id_free(pdev, peer);

		ol_txrx_peer_free_tids(peer);

		ol_txrx_dump_peer_access_list(peer);

		qdf_mem_free(peer);
	} else {
		access_list = qdf_atomic_read(
						&peer->access_list[debug_id]);
		qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
		if (!ref_silent)
			ol_txrx_info_high("[%d][%d]: ref delete peer %p ref_cnt -> %d",
					debug_id,
					access_list,
					peer, rc);
	}
	return rc;
}

/**
 * ol_txrx_clear_peer_internal() - ol internal function to clear peer
 * @peer: pointer to ol txrx peer structure
 *
 * Return: QDF Status
 */
static QDF_STATUS
ol_txrx_clear_peer_internal(struct ol_txrx_peer_t *peer)
{
	p_cds_sched_context sched_ctx = get_cds_sched_ctxt();
	/* Drop pending Rx frames in CDS */
	if (sched_ctx)
		cds_drop_rxpkt_by_staid(sched_ctx, peer->local_id);

	/* Purge the cached rx frame queue */
	ol_txrx_flush_rx_frames(peer, 1);

	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_DISC;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_clear_peer() - clear peer
 * @sta_id: sta id
 *
 * Return: QDF Status
 */
static QDF_STATUS ol_txrx_clear_peer(struct cdp_pdev *ppdev, uint8_t sta_id)
{
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	if (!pdev) {
		ol_txrx_err("%s: Unable to find pdev!",
			   __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (sta_id >= WLAN_MAX_STA_COUNT) {
		ol_txrx_err("Invalid sta id %d", sta_id);
		return QDF_STATUS_E_INVAL;
	}

	peer = ol_txrx_peer_find_by_local_id((struct cdp_pdev *)pdev, sta_id);

	/* Return success, if the peer is already cleared by
	 * data path via peer detach function.
	 */
	if (!peer)
		return QDF_STATUS_SUCCESS;

	return ol_txrx_clear_peer_internal(peer);

}

void peer_unmap_timer_work_function(void *param)
{
	WMA_LOGE("Enter: %s", __func__);
	/* Added for debugging only */
	ol_txrx_dump_peer_access_list(param);
	wlan_roam_debug_dump_table();
	cds_trigger_recovery(QDF_PEER_UNMAP_TIMEDOUT);
}

/**
 * peer_unmap_timer_handler() - peer unmap timer function
 * @data: peer object pointer
 *
 * Return: none
 */
void peer_unmap_timer_handler(void *data)
{
	ol_txrx_peer_handle peer = (ol_txrx_peer_handle)data;
	ol_txrx_pdev_handle txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	ol_txrx_err("all unmap events not received for peer %pK, ref_cnt %d",
		    peer, qdf_atomic_read(&peer->ref_cnt));
	ol_txrx_err("peer %pK (%02x:%02x:%02x:%02x:%02x:%02x)",
		    peer,
		    peer->mac_addr.raw[0], peer->mac_addr.raw[1],
		    peer->mac_addr.raw[2], peer->mac_addr.raw[3],
		    peer->mac_addr.raw[4], peer->mac_addr.raw[5]);
	if (!cds_is_driver_recovering() && !cds_is_fw_down()) {
		qdf_create_work(0, &txrx_pdev->peer_unmap_timer_work,
				peer_unmap_timer_work_function,
				peer);
		qdf_sched_work(0, &txrx_pdev->peer_unmap_timer_work);
	} else {
		ol_txrx_err("Recovery is in progress, ignore!");
	}
}


/**
 * ol_txrx_peer_detach() - Delete a peer's data object.
 * @peer - the object to detach
 * @bitmap - bitmap indicating special handling of request.
 *
 * When the host's control SW disassociates a peer, it calls
 * this function to detach and delete the peer. The reference
 * stored in the control peer object to the data peer
 * object (set up by a call to ol_peer_store()) is provided.
 *
 * Return: None
 */
static void ol_txrx_peer_detach(void *ppeer, uint32_t bitmap)
{
	ol_txrx_peer_handle peer = ppeer;
	struct ol_txrx_vdev_t *vdev = peer->vdev;

	/* redirect peer's rx delivery function to point to a discard func */
	peer->rx_opt_proc = ol_rx_discard;

	peer->valid = 0;

	/* flush all rx packets before clearing up the peer local_id */
	ol_txrx_clear_peer_internal(peer);

	/* debug print to dump rx reorder state */
	/* htt_rx_reorder_log_print(vdev->pdev->htt_pdev); */

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		   "%s:peer %pK (%02x:%02x:%02x:%02x:%02x:%02x)",
		   __func__, peer,
		   peer->mac_addr.raw[0], peer->mac_addr.raw[1],
		   peer->mac_addr.raw[2], peer->mac_addr.raw[3],
		   peer->mac_addr.raw[4], peer->mac_addr.raw[5]);

	qdf_spin_lock_bh(&vdev->pdev->last_real_peer_mutex);
	if (vdev->last_real_peer == peer)
		vdev->last_real_peer = NULL;
	qdf_spin_unlock_bh(&vdev->pdev->last_real_peer_mutex);
	htt_rx_reorder_log_print(peer->vdev->pdev->htt_pdev);

	/*
	 * set delete_in_progress to identify that wma
	 * is waiting for unmap massage for this peer
	 */
	qdf_atomic_set(&peer->delete_in_progress, 1);

	if (!(bitmap & (1 << CDP_PEER_DO_NOT_START_UNMAP_TIMER))) {
		if (vdev->opmode == wlan_op_mode_sta) {
			qdf_mem_copy(&peer->vdev->last_peer_mac_addr,
				&peer->mac_addr,
				sizeof(union ol_txrx_align_mac_addr_t));

			/*
			 * Create a timer to track unmap events when the
			 * sta peer gets deleted.
			 */
			qdf_timer_start(&peer->peer_unmap_timer,
					OL_TXRX_PEER_UNMAP_TIMEOUT);
			ol_txrx_info_high
				("started peer_unmap_timer for peer %pK",
				  peer);
		}
	}

	/*
	 * Remove the reference added during peer_attach.
	 * The peer will still be left allocated until the
	 * PEER_UNMAP message arrives to remove the other
	 * reference, added by the PEER_MAP message.
	 */
	ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);
}

/**
 * ol_txrx_peer_detach_force_delete() - Detach and delete a peer's data object
 * @ppeer - the object to detach
 *
 * Detach a peer and force peer object to be removed. It is called during
 * roaming scenario when the firmware has already deleted a peer.
 * Remove it from the peer_id_to_object map. Peer object is actually freed
 * when last reference is deleted.
 *
 * Return: None
 */
static void ol_txrx_peer_detach_force_delete(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;
	ol_txrx_pdev_handle pdev = peer->vdev->pdev;

	ol_txrx_info_high("%s peer %pK, peer->ref_cnt %d",
		__func__, peer, qdf_atomic_read(&peer->ref_cnt));

	/* Clear the peer_id_to_obj map entries */
	ol_txrx_peer_remove_obj_map_entries(pdev, peer);
	ol_txrx_peer_detach(peer, 1 << CDP_PEER_DELETE_NO_SPECIAL);
}

/**
 * ol_txrx_dump_tx_desc() - dump tx desc total and free count
 * @txrx_pdev: Pointer to txrx pdev
 *
 * Return: none
 */
static void ol_txrx_dump_tx_desc(ol_txrx_pdev_handle pdev_handle)
{
	struct ol_txrx_pdev_t *pdev = (ol_txrx_pdev_handle) pdev_handle;
	uint32_t total, num_free;

	if (ol_cfg_is_high_latency(pdev->ctrl_pdev))
		total = qdf_atomic_read(&pdev->orig_target_tx_credit);
	else
		total = ol_tx_get_desc_global_pool_size(pdev);

	num_free = ol_tx_get_total_free_desc(pdev);

	ol_txrx_info_high(
		   "total tx credit %d num_free %d",
		   total, num_free);

}

/**
 * ol_txrx_wait_for_pending_tx() - wait for tx queue to be empty
 * @timeout: timeout in ms
 *
 * Wait for tx queue to be empty, return timeout error if
 * queue doesn't empty before timeout occurs.
 *
 * Return:
 *    QDF_STATUS_SUCCESS if the queue empties,
 *    QDF_STATUS_E_TIMEOUT in case of timeout,
 *    QDF_STATUS_E_FAULT in case of missing handle
 */
static QDF_STATUS ol_txrx_wait_for_pending_tx(int timeout)
{
	struct ol_txrx_pdev_t *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (txrx_pdev == NULL) {
		ol_txrx_err(
			   "%s: txrx context is null", __func__);
		return QDF_STATUS_E_FAULT;
	}

	while (ol_txrx_get_tx_pending((struct cdp_pdev *)txrx_pdev)) {
		qdf_sleep(OL_ATH_TX_DRAIN_WAIT_DELAY);
		if (timeout <= 0) {
			ol_txrx_err(
				   "%s: tx frames are pending", __func__);
			ol_txrx_dump_tx_desc(txrx_pdev);
			return QDF_STATUS_E_TIMEOUT;
		}
		timeout = timeout - OL_ATH_TX_DRAIN_WAIT_DELAY;
	}
	return QDF_STATUS_SUCCESS;
}

#ifndef QCA_WIFI_3_0_EMU
#define SUSPEND_DRAIN_WAIT 500
#else
#define SUSPEND_DRAIN_WAIT 3000
#endif

#ifdef FEATURE_RUNTIME_PM
/**
 * ol_txrx_runtime_suspend() - ensure TXRX is ready to runtime suspend
 * @txrx_pdev: TXRX pdev context
 *
 * TXRX is ready to runtime suspend if there are no pending packets
 * in the tx queue.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ol_txrx_runtime_suspend(struct cdp_pdev *ppdev)
{
	struct ol_txrx_pdev_t *txrx_pdev = (struct ol_txrx_pdev_t *)ppdev;

	if (ol_txrx_get_tx_pending((struct cdp_pdev *)txrx_pdev))
		return QDF_STATUS_E_BUSY;
	else
		return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_runtime_resume() - ensure TXRX is ready to runtime resume
 * @txrx_pdev: TXRX pdev context
 *
 * This is a dummy function for symmetry.
 *
 * Return: QDF_STATUS_SUCCESS
 */
static QDF_STATUS ol_txrx_runtime_resume(struct cdp_pdev *ppdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ol_txrx_bus_suspend() - bus suspend
 * @ppdev: TXRX pdev context
 *
 * Ensure that ol_txrx is ready for bus suspend
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ol_txrx_bus_suspend(struct cdp_pdev *ppdev)
{
	return ol_txrx_wait_for_pending_tx(SUSPEND_DRAIN_WAIT);
}

/**
 * ol_txrx_bus_resume() - bus resume
 * @ppdev: TXRX pdev context
 *
 * Dummy function for symetry
 *
 * Return: QDF_STATUS_SUCCESS
 */
static QDF_STATUS ol_txrx_bus_resume(struct cdp_pdev *ppdev)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_get_tx_pending - Get the number of pending transmit
 * frames that are awaiting completion.
 *
 * @pdev - the data physical device object
 *  Mainly used in clean up path to make sure all buffers have been freed
 *
 * Return: count of pending frames
 */
int ol_txrx_get_tx_pending(struct cdp_pdev *ppdev)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	uint32_t total;

	if (ol_cfg_is_high_latency(pdev->ctrl_pdev))
		total = qdf_atomic_read(&pdev->orig_target_tx_credit);
	else
		total = ol_tx_get_desc_global_pool_size(pdev);

	return total - ol_tx_get_total_free_desc(pdev);
}

void ol_txrx_discard_tx_pending(ol_txrx_pdev_handle pdev_handle)
{
	ol_tx_desc_list tx_descs;
	/*
	 * First let hif do the qdf_atomic_dec_and_test(&tx_desc->ref_cnt)
	 * then let htt do the qdf_atomic_dec_and_test(&tx_desc->ref_cnt)
	 * which is tha same with normal data send complete path
	 */
	htt_tx_pending_discard(pdev_handle->htt_pdev);

	TAILQ_INIT(&tx_descs);
	ol_tx_queue_discard(pdev_handle, true, &tx_descs);
	/* Discard Frames in Discard List */
	ol_tx_desc_frame_list_free(pdev_handle, &tx_descs, 1 /* error */);

	ol_tx_discard_target_frms(pdev_handle);
}

static inline
uint64_t ol_txrx_stats_ptr_to_u64(struct ol_txrx_stats_req_internal *req)
{
	return (uint64_t) ((size_t) req);
}

static inline
struct ol_txrx_stats_req_internal *ol_txrx_u64_to_stats_ptr(uint64_t cookie)
{
	return (struct ol_txrx_stats_req_internal *)((size_t) cookie);
}

#ifdef currently_unused
void
ol_txrx_fw_stats_cfg(ol_txrx_vdev_handle vdev,
		     uint8_t cfg_stats_type, uint32_t cfg_val)
{
	uint64_t dummy_cookie = 0;

	htt_h2t_dbg_stats_get(vdev->pdev->htt_pdev, 0 /* upload mask */,
			      0 /* reset mask */,
			      cfg_stats_type, cfg_val, dummy_cookie);
}
#endif

static A_STATUS
ol_txrx_fw_stats_get(struct cdp_vdev *pvdev, struct ol_txrx_stats_req *req,
			bool per_vdev, bool response_expected)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	uint64_t cookie;
	struct ol_txrx_stats_req_internal *non_volatile_req;

	if (!pdev ||
	    req->stats_type_upload_mask >= 1 << HTT_DBG_NUM_STATS ||
	    req->stats_type_reset_mask >= 1 << HTT_DBG_NUM_STATS) {
		return A_ERROR;
	}

	/*
	 * Allocate a non-transient stats request object.
	 * (The one provided as an argument is likely allocated on the stack.)
	 */
	non_volatile_req = qdf_mem_malloc(sizeof(*non_volatile_req));
	if (!non_volatile_req)
		return A_NO_MEMORY;

	/* copy the caller's specifications */
	non_volatile_req->base = *req;
	non_volatile_req->serviced = 0;
	non_volatile_req->offset = 0;

	/* use the non-volatile request object's address as the cookie */
	cookie = ol_txrx_stats_ptr_to_u64(non_volatile_req);

	if (response_expected) {
		qdf_spin_lock_bh(&pdev->req_list_spinlock);
		TAILQ_INSERT_TAIL(&pdev->req_list, non_volatile_req, req_list_elem);
		pdev->req_list_depth++;
		qdf_spin_unlock_bh(&pdev->req_list_spinlock);
	}

	if (htt_h2t_dbg_stats_get(pdev->htt_pdev,
				  req->stats_type_upload_mask,
				  req->stats_type_reset_mask,
				  HTT_H2T_STATS_REQ_CFG_STAT_TYPE_INVALID, 0,
				  cookie)) {
		if (response_expected) {
			qdf_spin_lock_bh(&pdev->req_list_spinlock);
			TAILQ_REMOVE(&pdev->req_list, non_volatile_req, req_list_elem);
			pdev->req_list_depth--;
			qdf_spin_unlock_bh(&pdev->req_list_spinlock);
		}

		qdf_mem_free(non_volatile_req);
		return A_ERROR;
	}

	if (response_expected == false)
		qdf_mem_free(non_volatile_req);

	return A_OK;
}

void
ol_txrx_fw_stats_handler(ol_txrx_pdev_handle pdev,
			 uint64_t cookie, uint8_t *stats_info_list)
{
	enum htt_dbg_stats_type type;
	enum htt_dbg_stats_status status;
	int length;
	uint8_t *stats_data;
	struct ol_txrx_stats_req_internal *req, *tmp;
	int more = 0;
	int found = 0;

	req = ol_txrx_u64_to_stats_ptr(cookie);

	qdf_spin_lock_bh(&pdev->req_list_spinlock);
	TAILQ_FOREACH(tmp, &pdev->req_list, req_list_elem) {
		if (req == tmp) {
			found = 1;
			break;
		}
	}
	qdf_spin_unlock_bh(&pdev->req_list_spinlock);

	if (!found) {
		ol_txrx_err(
			"req(%pK) from firmware can't be found in the list\n", req);
		return;
	}

	do {
		htt_t2h_dbg_stats_hdr_parse(stats_info_list, &type, &status,
					    &length, &stats_data);
		if (status == HTT_DBG_STATS_STATUS_SERIES_DONE)
			break;
		if (status == HTT_DBG_STATS_STATUS_PRESENT ||
		    status == HTT_DBG_STATS_STATUS_PARTIAL) {
			uint8_t *buf;
			int bytes = 0;

			if (status == HTT_DBG_STATS_STATUS_PARTIAL)
				more = 1;
			if (req->base.print.verbose || req->base.print.concise)
				/* provide the header along with the data */
				htt_t2h_stats_print(stats_info_list,
						    req->base.print.concise);

			switch (type) {
			case HTT_DBG_STATS_WAL_PDEV_TXRX:
				bytes = sizeof(struct wlan_dbg_stats);
				if (req->base.copy.buf) {
					int lmt;

					lmt = sizeof(struct wlan_dbg_stats);
					if (req->base.copy.byte_limit < lmt)
						lmt = req->base.copy.byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, lmt);
				}
				break;
			case HTT_DBG_STATS_RX_REORDER:
				bytes = sizeof(struct rx_reorder_stats);
				if (req->base.copy.buf) {
					int lmt;

					lmt = sizeof(struct rx_reorder_stats);
					if (req->base.copy.byte_limit < lmt)
						lmt = req->base.copy.byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, lmt);
				}
				break;
			case HTT_DBG_STATS_RX_RATE_INFO:
				bytes = sizeof(wlan_dbg_rx_rate_info_t);
				if (req->base.copy.buf) {
					int lmt;

					lmt = sizeof(wlan_dbg_rx_rate_info_t);
					if (req->base.copy.byte_limit < lmt)
						lmt = req->base.copy.byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, lmt);
				}
				break;

			case HTT_DBG_STATS_TX_RATE_INFO:
				bytes = sizeof(wlan_dbg_tx_rate_info_t);
				if (req->base.copy.buf) {
					int lmt;

					lmt = sizeof(wlan_dbg_tx_rate_info_t);
					if (req->base.copy.byte_limit < lmt)
						lmt = req->base.copy.byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, lmt);
				}
				break;

			case HTT_DBG_STATS_TX_PPDU_LOG:
				bytes = 0;
				/* TO DO: specify how many bytes are present */
				/* TO DO: add copying to the requestor's buf */

			case HTT_DBG_STATS_RX_REMOTE_RING_BUFFER_INFO:
				bytes = sizeof(struct
						rx_remote_buffer_mgmt_stats);
				if (req->base.copy.buf) {
					int limit;

					limit = sizeof(struct
						rx_remote_buffer_mgmt_stats);
					if (req->base.copy.byte_limit < limit)
						limit = req->base.copy.
							byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, limit);
				}
				break;

			case HTT_DBG_STATS_TXBF_INFO:
				bytes = sizeof(struct wlan_dbg_txbf_data_stats);
				if (req->base.copy.buf) {
					int limit;

					limit = sizeof(struct
						wlan_dbg_txbf_data_stats);
					if (req->base.copy.byte_limit < limit)
						limit = req->base.copy.
							byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, limit);
				}
				break;

			case HTT_DBG_STATS_SND_INFO:
				bytes = sizeof(struct wlan_dbg_txbf_snd_stats);
				if (req->base.copy.buf) {
					int limit;

					limit = sizeof(struct
						wlan_dbg_txbf_snd_stats);
					if (req->base.copy.byte_limit < limit)
						limit = req->base.copy.
							byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, limit);
				}
				break;

			case HTT_DBG_STATS_TX_SELFGEN_INFO:
				bytes = sizeof(struct
					wlan_dbg_tx_selfgen_stats);
				if (req->base.copy.buf) {
					int limit;

					limit = sizeof(struct
						wlan_dbg_tx_selfgen_stats);
					if (req->base.copy.byte_limit < limit)
						limit = req->base.copy.
							byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, limit);
				}
				break;

			case HTT_DBG_STATS_ERROR_INFO:
				bytes =
				  sizeof(struct wlan_dbg_wifi2_error_stats);
				if (req->base.copy.buf) {
					int limit;

					limit = sizeof(struct
						wlan_dbg_wifi2_error_stats);
					if (req->base.copy.byte_limit < limit)
						limit = req->base.copy.
							byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, limit);
				}
				break;

			case HTT_DBG_STATS_TXBF_MUSU_NDPA_PKT:
				bytes =
				  sizeof(struct rx_txbf_musu_ndpa_pkts_stats);
				if (req->base.copy.buf) {
					int limit;

					limit = sizeof(struct
						rx_txbf_musu_ndpa_pkts_stats);
					if (req->base.copy.byte_limit <	limit)
						limit =
						req->base.copy.byte_limit;
					buf = req->base.copy.buf + req->offset;
					qdf_mem_copy(buf, stats_data, limit);
				}
				break;

			default:
				break;
			}
			buf = req->base.copy.buf ?
				req->base.copy.buf : stats_data;
			if (req->base.callback.fp)
				req->base.callback.fp(req->base.callback.ctxt,
						      type, buf, bytes);
		}
		stats_info_list += length;
	} while (1);

	if (!more) {
		qdf_spin_lock_bh(&pdev->req_list_spinlock);
		TAILQ_FOREACH(tmp, &pdev->req_list, req_list_elem) {
			if (req == tmp) {
				TAILQ_REMOVE(&pdev->req_list, req, req_list_elem);
				pdev->req_list_depth--;
				qdf_mem_free(req);
				break;
			}
		}
		qdf_spin_unlock_bh(&pdev->req_list_spinlock);
	}
}

#ifndef ATH_PERF_PWR_OFFLOAD /*---------------------------------------------*/
int ol_txrx_debug(ol_txrx_vdev_handle vdev, int debug_specs)
{
	if (debug_specs & TXRX_DBG_MASK_OBJS) {
#if defined(TXRX_DEBUG_LEVEL) && TXRX_DEBUG_LEVEL > 5
		ol_txrx_pdev_display(vdev->pdev, 0);
#else
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
			  "The pdev,vdev,peer display functions are disabled.\n To enable them, recompile with TXRX_DEBUG_LEVEL > 5");
#endif
	}
	if (debug_specs & TXRX_DBG_MASK_STATS)
		ol_txrx_stats_display(vdev->pdev,
				      QDF_STATS_VERBOSITY_LEVEL_HIGH);
	if (debug_specs & TXRX_DBG_MASK_PROT_ANALYZE) {
#if defined(ENABLE_TXRX_PROT_ANALYZE)
		ol_txrx_prot_ans_display(vdev->pdev);
#else
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
			  "txrx protocol analysis is disabled.\n To enable it, recompile with ENABLE_TXRX_PROT_ANALYZE defined");
#endif
	}
	if (debug_specs & TXRX_DBG_MASK_RX_REORDER_TRACE) {
#if defined(ENABLE_RX_REORDER_TRACE)
		ol_rx_reorder_trace_display(vdev->pdev, 0, 0);
#else
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
			  "rx reorder seq num trace is disabled.\n To enable it, recompile with ENABLE_RX_REORDER_TRACE defined");
#endif

	}
	return 0;
}
#endif

#ifdef currently_unused
int ol_txrx_aggr_cfg(ol_txrx_vdev_handle vdev,
		     int max_subfrms_ampdu, int max_subfrms_amsdu)
{
	return htt_h2t_aggr_cfg_msg(vdev->pdev->htt_pdev,
				    max_subfrms_ampdu, max_subfrms_amsdu);
}
#endif

#if defined(TXRX_DEBUG_LEVEL) && TXRX_DEBUG_LEVEL > 5
void ol_txrx_pdev_display(ol_txrx_pdev_handle pdev, int indent)
{
	struct ol_txrx_vdev_t *vdev;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*s%s:\n", indent, " ", "txrx pdev");
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*spdev object: %pK", indent + 4, " ", pdev);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*svdev list:", indent + 4, " ");
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		      ol_txrx_vdev_display(vdev, indent + 8);
	}
	ol_txrx_peer_find_display(pdev, indent + 4);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*stx desc pool: %d elems @ %pK", indent + 4, " ",
		  pdev->tx_desc.pool_size, pdev->tx_desc.array);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW, " ");
	htt_display(pdev->htt_pdev, indent);
}

void ol_txrx_vdev_display(ol_txrx_vdev_handle vdev, int indent)
{
	struct ol_txrx_peer_t *peer;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*stxrx vdev: %pK\n", indent, " ", vdev);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*sID: %d\n", indent + 4, " ", vdev->vdev_id);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*sMAC addr: %d:%d:%d:%d:%d:%d",
		  indent + 4, " ",
		  vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
		  vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
		  vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*speer list:", indent + 4, " ");
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		      ol_txrx_peer_display(peer, indent + 8);
	}
}

void ol_txrx_peer_display(ol_txrx_peer_handle peer, int indent)
{
	int i;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%*stxrx peer: %pK", indent, " ", peer);
	for (i = 0; i < MAX_NUM_PEER_ID_PER_PEER; i++) {
		if (peer->peer_ids[i] != HTT_INVALID_PEER) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
				  "%*sID: %d", indent + 4, " ",
				  peer->peer_ids[i]);
		}
	}
}
#endif /* TXRX_DEBUG_LEVEL */

/**
 * ol_txrx_stats() - update ol layer stats
 * @vdev_id: vdev_id
 * @buffer: pointer to buffer
 * @buf_len: length of the buffer
 *
 * Return: length of string
 */
static int
ol_txrx_stats(uint8_t vdev_id, char *buffer, unsigned int buf_len)
{
	uint32_t len = 0;

	struct ol_txrx_vdev_t *vdev =
			(struct ol_txrx_vdev_t *)
			ol_txrx_get_vdev_from_vdev_id(vdev_id);

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: vdev is NULL", __func__);
		snprintf(buffer, buf_len, "vdev not found");
		return len;
	}

	len = scnprintf(buffer, buf_len,
			"\n\nTXRX stats:\nllQueue State : %s\npause %u unpause %u\noverflow %u\nllQueue timer state : %s",
			((vdev->ll_pause.is_q_paused == false) ?
			 "UNPAUSED" : "PAUSED"),
			vdev->ll_pause.q_pause_cnt,
			vdev->ll_pause.q_unpause_cnt,
			vdev->ll_pause.q_overflow_cnt,
			((vdev->ll_pause.is_q_timer_on == false)
			 ? "NOT-RUNNING" : "RUNNING"));
	return len;
}

#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
/**
 * ol_txrx_disp_peer_cached_bufq_stats() - display peer cached_bufq stats
 * @peer: peer pointer
 *
 * Return: None
 */
static void ol_txrx_disp_peer_cached_bufq_stats(struct ol_txrx_peer_t *peer)
{
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "cached_bufq: curr %d drops %d hwm %d whatifs %d thresh %d",
		  peer->bufq_info.curr,
		  peer->bufq_info.dropped,
		  peer->bufq_info.high_water_mark,
		  peer->bufq_info.qdepth_no_thresh,
		  peer->bufq_info.thresh);
}

/**
 * ol_txrx_disp_peer_stats() - display peer stats
 * @pdev: pdev pointer
 *
 * Return: None
 */
static void ol_txrx_disp_peer_stats(ol_txrx_pdev_handle pdev)
{	int i;
	struct ol_txrx_peer_t *peer;
	struct hif_opaque_softc *osc =  cds_get_context(QDF_MODULE_ID_HIF);

	if (osc && hif_is_load_or_unload_in_progress(HIF_GET_SOFTC(osc)))
		return;

	for (i = 0; i < OL_TXRX_NUM_LOCAL_PEER_IDS; i++) {
		qdf_spin_lock_bh(&pdev->local_peer_ids.lock);
		peer = pdev->local_peer_ids.map[i];
		if (peer) {
			qdf_spin_lock_bh(&pdev->peer_ref_mutex);
			ol_txrx_peer_get_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);
			qdf_spin_unlock_bh(&pdev->peer_ref_mutex);
		}
		qdf_spin_unlock_bh(&pdev->local_peer_ids.lock);

		if (peer) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"stats: peer 0x%pK local peer id %d", peer, i);
			ol_txrx_disp_peer_cached_bufq_stats(peer);
			ol_txrx_peer_release_ref(peer,
						 PEER_DEBUG_ID_OL_INTERNAL);
		}
	}
}
#else
static void ol_txrx_disp_peer_stats(ol_txrx_pdev_handle pdev)
{
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		  "peer stats not supported w/o QCA_SUPPORT_TXRX_LOCAL_PEER_ID");
}
#endif

void ol_txrx_stats_display(ol_txrx_pdev_handle pdev,
			   enum qdf_stats_verbosity_level level)
{
	u64 tx_dropped =
		pdev->stats.pub.tx.dropped.download_fail.pkts
		  + pdev->stats.pub.tx.dropped.target_discard.pkts
		  + pdev->stats.pub.tx.dropped.no_ack.pkts
		  + pdev->stats.pub.tx.dropped.others.pkts;

	if (level == QDF_STATS_VERBOSITY_LEVEL_LOW) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
			  "STATS |%u %u|TX: %lld tso %lld ok %lld drops(%u-%lld %u-%lld %u-%lld ?-%lld hR-%lld)|RX: %lld drops(E %lld PI %lld ME %lld) fwd(S %d F %d SF %d)|",
			  pdev->tx_desc.num_free,
			  pdev->tx_desc.pool_size,
			  pdev->stats.pub.tx.from_stack.pkts,
			  pdev->stats.pub.tx.tso.tso_pkts.pkts,
			  pdev->stats.pub.tx.delivered.pkts,
			  htt_tx_status_download_fail,
			  pdev->stats.pub.tx.dropped.download_fail.pkts,
			  htt_tx_status_discard,
			  pdev->stats.pub.tx.dropped.target_discard.pkts,
			  htt_tx_status_no_ack,
			  pdev->stats.pub.tx.dropped.no_ack.pkts,
			  pdev->stats.pub.tx.dropped.others.pkts,
			  pdev->stats.pub.tx.dropped.host_reject.pkts,
			  pdev->stats.pub.rx.delivered.pkts,
			  pdev->stats.pub.rx.dropped_err.pkts,
			  pdev->stats.pub.rx.dropped_peer_invalid.pkts,
			  pdev->stats.pub.rx.dropped_mic_err.pkts,
			  pdev->stats.pub.rx.intra_bss_fwd.packets_stack,
			  pdev->stats.pub.rx.intra_bss_fwd.packets_fwd,
			  pdev->stats.pub.rx.intra_bss_fwd.packets_stack_n_fwd);
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "TX PATH Statistics:");
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "sent %lld msdus (%lld B), host rejected %lld (%lld B), dropped %lld (%lld B)",
		  pdev->stats.pub.tx.from_stack.pkts,
		  pdev->stats.pub.tx.from_stack.bytes,
		  pdev->stats.pub.tx.dropped.host_reject.pkts,
		  pdev->stats.pub.tx.dropped.host_reject.bytes,
		  tx_dropped,
		  pdev->stats.pub.tx.dropped.download_fail.bytes
		  + pdev->stats.pub.tx.dropped.target_discard.bytes
		  + pdev->stats.pub.tx.dropped.no_ack.bytes);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "successfully delivered: %lld (%lld B), download fail: %lld (%lld B), target discard: %lld (%lld B), no ack: %lld (%lld B) others: %lld (%lld B)",
		  pdev->stats.pub.tx.delivered.pkts,
		  pdev->stats.pub.tx.delivered.bytes,
		  pdev->stats.pub.tx.dropped.download_fail.pkts,
		  pdev->stats.pub.tx.dropped.download_fail.bytes,
		  pdev->stats.pub.tx.dropped.target_discard.pkts,
		  pdev->stats.pub.tx.dropped.target_discard.bytes,
		  pdev->stats.pub.tx.dropped.no_ack.pkts,
		  pdev->stats.pub.tx.dropped.no_ack.bytes,
		  pdev->stats.pub.tx.dropped.others.pkts,
		  pdev->stats.pub.tx.dropped.others.bytes);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "Tx completions per HTT message:\n"
		  "Single Packet  %d\n"
		  " 2-10 Packets  %d\n"
		  "11-20 Packets  %d\n"
		  "21-30 Packets  %d\n"
		  "31-40 Packets  %d\n"
		  "41-50 Packets  %d\n"
		  "51-60 Packets  %d\n"
		  "  60+ Packets  %d\n",
		  pdev->stats.pub.tx.comp_histogram.pkts_1,
		  pdev->stats.pub.tx.comp_histogram.pkts_2_10,
		  pdev->stats.pub.tx.comp_histogram.pkts_11_20,
		  pdev->stats.pub.tx.comp_histogram.pkts_21_30,
		  pdev->stats.pub.tx.comp_histogram.pkts_31_40,
		  pdev->stats.pub.tx.comp_histogram.pkts_41_50,
		  pdev->stats.pub.tx.comp_histogram.pkts_51_60,
		  pdev->stats.pub.tx.comp_histogram.pkts_61_plus);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "RX PATH Statistics:");
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "%lld ppdus, %lld mpdus, %lld msdus, %lld bytes\n"
		  "dropped: err %lld (%lld B), peer_invalid %lld (%lld B), mic_err %lld (%lld B)\n"
		  "msdus with frag_ind: %d msdus with offload_ind: %d",
		  pdev->stats.priv.rx.normal.ppdus,
		  pdev->stats.priv.rx.normal.mpdus,
		  pdev->stats.pub.rx.delivered.pkts,
		  pdev->stats.pub.rx.delivered.bytes,
		  pdev->stats.pub.rx.dropped_err.pkts,
		  pdev->stats.pub.rx.dropped_err.bytes,
		  pdev->stats.pub.rx.dropped_peer_invalid.pkts,
		  pdev->stats.pub.rx.dropped_peer_invalid.bytes,
		  pdev->stats.pub.rx.dropped_mic_err.pkts,
		  pdev->stats.pub.rx.dropped_mic_err.bytes,
		  pdev->stats.pub.rx.msdus_with_frag_ind,
		  pdev->stats.pub.rx.msdus_with_offload_ind);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "  fwd to stack %d, fwd to fw %d, fwd to stack & fw  %d\n",
		  pdev->stats.pub.rx.intra_bss_fwd.packets_stack,
		  pdev->stats.pub.rx.intra_bss_fwd.packets_fwd,
		  pdev->stats.pub.rx.intra_bss_fwd.packets_stack_n_fwd);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
		  "Rx packets per HTT message:\n"
		  "Single Packet  %d\n"
		  " 2-10 Packets  %d\n"
		  "11-20 Packets  %d\n"
		  "21-30 Packets  %d\n"
		  "31-40 Packets  %d\n"
		  "41-50 Packets  %d\n"
		  "51-60 Packets  %d\n"
		  "  60+ Packets  %d\n",
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_1,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_2_10,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_11_20,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_21_30,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_31_40,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_41_50,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_51_60,
		  pdev->stats.pub.rx.rx_ind_histogram.pkts_61_plus);

	ol_txrx_disp_peer_stats(pdev);
}

void ol_txrx_stats_clear(ol_txrx_pdev_handle pdev)
{
	qdf_mem_zero(&pdev->stats, sizeof(pdev->stats));
}

#if defined(ENABLE_TXRX_PROT_ANALYZE)

void ol_txrx_prot_ans_display(ol_txrx_pdev_handle pdev)
{
	ol_txrx_prot_an_display(pdev->prot_an_tx_sent);
	ol_txrx_prot_an_display(pdev->prot_an_rx_sent);
}

#endif /* ENABLE_TXRX_PROT_ANALYZE */

#ifdef QCA_SUPPORT_PEER_DATA_RX_RSSI
int16_t ol_txrx_peer_rssi(ol_txrx_peer_handle peer)
{
	return (peer->rssi_dbm == HTT_RSSI_INVALID) ?
	       OL_TXRX_RSSI_INVALID : peer->rssi_dbm;
}
#endif /* #ifdef QCA_SUPPORT_PEER_DATA_RX_RSSI */

#ifdef QCA_ENABLE_OL_TXRX_PEER_STATS
A_STATUS
ol_txrx_peer_stats_copy(ol_txrx_pdev_handle pdev,
			ol_txrx_peer_handle peer, ol_txrx_peer_stats_t *stats)
{
	qdf_assert(pdev && peer && stats);
	qdf_spin_lock_bh(&pdev->peer_stat_mutex);
	qdf_mem_copy(stats, &peer->stats, sizeof(*stats));
	qdf_spin_unlock_bh(&pdev->peer_stat_mutex);
	return A_OK;
}
#endif /* QCA_ENABLE_OL_TXRX_PEER_STATS */

static void ol_vdev_rx_set_intrabss_fwd(struct cdp_vdev *pvdev, bool val)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	if (NULL == vdev)
		return;

	vdev->disable_intrabss_fwd = val;
}

/**
 * ol_txrx_update_mac_id() - update mac_id for vdev
 * @vdev_id: vdev id
 * @mac_id: mac id
 *
 * Return: none
 */
static void ol_txrx_update_mac_id(uint8_t vdev_id, uint8_t mac_id)
{
	struct ol_txrx_vdev_t *vdev =
			(struct ol_txrx_vdev_t *)
			ol_txrx_get_vdev_from_vdev_id(vdev_id);

	if (NULL == vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid vdev_id %d", __func__, vdev_id);
		return;
	}
	vdev->mac_id = mac_id;
}

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL

/**
 * ol_txrx_get_vdev_from_sta_id() - get vdev from sta_id
 * @sta_id: sta_id
 *
 * Return: vdev handle
 *            NULL if not found.
 */
static ol_txrx_vdev_handle ol_txrx_get_vdev_from_sta_id(uint8_t sta_id)
{
	struct ol_txrx_peer_t *peer = NULL;
	ol_txrx_pdev_handle pdev = NULL;

	if (sta_id >= WLAN_MAX_STA_COUNT) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Invalid sta id passed");
		return NULL;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "PDEV not found for sta_id [%d]", sta_id);
		return NULL;
	}

	peer = ol_txrx_peer_find_by_local_id((struct cdp_pdev *)pdev, sta_id);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			  "PEER [%d] not found", sta_id);
		return NULL;
	}

	return peer->vdev;
}

/**
 * ol_txrx_register_tx_flow_control() - register tx flow control callback
 * @vdev_id: vdev_id
 * @flowControl: flow control callback
 * @osif_fc_ctx: callback context
 * @flow_control_is_pause: is vdev paused by flow control
 *
 * Return: 0 for sucess or error code
 */
static int ol_txrx_register_tx_flow_control(uint8_t vdev_id,
	ol_txrx_tx_flow_control_fp flowControl, void *osif_fc_ctx,
	ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause)
{
	struct ol_txrx_vdev_t *vdev =
		(struct ol_txrx_vdev_t *)ol_txrx_get_vdev_from_vdev_id(vdev_id);

	if (NULL == vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid vdev_id %d", __func__, vdev_id);
		return -EINVAL;
	}

	qdf_spin_lock_bh(&vdev->flow_control_lock);
	vdev->osif_flow_control_cb = flowControl;
	vdev->osif_flow_control_is_pause = flow_control_is_pause;
	vdev->osif_fc_ctx = osif_fc_ctx;
	qdf_spin_unlock_bh(&vdev->flow_control_lock);
	return 0;
}

/**
 * ol_txrx_de_register_tx_flow_control_cb() - deregister tx flow control
 *                                            callback
 * @vdev_id: vdev_id
 *
 * Return: 0 for success or error code
 */
static int ol_txrx_deregister_tx_flow_control_cb(uint8_t vdev_id)
{
	struct ol_txrx_vdev_t *vdev =
		(struct ol_txrx_vdev_t *)ol_txrx_get_vdev_from_vdev_id(vdev_id);

	if (NULL == vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid vdev_id", __func__);
		return -EINVAL;
	}

	qdf_spin_lock_bh(&vdev->flow_control_lock);
	vdev->osif_flow_control_cb = NULL;
	vdev->osif_flow_control_is_pause = NULL;
	vdev->osif_fc_ctx = NULL;
	qdf_spin_unlock_bh(&vdev->flow_control_lock);
	return 0;
}

/**
 * ol_txrx_get_tx_resource() - if tx resource less than low_watermark
 * @sta_id: sta id
 * @low_watermark: low watermark
 * @high_watermark_offset: high watermark offset value
 *
 * Return: true/false
 */
static bool
ol_txrx_get_tx_resource(uint8_t sta_id,
			unsigned int low_watermark,
			unsigned int high_watermark_offset)
{
	ol_txrx_vdev_handle vdev = ol_txrx_get_vdev_from_sta_id(sta_id);

	if (NULL == vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid sta_id %d", __func__, sta_id);
		/* Return true so caller do not understand that resource
		 * is less than low_watermark.
		 * sta_id validation will be done in ol_tx_send_data_frame
		 * and if sta_id is not registered then host will drop
		 * packet.
		 */
		return true;
	}

	qdf_spin_lock_bh(&vdev->pdev->tx_mutex);

	if (vdev->pdev->tx_desc.num_free < (uint16_t) low_watermark) {
		vdev->tx_fl_lwm = (uint16_t) low_watermark;
		vdev->tx_fl_hwm =
			(uint16_t) (low_watermark + high_watermark_offset);
		/* Not enough free resource, stop TX OS Q */
		qdf_atomic_set(&vdev->os_q_paused, 1);
		qdf_spin_unlock_bh(&vdev->pdev->tx_mutex);
		return false;
	}
	qdf_spin_unlock_bh(&vdev->pdev->tx_mutex);
	return true;
}

/**
 * ol_txrx_ll_set_tx_pause_q_depth() - set pause queue depth
 * @vdev_id: vdev id
 * @pause_q_depth: pause queue depth
 *
 * Return: 0 for success or error code
 */
static int
ol_txrx_ll_set_tx_pause_q_depth(uint8_t vdev_id, int pause_q_depth)
{
	struct ol_txrx_vdev_t *vdev =
		(struct ol_txrx_vdev_t *)ol_txrx_get_vdev_from_vdev_id(vdev_id);

	if (NULL == vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid vdev_id %d", __func__, vdev_id);
		return -EINVAL;
	}

	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	vdev->ll_pause.max_q_depth = pause_q_depth;
	qdf_spin_unlock_bh(&vdev->ll_pause.mutex);

	return 0;
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * ol_txrx_display_stats() - Display OL TXRX display stats
 * @value: Module id for which stats needs to be displayed
 *
 * Return: status
 */
static QDF_STATUS
ol_txrx_display_stats(void *soc, uint16_t value,
		      enum qdf_stats_verbosity_level verb_level)
{
	ol_txrx_pdev_handle pdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: pdev is NULL", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (value) {
	case CDP_TXRX_PATH_STATS:
		ol_txrx_stats_display(pdev, verb_level);
		break;
	case CDP_TXRX_TSO_STATS:
		ol_txrx_stats_display_tso(pdev);
		break;
	case CDP_DUMP_TX_FLOW_POOL_INFO:
		ol_tx_dump_flow_pool_info((void *)pdev);
		break;
	case CDP_TXRX_DESC_STATS:
		qdf_nbuf_tx_desc_count_display();
		break;
	case CDP_WLAN_RX_BUF_DEBUG_STATS:
		htt_display_rx_buf_debug(pdev->htt_pdev);
		break;
#ifdef CONFIG_HL_SUPPORT
	case CDP_SCHEDULER_STATS:
		ol_tx_sched_cur_state_display(pdev);
		ol_tx_sched_stats_display(pdev);
		break;
	case CDP_TX_QUEUE_STATS:
		ol_tx_queue_log_display(pdev);
		break;
#ifdef FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL
	case CDP_CREDIT_STATS:
		ol_tx_dump_group_credit_stats(pdev);
		break;
#endif

#ifdef DEBUG_HL_LOGGING
	case CDP_BUNDLE_STATS:
		htt_dump_bundle_stats(pdev->htt_pdev);
		break;
#endif
#endif
	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}
	return status;
}

/**
 * ol_txrx_clear_stats() - Clear OL TXRX stats
 * @value: Module id for which stats needs to be cleared
 *
 * Return: None
 */
static void ol_txrx_clear_stats(uint16_t value)
{
	ol_txrx_pdev_handle pdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: pdev is NULL", __func__);
		return;
	}

	switch (value) {
	case CDP_TXRX_PATH_STATS:
		ol_txrx_stats_clear(pdev);
		break;
	case CDP_TXRX_TSO_STATS:
		ol_txrx_tso_stats_clear(pdev);
		break;
	case CDP_DUMP_TX_FLOW_POOL_INFO:
		ol_tx_clear_flow_pool_stats();
		break;
	case CDP_TXRX_DESC_STATS:
		qdf_nbuf_tx_desc_count_clear();
		break;
#ifdef CONFIG_HL_SUPPORT
	case CDP_SCHEDULER_STATS:
		ol_tx_sched_stats_clear(pdev);
		break;
	case CDP_TX_QUEUE_STATS:
		ol_tx_queue_log_clear(pdev);
		break;
#ifdef FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL
	case CDP_CREDIT_STATS:
		ol_tx_clear_group_credit_stats(pdev);
		break;
#endif
	case CDP_BUNDLE_STATS:
		htt_clear_bundle_stats(pdev->htt_pdev);
		break;
#endif
	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

}

/**
 * ol_txrx_drop_nbuf_list() - drop an nbuf list
 * @buf_list: buffer list to be dropepd
 *
 * Return: int (number of bufs dropped)
 */
static inline int ol_txrx_drop_nbuf_list(qdf_nbuf_t buf_list)
{
	int num_dropped = 0;
	qdf_nbuf_t buf, next_buf;
	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	buf = buf_list;
	while (buf) {
		QDF_NBUF_CB_RX_PEER_CACHED_FRM(buf) = 1;
		next_buf = qdf_nbuf_queue_next(buf);
		if (pdev)
			TXRX_STATS_MSDU_INCR(pdev,
				 rx.dropped_peer_invalid, buf);
		qdf_nbuf_free(buf);
		buf = next_buf;
		num_dropped++;
	}
	return num_dropped;
}

/**
 * ol_rx_data_cb() - data rx callback
 * @peer: peer
 * @buf_list: buffer list
 * @staid: Station id
 *
 * Return: None
 */
static void ol_rx_data_cb(struct ol_txrx_pdev_t *pdev,
			  qdf_nbuf_t buf_list, uint16_t staid)
{
	void *osif_dev;
	uint8_t drop_count = 0;
	qdf_nbuf_t buf, next_buf;
	QDF_STATUS ret;
	ol_txrx_rx_fp data_rx = NULL;
	struct ol_txrx_peer_t *peer;

	if (qdf_unlikely(!pdev))
		goto free_buf;

	/* Do not use peer directly. Derive peer from staid to
	 * make sure that peer is valid.
	 */
	peer = ol_txrx_peer_get_ref_by_local_id((struct cdp_pdev *)pdev,
			staid, PEER_DEBUG_ID_OL_RX_THREAD);
	if (!peer)
		goto free_buf;

	qdf_spin_lock_bh(&peer->peer_info_lock);
	if (qdf_unlikely(!(peer->state >= OL_TXRX_PEER_STATE_CONN) ||
					 !peer->vdev->rx)) {
		qdf_spin_unlock_bh(&peer->peer_info_lock);
		ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_RX_THREAD);
		goto free_buf;
	}

	data_rx = peer->vdev->rx;
	osif_dev = peer->vdev->osif_dev;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	qdf_spin_lock_bh(&peer->bufq_info.bufq_lock);
	if (!list_empty(&peer->bufq_info.cached_bufq)) {
		qdf_spin_unlock_bh(&peer->bufq_info.bufq_lock);
		/* Flush the cached frames to HDD before passing new rx frame */
		ol_txrx_flush_rx_frames(peer, 0);
	} else
		qdf_spin_unlock_bh(&peer->bufq_info.bufq_lock);

	ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_RX_THREAD);

	buf = buf_list;
	while (buf) {
		next_buf = qdf_nbuf_queue_next(buf);
		qdf_nbuf_set_next(buf, NULL);   /* Add NULL terminator */
		ret = data_rx(osif_dev, buf);
		if (ret != QDF_STATUS_SUCCESS) {
			ol_txrx_err("Frame Rx to HDD failed");
			if (pdev)
				TXRX_STATS_MSDU_INCR(pdev, rx.dropped_err, buf);
			qdf_nbuf_free(buf);
		}
		buf = next_buf;
	}
	return;

free_buf:
	drop_count = ol_txrx_drop_nbuf_list(buf_list);
	ol_txrx_warn("%s:Dropped frames %u", __func__, drop_count);
}

/* print for every 16th packet */
#define OL_TXRX_PRINT_RATE_LIMIT_THRESH 0x0f
struct ol_rx_cached_buf *cache_buf;

/** helper function to drop packets
 *  Note: caller must hold the cached buq lock before invoking
 *  this function. Also, it assumes that the pointers passed in
 *  are valid (non-NULL)
 */
static inline void ol_txrx_drop_frames(
					struct ol_txrx_cached_bufq_t *bufqi,
					qdf_nbuf_t rx_buf_list)
{
	uint32_t dropped = ol_txrx_drop_nbuf_list(rx_buf_list);

	bufqi->dropped += dropped;
	bufqi->qdepth_no_thresh += dropped;

	if (bufqi->qdepth_no_thresh > bufqi->high_water_mark)
		bufqi->high_water_mark = bufqi->qdepth_no_thresh;
}

static QDF_STATUS ol_txrx_enqueue_rx_frames(
					struct ol_txrx_peer_t *peer,
					struct ol_txrx_cached_bufq_t *bufqi,
					qdf_nbuf_t rx_buf_list)
{
	struct ol_rx_cached_buf *cache_buf;
	qdf_nbuf_t buf, next_buf;
	static uint32_t count;

	if ((count++ & OL_TXRX_PRINT_RATE_LIMIT_THRESH) == 0)
		ol_txrx_info_high(
		   "Data on the peer before it is registered bufq->curr %d bufq->drops %d",
		   bufqi->curr, bufqi->dropped);

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	if (bufqi->curr >= bufqi->thresh) {
		ol_txrx_drop_frames(bufqi, rx_buf_list);
		qdf_spin_unlock_bh(&bufqi->bufq_lock);
		return QDF_STATUS_E_FAULT;
	}
	qdf_spin_unlock_bh(&bufqi->bufq_lock);

	buf = rx_buf_list;
	while (buf) {
		next_buf = qdf_nbuf_queue_next(buf);
		cache_buf = qdf_mem_malloc(sizeof(*cache_buf));
		if (!cache_buf) {
			ol_txrx_err(
				"Failed to allocate buf to cache the rx frames");
			qdf_nbuf_free(buf);
		} else {
			/* Add NULL terminator */
			qdf_nbuf_set_next(buf, NULL);
			cache_buf->buf = buf;
			if (peer && peer->valid) {
				qdf_spin_lock_bh(&bufqi->bufq_lock);
				list_add_tail(&cache_buf->list,
				      &bufqi->cached_bufq);
				bufqi->curr++;
				qdf_spin_unlock_bh(&bufqi->bufq_lock);
			} else {
				qdf_mem_free(cache_buf);
				rx_buf_list = buf;
				qdf_nbuf_set_next(rx_buf_list, next_buf);
				qdf_spin_lock_bh(&bufqi->bufq_lock);
				ol_txrx_drop_frames(bufqi, rx_buf_list);
				qdf_spin_unlock_bh(&bufqi->bufq_lock);
				return QDF_STATUS_E_FAULT;
			}
		}
		buf = next_buf;
	}
	return QDF_STATUS_SUCCESS;
}
/**
 * ol_rx_data_process() - process rx frame
 * @peer: peer
 * @rx_buf_list: rx buffer list
 *
 * Return: None
 */
void ol_rx_data_process(struct ol_txrx_peer_t *peer,
			qdf_nbuf_t rx_buf_list)
{
	/*
	 * Firmware data path active response will use shim RX thread
	 * T2H MSG running on SIRQ context,
	 * IPA kernel module API should not be called on SIRQ CTXT
	 */
	ol_txrx_rx_fp data_rx = NULL;
	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	uint8_t drop_count;

	if ((!peer) || (!pdev)) {
		ol_txrx_err("peer/pdev is NULL");
		goto drop_rx_buf;
	}

	qdf_assert(peer->vdev);

	qdf_spin_lock_bh(&peer->peer_info_lock);
	if (peer->state >= OL_TXRX_PEER_STATE_CONN)
		data_rx = peer->vdev->rx;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	/*
	 * If there is a data frame from peer before the peer is
	 * registered for data service, enqueue them on to pending queue
	 * which will be flushed to HDD once that station is registered.
	 */
	if (!data_rx) {
		if (ol_txrx_enqueue_rx_frames(peer, &peer->bufq_info,
					      rx_buf_list)
				!= QDF_STATUS_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: failed to enqueue rx frm to cached_bufq",
				  __func__);
	} else {
#ifdef QCA_CONFIG_SMP
		/*
		 * If the kernel is SMP, schedule rx thread to
		 * better use multicores.
		 */
		if (!ol_cfg_is_rx_thread_enabled(pdev->ctrl_pdev)) {
			ol_rx_data_cb(pdev, rx_buf_list, peer->local_id);
		} else {
			p_cds_sched_context sched_ctx =
				get_cds_sched_ctxt();
			struct cds_ol_rx_pkt *pkt;

			if (unlikely(!sched_ctx))
				goto drop_rx_buf;

			pkt = cds_alloc_ol_rx_pkt(sched_ctx);
			if (!pkt) {
				ol_txrx_info_high(
					   "No available Rx message buffer");
				goto drop_rx_buf;
			}
			pkt->callback = (cds_ol_rx_thread_cb)
					ol_rx_data_cb;
			pkt->context = (void *)pdev;
			pkt->Rxpkt = (void *)rx_buf_list;
			pkt->staId = peer->local_id;
			cds_indicate_rxpkt(sched_ctx, pkt);
		}
#else                           /* QCA_CONFIG_SMP */
		ol_rx_data_cb(pdev, rx_buf_list, peer->local_id);
#endif /* QCA_CONFIG_SMP */
	}

	return;

drop_rx_buf:
	drop_count = ol_txrx_drop_nbuf_list(rx_buf_list);
	ol_txrx_info_high("Dropped rx packets %u", drop_count);
}

/**
 * ol_txrx_register_peer() - register peer
 * @sta_desc: sta descriptor
 *
 * Return: QDF Status
 */
static QDF_STATUS ol_txrx_register_peer(struct ol_txrx_desc_type *sta_desc)
{
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	union ol_txrx_peer_update_param_t param;
	struct privacy_exemption privacy_filter;

	if (!pdev) {
		ol_txrx_err("Pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (sta_desc->sta_id >= WLAN_MAX_STA_COUNT) {
		ol_txrx_err("Invalid sta id :%d",
			 sta_desc->sta_id);
		return QDF_STATUS_E_INVAL;
	}

	peer = ol_txrx_peer_find_by_local_id((struct cdp_pdev *)pdev,
					     sta_desc->sta_id);
	if (!peer)
		return QDF_STATUS_E_FAULT;

	qdf_spin_lock_bh(&peer->peer_info_lock);
	peer->state = OL_TXRX_PEER_STATE_CONN;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	param.qos_capable = sta_desc->is_qos_enabled;
	ol_txrx_peer_update(peer->vdev, peer->mac_addr.raw, &param,
			    ol_txrx_peer_update_qos_capable);

	if (sta_desc->is_wapi_supported) {
		/*Privacy filter to accept unencrypted WAI frames */
		privacy_filter.ether_type = ETHERTYPE_WAI;
		privacy_filter.filter_type = PRIVACY_FILTER_ALWAYS;
		privacy_filter.packet_type = PRIVACY_FILTER_PACKET_BOTH;
		ol_txrx_set_privacy_filters(peer->vdev, &privacy_filter, 1);
	}

	ol_txrx_flush_rx_frames(peer, 0);
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_register_ocb_peer - Function to register the OCB peer
 * @mac_addr: MAC address of the self peer
 * @peer_id: Pointer to the peer ID
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS ol_txrx_register_ocb_peer(uint8_t *mac_addr,
				     uint8_t *peer_id)
{
	ol_txrx_pdev_handle pdev;
	ol_txrx_peer_handle peer;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		ol_txrx_err("%s: Unable to find pdev!",
			   __func__);
		return QDF_STATUS_E_FAILURE;
	}

	peer = ol_txrx_find_peer_by_addr((struct cdp_pdev *)pdev,
					 mac_addr, peer_id);
	if (!peer) {
		ol_txrx_err("%s: Unable to find OCB peer!",
			   __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ol_txrx_set_ocb_peer(pdev, peer);

	/* Set peer state to connected */
	ol_txrx_peer_state_update((struct cdp_pdev *)pdev, peer->mac_addr.raw,
				  OL_TXRX_PEER_STATE_AUTH);

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_set_ocb_peer - Function to store the OCB peer
 * @pdev: Handle to the HTT instance
 * @peer: Pointer to the peer
 */
void ol_txrx_set_ocb_peer(struct ol_txrx_pdev_t *pdev,
			  struct ol_txrx_peer_t *peer)
{
	if (pdev == NULL)
		return;

	pdev->ocb_peer = peer;
	pdev->ocb_peer_valid = (NULL != peer);
}

/**
 * ol_txrx_get_ocb_peer - Function to retrieve the OCB peer
 * @pdev: Handle to the HTT instance
 * @peer: Pointer to the returned peer
 *
 * Return: true if the peer is valid, false if not
 */
bool ol_txrx_get_ocb_peer(struct ol_txrx_pdev_t *pdev,
			  struct ol_txrx_peer_t **peer)
{
	int rc;

	if ((pdev == NULL) || (peer == NULL)) {
		rc = false;
		goto exit;
	}

	if (pdev->ocb_peer_valid) {
		*peer = pdev->ocb_peer;
		rc = true;
	} else {
		rc = false;
	}

exit:
	return rc;
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * ol_txrx_register_pause_cb() - register pause callback
 * @pause_cb: pause callback
 *
 * Return: QDF status
 */
static QDF_STATUS ol_txrx_register_pause_cb(struct cdp_soc_t *soc,
	tx_pause_callback pause_cb)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev || !pause_cb) {
		ol_txrx_err("pdev or pause_cb is NULL");
		return QDF_STATUS_E_INVAL;
	}
	pdev->pause_cb = pause_cb;
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ol_register_data_stall_detect_cb() - register data stall callback
 * @data_stall_detect_callback: data stall callback function
 *
 *
 * Return: QDF_STATUS Enumeration
 */
static QDF_STATUS ol_register_data_stall_detect_cb(
				data_stall_detect_cb data_stall_detect_callback)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (pdev == NULL) {
		ol_txrx_err("%s: pdev NULL!", __func__);
		return QDF_STATUS_E_INVAL;
	}
	pdev->data_stall_detect_callback = data_stall_detect_callback;
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_deregister_data_stall_detect_cb() - de-register data stall callback
 * @data_stall_detect_callback: data stall callback function
 *
 *
 * Return: QDF_STATUS Enumeration
 */
static QDF_STATUS ol_deregister_data_stall_detect_cb(
				data_stall_detect_cb data_stall_detect_callback)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (pdev == NULL) {
		ol_txrx_err("%s: pdev NULL!", __func__);
		return QDF_STATUS_E_INVAL;
	}
	pdev->data_stall_detect_callback = NULL;
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_post_data_stall_event() - post data stall event
 * @indicator: Module triggering data stall
 * @data_stall_type: data stall event type
 * @pdev_id: pdev id
 * @vdev_id_bitmap: vdev id bitmap
 * @recovery_type: data stall recovery type
 *
 * Return: None
 */
static void ol_txrx_post_data_stall_event(
				enum data_stall_log_event_indicator indicator,
				enum data_stall_log_event_type data_stall_type,
				uint32_t pdev_id, uint32_t vdev_id_bitmap,
				enum data_stall_log_recovery_type recovery_type)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;
	struct data_stall_event_info *data_stall_info;
	ol_txrx_pdev_handle pdev;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: pdev is NULL.", __func__);
		return;
	}
	data_stall_info = qdf_mem_malloc(sizeof(*data_stall_info));
	if (!data_stall_info) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: data_stall_info is NULL.", __func__);
		return;
	}
	data_stall_info->indicator = indicator;
	data_stall_info->data_stall_type = data_stall_type;
	data_stall_info->vdev_id_bitmap = vdev_id_bitmap;
	data_stall_info->pdev_id = pdev_id;
	data_stall_info->recovery_type = recovery_type;

	if (data_stall_info->data_stall_type ==
				DATA_STALL_LOG_FW_RX_REFILL_FAILED)
		htt_log_rx_ring_info(pdev->htt_pdev);

	sys_build_message_header(SYS_MSG_ID_DATA_STALL_MSG, &msg);
	/* Save callback and data */
	msg.callback = pdev->data_stall_detect_callback;
	msg.bodyptr = data_stall_info;
	msg.bodyval = 0;

	status = scheduler_post_msg(QDF_MODULE_ID_SYS, &msg);

	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to post data stall msg to SYS", __func__);
		qdf_mem_free(data_stall_info);
	}
}

void
ol_txrx_dump_pkt(qdf_nbuf_t nbuf, uint32_t nbuf_paddr, int len)
{
	qdf_print("%s: Pkt: VA 0x%pK PA 0x%llx len %d\n", __func__,
		  qdf_nbuf_data(nbuf), (unsigned long long int)nbuf_paddr, len);
	print_hex_dump(KERN_DEBUG, "Pkt:   ", DUMP_PREFIX_ADDRESS, 16, 4,
		       qdf_nbuf_data(nbuf), len, true);
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
bool
ol_txrx_fwd_desc_thresh_check(struct ol_txrx_vdev_t *vdev)
{
	struct ol_tx_flow_pool_t *pool;
	bool enough_desc_flag;

	if (!vdev)
		return false;

	pool = vdev->pool;

	if (!pool)
		return false;

	qdf_spin_lock_bh(&pool->flow_pool_lock);
	enough_desc_flag = (pool->avail_desc < (pool->stop_th +
				OL_TX_NON_FWD_RESERVE))
		? false : true;
	qdf_spin_unlock_bh(&pool->flow_pool_lock);
	return enough_desc_flag;
}
#else
bool ol_txrx_fwd_desc_thresh_check(struct ol_txrx_vdev_t *vdev)
{
	return true;
}
#endif

/**
 * ol_txrx_get_vdev_from_vdev_id() - get vdev from vdev_id
 * @vdev_id: vdev_id
 *
 * Return: vdev handle
 *            NULL if not found.
 */
struct cdp_vdev *ol_txrx_get_vdev_from_vdev_id(uint8_t vdev_id)
{
	ol_txrx_pdev_handle pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	ol_txrx_vdev_handle vdev = NULL;

	if (qdf_unlikely(!pdev))
		return NULL;

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (vdev->vdev_id == vdev_id)
			break;
	}

	return (struct cdp_vdev *)vdev;
}

/**
 * ol_txrx_set_wisa_mode() - set wisa mode
 * @vdev: vdev handle
 * @enable: enable flag
 *
 * Return: QDF STATUS
 */
static QDF_STATUS ol_txrx_set_wisa_mode(struct cdp_vdev *pvdev, bool enable)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	if (!vdev)
		return QDF_STATUS_E_INVAL;

	vdev->is_wisa_mode_enable = enable;
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_get_vdev_id() - get interface id from interface context
 * @pvdev: vdev handle
 *
 * Return: virtual interface id
 */
static uint16_t ol_txrx_get_vdev_id(struct cdp_vdev *pvdev)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;

	return vdev->vdev_id;
}

/**
 * ol_txrx_last_assoc_received() - get time of last assoc received
 * @ppeer: peer handle
 *
 * Return: pointer of the time of last assoc received
 */
static qdf_time_t *ol_txrx_last_assoc_received(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	return &peer->last_assoc_rcvd;
}

/**
 * ol_txrx_last_disassoc_received() - get time of last disassoc received
 * @ppeer: peer handle
 *
 * Return: pointer of the time of last disassoc received
 */
static qdf_time_t *ol_txrx_last_disassoc_received(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	return &peer->last_disassoc_rcvd;
}

/**
 * ol_txrx_last_deauth_received() - get time of last deauth received
 * @ppeer: peer handle
 *
 * Return: pointer of the time of last deauth received
 */
static qdf_time_t *ol_txrx_last_deauth_received(void *ppeer)
{
	ol_txrx_peer_handle peer = ppeer;

	return &peer->last_deauth_rcvd;
}

/**
 * ol_txrx_soc_attach_target() - attach soc target
 * @soc: soc handle
 *
 * MCL legacy OL do nothing here
 *
 * Return: 0
 */
static int ol_txrx_soc_attach_target(ol_txrx_soc_handle soc)
{
	/* MCL legacy OL do nothing here */
	return 0;
}

/**
 * ol_txrx_soc_detach() - detach soc target
 * @soc: soc handle
 *
 * MCL legacy OL do nothing here
 *
 * Return: noe
 */
static void ol_txrx_soc_detach(void *soc)
{
	qdf_mem_free(soc);
}

/**
 * ol_txrx_pkt_log_con_service() - connect packet log service
 * @ppdev: physical device handle
 * @scn: device context
 *
 * Return: noe
 */
static void ol_txrx_pkt_log_con_service(struct cdp_pdev *ppdev, void *scn)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;

	htt_pkt_log_init((struct cdp_pdev *)pdev, scn);
	pktlog_htc_attach();
}

/* OL wrapper functions for CDP abstraction */
/**
 * ol_txrx_wrapper_flush_rx_frames() - flush rx frames on the queue
 * @peer: peer handle
 * @drop: rx packets drop or deliver
 *
 * Return: none
 */
static void ol_txrx_wrapper_flush_rx_frames(void *peer, bool drop)
{
	ol_txrx_flush_rx_frames((ol_txrx_peer_handle)peer, drop);
}

/**
 * ol_txrx_wrapper_get_vdev_from_vdev_id() - get vdev instance from vdev id
 * @ppdev: pdev handle
 * @vdev_id: interface id
 *
 * Return: virtual interface instance
 */
static
struct cdp_vdev *ol_txrx_wrapper_get_vdev_from_vdev_id(struct cdp_pdev *ppdev,
		uint8_t vdev_id)
{
	return ol_txrx_get_vdev_from_vdev_id(vdev_id);
}

/**
 * ol_txrx_wrapper_register_peer() - register peer
 * @pdev: pdev handle
 * @sta_desc: peer description
 *
 * Return: QDF STATUS
 */
static QDF_STATUS ol_txrx_wrapper_register_peer(struct cdp_pdev *pdev,
		struct ol_txrx_desc_type *sta_desc)
{
	return ol_txrx_register_peer(sta_desc);
}

/**
 * ol_txrx_wrapper_peer_find_by_local_id() - Find a txrx peer handle
 * @pdev - the data physical device object
 * @local_peer_id - the ID txrx assigned locally to the peer in question
 *
 * The control SW typically uses the txrx peer handle to refer to the peer.
 * In unusual circumstances, if it is infeasible for the control SW maintain
 * the txrx peer handle but it can maintain a small integer local peer ID,
 * this function allows the peer handled to be retrieved, based on the local
 * peer ID.
 *
 * @return handle to the txrx peer object
 */
static void *
ol_txrx_wrapper_peer_find_by_local_id(struct cdp_pdev *pdev,
				      uint8_t local_peer_id)
{
	return (void *)ol_txrx_peer_find_by_local_id(
		pdev, local_peer_id);
}

/**
 * ol_txrx_wrapper_cfg_is_high_latency() - device is high or low latency device
 * @pdev: pdev handle
 *
 * Return: 1 high latency bus
 *         0 low latency bus
 */
static int ol_txrx_wrapper_cfg_is_high_latency(struct cdp_cfg *cfg_pdev)
{
	return ol_cfg_is_high_latency(cfg_pdev);
}

/**
 * ol_txrx_wrapper_peer_state_update() - specify the peer's authentication state
 * @data_peer - which peer has changed its state
 * @state - the new state of the peer
 *
 *  Specify the peer's authentication state (none, connected, authenticated)
 *  to allow the data SW to determine whether to filter out invalid data frames.
 *  (In the "connected" state, where security is enabled, but authentication
 *  has not completed, tx and rx data frames other than EAPOL or WAPI should
 *  be discarded.)
 *  This function is only relevant for systems in which the tx and rx filtering
 *  are done in the host rather than in the target.
 *
 * Return: QDF Status
 */
static QDF_STATUS ol_txrx_wrapper_peer_state_update(struct cdp_pdev *pdev,
		uint8_t *peer_mac, enum ol_txrx_peer_state state)
{
	return ol_txrx_peer_state_update(pdev,
			peer_mac, state);
}

/**
 * ol_txrx_wrapper_find_peer_by_addr() - find peer instance by address
 * @pdev: pdev handle
 * @peer_addr: peer address wnat to find
 * @peer_id: peer id
 *
 * Return: peer instance pointer
 */
static void *ol_txrx_wrapper_find_peer_by_addr(struct cdp_pdev *pdev,
		uint8_t *peer_addr, uint8_t *peer_id)
{
	return ol_txrx_find_peer_by_addr(pdev,
				peer_addr, peer_id);
}

/**
 * ol_txrx_wrapper_peer_get_ref_by_addr() - get peer reference by address
 * @pdev: pdev handle
 * @peer_addr: peer address we want to find
 * @peer_id: peer id
 * @debug_id: peer debug id for tracking
 *
 * Return: peer instance pointer
 */
static void *
ol_txrx_wrapper_peer_get_ref_by_addr(struct cdp_pdev *pdev,
				     u8 *peer_addr, uint8_t *peer_id,
				     enum peer_debug_id_type debug_id)
{
	return ol_txrx_peer_get_ref_by_addr((ol_txrx_pdev_handle)pdev,
					    peer_addr, peer_id, debug_id);
}

/**
 * ol_txrx_wrapper_peer_release_ref() - release peer reference
 * @peer: peer handle
 * @debug_id: peer debug id for tracking
 *
 * Release peer ref acquired by peer get ref api
 *
 * Return: void
 */
static void ol_txrx_wrapper_peer_release_ref(void *peer,
					     enum peer_debug_id_type debug_id)
{
	ol_txrx_peer_release_ref(peer, debug_id);
}

/**
 * ol_txrx_wrapper_set_flow_control_parameters() - set flow control parameters
 * @cfg_ctx: cfg context
 * @cfg_param: cfg parameters
 *
 * Return: none
 */
static void
ol_txrx_wrapper_set_flow_control_parameters(struct cdp_cfg *cfg_pdev,
		void *cfg_param)
{
	return ol_tx_set_flow_control_parameters(
		cfg_pdev,
		(struct txrx_pdev_cfg_param_t *)cfg_param);
}

#ifdef WDI_EVENT_ENABLE
void *ol_get_pldev(struct cdp_pdev *txrx_pdev)
{
	struct ol_txrx_pdev_t *pdev =
				 (struct ol_txrx_pdev_t *)txrx_pdev;
	if (pdev != NULL)
		return pdev->pl_dev;

	return NULL;
}
#endif

static struct cdp_cmn_ops ol_ops_cmn = {
	.txrx_soc_attach_target = ol_txrx_soc_attach_target,
	.txrx_vdev_attach = ol_txrx_vdev_attach,
	.txrx_vdev_detach = ol_txrx_vdev_detach,
	.txrx_pdev_attach = ol_txrx_pdev_attach,
	.txrx_pdev_attach_target = ol_txrx_pdev_attach_target,
	.txrx_pdev_post_attach = ol_txrx_pdev_post_attach,
	.txrx_pdev_pre_detach = ol_txrx_pdev_pre_detach,
	.txrx_pdev_detach = ol_txrx_pdev_detach,
	.txrx_peer_create = ol_txrx_peer_attach,
	.txrx_peer_setup = NULL,
	.txrx_peer_teardown = NULL,
	.txrx_peer_delete = ol_txrx_peer_detach,
	.txrx_vdev_register = ol_txrx_vdev_register,
	.txrx_soc_detach = ol_txrx_soc_detach,
	.txrx_get_vdev_mac_addr = ol_txrx_get_vdev_mac_addr,
	.txrx_get_vdev_from_vdev_id = ol_txrx_wrapper_get_vdev_from_vdev_id,
	.txrx_get_ctrl_pdev_from_vdev = ol_txrx_get_ctrl_pdev_from_vdev,
	.txrx_mgmt_send_ext = ol_txrx_mgmt_send_ext,
	.txrx_mgmt_tx_cb_set = ol_txrx_mgmt_tx_cb_set,
	.txrx_data_tx_cb_set = ol_txrx_data_tx_cb_set,
	.txrx_get_tx_pending = ol_txrx_get_tx_pending,
	.flush_cache_rx_queue = ol_txrx_flush_cache_rx_queue,
	.txrx_fw_stats_get = ol_txrx_fw_stats_get,
	.display_stats = ol_txrx_display_stats,
	/* TODO: Add other functions */
};

static struct cdp_misc_ops ol_ops_misc = {
	.set_ibss_vdev_heart_beat_timer =
		ol_txrx_set_ibss_vdev_heart_beat_timer,
#ifdef CONFIG_HL_SUPPORT
	.set_wmm_param = ol_txrx_set_wmm_param,
#endif /* CONFIG_HL_SUPPORT */
	.bad_peer_txctl_set_setting = ol_txrx_bad_peer_txctl_set_setting,
	.bad_peer_txctl_update_threshold =
		ol_txrx_bad_peer_txctl_update_threshold,
	.hl_tdls_flag_reset = ol_txrx_hl_tdls_flag_reset,
	.tx_non_std = ol_tx_non_std,
	.get_vdev_id = ol_txrx_get_vdev_id,
	.set_wisa_mode = ol_txrx_set_wisa_mode,
	.txrx_data_stall_cb_register = ol_register_data_stall_detect_cb,
	.txrx_data_stall_cb_deregister = ol_deregister_data_stall_detect_cb,
	.txrx_post_data_stall_event = ol_txrx_post_data_stall_event,
#ifdef FEATURE_RUNTIME_PM
	.runtime_suspend = ol_txrx_runtime_suspend,
	.runtime_resume = ol_txrx_runtime_resume,
#endif /* FEATURE_RUNTIME_PM */
	.get_opmode = ol_txrx_get_opmode,
	.mark_first_wakeup_packet = ol_tx_mark_first_wakeup_packet,
	.update_mac_id = ol_txrx_update_mac_id,
	.flush_rx_frames = ol_txrx_wrapper_flush_rx_frames,
	.get_intra_bss_fwd_pkts_count = ol_get_intra_bss_fwd_pkts_count,
	.pkt_log_init = htt_pkt_log_init,
	.pkt_log_con_service = ol_txrx_pkt_log_con_service
};

static struct cdp_flowctl_ops ol_ops_flowctl = {
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	.register_pause_cb = ol_txrx_register_pause_cb,
	.set_desc_global_pool_size = ol_tx_set_desc_global_pool_size,
	.dump_flow_pool_info = ol_tx_dump_flow_pool_info,
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
};

static struct cdp_lflowctl_ops ol_ops_l_flowctl = {
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	.register_tx_flow_control = ol_txrx_register_tx_flow_control,
	.deregister_tx_flow_control_cb = ol_txrx_deregister_tx_flow_control_cb,
	.flow_control_cb = ol_txrx_flow_control_cb,
	.get_tx_resource = ol_txrx_get_tx_resource,
	.ll_set_tx_pause_q_depth = ol_txrx_ll_set_tx_pause_q_depth,
	.vdev_flush = ol_txrx_vdev_flush,
	.vdev_pause = ol_txrx_vdev_pause,
	.vdev_unpause = ol_txrx_vdev_unpause
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
};

#ifdef IPA_OFFLOAD
static struct cdp_ipa_ops ol_ops_ipa = {
	.ipa_get_resource = ol_txrx_ipa_uc_get_resource,
	.ipa_set_doorbell_paddr = ol_txrx_ipa_uc_set_doorbell_paddr,
	.ipa_set_active = ol_txrx_ipa_uc_set_active,
	.ipa_op_response = ol_txrx_ipa_uc_op_response,
	.ipa_register_op_cb = ol_txrx_ipa_uc_register_op_cb,
	.ipa_get_stat = ol_txrx_ipa_uc_get_stat,
	.ipa_tx_data_frame = ol_tx_send_ipa_data_frame,
	.ipa_set_uc_tx_partition_base = ol_cfg_set_ipa_uc_tx_partition_base,
	.ipa_enable_autonomy = ol_txrx_ipa_enable_autonomy,
	.ipa_disable_autonomy = ol_txrx_ipa_disable_autonomy,
	.ipa_setup = ol_txrx_ipa_setup,
	.ipa_cleanup = ol_txrx_ipa_cleanup,
	.ipa_setup_iface = ol_txrx_ipa_setup_iface,
	.ipa_cleanup_iface = ol_txrx_ipa_cleanup_iface,
	.ipa_enable_pipes = ol_txrx_ipa_enable_pipes,
	.ipa_disable_pipes = ol_txrx_ipa_disable_pipes,
	.ipa_set_perf_level = ol_txrx_ipa_set_perf_level,
#ifdef FEATURE_METERING
	.ipa_uc_get_share_stats = ol_txrx_ipa_uc_get_share_stats,
	.ipa_uc_set_quota = ol_txrx_ipa_uc_set_quota
#endif
};
#endif

static struct cdp_bus_ops ol_ops_bus = {
	.bus_suspend = ol_txrx_bus_suspend,
	.bus_resume = ol_txrx_bus_resume
};

static struct cdp_ocb_ops ol_ops_ocb = {
	.set_ocb_chan_info = ol_txrx_set_ocb_chan_info,
	.get_ocb_chan_info = ol_txrx_get_ocb_chan_info
};

static struct cdp_throttle_ops ol_ops_throttle = {
#ifdef QCA_SUPPORT_TX_THROTTLE
	.throttle_init_period = ol_tx_throttle_init_period,
	.throttle_set_level = ol_tx_throttle_set_level
#endif /* QCA_SUPPORT_TX_THROTTLE */
};

static struct cdp_mob_stats_ops ol_ops_mob_stats = {
	.clear_stats = ol_txrx_clear_stats,
	.stats = ol_txrx_stats
};

static struct cdp_cfg_ops ol_ops_cfg = {
	.set_cfg_rx_fwd_disabled = ol_set_cfg_rx_fwd_disabled,
	.set_cfg_packet_log_enabled = ol_set_cfg_packet_log_enabled,
	.cfg_attach = ol_pdev_cfg_attach,
	.vdev_rx_set_intrabss_fwd = ol_vdev_rx_set_intrabss_fwd,
	.is_rx_fwd_disabled = ol_txrx_is_rx_fwd_disabled,
	.tx_set_is_mgmt_over_wmi_enabled = ol_tx_set_is_mgmt_over_wmi_enabled,
	.is_high_latency = ol_txrx_wrapper_cfg_is_high_latency,
	.set_flow_control_parameters =
		ol_txrx_wrapper_set_flow_control_parameters,
	.set_flow_steering = ol_set_cfg_flow_steering,
	.set_ptp_rx_opt_enabled = ol_set_cfg_ptp_rx_opt_enabled,
};

static struct cdp_peer_ops ol_ops_peer = {
	.register_peer = ol_txrx_wrapper_register_peer,
	.clear_peer = ol_txrx_clear_peer,
	.peer_get_ref_by_addr = ol_txrx_wrapper_peer_get_ref_by_addr,
	.peer_release_ref = ol_txrx_wrapper_peer_release_ref,
	.find_peer_by_addr = ol_txrx_wrapper_find_peer_by_addr,
	.find_peer_by_addr_and_vdev = ol_txrx_find_peer_by_addr_and_vdev,
	.local_peer_id = ol_txrx_local_peer_id,
	.peer_find_by_local_id = ol_txrx_wrapper_peer_find_by_local_id,
	.peer_state_update = ol_txrx_wrapper_peer_state_update,
	.get_vdevid = ol_txrx_get_vdevid,
	.get_vdev_by_sta_id = ol_txrx_get_vdev_by_sta_id,
	.register_ocb_peer = ol_txrx_register_ocb_peer,
	.peer_get_peer_mac_addr = ol_txrx_peer_get_peer_mac_addr,
	.get_peer_state = ol_txrx_get_peer_state,
	.get_vdev_for_peer = ol_txrx_get_vdev_for_peer,
	.update_ibss_add_peer_num_of_vdev =
		ol_txrx_update_ibss_add_peer_num_of_vdev,
	.remove_peers_for_vdev = ol_txrx_remove_peers_for_vdev,
	.remove_peers_for_vdev_no_lock = ol_txrx_remove_peers_for_vdev_no_lock,
#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)
	.copy_mac_addr_raw = ol_txrx_copy_mac_addr_raw,
	.add_last_real_peer = ol_txrx_add_last_real_peer,
	.is_vdev_restore_last_peer = is_vdev_restore_last_peer,
	.update_last_real_peer = ol_txrx_update_last_real_peer,
#endif /* CONFIG_HL_SUPPORT */
	.last_assoc_received = ol_txrx_last_assoc_received,
	.last_disassoc_received = ol_txrx_last_disassoc_received,
	.last_deauth_received = ol_txrx_last_deauth_received,
	.peer_detach_force_delete = ol_txrx_peer_detach_force_delete,
};

static struct cdp_tx_delay_ops ol_ops_delay = {
#ifdef QCA_COMPUTE_TX_DELAY
	.tx_delay = ol_tx_delay,
	.tx_delay_hist = ol_tx_delay_hist,
	.tx_packet_count = ol_tx_packet_count,
	.tx_set_compute_interval = ol_tx_set_compute_interval
#endif /* QCA_COMPUTE_TX_DELAY */
};

static struct cdp_pmf_ops ol_ops_pmf = {
	.get_pn_info = ol_txrx_get_pn_info
};

static struct cdp_ctrl_ops ol_ops_ctrl = {
	.txrx_get_pldev = ol_get_pldev,
	.txrx_wdi_event_sub = wdi_event_sub,
	.txrx_wdi_event_unsub = wdi_event_unsub,
};

/* WINplatform specific structures */
static struct cdp_me_ops ol_ops_me = {
	/* EMPTY FOR MCL */
};

static struct cdp_mon_ops ol_ops_mon = {
	/* EMPTY FOR MCL */
};

static struct cdp_host_stats_ops ol_ops_host_stats = {
	/* EMPTY FOR MCL */
};

static struct cdp_wds_ops ol_ops_wds = {
	/* EMPTY FOR MCL */
};

static struct cdp_raw_ops ol_ops_raw = {
	/* EMPTY FOR MCL */
};

static struct cdp_ops ol_txrx_ops = {
	.cmn_drv_ops = &ol_ops_cmn,
	.ctrl_ops = &ol_ops_ctrl,
	.me_ops = &ol_ops_me,
	.mon_ops = &ol_ops_mon,
	.host_stats_ops = &ol_ops_host_stats,
	.wds_ops = &ol_ops_wds,
	.raw_ops = &ol_ops_raw,
	.misc_ops = &ol_ops_misc,
	.cfg_ops = &ol_ops_cfg,
	.flowctl_ops = &ol_ops_flowctl,
	.l_flowctl_ops = &ol_ops_l_flowctl,
#ifdef IPA_OFFLOAD
	.ipa_ops = &ol_ops_ipa,
#endif
	.bus_ops = &ol_ops_bus,
	.ocb_ops = &ol_ops_ocb,
	.peer_ops = &ol_ops_peer,
	.throttle_ops = &ol_ops_throttle,
	.mob_stats_ops = &ol_ops_mob_stats,
	.delay_ops = &ol_ops_delay,
	.pmf_ops = &ol_ops_pmf
};

/*
 * Local prototype added to temporarily address warning caused by
 * -Wmissing-prototypes. A more correct solution, namely to expose
 * a prototype in an appropriate header file, will come later.
 */
struct cdp_soc_t *ol_txrx_soc_attach(void *scn_handle,
				     struct ol_if_ops *dp_ol_if_ops);
struct cdp_soc_t *ol_txrx_soc_attach(void *scn_handle,
				     struct ol_if_ops *dp_ol_if_ops)
{
	struct cdp_soc_t *soc = qdf_mem_malloc(sizeof(struct cdp_soc_t));

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: OL SOC memory allocation failed\n", __func__);
		return NULL;
	}

	soc->ops = &ol_txrx_ops;
	return soc;
}


