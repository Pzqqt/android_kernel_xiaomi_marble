// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, Google LLC. All rights reserved.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>

static int coresight_clk_disable_probe(struct platform_device *pdev)
{
	return 0;
}

static int coresight_clk_disable_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id coresight_clk_disable_match[] = {
	{ .compatible = "qcom,coresight-csr" },
	{}
};

static struct platform_driver coresight_clk_disable_driver = {
	.probe          = coresight_clk_disable_probe,
	.remove         = coresight_clk_disable_remove,
	.driver         = {
		.name   = "coresight-clk-disable",
		.of_match_table	= coresight_clk_disable_match,
		},
};

module_platform_driver(coresight_clk_disable_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("CoreSight DEBUGv8 and ETMv4 clock disable driver stub");
MODULE_AUTHOR("J. Avila <elavila@google.com>");
