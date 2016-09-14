/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_mem.h
 * Linux-specific definitions for QDF memory API's
 */

#ifndef __I_QDF_MEM_H
#define __I_QDF_MEM_H

#ifdef __KERNEL__
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 17)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#endif
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/vmalloc.h>
#include <linux/pci.h> /* pci_alloc_consistent */
#if CONFIG_MCL
#include <cds_queue.h>
#else
#include <sys/queue.h>
#endif
#else
/*
 * Provide dummy defs for kernel data types, functions, and enums
 * used in this header file.
 */
#define GFP_KERNEL 0
#define GFP_ATOMIC 0
#define kzalloc(size, flags) NULL
#define vmalloc(size)        NULL
#define kfree(buf)
#define vfree(buf)
#define pci_alloc_consistent(dev, size, paddr) NULL
#define __qdf_mempool_t void*
#endif /* __KERNEL__ */
#include <qdf_status.h>

#ifdef __KERNEL__
typedef struct mempool_elem {
	STAILQ_ENTRY(mempool_elem) mempool_entry;
} mempool_elem_t;

/**
 * typedef __qdf_mempool_ctxt_t - Memory pool context
 * @pool_id: pool identifier
 * @flags: flags
 * @elem_size: size of each pool element in bytes
 * @pool_mem: pool_addr address of the pool created
 * @mem_size: Total size of the pool in bytes
 * @free_list: free pool list
 * @lock: spinlock object
 * @max_elem: Maximum number of elements in tha pool
 * @free_cnt: Number of free elements available
 */
typedef struct __qdf_mempool_ctxt {
	int pool_id;
	u_int32_t flags;
	size_t elem_size;
	void *pool_mem;
	u_int32_t mem_size;
	STAILQ_HEAD(, mempool_elem) free_list;
	spinlock_t lock;
	u_int32_t max_elem;
	u_int32_t free_cnt;
} __qdf_mempool_ctxt_t;

#endif /* __KERNEL__ */

/* typedef for dma_data_direction */
typedef enum dma_data_direction __dma_data_direction;

/**
 * __qdf_str_cmp() - Compare two strings
 * @str1: First string
 * @str2: Second string
 *
 * Return: =0 equal
 * >0 not equal, if  str1  sorts lexicographically after str2
 * <0 not equal, if  str1  sorts lexicographically before str2
 */
static inline int32_t __qdf_str_cmp(const char *str1, const char *str2)
{
	return strcmp(str1, str2);
}

/**
 * __qdf_str_lcopy() - Copy from one string to another
 * @dest: destination string
 * @src: source string
 * @bytes: limit of num bytes to copy
 *
 * @return: 0 returns the initial value of dest
 */
static inline uint32_t __qdf_str_lcopy(char *dest, const char *src,
				    uint32_t bytes)
{
	return strlcpy(dest, src, bytes);
}

/**
 * __qdf_mem_map_nbytes_single - Map memory for DMA
 * @osdev: pomter OS device context
 * @buf: pointer to memory to be dma mapped
 * @dir: DMA map direction
 * @nbytes: number of bytes to be mapped.
 * @phy_addr: ponter to recive physical address.
 *
 * Return: success/failure
 */
static inline uint32_t __qdf_mem_map_nbytes_single(qdf_device_t osdev,
						  void *buf, qdf_dma_dir_t dir,
						  int nbytes,
						  uint32_t *phy_addr)
{
	/* assume that the OS only provides a single fragment */
	*phy_addr = dma_map_single(osdev->dev, buf, nbytes, dir);
	return dma_mapping_error(osdev->dev, *phy_addr) ?
	QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/**
 * __qdf_mem_unmap_nbytes_single() - un_map memory for DMA
 *
 * @osdev: pomter OS device context
 * @phy_addr: physical address of memory to be dma unmapped
 * @dir: DMA unmap direction
 * @nbytes: number of bytes to be unmapped.
 *
 * @return - none
 */
static inline void __qdf_mem_unmap_nbytes_single(qdf_device_t osdev,
						 uint32_t phy_addr,
						 qdf_dma_dir_t dir, int nbytes)
{
	dma_unmap_single(osdev->dev, phy_addr, nbytes, dir);
}
#ifdef __KERNEL__

typedef __qdf_mempool_ctxt_t *__qdf_mempool_t;

int __qdf_mempool_init(qdf_device_t osdev, __qdf_mempool_t *pool, int pool_cnt,
		       size_t pool_entry_size, u_int32_t flags);
void __qdf_mempool_destroy(qdf_device_t osdev, __qdf_mempool_t pool);
void *__qdf_mempool_alloc(qdf_device_t osdev, __qdf_mempool_t pool);
void __qdf_mempool_free(qdf_device_t osdev, __qdf_mempool_t pool, void *buf);

#define __qdf_mempool_elem_size(_pool) ((_pool)->elem_size);
#endif

/**
 * __qdf_str_len() - returns the length of a string
 * @str: input string
 * Return:
 * length of string
 */
static inline int32_t __qdf_str_len(const char *str)
{
	return strlen(str);
}

/**
 * __qdf_mem_cmp() - memory compare
 * @memory1: pointer to one location in memory to compare.
 * @memory2: pointer to second location in memory to compare.
 * @num_bytes: the number of bytes to compare.
 *
 * Function to compare two pieces of memory, similar to memcmp function
 * in standard C.
 * Return:
 * int32_t - returns an int value that tells if the memory
 * locations are equal or not equal.
 * 0 -- equal
 * < 0 -- *memory1 is less than *memory2
 * > 0 -- *memory1 is bigger than *memory2
 */
static inline int32_t __qdf_mem_cmp(const void *memory1, const void *memory2,
				    uint32_t num_bytes)
{
	return (int32_t) memcmp(memory1, memory2, num_bytes);
}

#endif /* __I_QDF_MEM_H */
