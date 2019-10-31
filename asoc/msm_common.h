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

struct msm_common_pdata {
	struct device_node *mi2s_gpio_p[MI2S_TDM_AUXPCM_MAX];
	struct mutex lock[MI2S_TDM_AUXPCM_MAX];
	atomic_t mi2s_gpio_ref_cnt[MI2S_TDM_AUXPCM_MAX];
};

struct msm_common_pdata *msm_common_get_pdata(struct snd_soc_card *card);

void msm_common_set_pdata(struct snd_soc_card *card,
			  struct msm_common_pdata *pdata);

int msm_common_snd_startup(struct snd_pcm_substream *substream);

void msm_common_snd_shutdown(struct snd_pcm_substream *substream);

int msm_common_snd_init(struct platform_device *pdev,
			struct snd_soc_card *card);

void msm_common_snd_deinit(struct msm_common_pdata *pdata);
#endif
