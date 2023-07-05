/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_mem
 * QCA driver framework (QDF) memory management APIs
 */

#if !defined(__QDF_MEMORY_H)
#define __QDF_MEMORY_H

/* Include Files */
#include <qdf_types.h>
#include <i_qdf_mem.h>
#include <i_qdf_trace.h>
#include <qdf_atomic.h>

#define QDF_CACHE_LINE_SZ __qdf_cache_line_sz

/**
 * qdf_align() - align to the given size.
 * @a: input that needs to be aligned.
 * @align_size: boundary on which 'a' has to be alinged.
 *
 * Return: aligned value.
 */
#define qdf_align(a, align_size)   __qdf_align(a, align_size)
#define qdf_page_size __page_size

/**
 * struct qdf_mem_dma_page_t - Allocated dmaable page
 * @page_v_addr_start: Page start virtual address
 * @page_v_addr_end: Page end virtual address
 * @page_p_addr: Page start physical address
 */
struct qdf_mem_dma_page_t {
	char *page_v_addr_start;
	char *page_v_addr_end;
	qdf_dma_addr_t page_p_addr;
};

/**
 * struct qdf_mem_multi_page_t - multiple page allocation information storage
 * @num_element_per_page: Number of element in single page
 * @num_pages: Number of allocation needed pages
 * @dma_pages: page information storage in case of coherent memory
 * @cacheable_pages: page information storage in case of cacheable memory
 * @is_mem_prealloc: flag for multiple pages pre-alloc or not
 */
struct qdf_mem_multi_page_t {
	uint16_t num_element_per_page;
	uint16_t num_pages;
	struct qdf_mem_dma_page_t *dma_pages;
	void **cacheable_pages;
	qdf_size_t page_size;
#ifdef DP_MEM_PRE_ALLOC
	uint8_t is_mem_prealloc;
#endif
};


/* Preprocessor definitions and constants */

typedef __qdf_mempool_t qdf_mempool_t;

/**
 * qdf_mem_init() - Initialize QDF memory module
 *
 * Return: None
 *
 */
void qdf_mem_init(void);

/**
 * qdf_mem_exit() - Exit QDF memory module
 *
 * Return: None
 *
 */
void qdf_mem_exit(void);

#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
#define qdf_untracked_mem_malloc(size) \
	__qdf_untracked_mem_malloc(size, __func__, __LINE__)

#define qdf_untracked_mem_free(ptr) \
	__qdf_untracked_mem_free(ptr)
#endif

#define QDF_MEM_FUNC_NAME_SIZE 48

#ifdef MEMORY_DEBUG
/**
 * qdf_mem_debug_config_get() - Get the user configuration of mem_debug_disabled
 *
 * Return: value of mem_debug_disabled qdf module argument
 */
bool qdf_mem_debug_config_get(void);

#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
/**
 * qdf_mem_debug_disabled_set() - Set mem_debug_disabled
 * @str_value: value of the module param
 *
 * This function will set qdf module param mem_debug_disabled
 *
 * Return: QDF_STATUS_SUCCESS on Success
 */
QDF_STATUS qdf_mem_debug_disabled_config_set(const char *str_value);
#endif

/**
 * qdf_mem_malloc_debug() - debug version of QDF memory allocation API
 * @size: Number of bytes of memory to allocate.
 * @func: Function name of the call site
 * @line: Line number of the call site
 * @caller: Address of the caller function
 * @flag: GFP flag
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory and add it to the qdf tracking list to check for memory leaks and
 * corruptions
 *
 * Return: A valid memory location on success, or NULL on failure
 */
void *qdf_mem_malloc_debug(size_t size, const char *func, uint32_t line,
			   void *caller, uint32_t flag);

#define qdf_mem_malloc(size) \
	qdf_mem_malloc_debug(size, __func__, __LINE__, QDF_RET_IP, 0)

#define qdf_mem_malloc_fl(size, func, line) \
	qdf_mem_malloc_debug(size, func, line, QDF_RET_IP, 0)

#define qdf_mem_malloc_atomic(size) \
	qdf_mem_malloc_debug(size, __func__, __LINE__, QDF_RET_IP, GFP_ATOMIC)
/**
 * qdf_mem_free_debug() - debug version of qdf_mem_free
 * @ptr: Pointer to the starting address of the memory to be freed.
 *
 * This function will free the memory pointed to by 'ptr'. It also checks for
 * memory corruption, underrun, overrun, double free, domain mismatch, etc.
 *
 * Return: none
 */
void qdf_mem_free_debug(void *ptr, const char *file, uint32_t line);

#define qdf_mem_free(ptr) \
	qdf_mem_free_debug(ptr, __func__, __LINE__)

void qdf_mem_multi_pages_alloc_debug(qdf_device_t osdev,
				     struct qdf_mem_multi_page_t *pages,
				     size_t element_size, uint16_t element_num,
				     qdf_dma_context_t memctxt, bool cacheable,
				     const char *func, uint32_t line,
				     void *caller);

#define qdf_mem_multi_pages_alloc(osdev, pages, element_size, element_num,\
				  memctxt, cacheable) \
	qdf_mem_multi_pages_alloc_debug(osdev, pages, element_size, \
					element_num, memctxt, cacheable, \
					__func__, __LINE__, QDF_RET_IP)

void qdf_mem_multi_pages_free_debug(qdf_device_t osdev,
				    struct qdf_mem_multi_page_t *pages,
				    qdf_dma_context_t memctxt, bool cacheable,
				    const char *func, uint32_t line);

#define qdf_mem_multi_pages_free(osdev, pages, memctxt, cacheable) \
	qdf_mem_multi_pages_free_debug(osdev, pages, memctxt, cacheable, \
				       __func__, __LINE__)

/**
 * qdf_mem_check_for_leaks() - Assert that the current memory domain is empty
 *
 * Call this to ensure there are no active memory allocations being tracked
 * against the current debug domain. For example, one should call this function
 * immediately before a call to qdf_debug_domain_set() as a memory leak
 * detection mechanism.
 *
 * e.g.
 *	qdf_debug_domain_set(QDF_DEBUG_DOMAIN_ACTIVE);
 *
 *	...
 *
 *	// memory is allocated and freed
 *
 *	...
 *
 *	// before transitioning back to inactive state,
 *	// make sure all active memory has been freed
 *	qdf_mem_check_for_leaks();
 *	qdf_debug_domain_set(QDF_DEBUG_DOMAIN_INIT);
 *
 *	...
 *
 *	// also, before program exit, make sure init time memory is freed
 *	qdf_mem_check_for_leaks();
 *	exit();
 *
 * Return: None
 */
void qdf_mem_check_for_leaks(void);

/**
 * qdf_mem_alloc_consistent_debug() - allocates consistent qdf memory
 * @osdev: OS device handle
 * @dev: Pointer to device handle
 * @size: Size to be allocated
 * @paddr: Physical address
 * @func: Function name of the call site
 * @line: line numbe rof the call site
 * @caller: Address of the caller function
 *
 * Return: pointer of allocated memory or null if memory alloc fails
 */
void *qdf_mem_alloc_consistent_debug(qdf_device_t osdev, void *dev,
				     qdf_size_t size, qdf_dma_addr_t *paddr,
				     const char *func, uint32_t line,
				     void *caller);

#define qdf_mem_alloc_consistent(osdev, dev, size, paddr) \
	qdf_mem_alloc_consistent_debug(osdev, dev, size, paddr, \
				       __func__, __LINE__, QDF_RET_IP)

/**
 * qdf_mem_free_consistent_debug() - free consistent qdf memory
 * @osdev: OS device handle
 * @size: Size to be allocated
 * @vaddr: virtual address
 * @paddr: Physical address
 * @memctx: Pointer to DMA context
 * @func: Function name of the call site
 * @line: line numbe rof the call site
 *
 * Return: none
 */
void qdf_mem_free_consistent_debug(qdf_device_t osdev, void *dev,
				   qdf_size_t size, void *vaddr,
				   qdf_dma_addr_t paddr,
				   qdf_dma_context_t memctx,
				   const char *func, uint32_t line);

#define qdf_mem_free_consistent(osdev, dev, size, vaddr, paddr, memctx) \
	qdf_mem_free_consistent_debug(osdev, dev, size, vaddr, paddr, memctx, \
				  __func__, __LINE__)
#else
static inline bool qdf_mem_debug_config_get(void)
{
	return false;
}

static inline
QDF_STATUS qdf_mem_debug_disabled_config_set(const char *str_value)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * qdf_mem_malloc() - allocation QDF memory
 * @size: Number of bytes of memory to allocate.
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory.
 *
 * Return:
 * Upon successful allocate, returns a non-NULL pointer to the allocated
 * memory.  If this function is unable to allocate the amount of memory
 * specified (for any reason) it returns NULL.
 */
#define qdf_mem_malloc(size) \
	__qdf_mem_malloc(size, __func__, __LINE__)

#define qdf_mem_malloc_fl(size, func, line) \
	__qdf_mem_malloc(size, func, line)

/**
 * qdf_mem_malloc_atomic() - allocation QDF memory atomically
 * @size: Number of bytes of memory to allocate.
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory.
 *
 * Return:
 * Upon successful allocate, returns a non-NULL pointer to the allocated
 * memory.  If this function is unable to allocate the amount of memory
 * specified (for any reason) it returns NULL.
 */
#define qdf_mem_malloc_atomic(size) \
	qdf_mem_malloc_atomic_fl(size, __func__, __LINE__)

void *qdf_mem_malloc_atomic_fl(qdf_size_t size,
			       const char *func,
			       uint32_t line);

#define qdf_mem_free(ptr) \
	__qdf_mem_free(ptr)

static inline void qdf_mem_check_for_leaks(void) { }

#define qdf_mem_alloc_consistent(osdev, dev, size, paddr) \
	__qdf_mem_alloc_consistent(osdev, dev, size, paddr, __func__, __LINE__)

#define qdf_mem_free_consistent(osdev, dev, size, vaddr, paddr, memctx) \
	__qdf_mem_free_consistent(osdev, dev, size, vaddr, paddr, memctx)

void qdf_mem_multi_pages_alloc(qdf_device_t osdev,
			       struct qdf_mem_multi_page_t *pages,
			       size_t element_size, uint16_t element_num,
			       qdf_dma_context_t memctxt, bool cacheable);

void qdf_mem_multi_pages_free(qdf_device_t osdev,
			      struct qdf_mem_multi_page_t *pages,
			      qdf_dma_context_t memctxt, bool cacheable);

#endif /* MEMORY_DEBUG */

/**
 * qdf_mem_malloc_flags: Get mem allocation flags
 *
 * Return the flag to be use for memory allocation
 * based on the context
 *
 * Returns: Based on the context, returns the GFP flag
 * for memory alloaction
 */
int qdf_mem_malloc_flags(void);

/**
 * qdf_prealloc_disabled_config_get() - Get the user configuration of
 *                                      prealloc_disabled
 *
 * Return: value of prealloc_disabled qdf module argument
 */
bool qdf_prealloc_disabled_config_get(void);

#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
/**
 * qdf_prealloc_disabled_config_set() - Set prealloc_disabled
 * @str_value: value of the module param
 *
 * This function will set qdf module param prealloc_disabled
 *
 * Return: QDF_STATUS_SUCCESS on Success
 */
QDF_STATUS qdf_prealloc_disabled_config_set(const char *str_value);
#endif

/**
 * qdf_mem_multi_pages_zero() - zero out each page memory
 * @pages: Multi page information storage
 * @cacheable: Coherent memory or cacheable memory
 *
 * This function will zero out each page memory
 *
 * Return: None
 */
void qdf_mem_multi_pages_zero(struct qdf_mem_multi_page_t *pages,
			      bool cacheable);

/**
 * qdf_aligned_malloc() - allocates aligned QDF memory.
 * @size: Size to be allocated
 * @vaddr_unaligned: Unaligned virtual address.
 * @paddr_unaligned: Unaligned physical address.
 * @paddr_aligned: Aligned physical address.
 * @align: Base address alignment.
 * @func: Function name of the call site.
 * @line: Line number of the call site.
 *
 * This function will dynamically allocate the specified number of bytes of
 * memory. Checks if the allocated base address is aligned with base_align.
 * If not, it frees the allocated memory, adds base_align to alloc size and
 * re-allocates the memory.
 *
 * Return:
 * Upon successful allocate, returns an aligned base address of the allocated
 * memory.  If this function is unable to allocate the amount of memory
 * specified (for any reason) it returns NULL.
 */
#define qdf_aligned_malloc(size, vaddr_unaligned, paddr_unaligned, \
			   paddr_aligned, align) \
	qdf_aligned_malloc_fl(size, vaddr_unaligned, paddr_unaligned, \
			   paddr_aligned, align, __func__, __LINE__)

void *qdf_aligned_malloc_fl(uint32_t *size, void **vaddr_unaligned,
			    qdf_dma_addr_t *paddr_unaligned,
			    qdf_dma_addr_t *paddr_aligned,
			    uint32_t align,
			    const char *func, uint32_t line);

/**
 * qdf_aligned_mem_alloc_consistent() - allocates consistent qdf memory
 * @osdev: OS device handle
 * @size: Size to be allocated
 * @vaddr_unaligned: Unaligned virtual address.
 * @paddr_unaligned: Unaligned physical address.
 * @paddr_aligned: Aligned physical address.
 * @align: Base address alignment.
 * @func: Function name of the call site.
 * @line: Line number of the call site.
 *
 * Return: pointer of allocated memory or null if memory alloc fails.
 */
#define qdf_aligned_mem_alloc_consistent(osdev, size, vaddr_unaligned, \
					 paddr_unaligned, paddr_aligned, \
					 align) \
	qdf_aligned_mem_alloc_consistent_fl(osdev, size, vaddr_unaligned, \
					    paddr_unaligned, paddr_aligned, \
					    align, __func__, __LINE__)

void *qdf_aligned_mem_alloc_consistent_fl(qdf_device_t osdev, uint32_t *size,
					  void **vaddr_unaligned,
					  qdf_dma_addr_t *paddr_unaligned,
					  qdf_dma_addr_t *paddr_aligned,
					  uint32_t align, const char *func,
					  uint32_t line);

#define qdf_mem_virt_to_phys(vaddr) virt_to_phys(vaddr)

void qdf_mem_set_io(void *ptr, uint32_t num_bytes, uint32_t value);

void qdf_mem_copy_toio(void *dst_addr, const void *src_addr,
					   uint32_t num_bytes);

/**
 * qdf_mem_set() - set (fill) memory with a specified byte value.
 * @ptr: Pointer to memory that will be set
 * @num_bytes: Number of bytes to be set
 * @value: Byte set in memory
 *
 * WARNING: parameter @num_bytes and @value are swapped comparing with
 * standard C function "memset", please ensure correct usage of this function!
 *
 * Return: None
 */
void qdf_mem_set(void *ptr, uint32_t num_bytes, uint32_t value);

/**
 * qdf_mem_zero() - zero out memory
 * @ptr: pointer to memory that will be set to zero
 * @num_bytes: number of bytes zero
 *
 * This function sets the memory location to all zeros, essentially clearing
 * the memory.
 *
 * Return: None
 */
static inline void qdf_mem_zero(void *ptr, uint32_t num_bytes)
{
	qdf_mem_set(ptr, num_bytes, 0);
}

/**
 * qdf_mem_copy() - copy memory
 * @dst_addr: Pointer to destination memory location (to copy to)
 * @src_addr: Pointer to source memory location (to copy from)
 * @num_bytes: Number of bytes to copy.
 *
 * Copy host memory from one location to another, similar to memcpy in
 * standard C.  Note this function does not specifically handle overlapping
 * source and destination memory locations.  Calling this function with
 * overlapping source and destination memory locations will result in
 * unpredictable results.  Use qdf_mem_move() if the memory locations
 * for the source and destination are overlapping (or could be overlapping!)
 *
 * Return: none
 */
void qdf_mem_copy(void *dst_addr, const void *src_addr, uint32_t num_bytes);

/**
 * qdf_mem_move() - move memory
 * @dst_addr: pointer to destination memory location (to move to)
 * @src_addr: pointer to source memory location (to move from)
 * @num_bytes: number of bytes to move.
 *
 * Move host memory from one location to another, similar to memmove in
 * standard C.  Note this function *does* handle overlapping
 * source and destination memory locations.

 * Return: None
 */
void qdf_mem_move(void *dst_addr, const void *src_addr, uint32_t num_bytes);

/**
 * qdf_mem_cmp() - memory compare
 * @left: pointer to one location in memory to compare
 * @right: pointer to second location in memory to compare
 * @size: the number of bytes to compare
 *
 * Function to compare two pieces of memory, similar to memcmp function
 * in standard C.
 *
 * Return:
 *	0 -- equal
 *	< 0 -- *memory1 is less than *memory2
 *	> 0 -- *memory1 is bigger than *memory2
 */
int qdf_mem_cmp(const void *left, const void *right, size_t size);

void qdf_ether_addr_copy(void *dst_addr, const void *src_addr);

/**
 * qdf_mem_map_nbytes_single - Map memory for DMA
 * @osdev: pomter OS device context
 * @buf: pointer to memory to be dma mapped
 * @dir: DMA map direction
 * @nbytes: number of bytes to be mapped.
 * @phy_addr: ponter to recive physical address.
 *
 * Return: success/failure
 */
static inline uint32_t qdf_mem_map_nbytes_single(qdf_device_t osdev, void *buf,
						 qdf_dma_dir_t dir, int nbytes,
						 qdf_dma_addr_t *phy_addr)
{
#if defined(HIF_PCI) || defined(HIF_IPCI)
	return __qdf_mem_map_nbytes_single(osdev, buf, dir, nbytes, phy_addr);
#else
	return 0;
#endif
}

static inline void qdf_mem_dma_cache_sync(qdf_device_t osdev,
					  qdf_dma_addr_t buf,
					  qdf_dma_dir_t dir,
					  int nbytes)
{
	__qdf_mem_dma_cache_sync(osdev, buf, dir, nbytes);
}

/**
 * qdf_mem_unmap_nbytes_single() - un_map memory for DMA
 * @osdev: pomter OS device context
 * @phy_addr: physical address of memory to be dma unmapped
 * @dir: DMA unmap direction
 * @nbytes: number of bytes to be unmapped.
 *
 * Return: none
 */
static inline void qdf_mem_unmap_nbytes_single(qdf_device_t osdev,
					       qdf_dma_addr_t phy_addr,
					       qdf_dma_dir_t dir,
					       int nbytes)
{
#if defined(HIF_PCI) || defined(HIF_IPCI)
	__qdf_mem_unmap_nbytes_single(osdev, phy_addr, dir, nbytes);
#endif
}

/**
 * qdf_mempool_init - Create and initialize memory pool
 * @osdev: platform device object
 * @pool_addr: address of the pool created
 * @elem_cnt: no. of elements in pool
 * @elem_size: size of each pool element in bytes
 * @flags: flags
 * Return: Handle to memory pool or NULL if allocation failed
 */
static inline int qdf_mempool_init(qdf_device_t osdev,
				   qdf_mempool_t *pool_addr, int elem_cnt,
				   size_t elem_size, uint32_t flags)
{
	return __qdf_mempool_init(osdev, pool_addr, elem_cnt, elem_size,
				  flags);
}

/**
 * qdf_mempool_destroy - Destroy memory pool
 * @osdev: platform device object
 * @Handle: to memory pool
 * Return: none
 */
static inline void qdf_mempool_destroy(qdf_device_t osdev, qdf_mempool_t pool)
{
	__qdf_mempool_destroy(osdev, pool);
}

/**
 * qdf_mempool_alloc - Allocate an element memory pool
 * @osdev: platform device object
 * @Handle: to memory pool
 * Return: Pointer to the allocated element or NULL if the pool is empty
 */
static inline void *qdf_mempool_alloc(qdf_device_t osdev, qdf_mempool_t pool)
{
	return (void *)__qdf_mempool_alloc(osdev, pool);
}

/**
 * qdf_mempool_free - Free a memory pool element
 * @osdev: Platform device object
 * @pool: Handle to memory pool
 * @buf: Element to be freed
 * Return: none
 */
static inline void qdf_mempool_free(qdf_device_t osdev, qdf_mempool_t pool,
				    void *buf)
{
	__qdf_mempool_free(osdev, pool, buf);
}

void qdf_mem_dma_sync_single_for_device(qdf_device_t osdev,
					qdf_dma_addr_t bus_addr,
					qdf_size_t size,
					__dma_data_direction direction);

void qdf_mem_dma_sync_single_for_cpu(qdf_device_t osdev,
					qdf_dma_addr_t bus_addr,
					qdf_size_t size,
					__dma_data_direction direction);

int qdf_mem_multi_page_link(qdf_device_t osdev,
		struct qdf_mem_multi_page_t *pages,
		uint32_t elem_size, uint32_t elem_count, uint8_t cacheable);

/**
 * qdf_mem_kmalloc_inc() - increment kmalloc allocated bytes count
 * @size: number of bytes to increment by
 *
 * Return: None
 */
void qdf_mem_kmalloc_inc(qdf_size_t size);

/**
 * qdf_mem_kmalloc_dec() - decrement kmalloc allocated bytes count
 * @size: number of bytes to decrement by
 *
 * Return: None
 */
void qdf_mem_kmalloc_dec(qdf_size_t size);

#ifdef CONFIG_WLAN_SYSFS_MEM_STATS
/**
 * qdf_mem_skb_inc() - increment total skb allocation size
 * @size: size to be added
 *
 * Return: none
 */
void qdf_mem_skb_inc(qdf_size_t size);

/**
 * qdf_mem_skb_dec() - decrement total skb allocation size
 * @size: size to be decremented
 *
 * Return: none
 */
void qdf_mem_skb_dec(qdf_size_t size);

/**
 * qdf_mem_skb_total_inc() - increment total skb allocation size
 * in host driver in both debug and perf builds
 * @size: size to be added
 *
 * Return: none
 */
void qdf_mem_skb_total_inc(qdf_size_t size);

/**
 * qdf_mem_skb_total_dec() - decrement total skb allocation size
 * in the host driver in debug and perf flavors
 * @size: size to be decremented
 *
 * Return: none
 */
void qdf_mem_skb_total_dec(qdf_size_t size);

/**
 * qdf_mem_dp_tx_skb_inc() - Increment Tx skb allocation size
 * @size: size to be added
 *
 * Return: none
 */
void qdf_mem_dp_tx_skb_inc(qdf_size_t size);

/**
 * qdf_mem_dp_tx_skb_dec() - Decrement Tx skb allocation size
 * @size: size to be decreased
 *
 * Return: none
 */
void qdf_mem_dp_tx_skb_dec(qdf_size_t size);

/**
 * qdf_mem_dp_rx_skb_inc() - Increment Rx skb allocation size
 * @size: size to be added
 *
 * Return: none
 */
void qdf_mem_dp_rx_skb_inc(qdf_size_t size);

/**
 * qdf_mem_dp_rx_skb_dec() - Decrement Rx skb allocation size
 * @size: size to be decreased
 *
 * Return: none
 */
void qdf_mem_dp_rx_skb_dec(qdf_size_t size);

/**
 * qdf_mem_dp_tx_skb_cnt_inc() - Increment Tx buffer count
 *
 * Return: none
 */
void qdf_mem_dp_tx_skb_cnt_inc(void);

/**
 * qdf_mem_dp_tx_skb_cnt_dec() - Decrement Tx buffer count
 *
 * Return: none
 */
void qdf_mem_dp_tx_skb_cnt_dec(void);

/**
 * qdf_mem_dp_rx_skb_cnt_inc() - Increment Rx buffer count
 *
 * Return: none
 */
void qdf_mem_dp_rx_skb_cnt_inc(void);

/**
 * qdf_mem_dp_rx_skb_cnt_dec() - Decrement Rx buffer count
 *
 * Return: none
 */
void qdf_mem_dp_rx_skb_cnt_dec(void);
#else

static inline void qdf_mem_skb_inc(qdf_size_t size)
{
}

static inline void qdf_mem_skb_dec(qdf_size_t size)
{
}

static inline void qdf_mem_skb_total_inc(qdf_size_t size)
{
}

static inline void qdf_mem_skb_total_dec(qdf_size_t size)
{
}

static inline void qdf_mem_dp_tx_skb_inc(qdf_size_t size)
{
}

static inline void qdf_mem_dp_tx_skb_dec(qdf_size_t size)
{
}

static inline void qdf_mem_dp_rx_skb_inc(qdf_size_t size)
{
}

static inline void qdf_mem_dp_rx_skb_dec(qdf_size_t size)
{
}

static inline void qdf_mem_dp_tx_skb_cnt_inc(void)
{
}

static inline void qdf_mem_dp_tx_skb_cnt_dec(void)
{
}

static inline void qdf_mem_dp_rx_skb_cnt_inc(void)
{
}

static inline void qdf_mem_dp_rx_skb_cnt_dec(void)
{
}
#endif /* CONFIG_WLAN_SYSFS_MEM_STATS */

/**
 * qdf_mem_map_table_alloc() - Allocate shared memory info structure
 * @num: number of required storage
 *
 * Allocate mapping table for DMA memory allocation. This is needed for
 * IPA-WLAN buffer sharing when SMMU Stage1 Translation is enabled.
 *
 * Return: shared memory info storage table pointer
 */
static inline qdf_mem_info_t *qdf_mem_map_table_alloc(uint32_t num)
{
	qdf_mem_info_t *mem_info_arr;

	mem_info_arr = qdf_mem_malloc(num * sizeof(mem_info_arr[0]));
	return mem_info_arr;
}

#ifdef ENHANCED_OS_ABSTRACTION
/**
 * qdf_update_mem_map_table() - Update DMA memory map info
 * @osdev: Parent device instance
 * @mem_info: Pointer to shared memory information
 * @dma_addr: dma address
 * @mem_size: memory size allocated
 *
 * Store DMA shared memory information
 *
 * Return: none
 */
void qdf_update_mem_map_table(qdf_device_t osdev,
			      qdf_mem_info_t *mem_info,
			      qdf_dma_addr_t dma_addr,
			      uint32_t mem_size);

/**
 * qdf_mem_paddr_from_dmaaddr() - get actual physical address from dma address
 * @osdev: Parent device instance
 * @dma_addr: DMA/IOVA address
 *
 * Get actual physical address from dma_addr based on SMMU enablement status.
 * IF SMMU Stage 1 tranlation is enabled, DMA APIs return IO virtual address
 * (IOVA) otherwise returns physical address. So get SMMU physical address
 * mapping from IOVA.
 *
 * Return: dmaable physical address
 */
qdf_dma_addr_t qdf_mem_paddr_from_dmaaddr(qdf_device_t osdev,
					  qdf_dma_addr_t dma_addr);
#else
static inline
void qdf_update_mem_map_table(qdf_device_t osdev,
			      qdf_mem_info_t *mem_info,
			      qdf_dma_addr_t dma_addr,
			      uint32_t mem_size)
{
	if (!mem_info) {
		qdf_nofl_err("%s: NULL mem_info", __func__);
		return;
	}

	__qdf_update_mem_map_table(osdev, mem_info, dma_addr, mem_size);
}

static inline
qdf_dma_addr_t qdf_mem_paddr_from_dmaaddr(qdf_device_t osdev,
					  qdf_dma_addr_t dma_addr)
{
	return __qdf_mem_paddr_from_dmaaddr(osdev, dma_addr);
}
#endif

/**
 * qdf_mem_smmu_s1_enabled() - Return SMMU stage 1 translation enable status
 * @osdev parent device instance
 *
 * Return: true if smmu s1 enabled, false if smmu s1 is bypassed
 */
static inline bool qdf_mem_smmu_s1_enabled(qdf_device_t osdev)
{
	return __qdf_mem_smmu_s1_enabled(osdev);
}

/**
 * qdf_mem_dma_get_sgtable() - Returns DMA memory scatter gather table
 * @dev: device instace
 * @sgt: scatter gather table pointer
 * @cpu_addr: HLOS virtual address
 * @dma_addr: dma address
 * @size: allocated memory size
 *
 * Return: physical address
 */
static inline int
qdf_mem_dma_get_sgtable(struct device *dev, void *sgt, void *cpu_addr,
			qdf_dma_addr_t dma_addr, size_t size)
{
	return __qdf_os_mem_dma_get_sgtable(dev, sgt, cpu_addr, dma_addr, size);
}

/**
 * qdf_mem_free_sgtable() - Free a previously allocated sg table
 * @sgt: the mapped sg table header
 *
 * Return: None
 */
static inline void
qdf_mem_free_sgtable(struct sg_table *sgt)
{
	__qdf_os_mem_free_sgtable(sgt);
}

/**
 * qdf_dma_get_sgtable_dma_addr() - Assigns DMA address to scatterlist elements
 * @sgt: scatter gather table pointer
 *
 * Return: None
 */
static inline void
qdf_dma_get_sgtable_dma_addr(struct sg_table *sgt)
{
	__qdf_dma_get_sgtable_dma_addr(sgt);
}

/**
 * qdf_mem_get_dma_addr() - Return dma address based on SMMU translation status.
 * @osdev: Parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Get dma address based on SMMU enablement status. If SMMU Stage 1
 * tranlation is enabled, DMA APIs return IO virtual address otherwise
 * returns physical address.
 *
 * Return: dma address
 */
static inline qdf_dma_addr_t qdf_mem_get_dma_addr(qdf_device_t osdev,
						  qdf_mem_info_t *mem_info)
{
	return __qdf_mem_get_dma_addr(osdev, mem_info);
}

/**
 * qdf_mem_get_dma_addr_ptr() - Return DMA address pointer from mem info struct
 * @osdev: Parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Based on smmu stage 1 translation enablement, return corresponding dma
 * address storage pointer.
 *
 * Return: dma address storage pointer
 */
static inline qdf_dma_addr_t *qdf_mem_get_dma_addr_ptr(qdf_device_t osdev,
						       qdf_mem_info_t *mem_info)
{
	return __qdf_mem_get_dma_addr_ptr(osdev, mem_info);
}


/**
 * qdf_mem_get_dma_size() - Return DMA memory size
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Return: DMA memory size
 */
static inline uint32_t
qdf_mem_get_dma_size(qdf_device_t osdev,
		       qdf_mem_info_t *mem_info)
{
	return __qdf_mem_get_dma_size(osdev, mem_info);
}

/**
 * qdf_mem_set_dma_size() - Set DMA memory size
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 * @mem_size: memory size allocated
 *
 * Return: none
 */
static inline void
qdf_mem_set_dma_size(qdf_device_t osdev,
		       qdf_mem_info_t *mem_info,
		       uint32_t mem_size)
{
	__qdf_mem_set_dma_size(osdev, mem_info, mem_size);
}

/**
 * qdf_mem_get_dma_size() - Return DMA physical address
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Return: DMA physical address
 */
static inline qdf_dma_addr_t
qdf_mem_get_dma_pa(qdf_device_t osdev,
		     qdf_mem_info_t *mem_info)
{
	return __qdf_mem_get_dma_pa(osdev, mem_info);
}

/**
 * qdf_mem_set_dma_size() - Set DMA physical address
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 * @dma_pa: DMA phsical address
 *
 * Return: none
 */
static inline void
qdf_mem_set_dma_pa(qdf_device_t osdev,
		     qdf_mem_info_t *mem_info,
		     qdf_dma_addr_t dma_pa)
{
	__qdf_mem_set_dma_pa(osdev, mem_info, dma_pa);
}

/**
 * qdf_mem_shared_mem_alloc() - Allocate DMA memory for shared resource
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 * @size: size to be allocated
 *
 * Allocate DMA memory which will be shared with external kernel module. This
 * information is needed for SMMU mapping.
 *
 * Return: 0 success
 */
qdf_shared_mem_t *qdf_mem_shared_mem_alloc(qdf_device_t osdev, uint32_t size);

/**
 * qdf_mem_shared_mem_free() - Free shared memory
 * @osdev: parent device instance
 * @shared_mem: shared memory information storage
 *
 * Free DMA shared memory resource
 *
 * Return: None
 */
static inline void qdf_mem_shared_mem_free(qdf_device_t osdev,
					   qdf_shared_mem_t *shared_mem)
{
	if (!shared_mem) {
		qdf_nofl_err("%s: NULL shared mem struct passed",
			     __func__);
		return;
	}

	if (shared_mem->vaddr) {
		qdf_mem_free_consistent(osdev, osdev->dev,
					qdf_mem_get_dma_size(osdev,
						&shared_mem->mem_info),
					shared_mem->vaddr,
					qdf_mem_get_dma_addr(osdev,
						&shared_mem->mem_info),
					qdf_get_dma_mem_context(shared_mem,
								memctx));
	}
	qdf_mem_free_sgtable(&shared_mem->sgtable);
	qdf_mem_free(shared_mem);
}

/**
 * qdf_dma_mem_stats_read() - Return the DMA memory allocated in
 * host driver
 *
 * Return: Total DMA memory allocated
 */
int32_t qdf_dma_mem_stats_read(void);

/**
 * qdf_heap_mem_stats_read() - Return the heap memory allocated
 * in host driver
 *
 * Return: Total heap memory allocated
 */
int32_t qdf_heap_mem_stats_read(void);

/**
 * qdf_skb_mem_stats_read() - Return the SKB memory allocated in
 * host driver
 *
 * Return: Total SKB memory allocated
 */
int32_t qdf_skb_mem_stats_read(void);

/**
 * qdf_skb_total_mem_stats_read() - Return the SKB memory allocated
 * in the host driver tracked in both debug and perf builds
 *
 * Return: Total SKB memory allocated
 */
int32_t qdf_skb_total_mem_stats_read(void);

/**
 * qdf_skb_max_mem_stats_read() - Return the max SKB memory
 * allocated in host driver. This is the high watermark for the
 * total SKB allocated in the host driver
 *
 * Return: None
 */
int32_t qdf_skb_max_mem_stats_read(void);

/**
 * qdf_mem_tx_desc_cnt_read() - Return the outstanding Tx descs
 * which are waiting on Tx completions
 *
 * Return: Outstanding Tx desc count
 */
int32_t qdf_mem_tx_desc_cnt_read(void);

/**
 * qdf_mem_tx_desc_max_read() - Return the max outstanding Tx
 * descs which are waiting on Tx completions. This is the high
 * watermark for the pending desc count
 *
 * Return: Max outstanding Tx desc count
 */
int32_t qdf_mem_tx_desc_max_read(void);

/**
 * qdf_mem_stats_init() - Initialize the qdf memstats fields on
 * creating the sysfs node
 *
 * Return: None
 */
void qdf_mem_stats_init(void);

/**
 * qdf_dp_tx_skb_mem_stats_read() - Return the SKB memory
 * allocated for Tx data path
 *
 * Return: Tx SKB memory allocated
 */
int32_t qdf_dp_tx_skb_mem_stats_read(void);

/**
 * qdf_dp_rx_skb_mem_stats_read() - Return the SKB memory
 * allocated for Rx data path
 *
 * Return: Rx SKB memory allocated
 */
int32_t qdf_dp_rx_skb_mem_stats_read(void);

/**
 * qdf_dp_tx_skb_max_mem_stats_read() - Return the high
 * watermark for the SKB memory allocated for Tx data path
 *
 * Return: Max Tx SKB memory allocated
 */
int32_t qdf_dp_tx_skb_max_mem_stats_read(void);

/**
 * qdf_dp_rx_skb_max_mem_stats_read() - Return the high
 * watermark for the SKB memory allocated for Rx data path
 *
 * Return: Max Rx SKB memory allocated
 */
int32_t qdf_dp_rx_skb_max_mem_stats_read(void);

/**
 * qdf_mem_dp_tx_skb_cnt_read() - Return number of buffers
 * allocated in the Tx data path by the host driver or
 * buffers coming from the n/w stack
 *
 * Return: Number of DP Tx buffers allocated
 */
int32_t qdf_mem_dp_tx_skb_cnt_read(void);

/**
 * qdf_mem_dp_tx_skb_max_cnt_read() - Return max number of
 * buffers allocated in the Tx data path
 *
 * Return: Max number of DP Tx buffers allocated
 */
int32_t qdf_mem_dp_tx_skb_max_cnt_read(void);

/**
 * qdf_mem_dp_rx_skb_cnt_read() - Return number of buffers
 * allocated in the Rx data path
 *
 * Return: Number of DP Rx buffers allocated
 */
int32_t qdf_mem_dp_rx_skb_cnt_read(void);

/**
 * qdf_mem_dp_rx_skb_max_cnt_read() - Return max number of
 * buffers allocated in the Rx data path
 *
 * Return: Max number of DP Rx buffers allocated
 */
int32_t qdf_mem_dp_rx_skb_max_cnt_read(void);

/**
 * qdf_mem_tx_desc_cnt_update() - Update the pending tx desc
 * count and the high watermark for pending tx desc count
 *
 * @pending_tx_descs: outstanding Tx desc count
 * @tx_descs_max: high watermark for outstanding Tx desc count
 *
 * Return: None
 */
void qdf_mem_tx_desc_cnt_update(qdf_atomic_t pending_tx_descs,
				int32_t tx_descs_max);

/**
 * qdf_mem_vfree() - Free the virtual memory pointed to by ptr
 * @ptr: Pointer to the starting address of the memory to
 * be freed.
 *
 * Return: None
 */
#define qdf_mem_vfree(ptr)   __qdf_mem_vfree(ptr)

/**
 * qdf_mem_valloc() - Allocate virtual memory for the given
 * size
 * @size: Number of bytes of memory to be allocated
 *
 * Return: Pointer to the starting address of the allocated virtual memory
 */
#define qdf_mem_valloc(size) __qdf_mem_valloc(size, __func__, __LINE__)

#if IS_ENABLED(CONFIG_ARM_SMMU) && defined(ENABLE_SMMU_S1_TRANSLATION)
/*
 * typedef qdf_iommu_domain_t: Platform indepedent iommu domain
 * abstraction
 */
typedef __qdf_iommu_domain_t qdf_iommu_domain_t;

/**
 * qdf_iommu_domain_get_attr() - API to get iommu domain attributes
 * @domain: iommu domain
 * @attr: iommu attribute
 * @data: data pointer
 *
 * Return: 0 on success, else errno
 */
int
qdf_iommu_domain_get_attr(qdf_iommu_domain_t *domain,
			  enum qdf_iommu_attr attr, void *data);
#endif
#endif /* __QDF_MEMORY_H */
