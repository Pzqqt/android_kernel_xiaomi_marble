/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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

#include <scheduler_core.h>

static struct scheduler_ctx g_sched_ctx;
static struct scheduler_ctx *gp_sched_ctx;

QDF_STATUS scheduler_create_ctx(void)
{
	gp_sched_ctx = &g_sched_ctx;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_destroy_ctx(void)
{
	gp_sched_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}

struct scheduler_ctx *scheduler_get_context(void)
{
	return gp_sched_ctx;
}


static QDF_STATUS scheduler_all_queues_init(
			struct scheduler_ctx *sched_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("enter"));
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	status = scheduler_mq_init(&sched_ctx->queue_ctx.free_msg_q);
	if (QDF_STATUS_SUCCESS != status)
		return status;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
		QDF_TRACE_LEVEL_ERROR, FL("free msg queue init complete"));

	/* Initialize all message queues */
	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++) {
		status = scheduler_mq_init(
				&sched_ctx->queue_ctx.sch_msg_q[i]);
		if (QDF_STATUS_SUCCESS != status)
			return status;
	}
	/* Initialize all qid to qidx mapping to invalid values */
	for (i = 0; i < QDF_MODULE_ID_MAX; i++)
		sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[i] =
					SCHEDULER_NUMBER_OF_MSG_QUEUE;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("exit"));

	return status;
}


static QDF_STATUS scheduler_all_queues_deinit(
		struct scheduler_ctx *sched_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("enter"));
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	scheduler_mq_deinit(&sched_ctx->queue_ctx.free_msg_q);

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
		  QDF_TRACE_LEVEL_ERROR, FL("free msg queue inited"));

	/* De-Initialize all message queues */
	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++)
		scheduler_mq_deinit(&sched_ctx->queue_ctx.sch_msg_q[i]);

	/* Initialize all qid to qidx mapping to invalid values */
	for (i = 0; i < QDF_MODULE_ID_MAX; i++)
		sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[i] =
					SCHEDULER_NUMBER_OF_MSG_QUEUE;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("exit"));
	return status;
}

QDF_STATUS scheduler_mq_init(struct scheduler_mq_type *msg_q)
{
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("Enter"));
	if (msg_q == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* Now initialize the lock */
	qdf_spinlock_create(&msg_q->mq_lock);
	/* Now initialize the List data structure */
	qdf_list_create(&msg_q->mq_list, SCHEDULER_CORE_MAX_MESSAGES);
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("Exit"));

	return QDF_STATUS_SUCCESS;
}

void scheduler_mq_deinit(struct scheduler_mq_type *msg_q)
{
	if (msg_q == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: NULL pointer passed", __func__);
		return;
	}
}

void scheduler_mq_put(struct scheduler_mq_type *msg_q,
			struct scheduler_msg_wrapper *msg_wrapper)
{
	if (msg_q == NULL || msg_wrapper == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: NULL pointer passed", __func__);
		return;
	}
	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	qdf_list_insert_back(&msg_q->mq_list, &msg_wrapper->msg_node);
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);

}

void scheduler_mq_put_front(struct scheduler_mq_type *msg_q,
			struct scheduler_msg_wrapper *msg_wrapper)
{
	if ((msg_q == NULL) || (msg_wrapper == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: NULL pointer passed", __func__);
		return;
	}
	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	qdf_list_insert_front(&msg_q->mq_list, &msg_wrapper->msg_node);
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);
}

struct scheduler_msg_wrapper *scheduler_mq_get(struct scheduler_mq_type *msg_q)
{
	qdf_list_node_t *listptr;
	struct scheduler_msg_wrapper *msg_wrapper = NULL;

	if (msg_q == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: NULL pointer passed", __func__);
		return NULL;
	}

	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	if (qdf_list_empty(&msg_q->mq_list)) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_WARN,
			  "%s: Scheduler Message Queue is empty", __func__);
	} else {
		listptr = msg_q->mq_list.anchor.next;
		msg_wrapper = (struct scheduler_msg_wrapper *)
					qdf_container_of(listptr,
						struct scheduler_msg_wrapper,
						msg_node);
		qdf_list_remove_node(&msg_q->mq_list, listptr);
	}
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);
	return msg_wrapper;

}

bool scheduler_is_mq_empty(struct scheduler_mq_type *msg_q)
{
	bool is_empty = false;

	if (msg_q == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: NULL pointer passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_spin_lock_irqsave(&msg_q->mq_lock);
	is_empty = qdf_list_empty(&msg_q->mq_list) ? true : false;
	qdf_spin_unlock_irqrestore(&msg_q->mq_lock);
	return is_empty;
}

QDF_STATUS scheduler_queues_deinit(struct scheduler_ctx *sched_ctx)
{
	return scheduler_all_queues_deinit(sched_ctx);
}

QDF_STATUS scheduler_queues_init(struct scheduler_ctx *sched_ctx)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int i;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("Enter"));
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	status = scheduler_all_queues_init(sched_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		scheduler_all_queues_deinit(sched_ctx);
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_FATAL,
				FL("Failed to initialize the msg queues"));
		return status;
	}
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
		QDF_TRACE_LEVEL_ERROR, FL("Queue init passed"));

	for (i = 0; i < SCHEDULER_CORE_MAX_MESSAGES; i++) {
		(sched_ctx->queue_ctx.msg_wrappers[i]).msg_buf =
			&(sched_ctx->queue_ctx.msg_buffers[i]);
		qdf_init_list_head(
			&sched_ctx->queue_ctx.msg_wrappers[i].msg_node);
		scheduler_mq_put(&sched_ctx->queue_ctx.free_msg_q,
			   &(sched_ctx->queue_ctx.msg_wrappers[i]));
	}
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR, FL("Exit"));
	return status;
}

static void scheduler_core_return_msg(struct scheduler_ctx *sch_ctx,
			struct scheduler_msg_wrapper *msg_wrapper)
{
	if (!sch_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			"%s: gp_cds_context != p_cds_context", __func__);
		return;
	}

	QDF_ASSERT(NULL != msg_wrapper);

	if (msg_wrapper == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			FL("msg_wrapper == NULL in function"));
		return;
	}

	/*
	 * Return the message on the free message queue
	 */
	qdf_init_list_head(&msg_wrapper->msg_node);
	scheduler_mq_put(&sch_ctx->queue_ctx.free_msg_q, msg_wrapper);
}

static void scheduler_thread_process_queues(struct scheduler_ctx *sch_ctx,
					    bool *shutdown)
{
	int i;
	QDF_STATUS vStatus = QDF_STATUS_E_FAILURE;
	struct scheduler_msg_wrapper *msg_wrapper = NULL;

	if (!sch_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("sch_ctx null"));
		return;
	}

	/* start with highest priority queue : timer queue at index 0 */
	i = 0;
	while (i < SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		/* Check if MC needs to shutdown */
		if (qdf_test_bit(MC_SHUTDOWN_EVENT_MASK,
					&sch_ctx->sch_event_flag)) {
			QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
				QDF_TRACE_LEVEL_ERROR,
				"%s: scheduler thread signaled to shutdown",
				__func__);
			*shutdown = true;
			/* Check for any Suspend Indication */
			if (qdf_test_and_clear_bit(MC_SUSPEND_EVENT_MASK,
						&sch_ctx->sch_event_flag)) {
				/* Unblock anyone waiting on suspend */
				if (gp_sched_ctx->hdd_callback)
					gp_sched_ctx->hdd_callback();
			}
			break;
		}
		if (scheduler_is_mq_empty(&sch_ctx->queue_ctx.sch_msg_q[i])) {
			/* check next queue */
			i++;
			continue;
		}
		msg_wrapper =
			scheduler_mq_get(&sch_ctx->queue_ctx.sch_msg_q[i]);
		if (msg_wrapper == NULL) {
			QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
				QDF_TRACE_LEVEL_ERROR,
				"%s: msg_wrapper is NULL", __func__);
			QDF_ASSERT(0);
			return;
		}
		if (sch_ctx->queue_ctx.scheduler_msg_process_fn[i]) {
			struct scheduler_msg *msg = msg_wrapper->msg_buf;

			sch_ctx->watchdog_msg_type = msg->type;
			sch_ctx->watchdog_callback = msg->callback;
			qdf_timer_start(&sch_ctx->watchdog_timer,
					SCHEDULER_WATCHDOG_TIMEOUT);
			vStatus = sch_ctx->queue_ctx.
					scheduler_msg_process_fn[i](msg);
			qdf_timer_stop(&sch_ctx->watchdog_timer);

			if (QDF_IS_STATUS_ERROR(vStatus)) {
				QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
					QDF_TRACE_LEVEL_ERROR,
					FL("Failed processing Qid[%d] message"),
					sch_ctx->queue_ctx.sch_msg_q[i].qid);
			}
			/* return message to the Core */
			scheduler_core_return_msg(sch_ctx, msg_wrapper);
		}

		/* start again with highest priority queue at index 0 */
		i = 0;
		continue;
	}
	/* Check for any Suspend Indication */
	if (qdf_test_and_clear_bit(MC_SUSPEND_EVENT_MASK,
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

	if (arg == NULL) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: Bad Args passed", __func__);
		return 0;
	}
	qdf_set_user_nice(current, -2);

	/* Ack back to the context from which the main controller thread
	 * has been created
	 */
	qdf_event_set(&sch_ctx->sch_start_event);
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
		  "%s: scheduler_thread %d (%s) starting up", __func__, current->pid,
		  current->comm);

	while (!shutdown) {
		/* This implements the execution model algorithm */
		retWaitStatus = qdf_wait_queue_interruptible(
					sch_ctx->sch_wait_queue,
					qdf_test_bit(MC_POST_EVENT_MASK,
						&sch_ctx->sch_event_flag) ||
					qdf_test_bit(MC_SUSPEND_EVENT_MASK,
						&sch_ctx->sch_event_flag));

		if (retWaitStatus == -ERESTARTSYS) {
			QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
					"%s: wait_event_interruptible returned -ERESTARTSYS",
					__func__);
			QDF_BUG(0);
		}
		qdf_clear_bit(MC_POST_EVENT_MASK, &sch_ctx->sch_event_flag);
		scheduler_thread_process_queues(sch_ctx, &shutdown);
	}
	/* If we get here the MC thread must exit */
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
		  "%s: Scheduler thread exiting!!!!", __func__);
	qdf_event_set(&sch_ctx->sch_shutdown);
	qdf_exit_thread(QDF_STATUS_SUCCESS);

	return 0;
}

void scheduler_cleanup_queues(struct scheduler_ctx *sch_ctx, int idx)
{
	struct scheduler_msg_wrapper *msg_wrapper = NULL;

	if (!sch_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return;
	}

	while ((msg_wrapper =
			scheduler_mq_get(&sch_ctx->queue_ctx.sch_msg_q[idx]))) {
		if (msg_wrapper->msg_buf != NULL) {
			QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO,
				"%s: Freeing MC WMA MSG message type %d",
				__func__, msg_wrapper->msg_buf->type);
			if (msg_wrapper->msg_buf->bodyptr)
				qdf_mem_free(
					(void *)msg_wrapper->msg_buf->bodyptr);
			msg_wrapper->msg_buf->bodyptr = NULL;
			msg_wrapper->msg_buf->bodyval = 0;
			msg_wrapper->msg_buf->type = 0;
		}
		scheduler_core_return_msg(sch_ctx, msg_wrapper);
	}
}
