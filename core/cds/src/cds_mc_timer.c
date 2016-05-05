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
 * DOC: cds_mc_timer.c
 * Connectivity driver services timer APIs
 */

#include <qdf_mc_timer.h>
#include "cds_mc_timer.h"
#include <wlan_qct_sys.h>
#include <qdf_trace.h>
#include "cds_mq.h"

/**
 * cds_linux_timer_callback() - timer callback, gets called at time out.
 * @data: unsigned long, holds the timer object.
 *
 * Return: None
 */
void cds_linux_timer_callback(unsigned long data)
{
	qdf_mc_timer_t *timer = (qdf_mc_timer_t *)data;
	cds_msg_t msg;
	QDF_STATUS status;

	qdf_mc_timer_callback_t callback = NULL;
	void *user_data = NULL;
	QDF_TIMER_TYPE type = QDF_TIMER_TYPE_SW;

	QDF_ASSERT(timer);

	if (timer == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
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
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "TIMER callback called in a wrong state=%d",
			  timer->state);
		return;
	}

	qdf_try_allowing_sleep(type);

	if (callback == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: No TIMER callback, Could not enqueue timer to any queue",
			  __func__);
		QDF_ASSERT(0);
		return;
	}

	/* serialize to the MC thread */
	sys_build_message_header(SYS_MSG_ID_MC_TIMER, &msg);
	msg.callback = callback;
	msg.bodyptr = user_data;
	msg.bodyval = 0;

	if (cds_mq_post_message(CDS_MQ_ID_SYS, &msg) == QDF_STATUS_SUCCESS)
		return;
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "%s: Could not enqueue timer to any queue", __func__);
	QDF_ASSERT(0);
}
