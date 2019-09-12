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
#include <asoc/msm-cdc-pinctrl.h>
#include <dsp/gecko-core.h>
#include <dsp/msm_audio_ion.h>
#include "msm_common.h"

#define to_asoc_mach_common_pdata(kobj) \
	container_of((kobj), struct msm_common_pdata, aud_dev_kobj)

#define DEVICE_ENABLE 1
#define DEVICE_DISABLE 0

static struct attribute device_state_attr = {
	.name = "state",
	.mode = 0660,
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
	pr_info("%s: pcm_id %d state %d \n", __func__, pcm_id, state);

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
			gecko_core_apm_close_all();
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
	if (strnstr(stream_name, "PRIMARY", strlen("PRIMARY")))
		return PRI_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "SECONDARY", strlen("SECONDARY")))
		return SEC_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "TERTIARY", strlen("TERTIARY")))
		return TER_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "QUATERNARY", strlen("QUATERNARY")))
		return QUAT_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "QUINARY", strlen("QUINARY")))
		return QUIN_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "SENARY", strlen("SENARY")))
		return SEN_MI2S_TDM_AUXPCM;
	else
		return -EINVAL;
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
