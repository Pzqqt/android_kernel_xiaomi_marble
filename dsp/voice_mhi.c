// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mhi.h>
#include <linux/mutex.h>
#include "voice_mhi.h"
#include <dsp/msm_audio_ion.h>
#include <dsp/gecko-core.h>
#include <dsp/audio_prm.h>
#include <ipc/gpr-lite.h>

#define VCPM_PARAM_ID_MAILBOX_MEMORY_CONFIG 0x080011E7

#define VOICE_MHI_STATE_SET(a, b) ((a) |= (1UL<<(b)))
#define VOICE_MHI_STATE_RESET(a, b) ((a) &= ~(1UL<<(b)))
#define VOICE_MHI_STATE_CHECK(a, b) (1UL & (a >> b))

#define CMD_STATUS_SUCCESS 0
#define CMD_STATUS_FAIL 1
#define TIMEOUT_MS 500
#define PORT_NUM 0x01
#define PORT_MASK 0x03
#define CONVERT_PORT_APR(x, y) (x << 8 | y)

#define VCPM_MODULE_INSTANCE_ID   0x00000004
#define APM_CMD_SET_CFG           0x01001006

enum voice_states {
	VOICE_MHI_INIT = 0,
	VOICE_MHI_PROBED = VOICE_MHI_INIT,
	VOICE_MHI_ADSP_UP,
	VOICE_MHI_SDX_UP,
	VOICE_MHI_INCALL
};

struct voice_mhi_addr {
	dma_addr_t base;
	uint32_t size;
};

struct voice_mhi_dev_info {
	struct platform_device *pdev;
	struct voice_mhi_addr phys_addr;
	struct voice_mhi_addr iova_pcie;
	struct voice_mhi_addr iova_adsp;
};

struct voice_mhi {
	struct voice_mhi_dev_info dev_info;
	struct mhi_device *mhi_dev;
	uint32_t vote_count;
	struct mutex mutex;
	enum voice_states voice_mhi_state;
	bool vote_enable;
	bool pcie_enabled;
	struct gpr_device *gdev;
	struct work_struct voice_mhi_work_pcie;
	struct work_struct voice_mhi_work_adsp;
	wait_queue_head_t voice_mhi_wait;
	u32 mvm_state;
	u32 async_err;
};
typedef struct vcpm_param_id_mailbox_memory_config_t
{
  uint32_t mailbox_mem_address_adsp_lsw;
  /**< Lower 32 bits of IO virtual address(understandable to ADSP) of Mailbox Memory carved out from APQ DDR 
       specifically for mailbox packet exchange between ADSP and MDM. */
 
  uint32_t mailbox_mem_address_adsp_msw;
  /**< Upper 32 bits of IO virtual address(understandable to ADSP) of Mailbox Memory carved out from APQ DDR 
       specifically for mailbox packet exchange between ADSP and MDM. */
 
  uint32_t mailbox_mem_address_pcie_lsw;
  /**< Lower 32 bits of IO virtual address(understandable to PCIe) of Mailbox Memory carved out from APQ DDR 
       specifically for mailbox packet exchange between ADSP and MDM. */
 
  uint32_t mailbox_mem_address_pcie_msw;
  /**< Upper 32 bits of IO virtual address(understandable to PCIe) of Mailbox Memory carved out from APQ DDR 
       specifically for mailbox packet exchange between ADSP and MDM. */
 
  uint32_t mem_size;
  /**< Size(in bytes) of the Mailbox Memory carved out from APQ DDR.
   */
}vcpm_param_id_mailbox_memory_config_t;

typedef struct vcpm_cmd_set_cfg_t {
	apm_cmd_header_t payload_header;
	apm_module_param_data_t module_payload;
	vcpm_param_id_mailbox_memory_config_t mb_memory_config;
}vcpm_cmd_set_cfg_t;




static struct voice_mhi voice_mhi_lcl;

static int voice_mhi_pcie_up_callback(struct mhi_device *,
					const struct mhi_device_id *);
static void voice_mhi_pcie_down_callback(struct mhi_device *);
static void voice_mhi_pcie_status_callback(struct mhi_device *, enum MHI_CB);
static int32_t voice_mhi_gpr_callback(struct gpr_device *adev, void *data);
//static int voice_mhi_notifier_service_cb(struct notifier_block *nb,
//					 unsigned long opcode, void *ptr);

// static struct notifier_block voice_mhi_service_nb = {
	// .notifier_call  = voice_mhi_notifier_service_cb,
	// .priority = -INT_MAX,
// };

static const struct mhi_device_id voice_mhi_match_table[] = {
	{ .chan = "AUDIO_VOICE_0", .driver_data = 0 },
	{},
};

static struct mhi_driver voice_mhi_driver = {
	.id_table = voice_mhi_match_table,
	.probe = voice_mhi_pcie_up_callback,
	.remove = voice_mhi_pcie_down_callback,
	.status_cb = voice_mhi_pcie_status_callback,
	.driver = {
		.name = "voice_mhi_audio",
		.owner = THIS_MODULE,
	},
};

// static int voice_mhi_notifier_service_cb(struct notifier_block *nb,
					 // unsigned long opcode, void *ptr)
// {
	// pr_err("%s: opcode 0x%lx\n", __func__, opcode);

	// switch (opcode) {
	// case AUDIO_NOTIFIER_SERVICE_DOWN:
		// if (voice_mhi_lcl.apr_mvm_handle) {
			// apr_reset(voice_mhi_lcl.apr_mvm_handle);
			// voice_mhi_lcl.apr_mvm_handle = NULL;
			// VOICE_MHI_STATE_RESET(voice_mhi_lcl.voice_mhi_state,
					// VOICE_MHI_ADSP_UP);
		// }
		// break;
	// case AUDIO_NOTIFIER_SERVICE_UP:
		// if (!VOICE_MHI_STATE_CHECK(voice_mhi_lcl.voice_mhi_state,
				// VOICE_MHI_ADSP_UP)) {
			// VOICE_MHI_STATE_SET(voice_mhi_lcl.voice_mhi_state,
					// VOICE_MHI_ADSP_UP);
			// schedule_work(&voice_mhi_lcl.voice_mhi_work_adsp);
		// }
		// break;
	// default:
		// break;
	// }

	// return NOTIFY_OK;

// }

static int32_t voice_mhi_gpr_callback(struct gpr_device *adev, void *data)
{
	struct gpr_hdr *hdr = (struct gpr_hdr *)data;
	uint32_t *payload =  GPR_PKT_GET_PAYLOAD(uint32_t, data);
	//bool resp_recieved = false;

	if (data == NULL) {
		pr_err("%s: data is NULL\n", __func__);
		return -EINVAL;
	}

	switch (hdr->opcode) {
	case GPR_IBASIC_RSP_RESULT:
		pr_err("%s: GPR_IBASIC_RSP_RESULT received",__func__);
		voice_mhi_lcl.mvm_state = CMD_STATUS_SUCCESS;
		voice_mhi_lcl.async_err = payload[1];
		wake_up(&voice_mhi_lcl.voice_mhi_wait);
		break;
	case VCPM_PARAM_ID_MAILBOX_MEMORY_CONFIG:
		pr_err("%s: cmd VCPM_PARAM_ID_MAILBOX_MEMORY_CONFIG\n",
					 __func__);
			voice_mhi_lcl.mvm_state = CMD_STATUS_SUCCESS;
			voice_mhi_lcl.async_err = payload[1];
			wake_up(&voice_mhi_lcl.voice_mhi_wait);
			break;
	default:
		pr_err("%s: Invalid opcode %d\n", __func__,
				hdr->opcode);
		break;

	}
	return 0;
}

/**
 * voice_mhi_start -
 *        Start vote for MHI/PCIe clock
 *
 * Returns 0 on success or error on failure
 */
int voice_mhi_start(void)
{
	int ret = 0;

	mutex_lock(&voice_mhi_lcl.mutex);
	if (voice_mhi_lcl.pcie_enabled) {
		if (!voice_mhi_lcl.mhi_dev) {
			pr_err("%s: NULL device found\n", __func__);
			ret = -EINVAL;
			goto done;
		}
		if (voice_mhi_lcl.vote_count == 0) {
			ret = mhi_device_get_sync(voice_mhi_lcl.mhi_dev,
					MHI_VOTE_DEVICE);
			if (ret) {
				pr_err("%s: mhi_device_get_sync failed\n",
					   __func__);
				ret = -EINVAL;
				goto done;
			}
			pr_err("%s: mhi_device_get_sync success\n", __func__);
		} else {
			/* For DSDA, no additional voting is needed */
			pr_err("%s: mhi is already voted\n", __func__);
		}
		voice_mhi_lcl.vote_count++;
	} else {
		/* PCIe not supported - return success*/
		goto done;
	}
done:
	mutex_unlock(&voice_mhi_lcl.mutex);

	return ret;
}
EXPORT_SYMBOL(voice_mhi_start);

/**
 * voice_mhi_end -
 *        End vote for MHI/PCIe clock
 *
 * Returns 0 on success or error on failure
 */
int voice_mhi_end(void)
{
	mutex_lock(&voice_mhi_lcl.mutex);
	if (voice_mhi_lcl.pcie_enabled) {
		if (!voice_mhi_lcl.mhi_dev || voice_mhi_lcl.vote_count == 0) {
			pr_err("%s: NULL device found\n", __func__);
			mutex_unlock(&voice_mhi_lcl.mutex);
			return -EINVAL;
		}

		if (voice_mhi_lcl.vote_count == 1)
			mhi_device_put(voice_mhi_lcl.mhi_dev, MHI_VOTE_DEVICE);
		voice_mhi_lcl.vote_count--;
	}
	mutex_unlock(&voice_mhi_lcl.mutex);

	return 0;
}
EXPORT_SYMBOL(voice_mhi_end);

static int voice_mhi_set_mailbox_memory_config(void)
{
	struct gpr_pkt *pkt;
	int ret = 0;
	struct gpr_device *gdev;
	vcpm_cmd_set_cfg_t vcpm_set_cfg;
	uint32_t size;

	//return 0;

	size = GPR_HDR_SIZE + sizeof (vcpm_cmd_set_cfg_t);
	pkt = kzalloc(size, GFP_KERNEL);

	if (!pkt) {
		pr_err("%s: memory allocation failed\n", __func__);
		return -ENOMEM;
	}

	if (!voice_mhi_lcl.gdev) {
		pr_err("%s: GPR handle is NULL\n", __func__);
		return -EINVAL;
	}

	memset(pkt, 0, sizeof(struct gpr_pkt));
	memset(&vcpm_set_cfg, 0, sizeof(vcpm_cmd_set_cfg_t));

	pkt->hdr.header = GPR_SET_FIELD(GPR_PKT_VERSION, GPR_PKT_VER) |
			 GPR_SET_FIELD(GPR_PKT_HEADER_SIZE, GPR_PKT_HEADER_WORD_SIZE_V) |
			 GPR_SET_FIELD(GPR_PKT_PACKET_SIZE, size);

	pkt->hdr.opcode = APM_CMD_SET_CFG;
	pkt->hdr.dst_port = VCPM_MODULE_INSTANCE_ID;
	pkt->hdr.src_port = GPR_SVC_VPM;
	pkt->hdr.dst_domain_id = GPR_IDS_DOMAIN_ID_ADSP_V;
	pkt->hdr.src_domain_id = GPR_IDS_DOMAIN_ID_APPS_V;

	mutex_lock(&voice_mhi_lcl.mutex);
	gdev = voice_mhi_lcl.gdev;

	/*
	 * Handle can be NULL as it is not tied to any session
	 */
	vcpm_set_cfg.payload_header.payload_address_lsw = 0;
	vcpm_set_cfg.payload_header.payload_address_msw = 0;
	vcpm_set_cfg.payload_header.mem_map_handle = 0;
	vcpm_set_cfg.payload_header.payload_size = sizeof(vcpm_cmd_set_cfg_t) - sizeof(apm_cmd_header_t);

	vcpm_set_cfg.module_payload.module_instance_id = VCPM_MODULE_INSTANCE_ID;
	vcpm_set_cfg.module_payload.error_code = 0;
	vcpm_set_cfg.module_payload.param_id = VCPM_PARAM_ID_MAILBOX_MEMORY_CONFIG;
	vcpm_set_cfg.module_payload.param_size =
		sizeof(vcpm_cmd_set_cfg_t) - sizeof(apm_cmd_header_t) - sizeof(apm_module_param_data_t);

	vcpm_set_cfg.mb_memory_config.mailbox_mem_address_pcie_lsw =
			(uint32_t) voice_mhi_lcl.dev_info.iova_pcie.base;
	vcpm_set_cfg.mb_memory_config.mailbox_mem_address_pcie_msw =
			(uint64_t) voice_mhi_lcl.dev_info.iova_pcie.base >> 32;
	vcpm_set_cfg.mb_memory_config.mailbox_mem_address_adsp_lsw =
			(uint32_t) voice_mhi_lcl.dev_info.iova_adsp.base;
	vcpm_set_cfg.mb_memory_config.mailbox_mem_address_adsp_msw =
			(uint64_t) voice_mhi_lcl.dev_info.iova_adsp.base >> 32;
	vcpm_set_cfg.mb_memory_config.mem_size = voice_mhi_lcl.dev_info.iova_adsp.size;

	voice_mhi_lcl.mvm_state = CMD_STATUS_FAIL;
	voice_mhi_lcl.async_err = 0;
	memcpy(&pkt->payload, &vcpm_set_cfg, sizeof(vcpm_cmd_set_cfg_t));

	ret = gpr_send_pkt(gdev, pkt);
	if (ret < 0) {
		pr_err("%s: Set mailbox memory config failed ret = %d\n",
				__func__, ret);
		goto unlock;
	}

	ret = wait_event_timeout(voice_mhi_lcl.voice_mhi_wait,
				 (voice_mhi_lcl.mvm_state ==
				 CMD_STATUS_SUCCESS),
				 msecs_to_jiffies(TIMEOUT_MS));
	if (!ret) {
		pr_err("%s: wait_event timeout\n", __func__);
		ret = -ETIME;
		goto unlock;
	}
	if (voice_mhi_lcl.async_err > 0) {
		pr_err("%s: DSP returned error[%d]\n",
				__func__, voice_mhi_lcl.async_err);
		ret = voice_mhi_lcl.async_err;
		goto unlock;
	}
	ret = 0;
unlock:
	mutex_unlock(&voice_mhi_lcl.mutex);
	return ret;
}

static void voice_mhi_map_pcie_and_send(struct work_struct *work)
{
	dma_addr_t iova, phys_addr;
	uint32_t mem_size;
	struct device *md;

	mutex_lock(&voice_mhi_lcl.mutex);
	if (voice_mhi_lcl.mhi_dev) {
		md = &voice_mhi_lcl.mhi_dev->dev;
	} else {
		pr_err("%s: MHI device handle is NULL\n", __func__);
		goto err;
	}

	phys_addr = voice_mhi_lcl.dev_info.phys_addr.base;
	mem_size = voice_mhi_lcl.dev_info.iova_pcie.size;
	if (md) {
		iova = dma_map_resource(md->parent, phys_addr, mem_size,
					DMA_BIDIRECTIONAL, 0);
		if (dma_mapping_error(md->parent, iova)) {
			pr_err("%s: dma_mapping_error\n", __func__);
			goto err;
		}
		pr_err("%s: dma_mapping_success iova:0x%lx\n",
				 __func__, (unsigned long)iova);
		voice_mhi_lcl.dev_info.iova_pcie.base = iova;
		mutex_unlock(&voice_mhi_lcl.mutex);

		if (gecko_core_is_apm_ready()) {
			voice_mhi_set_mailbox_memory_config();
			voice_mhi_start();
		}

		return;
	}

err:
	mutex_unlock(&voice_mhi_lcl.mutex);
	return;
}

static void voice_mhi_gpr_send(struct work_struct *work)
{
	// ret = voice_mhi_apr_register();
	// if (ret) {
		// pr_err("%s: APR registration failed %d\n", __func__, ret);
		// return;
	// }
	pr_err("%s: GPR is up \n", __func__);
	mutex_lock(&voice_mhi_lcl.mutex);
	if (VOICE_MHI_STATE_CHECK(voice_mhi_lcl.voice_mhi_state,
				VOICE_MHI_SDX_UP)) {
		mutex_unlock(&voice_mhi_lcl.mutex);
		if (gecko_core_is_apm_ready()) {
			voice_mhi_set_mailbox_memory_config();
			voice_mhi_start();
			return;
		} else {
			pr_err("%s: Gecko core not ready \n", __func__);
		}
	} else {
		mutex_unlock(&voice_mhi_lcl.mutex);
		schedule_work(&voice_mhi_lcl.voice_mhi_work_adsp);
	}
}

static int voice_mhi_pcie_up_callback(struct mhi_device *voice_mhi_dev,
				const struct mhi_device_id *id)
{

	if ((!voice_mhi_dev) || (id != &voice_mhi_match_table[0])) {
		pr_err("%s: Invalid device or table received\n", __func__);
		return -EINVAL;
	}
	pr_err("%s: MHI PCIe UP callback\n", __func__);
	mutex_lock(&voice_mhi_lcl.mutex);
	voice_mhi_lcl.mhi_dev = voice_mhi_dev;
	VOICE_MHI_STATE_SET(voice_mhi_lcl.voice_mhi_state, VOICE_MHI_SDX_UP);
	mutex_unlock(&voice_mhi_lcl.mutex);
	schedule_work(&voice_mhi_lcl.voice_mhi_work_pcie);
	return 0;
}

static void voice_mhi_pcie_down_callback(struct mhi_device *voice_mhi_dev)
{
	dma_addr_t iova;
	struct device *md = NULL;

	mutex_lock(&voice_mhi_lcl.mutex);

	if (voice_mhi_lcl.mhi_dev)
		md = &voice_mhi_lcl.mhi_dev->dev;

	VOICE_MHI_STATE_RESET(voice_mhi_lcl.voice_mhi_state, VOICE_MHI_SDX_UP);
	iova = voice_mhi_lcl.dev_info.iova_pcie.base;

	if (md)
		dma_unmap_resource(md->parent, iova, PAGE_SIZE,
				   DMA_BIDIRECTIONAL, 0);

	voice_mhi_lcl.mhi_dev = NULL;
	voice_mhi_lcl.vote_count = 0;
	mutex_unlock(&voice_mhi_lcl.mutex);
}

static void voice_mhi_pcie_status_callback(struct mhi_device *voice_mhi_dev,
					enum MHI_CB mhi_cb)
{

}

static int voice_mhi_gpr_probe(struct gpr_device *gdev)
{
	int ret = 0;

	pr_err("%s Enter ", __func__);
	mutex_lock(&voice_mhi_lcl.mutex);
	voice_mhi_lcl.gdev = gdev;
	VOICE_MHI_STATE_SET(voice_mhi_lcl.voice_mhi_state,
			VOICE_MHI_ADSP_UP);
	mutex_unlock(&voice_mhi_lcl.mutex);
	//schedule_work(&voice_mhi_lcl.voice_mhi_work_adsp);
	pr_err("%s Exit ", __func__);
	return ret;
}

static int voice_mhi_gpr_exit(struct gpr_device *gdev)
{
	mutex_lock(&voice_mhi_lcl.mutex);
	voice_mhi_end();
	voice_mhi_lcl.gdev = NULL;
	VOICE_MHI_STATE_RESET(voice_mhi_lcl.voice_mhi_state,
					VOICE_MHI_ADSP_UP);
	mutex_unlock(&voice_mhi_lcl.mutex);

	return 0;
}

static const struct of_device_id voice_mhi_gpr_device_id[]  = {
	{ .compatible = "qcom,voice_mhi_gpr" },
	{},
};
MODULE_DEVICE_TABLE(of, voice_mhi_gpr_device_id);

static struct gpr_driver voice_mhi_gpr_driver = {
	.probe = voice_mhi_gpr_probe,
	.remove = voice_mhi_gpr_exit,
	.callback = voice_mhi_gpr_callback,
	.driver = {
		.name = "qcom-voice_mhi_gpr",
		.of_match_table = of_match_ptr(voice_mhi_gpr_device_id),
	},
};


// static int voice_mhi_apr_register(void)
// {
	// int ret = 0;

	// mutex_lock(&voice_mhi_lcl.mutex);
	// voice_mhi_lcl.apr_mvm_handle = apr_register("ADSP", "MVM",
						// (apr_fn)voice_mhi_apr_callback,
						// CONVERT_PORT_APR(PORT_NUM,
							// PORT_MASK),
						// &voice_mhi_lcl);
	// if (voice_mhi_lcl.apr_mvm_handle == NULL) {
		// pr_err("%s: error in APR register\n", __func__);
		// ret = -ENODEV;
	// }
	// mutex_unlock(&voice_mhi_lcl.mutex);

	// return ret;
// }

static int voice_mhi_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *node;
	uint32_t mem_size = 0;
	void *ptr;
	dma_addr_t phys_addr, iova;
	const __be32 *cell;

	pr_err("%s: Enter\n", __func__);

	memset(&voice_mhi_lcl, 0, sizeof(voice_mhi_lcl));
	mutex_init(&voice_mhi_lcl.mutex);

	/* Add remaining init here */
	voice_mhi_lcl.pcie_enabled = false;
	voice_mhi_lcl.voice_mhi_state = VOICE_MHI_INIT;
	voice_mhi_lcl.vote_count = 0;
	voice_mhi_lcl.gdev = NULL;

	INIT_WORK(&voice_mhi_lcl.voice_mhi_work_pcie,
				voice_mhi_map_pcie_and_send);
	INIT_WORK(&voice_mhi_lcl.voice_mhi_work_adsp,
				voice_mhi_gpr_send);
	init_waitqueue_head(&voice_mhi_lcl.voice_mhi_wait);

	node = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (node) {
		cell = of_get_property(node, "size", NULL);
		if (cell)
			mem_size = of_read_number(cell, 2);
		else {
			pr_err("%s: cell not found\n", __func__);
			ret = -EINVAL;
			goto done;
		}
	} else {
		pr_err("%s: Node read failed\n", __func__);
		ret = -EINVAL;
		goto done;
	}

	pr_err("%s: mem_size = %d\n", __func__, mem_size);

	if (mem_size) {
		ptr = dma_alloc_attrs(&pdev->dev, mem_size, &phys_addr,
					GFP_KERNEL, DMA_ATTR_NO_KERNEL_MAPPING);
		if (IS_ERR_OR_NULL(ptr)) {
			pr_err("%s: Memory alloc failed\n", __func__);
			ret = -ENOMEM;
			goto done;
		} else {
			pr_err("%s: Memory alloc success phys_addr:0x%lx\n",
					 __func__, (unsigned long)phys_addr);
		}

		ret = msm_audio_ion_dma_map(&phys_addr, &iova, mem_size,
					DMA_BIDIRECTIONAL);
		if (ret) {
			pr_err("%s: dma mapping failed %d\n", __func__, ret);
			goto err_free;
		}
		pr_err("%s: dma_mapping_success iova:0x%lx\n",
				 __func__, (unsigned long)iova);
		voice_mhi_lcl.dev_info.iova_adsp.size = mem_size;
		voice_mhi_lcl.dev_info.iova_pcie.size = mem_size;
		voice_mhi_lcl.dev_info.pdev = pdev;
		voice_mhi_lcl.dev_info.phys_addr.base = phys_addr;
		voice_mhi_lcl.dev_info.iova_adsp.base = iova;

		ret = gpr_driver_register(&voice_mhi_gpr_driver);
		if (ret) {
			pr_err("%s: gpr driver register failed = %d\n",
				__func__, ret);
			ret = -EINVAL;
			goto err_free;
		}
		pr_err("%s: gpr register success",__func__);
		ret = mhi_driver_register(&voice_mhi_driver);
		if (ret) {
			pr_err("%s: mhi register failed %d\n", __func__, ret);
			goto err_free;
		}
		mutex_lock(&voice_mhi_lcl.mutex);
		voice_mhi_lcl.pcie_enabled = true;
		VOICE_MHI_STATE_SET(voice_mhi_lcl.voice_mhi_state,
				VOICE_MHI_PROBED);
		mutex_unlock(&voice_mhi_lcl.mutex);
	} else {
		pr_err("%s: Memory size can't be zero\n", __func__);
		ret = -ENOMEM;
		goto done;
	}

done:
	return ret;
err_free:
	dma_free_attrs(&pdev->dev, mem_size, ptr, phys_addr,
						   DMA_ATTR_NO_KERNEL_MAPPING);
	return ret;

}

static int voice_mhi_remove(struct platform_device *pdev)
{
	if (voice_mhi_lcl.gdev)
		voice_mhi_lcl.gdev = NULL;
	mhi_driver_unregister(&voice_mhi_driver);
	gpr_driver_unregister(&voice_mhi_gpr_driver);
	memset(&voice_mhi_lcl, 0, sizeof(voice_mhi_lcl));
	mutex_destroy(&voice_mhi_lcl.mutex);
	return 0;
}
static const struct of_device_id voice_mhi_of_match[]  = {
	{ .compatible = "qcom,voice-mhi-audio", },
	{},
};
static struct platform_driver voice_mhi_platform_driver = {
	.probe = voice_mhi_probe,
	.remove = voice_mhi_remove,
	.driver = {
		.name = "voice_mhi_audio",
		.owner = THIS_MODULE,
		.of_match_table = voice_mhi_of_match,
		.suppress_bind_attrs = true,
	}
};

module_platform_driver(voice_mhi_platform_driver);

MODULE_DESCRIPTION("Voice MHI module");
MODULE_LICENSE("GPL v2");
