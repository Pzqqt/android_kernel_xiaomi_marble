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
 *  DOC: wlan_hdd_sysfs_mem_stats.c
 *
 *  Implementation to add sysfs node wlan_mem_stats
 *
 */

#include <wlan_hdd_includes.h>
#include "osif_psoc_sync.h"
#include <wlan_hdd_sysfs.h>
#include <qdf_mem.h>
#include <wlan_hdd_sysfs_mem_stats.h>

static ssize_t __hdd_wlan_mem_stats_show(char *buf)
{
	return scnprintf(buf, PAGE_SIZE,
			 "DMA = %d HEAP = %d SKB = %d SKB_MAX = %d\n",
			 qdf_dma_mem_stats_read(),
			 qdf_heap_mem_stats_read(),
			 qdf_skb_total_mem_stats_read(),
			 qdf_skb_max_mem_stats_read());
}

static ssize_t __hdd_wlan_dp_mem_stats_show(char *buf)
{
	int32_t len = 0;

	len += scnprintf(buf + len, PAGE_SIZE,
			 "TX_NBUF_MEM = %d MAX_TX_NBUF_MEM = %d "
			 "RX_NBUF_MEM = %d MAX_RX_NBUF_MEM = %d "
			 "TX_NBUF_CNT = %d MAX_TX_NBUF_CNT = %d "
			 "RX_NBUF_CNT = %d MAX_RX_NBUF_CNT = %d "
			 "PENDING_TX_DESCS = %d MAX_PENDING_TX_DESCS = %d\n",
			 qdf_dp_tx_skb_mem_stats_read(),
			 qdf_dp_tx_skb_max_mem_stats_read(),
			 qdf_dp_rx_skb_mem_stats_read(),
			 qdf_dp_rx_skb_max_mem_stats_read(),
			 qdf_mem_dp_tx_skb_cnt_read(),
			 qdf_mem_dp_tx_skb_max_cnt_read(),
			 qdf_mem_dp_rx_skb_cnt_read(),
			 qdf_mem_dp_rx_skb_max_cnt_read(),
			 qdf_mem_tx_desc_cnt_read(),
			 qdf_mem_tx_desc_max_read());
	return len;
}

static ssize_t hdd_wlan_mem_stats_show(struct kobject *kobj,
				       struct kobj_attribute *attr,
				       char *buf)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct osif_psoc_sync *psoc_sync;
	ssize_t length;
	int errno;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = osif_psoc_sync_op_start(hdd_ctx->parent_dev, &psoc_sync);
	if (errno)
		return errno;

	length = __hdd_wlan_mem_stats_show(buf);
	if (psoc_sync)
		osif_psoc_sync_op_stop(psoc_sync);

	return length;
}

static ssize_t hdd_wlan_dp_mem_stats_show(struct kobject *kobj,
					  struct kobj_attribute *attr,
					  char *buf)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct osif_psoc_sync *psoc_sync;
	ssize_t length;
	int errno;

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = osif_psoc_sync_op_start(hdd_ctx->parent_dev, &psoc_sync);
	if (errno)
		return errno;

	length = __hdd_wlan_dp_mem_stats_show(buf);
	if (psoc_sync)
		osif_psoc_sync_op_stop(psoc_sync);

	return length;
}

static struct kobj_attribute mem_stats_attribute =
	__ATTR(wlan_mem_stats, 0440, hdd_wlan_mem_stats_show, NULL);

static struct kobj_attribute mem_dp_stats_attribute =
	__ATTR(wlan_dp_mem_stats, 0440, hdd_wlan_dp_mem_stats_show, NULL);

int hdd_sysfs_mem_stats_create(struct kobject *wlan_kobject)
{
	int error;

	if (!wlan_kobject) {
		hdd_err("Could not get wlan kobject!");
		return -EINVAL;
	}
	error = sysfs_create_file(wlan_kobject, &mem_stats_attribute.attr);
	if (error) {
		hdd_err("Failed to create sysfs file wlan_mem_stats");
		return -EINVAL;
	}
	error = sysfs_create_file(wlan_kobject, &mem_dp_stats_attribute.attr);
	if (error) {
		hdd_err("Failed to create sysfs file wlan_dp_mem_stats");
		sysfs_remove_file(wlan_kobject, &mem_stats_attribute.attr);
		return -EINVAL;
	}

	qdf_mem_stats_init();

	return error;
}

void hdd_sysfs_mem_stats_destroy(struct kobject *wlan_kobject)
{
	if (!wlan_kobject) {
		hdd_err("Could not get wlan kobject!");
		return;
	}
	sysfs_remove_file(wlan_kobject, &mem_dp_stats_attribute.attr);
	sysfs_remove_file(wlan_kobject, &mem_stats_attribute.attr);
}

