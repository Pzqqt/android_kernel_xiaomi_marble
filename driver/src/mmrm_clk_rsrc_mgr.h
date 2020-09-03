/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MMRM_CLK_RESOURCE_MGR_H_
#define _MMRM_CLK_RESOURCE_MGR_H_

#include <dt-bindings/regulator/qcom,rpmh-regulator-levels.h>
#include <linux/soc/qcom/msm_mmrm.h>

#include "mmrm_internal.h"

enum mmrm_clk_mgr_scheme {
	CLK_MGR_SCHEME_SW,
	CLK_MGR_SCHEME_CXIPEAK
};

enum mmrm_sw_vdd_levels {
	MMRM_VDD_LEVEL_SVS_L1,
	MMRM_VDD_LEVEL_NOM,
	MMRM_VDD_LEVEL_TURBO,
	MMRM_VDD_LEVEL_MAX
};

static int mmrm_sw_vdd_corner[] = {
	[MMRM_VDD_LEVEL_SVS_L1] = RPMH_REGULATOR_LEVEL_SVS_L1,
	[MMRM_VDD_LEVEL_NOM] = RPMH_REGULATOR_LEVEL_NOM,
	[MMRM_VDD_LEVEL_TURBO] = RPMH_REGULATOR_LEVEL_TURBO
};

#define MMRM_SW_CLIENTS_NUM_MAX 35

typedef int (*notifier_callback_fn_t)(
	struct mmrm_client_notifier_data *notifier_data);

struct mmrm_sw_clk_client_tbl_entry {
	char name[MMRM_CLK_CLIENT_NAME_SIZE];
	struct clk *clk;
	enum mmrm_client_priority pri;
	void *pvt_data; /* client user data */
	notifier_callback_fn_t notifier_cb_fn;

	/* prepared internally */
	u32 clk_src_id;
	bool pass_through;
	u32 min_level;
	u32 max_level;
	u64 freq[MMRM_VDD_LEVEL_MAX];
	u32 dyn_pwr[MMRM_VDD_LEVEL_MAX];
	u32 leak_pwr[MMRM_VDD_LEVEL_MAX];
	u32 current_ma[MMRM_VDD_LEVEL_MAX];

	/* reference to this entry */
	struct mmrm_client *client;

	/* configured clk rate */
	u64 clk_rate;
};

struct mmrm_sw_peak_current_data {
	u32 threshold;
	u32 aggreg_val;
};

struct mmrm_sw_clk_mgr_info {
	/* client data */
	struct mmrm_sw_clk_client_tbl_entry *clk_client_tbl;
	u32 tot_clk_clients;
	u32 enabled_clk_clients;

	/* peak current data */
	struct mmrm_sw_peak_current_data peak_cur_data;
};

struct mmrm_clk_mgr {
	struct mutex lock;
	enum mmrm_clk_mgr_scheme scheme;
	union {
		struct mmrm_sw_clk_mgr_info sw_info;
	} data;
	struct mmrm_clk_mgr_client_ops *clk_client_ops;
};

struct mmrm_clk_mgr_client_ops {
	/* client ops */
	struct mmrm_client*(*clk_client_reg)(
		struct mmrm_clk_mgr *clk_mgr,
		struct mmrm_clk_client_desc clk_desc,
		enum mmrm_client_priority priority, void *pvt_data,
		notifier_callback_fn_t nt_fn_cb);
	int (*clk_client_dereg)(
		struct mmrm_clk_mgr *clk_mgr,
		struct mmrm_client *client);
	int (*clk_client_setval)(struct mmrm_clk_mgr *clk_mgr,
		struct mmrm_client *client,
		struct mmrm_client_data *client_data, unsigned long val);
	int (*clk_client_setval_inrange)(struct mmrm_clk_mgr *clk_mgr,
		struct mmrm_client *client,
		struct mmrm_client_data *client_data,
		struct mmrm_client_res_value *val);
	int (*clk_client_getval)(struct mmrm_clk_mgr *clk_mgr,
		struct mmrm_client *client, struct mmrm_client_res_value *val);
};

/* clk mgr operations */
struct mmrm_clk_mgr_ops {
	int (*init_clk_mgr)(void *drv_data);
	int (*destroy_clk_mgr)(struct mmrm_clk_mgr *sw_clk_mgr);
};
int mmrm_get_clk_mgr_ops(void *drv_data);

/* clk mgr client operations */
struct mmrm_client *mmrm_clk_client_register(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client_desc *client_desc);
int mmrm_clk_client_deregister(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client);
int mmrm_clk_client_setval(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_data *client_data,
	unsigned long val);
int mmrm_clk_client_setval_inrange(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_data *client_data,
	struct mmrm_client_res_value *val);
int mmrm_clk_client_getval(struct mmrm_clk_mgr *clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_res_value *val);

/* sw clk mgr specific */
int mmrm_init_sw_clk_mgr(void *driver_data);
int mmrm_destroy_sw_clk_mgr(struct mmrm_clk_mgr *sw_clk_mgr);

#endif //_MMRM_CLK_RESOURCE_MGR_H_
