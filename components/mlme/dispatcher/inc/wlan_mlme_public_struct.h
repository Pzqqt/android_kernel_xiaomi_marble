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

/*
 * DOC: contains mlme structure definations
 */

#ifndef _WLAN_MLME_STRUCT_H_
#define _WLAN_MLME_STRUCT_H_

#include <wlan_cmn.h>

/**
 * struct mlme_ht_capabilities_info - HT Capabilities Info
 * @l_sig_tx_op_protection: L-SIG TXOP Protection Mechanism support
 * @stbc_control_frame: STBC Control frame support
 * @psmp: PSMP Support
 * @dsss_cck_mode_40_mhz: To indicate use of DSSS/CCK in 40Mhz
 * @maximal_amsdu_size: Maximum AMSDU Size - 0:3839 octes, 1:7935 octets
 * @delayed_ba: Support of Delayed Block Ack
 * @rx_stbc: Rx STBC Support - 0:Not Supported, 1: 1SS, 2: 1,2SS, 3: 1,2,3SS
 * @tx_stbc: Tx STBC Support
 * @short_gi_40_mhz: Short GI Support for HT40
 * @short_gi_20_mhz: Short GI support for HT20
 * @green_field: Support for HT Greenfield PPDUs
 * @mimo_power_save: SM Power Save Mode - 0:Static, 1:Dynamic, 3:Disabled, 2:Res
 * @supported_channel_width_set: Supported Chan Width - 0:20Mhz, 1:20Mhz & 40Mhz
 * @adv_coding_cap: Rx LDPC support
 */
#ifndef ANI_LITTLE_BIT_ENDIAN
struct mlme_ht_capabilities_info {
	uint16_t l_sig_tx_op_protection:1;
	uint16_t stbc_control_frame:1;
	uint16_t psmp:1;
	uint16_t dsss_cck_mode_40_mhz:1;
	uint16_t maximal_amsdu_size:1;
	uint16_t delayed_ba:1;
	uint16_t rx_stbc:2;
	uint16_t tx_stbc:1;
	uint16_t short_gi_40_mhz:1;
	uint16_t short_gi_20_mhz:1;
	uint16_t green_field:1;
	uint16_t mimo_power_save:2;
	uint16_t supported_channel_width_set:1;
	uint16_t adv_coding_cap:1;
} qdf_packed;
#else
struct mlme_ht_capabilities_info {
	uint16_t adv_coding_cap:1;
	uint16_t supported_channel_width_set:1;
	uint16_t mimo_power_save:2;
	uint16_t green_field:1;
	uint16_t short_gi_20_mhz:1;
	uint16_t short_gi_40_mhz:1;
	uint16_t tx_stbc:1;
	uint16_t rx_stbc:2;
	uint16_t delayed_ba:1;
	uint16_t maximal_amsdu_size:1;
	uint16_t dsss_cck_mode_40_mhz:1;
	uint16_t psmp:1;
	uint16_t stbc_control_frame:1;
	uint16_t l_sig_tx_op_protection:1;
} qdf_packed;
#endif

/**
 * struct wlan_mlme_ht_caps - HT Capabilities related config items
 * @ht_cap_info: HT capabilities Info Structure
 */
struct wlan_mlme_ht_caps {
	struct mlme_ht_capabilities_info ht_cap_info;
};

struct wlan_mlme_vht_caps {
	/* VHT related configs */
};

/**
 * struct wlan_mlme_rates - RATES related config items
 * @cfpPeriod: cfp period info
 * @cfpMaxDuration: cfp Max duration info
 * @max_htmcs_txdata: max HT mcs info for Tx
 * @disable_abg_rate_txdata: disable abg rate info for tx data
 * @sap_max_mcs_txdata: sap max mcs info
 * @disable_high_ht_mcs_2x2: disable high mcs for 2x2 info
 */
struct wlan_mlme_rates {
	uint8_t cfp_period;
	uint16_t cfp_max_duration;
	uint16_t max_htmcs_txdata;
	bool disable_abg_rate_txdata;
	uint16_t sap_max_mcs_txdata;
	uint8_t disable_high_ht_mcs_2x2;
};

/*
 * struct wlan_mlme_sap_protection_cfg - SAP erp protection config items
 *
 * @protection_enabled - Force enable protection. static via cfg
 * @protection_always_11g - Force protection enable for 11g. Static via cfg
 * @protection_force_policy - Protection force policy. Static via cfg
 * @ignore_peer_ht_mode - ignore the ht opmode of the peer. Dynamic via INI.
 *
 */
struct wlan_mlme_sap_protection {
	uint32_t protection_enabled;
	uint8_t protection_force_policy;
	bool ignore_peer_ht_mode;
};

/*
 * struct wlan_mlme_chainmask - All chainmask related cfg items
 *
 * @txchainmask1x1 - to set transmit chainmask
 * @rxchainmask1x1 - to set rx chainmask
 * @tx_chain_mask_cck - Used to enable/disable Cck ChainMask
 * @tx_chain_mask_1ss - Enables/disables tx chain Mask1ss
 * @num_11b_tx_chains - Number of Tx Chains in 11b mode
 * @num_11ag_tx_chains - Number of Tx Chains in 11ag mode
 * @tx_chain_mask_2g - tx chain mask for 2g
 * @rx_chain_mask_2g - rx chain mask for 2g
 * @tx_chain_mask_5g - tx chain mask for 5g
 * @rx_chain_mask_5g - rx chain mask for 5g
 */
struct wlan_mlme_chainmask {
	uint8_t txchainmask1x1;
	uint8_t rxchainmask1x1;
	bool tx_chain_mask_cck;
	uint8_t tx_chain_mask_1ss;
	uint16_t num_11b_tx_chains;
	uint16_t num_11ag_tx_chains;
	uint8_t tx_chain_mask_2g;
	uint8_t rx_chain_mask_2g;
	uint8_t tx_chain_mask_5g;
	uint8_t rx_chain_mask_5g;
};

/**
 * struct wlan_mlme_cfg - MLME config items
 * @ht_cfg: HT related CFG Items
 * @vht_cfg: VHT related CFG Items
 * @rates: Rates related cfg items
 * @sap_protection_cfg: SAP erp protection related CFG items
 */
struct wlan_mlme_cfg {
	struct wlan_mlme_ht_caps ht_caps;
	struct wlan_mlme_vht_caps vht_caps;
	struct wlan_mlme_rates rates;
	struct wlan_mlme_sap_protection sap_protection_cfg;
	struct wlan_mlme_chainmask chainmask_cfg;
};

#endif
