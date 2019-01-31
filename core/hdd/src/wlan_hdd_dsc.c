/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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

/**
 * struct hdd_psoc_sync - a psoc synchronization context
 * @dev: the device used as a lookup key
 * @dsc_psoc: the dsc_psoc used for synchronization
 * @in_use: indicates if the context is being used
 */
struct hdd_psoc_sync {
	struct device *dev;
	struct dsc_psoc *dsc_psoc;
	bool in_use;
};

#define WLAN_MAX_PSOCS 1
static struct hdd_psoc_sync __hdd_psoc_sync_arr[WLAN_MAX_PSOCS];
static qdf_spinlock_t __hdd_psoc_sync_lock;

#define hdd_psoc_sync_lock_create() qdf_spinlock_create(&__hdd_psoc_sync_lock)
#define hdd_psoc_sync_lock_destroy() qdf_spinlock_destroy(&__hdd_psoc_sync_lock)
#define hdd_psoc_sync_lock() qdf_spin_lock_bh(&__hdd_psoc_sync_lock)
#define hdd_psoc_sync_unlock() qdf_spin_unlock_bh(&__hdd_psoc_sync_lock)
#define hdd_psoc_sync_lock_assert() \
	QDF_BUG(qdf_spin_is_locked(&__hdd_psoc_sync_lock))

static struct hdd_psoc_sync *hdd_psoc_sync_lookup(struct device *dev)
{
	int i;

	hdd_psoc_sync_lock_assert();

	for (i = 0; i < QDF_ARRAY_SIZE(__hdd_psoc_sync_arr); i++) {
		struct hdd_psoc_sync *psoc_sync = __hdd_psoc_sync_arr + i;

		if (psoc_sync->dev == dev)
			return psoc_sync;
	}

	return NULL;
}

static struct hdd_psoc_sync *hdd_psoc_sync_get(void)
{
	int i;

	hdd_psoc_sync_lock_assert();

	for (i = 0; i < QDF_ARRAY_SIZE(__hdd_psoc_sync_arr); i++) {
		struct hdd_psoc_sync *psoc_sync = __hdd_psoc_sync_arr + i;

		if (!psoc_sync->in_use) {
			psoc_sync->in_use = true;
			return psoc_sync;
		}
	}

	return NULL;
}

static void hdd_psoc_sync_put(struct hdd_psoc_sync *psoc_sync)
{
	hdd_psoc_sync_lock_assert();

	qdf_mem_zero(psoc_sync, sizeof(*psoc_sync));
}

int hdd_psoc_sync_create(struct dsc_driver *dsc_driver,
			 struct hdd_psoc_sync **out_psoc_sync)
{
	QDF_STATUS status;
	struct hdd_psoc_sync *psoc_sync;

	QDF_BUG(dsc_driver);
	if (!dsc_driver)
		return -EINVAL;

	QDF_BUG(out_psoc_sync);
	if (!out_psoc_sync)
		return -EINVAL;

	hdd_psoc_sync_lock();
	psoc_sync = hdd_psoc_sync_get();
	hdd_psoc_sync_unlock();
	if (!psoc_sync)
		return -ENOMEM;

	status = dsc_psoc_create(dsc_driver, &psoc_sync->dsc_psoc);
	if (QDF_IS_STATUS_ERROR(status))
		goto sync_put;

	*out_psoc_sync = psoc_sync;

	return 0;

sync_put:
	hdd_psoc_sync_lock();
	hdd_psoc_sync_put(psoc_sync);
	hdd_psoc_sync_unlock();

	return qdf_status_to_os_return(status);
}

int __hdd_psoc_sync_create_with_trans(struct dsc_driver *dsc_driver,
				      struct hdd_psoc_sync **out_psoc_sync,
				      const char *desc)
{
	struct hdd_psoc_sync *psoc_sync;
	QDF_STATUS status;
	int errno;

	errno = hdd_psoc_sync_create(dsc_driver, &psoc_sync);
	if (errno)
		return errno;

	status = dsc_psoc_trans_start(psoc_sync->dsc_psoc, desc);
	if (QDF_IS_STATUS_ERROR(status))
		goto sync_destroy;

	*out_psoc_sync = psoc_sync;

	return 0;

sync_destroy:
	hdd_psoc_sync_destroy(psoc_sync);

	return qdf_status_to_os_return(status);
}

void hdd_psoc_sync_destroy(struct hdd_psoc_sync *psoc_sync)
{
	QDF_BUG(psoc_sync);
	if (!psoc_sync)
		return;

	dsc_psoc_destroy(&psoc_sync->dsc_psoc);

	hdd_psoc_sync_lock();
	hdd_psoc_sync_put(psoc_sync);
	hdd_psoc_sync_unlock();
}

void hdd_psoc_sync_register(struct device *dev,
			    struct hdd_psoc_sync *psoc_sync)
{
	QDF_BUG(dev);
	QDF_BUG(psoc_sync);
	if (!psoc_sync)
		return;

	hdd_psoc_sync_lock();
	psoc_sync->dev = dev;
	hdd_psoc_sync_unlock();
}

struct hdd_psoc_sync *hdd_psoc_sync_unregister(struct device *dev)
{
	struct hdd_psoc_sync *psoc_sync;

	QDF_BUG(dev);
	if (!dev)
		return NULL;

	hdd_psoc_sync_lock();
	psoc_sync = hdd_psoc_sync_lookup(dev);
	if (psoc_sync)
		psoc_sync->dev = NULL;
	hdd_psoc_sync_unlock();

	return psoc_sync;
}

typedef QDF_STATUS (*psoc_start_func)(struct dsc_psoc *, const char *);

static int __hdd_psoc_sync_start_callback(struct device *dev,
					  struct hdd_psoc_sync **out_psoc_sync,
					  const char *desc,
					  psoc_start_func psoc_start_cb)
{
	QDF_STATUS status;
	struct hdd_psoc_sync *psoc_sync;

	hdd_psoc_sync_lock_assert();

	*out_psoc_sync = NULL;

	psoc_sync = hdd_psoc_sync_lookup(dev);
	if (!psoc_sync)
		return -EAGAIN;

	status = psoc_start_cb(psoc_sync->dsc_psoc, desc);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	*out_psoc_sync = psoc_sync;

	return 0;
}

int __hdd_psoc_sync_trans_start(struct device *dev,
				struct hdd_psoc_sync **out_psoc_sync,
				const char *desc)
{
	int errno;

	hdd_psoc_sync_lock();
	errno = __hdd_psoc_sync_start_callback(dev, out_psoc_sync, desc,
					       dsc_psoc_trans_start);
	hdd_psoc_sync_unlock();

	return errno;
}

int __hdd_psoc_sync_trans_start_wait(struct device *dev,
				     struct hdd_psoc_sync **out_psoc_sync,
				     const char *desc)
{
	int errno;

	hdd_psoc_sync_lock();
	errno = __hdd_psoc_sync_start_callback(dev, out_psoc_sync, desc,
					       dsc_psoc_trans_start_wait);
	hdd_psoc_sync_unlock();

	return errno;
}

static QDF_STATUS __assert_trans_cb(struct dsc_psoc *dsc_psoc, const char *desc)
{
	dsc_psoc_assert_trans_protected(dsc_psoc);

	return QDF_STATUS_SUCCESS;
}

int hdd_psoc_sync_trans_resume(struct device *dev,
			       struct hdd_psoc_sync **out_psoc_sync)
{
	int errno;

	hdd_psoc_sync_lock();
	errno = __hdd_psoc_sync_start_callback(dev, out_psoc_sync, NULL,
					       __assert_trans_cb);
	hdd_psoc_sync_unlock();

	return errno;
}

void hdd_psoc_sync_trans_stop(struct hdd_psoc_sync *psoc_sync)
{
	dsc_psoc_trans_stop(psoc_sync->dsc_psoc);
}

void hdd_psoc_sync_assert_trans_protected(struct device *dev)
{
	struct hdd_psoc_sync *psoc_sync;

	hdd_psoc_sync_lock();

	psoc_sync = hdd_psoc_sync_lookup(dev);
	QDF_BUG(psoc_sync);
	if (psoc_sync)
		dsc_psoc_assert_trans_protected(psoc_sync->dsc_psoc);

	hdd_psoc_sync_unlock();
}

int __hdd_psoc_sync_op_start(struct device *dev,
			     struct hdd_psoc_sync **out_psoc_sync,
			     const char *func)
{
	int errno;

	hdd_psoc_sync_lock();
	errno = __hdd_psoc_sync_start_callback(dev, out_psoc_sync, func,
					       _dsc_psoc_op_start);
	hdd_psoc_sync_unlock();

	return errno;
}

void __hdd_psoc_sync_op_stop(struct hdd_psoc_sync *psoc_sync,
			     const char *func)
{
	_dsc_psoc_op_stop(psoc_sync->dsc_psoc, func);
}

void hdd_psoc_sync_wait_for_ops(struct hdd_psoc_sync *psoc_sync)
{
	dsc_psoc_wait_for_ops(psoc_sync->dsc_psoc);
}

/**
 * struct hdd_vdev_sync - a vdev synchronization context
 * @net_dev: the net_device used as a lookup key
 * @dsc_vdev: the dsc_vdev used for synchronization
 * @in_use: indicates if the context is being used
 */
struct hdd_vdev_sync {
	struct net_device *net_dev;
	struct dsc_vdev *dsc_vdev;
	bool in_use;
};

static struct hdd_vdev_sync __hdd_vdev_sync_arr[CSR_ROAM_SESSION_MAX];
static qdf_spinlock_t __hdd_vdev_sync_lock;

#define hdd_vdev_sync_lock_create() qdf_spinlock_create(&__hdd_vdev_sync_lock)
#define hdd_vdev_sync_lock_destroy() qdf_spinlock_destroy(&__hdd_vdev_sync_lock)
#define hdd_vdev_sync_lock() qdf_spin_lock_bh(&__hdd_vdev_sync_lock)
#define hdd_vdev_sync_unlock() qdf_spin_unlock_bh(&__hdd_vdev_sync_lock)
#define hdd_vdev_sync_lock_assert() \
	QDF_BUG(qdf_spin_is_locked(&__hdd_vdev_sync_lock))

static struct hdd_vdev_sync *hdd_vdev_sync_lookup(struct net_device *net_dev)
{
	int i;

	hdd_vdev_sync_lock_assert();

	for (i = 0; i < QDF_ARRAY_SIZE(__hdd_vdev_sync_arr); i++) {
		struct hdd_vdev_sync *vdev_sync = __hdd_vdev_sync_arr + i;

		if (vdev_sync->net_dev == net_dev)
			return vdev_sync;
	}

	return NULL;
}

static struct hdd_vdev_sync *hdd_vdev_sync_get(void)
{
	int i;

	hdd_vdev_sync_lock_assert();

	for (i = 0; i < QDF_ARRAY_SIZE(__hdd_vdev_sync_arr); i++) {
		struct hdd_vdev_sync *vdev_sync = __hdd_vdev_sync_arr + i;

		if (!vdev_sync->in_use) {
			vdev_sync->in_use = true;
			return vdev_sync;
		}
	}

	return NULL;
}

static void hdd_vdev_sync_put(struct hdd_vdev_sync *vdev_sync)
{
	hdd_vdev_sync_lock_assert();

	qdf_mem_zero(vdev_sync, sizeof(*vdev_sync));
}

int hdd_vdev_sync_create(struct wiphy *wiphy,
			 struct hdd_vdev_sync **out_vdev_sync)
{
	QDF_STATUS status;
	struct dsc_psoc *dsc_psoc;
	struct hdd_vdev_sync *vdev_sync;

	QDF_BUG(wiphy);
	if (!wiphy)
		return -EINVAL;

	QDF_BUG(out_vdev_sync);
	if (!out_vdev_sync)
		return -EINVAL;

	dsc_psoc = hdd_dsc_psoc_from_wiphy(wiphy);
	if (!dsc_psoc)
		return -EINVAL;

	hdd_vdev_sync_lock();
	vdev_sync = hdd_vdev_sync_get();
	hdd_vdev_sync_unlock();
	if (!vdev_sync)
		return -ENOMEM;

	status = dsc_vdev_create(dsc_psoc, &vdev_sync->dsc_vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto sync_put;

	*out_vdev_sync = vdev_sync;

	return 0;

sync_put:
	hdd_vdev_sync_lock();
	hdd_vdev_sync_put(vdev_sync);
	hdd_vdev_sync_unlock();

	return qdf_status_to_os_return(status);
}

int __hdd_vdev_sync_create_with_trans(struct wiphy *wiphy,
				      struct hdd_vdev_sync **out_vdev_sync,
				      const char *desc)
{
	struct hdd_vdev_sync *vdev_sync;
	QDF_STATUS status;
	int errno;

	errno = hdd_vdev_sync_create(wiphy, &vdev_sync);
	if (errno)
		return errno;

	status = dsc_vdev_trans_start(vdev_sync->dsc_vdev, desc);
	if (QDF_IS_STATUS_ERROR(status))
		goto sync_destroy;

	*out_vdev_sync = vdev_sync;

	return 0;

sync_destroy:
	hdd_vdev_sync_destroy(vdev_sync);

	return qdf_status_to_os_return(status);
}

void hdd_vdev_sync_destroy(struct hdd_vdev_sync *vdev_sync)
{
	QDF_BUG(vdev_sync);
	if (!vdev_sync)
		return;

	dsc_vdev_destroy(&vdev_sync->dsc_vdev);

	hdd_vdev_sync_lock();
	hdd_vdev_sync_put(vdev_sync);
	hdd_vdev_sync_unlock();
}

void hdd_vdev_sync_register(struct net_device *net_dev,
			    struct hdd_vdev_sync *vdev_sync)
{
	QDF_BUG(net_dev);
	QDF_BUG(vdev_sync);
	if (!vdev_sync)
		return;

	hdd_vdev_sync_lock();
	vdev_sync->net_dev = net_dev;
	hdd_vdev_sync_unlock();
}

struct hdd_vdev_sync *hdd_vdev_sync_unregister(struct net_device *net_dev)
{
	struct hdd_vdev_sync *vdev_sync;

	QDF_BUG(net_dev);
	if (!net_dev)
		return NULL;

	hdd_vdev_sync_lock();
	vdev_sync = hdd_vdev_sync_lookup(net_dev);
	if (vdev_sync)
		vdev_sync->net_dev = NULL;
	hdd_vdev_sync_unlock();

	return vdev_sync;
}

typedef QDF_STATUS (*vdev_start_func)(struct dsc_vdev *, const char *);

static int __hdd_vdev_sync_start_callback(struct net_device *net_dev,
					  struct hdd_vdev_sync **out_vdev_sync,
					  const char *desc,
					  vdev_start_func vdev_start_cb)
{
	QDF_STATUS status;
	struct hdd_vdev_sync *vdev_sync;

	hdd_vdev_sync_lock_assert();

	*out_vdev_sync = NULL;

	vdev_sync = hdd_vdev_sync_lookup(net_dev);
	if (!vdev_sync)
		return -EAGAIN;

	status = vdev_start_cb(vdev_sync->dsc_vdev, desc);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	*out_vdev_sync = vdev_sync;

	return 0;
}

int __hdd_vdev_sync_trans_start(struct net_device *net_dev,
				struct hdd_vdev_sync **out_vdev_sync,
				const char *desc)
{
	int errno;

	hdd_vdev_sync_lock();
	errno = __hdd_vdev_sync_start_callback(net_dev, out_vdev_sync, desc,
					       dsc_vdev_trans_start);
	hdd_vdev_sync_unlock();

	return errno;
}

int __hdd_vdev_sync_trans_start_wait(struct net_device *net_dev,
				     struct hdd_vdev_sync **out_vdev_sync,
				     const char *desc)
{
	int errno;

	hdd_vdev_sync_lock();
	errno = __hdd_vdev_sync_start_callback(net_dev, out_vdev_sync, desc,
					       dsc_vdev_trans_start_wait);
	hdd_vdev_sync_unlock();

	return errno;
}

void hdd_vdev_sync_trans_stop(struct hdd_vdev_sync *vdev_sync)
{
	dsc_vdev_trans_stop(vdev_sync->dsc_vdev);
}

void hdd_vdev_sync_assert_trans_protected(struct net_device *net_dev)
{
	struct hdd_vdev_sync *vdev_sync;

	hdd_vdev_sync_lock();

	vdev_sync = hdd_vdev_sync_lookup(net_dev);
	QDF_BUG(vdev_sync);
	if (vdev_sync)
		dsc_vdev_assert_trans_protected(vdev_sync->dsc_vdev);

	hdd_vdev_sync_unlock();
}

int __hdd_vdev_sync_op_start(struct net_device *net_dev,
			     struct hdd_vdev_sync **out_vdev_sync,
			     const char *func)
{
	int errno;

	hdd_vdev_sync_lock();
	errno = __hdd_vdev_sync_start_callback(net_dev, out_vdev_sync, func,
					       _dsc_vdev_op_start);
	hdd_vdev_sync_unlock();

	return errno;
}

void __hdd_vdev_sync_op_stop(struct hdd_vdev_sync *vdev_sync,
			     const char *func)
{
	_dsc_vdev_op_stop(vdev_sync->dsc_vdev, func);
}

void hdd_vdev_sync_wait_for_ops(struct hdd_vdev_sync *vdev_sync)
{
	dsc_vdev_wait_for_ops(vdev_sync->dsc_vdev);
}

void hdd_dsc_init(void)
{
	hdd_psoc_sync_lock_create();
	hdd_vdev_sync_lock_create();
}

void hdd_dsc_deinit(void)
{
	hdd_vdev_sync_lock_destroy();
	hdd_psoc_sync_lock_destroy();
}

struct dsc_psoc *hdd_dsc_psoc_from_wiphy(struct wiphy *wiphy)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_psoc_sync *psoc_sync;
	struct dsc_psoc *dsc_psoc = NULL;

	if (!hdd_ctx)
		return NULL;

	hdd_psoc_sync_lock();

	psoc_sync = hdd_psoc_sync_lookup(hdd_ctx->parent_dev);
	if (psoc_sync)
		dsc_psoc = psoc_sync->dsc_psoc;

	hdd_psoc_sync_unlock();

	return dsc_psoc;
}

