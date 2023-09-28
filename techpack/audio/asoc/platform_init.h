/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2018, 2020 The Linux Foundation. All rights reserved.
 */

#ifndef __PLATFORM_INIT_H__
#define __PLATFORM_INIT_H__
int msm_dai_q6_hdmi_init(void);
int msm_dai_q6_init(void);

void msm_dai_q6_exit(void);
void msm_dai_q6_hdmi_exit(void);

#if IS_ENABLED(CONFIG_WCD9XXX_CODEC_CORE)
int msm_dai_slim_init(void);
void msm_dai_slim_exit(void);
#else
static inline int msm_dai_slim_init(void)
{
	return 0;
};
static inline void msm_dai_slim_exit(void)
{
};
#endif
#endif

