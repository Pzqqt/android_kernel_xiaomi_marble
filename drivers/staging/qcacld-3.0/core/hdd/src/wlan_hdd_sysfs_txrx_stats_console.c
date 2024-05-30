/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_sysfs_txrx_stats_console.c
 *
 * implementation for creating sysfs files:
 *
 * txrx_stats
 */

#include <wlan_hdd_includes.h>
#include "osif_psoc_sync.h"
#include <wlan_hdd_sysfs.h>
#include <wlan_hdd_sysfs_txrx_stats_console.h>
#include <cds_api.h>
#include <qdf_status.h>

#ifdef WLAN_SYSFS_DP_STATS

#define HDD_WLAN_SYSFS_TXRX_STATS_USER_CMD_SIZE (10)
#define SYSFS_INPUT_BUF_SIZE			PAGE_SIZE

/*
 * __hdd_wlan_txrx_stats_store() - Calls into dp layer to
 * update the sysfs config values.
 * @hdd_ctx: hdd context
 * @buf: input buffer from user space
 * @count: input buffer size
 *
 * Return: Return the size of input buffer.
 */
static ssize_t
__hdd_wlan_txrx_stats_store(struct hdd_context *hdd_ctx,
			    const char  *buf, size_t count)
{
	char buf_local[HDD_WLAN_SYSFS_TXRX_STATS_USER_CMD_SIZE + 1];
	char *sptr, *token;
	uint32_t stat_type_requested;
	uint32_t mac_id;
	int ret;
	ol_txrx_soc_handle soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc) {
		hdd_err_rl("soc is NULL");
		return -EINVAL;
	}

	if (count > HDD_WLAN_SYSFS_TXRX_STATS_USER_CMD_SIZE)
		return -EINVAL;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = hdd_sysfs_validate_and_copy_buf(buf_local, sizeof(buf_local),
					      buf, count);

	if (ret)
		return -EINVAL;

	sptr = buf_local;
	/* get mac id */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &stat_type_requested))
		return -EINVAL;

	/* get stat type requested */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;
	if (kstrtou32(token, 0, &mac_id))
		return -EINVAL;

	if (!soc->ops->cmn_drv_ops->txrx_sysfs_set_stat_type) {
		hdd_err("txrx_sysfs_set_stat_type is NULL");
		return -EINVAL;
	}

	soc->ops->cmn_drv_ops->txrx_sysfs_set_stat_type(soc,
							stat_type_requested,
							mac_id);

	return count;
}

/*
 * __hdd_wlan_txrx_stats_show() - Calls into dp to fill stats.
 * @buf: output buffer
 *
 * Return: output buffer size.
 */
static ssize_t
__hdd_wlan_txrx_stats_show(char *buf)
{
	uint32_t size_of_output = 0;
	ol_txrx_soc_handle soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc) {
		hdd_err_rl("hdd dp_soc cds_get_context returned NULL");
		return -EINVAL;
	}

	if (!soc->ops->cmn_drv_ops->txrx_sysfs_fill_stats) {
		hdd_err_rl("txrx_sysfs_fill_stats is NULL");
		return -EINVAL;
	}
	soc->ops->cmn_drv_ops->txrx_sysfs_fill_stats(soc,
						     buf,
						     SYSFS_INPUT_BUF_SIZE);
	size_of_output = strlen(buf);
	return size_of_output;
}

/*
 * hdd_sysfs_dp_txrx_stats_show() - Registered as sysfs show function.
 * @kobj: kernel object
 * @att: kobject attribute
 * @buf: output buffer to be filled
 *
 * Return: Returns the length of the output buffer.
 */
static ssize_t
hdd_sysfs_dp_txrx_stats_show(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct osif_psoc_sync *psoc_sync = NULL;
	ssize_t length = 0;
	ssize_t errno = 0;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(hdd_ctx->wiphy), &psoc_sync);
	if (errno)
		return errno;

	length = __hdd_wlan_txrx_stats_show(buf);

	osif_psoc_sync_op_stop(psoc_sync);

	return length;
}

/*
 * hdd_sysfs_dp_txrx_stats_store() - Registered as sysfs write function.
 * @kobj: kernel object
 * @att: kobject attribute
 * @buf: input buffer from user space
 * @count:  size of the input buffer
 *
 * Return: Returns the length of the input buffer.
 */
static ssize_t
hdd_sysfs_dp_txrx_stats_store(struct kobject *kobj,
			      struct kobj_attribute *attr,
			      const char *buf,
			      size_t count)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct osif_psoc_sync *psoc_sync = NULL;
	ssize_t errno = 0;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(hdd_ctx->wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __hdd_wlan_txrx_stats_store(hdd_ctx, buf, count);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

static struct kobj_attribute dp_txrx_stats_attribute =
	__ATTR(txrx_stats, 0664,  hdd_sysfs_dp_txrx_stats_show,
	       hdd_sysfs_dp_txrx_stats_store);

/*
 * hdd_sysfs_dp_txrx_stats_sysfs_create() - Initialize sysfs file.
 * @driver_kobject: driver kobject
 *
 * Return: return sysfs int val.
 */
int hdd_sysfs_dp_txrx_stats_sysfs_create(struct kobject *driver_kobject)
{
	int error = 0;

	if (!driver_kobject) {
		hdd_err("could not get driver kobject");
		return -EINVAL;
	}

	error = sysfs_create_file(driver_kobject,
				  &dp_txrx_stats_attribute.attr);

	if (error)
		hdd_err("failed to create txrx_stats sysfs file");

	return error;
}

/*
 * hdd_sysfs_dp_txrx_stats_sysfs_destroy() - Sysfs deinitialize.
 * @driver_kobject: driver kobject
 *
 * Return: void
 */
void
hdd_sysfs_dp_txrx_stats_sysfs_destroy(struct kobject *driver_kobject)
{
	if (!driver_kobject) {
		hdd_err("failed to get driver kobject");
		return;
	}

	sysfs_remove_file(driver_kobject, &dp_txrx_stats_attribute.attr);
}
#endif /* WLAN_SYSFS_DP_STATS */
