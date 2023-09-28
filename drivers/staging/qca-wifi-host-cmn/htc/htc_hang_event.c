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
#include <qdf_hang_event_notifier.h>
#include <qdf_notifier.h>
#include "htc_hang_event.h"
#include "htc_internal.h"
#include "htc_credit_history.h"

static int htc_recovery_notifier_call(struct notifier_block *block,
				      unsigned long state,
				      void *data)
{
	htc_log_hang_credit_history(block, data);

	return NOTIFY_OK;
}

static qdf_notif_block htc_recovery_notifier = {
	.notif_block.notifier_call = htc_recovery_notifier_call,
};

QDF_STATUS htc_hang_event_notifier_register(HTC_TARGET *target)
{
	htc_recovery_notifier.priv_data = target;
	return qdf_hang_event_register_notifier(&htc_recovery_notifier);
}

QDF_STATUS htc_hang_event_notifier_unregister(void)
{
	return qdf_hang_event_unregister_notifier(&htc_recovery_notifier);
}
