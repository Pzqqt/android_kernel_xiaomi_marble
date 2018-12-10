/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2014-2015, 2017 The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_PCM_ROUTING_DEVDEP_H_
#define _MSM_PCM_ROUTING_DEVDEP_H_

#include <sound/soc.h>
#include "msm-pcm-routing-v2.h"

#ifdef CONFIG_SND_HWDEP
int msm_pcm_routing_hwdep_new(struct snd_soc_pcm_runtime *runtime,
			      struct msm_pcm_routing_bdai_data *msm_bedais);
void msm_pcm_routing_hwdep_free(struct snd_pcm *pcm);
#else
static inline int msm_pcm_routing_hwdep_new(struct snd_soc_pcm_runtime *runtime,
				struct msm_pcm_routing_bdai_data *msm_bedais)
{
	return 0;
}

static inline void msm_pcm_routing_hwdep_free(struct snd_pcm *pcm)
{
}
#endif
#endif
