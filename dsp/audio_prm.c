// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <ipc/gpr-lite.h>
#include <soc/snd_event.h>
#include <dsp/audio_prm.h>
#include <dsp/spf-core.h>
#include <dsp/audio_notifier.h>

#define TIMEOUT_MS 500
#define MAX_RETRY_COUNT 3
#define APM_READY_WAIT_DURATION 2

struct audio_prm {
	struct gpr_device *adev;
	wait_queue_head_t wait;
	struct mutex lock;
	bool resp_received;
	atomic_t state;
	atomic_t status;
	bool is_adsp_up;
};

static struct audio_prm g_prm;

static bool is_apm_ready_check_done = false;

static int audio_prm_callback(struct gpr_device *adev, void *data)
{
	struct gpr_hdr *hdr = (struct gpr_hdr *)data;
	uint32_t *payload =  GPR_PKT_GET_PAYLOAD(uint32_t, data);

	//dev_err(&adev->dev, "%s: Payload %x", __func__, hdr->opcode);
	switch (hdr->opcode) {
	case GPR_IBASIC_RSP_RESULT:
		pr_err("%s: Failed response received",__func__);
		atomic_set(&g_prm.status, payload[1]);
		g_prm.resp_received = true;
		break;
	case PRM_CMD_RSP_REQUEST_HW_RSC:
	case PRM_CMD_RSP_RELEASE_HW_RSC:
		/* payload[1] contains the error status for response */
		if (payload[1] != 0) {
			atomic_set(&g_prm.status, payload[1]);
			pr_err("%s: cmd = 0x%x returned error = 0x%x\n",
				__func__, payload[0], payload[1]);
		}
		g_prm.resp_received = true;
		/* payload[0] contains the param_ID for response */
		switch (payload[0]) {
		case PARAM_ID_RSC_AUDIO_HW_CLK:
		case PARAM_ID_RSC_LPASS_CORE:
		case PARAM_ID_RSC_HW_CORE:
			if (payload[1] != 0)
				pr_err("%s: PRM command failed with error %d\n",
					__func__, payload[1]);
			atomic_set(&g_prm.state, payload[1]);
			break;
		default:
			pr_err("%s: hit default case",__func__);
			break;
		};
	default:
		break;
	};
	if (g_prm.resp_received)
		wake_up(&g_prm.wait);
	return 0;
}

static int prm_gpr_send_pkt(struct gpr_pkt *pkt, wait_queue_head_t *wait)
{
	int ret = 0;
	int retry;

	if (wait)
		atomic_set(&g_prm.state, 1);
	atomic_set(&g_prm.status, 0);

	mutex_lock(&g_prm.lock);
	pr_debug("%s: enter",__func__);

	if (g_prm.adev == NULL) {
		pr_err("%s: apr is unregistered\n", __func__);
		mutex_unlock(&g_prm.lock);
		return -ENODEV;
	}
	if (!is_apm_ready_check_done && g_prm.is_adsp_up &&
			(gpr_get_q6_state() == GPR_SUBSYS_LOADED)) {
		pr_info("%s: apm ready check not done\n", __func__);
		retry = 0;
		while (!spf_core_is_apm_ready() || retry < MAX_RETRY_COUNT) {
			msleep(APM_READY_WAIT_DURATION);
			retry++;
		}
		is_apm_ready_check_done = true;
		pr_info("%s: apm ready check done\n", __func__);
	}
	g_prm.resp_received = false;
	ret = gpr_send_pkt(g_prm.adev, pkt);
	if (ret < 0) {
		pr_err("%s: packet not transmitted %d\n", __func__, ret);
		mutex_unlock(&g_prm.lock);
		return ret;
	}

	if (wait) {
		ret = wait_event_timeout(g_prm.wait,
				(g_prm.resp_received),
				msecs_to_jiffies(2 * TIMEOUT_MS));
		if (!ret) {
			pr_err("%s: pkt send timeout\n", __func__);
			ret = -ETIMEDOUT;
		} else if (atomic_read(&g_prm.status) > 0) {
			pr_err("%s: DSP returned error %d\n", __func__,
				atomic_read(&g_prm.status));
			ret = -EINVAL;
		} else {
			ret = 0;
		}
	}
	pr_debug("%s: exit",__func__);
	mutex_unlock(&g_prm.lock);
	return ret;
}

/**
 */
int audio_prm_set_lpass_hw_core_req(struct clk_cfg *cfg, uint32_t hw_core_id, uint8_t enable)
{
	struct gpr_pkt *pkt;
        prm_cmd_request_hw_core_t prm_rsc_request;
        int ret = 0;
        uint32_t size;

        size = GPR_HDR_SIZE + sizeof(prm_cmd_request_hw_core_t);
        pkt = kzalloc(size,  GFP_KERNEL);
        if (!pkt)
                return -ENOMEM;

        pkt->hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
                         GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
                         GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, size);

        pkt->hdr.src_port = GPR_SVC_ASM;
        pkt->hdr.dst_port = PRM_MODULE_INSTANCE_ID;
        pkt->hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
        pkt->hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
        pkt->hdr.token = 0; /* TBD */
	if (enable)
		pkt->hdr.opcode = PRM_CMD_REQUEST_HW_RSC;
	else
		pkt->hdr.opcode = PRM_CMD_RELEASE_HW_RSC;

        //pr_err("%s: clk_id %d size of cmd_req %ld \n",__func__, cfg->clk_id, sizeof(prm_cmd_request_hw_core_t));

        prm_rsc_request.payload_header.payload_address_lsw = 0;
        prm_rsc_request.payload_header.payload_address_msw = 0;
        prm_rsc_request.payload_header.mem_map_handle = 0;
        prm_rsc_request.payload_header.payload_size = sizeof(prm_cmd_request_hw_core_t) - sizeof(apm_cmd_header_t);

        /** Populate the param payload */
        prm_rsc_request.module_payload_0.module_instance_id = PRM_MODULE_INSTANCE_ID;
        prm_rsc_request.module_payload_0.error_code = 0;
        prm_rsc_request.module_payload_0.param_id = PARAM_ID_RSC_HW_CORE;
        prm_rsc_request.module_payload_0.param_size =
                sizeof(prm_cmd_request_hw_core_t) - sizeof(apm_cmd_header_t) - sizeof(apm_module_param_data_t);


        prm_rsc_request.hw_core_id = hw_core_id; // HW_CORE_ID_LPASS;

        memcpy(&pkt->payload, &prm_rsc_request, sizeof(prm_cmd_request_hw_core_t));

        ret = prm_gpr_send_pkt(pkt, &g_prm.wait);

        kfree(pkt);
        return ret;
}
EXPORT_SYMBOL(audio_prm_set_lpass_hw_core_req);

/**
 * prm_set_lpass_clk_cfg() - Set PRM clock
 *
 * Return: 0 if clock set is success
 */
static int audio_prm_set_lpass_clk_cfg_req(struct clk_cfg *cfg)
{
	struct gpr_pkt *pkt;
	prm_cmd_request_rsc_t prm_rsc_request;
	int ret = 0;
	uint32_t size;

	size = GPR_HDR_SIZE + sizeof(prm_cmd_request_rsc_t);
	pkt = kzalloc(size,  GFP_KERNEL);
	if (!pkt)
		return -ENOMEM;

	pkt->hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
			 GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
			 GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, size);

	pkt->hdr.src_port = GPR_SVC_ASM;
	pkt->hdr.dst_port = PRM_MODULE_INSTANCE_ID;
	pkt->hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
	pkt->hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
	pkt->hdr.token = 0; /* TBD */
	pkt->hdr.opcode = PRM_CMD_REQUEST_HW_RSC;

	//pr_err("%s: clk_id %d size of cmd_req %ld \n",__func__, cfg->clk_id, sizeof(prm_cmd_request_rsc_t));

	prm_rsc_request.payload_header.payload_address_lsw = 0;
	prm_rsc_request.payload_header.payload_address_msw = 0;
	prm_rsc_request.payload_header.mem_map_handle = 0;
	prm_rsc_request.payload_header.payload_size = sizeof(prm_cmd_request_rsc_t) - sizeof(apm_cmd_header_t);

	/** Populate the param payload */
	prm_rsc_request.module_payload_0.module_instance_id = PRM_MODULE_INSTANCE_ID;
	prm_rsc_request.module_payload_0.error_code = 0;
	prm_rsc_request.module_payload_0.param_id = PARAM_ID_RSC_AUDIO_HW_CLK;
	prm_rsc_request.module_payload_0.param_size =
		sizeof(prm_cmd_request_rsc_t) - sizeof(apm_cmd_header_t) - sizeof(apm_module_param_data_t);

	prm_rsc_request.num_clk_id_t.num_clock_id = MAX_AUD_HW_CLK_NUM_REQ;

	prm_rsc_request.clock_ids_t[0].clock_id = cfg->clk_id;
	prm_rsc_request.clock_ids_t[0].clock_freq = cfg->clk_freq_in_hz;
	prm_rsc_request.clock_ids_t[0].clock_attri = cfg->clk_attri;
	prm_rsc_request.clock_ids_t[0].clock_root = cfg->clk_root;

	memcpy(&pkt->payload, &prm_rsc_request, sizeof(prm_cmd_request_rsc_t));

	ret = prm_gpr_send_pkt(pkt, &g_prm.wait);

	kfree(pkt);
	return ret;
}

static int audio_prm_set_lpass_clk_cfg_rel(struct clk_cfg *cfg)
{

        struct gpr_pkt *pkt;
        prm_cmd_release_rsc_t prm_rsc_release;
        int ret = 0;
        uint32_t size;

        size = GPR_HDR_SIZE + sizeof(prm_cmd_release_rsc_t);
        pkt = kzalloc(size,  GFP_KERNEL);
        if (!pkt)
                return -ENOMEM;

        pkt->hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
                         GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
                         GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, size);

        pkt->hdr.src_port = GPR_SVC_ASM;
        pkt->hdr.dst_port = PRM_MODULE_INSTANCE_ID;
        pkt->hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
        pkt->hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
        pkt->hdr.token = 0; /* TBD */
        pkt->hdr.opcode = PRM_CMD_RELEASE_HW_RSC;

        //pr_err("%s: clk_id %d size of cmd_req %ld \n",__func__, cfg->clk_id, sizeof(prm_cmd_release_rsc_t));

        prm_rsc_release.payload_header.payload_address_lsw = 0;
        prm_rsc_release.payload_header.payload_address_msw = 0;
        prm_rsc_release.payload_header.mem_map_handle = 0;
        prm_rsc_release.payload_header.payload_size = sizeof(prm_cmd_release_rsc_t) - sizeof(apm_cmd_header_t);

        /** Populate the param payload */
        prm_rsc_release.module_payload_0.module_instance_id = PRM_MODULE_INSTANCE_ID;
        prm_rsc_release.module_payload_0.error_code = 0;
        prm_rsc_release.module_payload_0.param_id = PARAM_ID_RSC_AUDIO_HW_CLK;
        prm_rsc_release.module_payload_0.param_size =
                sizeof(prm_cmd_release_rsc_t) - sizeof(apm_cmd_header_t) - sizeof(apm_module_param_data_t);

        prm_rsc_release.num_clk_id_t.num_clock_id = MAX_AUD_HW_CLK_NUM_REQ;

        prm_rsc_release.clock_ids_t[0].clock_id = cfg->clk_id;

        memcpy(&pkt->payload, &prm_rsc_release, sizeof(prm_cmd_release_rsc_t));

        ret = prm_gpr_send_pkt(pkt, &g_prm.wait);

        kfree(pkt);
        return ret;
}

/**
 * audio_prm_set_cdc_earpa_duty_cycling_req() - send codec reg values
 * for codec duty cycling.
 *
 * Return: 0 if reg passing is success.
 */
int audio_prm_set_cdc_earpa_duty_cycling_req(struct prm_earpa_hw_intf_config *earpa_config,
								uint32_t enable)
{
	struct gpr_pkt *pkt;
	prm_cmd_request_cdc_duty_cycling_t prm_rsc_request_reg_info;
	int ret = 0;
	uint32_t size;

	size = GPR_HDR_SIZE + sizeof(prm_cmd_request_cdc_duty_cycling_t);
	pkt = kzalloc(size,  GFP_KERNEL);
	if (!pkt)
		return -ENOMEM;
	pkt->hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
			GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
			GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, size);

	pkt->hdr.src_port = GPR_SVC_ASM;
	pkt->hdr.dst_port = PRM_MODULE_INSTANCE_ID;
	pkt->hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
	pkt->hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;
	pkt->hdr.token = 0;
	if (enable)
		pkt->hdr.opcode = PRM_CMD_REQUEST_HW_RSC;
	else
		pkt->hdr.opcode = PRM_CMD_RELEASE_HW_RSC;

	memset(&prm_rsc_request_reg_info, 0, sizeof(prm_cmd_request_cdc_duty_cycling_t));
	prm_rsc_request_reg_info.payload_header.payload_address_lsw = 0;
	prm_rsc_request_reg_info.payload_header.payload_address_msw = 0;
	prm_rsc_request_reg_info.payload_header.mem_map_handle = 0;
	prm_rsc_request_reg_info.payload_header.payload_size =
			sizeof(prm_cmd_request_cdc_duty_cycling_t) - sizeof(apm_cmd_header_t);

	/* Populate the param payload */
	prm_rsc_request_reg_info.module_payload_0.module_instance_id =
							PRM_MODULE_INSTANCE_ID;
	prm_rsc_request_reg_info.module_payload_0.error_code = 0;
	prm_rsc_request_reg_info.module_payload_0.param_id =
						PARAM_ID_RSC_HW_CODEC_REG_INFO;
	prm_rsc_request_reg_info.module_payload_0.param_size =
			sizeof(prm_cmd_request_cdc_duty_cycling_t) -
			sizeof(apm_cmd_header_t) - sizeof(apm_module_param_data_t);
	prm_rsc_request_reg_info.hw_codec_reg_info_t.num_reg_info_t = MAX_EARPA_REG;
	/* Setting up DIGITAL Mute register value */
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_reg_id =
							HW_CODEC_DIG_REG_ID_MUTE_CTRL;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_reg_addr_msw = 0;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_reg_addr_lsw =
			earpa_config->ear_pa_hw_reg_cfg.lpass_cdc_rx0_rx_path_ctl_phy_addr;

	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].num_ops =
							MAX_EARPA_CDC_DUTY_CYC_OPERATION;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_op[0].hw_codec_op_id =
								HW_CODEC_OP_DIG_MUTE_ENABLE;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_op[0].hw_codec_op_value =
								DIG_MUTE_ENABLE;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_op[1].hw_codec_op_id =
								HW_CODEC_OP_DIG_MUTE_DISABLE;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[0].hw_codec_op[1].hw_codec_op_value =
								DIG_MUTE_DISABLE;
	/* Setting up LPASS_PA_REG Values */
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_reg_id =
						HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_reg_addr_msw = 0;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_reg_addr_lsw =
					earpa_config->ear_pa_hw_reg_cfg.lpass_wr_fifo_reg_phy_addr;

	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].num_ops =
							MAX_EARPA_CDC_DUTY_CYC_OPERATION;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_op[0].hw_codec_op_id =
							HW_CODEC_OP_ANA_PGA_ENABLE;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_op[0].hw_codec_op_value =
					earpa_config->ear_pa_pkd_cfg.ear_pa_enable_pkd_reg_addr;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_op[1].hw_codec_op_id =
							HW_CODEC_OP_ANA_PGA_DISABLE;
	prm_rsc_request_reg_info.hw_codec_reg_info_t.hw_codec_reg[1].hw_codec_op[1].hw_codec_op_value =
					earpa_config->ear_pa_pkd_cfg.ear_pa_disable_pkd_reg_addr;

	memcpy(&pkt->payload, &prm_rsc_request_reg_info, sizeof(prm_cmd_request_cdc_duty_cycling_t));
	ret = prm_gpr_send_pkt(pkt, &g_prm.wait);
	kfree(pkt);
	return ret;
}
EXPORT_SYMBOL(audio_prm_set_cdc_earpa_duty_cycling_req);

int audio_prm_set_lpass_clk_cfg (struct clk_cfg *clk, uint8_t enable)
{
	int ret = 0;
	if (enable)
		ret = audio_prm_set_lpass_clk_cfg_req (clk);
	else
		ret = audio_prm_set_lpass_clk_cfg_rel (clk);
	return ret;
}
EXPORT_SYMBOL(audio_prm_set_lpass_clk_cfg);

static int audio_prm_service_cb(struct notifier_block *this,
				unsigned long opcode, void *data)
{
	pr_info("%s: Service opcode 0x%lx\n", __func__, opcode);

	switch (opcode) {
	case AUDIO_NOTIFIER_SERVICE_DOWN:
		mutex_lock(&g_prm.lock);
		pr_debug("%s: making apm_ready check false\n", __func__);
		is_apm_ready_check_done = false;
		g_prm.is_adsp_up = false;
		mutex_unlock(&g_prm.lock);
		break;
	case AUDIO_NOTIFIER_SERVICE_UP:
		mutex_lock(&g_prm.lock);
		g_prm.is_adsp_up = true;
		mutex_unlock(&g_prm.lock);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block service_nb = {
	.notifier_call  = audio_prm_service_cb,
	.priority = -INT_MAX,
};

static int audio_prm_probe(struct gpr_device *adev)
{
	int ret = 0;

	ret = audio_notifier_register("audio_prm", AUDIO_NOTIFIER_ADSP_DOMAIN,
				      &service_nb);
	if (ret < 0) {
		if (ret != -EPROBE_DEFER)
			pr_err("%s: Audio notifier register failed ret = %d\n",
			       __func__, ret);
		return ret;
	}

	dev_set_drvdata(&adev->dev, &g_prm);

	g_prm.adev = adev;

	init_waitqueue_head(&g_prm.wait);
	g_prm.is_adsp_up = true;
	pr_err("%s: prm probe success\n", __func__);
	return ret;
}

static int audio_prm_remove(struct gpr_device *adev)
{
	int ret = 0;

	audio_notifier_deregister("audio_prm");
	mutex_lock(&g_prm.lock);
	g_prm.is_adsp_up = false;
	g_prm.adev = NULL;
	mutex_unlock(&g_prm.lock);
	return ret;
}

static const struct of_device_id audio_prm_device_id[]  = {
	{ .compatible = "qcom,audio_prm" },
	{},
};
MODULE_DEVICE_TABLE(of, audio_prm_device_id);

static struct gpr_driver qcom_audio_prm_driver = {
	.probe = audio_prm_probe,
	.remove = audio_prm_remove,
	.callback = audio_prm_callback,
	.driver = {
		.name = "qcom-audio_prm",
		.of_match_table = of_match_ptr(audio_prm_device_id),
	},
};

static int __init audio_prm_module_init(void)
{
	int ret;
	ret = gpr_driver_register(&qcom_audio_prm_driver);

	if (ret)
		pr_err("%s: gpr driver register failed = %d\n", __func__, ret);

	mutex_init(&g_prm.lock);

	return ret;
}

static void __exit audio_prm_module_exit(void)
{
	mutex_destroy(&g_prm.lock);
	gpr_driver_unregister(&qcom_audio_prm_driver);
}

module_init(audio_prm_module_init);
module_exit(audio_prm_module_exit);
MODULE_DESCRIPTION("audio prm");
MODULE_LICENSE("GPL v2");
