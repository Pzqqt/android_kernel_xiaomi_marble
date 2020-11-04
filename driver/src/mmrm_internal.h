/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MMRM_INTERNAL_H_
#define _MMRM_INTERNAL_H_

#include <linux/device.h>
#include <linux/types.h>

#include "mmrm_resources.h"
#include "mmrm_clk_rsrc_mgr.h"

struct mmrm_common_data {
	char key[128];
	int value;
};

struct mmrm_platform_data {
	struct mmrm_common_data *common_data;
	u32 common_data_length;
	u32 scheme;
};

struct mmrm_driver_data {
	/* platform data */
	struct mmrm_platform_data *platform_data;

	/* clk */
	struct mmrm_clk_platform_resources clk_res;
	struct mmrm_clk_mgr *clk_mgr;
	struct mmrm_clk_mgr_ops *clk_mgr_ops;

	/* debugfs */
	struct dentry *debugfs_root;
};

struct mmrm_platform_data *mmrm_get_platform_data(struct device *dev);

int mmrm_read_platform_resources(
	struct platform_device *pdev,
	struct mmrm_driver_data *drv_data);
int mmrm_free_platform_resources(struct mmrm_driver_data *drv_data);

int mmrm_init(struct mmrm_driver_data *drv_data);
int mmrm_deinit(struct mmrm_driver_data *drv_data);

#endif //_MMRM_INTERNAL_H_

