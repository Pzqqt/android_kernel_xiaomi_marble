/*
 * Copyright (c) 2011, 2014-2017 The Linux Foundation. All rights reserved.
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
#include <qdf_types.h>          /* qdf_dma_addr_t */
#include <qdf_mem.h>         /* qdf_mem_alloc_consistent et al */
#include <qdf_nbuf.h>           /* qdf_nbuf_t, etc. */
#include <qdf_time.h>           /* qdf_mdelay */

#include <htt.h>                /* htt_tx_msdu_desc_t */
#include <htc.h>                /* HTC_HDR_LENGTH */
#include <htc_api.h>            /* htc_flush_surprise_remove */
#include <ol_cfg.h>             /* ol_cfg_netbuf_frags_max, etc. */
#include <ol_htt_tx_api.h>      /* HTT_TX_DESC_VADDR_OFFSET */
#include <ol_txrx_htt_api.h>    /* ol_tx_msdu_id_storage */
#include <ol_txrx_internal.h>
#include <htt_internal.h>

#include <cds_utils.h>

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

#ifdef QCA_WIFI_3_0
#define IPA_UC_TX_BUF_FRAG_DESC_OFFSET 20
#define IPA_UC_TX_BUF_FRAG_HDR_OFFSET  64
#define IPA_UC_TX_BUF_TSO_HDR_SIZE     6
#define IPA_UC_TX_BUF_PADDR_HI_MASK    0x0000001F
#define IPA_UC_TX_BUF_PADDR_HI_OFFSET  32
#else
#define IPA_UC_TX_BUF_FRAG_DESC_OFFSET 16
#define IPA_UC_TX_BUF_FRAG_HDR_OFFSET  32
#endif /* QCA_WIFI_3_0 */

#if HTT_PADDR64
#define HTT_TX_DESC_FRAG_FIELD_HI_UPDATE(frag_filed_ptr)                       \
do {                                                                           \
	frag_filed_ptr++;                                                      \
	/* frags_desc_ptr.hi */                                                \
	*frag_filed_ptr = 0;                                                   \
} while (0)
#else
#define HTT_TX_DESC_FRAG_FIELD_HI_UPDATE(frag_filed_ptr) {}
#endif

/*--- setup / tear-down functions -------------------------------------------*/

static qdf_dma_addr_t htt_tx_get_paddr(htt_pdev_handle pdev,
				char *target_vaddr);

#ifdef HELIUMPLUS
/**
 * htt_tx_desc_get_size() - get tx descripotrs size
 * @pdev:	htt device instance pointer
 *
 * This function will get HTT TX descriptor size and fragment descriptor size
 *
 * Return: None
 */
static void htt_tx_desc_get_size(struct htt_pdev_t *pdev)
{
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
}

/**
 * htt_tx_frag_desc_field_update() - Update fragment descriptor field
 * @pdev:	htt device instance pointer
 * @fptr:	Fragment descriptor field pointer
 * @index:	Descriptor index to find page and offset
 * @desc_v_ptr:	descriptor virtual pointot to find offset
 *
 * This function will update fragment descriptor field with actual fragment
 * descriptor stating physical pointer
 *
 * Return: None
 */
static void htt_tx_frag_desc_field_update(struct htt_pdev_t *pdev,
		uint32_t *fptr, unsigned int index,
		struct htt_tx_msdu_desc_t *desc_v_ptr)
{
	unsigned int target_page;
	unsigned int offset;
	struct qdf_mem_dma_page_t *dma_page;

	target_page = index / pdev->frag_descs.desc_pages.num_element_per_page;
	offset = index % pdev->frag_descs.desc_pages.num_element_per_page;
	dma_page = &pdev->frag_descs.desc_pages.dma_pages[target_page];
	*fptr = (uint32_t)(dma_page->page_p_addr +
		offset * pdev->frag_descs.size);
	HTT_TX_DESC_FRAG_FIELD_HI_UPDATE(fptr);
	return;
}

/**
 * htt_tx_frag_desc_attach() - Attach fragment descriptor
 * @pdev:		htt device instance pointer
 * @desc_pool_elems:	Number of fragment descriptor
 *
 * This function will allocate fragment descriptor
 *
 * Return: 0 success
 */
static int htt_tx_frag_desc_attach(struct htt_pdev_t *pdev,
	uint16_t desc_pool_elems)
{
	pdev->frag_descs.pool_elems = desc_pool_elems;
	qdf_mem_multi_pages_alloc(pdev->osdev, &pdev->frag_descs.desc_pages,
		pdev->frag_descs.size, desc_pool_elems,
		qdf_get_dma_mem_context((&pdev->frag_descs), memctx), false);
	if ((0 == pdev->frag_descs.desc_pages.num_pages) ||
		(NULL == pdev->frag_descs.desc_pages.dma_pages)) {
		ol_txrx_err("FRAG descriptor alloc fail");
		return -ENOBUFS;
	}
	return 0;
}

/**
 * htt_tx_frag_desc_detach() - Detach fragment descriptor
 * @pdev:		htt device instance pointer
 *
 * This function will free fragment descriptor
 *
 * Return: None
 */
static void htt_tx_frag_desc_detach(struct htt_pdev_t *pdev)
{
	qdf_mem_multi_pages_free(pdev->osdev, &pdev->frag_descs.desc_pages,
		qdf_get_dma_mem_context((&pdev->frag_descs), memctx), false);
}

/**
 * htt_tx_frag_alloc() - Allocate single fragment descriptor from the pool
 * @pdev:		htt device instance pointer
 * @index:		Descriptor index
 * @frag_paddr:	        Fragment descriptor physical address
 * @frag_ptr:		Fragment descriptor virtual address
 *
 * This function will free fragment descriptor
 *
 * Return: None
 */
int htt_tx_frag_alloc(htt_pdev_handle pdev,
	u_int16_t index, qdf_dma_addr_t *frag_paddr, void **frag_ptr)
{
	uint16_t frag_page_index;
	uint16_t frag_elem_index;
	struct qdf_mem_dma_page_t *dma_page;

	/** Index should never be 0, since its used by the hardware
	    to terminate the link. */
	if (index >= pdev->tx_descs.pool_elems) {
		*frag_ptr = NULL;
		return 1;
	}

	frag_page_index = index /
		pdev->frag_descs.desc_pages.num_element_per_page;
	frag_elem_index = index %
		pdev->frag_descs.desc_pages.num_element_per_page;
	dma_page = &pdev->frag_descs.desc_pages.dma_pages[frag_page_index];

	*frag_ptr = dma_page->page_v_addr_start +
		frag_elem_index * pdev->frag_descs.size;
	if (((char *)(*frag_ptr) < dma_page->page_v_addr_start) ||
		((char *)(*frag_ptr) > dma_page->page_v_addr_end)) {
		*frag_ptr = NULL;
		return 1;
	}

	*frag_paddr = dma_page->page_p_addr +
		frag_elem_index * pdev->frag_descs.size;
	return 0;
}
#else

/**
 * htt_tx_desc_get_size() - get tx descripotrs size
 * @pdev:	htt device instance pointer
 *
 * This function will get HTT TX descriptor size and fragment descriptor size
 *
 * Return: None
 */
static inline void htt_tx_desc_get_size(struct htt_pdev_t *pdev)
{
	if (pdev->cfg.is_high_latency) {
		pdev->tx_descs.size = sizeof(struct htt_host_tx_desc_t);
	} else {
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
	}
}

#ifndef CONFIG_HL_SUPPORT

/**
 * htt_tx_frag_desc_field_update() - Update fragment descriptor field
 * @pdev:	htt device instance pointer
 * @fptr:	Fragment descriptor field pointer
 * @index:	Descriptor index to find page and offset
 * @desc_v_ptr:	descriptor virtual pointot to find offset
 *
 * This function will update fragment descriptor field with actual fragment
 * descriptor stating physical pointer
 *
 * Return: None
 */
static void htt_tx_frag_desc_field_update(struct htt_pdev_t *pdev,
		uint32_t *fptr, unsigned int index,
		struct htt_tx_msdu_desc_t *desc_v_ptr)
{
	*fptr = (uint32_t)htt_tx_get_paddr(pdev, (char *)desc_v_ptr) +
		HTT_TX_DESC_LEN;
}
#endif

/**
 * htt_tx_frag_desc_attach() - Attach fragment descriptor
 * @pdev:	htt device instance pointer
 * @desc_pool_elems:	Number of fragment descriptor
 *
 * This function will allocate fragment descriptor
 *
 * Return: 0 success
 */
static inline int htt_tx_frag_desc_attach(struct htt_pdev_t *pdev,
	int desc_pool_elems)
{
	return 0;
}

/**
 * htt_tx_frag_desc_detach() - Detach fragment descriptor
 * @pdev:		htt device instance pointer
 *
 * This function will free fragment descriptor
 *
 * Return: None
 */
static void htt_tx_frag_desc_detach(struct htt_pdev_t *pdev) {}
#endif /* HELIUMPLUS */

#ifdef CONFIG_HL_SUPPORT

/**
 * htt_tx_attach() - Attach HTT device instance
 * @pdev:		htt device instance pointer
 * @desc_pool_elems:	Number of TX descriptors
 *
 * This function will allocate HTT TX resources
 *
 * Return: 0 Success
 */
int htt_tx_attach(struct htt_pdev_t *pdev, int desc_pool_elems)
{
	int i, i_int, pool_size;
	uint32_t **p;
	uint32_t num_link = 0;
	uint16_t num_page, num_desc_per_page;
	void **cacheable_pages = NULL;

	htt_tx_desc_get_size(pdev);

	/*
	 * Make sure tx_descs.size is a multiple of 4-bytes.
	 * It should be, but round up just to be sure.
	 */
	pdev->tx_descs.size = (pdev->tx_descs.size + 3) & (~0x3);

	pdev->tx_descs.pool_elems = desc_pool_elems;
	pdev->tx_descs.alloc_cnt = 0;
	pool_size = pdev->tx_descs.pool_elems * pdev->tx_descs.size;
	qdf_mem_multi_pages_alloc(pdev->osdev, &pdev->tx_descs.desc_pages,
				  pdev->tx_descs.size,
				  pdev->tx_descs.pool_elems,
				  qdf_get_dma_mem_context((&pdev->tx_descs),
							  memctx), true);
	if ((0 == pdev->tx_descs.desc_pages.num_pages) ||
	    (NULL == pdev->tx_descs.desc_pages.cacheable_pages)) {
		ol_txrx_err("HTT desc alloc fail");
		goto out_fail;
	}
	num_page = pdev->tx_descs.desc_pages.num_pages;
	num_desc_per_page = pdev->tx_descs.desc_pages.num_element_per_page;

	/* link tx descriptors into a freelist */
	cacheable_pages = pdev->tx_descs.desc_pages.cacheable_pages;

	pdev->tx_descs.freelist = (uint32_t *)cacheable_pages[0];
	p = (uint32_t **)pdev->tx_descs.freelist;
	for (i = 0; i < num_page; i++) {
		for (i_int = 0; i_int < num_desc_per_page; i_int++) {
			if (i_int == (num_desc_per_page - 1)) {
				/*
				 * Last element on this page,
				 * should point next page
				 */
				if (!cacheable_pages[i + 1]) {
					ol_txrx_err("over flow num link %d\n",
						   num_link);
					goto free_htt_desc;
				}
				*p = (uint32_t *)cacheable_pages[i + 1];
			} else {
				*p = (uint32_t *)
					(((char *)p) + pdev->tx_descs.size);
			}
			num_link++;
			p = (uint32_t **) *p;
			/* Last link established exit */
			if (num_link == (pdev->tx_descs.pool_elems - 1))
				break;
		}
	}
	*p = NULL;

	if (htt_tx_frag_desc_attach(pdev, desc_pool_elems)) {
		ol_txrx_err("HTT Frag descriptor alloc fail");
		goto free_htt_desc;
	}

	/* success */
	return 0;

free_htt_desc:
	qdf_mem_multi_pages_free(pdev->osdev, &pdev->tx_descs.desc_pages,
				 qdf_get_dma_mem_context((&pdev->tx_descs),
							 memctx), true);
out_fail:
	return -ENOBUFS;
}

void htt_tx_detach(struct htt_pdev_t *pdev)
{
	if (!pdev) {
		qdf_print("htt tx detach invalid instance");
		return;
	}

	htt_tx_frag_desc_detach(pdev);
	qdf_mem_multi_pages_free(pdev->osdev, &pdev->tx_descs.desc_pages,
				 qdf_get_dma_mem_context((&pdev->tx_descs),
							 memctx), true);
}

/**
 * htt_tx_set_frag_desc_addr() - set up the fragmentation descriptor address
 * @pdev: pointer to the HTT instance making the allocation
 * @htt_tx_desc: Host tx decriptor that does not include HTC hdr
 * @index: index to alloc htt tx desc
 *
 *
 * Return: None
 */
static inline void
htt_tx_set_frag_desc_addr(struct htt_pdev_t *pdev,
			  struct htt_tx_msdu_desc_t *htt_tx_desc,
			  uint16_t index)
{
	return;
}

/**
 * htt_tx_desc_frags_table_set() - set up the descriptor and payload
 *				   to correspondinf fragments
 * @pdev: pointer to the HTT instance making the allocation
 * @htt_tx_desc: Host tx decriptor that does not include HTC hdr
 * @paddr: fragment physical address
 * @frag_desc_paddr_lo: frag descriptor address
 * @reset: reset
 *
 * Return: None
 */
void htt_tx_desc_frags_table_set(htt_pdev_handle pdev,
				 void *desc,
				 qdf_dma_addr_t paddr,
				 qdf_dma_addr_t frag_desc_paddr,
				 int reset)
{
	/* fragments table only applies to LL systems */
	return;
}

/**
 * htt_tx_credit_update() - get the number of credits by which the amount of
 *			    target credits needs to be updated
 * @pdev: htt context
 *
 * Return: number of credits
 */
int htt_tx_credit_update(struct htt_pdev_t *pdev)
{
	int credit_delta;
	credit_delta = QDF_MIN(qdf_atomic_read(
			&pdev->htt_tx_credit.target_delta),
			qdf_atomic_read(&pdev->htt_tx_credit.bus_delta));
	if (credit_delta) {
		qdf_atomic_add(-credit_delta,
			       &pdev->htt_tx_credit.target_delta);
		qdf_atomic_add(-credit_delta,
			       &pdev->htt_tx_credit.bus_delta);
	}
	return credit_delta;
}

/**
 * htt_tx_get_paddr() - get physical address for htt desc
 *
 * Get HTT descriptor physical address from virtaul address
 * Find page first and find offset
 * Not required for HL systems
 *
 * Return: Physical address of descriptor
 */
static inline
qdf_dma_addr_t htt_tx_get_paddr(htt_pdev_handle pdev,
				char *target_vaddr)
{
	return 0;
}


#else

int htt_tx_attach(struct htt_pdev_t *pdev, int desc_pool_elems)
{
	int i, i_int, pool_size;
	uint32_t **p;
	struct qdf_mem_dma_page_t *page_info;
	uint32_t num_link = 0;
	uint16_t num_page, num_desc_per_page;

	htt_tx_desc_get_size(pdev);

	/*
	 * Make sure tx_descs.size is a multiple of 4-bytes.
	 * It should be, but round up just to be sure.
	 */
	pdev->tx_descs.size = (pdev->tx_descs.size + 3) & (~0x3);

	pdev->tx_descs.pool_elems = desc_pool_elems;
	pdev->tx_descs.alloc_cnt = 0;
	pool_size = pdev->tx_descs.pool_elems * pdev->tx_descs.size;
	qdf_mem_multi_pages_alloc(pdev->osdev, &pdev->tx_descs.desc_pages,
		pdev->tx_descs.size, pdev->tx_descs.pool_elems,
		qdf_get_dma_mem_context((&pdev->tx_descs), memctx), false);
	if ((0 == pdev->tx_descs.desc_pages.num_pages) ||
		(NULL == pdev->tx_descs.desc_pages.dma_pages)) {
		ol_txrx_err("HTT desc alloc fail");
		goto out_fail;
	}
	num_page = pdev->tx_descs.desc_pages.num_pages;
	num_desc_per_page = pdev->tx_descs.desc_pages.num_element_per_page;

	/* link tx descriptors into a freelist */
	page_info = pdev->tx_descs.desc_pages.dma_pages;
	pdev->tx_descs.freelist = (uint32_t *)page_info->page_v_addr_start;
	p = (uint32_t **) pdev->tx_descs.freelist;
	for (i = 0; i < num_page; i++) {
		for (i_int = 0; i_int < num_desc_per_page; i_int++) {
			if (i_int == (num_desc_per_page - 1)) {
				/*
				 * Last element on this page,
				 * should pint next page */
				if (!page_info->page_v_addr_start) {
					ol_txrx_err("over flow num link %d\n",
						num_link);
					goto free_htt_desc;
				}
				page_info++;
				*p = (uint32_t *)page_info->page_v_addr_start;
			} else {
				*p = (uint32_t *)
					(((char *) p) + pdev->tx_descs.size);
			}
			num_link++;
			p = (uint32_t **) *p;
			/* Last link established exit */
			if (num_link == (pdev->tx_descs.pool_elems - 1))
				break;
		}
	}
	*p = NULL;

	if (htt_tx_frag_desc_attach(pdev, desc_pool_elems)) {
		ol_txrx_err("HTT Frag descriptor alloc fail");
		goto free_htt_desc;
	}

	/* success */
	return 0;

free_htt_desc:
	qdf_mem_multi_pages_free(pdev->osdev, &pdev->tx_descs.desc_pages,
		qdf_get_dma_mem_context((&pdev->tx_descs), memctx), false);
out_fail:
	return -ENOBUFS;
}

void htt_tx_detach(struct htt_pdev_t *pdev)
{
	if (!pdev) {
		qdf_print("htt tx detach invalid instance");
		return;
	}

	htt_tx_frag_desc_detach(pdev);
	qdf_mem_multi_pages_free(pdev->osdev, &pdev->tx_descs.desc_pages,
		qdf_get_dma_mem_context((&pdev->tx_descs), memctx), false);
}

static void
htt_tx_set_frag_desc_addr(struct htt_pdev_t *pdev,
			  struct htt_tx_msdu_desc_t *htt_tx_desc,
			  uint16_t index)
{
	uint32_t *fragmentation_descr_field_ptr;
	fragmentation_descr_field_ptr = (uint32_t *)
		((uint32_t *)htt_tx_desc) +
		HTT_TX_DESC_FRAGS_DESC_PADDR_OFFSET_DWORD;
	/*
	 * The fragmentation descriptor is allocated from consistent
	 * memory. Therefore, we can use the address directly rather
	 * than having to map it from a virtual/CPU address to a
	 * physical/bus address.
	 */
	htt_tx_frag_desc_field_update(pdev, fragmentation_descr_field_ptr,
				      index, htt_tx_desc);

		return;
}

void htt_tx_desc_frags_table_set(htt_pdev_handle pdev,
				 void *htt_tx_desc,
				 qdf_dma_addr_t paddr,
				 qdf_dma_addr_t frag_desc_paddr,
				 int reset)
{
	uint32_t *fragmentation_descr_field_ptr;

	fragmentation_descr_field_ptr = (uint32_t *)
		((uint32_t *) htt_tx_desc) +
		HTT_TX_DESC_FRAGS_DESC_PADDR_OFFSET_DWORD;
	if (reset) {
#if defined(HELIUMPLUS)
		*fragmentation_descr_field_ptr = frag_desc_paddr;
#else
		*fragmentation_descr_field_ptr =
			htt_tx_get_paddr(pdev, htt_tx_desc) + HTT_TX_DESC_LEN;
#endif
	} else {
		*fragmentation_descr_field_ptr = paddr;
	}
}

void htt_tx_pending_discard(htt_pdev_handle pdev)
{
	htc_flush_surprise_remove(pdev->htc_pdev);
}

static qdf_dma_addr_t htt_tx_get_paddr(htt_pdev_handle pdev,
				char *target_vaddr)
{
	uint16_t i;
	struct qdf_mem_dma_page_t *page_info = NULL;
	uint64_t offset;

	for (i = 0; i < pdev->tx_descs.desc_pages.num_pages; i++) {
		page_info = pdev->tx_descs.desc_pages.dma_pages + i;
		if (!page_info->page_v_addr_start) {
			qdf_assert(0);
			return 0;
		}
		if ((target_vaddr >= page_info->page_v_addr_start) &&
			(target_vaddr <= page_info->page_v_addr_end))
			break;
	}

	if (!page_info) {
		ol_txrx_err("invalid page_info");
		return 0;
	}

	offset = (uint64_t)(target_vaddr - page_info->page_v_addr_start);
	return page_info->page_p_addr + offset;
}

#endif

/*--- descriptor allocation functions ---------------------------------------*/

void *htt_tx_desc_alloc(htt_pdev_handle pdev, qdf_dma_addr_t *paddr,
			uint16_t index)
{
	struct htt_host_tx_desc_t *htt_host_tx_desc;    /* includes HTC hdr */
	struct htt_tx_msdu_desc_t *htt_tx_desc; /* doesn't include  HTC hdr */

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
	htt_tx_set_frag_desc_addr(pdev, htt_tx_desc, index);

	/*
	 * Include the headroom for the HTC frame header when specifying the
	 * physical address for the HTT tx descriptor.
	 */
	*paddr = (qdf_dma_addr_t)htt_tx_get_paddr(pdev, (char *)htt_host_tx_desc);
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

/* PUT THESE AS inline IN ol_htt_tx_api.h */

void htt_tx_desc_flag_postponed(htt_pdev_handle pdev, void *desc)
{
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
	qdf_nbuf_t msdu;
	int download_len = pdev->download_len;
	int packet_len;

	HTT_TX_NBUF_QUEUE_REMOVE(pdev, msdu);
	while (msdu != NULL) {
		int not_accepted;
		/* packet length includes HTT tx desc frag added above */
		packet_len = qdf_nbuf_len(msdu);
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
					  pdev->htc_tx_endpoint,
					  download_len);
		if (not_accepted) {
			HTT_TX_NBUF_QUEUE_INSERT_HEAD(pdev, msdu);
			return;
		}
		HTT_TX_NBUF_QUEUE_REMOVE(pdev, msdu);
	}
}

int htt_tx_send_std(htt_pdev_handle pdev, qdf_nbuf_t msdu, uint16_t msdu_id)
{

	int download_len = pdev->download_len;

	int packet_len;

	/* packet length includes HTT tx desc frag added above */
	packet_len = qdf_nbuf_len(msdu);
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

	if (QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_EXT_HEADER(msdu))
		download_len += sizeof(struct htt_tx_msdu_desc_ext_t);


	QDF_NBUF_UPDATE_TX_PKT_COUNT(msdu, QDF_NBUF_TX_PKT_HTT);
	DPTRACE(qdf_dp_trace(msdu, QDF_DP_TRACE_HTT_PACKET_PTR_RECORD,
				QDF_TRACE_DEFAULT_PDEV_ID,
				qdf_nbuf_data_addr(msdu),
				sizeof(qdf_nbuf_data(msdu)), QDF_TX));
	if (qdf_nbuf_queue_len(&pdev->txnbufq) > 0) {
		HTT_TX_NBUF_QUEUE_ADD(pdev, msdu);
		htt_tx_sched(pdev);
		return 0;
	}

	if (htc_send_data_pkt(pdev->htc_pdev, msdu,
			      pdev->htc_tx_endpoint, download_len)) {
		HTT_TX_NBUF_QUEUE_ADD(pdev, msdu);
	}

	return 0;               /* success */

}

#ifndef CONFIG_HL_SUPPORT
#ifdef FEATURE_RUNTIME_PM
/**
 * htt_tx_resume_handler() - resume callback for the htt endpoint
 * @context: a pointer to the htt context
 *
 * runs htt_tx_sched.
 */
void htt_tx_resume_handler(void *context)
{
	struct htt_pdev_t *pdev =  (struct htt_pdev_t *) context;

	htt_tx_sched(pdev);
}
#else
void
htt_tx_resume_handler(void *context) { }
#endif
#endif

qdf_nbuf_t
htt_tx_send_batch(htt_pdev_handle pdev, qdf_nbuf_t head_msdu, int num_msdus)
{
	qdf_print("*** %s curently only applies for HL systems\n", __func__);
	qdf_assert(0);
	return head_msdu;

}

int
htt_tx_send_nonstd(htt_pdev_handle pdev,
		   qdf_nbuf_t msdu,
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
	qdf_assert(download_len <= pdev->download_len);
	return htt_tx_send_std(pdev, msdu, msdu_id);
}

#else                           /*ATH_11AC_TXCOMPACT */

#ifdef QCA_TX_HTT2_SUPPORT
static inline HTC_ENDPOINT_ID
htt_tx_htt2_get_ep_id(htt_pdev_handle pdev, qdf_nbuf_t msdu)
{
	/*
	 * TX HTT2 service mainly for small sized frame and check if
	 * this candidate frame allow or not.
	 */
	if ((pdev->htc_tx_htt2_endpoint != ENDPOINT_UNUSED) &&
	    qdf_nbuf_get_tx_parallel_dnload_frm(msdu) &&
	    (qdf_nbuf_len(msdu) < pdev->htc_tx_htt2_max_size))
		return pdev->htc_tx_htt2_endpoint;
	else
		return pdev->htc_tx_endpoint;
}
#else
#define htt_tx_htt2_get_ep_id(pdev, msdu)     (pdev->htc_tx_endpoint)
#endif /* QCA_TX_HTT2_SUPPORT */

static inline int
htt_tx_send_base(htt_pdev_handle pdev,
		 qdf_nbuf_t msdu,
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
			   qdf_nbuf_get_frag_vaddr(msdu, 0);

	pkt = htt_htc_pkt_alloc(pdev);
	if (!pkt)
		return -ENOBUFS;       /* failure */

	pkt->msdu_id = msdu_id;
	pkt->pdev_ctxt = pdev->txrx_pdev;

	/* packet length includes HTT tx desc frag added above */
	packet_len = qdf_nbuf_len(msdu);
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

	QDF_NBUF_UPDATE_TX_PKT_COUNT(msdu, QDF_NBUF_TX_PKT_HTT);
	DPTRACE(qdf_dp_trace(msdu, QDF_DP_TRACE_HTT_PACKET_PTR_RECORD,
				QDF_TRACE_DEFAULT_PDEV_ID,
				qdf_nbuf_data_addr(msdu),
				sizeof(qdf_nbuf_data(msdu)), QDF_TX));
	htc_send_data_pkt(pdev->htc_pdev, &pkt->htc_pkt, more_data);

	return 0;               /* success */
}

qdf_nbuf_t
htt_tx_send_batch(htt_pdev_handle pdev, qdf_nbuf_t head_msdu, int num_msdus)
{
	qdf_nbuf_t rejected = NULL;
	uint16_t *msdu_id_storage;
	uint16_t msdu_id;
	qdf_nbuf_t msdu;
	/*
	 * FOR NOW, iterate through the batch, sending the frames singly.
	 * Eventually HTC and HIF should be able to accept a batch of
	 * data frames rather than singles.
	 */
	msdu = head_msdu;
	while (num_msdus--) {
		qdf_nbuf_t next_msdu = qdf_nbuf_next(msdu);
		msdu_id_storage = ol_tx_msdu_id_storage(msdu);
		msdu_id = *msdu_id_storage;

		/* htt_tx_send_base returns 0 as success and 1 as failure */
		if (htt_tx_send_base(pdev, msdu, msdu_id, pdev->download_len,
				     num_msdus)) {
			qdf_nbuf_set_next(msdu, rejected);
			rejected = msdu;
		}
		msdu = next_msdu;
	}
	return rejected;
}

int
htt_tx_send_nonstd(htt_pdev_handle pdev,
		   qdf_nbuf_t msdu,
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

int htt_tx_send_std(htt_pdev_handle pdev, qdf_nbuf_t msdu, uint16_t msdu_id)
{
	return htt_tx_send_base(pdev, msdu, msdu_id, pdev->download_len, 0);
}

#endif /*ATH_11AC_TXCOMPACT */

#if defined(HTT_DBG)
void htt_tx_desc_display(void *tx_desc)
{
	struct htt_tx_msdu_desc_t *htt_tx_desc;

	htt_tx_desc = (struct htt_tx_msdu_desc_t *)tx_desc;

	/* only works for little-endian */
	qdf_print("HTT tx desc (@ %p):", htt_tx_desc);
	qdf_print("  msg type = %d", htt_tx_desc->msg_type);
	qdf_print("  pkt subtype = %d", htt_tx_desc->pkt_subtype);
	qdf_print("  pkt type = %d", htt_tx_desc->pkt_type);
	qdf_print("  vdev ID = %d", htt_tx_desc->vdev_id);
	qdf_print("  ext TID = %d", htt_tx_desc->ext_tid);
	qdf_print("  postponed = %d", htt_tx_desc->postponed);
	qdf_print("  extension = %d", htt_tx_desc->extension);
	qdf_print("  cksum_offload = %d", htt_tx_desc->cksum_offload);
	qdf_print("  tx_compl_req= %d", htt_tx_desc->tx_compl_req);
	qdf_print("  length = %d", htt_tx_desc->len);
	qdf_print("  id = %d", htt_tx_desc->id);
#if HTT_PADDR64
	qdf_print("  frag desc addr.lo = %#x",
		  htt_tx_desc->frags_desc_ptr.lo);
	qdf_print("  frag desc addr.hi = %#x",
		  htt_tx_desc->frags_desc_ptr.hi);
#else /* ! HTT_PADDR64 */
	qdf_print("  frag desc addr = %#x", htt_tx_desc->frags_desc_ptr);
#endif /* HTT_PADDR64 */
	qdf_print("  peerid = %d", htt_tx_desc->peerid);
	qdf_print("  chanfreq = %d", htt_tx_desc->chanfreq);
}
#endif

#ifdef IPA_OFFLOAD
#ifdef QCA_WIFI_3_0
/**
 * htt_tx_ipa_uc_wdi_tx_buf_alloc() - Alloc WDI TX buffers
 * @pdev: htt context
 * @uc_tx_buf_sz: TX buffer size
 * @uc_tx_buf_cnt: TX Buffer count
 * @uc_tx_partition_base: IPA UC TX partition base value
 *
 * Allocate WDI TX buffers. Also note Rome supports only WDI 1.0.
 *
 * Return: 0 success
 */

static int htt_tx_ipa_uc_wdi_tx_buf_alloc(struct htt_pdev_t *pdev,
					  unsigned int uc_tx_buf_sz,
					  unsigned int uc_tx_buf_cnt,
					  unsigned int uc_tx_partition_base)
{
	unsigned int tx_buffer_count;
	unsigned int  tx_buffer_count_pwr2;
	void *buffer_vaddr;
	qdf_dma_addr_t buffer_paddr;
	uint32_t *header_ptr;
	qdf_dma_addr_t *ring_vaddr;
	uint16_t idx;

	ring_vaddr = (qdf_dma_addr_t *)pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr;
	/* Allocate TX buffers as many as possible */
	for (tx_buffer_count = 0;
	     tx_buffer_count < (uc_tx_buf_cnt - 1); tx_buffer_count++) {

		buffer_vaddr = qdf_mem_alloc_consistent(pdev->osdev,
			pdev->osdev->dev, uc_tx_buf_sz, &buffer_paddr);
		if (!buffer_vaddr) {
			qdf_print("IPA WDI TX buffer alloc fail %d allocated\n",
				tx_buffer_count);
			return tx_buffer_count;
		}

		header_ptr = buffer_vaddr;

		/* HTT control header */
		*header_ptr = HTT_IPA_UC_OFFLOAD_TX_HEADER_DEFAULT;
		header_ptr++;

		/* PKT ID */
		*header_ptr |= ((uint16_t) uc_tx_partition_base +
				tx_buffer_count) << 16;

		header_ptr++;

		/* Frag Desc Pointer */
		/* 64bits descriptor, Low 32bits */
		*header_ptr = qdf_get_lower_32_bits(buffer_paddr +
					IPA_UC_TX_BUF_FRAG_DESC_OFFSET);
		header_ptr++;

		/* 64bits descriptor, high 32bits */
		*header_ptr = qdf_get_upper_32_bits(buffer_paddr) &
			IPA_UC_TX_BUF_PADDR_HI_MASK;
		header_ptr++;

		/* chanreq, peerid */
		*header_ptr = 0xFFFFFFFF;
		header_ptr++;

		/* FRAG Header */
		/* 6 words TSO header */
		header_ptr += IPA_UC_TX_BUF_TSO_HDR_SIZE;
		*header_ptr = buffer_paddr + IPA_UC_TX_BUF_FRAG_HDR_OFFSET;

		*ring_vaddr = buffer_paddr;
		pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[tx_buffer_count] =
			buffer_vaddr;
		pdev->ipa_uc_tx_rsc.paddr_strg[tx_buffer_count] =
			buffer_paddr;
		/* Memory barrier to ensure actual value updated */

		ring_vaddr++;
	}

	/*
	 * Tx complete ring buffer count should be power of 2.
	 * So, allocated Tx buffer count should be one less than ring buffer
	 * size.
	 */
	tx_buffer_count_pwr2 = qdf_rounddown_pow_of_two(tx_buffer_count + 1)
			       - 1;
	if (tx_buffer_count > tx_buffer_count_pwr2) {
		QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_INFO,
			  "%s: Allocated Tx buffer count %d is rounded down to %d",
			  __func__, tx_buffer_count, tx_buffer_count_pwr2);

		/* Free over allocated buffers below power of 2 */
		for (idx = tx_buffer_count_pwr2; idx < tx_buffer_count; idx++) {
			if (pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx]) {
			    qdf_mem_free_consistent(
				pdev->osdev, pdev->osdev->dev,
				ol_cfg_ipa_uc_tx_buf_size(pdev->ctrl_pdev),
				pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx],
				pdev->ipa_uc_tx_rsc.paddr_strg[idx], 0);
			}
		}
	}

	if (tx_buffer_count_pwr2 < 0) {
		qdf_print("%s: Failed to round down Tx buffer count %d",
				__func__, tx_buffer_count_pwr2);
		tx_buffer_count_pwr2 = 0;
	}

	return tx_buffer_count_pwr2;
}
#else
static int htt_tx_ipa_uc_wdi_tx_buf_alloc(struct htt_pdev_t *pdev,
					  unsigned int uc_tx_buf_sz,
					  unsigned int uc_tx_buf_cnt,
					  unsigned int uc_tx_partition_base)
{
	unsigned int tx_buffer_count;
	unsigned int  tx_buffer_count_pwr2;
	qdf_nbuf_t buffer_vaddr;
	qdf_dma_addr_t buffer_paddr;
	uint32_t *header_ptr;
	uint32_t *ring_vaddr;
	uint16_t idx;
	QDF_STATUS status;

	ring_vaddr = pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr;
	/* Allocate TX buffers as many as possible */
	for (tx_buffer_count = 0;
	     tx_buffer_count < (uc_tx_buf_cnt - 1); tx_buffer_count++) {
		buffer_vaddr = qdf_nbuf_alloc(pdev->osdev,
					      uc_tx_buf_sz, 0, 4, false);
		if (!buffer_vaddr) {
			qdf_print("%s: TX BUF alloc fail, loop index: %d",
				  __func__, tx_buffer_count);
			return tx_buffer_count;
		}

		/* Init buffer */
		qdf_mem_zero(qdf_nbuf_data(buffer_vaddr), uc_tx_buf_sz);
		header_ptr = (uint32_t *) qdf_nbuf_data(buffer_vaddr);

		/* HTT control header */
		*header_ptr = HTT_IPA_UC_OFFLOAD_TX_HEADER_DEFAULT;
		header_ptr++;

		/* PKT ID */
		*header_ptr |= ((uint16_t) uc_tx_partition_base +
				tx_buffer_count) << 16;

		status = qdf_nbuf_map(pdev->osdev, buffer_vaddr,
				      QDF_DMA_BIDIRECTIONAL);
		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_ERROR,
			  "%s: nbuf map failed, loop index: %d",
			  __func__, tx_buffer_count);
			qdf_nbuf_free(buffer_vaddr);
			return tx_buffer_count;
		}

		buffer_paddr = qdf_nbuf_get_frag_paddr(buffer_vaddr, 0);
		header_ptr++;
		*header_ptr = (uint32_t) (buffer_paddr +
						IPA_UC_TX_BUF_FRAG_DESC_OFFSET);
		header_ptr++;
		*header_ptr = 0xFFFFFFFF;

		/* FRAG Header */
		header_ptr++;
		*header_ptr = buffer_paddr + IPA_UC_TX_BUF_FRAG_HDR_OFFSET;

		*ring_vaddr = buffer_paddr;
		pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[tx_buffer_count] =
			buffer_vaddr;
		/* Memory barrier to ensure actual value updated */

		ring_vaddr++;
	}

	/*
	 * Tx complete ring buffer count should be power of 2.
	 * So, allocated Tx buffer count should be one less than ring buffer
	 * size.
	 */
	tx_buffer_count_pwr2 = qdf_rounddown_pow_of_two(tx_buffer_count + 1)
			       - 1;
	if (tx_buffer_count > tx_buffer_count_pwr2) {
		QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_INFO,
			  "%s: Allocated Tx buffer count %d is rounded down to %d",
			  __func__, tx_buffer_count, tx_buffer_count_pwr2);

		/* Free over allocated buffers below power of 2 */
		for (idx = tx_buffer_count_pwr2; idx < tx_buffer_count; idx++) {
			if (pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx]) {
			    qdf_mem_free_consistent(
				pdev->osdev, pdev->osdev->dev,
				ol_cfg_ipa_uc_tx_buf_size(pdev->ctrl_pdev),
				pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx],
				pdev->ipa_uc_tx_rsc.paddr_strg[idx], 0);
			}
		}
	}

	if (tx_buffer_count_pwr2 < 0) {
		qdf_print("%s: Failed to round down Tx buffer count %d",
				__func__, tx_buffer_count_pwr2);
		tx_buffer_count_pwr2 = 0;
	}

	return tx_buffer_count_pwr2;
}
#endif

/**
 * htt_tx_ipa_uc_attach() - attach htt ipa uc tx resource
 * @pdev: htt context
 * @uc_tx_buf_sz: single tx buffer size
 * @uc_tx_buf_cnt: total tx buffer count
 * @uc_tx_partition_base: tx buffer partition start
 *
 * Return: 0 success
 *         ENOBUFS No memory fail
 */
int htt_tx_ipa_uc_attach(struct htt_pdev_t *pdev,
			 unsigned int uc_tx_buf_sz,
			 unsigned int uc_tx_buf_cnt,
			 unsigned int uc_tx_partition_base)
{
	int return_code = 0;
	unsigned int tx_comp_ring_size;

	/* Allocate CE Write Index WORD */
	pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr =
		qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev,
			4, &pdev->ipa_uc_tx_rsc.tx_ce_idx.paddr);
	if (!pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr) {
		qdf_print("%s: CE Write Index WORD alloc fail", __func__);
		return -ENOBUFS;
	}

	/* Allocate TX COMP Ring */
	tx_comp_ring_size = uc_tx_buf_cnt * sizeof(target_paddr_t);
	pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr =
		qdf_mem_alloc_consistent(
			pdev->osdev, pdev->osdev->dev,
			tx_comp_ring_size,
			&pdev->ipa_uc_tx_rsc.tx_comp_base.paddr);
	if (!pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr) {
		qdf_print("%s: TX COMP ring alloc fail", __func__);
		return_code = -ENOBUFS;
		goto free_tx_ce_idx;
	}

	qdf_mem_zero(pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr, tx_comp_ring_size);

	/* Allocate TX BUF vAddress Storage */
	pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg =
		qdf_mem_malloc(uc_tx_buf_cnt *
			sizeof(*pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg));
	if (!pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg) {
		qdf_print("%s: TX BUF POOL vaddr storage alloc fail", __func__);
		return_code = -ENOBUFS;
		goto free_tx_comp_base;
	}

	pdev->ipa_uc_tx_rsc.paddr_strg =
		qdf_mem_malloc(uc_tx_buf_cnt *
			sizeof(pdev->ipa_uc_tx_rsc.paddr_strg));
	if (!pdev->ipa_uc_tx_rsc.paddr_strg) {
		qdf_print("%s: TX BUF POOL paddr storage alloc fail", __func__);
		return_code = -ENOBUFS;
		goto free_tx_buf_pool_vaddr_strg;
	}

	pdev->ipa_uc_tx_rsc.alloc_tx_buf_cnt = htt_tx_ipa_uc_wdi_tx_buf_alloc(
		pdev, uc_tx_buf_sz, uc_tx_buf_cnt, uc_tx_partition_base);


	return 0;

free_tx_buf_pool_vaddr_strg:
	qdf_mem_free(pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg);
free_tx_comp_base:
	qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
				   tx_comp_ring_size,
				   pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr,
				   pdev->ipa_uc_tx_rsc.tx_comp_base.paddr,
				   qdf_get_dma_mem_context((&pdev->
							    ipa_uc_tx_rsc.
							    tx_comp_base),
							   memctx));
free_tx_ce_idx:
	qdf_mem_free_consistent(pdev->osdev, pdev->osdev->dev,
				   4,
				   pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr,
				   pdev->ipa_uc_tx_rsc.tx_ce_idx.paddr,
				   qdf_get_dma_mem_context((&pdev->
							    ipa_uc_tx_rsc.
							    tx_ce_idx),
							   memctx));
	return return_code;
}

/**
 * htt_tx_ipa_uc_detach() - Free WDI TX resources
 * @pdev: htt context
 *
 * Remove IPA WDI TX resources during device detach
 * Free all of allocated resources
 *
 * Return: 0 success
 */
int htt_tx_ipa_uc_detach(struct htt_pdev_t *pdev)
{
	uint16_t idx;

	if (pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr) {
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			4,
			pdev->ipa_uc_tx_rsc.tx_ce_idx.vaddr,
			pdev->ipa_uc_tx_rsc.tx_ce_idx.paddr,
			qdf_get_dma_mem_context(
				(&pdev->ipa_uc_tx_rsc.tx_ce_idx),
				memctx));
	}

	if (pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr) {
		qdf_mem_free_consistent(
			pdev->osdev, pdev->osdev->dev,
			ol_cfg_ipa_uc_tx_max_buf_cnt(pdev->ctrl_pdev) *
							sizeof(qdf_nbuf_t),
			pdev->ipa_uc_tx_rsc.tx_comp_base.vaddr,
			pdev->ipa_uc_tx_rsc.tx_comp_base.paddr,
			qdf_get_dma_mem_context((&pdev->ipa_uc_tx_rsc.
						 tx_comp_base),
						memctx));
	}

	/* Free each single buffer */
	for (idx = 0; idx < pdev->ipa_uc_tx_rsc.alloc_tx_buf_cnt; idx++) {
		if (pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx]) {
			qdf_mem_free_consistent(
				pdev->osdev, pdev->osdev->dev,
				ol_cfg_ipa_uc_tx_buf_size(pdev->ctrl_pdev),
				pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg[idx],
				pdev->ipa_uc_tx_rsc.paddr_strg[idx], 0);
		}
	}

	/* Free storage */
	qdf_mem_free(pdev->ipa_uc_tx_rsc.tx_buf_pool_vaddr_strg);
	qdf_mem_free(pdev->ipa_uc_tx_rsc.paddr_strg);

	return 0;
}
#endif /* IPA_OFFLOAD */

#if defined(FEATURE_TSO) && defined(HELIUMPLUS)
void
htt_tx_desc_fill_tso_info(htt_pdev_handle pdev, void *desc,
	 struct qdf_tso_info_t *tso_info)
{
	u_int32_t *word;
	int i;
	struct qdf_tso_seg_elem_t *tso_seg = tso_info->curr_seg;
	struct msdu_ext_desc_t *msdu_ext_desc = (struct msdu_ext_desc_t *)desc;

	word = (u_int32_t *)(desc);

	/* Initialize the TSO flags per MSDU */
	msdu_ext_desc->tso_flags =
		 tso_seg->seg.tso_flags;

	/* First 24 bytes (6*4) contain the TSO flags */
	TSO_DEBUG("%s seq# %u l2 len %d, ip len %d flags 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		  __func__,
		  tso_seg->seg.tso_flags.tcp_seq_num,
		  tso_seg->seg.tso_flags.l2_len,
		  tso_seg->seg.tso_flags.ip_len,
		  *word,
		  *(word + 1),
		  *(word + 2),
		  *(word + 3),
		  *(word + 4),
		  *(word + 5));

	word += 6;

	for (i = 0; i < tso_seg->seg.num_frags; i++) {
		uint32_t lo = 0;
		uint32_t hi = 0;
		qdf_dmaaddr_to_32s(tso_seg->seg.tso_frags[i].paddr,
						&lo, &hi);
		/* [31:0] first 32 bits of the buffer pointer  */
		*word = lo;
		word++;
		/* [15:0] the upper 16 bits of the first buffer pointer */
		/* [31:16] length of the first buffer */
		*word = (tso_seg->seg.tso_frags[i].length << 16) | hi;
		word++;
		TSO_DEBUG("%s frag[%d] ptr_low 0x%x ptr_hi 0x%x len %u\n",
			__func__, i,
			msdu_ext_desc->frags[i].u.frag32.ptr_low,
			msdu_ext_desc->frags[i].u.frag32.ptr_hi,
			msdu_ext_desc->frags[i].u.frag32.len);

	}

	if (tso_seg->seg.num_frags < FRAG_NUM_MAX) {
		*word = 0;
	}
}
#endif /* FEATURE_TSO */

/**
 * htt_get_ext_tid() - get ext_tid value
 * @type: extension header type
 * @ext_header_data: header data
 * @msdu_info: msdu info
 *
 * Return: ext_tid value
 */
static inline
int htt_get_ext_tid(enum extension_header_type type,
	void *ext_header_data, struct htt_msdu_info_t *msdu_info)
{
	if (type == OCB_MODE_EXT_HEADER && ext_header_data)
		return ((struct ocb_tx_ctrl_hdr_t *)ext_header_data)->ext_tid;
	else
		return msdu_info->info.ext_tid;
}

/**
 * htt_get_channel_freq() - get channel frequency
 * @type: extension header type
 * @ext_header_data: header data
 *
 * Return: channel frequency number
 */
static inline
int htt_get_channel_freq(enum extension_header_type type,
	void *ext_header_data)
{
	if (type == OCB_MODE_EXT_HEADER && ext_header_data)
		return ((struct ocb_tx_ctrl_hdr_t *)ext_header_data)
							->channel_freq;
	else
		return HTT_INVALID_CHANNEL;
}

/**
 * htt_fill_ocb_ext_header() - fill OCB extension header
 * @msdu: network buffer
 * @local_desc_ext: extension descriptor
 * @type: extension header type
 * @ext_header_data: header data
 * @is_dsrc: is dsrc is eenabled or not
 *
 * Return: none
 */
static
void htt_fill_ocb_ext_header(qdf_nbuf_t msdu,
	struct htt_tx_msdu_desc_ext_t *local_desc_ext,
	enum extension_header_type type, void *ext_header_data)
{
	struct ocb_tx_ctrl_hdr_t *tx_ctrl =
		(struct ocb_tx_ctrl_hdr_t *)ext_header_data;

	if (tx_ctrl->all_flags == 0)
		return;
	/*
	  * Copy the info that was read from TX control header from the
	  * user application to the extended HTT header.
	  * First copy everything
	  * to a local temp structure, and then copy everything to the
	  * actual uncached structure in one go to save memory writes.
	  */
	local_desc_ext->valid_pwr = tx_ctrl->valid_pwr;
	local_desc_ext->valid_mcs_mask = tx_ctrl->valid_datarate;
	local_desc_ext->valid_retries = tx_ctrl->valid_retries;
	local_desc_ext->valid_expire_tsf = tx_ctrl->valid_expire_tsf;
	local_desc_ext->valid_chainmask = tx_ctrl->valid_chain_mask;

	local_desc_ext->pwr = tx_ctrl->pwr;
	if (tx_ctrl->valid_datarate &&
			tx_ctrl->datarate <= htt_ofdm_datarate_max)
		local_desc_ext->mcs_mask =
			(1 << (tx_ctrl->datarate + 4));
	local_desc_ext->retry_limit = tx_ctrl->retry_limit;
	local_desc_ext->expire_tsf_lo = tx_ctrl->expire_tsf_lo;
	local_desc_ext->expire_tsf_hi = tx_ctrl->expire_tsf_hi;
	local_desc_ext->chain_mask = tx_ctrl->chain_mask;
	local_desc_ext->is_dsrc = 1;
	qdf_nbuf_push_head(msdu, sizeof(struct htt_tx_msdu_desc_ext_t));
	qdf_mem_copy(qdf_nbuf_data(msdu), local_desc_ext,
			sizeof(struct htt_tx_msdu_desc_ext_t));
	QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_EXT_HEADER(msdu) = 1;
}

/**
 * htt_fill_wisa_ext_header() - fill WiSA extension header
 * @msdu: network buffer
 * @local_desc_ext: extension descriptor
 * @type: extension header type
 * @ext_header_data: header data
 *
 * Return: none
 */
static
void htt_fill_wisa_ext_header(qdf_nbuf_t msdu,
	struct htt_tx_msdu_desc_ext_t *local_desc_ext,
	enum extension_header_type type, void *ext_header_data)
{
	void *qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	QDF_STATUS status;

	if (!qdf_ctx) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: qdf_ctx is NULL", __func__);
		return;
	}

	local_desc_ext->valid_mcs_mask = 1;
	if (WISA_MODE_EXT_HEADER_6MBPS == type)
		local_desc_ext->mcs_mask = htt_ofdm_datarate_6_mbps;
	else
		local_desc_ext->mcs_mask = htt_ofdm_datarate_24_mbps;
	local_desc_ext->valid_nss_mask = 1;
	local_desc_ext->nss_mask = 1;
	local_desc_ext->valid_bandwidth = 1;
	local_desc_ext->bandwidth_mask = htt_tx_bandwidth_20MHz;
	local_desc_ext->valid_guard_interval = 1;
	local_desc_ext->guard_interval = htt_tx_guard_interval_regular;

	/*
	 * Do dma_unmap and dma_map again if already mapped
	 * as adding extra bytes in skb
	 */
	if (QDF_NBUF_CB_PADDR(msdu) != 0)
		qdf_nbuf_unmap_single(qdf_ctx, msdu, QDF_DMA_TO_DEVICE);

	qdf_nbuf_push_head(msdu, sizeof(struct htt_tx_msdu_desc_ext_t));
	qdf_mem_copy(qdf_nbuf_data(msdu), local_desc_ext,
			sizeof(struct htt_tx_msdu_desc_ext_t));

	if (QDF_NBUF_CB_PADDR(msdu) != 0) {
		status = qdf_nbuf_map_single(qdf_ctx, msdu, QDF_DMA_TO_DEVICE);
		if (qdf_unlikely(status != QDF_STATUS_SUCCESS)) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
				"%s: nbuf map failed", __func__);
			return;
		}
	}
	QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_EXT_HEADER(msdu) = 1;
}

/**
 * htt_push_ext_header() - fill extension header
 * @msdu: network buffer
 * @local_desc_ext: extension descriptor
 * @type: extension header type
 * @ext_header_data: header data
 * @is_dsrc: is dsrc is eenabled or not
 *
 * Return: none
 */
static
void htt_push_ext_header(qdf_nbuf_t msdu,
	struct htt_tx_msdu_desc_ext_t *local_desc_ext,
	enum extension_header_type type, void *ext_header_data)
{
	switch (type) {
	case OCB_MODE_EXT_HEADER:
		htt_fill_ocb_ext_header(msdu, local_desc_ext,
					type, ext_header_data);
		break;
	case WISA_MODE_EXT_HEADER_6MBPS:
	case WISA_MODE_EXT_HEADER_24MBPS:
		htt_fill_wisa_ext_header(msdu, local_desc_ext,
					type, ext_header_data);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_INFO,
			"Invalid EXT header type %d\n", type);
		break;
	}
}

QDF_STATUS
htt_tx_desc_init(htt_pdev_handle pdev,
		 void *htt_tx_desc,
		 qdf_dma_addr_t htt_tx_desc_paddr,
		 uint16_t msdu_id,
		 qdf_nbuf_t msdu, struct htt_msdu_info_t *msdu_info,
		 struct qdf_tso_info_t *tso_info,
		 void *ext_header_data,
		 enum extension_header_type type)
{
	uint8_t  pkt_type, pkt_subtype = 0, ce_pkt_type = 0;
	uint32_t hw_classify = 0, data_attr = 0;
	uint32_t *word0, *word1, local_word3;
#if HTT_PADDR64
	uint32_t *word4;
#else /* ! HTT_PADDR64 */
	uint32_t *word3;
#endif /* HTT_PADDR64 */
	uint32_t local_word0, local_word1;
	struct htt_host_tx_desc_t *htt_host_tx_desc =
		(struct htt_host_tx_desc_t *)
		(((char *)htt_tx_desc) - HTT_TX_DESC_VADDR_OFFSET);
	bool desc_ext_required = (type != EXT_HEADER_NOT_PRESENT);
	uint16_t channel_freq;
	void *qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	QDF_STATUS status;

	if (qdf_unlikely(!qdf_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: qdf_ctx is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (qdf_unlikely(!msdu_info)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: bad arg: msdu_info is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (qdf_unlikely(!tso_info)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: bad arg: tso_info is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	word0 = (uint32_t *) htt_tx_desc;
	word1 = word0 + 1;
	/*
	 * word2 is frag desc pointer
	 * word3 or 4 is peer_id
	 */
#if HTT_PADDR64
	word4 = word0 + 4;      /* Dword 3 */
#else /* ! HTT_PADDR64  */
	word3 = word0 + 3;      /* Dword 3 */
#endif /* HTT_PADDR64 */

	pkt_type = msdu_info->info.l2_hdr_type;

	if (qdf_likely(pdev->cfg.ce_classify_enabled)) {
		if (qdf_likely(pkt_type == htt_pkt_type_eth2 ||
			pkt_type == htt_pkt_type_ethernet))
			qdf_nbuf_tx_info_get(msdu, pkt_type, pkt_subtype,
				     hw_classify);

		ce_pkt_type = htt_to_ce_pkt_type[pkt_type];
		if (0xffffffff == ce_pkt_type) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			"Invalid HTT pkt type %d\n", pkt_type);
			return QDF_STATUS_E_INVAL;
		}
	}

	/*
	 * HTT Tx Desc is in uncached memory. Used cached writes per word, to
	 * reduce unnecessary memory access.
	 */

	local_word0 = 0;

	HTT_H2T_MSG_TYPE_SET(local_word0, HTT_H2T_MSG_TYPE_TX_FRM);
	HTT_TX_DESC_PKT_TYPE_SET(local_word0, pkt_type);
	HTT_TX_DESC_PKT_SUBTYPE_SET(local_word0, pkt_subtype);
	HTT_TX_DESC_VDEV_ID_SET(local_word0, msdu_info->info.vdev_id);
	HTT_TX_DESC_EXT_TID_SET(local_word0, htt_get_ext_tid(type,
					ext_header_data, msdu_info));
	HTT_TX_DESC_EXTENSION_SET(local_word0, desc_ext_required);
	HTT_TX_DESC_EXT_TID_SET(local_word0, msdu_info->info.ext_tid);
	HTT_TX_DESC_CKSUM_OFFLOAD_SET(local_word0,
				      msdu_info->action.cksum_offload);
	if (pdev->cfg.is_high_latency)
		HTT_TX_DESC_TX_COMP_SET(local_word0, msdu_info->action.
							tx_comp_req);
	HTT_TX_DESC_NO_ENCRYPT_SET(local_word0,
				   msdu_info->action.do_encrypt ?
				   0 : 1);

	*word0 = local_word0;

	local_word1 = 0;

	if (tso_info->is_tso) {
		uint32_t total_len = tso_info->curr_seg->seg.total_len;

		HTT_TX_DESC_FRM_LEN_SET(local_word1, total_len);
		TSO_DEBUG("%s setting HTT TX DESC Len = curr_seg->seg.total_len %d\n",
			  __func__, total_len);
	} else {
		HTT_TX_DESC_FRM_LEN_SET(local_word1, qdf_nbuf_len(msdu));
	}

	HTT_TX_DESC_FRM_ID_SET(local_word1, msdu_id);
	*word1 = local_word1;

	/* Initialize peer_id to INVALID_PEER because
	   this is NOT Reinjection path */
	local_word3 = HTT_INVALID_PEER;
	channel_freq = htt_get_channel_freq(type, ext_header_data);
	if (channel_freq != HTT_INVALID_CHANNEL)
		HTT_TX_DESC_CHAN_FREQ_SET(local_word3, channel_freq);
#if HTT_PADDR64
	*word4 = local_word3;
#else /* ! HTT_PADDR64 */
	*word3 = local_word3;
#endif /* HTT_PADDR64 */

	/*
	 *  If any of the tx control flags are set, then we need the extended
	 *  HTT header.
	 */
	if (desc_ext_required) {
		struct htt_tx_msdu_desc_ext_t local_desc_ext = {0};

		htt_push_ext_header(msdu, &local_desc_ext,
			type, ext_header_data);
	}

	/*
	 * Specify that the data provided by the OS is a bytestream,
	 * and thus should not be byte-swapped during the HIF download
	 * even if the host is big-endian.
	 * There could be extra fragments added before the OS's fragments,
	 * e.g. for TSO, so it's incorrect to clear the frag 0 wordstream flag.
	 * Instead, clear the wordstream flag for the final fragment, which
	 * is certain to be (one of the) fragment(s) provided by the OS.
	 * Setting the flag for this final fragment suffices for specifying
	 * all fragments provided by the OS rather than added by the driver.
	 */
	qdf_nbuf_set_frag_is_wordstream(msdu, qdf_nbuf_get_num_frags(msdu) - 1,
					0);

	if (QDF_NBUF_CB_PADDR(msdu) == 0) {
		status = qdf_nbuf_map_single(qdf_ctx, msdu, QDF_DMA_TO_DEVICE);
		if (qdf_unlikely(status != QDF_STATUS_SUCCESS)) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: nbuf map failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
	}

	/* store a link to the HTT tx descriptor within the netbuf */
	qdf_nbuf_frag_push_head(msdu, sizeof(struct htt_host_tx_desc_t),
				(char *)htt_host_tx_desc, /* virtual addr */
				htt_tx_desc_paddr);

	/*
	 * Indicate that the HTT header (and HTC header) is a meta-data
	 * "wordstream", i.e. series of uint32_t, rather than a data
	 * bytestream.
	 * This allows the HIF download to byteswap the HTT + HTC headers if
	 * the host is big-endian, to convert to the target's little-endian
	 * format.
	 */
	qdf_nbuf_set_frag_is_wordstream(msdu, 0, 1);

	if (qdf_likely(pdev->cfg.ce_classify_enabled &&
		(msdu_info->info.l2_hdr_type != htt_pkt_type_mgmt))) {
		uint32_t pkt_offset = qdf_nbuf_get_frag_len(msdu, 0);

		data_attr = hw_classify << QDF_CE_TX_CLASSIFY_BIT_S;
		data_attr |= ce_pkt_type << QDF_CE_TX_PKT_TYPE_BIT_S;
		data_attr |= pkt_offset  << QDF_CE_TX_PKT_OFFSET_BIT_S;
	}

	qdf_nbuf_data_attr_set(msdu, data_attr);
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL

/**
 * htt_tx_group_credit_process() - process group data for
 *				   credit update indication
 * @pdev: pointer to htt device.
 * @msg_word: htt msg
 *
 * Return: None
 */
void htt_tx_group_credit_process(struct htt_pdev_t *pdev, u_int32_t *msg_word)
{
	int group_credit_sign;
	int32_t group_credit;
	u_int32_t group_credit_abs, vdev_id_mask, ac_mask;
	u_int8_t group_abs, group_id;
	u_int8_t group_offset = 0, more_group_present = 0;
	more_group_present = HTT_TX_CREDIT_TXQ_GRP_GET(*msg_word);

	while (more_group_present) {
		/* Parse the Group Data */
		group_id = HTT_TXQ_GROUP_ID_GET(*(msg_word+1
						+group_offset));
		group_credit_abs =
			HTT_TXQ_GROUP_CREDIT_COUNT_GET(*(msg_word+1
						+group_offset));
		group_credit_sign =
			HTT_TXQ_GROUP_SIGN_GET(*(msg_word+1
						+group_offset)) ? -1 : 1;
		group_credit = group_credit_sign * group_credit_abs;
		group_abs = HTT_TXQ_GROUP_ABS_GET(*(msg_word+1
						+group_offset));

		vdev_id_mask =
			HTT_TXQ_GROUP_VDEV_ID_MASK_GET(*(msg_word+2
						+group_offset));
		ac_mask = HTT_TXQ_GROUP_AC_MASK_GET(*(msg_word+2
						+group_offset));

		ol_txrx_update_tx_queue_groups(pdev->txrx_pdev, group_id,
					       group_credit, group_abs,
					       vdev_id_mask, ac_mask);
		more_group_present = HTT_TXQ_GROUP_EXT_GET(*(msg_word+1
						+group_offset));
		group_offset += HTT_TX_GROUP_INDEX_OFFSET;
	}
	ol_tx_update_group_credit_stats(pdev->txrx_pdev);
}
#endif

