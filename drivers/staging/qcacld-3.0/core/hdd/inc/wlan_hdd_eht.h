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

#if defined(WLAN_FEATURE_11BE) && defined(CFG80211_11BE_BASIC)
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
 *
 * update wiphy with the eht capabilties.
 *
 * Return: None
 */
void hdd_update_wiphy_eht_cap(struct hdd_context *hdd_ctx);

/**
 * wlan_hdd_get_mlo_link_id() - get link id and number of links
 * @beacon: beacon IE buffer
 * @link_id: link id to return
 * @num_link: total links
 *
 * Return: None
 */
void wlan_hdd_get_mlo_link_id(struct hdd_beacon_data *beacon,
			      uint8_t *link_id, uint8_t *num_link);

/**
 * hdd_set_11be_rate_code() - set 11be rate code
 * @adapter: net device adapter
 * @rate_code: new 11be rate code
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_set_11be_rate_code(struct hdd_adapter *adapter, uint16_t rate_code);

/**
 * hdd_sysfs_11be_rate_create() - Create sysfs entry to configure 11be rate
 * @adapter: net device adapter
 *
 * Return: None
 */
void hdd_sysfs_11be_rate_create(struct hdd_adapter *adapter);

/**
 * hdd_sysfs_11be_rate_destroy() - Destroy sysfs entry to configure 11be rate
 * @adapter: net device adapter
 *
 * Return: None
 */
void hdd_sysfs_11be_rate_destroy(struct hdd_adapter *adapter);
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
void hdd_update_wiphy_eht_cap(struct hdd_context *hdd_ctx)
{
}

static inline int
hdd_set_11be_rate_code(struct hdd_adapter *adapter, uint16_t rate_code)
{
	return 0;
}

static inline void hdd_sysfs_11be_rate_create(struct hdd_adapter *adapter)
{
}

static inline void hdd_sysfs_11be_rate_destroy(struct hdd_adapter *adapter)
{
}

static inline void wlan_hdd_get_mlo_link_id(struct hdd_beacon_data *beacon,
					    uint8_t *link_id, uint8_t *num_link)
{
}
#endif
#endif /* if !defined(WLAN_HDD_EHT_H)*/
