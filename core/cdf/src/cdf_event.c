/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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
 * DOC: cdf_event.c
 *
 * This source file contains linux specific definitions for CDF event APIs
 * The APIs mentioned in this file are used for initializing, setting,
 * resetting, destroying an event and waiting on an occurance of an event
 * among multiple events.
 */

/* Include Files */
#include "cdf_event.h"
#include "cdf_trace.h"

/* Preprocessor Definitions and Constants */

/* Type Declarations */

/* Global Data Definitions */

/* Static Variable Definitions */

/* Function Definitions and Documentation */

/**
 * cdf_event_init() - initializes a CDF event
 * @event: Pointer to the opaque event object to initialize
 *
 * The cdf_event_init() function initializes the specified event. Upon
 * successful initialization, the state of the event becomes initialized
 * and not signaled.
 *
 * An event must be initialized before it may be used in any other event
 * functions.
 *
 * Attempting to initialize an already initialized event results in
 * a failure.
 *
 * Return: CDF status
 */
CDF_STATUS cdf_event_init(cdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "NULL event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check for 'already initialized' event */
	if (LINUX_EVENT_COOKIE == event->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "Initialized event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_BUSY;
	}

	/* initialize new event */
	init_completion(&event->complete);
	event->cookie = LINUX_EVENT_COOKIE;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_event_set() - sets a CDF event
 * @event: The event to set to the signalled state
 *
 * The state of the specified event is set to signalled by calling
 * cdf_event_set().
 *
 * Any threads waiting on the event as a result of a cdf_event_wait() will
 * be unblocked and available to be scheduled for execution when the event
 * is signaled by a call to cdf_event_set().
 *
 *
 * Return: CDF status
 */

CDF_STATUS cdf_event_set(cdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "NULL event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check if event refers to an initialized object */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "Uninitialized event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_INVAL;
	}

	complete(&event->complete);

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_event_reset() - resets a CDF event
 * @event: The event to set to the NOT signalled state
 *
 * This function isn't required for Linux. Therefore, it doesn't do much.
 *
 * The state of the specified event is set to 'NOT signalled' by calling
 * cdf_event_reset().  The state of the event remains NOT signalled until an
 * explicit call to cdf_event_set().
 *
 * This function sets the event to a NOT signalled state even if the event was
 * signalled multiple times before being signaled.
 *
 *
 * Return: CDF status
 */
CDF_STATUS cdf_event_reset(cdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "NULL event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check to make sure it is an 'already initialized' event */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "Uninitialized event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_INVAL;
	}

	/* (re)initialize event */
	INIT_COMPLETION(event->complete);
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_event_destroy() - Destroys a CDF event
 * @event: The event object to be destroyed.
 *
 * This function doesn't do much in Linux. There is no need for the caller
 * to explicitly destroy an event after use.
 *
 * The os_event_destroy() function shall destroy the event object
 * referenced by event.  After a successful return from cdf_event_destroy()
 * the event object becomes, in effect, uninitialized.
 *
 * A destroyed event object can be reinitialized using cdf_event_init();
 * the results of otherwise referencing the object after it has been destroyed
 * are undefined.  Calls to CDF event functions to manipulate the lock such
 * as cdf_event_set() will fail if the event is destroyed.  Therefore,
 * don't use the event after it has been destroyed until it has
 * been re-initialized.
 *
 * Return: CDF status
 */

CDF_STATUS cdf_event_destroy(cdf_event_t *event)
{
	/* check for null pointer */
	if (NULL == event) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "NULL event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check to make sure it is an 'already initialized' event */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "Uninitialized event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_INVAL;
	}

	/* make sure nobody is waiting on the event */
	complete_all(&event->complete);

	/* destroy the event */
	memset(event, 0, sizeof(cdf_event_t));

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_wait_single_event() - Waits for a single event to be set.
 *
 * This API waits for the event to be set.
 *
 * @pEvent: Pointer to an event to wait on.
 * @timeout: Timeout value (in milliseconds).  This function returns
 *	if this interval elapses, regardless if any of the events have
 *	been set.  An input value of 0 for this timeout parameter means
 *	to wait infinitely, meaning a timeout will never occur.
 *
 * Return: CDF status
 */
CDF_STATUS cdf_wait_single_event(cdf_event_t *event, uint32_t timeout)
{
	if (in_interrupt()) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check for null pointer */
	if (NULL == event) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "NULL event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check if cookie is same as that of initialized event */
	if (LINUX_EVENT_COOKIE != event->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "Uninitialized event passed into %s", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_INVAL;
	}

	if (timeout) {
		long ret;
		ret = wait_for_completion_timeout(&event->complete,
						  msecs_to_jiffies(timeout));
		if (0 >= ret)
			return CDF_STATUS_E_TIMEOUT;
	} else {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "Zero timeout value passed into %s", __func__);
		return CDF_STATUS_E_FAULT;
	}

	return CDF_STATUS_SUCCESS;
}
