// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017, 2020 The Linux Foundation. All rights reserved.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include "platform_init.h"

static int __init audio_platform_init(void)
{
	msm_dai_q6_hdmi_init();
	msm_dai_q6_init();
	msm_dai_slim_init();

	return 0;
}

static void audio_platform_exit(void)
{
	msm_dai_slim_exit();
	msm_dai_q6_exit();
	msm_dai_q6_hdmi_exit();
}

module_init(audio_platform_init);
module_exit(audio_platform_exit);

MODULE_DESCRIPTION("Audio Platform driver");
MODULE_LICENSE("GPL v2");
