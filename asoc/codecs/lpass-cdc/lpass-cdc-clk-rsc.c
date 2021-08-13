// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/of_platform.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include "lpass-cdc.h"
#include "lpass-cdc-clk-rsc.h"

#define DRV_NAME "lpass-cdc-clk-rsc"
#define LPASS_CDC_CLK_NAME_LENGTH 30

static char clk_src_name[MAX_CLK][LPASS_CDC_CLK_NAME_LENGTH] = {
	"tx_core_clk",
	"rx_core_clk",
	"wsa_core_clk",
	"va_core_clk",
	"wsa2_core_clk",
	"rx_tx_core_clk",
	"wsa_tx_core_clk",
	"wsa2_tx_core_clk",
};

struct lpass_cdc_clk_rsc {
	struct device *dev;
	struct mutex rsc_clk_lock;
	struct mutex fs_gen_lock;
	struct clk *clk[MAX_CLK];
	int clk_cnt[MAX_CLK];
	int reg_seq_en_cnt;
	int va_tx_clk_cnt;
	bool dev_up;
	bool dev_up_gfmux;
	u32 num_fs_reg;
	u32 *fs_gen_seq;
	int default_clk_id[MAX_CLK];
	struct regmap *regmap;
	char __iomem *rx_clk_muxsel;
	char __iomem *wsa_clk_muxsel;
	char __iomem *va_clk_muxsel;
};

static int lpass_cdc_clk_rsc_cb(struct device *dev, u16 event)
{
	struct lpass_cdc_clk_rsc *priv;

	if (!dev) {
		pr_err("%s: Invalid device pointer\n",
				__func__);
		return -EINVAL;
	}

	priv = dev_get_drvdata(dev);
	if (!priv) {
		pr_err("%s: Invalid clk rsc priviate data\n",
				__func__);
		return -EINVAL;
	}

	mutex_lock(&priv->rsc_clk_lock);
	if (event == LPASS_CDC_MACRO_EVT_SSR_UP) {
		priv->dev_up = true;
	} else if (event == LPASS_CDC_MACRO_EVT_SSR_DOWN) {
		priv->dev_up = false;
		priv->dev_up_gfmux = false;
	} else if (event == LPASS_CDC_MACRO_EVT_SSR_GFMUX_UP) {
		priv->dev_up_gfmux = true;
	}
	mutex_unlock(&priv->rsc_clk_lock);

	return 0;
}

static char __iomem *lpass_cdc_clk_rsc_get_clk_muxsel(struct lpass_cdc_clk_rsc *priv,
						 int clk_id)
{
	switch (clk_id) {
	case RX_CORE_CLK:
		return priv->rx_clk_muxsel;
	case WSA_CORE_CLK:
	case WSA2_CORE_CLK:
		return priv->wsa_clk_muxsel;
	case VA_CORE_CLK:
		return priv->va_clk_muxsel;
	case TX_CORE_CLK:
	case RX_TX_CORE_CLK:
	case WSA_TX_CORE_CLK:
	case WSA2_TX_CORE_CLK:
	default:
		dev_err_ratelimited(priv->dev, "%s: Invalid case\n", __func__);
		break;
	}

	return NULL;
}

int lpass_cdc_rsc_clk_reset(struct device *dev, int clk_id)
{
	struct device *clk_dev = NULL;
	struct lpass_cdc_clk_rsc *priv = NULL;
	int count = 0;

	if (!dev) {
		pr_err("%s: dev is null %d\n", __func__);
		return -EINVAL;
	}

	if (clk_id < 0 || clk_id >= MAX_CLK) {
		pr_err("%s: Invalid clk_id: %d\n",
			__func__, clk_id);
		return -EINVAL;
	}

	clk_dev = lpass_cdc_get_rsc_clk_device_ptr(dev->parent);
	if (!clk_dev) {
		pr_err("%s: Invalid rsc clk device\n", __func__);
		return -EINVAL;
	}

	priv = dev_get_drvdata(clk_dev);
	if (!priv) {
		pr_err("%s: Invalid rsc clk priviate data\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&priv->rsc_clk_lock);
	while (__clk_is_enabled(priv->clk[clk_id])) {
		clk_disable_unprepare(priv->clk[clk_id]);
		count++;
	}
	dev_dbg(priv->dev,
		"%s: clock reset after ssr, count %d\n", __func__, count);

	trace_printk("%s: clock reset after ssr, count %d\n", __func__, count);
	while (count--) {
		clk_prepare_enable(priv->clk[clk_id]);
	}
	mutex_unlock(&priv->rsc_clk_lock);
	return 0;
}
EXPORT_SYMBOL(lpass_cdc_rsc_clk_reset);

void lpass_cdc_clk_rsc_enable_all_clocks(struct device *dev, bool enable)
{
	struct device *clk_dev = NULL;
	struct lpass_cdc_clk_rsc *priv = NULL;
	int i = 0;

	if (!dev) {
		pr_err("%s: dev is null %d\n", __func__);
		return;
	}

	clk_dev = lpass_cdc_get_rsc_clk_device_ptr(dev->parent);
	if (!clk_dev) {
		pr_err("%s: Invalid rsc clk device\n", __func__);
		return;
	}

	priv = dev_get_drvdata(clk_dev);
	if (!priv) {
		pr_err("%s: Invalid rsc clk private data\n", __func__);
		return;
	}
	mutex_lock(&priv->rsc_clk_lock);
	for (i = 0; i < MAX_CLK; i++) {
		if (enable) {
			if (priv->clk[i])
				clk_prepare_enable(priv->clk[i]);
		} else {
			if (priv->clk[i] && __clk_is_enabled(priv->clk[i]))
				clk_disable_unprepare(priv->clk[i]);
		}
	}
	mutex_unlock(&priv->rsc_clk_lock);
	return;
}
EXPORT_SYMBOL(lpass_cdc_clk_rsc_enable_all_clocks);

static int lpass_cdc_clk_rsc_mux0_clk_request(struct lpass_cdc_clk_rsc *priv,
					   int clk_id,
					   bool enable)
{
	int ret = 0;

	if (enable) {
		/* Enable Requested Core clk */
		if (priv->clk_cnt[clk_id] == 0) {
			ret = clk_prepare_enable(priv->clk[clk_id]);
			if (ret < 0) {
				dev_err_ratelimited(priv->dev, "%s:clk_id %d enable failed\n",
							__func__, clk_id);
				goto done;
			}
		}
		priv->clk_cnt[clk_id]++;
	} else {
		if (priv->clk_cnt[clk_id] <= 0) {
			dev_err_ratelimited(priv->dev, "%s: clk_id: %d is already disabled\n",
					__func__, clk_id);
			priv->clk_cnt[clk_id] = 0;
			goto done;
		}
		priv->clk_cnt[clk_id]--;
		if (priv->clk_cnt[clk_id] == 0)
			clk_disable_unprepare(priv->clk[clk_id]);
	}
done:
	return ret;
}

static int lpass_cdc_clk_rsc_mux1_clk_request(struct lpass_cdc_clk_rsc *priv,
					   int clk_id,
					   bool enable)
{
	char __iomem *clk_muxsel = NULL;
	int ret = 0;
	int default_clk_id = priv->default_clk_id[clk_id];
	u32 muxsel = 0;

	clk_muxsel = lpass_cdc_clk_rsc_get_clk_muxsel(priv, clk_id);
	if (!clk_muxsel) {
		ret = -EINVAL;
		goto done;
	}

	if (enable) {
		if (priv->clk_cnt[clk_id] == 0) {
			if (clk_id != VA_CORE_CLK) {
				ret = lpass_cdc_clk_rsc_mux0_clk_request(priv,
							default_clk_id,
							true);
				if (ret < 0)
					goto done;
			}

			ret = clk_prepare_enable(priv->clk[clk_id]);
			if (ret < 0) {
				dev_err_ratelimited(priv->dev, "%s:clk_id %d enable failed\n",
					__func__, clk_id);
				goto err_clk;
			}
			/*
			 * Temp SW workaround to address a glitch issue of
			 * VA GFMux instance responsible for switching from
			 * TX MCLK to VA MCLK. This configuration would be taken
			 * care in DSP itself
			 */
			if (clk_id != VA_CORE_CLK) {
				if (priv->dev_up_gfmux) {
					iowrite32(0x1, clk_muxsel);
					muxsel = ioread32(clk_muxsel);
					trace_printk("%s: muxsel value after enable: %d\n",
							__func__, muxsel);
				}
				lpass_cdc_clk_rsc_mux0_clk_request(priv, default_clk_id,
							   false);
			}
		}
		priv->clk_cnt[clk_id]++;
	} else {
		if (priv->clk_cnt[clk_id] <= 0) {
			dev_err_ratelimited(priv->dev, "%s: clk_id: %d is already disabled\n",
				__func__, clk_id);
			priv->clk_cnt[clk_id] = 0;
			goto done;
		}
		priv->clk_cnt[clk_id]--;
		if (priv->clk_cnt[clk_id] == 0) {
			/*
			 * Temp SW workaround to address a glitch issue
			 * of VA GFMux instance responsible for
			 * switching from TX MCLK to VA MCLK.
			 * This configuration would be taken
			 * care in DSP itself.
			 */
			if (clk_id != VA_CORE_CLK) {
				ret = lpass_cdc_clk_rsc_mux0_clk_request(priv,
						default_clk_id, true);
				if (!ret && priv->dev_up_gfmux) {
					iowrite32(0x0, clk_muxsel);
					muxsel = ioread32(clk_muxsel);
					trace_printk("%s: muxsel value after disable: %d\n",
						__func__, muxsel);
				}
			}
			clk_disable_unprepare(priv->clk[clk_id]);
			if (clk_id != VA_CORE_CLK && !ret)
				lpass_cdc_clk_rsc_mux0_clk_request(priv,
						default_clk_id, false);
		}
	}
	return ret;

err_clk:
	if (clk_id != VA_CORE_CLK)
		lpass_cdc_clk_rsc_mux0_clk_request(priv, default_clk_id, false);
done:
	return ret;
}

static int lpass_cdc_clk_rsc_check_and_update_va_clk(struct lpass_cdc_clk_rsc *priv,
						  bool mux_switch,
						  int clk_id,
						  bool enable)
{
	int ret = 0;

	if (enable) {
		if (clk_id == VA_CORE_CLK && mux_switch) {
			/*
			 * Handle the following usecase scenarios during enable
			 * 1. VA only, Active clk is VA_CORE_CLK
			 * 2. record -> record + VA, Active clk is TX_CORE_CLK
			 */
			if (priv->clk_cnt[TX_CORE_CLK] == 0) {
				ret = lpass_cdc_clk_rsc_mux1_clk_request(priv,
							 VA_CORE_CLK, enable);
				if (ret < 0)
					goto err;
			} else {
				ret = lpass_cdc_clk_rsc_mux0_clk_request(priv,
							TX_CORE_CLK, enable);
				if (ret < 0)
					goto err;
				priv->va_tx_clk_cnt++;
			}
		} else if ((priv->clk_cnt[TX_CORE_CLK] > 0) &&
			   (priv->clk_cnt[VA_CORE_CLK] > 0)) {
			/*
			 * Handle following concurrency scenario during enable
			 * 1. VA-> Record+VA, Increment TX CLK and Disable VA
			 * 2. VA-> Playback+VA, Increment TX CLK and Disable VA
			 */
			while (priv->clk_cnt[VA_CORE_CLK] > 0) {
				ret = lpass_cdc_clk_rsc_mux0_clk_request(priv,
							TX_CORE_CLK, true);
				if (ret < 0)
					goto err;

				lpass_cdc_clk_rsc_mux1_clk_request(priv,
							VA_CORE_CLK, false);
				priv->va_tx_clk_cnt++;
			}
		}
	} else {
		if (clk_id == VA_CORE_CLK && mux_switch) {
			/*
			 * Handle the following usecase scenarios during disable
			 * 1. VA only, disable VA_CORE_CLK
			 * 2. Record + VA -> Record, decrement TX CLK count
			 */
			if (priv->clk_cnt[VA_CORE_CLK]) {
				lpass_cdc_clk_rsc_mux1_clk_request(priv,
							VA_CORE_CLK, enable);
			} else if (priv->va_tx_clk_cnt) {
				lpass_cdc_clk_rsc_mux0_clk_request(priv,
							TX_CORE_CLK, enable);
				priv->va_tx_clk_cnt--;
			}
		} else if (priv->va_tx_clk_cnt == priv->clk_cnt[TX_CORE_CLK]) {
			/*
			 * Handle the following usecase scenarios during disable
			 * Record+VA-> VA: enable VA CLK, decrement TX CLK count
			 */
			while (priv->va_tx_clk_cnt) {
				ret = lpass_cdc_clk_rsc_mux1_clk_request(priv,
							VA_CORE_CLK, true);
				if (ret < 0)
					goto err;

				lpass_cdc_clk_rsc_mux0_clk_request(priv,
							TX_CORE_CLK, false);
				priv->va_tx_clk_cnt--;
			}
		}
	}

err:
	return ret;
}

/**
 * lpass_cdc_clk_rsc_fs_gen_request - request to enable/disable fs generation
 * sequence
 *
 * @dev: Macro device pointer
 * @enable: enable or disable flag
 */
void lpass_cdc_clk_rsc_fs_gen_request(struct device *dev, bool enable)
{
	int i;
	struct regmap *regmap;
	struct device *clk_dev = NULL;
	struct lpass_cdc_clk_rsc *priv = NULL;

	if (!dev) {
		pr_err("%s: dev is null %d\n", __func__);
		return;
	}
	clk_dev = lpass_cdc_get_rsc_clk_device_ptr(dev->parent);
	if (!clk_dev) {
		pr_err("%s: Invalid rsc clk device\n", __func__);
		return;
	}
	priv = dev_get_drvdata(clk_dev);
	if (!priv) {
		pr_err("%s: Invalid rsc clk priviate data\n", __func__);
		return;
	}
	regmap = dev_get_regmap(priv->dev->parent, NULL);
	if (!regmap) {
		pr_err("%s: regmap is null\n", __func__);
		return;
	}
	mutex_lock(&priv->fs_gen_lock);
	if (enable) {
		if (priv->reg_seq_en_cnt++ == 0) {
			for (i = 0; i < (priv->num_fs_reg * 3); i += 3) {
				dev_dbg(priv->dev, "%s: Register: %d, mask: %d, value: %d\n",
					__func__, priv->fs_gen_seq[i],
					priv->fs_gen_seq[i + 1],
					priv->fs_gen_seq[i + 2]);
				regmap_update_bits(regmap,
						   priv->fs_gen_seq[i],
						   priv->fs_gen_seq[i + 1],
						   priv->fs_gen_seq[i + 2]);
			}
		}
	} else {
		if (priv->reg_seq_en_cnt <= 0) {
			dev_err_ratelimited(priv->dev, "%s: req_seq_cnt: %d is already disabled\n",
				__func__, priv->reg_seq_en_cnt);
			priv->reg_seq_en_cnt = 0;
			mutex_unlock(&priv->fs_gen_lock);
			return;
		}
		if (--priv->reg_seq_en_cnt == 0) {
			for (i = ((priv->num_fs_reg - 1) * 3); i >= 0; i -= 3) {
				dev_dbg(priv->dev, "%s: Register: %d, mask: %d\n",
					__func__, priv->fs_gen_seq[i],
					priv->fs_gen_seq[i + 1]);
				regmap_update_bits(regmap, priv->fs_gen_seq[i],
						priv->fs_gen_seq[i + 1], 0x0);
			}
		}
	}
	mutex_unlock(&priv->fs_gen_lock);
}
EXPORT_SYMBOL(lpass_cdc_clk_rsc_fs_gen_request);

/**
 * lpass_cdc_clk_rsc_request_clock - request for clock to
 * enable/disable
 *
 * @dev: Macro device pointer.
 * @default_clk_id: mux0 Core clock ID input.
 * @clk_id_req: Core clock ID requested to enable/disable
 * @enable: enable or disable clock flag
 *
 * Returns 0 on success or -EINVAL on error.
 */
int lpass_cdc_clk_rsc_request_clock(struct device *dev,
				int default_clk_id,
				int clk_id_req,
				bool enable)
{
	int ret = 0;
	struct device *clk_dev = NULL;
	struct lpass_cdc_clk_rsc *priv = NULL;
	bool mux_switch = false;

	if (!dev) {
		pr_err("%s: dev is null %d\n", __func__);
		return -EINVAL;
	}
	if ((clk_id_req < 0 || clk_id_req >= MAX_CLK) &&
		(default_clk_id < 0 || default_clk_id >= MAX_CLK)) {
		pr_err("%s: Invalid clk_id_req: %d or default_clk_id: %d\n",
				__func__, clk_id_req, default_clk_id);
		return -EINVAL;
	}
	clk_dev = lpass_cdc_get_rsc_clk_device_ptr(dev->parent);
	if (!clk_dev) {
		pr_err("%s: Invalid rsc clk device\n", __func__);
		return -EINVAL;
	}
	priv = dev_get_drvdata(clk_dev);
	if (!priv) {
		pr_err("%s: Invalid rsc clk priviate data\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&priv->rsc_clk_lock);
	if (!priv->dev_up && enable) {
		dev_err_ratelimited(priv->dev, "%s: SSR is in progress..\n",
				__func__);
		trace_printk("%s: SSR is in progress..\n", __func__);
		ret = -EINVAL;
		goto err;
	}
	priv->default_clk_id[clk_id_req] = default_clk_id;
	if (default_clk_id != clk_id_req)
		mux_switch = true;

	if (mux_switch) {
		if (clk_id_req != VA_CORE_CLK) {
			ret = lpass_cdc_clk_rsc_mux1_clk_request(priv, clk_id_req,
							enable);
			if (ret < 0)
				goto err;
		}
	} else {
		ret = lpass_cdc_clk_rsc_mux0_clk_request(priv, clk_id_req, enable);
		if (ret < 0)
			goto err;
	}

	ret = lpass_cdc_clk_rsc_check_and_update_va_clk(priv, mux_switch,
						 clk_id_req,
						 enable);
	if (ret < 0)
		goto err;

	dev_dbg(priv->dev, "%s: clk_cnt: %d for requested clk: %d, enable: %d\n",
		__func__,  priv->clk_cnt[clk_id_req], clk_id_req,
		enable);
	trace_printk("%s: clk_cnt: %d for requested clk: %d, enable: %d\n",
		__func__,  priv->clk_cnt[clk_id_req], clk_id_req,
		enable);

	mutex_unlock(&priv->rsc_clk_lock);

	return 0;

err:
	mutex_unlock(&priv->rsc_clk_lock);
	return ret;
}
EXPORT_SYMBOL(lpass_cdc_clk_rsc_request_clock);


static int lpass_cdc_clk_rsc_probe(struct platform_device *pdev)
{
	int ret = 0, fs_gen_size, i, j;
	const char **clk_name_array;
	int clk_cnt;
	struct clk *clk;
	struct lpass_cdc_clk_rsc *priv = NULL;
	u32 muxsel = 0;

	priv = devm_kzalloc(&pdev->dev, sizeof(struct lpass_cdc_clk_rsc),
			    GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Get clk fs gen sequence from device tree */
	if (!of_find_property(pdev->dev.of_node, "qcom,fs-gen-sequence",
						  &fs_gen_size)) {
		dev_err(&pdev->dev, "%s: unable to find qcom,fs-gen-sequence property\n",
			__func__);
		ret = -EINVAL;
		goto err;
	}
	priv->num_fs_reg = fs_gen_size/(3 * sizeof(u32));
	priv->fs_gen_seq = devm_kzalloc(&pdev->dev, fs_gen_size, GFP_KERNEL);
	if (!priv->fs_gen_seq) {
		ret = -ENOMEM;
		goto err;
	}
	dev_dbg(&pdev->dev, "%s: num_fs_reg %d\n", __func__, priv->num_fs_reg);
	/* Parse fs-gen-sequence */
	ret = of_property_read_u32_array(pdev->dev.of_node,
					 "qcom,fs-gen-sequence",
					 priv->fs_gen_seq,
					 priv->num_fs_reg * 3);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: unable to parse fs-gen-sequence, ret = %d\n",
			__func__, ret);
		goto err;
	}

	/* Get clk details from device tree */
	clk_cnt = of_property_count_strings(pdev->dev.of_node, "clock-names");
	if (clk_cnt <= 0 || clk_cnt > MAX_CLK) {
		dev_err(&pdev->dev, "%s: Invalid number of clocks %d",
				__func__, clk_cnt);
		ret = -EINVAL;
		goto err;
	}
	clk_name_array = devm_kzalloc(&pdev->dev, clk_cnt * sizeof(char *),
					  GFP_KERNEL);
	if (!clk_name_array) {
		ret = -ENOMEM;
		goto err;
	}

	ret = of_property_read_string_array(pdev->dev.of_node, "clock-names",
					clk_name_array, clk_cnt);

	for (i = 0; i < MAX_CLK; i++) {
		priv->clk[i] = NULL;
		for (j = 0; j < clk_cnt; j++) {
			if (!strcmp(clk_src_name[i], clk_name_array[j])) {
				clk = devm_clk_get(&pdev->dev, clk_src_name[i]);
				if (IS_ERR(clk)) {
					ret = PTR_ERR(clk);
					dev_err(&pdev->dev, "%s: clk get failed for %s with ret %d\n",
						__func__, clk_src_name[i], ret);
					goto err;
				}
				priv->clk[i] = clk;
				dev_dbg(&pdev->dev, "%s: clk get success for clk name %s\n",
						__func__, clk_src_name[i]);
				break;
			}
		}
	}
	ret = of_property_read_u32(pdev->dev.of_node,
				 "qcom,rx_mclk_mode_muxsel", &muxsel);
	if (ret) {
		dev_dbg(&pdev->dev, "%s: could not find qcom,rx_mclk_mode_muxsel entry in dt\n",
			__func__);
	} else {
		priv->rx_clk_muxsel = devm_ioremap(&pdev->dev, muxsel, 0x4);
		if (!priv->rx_clk_muxsel) {
			dev_err(&pdev->dev, "%s: ioremap failed for rx muxsel\n",
				__func__);
			return -ENOMEM;
		}
	}
	ret = of_property_read_u32(pdev->dev.of_node,
				"qcom,wsa_mclk_mode_muxsel", &muxsel);
	if (ret) {
		dev_dbg(&pdev->dev, "%s: could not find qcom,wsa_mclk_mode_muxsel entry in dt\n",
			__func__);
	} else {
		priv->wsa_clk_muxsel = devm_ioremap(&pdev->dev, muxsel, 0x4);
		if (!priv->wsa_clk_muxsel) {
			dev_err(&pdev->dev, "%s: ioremap failed for wsa muxsel\n",
				__func__);
			return -ENOMEM;
		}
	}
	ret = of_property_read_u32(pdev->dev.of_node,
				 "qcom,va_mclk_mode_muxsel", &muxsel);
	if (ret) {
		dev_dbg(&pdev->dev, "%s: could not find qcom,va_mclk_mode_muxsel entry in dt\n",
			__func__);
	} else {
		priv->va_clk_muxsel = devm_ioremap(&pdev->dev, muxsel, 0x4);
		if (!priv->va_clk_muxsel) {
			dev_err(&pdev->dev, "%s: ioremap failed for va muxsel\n",
				__func__);
			return -ENOMEM;
		}
	}

	ret = lpass_cdc_register_res_clk(&pdev->dev, lpass_cdc_clk_rsc_cb);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: Failed to register cb %d",
				__func__, ret);
		goto err;
	}
	priv->dev = &pdev->dev;
	priv->dev_up = true;
	priv->dev_up_gfmux = true;
	mutex_init(&priv->rsc_clk_lock);
	mutex_init(&priv->fs_gen_lock);
	dev_set_drvdata(&pdev->dev, priv);

err:
	return ret;
}

static int lpass_cdc_clk_rsc_remove(struct platform_device *pdev)
{
	struct lpass_cdc_clk_rsc *priv = dev_get_drvdata(&pdev->dev);

	lpass_cdc_unregister_res_clk(&pdev->dev);
	of_platform_depopulate(&pdev->dev);
	if (!priv)
		return -EINVAL;
	mutex_destroy(&priv->rsc_clk_lock);
	mutex_destroy(&priv->fs_gen_lock);

	return 0;
}

static const struct of_device_id lpass_cdc_clk_rsc_dt_match[] = {
	{.compatible = "qcom,lpass-cdc-clk-rsc-mngr"},
	{}
};
MODULE_DEVICE_TABLE(of, lpass_cdc_clk_rsc_dt_match);

static struct platform_driver lpass_cdc_clk_rsc_mgr = {
	.driver = {
		.name = "lpass-cdc-clk-rsc-mngr",
		.owner = THIS_MODULE,
		.of_match_table = lpass_cdc_clk_rsc_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = lpass_cdc_clk_rsc_probe,
	.remove = lpass_cdc_clk_rsc_remove,
};

int lpass_cdc_clk_rsc_mgr_init(void)
{
	return platform_driver_register(&lpass_cdc_clk_rsc_mgr);
}

void lpass_cdc_clk_rsc_mgr_exit(void)
{
	platform_driver_unregister(&lpass_cdc_clk_rsc_mgr);
}
MODULE_DESCRIPTION("LPASS codec clock resource manager driver");
MODULE_LICENSE("GPL v2");
