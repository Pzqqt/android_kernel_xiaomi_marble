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
 * DOC: i_qdf_list.h
 * This file provides OS dependent list API's.
 */

#if !defined(__I_QDF_LIST_H)
#define __I_QDF_LIST_H

#include <linux/list.h>

/* Type declarations */
typedef struct list_head __qdf_list_node_t;

/* Preprocessor definitions and constants */

typedef struct qdf_list_s {
	__qdf_list_node_t anchor;
	uint32_t count;
	uint32_t max_size;
} __qdf_list_t;

/**
 * __qdf_list_create() - Initialize list head
 * @list: object of list
 * @max_size: max size of the list
 * Return: none
 */
static inline void __qdf_list_create(__qdf_list_t *list, uint32_t max_size)
{
	INIT_LIST_HEAD(&list->anchor);
	list->count = 0;
	list->max_size = max_size;
}

bool qdf_list_has_node(__qdf_list_t *list, __qdf_list_node_t *node);
#endif
