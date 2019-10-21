/* Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/jiffies.h>
#include <ipc/gpr-lite.h>
#include <dsp/gecko-core.h>

#define APM_STATE_READY_TIMEOUT_MS    10000
#define Q6_READY_TIMEOUT_MS 1000
#define APM_CMD_GET_GECKO_STATE 0x01001021
#define APM_CMD_RSP_GET_GECKO_STATE 0x02001007
#define APM_MODULE_INSTANCE_ID   0x00000001
#define GPR_SVC_ADSP_CORE 0x3

struct gecko_core {
	struct gpr_device *adev;
	wait_queue_head_t wait;
	struct mutex lock;
	bool resp_received;
	bool is_ready;
};

struct gecko_core_private {
        struct device *dev;
	struct mutex lock;
        struct gecko_core *gecko_core_drv;
        bool is_initial_boot;
        struct work_struct add_chld_dev_work;
};

static struct gecko_core_private *gecko_core_priv;
struct apm_cmd_rsp_get_gecko_status_t

{
	/* Gecko status
	 * @values
	 * 0 -> Not ready
	 * 1 -> Ready
	 */
	uint32_t status;

};

static int gecko_core_callback(struct gpr_device *adev, void *data)
{
	struct gecko_core *core = dev_get_drvdata(&adev->dev);
	struct apm_cmd_rsp_get_gecko_status_t *result;
	struct gpr_hdr *hdr = data;

	result = GPR_PKT_GET_PAYLOAD(struct apm_cmd_rsp_get_gecko_status_t, data);

	dev_err(&adev->dev ,"%s: Payload %x",__func__, hdr->opcode);
	switch (hdr->opcode) {
	case GPR_IBASIC_RSP_RESULT:
		dev_err(&adev->dev ,"%s: Failed response received",__func__);
		core->resp_received = true;
		break;
	case APM_CMD_RSP_GET_GECKO_STATE:
		dev_err(&adev->dev ,"%s: sucess response received",__func__);
		core->is_ready = result->status;
		core->resp_received = true;
		break;
	default:
		dev_err(&adev->dev, "Message ID from apm: 0x%x\n",
			hdr->opcode);
		break;
	}
	if (core->resp_received)
		wake_up(&core->wait);

	return 0;
}

static bool __gecko_core_is_apm_ready(struct gecko_core *core)
{
	struct gpr_device *adev = core->adev;
	struct gpr_pkt pkt;
	int rc;

	pkt.hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
			 GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
			 GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, GPR_PKT_HEADER_BYTE_SIZE_V);

	pkt.hdr.opcode = APM_CMD_GET_GECKO_STATE;
	pkt.hdr.dst_port = APM_MODULE_INSTANCE_ID;
	pkt.hdr.src_port = GPR_SVC_ADSP_CORE;
	pkt.hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
	pkt.hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
	pkt.hdr.opcode = APM_CMD_GET_GECKO_STATE;

	dev_err(gecko_core_priv->dev, "%s: send_command ret\n",	__func__);

	rc = gpr_send_pkt(adev, &pkt);
	if (rc < 0)
		return false;

	rc = wait_event_timeout(core->wait, (core->resp_received),
				msecs_to_jiffies(Q6_READY_TIMEOUT_MS));
	dev_err(gecko_core_priv->dev, "%s: wait event unblocked \n", __func__);
//	core->resp_received = true;
//	core->is_ready = true;
	if (rc > 0 && core->resp_received) {
		core->resp_received = false;

		if (core->is_ready)
			return true;
	} else {
		dev_err(gecko_core_priv->dev, "%s: command timedout, ret\n",
			__func__);
        }

	return false;
}

/**
 * gecko_core_is_apm_ready() - Get status of adsp
 *
 * Return: Will be an true if apm is ready and false if not.
 */
bool gecko_core_is_apm_ready(void)
{
	unsigned long  timeout;
	bool ret = false;
	struct gecko_core *core;

	if (!gecko_core_priv)
		return 0;

	core = gecko_core_priv->gecko_core_drv;
	if (!core)
		return 0;

	mutex_lock(&core->lock);
	timeout = jiffies + msecs_to_jiffies(APM_STATE_READY_TIMEOUT_MS);
	for (;;) {
		if (__gecko_core_is_apm_ready(core)) {
			ret = true;
			break;
		}
		usleep_range(300000, 300050);
		if (!time_after(timeout, jiffies)) {
			ret = false;
			break;
		}
	}

	mutex_unlock(&core->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(gecko_core_is_apm_ready);

static int gecko_core_probe(struct gpr_device *adev)
{
	struct gecko_core *core;
	pr_err("%s",__func__);
	if (!gecko_core_priv) {
		pr_err("%s: gecko_core platform probe not yet done\n", __func__);
		return -EPROBE_DEFER;
	}
	mutex_lock(&gecko_core_priv->lock);
	core = kzalloc(sizeof(*core), GFP_KERNEL);
	if (!core)
		return -ENOMEM;

	dev_set_drvdata(&adev->dev, core);

	mutex_init(&core->lock);
	core->adev = adev;
	init_waitqueue_head(&core->wait);
	gecko_core_priv->gecko_core_drv = core;
	if (gecko_core_priv->is_initial_boot)
		schedule_work(&gecko_core_priv->add_chld_dev_work);
	mutex_unlock(&gecko_core_priv->lock);

	return 0;
}

static int gecko_core_exit(struct gpr_device *adev)
{
	struct gecko_core *core = dev_get_drvdata(&adev->dev);
	if (!gecko_core_priv) {
		pr_err("%s: gecko_core platform probe not yet done\n", __func__);
		return -1;
	}
	mutex_lock(&gecko_core_priv->lock);
	gecko_core_priv->gecko_core_drv = NULL;
	kfree(core);
        mutex_unlock(&gecko_core_priv->lock);
	return 0;
}

static const struct of_device_id gecko_core_device_id[]  = {
	{ .compatible = "qcom,gecko_core" },
	{},
};
MODULE_DEVICE_TABLE(of, gecko_core_device_id);

static struct gpr_driver qcom_gecko_core_driver = {
	.probe = gecko_core_probe,
	.remove = gecko_core_exit,
	.callback = gecko_core_callback,
	.driver = {
		.name = "qcom-gecko_core",
		.of_match_table = of_match_ptr(gecko_core_device_id),
	},
};

static void gecko_core_add_child_devices(struct work_struct *work)
{
	int ret;
        pr_err("%s:enumarate machine driver\n", __func__);

	if(gecko_core_is_apm_ready()) {
		dev_err(gecko_core_priv->dev, "%s: apm is up\n",
			__func__);
	} else {
		dev_err(gecko_core_priv->dev, "%s: apm is not up\n",
			__func__);
		return;
	}

	ret = of_platform_populate(gecko_core_priv->dev->of_node,
			NULL, NULL, gecko_core_priv->dev);
	if (ret)
		dev_err(gecko_core_priv->dev, "%s: failed to add child nodes, ret=%d\n",
			__func__, ret);

        gecko_core_priv->is_initial_boot = false;

}

static int gecko_core_platform_driver_probe(struct platform_device *pdev)
{
	int ret = 0;
        pr_err("%s",__func__);

	gecko_core_priv = devm_kzalloc(&pdev->dev, sizeof(struct gecko_core_private), GFP_KERNEL);
	if (!gecko_core_priv)
		return -ENOMEM;

	gecko_core_priv->dev = &pdev->dev;

	mutex_init(&gecko_core_priv->lock);

	INIT_WORK(&gecko_core_priv->add_chld_dev_work, gecko_core_add_child_devices);

        ret = gpr_driver_register(&qcom_gecko_core_driver);
        if (ret) {
		pr_err("%s: gpr driver register failed = %d\n",
			__func__, ret);
		ret = 0;
	}
        gecko_core_priv->is_initial_boot = true;

#if 0
	ret = snd_event_client_register(&pdev->dev, &gpr_ssr_ops, NULL);
	if (ret) {
		pr_err("%s: Registration with SND event fwk failed ret = %d\n",
			__func__, ret);
		ret = 0;
	}
#endif

	return ret;
}

static int gecko_core_platform_driver_remove(struct platform_device *pdev)
{
	//snd_event_client_deregister(&pdev->dev);
        gpr_driver_unregister(&qcom_gecko_core_driver);
	gecko_core_priv = NULL;
	return 0;
}

static const struct of_device_id gecko_core_of_match[]  = {
	{ .compatible = "qcom,gecko-core-platform", },
	{},
};

static struct platform_driver gecko_core_driver = {
	.probe = gecko_core_platform_driver_probe,
	.remove = gecko_core_platform_driver_remove,
	.driver = {
		.name = "gecko-core-platform",
		.owner = THIS_MODULE,
		.of_match_table = gecko_core_of_match,
	}
};

module_platform_driver(gecko_core_driver);

MODULE_DESCRIPTION("q6 core");
MODULE_LICENSE("GPL v2");
