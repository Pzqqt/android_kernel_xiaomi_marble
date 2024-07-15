/*
 * Copyright (c) 2018, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_talloc.c
 *
 * OS-independent talloc implementation
 */

#ifdef WLAN_TALLOC_DEBUG

#include "i_qdf_talloc.h"
#include "qdf_hashtable.h"
#include "qdf_list.h"
#include "qdf_mc_timer.h"
#include "qdf_mem.h"
#include "qdf_module.h"
#include "qdf_status.h"
#include "qdf_str.h"
#include "qdf_talloc.h"

#define QDF_TALLOC_MAX_BYTES __page_size
#define QDF_TALLOC_SLEEP_TIMEOUT_MS 300
#define QDF_TALLOC_FUNC_NAME_SIZE 48
#define QDF_TALLOC_HT_BITS 8 /* 256 buckets */

static void
__qdf_talloc_log_nomem(const size_t size, const char *func, const uint16_t line)
{
	qdf_nofl_info("Failed to alloc %zuB; via %s():%d", size, func, line);
}

static void *
__qdf_zalloc_auto(const size_t size, const char *func, const uint16_t line)
{
	unsigned long start, duration;
	void *ptr;

	start = qdf_mc_timer_get_system_time();
	ptr = __zalloc_auto(size);
	duration = qdf_mc_timer_get_system_time() - start;

	if (duration > QDF_TALLOC_SLEEP_TIMEOUT_MS)
		qdf_nofl_info("Alloc slept; %lums, %zuB; via %s():%d",
			      duration, size, func, line);

	if (!ptr) {
		__qdf_talloc_log_nomem(size, func, line);
		return NULL;
	}

	qdf_mem_kmalloc_inc(__qdf_alloc_size(ptr));

	return ptr;
}

static void *
__qdf_zalloc_atomic(const size_t size, const char *func, const uint16_t line)
{
	void *ptr;

	ptr = __zalloc_atomic(size);
	if (!ptr) {
		__qdf_talloc_log_nomem(size, func, line);
		return NULL;
	}

	qdf_mem_kmalloc_inc(__qdf_alloc_size(ptr));

	return ptr;
}

static void __qdf_free(const void *ptr)
{
	qdf_mem_kmalloc_dec(__qdf_alloc_size(ptr));

	__k_free(ptr);
}

static qdf_ht_declare(__qdf_talloc_meta_ht, QDF_TALLOC_HT_BITS);
static qdf_spinlock_t __qdf_talloc_meta_lock;

/**
 * struct qdf_talloc_parent_meta - parent/children metadata for memory tracking
 * @entry: entry for membership in the parent hashtable
 * @children: list of associated children
 */
struct qdf_talloc_parent_meta {
	struct qdf_ht_entry entry;
	uintptr_t key;
	qdf_list_t children;
};

static struct qdf_talloc_parent_meta *
qdf_talloc_parent_meta_alloc(const void *parent,
			     const char *func, const uint16_t line)
{
	struct qdf_talloc_parent_meta *pmeta;

	pmeta = __qdf_zalloc_atomic(sizeof(*pmeta), func, line);
	if (!pmeta)
		return NULL;

	pmeta->key = (uintptr_t)parent;
	qdf_list_create(&pmeta->children, 0);
	qdf_ht_add(__qdf_talloc_meta_ht, &pmeta->entry, pmeta->key);

	return pmeta;
}

static void qdf_talloc_parent_meta_free(struct qdf_talloc_parent_meta *pmeta)
{
	qdf_ht_remove(&pmeta->entry);
	qdf_list_destroy(&pmeta->children);
	__k_free(pmeta);
}

static struct qdf_talloc_parent_meta *
qdf_talloc_parent_meta_lookup(const void *parent)
{
	struct qdf_talloc_parent_meta *pmeta;
	uintptr_t key = (uintptr_t)parent;

	qdf_ht_get(__qdf_talloc_meta_ht, pmeta, entry, key, key);

	return pmeta;
}

/**
 * struct qdf_talloc_child_meta - talloc child debug information
 * @parent: parent pointer used during allocation for leak tracking
 * @node: list node for membership in @parent's children list
 * @func: name of the function that requested the allocation
 * @line: line number of the call site in @func
 * @size: size of the allocation in bytes
 */
struct qdf_talloc_child_meta {
	const void *parent;
	qdf_list_node_t node;
	char func[QDF_TALLOC_FUNC_NAME_SIZE];
	uint16_t line;
	uint32_t size;
	uint32_t guard;
};

/**
 * struct qdf_talloc_header - talloc debug header information
 * @meta: child allocation metadata
 * @guard: a known value, used to detect out-of-bounds access
 */
struct qdf_talloc_header {
	struct qdf_talloc_child_meta meta;
	uint32_t guard;
};

/**
 * struct qdf_talloc_trailer - talloc debug trailer information
 * @guard: a known value, used to detect out-of-bounds access
 */
struct qdf_talloc_trailer {
	uint32_t guard;
};

static uint32_t QDF_TALLOC_GUARD = 0xaabbeeff;

#define QDF_TALLOC_DEBUG_SIZE \
	(sizeof(struct qdf_talloc_header) + sizeof(struct qdf_talloc_trailer))

static struct qdf_talloc_header *qdf_talloc_header(void *ptr)
{
	return (struct qdf_talloc_header *)ptr - 1;
}

static void *qdf_talloc_ptr(struct qdf_talloc_header *header)
{
	return header + 1;
}

static struct qdf_talloc_trailer *
qdf_talloc_trailer(struct qdf_talloc_header *header)
{
	void *ptr = qdf_talloc_ptr(header);
	size_t size = header->meta.size;

	return (struct qdf_talloc_trailer *)((uint8_t *)ptr + size);
}

static void qdf_talloc_meta_init(struct qdf_talloc_header *header,
				 const void *parent, const size_t size,
				 const char *func, const uint16_t line)
{
	struct qdf_talloc_trailer *trailer;

	/* copy the function name to support multi-*.ko configurations */
	qdf_str_lcopy(header->meta.func, func, sizeof(header->meta.func));
	header->meta.parent = parent;
	header->meta.line = line;
	header->meta.size = size;
	header->guard = QDF_TALLOC_GUARD;

	trailer = qdf_talloc_trailer(header);
	trailer->guard = QDF_TALLOC_GUARD;
}

static bool qdf_talloc_meta_assert_valid(struct qdf_talloc_header *header,
					 const char *func, const uint16_t line)
{
	struct qdf_talloc_trailer *trailer = qdf_talloc_trailer(header);
	bool is_valid = true;

	if (header->guard != QDF_TALLOC_GUARD) {
		qdf_nofl_alert("Corrupted header guard 0x%x (expected 0x%x)",
			       header->guard, QDF_TALLOC_GUARD);
		is_valid = false;
	}

	if (header->meta.size > QDF_TALLOC_MAX_BYTES) {
		qdf_nofl_alert("Corrupted allocation size %u (expected <= %zu)",
			       header->meta.size, QDF_TALLOC_MAX_BYTES);
		is_valid = false;
	}

	if (!qdf_list_node_in_any_list(&header->meta.node)) {
		qdf_nofl_alert("Corrupted header node or double free");
		is_valid = false;
	}

	if (trailer->guard != QDF_TALLOC_GUARD) {
		qdf_nofl_alert("Corrupted trailer guard 0x%x (expected 0x%x)",
			       trailer->guard, QDF_TALLOC_GUARD);
		is_valid = false;
	}

	if (!is_valid)
		QDF_DEBUG_PANIC("Fatal memory error detected @ %s():%d",
				func, line);

	return is_valid;
}

static void qdf_leaks_print_header(void)
{
	qdf_nofl_alert("-----------------------------------------------------");
	qdf_nofl_alert(" size      function():line");
	qdf_nofl_alert("-----------------------------------------------------");
}

static uint32_t qdf_leaks_print(const struct qdf_talloc_parent_meta *pmeta)
{
	struct qdf_talloc_child_meta *cmeta;
	uint32_t count = 0;

	qdf_list_for_each(&pmeta->children, cmeta, node) {
		qdf_nofl_alert("%6uB @ %s():%u",
			       cmeta->size, cmeta->func, cmeta->line);
		count++;
	}

	return count;
}

#define qdf_leaks_panic(count, func, line) \
	QDF_DEBUG_PANIC("%u fatal memory leaks detected @ %s():%u", \
			count, func, line)

QDF_STATUS qdf_talloc_feature_init(void)
{
	qdf_spinlock_create(&__qdf_talloc_meta_lock);
	qdf_ht_init(__qdf_talloc_meta_ht);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_talloc_feature_init);

void qdf_talloc_feature_deinit(void)
{
	qdf_spin_lock_bh(&__qdf_talloc_meta_lock);

	if (!qdf_ht_empty(__qdf_talloc_meta_ht)) {
		struct qdf_talloc_parent_meta *pmeta;
		uint32_t count = 0;
		int i;

		qdf_leaks_print_header();

		qdf_ht_for_each(__qdf_talloc_meta_ht, i, pmeta, entry)
			count += qdf_leaks_print(pmeta);

		qdf_leaks_panic(count, __func__, __LINE__);
	}

	qdf_spin_unlock_bh(&__qdf_talloc_meta_lock);

	qdf_ht_deinit(__qdf_talloc_meta_ht);
	qdf_spinlock_destroy(&__qdf_talloc_meta_lock);
}
qdf_export_symbol(qdf_talloc_feature_deinit);

static QDF_STATUS qdf_talloc_meta_insert(struct qdf_talloc_header *header,
					 const char *func, const uint16_t line)
{
	struct qdf_talloc_child_meta *cmeta = &header->meta;
	struct qdf_talloc_parent_meta *pmeta;

	pmeta = qdf_talloc_parent_meta_lookup(cmeta->parent);
	if (!pmeta)
		pmeta = qdf_talloc_parent_meta_alloc(cmeta->parent, func, line);
	if (!pmeta)
		return QDF_STATUS_E_NOMEM;

	qdf_list_insert_back(&pmeta->children, &cmeta->node);

	return QDF_STATUS_SUCCESS;
}

void *__qdf_talloc_fl(const void *parent, const size_t size,
		      const char *func, const uint16_t line)
{
	QDF_STATUS status;
	struct qdf_talloc_header *header;

	QDF_BUG(parent);
	if (!parent)
		return NULL;

	QDF_BUG(size <= QDF_TALLOC_MAX_BYTES);
	if (size > QDF_TALLOC_MAX_BYTES)
		return NULL;

	header = __qdf_zalloc_auto(size + QDF_TALLOC_DEBUG_SIZE, func, line);
	if (!header)
		return NULL;

	qdf_talloc_meta_init(header, parent, size, func, line);

	qdf_spin_lock_bh(&__qdf_talloc_meta_lock);
	status = qdf_talloc_meta_insert(header, func, line);
	qdf_spin_unlock_bh(&__qdf_talloc_meta_lock);

	if (QDF_IS_STATUS_ERROR(status)) {
		__qdf_free(header);
		return NULL;
	}

	return qdf_talloc_ptr(header);
}
qdf_export_symbol(__qdf_talloc_fl);

static void
__qdf_talloc_assert_no_children(const void *parent,
				const char *func, const uint16_t line)
{
	struct qdf_talloc_parent_meta *pmeta;
	uint32_t count;

	pmeta = qdf_talloc_parent_meta_lookup(parent);
	if (!pmeta)
		return;

	qdf_leaks_print_header();
	count = qdf_leaks_print(pmeta);
	qdf_leaks_panic(count, func, line);
}

static void qdf_talloc_meta_remove(struct qdf_talloc_header *header,
				   const char *func, const uint16_t line)
{
	struct qdf_talloc_child_meta *cmeta = &header->meta;
	struct qdf_talloc_parent_meta *pmeta;

	__qdf_talloc_assert_no_children(qdf_talloc_ptr(header), func, line);

	pmeta = qdf_talloc_parent_meta_lookup(cmeta->parent);
	if (!pmeta) {
		QDF_DEBUG_PANIC("double-free or free-no-allocate @ %s():%u",
				func, line);
		return;
	}

	qdf_list_remove_node(&pmeta->children, &cmeta->node);

	if (qdf_list_empty(&pmeta->children))
		qdf_talloc_parent_meta_free(pmeta);
}

void __qdf_tfree_fl(void *ptr, const char *func, const uint16_t line)
{
	struct qdf_talloc_header *header;

	QDF_BUG(ptr);
	if (!ptr)
		return;

	header = qdf_talloc_header(ptr);
	qdf_talloc_meta_assert_valid(header, func, line);

	qdf_spin_lock_bh(&__qdf_talloc_meta_lock);
	qdf_talloc_meta_remove(header, func, line);
	qdf_spin_unlock_bh(&__qdf_talloc_meta_lock);

	__qdf_free(header);
}
qdf_export_symbol(__qdf_tfree_fl);

void qdf_talloc_assert_no_children_fl(const void *parent,
				      const char *func, const uint16_t line)
{
	qdf_spin_lock_bh(&__qdf_talloc_meta_lock);
	__qdf_talloc_assert_no_children(parent, func, line);
	qdf_spin_unlock_bh(&__qdf_talloc_meta_lock);
}
qdf_export_symbol(qdf_talloc_assert_no_children_fl);

#endif /* WLAN_TALLOC_DEBUG */

