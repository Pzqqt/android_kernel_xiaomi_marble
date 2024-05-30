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
 * DOC: qld_priv.h
 * QLD: This file provies Private functions for qld
 */

#ifndef _QLD_PRIV_H_
#define _QLD_PRIV_H_

#include <qdf_lock.h>
#include <qdf_list.h>
#include <qld_api.h>

#define qld_alert(format, args...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_QLD, format, ## args)

#define qld_err(format, args...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_QLD, format, ## args)

#define qld_warn(format, args...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_QLD, format, ## args)

#define qld_info(format, args...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_QLD, format, ## args)

#define qld_debug(format, args...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_QLD, format, ## args)

/**
 * struct qld_list_handle - Top level qld structure
 * @qld_lock:      Spinlock for structure
 * @qld_list:      linked list for linking
 * @qld_max_list:  maximum list size
 */
struct qld_list_handle {
	qdf_spinlock_t qld_lock;
	qdf_list_t qld_list;
	uint32_t qld_max_list;
};

/**
 * struct qld_node - qld node
 * @node:          single node of linked list
 * @entry:         single qld_entry in list
 */
struct qld_node {
	qdf_list_node_t node;
	struct qld_entry entry;
};

#endif /*_QLD_PRIV_H_*/
