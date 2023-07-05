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
 * QCA driver framework for bridge related APIs prototype
 */

#ifndef __QAL_BRIDGE_H
#define __QAL_BRIDGE_H

/* Include Files */
#include "qdf_types.h"
#include "i_qal_bridge.h"

typedef __qal_notify_blk_t qal_notify_blk_t;
typedef __qal_netdev_t qal_netdev_t;
typedef __qal_br_fdb_entry_t qal_br_fdb_entry_t;

#ifdef ENHANCED_OS_ABSTRACTION

QDF_STATUS
qal_bridge_fdb_register_notify(qal_notify_blk_t nb);

QDF_STATUS
qal_bridge_fdb_unregister_notify(qal_notify_blk_t nb);

qal_br_fdb_entry_t
qal_bridge_fdb_has_entry(qal_netdev_t dev, const char *addr, uint16_t vid);

QDF_STATUS
qal_bridge_fdb_delete_by_netdev(qal_netdev_t dev,
				const unsigned char *addr, uint16_t vid);

QDF_STATUS
qal_bridge_fdb_update_register_notify(qal_notify_blk_t nb);

QDF_STATUS
qal_bridge_fdb_update_unregister_notify(qal_notify_blk_t nb);
#else
static inline QDF_STATUS
qal_bridge_fdb_register_notify(qal_notify_blk_t nb)
{
	return __qal_bridge_fdb_register_notify(nb);
}

static inline QDF_STATUS
qal_bridge_fdb_unregister_notify(qal_notify_blk_t nb)
{
	return __qal_bridge_fdb_unregister_notify(nb);
}

static inline qal_br_fdb_entry_t
qal_bridge_fdb_has_entry(qal_netdev_t dev, const char *addr, uint16_t vid)
{
	return __qal_bridge_fdb_has_entry(dev, addr, vid);
}

static inline QDF_STATUS
qal_bridge_fdb_delete_by_netdev(qal_netdev_t dev,
				const unsigned char *addr, uint16_t vid)
{
	return __qal_bridge_fdb_delete_by_netdev(dev, addr, vid);
}

static inline QDF_STATUS
qal_bridge_fdb_update_register_notify(qal_notify_blk_t nb)
{
	return __qal_bridge_fdb_update_register_notify(nb);
}

static inline QDF_STATUS
qal_bridge_fdb_update_unregister_notify(qal_notify_blk_t nb)
{
	return __qal_bridge_fdb_update_unregister_notify(nb);
}
#endif

#endif /* __QAL_BRIDGE_H */
