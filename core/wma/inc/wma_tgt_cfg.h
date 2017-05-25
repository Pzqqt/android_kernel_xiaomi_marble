/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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

#ifndef WMA_TGT_CFG_H
#define WMA_TGT_CFG_H

/**
 * struct wma_tgt_services - target services
 * @sta_power_save: sta power save
 * @uapsd: uapsd
 * @ap_dfs: ap dfs
 * @en_11ac: enable 11ac
 * @arp_offload: arp offload
 * @early_rx: early rx
 * @pno_offload: pno offload
 * @beacon_offload: beacon offload
 * @lte_coex_ant_share: LTE coex ant share
 * @en_tdls: enable tdls
 * @en_tdls_offchan: enable tdls offchan
 * @en_tdls_uapsd_buf_sta: enable sta tdls uapsd buf
 * @en_tdls_uapsd_sleep_sta: enable sta tdls uapsd sleep
 * @en_roam_offload: enable roam offload
 * @en_11ax: enable 11ax
 */
struct wma_tgt_services {
	uint32_t sta_power_save;
	uint32_t uapsd;
	uint32_t ap_dfs;
	uint32_t en_11ac;
	uint32_t arp_offload;
	uint32_t early_rx;
#ifdef FEATURE_WLAN_SCAN_PNO
	bool pno_offload;
#endif /* FEATURE_WLAN_SCAN_PNO */
	bool beacon_offload;
	bool pmf_offload;
	uint32_t lte_coex_ant_share;
#ifdef FEATURE_WLAN_TDLS
	bool en_tdls;
	bool en_tdls_offchan;
	bool en_tdls_uapsd_buf_sta;
	bool en_tdls_uapsd_sleep_sta;
#endif /* FEATURE_WLAN_TDLS */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	bool en_roam_offload;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	bool en_11ax;
};

/**
 * struct wma_tgt_ht_cap - ht capabalitiy
 * @mpdu_density: mpdu density
 * @ht_rx_stbc: ht rx stbc
 * @ht_tx_stbc: ht tx stbc
 * @ht_rx_ldpc: ht rx ldpc
 * @ht_sgi_20: ht sgi 20
 * @ht_sgi_40: ht sgi 40
 * @num_rf_chains: num of rf chains
 */
struct wma_tgt_ht_cap {
	uint32_t mpdu_density;
	bool ht_rx_stbc;
	bool ht_tx_stbc;
	bool ht_rx_ldpc;
	bool ht_sgi_20;
	bool ht_sgi_40;
	uint32_t num_rf_chains;
};

/**
 * struct wma_tgt_vht_cap - vht capabalities
 * @vht_max_mpdu: vht max mpdu
 * @supp_chan_width: supported channel width
 * @vht_rx_ldpc: vht rx ldpc
 * @vht_short_gi_80: vht short gi 80
 * @vht_short_gi_160: vht short gi 160
 * @vht_tx_stbc: vht tx stbc
 * @vht_rx_stbc: vht rx stbc
 * @vht_su_bformer: vht su bformer
 * @vht_su_bformee: vht su bformee
 * @vht_mu_bformer: vht mu bformer
 * @vht_mu_bformee: vht mu bformee
 * @vht_max_ampdu_len_exp: vht max ampdu len exp
 * @vht_txop_ps: vht txop ps
 */
struct wma_tgt_vht_cap {
	uint32_t vht_max_mpdu;
	uint32_t supp_chan_width;
	uint32_t vht_rx_ldpc;
	uint32_t vht_short_gi_80;
	uint32_t vht_short_gi_160;
	uint32_t vht_tx_stbc;
	uint32_t vht_rx_stbc;
	uint32_t vht_su_bformer;
	uint32_t vht_su_bformee;
	uint32_t vht_mu_bformer;
	uint32_t vht_mu_bformee;
	uint32_t vht_max_ampdu_len_exp;
	uint32_t vht_txop_ps;
};

/**
 * struct wma_dfs_radar_ind - dfs radar indication
 * @ieee_chan_number: ieee channel number
 * @chan_freq: channel freq
 * @dfs_radar_status: dfs radar status
 */
struct wma_dfs_radar_ind {
	uint8_t ieee_chan_number;
	uint32_t chan_freq;
	uint32_t dfs_radar_status;
};

/**
 * struct wma_tgt_cfg - target config
 * @target_fw_version: target fw version
 * @target_fw_vers_ext: target fw extended sub version
 * @band_cap: band capability
 * @reg_domain: reg domain
 * @eeprom_rd_ext: eeprom rd ext
 * @hw_macaddr: hw mcast addr
 * @services: struct wma_tgt_services
 * @ht_cap: struct wma_tgt_ht_cap
 * @vht_cap: struct wma_tgt_vht_cap
 * @max_intf_count: max interface count
 * @lpss_support: lpass support
 * @egap_support: enhanced green ap support
 * @nan_datapath_enabled: nan data path support
 * @bool is_ra_rate_limit_enabled: RA filter support
 * @he_cap: HE capability received from FW
 * @dfs_cac_offload: dfs and cac timer offloaded
 * @tx_bfee_8ss_enabled: Tx Beamformee support for 8x8
 */
struct wma_tgt_cfg {
	uint32_t target_fw_version;
	uint32_t target_fw_vers_ext;
	uint8_t band_cap;
	uint32_t reg_domain;
	uint32_t eeprom_rd_ext;
	struct qdf_mac_addr hw_macaddr;
	struct wma_tgt_services services;
	struct wma_tgt_ht_cap ht_cap;
	struct wma_tgt_vht_cap vht_cap;
	uint8_t max_intf_count;
#ifdef WLAN_FEATURE_LPSS
	uint8_t lpss_support;
#endif
	uint8_t ap_arpns_support;
#ifdef FEATURE_GREEN_AP
	bool egap_support;
#endif
	uint32_t fine_time_measurement_cap;
	bool bpf_enabled;
#ifdef FEATURE_WLAN_RA_FILTERING
	bool is_ra_rate_limit_enabled;
#endif
#ifdef WLAN_FEATURE_NAN_DATAPATH
	bool nan_datapath_enabled;
#endif
	bool sub_20_support;
	uint16_t wmi_max_len;
#ifdef WLAN_FEATURE_11AX
	tDot11fIEvendor_he_cap he_cap;
#endif
	bool dfs_cac_offload;
	bool tx_bfee_8ss_enabled;
};
#endif /* WMA_TGT_CFG_H */
