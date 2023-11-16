// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/arch_topology.h>
#include <sound/control.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <asoc/msm-cdc-pinctrl.h>
#include <dsp/spf-core.h>
#include <dsp/msm_audio_ion.h>
#include <sound/info.h>
#include <dsp/audio_prm.h>
#include <dsp/digital-cdc-rsc-mgr.h>

#include "msm_common.h"

#ifndef topology_cluster_id
#define topology_cluster_id(cpu) topology_physical_package_id(cpu)
#endif

struct snd_card_pdata {
	struct kobject snd_card_kobj;
	int card_status;
}*snd_card_pdata;

#define to_asoc_mach_common_pdata(kobj) \
	container_of((kobj), struct msm_common_pdata, aud_dev_kobj)

#define DEVICE_ENABLE 1
#define DEVICE_DISABLE 0

#define ARRAY_SZ 21
#define BUF_SZ 32
#define DIR_SZ 10

#define MAX_CODEC_DAI 8
#define TDM_SLOT_WIDTH_BITS 32
#define TDM_MAX_SLOTS 8
#define MI2S_NUM_CHANNELS 2

#define SAMPLING_RATE_44P1KHZ   44100
#define SAMPLING_RATE_88P2KHZ   88200
#define SAMPLING_RATE_176P4KHZ  176400
#define SAMPLING_RATE_352P8KHZ  352800

static struct attribute device_state_attr = {
	.name = "state",
	.mode = 0660,
};

static struct attribute card_state_attr = {
	.name = "card_state",
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
	uint32_t num_codec_dai;
	struct snd_soc_dai *dai[MAX_CODEC_DAI];
};

#define MAX_USR_INPUT 10
#define MAX_CPU_CLUSTER 3 /*Silver, Gold, Prime*/

static int qos_vote_status;
static struct dev_pm_qos_request latency_pm_qos_req; /* pm_qos request */
static unsigned int qos_client_active_cnt;
static int cluster_first_cpu[MAX_CPU_CLUSTER] = {-1, };
static struct dev_pm_qos_request *msm_audio_req = NULL;
static bool kregister_pm_qos_latency_controls = false;
#define MSM_LL_QOS_VALUE	300 /* time in us to ensure LPM doesn't go in C3/C4 */

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

int snd_card_notify_user(snd_card_status_t card_status)
{
	snd_card_pdata->card_status = card_status;
	sysfs_notify(&snd_card_pdata->snd_card_kobj, NULL, "card_state");
	return 0;
}

int snd_card_set_card_status(snd_card_status_t card_status)
{
	snd_card_pdata->card_status = card_status;
	return 0;
}

static ssize_t snd_card_sysfs_show(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	return snprintf(buf, BUF_SZ, "%d", snd_card_pdata->card_status);
}

static ssize_t snd_card_sysfs_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &snd_card_pdata->card_status);
	sysfs_notify(&snd_card_pdata->snd_card_kobj, NULL, "card_state");
	return 0;
}

static const struct sysfs_ops snd_card_sysfs_ops = {
	.show = snd_card_sysfs_show,
	.store = snd_card_sysfs_store,
};

static struct kobj_type snd_card_ktype = {
	.sysfs_ops = &snd_card_sysfs_ops,
};

int snd_card_sysfs_init(void)
{
	int ret = 0;
	char dir[DIR_SZ] = "snd_card";

	snd_card_pdata = kcalloc(1, sizeof(struct snd_card_pdata), GFP_KERNEL);
	ret = kobject_init_and_add(&snd_card_pdata->snd_card_kobj, &snd_card_ktype,
		kernel_kobj, dir);
	if (ret < 0) {
		pr_err("%s: Failed to add kobject %s, err = %d\n",
			__func__, dir, ret);
		goto done;
	}

	ret = sysfs_create_file(&snd_card_pdata->snd_card_kobj, &card_state_attr);
	if (ret < 0) {
		pr_err("%s: Failed to add snd_card sysfs entry to %s\n",
			__func__, dir);
		goto fail_create_file;
	}

	return ret;

fail_create_file:
	kobject_put(&snd_card_pdata->snd_card_kobj);
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

static int get_mi2s_tdm_auxpcm_intf_index(const char *stream_name)
{

	if (!strnstr(stream_name, "TDM", strlen(stream_name)) &&
	    !strnstr(stream_name, "MI2S", strlen(stream_name)) &&
	    !strnstr(stream_name, "AUXPCM", strlen(stream_name)))
		return -EINVAL;

	if (strnstr(stream_name, "LPAIF_RXTX", strlen(stream_name)))
		return QUAT_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "LPAIF_WSA", strlen(stream_name)))
		return SEN_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "LPAIF_VA", strlen(stream_name)))
		return QUIN_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "LPAIF_AUD", strlen(stream_name)))
		return SEC_MI2S_TDM_AUXPCM;
	else if (strnstr(stream_name, "LPAIF", strlen(stream_name))) {
		if (strnstr(stream_name, "PRIMARY", strlen(stream_name)))
			return PRI_MI2S_TDM_AUXPCM;
		else if (strnstr(stream_name, "TERTIARY", strlen(stream_name)))
			return TER_MI2S_TDM_AUXPCM;
	}
	pr_debug("%s: stream name %s does not match\n", __func__, stream_name);
	return -EINVAL;
}

static bool is_fractional_sample_rate(unsigned int sample_rate)
{
	switch (sample_rate) {
	case SAMPLING_RATE_44P1KHZ:
	case SAMPLING_RATE_88P2KHZ:
	case SAMPLING_RATE_176P4KHZ:
	case SAMPLING_RATE_352P8KHZ:
		return true;
	default:
		return false;
	}
	return false;
}

static int get_mi2s_clk_id(int index)
{
	int clk_id = -EINVAL;

	switch(index) {
	case PRI_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_PRI_MI2S_IBIT;
		break;
	case SEC_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_SEP_MI2S_IBIT;
		break;
	case TER_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_TER_MI2S_IBIT;
		break;
	case QUAT_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_QUAD_MI2S_IBIT;
		break;
	case QUIN_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_QUI_MI2S_IBIT;
		break;
	case SEN_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_SEN_MI2S_IBIT;
		break;
	default:
		pr_err("%s: Invalid interface index: %d\n", __func__, index);
	}
	pr_debug("%s: clk id: %d\n", __func__, clk_id);
	return clk_id;
}

static int get_tdm_clk_id(int index)
{
	int clk_id = -EINVAL;

	switch(index) {
	case PRI_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_PRI_TDM_IBIT;
		break;
	case SEC_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_SEP_TDM_IBIT;
		break;
	case TER_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_TER_TDM_IBIT;
		break;
	case QUAT_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_QUAD_TDM_IBIT;
		break;
	case QUIN_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_QUI_TDM_IBIT;
		break;
	case SEN_MI2S_TDM_AUXPCM:
		clk_id = CLOCK_ID_SEN_TDM_IBIT;
		break;
	default:
		pr_err("%s: Invalid interface index: %d\n", __func__, index);
	}
	pr_debug("%s: clk id: %d\n", __func__, clk_id);
	return clk_id;
}

int mi2s_tdm_hw_vote_req(struct msm_common_pdata *pdata, int enable)
{
	int ret = 0;

	if (!pdata || (pdata->lpass_audio_hw_vote == NULL)) {
		pr_err("%s: pdata or lpass audio hw vote node NULL", __func__);
		return -EINVAL;
	}

	pr_debug("%s: lpass audio hw vote for fractional sample rate enable: %d\n",
				__func__, enable);

	if (enable) {
		if (atomic_read(&pdata->lpass_audio_hw_vote_ref_cnt) == 0) {
			ret = digital_cdc_rsc_mgr_hw_vote_enable(pdata->lpass_audio_hw_vote, NULL);
			if (ret < 0) {
				pr_err("%s lpass audio hw vote enable failed %d\n",
					__func__, ret);
					return ret;
				}
			}
		atomic_inc(&pdata->lpass_audio_hw_vote_ref_cnt);
	} else {
		atomic_dec(&pdata->lpass_audio_hw_vote_ref_cnt);
		if (atomic_read(&pdata->lpass_audio_hw_vote_ref_cnt) == 0)
			digital_cdc_rsc_mgr_hw_vote_disable(pdata->lpass_audio_hw_vote, NULL);
		else if (atomic_read(&pdata->lpass_audio_hw_vote_ref_cnt) < 0)
			atomic_set(&pdata->lpass_audio_hw_vote_ref_cnt, 0);
	}
	return ret;
}

int msm_common_snd_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	int ret = 0;
	int slot_width = TDM_SLOT_WIDTH_BITS;
	int slots;
	int sample_width;
	unsigned int rate;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	const char *stream_name = rtd->dai_link->stream_name;
	struct snd_soc_card *card = rtd->card;
	struct msm_common_pdata *pdata = msm_common_get_pdata(card);
	int index = get_mi2s_tdm_auxpcm_intf_index(stream_name);
	struct clk_cfg intf_clk_cfg;

	dev_dbg(rtd->card->dev,
		"%s: substream = %s  stream = %d\n",
		__func__, substream->name, substream->stream);

	if (!pdata) {
		dev_err(rtd->card->dev, "%s: pdata is NULL\n", __func__);
		return -EINVAL;
	}

	if (index >= 0) {
		mutex_lock(&pdata->lock[index]);
		if (atomic_read(&pdata->lpass_intf_clk_ref_cnt[index]) == 0) {
			if ((strnstr(stream_name, "TDM", strlen(stream_name)))) {
				slots = pdata->tdm_max_slots;
				rate = params_rate(params);

				ret = get_tdm_clk_id(index);
				if ( ret < 0)
					goto done;

				intf_clk_cfg.clk_id = ret;
				intf_clk_cfg.clk_freq_in_hz = rate * slot_width * slots;
				intf_clk_cfg.clk_attri = pdata->tdm_clk_attribute[index];
				intf_clk_cfg.clk_root = 0;

				if (pdata->is_audio_hw_vote_required[index]  &&
					is_fractional_sample_rate(rate)) {
					ret = mi2s_tdm_hw_vote_req(pdata, 1);
					if (ret < 0) {
						pr_err("%s lpass audio hw vote enable failed %d\n",
							__func__, ret);
						goto done;
					}
				}
				pr_debug("%s: clk_id :%d clk freq %d\n", __func__,
					intf_clk_cfg.clk_id, intf_clk_cfg.clk_freq_in_hz);
				ret = audio_prm_set_lpass_clk_cfg(&intf_clk_cfg, 1);
				if (ret < 0) {
					pr_err("%s: prm lpass tdm clk cfg set failed ret %d\n",
						__func__, ret);
					goto done;
				}
			} else if ((strnstr(stream_name, "MI2S", strlen(stream_name)))) {

				ret =  get_mi2s_clk_id(index);
				if (ret < 0)
					goto done;

				intf_clk_cfg.clk_id =  ret;
				rate = params_rate(params);
				switch (params_format(params)) {
				case SNDRV_PCM_FORMAT_S24_LE:
				case SNDRV_PCM_FORMAT_S24_3LE:
				case SNDRV_PCM_FORMAT_S32_LE:
					sample_width = 32;
					break;
				case SNDRV_PCM_FORMAT_S16_LE:
				default:
					sample_width = 16;
					pr_debug("%s: bitwidth set to default : %d\n",
							__func__, sample_width);
				}

				intf_clk_cfg.clk_freq_in_hz = rate *
					MI2S_NUM_CHANNELS * sample_width;
				intf_clk_cfg.clk_attri = pdata->mi2s_clk_attribute[index];
				intf_clk_cfg.clk_root = CLOCK_ROOT_DEFAULT;

				if (pdata->is_audio_hw_vote_required[index]  &&
					is_fractional_sample_rate(rate)) {
					ret = mi2s_tdm_hw_vote_req(pdata, 1);
					if (ret < 0) {
						pr_err("%s lpass audio hw vote enable failed %d\n",
						__func__, ret);
						goto done;
					}
				}
				pr_debug("%s: mi2s clk_id :%d clk freq %d\n", __func__,
					intf_clk_cfg.clk_id, intf_clk_cfg.clk_freq_in_hz);
				ret = audio_prm_set_lpass_clk_cfg(&intf_clk_cfg, 1);
				if (ret < 0) {
					pr_err("%s: prm lpass mi2s clk cfg set failed ret %d\n",
						__func__, ret);
					goto done;
				}
			} else {
				pr_err("%s: unsupported stream name: %s\n",
					__func__, stream_name);
				goto done;
			}
		}
		atomic_inc(&pdata->lpass_intf_clk_ref_cnt[index]);
done:
		mutex_unlock(&pdata->lock[index]);
	}
	return ret;
}

int msm_common_snd_startup(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct msm_common_pdata *pdata = msm_common_get_pdata(card);
	const char *stream_name = rtd->dai_link->stream_name;
	int index = get_mi2s_tdm_auxpcm_intf_index(stream_name);

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
	struct snd_pcm_runtime *runtime = substream->runtime;
	const char *stream_name = rtd->dai_link->stream_name;
	int index = get_mi2s_tdm_auxpcm_intf_index(stream_name);
	struct clk_cfg intf_clk_cfg;
	unsigned int rate = runtime->rate;

	memset(&intf_clk_cfg, 0, sizeof(struct clk_cfg));
	pr_debug("%s(): substream = %s  stream = %d\n", __func__,
			substream->name, substream->stream);

	if (!pdata) {
		dev_err(card->dev, "%s: pdata is NULL\n", __func__);
		return;
	}

	check_userspace_service_state(rtd, pdata);

	if (index >= 0) {
		mutex_lock(&pdata->lock[index]);
		atomic_dec(&pdata->lpass_intf_clk_ref_cnt[index]);
		if (atomic_read(&pdata->lpass_intf_clk_ref_cnt[index]) == 0) {
			if ((strnstr(stream_name, "TDM", strlen(stream_name)))) {
				ret = get_tdm_clk_id(index);
				if (ret > 0) {
					intf_clk_cfg.clk_id = ret;
					ret = audio_prm_set_lpass_clk_cfg(&intf_clk_cfg, 0);
					if (ret < 0)
						pr_err("%s: prm tdm clk cfg set failed ret %d\n",
						__func__, ret);
				}
			} else if((strnstr(stream_name, "MI2S", strlen(stream_name)))) {
				ret = get_mi2s_clk_id(index);
				if (ret > 0) {
					intf_clk_cfg.clk_id = ret;
					ret = audio_prm_set_lpass_clk_cfg(&intf_clk_cfg, 0);
					if (ret < 0)
						pr_err("%s: prm mi2s clk cfg disable failed ret %d\n",
							__func__, ret);
				}
			} else {
				pr_err("%s: unsupported stream name: %s\n",
					__func__, stream_name);
			}

			if (pdata->is_audio_hw_vote_required[index]  &&
				is_fractional_sample_rate(rate)) {
				ret = mi2s_tdm_hw_vote_req(pdata, 0);
			}
		} else if (atomic_read(&pdata->lpass_intf_clk_ref_cnt[index]) < 0) {
			atomic_set(&pdata->lpass_intf_clk_ref_cnt[index], 0);
		}

		if (pdata->mi2s_gpio_p[index]) {
			atomic_dec(&pdata->mi2s_gpio_ref_cnt[index]);
			if (atomic_read(&pdata->mi2s_gpio_ref_cnt[index]) == 0)  {
				ret = msm_cdc_pinctrl_select_sleep_state(
					pdata->mi2s_gpio_p[index]);
				if (ret)
					dev_err(card->dev,
					"%s: pinctrl set actv fail %d\n",
					__func__, ret);
			} else if (atomic_read(&pdata->mi2s_gpio_ref_cnt[index]) < 0) {
				atomic_set(&pdata->mi2s_gpio_ref_cnt[index], 0);
			}
		}
		mutex_unlock(&pdata->lock[index]);
	}
}

static void msm_audio_add_qos_request()
{
	int num_req = 0;
	int cpu = 0;
	int ret = 0;
	int cid, prev_cid = -1;
	int cluster_num = 0;
	cpumask_t* cluster_cpu_mask = NULL;

	msm_audio_req = kcalloc(num_possible_cpus(),
			sizeof(struct dev_pm_qos_request), GFP_KERNEL);
	if (!msm_audio_req)
		return;

	for_each_cpu(cpu, cpu_possible_mask) {
		cid = topology_cluster_id(cpu);
		if(cid != prev_cid) {
			cluster_first_cpu[cluster_num++] = cpu;
			prev_cid = cid;
		}
	}

	/* Pick the first cluster as it represents the Silver cluster. */
	cluster_cpu_mask = topology_core_cpumask(cluster_first_cpu[0]);

	for_each_cpu(cpu, cluster_cpu_mask) {
		ret = dev_pm_qos_add_request(get_cpu_device(cpu),
				&msm_audio_req[cpu],
				DEV_PM_QOS_RESUME_LATENCY,
				PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
		if (ret < 0)
			pr_err("%s error (%d) adding resume latency to cpu %d.\n",
                                                __func__, ret, cpu);
		pr_debug("%s set cpu affinity to logical core %d.\n", __func__, cpu);

		/* Limit the request to 2 silver cpu cores. */
		if (++num_req == 2)
			break;
	}
}

static void msm_audio_remove_qos_request()
{
	int cpu = 0;
	int ret = 0;
	cpumask_t* cluster_cpu_mask = NULL;

	cluster_cpu_mask = topology_core_cpumask(cluster_first_cpu[0]);

	if (msm_audio_req) {
		for_each_cpu(cpu, cluster_cpu_mask) {
			ret = dev_pm_qos_remove_request(
					&msm_audio_req[cpu]);
			if (ret < 0)
				pr_err("%s error (%d) removing request from cpu %d.\n",
							__func__, ret, cpu);
			pr_debug("%s remove cpu affinity of core %d.\n", __func__, cpu);
		}
		kfree(msm_audio_req);
	}
}

int msm_common_snd_init(struct platform_device *pdev, struct snd_soc_card *card)
{
	struct msm_common_pdata *common_pdata = NULL;
	int count, ret = 0;
	uint32_t val_array[MI2S_TDM_AUXPCM_MAX] = {0};
	struct clk *lpass_audio_hw_vote = NULL;

	common_pdata = kcalloc(1, sizeof(struct msm_common_pdata), GFP_KERNEL);
	if (!common_pdata)
		return -ENOMEM;

	for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
		mutex_init(&common_pdata->lock[count]);
		atomic_set(&common_pdata->mi2s_gpio_ref_cnt[count], 0);
	}

	ret = of_property_read_u32(pdev->dev.of_node, "qcom,tdm-max-slots",
				&common_pdata->tdm_max_slots);
	if (ret) {
		dev_info(&pdev->dev, "%s: No DT match for tdm max slots\n",
			__func__);
	}
	if ((common_pdata->tdm_max_slots <= 0) || (common_pdata->tdm_max_slots >
			TDM_MAX_SLOTS)) {
		common_pdata->tdm_max_slots = TDM_MAX_SLOTS;
		dev_info(&pdev->dev, "%s: Using default tdm max slot: %d\n",
			__func__, common_pdata->tdm_max_slots);
	}

	/* Register LPASS audio hw vote */
	lpass_audio_hw_vote = devm_clk_get(&pdev->dev, "lpass_audio_hw_vote");
	if (IS_ERR(lpass_audio_hw_vote)) {
		ret = PTR_ERR(lpass_audio_hw_vote);
		dev_dbg(&pdev->dev, "%s: clk get %s failed %d\n",
			__func__, "lpass_audio_hw_vote", ret);
		lpass_audio_hw_vote = NULL;
		ret = 0;
	}
	common_pdata->lpass_audio_hw_vote = lpass_audio_hw_vote;

	ret = of_property_read_u32_array(pdev->dev.of_node,
				"qcom,mi2s-tdm-is-hw-vote-needed",
				val_array, MI2S_TDM_AUXPCM_MAX);
	if (ret) {
		dev_dbg(&pdev->dev, "%s:no qcom,mi2s-tdm-is-hw-vote-needed in DT node\n",
			__func__);
	} else {
		for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
			common_pdata->is_audio_hw_vote_required[count] =
					val_array[count];
		}
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "qcom,tdm-clk-attribute",
			val_array, MI2S_TDM_AUXPCM_MAX);
	if (ret) {
		dev_info(&pdev->dev,
			"%s: No DT match for tdm clk attribute, set to default\n", __func__);
		for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
			common_pdata->tdm_clk_attribute[count] =
				CLOCK_ATTRIBUTE_COUPLE_NO;
		}
	} else {
		for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
			common_pdata->tdm_clk_attribute[count] =
					val_array[count];
		}
	}

	ret = of_property_read_u32_array(pdev->dev.of_node, "qcom,mi2s-clk-attribute",
			val_array, MI2S_TDM_AUXPCM_MAX);
	if (ret) {
		dev_info(&pdev->dev,
			"%s: No DT match for mi2s clk attribute, set to default\n", __func__);
		for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
			common_pdata->mi2s_clk_attribute[count] =
				CLOCK_ATTRIBUTE_COUPLE_NO;
		}
	} else {
		for (count = 0; count < MI2S_TDM_AUXPCM_MAX; count++) {
			common_pdata->mi2s_clk_attribute[count] =
				val_array[count];
		}
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

    /* Add QoS request for audio tasks */
	msm_audio_add_qos_request();

	return 0;
};

void msm_common_snd_deinit(struct msm_common_pdata *common_pdata)
{
	int count;

	if (!common_pdata)
		return;

	msm_audio_remove_qos_request();

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
	struct snd_soc_dai *codec_dai = NULL;
	int backend_id = 0;
	uint32_t rx_ch[MAX_PORT] = {0}, tx_ch[MAX_PORT] = {0};
	uint32_t rx_ch_cnt = 0, tx_ch_cnt = 0;
	uint32_t *chmap_data = NULL;
	int ret = 0, len = 0, i = 0;

	if (kctl_pdata == NULL) {
		pr_debug("%s: chmap_pdata is not initialized\n", __func__);
		return -EINVAL;
	}

	codec_dai = kctl_pdata->dai[0];
	backend_id = kctl_pdata->id;

	switch (backend_id) {
	case SLIM: {
		uint32_t *chmap;
		uint32_t ch_cnt;

		ret = snd_soc_dai_get_channel_map(codec_dai,
				&tx_ch_cnt, tx_ch, &rx_ch_cnt, rx_ch);
		if (ret || (tx_ch_cnt == 0 && rx_ch_cnt == 0)) {
			pr_debug("%s: got incorrect channel map for backend_id:%d\n",
				 __func__, backend_id);
			return ret;
		}

		if (rx_ch_cnt) {
			chmap = rx_ch;
			ch_cnt = rx_ch_cnt;
		} else {
			chmap = tx_ch;
			ch_cnt = tx_ch_cnt;
		}
		if (ch_cnt > 2) {
			pr_err("%s: Incorrect channel count: %d\n", ch_cnt);
			return -EINVAL;
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
		uint32_t cur_rx_ch = 0, cur_tx_ch = 0;
		uint32_t cur_rx_ch_cnt = 0, cur_tx_ch_cnt = 0;

		for (i = 0; i < kctl_pdata->num_codec_dai; ++i) {
			codec_dai = kctl_pdata->dai[i];
			if(!codec_dai) {
				continue;
			}
			cur_rx_ch_cnt = 0;
			cur_tx_ch_cnt = 0;
			cur_tx_ch = 0;
			cur_rx_ch = 0;
			ret = snd_soc_dai_get_channel_map(codec_dai,
					&cur_tx_ch_cnt, &cur_tx_ch,
					&cur_rx_ch_cnt, &cur_rx_ch);

			/* DAIs that not supports get_channel_map should pass */
			if (ret && (ret != -ENOTSUPP)) {
				pr_err("%s: get channel map failed for backend_id:%d,"
					 " ret:%d\n",
					 __func__, backend_id, ret);
				return ret;
			}

			rx_ch_cnt += cur_rx_ch_cnt;
			tx_ch_cnt += cur_tx_ch_cnt;
			rx_ch[0] |= cur_rx_ch;
			tx_ch[0] |= cur_tx_ch;
		}

		/* reset return value from the loop above */
		ret = 0;
		if (rx_ch_cnt == 0 && tx_ch_cnt == 0) {
			pr_debug("%s: got incorrect channel map for backend_id:%d, ",
				"RX Channel Count:%d,"
				"TX Channel Count:%d\n",
				__func__, backend_id, rx_ch_cnt, tx_ch_cnt);
			return ret;
		}

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

static void msm_audio_update_qos_request(u32 latency)
{
	int cpu = 0;
	int ret = -1;
	int num_req = 0;
	cpumask_t* cluster_cpu_mask = NULL;

	cluster_cpu_mask = topology_core_cpumask(cluster_first_cpu[0]);

	if (msm_audio_req) {
		for_each_cpu(cpu, cluster_cpu_mask) {
			ret = dev_pm_qos_update_request(
					&msm_audio_req[cpu], latency);
			if (1 == ret ) {
				pr_debug("%s: updated latency of core %d to %u.\n",
								__func__, cpu, latency);
			} else if (0 == ret) {
				pr_debug("%s: latency of core %d not changed. latency %u.\n",
								__func__, cpu, latency);
			} else {
				pr_err("%s: failed to update latency of core %d, error %d \n",
								__func__, cpu, ret);
			}
			/* Limit the request to 2 Silver CPU cores. */
			if (++num_req == 2)
				break;
		}
	}
}

static int msm_qos_ctl_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	qos_vote_status = ucontrol->value.enumerated.item[0];
	if (qos_vote_status) {
		if (dev_pm_qos_request_active(&latency_pm_qos_req))
			dev_pm_qos_remove_request(&latency_pm_qos_req);

		qos_client_active_cnt++;
		if (qos_client_active_cnt == 1)
			msm_audio_update_qos_request(MSM_LL_QOS_VALUE);
	} else {
		if (qos_client_active_cnt > 0)
			qos_client_active_cnt--;
		if (qos_client_active_cnt == 0)
			msm_audio_update_qos_request(PM_QOS_CPU_LATENCY_DEFAULT_VALUE);
	}

	return 0;
}

static int msm_qos_ctl_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.enumerated.item[0] = qos_vote_status;
	return 0;
}

static const char *const qos_text[] = {"Disable", "Enable"};

static SOC_ENUM_SINGLE_EXT_DECL(qos_vote, qos_text);

static const struct snd_kcontrol_new card_pm_qos_controls[] = {
	SOC_ENUM_EXT("PM_QOS Vote", qos_vote,
			msm_qos_ctl_get, msm_qos_ctl_put),
};

static int register_pm_qos_latency_controls(struct snd_soc_pcm_runtime *rtd) {
	struct snd_soc_component *lpass_cdc_component = NULL;
	int ret = 0;
	lpass_cdc_component = snd_soc_rtdcom_lookup(rtd, "lpass-cdc");
	if (!lpass_cdc_component) {
		pr_err("%s: could not find component for lpass-cdc\n",
				__func__);
		return -EINVAL;
	}

	ret = snd_soc_add_component_controls(lpass_cdc_component,
			card_pm_qos_controls, ARRAY_SIZE(card_pm_qos_controls));
	if (ret < 0) {
		pr_err("%s: add common snd controls failed: %d\n",
				__func__, ret);
		return -EINVAL;
	}
	return 0;
}

int msm_common_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_component *component = NULL;
	struct snd_soc_dai_link *dai_link = rtd->dai_link;
	struct device *dev = rtd->card->dev;

	int ret = 0;
	int index = 0;
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
	if (!pdata) {
		ret = -ENOMEM;
		goto free_backend;
	}

	if ((!strncmp(backend_name, "SLIM", strlen("SLIM"))) ||
		(!strncmp(backend_name, "CODEC_DMA", strlen("CODEC_DMA")))) {
		ctl_len = strlen(dai_link->stream_name) + 1 +
				strlen(mixer_ctl_name) + 1;
		mixer_str = kzalloc(ctl_len, GFP_KERNEL);
		if (!mixer_str) {
			ret = -ENOMEM;
			goto free_backend;
		}

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

		pdata->dai[0] = codec_dai;
		pdata->num_codec_dai = 1;
		if (!strncmp(backend_name, "SLIM", strlen("SLIM"))) {
			pdata->id = SLIM;
		} else {
			pdata->id = CODEC_DMA;
			if (rtd->num_codecs <= MAX_CODEC_DAI) {
				pdata->num_codec_dai = rtd->num_codecs;
				for_each_rtd_codec_dais(rtd, index, codec_dai) {
					pdata->dai[index] = codec_dai;
				}
			}
		}
		kctl->private_data = pdata;
	}
	if (!kregister_pm_qos_latency_controls) {
		if (!register_pm_qos_latency_controls(rtd))
			kregister_pm_qos_latency_controls = true;
	}

free_mixer_str:
	if (mixer_str) {
		kfree(mixer_str);
		mixer_str = NULL;
	}

free_backend:
	if (backend_name) {
		kfree(backend_name);
		backend_name = NULL;
	}

	return ret;
}
