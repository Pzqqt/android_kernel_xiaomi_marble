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
 * DOC:  qdf_threads
 * QCA driver framework (QDF) thread related APIs
 */

#if !defined(__QDF_THREADS_H)
#define __QDF_THREADS_H

#include <qdf_types.h>
#include <qdf_util.h>

/* Function declarations and documenation */

void qdf_sleep(uint32_t ms_interval);

void qdf_sleep_us(uint32_t us_interval);

void qdf_busy_wait(uint32_t us_interval);

/**
 * qdf_set_user_nice() - set thread's nice value
 * @thread: pointer to thread
 * @nice: nice value
 *
 * Return: none
 */
void qdf_set_user_nice(qdf_thread_t *thread, long nice);

/**
 * qdf_create_thread() - create a kernel thread
 * @thread: pointer to thread
 * @nice: nice value
 *
 * Return: pointer to created kernel thread
 */
qdf_thread_t *qdf_create_thread(int (*thread_handler)(void *data), void *data,
				const char thread_name[]);

/**
 * qdf_wake_up_process() - wake up given thread
 * @thread: pointer to thread which needs to be woken up
 *
 * Return: none
 */
int qdf_wake_up_process(qdf_thread_t *thread);

#endif /* __QDF_THREADS_H */
