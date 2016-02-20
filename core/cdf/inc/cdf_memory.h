/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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

#if !defined(__CDF_MEMORY_H)
#define __CDF_MEMORY_H

/**
 * DOC: cdf_memory
 *
 * Connectivity driver framework (CDF) memory management APIs
 */

/* Include Files */
#include <qdf_types.h>

/**
 * struct cdf_mem_dma_page_t - Allocated dmaable page
 * @page_v_addr_start: Page start virtual address
 * @page_v_addr_end: Page end virtual address
 * @page_p_addr: Page start physical address
 */
struct cdf_mem_dma_page_t {
	char *page_v_addr_start;
	char *page_v_addr_end;
	qdf_dma_addr_t page_p_addr;
};

/**
 * struct cdf_mem_multi_page_t - multiple page allocation information storage
 * @num_element_per_page: Number of element in single page
 * @num_pages: Number of allocation needed pages
 * @dma_pages: page information storage in case of coherent memory
 * @cacheable_pages: page information storage in case of cacheable memory
 */
struct cdf_mem_multi_page_t {
	uint16_t num_element_per_page;
	uint16_t num_pages;
	struct cdf_mem_dma_page_t *dma_pages;
	void **cacheable_pages;
};

/* Preprocessor definitions and constants */

#ifdef MEMORY_DEBUG
void cdf_mem_clean(void);
void cdf_mem_init(void);
void cdf_mem_exit(void);
#else
/**
 * cdf_mem_init() - initialize cdf memory debug functionality
 *
 * Return: none
 */
static inline void cdf_mem_init(void)
{
}

/**
 * cdf_mem_exit() - exit cdf memory debug functionality
 *
 * Return: none
 */
static inline void cdf_mem_exit(void)
{
}
#endif
/* Type declarations */

/* Function declarations and documenation */

/**
 * cdf_mem_malloc() - allocation CDF memory
 * @size:	Number of bytes of memory to allocate.
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory.
 *
 *
 * Return:
 *	Upon successful allocate, returns a non-NULL pointer to the allocated
 *	memory.  If this function is unable to allocate the amount of memory
 *	specified (for any reason) it returns %NULL.
 *
 */
#ifdef MEMORY_DEBUG
#define cdf_mem_malloc(size) cdf_mem_malloc_debug(size, __FILE__, __LINE__)
void *cdf_mem_malloc_debug(size_t size, char *fileName, uint32_t lineNum);
#else
void *cdf_mem_malloc(size_t size);
#endif

/**
 *  cdf_mem_free() - free CDF memory
 *  @ptr:	Pointer to the starting address of the memory to be free'd.
 *
 *  This function will free the memory pointed to by 'ptr'.
 *
 *  Return:
 *	 Nothing
 *
 */
void cdf_mem_free(void *ptr);

/**
 * cdf_mem_set() - set (fill) memory with a specified byte value.
 * @pMemory:	Pointer to memory that will be set
 * @numBytes:	Number of bytes to be set
 * @value:	Byte set in memory
 *
 * Return:
 *    Nothing
 *
 */
void cdf_mem_set(void *ptr, uint32_t numBytes, uint32_t value);

/**
 * cdf_mem_zero() - zero out memory
 * @pMemory:	pointer to memory that will be set to zero
 * @numBytes:	number of bytes zero
 * @value:	byte set in memory
 *
 *  This function sets the memory location to all zeros, essentially clearing
 *  the memory.
 *
 * Return:
 *	Nothing
 *
 */
void cdf_mem_zero(void *ptr, uint32_t numBytes);

/**
 * cdf_mem_copy() - copy memory
 * @pDst:	Pointer to destination memory location (to copy to)
 * @pSrc:	Pointer to source memory location (to copy from)
 * @numBytes:	Number of bytes to copy.
 *
 * Copy host memory from one location to another, similar to memcpy in
 * standard C.  Note this function does not specifically handle overlapping
 * source and destination memory locations.  Calling this function with
 * overlapping source and destination memory locations will result in
 * unpredictable results.  Use cdf_mem_move() if the memory locations
 * for the source and destination are overlapping (or could be overlapping!)
 *
 * Return:
 *    Nothing
 *
 */
void cdf_mem_copy(void *pDst, const void *pSrc, uint32_t numBytes);

/**
 * cdf_mem_move() - move memory
 * @pDst:	pointer to destination memory location (to move to)
 * @pSrc:	pointer to source memory location (to move from)
 * @numBytes:	number of bytes to move.
 *
 * Move host memory from one location to another, similar to memmove in
 * standard C.  Note this function *does* handle overlapping
 * source and destination memory locations.

 * Return:
 *	Nothing
 */
void cdf_mem_move(void *pDst, const void *pSrc, uint32_t numBytes);

/**
 * cdf_mem_compare() - memory compare
 * @pMemory1:	pointer to one location in memory to compare.
 * @pMemory2:	pointer to second location in memory to compare.
 * @numBytes:	the number of bytes to compare.
 *
 * Function to compare two pieces of memory, similar to memcmp function
 * in standard C.
 *
 * Return:
 *	bool - returns a bool value that tells if the memory locations
 *	are equal or not equal.
 *
 */
bool cdf_mem_compare(const void *pMemory1, const void *pMemory2,
		     uint32_t numBytes);

/**
 * cdf_mem_compare2() - memory compare
 * @pMemory1: pointer to one location in memory to compare.
 * @pMemory2:	pointer to second location in memory to compare.
 * @numBytes:	the number of bytes to compare.
 *
 * Function to compare two pieces of memory, similar to memcmp function
 * in standard C.
 * Return:
 *	 int32_t - returns a bool value that tells if the memory
 *	 locations are equal or not equal.
 *	 0 -- equal
 *	 < 0 -- *pMemory1 is less than *pMemory2
 *	 > 0 -- *pMemory1 is bigger than *pMemory2
 */
int32_t cdf_mem_compare2(const void *pMemory1, const void *pMemory2,
			 uint32_t numBytes);

void *cdf_os_mem_alloc_consistent(qdf_device_t osdev, qdf_size_t size,
				  qdf_dma_addr_t *paddr,
				  qdf_dma_context_t mctx);
void
cdf_os_mem_free_consistent(qdf_device_t osdev,
			   qdf_size_t size,
			   void *vaddr,
			   qdf_dma_addr_t paddr, qdf_dma_context_t memctx);

void
cdf_os_mem_dma_sync_single_for_device(qdf_device_t osdev,
				      qdf_dma_addr_t bus_addr,
				      qdf_size_t size,
				      enum dma_data_direction direction);

/**
 * cdf_str_len() - returns the length of a string
 * @str:	input string
 *
 * Return:
 *	length of string
 */
static inline int32_t cdf_str_len(const char *str)
{
	return strlen(str);
}

void cdf_mem_multi_pages_alloc(qdf_device_t osdev,
				struct cdf_mem_multi_page_t *pages,
				size_t element_size,
				uint16_t element_num,
				qdf_dma_context_t memctxt,
				bool cacheable);

void cdf_mem_multi_pages_free(qdf_device_t osdev,
				struct cdf_mem_multi_page_t *pages,
				qdf_dma_context_t memctxt,
				bool cacheable);
#endif /* __CDF_MEMORY_H */
