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
 * DOC: qdf_periodic_work.h
 * A simple, periodic work type for repeatedly executing a callback with a
 * certain frequency.
 */

#ifndef __QDF_PERIODIC_WORK_H
#define __QDF_PERIODIC_WORK_H

#include "i_qdf_periodic_work.h"
#include "qdf_status.h"
#include "qdf_types.h"

typedef void (*qdf_periodic_work_cb)(void *context);

/**
 * struct qdf_periodic_work - a defered work type which executes a callback
 *	periodically until stopped
 * @dwork: OS-specific delayed work
 * @callback: the callback to be executed periodically
 * @context: the context to pass to the callback
 * @msec: the delay between executions in milliseconds
 */
struct qdf_periodic_work {
	struct __qdf_opaque_delayed_work dwork;
	qdf_periodic_work_cb callback;
	void *context;
	uint32_t msec;
};

/**
 * qdf_periodic_work_create() - initialized a periodic work @pwork
 * @pwork: the periodic work to initialize
 * @callback: the callback to be executed periodically
 * @context: the context to pass to the callback
 *
 * Return: QDF_STATUS
 */
#define qdf_periodic_work_create(pwork, callback, context) \
	__qdf_periodic_work_create(pwork, callback, context, __func__, __LINE__)

qdf_must_check QDF_STATUS
__qdf_periodic_work_create(struct qdf_periodic_work *pwork,
			   qdf_periodic_work_cb callback, void *context,
			   const char *func, uint32_t line);

/**
 * qdf_periodic_work_destroy() - deinitialize a periodic work @pwork
 * @pwork: the periodic work to de-initialize
 *
 * Return: None
 */
#define qdf_periodic_work_destroy(pwork) \
	__qdf_periodic_work_destroy(pwork, __func__, __LINE__)

void __qdf_periodic_work_destroy(struct qdf_periodic_work *pwork,
				 const char *func, uint32_t line);

/**
 * qdf_periodic_work_start() - begin periodic execution of @pwork callback
 * @pwork: the periodic work to start
 * @msec: the delay between executions in milliseconds
 *
 * Return: true if started successfully
 */
bool qdf_periodic_work_start(struct qdf_periodic_work *pwork, uint32_t msec);

/**
 * qdf_periodic_work_stop_async() - Asynchronously stop execution of @pwork
 * @pwork: the periodic work to stop
 *
 * When this returns, @pwork is guaranteed to not be queued, *but* its callback
 * may still be executing.
 *
 * This is safe to call from the @pwork callback.
 *
 * Return: true if @pwork was previously started
 */
bool qdf_periodic_work_stop_async(struct qdf_periodic_work *pwork);

/**
 * qdf_periodic_work_stop_sync() - Synchronously stop execution of @pwork
 * @pwork: the periodic work to stop
 *
 * When this returns, @pwork is guaranteed to not be queued, and its callback
 * not executing.
 *
 * This will deadlock if called from the @pwork callback.
 *
 * Return: true if @pwork was previously started
 */
bool qdf_periodic_work_stop_sync(struct qdf_periodic_work *pwork);

#ifdef WLAN_PERIODIC_WORK_DEBUG
/**
 * qdf_periodic_work_check_for_leaks() - assert no periodic work leaks
 *
 * Return: None
 */
void qdf_periodic_work_check_for_leaks(void);

/**
 * qdf_periodic_work_feature_init() - global init logic for periodic work
 *
 * Return: None
 */
void qdf_periodic_work_feature_init(void);

/**
 * qdf_periodic_work_feature_deinit() - global de-init logic for periodic work
 *
 * Return: None
 */
void qdf_periodic_work_feature_deinit(void);
#else
static inline void qdf_periodic_work_check_for_leaks(void) { }
static inline void qdf_periodic_work_feature_init(void) { }
static inline void qdf_periodic_work_feature_deinit(void) { }
#endif /* WLAN_PERIODIC_WORK_DEBUG */

#endif /* __QDF_PERIODIC_WORK_H */

