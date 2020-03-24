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
 * @file qdf_notifier.h
 * This file abstracts notifier chain call operations.
 */

#ifndef _QDF_NOTIFIER_H
#define _QDF_NOTIFIER_H

#include <i_qdf_notifier.h>
#include <qdf_status.h>

/*
 * qdf_notif_block - qdf notifier block
 * @__qdf_notifier_block: OS specific notifier block
 * @priv_data: private data of the notifier block
 */
typedef struct  {
	__qdf_notifier_block notif_block;
	void *priv_data;
} qdf_notif_block;

typedef __qdf_blocking_notif_head qdf_blocking_notif_head;
typedef __qdf_atomic_notif_head qdf_atomic_notif_head;
typedef __qdf_notifier_block qdf_notifier_block;

#ifdef WLAN_HANG_EVENT

/**
 * qdf_register_blocking_notifier_chain() - Register for blocking notifier chain
 * @qdf_blocking_notif_head: Head of blocking notifier chain
 * @qdf_notif_block: Notifier Block to be registered for this head chain
 *
 * This function is invoked to add a notifier block for the specific notifier
 * head chain.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_register_blocking_notifier_chain(qdf_blocking_notif_head *head,
						qdf_notif_block *qnb);
/**
 * qdf_unregister_blocking_notifier_chain() - Unregister for blocking notifier
 * chain
 * @qdf_blocking_notif_head: Head of blocking notifier chain
 * @qdf_notif_block: Notifier Block to be registered for this head chain
 *
 * This function is invoked to remove a notifier block for the specific notifier
 * head chain.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_unregister_blocking_notifier_chain(qdf_blocking_notif_head *head,
						  qdf_notif_block *qnb);
/**
 * qdf_blocking_notfier_call() - Invoke the function in the blocking chain
 * @qdf_blocking_notif_head: Head of blocking notifier chain
 * @state: state passed during the invoking of the notifier
 * @data: Private data to be passed to all the notifier functions
 *
 * This function is called to invoke all the notifier blocks for the specific
 * notifier chain with state and private data.
 * when success the notifier reply with NOTIFY_OK.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_blocking_notfier_call(qdf_blocking_notif_head *head,
				     unsigned long state, void *data);

/**
 * qdf_register_atomic_notifier_chain() - Register for atomic notifier chain
 * @qdf_blocking_notif_head: Head of atomic notifier chain
 * @qdf_notif_block: Notifier Block to be registered for this head chain
 *
 * This function is invoked to add a notifier block for the specific atomic
 * notifier head chain.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_register_atomic_notifier_chain(qdf_atomic_notif_head *head,
					      qdf_notif_block *qnb);
/**
 * qdf_unregister_atmoic_notifier_chain() - Unregister for atomic notifier
 * chain
 * @qdf_blocking_notif_head: Head of blocking notifier chain
 * @qdf_notif_block: Notifier Block to be registered for this head chain
 *
 * This function is invoked to remove a notifier block for the specific notifier
 * head chain.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_unregister_atomic_notifier_chain(qdf_atomic_notif_head *head,
						qdf_notif_block *qnb);
/**
 * qdf_blocking_notfier_call() - Invoke the function in the blocking chain
 * @qdf_blocking_notif_head: Head of blocking notifier chain
 * @v: Generally state passed during the invoking of the notifier
 * @data: Private data to be passed to all the notifier functions
 *
 * This function is invoke a notifier block for the specific notifier head chain
 * with state and private data. when success the notifier reply with NOTIFY_OK.
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_atomic_notfier_call(qdf_atomic_notif_head *head,
				   unsigned long v, void *data);
#else

static inline QDF_STATUS qdf_register_blocking_notifier_chain(
						qdf_blocking_notif_head *head,
						qdf_notif_block *qnb)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS qdf_unregister_blocking_notifier_chain(
						qdf_blocking_notif_head *head,
						qdf_notif_block *qnb)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS qdf_blocking_notfier_call(
						qdf_blocking_notif_head *head,
						unsigned long v, void *data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS qdf_register_atomic_notifier_chain(
						qdf_atomic_notif_head *head,
						qdf_notif_block *qnb)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS qdf_unregister_atomic_notifier_chain(
						qdf_atomic_notif_head *head,
						qdf_notif_block *qnb)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS qdf_atomic_notfier_call(qdf_atomic_notif_head *head,
						 unsigned long v, void *data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#endif
