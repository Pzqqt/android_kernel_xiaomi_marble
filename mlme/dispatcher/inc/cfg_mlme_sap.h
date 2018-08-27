/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_MLME_SAP_H
#define __CFG_MLME_SAP_H

#define CFG_SSID CFG_STRING( \
			"cfg_ssid", \
			0, \
			32, \
			"1,2,3,4,5,6,7,8,9,0", \
			"CFG_SSID")

#define CFG_BEACON_INTERVAL CFG_UINT( \
			"cfg_beacon_interval", \
			0, \
			65535, \
			100, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_BEACON_INTERVAL")

#define CFG_DTIM_PERIOD CFG_UINT( \
			"cfg_dtim_period", \
			0, \
			65535, \
			1, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_DTIM_PERIOD")

#define CFG_LISTEN_INTERVAL CFG_UINT( \
			"cfg_listen_interval", \
			0, \
			65535, \
			1, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_LISTEN_INTERVAL")

#define CFG_11G_ONLY_POLICY CFG_UINT( \
			"cfg_11g_only_policy", \
			0, \
			1, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_11G_ONLY_POLICY")

#define CFG_ASSOC_STA_LIMIT CFG_UINT( \
			"cfg_beacon_interval", \
			1, \
			32, \
			10, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_ASSOC_STA_LIMIT")

/*
 * <ini>
 * cfg_enable_lte_coex - enable LTE COEX
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable LTE COEX
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_ENABLE_LTE_COEX CFG_INI_BOOL( \
			"cfg_enable_lte_coex", \
			0, \
			"CFG_ENABLE_LTE_COEX")

#define CFG_RMC_ACTION_PERIOD_FREQUENCY CFG_UINT( \
			"cfg_rcm_action_period_frequency", \
			100, \
			1000, \
			300, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_RMC_ACTION_PERIOD_FREQUENCY")

/*
 * <ini>
 * cfg_rate_for_tx_mgmt - Set rate for tx mgmt
 * @Min: 0
 * @Max: 0xFF
 * @Default: 0xFF
 *
 * This ini is used to set rate for tx mgmt
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_RATE_FOR_TX_MGMT CFG_INI_UINT( \
			"cfg_rate_for_tx_mgmt", \
			0, \
			0xFF, \
			0xFF, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_RATE_FOR_TX_MGMT")

/*
 * <ini>
 * cfg_rate_for_tx_mgmt_2g - Set rate for tx mgmt 2g
 * @Min: 0
 * @Max: 255
 * @Default: 255
 *
 * This ini is used to set rate for tx mgmt 2g
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_RATE_FOR_TX_MGMT_2G CFG_INI_UINT( \
			"cfg_rate_for_tx_mgmt_2g", \
			0, \
			255, \
			255, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_RATE_FOR_TX_MGMT_2G")

/*
 * <ini>
 * cfg_rate_for_tx_mgmt_5g - Set rate for tx mgmt 5g
 * @Min: 0
 * @Max: 255
 * @Default: 255
 *
 * This ini is used to set rate for tx mgmt 5g
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_RATE_FOR_TX_MGMT_5G CFG_INI_UINT( \
			"cfg_rate_for_tx_mgmt_5g", \
			0, \
			255, \
			255, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_RATE_FOR_TX_MGMT_5G")

/*
 * <ini>
 * gTelescopicBeaconWakeupEn - Set teles copic beacon wakeup
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default teles copic beacon wakeup
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_INI_TELE_BCN_WAKEUP_EN CFG_INI_BOOL( \
			"gTelescopicBeaconWakeupEn", \
			0, \
			"CFG_TELE_BCN_WAKEUP_EN")

/*
 * <ini>
 * telescopicBeaconMaxListenInterval - Set teles scopic beacon max listen value
 * @Min: 0
 * @Max: 7
 * @Default: 5
 *
 * This ini is used to set teles scopic beacon max listen interval value
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_TELE_BCN_MAX_LI CFG_INI_UINT( \
			"telescopicBeaconMaxListenInterval", \
			0, \
			7, \
			5, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_TELE_BCN_MAX_LI")

/*
 * <ini>
 * gSapGetPeerInfo - Enable/Disable remote peer info query support
 * @Min: 0 - Disable remote peer info query support
 * @Max: 1 - Enable remote peer info query support
 * @Default: 0
 *
 * This ini is used to enable/disable remote peer info query support
 *
 * Usage: External
 *
 * </ini>
 */
 #define CFG_INI_SAP_GET_PEER_INFO CFG_INI_BOOL( \
			"gSapGetPeerInfo", \
			0, \
			"CFG_INI_SAP_GET_PEER_INFO")

/*
 * <ini>
 * gSapAllowAllChannel - Sap allow all channels
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to allow all channels for SAP
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_SAP_ALLOW_ALL_CHANNEL_PARAM CFG_INI_BOOL( \
			"gSapAllowAllChannel", \
			0, \
			"CFG_INI_SAP_ALLOW_ALL_CHANNEL_PARAM")

/*
 * <ini>
 * gSoftApMaxPeers - Set Max peers connected for SAP
 * @Min: 1
 * @Max: 32
 * @Default: 32
 *
 * This ini is used to set Max peers connected for SAP
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_SAP_MAX_NO_PEERS CFG_INI_UINT( \
			"gSoftApMaxPeers", \
			1, \
			32, \
			32, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_SAP_MAX_NO_PEERS")

/*
 * <ini>
 * gMaxOffloadPeers - Set max offload peers
 * @Min: 2
 * @Max: 5
 * @Default: 2
 *
 * This ini is used to set default teles copic beacon wakeup
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_SAP_MAX_OFFLOAD_PEERS CFG_INI_UINT( \
			"gMaxOffloadPeers", \
			2, \
			5, \
			2, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_SAP_MAX_OFFLOAD_PEERS")

/*
 * <ini>
 * gMaxOffloadReorderBuffs - Set max offload reorder buffs
 * @Min: 0
 * @Max: 3
 * @Default: 2
 *
 * This ini is used to set max offload reorder buffs
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_SAP_MAX_OFFLOAD_REORDER_BUFFS CFG_INI_UINT( \
			"gMaxOffloadReorderBuffs", \
			0, \
			3, \
			2, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_SAP_MAX_OFFLOAD_REORDER_BUFFS")

/*
 * <ini>
 * g_sap_chanswitch_beacon_cnt - Set channel switch beacon count
 * @Min: 1
 * @Max: 10
 * @Default: 10
 *
 * This ini is used to set channel switch beacon count
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_SAP_CH_SWITCH_BEACON_CNT CFG_INI_UINT( \
			"g_sap_chanswitch_beacon_cnt", \
			1, \
			10, \
			10, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_SAP_CH_SWITCH_BEACON_CNT")

/*
 * <ini>
 * g_sap_chanswitch_mode - channel switch mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to configure channel switch mode
 *
 * Related: none
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_INI_SAP_CH_SWITCH_MODE CFG_INI_BOOL( \
			"g_sap_chanswitch_mode", \
			1, \
			"CFG_INI_SAP_CH_SWITCH_MODE")

/*
 * <ini>
 * gEnableSapInternalRestart - Sap internal restart name
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used for sap internal restart name
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_SAP_INTERNAL_RESTART_NAME CFG_INI_BOOL( \
			"gEnableSapInternalRestart", \
			1, \
			"CFG_SAP_INTERNAL_RESTART_NAME")

/*
 * <ini>
 * gChanSwitchHostapdRateEnabled - Enable channale switch hostapd rate
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable channale switch hostapd rate
 *
 * Related: None
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
 #define CFG_INI_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_NAME CFG_INI_BOOL( \
			"gChanSwitchHostapdRateEnabled", \
			0, \
			"CFG_INI_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_NAME")

/*
 * gReducedBeaconInterval - beacon interval reduced
 * @Min: 0
 * @Max: 100
 * @Default: 0
 *
 * This ini is used to reduce beacon interval before channel
 * switch (when val great than 0, or the feature is disabled).
 * It would reduce the downtime on the STA side which is
 * waiting for beacons from the AP to resume back transmission.
 * Switch back the beacon_interval to its original value after
 * channel switch based on the timeout.
 *
 * Related: none
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_INI_REDUCED_BEACON_INTERVAL CFG_INI_UINT( \
			"gReducedBeaconInterval", \
			0, \
			100, \
			0, \
			CFG_VALUE_OR_DEFAULT, \
			"CFG_INI_REDUCED_BEACON_INTERVAL")

 #define CFG_SAP_ALL \
	CFG(CFG_SSID) \
	CFG(CFG_BEACON_INTERVAL) \
	CFG(CFG_DTIM_PERIOD) \
	CFG(CFG_LISTEN_INTERVAL) \
	CFG(CFG_11G_ONLY_POLICY) \
	CFG(CFG_ASSOC_STA_LIMIT) \
	CFG(CFG_INI_ENABLE_LTE_COEX) \
	CFG(CFG_RMC_ACTION_PERIOD_FREQUENCY) \
	CFG(CFG_INI_RATE_FOR_TX_MGMT) \
	CFG(CFG_INI_RATE_FOR_TX_MGMT_2G) \
	CFG(CFG_INI_RATE_FOR_TX_MGMT_5G) \
	CFG(CFG_INI_TELE_BCN_WAKEUP_EN) \
	CFG(CFG_INI_TELE_BCN_MAX_LI) \
	CFG(CFG_INI_SAP_GET_PEER_INFO) \
	CFG(CFG_INI_SAP_ALLOW_ALL_CHANNEL_PARAM) \
	CFG(CFG_INI_SAP_MAX_NO_PEERS) \
	CFG(CFG_INI_SAP_MAX_OFFLOAD_PEERS) \
	CFG(CFG_INI_SAP_MAX_OFFLOAD_REORDER_BUFFS) \
	CFG(CFG_INI_SAP_CH_SWITCH_BEACON_CNT) \
	CFG(CFG_INI_SAP_CH_SWITCH_MODE) \
	CFG(CFG_INI_SAP_INTERNAL_RESTART_NAME) \
	CFG(CFG_INI_CHAN_SWITCH_HOSTAPD_RATE_ENABLED_NAME) \
	CFG(CFG_INI_REDUCED_BEACON_INTERVAL)

#endif /* __CFG_MLME_SAP_H */
