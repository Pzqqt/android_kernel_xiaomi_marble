/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
#ifndef _DP_RX_MON_1_0_H_
#define _DP_RX_MON_1_0_H_

#include <dp_rx.h>
/*
 * MON_BUF_MIN_ENTRIES macro defines minimum number of network buffers
 * to be refilled in the RXDMA monitor buffer ring at init, remaining
 * buffers are replenished at the time of monitor vap creation
 */
#define MON_BUF_MIN_ENTRIES 64

/*
 * The below macro defines the maximum number of ring entries that would
 * be processed in a single instance when processing each of the non-monitoring
 * RXDMA2SW ring.
 */
#define MON_DROP_REAP_LIMIT 64

/*
 * The maximum headroom reserved for monitor destination buffer to
 * accomodate radiotap header and protocol flow tag
 */
#ifdef DP_RX_MON_MEM_FRAG
/*
 *  -------------------------------------------------
 * |       Protocol & Flow TAG      | Radiotap header|
 * |                                |  Length(128 B) |
 * |  ((4* QDF_NBUF_MAX_FRAGS) * 2) |                |
 *  -------------------------------------------------
 */
#define DP_RX_MON_MAX_RADIO_TAP_HDR (128)
#define DP_RX_MON_PF_TAG_LEN_PER_FRAG (4)
#define DP_RX_MON_TOT_PF_TAG_LEN \
	((DP_RX_MON_PF_TAG_LEN_PER_FRAG) * (QDF_NBUF_MAX_FRAGS))
#define DP_RX_MON_MAX_MONITOR_HEADER \
	((DP_RX_MON_TOT_PF_TAG_LEN * 2) + (DP_RX_MON_MAX_RADIO_TAP_HDR))
#endif

/* l2 header pad byte in case of Raw frame is Zero and 2 in non raw */
#define DP_RX_MON_RAW_L2_HDR_PAD_BYTE (0)
#define DP_RX_MON_NONRAW_L2_HDR_PAD_BYTE (2)

QDF_STATUS dp_rx_pdev_mon_status_buffers_alloc(struct dp_pdev *pdev,
					       uint32_t mac_id);
QDF_STATUS dp_rx_pdev_mon_status_desc_pool_alloc(struct dp_pdev *pdev,
						 uint32_t mac_id);
void dp_rx_pdev_mon_status_desc_pool_init(struct dp_pdev *pdev,
					  uint32_t mac_id);
void dp_rx_pdev_mon_status_desc_pool_deinit(struct dp_pdev *pdev,
					    uint32_t mac_id);
void dp_rx_pdev_mon_status_desc_pool_free(struct dp_pdev *pdev,
					  uint32_t mac_id);
void dp_rx_pdev_mon_status_buffers_free(struct dp_pdev *pdev, uint32_t mac_id);

QDF_STATUS dp_rx_pdev_mon_desc_pool_alloc(struct dp_pdev *pdev);
QDF_STATUS dp_rx_pdev_mon_buffers_alloc(struct dp_pdev *pdev);
void dp_rx_pdev_mon_buffers_free(struct dp_pdev *pdev);
void dp_rx_pdev_mon_desc_pool_init(struct dp_pdev *pdev);
void dp_rx_pdev_mon_desc_pool_deinit(struct dp_pdev *pdev);
void dp_rx_pdev_mon_desc_pool_free(struct dp_pdev *pdev);

/**
 * dp_rx_mon_dest_process() - Brain of the Rx processing functionality
 *	Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 * @soc: core txrx main contex
 * @int_ctx: interrupt context
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the core of Rx functionality. This is
 * expected to handle only non-error frames.
 *
 * Return: none
 */
#ifdef QCA_MONITOR_PKT_SUPPORT
void dp_rx_mon_dest_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			    uint32_t mac_id, uint32_t quota);

void dp_rx_pdev_mon_buf_buffers_free(struct dp_pdev *pdev, uint32_t mac_id);
QDF_STATUS
dp_rx_pdev_mon_buf_buffers_alloc(struct dp_pdev *pdev, uint32_t mac_id,
				 bool delayed_replenish);
QDF_STATUS
dp_rx_pdev_mon_buf_desc_pool_alloc(struct dp_pdev *pdev, uint32_t mac_id);
void
dp_rx_pdev_mon_buf_desc_pool_init(struct dp_pdev *pdev, uint32_t mac_id);
#else
static inline
void dp_rx_mon_dest_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			    uint32_t mac_id, uint32_t quota)
{
}

static inline
void dp_rx_pdev_mon_buf_buffers_free(struct dp_pdev *pdev, uint32_t mac_id)
{
}

static inline QDF_STATUS
dp_rx_pdev_mon_buf_buffers_alloc(struct dp_pdev *pdev, uint32_t mac_id,
				 bool delayed_replenish)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
dp_rx_pdev_mon_buf_desc_pool_alloc(struct dp_pdev *pdev, uint32_t mac_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
dp_rx_pdev_mon_buf_desc_pool_init(struct dp_pdev *pdev, uint32_t mac_id)
{
}
#endif

#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
/**
 * dp_mon_dest_srng_drop_for_mac() - Drop the mon dest ring packets for
 *  a given mac
 * @pdev: DP pdev
 * @mac_id: mac id
 *
 * Return: None
 */
uint32_t
dp_mon_dest_srng_drop_for_mac(struct dp_pdev *pdev, uint32_t mac_id);
#endif

/**
 * dp_rxdma_err_process() - RxDMA error processing functionality
 * @soc: core txrx main contex
 * @mac_id: mac id which is one of 3 mac_ids
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * Return: num of buffers processed
 */
uint32_t dp_rxdma_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
			      uint32_t mac_id, uint32_t quota);

/**
 * dp_mon_buf_delayed_replenish() - Helper routine to replenish monitor dest buf
 * @pdev: DP pdev object
 *
 * Return: None
 */
void dp_mon_buf_delayed_replenish(struct dp_pdev *pdev);

#ifdef QCA_MONITOR_PKT_SUPPORT
/**
 * dp_rx_mon_link_desc_return() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @dp_pdev: core txrx pdev context
 * @buf_addr_info: void pointer to monitor link descriptor buf addr info
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_mon_link_desc_return(struct dp_pdev *dp_pdev,
			   hal_buff_addrinfo_t buf_addr_info,
			   int mac_id);
#else
static inline QDF_STATUS
dp_rx_mon_link_desc_return(struct dp_pdev *dp_pdev,
			   hal_buff_addrinfo_t buf_addr_info,
			   int mac_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_mon_adjust_frag_len() - MPDU and MSDU may spread across
 *				multiple nbufs. This function
 *                              is to return data length in
 *				fragmented buffer
 * @soc: Datapath soc handle
 * @total_len: pointer to remaining data length.
 * @frag_len: pointer to data length in this fragment.
 * @l2_hdr_pad: l2 header padding
 */
static inline void dp_mon_adjust_frag_len(struct dp_soc *soc,
					  uint32_t *total_len,
					  uint32_t *frag_len,
					  uint16_t l2_hdr_pad)
{
	uint32_t rx_pkt_tlv_len = soc->rx_pkt_tlv_size;

	if (*total_len >= (RX_MONITOR_BUFFER_SIZE - rx_pkt_tlv_len)) {
		*frag_len = RX_MONITOR_BUFFER_SIZE - rx_pkt_tlv_len -
					l2_hdr_pad;
		*total_len -= *frag_len;
	} else {
		*frag_len = *total_len;
		*total_len = 0;
	}
}

/**
 * dp_rx_mon_frag_adjust_frag_len() - MPDU and MSDU may spread across
 * multiple nbufs. This function is to return data length in
 * fragmented buffer.
 * It takes input as max_limit for any buffer(as it changes based
 * on decap type and buffer sequence in MSDU.
 *
 * If MSDU is divided into multiple buffer then below format will
 * be max limit.
 * Decap type Non-Raw
 *--------------------------------
 *|  1st  |  2nd  | ...  | Last   |
 *| 1662  |  1664 | 1664 | <=1664 |
 *--------------------------------
 * Decap type Raw
 *--------------------------------
 *|  1st  |  2nd  | ...  | Last   |
 *| 1664  |  1664 | 1664 | <=1664 |
 *--------------------------------
 *
 * It also calculate if current buffer has placeholder to keep padding byte.
 *  --------------------------------
 * |       MAX LIMIT(1662/1664)     |
 *  --------------------------------
 * | Actual Data | Pad byte Pholder |
 *  --------------------------------
 *
 * @total_len: Remaining data length.
 * @frag_len:  Data length in this fragment.
 * @max_limit: Max limit of current buffer/MSDU.
 */
#ifdef DP_RX_MON_MEM_FRAG
static inline
void dp_rx_mon_frag_adjust_frag_len(uint32_t *total_len, uint32_t *frag_len,
				    uint32_t max_limit)
{
	if (*total_len >= max_limit) {
		*frag_len = max_limit;
		*total_len -= *frag_len;
	} else {
		*frag_len = *total_len;
		*total_len = 0;
	}
}

/**
 * DP_RX_MON_GET_NBUF_FROM_DESC() - Get nbuf from desc
 */
#define DP_RX_MON_GET_NBUF_FROM_DESC(rx_desc) \
	NULL

/**
 * dp_rx_mon_add_msdu_to_list_failure_handler() - Handler for nbuf buffer
 *                                                  attach failure
 *
 * @rx_tlv_hdr: rx_tlv_hdr
 * @pdev: struct dp_pdev *
 * @last: skb pointing to last skb in chained list at any moment
 * @head_msdu: parent skb in the chained list
 * @tail_msdu: Last skb in the chained list
 * @func_name: caller function name
 *
 * Return: void
 */
static inline void
dp_rx_mon_add_msdu_to_list_failure_handler(void *rx_tlv_hdr,
					   struct dp_pdev *pdev,
					   qdf_nbuf_t *last,
					   qdf_nbuf_t *head_msdu,
					   qdf_nbuf_t *tail_msdu,
					   const char *func_name)
{
	DP_STATS_INC(pdev, replenish.nbuf_alloc_fail, 1);
	qdf_frag_free(rx_tlv_hdr);
	if (head_msdu)
		qdf_nbuf_list_free(*head_msdu);
	dp_err("[%s] failed to allocate subsequent parent buffer to hold all frag\n",
	       func_name);
	if (head_msdu)
		*head_msdu = NULL;
	if (last)
		*last = NULL;
	if (tail_msdu)
		*tail_msdu = NULL;
}

/**
 * dp_rx_mon_get_paddr_from_desc() - Get paddr from desc
 */
static inline
qdf_dma_addr_t dp_rx_mon_get_paddr_from_desc(struct dp_rx_desc *rx_desc)
{
	return rx_desc->paddr_buf_start;
}

/**
 * DP_RX_MON_IS_BUFFER_ADDR_NULL() - Is Buffer received from hw is NULL
 */
#define DP_RX_MON_IS_BUFFER_ADDR_NULL(rx_desc) \
	(!(rx_desc->rx_buf_start))

#define DP_RX_MON_IS_MSDU_NOT_NULL(msdu) \
	true

/**
 * dp_rx_mon_buffer_free() - Free nbuf or frag memory
 * Free nbuf if feature is disabled, else free frag.
 *
 * @rx_desc: Rx desc
 */
static inline void
dp_rx_mon_buffer_free(struct dp_rx_desc *rx_desc)
{
	qdf_frag_free(rx_desc->rx_buf_start);
}

/**
 * dp_rx_mon_buffer_unmap() - Unmap nbuf or frag memory
 * Unmap nbuf if feature is disabled, else unmap frag.
 *
 * @soc: struct dp_soc *
 * @rx_desc: struct dp_rx_desc *
 * @size: Size to be unmapped
 */
static inline void
dp_rx_mon_buffer_unmap(struct dp_soc *soc, struct dp_rx_desc *rx_desc,
		       uint16_t size)
{
	qdf_mem_unmap_page(soc->osdev, rx_desc->paddr_buf_start,
			   size, QDF_DMA_FROM_DEVICE);
}

/**
 * dp_rx_mon_alloc_parent_buffer() - Allocate parent buffer to hold
 * radiotap header and accommodate all frag memory in nr_frag.
 *
 * @head_msdu: Ptr to hold allocated Msdu
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS dp_rx_mon_alloc_parent_buffer(qdf_nbuf_t *head_msdu)
{
	/*
	 * Headroom should accommodate radiotap header
	 * and protocol and flow tag for all frag
	 * Length reserved to accommodate Radiotap header
	 * is 128 bytes and length reserved for Protocol
	 * flow tag will vary based on QDF_NBUF_MAX_FRAGS.
	 */
	/*  -------------------------------------------------
	 * |       Protocol & Flow TAG      | Radiotap header|
	 * |                                |  Length(128 B) |
	 * |  ((4* QDF_NBUF_MAX_FRAGS) * 2) |                |
	 *  -------------------------------------------------
	 */

	*head_msdu = qdf_nbuf_alloc_no_recycler(DP_RX_MON_MAX_MONITOR_HEADER,
						DP_RX_MON_MAX_MONITOR_HEADER, 4);

	if (!(*head_msdu))
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(qdf_nbuf_head(*head_msdu), qdf_nbuf_headroom(*head_msdu));

	/* Set *head_msdu->next as NULL as all msdus are
	 * mapped via nr frags
	 */
	qdf_nbuf_set_next(*head_msdu, NULL);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_mon_parse_desc_buffer() - Parse desc buffer based.
 *
 * Below code will parse desc buffer, handle continuation frame,
 * adjust frag length and update l2_hdr_padding
 *
 * @soc                : struct dp_soc*
 * @msdu_info          : struct hal_rx_msdu_desc_info*
 * @is_frag_p          : is_frag *
 * @total_frag_len_p   : Remaining frag len to be updated
 * @frag_len_p         : frag len
 * @l2_hdr_offset_p    : l2 hdr offset
 * @rx_desc_tlv        : rx_desc_tlv
 * @is_frag_non_raw_p  : Non raw frag
 * @data               : NBUF Data
 */
static inline void
dp_rx_mon_parse_desc_buffer(struct dp_soc *dp_soc,
			    struct hal_rx_msdu_desc_info *msdu_info,
			    bool *is_frag_p, uint32_t *total_frag_len_p,
			    uint32_t *frag_len_p, uint16_t *l2_hdr_offset_p,
			    qdf_frag_t rx_desc_tlv,
			    bool *is_frag_non_raw_p, void *data)
{
	struct hal_rx_mon_dest_buf_info frame_info;
	uint16_t tot_payload_len =
			RX_MONITOR_BUFFER_SIZE - dp_soc->rx_pkt_tlv_size;

	if (msdu_info->msdu_flags & HAL_MSDU_F_MSDU_CONTINUATION) {
		/* First buffer of MSDU */
		if (!(*is_frag_p)) {
			/* Set total frag_len from msdu_len */
			*total_frag_len_p = msdu_info->msdu_len;

			*is_frag_p = true;
			if (HAL_HW_RX_DECAP_FORMAT_RAW ==
			    hal_rx_tlv_decap_format_get(dp_soc->hal_soc,
							rx_desc_tlv)) {
				*l2_hdr_offset_p =
					DP_RX_MON_RAW_L2_HDR_PAD_BYTE;
				frame_info.is_decap_raw = 1;
			} else {
				*l2_hdr_offset_p =
					DP_RX_MON_NONRAW_L2_HDR_PAD_BYTE;
				frame_info.is_decap_raw = 0;
				*is_frag_non_raw_p = true;
			}
			dp_rx_mon_frag_adjust_frag_len(total_frag_len_p,
						       frag_len_p,
						       tot_payload_len -
						       *l2_hdr_offset_p);

			frame_info.first_buffer = 1;
			frame_info.last_buffer = 0;
			hal_rx_priv_info_set_in_tlv(dp_soc->hal_soc,
						    rx_desc_tlv,
						    (uint8_t *)&frame_info,
						    sizeof(frame_info));
		} else {
			/*
			 * Continuation Middle frame
			 * Here max limit will be same for Raw and Non raw case.
			 */
			*l2_hdr_offset_p = DP_RX_MON_RAW_L2_HDR_PAD_BYTE;
			dp_rx_mon_frag_adjust_frag_len(total_frag_len_p,
						       frag_len_p,
						       tot_payload_len);

			/* Update frame info if is non raw frame */
			if (*is_frag_non_raw_p)
				frame_info.is_decap_raw = 0;
			else
				frame_info.is_decap_raw = 1;

			frame_info.first_buffer = 0;
			frame_info.last_buffer = 0;
			hal_rx_priv_info_set_in_tlv(dp_soc->hal_soc,
						    rx_desc_tlv,
						    (uint8_t *)&frame_info,
						    sizeof(frame_info));
		}
	} else {
		/**
		 * Last buffer of MSDU spread among multiple buffer
		 * Here max limit will be same for Raw and Non raw case.
		 */
		if (*is_frag_p) {
			*l2_hdr_offset_p = DP_RX_MON_RAW_L2_HDR_PAD_BYTE;

			dp_rx_mon_frag_adjust_frag_len(total_frag_len_p,
						       frag_len_p,
						       tot_payload_len);

			/* Update frame info if is non raw frame */
			if (*is_frag_non_raw_p)
				frame_info.is_decap_raw = 0;
			else
				frame_info.is_decap_raw = 1;

			frame_info.first_buffer = 0;
			frame_info.last_buffer = 1;
			hal_rx_priv_info_set_in_tlv(dp_soc->hal_soc,
						    rx_desc_tlv,
						    (uint8_t *)&frame_info,
						    sizeof(frame_info));
		} else {
			/* MSDU with single buffer */
			*frag_len_p = msdu_info->msdu_len;
			if (HAL_HW_RX_DECAP_FORMAT_RAW ==
			    hal_rx_tlv_decap_format_get(dp_soc->hal_soc,
							rx_desc_tlv)) {
				*l2_hdr_offset_p =
					DP_RX_MON_RAW_L2_HDR_PAD_BYTE;
				frame_info.is_decap_raw = 1;
			} else {
				*l2_hdr_offset_p =
					DP_RX_MON_NONRAW_L2_HDR_PAD_BYTE;
				frame_info.is_decap_raw = 0;
			}

			frame_info.first_buffer = 1;
			frame_info.last_buffer = 1;
			hal_rx_priv_info_set_in_tlv(dp_soc->hal_soc,
						    rx_desc_tlv,
						    (uint8_t *)&frame_info,
						    sizeof(frame_info));
		}
		/* Reset bool after complete processing of MSDU */
		*is_frag_p = false;
		*is_frag_non_raw_p = false;
	}
}

/**
 * dp_rx_mon_buffer_set_pktlen() - set pktlen for buffer
 */
static inline void dp_rx_mon_buffer_set_pktlen(qdf_nbuf_t msdu, uint32_t size)
{
}

/**
 * dp_rx_mon_add_msdu_to_list()- Add msdu to list and update head_msdu
 *      It will add reaped buffer frag to nr frag of parent msdu.
 * @soc: DP soc handle
 * @head_msdu: NULL if first time called else &msdu
 * @msdu: Msdu where frag address needs to be added via nr_frag
 * @last: Used to traverse in list if this feature is disabled.
 * @rx_desc_tlv: Frag address
 * @frag_len: Frag len
 * @l2_hdr_offset: l2 hdr padding
 */
static inline
QDF_STATUS dp_rx_mon_add_msdu_to_list(struct dp_soc *soc, qdf_nbuf_t *head_msdu,
				      qdf_nbuf_t msdu, qdf_nbuf_t *last,
				      qdf_frag_t rx_desc_tlv, uint32_t frag_len,
				      uint32_t l2_hdr_offset)
{
	uint32_t num_frags;
	qdf_nbuf_t msdu_curr;

	/* Here head_msdu and *head_msdu must not be NULL */
	/* Dont add frag to skb if frag length is zero. Drop frame */
	if (qdf_unlikely(!frag_len || !head_msdu || !(*head_msdu))) {
		dp_err("[%s] frag_len[%d] || head_msdu[%pK] || *head_msdu is Null while adding frag to skb\n",
		       __func__, frag_len, head_msdu);
		return QDF_STATUS_E_FAILURE;
	}

	/* In case of first desc of MPDU, assign curr msdu to *head_msdu */
	if (!qdf_nbuf_get_nr_frags(*head_msdu))
		msdu_curr = *head_msdu;
	else
		msdu_curr = *last;

	/* Current msdu must not be NULL */
	if (qdf_unlikely(!msdu_curr)) {
		dp_err("[%s] Current msdu can't be Null while adding frag to skb\n",
		       __func__);
		return QDF_STATUS_E_FAILURE;
	}

	num_frags = qdf_nbuf_get_nr_frags(msdu_curr);
	if (num_frags < QDF_NBUF_MAX_FRAGS) {
		qdf_nbuf_add_rx_frag(rx_desc_tlv, msdu_curr,
				     soc->rx_mon_pkt_tlv_size,
				     frag_len + l2_hdr_offset,
				     RX_MONITOR_BUFFER_SIZE,
				     false);
		if (*last != msdu_curr)
			*last = msdu_curr;
		return QDF_STATUS_SUCCESS;
	}

	/* Execution will reach here only if num_frags == QDF_NBUF_MAX_FRAGS */
	msdu_curr = NULL;
	if ((dp_rx_mon_alloc_parent_buffer(&msdu_curr))
	    != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	qdf_nbuf_add_rx_frag(rx_desc_tlv, msdu_curr, soc->rx_mon_pkt_tlv_size,
			     frag_len + l2_hdr_offset, RX_MONITOR_BUFFER_SIZE,
			     false);

	/* Add allocated nbuf in the chain */
	qdf_nbuf_set_next(*last, msdu_curr);

	/* Assign current msdu to last to avoid traversal */
	*last = msdu_curr;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_mon_init_tail_msdu() - Initialize tail msdu
 *
 * @head_msdu: Parent buffer to hold MPDU data
 * @msdu: Msdu to be updated in tail_msdu
 * @last: last msdu
 * @tail_msdu: Last msdu
 */
static inline
void dp_rx_mon_init_tail_msdu(qdf_nbuf_t *head_msdu, qdf_nbuf_t msdu,
			      qdf_nbuf_t last, qdf_nbuf_t *tail_msdu)
{
	if (!head_msdu || !(*head_msdu)) {
		*tail_msdu = NULL;
		return;
	}

	if (last)
		qdf_nbuf_set_next(last, NULL);
	*tail_msdu = last;
}

/**
 * dp_rx_mon_remove_raw_frame_fcs_len() - Remove FCS length for Raw Frame
 *
 * If feature is disabled, then removal happens in restitch logic.
 *
 * @soc: Datapath soc handle
 * @head_msdu: Head msdu
 * @tail_msdu: Tail msdu
 */
static inline
void dp_rx_mon_remove_raw_frame_fcs_len(struct dp_soc *soc,
					qdf_nbuf_t *head_msdu,
					qdf_nbuf_t *tail_msdu)
{
	qdf_frag_t addr;

	if (qdf_unlikely(!head_msdu || !tail_msdu || !(*head_msdu)))
		return;

	/* If *head_msdu is valid, then *tail_msdu must be valid */
	/* If head_msdu is valid, then it must have nr_frags */
	/* If tail_msdu is valid, then it must have nr_frags */

	/* Strip FCS_LEN for Raw frame */
	addr = qdf_nbuf_get_frag_addr(*head_msdu, 0);
	addr -= soc->rx_mon_pkt_tlv_size;
	if (hal_rx_tlv_decap_format_get(soc->hal_soc, addr) ==
		HAL_HW_RX_DECAP_FORMAT_RAW) {
		qdf_nbuf_trim_add_frag_size(*tail_msdu,
			qdf_nbuf_get_nr_frags(*tail_msdu) - 1,
					-HAL_RX_FCS_LEN, 0);
	}
}

/**
 * dp_rx_mon_get_buffer_data()- Get data from desc buffer
 * @rx_desc: desc
 *
 * Return address containing actual tlv content
 */
static inline
uint8_t *dp_rx_mon_get_buffer_data(struct dp_rx_desc *rx_desc)
{
	return rx_desc->rx_buf_start;
}

#else

#define DP_RX_MON_GET_NBUF_FROM_DESC(rx_desc) \
	(rx_desc->nbuf)

static inline void
dp_rx_mon_add_msdu_to_list_failure_handler(void *rx_tlv_hdr,
					   struct dp_pdev *pdev,
					   qdf_nbuf_t *last,
					   qdf_nbuf_t *head_msdu,
					   qdf_nbuf_t *tail_msdu,
					   const char *func_name)
{
}

static inline
qdf_dma_addr_t dp_rx_mon_get_paddr_from_desc(struct dp_rx_desc *rx_desc)
{
	qdf_dma_addr_t paddr = 0;
	qdf_nbuf_t msdu = NULL;

	msdu = rx_desc->nbuf;
	if (msdu)
		paddr = qdf_nbuf_get_frag_paddr(msdu, 0);

	return paddr;
}

#define DP_RX_MON_IS_BUFFER_ADDR_NULL(rx_desc) \
	(!(rx_desc->nbuf))

#define DP_RX_MON_IS_MSDU_NOT_NULL(msdu) \
	(msdu)

static inline void
dp_rx_mon_buffer_free(struct dp_rx_desc *rx_desc)
{
	qdf_nbuf_free(rx_desc->nbuf);
}

static inline void
dp_rx_mon_buffer_unmap(struct dp_soc *soc, struct dp_rx_desc *rx_desc,
		       uint16_t size)
{
	qdf_nbuf_unmap_nbytes_single(soc->osdev, rx_desc->nbuf,
				     QDF_DMA_FROM_DEVICE, size);
}

static inline
QDF_STATUS dp_rx_mon_alloc_parent_buffer(qdf_nbuf_t *head_msdu)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
dp_rx_mon_parse_desc_buffer(struct dp_soc *dp_soc,
			    struct hal_rx_msdu_desc_info *msdu_info,
			    bool *is_frag_p, uint32_t *total_frag_len_p,
			    uint32_t *frag_len_p, uint16_t *l2_hdr_offset_p,
			    qdf_frag_t rx_desc_tlv,
			    bool *is_frag_non_raw_p, void *data)
{
	/*
	 * HW structures call this L3 header padding
	 * -- even though this is actually the offset
	 * from the buffer beginning where the L2
	 * header begins.
	 */
	*l2_hdr_offset_p =
	hal_rx_msdu_end_l3_hdr_padding_get(dp_soc->hal_soc, data);

	if (msdu_info->msdu_flags & HAL_MSDU_F_MSDU_CONTINUATION) {
		if (!*(is_frag_p)) {
			*total_frag_len_p = msdu_info->msdu_len;
			*is_frag_p = true;
		}
		dp_mon_adjust_frag_len(dp_soc, total_frag_len_p, frag_len_p,
				       *l2_hdr_offset_p);
	} else {
		if (*is_frag_p) {
			dp_mon_adjust_frag_len(dp_soc, total_frag_len_p,
					       frag_len_p,
					       *l2_hdr_offset_p);
		} else {
			*frag_len_p = msdu_info->msdu_len;
		}
		*is_frag_p = false;
	}
}

static inline void dp_rx_mon_buffer_set_pktlen(qdf_nbuf_t msdu, uint32_t size)
{
	qdf_nbuf_set_pktlen(msdu, size);
}

static inline
QDF_STATUS dp_rx_mon_add_msdu_to_list(struct dp_soc *soc, qdf_nbuf_t *head_msdu,
				      qdf_nbuf_t msdu, qdf_nbuf_t *last,
				      qdf_frag_t rx_desc_tlv, uint32_t frag_len,
				      uint32_t l2_hdr_offset)
{
	if (head_msdu && !*head_msdu) {
		*head_msdu = msdu;
	} else {
		if (*last)
			qdf_nbuf_set_next(*last, msdu);
	}
	*last = msdu;
	return QDF_STATUS_SUCCESS;
}

static inline
void dp_rx_mon_init_tail_msdu(qdf_nbuf_t *head_msdu, qdf_nbuf_t msdu,
			      qdf_nbuf_t last, qdf_nbuf_t *tail_msdu)
{
	if (last)
		qdf_nbuf_set_next(last, NULL);

	*tail_msdu = msdu;
}

static inline
void dp_rx_mon_remove_raw_frame_fcs_len(struct dp_soc *soc,
					qdf_nbuf_t *head_msdu,
					qdf_nbuf_t *tail_msdu)
{
}

static inline
uint8_t *dp_rx_mon_get_buffer_data(struct dp_rx_desc *rx_desc)
{
	qdf_nbuf_t msdu = NULL;
	uint8_t *data = NULL;

	msdu = rx_desc->nbuf;
	if (qdf_likely(msdu))
		data = qdf_nbuf_data(msdu);
	return data;
}

#endif

/**
 * dp_rx_cookie_2_mon_link_desc() - Retrieve Link descriptor based on target
 * @pdev: core physical device context
 * @hal_buf_info: structure holding the buffer info
 * mac_id: mac number
 *
 * Return: link descriptor address
 */
static inline
void *dp_rx_cookie_2_mon_link_desc(struct dp_pdev *pdev,
				   struct hal_buf_info buf_info,
				   uint8_t mac_id)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return dp_rx_cookie_2_mon_link_desc_va(pdev, &buf_info,
						       mac_id);

	return dp_rx_cookie_2_link_desc_va(pdev->soc, &buf_info);
}

/**
 * dp_rx_monitor_link_desc_return() - Return Link descriptor based on target
 * @pdev: core physical device context
 * @p_last_buf_addr_info: MPDU Link descriptor
 * mac_id: mac number
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS dp_rx_monitor_link_desc_return(struct dp_pdev *pdev,
					  hal_buff_addrinfo_t
					  p_last_buf_addr_info,
					  uint8_t mac_id, uint8_t bm_action)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return dp_rx_mon_link_desc_return(pdev, p_last_buf_addr_info,
						  mac_id);

	return dp_rx_link_desc_return_by_addr(pdev->soc, p_last_buf_addr_info,
				      bm_action);
}

/**
 * dp_rxdma_get_mon_dst_ring() - Return the pointer to rxdma_err_dst_ring
 *					or mon_dst_ring based on the target
 * @pdev: core physical device context
 * @mac_for_pdev: mac_id number
 *
 * Return: ring address
 */
static inline
void *dp_rxdma_get_mon_dst_ring(struct dp_pdev *pdev,
				uint8_t mac_for_pdev)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return pdev->soc->rxdma_mon_dst_ring[mac_for_pdev].hal_srng;

	return pdev->soc->rxdma_err_dst_ring[mac_for_pdev].hal_srng;
}

/**
 * dp_rxdma_get_mon_buf_ring() - Return monitor buf ring address
 *				    based on target
 * @pdev: core physical device context
 * @mac_for_pdev: mac id number
 *
 * Return: ring address
 */
static inline
struct dp_srng *dp_rxdma_get_mon_buf_ring(struct dp_pdev *pdev,
					  uint8_t mac_for_pdev)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return &pdev->soc->rxdma_mon_buf_ring[mac_for_pdev];

	/* For MCL there is only 1 rx refill ring */
	return &pdev->soc->rx_refill_buf_ring[0];
}

/**
 * dp_rx_get_mon_desc() - Return Rx descriptor based on target
 * @soc: soc handle
 * @cookie: cookie value
 *
 * Return: Rx descriptor
 */
static inline
struct dp_rx_desc *dp_rx_get_mon_desc(struct dp_soc *soc,
				      uint32_t cookie)
{
	if (soc->wlan_cfg_ctx->rxdma1_enable)
		return dp_rx_cookie_2_va_mon_buf(soc, cookie);

	return dp_rx_cookie_2_va_rxdma_buf(soc, cookie);
}

#ifdef QCA_MONITOR_PKT_SUPPORT
/*
 * dp_mon_htt_dest_srng_setup(): monitor dest srng setup
 * @soc: DP SOC handle
 * @pdev: DP PDEV handle
 * @mac_id: MAC ID
 * @mac_for_pdev: PDEV mac
 *
 * Return: status: QDF_STATUS_SUCCESS - Success, non-zero: Failure
 */
QDF_STATUS dp_mon_htt_dest_srng_setup(struct dp_soc *soc,
				      struct dp_pdev *pdev,
				      int mac_id,
				      int mac_for_pdev);

/*
 * dp_mon_dest_rings_deinit(): deinit monitor dest rings
 * @pdev: DP PDEV handle
 * @lmac_id: MAC ID
 *
 * Return: status: None
 */
void dp_mon_dest_rings_deinit(struct dp_pdev *pdev, int lmac_id);

/*
 * dp_mon_dest_rings_free(): free monitor dest rings
 * @pdev: DP PDEV handle
 * @lmac_id: MAC ID
 *
 * Return: status: None
 */
void dp_mon_dest_rings_free(struct dp_pdev *pdev, int lmac_id);

/*
 * dp_mon_dest_rings_init(): init monitor dest rings
 * @pdev: DP PDEV handle
 * @lmac_id: MAC ID
 *
 * Return: status: QDF_STATUS_SUCCESS - Success, non-zero: Failure
 */
QDF_STATUS dp_mon_dest_rings_init(struct dp_pdev *pdev, int lmac_id);

/*
 * dp_mon_dest_rings_allocate(): allocate monitor dest rings
 * @pdev: DP PDEV handle
 * @lmac_id: MAC ID
 *
 * Return: status: QDF_STATUS_SUCCESS - Success, non-zero: Failure
 */
QDF_STATUS dp_mon_dest_rings_alloc(struct dp_pdev *pdev, int lmac_id);

#else
QDF_STATUS dp_mon_htt_dest_srng_setup(struct dp_soc *soc,
				      struct dp_pdev *pdev,
				      int mac_id,
				      int mac_for_pdev)
{
	return QDF_STATUS_SUCCESS;
}

static void dp_mon_dest_rings_deinit(struct dp_pdev *pdev, int lmac_id)
{
}

static void dp_mon_dest_rings_free(struct dp_pdev *pdev, int lmac_id)
{
}

static
QDF_STATUS dp_mon_dest_rings_init(struct dp_pdev *pdev, int lmac_id)
{
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_dest_rings_alloc(struct dp_pdev *pdev, int lmac_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_MONITOR_PKT_SUPPORT */

#endif /* _DP_RX_MON_1_0_H_ */
