/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

/* Debug variable to detect if controller thread is stuck */
static qdf_atomic_t scheduler_msg_post_fail_count;

static void scheduler_flush_mqs(struct scheduler_ctx *sched_ctx)
{
	int i;

	/* Here each of the MC thread MQ shall be drained and returned to the
	 * Core. Before returning a wrapper to the Core, the Scheduler message
	 * shall be freed first
	 */
	sched_info("Flushing scheduler message queue");

	QDF_ASSERT(sched_ctx);
	if (!sched_ctx) {
		sched_err("sched_ctx is NULL");
		return;
	}
	for (i = 0; i < SCHEDULER_NUMBER_OF_MSG_QUEUE; i++)
		scheduler_cleanup_queues(sched_ctx, i);
}

QDF_STATUS scheduler_disable(void)
{
	struct scheduler_ctx *sched_ctx;

	sched_info("Disabling Scheduler");

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx) {
		sched_err("sched_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* send shutdown signal to scheduler thread */
	qdf_atomic_set_bit(MC_SHUTDOWN_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_atomic_set_bit(MC_POST_EVENT_MASK, &sched_ctx->sch_event_flag);
	qdf_wake_up_interruptible(&sched_ctx->sch_wait_queue);

	/* wait for scheduler thread to shutdown */
	qdf_wait_single_event(&sched_ctx->sch_shutdown, 0);
	sched_ctx->sch_thread = NULL;

	/* flush any unprocessed scheduler messages */
	scheduler_flush_mqs(sched_ctx);

	return QDF_STATUS_SUCCESS;
}

static inline void scheduler_watchdog_notify(struct scheduler_ctx *sched)
{
	char symbol[QDF_SYMBOL_LEN];

	if (sched->watchdog_callback)
		qdf_sprint_symbol(symbol, sched->watchdog_callback);

	sched_err("WLAN_BUG_RCA: Callback %s (type 0x%x) exceeded its allotted time of %ds",
		  sched->watchdog_callback ? symbol : "<null>",
		  sched->watchdog_msg_type, SCHEDULER_WATCHDOG_TIMEOUT / 1000);
}

#ifdef CONFIG_SLUB_DEBUG_ON
static void scheduler_watchdog_timeout(void *arg)
{
	struct scheduler_ctx *sched = arg;

	scheduler_watchdog_notify(sched);
	if (sched->sch_thread)
		qdf_print_thread_trace(sched->sch_thread);

	/* avoid crashing during shutdown */
	if (qdf_atomic_test_bit(MC_SHUTDOWN_EVENT_MASK, &sched->sch_event_flag))
		return;

	sched_fatal("Going down for Scheduler Watchdog Bite!");
	QDF_BUG(0);
}
#else
static void scheduler_watchdog_timeout(void *arg)
{
	scheduler_watchdog_notify((struct scheduler_ctx *)arg);
}
#endif

QDF_STATUS scheduler_enable(void)
{
	struct scheduler_ctx *sched_ctx;

	sched_info("Enabling Scheduler");

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx) {
		sched_err("sched_ctx is null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_atomic_clear_bit(MC_SHUTDOWN_EVENT_MASK,
			     &sched_ctx->sch_event_flag);
	qdf_atomic_clear_bit(MC_POST_EVENT_MASK,
			     &sched_ctx->sch_event_flag);

	/* create the scheduler thread */
	sched_ctx->sch_thread = qdf_create_thread(scheduler_thread, sched_ctx,
						  "scheduler_thread");
	if (IS_ERR(sched_ctx->sch_thread)) {
		sched_err("Failed to create scheduler thread");
		return QDF_STATUS_E_RESOURCES;
	}

	sched_info("Scheduler thread created");

	/* wait for the scheduler thread to startup */
	qdf_wake_up_process(sched_ctx->sch_thread);
	qdf_wait_single_event(&sched_ctx->sch_start_event, 0);

	sched_info("Scheduler thread started");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_init(void)
{
	QDF_STATUS status;
	struct scheduler_ctx *sched_ctx;

	sched_info("Initializing Scheduler");

	status = scheduler_create_ctx();
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_err("Failed to create context; status:%d", status);
		return status;
	}

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx) {
		sched_err("sched_ctx is null");
		status = QDF_STATUS_E_FAILURE;
		goto ctx_destroy;
	}

	status = scheduler_queues_init(sched_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_err("Failed to init queues; status:%d", status);
		goto ctx_destroy;
	}

	status = qdf_event_create(&sched_ctx->sch_start_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_err("Failed to create start event; status:%d", status);
		goto queues_deinit;
	}

	status = qdf_event_create(&sched_ctx->sch_shutdown);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_err("Failed to create shutdown event; status:%d", status);
		goto start_event_destroy;
	}

	status = qdf_event_create(&sched_ctx->resume_sch_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		sched_err("Failed to create resume event; status:%d", status);
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

	sched_info("Deinitializing Scheduler");

	sched_ctx = scheduler_get_context();
	QDF_BUG(sched_ctx);
	if (!sched_ctx) {
		sched_err("sched_ctx is null");
		return QDF_STATUS_E_INVAL;
	}

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

QDF_STATUS scheduler_post_msg_by_priority(QDF_MODULE_ID qid,
		struct scheduler_msg *pMsg, bool is_high_priority)
{
	uint8_t qidx;
	uint32_t msg_wrapper_fail_count;
	struct scheduler_mq_type *target_mq = NULL;
	struct scheduler_msg_wrapper *msg_wrapper = NULL;
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (!pMsg) {
		sched_err("pMsg is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!sched_ctx) {
		sched_err("sched_ctx is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!sched_ctx->sch_thread) {
		sched_err("Cannot post message; scheduler thread is stopped");
		return QDF_STATUS_E_FAILURE;
	}

	if ((0 != pMsg->reserved) && (SYS_MSG_COOKIE != pMsg->reserved)) {
		sched_err("Un-initialized message pointer.. please initialize it");
		QDF_BUG(0);
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
		sched_err("Scheduler is deinitialized ignore msg");
		return QDF_STATUS_E_FAILURE;
	}

	if (!sched_ctx->queue_ctx.scheduler_msg_process_fn[qidx]) {
		sched_err("callback not registered for qid[%d]", qid);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	target_mq = &(sched_ctx->queue_ctx.sch_msg_q[qidx]);
	QDF_ASSERT(target_mq);
	if (target_mq == NULL) {
		sched_err("target_mq == NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* Try and get a free Msg wrapper */
	msg_wrapper = scheduler_mq_get(&sched_ctx->queue_ctx.free_msg_q);
	if (NULL == msg_wrapper) {
		msg_wrapper_fail_count =
			qdf_atomic_inc_return(&scheduler_msg_post_fail_count);
		/* log only 1st failure to avoid over running log buffer */
		if (msg_wrapper_fail_count == 1)
			sched_err("Scheduler message wrapper empty");

		if (SCHEDULER_WRAPPER_MAX_FAIL_COUNT == msg_wrapper_fail_count)
			QDF_BUG(0);

		return QDF_STATUS_E_RESOURCES;
	}
	qdf_atomic_set(&scheduler_msg_post_fail_count, 0);

	/* Copy the message now */
	qdf_mem_copy((void *)msg_wrapper->msg_buf,
			(void *)pMsg, sizeof(struct scheduler_msg));

	if (is_high_priority)
		scheduler_mq_put_front(target_mq, msg_wrapper);
	else
		scheduler_mq_put(target_mq, msg_wrapper);

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

	if (!sched_ctx) {
		QDF_ASSERT(0);
		sched_err("sched_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}

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

	if (!sched_ctx) {
		QDF_ASSERT(0);
		sched_err("sched_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}

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

	if (NULL == msg || NULL == sched_ctx) {
		sched_err("msg %pK sch %pK", msg, sched_ctx);
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
		sched_err("Msg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	os_if_msg_handler = msg->callback;

	if (NULL == os_if_msg_handler) {
		sched_err("Msg callback is NULL");
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
		sched_err("msg %pK sch %pK", msg, sched_ctx);
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
			sched_err("Timer cb is null");
			status = QDF_STATUS_E_FAILURE;
		}

		return status;
	} else {
		/* Legacy sys message handler */
		status = sched_ctx->legacy_sys_handler(msg);

		return status;
	}
}

QDF_STATUS scheduler_scan_mq_handler(struct scheduler_msg *msg)
{
	QDF_STATUS (*scan_q_msg_handler)(struct scheduler_msg *);

	if (NULL == msg) {
		sched_err("Msg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	scan_q_msg_handler = msg->callback;

	if (NULL == scan_q_msg_handler) {
		sched_err("Msg callback is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	scan_q_msg_handler(msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_register_wma_legacy_handler(scheduler_msg_process_fn_t
						wma_callback)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (NULL == sched_ctx) {
		sched_err("scheduler context is null");
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
		sched_err("scheduler context is null");
		return QDF_STATUS_E_FAILURE;
	}

	sched_ctx->legacy_sys_handler = sys_callback;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deregister_wma_legacy_handler(void)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (NULL == sched_ctx) {
		sched_err("scheduler context is null");
		return QDF_STATUS_E_FAILURE;
	}

	sched_ctx->legacy_wma_handler = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scheduler_deregister_sys_legacy_handler(void)
{
	struct scheduler_ctx *sched_ctx = scheduler_get_context();

	if (NULL == sched_ctx) {
		sched_err("scheduler context is null");
		return QDF_STATUS_E_FAILURE;
	}

	sched_ctx->legacy_sys_handler = NULL;

	return QDF_STATUS_SUCCESS;
}

void scheduler_mc_timer_callback(unsigned long data)
{
	qdf_mc_timer_t *timer = (qdf_mc_timer_t *)data;
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	qdf_mc_timer_callback_t callback = NULL;
	void *user_data = NULL;
	QDF_TIMER_TYPE type = QDF_TIMER_TYPE_SW;

	QDF_ASSERT(timer);

	if (timer == NULL) {
		sched_err("Null pointer passed in!");
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
		sched_err("TIMER callback called in a wrong state=%d",
			  timer->state);
		return;
	}

	qdf_try_allowing_sleep(type);

	if (callback == NULL) {
		sched_err("No TIMER callback, Couldn't enqueue timer to any queue");
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
	sched_err("Could not enqueue timer to timer queue");
}
