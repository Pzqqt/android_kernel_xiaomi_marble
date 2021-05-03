/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 */

#ifndef LPASS_CDC_H
#define LPASS_CDC_H

#include <sound/soc.h>
#include <linux/regmap.h>

#define LPASS_CDC_VERSION_1_0 0x0001
#define LPASS_CDC_VERSION_1_1 0x0002
#define LPASS_CDC_VERSION_1_2 0x0003
#define LPASS_CDC_VERSION_2_0 0x0004
#define LPASS_CDC_VERSION_2_1 0x0005
#define LPASS_CDC_VERSION_2_5 0x0006

enum {
	START_MACRO,
	TX_MACRO = START_MACRO,
	RX_MACRO,
	WSA_MACRO,
	VA_MACRO,
	WSA2_MACRO,
	MAX_MACRO
};

enum mclk_mux {
	MCLK_MUX0,
	MCLK_MUX1,
	MCLK_MUX_MAX
};

enum {
	LPASS_CDC_ADC0 = 1,
	LPASS_CDC_ADC1,
	LPASS_CDC_ADC2,
	LPASS_CDC_ADC3,
	LPASS_CDC_ADC_MAX
};

enum {
	LPASS_CDC_MACRO_EVT_RX_MUTE = 1, /* for RX mute/unmute */
	LPASS_CDC_MACRO_EVT_IMPED_TRUE, /* for imped true */
	LPASS_CDC_MACRO_EVT_IMPED_FALSE, /* for imped false */
	LPASS_CDC_MACRO_EVT_SSR_DOWN,
	LPASS_CDC_MACRO_EVT_SSR_UP,
	LPASS_CDC_MACRO_EVT_WAIT_VA_CLK_RESET,
	LPASS_CDC_MACRO_EVT_CLK_RESET,
	LPASS_CDC_MACRO_EVT_REG_WAKE_IRQ,
	LPASS_CDC_MACRO_EVT_RX_COMPANDER_SOFT_RST,
	LPASS_CDC_MACRO_EVT_BCS_CLK_OFF,
	LPASS_CDC_MACRO_EVT_SSR_GFMUX_UP,
	LPASS_CDC_MACRO_EVT_PRE_SSR_UP,
	LPASS_CDC_MACRO_EVT_RX_PA_GAIN_UPDATE,
	LPASS_CDC_MACRO_EVT_HPHL_HD2_ENABLE, /* Enable HD2 cfg for HPHL */
	LPASS_CDC_MACRO_EVT_HPHR_HD2_ENABLE, /* Enable HD2 cfg for HPHR */
};

enum {
	DMIC_TX = 0,
	DMIC_VA = 1,

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
	int (*clk_div_get)(struct snd_soc_component *component);
	int (*reg_evt_listener)(struct snd_soc_component *component, bool en);
	int (*clk_enable)(struct snd_soc_component *c, bool en);
	char __iomem *io_base;
	u16 clk_id_req;
	u16 default_clk_id;
};

typedef int (*rsc_clk_cb_t)(struct device *dev, u16 event);

#if IS_ENABLED(CONFIG_SND_SOC_LPASS_CDC)
int lpass_cdc_register_res_clk(struct device *dev, rsc_clk_cb_t cb);
void lpass_cdc_unregister_res_clk(struct device *dev);
bool lpass_cdc_is_va_macro_registered(struct device *dev);
int lpass_cdc_register_macro(struct device *dev, u16 macro_id,
			  struct macro_ops *ops);
void lpass_cdc_unregister_macro(struct device *dev, u16 macro_id);
struct device *lpass_cdc_get_device_ptr(struct device *dev, u16 macro_id);
struct device *lpass_cdc_get_rsc_clk_device_ptr(struct device *dev);
int lpass_cdc_info_create_codec_entry(
		struct snd_info_entry *codec_root,
		struct snd_soc_component *component);
int lpass_cdc_register_wake_irq(struct snd_soc_component *component, u32 data);
void lpass_cdc_clear_amic_tx_hold(struct device *dev, u16 adc_n);
int lpass_cdc_runtime_resume(struct device *dev);
int lpass_cdc_runtime_suspend(struct device *dev);
int lpass_cdc_set_port_map(struct snd_soc_component *component, u32 size, void *data);
int lpass_cdc_register_event_listener(struct snd_soc_component *component,
				   bool enable);
void lpass_cdc_wsa_pa_on(struct device *dev, bool adie_lb);
bool lpass_cdc_check_core_votes(struct device *dev);
int lpass_cdc_tx_mclk_enable(struct snd_soc_component *c, bool enable);
int lpass_cdc_get_version(struct device *dev);
int lpass_cdc_dmic_clk_enable(struct snd_soc_component *component,
			   u32 dmic, u32 tx_mode, bool enable);

/* RX MACRO utilities */
int lpass_cdc_rx_set_fir_capability(struct snd_soc_component *component,
					bool capable);
#else
static inline int lpass_cdc_register_res_clk(struct device *dev, rsc_clk_cb_t cb)
{
	return 0;
}
static inline void lpass_cdc_unregister_res_clk(struct device *dev)
{
}

static bool lpass_cdc_is_va_macro_registered(struct device *dev)
{
	return false;
}

static inline int lpass_cdc_register_macro(struct device *dev,
					u16 macro_id,
					struct macro_ops *ops)
{
	return 0;
}

static inline void lpass_cdc_unregister_macro(struct device *dev, u16 macro_id)
{
}

static inline struct device *lpass_cdc_get_device_ptr(struct device *dev,
						   u16 macro_id)
{
	return NULL;
}

static int lpass_cdc_info_create_codec_entry(
		struct snd_info_entry *codec_root,
		struct snd_soc_component *component)
{
	return 0;
}

static inline void lpass_cdc_clear_amic_tx_hold(struct device *dev, u16 adc_n)
{
}

static inline int lpass_cdc_register_wake_irq(struct snd_soc_component *component,
					   u32 data)
{
	return 0;
}

static inline int lpass_cdc_runtime_resume(struct device *dev)
{
	return 0;
}

static int lpass_cdc_runtime_suspend(struct device *dev)
{
	return 0;
}

static inline int lpass_cdc_set_port_map(struct snd_soc_component *component,
				u32 size, void *data)
{
	return 0;
}

static inline int lpass_cdc_register_event_listener(
					struct snd_soc_component *component,
					bool enable)
{
	return 0;
}

static void lpass_cdc_wsa_pa_on(struct device *dev, bool adie_lb)
{
}

static inline bool lpass_cdc_check_core_votes(struct device *dev)
{
	return false;
}

static int lpass_cdc_get_version(struct device *dev)
{
	return 0;
}

static int lpass_cdc_dmic_clk_enable(struct snd_soc_component *component,
			   u32 dmic, u32 tx_mode, bool enable)
{
	return 0;
}
static int lpass_cdc_tx_mclk_enable(struct snd_soc_component *c, bool enable)
{
	return 0;
}
/* RX MACRO utilities */
static int lpass_cdc_rx_set_fir_capability(struct snd_soc_component *component,
						bool capable)
{
	return 0;
}
#endif /* CONFIG_SND_SOC_LPASS_CDC */
#endif /* LPASS_CDC_H */
