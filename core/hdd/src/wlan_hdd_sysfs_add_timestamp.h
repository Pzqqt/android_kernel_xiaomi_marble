/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved..
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
 * DOC: wlan_hdd_sysfs_add_timestamp.h
 *
 * Implementation for creating sysfs files:
 *
 * dp_pkt_add_ts
 */

#ifndef _WLAN_HDD_SYSFS_DP_PKT_ADD_TIMESTAMP_H
#define _WLAN_HDD_SYSFS_DP_PKT_ADD_TIMESTAMP_H

#include <qdf_pkt_add_timestamp.h>

#if defined(WLAN_SYSFS) && defined(CONFIG_DP_PKT_ADD_TIMESTAMP)

/**
 * hdd_sysfs_dp_pkt_add_ts_create() - API to create dp trace related files
 * @driver_kobject: sysfs driver kobject
 *
 * file path: /sys/kernel/wifi/dp_pkt_add_ts
 *
 * usage:
 *      <protocol>:<destination port>:<offset in data>
 *      echo "TCP:5210:50,UDP:5110:50" > dp_pkt_add_ts (add protocol)
 *      echo "" > dp_pkt_add_ts (clear protocol)
 *
 * Return: 0 on success and errno on failure
 */
int hdd_sysfs_dp_pkt_add_ts_create(struct kobject *driver_kobject);

/**
 * hdd_sysfs_dp_pkt_add_ts_destroy() -
 *   API to destroy dp trace related files
 *
 * Return: none
 */
void
hdd_sysfs_dp_pkt_add_ts_destroy(struct kobject *driver_kobject);
#else
static inline int
hdd_sysfs_dp_pkt_add_ts_create(struct kobject *driver_kobject)
{
	return 0;
}

static inline void
hdd_sysfs_dp_pkt_add_ts_destroy(struct kobject *driver_kobject)
{
}
#endif
#endif /* #ifndef _WLAN_HDD_SYSFS_DP_PKT_ADD_TIMESTAMP_H */
