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
 * DOC : wlan_hdd_eht.h
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#if !defined(WLAN_HDD_EHT_H)
#define WLAN_HDD_EHT_H

struct hdd_context;
struct wma_tgt_cfg;
struct hdd_beacon_data;
struct sap_config;

#ifdef WLAN_FEATURE_11BE
/**
 * hdd_update_tgt_eht_cap() - Update EHT related capabilities
 * @hdd_ctx: HDD context
 * @eht_cap: Target EHT capabilities
 *
 * This function updates WNI CFG with Target capabilities received as part of
 * Default values present in WNI CFG are the values supported by FW/HW.
 * INI should be introduced if user control is required to control the value.
 *
 * Return: None
 */
void hdd_update_tgt_eht_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg);

/**
 * hdd_update_eht_cap_in_cfg() - update EHT cap in global CFG
 * @hdd_ctx: pointer to hdd context
 *
 * This API will update the EHT config in CFG after taking intersection
 * of INI and firmware capabilities provided reading CFG
 *
 * Return: 0 on success and errno on failure
 */
int hdd_update_eht_cap_in_cfg(struct hdd_context *hdd_ctx);

/**
 * wlan_hdd_check_11be_support() - check if beacon IE and update hw mode
 * @beacon: beacon IE buffer
 * @config: pointer to sap config
 *
 * Check if EHT cap IE is present in beacon IE, if present update hw mode
 * to 11be.
 *
 * Return: None
 */
void wlan_hdd_check_11be_support(struct hdd_beacon_data *beacon,
				 struct sap_config *config);

/**
 * hdd_update_wiphy_eht_cap() - update the wiphy with eht capabilities
 * @hdd_ctx: HDD context
 * @cfg: WMA target configuration
 *
 * update wiphy with the eht capabilties.
 *
 * Return: None
 */
void hdd_update_wiphy_eht_cap(struct hdd_context *hdd_ctx,
			      struct wma_tgt_cfg *cfg);
#else
static inline
void hdd_update_tgt_eht_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg)
{
}

static inline void wlan_hdd_check_11be_support(struct hdd_beacon_data *beacon,
					       struct sap_config *config)
{
}

static inline
void hdd_update_wiphy_eht_cap(struct hdd_context *hdd_ctx,
			      struct wma_tgt_cfg *cfg)
{
}
#endif
#endif /* if !defined(WLAN_HDD_EHT_H)*/
