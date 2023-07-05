/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_delayed_work.h
 * A simple, delayed work type for executing a callback after some delay.
 */

#ifndef __QDF_DELAYED_WORK_H
#define __QDF_DELAYED_WORK_H

#include "i_qdf_delayed_work.h"
#include "qdf_status.h"
#include "qdf_types.h"

typedef void (*qdf_delayed_work_cb)(void *context);

/**
 * struct qdf_delayed_work - a defered work type which executes a callback after
 *	some delay
 * @dwork: OS-specific delayed work
 * @callback: the callback to be executed
 * @context: the context to pass to the callback
 */
struct qdf_delayed_work {
	struct __qdf_opaque_delayed_work dwork;
	qdf_delayed_work_cb callback;
	void *context;
};

/**
 * qdf_delayed_work_create() - initialized a delayed work @dwork
 * @dwork: the delayed work to initialize
 * @callback: the callback to be executed
 * @context: the context to pass to the callback
 *
 * Return: QDF_STATUS
 */
#define qdf_delayed_work_create(dwork, callback, context) \
	__qdf_delayed_work_create(dwork, callback, context, __func__, __LINE__)

qdf_must_check QDF_STATUS
__qdf_delayed_work_create(struct qdf_delayed_work *dwork,
			  qdf_delayed_work_cb callback, void *context,
			  const char *func, uint32_t line);

/**
 * qdf_delayed_work_destroy() - deinitialize a delayed work @dwork
 * @dwork: the delayed work to de-initialize
 *
 * Return: None
 */
#define qdf_delayed_work_destroy(dwork) \
	__qdf_delayed_work_destroy(dwork, __func__, __LINE__)

void __qdf_delayed_work_destroy(struct qdf_delayed_work *dwork,
				const char *func, uint32_t line);

/**
 * qdf_delayed_work_start() - schedule execution of @dwork callback
 * @dwork: the delayed work to start
 * @msec: the delay before execution in milliseconds
 *
 * Return: true if started successfully
 */
#define qdf_delayed_work_start(dwork, msec) \
	__qdf_delayed_work_start(dwork, msec)

bool __qdf_delayed_work_start(struct qdf_delayed_work *dwork, uint32_t msec);

/**
 * qdf_delayed_work_stop_sync() - Synchronously stop execution of @dwork
 * @dwork: the delayed work to stop
 *
 * When this returns, @dwork is guaranteed to not be queued, and its callback
 * not executing.
 *
 * Return: true if @dwork was queued or running
 */
#define qdf_delayed_work_stop_sync(dwork) \
	__qdf_delayed_work_stop_sync(dwork)

bool __qdf_delayed_work_stop_sync(struct qdf_delayed_work *dwork);

/**
 * qdf_delayed_work_stop() - Stop execution of @dwork
 * @dwork: the delayed work to stop
 *
 * Kill off a pending delayed_work
 *
 * Return: true if dwork was pending and canceled
 */
#define qdf_delayed_work_stop(dwork) \
	__qdf_delayed_work_stop(dwork)

bool __qdf_delayed_work_stop(struct qdf_delayed_work *dwork);

#ifdef WLAN_DELAYED_WORK_DEBUG
/**
 * qdf_delayed_work_check_for_leaks() - assert no delayed work leaks
 *
 * Return: None
 */
void qdf_delayed_work_check_for_leaks(void);

/**
 * qdf_delayed_work_feature_init() - global init logic for delayed work
 *
 * Return: None
 */
void qdf_delayed_work_feature_init(void);

/**
 * qdf_delayed_work_feature_deinit() - global de-init logic for delayed work
 *
 * Return: None
 */
void qdf_delayed_work_feature_deinit(void);
#else
static inline void qdf_delayed_work_check_for_leaks(void) { }
static inline void qdf_delayed_work_feature_init(void) { }
static inline void qdf_delayed_work_feature_deinit(void) { }
#endif /* WLAN_DELAYED_WORK_DEBUG */

#endif /* __QDF_DELAYED_WORK_H */

