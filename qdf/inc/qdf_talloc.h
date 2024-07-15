/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_talloc.h - Public APIs for t(ree) alloc(ate) memory management
 *
 * These APIs allocate memory like malloc, but track those allocations via a
 * parent-child relationship, or tree. If the parent is freed while it still has
 * children, a panic will be triggered. This effectively gives you the ability
 * to limit the lifetime of an allocation by ensuring the child allocation
 * lifetime will be strictly less than the parent allocation lifetime.
 */

#ifndef __QDF_TALLOC_H
#define __QDF_TALLOC_H

#include "i_qdf_talloc.h"
#include "qdf_status.h"

/**
 * qdf_talloc() - t(ree) alloc(ate) memory
 * @parent: the parent memory of the new allocation
 * @size: requested size of the newly allocated memory
 *
 * Return: pointer to the newly allocated memory
 */
#define qdf_talloc(parent, size) \
	qdf_talloc_fl(parent, size, __func__, __LINE__)

/**
 * qdf_talloc_type() - t(ree) alloc(ate) memory for a type
 * @parent: the parent memory of the new allocation
 * @cursor: pointer to the type of memory to allocate
 *
 * This API automatically determines the correct size needed or an allocation
 * based on the type of @cursor. If you need to allocate an arbitrary number
 * of bytes, use qdf_talloc() instead.
 *
 * Return: pointer to the newly allocated memory
 */
#define qdf_talloc_type(parent, cursor) \
	qdf_talloc(parent, sizeof(*(cursor)))

/**
 * qdf_talloc_fl() - t(ree) alloc(ate) memory with function and line info
 * @parent: the parent memory of the new allocation
 * @size: requested size of the newly allocated memory
 * @func: name of the function requesting the allocation
 * @line: line number of the call site in @func
 *
 * Return: pointer to the newly allocated memory
 */
#define qdf_talloc_fl(parent, size, func, line) \
	__qdf_talloc_fl(parent, size, func, line)

/**
 * qdf_tfree() - free memory allocated using the *_talloc() function family
 * @inout_ptr: double point to memory to free, set to NULL
 *
 * Return: None
 */
#define qdf_tfree(ptr) \
	qdf_tfree_fl(ptr, __func__, __LINE__)

/**
 * qdf_tfree_fl() - free memory allocated using the *_talloc() function family
 *	with function and line info
 * @ptr: pointer to memory to free
 * @func: name of the function requesting the free
 * @line: line number of the call site in @func
 *
 * Return: None
 */
#define qdf_tfree_fl(ptr, func, line) \
do { \
	__qdf_tfree_fl(ptr, func, line); \
	ptr = (void *)1; \
} while (false)

/**
 * qdf_talloc_assert_no_children() - assert @parent has not child allocations
 * @parent: the parent memory ponter to check
 *
 * Return: None
 */
#define qdf_talloc_assert_no_children(parent) \
	qdf_talloc_assert_no_children_fl(parent, __func__, __LINE__)

#ifdef WLAN_TALLOC_DEBUG

/**
 * qdf_talloc_feature_init() - initialize the QDF talloc feature
 *
 * Must be called before allocating memory via a qdf_talloc API.
 *
 * Return: None
 */
QDF_STATUS qdf_talloc_feature_init(void);

/**
 * qdf_talloc_feature_deinit() - deinitialize the QDF talloc feature
 *
 * Memory must not be allocated via a qdf_talloc API after this is called. This
 * API asserts that the parent/child relationship table is empty in order to
 * catch memory leaks.
 *
 * Return: None
 */
void qdf_talloc_feature_deinit(void);

void *__qdf_talloc_fl(const void *parent, const size_t size,
		      const char *func, const uint16_t line);

void __qdf_tfree_fl(void *ptr, const char *func, const uint16_t line);

/**
 * qdf_talloc_assert_no_children_fl() - assert @parent has not child allocations
 * @parent: the parent memory ponter to check
 * @func: name of the function requesting the assert
 * @line: line number of the call site in @func
 *
 * Return: None
 */
void qdf_talloc_assert_no_children_fl(const void *parent,
				      const char *func, const uint16_t line);

#else /* WLAN_TALLOC_DEBUG */

static inline QDF_STATUS qdf_talloc_feature_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline void qdf_talloc_feature_deinit(void) { }

static inline void *__qdf_talloc_fl(const void *parent, const size_t size,
				    const char *func, const uint16_t line)
{
	return __zalloc_auto(size);
}

static inline void
__qdf_tfree_fl(void *ptr, const char *func, const uint16_t line)
{
	__k_free(ptr);
}

static inline void
qdf_talloc_assert_no_children_fl(const void *parent,
				 const char *func, const uint16_t line) { }

#endif /* WLAN_TALLOC_DEBUG */

#endif /* __QDF_TALLOC_H */
