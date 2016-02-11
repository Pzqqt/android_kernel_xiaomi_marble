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
#define MAX_NUM_CHAN    128
#define DBS_OPPORTUNISTIC_TIME    10
#define CONNECTION_UPDATE_TIMEOUT 500

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
 * @CDS_MCC: switch to MCC/SCC mode
 * @CDS_MCC_UPGRADE: switch to MCC/SCC mode & upgrade to 2x2
 * @CDS_MAX_CONC_PRIORITY_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum cds_conc_next_action {
	CDS_NOP = 0,
	CDS_DBS,
	CDS_DBS_DOWNGRADE,
	CDS_MCC,
	CDS_MCC_UPGRADE,
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
 * @mac: The HW mac it is running
 * @tx_spatial_stream: Tx spatial stream used by the connection
 * @rx_spatial_stream: Tx spatial stream used by the connection
 * @original_nss: nss negotiated at connection time
 * @vdev_id: vdev id of the connection
 * @in_use: if the table entry is active
 */
struct cds_conc_connection_info {
	enum cds_con_mode mode;
	uint8_t       chan;
	uint8_t       mac;
	enum cds_chain_mode chain_mask;
	uint8_t       tx_spatial_stream;
	uint8_t       rx_spatial_stream;
	uint32_t      original_nss;
	uint32_t      vdev_id;
	bool          in_use;
};

enum cds_conc_next_action cds_get_pref_hw_mode_for_chan(uint32_t vdev_id,
		uint32_t target_channel);
bool cds_is_connection_in_progress(void);
void cds_dump_concurrency_info(void);
void cds_set_concurrency_mode(enum tQDF_ADAPTER_MODE mode);
void cds_clear_concurrency_mode(enum tQDF_ADAPTER_MODE mode);
uint32_t cds_get_connection_count(void);
bool cds_is_sta_connection_pending(void);
void cds_change_sta_conn_pending_status(bool value);
void cds_change_sap_restart_required_status(bool value);
bool cds_set_connection_in_progress(bool value);
int cds_cfg80211_get_concurrency_matrix(struct wiphy *wiphy,
			struct wireless_dev *wdev,
			const void *data,
			int data_len);
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
void cds_check_concurrent_intf_and_restart_sap(
		hdd_station_ctx_t *hdd_sta_ctx,
		hdd_adapter_t *adapter);
#else
static inline void cds_check_concurrent_intf_and_restart_sap(
		hdd_station_ctx_t *hdd_sta_ctx,
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
QDF_STATUS cds_init_policy_mgr(void);
QDF_STATUS cds_deinit_policy_mgr(void);
QDF_STATUS cds_get_pcl(enum cds_con_mode mode,
				uint8_t *pcl_Channels, uint32_t *len);
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
				enum cds_conn_update_reason);
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
enum cds_con_mode cds_convert_device_mode_to_hdd_type(
				device_mode_t device_mode);
QDF_STATUS cds_soc_set_hw_mode(uint32_t session_id,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum cds_conn_update_reason reason);
enum cds_conc_next_action cds_need_opportunistic_upgrade(void);
QDF_STATUS cds_next_actions(uint32_t session_id,
		enum cds_conc_next_action action,
		enum cds_conn_update_reason reason);
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
QDF_STATUS cds_stop_start_opportunistic_timer(void);
QDF_STATUS cds_handle_hw_mode_change_on_csa(uint16_t session_id,
		uint8_t channel, uint8_t *bssid, void *dst, void *src,
		uint32_t numbytes);
#endif /* __CDS_CONCURRENCY_H */
