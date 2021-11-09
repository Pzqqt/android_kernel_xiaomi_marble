/*
 * Copyright (c) 2014-2018, 2021 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_event.h
 * This file provides OS abstraction for event APIs.
 */

#if !defined(__QDF_EVENT_H)
#define __QDF_EVENT_H

/* Include Files */
#include "qdf_status.h"
#include <qdf_types.h>
#include <i_qdf_event.h>
#include <qdf_trace.h>
#include <qdf_list.h>

/* Preprocessor definitions and constants */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef __qdf_event_t qdf_event_t;
/* Function declarations and documenation */

QDF_STATUS qdf_event_create(qdf_event_t *event);

/**
 * qdf_event_set() - sets a QDF event for single waiting threads
 * @event: The event to set to the signalled state
 *
 * The state of the specified event is set to signalled by calling
 * qdf_event_set().
 *
 * Single thread waiting on the event as a result of a qdf_event_wait() will
 * be unblocked and available to be scheduled for execution when the event
 * is signaled by a call to qdf_event_set().
 *
 * Return: QDF status
 */
QDF_STATUS qdf_event_set(qdf_event_t *event);

/**
 * qdf_event_set_all() - sets a QDF event for all waiting threads
 * @event: The event to set to the signalled state
 *
 * The state of the specified event is set to signalled by calling
 * qdf_event_set_all().
 *
 * Any threads waiting on the event as a result of a qdf_event_wait() will
 * be unblocked and available to be scheduled for execution when the event
 * is signaled by a call to qdf_event_set().
 *
 * Return: QDF status
 */
QDF_STATUS qdf_event_set_all(qdf_event_t *event);

QDF_STATUS qdf_event_reset(qdf_event_t *event);

QDF_STATUS qdf_event_destroy(qdf_event_t *event);

QDF_STATUS qdf_wait_single_event(qdf_event_t *event,
				 uint32_t timeout);

/**
 * qdf_complete_wait_events() - Sets all the events which are in the list.
 *
 * This function traverses the list of events and sets all of them. It
 * sets the flag force_set as TRUE to indicate that these events have
 * been forcefully set.
 *
 * Return: None
 */
void qdf_complete_wait_events(void);

/**
 * qdf_wait_for_event_completion() - Waits for an event to be set.
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
QDF_STATUS qdf_wait_for_event_completion(qdf_event_t *event,
				 uint32_t timeout);

/**
 * qdf_event_list_init() - Creates a list and spinlock for events.
 *
 * This function creates a list for maintaining events on which threads
 * wait for completion. A spinlock is also created to protect related
 * operations.
 *
 * Return: None
 */
void qdf_event_list_init(void);

/**
 * qdf_event_list_destroy() - Destroys list and spinlock created for events.
 *
 * This function destroys the list and spinlock created for events on which
 * threads wait for completion.
 *
 * Return: None
 */
void qdf_event_list_destroy(void);

/**
 * qdf_exit_thread() - exit thread execution
 * @status: QDF status
 *
 * Return: QDF status
 */
QDF_STATUS qdf_exit_thread(QDF_STATUS status);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __QDF_EVENT_H */
