/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 /**
  * DOC: qdf_hang_event_notifier
  * This file provides OS dependent QDF notifier call for hang event
  */

#ifndef QDF_HANG_EVENT_NOTIFIER_H
#define QDF_HANG_EVENT_NOTIFIER_H

#include <qdf_notifier.h>

#define QDF_HANG_EVENT_VERSION "1.0"
#define QDF_HANG_EVENT_DATA_SIZE 784

/**
 * qdf_notifier_data - Private data for notifier data
 * @hang_data: Data filled by notifier
 * @offset: Current offset of the hang data buffer
 */
struct qdf_notifer_data {
	uint8_t *hang_data;
	unsigned int offset;
};

#ifdef WLAN_HANG_EVENT
/**
 * qdf_hang_event_register_notifier() - Hang data notifier register
 * @nb: Notifier block
 *
 * This function registers notifier block for the hang data notifier chain
 * the registered function will be invoked when the hang data notifier call
 * is invoked.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_hang_event_register_notifier(qdf_notif_block *nb);

/**
 * qdf_hang_event_unregister_notifier() - Hang data notifier unregister
 * @nb: Notifier block
 *
 * This function unregisters notifier block for the hang data notifier chain.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_hang_event_unregister_notifier(qdf_notif_block *nb);

/**
 * qdf_hang_event_notifier_call() - Hang data notifier register
 * @v: state
 * @data: Private data for this notifier chain
 *
 * This function when invoked will call the functions registered with this
 * notifier chain.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_hang_event_notifier_call(unsigned long v, void *data);
#else
static inline
QDF_STATUS qdf_hang_event_register_notifier(qdf_notif_block *nb)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS qdf_hang_event_unregister_notifier(qdf_notif_block *nb)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS qdf_hang_event_notifier_call(unsigned long v, void *data)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
