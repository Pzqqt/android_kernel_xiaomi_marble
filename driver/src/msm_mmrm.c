// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/soc/qcom/msm_mmrm.h>
#include <linux/fs.h>

#include "mmrm_internal.h"
#include "mmrm_debug.h"
#include "mmrm_clk_rsrc_mgr.h"

#define	VERIFY_PDEV(pdev)	\
{							\
	if (!pdev) {			\
		d_mpr_e("%s: null platform dev\n", __func__);\
		rc = -EINVAL;		\
		goto err_exit; \
	}						\
}

#define RESET_DRV_DATA(drv_data)	\
{									\
	kfree(drv_data);				\
	drv_data = (void *) -EPROBE_DEFER; \
}

#define	MMRM_SYSFS_ENTRY_MAX_LEN PAGE_SIZE

extern int msm_mmrm_debug;
extern u8 msm_mmrm_enable_throttle_feature;
extern u8 msm_mmrm_allow_multiple_register;

struct mmrm_driver_data *drv_data = (void *) -EPROBE_DEFER;

struct mmrm_client *mmrm_client_register(struct mmrm_client_desc *client_desc)
{
	struct mmrm_client *client = NULL;



	/* check for null input */
	if (!client_desc) {
		d_mpr_e("%s: null input descriptor\n", __func__);
		goto err_exit;
	}

	if (drv_data == (void *) -EPROBE_DEFER) {
		d_mpr_e("%s: mmrm probe_init not done\n", __func__);
		goto err_exit;
	}

	/* check for client type, then register */
	if (client_desc->client_type == MMRM_CLIENT_CLOCK) {
		client = mmrm_clk_client_register(
					drv_data->clk_mgr, client_desc);
		if (!client) {
			d_mpr_e("%s: failed to register client\n", __func__);
			goto err_exit;
		}
	} else {
		d_mpr_e("%s: unknown client_type %d\n",
			__func__, client_desc->client_type);
		goto err_exit;
	}

	return client;

err_exit:
	d_mpr_e("%s: error exit\n", __func__);
	return client;
}
EXPORT_SYMBOL(mmrm_client_register);

int mmrm_client_deregister(struct mmrm_client *client)
{
	int rc = 0;


	/* check for null input */
	if (!client) {
		d_mpr_e("%s: invalid input client\n", __func__);
		rc = -EINVAL;
		goto err_exit;
	}

	if (drv_data == (void *) -EPROBE_DEFER) {
		d_mpr_e("%s: mmrm probe_init not done\n", __func__);
		goto err_exit;
	}

	/* check for client type, then deregister */
	if (client->client_type == MMRM_CLIENT_CLOCK) {
		rc = mmrm_clk_client_deregister(drv_data->clk_mgr, client);
		if (rc != 0) {
			d_mpr_e("%s: failed to deregister client\n", __func__);
			goto err_exit;
		}
	} else {
		d_mpr_e("%s: unknown client_type %d\n",
			__func__, client->client_type);
	}

	return rc;

err_exit:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}
EXPORT_SYMBOL(mmrm_client_deregister);

int mmrm_client_set_value(struct mmrm_client *client,
	struct mmrm_client_data *client_data, unsigned long val)
{
	int rc = 0;

	/* check for null input */
	if (!client || !client_data) {
		d_mpr_e("%s: invalid input client(%pK) client_data(%pK)\n",
			__func__, client, client_data);
		rc = -EINVAL;
		goto err_exit;
	}

	if (drv_data == (void *) -EPROBE_DEFER) {
		d_mpr_e("%s: mmrm probe_init not done\n", __func__);
		goto err_exit;
	}

	/* check for client type, then set value */
	if (client->client_type == MMRM_CLIENT_CLOCK) {
		rc = mmrm_clk_client_setval(drv_data->clk_mgr, client,
				client_data, val);
		if (rc != 0) {
			d_mpr_e("%s: failed to set value for client\n", __func__);
			goto err_exit;
		}
	} else {
		d_mpr_e("%s: unknown client_type %d\n",
			__func__, client->client_type);
	}

	return rc;

err_exit:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}
EXPORT_SYMBOL(mmrm_client_set_value);

int mmrm_client_set_value_in_range(struct mmrm_client *client,
	struct mmrm_client_data *client_data,
	struct mmrm_client_res_value *val)
{
	int rc = 0;

	/* check for null input */
	if (!client || !client_data || !val) {
		d_mpr_e(
			"%s: invalid input client(%pK) client_data(%pK) val(%pK)\n",
			__func__, client, client_data, val);
		rc = -EINVAL;
		goto err_exit;
	}

	if (drv_data == (void *) -EPROBE_DEFER) {
		d_mpr_e("%s: mmrm probe_init not done\n", __func__);
		goto err_exit;
	}

	/* check for client type, then set value */
	if (client->client_type == MMRM_CLIENT_CLOCK) {
		rc = mmrm_clk_client_setval_inrange(drv_data->clk_mgr,
				client, client_data, val);
		if (rc != 0) {
			d_mpr_e("%s: failed to set value for client\n", __func__);
			goto err_exit;
		}
	} else {
		d_mpr_e("%s: unknown client_type %d\n",
			__func__, client->client_type);
	}

	return rc;

err_exit:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}
EXPORT_SYMBOL(mmrm_client_set_value_in_range);

int mmrm_client_get_value(struct mmrm_client *client,
	struct mmrm_client_res_value *val)
{
	int rc = 0;

	/* check for null input */
	if (!client || !val) {
		d_mpr_e("%s: invalid input client(%pK) val(%pK)\n",
			__func__, client, val);
		rc = -EINVAL;
		goto err_exit;
	}

	if (drv_data == (void *) -EPROBE_DEFER) {
		d_mpr_e("%s: mmrm probe_init not done\n", __func__);
		goto err_exit;
	}

	/* check for client type, then get value */
	if (client->client_type == MMRM_CLIENT_CLOCK) {
		rc = mmrm_clk_client_getval(drv_data->clk_mgr,
				client, val);
		if (rc != 0) {
			d_mpr_e("%s: failed to get value for client\n", __func__);
			goto err_exit;
		}
	} else {
		d_mpr_e("%s: unknown client_type %d\n",
			__func__, client->client_type);
	}

	return rc;

err_exit:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}
EXPORT_SYMBOL(mmrm_client_get_value);

static int sysfs_get_param(const char *buf, u32 *param)
{
	int base;

	if (buf) {
		if ((buf[1] == 'x') || (buf[1] == 'X'))
			base = 16;
		else
			base = 10;

		if (kstrtou32(buf, base, param) != 0)
			return -EINVAL;
	}
	return 0;
}

static ssize_t mmrm_sysfs_debug_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;

	ret = scnprintf(buf, MMRM_SYSFS_ENTRY_MAX_LEN, "0x%x\n", msm_mmrm_debug);
	pr_info("%s: 0x%04X\n", __func__, msm_mmrm_debug);

	return ret;
}

static ssize_t mmrm_sysfs_debug_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	u32 reg_addr;

	ret = sysfs_get_param(buf, &reg_addr);
	if (ret == 0)
		msm_mmrm_debug = reg_addr;

	return count;
}

static ssize_t mmrm_sysfs_enable_throttle_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;

	ret = scnprintf(buf, MMRM_SYSFS_ENTRY_MAX_LEN, "0x%x\n", msm_mmrm_enable_throttle_feature);
	pr_info("%s: 0x%04X\n", __func__, msm_mmrm_enable_throttle_feature);

	return ret;
}

static ssize_t mmrm_sysfs_enable_throttle_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u32 reg_addr;
	int ret;

	ret = sysfs_get_param(buf, &reg_addr);
	if (ret == 0)
		msm_mmrm_enable_throttle_feature = (u8)reg_addr;

	return count;
}

static ssize_t mmrm_sysfs_allow_multiple_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;

	ret = scnprintf(buf, MMRM_SYSFS_ENTRY_MAX_LEN, "0x%x\n", msm_mmrm_allow_multiple_register);
	pr_info("%s: 0x%04X\n", __func__, msm_mmrm_allow_multiple_register);

	return ret;
}

static ssize_t mmrm_sysfs_allow_multiple_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u32 reg_addr;
	int ret;

	ret = sysfs_get_param(buf, &reg_addr);
	if (ret == 0)
		msm_mmrm_allow_multiple_register = (u8)reg_addr;

	return count;
}


static ssize_t dump_enabled_client_info_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc;

	rc = mmrm_clk_print_enabled_client_info(drv_data->clk_mgr, buf, MMRM_SYSFS_ENTRY_MAX_LEN);
	if (rc == 0)
		d_mpr_e("%s: failed to dump client info\n", __func__);

	return rc;
}

static DEVICE_ATTR(debug, 0644,
		mmrm_sysfs_debug_get,
		mmrm_sysfs_debug_set);

static DEVICE_ATTR(enable_throttle_feature, 0644,
		mmrm_sysfs_enable_throttle_get,
		mmrm_sysfs_enable_throttle_set);

static DEVICE_ATTR(allow_multiple_register, 0644,
		mmrm_sysfs_allow_multiple_get,
		mmrm_sysfs_allow_multiple_set);

static DEVICE_ATTR_RO(dump_enabled_client_info);


static struct attribute *mmrm_fs_attrs[] = {
		&dev_attr_debug.attr,
		&dev_attr_enable_throttle_feature.attr,
		&dev_attr_allow_multiple_register.attr,
		&dev_attr_dump_enabled_client_info.attr,
		NULL,
};

static struct attribute_group mmrm_fs_attrs_group = {
		.attrs = mmrm_fs_attrs,
};

static int msm_mmrm_probe_init(struct platform_device *pdev)
{
	int rc = 0;

	drv_data = kzalloc(sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data) {
		d_mpr_e("%s: unable to allocate memory for mmrm driver\n",
			__func__);
		rc = -ENOMEM;
		goto err_no_mem;
	}

	drv_data->platform_data = mmrm_get_platform_data(&pdev->dev);
	if (!drv_data->platform_data) {
		d_mpr_e("%s: unable to get platform data\n",
			__func__);
		rc = -EINVAL;
		goto err_get_drv_data;
	}

	drv_data->debugfs_root = msm_mmrm_debugfs_init();
	if (!drv_data->debugfs_root)
		d_mpr_e("%s: failed to create debugfs for mmrm\n", __func__);

	dev_set_drvdata(&pdev->dev, drv_data);

	rc = mmrm_read_platform_resources(pdev, drv_data);
	if (rc) {
		d_mpr_e("%s: unable to read platform resources for mmrm\n",
			__func__);
		goto err_read_pltfrm_rsc;
	}

	rc = mmrm_init(drv_data);
	if (rc) {
		d_mpr_e("%s: failed to init mmrm\n",
			__func__);
		goto err_mmrm_init;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &mmrm_fs_attrs_group)) {
		d_mpr_e("%s: failed to create sysfs\n",
			__func__);
	}

	return rc;

err_mmrm_init:
	msm_mmrm_debugfs_deinit(drv_data->debugfs_root);
err_read_pltfrm_rsc:
	mmrm_free_platform_resources(drv_data);
err_get_drv_data:
	RESET_DRV_DATA(drv_data);
err_no_mem:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}

static int msm_mmrm_probe(struct platform_device *pdev)
{
	int rc = -EINVAL;

	d_mpr_h("%s\n", __func__);

	VERIFY_PDEV(pdev)

	if (of_device_is_compatible(pdev->dev.of_node, "qcom,msm-mmrm"))
		return msm_mmrm_probe_init(pdev);

	d_mpr_e("%s: no compatible device node\n", __func__);
	return rc;

err_exit:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}

static int msm_mmrm_remove(struct platform_device *pdev)
{
	int rc = 0;

	VERIFY_PDEV(pdev);

	drv_data = dev_get_drvdata(&pdev->dev);
	if (!drv_data) {
		d_mpr_e("%s: null driver data\n", __func__);
		return -EINVAL;
	}

	sysfs_remove_group(&pdev->dev.kobj, &mmrm_fs_attrs_group);
	msm_mmrm_debugfs_deinit(drv_data->debugfs_root);
	mmrm_deinit(drv_data);
	mmrm_free_platform_resources(drv_data);
	dev_set_drvdata(&pdev->dev, NULL);
	RESET_DRV_DATA(drv_data);

	return rc;

err_exit:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}

static const struct of_device_id msm_mmrm_dt_match[] = {
	{.compatible = "qcom,msm-mmrm"},
	{}
};

MODULE_DEVICE_TABLE(of, msm_mmrm_dt_match);

static struct platform_driver msm_mmrm_driver = {
	.probe = msm_mmrm_probe,
	.remove = msm_mmrm_remove,
	.driver = {
		.name = "msm-mmrm",
		.of_match_table = msm_mmrm_dt_match,
	},
};

static int __init msm_mmrm_init(void)
{
	int rc = 0;

	rc = platform_driver_register(&msm_mmrm_driver);
	if (rc) {
		d_mpr_e("%s: failed to register platform driver\n",
			__func__);
		goto err_platform_drv_reg;
	}

	d_mpr_h("%s: success\n", __func__);
	return rc;

err_platform_drv_reg:
	d_mpr_e("%s: error = %d\n", __func__, rc);
	return rc;
}

static void __exit msm_mmrm_exit(void)
{
	platform_driver_unregister(&msm_mmrm_driver);
}

module_init(msm_mmrm_init);
module_exit(msm_mmrm_exit);

MODULE_DESCRIPTION("QTI MMRM Driver");
MODULE_LICENSE("GPL v2");
