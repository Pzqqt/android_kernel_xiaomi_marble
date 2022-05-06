/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_sysfs_dp_tx_delay_stats.c
 *
 * implementation for creating sysfs files:
 *
 * dp_tx_delay_stats
 */
#include <wlan_hdd_includes.h>
#include "osif_vdev_sync.h"
#include "wlan_hdd_object_manager.h"
#include <wlan_hdd_sysfs.h>
#include <wlan_hdd_sysfs_dp_tx_delay_stats.h>
#include <cdp_txrx_host_stats.h>

static ssize_t
__hdd_sysfs_dp_tx_delay_stats_show(struct net_device *net_dev,
				   char *buf)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	ol_txrx_soc_handle dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t value;
	int ret;

	if (hdd_validate_adapter(adapter) || !dp_soc)
		return -EINVAL;

	ret = wlan_hdd_validate_context(adapter->hdd_ctx);
	if (ret)
		return ret;

	if (!wlan_hdd_validate_modules_state(adapter->hdd_ctx))
		return -EINVAL;

	value = cdp_vdev_is_tx_delay_stats_enabled(dp_soc, adapter->vdev_id);

	hdd_debug("vdev_id: %d tx_delay_stats: %d", adapter->vdev_id, value);

	return scnprintf(buf, PAGE_SIZE, "%d\n", value);
}

static ssize_t hdd_sysfs_dp_tx_delay_stats_show(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __hdd_sysfs_dp_tx_delay_stats_show(net_dev, buf);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static ssize_t
__hdd_sysfs_dp_tx_delay_stats_store(struct net_device *net_dev, const char *buf,
				    size_t count)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	char buf_local[MAX_SYSFS_USER_COMMAND_SIZE_LENGTH + 1];
	ol_txrx_soc_handle dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	char *sptr, *token;
	uint8_t value;
	int ret;

	if (hdd_validate_adapter(adapter) || !dp_soc)
		return -EINVAL;

	ret = wlan_hdd_validate_context(adapter->hdd_ctx);
	if (ret)
		return ret;

	if (!wlan_hdd_validate_modules_state(adapter->hdd_ctx))
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
	if (kstrtou8(token, 0, &value))
		return -EINVAL;

	hdd_debug("vdev_id: %d tx_delay_stats: %d", adapter->vdev_id, value);

	cdp_enable_disable_vdev_tx_delay_stats(dp_soc, adapter->vdev_id, value);

	return count;
}

static ssize_t
hdd_sysfs_dp_tx_delay_stats_store(struct device *dev,
				  struct device_attribute *attr,
				  char const *buf, size_t count)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t errno_size;

	errno_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (errno_size)
		return errno_size;

	errno_size = __hdd_sysfs_dp_tx_delay_stats_store(net_dev, buf, count);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static DEVICE_ATTR(dp_tx_delay_stats, 0660,
		   hdd_sysfs_dp_tx_delay_stats_show,
		   hdd_sysfs_dp_tx_delay_stats_store);

int hdd_sysfs_dp_tx_delay_stats_create(struct hdd_adapter *adapter)
{
	int error;

	error = device_create_file(&adapter->dev->dev,
				   &dev_attr_dp_tx_delay_stats);
	if (error)
		hdd_err("could not create dp_tx_delay_stats sysfs file");

	return error;
}

void
hdd_sysfs_dp_tx_delay_stats_destroy(struct hdd_adapter *adapter)
{
	device_remove_file(&adapter->dev->dev, &dev_attr_dp_tx_delay_stats);
}
