// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/thermal.h>
#include <linux/pm_runtime.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <soc/swr-common.h>
#include <soc/swr-wcd.h>

#include <asoc/msm-cdc-pinctrl.h>
#include "lpass-cdc.h"
#include "lpass-cdc-comp.h"
#include "lpass-cdc-registers.h"
#include "lpass-cdc-wsa-macro.h"
#include "lpass-cdc-clk-rsc.h"

#define AUTO_SUSPEND_DELAY  50 /* delay in msec */
#define LPASS_CDC_WSA_MACRO_MAX_OFFSET 0x1000

#define LPASS_CDC_WSA_MACRO_RX_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |\
			SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |\
			SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)
#define LPASS_CDC_WSA_MACRO_RX_MIX_RATES (SNDRV_PCM_RATE_48000 |\
			SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)
#define LPASS_CDC_WSA_MACRO_VI_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |\
			SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000)
#define LPASS_CDC_WSA_MACRO_RX_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
		SNDRV_PCM_FMTBIT_S24_LE |\
		SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

#define LPASS_CDC_WSA_MACRO_ECHO_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |\
			SNDRV_PCM_RATE_48000)
#define LPASS_CDC_WSA_MACRO_ECHO_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
		SNDRV_PCM_FMTBIT_S24_LE |\
		SNDRV_PCM_FMTBIT_S24_3LE)

#define NUM_INTERPOLATORS 2

#define LPASS_CDC_WSA_MACRO_MUX_INP_SHFT 0x3
#define LPASS_CDC_WSA_MACRO_MUX_INP_MASK1 0x07
#define LPASS_CDC_WSA_MACRO_MUX_INP_MASK2 0x38
#define LPASS_CDC_WSA_MACRO_MUX_CFG_OFFSET 0x8
#define LPASS_CDC_WSA_MACRO_MUX_CFG1_OFFSET 0x4
#define LPASS_CDC_WSA_MACRO_RX_COMP_OFFSET \
		(LPASS_CDC_WSA_COMPANDER1_CTL0 - LPASS_CDC_WSA_COMPANDER0_CTL0)
#define LPASS_CDC_WSA_MACRO_RX_SOFTCLIP_OFFSET \
		(LPASS_CDC_WSA_SOFTCLIP1_CRC - LPASS_CDC_WSA_SOFTCLIP0_CRC)
#define LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET \
		(LPASS_CDC_WSA_RX1_RX_PATH_CTL - LPASS_CDC_WSA_RX0_RX_PATH_CTL)
#define LPASS_CDC_WSA_MACRO_RX_PATH_CFG3_OFFSET 0x10
#define LPASS_CDC_WSA_MACRO_RX_PATH_DSMDEM_OFFSET 0x4C
#define LPASS_CDC_WSA_MACRO_FS_RATE_MASK 0x0F
#define LPASS_CDC_WSA_MACRO_EC_MIX_TX0_MASK 0x03
#define LPASS_CDC_WSA_MACRO_EC_MIX_TX1_MASK 0x18

#define LPASS_CDC_WSA_MACRO_MAX_DMA_CH_PER_PORT 0x2
#define LPASS_CDC_WSA_MACRO_THERMAL_MAX_STATE 11

enum {
	LPASS_CDC_WSA_MACRO_RX0 = 0,
	LPASS_CDC_WSA_MACRO_RX1,
	LPASS_CDC_WSA_MACRO_RX_MIX,
	LPASS_CDC_WSA_MACRO_RX_MIX0 = LPASS_CDC_WSA_MACRO_RX_MIX,
	LPASS_CDC_WSA_MACRO_RX_MIX1,
	LPASS_CDC_WSA_MACRO_RX4,
	LPASS_CDC_WSA_MACRO_RX5,
	LPASS_CDC_WSA_MACRO_RX_MAX,
};

enum {
	LPASS_CDC_WSA_MACRO_TX0 = 0,
	LPASS_CDC_WSA_MACRO_TX1,
	LPASS_CDC_WSA_MACRO_TX_MAX,
};

enum {
	LPASS_CDC_WSA_MACRO_EC0_MUX = 0,
	LPASS_CDC_WSA_MACRO_EC1_MUX,
	LPASS_CDC_WSA_MACRO_EC_MUX_MAX,
};

enum {
	LPASS_CDC_WSA_MACRO_COMP1, /* SPK_L */
	LPASS_CDC_WSA_MACRO_COMP2, /* SPK_R */
	LPASS_CDC_WSA_MACRO_COMP_MAX
};

enum {
	LPASS_CDC_WSA_MACRO_SOFTCLIP0, /* RX0 */
	LPASS_CDC_WSA_MACRO_SOFTCLIP1, /* RX1 */
	LPASS_CDC_WSA_MACRO_SOFTCLIP_MAX
};

enum {
	INTn_1_INP_SEL_ZERO = 0,
	INTn_1_INP_SEL_RX0,
	INTn_1_INP_SEL_RX1,
	INTn_1_INP_SEL_RX2,
	INTn_1_INP_SEL_RX3,
	INTn_1_INP_SEL_RX4,
	INTn_1_INP_SEL_RX5,
	INTn_1_INP_SEL_DEC0,
	INTn_1_INP_SEL_DEC1,
};

enum {
	INTn_2_INP_SEL_ZERO = 0,
	INTn_2_INP_SEL_RX0,
	INTn_2_INP_SEL_RX1,
	INTn_2_INP_SEL_RX2,
	INTn_2_INP_SEL_RX3,
	INTn_2_INP_SEL_RX4,
	INTn_2_INP_SEL_RX5,
};

enum {
	WSA_MODE_21DB,
	WSA_MODE_19P5DB,
	WSA_MODE_18DB,
	WSA_MODE_16P5DB,
	WSA_MODE_15DB,
	WSA_MODE_13P5DB,
	WSA_MODE_12DB,
	WSA_MODE_10P5DB,
	WSA_MODE_9DB,
	WSA881X_MODE_18DB,
	WSA881x_MODE_13P5DB,
	WSA_MODE_MAX
};

static struct lpass_cdc_comp_setting comp_setting_table[WSA_MODE_MAX] =
{
	{42, 0, 42},
	{39, 0, 42},
	{36, 0, 42},
	{33, 0, 42},
	{30, 0, 42},
	{27, 0, 42},
	{24, 0, 42},
	{21, 0, 42},
	{18, 0, 42},
	{36, 0, 36},
	{27, 0, 36},
};

struct interp_sample_rate {
	int sample_rate;
	int rate_val;
};

/*
 * Structure used to update codec
 * register defaults after reset
 */
struct lpass_cdc_wsa_macro_reg_mask_val {
	u16 reg;
	u8 mask;
	u8 val;
};

static struct interp_sample_rate int_prim_sample_rate_val[] = {
	{8000, 0x0},	/* 8K */
	{16000, 0x1},	/* 16K */
	{24000, -EINVAL},/* 24K */
	{32000, 0x3},	/* 32K */
	{48000, 0x4},	/* 48K */
	{96000, 0x5},	/* 96K */
	{192000, 0x6},	/* 192K */
	{384000, 0x7},	/* 384K */
	{44100, 0x8}, /* 44.1K */
};

static struct interp_sample_rate int_mix_sample_rate_val[] = {
	{48000, 0x4},	/* 48K */
	{96000, 0x5},	/* 96K */
	{192000, 0x6},	/* 192K */
};

#define LPASS_CDC_WSA_MACRO_SWR_STRING_LEN 80

static int lpass_cdc_wsa_macro_core_vote(void *handle, bool enable);
static int lpass_cdc_wsa_macro_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params,
			       struct snd_soc_dai *dai);
static int lpass_cdc_wsa_macro_get_channel_map(struct snd_soc_dai *dai,
				unsigned int *tx_num, unsigned int *tx_slot,
				unsigned int *rx_num, unsigned int *rx_slot);
static int lpass_cdc_wsa_macro_mute_stream(struct snd_soc_dai *dai, int mute, int stream);
/* Hold instance to soundwire platform device */
struct lpass_cdc_wsa_macro_swr_ctrl_data {
	struct platform_device *wsa_swr_pdev;
};
static int lpass_cdc_wsa_macro_enable_vi_decimator(struct snd_soc_component *component);

#define LPASS_CDC_WSA_MACRO_SET_VOLUME_TLV(xname, xreg, xmin, xmax, tlv_array) \
{	.iface  = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname), \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ | \
		SNDRV_CTL_ELEM_ACCESS_READWRITE, \
	.tlv.p  = (tlv_array), \
	.info = snd_soc_info_volsw, .get = snd_soc_get_volsw,\
	.put = lpass_cdc_wsa_macro_set_digital_volume, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
	{.reg = xreg, .rreg = xreg,  \
	.min = xmin, .max = xmax, .platform_max = xmax, \
	.sign_bit = 7,} }

struct lpass_cdc_wsa_macro_swr_ctrl_platform_data {
	void *handle; /* holds codec private data */
	int (*read)(void *handle, int reg);
	int (*write)(void *handle, int reg, int val);
	int (*bulk_write)(void *handle, u32 *reg, u32 *val, size_t len);
	int (*clk)(void *handle, bool enable);
	int (*core_vote)(void *handle, bool enable);
	int (*handle_irq)(void *handle,
			  irqreturn_t (*swrm_irq_handler)(int irq,
							  void *data),
			  void *swrm_handle,
			  int action);
};

enum {
	LPASS_CDC_WSA_MACRO_AIF_INVALID = 0,
	LPASS_CDC_WSA_MACRO_AIF1_PB,
	LPASS_CDC_WSA_MACRO_AIF_MIX1_PB,
	LPASS_CDC_WSA_MACRO_AIF_VI,
	LPASS_CDC_WSA_MACRO_AIF_ECHO,
	LPASS_CDC_WSA_MACRO_MAX_DAIS,
};

#define LPASS_CDC_WSA_MACRO_CHILD_DEVICES_MAX 3

/*
 * @dev: wsa macro device pointer
 * @comp_enabled: compander enable mixer value set
 * @ec_hq: echo HQ enable mixer value set
 * @prim_int_users: Users of interpolator
 * @wsa_mclk_users: WSA MCLK users count
 * @swr_clk_users: SWR clk users count
 * @vi_feed_value: VI sense mask
 * @mclk_lock: to lock mclk operations
 * @swr_clk_lock: to lock swr master clock operations
 * @swr_ctrl_data: SoundWire data structure
 * @swr_plat_data: Soundwire platform data
 * @lpass_cdc_wsa_macro_add_child_devices_work: work for adding child devices
 * @wsa_swr_gpio_p: used by pinctrl API
 * @component: codec handle
 * @rx_0_count: RX0 interpolation users
 * @rx_1_count: RX1 interpolation users
 * @active_ch_mask: channel mask for all AIF DAIs
 * @active_ch_cnt: channel count of all AIF DAIs
 * @rx_port_value: mixer ctl value of WSA RX MUXes
 * @wsa_io_base: Base address of WSA macro addr space
 */
struct lpass_cdc_wsa_macro_priv {
	struct device *dev;
	int comp_enabled[LPASS_CDC_WSA_MACRO_COMP_MAX];
	int comp_mode[LPASS_CDC_WSA_MACRO_COMP_MAX];
	int ec_hq[LPASS_CDC_WSA_MACRO_RX1 + 1];
	u16 prim_int_users[LPASS_CDC_WSA_MACRO_RX1 + 1];
	u16 wsa_mclk_users;
	u16 swr_clk_users;
	bool dapm_mclk_enable;
	bool reset_swr;
	unsigned int vi_feed_value;
	struct mutex mclk_lock;
	struct mutex swr_clk_lock;
	struct lpass_cdc_wsa_macro_swr_ctrl_data *swr_ctrl_data;
	struct lpass_cdc_wsa_macro_swr_ctrl_platform_data swr_plat_data;
	struct work_struct lpass_cdc_wsa_macro_add_child_devices_work;
	struct device_node *wsa_swr_gpio_p;
	struct snd_soc_component *component;
	int rx_0_count;
	int rx_1_count;
	unsigned long active_ch_mask[LPASS_CDC_WSA_MACRO_MAX_DAIS];
	unsigned long active_ch_cnt[LPASS_CDC_WSA_MACRO_MAX_DAIS];
	int rx_port_value[LPASS_CDC_WSA_MACRO_RX_MAX];
	char __iomem *wsa_io_base;
	struct platform_device *pdev_child_devices
			[LPASS_CDC_WSA_MACRO_CHILD_DEVICES_MAX];
	int child_count;
	int wsa_spkrrecv;
	int spkr_gain_offset;
	int spkr_mode;
	int is_softclip_on[LPASS_CDC_WSA_MACRO_SOFTCLIP_MAX];
	int softclip_clk_users[LPASS_CDC_WSA_MACRO_SOFTCLIP_MAX];
	char __iomem *mclk_mode_muxsel;
	u16 default_clk_id;
	u32 pcm_rate_vi;
	int wsa_digital_mute_status[LPASS_CDC_WSA_MACRO_RX_MAX];
	u8 rx0_origin_gain;
	u8 rx1_origin_gain;
	struct thermal_cooling_device *tcdev;
	uint32_t thermal_cur_state;
	uint32_t thermal_max_state;
	struct work_struct lpass_cdc_wsa_macro_cooling_work;
	bool pre_dev_up;
};

static struct snd_soc_dai_driver lpass_cdc_wsa_macro_dai[];
static const DECLARE_TLV_DB_SCALE(digital_gain, 0, 1, 0);

static const char *const rx_text[] = {
	"ZERO", "RX0", "RX1", "RX_MIX0", "RX_MIX1", "RX4", "RX5", "DEC0", "DEC1"
};

static const char *const rx_mix_text[] = {
	"ZERO", "RX0", "RX1", "RX_MIX0", "RX_MIX1", "RX4", "RX5",
};

static const char *const rx_mix_ec_text[] = {
	"ZERO", "RX_MIX_TX0", "RX_MIX_TX1"
};

static const char *const rx_mux_text[] = {
	"ZERO", "AIF1_PB", "AIF_MIX1_PB"
};

static const char *const rx_sidetone_mix_text[] = {
	"ZERO", "SRC0"
};

static const char * const lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_text[] = {
	"OFF", "ON"
};

static const char *const lpass_cdc_wsa_macro_ear_spkrrecv_text[] = {
	"OFF", "ON"
};

static const char * const lpass_cdc_wsa_macro_comp_mode_text[] = {
	"G_21_DB", "G_19P5_DB", "G_18_DB", "G_16P5_DB", "G_15_DB",
	"G_13P5_DB", "G_12_DB", "G_10P5_DB", "G_9_DB",
	"WSA881X_G_18DB", "WSA881x_G_13P5DB"
};

static const struct snd_kcontrol_new wsa_int0_vbat_mix_switch[] = {
	SOC_DAPM_SINGLE("WSA RX0 VBAT Enable", SND_SOC_NOPM, 0, 1, 0)
};

static const struct snd_kcontrol_new wsa_int1_vbat_mix_switch[] = {
	SOC_DAPM_SINGLE("WSA RX1 VBAT Enable", SND_SOC_NOPM, 0, 1, 0)
};

static SOC_ENUM_SINGLE_EXT_DECL(lpass_cdc_wsa_macro_ear_spkrrecv_enum,
				lpass_cdc_wsa_macro_ear_spkrrecv_text);
static SOC_ENUM_SINGLE_EXT_DECL(lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_enum,
			lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_text);
static SOC_ENUM_SINGLE_EXT_DECL(lpass_cdc_wsa_macro_comp_mode_enum,
			lpass_cdc_wsa_macro_comp_mode_text);

/* RX INT0 */
static const struct soc_enum rx0_prim_inp0_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG0,
		0, 9, rx_text);

static const struct soc_enum rx0_prim_inp1_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG0,
		3, 9, rx_text);

static const struct soc_enum rx0_prim_inp2_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG1,
		3, 9, rx_text);

static const struct soc_enum rx0_mix_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG1,
		0, 7, rx_mix_text);

static const struct soc_enum rx0_sidetone_mix_enum =
	SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, 2, rx_sidetone_mix_text);

static const struct snd_kcontrol_new rx0_prim_inp0_mux =
	SOC_DAPM_ENUM("WSA_RX0 INP0 Mux", rx0_prim_inp0_chain_enum);

static const struct snd_kcontrol_new rx0_prim_inp1_mux =
	SOC_DAPM_ENUM("WSA_RX0 INP1 Mux", rx0_prim_inp1_chain_enum);

static const struct snd_kcontrol_new rx0_prim_inp2_mux =
	SOC_DAPM_ENUM("WSA_RX0 INP2 Mux", rx0_prim_inp2_chain_enum);

static const struct snd_kcontrol_new rx0_mix_mux =
	SOC_DAPM_ENUM("WSA_RX0 MIX Mux", rx0_mix_chain_enum);

static const struct snd_kcontrol_new rx0_sidetone_mix_mux =
	SOC_DAPM_ENUM("WSA_RX0 SIDETONE MIX Mux", rx0_sidetone_mix_enum);

/* RX INT1 */
static const struct soc_enum rx1_prim_inp0_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT1_CFG0,
		0, 9, rx_text);

static const struct soc_enum rx1_prim_inp1_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT1_CFG0,
		3, 9, rx_text);

static const struct soc_enum rx1_prim_inp2_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT1_CFG1,
		3, 9, rx_text);

static const struct soc_enum rx1_mix_chain_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_INT1_CFG1,
		0, 7, rx_mix_text);

static const struct snd_kcontrol_new rx1_prim_inp0_mux =
	SOC_DAPM_ENUM("WSA_RX1 INP0 Mux", rx1_prim_inp0_chain_enum);

static const struct snd_kcontrol_new rx1_prim_inp1_mux =
	SOC_DAPM_ENUM("WSA_RX1 INP1 Mux", rx1_prim_inp1_chain_enum);

static const struct snd_kcontrol_new rx1_prim_inp2_mux =
	SOC_DAPM_ENUM("WSA_RX1 INP2 Mux", rx1_prim_inp2_chain_enum);

static const struct snd_kcontrol_new rx1_mix_mux =
	SOC_DAPM_ENUM("WSA_RX1 MIX Mux", rx1_mix_chain_enum);

static const struct soc_enum rx_mix_ec0_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_MIX_CFG0,
		0, 3, rx_mix_ec_text);

static const struct soc_enum rx_mix_ec1_enum =
	SOC_ENUM_SINGLE(LPASS_CDC_WSA_RX_INP_MUX_RX_MIX_CFG0,
		3, 3, rx_mix_ec_text);

static const struct snd_kcontrol_new rx_mix_ec0_mux =
	SOC_DAPM_ENUM("WSA RX_MIX EC0_Mux", rx_mix_ec0_enum);

static const struct snd_kcontrol_new rx_mix_ec1_mux =
	SOC_DAPM_ENUM("WSA RX_MIX EC1_Mux", rx_mix_ec1_enum);

static struct snd_soc_dai_ops lpass_cdc_wsa_macro_dai_ops = {
	.hw_params = lpass_cdc_wsa_macro_hw_params,
	.get_channel_map = lpass_cdc_wsa_macro_get_channel_map,
	.mute_stream = lpass_cdc_wsa_macro_mute_stream,
};

static struct snd_soc_dai_driver lpass_cdc_wsa_macro_dai[] = {
	{
		.name = "wsa_macro_rx1",
		.id = LPASS_CDC_WSA_MACRO_AIF1_PB,
		.playback = {
			.stream_name = "WSA_AIF1 Playback",
			.rates = LPASS_CDC_WSA_MACRO_RX_RATES,
			.formats = LPASS_CDC_WSA_MACRO_RX_FORMATS,
			.rate_max = 384000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 2,
		},
		.ops = &lpass_cdc_wsa_macro_dai_ops,
	},
	{
		.name = "wsa_macro_rx_mix",
		.id = LPASS_CDC_WSA_MACRO_AIF_MIX1_PB,
		.playback = {
			.stream_name = "WSA_AIF_MIX1 Playback",
			.rates = LPASS_CDC_WSA_MACRO_RX_MIX_RATES,
			.formats = LPASS_CDC_WSA_MACRO_RX_FORMATS,
			.rate_max = 192000,
			.rate_min = 48000,
			.channels_min = 1,
			.channels_max = 2,
		},
		.ops = &lpass_cdc_wsa_macro_dai_ops,
	},
	{
		.name = "wsa_macro_vifeedback",
		.id = LPASS_CDC_WSA_MACRO_AIF_VI,
		.capture = {
			.stream_name = "WSA_AIF_VI Capture",
			.rates = LPASS_CDC_WSA_MACRO_VI_RATES,
			.formats = LPASS_CDC_WSA_MACRO_RX_FORMATS,
			.rate_max = 48000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 4,
		},
		.ops = &lpass_cdc_wsa_macro_dai_ops,
	},
	{
		.name = "wsa_macro_echo",
		.id = LPASS_CDC_WSA_MACRO_AIF_ECHO,
		.capture = {
			.stream_name = "WSA_AIF_ECHO Capture",
			.rates = LPASS_CDC_WSA_MACRO_ECHO_RATES,
			.formats = LPASS_CDC_WSA_MACRO_ECHO_FORMATS,
			.rate_max = 48000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 2,
		},
		.ops = &lpass_cdc_wsa_macro_dai_ops,
	},
};

static bool lpass_cdc_wsa_macro_get_data(struct snd_soc_component *component,
			       struct device **wsa_dev,
			       struct lpass_cdc_wsa_macro_priv **wsa_priv,
			       const char *func_name)
{
	*wsa_dev = lpass_cdc_get_device_ptr(component->dev,
							WSA_MACRO);
	if (!(*wsa_dev)) {
		dev_err(component->dev,
			"%s: null device for macro!\n", func_name);
		return false;
	}
	*wsa_priv = dev_get_drvdata((*wsa_dev));
	if (!(*wsa_priv) || !(*wsa_priv)->component) {
		dev_err(component->dev,
			"%s: priv is null for macro!\n", func_name);
		return false;
	}
	return true;
}

static int lpass_cdc_wsa_macro_set_port_map(struct snd_soc_component *component,
				u32 usecase, u32 size, void *data)
{
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	struct swrm_port_config port_cfg;
	int ret = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	memset(&port_cfg, 0, sizeof(port_cfg));
	port_cfg.uc = usecase;
	port_cfg.size = size;
	port_cfg.params = data;

	if (wsa_priv->swr_ctrl_data)
		ret = swrm_wcd_notify(
			wsa_priv->swr_ctrl_data[0].wsa_swr_pdev,
			SWR_SET_PORT_MAP, &port_cfg);

	return ret;
}

static int lpass_cdc_wsa_macro_set_prim_interpolator_rate(struct snd_soc_dai *dai,
					    u8 int_prim_fs_rate_reg_val,
					    u32 sample_rate)
{
	u8 int_1_mix1_inp;
	u32 j, port;
	u16 int_mux_cfg0, int_mux_cfg1;
	u16 int_fs_reg;
	u8 int_mux_cfg0_val, int_mux_cfg1_val;
	u8 inp0_sel, inp1_sel, inp2_sel;
	struct snd_soc_component *component = dai->component;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	for_each_set_bit(port, &wsa_priv->active_ch_mask[dai->id],
			 LPASS_CDC_WSA_MACRO_RX_MAX) {
		int_1_mix1_inp = port;
		if ((int_1_mix1_inp < LPASS_CDC_WSA_MACRO_RX0) ||
			(int_1_mix1_inp > LPASS_CDC_WSA_MACRO_RX_MIX1)) {
			dev_err(wsa_dev,
				"%s: Invalid RX port, Dai ID is %d\n",
				__func__, dai->id);
			return -EINVAL;
		}

		int_mux_cfg0 = LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG0;

		/*
		 * Loop through all interpolator MUX inputs and find out
		 * to which interpolator input, the cdc_dma rx port
		 * is connected
		 */
		for (j = 0; j < NUM_INTERPOLATORS; j++) {
			int_mux_cfg1 = int_mux_cfg0 + LPASS_CDC_WSA_MACRO_MUX_CFG1_OFFSET;

			int_mux_cfg0_val = snd_soc_component_read(component,
							int_mux_cfg0);
			int_mux_cfg1_val = snd_soc_component_read(component,
							int_mux_cfg1);
			inp0_sel = int_mux_cfg0_val & LPASS_CDC_WSA_MACRO_MUX_INP_MASK1;
			inp1_sel = (int_mux_cfg0_val >>
					LPASS_CDC_WSA_MACRO_MUX_INP_SHFT) &
					LPASS_CDC_WSA_MACRO_MUX_INP_MASK1;
			inp2_sel = (int_mux_cfg1_val >>
					LPASS_CDC_WSA_MACRO_MUX_INP_SHFT) &
					LPASS_CDC_WSA_MACRO_MUX_INP_MASK1;
			if ((inp0_sel == int_1_mix1_inp + INTn_1_INP_SEL_RX0) ||
			    (inp1_sel == int_1_mix1_inp + INTn_1_INP_SEL_RX0) ||
			    (inp2_sel == int_1_mix1_inp + INTn_1_INP_SEL_RX0)) {
				int_fs_reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL +
					     LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET * j;
				dev_dbg(wsa_dev,
					"%s: AIF_PB DAI(%d) connected to INT%u_1\n",
					__func__, dai->id, j);
				dev_dbg(wsa_dev,
					"%s: set INT%u_1 sample rate to %u\n",
					__func__, j, sample_rate);
				/* sample_rate is in Hz */
				snd_soc_component_update_bits(component,
						int_fs_reg,
						LPASS_CDC_WSA_MACRO_FS_RATE_MASK,
						int_prim_fs_rate_reg_val);
			}
			int_mux_cfg0 += LPASS_CDC_WSA_MACRO_MUX_CFG_OFFSET;
		}
	}

	return 0;
}

static int lpass_cdc_wsa_macro_set_mix_interpolator_rate(struct snd_soc_dai *dai,
					u8 int_mix_fs_rate_reg_val,
					u32 sample_rate)
{
	u8 int_2_inp;
	u32 j, port;
	u16 int_mux_cfg1, int_fs_reg;
	u8 int_mux_cfg1_val;
	struct snd_soc_component *component = dai->component;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;


	for_each_set_bit(port, &wsa_priv->active_ch_mask[dai->id],
			 LPASS_CDC_WSA_MACRO_RX_MAX) {
		int_2_inp = port;
		if ((int_2_inp < LPASS_CDC_WSA_MACRO_RX0) ||
			(int_2_inp > LPASS_CDC_WSA_MACRO_RX_MIX1)) {
			dev_err(wsa_dev,
				"%s: Invalid RX port, Dai ID is %d\n",
				__func__, dai->id);
			return -EINVAL;
		}

		int_mux_cfg1 = LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG1;
		for (j = 0; j < NUM_INTERPOLATORS; j++) {
			int_mux_cfg1_val = snd_soc_component_read(component,
							int_mux_cfg1) &
							LPASS_CDC_WSA_MACRO_MUX_INP_MASK1;
			if (int_mux_cfg1_val == int_2_inp +
							INTn_2_INP_SEL_RX0) {
				int_fs_reg =
					LPASS_CDC_WSA_RX0_RX_PATH_MIX_CTL +
					LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET * j;

				dev_dbg(wsa_dev,
					"%s: AIF_PB DAI(%d) connected to INT%u_2\n",
					__func__, dai->id, j);
				dev_dbg(wsa_dev,
					"%s: set INT%u_2 sample rate to %u\n",
					__func__, j, sample_rate);
				snd_soc_component_update_bits(component,
						int_fs_reg,
						LPASS_CDC_WSA_MACRO_FS_RATE_MASK,
						int_mix_fs_rate_reg_val);
			}
			int_mux_cfg1 += LPASS_CDC_WSA_MACRO_MUX_CFG_OFFSET;
		}
	}
	return 0;
}

static int lpass_cdc_wsa_macro_set_interpolator_rate(struct snd_soc_dai *dai,
				       u32 sample_rate)
{
	int rate_val = 0;
	int i, ret;

	/* set mixing path rate */
	for (i = 0; i < ARRAY_SIZE(int_mix_sample_rate_val); i++) {
		if (sample_rate ==
				int_mix_sample_rate_val[i].sample_rate) {
			rate_val =
				int_mix_sample_rate_val[i].rate_val;
			break;
		}
	}
	if ((i == ARRAY_SIZE(int_mix_sample_rate_val)) ||
			(rate_val < 0))
		goto prim_rate;
	ret = lpass_cdc_wsa_macro_set_mix_interpolator_rate(dai,
			(u8) rate_val, sample_rate);
prim_rate:
	/* set primary path sample rate */
	for (i = 0; i < ARRAY_SIZE(int_prim_sample_rate_val); i++) {
		if (sample_rate ==
				int_prim_sample_rate_val[i].sample_rate) {
			rate_val =
				int_prim_sample_rate_val[i].rate_val;
			break;
		}
	}
	if ((i == ARRAY_SIZE(int_prim_sample_rate_val)) ||
			(rate_val < 0))
		return -EINVAL;
	ret = lpass_cdc_wsa_macro_set_prim_interpolator_rate(dai,
			(u8) rate_val, sample_rate);
	return ret;
}

static int lpass_cdc_wsa_macro_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params,
			       struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	int ret;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	wsa_priv = dev_get_drvdata(wsa_dev);
	if (!wsa_priv)
		return -EINVAL;

	dev_dbg(component->dev,
		"%s: dai_name = %s DAI-ID %x rate %d num_ch %d\n", __func__,
		 dai->name, dai->id, params_rate(params),
		 params_channels(params));

	switch (substream->stream) {
	case SNDRV_PCM_STREAM_PLAYBACK:
		ret = lpass_cdc_wsa_macro_set_interpolator_rate(dai, params_rate(params));
		if (ret) {
			dev_err(component->dev,
				"%s: cannot set sample rate: %u\n",
				__func__, params_rate(params));
			return ret;
		}
		break;
	case SNDRV_PCM_STREAM_CAPTURE:
		if (dai->id == LPASS_CDC_WSA_MACRO_AIF_VI)
			wsa_priv->pcm_rate_vi = params_rate(params);
	default:
		break;
	}
	return 0;
}

static int lpass_cdc_wsa_macro_get_channel_map(struct snd_soc_dai *dai,
				unsigned int *tx_num, unsigned int *tx_slot,
				unsigned int *rx_num, unsigned int *rx_slot)
{
	struct snd_soc_component *component = dai->component;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u16 val = 0, mask = 0, cnt = 0, temp = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	wsa_priv = dev_get_drvdata(wsa_dev);
	if (!wsa_priv)
		return -EINVAL;

	switch (dai->id) {
	case LPASS_CDC_WSA_MACRO_AIF_VI:
		*tx_slot = wsa_priv->active_ch_mask[dai->id];
		*tx_num = wsa_priv->active_ch_cnt[dai->id];
		break;
	case LPASS_CDC_WSA_MACRO_AIF1_PB:
	case LPASS_CDC_WSA_MACRO_AIF_MIX1_PB:
		for_each_set_bit(temp, &wsa_priv->active_ch_mask[dai->id],
					LPASS_CDC_WSA_MACRO_RX_MAX) {
			mask |= (1 << temp);
			if (++cnt == LPASS_CDC_WSA_MACRO_MAX_DMA_CH_PER_PORT)
				break;
		}
		if (mask & 0x0C)
			mask = mask >> 0x2;
		*rx_slot = mask;
		*rx_num = cnt;
		break;
	case LPASS_CDC_WSA_MACRO_AIF_ECHO:
		val = snd_soc_component_read(component,
			LPASS_CDC_WSA_RX_INP_MUX_RX_MIX_CFG0);
		if (val & LPASS_CDC_WSA_MACRO_EC_MIX_TX1_MASK) {
			mask |= 0x2;
			cnt++;
		}
		if (val & LPASS_CDC_WSA_MACRO_EC_MIX_TX0_MASK) {
			mask |= 0x1;
			cnt++;
		}
		*tx_slot = mask;
		*tx_num = cnt;
		break;
	default:
		dev_err(wsa_dev, "%s: Invalid AIF\n", __func__);
		break;
	}
	return 0;
}

static void lpass_cdc_wsa_unmute_interpolator(struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	uint16_t j = 0, reg = 0, mix_reg = 0;

	switch (dai->id) {
	case LPASS_CDC_WSA_MACRO_AIF1_PB:
	case LPASS_CDC_WSA_MACRO_AIF_MIX1_PB:
		for (j = 0; j < NUM_INTERPOLATORS; ++j) {
			reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL +
				(j * LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET);
			mix_reg = LPASS_CDC_WSA_RX0_RX_PATH_MIX_CTL +
				(j * LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET);

			snd_soc_component_update_bits(component, reg, 0x10, 0x00);
			snd_soc_component_update_bits(component, mix_reg, 0x10, 0x00);
		}
	}
}

static int lpass_cdc_wsa_macro_mute_stream(struct snd_soc_dai *dai, int mute, int stream)
{
	struct snd_soc_component *component = dai->component;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	uint16_t j = 0, reg = 0, mix_reg = 0, dsm_reg = 0;
	u16 int_mux_cfg0 = 0, int_mux_cfg1 = 0;
	u8 int_mux_cfg0_val = 0, int_mux_cfg1_val = 0;
	bool adie_lb = false;

	if (mute)
		return 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	switch (dai->id) {
	case LPASS_CDC_WSA_MACRO_AIF1_PB:
	case LPASS_CDC_WSA_MACRO_AIF_MIX1_PB:
	for (j = 0; j < NUM_INTERPOLATORS; j++) {
		reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL +
				(j * LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET);
		mix_reg = LPASS_CDC_WSA_RX0_RX_PATH_MIX_CTL +
				(j * LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET);
		dsm_reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL +
				(j * LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET) +
				LPASS_CDC_WSA_MACRO_RX_PATH_DSMDEM_OFFSET;
		int_mux_cfg0 = LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG0 + j * 8;
		int_mux_cfg1 = int_mux_cfg0 + 4;
		int_mux_cfg0_val = snd_soc_component_read(component,
							int_mux_cfg0);
		int_mux_cfg1_val = snd_soc_component_read(component,
							int_mux_cfg1);
		if (snd_soc_component_read(component, dsm_reg) & 0x01) {
			if (int_mux_cfg0_val || (int_mux_cfg1_val & 0x38))
				snd_soc_component_update_bits(component, reg,
							0x20, 0x20);
			if (int_mux_cfg1_val & 0x07) {
				snd_soc_component_update_bits(component, reg,
							0x20, 0x20);
				snd_soc_component_update_bits(component,
						mix_reg, 0x20, 0x20);
			}
		}
	}
	lpass_cdc_wsa_pa_on(wsa_dev, adie_lb);
	lpass_cdc_wsa_unmute_interpolator(dai);
	lpass_cdc_wsa_macro_enable_vi_decimator(component);
		break;
	default:
		break;
	}
	return 0;
}
static int lpass_cdc_wsa_macro_mclk_enable(
				struct lpass_cdc_wsa_macro_priv *wsa_priv,
				 bool mclk_enable, bool dapm)
{
	struct regmap *regmap = dev_get_regmap(wsa_priv->dev->parent, NULL);
	int ret = 0;

	if (regmap == NULL) {
		dev_err(wsa_priv->dev, "%s: regmap is NULL\n", __func__);
		return -EINVAL;
	}

	dev_dbg(wsa_priv->dev, "%s: mclk_enable = %u, dapm = %d clk_users= %d\n",
		__func__, mclk_enable, dapm, wsa_priv->wsa_mclk_users);

	mutex_lock(&wsa_priv->mclk_lock);
	if (mclk_enable) {
		if (wsa_priv->wsa_mclk_users == 0) {
			ret = lpass_cdc_clk_rsc_request_clock(wsa_priv->dev,
							wsa_priv->default_clk_id,
							wsa_priv->default_clk_id,
							true);
			if (ret < 0) {
				dev_err_ratelimited(wsa_priv->dev,
					"%s: wsa request clock enable failed\n",
					__func__);
				goto exit;
			}
			lpass_cdc_clk_rsc_fs_gen_request(wsa_priv->dev,
						  true);
			regcache_mark_dirty(regmap);
			regcache_sync_region(regmap,
					WSA_START_OFFSET,
					WSA_MAX_OFFSET);
			/* 9.6MHz MCLK, set value 0x00 if other frequency */
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_TOP_FREQ_MCLK, 0x01, 0x01);
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_MCLK_CONTROL,
				0x01, 0x01);
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_FS_CNT_CONTROL,
				0x01, 0x01);
		}
		wsa_priv->wsa_mclk_users++;
	} else {
		if (wsa_priv->wsa_mclk_users <= 0) {
			dev_err(wsa_priv->dev, "%s: clock already disabled\n",
			__func__);
			wsa_priv->wsa_mclk_users = 0;
			goto exit;
		}
		wsa_priv->wsa_mclk_users--;
		if (wsa_priv->wsa_mclk_users == 0) {
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_FS_CNT_CONTROL,
				0x01, 0x00);
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_MCLK_CONTROL,
				0x01, 0x00);
			lpass_cdc_clk_rsc_fs_gen_request(wsa_priv->dev,
						  false);

			lpass_cdc_clk_rsc_request_clock(wsa_priv->dev,
						  wsa_priv->default_clk_id,
						  wsa_priv->default_clk_id,
						  false);
		}
	}
exit:
	mutex_unlock(&wsa_priv->mclk_lock);
	return ret;
}

static int lpass_cdc_wsa_macro_mclk_event(struct snd_soc_dapm_widget *w,
			       struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(w->dapm);
	int ret = 0;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	dev_dbg(wsa_dev, "%s: event = %d\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = lpass_cdc_wsa_macro_mclk_enable(wsa_priv, 1, true);
		if (ret)
			wsa_priv->dapm_mclk_enable = false;
		else
			wsa_priv->dapm_mclk_enable = true;
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (wsa_priv->dapm_mclk_enable) {
			lpass_cdc_wsa_macro_mclk_enable(wsa_priv, 0, true);
			wsa_priv->dapm_mclk_enable = false;
		}
		break;
	default:
		dev_err(wsa_priv->dev,
			"%s: invalid DAPM event %d\n", __func__, event);
		ret = -EINVAL;
	}
	return ret;
}

static int lpass_cdc_wsa_macro_event_handler(struct snd_soc_component *component,
				   u16 event, u32 data)
{
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	int ret = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	switch (event) {
	case LPASS_CDC_MACRO_EVT_SSR_DOWN:
		wsa_priv->pre_dev_up = false;
		trace_printk("%s, enter SSR down\n", __func__);
		if (wsa_priv->swr_ctrl_data) {
			swrm_wcd_notify(
				wsa_priv->swr_ctrl_data[0].wsa_swr_pdev,
				SWR_DEVICE_SSR_DOWN, NULL);
		}
		if ((!pm_runtime_enabled(wsa_dev) ||
		     !pm_runtime_suspended(wsa_dev))) {
			ret = lpass_cdc_runtime_suspend(wsa_dev);
			if (!ret) {
				pm_runtime_disable(wsa_dev);
				pm_runtime_set_suspended(wsa_dev);
				pm_runtime_enable(wsa_dev);
			}
		}
		break;
	case LPASS_CDC_MACRO_EVT_PRE_SSR_UP:
		break;
	case LPASS_CDC_MACRO_EVT_SSR_UP:
		trace_printk("%s, enter SSR up\n", __func__);
		wsa_priv->pre_dev_up = true;
		/* reset swr after ssr/pdr */
		wsa_priv->reset_swr = true;
		if (wsa_priv->swr_ctrl_data)
			swrm_wcd_notify(
				wsa_priv->swr_ctrl_data[0].wsa_swr_pdev,
				SWR_DEVICE_SSR_UP, NULL);
		break;
	case LPASS_CDC_MACRO_EVT_CLK_RESET:
		lpass_cdc_rsc_clk_reset(wsa_dev, WSA_CORE_CLK);
		lpass_cdc_rsc_clk_reset(wsa_dev, WSA_TX_CORE_CLK);
		break;
	}
	return 0;
}

static int lpass_cdc_wsa_macro_enable_vi_decimator(struct snd_soc_component *component)
{
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u8 val = 0x0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (test_bit(LPASS_CDC_WSA_MACRO_TX0,
			&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])||
		test_bit(LPASS_CDC_WSA_MACRO_TX1,
			&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
		usleep_range(5000, 5500);
	}

	dev_dbg(wsa_dev, "%s: wsa_priv->pcm_rate_vi %d\n", __func__, wsa_priv->pcm_rate_vi);
	switch (wsa_priv->pcm_rate_vi) {
		case 48000:
			val = 0x04;
			break;
		case 24000:
			val = 0x02;
			break;
		case 8000:
		default:
			val = 0x00;
			break;
	}

	if (test_bit(LPASS_CDC_WSA_MACRO_TX0,
		&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
		dev_dbg(wsa_dev, "%s: spkr1 enabled\n", __func__);
		/* Enable V&I sensing */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CTL,
		0x20, 0x20);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CTL,
		0x20, 0x20);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CTL,
		0x0F, val);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CTL,
			0x0F, val);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CTL,
			0x10, 0x10);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CTL,
			0x10, 0x10);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CTL,
			0x20, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CTL,
			0x20, 0x00);
	}
	if (test_bit(LPASS_CDC_WSA_MACRO_TX1,
		&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
		dev_dbg(wsa_dev, "%s: spkr2 enabled\n", __func__);
		/* Enable V&I sensing */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CTL,
			0x20, 0x20);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CTL,
			0x20, 0x20);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CTL,
			0x0F, val);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CTL,
			0x0F, val);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CTL,
			0x10, 0x10);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CTL,
			0x10, 0x10);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CTL,
			0x20, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CTL,
			0x20, 0x00);
	}
	return 0;
}

static int lpass_cdc_wsa_macro_disable_vi_feedback(struct snd_soc_dapm_widget *w,
					struct snd_kcontrol *kcontrol,
					int event)
{
	struct snd_soc_component *component =
			snd_soc_dapm_to_component(w->dapm);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	switch (event) {
	case SND_SOC_DAPM_POST_PMD:
		if (test_bit(LPASS_CDC_WSA_MACRO_TX0,
			&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
			/* Disable V&I sensing */
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CTL,
				0x20, 0x20);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CTL,
				0x20, 0x20);
			dev_dbg(wsa_dev, "%s: spkr1 disabled\n", __func__);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CTL,
				0x10, 0x00);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CTL,
				0x10, 0x00);
		}
		if (test_bit(LPASS_CDC_WSA_MACRO_TX1,
			&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
			/* Disable V&I sensing */
			dev_dbg(wsa_dev, "%s: spkr2 disabled\n", __func__);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CTL,
				0x20, 0x20);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CTL,
				0x20, 0x20);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CTL,
				0x10, 0x00);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CTL,
				0x10, 0x00);
		}
		break;
	}

	return 0;
}

static void lpass_cdc_wsa_macro_hd2_control(struct snd_soc_component *component,
				  u16 reg, int event)
{
	u16 hd2_scale_reg;
	u16 hd2_enable_reg = 0;

	if (reg == LPASS_CDC_WSA_RX0_RX_PATH_CTL) {
		hd2_scale_reg = LPASS_CDC_WSA_RX0_RX_PATH_SEC3;
		hd2_enable_reg = LPASS_CDC_WSA_RX0_RX_PATH_CFG0;
	}
	if (reg == LPASS_CDC_WSA_RX1_RX_PATH_CTL) {
		hd2_scale_reg = LPASS_CDC_WSA_RX1_RX_PATH_SEC3;
		hd2_enable_reg = LPASS_CDC_WSA_RX1_RX_PATH_CFG0;
	}

	if (hd2_enable_reg && SND_SOC_DAPM_EVENT_ON(event)) {
		snd_soc_component_update_bits(component, hd2_scale_reg,
						0x3C, 0x10);
		snd_soc_component_update_bits(component, hd2_scale_reg,
						0x03, 0x01);
		snd_soc_component_update_bits(component, hd2_enable_reg,
						0x04, 0x04);
	}

	if (hd2_enable_reg && SND_SOC_DAPM_EVENT_OFF(event)) {
		snd_soc_component_update_bits(component, hd2_enable_reg,
						0x04, 0x00);
		snd_soc_component_update_bits(component, hd2_scale_reg,
						0x03, 0x00);
		snd_soc_component_update_bits(component, hd2_scale_reg,
						0x3C, 0x00);
	}
}

static int lpass_cdc_wsa_macro_enable_swr(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(w->dapm);
	int ch_cnt;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (!(strnstr(w->name, "RX0", sizeof("WSA_RX0"))) &&
		    !wsa_priv->rx_0_count)
			wsa_priv->rx_0_count++;
		if (!(strnstr(w->name, "RX1", sizeof("WSA_RX1"))) &&
		    !wsa_priv->rx_1_count)
			wsa_priv->rx_1_count++;
		ch_cnt = wsa_priv->rx_0_count + wsa_priv->rx_1_count;

		if (wsa_priv->swr_ctrl_data) {
			swrm_wcd_notify(
				wsa_priv->swr_ctrl_data[0].wsa_swr_pdev,
				SWR_DEVICE_UP, NULL);
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (!(strnstr(w->name, "RX0", sizeof("WSA_RX0"))) &&
		    wsa_priv->rx_0_count)
			wsa_priv->rx_0_count--;
		if (!(strnstr(w->name, "RX1", sizeof("WSA_RX1"))) &&
		    wsa_priv->rx_1_count)
			wsa_priv->rx_1_count--;
		ch_cnt = wsa_priv->rx_0_count + wsa_priv->rx_1_count;

		break;
	}
	dev_dbg(wsa_priv->dev, "%s: current swr ch cnt: %d\n",
		__func__, wsa_priv->rx_0_count + wsa_priv->rx_1_count);

	return 0;
}

static int lpass_cdc_wsa_macro_enable_mix_path(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(w->dapm);
	u16 gain_reg;
	int offset_val = 0;
	int val = 0;

	dev_dbg(component->dev, "%s %d %s\n", __func__, event, w->name);

	if (!(strcmp(w->name, "WSA_RX0 MIX INP"))) {
		gain_reg = LPASS_CDC_WSA_RX0_RX_VOL_MIX_CTL;
	} else if (!(strcmp(w->name, "WSA_RX1 MIX INP"))) {
		gain_reg = LPASS_CDC_WSA_RX1_RX_VOL_MIX_CTL;
	} else {
		dev_err(component->dev, "%s: No gain register avail for %s\n",
			__func__, w->name);
		return 0;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		lpass_cdc_wsa_macro_enable_swr(w, kcontrol, event);
		val = snd_soc_component_read(component, gain_reg);
		val += offset_val;
		snd_soc_component_write(component, gain_reg, val);
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(component,
					w->reg, 0x20, 0x00);
		lpass_cdc_wsa_macro_enable_swr(w, kcontrol, event);
		break;
	}

	return 0;
}

static int lpass_cdc_wsa_macro_config_compander(struct snd_soc_component *component,
				int comp, int event)
{
	u16 comp_ctl0_reg, comp_ctl8_reg, rx_path_cfg0_reg;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u16 mode = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (comp >= LPASS_CDC_WSA_MACRO_COMP_MAX || comp < 0) {
		dev_err(component->dev, "%s: Invalid compander value: %d\n",
					__func__, comp);
		return -EINVAL;
	}

	dev_dbg(component->dev, "%s: event %d compander %d, enabled %d\n",
		__func__, event, comp + 1, wsa_priv->comp_enabled[comp]);

	if (!wsa_priv->comp_enabled[comp])
		return 0;

	mode = wsa_priv->comp_mode[comp];
	if (mode >= WSA_MODE_MAX || mode < 0)
		mode = 0;
	comp_ctl0_reg = LPASS_CDC_WSA_COMPANDER0_CTL0 +
					(comp * LPASS_CDC_WSA_MACRO_RX_COMP_OFFSET);
	comp_ctl8_reg = LPASS_CDC_WSA_COMPANDER0_CTL8 +
					(comp * LPASS_CDC_WSA_MACRO_RX_COMP_OFFSET);
	rx_path_cfg0_reg = LPASS_CDC_WSA_RX0_RX_PATH_CFG0 +
					(comp * LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET);

	if (SND_SOC_DAPM_EVENT_ON(event)) {
		lpass_cdc_update_compander_setting(component,
					comp_ctl8_reg,
					&comp_setting_table[mode]);
		/* Enable Compander Clock */
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x01, 0x01);
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x02, 0x02);
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x02, 0x00);
		snd_soc_component_update_bits(component, rx_path_cfg0_reg,
						0x02, 0x02);
	}

	if (SND_SOC_DAPM_EVENT_OFF(event)) {
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x04, 0x04);
		snd_soc_component_update_bits(component, rx_path_cfg0_reg,
						0x02, 0x00);
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x02, 0x02);
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x02, 0x00);
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x01, 0x00);
		snd_soc_component_update_bits(component, comp_ctl0_reg,
						0x04, 0x00);
	}

	return 0;
}

static void lpass_cdc_wsa_macro_enable_softclip_clk(struct snd_soc_component *component,
					 struct lpass_cdc_wsa_macro_priv *wsa_priv,
					 int path,
					 bool enable)
{
	u16 softclip_clk_reg = LPASS_CDC_WSA_SOFTCLIP0_CRC +
			(path * LPASS_CDC_WSA_MACRO_RX_SOFTCLIP_OFFSET);
	u8 softclip_mux_mask = (1 << path);
	u8 softclip_mux_value = (1 << path);

	dev_dbg(component->dev, "%s: path %d, enable %d\n",
		__func__, path, enable);
	if (enable) {
		if (wsa_priv->softclip_clk_users[path] == 0) {
			snd_soc_component_update_bits(component,
				softclip_clk_reg, 0x01, 0x01);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX_INP_MUX_SOFTCLIP_CFG0,
				softclip_mux_mask, softclip_mux_value);
		}
		wsa_priv->softclip_clk_users[path]++;
	} else {
		wsa_priv->softclip_clk_users[path]--;
		if (wsa_priv->softclip_clk_users[path] == 0) {
			snd_soc_component_update_bits(component,
				softclip_clk_reg, 0x01, 0x00);
			snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX_INP_MUX_SOFTCLIP_CFG0,
				softclip_mux_mask, 0x00);
		}
	}
}

static int lpass_cdc_wsa_macro_config_softclip(struct snd_soc_component *component,
				int path, int event)
{
	u16 softclip_ctrl_reg = 0;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	int softclip_path = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (path == LPASS_CDC_WSA_MACRO_COMP1)
		softclip_path = LPASS_CDC_WSA_MACRO_SOFTCLIP0;
	else if (path == LPASS_CDC_WSA_MACRO_COMP2)
		softclip_path = LPASS_CDC_WSA_MACRO_SOFTCLIP1;

	dev_dbg(component->dev, "%s: event %d path %d, enabled %d\n",
		__func__, event, softclip_path,
		wsa_priv->is_softclip_on[softclip_path]);

	if (!wsa_priv->is_softclip_on[softclip_path])
		return 0;

	softclip_ctrl_reg = LPASS_CDC_WSA_SOFTCLIP0_SOFTCLIP_CTRL +
				(softclip_path * LPASS_CDC_WSA_MACRO_RX_SOFTCLIP_OFFSET);

	if (SND_SOC_DAPM_EVENT_ON(event)) {
		/* Enable Softclip clock and mux */
		lpass_cdc_wsa_macro_enable_softclip_clk(component, wsa_priv,
				softclip_path, true);
		/* Enable Softclip control */
		snd_soc_component_update_bits(component, softclip_ctrl_reg,
				0x01, 0x01);
	}

	if (SND_SOC_DAPM_EVENT_OFF(event)) {
		snd_soc_component_update_bits(component, softclip_ctrl_reg,
				0x01, 0x00);
		lpass_cdc_wsa_macro_enable_softclip_clk(component, wsa_priv,
				softclip_path, false);
	}

	return 0;
}

static bool lpass_cdc_wsa_macro_adie_lb(struct snd_soc_component *component,
			      int interp_idx)
{
	u16 int_mux_cfg0 = 0, int_mux_cfg1 = 0;
	u8 int_mux_cfg0_val = 0, int_mux_cfg1_val = 0;
	u8 int_n_inp0 = 0, int_n_inp1 = 0, int_n_inp2 = 0;

	int_mux_cfg0 = LPASS_CDC_WSA_RX_INP_MUX_RX_INT0_CFG0 + interp_idx * 8;
	int_mux_cfg1 = int_mux_cfg0 + 4;
	int_mux_cfg0_val = snd_soc_component_read(component, int_mux_cfg0);
	int_mux_cfg1_val = snd_soc_component_read(component, int_mux_cfg1);

	int_n_inp0 = int_mux_cfg0_val & 0x0F;
	if (int_n_inp0 == INTn_1_INP_SEL_DEC0 ||
		int_n_inp0 == INTn_1_INP_SEL_DEC1)
		return true;

	int_n_inp1 = int_mux_cfg0_val >> 4;
	if (int_n_inp1 == INTn_1_INP_SEL_DEC0 ||
		int_n_inp1 == INTn_1_INP_SEL_DEC1)
		return true;

	int_n_inp2 = int_mux_cfg1_val >> 4;
	if (int_n_inp2 == INTn_1_INP_SEL_DEC0 ||
		int_n_inp2 == INTn_1_INP_SEL_DEC1)
		return true;

	return false;
}

static int lpass_cdc_wsa_macro_enable_main_path(struct snd_soc_dapm_widget *w,
				      struct snd_kcontrol *kcontrol,
				      int event)
{
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(w->dapm);
	u16 reg = 0;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	bool adie_lb = false;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;


	reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL +
			LPASS_CDC_WSA_MACRO_RX_PATH_OFFSET * w->shift;
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (lpass_cdc_wsa_macro_adie_lb(component, w->shift)) {
			adie_lb = true;
			snd_soc_component_update_bits(component,
						reg, 0x20, 0x20);
			lpass_cdc_wsa_pa_on(wsa_dev, adie_lb);
		}
		break;
	default:
		break;
	}
	return 0;
}

static int lpass_cdc_wsa_macro_interp_get_primary_reg(u16 reg, u16 *ind)
{
	u16 prim_int_reg = 0;

	switch (reg) {
	case LPASS_CDC_WSA_RX0_RX_PATH_CTL:
	case LPASS_CDC_WSA_RX0_RX_PATH_MIX_CTL:
		prim_int_reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL;
		*ind = 0;
		break;
	case LPASS_CDC_WSA_RX1_RX_PATH_CTL:
	case LPASS_CDC_WSA_RX1_RX_PATH_MIX_CTL:
		prim_int_reg = LPASS_CDC_WSA_RX1_RX_PATH_CTL;
		*ind = 1;
		break;
	}

	return prim_int_reg;
}

static int lpass_cdc_wsa_macro_enable_prim_interpolator(
				struct snd_soc_component *component,
				u16 reg, int event)
{
	u16 prim_int_reg;
	u16 ind = 0;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	prim_int_reg = lpass_cdc_wsa_macro_interp_get_primary_reg(reg, &ind);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		wsa_priv->prim_int_users[ind]++;
		if (wsa_priv->prim_int_users[ind] == 1) {
			snd_soc_component_update_bits(component,
				prim_int_reg + LPASS_CDC_WSA_MACRO_RX_PATH_CFG3_OFFSET,
				0x03, 0x03);
			snd_soc_component_update_bits(component, prim_int_reg,
					    0x10, 0x10);
			lpass_cdc_wsa_macro_hd2_control(component, prim_int_reg, event);
			snd_soc_component_update_bits(component,
				prim_int_reg + LPASS_CDC_WSA_MACRO_RX_PATH_DSMDEM_OFFSET,
				0x1, 0x1);
		}
		if ((reg != prim_int_reg) &&
		    ((snd_soc_component_read(
				component, prim_int_reg)) & 0x10))
			snd_soc_component_update_bits(component, reg,
					0x10, 0x10);
		break;
	case SND_SOC_DAPM_POST_PMD:
		wsa_priv->prim_int_users[ind]--;
		if (wsa_priv->prim_int_users[ind] == 0) {
			snd_soc_component_update_bits(component, prim_int_reg,
					1 << 0x5, 0 << 0x5);
			snd_soc_component_update_bits(component,
				prim_int_reg + LPASS_CDC_WSA_MACRO_RX_PATH_DSMDEM_OFFSET,
				0x1, 0x0);
			snd_soc_component_update_bits(component, prim_int_reg,
					0x40, 0x40);
			snd_soc_component_update_bits(component, prim_int_reg,
					0x40, 0x00);
			lpass_cdc_wsa_macro_hd2_control(component, prim_int_reg, event);
		}
		break;
	}

	dev_dbg(component->dev, "%s: primary interpolator: INT%d, users: %d\n",
		__func__, ind, wsa_priv->prim_int_users[ind]);
	return 0;
}

static int lpass_cdc_wsa_macro_enable_interpolator(struct snd_soc_dapm_widget *w,
					 struct snd_kcontrol *kcontrol,
					 int event)
{
	struct snd_soc_component *component =
			snd_soc_dapm_to_component(w->dapm);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u8 gain = 0;
	u16 reg = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	dev_dbg(component->dev, "%s %d %s\n", __func__, event, w->name);

	if (!(strcmp(w->name, "WSA_RX INT0 INTERP"))) {
		reg = LPASS_CDC_WSA_RX0_RX_PATH_CTL;
	} else if (!(strcmp(w->name, "WSA_RX INT1 INTERP"))) {
		reg = LPASS_CDC_WSA_RX1_RX_PATH_CTL;
	} else {
		dev_err(component->dev, "%s: Interpolator reg not found\n",
			__func__);
		return -EINVAL;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* Reset if needed */
		lpass_cdc_wsa_macro_enable_prim_interpolator(component, reg, event);
		break;
	case SND_SOC_DAPM_POST_PMU:
		if (!strcmp(w->name, "WSA_RX INT0 INTERP")) {
			gain = (u8)(wsa_priv->rx0_origin_gain -
					wsa_priv->thermal_cur_state);
			if (snd_soc_component_read(wsa_priv->component,
					 LPASS_CDC_WSA_RX0_RX_VOL_CTL) != gain) {
				snd_soc_component_update_bits(wsa_priv->component,
					LPASS_CDC_WSA_RX0_RX_VOL_CTL, 0xFF, gain);
				dev_dbg(wsa_priv->dev,
					"%s: RX0 current thermal state: %d, "
					"adjusted gain: %#x\n",
					__func__, wsa_priv->thermal_cur_state, gain);
			}
		}

		if (!strcmp(w->name, "WSA_RX INT1 INTERP")) {
			gain = (u8)(wsa_priv->rx1_origin_gain -
					wsa_priv->thermal_cur_state);
			if (snd_soc_component_read(wsa_priv->component,
					 LPASS_CDC_WSA_RX1_RX_VOL_CTL) != gain) {
				snd_soc_component_update_bits(wsa_priv->component,
					LPASS_CDC_WSA_RX1_RX_VOL_CTL, 0xFF, gain);
				dev_dbg(wsa_priv->dev,
					"%s: RX1 current thermal state: %d, "
					"adjusted gain: %#x\n",
					__func__, wsa_priv->thermal_cur_state, gain);
			}
		}

		lpass_cdc_wsa_macro_config_compander(component, w->shift, event);
		lpass_cdc_wsa_macro_config_softclip(component, w->shift, event);
		if(wsa_priv->wsa_spkrrecv)
			snd_soc_component_update_bits(component,
					LPASS_CDC_WSA_RX0_RX_PATH_CFG1,
					0x08, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX0_RX_PATH_CFG1,	0x08, 0x08);
		lpass_cdc_wsa_macro_config_compander(component, w->shift, event);
		lpass_cdc_wsa_macro_config_softclip(component, w->shift, event);
		lpass_cdc_wsa_macro_enable_prim_interpolator(component, reg, event);
		break;
	}

	return 0;
}

static int lpass_cdc_wsa_macro_spk_boost_event(struct snd_soc_dapm_widget *w,
				     struct snd_kcontrol *kcontrol,
				     int event)
{
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(w->dapm);
	u16 boost_path_ctl, boost_path_cfg1;

	dev_dbg(component->dev, "%s %s %d\n", __func__, w->name, event);

	if (!strcmp(w->name, "WSA_RX INT0 CHAIN")) {
		boost_path_ctl = LPASS_CDC_WSA_BOOST0_BOOST_PATH_CTL;
		boost_path_cfg1 = LPASS_CDC_WSA_RX0_RX_PATH_CFG1;
	} else if (!strcmp(w->name, "WSA_RX INT1 CHAIN")) {
		boost_path_ctl = LPASS_CDC_WSA_BOOST1_BOOST_PATH_CTL;
		boost_path_cfg1 = LPASS_CDC_WSA_RX1_RX_PATH_CFG1;
	} else {
		dev_err(component->dev, "%s: unknown widget: %s\n",
			__func__, w->name);
		return -EINVAL;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(component, boost_path_cfg1,
						0x01, 0x01);
		snd_soc_component_update_bits(component, boost_path_ctl,
						0x10, 0x10);
		break;
	case SND_SOC_DAPM_POST_PMU:
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(component, boost_path_ctl,
						0x10, 0x00);
		snd_soc_component_update_bits(component, boost_path_cfg1,
						0x01, 0x00);
		break;
	}

	return 0;
}


static int lpass_cdc_wsa_macro_enable_vbat(struct snd_soc_dapm_widget *w,
				 struct snd_kcontrol *kcontrol,
				 int event)
{
	struct snd_soc_component *component =
			snd_soc_dapm_to_component(w->dapm);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u16 vbat_path_cfg = 0;
	int softclip_path = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	dev_dbg(component->dev, "%s %s %d\n", __func__, w->name, event);
	if (!strcmp(w->name, "WSA_RX INT0 VBAT")) {
		vbat_path_cfg = LPASS_CDC_WSA_RX0_RX_PATH_CFG1;
		softclip_path = LPASS_CDC_WSA_MACRO_SOFTCLIP0;
	} else if (!strcmp(w->name, "WSA_RX INT1 VBAT")) {
		vbat_path_cfg = LPASS_CDC_WSA_RX1_RX_PATH_CFG1;
		softclip_path = LPASS_CDC_WSA_MACRO_SOFTCLIP1;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* Enable clock for VBAT block */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_PATH_CTL, 0x10, 0x10);
		/* Enable VBAT block */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG, 0x01, 0x01);
		/* Update interpolator with 384K path */
		snd_soc_component_update_bits(component, vbat_path_cfg,
			0x80, 0x80);
		/* Use attenuation mode */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG, 0x02, 0x00);
		/*
		 * BCL block needs softclip clock and mux config to be enabled
		 */
		lpass_cdc_wsa_macro_enable_softclip_clk(component, wsa_priv,
					softclip_path, true);
		/* Enable VBAT at channel level */
		snd_soc_component_update_bits(component, vbat_path_cfg,
				0x02, 0x02);
		/* Set the ATTK1 gain */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD1,
			0xFF, 0xFF);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD2,
			0xFF, 0x03);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD3,
			0xFF, 0x00);
		/* Set the ATTK2 gain */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD4,
			0xFF, 0xFF);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD5,
			0xFF, 0x03);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD6,
			0xFF, 0x00);
		/* Set the ATTK3 gain */
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD7,
			0xFF, 0xFF);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD8,
			0xFF, 0x03);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD9,
			0xFF, 0x00);
                /* Enable CB decode block clock */
                snd_soc_component_update_bits(component,
                        LPASS_CDC_WSA_CB_DECODE_CB_DECODE_CTL1, 0x01, 0x01);
                /* Enable BCL path */
                snd_soc_component_update_bits(component,
                        LPASS_CDC_WSA_CB_DECODE_CB_DECODE_CTL2, 0x01, 0x01);
                /* Request for BCL data */
                snd_soc_component_update_bits(component,
                        LPASS_CDC_WSA_CB_DECODE_CB_DECODE_CTL3, 0x01, 0x01);
		break;

	case SND_SOC_DAPM_POST_PMD:
                snd_soc_component_update_bits(component,
                        LPASS_CDC_WSA_CB_DECODE_CB_DECODE_CTL3, 0x01, 0x00);
                snd_soc_component_update_bits(component,
                        LPASS_CDC_WSA_CB_DECODE_CB_DECODE_CTL2, 0x01, 0x00);
                snd_soc_component_update_bits(component,
                        LPASS_CDC_WSA_CB_DECODE_CB_DECODE_CTL1, 0x01, 0x00);
		snd_soc_component_update_bits(component, vbat_path_cfg,
			0x80, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG,
			0x02, 0x02);
		snd_soc_component_update_bits(component, vbat_path_cfg,
			0x02, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD1,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD2,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD3,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD4,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD5,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD6,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD7,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD8,
			0xFF, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_BCL_GAIN_UPD9,
			0xFF, 0x00);
		lpass_cdc_wsa_macro_enable_softclip_clk(component, wsa_priv,
			softclip_path, false);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG, 0x01, 0x00);
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_PATH_CTL, 0x10, 0x00);
		break;
	default:
		dev_err(wsa_dev, "%s: Invalid event %d\n", __func__, event);
		break;
	}
	return 0;
}

static int lpass_cdc_wsa_macro_enable_echo(struct snd_soc_dapm_widget *w,
				 struct snd_kcontrol *kcontrol,
				 int event)
{
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(w->dapm);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u16 val, ec_tx = 0, ec_hq_reg;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	dev_dbg(wsa_dev, "%s %d %s\n", __func__, event, w->name);

	val = snd_soc_component_read(component,
				LPASS_CDC_WSA_RX_INP_MUX_RX_MIX_CFG0);
	if (!(strcmp(w->name, "WSA RX_MIX EC0_MUX")))
		ec_tx = (val & 0x07) - 1;
	else
		ec_tx = ((val & 0x38) >> 0x3) - 1;

	if (ec_tx < 0 || ec_tx >= (LPASS_CDC_WSA_MACRO_RX1 + 1)) {
		dev_err(wsa_dev, "%s: EC mix control not set correctly\n",
			__func__);
		return -EINVAL;
	}
	if (wsa_priv->ec_hq[ec_tx]) {
		snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX_INP_MUX_RX_MIX_CFG0,
				0x1 << ec_tx, 0x1 << ec_tx);
		ec_hq_reg = LPASS_CDC_WSA_EC_HQ0_EC_REF_HQ_PATH_CTL +
							0x40 * ec_tx;
		snd_soc_component_update_bits(component, ec_hq_reg, 0x01, 0x01);
		ec_hq_reg = LPASS_CDC_WSA_EC_HQ0_EC_REF_HQ_CFG0 +
							0x40 * ec_tx;
		/* default set to 48k */
		snd_soc_component_update_bits(component, ec_hq_reg, 0x1E, 0x08);
	}

	return 0;
}

static int lpass_cdc_wsa_macro_get_ec_hq(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{

	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	int ec_tx = ((struct soc_multi_mixer_control *)
		    kcontrol->private_value)->shift;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	ucontrol->value.integer.value[0] = wsa_priv->ec_hq[ec_tx];
	return 0;
}

static int lpass_cdc_wsa_macro_set_ec_hq(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	int ec_tx = ((struct soc_multi_mixer_control *)
		    kcontrol->private_value)->shift;
	int value = ucontrol->value.integer.value[0];
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	dev_dbg(wsa_dev, "%s: enable current %d, new %d\n",
		__func__, wsa_priv->ec_hq[ec_tx], value);
	wsa_priv->ec_hq[ec_tx] = value;

	return 0;
}

static int lpass_cdc_wsa_macro_get_rx_mute_status(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{

	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	int wsa_rx_shift = ((struct soc_multi_mixer_control *)
		       kcontrol->private_value)->shift;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	ucontrol->value.integer.value[0] =
		wsa_priv->wsa_digital_mute_status[wsa_rx_shift];
	return 0;
}

static int lpass_cdc_wsa_macro_set_rx_mute_status(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	int value = ucontrol->value.integer.value[0];
	int wsa_rx_shift = ((struct soc_multi_mixer_control *)
			kcontrol->private_value)->shift;
	int ret = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	pm_runtime_get_sync(wsa_priv->dev);
	switch (wsa_rx_shift) {
	case 0:
		snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX0_RX_PATH_CTL,
				0x10, value << 4);
		break;
	case 1:
		snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX1_RX_PATH_CTL,
				0x10, value << 4);
		break;
	case 2:
		snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX0_RX_PATH_MIX_CTL,
				0x10, value << 4);
		break;
	case 3:
		snd_soc_component_update_bits(component,
				LPASS_CDC_WSA_RX1_RX_PATH_MIX_CTL,
				0x10, value << 4);
		break;
	default:
		pr_err("%s: invalid argument rx_shift = %d\n", __func__,
			wsa_rx_shift);
		ret = -EINVAL;
	}
	pm_runtime_mark_last_busy(wsa_priv->dev);
	pm_runtime_put_autosuspend(wsa_priv->dev);

	dev_dbg(component->dev, "%s: WSA Digital Mute RX %d Enable %d\n",
		__func__, wsa_rx_shift, value);
	wsa_priv->wsa_digital_mute_status[wsa_rx_shift] = value;

	return ret;
}

static int lpass_cdc_wsa_macro_set_digital_volume(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	struct soc_mixer_control *mc =
			(struct soc_mixer_control *)kcontrol->private_value;
	u8 gain = 0;
	int ret = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (!wsa_priv) {
		pr_err("%s: priv is null for macro!\n",
			__func__);
		return -EINVAL;
	}

	ret = snd_soc_put_volsw(kcontrol, ucontrol);

	if (mc->reg == LPASS_CDC_WSA_RX0_RX_VOL_CTL) {
		wsa_priv->rx0_origin_gain =
			(u8)snd_soc_component_read(wsa_priv->component,
							mc->reg);
		gain = (u8)(wsa_priv->rx0_origin_gain -
				wsa_priv->thermal_cur_state);
	} else if (mc->reg == LPASS_CDC_WSA_RX1_RX_VOL_CTL) {
		wsa_priv->rx1_origin_gain =
			(u8)snd_soc_component_read(wsa_priv->component,
							mc->reg);
		gain = (u8)(wsa_priv->rx1_origin_gain -
				wsa_priv->thermal_cur_state);
	} else {
		dev_err(wsa_priv->dev,
			"%s: Incorrect RX Path selected\n", __func__);
		return -EINVAL;
	}

	/* only adjust gain if thermal state is positive */
	if (wsa_priv->dapm_mclk_enable &&
	    wsa_priv->thermal_cur_state > 0) {
		snd_soc_component_update_bits(wsa_priv->component,
			mc->reg, 0xFF, gain);
		dev_dbg(wsa_priv->dev,
			"%s: Current thermal state: %d, adjusted gain: %x\n",
			__func__, wsa_priv->thermal_cur_state, gain);
	}

	return ret;
}

static int lpass_cdc_wsa_macro_get_compander(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{

	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	int comp = ((struct soc_multi_mixer_control *)
		    kcontrol->private_value)->shift;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	ucontrol->value.integer.value[0] = wsa_priv->comp_enabled[comp];
	return 0;
}

static int lpass_cdc_wsa_macro_set_compander(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	int comp = ((struct soc_multi_mixer_control *)
		    kcontrol->private_value)->shift;
	int value = ucontrol->value.integer.value[0];
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	dev_dbg(component->dev, "%s: Compander %d enable current %d, new %d\n",
		__func__, comp + 1, wsa_priv->comp_enabled[comp], value);
	wsa_priv->comp_enabled[comp] = value;

	return 0;
}

static int lpass_cdc_wsa_macro_ear_spkrrecv_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	ucontrol->value.integer.value[0] = wsa_priv->wsa_spkrrecv;

	dev_dbg(component->dev, "%s: ucontrol->value.integer.value[0] = %ld\n",
		 __func__, ucontrol->value.integer.value[0]);

	return 0;
}

static int lpass_cdc_wsa_macro_ear_spkrrecv_put(struct snd_kcontrol *kcontrol,
                                        struct snd_ctl_elem_value *ucontrol)
{
        struct snd_soc_component *component =
                                snd_soc_kcontrol_component(kcontrol);
        struct device *wsa_dev = NULL;
        struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

        if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
                return -EINVAL;

        wsa_priv->wsa_spkrrecv = ucontrol->value.integer.value[0];

        dev_dbg(component->dev, "%s:spkrrecv status = %d\n",
                 __func__, wsa_priv->wsa_spkrrecv);

        return 0;
}

static int lpass_cdc_wsa_macro_comp_mode_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u16 idx = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (strnstr(kcontrol->id.name, "RX0", sizeof("WSA_RX0")))
		idx = LPASS_CDC_WSA_MACRO_COMP1;
	if (strnstr(kcontrol->id.name, "RX1", sizeof("WSA_RX1")))
		idx = LPASS_CDC_WSA_MACRO_COMP2;
	ucontrol->value.integer.value[0] = wsa_priv->comp_mode[idx];

	dev_dbg(component->dev, "%s: ucontrol->value.integer.value[0] = %ld\n",
		__func__, ucontrol->value.integer.value[0]);

	return 0;
}

static int lpass_cdc_wsa_macro_comp_mode_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
				snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	u16 idx = 0;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (strnstr(kcontrol->id.name, "RX0", sizeof("WSA_RX0")))
		idx = LPASS_CDC_WSA_MACRO_COMP1;
	if (strnstr(kcontrol->id.name, "RX1", sizeof("WSA_RX1")))
		idx = LPASS_CDC_WSA_MACRO_COMP2;

	if (ucontrol->value.integer.value[0] < WSA_MODE_MAX && ucontrol->value.integer.value[0] >= 0)
		wsa_priv->comp_mode[idx] = ucontrol->value.integer.value[0];
	else
		return 0;

	dev_dbg(component->dev, "%s: comp_mode = %d\n", __func__,
		wsa_priv->comp_mode[idx]);

	return 0;
}

static int lpass_cdc_wsa_macro_rx_mux_get(struct snd_kcontrol *kcontrol,
			  struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_widget *widget =
		snd_soc_dapm_kcontrol_widget(kcontrol);
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(widget->dapm);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	ucontrol->value.integer.value[0] =
			wsa_priv->rx_port_value[widget->shift];
	return 0;
}

static int lpass_cdc_wsa_macro_rx_mux_put(struct snd_kcontrol *kcontrol,
			  struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_widget *widget =
		snd_soc_dapm_kcontrol_widget(kcontrol);
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(widget->dapm);
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_dapm_update *update = NULL;
	u32 rx_port_value = ucontrol->value.integer.value[0];
	u32 bit_input = 0;
	u32 aif_rst;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	aif_rst = wsa_priv->rx_port_value[widget->shift];
	if (!rx_port_value) {
		if (aif_rst == 0) {
			dev_err(wsa_dev, "%s: AIF reset already\n", __func__);
			return 0;
		}
		if (aif_rst >= LPASS_CDC_WSA_MACRO_MAX_DAIS) {
			dev_err(wsa_dev, "%s: Invalid AIF reset\n", __func__);
			return 0;
		}
	}
	wsa_priv->rx_port_value[widget->shift] = rx_port_value;

	bit_input = widget->shift;

	dev_dbg(wsa_dev,
		"%s: mux input: %d, mux output: %d, bit: %d\n",
		__func__, rx_port_value, widget->shift, bit_input);

	switch (rx_port_value) {
	case 0:
		if (wsa_priv->active_ch_cnt[aif_rst]) {
			clear_bit(bit_input,
				  &wsa_priv->active_ch_mask[aif_rst]);
			wsa_priv->active_ch_cnt[aif_rst]--;
		}
		break;
	case 1:
	case 2:
		set_bit(bit_input,
			&wsa_priv->active_ch_mask[rx_port_value]);
		wsa_priv->active_ch_cnt[rx_port_value]++;
		break;
	default:
		dev_err(wsa_dev,
			"%s: Invalid AIF_ID for WSA RX MUX %d\n",
			__func__, rx_port_value);
		return -EINVAL;
	}

	snd_soc_dapm_mux_update_power(widget->dapm, kcontrol,
					rx_port_value, e, update);
	return 0;
}

static int lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_func_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
			snd_soc_kcontrol_component(kcontrol);

	ucontrol->value.integer.value[0] =
	    ((snd_soc_component_read(
		component, LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG) & 0x04) ?
	    1 : 0);

	dev_dbg(component->dev, "%s: value: %lu\n", __func__,
		ucontrol->value.integer.value[0]);

	return 0;
}

static int lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_func_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
			snd_soc_kcontrol_component(kcontrol);

	dev_dbg(component->dev, "%s: value: %lu\n", __func__,
		ucontrol->value.integer.value[0]);

	/* Set Vbat register configuration for GSM mode bit based on value */
	if (ucontrol->value.integer.value[0])
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG,
			0x04, 0x04);
	else
		snd_soc_component_update_bits(component,
			LPASS_CDC_WSA_VBAT_BCL_VBAT_CFG,
			0x04, 0x00);

	return 0;
}

static int lpass_cdc_wsa_macro_soft_clip_enable_get(struct snd_kcontrol *kcontrol,
					  struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
			snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	int path = ((struct soc_multi_mixer_control *)
		    kcontrol->private_value)->shift;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	ucontrol->value.integer.value[0] = wsa_priv->is_softclip_on[path];

	dev_dbg(component->dev, "%s: ucontrol->value.integer.value[0] = %ld\n",
		__func__, ucontrol->value.integer.value[0]);

	return 0;
}

static int lpass_cdc_wsa_macro_soft_clip_enable_put(struct snd_kcontrol *kcontrol,
					  struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
			snd_soc_kcontrol_component(kcontrol);
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;
	int path = ((struct soc_multi_mixer_control *)
		    kcontrol->private_value)->shift;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	wsa_priv->is_softclip_on[path] =  ucontrol->value.integer.value[0];

	dev_dbg(component->dev, "%s: soft clip enable for %d: %d\n", __func__,
		path, wsa_priv->is_softclip_on[path]);

	return 0;
}

static const struct snd_kcontrol_new lpass_cdc_wsa_macro_snd_controls[] = {
	SOC_ENUM_EXT("WSA SPKRRECV", lpass_cdc_wsa_macro_ear_spkrrecv_enum,
			lpass_cdc_wsa_macro_ear_spkrrecv_get,
			lpass_cdc_wsa_macro_ear_spkrrecv_put),
	SOC_ENUM_EXT("GSM mode Enable", lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_enum,
		     lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_func_get,
		     lpass_cdc_wsa_macro_vbat_bcl_gsm_mode_func_put),
	SOC_ENUM_EXT("WSA_RX0 comp_mode", lpass_cdc_wsa_macro_comp_mode_enum,
		     lpass_cdc_wsa_macro_comp_mode_get,
		     lpass_cdc_wsa_macro_comp_mode_put),
	SOC_ENUM_EXT("WSA_RX1 comp_mode", lpass_cdc_wsa_macro_comp_mode_enum,
		     lpass_cdc_wsa_macro_comp_mode_get,
		     lpass_cdc_wsa_macro_comp_mode_put),
	SOC_SINGLE_EXT("WSA_Softclip0 Enable", SND_SOC_NOPM,
			LPASS_CDC_WSA_MACRO_SOFTCLIP0, 1, 0,
			lpass_cdc_wsa_macro_soft_clip_enable_get,
			lpass_cdc_wsa_macro_soft_clip_enable_put),
	SOC_SINGLE_EXT("WSA_Softclip1 Enable", SND_SOC_NOPM,
			LPASS_CDC_WSA_MACRO_SOFTCLIP1, 1, 0,
			lpass_cdc_wsa_macro_soft_clip_enable_get,
			lpass_cdc_wsa_macro_soft_clip_enable_put),
	LPASS_CDC_WSA_MACRO_SET_VOLUME_TLV("WSA_RX0 Digital Volume",
					   LPASS_CDC_WSA_RX0_RX_VOL_CTL,
					   -84, 40, digital_gain),
	LPASS_CDC_WSA_MACRO_SET_VOLUME_TLV("WSA_RX1 Digital Volume",
					   LPASS_CDC_WSA_RX1_RX_VOL_CTL,
					   -84, 40, digital_gain),
	SOC_SINGLE_EXT("WSA_RX0 Digital Mute", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX0, 1,
			0, lpass_cdc_wsa_macro_get_rx_mute_status,
			lpass_cdc_wsa_macro_set_rx_mute_status),
	SOC_SINGLE_EXT("WSA_RX1 Digital Mute", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX1, 1,
			0, lpass_cdc_wsa_macro_get_rx_mute_status,
			lpass_cdc_wsa_macro_set_rx_mute_status),
	SOC_SINGLE_EXT("WSA_RX0_MIX Digital Mute", SND_SOC_NOPM,
			LPASS_CDC_WSA_MACRO_RX_MIX0, 1, 0, lpass_cdc_wsa_macro_get_rx_mute_status,
			lpass_cdc_wsa_macro_set_rx_mute_status),
	SOC_SINGLE_EXT("WSA_RX1_MIX Digital Mute", SND_SOC_NOPM,
			LPASS_CDC_WSA_MACRO_RX_MIX1, 1, 0, lpass_cdc_wsa_macro_get_rx_mute_status,
			lpass_cdc_wsa_macro_set_rx_mute_status),
	SOC_SINGLE_EXT("WSA_COMP1 Switch", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_COMP1, 1, 0,
		lpass_cdc_wsa_macro_get_compander, lpass_cdc_wsa_macro_set_compander),
	SOC_SINGLE_EXT("WSA_COMP2 Switch", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_COMP2, 1, 0,
		lpass_cdc_wsa_macro_get_compander, lpass_cdc_wsa_macro_set_compander),
	SOC_SINGLE_EXT("WSA_RX0 EC_HQ Switch", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX0,
			1, 0, lpass_cdc_wsa_macro_get_ec_hq, lpass_cdc_wsa_macro_set_ec_hq),
	SOC_SINGLE_EXT("WSA_RX1 EC_HQ Switch", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX1,
			1, 0, lpass_cdc_wsa_macro_get_ec_hq, lpass_cdc_wsa_macro_set_ec_hq),
};

static const struct soc_enum rx_mux_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(rx_mux_text), rx_mux_text);

static const struct snd_kcontrol_new rx_mux[LPASS_CDC_WSA_MACRO_RX_MAX] = {
	SOC_DAPM_ENUM_EXT("WSA RX0 Mux", rx_mux_enum,
			  lpass_cdc_wsa_macro_rx_mux_get, lpass_cdc_wsa_macro_rx_mux_put),
	SOC_DAPM_ENUM_EXT("WSA RX1 Mux", rx_mux_enum,
			  lpass_cdc_wsa_macro_rx_mux_get, lpass_cdc_wsa_macro_rx_mux_put),
	SOC_DAPM_ENUM_EXT("WSA RX_MIX0 Mux", rx_mux_enum,
			  lpass_cdc_wsa_macro_rx_mux_get, lpass_cdc_wsa_macro_rx_mux_put),
	SOC_DAPM_ENUM_EXT("WSA RX_MIX1 Mux", rx_mux_enum,
			  lpass_cdc_wsa_macro_rx_mux_get, lpass_cdc_wsa_macro_rx_mux_put),
	SOC_DAPM_ENUM_EXT("WSA RX4 Mux", rx_mux_enum,
			lpass_cdc_wsa_macro_rx_mux_get, lpass_cdc_wsa_macro_rx_mux_put),
	SOC_DAPM_ENUM_EXT("WSA RX5 Mux", rx_mux_enum,
			lpass_cdc_wsa_macro_rx_mux_get, lpass_cdc_wsa_macro_rx_mux_put),
};

static int lpass_cdc_wsa_macro_vi_feed_mixer_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_widget *widget =
		snd_soc_dapm_kcontrol_widget(kcontrol);
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(widget->dapm);
	struct soc_multi_mixer_control *mixer =
		((struct soc_multi_mixer_control *)kcontrol->private_value);
	u32 dai_id = widget->shift;
	u32 spk_tx_id = mixer->shift;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	if (test_bit(spk_tx_id, &wsa_priv->active_ch_mask[dai_id]))
		ucontrol->value.integer.value[0] = 1;
	else
		ucontrol->value.integer.value[0] = 0;

	return 0;
}

static int lpass_cdc_wsa_macro_vi_feed_mixer_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_widget *widget =
		snd_soc_dapm_kcontrol_widget(kcontrol);
	struct snd_soc_component *component =
				snd_soc_dapm_to_component(widget->dapm);
	struct soc_multi_mixer_control *mixer =
		((struct soc_multi_mixer_control *)kcontrol->private_value);
	u32 spk_tx_id = mixer->shift;
	u32 enable = ucontrol->value.integer.value[0];
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	wsa_priv->vi_feed_value = ucontrol->value.integer.value[0];

	if (enable) {
		if (spk_tx_id == LPASS_CDC_WSA_MACRO_TX0 &&
			!test_bit(LPASS_CDC_WSA_MACRO_TX0,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
			set_bit(LPASS_CDC_WSA_MACRO_TX0,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI]);
			wsa_priv->active_ch_cnt[LPASS_CDC_WSA_MACRO_AIF_VI]++;
		}
		if (spk_tx_id == LPASS_CDC_WSA_MACRO_TX1 &&
			!test_bit(LPASS_CDC_WSA_MACRO_TX1,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
			set_bit(LPASS_CDC_WSA_MACRO_TX1,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI]);
			wsa_priv->active_ch_cnt[LPASS_CDC_WSA_MACRO_AIF_VI]++;
		}
	} else {
		if (spk_tx_id == LPASS_CDC_WSA_MACRO_TX0 &&
			test_bit(LPASS_CDC_WSA_MACRO_TX0,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
			clear_bit(LPASS_CDC_WSA_MACRO_TX0,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI]);
			wsa_priv->active_ch_cnt[LPASS_CDC_WSA_MACRO_AIF_VI]--;
		}
		if (spk_tx_id == LPASS_CDC_WSA_MACRO_TX1 &&
			test_bit(LPASS_CDC_WSA_MACRO_TX1,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI])) {
			clear_bit(LPASS_CDC_WSA_MACRO_TX1,
				&wsa_priv->active_ch_mask[LPASS_CDC_WSA_MACRO_AIF_VI]);
			wsa_priv->active_ch_cnt[LPASS_CDC_WSA_MACRO_AIF_VI]--;
		}
	}
	snd_soc_dapm_mixer_update_power(widget->dapm, kcontrol, enable, NULL);

	return 0;
}

static const struct snd_kcontrol_new aif_vi_mixer[] = {
	SOC_SINGLE_EXT("WSA_SPKR_VI_1", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_TX0, 1, 0,
			lpass_cdc_wsa_macro_vi_feed_mixer_get,
			lpass_cdc_wsa_macro_vi_feed_mixer_put),
	SOC_SINGLE_EXT("WSA_SPKR_VI_2", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_TX1, 1, 0,
			lpass_cdc_wsa_macro_vi_feed_mixer_get,
			lpass_cdc_wsa_macro_vi_feed_mixer_put),
};

static const struct snd_soc_dapm_widget lpass_cdc_wsa_macro_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN("WSA AIF1 PB", "WSA_AIF1 Playback", 0,
		SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_AIF_IN("WSA AIF_MIX1 PB", "WSA_AIF_MIX1 Playback", 0,
		SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_AIF_OUT_E("WSA AIF_VI", "WSA_AIF_VI Capture", 0,
		SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_AIF_VI, 0,
		lpass_cdc_wsa_macro_disable_vi_feedback,
		SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_AIF_OUT("WSA AIF_ECHO", "WSA_AIF_ECHO Capture", 0,
		SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_MIXER("WSA_AIF_VI Mixer", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_AIF_VI,
		0, aif_vi_mixer, ARRAY_SIZE(aif_vi_mixer)),
	SND_SOC_DAPM_MUX_E("WSA RX_MIX EC0_MUX", SND_SOC_NOPM,
			LPASS_CDC_WSA_MACRO_EC0_MUX, 0,
			&rx_mix_ec0_mux, lpass_cdc_wsa_macro_enable_echo,
			SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA RX_MIX EC1_MUX", SND_SOC_NOPM,
			LPASS_CDC_WSA_MACRO_EC1_MUX, 0,
			&rx_mix_ec1_mux, lpass_cdc_wsa_macro_enable_echo,
			SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MUX("WSA RX0 MUX", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX0, 0,
				&rx_mux[LPASS_CDC_WSA_MACRO_RX0]),
	SND_SOC_DAPM_MUX("WSA RX1 MUX", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX1, 0,
				&rx_mux[LPASS_CDC_WSA_MACRO_RX1]),
	SND_SOC_DAPM_MUX("WSA RX_MIX0 MUX", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX_MIX0, 0,
				&rx_mux[LPASS_CDC_WSA_MACRO_RX_MIX0]),
	SND_SOC_DAPM_MUX("WSA RX_MIX1 MUX", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX_MIX1, 0,
				&rx_mux[LPASS_CDC_WSA_MACRO_RX_MIX1]),
	SND_SOC_DAPM_MUX("WSA RX4 MUX", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX4, 0,
				&rx_mux[LPASS_CDC_WSA_MACRO_RX4]),
	SND_SOC_DAPM_MUX("WSA RX5 MUX", SND_SOC_NOPM, LPASS_CDC_WSA_MACRO_RX5, 0,
				&rx_mux[LPASS_CDC_WSA_MACRO_RX5]),

	SND_SOC_DAPM_MIXER("WSA RX0", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("WSA RX1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("WSA RX_MIX0", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("WSA RX_MIX1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("WSA RX4", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("WSA RX5", SND_SOC_NOPM, 0, 0, NULL, 0),

	SND_SOC_DAPM_MUX_E("WSA_RX0 INP0", SND_SOC_NOPM, 0, 0,
		&rx0_prim_inp0_mux, lpass_cdc_wsa_macro_enable_swr,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX0 INP1", SND_SOC_NOPM, 0, 0,
		&rx0_prim_inp1_mux, lpass_cdc_wsa_macro_enable_swr,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX0 INP2", SND_SOC_NOPM, 0, 0,
		&rx0_prim_inp2_mux, lpass_cdc_wsa_macro_enable_swr,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX0 MIX INP", SND_SOC_NOPM,
		0, 0, &rx0_mix_mux, lpass_cdc_wsa_macro_enable_mix_path,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX1 INP0", SND_SOC_NOPM, 0, 0,
		&rx1_prim_inp0_mux, lpass_cdc_wsa_macro_enable_swr,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX1 INP1", SND_SOC_NOPM, 0, 0,
		&rx1_prim_inp1_mux, lpass_cdc_wsa_macro_enable_swr,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX1 INP2", SND_SOC_NOPM, 0, 0,
		&rx1_prim_inp2_mux, lpass_cdc_wsa_macro_enable_swr,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX_E("WSA_RX1 MIX INP", SND_SOC_NOPM,
		0, 0, &rx1_mix_mux, lpass_cdc_wsa_macro_enable_mix_path,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_PGA_E("WSA_RX INT0 MIX", SND_SOC_NOPM,
			0, 0, NULL, 0, lpass_cdc_wsa_macro_enable_main_path,
			SND_SOC_DAPM_PRE_PMU),
	SND_SOC_DAPM_PGA_E("WSA_RX INT1 MIX", SND_SOC_NOPM,
			1, 0, NULL, 0, lpass_cdc_wsa_macro_enable_main_path,
			SND_SOC_DAPM_PRE_PMU),
	SND_SOC_DAPM_MIXER("WSA_RX INT0 SEC MIX", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("WSA_RX INT1 SEC MIX", SND_SOC_NOPM, 0, 0, NULL, 0),

	SND_SOC_DAPM_MUX_E("WSA_RX0 INT0 SIDETONE MIX",
			   LPASS_CDC_WSA_RX0_RX_PATH_CFG1, 4, 0,
			   &rx0_sidetone_mix_mux, lpass_cdc_wsa_macro_enable_swr,
			  SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_INPUT("WSA SRC0_INP"),

	SND_SOC_DAPM_INPUT("WSA_TX DEC0_INP"),
	SND_SOC_DAPM_INPUT("WSA_TX DEC1_INP"),

	SND_SOC_DAPM_MIXER_E("WSA_RX INT0 INTERP", SND_SOC_NOPM,
		LPASS_CDC_WSA_MACRO_COMP1, 0, NULL, 0, lpass_cdc_wsa_macro_enable_interpolator,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("WSA_RX INT1 INTERP", SND_SOC_NOPM,
		LPASS_CDC_WSA_MACRO_COMP2, 0, NULL, 0, lpass_cdc_wsa_macro_enable_interpolator,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MIXER_E("WSA_RX INT0 CHAIN", SND_SOC_NOPM, 0, 0,
		NULL, 0, lpass_cdc_wsa_macro_spk_boost_event,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("WSA_RX INT1 CHAIN", SND_SOC_NOPM, 0, 0,
		NULL, 0, lpass_cdc_wsa_macro_spk_boost_event,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_MIXER_E("WSA_RX INT0 VBAT", SND_SOC_NOPM,
		0, 0, wsa_int0_vbat_mix_switch,
		ARRAY_SIZE(wsa_int0_vbat_mix_switch),
		lpass_cdc_wsa_macro_enable_vbat,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER_E("WSA_RX INT1 VBAT", SND_SOC_NOPM,
		0, 0, wsa_int1_vbat_mix_switch,
		ARRAY_SIZE(wsa_int1_vbat_mix_switch),
		lpass_cdc_wsa_macro_enable_vbat,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),

	SND_SOC_DAPM_INPUT("VIINPUT_WSA"),

	SND_SOC_DAPM_OUTPUT("WSA_SPK1 OUT"),
	SND_SOC_DAPM_OUTPUT("WSA_SPK2 OUT"),

	SND_SOC_DAPM_SUPPLY_S("WSA_MCLK", 0, SND_SOC_NOPM, 0, 0,
	lpass_cdc_wsa_macro_mclk_event, SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),
};

static const struct snd_soc_dapm_route wsa_audio_map[] = {
	/* VI Feedback */
	{"WSA_AIF_VI Mixer", "WSA_SPKR_VI_1", "VIINPUT_WSA"},
	{"WSA_AIF_VI Mixer", "WSA_SPKR_VI_2", "VIINPUT_WSA"},
	{"WSA AIF_VI", NULL, "WSA_AIF_VI Mixer"},
	{"WSA AIF_VI", NULL, "WSA_MCLK"},

	{"WSA RX_MIX EC0_MUX", "RX_MIX_TX0", "WSA_RX INT0 SEC MIX"},
	{"WSA RX_MIX EC1_MUX", "RX_MIX_TX0", "WSA_RX INT0 SEC MIX"},
	{"WSA RX_MIX EC0_MUX", "RX_MIX_TX1", "WSA_RX INT1 SEC MIX"},
	{"WSA RX_MIX EC1_MUX", "RX_MIX_TX1", "WSA_RX INT1 SEC MIX"},
	{"WSA AIF_ECHO", NULL, "WSA RX_MIX EC0_MUX"},
	{"WSA AIF_ECHO", NULL, "WSA RX_MIX EC1_MUX"},
	{"WSA AIF_ECHO", NULL, "WSA_MCLK"},

	{"WSA AIF1 PB", NULL, "WSA_MCLK"},
	{"WSA AIF_MIX1 PB", NULL, "WSA_MCLK"},

	{"WSA RX0 MUX", "AIF1_PB", "WSA AIF1 PB"},
	{"WSA RX1 MUX", "AIF1_PB", "WSA AIF1 PB"},
	{"WSA RX_MIX0 MUX", "AIF1_PB", "WSA AIF1 PB"},
	{"WSA RX_MIX1 MUX", "AIF1_PB", "WSA AIF1 PB"},
	{"WSA RX4 MUX", "AIF1_PB", "WSA AIF1 PB"},
	{"WSA RX5 MUX", "AIF1_PB", "WSA AIF1 PB"},

	{"WSA RX0 MUX", "AIF_MIX1_PB", "WSA AIF_MIX1 PB"},
	{"WSA RX1 MUX", "AIF_MIX1_PB", "WSA AIF_MIX1 PB"},
	{"WSA RX_MIX0 MUX", "AIF_MIX1_PB", "WSA AIF_MIX1 PB"},
	{"WSA RX_MIX1 MUX", "AIF_MIX1_PB", "WSA AIF_MIX1 PB"},
	{"WSA RX4 MUX", "AIF_MIX1_PB", "WSA AIF_MIX1 PB"},
	{"WSA RX5 MUX", "AIF_MIX1_PB", "WSA AIF_MIX1 PB"},

	{"WSA RX0", NULL, "WSA RX0 MUX"},
	{"WSA RX1", NULL, "WSA RX1 MUX"},
	{"WSA RX_MIX0", NULL, "WSA RX_MIX0 MUX"},
	{"WSA RX_MIX1", NULL, "WSA RX_MIX1 MUX"},
	{"WSA RX4", NULL, "WSA RX4 MUX"},
	{"WSA RX5", NULL, "WSA RX5 MUX"},

	{"WSA_RX0 INP0", "RX0", "WSA RX0"},
	{"WSA_RX0 INP0", "RX1", "WSA RX1"},
	{"WSA_RX0 INP0", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX0 INP0", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX0 INP0", "RX4", "WSA RX4"},
	{"WSA_RX0 INP0", "RX5", "WSA RX5"},
	{"WSA_RX0 INP0", "DEC0", "WSA_TX DEC0_INP"},
	{"WSA_RX0 INP0", "DEC1", "WSA_TX DEC1_INP"},
	{"WSA_RX INT0 MIX", NULL, "WSA_RX0 INP0"},

	{"WSA_RX0 INP1", "RX0", "WSA RX0"},
	{"WSA_RX0 INP1", "RX1", "WSA RX1"},
	{"WSA_RX0 INP1", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX0 INP1", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX0 INP1", "RX4", "WSA RX4"},
	{"WSA_RX0 INP1", "RX5", "WSA RX5"},
	{"WSA_RX0 INP1", "DEC0", "WSA_TX DEC0_INP"},
	{"WSA_RX0 INP1", "DEC1", "WSA_TX DEC1_INP"},
	{"WSA_RX INT0 MIX", NULL, "WSA_RX0 INP1"},

	{"WSA_RX0 INP2", "RX0", "WSA RX0"},
	{"WSA_RX0 INP2", "RX1", "WSA RX1"},
	{"WSA_RX0 INP2", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX0 INP2", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX0 INP2", "RX4", "WSA RX4"},
	{"WSA_RX0 INP2", "RX5", "WSA RX5"},
	{"WSA_RX0 INP2", "DEC0", "WSA_TX DEC0_INP"},
	{"WSA_RX0 INP2", "DEC1", "WSA_TX DEC1_INP"},
	{"WSA_RX INT0 MIX", NULL, "WSA_RX0 INP2"},

	{"WSA_RX0 MIX INP", "RX0", "WSA RX0"},
	{"WSA_RX0 MIX INP", "RX1", "WSA RX1"},
	{"WSA_RX0 MIX INP", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX0 MIX INP", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX0 MIX INP", "RX4", "WSA RX4"},
	{"WSA_RX0 MIX INP", "RX5", "WSA RX5"},
	{"WSA_RX INT0 SEC MIX", NULL, "WSA_RX0 MIX INP"},

	{"WSA_RX INT0 SEC MIX", NULL, "WSA_RX INT0 MIX"},
	{"WSA_RX INT0 INTERP", NULL, "WSA_RX INT0 SEC MIX"},
	{"WSA_RX0 INT0 SIDETONE MIX", "SRC0", "WSA SRC0_INP"},
	{"WSA_RX INT0 INTERP", NULL, "WSA_RX0 INT0 SIDETONE MIX"},
	{"WSA_RX INT0 CHAIN", NULL, "WSA_RX INT0 INTERP"},

	{"WSA_RX INT0 VBAT", "WSA RX0 VBAT Enable", "WSA_RX INT0 INTERP"},
	{"WSA_RX INT0 CHAIN", NULL, "WSA_RX INT0 VBAT"},

	{"WSA_SPK1 OUT", NULL, "WSA_RX INT0 CHAIN"},
	{"WSA_SPK1 OUT", NULL, "WSA_MCLK"},

	{"WSA_RX1 INP0", "RX0", "WSA RX0"},
	{"WSA_RX1 INP0", "RX1", "WSA RX1"},
	{"WSA_RX1 INP0", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX1 INP0", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX1 INP0", "RX4", "WSA RX4"},
	{"WSA_RX1 INP0", "RX5", "WSA RX5"},
	{"WSA_RX1 INP0", "DEC0", "WSA_TX DEC0_INP"},
	{"WSA_RX1 INP0", "DEC1", "WSA_TX DEC1_INP"},
	{"WSA_RX INT1 MIX", NULL, "WSA_RX1 INP0"},

	{"WSA_RX1 INP1", "RX0", "WSA RX0"},
	{"WSA_RX1 INP1", "RX1", "WSA RX1"},
	{"WSA_RX1 INP1", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX1 INP1", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX1 INP1", "RX4", "WSA RX4"},
	{"WSA_RX1 INP1", "RX5", "WSA RX5"},
	{"WSA_RX1 INP1", "DEC0", "WSA_TX DEC0_INP"},
	{"WSA_RX1 INP1", "DEC1", "WSA_TX DEC1_INP"},
	{"WSA_RX INT1 MIX", NULL, "WSA_RX1 INP1"},

	{"WSA_RX1 INP2", "RX0", "WSA RX0"},
	{"WSA_RX1 INP2", "RX1", "WSA RX1"},
	{"WSA_RX1 INP2", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX1 INP2", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX1 INP2", "RX4", "WSA RX4"},
	{"WSA_RX1 INP2", "RX5", "WSA RX5"},
	{"WSA_RX1 INP2", "DEC0", "WSA_TX DEC0_INP"},
	{"WSA_RX1 INP2", "DEC1", "WSA_TX DEC1_INP"},
	{"WSA_RX INT1 MIX", NULL, "WSA_RX1 INP2"},

	{"WSA_RX1 MIX INP", "RX0", "WSA RX0"},
	{"WSA_RX1 MIX INP", "RX1", "WSA RX1"},
	{"WSA_RX1 MIX INP", "RX_MIX0", "WSA RX_MIX0"},
	{"WSA_RX1 MIX INP", "RX_MIX1", "WSA RX_MIX1"},
	{"WSA_RX1 MIX INP", "RX4", "WSA RX4"},
	{"WSA_RX1 MIX INP", "RX5", "WSA RX5"},
	{"WSA_RX INT1 SEC MIX", NULL, "WSA_RX1 MIX INP"},

	{"WSA_RX INT1 SEC MIX", NULL, "WSA_RX INT1 MIX"},
	{"WSA_RX INT1 INTERP", NULL, "WSA_RX INT1 SEC MIX"},

	{"WSA_RX INT1 VBAT", "WSA RX1 VBAT Enable", "WSA_RX INT1 INTERP"},
	{"WSA_RX INT1 CHAIN", NULL, "WSA_RX INT1 VBAT"},

	{"WSA_RX INT1 CHAIN", NULL, "WSA_RX INT1 INTERP"},
	{"WSA_SPK2 OUT", NULL, "WSA_RX INT1 CHAIN"},
	{"WSA_SPK2 OUT", NULL, "WSA_MCLK"},
};

static const struct lpass_cdc_wsa_macro_reg_mask_val
				lpass_cdc_wsa_macro_reg_init[] = {
	{LPASS_CDC_WSA_BOOST0_BOOST_CFG1, 0x3F, 0x12},
	{LPASS_CDC_WSA_BOOST0_BOOST_CFG2, 0x1C, 0x08},
	{LPASS_CDC_WSA_COMPANDER0_CTL7, 0x1E, 0x18},
	{LPASS_CDC_WSA_BOOST1_BOOST_CFG1, 0x3F, 0x12},
	{LPASS_CDC_WSA_BOOST1_BOOST_CFG2, 0x1C, 0x08},
	{LPASS_CDC_WSA_COMPANDER1_CTL7, 0x1E, 0x18},
	{LPASS_CDC_WSA_BOOST0_BOOST_CTL, 0x70, 0x58},
	{LPASS_CDC_WSA_BOOST1_BOOST_CTL, 0x70, 0x58},
	{LPASS_CDC_WSA_RX0_RX_PATH_CFG1, 0x08, 0x08},
	{LPASS_CDC_WSA_RX1_RX_PATH_CFG1, 0x08, 0x08},
	{LPASS_CDC_WSA_TOP_TOP_CFG1, 0x02, 0x02},
	{LPASS_CDC_WSA_TOP_TOP_CFG1, 0x01, 0x01},
	{LPASS_CDC_WSA_TX0_SPKR_PROT_PATH_CFG0, 0x01, 0x01},
	{LPASS_CDC_WSA_TX1_SPKR_PROT_PATH_CFG0, 0x01, 0x01},
	{LPASS_CDC_WSA_TX2_SPKR_PROT_PATH_CFG0, 0x01, 0x01},
	{LPASS_CDC_WSA_TX3_SPKR_PROT_PATH_CFG0, 0x01, 0x01},
	{LPASS_CDC_WSA_COMPANDER0_CTL7, 0x01, 0x01},
	{LPASS_CDC_WSA_COMPANDER1_CTL7, 0x01, 0x01},
	{LPASS_CDC_WSA_RX0_RX_PATH_CFG0, 0x01, 0x01},
	{LPASS_CDC_WSA_RX1_RX_PATH_CFG0, 0x01, 0x01},
	{LPASS_CDC_WSA_RX0_RX_PATH_MIX_CFG, 0x01, 0x01},
	{LPASS_CDC_WSA_RX1_RX_PATH_MIX_CFG, 0x01, 0x01},
};

static void lpass_cdc_wsa_macro_init_reg(struct snd_soc_component *component)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lpass_cdc_wsa_macro_reg_init); i++)
		snd_soc_component_update_bits(component,
				lpass_cdc_wsa_macro_reg_init[i].reg,
				lpass_cdc_wsa_macro_reg_init[i].mask,
				lpass_cdc_wsa_macro_reg_init[i].val);
}

static int lpass_cdc_wsa_macro_core_vote(void *handle, bool enable)
{
	int rc = 0;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = (struct lpass_cdc_wsa_macro_priv *) handle;

	if (wsa_priv == NULL) {
		pr_err("%s: wsa priv data is NULL\n", __func__);
		return -EINVAL;
	}
	if (!wsa_priv->pre_dev_up && enable) {
		pr_debug("%s: adsp is not up\n", __func__);
		return -EINVAL;
	}

	if (enable) {
		pm_runtime_get_sync(wsa_priv->dev);
		if (lpass_cdc_check_core_votes(wsa_priv->dev))
			rc = 0;
		else
			rc = -ENOTSYNC;
	} else {
		pm_runtime_put_autosuspend(wsa_priv->dev);
		pm_runtime_mark_last_busy(wsa_priv->dev);
	}

	return rc;
}

static int wsa_swrm_clock(void *handle, bool enable)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv = (struct lpass_cdc_wsa_macro_priv *) handle;
	struct regmap *regmap = dev_get_regmap(wsa_priv->dev->parent, NULL);
	int ret = 0;

	if (regmap == NULL) {
		dev_err(wsa_priv->dev, "%s: regmap is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&wsa_priv->swr_clk_lock);

	trace_printk("%s: %s swrm clock %s\n",
		dev_name(wsa_priv->dev), __func__,
		(enable ? "enable" : "disable"));
	dev_dbg(wsa_priv->dev, "%s: swrm clock %s\n",
		__func__, (enable ? "enable" : "disable"));
	if (enable) {
		pm_runtime_get_sync(wsa_priv->dev);
		if (wsa_priv->swr_clk_users == 0) {
			ret = msm_cdc_pinctrl_select_active_state(
						wsa_priv->wsa_swr_gpio_p);
			if (ret < 0) {
				dev_err_ratelimited(wsa_priv->dev,
					"%s: wsa swr pinctrl enable failed\n",
					__func__);
				pm_runtime_mark_last_busy(wsa_priv->dev);
				pm_runtime_put_autosuspend(wsa_priv->dev);
				goto exit;
			}
			ret = lpass_cdc_wsa_macro_mclk_enable(wsa_priv, 1, true);
			if (ret < 0) {
				msm_cdc_pinctrl_select_sleep_state(
						wsa_priv->wsa_swr_gpio_p);
				dev_err_ratelimited(wsa_priv->dev,
					"%s: wsa request clock enable failed\n",
					__func__);
				pm_runtime_mark_last_busy(wsa_priv->dev);
				pm_runtime_put_autosuspend(wsa_priv->dev);
				goto exit;
			}
			if (wsa_priv->reset_swr)
				regmap_update_bits(regmap,
					LPASS_CDC_WSA_CLK_RST_CTRL_SWR_CONTROL,
					0x02, 0x02);
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_SWR_CONTROL,
				0x01, 0x01);
			if (wsa_priv->reset_swr)
				regmap_update_bits(regmap,
					LPASS_CDC_WSA_CLK_RST_CTRL_SWR_CONTROL,
					0x02, 0x00);
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_SWR_CONTROL,
				0x1C, 0x0C);
			wsa_priv->reset_swr = false;
		}
		wsa_priv->swr_clk_users++;
		pm_runtime_mark_last_busy(wsa_priv->dev);
		pm_runtime_put_autosuspend(wsa_priv->dev);
	} else {
		if (wsa_priv->swr_clk_users <= 0) {
			dev_err(wsa_priv->dev, "%s: clock already disabled\n",
			__func__);
			wsa_priv->swr_clk_users = 0;
			goto exit;
		}
		wsa_priv->swr_clk_users--;
		if (wsa_priv->swr_clk_users == 0) {
			regmap_update_bits(regmap,
				LPASS_CDC_WSA_CLK_RST_CTRL_SWR_CONTROL,
				0x01, 0x00);
			lpass_cdc_wsa_macro_mclk_enable(wsa_priv, 0, true);
			ret = msm_cdc_pinctrl_select_sleep_state(
						wsa_priv->wsa_swr_gpio_p);
			if (ret < 0) {
				dev_err_ratelimited(wsa_priv->dev,
					"%s: wsa swr pinctrl disable failed\n",
					__func__);
				goto exit;
			}
		}
	}
	trace_printk("%s: %s swrm clock users: %d\n",
		dev_name(wsa_priv->dev), __func__,
		wsa_priv->swr_clk_users);
	dev_dbg(wsa_priv->dev, "%s: swrm clock users %d\n",
		__func__, wsa_priv->swr_clk_users);
exit:
	mutex_unlock(&wsa_priv->swr_clk_lock);
	return ret;
}

/* Thermal Functions */
static int lpass_cdc_wsa_macro_get_max_state(
					struct thermal_cooling_device *cdev,
					unsigned long *state)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv = cdev->devdata;
	if (!wsa_priv) {
		pr_err("%s: cdev->devdata is NULL\n", __func__);
		return -EINVAL;
	}
	*state = wsa_priv->thermal_max_state;

	return 0;
}

static int lpass_cdc_wsa_macro_get_cur_state(
					struct thermal_cooling_device *cdev,
					unsigned long *state)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv = cdev->devdata;

	if (!wsa_priv) {
		pr_err("%s: cdev->devdata is NULL\n", __func__);
		return -EINVAL;
	}
	*state = wsa_priv->thermal_cur_state;

	pr_debug("%s: thermal current state:%lu\n", __func__, *state);
	return 0;
}

static int lpass_cdc_wsa_macro_set_cur_state(
					struct thermal_cooling_device *cdev,
					unsigned long state)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv = cdev->devdata;

	if (!wsa_priv || !wsa_priv->dev) {
		pr_err("%s: cdev->devdata is NULL\n", __func__);
		return -EINVAL;
	}

	if (state <= wsa_priv->thermal_max_state) {
		wsa_priv->thermal_cur_state = state;
	} else {
		dev_err(wsa_priv->dev,
			"%s: incorrect requested state:%d\n",
			__func__, state);
		return -EINVAL;
	}

	dev_dbg(wsa_priv->dev,
		"%s: set the thermal current state to %d\n",
		__func__, wsa_priv->thermal_cur_state);

	schedule_work(&wsa_priv->lpass_cdc_wsa_macro_cooling_work);

	return 0;
}

static struct thermal_cooling_device_ops wsa_cooling_ops = {
	.get_max_state = lpass_cdc_wsa_macro_get_max_state,
	.get_cur_state = lpass_cdc_wsa_macro_get_cur_state,
	.set_cur_state = lpass_cdc_wsa_macro_set_cur_state,
};

static int lpass_cdc_wsa_macro_init(struct snd_soc_component *component)
{
	struct snd_soc_dapm_context *dapm =
				snd_soc_component_get_dapm(component);
	int ret;
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	wsa_dev = lpass_cdc_get_device_ptr(component->dev, WSA_MACRO);
	if (!wsa_dev) {
		dev_err(component->dev,
			"%s: null device for macro!\n", __func__);
		return -EINVAL;
	}
	wsa_priv = dev_get_drvdata(wsa_dev);
	if (!wsa_priv) {
		dev_err(component->dev,
			"%s: priv is null for macro!\n", __func__);
		return -EINVAL;
	}

	ret = snd_soc_dapm_new_controls(dapm, lpass_cdc_wsa_macro_dapm_widgets,
					ARRAY_SIZE(lpass_cdc_wsa_macro_dapm_widgets));
	if (ret < 0) {
		dev_err(wsa_dev, "%s: Failed to add controls\n", __func__);
		return ret;
	}

	ret = snd_soc_dapm_add_routes(dapm, wsa_audio_map,
					ARRAY_SIZE(wsa_audio_map));
	if (ret < 0) {
		dev_err(wsa_dev, "%s: Failed to add routes\n", __func__);
		return ret;
	}

	ret = snd_soc_dapm_new_widgets(dapm->card);
	if (ret < 0) {
		dev_err(wsa_dev, "%s: Failed to add widgets\n", __func__);
		return ret;
	}

	ret = snd_soc_add_component_controls(component, lpass_cdc_wsa_macro_snd_controls,
				   ARRAY_SIZE(lpass_cdc_wsa_macro_snd_controls));
	if (ret < 0) {
		dev_err(wsa_dev, "%s: Failed to add snd_ctls\n", __func__);
		return ret;
	}
	snd_soc_dapm_ignore_suspend(dapm, "WSA_AIF1 Playback");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_AIF_MIX1 Playback");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_AIF_VI Capture");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_AIF_ECHO Capture");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_SPK1 OUT");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_SPK2 OUT");
	snd_soc_dapm_ignore_suspend(dapm, "VIINPUT_WSA");
	snd_soc_dapm_ignore_suspend(dapm, "WSA SRC0_INP");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_TX DEC0_INP");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_TX DEC1_INP");
	snd_soc_dapm_sync(dapm);

	wsa_priv->component = component;
	wsa_priv->spkr_gain_offset = LPASS_CDC_WSA_MACRO_GAIN_OFFSET_0_DB;
	lpass_cdc_wsa_macro_init_reg(component);

	return 0;
}

static int lpass_cdc_wsa_macro_deinit(struct snd_soc_component *component)
{
	struct device *wsa_dev = NULL;
	struct lpass_cdc_wsa_macro_priv *wsa_priv = NULL;

	if (!lpass_cdc_wsa_macro_get_data(component, &wsa_dev, &wsa_priv, __func__))
		return -EINVAL;

	wsa_priv->component = NULL;

	return 0;
}

static void lpass_cdc_wsa_macro_add_child_devices(struct work_struct *work)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv;
	struct platform_device *pdev;
	struct device_node *node;
	struct lpass_cdc_wsa_macro_swr_ctrl_data *swr_ctrl_data = NULL, *temp;
	int ret;
	u16 count = 0, ctrl_num = 0;
	struct lpass_cdc_wsa_macro_swr_ctrl_platform_data *platdata;
	char plat_dev_name[LPASS_CDC_WSA_MACRO_SWR_STRING_LEN];

	wsa_priv = container_of(work, struct lpass_cdc_wsa_macro_priv,
			     lpass_cdc_wsa_macro_add_child_devices_work);
	if (!wsa_priv) {
		pr_err("%s: Memory for wsa_priv does not exist\n",
			__func__);
		return;
	}
	if (!wsa_priv->dev || !wsa_priv->dev->of_node) {
		dev_err(wsa_priv->dev,
			"%s: DT node for wsa_priv does not exist\n", __func__);
		return;
	}

	platdata = &wsa_priv->swr_plat_data;
	wsa_priv->child_count = 0;

	for_each_available_child_of_node(wsa_priv->dev->of_node, node) {
		if (strnstr(node->name, "wsa_swr_master",
				strlen("wsa_swr_master")) != NULL)
			strlcpy(plat_dev_name, "wsa_swr_ctrl",
				(LPASS_CDC_WSA_MACRO_SWR_STRING_LEN - 1));
		else if (strnstr(node->name, "msm_cdc_pinctrl",
				 strlen("msm_cdc_pinctrl")) != NULL)
			strlcpy(plat_dev_name, node->name,
				(LPASS_CDC_WSA_MACRO_SWR_STRING_LEN - 1));
		else
			continue;

		pdev = platform_device_alloc(plat_dev_name, -1);
		if (!pdev) {
			dev_err(wsa_priv->dev, "%s: pdev memory alloc failed\n",
				__func__);
			ret = -ENOMEM;
			goto err;
		}
		pdev->dev.parent = wsa_priv->dev;
		pdev->dev.of_node = node;

		if (strnstr(node->name, "wsa_swr_master",
				strlen("wsa_swr_master")) != NULL) {
			ret = platform_device_add_data(pdev, platdata,
						       sizeof(*platdata));
			if (ret) {
				dev_err(&pdev->dev,
					"%s: cannot add plat data ctrl:%d\n",
					__func__, ctrl_num);
				goto fail_pdev_add;
			}

			temp = krealloc(swr_ctrl_data,
					(ctrl_num + 1) * sizeof(
					struct lpass_cdc_wsa_macro_swr_ctrl_data),
					GFP_KERNEL);
			if (!temp) {
				dev_err(&pdev->dev, "out of memory\n");
				ret = -ENOMEM;
				goto fail_pdev_add;
			}
			swr_ctrl_data = temp;
			swr_ctrl_data[ctrl_num].wsa_swr_pdev = pdev;
			ctrl_num++;
			dev_dbg(&pdev->dev,
				"%s: Adding soundwire ctrl device(s)\n",
				__func__);
			wsa_priv->swr_ctrl_data = swr_ctrl_data;
		}

		ret = platform_device_add(pdev);
		if (ret) {
			dev_err(&pdev->dev,
				"%s: Cannot add platform device\n",
				__func__);
			goto fail_pdev_add;
		}

		if (wsa_priv->child_count < LPASS_CDC_WSA_MACRO_CHILD_DEVICES_MAX)
			wsa_priv->pdev_child_devices[
					wsa_priv->child_count++] = pdev;
		else
			goto err;
	}

	return;
fail_pdev_add:
	for (count = 0; count < wsa_priv->child_count; count++)
		platform_device_put(wsa_priv->pdev_child_devices[count]);
err:
	return;
}

static void lpass_cdc_wsa_macro_cooling_adjust_gain(struct work_struct *work)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv;
	u8 gain = 0;

	wsa_priv = container_of(work, struct lpass_cdc_wsa_macro_priv,
			     lpass_cdc_wsa_macro_cooling_work);
	if (!wsa_priv) {
		pr_err("%s: priv is null for macro!\n",
			__func__);
		return;
	}
	if (!wsa_priv->dev || !wsa_priv->dev->of_node) {
		dev_err(wsa_priv->dev,
			"%s: DT node for wsa_priv does not exist\n", __func__);
		return;
	}

	/* Only adjust the volume when WSA clock is enabled */
	if (wsa_priv->dapm_mclk_enable) {
		gain = (u8)(wsa_priv->rx0_origin_gain -
				wsa_priv->thermal_cur_state);
		snd_soc_component_update_bits(wsa_priv->component,
			LPASS_CDC_WSA_RX0_RX_VOL_CTL, 0xFF, gain);
		dev_dbg(wsa_priv->dev,
			"%s: RX0 current thermal state: %d, "
			"adjusted gain: %#x\n",
			__func__, wsa_priv->thermal_cur_state, gain);

		gain = (u8)(wsa_priv->rx1_origin_gain -
				wsa_priv->thermal_cur_state);
		snd_soc_component_update_bits(wsa_priv->component,
			LPASS_CDC_WSA_RX1_RX_VOL_CTL, 0xFF, gain);
		dev_dbg(wsa_priv->dev,
			"%s: RX1 current thermal state: %d, "
			"adjusted gain: %#x\n",
			__func__, wsa_priv->thermal_cur_state, gain);
	}

	return;
}

static void lpass_cdc_wsa_macro_init_ops(struct macro_ops *ops,
			       char __iomem *wsa_io_base)
{
	memset(ops, 0, sizeof(struct macro_ops));
	ops->init = lpass_cdc_wsa_macro_init;
	ops->exit = lpass_cdc_wsa_macro_deinit;
	ops->io_base = wsa_io_base;
	ops->dai_ptr = lpass_cdc_wsa_macro_dai;
	ops->num_dais = ARRAY_SIZE(lpass_cdc_wsa_macro_dai);
	ops->event_handler = lpass_cdc_wsa_macro_event_handler;
	ops->set_port_map = lpass_cdc_wsa_macro_set_port_map;
}

static int lpass_cdc_wsa_macro_probe(struct platform_device *pdev)
{
	struct macro_ops ops;
	struct lpass_cdc_wsa_macro_priv *wsa_priv;
	u32 wsa_base_addr, default_clk_id, thermal_max_state;
	char __iomem *wsa_io_base;
	int ret = 0;
	u32 is_used_wsa_swr_gpio = 1;
	const char *is_used_wsa_swr_gpio_dt = "qcom,is-used-swr-gpio";

	if (!lpass_cdc_is_va_macro_registered(&pdev->dev)) {
		dev_err(&pdev->dev,
			"%s: va-macro not registered yet, defer\n", __func__);
		return -EPROBE_DEFER;
	}

	wsa_priv = devm_kzalloc(&pdev->dev, sizeof(struct lpass_cdc_wsa_macro_priv),
				GFP_KERNEL);
	if (!wsa_priv)
		return -ENOMEM;

	wsa_priv->pre_dev_up = true;
	wsa_priv->dev = &pdev->dev;
	ret = of_property_read_u32(pdev->dev.of_node, "reg",
				   &wsa_base_addr);
	if (ret) {
		dev_err(&pdev->dev, "%s: could not find %s entry in dt\n",
			__func__, "reg");
		return ret;
	}
	if (of_find_property(pdev->dev.of_node, is_used_wsa_swr_gpio_dt,
			     NULL)) {
		ret = of_property_read_u32(pdev->dev.of_node,
					   is_used_wsa_swr_gpio_dt,
					   &is_used_wsa_swr_gpio);
		if (ret) {
			dev_err(&pdev->dev, "%s: error reading %s in dt\n",
				__func__, is_used_wsa_swr_gpio_dt);
			is_used_wsa_swr_gpio = 1;
		}
	}
	wsa_priv->wsa_swr_gpio_p = of_parse_phandle(pdev->dev.of_node,
					"qcom,wsa-swr-gpios", 0);
	if (!wsa_priv->wsa_swr_gpio_p && is_used_wsa_swr_gpio) {
		dev_err(&pdev->dev, "%s: swr_gpios handle not provided!\n",
			__func__);
		return -EINVAL;
	}
	if (msm_cdc_pinctrl_get_state(wsa_priv->wsa_swr_gpio_p) < 0 &&
			is_used_wsa_swr_gpio) {
		dev_err(&pdev->dev, "%s: failed to get swr pin state\n",
			__func__);
		return -EPROBE_DEFER;
	}
	msm_cdc_pinctrl_set_wakeup_capable(
				wsa_priv->wsa_swr_gpio_p, false);

	wsa_io_base = devm_ioremap(&pdev->dev,
				   wsa_base_addr, LPASS_CDC_WSA_MACRO_MAX_OFFSET);
	if (!wsa_io_base) {
		dev_err(&pdev->dev, "%s: ioremap failed\n", __func__);
		return -EINVAL;
	}
	wsa_priv->wsa_io_base = wsa_io_base;
	wsa_priv->reset_swr = true;
	INIT_WORK(&wsa_priv->lpass_cdc_wsa_macro_add_child_devices_work,
		  lpass_cdc_wsa_macro_add_child_devices);
	INIT_WORK(&wsa_priv->lpass_cdc_wsa_macro_cooling_work,
		  lpass_cdc_wsa_macro_cooling_adjust_gain);
	wsa_priv->swr_plat_data.handle = (void *) wsa_priv;
	wsa_priv->swr_plat_data.read = NULL;
	wsa_priv->swr_plat_data.write = NULL;
	wsa_priv->swr_plat_data.bulk_write = NULL;
	wsa_priv->swr_plat_data.clk = wsa_swrm_clock;
	wsa_priv->swr_plat_data.core_vote = lpass_cdc_wsa_macro_core_vote;
	wsa_priv->swr_plat_data.handle_irq = NULL;

	ret = of_property_read_u32(pdev->dev.of_node, "qcom,default-clk-id",
				   &default_clk_id);
	if (ret) {
		dev_err(&pdev->dev, "%s: could not find %s entry in dt\n",
			__func__, "qcom,mux0-clk-id");
		default_clk_id = WSA_CORE_CLK;
	}

	wsa_priv->default_clk_id  = default_clk_id;

	dev_set_drvdata(&pdev->dev, wsa_priv);
	mutex_init(&wsa_priv->mclk_lock);
	mutex_init(&wsa_priv->swr_clk_lock);
	lpass_cdc_wsa_macro_init_ops(&ops, wsa_io_base);
	ops.clk_id_req = wsa_priv->default_clk_id;
	ops.default_clk_id = wsa_priv->default_clk_id;

	ret = lpass_cdc_register_macro(&pdev->dev, WSA_MACRO, &ops);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: register macro failed\n", __func__);
		goto reg_macro_fail;
	}

	if (of_find_property(wsa_priv->dev->of_node, "#cooling-cells", NULL)) {
		ret = of_property_read_u32(pdev->dev.of_node,
					   "qcom,thermal-max-state",
					   &thermal_max_state);
		if (ret) {
			dev_info(&pdev->dev, "%s: could not find %s entry in dt\n",
				__func__, "qcom,thermal-max-state");
			wsa_priv->thermal_max_state =
					LPASS_CDC_WSA_MACRO_THERMAL_MAX_STATE;
		} else {
			wsa_priv->thermal_max_state = thermal_max_state;
		}
		wsa_priv->tcdev = devm_thermal_of_cooling_device_register(
						&pdev->dev,
						wsa_priv->dev->of_node,
						"wsa", wsa_priv,
						&wsa_cooling_ops);
		if (IS_ERR(wsa_priv->tcdev)) {
			dev_err(&pdev->dev,
				"%s: failed to register wsa macro as cooling device\n",
				__func__);
			wsa_priv->tcdev = NULL;
		}
	}

	pm_runtime_set_autosuspend_delay(&pdev->dev, AUTO_SUSPEND_DELAY);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_set_suspended(&pdev->dev);
	pm_suspend_ignore_children(&pdev->dev, true);
	pm_runtime_enable(&pdev->dev);
	schedule_work(&wsa_priv->lpass_cdc_wsa_macro_add_child_devices_work);
	return ret;
reg_macro_fail:
	mutex_destroy(&wsa_priv->mclk_lock);
	mutex_destroy(&wsa_priv->swr_clk_lock);
	return ret;
}

static int lpass_cdc_wsa_macro_remove(struct platform_device *pdev)
{
	struct lpass_cdc_wsa_macro_priv *wsa_priv;
	u16 count = 0;

	wsa_priv = dev_get_drvdata(&pdev->dev);

	if (!wsa_priv)
		return -EINVAL;

	if (wsa_priv->tcdev)
		thermal_cooling_device_unregister(wsa_priv->tcdev);

	for (count = 0; count < wsa_priv->child_count &&
		count < LPASS_CDC_WSA_MACRO_CHILD_DEVICES_MAX; count++)
		platform_device_unregister(wsa_priv->pdev_child_devices[count]);

	pm_runtime_disable(&pdev->dev);
	pm_runtime_set_suspended(&pdev->dev);
	lpass_cdc_unregister_macro(&pdev->dev, WSA_MACRO);
	mutex_destroy(&wsa_priv->mclk_lock);
	mutex_destroy(&wsa_priv->swr_clk_lock);
	return 0;
}

static const struct of_device_id lpass_cdc_wsa_macro_dt_match[] = {
	{.compatible = "qcom,lpass-cdc-wsa-macro"},
	{}
};

static const struct dev_pm_ops lpass_cdc_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(
		pm_runtime_force_suspend,
		pm_runtime_force_resume
	)
	SET_RUNTIME_PM_OPS(
		lpass_cdc_runtime_suspend,
		lpass_cdc_runtime_resume,
		NULL
	)
};

static struct platform_driver lpass_cdc_wsa_macro_driver = {
	.driver = {
		.name = "lpass_cdc_wsa_macro",
		.owner = THIS_MODULE,
		.pm = &lpass_cdc_dev_pm_ops,
		.of_match_table = lpass_cdc_wsa_macro_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = lpass_cdc_wsa_macro_probe,
	.remove = lpass_cdc_wsa_macro_remove,
};

module_platform_driver(lpass_cdc_wsa_macro_driver);

MODULE_DESCRIPTION("WSA macro driver");
MODULE_LICENSE("GPL v2");
