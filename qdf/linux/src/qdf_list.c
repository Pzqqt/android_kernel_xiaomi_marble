/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_list.c
 *
 * QCA driver framework list manipulation APIs. QDF linked list
 * APIs are NOT thread safe so make sure to use appropriate locking mechanisms
 * to assure operations on the list are thread safe.
 */

/* Include files */
#include <qdf_list.h>
#include <qdf_module.h>

/* Function declarations and documenation */

QDF_STATUS qdf_list_insert_before(qdf_list_t *list,
	qdf_list_node_t *new_node, qdf_list_node_t *node)
{
	list_add_tail(new_node, node);
	list->count++;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_list_insert_before);

QDF_STATUS qdf_list_insert_after(qdf_list_t *list,
	qdf_list_node_t *new_node, qdf_list_node_t *node)
{
	list_add(new_node, node);
	list->count++;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_list_insert_after);

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
qdf_export_symbol(qdf_list_insert_front);

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
qdf_export_symbol(qdf_list_insert_back);

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
qdf_export_symbol(qdf_list_insert_back_size);

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
	list_del_init(list->anchor.next);
	list->count--;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_list_remove_front);

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
	list_del_init(list->anchor.prev);
	list->count--;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_list_remove_back);

bool qdf_list_has_node(qdf_list_t *list, qdf_list_node_t *node)
{
	qdf_list_node_t *tmp;

	list_for_each(tmp, &list->anchor) {
		if (tmp == node)
			return true;
	}

	return false;
}

/**
 * qdf_list_remove_node() - remove input node from list
 * @list: Pointer to list
 * @node_to_remove: Pointer to node which needs to be removed
 *
 * verifies that the node is in the list before removing it.
 * It is expected that the list being removed from is locked
 * when this function is being called.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_remove_node(qdf_list_t *list,
				qdf_list_node_t *node_to_remove)
{
	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	list_del_init(node_to_remove);
	list->count--;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_list_remove_node);

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
qdf_export_symbol(qdf_list_peek_front);

/**
 * qdf_list_peek_next() - peek next node of input node in the list
 * @list: Pointer to list
 * @node: Pointer to input node
 * @node2: Double pointer to store peeked node pointer
 *
 * Return: QDF status
 */
QDF_STATUS qdf_list_peek_next(qdf_list_t *list,
			      qdf_list_node_t *node,
			      qdf_list_node_t **node2)
{
	if (!list || !node || !node2)
		return QDF_STATUS_E_FAULT;

	if (list_empty(&list->anchor))
		return QDF_STATUS_E_EMPTY;

	if (node->next == &list->anchor)
		return QDF_STATUS_E_EMPTY;

	*node2 = node->next;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_list_peek_next);

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
qdf_export_symbol(qdf_list_empty);

bool qdf_list_node_in_any_list(const qdf_list_node_t *node)
{
	const struct list_head *linux_node = (const struct list_head *)node;

	if (!linux_node)
		return false;

	/* if the node is an empty list, it is not tied to an anchor node */
	if (list_empty(linux_node))
		return false;

	if (!linux_node->prev || !linux_node->next)
		return false;

	if (linux_node->prev->next != linux_node ||
	    linux_node->next->prev != linux_node)
		return false;

	return true;
}

