// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2012-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <dsp/audio_prm.h>
#include <dsp/apr_audio-v2.h>
#include <dsp/sp_params.h>
#include "msm-dai-q6-v2.h"
#include <asoc/core.h>


#define AFE_CLK_VERSION_V1    1

#define MSM_DAI_PRI_AUXPCM_DT_DEV_ID 1
#define MSM_DAI_SEC_AUXPCM_DT_DEV_ID 2
#define MSM_DAI_TERT_AUXPCM_DT_DEV_ID 3
#define MSM_DAI_QUAT_AUXPCM_DT_DEV_ID 4
#define MSM_DAI_QUIN_AUXPCM_DT_DEV_ID 5
#define MSM_DAI_SEN_AUXPCM_DT_DEV_ID 6

#define MSM_DAI_TWS_CHANNEL_MODE_ONE 1
#define MSM_DAI_TWS_CHANNEL_MODE_TWO 2

#define spdif_clock_value(rate) (2*rate*32*2)
#define CHANNEL_STATUS_SIZE 24
#define CHANNEL_STATUS_MASK_INIT 0x0
#define CHANNEL_STATUS_MASK 0x4
#define PREEMPH_MASK 0x38
#define PREEMPH_SHIFT 3
#define GET_PREEMPH(b) ((b & PREEMPH_MASK) >> PREEMPH_SHIFT)
#define AFE_API_VERSION_CLOCK_SET 1
#define MSM_DAI_SYSFS_ENTRY_MAX_LEN 64

#define DAI_FORMATS_S16_S24_S32_LE (SNDRV_PCM_FMTBIT_S16_LE | \
				    SNDRV_PCM_FMTBIT_S24_LE | \
				    SNDRV_PCM_FMTBIT_S32_LE)

//static int msm_mi2s_get_port_id(u32 mi2s_id, int stream, u16 *port_id);

enum {
	ENC_FMT_NONE,
	DEC_FMT_NONE = ENC_FMT_NONE,
	ENC_FMT_SBC = ASM_MEDIA_FMT_SBC,
	DEC_FMT_SBC = ASM_MEDIA_FMT_SBC,
	ENC_FMT_AAC_V2 = ASM_MEDIA_FMT_AAC_V2,
	DEC_FMT_AAC_V2 = ASM_MEDIA_FMT_AAC_V2,
	ENC_FMT_APTX = ASM_MEDIA_FMT_APTX,
	ENC_FMT_APTX_HD = ASM_MEDIA_FMT_APTX_HD,
	ENC_FMT_CELT = ASM_MEDIA_FMT_CELT,
	ENC_FMT_LDAC = ASM_MEDIA_FMT_LDAC,
	ENC_FMT_APTX_ADAPTIVE = ASM_MEDIA_FMT_APTX_ADAPTIVE,
	DEC_FMT_APTX_ADAPTIVE = ASM_MEDIA_FMT_APTX_ADAPTIVE,
	DEC_FMT_MP3 = ASM_MEDIA_FMT_MP3,
	ENC_FMT_APTX_AD_SPEECH = ASM_MEDIA_FMT_APTX_AD_SPEECH,
	DEC_FMT_APTX_AD_SPEECH = ASM_MEDIA_FMT_APTX_AD_SPEECH,
};

enum {
	SPKR_1,
	SPKR_2,
};

static const struct afe_clk_set lpass_clk_set_default = {
	AFE_API_VERSION_CLOCK_SET,
	CLOCK_ID_PRI_PCM_IBIT,
	OSR_CLOCK_2_P048_MHZ,
	CLOCK_ATTRIBUTE_COUPLE_NO,
	CLOCK_ROOT_DEFAULT,
	0,
};

static const struct afe_clk_cfg lpass_clk_cfg_default = {
	AFE_API_VERSION_I2S_CONFIG,
	OSR_CLOCK_2_P048_MHZ,
	0,
	Q6AFE_LPASS_CLK_SRC_INTERNAL,
	CLOCK_ROOT_DEFAULT,
	Q6AFE_LPASS_MODE_CLK1_VALID,
	0,
};
enum {
	STATUS_PORT_STARTED, /* track if AFE port has started */
	/* track AFE Tx port status for bi-directional transfers */
	STATUS_TX_PORT,
	/* track AFE Rx port status for bi-directional transfers */
	STATUS_RX_PORT,
	STATUS_MAX
};

enum {
	RATE_8KHZ,
	RATE_16KHZ,
	RATE_MAX_NUM_OF_AUX_PCM_RATES,
};

enum {
	IDX_PRIMARY_TDM_RX_0,
	IDX_PRIMARY_TDM_RX_1,
	IDX_PRIMARY_TDM_RX_2,
	IDX_PRIMARY_TDM_RX_3,
	IDX_PRIMARY_TDM_RX_4,
	IDX_PRIMARY_TDM_RX_5,
	IDX_PRIMARY_TDM_RX_6,
	IDX_PRIMARY_TDM_RX_7,
	IDX_PRIMARY_TDM_TX_0,
	IDX_PRIMARY_TDM_TX_1,
	IDX_PRIMARY_TDM_TX_2,
	IDX_PRIMARY_TDM_TX_3,
	IDX_PRIMARY_TDM_TX_4,
	IDX_PRIMARY_TDM_TX_5,
	IDX_PRIMARY_TDM_TX_6,
	IDX_PRIMARY_TDM_TX_7,
	IDX_SECONDARY_TDM_RX_0,
	IDX_SECONDARY_TDM_RX_1,
	IDX_SECONDARY_TDM_RX_2,
	IDX_SECONDARY_TDM_RX_3,
	IDX_SECONDARY_TDM_RX_4,
	IDX_SECONDARY_TDM_RX_5,
	IDX_SECONDARY_TDM_RX_6,
	IDX_SECONDARY_TDM_RX_7,
	IDX_SECONDARY_TDM_TX_0,
	IDX_SECONDARY_TDM_TX_1,
	IDX_SECONDARY_TDM_TX_2,
	IDX_SECONDARY_TDM_TX_3,
	IDX_SECONDARY_TDM_TX_4,
	IDX_SECONDARY_TDM_TX_5,
	IDX_SECONDARY_TDM_TX_6,
	IDX_SECONDARY_TDM_TX_7,
	IDX_TERTIARY_TDM_RX_0,
	IDX_TERTIARY_TDM_RX_1,
	IDX_TERTIARY_TDM_RX_2,
	IDX_TERTIARY_TDM_RX_3,
	IDX_TERTIARY_TDM_RX_4,
	IDX_TERTIARY_TDM_RX_5,
	IDX_TERTIARY_TDM_RX_6,
	IDX_TERTIARY_TDM_RX_7,
	IDX_TERTIARY_TDM_TX_0,
	IDX_TERTIARY_TDM_TX_1,
	IDX_TERTIARY_TDM_TX_2,
	IDX_TERTIARY_TDM_TX_3,
	IDX_TERTIARY_TDM_TX_4,
	IDX_TERTIARY_TDM_TX_5,
	IDX_TERTIARY_TDM_TX_6,
	IDX_TERTIARY_TDM_TX_7,
	IDX_QUATERNARY_TDM_RX_0,
	IDX_QUATERNARY_TDM_RX_1,
	IDX_QUATERNARY_TDM_RX_2,
	IDX_QUATERNARY_TDM_RX_3,
	IDX_QUATERNARY_TDM_RX_4,
	IDX_QUATERNARY_TDM_RX_5,
	IDX_QUATERNARY_TDM_RX_6,
	IDX_QUATERNARY_TDM_RX_7,
	IDX_QUATERNARY_TDM_TX_0,
	IDX_QUATERNARY_TDM_TX_1,
	IDX_QUATERNARY_TDM_TX_2,
	IDX_QUATERNARY_TDM_TX_3,
	IDX_QUATERNARY_TDM_TX_4,
	IDX_QUATERNARY_TDM_TX_5,
	IDX_QUATERNARY_TDM_TX_6,
	IDX_QUATERNARY_TDM_TX_7,
	IDX_QUINARY_TDM_RX_0,
	IDX_QUINARY_TDM_RX_1,
	IDX_QUINARY_TDM_RX_2,
	IDX_QUINARY_TDM_RX_3,
	IDX_QUINARY_TDM_RX_4,
	IDX_QUINARY_TDM_RX_5,
	IDX_QUINARY_TDM_RX_6,
	IDX_QUINARY_TDM_RX_7,
	IDX_QUINARY_TDM_TX_0,
	IDX_QUINARY_TDM_TX_1,
	IDX_QUINARY_TDM_TX_2,
	IDX_QUINARY_TDM_TX_3,
	IDX_QUINARY_TDM_TX_4,
	IDX_QUINARY_TDM_TX_5,
	IDX_QUINARY_TDM_TX_6,
	IDX_QUINARY_TDM_TX_7,
	IDX_SENARY_TDM_RX_0,
	IDX_SENARY_TDM_RX_1,
	IDX_SENARY_TDM_RX_2,
	IDX_SENARY_TDM_RX_3,
	IDX_SENARY_TDM_RX_4,
	IDX_SENARY_TDM_RX_5,
	IDX_SENARY_TDM_RX_6,
	IDX_SENARY_TDM_RX_7,
	IDX_SENARY_TDM_TX_0,
	IDX_SENARY_TDM_TX_1,
	IDX_SENARY_TDM_TX_2,
	IDX_SENARY_TDM_TX_3,
	IDX_SENARY_TDM_TX_4,
	IDX_SENARY_TDM_TX_5,
	IDX_SENARY_TDM_TX_6,
	IDX_SENARY_TDM_TX_7,
	IDX_TDM_MAX,
};

enum {
	IDX_GROUP_PRIMARY_TDM_RX,
	IDX_GROUP_PRIMARY_TDM_TX,
	IDX_GROUP_SECONDARY_TDM_RX,
	IDX_GROUP_SECONDARY_TDM_TX,
	IDX_GROUP_TERTIARY_TDM_RX,
	IDX_GROUP_TERTIARY_TDM_TX,
	IDX_GROUP_QUATERNARY_TDM_RX,
	IDX_GROUP_QUATERNARY_TDM_TX,
	IDX_GROUP_QUINARY_TDM_RX,
	IDX_GROUP_QUINARY_TDM_TX,
	IDX_GROUP_SENARY_TDM_RX,
	IDX_GROUP_SENARY_TDM_TX,
	IDX_GROUP_TDM_MAX,
};

struct msm_dai_q6_dai_data {
	DECLARE_BITMAP(status_mask, STATUS_MAX);
	DECLARE_BITMAP(hwfree_status, STATUS_MAX);
	u32 rate;
	u32 channels;
	u32 bitwidth;
	u32 cal_mode;
	u32 afe_rx_in_channels;
	u16 afe_rx_in_bitformat;
	u32 afe_tx_out_channels;
	u16 afe_tx_out_bitformat;
	struct afe_enc_config enc_config;
	struct afe_dec_config dec_config;
	union afe_port_config port_config;
	u16 vi_feed_mono;
};

struct msm_dai_q6_mi2s_dai_config {
	u16 pdata_mi2s_lines;
	struct msm_dai_q6_dai_data mi2s_dai_data;
};

struct msm_dai_q6_mi2s_dai_data {
	u32 is_island_dai;
	struct msm_dai_q6_mi2s_dai_config tx_dai;
	struct msm_dai_q6_mi2s_dai_config rx_dai;
};

struct msm_dai_q6_meta_mi2s_dai_data {
	DECLARE_BITMAP(status_mask, STATUS_MAX);
	u16 num_member_ports;
	u16 member_port_id[MAX_NUM_I2S_META_PORT_MEMBER_PORTS];
	u16 channel_mode[MAX_NUM_I2S_META_PORT_MEMBER_PORTS];
	u32 rate;
	u32 channels;
	u32 bitwidth;
	union afe_port_config port_config;
};

struct msm_dai_q6_cdc_dma_dai_data {
	DECLARE_BITMAP(status_mask, STATUS_MAX);
	DECLARE_BITMAP(hwfree_status, STATUS_MAX);
	u32 rate;
	u32 channels;
	u32 bitwidth;
	u32 is_island_dai;
	union afe_port_config port_config;
	u32 cdc_dma_data_align;
};

struct msm_dai_q6_auxpcm_dai_data {
	/* BITMAP to track Rx and Tx port usage count */
	DECLARE_BITMAP(auxpcm_port_status, STATUS_MAX);
	struct mutex rlock; /* auxpcm dev resource lock */
	u16 rx_pid; /* AUXPCM RX AFE port ID */
	u16 tx_pid; /* AUXPCM TX AFE port ID */
	u16 clk_ver;
	u32 is_island_dai;
	struct afe_clk_set clk_set; /* hold LPASS clock configuration */
	struct clk_cfg aux_clk_set; /* hold LPASS clock config. */
	struct msm_dai_q6_dai_data bdai_data; /* incoporate base DAI data */
};

struct msm_dai_q6_tdm_dai_data {
	DECLARE_BITMAP(status_mask, STATUS_MAX);
	u32 rate;
	u32 channels;
	u32 bitwidth;
	u32 num_group_ports;
	u32 is_island_dai;
	struct clk_cfg clk_set; /* hold LPASS clock config. */
	union afe_port_group_config group_cfg; /* hold tdm group config */
	struct afe_tdm_port_config port_cfg; /* hold tdm config */
	struct afe_param_id_tdm_lane_cfg lane_cfg; /* hold tdm lane config */
};

static DEFINE_MUTEX(tdm_mutex);

static atomic_t tdm_group_ref[IDX_GROUP_TDM_MAX];

static struct afe_param_id_tdm_lane_cfg tdm_lane_cfg = {
	AFE_GROUP_DEVICE_ID_QUINARY_TDM_RX,
	0x0,
};

/* cache of group cfg per parent node */
static struct afe_param_id_group_device_tdm_cfg tdm_group_cfg = {
	AFE_API_VERSION_GROUP_DEVICE_TDM_CONFIG,
	AFE_GROUP_DEVICE_ID_QUATERNARY_TDM_RX,
	0,
	{AFE_PORT_ID_QUATERNARY_TDM_RX,
	AFE_PORT_ID_QUATERNARY_TDM_RX_1,
	AFE_PORT_ID_QUATERNARY_TDM_RX_2,
	AFE_PORT_ID_QUATERNARY_TDM_RX_3,
	AFE_PORT_ID_QUATERNARY_TDM_RX_4,
	AFE_PORT_ID_QUATERNARY_TDM_RX_5,
	AFE_PORT_ID_QUATERNARY_TDM_RX_6,
	AFE_PORT_ID_QUATERNARY_TDM_RX_7},
	8,
	48000,
	32,
	8,
	32,
	0xFF,
};

static u32 num_tdm_group_ports;

static struct clk_cfg tdm_clk_set = {
	CLOCK_ID_QUAD_TDM_EBIT,
	IBIT_CLOCK_DISABLE,
	CLOCK_ATTRIBUTE_INVERT_COUPLE_NO,
	CLOCK_ROOT_DEFAULT,
};

static int msm_dai_q6_get_tdm_clk_ref(u16 id)
{
	switch (id) {
	case IDX_GROUP_PRIMARY_TDM_RX:
	case IDX_GROUP_PRIMARY_TDM_TX:
		return atomic_read(&tdm_group_ref[IDX_GROUP_PRIMARY_TDM_RX]) +
			atomic_read(&tdm_group_ref[IDX_GROUP_PRIMARY_TDM_TX]);
	case IDX_GROUP_SECONDARY_TDM_RX:
	case IDX_GROUP_SECONDARY_TDM_TX:
		return atomic_read(&tdm_group_ref[IDX_GROUP_SECONDARY_TDM_RX]) +
			atomic_read(&tdm_group_ref[IDX_GROUP_SECONDARY_TDM_TX]);
	case IDX_GROUP_TERTIARY_TDM_RX:
	case IDX_GROUP_TERTIARY_TDM_TX:
		return atomic_read(&tdm_group_ref[IDX_GROUP_TERTIARY_TDM_RX]) +
			atomic_read(&tdm_group_ref[IDX_GROUP_TERTIARY_TDM_TX]);
	case IDX_GROUP_QUATERNARY_TDM_RX:
	case IDX_GROUP_QUATERNARY_TDM_TX:
		return atomic_read(&tdm_group_ref[IDX_GROUP_QUATERNARY_TDM_RX]) +
			atomic_read(&tdm_group_ref[IDX_GROUP_QUATERNARY_TDM_TX]);
	case IDX_GROUP_QUINARY_TDM_RX:
	case IDX_GROUP_QUINARY_TDM_TX:
		return atomic_read(&tdm_group_ref[IDX_GROUP_QUINARY_TDM_RX]) +
			atomic_read(&tdm_group_ref[IDX_GROUP_QUINARY_TDM_TX]);
	case IDX_GROUP_SENARY_TDM_RX:
	case IDX_GROUP_SENARY_TDM_TX:
		return atomic_read(&tdm_group_ref[IDX_GROUP_SENARY_TDM_RX]) +
			atomic_read(&tdm_group_ref[IDX_GROUP_SENARY_TDM_TX]);
	default: return -EINVAL;
	}
}

int msm_dai_q6_get_group_idx(u16 id)
{
	switch (id) {
	case AFE_GROUP_DEVICE_ID_PRIMARY_TDM_RX:
	case AFE_PORT_ID_PRIMARY_TDM_RX:
	case AFE_PORT_ID_PRIMARY_TDM_RX_1:
	case AFE_PORT_ID_PRIMARY_TDM_RX_2:
	case AFE_PORT_ID_PRIMARY_TDM_RX_3:
	case AFE_PORT_ID_PRIMARY_TDM_RX_4:
	case AFE_PORT_ID_PRIMARY_TDM_RX_5:
	case AFE_PORT_ID_PRIMARY_TDM_RX_6:
	case AFE_PORT_ID_PRIMARY_TDM_RX_7:
		return IDX_GROUP_PRIMARY_TDM_RX;
	case AFE_GROUP_DEVICE_ID_PRIMARY_TDM_TX:
	case AFE_PORT_ID_PRIMARY_TDM_TX:
	case AFE_PORT_ID_PRIMARY_TDM_TX_1:
	case AFE_PORT_ID_PRIMARY_TDM_TX_2:
	case AFE_PORT_ID_PRIMARY_TDM_TX_3:
	case AFE_PORT_ID_PRIMARY_TDM_TX_4:
	case AFE_PORT_ID_PRIMARY_TDM_TX_5:
	case AFE_PORT_ID_PRIMARY_TDM_TX_6:
	case AFE_PORT_ID_PRIMARY_TDM_TX_7:
		return IDX_GROUP_PRIMARY_TDM_TX;
	case AFE_GROUP_DEVICE_ID_SECONDARY_TDM_RX:
	case AFE_PORT_ID_SECONDARY_TDM_RX:
	case AFE_PORT_ID_SECONDARY_TDM_RX_1:
	case AFE_PORT_ID_SECONDARY_TDM_RX_2:
	case AFE_PORT_ID_SECONDARY_TDM_RX_3:
	case AFE_PORT_ID_SECONDARY_TDM_RX_4:
	case AFE_PORT_ID_SECONDARY_TDM_RX_5:
	case AFE_PORT_ID_SECONDARY_TDM_RX_6:
	case AFE_PORT_ID_SECONDARY_TDM_RX_7:
		return IDX_GROUP_SECONDARY_TDM_RX;
	case AFE_GROUP_DEVICE_ID_SECONDARY_TDM_TX:
	case AFE_PORT_ID_SECONDARY_TDM_TX:
	case AFE_PORT_ID_SECONDARY_TDM_TX_1:
	case AFE_PORT_ID_SECONDARY_TDM_TX_2:
	case AFE_PORT_ID_SECONDARY_TDM_TX_3:
	case AFE_PORT_ID_SECONDARY_TDM_TX_4:
	case AFE_PORT_ID_SECONDARY_TDM_TX_5:
	case AFE_PORT_ID_SECONDARY_TDM_TX_6:
	case AFE_PORT_ID_SECONDARY_TDM_TX_7:
		return IDX_GROUP_SECONDARY_TDM_TX;
	case AFE_GROUP_DEVICE_ID_TERTIARY_TDM_RX:
	case AFE_PORT_ID_TERTIARY_TDM_RX:
	case AFE_PORT_ID_TERTIARY_TDM_RX_1:
	case AFE_PORT_ID_TERTIARY_TDM_RX_2:
	case AFE_PORT_ID_TERTIARY_TDM_RX_3:
	case AFE_PORT_ID_TERTIARY_TDM_RX_4:
	case AFE_PORT_ID_TERTIARY_TDM_RX_5:
	case AFE_PORT_ID_TERTIARY_TDM_RX_6:
	case AFE_PORT_ID_TERTIARY_TDM_RX_7:
		return IDX_GROUP_TERTIARY_TDM_RX;
	case AFE_GROUP_DEVICE_ID_TERTIARY_TDM_TX:
	case AFE_PORT_ID_TERTIARY_TDM_TX:
	case AFE_PORT_ID_TERTIARY_TDM_TX_1:
	case AFE_PORT_ID_TERTIARY_TDM_TX_2:
	case AFE_PORT_ID_TERTIARY_TDM_TX_3:
	case AFE_PORT_ID_TERTIARY_TDM_TX_4:
	case AFE_PORT_ID_TERTIARY_TDM_TX_5:
	case AFE_PORT_ID_TERTIARY_TDM_TX_6:
	case AFE_PORT_ID_TERTIARY_TDM_TX_7:
		return IDX_GROUP_TERTIARY_TDM_TX;
	case AFE_GROUP_DEVICE_ID_QUATERNARY_TDM_RX:
	case AFE_PORT_ID_QUATERNARY_TDM_RX:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_1:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_2:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_3:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_4:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_5:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_6:
	case AFE_PORT_ID_QUATERNARY_TDM_RX_7:
		return IDX_GROUP_QUATERNARY_TDM_RX;
	case AFE_GROUP_DEVICE_ID_QUATERNARY_TDM_TX:
	case AFE_PORT_ID_QUATERNARY_TDM_TX:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_1:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_2:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_3:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_4:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_5:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_6:
	case AFE_PORT_ID_QUATERNARY_TDM_TX_7:
		return IDX_GROUP_QUATERNARY_TDM_TX;
	case AFE_GROUP_DEVICE_ID_QUINARY_TDM_RX:
	case AFE_PORT_ID_QUINARY_TDM_RX:
	case AFE_PORT_ID_QUINARY_TDM_RX_1:
	case AFE_PORT_ID_QUINARY_TDM_RX_2:
	case AFE_PORT_ID_QUINARY_TDM_RX_3:
	case AFE_PORT_ID_QUINARY_TDM_RX_4:
	case AFE_PORT_ID_QUINARY_TDM_RX_5:
	case AFE_PORT_ID_QUINARY_TDM_RX_6:
	case AFE_PORT_ID_QUINARY_TDM_RX_7:
		return IDX_GROUP_QUINARY_TDM_RX;
	case AFE_GROUP_DEVICE_ID_QUINARY_TDM_TX:
	case AFE_PORT_ID_QUINARY_TDM_TX:
	case AFE_PORT_ID_QUINARY_TDM_TX_1:
	case AFE_PORT_ID_QUINARY_TDM_TX_2:
	case AFE_PORT_ID_QUINARY_TDM_TX_3:
	case AFE_PORT_ID_QUINARY_TDM_TX_4:
	case AFE_PORT_ID_QUINARY_TDM_TX_5:
	case AFE_PORT_ID_QUINARY_TDM_TX_6:
	case AFE_PORT_ID_QUINARY_TDM_TX_7:
		return IDX_GROUP_QUINARY_TDM_TX;
	case AFE_GROUP_DEVICE_ID_SENARY_TDM_RX:
	case AFE_PORT_ID_SENARY_TDM_RX:
	case AFE_PORT_ID_SENARY_TDM_RX_1:
	case AFE_PORT_ID_SENARY_TDM_RX_2:
	case AFE_PORT_ID_SENARY_TDM_RX_3:
	case AFE_PORT_ID_SENARY_TDM_RX_4:
	case AFE_PORT_ID_SENARY_TDM_RX_5:
	case AFE_PORT_ID_SENARY_TDM_RX_6:
	case AFE_PORT_ID_SENARY_TDM_RX_7:
		return IDX_GROUP_SENARY_TDM_RX;
	case AFE_GROUP_DEVICE_ID_SENARY_TDM_TX:
	case AFE_PORT_ID_SENARY_TDM_TX:
	case AFE_PORT_ID_SENARY_TDM_TX_1:
	case AFE_PORT_ID_SENARY_TDM_TX_2:
	case AFE_PORT_ID_SENARY_TDM_TX_3:
	case AFE_PORT_ID_SENARY_TDM_TX_4:
	case AFE_PORT_ID_SENARY_TDM_TX_5:
	case AFE_PORT_ID_SENARY_TDM_TX_6:
	case AFE_PORT_ID_SENARY_TDM_TX_7:
		return IDX_GROUP_SENARY_TDM_TX;
	default: return -EINVAL;
	}
}

int msm_dai_q6_get_port_idx(u16 id)
{
	switch (id) {
	case AFE_PORT_ID_PRIMARY_TDM_RX:
		return IDX_PRIMARY_TDM_RX_0;
	case AFE_PORT_ID_PRIMARY_TDM_TX:
		return IDX_PRIMARY_TDM_TX_0;
	case AFE_PORT_ID_PRIMARY_TDM_RX_1:
		return IDX_PRIMARY_TDM_RX_1;
	case AFE_PORT_ID_PRIMARY_TDM_TX_1:
		return IDX_PRIMARY_TDM_TX_1;
	case AFE_PORT_ID_PRIMARY_TDM_RX_2:
		return IDX_PRIMARY_TDM_RX_2;
	case AFE_PORT_ID_PRIMARY_TDM_TX_2:
		return IDX_PRIMARY_TDM_TX_2;
	case AFE_PORT_ID_PRIMARY_TDM_RX_3:
		return IDX_PRIMARY_TDM_RX_3;
	case AFE_PORT_ID_PRIMARY_TDM_TX_3:
		return IDX_PRIMARY_TDM_TX_3;
	case AFE_PORT_ID_PRIMARY_TDM_RX_4:
		return IDX_PRIMARY_TDM_RX_4;
	case AFE_PORT_ID_PRIMARY_TDM_TX_4:
		return IDX_PRIMARY_TDM_TX_4;
	case AFE_PORT_ID_PRIMARY_TDM_RX_5:
		return IDX_PRIMARY_TDM_RX_5;
	case AFE_PORT_ID_PRIMARY_TDM_TX_5:
		return IDX_PRIMARY_TDM_TX_5;
	case AFE_PORT_ID_PRIMARY_TDM_RX_6:
		return IDX_PRIMARY_TDM_RX_6;
	case AFE_PORT_ID_PRIMARY_TDM_TX_6:
		return IDX_PRIMARY_TDM_TX_6;
	case AFE_PORT_ID_PRIMARY_TDM_RX_7:
		return IDX_PRIMARY_TDM_RX_7;
	case AFE_PORT_ID_PRIMARY_TDM_TX_7:
		return IDX_PRIMARY_TDM_TX_7;
	case AFE_PORT_ID_SECONDARY_TDM_RX:
		return IDX_SECONDARY_TDM_RX_0;
	case AFE_PORT_ID_SECONDARY_TDM_TX:
		return IDX_SECONDARY_TDM_TX_0;
	case AFE_PORT_ID_SECONDARY_TDM_RX_1:
		return IDX_SECONDARY_TDM_RX_1;
	case AFE_PORT_ID_SECONDARY_TDM_TX_1:
		return IDX_SECONDARY_TDM_TX_1;
	case AFE_PORT_ID_SECONDARY_TDM_RX_2:
		return IDX_SECONDARY_TDM_RX_2;
	case AFE_PORT_ID_SECONDARY_TDM_TX_2:
		return IDX_SECONDARY_TDM_TX_2;
	case AFE_PORT_ID_SECONDARY_TDM_RX_3:
		return IDX_SECONDARY_TDM_RX_3;
	case AFE_PORT_ID_SECONDARY_TDM_TX_3:
		return IDX_SECONDARY_TDM_TX_3;
	case AFE_PORT_ID_SECONDARY_TDM_RX_4:
		return IDX_SECONDARY_TDM_RX_4;
	case AFE_PORT_ID_SECONDARY_TDM_TX_4:
		return IDX_SECONDARY_TDM_TX_4;
	case AFE_PORT_ID_SECONDARY_TDM_RX_5:
		return IDX_SECONDARY_TDM_RX_5;
	case AFE_PORT_ID_SECONDARY_TDM_TX_5:
		return IDX_SECONDARY_TDM_TX_5;
	case AFE_PORT_ID_SECONDARY_TDM_RX_6:
		return IDX_SECONDARY_TDM_RX_6;
	case AFE_PORT_ID_SECONDARY_TDM_TX_6:
		return IDX_SECONDARY_TDM_TX_6;
	case AFE_PORT_ID_SECONDARY_TDM_RX_7:
		return IDX_SECONDARY_TDM_RX_7;
	case AFE_PORT_ID_SECONDARY_TDM_TX_7:
		return IDX_SECONDARY_TDM_TX_7;
	case AFE_PORT_ID_TERTIARY_TDM_RX:
		return IDX_TERTIARY_TDM_RX_0;
	case AFE_PORT_ID_TERTIARY_TDM_TX:
		return IDX_TERTIARY_TDM_TX_0;
	case AFE_PORT_ID_TERTIARY_TDM_RX_1:
		return IDX_TERTIARY_TDM_RX_1;
	case AFE_PORT_ID_TERTIARY_TDM_TX_1:
		return IDX_TERTIARY_TDM_TX_1;
	case AFE_PORT_ID_TERTIARY_TDM_RX_2:
		return IDX_TERTIARY_TDM_RX_2;
	case AFE_PORT_ID_TERTIARY_TDM_TX_2:
		return IDX_TERTIARY_TDM_TX_2;
	case AFE_PORT_ID_TERTIARY_TDM_RX_3:
		return IDX_TERTIARY_TDM_RX_3;
	case AFE_PORT_ID_TERTIARY_TDM_TX_3:
		return IDX_TERTIARY_TDM_TX_3;
	case AFE_PORT_ID_TERTIARY_TDM_RX_4:
		return IDX_TERTIARY_TDM_RX_4;
	case AFE_PORT_ID_TERTIARY_TDM_TX_4:
		return IDX_TERTIARY_TDM_TX_4;
	case AFE_PORT_ID_TERTIARY_TDM_RX_5:
		return IDX_TERTIARY_TDM_RX_5;
	case AFE_PORT_ID_TERTIARY_TDM_TX_5:
		return IDX_TERTIARY_TDM_TX_5;
	case AFE_PORT_ID_TERTIARY_TDM_RX_6:
		return IDX_TERTIARY_TDM_RX_6;
	case AFE_PORT_ID_TERTIARY_TDM_TX_6:
		return IDX_TERTIARY_TDM_TX_6;
	case AFE_PORT_ID_TERTIARY_TDM_RX_7:
		return IDX_TERTIARY_TDM_RX_7;
	case AFE_PORT_ID_TERTIARY_TDM_TX_7:
		return IDX_TERTIARY_TDM_TX_7;
	case AFE_PORT_ID_QUATERNARY_TDM_RX:
		return IDX_QUATERNARY_TDM_RX_0;
	case AFE_PORT_ID_QUATERNARY_TDM_TX:
		return IDX_QUATERNARY_TDM_TX_0;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_1:
		return IDX_QUATERNARY_TDM_RX_1;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_1:
		return IDX_QUATERNARY_TDM_TX_1;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_2:
		return IDX_QUATERNARY_TDM_RX_2;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_2:
		return IDX_QUATERNARY_TDM_TX_2;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_3:
		return IDX_QUATERNARY_TDM_RX_3;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_3:
		return IDX_QUATERNARY_TDM_TX_3;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_4:
		return IDX_QUATERNARY_TDM_RX_4;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_4:
		return IDX_QUATERNARY_TDM_TX_4;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_5:
		return IDX_QUATERNARY_TDM_RX_5;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_5:
		return IDX_QUATERNARY_TDM_TX_5;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_6:
		return IDX_QUATERNARY_TDM_RX_6;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_6:
		return IDX_QUATERNARY_TDM_TX_6;
	case AFE_PORT_ID_QUATERNARY_TDM_RX_7:
		return IDX_QUATERNARY_TDM_RX_7;
	case AFE_PORT_ID_QUATERNARY_TDM_TX_7:
		return IDX_QUATERNARY_TDM_TX_7;
	case AFE_PORT_ID_QUINARY_TDM_RX:
		return IDX_QUINARY_TDM_RX_0;
	case AFE_PORT_ID_QUINARY_TDM_TX:
		return IDX_QUINARY_TDM_TX_0;
	case AFE_PORT_ID_QUINARY_TDM_RX_1:
		return IDX_QUINARY_TDM_RX_1;
	case AFE_PORT_ID_QUINARY_TDM_TX_1:
		return IDX_QUINARY_TDM_TX_1;
	case AFE_PORT_ID_QUINARY_TDM_RX_2:
		return IDX_QUINARY_TDM_RX_2;
	case AFE_PORT_ID_QUINARY_TDM_TX_2:
		return IDX_QUINARY_TDM_TX_2;
	case AFE_PORT_ID_QUINARY_TDM_RX_3:
		return IDX_QUINARY_TDM_RX_3;
	case AFE_PORT_ID_QUINARY_TDM_TX_3:
		return IDX_QUINARY_TDM_TX_3;
	case AFE_PORT_ID_QUINARY_TDM_RX_4:
		return IDX_QUINARY_TDM_RX_4;
	case AFE_PORT_ID_QUINARY_TDM_TX_4:
		return IDX_QUINARY_TDM_TX_4;
	case AFE_PORT_ID_QUINARY_TDM_RX_5:
		return IDX_QUINARY_TDM_RX_5;
	case AFE_PORT_ID_QUINARY_TDM_TX_5:
		return IDX_QUINARY_TDM_TX_5;
	case AFE_PORT_ID_QUINARY_TDM_RX_6:
		return IDX_QUINARY_TDM_RX_6;
	case AFE_PORT_ID_QUINARY_TDM_TX_6:
		return IDX_QUINARY_TDM_TX_6;
	case AFE_PORT_ID_QUINARY_TDM_RX_7:
		return IDX_QUINARY_TDM_RX_7;
	case AFE_PORT_ID_QUINARY_TDM_TX_7:
		return IDX_QUINARY_TDM_TX_7;
	case AFE_PORT_ID_SENARY_TDM_RX:
		return IDX_SENARY_TDM_RX_0;
	case AFE_PORT_ID_SENARY_TDM_TX:
		return IDX_SENARY_TDM_TX_0;
	case AFE_PORT_ID_SENARY_TDM_RX_1:
		return IDX_SENARY_TDM_RX_1;
	case AFE_PORT_ID_SENARY_TDM_TX_1:
		return IDX_SENARY_TDM_TX_1;
	case AFE_PORT_ID_SENARY_TDM_RX_2:
		return IDX_SENARY_TDM_RX_2;
	case AFE_PORT_ID_SENARY_TDM_TX_2:
		return IDX_SENARY_TDM_TX_2;
	case AFE_PORT_ID_SENARY_TDM_RX_3:
		return IDX_SENARY_TDM_RX_3;
	case AFE_PORT_ID_SENARY_TDM_TX_3:
		return IDX_SENARY_TDM_TX_3;
	case AFE_PORT_ID_SENARY_TDM_RX_4:
		return IDX_SENARY_TDM_RX_4;
	case AFE_PORT_ID_SENARY_TDM_TX_4:
		return IDX_SENARY_TDM_TX_4;
	case AFE_PORT_ID_SENARY_TDM_RX_5:
		return IDX_SENARY_TDM_RX_5;
	case AFE_PORT_ID_SENARY_TDM_TX_5:
		return IDX_SENARY_TDM_TX_5;
	case AFE_PORT_ID_SENARY_TDM_RX_6:
		return IDX_SENARY_TDM_RX_6;
	case AFE_PORT_ID_SENARY_TDM_TX_6:
		return IDX_SENARY_TDM_TX_6;
	case AFE_PORT_ID_SENARY_TDM_RX_7:
		return IDX_SENARY_TDM_RX_7;
	case AFE_PORT_ID_SENARY_TDM_TX_7:
		return IDX_SENARY_TDM_TX_7;
	default: return -EINVAL;
	}
}

static u16 msm_dai_q6_max_num_slot(int frame_rate)
{
	/* Max num of slots is bits per frame divided
	 * by bits per sample which is 16
	 */
	switch (frame_rate) {
	case AFE_PORT_PCM_BITS_PER_FRAME_8:
		return 0;
	case AFE_PORT_PCM_BITS_PER_FRAME_16:
		return 1;
	case AFE_PORT_PCM_BITS_PER_FRAME_32:
		return 2;
	case AFE_PORT_PCM_BITS_PER_FRAME_64:
		return 4;
	case AFE_PORT_PCM_BITS_PER_FRAME_128:
		return 8;
	case AFE_PORT_PCM_BITS_PER_FRAME_256:
		return 16;
	default:
		pr_err("%s Invalid bits per frame %d\n",
			__func__, frame_rate);
		return 0;
	}
}

static int msm_dai_q6_dai_add_route(struct snd_soc_dai *dai)
{
	struct snd_soc_dapm_route intercon;
	struct snd_soc_dapm_context *dapm;

	if (!dai) {
		pr_err("%s: Invalid params dai\n", __func__);
		return -EINVAL;
	}
	if (!dai->driver) {
		pr_err("%s: Invalid params dai driver\n", __func__);
		return -EINVAL;
	}
	dapm = snd_soc_component_get_dapm(dai->component);
	memset(&intercon, 0, sizeof(intercon));
	if (dai->driver->playback.stream_name &&
		dai->driver->playback.aif_name) {
		dev_dbg(dai->dev, "%s: add route for widget %s",
				__func__, dai->driver->playback.stream_name);
		intercon.source = dai->driver->playback.aif_name;
		intercon.sink = dai->driver->playback.stream_name;
		dev_dbg(dai->dev, "%s: src %s sink %s\n",
				__func__, intercon.source, intercon.sink);
		snd_soc_dapm_add_routes(dapm, &intercon, 1);
		snd_soc_dapm_ignore_suspend(dapm, intercon.sink);
	}
	if (dai->driver->capture.stream_name &&
		dai->driver->capture.aif_name) {
		dev_dbg(dai->dev, "%s: add route for widget %s",
				__func__, dai->driver->capture.stream_name);
		intercon.sink = dai->driver->capture.aif_name;
		intercon.source = dai->driver->capture.stream_name;
		dev_dbg(dai->dev, "%s: src %s sink %s\n",
				__func__, intercon.source, intercon.sink);
		snd_soc_dapm_add_routes(dapm, &intercon, 1);
		snd_soc_dapm_ignore_suspend(dapm, intercon.source);
	}
	return 0;
}

static int msm_dai_q6_auxpcm_hw_params(
				struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct msm_dai_q6_auxpcm_dai_data *aux_dai_data =
			dev_get_drvdata(dai->dev);
	struct msm_dai_q6_dai_data *dai_data = &aux_dai_data->bdai_data;
	int rc = 0;

	dai_data->channels = params_channels(params);
	dai_data->rate = params_rate(params);
	return rc;
}

static int msm_dai_q6_auxpcm_set_clk(
		struct msm_dai_q6_auxpcm_dai_data *aux_dai_data,
		u16 port_id, bool enable)
{
	int rc;

    aux_dai_data->aux_clk_set.clk_id = aux_dai_data->clk_set.clk_id;
    aux_dai_data->aux_clk_set.clk_freq_in_hz = aux_dai_data->clk_set.clk_freq_in_hz;
    aux_dai_data->aux_clk_set.clk_attri = aux_dai_data->clk_set.clk_attri;
    aux_dai_data->aux_clk_set.clk_root = aux_dai_data->clk_set.clk_root;

	pr_debug("%s: clk_ver: %d, port_id: %d, enable: %d freq:%d\n", __func__,
        aux_dai_data->clk_ver, port_id, enable, aux_dai_data->aux_clk_set.clk_freq_in_hz);
	aux_dai_data->clk_set.enable = enable;
	rc = audio_prm_set_lpass_clk_cfg(&aux_dai_data->aux_clk_set, enable);
	if (!enable)
	rc = audio_prm_set_lpass_clk_cfg(&aux_dai_data->aux_clk_set, enable);

	return rc;
}

static void msm_dai_q6_auxpcm_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	int rc = 0;
	struct msm_dai_q6_auxpcm_dai_data *aux_dai_data =
		dev_get_drvdata(dai->dev);

	mutex_lock(&aux_dai_data->rlock);

	if (!(test_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status) ||
	      test_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status))) {
		dev_dbg(dai->dev, "%s(): dai->id %d PCM ports already closed\n",
				__func__, dai->id);
		goto exit;
	}

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (test_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status))
			clear_bit(STATUS_TX_PORT,
				  aux_dai_data->auxpcm_port_status);
		else {
			dev_dbg(dai->dev, "%s: PCM_TX port already closed\n",
				__func__);
			goto exit;
		}
	} else if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (test_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status))
			clear_bit(STATUS_RX_PORT,
				  aux_dai_data->auxpcm_port_status);
		else {
			dev_dbg(dai->dev, "%s: PCM_RX port already closed\n",
				__func__);
			goto exit;
		}
	}
	if (test_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status) ||
	    test_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status)) {
		dev_dbg(dai->dev, "%s: cannot shutdown PCM ports\n",
			__func__);
		goto exit;
	}

	dev_dbg(dai->dev, "%s: dai->id = %d closing PCM AFE ports\n",
			__func__, dai->id);

	if (rc < 0)
		dev_err(dai->dev, "fail to close PCM_RX  AFE port\n");

	if (rc < 0)
		dev_err(dai->dev, "fail to close AUX PCM TX port\n");

	msm_dai_q6_auxpcm_set_clk(aux_dai_data, aux_dai_data->rx_pid, false);
	msm_dai_q6_auxpcm_set_clk(aux_dai_data, aux_dai_data->tx_pid, false);
exit:
	mutex_unlock(&aux_dai_data->rlock);
}

static int msm_dai_q6_auxpcm_prepare(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct msm_dai_q6_auxpcm_dai_data *aux_dai_data =
		dev_get_drvdata(dai->dev);
	struct msm_dai_q6_dai_data *dai_data = &aux_dai_data->bdai_data;
	struct msm_dai_auxpcm_pdata *auxpcm_pdata = NULL;
	int rc = 0;
        u32 pcm_clk_rate = 0;

	auxpcm_pdata = dai->dev->platform_data;
	mutex_lock(&aux_dai_data->rlock);

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (test_bit(STATUS_TX_PORT,
				aux_dai_data->auxpcm_port_status)) {
			dev_dbg(dai->dev, "%s: PCM_TX port already ON\n",
				__func__);
			goto exit;
		} else
			set_bit(STATUS_TX_PORT,
				  aux_dai_data->auxpcm_port_status);
	} else if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (test_bit(STATUS_RX_PORT,
				aux_dai_data->auxpcm_port_status)) {
			dev_dbg(dai->dev, "%s: PCM_RX port already ON\n",
				__func__);
			goto exit;
		} else
			set_bit(STATUS_RX_PORT,
				  aux_dai_data->auxpcm_port_status);
	}
	if (test_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status) &&
	    test_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status)) {
		dev_dbg(dai->dev, "%s: PCM ports already set\n", __func__);
		goto exit;
	}
	if (dai_data->rate == 8000) {
		pcm_clk_rate = auxpcm_pdata->mode_8k.pcm_clk_rate;
	} else if (dai_data->rate == 16000) {
		pcm_clk_rate = (auxpcm_pdata->mode_16k.pcm_clk_rate);
	} else if (dai_data->rate == 32000) {
		pcm_clk_rate = (auxpcm_pdata->mode_32k.pcm_clk_rate);
	} else if (dai_data->rate == 48000) {
		pcm_clk_rate = (auxpcm_pdata->mode_48k.pcm_clk_rate);
	} else {
		dev_err(dai->dev, "%s: Invalid AUX PCM rate %d\n", __func__,
			dai_data->rate);
		rc = -EINVAL;
		goto fail;
	}
        pcm_clk_rate = dai_data->rate*16*8; //sample_rate * bitwidth * number_of_slots
	memcpy(&aux_dai_data->clk_set, &lpass_clk_set_default,
				sizeof(struct afe_clk_set));
	aux_dai_data->clk_set.clk_freq_in_hz = pcm_clk_rate;

		switch (dai->id) {
		case MSM_DAI_PRI_AUXPCM_DT_DEV_ID:
			if (pcm_clk_rate)
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_PRI_PCM_IBIT;
			else
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_PRI_PCM_EBIT;
			break;
		case MSM_DAI_SEC_AUXPCM_DT_DEV_ID:
			if (pcm_clk_rate)
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_SEC_PCM_IBIT;
			else
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_SEC_PCM_EBIT;
			break;
		case MSM_DAI_TERT_AUXPCM_DT_DEV_ID:
			if (pcm_clk_rate)
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_TER_PCM_IBIT;
			else
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_TER_PCM_EBIT;
			break;
		case MSM_DAI_QUAT_AUXPCM_DT_DEV_ID:
			if (pcm_clk_rate)
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_QUAD_PCM_IBIT;
			else
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_QUAD_PCM_EBIT;
			break;
		case MSM_DAI_QUIN_AUXPCM_DT_DEV_ID:
			if (pcm_clk_rate)
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_QUI_PCM_IBIT;
			else
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_QUI_PCM_EBIT;
			break;
		case MSM_DAI_SEN_AUXPCM_DT_DEV_ID:
			if (pcm_clk_rate)
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_SEN_PCM_EBIT;
			else
				aux_dai_data->clk_set.clk_id =
					CLOCK_ID_SEN_PCM_EBIT;
			break;
		default:
			dev_err(dai->dev, "%s: AUXPCM id: %d not supported\n",
				__func__, dai->id);
			break;
		}

	rc = msm_dai_q6_auxpcm_set_clk(aux_dai_data,
				       aux_dai_data->rx_pid, true);
	if (rc < 0) {
		dev_err(dai->dev,
			"%s:afe_set_lpass_clock on RX pcm_src_clk failed\n",
			__func__);
		goto fail;
	}

	rc = msm_dai_q6_auxpcm_set_clk(aux_dai_data,
				       aux_dai_data->tx_pid, true);
	if (rc < 0) {
		dev_err(dai->dev,
			"%s:afe_set_lpass_clock on TX pcm_src_clk failed\n",
			__func__);
		goto fail;
	}

fail:
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		clear_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status);
	else if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		clear_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status);

exit:
	mutex_unlock(&aux_dai_data->rlock);
	return rc;
}

static int msm_dai_q6_dai_auxpcm_remove(struct snd_soc_dai *dai)
{
	struct msm_dai_q6_auxpcm_dai_data *aux_dai_data;

	aux_dai_data = dev_get_drvdata(dai->dev);

	dev_dbg(dai->dev, "%s: dai->id %d closing afe\n",
		__func__, dai->id);

	if (test_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status) ||
	    test_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status)) {
		clear_bit(STATUS_TX_PORT, aux_dai_data->auxpcm_port_status);
		clear_bit(STATUS_RX_PORT, aux_dai_data->auxpcm_port_status);
	}
	msm_dai_q6_auxpcm_set_clk(aux_dai_data, aux_dai_data->rx_pid, false);
	msm_dai_q6_auxpcm_set_clk(aux_dai_data, aux_dai_data->tx_pid, false);
	return 0;
}

/*
 * For single CPU DAI registration, the dai id needs to be
 * set explicitly in the dai probe as ASoC does not read
 * the cpu->driver->id field rather it assigns the dai id
 * from the device name that is in the form %s.%d. This dai
 * id should be assigned to back-end AFE port id and used
 * during dai prepare. For multiple dai registration, it
 * is not required to call this function, however the dai->
 * driver->id field must be defined and set to corresponding
 * AFE Port id.
 */
static inline void msm_dai_q6_set_dai_id(struct snd_soc_dai *dai)
{
	if (!dai->driver) {
		dev_err(dai->dev, "DAI driver is not set\n");
		return;
	}
	if (!dai->driver->id) {
		dev_dbg(dai->dev, "DAI driver id is not set\n");
		return;
	}
	dai->id = dai->driver->id;
}

static int msm_dai_q6_aux_pcm_probe(struct snd_soc_dai *dai)
{
	int rc = 0;
	struct msm_dai_q6_auxpcm_dai_data *dai_data = NULL;

	if (!dai) {
		pr_err("%s: Invalid params dai\n", __func__);
		return -EINVAL;
	}
	if (!dai->dev) {
		pr_err("%s: Invalid params dai dev\n", __func__);
		return -EINVAL;
	}

	msm_dai_q6_set_dai_id(dai);
	dai_data = dev_get_drvdata(dai->dev);

	rc = msm_dai_q6_dai_add_route(dai);
	return rc;
}

static struct snd_soc_dai_ops msm_dai_q6_auxpcm_ops = {
	.prepare	= msm_dai_q6_auxpcm_prepare,
	.hw_params	= msm_dai_q6_auxpcm_hw_params,
	.shutdown	= msm_dai_q6_auxpcm_shutdown,
};

static const struct snd_soc_component_driver
	msm_dai_q6_aux_pcm_dai_component = {
	.name		= "msm-auxpcm-dev",
};

static struct snd_soc_dai_driver msm_dai_q6_aux_pcm_dai[] = {
	{
		.playback = {
			.stream_name = "AUX PCM Playback",
			.aif_name = "AUX_PCM_RX",
                        .rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
                                SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_max = 48000,
			.rate_min = 8000,
		},
		.capture = {
			.stream_name = "AUX PCM Capture",
			.aif_name = "AUX_PCM_TX",
                        .rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
                                SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_max = 48000,
			.rate_min = 8000,
		},
		.id = MSM_DAI_PRI_AUXPCM_DT_DEV_ID,
		.name = "Pri AUX PCM",
		.ops = &msm_dai_q6_auxpcm_ops,
		.probe = msm_dai_q6_aux_pcm_probe,
		.remove = msm_dai_q6_dai_auxpcm_remove,
	},
	{
		.playback = {
			.stream_name = "Sec AUX PCM Playback",
			.aif_name = "SEC_AUX_PCM_RX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.capture = {
			.stream_name = "Sec AUX PCM Capture",
			.aif_name = "SEC_AUX_PCM_TX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.id = MSM_DAI_SEC_AUXPCM_DT_DEV_ID,
		.name = "Sec AUX PCM",
		.ops = &msm_dai_q6_auxpcm_ops,
		.probe = msm_dai_q6_aux_pcm_probe,
		.remove = msm_dai_q6_dai_auxpcm_remove,
	},
	{
		.playback = {
			.stream_name = "Tert AUX PCM Playback",
			.aif_name = "TERT_AUX_PCM_RX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.capture = {
			.stream_name = "Tert AUX PCM Capture",
			.aif_name = "TERT_AUX_PCM_TX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.id = MSM_DAI_TERT_AUXPCM_DT_DEV_ID,
		.name = "Tert AUX PCM",
		.ops = &msm_dai_q6_auxpcm_ops,
		.probe = msm_dai_q6_aux_pcm_probe,
		.remove = msm_dai_q6_dai_auxpcm_remove,
	},
	{
		.playback = {
			.stream_name = "Quat AUX PCM Playback",
			.aif_name = "QUAT_AUX_PCM_RX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.capture = {
			.stream_name = "Quat AUX PCM Capture",
			.aif_name = "QUAT_AUX_PCM_TX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.id = MSM_DAI_QUAT_AUXPCM_DT_DEV_ID,
		.name = "Quat AUX PCM",
		.ops = &msm_dai_q6_auxpcm_ops,
		.probe = msm_dai_q6_aux_pcm_probe,
		.remove = msm_dai_q6_dai_auxpcm_remove,
	},
	{
		.playback = {
			.stream_name = "Quin AUX PCM Playback",
			.aif_name = "QUIN_AUX_PCM_RX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.capture = {
			.stream_name = "Quin AUX PCM Capture",
			.aif_name = "QUIN_AUX_PCM_TX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.id = MSM_DAI_QUIN_AUXPCM_DT_DEV_ID,
		.name = "Quin AUX PCM",
		.ops = &msm_dai_q6_auxpcm_ops,
		.probe = msm_dai_q6_aux_pcm_probe,
		.remove = msm_dai_q6_dai_auxpcm_remove,
	},
	{
		.playback = {
			.stream_name = "Sen AUX PCM Playback",
			.aif_name = "SEN_AUX_PCM_RX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.capture = {
			.stream_name = "Sen AUX PCM Capture",
			.aif_name = "SEN_AUX_PCM_TX",
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_max = 16000,
			.rate_min = 8000,
		},
		.id = MSM_DAI_SEN_AUXPCM_DT_DEV_ID,
		.name = "Sen AUX PCM",
		.ops = &msm_dai_q6_auxpcm_ops,
		.probe = msm_dai_q6_aux_pcm_probe,
		.remove = msm_dai_q6_dai_auxpcm_remove,
	},
};
static u16 num_of_bits_set(u16 sd_line_mask)
{
	u8 num_bits_set = 0;

	while (sd_line_mask) {
		num_bits_set++;
		sd_line_mask = sd_line_mask & (sd_line_mask - 1);
	}
	return num_bits_set;
}

static int msm_auxpcm_dev_probe(struct platform_device *pdev)
{
	struct msm_dai_q6_auxpcm_dai_data *dai_data;
	struct msm_dai_auxpcm_pdata *auxpcm_pdata;
	uint32_t val_array[RATE_MAX_NUM_OF_AUX_PCM_RATES];
	uint32_t val = 0;
	const char *intf_name;
	int rc = 0, i = 0, len = 0;
	const uint32_t *slot_mapping_array = NULL;
	u32 array_length = 0;

	dai_data = kzalloc(sizeof(struct msm_dai_q6_auxpcm_dai_data),
			   GFP_KERNEL);
	if (!dai_data)
		return -ENOMEM;

	rc = of_property_read_u32(pdev->dev.of_node,
				    "qcom,msm-dai-is-island-supported",
				    &dai_data->is_island_dai);
	if (rc)
		dev_dbg(&pdev->dev, "island supported entry not found\n");

	auxpcm_pdata = kzalloc(sizeof(struct msm_dai_auxpcm_pdata),
				GFP_KERNEL);

	if (!auxpcm_pdata) {
		dev_err(&pdev->dev, "Failed to allocate memory for platform data\n");
		goto fail_pdata_nomem;
	}

	dev_dbg(&pdev->dev, "%s: dev %pK, dai_data %pK, auxpcm_pdata %pK\n",
		__func__, &pdev->dev, dai_data, auxpcm_pdata);

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-mode",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);
	if (rc) {
		dev_err(&pdev->dev, "%s: qcom,msm-cpudai-auxpcm-mode missing in DT node\n",
			__func__);
		goto fail_invalid_dt;
	}
	auxpcm_pdata->mode_8k.mode = (u16)val_array[RATE_8KHZ];
	auxpcm_pdata->mode_16k.mode = (u16)val_array[RATE_16KHZ];

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-sync",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);
	if (rc) {
		dev_err(&pdev->dev, "%s: qcom,msm-cpudai-auxpcm-sync missing in DT node\n",
			__func__);
		goto fail_invalid_dt;
	}
	auxpcm_pdata->mode_8k.sync = (u16)val_array[RATE_8KHZ];
	auxpcm_pdata->mode_16k.sync = (u16)val_array[RATE_16KHZ];

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-frame",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);

	if (rc) {
		dev_err(&pdev->dev, "%s: qcom,msm-cpudai-auxpcm-frame missing in DT node\n",
			__func__);
		goto fail_invalid_dt;
	}
	auxpcm_pdata->mode_8k.frame = (u16)val_array[RATE_8KHZ];
	auxpcm_pdata->mode_16k.frame = (u16)val_array[RATE_16KHZ];

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-quant",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);
	if (rc) {
		dev_err(&pdev->dev, "%s: qcom,msm-cpudai-auxpcm-quant missing in DT node\n",
			__func__);
		goto fail_invalid_dt;
	}
	auxpcm_pdata->mode_8k.quant = (u16)val_array[RATE_8KHZ];
	auxpcm_pdata->mode_16k.quant = (u16)val_array[RATE_16KHZ];

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-num-slots",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);
	if (rc) {
		dev_err(&pdev->dev, "%s: qcom,msm-cpudai-auxpcm-num-slots missing in DT node\n",
			__func__);
		goto fail_invalid_dt;
	}
	auxpcm_pdata->mode_8k.num_slots = (u16)val_array[RATE_8KHZ];

	if (auxpcm_pdata->mode_8k.num_slots >
	    msm_dai_q6_max_num_slot(auxpcm_pdata->mode_8k.frame)) {
		dev_err(&pdev->dev, "%s Max slots %d greater than DT node %d\n",
			 __func__,
			msm_dai_q6_max_num_slot(auxpcm_pdata->mode_8k.frame),
			auxpcm_pdata->mode_8k.num_slots);
		rc = -EINVAL;
		goto fail_invalid_dt;
	}
	auxpcm_pdata->mode_16k.num_slots = (u16)val_array[RATE_16KHZ];

	if (auxpcm_pdata->mode_16k.num_slots >
	    msm_dai_q6_max_num_slot(auxpcm_pdata->mode_16k.frame)) {
		dev_err(&pdev->dev, "%s Max slots %d greater than DT node %d\n",
			__func__,
			msm_dai_q6_max_num_slot(auxpcm_pdata->mode_16k.frame),
			auxpcm_pdata->mode_16k.num_slots);
		rc = -EINVAL;
		goto fail_invalid_dt;
	}

	slot_mapping_array = of_get_property(pdev->dev.of_node,
				"qcom,msm-cpudai-auxpcm-slot-mapping", &len);

	if (slot_mapping_array == NULL) {
		dev_err(&pdev->dev, "%s slot_mapping_array is not valid\n",
			__func__);
		rc = -EINVAL;
		goto fail_invalid_dt;
	}

	array_length = auxpcm_pdata->mode_8k.num_slots +
		       auxpcm_pdata->mode_16k.num_slots;

	if (len != sizeof(uint32_t) * array_length) {
		dev_err(&pdev->dev, "%s Length is %d and expected is %zd\n",
			__func__, len, sizeof(uint32_t) * array_length);
		rc = -EINVAL;
		goto fail_invalid_dt;
	}

	auxpcm_pdata->mode_8k.slot_mapping =
					kzalloc(sizeof(uint16_t) *
					    auxpcm_pdata->mode_8k.num_slots,
					    GFP_KERNEL);
	if (!auxpcm_pdata->mode_8k.slot_mapping) {
		dev_err(&pdev->dev, "%s No mem for mode_8k slot mapping\n",
			__func__);
		rc = -ENOMEM;
		goto fail_invalid_dt;
	}

	for (i = 0; i < auxpcm_pdata->mode_8k.num_slots; i++)
		auxpcm_pdata->mode_8k.slot_mapping[i] =
				(u16)be32_to_cpu(slot_mapping_array[i]);

	auxpcm_pdata->mode_16k.slot_mapping =
					kzalloc(sizeof(uint16_t) *
					     auxpcm_pdata->mode_16k.num_slots,
					     GFP_KERNEL);

	if (!auxpcm_pdata->mode_16k.slot_mapping) {
		dev_err(&pdev->dev, "%s No mem for mode_16k slot mapping\n",
			__func__);
		rc = -ENOMEM;
		goto fail_invalid_16k_slot_mapping;
	}

	for (i = 0; i < auxpcm_pdata->mode_16k.num_slots; i++)
		auxpcm_pdata->mode_16k.slot_mapping[i] =
			(u16)be32_to_cpu(slot_mapping_array[i +
					auxpcm_pdata->mode_8k.num_slots]);

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-data",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);
	if (rc) {
		dev_err(&pdev->dev, "%s: qcom,msm-cpudai-auxpcm-data missing in DT node\n",
			__func__);
		goto fail_invalid_dt1;
	}
	auxpcm_pdata->mode_8k.data = (u16)val_array[RATE_8KHZ];
	auxpcm_pdata->mode_16k.data = (u16)val_array[RATE_16KHZ];

	rc = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,msm-cpudai-auxpcm-pcm-clk-rate",
			val_array, RATE_MAX_NUM_OF_AUX_PCM_RATES);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: qcom,msm-cpudai-auxpcm-pcm-clk-rate missing in DT\n",
			__func__);
		goto fail_invalid_dt1;
	}
	auxpcm_pdata->mode_8k.pcm_clk_rate = (int)val_array[RATE_8KHZ];
	auxpcm_pdata->mode_16k.pcm_clk_rate = (int)val_array[RATE_16KHZ];

	rc = of_property_read_string(pdev->dev.of_node,
			"qcom,msm-auxpcm-interface", &intf_name);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: qcom,msm-auxpcm-interface missing in DT node\n",
			__func__);
		goto fail_nodev_intf;
	}

	if (!strcmp(intf_name, "primary")) {
		dai_data->rx_pid = AFE_PORT_ID_PRIMARY_PCM_RX;
		dai_data->tx_pid = AFE_PORT_ID_PRIMARY_PCM_TX;
		pdev->id = MSM_DAI_PRI_AUXPCM_DT_DEV_ID;
		i = 0;
	} else if (!strcmp(intf_name, "secondary")) {
		dai_data->rx_pid = AFE_PORT_ID_SECONDARY_PCM_RX;
		dai_data->tx_pid = AFE_PORT_ID_SECONDARY_PCM_TX;
		pdev->id = MSM_DAI_SEC_AUXPCM_DT_DEV_ID;
		i = 1;
	} else if (!strcmp(intf_name, "tertiary")) {
		dai_data->rx_pid = AFE_PORT_ID_TERTIARY_PCM_RX;
		dai_data->tx_pid = AFE_PORT_ID_TERTIARY_PCM_TX;
		pdev->id = MSM_DAI_TERT_AUXPCM_DT_DEV_ID;
		i = 2;
	} else if (!strcmp(intf_name, "quaternary")) {
		dai_data->rx_pid = AFE_PORT_ID_QUATERNARY_PCM_RX;
		dai_data->tx_pid = AFE_PORT_ID_QUATERNARY_PCM_TX;
		pdev->id = MSM_DAI_QUAT_AUXPCM_DT_DEV_ID;
		i = 3;
	} else if (!strcmp(intf_name, "quinary")) {
		dai_data->rx_pid = AFE_PORT_ID_QUINARY_PCM_RX;
		dai_data->tx_pid = AFE_PORT_ID_QUINARY_PCM_TX;
		pdev->id = MSM_DAI_QUIN_AUXPCM_DT_DEV_ID;
		i = 4;
	} else if (!strcmp(intf_name, "senary")) {
		dai_data->rx_pid = AFE_PORT_ID_SENARY_PCM_RX;
		dai_data->tx_pid = AFE_PORT_ID_SENARY_PCM_TX;
		pdev->id = MSM_DAI_SEN_AUXPCM_DT_DEV_ID;
		i = 5;
	} else {
		dev_err(&pdev->dev, "%s: invalid DT intf name %s\n",
			__func__, intf_name);
		goto fail_invalid_intf;
	}
	rc = of_property_read_u32(pdev->dev.of_node,
				  "qcom,msm-cpudai-afe-clk-ver", &val);
	if (rc)
		dai_data->clk_ver = AFE_CLK_VERSION_V1;
	else
		dai_data->clk_ver = val;

	mutex_init(&dai_data->rlock);
	dev_dbg(&pdev->dev, "dev name %s\n", dev_name(&pdev->dev));

	dev_set_drvdata(&pdev->dev, dai_data);
	pdev->dev.platform_data = (void *) auxpcm_pdata;

	rc = snd_soc_register_component(&pdev->dev,
			&msm_dai_q6_aux_pcm_dai_component,
			&msm_dai_q6_aux_pcm_dai[i], 1);
	if (rc) {
		dev_err(&pdev->dev, "%s: auxpcm dai reg failed, rc=%d\n",
				__func__, rc);
		goto fail_reg_dai;
	}

	return rc;

fail_reg_dai:
fail_invalid_intf:
fail_nodev_intf:
fail_invalid_dt1:
	kfree(auxpcm_pdata->mode_16k.slot_mapping);
fail_invalid_16k_slot_mapping:
	kfree(auxpcm_pdata->mode_8k.slot_mapping);
fail_invalid_dt:
	kfree(auxpcm_pdata);
fail_pdata_nomem:
	kfree(dai_data);
	return rc;
}

static int msm_auxpcm_dev_remove(struct platform_device *pdev)
{
	struct msm_dai_q6_auxpcm_dai_data *dai_data;

	dai_data = dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_component(&pdev->dev);

	mutex_destroy(&dai_data->rlock);
	kfree(dai_data);
	kfree(pdev->dev.platform_data);

	return 0;
}

static const struct of_device_id msm_auxpcm_dev_dt_match[] = {
	{ .compatible = "qcom,msm-auxpcm-dev", },
	{}
};


static struct platform_driver msm_auxpcm_dev_driver = {
	.probe  = msm_auxpcm_dev_probe,
	.remove = msm_auxpcm_dev_remove,
	.driver = {
		.name = "msm-auxpcm-dev",
		.owner = THIS_MODULE,
		.of_match_table = msm_auxpcm_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

/* Channel min and max are initialized base on platform data */
static struct snd_soc_dai_driver msm_dai_q6_slimbus_rx_dai[] = {
	{
		.playback = {
			.stream_name = "Slimbus Playback",
			.aif_name = "SLIMBUS_0_RX",
			.rates = SNDRV_PCM_RATE_8000_384000,
			.formats = DAI_FORMATS_S16_S24_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = SLIMBUS_0_RX,
	},
	{
		.playback = {
			.stream_name = "Slimbus6 Playback",
			.aif_name = "SLIMBUS_6_RX",
			.rates = SNDRV_PCM_RATE_8000_384000,
			.formats = DAI_FORMATS_S16_S24_S32_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = SLIMBUS_6_RX,
	},
};

static struct snd_soc_dai_driver msm_dai_q6_slimbus_tx_dai[] = {
	{
		.capture = {
			.stream_name = "Slimbus Capture",
			.aif_name = "SLIMBUS_0_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_96000 |
			SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 192000,
		},
		.id = SLIMBUS_0_TX,
	},
};

/* Channel min and max are initialized base on platform data */
static struct snd_soc_dai_driver msm_dai_q6_mi2s_dai[] = {
	{
		.playback = {
			.stream_name = "Primary MI2S Playback",
			.aif_name = "PRI_MI2S_RX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     384000,
		},
		.capture = {
			.stream_name = "Primary MI2S Capture",
			.aif_name = "PRI_MI2S_TX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.name = "Primary MI2S",
		.id = MSM_PRIM_MI2S,
	},

	{
		.playback = {
			.stream_name = "Secondary MI2S Playback",
			.aif_name = "SEC_MI2S_RX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.capture = {
			.stream_name = "Secondary MI2S Capture",
			.aif_name = "SEC_MI2S_TX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.name = "Secondary MI2S",
		.id = MSM_SEC_MI2S,
	},
	{
		.playback = {
			.stream_name = "Tertiary MI2S Playback",
			.aif_name = "TERT_MI2S_RX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.capture = {
			.stream_name = "Tertiary MI2S Capture",
			.aif_name = "TERT_MI2S_TX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.name = "Tertiary MI2S",
		.id = MSM_TERT_MI2S,
	},
	{
		.playback = {
			.stream_name = "Quaternary MI2S Playback",
			.aif_name = "QUAT_MI2S_RX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.capture = {
			.stream_name = "Quaternary MI2S Capture",
			.aif_name = "QUAT_MI2S_TX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.name = "Quaternary MI2S",
		.id = MSM_QUAT_MI2S,
	},
	{
		.playback = {
			.stream_name = "Quinary MI2S Playback",
			.aif_name = "QUIN_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_96000 |
			SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.capture = {
			.stream_name = "Quinary MI2S Capture",
			.aif_name = "QUIN_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "Quinary MI2S",
		.id = MSM_QUIN_MI2S,
	},
	{
		.playback = {
			.stream_name = "Senary MI2S Playback",
			.aif_name = "SEN_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.capture = {
			.stream_name = "Senary MI2S Capture",
			.aif_name = "SENARY_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "Senary MI2S",
		.id = MSM_SENARY_MI2S,
	},
	{
		.playback = {
			.stream_name = "Secondary MI2S Playback SD1",
			.aif_name = "SEC_MI2S_RX_SD1",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.id = MSM_SEC_MI2S_SD1,
	},
	{
		.playback = {
			.stream_name = "INT0 MI2S Playback",
			.aif_name = "INT0_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_44100 |
			SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.capture = {
			.stream_name = "INT0 MI2S Capture",
			.aif_name = "INT0_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT0 MI2S",
		.id = MSM_INT0_MI2S,
	},
	{
		.playback = {
			.stream_name = "INT1 MI2S Playback",
			.aif_name = "INT1_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.capture = {
			.stream_name = "INT1 MI2S Capture",
			.aif_name = "INT1_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT1 MI2S",
		.id = MSM_INT1_MI2S,
	},
	{
		.playback = {
			.stream_name = "INT2 MI2S Playback",
			.aif_name = "INT2_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.capture = {
			.stream_name = "INT2 MI2S Capture",
			.aif_name = "INT2_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT2 MI2S",
		.id = MSM_INT2_MI2S,
	},
	{
		.playback = {
			.stream_name = "INT3 MI2S Playback",
			.aif_name = "INT3_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.capture = {
			.stream_name = "INT3 MI2S Capture",
			.aif_name = "INT3_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT3 MI2S",
		.id = MSM_INT3_MI2S,
	},
	{
		.playback = {
			.stream_name = "INT4 MI2S Playback",
			.aif_name = "INT4_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_96000 |
			SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.capture = {
			.stream_name = "INT4 MI2S Capture",
			.aif_name = "INT4_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT4 MI2S",
		.id = MSM_INT4_MI2S,
	},
	{
		.playback = {
			.stream_name = "INT5 MI2S Playback",
			.aif_name = "INT5_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.capture = {
			.stream_name = "INT5 MI2S Capture",
			.aif_name = "INT5_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT5 MI2S",
		.id = MSM_INT5_MI2S,
	},
	{
		.playback = {
			.stream_name = "INT6 MI2S Playback",
			.aif_name = "INT6_MI2S_RX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.capture = {
			.stream_name = "INT6 MI2S Capture",
			.aif_name = "INT6_MI2S_TX",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
			SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     48000,
		},
		.name = "INT6 MI2S",
		.id = MSM_INT6_MI2S,
	},
};

static int msm_dai_q6_mi2s_get_lineconfig(u16 sd_lines, u16 *config_ptr,
					  unsigned int *ch_cnt)
{
	u8 num_of_sd_lines;

	num_of_sd_lines = num_of_bits_set(sd_lines);
	switch (num_of_sd_lines) {
	case 0:
		pr_debug("%s: no line is assigned\n", __func__);
		break;
	case 1:
		switch (sd_lines) {
		case MSM_MI2S_SD0:
			*config_ptr = AFE_PORT_I2S_SD0;
			break;
		case MSM_MI2S_SD1:
			*config_ptr = AFE_PORT_I2S_SD1;
			break;
		case MSM_MI2S_SD2:
			*config_ptr = AFE_PORT_I2S_SD2;
			break;
		case MSM_MI2S_SD3:
			*config_ptr = AFE_PORT_I2S_SD3;
			break;
		case MSM_MI2S_SD4:
			*config_ptr = AFE_PORT_I2S_SD4;
			break;
		case MSM_MI2S_SD5:
			*config_ptr = AFE_PORT_I2S_SD5;
			break;
		case MSM_MI2S_SD6:
			*config_ptr = AFE_PORT_I2S_SD6;
			break;
		case MSM_MI2S_SD7:
			*config_ptr = AFE_PORT_I2S_SD7;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 2:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1:
			*config_ptr = AFE_PORT_I2S_QUAD01;
			break;
		case MSM_MI2S_SD2 | MSM_MI2S_SD3:
			*config_ptr = AFE_PORT_I2S_QUAD23;
			break;
		case MSM_MI2S_SD4 | MSM_MI2S_SD5:
			*config_ptr = AFE_PORT_I2S_QUAD45;
			break;
		case MSM_MI2S_SD6 | MSM_MI2S_SD7:
			*config_ptr = AFE_PORT_I2S_QUAD67;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 3:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1 | MSM_MI2S_SD2:
			*config_ptr = AFE_PORT_I2S_6CHS;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 4:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1 | MSM_MI2S_SD2 | MSM_MI2S_SD3:
			*config_ptr = AFE_PORT_I2S_8CHS;
			break;
		case MSM_MI2S_SD4 | MSM_MI2S_SD5 | MSM_MI2S_SD6 | MSM_MI2S_SD7:
			*config_ptr = AFE_PORT_I2S_8CHS_2;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 5:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1 | MSM_MI2S_SD2
		   | MSM_MI2S_SD3 | MSM_MI2S_SD4:
			*config_ptr = AFE_PORT_I2S_10CHS;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 6:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1 | MSM_MI2S_SD2
		   | MSM_MI2S_SD3 | MSM_MI2S_SD4 | MSM_MI2S_SD5:
			*config_ptr = AFE_PORT_I2S_12CHS;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 7:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1 | MSM_MI2S_SD2 | MSM_MI2S_SD3
		   | MSM_MI2S_SD4 | MSM_MI2S_SD5 | MSM_MI2S_SD6:
			*config_ptr = AFE_PORT_I2S_14CHS;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	case 8:
		switch (sd_lines) {
		case MSM_MI2S_SD0 | MSM_MI2S_SD1 | MSM_MI2S_SD2 | MSM_MI2S_SD3
		   | MSM_MI2S_SD4 | MSM_MI2S_SD5 | MSM_MI2S_SD6 | MSM_MI2S_SD7:
			*config_ptr = AFE_PORT_I2S_16CHS;
			break;
		default:
			pr_err("%s: invalid SD lines %d\n",
				   __func__, sd_lines);
			goto error_invalid_data;
		}
		break;
	default:
		pr_err("%s: invalid SD lines %d\n", __func__, num_of_sd_lines);
		goto error_invalid_data;
	}
	*ch_cnt = num_of_sd_lines;
	return 0;

error_invalid_data:
	pr_err("%s: invalid data\n", __func__);
	return -EINVAL;
}

#if 0
static u16 msm_dai_q6_mi2s_get_num_channels(u16 config)
{
	switch (config) {
	case AFE_PORT_I2S_SD0:
	case AFE_PORT_I2S_SD1:
	case AFE_PORT_I2S_SD2:
	case AFE_PORT_I2S_SD3:
	case AFE_PORT_I2S_SD4:
	case AFE_PORT_I2S_SD5:
	case AFE_PORT_I2S_SD6:
	case AFE_PORT_I2S_SD7:
		return 2;
	case AFE_PORT_I2S_QUAD01:
	case AFE_PORT_I2S_QUAD23:
	case AFE_PORT_I2S_QUAD45:
	case AFE_PORT_I2S_QUAD67:
		return 4;
	case AFE_PORT_I2S_6CHS:
		return 6;
	case AFE_PORT_I2S_8CHS:
	case AFE_PORT_I2S_8CHS_2:
		return 8;
	case AFE_PORT_I2S_10CHS:
		return 10;
	case AFE_PORT_I2S_12CHS:
		return 12;
	case AFE_PORT_I2S_14CHS:
		return 14;
	case AFE_PORT_I2S_16CHS:
		return 16;
	default:
		pr_err("%s: invalid config\n", __func__);
		return 0;
	}
}
#endif

static int msm_dai_q6_mi2s_platform_data_validation(
	struct platform_device *pdev, struct snd_soc_dai_driver *dai_driver)
{
	struct msm_dai_q6_mi2s_dai_data *dai_data = dev_get_drvdata(&pdev->dev);
	struct msm_mi2s_pdata *mi2s_pdata =
			(struct msm_mi2s_pdata *) pdev->dev.platform_data;
	unsigned int ch_cnt;
	int rc = 0;
	u16 sd_line;

	if (mi2s_pdata == NULL) {
		pr_err("%s: mi2s_pdata NULL", __func__);
		return -EINVAL;
	}

	rc = msm_dai_q6_mi2s_get_lineconfig(mi2s_pdata->rx_sd_lines,
					    &sd_line, &ch_cnt);
	if (rc < 0) {
		dev_err(&pdev->dev, "invalid MI2S RX sd line config\n");
		goto rtn;
	}

	if (ch_cnt) {
		dai_data->rx_dai.mi2s_dai_data.port_config.i2s.channel_mode =
		sd_line;
		dai_data->rx_dai.pdata_mi2s_lines = sd_line;
		dai_driver->playback.channels_min = 1;
		dai_driver->playback.channels_max = ch_cnt << 1;
	} else {
		dai_driver->playback.channels_min = 0;
		dai_driver->playback.channels_max = 0;
	}
	rc = msm_dai_q6_mi2s_get_lineconfig(mi2s_pdata->tx_sd_lines,
					    &sd_line, &ch_cnt);
	if (rc < 0) {
		dev_err(&pdev->dev, "invalid MI2S TX sd line config\n");
		goto rtn;
	}

	if (ch_cnt) {
		dai_data->tx_dai.mi2s_dai_data.port_config.i2s.channel_mode =
		sd_line;
		dai_data->tx_dai.pdata_mi2s_lines = sd_line;
		dai_driver->capture.channels_min = 1;
		dai_driver->capture.channels_max = ch_cnt << 1;
	} else {
		dai_driver->capture.channels_min = 0;
		dai_driver->capture.channels_max = 0;
	}

	dev_dbg(&pdev->dev, "%s: playback sdline 0x%x capture sdline 0x%x\n",
		__func__, dai_data->rx_dai.pdata_mi2s_lines,
		dai_data->tx_dai.pdata_mi2s_lines);
	dev_dbg(&pdev->dev, "%s: playback ch_max %d capture ch_mx %d\n",
		__func__, dai_driver->playback.channels_max,
		dai_driver->capture.channels_max);
rtn:
	return rc;
}

static const struct snd_soc_component_driver msm_q6_mi2s_dai_component = {
	.name		= "msm-dai-q6-mi2s",
};
static int msm_dai_q6_mi2s_dev_probe(struct platform_device *pdev)
{
	struct msm_dai_q6_mi2s_dai_data *dai_data;
	const char *q6_mi2s_dev_id = "qcom,msm-dai-q6-mi2s-dev-id";
	u32 tx_line = 0;
	u32  rx_line = 0;
	u32 mi2s_intf = 0;
	struct msm_mi2s_pdata *mi2s_pdata;
	int rc;

	rc = of_property_read_u32(pdev->dev.of_node, q6_mi2s_dev_id,
				  &mi2s_intf);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: missing 0x%x in dt node\n", __func__, mi2s_intf);
		goto rtn;
	}

	dev_dbg(&pdev->dev, "dev name %s dev id 0x%x\n", dev_name(&pdev->dev),
		mi2s_intf);

	if ((mi2s_intf < MSM_MI2S_MIN || mi2s_intf > MSM_MI2S_MAX)
		|| (mi2s_intf >= ARRAY_SIZE(msm_dai_q6_mi2s_dai))) {
		dev_err(&pdev->dev,
			"%s: Invalid MI2S ID %u from Device Tree\n",
			__func__, mi2s_intf);
		rc = -ENXIO;
		goto rtn;
	}

	pdev->id = mi2s_intf;

	mi2s_pdata = kzalloc(sizeof(struct msm_mi2s_pdata), GFP_KERNEL);
	if (!mi2s_pdata) {
		rc = -ENOMEM;
		goto rtn;
	}

	rc = of_property_read_u32(pdev->dev.of_node, "qcom,msm-mi2s-rx-lines",
				  &rx_line);
	if (rc) {
		dev_err(&pdev->dev, "%s: Rx line from DT file %s\n", __func__,
			"qcom,msm-mi2s-rx-lines");
		goto free_pdata;
	}

	rc = of_property_read_u32(pdev->dev.of_node, "qcom,msm-mi2s-tx-lines",
				  &tx_line);
	if (rc) {
		dev_err(&pdev->dev, "%s: Tx line from DT file %s\n", __func__,
			"qcom,msm-mi2s-tx-lines");
		goto free_pdata;
	}
	dev_dbg(&pdev->dev, "dev name %s Rx line 0x%x , Tx ine 0x%x\n",
		dev_name(&pdev->dev), rx_line, tx_line);
	mi2s_pdata->rx_sd_lines = rx_line;
	mi2s_pdata->tx_sd_lines = tx_line;
	mi2s_pdata->intf_id = mi2s_intf;

	dai_data = kzalloc(sizeof(struct msm_dai_q6_mi2s_dai_data),
			   GFP_KERNEL);
	if (!dai_data) {
		rc = -ENOMEM;
		goto free_pdata;
	} else
		dev_set_drvdata(&pdev->dev, dai_data);

	rc = of_property_read_u32(pdev->dev.of_node,
				    "qcom,msm-dai-is-island-supported",
				    &dai_data->is_island_dai);
	if (rc)
		dev_dbg(&pdev->dev, "island supported entry not found\n");

	pdev->dev.platform_data = mi2s_pdata;

	rc = msm_dai_q6_mi2s_platform_data_validation(pdev,
			&msm_dai_q6_mi2s_dai[mi2s_intf]);
	if (rc < 0)
		goto free_dai_data;

	rc = snd_soc_register_component(&pdev->dev, &msm_q6_mi2s_dai_component,
	&msm_dai_q6_mi2s_dai[mi2s_intf], 1);
	if (rc < 0)
		goto err_register;
	return 0;

err_register:
	dev_err(&pdev->dev, "fail to msm_dai_q6_mi2s_dev_probe\n");
free_dai_data:
	kfree(dai_data);
free_pdata:
	kfree(mi2s_pdata);
rtn:
	return rc;
}

static int msm_dai_q6_mi2s_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

#if 0
static int msm_dai_q6_dai_meta_mi2s_probe(struct snd_soc_dai *dai)
{
	struct msm_meta_mi2s_pdata *meta_mi2s_pdata =
			(struct msm_meta_mi2s_pdata *) dai->dev->platform_data;
	int rc = 0;

	dai->id = meta_mi2s_pdata->intf_id;
	rc = msm_dai_q6_dai_add_route(dai);
	return rc;
}

static int msm_dai_q6_dai_meta_mi2s_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static int msm_dai_q6_meta_mi2s_startup(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	return 0;
}

static int msm_meta_mi2s_get_port_id(u32 mi2s_id, int stream, u16 *port_id)
{
	int ret = 0;

	switch (stream) {
	case SNDRV_PCM_STREAM_PLAYBACK:
		switch (mi2s_id) {
		case MSM_PRIM_META_MI2S:
			*port_id = AFE_PORT_ID_PRIMARY_META_MI2S_RX;
			break;
		case MSM_SEC_META_MI2S:
			*port_id = AFE_PORT_ID_SECONDARY_META_MI2S_RX;
			break;
		default:
			pr_err("%s: playback err id 0x%x\n",
				__func__, mi2s_id);
			ret = -1;
			break;
		}
		break;

	case SNDRV_PCM_STREAM_CAPTURE:
		switch (mi2s_id) {
		default:
			pr_err("%s: capture err id 0x%x\n", __func__, mi2s_id);
			ret = -1;
			break;
		}
		break;

	default:
		pr_err("%s: default err %d\n", __func__, stream);
		ret = -1;
		break;
	}
	pr_debug("%s: port_id = 0x%x\n", __func__, *port_id);
	return ret;
}

static int msm_dai_q6_meta_mi2s_prepare(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct msm_dai_q6_meta_mi2s_dai_data *dai_data =
		dev_get_drvdata(dai->dev);
	u16 port_id = 0;
	int rc = 0;

	if (msm_meta_mi2s_get_port_id(dai->id, substream->stream,
		&port_id) != 0) {
		dev_err(dai->dev, "%s: Invalid Port ID 0x%x\n",
			__func__, port_id);
		return -EINVAL;
	}

	dev_dbg(dai->dev, "%s: dai id %d, afe port id = 0x%x\n"
		"dai_data->channels = %u sample_rate = %u\n", __func__,
		dai->id, port_id, dai_data->channels, dai_data->rate);

	if (!test_bit(STATUS_PORT_STARTED, dai_data->status_mask)) {
		/* PORT START should be set if prepare called
		 * in active state.
		 */
		rc = afe_port_start(port_id, &dai_data->port_config,
				    dai_data->rate);
		if (rc < 0)
			dev_err(dai->dev, "fail to open AFE port 0x%x\n",
				dai->id);
		else
			set_bit(STATUS_PORT_STARTED,
				dai_data->status_mask);
	}

	return rc;
}

static int msm_dai_q6_meta_mi2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct msm_dai_q6_meta_mi2s_dai_data *dai_data =
		dev_get_drvdata(dai->dev);
	struct afe_param_id_meta_i2s_cfg *port_cfg =
		&dai_data->port_config.meta_i2s;
	int idx = 0;
	u16 port_channels = 0;
	u16 channels_left = 0;

	dai_data->channels = params_channels(params);
	channels_left = dai_data->channels;

	/* map requested channels to channels that member ports provide */
	for (idx = 0; idx < dai_data->num_member_ports; idx++) {
		port_channels = msm_dai_q6_mi2s_get_num_channels(
			dai_data->channel_mode[idx]);

		if (channels_left >= port_channels) {
			port_cfg->member_port_id[idx] =
				dai_data->member_port_id[idx];
			port_cfg->member_port_channel_mode[idx] =
				dai_data->channel_mode[idx];
			channels_left -= port_channels;
		} else {
			switch (channels_left) {
			case 15:
			case 16:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_16CHS;
					break;
				default:
					goto error_invalid_data;
				};
				break;
			case 13:
			case 14:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_14CHS:
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_14CHS;
					break;
				default:
					goto error_invalid_data;
				};
				break;
			case 11:
			case 12:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_12CHS:
				case AFE_PORT_I2S_14CHS:
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_12CHS;
					break;
				default:
					goto error_invalid_data;
				};
				break;
			case 9:
			case 10:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_10CHS:
				case AFE_PORT_I2S_12CHS:
				case AFE_PORT_I2S_14CHS:
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_10CHS;
					break;
				default:
					goto error_invalid_data;
				};
				break;
			case 8:
			case 7:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_8CHS:
				case AFE_PORT_I2S_10CHS:
				case AFE_PORT_I2S_12CHS:
				case AFE_PORT_I2S_14CHS:
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_8CHS;
					break;
				case AFE_PORT_I2S_8CHS_2:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_8CHS_2;
					break;
				default:
					goto error_invalid_data;
				};
				break;
			case 6:
			case 5:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_6CHS:
				case AFE_PORT_I2S_8CHS:
				case AFE_PORT_I2S_10CHS:
				case AFE_PORT_I2S_12CHS:
				case AFE_PORT_I2S_14CHS:
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_6CHS;
					break;
				default:
					goto error_invalid_data;
				};
				break;
			case 4:
			case 3:
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_SD0:
				case AFE_PORT_I2S_SD1:
				case AFE_PORT_I2S_SD2:
				case AFE_PORT_I2S_SD3:
				case AFE_PORT_I2S_SD4:
				case AFE_PORT_I2S_SD5:
				case AFE_PORT_I2S_SD6:
				case AFE_PORT_I2S_SD7:
					goto error_invalid_data;
				case AFE_PORT_I2S_QUAD01:
				case AFE_PORT_I2S_QUAD23:
				case AFE_PORT_I2S_QUAD45:
				case AFE_PORT_I2S_QUAD67:
					port_cfg->member_port_channel_mode[idx]
						= dai_data->channel_mode[idx];
					break;
				case AFE_PORT_I2S_8CHS_2:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_QUAD45;
					break;
				default:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_QUAD01;
				};
				break;
			case 2:
			case 1:
				if (dai_data->channel_mode[idx] <
					AFE_PORT_I2S_SD0)
					goto error_invalid_data;
				switch (dai_data->channel_mode[idx]) {
				case AFE_PORT_I2S_SD0:
				case AFE_PORT_I2S_SD1:
				case AFE_PORT_I2S_SD2:
				case AFE_PORT_I2S_SD3:
				case AFE_PORT_I2S_SD4:
				case AFE_PORT_I2S_SD5:
				case AFE_PORT_I2S_SD6:
				case AFE_PORT_I2S_SD7:
					port_cfg->member_port_channel_mode[idx]
						= dai_data->channel_mode[idx];
					break;
				case AFE_PORT_I2S_QUAD01:
				case AFE_PORT_I2S_6CHS:
				case AFE_PORT_I2S_8CHS:
				case AFE_PORT_I2S_10CHS:
				case AFE_PORT_I2S_12CHS:
				case AFE_PORT_I2S_14CHS:
				case AFE_PORT_I2S_16CHS:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_SD0;
					break;
				case AFE_PORT_I2S_QUAD23:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_SD2;
					break;
				case AFE_PORT_I2S_QUAD45:
				case AFE_PORT_I2S_8CHS_2:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_SD4;
					break;
				case AFE_PORT_I2S_QUAD67:
					port_cfg->member_port_channel_mode[idx]
						= AFE_PORT_I2S_SD6;
					break;
				}
				break;
			case 0:
				port_cfg->member_port_channel_mode[idx] = 0;
			}
			if (port_cfg->member_port_channel_mode[idx] == 0) {
				port_cfg->member_port_id[idx] =
					AFE_PORT_ID_INVALID;
			} else {
				port_cfg->member_port_id[idx] =
					dai_data->member_port_id[idx];
				channels_left -=
					msm_dai_q6_mi2s_get_num_channels(
					port_cfg->member_port_channel_mode[idx]);
			}
		}
	}

	if (channels_left > 0) {
		pr_err("%s: too many channels %d\n",
			__func__, dai_data->channels);
		return -EINVAL;
	}

	dai_data->rate = params_rate(params);
	port_cfg->sample_rate = dai_data->rate;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
	case SNDRV_PCM_FORMAT_SPECIAL:
		port_cfg->bit_width = 16;
		dai_data->bitwidth = 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S24_3LE:
		port_cfg->bit_width = 24;
		dai_data->bitwidth = 24;
		break;
	default:
		pr_err("%s: format %d\n",
			__func__, params_format(params));
		return -EINVAL;
	}

	port_cfg->minor_version = AFE_API_VERSION_META_I2S_CONFIG;
	port_cfg->data_format = AFE_LINEAR_PCM_DATA;

	dev_dbg(dai->dev, "%s: dai id %d dai_data->channels = %d\n"
		"bit_width = %hu ws_src = 0x%x sample_rate = %u\n"
		"member_ports 0x%x 0x%x 0x%x 0x%x\n"
		"sd_lines 0x%x 0x%x 0x%x 0x%x\n",
		__func__, dai->id, dai_data->channels,
		port_cfg->bit_width, port_cfg->ws_src, port_cfg->sample_rate,
		port_cfg->member_port_id[0],
		port_cfg->member_port_id[1],
		port_cfg->member_port_id[2],
		port_cfg->member_port_id[3],
		port_cfg->member_port_channel_mode[0],
		port_cfg->member_port_channel_mode[1],
		port_cfg->member_port_channel_mode[2],
		port_cfg->member_port_channel_mode[3]);
	return 0;

error_invalid_data:
	pr_err("%s: error when assigning member port %d channels (channels_left %d)\n",
		__func__, idx, channels_left);
	return -EINVAL;
}

static int msm_dai_q6_meta_mi2s_set_fmt(struct snd_soc_dai *dai,
	unsigned int fmt)
{
	struct msm_dai_q6_meta_mi2s_dai_data *dai_data =
		dev_get_drvdata(dai->dev);

	if (test_bit(STATUS_PORT_STARTED, dai_data->status_mask)) {
		dev_err(dai->dev, "%s: err chg meta i2s mode while dai running",
			__func__);
		return -EPERM;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		dai_data->port_config.meta_i2s.ws_src = 1;
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		dai_data->port_config.meta_i2s.ws_src = 0;
		break;
	default:
		pr_err("%s: fmt %d\n",
			__func__, fmt & SND_SOC_DAIFMT_MASTER_MASK);
		return -EINVAL;
	}

	return 0;
}

static void msm_dai_q6_meta_mi2s_shutdown(struct snd_pcm_substream *substream,
				     struct snd_soc_dai *dai)
{
	struct msm_dai_q6_meta_mi2s_dai_data *dai_data =
		dev_get_drvdata(dai->dev);
	u16 port_id = 0;
	int rc = 0;

	if (msm_meta_mi2s_get_port_id(dai->id, substream->stream,
				 &port_id) != 0) {
		dev_err(dai->dev, "%s: Invalid Port ID 0x%x\n",
			__func__, port_id);
	}

	dev_dbg(dai->dev, "%s: closing afe port id = 0x%x\n",
			__func__, port_id);

	if (test_bit(STATUS_PORT_STARTED, dai_data->status_mask)) {
		rc = afe_close(port_id);
		if (rc < 0)
			dev_err(dai->dev, "fail to close AFE port\n");
		clear_bit(STATUS_PORT_STARTED, dai_data->status_mask);
	}
}

static struct snd_soc_dai_ops msm_dai_q6_meta_mi2s_ops = {
	.startup	= msm_dai_q6_meta_mi2s_startup,
	.prepare	= msm_dai_q6_meta_mi2s_prepare,
	.hw_params	= msm_dai_q6_meta_mi2s_hw_params,
	.set_fmt	= msm_dai_q6_meta_mi2s_set_fmt,
	.shutdown	= msm_dai_q6_meta_mi2s_shutdown,
};

/* Channel min and max are initialized base on platform data */
static struct snd_soc_dai_driver msm_dai_q6_meta_mi2s_dai[] = {
	{
		.playback = {
			.stream_name = "Primary META MI2S Playback",
			.aif_name = "PRI_META_MI2S_RX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE,
			.rate_min =     8000,
			.rate_max =     384000,
		},
		.ops = &msm_dai_q6_meta_mi2s_ops,
		.name = "Primary META MI2S",
		.id = AFE_PORT_ID_PRIMARY_META_MI2S_RX,
		.probe = msm_dai_q6_dai_meta_mi2s_probe,
		.remove = msm_dai_q6_dai_meta_mi2s_remove,
	},
	{
		.playback = {
			.stream_name = "Secondary META MI2S Playback",
			.aif_name = "SEC_META_MI2S_RX",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				 SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |
				 SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
			.rate_min =     8000,
			.rate_max =     192000,
		},
		.ops = &msm_dai_q6_meta_mi2s_ops,
		.name = "Secondary META MI2S",
		.id = AFE_PORT_ID_SECONDARY_META_MI2S_RX,
		.probe = msm_dai_q6_dai_meta_mi2s_probe,
		.remove = msm_dai_q6_dai_meta_mi2s_remove,
	},
};

static int msm_dai_q6_meta_mi2s_platform_data_validation(
	struct platform_device *pdev, struct snd_soc_dai_driver *dai_driver)
{
	struct msm_dai_q6_meta_mi2s_dai_data *dai_data =
		dev_get_drvdata(&pdev->dev);
	struct msm_meta_mi2s_pdata *meta_mi2s_pdata =
		(struct msm_meta_mi2s_pdata *) pdev->dev.platform_data;
	int rc = 0;
	int idx = 0;
	u16 channel_mode = 0;
	unsigned int ch_cnt = 0;
	unsigned int ch_cnt_sum = 0;
	struct afe_param_id_meta_i2s_cfg *port_cfg =
		&dai_data->port_config.meta_i2s;

	if (meta_mi2s_pdata == NULL) {
		pr_err("%s: meta_mi2s_pdata NULL", __func__);
		return -EINVAL;
	}

	dai_data->num_member_ports = meta_mi2s_pdata->num_member_ports;
	for (idx = 0; idx < meta_mi2s_pdata->num_member_ports; idx++) {
		rc = msm_dai_q6_mi2s_get_lineconfig(
			meta_mi2s_pdata->sd_lines[idx],
			&channel_mode,
			&ch_cnt);
		if (rc < 0) {
			dev_err(&pdev->dev, "invalid META MI2S RX sd line config\n");
			goto rtn;
		}
		if (ch_cnt) {
			msm_mi2s_get_port_id(meta_mi2s_pdata->member_port[idx],
				SNDRV_PCM_STREAM_PLAYBACK,
				&dai_data->member_port_id[idx]);
			dai_data->channel_mode[idx] = channel_mode;
			port_cfg->member_port_id[idx] =
				dai_data->member_port_id[idx];
			port_cfg->member_port_channel_mode[idx] = channel_mode;
		}
		ch_cnt_sum += ch_cnt;
	}

	if (ch_cnt_sum) {
		dai_driver->playback.channels_min = 1;
		dai_driver->playback.channels_max = ch_cnt_sum << 1;
	} else {
		dai_driver->playback.channels_min = 0;
		dai_driver->playback.channels_max = 0;
	}

	dev_dbg(&pdev->dev, "%s: sdline 0x%x 0x%x 0x%x 0x%x\n", __func__,
		dai_data->channel_mode[0], dai_data->channel_mode[1],
		dai_data->channel_mode[2], dai_data->channel_mode[3]);
	dev_dbg(&pdev->dev, "%s: playback ch_max %d\n",
		__func__, dai_driver->playback.channels_max);
rtn:
	return rc;
}

static const struct snd_soc_component_driver msm_q6_meta_mi2s_dai_component = {
	.name		= "msm-dai-q6-meta-mi2s",
};

static int msm_dai_q6_meta_mi2s_dev_probe(struct platform_device *pdev)
{
	struct msm_dai_q6_meta_mi2s_dai_data *dai_data;
	const char *q6_meta_mi2s_dev_id = "qcom,msm-dai-q6-meta-mi2s-dev-id";
	u32 dev_id = 0;
	u32 meta_mi2s_intf = 0;
	struct msm_meta_mi2s_pdata *meta_mi2s_pdata;
	int rc;

	rc = of_property_read_u32(pdev->dev.of_node, q6_meta_mi2s_dev_id,
				  &dev_id);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: missing %s in dt node\n", __func__,
			q6_meta_mi2s_dev_id);
		goto rtn;
	}

	dev_dbg(&pdev->dev, "dev name %s dev id 0x%x\n", dev_name(&pdev->dev),
		dev_id);

	switch (dev_id) {
	case AFE_PORT_ID_PRIMARY_META_MI2S_RX:
		meta_mi2s_intf = 0;
		break;
	case AFE_PORT_ID_SECONDARY_META_MI2S_RX:
		meta_mi2s_intf = 1;
		break;
	default:
		dev_err(&pdev->dev,
			"%s: Invalid META MI2S ID 0x%x from Device Tree\n",
			__func__, dev_id);
		rc = -ENXIO;
		goto rtn;
	}

	pdev->id = dev_id;

	meta_mi2s_pdata = kzalloc(sizeof(struct msm_meta_mi2s_pdata),
		GFP_KERNEL);
	if (!meta_mi2s_pdata) {
		rc = -ENOMEM;
		goto rtn;
	}

	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-mi2s-num-members",
		&meta_mi2s_pdata->num_member_ports);
	if (rc) {
		dev_err(&pdev->dev, "%s: invalid num from DT file %s\n",
			__func__, "qcom,msm-mi2s-num-members");
		goto free_pdata;
	}

	if (meta_mi2s_pdata->num_member_ports >
		MAX_NUM_I2S_META_PORT_MEMBER_PORTS) {
		dev_err(&pdev->dev, "%s: num-members %d too large from DT file\n",
			__func__, meta_mi2s_pdata->num_member_ports);
		goto free_pdata;
	}

	rc = of_property_read_u32_array(pdev->dev.of_node,
		"qcom,msm-mi2s-member-id",
		meta_mi2s_pdata->member_port,
		meta_mi2s_pdata->num_member_ports);
	if (rc) {
		dev_err(&pdev->dev, "%s: member-id from DT file %s\n",
			__func__, "qcom,msm-mi2s-member-id");
		goto free_pdata;
	}

	rc = of_property_read_u32_array(pdev->dev.of_node,
		"qcom,msm-mi2s-rx-lines",
		meta_mi2s_pdata->sd_lines,
		meta_mi2s_pdata->num_member_ports);
	if (rc) {
		dev_err(&pdev->dev, "%s: Rx line from DT file %s\n",
			__func__, "qcom,msm-mi2s-rx-lines");
		goto free_pdata;
	}

	dev_dbg(&pdev->dev, "dev name %s num-members=%d\n",
		dev_name(&pdev->dev), meta_mi2s_pdata->num_member_ports);
	dev_dbg(&pdev->dev, "member array (%d, %d, %d, %d)\n",
		meta_mi2s_pdata->member_port[0],
		meta_mi2s_pdata->member_port[1],
		meta_mi2s_pdata->member_port[2],
		meta_mi2s_pdata->member_port[3]);
	dev_dbg(&pdev->dev, "sd-lines array (0x%x, 0x%x, 0x%x, 0x%x)\n",
		meta_mi2s_pdata->sd_lines[0],
		meta_mi2s_pdata->sd_lines[1],
		meta_mi2s_pdata->sd_lines[2],
		meta_mi2s_pdata->sd_lines[3]);

	meta_mi2s_pdata->intf_id = meta_mi2s_intf;

	dai_data = kzalloc(sizeof(struct msm_dai_q6_meta_mi2s_dai_data),
			   GFP_KERNEL);
	if (!dai_data) {
		rc = -ENOMEM;
		goto free_pdata;
	} else
		dev_set_drvdata(&pdev->dev, dai_data);

	pdev->dev.platform_data = meta_mi2s_pdata;

	rc = msm_dai_q6_meta_mi2s_platform_data_validation(pdev,
		&msm_dai_q6_meta_mi2s_dai[meta_mi2s_intf]);
	if (rc < 0)
		goto free_dai_data;

	rc = snd_soc_register_component(&pdev->dev,
		&msm_q6_meta_mi2s_dai_component,
		&msm_dai_q6_meta_mi2s_dai[meta_mi2s_intf], 1);
	if (rc < 0)
		goto err_register;
	return 0;

err_register:
	dev_err(&pdev->dev, "fail to %s\n", __func__);
free_dai_data:
	kfree(dai_data);
free_pdata:
	kfree(meta_mi2s_pdata);
rtn:
	return rc;
}

static int msm_dai_q6_meta_mi2s_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}
#endif
static const struct snd_soc_component_driver msm_dai_q6_component = {
	.name		= "msm-dai-q6-dev",
};

static int msm_dai_q6_dev_probe(struct platform_device *pdev)
{
	int rc, id, i, len;
	const char *q6_dev_id = "qcom,msm-dai-q6-dev-id";
	char stream_name[80];

	rc = of_property_read_u32(pdev->dev.of_node, q6_dev_id, &id);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: missing %s in dt node\n", __func__, q6_dev_id);
		return rc;
	}

	pdev->id = id;

	pr_debug("%s: dev name %s, id:%d\n", __func__,
		 dev_name(&pdev->dev), pdev->id);

	switch (id) {
	case SLIMBUS_0_RX:
		strlcpy(stream_name, "Slimbus Playback", 80);
		goto register_slim_playback;
	case SLIMBUS_6_RX:
		strlcpy(stream_name, "Slimbus6 Playback", 80);
		goto register_slim_playback;
register_slim_playback:
		rc = -ENODEV;
		len = strnlen(stream_name, 80);
		for (i = 0; i < ARRAY_SIZE(msm_dai_q6_slimbus_rx_dai); i++) {
			if (msm_dai_q6_slimbus_rx_dai[i].playback.stream_name &&
				!strcmp(stream_name,
				msm_dai_q6_slimbus_rx_dai[i]
				.playback.stream_name)) {
				rc = snd_soc_register_component(&pdev->dev,
				&msm_dai_q6_component,
				&msm_dai_q6_slimbus_rx_dai[i], 1);
				break;
			}
		}
		if (rc)
			pr_err("%s: Device not found stream name %s\n",
				__func__, stream_name);
		break;
	case SLIMBUS_0_TX:
		strlcpy(stream_name, "Slimbus Capture", 80);
		goto register_slim_capture;
register_slim_capture:
		rc = -ENODEV;
		len = strnlen(stream_name, 80);
		for (i = 0; i < ARRAY_SIZE(msm_dai_q6_slimbus_tx_dai); i++) {
			if (msm_dai_q6_slimbus_tx_dai[i].capture.stream_name &&
				!strcmp(stream_name,
				msm_dai_q6_slimbus_tx_dai[i]
				.capture.stream_name)) {
				rc = snd_soc_register_component(&pdev->dev,
				&msm_dai_q6_component,
				&msm_dai_q6_slimbus_tx_dai[i], 1);
				break;
			}
		}
		if (rc)
			pr_err("%s: Device not found stream name %s\n",
				__func__, stream_name);
		break;
	default:
		rc = -ENODEV;
		break;
	}

	return rc;
}

static int msm_dai_q6_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id msm_dai_q6_dev_dt_match[] = {
	{ .compatible = "qcom,msm-dai-q6-dev", },
	{ }
};
MODULE_DEVICE_TABLE(of, msm_dai_q6_dev_dt_match);

static struct platform_driver msm_dai_q6_dev = {
	.probe  = msm_dai_q6_dev_probe,
	.remove = msm_dai_q6_dev_remove,
	.driver = {
		.name = "msm-dai-q6-dev",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_q6_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

static int msm_dai_q6_probe(struct platform_device *pdev)
{
	int rc;

	pr_debug("%s: dev name %s, id:%d\n", __func__,
		 dev_name(&pdev->dev), pdev->id);
	rc = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (rc) {
		dev_err(&pdev->dev, "%s: failed to add child nodes, rc=%d\n",
			__func__, rc);
	} else
		dev_dbg(&pdev->dev, "%s: added child node\n", __func__);

	return rc;
}

static int msm_dai_q6_remove(struct platform_device *pdev)
{
	of_platform_depopulate(&pdev->dev);
	return 0;
}

static const struct of_device_id msm_dai_q6_dt_match[] = {
	{ .compatible = "qcom,msm-dai-q6", },
	{ }
};
MODULE_DEVICE_TABLE(of, msm_dai_q6_dt_match);
static struct platform_driver msm_dai_q6 = {
	.probe  = msm_dai_q6_probe,
	.remove = msm_dai_q6_remove,
	.driver = {
		.name = "msm-dai-q6",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_q6_dt_match,
		.suppress_bind_attrs = true,
	},
};

static int msm_dai_mi2s_q6_probe(struct platform_device *pdev)
{
	int rc;

	rc = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (rc) {
		dev_err(&pdev->dev, "%s: failed to add child nodes, rc=%d\n",
			__func__, rc);
	} else
		dev_dbg(&pdev->dev, "%s: added child node\n", __func__);
	return rc;
}

static int msm_dai_mi2s_q6_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id msm_dai_mi2s_dt_match[] = {
	{ .compatible = "qcom,msm-dai-mi2s", },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_dai_mi2s_dt_match);

static struct platform_driver msm_dai_mi2s_q6 = {
	.probe  = msm_dai_mi2s_q6_probe,
	.remove = msm_dai_mi2s_q6_remove,
	.driver = {
		.name = "msm-dai-mi2s",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_mi2s_dt_match,
		.suppress_bind_attrs = true,
	},
};

static const struct of_device_id msm_dai_q6_mi2s_dev_dt_match[] = {
	{ .compatible = "qcom,msm-dai-q6-mi2s", },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_dai_q6_mi2s_dev_dt_match);

static struct platform_driver msm_dai_q6_mi2s_driver = {
	.probe  = msm_dai_q6_mi2s_dev_probe,
	.remove  = msm_dai_q6_mi2s_dev_remove,
	.driver = {
		.name = "msm-dai-q6-mi2s",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_q6_mi2s_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

#if 0
static const struct of_device_id msm_dai_q6_meta_mi2s_dev_dt_match[] = {
	{ .compatible = "qcom,msm-dai-q6-meta-mi2s", },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_dai_q6_meta_mi2s_dev_dt_match);

static struct platform_driver msm_dai_q6_meta_mi2s_driver = {
	.probe  = msm_dai_q6_meta_mi2s_dev_probe,
	.remove  = msm_dai_q6_meta_mi2s_dev_remove,
	.driver = {
		.name = "msm-dai-q6-meta-mi2s",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_q6_meta_mi2s_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};
#endif

static int msm_dai_q6_tdm_set_clk_param(u32 group_id,
					struct clk_cfg *clk_set, u32 mode)
{
	switch (group_id) {
	case AFE_GROUP_DEVICE_ID_PRIMARY_TDM_RX:
	case AFE_GROUP_DEVICE_ID_PRIMARY_TDM_TX:
		if (mode)
			clk_set->clk_id = CLOCK_ID_PRI_TDM_IBIT;
		else
			clk_set->clk_id = CLOCK_ID_PRI_TDM_EBIT;
		break;
	case AFE_GROUP_DEVICE_ID_SECONDARY_TDM_RX:
	case AFE_GROUP_DEVICE_ID_SECONDARY_TDM_TX:
		if (mode)
			clk_set->clk_id = CLOCK_ID_SEC_TDM_IBIT;
		else
			clk_set->clk_id = CLOCK_ID_SEC_TDM_EBIT;
		break;
	case AFE_GROUP_DEVICE_ID_TERTIARY_TDM_RX:
	case AFE_GROUP_DEVICE_ID_TERTIARY_TDM_TX:
		if (mode)
			clk_set->clk_id = CLOCK_ID_TER_TDM_IBIT;
		else
			clk_set->clk_id = CLOCK_ID_TER_TDM_EBIT;
		break;
	case AFE_GROUP_DEVICE_ID_QUATERNARY_TDM_RX:
	case AFE_GROUP_DEVICE_ID_QUATERNARY_TDM_TX:
		if (mode)
			clk_set->clk_id = CLOCK_ID_QUAD_TDM_IBIT;
		else
			clk_set->clk_id = CLOCK_ID_QUAD_TDM_EBIT;
		break;
	case AFE_GROUP_DEVICE_ID_QUINARY_TDM_RX:
	case AFE_GROUP_DEVICE_ID_QUINARY_TDM_TX:
		if (mode)
			clk_set->clk_id = CLOCK_ID_QUI_TDM_IBIT;
		else
			clk_set->clk_id = CLOCK_ID_QUI_TDM_EBIT;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int msm_dai_tdm_q6_probe(struct platform_device *pdev)
{
	int rc = 0;
	const uint32_t *port_id_array = NULL;
	uint32_t array_length = 0;
	int i = 0;
	int group_idx = 0;
	u32 clk_mode = 0;

	/* extract tdm group info into static */
	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-group-id",
		(u32 *)&tdm_group_cfg.group_id);
	if (rc) {
		dev_err(&pdev->dev, "%s: Group ID from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-group-id");
		goto rtn;
	}
	dev_dbg(&pdev->dev, "%s: Group ID from DT file 0x%x\n",
		__func__, tdm_group_cfg.group_id);

	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-group-num-ports",
		&num_tdm_group_ports);
	if (rc) {
		dev_err(&pdev->dev, "%s: Group Num Ports from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-group-num-ports");
		goto rtn;
	}
	dev_dbg(&pdev->dev, "%s: Group Num Ports from DT file 0x%x\n",
		__func__, num_tdm_group_ports);

	if (num_tdm_group_ports > AFE_GROUP_DEVICE_NUM_PORTS) {
		dev_err(&pdev->dev, "%s Group Num Ports %d greater than Max %d\n",
			__func__, num_tdm_group_ports,
			AFE_GROUP_DEVICE_NUM_PORTS);
		rc = -EINVAL;
		goto rtn;
	}

	port_id_array = of_get_property(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-group-port-id",
		&array_length);
	if (port_id_array == NULL) {
		dev_err(&pdev->dev, "%s port_id_array is not valid\n",
			__func__);
		rc = -EINVAL;
		goto rtn;
	}
	if (array_length != sizeof(uint32_t) * num_tdm_group_ports) {
		dev_err(&pdev->dev, "%s array_length is %d, expected is %zd\n",
			__func__, array_length,
			sizeof(uint32_t) * num_tdm_group_ports);
		rc = -EINVAL;
		goto rtn;
	}

	for (i = 0; i < num_tdm_group_ports; i++)
		tdm_group_cfg.port_id[i] =
			(u16)be32_to_cpu(port_id_array[i]);
	/* Unused index should be filled with 0 or AFE_PORT_INVALID */
	for (i = num_tdm_group_ports; i < AFE_GROUP_DEVICE_NUM_PORTS; i++)
		tdm_group_cfg.port_id[i] =
			AFE_PORT_INVALID;

	/* extract tdm clk info into static */
	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-clk-rate",
		&tdm_clk_set.clk_freq_in_hz);
	if (rc) {
		dev_err(&pdev->dev, "%s: Clk Rate from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-clk-rate");
		goto rtn;
	}
	dev_dbg(&pdev->dev, "%s: Clk Rate from DT file %d\n",
		__func__, tdm_clk_set.clk_freq_in_hz);
        tdm_clk_set.clk_freq_in_hz = 48000*8*32;
	/* initialize static tdm clk attribute to default value */
        tdm_clk_set.clk_attri = CLOCK_ATTRIBUTE_COUPLE_NO;

	/* extract tdm clk attribute into static */
	if (of_find_property(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-clk-attribute", NULL)) {
		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-clk-attribute",
			&tdm_clk_set.clk_attri);
		if (rc) {
			dev_err(&pdev->dev, "%s: value for clk attribute not found %s\n",
				__func__, "qcom,msm-cpudai-tdm-clk-attribute");
			goto rtn;
		}
		dev_dbg(&pdev->dev, "%s: clk attribute from DT file %d\n",
			__func__, tdm_clk_set.clk_attri);
	} else
		dev_dbg(&pdev->dev, "%s: clk attribute not found\n", __func__);

	/* extract tdm lane cfg to static */
	tdm_lane_cfg.port_id = tdm_group_cfg.group_id;
	tdm_lane_cfg.lane_mask = AFE_LANE_MASK_INVALID;
	if (of_find_property(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-lane-mask", NULL)) {
		rc = of_property_read_u16(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-lane-mask",
			&tdm_lane_cfg.lane_mask);
		if (rc) {
			dev_err(&pdev->dev, "%s: value for tdm lane mask not found %s\n",
				__func__, "qcom,msm-cpudai-tdm-lane-mask");
			goto rtn;
		}
		dev_dbg(&pdev->dev, "%s: tdm lane mask from DT file %d\n",
			__func__, tdm_lane_cfg.lane_mask);
	} else
		dev_dbg(&pdev->dev, "%s: tdm lane mask not found\n", __func__);

	/* extract tdm clk src master/slave info into static */
	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-clk-internal",
		&clk_mode);
	if (rc) {
		dev_err(&pdev->dev, "%s: Clk id from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-clk-internal");
		goto rtn;
	}
	dev_dbg(&pdev->dev, "%s: Clk id from DT file %d\n",
		__func__, clk_mode);

	rc = msm_dai_q6_tdm_set_clk_param(tdm_group_cfg.group_id,
					  &tdm_clk_set, clk_mode);
	if (rc) {
		dev_err(&pdev->dev, "%s: group id not supported 0x%x\n",
			__func__, tdm_group_cfg.group_id);
		goto rtn;
	}

	/* other initializations within device group */
	group_idx = msm_dai_q6_get_group_idx(tdm_group_cfg.group_id);
	if (group_idx < 0) {
		dev_err(&pdev->dev, "%s: group id 0x%x not supported\n",
			__func__, tdm_group_cfg.group_id);
		rc = -EINVAL;
		goto rtn;
	}
	atomic_set(&tdm_group_ref[group_idx], 0);

	/* probe child node info */
	rc = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (rc) {
		dev_err(&pdev->dev, "%s: failed to add child nodes, rc=%d\n",
			__func__, rc);
		goto rtn;
	} else
		dev_dbg(&pdev->dev, "%s: added child node\n", __func__);

rtn:
	return rc;
}

static int msm_dai_tdm_q6_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id msm_dai_tdm_dt_match[] = {
	{ .compatible = "qcom,msm-dai-tdm", },
	{}
};

MODULE_DEVICE_TABLE(of, msm_dai_tdm_dt_match);

static struct platform_driver msm_dai_tdm_q6 = {
	.probe  = msm_dai_tdm_q6_probe,
	.remove = msm_dai_tdm_q6_remove,
	.driver = {
		.name = "msm-dai-tdm",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_tdm_dt_match,
		.suppress_bind_attrs = true,
	},
};

static int msm_dai_q6_tdm_set_clk(
		struct msm_dai_q6_tdm_dai_data *dai_data,
		u16 port_id, bool enable)
{
	int rc = 0;

	rc = audio_prm_set_lpass_clk_cfg(&dai_data->clk_set, enable);
	if (rc < 0)
		pr_err("%s: afe lpass clock failed, err:%d port_id:%d freq:%d clk_id:%d\n",
			__func__, rc, port_id, dai_data->clk_set.clk_freq_in_hz, dai_data->clk_set.clk_id);
	return rc;
}

static int msm_dai_q6_tdm_set_sysclk(struct snd_soc_dai *dai,
				int clk_id, unsigned int freq, int dir)
{
	struct msm_dai_q6_tdm_dai_data *dai_data =
		dev_get_drvdata(dai->dev);

	if ((dai->id >= AFE_PORT_ID_PRIMARY_TDM_RX) &&
		(dai->id <= AFE_PORT_ID_SENARY_TDM_TX_7)) {
		dai_data->clk_set.clk_freq_in_hz = freq;
	} else {
		dev_err(dai->dev, "%s: invalid dai id 0x%x\n",
			__func__, dai->id);
		return -EINVAL;
	}

	dev_dbg(dai->dev, "%s: dai id = 0x%x, group clk_freq = %d\n",
			__func__, dai->id, freq);
	return 0;
}

static int msm_dai_q6_tdm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct msm_dai_q6_tdm_dai_data *dai_data =
		dev_get_drvdata(dai->dev);

	struct afe_param_id_group_device_tdm_cfg *tdm_group =
		&dai_data->group_cfg.tdm_cfg;
	struct afe_param_id_tdm_cfg *tdm =
		&dai_data->port_cfg.tdm;
	struct afe_param_id_slot_mapping_cfg *slot_mapping =
		&dai_data->port_cfg.slot_mapping;
//	struct afe_param_id_slot_mapping_cfg_v2 *slot_mapping_v2 =
//		&dai_data->port_cfg.slot_mapping_v2;
	struct afe_param_id_custom_tdm_header_cfg *custom_tdm_header =
		&dai_data->port_cfg.custom_tdm_header;

	pr_debug("%s: dev_name: %s\n",
		__func__, dev_name(dai->dev));

	if ((params_channels(params) == 0) ||
		(params_channels(params) > 32)) {
		dev_err(dai->dev, "%s: invalid param channels %d\n",
			__func__, params_channels(params));
		return -EINVAL;
	}
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		dai_data->bitwidth = 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S24_3LE:
		dai_data->bitwidth = 24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		dai_data->bitwidth = 32;
		break;
	default:
		dev_err(dai->dev, "%s: invalid param format 0x%x\n",
			__func__, params_format(params));
		return -EINVAL;
	}
	dai_data->channels = params_channels(params);
	dai_data->rate = params_rate(params);
	dai_data->bitwidth = 16;

	/*
	 * update tdm group config param
	 * NOTE: group config is set to the same as slot config.
	 */
	tdm_group->bit_width = tdm_group->slot_width;

	/*
	 * for multi lane scenario
	 * Total number of active channels = number of active lanes * number of active slots.
	 */
	if (dai_data->lane_cfg.lane_mask != AFE_LANE_MASK_INVALID)
		tdm_group->num_channels = tdm_group->nslots_per_frame
			* num_of_bits_set(dai_data->lane_cfg.lane_mask);
	else
		tdm_group->num_channels = tdm_group->nslots_per_frame;

	tdm_group->sample_rate = dai_data->rate;

	pr_debug("%s: TDM GROUP:\n"
		"num_channels=%d sample_rate=%d bit_width=%d\n"
		"nslots_per_frame=%d slot_width=%d slot_mask=0x%x\n",
		__func__,
		tdm_group->num_channels,
		tdm_group->sample_rate,
		tdm_group->bit_width,
		tdm_group->nslots_per_frame,
		tdm_group->slot_width,
		tdm_group->slot_mask);
	pr_debug("%s: TDM GROUP:\n"
		"port_id[0]=0x%x port_id[1]=0x%x port_id[2]=0x%x port_id[3]=0x%x\n"
		"port_id[4]=0x%x port_id[5]=0x%x port_id[6]=0x%x port_id[7]=0x%x\n",
		__func__,
		tdm_group->port_id[0],
		tdm_group->port_id[1],
		tdm_group->port_id[2],
		tdm_group->port_id[3],
		tdm_group->port_id[4],
		tdm_group->port_id[5],
		tdm_group->port_id[6],
		tdm_group->port_id[7]);
	pr_debug("%s: TDM GROUP ID 0x%x lane mask 0x%x:\n",
		__func__,
		tdm_group->group_id,
		dai_data->lane_cfg.lane_mask);

	/*
	 * update tdm config param
	 * NOTE: channels/rate/bitwidth are per stream property
	 */
	tdm->num_channels = dai_data->channels;
	tdm->sample_rate = dai_data->rate;
	tdm->bit_width = dai_data->bitwidth;
	/*
	 * port slot config is the same as group slot config
	 * port slot mask should be set according to offset
	 */
	tdm->nslots_per_frame = tdm_group->nslots_per_frame;
	tdm->slot_width = tdm_group->slot_width;
	#if 0
	if (q6core_get_avcs_api_version_per_service(
		APRV2_IDS_SERVICE_ID_ADSP_AFE_V) >= AFE_API_VERSION_V3)
		tdm->slot_mask = tdm_param_set_slot_mask(
					slot_mapping_v2->offset,
					tdm_group->slot_width,
					tdm_group->nslots_per_frame);
	else
	#endif
	tdm->slot_mask = tdm_group->slot_mask;

	pr_debug("%s: TDM:\n"
		"num_channels=%d sample_rate=%d bit_width=%d\n"
		"nslots_per_frame=%d slot_width=%d slot_mask=0x%x\n"
		"data_format=0x%x sync_mode=0x%x sync_src=0x%x\n"
		"data_out=0x%x invert_sync=0x%x data_delay=0x%x\n",
		__func__,
		tdm->num_channels,
		tdm->sample_rate,
		tdm->bit_width,
		tdm->nslots_per_frame,
		tdm->slot_width,
		tdm->slot_mask,
		tdm->data_format,
		tdm->sync_mode,
		tdm->sync_src,
		tdm->ctrl_data_out_enable,
		tdm->ctrl_invert_sync_pulse,
		tdm->ctrl_sync_data_delay);
	#if 0
	if (q6core_get_avcs_api_version_per_service(
		APRV2_IDS_SERVICE_ID_ADSP_AFE_V) >= AFE_API_VERSION_V3) {
		/*
		 * update slot mapping v2 config param
		 * NOTE: channels/rate/bitwidth are per stream property
		 */
		slot_mapping_v2->bitwidth = dai_data->bitwidth;

	pr_debug("%s: SLOT MAPPING_V2:\n"
		"num_channel=%d bitwidth=%d data_align=0x%x\n",
		__func__,
		slot_mapping_v2->num_channel,
		slot_mapping_v2->bitwidth,
		slot_mapping_v2->data_align_type);
	pr_debug("%s: SLOT MAPPING V2:\n"
		"offset[0]=0x%x offset[1]=0x%x offset[2]=0x%x offset[3]=0x%x\n"
		"offset[4]=0x%x offset[5]=0x%x offset[6]=0x%x offset[7]=0x%x\n"
		"offset[8]=0x%x offset[9]=0x%x offset[10]=0x%x offset[11]=0x%x\n"
		"offset[12]=0x%x offset[13]=0x%x offset[14]=0x%x offset[15]=0x%x\n"
		"offset[16]=0x%x offset[17]=0x%x offset[18]=0x%x offset[19]=0x%x\n"
		"offset[20]=0x%x offset[21]=0x%x offset[22]=0x%x offset[23]=0x%x\n"
		"offset[24]=0x%x offset[25]=0x%x offset[26]=0x%x offset[27]=0x%x\n"
		"offset[28]=0x%x offset[29]=0x%x offset[30]=0x%x offset[31]=0x%x\n",
		__func__,
		slot_mapping_v2->offset[0],
		slot_mapping_v2->offset[1],
		slot_mapping_v2->offset[2],
		slot_mapping_v2->offset[3],
		slot_mapping_v2->offset[4],
		slot_mapping_v2->offset[5],
		slot_mapping_v2->offset[6],
		slot_mapping_v2->offset[7],
		slot_mapping_v2->offset[8],
		slot_mapping_v2->offset[9],
		slot_mapping_v2->offset[10],
		slot_mapping_v2->offset[11],
		slot_mapping_v2->offset[12],
		slot_mapping_v2->offset[13],
		slot_mapping_v2->offset[14],
		slot_mapping_v2->offset[15],
		slot_mapping_v2->offset[16],
		slot_mapping_v2->offset[17],
		slot_mapping_v2->offset[18],
		slot_mapping_v2->offset[19],
		slot_mapping_v2->offset[20],
		slot_mapping_v2->offset[21],
		slot_mapping_v2->offset[22],
		slot_mapping_v2->offset[23],
		slot_mapping_v2->offset[24],
		slot_mapping_v2->offset[25],
		slot_mapping_v2->offset[26],
		slot_mapping_v2->offset[27],
		slot_mapping_v2->offset[28],
		slot_mapping_v2->offset[29],
		slot_mapping_v2->offset[30],
		slot_mapping_v2->offset[31]);
	} else {
	#endif
		/*
		 * update slot mapping config param
		 * NOTE: channels/rate/bitwidth are per stream property
		 */
		slot_mapping->bitwidth = dai_data->bitwidth;

		pr_debug("%s: SLOT MAPPING:\n"
			"num_channel=%d bitwidth=%d data_align=0x%x\n",
			__func__,
			slot_mapping->num_channel,
			slot_mapping->bitwidth,
			slot_mapping->data_align_type);
		pr_debug("%s: SLOT MAPPING:\n"
			"offset[0]=0x%x offset[1]=0x%x offset[2]=0x%x offset[3]=0x%x\n"
			"offset[4]=0x%x offset[5]=0x%x offset[6]=0x%x offset[7]=0x%x\n",
			__func__,
			slot_mapping->offset[0],
			slot_mapping->offset[1],
			slot_mapping->offset[2],
			slot_mapping->offset[3],
			slot_mapping->offset[4],
			slot_mapping->offset[5],
			slot_mapping->offset[6],
			slot_mapping->offset[7]);
//	}
	/*
	 * update custom header config param
	 * NOTE: channels/rate/bitwidth are per playback stream property.
	 * custom tdm header only applicable to playback stream.
	 */
	if (custom_tdm_header->header_type !=
		AFE_CUSTOM_TDM_HEADER_TYPE_INVALID) {
		pr_debug("%s: CUSTOM TDM HEADER:\n"
			"start_offset=0x%x header_width=%d\n"
			"num_frame_repeat=%d header_type=0x%x\n",
			__func__,
			custom_tdm_header->start_offset,
			custom_tdm_header->header_width,
			custom_tdm_header->num_frame_repeat,
			custom_tdm_header->header_type);
		pr_debug("%s: CUSTOM TDM HEADER:\n"
			"header[0]=0x%x header[1]=0x%x header[2]=0x%x header[3]=0x%x\n"
			"header[4]=0x%x header[5]=0x%x header[6]=0x%x header[7]=0x%x\n",
			__func__,
			custom_tdm_header->header[0],
			custom_tdm_header->header[1],
			custom_tdm_header->header[2],
			custom_tdm_header->header[3],
			custom_tdm_header->header[4],
			custom_tdm_header->header[5],
			custom_tdm_header->header[6],
			custom_tdm_header->header[7]);
	}

	return 0;
}

static int msm_dai_q6_tdm_prepare(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	int rc = 0;
	struct msm_dai_q6_tdm_dai_data *dai_data =
		dev_get_drvdata(dai->dev);
	u16 group_id = dai_data->group_cfg.tdm_cfg.group_id;
	int group_idx = 0;
	atomic_t *group_ref = NULL;

	dev_dbg(dai->dev, "%s: dev_name: %s dev_id: 0x%x group_id: 0x%x\n",
		 __func__, dev_name(dai->dev), dai->dev->id, group_id);

	if (dai_data->port_cfg.custom_tdm_header.minor_version == 0)
		dev_dbg(dai->dev,
			 "%s: Custom tdm header not supported\n", __func__);

	group_idx = msm_dai_q6_get_group_idx(dai->id);
	if (group_idx < 0) {
		dev_err(dai->dev, "%s port id 0x%x not supported\n",
			__func__, dai->id);
		return -EINVAL;
	}

	mutex_lock(&tdm_mutex);

	group_ref = &tdm_group_ref[group_idx];

	if (!test_bit(STATUS_PORT_STARTED, dai_data->status_mask)) {

		if (msm_dai_q6_get_tdm_clk_ref(group_idx) == 0) {
			/* TX and RX share the same clk. So enable the clk
			 * per TDM interface. */
			rc = msm_dai_q6_tdm_set_clk(dai_data,
				dai->id, true);
			if (rc < 0) {
				dev_err(dai->dev, "%s: fail to enable AFE clk 0x%x\n",
					__func__, dai->id);
				goto rtn;
			}
		}
		set_bit(STATUS_PORT_STARTED,
			dai_data->status_mask);
		atomic_inc(group_ref);
	}

rtn:
	mutex_unlock(&tdm_mutex);
	return rc;
}

static void msm_dai_q6_tdm_shutdown(struct snd_pcm_substream *substream,
				     struct snd_soc_dai *dai)
{
	int rc = 0;
	struct msm_dai_q6_tdm_dai_data *dai_data =
		dev_get_drvdata(dai->dev);
	int group_idx = 0;
	atomic_t *group_ref = NULL;

	group_idx = msm_dai_q6_get_group_idx(dai->id);
	if (group_idx < 0) {
		dev_err(dai->dev, "%s port id 0x%x not supported\n",
			__func__, dai->id);
		return;
	}

	mutex_lock(&tdm_mutex);

	group_ref = &tdm_group_ref[group_idx];

	if (test_bit(STATUS_PORT_STARTED, dai_data->status_mask)) {
		atomic_dec(group_ref);
		clear_bit(STATUS_PORT_STARTED,
			dai_data->status_mask);

		if (msm_dai_q6_get_tdm_clk_ref(group_idx) == 0) {
			rc = msm_dai_q6_tdm_set_clk(dai_data,
				dai->id, false);
			if (rc < 0) {
				dev_err(dai->dev, "%s: fail to disable AFE clk 0x%x\n",
					__func__, dai->id);
			}
		}

		/* TODO: need to monitor PCM/MI2S/TDM HW status */
		/* NOTE: AFE should error out if HW resource contention */

	}

	mutex_unlock(&tdm_mutex);
}

static struct snd_soc_dai_ops msm_dai_q6_tdm_ops = {
	.prepare          = msm_dai_q6_tdm_prepare,
	.hw_params        = msm_dai_q6_tdm_hw_params,
	.set_sysclk       = msm_dai_q6_tdm_set_sysclk,
	.shutdown         = msm_dai_q6_tdm_shutdown,
};

static struct snd_soc_dai_driver msm_dai_q6_tdm_dai[] = {
	{
		.playback = {
			.stream_name = "Primary TDM0 Playback",
			.aif_name = "PRI_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX,
	},
	{
		.playback = {
			.stream_name = "Primary TDM1 Playback",
			.aif_name = "PRI_TDM_RX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_1,
	},
	{
		.playback = {
			.stream_name = "Primary TDM2 Playback",
			.aif_name = "PRI_TDM_RX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_2,
	},
	{
		.playback = {
			.stream_name = "Primary TDM3 Playback",
			.aif_name = "PRI_TDM_RX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_3,
	},
	{
		.playback = {
			.stream_name = "Primary TDM4 Playback",
			.aif_name = "PRI_TDM_RX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_4,
	},
	{
		.playback = {
			.stream_name = "Primary TDM5 Playback",
			.aif_name = "PRI_TDM_RX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_5,
	},
	{
		.playback = {
			.stream_name = "Primary TDM6 Playback",
			.aif_name = "PRI_TDM_RX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_6,
	},
	{
		.playback = {
			.stream_name = "Primary TDM7 Playback",
			.aif_name = "PRI_TDM_RX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_RX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_RX_7,
	},
	{
		.capture = {
			.stream_name = "Primary TDM0 Capture",
			.aif_name = "PRI_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX,
	},
	{
		.capture = {
			.stream_name = "Primary TDM1 Capture",
			.aif_name = "PRI_TDM_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_1,
	},
	{
		.capture = {
			.stream_name = "Primary TDM2 Capture",
			.aif_name = "PRI_TDM_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_2,
	},
	{
		.capture = {
			.stream_name = "Primary TDM3 Capture",
			.aif_name = "PRI_TDM_TX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_3,
	},
	{
		.capture = {
			.stream_name = "Primary TDM4 Capture",
			.aif_name = "PRI_TDM_TX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_4,
	},
	{
		.capture = {
			.stream_name = "Primary TDM5 Capture",
			.aif_name = "PRI_TDM_TX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_5,
	},
	{
		.capture = {
			.stream_name = "Primary TDM6 Capture",
			.aif_name = "PRI_TDM_TX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_6,
	},
	{
		.capture = {
			.stream_name = "Primary TDM7 Capture",
			.aif_name = "PRI_TDM_TX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "PRI_TDM_TX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_PRIMARY_TDM_TX_7,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM0 Playback",
			.aif_name = "SEC_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM1 Playback",
			.aif_name = "SEC_TDM_RX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_1,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM2 Playback",
			.aif_name = "SEC_TDM_RX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_2,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM3 Playback",
			.aif_name = "SEC_TDM_RX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_3,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM4 Playback",
			.aif_name = "SEC_TDM_RX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_4,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM5 Playback",
			.aif_name = "SEC_TDM_RX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_5,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM6 Playback",
			.aif_name = "SEC_TDM_RX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_6,
	},
	{
		.playback = {
			.stream_name = "Secondary TDM7 Playback",
			.aif_name = "SEC_TDM_RX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_RX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_RX_7,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM0 Capture",
			.aif_name = "SEC_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM1 Capture",
			.aif_name = "SEC_TDM_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_1,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM2 Capture",
			.aif_name = "SEC_TDM_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_2,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM3 Capture",
			.aif_name = "SEC_TDM_TX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_3,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM4 Capture",
			.aif_name = "SEC_TDM_TX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_4,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM5 Capture",
			.aif_name = "SEC_TDM_TX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_5,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM6 Capture",
			.aif_name = "SEC_TDM_TX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_6,
	},
	{
		.capture = {
			.stream_name = "Secondary TDM7 Capture",
			.aif_name = "SEC_TDM_TX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEC_TDM_TX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SECONDARY_TDM_TX_7,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM0 Playback",
			.aif_name = "TERT_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM1 Playback",
			.aif_name = "TERT_TDM_RX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_1,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM2 Playback",
			.aif_name = "TERT_TDM_RX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_2,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM3 Playback",
			.aif_name = "TERT_TDM_RX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_3,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM4 Playback",
			.aif_name = "TERT_TDM_RX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_4,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM5 Playback",
			.aif_name = "TERT_TDM_RX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_5,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM6 Playback",
			.aif_name = "TERT_TDM_RX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_6,
	},
	{
		.playback = {
			.stream_name = "Tertiary TDM7 Playback",
			.aif_name = "TERT_TDM_RX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_RX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_RX_7,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM0 Capture",
			.aif_name = "TERT_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM1 Capture",
			.aif_name = "TERT_TDM_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_1,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM2 Capture",
			.aif_name = "TERT_TDM_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_2,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM3 Capture",
			.aif_name = "TERT_TDM_TX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_3,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM4 Capture",
			.aif_name = "TERT_TDM_TX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_4,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM5 Capture",
			.aif_name = "TERT_TDM_TX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_5,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM6 Capture",
			.aif_name = "TERT_TDM_TX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_6,
	},
	{
		.capture = {
			.stream_name = "Tertiary TDM7 Capture",
			.aif_name = "TERT_TDM_TX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "TERT_TDM_TX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_TERTIARY_TDM_TX_7,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM0 Playback",
			.aif_name = "QUAT_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM1 Playback",
			.aif_name = "QUAT_TDM_RX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_1,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM2 Playback",
			.aif_name = "QUAT_TDM_RX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_2,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM3 Playback",
			.aif_name = "QUAT_TDM_RX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_3,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM4 Playback",
			.aif_name = "QUAT_TDM_RX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_4,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM5 Playback",
			.aif_name = "QUAT_TDM_RX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_5,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM6 Playback",
			.aif_name = "QUAT_TDM_RX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_6,
	},
	{
		.playback = {
			.stream_name = "Quaternary TDM7 Playback",
			.aif_name = "QUAT_TDM_RX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_RX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_RX_7,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM0 Capture",
			.aif_name = "QUAT_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM1 Capture",
			.aif_name = "QUAT_TDM_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_1,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM2 Capture",
			.aif_name = "QUAT_TDM_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_2,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM3 Capture",
			.aif_name = "QUAT_TDM_TX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_3,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM4 Capture",
			.aif_name = "QUAT_TDM_TX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_4,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM5 Capture",
			.aif_name = "QUAT_TDM_TX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_5,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM6 Capture",
			.aif_name = "QUAT_TDM_TX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_6,
	},
	{
		.capture = {
			.stream_name = "Quaternary TDM7 Capture",
			.aif_name = "QUAT_TDM_TX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUAT_TDM_TX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUATERNARY_TDM_TX_7,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM0 Playback",
			.aif_name = "QUIN_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM1 Playback",
			.aif_name = "QUIN_TDM_RX_1",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_1,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM2 Playback",
			.aif_name = "QUIN_TDM_RX_2",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_2,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM3 Playback",
			.aif_name = "QUIN_TDM_RX_3",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_3,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM4 Playback",
			.aif_name = "QUIN_TDM_RX_4",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_4,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM5 Playback",
			.aif_name = "QUIN_TDM_RX_5",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_5,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM6 Playback",
			.aif_name = "QUIN_TDM_RX_6",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_6,
	},
	{
		.playback = {
			.stream_name = "Quinary TDM7 Playback",
			.aif_name = "QUIN_TDM_RX_7",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_RX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_RX_7,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM0 Capture",
			.aif_name = "QUIN_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM1 Capture",
			.aif_name = "QUIN_TDM_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_1,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM2 Capture",
			.aif_name = "QUIN_TDM_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_2,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM3 Capture",
			.aif_name = "QUIN_TDM_TX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_3,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM4 Capture",
			.aif_name = "QUIN_TDM_TX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_4,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM5 Capture",
			.aif_name = "QUIN_TDM_TX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_5,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM6 Capture",
			.aif_name = "QUIN_TDM_TX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_6,
	},
	{
		.capture = {
			.stream_name = "Quinary TDM7 Capture",
			.aif_name = "QUIN_TDM_TX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 16,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "QUIN_TDM_TX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_QUINARY_TDM_TX_7,
	},
	{
		.playback = {
			.stream_name = "Senary TDM0 Playback",
			.aif_name = "SEN_TDM_RX_0",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX,
	},
	{
		.playback = {
			.stream_name = "Senary TDM1 Playback",
			.aif_name = "SEN_TDM_RX_1",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_1,
	},
	{
		.playback = {
			.stream_name = "Senary TDM2 Playback",
			.aif_name = "SEN_TDM_RX_2",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_2,
	},
	{
		.playback = {
			.stream_name = "Senary TDM3 Playback",
			.aif_name = "SEN_TDM_RX_3",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_3,
	},
	{
		.playback = {
			.stream_name = "Senary TDM4 Playback",
			.aif_name = "SEN_TDM_RX_4",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_4,
	},
	{
		.playback = {
			.stream_name = "Senary TDM5 Playback",
			.aif_name = "SEN_TDM_RX_5",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_5,
	},
	{
		.playback = {
			.stream_name = "Senary TDM6 Playback",
			.aif_name = "SEN_TDM_RX_6",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_6,
	},
	{
		.playback = {
			.stream_name = "Senary TDM7 Playback",
			.aif_name = "SEN_TDM_RX_7",
			.rates = SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_RX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_RX_7,
	},
	{
		.capture = {
			.stream_name = "Senary TDM0 Capture",
			.aif_name = "SEN_TDM_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_0",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX,
	},
	{
		.capture = {
			.stream_name = "Senary TDM1 Capture",
			.aif_name = "SEN_TDM_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_1",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_1,
	},
	{
		.capture = {
			.stream_name = "Senary TDM2 Capture",
			.aif_name = "SEN_TDM_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_2",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_2,
	},
	{
		.capture = {
			.stream_name = "Senary TDM3 Capture",
			.aif_name = "SEN_TDM_TX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_3",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_3,
	},
	{
		.capture = {
			.stream_name = "Senary TDM4 Capture",
			.aif_name = "SEN_TDM_TX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_4",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_4,
	},
	{
		.capture = {
			.stream_name = "Senary TDM5 Capture",
			.aif_name = "SEN_TDM_TX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_5",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_5,
	},
	{
		.capture = {
			.stream_name = "Senary TDM6 Capture",
			.aif_name = "SEN_TDM_TX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_6",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_6,
	},
	{
		.capture = {
			.stream_name = "Senary TDM7 Capture",
			.aif_name = "SEN_TDM_TX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_352800,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 352800,
		},
		.name = "SEN_TDM_TX_7",
		.ops = &msm_dai_q6_tdm_ops,
		.id = AFE_PORT_ID_SENARY_TDM_TX_7,
	},
};

static const struct snd_soc_component_driver msm_q6_tdm_dai_component = {
	.name		= "msm-dai-q6-tdm",
};

static int msm_dai_q6_tdm_dev_probe(struct platform_device *pdev)
{
	struct msm_dai_q6_tdm_dai_data *dai_data = NULL;
	struct afe_param_id_custom_tdm_header_cfg *custom_tdm_header = NULL;
	int rc = 0;
	u32 tdm_dev_id = 0;
	int port_idx = 0;
	struct device_node *tdm_parent_node = NULL;

	/* retrieve device/afe id */
	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-dev-id",
		&tdm_dev_id);
	if (rc) {
		dev_err(&pdev->dev, "%s: Device ID missing in DT file\n",
			__func__);
		goto rtn;
	}
	if ((tdm_dev_id < AFE_PORT_ID_TDM_PORT_RANGE_START) ||
		(tdm_dev_id > AFE_PORT_ID_TDM_PORT_RANGE_END)) {
		dev_err(&pdev->dev, "%s: Invalid TDM Device ID 0x%x in DT file\n",
			__func__, tdm_dev_id);
		rc = -ENXIO;
		goto rtn;
	}
	pdev->id = tdm_dev_id;

	dai_data = kzalloc(sizeof(struct msm_dai_q6_tdm_dai_data),
				GFP_KERNEL);
	if (!dai_data) {
		rc = -ENOMEM;
		dev_err(&pdev->dev,
			"%s Failed to allocate memory for tdm dai_data\n",
			__func__);
		goto rtn;
	}
	memset(dai_data, 0, sizeof(*dai_data));

	rc = of_property_read_u32(pdev->dev.of_node,
				    "qcom,msm-dai-is-island-supported",
				    &dai_data->is_island_dai);
	if (rc)
		dev_dbg(&pdev->dev, "island supported entry not found\n");

	/* TDM CFG */
	tdm_parent_node = of_get_parent(pdev->dev.of_node);
	rc = of_property_read_u32(tdm_parent_node,
		"qcom,msm-cpudai-tdm-sync-mode",
		(u32 *)&dai_data->port_cfg.tdm.sync_mode);
	if (rc) {
		dev_err(&pdev->dev, "%s: Sync Mode from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-sync-mode");
		goto free_dai_data;
	}
	dev_dbg(&pdev->dev, "%s: Sync Mode from DT file 0x%x\n",
		__func__, dai_data->port_cfg.tdm.sync_mode);

	rc = of_property_read_u32(tdm_parent_node,
		"qcom,msm-cpudai-tdm-sync-src",
		(u32 *)&dai_data->port_cfg.tdm.sync_src);
	if (rc) {
		dev_err(&pdev->dev, "%s: Sync Src from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-sync-src");
		goto free_dai_data;
	}
	dev_dbg(&pdev->dev, "%s: Sync Src from DT file 0x%x\n",
		__func__, dai_data->port_cfg.tdm.sync_src);

	rc = of_property_read_u32(tdm_parent_node,
		"qcom,msm-cpudai-tdm-data-out",
		(u32 *)&dai_data->port_cfg.tdm.ctrl_data_out_enable);
	if (rc) {
		dev_err(&pdev->dev, "%s: Data Out from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-data-out");
		goto free_dai_data;
	}
	dev_dbg(&pdev->dev, "%s: Data Out from DT file 0x%x\n",
		__func__, dai_data->port_cfg.tdm.ctrl_data_out_enable);

	rc = of_property_read_u32(tdm_parent_node,
		"qcom,msm-cpudai-tdm-invert-sync",
		(u32 *)&dai_data->port_cfg.tdm.ctrl_invert_sync_pulse);
	if (rc) {
		dev_err(&pdev->dev, "%s: Invert Sync from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-invert-sync");
		goto free_dai_data;
	}
	dev_dbg(&pdev->dev, "%s: Invert Sync from DT file 0x%x\n",
		__func__, dai_data->port_cfg.tdm.ctrl_invert_sync_pulse);

	rc = of_property_read_u32(tdm_parent_node,
		"qcom,msm-cpudai-tdm-data-delay",
		(u32 *)&dai_data->port_cfg.tdm.ctrl_sync_data_delay);
	if (rc) {
		dev_err(&pdev->dev, "%s: Data Delay from DT file %s\n",
			__func__, "qcom,msm-cpudai-tdm-data-delay");
		goto free_dai_data;
	}
	dev_dbg(&pdev->dev, "%s: Data Delay from DT file 0x%x\n",
		__func__, dai_data->port_cfg.tdm.ctrl_sync_data_delay);

	/* TDM CFG -- set default */
	dai_data->port_cfg.tdm.data_format = AFE_LINEAR_PCM_DATA;
	dai_data->port_cfg.tdm.tdm_cfg_minor_version =
		AFE_API_VERSION_TDM_CONFIG;

	/* TDM SLOT MAPPING CFG */
	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,msm-cpudai-tdm-data-align",
		&dai_data->port_cfg.slot_mapping.data_align_type);
	if (rc) {
		dev_err(&pdev->dev, "%s: Data Align from DT file %s\n",
			__func__,
			"qcom,msm-cpudai-tdm-data-align");
		goto free_dai_data;
	}
	dev_dbg(&pdev->dev, "%s: Data Align from DT file 0x%x\n",
		__func__, dai_data->port_cfg.slot_mapping.data_align_type);

	/* TDM SLOT MAPPING CFG -- set default */
	dai_data->port_cfg.slot_mapping.minor_version =
		AFE_API_VERSION_SLOT_MAPPING_CONFIG;
	dai_data->port_cfg.slot_mapping_v2.minor_version =
		AFE_API_VERSION_SLOT_MAPPING_CONFIG_V2;

	/* CUSTOM TDM HEADER CFG */
	custom_tdm_header = &dai_data->port_cfg.custom_tdm_header;
	if (of_find_property(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-header-start-offset", NULL) &&
		of_find_property(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-header-width", NULL) &&
		of_find_property(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-header-num-frame-repeat", NULL)) {
		/* if the property exist */
		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-header-start-offset",
			(u32 *)&custom_tdm_header->start_offset);
		if (rc) {
			dev_err(&pdev->dev, "%s: Header Start Offset from DT file %s\n",
				__func__,
				"qcom,msm-cpudai-tdm-header-start-offset");
			goto free_dai_data;
		}
		dev_dbg(&pdev->dev, "%s: Header Start Offset from DT file 0x%x\n",
			__func__, custom_tdm_header->start_offset);

		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-header-width",
			(u32 *)&custom_tdm_header->header_width);
		if (rc) {
			dev_err(&pdev->dev, "%s: Header Width from DT file %s\n",
				__func__, "qcom,msm-cpudai-tdm-header-width");
			goto free_dai_data;
		}
		dev_dbg(&pdev->dev, "%s: Header Width from DT file 0x%x\n",
			__func__, custom_tdm_header->header_width);

		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,msm-cpudai-tdm-header-num-frame-repeat",
			(u32 *)&custom_tdm_header->num_frame_repeat);
		if (rc) {
			dev_err(&pdev->dev, "%s: Header Num Frame Repeat from DT file %s\n",
				__func__,
				"qcom,msm-cpudai-tdm-header-num-frame-repeat");
			goto free_dai_data;
		}
		dev_dbg(&pdev->dev, "%s: Header Num Frame Repeat from DT file 0x%x\n",
			__func__, custom_tdm_header->num_frame_repeat);

		/* CUSTOM TDM HEADER CFG -- set default */
		custom_tdm_header->minor_version =
			AFE_API_VERSION_CUSTOM_TDM_HEADER_CONFIG;
		custom_tdm_header->header_type =
			AFE_CUSTOM_TDM_HEADER_TYPE_INVALID;
	} else {
		/* CUSTOM TDM HEADER CFG -- set default */
		custom_tdm_header->header_type =
			AFE_CUSTOM_TDM_HEADER_TYPE_INVALID;
		/* proceed with probe */
	}

	/* copy static clk per parent node */
	dai_data->clk_set = tdm_clk_set;
	/* copy static group cfg per parent node */
	dai_data->group_cfg.tdm_cfg = tdm_group_cfg;
	/* copy static num group ports per parent node */
	dai_data->num_group_ports = num_tdm_group_ports;
	dai_data->lane_cfg = tdm_lane_cfg;

	dev_set_drvdata(&pdev->dev, dai_data);

	port_idx = msm_dai_q6_get_port_idx(tdm_dev_id);
	if (port_idx < 0) {
		dev_err(&pdev->dev, "%s Port id 0x%x not supported\n",
			__func__, tdm_dev_id);
		rc = -EINVAL;
		goto free_dai_data;
	}

	rc = snd_soc_register_component(&pdev->dev,
		&msm_q6_tdm_dai_component,
		&msm_dai_q6_tdm_dai[port_idx], 1);

	if (rc) {
		dev_err(&pdev->dev, "%s: TDM dai 0x%x register failed, rc=%d\n",
			__func__, tdm_dev_id, rc);
		goto err_register;
	}

	return 0;

err_register:
free_dai_data:
	kfree(dai_data);
rtn:
	return rc;
}

static int msm_dai_q6_tdm_dev_remove(struct platform_device *pdev)
{
	struct msm_dai_q6_tdm_dai_data *dai_data =
		dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_component(&pdev->dev);

	kfree(dai_data);

	return 0;
}

static const struct of_device_id msm_dai_q6_tdm_dev_dt_match[] = {
	{ .compatible = "qcom,msm-dai-q6-tdm", },
	{}
};

MODULE_DEVICE_TABLE(of, msm_dai_q6_tdm_dev_dt_match);

static struct platform_driver msm_dai_q6_tdm_driver = {
	.probe  = msm_dai_q6_tdm_dev_probe,
	.remove  = msm_dai_q6_tdm_dev_remove,
	.driver = {
		.name = "msm-dai-q6-tdm",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_q6_tdm_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

static struct snd_soc_dai_driver msm_dai_q6_cdc_dma_dai[] = {
	{
		.playback = {
			.stream_name = "WSA CDC DMA0 Playback",
			.aif_name = "WSA_CDC_DMA_RX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "WSA_CDC_DMA_RX_0",
		.id = AFE_PORT_ID_WSA_CODEC_DMA_RX_0,
	},
	{
		.capture = {
			.stream_name = "WSA CDC DMA0 Capture",
			.aif_name = "WSA_CDC_DMA_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "WSA_CDC_DMA_TX_0",
		.id = AFE_PORT_ID_WSA_CODEC_DMA_TX_0,
	},
	{
		.playback = {
			.stream_name = "WSA CDC DMA1 Playback",
			.aif_name = "WSA_CDC_DMA_RX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "WSA_CDC_DMA_RX_1",
		.id = AFE_PORT_ID_WSA_CODEC_DMA_RX_1,
	},
	{
		.capture = {
			.stream_name = "WSA CDC DMA1 Capture",
			.aif_name = "WSA_CDC_DMA_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "WSA_CDC_DMA_TX_1",
		.id = AFE_PORT_ID_WSA_CODEC_DMA_TX_1,
	},
	{
		.capture = {
			.stream_name = "WSA CDC DMA2 Capture",
			.aif_name = "WSA_CDC_DMA_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "WSA_CDC_DMA_TX_2",
		.id = AFE_PORT_ID_WSA_CODEC_DMA_TX_2,
	},
	{
		.capture = {
			.stream_name = "VA CDC DMA0 Capture",
			.aif_name = "VA_CDC_DMA_TX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "VA_CDC_DMA_TX_0",
		.id = AFE_PORT_ID_VA_CODEC_DMA_TX_0,
	},
	{
		.capture = {
			.stream_name = "VA CDC DMA1 Capture",
			.aif_name = "VA_CDC_DMA_TX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "VA_CDC_DMA_TX_1",
		.id = AFE_PORT_ID_VA_CODEC_DMA_TX_1,
	},
	{
		.capture = {
			.stream_name = "VA CDC DMA2 Capture",
			.aif_name = "VA_CDC_DMA_TX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "VA_CDC_DMA_TX_2",
		.id = AFE_PORT_ID_VA_CODEC_DMA_TX_2,
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA0 Playback",
			.aif_name = "RX_CDC_DMA_RX_0",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "RX_CDC_DMA_RX_0",
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_0,
	},
	{
		.capture = {
			.stream_name = "TX CDC DMA0 Capture",
			.aif_name = "TX_CDC_DMA_TX_0",
			.rates = SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_192000 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 3,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.name = "TX_CDC_DMA_TX_0",
		.id = AFE_PORT_ID_TX_CODEC_DMA_TX_0,
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA1 Playback",
			.aif_name = "RX_CDC_DMA_RX_1",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_1,
		.name = "RX_CDC_DMA_RX_1",
	},
	{
		.capture = {
			.stream_name = "TX CDC DMA1 Capture",
			.aif_name = "TX_CDC_DMA_TX_1",
			.rates = SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_192000 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 3,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_TX_CODEC_DMA_TX_1,
		.name = "TX_CDC_DMA_TX_1",
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA2 Playback",
			.aif_name = "RX_CDC_DMA_RX_2",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_2,
		.name = "RX_CDC_DMA_RX_2",
	},
	{
		.capture = {
			.stream_name = "TX CDC DMA2 Capture",
			.aif_name = "TX_CDC_DMA_TX_2",
			.rates = SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_192000 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_TX_CODEC_DMA_TX_2,
		.name = "TX_CDC_DMA_TX_2",
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA3 Playback",
			.aif_name = "RX_CDC_DMA_RX_3",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_3,
		.name = "RX_CDC_DMA_RX_3",
	},
	{
		.capture = {
			.stream_name = "TX CDC DMA3 Capture",
			.aif_name = "TX_CDC_DMA_TX_3",
			.rates = SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_192000 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_TX_CODEC_DMA_TX_3,
		.name = "TX_CDC_DMA_TX_3",
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA4 Playback",
			.aif_name = "RX_CDC_DMA_RX_4",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 6,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_4,
		.name = "RX_CDC_DMA_RX_4",
	},
	{
		.capture = {
			.stream_name = "TX CDC DMA4 Capture",
			.aif_name = "TX_CDC_DMA_TX_4",
			.rates = SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_192000 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_TX_CODEC_DMA_TX_4,
		.name = "TX_CDC_DMA_TX_4",
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA5 Playback",
			.aif_name = "RX_CDC_DMA_RX_5",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 1,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_5,
		.name = "RX_CDC_DMA_RX_5",
	},
	{
		.capture = {
			.stream_name = "TX CDC DMA5 Capture",
			.aif_name = "TX_CDC_DMA_TX_5",
			.rates = SNDRV_PCM_RATE_8000 |
				SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 |
				SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_96000 |
				SNDRV_PCM_RATE_192000 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_TX_CODEC_DMA_TX_5,
		.name = "TX_CDC_DMA_TX_5",
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA6 Playback",
			.aif_name = "RX_CDC_DMA_RX_6",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_6,
		.name = "RX_CDC_DMA_RX_6",
	},
	{
		.playback = {
			.stream_name = "RX CDC DMA7 Playback",
			.aif_name = "RX_CDC_DMA_RX_7",
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
				SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_352800 |
				SNDRV_PCM_RATE_384000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S24_3LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.id = AFE_PORT_ID_RX_CODEC_DMA_RX_7,
		.name = "RX_CDC_DMA_RX_7",
	},
};

static const struct snd_soc_component_driver msm_q6_cdc_dma_dai_component = {
	.name = "msm-dai-cdc-dma-dev",
};

/* DT related probe for each codec DMA interface device */
static int msm_dai_q6_cdc_dma_dev_probe(struct platform_device *pdev)
{
	const char *q6_cdc_dma_dev_id = "qcom,msm-dai-cdc-dma-dev-id";
	u32 cdc_dma_id = 0;
	int i;
	int rc = 0;
	struct msm_dai_q6_cdc_dma_dai_data *dai_data = NULL;

	rc = of_property_read_u32(pdev->dev.of_node, q6_cdc_dma_dev_id,
				  &cdc_dma_id);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: missing 0x%x in dt node\n", __func__, cdc_dma_id);
		return rc;
	}

	dev_dbg(&pdev->dev, "%s: dev name %s dev id 0x%x\n", __func__,
		dev_name(&pdev->dev), cdc_dma_id);

	pdev->id = cdc_dma_id;

	dai_data = devm_kzalloc(&pdev->dev,
				sizeof(struct msm_dai_q6_cdc_dma_dai_data),
				GFP_KERNEL);

	if (!dai_data)
		return -ENOMEM;

	rc = of_property_read_u32(pdev->dev.of_node,
				    "qcom,msm-dai-is-island-supported",
				    &dai_data->is_island_dai);
	if (rc)
		dev_dbg(&pdev->dev, "island supported entry not found\n");

	rc = of_property_read_u32(pdev->dev.of_node,
				"qcom,msm-cdc-dma-data-align",
				&dai_data->cdc_dma_data_align);
	if (rc)
		dev_dbg(&pdev->dev, "cdc dma data align supported entry not found\n");

	dev_set_drvdata(&pdev->dev, dai_data);

	for (i = 0; i < ARRAY_SIZE(msm_dai_q6_cdc_dma_dai); i++) {
		if (msm_dai_q6_cdc_dma_dai[i].id == cdc_dma_id) {
			return snd_soc_register_component(&pdev->dev,
				&msm_q6_cdc_dma_dai_component,
				&msm_dai_q6_cdc_dma_dai[i], 1);
		}
	}
	return -ENODEV;
}

static int msm_dai_q6_cdc_dma_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id msm_dai_q6_cdc_dma_dev_dt_match[] = {
	{ .compatible = "qcom,msm-dai-cdc-dma-dev", },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_dai_q6_cdc_dma_dev_dt_match);

static struct platform_driver msm_dai_q6_cdc_dma_driver = {
	.probe  = msm_dai_q6_cdc_dma_dev_probe,
	.remove  = msm_dai_q6_cdc_dma_dev_remove,
	.driver = {
		.name = "msm-dai-cdc-dma-dev",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_q6_cdc_dma_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

/* DT related probe for codec DMA interface device group */
static int msm_dai_cdc_dma_q6_probe(struct platform_device *pdev)
{
	int rc;

	rc = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (rc) {
		dev_err(&pdev->dev, "%s: failed to add child nodes, rc=%d\n",
			__func__, rc);
	} else
		dev_dbg(&pdev->dev, "%s: added child node\n", __func__);
	return rc;
}

static int msm_dai_cdc_dma_q6_remove(struct platform_device *pdev)
{
	of_platform_depopulate(&pdev->dev);
	return 0;
}

static const struct of_device_id msm_dai_cdc_dma_dt_match[] = {
	{ .compatible = "qcom,msm-dai-cdc-dma", },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_dai_cdc_dma_dt_match);

static struct platform_driver msm_dai_cdc_dma_q6 = {
	.probe  = msm_dai_cdc_dma_q6_probe,
	.remove = msm_dai_cdc_dma_q6_remove,
	.driver = {
		.name = "msm-dai-cdc-dma",
		.owner = THIS_MODULE,
		.of_match_table = msm_dai_cdc_dma_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init msm_dai_q6_init(void)
{
	int rc;

	rc = platform_driver_register(&msm_auxpcm_dev_driver);
	if (rc) {
		pr_err("%s: fail to register auxpcm dev driver", __func__);
		goto fail;
	}

	rc = platform_driver_register(&msm_dai_q6);
	if (rc) {
		pr_err("%s: fail to register dai q6 driver", __func__);
		goto dai_q6_fail;
	}

	rc = platform_driver_register(&msm_dai_q6_dev);
	if (rc) {
		pr_err("%s: fail to register dai q6 dev driver", __func__);
		goto dai_q6_dev_fail;
	}

	rc = platform_driver_register(&msm_dai_q6_mi2s_driver);
	if (rc) {
		pr_err("%s: fail to register dai MI2S dev drv\n", __func__);
		goto dai_q6_mi2s_drv_fail;
	}
#if 0
	rc = platform_driver_register(&msm_dai_q6_meta_mi2s_driver);
	if (rc) {
		pr_err("%s: fail to register dai META MI2S dev drv\n",
			__func__);
		goto dai_q6_meta_mi2s_drv_fail;
	}
#endif
	rc = platform_driver_register(&msm_dai_mi2s_q6);
	if (rc) {
		pr_err("%s: fail to register dai MI2S\n", __func__);
		goto dai_mi2s_q6_fail;
	}

	rc = platform_driver_register(&msm_dai_q6_tdm_driver);
	if (rc) {
		pr_err("%s: fail to register dai TDM dev drv\n", __func__);
		goto dai_q6_tdm_drv_fail;
	}

	rc = platform_driver_register(&msm_dai_tdm_q6);
	if (rc) {
		pr_err("%s: fail to register dai TDM\n", __func__);
		goto dai_tdm_q6_fail;
	}

	rc = platform_driver_register(&msm_dai_q6_cdc_dma_driver);
	if (rc) {
		pr_err("%s: fail to register dai CDC DMA dev\n", __func__);
		goto dai_cdc_dma_q6_dev_fail;
	}

	rc = platform_driver_register(&msm_dai_cdc_dma_q6);
	if (rc) {
		pr_err("%s: fail to register dai CDC DMA\n", __func__);
		goto dai_cdc_dma_q6_fail;
	}
	return rc;

#if 0
	rc = platform_driver_register(&msm_dai_q6_spdif_driver);
	if (rc) {
		pr_err("%s: fail to register dai SPDIF\n", __func__);
		goto dai_spdif_q6_fail;
	}



dai_cdc_dma_q6_fail:
	platform_driver_unregister(&msm_dai_q6_cdc_dma_driver);
dai_cdc_dma_q6_dev_fail:
	platform_driver_unregister(&msm_dai_tdm_q6);
dai_q6_tdm_drv_fail:
	platform_driver_unregister(&msm_dai_q6_spdif_driver);
dai_spdif_q6_fail:
	platform_driver_unregister(&msm_auxpcm_dev_driver);
fail:
#endif

dai_cdc_dma_q6_fail:
	platform_driver_unregister(&msm_dai_q6_cdc_dma_driver);
dai_cdc_dma_q6_dev_fail:
	platform_driver_unregister(&msm_dai_tdm_q6);
dai_tdm_q6_fail:
	platform_driver_unregister(&msm_dai_q6_tdm_driver);
dai_q6_tdm_drv_fail:
	platform_driver_unregister(&msm_dai_mi2s_q6);
dai_mi2s_q6_fail:
	platform_driver_unregister(&msm_dai_q6_mi2s_driver);
dai_q6_mi2s_drv_fail:
    platform_driver_unregister(&msm_dai_q6_dev);
dai_q6_dev_fail:
    platform_driver_unregister(&msm_dai_q6);
dai_q6_fail:
    platform_driver_unregister(&msm_auxpcm_dev_driver);
fail:
        return 0;
}

void msm_dai_q6_exit(void)
{
	platform_driver_unregister(&msm_dai_cdc_dma_q6);
	platform_driver_unregister(&msm_dai_q6_cdc_dma_driver);
	platform_driver_unregister(&msm_dai_tdm_q6);
	platform_driver_unregister(&msm_dai_q6_tdm_driver);
	platform_driver_unregister(&msm_dai_mi2s_q6);
//	platform_driver_unregister(&msm_dai_q6_meta_mi2s_driver);
	platform_driver_unregister(&msm_dai_q6_mi2s_driver);
	platform_driver_unregister(&msm_dai_q6_dev);
	platform_driver_unregister(&msm_auxpcm_dev_driver);
}

/* Module information */
MODULE_DESCRIPTION("MSM DSP DAI driver");
MODULE_LICENSE("GPL v2");
