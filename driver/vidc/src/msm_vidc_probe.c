// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>

#include "msm_vidc_internal.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_core.h"
#include "venus_hfi.h"

#define BASE_DEVICE_NUMBER 32

static irqreturn_t msm_vidc_isr(int irq, void *data)
{
	struct msm_vidc_core *core = data;

	d_vpr_e("%s()\n", __func__);

	disable_irq_nosync(irq);
	queue_work(core->device_workq, &core->device_work);

	return IRQ_HANDLED;
}

static int msm_vidc_init_irq(struct msm_vidc_core *core)
{
	int rc = 0;
	struct msm_vidc_dt *dt;

	d_vpr_e("%s()\n", __func__);

	if (!core || !core->pdev || !core->dt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	dt = core->dt;

	core->register_base_addr = devm_ioremap_nocache(&core->pdev->dev,
			dt->register_base, dt->register_size);
	if (!core->register_base_addr) {
		d_vpr_e("could not map reg addr %pa of size %d\n",
			&dt->register_base, dt->register_size);
		goto exit;
	}

	rc = request_irq(dt->irq, msm_vidc_isr, IRQF_TRIGGER_HIGH,
				     "msm_vidc", core);
	if (unlikely(rc)) {
		d_vpr_e("%s: request_irq failed\n", __func__);
		goto exit;
	}
	disable_irq_nosync(dt->irq);

	d_vpr_h("%s: reg_base = %pa, reg_size = %d\n",
		__func__, &dt->register_base, dt->register_size);

	return 0;

exit:
	if (core->device_workq)
		destroy_workqueue(core->device_workq);

	return rc;
}

static struct attribute *msm_vidc_core_attrs[] = {
	NULL
};

static struct attribute_group msm_vidc_core_attr_group = {
	.attrs = msm_vidc_core_attrs,
};

static const struct of_device_id msm_vidc_dt_match[] = {
	{.compatible = "qcom,msm-vidc"},
	{.compatible = "qcom,msm-vidc,context-bank"},
	{}
};
MODULE_DEVICE_TABLE(of, msm_vidc_dt_match);


void msm_vidc_release_video_device(struct video_device *vdev)
{
	d_vpr_e("%s:\n", __func__);
}

static int msm_vidc_register_video_device(struct msm_vidc_core *core,
		enum msm_vidc_domain_type type, int nr)
{
	int rc = 0;
	int index;

	d_vpr_h("%s()\n", __func__);

	if (type == MSM_VIDC_DECODER)
		index = 0;
	else if (type == MSM_VIDC_ENCODER)
		index = 1;
	else
		return -EINVAL;

	core->vdev[index].vdev.release =
		msm_vidc_release_video_device;
	core->vdev[index].vdev.fops = core->v4l2_file_ops;
	core->vdev[index].vdev.ioctl_ops = core->v4l2_ioctl_ops;
	core->vdev[index].vdev.vfl_dir = VFL_DIR_M2M;
	core->vdev[index].type = type;
	core->vdev[index].vdev.v4l2_dev = &core->v4l2_dev;
	core->vdev[index].vdev.device_caps =
		V4L2_CAP_VIDEO_CAPTURE_MPLANE |
		V4L2_CAP_VIDEO_OUTPUT_MPLANE |
		V4L2_CAP_META_CAPTURE |
		V4L2_CAP_META_OUTPUT |
		V4L2_CAP_STREAMING;
	rc = video_register_device(&core->vdev[index].vdev,
					VFL_TYPE_GRABBER, nr);
	if (rc) {
		d_vpr_e("Failed to register the video device\n");
		return rc;
	}
	video_set_drvdata(&core->vdev[index].vdev, core);
	//rc = device_create_file(&core->vdev[index].vdev.dev, &dev_attr_link_name);
	if (rc) {
		d_vpr_e("Failed to create video device file\n");
		video_unregister_device(&core->vdev[index].vdev);
		return rc;
	}

	return 0;
}

static int msm_vidc_initialize_core(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s()\n", __func__);

	core->state = MSM_VIDC_CORE_DEINIT;

	core->device_workq = create_singlethread_workqueue("device_workq");
	if (!core->device_workq) {
		d_vpr_e("%s: create device workq failed\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	core->pm_workq = create_singlethread_workqueue("pm_workq");
	if (!core->pm_workq) {
		d_vpr_e("%s: create pm workq failed\n", __func__);
		destroy_workqueue(core->device_workq);
		rc = -EINVAL;
		goto exit;
	}

	mutex_init(&core->lock);
	INIT_LIST_HEAD(&core->instances);
	INIT_LIST_HEAD(&core->dangling_instances);

	INIT_WORK(&core->device_work, venus_hfi_work_handler);
	INIT_DELAYED_WORK(&core->pm_work, venus_hfi_pm_work_handler);
	INIT_DELAYED_WORK(&core->fw_unload_work, msm_vidc_fw_unload_handler);
	INIT_DELAYED_WORK(&core->batch_work, msm_vidc_batch_handler);
	INIT_WORK(&core->ssr_work, msm_vidc_ssr_handler);

exit:
	return rc;
}

static int msm_vidc_probe_video_device(struct platform_device *pdev)
{
	int rc = 0;
	struct msm_vidc_core *core;
	int nr = BASE_DEVICE_NUMBER;

	d_vpr_h("%s()\n", __func__);

	core = kzalloc(sizeof(*core), GFP_KERNEL);
	if (!core)
		return -ENOMEM;

	core->pdev = pdev;
	dev_set_drvdata(&pdev->dev, core);

	rc = msm_vidc_initialize_core(core);
	if (rc) {
		d_vpr_e("%s: init core failed with %d\n", __func__, rc);
		goto exit;
	}

	rc = msm_vidc_init_dt(pdev);
	if (rc) {
		d_vpr_e("%s: init dt failed with %d\n", __func__, rc);
		return -EINVAL;
	}

	rc = msm_vidc_init_platform(pdev);
	if (rc) {
		d_vpr_e("%s: init platform failed with %d\n", __func__, rc);
		return -EINVAL;
	}

	rc = msm_vidc_init_irq(core);
	if (rc)
		return rc;

	rc = sysfs_create_group(&pdev->dev.kobj, &msm_vidc_core_attr_group);
	if (rc) {
		d_vpr_e("Failed to create attributes\n");
		goto exit;
	}

	rc = v4l2_device_register(&pdev->dev, &core->v4l2_dev);
	if (rc) {
		d_vpr_e("Failed to register v4l2 device\n");
		goto exit;
	}

	/* setup the decoder device */
	rc = msm_vidc_register_video_device(core, MSM_VIDC_DECODER, nr);
	if (rc) {
		d_vpr_e("Failed to register video decoder\n");
		goto exit;
	}

	/* setup the encoder device */
	rc = msm_vidc_register_video_device(core, MSM_VIDC_ENCODER, nr + 1);
	if (rc) {
		d_vpr_e("Failed to register video encoder\n");
		goto exit;
	}

	//rc = msm_vidc_debugfs_init_core(core);

	d_vpr_h("populating sub devices\n");
	/*
	 * Trigger probe for each sub-device i.e. qcom,msm-vidc,context-bank.
	 * When msm_vidc_probe is called for each sub-device, parse the
	 * context-bank details and store it in core->resources.context_banks
	 * list.
	 */
	rc = of_platform_populate(pdev->dev.of_node, msm_vidc_dt_match, NULL,
			&pdev->dev);
	if (rc) {
		d_vpr_e("Failed to trigger probe for sub-devices\n");
		goto exit;
	}

exit:
	return rc;
}

static int msm_vidc_probe_context_bank(struct platform_device *pdev)
{
	d_vpr_h("%s()\n", __func__);

	return msm_vidc_read_context_bank_resources_from_dt(pdev);
}

static int msm_vidc_probe(struct platform_device *pdev)
{
	d_vpr_h("%s()\n", __func__);

	/*
	 * Sub devices probe will be triggered by of_platform_populate() towards
	 * the end of the probe function after msm-vidc device probe is
	 * completed. Return immediately after completing sub-device probe.
	 */
	if (of_device_is_compatible(pdev->dev.of_node, "qcom,msm-vidc")) {
		return msm_vidc_probe_video_device(pdev);
	} else if (of_device_is_compatible(pdev->dev.of_node,
				"qcom,msm-vidc,context-bank")) {
		return msm_vidc_probe_context_bank(pdev);
	}

	/* How did we end up here? */
	MSM_VIDC_ERROR(1);
	return -EINVAL;
}

static int msm_vidc_remove(struct platform_device *pdev)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);

/*
	struct msm_vidc_core *core;

	if (!pdev) {
		d_vpr_e("%s: invalid input %pK", __func__, pdev);
		return -EINVAL;
	}

	core = dev_get_drvdata(&pdev->dev);
	if (!core) {
		d_vpr_e("%s: invalid core", __func__);
		return -EINVAL;
	}

	if (core->vidc_core_workq)
		destroy_workqueue(core->vidc_core_workq);
	vidc_hfi_deinitialize(core->hfi_type, core->device);
	device_remove_file(&core->vdev[MSM_VIDC_ENCODER].vdev.dev,
				&dev_attr_link_name);
	video_unregister_device(&core->vdev[MSM_VIDC_ENCODER].vdev);
	device_remove_file(&core->vdev[MSM_VIDC_DECODER].vdev.dev,
				&dev_attr_link_name);
	video_unregister_device(&core->vdev[MSM_VIDC_DECODER].vdev);
	v4l2_device_unregister(&core->v4l2_dev);

	//msm_vidc_free_platform_resources(&core->resources);
	sysfs_remove_group(&pdev->dev.kobj, &msm_vidc_core_attr_group);
	dev_set_drvdata(&pdev->dev, NULL);
	mutex_destroy(&core->lock);
	kfree(core);
*/
	return rc;
}

static struct platform_driver msm_vidc_driver = {
	.probe = msm_vidc_probe,
	.remove = msm_vidc_remove,
	.driver = {
		.name = "msm_vidc_v4l2",
		.of_match_table = msm_vidc_dt_match,
	},
};

static int __init msm_vidc_init(void)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);

	rc = platform_driver_register(&msm_vidc_driver);
	if (rc) {
		d_vpr_e("Failed to register platform driver\n");
		return rc;
	}

	return 0;
}

static void __exit msm_vidc_exit(void)
{
	d_vpr_h("%s()\n", __func__);

	platform_driver_unregister(&msm_vidc_driver);
}

module_init(msm_vidc_init);
module_exit(msm_vidc_exit);

MODULE_SOFTDEP("pre: subsys-pil-tz");
MODULE_LICENSE("GPL v2");
