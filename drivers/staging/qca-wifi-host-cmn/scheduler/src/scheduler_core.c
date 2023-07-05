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

#include <scheduler_core.h>
#include <qdf_atomic.h>
#include "qdf_flex_mem.h"

static struct scheduler_ctx g_sched_ctx;
static struct scheduler_ctx *gp_sched_ctx;

DEFINE_QDF_FLEX_MEM_POOL(sched_pool, sizeof(struct scheduler_msg),
			 WLAN_SCHED_REDUCTION_LIMIT);

#ifdef WLAN_SCHED_HISTORY_SIZE

#define SCHEDULER_HISTORY_HEADER "|Callback                               "\
				 "|Message Type"			   \
				 "|Queue Duration(us)|Queue Depth"	   \
				 "|Run Duration(us)|"

#define SCHEDULER_HISTORY_LINE "--------------------------------------" \
			       "--------------------------------------" \
			       "--------------------------------------"

/**
 * struct sched_history_item - metrics for a scheduler message
 * @callback: the message's execution callback
 * @type_id: the message's type_id
 * @queue_id: Id of the queue the message was added to
 * @queue_start_us: timestamp when the message was queued in microseconds
 * @queue_duration_us: duration the message was queued in microseconds
 * @queue_depth: depth of the queue when the message was queued
 * @run_start_us: timesatmp when the message started execution in microseconds
 * @run_duration_us: duration the message was executed in microseconds
 */
struct sched_history_item {
	void *callback;
	uint32_t type_id;
	QDF_MODULE_ID queue_id;
	uint64_t queue_start_us;
	uint32_t queue_duration_us;
	uint32_t queue_depth;
	uint64_t run_start_us;
	uint32_t run_duration_us;
};

static struct sched_history_item sched_history[WLAN_SCHED_HISTORY_SIZE];
static uint32_t sched_history_index;

static void sched_history_queue(struct scheduler_mq_type *queue,
				struct scheduler_msg *msg)
{
	msg->queue_id = queue->qid;
	msg->queue_depth = qdf_list_size(&queue->mq_list);
	msg->queued_at_us = qdf_get_log_timestamp_usecs();
}

static void sched_history_start(struct scheduler_msg *msg)
{
	uint64_t started_at_us = qdf_get_log_timestamp_usecs();
	struct sched_history_item hist = {
		.callback = msg->callback,
		.type_id = msg->type,
		.queue_start_us = msg->queued_at_us,
		.queue_duration_us = started_at_us - msg->queued_at_us,
		.queue_depth = msg->queue_depth,
		.run_start_us = started_at_us,
	};

	sched_history[sched_history_index] = hist;
}

static void sched_history_stop(void)
{
	struct sched_history_item *hist = &sched_history[sched_history_index];
	uint64_t stopped_at_us = qdf_get_log_timestamp_usecs();

	hist->run_duration_us = stopped_at_us - hist->run_start_us;

	sched_history_index++;
	sched_history_index %= WLAN_SCHED_HISTORY_SIZE;
}

void sched_history_print(void)
{
	struct sched_history_item *history, *item;
	uint32_t history_idx;
	uint32_t idx, index;

	history = qdf_mem_malloc(sizeof(*history) * WLAN_SCHED_HISTORY_SIZE);

	if (!history) {
		sched_err("Mem alloc failed");
		return;
	}

	qdf_mem_copy(history, &sched_history,
		     (sizeof(*history) * WLAN_SCHED_HISTORY_SIZE));
	history_idx = sched_history_index;

	sched_nofl_fatal(SCHEDULER_HISTORY_LINE);
	sched_nofl_fatal(SCHEDULER_HISTORY_HEADER);
	sched_nofl_fatal(SCHEDULER_HISTORY_LINE);

	for (idx = 0; idx < WLAN_SCHED_HISTORY_SIZE; idx++) {
		index = (history_idx + idx) % WLAN_SCHED_HISTORY_SIZE;
		item = history + index;

		if (!item->callback)
			continue;

		sched_nofl_fatal("%40pF|%12d|%18d|%11d|%16d|",
				 item->callback, item->type_id,
				 item->queue_duration_us,
				 item->queue_depth,
				 item->run_duration_us);
	}

	sched_nofl_fatal(SCHEDULER_HISTORY_LINE);

	qdf_mem_free(history);
}
#else /* WLAN_SCHED_HISTORY_SIZE */

static inline void sched_history_queue(struct scheduler_mq_type *queue,
				       struct scheduler_msg *msg) { }
static inline void sched_history_start(struct scheduler_msg *msg) { }
static inline void sched_history_stop(void) { }
void sched_history_print(void) { }

#endif /* WLAN_SCHED_HISTORY_SIZE */

QDF_STATUS scheduler_create_ctx(void)
{
	qdf_flex_mem_init(&sched_pool);
	gp_sched_ctx = &g_sched_ctx;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_destroy_ctx(void)
{
	gp_sched_ctx = NULL;
	qdf_flex_mem_deinit(&sched_pool);

	return QDF_STATUS_SUCCESS;
}

struct scheduler_ctx *scheduler_get_context(void)
{
	QDF_BUG(gp_sched_ctx);

	return gp_sched_ctx;
}

static QDF_STATUS scheduler_mq_init(struct scheduler_mq_type *msg_q)
{
	sched_enter();

	qdf_spinlock_create(&msg_q->mq_lock);
	qdf_list_create(&msg_q->mq_list, SCHEDULER_CORE_MAX_MESSAGES);

	sched_exit();

	return QDF_STATUS_SUCCESS;
}

static void scheduler_mq_deinit(struct scheduler_mq_type *msg_q)
{
	sched_enter();

	qdf_list_destroy(&msg_q->mq_list);
	qdf_spinlock_destroy(&msg_q->mq_lock);

	sched_exit();
}

static qdf_atomic_t __sched_queue_depth;
static qdf_atomic_t __sched_dup_fail_count;

static QDF_STATUS scheduler_all_queues_init(struct scheduler_ctx *sched_ctx)
{
	QDF_STATUS status;
	int i;

	sched_enter();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	qdf_atomic_set(&__sched_queue_depth, 0);

	/* Initialize all message queues */
	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++) {
		status = scheduler_mq_init(&sched_ctx->queue_ctx.sch_msg_q[i]);
		if (QDF_STATUS_SUCCESS != status)
			return status;
	}

	/* Initialize all qid to qidx mapping to invalid values */
	for (i = 0; i < QDF_MODULE_ID_MAX; i++)
		sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[i] =
					SCHEDULER_NUMBER_OF_MSG_QUEUE;

	sched_exit();

	return status;
}

static QDF_STATUS scheduler_all_queues_deinit(struct scheduler_ctx *sched_ctx)
{
	int i;

	sched_enter();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	/* De-Initialize all message queues */
	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++)
		scheduler_mq_deinit(&sched_ctx->queue_ctx.sch_msg_q[i]);

	/* Initialize all qid to qidx mapping to invalid values */
	for (i = 0; i < QDF_MODULE_ID_MAX; i++)
		sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[i] =
					SCHEDULER_NUMBER_OF_MSG_QUEUE;

	sched_exit();

	return QDF_STATUS_SUCCESS;
}

void scheduler_mq_put(struct scheduler_mq_type *msg_q,
		      struct scheduler_msg *msg)
{
	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	sched_history_queue(msg_q, msg);
	qdf_list_insert_back(&msg_q->mq_list, &msg->node);
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);
}

void scheduler_mq_put_front(struct scheduler_mq_type *msg_q,
			    struct scheduler_msg *msg)
{
	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	sched_history_queue(msg_q, msg);
	qdf_list_insert_front(&msg_q->mq_list, &msg->node);
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);
}

struct scheduler_msg *scheduler_mq_get(struct scheduler_mq_type *msg_q)
{
	QDF_STATUS status;
	qdf_list_node_t *node;

	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	status = qdf_list_remove_front(&msg_q->mq_list, &node);
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);

	if (QDF_IS_STATUS_ERROR(status))
		return NULL;

	return qdf_container_of(node, struct scheduler_msg, node);
}

QDF_STATUS scheduler_queues_deinit(struct scheduler_ctx *sched_ctx)
{
	return scheduler_all_queues_deinit(sched_ctx);
}

QDF_STATUS scheduler_queues_init(struct scheduler_ctx *sched_ctx)
{
	QDF_STATUS status;

	sched_enter();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	status = scheduler_all_queues_init(sched_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		scheduler_all_queues_deinit(sched_ctx);
		sched_err("Failed to initialize the msg queues");
		return status;
	}

	sched_debug("Queue init passed");

	sched_exit();

	return QDF_STATUS_SUCCESS;
}

struct scheduler_msg *scheduler_core_msg_dup(struct scheduler_msg *msg)
{
	struct scheduler_msg *dup;

	if (qdf_atomic_inc_return(&__sched_queue_depth) >
	    SCHEDULER_CORE_MAX_MESSAGES)
		goto buffer_full;

	dup = qdf_flex_mem_alloc(&sched_pool);
	if (!dup) {
		sched_err("out of memory");
		goto dec_queue_count;
	}

	qdf_mem_copy(dup, msg, sizeof(*dup));

	qdf_atomic_set(&__sched_dup_fail_count, 0);

	return dup;

buffer_full:
	if (qdf_atomic_inc_return(&__sched_dup_fail_count) >
	    SCHEDULER_WRAPPER_MAX_FAIL_COUNT)
		QDF_DEBUG_PANIC("Scheduler buffer is full");


dec_queue_count:
	qdf_atomic_dec(&__sched_queue_depth);

	return NULL;
}

void scheduler_core_msg_free(struct scheduler_msg *msg)
{
	qdf_flex_mem_free(&sched_pool, msg);
	qdf_atomic_dec(&__sched_queue_depth);
}

static void scheduler_thread_process_queues(struct scheduler_ctx *sch_ctx,
					    bool *shutdown)
{
	int i;
	QDF_STATUS status;
	struct scheduler_msg *msg;

	if (!sch_ctx) {
		QDF_DEBUG_PANIC("sch_ctx is null");
		return;
	}

	/* start with highest priority queue : timer queue at index 0 */
	i = 0;
	while (i < SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		/* Check if MC needs to shutdown */
		if (qdf_atomic_test_bit(MC_SHUTDOWN_EVENT_MASK,
					&sch_ctx->sch_event_flag)) {
			sched_debug("scheduler thread signaled to shutdown");
			*shutdown = true;

			/* Check for any Suspend Indication */
			if (qdf_atomic_test_and_clear_bit(MC_SUSPEND_EVENT_MASK,
						&sch_ctx->sch_event_flag)) {
				/* Unblock anyone waiting on suspend */
				if (gp_sched_ctx->hdd_callback)
					gp_sched_ctx->hdd_callback();
			}

			break;
		}

		msg = scheduler_mq_get(&sch_ctx->queue_ctx.sch_msg_q[i]);
		if (!msg) {
			/* check next queue */
			i++;
			continue;
		}

		if (sch_ctx->queue_ctx.scheduler_msg_process_fn[i]) {
			sch_ctx->watchdog_msg_type = msg->type;
			sch_ctx->watchdog_callback = msg->callback;

			sched_history_start(msg);
			qdf_timer_start(&sch_ctx->watchdog_timer,
					sch_ctx->timeout);
			status = sch_ctx->queue_ctx.
					scheduler_msg_process_fn[i](msg);
			qdf_timer_stop(&sch_ctx->watchdog_timer);
			sched_history_stop();

			if (QDF_IS_STATUS_ERROR(status))
				sched_err("Failed processing Qid[%d] message",
					  sch_ctx->queue_ctx.sch_msg_q[i].qid);

			scheduler_core_msg_free(msg);
		}

		/* start again with highest priority queue at index 0 */
		i = 0;
	}

	/* Check for any Suspend Indication */
	if (qdf_atomic_test_and_clear_bit(MC_SUSPEND_EVENT_MASK,
			&sch_ctx->sch_event_flag)) {
		qdf_spin_lock(&sch_ctx->sch_thread_lock);
		qdf_event_reset(&sch_ctx->resume_sch_event);
		/* controller thread suspend completion callback */
		if (gp_sched_ctx->hdd_callback)
			gp_sched_ctx->hdd_callback();
		qdf_spin_unlock(&sch_ctx->sch_thread_lock);
		/* Wait for resume indication */
		qdf_wait_single_event(&sch_ctx->resume_sch_event, 0);
	}

	return;  /* Nothing to process wait on wait queue */
}

int scheduler_thread(void *arg)
{
	struct scheduler_ctx *sch_ctx = (struct scheduler_ctx *)arg;
	int retWaitStatus = 0;
	bool shutdown = false;

	if (!arg) {
		QDF_DEBUG_PANIC("arg is null");
		return 0;
	}
	qdf_set_user_nice(current, -2);

	/* Ack back to the context from which the main controller thread
	 * has been created
	 */
	qdf_event_set(&sch_ctx->sch_start_event);
	sched_debug("scheduler thread %d (%s) starting up",
		    current->pid, current->comm);

	while (!shutdown) {
		/* This implements the execution model algorithm */
		retWaitStatus = qdf_wait_queue_interruptible(
					sch_ctx->sch_wait_queue,
					qdf_atomic_test_bit(MC_POST_EVENT_MASK,
						&sch_ctx->sch_event_flag) ||
					qdf_atomic_test_bit(MC_SUSPEND_EVENT_MASK,
						&sch_ctx->sch_event_flag));

		if (retWaitStatus == -ERESTARTSYS)
			QDF_DEBUG_PANIC("Scheduler received -ERESTARTSYS");

		qdf_atomic_clear_bit(MC_POST_EVENT_MASK, &sch_ctx->sch_event_flag);
		scheduler_thread_process_queues(sch_ctx, &shutdown);
	}

	/* If we get here the scheduler thread must exit */
	sched_debug("Scheduler thread exiting");
	qdf_event_set(&sch_ctx->sch_shutdown);
	qdf_exit_thread(QDF_STATUS_SUCCESS);

	return 0;
}

static void scheduler_flush_single_queue(struct scheduler_mq_type *mq)
{
	struct scheduler_msg *msg;
	QDF_STATUS (*flush_cb)(struct scheduler_msg *);

	while ((msg = scheduler_mq_get(mq))) {
		if (msg->flush_callback) {
			sched_debug("Calling flush callback; type: %x",
				    msg->type);
			flush_cb = msg->flush_callback;
			flush_cb(msg);
		} else if (msg->bodyptr) {
			sched_debug("Freeing scheduler msg bodyptr; type: %x",
				    msg->type);
			qdf_mem_free(msg->bodyptr);
		}

		scheduler_core_msg_free(msg);
	}
}

void scheduler_queues_flush(struct scheduler_ctx *sched_ctx)
{
	struct scheduler_mq_type *mq;
	int i;

	sched_debug("Flushing scheduler message queues");

	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++) {
		mq = &sched_ctx->queue_ctx.sch_msg_q[i];
		scheduler_flush_single_queue(mq);
	}
}

