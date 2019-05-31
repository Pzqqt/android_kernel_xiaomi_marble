/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RX_H
#define _DP_RX_H

#include "hal_rx.h"
#include "dp_tx.h"
#include "dp_peer.h"
#include "dp_internal.h"

#ifdef RXDMA_OPTIMIZATION
#ifdef NO_RX_PKT_HDR_TLV
#define RX_BUFFER_ALIGNMENT     0
#else
#define RX_BUFFER_ALIGNMENT     128
#endif /* NO_RX_PKT_HDR_TLV */
#else /* RXDMA_OPTIMIZATION */
#define RX_BUFFER_ALIGNMENT     4
#endif /* RXDMA_OPTIMIZATION */

#ifdef QCA_HOST2FW_RXBUF_RING
#define DP_WBM2SW_RBM HAL_RX_BUF_RBM_SW1_BM

/**
 * For MCL cases, allocate as many RX descriptors as buffers in the SW2RXDMA
 * ring. This value may need to be tuned later.
 */
#define DP_RX_DESC_ALLOC_MULTIPLIER 1
#else
#define DP_WBM2SW_RBM HAL_RX_BUF_RBM_SW3_BM

/**
 * AP use cases need to allocate more RX Descriptors than the number of
 * entries avaialable in the SW2RXDMA buffer replenish ring. This is to account
 * for frames sitting in REO queues, HW-HW DMA rings etc. Hence using a
 * multiplication factor of 3, to allocate three times as many RX descriptors
 * as RX buffers.
 */
#define DP_RX_DESC_ALLOC_MULTIPLIER 3
#endif /* QCA_HOST2FW_RXBUF_RING */

#define RX_BUFFER_RESERVATION   0

#define DP_PEER_METADATA_PEER_ID_MASK	0x0000ffff
#define DP_PEER_METADATA_PEER_ID_SHIFT	0
#define DP_PEER_METADATA_VDEV_ID_MASK	0x00070000
#define DP_PEER_METADATA_VDEV_ID_SHIFT	16

#define DP_PEER_METADATA_PEER_ID_GET(_peer_metadata)		\
	(((_peer_metadata) & DP_PEER_METADATA_PEER_ID_MASK)	\
			>> DP_PEER_METADATA_PEER_ID_SHIFT)

#define DP_PEER_METADATA_ID_GET(_peer_metadata)			\
	(((_peer_metadata) & DP_PEER_METADATA_VDEV_ID_MASK)	\
			>> DP_PEER_METADATA_VDEV_ID_SHIFT)

#define DP_RX_DESC_MAGIC 0xdec0de

/**
 * struct dp_rx_desc
 *
 * @nbuf		: VA of the "skb" posted
 * @rx_buf_start	: VA of the original Rx buffer, before
 *			  movement of any skb->data pointer
 * @cookie		: index into the sw array which holds
 *			  the sw Rx descriptors
 *			  Cookie space is 21 bits:
 *			  lower 18 bits -- index
 *			  upper  3 bits -- pool_id
 * @pool_id		: pool Id for which this allocated.
 *			  Can only be used if there is no flow
 *			  steering
 * @in_use		  rx_desc is in use
 * @unmapped		  used to mark rx_desc an unmapped if the corresponding
 *			  nbuf is already unmapped
 */
struct dp_rx_desc {
	qdf_nbuf_t nbuf;
	uint8_t *rx_buf_start;
	uint32_t cookie;
	uint8_t	 pool_id;
#ifdef RX_DESC_DEBUG_CHECK
	uint32_t magic;
#endif
	uint8_t	in_use:1,
	unmapped:1;
};

/* RX Descriptor Multi Page memory alloc related */
#define DP_RX_DESC_OFFSET_NUM_BITS 8
#define DP_RX_DESC_PAGE_ID_NUM_BITS 8
#define DP_RX_DESC_POOL_ID_NUM_BITS 4

#define DP_RX_DESC_PAGE_ID_SHIFT DP_RX_DESC_OFFSET_NUM_BITS
#define DP_RX_DESC_POOL_ID_SHIFT \
		(DP_RX_DESC_OFFSET_NUM_BITS + DP_RX_DESC_PAGE_ID_NUM_BITS)
#define RX_DESC_MULTI_PAGE_COOKIE_POOL_ID_MASK \
	(((1 << DP_RX_DESC_POOL_ID_NUM_BITS) - 1) << DP_RX_DESC_POOL_ID_SHIFT)
#define RX_DESC_MULTI_PAGE_COOKIE_PAGE_ID_MASK	\
			(((1 << DP_RX_DESC_PAGE_ID_NUM_BITS) - 1) << \
			 DP_RX_DESC_PAGE_ID_SHIFT)
#define RX_DESC_MULTI_PAGE_COOKIE_OFFSET_MASK \
			((1 << DP_RX_DESC_OFFSET_NUM_BITS) - 1)
#define DP_RX_DESC_MULTI_PAGE_COOKIE_GET_POOL_ID(_cookie)		\
	(((_cookie) & RX_DESC_MULTI_PAGE_COOKIE_POOL_ID_MASK) >>	\
			DP_RX_DESC_POOL_ID_SHIFT)
#define DP_RX_DESC_MULTI_PAGE_COOKIE_GET_PAGE_ID(_cookie)		\
	(((_cookie) & RX_DESC_MULTI_PAGE_COOKIE_PAGE_ID_MASK) >>	\
			DP_RX_DESC_PAGE_ID_SHIFT)
#define DP_RX_DESC_MULTI_PAGE_COOKIE_GET_OFFSET(_cookie)		\
	((_cookie) & RX_DESC_MULTI_PAGE_COOKIE_OFFSET_MASK)

#define RX_DESC_COOKIE_INDEX_SHIFT		0
#define RX_DESC_COOKIE_INDEX_MASK		0x3ffff /* 18 bits */
#define RX_DESC_COOKIE_POOL_ID_SHIFT		18
#define RX_DESC_COOKIE_POOL_ID_MASK		0x1c0000

#define DP_RX_DESC_COOKIE_MAX	\
	(RX_DESC_COOKIE_INDEX_MASK | RX_DESC_COOKIE_POOL_ID_MASK)

#define DP_RX_DESC_COOKIE_POOL_ID_GET(_cookie)		\
	(((_cookie) & RX_DESC_COOKIE_POOL_ID_MASK) >>	\
			RX_DESC_COOKIE_POOL_ID_SHIFT)

#define DP_RX_DESC_COOKIE_INDEX_GET(_cookie)		\
	(((_cookie) & RX_DESC_COOKIE_INDEX_MASK) >>	\
			RX_DESC_COOKIE_INDEX_SHIFT)

/* DOC: Offset to obtain LLC hdr
 *
 * In the case of Wifi parse error
 * to reach LLC header from beginning
 * of VLAN tag we need to skip 8 bytes.
 * Vlan_tag(4)+length(2)+length added
 * by HW(2) = 8 bytes.
 */
#define DP_SKIP_VLAN		8

/**
 * struct dp_rx_cached_buf - rx cached buffer
 * @list: linked list node
 * @buf: skb buffer
 */
struct dp_rx_cached_buf {
	qdf_list_node_t node;
	qdf_nbuf_t buf;
};

/*
 *dp_rx_xor_block() - xor block of data
 *@b: destination data block
 *@a: source data block
 *@len: length of the data to process
 *
 *Returns: None
 */
static inline void dp_rx_xor_block(uint8_t *b, const uint8_t *a, qdf_size_t len)
{
	qdf_size_t i;

	for (i = 0; i < len; i++)
		b[i] ^= a[i];
}

/*
 *dp_rx_rotl() - rotate the bits left
 *@val: unsigned integer input value
 *@bits: number of bits
 *
 *Returns: Integer with left rotated by number of 'bits'
 */
static inline uint32_t dp_rx_rotl(uint32_t val, int bits)
{
	return (val << bits) | (val >> (32 - bits));
}

/*
 *dp_rx_rotr() - rotate the bits right
 *@val: unsigned integer input value
 *@bits: number of bits
 *
 *Returns: Integer with right rotated by number of 'bits'
 */
static inline uint32_t dp_rx_rotr(uint32_t val, int bits)
{
	return (val >> bits) | (val << (32 - bits));
}

/*
 * dp_set_rx_queue() - set queue_mapping in skb
 * @nbuf: skb
 * @queue_id: rx queue_id
 *
 * Return: void
 */
#ifdef QCA_OL_RX_MULTIQ_SUPPORT
static inline void dp_set_rx_queue(qdf_nbuf_t nbuf, uint8_t queue_id)
{
	qdf_nbuf_record_rx_queue(nbuf, queue_id);
	return;
}
#else
static inline void dp_set_rx_queue(qdf_nbuf_t nbuf, uint8_t queue_id)
{
}
#endif

/*
 *dp_rx_xswap() - swap the bits left
 *@val: unsigned integer input value
 *
 *Returns: Integer with bits swapped
 */
static inline uint32_t dp_rx_xswap(uint32_t val)
{
	return ((val & 0x00ff00ff) << 8) | ((val & 0xff00ff00) >> 8);
}

/*
 *dp_rx_get_le32_split() - get little endian 32 bits split
 *@b0: byte 0
 *@b1: byte 1
 *@b2: byte 2
 *@b3: byte 3
 *
 *Returns: Integer with split little endian 32 bits
 */
static inline uint32_t dp_rx_get_le32_split(uint8_t b0, uint8_t b1, uint8_t b2,
					uint8_t b3)
{
	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

/*
 *dp_rx_get_le32() - get little endian 32 bits
 *@b0: byte 0
 *@b1: byte 1
 *@b2: byte 2
 *@b3: byte 3
 *
 *Returns: Integer with little endian 32 bits
 */
static inline uint32_t dp_rx_get_le32(const uint8_t *p)
{
	return dp_rx_get_le32_split(p[0], p[1], p[2], p[3]);
}

/*
 * dp_rx_put_le32() - put little endian 32 bits
 * @p: destination char array
 * @v: source 32-bit integer
 *
 * Returns: None
 */
static inline void dp_rx_put_le32(uint8_t *p, uint32_t v)
{
	p[0] = (v) & 0xff;
	p[1] = (v >> 8) & 0xff;
	p[2] = (v >> 16) & 0xff;
	p[3] = (v >> 24) & 0xff;
}

/* Extract michal mic block of data */
#define dp_rx_michael_block(l, r)	\
	do {					\
		r ^= dp_rx_rotl(l, 17);	\
		l += r;				\
		r ^= dp_rx_xswap(l);		\
		l += r;				\
		r ^= dp_rx_rotl(l, 3);	\
		l += r;				\
		r ^= dp_rx_rotr(l, 2);	\
		l += r;				\
	} while (0)

/**
 * struct dp_rx_desc_list_elem_t
 *
 * @next		: Next pointer to form free list
 * @rx_desc		: DP Rx descriptor
 */
union dp_rx_desc_list_elem_t {
	union dp_rx_desc_list_elem_t *next;
	struct dp_rx_desc rx_desc;
};

#ifdef RX_DESC_MULTI_PAGE_ALLOC
/**
 * dp_rx_desc_find() - find dp rx descriptor from page ID and offset
 * @page_id: Page ID
 * @offset: Offset of the descriptor element
 *
 * Return: RX descriptor element
 */
union dp_rx_desc_list_elem_t *dp_rx_desc_find(uint16_t page_id, uint16_t offset,
					      struct rx_desc_pool *rx_pool);

static inline
struct dp_rx_desc *dp_get_rx_desc_from_cookie(struct dp_soc *soc,
					      struct rx_desc_pool *pool,
					      uint32_t cookie)
{
	uint8_t pool_id = DP_RX_DESC_MULTI_PAGE_COOKIE_GET_POOL_ID(cookie);
	uint16_t page_id = DP_RX_DESC_MULTI_PAGE_COOKIE_GET_PAGE_ID(cookie);
	uint8_t offset = DP_RX_DESC_MULTI_PAGE_COOKIE_GET_OFFSET(cookie);
	struct rx_desc_pool *rx_desc_pool;
	union dp_rx_desc_list_elem_t *rx_desc_elem;

	if (qdf_unlikely(pool_id >= MAX_RXDESC_POOLS))
		return NULL;

	rx_desc_pool = &pool[pool_id];
	rx_desc_elem = (union dp_rx_desc_list_elem_t *)
		(rx_desc_pool->desc_pages.cacheable_pages[page_id] +
		rx_desc_pool->elem_size * offset);

	return &rx_desc_elem->rx_desc;
}

/**
 * dp_rx_cookie_2_va_rxdma_buf() - Converts cookie to a virtual address of
 *			 the Rx descriptor on Rx DMA source ring buffer
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address
 *
 * Return: Pointer to the Rx descriptor
 */
static inline
struct dp_rx_desc *dp_rx_cookie_2_va_rxdma_buf(struct dp_soc *soc,
					       uint32_t cookie)
{
	return dp_get_rx_desc_from_cookie(soc, &soc->rx_desc_buf[0], cookie);
}

/**
 * dp_rx_cookie_2_va_mon_buf() - Converts cookie to a virtual address of
 *			 the Rx descriptor on monitor ring buffer
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address
 *
 * Return: Pointer to the Rx descriptor
 */
static inline
struct dp_rx_desc *dp_rx_cookie_2_va_mon_buf(struct dp_soc *soc,
					     uint32_t cookie)
{
	return dp_get_rx_desc_from_cookie(soc, &soc->rx_desc_mon[0], cookie);
}

/**
 * dp_rx_cookie_2_va_mon_status() - Converts cookie to a virtual address of
 *			 the Rx descriptor on monitor status ring buffer
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address
 *
 * Return: Pointer to the Rx descriptor
 */
static inline
struct dp_rx_desc *dp_rx_cookie_2_va_mon_status(struct dp_soc *soc,
						uint32_t cookie)
{
	return dp_get_rx_desc_from_cookie(soc, &soc->rx_desc_status[0], cookie);
}
#else
/**
 * dp_rx_cookie_2_va_rxdma_buf() - Converts cookie to a virtual address of
 *			 the Rx descriptor on Rx DMA source ring buffer
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_va_rxdma_buf(struct dp_soc *soc, uint32_t cookie)
{
	uint8_t pool_id = DP_RX_DESC_COOKIE_POOL_ID_GET(cookie);
	uint16_t index = DP_RX_DESC_COOKIE_INDEX_GET(cookie);
	struct rx_desc_pool *rx_desc_pool;

	if (qdf_unlikely(pool_id >= MAX_RXDESC_POOLS))
		return NULL;

	rx_desc_pool = &soc->rx_desc_buf[pool_id];

	if (qdf_unlikely(index >= rx_desc_pool->pool_size))
		return NULL;

	return &(soc->rx_desc_buf[pool_id].array[index].rx_desc);
}

/**
 * dp_rx_cookie_2_va_mon_buf() - Converts cookie to a virtual address of
 *			 the Rx descriptor on monitor ring buffer
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_va_mon_buf(struct dp_soc *soc, uint32_t cookie)
{
	uint8_t pool_id = DP_RX_DESC_COOKIE_POOL_ID_GET(cookie);
	uint16_t index = DP_RX_DESC_COOKIE_INDEX_GET(cookie);
	/* TODO */
	/* Add sanity for pool_id & index */
	return &(soc->rx_desc_mon[pool_id].array[index].rx_desc);
}

/**
 * dp_rx_cookie_2_va_mon_status() - Converts cookie to a virtual address of
 *			 the Rx descriptor on monitor status ring buffer
 * @soc: core txrx main context
 * @cookie: cookie used to lookup virtual address
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_va_mon_status(struct dp_soc *soc, uint32_t cookie)
{
	uint8_t pool_id = DP_RX_DESC_COOKIE_POOL_ID_GET(cookie);
	uint16_t index = DP_RX_DESC_COOKIE_INDEX_GET(cookie);
	/* TODO */
	/* Add sanity for pool_id & index */
	return &(soc->rx_desc_status[pool_id].array[index].rx_desc);
}
#endif /* RX_DESC_MULTI_PAGE_ALLOC */

void dp_rx_add_desc_list_to_free_list(struct dp_soc *soc,
				union dp_rx_desc_list_elem_t **local_desc_list,
				union dp_rx_desc_list_elem_t **tail,
				uint16_t pool_id,
				struct rx_desc_pool *rx_desc_pool);

uint16_t dp_rx_get_free_desc_list(struct dp_soc *soc, uint32_t pool_id,
				struct rx_desc_pool *rx_desc_pool,
				uint16_t num_descs,
				union dp_rx_desc_list_elem_t **desc_list,
				union dp_rx_desc_list_elem_t **tail);


QDF_STATUS dp_rx_pdev_attach(struct dp_pdev *pdev);

void dp_rx_pdev_detach(struct dp_pdev *pdev);


uint32_t
dp_rx_process(struct dp_intr *int_ctx, void *hal_ring, uint8_t reo_ring_num,
	      uint32_t quota);

/**
 * dp_rx_err_process() - Processes error frames routed to REO error ring
 * @int_ctx: pointer to DP interrupt context
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements error processing and top level demultiplexer
 * for all the frames routed to REO error ring.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t dp_rx_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
			   void *hal_ring, uint32_t quota);

/**
 * dp_rx_wbm_err_process() - Processes error frames routed to WBM release ring
 * @int_ctx: pointer to DP interrupt context
 * @soc: core txrx main context
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements error processing and top level demultiplexer
 * for all the frames routed to WBM2HOST sw release ring.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t
dp_rx_wbm_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
		      void *hal_ring, uint32_t quota);

/**
 * dp_rx_sg_create() - create a frag_list for MSDUs which are spread across
 *		     multiple nbufs.
 * @nbuf: pointer to the first msdu of an amsdu.
 * @rx_tlv_hdr: pointer to the start of RX TLV headers.
 *
 * This function implements the creation of RX frag_list for cases
 * where an MSDU is spread across multiple nbufs.
 *
 * Return: returns the head nbuf which contains complete frag_list.
 */
qdf_nbuf_t dp_rx_sg_create(qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr);

/*
 * dp_rx_desc_pool_alloc() - create a pool of software rx_descs
 *			     at the time of dp rx initialization
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @pool_size: number of Rx descriptor in the pool
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: QDF status
 */
QDF_STATUS dp_rx_desc_pool_alloc(struct dp_soc *soc, uint32_t pool_id,
				 uint32_t pool_size, struct rx_desc_pool *pool);

/*
 * dp_rx_desc_nbuf_and_pool_free() - free the sw rx desc pool called during
 *				     de-initialization of wifi module.
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: None
 */
void dp_rx_desc_nbuf_and_pool_free(struct dp_soc *soc, uint32_t pool_id,
				   struct rx_desc_pool *rx_desc_pool);

/*
 * dp_rx_desc_nbuf_free() - free the sw rx desc nbufs called during
 *			    de-initialization of wifi module.
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: None
 */
void dp_rx_desc_nbuf_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool);

/*
 * dp_rx_desc_pool_free() - free the sw rx desc array called during
 *			    de-initialization of wifi module.
 *
 * @soc: core txrx main context
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: None
 */
void dp_rx_desc_pool_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool);

void dp_rx_deliver_raw(struct dp_vdev *vdev, qdf_nbuf_t nbuf_list,
				struct dp_peer *peer);

/**
 * dp_rx_add_to_free_desc_list() - Adds to a local free descriptor list
 *
 * @head: pointer to the head of local free list
 * @tail: pointer to the tail of local free list
 * @new: new descriptor that is added to the free list
 *
 * Return: void:
 */
static inline
void dp_rx_add_to_free_desc_list(union dp_rx_desc_list_elem_t **head,
				 union dp_rx_desc_list_elem_t **tail,
				 struct dp_rx_desc *new)
{
	qdf_assert(head && new);

	new->nbuf = NULL;
	new->in_use = 0;

	((union dp_rx_desc_list_elem_t *)new)->next = *head;
	*head = (union dp_rx_desc_list_elem_t *)new;
	if (!*tail)
		*tail = *head;

}

uint8_t dp_rx_process_invalid_peer(struct dp_soc *soc, qdf_nbuf_t nbuf,
				   uint8_t mac_id);
void dp_rx_process_invalid_peer_wrapper(struct dp_soc *soc,
		qdf_nbuf_t mpdu, bool mpdu_done, uint8_t mac_id);
void dp_rx_process_mic_error(struct dp_soc *soc, qdf_nbuf_t nbuf,
			     uint8_t *rx_tlv_hdr, struct dp_peer *peer);
void dp_2k_jump_handle(struct dp_soc *soc, qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr,
		       uint16_t peer_id, uint8_t tid);


#define DP_RX_LIST_APPEND(head, tail, elem) \
	do {                                                          \
		if (!(head)) {                                        \
			(head) = (elem);                              \
			QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(head) = 1;\
		} else {                                              \
			qdf_nbuf_set_next((tail), (elem));            \
			QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(head)++;  \
		}                                                     \
		(tail) = (elem);                                      \
		qdf_nbuf_set_next((tail), NULL);                      \
	} while (0)

#ifndef BUILD_X86
static inline int check_x86_paddr(struct dp_soc *dp_soc, qdf_nbuf_t *rx_netbuf,
		qdf_dma_addr_t *paddr, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#else
#define MAX_RETRY 100
static inline int check_x86_paddr(struct dp_soc *dp_soc, qdf_nbuf_t *rx_netbuf,
		qdf_dma_addr_t *paddr, struct dp_pdev *pdev)
{
	uint32_t nbuf_retry = 0;
	int32_t ret;
	const uint32_t x86_phy_addr = 0x50000000;
	/*
	 * in M2M emulation platforms (x86) the memory below 0x50000000
	 * is reserved for target use, so any memory allocated in this
	 * region should not be used by host
	 */
	do {
		if (qdf_likely(*paddr > x86_phy_addr))
			return QDF_STATUS_SUCCESS;
		else {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
					"phy addr %pK exceeded 0x50000000 trying again",
					paddr);

			nbuf_retry++;
			if ((*rx_netbuf)) {
				qdf_nbuf_unmap_single(dp_soc->osdev, *rx_netbuf,
						QDF_DMA_FROM_DEVICE);
				/* Not freeing buffer intentionally.
				 * Observed that same buffer is getting
				 * re-allocated resulting in longer load time
				 * WMI init timeout.
				 * This buffer is anyway not useful so skip it.
				 **/
			}

			*rx_netbuf = qdf_nbuf_alloc(dp_soc->osdev,
							RX_BUFFER_SIZE,
							RX_BUFFER_RESERVATION,
							RX_BUFFER_ALIGNMENT,
							FALSE);

			if (qdf_unlikely(!(*rx_netbuf)))
				return QDF_STATUS_E_FAILURE;

			ret = qdf_nbuf_map_single(dp_soc->osdev, *rx_netbuf,
							QDF_DMA_FROM_DEVICE);

			if (qdf_unlikely(ret == QDF_STATUS_E_FAILURE)) {
				qdf_nbuf_free(*rx_netbuf);
				*rx_netbuf = NULL;
				continue;
			}

			*paddr = qdf_nbuf_get_frag_paddr(*rx_netbuf, 0);
		}
	} while (nbuf_retry < MAX_RETRY);

	if ((*rx_netbuf)) {
		qdf_nbuf_unmap_single(dp_soc->osdev, *rx_netbuf,
					QDF_DMA_FROM_DEVICE);
		qdf_nbuf_free(*rx_netbuf);
	}

	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * dp_rx_cookie_2_link_desc_va() - Converts cookie to a virtual address of
 *				   the MSDU Link Descriptor
 * @soc: core txrx main context
 * @buf_info: buf_info include cookie that used to lookup virtual address of
 * link descriptor Normally this is just an index into a per SOC array.
 *
 * This is the VA of the link descriptor, that HAL layer later uses to
 * retrieve the list of MSDU's for a given MPDU.
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_link_desc_va(struct dp_soc *soc,
				  struct hal_buf_info *buf_info)
{
	void *link_desc_va;
	uint32_t bank_id = LINK_DESC_COOKIE_BANK_ID(buf_info->sw_cookie);


	/* TODO */
	/* Add sanity for  cookie */

	link_desc_va = soc->link_desc_banks[bank_id].base_vaddr +
		(buf_info->paddr -
			soc->link_desc_banks[bank_id].base_paddr);

	return link_desc_va;
}

/**
 * dp_rx_cookie_2_mon_link_desc_va() - Converts cookie to a virtual address of
 *				   the MSDU Link Descriptor
 * @pdev: core txrx pdev context
 * @buf_info: buf_info includes cookie that used to lookup virtual address of
 * link descriptor. Normally this is just an index into a per pdev array.
 *
 * This is the VA of the link descriptor in monitor mode destination ring,
 * that HAL layer later uses to retrieve the list of MSDU's for a given MPDU.
 *
 * Return: void *: Virtual Address of the Rx descriptor
 */
static inline
void *dp_rx_cookie_2_mon_link_desc_va(struct dp_pdev *pdev,
				  struct hal_buf_info *buf_info,
				  int mac_id)
{
	void *link_desc_va;
	int mac_for_pdev = dp_get_mac_id_for_mac(pdev->soc, mac_id);

	/* TODO */
	/* Add sanity for  cookie */

	link_desc_va =
	   pdev->link_desc_banks[mac_for_pdev][buf_info->sw_cookie].base_vaddr +
	   (buf_info->paddr -
	   pdev->link_desc_banks[mac_for_pdev][buf_info->sw_cookie].base_paddr);

	return link_desc_va;
}

/**
 * dp_rx_defrag_concat() - Concatenate the fragments
 *
 * @dst: destination pointer to the buffer
 * @src: source pointer from where the fragment payload is to be copied
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS dp_rx_defrag_concat(qdf_nbuf_t dst, qdf_nbuf_t src)
{
	/*
	 * Inside qdf_nbuf_cat, if it is necessary to reallocate dst
	 * to provide space for src, the headroom portion is copied from
	 * the original dst buffer to the larger new dst buffer.
	 * (This is needed, because the headroom of the dst buffer
	 * contains the rx desc.)
	 */
	if (!qdf_nbuf_cat(dst, src)) {
		/*
		 * qdf_nbuf_cat does not free the src memory.
		 * Free src nbuf before returning
		 * For failure case the caller takes of freeing the nbuf
		 */
		qdf_nbuf_free(src);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_DEFRAG_ERROR;
}

#ifndef FEATURE_WDS
static inline QDF_STATUS dp_rx_ast_set_active(struct dp_soc *soc, uint16_t sa_idx, bool is_active)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
dp_rx_wds_srcport_learn(struct dp_soc *soc,
			uint8_t *rx_tlv_hdr,
			struct dp_peer *ta_peer,
			qdf_nbuf_t nbuf)
{
}
#endif

/*
 * dp_rx_desc_dump() - dump the sw rx descriptor
 *
 * @rx_desc: sw rx descriptor
 */
static inline void dp_rx_desc_dump(struct dp_rx_desc *rx_desc)
{
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
		  "rx_desc->nbuf: %pK, rx_desc->cookie: %d, rx_desc->pool_id: %d, rx_desc->in_use: %d, rx_desc->unmapped: %d",
		  rx_desc->nbuf, rx_desc->cookie, rx_desc->pool_id,
		  rx_desc->in_use, rx_desc->unmapped);
}

/*
 * check_qwrap_multicast_loopback() - Check if rx packet is a loopback packet.
 *					In qwrap mode, packets originated from
 *					any vdev should not loopback and
 *					should be dropped.
 * @vdev: vdev on which rx packet is received
 * @nbuf: rx pkt
 *
 */
#if ATH_SUPPORT_WRAP
static inline bool check_qwrap_multicast_loopback(struct dp_vdev *vdev,
						qdf_nbuf_t nbuf)
{
	struct dp_vdev *psta_vdev;
	struct dp_pdev *pdev = vdev->pdev;
	uint8_t *data = qdf_nbuf_data(nbuf);

	if (qdf_unlikely(vdev->proxysta_vdev)) {
		/* In qwrap isolation mode, allow loopback packets as all
		 * packets go to RootAP and Loopback on the mpsta.
		 */
		if (vdev->isolation_vdev)
			return false;
		TAILQ_FOREACH(psta_vdev, &pdev->vdev_list, vdev_list_elem) {
			if (qdf_unlikely(psta_vdev->proxysta_vdev &&
					 !qdf_mem_cmp(psta_vdev->mac_addr.raw,
						      &data[QDF_MAC_ADDR_SIZE],
						      QDF_MAC_ADDR_SIZE))) {
				/* Drop packet if source address is equal to
				 * any of the vdev addresses.
				 */
				return true;
			}
		}
	}
	return false;
}
#else
static inline bool check_qwrap_multicast_loopback(struct dp_vdev *vdev,
						qdf_nbuf_t nbuf)
{
	return false;
}
#endif

#if defined(WLAN_SUPPORT_RX_TAG_STATISTICS) && \
	defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG)
/**
 * dp_rx_update_rx_protocol_tag_stats() - Increments the protocol tag stats
 *                                        for the given protocol type
 * @soc: core txrx main context
 * @pdev: TXRX pdev context for which stats should be incremented
 * @protocol_index: Protocol index for which the stats should be incremented
 * @ring_index: REO ring number from which this tag was received.
 *
 * Since HKv2 is a SMP, two or more cores may simultaneously receive packets
 * of same type, and hence attempt to increment counters for the same protocol
 * type at the same time. This creates the possibility of missing stats.
 *
 * For example,  when two or more CPUs have each read the old tag value, V,
 * for protocol type, P and each increment the value to V+1. Instead, the
 * operations should have been  sequenced to achieve a final value of V+2.
 *
 * In order to avoid this scenario,  we can either use locks or store stats
 * on a per-CPU basis. Since tagging happens in the core data path, locks
 * are not preferred. Instead, we use a per-ring counter, since each CPU
 * operates on a REO ring.
 *
 * Return: void
 */
static inline void dp_rx_update_rx_protocol_tag_stats(struct dp_pdev *pdev,
						      uint16_t protocol_index,
						      uint16_t ring_index)
{
	if (ring_index >= MAX_REO_DEST_RINGS)
		return;

	pdev->reo_proto_tag_stats[ring_index][protocol_index].tag_ctr++;
}
#else
static inline void dp_rx_update_rx_protocol_tag_stats(struct dp_pdev *pdev,
						      uint16_t protocol_index,
						      uint16_t ring_index)
{
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */

#if defined(WLAN_SUPPORT_RX_TAG_STATISTICS) && \
	defined(WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG)
/**
 * dp_rx_update_rx_err_protocol_tag_stats() - Increments the protocol tag stats
 *                                        for the given protocol type
 *                                        received from exception ring
 * @soc: core txrx main context
 * @pdev: TXRX pdev context for which stats should be incremented
 * @protocol_index: Protocol index for which the stats should be incremented
 *
 * In HKv2, all exception packets are received on Ring-0 (along with normal
 * Rx). Hence tags are maintained separately for exception ring as well.
 *
 * Return: void
 */
static inline
void dp_rx_update_rx_err_protocol_tag_stats(struct dp_pdev *pdev,
					    uint16_t protocol_index)
{
	pdev->rx_err_proto_tag_stats[protocol_index].tag_ctr++;
}
#else
static inline
void dp_rx_update_rx_err_protocol_tag_stats(struct dp_pdev *pdev,
					    uint16_t protocol_index)
{
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */
/**
 * dp_rx_update_protocol_tag() - Reads CCE metadata from the RX MSDU end TLV
 *                              and set the corresponding tag in QDF packet
 * @soc: core txrx main context
 * @vdev: vdev on which the packet is received
 * @nbuf: QDF pkt buffer on which the protocol tag should be set
 * @rx_tlv_hdr: rBbase address where the RX TLVs starts
 * @ring_index: REO ring number, not used for error & monitor ring
 * @is_reo_exception: flag to indicate if rx from REO ring or exception ring
 * @is_update_stats: flag to indicate whether to update stats or not
 * Return: void
 */
#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
static inline void
dp_rx_update_protocol_tag(struct dp_soc *soc, struct dp_vdev *vdev,
			  qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr,
			  uint16_t ring_index,
			  bool is_reo_exception, bool is_update_stats)
{
	uint16_t cce_metadata = RX_PROTOCOL_TAG_START_OFFSET;
	bool     cce_match = false;
	struct   dp_pdev *pdev;
	uint16_t protocol_tag = 0;

	if (qdf_unlikely(!vdev))
		return;

	pdev = vdev->pdev;

	if (qdf_likely(!pdev->is_rx_protocol_tagging_enabled))
		return;

	/*
	 * In case of raw frames, rx_attention and rx_msdu_end tlv
	 * may be stale or invalid. Do not tag such frames.
	 * Default decap_type is set to ethernet for monitor vdev,
	 * therefore, cannot check decap_type for monitor mode.
	 * We will call this only for eth frames from dp_rx_mon_dest.c.
	 */
	if (qdf_likely(!(pdev->monitor_vdev && pdev->monitor_vdev == vdev) &&
		       (vdev->rx_decap_type !=  htt_cmn_pkt_type_ethernet)))
		return;

	/*
	 * Check whether HW has filled in the CCE metadata in
	 * this packet, if not filled, just return
	 */
	if (qdf_likely(!hal_rx_msdu_cce_match_get(rx_tlv_hdr)))
		return;

	cce_match = true;
	/* Get the cce_metadata from RX MSDU TLV */
	cce_metadata = (hal_rx_msdu_cce_metadata_get(rx_tlv_hdr) &
			RX_MSDU_END_16_CCE_METADATA_MASK);
	/*
	 * Received CCE metadata should be within the
	 * valid limits
	 */
	qdf_assert_always((cce_metadata >= RX_PROTOCOL_TAG_START_OFFSET) &&
			  (cce_metadata < (RX_PROTOCOL_TAG_START_OFFSET +
			   RX_PROTOCOL_TAG_MAX)));

	/*
	 * The CCE metadata received is just the
	 * packet_type + RX_PROTOCOL_TAG_START_OFFSET
	 */
	cce_metadata -= RX_PROTOCOL_TAG_START_OFFSET;

	/*
	 * Update the QDF packet with the user-specified
	 * tag/metadata by looking up tag value for
	 * received protocol type.
	 */
	protocol_tag = pdev->rx_proto_tag_map[cce_metadata].tag;
	qdf_nbuf_set_rx_protocol_tag(nbuf, protocol_tag);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		  "Seq:%u decap:%u CCE Match:%d ProtoID:%u Tag:%u US:%d",
		  hal_rx_get_rx_sequence(rx_tlv_hdr),
		  vdev->rx_decap_type, cce_match, cce_metadata,
		  protocol_tag, is_update_stats);

	if (qdf_likely(!is_update_stats))
		return;

	if (qdf_unlikely(is_reo_exception)) {
		dp_rx_update_rx_err_protocol_tag_stats(pdev,
						       cce_metadata);
	} else {
		dp_rx_update_rx_protocol_tag_stats(pdev,
						   cce_metadata,
						   ring_index);
	}

}
#else
static inline void
dp_rx_update_protocol_tag(struct dp_soc *soc, struct dp_vdev *vdev,
			  qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr,
			  uint16_t ring_index,
			  bool is_reo_exception, bool is_update_stats)
{
	/* Stub API */
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

/**
 * dp_rx_mon_update_protocol_tag() - Performs necessary checks for monitor mode
 *				and then tags appropriate packets
 * @soc: core txrx main context
 * @vdev: pdev on which packet is received
 * @msdu: QDF packet buffer on which the protocol tag should be set
 * @rx_desc: base address where the RX TLVs start
 * Return: void
 */
#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
static inline
void dp_rx_mon_update_protocol_tag(struct dp_soc *soc, struct dp_pdev *dp_pdev,
				   qdf_nbuf_t msdu, void *rx_desc)
{
	uint32_t msdu_ppdu_id = 0;
	struct mon_rx_status *mon_recv_status;

	if (qdf_likely(!dp_pdev->is_rx_protocol_tagging_enabled))
		return;

	if (qdf_likely(!dp_pdev->monitor_vdev))
		return;

	if (qdf_likely(1 != dp_pdev->ppdu_info.rx_status.rxpcu_filter_pass))
		return;

	msdu_ppdu_id = HAL_RX_HW_DESC_GET_PPDUID_GET(rx_desc);

	if (msdu_ppdu_id != dp_pdev->ppdu_info.com_info.ppdu_id) {
		QDF_TRACE(QDF_MODULE_ID_DP,
			  QDF_TRACE_LEVEL_ERROR,
			  "msdu_ppdu_id=%x,com_info.ppdu_id=%x",
			  msdu_ppdu_id,
			  dp_pdev->ppdu_info.com_info.ppdu_id);
		return;
	}

	/*
	 * Update the protocol tag in SKB for packets received on BSS.
	 * Do not update tag stats since it would double actual received count
	 */
	mon_recv_status = &dp_pdev->ppdu_info.rx_status;
	if (mon_recv_status->frame_control_info_valid &&
	    ((mon_recv_status->frame_control & IEEE80211_FC0_TYPE_MASK) ==
	      IEEE80211_FC0_TYPE_DATA)) {
		dp_rx_update_protocol_tag(soc,
					  dp_pdev->monitor_vdev,
					  msdu, rx_desc,
					  MAX_REO_DEST_RINGS,
					  false, false);
	}
}
#else
static inline
void dp_rx_mon_update_protocol_tag(struct dp_soc *soc, struct dp_pdev *dp_pdev,
				   qdf_nbuf_t msdu, void *rx_desc)
{
	/* Stub API */
}
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */
/*
 * dp_rx_buffers_replenish() - replenish rxdma ring with rx nbufs
 *			       called during dp rx initialization
 *			       and at the end of dp_rx_process.
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @dp_rxdma_srng: dp rxdma circular ring
 * @rx_desc_pool: Pointer to free Rx descriptor pool
 * @num_req_buffers: number of buffer to be replenished
 * @desc_list: list of descs if called from dp_rx_process
 *	       or NULL during dp rx initialization or out of buffer
 *	       interrupt.
 * @tail: tail of descs list
 * Return: return success or failure
 */
QDF_STATUS dp_rx_buffers_replenish(struct dp_soc *dp_soc, uint32_t mac_id,
				 struct dp_srng *dp_rxdma_srng,
				 struct rx_desc_pool *rx_desc_pool,
				 uint32_t num_req_buffers,
				 union dp_rx_desc_list_elem_t **desc_list,
				 union dp_rx_desc_list_elem_t **tail);

/**
 * dp_rx_link_desc_return() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @soc: core DP main context
 * @buf_addr_info: opaque pointer to the REO error ring descriptor
 * @buf_addr_info: void pointer to the buffer_addr_info
 * @bm_action: put to idle_list or release to msdu_list
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_link_desc_return(struct dp_soc *soc, void *ring_desc, uint8_t bm_action);

QDF_STATUS
dp_rx_link_desc_buf_return(struct dp_soc *soc, struct dp_srng *dp_rxdma_srng,
				void *buf_addr_info, uint8_t bm_action);
/**
 * dp_rx_link_desc_return_by_addr - Return a MPDU link descriptor to
 *					(WBM) by address
 *
 * @soc: core DP main context
 * @link_desc_addr: link descriptor addr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_link_desc_return_by_addr(struct dp_soc *soc, void *link_desc_addr,
					uint8_t bm_action);

/**
 * dp_rxdma_err_process() - RxDMA error processing functionality
 * @soc: core txrx main contex
 * @mac_id: mac id which is one of 3 mac_ids
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * Return: num of buffers processed
 */
uint32_t
dp_rxdma_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
		     uint32_t mac_id, uint32_t quota);

void dp_rx_fill_mesh_stats(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				uint8_t *rx_tlv_hdr, struct dp_peer *peer);
QDF_STATUS dp_rx_filter_mesh_packets(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
					uint8_t *rx_tlv_hdr);

int dp_wds_rx_policy_check(uint8_t *rx_tlv_hdr, struct dp_vdev *vdev,
			   struct dp_peer *peer);

qdf_nbuf_t
dp_rx_nbuf_prepare(struct dp_soc *soc, struct dp_pdev *pdev);

void dp_rx_dump_info_and_assert(struct dp_soc *soc, void *hal_ring,
				void *ring_desc, struct dp_rx_desc *rx_desc);

void dp_rx_compute_delay(struct dp_vdev *vdev, qdf_nbuf_t nbuf);
#ifdef RX_DESC_DEBUG_CHECK
/**
 * dp_rx_desc_check_magic() - check the magic value in dp_rx_desc
 * @rx_desc: rx descriptor pointer
 *
 * Return: true, if magic is correct, else false.
 */
static inline bool dp_rx_desc_check_magic(struct dp_rx_desc *rx_desc)
{
	if (qdf_unlikely(rx_desc->magic != DP_RX_DESC_MAGIC))
		return false;

	rx_desc->magic = 0;
	return true;
}

/**
 * dp_rx_desc_prep() - prepare rx desc
 * @rx_desc: rx descriptor pointer to be prepared
 * @nbuf: nbuf to be associated with rx_desc
 *
 * Note: assumption is that we are associating a nbuf which is mapped
 *
 * Return: none
 */
static inline void dp_rx_desc_prep(struct dp_rx_desc *rx_desc, qdf_nbuf_t nbuf)
{
	rx_desc->magic = DP_RX_DESC_MAGIC;
	rx_desc->nbuf = nbuf;
	rx_desc->unmapped = 0;
}

#else

static inline bool dp_rx_desc_check_magic(struct dp_rx_desc *rx_desc)
{
	return true;
}

static inline void dp_rx_desc_prep(struct dp_rx_desc *rx_desc, qdf_nbuf_t nbuf)
{
	rx_desc->nbuf = nbuf;
	rx_desc->unmapped = 0;
}
#endif /* RX_DESC_DEBUG_CHECK */

void dp_rx_process_rxdma_err(struct dp_soc *soc, qdf_nbuf_t nbuf,
			     uint8_t *rx_tlv_hdr, struct dp_peer *peer,
			     uint8_t err_code, uint8_t mac_id);

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
#endif /* _DP_RX_H */
