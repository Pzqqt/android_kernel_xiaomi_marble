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
 * DOC: qdf_nbuf_frag.h
 * This file defines the nbuf frag abstraction.
 */

#ifndef _QDF_NBUF_FRAG_H
#define _QDF_NBUF_FRAG_H

#include <i_qdf_trace.h>
#include <i_qdf_nbuf_frag.h>

/*
 * typedef qdf_frag_t - Platform independent frag address abstraction
 */
typedef __qdf_frag_t qdf_frag_t;

/**
 * Maximum number of frags an SKB can hold
 */
#define QDF_NBUF_MAX_FRAGS __QDF_NBUF_MAX_FRAGS

/**
 * qdf_mem_map_page() - Map Page
 * @osdev: qdf_device_t
 * @buf: Virtual page address to be mapped
 * @dir: qdf_dma_dir_t
 * @nbytes: Size of memory to be mapped
 * @paddr: Corresponding mapped physical address
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS qdf_mem_map_page(qdf_device_t osdev, qdf_frag_t buf,
					  qdf_dma_dir_t dir, size_t nbytes,
					  qdf_dma_addr_t *phy_addr)
{
	return __qdf_mem_map_page(osdev, buf, dir, nbytes, phy_addr);
}

/**
 * qdf_mem_unmap_page() - Unmap Page
 * @osdev: qdf_device_t
 * @paddr: Physical memory to be unmapped
 * @nbytes: Size of memory to be unmapped
 * @dir: qdf_dma_dir_t
 */
static inline void qdf_mem_unmap_page(qdf_device_t osdev, qdf_dma_addr_t paddr,
				      size_t nbytes, qdf_dma_dir_t dir)
{
	__qdf_mem_unmap_page(osdev, paddr, nbytes, dir);
}

/**
 * qdf_frag_free() - Free allocated frag memory
 * @vaddr: Frag address to be freed.
 */
static inline void qdf_frag_free(qdf_frag_t vaddr)
{
	__qdf_frag_free(vaddr);
}

/**
 * qdf_frag_alloc() - Allocate frag memory
 * @fragsz: Size of frag memory to be allocated
 *
 * Return: Allcated frag address
 */
static inline qdf_frag_t qdf_frag_alloc(unsigned int fragsz)
{
	return __qdf_frag_alloc(fragsz);
}
#endif /* _QDF_NBUF_FRAG_H */
