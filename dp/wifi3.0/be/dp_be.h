/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#ifndef __DP_BE_H
#define __DP_BE_H

#include <dp_types.h>
#include <hal_be_tx.h>

/* maximum number of entries in one page of secondary page table */
#define DP_CC_SPT_PAGE_MAX_ENTRIES 512

/* maximum number of entries in primary page table */
#define DP_CC_PPT_MAX_ENTRIES 1024

/* cookie conversion required CMEM offset from CMEM pool */
#define DP_CC_MEM_OFFSET_IN_CMEM 0

/* cookie conversion primary page table size 4K */
#define DP_CC_PPT_MEM_SIZE 4096

/* FST required CMEM offset from CMEM pool */
#define DP_FST_MEM_OFFSET_IN_CMEM \
	(DP_CC_MEM_OFFSET_IN_CMEM + DP_CC_PPT_MEM_SIZE)

/* lower 9 bits in Desc ID for offset in page of SPT */
#define DP_CC_DESC_ID_SPT_VA_OS_SHIFT 0

#define DP_CC_DESC_ID_SPT_VA_OS_MASK 0x1FF

#define DP_CC_DESC_ID_SPT_VA_OS_LSB 0

#define DP_CC_DESC_ID_SPT_VA_OS_MSB 8

/* for 4k unaligned case */
#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_SHIFT 9

#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_MASK 0xFFE00

#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_LSB 9

#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_MSB 19

#define DP_CC_PPT_ENTRY_SIZE_4K_UNALIGNED 8

/* for 4k aligned case */
#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_SHIFT 10

#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_MASK 0xFFC00

#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_LSB 10

#define DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_MSB 19

#define DP_CC_DESC_ID_PPT_PAGE_HIGH_32BIT_4K_ALIGNED_SHIFT 9

#define DP_CC_DESC_ID_PPT_PAGE_HIGH_32BIT_4K_ALIGNED_MASK 0x200

#define DP_CC_PPT_ENTRY_SIZE_4K_ALIGNED 4

/* 4K aligned case, number of bits HW append for one PPT entry value */
#define DP_CC_PPT_ENTRY_HW_APEND_BITS_4K_ALIGNED 12

/**
 * struct dp_spt_page_desc - secondary page table page descriptors
 * @next: pointer to next linked SPT page Desc
 * @page_v_addr: page virtual address
 * @page_p_addr: page physical address
 * @ppt_index: entry index in primary page table where this page physical
		address stored
 * @avail_entry_index: index for available entry that store TX/RX Desc VA
 */
struct dp_spt_page_desc {
	struct dp_spt_page_desc *next;
	uint8_t *page_v_addr;
	qdf_dma_addr_t page_p_addr;
	uint16_t ppt_index;
	uint16_t avail_entry_index;
};

/**
 * struct dp_hw_cookie_conversion_t - main context for HW cookie conversion
 * @cmem_base: CMEM base address for primary page table setup
 * @total_page_num: total DDR page allocated
 * @free_page_num: available DDR page number for TX/RX Desc ID initialization
 * @page_desc_freelist: available page Desc list
 * @page_desc_base: page Desc buffer base address.
 * @page_pool: DDR pages pool
 * @cc_lock: locks for page acquiring/free
 */
struct dp_hw_cookie_conversion_t {
	uint32_t cmem_base;
	uint32_t total_page_num;
	uint32_t free_page_num;
	struct dp_spt_page_desc *page_desc_freelist;
	struct dp_spt_page_desc *page_desc_base;
	struct qdf_mem_multi_page_t page_pool;
	qdf_spinlock_t cc_lock;
};

/**
 * struct dp_spt_page_desc_list - containor of SPT page desc list info
 * @spt_page_list_head: head of SPT page descriptor list
 * @spt_page_list_tail: tail of SPT page descriptor list
 * @num_spt_pages: number of SPT page descriptor allocated
 */
struct dp_spt_page_desc_list {
	struct dp_spt_page_desc *spt_page_list_head;
	struct dp_spt_page_desc *spt_page_list_tail;
	uint16_t num_spt_pages;
};

#define DP_CC_SPT_PAGE_UPDATE_VA(_page_base_va, _index, _desc_va) \
	{ (_page_base_va)[_index] = (uintptr_t)(_desc_va); }

/**
 * struct dp_tx_bank_profile - DP wrapper for TCL banks
 * @is_configured: flag indicating if this bank is configured
 * @ref_count: ref count indicating number of users of the bank
 * @bank_config: HAL TX bank configuration
 */
struct dp_tx_bank_profile {
	uint8_t is_configured;
	qdf_atomic_t  ref_count;
	union hal_tx_bank_config bank_config;
};

/**
 * struct dp_soc_be - Extended DP soc for BE targets
 * @soc: dp soc structure
 * @num_bank_profiles: num TX bank profiles
 * @bank_profiles: bank profiles for various TX banks
 * @hw_cc_ctx: core context of HW cookie conversion
 * @tx_spt_page_desc: spt page desc allocated for TX desc pool
 * @rx_spt_page_desc: spt page desc allocated for RX desc pool
 */
struct dp_soc_be {
	struct dp_soc soc;
	uint8_t num_bank_profiles;
	qdf_mutex_t tx_bank_lock;
	struct dp_tx_bank_profile *bank_profiles;
	struct dp_hw_cookie_conversion_t hw_cc_ctx;
	struct dp_spt_page_desc_list tx_spt_page_desc[MAX_TXDESC_POOLS];
	struct dp_spt_page_desc_list rx_spt_page_desc[MAX_RXDESC_POOLS];
};

/**
 * struct dp_pdev_be - Extended DP pdev for BE targets
 * @pdev: dp pdev structure
 */
struct dp_pdev_be {
	struct dp_pdev pdev;
};

/**
 * struct dp_vdev_be - Extended DP vdev for BE targets
 * @vdev: dp vdev structure
 * @bank_id: bank_id to be used for TX
 * @vdev_id_check_en: flag if HW vdev_id check is enabled for vdev
 */
struct dp_vdev_be {
	struct dp_vdev vdev;
	int8_t bank_id;
	uint8_t vdev_id_check_en;
};

/**
 * struct dp_peer_be - Extended DP peer for BE targets
 * @dp_peer: dp peer structure
 */
struct dp_peer_be {
	struct dp_peer peer;
};

/**
 * dp_get_soc_context_size_be() - get context size for target specific DP soc
 *
 * Return: value in bytes for BE specific soc structure
 */
qdf_size_t dp_get_soc_context_size_be(void);

/**
 * dp_initialize_arch_ops_be() - initialize BE specific arch ops
 * @arch_ops: arch ops pointer
 *
 * Return: none
 */
void dp_initialize_arch_ops_be(struct dp_arch_ops *arch_ops);

/**
 * dp_get_context_size_be() - get BE specific size for peer/vdev/pdev/soc
 * @arch_ops: arch ops pointer
 *
 * Return: size in bytes for the context_type
 */
qdf_size_t dp_get_context_size_be(enum dp_context_type context_type);

/**
 * dp_get_be_soc_from_dp_soc() - get dp_soc_be from dp_soc
 * @soc: dp_soc pointer
 *
 * Return: dp_soc_be pointer
 */
static inline struct dp_soc_be *dp_get_be_soc_from_dp_soc(struct dp_soc *soc)
{
	return (struct dp_soc_be *)soc;
}

/**
 * dp_get_be_pdev_from_dp_pdev() - get dp_pdev_be from dp_pdev
 * @pdev: dp_pdev pointer
 *
 * Return: dp_pdev_be pointer
 */
static inline
struct dp_pdev_be *dp_get_be_pdev_from_dp_pdev(struct dp_pdev *pdev)
{
	return (struct dp_pdev_be *)pdev;
}

/**
 * dp_get_be_vdev_from_dp_vdev() - get dp_vdev_be from dp_vdev
 * @vdev: dp_vdev pointer
 *
 * Return: dp_vdev_be pointer
 */
static inline
struct dp_vdev_be *dp_get_be_vdev_from_dp_vdev(struct dp_vdev *vdev)
{
	return (struct dp_vdev_be *)vdev;
}

/**
 * dp_get_be_peer_from_dp_peer() - get dp_peer_be from dp_peer
 * @peer: dp_peer pointer
 *
 * Return: dp_peer_be pointer
 */
static inline
struct dp_peer_be *dp_get_be_peer_from_dp_peer(struct dp_peer *peer)
{
	return (struct dp_peer_be *)peer;
}

/**
 * dp_cc_spt_page_desc_alloc() - allocate SPT DDR page descriptor from pool
 * @be_soc: beryllium soc handler
 * @list_head: pointer to page desc head
 * @list_tail: pointer to page desc tail
 * @desc_num: number of TX/RX Descs required for SPT pages
 *
 * Return: number of SPT page Desc allocated
 */
uint16_t dp_cc_spt_page_desc_alloc(struct dp_soc_be *be_soc,
				   struct dp_spt_page_desc **list_head,
				   struct dp_spt_page_desc **list_tail,
				   uint16_t desc_num);
/**
 * dp_cc_spt_page_desc_free() - free SPT DDR page descriptor to pool
 * @be_soc: beryllium soc handler
 * @list_head: pointer to page desc head
 * @list_tail: pointer to page desc tail
 * @page_nums: number of page desc freed back to pool
 */
void dp_cc_spt_page_desc_free(struct dp_soc_be *be_soc,
			      struct dp_spt_page_desc **list_head,
			      struct dp_spt_page_desc **list_tail,
			      uint16_t page_nums);

/**
 * dp_cc_desc_id_generate() - generate SW cookie ID according to
				DDR page 4K aligned or not
 * @ppt_index: offset index in primary page table
 * @spt_index: offset index in sceondary DDR page
 * @page_4k_align: DDR page address 4K aligned or not
 *
 * for 4K aligned DDR page, ppt_index offset is using 4 bytes entry,
 * while HW use 8 bytes offset index, need 10th bit to indicate it's
 * in high 32bits or low 32bits.
 * for 4k un-aligned DDR page, ppt_index offset is using 8bytes entry,
 * it's match with HW assuming.
 *
 * Return: cookie ID
 */
static inline uint32_t dp_cc_desc_id_generate(uint16_t ppt_index,
					      uint16_t spt_index,
					      bool page_4k_align)
{
	uint32_t id = 0;

	if (qdf_likely(page_4k_align))
		id =
		((ppt_index / 2) <<
		DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_SHIFT) |
		((ppt_index % 2) <<
		DP_CC_DESC_ID_PPT_PAGE_HIGH_32BIT_4K_ALIGNED_SHIFT) |
		spt_index;
	else
		id =
		(ppt_index <<
		DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_SHIFT) |
		spt_index;

	return id;
}

/**
 * dp_cc_desc_va_find() - find TX/RX Descs virtual address by ID
 * @be_soc: be soc handle
 * @desc_id: TX/RX Dess ID
 * @page_4k_align: DDR page address 4K aligned or not
 *
 * Return: TX/RX Desc virtual address
 */
static inline void *dp_cc_desc_find(struct dp_soc *soc,
				    uint32_t desc_id,
				    bool page_4k_align)
{
	struct dp_soc_be *be_soc;
	struct dp_hw_cookie_conversion_t *cc_ctx;
	uint16_t ppt_page_id, spt_va_id;
	uint64_t *spt_page_va;

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	cc_ctx = &be_soc->hw_cc_ctx;
	if (qdf_likely(page_4k_align))
		ppt_page_id =
			(((desc_id &
			DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_MASK) >>
			DP_CC_DESC_ID_PPT_PAGE_OS_4K_ALIGNED_SHIFT) * 2) +
			((desc_id &
			DP_CC_DESC_ID_PPT_PAGE_HIGH_32BIT_4K_ALIGNED_MASK) >>
			DP_CC_DESC_ID_PPT_PAGE_HIGH_32BIT_4K_ALIGNED_SHIFT);
	else
		ppt_page_id =
			(desc_id &
			DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_MASK) >>
			DP_CC_DESC_ID_PPT_PAGE_OS_4K_UNALIGNED_SHIFT;

	spt_va_id = (desc_id & DP_CC_DESC_ID_SPT_VA_OS_MASK) >>
			DP_CC_DESC_ID_SPT_VA_OS_SHIFT;

	/*
	 * ppt index in cmem is same order where the page in the
	 * page desc array during initialization.
	 * entry size in DDR page is 64 bits, then
	 * (1) 64 bits OS, (uint64_t *) --> (void *) conversion, no issue.
	 * (2) 32 bits OS, TX/RX Desc VA size is 32bits, (uint64_t *) -->
	 * (void *) conversion, lower 32 bits from uint64_t is saved, no issue
	 * as higer 32 bits is 0.
	 */
	spt_page_va =
		(uint64_t *)cc_ctx->page_desc_base[ppt_page_id].page_v_addr;

	return (void *)(uintptr_t)(*(spt_page_va  + spt_va_id));
}
#endif
