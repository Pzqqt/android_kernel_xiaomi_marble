/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_slist.h
 *
 * A minimal, singly linked list implementation, with push front, pop front, and
 * remove capabilities. These are all O(1) operations.
 *
 * In order to remove an item, a pointer to the previous item must be known.
 * Thus, removing an item is most efficient when combined with
 * qdf_slist_for_each_del(). For cases where you need efficient removal of an
 * arbitrary list node without iteration, consider using the doubly linked list
 * qdf_list instead.
 */

#ifndef __QDF_SLIST_H
#define __QDF_SLIST_H

#include "qdf_trace.h"
#include "qdf_util.h"

#define __qdf_slist_poison ((void *)0xdeaddeaddeaddeadull)

/**
 * struct qdf_slist - a singly linked list
 * @head: pointer to the head of the list
 */
struct qdf_slist {
	struct qdf_slist_node *head;
};

/**
 * struct qdf_slist_node - a singly linked list node
 * @next: pointer to the next node in the list, NULL if there is none
 */
struct qdf_slist_node {
	struct qdf_slist_node *next;
};

#define __qdf_slist_item(node, cursor, node_field) ({ \
	struct qdf_slist_node *__n = (node); \
	(__n ? qdf_container_of(__n, typeof(*(cursor)), node_field) : NULL); })

#define __qdf_slist_next_item(slist, cursor, node_field) \
	__qdf_slist_item(cursor ? (cursor)->node_field.next : \
			 (slist)->head, cursor, node_field)

/**
 * qdf_slist_for_each - iterate over all of the items in @slist
 * @slist: pointer to the qdf_slist to iterate over
 * @cursor: cursor pointer of the list's item type, populated for each item
 * @node_field: name of the qdf_slist_node field in the item's type
 */
#define qdf_slist_for_each(slist, cursor, node_field) \
	for (cursor = __qdf_slist_item((slist)->head, cursor, node_field); \
	     cursor; \
	     cursor = __qdf_slist_item((cursor)->node_field.next, \
				       cursor, node_field))

/**
 * qdf_slist_for_each_del - iterate over all of the items in @slist,
 *	allowing for the safe deletion of nodes during iteration
 * @slist: pointer to the qdf_slist to iterate over
 * @prev: cursor pointer, populated with the previous item
 * @cursor: cursor pointer of the list's item type, populated for each item
 * @node_field: name of the qdf_slist_node field in the item's type
 */
#define qdf_slist_for_each_del(slist, prev, cursor, node_field) \
	for (prev = NULL, \
	     cursor = __qdf_slist_item((slist)->head, cursor, node_field); \
	     cursor; \
	     prev = __qdf_slist_next_item(slist, prev, node_field) == \
		cursor ? cursor : prev, \
	     cursor = __qdf_slist_next_item(slist, prev, node_field))

/**
 * qdf_slist_init() - initialize a qdf_slist
 * @slist: the list to initialize
 *
 * Return: None
 */
static inline void qdf_slist_init(struct qdf_slist *slist)
{
	slist->head = NULL;
}

/**
 * qdf_slist_deinit() - deinitialize a qdf_slist
 * @slist: the list to deinitialize
 *
 * Return: None
 */
static inline void qdf_slist_deinit(struct qdf_slist *slist)
{
	QDF_BUG(!slist->head);
	slist->head = __qdf_slist_poison;
}

/**
 * qdf_slist_empty() - check if a qdf_slist is empty
 * @slist: the list to check
 *
 * Return: true if @slist contains zero items
 */
static inline bool qdf_slist_empty(struct qdf_slist *slist)
{
	return !slist->head;
}

/**
 * qdf_slist_push() - push an item into the front of a qdf_slist
 * @slist: the list to push into
 * @cursor: the item to push
 * @node_field: name of the qdf_slist_node field in the item's type
 *
 * Return: None
 */
#define qdf_slist_push(slist, cursor, node_field) \
	__qdf_slist_push(slist, &(cursor)->node_field)

static inline void
__qdf_slist_push(struct qdf_slist *slist, struct qdf_slist_node *node)
{
	node->next = slist->head;
	slist->head = node;
}

/**
 * qdf_slist_pop() - pop an item from the front of a qdf_slist
 * @slist: the list to pop from
 * @cursor: cursor pointer of the list's item type, not populated
 * @node_field: name of the qdf_slist_node field in the item's type
 *
 * Return: pointer to the popped item, NULL if @slist was empty
 */
#define qdf_slist_pop(slist, cursor, node_field) \
	__qdf_slist_item(__qdf_slist_pop(slist), cursor, node_field)

static inline struct qdf_slist_node *__qdf_slist_pop(struct qdf_slist *slist)
{
	struct qdf_slist_node *node = slist->head;

	if (!node)
		return NULL;

	slist->head = node->next;
	node->next = __qdf_slist_poison;

	return node;
}

/**
 * qdf_slist_remove() - remove an item from a qdf_slist
 * @slist: the list to remove from
 * @prev: pointer to the item previous to the item to remove, NULL removes head
 * @node_field: name of the qdf_slist_node field in the item's type
 *
 * Return: pointer to the removed item, NULL if none was removed
 */
#define qdf_slist_remove(slist, prev, node_field) \
	__qdf_slist_item(__qdf_slist_remove(slist, \
			 prev ? &(prev)->node_field : NULL), prev, node_field)

static inline struct qdf_slist_node *
__qdf_slist_remove(struct qdf_slist *slist, struct qdf_slist_node *prev)
{
	struct qdf_slist_node *node;

	if (!prev)
		return __qdf_slist_pop(slist);

	if (!prev->next)
		return NULL;

	node = prev->next;
	prev->next = node->next;
	node->next = __qdf_slist_poison;

	return node;
}

#endif /* __QDF_SLIST_H */

