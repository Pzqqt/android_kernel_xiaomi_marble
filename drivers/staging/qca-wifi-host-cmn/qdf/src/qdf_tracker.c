/*
 * Copyright (c) 2019, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "qdf_debug_domain.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_module.h"
#include "qdf_ptr_hash.h"
#include "qdf_status.h"
#include "qdf_str.h"
#include "qdf_tracker.h"
#include "qdf_types.h"

#ifdef CONFIG_LEAK_DETECTION
struct qdf_tracker_node {
	struct qdf_ptr_hash_entry entry;
	enum qdf_debug_domain domain;
	char func[QDF_TRACKER_FUNC_SIZE];
	uint32_t line;
};

void qdf_tracker_init(struct qdf_tracker *tracker)
{
	qdf_spinlock_create(&tracker->lock);
	qdf_ptr_hash_init(tracker->ht);
}
qdf_export_symbol(qdf_tracker_init);

void qdf_tracker_deinit(struct qdf_tracker *tracker)
{
	qdf_tracker_check_for_leaks(tracker);

	qdf_spin_lock_bh(&tracker->lock);
	QDF_BUG(qdf_ptr_hash_empty(tracker->ht));
	qdf_spin_unlock_bh(&tracker->lock);

	qdf_ptr_hash_deinit(tracker->ht);
	qdf_spinlock_destroy(&tracker->lock);
}
qdf_export_symbol(qdf_tracker_deinit);

static inline void qdf_tracker_print_break(void)
{
	qdf_nofl_alert("-----------------------------------------------------");
}

static uint32_t qdf_tracker_leaks_print(struct qdf_tracker *tracker,
					enum qdf_debug_domain domain)
{
	struct qdf_ptr_hash_bucket *bucket;
	struct qdf_tracker_node *node;
	bool print_header = true;
	uint32_t count = 0;

	qdf_ptr_hash_for_each(tracker->ht, bucket, node, entry) {
		if (node->domain != domain)
			continue;

		if (print_header) {
			print_header = false;
			qdf_nofl_alert("%s detected in %s domain!",
				       tracker->leak_title,
				       qdf_debug_domain_name(domain));
			qdf_tracker_print_break();
		}

		count++;
		qdf_nofl_alert("0x%lx @ %s:%u", node->entry.key,
			       node->func, node->line);
	}

	if (count)
		qdf_tracker_print_break();

	return count;
}

void qdf_tracker_check_for_leaks(struct qdf_tracker *tracker)
{
	enum qdf_debug_domain domain = qdf_debug_domain_get();
	uint32_t leaks;

	qdf_spin_lock_bh(&tracker->lock);
	leaks = qdf_tracker_leaks_print(tracker, domain);
	if (leaks)
		QDF_DEBUG_PANIC("%u fatal %s detected in %s domain!",
				leaks, tracker->leak_title,
				qdf_debug_domain_name(domain));
	qdf_spin_unlock_bh(&tracker->lock);
}
qdf_export_symbol(qdf_tracker_check_for_leaks);

QDF_STATUS qdf_tracker_track(struct qdf_tracker *tracker, void *ptr,
			     const char *func, uint32_t line)
{
	struct qdf_tracker_node *node;

	QDF_BUG(ptr);
	if (!ptr)
		return QDF_STATUS_E_INVAL;

	qdf_spin_lock_bh(&tracker->lock);
	node = qdf_ptr_hash_get(tracker->ht, ptr, node, entry);
	if (node)
		QDF_DEBUG_PANIC("Double %s (via %s:%u); last %s from %s:%u",
				tracker->track_title, func, line,
				tracker->track_title, node->func, node->line);
	qdf_spin_unlock_bh(&tracker->lock);

	if (node)
		return QDF_STATUS_E_ALREADY;

	node = qdf_mem_malloc(sizeof(*node));
	if (!node)
		return QDF_STATUS_E_NOMEM;

	node->domain = qdf_debug_domain_get();
	qdf_str_lcopy(node->func, func, QDF_TRACKER_FUNC_SIZE);
	node->line = line;

	qdf_spin_lock_bh(&tracker->lock);
	qdf_ptr_hash_add(tracker->ht, ptr, node, entry);
	qdf_spin_unlock_bh(&tracker->lock);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_tracker_track);

void qdf_tracker_untrack(struct qdf_tracker *tracker, void *ptr,
			 const char *func, uint32_t line)
{
	enum qdf_debug_domain domain = qdf_debug_domain_get();
	struct qdf_tracker_node *node;

	QDF_BUG(ptr);
	if (!ptr)
		return;

	qdf_spin_lock_bh(&tracker->lock);
	node = qdf_ptr_hash_remove(tracker->ht, ptr, node, entry);
	if (!node)
		QDF_DEBUG_PANIC("Double %s (via %s:%u)",
				tracker->untrack_title, func, line);
	else if (node->domain != domain)
		QDF_DEBUG_PANIC("%s domain mismatch; tracked:%s, %s:%u; untracked:%s , %s:%u",
				tracker->untrack_title,
				qdf_debug_domain_name(node->domain),
				node->func, node->line,
				qdf_debug_domain_name(domain),
				func, line);
	qdf_spin_unlock_bh(&tracker->lock);

	if (node)
		qdf_mem_free(node);
}
qdf_export_symbol(qdf_tracker_untrack);

bool qdf_tracker_lookup(struct qdf_tracker *tracker, void *ptr,
			char (*out_func)[QDF_TRACKER_FUNC_SIZE],
			uint32_t *out_line)
{
	struct qdf_tracker_node *node;

	qdf_spin_lock_bh(&tracker->lock);
	node = qdf_ptr_hash_get(tracker->ht, ptr, node, entry);
	if (node) {
		qdf_str_lcopy((char *)out_func, node->func,
			      QDF_TRACKER_FUNC_SIZE);
		*out_line = node->line;
	}
	qdf_spin_unlock_bh(&tracker->lock);

	return !!node;
}
#endif
