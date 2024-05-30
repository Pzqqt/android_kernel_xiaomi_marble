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
 * DOC: qdf_debug_domain
 * QCA driver framework (QDF) debug domain APIs. Debug domains are used to track
 * resource allocations across different driver states, particularly for runtime
 * leak detection.
 */

#ifndef __QDF_DEBUG_DOMAIN_H
#define __QDF_DEBUG_DOMAIN_H

#include "qdf_types.h"

/**
 * struct qdf_debug_domain - debug domains for tracking resource allocations
 * @QDF_DEBUG_DOMAIN_INIT: The default debug domain, tied to driver load
 * @QDF_DEBUG_DOMAIN_ACTIVE: The active debug domain, tied some "running" state
 * @QDF_DEBUG_DOMAIN_COUNT: The number of debug domains for iterating, etc.
 */
enum qdf_debug_domain {
	QDF_DEBUG_DOMAIN_INIT,
	QDF_DEBUG_DOMAIN_ACTIVE,

	/* keep last */
	QDF_DEBUG_DOMAIN_COUNT,
};

/**
 * qdf_debug_domain_get() - Get the current debug domain
 *
 * Return: the current debug domain
 */
enum qdf_debug_domain qdf_debug_domain_get(void);

/**
 * qdf_debug_domain_set() - Set the current debug domain
 * @domain: the domain to change to
 *
 * Return: None
 */
void qdf_debug_domain_set(enum qdf_debug_domain domain);

/**
 * qdf_debug_domain_name() - Get the human readable name of a debug domain
 * @domain: The domain to return the name of
 *
 * Return: name of the given domain
 */
const char *qdf_debug_domain_name(enum qdf_debug_domain domain);

/**
 * qdf_debug_domain_valid() - bounds checks the given domain
 * @domain: the domain to validate
 *
 * Return: true is the given domain is a valid debug domain
 */
bool qdf_debug_domain_valid(enum qdf_debug_domain domain);

#endif /* __QDF_DEBUG_DOMAIN_H */
