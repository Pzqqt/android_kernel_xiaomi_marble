/*
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
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
 * This file provides OS dependent network interface related APIs
 */

#include "qdf_net_if.h"
#include "qdf_types.h"
#include "qdf_module.h"
#include "qdf_util.h"
#include <linux/netdevice.h>

QDF_STATUS
qdf_net_if_create_dummy_if(struct qdf_net_if *nif)
{
	int ret;

	if (!nif)
		return QDF_STATUS_E_INVAL;

	ret = init_dummy_netdev((struct net_device *)nif);

	return qdf_status_from_os_return(ret);
}

qdf_export_symbol(qdf_net_if_create_dummy_if);

/**
 * qdf_net_if_get_devname() - Retrieve netdevice name
 * @nif: Abstraction of netdevice
 *
 * Return: netdevice name
 */
char *qdf_net_if_get_devname(struct qdf_net_if *nif)
{
	if (!nif)
		return NULL;

	return (((struct net_device *)nif)->name);
}

qdf_export_symbol(qdf_net_if_get_devname);
