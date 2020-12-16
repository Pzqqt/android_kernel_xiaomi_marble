/* Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <sound/control.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <asoc/msm-cdc-pinctrl.h>
#include <dsp/spf-core.h>
#include <dsp/msm_audio_ion.h>
#include <sound/info.h>

#include "msm_common.h"

#define to_asoc_mach_common_pdata(kobj) \
	container_of((kobj), struct msm_common_pdata, aud_dev_kobj)

#define DEVICE_ENABLE 1
#define DEVICE_DISABLE 0

#define ARRAY_SZ 21

static struct attribute device_state_attr = {
	.name = "state",
	.mode = 0660,
};

#define MAX_PORT 20
#define CODEC_CHMAP "Channel Map"

enum backend_id {
	SLIM = 1,
	CODEC_DMA,
};

struct chmap_pdata {
	int id;
	struct snd_soc_dai *dai;
};

#define MAX_USR_INPUT 10

static ssize_t aud_dev_sysfs_store(struct kobject *kobj,
		struct attribute *attr,
		const char *buf, size_t count)
{
	ssize_t ret = -EINVAL;
	struct msm_common_pdata *pdata = to_asoc_mach_common_pdata(kobj);
	uint32_t pcm_id, state = 0;

	if (count > MAX_USR_INPUT) {
		pr_err("%s: invalid string written", __func__);
		goto done;
	}

	sscanf(buf, "%d %d", &pcm_id, &state);

	if ((pcm_id > pdata->num_aud_devs) || (pcm_id < 0)) {
		pr_err("%s: invalid pcm id %d \n", __func__, pcm_id);
		goto done;
	}

	if ((state > DEVICE_ENABLE) || (state < DEVICE_DISABLE)) {
		pr_err("%s: invalid state %d \n", __func__, state);
		goto done;
	}
	pr_debug("%s: pcm_id %d state %d \n", __func__, pcm_id, state);

	pdata->aud_dev_state[pcm_id] = state;
	if ( state == DEVICE_ENABLE && (pdata->dsp_sessions_closed != 0))
		pdata->dsp_sessions_closed = 0;

	ret = count;
done:
	return ret;
}

static const struct sysfs_ops aud_dev_sysfs_ops = {
	.store = aud_dev_sysfs_store,
};

static struct kobj_type aud_dev_ktype = {
	.sysfs_ops = &aud_dev_sysfs_ops,
};

static int aud_dev_sysfs_init(struct msm_common_pdata *pdata)
{
	int ret = 0;
	char dir[10] = "aud_dev";

	ret = kobject_init_and_add(&pdata->aud_dev_kobj, &aud_dev_ktype,
		      kernel_kobj, dir);
	if (ret < 0) {
		 pr_err("%s: Failed to add kobject %s, err = %d\n",
			  __func__, dir, ret);
		 goto done;
	}

	ret = sysfs_create_file(&pdata->aud_dev_kobj, &device_state_attr);
	if (ret < 0) {
		pr_err("%s: Failed to add wdsp_boot sysfs entry to %s\n",
			__func__, dir);
		goto fail_create_file;
	}

	return ret;

fail_create_file:
	kobject_put(&pdata->aud_dev_kobj);
done:
	return ret;
}

static void check_userspace_service_state(struct snd_soc_pcm_runtime *rtd,
						struct msm_common_pdata *pdata)
{
	dev_info(rtd->card->dev,"%s: pcm_id %d state %d\n", __func__,
				rtd->num, pdata->aud_dev_state[rtd->num]);

	if (pdata->aud_dev_state[rtd->num] == DEVICE_ENABLE) {
		dev_info(rtd->card->dev, "%s userspace service crashed\n",
					__func__);
		if (pdata->dsp_sessions_closed == 0) {
			/*Issue close all graph cmd to DSP*/
			spf_core_apm_close_all();
			/*unmap all dma mapped buffers*/
			msm_audio_ion_crash_handler();
			pdata->dsp_sessions_closed = 1;
		}
		/*Reset the state as sysfs node wont be triggred*/
		pdata->aud_dev_state[rtd->num] = 0;
	}
}

static int get_intf_index(const char *stream_name)
{
	if (strnstr(stream_name, "PRIMARY", strlen(stream_name)))
		return PRI_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "SECONDARY", strlen(stream_name)))
		return SEC_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "TERTIARY", strlen(stream_name)))
		return TER_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "QUATERNARY", strlen(stream_name)))
		return QUAT_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "QUINARY", strlen(stream_name)))
		return QUIN_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "SENARY", strlen(stream_name)))
		return SEN_MI2S_TDM_AUXPCM;
	else {
		pr_debug("%s: stream name %s does not match\n", __func__, stream_name);
		return -EINVAL;
	}
}

int msm_common_snd_startup(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct msm_common_pdata *pdata = msm_common_get_pdata(card);
	const char *stream_name = rtd->dai_link->stream_name;
	int index = get_intf_index(stream_name);

	dev_dbg(rtd->card->dev,
		"%s: substream = %s  stream = %d\n",
		__func__, substream->name, substream->stream);

	if (!pdata) {
		dev_err(rtd->card->dev, "%s: pdata is NULL\n", __func__);
		return -EINVAL;
	}

	if (index >= 0) {
		mutex_lock(&pdata->lock[index]);
		if (pdata->mi2s_gpio_p[index]) {
			if (atomic_read(&(pdata->mi2s_gpio_ref_cnt[index])) == 0) {
				ret = msm_cdc_pinctrl_select_active_state(
						pdata->mi2s_gpio_p[index]);
				if (ret) {
				  pr_err("%s:pinctrl set actve fail with %d\n",
							__func__, ret);
					goto done;
				}
			}
			atomic_inc(&(pdata->mi2s_gpio_ref_cnt[index]));
		}
done:
		mutex_unlock(&pdata->lock[index]);
	}
	return ret;
}

void msm_common_snd_shutdown(struct snd_pcm_substream *substream)
{
	int ret;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct msm_common_pdata *pdata = msm_common_get_pdata(card);
	const char *stream_name = rtd->dai_link->stream_name;
	int index = get_intf_index(stream_name);

	pr_debug("%s(): substream = %s  stream = %d\n", __func__,
			substream->name, substream->stream);

	if (!pdata) {
		dev_err(card->dev, "%s: pdata is NULL\n", __func__);
		return;
	}

	check_userspace_service_state(rtd, pdata);

	if (index >= 0) {
		mutex_lock(&pdata->lock[index]);
		if (pdata->mi2s_gpio_p[index]) {
			atomic_dec(&pdata->mi2s_gpio_ref_cnt[index]);
			if (atomic_read(&pdata->mi2s_gpio_ref_cnt[index]) == 0) {
				ret = msm_cdc_pinctrl_select_active_state(
						pdata->mi2s_gpio_p[index]);
				if (ret)
					dev_err(card->dev,
					"%s: pinctrl set actv fail %d\n",
					__func__, ret);
			}
		}
		mutex_unlock(&pdata->lock[index]);
	}
}

int msm_common_snd_init(struct platform_device *pdev, struct snd_soc_card *card)
{
	struct msm_common_pdata *common_pdata = NULL;
	int count;

	common_pdata = kcalloc(1, sizeof(struct msm_common_pdata), GFP_KERNEL);
	if (!common_pdata)
		return -ENOMEM;

	for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
		mutex_init(&common_pdata->lock[count]);
		atomic_set(&common_pdata->mi2s_gpio_ref_cnt[count], 0);
	}

	common_pdata->mi2s_gpio_p[PRI_MI2S_TDM_AUXPCM] = of_parse_phandle(pdev->dev.of_node,
			"qcom,pri-mi2s-gpios", 0);
	common_pdata->mi2s_gpio_p[SEC_MI2S_TDM_AUXPCM] = of_parse_phandle(pdev->dev.of_node,
			"qcom,sec-mi2s-gpios", 0);
	common_pdata->mi2s_gpio_p[TER_MI2S_TDM_AUXPCM] = of_parse_phandle(pdev->dev.of_node,
			"qcom,tert-mi2s-gpios", 0);
	common_pdata->mi2s_gpio_p[QUAT_MI2S_TDM_AUXPCM] = of_parse_phandle(pdev->dev.of_node,
			"qcom,quat-mi2s-gpios", 0);
	common_pdata->mi2s_gpio_p[QUIN_MI2S_TDM_AUXPCM] = of_parse_phandle(pdev->dev.of_node,
			"qcom,quin-mi2s-gpios", 0);
	common_pdata->mi2s_gpio_p[SEN_MI2S_TDM_AUXPCM] = of_parse_phandle(pdev->dev.of_node,
			"qcom,sen-mi2s-gpios", 0);
	common_pdata->aud_dev_state = devm_kcalloc(&pdev->dev, card->num_links,
						sizeof(uint8_t), GFP_KERNEL);
	dev_info(&pdev->dev, "num_links %d \n", card->num_links);
	common_pdata->num_aud_devs = card->num_links;

	aud_dev_sysfs_init(common_pdata);

	msm_common_set_pdata(card, common_pdata);
	return 0;
};

void msm_common_snd_deinit(struct msm_common_pdata *common_pdata)
{
	int count;

	if (!common_pdata)
		return;

	for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
		mutex_destroy(&common_pdata->lock[count]);
	}
}

int msm_channel_map_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BYTES;
	uinfo->count = sizeof(uint32_t) * MAX_PORT;

	return 0;
}

int msm_channel_map_get(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct chmap_pdata *kctl_pdata =
			(struct chmap_pdata *)kcontrol->private_data;
	struct snd_soc_dai *codec_dai = kctl_pdata->dai;
	int backend_id = kctl_pdata->id;
	uint32_t rx_ch[MAX_PORT], tx_ch[MAX_PORT];
	uint32_t rx_ch_cnt = 0, tx_ch_cnt = 0;
	uint32_t *chmap_data = NULL;
	int ret = 0, len = 0;

	if (kctl_pdata == NULL) {
		pr_debug("%s: chmap_pdata is not initialized\n", __func__);
		return -EINVAL;
	}

	ret = snd_soc_dai_get_channel_map(codec_dai,
			&tx_ch_cnt, tx_ch, &rx_ch_cnt, rx_ch);
	if (ret || (tx_ch_cnt == 0 && rx_ch_cnt == 0)) {
		pr_err("%s: get channel map failed for %d\n",
				__func__, backend_id);
		return ret;
	}
	switch (backend_id) {
	case SLIM: {
		uint32_t *chmap;
		uint32_t ch_cnt, i;

		if (rx_ch_cnt) {
			chmap = rx_ch;
			ch_cnt = rx_ch_cnt;
		} else {
			chmap = tx_ch;
			ch_cnt = tx_ch_cnt;
		}
		len = sizeof(uint32_t) * (ch_cnt + 1);
		chmap_data = kzalloc(len, GFP_KERNEL);
		if (!chmap_data)
			return -ENOMEM;

		chmap_data[0] = ch_cnt;
		for (i = 0; i < ch_cnt; i++)
			chmap_data[i+1] = chmap[i];

		memcpy(ucontrol->value.bytes.data, chmap_data, len);
		break;
	}
	case CODEC_DMA: {
		chmap_data = kzalloc(sizeof(uint32_t) * 2, GFP_KERNEL);
		if (!chmap_data)
			return -ENOMEM;

		if (rx_ch_cnt) {
			chmap_data[0] = rx_ch_cnt;
			chmap_data[1] = rx_ch[0];
		} else {
			chmap_data[0] = tx_ch_cnt;
			chmap_data[1] = tx_ch[0];
		}
		memcpy(ucontrol->value.bytes.data, chmap_data,
					sizeof(uint32_t) * 2);
		break;
	}
	default:
		pr_err("%s, Invalid backend %d\n", __func__, backend_id);
		ret = -EINVAL;
		break;
	}
	kfree(chmap_data);

	return ret;
}

void msm_common_get_backend_name(const char *stream_name, char **backend_name)
{
	char arg[ARRAY_SZ] = {0};
	char value[61] = {0};

	sscanf(stream_name, "%20[^-]-%60s", arg, value);
	*backend_name = kzalloc(ARRAY_SZ, GFP_KERNEL);
	if (!(*backend_name))
		return;

	strlcpy(*backend_name, arg, ARRAY_SZ);
}

int msm_common_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_component *component = NULL;
	struct snd_soc_dai_link *dai_link = rtd->dai_link;
	struct device *dev = rtd->card->dev;

	int ret = 0;
	const char *mixer_ctl_name = CODEC_CHMAP;
	char *mixer_str = NULL;
	char *backend_name = NULL;
	uint32_t ctl_len = 0;
	struct chmap_pdata *pdata;
	struct snd_kcontrol *kctl;
	struct snd_kcontrol_new msm_common_channel_map[1] = {
		{
			.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
			.name = "?",
			.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
			.info = msm_channel_map_info,
			.get = msm_channel_map_get,
			.private_value = 0,
		}
	};

	if (!codec_dai) {
		pr_err("%s: failed to get codec dai", __func__);
		return -EINVAL;
	}
	component = codec_dai->component;

	msm_common_get_backend_name(dai_link->stream_name, &backend_name);
	if (!backend_name) {
		pr_err("%s: failed to get backend name", __func__);
		return -EINVAL;
	}

	pdata = devm_kzalloc(dev, sizeof(struct chmap_pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	if ((!strncmp(backend_name, "SLIM", strlen("SLIM"))) ||
		(!strncmp(backend_name, "CODEC_DMA", strlen("CODEC_DMA")))) {
		ctl_len = strlen(dai_link->stream_name) + 1 +
				strlen(mixer_ctl_name) + 1;
		mixer_str = kzalloc(ctl_len, GFP_KERNEL);
		if (!mixer_str)
			return -ENOMEM;

		snprintf(mixer_str, ctl_len, "%s %s", dai_link->stream_name,
				mixer_ctl_name);
		msm_common_channel_map[0].name = mixer_str;
		msm_common_channel_map[0].private_value = 0;
		pr_debug("Registering new mixer ctl %s\n", mixer_str);
		ret = snd_soc_add_component_controls(component,
				msm_common_channel_map,
				ARRAY_SIZE(msm_common_channel_map));
		kctl = snd_soc_card_get_kcontrol(rtd->card, mixer_str);
		if (!kctl) {
			pr_err("failed to get kctl %s\n", mixer_str);
			ret = -EINVAL;
			goto free_mixer_str;
		}
		if (!strncmp(backend_name, "SLIM", strlen("SLIM")))
			pdata->id = SLIM;
		else
			pdata->id = CODEC_DMA;

		pdata->dai = codec_dai;
		kctl->private_data = pdata;
free_mixer_str:
		kfree(backend_name);
		kfree(mixer_str);
	}

	return ret;
}
