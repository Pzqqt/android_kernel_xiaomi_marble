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
#include "asoc/msm-cdc-pinctrl.h"
#include "msm_common.h"

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

	if (index < 0) {
		dev_err(rtd->card->dev,
			"%s: Invalid Backend interface %d\n", __func__, index);
		return -EINVAL;
	}

	mutex_lock(&pdata->lock[index]);
	if (pdata->mi2s_gpio_p[index]) {
		if (atomic_read(&(pdata->mi2s_gpio_ref_cnt[index])) == 0) {
			ret = msm_cdc_pinctrl_select_active_state(
					pdata->mi2s_gpio_p[index]);
			if (ret) {
				pr_err("%s: GPIO pinctrl set active failed with %d\n",
						__func__, ret);
				goto done;
			}
		}
		atomic_inc(&(pdata->mi2s_gpio_ref_cnt[index]));
	}
done:
	mutex_unlock(&pdata->lock[index]);
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

	if (index < 0) {
		dev_err(card->dev,
			"%s: Invalid Backend interface %d\n", __func__, index);
		return;
	}

	mutex_lock(&pdata->lock[index]);
	if (pdata->mi2s_gpio_p[index]) {
		atomic_dec(&pdata->mi2s_gpio_ref_cnt[index]);
		if (atomic_read(&pdata->mi2s_gpio_ref_cnt[index]) == 0) {
			ret = msm_cdc_pinctrl_select_active_state(
					pdata->mi2s_gpio_p[index]);
			if (ret)
				dev_err(card->dev, "%s: GPIO pinctrl set active failed with %d\n",
						__func__, ret);
		}
	}
	mutex_unlock(&pdata->lock[index]);
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
