/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __QDF_TRACKER_H
#define __QDF_TRACKER_H

#include "qdf_lock.h"
#include "qdf_ptr_hash.h"
#include "qdf_status.h"
#include "qdf_types.h"

#define QDF_TRACKER_FUNC_SIZE 48

/**
 * struct qdf_tracker - a generic type for tracking resources
 * @leak_title: the string title to use when logging leaks
 * @track_title: the string title to use when logging double tracking issues
 * @untrack_title: the string title to use when logging double untracking issues
 * @lock: lock for simultaneous access to @ht
 * @ht: the hashtable used for storing tracking information
 */
struct qdf_tracker {
	const char *leak_title;
	const char *track_title;
	const char *untrack_title;
	struct qdf_spinlock lock;
	struct qdf_ptr_hash *ht;
};

/**
 * qdf_tracker_declare() - statically declare a qdf_tacker instance
 * @name: C identifier to use for the new qdf_tracker
 * @bits: the number of bits to use for hashing the resource pointers
 * @leak_title: the string title to use when logging leaks
 * @track_title: the string title to use when logging double tracking issues
 * @untrack_title: the string title to use when logging double untracking issues
 */
#define qdf_tracker_declare(name, bits, _leak_title, \
			    _track_title, _untrack_title) \
qdf_ptr_hash_declare(name ## _ht, bits); \
struct qdf_tracker name = { \
	.leak_title = _leak_title, \
	.track_title = _track_title, \
	.untrack_title = _untrack_title, \
	.ht = qdf_ptr_hash_ptr(name ## _ht), \
}

#ifdef CONFIG_LEAK_DETECTION
/**
 * qdf_tracker_init() - initialize a qdf_tracker
 * @tracker: the qdf_tracker to initialize
 *
 * Return: None
 */
void qdf_tracker_init(struct qdf_tracker *tracker);

/**
 * qdf_tracker_deinit() - de-initialize a qdf_tracker
 * @tracker: the qdf_tracker to de-initialize
 *
 * Return: None
 */
void qdf_tracker_deinit(struct qdf_tracker *tracker);

/**
 * qdf_tracker_track() - track a resource with @tracker
 * @tracker: the qdf_tracker to track with
 * @ptr: an opaque pointer to the resource to track
 * @func: name of the caller function operating on @ptr
 * @line: line number of the call site operating on @ptr
 *
 * Return: QDF_STATUS
 */
qdf_must_check QDF_STATUS
qdf_tracker_track(struct qdf_tracker *tracker, void *ptr,
		  const char *func, uint32_t line);

/**
 * qdf_tracker_untrack() - untrack a resource with @tracker
 * @tracker: the qdf_tracker used to track @ptr
 * @ptr: an opaque pointer to the resource to untrack
 * @func: name of the caller function operating on @ptr
 * @line: line number of the call site operating on @ptr
 *
 * Return: None
 */
void qdf_tracker_untrack(struct qdf_tracker *tracker, void *ptr,
			 const char *func, uint32_t line);

/**
 * qdf_tracker_check_for_leaks() - assert @tracker has no tracked resources
 *	for the current debug domain
 * @tracker: the qdf_tracker to check
 *
 * Return: None
 */
void qdf_tracker_check_for_leaks(struct qdf_tracker *tracker);

/**
 * qdf_tracker_lookup() - query tracking information for @ptr
 * @tracker: the qdf_tracker to check
 * @ptr: the opaque pointer of the resource to lookup
 * @out_func: function name provided when @ptr was tracked, populated on success
 * @out_line: line number provided when @ptr was tracked, populated on success
 *
 * Note: @out_func is assumed to be sizeof(QDF_TRACKER_FUNC_SIZE).
 *
 * Return: true if @tracker is tracking @ptr
 */
qdf_must_check bool
qdf_tracker_lookup(struct qdf_tracker *tracker, void *ptr,
		   char (*out_func)[QDF_TRACKER_FUNC_SIZE],
		   uint32_t *out_line);
#else
static inline
void qdf_tracker_init(struct qdf_tracker *tracker)
{
}

static inline
void qdf_tracker_deinit(struct qdf_tracker *tracker)
{
}

static inline qdf_must_check QDF_STATUS
qdf_tracker_track(struct qdf_tracker *tracker, void *ptr,
		  const char *func, uint32_t line)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void qdf_tracker_untrack(struct qdf_tracker *tracker, void *ptr,
			 const char *func, uint32_t line)
{
}

static inline
void qdf_tracker_check_for_leaks(struct qdf_tracker *tracker)
{
}

static inline qdf_must_check bool
qdf_tracker_lookup(struct qdf_tracker *tracker, void *ptr,
		   char (*out_func)[QDF_TRACKER_FUNC_SIZE],
		   uint32_t *out_line)
{
	return false;
}
#endif
#endif /* __QDF_TRACKER_H */

