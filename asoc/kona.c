// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <linux/soc/qcom/fsa4480-i2c.h>
#include <sound/control.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/info.h>
#include <soc/snd_event.h>
#include <dsp/audio_prm.h>
#include <soc/swr-common.h>
#include "device_event.h"
#include "asoc/msm-cdc-pinctrl.h"
#include "asoc/wcd-mbhc-v2.h"
#include "codecs/wcd938x/wcd938x-mbhc.h"
#include "codecs/wsa881x.h"
#include "codecs/wcd938x/wcd938x.h"
#include "codecs/bolero/bolero-cdc.h"
#include <dt-bindings/sound/audio-codec-port-types.h>
#include "codecs/bolero/wsa-macro.h"
#include "kona-port-config.h"
#include "msm-audio-defs.h"
#include "msm_common.h"

#define DRV_NAME "kona-asoc-snd"
#define __CHIPSET__ "KONA "
#define MSM_DAILINK_NAME(name) (__CHIPSET__#name)

#define WCD9XXX_MBHC_DEF_RLOADS     5
#define WCD9XXX_MBHC_DEF_BUTTONS    8
#define CODEC_EXT_CLK_RATE          9600000
#define DEV_NAME_STR_LEN            32
#define WCD_MBHC_HS_V_MAX           1600

#define WSA8810_NAME_1 "wsa881x.1020170211"
#define WSA8810_NAME_2 "wsa881x.1020170212"

#define WSA8815_NAME_1 "wsa881x.1021170213"
#define WSA8815_NAME_2 "wsa881x.1021170214"
#define WCN_CDC_SLIM_RX_CH_MAX 2
#define WCN_CDC_SLIM_TX_CH_MAX 2

struct msm_asoc_mach_data {
	struct snd_info_entry *codec_root;
	struct msm_common_pdata *common_pdata;
	int lito_v2_enabled;
	struct device_node *dmic01_gpio_p; /* used by pinctrl API */
	struct device_node *dmic23_gpio_p; /* used by pinctrl API */
	struct device_node *dmic45_gpio_p; /* used by pinctrl API */
	struct device_node *us_euro_gpio_p; /* used by pinctrl API */
	struct pinctrl *usbc_en2_gpio_p; /* used by pinctrl API */
	struct device_node *hph_en1_gpio_p; /* used by pinctrl API */
	struct device_node *hph_en0_gpio_p; /* used by pinctrl API */
	struct device_node *fsa_handle;
};

struct msm_wsa881x_dev_info {
	struct device_node *of_node;
	u32 index;
};

struct aux_codec_dev_info {
	struct device_node *of_node;
	u32 index;
};

static bool is_initial_boot;
static bool codec_reg_done;
static struct snd_soc_aux_dev *msm_aux_dev;
static struct snd_soc_codec_conf *msm_codec_conf;
static struct snd_soc_card snd_soc_card_kona_msm;
static int dmic_0_1_gpio_cnt;
static int dmic_2_3_gpio_cnt;
static int dmic_4_5_gpio_cnt;

static void *def_wcd_mbhc_cal(void);

/*
 * Need to report LINEIN
 * if R/L channel impedance is larger than 5K ohm
 */
static struct wcd_mbhc_config wcd_mbhc_cfg = {
	.read_fw_bin = false,
	.calibration = NULL,
	.detect_extn_cable = true,
	.mono_stero_detection = false,
	.swap_gnd_mic = NULL,
	.hs_ext_micbias = true,
	.key_code[0] = KEY_MEDIA,
	.key_code[1] = KEY_VOICECOMMAND,
	.key_code[2] = KEY_VOLUMEUP,
	.key_code[3] = KEY_VOLUMEDOWN,
	.key_code[4] = 0,
	.key_code[5] = 0,
	.key_code[6] = 0,
	.key_code[7] = 0,
	.linein_th = 5000,
	.moisture_en = false,
	.mbhc_micbias = MIC_BIAS_2,
	.anc_micbias = MIC_BIAS_2,
	.enable_anc_mic_detect = false,
	.moisture_duty_cycle_en = true,
};

static bool msm_usbc_swap_gnd_mic(struct snd_soc_component *component, bool active)
{
	struct snd_soc_card *card = component->card;
	struct msm_asoc_mach_data *pdata =
				snd_soc_card_get_drvdata(card);

	if (!pdata->fsa_handle)
		return false;

	return fsa4480_switch_event(pdata->fsa_handle, FSA_MIC_GND_SWAP);
}

static bool msm_swap_gnd_mic(struct snd_soc_component *component, bool active)
{
	int value = 0;
	bool ret = false;
	struct snd_soc_card *card;
	struct msm_asoc_mach_data *pdata;

	if (!component) {
		pr_err("%s component is NULL\n", __func__);
		return false;
	}
	card = component->card;
	pdata = snd_soc_card_get_drvdata(card);

	if (!pdata)
		return false;

	if (wcd_mbhc_cfg.enable_usbc_analog)
		return msm_usbc_swap_gnd_mic(component, active);

	/* if usbc is not defined, swap using us_euro_gpio_p */
	if (pdata->us_euro_gpio_p) {
		value = msm_cdc_pinctrl_get_state(
				pdata->us_euro_gpio_p);
		if (value)
			msm_cdc_pinctrl_select_sleep_state(
					pdata->us_euro_gpio_p);
		else
			msm_cdc_pinctrl_select_active_state(
					pdata->us_euro_gpio_p);
		dev_dbg(component->dev, "%s: swap select switch %d to %d\n",
			__func__, value, !value);
		ret = true;
	}

	return ret;
}

static int msm_dmic_event(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct msm_asoc_mach_data *pdata = NULL;
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	int ret = 0;
	u32 dmic_idx;
	int *dmic_gpio_cnt;
	struct device_node *dmic_gpio;
	char  *wname;

	wname = strpbrk(w->name, "012345");
	if (!wname) {
		dev_err(component->dev, "%s: widget not found\n", __func__);
		return -EINVAL;
	}

	ret = kstrtouint(wname, 10, &dmic_idx);
	if (ret < 0) {
		dev_err(component->dev, "%s: Invalid DMIC line on the codec\n",
			__func__);
		return -EINVAL;
	}

	pdata = snd_soc_card_get_drvdata(component->card);

	switch (dmic_idx) {
	case 0:
	case 1:
		dmic_gpio_cnt = &dmic_0_1_gpio_cnt;
		dmic_gpio = pdata->dmic01_gpio_p;
		break;
	case 2:
	case 3:
		dmic_gpio_cnt = &dmic_2_3_gpio_cnt;
		dmic_gpio = pdata->dmic23_gpio_p;
		break;
	case 4:
	case 5:
		dmic_gpio_cnt = &dmic_4_5_gpio_cnt;
		dmic_gpio = pdata->dmic45_gpio_p;
		break;
	default:
		dev_err(component->dev, "%s: Invalid DMIC Selection\n",
			__func__);
		return -EINVAL;
	}

	dev_dbg(component->dev, "%s: event %d DMIC%d dmic_gpio_cnt %d\n",
			__func__, event, dmic_idx, *dmic_gpio_cnt);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		(*dmic_gpio_cnt)++;
		if (*dmic_gpio_cnt == 1) {
			ret = msm_cdc_pinctrl_select_active_state(
						dmic_gpio);
			if (ret < 0) {
				pr_err("%s: gpio set cannot be activated %sd",
					__func__, "dmic_gpio");
				return ret;
			}
		}

		break;
	case SND_SOC_DAPM_POST_PMD:
		(*dmic_gpio_cnt)--;
		if (*dmic_gpio_cnt == 0) {
			ret = msm_cdc_pinctrl_select_sleep_state(
					dmic_gpio);
			if (ret < 0) {
				pr_err("%s: gpio set cannot be de-activated %sd",
					__func__, "dmic_gpio");
				return ret;
			}
		}
		break;
	default:
		pr_err("%s: invalid DAPM event %d\n", __func__, event);
		return -EINVAL;
	}
	return 0;
}

static const struct snd_soc_dapm_widget msm_int_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("Analog Mic1", NULL),
	SND_SOC_DAPM_MIC("Analog Mic2", NULL),
	SND_SOC_DAPM_MIC("Analog Mic3", NULL),
	SND_SOC_DAPM_MIC("Analog Mic4", NULL),
	SND_SOC_DAPM_MIC("Analog Mic5", NULL),
	SND_SOC_DAPM_MIC("Digital Mic0", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic1", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic2", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic3", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic4", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic5", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic6", NULL),
	SND_SOC_DAPM_MIC("Digital Mic7", NULL),
};


static int msm_int_audrx_init(struct snd_soc_pcm_runtime *rtd)
{
	int ret = 0;
	struct snd_soc_component *component;
	struct snd_soc_dapm_context *dapm;
	struct snd_card *card;
	struct snd_info_entry *entry;
	struct snd_soc_component *aux_comp;
	struct msm_asoc_mach_data *pdata =
				snd_soc_card_get_drvdata(rtd->card);

	component = snd_soc_rtdcom_lookup(rtd, "bolero_codec");
	if (!component) {
		ret = -EINVAL;
		pr_err("%s: could not find component for bolero_codec\n",
			__func__);
		return ret;
	}

	dapm = snd_soc_component_get_dapm(component);

	snd_soc_dapm_new_controls(dapm, msm_int_dapm_widgets,
				ARRAY_SIZE(msm_int_dapm_widgets));

	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic0");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic1");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic2");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic3");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic4");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic5");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic6");
	snd_soc_dapm_ignore_suspend(dapm, "Digital Mic7");

	snd_soc_dapm_ignore_suspend(dapm, "Analog Mic1");
	snd_soc_dapm_ignore_suspend(dapm, "Analog Mic2");
	snd_soc_dapm_ignore_suspend(dapm, "Analog Mic3");
	snd_soc_dapm_ignore_suspend(dapm, "Analog Mic4");
	snd_soc_dapm_ignore_suspend(dapm, "Analog Mic5");

	snd_soc_dapm_ignore_suspend(dapm, "WSA_SPK1 OUT");
	snd_soc_dapm_ignore_suspend(dapm, "WSA_SPK2 OUT");
	snd_soc_dapm_ignore_suspend(dapm, "WSA AIF VI");
	snd_soc_dapm_ignore_suspend(dapm, "VIINPUT_WSA");

	snd_soc_dapm_sync(dapm);

	/*
	 * Send speaker configuration only for WSA8810.
	 * Default configuration is for WSA8815.
	 */
	dev_dbg(component->dev, "%s: Number of aux devices: %d\n",
		__func__, rtd->card->num_aux_devs);
	if (rtd->card->num_aux_devs &&
	    !list_empty(&rtd->card->component_dev_list)) {
		list_for_each_entry(aux_comp,
				&rtd->card->aux_comp_list,
				card_aux_list) {
			if (aux_comp->name != NULL && (
				!strcmp(aux_comp->name, WSA8810_NAME_1) ||
		    		!strcmp(aux_comp->name, WSA8810_NAME_2))) {
				wsa_macro_set_spkr_mode(component,
						WSA_MACRO_SPKR_MODE_1);
				wsa_macro_set_spkr_gain_offset(component,
						WSA_MACRO_GAIN_OFFSET_M1P5_DB);
			}
		}
		if (pdata->lito_v2_enabled) {
			/*
			 * Enable tx data line3 for saipan version v2 amd
			 * write corresponding lpi register.
			 */
			bolero_set_port_map(component, ARRAY_SIZE(sm_port_map_v2),
					sm_port_map_v2);
		} else {
			bolero_set_port_map(component, ARRAY_SIZE(sm_port_map),
					sm_port_map);
		}
	}
	card = rtd->card->snd_card;
	if (!pdata->codec_root) {
		entry = snd_info_create_subdir(card->module, "codecs",
						 card->proc_root);
		if (!entry) {
			pr_debug("%s: Cannot create codecs module entry\n",
				 __func__);
			ret = 0;
			goto err;
		}
		pdata->codec_root = entry;
	}
	bolero_info_create_codec_entry(pdata->codec_root, component);
	bolero_register_wake_irq(component, false);
	codec_reg_done = true;
	msm_common_dai_link_init(rtd);
	return 0;
err:
	return ret;
}

static void *def_wcd_mbhc_cal(void)
{
	void *wcd_mbhc_cal;
	struct wcd_mbhc_btn_detect_cfg *btn_cfg;
	u16 *btn_high;

	wcd_mbhc_cal = kzalloc(WCD_MBHC_CAL_SIZE(WCD_MBHC_DEF_BUTTONS,
				WCD9XXX_MBHC_DEF_RLOADS), GFP_KERNEL);
	if (!wcd_mbhc_cal)
		return NULL;

	WCD_MBHC_CAL_PLUG_TYPE_PTR(wcd_mbhc_cal)->v_hs_max = WCD_MBHC_HS_V_MAX;
	WCD_MBHC_CAL_BTN_DET_PTR(wcd_mbhc_cal)->num_btn = WCD_MBHC_DEF_BUTTONS;
	btn_cfg = WCD_MBHC_CAL_BTN_DET_PTR(wcd_mbhc_cal);
	btn_high = ((void *)&btn_cfg->_v_btn_low) +
		(sizeof(btn_cfg->_v_btn_low[0]) * btn_cfg->num_btn);

	btn_high[0] = 75;
	btn_high[1] = 150;
	btn_high[2] = 237;
	btn_high[3] = 500;
	btn_high[4] = 500;
	btn_high[5] = 500;
	btn_high[6] = 500;
	btn_high[7] = 500;

	return wcd_mbhc_cal;
}

static int msm_wcn_init(struct snd_soc_pcm_runtime *rtd)
{
	unsigned int rx_ch[WCN_CDC_SLIM_RX_CH_MAX] = {157, 158};
	unsigned int tx_ch[WCN_CDC_SLIM_TX_CH_MAX]  = {159, 160};
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret = 0;

	ret =  snd_soc_dai_set_channel_map(codec_dai, ARRAY_SIZE(tx_ch),
			tx_ch, ARRAY_SIZE(rx_ch), rx_ch);
	if (ret)
		return ret;

	msm_common_dai_link_init(rtd);
	return ret;
}

static struct snd_soc_ops msm_common_be_ops = {
	.startup = msm_common_snd_startup,
	.shutdown = msm_common_snd_shutdown,
};

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link msm_common_dai_links[] = {
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_0,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_0,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "wsa_macro_rx1",
		.codec_name = "bolero_codec",
		.init = &msm_int_audrx_init,
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_1,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_1,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "wsa_macro_rx_mix",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_TX_1,
		.stream_name = LPASS_BE_WSA_CDC_DMA_TX_1,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "wsa_macro_echo",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_0,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_0,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "rx_macro_rx1",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_1,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_1,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "rx_macro_rx2",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_2,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_2,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "rx_macro_rx3",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_3,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_3,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "rx_macro_rx4",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_TX_CDC_DMA_TX_3,
		.stream_name = LPASS_BE_TX_CDC_DMA_TX_3,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "tx_macro_tx1",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_TX_CDC_DMA_TX_4,
		.stream_name = LPASS_BE_TX_CDC_DMA_TX_4,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "tx_macro_tx2",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_VA_CDC_DMA_TX_0,
		.stream_name = LPASS_BE_VA_CDC_DMA_TX_0,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "va_macro_tx1",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_VA_CDC_DMA_TX_1,
		.stream_name = LPASS_BE_VA_CDC_DMA_TX_1,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "va_macro_tx2",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_VA_CDC_DMA_TX_2,
		.stream_name = LPASS_BE_VA_CDC_DMA_TX_2,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "va_macro_tx3",
		.codec_name = "bolero_codec",
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_SLIMBUS_7_RX,
		.stream_name = LPASS_BE_SLIMBUS_7_RX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_name = "btfmslim_slave",
		.ops = &msm_common_be_ops,
		.codec_dai_name = "btfm_bt_sco_a2dp_slim_rx",
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.init = &msm_wcn_init,
	},
	{
		.name = LPASS_BE_SLIMBUS_7_TX,
		.stream_name = LPASS_BE_SLIMBUS_7_TX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_name = "btfmslim_slave",
		.ops = &msm_common_be_ops,
		.codec_dai_name = "btfm_bt_sco_slim_tx",
		.ignore_suspend = 1,
	},
	{
		.name = LPASS_BE_DISPLAY_PORT_RX,
		.stream_name = LPASS_BE_DISPLAY_PORT_RX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.codec_name = "msm-ext-disp-audio-codec-rx",
		.codec_dai_name = "msm_dp_audio_codec_rx_dai",
		.dpcm_playback = 1,
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
	},
	{
		.name = LPASS_BE_USB_AUDIO_RX,
		.stream_name = LPASS_BE_USB_AUDIO_RX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dpcm_playback = 1,
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
	},
	{
		.name = LPASS_BE_USB_AUDIO_TX,
		.stream_name = LPASS_BE_USB_AUDIO_TX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dpcm_capture = 1,
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
	},
	{
		.name = LPASS_BE_RT_PROXY_PCM_RX,
		.stream_name = LPASS_BE_RT_PROXY_PCM_RX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
	},
	{
		.name = LPASS_BE_RT_PROXY_PCM_TX,
		.stream_name = LPASS_BE_RT_PROXY_PCM_TX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ignore_suspend = 1,
	},
};

static struct snd_soc_dai_link msm_kona_dai_links[
			ARRAY_SIZE(msm_common_dai_links)];

static int msm_populate_dai_link_component_of_node(
					struct snd_soc_card *card)
{
	int i, index, ret = 0;
	struct device *cdev = card->dev;
	struct snd_soc_dai_link *dai_link = card->dai_link;
	struct device_node *np;

	if (!cdev) {
		dev_err(cdev, "%s: Sound card device memory NULL\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < card->num_links; i++) {
		if (dai_link[i].init == NULL)
			dai_link[i].init = &msm_common_dai_link_init;

		/* populate codec_of_node for snd card dai links */
		if (dai_link[i].codec_name && !dai_link[i].codec_of_node) {
			index = of_property_match_string(cdev->of_node,
						 "asoc-codec-names",
						 dai_link[i].codec_name);
			if (index < 0)
				continue;
			np = of_parse_phandle(cdev->of_node, "asoc-codec",
					      index);
			if (!np) {
				dev_err(cdev, "%s: retrieving phandle for codec %s failed\n",
					__func__, dai_link[i].codec_name);
				ret = -ENODEV;
				goto err;
			}
			dai_link[i].codec_of_node = np;
			dai_link[i].codec_name = NULL;
		}
	}

err:
	return ret;
}

static int msm_audrx_stub_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int msm_snd_stub_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params)
{
	return 0;
}

static struct snd_soc_ops msm_stub_be_ops = {
	.hw_params = msm_snd_stub_hw_params,
};

struct snd_soc_card snd_soc_card_stub_msm = {
	.name		= "kona-stub-snd-card",
};

static struct snd_soc_dai_link msm_stub_be_dai_links[] = {
	/* Backend DAI Links */
	{
		.name = LPASS_BE_PRI_AUXPCM_RX,
		.stream_name = LPASS_BE_PRI_AUXPCM_RX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.codec_name = "msm-stub-codec.1",
		.codec_dai_name = "msm-stub-rx",
		.dpcm_playback = 1,
		.init = &msm_audrx_stub_init,
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
	},
	{
		.name = LPASS_BE_PRI_AUXPCM_TX,
		.stream_name = LPASS_BE_PRI_AUXPCM_RX,
		.cpu_dai_name = "snd-soc-dummy-dai",
		.codec_name = "msm-stub-codec.1",
		.codec_dai_name = "msm-stub-tx",
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
	},
};

static struct snd_soc_dai_link msm_stub_dai_links[
			 ARRAY_SIZE(msm_stub_be_dai_links)];

static const struct of_device_id kona_asoc_machine_of_match[]  = {
	{ .compatible = "qcom,kona-asoc-snd",
	  .data = "codec"},
	{ .compatible = "qcom,kona-asoc-snd-stub",
	  .data = "stub_codec"},
	{},
};

static struct snd_soc_card *populate_snd_card_dailinks(struct device *dev)
{
	struct snd_soc_card *card = NULL;
	struct snd_soc_dai_link *dailink = NULL;
	int total_links = 0;
	const struct of_device_id *match;

	match = of_match_node(kona_asoc_machine_of_match, dev->of_node);
	if (!match) {
		dev_err(dev, "%s: No DT match found for sound card\n",
			__func__);
		return NULL;
	}

	if (!strcmp(match->data, "codec")) {
		card = &snd_soc_card_kona_msm;
		memcpy(msm_kona_dai_links,
		       msm_common_dai_links,
		       sizeof(msm_common_dai_links));
		total_links += ARRAY_SIZE(msm_common_dai_links);

		dailink = msm_kona_dai_links;
	} else if(!strcmp(match->data, "stub_codec")) {
		card = &snd_soc_card_stub_msm;
		memcpy(msm_stub_dai_links,
		       msm_stub_be_dai_links,
		       sizeof(msm_stub_be_dai_links));
		total_links = ARRAY_SIZE(msm_stub_be_dai_links);

		dailink = msm_stub_dai_links;
	}

	if (card) {
		card->dai_link = dailink;
		card->num_links = total_links;
	}

	return card;
}

static int msm_wsa881x_init(struct snd_soc_component *component)
{
	u8 spkleft_ports[WSA881X_MAX_SWR_PORTS] = {0, 1, 2, 3};
	u8 spkright_ports[WSA881X_MAX_SWR_PORTS] = {0, 1, 2, 3};
	u8 spkleft_port_types[WSA881X_MAX_SWR_PORTS] = {SPKR_L, SPKR_L_COMP,
						SPKR_L_BOOST, SPKR_L_VI};
	u8 spkright_port_types[WSA881X_MAX_SWR_PORTS] = {SPKR_R, SPKR_R_COMP,
						SPKR_R_BOOST, SPKR_R_VI};
	unsigned int ch_rate[WSA881X_MAX_SWR_PORTS] = {2400, 600, 300, 1200};
	unsigned int ch_mask[WSA881X_MAX_SWR_PORTS] = {0x1, 0xF, 0x3, 0x3};
	struct msm_asoc_mach_data *pdata;
	struct snd_soc_dapm_context *dapm;
	struct snd_card *card;
	struct snd_info_entry *entry;
	int ret = 0;

	if (!component) {
		pr_err("%s component is NULL\n", __func__);
		return -EINVAL;
	}

	card = component->card->snd_card;
	dapm = snd_soc_component_get_dapm(component);

	if (!strcmp(component->name_prefix, "SpkrLeft")) {
		dev_dbg(component->dev, "%s: setting left ch map to codec %s\n",
			__func__, component->name);
		wsa881x_set_channel_map(component, &spkleft_ports[0],
				WSA881X_MAX_SWR_PORTS, &ch_mask[0],
				&ch_rate[0], &spkleft_port_types[0]);
		if (dapm->component) {
			snd_soc_dapm_ignore_suspend(dapm, "SpkrLeft IN");
			snd_soc_dapm_ignore_suspend(dapm, "SpkrLeft SPKR");
		}
	} else if (!strcmp(component->name_prefix, "SpkrRight")) {
		dev_dbg(component->dev, "%s: setting right ch map to codec %s\n",
			__func__, component->name);
		wsa881x_set_channel_map(component, &spkright_ports[0],
				WSA881X_MAX_SWR_PORTS, &ch_mask[0],
				&ch_rate[0], &spkright_port_types[0]);
		if (dapm->component) {
			snd_soc_dapm_ignore_suspend(dapm, "SpkrRight IN");
			snd_soc_dapm_ignore_suspend(dapm, "SpkrRight SPKR");
		}
	} else {
		dev_err(component->dev, "%s: wrong codec name %s\n", __func__,
			component->name);
		ret = -EINVAL;
		goto err;
	}
	pdata = snd_soc_card_get_drvdata(component->card);
	if (!pdata->codec_root) {
		entry = snd_info_create_subdir(card->module, "codecs",
						 card->proc_root);
		if (!entry) {
			pr_err("%s: Cannot create codecs module entry\n",
				 __func__);
			ret = 0;
			goto err;
		}
		pdata->codec_root = entry;
	}
	wsa881x_codec_info_create_codec_entry(pdata->codec_root,
					      component);
err:
	return ret;
}

static int msm_aux_codec_init(struct snd_soc_component *component)
{
	struct snd_soc_dapm_context *dapm = snd_soc_component_get_dapm(component);
	int ret = 0;
	int codec_variant = -1;
	void *mbhc_calibration;
	struct snd_info_entry *entry;
	struct snd_card *card = component->card->snd_card;
	struct msm_asoc_mach_data *pdata;

	snd_soc_dapm_ignore_suspend(dapm, "EAR");
	snd_soc_dapm_ignore_suspend(dapm, "AUX");
	snd_soc_dapm_ignore_suspend(dapm, "HPHL");
	snd_soc_dapm_ignore_suspend(dapm, "HPHR");
	snd_soc_dapm_ignore_suspend(dapm, "AMIC1");
	snd_soc_dapm_ignore_suspend(dapm, "AMIC2");
	snd_soc_dapm_ignore_suspend(dapm, "AMIC3");
	snd_soc_dapm_ignore_suspend(dapm, "AMIC4");
	snd_soc_dapm_sync(dapm);

	pdata = snd_soc_card_get_drvdata(component->card);
	if (!pdata->codec_root) {
		entry = snd_info_create_subdir(card->module, "codecs",
						 card->proc_root);
		if (!entry) {
			dev_dbg(component->dev, "%s: Cannot create codecs module entry\n",
				 __func__);
			ret = 0;
			goto mbhc_cfg_cal;
		}
		pdata->codec_root = entry;
	}
	wcd938x_info_create_codec_entry(pdata->codec_root, component);

	codec_variant = wcd938x_get_codec_variant(component);
	dev_dbg(component->dev, "%s: variant %d\n", __func__, codec_variant);

mbhc_cfg_cal:
	mbhc_calibration = def_wcd_mbhc_cal();
	if (!mbhc_calibration)
		return -ENOMEM;
	wcd_mbhc_cfg.calibration = mbhc_calibration;
	ret = wcd938x_mbhc_hs_detect(component, &wcd_mbhc_cfg);
	if (ret) {
		dev_err(component->dev, "%s: mbhc hs detect failed, err:%d\n",
			__func__, ret);
		goto err_hs_detect;
	}
	return 0;

err_hs_detect:
	kfree(mbhc_calibration);
	return ret;
}

static int msm_init_aux_dev(struct platform_device *pdev,
				struct snd_soc_card *card)
{
	struct device_node *wsa_of_node;
	struct device_node *aux_codec_of_node;
	u32 wsa_max_devs;
	u32 wsa_dev_cnt;
	u32 codec_max_aux_devs = 0;
	u32 codec_aux_dev_cnt = 0;
	int i;
	struct msm_wsa881x_dev_info *wsa881x_dev_info;
	struct aux_codec_dev_info *aux_cdc_dev_info;
	const char *auxdev_name_prefix[1];
	char *dev_name_str = NULL;
	int found = 0;
	int codecs_found = 0;
	int ret = 0;

	/* Get maximum WSA device count for this platform */
	ret = of_property_read_u32(pdev->dev.of_node,
				   "qcom,wsa-max-devs", &wsa_max_devs);
	if (ret) {
		dev_info(&pdev->dev,
			 "%s: wsa-max-devs property missing in DT %s, ret = %d\n",
			 __func__, pdev->dev.of_node->full_name, ret);
		wsa_max_devs = 0;
		goto codec_aux_dev;
	}
	if (wsa_max_devs == 0) {
		dev_warn(&pdev->dev,
			 "%s: Max WSA devices is 0 for this target?\n",
			 __func__);
		goto codec_aux_dev;
	}

	/* Get count of WSA device phandles for this platform */
	wsa_dev_cnt = of_count_phandle_with_args(pdev->dev.of_node,
						 "qcom,wsa-devs", NULL);
	if (wsa_dev_cnt == -ENOENT) {
		dev_warn(&pdev->dev, "%s: No wsa device defined in DT.\n",
			 __func__);
		goto err;
	} else if (wsa_dev_cnt <= 0) {
		dev_err(&pdev->dev,
			"%s: Error reading wsa device from DT. wsa_dev_cnt = %d\n",
			__func__, wsa_dev_cnt);
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Expect total phandles count to be NOT less than maximum possible
	 * WSA count. However, if it is less, then assign same value to
	 * max count as well.
	 */
	if (wsa_dev_cnt < wsa_max_devs) {
		dev_dbg(&pdev->dev,
			"%s: wsa_max_devs = %d cannot exceed wsa_dev_cnt = %d\n",
			__func__, wsa_max_devs, wsa_dev_cnt);
		wsa_max_devs = wsa_dev_cnt;
	}

	/* Make sure prefix string passed for each WSA device */
	ret = of_property_count_strings(pdev->dev.of_node,
					"qcom,wsa-aux-dev-prefix");
	if (ret != wsa_dev_cnt) {
		dev_err(&pdev->dev,
			"%s: expecting %d wsa prefix. Defined only %d in DT\n",
			__func__, wsa_dev_cnt, ret);
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Alloc mem to store phandle and index info of WSA device, if already
	 * registered with ALSA core
	 */
	wsa881x_dev_info = devm_kcalloc(&pdev->dev, wsa_max_devs,
					sizeof(struct msm_wsa881x_dev_info),
					GFP_KERNEL);
	if (!wsa881x_dev_info) {
		ret = -ENOMEM;
		goto err;
	}

	/*
	 * search and check whether all WSA devices are already
	 * registered with ALSA core or not. If found a node, store
	 * the node and the index in a local array of struct for later
	 * use.
	 */
	for (i = 0; i < wsa_dev_cnt; i++) {
		wsa_of_node = of_parse_phandle(pdev->dev.of_node,
					    "qcom,wsa-devs", i);
		if (unlikely(!wsa_of_node)) {
			/* we should not be here */
			dev_err(&pdev->dev,
				"%s: wsa dev node is not present\n",
				__func__);
			ret = -EINVAL;
			goto err;
		}
		if (soc_find_component(wsa_of_node, NULL)) {
			/* WSA device registered with ALSA core */
			wsa881x_dev_info[found].of_node = wsa_of_node;
			wsa881x_dev_info[found].index = i;
			found++;
			if (found == wsa_max_devs)
				break;
		}
	}

	if (found < wsa_max_devs) {
		dev_dbg(&pdev->dev,
			"%s: failed to find %d components. Found only %d\n",
			__func__, wsa_max_devs, found);
		return -EPROBE_DEFER;
	}
	dev_info(&pdev->dev,
		"%s: found %d wsa881x devices registered with ALSA core\n",
		__func__, found);

codec_aux_dev:
	/* Get maximum aux codec device count for this platform */
	ret = of_property_read_u32(pdev->dev.of_node,
				   "qcom,codec-max-aux-devs",
				   &codec_max_aux_devs);
	if (ret) {
		dev_err(&pdev->dev,
			 "%s: codec-max-aux-devs property missing in DT %s, ret = %d\n",
			 __func__, pdev->dev.of_node->full_name, ret);
		codec_max_aux_devs = 0;
		goto aux_dev_register;
	}
	if (codec_max_aux_devs == 0) {
		dev_dbg(&pdev->dev,
			 "%s: Max aux codec devices is 0 for this target?\n",
			 __func__);
		goto aux_dev_register;
	}

	/* Get count of aux codec device phandles for this platform */
	codec_aux_dev_cnt = of_count_phandle_with_args(
				pdev->dev.of_node,
				"qcom,codec-aux-devs", NULL);
	if (codec_aux_dev_cnt == -ENOENT) {
		dev_warn(&pdev->dev, "%s: No aux codec defined in DT.\n",
			 __func__);
		goto err;
	} else if (codec_aux_dev_cnt <= 0) {
		dev_err(&pdev->dev,
			"%s: Error reading aux codec device from DT, dev_cnt=%d\n",
			__func__, codec_aux_dev_cnt);
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Expect total phandles count to be NOT less than maximum possible
	 * AUX device count. However, if it is less, then assign same value to
	 * max count as well.
	 */
	if (codec_aux_dev_cnt < codec_max_aux_devs) {
		dev_dbg(&pdev->dev,
			"%s: codec_max_aux_devs = %d cannot exceed codec_aux_dev_cnt = %d\n",
			__func__, codec_max_aux_devs,
			codec_aux_dev_cnt);
		codec_max_aux_devs = codec_aux_dev_cnt;
	}

	/*
	 * Alloc mem to store phandle and index info of aux codec
	 * if already registered with ALSA core
	 */
	aux_cdc_dev_info = devm_kcalloc(&pdev->dev, codec_aux_dev_cnt,
				sizeof(struct aux_codec_dev_info),
				GFP_KERNEL);
	if (!aux_cdc_dev_info) {
		ret = -ENOMEM;
		goto err;
	}

	/*
	 * search and check whether all aux codecs are already
	 * registered with ALSA core or not. If found a node, store
	 * the node and the index in a local array of struct for later
	 * use.
	 */
	for (i = 0; i < codec_aux_dev_cnt; i++) {
		aux_codec_of_node = of_parse_phandle(pdev->dev.of_node,
					    "qcom,codec-aux-devs", i);
		if (unlikely(!aux_codec_of_node)) {
			/* we should not be here */
			dev_err(&pdev->dev,
				"%s: aux codec dev node is not present\n",
				__func__);
			ret = -EINVAL;
			goto err;
		}
		if (soc_find_component(aux_codec_of_node, NULL)) {
			/* AUX codec registered with ALSA core */
			aux_cdc_dev_info[codecs_found].of_node =
						aux_codec_of_node;
			aux_cdc_dev_info[codecs_found].index = i;
			codecs_found++;
		}
	}

	if (codecs_found < codec_aux_dev_cnt) {
		dev_dbg(&pdev->dev,
			"%s: failed to find %d components. Found only %d\n",
			__func__, codec_aux_dev_cnt, codecs_found);
		return -EPROBE_DEFER;
	}
	dev_info(&pdev->dev,
		"%s: found %d AUX codecs registered with ALSA core\n",
		__func__, codecs_found);

aux_dev_register:
	card->num_aux_devs = wsa_max_devs + codec_aux_dev_cnt;
	card->num_configs = wsa_max_devs + codec_aux_dev_cnt;

	/* Alloc array of AUX devs struct */
	msm_aux_dev = devm_kcalloc(&pdev->dev, card->num_aux_devs,
				       sizeof(struct snd_soc_aux_dev),
				       GFP_KERNEL);
	if (!msm_aux_dev) {
		ret = -ENOMEM;
		goto err;
	}

	/* Alloc array of codec conf struct */
	msm_codec_conf = devm_kcalloc(&pdev->dev, card->num_configs,
					  sizeof(struct snd_soc_codec_conf),
					  GFP_KERNEL);
	if (!msm_codec_conf) {
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < wsa_max_devs; i++) {
		dev_name_str = devm_kzalloc(&pdev->dev, DEV_NAME_STR_LEN,
					    GFP_KERNEL);
		if (!dev_name_str) {
			ret = -ENOMEM;
			goto err;
		}

		ret = of_property_read_string_index(pdev->dev.of_node,
						    "qcom,wsa-aux-dev-prefix",
						    wsa881x_dev_info[i].index,
						    auxdev_name_prefix);
		if (ret) {
			dev_err(&pdev->dev,
				"%s: failed to read wsa aux dev prefix, ret = %d\n",
				__func__, ret);
			ret = -EINVAL;
			goto err;
		}

		snprintf(dev_name_str, strlen("wsa881x.%d"), "wsa881x.%d", i);
		msm_aux_dev[i].name = dev_name_str;
		msm_aux_dev[i].codec_name = NULL;
		msm_aux_dev[i].codec_of_node =
					wsa881x_dev_info[i].of_node;
		msm_aux_dev[i].init = msm_wsa881x_init;
		msm_codec_conf[i].dev_name = NULL;
		msm_codec_conf[i].name_prefix = auxdev_name_prefix[0];
		msm_codec_conf[i].of_node =
				wsa881x_dev_info[i].of_node;
	}

	for (i = 0; i < codec_aux_dev_cnt; i++) {
		msm_aux_dev[wsa_max_devs + i].name = NULL;
		msm_aux_dev[wsa_max_devs + i].codec_name = NULL;
		msm_aux_dev[wsa_max_devs + i].codec_of_node =
					aux_cdc_dev_info[i].of_node;
		msm_aux_dev[wsa_max_devs + i].init =  msm_aux_codec_init;
		msm_codec_conf[wsa_max_devs + i].dev_name = NULL;
		msm_codec_conf[wsa_max_devs + i].name_prefix =
						NULL;
		msm_codec_conf[wsa_max_devs + i].of_node =
				aux_cdc_dev_info[i].of_node;
	}

	card->codec_conf = msm_codec_conf;
	card->aux_dev = msm_aux_dev;
err:
	return ret;
}

static int kona_ssr_enable(struct device *dev, void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	int ret = 0;

	if (!card) {
		dev_err(dev, "%s: card is NULL\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	if (!strcmp(card->name, "kona-stub-snd-card")) {
		/* TODO */
		dev_dbg(dev, "%s: TODO \n", __func__);
	}

	snd_soc_card_change_online_state(card, 1);
	dev_dbg(dev, "%s: setting snd_card to ONLINE\n", __func__);

err:
	return ret;
}

static void kona_ssr_disable(struct device *dev, void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	if (!card) {
		dev_err(dev, "%s: card is NULL\n", __func__);
		return;
	}

	dev_dbg(dev, "%s: setting snd_card to OFFLINE\n", __func__);
	snd_soc_card_change_online_state(card, 0);

	if (!strcmp(card->name, "kona-stub-snd-card")) {
		/* TODO */
		dev_dbg(dev, "%s: TODO \n", __func__);
	}
}

static const struct snd_event_ops kona_ssr_ops = {
	.enable = kona_ssr_enable,
	.disable = kona_ssr_disable,
};

static int msm_audio_ssr_compare(struct device *dev, void *data)
{
	struct device_node *node = data;

	dev_dbg(dev, "%s: dev->of_node = 0x%p, node = 0x%p\n",
		__func__, dev->of_node, node);
	return (dev->of_node && dev->of_node == node);
}

static int msm_audio_ssr_register(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct snd_event_clients *ssr_clients = NULL;
	struct device_node *node = NULL;
	int ret = 0;
	int i = 0;

	for (i = 0; ; i++) {
		node = of_parse_phandle(np, "qcom,msm_audio_ssr_devs", i);
		if (!node)
			break;
		snd_event_mstr_add_client(&ssr_clients,
					msm_audio_ssr_compare, node);
	}

	ret = snd_event_master_register(dev, &kona_ssr_ops,
					ssr_clients, NULL);
	if (!ret)
		snd_event_notify(dev, SND_EVENT_UP);

	return ret;
}

struct msm_common_pdata *msm_common_get_pdata(struct snd_soc_card *card)
{
	struct msm_asoc_mach_data *pdata = snd_soc_card_get_drvdata(card);

	if (!pdata)
		return NULL;

	return pdata->common_pdata;
}

void msm_common_set_pdata(struct snd_soc_card *card,
			  struct msm_common_pdata *common_pdata)
{
	struct msm_asoc_mach_data *pdata = snd_soc_card_get_drvdata(card);

	if (!pdata)
		return;

	pdata->common_pdata = common_pdata;
}

static int msm_asoc_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = NULL;
	struct msm_asoc_mach_data *pdata = NULL;
	const char *mbhc_audio_jack_type = NULL;
	int ret = 0;

	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "%s: No platform supplied from device tree\n", __func__);
		return -EINVAL;
	}

	pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct msm_asoc_mach_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	of_property_read_u32(pdev->dev.of_node,
				"qcom,lito-is-v2-enabled",
				&pdata->lito_v2_enabled);

	card = populate_snd_card_dailinks(&pdev->dev);
	if (!card) {
		dev_err(&pdev->dev, "%s: Card uninitialized\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, pdata);

	ret = snd_soc_of_parse_card_name(card, "qcom,model");
	if (ret) {
		dev_err(&pdev->dev, "%s: parse card name failed, err:%d\n",
			__func__, ret);
		goto err;
	}

	ret = snd_soc_of_parse_audio_routing(card, "qcom,audio-routing");
	if (ret) {
		dev_err(&pdev->dev, "%s: parse audio routing failed, err:%d\n",
			__func__, ret);
		goto err;
	}

	ret = msm_populate_dai_link_component_of_node(card);
	if (ret) {
		ret = -EPROBE_DEFER;
		goto err;
	}

	ret = msm_init_aux_dev(pdev, card);
	if (ret)
		goto err;

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret == -EPROBE_DEFER) {
		if (codec_reg_done)
			ret = -EINVAL;
		goto err;
	} else if (ret) {
		dev_err(&pdev->dev, "%s: snd_soc_register_card failed (%d)\n",
			__func__, ret);
		goto err;
	}
	dev_info(&pdev->dev, "%s: Sound card %s registered\n",
		 __func__, card->name);

	msm_common_snd_init(pdev, card);

	pdata->hph_en1_gpio_p = of_parse_phandle(pdev->dev.of_node,
						"qcom,hph-en1-gpio", 0);
	if (!pdata->hph_en1_gpio_p) {
		dev_dbg(&pdev->dev, "%s: property %s not detected in node %s\n",
			__func__, "qcom,hph-en1-gpio",
			pdev->dev.of_node->full_name);
	}

	pdata->hph_en0_gpio_p = of_parse_phandle(pdev->dev.of_node,
						"qcom,hph-en0-gpio", 0);
	if (!pdata->hph_en0_gpio_p) {
		dev_dbg(&pdev->dev, "%s: property %s not detected in node %s\n",
			__func__, "qcom,hph-en0-gpio",
			pdev->dev.of_node->full_name);
	}

	ret = of_property_read_string(pdev->dev.of_node,
		"qcom,mbhc-audio-jack-type", &mbhc_audio_jack_type);
	if (ret) {
		dev_dbg(&pdev->dev, "%s: Looking up %s property in node %s failed\n",
			__func__, "qcom,mbhc-audio-jack-type",
			pdev->dev.of_node->full_name);
		dev_dbg(&pdev->dev, "Jack type properties set to default\n");
	} else {
		if (!strcmp(mbhc_audio_jack_type, "4-pole-jack")) {
			wcd_mbhc_cfg.enable_anc_mic_detect = false;
			dev_dbg(&pdev->dev, "This hardware has 4 pole jack");
		} else if (!strcmp(mbhc_audio_jack_type, "5-pole-jack")) {
			wcd_mbhc_cfg.enable_anc_mic_detect = true;
			dev_dbg(&pdev->dev, "This hardware has 5 pole jack");
		} else if (!strcmp(mbhc_audio_jack_type, "6-pole-jack")) {
			wcd_mbhc_cfg.enable_anc_mic_detect = true;
			dev_dbg(&pdev->dev, "This hardware has 6 pole jack");
		} else {
			wcd_mbhc_cfg.enable_anc_mic_detect = false;
			dev_dbg(&pdev->dev, "Unknown value, set to default\n");
		}
	}
	/*
	 * Parse US-Euro gpio info from DT. Report no error if us-euro
	 * entry is not found in DT file as some targets do not support
	 * US-Euro detection
	 */
	pdata->us_euro_gpio_p = of_parse_phandle(pdev->dev.of_node,
					"qcom,us-euro-gpios", 0);
	if (!pdata->us_euro_gpio_p) {
		dev_dbg(&pdev->dev, "property %s not detected in node %s",
			"qcom,us-euro-gpios", pdev->dev.of_node->full_name);
	} else {
		dev_dbg(&pdev->dev, "%s detected\n",
			"qcom,us-euro-gpios");
		wcd_mbhc_cfg.swap_gnd_mic = msm_swap_gnd_mic;
	}

	if (wcd_mbhc_cfg.enable_usbc_analog)
		wcd_mbhc_cfg.swap_gnd_mic = msm_usbc_swap_gnd_mic;

	pdata->fsa_handle = of_parse_phandle(pdev->dev.of_node,
					"fsa4480-i2c-handle", 0);
	if (!pdata->fsa_handle)
		dev_dbg(&pdev->dev, "property %s not detected in node %s\n",
			"fsa4480-i2c-handle", pdev->dev.of_node->full_name);

	pdata->dmic01_gpio_p = of_parse_phandle(pdev->dev.of_node,
					      "qcom,cdc-dmic01-gpios",
					       0);
	pdata->dmic23_gpio_p = of_parse_phandle(pdev->dev.of_node,
					      "qcom,cdc-dmic23-gpios",
					       0);
	pdata->dmic45_gpio_p = of_parse_phandle(pdev->dev.of_node,
					      "qcom,cdc-dmic45-gpios",
					       0);
	if (pdata->dmic01_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic01_gpio_p, false);
	if (pdata->dmic23_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic23_gpio_p, false);
	if (pdata->dmic45_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic45_gpio_p, false);

	ret = msm_audio_ssr_register(&pdev->dev);
	if (ret)
		pr_err("%s: Registration with SND event FWK failed ret = %d\n",
			__func__, ret);

	is_initial_boot = true;

	return 0;
err:
	devm_kfree(&pdev->dev, pdata);
	return ret;
}

static int msm_asoc_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct msm_asoc_mach_data *pdata = NULL;
	struct msm_common_pdata *common_pdata = NULL;

	if (card)
		pdata = snd_soc_card_get_drvdata(card);

	if (pdata)
		common_pdata = pdata->common_pdata;

	msm_common_snd_deinit(common_pdata);

	snd_event_master_deregister(&pdev->dev);
	snd_soc_unregister_card(card);

	return 0;
}

static struct platform_driver kona_asoc_machine_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = kona_asoc_machine_of_match,
		.suppress_bind_attrs = true,
	},
	.probe = msm_asoc_machine_probe,
	.remove = msm_asoc_machine_remove,
};
module_platform_driver(kona_asoc_machine_driver);

MODULE_DESCRIPTION("ALSA SoC msm");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, kona_asoc_machine_of_match);
