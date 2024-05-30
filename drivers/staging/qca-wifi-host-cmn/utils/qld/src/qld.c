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
 * DOC: qld
 * QLD: main file of QCA Live Dump (QLD)
 */

#include "qld_priv.h"
#include "qld_api.h"
#include "qdf_module.h"

/* Handle for qld structure */
static struct qld_list_handle *qld_handle;

bool is_qld_enable(void)
{
	if (!qld_handle)
		return false;

	return true;
}

qdf_export_symbol(is_qld_enable);

int qld_list_init(uint32_t max_list)
{
	if (!max_list)
		return -EINVAL;

	qld_handle = qdf_mem_malloc(sizeof(*qld_handle));

	if (!qld_handle)
		return -ENOMEM;

	qdf_spinlock_create(&qld_handle->qld_lock);
	qld_handle->qld_max_list = max_list;
	qdf_list_create(&qld_handle->qld_list, qld_handle->qld_max_list);
	qld_debug("LIST init with max size of %u", qld_handle->qld_max_list);
	return 0;
}

qdf_export_symbol(qld_list_init);

int qld_list_deinit(void)
{
	if (!qld_handle) {
		qld_err("Handle NULL");
		return -EINVAL;
	}
	/* Delete the list */
	qld_list_delete();
	qdf_list_destroy(&qld_handle->qld_list);
	qdf_spinlock_destroy(&qld_handle->qld_lock);
	qdf_mem_free(qld_handle);
	qld_handle = NULL;
	qld_debug("LIST De-initialized");
	return 0;
}

qdf_export_symbol(qld_list_deinit);

int qld_list_delete(void)
{
	struct qld_node *qld;
	qdf_list_node_t *node = NULL;
	qdf_list_t *list;

	if (!qld_handle) {
		qld_err("Handle NULL");
		return -EINVAL;
	}
	list = &qld_handle->qld_list;
	qdf_spinlock_acquire(&qld_handle->qld_lock);
	/* Check and remove the elements of list */
	while (qdf_list_remove_front(list, &node) == QDF_STATUS_SUCCESS) {
		qld = qdf_container_of(node, struct qld_node, node);
		qdf_mem_free(qld);
	}
	qdf_spinlock_release(&qld_handle->qld_lock);
	qld_debug("LIST Emptied");
	return 0;
}

qdf_export_symbol(qld_list_delete);

int qld_register(void *addr, size_t size, char *name)
{
	struct qld_node *qld;
	uint32_t list_count = 0;

	if (!qld_handle || !addr) {
		qld_err("Handle or address is NULL");
		return -EINVAL;
	}

	if ((qld_get_list_count(&list_count) != 0)) {
		qdf_err("QLD: Invalid list count");
		return -EINVAL;
	}
	if (list_count >= qld_handle->qld_max_list) {
		qld_err("List full,reg failed.Increase list size");
		return -EINVAL;
	}
	/* Check if data is already registered */
	qdf_spinlock_acquire(&qld_handle->qld_lock);
	qdf_list_for_each(&qld_handle->qld_list, qld, node) {
		if (qld->entry.addr == (uintptr_t)addr) {
			qld_err("%s already registered", qld->entry.name);
			qdf_spinlock_release(&qld_handle->qld_lock);
			return -EINVAL;
		}
	}
	qdf_spinlock_release(&qld_handle->qld_lock);
	qld = qdf_mem_malloc(sizeof(*qld));
	if (!qld)
		return -ENOMEM;

	qld_debug("Insert addr=%pK size=%zu name=%s", (void *)addr, size, name);
	qdf_spinlock_acquire(&qld_handle->qld_lock);
	qld->entry.addr =  (uintptr_t)addr;
	qld->entry.size = size;
	qdf_snprintf(qld->entry.name, sizeof(qld->entry.name), "%s", name);
	qdf_list_insert_front(&qld_handle->qld_list, &qld->node);
	qdf_spinlock_release(&qld_handle->qld_lock);
	return 0;
}

qdf_export_symbol(qld_register);

int qld_unregister(void *addr)
{
	struct qld_node *qld  = NULL;

	if (!qld_handle || !addr) {
		qld_err("Handle or address is NULL");
		return -EINVAL;
	}

	qdf_spinlock_acquire(&qld_handle->qld_lock);
	qdf_list_for_each(&qld_handle->qld_list, qld, node) {
		if (qld->entry.addr == (uintptr_t)addr)
			break;
	}
	qdf_list_remove_node(&qld_handle->qld_list, &qld->node);
	qld_debug("Delete name=%s, size=%zu", qld->entry.name, qld->entry.size);
	qdf_mem_free(qld);
	qdf_spinlock_release(&qld_handle->qld_lock);
	return 0;
}

qdf_export_symbol(qld_unregister);

int qld_iterate_list(qld_iter_func gen_table, void *qld_req)
{
	struct qld_node *qld  = NULL;

	if (!qld_handle)
		return -EINVAL;

	if (!qld_req || !gen_table) {
		qld_err("req buffer or func is NULL %s", __func__);
		return -EINVAL;
	}
	qdf_spinlock_acquire(&qld_handle->qld_lock);
	qdf_list_for_each(&qld_handle->qld_list, qld, node) {
		(gen_table)(qld_req, &qld->entry);
	}
	qdf_spinlock_release(&qld_handle->qld_lock);
	return 0;
}

qdf_export_symbol(qld_iterate_list);

int qld_get_list_count(uint32_t *list_count)
{
	if (!qld_handle) {
		qld_err("Handle NULL");
		return -EINVAL;
	}
	*list_count = qld_handle->qld_list.count;
	return 0;
}

qdf_export_symbol(qld_get_list_count);
