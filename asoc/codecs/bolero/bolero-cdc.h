/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#ifndef BOLERO_CDC_H
#define BOLERO_CDC_H

#include <sound/soc.h>
#include <linux/regmap.h>

enum {
	START_MACRO,
	TX_MACRO = START_MACRO,
	RX_MACRO,
	WSA_MACRO,
	VA_MACRO,
	MAX_MACRO
};

enum mclk_mux {
	MCLK_MUX0,
	MCLK_MUX1,
	MCLK_MUX_MAX
};

struct macro_ops {
	int (*init)(struct snd_soc_codec *codec);
	int (*exit)(struct snd_soc_codec *codec);
	u16 num_dais;
	struct device *dev;
	struct snd_soc_dai_driver *dai_ptr;
	int (*mclk_fn)(struct device *dev, bool enable);
	char __iomem *io_base;
};

#if IS_ENABLED(CONFIG_SND_SOC_BOLERO)
int bolero_register_macro(struct device *dev, u16 macro_id,
			  struct macro_ops *ops);
void bolero_unregister_macro(struct device *dev, u16 macro_id);
struct device *bolero_get_device_ptr(struct device *dev, u16 macro_id);
int bolero_request_clock(struct device *dev, u16 macro_id,
			 enum mclk_mux mclk_mux_id,
			 bool enable);
int bolero_info_create_codec_entry(
		struct snd_info_entry *codec_root,
		struct snd_soc_codec *codec);
#else
static inline int bolero_register_macro(struct device *dev,
					u16 macro_id,
					struct macro_ops *ops)
{
	return 0;
}

static inline void bolero_unregister_macro(struct device *dev, u16 macro_id)
{
}

static inline struct device *bolero_get_device_ptr(struct device *dev,
						   u16 macro_id)
{
	return NULL;
}

static inline int bolero_request_clock(struct device *dev, u16 macro_id,
				       enum mclk_mux mclk_mux_id,
				       bool enable)
{
	return 0;
}

static int bolero_info_create_codec_entry(
		struct snd_info_entry *codec_root,
		struct snd_soc_codec *codec)
{
	return 0;
}
#endif /* CONFIG_SND_SOC_BOLERO */
#endif /* BOLERO_CDC_H */
