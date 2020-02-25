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
 * DOC: wlan_hdd_debugfs_unit_test.c
 *
 * WLAN Host Device Driver implementation to create debugfs
 * unit_test_host/unit_test_target/wlan_suspend/wlan_resume
 */
#include "wlan_hdd_main.h"
#include "osif_psoc_sync.h"
#include "osif_vdev_sync.h"
#include "wlan_dsc_test.h"
#include "wlan_hdd_unit_test.h"
#include "wlan_hdd_debugfs_unit_test.h"
#include "wlan_module_ids.h"
#include "wma.h"
#include "wlan_hdd_power.h"

#define MAX_USER_COMMAND_SIZE_UNIT_TEST_TARGET 256

#ifdef WLAN_UNIT_TEST
#define MAX_USER_COMMAND_SIZE_UNIT_TEST_HOST 32

/**
 * __wlan_hdd_write_unit_test_host_debugfs()
 *    - host unit test debugfs handler
 *
 * @net_dev: net_device context used to register the debugfs file
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed or errno
 */
static ssize_t __wlan_hdd_write_unit_test_host_debugfs(
		struct hdd_context *hdd_ctx,
		const char __user *buf, size_t count,
		loff_t *ppos)
{
	char name[MAX_USER_COMMAND_SIZE_UNIT_TEST_HOST + 1];
	int ret;

	if (count > MAX_USER_COMMAND_SIZE_UNIT_TEST_HOST) {
		hdd_err_rl("Command length is larger than %d bytes",
			   MAX_USER_COMMAND_SIZE_UNIT_TEST_HOST);
		return -EINVAL;
	}

	/* Get command from user */
	if (copy_from_user(name, buf, count))
		return -EFAULT;
	/* default 'echo' cmd takes new line character to here*/
	if (name[count - 1] == '\n')
		name[count - 1] = '\0';
	else
		name[count] = '\0';

	hdd_nofl_info("unit_test: count %zu name: %s", count, name);

	ret = wlan_hdd_unit_test(hdd_ctx, name);
	if (ret != 0)
		return ret;

	return count;
}

/**
 * wlan_hdd_write_unit_test_host_debugfs()
 *    - wrapper for __wlan_hdd_write_unit_test_host_debugfs
 *
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: number of bytes processed or errno
 */
static ssize_t wlan_hdd_write_unit_test_host_debugfs(
		struct file *file,
		const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct hdd_context *hdd_ctx = file_inode(file)->i_private;
	struct osif_psoc_sync *psoc_sync;
	ssize_t errno_size;

	errno_size = wlan_hdd_validate_context(hdd_ctx);
	if (errno_size)
		return errno_size;

	errno_size = osif_psoc_sync_op_start(wiphy_dev(hdd_ctx->wiphy),
					     &psoc_sync);
	if (errno_size)
		return errno_size;

	errno_size = __wlan_hdd_write_unit_test_host_debugfs(
				hdd_ctx, buf, count, ppos);
	if (errno_size < 0)
		hdd_err_rl("err_size %zd", errno_size);

	osif_psoc_sync_op_stop(psoc_sync);
	return errno_size;
}

static const struct file_operations fops_unit_test_host_debugfs = {
	.write = wlan_hdd_write_unit_test_host_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

int wlan_hdd_debugfs_unit_test_host_create(struct hdd_context *hdd_ctx)
{
	if (!debugfs_create_file("unit_test_host", 00400 | 00200,
				 qdf_debugfs_get_root(),
				 hdd_ctx, &fops_unit_test_host_debugfs))
		return -EINVAL;

	return 0;
}
#endif /* WLAN_UNIT_TEST */

#ifdef WLAN_SUSPEND_RESUME_TEST
#define MAX_USER_COMMAND_SIZE_SUSPEND 32

/**
 * __wlan_hdd_write_suspend_debugfs()
 *    - suspend test debugfs handler
 *
 * @net_dev: net_device context used to register the debugfs file
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed
 */
static ssize_t __wlan_hdd_write_suspend_debugfs(
		struct net_device *net_dev,
		const char __user *buf, size_t count,
		loff_t *ppos)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net_dev);
	struct hdd_context *hdd_ctx;
	char buf_local[MAX_USER_COMMAND_SIZE_SUSPEND + 1];
	char *sptr, *token;
	int ret, pause_setting, resume_setting;

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

	if (count > MAX_USER_COMMAND_SIZE_SUSPEND) {
		hdd_err_rl("Command length is larger than %d bytes",
			   MAX_USER_COMMAND_SIZE_SUSPEND);
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

	/* Get pause_setting */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &pause_setting))
		return -EINVAL;

	/* Get resume_setting */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &resume_setting))
		return -EINVAL;

	hdd_nofl_info("unit_test: pause_setting %d, resume_setting %d",
		      pause_setting, resume_setting);
	ret = hdd_wlan_fake_apps_suspend(hdd_ctx->wiphy, net_dev,
					 pause_setting, resume_setting);
	if (ret != 0) {
		hdd_err_rl("suspend test failed");
		return -EINVAL;
	}

	return count;
}

/**
 * wlan_hdd_write_suspend_debugfs()
 *    - wrapper for __wlan_hdd_write_suspend_debugfs
 *
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: number of bytes processed or errno
 */
static ssize_t wlan_hdd_write_suspend_debugfs(
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

	errno_size = __wlan_hdd_write_suspend_debugfs(
				net_dev, buf, count, ppos);
	if (errno_size < 0)
		hdd_err_rl("errno_size %zd", errno_size);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static const struct file_operations fops_suspend_debugfs = {
	.write = wlan_hdd_write_suspend_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

int wlan_hdd_debugfs_suspend_create(struct hdd_adapter *adapter)
{
	struct net_device *net_dev = adapter->dev;

	if (!debugfs_create_file("wlan_suspend", 00400 | 00200,
				 adapter->debugfs_phy,
				 net_dev, &fops_suspend_debugfs))
		return -EINVAL;

	return 0;
}

/**
 * __wlan_hdd_write_resume_debugfs()
 *    - resume test debugfs handler
 *
 * @net_dev: net_device context used to register the debugfs file
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed
 */
static ssize_t __wlan_hdd_write_resume_debugfs(
		struct net_device *net_dev,
		const char __user *buf, size_t count,
		loff_t *ppos)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net_dev);
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

	ret = hdd_wlan_fake_apps_resume(hdd_ctx->wiphy, net_dev);
	if (ret != 0) {
		hdd_err_rl("resume test failed");
		return -EINVAL;
	}

	return count;
}

/**
 * wlan_hdd_write_resume_debugfs()
 *    - wrapper for __wlan_hdd_write_resume_debugfs
 *
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: number of bytes processed or errno
 */
static ssize_t wlan_hdd_write_resume_debugfs(
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

	errno_size = __wlan_hdd_write_resume_debugfs(
				net_dev, buf, count, ppos);
	if (errno_size < 0)
		hdd_err_rl("errno_size %zd", errno_size);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static const struct file_operations fops_resume_debugfs = {
	.write = wlan_hdd_write_resume_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

int wlan_hdd_debugfs_resume_create(struct hdd_adapter *adapter)
{
	struct net_device *net_dev = adapter->dev;

	if (!debugfs_create_file("wlan_resume", 00400 | 00200,
				 adapter->debugfs_phy,
				 net_dev, &fops_resume_debugfs))
		return -EINVAL;

	return 0;
}
#endif

/**
 * __wlan_hdd_write_unit_test_target_debugfs()
 *    - target unit test debugfs handler
 *
 * @net_dev: net_device context used to register the debugfs file
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed
 */
static ssize_t __wlan_hdd_write_unit_test_target_debugfs(
		struct net_device *net_dev,
		const char __user *buf, size_t count,
		loff_t *ppos)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net_dev);
	struct hdd_context *hdd_ctx;
	char buf_local[MAX_USER_COMMAND_SIZE_UNIT_TEST_TARGET + 1];
	char *sptr, *token;
	uint32_t apps_args[WMA_MAX_NUM_ARGS];
	int module_id, args_num, ret, i;
	QDF_STATUS status;

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

	if (count > MAX_USER_COMMAND_SIZE_UNIT_TEST_TARGET) {
		hdd_err_rl("Command length is larger than %d bytes",
			   MAX_USER_COMMAND_SIZE_UNIT_TEST_TARGET);
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

	/* Get module_id */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &module_id))
		return -EINVAL;

	/* Get args_num */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &args_num))
		return -EINVAL;

	if (module_id < WLAN_MODULE_ID_MIN ||
	    module_id >= WLAN_MODULE_ID_MAX) {
		hdd_err_rl("Invalid MODULE ID %d", module_id);
		return -EINVAL;
	}
	if (args_num > WMA_MAX_NUM_ARGS) {
		hdd_err_rl("Too many args %d", args_num);
		return -EINVAL;
	}

	for (i = 0; i < args_num; i++) {
		token = strsep(&sptr, " ");
		if (!token) {
			hdd_err_rl("not enough args(%d), expected args_num:%d",
				   i, args_num);
			return -EINVAL;
		}
		if (kstrtou32(token, 0, &apps_args[i]))
			return -EINVAL;
	}

	status = sme_send_unit_test_cmd(adapter->vdev_id,
					module_id,
					args_num,
					&apps_args[0]);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err_rl("sme_send_unit_test_cmd returned %d", status);
		return -EINVAL;
	}

	return count;
}

/**
 * wlan_hdd_write_unit_test_target_debugfs()
 *    - wrapper for __wlan_hdd_write_unit_test_target_debugfs
 *
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: number of bytes processed or errno
 */
static ssize_t wlan_hdd_write_unit_test_target_debugfs(
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

	errno_size = __wlan_hdd_write_unit_test_target_debugfs(
				net_dev, buf, count, ppos);
	if (errno_size < 0)
		hdd_err_rl("errno_size %zd", errno_size);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static const struct file_operations fops_unit_test_target_debugfs = {
	.write = wlan_hdd_write_unit_test_target_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

int wlan_hdd_debugfs_unit_test_target_create(struct hdd_adapter *adapter)
{
	struct net_device *net_dev = adapter->dev;

	if (!debugfs_create_file("unit_test_target", 00400 | 00200,
				 adapter->debugfs_phy,
				 net_dev, &fops_unit_test_target_debugfs))
		return -EINVAL;

	return 0;
}
