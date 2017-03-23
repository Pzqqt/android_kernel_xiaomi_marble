/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifdef RXDMA_OPTIMIZATION
#define RX_BUFFER_ALIGNMENT     128
#else /* RXDMA_OPTIMIZATION */
#define RX_BUFFER_ALIGNMENT     4
#endif /* RXDMA_OPTIMIZATION */

#define RX_BUFFER_SIZE          2048
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
 */
struct dp_rx_desc {
	qdf_nbuf_t nbuf;
	uint8_t *rx_buf_start;
	uint32_t cookie;
	uint8_t	 pool_id;
};

#define RX_DESC_COOKIE_INDEX_SHIFT		0
#define RX_DESC_COOKIE_INDEX_MASK		0x3ffff /* 18 bits */
#define RX_DESC_COOKIE_POOL_ID_SHIFT		18
#define RX_DESC_COOKIE_POOL_ID_MASK		0x1c0000

#define DP_RX_DESC_COOKIE_POOL_ID_GET(_cookie)		\
	(((_cookie) & RX_DESC_COOKIE_POOL_ID_MASK) >>	\
			RX_DESC_COOKIE_POOL_ID_SHIFT)

#define DP_RX_DESC_COOKIE_INDEX_GET(_cookie)		\
	(((_cookie) & RX_DESC_COOKIE_INDEX_MASK) >>	\
			RX_DESC_COOKIE_INDEX_SHIFT)

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
	/* TODO */
	/* Add sanity for pool_id & index */
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
dp_rx_process(struct dp_intr *int_ctx, void *hal_ring, uint32_t quota);

uint32_t dp_rx_err_process(struct dp_soc *soc, void *hal_ring, uint32_t quota);

uint32_t
dp_rx_wbm_err_process(struct dp_soc *soc, void *hal_ring, uint32_t quota);

QDF_STATUS dp_rx_desc_pool_alloc(struct dp_soc *soc,
				uint32_t pool_id,
				uint32_t pool_size,
				struct rx_desc_pool *rx_desc_pool);

void dp_rx_desc_pool_free(struct dp_soc *soc,
				uint32_t pool_id,
				struct rx_desc_pool *rx_desc_pool);

void dp_rx_deliver_raw(struct dp_vdev *vdev, qdf_nbuf_t nbuf_list);

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

	((union dp_rx_desc_list_elem_t *)new)->next = *head;
	*head = (union dp_rx_desc_list_elem_t *)new;
	if (*tail == NULL)
		*tail = *head;

}

/**
 * dp_rx_wds_srcport_learn() - Add or update the STA PEER which
 *				is behind the WDS repeater.
 *
 * @soc: core txrx main context
 * @rx_tlv_hdr: base address of RX TLV header
 * @ta_peer: WDS repeater peer
 * @nbuf: rx pkt
 *
 * Return: void:
 */
#ifndef CONFIG_MCL
static inline void
dp_rx_wds_srcport_learn(struct dp_soc *soc,
			 uint8_t *rx_tlv_hdr,
			 struct dp_peer *ta_peer,
			 qdf_nbuf_t nbuf)
{
	uint16_t sa_sw_peer_id = hal_rx_msdu_end_sa_sw_peer_id_get(rx_tlv_hdr);
	uint32_t flags = IEEE80211_NODE_F_WDS_HM;
	uint32_t ret = 0;
	uint8_t wds_src_mac[IEEE80211_ADDR_LEN];

	memcpy(wds_src_mac, (qdf_nbuf_data(nbuf) + IEEE80211_ADDR_LEN),
		IEEE80211_ADDR_LEN);

	if (!hal_rx_msdu_end_sa_is_valid_get(rx_tlv_hdr)) {
		ret = soc->cdp_soc.ol_ops->peer_add_wds_entry(
						soc->osif_soc,
						wds_src_mac,
						ta_peer->mac_addr.raw,
						flags);
	} else if (sa_sw_peer_id != ta_peer->peer_ids[0]) {
		ret = soc->cdp_soc.ol_ops->peer_update_wds_entry(
						soc->osif_soc,
						wds_src_mac,
						ta_peer->mac_addr.raw,
						flags);
	}
	return;
}
#else
static inline void
dp_rx_wds_srcport_learn(struct dp_soc *soc,
			 uint8_t *rx_tlv_hdr,
			 struct dp_peer *ta_peer,
			 qdf_nbuf_t nbuf)
{
}
#endif

uint8_t dp_rx_process_invalid_peer(struct dp_soc *soc, qdf_nbuf_t nbuf);
#define DP_RX_LIST_APPEND(head, tail, elem) \
do {                                                \
	if (!(head)) {                              \
		(head) = (elem);                    \
	} else {                                    \
		qdf_nbuf_set_next((tail), (elem));  \
	}                                           \
	(tail) = (elem);                            \
	qdf_nbuf_set_next((tail), NULL);            \
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
				"phy addr %p exceded 0x50000000 trying again\n",
				paddr);

			nbuf_retry++;
			if ((*rx_netbuf)) {
				qdf_nbuf_unmap_single(dp_soc->osdev, *rx_netbuf,
							QDF_DMA_BIDIRECTIONAL);
				/* Not freeing buffer intentionally.
				 * Observed that same buffer is getting
				 * re-allocated resulting in longer load time
				 * WMI init timeout.
				 * This buffer is anyway not useful so skip it.
				 **/
			}

			*rx_netbuf = qdf_nbuf_alloc(pdev->osif_pdev,
							RX_BUFFER_SIZE,
							RX_BUFFER_RESERVATION,
							RX_BUFFER_ALIGNMENT,
							FALSE);

			if (qdf_unlikely(!(*rx_netbuf)))
				return QDF_STATUS_E_FAILURE;

			ret = qdf_nbuf_map_single(dp_soc->osdev, *rx_netbuf,
							QDF_DMA_BIDIRECTIONAL);

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
					QDF_DMA_BIDIRECTIONAL);
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

	/* TODO */
	/* Add sanity for  cookie */

	link_desc_va = soc->link_desc_banks[buf_info->sw_cookie].base_vaddr +
		(buf_info->paddr -
			soc->link_desc_banks[buf_info->sw_cookie].base_paddr);

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
				  struct hal_buf_info *buf_info)
{
	void *link_desc_va;

	/* TODO */
	/* Add sanity for  cookie */

	link_desc_va = pdev->link_desc_banks[buf_info->sw_cookie].base_vaddr +
		(buf_info->paddr -
			pdev->link_desc_banks[buf_info->sw_cookie].base_paddr);

	return link_desc_va;
}

/*
 * dp_rx_buffers_replenish() - replenish rxdma ring with rx nbufs
 *			       called during dp rx initialization
 *			       and at the end of dp_rx_process.
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @dp_rxdma_srng: dp rxdma circular ring
 * @rx_desc_pool: Poiter to free Rx descriptor pool
 * @num_req_buffers: number of buffer to be replenished
 * @desc_list: list of descs if called from dp_rx_process
 *	       or NULL during dp rx initialization or out of buffer
 *	       interrupt.
 * @tail: tail of descs list
 * @owner: who owns the nbuf (host, NSS etc...)
 * Return: return success or failure
 */
QDF_STATUS dp_rx_buffers_replenish(struct dp_soc *dp_soc, uint32_t mac_id,
				 struct dp_srng *dp_rxdma_srng,
				 struct rx_desc_pool *rx_desc_pool,
				 uint32_t num_req_buffers,
				 union dp_rx_desc_list_elem_t **desc_list,
				 union dp_rx_desc_list_elem_t **tail,
				 uint8_t owner);

/**
 * dp_rx_link_desc_return() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @soc: core DP main context
 * @buf_addr_info: opaque pointer to the REO error ring descriptor
 * @buf_addr_info: void pointer to the buffer_addr_info
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_link_desc_buf_return(struct dp_soc *soc, struct dp_srng *dp_rxdma_srng,
				void *buf_addr_info);
#endif /* _DP_RX_H */
