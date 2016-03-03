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
QDF_STATUS qdf_list_insert_front(qdf_list_t *list, qdf_list_node_t *node);

QDF_STATUS qdf_list_insert_back_size(qdf_list_t *list, qdf_list_node_t *node,
				     uint32_t *size);

QDF_STATUS qdf_list_remove_front(qdf_list_t *list, qdf_list_node_t **node1);

QDF_STATUS qdf_list_peek_next(qdf_list_t *list,	qdf_list_node_t *node,
			      qdf_list_node_t **node1);

/**
 * qdf_list_create() - Initialize list head
 * @list: object of list
 * @max_size: max size of the list
 * Return: none
 */
static inline void qdf_list_create(__qdf_list_t *list, uint32_t max_size)
{
	__qdf_list_create(list, max_size);
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
	return list->count;
}

QDF_STATUS qdf_list_insert_back(qdf_list_t *list, qdf_list_node_t *node);

QDF_STATUS qdf_list_remove_back(qdf_list_t *list, qdf_list_node_t **node1);

QDF_STATUS qdf_list_peek_front(qdf_list_t *list, qdf_list_node_t **node1);

QDF_STATUS qdf_list_remove_node(qdf_list_t *list,
				qdf_list_node_t *node_to_remove);

bool qdf_list_empty(qdf_list_t *list);

#endif /* __QDF_LIST_H */
