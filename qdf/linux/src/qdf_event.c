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
 * DOC: qdf_event.c
 *
 * This source file contains linux specific definitions for QDF event APIs
 * The APIs mentioned in this file are used for initializing, setting,
 * resetting, destroying an event and waiting on an occurance of an event
 * among multiple events.
 */

/* Include Files */
#include "qdf_event.h"
#include <linux/export.h>

/* Function Definitions and Documentation */

/**
 * qdf_event_create() - initializes a QDF event
 * @event: Pointer to the opaque event object to initialize
 *
 * The qdf_event_create() function initializes the specified event. Upon
 * successful initialization, the state of the event becomes initialized
 * and not signalled.
 *
 * An event must be initialized before it may be used in any other event
 * functions.
 * Attempting to initialize an already initialized event results in
 * a failure.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_event_create(qdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "NULL event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check for 'already initialized' event */
	if (LINUX_EVENT_COOKIE == event->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Initialized event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_BUSY;
	}

	/* initialize new event */
	init_completion(&event->complete);
	event->cookie = LINUX_EVENT_COOKIE;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_event_create);

/**
 * qdf_event_set() - sets a QDF event
 * @event: The event to set to the signalled state
 *
 * The state of the specified event is set to signalled by calling
 * qdf_event_set().
 *
 * Any threads waiting on the event as a result of a qdf_event_wait() will
 * be unblocked and available to be scheduled for execution when the event
 * is signaled by a call to qdf_event_set().
 *
 * Return: QDF status
 */
QDF_STATUS qdf_event_set(qdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "NULL event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check if event refers to an initialized object */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Uninitialized event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	complete(&event->complete);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_event_set);

/**
 * qdf_event_reset() - resets a QDF event
 * @event: The event to set to the NOT signalled state
 *
 * This function isn't required for Linux. Therefore, it doesn't do much.
 *
 * The state of the specified event is set to 'NOT signalled' by calling
 * qdf_event_reset().  The state of the event remains NOT signalled until an
 * explicit call to qdf_event_set().
 *
 * This function sets the event to a NOT signalled state even if the event was
 * signalled multiple times before being signaled.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_event_reset(qdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "NULL event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check to make sure it is an 'already initialized' event */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Uninitialized event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	/* (re)initialize event */
	INIT_COMPLETION(event->complete);
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_event_reset);

/**
 * qdf_event_destroy() - Destroys a QDF event
 * @event: The event object to be destroyed.
 *
 * This function doesn't do much in Linux. There is no need for the caller
 * to explicitly destroy an event after use.
 *
 * The os_event_destroy() function shall destroy the event object
 * referenced by event.  After a successful return from qdf_event_destroy()
 * the event object becomes, in effect, uninitialized.
 *
 * A destroyed event object can be reinitialized using qdf_event_create();
 * the results of otherwise referencing the object after it has been destroyed
 * are undefined.  Calls to QDF event functions to manipulate the lock such
 * as qdf_event_set() will fail if the event is destroyed.  Therefore,
 * don't use the event after it has been destroyed until it has
 * been re-initialized.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_event_destroy(qdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "NULL event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check to make sure it is an 'already initialized' event */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Uninitialized event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	/* make sure nobody is waiting on the event */
	complete_all(&event->complete);

	/* destroy the event */
	memset(event, 0, sizeof(qdf_event_t));

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_event_destroy);

/**
 * qdf_wait_single_event() - Waits for a single event to be set.
 * This API waits for the event to be set.
 *
 * @event: Pointer to an event to wait on.
 * @timeout: Timeout value (in milliseconds).  This function returns
 * if this interval elapses, regardless if any of the events have
 * been set.  An input value of 0 for this timeout parameter means
 * to wait infinitely, meaning a timeout will never occur.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_wait_single_event(qdf_event_t *event, uint32_t timeout)
{
	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check for null pointer */
	if (NULL == event) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "NULL event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check if cookie is same as that of initialized event */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Uninitialized event passed into %s", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	if (timeout) {
		long ret;
		ret = wait_for_completion_timeout(&event->complete,
						  msecs_to_jiffies(timeout));
		if (0 >= ret)
			return QDF_STATUS_E_TIMEOUT;
	} else {
		wait_for_completion(&event->complete);
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
			  "Signaled for completion %s", __func__);
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_wait_single_event);
