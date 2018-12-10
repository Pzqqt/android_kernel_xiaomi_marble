/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 */

#ifndef _WCD937X_H
#define _WCD937X_H

#ifdef CONFIG_SND_SOC_WCD937X
extern int wcd937x_info_create_codec_entry(struct snd_info_entry *codec_root,
				    struct snd_soc_component *component);
#else
extern int wcd937x_info_create_codec_entry(struct snd_info_entry *codec_root,
				    struct snd_soc_component *component)
{
	return 0;
}
#endif /* CONFIG_SND_SOC_WCD937X */

#endif
