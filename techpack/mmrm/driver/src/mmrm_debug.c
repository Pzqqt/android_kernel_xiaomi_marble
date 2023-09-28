// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/slab.h>

#include "mmrm_debug.h"

int msm_mmrm_debug = MMRM_ERR | MMRM_WARN | MMRM_PRINTK;
u8 msm_mmrm_enable_throttle_feature = 1;
u8 msm_mmrm_allow_multiple_register = 0;



#define MAX_DBG_BUF_SIZE 4096

static ssize_t msm_mmrm_debugfs_info_read(
	struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char *dbuf, *cur, *end;
	ssize_t len = 0;

	dbuf = kzalloc(MAX_DBG_BUF_SIZE, GFP_KERNEL);
	if (!dbuf) {
		d_mpr_e("%s: Allocation failed!\n", __func__);
		return -ENOMEM;
	}
	cur = dbuf;
	end = cur + MAX_DBG_BUF_SIZE;

	// TODO: Fill cur here

	len = simple_read_from_buffer(buf, count, ppos, dbuf, cur - dbuf);
	kfree(dbuf);
	return len;
}

const struct file_operations msm_mmrm_debugfs_info_fops = {
	.open = simple_open,
	.read = msm_mmrm_debugfs_info_read,
};

struct dentry *msm_mmrm_debugfs_init(void)
{
	struct dentry *dir;
	int file_val;

	/* create a directory in debugfs root (/sys/kernel/debug) */
	dir = debugfs_create_dir("msm_mmrm", NULL);
	if (IS_ERR_OR_NULL(dir)) {
		d_mpr_e("%s: Call to debugfs_create_dir(%s) failed!\n", __func__, "mmrm");
		goto failed_create_dir;
	}

	/* basic info */
	if (!debugfs_create_file("info", 0444, dir, &file_val, &msm_mmrm_debugfs_info_fops)) {
		d_mpr_e("%s: Call to debugfs_create_file(%s) failed!\n", __func__, "info");
		goto failed_create_dir;
	}

	/* add other params here */
	debugfs_create_u32("debug_level", 0644, dir, &msm_mmrm_debug);
	debugfs_create_u8("allow_multiple_register", 0644, dir, &msm_mmrm_allow_multiple_register);
	debugfs_create_u8("enable_throttle_feature", 0644, dir, &msm_mmrm_enable_throttle_feature);

	return dir;

failed_create_dir:
	d_mpr_e("%s: error\n", __func__);
	return NULL;
}

void msm_mmrm_debugfs_deinit(struct dentry *dir)
{
	debugfs_remove_recursive(dir);
}
