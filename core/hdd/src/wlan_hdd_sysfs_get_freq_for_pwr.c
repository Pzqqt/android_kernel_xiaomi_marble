/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wlan_hdd_sysfs_get_freq_for_pwr.c
 *
 * implementation for creating sysfs file valid_freq
 */

#include <wlan_hdd_includes.h>
#include "osif_vdev_sync.h"
#include <wlan_hdd_sysfs.h>
#include "wlan_hdd_sysfs_get_freq_for_pwr.h"
#include "osif_psoc_sync.h"
#include "reg_services_public_struct.h"
#include <wma_api.h>

static ssize_t
__hdd_sysfs_power_level_store(struct hdd_context *hdd_ctx,
			      char const *buf, size_t count)
{
	char buf_local[MAX_SYSFS_USER_COMMAND_SIZE_LENGTH + 1];
	char *sptr, *token;
	int ret;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = hdd_sysfs_validate_and_copy_buf(buf_local, sizeof(buf_local),
					      buf, count);
	if (ret) {
		hdd_err_rl("invalid input");
		return ret;
	}
	sptr = buf_local;
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;

	if (!strncasecmp(token, "VLP", strlen("VLP")))
		hdd_ctx->power_type = REG_VERY_LOW_POWER_AP;
	else if (!strncasecmp(token, "LP", strlen("LP")))
		hdd_ctx->power_type = REG_INDOOR_AP;
	else if (!strncasecmp(token, "SP", strlen("SP")))
		hdd_ctx->power_type = REG_STANDARD_POWER_AP;
	else
		hdd_ctx->power_type = REG_MAX_SUPP_AP_TYPE;

	hdd_debug("power level %s(%d)", token,
		  hdd_ctx->power_type);

	return count;
}

static ssize_t
wlan_hdd_sysfs_power_store(struct kobject *kobj,
			   struct kobj_attribute *attr,
			   char const *buf, size_t count)
{
	struct osif_psoc_sync *psoc_sync;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	ssize_t err_size;

	if (wlan_hdd_validate_context(hdd_ctx))
		return 0;

	err_size = osif_psoc_sync_op_start(wiphy_dev(hdd_ctx->wiphy),
					   &psoc_sync);
	if (err_size)
		return err_size;

	err_size = __hdd_sysfs_power_level_store(hdd_ctx, buf, count);
	osif_psoc_sync_op_stop(psoc_sync);

	return err_size;
}

static ssize_t
__wlan_hdd_sysfs_freq_show(struct hdd_context *hdd_ctx,
			   struct kobj_attribute *attr, char *buf)
{
	int ret = 0;
	struct regulatory_channel chan_list[NUM_6GHZ_CHANNELS];
	QDF_STATUS status;
	uint32_t i;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = scnprintf(buf, PAGE_SIZE, "freq list for power type %s\n",
			wlan_reg_get_power_string(hdd_ctx->power_type));

	if (!strcmp(wlan_reg_get_power_string(hdd_ctx->power_type), "INVALID"))
		return -EINVAL;

	status = wlan_reg_get_6g_ap_master_chan_list(
						hdd_ctx->pdev,
						hdd_ctx->power_type,
						chan_list);

	for (i = 0; i < NUM_6GHZ_CHANNELS; i++) {
		if ((chan_list[i].state != CHANNEL_STATE_DISABLE) &&
		    !(chan_list[i].chan_flags & REGULATORY_CHAN_DISABLED)) {
			if ((PAGE_SIZE - ret) <= 0)
				return ret;
			ret += scnprintf(buf + ret, PAGE_SIZE - ret,
					"%d  ", chan_list[i].center_freq);
		}
	}
	return ret;
}

static ssize_t wlan_hdd_sysfs_freq_show(struct kobject *kobj,
					struct kobj_attribute *attr,
					char *buf)
{
	struct osif_psoc_sync *psoc_sync;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	ssize_t err_size;
	int ret;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	err_size = osif_psoc_sync_op_start(wiphy_dev(hdd_ctx->wiphy),
					   &psoc_sync);
	if (err_size)
		return err_size;

	err_size = __wlan_hdd_sysfs_freq_show(hdd_ctx, attr, buf);

	osif_psoc_sync_op_stop(psoc_sync);

	return err_size;
}

static struct kobj_attribute valid_freq_attribute =
__ATTR(valid_freq, 0664, wlan_hdd_sysfs_freq_show, wlan_hdd_sysfs_power_store);

int hdd_sysfs_get_valid_freq_for_power_create(struct kobject *driver_kobject)
{
	int error;

	error = sysfs_create_file(driver_kobject, &valid_freq_attribute.attr);
	if (error)
		hdd_err("could not create valid_freq sysfs file");

	return error;
}

void
hdd_sysfs_get_valid_freq_for_power_destroy(struct kobject *driver_kobject)
{
	if (!driver_kobject) {
		hdd_err("could not get driver kobject!");
		return;
	}
	sysfs_remove_file(driver_kobject, &valid_freq_attribute.attr);
}
