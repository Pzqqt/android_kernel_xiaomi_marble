// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/component.h>
#include <soc/soundwire.h>

struct wcd938x_slave_priv {
	struct swr_device *swr_slave;
};

static int wcd938x_slave_bind(struct device *dev,
				struct device *master, void *data)
{
	int ret = 0;
	uint8_t devnum = 0;
	struct swr_device *pdev = to_swr_device(dev);

	if (!pdev) {
		pr_err("%s: invalid swr device handle\n", __func__);
		return -EINVAL;
	}

	ret = swr_get_logical_dev_num(pdev, pdev->addr, &devnum);
	if (ret) {
		dev_dbg(&pdev->dev,
			"%s get devnum %d for dev addr %lx failed\n",
			__func__, devnum, pdev->addr);
		return ret;
	}
	pdev->dev_num = devnum;

	return ret;
}

static void wcd938x_slave_unbind(struct device *dev,
				struct device *master, void *data)
{
	struct wcd938x_slave_priv *wcd938x_slave = NULL;
	struct swr_device *pdev = to_swr_device(dev);

	wcd938x_slave = swr_get_dev_data(pdev);
	if (!wcd938x_slave) {
		dev_err(&pdev->dev, "%s: wcd938x_slave is NULL\n", __func__);
		return;
	}
}

static const struct swr_device_id wcd938x_swr_id[] = {
	{"wcd938x-slave", 0},
	{}
};

static const struct of_device_id wcd938x_swr_dt_match[] = {
	{
		.compatible = "qcom,wcd938x-slave",
	},
	{}
};

static const struct component_ops wcd938x_slave_comp_ops = {
	.bind   = wcd938x_slave_bind,
	.unbind = wcd938x_slave_unbind,
};

static int wcd938x_swr_probe(struct swr_device *pdev)
{
	struct wcd938x_slave_priv *wcd938x_slave = NULL;

	wcd938x_slave = devm_kzalloc(&pdev->dev,
				sizeof(struct wcd938x_slave_priv), GFP_KERNEL);
	if (!wcd938x_slave)
		return -ENOMEM;

	swr_set_dev_data(pdev, wcd938x_slave);

	wcd938x_slave->swr_slave = pdev;

	return component_add(&pdev->dev, &wcd938x_slave_comp_ops);
}

static int wcd938x_swr_remove(struct swr_device *pdev)
{
	component_del(&pdev->dev, &wcd938x_slave_comp_ops);
	swr_set_dev_data(pdev, NULL);
	swr_remove_device(pdev);

	return 0;
}

static struct swr_driver wcd938x_slave_driver = {
	.driver = {
		.name = "wcd938x-slave",
		.owner = THIS_MODULE,
		.of_match_table = wcd938x_swr_dt_match,
	},
	.probe = wcd938x_swr_probe,
	.remove = wcd938x_swr_remove,
	.id_table = wcd938x_swr_id,
};

static int __init wcd938x_slave_init(void)
{
	return swr_driver_register(&wcd938x_slave_driver);
}

static void __exit wcd938x_slave_exit(void)
{
	swr_driver_unregister(&wcd938x_slave_driver);
}

module_init(wcd938x_slave_init);
module_exit(wcd938x_slave_exit);

MODULE_DESCRIPTION("WCD938X Swr Slave driver");
MODULE_LICENSE("GPL v2");
