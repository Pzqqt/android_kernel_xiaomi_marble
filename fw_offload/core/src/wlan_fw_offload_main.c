/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: define internal APIs related to the fwol component
 */

#include "wlan_fw_offload_main.h"
#include "cfg_ucfg_api.h"

struct wlan_fwol_psoc_obj *fwol_get_psoc_obj(struct wlan_objmgr_psoc *psoc)
{
	return wlan_objmgr_psoc_get_comp_private_obj(psoc,
						     WLAN_UMAC_COMP_FWOL);
}

static void
fwol_init_coex_config_in_cfg(struct wlan_objmgr_psoc *psoc,
			     struct wlan_fwol_coex_config *coex_config)
{
	coex_config->btc_mode = cfg_get(psoc, CFG_BTC_MODE);
	coex_config->antenna_isolation = cfg_get(psoc, CFG_ANTENNA_ISOLATION);
	coex_config->max_tx_power_for_btc =
				cfg_get(psoc, CFG_MAX_TX_POWER_FOR_BTC);
	coex_config->wlan_low_rssi_threshold =
				cfg_get(psoc, CFG_WLAN_LOW_RSSI_THRESHOLD);
	coex_config->bt_low_rssi_threshold =
				cfg_get(psoc, CFG_BT_LOW_RSSI_THRESHOLD);
	coex_config->bt_interference_low_ll =
				cfg_get(psoc, CFG_BT_INTERFERENCE_LOW_LL);
	coex_config->bt_interference_low_ul =
				cfg_get(psoc, CFG_BT_INTERFERENCE_LOW_UL);
	coex_config->bt_interference_medium_ll =
				cfg_get(psoc, CFG_BT_INTERFERENCE_MEDIUM_LL);
	coex_config->bt_interference_medium_ul =
				cfg_get(psoc, CFG_BT_INTERFERENCE_MEDIUM_UL);
	coex_config->bt_interference_high_ll =
				cfg_get(psoc, CFG_BT_INTERFERENCE_HIGH_LL);
	coex_config->bt_interference_high_ul =
				cfg_get(psoc, CFG_BT_INTERFERENCE_HIGH_UL);
}

static void
fwol_init_thermal_temp_in_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_fwol_thermal_temp *thermal_temp)
{
	thermal_temp->thermal_temp_min_level0 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MIN_LEVEL0);
	thermal_temp->thermal_temp_max_level0 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MAX_LEVEL0);
	thermal_temp->thermal_temp_min_level1 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MIN_LEVEL1);
	thermal_temp->thermal_temp_max_level1 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MAX_LEVEL1);
	thermal_temp->thermal_temp_min_level2 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MIN_LEVEL2);
	thermal_temp->thermal_temp_max_level2 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MAX_LEVEL2);
	thermal_temp->thermal_temp_min_level3 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MIN_LEVEL3);
	thermal_temp->thermal_temp_max_level3 =
				cfg_get(psoc, CFG_THERMAL_TEMP_MAX_LEVEL3);
}

static void
fwol_init_ie_whiltelist_in_cfg(struct wlan_objmgr_psoc *psoc,
			       struct wlan_fwol_ie_whitelist *whitelist)
{
	whitelist->ie_whitelist = cfg_get(psoc, CFG_PROBE_REQ_IE_WHITELIST);
	whitelist->ie_bitmap_0 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP0);
	whitelist->ie_bitmap_1 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP1);
	whitelist->ie_bitmap_2 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP2);
	whitelist->ie_bitmap_3 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP3);
	whitelist->ie_bitmap_4 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP4);
	whitelist->ie_bitmap_5 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP5);
	whitelist->ie_bitmap_6 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP6);
	whitelist->ie_bitmap_7 = cfg_get(psoc, CFG_PROBE_REQ_IE_BIT_MAP7);
}

/**
 * ucfg_fwol_fetch_dhcp_server_settings: Populate the enable_dhcp_server_offload
 * and dhcp_max_num_clients from cfg
 * @psoc: The global psoc handler
 * @fwol_cfg: The cfg structure
 *
 * Return: none
 */
#ifdef DHCP_SERVER_OFFLOAD
static void ucfg_fwol_fetch_dhcp_server_settings(struct wlan_objmgr_psoc *psoc,
						 struct wlan_fwol_cfg *fwol_cfg)
{
	fwol_cfg->enable_dhcp_server_offload =
			cfg_get(psoc, CFG_DHCP_SERVER_OFFLOAD_SUPPORT);
	fwol_cfg->dhcp_max_num_clients =
			cfg_get(psoc, CFG_DHCP_SERVER_OFFLOAD_NUM_CLIENT);
}
#else
static void ucfg_fwol_fetch_dhcp_server_settings(struct wlan_objmgr_psoc *psoc,
						 struct wlan_fwol_cfg *fwol_cfg)
{
}
#endif

/**
 * ucfg_fwol_fetch_tsf_gpio_pin: Populate the tsf_gpio_pin from cfg
 * @psoc: The global psoc handler
 * @fwol_cfg: The cfg structure
 *
 * Return: none
 */
#ifdef WLAN_FEATURE_TSF
static void ucfg_fwol_fetch_tsf_gpio_pin(struct wlan_objmgr_psoc *psoc,
					 struct wlan_fwol_cfg *fwol_cfg)
{
	fwol_cfg->tsf_gpio_pin = cfg_get(psoc, CFG_SET_TSF_GPIO_PIN);
}
#else
static void ucfg_fwol_fetch_tsf_gpio_pin(struct wlan_objmgr_psoc *psoc,
					 struct wlan_fwol_cfg *fwol_cfg)
{
}
#endif

/**
 * ucfg_fwol_fetch_ra_filter: Populate the RA filter enabled or not from cfg
 * @psoc: The global psoc handler
 * @fwol_cfg: The cfg structure
 *
 * Return: none
 */
#ifdef FEATURE_WLAN_RA_FILTERING
static void ucfg_fwol_fetch_ra_filter(struct wlan_objmgr_psoc *psoc,
				      struct wlan_fwol_cfg *fwol_cfg)
{
	fwol_cfg->is_rate_limit_enabled = cfg_get(psoc, CFG_RA_FILTER_ENABLE);
}
#else
static void ucfg_fwol_fetch_ra_filter(struct wlan_objmgr_psoc *psoc,
				      struct wlan_fwol_cfg *fwol_cfg)
{
}
#endif

QDF_STATUS fwol_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_fwol_psoc_obj *fwol_obj;
	struct wlan_fwol_cfg *fwol_cfg;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj) {
		fwol_err("Failed to get FWOL Obj");
		return QDF_STATUS_E_FAILURE;
	}

	fwol_cfg = &fwol_obj->cfg;

	fwol_init_coex_config_in_cfg(psoc, &fwol_cfg->coex_config);
	fwol_init_thermal_temp_in_cfg(psoc, &fwol_cfg->thermal_temp_cfg);
	fwol_init_ie_whiltelist_in_cfg(psoc, &fwol_cfg->ie_whitelist_cfg);
	fwol_cfg->ani_enabled = cfg_get(psoc, CFG_ENABLE_ANI);
	fwol_cfg->enable_rts_sifsbursting =
				cfg_get(psoc, CFG_SET_RTS_FOR_SIFS_BURSTING);
	fwol_cfg->max_mpdus_inampdu = cfg_get(psoc, CFG_MAX_MPDUS_IN_AMPDU);
	fwol_cfg->arp_ac_category = cfg_get(psoc, CFG_ARP_AC_CATEGORY);
	fwol_cfg->enable_phy_reg_retention = cfg_get(psoc, CFG_ENABLE_PHY_REG);
	fwol_cfg->upper_brssi_thresh = cfg_get(psoc, CFG_UPPER_BRSSI_THRESH);
	fwol_cfg->lower_brssi_thresh = cfg_get(psoc, CFG_LOWER_BRSSI_THRESH);
	fwol_cfg->enable_dtim_1chrx = cfg_get(psoc, CFG_DTIM_1CHRX_ENABLE);
	fwol_cfg->alternative_chainmask_enabled =
				cfg_get(psoc, CFG_ENABLE_COEX_ALT_CHAINMASK);
	fwol_cfg->smart_chainmask_enabled =
				cfg_get(psoc, CFG_ENABLE_SMART_CHAINMASK);
	fwol_cfg->get_rts_profile = cfg_get(psoc, CFG_ENABLE_FW_RTS_PROFILE);
	fwol_cfg->enable_fw_log_level =
				cfg_get(psoc, CFG_ENABLE_FW_DEBUG_LOG_LEVEL);
	fwol_cfg->enable_fw_log_type = cfg_get(psoc, CFG_ENABLE_FW_LOG_TYPE);
	ucfg_fwol_fetch_ra_filter(psoc, fwol_cfg);
	ucfg_fwol_fetch_tsf_gpio_pin(psoc, fwol_cfg);
	ucfg_fwol_fetch_dhcp_server_settings(psoc, fwol_cfg);

	return status;
}

QDF_STATUS fwol_cfg_on_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	/* Clear the CFG structure */
	return QDF_STATUS_SUCCESS;
}
