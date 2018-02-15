/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_hdd_debugfs_csr.h
 *
 * WLAN Host Device Driver implementation to update
 * debugfs with connect, scan and roam information
 */

#ifndef _WLAN_HDD_DEBUGFS_CSR_H
#define _WLAN_HDD_DEBUGFS_CSR_H

#include <wlan_hdd_includes.h>

#ifdef WLAN_DEBUGFS

/**
 * struct wlan_hdd_debugfs_buffer_info - Debugfs buffer info
 * @length: current length of the debugfs buffer
 * @max_buf_len: maximum buffer length of the debugfs buffer
 * @id: id from enum hdd_debugfs_file_id used to identify file
 * @data: start of debugfs buffer from which file read starts
 * @adapter: pointer to adapter
 *
 * This structure is used to hold the debugfs buffer details and is stored in
 * private data of file argument in file open operation.
 */
struct wlan_hdd_debugfs_buffer_info {
	ssize_t length;
	ssize_t max_buf_len;
	enum hdd_debugfs_file_id id;
	uint8_t *data;
	struct hdd_adapter *adapter;
};

/**
 * wlan_hdd_debugfs_csr_init() - Create wifi diagnostic debugfs files
 * @adapter: pointer to adapter for which debugfs files are to be created
 *
 * Return: None
 */
void wlan_hdd_debugfs_csr_init(struct hdd_adapter *adapter);

/**
 * wlan_hdd_debugfs_csr_deinit() - Remove wifi diagnostic debugfs files
 * @adapter: pointer to adapter for which debugfs files are to be removed
 *
 * Return: None
 */
void wlan_hdd_debugfs_csr_deinit(struct hdd_adapter *adapter);

/**
 * wlan_hdd_current_time_info_debugfs() - API to get time into user buffer
 * @buf: output buffer to hold current time when queried
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes copied
 */
ssize_t
wlan_hdd_current_time_info_debugfs(uint8_t *buf, ssize_t buf_avail_len);

#else
/**
 * wlan_hdd_debugfs_csr_init() - Create wifi diagnostic debugfs files
 * @adapter: pointer to adapter for which debugfs files are to be created
 *
 * Return: None
 */
static inline void wlan_hdd_debugfs_csr_init(struct hdd_adapter *adapter)
{
}

/**
 * wlan_hdd_debugfs_csr_deinit() - Remove wifi diagnostic debugfs files
 * @adapter: pointer to adapter for which debugfs files are to be removed
 *
 * Return: None
 */
static inline void wlan_hdd_debugfs_csr_deinit(struct hdd_adapter *adapter)
{
}

/**
 * wlan_hdd_current_time_info_debugfs() - API to get time into user buffer
 * @buf: output buffer to hold current time when queried
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes copied
 */
static inline ssize_t
wlan_hdd_current_time_info_debugfs(uint8_t *buf, ssize_t buf_avail_len)
{
	return 0;
}

#endif

#endif /* _WLAN_HDD_DEBUGFS_CSR_H */
