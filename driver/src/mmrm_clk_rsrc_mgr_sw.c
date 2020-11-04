// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */
#include <linux/slab.h>
#include <dt-bindings/regulator/qcom,rpmh-regulator-levels.h>
#include <linux/clk.h>
#include <linux/clk/qcom.h>

#include "mmrm_debug.h"
#include "mmrm_clk_rsrc_mgr.h"
#include "mmrm_fixedpoint.h"

#define Q16_INT(q) ((q) >> 16)
#define Q16_FRAC(q) ((((q) & 0xFFFF) * 100) >> 16)

static struct mmrm_client *mmrm_sw_clk_client_register(
	struct mmrm_clk_mgr *sw_clk_mgr,
	struct mmrm_clk_client_desc clk_desc,
	enum mmrm_client_priority priority,
	void *pvt_data,
	notifier_callback_fn_t not_fn_cb)
{
	int rc = 0;
	struct mmrm_client *clk_client = NULL;
	struct mmrm_sw_clk_mgr_info *sinfo = &(sw_clk_mgr->data.sw_info);
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry;

	u32 c = 0;
	u32 clk_client_src_id = 0;

	d_mpr_h("%s: entering\n", __func__);

	mutex_lock(&sw_clk_mgr->lock);

	/* check if entry is free in table */
	if (sinfo->tot_clk_clients == sinfo->enabled_clk_clients) {
		d_mpr_e("%s: no free entry to register a clk client\n",
			__func__);
		rc = -EINVAL;
		goto err_nofree_entry;
	}

	/* look for entry that matches domain and id */
	clk_client_src_id = (clk_desc.client_domain << 16 | clk_desc.client_id);
	for (c = 0; c < sinfo->tot_clk_clients; c++) {
		if (clk_client_src_id == sinfo->clk_client_tbl[c].clk_src_id)
			break;
	}

	/* entry not found */
	if (c == sinfo->tot_clk_clients) {
		d_mpr_e("%s: unknown clk client %d\n",
			__func__, clk_client_src_id);
		rc = -EINVAL;
		goto err_nofree_entry;
	}

	tbl_entry = &sinfo->clk_client_tbl[c];

	/* entry already registered */
	if (tbl_entry->client) {
		d_mpr_e("%s: client csid(%d) already registered\n",
			__func__, tbl_entry->clk_src_id);
		rc = -EINVAL;
		goto err_already_registered;
	}

	/* populate the entry */
	clk_client = kzalloc(sizeof(*clk_client), GFP_KERNEL);
	if (!clk_client) {
		d_mpr_e("%s: failed to allocate memory for clk_client\n",
			__func__);
		rc = -ENOMEM;
		goto err_fail_alloc_clk_client;
	}

	clk_client->client_uid = c;
	clk_client->client_type = MMRM_CLIENT_CLOCK;

	/* copy the entries provided by client */
	tbl_entry->client = clk_client;
	strlcpy(tbl_entry->name, clk_desc.name, MMRM_CLK_CLIENT_NAME_SIZE);
	tbl_entry->clk = clk_desc.clk;
	tbl_entry->pri = priority;
	tbl_entry->pvt_data = pvt_data;
	tbl_entry->notifier_cb_fn = not_fn_cb;

	/* print table entry */
	d_mpr_h("%s: csid(%d) name(%s) pri(%d) pvt(%p) notifier(%p)\n",
		__func__,
		tbl_entry->clk_src_id,
		tbl_entry->name,
		tbl_entry->pri,
		tbl_entry->pvt_data,
		tbl_entry->notifier_cb_fn);

	/* print power entries for the clk src */
	d_mpr_h("%s: csid(%d) l1_cur_ma(%d) l2_cur_ma(%d) l3_cur_ma(%d)\n",
		__func__,
		tbl_entry->clk_src_id,
		tbl_entry->current_ma[MMRM_VDD_LEVEL_SVS_L1],
		tbl_entry->current_ma[MMRM_VDD_LEVEL_NOM],
		tbl_entry->current_ma[MMRM_VDD_LEVEL_TURBO]);

	mutex_unlock(&sw_clk_mgr->lock);

	d_mpr_h("%s: exiting with success\n", __func__);
	return clk_client;

err_fail_alloc_clk_client:
err_already_registered:
err_nofree_entry:
	mutex_unlock(&sw_clk_mgr->lock);

	d_mpr_h("%s: error exit\n", __func__);
	return NULL;
}

static int mmrm_sw_clk_client_deregister(struct mmrm_clk_mgr *sw_clk_mgr,
	struct mmrm_client *client)
{
	int rc =  0;
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry;
	struct mmrm_sw_clk_mgr_info *sinfo = &(sw_clk_mgr->data.sw_info);

	d_mpr_h("%s: entering\n", __func__);

	/* validate the client ptr */
	if (!client || client->client_uid >= sinfo->tot_clk_clients) {
		d_mpr_e("%s: invalid client uid (%d)\n",
			__func__, client->client_uid);
		rc = -EINVAL;
		goto err_not_valid_client;
	}

	mutex_lock(&sw_clk_mgr->lock);

	tbl_entry = &sinfo->clk_client_tbl[client->client_uid];
	kfree(tbl_entry->client);
	tbl_entry->client = NULL;
	tbl_entry->clk = NULL;
	tbl_entry->pri = 0x0;
	tbl_entry->pvt_data = NULL;
	tbl_entry->notifier_cb_fn = NULL;

	mutex_unlock(&sw_clk_mgr->lock);

	d_mpr_h("%s: exiting with success\n", __func__);
	return rc;

err_not_valid_client:
	d_mpr_h("%s: error exit\n", __func__);
	return rc;
}

static int mmrm_sw_get_req_current(
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry,
	unsigned long clk_val, u32 *req_current)
{
	int rc = 0;
	u32 i;
	int voltage_corner = mmrm_sw_vdd_corner[MMRM_VDD_LEVEL_NOM];

	/* get voltage corner */
	/* TBD: voltage_corner = qcom_clk_get_voltage(tbl_entry->clk, val); */
	for (i = 0; i < MMRM_VDD_LEVEL_MAX; i++) {
		if (voltage_corner == mmrm_sw_vdd_corner[i])
			break;
	}

	if (i == MMRM_VDD_LEVEL_MAX) {
		d_mpr_e("%s: csid(%d): invalid voltage corner(%d) for rate(%lld)\n",
			__func__, tbl_entry->clk_src_id,
			voltage_corner, clk_val);
		rc = -EINVAL;
		goto err_invalid_corner;
	}

	/* get current for the voltage corner */
	*req_current = tbl_entry->current_ma[i];

	return rc;

err_invalid_corner:
	return rc;
}

static int mmrm_sw_check_peak_current(
	struct mmrm_sw_clk_mgr_info *sinfo,
	u32 req_cur)
{
	int rc = 0;
	struct mmrm_sw_peak_current_data *peak_data = &sinfo->peak_cur_data;

	/* check for peak overshoot */
	if ((peak_data->aggreg_val + req_cur) >= peak_data->threshold) {
		rc = -EINVAL;
		/* TBD: return from here */
	}

	/* update peak current */
	peak_data->aggreg_val += req_cur;

	return rc;
}

static int mmrm_sw_clk_client_setval(struct mmrm_clk_mgr *sw_clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_data *client_data,
	unsigned long val)
{
	int rc = 0;
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry;
	struct mmrm_sw_clk_mgr_info *sinfo = &(sw_clk_mgr->data.sw_info);
	u32 req_cur;

	d_mpr_h("%s: entering\n", __func__);

	/* validate input params */
	if (!client || client->client_uid >= sinfo->tot_clk_clients) {
		d_mpr_e("%s: invalid client uid (%d)\n",
			__func__, client->client_uid);
		rc = -EINVAL;
		goto err_invalid_client;
	}

	/* get table entry */
	tbl_entry = &sinfo->clk_client_tbl[client->client_uid];
	if (!tbl_entry->clk) {
		d_mpr_e("%s: clk src not registered\n");
		rc = -EINVAL;
		goto err_invalid_client;
	}

	/* check if already configured */
	if (tbl_entry->clk_rate == val) {
		d_mpr_h("%s: csid(%d) same as previous clk rate %lld\n",
			__func__, tbl_entry->clk_src_id, val);
		goto exit_no_err;
	}

	/* get the required current val */
	rc = mmrm_sw_get_req_current(tbl_entry, val, &req_cur);
	if (rc || !req_cur) {
		d_mpr_e("%s: csid(%d) unable to get req current\n",
			__func__, tbl_entry->clk_src_id);
		rc = -EINVAL;
		goto err_invalid_clk_val;
	}

	mutex_lock(&sw_clk_mgr->lock);

	/* check & update for peak current */
	rc = mmrm_sw_check_peak_current(sinfo, req_cur);
	if (!rc) {
		d_mpr_e("%s: csid (%d) peak overshoot req_cur(%d) peak_cur(%d)\n",
			__func__, tbl_entry->clk_src_id, req_cur,
			sinfo->peak_cur_data.aggreg_val);
		/* TBD: unlock & check for mitigation */
	}

	/* update the current rate value */
	tbl_entry->clk_rate = val;
	mutex_unlock(&sw_clk_mgr->lock);

	/* set clock rate */
	d_mpr_e("%s: csid(%d) setting clk rate %llu\n", __func__,
		tbl_entry->clk_src_id, val);
	rc = clk_set_rate(tbl_entry->clk, val);
	if (rc) {
		d_mpr_e("%s: csid(%d) failed to set clock rate %llu\n",
		__func__, tbl_entry->clk_src_id, val);
		rc = -EINVAL;
		/* TBD: incase of failure clk_rate is invalid */
		goto err_clk_set_fail;
	}

exit_no_err:
	d_mpr_h("%s: exiting with success\n", __func__);
	return rc;

err_invalid_clk_val:
err_invalid_client:
err_clk_set_fail:
	d_mpr_h("%s: error exit\n", __func__);
	return rc;
}

static int mmrm_sw_clk_client_setval_inrange(struct mmrm_clk_mgr *sw_clk_mgr,
		struct mmrm_client *client,
		struct mmrm_client_data *client_data,
		struct mmrm_client_res_value *val)
{
	d_mpr_h("%s: entering\n", __func__);

	/* TBD: add support for set val in range */
	return mmrm_sw_clk_client_setval(sw_clk_mgr, client, client_data,
		val->cur);
}

static int mmrm_sw_clk_client_getval(struct mmrm_clk_mgr *sw_clk_mgr,
	struct mmrm_client *client,
	struct mmrm_client_res_value *val)
{
	int rc = 0;
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry;
	struct mmrm_sw_clk_mgr_info *sinfo = &(sw_clk_mgr->data.sw_info);

	d_mpr_h("%s: entering\n", __func__);

	/* validate input params */
	if (!client || client->client_uid >= sinfo->tot_clk_clients) {
		d_mpr_e("%s: invalid client uid (%d)\n",
			__func__, client->client_uid);
		rc = -EINVAL;
		goto err_invalid_client;
	}

	tbl_entry = &sinfo->clk_client_tbl[client->client_uid];
	if (!tbl_entry->clk) {
		d_mpr_e("%s: clk src not registered\n");
		rc = -EINVAL;
		goto err_invalid_client;
	}

	/* return previously configured value */
	/* TBD: Identify the min & max values */
	val->min = tbl_entry->clk_rate;
	val->cur = tbl_entry->clk_rate;
	val->max = tbl_entry->clk_rate;

	d_mpr_h("%s: exiting with success\n", __func__);
	return rc;

err_invalid_client:
	d_mpr_h("%s: error exit\n", __func__);
	return rc;
}

static struct mmrm_clk_mgr_client_ops clk_client_swops = {
	.clk_client_reg = mmrm_sw_clk_client_register,
	.clk_client_dereg = mmrm_sw_clk_client_deregister,
	.clk_client_setval = mmrm_sw_clk_client_setval,
	.clk_client_setval_inrange = mmrm_sw_clk_client_setval_inrange,
	.clk_client_getval = mmrm_sw_clk_client_getval,
};

static int mmrm_sw_update_entries(struct mmrm_clk_platform_resources *cres,
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry)
{
	u32 i;
	struct voltage_corner_set *cset = &cres->corner_set;
	u32 scaling_factor = 0, voltage_factor = 0;
	fp_t nom_dyn_pwr, nom_leak_pwr, freq_sc, dyn_sc, leak_sc,
		volt, dyn_pwr, leak_pwr, pwr_mw;

	nom_dyn_pwr = FP_INT(tbl_entry->dyn_pwr[MMRM_VDD_LEVEL_NOM]);
	nom_leak_pwr = FP(Q16_INT(tbl_entry->leak_pwr[MMRM_VDD_LEVEL_NOM]),
		Q16_FRAC(tbl_entry->leak_pwr[MMRM_VDD_LEVEL_NOM]), 100);

	/* freq scaling only for svsl1, TBD: enhance with actual numbers */
	freq_sc = FP(0, 86, 100);

	/* update power & current entries for all levels */
	for (i = 0; i < MMRM_VDD_LEVEL_MAX; i++) {
		scaling_factor = cset->corner_tbl[i].scaling_factor_dyn;
		dyn_sc = FP(
			Q16_INT(scaling_factor), Q16_FRAC(scaling_factor), 100);

		scaling_factor = cset->corner_tbl[i].scaling_factor_leak;
		leak_sc = FP(
			Q16_INT(scaling_factor), Q16_FRAC(scaling_factor), 100);

		voltage_factor = cset->corner_tbl[i].volt_factor;
		volt = FP(
			Q16_INT(voltage_factor), Q16_FRAC(voltage_factor), 100);

		if (!i)
			pwr_mw = fp_mult(nom_dyn_pwr, freq_sc);
		else
			pwr_mw = nom_dyn_pwr;

		dyn_pwr = fp_mult(pwr_mw, dyn_sc);
		leak_pwr = fp_mult(nom_leak_pwr, leak_sc);

		tbl_entry->dyn_pwr[i] = fp_round(dyn_pwr);
		tbl_entry->leak_pwr[i] = fp_round(leak_pwr);
		tbl_entry->current_ma[i] =
			fp_round(fp_div((dyn_pwr+leak_pwr), volt));

		d_mpr_h("%s: csid(%d) corner(%s) dyn_pwr(%zu) leak_pwr(%zu) tot_pwr(%d) cur_ma(%d)\n",
			__func__,
			tbl_entry->clk_src_id,
			cset->corner_tbl[i].name,
			tbl_entry->dyn_pwr[i],
			tbl_entry->leak_pwr[i],
			fp_round(dyn_pwr+leak_pwr),
			tbl_entry->current_ma[i]);
	}

	return 0;
}

static int mmrm_sw_prepare_table(struct mmrm_clk_platform_resources *cres,
	struct mmrm_sw_clk_mgr_info *sinfo)
{
	int rc = 0;
	u32 c;
	struct mmrm_sw_clk_client_tbl_entry *tbl_entry;
	struct nom_clk_src_info *nom_tbl_entry;

	d_mpr_h("%s: entering\n", __func__);

	/* read all resource entries */
	for (c = 0; c < sinfo->tot_clk_clients; c++) {
		tbl_entry = &sinfo->clk_client_tbl[c];
		nom_tbl_entry = &cres->nom_clk_set.clk_src_tbl[c];

		tbl_entry->clk_src_id = (nom_tbl_entry->domain << 16 |
			nom_tbl_entry->clk_src_id);
		tbl_entry->dyn_pwr[MMRM_VDD_LEVEL_NOM] =
			nom_tbl_entry->nom_dyn_pwr;
		tbl_entry->leak_pwr[MMRM_VDD_LEVEL_NOM] =
			nom_tbl_entry->nom_leak_pwr;

		d_mpr_h("%s: updating csid(%d) dyn_pwr(%d) leak_pwr(%d)\n",
			__func__,
			tbl_entry->clk_src_id,
			tbl_entry->dyn_pwr[MMRM_VDD_LEVEL_NOM],
			tbl_entry->leak_pwr[MMRM_VDD_LEVEL_NOM]);

		/* calculate current & scale power for other levels */
		rc = mmrm_sw_update_entries(cres, tbl_entry);
		if (rc) {
			d_mpr_e("%s: csid(%d) failed to prepare table\n",
				__func__, tbl_entry->clk_src_id);
		}
	}

	/* print the tables */
	for (c = 0; c < sinfo->tot_clk_clients; c++) {
		tbl_entry = &sinfo->clk_client_tbl[c];
		d_mpr_h("%s: csid(%d) l1_cur_ma(%d) l2_cur_ma(%d) l3_cur_ma(%d)\n",
			__func__,
			tbl_entry->clk_src_id,
			tbl_entry->current_ma[MMRM_VDD_LEVEL_SVS_L1],
			tbl_entry->current_ma[MMRM_VDD_LEVEL_NOM],
			tbl_entry->current_ma[MMRM_VDD_LEVEL_TURBO]);
	}

	d_mpr_h("%s: exiting\n", __func__);
	return rc;
}

int mmrm_init_sw_clk_mgr(void *driver_data)
{
	int rc = 0;
	struct mmrm_driver_data *drv_data =
		(struct mmrm_driver_data *)driver_data;
	struct mmrm_clk_platform_resources *cres = &drv_data->clk_res;
	struct mmrm_sw_clk_mgr_info *sinfo = NULL;
	struct mmrm_clk_mgr *sw_clk_mgr = NULL;
	u32 tbl_size = 0;

	d_mpr_h("%s: entering\n", __func__);

	/* mmrm_sw_clk_mgr */
	sw_clk_mgr = kzalloc(sizeof(*sw_clk_mgr), GFP_KERNEL);
	if (!sw_clk_mgr) {
		d_mpr_e("%s: failed to allocate memory for sw_clk_mgr\n",
			__func__);
		rc = -ENOMEM;
		goto err_fail_sw_clk_mgr;
	}

	/* initialize the tables */
	tbl_size = sizeof(struct mmrm_sw_clk_client_tbl_entry) *
		cres->nom_clk_set.count;

	sinfo = &(sw_clk_mgr->data.sw_info);
	sinfo->clk_client_tbl = kzalloc(tbl_size, GFP_KERNEL);
	if (!sinfo->clk_client_tbl) {
		d_mpr_e(
			"%s: failed to allocate memory for clk_client_tbl (%d)\n",
			__func__, cres->nom_clk_set.count);
		rc = -ENOMEM;
		goto err_fail_clk_tbl;
	}
	sinfo->tot_clk_clients = cres->nom_clk_set.count;
	sinfo->enabled_clk_clients = 0;

	/* prepare table entries */
	rc = mmrm_sw_prepare_table(cres, sinfo);
	if (rc) {
		d_mpr_e("%s: failed to prepare clk table\n", __func__);
		rc = -ENOMEM;
		goto err_fail_prep_tbl;
	}

	/* update the peak current threshold */
	sinfo->peak_cur_data.threshold = cres->threshold;
	sinfo->peak_cur_data.aggreg_val = 0;

	/* initialize mutex for sw clk mgr */
	mutex_init(&sw_clk_mgr->lock);
	sw_clk_mgr->scheme = drv_data->clk_res.scheme;

	/* clk client operations */
	sw_clk_mgr->clk_client_ops = &clk_client_swops;
	drv_data->clk_mgr = sw_clk_mgr;

	d_mpr_h("%s: exiting with success\n", __func__);
	return rc;

err_fail_prep_tbl:
	kfree(sinfo->clk_client_tbl);
err_fail_clk_tbl:
	kfree(sw_clk_mgr);
	drv_data->clk_mgr = NULL;
err_fail_sw_clk_mgr:
	d_mpr_h("%s: error exit\n", __func__);
	return rc;
}

int mmrm_destroy_sw_clk_mgr(struct mmrm_clk_mgr *sw_clk_mgr)
{
	int rc = 0;

	if (!sw_clk_mgr) {
		d_mpr_e("%s: sw_clk_mgr null\n", __func__);
		return -EINVAL;
	}

	kfree(sw_clk_mgr->data.sw_info.clk_client_tbl);
	mutex_destroy(&sw_clk_mgr->lock);
	kfree(sw_clk_mgr);

	return rc;
}
