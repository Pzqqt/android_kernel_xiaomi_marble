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

/**
 * typedef __qdf_frag_t - Abstraction for void * for frag address
 */
typedef void *__qdf_frag_t;

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
 */
static inline void __qdf_frag_free(__qdf_frag_t vaddr)
{
	skb_free_frag(vaddr);
}

/**
 * __qdf_frag_alloc() - Allocate frag Memory
 * @fragsz: Size of frag memory to be allocated
 *
 * Return: Allocated frag addr.
 */
static inline __qdf_frag_t __qdf_frag_alloc(unsigned int fragsz)
{
	return netdev_alloc_frag(fragsz);
}
#endif /* _I_QDF_NBUF_FRAG_H */
