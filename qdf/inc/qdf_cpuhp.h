/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_cpuhp (CPU hotplug)
 * QCA driver framework (QDF) CPU hotplug APIs
 */

#ifndef __QDF_CPUHP_H
#define __QDF_CPUHP_H

#include "qdf_status.h"
#include "qdf_types.h"

/**
 * struct qdf_cpuhp_handler - an opaque hotplug event registration handle
 */
struct qdf_cpuhp_handler;

typedef void (*qdf_cpuhp_callback)(void *context, uint32_t cpu);

#ifdef QCA_CONFIG_SMP
/**
 * qdf_cpuhp_init() - Initialize the CPU hotplug event infrastructure
 *
 * To be called once, globally.
 *
 * Return: None
 */
QDF_STATUS qdf_cpuhp_init(void);

/**
 * qdf_cpuhp_deinit() - De-initialize the CPU hotplug event infrastructure
 *
 * To be called once, globally.
 *
 * Return: None
 */
QDF_STATUS qdf_cpuhp_deinit(void);

/**
 * qdf_cpuhp_register() - Register for CPU up/down event notifications
 * @handler: a double pointer to the event registration handle to allocate
 * @context: an opaque context to pass back to event listeners
 * @up_callback: the function pointer to invoke for CPU up events
 * @down_callback: the function pointer to invoke for CPU down events
 *
 * "Up" happens just after the CPU is up. Inversely, "down" happens just before
 * the CPU goes down.
 *
 * @handler will point to a valid memory address on success, or NULL on failure.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_cpuhp_register(struct qdf_cpuhp_handler **handler,
			      void *context,
			      qdf_cpuhp_callback up_callback,
			      qdf_cpuhp_callback down_callback);

/**
 * qdf_cpuhp_unregister() - Un-register for CPU up/down event notifications
 * @handler: a double pointer to the event registration handle to de-allocate
 *
 * @handler point to NULL upon completion
 *
 * Return: None
 */
void qdf_cpuhp_unregister(struct qdf_cpuhp_handler **handler);
#else
QDF_STATUS qdf_cpuhp_init(void)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_cpuhp_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_cpuhp_register(struct qdf_cpuhp_handler **handler,
			      void *context,
			      qdf_cpuhp_callback up_callback,
			      qdf_cpuhp_callback down_callback)
{
	return QDF_STATUS_SUCCESS;
}

void qdf_cpuhp_unregister(struct qdf_cpuhp_handler **handler) {}
#endif /* QCA_CONFIG_SMP */

#endif /* __QDF_CPUHP_H */
