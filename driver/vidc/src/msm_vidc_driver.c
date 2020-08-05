// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/iommu.h>
#include <linux/workqueue.h>
#include <media/msm_vidc_utils.h>
#include <media/msm_media_info.h>

#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"

u32 msm_vidc_convert_color_fmt(u32 v4l2_fmt)
{
	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV12:
		return COLOR_FMT_NV12;
	case V4L2_PIX_FMT_NV21:
		return COLOR_FMT_NV21;
	case V4L2_PIX_FMT_NV12_512:
		return COLOR_FMT_NV12_512;
	case V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS:
		return COLOR_FMT_P010;
	case V4L2_PIX_FMT_NV12_UBWC:
		return COLOR_FMT_NV12_UBWC;
	case V4L2_PIX_FMT_NV12_TP10_UBWC:
		return COLOR_FMT_NV12_BPP10_UBWC;
	case V4L2_PIX_FMT_RGBA8888_UBWC:
		return COLOR_FMT_RGBA8888_UBWC;
	default:
		d_vpr_e(
			"Invalid v4l2 color fmt FMT : %x, Set default(NV12)",
			v4l2_fmt);
		return COLOR_FMT_NV12;
	}
}

int msm_vidc_setup_event_queue(struct msm_vidc_inst *inst)
{
	int rc = 0;

	return rc;
}

int msm_vidc_queue_init(struct msm_vidc_inst *inst)
{
	int rc = 0;

	return rc;
}

int msm_vidc_add_session(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst *i;
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	mutex_lock(&core->lock);
	list_for_each_entry(i, &core->instances, list)
		count++;

	if (count < MAX_SUPPORTED_INSTANCES) {
		list_add_tail(&inst->list, &core->instances);
	} else {
		d_vpr_e("%s: total sessions %d reached max limit %d\n",
			__func__, count, MAX_SUPPORTED_INSTANCES);
		rc = -EINVAL;
	}
	mutex_unlock(&core->lock);

	/* assign session_id */
	inst->session_id = count + 1;
	inst->sid = inst->session_id;

	return rc;
}

int msm_vidc_core_init(struct msm_vidc_core *core)
{
	int rc;

	d_vpr_h("%s()\n", __func__);
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	if (core->state == MSM_VIDC_CORE_ERROR) {
		d_vpr_e("%s: core invalid state\n", __func__);
		rc = -EINVAL;
		goto unlock;
	}
	if (core->state == MSM_VIDC_CORE_INIT) {
		rc = 0;
		goto unlock;
	}

	rc = venus_hfi_core_init(core);
	if (rc) {
		d_vpr_e("%s: core init failed\n", __func__);
		core->state = MSM_VIDC_CORE_DEINIT;
		goto unlock;
	}

	core->state = MSM_VIDC_CORE_INIT;
	core->smmu_fault_handled = false;
	core->ssr.trigger = false;

unlock:
	mutex_unlock(&core->lock);
	return rc;
}

int msm_vidc_smmu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *data)
{
	return -EINVAL;
}

int msm_vidc_trigger_ssr(struct msm_vidc_core *core,
		enum msm_vidc_ssr_trigger_type type)
{
	return 0;
}

void msm_vidc_ssr_handler(struct work_struct *work)
{
}

void msm_vidc_pm_work_handler(struct work_struct *work)
{
}

void msm_vidc_fw_unload_handler(struct work_struct *work)
{
}

void msm_vidc_batch_handler(struct work_struct *work)
{
}
