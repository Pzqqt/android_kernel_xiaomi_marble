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

#include <scheduler_api.h>
#include <scheduler_core.h>
#include <qdf_atomic.h>
#include <qdf_module.h>
#include <qdf_platform.h>

QDF_STATUS scheduler_disable(void)
{
	struct scheduler_ctx *sched_ctx;

	sched_debug("Disabling Scheduler");

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_INVAL;

	/* send shutdown signal to scheduler thread */
	qdf_atomic_set_bit(MC_SHUTDOWN_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_atomic_set_bit(MC_POST_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_wake_up_interruptible(&sched_ctx->sch_wait_queue);

	/* wait for scheduler thread to shutdown */
	qdf_wait_single_event(&sched_ctx->sch_shutdown, 0);
	sched_ctx->sch_thread = NULL;

	/* flush any unprocessed scheduler messages */
	scheduler_queues_flush(sched_ctx);

	return QDF_STATUS_SUCCESS;
}

static inline void scheduler_watchdog_notify(struct scheduler_ctx *sched)
{
	char symbol[QDF_SYMBOL_LEN];

	if (sched->watchdog_callback)
		qdf_sprint_symbol(symbol, sched->watchdog_callback);

	sched_fatal("Callback %s (type 0x%x) exceeded its allotted time of %ds",
		    sched->watchdog_callback ? symbol : "<null>",
		    sched->watchdog_msg_type,
		    SCHEDULER_WATCHDOG_TIMEOUT / 1000);
}

static void scheduler_watchdog_timeout(void *arg)
{
	struct scheduler_ctx *sched = arg;

	if (qdf_is_recovering()) {
		sched_debug("Recovery is in progress ignore timeout");
		return;
	}

	scheduler_watchdog_notify(sched);
	if (sched->sch_thread)
		qdf_print_thread_trace(sched->sch_thread);

	/* avoid crashing during shutdown */
	if (qdf_atomic_test_bit(MC_SHUTDOWN_EVENT_MASK, &sched->sch_event_flag))
		return;

	QDF_DEBUG_PANIC("Going down for Scheduler Watchdog Bite!");
}

QDF_STATUS scheduler_enable(void)
{
	struct scheduler_ctx *sched_ctx;

	sched_debug("Enabling Scheduler");

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_INVAL;

	qdf_atomic_clear_bit(MC_SHUTDOWN_EVENT_MASK,
			     &sched_ctx->sch_event_flag);
	qdf_atomic_clear_bit(MC_POST_EVENT_MASK,
			     &sched_ctx->sch_event_flag);

	/* create the scheduler thread */
	sched_ctx->sch_thread = qdf_create_thread(scheduler_thread, sched_ctx,
						  "scheduler_thread");
	if (!sched_ctx->sch_thread) {
		sched_fatal("Failed to create scheduler thread");
		return QDF_STATUS_E_RESOURCES;
	}

	sched_debug("Scheduler thread created");

	/* wait for the scheduler thread to startup */
	qdf_wake_up_process(sched_ctx->sch_thread);
	qdf_wait_single_event(&sched_ctx->sch_start_event, 0);

	sched_debug("Scheduler thread started");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_init(void)
{
	QDF_STATUS status;
	struct scheduler_ctx *sched_ctx;

	sched_debug("Initializing Scheduler");

	status = scheduler_create_ctx();
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_fatal("Failed to create context; status:%d", status);
		return status;
	}

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx) {
		status = QDF_STATUS_E_FAILURE;
		goto ctx_destroy;
	}

	status = scheduler_queues_init(sched_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_fatal("Failed to init queues; status:%d", status);
		goto ctx_destroy;
	}

	status = qdf_event_create(&sched_ctx->sch_start_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_fatal("Failed to create start event; status:%d", status);
		goto queues_deinit;
	}

	status = qdf_event_create(&sched_ctx->sch_shutdown);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_fatal("Failed to create shutdown event; status:%d",
			    status);
		goto start_event_destroy;
	}

	status = qdf_event_create(&sched_ctx->resume_sch_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_fatal("Failed to create resume event; status:%d", status);
		goto shutdown_event_destroy;
	}

	qdf_spinlock_create(&sched_ctx->sch_thread_lock);
	qdf_init_waitqueue_head(&sched_ctx->sch_wait_queue);
	sched_ctx->sch_event_flag = 0;
	qdf_timer_init(NULL,
		       &sched_ctx->watchdog_timer,
		       &scheduler_watchdog_timeout,
		       sched_ctx,
		       QDF_TIMER_TYPE_SW);

	qdf_register_mc_timer_callback(scheduler_mc_timer_callback);

	return QDF_STATUS_SUCCESS;

shutdown_event_destroy:
	qdf_event_destroy(&sched_ctx->sch_shutdown);

start_event_destroy:
	qdf_event_destroy(&sched_ctx->sch_start_event);

queues_deinit:
	scheduler_queues_deinit(sched_ctx);

ctx_destroy:
	scheduler_destroy_ctx();

	return status;
}

QDF_STATUS scheduler_deinit(void)
{
	QDF_STATUS status;
	struct scheduler_ctx *sched_ctx;

	sched_debug("Deinitializing Scheduler");

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_INVAL;

	qdf_timer_free(&sched_ctx->watchdog_timer);
	qdf_spinlock_destroy(&sched_ctx->sch_thread_lock);
	qdf_event_destroy(&sched_ctx->resume_sch_event);
	qdf_event_destroy(&sched_ctx->sch_shutdown);
	qdf_event_destroy(&sched_ctx->sch_start_event);

	status = scheduler_queues_deinit(sched_ctx);
	if (QDF_IS_STATUS_ERROR(status))
		sched_err("Failed to deinit queues; status:%d", status);

	status = scheduler_destroy_ctx();
	if (QDF_IS_STATUS_ERROR(status))
		sched_err("Failed to destroy context; status:%d", status);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_post_msg_by_priority(uint32_t qid,
					  struct scheduler_msg *msg,
					  bool is_high_priority)
{
	uint8_t qidx;
	struct scheduler_mq_type *target_mq;
	struct scheduler_msg *queue_msg;
	struct scheduler_ctx *sched_ctx;
	uint16_t src_id;
	uint16_t dest_id;
	uint16_t que_id;

	QDF_BUG(msg);
	if (!msg)
		return QDF_STATUS_E_INVAL;

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_INVAL;

	if (!sched_ctx->sch_thread) {
		sched_err("Cannot post message; scheduler thread is stopped");
		return QDF_STATUS_E_FAILURE;
	}

	if (msg->reserved != 0 && msg->reserved != SYS_MSG_COOKIE) {
		QDF_DEBUG_PANIC("Scheduler messages must be initialized");
		return QDF_STATUS_E_FAILURE;
	}

	dest_id = scheduler_get_dest_id(qid);
	src_id = scheduler_get_src_id(qid);
	que_id = scheduler_get_que_id(qid);

	if (que_id >= QDF_MODULE_ID_MAX || src_id >= QDF_MODULE_ID_MAX ||
	    dest_id >= QDF_MODULE_ID_MAX) {
		sched_err("Src_id/Dest_id invalid, cannot post message");
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
	if (QDF_MODULE_ID_WMA == que_id) {
		msg->callback = NULL;
		/* change legacy WMA message id to new target_if mq id */
		que_id = QDF_MODULE_ID_TARGET_IF;
	}
	qdf_mtrace(src_id, dest_id, msg->type, 0xFF, 0);

	qidx = sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[que_id];
	if (qidx >= SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		sched_err("Scheduler is deinitialized ignore msg");
		return QDF_STATUS_E_FAILURE;
	}

	if (!sched_ctx->queue_ctx.scheduler_msg_process_fn[qidx]) {
		QDF_DEBUG_PANIC("callback not registered for qid[%d]", que_id);
		return QDF_STATUS_E_FAILURE;
	}

	target_mq = &(sched_ctx->queue_ctx.sch_msg_q[qidx]);

	queue_msg = scheduler_core_msg_dup(msg);
	if (!queue_msg)
		return QDF_STATUS_E_NOMEM;

	if (is_high_priority)
		scheduler_mq_put_front(target_mq, queue_msg);
	else
		scheduler_mq_put(target_mq, queue_msg);

	qdf_atomic_set_bit(MC_POST_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_wake_up_interruptible(&sched_ctx->sch_wait_queue);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_register_module(QDF_MODULE_ID qid,
				     scheduler_msg_process_fn_t callback)
{
	struct scheduler_mq_ctx *ctx;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	sched_enter();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	if (sched_ctx->sch_last_qidx >= SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		sched_err("Already registered max %d no of message queues",
			  SCHEDULER_NUMBER_OF_MSG_QUEUE);
		return QDF_STATUS_E_FAILURE;
	}

	ctx = &sched_ctx->queue_ctx;
	ctx->scheduler_msg_qid_to_qidx[qid] = sched_ctx->sch_last_qidx;
	ctx->sch_msg_q[sched_ctx->sch_last_qidx].qid = qid;
	ctx->scheduler_msg_process_fn[sched_ctx->sch_last_qidx] = callback;
	sched_ctx->sch_last_qidx++;

	sched_exit();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deregister_module(QDF_MODULE_ID qid)
{
	struct scheduler_mq_ctx *ctx;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();
	uint8_t qidx;

	sched_enter();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	ctx = &sched_ctx->queue_ctx;
	qidx = ctx->scheduler_msg_qid_to_qidx[qid];
	ctx->scheduler_msg_process_fn[qidx] = NULL;
	sched_ctx->sch_last_qidx--;
	ctx->scheduler_msg_qid_to_qidx[qidx] = SCHEDULER_NUMBER_OF_MSG_QUEUE;

	sched_exit();

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
		qdf_atomic_set_bit(event_mask, &sched_ctx->sch_event_flag);
}

void scheduler_clear_event_mask(uint32_t event_mask)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (sched_ctx)
		qdf_atomic_clear_bit(event_mask, &sched_ctx->sch_event_flag);
}

QDF_STATUS scheduler_target_if_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();
	QDF_STATUS (*target_if_msg_handler)(struct scheduler_msg *);

	QDF_BUG(msg);
	if (!msg)
		return QDF_STATUS_E_FAILURE;

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

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
	if (!target_if_msg_handler)
		status = sched_ctx->legacy_wma_handler(msg);
	else
		status = target_if_msg_handler(msg);

	return status;
}

QDF_STATUS scheduler_os_if_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS (*os_if_msg_handler)(struct scheduler_msg *);

	QDF_BUG(msg);
	if (!msg)
		return QDF_STATUS_E_FAILURE;

	os_if_msg_handler = msg->callback;

	QDF_BUG(os_if_msg_handler);
	if (!os_if_msg_handler)
		return QDF_STATUS_E_FAILURE;

	os_if_msg_handler(msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_timer_q_mq_handler(struct scheduler_msg *msg)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();
	qdf_mc_timer_callback_t timer_callback;

	QDF_BUG(msg);
	if (!msg)
		return QDF_STATUS_E_FAILURE;

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	/* legacy sys message handler? */
	if (msg->reserved != SYS_MSG_COOKIE || msg->type != SYS_MSG_ID_MC_TIMER)
		return sched_ctx->legacy_sys_handler(msg);

	timer_callback = msg->callback;
	QDF_BUG(timer_callback);
	if (!timer_callback)
		return QDF_STATUS_E_FAILURE;

	timer_callback(msg->bodyptr);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_mlme_mq_handler(struct scheduler_msg *msg)
{
	scheduler_msg_process_fn_t mlme_msg_handler;

	QDF_BUG(msg);
	if (!msg)
		return QDF_STATUS_E_FAILURE;

	mlme_msg_handler = msg->callback;

	QDF_BUG(mlme_msg_handler);
	if (!mlme_msg_handler)
		return QDF_STATUS_E_FAILURE;

	mlme_msg_handler(msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_scan_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS (*scan_q_msg_handler)(struct scheduler_msg *);

	QDF_BUG(msg);
	if (!msg)
		return QDF_STATUS_E_FAILURE;

	scan_q_msg_handler = msg->callback;

	QDF_BUG(scan_q_msg_handler);
	if (!scan_q_msg_handler)
		return QDF_STATUS_E_FAILURE;

	scan_q_msg_handler(msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_register_wma_legacy_handler(scheduler_msg_process_fn_t
						wma_callback)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	sched_ctx->legacy_wma_handler = wma_callback;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_register_sys_legacy_handler(scheduler_msg_process_fn_t
						sys_callback)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	sched_ctx->legacy_sys_handler = sys_callback;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deregister_wma_legacy_handler(void)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	sched_ctx->legacy_wma_handler = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deregister_sys_legacy_handler(void)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	QDF_BUG(sched_ctx);
	if (!sched_ctx)
		return QDF_STATUS_E_FAILURE;

	sched_ctx->legacy_sys_handler = NULL;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scheduler_msg_flush_noop(struct scheduler_msg *msg)
{
	return QDF_STATUS_SUCCESS;
}

void scheduler_mc_timer_callback(qdf_mc_timer_t *timer)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	qdf_mc_timer_callback_t callback = NULL;
	void *user_data = NULL;
	QDF_TIMER_TYPE type = QDF_TIMER_TYPE_SW;

	QDF_BUG(timer);
	if (!timer)
		return;

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

	if (QDF_IS_STATUS_ERROR(status)) {
		sched_debug("MC timer fired but is not running; skip callback");
		return;
	}

	qdf_try_allowing_sleep(type);

	QDF_BUG(callback);
	if (!callback)
		return;

	/* serialize to scheduler controller thread */
	msg.type = SYS_MSG_ID_MC_TIMER;
	msg.reserved = SYS_MSG_COOKIE;
	msg.callback = callback;
	msg.bodyptr = user_data;
	msg.bodyval = 0;

	/* bodyptr points to user data, do not free it during msg flush */
	msg.flush_callback = scheduler_msg_flush_noop;

	status = scheduler_post_message(QDF_MODULE_ID_SCHEDULER,
					QDF_MODULE_ID_SCHEDULER,
					QDF_MODULE_ID_SYS, &msg);
	if (QDF_IS_STATUS_ERROR(status))
		sched_err("Could not enqueue timer to timer queue");
}

QDF_STATUS scheduler_get_queue_size(QDF_MODULE_ID qid, uint32_t *size)
{
	uint8_t qidx;
	struct scheduler_mq_type *target_mq;
	struct scheduler_ctx *sched_ctx;

	sched_ctx = scheduler_get_context();
	if (!sched_ctx)
		return QDF_STATUS_E_INVAL;

	/* WMA also uses the target_if queue, so replace the QID */
	if (QDF_MODULE_ID_WMA == qid)
		qid = QDF_MODULE_ID_TARGET_IF;

	qidx = sched_ctx->queue_ctx.scheduler_msg_qid_to_qidx[qid];
	if (qidx >= SCHEDULER_NUMBER_OF_MSG_QUEUE) {
		sched_err("Scheduler is deinitialized");
		return QDF_STATUS_E_FAILURE;
	}

	target_mq = &(sched_ctx->queue_ctx.sch_msg_q[qidx]);

	*size = qdf_list_size(&target_mq->mq_list);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_post_message_debug(QDF_MODULE_ID src_id,
					QDF_MODULE_ID dest_id,
					QDF_MODULE_ID que_id,
					struct scheduler_msg *msg,
					int line,
					const char *func)
{
	QDF_STATUS status;

	status = scheduler_post_msg(scheduler_get_qid(src_id, dest_id, que_id),
				    msg);

	if (QDF_IS_STATUS_ERROR(status))
		sched_err("couldn't post from %d to %d - called from %d, %s",
			  src_id, dest_id, line, func);

	return status;
}

qdf_export_symbol(scheduler_post_message_debug);
