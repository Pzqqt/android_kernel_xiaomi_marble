/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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

#if !defined(__SCHEDULER_CORE_H)
#define __SCHEDULER_CORE_H

#include <qdf_threads.h>
#include <qdf_timer.h>
#include <scheduler_api.h>
#include <qdf_list.h>

#ifndef SCHEDULER_CORE_MAX_MESSAGES
#define SCHEDULER_CORE_MAX_MESSAGES 4000
#endif
#ifndef WLAN_SCHED_REDUCTION_LIMIT
#define WLAN_SCHED_REDUCTION_LIMIT 32
#endif
#define SCHEDULER_NUMBER_OF_MSG_QUEUE 6
#define SCHEDULER_WRAPPER_MAX_FAIL_COUNT (SCHEDULER_CORE_MAX_MESSAGES * 3)
#define SCHEDULER_WATCHDOG_TIMEOUT (10 * 1000) /* 10s */

#define sched_fatal(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_SCHEDULER, params)
#define sched_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_SCHEDULER, params)
#define sched_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_SCHEDULER, params)
#define sched_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_SCHEDULER, params)
#define sched_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_SCHEDULER, params)

#define sched_nofl_fatal(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_SCHEDULER, params)
#define sched_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SCHEDULER, params)
#define sched_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_SCHEDULER, params)
#define sched_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_SCHEDULER, params)
#define sched_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_SCHEDULER, params)

#define sched_enter() sched_debug("Enter")
#define sched_exit() sched_debug("Exit")

/**
 * struct scheduler_mq_type -  scheduler message queue
 * @mq_lock: message queue lock
 * @mq_list: message queue list
 * @qid: queue id
 */
struct scheduler_mq_type {
	qdf_spinlock_t mq_lock;
	qdf_list_t mq_list;
	QDF_MODULE_ID qid;
};

/**
 * struct scheduler_mq_ctx - scheduler message queue context
 * @sch_msg_q: scheduler message queue
 * @scheduler_msg_qid_to_qidx: message qid to qidx mapping
 * @scheduler_msg_process_fn: array of message queue handler function pointers
 */
struct scheduler_mq_ctx {
	struct scheduler_mq_type sch_msg_q[SCHEDULER_NUMBER_OF_MSG_QUEUE];
	uint8_t scheduler_msg_qid_to_qidx[QDF_MODULE_ID_MAX];
	QDF_STATUS (*scheduler_msg_process_fn[SCHEDULER_NUMBER_OF_MSG_QUEUE])
					(struct scheduler_msg *msg);
};

/**
 * struct scheduler_ctx - scheduler context
 * @queue_ctx: message queue context
 * @sch_start_event: scheduler thread start wait event
 * @sch_thread: scheduler thread
 * @sch_shutdown: scheduler thread shutdown wait event
 * @sch_wait_queue: scheduler wait queue
 * @sch_event_flag: scheduler events flag
 * @resume_sch_event: scheduler resume wait event
 * @sch_thread_lock: scheduler thread lock
 * @sch_last_qidx: scheduler last qidx allocation
 * @watchdog_msg_type: 'type' of the current msg being processed
 * @hdd_callback: os if suspend callback
 * @legacy_wma_handler: legacy wma message handler
 * @legacy_sys_handler: legacy sys message handler
 * @watchdog_timer: timer for triggering a scheduler watchdog bite
 * @watchdog_callback: the callback of the current msg being processed
 */
struct scheduler_ctx {
	struct scheduler_mq_ctx queue_ctx;
	qdf_event_t sch_start_event;
	qdf_thread_t *sch_thread;
	qdf_event_t sch_shutdown;
	qdf_wait_queue_head_t sch_wait_queue;
	unsigned long sch_event_flag;
	qdf_event_t resume_sch_event;
	qdf_spinlock_t sch_thread_lock;
	uint8_t sch_last_qidx;
	uint16_t watchdog_msg_type;
	hdd_suspend_callback hdd_callback;
	scheduler_msg_process_fn_t legacy_wma_handler;
	scheduler_msg_process_fn_t legacy_sys_handler;
	qdf_timer_t watchdog_timer;
	void *watchdog_callback;
};

/**
 * scheduler_core_msg_dup() duplicate the given scheduler message
 * @msg: the message to duplicated
 *
 * Note: Duplicated messages must be freed using scheduler_core_msg_free().
 *
 * Return: pointer to the duplicated message
 */
struct scheduler_msg *scheduler_core_msg_dup(struct scheduler_msg *msg);

/**
 * scheduler_core_msg_free() - free the given scheduler message
 * @msg: the duplicated message to free
 *
 * Return: None
 */
void scheduler_core_msg_free(struct scheduler_msg *msg);

/**
 * scheduler_get_context() - to get scheduler context
 *
 * This routine is used retrieve scheduler context
 *
 * Return: Pointer to scheduler context
 */
struct scheduler_ctx *scheduler_get_context(void);

/**
 * scheduler_thread() - spawned thread will execute this routine
 * @arg: pointer to scheduler context
 *
 * Newly created thread will use this routine to perform its duty
 *
 * Return: none
 */
int scheduler_thread(void *arg);

/**
 * scheduler_create_ctx() - to create scheduler context
 *
 * This routine is used to create scheduler context
 *
 * Return: QDF_STATUS based on success or failure
 */
QDF_STATUS scheduler_create_ctx(void);
/**
 * scheduler_destroy_ctx() - to destroy scheduler context
 *
 * This routine is used to destroy scheduler context
 *
 * Return: QDF_STATUS based on success or failure
 */
QDF_STATUS scheduler_destroy_ctx(void);

/**
 * scheduler_mq_put() - put message in the back of queue
 * @msg_q: Pointer to the message queue
 * @msg: the message to enqueue
 *
 * This function is used to put message in back of provided message
 * queue
 *
 *  Return: none
 */
void scheduler_mq_put(struct scheduler_mq_type *msg_q,
		      struct scheduler_msg *msg);
/**
 * scheduler_mq_put_front() - put message in the front of queue
 * @msg_q: Pointer to the message queue
 * @msg: the message to enqueue
 *
 * This function is used to put message in front of provided message
 * queue
 *
 *  Return: none
 */
void scheduler_mq_put_front(struct scheduler_mq_type *msg_q,
			    struct scheduler_msg *msg);
/**
 * scheduler_mq_get() - to get message from message queue
 * @msg_q: Pointer to the message queue
 *
 * This function is used to get message from given message queue
 *
 *  Return: none
 */
struct scheduler_msg *scheduler_mq_get(struct scheduler_mq_type *msg_q);

/**
 * scheduler_queues_init() - to initialize all the modules' queues
 * @sched_ctx: pointer to scheduler context
 *
 * This function is used to initialize the queues for all the modules
 *
 * Return: QDF_STATUS based on success of failure
 */
QDF_STATUS scheduler_queues_init(struct scheduler_ctx *sched_ctx);

/**
 * scheduler_queues_deinit() - to de-initialize all the modules' queues
 * @sched_ctx: pointer to scheduler context
 *
 * This function is used to de-initialize the queues for all the modules
 *
 * Return: QDF_STATUS based on success of failure
 */
QDF_STATUS scheduler_queues_deinit(struct scheduler_ctx *gp_sch_ctx);

/**
 * scheduler_queues_flush() - flush all of the scheduler queues
 * @sch_ctx: pointer to scheduler context
 *
 * This routine  is used to clean the module's queues
 *
 * Return: none
 */
void scheduler_queues_flush(struct scheduler_ctx *sched_ctx);
#endif
