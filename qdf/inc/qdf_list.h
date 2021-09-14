/*
 * Copyright (c) 2014-2018, 2021 The Linux Foundation. All rights reserved.
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
 *  DOC: qdf_list.h
 *  QCA driver framework (QDF) list APIs
 *  Definitions for QDF Linked Lists API
 *
 *  Lists are implemented as a doubly linked list. An item in a list can
 *  be of any type as long as the datatype contains a field of type
 *  qdf_link_t.
 *
 *  In general, a list is a doubly linked list of items with a pointer
 *  to the front of the list and a pointer to the end of the list.  The
 *  list items contain a forward and back link.
 *
 *  QDF linked list APIs are NOT thread safe so make sure to use appropriate
 *  locking mechanisms to assure operations on the list are thread safe.
 */

#if !defined(__QDF_LIST_H)
#define __QDF_LIST_H

/* Include Files */
#include <qdf_types.h>
#include <qdf_status.h>
#include <i_qdf_list.h>
#include <qdf_trace.h>

typedef __qdf_list_node_t qdf_list_node_t;
typedef __qdf_list_t qdf_list_t;

/* Function declarations */

/**
 * qdf_list_insert_before() - insert new node before the node
 * @list: Pointer to list
 * @new_node: Pointer to input node
 * @node: node before which new node should be added.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_insert_before(qdf_list_t *list,
	qdf_list_node_t *new_node, qdf_list_node_t *node);
/**
 * qdf_list_insert_after() - insert new node after the node
 * @list: Pointer to list
 * @new_node: Pointer to input node
 * @node: node after which new node should be added.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_insert_after(qdf_list_t *list,
	qdf_list_node_t *new_node, qdf_list_node_t *node);
QDF_STATUS qdf_list_insert_front(qdf_list_t *list, qdf_list_node_t *node);

QDF_STATUS qdf_list_insert_back_size(qdf_list_t *list, qdf_list_node_t *node,
				     uint32_t *size);

QDF_STATUS qdf_list_remove_front(qdf_list_t *list, qdf_list_node_t **node1);

QDF_STATUS qdf_list_peek_next(qdf_list_t *list,	qdf_list_node_t *node,
			      qdf_list_node_t **node1);

/**
 * qdf_list_create() - Create qdf list and initialize list head
 * @list: object of list
 * @max_size: max size of the list
 *
 * Return: none
 */
static inline void qdf_list_create(__qdf_list_t *list, uint32_t max_size)
{
	__qdf_list_create(list, max_size);
}

#define QDF_LIST_ANCHOR(list) __QDF_LIST_ANCHOR(list)

#define QDF_LIST_NODE_INIT(prev, next) __QDF_LIST_NODE_INIT(prev, next)
#define QDF_LIST_NODE_INIT_SINGLE(node) __QDF_LIST_NODE_INIT_SINGLE(node)

#define QDF_LIST_INIT(tail, head) __QDF_LIST_INIT(tail, head)
#define QDF_LIST_INIT_SINGLE(node) __QDF_LIST_INIT_SINGLE(node)
#define QDF_LIST_INIT_EMPTY(list) __QDF_LIST_INIT_EMPTY(list)

#define qdf_list_for_each(list_ptr, cursor, node_field) \
	__qdf_list_for_each(list_ptr, cursor, node_field)

#define qdf_list_for_each_del(list_ptr, cursor, next, node_field) \
	__qdf_list_for_each_del(list_ptr, cursor, next, node_field)

#define qdf_list_for_each_from(list_ptr, cursor, node_field) \
	__qdf_list_for_each_from(list_ptr, cursor, node_field)

#define qdf_list_first_entry_or_null(list_ptr, type, node_field) \
	__qdf_list_first_entry_or_null(list_ptr, type, node_field)

/**
 * qdf_init_list_head() - initialize list head
 * @list_head: pointer to list head
 *
 * Return: none
 */
static inline void qdf_init_list_head(__qdf_list_node_t *list_head)
{
	__qdf_init_list_head(list_head);
}

/**
 * qdf_list_destroy() - Destroy the list
 * @list: object of list
 * Return: none
 */
static inline void qdf_list_destroy(qdf_list_t *list)
{
	if (list->count != 0) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: list length not equal to zero", __func__);
		QDF_ASSERT(0);
	}
}

/**
 * qdf_list_size() - gives the size of the list
 * @list: object of list
 * @size: size of the list
 * Return: uint32_t
 */
static inline uint32_t qdf_list_size(qdf_list_t *list)
{
	return __qdf_list_size(list);
}

/**
 * qdf_list_max_size() - gives the max size of the list
 * @list: object of list
 * Return: max size of the list
 */
static inline uint32_t qdf_list_max_size(qdf_list_t *list)
{
	return __qdf_list_max_size(list);
}

QDF_STATUS qdf_list_insert_back(qdf_list_t *list, qdf_list_node_t *node);

QDF_STATUS qdf_list_remove_back(qdf_list_t *list, qdf_list_node_t **node1);

QDF_STATUS qdf_list_peek_front(qdf_list_t *list, qdf_list_node_t **node1);

QDF_STATUS qdf_list_remove_node(qdf_list_t *list,
				qdf_list_node_t *node_to_remove);

bool qdf_list_empty(qdf_list_t *list);

/**
 * qdf_list_has_node() - check if a node is in a list
 * @list: pointer to the list being searched
 * @node: pointer to the node to search for
 *
 * This API has a time complexity of O(n).
 *
 * Return: true if the node is in the list
 */
bool qdf_list_has_node(qdf_list_t *list, qdf_list_node_t *node);

/**
 * qdf_list_node_in_any_list() - ensure @node is a member of a list
 * @node: list node to check
 *
 * This API has a time complexity of O(1). See also qdf_list_has_node().
 *
 * Return: true, if @node appears to be in a list
 */
bool qdf_list_node_in_any_list(const qdf_list_node_t *node);

#endif /* __QDF_LIST_H */
