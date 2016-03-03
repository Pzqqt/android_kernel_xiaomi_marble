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
 * DOC: qdf_list.c
 *
 * QCA driver framework list manipulation APIs. QDF linked list
 * APIs are NOT thread safe so make sure to use appropriate locking mechanisms
 * to assure operations on the list are thread safe.
 */

/* Include files */
#include <qdf_list.h>

/* Function declarations and documenation */

/**
 * qdf_list_insert_front() - insert input node at front of the list
 * @list: Pointer to list
 * @node: Pointer to input node
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_insert_front(qdf_list_t *list, qdf_list_node_t *node)
{
	list_add(node, &list->anchor);
	list->count++;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_insert_front);

/**
 * qdf_list_insert_back() - insert input node at back of the list
 * @list: Pointer to list
 * @node: Pointer to input node
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_insert_back(qdf_list_t *list, qdf_list_node_t *node)
{
	list_add_tail(node, &list->anchor);
	list->count++;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_insert_back);

/**
 * qdf_list_insert_back_size() - insert input node at back of list and save
 * list size
 * @list: Pointer to list
 * @node: Pointer to input node
 * @p_size: Pointer to store list size
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_insert_back_size(qdf_list_t *list,
				     qdf_list_node_t *node, uint32_t *p_size)
{
	list_add_tail(node, &list->anchor);
	list->count++;
	*p_size = list->count;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_insert_back_size);

/**
 * qdf_list_remove_front() - remove node from front of the list
 * @list: Pointer to list
 * @node2: Double pointer to store the node which is removed from list
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_remove_front(qdf_list_t *list, qdf_list_node_t **node2)
{
	struct list_head *listptr;

	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	listptr = list->anchor.next;
	*node2 = listptr;
	list_del(list->anchor.next);
	list->count--;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_remove_front);

/**
 * qdf_list_remove_back() - remove node from end of the list
 * @list: Pointer to list
 * @node2: Double pointer to store node which is removed from list
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_remove_back(qdf_list_t *list, qdf_list_node_t **node2)
{
	struct list_head *listptr;

	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	listptr = list->anchor.prev;
	*node2 = listptr;
	list_del(list->anchor.prev);
	list->count--;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_remove_back);

/**
 * qdf_list_remove_node() - remove input node from list
 * @list: Pointer to list
 * @node_to_remove: Pointer to node which needs to be removed
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_remove_node(qdf_list_t *list,
				qdf_list_node_t *node_to_remove)
{
	qdf_list_node_t *tmp;
	int found = 0;

	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	/* verify that node_to_remove is indeed part of list list */
	list_for_each(tmp, &list->anchor) {
		if (tmp == node_to_remove) {
			found = 1;
			break;
		}
	}
	if (found == 0)
		return QDF_STATUS_E_INVAL;

	list_del(node_to_remove);
	list->count--;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_remove_node);

/**
 * qdf_list_peek_front() - peek front node from list
 * @list: Pointer to list
 * @node2: Double pointer to store peeked node pointer
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_peek_front(qdf_list_t *list, qdf_list_node_t **node2)
{
	struct list_head *listptr;
	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	listptr = list->anchor.next;
	*node2 = listptr;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_peek_front);

/**
 * qdf_list_peek_next() - peek next node of input node in the list
 * @list: Pointer to list
 * @node: Pointer to input node
 * @node2: Double pointer to store peeked node pointer
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_peek_next(qdf_list_t *list, qdf_list_node_t *node,
			      qdf_list_node_t **node2)
{
	struct list_head *listptr;
	int found = 0;
	qdf_list_node_t *tmp;

	if ((list == NULL) || (node == NULL) || (node2 == NULL))
		return QDF_STATUS_E_FAULT;

	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	/* verify that node is indeed part of list list */
	list_for_each(tmp, &list->anchor) {
		if (tmp == node) {
			found = 1;
			break;
		}
	}

	if (found == 0)
		return QDF_STATUS_E_INVAL;

	listptr = node->next;
	if (listptr == &list->anchor)
		return QDF_STATUS_E_EMPTY;

	*node2 = listptr;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_list_peek_next);

/**
 * qdf_list_empty() - check if the list is empty
 * @list: pointer to the list
 *
 * Return: true if the list is empty and false otherwise.
 */
bool qdf_list_empty(qdf_list_t *list)
{
	return list_empty(&list->anchor);
}
EXPORT_SYMBOL(qdf_list_empty);
