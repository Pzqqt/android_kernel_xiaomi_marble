/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 *
 */

#ifndef _WLAN_NAN_MSG_H_
#define _WLAN_NAN_MSG_H_
#include <a_osapi.h>
#include <wlan_nan_platform.h>

#ifndef WLAN_NAN_MSG_COMMON_HEADER_V2
    /*
     * For backwards compatibility, use the deprecated version of
     * wlan_nan_msg_common.h in systems that have not been updated
     * to use the defs from the replacement wlan_nan_msg_common_v2.h
     */
    #include <wlan_nan_msg_common.h>
#else
    #include <wlan_nan_msg_common_v2.h>
    typedef nan_msg_header_t tNanMsgHeader;
    typedef nan_msg_header_tp tpNanMsgHeader;
#endif

/*---------------------------------------------------------------------------
* WLAN NAN CONSTANTS
*--------------------------------------------------------------------------*/
#ifndef PACKED_PRE
#define PACKED_PRE PREPACK
#endif

#ifndef PACKED_POST
#define PACKED_POST POSTPACK
#endif

/* ALL THE INTERFACE DEFINITIONS ARE ASSUMED TO BE IN LITTLE ENDIAN ORDER.
 * BIG ENDIAN HOST IS RESPONSIBLE TO SEND/INTERPRET MESSAGES IN LITTLE
 * ENDIAN FORMAT ONLY
 */

/** 2 word representation of MAC addr */
typedef struct {
    A_UINT32 mac_addr31to0;
    A_UINT32 mac_addr47to32;
} wlan_nan_mac_addr;

/* NAN Statistics Request ID Codes */
typedef enum
{
    NAN_STATS_ID_FIRST = 0,
    NAN_STATS_ID_DE_PUBLISH = NAN_STATS_ID_FIRST,
    NAN_STATS_ID_DE_SUBSCRIBE,
    NAN_STATS_ID_DE_MAC,
    NAN_STATS_ID_DE_TIMING_SYNC,
    NAN_STATS_ID_DE_DW,
    NAN_STATS_ID_DE,
    NAN_STATS_ID_LAST
} tNanStatsId;

typedef enum
{
    NAN_TLV_TYPE_FIRST = 0,

    /* Service Discovery Frame types */
    NAN_TLV_TYPE_SDF_FIRST = NAN_TLV_TYPE_FIRST,
    NAN_TLV_TYPE_SERVICE_NAME = NAN_TLV_TYPE_SDF_FIRST,
    NAN_TLV_TYPE_SDF_MATCH_FILTER,
    NAN_TLV_TYPE_TX_MATCH_FILTER,
    NAN_TLV_TYPE_RX_MATCH_FILTER,
    NAN_TLV_TYPE_SERVICE_SPECIFIC_INFO,
    NAN_TLV_TYPE_EXT_SERVICE_SPECIFIC_INFO = 5,
    NAN_TLV_TYPE_VENDOR_SPECIFIC_ATTRIBUTE_TRANSMIT = 6,
    NAN_TLV_TYPE_VENDOR_SPECIFIC_ATTRIBUTE_RECEIVE = 7,
    NAN_TLV_TYPE_POST_NAN_CONNECTIVITY_CAPABILITIES_RECEIVE = 8,
    NAN_TLV_TYPE_POST_NAN_DISCOVERY_ATTRIBUTE_RECEIVE = 9,
    NAN_TLV_TYPE_BEACON_SDF_PAYLOAD_RECEIVE = 10,
    NAN_TLV_TYPE_NAN_DATA_PATH_PARAMS = 11,
    NAN_TLV_TYPE_NAN_DATA_SUPPORTED_BAND = 12,
    NAN_TLV_TYPE_2G_COMMITTED_DW = 13,
    NAN_TLV_TYPE_5G_COMMITTED_DW = 14,
    NAN_TLV_TYPE_NAN_DATA_RSPNR_MODE = 15,
    NAN_TLV_TYPE_NAN_DATA_ENABLED_IN_MATCH = 16,
    NAN_TLV_TYPE_NAN20_RSPNR_ACCEPT_POLICY = 17,
    NAN_TLV_TYPE_NAN_CSID = 18,
    NAN_TLV_TYPE_NAN_SCID = 19,
    NAN_TLV_TYPE_NAN_PMK = 20,
    NAN_TLV_TYPE_SDEA_CTRL_PARAMS = 21,
    NAN_TLV_TYPE_NAN20_RANGING_CONFIGURATION = 22,
    NAN_TLV_TYPE_CONFIG_DISCOVERY_INDICATIONS = 23,
    NAN_TLV_TYPE_NAN20_RANGING_REQUEST = 24,
    NAN_TLV_TYPE_NAN20_RANGING_RESULT = 25,
    NAN_TLV_TYPE_NAN20_RANGING_REQUEST_RECEIVED = 26,
    NAN_TLV_TYPE_NAN_PASSPHRASE = 27,
    NAN_TLV_TYPE_NAN_EXTENDED_SSI = 28,
    NAN_TLV_TYPE_DEV_CAP_ATTR_CAPABILITY = 29,
    NAN_TLV_TYPE_TRANSPORT_IP_PARAM = 30,
    NAN_TLV_TYPE_SERVICE_ID = 31,
    NAN_TLV_TYPE_PAIRING_CONFIGURATION = 32,
    NAN_TLV_TYPE_PAIRING_MATCH_PARAMS = 33,
    NAN_TLV_TYPE_BOOTSTRAPPING_PARAMS = 34,
    NAN_TLV_TYPE_BOOTSTRAPPING_COOKIE = 35,
    NAN_TLV_TYPE_NIRA_NOUNCE = 36,
    NAN_TLV_TYPE_NIRA_TAG = 37,
    NAN_TLV_TYPE_NAN_CSID_EXT = 38, /* replacement for NAN_TLV_TYPE_NAN_CSID */
    NAN_TLV_TYPE_CSIA_CAP = 39,
    NAN_TLV_TYPE_SDF_LAST = 4095,

    /* Configuration types */
    NAN_TLV_TYPE_CONFIG_FIRST = 4096,
    NAN_TLV_TYPE_24G_SUPPORT = NAN_TLV_TYPE_CONFIG_FIRST, //4096
    NAN_TLV_TYPE_24G_BEACON, //4097
    NAN_TLV_TYPE_24G_SDF, //4098
    NAN_TLV_TYPE_24G_RSSI_CLOSE, //4099
    NAN_TLV_TYPE_24G_RSSI_MIDDLE, //4100
    NAN_TLV_TYPE_24G_RSSI_CLOSE_PROXIMITY, //4101
    NAN_TLV_TYPE_5G_SUPPORT, //4102
    NAN_TLV_TYPE_5G_BEACON, //4103
    NAN_TLV_TYPE_5G_SDF, //4104
    NAN_TLV_TYPE_5G_RSSI_CLOSE, //4105
    NAN_TLV_TYPE_5G_RSSI_MIDDLE, //4106
    NAN_TLV_TYPE_5G_RSSI_CLOSE_PROXIMITY, //4107
    NAN_TLV_TYPE_SID_BEACON, //4108
    NAN_TLV_TYPE_HOP_COUNT_LIMIT, //4109
    NAN_TLV_TYPE_MASTER_PREFERENCE, //4110
    NAN_TLV_TYPE_CLUSTER_ID_LOW, //4111
    NAN_TLV_TYPE_CLUSTER_ID_HIGH, //4112
    NAN_TLV_TYPE_RSSI_AVERAGING_WINDOW_SIZE, //4113
    NAN_TLV_TYPE_CLUSTER_OUI_NETWORK_ID, //4114
    NAN_TLV_TYPE_SOURCE_MAC_ADDRESS, //4115
    NAN_TLV_TYPE_CLUSTER_ATTRIBUTE_IN_SDF, //4116
    NAN_TLV_TYPE_SOCIAL_CHANNEL_SCAN_PARAMS, //4117
    NAN_TLV_TYPE_DEBUGGING_FLAGS, //4118
    NAN_TLV_TYPE_POST_NAN_CONNECTIVITY_CAPABILITIES_TRANSMIT, //4119
    NAN_TLV_TYPE_POST_NAN_DISCOVERY_ATTRIBUTE_TRANSMIT, //4120
    NAN_TLV_TYPE_FURTHER_AVAILABILITY_MAP, //4121
    NAN_TLV_TYPE_HOP_COUNT_FORCE, //4122
    NAN_TLV_TYPE_RANDOM_FACTOR_FORCE, //4123
    NAN_TLV_TYPE_RANDOM_UPDATE_TIME, //4124
    NAN_TLV_TYPE_EARLY_WAKEUP, //4125
    NAN_TLV_TYPE_PERIODIC_SCAN_INTERVAL, //4126
    // 4127 unused
    NAN_TLV_TYPE_DW_INTERVAL = 4128, //4128
    NAN_TLV_TYPE_DB_INTERVAL, //4129
    NAN_TLV_TYPE_FURTHER_AVAILABILITY, //4130
    NAN_TLV_TYPE_24G_CHANNEL, //4131
    NAN_TLV_TYPE_5G_CHANNEL, //4132
    NAN_TLV_TYPE_DISC_MAC_ADDR_RANDOM_INTERVAL, //4133
    NAN_TLV_TYPE_RANGING_AUTO_RESPONSE_CFG, // 4134
    NAN_TLV_TYPE_RANGING_AUTO_RESPONE_CFG =
    NAN_TLV_TYPE_RANGING_AUTO_RESPONSE_CFG, // 4134
    NAN_TLV_TYPE_SUBSCRIBE_SID_BEACON, // 4135
    NAN_TLV_TYPE_DW_EARLY_TERMINATION, // 4136
    NAN_TLV_TYPE_TX_RX_CHAINS, // 4137
    NAN_TLV_TYPE_SYSTEM_RANGING_ENABLED, // 4138
    NAN_TLV_TYPE_UNSYNC_DISCOVERY_ENABLED, // 4139
    NAN_TLV_TYPE_FOLLOWUP_MGMT_RX_ENABLED, // 4140

    NAN_TLV_TYPE_CONFIG_LAST = 8191,

    /* Attributes types */
    NAN_TLV_TYPE_ATTRS_FIRST = 8192,

    NAN_TLV_TYPE_AVAILABILITY_INTERVALS_MAP = NAN_TLV_TYPE_ATTRS_FIRST,
    NAN_TLV_TYPE_WLAN_MESH_ID,
    NAN_TLV_TYPE_MAC_ADDRESS,
    NAN_TLV_TYPE_RECEIVED_RSSI_VALUE,
    NAN_TLV_TYPE_CLUSTER_ATTRIBUTE,
    NAN_TLV_TYPE_WLAN_INFRA_SSID,
    NAN_TLV_TYPE_NAN_SHARED_KEY_DESC_ATTR,
    NAN_TLV_TYPE_ATTRS_LAST = 12287,

    /* Events types */
    NAN_TLV_TYPE_EVENTS_FIRST = 12288,
    NAN_TLV_TYPE_SELF_STA_MAC_ADDR = NAN_TLV_TYPE_EVENTS_FIRST,
    NAN_TLV_TYPE_STARTED_CLUSTER,
    NAN_TLV_TYPE_JOINED_CLUSTER,
    NAN_TLV_TYPE_CLUSTER_SCAN_RESULTS,
    NAN_TLV_TYPE_FAW_MEM_AVAIL,
    NAN_TLV_TYPE_EVENTS_LAST = 16383,

    /* TCA types */
    NAN_TLV_TYPE_TCA_FIRST = 16384,
    NAN_TLV_TYPE_CLUSTER_SIZE_REQ = NAN_TLV_TYPE_TCA_FIRST,
    NAN_TLV_TYPE_CLUSTER_SIZE_RSP,
    NAN_TLV_TYPE_TCA_LAST = 32767,

    /* Statistics types */
    NAN_TLV_TYPE_STATS_FIRST = 32768,
    NAN_TLV_TYPE_DE_PUBLISH_STATS = NAN_TLV_TYPE_STATS_FIRST,
    NAN_TLV_TYPE_DE_SUBSCRIBE_STATS,
    NAN_TLV_TYPE_DE_MAC_STATS,
    NAN_TLV_TYPE_DE_TIMING_SYNC_STATS,
    NAN_TLV_TYPE_DE_DW_STATS,
    NAN_TLV_TYPE_DE_STATS,
    NAN_TLV_TYPE_STATS_LAST = 36863,

    /* Testmode types */
    NAN_TLV_TYPE_TESTMODE_FIRST = 36864,
    NAN_TLV_TYPE_TESTMODE_GENERIC_CMD = NAN_TLV_TYPE_TESTMODE_FIRST,
    NAN_TLV_TYPE_TESTMODE_LAST = 37000,

    /* NAN Security types */
    NAN_TLV_TYPE_SEC_FIRST = 37001,
    NAN_TLV_TYPE_SEC_IGTK_KDE = NAN_TLV_TYPE_SEC_FIRST,
    NAN_TLV_TYPE_SEC_BIGTK_KDE,
    NAN_TLV_TYPE_SEC_NM_TK,
    NAN_TLV_TYPE_GROUP_KEYS_PARAM,
    NAN_TLV_TYPE_SEC_LAST = 37100,

    /* NAN OEM Configuration types */
    NAN_TLV_TYPE_OEM_DATA_FIRST = 37101,
    NAN_TLV_TYPE_OEM1_DATA = NAN_TLV_TYPE_OEM_DATA_FIRST,
    NAN_TLV_TYPE_OEM_DATA_LAST  = 37150,


    NAN_TLV_TYPE_LAST = 65535
} tNanTlvType;


/* NAN Publish Types */
typedef enum
{
    NAN_PUBLISH_TYPE_UNSOLICITED = 0,
    NAN_PUBLISH_TYPE_SOLICITED,
    NAN_PUBLISH_TYPE_UNSOLICITED_SOLICITED,
    NAN_PUBLISH_TYPE_LAST,
} tNanPublishType;

/* NAN Transmit Types */
typedef enum
{
    NAN_TX_TYPE_BROADCAST = 0,
    NAN_TX_TYPE_UNICAST,
    NAN_TX_TYPE_LAST
} tNanTxType;

/* NAN Match Algorithms */
typedef enum
{
    NAN_MATCH_ALG_FIRST = 0,
    NAN_MATCH_ALG_MATCH_ONCE = NAN_MATCH_ALG_FIRST,
    NAN_MATCH_ALG_MATCH_CONTINUOUS,
    NAN_MATCH_ALG_NEVER,
    NAN_MATCH_ALG_LAST
} tNanMatchAlg;

/* NAN Indication Disable Flag */
typedef enum
{
    NAN_IND_ALWAYS = 0,
    NAN_IND_NEVER,
} tNanIndDisableFlag;

/* NAN Transmit Priorities */
typedef enum
{
    NAN_TX_PRIORITY_LOW = 0,
    NAN_TX_PRIORITY_NORMAL,
    NAN_TX_PRIORITY_HIGH,
    /* This is for special case if the service
     * want to respond in same DW */
    NAN_TX_PRIORITY_CRITICAL,
    NAN_TX_PRIORITY_LAST
} tNanTxPriority;

/* PMK length */
#define NAN_PMK_LEN          32
/* Passphrase length */
#define NAN_PASSPHRASE_MIN_LEN  8
#define NAN_PASSPHRASE_MAX_LEN  63
/* Supported NAN SCID buffer length */
#define NAN_SCID_BUF_LEN     256

/* NAN Status Codes */
typedef enum
{
    /* Protocol Response Codes */
    NAN_STATUS_SUCCESS = 0,
    NAN_STATUS_TIMEOUT = 1,
    NAN_STATUS_DE_FAILURE = 2,
    NAN_STATUS_INVALID_MSG_VERSION = 3,
    NAN_STATUS_INVALID_MSG_LEN = 4,
    NAN_STATUS_INVALID_MSG_ID = 5,
    NAN_STATUS_INVALID_HANDLE = 6,
    NAN_STATUS_NO_SPACE_AVAILABLE = 7,
    NAN_STATUS_INVALID_PUBLISH_TYPE = 8,
    NAN_STATUS_INVALID_TX_TYPE = 9,
    NAN_STATUS_INVALID_MATCH_ALGORITHM = 10,
    NAN_STATUS_DISABLE_IN_PROGRESS = 11,
    NAN_STATUS_INVALID_TLV_LEN = 12,
    NAN_STATUS_INVALID_TLV_TYPE =13,
    NAN_STATUS_MISSING_TLV_TYPE = 14,
    NAN_STATUS_INVALID_TOTAL_TLVS_LEN = 15,
    NAN_STATUS_INVALID_MATCH_HANDLE = 16,
    NAN_STATUS_INVALID_TLV_VALUE = 17,
    NAN_STATUS_INVALID_TX_PRIORITY = 18,
    NAN_STATUS_INVALID_CONNECTION_MAP = 19,
    NAN_STATUS_INVALID_TCA_ID = 20,
    NAN_STATUS_INVALID_STATS_ID = 21,
    NAN_STATUS_NAN_NOT_ALLOWED = 22,
    NAN_STATUS_NO_OTA_ACK = 23,
    NAN_STATUS_TX_FAIL = 24,
    NAN_STATUS_MULTIPLE_ENABLE = 25,
    NAN_STATUS_FOLLOWUP_QUEUE_FULL = 26,
    NAN_STATUS_INVALID_5G_CHANNEL = 27,
    NAN_STATUS_POLICY_MANAGER_NOT_SINGLE_MAC_MODE = 28,
    NAN_STATUS_VDEV_NOT_CREATED = 29,

    /* Configuration Response Codes */
    NAN_STATUS_INVALID_RSSI_CLOSE_VALUE = 4096,
    NAN_STATUS_INVALID_RSSI_MEDIUM_VALUE = 4097,
    NAN_STATUS_INVALID_HOP_COUNT_LIMIT = 4098,
    NAN_STATUS_INVALID_MASTER_PREFERENCE_VALUE = 4099,
    NAN_STATUS_INVALID_LOW_CLUSTER_ID_VALUE = 4100,
    NAN_STATUS_INVALID_HIGH_CLUSTER_ID_VALUE = 4101,
    NAN_STATUS_INVALID_BACKGROUND_SCAN_PERIOD = 4102,
    NAN_STATUS_INVALID_RSSI_PROXIMITY_VALUE = 4103,
    NAN_STATUS_INVALID_SCAN_CHANNEL = 4104,
    NAN_STATUS_INVALID_POST_NAN_CONNECTIVITY_CAPABILITIES_BITMAP = 4105,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_NUMCHAN_VALUE = 4106,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_DURATION_VALUE = 4107,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_CLASS_VALUE = 4108,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_CHANNEL_VALUE = 4109,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_AVAILABILITY_INTERVAL_BITMAP_VALUE = 4110,
    NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_MAP_ID = 4111,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_CONN_TYPE_VALUE = 4112,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_DEVICE_ROLE_VALUE = 4113,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_DURATION_VALUE = 4114,
    NAN_STATUS_INVALID_POST_NAN_DISCOVERY_BITMAP_VALUE = 4115,
    NAN_STATUS_MISSING_FUTHER_AVAILABILITY_MAP = 4116,
    NAN_STATUS_INVALID_BAND_CONFIG_FLAGS = 4117,
    NAN_STATUS_INVALID_RANDOM_FACTOR_UPDATE_TIME_VALUE = 4118,
    NAN_STATUS_INVALID_ONGOING_SCAN_PERIOD = 4119,
    NAN_STATUS_INVALID_DW_INTERVAL_VALUE = 4120,
    NAN_STATUS_INVALID_DB_INTERVAL_VALUE = 4121,

    /* Protocol Termination Indication Reason Codes */
    NAN_TERMINATED_REASON_INVALID = 8192,
    NAN_TERMINATED_REASON_TIMEOUT = 8193,
    NAN_TERMINATED_REASON_USER_REQUEST = 8194,
    NAN_TERMINATED_REASON_FAILURE = 8195,
    NAN_TERMINATED_REASON_COUNT_REACHED = 8196,
    NAN_TERMINATED_REASON_DE_SHUTDOWN = 8197, /* Deprecated */
    NAN_TERMINATED_REASON_DISABLE_IN_PROGRESS = 8198,
    NAN_TERMINATED_REASON_POST_DISC_ATTR_EXPIRED = 8199,
    NAN_TERMINATED_REASON_POST_DISC_LEN_EXCEEDED = 8200,
    NAN_TERMINATED_REASON_FURTHER_AVAIL_MAP_EMPTY = 8201,

    /* Status related to Key Install and PN */
    NAN_STATUS_INVALID_IGTK_PARAMS = 8501,
    NAN_STATUS_INVALID_BIGTK_PARAMS = 8502,
    NAN_STATUS_INVALID_TX_KEY_NOT_PRESENT = 8503,
    NAN_STATUS_INVALID_NO_PEER_ENTRY = 8504,

    /* 9000-9500 NDP Status type */
    NDP_UNSUPPORTED_CONCURRENCY = 9000,
    NDP_NAN_DATA_IFACE_CREATE_FAILED = 9001,
    NDP_NAN_DATA_IFACE_DELETE_FAILED = 9002,
    NDP_DATA_INITIATOR_REQUEST_FAILED = 9003,
    NDP_DATA_RESPONDER_REQUEST_FAILED = 9004,
    NDP_INVALID_SERVICE_INSTANCE_ID = 9005,
    NDP_INVALID_NDP_INSTANCE_ID = 9006,
    NDP_INVALID_RESPONSE_CODE = 9007,
    NDP_INVALID_APP_INFO_LEN = 9008,
    /* OTA failures and timeouts during negotiation */
    NDP_MGMT_FRAME_REQUEST_FAILED = 9009,
    NDP_MGMT_FRAME_RESPONSE_FAILED = 9010,
    NDP_MGMT_FRAME_CONFIRM_FAILED = 9011,
    NDP_END_FAILED = 9012,
    NDP_MGMT_FRAME_END_REQUEST_FAILED = 9013,
    NDP_MGMT_FRAME_SECURITY_INSTALL_FAILED = 9014,

    /* 9500 onwards vendor specific error codes */
    NDP_VENDOR_SPECIFIC_ERROR = 9500

} tNanStatusType;

/* Enumeration for Version */
typedef enum
{
   NAN_MSG_VERSION1 = 1,
}tNanMsgVersion;

/* NAN Error Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanErrorRspMsg, *tpNanErrorRspMsg;

/* NAN Configuration Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanConfigurationRspMsg, *tpNanConfigurationRspMsg;

/* NAN Publish Service Cancel Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanPublishServiceCancelRspMsg, *tpNanPublishServiceCancelRspMsg;

/* NAN Publish Service Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanPublishServiceRspMsg, *tpNanPublishServiceRspMsg;

/* NAN Subscribe Service Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanSubscribeServiceRspMsg, *tpNanSubscribeServiceRspMsg;

/* NAN Subscribe Service Cancel Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanSubscribeServiceCancelRspMsg, *tpNanSubscribeServiceCancelRspMsg;

/* NAN Transmit Followup Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanTransmitFollowupRspMsg, *tpNanTransmitFollowupRspMsg;

/* NAN Self Transmit Followup Indication */
typedef PACKED_PRE struct PACKED_POST
{
    tNanMsgHeader nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanSelfTransmitFollowupIndMsg, *tpNanSelfTransmitFollowupIndMsg;

/* NAN Statistics Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT16 status;
    A_UINT16 value;
    A_UINT8 statsId;
    A_UINT8 reserved;
} tNanStatsRspParams, *tpNanStatsRspParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
    tNanStatsRspParams  statsRspParams;
    A_UINT8 ptlv[1];
} tNanStatsRspMsg, *tpNanStatsRspMsg;

/* NAN Enable Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 value;
} tNanEnableRspMsg, *tpNanEnableRspMsg;

/* NAN Disable Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status;
    A_UINT16 reserved;
} tNanDisableRspMsg, *tpNanDisableRspMsg;

/* NAN TCA Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /* status of the request */
    A_UINT16 status;
    A_UINT16 value;
} tNanTcaRspMsg, *tpNanTcaRspMsg;

/* NAN Beacon Sdf Payload Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /* status of the request */
    A_UINT16        status;
    A_UINT16        value;
} tNanBcnSdfPayloadRspMsg, *tpNanBcnSdfPayloadRspMsg;

/* NAN Capabilities Rsp */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 status;
    A_UINT32 value;
    A_UINT32 maxConcurrentNanClusters;
    A_UINT32 maxNumPublishes;
    A_UINT32 maxNumSubscribes;
    A_UINT32 maxServiceNameLen;
    A_UINT32 maxMatchFilterLen;
    A_UINT32 maxTotalMatchFilterLen;
    A_UINT32 maxServiceSpecificInfoLen;
    A_UINT32 maxVsaDataLen;
    A_UINT32 maxMeshDataLen;
    A_UINT32 maxNanDataInterfaces;
    A_UINT32 maxNanDataPathSessions;
    A_UINT32 maxNanDataAppInfoLen;
    A_UINT32 maxNanQueuedTransmitFollowupMsgs;
    A_UINT32 nanDataSupportedBands;
    A_UINT32 nanCSIDSupported;
    /* Target capability max SCID buffer supported */
    A_UINT32 maxSCIDBufsupported;

    A_UINT32 nanSecuritySupported:1;
    A_UINT32 maxExtServiceSpecificInfoLen:16;
    A_UINT32 maxNanRttInitiatorSupported:5;
    A_UINT32 maxNanRttResponderSupported:5;
    A_UINT32 nanNDPESupported:1;
    A_UINT32 nanPairingSupported:1;
    A_UINT32 nanUSDPublisherSupported:1;
    A_UINT32 nanUSDSubscriberSupported:1;
    A_UINT32 reserved:1;

    A_UINT32 maxSubscribeAddress;
    A_UINT32 maxNanPairingSessions;
    A_UINT32 nanGroupMfpCap;
} tNanCapabilitiesRspParams, *tpNanCapabilitiesRspParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
    tNanCapabilitiesRspParams  capabilitiesRspParams;
} tNanCapabilitiesRspMsg, *tpNanCapabilitiesRspMsg;

/* NAN Beacon Sdf Payload Ind */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /*
    * TLVs:
    *
     * Required: MAC Address (when VSA Receive is present)
     * Optional: VSA Receive, Beacon SDF Payload Receive
    */
    A_UINT8         ptlv[1];
} tNanBcnSdfPayloadIndMsg, *tpNanBcnSdfPayloadIndMsg;

/* NAN TCA Ind TLV */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 rising:1;
    A_UINT32 falling:1;
    A_UINT32 reserved:30;
    A_UINT32 value;
} tNanTcaIndTlv, *tpNanTcaIndTlv;

/* NAN TCA Ind */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
    /*
     * TLVs:
     *
     * Required: TCA Ind TLV (Cluster Size Rsp).
     */
    A_UINT8 ptlv[1];
} tNanTcaIndMsg, *tpNanTcaIndMsg;

/* NAN Disable Ind */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /* reason for the termination */
    A_UINT16 reason;
    A_UINT16 reserved;
} tNanDisableIndMsg, *tpNanDisableIndMsg;

/* Event Ind */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
    /*
     * Excludes TLVs
     *
     * Optional: Self-Station MAC Address, Started Cluster, Joined Cluster
     */
    A_UINT8 ptlv[1];
} tNanEventIndMsg, *tpNanEventIndMsg;

/* NAN Publish Followup Ind */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 matchHandle;
    A_UINT32 window:1;
    A_UINT32 reserved:31;
    /*
     * Excludes TLVs
     *
     * Required: Service Specific Info or Extended Service Specific Info
     */
} tNanFollowupIndParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER           halHeader;
    tNanMsgHeader           nanHeader;
    tNanFollowupIndParams   followupIndParams;
    /*
     * Excludes TLVs
     *
     * Required: MAC Address, Service Specific Info (D bit is 0)
     * Optional: Service Specific Info (D bit is 1)
     */
    A_UINT8 ptlv[1];
} tNanFollowupIndMsg, *tpNanFollowupIndMsg;

/* NAN Subscribe Terminated Ind */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /* reason for the termination */
    A_UINT16 reason;
    A_UINT16 reserved;
} tNanSubscribeTerminatedIndMsg, *tpNanSubscribeTerminatedIndMsg;

/* NAN Unmatch Ind */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 matchHandle;
} tNanUnmatchIndParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER                   halHeader;
    tNanMsgHeader                   nanHeader;
    tNanUnmatchIndParams            unmatchIndParams;
} tNanUnmatchIndMsg, *tpNanUnmatchIndMsg;

/* NAN Match Ind */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 matchHandle;
    A_UINT32 beaconFrameFlag:1;
    A_UINT32 cacheExhaustedFlag:1;
    A_UINT32 reserved:30;
} tNanMatchIndParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER               halHeader;
    tNanMsgHeader               nanHeader;
    tNanMatchIndParams          matchIndParams;
    /*
     * Excludes TLVs
     *
     * Required: MAC Address
     * Optional: Service Specific Info, SDF Match Filter
     *           Received RSSI Value, Post NAN Connectivity Capabilities Rx,
     *           Post NAN Discovery Attribute Rx, Further Availability Map,
     *           Cluster Attribute, Sdea ctrl params, NAN CSID, NAN SCID
     */
    A_UINT8 ptlv[1];
} tNanMatchIndMsg, *tpNanMatchIndMsg;

/* NAN Publish Terminated Ind */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 reason;
    A_UINT16 reserved;
} tNanPublishTerminatedIndMsg, *tpNanPublishTerminatedIndMsg;

/* NAN Publish Replied Ind */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 matchHandle;
} tNanPublishRepliedIndParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader nanHeader;
    tNanPublishRepliedIndParams publishRepliedIndParams;
    /*
     * Excludes TLVs
     *
     * Required: MAC Address
     * Optional: Received RSSI Value, Post NAN Connectivity Capabilities Rx,
     *           Post NAN Discovery Attribute Rx, Further Availability Map,
     *           Cluster Attribute, Sdea ctrl params, NAN CSID, NAN SCID buffer
     */
    A_UINT8 ptlv[1];
} tNanPublishRepliedIndMsg, *tpNanPublishRepliedIndMsg;

/* NAN Enable Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /*
     * TLVs:
     *
     * Required: Master Preference, Cluster Low, Cluster High
     * Optional: 2.4G Support, 2.4G Beacons, 2.4G Discovery, 2.4G RSSI Close,
     *           2.4G RSSI Middle, 2.4G RSSI Close Proximity, 5G Support,
     *           5G Beacons, 5G Discovery, 5G RSSI Close, 5G RSSI Middle,
     *           5G RSSI Close Proximity, SID Beacon, Hop Count Limit,
     *           Random Factor Force, Hop Count Force, Master Preference,
     *           Cluster Low, Cluster High, RSSI Averaging Window Size,
     *           Cluster OUI Network ID, Source MAC Address,
     *           Cluster Attribute in SDF, Social Channel Scan Parameters,
     *           Debugging Flags, Post NAN Connectivity Capabilities Tx,
     *           Post NAN Discovery Attribute Tx, Further Availability Map
     */
    A_UINT8 ptlv[1];
} tNanEnableReqMsg, *tpNanEnableReqMsg;

/* NAN Disable Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
} tNanDisableReqMsg, *tpNanDisableReqMsg;

/* NAN Configuration Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /*
     * TLVs:
     *
     * Required: None.
     * Optional: 2.4G RSSI Close Proximity, 5G RSSI Close Proximity,
     *           SID Beacon, Random Factor Force, Hop Count Force,
     *           Master Preference, RSSI Averaging Window Size,
     *           Cluster Attribute, Social Channel Scan Parameters,
     *           Debugging Flags, Post NAN Connectivity Capabilities Tx,
     *           Post NAN Discovery Attribute Tx, Further Availability Map
     */
    A_UINT8 ptlv[1];
} tNanConfigurationReqMsg, *tpNanConfigurationReqMsg;

/* NAN Publish Service Req */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT16 ttl;
    A_UINT16 period;
    A_UINT32 replyIndFlag:1;
    A_UINT32 publishType:2;
    A_UINT32 txType:1;
    A_UINT32 useRssi:1;
    A_UINT32 otaFlag:1;
    A_UINT32 matchAlg:2;
    A_UINT32 count:8;
    A_UINT32 connMap:8;
    A_UINT32 pubTerminatedIndDisableFlag:1;
    A_UINT32 pubUnmatchIndDisableFlag:1;
    A_UINT32 followupRxIndDisableFlag:1;
    A_UINT32 reserved2:5;
} tNanPublishServiceReqParams, *tpNanPublishServiceReqParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER               halHeader;
    tNanMsgHeader               nanHeader;
    tNanPublishServiceReqParams publishServiceReqParams;
    /*
     * Excludes TLVs
     *
     * Required: Service Name
     * Optional: Service Specific Info, Tx Match Filter, Rx Match Filter
     *           Sdea ctrl params, NAN CSID, PMK, PASSPHRASE
     */
    A_UINT8 ptlv[1];
} tNanPublishServiceReqMsg, *tpNanPublishServiceReqMsg;

/* NAN Publish Service Cancel Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER halHeader;
    tNanMsgHeader nanHeader;
} tNanPublishServiceCancelReqMsg, *tpNanPublishServiceCancelReqMsg;

/* NAN Subscribe Service Req */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT16 ttl;
    A_UINT16 period;
    A_UINT32 subscribeType:1;
    A_UINT32 srfAttr:1;
    A_UINT32 srfInclude:1;
    A_UINT32 srfSend:1;
    A_UINT32 ssiRequired:1;
    A_UINT32 matchAlg:2;
    A_UINT32 reserved1:1;
    A_UINT32 count:8;
    A_UINT32 useRssi:1;
    A_UINT32 otaFlag:1;
    A_UINT32 subTerminatedIndDisableFlag:1;
    A_UINT32 subUnmatchIndDisableFlag:1;
    A_UINT32 followupRxIndDisableFlag:1;
    A_UINT32 reserved2:3;
    A_UINT32 connMap:8;
    /*
     * Excludes TLVs
     *
     * Required: Service Name
     * Optional: Rx Match Filter, Tx Match Filter, Service Specific Info,
     *           Group Key
     */
} tNanSubscribeServiceReqParams, *tpNanSubscribeServiceReqParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER                   halHeader;
    tNanMsgHeader                   nanHeader;
    tNanSubscribeServiceReqParams   subscribeServiceReqParams;
    /*
     * Excludes TLVs
     *
     * Required: Service Name
     * Optional: Rx Match Filter, Tx Match Filter, Service Specific Info,
     *           Sdea ctrl params, NAN CSID, PMK, PASSPHRASE
     */
    A_UINT8 ptlv[1];
} tNanSubscribeServiceReqMsg, *tpNanSubscribeServiceReqMsg;

/* NAN Subscribe Service Cancel Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER halHeader;
    tNanMsgHeader nanHeader;
} tNanSubscribeServiceCancelReqMsg, *tpNanSubscribeServiceCancelReqMsg;

/* NAN Transmit Followup Req */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 matchHandle;
    A_UINT32 priority:4;
    A_UINT32 window:1;
    A_UINT32 followupTxRspDisableFlag:1;
    A_UINT32 reserved:26;
    /*
     * Excludes TLVs
     *
     * Required: Service Specific Info or Extended Service Specific Info
     */
} tNanTransmitFollowupReqParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER                   halHeader;
    tNanMsgHeader                   nanHeader;
    tNanTransmitFollowupReqParams   transmitFollowupReqParams;
    /*
     * Excludes TLVs
     *
     * Required: MAC Address, Service Specific Info (D bit is 0)
     * Optional: Service Specific Info (D bit is 1)
     */
    A_UINT8 ptlv[1];
} tNanTransmitFollowupReqMsg, *tpNanTransmitFollowupReqMsg;

/* NAN Statistics Req */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 statsId:8;
    A_UINT32 clear:1;
    A_UINT32 reserved:23;
} tNanStatsReqParams, *tpNanStatsReqParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
    tNanStatsReqParams  statsReqParams;
} tNanStatsReqMsg, *tpNanStatsReqMsg;

/* NAN TCA Req TLV */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 rising:1;
    A_UINT32 falling:1;
    A_UINT32 clear:1;
    A_UINT32 reserved:29;
    A_UINT32 threshold;
} tNanTcaReqParams, *tpNanTcaReqParams;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
    /*
     * TLVs:
     *
     * Required: TCA Req TLV (Cluster Size Req).
     */
    A_UINT8 ptlv[1];
} tNanTcaReqMsg, *tpNanTcaReqMsg;

/* Publish statistics. */
typedef struct
{
    A_UINT32 validPublishServiceReqMsgs;
    A_UINT32 validPublishServiceRspMsgs;
    A_UINT32 validPublishServiceCancelReqMsgs;
    A_UINT32 validPublishServiceCancelRspMsgs;
    A_UINT32 validPublishRepliedIndMsgs;
    A_UINT32 validPublishTerminatedIndMsgs;
    A_UINT32 validActiveSubscribes;
    A_UINT32 validMatches;
    A_UINT32 validFollowups;
    A_UINT32 invalidPublishServiceReqMsgs;
    A_UINT32 invalidPublishServiceCancelReqMsgs;
    A_UINT32 invalidActiveSubscribes;
    A_UINT32 invalidMatches;
    A_UINT32 invalidFollowups;
    A_UINT32 publishCount;
    A_UINT32 publishNewMatchCount;
    A_UINT32 pubsubGlobalNewMatchCount;
} tNanPublishStats, *tpNanPublishStats;

/* Subscribe statistics. */
typedef struct
{
    A_UINT32 validSubscribeServiceReqMsgs;
    A_UINT32 validSubscribeServiceRspMsgs;
    A_UINT32 validSubscribeServiceCancelReqMsgs;
    A_UINT32 validSubscribeServiceCancelRspMsgs;
    A_UINT32 validSubscribeTerminatedIndMsgs;
    A_UINT32 validSubscribeMatchIndMsgs;
    A_UINT32 validSubscribeUnmatchIndMsgs;
    A_UINT32 validSolicitedPublishes;
    A_UINT32 validMatches;
    A_UINT32 validFollowups;
    A_UINT32 invalidSubscribeServiceReqMsgs;
    A_UINT32 invalidSubscribeServiceCancelReqMsgs;
    A_UINT32 invalidSubscribeFollowupReqMsgs;
    A_UINT32 invalidSolicitedPublishes;
    A_UINT32 invalidMatches;
    A_UINT32 invalidFollowups;
    A_UINT32 subscribeCount;
    A_UINT32 bloomFilterIndex;
    A_UINT32 subscribeNewMatchCount;
    A_UINT32 pubsubGlobalNewMatchCount;
} tNanSubscribeStats, *tpNanSubscribeStats;

/* NAN MAC Statistics. Used for MAC and DW statistics. */
typedef struct
{
    /* RX stats */
    A_UINT32 validFrames;
    A_UINT32 validActionFrames;
    A_UINT32 validBeaconFrames;
    A_UINT32 ignoredActionFrames;
    A_UINT32 ignoredBeaconFrames;
    A_UINT32 invalidFrames;
    A_UINT32 invalidActionFrames;
    A_UINT32 invalidBeaconFrames;
    A_UINT32 invalidMacHeaders;
    A_UINT32 invalidPafHeaders;
    A_UINT32 nonNanBeaconFrames;

    A_UINT32 earlyActionFrames;
    A_UINT32 inDwActionFrames;
    A_UINT32 lateActionFrames;

    /* TX stats */
    A_UINT32 framesQueued;
    A_UINT32 totalTRSpUpdates;
    A_UINT32 completeByTRSp;
    A_UINT32 completeByTp75DW;
    A_UINT32 completeByTendDW;
    A_UINT32 lateActionFramesTx;

    /* Misc stats - ignored for DW. */
    A_UINT32 twIncreases;
    A_UINT32 twDecreases;
    A_UINT32 twChanges;
    A_UINT32 twHighwater;
    A_UINT32 bloomFilterIndex;

} tNanMacStats, *tpNanMacStats;

/* NAN Sync Statistics: TBD */
typedef struct
{
    A_UINT64 currTsf;
    A_UINT64 myRank;
    A_UINT64 currAmRank;
    A_UINT64 lastAmRank;
    A_UINT32 currAmBTT;
    A_UINT32 lastAmBTT;
    A_UINT8  currAmHopCount;
    A_UINT8  currRole;
    A_UINT16 currClusterId;

    A_UINT32 reserved1; /* NOTE: Needed for padding for uint64 alignment of following 4 fields */

    A_UINT64 timeSpentInCurrRole;
    A_UINT64 totalTimeSpentAsMaster;
    A_UINT64 totalTimeSpentAsNonMasterSync;
    A_UINT64 totalTimeSpentAsNonMasterNonSync;
    A_UINT32 transitionsToAnchorMaster;
    A_UINT32 transitionsToMaster;
    A_UINT32 transitionsToNonMasterSync;
    A_UINT32 transitionsToNonMasterNonSync;
    A_UINT32 amrUpdateCount;
    A_UINT32 amrUpdateRankChangedCount;
    A_UINT32 amrUpdateBTTChangedCount;
    A_UINT32 amrUpdateHcChangedCount;
    A_UINT32 amrUpdateNewDeviceCount;
    A_UINT32 amrExpireCount;
    A_UINT32 mergeCount;
    A_UINT32 beaconsAboveHcLimit;
    A_UINT32 beaconsBelowRssiThresh;
    A_UINT32 beaconsIgnoredNoSpace;
    A_UINT32 beaconsForOurCluster;
    A_UINT32 beaconsForOtherCluster;
    A_UINT32 beaconCancelRequests;
    A_UINT32 beaconCancelFailures;
    A_UINT32 beaconUpdateRequests;
    A_UINT32 beaconUpdateFailures;
    A_UINT32 syncBeaconTxAttempts;
    A_UINT32 syncBeaconTxFailures;
    A_UINT32 discBeaconTxAttempts;
    A_UINT32 discBeaconTxFailures;
    A_UINT32 amHopCountExpireCount;
    A_UINT32 ndpChannelFreq;
    A_UINT32 ndpChannelFreq2;
    A_UINT32 schedUpdateChannelFreq;

} tNanSyncStats, *tpNanSyncStats;

/* NAN Misc DE Statistics */
typedef struct
{
    A_UINT32 validErrorRspMsgs;
    A_UINT32 validTransmitFollowupReqMsgs;
    A_UINT32 validTransmitFollowupRspMsgs;
    A_UINT32 validFollowupIndMsgs;
    A_UINT32 validConfigurationReqMsgs;
    A_UINT32 validConfigurationRspMsgs;
    A_UINT32 validStatsReqMsgs;
    A_UINT32 validStatsRspMsgs;
    A_UINT32 validEnableReqMsgs;
    A_UINT32 validEnableRspMsgs;
    A_UINT32 validDisableReqMsgs;
    A_UINT32 validDisableRspMsgs;
    A_UINT32 validDisableIndMsgs;
    A_UINT32 validEventIndMsgs;
    A_UINT32 validTcaReqMsgs;
    A_UINT32 validTcaRspMsgs;
    A_UINT32 validTcaIndMsgs;
    A_UINT32 invalidTransmitFollowupReqMsgs;
    A_UINT32 invalidConfigurationReqMsgs;
    A_UINT32 invalidStatsReqMsgs;
    A_UINT32 invalidEnableReqMsgs;
    A_UINT32 invalidDisableReqMsgs;
    A_UINT32 invalidTcaReqMsgs;
    A_UINT32 validBcnSdfPayloadReqMsgs;
    A_UINT32 validBcnSdfPayloadRspMsgs;
    A_UINT32 validBcnSdfPayloadIndMsgs;
    A_UINT32 invalidBcnSdfPayloadReqMsgs;
    A_UINT32 validCapabilitiesReqMsgs;
    A_UINT32 invalidCapabilitiesReqMsgs;
    A_UINT32 validCapabilitiesRspMsgs;
} tNanDeStats, *tpNanDeStats;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 s:1;
    A_UINT8 count:7;
} tNanSidAttr, *tpSidAttr;

/* NAN BcnSdfPayload Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    /*
     * TLVs:
     *
     * Required:  VSA Transmit
     * Optional:  None
     */
    A_UINT8         ptlv[1];
} tNanBcnSdfPayloadReqMsg, *tpNanBcnSdfPayloadReqMsg;

/* Get NAN Capabilities Req */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER       halHeader;
    tNanMsgHeader       nanHeader;
} tNanCapabilitiesReqMsg, *tpNanCapabilitiesReqMsg;

/* These are in the same order as the Conn Types below. */
#define NAN_CONN_MAP_NONE               0x00
#define NAN_CONN_MAP_WLAN_INFRA         0x01
#define NAN_CONN_MAP_P2P_OPERATION      0x02
#define NAN_CONN_MAP_WLAN_IBSS          0x04
#define NAN_CONN_MAP_WLAN_MESH          0x08
#define NAN_CONN_MAP_FURTHER_SVC_AVAIL  0x10
#define NAN_CONN_MAP_WLAN_RANGING       0x20
#define NAN_CONN_MAP_RESERVED           0x40
#define NAN_CONN_MAP_WILDCARD           0x80

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 channel;
    A_UINT8 dwellTime;
    A_UINT16 scanPeriod;
} tNanSocialChannelScanParams, *tpSocialChannelScanParams;

#define NAN_POST_NAN_CONN_CAP_WFD           0x0001
#define NAN_POST_NAN_CONN_CAP_WFDS          0x0002
#define NAN_POST_NAN_CONN_CAP_TDLS          0x0004
#define NAN_POST_NAN_CONN_CAP_WLAN_INFRA    0x0008
#define NAN_POST_NAN_CONN_CAP_IBSS          0x0010
#define NAN_POST_NAN_CONN_CAP_MESH          0x0020
#define NAN_POST_NAN_CONN_CAP_RESERVED      0xFFC0

/* Post-NAN Connectivity Capabilities Transmit TLV */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT16 connCap;
    A_UINT16 repeat:1;
    A_UINT16 reserved:15;
} tNanPostNanConnCapsTxTlv, *tpNanPostNanConnCapsTxTlv;

/* Post-NAN Connectivity Capabilities Receive TLV */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT16 connCap;
    A_UINT16 reserved;
} tNanPostNanConnCapsRxTlv, *tpNanPostNanConnCapsRxTlv;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 availIntDuration:2;

    A_UINT8 mapId:4;
    A_UINT8 reserved:2;
} tNanApiEntryCtrl;

typedef enum
{
    NAN_FAM_AID_FIRST = 0,
    NAN_FAM_AID_16TU = NAN_FAM_AID_FIRST,
    NAN_FAM_AID_32TU,
    NAN_FAM_AID_64TU,
    NAN_FAM_AID_RESERVED,
    NAN_FAM_AID_LAST = NAN_FAM_AID_RESERVED,
    NAN_NUM_FAM_AID = NAN_FAM_AID_LAST
} tNanFamAid;

/*
 * Convenience types for the Availability Intervals Bitmap. Could do
 * these as arrays of U8s too. They may or may not be useful...
 */
typedef A_UINT8 tNanAiBitmap64tu;
typedef A_UINT16 tNanAiBitmap32tu;
typedef A_UINT32 tNanAiBitmap16tu;

/*
 * Valid Operating Classes were derived from IEEE Std. 802.11-2012 Annex E
 * Table E-4 Global Operating Classes and, filtered by channel, are: 81, 83,
 * 84, 103, 114, 115, 116, 124, 125. (I think).
 */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 opClass;
    A_UINT8 channel;
    A_UINT8 availIntBitmap[4];
} tNanFurtherAvailabilityChan, *tpNanFurtherAvailabilityChan;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 numChan;
    tNanApiEntryCtrl entryCtrl;
    A_UINT8 pFaChan[1];
} tNanFurtherAvailabilityMapAttrTlv, *tpNanFurtherAvailabilityMapAttrTlv;

typedef enum
{
    NAN_CONN_TYPE_FIRST = 0,
    NAN_CONN_TYPE_WLAN_INFRA = NAN_CONN_TYPE_FIRST,
    NAN_CONN_TYPE_P2P_OPERATION,
    NAN_CONN_TYPE_WLAN_IBSS,
    NAN_CONN_TYPE_WLAN_MESH,
    NAN_CONN_TYPE_FURTHER_SVC_AVAIL,
    NAN_CONN_TYPE_WLAN_RANGING,
    NAN_CONN_TYPE_LAST,
    NAN_CONN_TYPE_RESERVED = NAN_CONN_TYPE_LAST,
    NAN_CONN_TYPE_WILDCARD
} tNanConnType;

/* Device Roles */
typedef enum
{
    NAN_DEVICE_ROLE_FIRST = 0,
    NAN_DEVICE_ROLE_WLAN_INFRA_FIRST = NAN_DEVICE_ROLE_FIRST,
    NAN_DEVICE_ROLE_WLAN_AP = NAN_DEVICE_ROLE_WLAN_INFRA_FIRST,
    NAN_DEVICE_ROLE_WLAN_STA,
    NAN_DEVICE_ROLE_WLAN_INFRA_LAST,
    NAN_DEVICE_ROLE_P2P_OPERATION_FIRST = NAN_DEVICE_ROLE_WLAN_INFRA_LAST,
    NAN_DEVICE_ROLE_P2P_GROUP_OWNER = NAN_DEVICE_ROLE_P2P_OPERATION_FIRST,
    NAN_DEVICE_ROLE_P2P_DEVICE,
    NAN_DEVICE_ROLE_P2P_CLIENT,
    NAN_DEVICE_ROLE_P2P_OPERATION_LAST,
    NAN_DEVICE_ROLE_LAST = NAN_DEVICE_ROLE_P2P_OPERATION_LAST
} tNanDeviceRole;

/*
 * Post-NAN Discovery Attribute MAC Address Usage:
 *
 *   Connection Type   |   Device Role   |    MAC Address Usage
 * --------------------+-----------------+-------------------------
 * WLAN Infrastructure | N/A             | MSSID of the AP
 * P2P Operation       | P2P_Group Owner | P2P Group Owner's address
 * P2P Operation       | P2P Device      | P2P Client's address
 * WLAN IBSS           | N/A             | BSSID
 * WLAN Mesh           | N/A             | BSSID
 * Other               | ???             | ???
 *
 */

/* Post-NAN Discovery Attribute Transmit TLV. */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 connType;
    A_UINT8 deviceRole;
    A_UINT16 repeat:1;
    A_UINT16 duration:2;
    A_UINT16 reserved:13;
    A_UINT8 availIntBitmap[4];
    /*
     * TLVs:
     *
     * Required: WLAN Mesh ID (if connType is WLAN_MESH),
     *           MAC Address (if connType is WLAN Infra, P2P, IBSS, or Mesh)
     */
    A_UINT8 ptlv[1];
} tNanPostNanDiscoveryAttrTxTlv, tpNanPostNanDiscoveryAttrTxTlv;

/* Post-NAN Discovery Attribute Receive TLV. */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 connType;
    A_UINT8 deviceRole;
    A_UINT16 duration:2;
    A_UINT16 mapId:4;
    A_UINT16 reserved:10;
    A_UINT8 availIntBitmap[4];
    /*
     * TLVs:
     *
     * Required: WLAN Mesh ID (if connType is WLAN_MESH),
     *           MAC Address (if connType is WLAN Infra, P2P, IBSS, or Mesh)
     */
    A_UINT8 ptlv[1];
} tNanPostNanDiscoveryAttrRxTlv, tpNanPostNanDiscoveryAttrRxTlv;

/* NAN Vendor Specific Attribute Transmit/Receive TLV */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 rflag:1;    /* Ignored for receive case. */
    A_UINT8 pbits:3;
    A_UINT8 reserved:4;
    A_UINT8 oui[3];

    A_UINT8 vsaData[1];
} tNanVsaTlv, *tpNanVsaTlv;

/*NAN 2.0 - SDEA*/
typedef struct
{
    A_UINT32 data_path_type:1;
    A_UINT32 reserved:31;
} tNanDataPathParamsTlv, *tpNanDataPathParamsTlv;

/* NAN 2.0 - SDEA CTRL */
typedef struct
{
    A_UINT32 fsd_required:1;
    A_UINT32 fsd_with_gas:1;
    A_UINT32 data_path_required:1;
    A_UINT32 data_path_type:1;
    A_UINT32 multicast_type:1;
    A_UINT32 qos_required:1;
    A_UINT32 security_required:1;
    A_UINT32 ranging_required:1;
    A_UINT32 range_limit_present:1;
    A_UINT32 service_update_ind_present:1;
    A_UINT32 reserved1:6;
    // Above 16 bits are used to send Over The Air as part of SDEA control
    // Below 16 bits are used for internal purposes
    A_UINT32 range_report_needed:1;
    A_UINT32 reserved2:15;
} tNanSdeaCtrlParamsTlv, *tpNanSdeaCtrlParamsTlv;

typedef struct
{
    A_UINT32 data_path_enabled_in_match:1;
    A_UINT32 reserved:31;
} tNanDataPathParmsEnabledMatchTlv, *tpNanDataPathParmsEnabledMatchTlv;

/* NAN Data responder mode */
typedef enum
{
    NAN_DATA_RSPNR_MODE_AUTO    = 0,
    NAN_DATA_RSPNR_MODE_ACCEPT  = 1,
    NAN_DATA_RSPNR_MODE_REJECT  = 2,
    NAN_DATA_RSPNR_MODE_COUNTER = 3,
    NAN_DATA_RSPNR_MODE_COUNTER_NO_CHANNEL_CHANGE = 4,
} tNanDataRspnrMode;

/**
 * NDC respond mode
 */
typedef enum
{
    NAN_NDC_MODE_AUTO    = 0,
    NAN_NDC_MODE_COUNTER = 1,
} tNanNdcRspMode;

/**
 * Nan20 accept policy
 */
typedef enum {
    NAN20_ACCEPT_POLICY_NONE = 0,
    NAN20_ACCEPT_POLICY_ALL = 1,
} tNan20AcceptPolicy;

/* NAN Cipher Suites Shared Key - to be deprecated */
typedef enum {
    NCS_SK_128 = 1,
    NCS_SK_256 = 2,
    NCS_SK_ALL = 3,
} NanCSID;

/* New Host to use below to send CSID entries as a bitmap */
typedef enum {
    NAN_NCS_F_SK_128       =  0x00001,
    NAN_NCS_F_SK_256       =  0x00002,
    NAN_NCS_F_PK_2WDH_128  =  0x00004,
    NAN_NCS_F_PK_2WDH_256  =  0x00008,
    NAN_NCS_F_GTK_CCMP_128 =  0x00010,
    NAN_NCS_F_GTK_GCMP_256 =  0x00020,
    NAN_NCS_F_PK_PASN_128  =  0x00040,
    NAN_NCS_F_PK_PASN_256  =  0x00080,
} NanCSIDBits;

/**
 * Security configuration rules between Upper layer and NAN target
 * CSID and PMK are optional parameters
 * Target usese the device capability CSID if Service layer provide.
 *
 * For these security parameter host will act as pass through
 * Here are the rules for security parametrs between upper layer and target
 *
 * NDP Request
 *      1. 4 byte CSID
 *      2. 32 byte PMK
 *      3. PASSPHRASE >=8 and <= 63 byte
 *      4. Service Name (<=255)
 *
 * NDP Responder Request
 *      1. 4 byte CSID
 *      2. 32 byte PMK
 *      3. PASSPHRASE >=8 and <= 63 byte
 *      4. Service Name (<=255)
 *
 * NDP Indication
 *      1. 4 byte CSID
 *      2. 256 byte SCID buffer
 */

/* NAN Cipher Version */
typedef enum {
    NAN_CIPHER_128 = 0, /* 0: 128-bit NIK, 64-bit Nonce, 64-bit Tag, HMAC-SHA-256 */
    /* 1 -7 reserved */
} NanCipherVersion;

/* NAN Group MFP support */
#define NAN_GTKSA_IGTKSA_BIGTKSA_NOT_SUPPORTED             0x00
#define NAN_GTKSA_IGTKSA_SUPPORTED_BIGTKSA_NOT_SUPPORTED   0x01
#define NAN_GTKSA_IGTKSA_BIGTKSA_SUPPORTED                 0x02

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32  inner_threshold;
    A_UINT32  outer_threshold;
} t_nan_geo_fence_descriptor, *tp_nan_geo_fence_descriptor;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 range_resolution;
    A_UINT32 range_interval;
    A_UINT32 ranging_indication_event;
    t_nan_geo_fence_descriptor geo_gence_threshold;

} t_nan_range_config_params, *tp_nan_range_config_params;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 range_measurement;
    A_UINT32 event_type;
    A_UINT32 range_id;
} t_nan_range_result_params, *tp_nan_range_result_params;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    /* TLV Required:
       MANDATORY
       1. MAC_ADDRESS
       2. t_nan_range_result_params
    */
    A_UINT8 ptlv[1];
} t_nan_range_result_ind, *tp_nan_range_result_ind;

/* This is the TLV used to trigger ranging requests*/
typedef PACKED_PRE struct PACKED_POST
{
    wlan_nan_mac_addr  range_mac_addr;
    A_UINT32      range_id; // Match handle in match_ind, publish_id in result ind
    A_UINT32      ranging_accept:1;
    A_UINT32      ranging_reject:1;
    A_UINT32      ranging_cancel:1;
    A_UINT32      reserved:29;
} t_nan_range_req_msg, *tp_nan_range_req_msg;

typedef PACKED_PRE struct PACKED_POST
{
    wlan_nan_mac_addr  range_mac_addr;
    A_UINT32      range_id; // This will publish_id in case of receiving publish.
} t_nan_range_req_recvd_msg,*tp_nan_range_req_recvd_msg;

typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    /*TLV Required
      1. t_nan_range_req_recvd_msg
    */
    A_UINT8 ptlv[1];
}t_nan_range_req_ind, *tp_nan_range_req_ind;

/************************** Test Mode ***************************/

typedef PACKED_PRE struct PACKED_POST
{
    tNanMsgHeader nanHeader;
    /*
     * Excludes TLVs
     *
     * Optional: Nan Availability
     *
     */
    A_UINT8 ptlv[1];
} tNanTestModeReqMsg, *tpNanTestModeReqMsg;

typedef enum {
    NAN_DATA_PATH_M4_RESPONSE_ACCEPT = 1,
    NAN_DATA_PATH_M4_RESPONSE_REJECT = 2,
    NAN_DATA_PATH_M4_RESPONSE_BAD_MIC = 3
} tNdpM4ResponseType;

typedef enum {
    NAN_SCHED_VALID = 0,
    NAN_SCHED_INVALID_BAD_FA = 1,
    NAN_SCHED_INVALID_BAD_NDC = 2,
    NAN_SCHED_INVALID_BAD_IMMU = 3,
} NanSchedType;

typedef enum {
    NMF_CLEAR_DISABLE = 0,
    NMF_CLEAR_ENABLE = 1,
} NMFClearEnable;

typedef enum
{
    NAN_TEST_MODE_CMD_NAN_AVAILABILITY = 1,
    NAN_TEST_MODE_CMD_NDP_INCLUDE_IMMUTABLE = 2,
    NAN_TEST_MODE_CMD_NDP_AVOID_CHANNEL = 3,
    NAN_TEST_MODE_CMD_NAN_SUPPORTED_BANDS = 4,
    NAN_TEST_MODE_CMD_AUTO_RESPONDER_MODE = 5,
    NAN_TEST_MODE_CMD_M4_RESPONSE_TYPE = 6,
    NAN_TEST_MODE_CMD_NAN_SCHED_TYPE =7,
    NAN_TEST_MODE_CMD_NMF_CLEAR_CONFIG =8,
    NAN_TEST_MODE_CMD_NAN_SCHED_UPDATE_ULW_NOTIFY = 9,
    NAN_TEST_MODE_CMD_NAN_SCHED_UPDATE_NDL_NEGOTIATE = 10,
    NAN_TEST_MODE_CMD_NAN_SCHED_UPDATE_NDL_NOTIFY = 11,
    NAN_TEST_MODE_CMD_NAN_AVAILABILITY_MAP_ORDER = 12,
    NAN_TEST_MODE_CMD_NDL_QOS_TEST = 13, //CONFIG_QoS
    NAN_TEST_MODE_CMD_DEVICE_TYPE = 14,
    NAN_TEST_MODE_CMD_DISABLE_NDPE = 15,
    NAN_TEST_MODE_CMD_ENABLE_NDP = 16,
    NAN_TEST_MODE_CMD_DISABLE_IPV6_LINK_LOCAL = 17,
    NAN_TEST_MODE_CMD_TRANSPORT_IP_PARAM = 18,
    NAN_TEST_MODE_CMD_S3_ATTR_PARAMS = 19,
    NAN_TEST_MODE_CMD_SCHED_UPDATE_S3_NOTIFY = 20,
    NAN_TEST_MODE_CMD_PMK = 21,
} tNanTestModeCmd;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 cmd;
    /* Followed by command data content (Aligned to A_UINT32 size)
     *
     * command: NAN_TEST_MODE_CMD_NAN_AVAILABILITY
     * content: NAN Avaiability attribute blob
     *
     * command: NAN_TEST_MODE_CMD_NDP_INCLUDE_IMMUTABLE
     * content: A_UINT32 value (0 - Ignore 1 - Include immuatable, 2 - Don't include immutable)
     *
     * command: NAN_TEST_MODE_CMD_NDP_AVOID_CHANNEL
     * content: A_UINT32 channel_frequency; (0 - Ignore)
     *
     * command: NAN_TEST_MODE_CMD_NAN_SCHED_TYPE
     * content: A_UINT32 value; (0 - valid schedule(default) 1 - Bad FA, 2 send bad NDC, 3 - Bad immuatable)
     *
     * command: NAN_TEST_MODE_CMD_NMF_CLEAR_CONFIG
     * content: A_UINT32 value; (0 - Send NMF Encrypted(default), 1 - Send Clear NMF)
     *
     * command: NAN_TEST_MODE_CMD_NAN_SCHED_UPDATE_ULW_NOTIFY
     * content: A_UINT32 channelAvailabilityPresent; (0 - channel availability not present, 1 - channel availability present)
     * content: A_UINT32 channelAvailabilityValue; (0 - channel not available, 1 - channel available)
     *
     * command: NAN_TEST_MODE_CMD_DEVICE_TYPE
     * content: A_UINT32 deviceType; (0 - ignore, 1 - TestBed, 2 - DUT)
     *
     * command: NAN_TEST_MODE_CMD_DISABLE_NDPE
     * content: A_UINT32 disableNDPE; (0 - Send NDPE based on NDPE parameters present, 1 - TestBed(Dont include the NDPE attribute))
     *
     * command: NAN_TEST_MODE_CMD_ENABLE_NDP
     * content: A_UINT32 enableNDP; (0 - Dont send the NDP attribute if we send the NDPE attribute,
     *                              (1 - TestBed(Include the NDP attribute irrespective of the NDP attribute present or not))
     *
     * command: NAN_TEST_MODE_CMD_DISABLE_IPV6_LINK_LOCAL
     * content: A_UINT32 disableIPv6_linklocal; (0 - Send IPv6 link local based on IPv6 support, 1 - TestBed(Dont include the IPv6 TLV list))
     *
     * command: NAN_TEST_MODE_CMD_TRANSPORT_IP_PARAM
     * content: struct tNdpTransIpParams
     *
     * command: NAN_TEST_MODE_CMD_S3_ATTR_PARAMS
     * content: struct tNanS3Params
     *
     * command: NAN_TEST_MODE_CMD_SCHED_UPDATE_S3_NOTIFY
     * content: struct tNanS3Params
     */
    A_UINT8 data[1];
} t_nan_test_mode_cmd_params;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 dfs_master:1;
    A_UINT32 ext_key_id:1;
    A_UINT32 simu_ndp_data_recept:1;
    A_UINT32 ndpe_attr_supp:1;
    A_UINT32 reserved:28;
} tNanDevCapAttrCap, *tpNanDevCapAttrCap;

/***************************************************************/

#define NAN_MSG_IPV6_INTF_ADDR_LEN    16

typedef struct {
    /* Presence of ipv6_intf_addr */
    A_UINT32 ipv6_addr_present;
    /* Presence of transport Port */
    A_UINT32 trans_port_present;
    /* Presence of  transport Protocol */
    A_UINT32 trans_proto_present;
    /* ipv6 Interface address */
    A_UINT8  ipv6_intf_addr[NAN_MSG_IPV6_INTF_ADDR_LEN];
    /* Transport Port */
    A_UINT32 transport_port;
    /* Transport Protocol */
    A_UINT32 transport_protocol;
} tNdpTransIpParams;

typedef enum {
    NAN_BOOTSTRAPPING_METHOD_PIN_CODE_DISPLAY = 1,
    NAN_BOOTSTRAPPING_METHOD_PASSPHRASE_DISPLAY = 2,
    NAN_BOOTSTRAPPING_METHOD_QR_CODE_DISPLAY = 3,
    NAN_BOOTSTRAPPING_METHOD_NFC_TAG = 4,
    NAN_BOOTSTRAPPING_METHOD_KEYPAD_PIN_CODE = 5,
    NAN_BOOTSTRAPPING_METHOD_KEYPAD_PASSPHRASE = 6,
    NAN_BOOTSTRAPPING_METHOD_QR_CODE_SCAN = 7,
    NAN_BOOTSTRAPPING_METHOD_NFC_READER = 8,
    /* Bit 9 to 13 reserved */
    NAN_BOOTSTRAPPING_METHOD_SERVICE_MANAGED_BOOTSTRAPPING = 14,
    NAN_BOOTSTRAPPING_METHOD_BOOTSTRAPPING_HANDSHAKE_SKIPPED = 15,
} tNaNBootstrappingMethod;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 pairing_setup_required:1;
    A_UINT32 npk_nik_caching_required:1;
    A_UINT32 bootstapping_method_bitmap:16;
    A_UINT32 reserved:14;
} tNanPairingConfigurationParams, *tpNanPairingConfigurationParams;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 pairing_setup_required:1;
    A_UINT32 npk_nik_caching_required:1;
    A_UINT32 bootstapping_method_bitmap:16;
    A_UINT32 reserved:14;
} tNanPairingParamsMatchTlv, *tpNanPairingParamsMatchTlv;

typedef PACKED_PRE struct PACKED_POST
{
    A_UINT8 type;
    A_UINT8 status;
    A_UINT8 dialog_token;
    A_UINT8 reason_code;
    A_UINT16 bootstapping_method_bitmap;
    A_UINT16 comeback_after;
} tNanBootstrappingParams, *tpNanBootstrappingParams;

/* NAN Identity Resolution Params */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 cipher_version:8;
    A_UINT32 reserved:24;
} tNanIdentityResolutionParams;

/* NAN Identity Resolution Indication : HAL -> Target */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER                   halHeader;
    tNanMsgHeader                   nanHeader;
    tNanIdentityResolutionParams   identityresolutionParams;
    /*
     * Excludes TLVs
     *
     * Required: Nounce, Tag
     */
    A_UINT8 ptlv[1];
} tNanIdentityResolutionIndMsg, *tpNanIdentityResolutionIndMsg;

/* NAN pairing roles */
#define NAN_PAIRING_ROLE_INITIATOR 0
#define NAN_PAIRING_ROLE_RESPONDER 1

/* NAN Pairing Request Params */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 pairing_handle;
    A_UINT32 pairing_role:1;
    A_UINT32 pairing_verification:1;
    A_UINT32 cipher_suite:8;
    A_UINT32 reserved:22;
    A_UINT32 reserved2;
    /* NOTE:
     * This struct cannot be expanded, due to backwards-compatibility
     * requirements.
     */
} tNanPairingIndParams;

/* NAN Pairing Indication : HAL -> Target */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    tNanPairingIndParams        pairingIndParams;
    A_UINT32 reserved[2];
    /* TLVs Required:
     * MANDATORY
     * 1. MAC_ADDRESS (Peer NMI)
     * 2. NM_TK (The TK derived from pairing)
     */
    A_UINT8 ptlv[1];
    /* NOTE:
     * This struct cannot be expanded, due to the above variable-length array.
     */
} tNanPairingIndMsg, *tpNanPairingIndMsg;

/* NAN UnPairing Indication : HAL -> Target */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    A_UINT32                    pairing_handle;
} tNanUnPairingIndMsg, *tpNanUnPairingIndMsg;

/* NAN OEM REQ */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    /*
     * TLVs:
     *
     * Required: OEM request in the form of opaque data blob.
     */
    A_UINT8 ptlv[1];
    /* NOTE:
     * This struct cannot be expanded, due to the above variable-length array.
     */
} tNanOemReqMsg, *tpNanOemReqMsg;

/* NAN OEM RSP */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    A_UINT16 status;
    A_UINT16 value;
    /*
     * TLVs:
     *
     * Required: OEM response in the form of opaque data blob.
     */
    A_UINT8 ptlv[1];
    /* NOTE:
     * This struct cannot be expanded, due to the above variable-length array.
     */
} tNanOemRspMsg, *tpNanOemRspMsg;

/* NAN OEM IND */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    A_UINT16 reserved[2];
    /*
     * TLVs:
     *
     * Required: OEM indication in the form of opaque data blob.
     */
    A_UINT8 ptlv[1];
    /* NOTE:
     * This struct cannot be expanded, due to the above variable-length array.
     */
} tNanOemIndMsg, *tpNanOemIndMsg;

typedef struct {
    A_UINT32 entry_control:8;
    A_UINT32 time_bitmap_control:16;
    A_UINT32 reserved:8;
    A_UINT32 time_bitmap;
} tNanS3Params, *tpNanS3Params;

#define NAN_MAX_GROUP_KEY_LEN 32
#define NAN_MAX_GROUP_KEY_RSC_LEN 6

/* NAN Group Key params */
typedef PACKED_PRE struct PACKED_POST
{
    A_UINT32 key_cipher:16;
    A_UINT32 key_idx:8;
    A_UINT32 key_len:8;
    A_UINT8  key_data[NAN_MAX_GROUP_KEY_LEN];
    A_UINT8  key_rsc[NAN_MAX_GROUP_KEY_RSC_LEN];
} tNanGroupKeyParamsTlv, *tpNanGroupKeyParamsTlv;

/* NAN Group Key Install Req Msg */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;

    /*
     * TLVs
     *
     * Required: MAC address, one or more tNanGroupKeyParamsTlv.
     */
    A_UINT8 ptlv[1];
    /* NOTE:
     * This struct cannot be expanded, due to the above variable-length array.
     */
} tNanGroupKeyInstallReqMsg, *tpNanGroupKeyInstallReqMsg;

/* NAN Group Key Install Rsp Msg */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status; /* tNanStatusType */
    A_UINT16 value;
} tNanGroupKeyInstallRspMsg, *tpNanGroupKeyInstallRspMsg;

/* NAN Group Key TX PN fetch Req Msg */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER     halHeader;
    tNanMsgHeader               nanHeader;
    A_UINT32                    key_idx;
} tNanTxPnReqMsg, *tpNanTxPnReqMsg;

/* NAN Group Key TX PN fetch Rsp Msg */
typedef PACKED_PRE struct PACKED_POST
{
    NAN_PLATFORM_MSG_HEADER   halHeader;
    tNanMsgHeader   nanHeader;
    A_UINT16 status; /* tNanStatusType */
    A_UINT16 value;
    A_UINT8  key_rsc[NAN_MAX_GROUP_KEY_RSC_LEN];
} tNanTxPnRspMsg, *tpNanTxPnRspMsg;


#endif /* WLAN_NAN_MSG_H */
