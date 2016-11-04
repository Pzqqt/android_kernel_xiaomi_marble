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
#ifndef __DP_TX_H
#define __DP_TX_H

#include <qdf_types.h>
#include <qdf_nbuf.h>
#include <dp_types.h>

#define DP_STATS_ADD(x, y, z)
#define DP_STATS_SUB(x, y, z)
#define DP_STATS_MSDU_INCR(x, y, z)

#define DP_TX_MAX_NUM_FRAGS 6

#define DP_TX_DESC_FLAG_ALLOCATED	0x1
#define DP_TX_DESC_FLAG_TO_FW		0x2
#define DP_TX_DESC_FLAG_FRAG		0x4
#define DP_TX_DESC_FLAG_RAW		0x8
#define DP_TX_DESC_FLAG_MESH		0x10
#define DP_TX_DESC_FLAG_QUEUED_TX		0x20
#define DP_TX_DESC_FLAG_COMPLETED_TX		0x40

#define DP_TX_FREE_SINGLE_BUF(soc, vdev, buf)                  \
do {                                                           \
	qdf_nbuf_unmap(soc->osdev, buf, QDF_DMA_FROM_DEVICE);  \
	qdf_nbuf_free(buf);                                    \
} while (0)

#define OCB_HEADER_VERSION	 1

/**
 * struct dp_tx_frag_info_s
 * @vaddr: hlos vritual address for buffer
 * @paddr_lo: physical address lower 32bits
 * @paddr_hi: physical address higher bits
 * @len: length of the buffer
 */
struct dp_tx_frag_info_s {
	uint8_t  *vaddr;
	uint32_t paddr_lo;
	uint16_t paddr_hi;
	uint16_t len;
};

/**
 * struct dp_tx_seg_info_s - Segmentation Descriptor
 * @nbuf: NBUF pointer if segment corresponds to separate nbuf
 * @frag_cnt: Fragment count in this segment
 * @total_len: Total length of segment
 * @frags: per-Fragment information
 * @next: pointer to next MSDU segment
 */
struct dp_tx_seg_info_s  {
	qdf_nbuf_t nbuf;
	uint16_t frag_cnt;
	uint16_t total_len;
	struct dp_tx_frag_info_s frags[DP_TX_MAX_NUM_FRAGS];
	struct dp_tx_seg_info_s *next;
};

/**
 * struct dp_tx_sg_info_s - Scatter Gather Descriptor
 * @num_segs: Number of segments (TSO/ME) in the frame
 * @total_len: Total length of the frame
 * @curr_seg: Points to current segment descriptor to be processed. Chain of
 * 	      descriptors for SG frames/multicast-unicast converted packets.
 *
 * Used for SG (802.3 or Raw) frames and Multicast-Unicast converted frames to
 * carry fragmentation information
 * Raw Frames will be handed over to driver as an SKB chain with MPDU boundaries
 * indicated through flags in SKB CB (first_msdu and last_msdu). This will be
 * converted into set of skb sg (nr_frags) structures.
 */
struct dp_tx_sg_info_s {
	uint32_t num_segs;
	uint32_t total_len;
	struct dp_tx_seg_info_s *curr_seg;
};

/**
 * struct dp_tx_queue - Tx queue
 * @desc_pool_id: Descriptor Pool to be used for the tx queue
 * @ring_id: TCL descriptor ring ID corresponding to the tx queue
 *
 * Tx queue contains information of the software (Descriptor pool)
 * and hardware resources (TCL ring id) to be used for a particular
 * transmit queue (obtained from skb_queue_mapping in case of linux)
 */
struct dp_tx_queue {
	uint8_t desc_pool_id;
	uint8_t ring_id;
};

/**
 * struct dp_tx_msdu_info_s - MSDU Descriptor
 * @frm_type: Frame type - Regular/TSO/SG/Multicast enhancement
 * @tx_queue: Tx queue on which this MSDU should be transmitted
 * @num_seg: Number of segments (TSO)
 * @tid: TID (override) that is sent from HLOS
 * @u.tso_info: TSO information for TSO frame types
 * 	     (chain of the TSO segments, number of segments)
 * @u.sg_info: Scatter Gather information for non-TSO SG frames
 *
 * This structure holds the complete MSDU information needed to program the
 * Hardware TCL and MSDU extension descriptors for different frame types
 *
 */
struct dp_tx_msdu_info_s {
	enum dp_tx_frm_type frm_type;
	struct dp_tx_queue tx_queue;
	uint32_t num_seg;
	uint8_t tid;
	union {
		struct qdf_tso_info_t tso_info;
		struct dp_tx_sg_info_s sg_info;
	} u;
};

struct meta_hdr_s {
	uint8_t magic;
	uint8_t flags;
	uint8_t channel;
	uint8_t keyix;
	uint8_t rssi;
	uint8_t silence;
	uint8_t power;
	uint8_t retries;
	uint8_t max_tries[2];
	uint8_t rates[2];
	uint8_t unused[4];
};

#define METAHDR_FLAG_TX                 (1<<0) /* packet transmission */
#define METAHDR_FLAG_TX_FAIL            (1<<1) /* transmission failed */
#define METAHDR_FLAG_TX_USED_ALT_RATE   (1<<2) /* used alternate bitrate */
#define METAHDR_FLAG_AUTO_RATE          (1<<5)
#define METAHDR_FLAG_NOENCRYPT          (1<<6)
#define METAHDR_FLAG_NOQOS              (1<<7)

#define METAHDR_FLAG_RX_ERR             (1<<3) /* failed crc check */
#define METAHDR_FLAG_RX_MORE            (1<<4) /* first part of a frag skb */
#define METAHDR_FLAG_LOG                (1<<7)

#define METAHDR_FLAG_RX_4SS             (1<<1) /* rx 4ss frame */

QDF_STATUS dp_tx_vdev_attach(struct dp_vdev *vdev);
QDF_STATUS dp_tx_vdev_detach(struct dp_vdev *vdev);

QDF_STATUS dp_tx_soc_attach(struct dp_soc *soc);
QDF_STATUS dp_tx_soc_detach(struct dp_soc *soc);

QDF_STATUS dp_tx_pdev_detach(struct dp_pdev *pdev);
QDF_STATUS dp_tx_pdev_attach(struct dp_pdev *pdev);

qdf_nbuf_t dp_tx_send(void *data_vdev, qdf_nbuf_t nbuf);

uint32_t dp_tx_comp_handler(struct dp_soc *soc, uint32_t ring_id,
		uint32_t budget);

/* TODO TX_FEATURE_NOT_YET */
static inline void dp_tx_comp_process_exception(struct dp_tx_desc_s *tx_desc)
{
	return;
}
static inline void dp_tx_comp_process_tx_status(struct dp_tx_desc_s *tx_desc)
{
	return;
}
static inline QDF_STATUS dp_tx_flow_control(struct dp_vdev *vdev)
{
	return  QDF_STATUS_SUCCESS;
}
static inline QDF_STATUS dp_tx_prepare_tso(struct dp_vdev *vdev,
		qdf_nbuf_t nbuf, struct dp_tx_msdu_info_s *msdu_info)
{
	return  QDF_STATUS_SUCCESS;
}
static inline qdf_nbuf_t dp_tx_prepare_me(struct dp_vdev *vdev,
		qdf_nbuf_t nbuf, struct dp_tx_msdu_info_s *msdu_info)
{
	return nbuf;
}
/* TODO TX_FEATURE_NOT_YET */
#endif
