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

#if !defined(__CDF_EVENT_H)
#define __CDF_EVENT_H

/**
 * DOC: cdf_event.h
 *
 * Connectivity driver framework (CDF) events API
 *
 **/

/* Include Files */
#include "cdf_status.h"
#include "cdf_types.h"
#include "i_cdf_event.h"

/* Preprocessor definitions and constants */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Type declarations */

/* Function declarations and documenation */

/**
 * cdf_event_init() - initializes the specified event
 *
 * @event:	Pointer to CDF event object to initialize
 *
 * Initializes the specified event. Upon successful initialization the state
 * of the event becomes initialized and not signaled.
 *
 * Return:
 *    CDF_STATUS_SUCCESS - Event was successfully initialized and is ready to
 *                         be used
 *    Otherwise failure CDF reason code
 */

CDF_STATUS cdf_event_init(cdf_event_t *event);

/**
 * cdf_event_set() -  set a CDF event
 *
 * @event:	Pointer of CDF event to set to the signalled state
 *
 * The state of the specified event is set to 'signalled by calling
 * cdf_event_set().  The state of the event remains signalled until an
 * explicit call to cdf_event_reset().
 *
 * Any threads waiting on the event as a result of a cdf_event_wait() will
 * be unblocked and available to be scheduled for execution when the event
 * is signaled by a call to cdf_event_set().
 *
 * Return:
 *      CDF_STATUS_SUCCESS - Event was successfully set
 *      Otherwise failure CDF reason code
 */
CDF_STATUS cdf_event_set(cdf_event_t *event);

/**
 * cdf_event_reset() -  reset a CDF event
 *
 * @event:	Pointer of CDF event to reset
 *
 * The state of the specified event is set to 'NOT signalled' by calling
 * cdf_event_reset().  The state of the event remains NOT signalled until an
 * explicit call to cdf_event_set().
 *
 * This function sets the event to a NOT signalled state even if the event was
 * signalled multiple times before being signaled.
 *
 * Return:
 *      CDF_STATUS_SUCCESS - Event was successfully reset
 *      Otherwise failure CDF reason code
 */
CDF_STATUS cdf_event_reset(cdf_event_t *event);

/**
 * cdf_event_destroy() -  destroy a CDF event
 *
 * @event:	Pointer of CDF event to destroy
 *
 * The function destroys the event object referenced by event.
 * After a successful return from cdf_event_destroy() the event object becomes,
 * in effect, uninitialized.
 *
 * A destroyed event object can be reinitialized using cdf_event_init();
 * the results of otherwise referencing the object after it has been destroyed
 * are undefined.  Calls to CDF event functions to manipulate the lock such
 * as cdf_event_set() will fail if the event is destroyed.  Therefore,
 * don't use the event after it has been destroyed until it has
 * been re-initialized.
 *
 * Return:
 *      CDF_STATUS_SUCCESS - Event was successfully destroyed
 *      Otherwise failure CDF reason code
 */
CDF_STATUS cdf_event_destroy(cdf_event_t *event);

/**
 * cdf_wait_single_event() -  wait for a single input CDF event to be set
 *
 * @event:	Pointer of CDF event to wait on
 * @timeout:	Timeout value in milli seconds
 *
 * This API waits for the event to be set. This function returns
 * if this interval elapses, regardless if any of the events have
 * been set.  An input value of 0 for this timeout parameter means
 * to wait infinitely, meaning a timeout will never occur.
 *
 *
 * Return:
 *    CDF_STATUS_SUCCESS - the wait was satisifed by the event being
 *                         set.
 *
 *    CDF_STATUS_E_TIMEOUT - the timeout interval elapsed before the
 *    event was set.
 *
 *    CDF_STATUS_E_INVAL - The value specified by event is invalid.
 */
CDF_STATUS cdf_wait_single_event(cdf_event_t *pEvent,
				 uint32_t timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CDF_EVENT_H */
