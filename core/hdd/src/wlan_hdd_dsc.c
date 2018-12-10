/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#include "sme_api.h"
#include "qdf_lock.h"
#include "qdf_status.h"
#include "qdf_types.h"
#include "wlan_dsc.h"
#include "wlan_hdd_dsc.h"

struct dsc_psoc *hdd_dsc_psoc_from_wiphy(struct wiphy *wiphy)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);

	if (!hdd_ctx)
		return NULL;

	if (!hdd_ctx->hdd_psoc)
		return NULL;

	return hdd_ctx->hdd_psoc->dsc_psoc;
}

/**
 * struct hdd_vdev_kvp - net_device/dsc_vdev key-value-pair for mapping a
 *	net_device to a dsc_vdev
 * @net_dev: the net_device key
 * @dsc_vdev: the dsc_vdev value
 */
struct hdd_vdev_kvp {
	struct net_device *net_dev;
	struct dsc_vdev *dsc_vdev;
};

static struct hdd_vdev_kvp __hdd_vdev_map[CSR_ROAM_SESSION_MAX];
static qdf_spinlock_t __hdd_vdev_lock;

#define hdd_vdev_map_lock() qdf_spin_lock_bh(&__hdd_vdev_lock)
#define hdd_vdev_map_unlock() qdf_spin_unlock_bh(&__hdd_vdev_lock)
#define hdd_vdev_map_lock_assert() QDF_BUG(qdf_spin_is_locked(&__hdd_vdev_lock))

static struct hdd_vdev_kvp *hdd_vdev_kvp_get(struct net_device *net_dev)
{
	int i;

	hdd_vdev_map_lock_assert();

	for (i = 0; i < QDF_ARRAY_SIZE(__hdd_vdev_map); i++) {
		struct hdd_vdev_kvp *kvp = __hdd_vdev_map + i;

		if (kvp->net_dev == net_dev)
			return kvp;
	}

	return NULL;
}

void hdd_vdev_ops_register(struct net_device *net_dev,
			   struct dsc_vdev *dsc_vdev)
{
	struct hdd_vdev_kvp *kvp;

	QDF_BUG(net_dev);
	if (!net_dev)
		return;

	QDF_BUG(dsc_vdev);
	if (!dsc_vdev)
		return;

	hdd_vdev_map_lock();

	kvp = hdd_vdev_kvp_get(NULL);
	QDF_BUG(kvp);
	if (kvp) {
		kvp->net_dev = net_dev;
		kvp->dsc_vdev = dsc_vdev;
	}

	hdd_vdev_map_unlock();
}

void hdd_vdev_ops_unregister(struct net_device *net_dev)
{
	struct hdd_vdev_kvp *kvp;

	QDF_BUG(net_dev);
	if (!net_dev)
		return;

	hdd_vdev_map_lock();

	kvp = hdd_vdev_kvp_get(net_dev);
	QDF_BUG(kvp);
	if (kvp) {
		kvp->net_dev = NULL;
		kvp->dsc_vdev = NULL;
	}

	hdd_vdev_map_unlock();
}

static struct hdd_vdev_op *hdd_vdev_to_op(struct dsc_vdev *dsc_vdev)
{
	return (struct hdd_vdev_op *)dsc_vdev;
}

static struct dsc_vdev *hdd_op_to_vdev(struct hdd_vdev_op *op)
{
	return (struct dsc_vdev *)op;
}

struct hdd_vdev_op *__hdd_vdev_op_start(struct net_device *net_dev,
					const char *func)
{
	QDF_STATUS status = QDF_STATUS_E_NULL_VALUE;
	struct hdd_vdev_kvp *kvp;

	QDF_BUG(net_dev);
	if (!net_dev)
		return NULL;

	hdd_vdev_map_lock();

	kvp = hdd_vdev_kvp_get(net_dev);
	if (kvp)
		status = _dsc_vdev_op_start(kvp->dsc_vdev, func);

	hdd_vdev_map_unlock();

	if (QDF_IS_STATUS_ERROR(status))
		return NULL;

	return hdd_vdev_to_op(kvp->dsc_vdev);
}

void __hdd_vdev_op_stop(struct hdd_vdev_op *op, const char *func)
{
	_dsc_vdev_op_stop(hdd_op_to_vdev(op), func);
}

void hdd_dsc_init(void)
{
	qdf_spinlock_create(&__hdd_vdev_lock);
}

void hdd_dsc_deinit(void)
{
	qdf_spinlock_destroy(&__hdd_vdev_lock);
}

