// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2020-2022, The Linux Foundation. All rights reserved.
 */

#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>

#include "msm_vidc_internal.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_core.h"
#include "msm_vidc_memory.h"
#include "venus_hfi.h"

#define BASE_DEVICE_NUMBER 32

struct msm_vidc_core *g_core;

static int msm_vidc_deinit_irq(struct msm_vidc_core *core)
{
	struct msm_vidc_dt *dt;

	if (!core || !core->pdev || !core->dt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	dt = core->dt;
	d_vpr_h("%s: reg_base = %pa, reg_size = %#x\n",
		__func__, &dt->register_base, dt->register_size);

	if (dt->irq)
		free_irq(dt->irq, core);
	dt->irq = 0;

	if (core->register_base_addr)
		devm_iounmap(&core->pdev->dev, core->register_base_addr);
	core->register_base_addr = 0;
	return 0;
}

static int msm_vidc_init_irq(struct msm_vidc_core *core)
{
	int rc = 0;
	struct msm_vidc_dt *dt;

	if (!core || !core->pdev || !core->dt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	dt = core->dt;

	core->register_base_addr = devm_ioremap(&core->pdev->dev,
			dt->register_base, dt->register_size);
	if (!core->register_base_addr) {
		d_vpr_e("could not map reg addr %pa of size %d\n",
			&dt->register_base, dt->register_size);
		rc = -EINVAL;
		goto exit;
	}

	rc = devm_request_threaded_irq(&core->pdev->dev, dt->irq, venus_hfi_isr,
			venus_hfi_isr_handler, IRQF_TRIGGER_HIGH, "msm-vidc", core);
	if (rc) {
		d_vpr_e("%s: Failed to allocate venus IRQ\n", __func__);
		goto exit;
	}
	disable_irq_nosync(dt->irq);

	d_vpr_h("%s: reg_base = %pa, reg_size = %d\n",
		__func__, &dt->register_base, dt->register_size);

	return 0;

exit:
	msm_vidc_deinit_irq(core);
	return rc;
}

static ssize_t sku_version_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct msm_vidc_core *core;

	/*
	 * Default sku version: 0
	 * driver possibly not probed yet or not the main device.
	 */
	if (!dev || !dev->driver ||
		!of_device_is_compatible(dev->of_node, "qcom,msm-vidc"))
		return 0;

	core = dev_get_drvdata(dev);
	if (!core || !core->platform) {
		d_vpr_e("%s: invalid core\n", __func__);
		return 0;
	}

	return scnprintf(buf, PAGE_SIZE, "%d",
			core->platform->data.sku_version);
}

static DEVICE_ATTR_RO(sku_version);

static struct attribute *msm_vidc_core_attrs[] = {
	&dev_attr_sku_version.attr,
	NULL
};

static struct attribute_group msm_vidc_core_attr_group = {
	.attrs = msm_vidc_core_attrs,
};

static const struct of_device_id msm_vidc_dt_match[] = {
	{.compatible = "qcom,msm-vidc"},
	{.compatible = "qcom,msm-vidc,context-bank"},
	MSM_VIDC_EMPTY_BRACE
};
MODULE_DEVICE_TABLE(of, msm_vidc_dt_match);

static void msm_vidc_release_video_device(struct video_device *vdev)
{
	d_vpr_e("%s:\n", __func__);
}

static void msm_vidc_unregister_video_device(struct msm_vidc_core *core,
		enum msm_vidc_domain_type type)
{
	int index;

	d_vpr_h("%s()\n", __func__);

	if (type == MSM_VIDC_DECODER)
		index = 0;
	else if (type == MSM_VIDC_ENCODER)
		index = 1;
	else
		return;

	//rc = device_create_file(&core->vdev[index].vdev.dev, &dev_attr_link_name);
	video_set_drvdata(&core->vdev[index].vdev, NULL);
	video_unregister_device(&core->vdev[index].vdev);
	//memset vdev to 0
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
	if (type == MSM_VIDC_DECODER)
		core->vdev[index].vdev.ioctl_ops = core->v4l2_ioctl_ops_dec;
	else
		core->vdev[index].vdev.ioctl_ops = core->v4l2_ioctl_ops_enc;
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
					VFL_TYPE_VIDEO, nr);
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

#ifdef CONFIG_MSM_MMRM
static int msm_vidc_check_mmrm_support(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core || !core->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!core->capabilities[MMRM].value)
		goto exit;

	if (!mmrm_client_check_scaling_supported(MMRM_CLIENT_CLOCK, 0)) {
		d_vpr_e("%s: MMRM not supported\n", __func__);
		core->capabilities[MMRM].value = 0;
	}

exit:
	d_vpr_h("%s: %d\n", __func__, core->capabilities[MMRM].value);
	return rc;
}
#else
static int msm_vidc_check_mmrm_support(struct msm_vidc_core *core)
{
	return 0;
}
#endif

static int msm_vidc_deinitialize_core(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s()\n", __func__);

	mutex_destroy(&core->lock);
	msm_vidc_change_core_state(core, MSM_VIDC_CORE_DEINIT, __func__);

	msm_vidc_vmem_free((void **)&core->response_packet);
	msm_vidc_vmem_free((void **)&core->packet);
	core->response_packet = NULL;
	core->packet = NULL;

	if (core->batch_workq)
		destroy_workqueue(core->batch_workq);

	if (core->pm_workq)
		destroy_workqueue(core->pm_workq);

	core->batch_workq = NULL;
	core->pm_workq = NULL;

	return rc;
}

static int msm_vidc_initialize_core(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s()\n", __func__);

	msm_vidc_change_core_state(core, MSM_VIDC_CORE_DEINIT, __func__);

	core->pm_workq = create_singlethread_workqueue("pm_workq");
	if (!core->pm_workq) {
		d_vpr_e("%s: create pm workq failed\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	core->batch_workq = create_singlethread_workqueue("batch_workq");
	if (!core->batch_workq) {
		d_vpr_e("%s: create batch workq failed\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	core->packet_size = 4096;
	rc = msm_vidc_vmem_alloc(core->packet_size,
			(void **)&core->packet, "core packet");
	if (rc)
		goto exit;

	rc = msm_vidc_vmem_alloc(core->packet_size,
			(void **)&core->response_packet, "core response packet");
	if (rc)
		goto exit;

	mutex_init(&core->lock);
	init_completion(&core->init_done);
	INIT_LIST_HEAD(&core->instances);
	INIT_LIST_HEAD(&core->dangling_instances);

	INIT_DELAYED_WORK(&core->pm_work, venus_hfi_pm_work_handler);
	INIT_DELAYED_WORK(&core->fw_unload_work, msm_vidc_fw_unload_handler);
	INIT_WORK(&core->ssr_work, msm_vidc_ssr_handler);

	return 0;
exit:
	msm_vidc_vmem_free((void **)&core->response_packet);
	msm_vidc_vmem_free((void **)&core->packet);
	core->response_packet = NULL;
	core->packet = NULL;
	if (core->batch_workq)
		destroy_workqueue(core->batch_workq);
	if (core->pm_workq)
		destroy_workqueue(core->pm_workq);
	core->batch_workq = NULL;
	core->pm_workq = NULL;

	return rc;
}

static int msm_vidc_remove(struct platform_device* pdev)
{
	struct msm_vidc_core* core;

	if (!pdev) {
		d_vpr_e("%s: invalid input %pK", __func__, pdev);
		return -EINVAL;
	}
	core = dev_get_drvdata(&pdev->dev);
	if (!core) {
		d_vpr_e("%s: invalid core", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);

	msm_vidc_core_deinit(core, true);

	venus_hfi_interface_queues_deinit(core);

	msm_vidc_unregister_video_device(core, MSM_VIDC_ENCODER);
	msm_vidc_unregister_video_device(core, MSM_VIDC_DECODER);
	//device_remove_file(&core->vdev[MSM_VIDC_ENCODER].vdev.dev,
		//&dev_attr_link_name);
	//device_remove_file(&core->vdev[MSM_VIDC_DECODER].vdev.dev,
		//&dev_attr_link_name);
	v4l2_device_unregister(&core->v4l2_dev);
	sysfs_remove_group(&pdev->dev.kobj, &msm_vidc_core_attr_group);

	msm_vidc_deinit_instance_caps(core);
	msm_vidc_deinit_core_caps(core);

	msm_vidc_deinit_irq(core);
	msm_vidc_deinit_platform(pdev);
	msm_vidc_deinit_dt(pdev);
	msm_vidc_deinitialize_core(core);

	dev_set_drvdata(&pdev->dev, NULL);
	debugfs_remove_recursive(core->debugfs_parent);
	msm_vidc_vmem_free((void **)&core);
	g_core = NULL;

	return 0;
}

static int msm_vidc_probe_video_device(struct platform_device *pdev)
{
	int rc = 0;
	struct msm_vidc_core *core = NULL;
	int nr = BASE_DEVICE_NUMBER;

	d_vpr_h("%s()\n", __func__);

	rc = msm_vidc_vmem_alloc(sizeof(*core), (void **)&core, __func__);
	if (rc)
		return rc;
	g_core = core;

	core->debugfs_parent = msm_vidc_debugfs_init_drv();
	if (!core->debugfs_parent)
		d_vpr_h("Failed to create debugfs for msm_vidc\n");

	core->pdev = pdev;
	dev_set_drvdata(&pdev->dev, core);

	rc = msm_vidc_initialize_core(core);
	if (rc) {
		d_vpr_e("%s: init core failed with %d\n", __func__, rc);
		goto init_core_failed;
	}

	rc = msm_vidc_init_dt(pdev);
	if (rc) {
		d_vpr_e("%s: init dt failed with %d\n", __func__, rc);
		rc = -EINVAL;
		goto init_dt_failed;
	}

	rc = msm_vidc_init_platform(pdev);
	if (rc) {
		d_vpr_e("%s: init platform failed with %d\n", __func__, rc);
		rc = -EINVAL;
		goto init_plat_failed;
	}

	rc = msm_vidc_init_irq(core);
	if (rc) {
		d_vpr_e("%s: init irq failed with %d\n", __func__, rc);
		goto init_irq_failed;
	}

	rc = msm_vidc_init_core_caps(core);
	if (rc) {
		d_vpr_e("%s: init core caps failed with %d\n", __func__, rc);
		goto init_core_caps_fail;
	}

	rc = msm_vidc_init_instance_caps(core);
	if (rc) {
		d_vpr_e("%s: init inst cap failed with %d\n", __func__, rc);
		goto init_inst_caps_fail;
	}

	rc = sysfs_create_group(&pdev->dev.kobj, &msm_vidc_core_attr_group);
	if (rc) {
		d_vpr_e("Failed to create attributes\n");
		goto init_group_failed;
	}

	rc = v4l2_device_register(&pdev->dev, &core->v4l2_dev);
	if (rc) {
		d_vpr_e("Failed to register v4l2 device\n");
		goto v4l2_reg_failed;
	}

	/* setup the decoder device */
	rc = msm_vidc_register_video_device(core, MSM_VIDC_DECODER, nr);
	if (rc) {
		d_vpr_e("Failed to register video decoder\n");
		goto dec_reg_failed;
	}

	/* setup the encoder device */
	rc = msm_vidc_register_video_device(core, MSM_VIDC_ENCODER, nr + 1);
	if (rc) {
		d_vpr_e("Failed to register video encoder\n");
		goto enc_reg_failed;
	}

	rc = msm_vidc_check_mmrm_support(core);
	if (rc) {
		d_vpr_e("Failed to check MMRM scaling support\n");
		rc = 0; /* Ignore error */
	}

	core->debugfs_root = msm_vidc_debugfs_init_core(core);
	if (!core->debugfs_root)
		d_vpr_h("Failed to init debugfs core\n");

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
		goto sub_dev_failed;
	}

	return rc;

sub_dev_failed:
	msm_vidc_unregister_video_device(core, MSM_VIDC_ENCODER);
enc_reg_failed:
	msm_vidc_unregister_video_device(core, MSM_VIDC_DECODER);
dec_reg_failed:
	v4l2_device_unregister(&core->v4l2_dev);
v4l2_reg_failed:
	sysfs_remove_group(&pdev->dev.kobj, &msm_vidc_core_attr_group);
init_group_failed:
	msm_vidc_deinit_instance_caps(core);
init_inst_caps_fail:
	msm_vidc_deinit_core_caps(core);
init_core_caps_fail:
	msm_vidc_deinit_irq(core);
init_irq_failed:
	msm_vidc_deinit_platform(pdev);
init_plat_failed:
	msm_vidc_deinit_dt(pdev);
init_dt_failed:
	msm_vidc_deinitialize_core(core);
init_core_failed:
	dev_set_drvdata(&pdev->dev, NULL);
	debugfs_remove_recursive(core->debugfs_parent);
	msm_vidc_vmem_free((void **)&core);
	g_core = NULL;

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
	WARN_ON(1);
	return -EINVAL;
}

static int msm_vidc_pm_suspend(struct device *dev)
{
	int rc = 0;
	struct msm_vidc_core *core;

	/*
	 * Bail out if
	 * - driver possibly not probed yet
	 * - not the main device. We don't support power management on
	 *   subdevices (e.g. context banks)
	 */
	if (!dev || !dev->driver ||
		!of_device_is_compatible(dev->of_node, "qcom,msm-vidc"))
		return 0;

	core = dev_get_drvdata(dev);
	if (!core) {
		d_vpr_e("%s: invalid core\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s\n", __func__);
#ifdef CONFIG_DEEPSLEEP
	if (pm_suspend_via_firmware()) {
		d_vpr_l("%s : deepsleep is triggered\n", __func__);
		rc = msm_vidc_schedule_core_deinit(core, true);
	} else {
		rc = msm_vidc_suspend(core);
	}
#else
	rc = msm_vidc_suspend(core);
#endif
	if (rc == -ENOTSUPP)
		rc = 0;
	else if (rc)
		d_vpr_e("Failed to suspend: %d\n", rc);
	else
		core->pm_suspended  = true;

	return rc;
}

static int msm_vidc_pm_resume(struct device *dev)
{
	struct msm_vidc_core *core;

	/*
	 * Bail out if
	 * - driver possibly not probed yet
	 * - not the main device. We don't support power management on
	 *   subdevices (e.g. context banks)
	 */
	if (!dev || !dev->driver ||
		!of_device_is_compatible(dev->of_node, "qcom,msm-vidc"))
		return 0;

	core = dev_get_drvdata(dev);
	if (!core) {
		d_vpr_e("%s: invalid core\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s\n", __func__);
	core->pm_suspended  = false;
	return 0;
}

static const struct dev_pm_ops msm_vidc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(msm_vidc_pm_suspend, msm_vidc_pm_resume)
};

struct platform_driver msm_vidc_driver = {
	.probe = msm_vidc_probe,
	.remove = msm_vidc_remove,
	.driver = {
		.name = "msm_vidc_v4l2",
		.of_match_table = msm_vidc_dt_match,
		.pm = &msm_vidc_pm_ops,
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

MODULE_SOFTDEP("pre: subsys-pil-tz msm-mmrm");
MODULE_LICENSE("GPL v2");
