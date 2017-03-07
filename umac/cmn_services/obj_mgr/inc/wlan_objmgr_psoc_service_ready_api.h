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

#define PSOC_SERVICE_BM_SIZE ((128 + sizeof(uint32_t) - 1) / sizeof(uint32_t))
#define PSOC_HOST_MAX_NUM_SS (8)
#define PSOC_HOST_MAX_PHY_SIZE (3)
#define PSOC_MAX_HW_MODE (2)
#define PSOC_MAX_MAC_PHY_CAP (5)
#define PSOC_MAX_PHY_REG_CAP (3)
#define PSOC_MAX_CHAINMASK_TABLES (5)

/* forward declaration of object manager psoc object type */
struct wlan_objmgr_psoc;

/**
 * struct wlan_psoc_hal_reg_capability - hal reg table in psoc
 * @eeprom_rd: regdomain value specified in EEPROM
 * @eeprom_rd_ext: regdomain
 * @regcap1: CAP1 capabilities bit map
 * @regcap2: REGDMN EEPROM CAP
 * @wireless_modes: REGDMN MODE
 * @low_2ghz_chan: lower 2.4GHz channels
 * @high_2ghz_chan: higher 2.4 GHz channels
 * @low_5ghz_chan: lower 5 GHz channels
 * @high_5ghz_chan: higher 5 GHz channels
 */
struct wlan_psoc_hal_reg_capability {
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
 * struct wlan_psoc_target_capability_info - target capabilities in psoc
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
 */
struct wlan_psoc_target_capability_info {
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
 */
struct wlan_objmgr_psoc_service_ready_param {
	uint32_t service_bitmap[PSOC_SERVICE_BM_SIZE];
	struct wlan_psoc_target_capability_info target_caps;
	struct wlan_psoc_hal_reg_capability hal_reg_cap;
};

/**
 * struct wlan_psoc_host_hal_reg_capabilities_ext: Below are Reg caps per PHY.
 *                       Please note PHY ID starts with 0.
 * @phy_id: phy id starts with 0.
 * @eeprom_reg_domain: regdomain value specified in EEPROM
 * @eeprom_reg_domain_ext: regdomain
 * @regcap1: CAP1 capabilities bit map, see REGDMN_CAP1_ defines
 * @regcap2: REGDMN EEPROM CAP, see REGDMN_EEPROM_EEREGCAP_ defines
 * @wireless_modes: REGDMN MODE, see REGDMN_MODE_ enum
 * @low_2ghz_chan: 2G channel low
 * @high_2ghz_chan: 2G channel High
 * @low_5ghz_chan: 5G channel low
 * @high_5ghz_chan: 5G channel High
 */
struct wlan_psoc_host_hal_reg_capabilities_ext {
	uint32_t phy_id;
	uint32_t eeprom_reg_domain;
	uint32_t eeprom_reg_domain_ext;
	uint32_t regcap1;
	uint32_t regcap2;
	uint32_t wireless_modes;
	uint32_t low_2ghz_chan;
	uint32_t high_2ghz_chan;
	uint32_t low_5ghz_chan;
	uint32_t high_5ghz_chan;
};

/**
 * struct wlan_psoc_host_ppe_threshold - PPE threshold
 * @numss_m1: NSS - 1
 * @ru_bit_mask: RU bit mask indicating the supported RU's
 * @ppet16_ppet8_ru3_ru0: ppet8 and ppet16 for max num ss
 */
struct wlan_psoc_host_ppe_threshold {
	uint32_t numss_m1;
	uint32_t ru_bit_mask;
	uint32_t ppet16_ppet8_ru3_ru0[PSOC_HOST_MAX_NUM_SS];
};

/**
 * struct wlan_psoc_host_mac_phy_caps - Phy caps recvd in EXT service
 *  @hw_mode_id: identify a particular set of HW characteristics,
 *        as specified by the subsequent fields. WMI_MAC_PHY_CAPABILITIES
 *        element must be mapped to its parent WMI_HW_MODE_CAPABILITIES
 *        element using hw_mode_id. No particular ordering of
 *        WMI_MAC_PHY_CAPABILITIES elements should be
 *        assumed, though in practice the elements may always be ordered
 *        by hw_mode_id.
 * @pdev_id: pdev_id starts with 1. pdev_id 1 => phy_id 0, pdev_id 2 => phy_id 1
 * @phy_id: Starts with 0
 * @bitmap of supported modulations
 * @supported_bands: supported bands, enum WLAN_BAND_CAPABILITY
 * @ampdu_density: ampdu density 0 for no restriction, 1 for 1/4 us,
 *        2 for 1/2 us, 3 for 1 us,4 for 2 us, 5 for 4 us,
 *        6 for 8 us,7 for 16 us
 * @max_bw_supported_2G: max bw supported 2G, enum wmi_channel_width
 * @ht_cap_info_2G: WMI HT Capability, WMI_HT_CAP defines
 * @vht_cap_info_2G: VHT capability info field of 802.11ac, WMI_VHT_CAP defines
 * @vht_supp_mcs_2G: VHT Supported MCS Set field Rx/Tx same
 *        The max VHT-MCS for n SS subfield (where n = 1,...,8) is encoded as
 *        follows
 *         - 0 indicates support for VHT-MCS 0-7 for n spatial streams
 *         - 1 indicates support for VHT-MCS 0-8 for n spatial streams
 *         - 2 indicates support for VHT-MCS 0-9 for n spatial streams
 *         - 3 indicates that n spatial streams is not supported
 * @he_cap_info_2G: HE capability info field of 802.11ax, WMI_HE_CAP defines
 * @he_supp_mcs_2G: HE Supported MCS Set field Rx/Tx same
 * @tx_chain_mask_2G: Valid Transmit chain mask
 * @rx_chain_mask_2G: Valid Receive chain mask
 * @max_bw_supported_5G: max bw supported 5G, enum wmi_channel_width
 * @ht_cap_info_5G: WMI HT Capability, WMI_HT_CAP defines
 * @vht_cap_info_5G: VHT capability info field of 802.11ac, WMI_VHT_CAP defines
 * @vht_supp_mcs_5G: VHT Supported MCS Set field Rx/Tx same
 *        The max VHT-MCS for n SS subfield (where n = 1,...,8) is encoded as
 *        follows
 *        - 0 indicates support for VHT-MCS 0-7 for n spatial streams
 *        - 1 indicates support for VHT-MCS 0-8 for n spatial streams
 *        - 2 indicates support for VHT-MCS 0-9 for n spatial streams
 *        - 3 indicates that n spatial streams is not supported
 * @he_cap_info_5G: HE capability info field of 802.11ax, WMI_HE_CAP defines
 * @he_supp_mcs_5G: HE Supported MCS Set field Rx/Tx same
 * @tx_chain_mask_5G: Valid Transmit chain mask
 * @rx_chain_mask_5G: Valid Receive chain mask
 * @he_cap_phy_info_2G: 2G HE capability phy field
 * @he_cap_phy_info_5G: 5G HE capability phy field
 * @he_ppet2G: 2G HE PPET info
 * @he_ppet5G: 5G HE PPET info
 * @chainmask_table_id: chain mask table id
 */
struct wlan_psoc_host_mac_phy_caps {
	uint32_t hw_mode_id;
	uint32_t pdev_id;
	uint32_t phy_id;
	uint32_t supports_11b:1,
		 supports_11g:1,
		 supports_11a:1,
		 supports_11n:1,
		 supports_11ac:1,
		 supports_11ax:1;
	uint32_t supported_bands;
	uint32_t ampdu_density;
	uint32_t max_bw_supported_2G;
	uint32_t ht_cap_info_2G;
	uint32_t vht_cap_info_2G;
	uint32_t vht_supp_mcs_2G;
	uint32_t he_cap_info_2G;
	uint32_t he_supp_mcs_2G;
	uint32_t tx_chain_mask_2G;
	uint32_t rx_chain_mask_2G;
	uint32_t max_bw_supported_5G;
	uint32_t ht_cap_info_5G;
	uint32_t vht_cap_info_5G;
	uint32_t vht_supp_mcs_5G;
	uint32_t he_cap_info_5G;
	uint32_t he_supp_mcs_5G;
	uint32_t tx_chain_mask_5G;
	uint32_t rx_chain_mask_5G;
	uint32_t he_cap_phy_info_2G[PSOC_HOST_MAX_PHY_SIZE];
	uint32_t he_cap_phy_info_5G[PSOC_HOST_MAX_PHY_SIZE];
	struct wlan_psoc_host_ppe_threshold he_ppet2G;
	struct wlan_psoc_host_ppe_threshold he_ppet5G;
	uint32_t chainmask_table_id;
};

/**
 * struct wlan_psoc_host_hw_mode_caps - HW mode capabilities in EXT event
 * @hw_mode_id: identify a particular set of HW characteristics,
 *              as specified by the subsequent fields
 * @phy_id_map: BIT0 represents phy_id 0, BIT1 represent phy_id 1 and so on
 * @hw_mode_config_type: HW mode config type
 */
struct wlan_psoc_host_hw_mode_caps {
	uint32_t hw_mode_id;
	uint32_t phy_id_map;
	uint32_t hw_mode_config_type;
};

/**
 * struct wlan_psoc_host_chainmask_capabilities - chain mask capabilities list
 * @supports_chan_width_20: channel width 20 support for this chain mask.
 * @supports_chan_width_40: channel width 40 support for this chain mask.
 * @supports_chan_width_80: channel width 80 support for this chain mask.
 * @supports_chan_width_160: channel width 160 support for this chain mask.
 * @supports_chan_width_80P80: channel width 80P80 support for this chain mask.
 * @chain_mask_2G: 2G support for this chain mask.
 * @chain_mask_5G: 5G support for this chain mask.
 * @chain_mask_tx: Tx support for this chain mask.
 * @chain_mask_rx: Rx support for this chain mask.
 * @supports_aDFS: Agile DFS support for this chain mask.
 * @chainmask: chain mask value.
 */
struct wlan_psoc_host_chainmask_capabilities {
	uint32_t supports_chan_width_20:1,
		 supports_chan_width_40:1,
		 supports_chan_width_80:1,
		 supports_chan_width_160:1,
		 supports_chan_width_80P80:1,
		 reserved:22,
		 chain_mask_2G:1,
		 chain_mask_5G:1,
		 chain_mask_tx:1,
		 chain_mask_rx:1,
		 supports_aDFS:1;
	uint32_t chainmask;
};

/**
 * struct wlan_psoc_host_chainmask_table - chain mask table
 * @table_id: tableid.
 * @num_valid_chainmasks: num valid chainmasks.
 * @cap_list: pointer to wlan_psoc_host_chainmask_capabilities list.
 */
struct wlan_psoc_host_chainmask_table {
	uint32_t table_id;
	uint32_t num_valid_chainmasks;
	struct wlan_psoc_host_chainmask_capabilities *cap_list;
};

/**
 * struct wlan_psoc_host_service_ext_param - EXT service base params in event
 * @default_conc_scan_config_bits: Default concurrenct scan config
 * @default_fw_config_bits: Default HW config bits
 * @wlan_psoc_host_ppe_threshold ppet: Host PPE threshold struct
 * @he_cap_info: HE capabality info
 * @mpdu_density: units are microseconds
 * @max_bssid_rx_filters: Maximum no of BSSID based RX filters host can program
 *                        Value 0 means FW hasn't given any limit to host.
 * @num_hw_modes: Number of HW modes in event
 * @num_phy: Number of Phy mode.
 * @num_chainmask_tables: Number of chain mask tables.
 * @chainmask_table: Available chain mask tables.
 */
struct wlan_psoc_host_service_ext_param {
	uint32_t default_conc_scan_config_bits;
	uint32_t default_fw_config_bits;
	struct wlan_psoc_host_ppe_threshold ppet;
	uint32_t he_cap_info;
	uint32_t mpdu_density;
	uint32_t max_bssid_rx_filters;
	uint32_t num_hw_modes;
	uint32_t num_phy;
	uint32_t num_chainmask_tables;
	struct wlan_psoc_host_chainmask_table chainmask_table[PSOC_MAX_CHAINMASK_TABLES];
};

/**
 * struct wlan_objmgr_psoc_ext_service_ready_param - psoc ext service ready
 * @service_ext_param: service ext param
 * @hw_mode_caps: hw mode caps
 * @mac_phy_cap: mac phy cap
 * @reg_cap: regulatory capability
 */
struct wlan_objmgr_psoc_ext_service_ready_param {
	struct wlan_psoc_host_service_ext_param service_ext_param;
	struct wlan_psoc_host_hw_mode_caps hw_mode_caps[PSOC_MAX_HW_MODE];
	struct wlan_psoc_host_mac_phy_caps
			mac_phy_cap[PSOC_MAX_MAC_PHY_CAP];
	struct wlan_psoc_host_hal_reg_capabilities_ext
			reg_cap[PSOC_MAX_PHY_REG_CAP];
};

/**
 * wlan_objmgr_populate_service_ready_data() - populate wmi service ready data
 *                                             in psoc
 * @psoc: psoc object pointer
 * @service_ready_data: pointer to wmi service ready data
 *
 * Return: QDF status
 */
void
wlan_objmgr_populate_service_ready_data(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_psoc_service_ready_param *service_ready_data);

/**
 * wlan_objmgr_populate_ext_service_ready_data() - populate wmi ext service
 *                                                 ready data in psoc
 * @psoc: psoc object pointer
 * @ext_service_data: pointer to ext wmi service ready data
 *
 * Return: QDF status
 */
void
wlan_objmgr_populate_ext_service_ready_data(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_psoc_ext_service_ready_param *ext_service_data);

/**
 * wlan_objmgr_ext_service_ready_chainmask_table_caplist_alloc()
 *	- allocate chainmask table capability list.
 * @service_ext_param: pointer to server ext param.
 *
 * Allocates capability list based on num_valid_chainmasks for that table.
 *
 * Return: QDF Status.
 */
QDF_STATUS wlan_objmgr_ext_service_ready_chainmask_table_caplist_alloc(
		struct wlan_psoc_host_service_ext_param *service_ext_param);

/**
 * wlan_objmgr_ext_service_ready_chainmask_table_caplist_free()
 *	-free chainmask table capability list.
 * @service_ext_param: pointer to server ext param.
 *
 * free capability list based on num_valid_chainmasks for that table.
 *
 * Return: QDF Status.
 */
QDF_STATUS wlan_objmgr_ext_service_ready_chainmask_table_caplist_free(
		struct wlan_psoc_host_service_ext_param *service_ext_param);

#endif /* _WLAN_OBJMGR_PSOC_SERVICE_READY_API_H_*/
