/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/component.h>
#include <sound/soc.h>
#include <soc/soundwire.h>
#include "../msm-cdc-pinctrl.h"

struct wcd937x_priv {
	struct device *dev;
	struct snd_soc_codec *codec;
	struct device_node *rst_np;
	struct swr_device *rx_swr_dev;
	struct swr_device *tx_swr_dev;
};

struct wcd937x_pdata {
	struct device_node *rst_np;
	struct device_node *rx_slave;
	struct device_node *tx_slave;
};

static int wcd937x_soc_codec_probe(struct snd_soc_codec *codec)
{
	struct wcd937x_priv *wcd937x = snd_soc_codec_get_drvdata(codec);

	if (!wcd937x)
		return -EINVAL;

	return 0;
}

static int wcd937x_soc_codec_remove(struct snd_soc_codec *codec)
{
	struct wcd937x_priv *wcd937x = snd_soc_codec_get_drvdata(codec);

	if (!wcd937x)
		return -EINVAL;

	return 0;
}

static struct regmap *wcd937x_get_regmap(struct device *dev)
{
	struct wcd937x_priv *wcd937x = dev_get_drvdata(dev);

	return wcd937x->regmap;
}

static const struct snd_kcontrol_new wcd937x_snd_controls[] = {
};

static const struct snd_soc_dapm_widget wcd937x_dapm_widgets[] = {
};

static const struct snd_soc_dapm_route wcd937x_audio_map[] = {
};

static struct snd_soc_codec_driver soc_codec_dev_wcd937x = {
	.probe = wcd937x_soc_codec_probe,
	.remove = wcd937x_soc_codec_remove,
	.get_regmap = wcd937x_get_regmap,
	.component_driver = {
		.controls = wcd937x_snd_controls,
		.num_controls = ARRAY_SIZE(wcd937x_snd_controls),
		.dapm_widgets = wcd937x_dapm_widgets,
		.num_dapm_widgets = ARRAY_SIZE(wcd937x_dapm_widgets),
		.dapm_routes = wcd937x_audio_map,
		.num_dapm_routes = ARRAY_SIZE(wcd937x_audio_map),
	},
};

int wcd937x_reset(struct device *dev)
{
	struct wcd937x_priv *wcd937x = NULL;
	int rc = 0;
	int value = 0;

	if (!dev)
		return -ENODEV;

	wcd937x = dev_get_drvdata(dev);
	if (!wcd937x)
		return -EINVAL;

	if (!wcd937x->rst_np) {
		dev_err(dev, "%s: reset gpio device node not specified\n",
				__func__);
		return -EINVAL;
	}

	value = msm_cdc_pinctrl_get_state(wcd937x->rst_np);
	if (value > 0)
		return 0;

	rc = msm_cdc_pinctrl_select_sleep_state(wcd937x->rst_np);
	if (rc) {
		dev_err(dev, "%s: wcd sleep state request fail!\n",
				__func__);
		return rc;
	}

	/* 20ms sleep required after pulling the reset gpio to LOW */
	msleep(20);

	rc = msm_cdc_pinctrl_select_active_state(wcd937x->rst_np);
	if (rc) {
		dev_err(dev, "%s: wcd active state request fail!\n",
				__func__);
		return rc;
	}
	msleep(20);

	return rc;
}

struct wcd937x_pdata *wcd937x_populate_dt_data(struct device *dev)
{
	struct wcd937x_pdata *pdata = NULL;

	pdata = devm_kzalloc(dev, sizeof(struct wcd937x_pdata),
				GFP_KERNEL);
	if (!pdata)
		return NULL;

	pdata->rst_np = of_parse_phandle(dev->of_node,
			"qcom,wcd937x-reset-node", 0);
	if (!pdata->rst_np) {
		dev_err(dev, "%s: Looking up %s property in node %s failed\n",
				__func__, "qcom,wcd937x-reset-node",
				dev->of_node->full_name);
		return NULL;
	}

	pdata->rx_slave = of_parse_phandle(dev->of_node, "qcom,rx-slave", 0);
	pdata->tx_slave = of_parse_phandle(dev->of_node, "qcom,tx-slave", 0);

	return pdata;
}

static int wcd937x_bind(struct device *dev)
{
	int ret = 0;
	struct wcd937x_priv *wcd937x = NULL;
	struct wcd937x_pdata *pdata = NULL;

	wcd937x = devm_kzalloc(dev, sizeof(struct wcd937x_priv), GFP_KERNEL);
	if (!wcd937x)
		return -ENOMEM;

	dev_set_drvdata(dev, wcd937x);

	pdata = wcd937x_populate_dt_data(dev);
	if (!pdata) {
		dev_err(dev, "%s: Fail to obtain platform data\n", __func__);
		return -EINVAL;
	}

	wcd937x->rst_np = pdata->rst_np;
	wcd937x_reset(dev);
	/*
	 * Add 5msec delay to provide sufficient time for
	 * soundwire auto enumeration of slave devices as
	 * as per HW requirement.
	 */
	usleep_range(5000, 5010);
	ret = component_bind_all(dev, wcd937x);
	if (ret) {
		dev_err(dev, "%s: Slave bind failed, ret = %d\n",
			__func__, ret);
		return ret;
	}

	wcd937x->rx_swr_dev = get_matching_swr_slave_device(pdata->rx_slave);
	if (!wcd937x->rx_swr_dev) {
		dev_err(dev, "%s: Could not find RX swr slave device\n",
			 __func__);
		ret = -ENODEV;
		goto err;
	}

	wcd937x->tx_swr_dev = get_matching_swr_slave_device(pdata->tx_slave);
	if (!wcd937x->tx_swr_dev) {
		dev_err(dev, "%s: Could not find TX swr slave device\n",
			__func__);
		ret = -ENODEV;
		goto err;
	}

	ret = snd_soc_register_codec(dev, &soc_codec_dev_wcd937x,
			NULL, 0);
	if (ret) {
		dev_err(dev, "%s: Codec registration failed\n",
				__func__);
		goto err;
	}

	return ret;
err:
	component_unbind_all(dev, wcd937x);
	return ret;
}

static void wcd937x_unbind(struct device *dev)
{
	struct wcd937x_priv *wcd937x = dev_get_drvdata(dev);

	snd_soc_unregister_codec(dev);
	component_unbind_all(dev, wcd937x);
}

static const struct of_device_id wcd937x_dt_match[] = {
	{ .compatible = "qcom,wcd937x-codec" },
	{}
};

static const struct component_master_ops wcd937x_comp_ops = {
	.bind   = wcd937x_bind,
	.unbind = wcd937x_unbind,
};

static int wcd937x_compare_of(struct device *dev, void *data)
{
	return dev->of_node == data;
}

static void wcd937x_release_of(struct device *dev, void *data)
{
	of_node_put(data);
}

static int wcd937x_add_slave_components(struct device *dev,
				struct component_match **matchptr)
{
	struct device_node *np, *rx_node, *tx_node;

	np = dev->of_node;

	rx_node = of_parse_phandle(np, "qcom,rx-slave", 0);
	if (!rx_node) {
		dev_err(dev, "%s: Rx-slave node not defined\n", __func__);
		return -ENODEV;
	}
	of_node_get(rx_node);
	component_match_add_release(dev, matchptr,
			wcd937x_release_of,
			wcd937x_compare_of,
			rx_node);

	tx_node = of_parse_phandle(np, "qcom,tx-slave", 0);
	if (!tx_node) {
		dev_err(dev, "%s: Tx-slave node not defined\n", __func__);
			return -ENODEV;
	}
	of_node_get(tx_node);
	component_match_add_release(dev, matchptr,
			wcd937x_release_of,
			wcd937x_compare_of,
			tx_node);
	return 0;
}

static int wcd937x_probe(struct platform_device *pdev)
{
	struct component_match *match = NULL;
	int ret;

	ret = wcd937x_add_slave_components(&pdev->dev, &match);
	if (ret)
		return ret;

	return component_master_add_with_match(&pdev->dev,
					&wcd937x_comp_ops, match);
}

static int wcd937x_remove(struct platform_device *pdev)
{
	component_master_del(&pdev->dev, &wcd937x_comp_ops);
	return 0;
}

static struct platform_driver wcd937x_codec_driver = {
	.probe = wcd937x_probe,
	.remove = wcd937x_remove,
	.driver = {
		.name = "wcd937x_codec",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wcd937x_dt_match),
	},
};

module_platform_driver(wcd937x_codec_driver);
MODULE_DESCRIPTION("WCD937X Codec driver");
MODULE_LICENSE("GPL v2");
