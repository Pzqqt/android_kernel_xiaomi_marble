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

/* Preprocessor definitions and constants */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef __qdf_event_t qdf_event_t;
/* Function declarations and documenation */

QDF_STATUS qdf_event_create(qdf_event_t *event);

QDF_STATUS qdf_event_set(qdf_event_t *event);

QDF_STATUS qdf_event_reset(qdf_event_t *event);

QDF_STATUS qdf_event_destroy(qdf_event_t *event);

QDF_STATUS qdf_wait_single_event(qdf_event_t *event,
				 uint32_t timeout);
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
