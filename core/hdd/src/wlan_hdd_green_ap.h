/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_HDD_GREEN_AP_H
#define __WLAN_HDD_GREEN_AP_H

/**
 * DOC: wlan_hdd_green_ap.h
 *
 * WLAN Host Device Driver Green AP API specification
 */

struct hdd_context_s;
struct wma_tgt_cfg;

#ifdef FEATURE_GREEN_AP
/**
 * hdd_green_ap_init() - Initialize Green AP feature
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_green_ap_init(struct hdd_context_s *hdd_ctx);

/**
 * hdd_green_ap_deinit() - De-initialize Green AP feature
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_green_ap_deinit(struct hdd_context_s *hdd_ctx);

/**
 * hdd_green_ap_start_bss() - Notify Green AP of Start BSS event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_green_ap_start_bss(struct hdd_context_s *hdd_ctx);

/**
 * hdd_green_ap_stop_bss() - Notify Green AP of Stop BSS event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_green_ap_stop_bss(struct hdd_context_s *hdd_ctx);

/**
 * hdd_green_ap_add_sta() - Notify Green AP of Add Station event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_green_ap_add_sta(struct hdd_context_s *hdd_ctx);

/**
 * hdd_green_ap_del_sta() - Notify Green AP of Delete Station event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_green_ap_del_sta(struct hdd_context_s *hdd_ctx);

/**
 * hdd_green_ap_target_config() - Handle Green AP target configuration
 * @hdd_ctx: HDD global context where Green AP information is stored
 * @target_config: Target configuration containing Green AP info
 *
 * This function updates the HDD context with Green AP-specific
 * information provided by the target.
 *
 * Return: none
 */
void hdd_green_ap_target_config(struct hdd_context_s *hdd_ctx,
				struct wma_tgt_cfg *target_config);
#else
static inline void hdd_green_ap_init(struct hdd_context_s *hdd_ctx) {}
static inline void hdd_green_ap_deinit(struct hdd_context_s *hdd_ctx) {}
static inline void hdd_green_ap_start_bss(struct hdd_context_s *hdd_ctx) {}
static inline void hdd_green_ap_stop_bss(struct hdd_context_s *hdd_ctx) {}
static inline void hdd_green_ap_add_sta(struct hdd_context_s *hdd_ctx) {}
static inline void hdd_green_ap_del_sta(struct hdd_context_s *hdd_ctx) {}
static inline void hdd_green_ap_target_config(struct hdd_context_s *hdd_ctx,
					      struct wma_tgt_cfg *target_config)
{
}
#endif /* FEATURE_GREEN_AP */
#endif /* __WLAN_HDD_GREEN_AP_H */
