/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * DOC: qal_bridge
 * This file provides OS dependent bridge related APIs
 */

#include <linux/if_bridge.h>
#include <qdf_module.h>
#include "qal_bridge.h"
#include "qdf_types.h"

QDF_STATUS
qal_bridge_fdb_register_notify(qal_notify_blk_t nb)
{
	if (!nb)
		return QDF_STATUS_E_INVAL;

	br_fdb_register_notify(nb);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qal_bridge_fdb_register_notify);

QDF_STATUS
qal_bridge_fdb_unregister_notify(qal_notify_blk_t nb)
{
	if (!nb)
		return QDF_STATUS_E_INVAL;

	br_fdb_unregister_notify(nb);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qal_bridge_fdb_unregister_notify);

qal_br_fdb_entry_t
qal_bridge_fdb_has_entry(qal_netdev_t dev, const char *addr, uint16_t vid)
{
	if (!dev)
		return NULL;

	return br_fdb_has_entry(dev, addr, vid);
}

qdf_export_symbol(qal_bridge_fdb_has_entry);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 24)
QDF_STATUS
qal_bridge_fdb_delete_by_netdev(__qal_netdev_t dev,
				  const unsigned char *addr, uint16_t vid)
{
	/* Use 5.4-specific API */
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS
qal_bridge_fdb_delete_by_netdev(qal_netdev_t dev,
				const unsigned char *addr, uint16_t vid)
{
	int ret;

	if (!dev)
		return QDF_STATUS_E_INVAL;

	ret = br_fdb_delete_by_netdev(dev, addr, vid);

	return qdf_status_from_os_return(ret);
}
#endif

qdf_export_symbol(qal_bridge_fdb_delete_by_netdev);

QDF_STATUS
qal_bridge_fdb_update_register_notify(qal_notify_blk_t nb)
{
	if (!nb)
		return QDF_STATUS_E_INVAL;

	br_fdb_update_register_notify(nb);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qal_bridge_fdb_update_register_notify);

QDF_STATUS
qal_bridge_fdb_update_unregister_notify(qal_notify_blk_t nb)
{
	if (!nb)
		return QDF_STATUS_E_INVAL;

	br_fdb_update_unregister_notify(nb);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qal_bridge_fdb_update_unregister_notify);
