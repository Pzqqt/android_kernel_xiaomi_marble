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

#ifndef _WLAN_HDD_SYSFS_H_
#define _WLAN_HDD_SYSFS_H_

#ifdef WLAN_SYSFS
/**
 * hdd_sysfs_create_driver_root_obj() - create driver root kobject
 *
 * Return: none
 */
void hdd_sysfs_create_driver_root_obj(void);

/**
 * hdd_sysfs_destroy_driver_root_obj() - destroy driver root kobject
 *
 * Return: none
 */
void hdd_sysfs_destroy_driver_root_obj(void);

/**
 * hdd_sysfs_create_version_interface() - create version interface
 * @psoc: PSOC ptr
 *
 * Return: none
 */
void hdd_sysfs_create_version_interface(struct wlan_objmgr_psoc *psoc);

/**
 * hdd_sysfs_destroy_version_interface() - destroy version interface
 *
 * Return: none
 */
void hdd_sysfs_destroy_version_interface(void);
/**
 * hdd_sysfs_create_powerstats_interface() - create power_stats interface
 *
 * Return: none
 */
void hdd_sysfs_create_powerstats_interface(void);
/**
 * hdd_sysfs_destroy_powerstats_interface() - destroy power_stats interface
 *
 * Return: none
 */
void hdd_sysfs_destroy_powerstats_interface(void);
#else
static inline
void hdd_sysfs_create_driver_root_obj(void)
{
}

static inline
void hdd_sysfs_destroy_driver_root_obj(void)
{
}

static inline
void hdd_sysfs_create_version_interface(struct wlan_objmgr_psoc *psoc)
{
}

static inline
void hdd_sysfs_destroy_version_interface(void)
{
}

static inline
void hdd_sysfs_create_powerstats_interface(void)
{
}

static inline
void hdd_sysfs_destroy_powerstats_interface(void)
{
}
#endif

#ifdef WLAN_FEATURE_BEACON_RECEPTION_STATS
/**
 * hdd_sysfs_create_adapter_root_obj() - create adapter sysfs entries
 * @adapter: HDD adapter
 *
 * Return: none
 */
void hdd_sysfs_create_adapter_root_obj(struct hdd_adapter *adapter);
/**
 * hdd_sysfs_destroy_adapter_root_obj() - Destroy adapter sysfs entries
 * @adapter: HDD adapter
 *
 * Return: none
 */
void hdd_sysfs_destroy_adapter_root_obj(struct hdd_adapter *adapter);
#else
static inline
void hdd_sysfs_create_adapter_root_obj(struct hdd_adapter *adapter)
{
}

static inline
void hdd_sysfs_destroy_adapter_root_obj(struct hdd_adapter *adapter)
{
}
#endif
#endif
