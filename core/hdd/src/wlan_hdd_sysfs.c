/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: wlan_hdd_sysfs.c
 *
 *  WLAN Host Device Driver implementation
 *
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/string.h>
#include "wlan_hdd_includes.h"
#include "wlan_hdd_sysfs.h"
#include "qwlan_version.h"
#include "cds_api.h"

#define MAX_PSOC_ID_SIZE 10

#ifdef MULTI_IF_NAME
#define DRIVER_NAME MULTI_IF_NAME
#else
#define DRIVER_NAME "wlan"
#endif

static struct kobject *wlan_kobject;
static struct kobject *driver_kobject;
static struct kobject *fw_kobject;
static struct kobject *psoc_kobject;

static ssize_t __show_driver_version(struct kobject *kobj,
				     struct kobj_attribute *attr,
				     char *buf)
{
	return scnprintf(buf, PAGE_SIZE, QWLAN_VERSIONSTR);
}

static ssize_t show_driver_version(struct kobject *kobj,
				   struct kobj_attribute *attr,
				   char *buf)
{
	ssize_t ret_val;

	cds_ssr_protect(__func__);
	ret_val = __show_driver_version(kobj, attr, buf);
	cds_ssr_unprotect(__func__);

	return ret_val;
}

static ssize_t __show_fw_version(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 char *buf)
{
	uint32_t major_spid = 0, minor_spid = 0, siid = 0, crmid = 0;
	uint32_t sub_id = 0;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	int ret;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret) {
		hdd_err("hdd ctx is invalid");
		return ret;
	}

	hdd_debug("Rcvd req for FW version");
	hdd_get_fw_version(hdd_ctx, &major_spid, &minor_spid, &siid,
			   &crmid);
	sub_id = (hdd_ctx->target_fw_vers_ext & 0xf0000000) >> 28;

	return scnprintf(buf, PAGE_SIZE,
			 "FW:%d.%d.%d.%d.%d HW:%s Board version: %x Ref design id: %x Customer id: %x Project id: %x Board Data Rev: %x\n",
			 major_spid, minor_spid, siid, crmid, sub_id,
			 hdd_ctx->target_hw_name,
			 hdd_ctx->hw_bd_info.bdf_version,
			 hdd_ctx->hw_bd_info.ref_design_id,
			 hdd_ctx->hw_bd_info.customer_id,
			 hdd_ctx->hw_bd_info.project_id,
			 hdd_ctx->hw_bd_info.board_data_rev);
}

static ssize_t show_fw_version(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       char *buf)
{
	ssize_t ret_val;

	cds_ssr_protect(__func__);
	ret_val = __show_fw_version(kobj, attr, buf);
	cds_ssr_unprotect(__func__);

	return ret_val;
}

static struct kobj_attribute dr_ver_attribute =
	__ATTR(driver_version, 0440, show_driver_version, NULL);
static struct kobj_attribute fw_ver_attribute =
	__ATTR(version, 0440, show_fw_version, NULL);

void hdd_sysfs_create_version_interface(struct wlan_objmgr_psoc *psoc)
{
	int error = 0;
	uint32_t psoc_id;
	char buf[MAX_PSOC_ID_SIZE];

	wlan_kobject = kobject_create_and_add("wifi", kernel_kobj);
	if (!wlan_kobject) {
		hdd_err("could not allocate wlan kobject");
		return;
	}

	driver_kobject = kobject_create_and_add(DRIVER_NAME, wlan_kobject);
	if (!driver_kobject) {
		hdd_err("could not allocate driver kobject");
		goto free_wlan_kobj;
	}

	error = sysfs_create_file(driver_kobject, &dr_ver_attribute.attr);
	if (error) {
		hdd_err("could not create driver sysfs file");
		goto free_drv_kobj;
	}

	fw_kobject = kobject_create_and_add("fw", driver_kobject);
	if (!fw_kobject) {
		hdd_err("could not allocate fw kobject");
		goto free_fw_kobj;
	}

	psoc_id = wlan_psoc_get_nif_phy_version(psoc);
	scnprintf(buf, PAGE_SIZE, "%d", psoc_id);

	psoc_kobject = kobject_create_and_add(buf, fw_kobject);
	if (!psoc_kobject) {
		hdd_err("could not allocate psoc kobject");
		goto free_fw_kobj;
	}

	error = sysfs_create_file(psoc_kobject, &fw_ver_attribute.attr);
	if (error) {
		hdd_err("could not create fw sysfs file");
		goto free_psoc_kobj;
	}

	return;

free_psoc_kobj:
	kobject_put(psoc_kobject);
	psoc_kobject = NULL;

free_fw_kobj:
	kobject_put(fw_kobject);
	fw_kobject = NULL;

free_drv_kobj:
	kobject_put(driver_kobject);
	driver_kobject = NULL;

free_wlan_kobj:
	kobject_put(wlan_kobject);
	wlan_kobject = NULL;
}

void hdd_sysfs_destroy_version_interface(void)
{
	if (psoc_kobject) {
		kobject_put(psoc_kobject);
		psoc_kobject = NULL;
		kobject_put(fw_kobject);
		fw_kobject = NULL;
		kobject_put(driver_kobject);
		driver_kobject = NULL;
		kobject_put(wlan_kobject);
		wlan_kobject = NULL;
	}
}
