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

#include "qdf_debug_domain.h"
#include "qdf_trace.h"

static enum qdf_debug_domain qdf_debug_domain_current = QDF_DEBUG_DOMAIN_INIT;

enum qdf_debug_domain qdf_debug_domain_get(void)
{
	return qdf_debug_domain_current;
}

void qdf_debug_domain_set(enum qdf_debug_domain domain)
{
	QDF_BUG(qdf_debug_domain_valid(domain));
	if (!qdf_debug_domain_valid(domain))
		return;

	qdf_debug_domain_current = domain;
}

const char *qdf_debug_domain_name(enum qdf_debug_domain domain)
{
	switch (domain) {
	case QDF_DEBUG_DOMAIN_INIT:
		return "Init";
	case QDF_DEBUG_DOMAIN_ACTIVE:
		return "Active";
	default:
		return "Invalid";
	}
}

bool qdf_debug_domain_valid(enum qdf_debug_domain domain)
{
	return domain >= QDF_DEBUG_DOMAIN_INIT &&
		domain < QDF_DEBUG_DOMAIN_COUNT;
}
