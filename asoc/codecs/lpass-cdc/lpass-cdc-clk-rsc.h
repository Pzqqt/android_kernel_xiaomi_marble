/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 */

#ifndef LPASS_CDC_CLK_RSC_H
#define LPASS_CDC_CLK_RSC_H

#include <linux/regmap.h>
#include <bindings/qcom,lpass-cdc-clk-rsc.h>

#if IS_ENABLED(CONFIG_SND_SOC_LPASS_CDC)
int lpass_cdc_clk_rsc_mgr_init(void);
void lpass_cdc_clk_rsc_mgr_exit(void);
void lpass_cdc_clk_rsc_fs_gen_request(struct device *dev,
						bool enable);
int lpass_cdc_clk_rsc_request_clock(struct device *dev,
				int default_clk_id,
				int clk_id_req,
				bool enable);
int lpass_cdc_rsc_clk_reset(struct device *dev, int clk_id);
void lpass_cdc_clk_rsc_enable_all_clocks(struct device *dev, bool enable);
#else
static inline void lpass_cdc_clk_rsc_fs_gen_request(struct device *dev,
						bool enable)
{
}
static inline int lpass_cdc_clk_rsc_mgr_init(void)
{
	return 0;
}
static inline void lpass_cdc_clk_rsc_mgr_exit(void)
{
}
static inline int lpass_cdc_clk_rsc_request_clock(struct device *dev,
				int default_clk_id,
				int clk_id_req,
				bool enable)
{
	return 0;
}
static inline int lpass_cdc_rsc_clk_reset(struct device *dev, int clk_id)
{
	return 0;
}
static inline void lpass_cdc_clk_rsc_enable_all_clocks(struct device *dev,
						    bool enable)
{
	return;
}
#endif /* CONFIG_SND_SOC_LPASS_CDC */
#endif /* LPASS_CDC_CLK_RSC_H */
