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

#ifndef __AUDIO_PRM_H__
#define __AUDIO_PRM_H__

#define PRM_MODULE_INSTANCE_ID 0x00000002

/* Request and release response payload of the #PRM_CMD_RSP_REQUEST_HW_RSC
 * and PRM_CMD_RSP_ RELEASE_HW_RSC commands respectively.
 * This is sent in response to request and release common payload commands.
 */
struct prm_rsp_req_rel_hw_rsc {
	/* ID of the resource being requested. The supported values
	 * are HW_RSC_ID_AUDIO_HW_CLK and HW_RSC_ID_LPASS_CORE_CLK.
	 */
	uint32_t param_id;

	/* Overall command response status with value 0 for success and non-zero
	 * value for command failure. In case of partial or complete command
	 * failure, this field is followed by resource specific response payload.
	 */
	uint32_t status;
};

/* Clock configuration structure for request */
struct clk_cfg {
	/* Unique ID of the clock being requested. */
	uint32_t clk_id;
	/* Clock frequency in Hz. */
	uint32_t clk_freq_in_hz;
	/* Clock attributes with values
	 * 0:invalid
	 * 1:CLOCK_ATTR_NO_COUPLING
	 * 2:CLOCK_ATTR_COUPLE_DIVIDEND
	 * 3:CLOCK_ATTR_COUPLE_DIVISOR
	 * 4:CLOCK_ATTR_INVERT_NO_COUPLING*/
	uint32_t clk_attri;
	/* Clock root for this clock with 0 as DEFAULT. */
	uint32_t clk_root;
};

/* Below payload is used to report failed clock ID's and error code. */
struct hw_rsc_clk_req_rel_rsp {
	/* Number of failures while requesting for 1 or more clock IDs. */
	uint32_t num_failures;
};

/* Clock status structure for response payload */
struct clock_status {
	/* ID of the clock being requested as part of the REQUEST
	 * command and resulted in failure.
	 */
	uint32_t clock_id;

	/* Error code corresponding to failure occurred while
	 * requesting this clock ID.
	 */
	uint32_t status;
};

typedef struct apm_cmd_header_t
{
	uint32_t payload_address_lsw;
	/**< Lower 32 bits of the payload address. */
	uint32_t payload_address_msw;
	/**< Upper 32 bits of the payload address.

	The 64-bit number formed by payload_address_lsw and
	payload_address_msw must be aligned to a 32-byte boundary and be in
	contiguous memory.

	@values
	- For a 32-bit shared memory address, this field must be set to 0.
	- For a 36-bit shared memory address, bits 31 to 4 of this field must
	  be set to 0. @tablebulletend */
	uint32_t mem_map_handle;
	/**< Unique identifier for a shared memory address.

	@values
	- NULL -- The message is in the payload (in-band).
	- Non-NULL -- The parameter data payload begins at the address
	 specified by a pointer to the physical address of the payload in
	 shared memory (out-of-band).
	@contcell
	The aDSP returns this memory map handle through
	#apm_CMD_SHARED_MEM_MAP_REGIONS.

	An optional field is available if parameter data is in-band:
	%afe_port_param_data_v2_t param_data[...].
	See <b>Parameter data variable payload</b>. */

	uint32_t payload_size;
	/**< Actual size of the variable payload accompanying the message or in
	shared memory. This field is used for parsing both in-band and
	out-of-band data.

	@values > 0 bytes, in multiples of 4 bytes */
}apm_cmd_header_t;

typedef struct apm_module_param_data_t
{
	uint32_t module_instance_id;
	/**< Valid instance ID of module
	@values  */

	uint32_t param_id;
	/**< Valid ID of the parameter.

	@values See Chapter */

	uint32_t param_size;
	/**< Size of the parameter data based upon the
	module_instance_id/param_id combination.
	@values > 0 bytes, in multiples of
	4 bytes at least */

	uint32_t error_code;
	/**< Error code populated by the entity hosting the	module.
	 Applicable only for out-of-band command mode  */
}apm_module_param_data_t;

typedef struct audio_hw_clk_cfg_req_param_t
{
	uint32_t num_clock_id;
	/**< Number of clock ID's being configured */
}audio_hw_clk_cfg_req_param_t;

typedef struct audio_hw_clk_cfg_t
{
	uint32_t clock_id;
	/**< Unique Clock ID of the clock being requested */

	uint32_t clock_freq;
	/**< Clock frequency in Hz to set. */

	uint32_t clock_attri;
	/**< Divider for two clocks that are coupled, if necessary:
	divider = A/B, where A is the dividend and B is the divisor.
	@values
	- #_CLOCK_ATTRIBUTE_COUPLE_NO -- For no divider-related clocks
	- #_CLOCK_ATTRIBUTE_COUPLE_DIVIDEND
	- #_CLOCK_ATTRIBUTE_COUPLE_DIVISOR
	- #_CLOCK_ATTRIBUTE_INVERT_COUPLE_NO */

	uint32_t clock_root;
	/**< Root clock source.
	@values #_CLOCK_ROOT_DEFAULT
	Currently, only _CLOCK_ROOT_DEFAULT is valid. */
}audio_hw_clk_cfg_t;

typedef struct audio_hw_clk_rel_cfg_t
{
	uint32_t clock_id;
}audio_hw_clk_rel_cfg_t;

#define MAX_AUD_HW_CLK_NUM_REQ 1

typedef struct prm_cmd_request_rsc_t
{
	apm_cmd_header_t payload_header;
	apm_module_param_data_t module_payload_0;
	audio_hw_clk_cfg_req_param_t num_clk_id_t;
	audio_hw_clk_cfg_t clock_ids_t[MAX_AUD_HW_CLK_NUM_REQ];
}prm_cmd_request_rsc_t;

typedef struct prm_cmd_release_rsc_t
{
	apm_cmd_header_t payload_header;
	apm_module_param_data_t module_payload_0;
	audio_hw_clk_cfg_req_param_t num_clk_id_t;
	audio_hw_clk_rel_cfg_t clock_ids_t[MAX_AUD_HW_CLK_NUM_REQ];
}prm_cmd_release_rsc_t;

typedef struct prm_cmd_request_hw_core_t
{
        apm_cmd_header_t payload_header;
        apm_module_param_data_t module_payload_0;
        uint32_t hw_core_id;
}prm_cmd_request_hw_core_t;


#define PRM_CMD_REQUEST_HW_RSC 0x0100100F

#define PRM_CMD_RELEASE_HW_RSC 0x01001010

#define PRM_CMD_RSP_REQUEST_HW_RSC 0x02001002

#define PRM_CMD_RSP_RELEASE_HW_RSC 0x02001003

/* Param ID for audio hardware clock */

#define PARAM_ID_RSC_AUDIO_HW_CLK 0x0800102C

/* Param ID for lpass core clock */

#define PARAM_ID_RSC_LPASS_CORE 0x0800102B
#define PARAM_ID_RSC_HW_CORE 0x08001032

#define HW_RSC_ID_AUDIO_HW_CLK 0x0800102C

#define MAX_EARPA_REG 2
#define MAX_EARPA_CDC_DUTY_CYC_OPERATION 2

typedef struct audio_hw_codec_op_info_t {
	uint32_t hw_codec_op_id;
	uint32_t hw_codec_op_value;
} audio_hw_codec_op_info_t;

typedef struct audio_hw_codec_reg_op_info_t {
	uint32_t hw_codec_reg_id;
	uint32_t hw_codec_reg_addr_msw;
	uint32_t hw_codec_reg_addr_lsw;
	uint32_t num_ops;
	audio_hw_codec_op_info_t hw_codec_op[MAX_EARPA_REG];
} audio_hw_codec_reg_op_info_t;

typedef struct audio_hw_codec_reg_info_t {
	uint32_t num_reg_info_t;
	audio_hw_codec_reg_op_info_t hw_codec_reg[MAX_EARPA_REG];
} audio_hw_codec_reg_info_t;

typedef struct prm_cmd_request_cdc_duty_cycling_t {
	apm_cmd_header_t payload_header;
	apm_module_param_data_t module_payload_0;
	audio_hw_codec_reg_info_t   hw_codec_reg_info_t;
} prm_cmd_request_cdc_duty_cycling_t;

/* earpa_register config */
#define DIG_MUTE_ENABLE 0x34
#define DIG_MUTE_DISABLE 0x24

struct lpass_swr_ear_pa_dep_cfg_t {
	uint32_t ear_pa_enable_pkd_reg_addr;
	uint32_t ear_pa_disable_pkd_reg_addr;
} __packed;

struct lpass_swr_ear_pa_reg_cfg_t {
	uint32_t lpass_cdc_rx0_rx_path_ctl_phy_addr;
	uint32_t lpass_wr_fifo_reg_phy_addr;
} __packed;

struct prm_earpa_hw_intf_config {
	struct lpass_swr_ear_pa_reg_cfg_t ear_pa_hw_reg_cfg;
	struct lpass_swr_ear_pa_dep_cfg_t ear_pa_pkd_cfg;
	uint32_t ear_pa_pkd_reg_addr;
	const char  *backend_used;
} __packed;

#define PARAM_ID_RSC_HW_CODEC_REG_INFO 0x0800131B

#define HW_CODEC_DIG_REG_ID_MUTE_CTRL 0x1
#define HW_CODEC_OP_DIG_MUTE_ENABLE 0x1
#define HW_CODEC_OP_DIG_MUTE_DISABLE 0x2

#define HW_CODEC_ANALOG_REG_ID_CMD_FIFO_WRITE 0x2
#define HW_CODEC_OP_ANA_PGA_ENABLE 0x3
#define HW_CODEC_OP_ANA_PGA_DISABLE 0x4

/* Supported OSR clock values */
#define OSR_CLOCK_12_P288_MHZ	0xBB8000
#define OSR_CLOCK_11_P2896_MHZ	0xAC4400
#define OSR_CLOCK_9_P600_MHZ	0x927C00
#define OSR_CLOCK_8_P192_MHZ	0x7D0000
#define OSR_CLOCK_6_P144_MHZ	0x5DC000
#define OSR_CLOCK_4_P096_MHZ	0x3E8000
#define OSR_CLOCK_3_P072_MHZ	0x2EE000
#define OSR_CLOCK_2_P048_MHZ	0x1F4000
#define OSR_CLOCK_1_P536_MHZ	0x177000
#define OSR_CLOCK_1_P024_MHZ	0xFA000
#define OSR_CLOCK_768_kHZ	0xBB800
#define OSR_CLOCK_512_kHZ	0x7D000
#define OSR_CLOCK_DISABLE	0x0

/* Supported Bit clock values */
#define IBIT_CLOCK_12_P288_MHZ	0xBB8000
#define IBIT_CLOCK_11_P2896_MHZ	0xAC4400
#define IBIT_CLOCK_8_P192_MHZ	0x7D0000
#define IBIT_CLOCK_6_P144_MHZ	0x5DC000
#define IBIT_CLOCK_4_P096_MHZ	0x3E8000
#define IBIT_CLOCK_3_P072_MHZ	0x2EE000
#define IBIT_CLOCK_2_P8224_MHZ	0x2b1100
#define IBIT_CLOCK_2_P048_MHZ	0x1F4000
#define IBIT_CLOCK_1_P536_MHZ	0x177000
#define IBIT_CLOCK_1_P4112_MHZ	0x158880
#define IBIT_CLOCK_1_P024_MHZ	 0xFA000
#define IBIT_CLOCK_768_KHZ	 0xBB800
#define IBIT_CLOCK_512_KHZ	 0x7D000
#define IBIT_CLOCK_256_KHZ	 0x3E800
#define IBIT_CLOCK_DISABLE	     0x0

/** Clock ID of the primary MI2S internal bit clock (IBIT). */

#define CLOCK_ID_PRI_MI2S_IBIT 0x100

/** Clock ID of the primary MI2S external bit clock (EBIT). */

#define CLOCK_ID_PRI_MI2S_EBIT 0x101

/** Clock ID of the secondary MI2S IBIT. */

#define CLOCK_ID_SEC_MI2S_IBIT 0x102

/** Clock ID of the secondary MI2S EBIT. */

#define CLOCK_ID_SEC_MI2S_EBIT 0x103

/** Clock ID of the tertiary MI2S IBIT. */

#define CLOCK_ID_TER_MI2S_IBIT 0x104

/** Clock ID of the tertiary MI2S EBIT. */

#define CLOCK_ID_TER_MI2S_EBIT 0x105

/** Clock ID of the quaternary MI2S IBIT. */

#define CLOCK_ID_QUAD_MI2S_IBIT 0x106

/** Clock ID of the quaternary MI2S EBIT. */

#define CLOCK_ID_QUAD_MI2S_EBIT 0x107

/** Clock ID of the quinary MI2S IBIT. */

#define CLOCK_ID_QUI_MI2S_IBIT 0x108

/** Clock ID of the quinary MI2S EBIT. */

#define CLOCK_ID_QUI_MI2S_EBIT 0x109

/** Clock ID of the quinary MI2S OSR. */

#define CLOCK_ID_QUI_MI2S_OSR 0x10A

/** Clock ID of the senary MI2S IBIT. */

#define CLOCK_ID_SEN_MI2S_IBIT 0x10B

/** Clock ID of the senary MI2S EBIT. */

#define CLOCK_ID_SEN_MI2S_EBIT 0x10C

/** Clock ID of the septenary MI2S IBIT. */
#define CLOCK_ID_SEP_MI2S_IBIT 0x10D

/** Clock ID of the septenary MI2S EBIT. */
#define CLOCK_ID_SEP_MI2S_EBIT 0x10E

/** ID of I2S IBIT clock 0 that is used with integrated codec. */

#define CLOCK_ID_INT0_I2S_IBIT 0x10D

/** ID of I2S IBIT clock 1 that is used with integrated codec. */

#define CLOCK_ID_INT1_I2S_IBIT 0x10E

/** ID of I2S IBIT clock 2 that is used with integrated codec. */

#define CLOCK_ID_INT2_I2S_IBIT 0x10F

/** ID of I2S IBIT clock 3 that is used with integrated codec. */

#define CLOCK_ID_INT3_I2S_IBIT 0x110

/** ID of I2S IBIT clock 4 that is used with integrated codec. */

#define CLOCK_ID_INT4_I2S_IBIT 0x111

/** ID of I2S IBIT clock 5 that is used with integrated codec. */

#define CLOCK_ID_INT5_I2S_IBIT 0x112

/** ID of I2S IBIT clock 6 that is used with integrated codec. */

#define CLOCK_ID_INT6_I2S_IBIT 0x113

/** Clock ID of the primary PCM IBIT. */

#define CLOCK_ID_PRI_PCM_IBIT 0x200

/** Clock ID of the primary PCM EBIT. */

#define CLOCK_ID_PRI_PCM_EBIT 0x201

/** Clock ID of the secondary PCM IBIT. */

#define CLOCK_ID_SEC_PCM_IBIT 0x202

/** Clock ID of the secondary PCM EBIT. */

#define CLOCK_ID_SEC_PCM_EBIT 0x203

/** Clock ID of the tertiary PCM IBIT. */

#define CLOCK_ID_TER_PCM_IBIT 0x204

/** Clock ID of the tertiary PCM EBIT. */

#define CLOCK_ID_TER_PCM_EBIT 0x205

/** Clock ID of the quaternary PCM IBIT. */

#define CLOCK_ID_QUAD_PCM_IBIT 0x206

/** Clock ID of the quaternary PCM EBIT. */

#define CLOCK_ID_QUAD_PCM_EBIT 0x207

/** Clock ID of the quinary PCM IBIT. */

#define CLOCK_ID_QUI_PCM_IBIT 0x208

/** Clock ID of the quinary PCM EBIT. */

#define CLOCK_ID_QUI_PCM_EBIT 0x209

/** Clock ID of the quinary PCM OSR. */

#define CLOCK_ID_QUI_PCM_OSR 0x20A

/** Clock ID of the senary PCM IBIT. */

#define CLOCK_ID_SEN_PCM_IBIT 0x20B

/** Clock ID of the senary PCM EBIT. */

#define CLOCK_ID_SEN_PCM_EBIT 0x20C

/** Clock ID for the primary TDM IBIT. */

#define CLOCK_ID_PRI_TDM_IBIT 0x200

/** Clock ID for the primary TDM EBIT. */

#define CLOCK_ID_PRI_TDM_EBIT 0x201

/** Clock ID for the secondary TDM IBIT. */

#define CLOCK_ID_SEC_TDM_IBIT 0x202

/** Clock ID for the secondary TDM EBIT. */

#define CLOCK_ID_SEC_TDM_EBIT 0x203

/** Clock ID for the tertiary TDM IBIT. */

#define CLOCK_ID_TER_TDM_IBIT 0x204

/** Clock ID for the tertiary TDM EBIT. */

#define CLOCK_ID_TER_TDM_EBIT 0x205

/** Clock ID for the quaternary TDM IBIT. */

#define CLOCK_ID_QUAD_TDM_IBIT 0x206

/** Clock ID for the quaternary TDM EBIT. */

#define CLOCK_ID_QUAD_TDM_EBIT 0x207

/** Clock ID for the quinary TDM IBIT. */

#define CLOCK_ID_QUI_TDM_IBIT 0x208

/** Clock ID for the quinary TDM EBIT. */

#define CLOCK_ID_QUI_TDM_EBIT 0x209

/** Clock ID for the quinary TDM OSR. */

#define CLOCK_ID_QUI_TDM_OSR 0x20A

/** Clock ID for the senary TDM IBIT. */

#define CLOCK_ID_SEN_TDM_IBIT 0x20B

/** Clock ID for the senary TDM EBIT. */

#define CLOCK_ID_SEN_TDM_EBIT 0x20C

/** Clock ID of the septenary TDM IBIT. */
#define CLOCK_ID_SEP_TDM_IBIT 0x20D

/** Clock ID of the septenary TDM EBIT. */
#define CLOCK_ID_SEP_TDM_EBIT 0x20E

/** Clock ID for MCLK 1. */

#define CLOCK_ID_MCLK_1 0x300

/** Clock ID for MCLK 2. */

#define CLOCK_ID_MCLK_2 0x301

/** Clock ID for MCLK 3. */

#define CLOCK_ID_MCLK_3 0x302

/** Clock ID for MCLK 4. */

#define CLOCK_ID_MCLK_4 0x303

/** Clock ID for MCLK 5. */

#define CLOCK_ID_MCLK_5 0x304

/** Clock ID for MCLK for WSA core */

#define CLOCK_ID_WSA_CORE_MCLK 0x305

/** Clock ID for NPL MCLK for WSA core */

#define CLOCK_ID_WSA_CORE_NPL_MCLK 0x306

/** Clock ID for 2X WSA MCLK (2X MCLK or NPL MCLK, both are same) */

#define CLOCK_ID_WSA_CORE_2X_MCLK CLOCK_ID_WSA_CORE_NPL_MCLK

/** Clock ID for MCLK for VA core */

#define CLOCK_ID_VA_CORE_MCLK 0x307

/** Clock ID for the primary SPDIF output core. */

#define CLOCK_ID_PRI_SPDIF_OUTPUT_CORE 0x400

/** Clock ID for the secondary SPDIF output core. */

#define CLOCK_ID_SEC_SPDIF_OUTPUT_CORE 0x401

/** Clock ID for the primary SPDIF input core. */

#define CLOCK_ID_PRI_SPDIF_INPUT_CORE 0x402

/** Clock ID for the secondary SPDIF input core. */

#define CLOCK_ID_SEC_SPDIF_INPUT_CORE 0x403

/** Clock ID for the secondary SPDIF output NPL clk. */

#define CLOCK_ID_PRI_SPDIF_OUTPUT_NPL 0x404

/** Clock ID for the primary SPDIF output NPL clk. */

#define CLOCK_ID_SEC_SPDIF_OUTPUT_NPL 0x405

/** Clock attribute is invalid (reserved for internal use). */
#define CLOCK_ATTRIBUTE_INVALID 0x0

/** Clock attribute for no coupled clocks. */
#define CLOCK_ATTRIBUTE_COUPLE_NO 0x1

/** Clock attribute for the dividend of the coupled clocks. */
#define CLOCK_ATTRIBUTE_COUPLE_DIVIDEND 0x2

/** Clock attribute for the divisor of the coupled clocks. */
#define CLOCK_ATTRIBUTE_COUPLE_DIVISOR 0x3

/** Clock attribute for the invert-and-no-couple case. */
#define CLOCK_ATTRIBUTE_INVERT_COUPLE_NO 0x4

/** Default root clock source. */
#define CLOCK_ROOT_DEFAULT 0x0

/** Hardware core identifier for LPASS. */
#define HW_CORE_ID_LPASS 0x1

/** Hardware core identifier for digital codec. */
#define HW_CORE_ID_DCODEC 0x2

int audio_prm_set_lpass_clk_cfg(struct clk_cfg *cfg, uint8_t enable);
int audio_prm_set_lpass_hw_core_req(struct clk_cfg *cfg, uint32_t hw_core_id, uint8_t enable);
int audio_prm_set_cdc_earpa_duty_cycling_req(struct prm_earpa_hw_intf_config *earpa_config,
									uint32_t enable);

#endif
