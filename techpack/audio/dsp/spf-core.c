/* Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
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
#include <dsp/spf-core.h>
#include <dsp/digital-cdc-rsc-mgr.h>

#define APM_STATE_READY_TIMEOUT_MS    10000
#define Q6_READY_TIMEOUT_MS 1000
#define Q6_CLOSE_ALL_TIMEOUT_MS 5000
#define APM_CMD_GET_SPF_STATE 0x01001021
#define APM_CMD_CLOSE_ALL 0x01001013
#define APM_CMD_RSP_GET_SPF_STATE 0x02001007
#define APM_MODULE_INSTANCE_ID   0x00000001
#define GPR_SVC_ADSP_CORE 0x3

struct spf_core {
	struct gpr_device *adev;
	wait_queue_head_t wait;
	struct mutex lock;
	bool resp_received;
	int32_t status;
};

struct spf_core_private {
        struct device *dev;
	struct mutex lock;
        struct spf_core *spf_core_drv;
        bool is_initial_boot;
        struct work_struct add_chld_dev_work;
};

static struct spf_core_private *spf_core_priv;

/* used to decode basic responses from Gecko */
struct spf_cmd_basic_rsp {
	uint32_t opcode;
	int32_t status;
};

struct apm_cmd_rsp_get_spf_status_t

{
	/* Gecko status
	 * @values
	 * 0 -> Not ready
	 * 1 -> Ready
	 */
	uint32_t status;

};

static int spf_core_callback(struct gpr_device *adev, void *data)
{
	struct spf_core *core = dev_get_drvdata(&adev->dev);
	struct apm_cmd_rsp_get_spf_status_t *spf_status_rsp;
	struct spf_cmd_basic_rsp *basic_rsp;
	struct gpr_hdr *hdr = data;


	dev_info(&adev->dev ,"%s: Payload %x",__func__, hdr->opcode);
	switch (hdr->opcode) {
	case GPR_IBASIC_RSP_RESULT:
		basic_rsp = GPR_PKT_GET_PAYLOAD(
				struct spf_cmd_basic_rsp,
				data);
		dev_info(&adev->dev ,"%s: op %x status %d", __func__,
				basic_rsp->opcode, basic_rsp->status);
		if (basic_rsp->opcode == APM_CMD_CLOSE_ALL) {
			core->status = basic_rsp->status;
		} else {
			dev_err(&adev->dev ,"%s: Failed response received",
					__func__);
		}
		core->resp_received = true;
		break;
	case APM_CMD_RSP_GET_SPF_STATE:
		spf_status_rsp =
				GPR_PKT_GET_PAYLOAD(
					struct apm_cmd_rsp_get_spf_status_t,
					data);
		dev_info(&adev->dev ,"%s: sucess response received",__func__);
		core->status = spf_status_rsp->status;
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

static bool __spf_core_is_apm_ready(struct spf_core *core)
{
	struct gpr_device *adev = core->adev;
	struct gpr_pkt pkt;
	int rc;
	bool ret = false;

	pkt.hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
			 GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
			 GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, GPR_PKT_HEADER_BYTE_SIZE_V);

	pkt.hdr.dst_port = APM_MODULE_INSTANCE_ID;
	pkt.hdr.src_port = GPR_SVC_ADSP_CORE;
	pkt.hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
	pkt.hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
	pkt.hdr.opcode = APM_CMD_GET_SPF_STATE;

	dev_err(spf_core_priv->dev, "%s: send_command ret\n",	__func__);

	rc = gpr_send_pkt(adev, &pkt);
	if (rc < 0) {
		ret = false;
		goto done;
	}

	rc = wait_event_timeout(core->wait, (core->resp_received),
				msecs_to_jiffies(Q6_READY_TIMEOUT_MS));
	dev_dbg(spf_core_priv->dev, "%s: wait event unblocked \n", __func__);

	if (rc > 0 && core->resp_received) {
		ret = core->status;
	} else {
		dev_err(spf_core_priv->dev, "%s: command timedout, ret\n",
			__func__);
        }
done:
	core->resp_received = false;
	return ret;
}

/**
 * spf_core_is_apm_ready() - Get status of adsp
 *
 * Return: Will return true if apm is ready and false if not.
 */
bool spf_core_is_apm_ready(void)
{
	unsigned long  timeout;
	bool ret = false;
	struct spf_core *core;

	if (!spf_core_priv)
		return ret;

	mutex_lock(&spf_core_priv->lock);
	core = spf_core_priv->spf_core_drv;
	if (!core)
		goto done;

	timeout = jiffies + msecs_to_jiffies(APM_STATE_READY_TIMEOUT_MS);
	mutex_lock(&core->lock);
	for (;;) {
		if (__spf_core_is_apm_ready(core)) {
			ret = true;
			break;
		}
		usleep_range(50000, 50050);
		if (!time_after(timeout, jiffies)) {
			ret = false;
			break;
		}
	}

	mutex_unlock(&core->lock);
done:
	mutex_unlock(&spf_core_priv->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(spf_core_is_apm_ready);

/**
 * spf_core_apm_close_all() - Get status of adsp
 *
 * Return: Will be return true if apm is ready and false if not.
 */
void spf_core_apm_close_all(void)
{
	int rc = 0;
	struct spf_core *core;
	struct gpr_pkt pkt;
	struct gpr_device *adev = NULL;

	if (!spf_core_priv)
		return;

	mutex_lock(&spf_core_priv->lock);
	core = spf_core_priv->spf_core_drv;
	if (!core) {
		mutex_unlock(&spf_core_priv->lock);
		return;
	}

	mutex_lock(&core->lock);

	adev = core->adev;

	pkt.hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
			 GPR_SET_FIELD(GPR_PKT_HEADER_SIZE,
					GPR_PKT_HEADER_WORD_SIZE_V) |
			 GPR_SET_FIELD(GPR_PKT_PACKET_SIZE,
					GPR_PKT_HEADER_BYTE_SIZE_V);

	pkt.hdr.dst_port = APM_MODULE_INSTANCE_ID;
	pkt.hdr.src_port = GPR_SVC_ADSP_CORE;
	pkt.hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
	pkt.hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
	pkt.hdr.opcode = APM_CMD_CLOSE_ALL;

	dev_info(spf_core_priv->dev, "%s: send_command \n", __func__);

	rc = gpr_send_pkt(adev, &pkt);
	if (rc < 0) {
		dev_err(spf_core_priv->dev, "%s: send_pkt_failed %d\n",
				__func__, rc);
		goto done;
	}


	/* While graph_open is processing by the SPF, apps receives
	 * userspace(agm/pal) crash which will triggers spf_close_all
	 * cmd from msm common drivers and immediately calls
	 * msm_audio_ion_crash_handler() which will un-maps the memory. But
	 * here SPF is still in processing the graph_open, recieved spf_close_all
	 * cmd is queued in SPF. Due to un-mapping is done immediately in HLOS
	 * will resulting in SMMU fault.
	 * To avoid such scenarios, increased the spf_close_all cmd timeout,
	 * because the AGM timeout for the graph_open is 4sec, so increase the timeout
	 * for spf_close_all cmd response until graph open completes or timed out.
	*/
	rc = wait_event_timeout(core->wait, (core->resp_received),
				msecs_to_jiffies(Q6_CLOSE_ALL_TIMEOUT_MS));
	dev_info(spf_core_priv->dev, "%s: wait event unblocked \n", __func__);
	if (rc > 0 && core->resp_received) {
		if (core->status != 0)
			dev_err(spf_core_priv->dev, "%s, cmd failed status %d",
					__func__, core->status);
	} else {
		dev_err(spf_core_priv->dev, "%s: command timedout, ret\n",
			__func__);
        }

done:
	core->resp_received = false;
	mutex_unlock(&core->lock);
	mutex_unlock(&spf_core_priv->lock);
	return;
}
EXPORT_SYMBOL_GPL(spf_core_apm_close_all);


static int spf_core_probe(struct gpr_device *adev)
{
	struct spf_core *core;
	pr_err("%s",__func__);
	if (!spf_core_priv) {
		pr_err("%s: spf_core platform probe not yet done\n", __func__);
		return -EPROBE_DEFER;
	}
	mutex_lock(&spf_core_priv->lock);
	core = kzalloc(sizeof(*core), GFP_KERNEL);
	if (!core) {
		mutex_unlock(&spf_core_priv->lock);
		return -ENOMEM;
	}

	dev_set_drvdata(&adev->dev, core);

	mutex_init(&core->lock);
	core->adev = adev;
	init_waitqueue_head(&core->wait);
	spf_core_priv->spf_core_drv = core;
	if (spf_core_priv->is_initial_boot)
		schedule_work(&spf_core_priv->add_chld_dev_work);
	mutex_unlock(&spf_core_priv->lock);

	return 0;
}

static int spf_core_exit(struct gpr_device *adev)
{
	struct spf_core *core = dev_get_drvdata(&adev->dev);
	if (!spf_core_priv) {
		pr_err("%s: spf_core platform probe not yet done\n", __func__);
		return -1;
	}
	mutex_lock(&spf_core_priv->lock);
	spf_core_priv->spf_core_drv = NULL;
	kfree(core);
        mutex_unlock(&spf_core_priv->lock);
	return 0;
}

static const struct of_device_id spf_core_device_id[]  = {
	{ .compatible = "qcom,spf_core" },
	{},
};
MODULE_DEVICE_TABLE(of, spf_core_device_id);

static struct gpr_driver qcom_spf_core_driver = {
	.probe = spf_core_probe,
	.remove = spf_core_exit,
	.callback = spf_core_callback,
	.driver = {
		.name = "qcom-spf_core",
		.of_match_table = of_match_ptr(spf_core_device_id),
	},
};

static void spf_core_add_child_devices(struct work_struct *work)
{
	int ret;
        pr_err("%s:enumarate machine driver\n", __func__);

	if(spf_core_is_apm_ready()) {
		dev_err(spf_core_priv->dev, "%s: apm is up\n",
			__func__);
	} else {
		dev_err(spf_core_priv->dev, "%s: apm is not up\n",
			__func__);
		return;
	}

	ret = of_platform_populate(spf_core_priv->dev->of_node,
			NULL, NULL, spf_core_priv->dev);
	if (ret)
		dev_err(spf_core_priv->dev, "%s: failed to add child nodes, ret=%d\n",
			__func__, ret);

        spf_core_priv->is_initial_boot = false;

}

static int spf_core_platform_driver_probe(struct platform_device *pdev)
{
	int ret = 0;
        pr_err("%s",__func__);

	spf_core_priv = devm_kzalloc(&pdev->dev, sizeof(struct spf_core_private), GFP_KERNEL);
	if (!spf_core_priv)
		return -ENOMEM;

	spf_core_priv->dev = &pdev->dev;

	mutex_init(&spf_core_priv->lock);

	INIT_WORK(&spf_core_priv->add_chld_dev_work, spf_core_add_child_devices);

	spf_core_priv->is_initial_boot = true;
	ret = gpr_driver_register(&qcom_spf_core_driver);
	if (ret) {
		pr_err("%s: gpr driver register failed = %d\n",
			__func__, ret);
		ret = 0;
	}

#if 0
	ret = snd_event_client_register(&pdev->dev, &gpr_ssr_ops, NULL);
	if (ret) {
		pr_err("%s: Registration with SND event fwk failed ret = %d\n",
			__func__, ret);
		ret = 0;
	}
#endif
    digital_cdc_rsc_mgr_init();

	return ret;
}

static int spf_core_platform_driver_remove(struct platform_device *pdev)
{
	//snd_event_client_deregister(&pdev->dev);
        gpr_driver_unregister(&qcom_spf_core_driver);
	spf_core_priv = NULL;
    digital_cdc_rsc_mgr_exit();
	return 0;
}

static const struct of_device_id spf_core_of_match[]  = {
	{ .compatible = "qcom,spf-core-platform", },
	{},
};

static struct platform_driver spf_core_driver = {
	.probe = spf_core_platform_driver_probe,
	.remove = spf_core_platform_driver_remove,
	.driver = {
		.name = "spf-core-platform",
		.owner = THIS_MODULE,
		.of_match_table = spf_core_of_match,
	}
};

module_platform_driver(spf_core_driver);

MODULE_DESCRIPTION("q6 core");
MODULE_LICENSE("GPL v2");
