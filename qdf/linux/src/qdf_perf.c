/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_perf
 * This file provides OS dependent perf API's.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#include <qdf_perf.h>
#include <qdf_module.h>
#ifdef QCA_PERF_PROFILING

qdf_perf_entry_t     perf_root = {{0, 0} };

/**
 * qdf_perfmod_init() - Module init
 *
 * return: int
 */
int
qdf_perfmod_init(void)
{
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "Perf Debug Module Init");
	INIT_LIST_HEAD(&perf_root.list);
	INIT_LIST_HEAD(&perf_root.child);
	perf_root.proc = proc_mkdir(PROCFS_PERF_DIRNAME, 0);
	return 0;
}
qdf_export_symbol(qdf_perfmod_init);

/**
 * qdf_perfmod_exit() - Module exit
 *
 * Return: none
 */
void
qdf_perfmod_exit(void)
{
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "Perf Debug Module Exit");
	remove_proc_entry(PROCFS_PERF_DIRNAME, 0);
}
qdf_export_symbol(qdf_perfmod_exit);

/**
 * __qdf_perf_init() - Create the perf entry
 * @parent: parent perf id
 * @id_name: name of perf id
 * @type: type of perf counter
 *
 * return: perf id
 */
qdf_perf_id_t
__qdf_perf_init(qdf_perf_id_t parent, uint8_t *id_name,
		qdf_perf_cntr_t type)
{
	qdf_perf_entry_t    *entry  = NULL;
	qdf_perf_entry_t    *pentry = PERF_ENTRY(parent);

	if (type >= CNTR_LAST) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s:%s Invalid perf-type", __FILE__, __func__);
		goto done;
	}

	if (!pentry)
		pentry = &perf_root;
	entry = kmalloc(sizeof(struct qdf_perf_entry), GFP_ATOMIC);

	if (!entry) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  " Out of Memory,:%s", __func__);
		return NULL;
	}

	memset(entry, 0, sizeof(struct qdf_perf_entry));

	INIT_LIST_HEAD(&entry->list);
	INIT_LIST_HEAD(&entry->child);

	spin_lock_init(&entry->lock_irq);

	list_add_tail(&entry->list, &pentry->child);

	entry->name = id_name;
	entry->type = type;

	if (type == CNTR_GROUP) {
		entry->proc = proc_mkdir(id_name, pentry->proc);
		goto done;
	}

	entry->parent   = pentry;
	entry->proc     = create_proc_entry(id_name, S_IFREG|S_IRUGO|S_IWUSR,
					pentry->proc);
	entry->proc->data       = entry;
	entry->proc->read_proc  = api_tbl[type].proc_read;
	entry->proc->write_proc = api_tbl[type].proc_write;

	/*
	 * Initialize the Event with default values
	 */
	api_tbl[type].init(entry, api_tbl[type].def_val);

done:
	return entry;
}
qdf_export_symbol(__qdf_perf_init);

/**
 * __qdf_perf_destroy - Destroy the perf entry
 * @id: pointer to qdf_perf_id_t
 *
 * @return: bool
 */
bool __qdf_perf_destroy(qdf_perf_id_t  id)
{
	qdf_perf_entry_t     *entry  = PERF_ENTRY(id),
		*parent = entry->parent;

	if (!list_empty(&entry->child)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Child's are alive, Can't delete");
		return A_FALSE;
	}

	remove_proc_entry(entry->name, parent->proc);

	list_del(&entry->list);

	vfree(entry);

	return true;
}
qdf_export_symbol(__qdf_perf_destroy);

/**
 * __qdf_perf_start - Start the sampling
 * @id: Instance of qdf_perf_id_t
 *
 * Returns: none
 */
void __qdf_perf_start(qdf_perf_id_t id)
{
	qdf_perf_entry_t *entry = PERF_ENTRY(id);

	api_tbl[entry->type].sample(entry, 0);
}
qdf_export_symbol(__qdf_perf_start);

/**
 * __qdf_perf_end - Stop sampling
 * @id: Instance of qdf_perf_id_t
 *
 * Returns: none
 */
void __qdf_perf_end(qdf_perf_id_t id)
{
	qdf_perf_entry_t *entry = PERF_ENTRY(id);

	api_tbl[entry->type].sample(entry, 1);
}
qdf_export_symbol(__qdf_perf_end);

#endif /* QCA_PERF_PROFILING */
