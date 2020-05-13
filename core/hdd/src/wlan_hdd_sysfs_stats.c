/*
 * Copyright (c) 2011-2020, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_sysfs_stats.c
 *
 * implementation for creating sysfs files:
 *
 *   dump_stats
 *   clear_stats
 */

#include <wlan_hdd_includes.h>
#include "osif_vdev_sync.h"
#include <wlan_hdd_sysfs.h>
#include "wlan_hdd_sysfs_stats.h"

static int stats_id = -1;

static ssize_t
__wlan_hdd_sysfs_dump_stats_store(struct net_device *net_dev, char const *buf,
				  size_t count)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	struct hdd_context *hdd_ctx;
	char buf_local[MAX_SYSFS_USER_COMMAND_SIZE_LENGTH + 1];
	char *sptr, *token;
	int value, ret;

	if (hdd_validate_adapter(adapter)) {
		hdd_err_rl("adapter validate fail");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = hdd_sysfs_validate_and_copy_buf(buf_local, sizeof(buf_local),
					      buf, count);
	if (ret) {
		hdd_err_rl("invalid input");
		return ret;
	}

	sptr = buf_local;
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &value))
		return -EINVAL;

	hdd_debug("dump_stats %d", value);

	stats_id = value;

	return count;
}

static ssize_t wlan_hdd_sysfs_dump_stats_store(struct device *dev,
					       struct device_attribute *attr,
					       char const *buf, size_t count)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __wlan_hdd_sysfs_dump_stats_store(net_dev, buf, count);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static ssize_t
__wlan_hdd_sysfs_dump_stats_show(struct net_device *net_dev, char *buf)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	struct hdd_context *hdd_ctx;
	int ret;

	if (hdd_validate_adapter(adapter)) {
		hdd_err_rl("adapter validate fail");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = hdd_wlan_dump_stats(adapter, stats_id);

	return ret;
}

static ssize_t wlan_hdd_sysfs_dump_stats_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __wlan_hdd_sysfs_dump_stats_show(net_dev, buf);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static DEVICE_ATTR(dump_stats, 0660, wlan_hdd_sysfs_dump_stats_show,
		   wlan_hdd_sysfs_dump_stats_store);

static ssize_t
__wlan_hdd_sysfs_clear_stats_store(struct net_device *net_dev, char const *buf,
				   size_t count)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	struct hdd_context *hdd_ctx;
	char buf_local[MAX_SYSFS_USER_COMMAND_SIZE_LENGTH + 1];
	char *sptr, *token;
	int value, ret;

	if (hdd_validate_adapter(adapter)) {
		hdd_err_rl("adapter validate fail");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = hdd_sysfs_validate_and_copy_buf(buf_local, sizeof(buf_local),
					      buf, count);
	if (ret) {
		hdd_err_rl("invalid input");
		return ret;
	}

	sptr = buf_local;
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &value))
		return -EINVAL;

	hdd_debug("clear_stats %d", value);

	ret = hdd_wlan_clear_stats(adapter, value);
	if (ret) {
		hdd_err_rl("Failed to clear stats");
		return ret;
	}

	return count;
}

static ssize_t wlan_hdd_sysfs_clear_stats_store(struct device *dev,
						struct device_attribute *attr,
						char const *buf, size_t count)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __wlan_hdd_sysfs_clear_stats_store(net_dev, buf, count);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static DEVICE_ATTR(clear_stats, 0220, NULL,
		   wlan_hdd_sysfs_clear_stats_store);

int hdd_sysfs_stats_create(struct hdd_adapter *adapter)
{
	int error;

	error = device_create_file(&adapter->dev->dev, &dev_attr_dump_stats);
	if (error)
		hdd_err("could not create dump_stats sysfs file");

	error = device_create_file(&adapter->dev->dev, &dev_attr_clear_stats);
	if (error)
		hdd_err("could not create clear_stats sysfs file");

	return error;
}

void hdd_sysfs_stats_destroy(struct hdd_adapter *adapter)
{
	device_remove_file(&adapter->dev->dev, &dev_attr_dump_stats);
	device_remove_file(&adapter->dev->dev, &dev_attr_clear_stats);
}
