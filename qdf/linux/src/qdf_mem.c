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
 * DOC: qdf_mem
 * This file provides OS dependent memory management APIs
 */

#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "qdf_lock.h"
#include "qdf_mc_timer.h"
#include "qdf_module.h"
#include <qdf_trace.h>

#ifdef CONFIG_MCL
#include <host_diag_core_event.h>
#else
#define host_log_low_resource_failure(code) do {} while (0)
#endif

#if defined(CONFIG_CNSS)
#include <net/cnss.h>
#endif

#ifdef CONFIG_WCNSS_MEM_PRE_ALLOC
#include <net/cnss_prealloc.h>
#endif

#ifdef MEMORY_DEBUG
#include <qdf_list.h>
qdf_list_t qdf_mem_list;
qdf_spinlock_t qdf_mem_list_lock;

static uint8_t WLAN_MEM_HEADER[] = { 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
					0x67, 0x68 };
static uint8_t WLAN_MEM_TAIL[] = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
					0x86, 0x87 };

/**
 * struct s_qdf_mem_struct - memory object to dubug
 * @node: node to the list
 * @filename: name of file
 * @line_num: line number
 * @size: size of the file
 * @header: array that contains header
 */
struct s_qdf_mem_struct {
	qdf_list_node_t node;
	char *file_name;
	unsigned int line_num;
	unsigned int size;
	uint8_t header[8];
};
#endif

/* Preprocessor Definitions and Constants */
#define QDF_GET_MEMORY_TIME_THRESHOLD 300

int qdf_dbg_mask;
qdf_declare_param(qdf_dbg_mask, int);
EXPORT_SYMBOL(qdf_dbg_mask);

u_int8_t prealloc_disabled = 1;
qdf_declare_param(prealloc_disabled, byte);
EXPORT_SYMBOL(prealloc_disabled);

/**
 * __qdf_mempool_init() - Create and initialize memory pool
 *
 * @osdev: platform device object
 * @pool_addr: address of the pool created
 * @elem_cnt: no. of elements in pool
 * @elem_size: size of each pool element in bytes
 * @flags: flags
 *
 * return: Handle to memory pool or NULL if allocation failed
 */
int __qdf_mempool_init(qdf_device_t osdev, __qdf_mempool_t *pool_addr,
		       int elem_cnt, size_t elem_size, u_int32_t flags)
{
	__qdf_mempool_ctxt_t *new_pool = NULL;
	u_int32_t align = L1_CACHE_BYTES;
	unsigned long aligned_pool_mem;
	int pool_id;
	int i;

	if (prealloc_disabled) {
		/* TBD: We can maintain a list of pools in qdf_device_t
		 * to help debugging
		 * when pre-allocation is not enabled
		 */
		new_pool = (__qdf_mempool_ctxt_t *)
			kmalloc(sizeof(__qdf_mempool_ctxt_t), GFP_KERNEL);
		if (new_pool == NULL)
			return QDF_STATUS_E_NOMEM;

		memset(new_pool, 0, sizeof(*new_pool));
		/* TBD: define flags for zeroing buffers etc */
		new_pool->flags = flags;
		new_pool->elem_size = elem_size;
		new_pool->max_elem = elem_cnt;
		*pool_addr = new_pool;
		return 0;
	}

	for (pool_id = 0; pool_id < MAX_MEM_POOLS; pool_id++) {
		if (osdev->mem_pool[pool_id] == NULL)
			break;
	}

	if (pool_id == MAX_MEM_POOLS)
		return -ENOMEM;

	new_pool = osdev->mem_pool[pool_id] = (__qdf_mempool_ctxt_t *)
		kmalloc(sizeof(__qdf_mempool_ctxt_t), GFP_KERNEL);
	if (new_pool == NULL)
		return -ENOMEM;

	memset(new_pool, 0, sizeof(*new_pool));
	/* TBD: define flags for zeroing buffers etc */
	new_pool->flags = flags;
	new_pool->pool_id = pool_id;

	/* Round up the element size to cacheline */
	new_pool->elem_size = roundup(elem_size, L1_CACHE_BYTES);
	new_pool->mem_size = elem_cnt * new_pool->elem_size +
				((align)?(align - 1):0);

	new_pool->pool_mem = kzalloc(new_pool->mem_size, GFP_KERNEL);
	if (new_pool->pool_mem == NULL) {
			/* TBD: Check if we need get_free_pages above */
		kfree(new_pool);
		osdev->mem_pool[pool_id] = NULL;
		return -ENOMEM;
	}

	spin_lock_init(&new_pool->lock);

	/* Initialize free list */
	aligned_pool_mem = (unsigned long)(new_pool->pool_mem) +
			((align) ? (unsigned long)(new_pool->pool_mem)%align:0);
	STAILQ_INIT(&new_pool->free_list);

	for (i = 0; i < elem_cnt; i++)
		STAILQ_INSERT_TAIL(&(new_pool->free_list),
			(mempool_elem_t *)(aligned_pool_mem +
			(new_pool->elem_size * i)), mempool_entry);


	new_pool->free_cnt = elem_cnt;
	*pool_addr = new_pool;
	return 0;
}
EXPORT_SYMBOL(__qdf_mempool_init);

/**
 * __qdf_mempool_destroy() - Destroy memory pool
 * @osdev: platform device object
 * @Handle: to memory pool
 *
 * Returns: none
 */
void __qdf_mempool_destroy(qdf_device_t osdev, __qdf_mempool_t pool)
{
	int pool_id = 0;

	if (!pool)
		return;

	if (prealloc_disabled) {
		kfree(pool);
		return;
	}

	pool_id = pool->pool_id;

	/* TBD: Check if free count matches elem_cnt if debug is enabled */
	kfree(pool->pool_mem);
	kfree(pool);
	osdev->mem_pool[pool_id] = NULL;
}
EXPORT_SYMBOL(__qdf_mempool_destroy);

/**
 * __qdf_mempool_alloc() - Allocate an element memory pool
 *
 * @osdev: platform device object
 * @Handle: to memory pool
 *
 * Return: Pointer to the allocated element or NULL if the pool is empty
 */
void *__qdf_mempool_alloc(qdf_device_t osdev, __qdf_mempool_t pool)
{
	void *buf = NULL;

	if (!pool)
		return NULL;

	if (prealloc_disabled)
		return  qdf_mem_malloc(pool->elem_size);

	spin_lock_bh(&pool->lock);

	buf = STAILQ_FIRST(&pool->free_list);
	if (buf != NULL) {
		STAILQ_REMOVE_HEAD(&pool->free_list, mempool_entry);
		pool->free_cnt--;
	}

	/* TBD: Update free count if debug is enabled */
	spin_unlock_bh(&pool->lock);

	return buf;
}
EXPORT_SYMBOL(__qdf_mempool_alloc);

/**
 * __qdf_mempool_free() - Free a memory pool element
 * @osdev: Platform device object
 * @pool: Handle to memory pool
 * @buf: Element to be freed
 *
 * Returns: none
 */
void __qdf_mempool_free(qdf_device_t osdev, __qdf_mempool_t pool, void *buf)
{
	if (!pool)
		return;


	if (prealloc_disabled)
		return qdf_mem_free(buf);

	spin_lock_bh(&pool->lock);
	pool->free_cnt++;

	STAILQ_INSERT_TAIL
		(&pool->free_list, (mempool_elem_t *)buf, mempool_entry);
	spin_unlock_bh(&pool->lock);
}
EXPORT_SYMBOL(__qdf_mempool_free);

/**
 * qdf_mem_alloc_outline() - allocation QDF memory
 * @osdev: platform device object
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
void *
qdf_mem_alloc_outline(qdf_device_t osdev, size_t size)
{
	return qdf_mem_malloc(size);
}
EXPORT_SYMBOL(qdf_mem_alloc_outline);

/**
 * qdf_mem_free_outline() - QDF memory free API
 * @ptr: Pointer to the starting address of the memory to be free'd.
 *
 * This function will free the memory pointed to by 'ptr'. It also checks
 * is memory is corrupted or getting double freed and panic.
 *
 * Return: none
 */
void
qdf_mem_free_outline(void *buf)
{
	qdf_mem_free(buf);
}
EXPORT_SYMBOL(qdf_mem_free_outline);

/**
 * qdf_mem_zero_outline() - zero out memory
 * @buf: pointer to memory that will be set to zero
 * @size: number of bytes zero
 *
 * This function sets the memory location to all zeros, essentially clearing
 * the memory.
 *
 * Return: none
 */
void
qdf_mem_zero_outline(void *buf, qdf_size_t size)
{
	qdf_mem_zero(buf, size);
}
EXPORT_SYMBOL(qdf_mem_zero_outline);

/* External Function implementation */
#ifdef MEMORY_DEBUG

/**
 * qdf_mem_init() - initialize qdf memory debug functionality
 *
 * Return: none
 */
void qdf_mem_init(void)
{
	/* Initalizing the list with maximum size of 60000 */
	qdf_list_create(&qdf_mem_list, 60000);
	qdf_spinlock_create(&qdf_mem_list_lock);
	qdf_net_buf_debug_init();
	return;
}
EXPORT_SYMBOL(qdf_mem_init);

/**
 * qdf_mem_clean() - display memory leak debug info and free leaked pointers
 *
 * Return: none
 */
void qdf_mem_clean(void)
{
	uint32_t list_size;
	list_size = qdf_list_size(&qdf_mem_list);
	if (list_size) {
		qdf_list_node_t *node;
		QDF_STATUS qdf_status;

		struct s_qdf_mem_struct *mem_struct;
		char *prev_mleak_file = "";
		unsigned int prev_mleak_line_num = 0;
		unsigned int prev_mleak_sz = 0;
		unsigned int mleak_cnt = 0;

		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
			  "%s: List is not Empty. list_size %d ",
			  __func__, (int)list_size);

		do {
			qdf_spin_lock(&qdf_mem_list_lock);
			qdf_status =
				qdf_list_remove_front(&qdf_mem_list, &node);
			qdf_spin_unlock(&qdf_mem_list_lock);
			if (QDF_STATUS_SUCCESS == qdf_status) {
				mem_struct = (struct s_qdf_mem_struct *)node;
				/* Take care to log only once multiple memory
				   leaks from the same place */
				if (strcmp(prev_mleak_file,
					mem_struct->file_name)
				    || (prev_mleak_line_num !=
					mem_struct->line_num)
				    || (prev_mleak_sz != mem_struct->size)) {
					if (mleak_cnt != 0) {
						QDF_TRACE(QDF_MODULE_ID_QDF,
							  QDF_TRACE_LEVEL_FATAL,
							  "%d Time Memory Leak@ File %s, @Line %d, size %d",
							  mleak_cnt,
							  prev_mleak_file,
							  prev_mleak_line_num,
							  prev_mleak_sz);
					}
					prev_mleak_file = mem_struct->file_name;
					prev_mleak_line_num =
						 mem_struct->line_num;
					prev_mleak_sz = mem_struct->size;
					mleak_cnt = 0;
				}
				mleak_cnt++;
				kfree((void *)mem_struct);
			}
		} while (qdf_status == QDF_STATUS_SUCCESS);

		/* Print last memory leak from the module */
		if (mleak_cnt) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
				  "%d Time memory Leak@ File %s, @Line %d, size %d",
				  mleak_cnt, prev_mleak_file,
				  prev_mleak_line_num, prev_mleak_sz);
		}
#ifdef CONFIG_HALT_KMEMLEAK
		BUG_ON(0);
#endif
	}
}
EXPORT_SYMBOL(qdf_mem_clean);

/**
 * qdf_mem_exit() - exit qdf memory debug functionality
 *
 * Return: none
 */
void qdf_mem_exit(void)
{
	qdf_net_buf_debug_exit();
	qdf_mem_clean();
	qdf_list_destroy(&qdf_mem_list);
}
EXPORT_SYMBOL(qdf_mem_exit);

/**
 * qdf_mem_malloc_debug() - debug version of QDF memory allocation API
 * @size: Number of bytes of memory to allocate.
 * @file_name: File name from which memory allocation is called
 * @line_num: Line number from which memory allocation is called
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory and ad it in qdf tracking list to check against memory leaks and
 * corruptions
 *
 * Return:
 * Upon successful allocate, returns a non-NULL pointer to the allocated
 * memory.  If this function is unable to allocate the amount of memory
 * specified (for any reason) it returns %NULL.
 */
void *qdf_mem_malloc_debug(size_t size,
			char *file_name, uint32_t line_num)
{
	struct s_qdf_mem_struct *mem_struct;
	void *mem_ptr = NULL;
	uint32_t new_size;
	int flags = GFP_KERNEL;
	unsigned long  time_before_kzalloc;

	if (size > (1024 * 1024) || size == 0) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: called with invalid arg; passed in %zu !!!",
			  __func__, size);
		host_log_low_resource_failure(WIFI_EVENT_MEMORY_FAILURE);
		return NULL;
	}

#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
	if (size > WCNSS_PRE_ALLOC_GET_THRESHOLD) {
		void *pmem;
		pmem = wcnss_prealloc_get(size);
		if (NULL != pmem) {
			memset(pmem, 0, size);
			return pmem;
		}
	}
#endif

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	new_size = size + sizeof(struct s_qdf_mem_struct) + 8;/*TBD: what is 8*/
	time_before_kzalloc = qdf_mc_timer_get_system_time();
	mem_struct = (struct s_qdf_mem_struct *)kzalloc(new_size, flags);
	/**
	 * If time taken by kmalloc is greater than
	 * QDF_GET_MEMORY_TIME_THRESHOLD msec
	 */
	if (qdf_mc_timer_get_system_time() - time_before_kzalloc >=
					  QDF_GET_MEMORY_TIME_THRESHOLD)
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "%s: kzalloc took %lu msec for size %zu called from %p_s at line %d",
			 __func__,
			 qdf_mc_timer_get_system_time() - time_before_kzalloc,
			 size, (void *)_RET_IP_, line_num);

	if (mem_struct != NULL) {
		QDF_STATUS qdf_status;

		mem_struct->file_name = file_name;
		mem_struct->line_num = line_num;
		mem_struct->size = size;

		qdf_mem_copy(&mem_struct->header[0],
			     &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER));

		qdf_mem_copy((uint8_t *) (mem_struct + 1) + size,
			     &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL));

		qdf_spin_lock_irqsave(&qdf_mem_list_lock);
		qdf_status = qdf_list_insert_front(&qdf_mem_list,
						   &mem_struct->node);
		qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "%s: Unable to insert node into List qdf_status %d",
				  __func__, qdf_status);
		}

		mem_ptr = (void *)(mem_struct + 1);
	}

	if (!mem_ptr)
		host_log_low_resource_failure(WIFI_EVENT_MEMORY_FAILURE);

	return mem_ptr;
}
EXPORT_SYMBOL(qdf_mem_malloc_debug);

/**
 * qdf_mem_validate_node_for_free() - validate that the node is in a list
 * @qdf_node: node to check for being in a list
 *
 * qdf_node should be a non null value.
 *
 * Return: true if the node validly linked in an anchored doubly linked list
 */
static bool qdf_mem_validate_node_for_free(qdf_list_node_t *qdf_node)
{
	struct list_head *node = qdf_node;

	/*
	 * if the node is an empty list, it is not tied to an anchor node
	 * and must have been removed with list_del_init
	 */
	if (list_empty(node))
		return false;

	if (node->prev == NULL)
		return false;

	if (node->next == NULL)
		return false;

	if (node->prev->next != node)
		return false;

	if (node->next->prev != node)
		return false;

	return true;
}



/**
 * qdf_mem_free() - QDF memory free API
 * @ptr: Pointer to the starting address of the memory to be free'd.
 *
 * This function will free the memory pointed to by 'ptr'. It also checks
 * is memory is corrupted or getting double freed and panic.
 *
 * Return: none
 */
void qdf_mem_free(void *ptr)
{
	struct s_qdf_mem_struct *mem_struct;

	/* freeing a null pointer is valid */
	if (qdf_unlikely(ptr == NULL))
		return;

	mem_struct = ((struct s_qdf_mem_struct *)ptr) - 1;

	if (qdf_unlikely(mem_struct == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: null mem_struct", __func__);
		QDF_BUG(0);
	}

#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
	if (wcnss_prealloc_put(ptr))
		return;
#endif

	qdf_spin_lock_irqsave(&qdf_mem_list_lock);

	/*
	 * invalid memory access when checking the header/tailer
	 * would be a use after free and would indicate a double free
	 * or invalid pointer passed.
	 */
	if (qdf_mem_cmp(mem_struct->header, &WLAN_MEM_HEADER[0],
			sizeof(WLAN_MEM_HEADER)))
		goto error;

	/*
	 * invalid memory access while checking validate node
	 * would indicate corruption in the nodes pointed to
	 */
	if (!qdf_mem_validate_node_for_free(&mem_struct->node))
		goto error;

	/*
	 * invalid memory access here is unlikely and would imply
	 * that the size value was corrupted/incorrect.
	 * It is unlikely that the above checks would pass in a
	 * double free case.
	 */
	if (qdf_mem_cmp((uint8_t *) ptr + mem_struct->size,
			&WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL)))
		goto error;

	/*
	 * make the node an empty list before doing the spin unlock
	 * The empty list check will guarantee that we avoid a race condition.
	 */
	list_del_init(&mem_struct->node);
	qdf_mem_list.count--;
	qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
	kfree(mem_struct);
	return;

error:
	if (!qdf_list_has_node(&qdf_mem_list, &mem_struct->node)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: Unallocated memory (double free?)",
			  __func__);
		qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
		QDF_BUG(0);
	}

	if (qdf_mem_cmp(mem_struct->header, &WLAN_MEM_HEADER[0],
				sizeof(WLAN_MEM_HEADER))) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "Memory Header is corrupted.");
		qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
		QDF_BUG(0);
	}

	if (!qdf_mem_validate_node_for_free(&mem_struct->node)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "Memory_struct is corrupted.");
		qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
		QDF_BUG(0);
	}

	if (qdf_mem_cmp((uint8_t *) ptr + mem_struct->size,
			&WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL))) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "Memory Trailer is corrupted. mem_info: Filename %s, line_num %d",
			  mem_struct->file_name, (int)mem_struct->line_num);
		qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
		QDF_BUG(0);
	}

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
		  "%s unexpected error", __func__);
	qdf_spin_unlock_irqrestore(&qdf_mem_list_lock);
	QDF_BUG(0);
}
EXPORT_SYMBOL(qdf_mem_free);
#else

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
void *qdf_mem_malloc(size_t size)
{
	int flags = GFP_KERNEL;

	if (in_interrupt() || irqs_disabled())
		flags = GFP_ATOMIC;

	return kzalloc(size, flags);
}
EXPORT_SYMBOL(qdf_mem_malloc);

/**
 * qdf_mem_free() - free QDF memory
 * @ptr: Pointer to the starting address of the memory to be free'd.
 *
 * This function will free the memory pointed to by 'ptr'.
 *
 * Return: None
 */
void qdf_mem_free(void *ptr)
{
	if (ptr == NULL)
		return;
#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
	if (wcnss_prealloc_put(ptr))
		return;
#endif
	kfree(ptr);
}
EXPORT_SYMBOL(qdf_mem_free);
#endif

/**
 * qdf_mem_multi_pages_alloc() - allocate large size of kernel memory
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @element_size: Each element size
 * @element_num: Total number of elements should be allocated
 * @memctxt: Memory context
 * @cacheable: Coherent memory or cacheable memory
 *
 * This function will allocate large size of memory over multiple pages.
 * Large size of contiguous memory allocation will fail frequently, then
 * instead of allocate large memory by one shot, allocate through multiple, non
 * contiguous memory and combine pages when actual usage
 *
 * Return: None
 */
void qdf_mem_multi_pages_alloc(qdf_device_t osdev,
			       struct qdf_mem_multi_page_t *pages,
			       size_t element_size, uint16_t element_num,
			       qdf_dma_context_t memctxt, bool cacheable)
{
	uint16_t page_idx;
	struct qdf_mem_dma_page_t *dma_pages;
	void **cacheable_pages = NULL;
	uint16_t i;

	pages->num_element_per_page = PAGE_SIZE / element_size;
	if (!pages->num_element_per_page) {
		qdf_print("Invalid page %d or element size %d",
			  (int)PAGE_SIZE, (int)element_size);
		goto out_fail;
	}

	pages->num_pages = element_num / pages->num_element_per_page;
	if (element_num % pages->num_element_per_page)
		pages->num_pages++;

	if (cacheable) {
		/* Pages information storage */
		pages->cacheable_pages = qdf_mem_malloc(
			pages->num_pages * sizeof(pages->cacheable_pages));
		if (!pages->cacheable_pages) {
			qdf_print("Cacheable page storage alloc fail");
			goto out_fail;
		}

		cacheable_pages = pages->cacheable_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			cacheable_pages[page_idx] = qdf_mem_malloc(PAGE_SIZE);
			if (!cacheable_pages[page_idx]) {
				qdf_print("cacheable page alloc fail, pi %d",
					  page_idx);
				goto page_alloc_fail;
			}
		}
		pages->dma_pages = NULL;
	} else {
		pages->dma_pages = qdf_mem_malloc(
			pages->num_pages * sizeof(struct qdf_mem_dma_page_t));
		if (!pages->dma_pages) {
			qdf_print("dmaable page storage alloc fail");
			goto out_fail;
		}

		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			dma_pages->page_v_addr_start =
				qdf_mem_alloc_consistent(osdev, osdev->dev,
					 PAGE_SIZE,
					&dma_pages->page_p_addr);
			if (!dma_pages->page_v_addr_start) {
				qdf_print("dmaable page alloc fail pi %d",
					page_idx);
				goto page_alloc_fail;
			}
			dma_pages->page_v_addr_end =
				dma_pages->page_v_addr_start + PAGE_SIZE;
			dma_pages++;
		}
		pages->cacheable_pages = NULL;
	}
	return;

page_alloc_fail:
	if (cacheable) {
		for (i = 0; i < page_idx; i++)
			qdf_mem_free(pages->cacheable_pages[i]);
		qdf_mem_free(pages->cacheable_pages);
	} else {
		dma_pages = pages->dma_pages;
		for (i = 0; i < page_idx; i++) {
			qdf_mem_free_consistent(osdev, osdev->dev, PAGE_SIZE,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt);
			dma_pages++;
		}
		qdf_mem_free(pages->dma_pages);
	}

out_fail:
	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
	return;
}
EXPORT_SYMBOL(qdf_mem_multi_pages_alloc);

/**
 * qdf_mem_multi_pages_free() - free large size of kernel memory
 * @osdev: OS device handle pointer
 * @pages: Multi page information storage
 * @memctxt: Memory context
 * @cacheable: Coherent memory or cacheable memory
 *
 * This function will free large size of memory over multiple pages.
 *
 * Return: None
 */
void qdf_mem_multi_pages_free(qdf_device_t osdev,
			      struct qdf_mem_multi_page_t *pages,
			      qdf_dma_context_t memctxt, bool cacheable)
{
	unsigned int page_idx;
	struct qdf_mem_dma_page_t *dma_pages;

	if (cacheable) {
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++)
			qdf_mem_free(pages->cacheable_pages[page_idx]);
		qdf_mem_free(pages->cacheable_pages);
	} else {
		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			qdf_mem_free_consistent(osdev, osdev->dev, PAGE_SIZE,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt);
			dma_pages++;
		}
		qdf_mem_free(pages->dma_pages);
	}

	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
	return;
}
EXPORT_SYMBOL(qdf_mem_multi_pages_free);

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
void qdf_mem_copy(void *dst_addr, const void *src_addr, uint32_t num_bytes)
{
	if (0 == num_bytes) {
		/* special case where dst_addr or src_addr can be NULL */
		return;
	}

	if ((dst_addr == NULL) || (src_addr == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, source:%p destination:%p",
			  __func__, src_addr, dst_addr);
		QDF_ASSERT(0);
		return;
	}
	memcpy(dst_addr, src_addr, num_bytes);
}
EXPORT_SYMBOL(qdf_mem_copy);

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
void qdf_mem_zero(void *ptr, uint32_t num_bytes)
{
	if (0 == num_bytes) {
		/* special case where ptr can be NULL */
		return;
	}

	if (ptr == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter ptr", __func__);
		return;
	}
	memset(ptr, 0, num_bytes);
}
EXPORT_SYMBOL(qdf_mem_zero);

/**
 * qdf_mem_set() - set (fill) memory with a specified byte value.
 * @ptr: Pointer to memory that will be set
 * @num_bytes: Number of bytes to be set
 * @value: Byte set in memory
 *
 * Return: None
 */
void qdf_mem_set(void *ptr, uint32_t num_bytes, uint32_t value)
{
	if (ptr == NULL) {
		qdf_print("%s called with NULL parameter ptr", __func__);
		return;
	}
	memset(ptr, value, num_bytes);
}
EXPORT_SYMBOL(qdf_mem_set);

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
void qdf_mem_move(void *dst_addr, const void *src_addr, uint32_t num_bytes)
{
	if (0 == num_bytes) {
		/* special case where dst_addr or src_addr can be NULL */
		return;
	}

	if ((dst_addr == NULL) || (src_addr == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, source:%p destination:%p",
			  __func__, src_addr, dst_addr);
		QDF_ASSERT(0);
		return;
	}
	memmove(dst_addr, src_addr, num_bytes);
}
EXPORT_SYMBOL(qdf_mem_move);

#if defined(A_SIMOS_DEVHOST) || defined(HIF_SDIO) || defined(HIF_USB)
/**
 * qdf_mem_alloc_consistent() - allocates consistent qdf memory
 * @osdev: OS device handle
 * @dev: Pointer to device handle
 * @size: Size to be allocated
 * @phy_addr: Physical address
 *
 * Return: pointer of allocated memory or null if memory alloc fails
 */
void *qdf_mem_alloc_consistent(qdf_device_t osdev, void *dev, qdf_size_t size,
			       qdf_dma_addr_t *phy_addr)
{
	void *vaddr;

	vaddr = qdf_mem_malloc(size);
	*phy_addr = ((uintptr_t) vaddr);
	/* using this type conversion to suppress "cast from pointer to integer
	 * of different size" warning on some platforms
	 */
	BUILD_BUG_ON(sizeof(*phy_addr) < sizeof(vaddr));
	return vaddr;
}

#else
void *qdf_mem_alloc_consistent(qdf_device_t osdev, void *dev, qdf_size_t size,
			       qdf_dma_addr_t *phy_addr)
{
	int flags = GFP_KERNEL;
	void *alloc_mem = NULL;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	alloc_mem = dma_alloc_coherent(dev, size, phy_addr, flags);
	if (alloc_mem == NULL)
		qdf_print("%s Warning: unable to alloc consistent memory of size %zu!\n",
			__func__, size);
	return alloc_mem;
}

#endif
EXPORT_SYMBOL(qdf_mem_alloc_consistent);

#if defined(A_SIMOS_DEVHOST) ||  defined(HIF_SDIO) || defined(HIF_USB)
/**
 * qdf_mem_free_consistent() - free consistent qdf memory
 * @osdev: OS device handle
 * @size: Size to be allocated
 * @vaddr: virtual address
 * @phy_addr: Physical address
 * @mctx: Pointer to DMA context
 *
 * Return: none
 */
inline void qdf_mem_free_consistent(qdf_device_t osdev, void *dev,
				    qdf_size_t size, void *vaddr,
				    qdf_dma_addr_t phy_addr,
				    qdf_dma_context_t memctx)
{
	qdf_mem_free(vaddr);
	return;
}

#else
inline void qdf_mem_free_consistent(qdf_device_t osdev, void *dev,
				    qdf_size_t size, void *vaddr,
				    qdf_dma_addr_t phy_addr,
				    qdf_dma_context_t memctx)
{
	dma_free_coherent(dev, size, vaddr, phy_addr);
}

#endif
EXPORT_SYMBOL(qdf_mem_free_consistent);

/**
 * qdf_mem_dma_sync_single_for_device() - assign memory to device
 * @osdev: OS device handle
 * @bus_addr: dma address to give to the device
 * @size: Size of the memory block
 * @direction: direction data will be DMAed
 *
 * Assign memory to the remote device.
 * The cache lines are flushed to ram or invalidated as needed.
 *
 * Return: none
 */
void qdf_mem_dma_sync_single_for_device(qdf_device_t osdev,
					qdf_dma_addr_t bus_addr,
					qdf_size_t size,
					enum dma_data_direction direction)
{
	dma_sync_single_for_device(osdev->dev, bus_addr,  size, direction);
}
EXPORT_SYMBOL(qdf_mem_dma_sync_single_for_device);

/**
 * qdf_mem_dma_sync_single_for_cpu() - assign memory to CPU
 * @osdev: OS device handle
 * @bus_addr: dma address to give to the cpu
 * @size: Size of the memory block
 * @direction: direction data will be DMAed
 *
 * Assign memory to the CPU.
 *
 * Return: none
 */
void qdf_mem_dma_sync_single_for_cpu(qdf_device_t osdev,
				     qdf_dma_addr_t bus_addr,
				     qdf_size_t size,
				     enum dma_data_direction direction)
{
	dma_sync_single_for_cpu(osdev->dev, bus_addr,  size, direction);
}
EXPORT_SYMBOL(qdf_mem_dma_sync_single_for_cpu);
