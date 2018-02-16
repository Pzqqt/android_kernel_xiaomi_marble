/* Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
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
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/info.h>
#include <dsp/audio_notifier.h>
#include <dsp/q6afe-v2.h>
#include <dsp/q6core.h>
#include "device_event.h"
#include "msm-pcm-routing-v2.h"
#include "codecs/wsa881x.h"

#define DRV_NAME "sdm855-asoc-snd"

#define __CHIPSET__ "SDM855 "
#define MSM_DAILINK_NAME(name) (__CHIPSET__#name)

#define DEV_NAME_STR_LEN            32

#define SAMPLING_RATE_8KHZ      8000
#define SAMPLING_RATE_11P025KHZ 11025
#define SAMPLING_RATE_16KHZ     16000
#define SAMPLING_RATE_22P05KHZ  22050
#define SAMPLING_RATE_32KHZ     32000
#define SAMPLING_RATE_44P1KHZ   44100
#define SAMPLING_RATE_48KHZ     48000
#define SAMPLING_RATE_88P2KHZ   88200
#define SAMPLING_RATE_96KHZ     96000
#define SAMPLING_RATE_176P4KHZ  176400
#define SAMPLING_RATE_192KHZ    192000
#define SAMPLING_RATE_352P8KHZ  352800
#define SAMPLING_RATE_384KHZ    384000

struct msm_wsa881x_dev_info {
	struct device_node *of_node;
	u32 index;
};

struct msm_asoc_mach_data {
	u32 mclk_freq;
	struct snd_info_entry *codec_root;
};

struct dev_config {
	u32 sample_rate;
	u32 bit_format;
	u32 channels;
};

static struct platform_device *spdev;

static struct snd_soc_aux_dev *msm_aux_dev;
static struct snd_soc_codec_conf *msm_codec_conf;

enum {
	SLIM_RX_0 = 0,
	SLIM_RX_MAX,
};

enum {
	SLIM_TX_0 = 0,
	SLIM_TX_MAX,
};

/* Default configuration of slimbus channels */
static struct dev_config slim_rx_cfg[] = {
	[SLIM_RX_0] = {SAMPLING_RATE_48KHZ, SNDRV_PCM_FORMAT_S16_LE, 1},
};

static struct dev_config slim_tx_cfg[] = {
	[SLIM_TX_0] = {SAMPLING_RATE_48KHZ, SNDRV_PCM_FORMAT_S16_LE, 1},
};

static const char *const slim_rx_ch_text[] = {"One", "Two"};
static const char *const slim_tx_ch_text[] = {"One", "Two", "Three", "Four",
						"Five", "Six", "Seven",
						"Eight"};
static char const *slim_sample_rate_text[] = {"KHZ_8", "KHZ_16",
					"KHZ_32", "KHZ_44P1", "KHZ_48",
					"KHZ_88P2", "KHZ_96", "KHZ_176P4",
					"KHZ_192", "KHZ_352P8", "KHZ_384"};
static char const *bit_format_text[] = {"S16_LE", "S24_LE", "S24_3LE",
					  "S32_LE"};

static SOC_ENUM_SINGLE_EXT_DECL(slim_0_rx_chs, slim_rx_ch_text);
static SOC_ENUM_SINGLE_EXT_DECL(slim_0_tx_chs, slim_tx_ch_text);
static SOC_ENUM_SINGLE_EXT_DECL(slim_0_rx_format, bit_format_text);
static SOC_ENUM_SINGLE_EXT_DECL(slim_0_tx_format, bit_format_text);
static SOC_ENUM_SINGLE_EXT_DECL(slim_0_rx_sample_rate, slim_sample_rate_text);
static SOC_ENUM_SINGLE_EXT_DECL(slim_0_tx_sample_rate, slim_sample_rate_text);

static int slim_get_sample_rate_val(int sample_rate)
{
	int sample_rate_val = 0;

	switch (sample_rate) {
	case SAMPLING_RATE_8KHZ:
		sample_rate_val = 0;
		break;
	case SAMPLING_RATE_16KHZ:
		sample_rate_val = 1;
		break;
	case SAMPLING_RATE_32KHZ:
		sample_rate_val = 2;
		break;
	case SAMPLING_RATE_44P1KHZ:
		sample_rate_val = 3;
		break;
	case SAMPLING_RATE_48KHZ:
		sample_rate_val = 4;
		break;
	case SAMPLING_RATE_88P2KHZ:
		sample_rate_val = 5;
		break;
	case SAMPLING_RATE_96KHZ:
		sample_rate_val = 6;
		break;
	case SAMPLING_RATE_176P4KHZ:
		sample_rate_val = 7;
		break;
	case SAMPLING_RATE_192KHZ:
		sample_rate_val = 8;
		break;
	case SAMPLING_RATE_352P8KHZ:
		sample_rate_val = 9;
		break;
	case SAMPLING_RATE_384KHZ:
		sample_rate_val = 10;
		break;
	default:
		sample_rate_val = 4;
		break;
	}
	return sample_rate_val;
}

static int slim_get_sample_rate(int value)
{
	int sample_rate = 0;

	switch (value) {
	case 0:
		sample_rate = SAMPLING_RATE_8KHZ;
		break;
	case 1:
		sample_rate = SAMPLING_RATE_16KHZ;
		break;
	case 2:
		sample_rate = SAMPLING_RATE_32KHZ;
		break;
	case 3:
		sample_rate = SAMPLING_RATE_44P1KHZ;
		break;
	case 4:
		sample_rate = SAMPLING_RATE_48KHZ;
		break;
	case 5:
		sample_rate = SAMPLING_RATE_88P2KHZ;
		break;
	case 6:
		sample_rate = SAMPLING_RATE_96KHZ;
		break;
	case 7:
		sample_rate = SAMPLING_RATE_176P4KHZ;
		break;
	case 8:
		sample_rate = SAMPLING_RATE_192KHZ;
		break;
	case 9:
		sample_rate = SAMPLING_RATE_352P8KHZ;
		break;
	case 10:
		sample_rate = SAMPLING_RATE_384KHZ;
		break;
	default:
		sample_rate = SAMPLING_RATE_48KHZ;
		break;
	}
	return sample_rate;
}

static int slim_get_bit_format_val(int bit_format)
{
	int val = 0;

	switch (bit_format) {
	case SNDRV_PCM_FORMAT_S32_LE:
		val = 3;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		val = 2;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		val = 1;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
	default:
		val = 0;
		break;
	}
	return val;
}

static int slim_get_bit_format(int val)
{
	int bit_fmt = SNDRV_PCM_FORMAT_S16_LE;

	switch (val) {
	case 0:
		bit_fmt = SNDRV_PCM_FORMAT_S16_LE;
		break;
	case 1:
		bit_fmt = SNDRV_PCM_FORMAT_S24_LE;
		break;
	case 2:
		bit_fmt = SNDRV_PCM_FORMAT_S24_3LE;
		break;
	case 3:
		bit_fmt = SNDRV_PCM_FORMAT_S32_LE;
		break;
	default:
		bit_fmt = SNDRV_PCM_FORMAT_S16_LE;
		break;
	}
	return bit_fmt;
}

static int slim_get_port_idx(struct snd_kcontrol *kcontrol)
{
	int port_id = 0;

	if (strnstr(kcontrol->id.name, "SLIM_0_RX", sizeof("SLIM_0_RX"))) {
		port_id = SLIM_RX_0;
	} else if (strnstr(kcontrol->id.name,
		   "SLIM_0_TX", sizeof("SLIM_0_TX"))) {
		port_id = SLIM_TX_0;
	} else {
		pr_err("%s: unsupported channel: %s",
			__func__, kcontrol->id.name);
		return -EINVAL;
	}

	return port_id;
}

static int slim_rx_sample_rate_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	ucontrol->value.enumerated.item[0] =
		slim_get_sample_rate_val(slim_rx_cfg[ch_num].sample_rate);

	pr_debug("%s: slim[%d]_rx_sample_rate = %d, item = %d\n", __func__,
		 ch_num, slim_rx_cfg[ch_num].sample_rate,
		 ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_rx_sample_rate_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	slim_rx_cfg[ch_num].sample_rate =
		slim_get_sample_rate(ucontrol->value.enumerated.item[0]);

	pr_debug("%s: slim[%d]_rx_sample_rate = %d, item = %d\n", __func__,
		 ch_num, slim_rx_cfg[ch_num].sample_rate,
		 ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_tx_sample_rate_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	ucontrol->value.enumerated.item[0] =
		slim_get_sample_rate_val(slim_tx_cfg[ch_num].sample_rate);

	pr_debug("%s: slim[%d]_tx_sample_rate = %d, item = %d\n", __func__,
		 ch_num, slim_tx_cfg[ch_num].sample_rate,
		 ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_tx_sample_rate_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	int sample_rate = 0;
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	sample_rate = slim_get_sample_rate(ucontrol->value.enumerated.item[0]);
	if (sample_rate == SAMPLING_RATE_44P1KHZ) {
		pr_err("%s: Unsupported sample rate %d: for Tx path\n",
			__func__, sample_rate);
		return -EINVAL;
	}
	slim_tx_cfg[ch_num].sample_rate = sample_rate;

	pr_debug("%s: slim[%d]_tx_sample_rate = %d, value = %d\n", __func__,
		 ch_num, slim_tx_cfg[ch_num].sample_rate,
		 ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_rx_bit_format_get(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	ucontrol->value.enumerated.item[0] =
			slim_get_bit_format_val(slim_rx_cfg[ch_num].bit_format);

	pr_debug("%s: slim[%d]_rx_bit_format = %d, ucontrol value = %d\n",
		 __func__, ch_num, slim_rx_cfg[ch_num].bit_format,
			ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_rx_bit_format_put(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	slim_rx_cfg[ch_num].bit_format =
		slim_get_bit_format(ucontrol->value.enumerated.item[0]);

	pr_debug("%s: slim[%d]_rx_bit_format = %d, ucontrol value = %d\n",
		 __func__, ch_num, slim_rx_cfg[ch_num].bit_format,
			ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_tx_bit_format_get(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	ucontrol->value.enumerated.item[0] =
			slim_get_bit_format_val(slim_tx_cfg[ch_num].bit_format);

	pr_debug("%s: slim[%d]_tx_bit_format = %d, ucontrol value = %d\n",
		 __func__, ch_num, slim_tx_cfg[ch_num].bit_format,
			ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_tx_bit_format_put(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	slim_tx_cfg[ch_num].bit_format =
		slim_get_bit_format(ucontrol->value.enumerated.item[0]);

	pr_debug("%s: slim[%d]_tx_bit_format = %d, ucontrol value = %d\n",
		 __func__, ch_num, slim_tx_cfg[ch_num].bit_format,
			ucontrol->value.enumerated.item[0]);

	return 0;
}

static int slim_rx_ch_get(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	pr_debug("%s: msm_slim_[%d]_rx_ch  = %d\n", __func__,
		 ch_num, slim_rx_cfg[ch_num].channels);
	ucontrol->value.enumerated.item[0] = slim_rx_cfg[ch_num].channels - 1;

	return 0;
}

static int slim_rx_ch_put(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	slim_rx_cfg[ch_num].channels = ucontrol->value.enumerated.item[0] + 1;
	pr_debug("%s: msm_slim_[%d]_rx_ch  = %d\n", __func__,
		 ch_num, slim_rx_cfg[ch_num].channels);

	return 1;
}

static int slim_tx_ch_get(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	pr_debug("%s: msm_slim_[%d]_tx_ch  = %d\n", __func__,
		 ch_num, slim_tx_cfg[ch_num].channels);
	ucontrol->value.enumerated.item[0] = slim_tx_cfg[ch_num].channels - 1;

	return 0;
}

static int slim_tx_ch_put(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	int ch_num = slim_get_port_idx(kcontrol);

	if (ch_num < 0)
		return ch_num;

	slim_tx_cfg[ch_num].channels = ucontrol->value.enumerated.item[0] + 1;
	pr_debug("%s: msm_slim_[%d]_tx_ch = %d\n", __func__,
		 ch_num, slim_tx_cfg[ch_num].channels);

	return 1;
}

static const struct snd_kcontrol_new msm_snd_controls[] = {
	SOC_ENUM_EXT("SLIM_0_RX Channels", slim_0_rx_chs,
			slim_rx_ch_get, slim_rx_ch_put),
	SOC_ENUM_EXT("SLIM_0_TX Channels", slim_0_tx_chs,
			slim_tx_ch_get, slim_tx_ch_put),
	SOC_ENUM_EXT("SLIM_0_RX Format", slim_0_rx_format,
			slim_rx_bit_format_get, slim_rx_bit_format_put),
	SOC_ENUM_EXT("SLIM_0_TX Format", slim_0_tx_format,
			slim_tx_bit_format_get, slim_tx_bit_format_put),
	SOC_ENUM_EXT("SLIM_0_RX SampleRate", slim_0_rx_sample_rate,
			slim_rx_sample_rate_get, slim_rx_sample_rate_put),
	SOC_ENUM_EXT("SLIM_0_TX SampleRate", slim_0_tx_sample_rate,
			slim_tx_sample_rate_get, slim_tx_sample_rate_put),
};

static inline int param_is_mask(int p)
{
	return (p >= SNDRV_PCM_HW_PARAM_FIRST_MASK) &&
			(p <= SNDRV_PCM_HW_PARAM_LAST_MASK);
}

static inline struct snd_mask *param_to_mask(struct snd_pcm_hw_params *p,
					     int n)
{
	return &(p->masks[n - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
}

static void param_set_mask(struct snd_pcm_hw_params *p, int n,
			   unsigned int bit)
{
	if (bit >= SNDRV_MASK_MAX)
		return;
	if (param_is_mask(n)) {
		struct snd_mask *m = param_to_mask(p, n);

		m->bits[0] = 0;
		m->bits[1] = 0;
		m->bits[bit >> 5] |= (1 << (bit & 31));
	}
}

static int msm_slim_get_ch_from_beid(int32_t be_id)
{
	int ch_id = 0;

	switch (be_id) {
	case MSM_BACKEND_DAI_SLIMBUS_0_RX:
		ch_id = SLIM_RX_0;
		break;
	case MSM_BACKEND_DAI_SLIMBUS_0_TX:
		ch_id = SLIM_TX_0;
		break;
	default:
		ch_id = SLIM_RX_0;
		break;
	}

	return ch_id;
}

static int msm_be_hw_params_fixup(struct snd_soc_pcm_runtime *rtd,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_dai_link *dai_link = rtd->dai_link;
	struct snd_interval *rate = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_RATE);
	struct snd_interval *channels = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_CHANNELS);
	int rc = 0;
	int idx;

	pr_debug("%s: format = %d, rate = %d\n",
		  __func__, params_format(params), params_rate(params));

	switch (dai_link->id) {
	case MSM_BACKEND_DAI_SLIMBUS_0_RX:
		idx = msm_slim_get_ch_from_beid(dai_link->id);
		param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT,
				slim_rx_cfg[idx].bit_format);
		rate->min = rate->max = slim_rx_cfg[idx].sample_rate;
		channels->min = channels->max = slim_rx_cfg[idx].channels;
		break;
	case MSM_BACKEND_DAI_SLIMBUS_0_TX:
		idx = msm_slim_get_ch_from_beid(dai_link->id);
		param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT,
				slim_tx_cfg[idx].bit_format);
		rate->min = rate->max = slim_tx_cfg[idx].sample_rate;
		channels->min = channels->max = slim_tx_cfg[idx].channels;
		break;
	default:
		rate->min = rate->max = SAMPLING_RATE_48KHZ;
		break;
	}

	return rc;
}

static int msm_populate_dai_link_component_of_node(
					struct snd_soc_card *card)
{
	int i, index, ret = 0;
	struct device *cdev = card->dev;
	struct snd_soc_dai_link *dai_link = card->dai_link;
	struct device_node *np;

	if (!cdev) {
		pr_err("%s: Sound card device memory NULL\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < card->num_links; i++) {
		if (dai_link[i].platform_of_node && dai_link[i].cpu_of_node)
			continue;

		/* populate platform_of_node for snd card dai links */
		if (dai_link[i].platform_name &&
		    !dai_link[i].platform_of_node) {
			index = of_property_match_string(cdev->of_node,
						"asoc-platform-names",
						dai_link[i].platform_name);
			if (index < 0) {
				pr_err("%s: No match found for platform name: %s\n",
					__func__, dai_link[i].platform_name);
				ret = index;
				goto err;
			}
			np = of_parse_phandle(cdev->of_node, "asoc-platform",
					      index);
			if (!np) {
				pr_err("%s: retrieving phandle for platform %s, index %d failed\n",
					__func__, dai_link[i].platform_name,
					index);
				ret = -ENODEV;
				goto err;
			}
			dai_link[i].platform_of_node = np;
			dai_link[i].platform_name = NULL;
		}

		/* populate cpu_of_node for snd card dai links */
		if (dai_link[i].cpu_dai_name && !dai_link[i].cpu_of_node) {
			index = of_property_match_string(cdev->of_node,
						 "asoc-cpu-names",
						 dai_link[i].cpu_dai_name);
			if (index >= 0) {
				np = of_parse_phandle(cdev->of_node, "asoc-cpu",
						index);
				if (!np) {
					pr_err("%s: retrieving phandle for cpu dai %s failed\n",
						__func__,
						dai_link[i].cpu_dai_name);
					ret = -ENODEV;
					goto err;
				}
				dai_link[i].cpu_of_node = np;
				dai_link[i].cpu_dai_name = NULL;
			}
		}

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
				pr_err("%s: retrieving phandle for codec %s failed\n",
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
	int ret = 0;
	struct snd_soc_codec *codec = rtd->codec;

	ret = snd_soc_add_codec_controls(codec, msm_snd_controls,
					 ARRAY_SIZE(msm_snd_controls));
	if (ret < 0) {
		dev_err(codec->dev,
			"%s: add_codec_controls failed, err = %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int msm_snd_stub_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	int ret = 0;
	unsigned int rx_ch[] = {144, 145, 146, 147, 148, 149, 150,
				151};
	unsigned int tx_ch[] = {128, 129, 130, 131, 132, 133,
				134, 135, 136, 137, 138, 139,
				140, 141, 142, 143};

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		ret = snd_soc_dai_set_channel_map(cpu_dai, 0, 0,
						  slim_rx_cfg[SLIM_RX_0].channels,
						  rx_ch);
		if (ret < 0)
			pr_err("%s: RX failed to set cpu chan map error %d\n",
				__func__, ret);
	} else {
		ret = snd_soc_dai_set_channel_map(cpu_dai,
						  slim_tx_cfg[SLIM_TX_0].channels,
						  tx_ch, 0, 0);
		if (ret < 0)
			pr_err("%s: TX failed to set cpu chan map error %d\n",
				__func__, ret);
	}

	return ret;
}

static struct snd_soc_ops msm_stub_be_ops = {
	.hw_params = msm_snd_stub_hw_params,
};

static struct snd_soc_dai_link msm_stub_fe_dai_links[] = {

	/* FrontEnd DAI Links */
	{
		.name = "MSMSTUB Media1",
		.stream_name = "MultiMedia1",
		.cpu_dai_name = "MultiMedia1",
		.platform_name = "msm-pcm-dsp.0",
		.dynamic = 1,
		.async_ops = ASYNC_DPCM_SND_SOC_PREPARE,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = MSM_FRONTEND_DAI_MULTIMEDIA1
	},
};

static struct snd_soc_dai_link msm_stub_be_dai_links[] = {

	/* Backend DAI Links */
	{
		.name = LPASS_BE_SLIMBUS_0_RX,
		.stream_name = "Slimbus Playback",
		.cpu_dai_name = "msm-dai-q6-dev.16384",
		.platform_name = "msm-pcm-routing",
		.codec_name = "msm-stub-codec.1",
		.codec_dai_name = "msm-stub-rx",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = MSM_BACKEND_DAI_SLIMBUS_0_RX,
		.init = &msm_audrx_stub_init,
		.be_hw_params_fixup = msm_be_hw_params_fixup,
		.ignore_pmdown_time = 1, /* dai link has playback support */
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
	},
	{
		.name = LPASS_BE_SLIMBUS_0_TX,
		.stream_name = "Slimbus Capture",
		.cpu_dai_name = "msm-dai-q6-dev.16385",
		.platform_name = "msm-pcm-routing",
		.codec_name = "msm-stub-codec.1",
		.codec_dai_name = "msm-stub-tx",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = MSM_BACKEND_DAI_SLIMBUS_0_TX,
		.be_hw_params_fixup = msm_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
	},
};

static struct snd_soc_dai_link msm_stub_dai_links[
			 ARRAY_SIZE(msm_stub_fe_dai_links) +
			 ARRAY_SIZE(msm_stub_be_dai_links)];

struct snd_soc_card snd_soc_card_stub_msm = {
	.name		= "sdm855-stub-snd-card",
};

static const struct of_device_id sdm855_asoc_machine_of_match[]  = {
	{ .compatible = "qcom,sdm855-asoc-snd-stub",
	  .data = "stub_codec"},
	{},
};

static int sdm855_notifier_service_cb(struct notifier_block *this,
					unsigned long opcode, void *ptr)
{
	return NOTIFY_OK;
}

static struct notifier_block service_nb = {
	.notifier_call  = sdm855_notifier_service_cb,
	.priority = -INT_MAX,
};

static struct snd_soc_card *populate_snd_card_dailinks(struct device *dev)
{
	struct snd_soc_card *card = NULL;
	struct snd_soc_dai_link *dailink;
	int len_1, len_2;
	int total_links;
	const struct of_device_id *match;

	match = of_match_node(sdm855_asoc_machine_of_match, dev->of_node);
	if (!match) {
		dev_err(dev, "%s: No DT match found for sound card\n",
			__func__);
		return NULL;
	}

	if (!strcmp(match->data, "stub_codec")) {
		card = &snd_soc_card_stub_msm;
		len_1 = ARRAY_SIZE(msm_stub_fe_dai_links);
		len_2 = len_1 + ARRAY_SIZE(msm_stub_be_dai_links);

		memcpy(msm_stub_dai_links,
		       msm_stub_fe_dai_links,
		       sizeof(msm_stub_fe_dai_links));
		memcpy(msm_stub_dai_links + len_1,
		       msm_stub_be_dai_links,
		       sizeof(msm_stub_be_dai_links));

		dailink = msm_stub_dai_links;
		total_links = len_2;
	}

	if (card) {
		card->dai_link = dailink;
		card->num_links = total_links;
	}

	return card;
}

static int msm_wsa881x_init(struct snd_soc_component *component)
{
	u8 spkleft_ports[WSA881X_MAX_SWR_PORTS] = {100, 101, 102, 106};
	u8 spkright_ports[WSA881X_MAX_SWR_PORTS] = {103, 104, 105, 107};
	unsigned int ch_rate[WSA881X_MAX_SWR_PORTS] = {2400, 600, 300, 1200};
	unsigned int ch_mask[WSA881X_MAX_SWR_PORTS] = {0x1, 0xF, 0x3, 0x3};
	struct snd_soc_codec *codec = snd_soc_component_to_codec(component);
	struct msm_asoc_mach_data *pdata;
	struct snd_soc_dapm_context *dapm;
	int ret = 0;

	if (!codec) {
		pr_err("%s codec is NULL\n", __func__);
		return -EINVAL;
	}

	dapm = snd_soc_codec_get_dapm(codec);

	if (!strcmp(component->name_prefix, "SpkrLeft")) {
		dev_dbg(codec->dev, "%s: setting left ch map to codec %s\n",
			__func__, codec->component.name);
		wsa881x_set_channel_map(codec, &spkleft_ports[0],
				WSA881X_MAX_SWR_PORTS, &ch_mask[0],
				&ch_rate[0]);
		if (dapm->component) {
			snd_soc_dapm_ignore_suspend(dapm, "SpkrLeft IN");
			snd_soc_dapm_ignore_suspend(dapm, "SpkrLeft SPKR");
		}
	} else if (!strcmp(component->name_prefix, "SpkrRight")) {
		dev_dbg(codec->dev, "%s: setting right ch map to codec %s\n",
			__func__, codec->component.name);
		wsa881x_set_channel_map(codec, &spkright_ports[0],
				WSA881X_MAX_SWR_PORTS, &ch_mask[0],
				&ch_rate[0]);
		if (dapm->component) {
			snd_soc_dapm_ignore_suspend(dapm, "SpkrRight IN");
			snd_soc_dapm_ignore_suspend(dapm, "SpkrRight SPKR");
		}
	} else {
		dev_err(codec->dev, "%s: wrong codec name %s\n", __func__,
			codec->component.name);
		ret = -EINVAL;
		goto err;
	}
	pdata = snd_soc_card_get_drvdata(component->card);
	if (pdata && pdata->codec_root)
		wsa881x_codec_info_create_codec_entry(pdata->codec_root,
						      codec);

err:
	return ret;
}

static int msm_init_wsa_dev(struct platform_device *pdev,
				struct snd_soc_card *card)
{
	struct device_node *wsa_of_node;
	u32 wsa_max_devs;
	u32 wsa_dev_cnt;
	int i;
	struct msm_wsa881x_dev_info *wsa881x_dev_info;
	const char *wsa_auxdev_name_prefix[1];
	char *dev_name_str = NULL;
	int found = 0;
	int ret = 0;

	/* Get maximum WSA device count for this platform */
	ret = of_property_read_u32(pdev->dev.of_node,
				   "qcom,wsa-max-devs", &wsa_max_devs);
	if (ret) {
		dev_info(&pdev->dev,
			 "%s: wsa-max-devs property missing in DT %s, ret = %d\n",
			 __func__, pdev->dev.of_node->full_name, ret);
		card->num_aux_devs = 0;
		return 0;
	}
	if (wsa_max_devs == 0) {
		dev_warn(&pdev->dev,
			 "%s: Max WSA devices is 0 for this target?\n",
			 __func__);
		card->num_aux_devs = 0;
		return 0;
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
			goto err_free_dev_info;
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

	card->num_aux_devs = wsa_max_devs;
	card->num_configs = wsa_max_devs;

	/* Alloc array of AUX devs struct */
	msm_aux_dev = devm_kcalloc(&pdev->dev, card->num_aux_devs,
				       sizeof(struct snd_soc_aux_dev),
				       GFP_KERNEL);
	if (!msm_aux_dev) {
		ret = -ENOMEM;
		goto err_free_dev_info;
	}

	/* Alloc array of codec conf struct */
	msm_codec_conf = devm_kcalloc(&pdev->dev, card->num_aux_devs,
					  sizeof(struct snd_soc_codec_conf),
					  GFP_KERNEL);
	if (!msm_codec_conf) {
		ret = -ENOMEM;
		goto err_free_aux_dev;
	}

	for (i = 0; i < card->num_aux_devs; i++) {
		dev_name_str = devm_kzalloc(&pdev->dev, DEV_NAME_STR_LEN,
					    GFP_KERNEL);
		if (!dev_name_str) {
			ret = -ENOMEM;
			goto err_free_cdc_conf;
		}

		ret = of_property_read_string_index(pdev->dev.of_node,
						    "qcom,wsa-aux-dev-prefix",
						    wsa881x_dev_info[i].index,
						    wsa_auxdev_name_prefix);
		if (ret) {
			dev_err(&pdev->dev,
				"%s: failed to read wsa aux dev prefix, ret = %d\n",
				__func__, ret);
			ret = -EINVAL;
			goto err_free_dev_name_str;
		}

		snprintf(dev_name_str, strlen("wsa881x.%d"), "wsa881x.%d", i);
		msm_aux_dev[i].name = dev_name_str;
		msm_aux_dev[i].codec_name = NULL;
		msm_aux_dev[i].codec_of_node =
					wsa881x_dev_info[i].of_node;
		msm_aux_dev[i].init = msm_wsa881x_init;
		msm_codec_conf[i].dev_name = NULL;
		msm_codec_conf[i].name_prefix = wsa_auxdev_name_prefix[0];
		msm_codec_conf[i].of_node =
				wsa881x_dev_info[i].of_node;
	}
	card->codec_conf = msm_codec_conf;
	card->aux_dev = msm_aux_dev;

	return 0;

err_free_dev_name_str:
	devm_kfree(&pdev->dev, dev_name_str);
err_free_cdc_conf:
	devm_kfree(&pdev->dev, msm_codec_conf);
err_free_aux_dev:
	devm_kfree(&pdev->dev, msm_aux_dev);
err_free_dev_info:
	devm_kfree(&pdev->dev, wsa881x_dev_info);
err:
	return ret;
}

static int msm_asoc_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card;
	struct msm_asoc_mach_data *pdata;
	const struct of_device_id *match;
	int ret;

	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "No platform supplied from device tree\n");
		return -EINVAL;
	}

	pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct msm_asoc_mach_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

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
		dev_err(&pdev->dev, "parse card name failed, err:%d\n",
			ret);
		goto err;
	}

	match = of_match_node(sdm855_asoc_machine_of_match,
			pdev->dev.of_node);
	if (!match) {
		dev_err(&pdev->dev, "%s: no matched codec is found.\n",
			__func__);
		goto err;
	}

	ret = msm_populate_dai_link_component_of_node(card);
	if (ret) {
		ret = -EPROBE_DEFER;
		goto err;
	}
	ret = msm_init_wsa_dev(pdev, card);
	if (ret)
		goto err;

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret) {
		dev_err(&pdev->dev, "%s: snd_soc_register_card failed (%d)\n",
			__func__, ret);
		goto err;
	}
	dev_info(&pdev->dev, "%s: Sound card %s registered\n",
		 __func__, card->name);
	spdev = pdev;

	ret = audio_notifier_register("sdm855", AUDIO_NOTIFIER_ADSP_DOMAIN,
				      &service_nb);
	if (ret < 0)
		pr_err("%s: Audio notifier register failed ret = %d\n",
			__func__, ret);

	return 0;
err:
	devm_kfree(&pdev->dev, pdata);
	return ret;
}

static int msm_asoc_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);
	audio_notifier_deregister("sdm855");
	return 0;
}

static struct platform_driver sdm855_asoc_machine_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = sdm855_asoc_machine_of_match,
	},
	.probe = msm_asoc_machine_probe,
	.remove = msm_asoc_machine_remove,
};
module_platform_driver(sdm855_asoc_machine_driver);

MODULE_DESCRIPTION("ALSA SoC msm");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, sdm855_asoc_machine_of_match);
