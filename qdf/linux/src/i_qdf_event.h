/*
 * Copyright (c) 2014-2017,2020 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_event.h
 * This file provides OS dependent event API's.
 */

#if !defined(__I_QDF_EVENT_H)
#define __I_QDF_EVENT_H

#include <linux/completion.h>

/**
 * qdf_event_t - manages events
 * @complete: instance to completion
 * @cookie: unsigned int
 * @done: indicate completion
 * @force_set: indicate forceful completion
 */
typedef struct qdf_evt {
	struct completion complete;
	uint32_t cookie;
	bool done;
	bool force_set;
} __qdf_event_t;

/* Preprocessor definitions and constants */
#define LINUX_EVENT_COOKIE 0x12341234

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
#define INIT_COMPLETION(event) reinit_completion(&event)
#endif

#endif /*__I_QDF_EVENT_H*/
