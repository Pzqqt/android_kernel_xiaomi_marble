/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

#include <qdf_atomic.h>         /* qdf_atomic_inc, etc. */
#include <qdf_lock.h>           /* qdf_os_spinlock */
#include <qdf_time.h>           /* qdf_system_ticks, etc. */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <qdf_net_types.h>      /* QDF_NBUF_TX_EXT_TID_INVALID */

#include <cds_queue.h>          /* TAILQ */
#ifdef QCA_COMPUTE_TX_DELAY
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
#include <ol_tx_send.h>

#include <ol_cfg.h>             /* ol_cfg_is_high_latency */
#include <ol_tx_sched.h>
#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
#include <ol_txrx_encap.h>      /* OL_TX_RESTORE_HDR, etc */
#endif
#include <ol_tx_queue.h>
#include <ol_txrx.h>
#include <pktlog_ac_fmt.h>
#include <cdp_txrx_handle.h>
#include <wlan_reg_services_api.h>

#ifdef QCA_HL_NETDEV_FLOW_CONTROL
static u16 ol_txrx_tx_desc_alloc_table[TXRX_FC_MAX] = {
	[TXRX_FC_5GH_80M_2x2] = 2000,
	[TXRX_FC_2GH_40M_2x2] = 800,
};
#endif /* QCA_HL_NETDEV_FLOW_CONTROL */

/* tx filtering is handled within the target FW */
#define TX_FILTER_CHECK(tx_msdu_info) 0 /* don't filter */

u_int16_t
ol_tx_desc_pool_size_hl(struct cdp_cfg *ctrl_pdev)
{
	uint16_t desc_pool_size;
	uint16_t steady_state_tx_lifetime_ms;
	uint16_t safety_factor;

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

#ifdef CONFIG_TX_DESC_HI_PRIO_RESERVE

/**
 * ol_tx_hl_desc_alloc() - Allocate and initialize a tx descriptor
 *                        for a HL system.
 * @pdev: the data physical device sending the data
 * @vdev: the virtual device sending the data
 * @msdu: the tx frame
 * @msdu_info: the tx meta data
 *
 * Return: the tx decriptor
 */
static inline
struct ol_tx_desc_t *ol_tx_hl_desc_alloc(struct ol_txrx_pdev_t *pdev,
					 struct ol_txrx_vdev_t *vdev,
					 qdf_nbuf_t msdu,
					 struct ol_txrx_msdu_info_t *msdu_info)
{
	struct ol_tx_desc_t *tx_desc = NULL;

	if (qdf_atomic_read(&pdev->tx_queue.rsrc_cnt) >
	    TXRX_HL_TX_DESC_HI_PRIO_RESERVED) {
		tx_desc = ol_tx_desc_hl(pdev, vdev, msdu, msdu_info);
	} else if (qdf_nbuf_is_ipv4_pkt(msdu) == true) {
		if ((QDF_NBUF_CB_GET_PACKET_TYPE(msdu) ==
		    QDF_NBUF_CB_PACKET_TYPE_DHCP) ||
		    (QDF_NBUF_CB_GET_PACKET_TYPE(msdu) ==
		    QDF_NBUF_CB_PACKET_TYPE_EAPOL)) {
			tx_desc = ol_tx_desc_hl(pdev, vdev, msdu, msdu_info);
			ol_txrx_info("Got tx desc from resv pool\n");
		}
	}
	return tx_desc;
}

#elif defined(QCA_HL_NETDEV_FLOW_CONTROL)
bool ol_tx_desc_is_high_prio(qdf_nbuf_t msdu)
{
	enum qdf_proto_subtype proto_subtype;
	bool high_prio = false;

	if (qdf_nbuf_is_ipv4_pkt(msdu) == true) {
		if ((QDF_NBUF_CB_GET_PACKET_TYPE(msdu) ==
		    QDF_NBUF_CB_PACKET_TYPE_DHCP) ||
		    (QDF_NBUF_CB_GET_PACKET_TYPE(msdu) ==
		    QDF_NBUF_CB_PACKET_TYPE_EAPOL))
			high_prio = true;
	} else if (QDF_NBUF_CB_GET_PACKET_TYPE(msdu) ==
		   QDF_NBUF_CB_PACKET_TYPE_ARP) {
		high_prio = true;
	} else if ((QDF_NBUF_CB_GET_PACKET_TYPE(msdu) ==
		   QDF_NBUF_CB_PACKET_TYPE_ICMPv6)) {
		proto_subtype = qdf_nbuf_get_icmpv6_subtype(msdu);
		switch (proto_subtype) {
		case QDF_PROTO_ICMPV6_NA:
		case QDF_PROTO_ICMPV6_NS:
			high_prio = true;
		default:
			high_prio = false;
		}
	}
	return high_prio;
}

static inline
struct ol_tx_desc_t *ol_tx_hl_desc_alloc(struct ol_txrx_pdev_t *pdev,
					 struct ol_txrx_vdev_t *vdev,
					 qdf_nbuf_t msdu,
					 struct ol_txrx_msdu_info_t *msdu_info)
{
	struct ol_tx_desc_t *tx_desc =
			ol_tx_desc_hl(pdev, vdev, msdu, msdu_info);

	if (!tx_desc)
		return NULL;

	qdf_spin_lock_bh(&pdev->tx_mutex);
	/* return if TX flow control disabled */
	if (vdev->tx_desc_limit == 0) {
		qdf_spin_unlock_bh(&pdev->tx_mutex);
		return tx_desc;
	}

	if (!qdf_atomic_read(&vdev->os_q_paused) &&
	    (qdf_atomic_read(&vdev->tx_desc_count) >= vdev->queue_stop_th)) {
		/*
		 * Pause normal priority
		 * netdev queues if tx desc limit crosses
		 */
		pdev->pause_cb(vdev->vdev_id,
			       WLAN_STOP_NON_PRIORITY_QUEUE,
			       WLAN_DATA_FLOW_CONTROL);
		qdf_atomic_set(&vdev->os_q_paused, 1);
	} else if (ol_tx_desc_is_high_prio(msdu) && !vdev->prio_q_paused &&
		   (qdf_atomic_read(&vdev->tx_desc_count)
		   == vdev->tx_desc_limit)) {
		/* Pause high priority queue */
		pdev->pause_cb(vdev->vdev_id,
			       WLAN_NETIF_PRIORITY_QUEUE_OFF,
			       WLAN_DATA_FLOW_CONTROL_PRIORITY);
		vdev->prio_q_paused = 1;
	}
	qdf_spin_unlock_bh(&pdev->tx_mutex);

	return tx_desc;
}

#else

static inline
struct ol_tx_desc_t *ol_tx_hl_desc_alloc(struct ol_txrx_pdev_t *pdev,
					 struct ol_txrx_vdev_t *vdev,
					 qdf_nbuf_t msdu,
					 struct ol_txrx_msdu_info_t *msdu_info)
{
	struct ol_tx_desc_t *tx_desc = NULL;

	tx_desc = ol_tx_desc_hl(pdev, vdev, msdu, msdu_info);
	return tx_desc;
}
#endif

#ifdef QCA_HL_NETDEV_FLOW_CONTROL
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
	 * Define the value separately.
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
	threshold_high = ((15 * desc_pool_size) / 100) / 2;

	return threshold_high;
}

#else

static inline uint16_t
ol_txrx_rsrc_threshold_lo(int desc_pool_size)
{
	int threshold_low;
	/* always maintain a 5% margin of unallocated descriptors */
	threshold_low = (5 * desc_pool_size) / 100;

	return threshold_low;
}

static inline uint16_t
ol_txrx_rsrc_threshold_hi(int desc_pool_size)
{
	int threshold_high;
	/* when freeing up descriptors, keep going until
	 * there's a 15% margin
	 */
	threshold_high = (15 * desc_pool_size) / 100;

	return threshold_high;
}
#endif

void ol_tx_init_pdev(ol_txrx_pdev_handle pdev)
{
	uint16_t desc_pool_size, i;

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
}

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
static inline int ol_tx_encap_wrapper(struct ol_txrx_pdev_t *pdev,
				      ol_txrx_vdev_handle vdev,
				      struct ol_tx_desc_t *tx_desc,
				      qdf_nbuf_t msdu,
				      struct ol_txrx_msdu_info_t *tx_msdu_info)
{
	if (OL_TX_ENCAP(vdev, tx_desc, msdu, tx_msdu_info) != A_OK) {
		qdf_atomic_inc(&pdev->tx_queue.rsrc_cnt);
		ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1);
		if (tx_msdu_info->peer) {
			/* remove the peer reference added above */
			ol_txrx_peer_release_ref(tx_msdu_info->peer,
						 PEER_DEBUG_ID_OL_INTERNAL);
		}
		return -EINVAL;
	}

	return 0;
}
#else
static inline int ol_tx_encap_wrapper(struct ol_txrx_pdev_t *pdev,
				      ol_txrx_vdev_handle vdev,
				      struct ol_tx_desc_t *tx_desc,
				      qdf_nbuf_t msdu,
				      struct ol_txrx_msdu_info_t *tx_msdu_info)
{
	/* no-op */
	return 0;
}
#endif

/**
 * parse_ocb_tx_header() - Function to check for OCB
 * @msdu:   Pointer to OS packet (qdf_nbuf_t)
 * @tx_ctrl: TX control header on a packet and extract it if present
 *
 * Return: true if ocb parsing is successful
 */
#ifdef WLAN_FEATURE_DSRC
#define OCB_HEADER_VERSION     1
static bool parse_ocb_tx_header(qdf_nbuf_t msdu,
				struct ocb_tx_ctrl_hdr_t *tx_ctrl)
{
	qdf_ether_header_t *eth_hdr_p;
	struct ocb_tx_ctrl_hdr_t *tx_ctrl_hdr;

	/* Check if TX control header is present */
	eth_hdr_p = (qdf_ether_header_t *)qdf_nbuf_data(msdu);
	if (eth_hdr_p->ether_type != QDF_SWAP_U16(ETHERTYPE_OCB_TX))
		/* TX control header is not present. Nothing to do.. */
		return true;

	/* Remove the ethernet header */
	qdf_nbuf_pull_head(msdu, sizeof(qdf_ether_header_t));

	/* Parse the TX control header */
	tx_ctrl_hdr = (struct ocb_tx_ctrl_hdr_t *)qdf_nbuf_data(msdu);

	if (tx_ctrl_hdr->version == OCB_HEADER_VERSION) {
		if (tx_ctrl)
			qdf_mem_copy(tx_ctrl, tx_ctrl_hdr,
				     sizeof(*tx_ctrl_hdr));
	} else {
		/* The TX control header is invalid. */
		return false;
	}

	/* Remove the TX control header */
	qdf_nbuf_pull_head(msdu, tx_ctrl_hdr->length);
	return true;
}
#else
static bool parse_ocb_tx_header(qdf_nbuf_t msdu,
				struct ocb_tx_ctrl_hdr_t *tx_ctrl)
{
	return true;
}
#endif

/**
 * ol_txrx_mgmt_tx_desc_alloc() - Allocate and initialize a tx descriptor
 *				 for management frame
 * @pdev: the data physical device sending the data
 * @vdev: the virtual device sending the data
 * @tx_mgmt_frm: the tx management frame
 * @tx_msdu_info: the tx meta data
 *
 * Return: the tx decriptor
 */
struct ol_tx_desc_t *
ol_txrx_mgmt_tx_desc_alloc(
	struct ol_txrx_pdev_t *pdev,
	struct ol_txrx_vdev_t *vdev,
	qdf_nbuf_t tx_mgmt_frm,
	struct ol_txrx_msdu_info_t *tx_msdu_info)
{
	struct ol_tx_desc_t *tx_desc;

	tx_msdu_info->htt.action.tx_comp_req = 1;
	tx_desc = ol_tx_desc_hl(pdev, vdev, tx_mgmt_frm, tx_msdu_info);
	return tx_desc;
}

/**
 * ol_txrx_mgmt_send_frame() - send a management frame
 * @vdev: virtual device sending the frame
 * @tx_desc: tx desc
 * @tx_mgmt_frm: management frame to send
 * @tx_msdu_info: the tx meta data
 * @chanfreq: download change frequency
 *
 * Return:
 *      0 -> the frame is accepted for transmission, -OR-
 *      1 -> the frame was not accepted
 */
int ol_txrx_mgmt_send_frame(
	struct ol_txrx_vdev_t *vdev,
	struct ol_tx_desc_t *tx_desc,
	qdf_nbuf_t tx_mgmt_frm,
	struct ol_txrx_msdu_info_t *tx_msdu_info,
	uint16_t chanfreq)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	struct ol_tx_frms_queue_t *txq;
	int status = 1;

	/*
	 * 1.  Look up the peer and queue the frame in the peer's mgmt queue.
	 * 2.  Invoke the download scheduler.
	 */
	txq = ol_tx_classify_mgmt(vdev, tx_desc, tx_mgmt_frm, tx_msdu_info);
	if (!txq) {
		/* TXRX_STATS_MSDU_LIST_INCR(vdev->pdev, tx.dropped.no_txq,
		 *			     msdu);
		 */
		qdf_atomic_inc(&pdev->tx_queue.rsrc_cnt);
		ol_tx_desc_frame_free_nonstd(vdev->pdev, tx_desc,
					     1 /* error */);
		goto out; /* can't accept the tx mgmt frame */
	}
	/* Initialize the HTT tx desc l2 header offset field.
	 * Even though tx encap does not apply to mgmt frames,
	 * htt_tx_desc_mpdu_header still needs to be called,
	 * to specifiy that there was no L2 header added by tx encap,
	 * so the frame's length does not need to be adjusted to account for
	 * an added L2 header.
	 */
	htt_tx_desc_mpdu_header(tx_desc->htt_tx_desc, 0);
	if (qdf_unlikely(htt_tx_desc_init(
			pdev->htt_pdev, tx_desc->htt_tx_desc,
			tx_desc->htt_tx_desc_paddr,
			ol_tx_desc_id(pdev, tx_desc),
			tx_mgmt_frm,
			&tx_msdu_info->htt, &tx_msdu_info->tso_info, NULL, 0)))
		goto out;
	htt_tx_desc_display(tx_desc->htt_tx_desc);
	htt_tx_desc_set_chanfreq(tx_desc->htt_tx_desc, chanfreq);

	ol_tx_enqueue(vdev->pdev, txq, tx_desc, tx_msdu_info);
	ol_tx_sched(vdev->pdev);
	status = 0;
out:
	if (tx_msdu_info->peer) {
		/* remove the peer reference added above */
		ol_txrx_peer_release_ref(tx_msdu_info->peer,
					 PEER_DEBUG_ID_OL_INTERNAL);
	}

	return status;
}

/**
 * ol_tx_hl_base() - send tx frames for a HL system.
 * @vdev: the virtual device sending the data
 * @tx_spec: indicate what non-standard transmission actions to apply
 * @msdu_list: the tx frames to send
 * @tx_comp_req: tx completion req
 *
 * Return: NULL if all MSDUs are accepted
 */
static inline qdf_nbuf_t
ol_tx_hl_base(
	ol_txrx_vdev_handle vdev,
	enum ol_tx_spec tx_spec,
	qdf_nbuf_t msdu_list,
	int tx_comp_req)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	qdf_nbuf_t msdu = msdu_list;
	struct ol_txrx_msdu_info_t tx_msdu_info;
	struct ocb_tx_ctrl_hdr_t tx_ctrl;
	htt_pdev_handle htt_pdev = pdev->htt_pdev;

	tx_msdu_info.tso_info.is_tso = 0;

	/*
	 * The msdu_list variable could be used instead of the msdu var,
	 * but just to clarify which operations are done on a single MSDU
	 * vs. a list of MSDUs, use a distinct variable for single MSDUs
	 * within the list.
	 */
	while (msdu) {
		qdf_nbuf_t next;
		struct ol_tx_frms_queue_t *txq;
		struct ol_tx_desc_t *tx_desc = NULL;

		qdf_mem_zero(&tx_ctrl, sizeof(tx_ctrl));
		tx_msdu_info.peer = NULL;
		/*
		 * The netbuf will get stored into a (peer-TID) tx queue list
		 * inside the ol_tx_classify_store function or else dropped,
		 * so store the next pointer immediately.
		 */
		next = qdf_nbuf_next(msdu);

		tx_desc = ol_tx_hl_desc_alloc(pdev, vdev, msdu, &tx_msdu_info);

		if (!tx_desc) {
			/*
			 * If we're out of tx descs, there's no need to try
			 * to allocate tx descs for the remaining MSDUs.
			 */
			TXRX_STATS_MSDU_LIST_INCR(pdev, tx.dropped.host_reject,
						  msdu);
			return msdu; /* the list of unaccepted MSDUs */
		}

		/* OL_TXRX_PROT_AN_LOG(pdev->prot_an_tx_sent, msdu);*/

		if (tx_spec != OL_TX_SPEC_STD) {
#if defined(FEATURE_WLAN_TDLS)
			if (tx_spec & OL_TX_SPEC_NO_FREE) {
				tx_desc->pkt_type = OL_TX_FRM_NO_FREE;
			} else if (tx_spec & OL_TX_SPEC_TSO) {
#else
				if (tx_spec & OL_TX_SPEC_TSO) {
#endif
					tx_desc->pkt_type = OL_TX_FRM_TSO;
				}
				if (ol_txrx_tx_is_raw(tx_spec)) {
					/* CHECK THIS: does this need
					 * to happen after htt_tx_desc_init?
					 */
					/* different types of raw frames */
					u_int8_t sub_type =
						ol_txrx_tx_raw_subtype(
								tx_spec);
					htt_tx_desc_type(htt_pdev,
							 tx_desc->htt_tx_desc,
							 htt_pkt_type_raw,
							 sub_type);
				}
			}

			tx_msdu_info.htt.info.ext_tid = qdf_nbuf_get_tid(msdu);
			tx_msdu_info.htt.info.vdev_id = vdev->vdev_id;
			tx_msdu_info.htt.info.frame_type = htt_frm_type_data;
			tx_msdu_info.htt.info.l2_hdr_type = pdev->htt_pkt_type;
			tx_msdu_info.htt.action.tx_comp_req = tx_comp_req;

			/* If the vdev is in OCB mode,
			 * parse the tx control header.
			 */
			if (vdev->opmode == wlan_op_mode_ocb) {
				if (!parse_ocb_tx_header(msdu, &tx_ctrl)) {
					/* There was an error parsing
					 * the header.Skip this packet.
					 */
					goto MSDU_LOOP_BOTTOM;
				}
			}

			txq = ol_tx_classify(vdev, tx_desc, msdu,
					     &tx_msdu_info);

			/* initialize the HW tx descriptor */
			htt_tx_desc_init(
					pdev->htt_pdev, tx_desc->htt_tx_desc,
					tx_desc->htt_tx_desc_paddr,
					ol_tx_desc_id(pdev, tx_desc),
					msdu,
					&tx_msdu_info.htt,
					&tx_msdu_info.tso_info,
					&tx_ctrl,
					vdev->opmode == wlan_op_mode_ocb);

			if ((!txq) || TX_FILTER_CHECK(&tx_msdu_info)) {
				/* drop this frame,
				 * but try sending subsequent frames
				 */
				/* TXRX_STATS_MSDU_LIST_INCR(pdev,
				 * tx.dropped.no_txq, msdu);
				 */
				qdf_atomic_inc(&pdev->tx_queue.rsrc_cnt);
				ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1);
				if (tx_msdu_info.peer) {
					/* remove the peer reference
					 * added above
					 */
					ol_txrx_peer_release_ref(
						tx_msdu_info.peer,
						PEER_DEBUG_ID_OL_INTERNAL);
				}
				goto MSDU_LOOP_BOTTOM;
			}

			if (tx_msdu_info.peer) {
				/*
				 * If the state is not associated then drop all
				 * the data packets received for that peer
				 */
				if (tx_msdu_info.peer->state ==
						OL_TXRX_PEER_STATE_DISC) {
					qdf_atomic_inc(
						&pdev->tx_queue.rsrc_cnt);
					ol_tx_desc_frame_free_nonstd(pdev,
								     tx_desc,
								     1);
					ol_txrx_peer_release_ref(
						tx_msdu_info.peer,
						PEER_DEBUG_ID_OL_INTERNAL);
					msdu = next;
					continue;
				} else if (tx_msdu_info.peer->state !=
						OL_TXRX_PEER_STATE_AUTH) {
					if (tx_msdu_info.htt.info.ethertype !=
						ETHERTYPE_PAE &&
						tx_msdu_info.htt.info.ethertype
							!= ETHERTYPE_WAI) {
						qdf_atomic_inc(
							&pdev->tx_queue.
								rsrc_cnt);
						ol_tx_desc_frame_free_nonstd(
								pdev,
								tx_desc, 1);
						ol_txrx_peer_release_ref(
						 tx_msdu_info.peer,
						 PEER_DEBUG_ID_OL_INTERNAL);
						msdu = next;
						continue;
					}
				}
			}
			/*
			 * Initialize the HTT tx desc l2 header offset field.
			 * htt_tx_desc_mpdu_header  needs to be called to
			 * make sure, the l2 header size is initialized
			 * correctly to handle cases where TX ENCAP is disabled
			 * or Tx Encap fails to perform Encap
			 */
			htt_tx_desc_mpdu_header(tx_desc->htt_tx_desc, 0);

			/*
			 * Note: when the driver is built without support for
			 * SW tx encap,the following macro is a no-op.
			 * When the driver is built with support for SW tx
			 * encap, it performs encap, and if an error is
			 * encountered, jumps to the MSDU_LOOP_BOTTOM label.
			 */
			if (ol_tx_encap_wrapper(pdev, vdev, tx_desc, msdu,
						&tx_msdu_info))
				goto MSDU_LOOP_BOTTOM;

			/*
			 * If debug display is enabled, show the meta-data
			 * being downloaded to the target via the
			 * HTT tx descriptor.
			 */
			htt_tx_desc_display(tx_desc->htt_tx_desc);

			ol_tx_enqueue(pdev, txq, tx_desc, &tx_msdu_info);
			if (tx_msdu_info.peer) {
				OL_TX_PEER_STATS_UPDATE(tx_msdu_info.peer,
							msdu);
				/* remove the peer reference added above */
				ol_txrx_peer_release_ref
						(tx_msdu_info.peer,
						 PEER_DEBUG_ID_OL_INTERNAL);
			}
MSDU_LOOP_BOTTOM:
			msdu = next;
		}
		ol_tx_sched(pdev);
		return NULL; /* all MSDUs were accepted */
}

qdf_nbuf_t
ol_tx_hl(ol_txrx_vdev_handle vdev, qdf_nbuf_t msdu_list)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	int tx_comp_req = pdev->cfg.default_tx_comp_req ||
				pdev->cfg.request_tx_comp;

	return ol_tx_hl_base(vdev, OL_TX_SPEC_STD, msdu_list, tx_comp_req);
}

qdf_nbuf_t ol_tx_non_std_hl(struct ol_txrx_vdev_t *vdev,
			    enum ol_tx_spec tx_spec,
			    qdf_nbuf_t msdu_list)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	int tx_comp_req = pdev->cfg.default_tx_comp_req ||
				pdev->cfg.request_tx_comp;

	if (!tx_comp_req) {
		if ((tx_spec == OL_TX_SPEC_NO_FREE) &&
		    (pdev->tx_data_callback.func))
			tx_comp_req = 1;
	}
	return ol_tx_hl_base(vdev, tx_spec, msdu_list, tx_comp_req);
}

#ifdef FEATURE_WLAN_TDLS
/**
 * ol_txrx_copy_mac_addr_raw() - copy raw mac addr
 * @vdev: the data virtual device
 * @bss_addr: bss address
 *
 * Return: None
 */
void ol_txrx_copy_mac_addr_raw(struct cdp_vdev *pvdev, uint8_t *bss_addr)
{
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t  *)pvdev;

	qdf_spin_lock_bh(&vdev->pdev->last_real_peer_mutex);
	if (bss_addr && vdev->last_real_peer &&
	    !qdf_mem_cmp((u8 *)bss_addr,
			     vdev->last_real_peer->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE))
		qdf_mem_copy(vdev->hl_tdls_ap_mac_addr.raw,
			     vdev->last_real_peer->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE);
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
void
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
bool is_vdev_restore_last_peer(void *ppeer)
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
void ol_txrx_update_last_real_peer(struct cdp_pdev *ppdev, void *pvdev,
				   uint8_t *peer_id, bool restore_last_peer)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	struct ol_txrx_vdev_t *vdev = (struct ol_txrx_vdev_t *)pvdev;
	struct ol_txrx_peer_t *peer;

	if (!restore_last_peer)
		return;

	peer = ol_txrx_find_peer_by_addr((struct cdp_pdev *)pdev,
					 vdev->hl_tdls_ap_mac_addr.raw,
					 peer_id);

	qdf_spin_lock_bh(&pdev->last_real_peer_mutex);
	if (!vdev->last_real_peer && peer &&
	    (peer->peer_ids[0] != HTT_INVALID_PEER_ID))
		vdev->last_real_peer = peer;
	qdf_spin_unlock_bh(&pdev->last_real_peer_mutex);
}
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(DEBUG_HL_LOGGING)
/**
 * ol_txrx_pdev_txq_log_init() - initialise pdev txq logs
 * @pdev: the physical device object
 *
 * Return: None
 */
void ol_txrx_pdev_txq_log_init(struct ol_txrx_pdev_t *pdev)
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
void ol_txrx_pdev_txq_log_destroy(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_destroy(&pdev->txq_log_spinlock);
}
#endif

#if defined(DEBUG_HL_LOGGING)

/**
 * ol_txrx_pdev_grp_stats_init() - initialise group stat spinlock for pdev
 * @pdev: the physical device object
 *
 * Return: None
 */
void ol_txrx_pdev_grp_stats_init(struct ol_txrx_pdev_t *pdev)
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
void ol_txrx_pdev_grp_stat_destroy(struct ol_txrx_pdev_t *pdev)
{
	qdf_spinlock_destroy(&pdev->grp_stat_spinlock);
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

/**
 * ol_txrx_vdev_txqs_init() - initialise vdev tx queues
 * @vdev: the virtual device object
 *
 * Return: None
 */
void ol_txrx_vdev_txqs_init(struct ol_txrx_vdev_t *vdev)
{
	uint8_t i;

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
void ol_txrx_vdev_tx_queue_free(struct ol_txrx_vdev_t *vdev)
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
void ol_txrx_peer_txqs_init(struct ol_txrx_pdev_t *pdev,
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
void ol_txrx_peer_tx_queue_free(struct ol_txrx_pdev_t *pdev,
				struct ol_txrx_peer_t *peer)
{
	struct ol_tx_frms_queue_t *txq;
	uint8_t i;

	for (i = 0; i < OL_TX_NUM_TIDS; i++) {
		txq = &peer->txqs[i];
		ol_tx_queue_free(pdev, txq, i, true);
	}
}

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
		ol_txrx_warn("invalid group_id=%u, ignore update", group_id);
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

	credit += ol_txrx_distribute_group_credits(pdev, group_id,
						   vdev_id_mask);
	/*
	 * membership (vdev id mask and ac mask) is not matching
	 * TODO: ignoring ac mask for now
	 */
	qdf_assert(ac_mask == 0xffff);
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

#if defined(FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL) && \
	defined(FEATURE_HL_DBS_GROUP_CREDIT_SHARING)
#define MIN_INIT_GROUP_CREDITS	10
int ol_txrx_distribute_group_credits(struct ol_txrx_pdev_t *pdev,
				     u8 group_id,
				     u32 vdevid_mask_new)
{
	struct ol_tx_queue_group_t *grp = &pdev->txq_grps[group_id];
	struct ol_tx_queue_group_t *grp_nxt = &pdev->txq_grps[!group_id];
	int creds_nxt = qdf_atomic_read(&grp_nxt->credit);
	int vdevid_mask = OL_TXQ_GROUP_VDEV_ID_MASK_GET(grp->membership);
	int vdevid_mask_othgrp =
		OL_TXQ_GROUP_VDEV_ID_MASK_GET(grp_nxt->membership);
	int creds_distribute = 0;

	/* if vdev added to the group is the first vdev */
	if ((vdevid_mask == 0) && (vdevid_mask_new != 0)) {
		/* if other group has members */
		if (vdevid_mask_othgrp) {
			if (creds_nxt < MIN_INIT_GROUP_CREDITS)
				creds_distribute = creds_nxt / 2;
			else
				creds_distribute = MIN_INIT_GROUP_CREDITS;

			ol_txrx_update_group_credit(grp_nxt, -creds_distribute,
						    0);
		} else {
			/*
			 * Other grp has no members, give all credits to this
			 * grp.
			 */
			creds_distribute =
				qdf_atomic_read(&pdev->target_tx_credit);
		}
	/* if all vdevs are removed from this grp */
	} else if ((vdevid_mask != 0) && (vdevid_mask_new == 0)) {
		if (vdevid_mask_othgrp)
			/* Transfer credits to other grp */
			ol_txrx_update_group_credit(grp_nxt,
						    qdf_atomic_read(&grp->
						    credit),
						    0);
		/* Set current grp credits to zero */
		ol_txrx_update_group_credit(grp, 0, 1);
	}

	return creds_distribute;
}
#endif /*
	* FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL &&
	* FEATURE_HL_DBS_GROUP_CREDIT_SHARING
	*/

#ifdef QCA_HL_NETDEV_FLOW_CONTROL
/**
 * ol_txrx_register_hl_flow_control() -register hl netdev flow control callback
 * @vdev_id: vdev_id
 * @flowControl: flow control callback
 *
 * Return: 0 for success or error code
 */
int ol_txrx_register_hl_flow_control(struct cdp_soc_t *soc,
				     tx_pause_callback flowcontrol)
{
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	u32 desc_pool_size = ol_tx_desc_pool_size_hl(pdev->ctrl_pdev);

	/*
	 * Assert if the tx descriptor pool size meets the requirements
	 * Maximum 2 sessions are allowed on a band.
	 */
	QDF_ASSERT((2 * ol_txrx_tx_desc_alloc_table[TXRX_FC_5GH_80M_2x2] +
		    ol_txrx_tx_desc_alloc_table[TXRX_FC_2GH_40M_2x2])
		    <= desc_pool_size);

	if (!pdev || !flowcontrol) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "pdev or pause_cb is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev->pause_cb = flowcontrol;
	return 0;
}

int ol_txrx_set_vdev_os_queue_status(u8 vdev_id,
				     enum netif_action_type action)
{
	struct ol_txrx_vdev_t *vdev =
	(struct ol_txrx_vdev_t *)ol_txrx_get_vdev_from_vdev_id(vdev_id);

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid vdev_id %d", __func__, vdev_id);
		return -EINVAL;
	}

	switch (action) {
	case WLAN_NETIF_PRIORITY_QUEUE_ON:
		qdf_spin_lock_bh(&vdev->pdev->tx_mutex);
		vdev->prio_q_paused = 0;
		qdf_spin_unlock_bh(&vdev->pdev->tx_mutex);
		break;
	case WLAN_WAKE_NON_PRIORITY_QUEUE:
		qdf_atomic_set(&vdev->os_q_paused, 0);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid action %d", __func__, action);
		return -EINVAL;
	}
	return 0;
}

/**
 * ol_txrx_set_vdev_tx_desc_limit() - Set TX descriptor limits for a vdev
 * @vdev_id: vdev id for the vdev under consideration.
 * @chan: Channel on which the vdev has been started.
 */
int ol_txrx_set_vdev_tx_desc_limit(u8 vdev_id, u8 chan)
{
	struct ol_txrx_vdev_t *vdev =
	(struct ol_txrx_vdev_t *)ol_txrx_get_vdev_from_vdev_id(vdev_id);
	enum ol_txrx_fc_limit_id fc_limit_id;
	u32 td_limit;

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid vdev_id %d", __func__, vdev_id);
		return -EINVAL;
	}

	/* TODO: Handle no of spatial streams and channel BW */
	if (WLAN_REG_IS_5GHZ_CH(chan))
		fc_limit_id = TXRX_FC_5GH_80M_2x2;
	else
		fc_limit_id = TXRX_FC_2GH_40M_2x2;

	qdf_spin_lock_bh(&vdev->pdev->tx_mutex);
	td_limit = ol_txrx_tx_desc_alloc_table[fc_limit_id];
	vdev->tx_desc_limit = td_limit;
	vdev->queue_stop_th = td_limit - TXRX_HL_TX_DESC_HI_PRIO_RESERVED;
	vdev->queue_restart_th = td_limit - TXRX_HL_TX_DESC_QUEUE_RESTART_TH;
	qdf_spin_unlock_bh(&vdev->pdev->tx_mutex);

	return 0;
}
#endif /* QCA_HL_NETDEV_FLOW_CONTROL */
