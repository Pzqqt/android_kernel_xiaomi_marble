// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/iommu.h>
#include <linux/of.h>
#include <linux/sort.h>

#include "msm_vidc_dt.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"

static size_t get_u32_array_num_elements(struct device_node *np,
					char *name)
{
	int len;
	size_t num_elements = 0;

	if (!of_get_property(np, name, &len)) {
		d_vpr_e("Failed to read %s from device tree\n", name);
		goto fail_read;
	}

	num_elements = len / sizeof(u32);
	if (num_elements <= 0) {
		d_vpr_e("%s not specified in device tree\n", name);
		goto fail_read;
	}
	return num_elements;

fail_read:
	return 0;
}

/**
 * msm_vidc_load_u32_table() - load dtsi table entries
 * @pdev: A pointer to the platform device.
 * @of_node:      A pointer to the device node.
 * @table_name:   A pointer to the dtsi table entry name.
 * @struct_size:  The size of the structure which is nothing but
 *                a single entry in the dtsi table.
 * @table:        A pointer to the table pointer which needs to be
 *                filled by the dtsi table entries.
 * @num_elements: Number of elements pointer which needs to be filled
 *                with the number of elements in the table.
 *
 * This is a generic implementation to load single or multiple array
 * table from dtsi. The array elements should be of size equal to u32.
 *
 * Return:        Return '0' for success else appropriate error value.
 */
static int msm_vidc_load_u32_table(struct platform_device *pdev,
		struct device_node *of_node, char *table_name, int struct_size,
		u32 **table, u32 *num_elements)
{
	int rc = 0, num_elemts = 0;
	u32 *ptbl = NULL;

	if (!of_find_property(of_node, table_name, NULL)) {
		d_vpr_h("%s not found\n", table_name);
		return 0;
	}

	num_elemts = get_u32_array_num_elements(of_node, table_name);
	if (!num_elemts) {
		d_vpr_e("no elements in %s\n", table_name);
		return 0;
	}
	num_elemts /= struct_size / sizeof(u32);

	ptbl = devm_kzalloc(&pdev->dev, num_elemts * struct_size, GFP_KERNEL);
	if (!ptbl) {
		d_vpr_e("Failed to alloc table %s\n", table_name);
		return -ENOMEM;
	}

	if (of_property_read_u32_array(of_node, table_name, ptbl,
			num_elemts * struct_size / sizeof(u32))) {
		d_vpr_e("Failed to read %s\n", table_name);
		return -EINVAL;
	}

	*table = ptbl;
	if (num_elements)
		*num_elements = num_elemts;

	return rc;
}

/* A comparator to compare loads (needed later on) */
static int cmp(const void *a, const void *b)
{
	/* want to sort in reverse so flip the comparison */
	return ((struct allowed_clock_rates_table *)b)->clock_rate -
		((struct allowed_clock_rates_table *)a)->clock_rate;
}

static void msm_vidc_free_allowed_clocks_table(struct msm_vidc_dt *dt)
{
	dt->allowed_clks_tbl = NULL;
}

static void msm_vidc_free_reg_table(struct msm_vidc_dt *dt)
{
	dt->reg_set.reg_tbl = NULL;
}

static void msm_vidc_free_qdss_addr_table(struct msm_vidc_dt *dt)
{
	dt->qdss_addr_set.addr_tbl = NULL;
}

static void msm_vidc_free_bus_table(struct msm_vidc_dt *dt)
{
	dt->bus_set.bus_tbl = NULL;
	dt->bus_set.count = 0;
}

static void msm_vidc_free_buffer_usage_table(struct msm_vidc_dt *dt)
{
	dt->buffer_usage_set.buffer_usage_tbl = NULL;
}

static void msm_vidc_free_regulator_table(struct msm_vidc_dt *dt)
{
	int c = 0;

	for (c = 0; c < dt->regulator_set.count; ++c) {
		struct regulator_info *rinfo =
			&dt->regulator_set.regulator_tbl[c];

		rinfo->name = NULL;
	}

	dt->regulator_set.regulator_tbl = NULL;
	dt->regulator_set.count = 0;
}

static void msm_vidc_free_clock_table(struct msm_vidc_dt *dt)
{
	dt->clock_set.clock_tbl = NULL;
	dt->clock_set.count = 0;
}

static int msm_vidc_load_fw_name(struct msm_vidc_core *core)
{
	struct platform_device *pdev = core->pdev;

	return of_property_read_string_index(pdev->dev.of_node,
				"vidc,firmware-name", 0, &core->dt->fw_name);
}

static int msm_vidc_load_reg_table(struct msm_vidc_core *core)
{
	struct reg_set *reg_set;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	int i;
	int rc = 0;

	if (!of_find_property(pdev->dev.of_node, "qcom,reg-presets", NULL)) {
		/*
		 * qcom,reg-presets is an optional property.  It likely won't be
		 * present if we don't have any register settings to program
		 */
		d_vpr_h("reg-presets not found\n");
		return 0;
	}

	reg_set = &dt->reg_set;
	reg_set->count = get_u32_array_num_elements(pdev->dev.of_node,
			"qcom,reg-presets");
	reg_set->count /=  sizeof(*reg_set->reg_tbl) / sizeof(u32);

	if (!reg_set->count) {
		d_vpr_h("no elements in reg set\n");
		return rc;
	}

	reg_set->reg_tbl = devm_kzalloc(&pdev->dev, reg_set->count *
			sizeof(*(reg_set->reg_tbl)), GFP_KERNEL);
	if (!reg_set->reg_tbl) {
		d_vpr_e("%s: Failed to alloc register table\n", __func__);
		return -ENOMEM;
	}

	if (of_property_read_u32_array(pdev->dev.of_node, "qcom,reg-presets",
		(u32 *)reg_set->reg_tbl, reg_set->count * 3)) {
		d_vpr_e("Failed to read register table\n");
		msm_vidc_free_reg_table(core->dt);
		return -EINVAL;
	}
	for (i = 0; i < reg_set->count; i++) {
		d_vpr_h("reg = %#x, value = %#x, mask = %#x\n",
			reg_set->reg_tbl[i].reg, reg_set->reg_tbl[i].value,
			reg_set->reg_tbl[i].mask);
	}
	return rc;
}
static int msm_vidc_load_qdss_table(struct msm_vidc_core *core)
{
	struct addr_set *qdss_addr_set;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	int i;
	int rc = 0;

	if (!of_find_property(pdev->dev.of_node, "qcom,qdss-presets", NULL)) {
		/*
		 * qcom,qdss-presets is an optional property. It likely won't be
		 * present if we don't have any register settings to program
		 */
		d_vpr_h("qdss-presets not found\n");
		return rc;
	}

	qdss_addr_set = &dt->qdss_addr_set;
	qdss_addr_set->count = get_u32_array_num_elements(pdev->dev.of_node,
					"qcom,qdss-presets");
	qdss_addr_set->count /= sizeof(*qdss_addr_set->addr_tbl) / sizeof(u32);

	if (!qdss_addr_set->count) {
		d_vpr_h("no elements in qdss reg set\n");
		return rc;
	}

	qdss_addr_set->addr_tbl = devm_kzalloc(&pdev->dev,
			qdss_addr_set->count * sizeof(*qdss_addr_set->addr_tbl),
			GFP_KERNEL);
	if (!qdss_addr_set->addr_tbl) {
		d_vpr_e("%s: Failed to alloc register table\n", __func__);
		rc = -ENOMEM;
		goto err_qdss_addr_tbl;
	}

	rc = of_property_read_u32_array(pdev->dev.of_node, "qcom,qdss-presets",
		(u32 *)qdss_addr_set->addr_tbl, qdss_addr_set->count * 2);
	if (rc) {
		d_vpr_e("Failed to read qdss address table\n");
		msm_vidc_free_qdss_addr_table(core->dt);
		rc = -EINVAL;
		goto err_qdss_addr_tbl;
	}

	for (i = 0; i < qdss_addr_set->count; i++) {
		d_vpr_h("qdss addr = %x, value = %x\n",
				qdss_addr_set->addr_tbl[i].start,
				qdss_addr_set->addr_tbl[i].size);
	}
err_qdss_addr_tbl:
	return rc;
}

static int msm_vidc_load_subcache_info(struct msm_vidc_core *core)
{
	int rc = 0, num_subcaches = 0, c;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	struct subcache_set *subcaches = &dt->subcache_set;

	num_subcaches = of_property_count_strings(pdev->dev.of_node,
		"cache-slice-names");
	if (num_subcaches <= 0) {
		d_vpr_h("No subcaches found\n");
		goto err_load_subcache_table_fail;
	}

	subcaches->subcache_tbl = devm_kzalloc(&pdev->dev,
		sizeof(*subcaches->subcache_tbl) * num_subcaches, GFP_KERNEL);
	if (!subcaches->subcache_tbl) {
		d_vpr_e("Failed to allocate memory for subcache tbl\n");
		rc = -ENOMEM;
		goto err_load_subcache_table_fail;
	}

	subcaches->count = num_subcaches;
	d_vpr_h("Found %d subcaches\n", num_subcaches);

	for (c = 0; c < num_subcaches; ++c) {
		struct subcache_info *vsc = &dt->subcache_set.subcache_tbl[c];

		of_property_read_string_index(pdev->dev.of_node,
			"cache-slice-names", c, &vsc->name);
	}

	dt->sys_cache_present = true;

	return 0;

err_load_subcache_table_fail:
	dt->sys_cache_present = false;
	subcaches->count = 0;
	subcaches->subcache_tbl = NULL;

	return rc;
}

static int msm_vidc_load_allowed_clocks_table(
		struct msm_vidc_core *core)
{
	int rc = 0;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	int i;

	if (!of_find_property(pdev->dev.of_node,
			"qcom,allowed-clock-rates", NULL)) {
		d_vpr_h("allowed-clock-rates not found\n");
		return 0;
	}

	rc = msm_vidc_load_u32_table(pdev, pdev->dev.of_node,
				"qcom,allowed-clock-rates",
				sizeof(*dt->allowed_clks_tbl),
				(u32 **)&dt->allowed_clks_tbl,
				&dt->allowed_clks_tbl_size);
	if (rc) {
		d_vpr_e("%s: failed to read allowed clocks table\n", __func__);
		return rc;
	}

	sort(dt->allowed_clks_tbl, dt->allowed_clks_tbl_size,
		 sizeof(*dt->allowed_clks_tbl), cmp, NULL);

	d_vpr_h("Found allowed clock rates\n");
	for (i = 0; i < dt->allowed_clks_tbl_size; i++)
		d_vpr_h("    %d\n", dt->allowed_clks_tbl[i]);

	return 0;
}

static int msm_vidc_load_bus_table(struct msm_vidc_core *core)
{
	int rc = 0;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	struct bus_set *buses = &dt->bus_set;
	int c = 0, num_buses = 0;
	u32 *bus_ranges = NULL;

	num_buses = of_property_count_strings(pdev->dev.of_node,
				"interconnect-names");
	if (num_buses <= 0) {
		d_vpr_e("No buses found\n");
		return -EINVAL;
	}

	buses->count = num_buses;
	d_vpr_h("Found %d bus interconnects\n", num_buses);

	bus_ranges = kzalloc(2 * num_buses * sizeof(*bus_ranges), GFP_KERNEL);
	if (!bus_ranges) {
		d_vpr_e("No memory to read bus ranges\n");
		return -ENOMEM;
	}

	rc = of_property_read_u32_array(pdev->dev.of_node,
				"qcom,bus-range-kbps", bus_ranges,
				num_buses * 2);
	if (rc) {
		d_vpr_e(
			"Failed to read bus ranges: defaulting to <0 INT_MAX>\n");
		for (c = 0; c < num_buses; c++) {
			bus_ranges[c * 2] = 0;
			bus_ranges[c * 2 + 1] = INT_MAX;
		}
	}

	buses->bus_tbl = devm_kzalloc(&pdev->dev, num_buses *
				sizeof(*buses->bus_tbl), GFP_KERNEL);
	if (!buses->bus_tbl) {
		d_vpr_e("No memory for bus table\n");
		rc = -ENOMEM;
		goto exit;
	}

	for (c = 0; c < num_buses; c++) {
		struct bus_info *bus = &dt->bus_set.bus_tbl[c];

		of_property_read_string_index(pdev->dev.of_node,
			"interconnect-names", c, &bus->name);

		bus->dev = &pdev->dev;
		bus->range[0] = bus_ranges[c * 2];
		bus->range[1] = bus_ranges[c * 2 + 1];

		d_vpr_h("Found bus %s, range [%d %d]\n", bus->name,
			bus->range[0], bus->range[1]);
	}

exit:
	kfree(bus_ranges);
	return rc;
}

/* TODO: move this to platform data */
static int msm_vidc_load_buffer_usage_table(struct msm_vidc_core *core)
{
	int rc = 0;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	struct buffer_usage_set *buffer_usage_set = &dt->buffer_usage_set;

	if (!of_find_property(pdev->dev.of_node,
				"qcom,buffer-type-tz-usage-table", NULL)) {
		/*
		 * qcom,buffer-type-tz-usage-table is an optional property.  It
		 * likely won't be present if the core doesn't support content
		 * protection
		 */
		d_vpr_h("buffer-type-tz-usage-table not found\n");
		return 0;
	}

	buffer_usage_set->count = get_u32_array_num_elements(
		pdev->dev.of_node, "qcom,buffer-type-tz-usage-table");
	buffer_usage_set->count /=
		sizeof(*buffer_usage_set->buffer_usage_tbl) / sizeof(u32);
	if (!buffer_usage_set->count) {
		d_vpr_h("no elements in buffer usage set\n");
		return 0;
	}

	buffer_usage_set->buffer_usage_tbl = devm_kzalloc(&pdev->dev,
			buffer_usage_set->count *
			sizeof(*buffer_usage_set->buffer_usage_tbl),
			GFP_KERNEL);
	if (!buffer_usage_set->buffer_usage_tbl) {
		d_vpr_e("%s: Failed to alloc buffer usage table\n",
			__func__);
		rc = -ENOMEM;
		goto err_load_buf_usage;
	}

	rc = of_property_read_u32_array(pdev->dev.of_node,
		    "qcom,buffer-type-tz-usage-table",
		(u32 *)buffer_usage_set->buffer_usage_tbl,
		buffer_usage_set->count *
		sizeof(*buffer_usage_set->buffer_usage_tbl) / sizeof(u32));
	if (rc) {
		d_vpr_e("Failed to read buffer usage table\n");
		goto err_load_buf_usage;
	}

	return 0;
err_load_buf_usage:
	msm_vidc_free_buffer_usage_table(core->dt);
	return rc;
}

static int msm_vidc_load_regulator_table(struct msm_vidc_core *core)
{
	int rc = 0;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	struct regulator_set *regulators = &dt->regulator_set;
	struct device_node *domains_parent_node = NULL;
	struct property *domains_property = NULL;
	int reg_count = 0;

	regulators->count = 0;
	regulators->regulator_tbl = NULL;

	domains_parent_node = pdev->dev.of_node;
	for_each_property_of_node(domains_parent_node, domains_property) {
		const char *search_string = "-supply";
		char *supply;
		bool matched = false;

		/* check if current property is possibly a regulator */
		supply = strnstr(domains_property->name, search_string,
				strlen(domains_property->name) + 1);
		matched = supply && (*(supply + strlen(search_string)) == '\0');
		if (!matched)
			continue;

		reg_count++;
	}

	regulators->regulator_tbl = devm_kzalloc(&pdev->dev,
			sizeof(*regulators->regulator_tbl) *
			reg_count, GFP_KERNEL);

	if (!regulators->regulator_tbl) {
		rc = -ENOMEM;
		d_vpr_e("Failed to alloc memory for regulator table\n");
		goto err_reg_tbl_alloc;
	}

	for_each_property_of_node(domains_parent_node, domains_property) {
		const char *search_string = "-supply";
		char *supply;
		bool matched = false;
		struct device_node *regulator_node = NULL;
		struct regulator_info *rinfo = NULL;

		/* check if current property is possibly a regulator */
		supply = strnstr(domains_property->name, search_string,
				strlen(domains_property->name) + 1);
		matched = supply && (supply[strlen(search_string)] == '\0');
		if (!matched)
			continue;

		/* make sure prop isn't being misused */
		regulator_node = of_parse_phandle(domains_parent_node,
				domains_property->name, 0);
		if (IS_ERR(regulator_node)) {
			d_vpr_e("%s is not a phandle\n",
				domains_property->name);
			continue;
		}
		regulators->count++;

		/* populate regulator info */
		rinfo = &regulators->regulator_tbl[regulators->count - 1];
		rinfo->name = devm_kzalloc(&pdev->dev,
			(supply - domains_property->name) + 1, GFP_KERNEL);
		if (!rinfo->name) {
			rc = -ENOMEM;
			d_vpr_e("Failed to alloc memory for regulator name\n");
			goto err_reg_name_alloc;
		}
		strlcpy(rinfo->name, domains_property->name,
			(supply - domains_property->name) + 1);

		rinfo->has_hw_power_collapse = of_property_read_bool(
			regulator_node, "qcom,support-hw-trigger");

		d_vpr_h("Found regulator %s: h/w collapse = %s\n",
				rinfo->name,
				rinfo->has_hw_power_collapse ? "yes" : "no");
	}

	if (!regulators->count)
		d_vpr_h("No regulators found");

	return 0;

err_reg_name_alloc:
err_reg_tbl_alloc:
	msm_vidc_free_regulator_table(core->dt);
	return rc;
}

static int msm_vidc_load_clock_table(struct msm_vidc_core *core)
{
	int rc = 0, num_clocks = 0, c = 0;
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	int *clock_props = NULL;
	struct clock_set *clocks = &dt->clock_set;

	num_clocks = of_property_count_strings(pdev->dev.of_node,
				"clock-names");
	if (num_clocks <= 0) {
		d_vpr_h("No clocks found\n");
		clocks->count = 0;
		rc = 0;
		goto err_load_clk_table_fail;
	}

	clock_props = devm_kzalloc(&pdev->dev, num_clocks *
			sizeof(*clock_props), GFP_KERNEL);
	if (!clock_props) {
		d_vpr_e("No memory to read clock properties\n");
		rc = -ENOMEM;
		goto err_load_clk_table_fail;
	}

	rc = of_property_read_u32_array(pdev->dev.of_node,
				"qcom,clock-configs", clock_props,
				num_clocks);
	if (rc) {
		d_vpr_e("Failed to read clock properties: %d\n", rc);
		goto err_load_clk_prop_fail;
	}

	clocks->clock_tbl = devm_kzalloc(&pdev->dev, sizeof(*clocks->clock_tbl)
			* num_clocks, GFP_KERNEL);
	if (!clocks->clock_tbl) {
		d_vpr_e("Failed to allocate memory for clock tbl\n");
		rc = -ENOMEM;
		goto err_load_clk_prop_fail;
	}

	clocks->count = num_clocks;
	d_vpr_h("Found %d clocks\n", num_clocks);

	for (c = 0; c < num_clocks; ++c) {
		struct clock_info *vc = &dt->clock_set.clock_tbl[c];

		of_property_read_string_index(pdev->dev.of_node,
				"clock-names", c, &vc->name);

		if (clock_props[c] & CLOCK_PROP_HAS_SCALING) {
			vc->has_scaling = true;
		} else {
			vc->has_scaling = false;
		}

		if (clock_props[c] & CLOCK_PROP_HAS_MEM_RETENTION)
			vc->has_mem_retention = true;
		else
			vc->has_mem_retention = false;

		d_vpr_h("Found clock %s: scale-able = %s\n", vc->name,
			vc->has_scaling ? "yes" : "no");
	}


	return 0;

err_load_clk_prop_fail:
err_load_clk_table_fail:
	return rc;
}

static int msm_vidc_load_reset_table(struct msm_vidc_core *core)
{
	struct platform_device *pdev = core->pdev;
	struct msm_vidc_dt *dt = core->dt;
	struct reset_set *rst = &dt->reset_set;
	int num_clocks = 0, c = 0;

	num_clocks = of_property_count_strings(pdev->dev.of_node,
				"reset-names");
	if (num_clocks <= 0) {
		d_vpr_h("No reset clocks found\n");
		rst->count = 0;
		return 0;
	}

	rst->reset_tbl = devm_kcalloc(&pdev->dev, num_clocks,
			sizeof(*rst->reset_tbl), GFP_KERNEL);
	if (!rst->reset_tbl)
		return -ENOMEM;

	rst->count = num_clocks;
	d_vpr_h("Found %d reset clocks\n", num_clocks);

	for (c = 0; c < num_clocks; ++c) {
		struct reset_info *rc = &dt->reset_set.reset_tbl[c];

		of_property_read_string_index(pdev->dev.of_node,
				"reset-names", c, &rc->name);
	}

	return 0;
}

static int msm_decide_dt_node(struct msm_vidc_core *core)
{
	int rc = 0;
	struct platform_device *pdev = core->pdev;
	u32 sku_index = 0;

	rc = of_property_read_u32(pdev->dev.of_node, "sku-index",
			&sku_index);
	if (rc) {
		d_vpr_h("'sku_index' not found in node\n");
		return 0;
	}

	return 0;
}

static int msm_vidc_read_resources_from_dt(struct platform_device *pdev)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct msm_vidc_dt *dt;
	struct resource *kres;

	if (!pdev) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core = dev_get_drvdata(&pdev->dev);
	if (!core || !core->dt) {
		d_vpr_e("%s: core not found in device %s",
				dev_name(&pdev->dev));
		return -EINVAL;
	}
	dt = core->dt;

	rc = msm_decide_dt_node(core);
	if (rc)
		return rc;

	INIT_LIST_HEAD(&dt->context_banks);

	kres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dt->register_base = kres ? kres->start : -1;
	dt->register_size = kres ? (kres->end + 1 - kres->start) : -1;
	d_vpr_h("%s: register base %pa, size %#x\n",
		__func__, &dt->register_base, dt->register_size);

	kres = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	dt->irq = kres ? kres->start : -1;
	d_vpr_h("%s: irq %d\n", __func__, dt->irq);

	rc = msm_vidc_load_fw_name(core);
	if (rc)
		d_vpr_e("%s: failed to load fw name, rc %d, using default fw\n",
			__func__, rc);

	rc = msm_vidc_load_subcache_info(core);
	if (rc)
		d_vpr_e("Failed to load subcache info: %d\n", rc);

	rc = msm_vidc_load_qdss_table(core);
	if (rc)
		d_vpr_e("Failed to load qdss reg table: %d\n", rc);

	rc = msm_vidc_load_reg_table(core);
	if (rc) {
		d_vpr_e("Failed to load reg table: %d\n", rc);
		goto err_load_reg_table;
	}

	// TODO: move this table to platform
	rc = msm_vidc_load_buffer_usage_table(core);
	if (rc) {
		d_vpr_e("Failed to load buffer usage table: %d\n", rc);
		goto err_load_buffer_usage_table;
	}

	rc = msm_vidc_load_regulator_table(core);
	if (rc) {
		d_vpr_e("Failed to load list of regulators %d\n", rc);
		goto err_load_regulator_table;
	}

	rc = msm_vidc_load_bus_table(core);
	if (rc) {
		d_vpr_e("Failed to load bus table: %d\n", rc);
		goto err_load_bus_table;
	}

	rc = msm_vidc_load_clock_table(core);
	if (rc) {
		d_vpr_e("Failed to load clock table: %d\n", rc);
		goto err_load_clock_table;
	}

	// TODO: move this table to platform
	rc = msm_vidc_load_allowed_clocks_table(core);
	if (rc) {
		d_vpr_e("Failed to load allowed clocks table: %d\n", rc);
		goto err_load_allowed_clocks_table;
	}

	rc = msm_vidc_load_reset_table(core);
	if (rc) {
		d_vpr_e("Failed to load reset table: %d\n", rc);
		goto err_load_reset_table;
	}

	return rc;

err_load_reset_table:
	msm_vidc_free_allowed_clocks_table(core->dt);
err_load_allowed_clocks_table:
	msm_vidc_free_clock_table(core->dt);
err_load_clock_table:
	msm_vidc_free_bus_table(core->dt);
err_load_bus_table:
	msm_vidc_free_regulator_table(core->dt);
err_load_regulator_table:
	msm_vidc_free_buffer_usage_table(core->dt);
err_load_buffer_usage_table:
	msm_vidc_free_reg_table(core->dt);
err_load_reg_table:
	return rc;
}

static int msm_vidc_setup_context_bank(struct msm_vidc_core *core,
		struct context_bank_info *cb, struct device *dev)
{
	int rc = 0;
	struct bus_type *bus;

	if (!core || !dev || !cb) {
		d_vpr_e("%s: Invalid Input params\n", __func__);
		return -EINVAL;
	}
	cb->dev = dev;

	bus = cb->dev->bus;
	if (IS_ERR_OR_NULL(bus)) {
		d_vpr_e("%s: failed to get bus type\n", __func__);
		rc = PTR_ERR(bus) ? PTR_ERR(bus) : -ENODEV;
		goto remove_cb;
	}

	cb->domain = iommu_get_domain_for_dev(cb->dev);

	/*
	 * configure device segment size and segment boundary to ensure
	 * iommu mapping returns one mapping (which is required for partial
	 * cache operations)
	 */
	if (!dev->dma_parms)
		dev->dma_parms =
			devm_kzalloc(dev, sizeof(*dev->dma_parms), GFP_KERNEL);
	dma_set_max_seg_size(dev, (unsigned int)DMA_BIT_MASK(32));
	dma_set_seg_boundary(dev, (unsigned long)DMA_BIT_MASK(64));

	d_vpr_h("Attached %s and created mapping\n", dev_name(dev));
	d_vpr_h(
		"Context bank: %s, buffer_type: %#x, is_secure: %d, address range start: %#x, size: %#x, dev: %pK, domain: %pK",
		cb->name, cb->buffer_type, cb->is_secure, cb->addr_range.start,
		cb->addr_range.size, cb->dev, cb->domain);

remove_cb:
	return rc;
}

static int msm_vidc_populate_context_bank(struct device *dev,
		struct msm_vidc_core *core)
{
	int rc = 0;
	struct context_bank_info *cb = NULL;
	struct device_node *np = NULL;

	if (!dev || !core || !core->dt) {
		d_vpr_e("%s: invalid inputs\n", __func__);
		return -EINVAL;
	}

	np = dev->of_node;
	cb = devm_kzalloc(dev, sizeof(*cb), GFP_KERNEL);
	if (!cb) {
		d_vpr_e("%s: Failed to allocate cb\n", __func__);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&cb->list);
	list_add_tail(&cb->list, &core->dt->context_banks);

	rc = of_property_read_string(np, "label", &cb->name);
	if (rc) {
		d_vpr_h("Failed to read cb label from device tree\n");
		rc = 0;
	}

	d_vpr_h("%s: context bank has name %s\n", __func__, cb->name);
	rc = of_property_read_u32_array(np, "virtual-addr-pool",
			(u32 *)&cb->addr_range, 2);
	if (rc) {
		d_vpr_e("Could not read addr pool: context bank: %s %d\n",
			cb->name, rc);
		goto err_setup_cb;
	}

	cb->is_secure = of_property_read_bool(np, "qcom,secure-context-bank");
	d_vpr_h("context bank %s: secure = %d\n",
			cb->name, cb->is_secure);

	/* setup buffer type for each sub device*/
	rc = of_property_read_u32(np, "buffer-types", &cb->buffer_type);
	if (rc) {
		d_vpr_e("failed to load buffer_type info %d\n", rc);
		rc = -ENOENT;
		goto err_setup_cb;
	}
	d_vpr_h("context bank %s address start %x size %x buffer_type %x\n",
		cb->name, cb->addr_range.start,
		cb->addr_range.size, cb->buffer_type);

	rc = msm_vidc_setup_context_bank(core, cb, dev);
	if (rc) {
		d_vpr_e("Cannot setup context bank %d\n", rc);
		goto err_setup_cb;
	}

	iommu_set_fault_handler(cb->domain,
		msm_vidc_smmu_fault_handler, (void *)core);

	return 0;

err_setup_cb:
	list_del(&cb->list);
	return rc;
}

int msm_vidc_read_context_bank_resources_from_dt(struct platform_device *pdev)
{
	struct msm_vidc_core *core;
	int rc = 0;

	if (!pdev) {
		d_vpr_e("Invalid platform device\n");
		return -EINVAL;
	} else if (!pdev->dev.parent) {
		d_vpr_e("Failed to find a parent for %s\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}

	core = dev_get_drvdata(pdev->dev.parent);
	if (!core) {
		d_vpr_e("Failed to find cookie in parent device %s",
				dev_name(pdev->dev.parent));
		return -EINVAL;
	}

	rc = msm_vidc_populate_context_bank(&pdev->dev, core);
	if (rc)
		d_vpr_e("Failed to probe context bank\n");
	else
		d_vpr_h("Successfully probed context bank\n");

	return rc;
}

void msm_vidc_deinit_dt(struct platform_device *pdev)
{
	struct msm_vidc_core *core;

	if (!pdev) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	core = dev_get_drvdata(&pdev->dev);
	if (!core) {
		d_vpr_e("%s: core not found in device %s",
				dev_name(&pdev->dev));
		return;
	} else if (!core->dt) {
		d_vpr_e("%s: invalid dt in device %s",
				dev_name(&pdev->dev));
		return;
	}

	msm_vidc_free_clock_table(core->dt);
	msm_vidc_free_regulator_table(core->dt);
	msm_vidc_free_allowed_clocks_table(core->dt);
	msm_vidc_free_reg_table(core->dt);
	msm_vidc_free_qdss_addr_table(core->dt);
	msm_vidc_free_bus_table(core->dt);
	msm_vidc_free_buffer_usage_table(core->dt);
}

int msm_vidc_init_dt(struct platform_device *pdev)
{
	int rc = 0;
	struct msm_vidc_dt *dt;
	struct msm_vidc_core *core;

	if (!pdev) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core = dev_get_drvdata(&pdev->dev);
	if (!core) {
		d_vpr_e("%s: core not found in device %s",
				dev_name(&pdev->dev));
		return -EINVAL;
	}

	dt = kzalloc(sizeof(struct msm_vidc_dt), GFP_KERNEL);
	if (!dt)
		return -ENOMEM;

	core->dt = dt;
	dt->core = core;

	rc = msm_vidc_read_resources_from_dt(pdev);
	if (rc)
		return rc;

	return 0;
}