/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#ifndef __CDS_CONCURRENCY_H
#define __CDS_CONCURRENCY_H

/**
 * DOC: cds_concurrency.h
 *
 * CDS Concurrenct Connection Management entity
 */

/* Include files */

#include "wlan_hdd_main.h"

#define MAX_NUMBER_OF_CONC_CONNECTIONS 3
#define DBS_OPPORTUNISTIC_TIME    10
#ifdef QCA_WIFI_3_0_EMU
#define CONNECTION_UPDATE_TIMEOUT 3000
#else
#define CONNECTION_UPDATE_TIMEOUT 1000
#endif

/* Some max value greater than the max length of the channel list */
#define MAX_WEIGHT_OF_PCL_CHANNELS 255
/* Some fixed weight difference between the groups */
#define PCL_GROUPS_WEIGHT_DIFFERENCE 20

/* Currently max, only 3 groups are possible as per 'enum cds_pcl_type'.
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

#define WEIGHT_OF_NON_PCL_CHANNELS 1
#define WEIGHT_OF_DISALLOWED_CHANNELS 0

/**
 * enum hw_mode_ss_config - Possible spatial stream configuration
 * @SS_0x0: Unused Tx and Rx of MAC
 * @SS_1x1: 1 Tx SS and 1 Rx SS
 * @SS_2x2: 2 Tx SS and 2 Rx SS
 * @SS_3x3: 3 Tx SS and 3 Rx SS
 * @SS_4x4: 4 Tx SS and 4 Rx SS
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
 * @HW_MODE_DBS: DFS capable
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
 * enum cds_pcl_group_id - Identifies the pcl groups to be used
 * @CDS_PCL_GROUP_ID1_ID2: Use weights of group1 and group2
 * @CDS_PCL_GROUP_ID2_ID2: Use weights of group2 and group3
 *
 * Since maximum of three groups are possible, this will indicate which
 * PCL group needs to be used.
 */
enum cds_pcl_group_id {
	CDS_PCL_GROUP_ID1_ID2,
	CDS_PCL_GROUP_ID2_ID3,
};

/**
 * cds_pcl_channel_order - Order in which the PCL is requested
 * @CDS_PCL_ORDER_NONE: no order
 * @CDS_PCL_ORDER_24G_THEN_5G: 2.4 Ghz channel followed by 5 Ghz channel
 * @CDS_PCL_ORDER_5G_THEN_2G: 5 Ghz channel followed by 2.4 Ghz channel
 *
 * Order in which the PCL is requested
 */
enum cds_pcl_channel_order {
	CDS_PCL_ORDER_NONE,
	CDS_PCL_ORDER_24G_THEN_5G,
	CDS_PCL_ORDER_5G_THEN_2G,
};

/**
 * enum cds_max_rx_ss - Maximum number of receive spatial streams
 * @CDS_RX_NSS_1: Receive Nss = 1
 * @CDS_RX_NSS_2: Receive Nss = 2
 * @CDS_RX_NSS_3: Receive Nss = 3
 * @CDS_RX_NSS_4: Receive Nss = 4
 * @CDS_RX_NSS_5: Receive Nss = 5
 * @CDS_RX_NSS_6: Receive Nss = 6
 * @CDS_RX_NSS_7: Receive Nss = 7
 * @CDS_RX_NSS_8: Receive Nss = 8
 *
 * Indicates the maximum number of spatial streams that the STA can receive
 */
enum cds_max_rx_ss {
	CDS_RX_NSS_1 = 0,
	CDS_RX_NSS_2 = 1,
	CDS_RX_NSS_3 = 2,
	CDS_RX_NSS_4 = 3,
	CDS_RX_NSS_5 = 4,
	CDS_RX_NSS_6 = 5,
	CDS_RX_NSS_7 = 6,
	CDS_RX_NSS_8 = 7,
	CDS_RX_NSS_MAX,
};

/**
 * enum cds_chain_mode - Chain Mask tx & rx combination.
 *
 * @CDS_ONE_ONE: One for Tx, One for Rx
 * @CDS_TWO_TWO: Two for Tx, Two for Rx
 * @CDS_MAX_NO_OF_CHAIN_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_chain_mode {
	CDS_ONE_ONE = 0,
	CDS_TWO_TWO,
	CDS_MAX_NO_OF_CHAIN_MODE
};

/**
 * enum cds_conc_priority_mode - t/p, powersave, latency.
 *
 * @CDS_THROUGHPUT: t/p is the priority
 * @CDS_POWERSAVE: powersave is the priority
 * @CDS_LATENCY: latency is the priority
 * @CDS_MAX_CONC_PRIORITY_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_conc_priority_mode {
	CDS_THROUGHPUT = 0,
	CDS_POWERSAVE,
	CDS_LATENCY,
	CDS_MAX_CONC_PRIORITY_MODE
};

/**
 * enum cds_con_mode - concurrency mode for PCL table
 *
 * @CDS_STA_MODE: station mode
 * @CDS_SAP_MODE: SAP mode
 * @CDS_P2P_CLIENT_MODE: P2P client mode
 * @CDS_P2P_GO_MODE: P2P Go mode
 * @CDS_IBSS_MODE: IBSS mode
 * @CDS_MAX_NUM_OF_MODE: max value place holder
 */
enum cds_con_mode {
	CDS_STA_MODE = 0,
	CDS_SAP_MODE,
	CDS_P2P_CLIENT_MODE,
	CDS_P2P_GO_MODE,
	CDS_IBSS_MODE,
	CDS_MAX_NUM_OF_MODE
};

/**
 * enum cds_mac_use - MACs that are used
 * @CDS_MAC0: Only MAC0 is used
 * @CDS_MAC1: Only MAC1 is used
 * @CDS_MAC0_AND_MAC1: Both MAC0 and MAC1 are used
 */
enum cds_mac_use {
	CDS_MAC0 = 1,
	CDS_MAC1 = 2,
	CDS_MAC0_AND_MAC1 = 3
};

/**
 * enum cds_pcl_type - Various types of Preferred channel list (PCL).
 *
 * @CDS_NONE: No channel preference
 * @CDS_24G: 2.4 Ghz channels only
 * @CDS_5G: 5 Ghz channels only
 * @CDS_SCC_CH: SCC channel only
 * @CDS_MCC_CH: MCC channels only
 * @CDS_SCC_CH_24G: SCC channel & 2.4 Ghz channels
 * @CDS_SCC_CH_5G: SCC channel & 5 Ghz channels
 * @CDS_24G_SCC_CH: 2.4 Ghz channels & SCC channel
 * @CDS_5G_SCC_CH: 5 Ghz channels & SCC channel
 * @CDS_SCC_ON_5_SCC_ON_24_24G: SCC channel on 5 Ghz, SCC
 *	channel on 2.4 Ghz & 2.4 Ghz channels
 * @CDS_SCC_ON_5_SCC_ON_24_5G: SCC channel on 5 Ghz, SCC channel
 *	on 2.4 Ghz & 5 Ghz channels
 * @CDS_SCC_ON_24_SCC_ON_5_24G: SCC channel on 2.4 Ghz, SCC
 *	channel on 5 Ghz & 2.4 Ghz channels
 * @CDS_SCC_ON_24_SCC_ON_5_5G: SCC channel on 2.4 Ghz, SCC
 *	channel on 5 Ghz & 5 Ghz channels
 * @CDS_SCC_ON_5_SCC_ON_24: SCC channel on 5 Ghz, SCC channel on
 *	2.4 Ghz
 * @CDS_SCC_ON_24_SCC_ON_5: SCC channel on 2.4 Ghz, SCC channel
 *	on 5 Ghz
 * @CDS_MCC_CH_24G: MCC channels & 2.4 Ghz channels
 * @CDS_MCC_CH_5G:  MCC channels & 5 Ghz channels
 * @CDS_24G_MCC_CH: 2.4 Ghz channels & MCC channels
 * @CDS_5G_MCC_CH: 5 Ghz channels & MCC channels
 * @CDS_MAX_PCL_TYPE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_pcl_type {
	CDS_NONE = 0,
	CDS_24G,
	CDS_5G,
	CDS_SCC_CH,
	CDS_MCC_CH,
	CDS_SCC_CH_24G,
	CDS_SCC_CH_5G,
	CDS_24G_SCC_CH,
	CDS_5G_SCC_CH,
	CDS_SCC_ON_5_SCC_ON_24_24G,
	CDS_SCC_ON_5_SCC_ON_24_5G,
	CDS_SCC_ON_24_SCC_ON_5_24G,
	CDS_SCC_ON_24_SCC_ON_5_5G,
	CDS_SCC_ON_5_SCC_ON_24,
	CDS_SCC_ON_24_SCC_ON_5,
	CDS_MCC_CH_24G,
	CDS_MCC_CH_5G,
	CDS_24G_MCC_CH,
	CDS_5G_MCC_CH,

	CDS_MAX_PCL_TYPE
};

/**
 * enum cds_one_connection_mode - Combination of first connection
 * type, band & spatial stream used.
 *
 * @CDS_STA_24_1x1: STA connection using 1x1@2.4 Ghz
 * @CDS_STA_24_2x2: STA connection using 2x2@2.4 Ghz
 * @CDS_STA_5_1x1: STA connection using 1x1@5 Ghz
 * @CDS_STA_5_2x2: STA connection using 2x2@5 Ghz
 * @CDS_P2P_CLI_24_1x1: P2P Client connection using 1x1@2.4 Ghz
 * @CDS_P2P_CLI_24_2x2: P2P Client connection using 2x2@2.4 Ghz
 * @CDS_P2P_CLI_5_1x1: P2P Client connection using 1x1@5 Ghz
 * @CDS_P2P_CLI_5_2x2: P2P Client connection using 2x2@5 Ghz
 * @CDS_P2P_GO_24_1x1: P2P GO connection using 1x1@2.4 Ghz
 * @CDS_P2P_GO_24_2x2: P2P GO connection using 2x2@2.4 Ghz
 * @CDS_P2P_GO_5_1x1: P2P GO connection using 1x1@5 Ghz
 * @CDS_P2P_GO_5_2x2: P2P GO connection using 2x2@5 Ghz
 * @CDS_SAP_24_1x1: SAP connection using 1x1@2.4 Ghz
 * @CDS_SAP_24_2x2: SAP connection using 2x2@2.4 Ghz
 * @CDS_SAP_5_1x1: SAP connection using 1x1@5 Ghz
 * @CDS_SAP_5_1x1: SAP connection using 2x2@5 Ghz
 * @CDS_IBSS_24_1x1:  IBSS connection using 1x1@2.4 Ghz
 * @CDS_IBSS_24_2x2:  IBSS connection using 2x2@2.4 Ghz
 * @CDS_IBSS_5_1x1:  IBSS connection using 1x1@5 Ghz
 * @CDS_IBSS_5_2x2:  IBSS connection using 2x2@5 Ghz
 * @CDS_MAX_ONE_CONNECTION_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_one_connection_mode {
	CDS_STA_24_1x1 = 0,
	CDS_STA_24_2x2,
	CDS_STA_5_1x1,
	CDS_STA_5_2x2,
	CDS_P2P_CLI_24_1x1,
	CDS_P2P_CLI_24_2x2,
	CDS_P2P_CLI_5_1x1,
	CDS_P2P_CLI_5_2x2,
	CDS_P2P_GO_24_1x1,
	CDS_P2P_GO_24_2x2,
	CDS_P2P_GO_5_1x1,
	CDS_P2P_GO_5_2x2,
	CDS_SAP_24_1x1,
	CDS_SAP_24_2x2,
	CDS_SAP_5_1x1,
	CDS_SAP_5_2x2,
	CDS_IBSS_24_1x1,
	CDS_IBSS_24_2x2,
	CDS_IBSS_5_1x1,
	CDS_IBSS_5_2x2,

	CDS_MAX_ONE_CONNECTION_MODE
};

/**
 * enum cds_two_connection_mode - Combination of first two
 * connections type, concurrency state, band & spatial stream
 * used.
 *
 * @CDS_STA_SAP_SCC_24_1x1: STA & SAP connection on SCC using
 *			1x1@2.4 Ghz
 * @CDS_STA_SAP_SCC_24_2x2: STA & SAP connection on SCC using
 *			2x2@2.4 Ghz
 * @CDS_STA_SAP_MCC_24_1x1: STA & SAP connection on MCC using
 *			1x1@2.4 Ghz
 * @CDS_STA_SAP_MCC_24_2x2: STA & SAP connection on MCC using
 *			2x2@2.4 Ghz
 * @CDS_STA_SAP_SCC_5_1x1: STA & SAP connection on SCC using
 *			1x1@5 Ghz
 * @CDS_STA_SAP_SCC_5_2x2: STA & SAP connection on SCC using
 *			2x2@5 Ghz
 * @CDS_STA_SAP_MCC_5_1x1: STA & SAP connection on MCC using
 *			1x1@5 Ghz
 * @CDS_STA_SAP_MCC_5_2x2: STA & SAP connection on MCC using
 *			2x2@5 Ghz
 * @CDS_STA_SAP_DBS_1x1,: STA & SAP connection on DBS using 1x1
 * @CDS_STA_P2P_GO_SCC_24_1x1: STA & P2P GO connection on SCC
 *			using 1x1@2.4 Ghz
 * @CDS_STA_P2P_GO_SCC_24_2x2: STA & P2P GO connection on SCC
 *			using 2x2@2.4 Ghz
 * @CDS_STA_P2P_GO_MCC_24_1x1: STA & P2P GO connection on MCC
 *			using 1x1@2.4 Ghz
 * @CDS_STA_P2P_GO_MCC_24_2x2: STA & P2P GO connection on MCC
 *			using 2x2@2.4 Ghz
 * @CDS_STA_P2P_GO_SCC_5_1x1: STA & P2P GO connection on SCC
 *			using 1x1@5 Ghz
 * @CDS_STA_P2P_GO_SCC_5_2x2: STA & P2P GO connection on SCC
 *			using 2x2@5 Ghz
 * @CDS_STA_P2P_GO_MCC_5_1x1: STA & P2P GO connection on MCC
 *			using 1x1@5 Ghz
 * @CDS_STA_P2P_GO_MCC_5_2x2: STA & P2P GO connection on MCC
 *			using 2x2@5 Ghz
 * @CDS_STA_P2P_GO_DBS_1x1: STA & P2P GO connection on DBS using
 *			1x1
 * @CDS_STA_P2P_CLI_SCC_24_1x1: STA & P2P CLI connection on SCC
 *			using 1x1@2.4 Ghz
 * @CDS_STA_P2P_CLI_SCC_24_2x2: STA & P2P CLI connection on SCC
 *			using 2x2@2.4 Ghz
 * @CDS_STA_P2P_CLI_MCC_24_1x1: STA & P2P CLI connection on MCC
 *			using 1x1@2.4 Ghz
 * @CDS_STA_P2P_CLI_MCC_24_2x2: STA & P2P CLI connection on MCC
 *			using 2x2@2.4 Ghz
 * @CDS_STA_P2P_CLI_SCC_5_1x1: STA & P2P CLI connection on SCC
 *			using 1x1@5 Ghz
 * @CDS_STA_P2P_CLI_SCC_5_2x2: STA & P2P CLI connection on SCC
 *			using 2x2@5 Ghz
 * @CDS_STA_P2P_CLI_MCC_5_1x1: STA & P2P CLI connection on MCC
 *			using 1x1@5 Ghz
 * @CDS_STA_P2P_CLI_MCC_5_2x2: STA & P2P CLI connection on MCC
 *			using 2x2@5 Ghz
 * @CDS_STA_P2P_CLI_DBS_1x1: STA & P2P CLI connection on DBS
 *			using 1x1
 * @CDS_P2P_GO_P2P_CLI_SCC_24_1x1: P2P GO & CLI connection on
 *			SCC using 1x1@2.4 Ghz
 * @CDS_P2P_GO_P2P_CLI_SCC_24_2x2: P2P GO & CLI connection on
 *			SCC using 2x2@2.4 Ghz
 * @CDS_P2P_GO_P2P_CLI_MCC_24_1x1: P2P GO & CLI connection on
 *			MCC using 1x1@2.4 Ghz
 * @CDS_P2P_GO_P2P_CLI_MCC_24_2x2: P2P GO & CLI connection on
 *			MCC using 2x2@2.4 Ghz
 * @CDS_P2P_GO_P2P_CLI_SCC_5_1x1: P2P GO & CLI connection on
 *			SCC using 1x1@5 Ghz
 * @CDS_P2P_GO_P2P_CLI_SCC_5_2x2: P2P GO & CLI connection on
 *			SCC using 2x2@5 Ghz
 * @CDS_P2P_GO_P2P_CLI_MCC_5_1x1: P2P GO & CLI connection on
 *			MCC using 1x1@5 Ghz
 * @CDS_P2P_GO_P2P_CLI_MCC_5_2x2: P2P GO & CLI connection on
 *			MCC using 2x2@5 Ghz
 * @CDS_P2P_GO_P2P_CLI_DBS_1x1: P2P GO & CLI connection on DBS
 *			using 1x1
 * @CDS_P2P_GO_SAP_SCC_24_1x1: P2P GO & SAP connection on
 *			SCC using 1x1@2.4 Ghz
 * @CDS_P2P_GO_SAP_SCC_24_2x2: P2P GO & SAP connection on
 *			SCC using 2x2@2.4 Ghz
 * @CDS_P2P_GO_SAP_MCC_24_1x1: P2P GO & SAP connection on
 *			MCC using 1x1@2.4 Ghz
 * @CDS_P2P_GO_SAP_MCC_24_2x2: P2P GO & SAP connection on
 *			MCC using 2x2@2.4 Ghz
 * @CDS_P2P_GO_SAP_SCC_5_1x1: P2P GO & SAP connection on
 *			SCC using 1x1@5 Ghz
 * @CDS_P2P_GO_SAP_SCC_5_2x2: P2P GO & SAP connection on
 *			SCC using 2x2@5 Ghz
 * @CDS_P2P_GO_SAP_MCC_5_1x1: P2P GO & SAP connection on
 *			MCC using 1x1@5 Ghz
 * @CDS_P2P_GO_SAP_MCC_5_2x2: P2P GO & SAP connection on
 *			MCC using 2x2@5 Ghz
 * @CDS_P2P_GO_SAP_DBS_1x1: P2P GO & SAP connection on DBS using
 *			1x1
 * @CDS_P2P_CLI_SAP_SCC_24_1x1: CLI & SAP connection on SCC using
 *			1x1@2.4 Ghz
 * @CDS_P2P_CLI_SAP_SCC_24_2x2: CLI & SAP connection on SCC using
 *			2x2@2.4 Ghz
 * @CDS_P2P_CLI_SAP_MCC_24_1x1: CLI & SAP connection on MCC using
 *			1x1@2.4 Ghz
 * @CDS_P2P_CLI_SAP_MCC_24_2x2: CLI & SAP connection on MCC using
 *			2x2@2.4 Ghz
 * @CDS_P2P_CLI_SAP_SCC_5_1x1: CLI & SAP connection on SCC using
 *			1x1@5 Ghz
 * @CDS_P2P_CLI_SAP_SCC_5_2x2: CLI & SAP connection on SCC using
 *			2x2@5 Ghz
 * @CDS_P2P_CLI_SAP_MCC_5_1x1: CLI & SAP connection on MCC using
 *			1x1@5 Ghz
 * @CDS_P2P_CLI_SAP_MCC_5_2x2: CLI & SAP connection on MCC using
 *			2x2@5 Ghz
 * @CDS_P2P_STA_SAP_MCC_24_5_1x1: CLI and SAP connecting on MCC
 *			in 2.4 and 5GHz 1x1
 * @CDS_P2P_STA_SAP_MCC_24_5_2x2: CLI and SAP connecting on MCC
			in 2.4 and 5GHz 2x2
 * @CDS_P2P_CLI_SAP_DBS_1x1,: CLI & SAP connection on DBS using 1x1

 * @CDS_MAX_TWO_CONNECTION_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_two_connection_mode {
	CDS_STA_SAP_SCC_24_1x1 = 0,
	CDS_STA_SAP_SCC_24_2x2,
	CDS_STA_SAP_MCC_24_1x1,
	CDS_STA_SAP_MCC_24_2x2,
	CDS_STA_SAP_SCC_5_1x1,
	CDS_STA_SAP_SCC_5_2x2,
	CDS_STA_SAP_MCC_5_1x1,
	CDS_STA_SAP_MCC_5_2x2,
	CDS_STA_SAP_MCC_24_5_1x1,
	CDS_STA_SAP_MCC_24_5_2x2,
	CDS_STA_SAP_DBS_1x1,
	CDS_STA_P2P_GO_SCC_24_1x1,
	CDS_STA_P2P_GO_SCC_24_2x2,
	CDS_STA_P2P_GO_MCC_24_1x1,
	CDS_STA_P2P_GO_MCC_24_2x2,
	CDS_STA_P2P_GO_SCC_5_1x1,
	CDS_STA_P2P_GO_SCC_5_2x2,
	CDS_STA_P2P_GO_MCC_5_1x1,
	CDS_STA_P2P_GO_MCC_5_2x2,
	CDS_STA_P2P_GO_MCC_24_5_1x1,
	CDS_STA_P2P_GO_MCC_24_5_2x2,
	CDS_STA_P2P_GO_DBS_1x1,
	CDS_STA_P2P_CLI_SCC_24_1x1,
	CDS_STA_P2P_CLI_SCC_24_2x2,
	CDS_STA_P2P_CLI_MCC_24_1x1,
	CDS_STA_P2P_CLI_MCC_24_2x2,
	CDS_STA_P2P_CLI_SCC_5_1x1,
	CDS_STA_P2P_CLI_SCC_5_2x2,
	CDS_STA_P2P_CLI_MCC_5_1x1,
	CDS_STA_P2P_CLI_MCC_5_2x2,
	CDS_STA_P2P_CLI_MCC_24_5_1x1,
	CDS_STA_P2P_CLI_MCC_24_5_2x2,
	CDS_STA_P2P_CLI_DBS_1x1,
	CDS_P2P_GO_P2P_CLI_SCC_24_1x1,
	CDS_P2P_GO_P2P_CLI_SCC_24_2x2,
	CDS_P2P_GO_P2P_CLI_MCC_24_1x1,
	CDS_P2P_GO_P2P_CLI_MCC_24_2x2,
	CDS_P2P_GO_P2P_CLI_SCC_5_1x1,
	CDS_P2P_GO_P2P_CLI_SCC_5_2x2,
	CDS_P2P_GO_P2P_CLI_MCC_5_1x1,
	CDS_P2P_GO_P2P_CLI_MCC_5_2x2,
	CDS_P2P_GO_P2P_CLI_MCC_24_5_1x1,
	CDS_P2P_GO_P2P_CLI_MCC_24_5_2x2,
	CDS_P2P_GO_P2P_CLI_DBS_1x1,
	CDS_P2P_GO_SAP_SCC_24_1x1,
	CDS_P2P_GO_SAP_SCC_24_2x2,
	CDS_P2P_GO_SAP_MCC_24_1x1,
	CDS_P2P_GO_SAP_MCC_24_2x2,
	CDS_P2P_GO_SAP_SCC_5_1x1,
	CDS_P2P_GO_SAP_SCC_5_2x2,
	CDS_P2P_GO_SAP_MCC_5_1x1,
	CDS_P2P_GO_SAP_MCC_5_2x2,
	CDS_P2P_GO_SAP_MCC_24_5_1x1,
	CDS_P2P_GO_SAP_MCC_24_5_2x2,
	CDS_P2P_GO_SAP_DBS_1x1,
	CDS_P2P_CLI_SAP_SCC_24_1x1,
	CDS_P2P_CLI_SAP_SCC_24_2x2,
	CDS_P2P_CLI_SAP_MCC_24_1x1,
	CDS_P2P_CLI_SAP_MCC_24_2x2,
	CDS_P2P_CLI_SAP_SCC_5_1x1,
	CDS_P2P_CLI_SAP_SCC_5_2x2,
	CDS_P2P_CLI_SAP_MCC_5_1x1,
	CDS_P2P_CLI_SAP_MCC_5_2x2,
	CDS_P2P_CLI_SAP_MCC_24_5_1x1,
	CDS_P2P_CLI_SAP_MCC_24_5_2x2,
	CDS_P2P_CLI_SAP_DBS_1x1,

	CDS_MAX_TWO_CONNECTION_MODE
};

/**
 * enum cds_conc_next_action - actions to be taken on old
 * connections.
 *
 * @CDS_NOP: No action
 * @CDS_DBS: switch to DBS mode
 * @CDS_DBS_DOWNGRADE: switch to DBS mode & downgrade to 1x1
 * @CDS_SINGLE_MAC: switch to MCC/SCC mode
 * @CDS_SINGLE_MAC_UPGRADE: switch to MCC/SCC mode & upgrade to 2x2
 * @CDS_MAX_CONC_PRIORITY_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_conc_next_action {
	CDS_NOP = 0,
	CDS_DBS,
	CDS_DBS_DOWNGRADE,
	CDS_SINGLE_MAC,
	CDS_SINGLE_MAC_UPGRADE,
	CDS_MAX_CONC_NEXT_ACTION
};

/**
 * enum cds_band - wifi band.
 *
 * @CDS_BAND_24: 2.4 Ghz band
 * @CDS_BAND_5: 5 Ghz band
 * @CDS_MAX_BAND: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_band {
	CDS_BAND_24 = 0,
	CDS_BAND_5,
	CDS_MAX_BAND
};

/**
 * struct cds_conc_connection_info - information of all existing
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
struct cds_conc_connection_info {
	enum cds_con_mode mode;
	uint8_t       chan;
	enum hw_mode_bandwidth bw;
	uint8_t       mac;
	enum cds_chain_mode chain_mask;
	uint32_t      original_nss;
	uint32_t      vdev_id;
	bool          in_use;
};

bool cds_is_connection_in_progress(void);
void cds_dump_concurrency_info(void);
bool cds_check_is_tdls_allowed(enum tQDF_ADAPTER_MODE device_mode);
void cds_set_tdls_ct_mode(hdd_context_t *hdd_ctx);
void cds_set_concurrency_mode(enum tQDF_ADAPTER_MODE mode);
void cds_clear_concurrency_mode(enum tQDF_ADAPTER_MODE mode);
uint32_t cds_get_connection_count(void);
bool cds_is_sta_connection_pending(void);
void cds_change_sta_conn_pending_status(bool value);
void cds_change_sap_restart_required_status(bool value);
bool cds_set_connection_in_progress(bool value);
uint32_t cds_get_concurrency_mode(void);
QDF_STATUS cds_check_and_restart_sap(eCsrRoamResult roam_result,
		hdd_station_ctx_t *hdd_sta_ctx);
void cds_handle_conc_rule1(hdd_adapter_t *adapter,
		tCsrRoamProfile *roam_profile);
#ifdef FEATURE_WLAN_CH_AVOID
bool cds_handle_conc_rule2(hdd_adapter_t *adapter,
		tCsrRoamProfile *roam_profile,
		uint32_t *roam_id);
#else
static inline bool cds_handle_conc_rule2(hdd_adapter_t *adapter,
		tCsrRoamProfile *roam_profile,
		uint32_t *roam_id)
{
		return true;
}
#endif /* FEATURE_WLAN_CH_AVOID */
uint8_t cds_search_and_check_for_session_conc(uint8_t session_id,
		tCsrRoamProfile *roam_profile);
bool cds_check_for_session_conc(uint8_t session_id, uint8_t channel);
QDF_STATUS cds_handle_conc_multiport(uint8_t session_id, uint8_t channel);

#ifdef FEATURE_WLAN_FORCE_SAP_SCC
void cds_force_sap_on_scc(eCsrRoamResult roam_result,
		uint8_t channel_id);
#else
static inline void cds_force_sap_on_scc(eCsrRoamResult roam_result,
				uint8_t channel_id)
{

}
#endif /* FEATURE_WLAN_FORCE_SAP_SCC */

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
void cds_check_concurrent_intf_and_restart_sap(hdd_adapter_t *adapter);
#else
static inline void cds_check_concurrent_intf_and_restart_sap(
						hdd_adapter_t *adapter)
{

}
#endif /* FEATURE_WLAN_MCC_TO_SCC_SWITCH */
uint8_t cds_is_mcc_in_24G(void);
int32_t cds_set_mas(hdd_adapter_t *adapter, uint8_t mas_value);
int cds_set_mcc_p2p_quota(hdd_adapter_t *hostapd_adapter,
		uint32_t set_value);
QDF_STATUS cds_change_mcc_go_beacon_interval(hdd_adapter_t *pHostapdAdapter);
int cds_go_set_mcc_p2p_quota(hdd_adapter_t *hostapd_adapter,
		uint32_t set_value);
void cds_set_mcc_latency(hdd_adapter_t *adapter, int set_value);
#if defined(FEATURE_WLAN_MCC_TO_SCC_SWITCH)
void cds_change_sap_channel_with_csa(hdd_adapter_t *adapter,
						hdd_ap_ctx_t *hdd_ap_ctx);
#else
static inline void cds_change_sap_channel_with_csa(hdd_adapter_t *adapter,
		hdd_ap_ctx_t *hdd_ap_ctx)
{

}
#endif

#if defined(FEATURE_WLAN_MCC_TO_SCC_SWITCH) || \
		defined(FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE)
void cds_restart_sap(hdd_adapter_t *ap_adapter);
#else
static inline void cds_restart_sap(hdd_adapter_t *ap_adapter)
{

}
#endif /* FEATURE_WLAN_MCC_TO_SCC_SWITCH ||
	* FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE
	*/

#ifdef FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE
void cds_check_and_restart_sap_with_non_dfs_acs(void);
#else
static inline void cds_check_and_restart_sap_with_non_dfs_acs(void)
{

}
#endif /* FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE */
void cds_incr_active_session(enum tQDF_ADAPTER_MODE mode,
				uint8_t sessionId);
void cds_decr_active_session(enum tQDF_ADAPTER_MODE mode,
				uint8_t sessionId);
void cds_decr_session_set_pcl(enum tQDF_ADAPTER_MODE mode,
		uint8_t session_id);
QDF_STATUS cds_init_policy_mgr(struct cds_sme_cbacks *sme_cbacks);
QDF_STATUS cds_deinit_policy_mgr(void);
uint8_t cds_get_channel(enum cds_con_mode mode, uint32_t *vdev_id);
QDF_STATUS cds_get_pcl(enum cds_con_mode mode,
			uint8_t *pcl_channels, uint32_t *len,
			uint8_t *pcl_weight, uint32_t weight_len);
void cds_update_with_safe_channel_list(uint8_t *pcl_channels, uint32_t *len,
		uint8_t *weight_list, uint32_t weight_len);
uint8_t cds_get_nondfs_preferred_channel(enum cds_con_mode mode,
					bool for_existing_conn);
bool cds_is_any_nondfs_chnl_present(uint8_t *channel);
bool cds_allow_concurrency(enum cds_con_mode mode,
				uint8_t channel, enum hw_mode_bandwidth bw);
enum cds_conc_priority_mode cds_get_first_connection_pcl_table_index(void);
enum cds_one_connection_mode cds_get_second_connection_pcl_table_index(void);
enum cds_two_connection_mode cds_get_third_connection_pcl_table_index(void);
QDF_STATUS cds_incr_connection_count(uint32_t vdev_id);
QDF_STATUS cds_update_connection_info(uint32_t vdev_id);
QDF_STATUS cds_decr_connection_count(uint32_t vdev_id);
QDF_STATUS cds_current_connections_update(uint32_t session_id,
				uint8_t channel,
				enum sir_conn_update_reason);
bool cds_is_ibss_conn_exist(uint8_t *ibss_channel);
#ifdef MPC_UT_FRAMEWORK
QDF_STATUS cds_incr_connection_count_utfw(
		uint32_t vdev_id, uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id);
QDF_STATUS cds_update_connection_info_utfw(
		uint32_t vdev_id, uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id);
QDF_STATUS cds_decr_connection_count_utfw(
		uint32_t del_all, uint32_t vdev_id);
struct cds_conc_connection_info *cds_get_conn_info(uint32_t *len);
enum cds_pcl_type get_pcl_from_first_conn_table(enum cds_con_mode type,
		enum cds_conc_priority_mode sys_pref);
enum cds_pcl_type get_pcl_from_second_conn_table(
	enum cds_one_connection_mode idx, enum cds_con_mode type,
	enum cds_conc_priority_mode sys_pref, uint8_t dbs_capable);
enum cds_pcl_type get_pcl_from_third_conn_table(
	enum cds_two_connection_mode idx, enum cds_con_mode type,
	enum cds_conc_priority_mode sys_pref, uint8_t dbs_capable);
#else
static inline QDF_STATUS cds_incr_connection_count_utfw(uint32_t vdev_id,
		uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id)
{
	return QDF_STATUS_SUCCESS;
}
static inline QDF_STATUS cds_update_connection_info_utfw(uint32_t vdev_id,
		uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id)
{
	return QDF_STATUS_SUCCESS;
}
static inline QDF_STATUS cds_decr_connection_count_utfw(uint32_t del_all,
		uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
static inline struct cds_conc_connection_info *cds_get_conn_info(uint32_t *len)
{
	return NULL;
}
#endif

enum cds_con_mode cds_convert_device_mode_to_qdf_type(
				enum tQDF_ADAPTER_MODE device_mode);
QDF_STATUS cds_pdev_set_hw_mode(uint32_t session_id,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum sir_conn_update_reason reason);
enum cds_conc_next_action cds_need_opportunistic_upgrade(void);
QDF_STATUS cds_next_actions(uint32_t session_id,
		enum cds_conc_next_action action,
		enum sir_conn_update_reason reason);
void cds_set_dual_mac_scan_config(uint8_t dbs_val,
		uint8_t dbs_plus_agile_scan_val,
		uint8_t single_mac_scan_with_dbs_val);
void cds_set_dual_mac_fw_mode_config(uint8_t dbs,
		uint8_t dfs);
void cds_soc_set_dual_mac_cfg_cb(enum set_hw_mode_status status,
		uint32_t scan_config,
		uint32_t fw_mode_config);
bool cds_map_concurrency_mode(enum tQDF_ADAPTER_MODE *old_mode,
		enum cds_con_mode *new_mode);
QDF_STATUS cds_get_channel_from_scan_result(hdd_adapter_t *adapter,
		tCsrRoamProfile *roam_profile, uint8_t *channel);

enum tQDF_GLOBAL_CON_MODE cds_get_conparam(void);
bool cds_concurrent_open_sessions_running(void);
bool cds_max_concurrent_connections_reached(void);
void cds_clear_concurrent_session_count(void);
bool cds_is_multiple_active_sta_sessions(void);
bool cds_is_sta_active_connection_exists(void);
bool cds_concurrent_beaconing_sessions_running(void);
QDF_STATUS qdf_wait_for_connection_update(void);
QDF_STATUS qdf_reset_connection_update(void);
QDF_STATUS qdf_set_connection_update(void);
QDF_STATUS qdf_init_connection_update(void);
QDF_STATUS cds_restart_opportunistic_timer(bool check_state);
QDF_STATUS cds_modify_sap_pcl_based_on_mandatory_channel(uint8_t *pcl_list_org,
		uint8_t *weight_list_org,
		uint32_t *pcl_len_org);
QDF_STATUS cds_update_and_wait_for_connection_update(uint8_t session_id,
		uint8_t channel, enum sir_conn_update_reason reason);
bool cds_is_sap_mandatory_channel_set(void);
bool cds_list_has_24GHz_channel(uint8_t *channel_list, uint32_t list_len);
QDF_STATUS cds_get_valid_chans(uint8_t *chan_list, uint32_t *list_len);
QDF_STATUS cds_get_nss_for_vdev(enum cds_con_mode mode,
		uint8_t *nss_2g, uint8_t *nss_5g);
QDF_STATUS cds_get_sap_mandatory_channel(uint32_t *chan);
QDF_STATUS cds_set_sap_mandatory_channels(uint8_t *channels, uint32_t len);
QDF_STATUS cds_reset_sap_mandatory_channels(void);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
QDF_STATUS cds_register_sap_restart_channel_switch_cb(
		void (*sap_restart_chan_switch_cb)(void *, uint32_t, uint32_t));
QDF_STATUS cds_deregister_sap_restart_channel_switch_cb(void);
#endif
bool cds_is_any_mode_active_on_band_along_with_session(uint8_t session_id,
						       enum cds_band band);
QDF_STATUS cds_get_mac_id_by_session_id(uint8_t session_id, uint8_t *mac_id);
QDF_STATUS cds_get_mcc_session_id_on_mac(uint8_t mac_id, uint8_t session_id,
						uint8_t *mcc_session_id);
uint8_t cds_get_mcc_operating_channel(uint8_t session_id);
QDF_STATUS cds_get_pcl_for_existing_conn(enum cds_con_mode mode,
			uint8_t *pcl_ch, uint32_t *len,
			uint8_t *weight_list, uint32_t weight_len);
QDF_STATUS cds_get_valid_chan_weights(struct sir_pcl_chan_weights *weight);
QDF_STATUS cds_set_hw_mode_on_channel_switch(uint8_t session_id);
void cds_set_do_hw_mode_change_flag(bool flag);
bool cds_is_hw_mode_change_after_vdev_up(void);
void cds_dump_connection_status_info(void);
uint32_t cds_mode_specific_connection_count(enum cds_con_mode mode,
						uint32_t *list);
void cds_hw_mode_transition_cb(uint32_t old_hw_mode_index,
			uint32_t new_hw_mode_index,
			uint32_t num_vdev_mac_entries,
			 struct sir_vdev_mac_map *vdev_mac_map);
#endif /* __CDS_CONCURRENCY_H */
