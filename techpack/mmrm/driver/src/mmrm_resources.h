/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _MMRM_RESOURCES_H_
#define _MMRM_RESOURCES_H_

#include <linux/platform_device.h>
#define MMRM_MAX_THROTTLE_CLIENTS 5

struct corner_info {
	const char *name;
	u32 volt_factor;
	u32 scaling_factor_dyn;
	u32 scaling_factor_leak;
};

struct voltage_corner_set {
	struct corner_info *corner_tbl;
	u32 count;
};

struct nom_clk_src_info {
	u32 domain;
	u32 clk_src_id;
	u32 nom_dyn_pwr;
	u32 nom_leak_pwr;
	u32 num_hw_block;
};

struct nom_clk_src_set {
	struct nom_clk_src_info *clk_src_tbl;
	u32 count;
};

struct mmrm_clk_platform_resources {
	struct platform_device *pdev;
	u32 threshold;
	u32 scheme;
	u32 clsid_threshold_clients[MMRM_MAX_THROTTLE_CLIENTS];
	u16 throttle_clients_data_length;
	struct voltage_corner_set corner_set;
	struct nom_clk_src_set nom_clk_set;
};

#endif
