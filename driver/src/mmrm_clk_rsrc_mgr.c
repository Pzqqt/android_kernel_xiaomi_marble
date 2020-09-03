// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "mmrm_clk_rsrc_mgr.h"
#include "mmrm_debug.h"

/* sw clk mgr ops */
static struct mmrm_clk_mgr_ops sw_clk_mgr_ops = {
	.init_clk_mgr = mmrm_init_sw_clk_mgr,
	.destroy_clk_mgr = mmrm_destroy_sw_clk_mgr,
};

int mmrm_get_clk_mgr_ops(void *driver_data)
{
	int rc = 0;
	struct mmrm_driver_data *drv_data =
		(struct mmrm_driver_data *)driver_data;

	if (drv_data->clk_res.scheme == CLK_MGR_SCHEME_SW) {
		drv_data->clk_mgr_ops = &sw_clk_mgr_ops;
	} else if (drv_data->clk_res.scheme == CLK_MGR_SCHEME_CXIPEAK) {
		d_mpr_e("%s: cxipeak is not supported with mmrm\n", __func__);
		rc = -EINVAL;
		goto err_exit;
	} else {
		d_mpr_e("%s: unsupported clk mgr scheme\n", __func__);
		goto err_exit;
	}

	return rc;

err_exit:
	return rc;
}

struct mmrm_client *mmrm_clk_client_register(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client_desc *client_desc)
{
	if (!clk_mgr || !clk_mgr->clk_client_ops ||
		!clk_mgr->clk_client_ops->clk_client_reg) {
		d_mpr_e("%s: invalid clk mgr\n", __func__);
		return NULL;
	}

	return clk_mgr->clk_client_ops->clk_client_reg(clk_mgr,
			client_desc->client_info.desc,
			client_desc->priority,
			client_desc->pvt_data,
			client_desc->notifier_callback_fn);
}

int mmrm_clk_client_deregister(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client)
{
	if (!clk_mgr || !clk_mgr->clk_client_ops ||
		!clk_mgr->clk_client_ops->clk_client_dereg) {
		d_mpr_e("%s: invalid clk mgr\n", __func__);
		return -EINVAL;
	}

	return clk_mgr->clk_client_ops->clk_client_dereg(clk_mgr, client);
}


int mmrm_clk_client_setval(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_data *client_data,
	unsigned long val)
{
	if (!clk_mgr || !clk_mgr->clk_client_ops ||
		!clk_mgr->clk_client_ops->clk_client_setval) {
		d_mpr_e("%s: invalid clk mgr\n", __func__);
		return -EINVAL;
	}

	return clk_mgr->clk_client_ops->clk_client_setval(
		clk_mgr, client, client_data, val);
}

int mmrm_clk_client_setval_inrange(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_data *client_data,
	struct mmrm_client_res_value *val)
{
	if (!clk_mgr || !clk_mgr->clk_client_ops ||
		!clk_mgr->clk_client_ops->clk_client_setval_inrange) {
		d_mpr_e("%s: invalid clk mgr\n", __func__);
		return -EINVAL;
	}

	return clk_mgr->clk_client_ops->clk_client_setval_inrange(
		clk_mgr, client, client_data, val);
}

int mmrm_clk_client_getval(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_res_value *val)
{
	if (!clk_mgr || !clk_mgr->clk_client_ops ||
		!clk_mgr->clk_client_ops->clk_client_getval) {
		d_mpr_e("%s: invalid clk mgr\n", __func__);
		return -EINVAL;
	}

	return clk_mgr->clk_client_ops->clk_client_getval(
		clk_mgr, client, val);
}
