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
#include <i_qdf_notifier.h>
#include <qdf_status.h>
#include <qdf_notifier.h>

QDF_STATUS qdf_register_blocking_notifier_chain(qdf_blocking_notif_head *head,
						qdf_notif_block *qnb)
{
	int ret;

	ret = __qdf_register_blocking_notifier_chain(head, &qnb->notif_block);

	return qdf_status_from_os_return(ret);
}

QDF_STATUS qdf_unregister_blocking_notifier_chain(qdf_blocking_notif_head *head,
						  qdf_notif_block *qnb)
{
	int ret;

	ret = __qdf_unregister_blocking_notifier_chain(head,
						       &qnb->notif_block);

	return qdf_status_from_os_return(ret);
}

QDF_STATUS qdf_blocking_notfier_call(qdf_blocking_notif_head *head,
				     unsigned long state, void *data)
{
	int ret;

	ret = __qdf_blocking_notfier_call(head, state, data);

	return qdf_status_from_os_return(ret);
}

QDF_STATUS qdf_register_atomic_notifier_chain(qdf_atomic_notif_head *head,
					      qdf_notif_block *qnb)
{
	int ret;

	ret = __qdf_register_atomic_notifier_chain(head, &qnb->notif_block);

	return qdf_status_from_os_return(ret);
}

QDF_STATUS qdf_unregister_atomic_notifier_chain(qdf_atomic_notif_head *head,
						qdf_notif_block *qnb)
{
	int ret;

	ret = __qdf_unregister_atomic_notifier_chain(head, &qnb->notif_block);

	return qdf_status_from_os_return(ret);
}

QDF_STATUS qdf_atomic_notfier_call(qdf_atomic_notif_head *head,
				   unsigned long v, void *data)
{
	int ret;

	ret = __qdf_atomic_notifier_call(head, v, data);

	return qdf_status_from_os_return(ret);
}
