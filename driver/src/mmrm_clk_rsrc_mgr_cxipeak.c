// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

struct mmrm_client *mmrm_cxipeak_clk_client_register(
	struct mmrm_clk_client_desc clk_desc,
	enum mmrm_client_priority priority, void *pvt_data,
	notifier_callback_fn_t not_fn_cb)
{
	return NULL;
}

int mmrm_cxipeak_clk_client_deregister(struct mmrm_client *client)
{
	return 0;
}

int mmrm_cxipeak_clk_client_set_value(
	struct mmrm_client_data *client_data, unsigned long val)
{
	return 0;
}

int mmrm_cxipeak_clk_client_get_value(struct mmrm_client *client,
	struct mmrm_client_res_value *val)
{
	return 0;
}

static struct mmrm_clk_mgr_client_ops clk_client_cxipeakops = {
	.clk_client_reg = mmrm_cxipeak_clk_client_register,
	.clk_client_dereg = mmrm_cxipeak_clk_client_deregister,
	.clk_client_setval = mmrm_cxipeak_clk_client_setval,
	.clk_client_getval = mmrm_cxipeak_clk_client_getval,
};


int mmrm_init_cxipeak_clk_mgr(void *driver_data)
{
	return 0;
}

int mmrm_destroy_cxipeak_clk_mgr(struct mmrm_clk_mgr *cxipeak_clk_mgr)
{
	return 0;
}
