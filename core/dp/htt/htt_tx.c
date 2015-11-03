/*
 * Copyright (c) 2011, 2014-2015 The Linux Foundation. All rights reserved.
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

/**
 * @file htt_tx.c
 * @brief Implement transmit aspects of HTT.
 * @details
 *  This file contains three categories of HTT tx code:
 *  1.  An abstraction of the tx descriptor, to hide the
 *      differences between the HL vs. LL tx descriptor.
 *  2.  Functions for allocating and freeing HTT tx descriptors.
 *  3.  The function that accepts a tx frame from txrx and sends the
 *      tx frame to HTC.
 */
#include <osdep.h>              /* uint32_t, offsetof, etc. */
#include <cdf_types.h>          /* cdf_dma_addr_t */
#include <cdf_memory.h>         /* cdf_os_mem_alloc_consistent et al */
#include <cdf_nbuf.h>           /* cdf_nbuf_t, etc. */
#include <cdf_time.h>           /* cdf_mdelay */

#include <htt.h>                /* htt_tx_msdu_desc_t */
#include <htc.h>                /* HTC_HDR_LENGTH */
#include <htc_api.h>            /* htc_flush_surprise_remove */
#include <ol_cfg.h>             /* ol_cfg_netbuf_frags_max, etc. */
#include <ol_htt_tx_api.h>      /* HTT_TX_DESC_VADDR_OFFSET */
#include <ol_txrx_htt_api.h>    /* ol_tx_msdu_id_storage */
#include <htt_internal.h>

/* IPA Micro controler TX data packet HTT Header Preset */
/* 31 | 30  29 | 28 | 27 | 26  22  | 21   16 | 15  13   | 12  8      | 7 0
   *----------------------------------------------------------------------------
 * R  | CS  OL | R  | PP | ext TID | vdev ID | pkt type | pkt subtyp | msg type
 * 0  | 0      | 0  |    | 0x1F    | 0       | 2        | 0          | 0x01
 ***----------------------------------------------------------------------------
 * pkt ID                                    | pkt length
 ***----------------------------------------------------------------------------
 *                                frag_desc_ptr
 ***----------------------------------------------------------------------------
 *                                   peer_id
 ***----------------------------------------------------------------------------
 */
#define HTT_IPA_UC_OFFLOAD_TX_HEADER_DEFAULT 0x07C04001

/*--- setup / tear-down functions -------------------------------------------*/

#ifdef QCA_SUPPORT_TXDESC_SANITY_CHECKS
uint32_t *g_dbg_htt_desc_end_addr, *g_dbg_htt_desc_start_addr;
#endif

int htt_tx_attach(struct htt_pdev_t *pdev, int desc_pool_elems)
{
	int i, pool_size;
	uint32_t **p;
	cdf_dma_addr_t pool_paddr;

#if defined(HELIUMPLUS_PADDR64)
	pdev->tx_descs.size = sizeof(struct htt_host_tx_desc_t);

	if (HTT_WIFI_IP_VERSION(pdev->wifi_ip_ver.major, 0x2)) {
		/*
		* sizeof MSDU_EXT/Fragmentation descriptor.
		*/
		pdev->frag_descs.size = sizeof(struct msdu_ext_desc_t);
	} else {
		/*
		 * Add the fragmentation descriptor elements.
		 * Add the most that the OS may deliver, plus one more
		 * in case the txrx code adds a prefix fragment (for
		 * TSO or audio interworking SNAP header)
		 */
		pdev->frag_descs.size =
			(ol_cfg_netbuf_frags_max(pdev->ctrl_pdev)+1) * 8
			+ 4;
	}
#else /* ! defined(HELIUMPLUS_PADDR64) */
	/*
	 * Start with the size of the base struct
	 * that actually gets downloaded.
	 *
	 * Add the fragmentation descriptor elements.
	 * Add the most that the OS may deliver, plus one more
	 * in case the txrx code adds a prefix fragment (for
	 * TSO or audio interworking SNAP header)
	 */
	pdev->tx_descs.size =
		sizeof(struct htt_host_tx_desc_t)
		+ (ol_cfg_netbuf_frags_max(pdev->ctrl_pdev) + 1) * 8
		   /* 2x uint32_t */
		+ 4; /* uint32_t fragmentation list terminator */

	if (pdev->tx_descs.size < sizeof(uint32_t *))
		pdev->tx_descs.size = sizeof(uint32_t *);
#endif /* defined(HELIUMPLUS_PADDR64) */
	/*
	 * Make sure tx_descs.size is a multiple of 4-bytes.
	 * It should be, but round up just to be sure.
	 */
	pdev->tx_descs.size = (pdev->tx_descs.size + 3) & (~0x3);

	pdev->tx_descs.pool_elems = desc_pool_elems;
	pdev->tx_descs.alloc_cnt = 0;

	pool_size = pdev->tx_descs.pool_elems * pdev->tx_descs.size;

	pdev->tx_descs.pool_vaddr =
		cdf_os_mem_alloc_consistent(
			pdev->osdev, pool_size,
			&pool_paddr,
			cdf_get_dma_mem_context((&pdev->tx_descs), memctx));

	pdev->tx_descs.pool_paddr = pool_paddr;

	if (!pdev->tx_descs.pool_vaddr)
		return -ENOBUFS;       /* failure */

	cdf_print("%s:htt_desc_start:0x%p htt_desc_end:0x%p\n", __func__,
		  pdev->tx_descs.pool_vaddr,
		  (uint32_t *) (pdev->tx_descs.pool_vaddr + pool_size));

#if defined(HELIUMPLUS_PADDR64)
	pdev->frag_descs.pool_elems = desc_pool_elems;
	/*
	 * Allocate space for MSDU extension descriptor
	 * H/W expects this in contiguous memory
	 */
	pool_size = pdev->frag_descs.pool_elems * pdev->frag_descs.size;

	pdev->frag_descs.pool_vaddr = cdf_os_mem_alloc_consistent(
		pdev->osdev, pool_size, &pool_paddr,
		cdf_get_dma_mem_context((&pdev->frag_descs), memctx));

	if (!pdev->frag_descs.pool_vaddr)
		return -ENOBUFS; /* failure */

	pdev->frag_descs.pool_paddr = pool_paddr;

	cdf_print("%s:MSDU Ext.Table Start:0x%p MSDU Ext.Table End:0x%p\n",
		  __func__, pdev->frag_descs.pool_vaddr,
		  (u_int32_t *) (pdev->frag_descs.pool_vaddr + pool_size));
#endif /* defined(HELIUMPLUS_PADDR64) */

#ifdef QCA_SUPPORT_TXDESC_SANITY_CHECKS
	g_dbg_htt_desc_end_addr = (uint32_t *)
				  (pdev->tx_descs.pool_vaddr + pool_size);
	g_dbg_htt_desc_start_addr = (uint32_t *) pdev->tx_descs.pool_vaddr;
#endif

	/* link tx descriptors into a freelist */
	pdev->tx_descs.freelist = (uint32_t *) pdev->tx_descs.pool_vaddr;
	p = (uint32_t **) pdev->tx_descs.freelist;
	for (i = 0; i < desc_pool_elems - 1; i++) {
		*p = (uint32_t *) (((char *)p) + pdev->tx_descs.size);
		p = (uint32_t **) *p;
	}
	*p = NULL;

	return 0;               /* success */
}

void htt_tx_detach(struct htt_pdev_t *pdev)
{
	if (pdev) {
		cdf_os_mem_free_consistent(
			pdev->osdev,
			/* pool_size */
			pdev->tx_descs.pool_elems * pdev->tx_descs.size,
			pdev->tx_descs.pool_vaddr,
			pdev->tx_descs.pool_paddr,
			cdf_get_dma_mem_context((&pdev->tx_descs), memctx));
#if defined(HELIUMPLUS_PADDR64)
		cdf_os_mem_free_consistent(
			pdev->osdev,
			/* pool_size */
			pdev->frag_descs.pool_elems *
			pdev->frag_descs.size,
			pdev->frag_descs.pool_vaddr,
			pdev->frag_descs.pool_paddr,
			cdf_get_dma_mem_context((&pdev->frag_descs), memctx));
#endif /* defined(HELIUMPLUS_PADDR64) */
	}
}

/*--- descriptor allocation functions ---------------------------------------*/

void *htt_tx_desc_alloc(htt_pdev_handle pdev, uint32_t *paddr_lo)
{
	struct htt_host_tx_desc_t *htt_host_tx_desc;    /* includes HTC hdr */
	struct htt_tx_msdu_desc_t *htt_tx_desc; /* doesn't include  HTC hdr */
	uint16_t index;
	uint32_t *fragmentation_descr_field_ptr;

	htt_host_tx_desc = (struct htt_host_tx_desc_t *)pdev->tx_descs.freelist;
	if (!htt_host_tx_desc)
		return NULL;    /* pool is exhausted */

	htt_tx_desc = &htt_host_tx_desc->align32.tx_desc;

	if (pdev->tx_descs.freelist) {
		pdev->tx_descs.freelist =
			*((uint32_t **) pdev->tx_descs.freelist);
		pdev->tx_descs.alloc_cnt++;
	}
	/*
	 * For LL, set up the fragmentation descriptor address.
	 * Currently, this HTT tx desc allocation is performed once up front.
	 * If this is changed to have the allocation done during tx, then it
	 * would be helpful to have separate htt_tx_desc_alloc functions for
	 * HL vs. LL, to remove the below conditional branch.
	 */
	fragmentation_descr_field_ptr = (uint32_t *)
		((uint32_t *) htt_tx_desc) +
		HTT_TX_DESC_FRAGS_DESC_PADDR_OFFSET_DWORD;

	index = ((char *)htt_host_tx_desc -
		 (char *)(((struct htt_host_tx_desc_t *)
			   pdev->tx_descs.pool_vaddr))) /
		pdev->tx_descs.size;
	/*
	 * The fragmentation descriptor is allocated from consistent
	 * memory. Therefore, we can use the address directly rather
	 * than having to map it from a virtual/CPU address to a
	 * physical/bus address.
	 */
#if defined(HELIUMPLUS_PADDR64)
#if HTT_PADDR64
	/* this is: frags_desc_ptr.lo */
	*fragmentation_descr_field_ptr = (uint32_t)
		(pdev->frag_descs.pool_paddr +
		 (pdev->frag_descs.size * index));
	fragmentation_descr_field_ptr++;
	/* frags_desc_ptr.hi */
	*fragmentation_descr_field_ptr = 0;
#else /* ! HTT_PADDR64 */
	*fragmentation_descr_field_ptr = (uint32_t)
		(pdev->frag_descs.pool_paddr +
		 (pdev->frag_descs.size * index));
	cdf_print("%s %d: i %d frag_paddr 0x%x\n",
		  __func__, __LINE__, index,
		  (*fragmentation_descr_field_ptr));
#endif /* HTT_PADDR64 */
#else /* !HELIUMPLUS_PADDR64 */
	*fragmentation_descr_field_ptr =
		HTT_TX_DESC_PADDR(pdev, htt_tx_desc) + HTT_TX_DESC_LEN;
#endif /* HELIUMPLUS_PADDR64 */

	/*
	 * Include the headroom for the HTC frame header when specifying the
	 * physical address for the HTT tx descriptor.
	 */
	*paddr_lo = (uint32_t) HTT_TX_DESC_PADDR(pdev, htt_host_tx_desc);
	/*
	 * The allocated tx descriptor space includes headroom for a
	 * HTC frame header.  Hide this headroom, so that we don't have
	 * to jump past the headroom each time we program a field within
	 * the tx desc, but only once when we download the tx desc (and
	 * the headroom) to the target via HTC.
	 * Skip past the headroom and return the address of the HTT tx desc.
	 */
	return (void *)htt_tx_desc;
}

void htt_tx_desc_free(htt_pdev_handle pdev, void *tx_desc)
{
	char *htt_host_tx_desc = tx_desc;
	/* rewind over the HTC frame header space */
	htt_host_tx_desc -=
		offsetof(struct htt_host_tx_desc_t, align32.tx_desc);
	*((uint32_t **) htt_host_tx_desc) = pdev->tx_descs.freelist;
	pdev->tx_descs.freelist = (uint32_t *) htt_host_tx_desc;
	pdev->tx_descs.alloc_cnt--;
}

/*--- descriptor field access methods ---------------------------------------*/

void htt_tx_desc_frags_table_set(htt_pdev_handle pdev,
				 void *htt_tx_desc,
				 uint32_t paddr,
				 uint32_t frag_desc_paddr_lo,
				 int reset)
{
	uint32_t *fragmentation_descr_field_ptr;

	fragmentation_descr_field_ptr = (uint32_t *)
		((uint32_t *) htt_tx_desc) +
		HTT_TX_DESC_FRAGS_DESC_PADDR_OFFSET_DWORD;
	if (reset) {
#if defined(HELIUMPLUS_PADDR64)
		*fragmentation_descr_field_ptr = frag_desc_paddr_lo;
#else
		*fragmentation_descr_field_ptr =
			HTT_TX_DESC_PADDR(pdev, htt_tx_desc) + HTT_TX_DESC_LEN;
#endif
	} else {
		*fragmentation_descr_field_ptr = paddr;
	}
}

#if defined(HELIUMPLUS_PADDR64)
void *
htt_tx_frag_alloc(htt_pdev_handle pdev,
		  u_int16_t index,
		  u_int32_t *frag_paddr_lo)
{
	/** Index should never be 0, since its used by the hardware
	    to terminate the link. */
	if (index >= pdev->tx_descs.pool_elems)
		return NULL;

	*frag_paddr_lo = (uint32_t)
		(pdev->frag_descs.pool_paddr + (pdev->frag_descs.size * index));

	return ((char *) pdev->frag_descs.pool_vaddr) +
		(pdev->frag_descs.size * index);
}
#endif /* defined(HELIUMPLUS_PADDR64) */

/* PUT THESE AS INLINE IN ol_htt_tx_api.h */

void htt_tx_desc_flag_postponed(htt_pdev_handle pdev, void *desc)
{
}

void htt_tx_pending_discard(htt_pdev_handle pdev)
{
	htc_flush_surprise_remove(pdev->htc_pdev);
}

void htt_tx_desc_flag_batch_more(htt_pdev_handle pdev, void *desc)
{
}

/*--- tx send function ------------------------------------------------------*/

#ifdef ATH_11AC_TXCOMPACT

/* Scheduling the Queued packets in HTT which could not be sent out
   because of No CE desc*/
void htt_tx_sched(htt_pdev_handle pdev)
{
	cdf_nbuf_t msdu;
	int download_len = pdev->download_len;
	int packet_len;

	HTT_TX_NBUF_QUEUE_REMOVE(pdev, msdu);
	while (msdu != NULL) {
		int not_accepted;
		/* packet length includes HTT tx desc frag added above */
		packet_len = cdf_nbuf_len(msdu);
		if (packet_len < download_len) {
			/*
			 * This case of packet length being less than the
			 * nominal download length can happen for a couple
			 * of reasons:
			 * In HL, the nominal download length is a large
			 * artificial value.
			 * In LL, the frame may not have the optional header
			 * fields accounted for in the nominal download size
			 * (LLC/SNAP header, IPv4 or IPv6 header).
			 */
			download_len = packet_len;
		}

		not_accepted =
			htc_send_data_pkt(pdev->htc_pdev, msdu,
					  pdev->htc_endpoint,
					  download_len);
		if (not_accepted) {
			HTT_TX_NBUF_QUEUE_INSERT_HEAD(pdev, msdu);
			return;
		}
		HTT_TX_NBUF_QUEUE_REMOVE(pdev, msdu);
	}
}

int htt_tx_send_std(htt_pdev_handle pdev, cdf_nbuf_t msdu, uint16_t msdu_id)
{

	int download_len = pdev->download_len;

	int packet_len;

	/* packet length includes HTT tx desc frag added above */
	packet_len = cdf_nbuf_len(msdu);
	if (packet_len < download_len) {
		/*
		 * This case of packet length being less than the nominal
		 * download length can happen for a couple of reasons:
		 * In HL, the nominal download length is a large artificial
		 * value.
		 * In LL, the frame may not have the optional header fields
		 * accounted for in the nominal download size (LLC/SNAP header,
		 * IPv4 or IPv6 header).
		 */
		download_len = packet_len;
	}

	NBUF_UPDATE_TX_PKT_COUNT(msdu, NBUF_TX_PKT_HTT);
	DPTRACE(cdf_dp_trace(msdu, CDF_DP_TRACE_HTT_PACKET_PTR_RECORD,
				(uint8_t *)(cdf_nbuf_data(msdu)),
				sizeof(cdf_nbuf_data(msdu))));
	if (cdf_nbuf_queue_len(&pdev->txnbufq) > 0) {
		HTT_TX_NBUF_QUEUE_ADD(pdev, msdu);
		htt_tx_sched(pdev);
		return 0;
	}

	cdf_nbuf_trace_update(msdu, "HT:T:");
	if (htc_send_data_pkt
		    (pdev->htc_pdev, msdu, pdev->htc_endpoint, download_len)) {
		HTT_TX_NBUF_QUEUE_ADD(pdev, msdu);
	}

	return 0;               /* success */

}

cdf_nbuf_t
htt_tx_send_batch(htt_pdev_handle pdev, cdf_nbuf_t head_msdu, int num_msdus)
{
	cdf_print("*** %s curently only applies for HL systems\n", __func__);
	cdf_assert(0);
	return head_msdu;

}

int
htt_tx_send_nonstd(htt_pdev_handle pdev,
		   cdf_nbuf_t msdu,
		   uint16_t msdu_id, enum htt_pkt_type pkt_type)
{
	int download_len;

	/*
	 * The pkt_type could be checked to see what L2 header type is present,
	 * and then the L2 header could be examined to determine its length.
	 * But for simplicity, just use the maximum possible header size,
	 * rather than computing the actual header size.
	 */
	download_len = sizeof(struct htt_host_tx_desc_t)
		+ HTT_TX_HDR_SIZE_OUTER_HDR_MAX /* worst case */
		+ HTT_TX_HDR_SIZE_802_1Q
		+ HTT_TX_HDR_SIZE_LLC_SNAP
		+ ol_cfg_tx_download_size(pdev->ctrl_pdev);
	cdf_assert(download_len <= pdev->download_len);
	return htt_tx_send_std(pdev, msdu, msdu_id);
}

#else                           /*ATH_11AC_TXCOMPACT */

#ifdef QCA_TX_HTT2_SUPPORT
static inline HTC_ENDPOINT_ID
htt_tx_htt2_get_ep_id(htt_pdev_handle pdev, cdf_nbuf_t msdu)
{
	/*
	 * TX HTT2 service mainly for small sized frame and check if
	 * this candidate frame allow or not.
	 */
	if ((pdev->htc_tx_htt2_endpoint != ENDPOINT_UNUSED) &&
	    cdf_nbuf_get_tx_parallel_dnload_frm(msdu) &&
	    (cdf_nbuf_len(msdu) < pdev->htc_tx_htt2_max_size))
		return pdev->htc_tx_htt2_endpoint;
	else
		return pdev->htc_endpoint;
}
#else
#define htt_tx_htt2_get_ep_id(pdev, msdu)     (pdev->htc_endpoint)
#endif /* QCA_TX_HTT2_SUPPORT */

static inline int
htt_tx_send_base(htt_pdev_handle pdev,
		 cdf_nbuf_t msdu,
		 uint16_t msdu_id, int download_len, uint8_t more_data)
{
	struct htt_host_tx_desc_t *htt_host_tx_desc;
	struct htt_htc_pkt *pkt;
	int packet_len;
	HTC_ENDPOINT_ID ep_id;

	/*
	 * The HTT tx descriptor was attached as the prefix fragment to the
	 * msdu netbuf during the call to htt_tx_desc_init.
	 * Retrieve it so we can provide its HTC header space to HTC.
	 */
	htt_host_tx_desc = (struct htt_host_tx_desc_t *)
			   cdf_nbuf_get_frag_vaddr(msdu, 0);

	pkt = htt_htc_pkt_alloc(pdev);
	if (!pkt)
		return -ENOBUFS;       /* failure */

	pkt->msdu_id = msdu_id;
	pkt->pdev_ctxt = pdev->txrx_pdev;

	/* packet length includes HTT tx desc frag added above */
	packet_len = cdf_nbuf_len(msdu);
	if (packet_len < download_len) {
		/*
		 * This case of packet length being less than the nominal
		 * download length can happen for a couple reasons:
		 * In HL, the nominal download length is a large artificial
		 * value.
		 * In LL, the frame may not have the optional header fields
		 * accounted for in the nominal download size (LLC/SNAP header,
		 * IPv4 or IPv6 header).
		 */
		download_len = packet_len;
	}

	ep_id = htt_tx_htt2_get_ep_id(pdev, msdu);

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			       pdev->tx_send_complete_part2,
			       (unsigned char *)htt_host_tx_desc,
			       download_len - HTC_HDR_LENGTH,
			       ep_id,
			       1); /* tag - not relevant here */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msdu);

	cdf_nbuf_trace_update(msdu, "HT:T:");
	NBUF_UPDATE_TX_PKT_COUNT(msdu, NBUF_TX_PKT_HTT);
	DPTRACE(cdf_dp_trace(msdu, CDF_DP_TRACE_HTT_PACKET_PTR_RECORD,
				(uint8_t *)(cdf_nbuf_data(msdu)),
				sizeof(cdf_nbuf_data(msdu))));
	htc_send_data_pkt(pdev->htc_pdev, &pkt->htc_pkt, more_data);

	return 0;               /* success */
}

cdf_nbuf_t
htt_tx_send_batch(htt_pdev_handle pdev, cdf_nbuf_t head_msdu, int num_msdus)
{
	cdf_nbuf_t rejected = NULL;
	uint16_t *msdu_id_storage;
	uint16_t msdu_id;
	cdf_nbuf_t msdu;
	/*
	 * FOR NOW, iterate through the batch, sending the frames singly.
	 * Eventually HTC and HIF should be able to accept a batch of
	 * data frames rather than singles.
	 */
	msdu = head_msdu;
	while (num_msdus--) {
		cdf_nbuf_t next_msdu = cdf_nbuf_next(msdu);
		msdu_id_storage = ol_tx_msdu_id_storage(msdu);
		msdu_id = *msdu_id_storage;

		/* htt_tx_send_base returns 0 as success and 1 as failure */
		if (htt_tx_send_base(pdev, msdu, msdu_id, pdev->download_len,
				     num_msdus)) {
			cdf_nbuf_set_next(msdu, rejected);
			rejected = msdu;
		}
		msdu = next_msdu;
	}
	return rejected;
}

int
htt_tx_send_nonstd(htt_pdev_handle pdev,
		   cdf_nbuf_t msdu,
		   uint16_t msdu_id, enum htt_pkt_type pkt_type)
{
	int download_len;

	/*
	 * The pkt_type could be checked to see what L2 header type is present,
	 * and then the L2 header could be examined to determine its length.
	 * But for simplicity, just use the maximum possible header size,
	 * rather than computing the actual header size.
	 */
	download_len = sizeof(struct htt_host_tx_desc_t)
		+ HTT_TX_HDR_SIZE_OUTER_HDR_MAX      /* worst case */
		+ HTT_TX_HDR_SIZE_802_1Q
		+ HTT_TX_HDR_SIZE_LLC_SNAP
		+ ol_cfg_tx_download_size(pdev->ctrl_pdev);
	return htt_tx_send_base(pdev, msdu, msdu_id, download_len, 0);
}

int htt_tx_send_std(htt_pdev_handle pdev, cdf_nbuf_t msdu, uint16_t msdu_id)
{
	return htt_tx_send_base(pdev, msdu, msdu_id, pdev->download_len, 0);
}

#endif /*ATH_11AC_TXCOMPACT */
#ifdef HTT_DBG
void htt_tx_desc_display(void *tx_desc)
{
	struct htt_tx_msdu_desc_t *htt_tx_desc;

	htt_tx_desc = (struct htt_tx_msdu_desc_t *)tx_desc;

	/* only works for little-endian */
	cdf_print("HTT tx desc (@ %p):\n", htt_tx_desc);
	cdf_print("  msg type = %d\n", htt_tx_desc->msg_type);
	cdf_print("  pkt subtype = %d\n", htt_tx_desc->pkt_subtype);
	cdf_print("  pkt type = %d\n", htt_tx_desc->pkt_type);
	cdf_print("  vdev ID = %d\n", htt_tx_desc->vdev_id);
	cdf_print("  ext TID = %d\n", htt_tx_desc->ext_tid);
	cdf_print("  postponed = %d\n", htt_tx_desc->postponed);
#if HTT_PADDR64
	cdf_print("  reserved_dword0_bits28 = %d\n", htt_tx_desc->reserved_dword0_bits28);
	cdf_print("  cksum_offload = %d\n", htt_tx_desc->cksum_offload);
	cdf_print("  tx_compl_req= %d\n", htt_tx_desc->tx_compl_req);
#else /* !HTT_PADDR64 */
	cdf_print("  batch more = %d\n", htt_tx_desc->more_in_batch);
#endif /* HTT_PADDR64 */
	cdf_print("  length = %d\n", htt_tx_desc->len);
	cdf_print("  id = %d\n", htt_tx_desc->id);
#if HTT_PADDR64
	cdf_print("  frag desc addr.lo = %#x\n",
		  htt_tx_desc->frags_desc_ptr.lo);
	cdf_print("  frag desc addr.hi = %#x\n",
		  htt_tx_desc->frags_desc_ptr.hi);
	cdf_print("  peerid = %d\n", htt_tx_desc->peerid);
	cdf_print("  chanfreq = %d\n", htt_tx_desc->chanfreq);
#else /* ! HTT_PADDR64 */
	cdf_print("  frag desc addr = %#x\n", htt_tx_desc->frags_desc_ptr);
#endif /* HTT_PADDR64 */
}
#endif

#ifdef IPA_OFFLOAD
int htt_tx_ipa_uc_attach(struct htt_pdev_t *pdev,
			 unsigned int uc_tx_buf_sz,
			 unsigned int uc_tx_buf_cnt,
			 unsigned int uc_tx_partition_base)
{
	unsigned int tx_buffer_count;
	cdf_nbuf_t buffer_vaddr;
	uint32_t buffer_paddr;
	uint32_t *header_ptr;
	uint32_t *ring_vaddr;
	int return_code = 0;
	unsigned int tx_comp_ring_size;

	/* Allocate CE Write Index WORD */
	pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr =
		cdf_os_mem_alloc_consistent(
			pdev->osdev,
			4,
			&pdev->ipa_uc_tx_rsc.tx_ce_idx.paddr,
			cdf_get_dma_mem_context(
				(&pdev->ipa_uc_tx_rsc.tx_ce_idx),
				memctx));
	if (!pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr) {
		cdf_print("%s: CE Write Index WORD alloc fail", __func__);
		return -ENOBUFS;
	}

	/* Allocate TX COMP Ring */
	tx_comp_ring_size = uc_tx_buf_cnt * sizeof(cdf_nbuf_t);
	pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr =
		cdf_os_mem_alloc_consistent(
			pdev->osdev,
			tx_comp_ring_size,
			&pdev->ipa_uc_tx_rsc.tx_comp_base.paddr,
			cdf_get_dma_mem_context((&pdev->ipa_uc_tx_rsc.
						 tx_comp_base),
						memctx));
	if (!pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr) {
		cdf_print("%s: TX COMP ring alloc fail", __func__);
		return_code = -ENOBUFS;
		goto free_tx_ce_idx;
	}

	cdf_mem_zero(pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr, tx_comp_ring_size);

	/* Allocate TX BUF vAddress Storage */
	pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg =
		(cdf_nbuf_t *) cdf_mem_malloc(uc_tx_buf_cnt *
					      sizeof(cdf_nbuf_t));
	if (!pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg) {
		cdf_print("%s: TX BUF POOL vaddr storage alloc fail", __func__);
		return_code = -ENOBUFS;
		goto free_tx_comp_base;
	}
	cdf_mem_zero(pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg,
		     uc_tx_buf_cnt * sizeof(cdf_nbuf_t));

	ring_vaddr = pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr;
	/* Allocate TX buffers as many as possible */
	for (tx_buffer_count = 0;
	     tx_buffer_count < (uc_tx_buf_cnt - 1); tx_buffer_count++) {
		buffer_vaddr = cdf_nbuf_alloc(pdev->osdev,
					      uc_tx_buf_sz, 0, 4, false);
		if (!buffer_vaddr) {
			cdf_print("%s: TX BUF alloc fail, loop index: %d",
				  __func__, tx_buffer_count);
			return 0;
		}

		/* Init buffer */
		cdf_mem_zero(cdf_nbuf_data(buffer_vaddr), uc_tx_buf_sz);
		header_ptr = (uint32_t *) cdf_nbuf_data(buffer_vaddr);

		/* HTT control header */
		*header_ptr = HTT_IPA_UC_OFFLOAD_TX_HEADER_DEFAULT;
		header_ptr++;

		/* PKT ID */
		*header_ptr |= ((uint16_t) uc_tx_partition_base +
				tx_buffer_count) << 16;

		cdf_nbuf_map(pdev->osdev, buffer_vaddr, CDF_DMA_BIDIRECTIONAL);
		buffer_paddr = cdf_nbuf_get_frag_paddr_lo(buffer_vaddr, 0);
		header_ptr++;

		/* Frag Desc Pointer */
		/* 64bits descriptor, Low 32bits */
		*header_ptr = (uint32_t) (buffer_paddr + 20);
		header_ptr++;

		/* 64bits descriptor, high 32bits */
		*header_ptr = 0;
		header_ptr++;

		/* chanreq, peerid */
		*header_ptr = 0xFFFFFFFF;

		/* FRAG Header */
		/* 6 words TSO header */
		header_ptr += 6;
		*header_ptr = buffer_paddr + 64;

		*ring_vaddr = buffer_paddr;
		pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[tx_buffer_count] =
			buffer_vaddr;
		/* Memory barrier to ensure actual value updated */

		ring_vaddr += 2;
	}

	pdev->ipa_uc_tx_rsc.alloc_tx_buf_cnt = tx_buffer_count;

	return 0;

free_tx_comp_base:
	cdf_os_mem_free_consistent(pdev->osdev,
				   ol_cfg_ipa_uc_tx_max_buf_cnt(pdev->
								ctrl_pdev) * 4,
				   pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr,
				   pdev->ipa_uc_tx_rsc.tx_comp_base.paddr,
				   cdf_get_dma_mem_context((&pdev->
							    ipa_uc_tx_rsc.
							    tx_comp_base),
							   memctx));
free_tx_ce_idx:
	cdf_os_mem_free_consistent(pdev->osdev,
				   4,
				   pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr,
				   pdev->ipa_uc_tx_rsc.tx_ce_idx.paddr,
				   cdf_get_dma_mem_context((&pdev->
							    ipa_uc_tx_rsc.
							    tx_ce_idx),
							   memctx));
	return return_code;
}

int htt_tx_ipa_uc_detach(struct htt_pdev_t *pdev)
{
	uint16_t idx;

	if (pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr) {
		cdf_os_mem_free_consistent(
			pdev->osdev,
			4,
			pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr,
			pdev->ipa_uc_tx_rsc.tx_ce_idx.paddr,
			cdf_get_dma_mem_context(
				(&pdev->ipa_uc_tx_rsc.tx_ce_idx),
				memctx));
	}

	if (pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr) {
		cdf_os_mem_free_consistent(
			pdev->osdev,
			ol_cfg_ipa_uc_tx_max_buf_cnt(pdev->ctrl_pdev) * 4,
			pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr,
			pdev->ipa_uc_tx_rsc.tx_comp_base.paddr,
			cdf_get_dma_mem_context((&pdev->ipa_uc_tx_rsc.
						 tx_comp_base),
						memctx));
	}

	/* Free each single buffer */
	for (idx = 0; idx < pdev->ipa_uc_tx_rsc.alloc_tx_buf_cnt; idx++) {
		if (pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx]) {
			cdf_nbuf_unmap(pdev->osdev,
				       pdev->ipa_uc_tx_rsc.
				       tx_buf_pool_vaddr_strg[idx],
				       CDF_DMA_FROM_DEVICE);
			cdf_nbuf_free(pdev->ipa_uc_tx_rsc.
				      tx_buf_pool_vaddr_strg[idx]);
		}
	}

	/* Free storage */
	cdf_mem_free(pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg);

	return 0;
}
#endif /* IPA_OFFLOAD */

#if defined(FEATURE_TSO)
void
htt_tx_desc_fill_tso_info(htt_pdev_handle pdev, void *desc,
	 struct cdf_tso_info_t *tso_info)
{
	u_int32_t *word;
	int i;
	struct cdf_tso_seg_elem_t *tso_seg = tso_info->curr_seg;
	struct msdu_ext_desc_t *msdu_ext_desc = (struct msdu_ext_desc_t *)desc;

	word = (u_int32_t *)(desc);

	/* Initialize the TSO flags per MSDU */
	((struct msdu_ext_desc_t *)msdu_ext_desc)->tso_flags =
		 tso_seg->seg.tso_flags;

	/* First 24 bytes (6*4) contain the TSO flags */
	word += 6;

	for (i = 0; i < tso_seg->seg.num_frags; i++) {
		/* [31:0] first 32 bits of the buffer pointer  */
		*word = tso_seg->seg.tso_frags[i].paddr_low_32;
		word++;
		/* [15:0] the upper 16 bits of the first buffer pointer */
		/* [31:16] length of the first buffer */
		*word = (tso_seg->seg.tso_frags[i].length << 16);
		word++;
	}

	if (tso_seg->seg.num_frags < FRAG_NUM_MAX) {
		*word = 0;
		word++;
		*word = 0;
	}
}
#endif /* FEATURE_TSO */
