// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/component.h>
#include <soc/soundwire.h>

struct rouleur_slave_priv {
	struct swr_device *swr_slave;
};

static int rouleur_slave_bind(struct device *dev,
				struct device *master, void *data)
{
	int ret = 0;
	struct rouleur_slave_priv *rouleur_slave = NULL;
	uint8_t devnum = 0;
	struct swr_device *pdev = to_swr_device(dev);

	if (pdev == NULL) {
		dev_err(dev, "%s: pdev is NULL\n", __func__);
		return -EINVAL;
	}

	rouleur_slave = devm_kzalloc(&pdev->dev,
				sizeof(struct rouleur_slave_priv), GFP_KERNEL);
	if (!rouleur_slave)
		return -ENOMEM;

	swr_set_dev_data(pdev, rouleur_slave);

	rouleur_slave->swr_slave = pdev;

	ret = swr_get_logical_dev_num(pdev, pdev->addr, &devnum);
	if (ret) {
		dev_dbg(&pdev->dev,
				"%s get devnum %d for dev addr %lx failed\n",
				__func__, devnum, pdev->addr);
		swr_remove_device(pdev);
		return ret;
	}
	pdev->dev_num = devnum;

	return ret;
}

static void rouleur_slave_unbind(struct device *dev,
				struct device *master, void *data)
{
	struct rouleur_slave_priv *rouleur_slave = NULL;
	struct swr_device *pdev = to_swr_device(dev);

	if (pdev == NULL) {
		dev_err(dev, "%s: pdev is NULL\n", __func__);
		return;
	}

	rouleur_slave = swr_get_dev_data(pdev);
	if (!rouleur_slave) {
		dev_err(&pdev->dev, "%s: rouleur_slave is NULL\n", __func__);
		return;
	}

	swr_set_dev_data(pdev, NULL);
}

static const struct swr_device_id rouleur_swr_id[] = {
	{"rouleur-slave", 0},
	{}
};

static const struct of_device_id rouleur_swr_dt_match[] = {
	{
		.compatible = "qcom,rouleur-slave",
	},
	{}
};

static const struct component_ops rouleur_slave_comp_ops = {
	.bind   = rouleur_slave_bind,
	.unbind = rouleur_slave_unbind,
};

static int rouleur_swr_up(struct swr_device *pdev)
{
	return 0;
}

static int rouleur_swr_down(struct swr_device *pdev)
{
	return 0;
}

static int rouleur_swr_reset(struct swr_device *pdev)
{
	return 0;
}

static int rouleur_swr_probe(struct swr_device *pdev)
{
	return component_add(&pdev->dev, &rouleur_slave_comp_ops);
}

static int rouleur_swr_remove(struct swr_device *pdev)
{
	component_del(&pdev->dev, &rouleur_slave_comp_ops);
	return 0;
}

static struct swr_driver rouleur_slave_driver = {
	.driver = {
		.name = "rouleur-slave",
		.owner = THIS_MODULE,
		.of_match_table = rouleur_swr_dt_match,
	},
	.probe = rouleur_swr_probe,
	.remove = rouleur_swr_remove,
	.id_table = rouleur_swr_id,
	.device_up = rouleur_swr_up,
	.device_down = rouleur_swr_down,
	.reset_device = rouleur_swr_reset,
};

static int __init rouleur_slave_init(void)
{
	return swr_driver_register(&rouleur_slave_driver);
}

static void __exit rouleur_slave_exit(void)
{
	swr_driver_unregister(&rouleur_slave_driver);
}

module_init(rouleur_slave_init);
module_exit(rouleur_slave_exit);

MODULE_DESCRIPTION("WCD937X Swr Slave driver");
MODULE_LICENSE("GPL v2");
