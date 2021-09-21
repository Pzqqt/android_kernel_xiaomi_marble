/* Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
#ifndef _MSM_COMMON_H_
#define _MSM_COMMON_H_

#include <sound/soc.h>
#include <sound/pcm.h>

enum {
	MI2S = 0,
	TDM,
	AUXPCM,
};

enum {
	PRI_MI2S_TDM_AUXPCM = 0,
	SEC_MI2S_TDM_AUXPCM,
	TER_MI2S_TDM_AUXPCM,
	QUAT_MI2S_TDM_AUXPCM,
	QUIN_MI2S_TDM_AUXPCM,
	SEN_MI2S_TDM_AUXPCM,
	MI2S_TDM_AUXPCM_MAX,
};

typedef enum snd_card_status_t {
	SND_CARD_STATUS_OFFLINE = 0,
	SND_CARD_STATUS_ONLINE  = 1,
} snd_card_status_t;

struct msm_common_pdata {
	uint8_t *aud_dev_state;
	struct kobject aud_dev_kobj;
	uint8_t dsp_sessions_closed;
	uint32_t num_aud_devs;
	struct device_node *mi2s_gpio_p[MI2S_TDM_AUXPCM_MAX];
	struct mutex lock[MI2S_TDM_AUXPCM_MAX];
	u32 tdm_max_slots; /* Max TDM slots used */
	atomic_t mi2s_gpio_ref_cnt[MI2S_TDM_AUXPCM_MAX];
	atomic_t lpass_intf_clk_ref_cnt[MI2S_TDM_AUXPCM_MAX];
	atomic_t lpass_audio_hw_vote_ref_cnt;
	struct clk *lpass_audio_hw_vote;
	uint32_t is_audio_hw_vote_required[MI2S_TDM_AUXPCM_MAX];
	uint32_t tdm_clk_attribute[MI2S_TDM_AUXPCM_MAX];
	uint32_t mi2s_clk_attribute[MI2S_TDM_AUXPCM_MAX];
};

int snd_card_notify_user(snd_card_status_t card_status);
int snd_card_set_card_status(snd_card_status_t card_status);
struct msm_common_pdata *msm_common_get_pdata(struct snd_soc_card *card);

void msm_common_set_pdata(struct snd_soc_card *card,
			  struct msm_common_pdata *pdata);

int snd_card_sysfs_init(void);

int msm_common_snd_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params);

int msm_common_snd_startup(struct snd_pcm_substream *substream);

void msm_common_snd_shutdown(struct snd_pcm_substream *substream);

int msm_common_snd_init(struct platform_device *pdev,
			struct snd_soc_card *card);

void msm_common_snd_deinit(struct msm_common_pdata *pdata);

int msm_common_dai_link_init(struct snd_soc_pcm_runtime *rtd);
#endif
