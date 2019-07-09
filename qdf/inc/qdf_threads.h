/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC:  qdf_threads
 * QCA driver framework (QDF) thread related APIs
 */

#if !defined(__QDF_THREADS_H)
#define __QDF_THREADS_H

#include <qdf_types.h>
#include "i_qdf_threads.h"

typedef __qdf_thread_t qdf_thread_t;
typedef QDF_STATUS (*qdf_thread_func)(void *context);

/* Function declarations and documenation */

void qdf_sleep(uint32_t ms_interval);

void qdf_sleep_us(uint32_t us_interval);

void qdf_busy_wait(uint32_t us_interval);

/**
 * qdf_set_wake_up_idle() - set wakeup idle value
 * @idle: true/false value for wake up idle
 *
 * Return: none
 */
void qdf_set_wake_up_idle(bool idle);

/**
 * qdf_set_user_nice() - set thread's nice value
 * @thread: pointer to thread
 * @nice: nice value
 *
 * Return: void
 */
void qdf_set_user_nice(qdf_thread_t *thread, long nice);

/**
 * qdf_create_thread() - create a kernel thread
 * @thread: pointer to thread
 * @nice: nice value
 *
 * Return: pointer to created kernel thread on success else NULL
 */
qdf_thread_t *qdf_create_thread(int (*thread_handler)(void *data), void *data,
				const char thread_name[]);

/**
 * qdf_thread_run() - run the given function in a new thread
 *
 * You must call qdf_thread_join() to avoid a reasource leak!
 *
 * For more flexibility, use qdf_create_thread() instead.
 *
 * Return: a new qdf_thread pointer
 */
qdf_thread_t *qdf_thread_run(qdf_thread_func callback, void *context);

/**
 * qdf_thread_join() - signal and wait for a thread to stop
 *
 * This sets a flag that the given thread can check to see if it should exit.
 * The thread can check to see if this flag has been set by calling
 * qdf_thread_should_stop().
 *
 * Return: QDF_STATUS - the return value from the thread function
 */
QDF_STATUS qdf_thread_join(qdf_thread_t *thread);

/**
 * qdf_thread_should_stop() - true if the current thread was signalled to stop
 *
 * If qdf_thread_join() has been called on the current thread, this API returns
 * true. Otherwise, this returns false.
 *
 * Return: true if the current thread should stop
 */
bool qdf_thread_should_stop(void);

/**
 * qdf_wake_up_process() - wake up given thread
 * @thread: pointer to thread which needs to be woken up
 *
 * Return: none
 */
int qdf_wake_up_process(qdf_thread_t *thread);

/**
 * qdf_print_stack_trace_thread() - prints the stack trace of the given thread
 * @thread: the thread for which the stack trace will be printed
 *
 * Return: None
 */
void qdf_print_thread_trace(qdf_thread_t *thread);

/**
 * qdf_get_current_task() - get current task struct
 *
 * Return: pointer to task struct
 */
qdf_thread_t *qdf_get_current_task(void);

/**
 * qdf_get_current_pid() - get current task's process id
 *
 * Return: current task's process id (int)
 */
int qdf_get_current_pid(void);

/**
 * qdf_get_current_comm() - get current task's command name
 *
 * Return: current task's command name(char *)
 */
const char *qdf_get_current_comm(void);

/**
 * qdf_thread_set_cpus_allowed_mask() - set cpu mask for a particular thread
 * @thread: thread for which new cpu mask is set
 * @new_mask: new cpu mask to be set for the thread
 *
 * Return: None
 */
void
qdf_thread_set_cpus_allowed_mask(qdf_thread_t *thread, qdf_cpu_mask *new_mask);

/**
 * qdf_cpumask_clear() - clear all cpus in a cpumask
 * @dstp: cpumask pointer
 *
 * Return: None
 */
void qdf_cpumask_clear(qdf_cpu_mask *dstp);

/**
 * qdf_cpumask_set_cpu() - set a cpu in a cpumask
 * @cpu: cpu number
 * @dstp: cpumask pointer
 *
 * Return: None
 */
void qdf_cpumask_set_cpu(unsigned int cpu, qdf_cpu_mask *dstp);

/**
 * qdf_cpumask_setall - set all cpus
 * @dstp: cpumask pointer
 *
 * Return: None
 */
void qdf_cpumask_setall(qdf_cpu_mask *dstp);
#endif /* __QDF_THREADS_H */
