/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_sysfs_get_freq_for_pwr.h
 *
 * implementation for creating sysfs file valid_freq that contains
 * valid freq list for provided power type
 */
#ifndef _WLAN_HDD_SYSFS_FREQ_FOR_PWR_H
#define _WLAN_HDD_SYSFS_FREQ_FOR_PWR_H
#if defined(WLAN_SYSFS) && defined(CONFIG_WLAN_FREQ_LIST)
/**
 * hdd_sysfs_get_valid_freq_for_power_create() - API to create get_valid_freq
 * @driver_kobject: sysfs driver kobject
 *
 * file path: /sys/kernel/wifi/valid_freq
 *
 * usage:
 *      echo "VLP"/"LP"/"SP" > valid_freq
 *
 * Return: 0 on success and errno on failure
 */
int hdd_sysfs_get_valid_freq_for_power_create(struct kobject *driver_kobject);

/**
 * hdd_sysfs_get_valid_freq_for_power_destroy() -
 * API to destroy get_valid_freq sysfs
 *
 * Return: none
 */
void
hdd_sysfs_get_valid_freq_for_power_destroy(struct kobject *driver_kobject);
#else
static inline int
hdd_sysfs_get_valid_freq_for_power_create(struct kobject *driver_kobject)
{
	return 0;
}

static inline void
hdd_sysfs_get_valid_freq_for_power_destroy(struct kobject *driver_kobject)
{
}
#endif
#endif
