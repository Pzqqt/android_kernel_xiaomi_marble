/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: service_ready_param.h
 *
 * Public structures to access (ext)service ready data
 */
#ifndef _SERVICE_READY_PARAM_H_
#define _SERVICE_READY_PARAM_H_

#include "qdf_types.h"
#ifdef WLAN_SUPPORT_RF_CHARACTERIZATION
#include "wmi_unified_param.h"
#endif


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
 * @fw_version: FW build version
 * @fw_version_1: Second dword of FW version (Valid for non-tlv FW)
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
	uint32_t fw_version;
	uint32_t fw_version_1;
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
 * struct wlan_psoc_host_hal_reg_cap_ext - extended regulatory capabilities
 * recvd in EXT service
 * @wireless_modes: REGDMN MODE
 * @low_2ghz_chan: lower 2.4GHz channels
 * @high_2ghz_chan: higher 2.4 GHz channels
 * @low_5ghz_chan: lower 5 GHz channels
 * @high_5ghz_chan: higher 5 GHz channels
 */
struct wlan_psoc_host_hal_reg_cap_ext {
	uint64_t wireless_modes;
	uint32_t low_2ghz_chan;
	uint32_t high_2ghz_chan;
	uint32_t low_5ghz_chan;
	uint32_t high_5ghz_chan;
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
 * @phy_idx: Index to mac phy caps structure for the given hw_mode_id and phy_id
 * @hw_mode_config_type: holds the enum wmi_hw_mode_config_type
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
 * @he_cap_info_2G[]: HE capability info field of 802.11ax, WMI_HE_CAP defines
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
 * @he_cap_info_5G[]: HE capability info field of 802.11ax, WMI_HE_CAP defines
 * @he_supp_mcs_5G: HE Supported MCS Set field Rx/Tx same
 * @tx_chain_mask_5G: Valid Transmit chain mask
 * @rx_chain_mask_5G: Valid Receive chain mask
 * @he_cap_phy_info_2G: 2G HE capability phy field
 * @he_cap_phy_info_5G: 5G HE capability phy field
 * @he_cap_info_internal: HE PHY internal feature capability
 * @he_ppet2G: 2G HE PPET info
 * @he_ppet5G: 5G HE PPET info
 * @chainmask_table_id: chain mask table id
 * @lmac_id: hw mac id
 * @reg_cap_ext: extended regulatory capabilities
 * @tgt_pdev_id: target pdev id assigned and used by firmware
 * @nss_ratio_enabled: This flag is set if nss ratio is received from FW as part
 *                     of service ready ext event.
 * @nss_ratio: nss ratio is used to calculate the NSS value for 160MHz.
 * @hw_link_id: Unique link id across SoCs used to identify link in Multi-SoC ML
 */
struct wlan_psoc_host_mac_phy_caps {
	uint32_t hw_mode_id;
	uint32_t pdev_id;
	uint32_t phy_id;
	uint8_t phy_idx;
	int hw_mode_config_type;
	uint32_t supports_11b:1,
		 supports_11g:1,
		 supports_11a:1,
		 supports_11n:1,
		 supports_11ac:1,
		 supports_11ax:1,
#ifdef WLAN_FEATURE_11BE
		 supports_11be:1,
#endif
		 reserved:25;
	uint32_t supported_bands;
	uint32_t ampdu_density;
	uint32_t max_bw_supported_2G;
	uint32_t ht_cap_info_2G;
	uint32_t vht_cap_info_2G;
	uint32_t vht_supp_mcs_2G;
	uint32_t he_cap_info_2G[PSOC_HOST_MAX_MAC_SIZE];
	uint32_t he_supp_mcs_2G;
	uint32_t tx_chain_mask_2G;
	uint32_t rx_chain_mask_2G;
	uint32_t max_bw_supported_5G;
	uint32_t ht_cap_info_5G;
	uint32_t vht_cap_info_5G;
	uint32_t vht_supp_mcs_5G;
	uint32_t he_cap_info_5G[PSOC_HOST_MAX_MAC_SIZE];
	uint32_t he_supp_mcs_5G;
	uint32_t tx_chain_mask_5G;
	uint32_t rx_chain_mask_5G;
	uint32_t he_cap_phy_info_2G[PSOC_HOST_MAX_PHY_SIZE];
	uint32_t he_cap_phy_info_5G[PSOC_HOST_MAX_PHY_SIZE];
	uint32_t he_cap_info_internal;
	struct wlan_psoc_host_ppe_threshold he_ppet2G;
	struct wlan_psoc_host_ppe_threshold he_ppet5G;
	uint32_t chainmask_table_id;
	uint32_t lmac_id;
	struct wlan_psoc_host_hal_reg_cap_ext reg_cap_ext;
	uint32_t tgt_pdev_id;
	bool nss_ratio_enabled;
	uint8_t nss_ratio_info;
#if defined(WLAN_FEATURE_11BE_MLO) && defined(WLAN_MLO_MULTI_CHIP)
	uint16_t hw_link_id;
#endif
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

/*
 * struct wlan_psoc_host_mac_phy_caps_ext2 - Phy caps received in EXT2 service
 * @hw_mode_id: HW mode id
 * @pdev_id: Pdev id
 * @phy_id: Phy id
 * @wireless_modes_ext: Extended wireless modes
 * @eht_cap_info_2G[]: EHT capability info field of 802.11ax, WMI_HE_CAP defines
 * @eht_supp_mcs_2G: EHT Supported MCS Set field Rx/Tx same
 * @eht_cap_info_5G[]: EHT capability info field of 802.11ax, WMI_HE_CAP defines
 * @eht_supp_mcs_5G: EHT Supported MCS Set field Rx/Tx same
 * @eht_cap_phy_info_2G: 2G EHT capability phy field
 * @eht_cap_phy_info_5G: 5G EHT capability phy field
 * @eht_cap_info_internal: EHT PHY internal feature capability
 * @eht_supp_mcs_ext_2G: 2G EHT Supported MCS Set for Rx/Tx as per 11be D1.2
 * @eht_supp_mcs_ext_5G: 5G EHT Supported MCS Set for Rx/Tx as per 11be D1.2
 */
struct wlan_psoc_host_mac_phy_caps_ext2 {
	uint32_t hw_mode_id;
	uint32_t pdev_id;
	uint32_t phy_id;
	uint64_t wireless_modes_ext;
#ifdef WLAN_FEATURE_11BE
	uint32_t eht_cap_info_2G[PSOC_HOST_MAX_EHT_MAC_SIZE];
	uint32_t eht_supp_mcs_2G;
	uint32_t eht_cap_info_5G[PSOC_HOST_MAX_EHT_MAC_SIZE];
	uint32_t eht_supp_mcs_5G;
	uint32_t eht_cap_phy_info_2G[PSOC_HOST_MAX_EHT_PHY_SIZE];
	uint32_t eht_cap_phy_info_5G[PSOC_HOST_MAX_EHT_PHY_SIZE];
	uint32_t eht_cap_info_internal;
	uint32_t eht_supp_mcs_ext_2G[PSOC_HOST_EHT_MCS_NSS_MAP_2G_SIZE];
	uint32_t eht_supp_mcs_ext_5G[PSOC_HOST_EHT_MCS_NSS_MAP_5G_SIZE];
#endif
};

/*
 * struct wlan_psoc_host_scan_radio_caps - scan radio capabilities
 * @phy_id: phy id
 * @scan_radio_supported: indicates scan radio support
 * @dfs_en: indicates DFS needs to be enabled/disabled for scan radio vap
 */
struct wlan_psoc_host_scan_radio_caps {
	uint32_t phy_id;
	bool scan_radio_supported;
	bool dfs_en;
};

/**
 * struct wlan_psoc_host_dbr_ring_caps - Direct buffer rx module ring
 *                                       capability maintained by PSOC
 * @pdev_id: Pdev id of the pdev
 * @mod_id: Module id
 * @ring_elems_min: Minimum number of pointers in the ring
 * @min_buf_size: Minimum size of each buffer entry in the ring
 * @min_buf_align: Minimum alignment of the addresses in the ring
 */
struct wlan_psoc_host_dbr_ring_caps {
	uint32_t pdev_id;
	uint32_t mod_id;
	uint32_t ring_elems_min;
	uint32_t min_buf_size;
	uint32_t min_buf_align;
};

/**
 * struct wlan_psoc_host_spectral_scaling_params - Spectral scaling params
 * @pdev_id: Pdev id of the pdev
 * @formula_id: Formula id
 * @low_level_offset: Low level offset
 * @high_level_offset: High level offset
 * @rssi_thr: RSSI threshold
 * @default_agc_max_gain: Default agc max gain
 */
struct wlan_psoc_host_spectral_scaling_params {
	uint32_t pdev_id;
	uint32_t formula_id;
	uint32_t low_level_offset;
	uint32_t high_level_offset;
	uint32_t rssi_thr;
	uint32_t default_agc_max_gain;
};

/**
 * struct wlan_psoc_host_chainmask_capabilities - chain mask capabilities list
 * @supports_chan_width_20: channel width 20 support for this chain mask.
 * @supports_chan_width_40: channel width 40 support for this chain mask.
 * @supports_chan_width_80: channel width 80 support for this chain mask.
 * @supports_chan_width_160: channel width 160 support for this chain mask.
 * @supports_chan_width_80P80: channel width 80P80 support for this chain mask.
 * @supports_aSpectral: Agile Spectral support for this chain mask.
 * @supports_aSpectral_160: Agile Spectral support in 160 MHz.
 * @supports_aDFS_160: Agile DFS support in 160 MHz for this chain mask.
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
#ifdef WLAN_FEATURE_11BE
		 supports_chan_width_320:1,
#endif
		 supports_aSpectral:1,
		 supports_aSpectral_160:1,
		 supports_aDFS_160:1,
		 reserved:19,
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
 * @fw_build_vers_ext: Extended FW build version info.
 *                        bits 27:0 rsvd
 *                        bits 31:28 CRM sub ID
 * @num_hw_modes: Number of HW modes in event
 * @num_phy: Number of Phy mode.
 * @num_chainmask_tables: Number of chain mask tables.
 * @num_dbr_ring_caps: Number of direct buf rx ring capabilities
 * @max_bssid_indicator: Maximum number of VAPs in MBSS IE
 * @num_bin_scaling_params: Number of Spectral bin scaling parameters
 * @chainmask_table: Available chain mask tables.
 * @sar_version: SAR version info
 */
struct wlan_psoc_host_service_ext_param {
	uint32_t default_conc_scan_config_bits;
	uint32_t default_fw_config_bits;
	struct wlan_psoc_host_ppe_threshold ppet;
	uint32_t he_cap_info;
	uint32_t mpdu_density;
	uint32_t max_bssid_rx_filters;
	uint32_t fw_build_vers_ext;
	uint32_t num_hw_modes;
	uint32_t num_phy;
	uint32_t num_chainmask_tables;
	uint32_t num_dbr_ring_caps;
	uint32_t max_bssid_indicator;
	uint32_t num_bin_scaling_params;
	struct wlan_psoc_host_chainmask_table
		chainmask_table[PSOC_MAX_CHAINMASK_TABLES];
	uint32_t sar_version;
};

/**
 * struct wlan_psoc_host_service_ext2_param - EXT service base params in event
 * reg_db_version_major: REG DB version major number
 * reg_db_version_minor: REG DB version minor number
 * bdf_reg_db_version_major: BDF REG DB version major number
 * bdf_reg_db_version_minor: BDF REG DB version minor number
 * @num_dbr_ring_caps: Number of direct buf rx ring capabilities
 * @chwidth_num_peer_caps: Peer limit for peer_chan_width_switch WMI cmd
 * @max_ndp_sessions: Max number of ndp session fw supports
 * @preamble_puncture_bw_cap: Preamble Puncturing Tx support
 * @num_scan_radio_caps: Number of scan radio capabilities
 * @max_users_dl_ofdma: Max number of users per-PPDU for Downlink OFDMA
 * @max_users_ul_ofdma: Max number of users per-PPDU for Uplink OFDMA
 * @max_users_dl_mumimo: Max number of users per-PPDU for Downlink MU-MIMO
 * @max_users_ul_mumimo: Max number of users per-PPDU for Uplink MU-MIMO
 * @twt_ack_support_cap: TWT ack capability support
 * @target_cap_flags: Rx peer metadata version number used by target
 */
struct wlan_psoc_host_service_ext2_param {
	uint8_t reg_db_version_major;
	uint8_t reg_db_version_minor;
	uint8_t bdf_reg_db_version_major;
	uint8_t bdf_reg_db_version_minor;
	uint32_t num_dbr_ring_caps;
	uint32_t chwidth_num_peer_caps;
	uint32_t max_ndp_sessions;
	uint32_t preamble_puncture_bw_cap;
	uint8_t num_scan_radio_caps;
	uint16_t max_users_dl_ofdma;
	uint16_t max_users_ul_ofdma;
	uint16_t max_users_dl_mumimo;
	uint16_t max_users_ul_mumimo;
	uint32_t twt_ack_support_cap:1;
	uint32_t target_cap_flags;
};

#endif /* _SERVICE_READY_PARAM_H_*/
