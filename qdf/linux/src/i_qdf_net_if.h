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

/**
 * DOC: qdf_net_if
 * QCA driver framework (QDF) network interface management APIs
 */

#if !defined(__I_QDF_NET_IF_H)
#define __I_QDF_NET_IF_H

/* Include Files */
#include <qdf_types.h>
#include <qdf_util.h>
#include <linux/netdevice.h>

struct qdf_net_if;

/**
 * __qdf_net_if_create_dummy_if() - create dummy interface
 * @nif: interface handle
 *
 * This function will create a dummy network interface
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_net_if_create_dummy_if(struct qdf_net_if *nif)
{
	int ret;

	ret = init_dummy_netdev((struct net_device *)nif);

	return qdf_status_from_os_return(ret);
}
#endif /*__I_QDF_NET_IF_H */
