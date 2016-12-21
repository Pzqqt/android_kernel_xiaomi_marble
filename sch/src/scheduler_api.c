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

#include <scheduler_api.h>
#include <scheduler_core.h>

static void scheduler_flush_mqs(struct scheduler_ctx *sched_ctx)
{
	int i;

	/* Here each of the MC thread MQ shall be drained and returned to the
	 * Core. Before returning a wrapper to the Core, the Scheduler message
	 * shall be freed first
	 */
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO,
		  ("Flushing scheduler message queue"));

	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: sched_ctx is NULL", __func__);
		return;
	}
	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++)
		scheduler_cleanup_queues(sched_ctx, i);
}

static QDF_STATUS scheduler_close(struct scheduler_ctx *sched_ctx)
{
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s: invoked", __func__);
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: sched_ctx == NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* shut down scheduler thread */
	qdf_set_bit(MC_SHUTDOWN_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_set_bit(MC_POST_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_wake_up_interruptible(&sched_ctx->sch_wait_queue);
	/* Wait for MC to exit */
	qdf_wait_single_event(&sched_ctx->sch_shutdown, 0);
	sched_ctx->sch_thread = 0;

	/* Clean up message queues of MC thread */
	scheduler_flush_mqs(sched_ctx);

	/* Deinit all the queues */
	scheduler_queues_deinit(sched_ctx);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scheduler_open(struct scheduler_ctx *sched_ctx)
{
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Opening the QDF Scheduler", __func__);
	/* Sanity checks */
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* Initialize the helper events and event queues */
	qdf_event_create(&sched_ctx->sch_start_event);
	qdf_event_create(&sched_ctx->sch_shutdown);
	qdf_event_create(&sched_ctx->resume_sch_event);
	qdf_spinlock_create(&sched_ctx->sch_thread_lock);
	qdf_init_waitqueue_head(&sched_ctx->sch_wait_queue);
	sched_ctx->sch_event_flag = 0;
	/* Create the Scheduler Main Controller thread */
	sched_ctx->sch_thread = qdf_create_thread(scheduler_thread,
					sched_ctx, "scheduler_thread");
	if (IS_ERR(sched_ctx->sch_thread)) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_FATAL,
			  "%s: Could not Create QDF Main Thread Controller",
			  __func__);
		scheduler_queues_deinit(sched_ctx);
		return QDF_STATUS_E_RESOURCES;
	}
	/* start the thread here */
	qdf_wake_up_process(sched_ctx->sch_thread);
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
		  "%s: QDF Main Controller thread Created", __func__);

	/*
	 * Now make sure all threads have started before we exit.
	 * Each thread should normally ACK back when it starts.
	 */
	qdf_wait_single_event(&sched_ctx->sch_start_event, 0);
	/* We're good now: Let's get the ball rolling!!! */
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
		  "%s: Scheduler thread has started", __func__);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct scheduler_ctx *sched_ctx;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("Opening Scheduler"));
	status = scheduler_create_ctx();
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("can't create scheduler ctx"));
		return status;
	}
	sched_ctx = scheduler_get_context();
	status = scheduler_queues_init(sched_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("Queue init failed"));
		scheduler_destroy_ctx();
		return status;
	}
	status = scheduler_open(sched_ctx);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		/* Critical Error ...  Cannot proceed further */
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_FATAL,
				"Failed to open QDF Scheduler");
		QDF_ASSERT(0);
		scheduler_queues_deinit(sched_ctx);
		scheduler_destroy_ctx();
	}
	qdf_register_mc_timer_callback(scheduler_mc_timer_callback);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("Closing Scheduler"));
	status = scheduler_close(sched_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("Scheduler close failed"));
		return status;
	}
	return scheduler_destroy_ctx();
}


QDF_STATUS scheduler_post_msg_by_priority(QDF_MODULE_ID qid,
		struct scheduler_msg *pMsg, bool is_high_priority)
{
	uint8_t qidx;
	struct scheduler_mq_type *target_mq = NULL;
	struct scheduler_msg_wrapper *msg_wrapper = NULL;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (!sched_ctx || !pMsg) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: Null params or global sch context is null",
				__func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	/* Target_If is a special message queue in phase 3 convergence beacause
	 * its used by both legacy WMA and as well as new UMAC components which
	 * directly populate callback handlers in message body.
	 * 1) WMA legacy messages should not have callback
	 * 2) New target_if message needs to have valid callback
	 * Clear callback handler for legacy WMA messages such that in case
	 * if someone is sending legacy WMA message from stack which has
	 * uninitialized callback then its handled properly. Also change
	 * legacy WMA message queue id to target_if queue such that its  always
	 * handled in right order.
	 */
	if (QDF_MODULE_ID_WMA == qid) {
		pMsg->callback = NULL;
		/* change legacy WMA message id to new target_if mq id */
		qid = QDF_MODULE_ID_TARGET_IF;
	}

	qidx = sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[qid];
	if (qidx >= SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("Scheduler is deinitialized ignore msg"));
		return QDF_STATUS_E_FAILURE;
	}
	if (!sched_ctx->queue_ctx.scheduler_msg_process_fn[qidx]) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("callback not registered for qid[%d]"), qid);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	target_mq = &(sched_ctx->queue_ctx.sch_msg_q[qidx]);
	QDF_ASSERT(target_mq);
	if (target_mq == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				"%s: target_mq == NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Try and get a free Msg wrapper */
	msg_wrapper = scheduler_mq_get(&sched_ctx->queue_ctx.free_msg_q);

	if (NULL == msg_wrapper) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  FL("message wrapper empty"));
		return QDF_STATUS_E_RESOURCES;
	}

	/* Copy the message now */
	qdf_mem_copy((void *)msg_wrapper->msg_buf,
			(void *)pMsg, sizeof(struct scheduler_msg));

	if (is_high_priority)
		scheduler_mq_put_front(target_mq, msg_wrapper);
	else
		scheduler_mq_put(target_mq, msg_wrapper);

	qdf_set_bit(MC_POST_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_wake_up_interruptible(&sched_ctx->sch_wait_queue);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_register_module(QDF_MODULE_ID qid,
		scheduler_msg_process_fn_t callback)
{
	struct scheduler_mq_ctx *ctx;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO,
		FL("Enter"));
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			FL("sched_ctx is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (sched_ctx->sch_last_qidx >= SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR,
			FL("Already registered max %d no of message queues"),
				SCHEDULER_NUMBER_OF_MSG_QUEUE);
		return QDF_STATUS_E_FAILURE;
	}

	ctx = &sched_ctx->queue_ctx;
	ctx->scheduler_msg_qid_to_qidx[qid] = sched_ctx->sch_last_qidx;
	ctx->sch_msg_q[sched_ctx->sch_last_qidx].qid = qid;
	ctx->scheduler_msg_process_fn[sched_ctx->sch_last_qidx] = callback;
	sched_ctx->sch_last_qidx++;
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO,
		FL("Exit"));
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deregister_module(QDF_MODULE_ID qid)
{
	struct scheduler_mq_ctx *ctx;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();
	uint8_t qidx;

	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO,
		FL("Enter"));
	if (!sched_ctx) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
				FL("sched_ctx is NULL"));
		return QDF_STATUS_E_FAILURE;
	}
	ctx = &sched_ctx->queue_ctx;
	qidx = ctx->scheduler_msg_qid_to_qidx[qid];
	ctx->scheduler_msg_process_fn[qidx] = NULL;
	ctx->scheduler_msg_qid_to_qidx[qidx] = SCHEDULER_NUMBER_OF_MSG_QUEUE;
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_INFO,
		FL("Exit"));
	return QDF_STATUS_SUCCESS;
}

void scheduler_resume(void)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (sched_ctx)
		qdf_event_set(&sched_ctx->resume_sch_event);
}

void scheduler_register_hdd_suspend_callback(hdd_suspend_callback callback)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (sched_ctx)
		sched_ctx->hdd_callback = callback;
}
void scheduler_wake_up_controller_thread(void)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (sched_ctx)
		qdf_wake_up_interruptible(&sched_ctx->sch_wait_queue);
}
void scheduler_set_event_mask(uint32_t event_mask)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (sched_ctx)
		qdf_set_bit(event_mask, &sched_ctx->sch_event_flag);
}

void scheduler_clear_event_mask(uint32_t event_mask)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (sched_ctx)
		qdf_clear_bit(event_mask, &sched_ctx->sch_event_flag);
}

QDF_STATUS scheduler_target_if_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();
	QDF_STATUS (*target_if_msg_handler)(struct scheduler_msg *);

	if (NULL == msg || NULL == sched_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR, FL("msg %p sch %p"),
			msg, sched_ctx);
		return QDF_STATUS_E_FAILURE;
	}

	target_if_msg_handler = msg->callback;

	/* Target_If is a special message queue in phase 3 convergence beacause
	 * its used by both legacy WMA and as well as new UMAC components. New
	 * UMAC components directly pass their message handlers as callback in
	 * message body.
	 * 1) All Legacy WMA messages do not contain message callback so invoke
	 *    registered legacy WMA handler. Scheduler message posting APIs
	 *    makes sure legacy WMA messages do not have callbacks.
	 * 2) For new messages which have valid callbacks invoke their callbacks
	 *    directly.
	 */
	if (NULL == target_if_msg_handler)
		status = sched_ctx->legacy_wma_handler(msg);
	else
		status = target_if_msg_handler(msg);

	return status;
}

QDF_STATUS scheduler_os_if_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS (*os_if_msg_handler)(struct scheduler_msg *);

	if (NULL == msg) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR, FL("Msg is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	os_if_msg_handler = msg->callback;

	if (NULL == os_if_msg_handler) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR, FL("Msg callback is NULL"));
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	os_if_msg_handler(msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_timer_q_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();
	qdf_mc_timer_callback_t timer_q_msg_handler;

	if (NULL == msg || NULL == sched_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR, FL("msg %p sch %p"),
			msg, sched_ctx);
		return QDF_STATUS_E_FAILURE;
	}

	timer_q_msg_handler = msg->callback;

	/* Timer message handler */
	if (SYS_MSG_COOKIE == msg->reserved &&
		SYS_MSG_ID_MC_TIMER == msg->type) {
		if (timer_q_msg_handler) {
			status = QDF_STATUS_SUCCESS;
			timer_q_msg_handler(msg->bodyptr);
		} else {
			QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
				QDF_TRACE_LEVEL_ERROR, FL("Timer cb is null"));
			status = QDF_STATUS_E_FAILURE;
		}
		return status;
	} else {
		/* Legacy sys message handler */
		status = sched_ctx->legacy_sys_handler(msg);
		return status;
	}
}

QDF_STATUS scheduler_register_wma_legacy_handler(scheduler_msg_process_fn_t
						wma_callback)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (NULL == sched_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR, FL("scheduler context is null"));
		return QDF_STATUS_E_FAILURE;
	}

	sched_ctx->legacy_wma_handler = wma_callback;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_register_sys_legacy_handler(scheduler_msg_process_fn_t
						sys_callback)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (NULL == sched_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER,
			QDF_TRACE_LEVEL_ERROR, FL("scheduler context is null"));
		return QDF_STATUS_E_FAILURE;
	}

	sched_ctx->legacy_sys_handler = sys_callback;

	return QDF_STATUS_SUCCESS;
}

void scheduler_mc_timer_callback(unsigned long data)
{
	qdf_mc_timer_t *timer = (qdf_mc_timer_t *)data;
	struct scheduler_msg msg;
	QDF_STATUS status;

	qdf_mc_timer_callback_t callback = NULL;
	void *user_data = NULL;
	QDF_TIMER_TYPE type = QDF_TIMER_TYPE_SW;

	QDF_ASSERT(timer);

	if (timer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s Null pointer passed in!", __func__);
		return;
	}

	qdf_spin_lock_irqsave(&timer->platform_info.spinlock);

	switch (timer->state) {
	case QDF_TIMER_STATE_STARTING:
		/* we are in this state because someone just started the timer,
		 * MC timer got started and expired, but the time content have
		 * not been updated this is a rare race condition!
		 */
		timer->state = QDF_TIMER_STATE_STOPPED;
		status = QDF_STATUS_E_ALREADY;
		break;

	case QDF_TIMER_STATE_STOPPED:
		status = QDF_STATUS_E_ALREADY;
		break;

	case QDF_TIMER_STATE_UNUSED:
		status = QDF_STATUS_E_EXISTS;
		break;

	case QDF_TIMER_STATE_RUNNING:
		/* need to go to stop state here because the call-back function
		 * may restart timer (to emulate periodic timer)
		 */
		timer->state = QDF_TIMER_STATE_STOPPED;
		/* copy the relevant timer information to local variables;
		 * once we exits from this critical section, the timer content
		 * may be modified by other tasks
		 */
		callback = timer->callback;
		user_data = timer->user_data;
		type = timer->type;
		status = QDF_STATUS_SUCCESS;
		break;

	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_FAULT;
		break;
	}

	qdf_spin_unlock_irqrestore(&timer->platform_info.spinlock);

	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "TIMER callback called in a wrong state=%d",
			  timer->state);
		return;
	}

	qdf_try_allowing_sleep(type);

	if (callback == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
			  "%s: No TIMER callback, Couldn't enqueue timer to any queue",
			  __func__);
		QDF_ASSERT(0);
		return;
	}

	/* serialize to scheduler controller thread */
	msg.type = SYS_MSG_ID_MC_TIMER;
	msg.reserved = SYS_MSG_COOKIE;
	msg.callback = callback;
	msg.bodyptr = user_data;
	msg.bodyval = 0;

	if (scheduler_post_msg(QDF_MODULE_ID_SYS, &msg) == QDF_STATUS_SUCCESS)
		return;
	QDF_TRACE(QDF_MODULE_ID_SCHEDULER, QDF_TRACE_LEVEL_ERROR,
		  "%s: Could not enqueue timer to timer queue", __func__);
}
