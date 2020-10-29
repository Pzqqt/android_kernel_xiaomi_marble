/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

/**
 * qdf_net_if_get_dev_by_name() - Find a network device by its name
 * @nif_name: network device name
 *
 * This function retrieves the network device by its name
 *
 * Return: qdf network device
 */
static inline struct qdf_net_if *
__qdf_net_if_get_dev_by_name(char *nif_name)
{
	if (!nif_name)
		return NULL;

	return ((struct qdf_net_if *)dev_get_by_name(&init_net, nif_name));
}

/**
 * qdf_net_if_release_dev() - Release reference to network device
 * @nif: network device
 *
 * This function releases reference to the network device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qdf_net_if_release_dev(struct qdf_net_if  *nif)
{
	if (!nif)
		return QDF_STATUS_E_INVAL;

	dev_put((struct net_device *)nif);

	return QDF_STATUS_SUCCESS;
}
#endif /*__I_QDF_NET_IF_H */
