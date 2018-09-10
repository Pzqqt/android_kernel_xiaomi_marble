/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
*/

#ifndef __Q6_INIT_H__
#define __Q6_INIT_H__
int adsp_err_init(void);
int adm_init(void);
int afe_init(void);
int q6asm_init(void);
int q6lsm_init(void);
int voice_init(void);
int audio_cal_init(void);
int core_init(void);
int rtac_init(void);
int msm_audio_ion_init(void);
int audio_slimslave_init(void);
int avtimer_init(void);
#ifdef CONFIG_MSM_MDF
int msm_mdf_init(void);
void msm_mdf_exit(void);
#else
static inline int msm_mdf_init(void)
{
	return 0;
}

static inline void msm_mdf_exit(void)
{
	return;
}
#endif

void avtimer_exit(void);
void audio_slimslave_exit(void);
void msm_audio_ion_exit(void);
void rtac_exit(void);
void core_exit(void);
void audio_cal_exit(void);
void voice_exit(void);
void q6lsm_exit(void);
void q6asm_exit(void);
void afe_exit(void);
void adm_exit(void);
void adsp_err_exit(void);
#endif

