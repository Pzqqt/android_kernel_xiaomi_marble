/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
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

enum {
	BOLERO_ADC0 = 1,
	BOLERO_ADC1,
	BOLERO_ADC2,
	BOLERO_ADC3,
	BOLERO_ADC_MAX
};

enum {
	BOLERO_MACRO_EVT_RX_MUTE = 1, /* for RX mute/unmute */
	BOLERO_MACRO_EVT_IMPED_TRUE, /* for imped true */
	BOLERO_MACRO_EVT_IMPED_FALSE, /* for imped false */
	BOLERO_MACRO_EVT_SSR_DOWN,
	BOLERO_MACRO_EVT_SSR_UP,
	BOLERO_MACRO_EVT_WAIT_VA_CLK_RESET,
	BOLERO_MACRO_EVT_CLK_RESET,
	BOLERO_MACRO_EVT_REG_WAKE_IRQ,
	BOLERO_MACRO_EVT_RX_COMPANDER_SOFT_RST
};

struct macro_ops {
	int (*init)(struct snd_soc_component *component);
	int (*exit)(struct snd_soc_component *component);
	u16 num_dais;
	struct device *dev;
	struct snd_soc_dai_driver *dai_ptr;
	int (*mclk_fn)(struct device *dev, bool enable);
	int (*event_handler)(struct snd_soc_component *component, u16 event,
			     u32 data);
	int (*reg_wake_irq)(struct snd_soc_component *component, u32 data);
	int (*set_port_map)(struct snd_soc_component *component, u32 uc,
			    u32 size, void *data);
	int (*clk_switch)(struct snd_soc_component *component);
	char __iomem *io_base;
	u16 clk_id_req;
	u16 default_clk_id;
};

typedef int (*rsc_clk_cb_t)(struct device *dev, u16 event);

#if IS_ENABLED(CONFIG_SND_SOC_BOLERO)
int bolero_register_res_clk(struct device *dev, rsc_clk_cb_t cb);
void bolero_unregister_res_clk(struct device *dev);
int bolero_register_macro(struct device *dev, u16 macro_id,
			  struct macro_ops *ops);
void bolero_unregister_macro(struct device *dev, u16 macro_id);
struct device *bolero_get_device_ptr(struct device *dev, u16 macro_id);
struct device *bolero_get_rsc_clk_device_ptr(struct device *dev);
int bolero_info_create_codec_entry(
		struct snd_info_entry *codec_root,
		struct snd_soc_component *component);
int bolero_register_wake_irq(struct snd_soc_component *component, u32 data);
void bolero_clear_amic_tx_hold(struct device *dev, u16 adc_n);
int bolero_runtime_resume(struct device *dev);
int bolero_runtime_suspend(struct device *dev);
int bolero_set_port_map(struct snd_soc_component *component, u32 size, void *data);
int bolero_tx_clk_switch(struct snd_soc_component *component);
#else
static inline int bolero_register_res_clk(struct device *dev, rsc_clk_cb_t cb)
{
	return 0;
}
static inline void bolero_unregister_res_clk(struct device *dev)
{
}

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

static int bolero_info_create_codec_entry(
		struct snd_info_entry *codec_root,
		struct snd_soc_component *component)
{
	return 0;
}

static inline void bolero_clear_amic_tx_hold(struct device *dev, u16 adc_n)
{
}

static inline int bolero_register_wake_irq(struct snd_soc_component *component,
					   u32 data)
{
	return 0;
}

static inline int bolero_runtime_resume(struct device *dev)
{
	return 0;
}

static int bolero_runtime_suspend(struct device *dev)
{
	return 0;
}

static inline int bolero_set_port_map(struct snd_soc_component *component,
				u32 size, void *data)
{
	return 0;
}

static inline int bolero_tx_clk_switch(struct snd_soc_component *component)
{
	return 0;
}

#endif /* CONFIG_SND_SOC_BOLERO */
#endif /* BOLERO_CDC_H */
