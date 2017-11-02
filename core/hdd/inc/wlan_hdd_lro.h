/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

#ifndef __WLAN_HDD_LRO_H__
#define __WLAN_HDD_LRO_H__
/**
 * DOC: wlan_hdd_lro.h
 *
 * WLAN LRO interface module headers
 */

struct hdd_context;

/**
 * enum hdd_lro_rx_status - LRO receive frame status
 * @HDD_LRO_RX: frame sent over the LRO interface
 * @HDD_LRO_NO_RX: frame not sent over the LRO interface
 */
enum hdd_lro_rx_status {
	HDD_LRO_RX = 0,
	HDD_LRO_NO_RX = 1,
};

#if defined(FEATURE_LRO)
int hdd_lro_init(struct hdd_context *hdd_ctx);

enum hdd_lro_rx_status hdd_lro_rx(struct hdd_context *hdd_ctx,
	 struct hdd_adapter *adapter, struct sk_buff *skb);
void hdd_lro_display_stats(struct hdd_context *hdd_ctx);

/**
 * hdd_lro_set_reset() - vendor command for Disable/Enable LRO
 * @hdd_ctx: hdd context
 * @hdd_adapter_t: adapter
 * @enable_flag: enable or disable LRO.
 *
 * Return: none
 */
QDF_STATUS hdd_lro_set_reset(struct hdd_context *hdd_ctx,
					  struct hdd_adapter *adapter,
					  uint8_t enable_flag);
void hdd_disable_lro_in_concurrency(bool);
/**
 * hdd_disable_lro_for_low_tput() - enable/disable LRO based on tput
 * hdd_ctx: hdd context
 * disable: boolean to enable/disable LRO
 *
 * This API enables/disables LRO based on tput.
 *
 * Return: void
 */
void hdd_disable_lro_for_low_tput(struct hdd_context *hdd_ctx, bool disable);
#else
static inline int hdd_lro_init(struct hdd_context *hdd_ctx)
{
	return 0;
}

static inline enum hdd_lro_rx_status hdd_lro_rx(struct hdd_context *hdd_ctx,
	 struct hdd_adapter *adapter, struct sk_buff *skb)
{
	return HDD_LRO_NO_RX;
}

static inline void hdd_lro_display_stats(struct hdd_context *hdd_ctx)
{
}

static inline QDF_STATUS hdd_lro_set_reset(struct hdd_context *hdd_ctx,
					  struct hdd_adapter *adapter,
					  uint8_t enable_flag)
{
	return 0;
}
static inline void hdd_disable_lro_in_concurrency(bool disable)
{
}

static inline void
hdd_disable_lro_for_low_tput(struct hdd_context *hdd_ctx, bool disable)
{
}
#endif /* FEATURE_LRO */
#endif /* __WLAN_HDD_LRO_H__ */
