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
 * DOC: qdf_threads
 * QCA driver framework (QDF) thread APIs
 */

/* Include Files */
#include <qdf_threads.h>
#include <qdf_types.h>
#include <qdf_trace.h>
#include <linux/jiffies.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
#include <linux/sched.h>
#else
#include <linux/sched/signal.h>
#endif /* KERNEL_VERSION(4, 11, 0) */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/stacktrace.h>
#include <qdf_defer.h>
#include <qdf_module.h>

/* Function declarations and documenation */

typedef int (*qdf_thread_os_func)(void *data);

/**
 *  qdf_sleep() - sleep
 *  @ms_interval : Number of milliseconds to suspend the current thread.
 *  A value of 0 may or may not cause the current thread to yield.
 *
 *  This function suspends the execution of the current thread
 *  until the specified time out interval elapses.
 *
 *  Return: none
 */
void qdf_sleep(uint32_t ms_interval)
{
	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		return;
	}
	msleep_interruptible(ms_interval);
}
qdf_export_symbol(qdf_sleep);

/**
 *  qdf_sleep_us() - sleep
 *  @us_interval : Number of microseconds to suspend the current thread.
 *  A value of 0 may or may not cause the current thread to yield.
 *
 *  This function suspends the execution of the current thread
 *  until the specified time out interval elapses.
 *
 *  Return : none
 */
void qdf_sleep_us(uint32_t us_interval)
{
	unsigned long timeout = usecs_to_jiffies(us_interval) + 1;

	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		return;
	}

	while (timeout && !signal_pending(current))
		timeout = schedule_timeout_interruptible(timeout);
}
qdf_export_symbol(qdf_sleep_us);

/**
 *  qdf_busy_wait() - busy wait
 *  @us_interval : Number of microseconds to busy wait.
 *
 *  This function places the current thread in busy wait until the specified
 *  time out interval elapses. If the interval is greater than 50us on WM, the
 *  behaviour is undefined.
 *
 *  Return : none
 */
void qdf_busy_wait(uint32_t us_interval)
{
	udelay(us_interval);
}
qdf_export_symbol(qdf_busy_wait);

#ifdef PF_WAKE_UP_IDLE
void qdf_set_wake_up_idle(bool idle)
{
	set_wake_up_idle(idle);
}
#else
void qdf_set_wake_up_idle(bool idle)
{
}
#endif /* PF_WAKE_UP_IDLE */

qdf_export_symbol(qdf_set_wake_up_idle);

void qdf_set_user_nice(qdf_thread_t *thread, long nice)
{
	set_user_nice(thread, nice);
}
qdf_export_symbol(qdf_set_user_nice);

qdf_thread_t *qdf_create_thread(int (*thread_handler)(void *data), void *data,
				const char thread_name[])
{
	struct task_struct *task;

	task = kthread_create(thread_handler, data, thread_name);

	if (IS_ERR(task))
		return NULL;

	return task;
}
qdf_export_symbol(qdf_create_thread);

static uint16_t qdf_thread_id;

qdf_thread_t *qdf_thread_run(qdf_thread_func callback, void *context)
{
	struct task_struct *thread;

	thread = kthread_create((qdf_thread_os_func)callback, context,
				"qdf %u", qdf_thread_id++);
	if (IS_ERR(thread))
		return NULL;

	get_task_struct(thread);
	wake_up_process(thread);

	return thread;
}
qdf_export_symbol(qdf_thread_run);

QDF_STATUS qdf_thread_join(qdf_thread_t *thread)
{
	QDF_STATUS status;

	QDF_BUG(thread);

	status = (QDF_STATUS)kthread_stop(thread);
	put_task_struct(thread);

	return status;
}
qdf_export_symbol(qdf_thread_join);

bool qdf_thread_should_stop(void)
{
	return kthread_should_stop();
}
qdf_export_symbol(qdf_thread_should_stop);

int qdf_wake_up_process(qdf_thread_t *thread)
{
	return wake_up_process(thread);
}
qdf_export_symbol(qdf_wake_up_process);

/* save_stack_trace_tsk() is exported for:
 * 1) non-arm architectures
 * 2) arm architectures in kernel versions >=4.14
 * 3) backported kernels defining BACKPORTED_EXPORT_SAVE_STACK_TRACE_TSK_ARM
 */
#if ((defined(WLAN_HOST_ARCH_ARM) && !WLAN_HOST_ARCH_ARM) || \
	LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0) || \
	defined(BACKPORTED_EXPORT_SAVE_STACK_TRACE_TSK_ARM)) && \
	defined(CONFIG_STACKTRACE) && !defined(CONFIG_ARCH_STACKWALK)
#define QDF_PRINT_TRACE_COUNT 32

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
void qdf_print_thread_trace(qdf_thread_t *thread)
{
	const int spaces = 4;
	struct task_struct *task = thread;
	unsigned long entries[QDF_PRINT_TRACE_COUNT] = {0};
	struct stack_trace trace = {
		.nr_entries = 0,
		.skip = 0,
		.entries = &entries[0],
		.max_entries = QDF_PRINT_TRACE_COUNT,
	};

	save_stack_trace_tsk(task, &trace);
	stack_trace_print(entries, trace.nr_entries, spaces);
}
#else
void qdf_print_thread_trace(qdf_thread_t *thread)
{
	const int spaces = 4;
	struct task_struct *task = thread;
	unsigned long entries[QDF_PRINT_TRACE_COUNT] = {0};
	struct stack_trace trace = {
		.nr_entries = 0,
		.skip = 0,
		.entries = &entries[0],
		.max_entries = QDF_PRINT_TRACE_COUNT,
	};

	save_stack_trace_tsk(task, &trace);
	print_stack_trace(&trace, spaces);
}
#endif

#else
void qdf_print_thread_trace(qdf_thread_t *thread) { }
#endif /* KERNEL_VERSION(4, 14, 0) */
qdf_export_symbol(qdf_print_thread_trace);

qdf_thread_t *qdf_get_current_task(void)
{
	return current;
}
qdf_export_symbol(qdf_get_current_task);

int qdf_get_current_pid(void)
{
	return current->pid;
}
qdf_export_symbol(qdf_get_current_pid);

const char *qdf_get_current_comm(void)
{
	return current->comm;
}
qdf_export_symbol(qdf_get_current_comm);

void
qdf_thread_set_cpus_allowed_mask(qdf_thread_t *thread, qdf_cpu_mask *new_mask)
{
	set_cpus_allowed_ptr(thread, new_mask);
}

qdf_export_symbol(qdf_thread_set_cpus_allowed_mask);

void qdf_cpumask_clear(qdf_cpu_mask *dstp)
{
	cpumask_clear(dstp);
}

qdf_export_symbol(qdf_cpumask_clear);

void qdf_cpumask_set_cpu(unsigned int cpu, qdf_cpu_mask *dstp)
{
	cpumask_set_cpu(cpu, dstp);
}

qdf_export_symbol(qdf_cpumask_set_cpu);

void qdf_cpumask_setall(qdf_cpu_mask *dstp)
{
	cpumask_setall(dstp);
}

qdf_export_symbol(qdf_cpumask_setall);
