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
 * DOC: qdf_event.c
 *
 * This source file contains linux specific definitions for QDF event APIs
 * The APIs mentioned in this file are used for initializing, setting,
 * resetting, destroying an event and waiting on an occurrence of an event
 * among multiple events.
 */

/* Include Files */
#include "qdf_event.h"
#include "qdf_mc_timer.h"
#include "qdf_timer.h"
#include <qdf_module.h>

struct qdf_evt_node {
	qdf_list_node_t node;
	qdf_event_t *pevent;
};

#define MAX_WAIT_EVENTS 10

static qdf_list_t qdf_wait_event_list;
static qdf_spinlock_t qdf_wait_event_lock;

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
	QDF_BUG(event);
	if (!event)
		return QDF_STATUS_E_FAULT;

	/* check for 'already initialized' event */
	QDF_BUG(event->cookie != LINUX_EVENT_COOKIE);
	if (event->cookie == LINUX_EVENT_COOKIE)
		return QDF_STATUS_E_BUSY;

	/* initialize new event */
	init_completion(&event->complete);
	event->cookie = LINUX_EVENT_COOKIE;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_event_create);

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
	QDF_BUG(event);
	if (!event)
		return QDF_STATUS_E_FAULT;

	/* ensure event is initialized */
	QDF_BUG(event->cookie == LINUX_EVENT_COOKIE);
	if (event->cookie != LINUX_EVENT_COOKIE)
		return QDF_STATUS_E_INVAL;

	complete(&event->complete);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_event_set);

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
	QDF_BUG(event);
	if (!event)
		return QDF_STATUS_E_FAULT;

	/* ensure event is initialized */
	QDF_BUG(event->cookie == LINUX_EVENT_COOKIE);
	if (event->cookie != LINUX_EVENT_COOKIE)
		return QDF_STATUS_E_INVAL;

	/* (re)initialize event */
	event->force_set = false;
	INIT_COMPLETION(event->complete);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_event_reset);

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
	QDF_BUG(event);
	if (!event)
		return QDF_STATUS_E_FAULT;

	/* ensure event is initialized */
	QDF_BUG(event->cookie == LINUX_EVENT_COOKIE);
	if (event->cookie != LINUX_EVENT_COOKIE)
		return QDF_STATUS_E_INVAL;

	/* make sure nobody is waiting on the event */
	complete_all(&event->complete);

	/* destroy the event */
	memset(event, 0, sizeof(qdf_event_t));

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_event_destroy);

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
	QDF_BUG(!in_interrupt());
	if (in_interrupt())
		return QDF_STATUS_E_FAULT;

	QDF_BUG(event);
	if (!event)
		return QDF_STATUS_E_FAULT;

	/* ensure event is initialized */
	QDF_BUG(event->cookie == LINUX_EVENT_COOKIE);
	if (event->cookie != LINUX_EVENT_COOKIE)
		return QDF_STATUS_E_INVAL;

	if (timeout) {
		long ret;

		ret = wait_for_completion_timeout(
				&event->complete,
				__qdf_scaled_msecs_to_jiffies(timeout));

		if (ret <= 0)
			return QDF_STATUS_E_TIMEOUT;
	} else {
		wait_for_completion(&event->complete);
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_wait_single_event);

/**
 * qdf_complete_wait_events() - Sets all the events which are in the list.
 *
 * This function traverses the list of events and sets all of them. It
 * sets the flag force_set as TRUE to indicate that these events have
 * been forcefully set.
 *
 * Return: None
 */
void qdf_complete_wait_events(void)
{
	struct qdf_evt_node *event_node = NULL;
	qdf_list_node_t *list_node = NULL;
	QDF_STATUS status;

	if (qdf_list_empty(&qdf_wait_event_list))
		return;

	qdf_spin_lock(&qdf_wait_event_lock);
	qdf_list_peek_front(&qdf_wait_event_list,
			    &list_node);

	while (list_node) {
		event_node = qdf_container_of(list_node,
						struct qdf_evt_node, node);

		event_node->pevent->force_set = true;
		qdf_event_set(event_node->pevent);

		status = qdf_list_peek_next(&qdf_wait_event_list,
					&event_node->node, &list_node);

		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
	}
	qdf_spin_unlock(&qdf_wait_event_lock);
}
qdf_export_symbol(qdf_complete_wait_events);

/**
 * qdf_wait_for_event_completion() - Waits for an event to be set.
 *
 * @event: Pointer to an event to wait on.
 * @timeout: Timeout value (in milliseconds).
 *
 * This function adds the event in a list and waits on it until it
 * is set or the timeout duration elapses. The purpose of waiting
 * is considered complete only if the event is set and the flag
 * force_set is FALSE, it returns success in this case. In other
 * cases it returns appropriate error status.
 *
 * Return: QDF status
 */
QDF_STATUS qdf_wait_for_event_completion(qdf_event_t *event, uint32_t timeout)
{
	struct qdf_evt_node *event_node;
	QDF_STATUS status;

	QDF_BUG(!in_interrupt());
	if (in_interrupt())
		return QDF_STATUS_E_FAULT;

	QDF_BUG(event);
	if (!event)
		return QDF_STATUS_E_FAULT;

	/* ensure event is initialized */
	QDF_BUG(event->cookie == LINUX_EVENT_COOKIE);
	if (event->cookie != LINUX_EVENT_COOKIE)
		return QDF_STATUS_E_INVAL;

	event_node = qdf_mem_malloc(sizeof(*event_node));
	if (!event_node)
		return QDF_STATUS_E_NOMEM;

	event_node->pevent = event;

	qdf_spin_lock(&qdf_wait_event_lock);
	status = qdf_list_insert_back(&qdf_wait_event_list, &event_node->node);
	qdf_spin_unlock(&qdf_wait_event_lock);

	if (QDF_STATUS_SUCCESS != status) {
		qdf_err("Failed to insert event into tracking list");
		goto free_node;
	}

	if (timeout) {
		long ret;

		/* update the timeout if it's on an emulation platform */
		timeout *= qdf_timer_get_multiplier();
		ret = wait_for_completion_timeout(&event->complete,
						  msecs_to_jiffies(timeout));

		if (ret <= 0) {
			status = QDF_STATUS_E_TIMEOUT;
			goto list_remove;
		}
	} else {
		wait_for_completion(&event->complete);
	}

	/* if event was forcefully completed, return failure */
	if (event->force_set)
		status = QDF_STATUS_E_FAULT;

list_remove:
	qdf_spin_lock(&qdf_wait_event_lock);
	qdf_list_remove_node(&qdf_wait_event_list, &event_node->node);
	qdf_spin_unlock(&qdf_wait_event_lock);

free_node:
	qdf_mem_free(event_node);

	return status;
}
qdf_export_symbol(qdf_wait_for_event_completion);

/**
 * qdf_event_list_init() - Creates a list and spinlock for events.
 *
 * This function creates a list for maintaining events on which threads
 * wait for completion. A spinlock is also created to protect related
 * oprations.
 *
 * Return: None
 */
void qdf_event_list_init(void)
{
	qdf_list_create(&qdf_wait_event_list, MAX_WAIT_EVENTS);
	qdf_spinlock_create(&qdf_wait_event_lock);
}
qdf_export_symbol(qdf_event_list_init);

/**
 * qdf_event_list_destroy() - Destroys list and spinlock created for events.
 *
 * This function destroys the list and spinlock created for events on which
 * threads wait for completion.
 *
 * Return: None
 */
void qdf_event_list_destroy(void)
{
	qdf_list_destroy(&qdf_wait_event_list);
	qdf_spinlock_destroy(&qdf_wait_event_lock);
}
qdf_export_symbol(qdf_event_list_destroy);

QDF_STATUS qdf_exit_thread(QDF_STATUS status)
{
	if (status == QDF_STATUS_SUCCESS)
		do_exit(0);
	else
		do_exit(SIGKILL);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_exit_thread);
