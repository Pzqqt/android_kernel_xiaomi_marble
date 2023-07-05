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

#include <qdf_util.h>
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

#ifdef NBUF_FRAG_MEMORY_DEBUG
/**
 * qdf_frag_debug_init() - Initialize frag debug tracker
 *
 * Return: none
 */
void qdf_frag_debug_init(void);

/**
 * qdf_frag_debug_exit() - Destroy frag debug tracker
 *
 * Return: none
 */
void qdf_frag_debug_exit(void);

/**
 * qdf_frag_debug_add_node() - Add frag node in the debug hash table
 * @fragp: Pointer to frag
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: none
 */
void qdf_frag_debug_add_node(qdf_frag_t fragp, const char *func_name,
			     uint32_t line_num);

/**
 * qdf_frag_debug_refcount_inc() - Increment refcount for frag node
 * @fragp: Pointer to frag
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: none
 */
void qdf_frag_debug_refcount_inc(qdf_frag_t fragp, const char *func_name,
				 uint32_t line_num);

/**
 * qdf_frag_debug_refcount_dec() - Decrement refcount for frag node
 * @fragp: Pointer to frag
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: none
 */
void qdf_frag_debug_refcount_dec(qdf_frag_t fragp, const char *func_name,
				 uint32_t line_num);

/**
 * qdf_frag_debug_delete_node() - Remove frag node from debug hash table
 * @fragp: Pointer to frag
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: none
 */
void qdf_frag_debug_delete_node(qdf_frag_t fragp, const char *func_name,
				uint32_t line_num);

/**
 * qdf_frag_debug_update_addr() - Update frag address in debug tracker
 * @p_fragp: Previous frag address
 * @n_fragp: New frag address
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: none
 */
void qdf_frag_debug_update_addr(qdf_frag_t p_fragp, qdf_frag_t n_fragp,
				const char *func_name, uint32_t line_num);

#define qdf_frag_alloc(s) \
	qdf_frag_alloc_debug(s, __func__, __LINE__)

/**
 * qdf_frag_alloc_debug() - Allocate frag memory
 * @fragsz: Size of frag memory to be allocated
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: Allocated frag address
 */
qdf_frag_t qdf_frag_alloc_debug(unsigned int fragsz, const char *func_name,
				uint32_t line_num);

#define qdf_frag_free(p) \
	qdf_frag_free_debug(p, __func__, __LINE__)

/**
 * qdf_frag_free_debug() - Free allocated frag memory
 * @vaddr: Frag address to be freed
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: none
 */
void qdf_frag_free_debug(qdf_frag_t vaddr, const char *func_name,
			 uint32_t line_num);

#else /* NBUF_FRAG_MEMORY_DEBUG */

static inline void qdf_frag_debug_init(void)
{
}

static inline void qdf_frag_debug_exit(void)
{
}

static inline void qdf_frag_debug_add_node(qdf_frag_t fragp,
					   const char *func_name,
					   uint32_t line_num)
{
}

static inline void qdf_frag_debug_refcount_inc(qdf_frag_t fragp,
					       const char *func_name,
					       uint32_t line_num)
{
}

static inline void qdf_frag_debug_refcount_dec(qdf_frag_t fragp,
					       const char *func_name,
					       uint32_t line_num)
{
}

static inline void qdf_frag_debug_delete_node(qdf_frag_t fragp,
					      const char *func_name,
					      uint32_t line_num)
{
}

static inline void qdf_frag_debug_update_addr(qdf_frag_t p_fragp,
					      qdf_frag_t n_fragp,
					      const char *func_name,
					      uint32_t line_num)
{
}

/**
 * qdf_frag_alloc() - Allocate frag memory
 * @fragsz: Size of frag memory to be allocated
 *
 * Return: Allocated frag address
 */
static inline qdf_frag_t qdf_frag_alloc(unsigned int fragsz)
{
	return __qdf_frag_alloc(fragsz);
}

/**
 * qdf_frag_free() - Free allocated frag memory
 * @vaddr: Frag address to be freed
 *
 * Return: none
 */
static inline void qdf_frag_free(qdf_frag_t vaddr)
{
	__qdf_frag_free(vaddr);
}

#endif /* NBUF_FRAG_MEMORY_DEBUG */

/**
 * qdf_frag_count_get() - Get global frag gauge
 *
 * Return: Global frag gauge
 */
static inline uint32_t qdf_frag_count_get(void)
{
	return __qdf_frag_count_get();
}

/**
 * qdf_frag_count_inc() - Increment global frag count
 * @value: Increment value
 *
 * Return: none
 */
static inline void qdf_frag_count_inc(uint32_t value)
{
	return __qdf_frag_count_inc(value);
}

/**
 * qdf_frag_count_dec() - Decrement global frag count
 * @value: Decrement value
 *
 * Return: none
 */
static inline void qdf_frag_count_dec(uint32_t value)
{
	return __qdf_frag_count_dec(value);
}

/**
 * qdf_frag_mod_init() - Initialization routine for qdf_frag
 *
 * Return: none
 */
static inline void qdf_frag_mod_init(void)
{
	return __qdf_frag_mod_init();
}

/**
 * qdf_frag_mod_exit() - Unintialization routine for qdf_frag
 *
 * Return: none
 */
static inline void qdf_frag_mod_exit(void)
{
	return __qdf_frag_mod_exit();
}

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

#endif /* _QDF_NBUF_FRAG_H */
