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
 * DOC: wlan_objmgr_psoc_service_ready_api.h
 *
 * Public APIs to access (ext)service ready data from psoc object
 */
#ifndef _WLAN_OBJMGR_PSOC_SERVICE_READY_API_H_
#define _WLAN_OBJMGR_PSOC_SERVICE_READY_API_H_

#include "qdf_types.h"

/* 128 is derived from definition of WMI_MAX_SERVICE */
#define OBJMGR_SERVICE_BM_SIZE ((128 + sizeof(uint32_t) - 1) / sizeof(uint32_t))

/**
 * struct wlan_objmgr_hal_reg_capability - hal reg table in psoc
 * @eeprom_rd: regdomain value specified in EEPROM
 * @eeprom_rd_ext: regdomain
 * @regcap1: CAP1 capabilities bit map
 * @regcap2: REGDMN EEPROM CAP
 * @wireless_modes: REGDMN MODE
 * @low_2ghz_chan: lower 2.4GHz channels
 * @high_2ghz_chan: higher 2.4 GHz channels
 * @low_5ghz_chan: lower 5 GHz channels
 * @high_5ghz_chan: higher 5 GHz channels
 *
 * wlan_objmgr_hal_reg_capability is derived from TARGET_HAL_REG_CAPABILITIES
 * because of WIN direct attach limitation which blocks WMI data structures and
 * WMI APIs direct usage in common code. So whenever TARGET_HAL_REG_CAPABILITIES
 * changes wlan_objmgr_hal_reg_capability also needs to be updated accordingly.
 */
struct wlan_objmgr_hal_reg_capability {
	uint32_t eeprom_rd;
	uint32_t eeprom_rd_ext;
	uint32_t regcap1;
	uint32_t regcap2;
	uint32_t wireless_modes;
	uint32_t low_2ghz_chan;
	uint32_t high_2ghz_chan;
	uint32_t low_5ghz_chan;
	uint32_t high_5ghz_chan;
};

/**
 * struct wlan_objmgr_target_capability_info - target capabilities in psoc
 * @phy_capability: PHY capabilities
 * @max_frag_entry: Maximum frag entries
 * @num_rf_chains: Number of RF chains supported
 * @ht_cap_info: HT cap info
 * @vht_cap_info: VHT cap info
 * @vht_supp_mcs: VHT Supported MCS
 * @hw_min_tx_power: HW minimum tx power
 * @hw_max_tx_power: HW maximum tx power
 * @sys_cap_info: sys capability info
 * @min_pkt_size_enable: Enterprise mode short pkt enable
 * @max_bcn_ie_size: Max beacon and probe rsp IE offload size
 * @max_num_scan_channels: Max scan channels
 * @max_supported_macs: max supported MCS
 * @wmi_fw_sub_feat_caps: FW sub feature capabilities
 * @txrx_chainmask: TXRX chain mask
 * @default_dbs_hw_mode_index: DBS hw mode index
 * @num_msdu_desc: number of msdu desc
 *
 * wlan_objmgr_target_capability_info is derived from target_capability_info
 * because of WIN direct attach limitation which blocks WMI data structures and
 * WMI APIs direct usage in common code. So whenever target_capability_info
 * changes wlan_objmgr_target_capability_info also needs to be updated
 * accordingly.
 */
struct wlan_objmgr_target_capability_info {
	uint32_t phy_capability;
	uint32_t max_frag_entry;
	uint32_t num_rf_chains;
	uint32_t ht_cap_info;
	uint32_t vht_cap_info;
	uint32_t vht_supp_mcs;
	uint32_t hw_min_tx_power;
	uint32_t hw_max_tx_power;
	uint32_t sys_cap_info;
	uint32_t min_pkt_size_enable;
	uint32_t max_bcn_ie_size;
	uint32_t max_num_scan_channels;
	uint32_t max_supported_macs;
	uint32_t wmi_fw_sub_feat_caps;
	uint32_t txrx_chainmask;
	uint32_t default_dbs_hw_mode_index;
	uint32_t num_msdu_desc;
};

/**
 * struct wlan_objmgr_psoc_service_ready_param - psoc service ready structure
 * @service_bitmap: service bitmap
 * @target_caps: traget capability
 * @hal_reg_cap: hal reg capability
 *
 * wlan_objmgr_psoc_service_ready_param is derived from wmi_service_ready_param
 * because of WIN direct attach limitation which blocks WMI data structures and
 * WMI APIs direct usage in common code. So whenever wmi_service_ready_param
 * changes wlan_objmgr_psoc_service_ready_param also needs to be updated
 * accordingly.
 */
struct wlan_objmgr_psoc_service_ready_param {
	uint32_t service_bitmap[OBJMGR_SERVICE_BM_SIZE];
	struct wlan_objmgr_target_capability_info target_caps;
	struct wlan_objmgr_hal_reg_capability hal_reg_cap;
};

#endif /* _WLAN_OBJMGR_PSOC_SERVICE_READY_API_H_*/
