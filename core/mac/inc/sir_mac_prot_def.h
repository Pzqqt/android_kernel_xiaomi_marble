/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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


/*
 * This file sir_mac_prot_def.h contains the MAC/PHY protocol
 * definitions used across various projects.
 */

#ifndef __MAC_PROT_DEFS_H
#define __MAC_PROT_DEFS_H

#include <linux/if_ether.h>

#include "cds_api.h"
#include "sir_types.h"
#include "wni_cfg.h"
#include <lim_fils_defs.h>

/* /Capability information related */
#define CAPABILITY_INFO_DELAYED_BA_BIT 14
#define CAPABILITY_INFO_IMMEDIATE_BA_BIT 15

/* / 11h MAC defaults */
#define SIR_11A_CHANNEL_BEGIN           34
#define SIR_11A_CHANNEL_END             165
#define SIR_11B_CHANNEL_BEGIN           1
#define SIR_11B_CHANNEL_END             14
#define SIR_11A_FREQUENCY_OFFSET        4
#define SIR_11B_FREQUENCY_OFFSET        1
#define SIR_11P_CHANNEL_BEGIN           170
#define SIR_11P_CHANNEL_END             184

/* / Current version of 802.11 */
#define SIR_MAC_PROTOCOL_VERSION 0

/* Frame Type definitions */

#define SIR_MAC_MGMT_FRAME    0x0
#define SIR_MAC_CTRL_FRAME    0x1
#define SIR_MAC_DATA_FRAME    0x2

#define SIR_MAC_FRAME_TYPE_START   0x0
#define SIR_MAC_FRAME_TYPE_END     0x3

/* Control frame subtype definitions */

#define SIR_MAC_CTRL_RR         4
#define SIR_MAC_CTRL_BAR        8
#define SIR_MAC_CTRL_BA         9
#define SIR_MAC_CTRL_PS_POLL    10
#define SIR_MAC_CTRL_RTS        11
#define SIR_MAC_CTRL_CTS        12
#define SIR_MAC_CTRL_ACK        13
#define SIR_MAC_CTRL_CF_END     14
#define SIR_MAC_CTRL_CF_END_ACK 15

#define SIR_MAC_MAX_DURATION_MICRO_SECONDS       32767

/* Data frame subtype definitions */
#define SIR_MAC_DATA_DATA                 0
#define SIR_MAC_DATA_DATA_ACK             1
#define SIR_MAC_DATA_DATA_POLL            2
#define SIR_MAC_DATA_DATA_ACK_POLL        3
#define SIR_MAC_DATA_NULL                 4
#define SIR_MAC_DATA_NULL_ACK             5
#define SIR_MAC_DATA_NULL_POLL            6
#define SIR_MAC_DATA_NULL_ACK_POLL        7
#define SIR_MAC_DATA_QOS_DATA             8
#define SIR_MAC_DATA_QOS_DATA_ACK         9
#define SIR_MAC_DATA_QOS_DATA_POLL        10
#define SIR_MAC_DATA_QOS_DATA_ACK_POLL    11
#define SIR_MAC_DATA_QOS_NULL             12
#define SIR_MAC_DATA_QOS_NULL_ACK         13
#define SIR_MAC_DATA_QOS_NULL_POLL        14
#define SIR_MAC_DATA_QOS_NULL_ACK_POLL    15

#define SIR_MAC_FRAME_SUBTYPE_START       0
#define SIR_MAC_FRAME_SUBTYPE_END         16

#define SIR_MAC_DATA_QOS_MASK             8
#define SIR_MAC_DATA_NULL_MASK            4
#define SIR_MAC_DATA_POLL_MASK            2
#define SIR_MAC_DATA_ACK_MASK             1

/* Management frame subtype definitions */

#define SIR_MAC_MGMT_ASSOC_REQ    0x0
#define SIR_MAC_MGMT_ASSOC_RSP    0x1
#define SIR_MAC_MGMT_REASSOC_REQ  0x2
#define SIR_MAC_MGMT_REASSOC_RSP  0x3
#define SIR_MAC_MGMT_PROBE_REQ    0x4
#define SIR_MAC_MGMT_PROBE_RSP    0x5
#define SIR_MAC_MGMT_TIME_ADVERT  0x6
#define SIR_MAC_MGMT_BEACON       0x8
#define SIR_MAC_MGMT_ATIM         0x9
#define SIR_MAC_MGMT_DISASSOC     0xA
#define SIR_MAC_MGMT_AUTH         0xB
#define SIR_MAC_MGMT_DEAUTH       0xC
#define SIR_MAC_MGMT_ACTION       0xD
#define SIR_MAC_MGMT_RESERVED15   0xF

/* Action frame categories */

#define SIR_MAC_ACTION_SPECTRUM_MGMT   0
#define SIR_MAC_ACTION_QOS_MGMT        1
#define SIR_MAC_ACTION_DLP             2
#define SIR_MAC_ACTION_BLKACK          3
#define SIR_MAC_ACTION_PUBLIC_USAGE    4
#define SIR_MAC_ACTION_RRM             5
#define SIR_MAC_ACTION_FAST_BSS_TRNST  6
#define SIR_MAC_ACTION_HT              7
#define SIR_MAC_ACTION_SA_QUERY        8
#define SIR_MAC_ACTION_PROT_DUAL_PUB   9
#define SIR_MAC_ACTION_WNM            10
#define SIR_MAC_ACTION_UNPROT_WNM     11
#define SIR_MAC_ACTION_TDLS           12
#define SIR_MAC_ACITON_MESH           13
#define SIR_MAC_ACTION_MHF            14
#define SIR_MAC_SELF_PROTECTED        15
#define SIR_MAC_ACTION_WME            17
#define SIR_MAC_ACTION_FST            18
#define SIR_MAC_ACTION_VHT            21
#define SIR_MAC_ACTION_MAX            256

#define SIR_MAC_ACTION_TX             1
#define SIR_MAC_ACTION_RX             2

/* QoS management action codes */

#define SIR_MAC_QOS_ADD_TS_REQ      0
#define SIR_MAC_QOS_ADD_TS_RSP      1
#define SIR_MAC_QOS_DEL_TS_REQ      2
#define SIR_MAC_QOS_SCHEDULE        3
#define SIR_MAC_QOS_MAP_CONFIGURE   4
/* and these are proprietary */
#define SIR_MAC_QOS_DEF_BA_REQ      4
#define SIR_MAC_QOS_DEF_BA_RSP      5

#define SIR_MAC_ADDBA_REQ     0
#define SIR_MAC_ADDBA_RSP     1
#define SIR_MAC_DELBA_REQ     2

#define SIR_MAC_BA_POLICY_DELAYED      0
#define SIR_MAC_BA_POLICY_IMMEDIATE    1
#define SIR_MAC_BA_AMSDU_SUPPORTED     1

#ifdef ANI_SUPPORT_11H
#define SIR_MAC_ACTION_MEASURE_REQUEST_ID      0
#define SIR_MAC_ACTION_MEASURE_REPORT_ID       1
#define SIR_MAC_ACTION_TPC_REQUEST_ID          2
#define SIR_MAC_ACTION_TPC_REPORT_ID           3
#endif /* ANI_SUPPORT_11H */
#define SIR_MAC_ACTION_CHANNEL_SWITCH_ID       4

#ifdef ANI_SUPPORT_11H
#define SIR_MAC_BASIC_MEASUREMENT_TYPE         0
#define SIR_MAC_CCA_MEASUREMENT_TYPE           1
#define SIR_MAC_RPI_MEASUREMENT_TYPE           2
#endif /* ANI_SUPPORT_11H */

/* RRM related. */
/* Refer IEEE Std 802.11k-2008, Section 7.3.2.21, table 7.29 */

#define SIR_MAC_RRM_CHANNEL_LOAD_TYPE          3
#define SIR_MAC_RRM_NOISE_HISTOGRAM_BEACON     4
#define SIR_MAC_RRM_BEACON_TYPE                5
#define SIR_MAC_RRM_FRAME_TYPE                 6
#define SIR_MAC_RRM_STA_STATISTICS_TYPE        7
#define SIR_MAC_RRM_LCI_TYPE                   8
#define SIR_MAC_RRM_TSM_TYPE                   9
#define SIR_MAC_RRM_LOCATION_CIVIC_TYPE        11
#define SIR_MAC_RRM_FINE_TIME_MEAS_TYPE        16

/* RRM action codes */
#define SIR_MAC_RRM_RADIO_MEASURE_REQ          0
#define SIR_MAC_RRM_RADIO_MEASURE_RPT          1
#define SIR_MAC_RRM_LINK_MEASUREMENT_REQ       2
#define SIR_MAC_RRM_LINK_MEASUREMENT_RPT       3
#define SIR_MAC_RRM_NEIGHBOR_REQ               4
#define SIR_MAC_RRM_NEIGHBOR_RPT               5

/* VHT Action Field */
#define SIR_MAC_VHT_GID_NOTIFICATION           1
#define SIR_MAC_VHT_OPMODE_NOTIFICATION        2

#define NUM_OF_SOUNDING_DIMENSIONS	1 /*Nss - 1, (Nss = 2 for 2x2)*/
/* HT Action Field Codes */
#define SIR_MAC_SM_POWER_SAVE       1

/* DLP action frame types */
#define SIR_MAC_DLP_REQ             0
#define SIR_MAC_DLP_RSP             1
#define SIR_MAC_DLP_TEARDOWN        2

/* block acknowledgement action frame types */
#define SIR_MAC_ACTION_VENDOR_SPECIFIC 9
#define SIR_MAC_ACTION_VENDOR_SPECIFIC_CATEGORY     0x7F
#define SIR_MAC_ACTION_P2P_SUBTYPE_PRESENCE_RSP     2

/* Public Action for 20/40 BSS Coexistence */
#define SIR_MAC_ACTION_2040_BSS_COEXISTENCE     0
#define SIR_MAC_ACTION_EXT_CHANNEL_SWITCH_ID    4

/* Public Action frames for GAS */
#define SIR_MAC_ACTION_GAS_INITIAL_REQUEST      0x0A
#define SIR_MAC_ACTION_GAS_INITIAL_RESPONSE     0x0B
#define SIR_MAC_ACTION_GAS_COMEBACK_REQUEST     0x0C
#define SIR_MAC_ACTION_GAS_COMEBACK_RESPONSE    0x0D

#ifdef WLAN_FEATURE_11W
/* 11w SA query request/response action frame category code */
#define SIR_MAC_SA_QUERY_REQ             0
#define SIR_MAC_SA_QUERY_RSP             1
#endif

#ifdef FEATURE_WLAN_TDLS
#define SIR_MAC_TDLS_SETUP_REQ           0
#define SIR_MAC_TDLS_SETUP_RSP           1
#define SIR_MAC_TDLS_SETUP_CNF           2
#define SIR_MAC_TDLS_TEARDOWN            3
#define SIR_MAC_TDLS_PEER_TRAFFIC_IND    4
#define SIR_MAC_TDLS_CH_SWITCH_REQ       5
#define SIR_MAC_TDLS_CH_SWITCH_RSP       6
#define SIR_MAC_TDLS_PEER_TRAFFIC_RSP    9
#define SIR_MAC_TDLS_DIS_REQ             10
#define SIR_MAC_TDLS_DIS_RSP             14
#endif

/* WNM Action field values; IEEE Std 802.11-2012, 8.5.14.1, Table 8-250 */
#define SIR_MAC_WNM_BSS_TM_QUERY         6
#define SIR_MAC_WNM_BSS_TM_REQUEST       7
#define SIR_MAC_WNM_BSS_TM_RESPONSE      8
#define SIR_MAC_WNM_NOTIF_REQUEST        26
#define SIR_MAC_WNM_NOTIF_RESPONSE       27

/* Protected Dual of Public Action(PDPA) frames Action field */
#define SIR_MAC_PDPA_GAS_INIT_REQ      10
#define SIR_MAC_PDPA_GAS_INIT_RSP      11
#define SIR_MAC_PDPA_GAS_COMEBACK_REQ  12
#define SIR_MAC_PDPA_GAS_COMEBACK_RSP  13

#define SIR_MAC_MAX_RANDOM_LENGTH   2306

/* ----------------------------------------------------------------------------- */
/* EID (Element ID) definitions */
/* and their min/max lengths */
/* ----------------------------------------------------------------------------- */

#define SIR_MAC_SSID_EID               0
#define SIR_MAC_SSID_EID_MIN               0
#define SIR_MAC_SSID_EID_MAX               32
#define SIR_MAC_RATESET_EID            1
#define SIR_MAC_RATESET_EID_MIN            1
#define SIR_MAC_RATESET_EID_MAX            12
#define SIR_MAC_FH_PARAM_SET_EID       2
#define SIR_MAC_FH_PARAM_SET_EID_MIN       5
#define SIR_MAC_FH_PARAM_SET_EID_MAX       5
#define SIR_MAC_DS_PARAM_SET_EID       3
#define SIR_MAC_DS_PARAM_SET_EID_MIN       1
#define SIR_MAC_DS_PARAM_SET_EID_MAX       1
#define SIR_MAC_CF_PARAM_SET_EID       4
#define SIR_MAC_CF_PARAM_SET_EID_MIN       6
#define SIR_MAC_CF_PARAM_SET_EID_MAX       6
#define SIR_MAC_TIM_EID                5
#define SIR_MAC_TIM_EID_MIN                3
#define SIR_MAC_TIM_EID_MAX                254
#define SIR_MAC_IBSS_PARAM_SET_EID     6
#define SIR_MAC_IBSS_PARAM_SET_EID_MIN     2
#define SIR_MAC_IBSS_PARAM_SET_EID_MAX     2
#define SIR_MAC_COUNTRY_EID            7
#define SIR_MAC_COUNTRY_EID_MIN            6
#define SIR_MAC_COUNTRY_EID_MAX            254
#define SIR_MAC_FH_PARAMS_EID          8
#define SIR_MAC_FH_PARAMS_EID_MIN          4
#define SIR_MAC_FH_PARAMS_EID_MAX          4
#define SIR_MAC_FH_PATTERN_EID         9
#define SIR_MAC_FH_PATTERN_EID_MIN         4
#define SIR_MAC_FH_PATTERN_EID_MAX         254
#define SIR_MAC_REQUEST_EID            10
#define SIR_MAC_REQUEST_EID_MIN            1
#define SIR_MAC_REQUEST_EID_MAX            255
#define SIR_MAC_QBSS_LOAD_EID          11
#define SIR_MAC_QBSS_LOAD_EID_MIN          5
#define SIR_MAC_QBSS_LOAD_EID_MAX          5
#define SIR_MAC_EDCA_PARAM_SET_EID     12       /* EDCA parameter set */
#define SIR_MAC_EDCA_PARAM_SET_EID_MIN     18
#define SIR_MAC_EDCA_PARAM_SET_EID_MAX     20   /* TBD temp - change backto 18 */
#define SIR_MAC_TSPEC_EID              13
#define SIR_MAC_TSPEC_EID_MIN              55
#define SIR_MAC_TSPEC_EID_MAX              55
#define SIR_MAC_TCLAS_EID              14
#define SIR_MAC_TCLAS_EID_MIN              4
#define SIR_MAC_TCLAS_EID_MAX              255
#define SIR_MAC_QOS_SCHEDULE_EID       15
#define SIR_MAC_QOS_SCHEDULE_EID_MIN       14
#define SIR_MAC_QOS_SCHEDULE_EID_MAX       14
#define SIR_MAC_CHALLENGE_TEXT_EID     16
#define SIR_MAC_CHALLENGE_TEXT_EID_MIN     1
#define SIR_MAC_CHALLENGE_TEXT_EID_MAX     253
/* reserved       17-31 */
#define SIR_MAC_PWR_CONSTRAINT_EID     32
#define SIR_MAC_PWR_CONSTRAINT_EID_MIN     1
#define SIR_MAC_PWR_CONSTRAINT_EID_MAX     1
#define SIR_MAC_PWR_CAPABILITY_EID     33
#define SIR_MAC_PWR_CAPABILITY_EID_MIN     2
#define SIR_MAC_PWR_CAPABILITY_EID_MAX     2
#define SIR_MAC_TPC_REQ_EID            34
#define SIR_MAC_TPC_REQ_EID_MIN            0
#define SIR_MAC_TPC_REQ_EID_MAX            255
/* SIR_MAC_EXTENDED_CAP_EID    35 */
#define SIR_MAC_TPC_RPT_EID            35
#define SIR_MAC_TPC_RPT_EID_MIN            2
#define SIR_MAC_TPC_RPT_EID_MAX            2
#define SIR_MAC_SPRTD_CHNLS_EID        36
#define SIR_MAC_SPRTD_CHNLS_EID_MIN        2
#define SIR_MAC_SPRTD_CHNLS_EID_MAX        254
#define SIR_MAC_CHNL_SWITCH_ANN_EID    37
#define SIR_MAC_CHNL_SWITCH_ANN_EID_MIN    3
#define SIR_MAC_CHNL_SWITCH_ANN_EID_MAX    3
#define SIR_MAC_MEAS_REQ_EID           38
#define SIR_MAC_MEAS_REQ_EID_MIN           3
#define SIR_MAC_MEAS_REQ_EID_MAX           255
#define SIR_MAC_MEAS_RPT_EID           39
#define SIR_MAC_MEAS_RPT_EID_MIN           3
#define SIR_MAC_MEAS_RPT_EID_MAX           255
#define SIR_MAC_QUIET_EID              40
#define SIR_MAC_QUIET_EID_MIN              6
#define SIR_MAC_QUIET_EID_MAX              6
#define SIR_MAC_IBSS_DFS_EID           41
#define SIR_MAC_IBSS_DFS_EID_MIN           7
#define SIR_MAC_IBSS_DFS_EID_MAX           255
#define SIR_MAC_ERP_INFO_EID           42
#define SIR_MAC_ERP_INFO_EID_MIN           0
#define SIR_MAC_ERP_INFO_EID_MAX           255
#define SIR_MAC_TS_DELAY_EID           43
#define SIR_MAC_TS_DELAY_EID_MIN           4
#define SIR_MAC_TS_DELAY_EID_MAX           4
#define SIR_MAC_TCLAS_PROC_EID         44
#define SIR_MAC_TCLAS_PROC_EID_MIN         1
#define SIR_MAC_TCLAS_PROC_EID_MAX         1
#define SIR_MAC_QOS_CAPABILITY_EID     46
#define SIR_MAC_QOS_CAPABILITY_EID_MIN     1
#define SIR_MAC_QOS_CAPABILITY_EID_MAX     1
#define SIR_MAC_RSN_EID                48
#define SIR_MAC_RSN_EID_MIN                4
#define SIR_MAC_RSN_EID_MAX                254

/* using reserved EID for Qos Action IE for now, */
/* need to check 11e spec for the actual EID */
#define SIR_MAC_QOS_ACTION_EID         49
#define SIR_MAC_QOS_ACTION_EID_MIN         4
#define SIR_MAC_QOS_ACTION_EID_MAX         255
#define SIR_MAC_EXTENDED_RATE_EID      50
#define SIR_MAC_EXTENDED_RATE_EID_MIN      0
#define SIR_MAC_EXTENDED_RATE_EID_MAX      255
#define SIR_MAC_CHNL_EXTENDED_SWITCH_ANN_EID 60
#define SIR_MAC_CHNL_EXTENDED_SWITCH_ANN_EID_MIN    0
#define SIR_MAC_CHNL_EXTENDED_SWITCH_ANN_EID_MAX    255

#define SIR_MAC_OPERATING_CLASS_EID    59
#define SIR_MAC_OPERATING_CLASS_EID_MIN    2
#define SIR_MAC_OPERATING_CLASS_EID_MAX    253
/* reserved       51-69 */
#define SIR_MAC_RM_ENABLED_CAPABILITY_EID      70
#define SIR_MAC_RM_ENABLED_CAPABILITY_EID_MIN  5
#define SIR_MAC_RM_ENABLED_CAPABILITY_EID_MAX  5
/* reserved       71-220 */
#define SIR_MAC_WPA_EID                221
#define SIR_MAC_WPA_EID_MIN                0
#define SIR_MAC_WPA_EID_MAX                255

#define SIR_MAC_EID_VENDOR                221

#define SIR_MAC_WAPI_EID                68
/* reserved                            222-254 */
#define SIR_MAC_HT_CAPABILITIES_EID    45
#define SIR_MAC_HT_CAPABILITIES_EID_MIN    0
#define SIR_MAC_HT_CAPABILITIES_EID_MAX    255
#define SIR_MAC_HT_INFO_EID      61
#define SIR_MAC_HT_INFO_EID_MIN    0
#define SIR_MAC_HT_INFO_EID_MAX    255

#define SIR_MAC_VHT_CAPABILITIES_EID   191
#define SIR_MAC_VHT_OPERATION_EID      192
#define SIR_MAC_VHT_EXT_BSS_LOAD_EID   193
#define SIR_MAC_VHT_OPMODE_EID         199
#define SIR_MAC_MAX_SUPPORTED_MCS_SET    16

#define VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_1_1       390
#define VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_1_1       390
#define VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_2_2       780
#define VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_2_2       780

#define VHT_CAP_160_SUPP 1
#define VHT_CAP_160_AND_80P80_SUPP 2

#define VHT_MCS_1x1 0xFFFC
#define VHT_MCS_2x2 0xFFF3

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
#define SIR_MAC_QCOM_VENDOR_EID      200
#define SIR_MAC_QCOM_VENDOR_OUI      "\x00\xA0\xC6"
#define SIR_MAC_QCOM_VENDOR_SIZE     3
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

/* / Workaround IE to change beacon length when it is 4*n+1 */
#define SIR_MAC_ANI_WORKAROUND_EID     255
#define SIR_MAC_ANI_WORKAROUND_EID_MIN     0
#define SIR_MAC_ANI_WORKAROUND_EID_MAX     255

#define SIR_MAC_MAX_ADD_IE_LENGTH       2048

/* / Maximum length of each IE */
#define SIR_MAC_MAX_IE_LENGTH       255

/* / Maximum length of each IE */
#define SIR_MAC_RSN_IE_MAX_LENGTH   255
/* / Minimum length of each IE */
#define SIR_MAC_RSN_IE_MIN_LENGTH   2
#define SIR_MAC_WPA_IE_MIN_LENGTH   6

#ifdef FEATURE_WLAN_ESE
#define ESE_VERSION_4               4
#define ESE_VERSION_SUPPORTED       ESE_VERSION_4

/* When station sends Radio Management Cap. */
/* State should be normal=1 */
/* Mbssid Mask should be 0 */
#define RM_STATE_NORMAL             1
#endif

#define SIR_MAC_OUI_VERSION_1         1

/* OWE DH Parameter element https://tools.ietf.org/html/rfc8110 */
#define SIR_DH_PARAMETER_ELEMENT_EXT_EID 32

/* OUI and type definition for WPA IE in network byte order */
#define SIR_MAC_WPA_OUI             0x01F25000
#define SIR_MAC_WME_OUI             0x02F25000
#define SIR_MAC_WSM_OUI             SIR_MAC_WME_OUI
#define SIR_MAC_WSC_OUI             "\x00\x50\xf2\x04"
#define SIR_MAC_WSC_OUI_SIZE        4
#define SIR_MAC_P2P_OUI             "\x50\x6f\x9a\x09"
#define SIR_MAC_P2P_OUI_SIZE        4
#define SIR_P2P_NOA_ATTR            12
#define SIR_MAX_NOA_ATTR_LEN        31
#define SIR_MAX_NOA_DESCR           2
#define SIR_P2P_IE_HEADER_LEN       6

#define SIR_MAC_CISCO_OUI "\x00\x40\x96"
#define SIR_MAC_CISCO_OUI_SIZE 3

#define SIR_MAC_QCN_OUI_TYPE   "\x8c\xfd\xf0\x01"
#define SIR_MAC_QCN_OUI_TYPE_SIZE  4

/* min size of wme oui header: oui(3) + type + subtype + version */
#define SIR_MAC_OUI_WME_HDR_MIN       6

/* OUI subtype and their lengths */
#define SIR_MAC_OUI_SUBTYPE_WME_INFO  0
#define SIR_MAC_OUI_WME_INFO_MIN      7
#define SIR_MAC_OUI_WME_INFO_MAX      7

#define SIR_MAC_OUI_SUBTYPE_WME_PARAM 1
#define SIR_MAC_OUI_WME_PARAM_MIN     24
#define SIR_MAC_OUI_WME_PARAM_MAX     24

#define SIR_MAC_OUI_SUBTYPE_WME_TSPEC 2
#define SIR_MAC_OUI_WME_TSPEC_MIN     61
#define SIR_MAC_OUI_WME_TSPEC_MAX     61

#define SIR_MAC_OUI_SUBTYPE_WSM_TSPEC 2 /* same as WME TSPEC */
#define SIR_MAC_OUI_WSM_TSPEC_MIN     61
#define SIR_MAC_OUI_WSM_TSPEC_MAX     61

/* reserved subtypes                        3-4 */
/* WSM capability */
#define SIR_MAC_OUI_SUBTYPE_WSM_CAPABLE     5
#define SIR_MAC_OUI_WSM_CAPABLE_MIN         7
#define SIR_MAC_OUI_WSM_CAPABLE_MAX         7
/* WSM classifier */
#define SIR_MAC_OUI_SUBTYPE_WSM_TCLAS       6
#define SIR_MAC_OUI_WSM_TCLAS_MIN           10
#define SIR_MAC_OUI_WSM_TCLAS_MAX           255
/* classifier processing element */
#define SIR_MAC_OUI_SUBTYPE_WSM_TCLASPROC   7
#define SIR_MAC_OUI_WSM_TCLASPROC_MIN       7
#define SIR_MAC_OUI_WSM_TCLASPROC_MAX       7
/* tspec delay element */
#define SIR_MAC_OUI_SUBTYPE_WSM_TSDELAY     8
#define SIR_MAC_OUI_WSM_TSDELAY_MIN         10
#define SIR_MAC_OUI_WSM_TSDELAY_MAX         10
/* schedule element */
#define SIR_MAC_OUI_SUBTYPE_WSM_SCHEDULE    9
#define SIR_MAC_OUI_WSM_SCHEDULE_MIN        20
#define SIR_MAC_OUI_WSM_SCHEDULE_MAX        20

#ifdef WLAN_NS_OFFLOAD
#define SIR_MAC_NS_OFFLOAD_SIZE             1   /* support only one IPv6 offload */
/* Number of target IP V6 addresses for NS offload */
#define SIR_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA   16
#define SIR_MAC_IPV6_ADDR_LEN               16
#define SIR_IPV6_ADDR_VALID                 1
#define SIR_IPV6_ADDR_UC_TYPE               0
#define SIR_IPV6_ADDR_AC_TYPE               1
#endif /* WLAN_NS_OFFLOAD */

/* ----------------------------------------------------------------------------- */

/* OFFSET definitions for fixed fields in Management frames */

/* Beacon/Probe Response offsets */
#define SIR_MAC_TS_OFFSET                    0
#define SIR_MAC_BEACON_INT_OFFSET            8  /* Beacon Interval offset */
#define SIR_MAC_B_PR_CAPAB_OFFSET            10
#define SIR_MAC_B_PR_SSID_OFFSET             12

/* Association/Reassociation offsets */
#define SIR_MAC_ASSOC_CAPAB_OFFSET           0
#define SIR_MAC_LISTEN_INT_OFFSET            2  /* Listen Interval offset */
#define SIR_MAC_ASSOC_SSID_OFFSET            4
#define SIR_MAC_CURRENT_AP_OFFSET            4
#define SIR_MAC_REASSOC_SSID_OFFSET          10
#define SIR_MAC_ASSOC_STATUS_CODE_OFFSET     2
#define SIR_MAC_ASSOC_AID_OFFSET             4
#define SIR_MAC_ASSOC_RSP_RATE_OFFSET        6

/* Disassociation/Deauthentication offsets */
#define SIR_MAC_REASON_CODE_OFFSET           0

/* Probe Request offset */
#define SIR_MAC_PROBE_REQ_SSID_OFFSET        0

/* Authentication offsets */
#define SIR_MAC_AUTH_ALGO_OFFSET             0
#define SIR_MAC_AUTH_XACT_SEQNUM_OFFSET      2
#define SIR_MAC_AUTH_STATUS_CODE_OFFSET      4

/* / Transaction sequence number definitions (used in Authentication frames) */
#define    SIR_MAC_AUTH_FRAME_1        1
#define    SIR_MAC_AUTH_FRAME_2        2
#define    SIR_MAC_AUTH_FRAME_3        3
#define    SIR_MAC_AUTH_FRAME_4        4

/* / Protocol defined MAX definitions */
#define SIR_MAC_MAX_SSID_LENGTH              32
#define SIR_MAC_MAX_NUMBER_OF_RATES          12
#define SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS      4
#define SIR_MAC_KEY_LENGTH                   13 /* WEP Maximum key length size */
#define SIR_MAC_AUTH_CHALLENGE_LENGTH        253
#define SIR_MAC_SAP_AUTH_CHALLENGE_LENGTH    128
#define SIR_MAC_WEP_IV_LENGTH                4
#define SIR_MAC_WEP_ICV_LENGTH               4
#define SIR_MAC_CHALLENGE_ID_LEN             2

/* 2 bytes each for auth algo number, transaction number and status code */
#define SIR_MAC_AUTH_FRAME_INFO_LEN          6
/* 2 bytes for ID and length + SIR_MAC_AUTH_CHALLENGE_LENGTH */
#define SIR_MAC_AUTH_CHALLENGE_BODY_LEN    (SIR_MAC_CHALLENGE_ID_LEN + \
					    SIR_MAC_AUTH_CHALLENGE_LENGTH)

/* / MAX key length when ULA is used */
#define SIR_MAC_MAX_KEY_LENGTH               32
#define SIR_MAC_MAX_KEY_RSC_LEN              16

/* / Macro definitions for get/set on FC fields */
#define SIR_MAC_GET_PROT_VERSION(x)      ((((uint16_t) x) & 0x0300) >> 8)
#define SIR_MAC_GET_FRAME_TYPE(x)        ((((uint16_t) x) & 0x0C00) >> 8)
#define SIR_MAC_GET_FRAME_SUB_TYPE(x)    ((((uint16_t) x) & 0xF000) >> 12)
#define SIR_MAC_GET_WEP_BIT_IN_FC(x)     (((uint16_t) x) & 0x0040)
#define SIR_MAC_SET_PROT_VERSION(x)      ((uint16_t) x)
#define SIR_MAC_SET_FRAME_TYPE(x)        (((uint16_t) x) << 2)
#define SIR_MAC_SET_FRAME_SUB_TYPE(x)    (((uint16_t) x) << 4)
#define SIR_MAC_SET_WEP_BIT_IN_FC(x)     (((uint16_t) x) << 14)

/* / Macro definitions for get/set on capabilityInfo bits */
#define SIR_MAC_GET_ESS(x)               (((uint16_t) x) & 0x0001)
#define SIR_MAC_GET_IBSS(x)              ((((uint16_t) x) & 0x0002) >> 1)
#define SIR_MAC_GET_CF_POLLABLE(x)       ((((uint16_t) x) & 0x0004) >> 2)
#define SIR_MAC_GET_CF_POLL_REQ(x)       ((((uint16_t) x) & 0x0008) >> 3)
#define SIR_MAC_GET_PRIVACY(x)           ((((uint16_t) x) & 0x0010) >> 4)
#define SIR_MAC_GET_SHORT_PREAMBLE(x)    ((((uint16_t) x) & 0x0020) >> 5)
#define SIR_MAC_GET_SPECTRUM_MGMT(x)     ((((uint16_t) x) & 0x0100) >> 8)
#define SIR_MAC_GET_QOS(x)               ((((uint16_t) x) & 0x0200) >> 9)
#define SIR_MAC_GET_SHORT_SLOT_TIME(x)   ((((uint16_t) x) & 0x0400) >> 10)
#define SIR_MAC_GET_APSD(x)              ((((uint16_t) x) & 0x0800) >> 11)
#define SIR_MAC_GET_RRM(x)               ((((uint16_t) x) & 0x1000) >> 12)
#define SIR_MAC_GET_BLOCK_ACK(x)         ((((uint16_t) x) & 0xc000) >> CAPABILITY_INFO_DELAYED_BA_BIT)
#define SIR_MAC_SET_ESS(x)               (((uint16_t) x) | 0x0001)
#define SIR_MAC_SET_IBSS(x)              (((uint16_t) x) | 0x0002)
#define SIR_MAC_SET_CF_POLLABLE(x)       (((uint16_t) x) | 0x0004)
#define SIR_MAC_SET_CF_POLL_REQ(x)       (((uint16_t) x) | 0x0008)
#define SIR_MAC_SET_PRIVACY(x)           (((uint16_t) x) | 0x0010)
#define SIR_MAC_SET_SHORT_PREAMBLE(x)    (((uint16_t) x) | 0x0020)
#define SIR_MAC_SET_SPECTRUM_MGMT(x)     (((uint16_t) x) | 0x0100)
#define SIR_MAC_SET_QOS(x)               (((uint16_t) x) | 0x0200)
#define SIR_MAC_SET_SHORT_SLOT_TIME(x)   (((uint16_t) x) | 0x0400)
#define SIR_MAC_SET_APSD(x)              (((uint16_t) x) | 0x0800)
#define SIR_MAC_SET_RRM(x)               (((uint16_t) x) | 0x1000)
#define SIR_MAC_SET_GROUP_ACK(x)         (((uint16_t) x) | 0x4000)

#define SIR_MAC_GET_VHT_MAX_AMPDU_EXPO(x) ((((uint32_t) x) & 0x03800000) >> 23)

/* bitname must be one of the above, eg ESS, CF_POLLABLE, etc. */
#define SIR_MAC_CLEAR_CAPABILITY(u16value, bitname) \
	((u16value) &= (~(SIR_MAC_SET_ ## bitname(0))))

#define IS_WES_MODE_ENABLED(x) \
	((x)->roam.configParam.isWESModeEnabled)

#define BA_RECIPIENT       1
#define BA_INITIATOR       2
#define BA_BOTH_DIRECTIONS 3

#define SIR_MAC_VENDOR_AP_1_OUI             "\x00\x0C\x43"
#define SIR_MAC_VENDOR_AP_1_OUI_LEN         3

#define SIR_MAC_VENDOR_AP_2_OUI             "\x00\x10\x18"
#define SIR_MAC_VENDOR_AP_2_OUI_LEN         3

#define SIR_MAC_VENDOR_AP_2_DATA            "\x02\xFF\xF0\x2C\x00\x00"
#define SIR_MAC_VENDOR_AP_2_DATA_2          "\x02\xFF\x04\x0C\x00\x00"
#define SIR_MAC_VENDOR_AP_2_DATA_LEN        6

#define SIR_MAC_VENDOR_AP_3_OUI             "\x00\x03\x7F"
#define SIR_MAC_VENDOR_AP_3_OUI_LEN         3

#define SIR_MAC_VENDOR_AP_4_OUI             "\x8C\xFD\xF0"
#define SIR_MAC_VENDOR_AP_4_OUI_LEN         3

/* / Status Code (present in Management response frames) enum */

typedef enum eSirMacStatusCodes {
	eSIR_MAC_SUCCESS_STATUS = 0,    /* Reserved */
	eSIR_MAC_UNSPEC_FAILURE_STATUS = 1,     /* Unspecified reason */
	/* 802.11 reserved                              2-9 */
	/*
	   WMM status codes(standard 1.1 table 9)
	   Table 9 ADDTS Response Status Codes
	   Value Operation
	   0 Admission accepted
	   1 Invalid parameters
	   2 Reserved
	   3 Refused
	   4-255 Reserved
	 */
	eSIR_MAC_WME_INVALID_PARAMS_STATUS = 1, /* ?? */
	eSIR_MAC_WME_REFUSED_STATUS = 3,        /* ?? */
	eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS = 10,        /* Cannot support all requested capabilities in the Capability Information field */
	eSIR_MAC_INABLITY_TO_CONFIRM_ASSOC_STATUS = 11, /* Reassociation denied due to inability to confirm that association exists */
	eSIR_MAC_OUTSIDE_SCOPE_OF_SPEC_STATUS = 12,     /* Association denied due to reason outside the scope of this standard */
	eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS = 13,   /* Responding station does not support the specified authentication algorithm */
	eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS = 14,     /* Received an Authentication frame with authentication transaction sequence number */
	/* out of expected sequence */
	eSIR_MAC_CHALLENGE_FAILURE_STATUS = 15, /* Authentication rejected because of challenge failure */
	eSIR_MAC_AUTH_RSP_TIMEOUT_STATUS = 16,  /* Authentication rejected due to timeout waiting for next frame in sequence */
	eSIR_MAC_MAX_ASSOC_STA_REACHED_STATUS = 17,     /* Association denied because AP is unable to handle additional associated stations */
	eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS = 18, /* Association denied due to requesting station not supporting all of the data rates in the */
	/* BSSBasicRateSet parameter */
	eSIR_MAC_SHORT_PREAMBLE_NOT_SUPPORTED_STATUS = 19,      /* Association denied due to requesting station not supporting the short preamble */
	/* option */
	eSIR_MAC_PBCC_NOT_SUPPORTED_STATUS = 20,        /* Association denied due to requesting station not supporting the PBCC modulation */
	/* option */
	eSIR_MAC_CHANNEL_AGILITY_NOT_SUPPORTED_STATUS = 21,     /* Association denied due to requesting station not supporting the Channel Agility */
	/* option */
	eSIR_MAC_SPECTRUM_MGMT_REQD_STATUS = 22,        /* Association request rejected because Spectrum Management capability is required */
	eSIR_MAC_PWR_CAPABILITY_BAD_STATUS = 23,        /* Association request rejected because the information in the Power Capability */
	/* element is unacceptable */
	eSIR_MAC_SPRTD_CHANNELS_BAD_STATUS = 24,        /* Association request rejected because the information in the Supported Channels */
	/* element is unacceptable */
	eSIR_MAC_SHORT_SLOT_NOT_SUPORTED_STATUS = 25,   /* Association denied due to requesting station not supporting the Short Slot Time */
	/* option */
	eSIR_MAC_DSSS_OFDM_NOT_SUPPORTED_STATUS = 26,   /* Association denied due to requesting station not supporting the DSSS-OFDM option */
	/* reserved                                     27-29 */
	eSIR_MAC_TRY_AGAIN_LATER = 30,  /* Association request rejected temporarily, try again later */
	/* reserved                                     31 */
	eSIR_MAC_QOS_UNSPECIFIED_FAILURE_STATUS = 32,   /* Unspecified, QoS-related failure */
	eSIR_MAC_QAP_NO_BANDWIDTH_STATUS = 33,  /* Association denied because QoS AP has insufficient bandwidth to handle another */
	/* QoS STA */
	/*
	 * Association denied due to excessive frame loss rates
	 * and/or poor conditions/RSSI on cur channel
	 */
	eSIR_MAC_XS_FRAME_LOSS_POOR_CHANNEL_RSSI_STATUS = 34,
	/* rent operating channel */
	eSIR_MAC_STA_QOS_NOT_SUPPORTED_STATUS = 35,     /* Association (with QoS BSS) denied because the requesting STA does not support the */
	/* QoS facility */
	eSIR_MAC_STA_BLK_ACK_NOT_SUPPORTED_STATUS = 36, /* Reserved */
	eSIR_MAC_REQ_DECLINED_STATUS = 37,      /* The request has been declined */
	eSIR_MAC_INVALID_PARAM_STATUS = 38,     /* The request has not been successful as one or more parameters have invalid values */
	eSIR_MAC_TS_NOT_HONOURED_STATUS = 39,   /* The TS has not been created because the request cannot be honored; however, a suggested */
	/* TSPEC is provided so that the initiating STA may attempt to set another TS */
	/* with the suggested changes to the TSPEC */
	eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS = 40,       /* Invalid information element, i.e., an information element defined in this standard for */
	/* which the content does not meet the specifications in Clause 7 */
	eSIR_MAC_INVALID_GROUP_CIPHER_STATUS = 41,      /* Invalid group cipher */
	eSIR_MAC_INVALID_PAIRWISE_CIPHER_STATUS = 42,   /* Invalid pairwise cipher */
	eSIR_MAC_INVALID_AKMP_STATUS = 43,      /* Invalid AKMP */
	eSIR_MAC_UNSUPPORTED_RSN_IE_VERSION_STATUS = 44,        /* Unsupported RSN information element version */
	eSIR_MAC_INVALID_RSN_IE_CAPABILITIES_STATUS = 45,       /* Invalid RSN information element capabilities */
	eSIR_MAC_CIPHER_SUITE_REJECTED_STATUS = 46,     /* Cipher suite rejected because of security policy */
	eSIR_MAC_TS_NOT_CREATED_STATUS = 47,    /* The TS has not been created; however, the HC may be capable of creating a TS, in */
	/* response to a request, after the time indicated in the TS Delay element */
	eSIR_MAC_DL_NOT_ALLOWED_STATUS = 48,    /* Direct link is not allowed in the BSS by policy */
	eSIR_MAC_DEST_STA_NOT_KNOWN_STATUS = 49,        /* The Destination STA is not present within this BSS */
	eSIR_MAC_DEST_STA_NOT_QSTA_STATUS = 50, /* The Destination STA is not a QoS STA */
	eSIR_MAC_INVALID_LISTEN_INTERVAL_STATUS = 51,   /* Association denied because the ListenInterval is too large */

	eSIR_MAC_DSSS_CCK_RATE_MUST_SUPPORT_STATUS = 52,        /* FIXME: */
	eSIR_MAC_DSSS_CCK_RATE_NOT_SUPPORT_STATUS = 53,
	eSIR_MAC_PSMP_CONTROLLED_ACCESS_ONLY_STATUS = 54,
#ifdef FEATURE_WLAN_ESE
	eSIR_MAC_ESE_UNSPECIFIED_QOS_FAILURE_STATUS = 200,      /* ESE-Unspecified, QoS related failure in (Re)Assoc response frames */
	eSIR_MAC_ESE_TSPEC_REQ_REFUSED_STATUS = 201,    /* ESE-TSPEC request refused due to AP's policy configuration in AddTs Rsp, (Re)Assoc Rsp. */
	eSIR_MAC_ESE_ASSOC_DENIED_INSUFF_BW_STATUS = 202,       /* ESE-Assoc denied due to insufficient bandwidth to handle new TS in (Re)Assoc Rsp. */
	eSIR_MAC_ESE_INVALID_PARAMETERS_STATUS = 203,   /* ESE-Invalid parameters. (Re)Assoc request had one or more TSPEC parameters with */
	/* invalid values. */
#endif

} tSirMacStatusCodes;

/**
 * Reason Code (present in Deauthentication/Disassociation
 * Management frames) enum
 */
typedef enum eSirMacReasonCodes {
	eSIR_MAC_UNSPEC_FAILURE_REASON = 1,     /* Unspecified reason */
	eSIR_MAC_PREV_AUTH_NOT_VALID_REASON = 2,        /* Previous authentication no longer valid */
	eSIR_MAC_DEAUTH_LEAVING_BSS_REASON = 3, /* Deauthenticated because sending station is leaving (or has left) IBSS or ESS */
	eSIR_MAC_DISASSOC_DUE_TO_INACTIVITY_REASON = 4, /* Disassociated due to inactivity */
	eSIR_MAC_DISASSOC_DUE_TO_DISABILITY_REASON = 5, /* Disassociated because AP is unable to handle all currently associated stations */
	eSIR_MAC_CLASS2_FRAME_FROM_NON_AUTH_STA_REASON = 6,     /* Class 2 frame received from nonauthenticated station */
	eSIR_MAC_CLASS3_FRAME_FROM_NON_ASSOC_STA_REASON = 7,    /* Class 3 frame received from nonassociated station */
	eSIR_MAC_DISASSOC_LEAVING_BSS_REASON = 8,       /* Disassociated because sending station is leaving (or has left) BSS */
	eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON = 9,  /* Station requesting (re)association is not authenticated with responding station */
	eSIR_MAC_PWR_CAPABILITY_BAD_REASON = 10,        /* Disassociated because the information in the Power Capability element is unacceptable */
	eSIR_MAC_SPRTD_CHANNELS_BAD_REASON = 11,        /* Disassociated because the information in the Supported Channels element is unacceptable */
	/* reserved                                        12 */
	eSIR_MAC_INVALID_IE_REASON = 13,        /* Invalid information element, i.e., an information element defined in this standard for */
	/* which the content does not meet the specifications in Clause 7 */
	eSIR_MAC_MIC_FAILURE_REASON = 14,       /* Message integrity code (MIC) failure */
	eSIR_MAC_4WAY_HANDSHAKE_TIMEOUT_REASON = 15,    /* 4-Way Handshake timeout */
	eSIR_MAC_GR_KEY_UPDATE_TIMEOUT_REASON = 16,     /* Group Key Handshake timeout */
	eSIR_MAC_RSN_IE_MISMATCH_REASON = 17,   /* Information element in 4-Way Handshake different from (Re)Association Request/Probe */
	/* Response/Beacon frame */
	eSIR_MAC_INVALID_MC_CIPHER_REASON = 18, /* Invalid group cipher */
	eSIR_MAC_INVALID_UC_CIPHER_REASON = 19, /* Invalid pairwise cipher */
	eSIR_MAC_INVALID_AKMP_REASON = 20,      /* Invalid AKMP */
	eSIR_MAC_UNSUPPORTED_RSN_IE_VER_REASON = 21,    /* Unsupported RSN information element version */
	eSIR_MAC_INVALID_RSN_CAPABILITIES_REASON = 22,  /* Invalid RSN information element capabilities */
	eSIR_MAC_1X_AUTH_FAILURE_REASON = 23,   /* IEEE 802.1X authentication failed */
	eSIR_MAC_CIPHER_SUITE_REJECTED_REASON = 24,     /* Cipher suite rejected because of the security policy */
#ifdef FEATURE_WLAN_TDLS
	eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE = 25,   /* TDLS direct link teardown due to TDLS peer STA unreachable via the TDLS direct link */
	eSIR_MAC_TDLS_TEARDOWN_UNSPEC_REASON = 26,      /* TDLS direct link teardown for unspecified reason */
#endif
	/* reserved                                        27 - 30 */
#ifdef WLAN_FEATURE_11W
	eSIR_MAC_ROBUST_MGMT_FRAMES_POLICY_VIOLATION = 31,      /* Robust management frames policy violation */
#endif
	eSIR_MAC_QOS_UNSPECIFIED_REASON = 32,   /* Disassociated for unspecified, QoS-related reason */
	eSIR_MAC_QAP_NO_BANDWIDTH_REASON = 33,  /* Disassociated because QoS AP lacks sufficient bandwidth for this QoS STA */
	eSIR_MAC_XS_UNACKED_FRAMES_REASON = 34, /* Disassociated because excessive number of frames need to be acknowledged, but are not */
	/* acknowledged due to AP transmissions and/or poor channel conditions */
	eSIR_MAC_BAD_TXOP_USE_REASON = 35,      /* Disassociated because STA is transmitting outside the limits of its TXOPs */
	eSIR_MAC_PEER_STA_REQ_LEAVING_BSS_REASON = 36,  /* Requested from peer STA as the STA is leaving the BSS (or resetting) */
	eSIR_MAC_PEER_REJECT_MECHANISIM_REASON = 37,    /* Requested from peer STA as it does not want to use the mechanism */
	eSIR_MAC_MECHANISM_NOT_SETUP_REASON = 38,       /* Requested from peer STA as the STA received frames using the mechanism for which a */
	/* setup is required */
	eSIR_MAC_PEER_TIMEDOUT_REASON = 39,     /* Requested from peer STA due to timeout */
	eSIR_MAC_CIPHER_NOT_SUPPORTED_REASON = 45,      /* Peer STA does not support the requested cipher suite */
	eSIR_MAC_DISASSOC_DUE_TO_FTHANDOFF_REASON = 46, /* FT reason */
	/* reserved                                         47 - 65535. */
	eSIR_BEACON_MISSED = 65534,     /* We invented this to tell beacon missed case */
} tSirMacReasonCodes;

/* BA Initiator v/s Recipient */
typedef enum eBADirection {
	eBA_RECIPIENT,
	eBA_INITIATOR
} tBADirection;

/* A-MPDU/BA Enable/Disable in Tx/Rx direction */
typedef enum eBAEnable {
	eBA_DISABLE,
	eBA_ENABLE
} tBAEnable;

/* A-MPDU/BA Policy */
typedef enum eBAPolicy {
	eBA_UNCOMPRESSED,
	eBA_COMPRESSED
} tBAPolicy;

/* A-MPDU/BA Policy */
typedef enum eBAPolicyType {
	eBA_POLICY_DELAYED,
	eBA_POLICY_IMMEDIATE
} tBAPolicyType;

/* / Frame control field format (2 bytes) */
typedef struct sSirMacFrameCtl {

#ifndef ANI_LITTLE_BIT_ENDIAN

	uint8_t subType:4;
	uint8_t type:2;
	uint8_t protVer:2;

	uint8_t order:1;
	uint8_t wep:1;
	uint8_t moreData:1;
	uint8_t powerMgmt:1;
	uint8_t retry:1;
	uint8_t moreFrag:1;
	uint8_t fromDS:1;
	uint8_t toDS:1;

#else

	uint8_t protVer:2;
	uint8_t type:2;
	uint8_t subType:4;

	uint8_t toDS:1;
	uint8_t fromDS:1;
	uint8_t moreFrag:1;
	uint8_t retry:1;
	uint8_t powerMgmt:1;
	uint8_t moreData:1;
	uint8_t wep:1;
	uint8_t order:1;

#endif

} qdf_packed tSirMacFrameCtl, *tpSirMacFrameCtl;

/* / Sequence control field */
typedef struct sSirMacSeqCtl {

#ifndef ANI_LITTLE_BIT_ENDIAN

	uint8_t seqNumLo:4;
	uint8_t fragNum:4;

	uint8_t seqNumHi:8;

#else

	uint8_t fragNum:4;
	uint8_t seqNumLo:4;
	uint8_t seqNumHi:8;

#endif
} qdf_packed tSirMacSeqCtl, *tpSirMacSeqCtl;

/* / QoS control field */
typedef struct sSirMacQosCtl {

#ifndef ANI_LITTLE_BIT_ENDIAN

	uint8_t rsvd:1;
	uint8_t ackPolicy:2;
	uint8_t esop_txopUnit:1;
	uint8_t tid:4;

	uint8_t txop:8;

#else

	uint8_t tid:4;
	uint8_t esop_txopUnit:1;
	uint8_t ackPolicy:2;
	uint8_t rsvd:1;

	uint8_t txop:8;

#endif
} qdf_packed tSirMacQosCtl, *tpSirMacQosCtl;

/* / Length (in bytes) of MAC header in 3 address format */
#define SIR_MAC_HDR_LEN_3A    24

typedef uint8_t tSirMacAddr[ETH_ALEN];

/* / 3 address MAC data header format (24/26 bytes) */
typedef struct sSirMacDot3Hdr {
	tSirMacAddr da;
	tSirMacAddr sa;
	uint16_t length;
} qdf_packed tSirMacDot3Hdr, *tpSirMacDot3Hdr;

/* / 3 address MAC data header format (24/26 bytes) */
typedef struct sSirMacDataHdr3a {
	tSirMacFrameCtl fc;
	uint8_t durationLo;
	uint8_t durationHi;
	tSirMacAddr addr1;
	tSirMacAddr addr2;
	tSirMacAddr addr3;
	tSirMacSeqCtl seqControl;
	tSirMacQosCtl qosControl;
} qdf_packed tSirMacDataHdr3a, *tpSirMacDataHdr3a;

/* / Management header format */
typedef struct sSirMacMgmtHdr {
	tSirMacFrameCtl fc;
	uint8_t durationLo;
	uint8_t durationHi;
	tSirMacAddr da;
	tSirMacAddr sa;
	tSirMacAddr bssId;
	tSirMacSeqCtl seqControl;
} qdf_packed tSirMacMgmtHdr, *tpSirMacMgmtHdr;

/* / ERP information field */
typedef struct sSirMacErpInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t reserved:5;
	uint8_t barkerPreambleMode:1;
	uint8_t useProtection:1;
	uint8_t nonErpPresent:1;
#else
	uint8_t nonErpPresent:1;
	uint8_t useProtection:1;
	uint8_t barkerPreambleMode:1;
	uint8_t reserved:5;
#endif
} qdf_packed tSirMacErpInfo, *tpSirMacErpInfo;

/* / Capability information field */
typedef struct sSirMacCapabilityInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t immediateBA:1;
	uint16_t delayedBA:1;
	uint16_t dsssOfdm:1;
	uint16_t rrm:1;
	uint16_t apsd:1;
	uint16_t shortSlotTime:1;
	uint16_t qos:1;
	uint16_t spectrumMgt:1;
	uint16_t channelAgility:1;
	uint16_t pbcc:1;
	uint16_t shortPreamble:1;
	uint16_t privacy:1;
	uint16_t cfPollReq:1;
	uint16_t cfPollable:1;
	uint16_t ibss:1;
	uint16_t ess:1;
#else
	uint16_t ess:1;
	uint16_t ibss:1;
	uint16_t cfPollable:1;
	uint16_t cfPollReq:1;
	uint16_t privacy:1;
	uint16_t shortPreamble:1;
	uint16_t pbcc:1;
	uint16_t channelAgility:1;
	uint16_t spectrumMgt:1;
	uint16_t qos:1;
	uint16_t shortSlotTime:1;
	uint16_t apsd:1;
	uint16_t rrm:1;
	uint16_t dsssOfdm:1;
	uint16_t delayedBA:1;
	uint16_t immediateBA:1;
#endif
} qdf_packed tSirMacCapabilityInfo, *tpSirMacCapabilityInfo;

typedef struct sSirMacCfParamSet {
	uint8_t cfpCount;
	uint8_t cfpPeriod;
	uint16_t cfpMaxDuration;
	uint16_t cfpDurRemaining;
} qdf_packed tSirMacCfParamSet;

typedef struct sSirMacTim {
	uint8_t dtimCount;
	uint8_t dtimPeriod;
	uint8_t bitmapControl;
	uint8_t bitmapLength;
	uint8_t bitmap[251];
} qdf_packed tSirMacTim;

/* 12 Bytes long because this structure can be used to represent rate */
/* and extended rate set IEs */
/* The parser assume this to be at least 12 */
typedef struct sSirMacRateSet {
	uint8_t numRates;
	uint8_t rate[SIR_MAC_RATESET_EID_MAX];
} qdf_packed tSirMacRateSet;

/** struct merged_mac_rate_set - merged mac rate set
 * @num_rates: num of rates
 * @rate: rate list
 */
struct merged_mac_rate_set {
	uint8_t num_rates;
	uint8_t rate[2 * SIR_MAC_RATESET_EID_MAX];
};

typedef struct sSirMacSSid {
	uint8_t length;
	uint8_t ssId[SIR_MAC_MAX_SSID_LENGTH];
} qdf_packed tSirMacSSid;

typedef struct sSirMacWpaInfo {
	uint8_t length;
	uint8_t info[SIR_MAC_MAX_IE_LENGTH];
} qdf_packed tSirMacWpaInfo, *tpSirMacWpaInfo,
tSirMacRsnInfo, *tpSirMacRsnInfo;
typedef struct sSirMacWapiInfo {
	uint8_t length;
	uint8_t info[SIR_MAC_MAX_IE_LENGTH];
} qdf_packed tSirMacWapiInfo, *tpSirMacWapiInfo,
tSirMacWapiInfo, *tpSirMacWapiInfo;

typedef struct sSirMacFHParamSet {
	uint16_t dwellTime;
	uint8_t hopSet;
	uint8_t hopPattern;
	uint8_t hopIndex;
} tSirMacFHParamSet, *tpSirMacFHParamSet;

typedef struct sSirMacIBSSParams {
	uint16_t atim;
} tSirMacIBSSParams, *tpSirMacIBSSParams;

typedef struct sSirMacRRMEnabledCap {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t reserved:6;
	uint8_t AntennaInformation:1;
	uint8_t BSSAvailAdmission:1;
	uint8_t BssAvgAccessDelay:1;
	uint8_t RSNIMeasurement:1;
	uint8_t RCPIMeasurement:1;
	uint8_t NeighborTSFOffset:1;
	uint8_t MeasurementPilotEnabled:1;
	uint8_t MeasurementPilot:3;
	uint8_t nonOperatinChanMax:3;
	uint8_t operatingChanMax:3;
	uint8_t RRMMIBEnabled:1;
	uint8_t APChanReport:1;
	uint8_t triggeredTCM:1;
	uint8_t TCMCapability:1;
	uint8_t LCIAzimuth:1;
	uint8_t LCIMeasurement:1;
	uint8_t statistics:1;
	uint8_t NoiseHistogram:1;
	uint8_t ChannelLoad:1;
	uint8_t FrameMeasurement:1;
	uint8_t BeaconRepCond:1;
	uint8_t BeaconTable:1;
	uint8_t BeaconActive:1;
	uint8_t BeaconPassive:1;
	uint8_t repeated:1;
	uint8_t parallel:1;
	uint8_t NeighborRpt:1;
	uint8_t LinkMeasurement:1;
	uint8_t present;
#else
	uint8_t present;
	uint8_t LinkMeasurement:1;
	uint8_t NeighborRpt:1;
	uint8_t parallel:1;
	uint8_t repeated:1;
	uint8_t BeaconPassive:1;
	uint8_t BeaconActive:1;
	uint8_t BeaconTable:1;
	uint8_t BeaconRepCond:1;
	uint8_t FrameMeasurement:1;
	uint8_t ChannelLoad:1;
	uint8_t NoiseHistogram:1;
	uint8_t statistics:1;
	uint8_t LCIMeasurement:1;
	uint8_t LCIAzimuth:1;
	uint8_t TCMCapability:1;
	uint8_t triggeredTCM:1;
	uint8_t APChanReport:1;
	uint8_t RRMMIBEnabled:1;
	uint8_t operatingChanMax:3;
	uint8_t nonOperatinChanMax:3;
	uint8_t MeasurementPilot:3;
	uint8_t MeasurementPilotEnabled:1;
	uint8_t NeighborTSFOffset:1;
	uint8_t RCPIMeasurement:1;
	uint8_t RSNIMeasurement:1;
	uint8_t BssAvgAccessDelay:1;
	uint8_t BSSAvailAdmission:1;
	uint8_t AntennaInformation:1;
	uint8_t reserved:6;
#endif
} tSirMacRRMEnabledCap, *tpSirMacRRMEnabledCap;

/* ----------------
 *  EDCA Profiles
 * ---------------
 */

#define EDCA_AC_BE 0
#define EDCA_AC_BK 1
#define EDCA_AC_VI 2
#define EDCA_AC_VO 3
#define AC_MGMT_LO 4
#define AC_MGMT_HI 5
#define MAX_NUM_AC 4

/* access categories */
#define SIR_MAC_EDCAACI_BESTEFFORT  (EDCA_AC_BE)
#define SIR_MAC_EDCAACI_BACKGROUND  (EDCA_AC_BK)
#define SIR_MAC_EDCAACI_VIDEO       (EDCA_AC_VI)
#define SIR_MAC_EDCAACI_VOICE       (EDCA_AC_VO)

/* access category record */
typedef struct sSirMacAciAifsn {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t rsvd:1;
	uint8_t aci:2;
	uint8_t acm:1;
	uint8_t aifsn:4;
#else
	uint8_t aifsn:4;
	uint8_t acm:1;
	uint8_t aci:2;
	uint8_t rsvd:1;
#endif
} qdf_packed tSirMacAciAifsn;

/* contention window size */
typedef struct sSirMacCW {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t max:4;
	uint8_t min:4;
#else
	uint8_t min:4;
	uint8_t max:4;
#endif
} qdf_packed tSirMacCW;

typedef struct sSirMacEdcaParamRecord {
	tSirMacAciAifsn aci;
	tSirMacCW cw;
	uint16_t txoplimit;
	uint8_t no_ack;
} qdf_packed tSirMacEdcaParamRecord;

typedef struct sSirMacQosInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t uapsd:1;
	uint8_t txopreq:1;
	uint8_t qreq:1;
	uint8_t qack:1;
	uint8_t count:4;
#else
	uint8_t count:4;
	uint8_t qack:1;
	uint8_t qreq:1;
	uint8_t txopreq:1;
	uint8_t uapsd:1;
#endif
} qdf_packed tSirMacQosInfo;

typedef struct sSirMacQosInfoStation {
#ifdef ANI_LITTLE_BIT_ENDIAN
	uint8_t acvo_uapsd:1;
	uint8_t acvi_uapsd:1;
	uint8_t acbk_uapsd:1;
	uint8_t acbe_uapsd:1;
	uint8_t qack:1;
	uint8_t maxSpLen:2;
	uint8_t moreDataAck:1;
#else
	uint8_t moreDataAck:1;
	uint8_t maxSpLen:2;
	uint8_t qack:1;
	uint8_t acbe_uapsd:1;
	uint8_t acbk_uapsd:1;
	uint8_t acvi_uapsd:1;
	uint8_t acvo_uapsd:1;
#endif
} qdf_packed tSirMacQosInfoStation, *tpSirMacQosInfoStation;

typedef struct sSirMacEdcaParamSetIE {
	uint8_t type;
	uint8_t length;
	tSirMacQosInfo qosInfo;
	uint8_t rsvd;
	tSirMacEdcaParamRecord acbe;    /* best effort */
	tSirMacEdcaParamRecord acbk;    /* background */
	tSirMacEdcaParamRecord acvi;    /* video */
	tSirMacEdcaParamRecord acvo;    /* voice */
} qdf_packed tSirMacEdcaParamSetIE;

typedef struct sSirMacQoSParams {
	uint8_t count;
	uint16_t limit;
	uint8_t CWmin[8];
	uint8_t AIFS[8];
} qdf_packed tSirMacQoSParams;

/* ts info direction field can take any of these values */
#define SIR_MAC_DIRECTION_UPLINK    0
#define SIR_MAC_DIRECTION_DNLINK    1
#define SIR_MAC_DIRECTION_DIRECT    2
#define SIR_MAC_DIRECTION_BIDIR     3

/* access policy */
/* reserved                         0 */
#define SIR_MAC_ACCESSPOLICY_EDCA   1
#define SIR_MAC_ACCESSPOLICY_HCCA   2
#define SIR_MAC_ACCESSPOLICY_BOTH   3

typedef struct sSirMacTSInfoTfc {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t burstSizeDefn:1;
	uint8_t reserved:7;
#else
	uint8_t reserved:7;
	uint8_t burstSizeDefn:1;
#endif

#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t ackPolicy:2;
	uint16_t userPrio:3;
	uint16_t psb:1;
	uint16_t aggregation:1;
	uint16_t accessPolicy:2;
	uint16_t direction:2;
	uint16_t tsid:4;
	uint16_t trafficType:1;
#else
	uint16_t trafficType:1;
	uint16_t tsid:4;
	uint16_t direction:2;
	uint16_t accessPolicy:2;
	uint16_t aggregation:1;
	uint16_t psb:1;
	uint16_t userPrio:3;
	uint16_t ackPolicy:2;
#endif
} qdf_packed tSirMacTSInfoTfc;

typedef struct sSirMacTSInfoSch {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t rsvd:7;
	uint8_t schedule:1;
#else
	uint8_t schedule:1;
	uint8_t rsvd:7;
#endif
} qdf_packed tSirMacTSInfoSch;

typedef struct sSirMacTSInfo {
	tSirMacTSInfoTfc traffic;
	tSirMacTSInfoSch schedule;
} qdf_packed tSirMacTSInfo;

typedef struct sSirMacTspecIE {
	uint8_t type;
	uint8_t length;
	tSirMacTSInfo tsinfo;
	uint16_t nomMsduSz;
	uint16_t maxMsduSz;
	uint32_t minSvcInterval;
	uint32_t maxSvcInterval;
	uint32_t inactInterval;
	uint32_t suspendInterval;
	uint32_t svcStartTime;
	uint32_t minDataRate;
	uint32_t meanDataRate;
	uint32_t peakDataRate;
	uint32_t maxBurstSz;
	uint32_t delayBound;
	uint32_t minPhyRate;
	uint16_t surplusBw;
	uint16_t mediumTime;
} qdf_packed tSirMacTspecIE;

/* frame classifier types */
#define SIR_MAC_TCLASTYPE_ETHERNET 0
#define SIR_MAC_TCLASTYPE_TCPUDPIP 1
#define SIR_MAC_TCLASTYPE_8021DQ   2
/* reserved                        3-255 */

typedef struct sSirMacTclasParamEthernet {
	tSirMacAddr srcAddr;
	tSirMacAddr dstAddr;
	uint16_t type;
} qdf_packed tSirMacTclasParamEthernet;

typedef struct sSirMacTclasParamIPv4 {
	uint8_t version;
	uint8_t srcIpAddr[4];
	uint8_t dstIpAddr[4];
	uint16_t srcPort;
	uint16_t dstPort;
	uint8_t dscp;
	uint8_t protocol;
	uint8_t rsvd;
} qdf_packed tSirMacTclasParamIPv4;

#define SIR_MAC_TCLAS_IPV4  4
#define SIR_MAC_TCLAS_IPV6  6

typedef struct sSirMacTclasParamIPv6 {
	uint8_t version;
	uint8_t srcIpAddr[16];
	uint8_t dstIpAddr[16];
	uint16_t srcPort;
	uint16_t dstPort;
	uint8_t flowLabel[3];
} qdf_packed tSirMacTclasParamIPv6;

typedef struct sSirMacTclasParam8021dq {
	uint16_t tag;
} qdf_packed tSirMacTclasParam8021dq;

typedef struct sSirMacTclasIE {
	uint8_t type;
	uint8_t length;
	uint8_t userPrio;
	uint8_t classifierType;
	uint8_t classifierMask;
} qdf_packed tSirMacTclasIE;

typedef struct sSirMacTsDelayIE {
	uint8_t type;
	uint8_t length;
	uint32_t delay;
} qdf_packed tSirMacTsDelayIE;

typedef struct sSirMacScheduleInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t rsvd:9;
	uint16_t direction:2;
	uint16_t tsid:4;
	uint16_t aggregation:1;
#else
	uint16_t aggregation:1;
	uint16_t tsid:4;
	uint16_t direction:2;
	uint16_t rsvd:9;
#endif
} qdf_packed tSirMacScheduleInfo;

typedef struct sSirMacScheduleIE {
	uint8_t type;
	uint8_t length;
	tSirMacScheduleInfo info;
	uint32_t svcStartTime;
	uint32_t svcInterval;
	uint16_t maxSvcDuration;
	uint16_t specInterval;
} qdf_packed tSirMacScheduleIE;

typedef struct sSirMacQosCapabilityIE {
	uint8_t type;
	uint8_t length;
	tSirMacQosInfo qosInfo;
} qdf_packed tSirMacQosCapabilityIE;

typedef struct sSirMacQosCapabilityStaIE {
	uint8_t type;
	uint8_t length;
	tSirMacQosInfoStation qosInfo;
} qdf_packed tSirMacQosCapabilityStaIE;

typedef uint32_t tSirMacTimeStamp[2];

typedef uint16_t tSirMacBeaconInterval;

typedef uint16_t tSirMacListenInterval;

typedef uint8_t tSirMacChanNum;

/* IE definitions */
typedef struct sSirMacSSidIE {
	uint8_t type;
	tSirMacSSid ssId;
} qdf_packed tSirMacSSidIE;

typedef struct sSirMacRateSetIE {
	uint8_t type;
	tSirMacRateSet supportedRateSet;
} qdf_packed tSirMacRateSetIE;

typedef struct sSirMacDsParamSetIE {
	uint8_t type;
	uint8_t length;
	tSirMacChanNum channelNumber;
} qdf_packed tSirMacDsParamSetIE;

typedef struct sSirMacCfParamSetIE {
	uint8_t type;
	uint8_t length;
	tSirMacCfParamSet cfParams;
} qdf_packed tSirMacCfParamSetIE;

typedef struct sSirMacChanInfo {
	tSirMacChanNum firstChanNum;
	uint8_t numChannels;
	int8_t maxTxPower;
} qdf_packed tSirMacChanInfo;

typedef struct sSirMacNonErpPresentIE {
	uint8_t type;
	uint8_t length;
	uint8_t erp;
} qdf_packed tSirMacNonErpPresentIE;

typedef struct sSirMacPowerCapabilityIE {
	uint8_t type;
	uint8_t length;
	uint8_t minTxPower;
	uint8_t maxTxPower;
} tSirMacPowerCapabilityIE;

typedef struct sSirMacSupportedChannelIE {
	uint8_t type;
	uint8_t length;
	uint8_t supportedChannels[96];
} tSirMacSupportedChannelIE;

typedef struct sSirMacMeasReqField {
	uint8_t channelNumber;
	uint8_t measStartTime[8];
	uint16_t measDuration;
} tSirMacMeasReqField, *tpSirMacMeasReqField;

typedef struct sSirMacMeasReqIE {
	uint8_t type;
	uint8_t length;
	uint8_t measToken;
	uint8_t measReqMode;
	uint8_t measType;
	tSirMacMeasReqField measReqField;
} tSirMacMeasReqIE, *tpSirMacMeasReqIE;

#define SIR_MAC_MAX_SUPP_RATES            32

#define SIR_MAC_MAX_SUPP_CHANNELS            100
#define SIR_MAC_MAX_EXTN_CAP               8

/* VHT Capabilities Info */
typedef struct sSirMacVHTCapabilityInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint32_t reserved1:2;
	uint32_t txAntPattern:1;
	uint32_t rxAntPattern:1;
	uint32_t vhtLinkAdaptCap:2;
	uint32_t maxAMPDULenExp:3;
	uint32_t htcVHTCap:1;
	uint32_t vhtTXOPPS:1;
	uint32_t muBeamformeeCap:1;
	uint32_t muBeamformerCap:1;
	uint32_t numSoundingDim:3;
	uint32_t csnofBeamformerAntSup:3;
	uint32_t suBeamformeeCap:1;
	uint32_t suBeamFormerCap:1;
	uint32_t rxSTBC:3;
	uint32_t txSTBC:1;
	uint32_t shortGI160and80plus80MHz:1;
	uint32_t shortGI80MHz:1;
	uint32_t ldpcCodingCap:1;
	uint32_t supportedChannelWidthSet:2;
	uint32_t maxMPDULen:2;
#else
	uint32_t maxMPDULen:2;
	uint32_t supportedChannelWidthSet:2;
	uint32_t ldpcCodingCap:1;
	uint32_t shortGI80MHz:1;
	uint32_t shortGI160and80plus80MHz:1;
	uint32_t txSTBC:1;
	uint32_t rxSTBC:3;
	uint32_t suBeamFormerCap:1;
	uint32_t suBeamformeeCap:1;
	uint32_t csnofBeamformerAntSup:3;
	uint32_t numSoundingDim:3;
	uint32_t muBeamformerCap:1;
	uint32_t muBeamformeeCap:1;
	uint32_t vhtTXOPPS:1;
	uint32_t htcVHTCap:1;
	uint32_t maxAMPDULenExp:3;
	uint32_t vhtLinkAdaptCap:2;
	uint32_t rxAntPattern:1;
	uint32_t txAntPattern:1;
	uint32_t reserved1:2;
#endif
} qdf_packed tSirMacVHTCapabilityInfo;

typedef struct sSirMacVHTTxSupDataRateInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t reserved:3;
	uint16_t txSupDataRate:13;
#else
	uint16_t txSupDataRate:13;
	uint16_t reserved:3;
#endif
} qdf_packed tSirMacVHTTxSupDataRateInfo;

typedef struct sSirMacVHTRxSupDataRateInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t reserved:3;
	uint16_t rxSupDataRate:13;
#else
	uint16_t rxSupDataRate:13;
	uint16_t reserved:3;
#endif
} qdf_packed tSirMacVHTRxSupDataRateInfo;

/**
 * struct sSirVhtMcsInfo - VHT MCS information
 * @rx_mcs_map: RX MCS map 2 bits for each stream, total 8 streams
 * @rx_highest: Indicates highest long GI VHT PPDU data rate
 *      STA can receive. Rate expressed in units of 1 Mbps.
 *      If this field is 0 this value should not be used to
 *      consider the highest RX data rate supported.
 * @tx_mcs_map: TX MCS map 2 bits for each stream, total 8 streams
 * @tx_highest: Indicates highest long GI VHT PPDU data rate
 *      STA can transmit. Rate expressed in units of 1 Mbps.
 *      If this field is 0 this value should not be used to
 *      consider the highest TX data rate supported.
 */
typedef struct sSirVhtMcsInfo {
	uint16_t rxMcsMap;
	uint16_t rxHighest;
	uint16_t txMcsMap;
	uint16_t txHighest;
} tSirVhtMcsInfo;

/**
 * struct sSirVHtCap - VHT capabilities
 *
 * This structure is the "VHT capabilities element" as
 * described in 802.11ac D3.0 8.4.2.160
 * @vht_cap_info: VHT capability info
 * @supp_mcs: VHT MCS supported rates
 */
typedef struct sSirVHtCap {
	uint32_t vhtCapInfo;
	tSirVhtMcsInfo suppMcs;
} tSirVHTCap;

/**
 * struct sSirHtCap - HT capabilities
 *
 * This structure refers to "HT capabilities element" as
 * described in 802.11n draft section 7.3.2.52
 */

typedef struct sSirHtCap {
	uint16_t capInfo;
	uint8_t ampduParamsInfo;
	uint8_t suppMcsSet[16];
	uint16_t extendedHtCapInfo;
	uint32_t txBFCapInfo;
	uint8_t antennaSelectionInfo;
} tSirHTCap;

/* HT Cap and HT IE Size defines */
#define HT_CAPABILITY_IE_SIZE                       28
#define HT_INFO_IE_SIZE                                          24

/* */
/* Determines the current operating mode of the 802.11n STA */
/* */

typedef enum eSirMacHTOperatingMode {
	eSIR_HT_OP_MODE_PURE,   /* No Protection */
	eSIR_HT_OP_MODE_OVERLAP_LEGACY, /* Overlap Legacy device present, protection is optional */
	eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT,     /* No legacy device, but 20 MHz HT present */
	eSIR_HT_OP_MODE_MIXED   /* Protetion is required */
} tSirMacHTOperatingMode;

/* Spatial Multiplexing(SM) Power Save mode */
typedef enum eSirMacHTMIMOPowerSaveState {
	eSIR_HT_MIMO_PS_STATIC = 0,     /* Static SM Power Save mode */
	eSIR_HT_MIMO_PS_DYNAMIC = 1,    /* Dynamic SM Power Save mode */
	eSIR_HT_MIMO_PS_NA = 2, /* reserved */
	eSIR_HT_MIMO_PS_NO_LIMIT = 3    /* SM Power Save disabled */
} tSirMacHTMIMOPowerSaveState;

typedef enum eSirMacHTChannelWidth {
	eHT_CHANNEL_WIDTH_20MHZ = 0,
	eHT_CHANNEL_WIDTH_40MHZ = 1,
	eHT_CHANNEL_WIDTH_80MHZ = 2,
	eHT_CHANNEL_WIDTH_160MHZ = 3,
	eHT_CHANNEL_WIDTH_80P80MHZ = 4,
	eHT_MAX_CHANNEL_WIDTH
} tSirMacHTChannelWidth;

typedef enum eSirMacHTChannelType {
	eHT_CHAN_NO_HT = 0,
	eHT_CHAN_HT20 = 1,
	eHT_CHAN_HT40MINUS = 2,
	eHT_CHAN_HT40PLUS = 3
} tSirMacHTChannelType;

/* Packet struct for HT capability */
typedef struct sHtCaps {
	uint16_t advCodingCap:1;
	uint16_t supportedChannelWidthSet:1;
	uint16_t mimoPowerSave:2;
	uint16_t greenField:1;
	uint16_t shortGI20MHz:1;
	uint16_t shortGI40MHz:1;
	uint16_t txSTBC:1;
	uint16_t rxSTBC:2;
	uint16_t delayedBA:1;
	uint16_t maximalAMSDUsize:1;
	uint16_t dsssCckMode40MHz:1;
	uint16_t psmp:1;
	uint16_t stbcControlFrame:1;
	uint16_t lsigTXOPProtection:1;
	uint8_t maxRxAMPDUFactor:2;
	uint8_t mpduDensity:3;
	uint8_t reserved1:3;
	uint8_t supportedMCSSet[16];
	uint16_t pco:1;
	uint16_t transitionTime:2;
	uint16_t reserved2:5;
	uint16_t mcsFeedback:2;
	uint16_t reserved3:6;
	uint32_t txBF:1;
	uint32_t rxStaggeredSounding:1;
	uint32_t txStaggeredSounding:1;
	uint32_t rxZLF:1;
	uint32_t txZLF:1;
	uint32_t implicitTxBF:1;
	uint32_t calibration:2;
	uint32_t explicitCSITxBF:1;
	uint32_t explicitUncompressedSteeringMatrix:1;
	uint32_t explicitBFCSIFeedback:3;
	uint32_t explicitUncompressedSteeringMatrixFeedback:3;
	uint32_t explicitCompressedSteeringMatrixFeedback:3;
	uint32_t csiNumBFAntennae:2;
	uint32_t uncompressedSteeringMatrixBFAntennae:2;
	uint32_t compressedSteeringMatrixBFAntennae:2;
	uint32_t reserved4:7;
	uint8_t antennaSelection:1;
	uint8_t explicitCSIFeedbackTx:1;
	uint8_t antennaIndicesFeedbackTx:1;
	uint8_t explicitCSIFeedback:1;
	uint8_t antennaIndicesFeedback:1;
	uint8_t rxAS:1;
	uint8_t txSoundingPPDUs:1;
	uint8_t reserved5:1;

} qdf_packed tHtCaps;

/* During 11h channel switch, the AP can indicate if the
 * STA needs to stop the transmission or continue until the
 * channel-switch.
 * eSIR_CHANSW_MODE_NORMAL - STA can continue transmission
 * eSIR_CHANSW_MODE_SILENT - STA should stop transmission
 */
typedef enum eSirMacChanSwMode {
	eSIR_CHANSW_MODE_NORMAL = 0,
	eSIR_CHANSW_MODE_SILENT = 1
} tSirMacChanSwitchMode;

typedef struct _BarControl {

#ifndef ANI_BIG_BYTE_ENDIAN

	uint16_t barAckPolicy:1;
	uint16_t multiTID:1;
	uint16_t bitMap:1;
	uint16_t rsvd:9;
	uint16_t numTID:4;

#else
	uint16_t numTID:4;
	uint16_t rsvd:9;
	uint16_t bitMap:1;
	uint16_t multiTID:1;
	uint16_t barAckPolicy:1;

#endif

} qdf_packed barCtrlType;

typedef struct _BARFrmStruct {
	tSirMacFrameCtl fc;
	uint16_t duration;
	tSirMacAddr rxAddr;
	tSirMacAddr txAddr;
	barCtrlType barControl;
	tSirMacSeqCtl ssnCtrl;
} qdf_packed BARFrmType;

/* Supported MCS set */
#define SIZE_OF_SUPPORTED_MCS_SET                          16
#define SIZE_OF_BASIC_MCS_SET                              16
#define VALID_MCS_SIZE                                     77   /* 0-76 */
#define MCS_RX_HIGHEST_SUPPORTED_RATE_BYTE_OFFSET          10
#define VALID_MAX_MCS_INDEX                                8

/* */
/* The following enums will be used to get the "current" HT Capabilities of */
/* the local STA in a generic fashion. In other words, the following enums */
/* identify the HT capabilities that can be queried or set. */
/* */
typedef enum eHTCapability {
	eHT_LSIG_TXOP_PROTECTION,
	eHT_STBC_CONTROL_FRAME,
	eHT_PSMP,
	eHT_DSSS_CCK_MODE_40MHZ,
	eHT_MAX_AMSDU_LENGTH,
	eHT_MAX_AMSDU_NUM,
	eHT_RX_STBC,
	eHT_TX_STBC,
	eHT_SHORT_GI_40MHZ,
	eHT_SHORT_GI_20MHZ,
	eHT_GREENFIELD,
	eHT_MIMO_POWER_SAVE,
	eHT_SUPPORTED_CHANNEL_WIDTH_SET,
	eHT_ADVANCED_CODING,
	eHT_MAX_RX_AMPDU_FACTOR,
	eHT_MPDU_DENSITY,
	eHT_PCO,
	eHT_TRANSITION_TIME,
	eHT_MCS_FEEDBACK,
	eHT_TX_BEAMFORMING,
	eHT_ANTENNA_SELECTION,
	/* The following come under Additional HT Capabilities */
	eHT_SI_GRANULARITY,
	eHT_CONTROLLED_ACCESS,
	eHT_RIFS_MODE,
	eHT_RECOMMENDED_TX_WIDTH_SET,
	eHT_EXTENSION_CHANNEL_OFFSET,
	eHT_OP_MODE,
	eHT_BASIC_STBC_MCS,
	eHT_DUAL_CTS_PROTECTION,
	eHT_LSIG_TXOP_PROTECTION_FULL_SUPPORT,
	eHT_PCO_ACTIVE,
	eHT_PCO_PHASE
} tHTCapability;

/* HT Capabilities Info */
typedef struct sSirMacHTCapabilityInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t lsigTXOPProtection:1;  /* Dynamic state */
	uint16_t stbcControlFrame:1;    /* Static via CFG */
	uint16_t psmp:1;        /* Static via CFG */
	uint16_t dsssCckMode40MHz:1;    /* Static via CFG */
	uint16_t maximalAMSDUsize:1;    /* Static via CFG */
	uint16_t delayedBA:1;   /* Static via CFG */
	uint16_t rxSTBC:2;      /* Static via CFG */
	uint16_t txSTBC:1;      /* Static via CFG */
	uint16_t shortGI40MHz:1;        /* Static via CFG */
	uint16_t shortGI20MHz:1;        /* Static via CFG */
	uint16_t greenField:1;  /* Static via CFG */
	uint16_t mimoPowerSave:2;       /* Dynamic state */
	uint16_t supportedChannelWidthSet:1;    /* Static via CFG */
	uint16_t advCodingCap:1;        /* Static via CFG */
#else
	uint16_t advCodingCap:1;
	uint16_t supportedChannelWidthSet:1;
	uint16_t mimoPowerSave:2;
	uint16_t greenField:1;
	uint16_t shortGI20MHz:1;
	uint16_t shortGI40MHz:1;
	uint16_t txSTBC:1;
	uint16_t rxSTBC:2;
	uint16_t delayedBA:1;
	uint16_t maximalAMSDUsize:1;
	uint16_t dsssCckMode40MHz:1;
	uint16_t psmp:1;
	uint16_t stbcControlFrame:1;
	uint16_t lsigTXOPProtection:1;
#endif
} qdf_packed tSirMacHTCapabilityInfo;

/* HT Parameters Info */
typedef struct sSirMacHTParametersInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t reserved:3;
	uint8_t mpduDensity:3;  /* Dynamic state */
	uint8_t maxRxAMPDUFactor:2;     /* Dynamic state */
#else
	uint8_t maxRxAMPDUFactor:2;
	uint8_t mpduDensity:3;
	uint8_t reserved:3;
#endif
} qdf_packed tSirMacHTParametersInfo;

/* Extended HT Capabilities Info */
typedef struct sSirMacExtendedHTCapabilityInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t reserved2:6;
	uint16_t mcsFeedback:2; /* Static via CFG */
	uint16_t reserved1:5;
	uint16_t transitionTime:2;      /* Static via CFG */
	uint16_t pco:1;         /* Static via CFG */
#else
	uint16_t pco:1;
	uint16_t transitionTime:2;
	uint16_t reserved1:5;
	uint16_t mcsFeedback:2;
	uint16_t reserved2:6;
#endif
} qdf_packed tSirMacExtendedHTCapabilityInfo;

/* IEEE 802.11n/D7.0 - 7.3.2.57.4 */
/* Part of the "supported MCS set field" */
typedef struct sSirMacRxHighestSupportRate {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t reserved:6;
	uint16_t rate:10;
#else
	uint16_t rate:10;
	uint16_t reserved:6;
#endif
} qdf_packed tSirMacRxHighestSupportRate, *tpSirMacRxHighestSupportRate;

/* Transmit Beam Forming Capabilities Info */
typedef struct sSirMacTxBFCapabilityInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint32_t reserved:7;
	uint32_t compressedSteeringMatrixBFAntennae:2;  /* Static via CFG */
	/* Static via CFG */
	uint32_t uncompressedSteeringMatrixBFAntennae:2;
	uint32_t csiNumBFAntennae:2;    /* Static via CFG */
	/* Static via CFG */
	uint32_t explicitCompressedSteeringMatrixFeedback:3;
	/* Static via CFG */
	uint32_t explicitUncompressedSteeringMatrixFeedback:3;
	uint32_t explicitBFCSIFeedback:3;       /* Static via CFG */
	uint32_t explicitUncompressedSteeringMatrix:1;  /* Static via CFG */
	uint32_t explicitCSITxBF:1;     /* Static via CFG */
	uint32_t calibration:2; /* Static via CFG */
	uint32_t implicitTxBF:1;        /* Static via CFG */
	uint32_t txZLF:1;       /* Static via CFG */
	uint32_t rxZLF:1;       /* Static via CFG */
	uint32_t txStaggeredSounding:1; /* Static via CFG */
	uint32_t rxStaggeredSounding:1; /* Static via CFG */
	uint32_t txBF:1;        /* Static via CFG */
#else
	uint32_t txBF:1;
	uint32_t rxStaggeredSounding:1;
	uint32_t txStaggeredSounding:1;
	uint32_t rxZLF:1;
	uint32_t txZLF:1;
	uint32_t implicitTxBF:1;
	uint32_t calibration:2;
	uint32_t explicitCSITxBF:1;
	uint32_t explicitUncompressedSteeringMatrix:1;
	uint32_t explicitBFCSIFeedback:3;
	uint32_t explicitUncompressedSteeringMatrixFeedback:3;
	uint32_t explicitCompressedSteeringMatrixFeedback:3;
	uint32_t csiNumBFAntennae:2;
	uint32_t uncompressedSteeringMatrixBFAntennae:2;
	uint32_t compressedSteeringMatrixBFAntennae:2;
	uint32_t reserved:7;
#endif
} qdf_packed tSirMacTxBFCapabilityInfo;

/* Antenna Selection Capability Info */
typedef struct sSirMacASCapabilityInfo {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t reserved2:1;
	uint8_t txSoundingPPDUs:1;      /* Static via CFG */
	uint8_t rxAS:1;         /* Static via CFG */
	uint8_t antennaIndicesFeedback:1;       /* Static via CFG */
	uint8_t explicitCSIFeedback:1;  /* Static via CFG */
	uint8_t antennaIndicesFeedbackTx:1;     /* Static via CFG */
	uint8_t explicitCSIFeedbackTx:1;        /* Static via CFG */
	uint8_t antennaSelection:1;     /* Static via CFG */
#else
	uint8_t antennaSelection:1;
	uint8_t explicitCSIFeedbackTx:1;
	uint8_t antennaIndicesFeedbackTx:1;
	uint8_t explicitCSIFeedback:1;
	uint8_t antennaIndicesFeedback:1;
	uint8_t rxAS:1;
	uint8_t txSoundingPPDUs:1;
	uint8_t reserved2:1;
#endif
} qdf_packed tSirMacASCapabilityInfo;

/* Additional HT IE Field1 */
typedef struct sSirMacHTInfoField1 {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t serviceIntervalGranularity:3;   /* Dynamic state */
	uint8_t controlledAccessOnly:1; /* Static via CFG */
	uint8_t rifsMode:1;     /* Dynamic state */
	uint8_t recommendedTxWidthSet:1;        /* Dynamic state */
	uint8_t secondaryChannelOffset:2;       /* Dynamic state */
#else
	uint8_t secondaryChannelOffset:2;
	uint8_t recommendedTxWidthSet:1;
	uint8_t rifsMode:1;
	uint8_t controlledAccessOnly:1;
	uint8_t serviceIntervalGranularity:3;
#endif
} qdf_packed tSirMacHTInfoField1;

/* Additional HT IE Field2 */
typedef struct sSirMacHTInfoField2 {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t reserved:11;
	uint16_t obssNonHTStaPresent:1; /*added for Obss  */
	uint16_t transmitBurstLimit:1;
	uint16_t nonGFDevicesPresent:1;
	uint16_t opMode:2;      /* Dynamic state */
#else
	uint16_t opMode:2;
	uint16_t nonGFDevicesPresent:1;
	uint16_t transmitBurstLimit:1;
	uint16_t obssNonHTStaPresent:1; /*added for Obss  */
	uint16_t reserved:11;
#endif
} qdf_packed tSirMacHTInfoField2;

/* Additional HT IE Field3 */
typedef struct sSirMacHTInfoField3 {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t reserved:4;
	uint16_t pcoPhase:1;    /* Dynamic state */
	uint16_t pcoActive:1;   /* Dynamic state */
	uint16_t lsigTXOPProtectionFullSupport:1;       /* Dynamic state */
	uint16_t secondaryBeacon:1;     /* Dynamic state */
	uint16_t dualCTSProtection:1;   /* Dynamic state */
	uint16_t basicSTBCMCS:7;        /* Dynamic state */
#else
	uint16_t basicSTBCMCS:7;
	uint16_t dualCTSProtection:1;
	uint16_t secondaryBeacon:1;
	uint16_t lsigTXOPProtectionFullSupport:1;
	uint16_t pcoActive:1;
	uint16_t pcoPhase:1;
	uint16_t reserved:4;
#endif
} qdf_packed tSirMacHTInfoField3;

typedef struct sSirMacProbeReqFrame {
	tSirMacSSidIE ssIdIE;
	tSirMacRateSetIE rateSetIE;
	tSirMacRateSetIE extendedRateSetIE;
} qdf_packed tSirMacProbeReqFrame, *tpSirMacProbeReqFrame;

typedef struct sSirMacProbeRspFrame {
	tSirMacTimeStamp ts;
	tSirMacBeaconInterval beaconInterval;
	tSirMacCapabilityInfo capabilityInfo;
	tSirMacSSidIE ssIdIE;
	tSirMacRateSetIE rateSetIE;
	tSirMacRateSetIE extendedRateSetIE;
	tSirMacNonErpPresentIE nonErpPresent;
	tSirMacDsParamSetIE dsParamsIE;
	tSirMacCfParamSetIE cfParamsIE;
} qdf_packed tSirMacProbeRspFrame, *tpSirMacProbeRspFrame;

typedef struct sSirMacAuthFrameBody {
	uint16_t authAlgoNumber;
	uint16_t authTransactionSeqNumber;
	uint16_t authStatusCode;
	uint8_t type;           /* = SIR_MAC_CHALLENGE_TEXT_EID */
	uint8_t length;         /* = SIR_MAC_AUTH_CHALLENGE_LENGTH */
	uint8_t challengeText[SIR_MAC_AUTH_CHALLENGE_LENGTH];
#ifdef WLAN_FEATURE_FILS_SK
	tSirMacRsnInfo rsn_ie;
	uint8_t assoc_delay_info;
	uint8_t session[SIR_FILS_SESSION_LENGTH];
	uint8_t wrapped_data_len;
	uint8_t wrapped_data[SIR_FILS_WRAPPED_DATA_MAX_SIZE];
	uint8_t nonce[SIR_FILS_NONCE_LENGTH];
#endif
} qdf_packed tSirMacAuthFrameBody, *tpSirMacAuthFrameBody;

typedef struct sSirMacAuthenticationFrame {
	tSirMacAuthFrameBody authFrameBody;
} qdf_packed tSirMacAuthFrame, *tpSirMacAuthFrame;

typedef struct sSirMacAssocReqFrame {
	tSirMacCapabilityInfo capabilityInfo;
	uint16_t listenInterval;
	tSirMacSSidIE ssIdIE;
	tSirMacRateSetIE rateSetIE;
	tSirMacRateSetIE extendedRateSetIE;
} qdf_packed tSirMacAssocReqFrame, *tpSirMacAssocReqFrame;

typedef struct sSirMacAssocRspFrame {
	tSirMacCapabilityInfo capabilityInfo;
	uint16_t statusCode;
	uint16_t aid;
	tSirMacRateSetIE supportedRates;
	tSirMacRateSetIE extendedRateSetIE;
} qdf_packed tSirMacAssocRspFrame, *tpSirMacAssocRspFrame;

typedef struct sSirMacDisassocFrame {
	uint16_t reasonCode;
} qdf_packed tSirMacDisassocFrame, *tpSirMacDisassocFrame;

typedef struct sDSirMacDeauthFrame {
	uint16_t reasonCode;
} qdf_packed tSirMacDeauthFrame, *tpSirMacDeauthFrame;

/* / Common header for all action frames */
typedef struct sSirMacActionFrameHdr {
	uint8_t category;
	uint8_t actionID;
} qdf_packed tSirMacActionFrameHdr, *tpSirMacActionFrameHdr;

typedef struct sSirMacVendorSpecificFrameHdr {
	uint8_t category;
	uint8_t Oui[4];
} qdf_packed tSirMacVendorSpecificFrameHdr,
*tpSirMacVendorSpecificFrameHdr;

typedef struct sSirMacVendorSpecificPublicActionFrameHdr {
	uint8_t category;
	uint8_t actionID;
	uint8_t Oui[4];
	uint8_t OuiSubType;
	uint8_t dialogToken;
} qdf_packed tSirMacVendorSpecificPublicActionFrameHdr,
*tpSirMacVendorSpecificPublicActionFrameHdr;

typedef struct sSirMacP2PActionFrameHdr {
	uint8_t category;
	uint8_t Oui[4];
	uint8_t OuiSubType;
	uint8_t dialogToken;
} qdf_packed tSirMacP2PActionFrameHdr, *tpSirMacP2PActionFrameHdr;

typedef struct sSirMacMeasActionFrameHdr {
	uint8_t category;
	uint8_t actionID;
	uint8_t dialogToken;
} tSirMacMeasActionFrameHdr, *tpSirMacMeasActionFrameHdr;

#ifdef ANI_SUPPORT_11H
typedef struct sSirMacTpcReqActionFrame {
	tSirMacMeasActionFrameHdr actionHeader;
	uint8_t type;
	uint8_t length;
} tSirMacTpcReqActionFrame, *tpSirMacTpcReqActionFrame;
typedef struct sSirMacMeasReqActionFrame {
	tSirMacMeasActionFrameHdr actionHeader;
	tSirMacMeasReqIE measReqIE;
} tSirMacMeasReqActionFrame, *tpSirMacMeasReqActionFrame;
#endif

typedef struct sSirMacNeighborReportReq {
	uint8_t dialogToken;
	uint8_t ssid_present;
	tSirMacSSid ssid;
} tSirMacNeighborReportReq, *tpSirMacNeighborReportReq;

typedef struct sSirMacLinkReport {
	uint8_t dialogToken;
	uint8_t txPower;
	uint8_t rxAntenna;
	uint8_t txAntenna;
	uint8_t rcpi;
	uint8_t rsni;
} tSirMacLinkReport, *tpSirMacLinkReport;

#define BEACON_REPORT_MAX_IES 224       /* Refer IEEE 802.11k-2008, Table 7-31d */
typedef struct sSirMacBeaconReport {
	uint8_t regClass;
	uint8_t channel;
	uint8_t measStartTime[8];
	uint8_t measDuration;
	uint8_t phyType;
	uint8_t bcnProbeRsp;
	uint8_t rsni;
	uint8_t rcpi;
	tSirMacAddr bssid;
	uint8_t antennaId;
	uint32_t parentTSF;
	uint8_t numIes;
	uint8_t Ies[BEACON_REPORT_MAX_IES];

} tSirMacBeaconReport, *tpSirMacBeaconReport;

#define RADIO_REPORTS_MAX_IN_A_FRAME 4
typedef struct sSirMacRadioMeasureReport {
	uint8_t token;
	uint8_t refused;
	uint8_t incapable;
	uint8_t type;
	union {
		tSirMacBeaconReport beaconReport;
	} report;

} tSirMacRadioMeasureReport, *tpSirMacRadioMeasureReport;

#ifdef WLAN_FEATURE_11AX
struct he_cap_network_endian {
	uint32_t htc_he:1;
	uint32_t twt_request:1;
	uint32_t twt_responder:1;
	uint32_t fragmentation:2;
	uint32_t max_num_frag_msdu:3;
	uint32_t min_frag_size:2;
	uint32_t trigger_frm_mac_pad:2;
	uint32_t multi_tid_aggr:3;
	uint32_t he_link_adaptation:2;
	uint32_t all_ack:1;
	uint32_t ul_mu_rsp_sched:1;
	uint32_t a_bsr:1;
	uint32_t broadcast_twt:1;
	uint32_t ba_32bit_bitmap:1;
	uint32_t mu_cascade:1;
	uint32_t ack_enabled_multitid:1;
	uint32_t dl_mu_ba:1;
	uint32_t omi_a_ctrl:1;
	uint32_t ofdma_ra:1;
	uint32_t max_ampdu_len:2;
	uint32_t amsdu_frag:1;
	uint32_t flex_twt_sched:1;
	uint32_t rx_ctrl_frame:1;

	uint8_t bsrp_ampdu_aggr:1;
	uint8_t qtp:1;
	uint8_t a_bqr:1;
	uint8_t sr_responder:1;
	uint8_t ndp_feedback_supp:1;
	uint8_t ops_supp:1;
	uint8_t amsdu_in_ampdu:1;
	uint8_t reserved1:1;

	uint32_t dual_band:1;
	uint32_t chan_width:7;
	uint32_t rx_pream_puncturing:4;
	uint32_t device_class:1;
	uint32_t ldpc_coding:1;
	uint32_t he_1x_ltf_800_gi_ppdu:1;
	uint32_t midamble_rx_max_nsts:2;
	uint32_t he_4x_ltf_3200_gi_ndp:1;
	uint32_t tx_stbc_lt_80mhz:1;
	uint32_t rx_stbc_lt_80mhz:1;
	uint32_t doppler:2;
	uint32_t ul_mu:2;
	uint32_t dcm_enc_tx:3;
	uint32_t dcm_enc_rx:3;
	uint32_t ul_he_mu:1;
	uint32_t su_beamformer:1;

	uint32_t su_beamformee:1;
	uint32_t mu_beamformer:1;
	uint32_t bfee_sts_lt_80:3;
	uint32_t bfee_sts_gt_80:3;
	uint32_t num_sounding_lt_80:3;
	uint32_t num_sounding_gt_80:3;
	uint32_t su_feedback_tone16:1;
	uint32_t mu_feedback_tone16:1;
	uint32_t codebook_su:1;
	uint32_t codebook_mu:1;
	uint32_t beamforming_feedback:3;
	uint32_t he_er_su_ppdu:1;
	uint32_t dl_mu_mimo_part_bw:1;
	uint32_t ppet_present:1;
	uint32_t srp:1;
	uint32_t power_boost:1;
	uint32_t he_ltf_800_gi_4x:1;
	uint32_t max_nc:3;
	uint32_t tx_stbc_gt_80mhz:1;
	uint32_t rx_stbc_gt_80mhz:1;

	uint8_t er_he_ltf_800_gi_4x:1;
	uint8_t he_ppdu_20_in_40Mhz_2G:1;
	uint8_t he_ppdu_20_in_160_80p80Mhz:1;
	uint8_t he_ppdu_80_in_160_80p80Mhz:1;
	uint8_t er_1x_he_ltf_gi:1;
	uint8_t midamble_rx_1x_he_ltf:1;
	uint8_t reserved2:2;

	uint16_t rx_he_mcs_map_lt_80;
	uint16_t tx_he_mcs_map_lt_80;
	uint16_t rx_he_mcs_map_160;
	uint16_t tx_he_mcs_map_160;
	uint16_t rx_he_mcs_map_80_80;
	uint16_t tx_he_mcs_map_80_80;
} qdf_packed;

struct he_ops_network_endian {
	uint32_t            bss_color:6;
	uint32_t           default_pe:3;
	uint32_t         twt_required:1;
	uint32_t        rts_threshold:10;
	uint32_t      partial_bss_col:1;
	uint32_t     vht_oper_present:1;
	uint32_t            reserved1:6;
	uint32_t            mbssid_ap:1;
	uint32_t         tx_bssid_ind:1;
	uint32_t     bss_col_disabled:1;
	uint32_t            reserved2:1;
	uint8_t             basic_mcs_nss[2];
	union {
		struct {
			uint8_t chan_width;
			uint8_t center_freq_seg0;
			uint8_t center_freq_seg1;
		} info; /* vht_oper_present = 1 */
	} vht_oper;
	union {
		struct {
			uint8_t data;
		} info; /* mbssid_ap = 1 */
	} maxbssid_ind;
} qdf_packed;

/* HE Capabilities Info */
struct he_capability_info {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint32_t rx_ctrl_frame:1;
	uint32_t flex_twt_sched:1;
	uint32_t amsdu_frag:1;
	uint32_t max_ampdu_len:2;
	uint32_t ofdma_ra:1;
	uint32_t omi_a_ctrl:1;
	uint32_t dl_mu_ba:1;
	uint32_t ack_enabled_multitid:1;
	uint32_t mu_cascade:1;
	uint32_t ba_32bit_bitmap:1;
	uint32_t broadcast_twt:1;
	uint32_t a_bsr:1;
	uint32_t ul_mu_rsp_sched:1;
	uint32_t all_ack:1;
	uint32_t he_link_adaptation:2;
	uint32_t multi_tid_aggr:3;
	uint32_t trigger_frm_mac_pad:2;
	uint32_t min_frag_size:2;
	uint32_t max_num_frag_msdu:3;
	uint32_t fragmentation:2;
	uint32_t twt_responder:1;
	uint32_t twt_request:1;
	uint32_t htc_he:1;

	uint8_t reserved1:1;
	uint8_t amsdu_in_ampdu:1;
	uint8_t ops_supp:1;
	uint8_t ndp_feedback_supp:1;
	uint8_t sr_responder:1;
	uint8_t a_bqr:1;
	uint8_t qtp:1;
	uint8_t bsrp_ampdu_aggr:1;

	uint32_t su_beamformer:1;
	uint32_t ul_he_mu:1;
	uint32_t dcm_enc_rx:3;
	uint32_t dcm_enc_tx:3;
	uint32_t ul_mu:2;
	uint32_t doppler:2;
	uint32_t rx_stbc_lt_80mhz:1;
	uint32_t tx_stbc_lt_80mhz:1;
	uint32_t he_4x_ltf_3200_gi_ndp:1;
	uint32_t midamble_rx_max_nsts:2;
	uint32_t he_1x_ltf_800_gi_ppdu:1;
	uint32_t ldpc_coding:1;
	uint32_t device_class:1;
	uint32_t rx_pream_puncturing:4;
	uint32_t chan_width:7;
	uint32_t dual_band:1;

	uint32_t rx_stbc_gt_80mhz:1;
	uint32_t tx_stbc_gt_80mhz:1;
	uint32_t max_nc:3;
	uint32_t he_ltf_800_gi_4x:1;
	uint32_t power_boost:1;
	uint32_t srp:1;
	uint32_t ppet_present:1;
	uint32_t dl_mu_mimo_part_bw:1;
	uint32_t he_er_su_ppdu:1;
	uint32_t beamforming_feedback:3;
	uint32_t codebook_mu:1;
	uint32_t codebook_su:1;
	uint32_t mu_feedback_tone16:1;
	uint32_t su_feedback_tone16:1;
	uint32_t num_sounding_gt_80:3;
	uint32_t num_sounding_lt_80:3;
	uint32_t bfee_sts_gt_80:3;
	uint32_t bfee_sts_lt_80:3;
	uint32_t mu_beamformer:1;
	uint32_t su_beamformee:1;

	uint8_t reserved2:2;
	uint8_t midamble_rx_1x_he_ltf:1;
	uint8_t er_1x_he_ltf_gi:1;
	uint8_t he_ppdu_80_in_160_80p80Mhz:1;
	uint8_t he_ppdu_20_in_160_80p80Mhz:1;
	uint8_t he_ppdu_20_in_40Mhz_2G:1;
	uint8_t er_he_ltf_800_gi_4x:1;

	uint16_t tx_he_mcs_map_80_80;
	uint16_t rx_he_mcs_map_80_80;
	uint16_t tx_he_mcs_map_160;
	uint16_t rx_he_mcs_map_160;
	uint16_t tx_he_mcs_map_lt_80;
	uint16_t rx_he_mcs_map_lt_80;
#else
	uint32_t htc_he:1;
	uint32_t twt_request:1;
	uint32_t twt_responder:1;
	uint32_t fragmentation:2;
	uint32_t max_num_frag_msdu:3;
	uint32_t min_frag_size:2;
	uint32_t trigger_frm_mac_pad:2;
	uint32_t multi_tid_aggr:3;
	uint32_t he_link_adaptation:2;
	uint32_t all_ack:1;
	uint32_t ul_mu_rsp_sched:1;
	uint32_t a_bsr:1;
	uint32_t broadcast_twt:1;
	uint32_t ba_32bit_bitmap:1;
	uint32_t mu_cascade:1;
	uint32_t ack_enabled_multitid:1;
	uint32_t dl_mu_ba:1;
	uint32_t omi_a_ctrl:1;
	uint32_t ofdma_ra:1;
	uint32_t max_ampdu_len:2;
	uint32_t amsdu_frag:1;
	uint32_t flex_twt_sched:1;
	uint32_t rx_ctrl_frame:1;

	uint8_t bsrp_ampdu_aggr:1;
	uint8_t qtp:1;
	uint8_t a_bqr:1;
	uint8_t sr_responder:1;
	uint8_t ndp_feedback_supp:1;
	uint8_t ops_supp:1;
	uint8_t amsdu_in_ampdu:1;
	uint8_t reserved1:1;

	uint32_t dual_band:1;
	uint32_t chan_width:7;
	uint32_t rx_pream_puncturing:4;
	uint32_t device_class:1;
	uint32_t ldpc_coding:1;
	uint32_t he_1x_ltf_800_gi_ppdu:1;
	uint32_t midamble_rx_max_nsts:2;
	uint32_t he_4x_ltf_3200_gi_ndp:1;
	uint32_t tx_stbc_lt_80mhz:1;
	uint32_t rx_stbc_lt_80mhz:1;
	uint32_t doppler:2;
	uint32_t ul_mu:2;
	uint32_t dcm_enc_tx:3;
	uint32_t dcm_enc_rx:3;
	uint32_t ul_he_mu:1;
	uint32_t su_beamformer:1;

	uint32_t su_beamformee:1;
	uint32_t mu_beamformer:1;
	uint32_t bfee_sts_lt_80:3;
	uint32_t bfee_sts_gt_80:3;
	uint32_t num_sounding_lt_80:3;
	uint32_t num_sounding_gt_80:3;
	uint32_t su_feedback_tone16:1;
	uint32_t mu_feedback_tone16:1;
	uint32_t codebook_su:1;
	uint32_t codebook_mu:1;
	uint32_t beamforming_feedback:3;
	uint32_t he_er_su_ppdu:1;
	uint32_t dl_mu_mimo_part_bw:1;
	uint32_t ppet_present:1;
	uint32_t srp:1;
	uint32_t power_boost:1;
	uint32_t he_ltf_800_gi_4x:1;
	uint32_t max_nc:3;
	uint32_t tx_stbc_gt_80mhz:1;
	uint32_t rx_stbc_gt_80mhz:1;

	uint8_t er_he_ltf_800_gi_4x:1;
	uint8_t he_ppdu_20_in_40Mhz_2G:1;
	uint8_t he_ppdu_20_in_160_80p80Mhz:1;
	uint8_t he_ppdu_80_in_160_80p80Mhz:1;
	uint8_t er_1x_he_ltf_gi:1;
	uint8_t midamble_rx_1x_he_ltf:1;
	uint8_t reserved2:2;

	uint16_t rx_he_mcs_map_lt_80;
	uint16_t tx_he_mcs_map_lt_80;
	uint16_t rx_he_mcs_map_160;
	uint16_t tx_he_mcs_map_160;
	uint16_t rx_he_mcs_map_80_80;
	uint16_t tx_he_mcs_map_80_80;
#endif
} qdf_packed;
#endif

/*
 * frame parser does not include optional 160 and 80+80 mcs set for MIN IE len
 */
#define SIR_MAC_HE_CAP_MIN_LEN       (DOT11F_IE_HE_CAP_MIN_LEN + 8)

/* QOS action frame definitions */

/* max number of possible tclas elements in any frame */
#define SIR_MAC_TCLASIE_MAXNUM  2

/* 11b rate encoding in MAC format */

#define SIR_MAC_RATE_1   0x02
#define SIR_MAC_RATE_2   0x04
#define SIR_MAC_RATE_5_5 0x0B
#define SIR_MAC_RATE_11  0x16

/* 11a/g rate encoding in MAC format */

#define SIR_MAC_RATE_6   0x0C
#define SIR_MAC_RATE_9   0x12
#define SIR_MAC_RATE_12  0x18
#define SIR_MAC_RATE_18  0x24
#define SIR_MAC_RATE_24  0x30
#define SIR_MAC_RATE_36  0x48
#define SIR_MAC_RATE_48  0x60
#define SIR_MAC_RATE_54  0x6C

/* ANI legacy supported rates */
#define SIR_MAC_RATE_72  0x01
#define SIR_MAC_RATE_96  0x03
#define SIR_MAC_RATE_108 0x05

/* ANI enhanced rates */
#define SIR_MAC_RATE_42  1000
#define SIR_MAC_RATE_84  1001
#define SIR_MAC_RATE_126 1002
#define SIR_MAC_RATE_144 1003
#define SIR_MAC_RATE_168 1004
#define SIR_MAC_RATE_192 1005
#define SIR_MAC_RATE_216 1006
#define SIR_MAC_RATE_240 1007

#define SIR_MAC_RATE_1_BITMAP    (1<<0)
#define SIR_MAC_RATE_2_BITMAP    (1<<1)
#define SIR_MAC_RATE_5_5_BITMAP  (1<<2)
#define SIR_MAC_RATE_11_BITMAP   (1<<3)
#define SIR_MAC_RATE_6_BITMAP    (1<<4)
#define SIR_MAC_RATE_9_BITMAP    (1<<5)
#define SIR_MAC_RATE_12_BITMAP   (1<<6)
#define SIR_MAC_RATE_18_BITMAP   (1<<7)
#define SIR_MAC_RATE_24_BITMAP   (1<<8)
#define SIR_MAC_RATE_36_BITMAP   (1<<9)
#define SIR_MAC_RATE_48_BITMAP   (1<<10)
#define SIR_MAC_RATE_54_BITMAP   (1<<11)

#define sirIsArate(x) ((((uint8_t)x) == SIR_MAC_RATE_6)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_9)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_12) || \
		       (((uint8_t)x) == SIR_MAC_RATE_18) || \
		       (((uint8_t)x) == SIR_MAC_RATE_24) || \
		       (((uint8_t)x) == SIR_MAC_RATE_36) || \
		       (((uint8_t)x) == SIR_MAC_RATE_48) || \
		       (((uint8_t)x) == SIR_MAC_RATE_54))

#define sirIsBrate(x) ((((uint8_t)x) == SIR_MAC_RATE_1)   || \
		       (((uint8_t)x) == SIR_MAC_RATE_2)   || \
		       (((uint8_t)x) == SIR_MAC_RATE_5_5) || \
		       (((uint8_t)x) == SIR_MAC_RATE_11))

#define sirIsGrate(x) ((((uint8_t)x) == SIR_MAC_RATE_1)   || \
		       (((uint8_t)x) == SIR_MAC_RATE_2)   || \
		       (((uint8_t)x) == SIR_MAC_RATE_5_5) || \
		       (((uint8_t)x) == SIR_MAC_RATE_11)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_6)   || \
		       (((uint8_t)x) == SIR_MAC_RATE_9)   || \
		       (((uint8_t)x) == SIR_MAC_RATE_12)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_18)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_24)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_36)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_48)  || \
		       (((uint8_t)x) == SIR_MAC_RATE_54))

#define SIR_MAC_MIN_IE_LEN 2    /* Minimum IE length for IE validation */

#define SIR_MAC_TI_TYPE_REASSOC_DEADLINE        1
#define SIR_MAC_TI_TYPE_KEY_LIFETIME            2
#define SIR_MAC_TI_TYPE_ASSOC_COMEBACK          3

#define SIR_MAC_VHT_CAP_MAX_MPDU_LEN              0
#define SIR_MAC_VHT_CAP_SUPP_CH_WIDTH_SET         2
#define SIR_MAC_VHT_CAP_LDPC_CODING_CAP           4
#define SIR_MAC_VHT_CAP_SHORTGI_80MHZ             5
#define SIR_MAC_VHT_CAP_SHORTGI_160_80_80MHZ      6
#define SIR_MAC_VHT_CAP_TXSTBC                    7
#define SIR_MAC_VHT_CAP_RXSTBC                    8
#define SIR_MAC_VHT_CAP_SU_BEAMFORMER_CAP         11
#define SIR_MAC_VHT_CAP_SU_BEAMFORMEE_CAP         12
#define SIR_MAC_VHT_CAP_CSN_BEAMORMER_ANT_SUP     13
#define SIR_MAC_VHT_CAP_NUM_SOUNDING_DIM          16
#define SIR_MAC_VHT_CAP_NUM_BEAM_FORMER_CAP       19
#define SIR_MAC_VHT_CAP_NUM_BEAM_FORMEE_CAP       20
#define SIR_MAC_VHT_CAP_TXOPPS                    21
#define SIR_MAC_VHT_CAP_HTC_CAP                   22
#define SIR_MAC_VHT_CAP_MAX_AMDU_LEN_EXPO         23
#define SIR_MAC_VHT_CAP_LINK_ADAPT_CAP            26
#define SIR_MAC_VHT_CAP_RX_ANTENNA_PATTERN        28
#define SIR_MAC_VHT_CAP_TX_ANTENNA_PATTERN        29
#define SIR_MAC_VHT_CAP_RESERVED2                 30

#define SIR_MAC_HT_CAP_ADVCODING_S                 0
#define SIR_MAC_HT_CAP_CHWIDTH40_S                 1
#define SIR_MAC_HT_CAP_SMPOWERSAVE_DYNAMIC_S       2
#define SIR_MAC_HT_CAP_SM_RESERVED_S               3
#define SIR_MAC_HT_CAP_GREENFIELD_S                4
#define SIR_MAC_HT_CAP_SHORTGI20MHZ_S              5
#define SIR_MAC_HT_CAP_SHORTGI40MHZ_S              6
#define SIR_MAC_HT_CAP_TXSTBC_S                    7
#define SIR_MAC_HT_CAP_RXSTBC_S                    8
#define SIR_MAC_HT_CAP_DELAYEDBLKACK_S            10
#define SIR_MAC_HT_CAP_MAXAMSDUSIZE_S             11
#define SIR_MAC_HT_CAP_DSSSCCK40_S                12
#define SIR_MAC_HT_CAP_PSMP_S                     13
#define SIR_MAC_HT_CAP_INTOLERANT40_S             14
#define SIR_MAC_HT_CAP_LSIGTXOPPROT_S             15

#define SIR_MAC_TXSTBC                             1
#define SIR_MAC_RXSTBC                             1

#endif /* __MAC_PROT_DEFS_H */
