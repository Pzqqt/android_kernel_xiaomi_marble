/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_net_if
 * QCA driver framework (QDF) network interface management APIs
 */

#if !defined(__QDF_NET_IF_H)
#define __QDF_NET_IF_H

/* Include Files */
#include <qdf_types.h>
#include <i_qdf_net_if.h>

struct qdf_net_if;

#ifdef ENHANCED_OS_ABSTRACTION
/**
 * qdf_net_if_create_dummy_if() - create dummy interface
 * @nif: interface handle
 *
 * This function will create a dummy network interface
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qdf_net_if_create_dummy_if(struct qdf_net_if *nif);
#else
static inline QDF_STATUS
qdf_net_if_create_dummy_if(struct qdf_net_if *nif)
{
	return __qdf_net_if_create_dummy_if(nif);
}
#endif
#endif /* __QDF_NET_IF_H */
