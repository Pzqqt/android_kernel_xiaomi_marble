/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_debugfs_unit_test.h
 *
 * WLAN Host Device Driver implementation to create debugfs
 * unit_test_host/unit_test_target/wlan_suspend/wlan_resume
 */

#ifndef _WLAN_HDD_DEBUGFS_UNIT_TEST_H
#define _WLAN_HDD_DEBUGFS_UNIT_TEST_H

#if defined(WLAN_DEBUGFS) && defined(WLAN_UNIT_TEST)
/**
 * hdd_debugfs_unit_test_host_create() - API to create unit_test_target
 * @hdd_ctx: hdd context
 *
 * this file is created per driver.
 * file path:  /sys/kernel/debug/wlan_xx/unit_test_host
 *                (wlan_xx is driver name)
 * usage:
 *      echo 'all'>unit_test_host
 *      echo 'qdf_periodic_work'>unit_test_host
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_debugfs_unit_test_host_create(struct hdd_context *hdd_ctx);
#else
static inline int
wlan_hdd_debugfs_unit_test_host_create(struct hdd_context *hdd_ctx)
{
	return 0;
}
#endif

#if defined(WLAN_DEBUGFS) && defined(WLAN_SUSPEND_RESUME_TEST)
/**
 * wlan_hdd_debugfs_suspend_create() - API to create wlan_suspend
 * @adapter: hdd adapter
 *
 * this file is created per adapter.
 * file path: /sys/kernel/debug/wlan_xx/wlan_suspend
 *                (wlan_xx is adapter name)
 * usage:
 *      echo [arg_0] [arg_1] > wlan_suspend
 *            arg_0 from enum wow_interface_pause
 *            arg_1 from enum wow_resume_trigger
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_debugfs_suspend_create(struct hdd_adapter *adapter);

/**
 * wlan_hdd_debugfs_resume_create() - API to create wlan_resume
 * @adapter: hdd adapter
 *
 * this file is created per adapter.
 * file path: /sys/kernel/debug/wlan_xx/wlan_resume
 *                (wlan_xx is adapter name)
 * usage:
 *		echo > wlan_resume
 *
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_debugfs_resume_create(struct hdd_adapter *adapter);
#else
static inline int
wlan_hdd_debugfs_suspend_create(struct hdd_adapter *adapter)
{
	return 0;
}

static inline int
wlan_hdd_debugfs_resume_create(struct hdd_adapter *adapter)
{
	return 0;
}
#endif

#ifdef WLAN_DEBUGFS
/**
 * hdd_debugfs_unit_test_target_create() - API to create unit_test_target file
 * @adapter: hdd adapter
 *
 * this file is created per adapter.
 * file path: /sys/kernel/debug/wlan_xx/unit_test_target
 *                (wlan_xx is adapter name)
 * usage:
 *      echo [module_id] [arg_num] [arg_0] [arg_xx] ... >unit_test_target
 *      echo '5' '2' '2' '1'>unit_test_target
 *      echo "5 2 2 1">unit_test_target  //using one null space as delimiter
 *
 * Return: 0 on success and errno on failure
 */
int wlan_hdd_debugfs_unit_test_target_create(struct hdd_adapter *adapter);
#else
static inline int
wlan_hdd_debugfs_unit_test_target_create(struct hdd_adapter *adapter)
{
	return 0;
}
#endif  /* WLAN_DEBUGFS */
#endif /* _WLAN_HDD_DEBUGFS_UNIT_TEST_H */
