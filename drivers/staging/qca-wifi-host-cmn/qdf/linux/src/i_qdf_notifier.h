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
  * DOC: i_qdf_notifier.h
  *
  * Linux-specific definitions for use by QDF notifier APIs
  */

#ifndef __I_QDF_NOTIFIER_H
#define __I_QDF_NOTIFIER_H

#include <linux/notifier.h>

typedef struct blocking_notifier_head __qdf_blocking_notif_head;
typedef struct atomic_notifier_head __qdf_atomic_notif_head;
typedef struct notifier_block __qdf_notifier_block;
#define qdf_blocking_notifier_init(p) BLOCKING_NOTIFIER_HEAD(p);
#define qdf_atomic_notifier_init(p) ATOMIC_NOTIFIER_HEAD(p);

static inline int
__qdf_register_blocking_notifier_chain(__qdf_blocking_notif_head *head,
				       __qdf_notifier_block *qnb)
{
	return blocking_notifier_chain_register(head, qnb);
}

static inline int
__qdf_unregister_blocking_notifier_chain(__qdf_blocking_notif_head *head,
					 __qdf_notifier_block *qnb)
{
	return blocking_notifier_chain_unregister(head, qnb);
}

static inline int
__qdf_blocking_notfier_call(__qdf_blocking_notif_head *head,
			    unsigned long v, void *data)
{
	return blocking_notifier_call_chain(head, v, data);
}

static inline int
__qdf_register_atomic_notifier_chain(__qdf_atomic_notif_head *head,
				     __qdf_notifier_block *qnb)
{
	return atomic_notifier_chain_register(head, qnb);
}

static inline int
__qdf_unregister_atomic_notifier_chain(__qdf_atomic_notif_head *head,
				       __qdf_notifier_block *qnb)
{
	return atomic_notifier_chain_unregister(head, qnb);
}

static inline int
__qdf_atomic_notifier_call(__qdf_atomic_notif_head *head,
			   unsigned long v, void *data)
{
	return atomic_notifier_call_chain(head, v, data);
}

#endif
