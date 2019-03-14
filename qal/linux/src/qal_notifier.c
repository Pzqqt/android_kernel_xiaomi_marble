/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#include "qal_notifier.h"
#include "qdf_module.h"
#include "qdf_trace.h"

static struct notifier_block qal_panic_nb;

static qal_panic_notifier qal_panic_cb;

static int qal_panic_notifier_handler(struct notifier_block *nb,
				      unsigned long action, void *data)
{
	qal_panic_cb(data);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qal_register_panic_notifier(qal_panic_notifier cb)
{
	QDF_ASSERT(!qal_panic_cb);
	if (qal_panic_cb) {
		qdf_err("Panic notifier already registered");
		return QDF_STATUS_E_FAILURE;
	}

	qal_panic_cb = cb;

	qal_panic_nb.notifier_call = qal_panic_notifier_handler;
	atomic_notifier_chain_register(&panic_notifier_list,
				       &qal_panic_nb);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qal_register_panic_notifier);

QDF_STATUS qal_unregister_panic_notifier(void)
{
	atomic_notifier_chain_unregister(&panic_notifier_list,
					 &qal_panic_nb);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qal_unregister_panic_notifier);
