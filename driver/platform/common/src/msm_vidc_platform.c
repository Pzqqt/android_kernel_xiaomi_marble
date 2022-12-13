// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/sort.h>

#include "msm_vidc_platform.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_v4l2.h"
#include "msm_vidc_vb2.h"
#include "msm_vidc_control.h"
#include "msm_vidc_core.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"
#if defined(CONFIG_MSM_VIDC_WAIPIO)
#include "msm_vidc_waipio.h"
#endif
#if defined(CONFIG_MSM_VIDC_DIWALI)
#include "msm_vidc_diwali.h"
#endif
#if defined(CONFIG_MSM_VIDC_KHAJE)
#include "msm_vidc_khaje.h"
#endif
#if defined(CONFIG_MSM_VIDC_MONACO)
#include "msm_vidc_monaco.h"
#endif
#if defined(CONFIG_MSM_VIDC_PARROT)
#include "msm_vidc_parrot.h"
#endif
#if defined(CONFIG_MSM_VIDC_RAVELIN)
#include "msm_vidc_ravelin.h"
#endif
#if defined(CONFIG_MSM_VIDC_NEO)
#include "msm_vidc_neo.h"
#endif
#if defined(CONFIG_MSM_VIDC_IRIS2) || defined(CONFIG_MSM_VIDC_IRIS3)
#include "msm_vidc_iris2.h"
#endif
#if defined(CONFIG_MSM_VIDC_AR50LT)
#include "msm_vidc_ar50lt.h"
#endif
/*
 * Custom conversion coefficients for resolution: 176x144 negative
 * coeffs are converted to s4.9 format
 * (e.g. -22 converted to ((1 << 13) - 22)
 * 3x3 transformation matrix coefficients in s4.9 fixed point format
 */
u32 vpe_csc_custom_matrix_coeff[MAX_MATRIX_COEFFS] = {
	440, 8140, 8098, 0, 460, 52, 0, 34, 463
};

/* offset coefficients in s9 fixed point format */
u32 vpe_csc_custom_bias_coeff[MAX_BIAS_COEFFS] = {
	53, 0, 4
};

/* clamping value for Y/U/V([min,max] for Y/U/V) */
u32 vpe_csc_custom_limit_coeff[MAX_LIMIT_COEFFS] = {
	16, 235, 16, 240, 16, 240
};

static struct v4l2_file_operations msm_v4l2_file_operations = {
	.owner                          = THIS_MODULE,
	.open                           = msm_v4l2_open,
	.release                        = msm_v4l2_close,
	.unlocked_ioctl                 = video_ioctl2,
	.poll                           = msm_v4l2_poll,
};

static struct v4l2_ioctl_ops msm_v4l2_ioctl_ops_enc = {
	.vidioc_querycap                = msm_v4l2_querycap,
	.vidioc_enum_fmt_vid_cap        = msm_v4l2_enum_fmt,
	.vidioc_enum_fmt_vid_out        = msm_v4l2_enum_fmt,
	.vidioc_enum_framesizes         = msm_v4l2_enum_framesizes,
	.vidioc_enum_frameintervals     = msm_v4l2_enum_frameintervals,
	.vidioc_try_fmt_vid_cap_mplane  = msm_v4l2_try_fmt,
	.vidioc_try_fmt_vid_out_mplane  = msm_v4l2_try_fmt,
	.vidioc_s_fmt_vid_cap           = msm_v4l2_s_fmt,
	.vidioc_s_fmt_vid_out           = msm_v4l2_s_fmt,
	.vidioc_s_fmt_vid_cap_mplane    = msm_v4l2_s_fmt,
	.vidioc_s_fmt_vid_out_mplane    = msm_v4l2_s_fmt,
	.vidioc_s_fmt_meta_out          = msm_v4l2_s_fmt,
	.vidioc_s_fmt_meta_cap          = msm_v4l2_s_fmt,
	.vidioc_g_fmt_vid_cap           = msm_v4l2_g_fmt,
	.vidioc_g_fmt_vid_out           = msm_v4l2_g_fmt,
	.vidioc_g_fmt_vid_cap_mplane    = msm_v4l2_g_fmt,
	.vidioc_g_fmt_vid_out_mplane    = msm_v4l2_g_fmt,
	.vidioc_g_fmt_meta_out          = msm_v4l2_g_fmt,
	.vidioc_g_fmt_meta_cap          = msm_v4l2_g_fmt,
	.vidioc_g_selection             = msm_v4l2_g_selection,
	.vidioc_s_selection             = msm_v4l2_s_selection,
	.vidioc_s_parm                  = msm_v4l2_s_parm,
	.vidioc_g_parm                  = msm_v4l2_g_parm,
	.vidioc_reqbufs                 = msm_v4l2_reqbufs,
	.vidioc_qbuf                    = msm_v4l2_qbuf,
	.vidioc_dqbuf                   = msm_v4l2_dqbuf,
	.vidioc_streamon                = msm_v4l2_streamon,
	.vidioc_streamoff               = msm_v4l2_streamoff,
	.vidioc_s_ctrl                  = msm_v4l2_s_ctrl,
	.vidioc_g_ctrl                  = msm_v4l2_g_ctrl,
	.vidioc_queryctrl               = msm_v4l2_queryctrl,
	.vidioc_querymenu               = msm_v4l2_querymenu,
	.vidioc_subscribe_event         = msm_v4l2_subscribe_event,
	.vidioc_unsubscribe_event       = msm_v4l2_unsubscribe_event,
	.vidioc_encoder_cmd             = msm_v4l2_encoder_cmd,
};

static struct v4l2_ioctl_ops msm_v4l2_ioctl_ops_dec = {
	.vidioc_querycap                = msm_v4l2_querycap,
	.vidioc_enum_fmt_vid_cap        = msm_v4l2_enum_fmt,
	.vidioc_enum_fmt_vid_out        = msm_v4l2_enum_fmt,
	.vidioc_enum_framesizes         = msm_v4l2_enum_framesizes,
	.vidioc_enum_frameintervals     = msm_v4l2_enum_frameintervals,
	.vidioc_try_fmt_vid_cap_mplane  = msm_v4l2_try_fmt,
	.vidioc_try_fmt_vid_out_mplane  = msm_v4l2_try_fmt,
	.vidioc_s_fmt_vid_cap           = msm_v4l2_s_fmt,
	.vidioc_s_fmt_vid_out           = msm_v4l2_s_fmt,
	.vidioc_s_fmt_vid_cap_mplane    = msm_v4l2_s_fmt,
	.vidioc_s_fmt_vid_out_mplane    = msm_v4l2_s_fmt,
	.vidioc_s_fmt_meta_out          = msm_v4l2_s_fmt,
	.vidioc_s_fmt_meta_cap          = msm_v4l2_s_fmt,
	.vidioc_g_fmt_vid_cap           = msm_v4l2_g_fmt,
	.vidioc_g_fmt_vid_out           = msm_v4l2_g_fmt,
	.vidioc_g_fmt_vid_cap_mplane    = msm_v4l2_g_fmt,
	.vidioc_g_fmt_vid_out_mplane    = msm_v4l2_g_fmt,
	.vidioc_g_fmt_meta_out          = msm_v4l2_g_fmt,
	.vidioc_g_fmt_meta_cap          = msm_v4l2_g_fmt,
	.vidioc_g_selection             = msm_v4l2_g_selection,
	.vidioc_s_selection             = msm_v4l2_s_selection,
	.vidioc_s_parm                  = msm_v4l2_s_parm,
	.vidioc_g_parm                  = msm_v4l2_g_parm,
	.vidioc_reqbufs                 = msm_v4l2_reqbufs,
	.vidioc_qbuf                    = msm_v4l2_qbuf,
	.vidioc_dqbuf                   = msm_v4l2_dqbuf,
	.vidioc_streamon                = msm_v4l2_streamon,
	.vidioc_streamoff               = msm_v4l2_streamoff,
	.vidioc_s_ctrl                  = msm_v4l2_s_ctrl,
	.vidioc_g_ctrl                  = msm_v4l2_g_ctrl,
	.vidioc_queryctrl               = msm_v4l2_queryctrl,
	.vidioc_querymenu               = msm_v4l2_querymenu,
	.vidioc_subscribe_event         = msm_v4l2_subscribe_event,
	.vidioc_unsubscribe_event       = msm_v4l2_unsubscribe_event,
	.vidioc_decoder_cmd             = msm_v4l2_decoder_cmd,
};

static struct v4l2_ctrl_ops msm_v4l2_ctrl_ops = {
	.s_ctrl                         = msm_v4l2_op_s_ctrl,
};

static struct vb2_ops msm_vb2_ops = {
	.queue_setup                    = msm_vidc_queue_setup,
	.start_streaming                = msm_vidc_start_streaming,
	.buf_queue                      = msm_vidc_buf_queue,
	.buf_cleanup                    = msm_vidc_buf_cleanup,
	.stop_streaming                 = msm_vidc_stop_streaming,
};

static struct vb2_mem_ops msm_vb2_mem_ops = {
	.get_userptr                    = msm_vb2_get_userptr,
	.put_userptr                    = msm_vb2_put_userptr,
	.attach_dmabuf                  = msm_vb2_attach_dmabuf,
	.detach_dmabuf                  = msm_vb2_detach_dmabuf,
	.map_dmabuf                     = msm_vb2_map_dmabuf,
	.unmap_dmabuf                   = msm_vb2_unmap_dmabuf,
};

static struct msm_vidc_platform_ops msm_platform_ops = {
	.buffer_region                  = msm_vidc_get_buffer_region,
};

static int msm_vidc_init_ops(struct msm_vidc_core *core)
{
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s: initialize ops\n", __func__);
	core->v4l2_file_ops = &msm_v4l2_file_operations;
	core->v4l2_ioctl_ops_enc = &msm_v4l2_ioctl_ops_enc;
	core->v4l2_ioctl_ops_dec = &msm_v4l2_ioctl_ops_dec;
	core->v4l2_ctrl_ops = &msm_v4l2_ctrl_ops;
	core->vb2_ops = &msm_vb2_ops;
	core->vb2_mem_ops = &msm_vb2_mem_ops;
	core->platform_ops = &msm_platform_ops;

	return 0;
}

static int msm_vidc_deinit_platform_variant(struct msm_vidc_core *core, struct device *dev)
{
	int rc = -EINVAL;

	if (!core || !dev) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);

#if defined(CONFIG_MSM_VIDC_WAIPIO)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-waipio")) {
		rc = msm_vidc_deinit_platform_waipio(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-waipio with %d\n",
				__func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_DIWALI)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-diwali")) {
		rc = msm_vidc_deinit_platform_diwali(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-diwali with %d\n",
					__func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_KHAJE)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-khaje")) {
		rc = msm_vidc_deinit_platform_khaje(core, dev);
		if (rc)
			d_vpr_e("%s: failed with %d\n", __func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_MONACO)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-monaco")) {
		rc = msm_vidc_deinit_platform_monaco(core, dev);
		if (rc)
			d_vpr_e("%s: failed with %d\n", __func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_NEO)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-neo")) {
		rc = msm_vidc_deinit_platform_neo(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-neo with %d\n",
				__func__, rc);
		return rc;
	}
#endif
#if defined(CONFIG_MSM_VIDC_PARROT)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-parrot")) {
		rc = msm_vidc_deinit_platform_parrot(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-parrot with %d\n",
				__func__, rc);
		return rc;
	}
#endif
#if defined(CONFIG_MSM_VIDC_RAVELIN)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-ravelin")) {
		rc = msm_vidc_deinit_platform_ravelin(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-ravelin with %d\n",
				__func__, rc);
		return rc;
	}
#endif

	return rc;
}

static int msm_vidc_init_platform_variant(struct msm_vidc_core *core, struct device *dev)
{
	int rc = -EINVAL;

	if (!core || !dev) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);

#if defined(CONFIG_MSM_VIDC_WAIPIO)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-waipio")) {
		rc = msm_vidc_init_platform_waipio(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-waipio with %d\n",
				__func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_DIWALI)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-diwali")) {
		rc = msm_vidc_init_platform_diwali(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-diwali with %d\n",
				__func__, rc);
		return rc;
	}
#endif
#if defined(CONFIG_MSM_VIDC_PARROT)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-parrot")) {
		rc = msm_vidc_init_platform_parrot(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-parrot with %d\n",
				__func__, rc);
		return rc;
	}
#endif
#if defined(CONFIG_MSM_VIDC_RAVELIN)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-ravelin")) {
		rc = msm_vidc_init_platform_ravelin(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-ravelin with %d\n",
				__func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_NEO)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-neo")) {
		rc = msm_vidc_init_platform_neo(core, dev);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-neo with %d\n",
				__func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_KHAJE)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-khaje")) {
		rc = msm_vidc_init_platform_khaje(core, dev);
		if (rc)
			d_vpr_e("%s: failed with %d\n", __func__, rc);
		return rc;
	}
#endif

#if defined(CONFIG_MSM_VIDC_MONACO)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-monaco")) {
		rc = msm_vidc_init_platform_monaco(core, dev);
		if (rc)
			d_vpr_e("%s: failed with %d\n", __func__, rc);
		return rc;
	}
#endif

	return rc;
}

static int msm_vidc_deinit_vpu(struct msm_vidc_core *core, struct device *dev)
{
	int rc = -EINVAL;

	if (!core || !dev) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);

#if defined(CONFIG_MSM_VIDC_IRIS2)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-iris2")) {
		rc = msm_vidc_deinit_iris2(core);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-iris2 with %d\n",
				__func__, rc);
	}
#endif

#if defined(CONFIG_MSM_VIDC_IRIS3)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-iris3")) {
		rc = msm_vidc_deinit_iris2(core);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-iris2 with %d\n",
				__func__, rc);
	}
#endif
#if defined(CONFIG_MSM_VIDC_AR50LT)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-ar50lt")) {
		rc = msm_vidc_deinit_ar50lt(core);
		if (rc)
			d_vpr_e("%s: failed with %d\n", __func__, rc);
		return rc;
	}
#endif
	return rc;
}

static int msm_vidc_init_vpu(struct msm_vidc_core *core, struct device *dev)
{
	int rc = -EINVAL;

	if (!core || !dev) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

#if defined(CONFIG_MSM_VIDC_IRIS2)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-iris2")) {
		rc = msm_vidc_init_iris2(core);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-iris2 with %d\n",
				__func__, rc);
	}
#endif

#if defined(CONFIG_MSM_VIDC_IRIS3)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-iris3")) {
		rc = msm_vidc_init_iris2(core);
		if (rc)
			d_vpr_e("%s: failed msm-vidc-iris2 with %d\n",
				__func__, rc);
	}
#endif
#if defined(CONFIG_MSM_VIDC_AR50LT)
	if (of_device_is_compatible(dev->of_node, "qcom,msm-vidc-ar50lt")) {
		rc = msm_vidc_init_ar50lt(core);
		if (rc)
			d_vpr_e("%s: failed with %d\n", __func__, rc);
		return rc;
	}
#endif
	return rc;
}

int msm_vidc_deinit_platform(struct platform_device *pdev)
{
	struct msm_vidc_core *core;

	if (!pdev) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core = dev_get_drvdata(&pdev->dev);
	if (!core) {
		d_vpr_e("%s: core not found in device %s",
			__func__, dev_name(&pdev->dev));
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);

	msm_vidc_deinit_vpu(core, &pdev->dev);
	msm_vidc_deinit_platform_variant(core, &pdev->dev);

	msm_vidc_vmem_free((void **)&core->platform);
	return 0;
}

int msm_vidc_init_platform(struct platform_device *pdev)
{
	int rc = 0;
	struct msm_vidc_platform *platform = NULL;
	struct msm_vidc_core *core;

	if (!pdev) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);

	core = dev_get_drvdata(&pdev->dev);
	if (!core) {
		d_vpr_e("%s: core not found in device %s",
			__func__, dev_name(&pdev->dev));
		return -EINVAL;
	}

	rc = msm_vidc_vmem_alloc(sizeof(struct msm_vidc_platform),
			(void **)&platform, __func__);
	if (rc)
		return rc;

	core->platform = platform;
	platform->core = core;

	/* selected ops can be re-assigned in platform specific file */
	rc = msm_vidc_init_ops(core);
	if (rc)
		return rc;

	rc = msm_vidc_init_platform_variant(core, &pdev->dev);
	if (rc)
		return rc;

	rc = msm_vidc_init_vpu(core, &pdev->dev);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_read_efuse(struct msm_vidc_core *core)
{
	int rc = 0;
	void __iomem *base;
	u32 i = 0, efuse = 0, efuse_data_count = 0;
	struct msm_vidc_efuse_data *efuse_data = NULL;
	struct msm_vidc_platform_data *platform_data;

	if (!core || !core->platform || !core->pdev) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	platform_data = &core->platform->data;
	efuse_data = platform_data->efuse_data;
	efuse_data_count = platform_data->efuse_data_size;

	if (!efuse_data)
		return 0;

	for (i = 0; i < efuse_data_count; i++) {
		switch (efuse_data[i].purpose) {
		case SKU_VERSION:
			base = devm_ioremap(&core->pdev->dev, efuse_data[i].start_address,
					efuse_data[i].size);
			if (!base) {
				d_vpr_e("failed efuse: start %#x, size %d\n",
					efuse_data[i].start_address,
					efuse_data[i].size);
				return -EINVAL;
			}
			efuse = readl_relaxed(base);
			platform_data->sku_version =
					(efuse & efuse_data[i].mask) >>
					efuse_data[i].shift;
			break;
		default:
			break;
		}
		if (platform_data->sku_version) {
			d_vpr_h("efuse 0x%x, platform version 0x%x\n",
				efuse, platform_data->sku_version);
			break;
		}
	}
	return rc;
}

void msm_vidc_ddr_ubwc_config(
	struct msm_vidc_platform_data *platform_data, u32 hbb_override_val)
{
	uint32_t ddr_type = DDR_TYPE_LPDDR5;

	if (!platform_data || !platform_data->ubwc_config) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

#if (KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
	ddr_type = of_fdt_get_ddrtype();
	if (ddr_type == -ENOENT)
		d_vpr_e("Failed to get ddr type, use LPDDR5\n");
#endif

	if (platform_data->ubwc_config &&
		(ddr_type == DDR_TYPE_LPDDR4 ||
		 ddr_type == DDR_TYPE_LPDDR4X))
		platform_data->ubwc_config->highest_bank_bit = hbb_override_val;

	d_vpr_h("DDR Type 0x%x hbb 0x%x\n",
		ddr_type, platform_data->ubwc_config ?
		platform_data->ubwc_config->highest_bank_bit : -1);
}

void msm_vidc_sort_table(struct msm_vidc_core *core)
{
	u32 i = 0;

	if (!core || !core->dt || !core->dt->allowed_clks_tbl) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	sort(core->dt->allowed_clks_tbl, core->dt->allowed_clks_tbl_size,
		sizeof(*core->dt->allowed_clks_tbl), cmp, NULL);
	d_vpr_h("Updated allowed clock rates\n");
	for (i = 0; i < core->dt->allowed_clks_tbl_size; i++)
		d_vpr_h("    %d\n", core->dt->allowed_clks_tbl[i]);
}
