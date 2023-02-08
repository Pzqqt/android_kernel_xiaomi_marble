// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/pm_qos.h>
#include <linux/nvmem-consumer.h>
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
#include <soc/soundwire.h>
#include "device_event.h"
#include "asoc/msm-cdc-pinctrl.h"
#include "asoc/wcd-mbhc-v2.h"
#include "codecs/wcd938x/wcd938x-mbhc.h"
#include "codecs/wcd937x/wcd937x-mbhc.h"
#include "codecs/wsa883x/wsa883x.h"
#include "codecs/wsa881x.h"
#include "codecs/wcd938x/wcd938x.h"
#include "codecs/wcd937x/wcd937x.h"
#include "codecs/lpass-cdc/lpass-cdc.h"
#include <bindings/audio-codec-port-types.h>
#include "codecs/lpass-cdc/lpass-cdc-wsa-macro.h"
#include "waipio-port-config.h"
#include "msm-audio-defs.h"
#include "msm_common.h"
#include "msm_dailink.h"

#define DRV_NAME "waipio-asoc-snd"
#define __CHIPSET__ "WAIPIO "
#define MSM_DAILINK_NAME(name) (__CHIPSET__#name)

#define WCD9XXX_MBHC_DEF_RLOADS     5
#define WCD9XXX_MBHC_DEF_BUTTONS    8
#define CODEC_EXT_CLK_RATE          9600000
#define DEV_NAME_STR_LEN            32
#define WCD_MBHC_HS_V_MAX           1600

#define WCN_CDC_SLIM_RX_CH_MAX 2
#define WCN_CDC_SLIM_TX_CH_MAX 2
#define WCN_CDC_SLIM_TX_CH_MAX_FM 3

/* Number of WSAs */
#define MONO_SPEAKER    1
#define STEREO_SPEAKER  2
#define QUAD_SPEAKER    4

struct msm_asoc_mach_data {
	struct snd_info_entry *codec_root;
	struct msm_common_pdata *common_pdata;
	int usbc_en2_gpio; /* used by gpio driver API */
	struct device_node *dmic01_gpio_p; /* used by pinctrl API */
	struct device_node *dmic23_gpio_p; /* used by pinctrl API */
	struct device_node *dmic45_gpio_p; /* used by pinctrl API */
	struct device_node *dmic67_gpio_p; /* used by pinctrl API */
	struct pinctrl *usbc_en2_gpio_p; /* used by pinctrl API */
	bool is_afe_config_done;
	struct device_node *fsa_handle;
	struct clk *lpass_audio_hw_vote;
	int core_audio_vote_count;
	u32 wsa_max_devs;
	int wsa881x_support;
	int wcd_disabled;
	int (*get_dev_num)(struct snd_soc_component *);
	int backend_used;
	struct prm_earpa_hw_intf_config upd_config;
};

static bool is_initial_boot;
static bool codec_reg_done;
static struct snd_soc_card snd_soc_card_waipio_msm;
static int dmic_0_1_gpio_cnt;
static int dmic_2_3_gpio_cnt;
static int dmic_4_5_gpio_cnt;
static int dmic_6_7_gpio_cnt;

static void *def_wcd_mbhc_cal(void);

static int msm_rx_tx_codec_init(struct snd_soc_pcm_runtime*);
static int msm_int_wsa_init(struct snd_soc_pcm_runtime*);

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

static void msm_parse_upd_configuration(struct platform_device *pdev,
					struct msm_asoc_mach_data *pdata)
{
	int ret = 0;
	u32 dt_values[2];

	if (!pdev || !pdata)
		return;

	ret = of_property_read_string(pdev->dev.of_node,
		"qcom,upd_backends_used", &pdata->upd_config.backend_used);
	if (ret) {
		pr_debug("%s:could not find %s entry in dt\n",
			__func__, "qcom,upd_backends_used");
		return;
	}

	ret = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,upd_lpass_reg_addr", dt_values, MAX_EARPA_REG);
	if (ret) {
		pr_debug("%s: could not find %s entry in dt\n",
				__func__, "qcom,upd_lpass_reg_addr");
		return;
	} else {
		pdata->upd_config.ear_pa_hw_reg_cfg.lpass_cdc_rx0_rx_path_ctl_phy_addr =
									dt_values[0];
		pdata->upd_config.ear_pa_hw_reg_cfg.lpass_wr_fifo_reg_phy_addr =
								dt_values[1];
	}

	ret = of_property_read_u32(pdev->dev.of_node,
			"qcom,upd_ear_pa_reg_addr", &pdata->upd_config.ear_pa_pkd_reg_addr);
	if (ret) {
		pr_debug("%s: could not find %s entry in dt\n",
			__func__, "qcom,upd_ear_pa_reg_addr");
	}
}

static void msm_set_upd_config(struct snd_soc_pcm_runtime *rtd)
{
	int val1 = 0, val2 = 0, ret = 0;
	u8  dev_num = 0;
	struct snd_soc_component *component = NULL;
	struct snd_soc_card *card = NULL;
	struct msm_asoc_mach_data *pdata = NULL;

	if (!rtd) {
		pr_err("%s: rtd is NULL\n", __func__);
		return;
	}

	card = rtd->card;
	pdata = snd_soc_card_get_drvdata(rtd->card);
	if (!pdata) {
		pr_err("%s: pdata is NULL\n", __func__);
		return;
	}

	if (!pdata->upd_config.ear_pa_hw_reg_cfg.lpass_cdc_rx0_rx_path_ctl_phy_addr ||
		!pdata->upd_config.ear_pa_hw_reg_cfg.lpass_wr_fifo_reg_phy_addr ||
                !pdata->upd_config.ear_pa_pkd_reg_addr) {
		pr_err("%s: upd static configuration is not set\n", __func__);
		return;
	}

	if (!strcmp(pdata->upd_config.backend_used, "wsa")) {
		if (pdata->wsa_max_devs > 0) {
			component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.1");
			if (!component) {
				pr_err("%s: %s component is NULL\n", __func__,
					"wsa-codec.1");
				return;
			}
		} else {
			pr_info("%s wsa_max_devs are NULL\n", __func__);
			return;
		}
	} else {
		component = snd_soc_rtdcom_lookup(rtd, WCD938X_DRV_NAME);
		if (!component) {
			component = snd_soc_rtdcom_lookup(rtd, WCD937X_DRV_NAME);
			if (!component) {
				pr_err("%s component is NULL\n", __func__);
				return;
			}
		}
	}

	if (!strcmp(pdata->upd_config.backend_used, "wsa")) {
		if (strstr(card->name, "wsa881x"))
			pdata->get_dev_num = wsa881x_codec_get_dev_num;
		else
			pdata->get_dev_num = wsa883x_codec_get_dev_num;
	} else {
		if (!strncmp(component->driver->name, WCD937X_DRV_NAME,
				strlen(WCD937X_DRV_NAME))){
			pdata->get_dev_num = wcd937x_codec_get_dev_num;
		} else if (!strncmp(component->driver->name, WCD938X_DRV_NAME,
				strlen(WCD938X_DRV_NAME))){
			pdata->get_dev_num = wcd938x_codec_get_dev_num;
		}
	}

	if (!pdata->get_dev_num) {
		pr_err("%s: get_dev_num is NULL\n", __func__);
		return;
	}

	dev_num = pdata->get_dev_num(component);
	if (dev_num < 0 || dev_num > 6) {
		pr_err("%s: invalid slave dev num : %d\n", __func__,
							dev_num);
		return;
	}

	pdata->upd_config.ear_pa_pkd_cfg.ear_pa_enable_pkd_reg_addr =
				pdata->upd_config.ear_pa_pkd_reg_addr & 0xFFFF;
	pdata->upd_config.ear_pa_pkd_cfg.ear_pa_disable_pkd_reg_addr =
				pdata->upd_config.ear_pa_pkd_reg_addr & 0xFFFF;

	val1 = val2 = 0;

	/* bits 16:19 carry command id */
	val1 |= 1 << 16;

	/* bits 20:23 carry swr device number */
	val1 |= dev_num << 20;

	/*
	 * bits 24:31 carry 8 bit data to disable or enable ear pa
	 * for wcd 7bit is global enable bit - 1 -enable. 0 - disable
	 * for wsa 0bit is global enable bit - 1 -enable, 0 - disable
	*/
	val2 = val1;

	if (!strcmp(pdata->upd_config.backend_used, "wsa"))
		val1 |= 1 << 24;
	else
		val1 |= 1 << 31;

	pdata->upd_config.ear_pa_pkd_cfg.ear_pa_enable_pkd_reg_addr |= val1;
	pdata->upd_config.ear_pa_pkd_cfg.ear_pa_disable_pkd_reg_addr |= val2;

	ret = audio_prm_set_cdc_earpa_duty_cycling_req(&pdata->upd_config, 1);
	if (ret < 0)
		pr_err("%s: upd cdc duty cycling registration failed\n", __func__);
}

static struct snd_soc_ops msm_common_be_ops = {
	.hw_params = msm_common_snd_hw_params,
	.startup = msm_common_snd_startup,
	.shutdown = msm_common_snd_shutdown,
};

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

	wname = strpbrk(w->name, "01234567");
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
	case 6:
	case 7:
		dmic_gpio_cnt = &dmic_6_7_gpio_cnt;
		dmic_gpio = pdata->dmic67_gpio_p;
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
	SND_SOC_DAPM_MIC("Digital Mic6", msm_dmic_event),
	SND_SOC_DAPM_MIC("Digital Mic7", msm_dmic_event),
};

static int msm_wcn_init(struct snd_soc_pcm_runtime *rtd)
{
	unsigned int rx_ch[WCN_CDC_SLIM_RX_CH_MAX] = {157, 158};
	unsigned int tx_ch[WCN_CDC_SLIM_TX_CH_MAX]  = {159, 160};
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int ret = 0;

	ret = snd_soc_dai_set_channel_map(codec_dai, ARRAY_SIZE(tx_ch),
					   tx_ch, ARRAY_SIZE(rx_ch), rx_ch);
	if (ret)
		return ret;

	msm_common_dai_link_init(rtd);
	return ret;
}

static int msm_wcn_init_btfm(struct snd_soc_pcm_runtime *rtd)
{
	unsigned int rx_ch[WCN_CDC_SLIM_RX_CH_MAX] = {157, 158};
	unsigned int tx_ch[WCN_CDC_SLIM_TX_CH_MAX_FM]  = {159, 160, 161};
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int ret = 0;

	ret = snd_soc_dai_set_channel_map(codec_dai, ARRAY_SIZE(tx_ch),
					   tx_ch, ARRAY_SIZE(rx_ch), rx_ch);
	if (ret)
		return ret;

	msm_common_dai_link_init(rtd);
	return ret;
}

static struct snd_info_entry *msm_snd_info_create_subdir(struct module *mod,
				const char *name,
				struct snd_info_entry *parent)
{
	struct snd_info_entry *entry;

	entry = snd_info_create_module_entry(mod, name, parent);
	if (!entry)
		return NULL;
	entry->mode = S_IFDIR | 0555;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return NULL;
	}
	return entry;
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

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link msm_common_be_dai_links[] = {
	/* Proxy Tx BACK END DAI Link */
	{
		.name = LPASS_BE_RT_PROXY_PCM_TX,
		.stream_name = LPASS_BE_RT_PROXY_PCM_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(proxy_tx),
	},
	/* Proxy Rx BACK END DAI Link */
	{
		.name = LPASS_BE_RT_PROXY_PCM_RX,
		.stream_name = LPASS_BE_RT_PROXY_PCM_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(proxy_rx),
	},
	{
		.name = LPASS_BE_USB_AUDIO_RX,
		.stream_name = LPASS_BE_USB_AUDIO_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(usb_audio_rx),
	},
	{
		.name = LPASS_BE_USB_AUDIO_TX,
		.stream_name = LPASS_BE_USB_AUDIO_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(usb_audio_tx),
	},
};

static struct snd_soc_dai_link msm_wcn_be_dai_links[] = {
	{
		.name = LPASS_BE_SLIMBUS_7_RX,
		.stream_name = LPASS_BE_SLIMBUS_7_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.init = &msm_wcn_init,
		.ops = &msm_common_be_ops,
		/* dai link has playback support */
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(slimbus_7_rx),
	},
	{
		.name = LPASS_BE_SLIMBUS_7_TX,
		.stream_name = LPASS_BE_SLIMBUS_7_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(slimbus_7_tx),
	},
};

static struct snd_soc_dai_link msm_wcn_btfm_be_dai_links[] = {
	{
		.name = LPASS_BE_SLIMBUS_7_RX,
		.stream_name = LPASS_BE_SLIMBUS_7_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.init = &msm_wcn_init_btfm,
		.ops = &msm_common_be_ops,
		/* dai link has playback support */
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(slimbus_7_rx),
	},
	{
		.name = LPASS_BE_SLIMBUS_7_TX,
		.stream_name = LPASS_BE_SLIMBUS_7_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(slimbus_7_tx),
	},
	{
		.name = LPASS_BE_SLIMBUS_8_TX,
		.stream_name = LPASS_BE_SLIMBUS_8_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(slimbus_8_tx),
	},
};

static struct snd_soc_dai_link ext_disp_be_dai_link[] = {
	/* DISP PORT BACK END DAI Link */
	{
		.name = LPASS_BE_DISPLAY_PORT_RX,
		.stream_name = LPASS_BE_DISPLAY_PORT_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(display_port),
	},
};

static struct snd_soc_dai_link msm_wsa_cdc_dma_be_dai_links[] = {
	/* WSA CDC DMA Backend DAI Links */
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_0,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_dma_rx0),
		.init = &msm_int_wsa_init,
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_1,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_1,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_dma_rx1),
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_TX_1,
		.stream_name = LPASS_BE_WSA_CDC_DMA_TX_1,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_dma_tx1),
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_TX_0,
		.stream_name = LPASS_BE_WSA_CDC_DMA_TX_0,
		.capture_only = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		/* .no_host_mode = SND_SOC_DAI_LINK_NO_HOST, */
		SND_SOC_DAILINK_REG(vi_feedback),
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_0_VIRT,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_0_VIRT,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_dma_rx0),
	},
};

static struct snd_soc_dai_link msm_wsa2_cdc_dma_be_dai_links[] = {
	/* WSA2 CDC DMA Backend DAI Links */
	{
		.name = LPASS_BE_WSA2_CDC_DMA_RX_0,
		.stream_name = LPASS_BE_WSA2_CDC_DMA_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa2_dma_rx0),
	},
	{
		.name = LPASS_BE_WSA2_CDC_DMA_RX_1,
		.stream_name = LPASS_BE_WSA2_CDC_DMA_RX_1,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa2_dma_rx1),
	},
	{
		.name = LPASS_BE_WSA2_CDC_DMA_TX_1,
		.stream_name = LPASS_BE_WSA2_CDC_DMA_TX_1,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa2_dma_tx1),
	},
	{
		.name = LPASS_BE_WSA2_CDC_DMA_TX_0,
		.stream_name = LPASS_BE_WSA2_CDC_DMA_TX_0,
		.capture_only = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		/* .no_host_mode = SND_SOC_DAI_LINK_NO_HOST, */
		SND_SOC_DAILINK_REG(wsa2_vi_feedback),
	},
};

static struct snd_soc_dai_link msm_wsa_wsa2_cdc_dma_be_dai_links[] = {
	/* WSA CDC DMA Backend DAI Links */
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_0,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_wsa2_dma_rx0),
		.init = &msm_int_wsa_init,
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_RX_1,
		.stream_name = LPASS_BE_WSA_CDC_DMA_RX_1,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_wsa2_dma_rx1),
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_TX_1,
		.stream_name = LPASS_BE_WSA_CDC_DMA_TX_1,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(wsa_wsa2_dma_tx1),
	},
	{
		.name = LPASS_BE_WSA_CDC_DMA_TX_0,
		.stream_name = LPASS_BE_WSA_CDC_DMA_TX_0,
		.capture_only = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		/* .no_host_mode = SND_SOC_DAI_LINK_NO_HOST, */
		SND_SOC_DAILINK_REG(wsa_wsa2_vi_feedback),
	},
};

static struct snd_soc_dai_link msm_rx_tx_cdc_dma_be_dai_links[] = {
	/* RX CDC DMA Backend DAI Links */
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_0,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(rx_dma_rx0),
		.init = &msm_rx_tx_codec_init,
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_1,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_1,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(rx_dma_rx1),
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_2,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_2,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(rx_dma_rx2),
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_3,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_3,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(rx_dma_rx3),
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_5,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_5,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(rx_dma_rx5),
	},
	{
		.name = LPASS_BE_RX_CDC_DMA_RX_6,
		.stream_name = LPASS_BE_RX_CDC_DMA_RX_6,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(rx_dma_rx6),
	},
	/* TX CDC DMA Backend DAI Links */
	{
		.name = LPASS_BE_TX_CDC_DMA_TX_3,
		.stream_name = LPASS_BE_TX_CDC_DMA_TX_3,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(tx_dma_tx3),
	},
	{
		.name = LPASS_BE_TX_CDC_DMA_TX_4,
		.stream_name = LPASS_BE_TX_CDC_DMA_TX_4,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(tx_dma_tx4),
	},
};

static struct snd_soc_dai_link msm_va_cdc_dma_be_dai_links[] = {
	{
		.name = LPASS_BE_VA_CDC_DMA_TX_0,
		.stream_name = LPASS_BE_VA_CDC_DMA_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(va_dma_tx0),
	},
	{
		.name = LPASS_BE_VA_CDC_DMA_TX_1,
		.stream_name = LPASS_BE_VA_CDC_DMA_TX_1,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(va_dma_tx1),
	},
	{
		.name = LPASS_BE_VA_CDC_DMA_TX_2,
		.stream_name = LPASS_BE_VA_CDC_DMA_TX_2,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		.ops = &msm_common_be_ops,
		SND_SOC_DAILINK_REG(va_dma_tx2),
	},
};

/*
 * I2S interface pinctrl mapping
 * ------------------------------------
 * Primary	- pri_mi2s
 * Secondary	- lpi_i2s3
 * Tertiary	- tert_mi2s
 * Quaternary	- quat_mi2s (lpi_i2s0)
 * Quinary	- lpi_i2s1
 * Senary	- lpi_i2s2
 * ------------------------------------
 */
static struct snd_soc_dai_link msm_mi2s_dai_links[] = {
	{
		.name = LPASS_BE_PRI_MI2S_RX,
		.stream_name = LPASS_BE_PRI_MI2S_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(pri_mi2s_rx),
	},
	{
		.name = LPASS_BE_PRI_MI2S_TX,
		.stream_name = LPASS_BE_PRI_MI2S_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(pri_mi2s_tx),
	},
	{
		.name = LPASS_BE_SEC_MI2S_RX,
		.stream_name = LPASS_BE_SEC_MI2S_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(sec_mi2s_rx),
	},
	{
		.name = LPASS_BE_SEC_MI2S_TX,
		.stream_name = LPASS_BE_SEC_MI2S_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(sec_mi2s_tx),
	},
	{
		.name = LPASS_BE_TERT_MI2S_RX,
		.stream_name = LPASS_BE_TERT_MI2S_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(tert_mi2s_rx),
	},
	{
		.name = LPASS_BE_TERT_MI2S_TX,
		.stream_name = LPASS_BE_TERT_MI2S_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(tert_mi2s_tx),
	},
	{
		.name = LPASS_BE_QUAT_MI2S_RX,
		.stream_name = LPASS_BE_QUAT_MI2S_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(quat_mi2s_rx),
	},
	{
		.name = LPASS_BE_QUAT_MI2S_TX,
		.stream_name = LPASS_BE_QUAT_MI2S_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(quat_mi2s_tx),
	},
	{
		.name = LPASS_BE_QUIN_MI2S_RX,
		.stream_name = LPASS_BE_QUIN_MI2S_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(quin_mi2s_rx),
	},
	{
		.name = LPASS_BE_QUIN_MI2S_TX,
		.stream_name = LPASS_BE_QUIN_MI2S_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(quin_mi2s_tx),
	},
	{
		.name = LPASS_BE_SEN_MI2S_RX,
		.stream_name = LPASS_BE_SEN_MI2S_RX,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(sen_mi2s_rx),
	},
	{
		.name = LPASS_BE_SEN_MI2S_TX,
		.stream_name = LPASS_BE_SEN_MI2S_TX,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(sen_mi2s_tx),
	},
};

static struct snd_soc_dai_link msm_tdm_dai_links[] = {
	{
		.name = LPASS_BE_PRI_TDM_RX_0,
		.stream_name = LPASS_BE_PRI_TDM_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(pri_tdm_rx_0),
	},
	{
		.name = LPASS_BE_PRI_TDM_TX_0,
		.stream_name = LPASS_BE_PRI_TDM_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(pri_tdm_tx_0),
	},
	{
		.name = LPASS_BE_SEC_TDM_RX_0,
		.stream_name = LPASS_BE_SEC_TDM_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(sec_tdm_rx_0),
	},
	{
		.name = LPASS_BE_SEC_TDM_TX_0,
		.stream_name = LPASS_BE_SEC_TDM_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(sec_tdm_tx_0),
	},
	{
		.name = LPASS_BE_TERT_TDM_RX_0,
		.stream_name = LPASS_BE_TERT_TDM_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(tert_tdm_rx_0),
	},
	{
		.name = LPASS_BE_TERT_TDM_TX_0,
		.stream_name = LPASS_BE_TERT_TDM_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(tert_tdm_tx_0),
	},
	{
		.name = LPASS_BE_QUAT_TDM_RX_0,
		.stream_name = LPASS_BE_QUAT_TDM_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(quat_tdm_rx_0),
	},
	{
		.name = LPASS_BE_QUAT_TDM_TX_0,
		.stream_name = LPASS_BE_QUAT_TDM_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(quat_tdm_tx_0),
	},
	{
		.name = LPASS_BE_QUIN_TDM_RX_0,
		.stream_name = LPASS_BE_QUIN_TDM_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(quin_tdm_rx_0),
	},
	{
		.name = LPASS_BE_QUIN_TDM_TX_0,
		.stream_name = LPASS_BE_QUIN_TDM_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(quin_tdm_tx_0),
	},
	{
		.name = LPASS_BE_SEN_TDM_RX_0,
		.stream_name = LPASS_BE_SEN_TDM_RX_0,
		.playback_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(sen_tdm_rx_0),
	},
	{
		.name = LPASS_BE_SEN_TDM_TX_0,
		.stream_name = LPASS_BE_SEN_TDM_TX_0,
		.capture_only = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ops = &msm_common_be_ops,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(sen_tdm_tx_0),
	},
};

static struct snd_soc_dai_link msm_waipio_dai_links[
			ARRAY_SIZE(msm_wsa_cdc_dma_be_dai_links) +
			ARRAY_SIZE(msm_wsa2_cdc_dma_be_dai_links) +
			ARRAY_SIZE(msm_wsa_wsa2_cdc_dma_be_dai_links) +
			ARRAY_SIZE(msm_rx_tx_cdc_dma_be_dai_links) +
			ARRAY_SIZE(msm_va_cdc_dma_be_dai_links) +
			ARRAY_SIZE(ext_disp_be_dai_link) +
			ARRAY_SIZE(msm_common_be_dai_links) +
			ARRAY_SIZE(msm_wcn_be_dai_links) +
			ARRAY_SIZE(msm_wcn_btfm_be_dai_links) +
			ARRAY_SIZE(msm_mi2s_dai_links) +
			ARRAY_SIZE(msm_tdm_dai_links)];


static int msm_populate_dai_link_component_of_node(
					struct snd_soc_card *card)
{
	int i, j, index, ret = 0;
	struct device *cdev = card->dev;
	struct snd_soc_dai_link *dai_link = card->dai_link;
	struct device_node *np = NULL;
	int codecs_enabled = 0;
	struct snd_soc_dai_link_component *codecs_comp = NULL;

	if (!cdev) {
		dev_err(cdev, "%s: Sound card device memory NULL\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < card->num_links; i++) {
		if (dai_link[i].init == NULL)
			dai_link[i].init = &msm_common_dai_link_init;

		/* populate codec_of_node for snd card dai links */
		if (dai_link[i].num_codecs > 0) {
			for (j = 0; j < dai_link[i].num_codecs; j++) {
				if (dai_link[i].codecs[j].of_node ||
						!dai_link[i].codecs[j].name)
					continue;

				index = of_property_match_string(cdev->of_node,
						"asoc-codec-names",
						dai_link[i].codecs[j].name);
				if (index < 0)
					continue;
				np = of_parse_phandle(cdev->of_node,
						      "asoc-codec",
						      index);
				if (!np) {
					dev_err(cdev, "%s: retrieving phandle for codec %s failed\n",
						__func__,
						dai_link[i].codecs[j].name);
					ret = -ENODEV;
					goto err;
				}
				dai_link[i].codecs[j].of_node = np;
				dai_link[i].codecs[j].name = NULL;
			}
		}
	}

	/* In multi-codec scenario, check if codecs are enabled for this platform */
	for (i = 0; i < card->num_links; i++) {
		codecs_enabled = 0;
		if (dai_link[i].num_codecs > 1) {
			for (j = 0; j < dai_link[i].num_codecs; j++) {
				if (!dai_link[i].codecs[j].of_node)
					continue;

				np = dai_link[i].codecs[j].of_node;
				if (!of_device_is_available(np)) {
				    dev_err(cdev, "%s: codec is disabled: %s\n",
								__func__,
								np->full_name);
							dai_link[i].codecs[j].of_node = NULL;
							continue;
				}

				codecs_enabled++;
			}
			if (codecs_enabled > 0 &&
				    codecs_enabled < dai_link[i].num_codecs) {
				codecs_comp = devm_kzalloc(cdev,
				    sizeof(struct snd_soc_dai_link_component)
				    * codecs_enabled, GFP_KERNEL);
				if (!codecs_comp) {
					dev_err(cdev, "%s: %s dailink codec component alloc failed\n",
						__func__, dai_link[i].name);
					ret = -ENOMEM;
					goto err;
				}
				index = 0;
				for (j = 0; j < dai_link[i].num_codecs; j++) {
					if(dai_link[i].codecs[j].of_node) {
						codecs_comp[index].of_node =
						  dai_link[i].codecs[j].of_node;
						codecs_comp[index].dai_name =
						  dai_link[i].codecs[j].dai_name;
						codecs_comp[index].name = NULL;
						index++;
					}
				}
				dai_link[i].codecs = codecs_comp;
				dai_link[i].num_codecs = codecs_enabled;
			}
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
	.name		= "waipio-stub-snd-card",
};

static struct snd_soc_dai_link msm_stub_be_dai_links[] = {
	/* Backend DAI Links */
	{
		.name = LPASS_BE_PRI_AUXPCM_RX,
		.stream_name = LPASS_BE_PRI_AUXPCM_RX,
		.playback_only = 1,
		.init = &msm_audrx_stub_init,
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
		SND_SOC_DAILINK_REG(auxpcm_rx),
	},
	{
		.name = LPASS_BE_PRI_AUXPCM_TX,
		.stream_name = LPASS_BE_PRI_AUXPCM_TX,
		.capture_only = 1,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
		SND_SOC_DAILINK_REG(auxpcm_tx),
	},
};

static struct snd_soc_dai_link msm_stub_dai_links[
			 ARRAY_SIZE(msm_stub_be_dai_links)];

static const struct of_device_id waipio_asoc_machine_of_match[]  = {
	{ .compatible = "qcom,waipio-asoc-snd",
	  .data = "codec"},
	{ .compatible = "qcom,waipio-asoc-snd-stub",
	  .data = "stub_codec"},
	{},
};

static int msm_snd_card_late_probe(struct snd_soc_card *card)
{
	struct snd_soc_component *component = NULL;
	struct snd_soc_pcm_runtime *rtd;
	int ret = 0;
	void *mbhc_calibration;
	bool is_wcd937x = false;

	rtd = snd_soc_get_pcm_runtime(card, &card->dai_link[0]);
	if (!rtd) {
		dev_err(card->dev,
			"%s: snd_soc_get_pcm_runtime for %s failed!\n",
			__func__, card->dai_link[0]);
		return -EINVAL;
	}

	component = snd_soc_rtdcom_lookup(rtd, WCD938X_DRV_NAME);
	if (!component) {
		component = snd_soc_rtdcom_lookup(rtd, WCD937X_DRV_NAME);
		if (!component) {
			pr_err("%s component is NULL\n", __func__);
			return -EINVAL;
		} else {
			is_wcd937x = true;
		}
	}

	mbhc_calibration = def_wcd_mbhc_cal();
	if (!mbhc_calibration)
		return -ENOMEM;
	wcd_mbhc_cfg.calibration = mbhc_calibration;

	if (!is_wcd937x)
		ret = wcd938x_mbhc_hs_detect(component, &wcd_mbhc_cfg);
	else
		ret = wcd937x_mbhc_hs_detect(component, &wcd_mbhc_cfg);

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

static struct snd_soc_card *populate_snd_card_dailinks(struct device *dev, int wsa_max_devs)
{
	struct snd_soc_card *card = NULL;
	struct snd_soc_dai_link *dailink = NULL;
	int total_links = 0;
	int rc = 0;
	u32 val = 0;
	const struct of_device_id *match;

	match = of_match_node(waipio_asoc_machine_of_match, dev->of_node);
	if (!match) {
		dev_err(dev, "%s: No DT match found for sound card\n",
			__func__);
		return NULL;
	}

	if (!strcmp(match->data, "codec")) {
		card = &snd_soc_card_waipio_msm;

		/* late probe uses dai link at index '0' to get wcd component */
		memcpy(msm_waipio_dai_links + total_links,
		       msm_rx_tx_cdc_dma_be_dai_links,
		       sizeof(msm_rx_tx_cdc_dma_be_dai_links));
		total_links +=
			ARRAY_SIZE(msm_rx_tx_cdc_dma_be_dai_links);

		switch (wsa_max_devs) {
		case MONO_SPEAKER:
		case STEREO_SPEAKER:
			memcpy(msm_waipio_dai_links + total_links,
			       msm_wsa_cdc_dma_be_dai_links,
			       sizeof(msm_wsa_cdc_dma_be_dai_links));
			total_links += ARRAY_SIZE(msm_wsa_cdc_dma_be_dai_links);
			break;
		case QUAD_SPEAKER:
			memcpy(msm_waipio_dai_links + total_links,
			       msm_wsa2_cdc_dma_be_dai_links,
			       sizeof(msm_wsa2_cdc_dma_be_dai_links));
			total_links += ARRAY_SIZE(msm_wsa2_cdc_dma_be_dai_links);

			memcpy(msm_waipio_dai_links + total_links,
			       msm_wsa_wsa2_cdc_dma_be_dai_links,
			       sizeof(msm_wsa_wsa2_cdc_dma_be_dai_links));
			total_links += ARRAY_SIZE(msm_wsa_wsa2_cdc_dma_be_dai_links);
			break;
		default:
			dev_dbg(dev,
				"%s: Unexpected number of WSAs, wsa_max_devs: %d\n",
				__func__, wsa_max_devs);
			break;
		}

		memcpy(msm_waipio_dai_links + total_links,
		       msm_va_cdc_dma_be_dai_links,
		       sizeof(msm_va_cdc_dma_be_dai_links));
		total_links += ARRAY_SIZE(msm_va_cdc_dma_be_dai_links);

		memcpy(msm_waipio_dai_links + total_links,
		       msm_common_be_dai_links,
		       sizeof(msm_common_be_dai_links));
		total_links += ARRAY_SIZE(msm_common_be_dai_links);

		rc = of_property_read_u32(dev->of_node,
				"qcom,mi2s-audio-intf", &val);
		if (!rc && val) {
			memcpy(msm_waipio_dai_links + total_links,
					msm_mi2s_dai_links,
					sizeof(msm_mi2s_dai_links));
			total_links += ARRAY_SIZE(msm_mi2s_dai_links);
		}

		rc = of_property_read_u32(dev->of_node,
				"qcom,tdm-audio-intf", &val);
		if (!rc && val) {
			memcpy(msm_waipio_dai_links + total_links,
					msm_tdm_dai_links,
					sizeof(msm_tdm_dai_links));
			total_links += ARRAY_SIZE(msm_tdm_dai_links);
		}

		rc = of_property_read_u32(dev->of_node,
					   "qcom,ext-disp-audio-rx", &val);
		if (!rc && val) {
			dev_dbg(dev, "%s(): ext disp audio support present\n",
				__func__);
			memcpy(msm_waipio_dai_links + total_links,
			       ext_disp_be_dai_link,
			       sizeof(ext_disp_be_dai_link));
			total_links += ARRAY_SIZE(ext_disp_be_dai_link);
		}

		rc = of_property_read_u32(dev->of_node, "qcom,wcn-bt", &val);
		if (!rc && val) {
			dev_dbg(dev, "%s(): WCN BT support present\n",
				__func__);
			memcpy(msm_waipio_dai_links + total_links,
			       msm_wcn_be_dai_links,
			       sizeof(msm_wcn_be_dai_links));
			total_links += ARRAY_SIZE(msm_wcn_be_dai_links);
		}

		rc = of_property_read_u32(dev->of_node, "qcom,wcn-btfm", &val);
		if (!rc && val) {
			dev_dbg(dev, "%s(): WCN BT FM support present\n",
				__func__);
			memcpy(msm_waipio_dai_links + total_links,
			       msm_wcn_btfm_be_dai_links,
			       sizeof(msm_wcn_btfm_be_dai_links));
			total_links += ARRAY_SIZE(msm_wcn_btfm_be_dai_links);
		}

		dailink = msm_waipio_dai_links;
	} else if(!strcmp(match->data, "stub_codec")) {
		card = &snd_soc_card_stub_msm;

		memcpy(msm_stub_dai_links,
		       msm_stub_be_dai_links,
		       sizeof(msm_stub_be_dai_links));

		dailink = msm_stub_dai_links;
		total_links = ARRAY_SIZE(msm_stub_be_dai_links);
	}

	if (card) {
		card->dai_link = dailink;
		card->num_links = total_links;
		card->late_probe = msm_snd_card_late_probe;
	}

	return card;
}

static int msm_int_wsa883x_init(struct snd_soc_pcm_runtime *rtd)
{
	u8 spkleft_ports[WSA883X_MAX_SWR_PORTS] = {0, 1, 2, 3};
	u8 spkright_ports[WSA883X_MAX_SWR_PORTS] = {0, 1, 2, 3};
	u8 spkleft_port_types[WSA883X_MAX_SWR_PORTS] = {SPKR_L, SPKR_L_COMP,
						SPKR_L_BOOST, SPKR_L_VI};
	u8 spkright_port_types[WSA883X_MAX_SWR_PORTS] = {SPKR_R, SPKR_R_COMP,
						SPKR_R_BOOST, SPKR_R_VI};
	unsigned int ch_rate[WSA883X_MAX_SWR_PORTS] = {SWR_CLK_RATE_2P4MHZ, SWR_CLK_RATE_0P6MHZ,
							SWR_CLK_RATE_0P3MHZ, SWR_CLK_RATE_1P2MHZ};
	unsigned int ch_mask[WSA883X_MAX_SWR_PORTS] = {0x1, 0xF, 0x3, 0x3};
	struct snd_soc_component *component = NULL;
	struct msm_asoc_mach_data *pdata =
				snd_soc_card_get_drvdata(rtd->card);

	if (pdata->wsa_max_devs > 0) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.1");
		if (!component) {
			pr_err("%s: wsa-codec.1 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa883x_set_channel_map(component, &spkleft_ports[0],
			WSA883X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkleft_port_types[0]);

		wsa883x_codec_info_create_codec_entry(pdata->codec_root,
				component);
	}

	/* If current platform has more than one WSA */
	if (pdata->wsa_max_devs > 1) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.2");
		if (!component) {
			pr_err("%s: wsa-codec.2 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa883x_set_channel_map(component, &spkright_ports[0],
			WSA883X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkright_port_types[0]);

		wsa883x_codec_info_create_codec_entry(pdata->codec_root,
			component);
	}

	if (pdata->wsa_max_devs > 2) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.3");
		if (!component) {
			pr_err("%s: wsa-codec.3 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa883x_set_channel_map(component, &spkleft_ports[0],
			WSA883X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkleft_port_types[0]);

		wsa883x_codec_info_create_codec_entry(pdata->codec_root,
			component);
	}

	if (pdata->wsa_max_devs > 3) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.4");
		if (!component) {
			pr_err("%s: wsa-codec.4 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa883x_set_channel_map(component, &spkright_ports[0],
			WSA883X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkright_port_types[0]);

		wsa883x_codec_info_create_codec_entry(pdata->codec_root,
			component);
	}

	msm_common_dai_link_init(rtd);

	return 0;
}

static int msm_int_wsa881x_init(struct snd_soc_pcm_runtime *rtd)
{
	u8 spkleft_ports[WSA881X_MAX_SWR_PORTS] = {0, 1, 2, 3};
	u8 spkright_ports[WSA881X_MAX_SWR_PORTS] = {0, 1, 2, 3};
	u8 spkleft_port_types[WSA881X_MAX_SWR_PORTS] = {SPKR_L, SPKR_L_COMP,
						SPKR_L_BOOST, SPKR_L_VI};
	u8 spkright_port_types[WSA881X_MAX_SWR_PORTS] = {SPKR_R, SPKR_R_COMP,
						SPKR_R_BOOST, SPKR_R_VI};
	unsigned int ch_rate[WSA881X_MAX_SWR_PORTS] = {SWR_CLK_RATE_2P4MHZ, SWR_CLK_RATE_0P6MHZ,
							SWR_CLK_RATE_0P3MHZ, SWR_CLK_RATE_1P2MHZ};
	unsigned int ch_mask[WSA881X_MAX_SWR_PORTS] = {0x1, 0xF, 0x3, 0x3};
	struct snd_soc_component *component = NULL;
	struct msm_asoc_mach_data *pdata =
				snd_soc_card_get_drvdata(rtd->card);

	if (pdata->wsa_max_devs > 0) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.1");
		if (!component) {
			pr_err("%s: wsa-codec.1 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa881x_set_channel_map(component, &spkleft_ports[0],
			WSA881X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkleft_port_types[0]);

		wsa881x_codec_info_create_codec_entry(pdata->codec_root,
				component);
	}

	/* If current platform has more than one WSA */
	if (pdata->wsa_max_devs > 1) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.2");
		if (!component) {
			pr_err("%s: wsa-codec.2 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa881x_set_channel_map(component, &spkright_ports[0],
			WSA881X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkright_port_types[0]);

		wsa881x_codec_info_create_codec_entry(pdata->codec_root,
			component);
	}

	if (pdata->wsa_max_devs > 2) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.3");
		if (!component) {
			pr_err("%s: wsa-codec.3 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa881x_set_channel_map(component, &spkleft_ports[0],
			WSA881X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkleft_port_types[0]);

		wsa881x_codec_info_create_codec_entry(pdata->codec_root,
			component);
	}

	if (pdata->wsa_max_devs > 3) {
		component = snd_soc_rtdcom_lookup(rtd, "wsa-codec.4");
		if (!component) {
			pr_err("%s: wsa-codec.4 component is NULL\n", __func__);
			return -EINVAL;
		}

		wsa881x_set_channel_map(component, &spkright_ports[0],
			WSA881X_MAX_SWR_PORTS, &ch_mask[0],
			&ch_rate[0], &spkright_port_types[0]);

		wsa881x_codec_info_create_codec_entry(pdata->codec_root,
			component);
	}

	msm_common_dai_link_init(rtd);

	return 0;
}

static int msm_int_wsa_init(struct snd_soc_pcm_runtime *rtd)
{
	struct msm_asoc_mach_data *pdata =
				snd_soc_card_get_drvdata(rtd->card);

	if (pdata->wsa881x_support)
		return msm_int_wsa881x_init(rtd);

	return msm_int_wsa883x_init(rtd);
}

static int msm_rx_tx_codec_init(struct snd_soc_pcm_runtime *rtd)
{
	int codec_variant = -1;
	struct snd_soc_component *component = NULL;
	struct snd_soc_component *lpass_cdc_component = NULL;
	struct snd_soc_dapm_context *dapm = NULL;
	struct snd_info_entry *entry = NULL;
	struct snd_card *card = NULL;
	struct msm_asoc_mach_data *pdata =
				snd_soc_card_get_drvdata(rtd->card);
	int ret = 0;

	lpass_cdc_component = snd_soc_rtdcom_lookup(rtd, "lpass-cdc");
	if (!lpass_cdc_component) {
		pr_err("%s: could not find component for lpass-cdc\n",
			__func__);
		return ret;
	}

	dapm = snd_soc_component_get_dapm(lpass_cdc_component);

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


	card = rtd->card->snd_card;
	if (!pdata->codec_root) {
		entry = msm_snd_info_create_subdir(card->module, "codecs",
						 card->proc_root);
		if (!entry) {
			pr_debug("%s: Cannot create codecs module entry\n",
				 __func__);
			return ret;
		}
		pdata->codec_root = entry;
	}
	lpass_cdc_info_create_codec_entry(pdata->codec_root, lpass_cdc_component);
	lpass_cdc_register_wake_irq(lpass_cdc_component, false);

	if (pdata->wcd_disabled)
		goto done;

	component = snd_soc_rtdcom_lookup(rtd, WCD938X_DRV_NAME);
	if (!component) {
		component = snd_soc_rtdcom_lookup(rtd, WCD937X_DRV_NAME);
		if (!component) {
			pr_err("%s component is NULL\n", __func__);
			return -EINVAL;
		}
	}
	dapm = snd_soc_component_get_dapm(component);
	card = component->card->snd_card;

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
		entry = msm_snd_info_create_subdir(card->module, "codecs",
						 card->proc_root);
		if (!entry) {
			dev_dbg(component->dev, "%s: Cannot create codecs module entry\n",
				 __func__);
			 return 0;
		}
		pdata->codec_root = entry;
	}
	if(!strncmp(component->driver->name, WCD937X_DRV_NAME,
			strlen(WCD937X_DRV_NAME))){
		wcd937x_info_create_codec_entry(pdata->codec_root, component);
		codec_variant = wcd937x_get_codec_variant(component);
		dev_dbg(component->dev, "%s: variant %d\n",__func__, codec_variant);
		lpass_cdc_set_port_map(lpass_cdc_component,
			ARRAY_SIZE(sm_port_map_wcd937x), sm_port_map_wcd937x);
	} else {
		wcd938x_info_create_codec_entry(pdata->codec_root, component);
		codec_variant = wcd938x_get_codec_variant(component);
		dev_dbg(component->dev, "%s: variant %d\n", __func__, codec_variant);
		lpass_cdc_set_port_map(lpass_cdc_component, ARRAY_SIZE(sm_port_map), sm_port_map);

		/* check if the variant is wcd9385 and set RX HIFI filter capability */
		if (codec_variant == WCD9385)
			ret = lpass_cdc_rx_set_fir_capability(lpass_cdc_component, true);
		else
			ret = lpass_cdc_rx_set_fir_capability(lpass_cdc_component, false);
	}

	if (ret < 0) {
		dev_err(component->dev, "%s: set fir capability failed: %d\n",
			__func__, ret);
		return ret;
	}
done:
	codec_reg_done = true;
	msm_common_dai_link_init(rtd);

	return ret;
}

static int waipio_ssr_enable(struct device *dev, void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct snd_soc_pcm_runtime *rtd = NULL, *rtd_wcd = NULL, *rtd_wsa = NULL;
	struct msm_asoc_mach_data *pdata = NULL;
	int ret = 0;

	if (!card) {
		dev_err(dev, "%s: card is NULL\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	if (!strcmp(card->name, "waipio-stub-snd-card")) {
		/* TODO */
		dev_dbg(dev, "%s: TODO \n", __func__);
	}

	snd_card_notify_user(SND_CARD_STATUS_ONLINE);
	dev_dbg(dev, "%s: setting snd_card to ONLINE\n", __func__);

	pdata = snd_soc_card_get_drvdata(card);
	if (!pdata) {
		dev_dbg(dev, "%s: pdata is NULL \n", __func__);
		goto err;
	}
	rtd_wcd = snd_soc_get_pcm_runtime(card, &card->dai_link[0]);
	if (!rtd_wcd) {
		dev_dbg(dev,
			"%s: snd_soc_get_pcm_runtime for %s failed!\n",
			__func__, card->dai_link[0]);
	}

	if (pdata->wsa_max_devs > 0) {
		rtd_wsa = snd_soc_get_pcm_runtime(card,
			&card->dai_link[ARRAY_SIZE(msm_rx_tx_cdc_dma_be_dai_links)]);
		if (!rtd_wsa) {
			dev_dbg(dev,
			"%s: snd_soc_get_pcm_runtime for %s failed!\n",
			__func__, card->dai_link[ARRAY_SIZE(msm_rx_tx_cdc_dma_be_dai_links)]);
		}
	}
	/* set UPD configuration */
	if(!pdata->upd_config.backend_used) {
		dev_dbg(dev,
		"%s: upd- backend_used is NULL\n", __func__);
		goto err;
	}
	if (!strcmp(pdata->upd_config.backend_used, "wsa")) {
		if (!rtd_wsa)
			goto err;
		else
			rtd = rtd_wsa;
	} else if(!strcmp(pdata->upd_config.backend_used, "wcd")) {
		if (!rtd_wcd &&!pdata->wcd_disabled)
			goto err;
		else
			rtd = rtd_wcd;
	} else {
		dev_err(card->dev, "%s: Invalid backend to set UPD config\n",
			__func__);
		goto err;
	}

	if (rtd)
		msm_set_upd_config(rtd);

err:
	return ret;
}

static void waipio_ssr_disable(struct device *dev, void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	if (!card) {
		dev_err(dev, "%s: card is NULL\n", __func__);
		return;
	}

	dev_dbg(dev, "%s: setting snd_card to OFFLINE\n", __func__);
	snd_card_notify_user(SND_CARD_STATUS_OFFLINE);

	if (!strcmp(card->name, "waipio-stub-snd-card")) {
		/* TODO */
		dev_dbg(dev, "%s: TODO \n", __func__);
	}
}

static const struct snd_event_ops waipio_ssr_ops = {
	.enable = waipio_ssr_enable,
	.disable = waipio_ssr_disable,
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

	ret = snd_event_master_register(dev, &waipio_ssr_ops,
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

static int msm_asoc_parse_soundcard_name(struct platform_device *pdev,
					 struct snd_soc_card *card)
{
	struct nvmem_cell *cell = NULL;
	size_t len = 0;
	u32 *buf = NULL;
	u32 adsp_var_idx = 0;
	int ret = 0;

	/* get adsp variant idx */
	cell = nvmem_cell_get(&pdev->dev, "adsp_variant");
	if (IS_ERR_OR_NULL(cell)) {
		dev_dbg(&pdev->dev, "%s: FAILED to get nvmem cell \n", __func__);
		goto parse;
	}
	buf = nvmem_cell_read(cell, &len);
	nvmem_cell_put(cell);
	if (IS_ERR_OR_NULL(buf)) {
		dev_dbg(&pdev->dev, "%s: FAILED to read nvmem cell \n", __func__);
		goto parse;
	}
	if (len <= 0 || len > sizeof(u32)) {
		dev_dbg(&pdev->dev, "%s: nvmem cell length out of range: %d\n",
			__func__, len);
		kfree(buf);
		goto parse;
	}
	memcpy(&adsp_var_idx, buf, len);
	kfree(buf);

parse:
	if(adsp_var_idx)
		ret = snd_soc_of_parse_card_name(card, "qcom,sku-model");
	else
		ret = snd_soc_of_parse_card_name(card, "qcom,model");

	if (ret)
		dev_err(&pdev->dev, "%s: parse card name failed, err:%d\n",
			__func__, ret);

	return ret;
}

static int msm_asoc_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = NULL;
	struct msm_asoc_mach_data *pdata = NULL;
	int ret = 0;
	struct clk *lpass_audio_hw_vote = NULL;

	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "%s: No platform supplied from device tree\n", __func__);
		return -EINVAL;
	}
	pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct msm_asoc_mach_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	of_property_read_u32(pdev->dev.of_node,
						"qcom,wcd-disabled",
						&pdata->wcd_disabled);

	/* Get maximum WSA device count for this platform */
	ret = of_property_read_u32(pdev->dev.of_node,
		"qcom,wsa-max-devs", &pdata->wsa_max_devs);
	if (ret) {
		dev_info(&pdev->dev,
		"%s: wsa-max-devs property missing in DT %s, ret = %d\n",
		__func__, pdev->dev.of_node->full_name, ret);
		pdata->wsa_max_devs = 0;
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				"qcom,wsa881x-support", &pdata->wsa881x_support);
	if (ret) {
		dev_info(&pdev->dev, "%s: does not upport wsa881x, ret = %d\n",
					__func__, ret);
		pdata->wsa881x_support = 0;
	}

	card = populate_snd_card_dailinks(&pdev->dev, pdata->wsa_max_devs);
	if (!card) {
		dev_err(&pdev->dev, "%s: Card uninitialized\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, pdata);

	ret = msm_asoc_parse_soundcard_name(pdev, card);
	if (ret) {
		dev_err(&pdev->dev, "%s: parse soundcard name failed, err:%d\n",
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

	/* parse upd configuration */
	msm_parse_upd_configuration(pdev, pdata);

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
	pdata->dmic67_gpio_p = of_parse_phandle(pdev->dev.of_node,
					      "qcom,cdc-dmic67-gpios",
					       0);
	if (pdata->dmic01_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic01_gpio_p, false);
	if (pdata->dmic23_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic23_gpio_p, false);
	if (pdata->dmic45_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic45_gpio_p, false);
	if (pdata->dmic67_gpio_p)
		msm_cdc_pinctrl_set_wakeup_capable(pdata->dmic67_gpio_p, false);

	msm_common_snd_init(pdev, card);

	/* Register LPASS audio hw vote */
	lpass_audio_hw_vote = devm_clk_get(&pdev->dev, "lpass_audio_hw_vote");
	if (IS_ERR(lpass_audio_hw_vote)) {
		ret = PTR_ERR(lpass_audio_hw_vote);
		dev_dbg(&pdev->dev, "%s: clk get %s failed %d\n",
			__func__, "lpass_audio_hw_vote", ret);
		lpass_audio_hw_vote = NULL;
		ret = 0;
	}

	pdata->lpass_audio_hw_vote = lpass_audio_hw_vote;
	pdata->core_audio_vote_count = 0;

	ret = msm_audio_ssr_register(&pdev->dev);
	if (ret)
		pr_err("%s: Registration with SND event FWK failed ret = %d\n",
			__func__, ret);

	is_initial_boot = true;

	/* change card status to ONLINE */
	dev_dbg(&pdev->dev, "%s: setting snd_card to ONLINE\n", __func__);
	snd_card_set_card_status(SND_CARD_STATUS_ONLINE);

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

static struct platform_driver waipio_asoc_machine_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = waipio_asoc_machine_of_match,
		.suppress_bind_attrs = true,
	},
	.probe = msm_asoc_machine_probe,
	.remove = msm_asoc_machine_remove,
};

static int __init msm_asoc_machine_init(void)
{
	snd_card_sysfs_init();
	return platform_driver_register(&waipio_asoc_machine_driver);
}
module_init(msm_asoc_machine_init);

static void __exit msm_asoc_machine_exit(void)
{
	platform_driver_unregister(&waipio_asoc_machine_driver);
}
module_exit(msm_asoc_machine_exit);

MODULE_SOFTDEP("pre: bt_fm_slim");
MODULE_DESCRIPTION("ALSA SoC msm");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, waipio_asoc_machine_of_match);
