/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_POLICY_MGR_PUBLIC_STRUCT_H
#define __WLAN_POLICY_MGR_PUBLIC_STRUCT_H

/**
 * DOC: wlan_policy_mgr_public_struct.h
 *
 * Concurrenct Connection Management entity
 */

/* Include files */
#include <wmi_unified_api.h>

#define POLICY_MGR_MAX_CHANNEL_LIST 128

/**
 *  Some max value greater than the max length of the channel list
 */
#define MAX_WEIGHT_OF_PCL_CHANNELS 255
/**
 *  Some fixed weight difference between the groups
 */
#define PCL_GROUPS_WEIGHT_DIFFERENCE 20

/**
 * Currently max, only 3 groups are possible as per 'enum policy_mgr_pcl_type'.
 * i.e., in a PCL only 3 groups of channels can be present
 * e.g., SCC channel on 2.4 Ghz, SCC channel on 5 Ghz & 5 Ghz channels.
 * Group 1 has highest priority, group 2 has the next higher priority
 * and so on.
 */
#define WEIGHT_OF_GROUP1_PCL_CHANNELS MAX_WEIGHT_OF_PCL_CHANNELS
#define WEIGHT_OF_GROUP2_PCL_CHANNELS \
	(WEIGHT_OF_GROUP1_PCL_CHANNELS - PCL_GROUPS_WEIGHT_DIFFERENCE)
#define WEIGHT_OF_GROUP3_PCL_CHANNELS \
	(WEIGHT_OF_GROUP2_PCL_CHANNELS - PCL_GROUPS_WEIGHT_DIFFERENCE)
#define WEIGHT_OF_GROUP4_PCL_CHANNELS \
	(WEIGHT_OF_GROUP3_PCL_CHANNELS - PCL_GROUPS_WEIGHT_DIFFERENCE)

#define WEIGHT_OF_NON_PCL_CHANNELS 1
#define WEIGHT_OF_DISALLOWED_CHANNELS 0

/**
 * enum hw_mode_ss_config - Possible spatial stream configuration
 * @HW_MODE_SS_0x0: Unused Tx and Rx of MAC
 * @HW_MODE_SS_1x1: 1 Tx SS and 1 Rx SS
 * @HW_MODE_SS_2x2: 2 Tx SS and 2 Rx SS
 * @HW_MODE_SS_3x3: 3 Tx SS and 3 Rx SS
 * @HW_MODE_SS_4x4: 4 Tx SS and 4 Rx SS
 *
 * Note: Right now only 1x1 and 2x2 are being supported. Other modes should
 * be added when supported. Asymmetric configuration like 1x2, 2x1 are also
 * not supported now. But, they are still valid. Right now, Tx/Rx SS support is
 * 4 bits long. So, we can go upto 15x15
 */
enum hw_mode_ss_config {
	HW_MODE_SS_0x0,
	HW_MODE_SS_1x1,
	HW_MODE_SS_2x2,
	HW_MODE_SS_3x3,
	HW_MODE_SS_4x4,
};

/**
 * enum hw_mode_dbs_capab - DBS HW mode capability
 * @HW_MODE_DBS_NONE: Non DBS capable
 * @HW_MODE_DBS: DBS capable
 */
enum hw_mode_dbs_capab {
	HW_MODE_DBS_NONE,
	HW_MODE_DBS,
};

/**
 * enum hw_mode_agile_dfs_capab - Agile DFS HW mode capability
 * @HW_MODE_AGILE_DFS_NONE: Non Agile DFS capable
 * @HW_MODE_AGILE_DFS: Agile DFS capable
 */
enum hw_mode_agile_dfs_capab {
	HW_MODE_AGILE_DFS_NONE,
	HW_MODE_AGILE_DFS,
};

/**
 * enum hw_mode_sbs_capab - SBS HW mode capability
 * @HW_MODE_SBS_NONE: Non SBS capable
 * @HW_MODE_SBS: SBS capable
 */
enum hw_mode_sbs_capab {
	HW_MODE_SBS_NONE,
	HW_MODE_SBS,
};

/**
 * enum policy_mgr_pcl_group_id - Identifies the pcl groups to be used
 * @POLICY_MGR_PCL_GROUP_ID1_ID2: Use weights of group1 and group2
 * @POLICY_MGR_PCL_GROUP_ID2_ID3: Use weights of group2 and group3
 * @POLICY_MGR_PCL_GROUP_ID3_ID4: Use weights of group3 and group4
 *
 * Since maximum of three groups are possible, this will indicate which
 * PCL group needs to be used.
 */
enum policy_mgr_pcl_group_id {
	POLICY_MGR_PCL_GROUP_ID1_ID2,
	POLICY_MGR_PCL_GROUP_ID2_ID3,
	POLICY_MGR_PCL_GROUP_ID3_ID4,
};

/**
 * policy_mgr_pcl_channel_order - Order in which the PCL is requested
 * @POLICY_MGR_PCL_ORDER_NONE: no order
 * @POLICY_MGR_PCL_ORDER_24G_THEN_5G: 2.4 Ghz channel followed by 5 Ghz channel
 * @POLICY_MGR_PCL_ORDER_5G_THEN_2G: 5 Ghz channel followed by 2.4 Ghz channel
 *
 * Order in which the PCL is requested
 */
enum policy_mgr_pcl_channel_order {
	POLICY_MGR_PCL_ORDER_NONE,
	POLICY_MGR_PCL_ORDER_24G_THEN_5G,
	POLICY_MGR_PCL_ORDER_5G_THEN_2G,
};

/**
 * enum policy_mgr_max_rx_ss - Maximum number of receive spatial streams
 * @POLICY_MGR_RX_NSS_1: Receive Nss = 1
 * @POLICY_MGR_RX_NSS_2: Receive Nss = 2
 * @POLICY_MGR_RX_NSS_3: Receive Nss = 3
 * @POLICY_MGR_RX_NSS_4: Receive Nss = 4
 * @POLICY_MGR_RX_NSS_5: Receive Nss = 5
 * @POLICY_MGR_RX_NSS_6: Receive Nss = 6
 * @POLICY_MGR_RX_NSS_7: Receive Nss = 7
 * @POLICY_MGR_RX_NSS_8: Receive Nss = 8
 *
 * Indicates the maximum number of spatial streams that the STA can receive
 */
enum policy_mgr_max_rx_ss {
	POLICY_MGR_RX_NSS_1 = 0,
	POLICY_MGR_RX_NSS_2 = 1,
	POLICY_MGR_RX_NSS_3 = 2,
	POLICY_MGR_RX_NSS_4 = 3,
	POLICY_MGR_RX_NSS_5 = 4,
	POLICY_MGR_RX_NSS_6 = 5,
	POLICY_MGR_RX_NSS_7 = 6,
	POLICY_MGR_RX_NSS_8 = 7,
	POLICY_MGR_RX_NSS_MAX,
};

/**
 * enum policy_mgr_chain_mode - Chain Mask tx & rx combination.
 *
 * @POLICY_MGR_ONE_ONE: One for Tx, One for Rx
 * @POLICY_MGR_TWO_TWO: Two for Tx, Two for Rx
 * @POLICY_MGR_MAX_NO_OF_CHAIN_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_chain_mode {
	POLICY_MGR_ONE_ONE = 0,
	POLICY_MGR_TWO_TWO,
	POLICY_MGR_MAX_NO_OF_CHAIN_MODE
};

/**
 * enum policy_mgr_conc_priority_mode - t/p, powersave, latency.
 *
 * @PM_THROUGHPUT: t/p is the priority
 * @PM_POWERSAVE: powersave is the priority
 * @PM_LATENCY: latency is the priority
 * @PM_MAX_CONC_PRIORITY_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_conc_priority_mode {
	PM_THROUGHPUT = 0,
	PM_POWERSAVE,
	PM_LATENCY,
	PM_MAX_CONC_PRIORITY_MODE
};

/**
 * enum policy_mgr_con_mode - concurrency mode for PCL table
 *
 * @PM_STA_MODE: station mode
 * @PM_SAP_MODE: SAP mode
 * @PM_P2P_CLIENT_MODE: P2P client mode
 * @PM_P2P_GO_MODE: P2P Go mode
 * @PM_IBSS_MODE: IBSS mode
 * @PM_MAX_NUM_OF_MODE: max value place holder
 */
enum policy_mgr_con_mode {
	PM_STA_MODE = 0,
	PM_SAP_MODE,
	PM_P2P_CLIENT_MODE,
	PM_P2P_GO_MODE,
	PM_IBSS_MODE,
	PM_MAX_NUM_OF_MODE
};

/**
 * enum policy_mgr_mac_use - MACs that are used
 * @POLICY_MGR_MAC0: Only MAC0 is used
 * @POLICY_MGR_MAC1: Only MAC1 is used
 * @POLICY_MGR_MAC0_AND_MAC1: Both MAC0 and MAC1 are used
 */
enum policy_mgr_mac_use {
	POLICY_MGR_MAC0 = 1,
	POLICY_MGR_MAC1 = 2,
	POLICY_MGR_MAC0_AND_MAC1 = 3
};

/**
 * enum policy_mgr_pcl_type - Various types of Preferred channel list (PCL).
 *
 * @PM_NONE: No channel preference
 * @PM_24G: 2.4 Ghz channels only
 * @PM_5G: 5 Ghz channels only
 * @PM_SCC_CH: SCC channel only
 * @PM_MCC_CH: MCC channels only
 * @PM_SBS_CH: SBS channels only
 * @PM_SCC_CH_24G: SCC channel & 2.4 Ghz channels
 * @PM_SCC_CH_5G: SCC channel & 5 Ghz channels
 * @PM_24G_SCC_CH: 2.4 Ghz channels & SCC channel
 * @PM_5G_SCC_CH: 5 Ghz channels & SCC channel
 * @PM_SCC_ON_5_SCC_ON_24_24G: SCC channel on 5 Ghz, SCC
 *	channel on 2.4 Ghz & 2.4 Ghz channels
 * @PM_SCC_ON_5_SCC_ON_24_5G: SCC channel on 5 Ghz, SCC channel
 *	on 2.4 Ghz & 5 Ghz channels
 * @PM_SCC_ON_24_SCC_ON_5_24G: SCC channel on 2.4 Ghz, SCC
 *	channel on 5 Ghz & 2.4 Ghz channels
 * @PM_SCC_ON_24_SCC_ON_5_5G: SCC channel on 2.4 Ghz, SCC
 *	channel on 5 Ghz & 5 Ghz channels
 * @PM_SCC_ON_5_SCC_ON_24: SCC channel on 5 Ghz, SCC channel on
 *	2.4 Ghz
 * @PM_SCC_ON_24_SCC_ON_5: SCC channel on 2.4 Ghz, SCC channel
 *	on 5 Ghz
 * @PM_MCC_CH_24G: MCC channels & 2.4 Ghz channels
 * @PM_MCC_CH_5G:  MCC channels & 5 Ghz channels
 * @PM_24G_MCC_CH: 2.4 Ghz channels & MCC channels
 * @PM_5G_MCC_CH: 5 Ghz channels & MCC channels
 * @PM_SBS_CH_5G: SBS channels & rest of 5 Ghz channels
 * @PM_24G_SCC_CH_SBS_CH: 2.4 Ghz channels, SCC channel & SBS channels
 * @PM_24G_SCC_CH_SBS_CH_5G: 2.4 Ghz channels, SCC channel,
 *      SBS channels & rest of the 5G channels
 * @PM_24G_SBS_CH_MCC_CH: 2.4 Ghz channels, SBS channels & MCC channels
 * @PM_MAX_PCL_TYPE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_pcl_type {
	PM_NONE = 0,
	PM_24G,
	PM_5G,
	PM_SCC_CH,
	PM_MCC_CH,
	PM_SBS_CH,
	PM_SCC_CH_24G,
	PM_SCC_CH_5G,
	PM_24G_SCC_CH,
	PM_5G_SCC_CH,
	PM_SCC_ON_5_SCC_ON_24_24G,
	PM_SCC_ON_5_SCC_ON_24_5G,
	PM_SCC_ON_24_SCC_ON_5_24G,
	PM_SCC_ON_24_SCC_ON_5_5G,
	PM_SCC_ON_5_SCC_ON_24,
	PM_SCC_ON_24_SCC_ON_5,
	PM_MCC_CH_24G,
	PM_MCC_CH_5G,
	PM_24G_MCC_CH,
	PM_5G_MCC_CH,
	PM_SBS_CH_5G,
	PM_24G_SCC_CH_SBS_CH,
	PM_24G_SCC_CH_SBS_CH_5G,
	PM_24G_SBS_CH_MCC_CH,

	PM_MAX_PCL_TYPE
};

/**
 * enum policy_mgr_one_connection_mode - Combination of first connection
 * type, band & spatial stream used.
 *
 * @PM_STA_24_1x1: STA connection using 1x1@2.4 Ghz
 * @PM_STA_24_2x2: STA connection using 2x2@2.4 Ghz
 * @PM_STA_5_1x1: STA connection using 1x1@5 Ghz
 * @PM_STA_5_2x2: STA connection using 2x2@5 Ghz
 * @PM_P2P_CLI_24_1x1: P2P Client connection using 1x1@2.4 Ghz
 * @PM_P2P_CLI_24_2x2: P2P Client connection using 2x2@2.4 Ghz
 * @PM_P2P_CLI_5_1x1: P2P Client connection using 1x1@5 Ghz
 * @PM_P2P_CLI_5_2x2: P2P Client connection using 2x2@5 Ghz
 * @PM_P2P_GO_24_1x1: P2P GO connection using 1x1@2.4 Ghz
 * @PM_P2P_GO_24_2x2: P2P GO connection using 2x2@2.4 Ghz
 * @PM_P2P_GO_5_1x1: P2P GO connection using 1x1@5 Ghz
 * @PM_P2P_GO_5_2x2: P2P GO connection using 2x2@5 Ghz
 * @PM_SAP_24_1x1: SAP connection using 1x1@2.4 Ghz
 * @PM_SAP_24_2x2: SAP connection using 2x2@2.4 Ghz
 * @PM_SAP_5_1x1: SAP connection using 1x1@5 Ghz
 * @PM_SAP_5_1x1: SAP connection using 2x2@5 Ghz
 * @PM_IBSS_24_1x1:  IBSS connection using 1x1@2.4 Ghz
 * @PM_IBSS_24_2x2:  IBSS connection using 2x2@2.4 Ghz
 * @PM_IBSS_5_1x1:  IBSS connection using 1x1@5 Ghz
 * @PM_IBSS_5_2x2:  IBSS connection using 2x2@5 Ghz
 * @PM_MAX_ONE_CONNECTION_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_one_connection_mode {
	PM_STA_24_1x1 = 0,
	PM_STA_24_2x2,
	PM_STA_5_1x1,
	PM_STA_5_2x2,
	PM_P2P_CLI_24_1x1,
	PM_P2P_CLI_24_2x2,
	PM_P2P_CLI_5_1x1,
	PM_P2P_CLI_5_2x2,
	PM_P2P_GO_24_1x1,
	PM_P2P_GO_24_2x2,
	PM_P2P_GO_5_1x1,
	PM_P2P_GO_5_2x2,
	PM_SAP_24_1x1,
	PM_SAP_24_2x2,
	PM_SAP_5_1x1,
	PM_SAP_5_2x2,
	PM_IBSS_24_1x1,
	PM_IBSS_24_2x2,
	PM_IBSS_5_1x1,
	PM_IBSS_5_2x2,

	PM_MAX_ONE_CONNECTION_MODE
};

/**
 * enum policy_mgr_two_connection_mode - Combination of first two
 * connections type, concurrency state, band & spatial stream
 * used.
 *
 * @PM_STA_SAP_SCC_24_1x1: STA & SAP connection on SCC using
 *			1x1@2.4 Ghz
 * @PM_STA_SAP_SCC_24_2x2: STA & SAP connection on SCC using
 *			2x2@2.4 Ghz
 * @PM_STA_SAP_MCC_24_1x1: STA & SAP connection on MCC using
 *			1x1@2.4 Ghz
 * @PM_STA_SAP_MCC_24_2x2: STA & SAP connection on MCC using
 *			2x2@2.4 Ghz
 * @PM_STA_SAP_SCC_5_1x1: STA & SAP connection on SCC using
 *			1x1@5 Ghz
 * @PM_STA_SAP_SCC_5_2x2: STA & SAP connection on SCC using
 *			2x2@5 Ghz
 * @PM_STA_SAP_MCC_5_1x1: STA & SAP connection on MCC using
 *			1x1@5 Ghz
 * @PM_STA_SAP_MCC_5_2x2: STA & SAP connection on MCC using
 *			2x2@5 Ghz
 * @PM_STA_SAP_DBS_1x1: STA & SAP connection on DBS using 1x1
 * @PM_STA_SAP_DBS_2x2: STA & SAP connection on DBS using 2x2
 * @PM_STA_SAP_SBS_5_1x1: STA & SAP connection on 5G SBS using 1x1
 * @PM_STA_P2P_GO_SCC_24_1x1: STA & P2P GO connection on SCC
 *			using 1x1@2.4 Ghz
 * @PM_STA_P2P_GO_SCC_24_2x2: STA & P2P GO connection on SCC
 *			using 2x2@2.4 Ghz
 * @PM_STA_P2P_GO_MCC_24_1x1: STA & P2P GO connection on MCC
 *			using 1x1@2.4 Ghz
 * @PM_STA_P2P_GO_MCC_24_2x2: STA & P2P GO connection on MCC
 *			using 2x2@2.4 Ghz
 * @PM_STA_P2P_GO_SCC_5_1x1: STA & P2P GO connection on SCC
 *			using 1x1@5 Ghz
 * @PM_STA_P2P_GO_SCC_5_2x2: STA & P2P GO connection on SCC
 *			using 2x2@5 Ghz
 * @PM_STA_P2P_GO_MCC_5_1x1: STA & P2P GO connection on MCC
 *			using 1x1@5 Ghz
 * @PM_STA_P2P_GO_MCC_5_2x2: STA & P2P GO connection on MCC
 *			using 2x2@5 Ghz
 * @PM_STA_P2P_GO_DBS_1x1: STA & P2P GO connection on DBS using
 *			1x1
 * @PM_STA_P2P_GO_DBS_2x2: STA & P2P GO connection on DBS using
 *			2x2
 * @PM_STA_P2P_GO_SBS_5_1x1: STA & P2P GO connection on 5G SBS
 *			using 1x1
 * @PM_STA_P2P_CLI_SCC_24_1x1: STA & P2P CLI connection on SCC
 *			using 1x1@2.4 Ghz
 * @PM_STA_P2P_CLI_SCC_24_2x2: STA & P2P CLI connection on SCC
 *			using 2x2@2.4 Ghz
 * @PM_STA_P2P_CLI_MCC_24_1x1: STA & P2P CLI connection on MCC
 *			using 1x1@2.4 Ghz
 * @PM_STA_P2P_CLI_MCC_24_2x2: STA & P2P CLI connection on MCC
 *			using 2x2@2.4 Ghz
 * @PM_STA_P2P_CLI_SCC_5_1x1: STA & P2P CLI connection on SCC
 *			using 1x1@5 Ghz
 * @PM_STA_P2P_CLI_SCC_5_2x2: STA & P2P CLI connection on SCC
 *			using 2x2@5 Ghz
 * @PM_STA_P2P_CLI_MCC_5_1x1: STA & P2P CLI connection on MCC
 *			using 1x1@5 Ghz
 * @PM_STA_P2P_CLI_MCC_5_2x2: STA & P2P CLI connection on MCC
 *			using 2x2@5 Ghz
 * @PM_STA_P2P_CLI_DBS_1x1: STA & P2P CLI connection on DBS
 *			using 1x1
 * @PM_STA_P2P_CLI_DBS_2x2: STA & P2P CLI connection on DBS
 *			using 2x2
 * @PM_STA_P2P_CLI_SBS_5_1x1: STA & P2P CLI connection on 5G
 *			SBS using 1x1
 * @PM_P2P_GO_P2P_CLI_SCC_24_1x1: P2P GO & CLI connection on
 *			SCC using 1x1@2.4 Ghz
 * @PM_P2P_GO_P2P_CLI_SCC_24_2x2: P2P GO & CLI connection on
 *			SCC using 2x2@2.4 Ghz
 * @PM_P2P_GO_P2P_CLI_MCC_24_1x1: P2P GO & CLI connection on
 *			MCC using 1x1@2.4 Ghz
 * @PM_P2P_GO_P2P_CLI_MCC_24_2x2: P2P GO & CLI connection on
 *			MCC using 2x2@2.4 Ghz
 * @PM_P2P_GO_P2P_CLI_SCC_5_1x1: P2P GO & CLI connection on
 *			SCC using 1x1@5 Ghz
 * @PM_P2P_GO_P2P_CLI_SCC_5_2x2: P2P GO & CLI connection on
 *			SCC using 2x2@5 Ghz
 * @PM_P2P_GO_P2P_CLI_MCC_5_1x1: P2P GO & CLI connection on
 *			MCC using 1x1@5 Ghz
 * @PM_P2P_GO_P2P_CLI_MCC_5_2x2: P2P GO & CLI connection on
 *			MCC using 2x2@5 Ghz
 * @PM_P2P_GO_P2P_CLI_DBS_1x1: P2P GO & CLI connection on DBS
 *			using 1x1
 * @PM_P2P_GO_P2P_CLI_DBS_2x2: P2P GO & P2P CLI connection
 *			on DBS using 2x2
 * @PM_P2P_GO_P2P_CLI_SBS_5_1x1: P2P GO & P2P CLI connection
 *			on 5G SBS using 1x1
 * @PM_P2P_GO_SAP_SCC_24_1x1: P2P GO & SAP connection on
 *			SCC using 1x1@2.4 Ghz
 * @PM_P2P_GO_SAP_SCC_24_2x2: P2P GO & SAP connection on
 *			SCC using 2x2@2.4 Ghz
 * @PM_P2P_GO_SAP_MCC_24_1x1: P2P GO & SAP connection on
 *			MCC using 1x1@2.4 Ghz
 * @PM_P2P_GO_SAP_MCC_24_2x2: P2P GO & SAP connection on
 *			MCC using 2x2@2.4 Ghz
 * @PM_P2P_GO_SAP_SCC_5_1x1: P2P GO & SAP connection on
 *			SCC using 1x1@5 Ghz
 * @PM_P2P_GO_SAP_SCC_5_2x2: P2P GO & SAP connection on
 *			SCC using 2x2@5 Ghz
 * @PM_P2P_GO_SAP_MCC_5_1x1: P2P GO & SAP connection on
 *			MCC using 1x1@5 Ghz
 * @PM_P2P_GO_SAP_MCC_5_2x2: P2P GO & SAP connection on
 *			MCC using 2x2@5 Ghz
 * @PM_P2P_GO_SAP_DBS_1x1: P2P GO & SAP connection on DBS using
 *			1x1
 * @PM_P2P_GO_SAP_DBS_2x2: P2P GO & SAP connection on DBS using
 *			2x2
 * @PM_P2P_GO_SAP_SBS_5_1x1: P2P GO & SAP connection on 5G SBS
 *			using 1x1
 * @PM_P2P_CLI_SAP_SCC_24_1x1: CLI & SAP connection on SCC using
 *			1x1@2.4 Ghz
 * @PM_P2P_CLI_SAP_SCC_24_2x2: CLI & SAP connection on SCC using
 *			2x2@2.4 Ghz
 * @PM_P2P_CLI_SAP_MCC_24_1x1: CLI & SAP connection on MCC using
 *			1x1@2.4 Ghz
 * @PM_P2P_CLI_SAP_MCC_24_2x2: CLI & SAP connection on MCC using
 *			2x2@2.4 Ghz
 * @PM_P2P_CLI_SAP_SCC_5_1x1: CLI & SAP connection on SCC using
 *			1x1@5 Ghz
 * @PM_P2P_CLI_SAP_SCC_5_2x2: CLI & SAP connection on SCC using
 *			2x2@5 Ghz
 * @PM_P2P_CLI_SAP_MCC_5_1x1: CLI & SAP connection on MCC using
 *			1x1@5 Ghz
 * @PM_P2P_CLI_SAP_MCC_5_2x2: CLI & SAP connection on MCC using
 *			2x2@5 Ghz
 * @POLICY_MGR_P2P_STA_SAP_MCC_24_5_1x1: CLI and SAP connecting on MCC
 *			in 2.4 and 5GHz 1x1
 * @POLICY_MGR_P2P_STA_SAP_MCC_24_5_2x2: CLI and SAP connecting on MCC
 *			in 2.4 and 5GHz 2x2
 * @PM_P2P_CLI_SAP_DBS_1x1,: CLI & SAP connection on DBS using 1x1
 * @PM_P2P_CLI_SAP_DBS_2x2: P2P CLI & SAP connection on DBS using
 *			2x2
 * @PM_P2P_CLI_SAP_SBS_5_1x1: P2P CLI & SAP connection on 5G SBS
 *			using 1x1
 * @PM_SAP_SAP_SCC_24_1x1: SAP & SAP connection on
 *			SCC using 1x1@2.4 Ghz
 * @PM_SAP_SAP_SCC_24_2x2: SAP & SAP connection on
 *			SCC using 2x2@2.4 Ghz
 * @PM_SAP_SAP_MCC_24_1x1: SAP & SAP connection on
 *			MCC using 1x1@2.4 Ghz
 * @PM_SAP_SAP_MCC_24_2x2: SAP & SAP connection on
 *			MCC using 2x2@2.4 Ghz
 * @PM_SAP_SAP_SCC_5_1x1: SAP & SAP connection on
 *			SCC using 1x1@5 Ghz
 * @PM_SAP_SAP_SCC_5_2x2: SAP & SAP connection on
 *			SCC using 2x2@5 Ghz
 * @PM_SAP_SAP_MCC_5_1x1: SAP & SAP connection on
 *			MCC using 1x1@5 Ghz
 * @PM_SAP_SAP_MCC_5_2x2: SAP & SAP connection on
 *          MCC using 2x2@5 Ghz
 * @PM_SAP_SAP_MCC_24_5_1x1: SAP & SAP connection on
 *			MCC in 2.4 and 5GHz 1x1
 * @PM_SAP_SAP_MCC_24_5_2x2: SAP & SAP connection on
 *			MCC in 2.4 and 5GHz 2x2
 * @PM_SAP_SAP_DBS_1x1: SAP & SAP connection on DBS using
 *			1x1
 * @PM_SAP_SAP_DBS_2x2: SAP & SAP connection on DBS using 2x2
 * @PM_SAP_SAP_SBS_5_1x1: SAP & SAP connection on 5G SBS using 1x1
 * @PM_STA_STA_SCC_24_1x1: STA & STA connection on
 *			SCC using 1x1@2.4 Ghz
 * @PM_STA_STA_SCC_24_2x2: STA & STA connection on
 *			SCC using 2x2@2.4 Ghz
 * @PM_STA_STA_MCC_24_1x1: STA & STA connection on
 *			MCC using 1x1@2.4 Ghz
 * @PM_STA_STA_MCC_24_2x2: STA & STA connection on
 *			MCC using 2x2@2.4 Ghz
 * @PM_STA_STA_SCC_5_1x1: STA & STA connection on
 *			SCC using 1x1@5 Ghz
 * @PM_STA_STA_SCC_5_2x2: STA & STA connection on
 *			SCC using 2x2@5 Ghz
 * @PM_STA_STA_MCC_5_1x1: STA & STA connection on
 *			MCC using 1x1@5 Ghz
 * @PM_STA_STA_MCC_5_2x2: STA & STA connection on
 *          MCC using 2x2@5 Ghz
 * @PM_STA_STA_MCC_24_5_1x1: STA & STA connection on
 *			MCC in 2.4 and 5GHz 1x1
 * @PM_STA_STA_MCC_24_5_2x2: STA & STA connection on
 *			MCC in 2.4 and 5GHz 2x2
 * @PM_STA_STA_DBS_1x1: STA & STA connection on DBS using
 *			1x1
 * @PM_STA_STA_DBS_2x2: STA & STA connection on DBS using 2x2
 * @PM_STA_STA_SBS_5_1x1: STA & STA connection on 5G SBS using 1x1
 *
 * These are generic IDs that identify the various roles in the
 * software system
 */
enum policy_mgr_two_connection_mode {
	PM_STA_SAP_SCC_24_1x1 = 0,
	PM_STA_SAP_SCC_24_2x2,
	PM_STA_SAP_MCC_24_1x1,
	PM_STA_SAP_MCC_24_2x2,
	PM_STA_SAP_SCC_5_1x1,
	PM_STA_SAP_SCC_5_2x2,
	PM_STA_SAP_MCC_5_1x1,
	PM_STA_SAP_MCC_5_2x2,
	PM_STA_SAP_MCC_24_5_1x1,
	PM_STA_SAP_MCC_24_5_2x2,
	PM_STA_SAP_DBS_1x1,
	PM_STA_SAP_DBS_2x2,
	PM_STA_SAP_SBS_5_1x1,
	PM_STA_P2P_GO_SCC_24_1x1,
	PM_STA_P2P_GO_SCC_24_2x2,
	PM_STA_P2P_GO_MCC_24_1x1,
	PM_STA_P2P_GO_MCC_24_2x2,
	PM_STA_P2P_GO_SCC_5_1x1,
	PM_STA_P2P_GO_SCC_5_2x2,
	PM_STA_P2P_GO_MCC_5_1x1,
	PM_STA_P2P_GO_MCC_5_2x2,
	PM_STA_P2P_GO_MCC_24_5_1x1,
	PM_STA_P2P_GO_MCC_24_5_2x2,
	PM_STA_P2P_GO_DBS_1x1,
	PM_STA_P2P_GO_DBS_2x2,
	PM_STA_P2P_GO_SBS_5_1x1,
	PM_STA_P2P_CLI_SCC_24_1x1,
	PM_STA_P2P_CLI_SCC_24_2x2,
	PM_STA_P2P_CLI_MCC_24_1x1,
	PM_STA_P2P_CLI_MCC_24_2x2,
	PM_STA_P2P_CLI_SCC_5_1x1,
	PM_STA_P2P_CLI_SCC_5_2x2,
	PM_STA_P2P_CLI_MCC_5_1x1,
	PM_STA_P2P_CLI_MCC_5_2x2,
	PM_STA_P2P_CLI_MCC_24_5_1x1,
	PM_STA_P2P_CLI_MCC_24_5_2x2,
	PM_STA_P2P_CLI_DBS_1x1,
	PM_STA_P2P_CLI_DBS_2x2,
	PM_STA_P2P_CLI_SBS_5_1x1,
	PM_P2P_GO_P2P_CLI_SCC_24_1x1,
	PM_P2P_GO_P2P_CLI_SCC_24_2x2,
	PM_P2P_GO_P2P_CLI_MCC_24_1x1,
	PM_P2P_GO_P2P_CLI_MCC_24_2x2,
	PM_P2P_GO_P2P_CLI_SCC_5_1x1,
	PM_P2P_GO_P2P_CLI_SCC_5_2x2,
	PM_P2P_GO_P2P_CLI_MCC_5_1x1,
	PM_P2P_GO_P2P_CLI_MCC_5_2x2,
	PM_P2P_GO_P2P_CLI_MCC_24_5_1x1,
	PM_P2P_GO_P2P_CLI_MCC_24_5_2x2,
	PM_P2P_GO_P2P_CLI_DBS_1x1,
	PM_P2P_GO_P2P_CLI_DBS_2x2,
	PM_P2P_GO_P2P_CLI_SBS_5_1x1,
	PM_P2P_GO_SAP_SCC_24_1x1,
	PM_P2P_GO_SAP_SCC_24_2x2,
	PM_P2P_GO_SAP_MCC_24_1x1,
	PM_P2P_GO_SAP_MCC_24_2x2,
	PM_P2P_GO_SAP_SCC_5_1x1,
	PM_P2P_GO_SAP_SCC_5_2x2,
	PM_P2P_GO_SAP_MCC_5_1x1,
	PM_P2P_GO_SAP_MCC_5_2x2,
	PM_P2P_GO_SAP_MCC_24_5_1x1,
	PM_P2P_GO_SAP_MCC_24_5_2x2,
	PM_P2P_GO_SAP_DBS_1x1,
	PM_P2P_GO_SAP_DBS_2x2,
	PM_P2P_GO_SAP_SBS_5_1x1,
	PM_P2P_CLI_SAP_SCC_24_1x1,
	PM_P2P_CLI_SAP_SCC_24_2x2,
	PM_P2P_CLI_SAP_MCC_24_1x1,
	PM_P2P_CLI_SAP_MCC_24_2x2,
	PM_P2P_CLI_SAP_SCC_5_1x1,
	PM_P2P_CLI_SAP_SCC_5_2x2,
	PM_P2P_CLI_SAP_MCC_5_1x1,
	PM_P2P_CLI_SAP_MCC_5_2x2,
	PM_P2P_CLI_SAP_MCC_24_5_1x1,
	PM_P2P_CLI_SAP_MCC_24_5_2x2,
	PM_P2P_CLI_SAP_DBS_1x1,
	PM_P2P_CLI_SAP_DBS_2x2,
	PM_P2P_CLI_SAP_SBS_5_1x1,
	PM_SAP_SAP_SCC_24_1x1,
	PM_SAP_SAP_SCC_24_2x2,
	PM_SAP_SAP_MCC_24_1x1,
	PM_SAP_SAP_MCC_24_2x2,
	PM_SAP_SAP_SCC_5_1x1,
	PM_SAP_SAP_SCC_5_2x2,
	PM_SAP_SAP_MCC_5_1x1,
	PM_SAP_SAP_MCC_5_2x2,
	PM_SAP_SAP_MCC_24_5_1x1,
	PM_SAP_SAP_MCC_24_5_2x2,
	PM_SAP_SAP_DBS_1x1,
	PM_SAP_SAP_DBS_2x2,
	PM_SAP_SAP_SBS_5_1x1,
	PM_STA_STA_SCC_24_1x1,
	PM_STA_STA_SCC_24_2x2,
	PM_STA_STA_MCC_24_1x1,
	PM_STA_STA_MCC_24_2x2,
	PM_STA_STA_SCC_5_1x1,
	PM_STA_STA_SCC_5_2x2,
	PM_STA_STA_MCC_5_1x1,
	PM_STA_STA_MCC_5_2x2,
	PM_STA_STA_MCC_24_5_1x1,
	PM_STA_STA_MCC_24_5_2x2,
	PM_STA_STA_DBS_1x1,
	PM_STA_STA_DBS_2x2,
	PM_STA_STA_SBS_5_1x1,

	PM_MAX_TWO_CONNECTION_MODE
};

/**
 * enum policy_mgr_conc_next_action - actions to be taken on old
 * connections.
 *
 * @PM_NOP: No action
 * @PM_DBS: switch to DBS mode
 * @PM_DBS_DOWNGRADE: switch to DBS mode & downgrade to 1x1
 * @PM_DBS_UPGRADE: switch to DBS mode & upgrade to 2x2
 * @PM_SINGLE_MAC: switch to MCC/SCC mode
 * @PM_SINGLE_MAC_UPGRADE: switch to MCC/SCC mode & upgrade to 2x2
 * @PM_SBS: switch to SBS mode
 * @PM_SBS_DOWNGRADE: switch to SBS mode & downgrade to 1x1
 * @PM_DOWNGRADE: downgrade to 1x1
 * @PM_UPGRADE: upgrade to 2x2
 * @PM_MAX_CONC_PRIORITY_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_conc_next_action {
	PM_NOP = 0,
	PM_DBS,
	PM_DBS_DOWNGRADE,
	PM_DBS_UPGRADE,
	PM_SINGLE_MAC,
	PM_SINGLE_MAC_UPGRADE,
	PM_SBS,
	PM_SBS_DOWNGRADE,
	PM_DOWNGRADE,
	PM_UPGRADE,

	PM_MAX_CONC_NEXT_ACTION
};

/**
 * enum policy_mgr_band - wifi band.
 *
 * @POLICY_MGR_BAND_24: 2.4 Ghz band
 * @POLICY_MGR_BAND_5: 5 Ghz band
 * @POLICY_MGR_MAX_BAND: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_band {
	POLICY_MGR_BAND_24 = 0,
	POLICY_MGR_BAND_5,
	POLICY_MGR_MAX_BAND
};

/**
 * enum policy_mgr_conn_update_reason: Reason for conc connection update
 * @POLICY_MGR_UPDATE_REASON_SET_OPER_CHAN: Set probable operating channel
 * @POLICY_MGR_UPDATE_REASON_JOIN_IBSS: Join IBSS
 * @POLICY_MGR_UPDATE_REASON_UT: Unit test related
 * @POLICY_MGR_UPDATE_REASON_START_AP: Start AP
 * @POLICY_MGR_UPDATE_REASON_NORMAL_STA: Connection to Normal STA
 * @POLICY_MGR_UPDATE_REASON_HIDDEN_STA: Connection to Hidden STA
 * @POLICY_MGR_UPDATE_REASON_OPPORTUNISTIC: Opportunistic HW mode update
 * @POLICY_MGR_UPDATE_REASON_NSS_UPDATE: NSS update
 * @POLICY_MGR_UPDATE_REASON_CHANNEL_SWITCH: Channel switch
 * @POLICY_MGR_UPDATE_REASON_CHANNEL_SWITCH_STA: Channel switch for STA
 */
enum policy_mgr_conn_update_reason {
	POLICY_MGR_UPDATE_REASON_SET_OPER_CHAN,
	POLICY_MGR_UPDATE_REASON_JOIN_IBSS,
	POLICY_MGR_UPDATE_REASON_UT,
	POLICY_MGR_UPDATE_REASON_START_AP,
	POLICY_MGR_UPDATE_REASON_NORMAL_STA,
	POLICY_MGR_UPDATE_REASON_HIDDEN_STA,
	POLICY_MGR_UPDATE_REASON_OPPORTUNISTIC,
	POLICY_MGR_UPDATE_REASON_NSS_UPDATE,
	POLICY_MGR_UPDATE_REASON_CHANNEL_SWITCH,
	POLICY_MGR_UPDATE_REASON_CHANNEL_SWITCH_STA,
	POLICY_MGR_UPDATE_REASON_PRE_CAC,
};

/**
 * enum hw_mode_bandwidth - bandwidth of wifi channel.
 *
 * @HW_MODE_5_MHZ: 5 Mhz bandwidth
 * @HW_MODE_10_MHZ: 10 Mhz bandwidth
 * @HW_MODE_20_MHZ: 20 Mhz bandwidth
 * @HW_MODE_40_MHZ: 40 Mhz bandwidth
 * @HW_MODE_80_MHZ: 80 Mhz bandwidth
 * @HW_MODE_80_PLUS_80_MHZ: 80 Mhz plus 80 Mhz bandwidth
 * @HW_MODE_160_MHZ: 160 Mhz bandwidth
 * @HW_MODE_MAX_BANDWIDTH: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum hw_mode_bandwidth {
	HW_MODE_BW_NONE,
	HW_MODE_5_MHZ,
	HW_MODE_10_MHZ,
	HW_MODE_20_MHZ,
	HW_MODE_40_MHZ,
	HW_MODE_80_MHZ,
	HW_MODE_80_PLUS_80_MHZ,
	HW_MODE_160_MHZ,
	HW_MODE_MAX_BANDWIDTH
};

/**
 * enum set_hw_mode_status - Status of set HW mode command
 * @SET_HW_MODE_STATUS_OK: command successful
 * @SET_HW_MODE_STATUS_EINVAL: Requested invalid hw_mode
 * @SET_HW_MODE_STATUS_ECANCELED: HW mode change cancelled
 * @SET_HW_MODE_STATUS_ENOTSUP: HW mode not supported
 * @SET_HW_MODE_STATUS_EHARDWARE: HW mode change prevented by hardware
 * @SET_HW_MODE_STATUS_EPENDING: HW mode change is pending
 * @SET_HW_MODE_STATUS_ECOEX: HW mode change conflict with Coex
 */
enum set_hw_mode_status {
	SET_HW_MODE_STATUS_OK,
	SET_HW_MODE_STATUS_EINVAL,
	SET_HW_MODE_STATUS_ECANCELED,
	SET_HW_MODE_STATUS_ENOTSUP,
	SET_HW_MODE_STATUS_EHARDWARE,
	SET_HW_MODE_STATUS_EPENDING,
	SET_HW_MODE_STATUS_ECOEX,
};

/**
 * enum policy_mgr_hw_mode_change - identify the HW mode switching to.
 *
 * @POLICY_MGR_HW_MODE_NOT_IN_PROGRESS: HW mode change not in progress
 * @POLICY_MGR_SMM_IN_PROGRESS: switching to SMM mode
 * @POLICY_MGR_DBS_IN_PROGRESS: switching to DBS mode
 * @POLICY_MGR_SBS_IN_PROGRESS: switching to SBS mode
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum policy_mgr_hw_mode_change {
	POLICY_MGR_HW_MODE_NOT_IN_PROGRESS = 0,
	POLICY_MGR_SMM_IN_PROGRESS,
	POLICY_MGR_DBS_IN_PROGRESS,
	POLICY_MGR_SBS_IN_PROGRESS
};

/**
 * enum dbs_support - structure to define INI values and their meaning
 *
 * @ENABLE_DBS_CXN_AND_SCAN: Enable DBS support for connection and scan
 * @DISABLE_DBS_CXN_AND_SCAN: Disable DBS support for connection and scan
 * @DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN: disable dbs support for
 * connection but keep dbs support for scan
 * @DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN_WITH_ASYNC_SCAN_OFF: disable dbs support
 * for connection but keep dbs for scan but switch off the async scan
 * @ENABLE_DBS_CXN_AND_ENABLE_SCAN_WITH_ASYNC_SCAN_OFF: enable dbs support for
 * connection and scan but switch off the async scan
 */
enum dbs_support {
	ENABLE_DBS_CXN_AND_SCAN,
	DISABLE_DBS_CXN_AND_SCAN,
	DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN,
	DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN_WITH_ASYNC_SCAN_OFF,
	ENABLE_DBS_CXN_AND_ENABLE_SCAN_WITH_ASYNC_SCAN_OFF,
};

/**
 * struct policy_mgr_conc_connection_info - information of all existing
 * connections in the wlan system
 *
 * @mode: connection type
 * @chan: channel of the connection
 * @bw: channel bandwidth used for the connection
 * @mac: The HW mac it is running
 * @chain_mask: The original capability advertised by HW
 * @original_nss: nss negotiated at connection time
 * @vdev_id: vdev id of the connection
 * @in_use: if the table entry is active
 */
struct policy_mgr_conc_connection_info {
	enum policy_mgr_con_mode mode;
	uint8_t       chan;
	enum hw_mode_bandwidth bw;
	uint8_t       mac;
	enum policy_mgr_chain_mode chain_mask;
	uint32_t      original_nss;
	uint32_t      vdev_id;
	bool          in_use;
};

/**
 * struct policy_mgr_hw_mode_params - HW mode params
 * @mac0_tx_ss: MAC0 Tx spatial stream
 * @mac0_rx_ss: MAC0 Rx spatial stream
 * @mac1_tx_ss: MAC1 Tx spatial stream
 * @mac1_rx_ss: MAC1 Rx spatial stream
 * @mac0_bw: MAC0 bandwidth
 * @mac1_bw: MAC1 bandwidth
 * @dbs_cap: DBS capabality
 * @agile_dfs_cap: Agile DFS capabality
 */
struct policy_mgr_hw_mode_params {
	uint8_t mac0_tx_ss;
	uint8_t mac0_rx_ss;
	uint8_t mac1_tx_ss;
	uint8_t mac1_rx_ss;
	uint8_t mac0_bw;
	uint8_t mac1_bw;
	uint8_t dbs_cap;
	uint8_t agile_dfs_cap;
	uint8_t sbs_cap;
};

/**
 * struct policy_mgr_vdev_mac_map - vdev id-mac id map
 * @vdev_id: VDEV id
 * @mac_id: MAC id
 */
struct policy_mgr_vdev_mac_map {
	uint32_t vdev_id;
	uint32_t mac_id;
};

/**
 * struct policy_mgr_dual_mac_config - Dual MAC configuration
 * @scan_config: Scan configuration
 * @fw_mode_config: FW mode configuration
 * @set_dual_mac_cb: Callback function to be executed on response to the command
 */
struct policy_mgr_dual_mac_config {
	uint32_t scan_config;
	uint32_t fw_mode_config;
	void *set_dual_mac_cb;
};

/**
 * struct policy_mgr_hw_mode - Format of set HW mode
 * @hw_mode_index: Index of HW mode to be set
 * @set_hw_mode_cb: HDD set HW mode callback
 * @reason: Reason for HW mode change
 * @session_id: Session id
 * @next_action: next action to happen at policy mgr
 * @context: psoc context
 */
struct policy_mgr_hw_mode {
	uint32_t hw_mode_index;
	void *set_hw_mode_cb;
	enum policy_mgr_conn_update_reason reason;
	uint32_t session_id;
	uint8_t next_action;
	struct wlan_objmgr_psoc *context;
};

/**
 * struct policy_mgr_pcl_list - Format of PCL
 * @pcl_list: List of preferred channels
 * @weight_list: Weights of the PCL
 * @pcl_len: Number of channels in the PCL
 */
struct policy_mgr_pcl_list {
	uint8_t pcl_list[POLICY_MGR_MAX_CHANNEL_LIST];
	uint8_t weight_list[POLICY_MGR_MAX_CHANNEL_LIST];
	uint32_t pcl_len;
};

/**
 * struct policy_mgr_pcl_chan_weights - Params to get the valid weighed list
 * @pcl_list: Preferred channel list already sorted in the order of preference
 * @pcl_len: Length of the PCL
 * @saved_chan_list: Valid channel list updated as part of
 * WMA_UPDATE_CHAN_LIST_REQ
 * @saved_num_chan: Length of the valid channel list
 * @weighed_valid_list: Weights of the valid channel list. This will have one
 * to one mapping with valid_chan_list. FW expects channel order and size to be
 * as per the list provided in WMI_SCAN_CHAN_LIST_CMDID.
 * @weight_list: Weights assigned by policy manager
 */
struct policy_mgr_pcl_chan_weights {
	uint8_t pcl_list[POLICY_MGR_MAX_CHANNEL_LIST];
	uint32_t pcl_len;
	uint8_t saved_chan_list[POLICY_MGR_MAX_CHANNEL_LIST];
	uint32_t saved_num_chan;
	uint8_t weighed_valid_list[POLICY_MGR_MAX_CHANNEL_LIST];
	uint8_t weight_list[POLICY_MGR_MAX_CHANNEL_LIST];
};

/**
 * struct policy_mgr_vdev_entry_info - vdev related param to be
 * used by policy manager
 * @type: type
 * @sub_type: sub type
 * @mhz: channel frequency in MHz
 * @chan_width: channel bandwidth
 * @mac_id: the mac on which vdev is on
 */
struct policy_mgr_vdev_entry_info {
	uint32_t type;
	uint32_t sub_type;
	uint32_t mhz;
	uint32_t chan_width;
	uint32_t mac_id;
};

/**
 * struct dbs_hw_mode_info - WLAN_DBS_HW_MODES_TLV Format
 * @tlv_header: TLV header, TLV tag and len; tag equals WMITLV_TAG_ARRAY_UINT32
 * @hw_mode_list: WLAN_DBS_HW_MODE_LIST entries
 */
struct dbs_hw_mode_info {
	uint32_t tlv_header;
	uint32_t *hw_mode_list;
};

/**
 * struct extended_caps - new extended caps given by firmware
 * @num_hw_modes: number of hardware modes for current SOC
 * @each_hw_mode_cap: hw mode id to phy id mapping
 * @each_phy_cap_per_hwmode: PHY's caps for each hw mode
 * @num_phy_for_hal_reg_cap: number of phy for hal reg cap
 * @hw_mode_to_mac_cap_map: map between hw_mode to capabilities
 */
struct extended_caps {
	WMI_SOC_MAC_PHY_HW_MODE_CAPS num_hw_modes;
	WMI_HW_MODE_CAPABILITIES *each_hw_mode_cap;
	WMI_MAC_PHY_CAPABILITIES *each_phy_cap_per_hwmode;
	WMI_SOC_HAL_REG_CAPABILITIES num_phy_for_hal_reg_cap;
	WMI_HAL_REG_CAPABILITIES_EXT *each_phy_hal_reg_cap;
	struct hw_mode_idx_to_mac_cap_idx *hw_mode_to_mac_cap_map;
};

/**
 * struct dual_mac_config - Dual MAC configurations
 * @prev_scan_config: Previous scan configuration
 * @prev_fw_mode_config: Previous FW mode configuration
 * @cur_scan_config: Current scan configuration
 * @cur_fw_mode_config: Current FW mode configuration
 * @req_scan_config: Requested scan configuration
 * @req_fw_mode_config: Requested FW mode configuration
 */
struct dual_mac_config {
	uint32_t prev_scan_config;
	uint32_t prev_fw_mode_config;
	uint32_t cur_scan_config;
	uint32_t cur_fw_mode_config;
	uint32_t req_scan_config;
	uint32_t req_fw_mode_config;
};

/**
 * struct policy_mgr_user_cfg - Policy manager user config variables
 * @enable_mcc_adaptive_scheduler: Enable MCC adaptive scheduler
 * @max_concurrent_active_sessions: User allowed maximum active
 *                              connections
 * @conc_system_pref: System preference for PCL table
 * @enable2x2: 2x2 chain mask user config
 * @mcc_to_scc_switch_mode: Control SAP channel in concurrency
 * @sub_20_mhz_enabled: Is 5 or 10 Mhz enabled
 */
struct policy_mgr_user_cfg {
	uint8_t enable_mcc_adaptive_scheduler;
	uint8_t max_concurrent_active_sessions;
	uint8_t conc_system_pref;
	bool enable2x2;
	uint32_t mcc_to_scc_switch_mode;
	bool sub_20_mhz_enabled;
	bool is_sta_sap_scc_allowed_on_dfs_chan;
	uint32_t channel_select_logic_conc;
};

/**
 * struct dbs_nss - Number of spatial streams in DBS mode
 * @mac0_ss: Number of spatial streams on MAC0
 * @mac1_ss: Number of spatial streams on MAC1
 */
struct dbs_nss {
	enum hw_mode_ss_config mac0_ss;
	enum hw_mode_ss_config mac1_ss;
};
#endif /* __WLAN_POLICY_MGR_PUBLIC_STRUCT_H */
