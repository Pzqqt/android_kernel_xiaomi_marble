// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 */

#include "msm_cvp_common.h"
#include "cvp_hfi_api.h"
#include "msm_cvp_debug.h"
#include "msm_cvp_clocks.h"

int msm_cvp_set_clocks(struct msm_cvp_core *core)
{
	struct cvp_hfi_device *hdev;
	int rc;

	if (!core || !core->device) {
		dprintk(CVP_ERR, "%s Invalid args: %pK\n", __func__, core);
		return -EINVAL;
	}

	hdev = core->device;
	rc = call_hfi_op(hdev, scale_clocks,
		hdev->hfi_device_data, core->curr_freq);
	return rc;
}

int msm_cvp_mmrm_register(struct iris_hfi_device *device)
{
	int rc = 0;
	struct clock_info *cl = NULL;
	char *name = (char *)device->mmrm_desc.client_info.desc.name;

	if (!device) {
		dprintk(CVP_ERR, "%s invalid device\n", __func__);
		return -EINVAL;
	}

	device->mmrm_cvp = NULL;
	device->mmrm_desc.client_type=MMRM_CLIENT_CLOCK;
	device->mmrm_desc.priority=MMRM_CLIENT_PRIOR_LOW;
	device->mmrm_desc.client_info.desc.client_domain=MMRM_CLIENT_DOMAIN_CVP;
	/* TODO: use proper way to retrieve client id via dtsi */
	device->mmrm_desc.client_info.desc.client_id = 8;
	iris_hfi_for_each_clock(device, cl) {
		if (cl->has_scaling) {	/* only clk source enabled in dtsi */
			device->mmrm_desc.client_info.desc.clk=cl->clk;
			//device->mmrm_desc.client_info.desc.client_id=cl->clk_id;
			strlcpy(name, cl->name,
			sizeof(device->mmrm_desc.client_info.desc.name));
		}
	}

	dprintk(CVP_PWR,
		"%s: Register for %s, clk_id %d\n",
		__func__, device->mmrm_desc.client_info.desc.name,
		device->mmrm_desc.client_info.desc.client_id);

	device->mmrm_cvp = mmrm_client_register(&(device->mmrm_desc));
	if (device->mmrm_cvp == NULL) {
		dprintk(CVP_ERR,
			"%s: Failed mmrm_client_register with mmrm_cvp: %p\n",
			__func__, device->mmrm_cvp);
		rc = -ENOENT;
	} else {
		dprintk(CVP_PWR,
			"%s: mmrm_client_register done: %p, type:%d, uid:%ld\n",
			__func__, device->mmrm_cvp,
			device->mmrm_cvp->client_type,
			device->mmrm_cvp->client_uid);
	}

	return rc;
}

int msm_cvp_mmrm_set_value_in_range(struct iris_hfi_device *device,
	u32 freq_min, u32 freq_cur)
{
	int rc = 0;
	struct mmrm_client_res_value val;
	struct mmrm_client_data data;

	if (!device) {
		dprintk(CVP_ERR, "%s invalid device\n", __func__);
		return -EINVAL;
	}

	dprintk(CVP_PWR,
		"%s: set clock rate for mmrm_cvp: %p, type :%d, uid: %ld\n",
		__func__, device->mmrm_cvp,
		device->mmrm_cvp->client_type, device->mmrm_cvp->client_uid);

	val.min = freq_min;
	val.cur = freq_cur;
	data.num_hw_blocks = 1;
	data.flags = 0;		/* Not MMRM_CLIENT_DATA_FLAG_RESERVE_ONLY */

	dprintk(CVP_PWR,
		"%s: set clock rate to min %u cur %u: %d\n",
		__func__, val.min, val.cur, rc);

	rc = mmrm_client_set_value_in_range(device->mmrm_cvp, &data, &val);
	if (rc) {
		dprintk(CVP_ERR,
			"%s: Failed to set clock rate to min %u cur %u: %d\n",
			__func__, val.min, val.cur, rc);
	}
	return rc;
}
int msm_cvp_set_clocks_impl(struct iris_hfi_device *device, u32 freq)
{
	struct clock_info *cl;
	int rc = 0;
	int fsrc2clk = 3;
	// ratio factor for clock source : clk
	u32 freq_min = device->res->allowed_clks_tbl[0].clock_rate * fsrc2clk;

	dprintk(CVP_PWR, "%s: entering with freq : %ld\n", __func__, freq);

	iris_hfi_for_each_clock(device, cl) {
		if (cl->has_scaling) {/* has_scaling */
			device->clk_freq = freq;
			if (msm_cvp_clock_voting)
				freq = msm_cvp_clock_voting;

			freq = freq * fsrc2clk;
			dprintk(CVP_PWR,
				"%s: clock source rate set to: %ld\n",
				__func__, freq);

			if (device->mmrm_cvp != NULL) {
				/* min freq : 1st element value in the table */
				rc = msm_cvp_mmrm_set_value_in_range(device,
					freq_min, freq);
				if (rc) {
					dprintk(CVP_ERR,
						"Failed set clock %s: %d\n",
						cl->name, rc);
					return rc;
				}
			}
			else {
				dprintk(CVP_PWR,
					"%s: set clock with clk_set_rate\n",
					__func__);
				rc = clk_set_rate(cl->clk, freq);
				if (rc) {
					dprintk(CVP_ERR,
						"Failed set clock %u %s: %d\n",
						freq, cl->name, rc);
					return rc;
				}

				dprintk(CVP_PWR, "Scaling clock %s to %u\n",
					cl->name, freq);
			}
		}
	}

	return 0;
}

int msm_cvp_scale_clocks(struct iris_hfi_device *device)
{
	int rc = 0;
	struct allowed_clock_rates_table *allowed_clks_tbl = NULL;
	u32 rate = 0;

	allowed_clks_tbl = device->res->allowed_clks_tbl;

	rate = device->clk_freq ? device->clk_freq :
		allowed_clks_tbl[0].clock_rate;

	dprintk(CVP_PWR, "%s: scale clock rate %d\n", __func__, rate);
	rc = msm_cvp_set_clocks_impl(device, rate);
	return rc;
}

int msm_cvp_prepare_enable_clk(struct iris_hfi_device *device,
		const char *name)
{
	struct clock_info *cl = NULL;
	int rc = 0;

	if (!device) {
		dprintk(CVP_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	}

	iris_hfi_for_each_clock(device, cl) {
		if (strcmp(cl->name, name))
                        continue;
		/*
		* For the clocks we control, set the rate prior to preparing
		* them.  Since we don't really have a load at this point,
		* scale it to the lowest frequency possible
		*/
		if (cl->has_scaling) {
			if (device->mmrm_cvp != NULL) {
				// set min freq and cur freq to 0;
				rc = msm_cvp_mmrm_set_value_in_range(device,
						0, 0);
				if (rc)
					dprintk(CVP_ERR,
						"%s Failed set clock %s: %d\n",
						__func__, cl->name, rc);
			}
			else {
				dprintk(CVP_PWR,
					"%s: set clock with clk_set_rate\n",
					__func__);
				clk_set_rate(cl->clk,
						clk_round_rate(cl->clk, 0));
			}
		}
		rc = clk_prepare_enable(cl->clk);
		if (rc) {
			dprintk(CVP_ERR, "Failed to enable clock %s\n",
				cl->name);
			return rc;
		}
		if (!__clk_is_enabled(cl->clk)) {
			dprintk(CVP_ERR, "%s: clock %s not enabled\n",
					__func__, cl->name);
			clk_disable_unprepare(cl->clk);
			return -EINVAL;
		}

		dprintk(CVP_PWR, "Clock: %s prepared and enabled\n",
				cl->name);
		return 0;
	}

	dprintk(CVP_ERR, "%s clock %s not found\n", __func__, name);
	return -EINVAL;
}

int msm_cvp_disable_unprepare_clk(struct iris_hfi_device *device,
		const char *name)
{
	struct clock_info *cl;
	int rc = 0;

	if (!device) {
		dprintk(CVP_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	}

	iris_hfi_for_each_clock_reverse(device, cl) {
		if (strcmp(cl->name, name))
			continue;
		clk_disable_unprepare(cl->clk);
		dprintk(CVP_PWR, "Clock: %s disable and unprepare\n",
			cl->name);

		if (cl->has_scaling) {
			if (device->mmrm_cvp != NULL) {
				// set min freq and cur freq to 0;
				rc = msm_cvp_mmrm_set_value_in_range(device,
					0, 0);
				if (rc)
					dprintk(CVP_ERR,
						"%s Failed set clock %s: %d\n",
						__func__, cl->name, rc);
			}
		}
		return 0;
	}

	dprintk(CVP_ERR, "%s clock %s not found\n", __func__, name);
	return -EINVAL;
}

int msm_cvp_init_clocks(struct iris_hfi_device *device)
{
	int rc = 0;
	struct clock_info *cl = NULL;

	if (!device) {
		dprintk(CVP_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	}

	iris_hfi_for_each_clock(device, cl) {

		dprintk(CVP_PWR, "%s: scalable? %d, count %d\n",
			cl->name, cl->has_scaling, cl->count);
	}

	iris_hfi_for_each_clock(device, cl) {
		if (!cl->clk) {
			cl->clk = clk_get(&device->res->pdev->dev, cl->name);
			if (IS_ERR_OR_NULL(cl->clk)) {
				dprintk(CVP_ERR,
					"Failed to get clock: %s\n", cl->name);
				rc = PTR_ERR(cl->clk) ? : -EINVAL;
				cl->clk = NULL;
				goto err_clk_get;
			}
		}
	}
	device->clk_freq = 0;
	return 0;

err_clk_get:
	msm_cvp_deinit_clocks(device);
	return rc;
}

void msm_cvp_deinit_clocks(struct iris_hfi_device *device)
{
	struct clock_info *cl;

	device->clk_freq = 0;
	iris_hfi_for_each_clock_reverse(device, cl) {
		if (cl->clk) {
			clk_put(cl->clk);
			cl->clk = NULL;
		}
	}
}
