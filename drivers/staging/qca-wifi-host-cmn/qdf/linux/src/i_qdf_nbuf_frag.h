/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: i_qdf_nbuf_frag.h
 * This file provides OS dependent nbuf frag API's.
 */

#ifndef _I_QDF_NBUF_FRAG_H
#define _I_QDF_NBUF_FRAG_H

#include <qdf_net_types.h>
#include <qdf_mem.h>

#define QDF_NBUF_FRAG_DEBUG_COUNT_ZERO    0
#define QDF_NBUF_FRAG_DEBUG_COUNT_ONE     1

/**
 * typedef __qdf_frag_t - Abstraction for void * for frag address
 */
typedef void *__qdf_frag_t;

#ifdef QDF_NBUF_FRAG_GLOBAL_COUNT

/**
 * __qdf_frag_count_get() - Get global frag count
 *
 * Return: Global frag gauge
 */
uint32_t __qdf_frag_count_get(void);

/**
 * __qdf_frag_count_inc() - Increment frag global count
 * @value: Increment value
 *
 * Return: none
 */
void __qdf_frag_count_inc(uint32_t value);

/**
 * __qdf_frag_count_dec() - Decrement frag global count
 * @value: Decrement value
 *
 * Return: none
 */
void __qdf_frag_count_dec(uint32_t value);

/*
 * __qdf_frag_mod_init() - Initialization routine for qdf_frag
 *
 * Return: none
 */
void __qdf_frag_mod_init(void);

/**
 * __qdf_frag_mod_exit() - Uninitialization routine for qdf_frag
 *
 * Return: none
 */
void __qdf_frag_mod_exit(void);

#else
static inline uint32_t __qdf_frag_count_get(void)
{
	return 0;
}

static inline void __qdf_frag_count_inc(uint32_t value)
{
}

static inline void __qdf_frag_count_dec(uint32_t value)
{
}

static inline void __qdf_frag_mod_init(void)
{
}

static inline void __qdf_frag_mod_exit(void)
{
}
#endif /* QDF_NBUF_FRAG_GLOBAL_COUNT */

/**
 * Maximum number of frags an SKB can hold
 */
#define __QDF_NBUF_MAX_FRAGS MAX_SKB_FRAGS

/**
 * __qdf_mem_unmap_page() - Unmap frag memory
 * @osdev: qdf_device_t
 * @paddr: Address to be unmapped
 * @nbytes: Number of bytes to be unmapped
 * @dir: qdf_dma_dir_t
 */
void __qdf_mem_unmap_page(qdf_device_t osdev, qdf_dma_addr_t paddr,
			  size_t nbytes, qdf_dma_dir_t dir);

/**
 * __qdf_mem_map_page() - Map frag memory
 * @osdev: qdf_device_t
 * @buf: Vaddr to be mapped
 * @dir: qdf_dma_dir_t
 * @nbytes: Number of bytes to be mapped
 * @paddr: Mapped physical address
 *
 * Return: QDF_STATUS
 */
QDF_STATUS __qdf_mem_map_page(qdf_device_t osdev, __qdf_frag_t buf,
			      qdf_dma_dir_t dir, size_t nbytes,
			      qdf_dma_addr_t *phy_addr);

/**
 * __qdf_frag_free() - Free allocated frag memory
 * @vaddr: Frag address to be freed
 *
 * Return: none
 */
static inline void __qdf_frag_free(__qdf_frag_t vaddr)
{
	if (qdf_likely(vaddr)) {
		skb_free_frag(vaddr);
		__qdf_frag_count_dec(QDF_NBUF_FRAG_DEBUG_COUNT_ONE);
	}
}

/**
 * __qdf_frag_alloc() - Allocate frag Memory
 * @fragsz: Size of frag memory to be allocated
 *
 * Return: Allocated frag addr.
 */
static inline __qdf_frag_t __qdf_frag_alloc(unsigned int fragsz)
{
	__qdf_frag_t p_frag = netdev_alloc_frag(fragsz);

	if (p_frag)
		__qdf_frag_count_inc(QDF_NBUF_FRAG_DEBUG_COUNT_ONE);
	return p_frag;
}

#endif /* _I_QDF_NBUF_FRAG_H */
