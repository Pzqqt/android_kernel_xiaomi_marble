/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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

#include <qdf_atomic.h>         /* qdf_atomic_inc, etc. */
#include <qdf_lock.h>           /* qdf_os_spinlock */
#include <qdf_time.h>           /* qdf_system_ticks, etc. */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <qdf_net_types.h>      /* QDF_NBUF_TX_EXT_TID_INVALID */

#include <cds_queue.h>          /* TAILQ */
#ifdef QCA_COMPUTE_TX_DELAY
#include <ieee80211.h>          /* ieee80211_frame, etc. */
#include <enet.h>               /* ethernet_hdr_t, etc. */
#include <ipv6_defs.h>          /* ipv6_traffic_class */
#endif

#include <ol_txrx_api.h>        /* ol_txrx_vdev_handle, etc. */
#include <ol_htt_tx_api.h>      /* htt_tx_compl_desc_id */
#include <ol_txrx_htt_api.h>    /* htt_tx_status */

#include <ol_ctrl_txrx_api.h>
#include <cdp_txrx_tx_delay.h>
#include <ol_txrx_types.h>      /* ol_txrx_vdev_t, etc */
#include <ol_tx_desc.h>         /* ol_tx_desc_find, ol_tx_desc_frame_free */
#ifdef QCA_COMPUTE_TX_DELAY
#include <ol_tx_classify.h>     /* ol_tx_dest_addr_find */
#endif
#include <ol_txrx_internal.h>   /* OL_TX_DESC_NO_REFS, etc. */
#include <ol_osif_txrx_api.h>
#include <ol_tx.h>              /* ol_tx_reinject */

#include <ol_cfg.h>             /* ol_cfg_is_high_latency */
#include <ol_tx_sched.h>
#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
#include <ol_txrx_encap.h>      /* OL_TX_RESTORE_HDR, etc */
#endif
#include <ol_tx_queue.h>
#include <ol_txrx.h>


#ifdef TX_CREDIT_RECLAIM_SUPPORT

#define OL_TX_CREDIT_RECLAIM(pdev)					\
	do {								\
		if (qdf_atomic_read(&pdev->target_tx_credit)  <		\
		    ol_cfg_tx_credit_lwm(pdev->ctrl_pdev)) {		\
			ol_osif_ath_tasklet(pdev->osdev);		\
		}							\
	} while (0)

#else

#define OL_TX_CREDIT_RECLAIM(pdev)

#endif /* TX_CREDIT_RECLAIM_SUPPORT */

#if defined(CONFIG_HL_SUPPORT) || defined(TX_CREDIT_RECLAIM_SUPPORT)

/*
 * HL needs to keep track of the amount of credit available to download
 * tx frames to the target - the download scheduler decides when to
 * download frames, and which frames to download, based on the credit
 * availability.
 * LL systems that use TX_CREDIT_RECLAIM_SUPPORT also need to keep track
 * of the target_tx_credit, to determine when to poll for tx completion
 * messages.
 */
static inline void
ol_tx_target_credit_decr_int(struct ol_txrx_pdev_t *pdev, int delta)
{
	qdf_atomic_add(-1 * delta, &pdev->target_tx_credit);
}

static inline void
ol_tx_target_credit_incr_int(struct ol_txrx_pdev_t *pdev, int delta)
{
	qdf_atomic_add(delta, &pdev->target_tx_credit);
}
#else

static inline void
ol_tx_target_credit_decr_int(struct ol_txrx_pdev_t *pdev, int delta)
{
	return;
}

static inline void
ol_tx_target_credit_incr_int(struct ol_txrx_pdev_t *pdev, int delta)
{
	return;
}
#endif

#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL)

/**
 * ol_tx_flow_ct_unpause_os_q() - Unpause OS Q
 * @pdev: physical device object
 *
 *
 * Return: None
 */
static void ol_tx_flow_ct_unpause_os_q(ol_txrx_pdev_handle pdev)
{
	struct ol_txrx_vdev_t *vdev;
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (qdf_atomic_read(&vdev->os_q_paused) &&
		    (vdev->tx_fl_hwm != 0)) {
			qdf_spin_lock(&pdev->tx_mutex);
			if (pdev->tx_desc.num_free > vdev->tx_fl_hwm) {
				qdf_atomic_set(&vdev->os_q_paused, 0);
				qdf_spin_unlock(&pdev->tx_mutex);
				ol_txrx_flow_control_cb(vdev, true);
			} else {
				qdf_spin_unlock(&pdev->tx_mutex);
			}
		}
	}
}
#elif defined(CONFIG_HL_SUPPORT) && defined(CONFIG_PER_VDEV_TX_DESC_POOL)

static void ol_tx_flow_ct_unpause_os_q(ol_txrx_pdev_handle pdev)
{
	struct ol_txrx_vdev_t *vdev;
	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (qdf_atomic_read(&vdev->os_q_paused) &&
		    (vdev->tx_fl_hwm != 0)) {
			qdf_spin_lock(&pdev->tx_mutex);
			if (((ol_tx_desc_pool_size_hl(
					vdev->pdev->ctrl_pdev) >> 1)
					- TXRX_HL_TX_FLOW_CTRL_MGMT_RESERVED)
					- qdf_atomic_read(&vdev->tx_desc_count)
					> vdev->tx_fl_hwm) {
				qdf_atomic_set(&vdev->os_q_paused, 0);
				qdf_spin_unlock(&pdev->tx_mutex);
				vdev->osif_flow_control_cb(vdev, true);
			} else {
				qdf_spin_unlock(&pdev->tx_mutex);
			}
		}
	}
}
#else

static inline void ol_tx_flow_ct_unpause_os_q(ol_txrx_pdev_handle pdev)
{
	return;
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

static inline uint16_t
ol_tx_send_base(struct ol_txrx_pdev_t *pdev,
		struct ol_tx_desc_t *tx_desc, qdf_nbuf_t msdu)
{
	int msdu_credit_consumed;

	TX_CREDIT_DEBUG_PRINT("TX %d bytes\n", qdf_nbuf_len(msdu));
	TX_CREDIT_DEBUG_PRINT(" <HTT> Decrease credit %d - 1 = %d, len:%d.\n",
			      qdf_atomic_read(&pdev->target_tx_credit),
			      qdf_atomic_read(&pdev->target_tx_credit) - 1,
			      qdf_nbuf_len(msdu));

	msdu_credit_consumed = htt_tx_msdu_credit(msdu);
	ol_tx_target_credit_decr_int(pdev, msdu_credit_consumed);
	OL_TX_CREDIT_RECLAIM(pdev);

	/*
	 * When the tx frame is downloaded to the target, there are two
	 * outstanding references:
	 * 1.  The host download SW (HTT, HTC, HIF)
	 *     This reference is cleared by the ol_tx_send_done callback
	 *     functions.
	 * 2.  The target FW
	 *     This reference is cleared by the ol_tx_completion_handler
	 *     function.
	 * It is extremely probable that the download completion is processed
	 * before the tx completion message.  However, under exceptional
	 * conditions the tx completion may be processed first.  Thus, rather
	 * that assuming that reference (1) is done before reference (2),
	 * explicit reference tracking is needed.
	 * Double-increment the ref count to account for both references
	 * described above.
	 */

	OL_TX_DESC_REF_INIT(tx_desc);
	OL_TX_DESC_REF_INC(tx_desc);
	OL_TX_DESC_REF_INC(tx_desc);

	return msdu_credit_consumed;
}

void
ol_tx_send(struct ol_txrx_pdev_t *pdev,
	   struct ol_tx_desc_t *tx_desc, qdf_nbuf_t msdu, uint8_t vdev_id)
{
	int msdu_credit_consumed;
	uint16_t id;
	int failed;

	msdu_credit_consumed = ol_tx_send_base(pdev, tx_desc, msdu);
	id = ol_tx_desc_id(pdev, tx_desc);
	QDF_NBUF_UPDATE_TX_PKT_COUNT(msdu, QDF_NBUF_TX_PKT_TXRX);
	DPTRACE(qdf_dp_trace_ptr(msdu, QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD,
				qdf_nbuf_data_addr(msdu),
				sizeof(qdf_nbuf_data(msdu)), tx_desc->id,
				vdev_id));
	failed = htt_tx_send_std(pdev->htt_pdev, msdu, id);
	if (qdf_unlikely(failed)) {
		ol_tx_target_credit_incr_int(pdev, msdu_credit_consumed);
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1 /* had error */);
	}
}

void
ol_tx_send_batch(struct ol_txrx_pdev_t *pdev,
		 qdf_nbuf_t head_msdu, int num_msdus)
{
	qdf_nbuf_t rejected;
	OL_TX_CREDIT_RECLAIM(pdev);

	rejected = htt_tx_send_batch(pdev->htt_pdev, head_msdu, num_msdus);
	while (qdf_unlikely(rejected)) {
		struct ol_tx_desc_t *tx_desc;
		uint16_t *msdu_id_storage;
		qdf_nbuf_t next;

		next = qdf_nbuf_next(rejected);
		msdu_id_storage = ol_tx_msdu_id_storage(rejected);
		tx_desc = ol_tx_desc_find(pdev, *msdu_id_storage);

		ol_tx_target_credit_incr(pdev, rejected);
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1 /* had error */);

		rejected = next;
	}
}

void
ol_tx_send_nonstd(struct ol_txrx_pdev_t *pdev,
		  struct ol_tx_desc_t *tx_desc,
		  qdf_nbuf_t msdu, enum htt_pkt_type pkt_type)
{
	int msdu_credit_consumed;
	uint16_t id;
	int failed;

	msdu_credit_consumed = ol_tx_send_base(pdev, tx_desc, msdu);
	id = ol_tx_desc_id(pdev, tx_desc);
	QDF_NBUF_UPDATE_TX_PKT_COUNT(msdu, QDF_NBUF_TX_PKT_TXRX);
	failed = htt_tx_send_nonstd(pdev->htt_pdev, msdu, id, pkt_type);
	if (failed) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_ERR,
			   "Error: freeing tx frame after htt_tx failed");
		ol_tx_target_credit_incr_int(pdev, msdu_credit_consumed);
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1 /* had error */);
	}
}

static inline void
ol_tx_download_done_base(struct ol_txrx_pdev_t *pdev,
			 A_STATUS status, qdf_nbuf_t msdu, uint16_t msdu_id)
{
	struct ol_tx_desc_t *tx_desc;

	tx_desc = ol_tx_desc_find(pdev, msdu_id);
	qdf_assert(tx_desc);

	/*
	 * If the download is done for
	 * the Management frame then
	 * call the download callback if registered
	 */
	if (tx_desc->pkt_type >= OL_TXRX_MGMT_TYPE_BASE) {
		int tx_mgmt_index = tx_desc->pkt_type - OL_TXRX_MGMT_TYPE_BASE;
		ol_txrx_mgmt_tx_cb download_cb =
			pdev->tx_mgmt.callbacks[tx_mgmt_index].download_cb;

		if (download_cb) {
			download_cb(pdev->tx_mgmt.callbacks[tx_mgmt_index].ctxt,
				    tx_desc->netbuf, status != A_OK);
		}
	}

	if (status != A_OK) {
		ol_tx_target_credit_incr(pdev, msdu);
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc,
					     1 /* download err */);
	} else {
		if (OL_TX_DESC_NO_REFS(tx_desc)) {
			/*
			 * The decremented value was zero - free the frame.
			 * Use the tx status recorded previously during
			 * tx completion handling.
			 */
			ol_tx_desc_frame_free_nonstd(pdev, tx_desc,
						     tx_desc->status !=
						     htt_tx_status_ok);
		}
	}
}

void
ol_tx_download_done_ll(void *pdev,
		       A_STATUS status, qdf_nbuf_t msdu, uint16_t msdu_id)
{
	ol_tx_download_done_base((struct ol_txrx_pdev_t *)pdev, status, msdu,
				 msdu_id);
}

void
ol_tx_download_done_hl_retain(void *txrx_pdev,
			      A_STATUS status,
			      qdf_nbuf_t msdu, uint16_t msdu_id)
{
	struct ol_txrx_pdev_t *pdev = txrx_pdev;
	ol_tx_download_done_base(pdev, status, msdu, msdu_id);
}

void
ol_tx_download_done_hl_free(void *txrx_pdev,
			    A_STATUS status, qdf_nbuf_t msdu, uint16_t msdu_id)
{
	struct ol_txrx_pdev_t *pdev = txrx_pdev;
	struct ol_tx_desc_t *tx_desc;

	tx_desc = ol_tx_desc_find(pdev, msdu_id);
	qdf_assert(tx_desc);

	ol_tx_download_done_base(pdev, status, msdu, msdu_id);

	if ((tx_desc->pkt_type != OL_TX_FRM_NO_FREE) &&
	    (tx_desc->pkt_type < OL_TXRX_MGMT_TYPE_BASE)) {
		qdf_atomic_add(1, &pdev->tx_queue.rsrc_cnt);
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc, status != A_OK);
	}
}

void ol_tx_target_credit_init(struct ol_txrx_pdev_t *pdev, int credit_delta)
{
	qdf_atomic_add(credit_delta, &pdev->orig_target_tx_credit);
}

void ol_tx_target_credit_update(struct ol_txrx_pdev_t *pdev, int credit_delta)
{
	TX_CREDIT_DEBUG_PRINT(" <HTT> Increase credit %d + %d = %d\n",
			      qdf_atomic_read(&pdev->target_tx_credit),
			      credit_delta,
			      qdf_atomic_read(&pdev->target_tx_credit) +
			      credit_delta);
	qdf_atomic_add(credit_delta, &pdev->target_tx_credit);
}

#ifdef QCA_COMPUTE_TX_DELAY

static void
ol_tx_delay_compute(struct ol_txrx_pdev_t *pdev,
		    enum htt_tx_status status,
		    uint16_t *desc_ids, int num_msdus);

#else
static inline void
ol_tx_delay_compute(struct ol_txrx_pdev_t *pdev,
		    enum htt_tx_status status,
		    uint16_t *desc_ids, int num_msdus)
{
	return;
}
#endif /* QCA_COMPUTE_TX_DELAY */

#ifndef OL_TX_RESTORE_HDR
#define OL_TX_RESTORE_HDR(__tx_desc, __msdu)
#endif
/*
 * The following macros could have been inline functions too.
 * The only rationale for choosing macros, is to force the compiler to inline
 * the implementation, which cannot be controlled for actual "inline" functions,
 * since "inline" is only a hint to the compiler.
 * In the performance path, we choose to force the inlining, in preference to
 * type-checking offered by the actual inlined functions.
 */
#define ol_tx_msdu_complete_batch(_pdev, _tx_desc, _tx_descs, _status) \
	TAILQ_INSERT_TAIL(&(_tx_descs), (_tx_desc), tx_desc_list_elem)
#ifndef ATH_11AC_TXCOMPACT
#define ol_tx_msdu_complete_single(_pdev, _tx_desc, _netbuf,\
				   _lcl_freelist, _tx_desc_last)	\
	do {								\
		qdf_atomic_init(&(_tx_desc)->ref_cnt);			\
		/* restore orginal hdr offset */			\
		OL_TX_RESTORE_HDR((_tx_desc), (_netbuf));		\
		qdf_nbuf_unmap((_pdev)->osdev, (_netbuf), QDF_DMA_TO_DEVICE); \
		qdf_nbuf_free((_netbuf));				\
		((union ol_tx_desc_list_elem_t *)(_tx_desc))->next =	\
			(_lcl_freelist);				\
		if (qdf_unlikely(!lcl_freelist)) {			\
			(_tx_desc_last) = (union ol_tx_desc_list_elem_t *)\
				(_tx_desc);				\
		}							\
		(_lcl_freelist) = (union ol_tx_desc_list_elem_t *)(_tx_desc); \
	} while (0)
#else    /*!ATH_11AC_TXCOMPACT */
#define ol_tx_msdu_complete_single(_pdev, _tx_desc, _netbuf,\
				   _lcl_freelist, _tx_desc_last)	\
	do {								\
		/* restore orginal hdr offset */			\
		OL_TX_RESTORE_HDR((_tx_desc), (_netbuf));		\
		qdf_nbuf_unmap((_pdev)->osdev, (_netbuf), QDF_DMA_TO_DEVICE); \
		qdf_nbuf_free((_netbuf));				\
		((union ol_tx_desc_list_elem_t *)(_tx_desc))->next =	\
			(_lcl_freelist);				\
		if (qdf_unlikely(!lcl_freelist)) {			\
			(_tx_desc_last) = (union ol_tx_desc_list_elem_t *)\
				(_tx_desc);				\
		}							\
		(_lcl_freelist) = (union ol_tx_desc_list_elem_t *)(_tx_desc); \
	} while (0)

#endif /*!ATH_11AC_TXCOMPACT */

#ifdef QCA_TX_SINGLE_COMPLETIONS
#ifdef QCA_TX_STD_PATH_ONLY
#define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs,			\
			    _netbuf, _lcl_freelist,			\
			    _tx_desc_last, _status)			\
	ol_tx_msdu_complete_single((_pdev), (_tx_desc),			\
				   (_netbuf), (_lcl_freelist),		\
				   _tx_desc_last)
#else                           /* !QCA_TX_STD_PATH_ONLY */
#define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs,			\
			    _netbuf, _lcl_freelist,			\
			    _tx_desc_last, _status)			\
	do {								\
		if (qdf_likely((_tx_desc)->pkt_type == OL_TX_FRM_STD)) { \
			ol_tx_msdu_complete_single((_pdev), (_tx_desc),\
						   (_netbuf), (_lcl_freelist), \
						   (_tx_desc_last));	\
		} else {						\
			ol_tx_desc_frame_free_nonstd(			\
				(_pdev), (_tx_desc),			\
				(_status) != htt_tx_status_ok);		\
		}							\
	} while (0)
#endif /* !QCA_TX_STD_PATH_ONLY */
#else                           /* !QCA_TX_SINGLE_COMPLETIONS */
#ifdef QCA_TX_STD_PATH_ONLY
#define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs,			\
			    _netbuf, _lcl_freelist,			\
			    _tx_desc_last, _status)			\
	ol_tx_msdus_complete_batch((_pdev), (_tx_desc), (_tx_descs), (_status))
#else                           /* !QCA_TX_STD_PATH_ONLY */
#define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs,			\
			    _netbuf, _lcl_freelist,			\
			    _tx_desc_last, _status)			\
	do {								\
		if (qdf_likely((_tx_desc)->pkt_type == OL_TX_FRM_STD)) { \
			ol_tx_msdu_complete_batch((_pdev), (_tx_desc),	\
						  (_tx_descs), (_status)); \
		} else {						\
			ol_tx_desc_frame_free_nonstd((_pdev), (_tx_desc), \
						     (_status) !=	\
						     htt_tx_status_ok); \
		}							\
	} while (0)
#endif /* !QCA_TX_STD_PATH_ONLY */
#endif /* QCA_TX_SINGLE_COMPLETIONS */

void ol_tx_discard_target_frms(ol_txrx_pdev_handle pdev)
{
	int i = 0;
	struct ol_tx_desc_t *tx_desc;

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
			TXRX_PRINT(TXRX_PRINT_LEVEL_WARN,
				   "Warning: freeing tx frame "
				   "(no tx completion from the target)\n");
			ol_tx_desc_frame_free_nonstd(pdev,
						     tx_desc, 1);
		}
	}
}

void ol_tx_credit_completion_handler(ol_txrx_pdev_handle pdev, int credits)
{
	ol_tx_target_credit_update(pdev, credits);

	if (pdev->cfg.is_high_latency)
		ol_tx_sched(pdev);

	/* UNPAUSE OS Q */
	ol_tx_flow_ct_unpause_os_q(pdev);
}

/* WARNING: ol_tx_inspect_handler()'s bahavior is similar to that of
   ol_tx_completion_handler().
   * any change in ol_tx_completion_handler() must be mirrored in
   ol_tx_inspect_handler().
*/
void
ol_tx_completion_handler(ol_txrx_pdev_handle pdev,
			 int num_msdus,
			 enum htt_tx_status status, void *tx_desc_id_iterator)
{
	int i;
	uint16_t *desc_ids = (uint16_t *) tx_desc_id_iterator;
	uint16_t tx_desc_id;
	struct ol_tx_desc_t *tx_desc;
	uint32_t byte_cnt = 0;
	qdf_nbuf_t netbuf;

	union ol_tx_desc_list_elem_t *lcl_freelist = NULL;
	union ol_tx_desc_list_elem_t *tx_desc_last = NULL;
	ol_tx_desc_list tx_descs;
	TAILQ_INIT(&tx_descs);

	ol_tx_delay_compute(pdev, status, desc_ids, num_msdus);

	for (i = 0; i < num_msdus; i++) {
		tx_desc_id = desc_ids[i];
		tx_desc = ol_tx_desc_find(pdev, tx_desc_id);
		tx_desc->status = status;
		netbuf = tx_desc->netbuf;
		QDF_NBUF_UPDATE_TX_PKT_COUNT(netbuf, QDF_NBUF_TX_PKT_FREE);
		DPTRACE(qdf_dp_trace_ptr(netbuf,
			QDF_DP_TRACE_FREE_PACKET_PTR_RECORD,
			qdf_nbuf_data_addr(netbuf),
			sizeof(qdf_nbuf_data(netbuf)), tx_desc->id, status));
		htc_pm_runtime_put(pdev->htt_pdev->htc_pdev);
		ol_tx_desc_update_group_credit(pdev, tx_desc_id, 1, 0, status);
		/* Per SDU update of byte count */
		byte_cnt += qdf_nbuf_len(netbuf);
		if (OL_TX_DESC_NO_REFS(tx_desc)) {
			ol_tx_statistics(
				pdev->ctrl_pdev,
				HTT_TX_DESC_VDEV_ID_GET(*((uint32_t *)
							  (tx_desc->
							   htt_tx_desc))),
				status != htt_tx_status_ok);
			ol_tx_msdu_complete(pdev, tx_desc, tx_descs, netbuf,
					    lcl_freelist, tx_desc_last, status);
		}
#ifdef QCA_SUPPORT_TXDESC_SANITY_CHECKS
		tx_desc->pkt_type = 0xff;
#ifdef QCA_COMPUTE_TX_DELAY
		tx_desc->entry_timestamp_ticks = 0xffffffff;
#endif
#endif
	}

	/* One shot protected access to pdev freelist, when setup */
	if (lcl_freelist) {
		qdf_spin_lock(&pdev->tx_mutex);
		tx_desc_last->next = pdev->tx_desc.freelist;
		pdev->tx_desc.freelist = lcl_freelist;
		pdev->tx_desc.num_free += (uint16_t) num_msdus;
		qdf_spin_unlock(&pdev->tx_mutex);
	} else {
		ol_tx_desc_frame_list_free(pdev, &tx_descs,
					   status != htt_tx_status_ok);
	}

	if (pdev->cfg.is_high_latency) {
		/*
		 * Credit was already explicitly updated by HTT,
		 * but update the number of available tx descriptors,
		 * then invoke the scheduler, since new credit is probably
		 * available now.
		 */
		qdf_atomic_add(num_msdus, &pdev->tx_queue.rsrc_cnt);
		ol_tx_sched(pdev);
	} else {
		ol_tx_target_credit_adjust(num_msdus, pdev, NULL);
	}

	/* UNPAUSE OS Q */
	ol_tx_flow_ct_unpause_os_q(pdev);
	/* Do one shot statistics */
	TXRX_STATS_UPDATE_TX_STATS(pdev, status, num_msdus, byte_cnt);
}

#ifdef FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL

void ol_tx_desc_update_group_credit(ol_txrx_pdev_handle pdev,
		u_int16_t tx_desc_id, int credit, u_int8_t absolute,
		enum htt_tx_status status)
{
	uint8_t i, is_member;
	uint16_t vdev_id_mask;
	struct ol_tx_desc_t *tx_desc;

	tx_desc = ol_tx_desc_find(pdev, tx_desc_id);
	for (i = 0; i < OL_TX_MAX_TXQ_GROUPS; i++) {
		vdev_id_mask =
			OL_TXQ_GROUP_VDEV_ID_MASK_GET(
					pdev->txq_grps[i].membership);
		is_member = OL_TXQ_GROUP_VDEV_ID_BIT_MASK_GET(vdev_id_mask,
				tx_desc->vdev->vdev_id);
		if (is_member) {
			ol_txrx_update_group_credit(&pdev->txq_grps[i],
						    credit, absolute);
			break;
		}
	}
	ol_tx_update_group_credit_stats(pdev);
}

#ifdef DEBUG_HL_LOGGING

void ol_tx_update_group_credit_stats(ol_txrx_pdev_handle pdev)
{
	uint16_t curr_index;
	uint8_t i;

	qdf_spin_lock_bh(&pdev->grp_stat_spinlock);
	pdev->grp_stats.last_valid_index++;
	if (pdev->grp_stats.last_valid_index > (OL_TX_GROUP_STATS_LOG_SIZE
				- 1)) {
		pdev->grp_stats.last_valid_index -= OL_TX_GROUP_STATS_LOG_SIZE;
		pdev->grp_stats.wrap_around = 1;
	}
	curr_index = pdev->grp_stats.last_valid_index;

	for (i = 0; i < OL_TX_MAX_TXQ_GROUPS; i++) {
		pdev->grp_stats.stats[curr_index].grp[i].member_vdevs =
			OL_TXQ_GROUP_VDEV_ID_MASK_GET(
					pdev->txq_grps[i].membership);
		pdev->grp_stats.stats[curr_index].grp[i].credit =
			qdf_atomic_read(&pdev->txq_grps[i].credit);
	}

	qdf_spin_unlock_bh(&pdev->grp_stat_spinlock);
}

void ol_tx_dump_group_credit_stats(ol_txrx_pdev_handle pdev)
{
	uint16_t i, j, is_break = 0;
	int16_t curr_index, old_index, wrap_around;
	uint16_t curr_credit, old_credit, mem_vdevs;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		  "Group credit stats:");
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		  "  No: GrpID: Credit: Change: vdev_map");

	qdf_spin_lock_bh(&pdev->grp_stat_spinlock);
	curr_index = pdev->grp_stats.last_valid_index;
	wrap_around = pdev->grp_stats.wrap_around;
	qdf_spin_unlock_bh(&pdev->grp_stat_spinlock);

	if (curr_index < 0) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Not initialized");
		return;
	}

	for (i = 0; i < OL_TX_GROUP_STATS_LOG_SIZE; i++) {
		old_index = curr_index - 1;
		if (old_index < 0) {
			if (wrap_around == 0)
				is_break = 1;
			else
				old_index = OL_TX_GROUP_STATS_LOG_SIZE - 1;
		}

		for (j = 0; j < OL_TX_MAX_TXQ_GROUPS; j++) {
			qdf_spin_lock_bh(&pdev->grp_stat_spinlock);
			curr_credit =
				pdev->grp_stats.stats[curr_index].
								grp[j].credit;
			if (!is_break)
				old_credit =
					pdev->grp_stats.stats[old_index].
								grp[j].credit;

			mem_vdevs =
				pdev->grp_stats.stats[curr_index].grp[j].
								member_vdevs;
			qdf_spin_unlock_bh(&pdev->grp_stat_spinlock);

			if (!is_break)
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_ERROR,
					  "%4d: %5d: %6d %6d %8x",
					  curr_index, j,
					  curr_credit,
					  (curr_credit - old_credit),
					  mem_vdevs);
			else
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_ERROR,
					  "%4d: %5d: %6d %6s %8x",
					  curr_index, j,
					  curr_credit, "NA", mem_vdevs);
		}

		if (is_break)
			break;

		curr_index = old_index;
	}
}

void ol_tx_clear_group_credit_stats(ol_txrx_pdev_handle pdev)
{
	qdf_spin_lock_bh(&pdev->grp_stat_spinlock);
	qdf_mem_zero(&pdev->grp_stats, sizeof(pdev->grp_stats));
	pdev->grp_stats.last_valid_index = -1;
	pdev->grp_stats.wrap_around = 0;
	qdf_spin_unlock_bh(&pdev->grp_stat_spinlock);
}
#endif
#endif

/*
 * ol_tx_single_completion_handler performs the same tx completion
 * processing as ol_tx_completion_handler, but for a single frame.
 * ol_tx_completion_handler is optimized to handle batch completions
 * as efficiently as possible; in contrast ol_tx_single_completion_handler
 * handles single frames as simply and generally as possible.
 * Thus, this ol_tx_single_completion_handler function is suitable for
 * intermittent usage, such as for tx mgmt frames.
 */
void
ol_tx_single_completion_handler(ol_txrx_pdev_handle pdev,
				enum htt_tx_status status, uint16_t tx_desc_id)
{
	struct ol_tx_desc_t *tx_desc;
	qdf_nbuf_t netbuf;

	tx_desc = ol_tx_desc_find(pdev, tx_desc_id);
	tx_desc->status = status;
	netbuf = tx_desc->netbuf;

	QDF_NBUF_UPDATE_TX_PKT_COUNT(netbuf, QDF_NBUF_TX_PKT_FREE);
	/* Do one shot statistics */
	TXRX_STATS_UPDATE_TX_STATS(pdev, status, 1, qdf_nbuf_len(netbuf));

	if (OL_TX_DESC_NO_REFS(tx_desc)) {
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc,
					     status != htt_tx_status_ok);
	}

	TX_CREDIT_DEBUG_PRINT(" <HTT> Increase credit %d + %d = %d\n",
			      qdf_atomic_read(&pdev->target_tx_credit),
			      1, qdf_atomic_read(&pdev->target_tx_credit) + 1);

	if (pdev->cfg.is_high_latency) {
		/*
		 * Credit was already explicitly updated by HTT,
		 * but update the number of available tx descriptors,
		 * then invoke the scheduler, since new credit is probably
		 * available now.
		 */
		qdf_atomic_add(1, &pdev->tx_queue.rsrc_cnt);
		ol_tx_sched(pdev);
	} else {
		qdf_atomic_add(1, &pdev->target_tx_credit);
	}
}

/* WARNING: ol_tx_inspect_handler()'s bahavior is similar to that of
   ol_tx_completion_handler().
 * any change in ol_tx_completion_handler() must be mirrored here.
 */
void
ol_tx_inspect_handler(ol_txrx_pdev_handle pdev,
		      int num_msdus, void *tx_desc_id_iterator)
{
	uint16_t vdev_id, i;
	struct ol_txrx_vdev_t *vdev;
	uint16_t *desc_ids = (uint16_t *) tx_desc_id_iterator;
	uint16_t tx_desc_id;
	struct ol_tx_desc_t *tx_desc;
	union ol_tx_desc_list_elem_t *lcl_freelist = NULL;
	union ol_tx_desc_list_elem_t *tx_desc_last = NULL;
	qdf_nbuf_t netbuf;
	ol_tx_desc_list tx_descs;
	TAILQ_INIT(&tx_descs);

	for (i = 0; i < num_msdus; i++) {
		tx_desc_id = desc_ids[i];
		tx_desc = ol_tx_desc_find(pdev, tx_desc_id);
		netbuf = tx_desc->netbuf;

		/* find the "vdev" this tx_desc belongs to */
		vdev_id = HTT_TX_DESC_VDEV_ID_GET(*((uint32_t *)
						    (tx_desc->htt_tx_desc)));
		TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
			if (vdev->vdev_id == vdev_id)
				break;
		}

		/* vdev now points to the vdev for this descriptor. */

#ifndef ATH_11AC_TXCOMPACT
		/* save this multicast packet to local free list */
		if (qdf_atomic_dec_and_test(&tx_desc->ref_cnt))
#endif
		{
			/* for this function only, force htt status to be
			   "htt_tx_status_ok"
			 * for graceful freeing of this multicast frame
			 */
			ol_tx_msdu_complete(pdev, tx_desc, tx_descs, netbuf,
					    lcl_freelist, tx_desc_last,
					    htt_tx_status_ok);
		}
	}

	if (lcl_freelist) {
		qdf_spin_lock(&pdev->tx_mutex);
		tx_desc_last->next = pdev->tx_desc.freelist;
		pdev->tx_desc.freelist = lcl_freelist;
		qdf_spin_unlock(&pdev->tx_mutex);
	} else {
		ol_tx_desc_frame_list_free(pdev, &tx_descs,
					   htt_tx_status_discard);
	}
	TX_CREDIT_DEBUG_PRINT(" <HTT> Increase HTT credit %d + %d = %d..\n",
			      qdf_atomic_read(&pdev->target_tx_credit),
			      num_msdus,
			      qdf_atomic_read(&pdev->target_tx_credit) +
			      num_msdus);

	if (pdev->cfg.is_high_latency) {
		/* credit was already explicitly updated by HTT */
		ol_tx_sched(pdev);
	} else {
		ol_tx_target_credit_adjust(num_msdus, pdev, NULL);
	}
}

#ifdef QCA_COMPUTE_TX_DELAY
/**
 * @brief updates the compute interval period for TSM stats.
 * @details
 * @param interval - interval for stats computation
 */
void ol_tx_set_compute_interval(ol_txrx_pdev_handle pdev, uint32_t interval)
{
	pdev->tx_delay.avg_period_ticks = qdf_system_msecs_to_ticks(interval);
}

/**
 * @brief Return the uplink (transmitted) packet count and loss count.
 * @details
 *  This function will be called for getting uplink packet count and
 *  loss count for given stream (access category) a regular interval.
 *  This also resets the counters hence, the value returned is packets
 *  counted in last 5(default) second interval. These counter are
 *  incremented per access category in ol_tx_completion_handler()
 *
 * @param category - access category of interest
 * @param out_packet_count - number of packets transmitted
 * @param out_packet_loss_count - number of packets lost
 */
void
ol_tx_packet_count(ol_txrx_pdev_handle pdev,
		   uint16_t *out_packet_count,
		   uint16_t *out_packet_loss_count, int category)
{
	*out_packet_count = pdev->packet_count[category];
	*out_packet_loss_count = pdev->packet_loss_count[category];
	pdev->packet_count[category] = 0;
	pdev->packet_loss_count[category] = 0;
}

uint32_t ol_tx_delay_avg(uint64_t sum, uint32_t num)
{
	uint32_t sum32;
	int shift = 0;
	/*
	 * To avoid doing a 64-bit divide, shift the sum down until it is
	 * no more than 32 bits (and shift the denominator to match).
	 */
	while ((sum >> 32) != 0) {
		sum >>= 1;
		shift++;
	}
	sum32 = (uint32_t) sum;
	num >>= shift;
	return (sum32 + (num >> 1)) / num;      /* round to nearest */
}

void
ol_tx_delay(ol_txrx_pdev_handle pdev,
	    uint32_t *queue_delay_microsec,
	    uint32_t *tx_delay_microsec, int category)
{
	int index;
	uint32_t avg_delay_ticks;
	struct ol_tx_delay_data *data;

	qdf_assert(category >= 0 && category < QCA_TX_DELAY_NUM_CATEGORIES);

	qdf_spin_lock_bh(&pdev->tx_delay.mutex);
	index = 1 - pdev->tx_delay.cats[category].in_progress_idx;

	data = &pdev->tx_delay.cats[category].copies[index];

	if (data->avgs.transmit_num > 0) {
		avg_delay_ticks =
			ol_tx_delay_avg(data->avgs.transmit_sum_ticks,
					data->avgs.transmit_num);
		*tx_delay_microsec =
			qdf_system_ticks_to_msecs(avg_delay_ticks * 1000);
	} else {
		/*
		 * This case should only happen if there's a query
		 * within 5 sec after the first tx data frame.
		 */
		*tx_delay_microsec = 0;
	}
	if (data->avgs.queue_num > 0) {
		avg_delay_ticks =
			ol_tx_delay_avg(data->avgs.queue_sum_ticks,
					data->avgs.queue_num);
		*queue_delay_microsec =
			qdf_system_ticks_to_msecs(avg_delay_ticks * 1000);
	} else {
		/*
		 * This case should only happen if there's a query
		 * within 5 sec after the first tx data frame.
		 */
		*queue_delay_microsec = 0;
	}

	qdf_spin_unlock_bh(&pdev->tx_delay.mutex);
}

void
ol_tx_delay_hist(ol_txrx_pdev_handle pdev,
		 uint16_t *report_bin_values, int category)
{
	int index, i, j;
	struct ol_tx_delay_data *data;

	qdf_assert(category >= 0 && category < QCA_TX_DELAY_NUM_CATEGORIES);

	qdf_spin_lock_bh(&pdev->tx_delay.mutex);
	index = 1 - pdev->tx_delay.cats[category].in_progress_idx;

	data = &pdev->tx_delay.cats[category].copies[index];

	for (i = 0, j = 0; i < QCA_TX_DELAY_HIST_REPORT_BINS - 1; i++) {
		uint16_t internal_bin_sum = 0;
		while (j < (1 << i))
			internal_bin_sum += data->hist_bins_queue[j++];

		report_bin_values[i] = internal_bin_sum;
	}
	report_bin_values[i] = data->hist_bins_queue[j];        /* overflow */

	qdf_spin_unlock_bh(&pdev->tx_delay.mutex);
}

#ifdef QCA_COMPUTE_TX_DELAY_PER_TID

static uint8_t
ol_tx_delay_tid_from_l3_hdr(struct ol_txrx_pdev_t *pdev,
			    qdf_nbuf_t msdu, struct ol_tx_desc_t *tx_desc)
{
	uint16_t ethertype;
	uint8_t *dest_addr, *l3_hdr;
	int is_mgmt, is_mcast;
	int l2_hdr_size;

	dest_addr = ol_tx_dest_addr_find(pdev, msdu);
	if (NULL == dest_addr)
		return QDF_NBUF_TX_EXT_TID_INVALID;

	is_mcast = IEEE80211_IS_MULTICAST(dest_addr);
	is_mgmt = tx_desc->pkt_type >= OL_TXRX_MGMT_TYPE_BASE;
	if (is_mgmt) {
		return (is_mcast) ?
		       OL_TX_NUM_TIDS + OL_TX_VDEV_DEFAULT_MGMT :
		       HTT_TX_EXT_TID_MGMT;
	}
	if (is_mcast)
		return OL_TX_NUM_TIDS + OL_TX_VDEV_MCAST_BCAST;

	if (pdev->frame_format == wlan_frm_fmt_802_3) {
		struct ethernet_hdr_t *enet_hdr;
		enet_hdr = (struct ethernet_hdr_t *)qdf_nbuf_data(msdu);
		l2_hdr_size = sizeof(struct ethernet_hdr_t);
		ethertype =
			(enet_hdr->ethertype[0] << 8) | enet_hdr->ethertype[1];
		if (!IS_ETHERTYPE(ethertype)) {
			struct llc_snap_hdr_t *llc_hdr;
			llc_hdr = (struct llc_snap_hdr_t *)
				  (qdf_nbuf_data(msdu) + l2_hdr_size);
			l2_hdr_size += sizeof(struct llc_snap_hdr_t);
			ethertype =
				(llc_hdr->ethertype[0] << 8) | llc_hdr->
				ethertype[1];
		}
	} else {
		struct llc_snap_hdr_t *llc_hdr;
		l2_hdr_size = sizeof(struct ieee80211_frame);
		llc_hdr = (struct llc_snap_hdr_t *)(qdf_nbuf_data(msdu)
						    + l2_hdr_size);
		l2_hdr_size += sizeof(struct llc_snap_hdr_t);
		ethertype =
			(llc_hdr->ethertype[0] << 8) | llc_hdr->ethertype[1];
	}
	l3_hdr = qdf_nbuf_data(msdu) + l2_hdr_size;
	if (ETHERTYPE_IPV4 == ethertype) {
		return (((struct ipv4_hdr_t *)l3_hdr)->tos >> 5) & 0x7;
	} else if (ETHERTYPE_IPV6 == ethertype) {
		return (ipv6_traffic_class((struct ipv6_hdr_t *)l3_hdr) >> 5) &
		       0x7;
	} else {
		return QDF_NBUF_TX_EXT_TID_INVALID;
	}
}
#endif

static int ol_tx_delay_category(struct ol_txrx_pdev_t *pdev, uint16_t msdu_id)
{
#ifdef QCA_COMPUTE_TX_DELAY_PER_TID
	struct ol_tx_desc_t *tx_desc = ol_tx_desc_find(pdev, msdu_id);
	uint8_t tid;

	qdf_nbuf_t msdu = tx_desc->netbuf;
	tid = qdf_nbuf_get_tid(msdu);
	if (tid == QDF_NBUF_TX_EXT_TID_INVALID) {
		tid = ol_tx_delay_tid_from_l3_hdr(pdev, msdu, tx_desc);
		if (tid == QDF_NBUF_TX_EXT_TID_INVALID) {
			/* TID could not be determined
			   (this is not an IP frame?) */
			return -EINVAL;
		}
	}
	return tid;
#else
	return 0;
#endif
}

static inline int
ol_tx_delay_hist_bin(struct ol_txrx_pdev_t *pdev, uint32_t delay_ticks)
{
	int bin;
	/*
	 * For speed, multiply and shift to approximate a divide. This causes
	 * a small error, but the approximation error should be much less
	 * than the other uncertainties in the tx delay computation.
	 */
	bin = (delay_ticks * pdev->tx_delay.hist_internal_bin_width_mult) >>
	      pdev->tx_delay.hist_internal_bin_width_shift;
	if (bin >= QCA_TX_DELAY_HIST_INTERNAL_BINS)
		bin = QCA_TX_DELAY_HIST_INTERNAL_BINS - 1;

	return bin;
}

static void
ol_tx_delay_compute(struct ol_txrx_pdev_t *pdev,
		    enum htt_tx_status status,
		    uint16_t *desc_ids, int num_msdus)
{
	int i, index, cat;
	uint32_t now_ticks = qdf_system_ticks();
	uint32_t tx_delay_transmit_ticks, tx_delay_queue_ticks;
	uint32_t avg_time_ticks;
	struct ol_tx_delay_data *data;

	qdf_assert(num_msdus > 0);

	/*
	 * keep static counters for total packet and lost packets
	 * reset them in ol_tx_delay(), function used to fetch the stats
	 */

	cat = ol_tx_delay_category(pdev, desc_ids[0]);
	if (cat < 0 || cat >= QCA_TX_DELAY_NUM_CATEGORIES)
		return;

	pdev->packet_count[cat] = pdev->packet_count[cat] + num_msdus;
	if (status != htt_tx_status_ok) {
		for (i = 0; i < num_msdus; i++) {
			cat = ol_tx_delay_category(pdev, desc_ids[i]);
			if (cat < 0 || cat >= QCA_TX_DELAY_NUM_CATEGORIES)
				return;
			pdev->packet_loss_count[cat]++;
		}
		return;
	}

	/* since we may switch the ping-pong index, provide mutex w. readers */
	qdf_spin_lock_bh(&pdev->tx_delay.mutex);
	index = pdev->tx_delay.cats[cat].in_progress_idx;

	data = &pdev->tx_delay.cats[cat].copies[index];

	if (pdev->tx_delay.tx_compl_timestamp_ticks != 0) {
		tx_delay_transmit_ticks =
			now_ticks - pdev->tx_delay.tx_compl_timestamp_ticks;
		/*
		 * We'd like to account for the number of MSDUs that were
		 * transmitted together, but we don't know this.  All we know
		 * is the number of MSDUs that were acked together.
		 * Since the frame error rate is small, this is nearly the same
		 * as the number of frames transmitted together.
		 */
		data->avgs.transmit_sum_ticks += tx_delay_transmit_ticks;
		data->avgs.transmit_num += num_msdus;
	}
	pdev->tx_delay.tx_compl_timestamp_ticks = now_ticks;

	for (i = 0; i < num_msdus; i++) {
		int bin;
		uint16_t id = desc_ids[i];
		struct ol_tx_desc_t *tx_desc = ol_tx_desc_find(pdev, id);

		tx_delay_queue_ticks =
			now_ticks - tx_desc->entry_timestamp_ticks;

		data->avgs.queue_sum_ticks += tx_delay_queue_ticks;
		data->avgs.queue_num++;
		bin = ol_tx_delay_hist_bin(pdev, tx_delay_queue_ticks);
		data->hist_bins_queue[bin]++;
	}

	/* check if it's time to start a new average */
	avg_time_ticks =
		now_ticks - pdev->tx_delay.cats[cat].avg_start_time_ticks;
	if (avg_time_ticks > pdev->tx_delay.avg_period_ticks) {
		pdev->tx_delay.cats[cat].avg_start_time_ticks = now_ticks;
		index = 1 - index;
		pdev->tx_delay.cats[cat].in_progress_idx = index;
		qdf_mem_zero(&pdev->tx_delay.cats[cat].copies[index],
			     sizeof(pdev->tx_delay.cats[cat].copies[index]));
	}

	qdf_spin_unlock_bh(&pdev->tx_delay.mutex);
}

#endif /* QCA_COMPUTE_TX_DELAY */
