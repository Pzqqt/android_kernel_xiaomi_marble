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
	char *name = (char *)device->mmrm_cvp_desc.client_info.desc.name;

	if (!device) {
		dprintk(CVP_ERR, "%s invalid device\n", __func__);
		return -EINVAL;
	}

	device->mmrm_cvp = NULL;
	device->mmrm_cvp_desc.client_type = MMRM_CLIENT_CLOCK;
	device->mmrm_cvp_desc.client_info.desc.client_domain = MMRM_CLIENT_DOMAIN_CVP;
	/* TODO: use proper way to retrieve client id via dtsi */
	device->mmrm_cvp_desc.client_info.desc.client_id = 8;

	iris_hfi_for_each_clock(device, cl) {
		if (cl->has_scaling) {	/* only clk source enabled in dtsi */
			device->mmrm_cvp_desc.client_info.desc.clk = cl->clk;
			strlcpy(name, cl->name,
				sizeof(device->mmrm_cvp_desc.client_info.desc.name));
		}
	}
	device->mmrm_cvp_desc.priority = MMRM_CLIENT_PRIOR_LOW;

	dprintk(CVP_PWR,
		"%s: Register for %s\n",
		__func__, device->mmrm_cvp_desc.client_info.desc.name);

	device->mmrm_cvp = mmrm_client_register(&(device->mmrm_cvp_desc));
	if (device->mmrm_cvp == NULL) {
		dprintk(CVP_ERR, "%s: Failed mmrm_client_register with mmrm_cvp: %p\n",
			__func__, device->mmrm_cvp);
		rc = -ENOENT;
	} else {
		dprintk(CVP_PWR,
			"%s: Succeed mmrm_client_register with mmrm_cvp: %p, type :%d, uid: %ld\n",
			__func__, device->mmrm_cvp,
			device->mmrm_cvp->client_type, device->mmrm_cvp->client_uid);
	}

	return rc;
}

int msm_cvp_mmrm_set_value_in_range(struct iris_hfi_device *device, u32 freq_min, u32 freq_cur)
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
