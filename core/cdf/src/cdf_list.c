/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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
 * DOC: cdf_list.c
 *
 * Connectivity driver framework list manipulation APIs. CDF linked list
 * APIs are NOT thread safe so make sure to use appropriate locking mechanisms
 * to assure operations on the list are thread safe.
 */

/* Include files */
#include <cdf_list.h>
#include <cdf_trace.h>

/* Preprocessor definitions and constants */

/* Type declarations */

/* Function declarations and documenation */

/**
 * cdf_list_insert_front() - insert input node at front of the list
 * @pList: Pointer to list
 * @pNode: Pointer to input node
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_insert_front(cdf_list_t *pList, cdf_list_node_t *pNode)
{
	list_add(pNode, &pList->anchor);
	pList->count++;
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_insert_back() - insert input node at back of the list
 * @pList: Pointer to list
 * @pNode: Pointer to input node
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_insert_back(cdf_list_t *pList, cdf_list_node_t *pNode)
{
	list_add_tail(pNode, &pList->anchor);
	pList->count++;
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_insert_back_size() - insert input node at back of list and save
 *				 list size
 * @pList: Pointer to list
 * @pNode: Pointer to input node
 * @pSize: Pointer to store list size
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_insert_back_size(cdf_list_t *pList,
				     cdf_list_node_t *pNode, uint32_t *pSize)
{
	list_add_tail(pNode, &pList->anchor);
	pList->count++;
	*pSize = pList->count;
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_remove_front() - remove node from front of the list
 * @pList: Pointer to list
 * @ppNode: Double pointer to store the node which is removed from list
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_remove_front(cdf_list_t *pList, cdf_list_node_t **ppNode)
{
	struct list_head *listptr;

	if (list_empty(&pList->anchor))
		return CDF_STATUS_E_EMPTY;

	listptr = pList->anchor.next;
	*ppNode = listptr;
	list_del(pList->anchor.next);
	pList->count--;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_remove_back() - remove node from end of the list
 * @pList: Pointer to list
 * @ppNode: Double pointer to store node which is removed from list
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_remove_back(cdf_list_t *pList, cdf_list_node_t **ppNode)
{
	struct list_head *listptr;

	if (list_empty(&pList->anchor))
		return CDF_STATUS_E_EMPTY;

	listptr = pList->anchor.prev;
	*ppNode = listptr;
	list_del(pList->anchor.prev);
	pList->count--;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_remove_node() - remove input node from list
 * @pList: Pointer to list
 * @pNodeToRemove: Pointer to node which needs to be removed
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_remove_node(cdf_list_t *pList,
				cdf_list_node_t *pNodeToRemove)
{
	cdf_list_node_t *tmp;
	int found = 0;

	if (list_empty(&pList->anchor))
		return CDF_STATUS_E_EMPTY;

	/* verify that pNodeToRemove is indeed part of list pList */
	list_for_each(tmp, &pList->anchor) {
		if (tmp == pNodeToRemove) {
			found = 1;
			break;
		}
	}
	if (found == 0)
		return CDF_STATUS_E_INVAL;

	list_del(pNodeToRemove);
	pList->count--;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_peek_front() - peek front node from list
 * @pList: Pointer to list
 * @ppNode: Double pointer to store peeked node pointer
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_peek_front(cdf_list_t *pList, cdf_list_node_t **ppNode)
{
	struct list_head *listptr;
	if (list_empty(&pList->anchor))
		return CDF_STATUS_E_EMPTY;

	listptr = pList->anchor.next;
	*ppNode = listptr;
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_peek_next() - peek next node of input node in the list
 * @pList: Pointer to list
 * @pNode: Pointer to input node
 * @ppNode: Double pointer to store peeked node pointer
 *
 * Return: CDF status
 */
CDF_STATUS cdf_list_peek_next(cdf_list_t *pList, cdf_list_node_t *pNode,
			      cdf_list_node_t **ppNode)
{
	struct list_head *listptr;
	int found = 0;
	cdf_list_node_t *tmp;

	if ((pList == NULL) || (pNode == NULL) || (ppNode == NULL))
		return CDF_STATUS_E_FAULT;

	if (list_empty(&pList->anchor))
		return CDF_STATUS_E_EMPTY;

	/* verify that pNode is indeed part of list pList */
	list_for_each(tmp, &pList->anchor) {
		if (tmp == pNode) {
			found = 1;
			break;
		}
	}

	if (found == 0)
		return CDF_STATUS_E_INVAL;

	listptr = pNode->next;
	if (listptr == &pList->anchor)
		return CDF_STATUS_E_EMPTY;

	*ppNode = listptr;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_list_empty() - check if the list is empty
 * @list: pointer to the list
 *
 * Return: true if the list is empty and false otherwise.
 */
bool cdf_list_empty(cdf_list_t *list)
{
	return list_empty(&list->anchor);
}
