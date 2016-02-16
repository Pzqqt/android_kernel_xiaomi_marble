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

#if !defined(__CDF_LIST_H)
#define __CDF_LIST_H

/**
 *  DOC: cdf_list.h
 *
 *  Connectivity driver framework (CDF) list APIs
 *
 *  Definitions for CDF Linked Lists API
 *
 *  Lists are implemented as a doubly linked list. An item in a list can
 *  be of any type as long as the datatype contains a field of type
 *  cdf_link_t.
 *
 *  In general, a list is a doubly linked list of items with a pointer
 *  to the front of the list and a pointer to the end of the list.  The
 *  list items contain a forward and back link.
 *
 *  CDF linked list APIs are NOT thread safe so make sure to use appropriate
 *  locking mechanisms to assure operations on the list are thread safe.
 */

/* Include Files */
#include <cdf_types.h>
#include <cdf_status.h>
#include <qdf_status.h>
#include <cdf_trace.h>
#include <linux/list.h>

/* Preprocessor definitions and constants */

/* Type declarations */

typedef struct list_head cdf_list_node_t;

typedef struct cdf_list_s {
	cdf_list_node_t anchor;
	uint32_t count;
	uint32_t max_size;
} cdf_list_t;

/* Function declarations */

CDF_INLINE_FN void cdf_list_init(cdf_list_t *p_list, uint32_t max_size)
{
	INIT_LIST_HEAD(&p_list->anchor);
	p_list->count = 0;
	p_list->max_size = max_size;
}

CDF_INLINE_FN void cdf_list_destroy(cdf_list_t *p_list)
{
	if (p_list->count != 0) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: list length not equal to zero", __func__);
		CDF_ASSERT(0);
	}
}

CDF_INLINE_FN void cdf_list_size(cdf_list_t *p_list, uint32_t *p_size)
{
	*p_size = p_list->count;
}

CDF_STATUS cdf_list_insert_front(cdf_list_t *p_list, cdf_list_node_t *p_node);

CDF_STATUS cdf_list_insert_back(cdf_list_t *p_list, cdf_list_node_t *p_node);

CDF_STATUS cdf_list_insert_back_size(cdf_list_t *p_list,
				     cdf_list_node_t *p_node, uint32_t *p_size);

CDF_STATUS cdf_list_remove_front(cdf_list_t *p_list, cdf_list_node_t **pp_node);

CDF_STATUS cdf_list_remove_back(cdf_list_t *p_list, cdf_list_node_t **pp_node);

CDF_STATUS cdf_list_peek_front(cdf_list_t *p_list, cdf_list_node_t **pp_node);

CDF_STATUS cdf_list_peek_next(cdf_list_t *p_list, cdf_list_node_t *p_node,
			      cdf_list_node_t **pp_node);

CDF_STATUS cdf_list_remove_node(cdf_list_t *p_list,
				cdf_list_node_t *p_node_to_remove);

bool cdf_list_empty(cdf_list_t *list);

#endif /* __CDF_LIST_H */
