/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_cpuhp (CPU hotplug)
 * QCA driver framework (QDF) CPU hotplug APIs
 */

#include "qdf_cpuhp.h"
#include "i_qdf_cpuhp.h"
#include "qdf_list.h"
#include "qdf_lock.h"

static qdf_mutex_t qdf_cpuhp_lock;
static qdf_list_t qdf_cpuhp_handlers;

struct qdf_cpuhp_handler {
	qdf_list_node_t node;
	void *context;
	qdf_cpuhp_callback up_callback;
	qdf_cpuhp_callback down_callback;
};

static void qdf_cpuhp_on_up(uint32_t cpu)
{
	QDF_STATUS status;
	qdf_list_node_t *node;

	qdf_mutex_acquire(&qdf_cpuhp_lock);

	status = qdf_list_peek_front(&qdf_cpuhp_handlers, &node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		struct qdf_cpuhp_handler *handler =
			qdf_container_of(node, struct qdf_cpuhp_handler, node);
		if (handler->up_callback)
			handler->up_callback(handler->context, cpu);

		status = qdf_list_peek_next(&qdf_cpuhp_handlers, node, &node);
	}

	qdf_mutex_release(&qdf_cpuhp_lock);
}

static void qdf_cpuhp_on_down(uint32_t cpu)
{
	QDF_STATUS status;
	qdf_list_node_t *node;

	qdf_mutex_acquire(&qdf_cpuhp_lock);

	status = qdf_list_peek_front(&qdf_cpuhp_handlers, &node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		struct qdf_cpuhp_handler *handler =
			qdf_container_of(node, struct qdf_cpuhp_handler, node);
		if (handler->down_callback)
			handler->down_callback(handler->context, cpu);

		status = qdf_list_peek_next(&qdf_cpuhp_handlers, node, &node);
	}

	qdf_mutex_release(&qdf_cpuhp_lock);
}

QDF_STATUS qdf_cpuhp_init(void)
{
	QDF_STATUS status;

	status = qdf_mutex_create(&qdf_cpuhp_lock);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	qdf_list_create(&qdf_cpuhp_handlers, 0);

	__qdf_cpuhp_os_init(qdf_cpuhp_on_up, qdf_cpuhp_on_down);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_cpuhp_deinit(void)
{
	__qdf_cpuhp_os_deinit();
	qdf_list_destroy(&qdf_cpuhp_handlers);
	return qdf_mutex_destroy(&qdf_cpuhp_lock);
}

QDF_STATUS qdf_cpuhp_register(struct qdf_cpuhp_handler **out_handler,
			      void *context,
			      qdf_cpuhp_callback up_callback,
			      qdf_cpuhp_callback down_callback)
{
	QDF_STATUS status;
	struct qdf_cpuhp_handler *handler;

	*out_handler = NULL;

	handler = qdf_mem_malloc(sizeof(*handler));
	if (!handler)
		return QDF_STATUS_E_NOMEM;

	handler->context = context;
	handler->up_callback = up_callback;
	handler->down_callback = down_callback;

	status = qdf_mutex_acquire(&qdf_cpuhp_lock);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_handler;

	status = qdf_list_insert_back(&qdf_cpuhp_handlers, &handler->node);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_lock;

	/* this can fail, but there isn't a good way to recover... */
	qdf_mutex_release(&qdf_cpuhp_lock);

	*out_handler = handler;

	return QDF_STATUS_SUCCESS;

release_lock:
	qdf_mutex_release(&qdf_cpuhp_lock);

free_handler:
	qdf_mem_free(handler);

	return status;
}

void qdf_cpuhp_unregister(struct qdf_cpuhp_handler **out_handler)
{
	struct qdf_cpuhp_handler *handler = *out_handler;

	QDF_BUG(handler);
	if (!handler)
		return;

	qdf_mutex_acquire(&qdf_cpuhp_lock);
	qdf_list_remove_node(&qdf_cpuhp_handlers, &handler->node);
	qdf_mutex_release(&qdf_cpuhp_lock);

	qdf_mem_free(handler);
	*out_handler = NULL;
}

