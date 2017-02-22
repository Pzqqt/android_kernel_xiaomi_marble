/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC : wlan_hdd_he.h
 *
 * WLAN Host Device Driver file for 802.11ax (High Efficiency) support.
 *
 */

#if !defined(WLAN_HDD_HE_H)
#define WLAN_HDD_HE_H

struct hdd_context_s;
struct wma_tgt_cfg;
struct beacon_data_s;
struct sap_Config;

#ifdef WLAN_FEATURE_11AX
/**
 * enum qca_wlan_vendor_attr_get_he_capabilities - attributes for HE caps.
 *						  vendor command.
 * @QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_INVALID - invalid
 * @QCA_WLAN_VENDOR_ATTR_HE_SUPPORTED - to check if HE capabilities is supported
 * @QCA_WLAN_VENDOR_ATTR_PHY_CAPAB - to get HE PHY capabilities
 * @QCA_WLAN_VENDOR_ATTR_MAC_CAPAB - to get HE MAC capabilities
 * @QCA_WLAN_VENDOR_ATTR_HE_MCS - to get HE MCS
 * @QCA_WLAN_VENDOR_ATTR_NUM_SS - to get NUM SS
 * @QCA_WLAN_VENDOR_ATTR_RU_IDX_MASK - to get RU index mask
 * @QCA_WLAN_VENDOR_ATTR_RU_COUNT - to get RU count,
 * @QCA_WLAN_VENDOR_ATTR_PPE_THRESHOLD - to get PPE Threshold,
 * @QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_AFTER_LAST - next to last valid enum
 * @QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_MAX - max value supported
 *
 * enum values are used for NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES sub command.
 */
enum qca_wlan_vendor_attr_get_he_capabilities {
	QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_HE_SUPPORTED,
	QCA_WLAN_VENDOR_ATTR_PHY_CAPAB,
	QCA_WLAN_VENDOR_ATTR_MAC_CAPAB,
	QCA_WLAN_VENDOR_ATTR_HE_MCS,
	QCA_WLAN_VENDOR_ATTR_NUM_SS = 5,
	QCA_WLAN_VENDOR_ATTR_RU_IDX_MASK,
	QCA_WLAN_VENDOR_ATTR_PPE_THRESHOLD,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_MAX =
	QCA_WLAN_VENDOR_ATTR_HE_CAPABILITIES_AFTER_LAST - 1,
};

void hdd_update_tgt_he_cap(struct hdd_context_s *hdd_ctx,
			   struct wma_tgt_cfg *cfg);
void wlan_hdd_check_11ax_support(struct beacon_data_s *beacon,
				 struct sap_Config *config);
void hdd_he_print_ini_config(hdd_context_t *hdd_ctx);
int hdd_update_he_cap_in_cfg(hdd_context_t *hdd_ctx);
void hdd_he_set_sme_config(tSmeConfigParams *sme_config,
			   struct hdd_config *config);

/**
 * wlan_hdd_cfg80211_get_he_cap() - get HE Capabilities
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 if success, non-zero for failure
 */
int wlan_hdd_cfg80211_get_he_cap(struct wiphy *wiphy,
				 struct wireless_dev *wdev, const void *data,
				 int data_len);
#define FEATURE_11AX_VENDOR_COMMANDS					\
{									\
	.info.vendor_id = QCA_NL80211_VENDOR_ID,			\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES,	\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		 WIPHY_VENDOR_CMD_NEED_NETDEV,				\
	.doit = wlan_hdd_cfg80211_get_he_cap				\
},

#else
static inline void hdd_update_tgt_he_cap(struct hdd_context_s *hdd_ctx,
					 struct wma_tgt_cfg *cfg)
{
}

static inline void wlan_hdd_check_11ax_support(struct beacon_data_s *beacon,
					       struct sap_Config *config)
{
}

static inline void hdd_he_print_ini_config(hdd_context_t *hdd_ctx)
{
}

static inline int hdd_update_he_cap_in_cfg(hdd_context_t *hdd_ctx)
{
	return 0;
}

static inline void hdd_he_set_sme_config(tSmeConfigParams *sme_config,
					 struct hdd_config *config)
{
}

/* dummy definition */
#define FEATURE_11AX_VENDOR_COMMANDS

#endif
#endif /* if !defined(WLAN_HDD_HE_H)*/
