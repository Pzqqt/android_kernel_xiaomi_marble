// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include "dsi_pll.h"

static int dsi_pll_clock_register(struct platform_device *pdev,
				struct dsi_pll_resource *pll_res)
{
	int rc;

	switch (pll_res->pll_revision) {
	case DSI_PLL_5NM:
		rc = dsi_pll_clock_register_5nm(pdev, pll_res);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	if (rc)
		DSI_PLL_ERR(pll_res, "clock register failed rc=%d\n", rc);

	return rc;
}

static inline int dsi_pll_get_ioresources(struct platform_device *pdev,
				void __iomem **regmap, char *resource_name)
{
	int rc = 0;
	struct resource *rsc = platform_get_resource_byname(pdev,
						IORESOURCE_MEM, resource_name);
	if (rsc) {
		if (!regmap)
			return -ENOMEM;

		*regmap = devm_ioremap(&pdev->dev,
					rsc->start, resource_size(rsc));
		if (!*regmap)
			return -ENOMEM;
	}
	return rc;
}

int dsi_pll_init(struct platform_device *pdev, struct dsi_pll_resource **pll)
{
	int rc = 0;
	const char *label;
	struct dsi_pll_resource *pll_res = NULL;

	if (!pdev->dev.of_node) {
		pr_err("Invalid DSI PHY node\n");
		return -ENOTSUPP;
	}

	pll_res = devm_kzalloc(&pdev->dev, sizeof(struct dsi_pll_resource),
								GFP_KERNEL);
	if (!pll_res)
		return -ENOMEM;

	*pll = pll_res;

	label = of_get_property(pdev->dev.of_node, "pll-label", NULL);
	if (!label) {
		DSI_PLL_ERR(pll_res, "DSI pll label not specified\n");
		return 0;
	}

	DSI_PLL_INFO(pll_res, "DSI pll label = %s\n", label);

	/**
	  * Currently, Only supports 5nm PLL version. Will add
	  * support for other versions as needed.
	  */

	if (!strcmp(label, "dsi_pll_5nm"))
		pll_res->pll_revision = DSI_PLL_5NM;
	else
		return -ENOTSUPP;

	rc = of_property_read_u32(pdev->dev.of_node, "cell-index",
			&pll_res->index);
	if (rc) {
		DSI_PLL_ERR(pll_res, "Unable to get the cell-index rc=%d\n", rc);
		pll_res->index = 0;
	}

	pll_res->ssc_en = of_property_read_bool(pdev->dev.of_node,
						"qcom,dsi-pll-ssc-en");

	if (pll_res->ssc_en) {
		DSI_PLL_INFO(pll_res, "PLL SSC enabled\n");

		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,ssc-frequency-hz", &pll_res->ssc_freq);

		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,ssc-ppm", &pll_res->ssc_ppm);

		pll_res->ssc_center = false;

		label = of_get_property(pdev->dev.of_node,
			"qcom,dsi-pll-ssc-mode", NULL);

		if (label && !strcmp(label, "center-spread"))
			pll_res->ssc_center = true;
	}


	if (dsi_pll_get_ioresources(pdev, &pll_res->pll_base, "pll_base")) {
		DSI_PLL_ERR(pll_res, "Unable to remap pll base resources\n");
		return -ENOMEM;
	}

	pr_info("PLL base=%p\n", pll_res->pll_base);

	if (dsi_pll_get_ioresources(pdev, &pll_res->phy_base, "dsi_phy")) {
		DSI_PLL_ERR(pll_res, "Unable to remap pll phy base resources\n");
		return -ENOMEM;
	}

	if (dsi_pll_get_ioresources(pdev, &pll_res->dyn_pll_base,
							"dyn_refresh_base")) {
		DSI_PLL_ERR(pll_res, "Unable to remap dynamic pll base resources\n");
		return -ENOMEM;
	}

	if (dsi_pll_get_ioresources(pdev, &pll_res->gdsc_base, "gdsc_base")) {
		DSI_PLL_ERR(pll_res, "Unable to remap gdsc base resources\n");
		return -ENOMEM;
	}

	rc = dsi_pll_clock_register(pdev, pll_res);
	if (rc) {
		DSI_PLL_ERR(pll_res, "clock register failed rc=%d\n", rc);
		return -EINVAL;
	}

	return rc;

}
