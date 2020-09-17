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
 * DOC: qdf_nbuf_frag.c
 * QCA driver framework(QDF) network nbuf frag management APIs
 */

#include <qdf_atomic.h>
#include <qdf_list.h>
#include <qdf_debugfs.h>
#include <qdf_module.h>
#include <qdf_nbuf_frag.h>
#include <qdf_trace.h>
#include "qdf_str.h"

#ifdef QDF_NBUF_FRAG_GLOBAL_COUNT
#define FRAG_DEBUGFS_NAME    "frag_counters"
static qdf_atomic_t frag_count;
#endif

#if defined(NBUF_FRAG_MEMORY_DEBUG) || defined(QDF_NBUF_FRAG_GLOBAL_COUNT)
static bool is_initial_mem_debug_disabled;
#endif

#ifdef QDF_NBUF_FRAG_GLOBAL_COUNT

uint32_t __qdf_frag_count_get(void)
{
	return qdf_atomic_read(&frag_count);
}

qdf_export_symbol(__qdf_frag_count_get);

void __qdf_frag_count_inc(uint32_t value)
{
	if (qdf_likely(is_initial_mem_debug_disabled))
		return;

	qdf_atomic_add(value, &frag_count);
}

qdf_export_symbol(__qdf_frag_count_inc);

void __qdf_frag_count_dec(uint32_t value)
{
	if (qdf_likely(is_initial_mem_debug_disabled))
		return;

	qdf_atomic_sub(value, &frag_count);
}

qdf_export_symbol(__qdf_frag_count_dec);

void __qdf_frag_mod_init(void)
{
	is_initial_mem_debug_disabled = qdf_mem_debug_config_get();
	qdf_atomic_init(&frag_count);
	qdf_debugfs_create_atomic(FRAG_DEBUGFS_NAME, S_IRUSR, NULL,
				  &frag_count);
}

void __qdf_frag_mod_exit(void)
{
}
#endif /* QDF_NBUF_FRAG_GLOBAL_COUNT */

#ifdef NBUF_FRAG_MEMORY_DEBUG

#define QDF_FRAG_TRACK_MAX_SIZE    1024

/**
 * struct qdf_frag_track_node_t - Network frag tracking node structure
 * @hnode: list_head for next and prev pointers
 * @p_frag: Pointer to frag
 * @alloc_func_name: Function where frag is allocated
 * @alloc_func_line: Allocation function line no.
 * @refcount: No. of refereces to the frag
 * @last_func_name: Function where frag recently accessed
 * @last_func_line_num: Line number of last function
 *
 **/
struct qdf_frag_track_node_t {
	qdf_list_node_t hnode;
	qdf_frag_t p_frag;
	char alloc_func_name[QDF_MEM_FUNC_NAME_SIZE];
	uint32_t alloc_func_line;
	uint8_t refcount;
	char last_func_name[QDF_MEM_FUNC_NAME_SIZE];
	uint32_t last_func_line;
};

/**
 * struct qdf_frag_tracking_list_t - Frag node tracking list
 * @track_list: qdf_list_t for maintaining the list
 * @list_lock: Lock over the list
 *
 */
typedef struct qdf_frag_tracking_list_t {
	qdf_list_t track_list;
	qdf_spinlock_t list_lock;
} qdf_frag_tracking_list;

typedef struct qdf_frag_track_node_t QDF_FRAG_TRACK;

/**
 * Array of tracking list for maintaining
 * allocated debug frag nodes as per the calculated
 * hash value.
 */
static qdf_frag_tracking_list gp_qdf_frag_track_tbl[QDF_FRAG_TRACK_MAX_SIZE];

static struct kmem_cache *frag_tracking_cache;

/* Tracking list for maintaining the free debug frag nodes */
static qdf_frag_tracking_list qdf_frag_track_free_list;

/**
 * Parameters for statistics
 * qdf_frag_track_free_list_count: No. of free nodes
 * qdf_frag_track_used_list_count : No. of nodes used
 * qdf_frag_track_max_used : Max no. of nodes used during execution
 * qdf_frag_track_max_free : Max free nodes observed during execution
 * qdf_frag_track_max_allocated: Max no. of allocated nodes
 */
static uint32_t qdf_frag_track_free_list_count;
static uint32_t qdf_frag_track_used_list_count;
static uint32_t qdf_frag_track_max_used;
static uint32_t qdf_frag_track_max_free;
static uint32_t qdf_frag_track_max_allocated;

/**
 * qdf_frag_update_max_used() - Update qdf_frag_track_max_used tracking variable
 *
 * Tracks the max number of frags that the wlan driver was tracking at any one
 * time
 *
 * Return: none
 **/
static inline void qdf_frag_update_max_used(void)
{
	int sum;

	/* Update max_used if it is less than used list count */
	if (qdf_frag_track_max_used < qdf_frag_track_used_list_count)
		qdf_frag_track_max_used = qdf_frag_track_used_list_count;

	/* Calculate no. of allocated nodes */
	sum = qdf_frag_track_used_list_count + qdf_frag_track_free_list_count;

	/* Update max allocated if less then no. of allocated nodes */
	if (qdf_frag_track_max_allocated < sum)
		qdf_frag_track_max_allocated = sum;
}

/**
 * qdf_frag_update_max_free() - Update qdf_frag_track_max_free
 *
 * Tracks the max number tracking buffers kept in the freelist.
 *
 * Return: none
 */
static inline void qdf_frag_update_max_free(void)
{
	if (qdf_frag_track_max_free < qdf_frag_track_free_list_count)
		qdf_frag_track_max_free = qdf_frag_track_free_list_count;
}

/**
 * qdf_frag_track_alloc() - Allocate a cookie to track frags allocated by wlan
 *
 * This function pulls from freelist if possible,otherwise uses kmem_cache_alloc
 * This function also adds fexibility to adjust the allocation and freelist
 * schemes.
 *
 * Return: Pointer to an unused QDF_FRAG_TRACK structure which may not be zeroed
 */
static QDF_FRAG_TRACK *qdf_frag_track_alloc(void)
{
	int flags = GFP_KERNEL;
	QDF_FRAG_TRACK *frag_track_node = NULL;
	qdf_list_node_t *temp_list_node;

	qdf_spin_lock_irqsave(&qdf_frag_track_free_list.list_lock);
	qdf_frag_track_used_list_count++;

	if (!qdf_list_empty(&qdf_frag_track_free_list.track_list)) {
		qdf_list_remove_front(&qdf_frag_track_free_list.track_list,
				      &temp_list_node);
		frag_track_node = qdf_container_of(temp_list_node,
						   struct qdf_frag_track_node_t,
						   hnode);
		qdf_frag_track_free_list_count--;
	}

	qdf_frag_update_max_used();
	qdf_spin_unlock_irqrestore(&qdf_frag_track_free_list.list_lock);

	if (frag_track_node)
		return frag_track_node;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	frag_track_node = kmem_cache_alloc(frag_tracking_cache, flags);
	if (frag_track_node)
		qdf_init_list_head(&frag_track_node->hnode);

	return frag_track_node;
}

/* FREEQ_POOLSIZE initial and minimum desired freelist poolsize */
#define FREEQ_POOLSIZE    2048

/**
 * qdf_frag_track_free() - Free the frag tracking cookie.
 * @frag_track_node : Debug frag node address
 *
 * Matches calls to qdf_frag_track_alloc.
 * Either frees the tracking cookie to kernel or an internal
 * freelist based on the size of the freelist.
 *
 * Return: none
 */
static void qdf_frag_track_free(QDF_FRAG_TRACK *frag_track_node)
{
	if (!frag_track_node)
		return;

	/*
	 * Try to shrink the freelist if free_list_count > than FREEQ_POOLSIZE
	 * only shrink the freelist if it is bigger than twice the number of
	 * frags in use. Otherwise add the frag debug track node to the front
	 * of qdf_frag_track_free_list.
	 */

	qdf_spin_lock_irqsave(&qdf_frag_track_free_list.list_lock);

	qdf_frag_track_used_list_count--;
	if (qdf_frag_track_free_list_count > FREEQ_POOLSIZE &&
	    (qdf_frag_track_free_list_count >
	    qdf_frag_track_used_list_count << 1)) {
		kmem_cache_free(frag_tracking_cache, frag_track_node);
	} else {
		qdf_list_insert_front(&qdf_frag_track_free_list.track_list,
				      &frag_track_node->hnode);
		qdf_frag_track_free_list_count++;
	}
	qdf_frag_update_max_free();
	qdf_spin_unlock_irqrestore(&qdf_frag_track_free_list.list_lock);
}

/**
 * qdf_frag_track_prefill() - Prefill the frag tracking cookie freelist
 *
 * Return: none
 */
static void qdf_frag_track_prefill(void)
{
	int index;
	QDF_FRAG_TRACK *curr_node, *next_node;
	qdf_list_t temp_list;

	qdf_list_create(&temp_list, 0);

	/* Prepopulate the freelist */
	for (index = 0; index < FREEQ_POOLSIZE; index++) {
		curr_node = qdf_frag_track_alloc();
		if (!curr_node)
			continue;
		qdf_list_insert_front(&temp_list, &curr_node->hnode);
	}

	curr_node = NULL;
	next_node = NULL;

	qdf_list_for_each_del(&temp_list, curr_node, next_node, hnode) {
		qdf_list_remove_node(&temp_list, &curr_node->hnode);
		qdf_frag_track_free(curr_node);
	}

	/* prefilled buffers should not count as used */
	qdf_frag_track_max_used = 0;

	qdf_list_destroy(&temp_list);
}

/**
 * qdf_frag_track_memory_manager_create() - Manager for frag tracking cookies
 *
 * This initializes the memory manager for the frag tracking cookies. Because
 * these cookies are all the same size and only used in this feature, we can
 * use a kmem_cache to provide tracking as well as to speed up allocations.
 * To avoid the overhead of allocating and freeing the buffers (including SLUB
 * features) a freelist is prepopulated here.
 *
 * Return: none
 */
static void qdf_frag_track_memory_manager_create(void)
{
	qdf_spinlock_create(&qdf_frag_track_free_list.list_lock);
	qdf_list_create(&qdf_frag_track_free_list.track_list, 0);
	frag_tracking_cache = kmem_cache_create("qdf_frag_tracking_cache",
						sizeof(QDF_FRAG_TRACK),
						0, 0, NULL);

	qdf_frag_track_prefill();
}

/**
 * qdf_frag_track_memory_manager_destroy() - Manager for frag tracking cookies
 *
 * Empty the freelist and print out usage statistics when it is no longer
 * needed. Also the kmem_cache should be destroyed here so that it can warn if
 * any frag tracking cookies were leaked.
 *
 * Return: none
 */
static void qdf_frag_track_memory_manager_destroy(void)
{
	QDF_FRAG_TRACK *curr_node, *next_node;

	curr_node = next_node = NULL;

	qdf_spin_lock_irqsave(&qdf_frag_track_free_list.list_lock);

	if (qdf_frag_track_max_used > FREEQ_POOLSIZE * 4)
		qdf_info("Unexpectedly large max_used count %d",
			  qdf_frag_track_max_used);

	if (qdf_frag_track_max_used < qdf_frag_track_max_allocated)
		qdf_info("%d Unused trackers were allocated",
			  qdf_frag_track_max_allocated -
			  qdf_frag_track_max_used);

	if (qdf_frag_track_free_list_count > FREEQ_POOLSIZE &&
	    qdf_frag_track_free_list_count > 3 * qdf_frag_track_max_used / 4)
		qdf_info("Check freelist shrinking functionality");

	qdf_info("%d Residual freelist size", qdf_frag_track_free_list_count);

	qdf_info("%d Max freelist size observed", qdf_frag_track_max_free);

	qdf_info("%d Max buffers used observed", qdf_frag_track_max_used);

	qdf_info("%d Max buffers allocated observed",
		  qdf_frag_track_max_allocated);

	qdf_list_for_each_del(&qdf_frag_track_free_list.track_list,
			      curr_node, next_node, hnode) {
		qdf_list_remove_node(&qdf_frag_track_free_list.track_list,
				     &curr_node->hnode);
		kmem_cache_free(frag_tracking_cache, curr_node);
		qdf_frag_track_free_list_count--;
	}

	if (qdf_frag_track_free_list_count != 0)
		qdf_info("%d Unfreed tracking memory lost in freelist",
			 qdf_frag_track_free_list_count);

	if (qdf_frag_track_used_list_count != 0)
		qdf_info("%d Unfreed tracking memory still in use",
			 qdf_frag_track_used_list_count);

	qdf_spin_unlock_irqrestore(&qdf_frag_track_free_list.list_lock);
	kmem_cache_destroy(frag_tracking_cache);

	qdf_list_destroy(&qdf_frag_track_free_list.track_list);
	qdf_spinlock_destroy(&qdf_frag_track_free_list.list_lock);
}

/**
 * qdf_frag_debug_init() - Initialize network frag debug functionality
 *
 * QDF frag buffer debug feature tracks all frags allocated by WLAN driver
 * in a hash table and when driver is unloaded it reports about leaked frags.
 *
 * Return: none
 */
void qdf_frag_debug_init(void)
{
	uint32_t index;

	is_initial_mem_debug_disabled = qdf_mem_debug_config_get();

	if (is_initial_mem_debug_disabled)
		return;

	qdf_frag_track_memory_manager_create();

	for (index = 0; index < QDF_FRAG_TRACK_MAX_SIZE; index++) {
		qdf_list_create(&gp_qdf_frag_track_tbl[index].track_list, 0);
		qdf_spinlock_create(&gp_qdf_frag_track_tbl[index].list_lock);
	}
}

qdf_export_symbol(qdf_frag_debug_init);

/**
 * qdf_frag_buf_debug_exit() - Exit network frag debug functionality
 *
 * Exit network frag tracking debug functionality and log frag memory leaks
 *
 * Return: none
 */
void qdf_frag_debug_exit(void)
{
	uint32_t index;
	QDF_FRAG_TRACK *p_node;
	QDF_FRAG_TRACK *p_prev;

	if (is_initial_mem_debug_disabled)
		return;

	for (index = 0; index < QDF_FRAG_TRACK_MAX_SIZE; index++) {
		qdf_spin_lock_irqsave(&gp_qdf_frag_track_tbl[index].list_lock);
		qdf_list_for_each_del(&gp_qdf_frag_track_tbl[index].track_list,
				      p_prev, p_node, hnode) {
			qdf_list_remove_node(
				&gp_qdf_frag_track_tbl[index].track_list,
				&p_prev->hnode);
			qdf_info("******Frag Memory Leak******");
			qdf_info("@Frag Address: %pK", p_prev->p_frag);
			qdf_info("@Refcount: %u", p_prev->refcount);
			qdf_info("@Alloc Func Name: %s, @Alloc Func Line: %d",
				 p_prev->alloc_func_name,
				 p_prev->alloc_func_line);
			qdf_info("@Last Func Name: %s, @Last Func Line: %d",
				 p_prev->last_func_name,
				 p_prev->last_func_line);
			qdf_info("****************************");

			qdf_frag_track_free(p_prev);
		}
		qdf_list_destroy(&gp_qdf_frag_track_tbl[index].track_list);
		qdf_spin_unlock_irqrestore(
				&gp_qdf_frag_track_tbl[index].list_lock);
		qdf_spinlock_destroy(&gp_qdf_frag_track_tbl[index].list_lock);
	}

	qdf_frag_track_memory_manager_destroy();
}

qdf_export_symbol(qdf_frag_debug_exit);

/**
 * qdf_frag_debug_hash() - Hash network frag pointer
 * @p_frag: Frag address
 *
 * Return: hash value
 */
static uint32_t qdf_frag_debug_hash(qdf_frag_t p_frag)
{
	uint32_t index;

	index = (uint32_t)(((uintptr_t)p_frag) >> 4);
	index += (uint32_t)(((uintptr_t)p_frag) >> 14);
	index &= (QDF_FRAG_TRACK_MAX_SIZE - 1);

	return index;
}

/**
 * qdf_frag_debug_look_up() - Look up network frag in debug hash table
 * @p_frag: Frag address
 *
 * Return: If frag is found in hash table then return pointer to network frag
 *	else return NULL
 */
static QDF_FRAG_TRACK *qdf_frag_debug_look_up(qdf_frag_t p_frag)
{
	uint32_t index;
	QDF_FRAG_TRACK *p_node;

	index = qdf_frag_debug_hash(p_frag);

	qdf_list_for_each(&gp_qdf_frag_track_tbl[index].track_list, p_node,
			  hnode) {
		if (p_node->p_frag == p_frag)
			return p_node;
	}

	return NULL;
}

/**
 * __qdf_frag_debug_add_node()- Add frag node to debug tracker
 * @fragp: Frag Pointer
 * @idx: Index
 * @func_name: Caller function name
 * @line_num: Caller function line no.
 *
 * Return: Allocated frag tracker node address
 */
static QDF_FRAG_TRACK *__qdf_frag_debug_add_node(qdf_frag_t fragp,
						 uint32_t idx,
						 const char *func_name,
						 uint32_t line_num)
{
	QDF_FRAG_TRACK *p_node;

	p_node = qdf_frag_track_alloc();

	if (p_node) {
		p_node->p_frag = fragp;
		qdf_str_lcopy(p_node->alloc_func_name, func_name,
			      QDF_MEM_FUNC_NAME_SIZE);
		p_node->alloc_func_line = line_num;
		p_node->refcount = QDF_NBUF_FRAG_DEBUG_COUNT_ZERO;

		qdf_str_lcopy(p_node->last_func_name, func_name,
			      QDF_MEM_FUNC_NAME_SIZE);
		p_node->last_func_line = line_num;

		qdf_list_insert_front(&gp_qdf_frag_track_tbl[idx].track_list,
				      &p_node->hnode);
	}
	return p_node;
}

/**
 * __qdf_frag_debug_delete_node()- Remove frag node from debug tracker
 * @p_node: Frag node address in debug tracker
 * @idx: Index
 *
 * Return: none
 */
static void __qdf_frag_debug_delete_node(QDF_FRAG_TRACK *p_node, uint32_t idx)
{
	if (idx < QDF_FRAG_TRACK_MAX_SIZE) {
		qdf_list_remove_node(&gp_qdf_frag_track_tbl[idx].track_list,
				     &p_node->hnode);
		qdf_frag_track_free(p_node);
	} else {
		qdf_info("Index value exceeds %d for delete node operation",
			  QDF_FRAG_TRACK_MAX_SIZE);
	}
}

void qdf_frag_debug_add_node(qdf_frag_t fragp, const char *func_name,
			     uint32_t line_num)
{
	uint32_t index;
	QDF_FRAG_TRACK *p_node;

	if (is_initial_mem_debug_disabled)
		return;

	index = qdf_frag_debug_hash(fragp);

	qdf_spin_lock_irqsave(&gp_qdf_frag_track_tbl[index].list_lock);

	p_node = qdf_frag_debug_look_up(fragp);

	if (p_node) {
		qdf_info("Double addition of frag %pK to debug tracker!!",
			 fragp);
		qdf_info("Already added from %s %d Current addition from %s %d",
			  p_node->alloc_func_name,
			  p_node->alloc_func_line, func_name, line_num);
	} else {
		p_node = __qdf_frag_debug_add_node(fragp, index, func_name,
						   line_num);
		if (!p_node)
			qdf_info("Memory allocation failed !! "
				 "Add node oprt failed for frag %pK from %s %d",
				 fragp, func_name, line_num);
	}
	qdf_spin_unlock_irqrestore(&gp_qdf_frag_track_tbl[index].list_lock);
}

void qdf_frag_debug_refcount_inc(qdf_frag_t fragp, const char *func_name,
				 uint32_t line_num)
{
	uint32_t index;
	QDF_FRAG_TRACK *p_node;

	if (is_initial_mem_debug_disabled)
		return;

	index = qdf_frag_debug_hash(fragp);

	qdf_spin_lock_irqsave(&gp_qdf_frag_track_tbl[index].list_lock);

	p_node = qdf_frag_debug_look_up(fragp);

	if (p_node) {
		(p_node->refcount)++;

		qdf_str_lcopy(p_node->last_func_name, func_name,
			      QDF_MEM_FUNC_NAME_SIZE);
		p_node->last_func_line = line_num;
	} else {
		p_node = __qdf_frag_debug_add_node(fragp, index, func_name,
						   line_num);
		if (p_node)
			p_node->refcount = QDF_NBUF_FRAG_DEBUG_COUNT_ONE;
		else
			qdf_info("Memory allocation failed !! "
				 "Refcount inc failed for frag %pK from %s %d",
				 fragp, func_name, line_num);
	}
	qdf_spin_unlock_irqrestore(&gp_qdf_frag_track_tbl[index].list_lock);
}

void qdf_frag_debug_refcount_dec(qdf_frag_t fragp, const char *func_name,
				 uint32_t line_num)
{
	uint32_t index;
	QDF_FRAG_TRACK *p_node;

	if (is_initial_mem_debug_disabled)
		return;

	index = qdf_frag_debug_hash(fragp);

	qdf_spin_lock_irqsave(&gp_qdf_frag_track_tbl[index].list_lock);

	p_node = qdf_frag_debug_look_up(fragp);

	if (p_node) {
		if (!(p_node->refcount)) {
			qdf_info("Refcount dec oprt for frag %pK not permitted "
				 "as refcount=0", fragp);
			goto done;
		}
		(p_node->refcount)--;

		if (!(p_node->refcount)) {
			/* Remove frag debug node when refcount reaches 0 */
			__qdf_frag_debug_delete_node(p_node, index);
		} else {
			qdf_str_lcopy(p_node->last_func_name, func_name,
				      QDF_MEM_FUNC_NAME_SIZE);
			p_node->last_func_line = line_num;
		}
	} else {
		qdf_info("Unallocated frag !! Could not track frag %pK", fragp);
		qdf_info("Refcount dec oprt failed for frag %pK from %s %d",
			 fragp, func_name, line_num);
	}
done:
	qdf_spin_unlock_irqrestore(&gp_qdf_frag_track_tbl[index].list_lock);
}

void qdf_frag_debug_delete_node(qdf_frag_t fragp, const char *func_name,
				uint32_t line_num)
{
	uint32_t index;
	QDF_FRAG_TRACK *p_node;

	if (is_initial_mem_debug_disabled)
		return;

	index = qdf_frag_debug_hash(fragp);

	qdf_spin_lock_irqsave(&gp_qdf_frag_track_tbl[index].list_lock);

	p_node = qdf_frag_debug_look_up(fragp);

	if (p_node) {
		if (p_node->refcount) {
			qdf_info("Frag %pK has refcount %d", fragp,
				 p_node->refcount);
			qdf_info("Delete oprt failed for frag %pK from %s %d",
				 fragp, func_name, line_num);
		} else {
			/* Remove node from tracker as refcount=0 */
			__qdf_frag_debug_delete_node(p_node, index);
		}
	} else {
		qdf_info("Unallocated frag !! Double free of frag %pK", fragp);
		qdf_info("Could not track frag %pK for delete oprt from %s %d",
			 fragp, func_name, line_num);
	}

	qdf_spin_unlock_irqrestore(&gp_qdf_frag_track_tbl[index].list_lock);
}

void qdf_frag_debug_update_addr(qdf_frag_t p_fragp, qdf_frag_t n_fragp,
				const char *func_name, uint32_t line_num)
{
	uint32_t prev_index, new_index;
	QDF_FRAG_TRACK *p_node;

	if (is_initial_mem_debug_disabled)
		return;

	prev_index = qdf_frag_debug_hash(p_fragp);

	new_index = qdf_frag_debug_hash(n_fragp);

	qdf_spin_lock_irqsave(&gp_qdf_frag_track_tbl[prev_index].list_lock);

	p_node = qdf_frag_debug_look_up(p_fragp);

	if (!p_node) {
		qdf_info("Unallocated frag !! Could not track frag %pK",
			 p_fragp);
		qdf_info("Update address oprt failed for frag %pK from %s %d",
			 p_fragp, func_name, line_num);
		qdf_spin_unlock_irqrestore(
				&gp_qdf_frag_track_tbl[prev_index].list_lock);
	} else {
		/* Update frag address */
		p_node->p_frag = n_fragp;

		qdf_str_lcopy(p_node->last_func_name, func_name,
			      QDF_MEM_FUNC_NAME_SIZE);
		p_node->last_func_line = line_num;

		if (prev_index != new_index) {
			qdf_list_remove_node(
				&gp_qdf_frag_track_tbl[prev_index].track_list,
				&p_node->hnode);

			qdf_spin_unlock_irqrestore(
				&gp_qdf_frag_track_tbl[prev_index].list_lock);

			qdf_spin_lock_irqsave(
				&gp_qdf_frag_track_tbl[new_index].list_lock);

			qdf_list_insert_front(
				&gp_qdf_frag_track_tbl[new_index].track_list,
				&p_node->hnode);

			qdf_spin_unlock_irqrestore(
				&gp_qdf_frag_track_tbl[new_index].list_lock);
		} else {
			qdf_spin_unlock_irqrestore(
				&gp_qdf_frag_track_tbl[prev_index].list_lock);
		}
	}
}

qdf_frag_t qdf_frag_alloc_debug(unsigned int frag_size, const char *func_name,
				uint32_t line_num)
{
	qdf_frag_t p_frag;

	if (is_initial_mem_debug_disabled)
		return __qdf_frag_alloc(frag_size);

	p_frag =  __qdf_frag_alloc(frag_size);

	/* Store frag in QDF Frag Tracking Table */
	if (qdf_likely(p_frag))
		qdf_frag_debug_add_node(p_frag, func_name, line_num);

	return p_frag;
}

qdf_export_symbol(qdf_frag_alloc_debug);

void qdf_frag_free_debug(qdf_frag_t vaddr, const char *func_name,
			 uint32_t line_num)
{
	if (qdf_unlikely(!vaddr))
		return;

	if (is_initial_mem_debug_disabled)
		goto free_frag;

	qdf_frag_debug_delete_node(vaddr, func_name, line_num);
free_frag:
	__qdf_frag_free(vaddr);
}

qdf_export_symbol(qdf_frag_free_debug);

#endif /* NBUF_FRAG_MEMORY_DEBUG */

#if defined(HIF_PCI)
QDF_STATUS __qdf_mem_map_page(qdf_device_t osdev, __qdf_frag_t buf,
			      qdf_dma_dir_t dir, size_t nbytes,
			      qdf_dma_addr_t *phy_addr)
{
	struct page *page;
	unsigned long offset;

	page = virt_to_head_page(buf);
	offset = buf - page_address(page);
	*phy_addr = dma_map_page(osdev->dev, page, offset, nbytes,
				 __qdf_dma_dir_to_os(dir));

	return dma_mapping_error(osdev->dev, *phy_addr) ?
		QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS __qdf_mem_map_page(qdf_device_t osdev, __qdf_frag_t buf,
			      qdf_dma_dir_t dir, size_t nbytes,
			      qdf_dma_addr_t *phy_addr)
{
	return QDF_STATUS_SUCCESS;
}
#endif

qdf_export_symbol(__qdf_mem_map_page);

#if defined(HIF_PCI)
void __qdf_mem_unmap_page(qdf_device_t osdev, qdf_dma_addr_t paddr,
			  size_t nbytes, qdf_dma_dir_t dir)
{
	dma_unmap_page(osdev->dev, paddr, nbytes,
		       __qdf_dma_dir_to_os(dir));
}
#else
void __qdf_mem_unmap_page(qdf_device_t osdev, qdf_dma_addr_t paddr,
			  size_t nbytes, qdf_dma_dir_t dir)
{
}
#endif

qdf_export_symbol(__qdf_mem_unmap_page);
