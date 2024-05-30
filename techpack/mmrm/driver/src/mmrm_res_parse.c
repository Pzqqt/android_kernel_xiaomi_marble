// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/types.h>
#include <linux/of_platform.h>

#include "mmrm_internal.h"
#include "mmrm_debug.h"
#include "mmrm_clk_rsrc_mgr.h"


static int mmrm_find_key_value(
	struct mmrm_platform_data *pdata, const char *key)
{
	int i = 0;
	struct mmrm_common_data *cdata = pdata->common_data;
	int size = pdata->common_data_length;

	for (i = 0; i < size; i++) {
		if (!strcmp(cdata[i].key, key))
			return cdata[i].value;
	}
	return 0;
}

static int mmrm_read_clk_pltfrm_rsrc_frm_drv_data(
	struct mmrm_driver_data *ddata)
{
	struct mmrm_platform_data *pdata;
	struct mmrm_clk_platform_resources *cres;
	int i = 0;

	pdata = ddata->platform_data;
	cres = &ddata->clk_res;

	cres->threshold = mmrm_find_key_value(pdata,
						"qcom,mmrm_clk_threshold");
	d_mpr_h("%s: configured mmrm clk threshold %d\n",
		__func__, cres->threshold);

	cres->scheme = mmrm_find_key_value(pdata,
					"qcom,mmrm_clk_mgr_scheme");
	d_mpr_h("%s: configured mmrm scheme %d\n",
		__func__, cres->scheme);
	cres->throttle_clients_data_length = pdata->throttle_clk_clients_data_length;

	for (i = 0; i < pdata->throttle_clk_clients_data_length; i++) {
		cres->clsid_threshold_clients[i] =
			(pdata->throttle_clk_clients_data[i].domain << 16
				| pdata->throttle_clk_clients_data[i].id);
	}

	return 0;
}

static void mmrm_free_rail_corner_table(
	struct mmrm_clk_platform_resources *cres)
{
	cres->corner_set.corner_tbl = NULL;
	cres->corner_set.count = 0;
}

static int mmrm_load_mm_rail_corner_table(
	struct mmrm_clk_platform_resources *cres)
{
	int rc = 0, num_corners = 0, c = 0;
	struct voltage_corner_set *corners = &cres->corner_set;
	struct platform_device *pdev = cres->pdev;

	num_corners = of_property_count_strings(pdev->dev.of_node,
		"mm-rail-corners");
	if (num_corners <= 0) {
		d_mpr_e("%s: no mm rail corners found\n",
			__func__);
		corners->count = 0;
		goto err_load_corner_tbl;
	}

	corners->corner_tbl = devm_kzalloc(&pdev->dev,
		sizeof(*corners->corner_tbl) * num_corners, GFP_KERNEL);
	if (!corners->corner_tbl) {
		d_mpr_e("%s: failed to allocate memory for corner_tbl\n",
			__func__);
		rc = -ENOMEM;
		goto err_load_corner_tbl;
	}
	corners->count = num_corners;
	d_mpr_h("%s: found %d corners\n",
		__func__, num_corners);

	for (c = 0; c < num_corners; c++) {
		struct corner_info *ci = &corners->corner_tbl[c];

		of_property_read_string_index(pdev->dev.of_node,
			"mm-rail-corners", c, &ci->name);
		of_property_read_u32_index(pdev->dev.of_node,
			"mm-rail-fact-volt", c, &ci->volt_factor);
		of_property_read_u32_index(pdev->dev.of_node,
			"scaling-fact-dyn", c, &ci->scaling_factor_dyn);
		of_property_read_u32_index(pdev->dev.of_node,
			"scaling-fact-leak", c, &ci->scaling_factor_leak);
	}

	/* print corner tables */
	for (c = 0; c < num_corners; c++) {
		struct corner_info *ci = &corners->corner_tbl[c];

		d_mpr_h(
			"%s: corner_name:%s volt_factor: %d sc_dyn: %d sc_leak: %d\n",
			__func__, ci->name, ci->volt_factor,
			ci->scaling_factor_dyn, ci->scaling_factor_leak);
	}

	return 0;

err_load_corner_tbl:
	return rc;
}

static void mmrm_free_nom_clk_src_table(
	struct mmrm_clk_platform_resources *cres)
{
	cres->nom_clk_set.clk_src_tbl = NULL;
	cres->nom_clk_set.count = 0;
}

static int mmrm_load_nom_clk_src_table(
	struct mmrm_clk_platform_resources *cres)
{
	int rc = 0, num_clk_src = 0, c = 0, size_clk_src = 0, entry_offset = 5;

	struct platform_device *pdev = cres->pdev;
	struct nom_clk_src_set *clk_srcs = &cres->nom_clk_set;

	of_find_property(pdev->dev.of_node, "mmrm-client-info", &size_clk_src);
	if ((size_clk_src < sizeof(*clk_srcs->clk_src_tbl)) ||
		(size_clk_src % sizeof(*clk_srcs->clk_src_tbl))) {
		d_mpr_e("%s: invalid size(%d) of clk src table\n",
			__func__, size_clk_src);
		clk_srcs->count = 0;
		goto err_load_clk_src_tbl;
	}

	clk_srcs->clk_src_tbl = devm_kzalloc(&pdev->dev,
		size_clk_src, GFP_KERNEL);
	if (!clk_srcs->clk_src_tbl) {
		d_mpr_e("%s: failed to allocate memory for clk_src_tbl\n",
			__func__);
		rc = -ENOMEM;
		goto err_load_clk_src_tbl;
	}
	num_clk_src = size_clk_src / sizeof(struct nom_clk_src_info);
	clk_srcs->count = num_clk_src;
	d_mpr_h("%s: found %d clk_srcs size %d\n",
		__func__, num_clk_src, size_clk_src);
	for (c = 0; c < num_clk_src; c++) {
		struct nom_clk_src_info *ci = &clk_srcs->clk_src_tbl[c];

		of_property_read_u32_index(pdev->dev.of_node,
			"mmrm-client-info", (c*entry_offset), &ci->domain);
		of_property_read_u32_index(pdev->dev.of_node,
			"mmrm-client-info", (c*entry_offset+1), &ci->clk_src_id);
		of_property_read_u32_index(pdev->dev.of_node,
			"mmrm-client-info", (c*entry_offset+2),
			&ci->nom_dyn_pwr);
		of_property_read_u32_index(pdev->dev.of_node,
			"mmrm-client-info", (c*entry_offset+3),
			&ci->nom_leak_pwr);
		of_property_read_u32_index(pdev->dev.of_node,
			"mmrm-client-info", (c*entry_offset+4),
			&ci->num_hw_block);
	}

	/* print corner tables */
	for (c = 0; c < num_clk_src; c++) {
		struct nom_clk_src_info *ci = &clk_srcs->clk_src_tbl[c];

		d_mpr_h("%s: domain: %d clk_src: %d dyn_pwr: %d leak_pwr: %d\n",
			__func__, ci->domain, ci->clk_src_id, ci->nom_dyn_pwr,
			ci->nom_leak_pwr);
	}

	return 0;

err_load_clk_src_tbl:
	return rc;
}

static int mmrm_read_clk_pltfrm_rsrc_frm_dt(
	struct mmrm_clk_platform_resources *cres)
{
	int rc = 0;

	rc = mmrm_load_mm_rail_corner_table(cres);
	if (rc) {
		d_mpr_e("%s: failed to load mm rail corner table\n",
			__func__);
		goto err_load_mmrm_rail_table;
	}

	if (cres->scheme == CLK_MGR_SCHEME_SW) {
		rc = mmrm_load_nom_clk_src_table(cres);
		if (rc) {
			d_mpr_e("%s: failed to load nom clk src table\n",
				__func__);
			goto err_load_nom_clk_src_table;
		}
	} else if (cres->scheme == CLK_MGR_SCHEME_CXIPEAK) {
		d_mpr_e("%s: cxipeak is not supported with mmrm\n",
			__func__);
		rc = -EINVAL;
		goto err_load_mmrm_rail_table;
	}

	return rc;

err_load_nom_clk_src_table:
	mmrm_free_nom_clk_src_table(cres);

err_load_mmrm_rail_table:
	mmrm_free_rail_corner_table(cres);
	return rc;
}

int mmrm_count_clk_clients_frm_dt(struct platform_device *pdev)
{
	u32 size_clk_src = 0, num_clk_src = 0;

	of_find_property(pdev->dev.of_node, "mmrm-client-info", &size_clk_src);
	num_clk_src = size_clk_src / sizeof(struct nom_clk_src_info);
	d_mpr_h("%s: found %d clk_srcs size %d\n",
		__func__, num_clk_src, size_clk_src);

	return num_clk_src;
}

int mmrm_read_platform_resources(struct platform_device *pdev,
	struct mmrm_driver_data *drv_data)
{
	int rc = 0;

	if (pdev->dev.of_node) {

		/* clk resources */
		drv_data->clk_res.pdev = pdev;

		rc = mmrm_read_clk_pltfrm_rsrc_frm_drv_data(drv_data);
		if (rc) {
			d_mpr_e(
				"%s: failed to read clk platform res from driver\n",
				__func__);
			goto exit;
		}
		rc = mmrm_read_clk_pltfrm_rsrc_frm_dt(&drv_data->clk_res);
		if (rc) {
			d_mpr_e("%s: failed to read clk platform res from dt\n",
				__func__);
			goto exit;
		}
	} else {
		d_mpr_e("%s: of node is null\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

exit:
	return rc;
}

int mmrm_free_platform_resources(struct mmrm_driver_data *drv_data)
{
	int rc = 0;

	/* free clk resources */
	mmrm_free_nom_clk_src_table(&drv_data->clk_res);
	mmrm_free_rail_corner_table(&drv_data->clk_res);

	return rc;
}
