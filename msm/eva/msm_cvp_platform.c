// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/of_fdt.h>
#include "msm_cvp_internal.h"
#include "msm_cvp_debug.h"
#include "cvp_hfi_api.h"
#include "cvp_hfi.h"

#define UBWC_CONFIG(mco, mlo, hbo, bslo, bso, rs, mc, ml, hbb, bsl, bsp) \
{	\
	.override_bit_info.max_channel_override = mco,	\
	.override_bit_info.mal_length_override = mlo,	\
	.override_bit_info.hb_override = hbo,	\
	.override_bit_info.bank_swzl_level_override = bslo,	\
	.override_bit_info.bank_spreading_override = bso,	\
	.override_bit_info.reserved = rs,	\
	.max_channels = mc,	\
	.mal_length = ml,	\
	.highest_bank_bit = hbb,	\
	.bank_swzl_level = bsl,	\
	.bank_spreading = bsp,	\
}

static struct msm_cvp_common_data default_common_data[] = {
	{
		.key = "qcom,never-unload-fw",
		.value = 1,
	},
};

static struct msm_cvp_common_data sm8450_common_data[] = {
	{
		.key = "qcom,auto-pil",
		.value = 1,
	},
	{
		.key = "qcom,never-unload-fw",
		.value = 1,
	},
	{
		.key = "qcom,sw-power-collapse",
		.value = 1,
	},
	{
		.key = "qcom,domain-attr-non-fatal-faults",
		.value = 0,
	},
	{
		.key = "qcom,max-secure-instances",
		.value = 2,             /*
					 * As per design driver allows 3rd
					 * instance as well since the secure
					 * flags were updated later for the
					 * current instance. Hence total
					 * secure sessions would be
					 * max-secure-instances + 1.
					 */
	},
	{
		.key = "qcom,max-ssr-allowed",
		.value = 1,		/*
					 * Maxinum number of SSR before BUG_ON
					 */
	},
	{
		.key = "qcom,power-collapse-delay",
		.value = 3000,
	},
	{
		.key = "qcom,hw-resp-timeout",
		.value = 2000,
	},
	{
		.key = "qcom,dsp-resp-timeout",
		.value = 1000,
	},
	{
		.key = "qcom,debug-timeout",
		.value = 0,
	},
	{
		.key = "qcom,dsp-enabled",
		.value = 1,
	}
};

static struct msm_cvp_common_data sm8550_common_data[] = {
	{
		.key = "qcom,auto-pil",
		.value = 0,
	},
	{
		.key = "qcom,never-unload-fw",
		.value = 1,
	},
	{
		.key = "qcom,sw-power-collapse",
		.value = 0,
	},
	{
		.key = "qcom,domain-attr-non-fatal-faults",
		.value = 0,
	},
	{
		.key = "qcom,max-secure-instances",
		.value = 2,             /*
					 * As per design driver allows 3rd
					 * instance as well since the secure
					 * flags were updated later for the
					 * current instance. Hence total
					 * secure sessions would be
					 * max-secure-instances + 1.
					 */
	},
	{
		.key = "qcom,max-ssr-allowed",
		.value = 1,		/*
					 * Maxinum number of SSR before BUG_ON
					 */
	},
	{
		.key = "qcom,power-collapse-delay",
		.value = 3000,
	},
	{
		.key = "qcom,hw-resp-timeout",
		.value = 2000,
	},
	{
		.key = "qcom,dsp-resp-timeout",
		.value = 1000,
	},
	{
		.key = "qcom,debug-timeout",
		.value = 0,
	},
	{
		.key = "qcom,dsp-enabled",
		.value = 0,
	}
};



/* Default UBWC config for LPDDR5 */
static struct msm_cvp_ubwc_config_data kona_ubwc_data[] = {
	UBWC_CONFIG(1, 1, 1, 0, 0, 0, 8, 32, 16, 0, 0),
};

static struct msm_cvp_qos_setting waipio_noc_qos = {
	.axi_qos = 0x99,
	.prioritylut_low = 0x22222222,
	.prioritylut_high = 0x33333333,
	.urgency_low = 0x1022,
	.dangerlut_low = 0x0,
	.safelut_low = 0xffff,
};

static struct msm_cvp_platform_data default_data = {
	.common_data = default_common_data,
	.common_data_length =  ARRAY_SIZE(default_common_data),
	.sku_version = 0,
	.vpu_ver = VPU_VERSION_5,
	.ubwc_config = 0x0,
	.noc_qos = 0x0,
};

static struct msm_cvp_platform_data sm8450_data = {
	.common_data = sm8450_common_data,
	.common_data_length =  ARRAY_SIZE(sm8450_common_data),
	.sku_version = 0,
	.vpu_ver = VPU_VERSION_5,
	.ubwc_config = kona_ubwc_data,
	.noc_qos = &waipio_noc_qos,
};

static struct msm_cvp_platform_data sm8550_data = {
	.common_data = sm8550_common_data,
	.common_data_length =  ARRAY_SIZE(sm8550_common_data),
	.sku_version = 0,
	.vpu_ver = VPU_VERSION_5,
	.ubwc_config = kona_ubwc_data,
	.noc_qos = 0x0,
};

static const struct of_device_id msm_cvp_dt_match[] = {
	{
		.compatible = "qcom,waipio-cvp",
		.data = &sm8450_data,
	},
	{
		.compatible = "qcom,kailua-cvp",
		.data = &sm8550_data,
	},

	{},
};

const struct msm_cvp_hfi_defs cvp_hfi_defs[] = {
	{
		.size = HFI_DFS_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DFS_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DFS_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DFS_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_SGM_OF_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_SGM_OF_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_WARP_NCC_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_WARP_NCC_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_WARP_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_WARP_DS_PARAMS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_WARP_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DMM_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DMM_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_DMM_PARAMS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DMM_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DMM_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_PERSIST_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_SET_PERSIST_BUFFERS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xffffffff,
		.type = HFI_CMD_SESSION_CVP_RELEASE_PERSIST_BUFFERS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DS_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DS,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_OF_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_CV_TME_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_OF_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_CV_TME_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_ODT_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_CV_ODT_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_ODT_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_CV_ODT_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_OD_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_CV_OD_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_OD_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_CV_OD_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_NCC_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_NCC_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_NCC_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_NCC_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_ICA_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_ICA_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_ICA_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_ICA_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_HCD_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_HCD_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_HCD_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_HCD_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DCM_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DC_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DCM_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DC_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DCM_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DCM_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_DCM_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_DCM_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_PYS_HCD_CONFIG_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_PYS_HCD_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = HFI_PYS_HCD_FRAME_CMD_SIZE,
		.type = HFI_CMD_SESSION_CVP_PYS_HCD_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_SET_MODEL_BUFFERS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_SET_SNAPSHOT_BUFFERS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_RELEASE_SNAPSHOT_BUFFERS,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_SET_SNAPSHOT_MODE,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_SNAPSHOT_WRITE_DONE,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_FD_CONFIG,
		.is_config_pkt = true,
		.resp = HAL_NO_RESP,
	},
	{
		.size = 0xFFFFFFFF,
		.type = HFI_CMD_SESSION_CVP_FD_FRAME,
		.is_config_pkt = false,
		.resp = HAL_NO_RESP,
	},

};

int get_pkt_array_size(void)
{
	return ARRAY_SIZE(cvp_hfi_defs);
}

int get_pkt_index(struct cvp_hal_session_cmd_pkt *hdr)
{
	int i;

	for (i = 0; i < get_pkt_array_size(); i++)
		if (cvp_hfi_defs[i].type == hdr->packet_type)
			return i;

	return -EINVAL;
}

MODULE_DEVICE_TABLE(of, msm_cvp_dt_match);

int cvp_of_fdt_get_ddrtype(void)
{
#ifdef FIXED_DDR_TYPE
	/* of_fdt_get_ddrtype() is usually unavailable during pre-sil */
	return DDR_TYPE_LPDDR5;
#else
	return of_fdt_get_ddrtype();
#endif
}

void *cvp_get_drv_data(struct device *dev)
{
	struct msm_cvp_platform_data *driver_data;
	const struct of_device_id *match;
	uint32_t ddr_type = DDR_TYPE_LPDDR5;

	driver_data = &default_data;

	if (!IS_ENABLED(CONFIG_OF) || !dev->of_node)
		goto exit;

	match = of_match_node(msm_cvp_dt_match, dev->of_node);

	if (!match)
		return NULL;

	driver_data = (struct msm_cvp_platform_data *)match->data;

	if (!strcmp(match->compatible, "qcom,waipio-cvp")) {
		ddr_type = cvp_of_fdt_get_ddrtype();
		if (ddr_type == -ENOENT) {
			dprintk(CVP_ERR,
				"Failed to get ddr type, use LPDDR5\n");
		}

		if (driver_data->ubwc_config &&
			(ddr_type == DDR_TYPE_LPDDR4 ||
			ddr_type == DDR_TYPE_LPDDR4X))
			driver_data->ubwc_config->highest_bank_bit = 15;
		dprintk(CVP_CORE, "DDR Type 0x%x hbb 0x%x\n",
			ddr_type, driver_data->ubwc_config ?
			driver_data->ubwc_config->highest_bank_bit : -1);
	}
exit:
	return driver_data;
}
