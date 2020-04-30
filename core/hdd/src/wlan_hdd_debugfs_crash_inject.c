/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_debugfs_crash_inject.c
 *
 * implementation for creating debugfs file crash_inject
 */

#include <wlan_hdd_includes.h>
#include "osif_vdev_sync.h"
#include "wlan_hdd_debugfs_crash_inject.h"

/* strlen("1 1") + 1(\n) */
#define MIN_USER_COMMAND_SIZE_CRASH_INJECT 4
#define MAX_USER_COMMAND_SIZE_CRASH_INJECT 32

/**
 * __wlan_hdd_write_crash_inject_debugfs()
 *    - crash inject test debugfs handler
 *
 * @net_dev: net_device context used to register the debugfs file
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed
 */
static ssize_t __wlan_hdd_write_crash_inject_debugfs(
		struct net_device *net_dev,
		const char __user *buf, size_t count,
		loff_t *ppos)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net_dev);
	struct hdd_context *hdd_ctx;
	char buf_local[MAX_USER_COMMAND_SIZE_CRASH_INJECT + 1];
	char *sptr, *token;
	uint32_t val1, val2;
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

	if (count < MIN_USER_COMMAND_SIZE_CRASH_INJECT ||
	    count > MAX_USER_COMMAND_SIZE_CRASH_INJECT) {
		hdd_err_rl("Command length (%zu) is invalid, expected [%d, %d]",
			   count,
			   MIN_USER_COMMAND_SIZE_CRASH_INJECT,
			   MAX_USER_COMMAND_SIZE_CRASH_INJECT);
		return -EINVAL;
	}

	/* Get command from user */
	if (copy_from_user(buf_local, buf, count))
		return -EFAULT;
	/* default 'echo' cmd takes new line character to here*/
	if (buf_local[count - 1] == '\n')
		buf_local[count - 1] = '\0';
	else
		buf_local[count] = '\0';

	sptr = buf_local;
	hdd_nofl_info("unit_test: count %zu buf_local:(%s) net_devname %s",
		      count, buf_local, net_dev->name);

	/* Get val1 */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &val1))
		return -EINVAL;

	/* Get val2 */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &val2))
		return -EINVAL;

	ret = hdd_crash_inject(adapter, val1, val2);
	if (ret != 0) {
		hdd_err_rl("hdd_crash_inject returned %d", ret);
		return -EINVAL;
	}

	return count;
}

/**
 * wlan_hdd_write_crash_inject_debugfs()
 *    - wrapper for __wlan_hdd_write_crash_inject_debugfs
 *
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: number of bytes processed or errno
 */
static ssize_t wlan_hdd_write_crash_inject_debugfs(
		struct file *file,
		const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct net_device *net_dev = file_inode(file)->i_private;
	struct osif_vdev_sync *vdev_sync;
	ssize_t errno_size;

	errno_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (errno_size)
		return errno_size;

	errno_size = __wlan_hdd_write_crash_inject_debugfs(
				net_dev, buf, count, ppos);
	if (errno_size < 0)
		hdd_err_rl("errno_size %zd", errno_size);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static const struct file_operations fops_crash_inject_debugfs = {
	.write = wlan_hdd_write_crash_inject_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

int wlan_hdd_debugfs_crash_inject_create(struct hdd_adapter *adapter)
{
	struct net_device *net_dev = adapter->dev;

	if (!debugfs_create_file("crash_inject", 00400 | 00200,
				 adapter->debugfs_phy,
				 net_dev, &fops_crash_inject_debugfs))
		return -EINVAL;

	return 0;
}
