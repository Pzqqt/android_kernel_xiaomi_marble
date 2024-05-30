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
 * DOC: wlan_hdd_sysfs_txrx_stats
 *
 * implementation for creating sysfs files:
 *
 * txrx_stats
 */

int
hdd_sysfs_dp_txrx_stats_sysfs_create(struct kobject *drv_kobj);

void
hdd_sysfs_dp_txrx_stats_sysfs_destroy(struct kobject *drv_kobj);

#ifdef WLAN_SYSFS_DP_STATS

/**
 * hdd_sysfs_dp_txrx_stats_sysfs_create() - API to create txrx stats related
 * sysfs entry.
 * @drv_kobj: sysfs driver kobject
 *
 * file path: /sys/kernel/wifi/txrx_stats
 *
 * Return: 0 on success and errno on failure
 */

int
hdd_sysfs_dp_txrx_stats_sysfs_create(struct kobject *drv_kobj);

/**
 * hdd_sysfs_dp_txrx_stats_sysfs_destroy() - API to destroy txrx stats
 * related sysfs entry.
 * @drv_kobj: sysfs driver kobject
 *
 * Return: None
 */

void
hdd_sysfs_dp_txrx_stats_sysfs_destroy(struct kobject *drv_kobj);

#else /* WLAN_SYSFS_DP_STATS */

int
hdd_sysfs_dp_txrx_stats_sysfs_create(struct kobject *drv_kobj)
{
	return 0;
}

void
hdd_sysfs_dp_txrx_stats_sysfs_destroy(struct kobject *drv_kobj)
{
}

#endif /* WLAN_SYSFS_DP_STATS */
