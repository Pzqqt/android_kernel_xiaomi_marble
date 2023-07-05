/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _WLAN_NAN_MSG_COMMON_V2_H_
#define _WLAN_NAN_MSG_COMMON_V2_H_
/*
 * WLAN NAN CONSTANTS
 */
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

/* NAN message IDs */
typedef enum {
    NAN_MSG_ID_ERROR_RSP                    = 0,
    NAN_MSG_ID_CONFIGURATION_REQ            = 1,
    NAN_MSG_ID_CONFIGURATION_RSP            = 2,
    NAN_MSG_ID_PUBLISH_SERVICE_REQ          = 3,
    NAN_MSG_ID_PUBLISH_SERVICE_RSP          = 4,
    NAN_MSG_ID_PUBLISH_SERVICE_CANCEL_REQ   = 5,
    NAN_MSG_ID_PUBLISH_SERVICE_CANCEL_RSP   = 6,
    NAN_MSG_ID_PUBLISH_REPLIED_IND          = 7,
    NAN_MSG_ID_PUBLISH_TERMINATED_IND       = 8,
    NAN_MSG_ID_SUBSCRIBE_SERVICE_REQ        = 9,
    NAN_MSG_ID_SUBSCRIBE_SERVICE_RSP        = 10,
    NAN_MSG_ID_SUBSCRIBE_SERVICE_CANCEL_REQ = 11,
    NAN_MSG_ID_SUBSCRIBE_SERVICE_CANCEL_RSP = 12,
    NAN_MSG_ID_MATCH_IND                    = 13,
    NAN_MSG_ID_UNMATCH_IND                  = 14,
    NAN_MSG_ID_SUBSCRIBE_TERMINATED_IND     = 15,
    NAN_MSG_ID_DE_EVENT_IND                 = 16,
    NAN_MSG_ID_TRANSMIT_FOLLOWUP_REQ        = 17,
    NAN_MSG_ID_TRANSMIT_FOLLOWUP_RSP        = 18,
    NAN_MSG_ID_FOLLOWUP_IND                 = 19,
    NAN_MSG_ID_STATS_REQ                    = 20,
    NAN_MSG_ID_STATS_RSP                    = 21,
    NAN_MSG_ID_ENABLE_REQ                   = 22,
    NAN_MSG_ID_ENABLE_RSP                   = 23,
    NAN_MSG_ID_DISABLE_REQ                  = 24,
    NAN_MSG_ID_DISABLE_RSP                  = 25,
    NAN_MSG_ID_DISABLE_IND                  = 26,
    NAN_MSG_ID_TCA_REQ                      = 27,
    NAN_MSG_ID_TCA_RSP                      = 28,
    NAN_MSG_ID_TCA_IND                      = 29,
    NAN_MSG_ID_BEACON_SDF_REQ               = 30,
    NAN_MSG_ID_BEACON_SDF_RSP               = 31,
    NAN_MSG_ID_BEACON_SDF_IND               = 32,
    NAN_MSG_ID_CAPABILITIES_REQ             = 33,
    NAN_MSG_ID_CAPABILITIES_RSP             = 34,
    NAN_MSG_ID_SELF_TRANSMIT_FOLLOWUP_IND   = 35,
    NAN_MSG_ID_RANGING_REQUEST_IND          = 36,
    NAN_MSG_ID_RANGING_RESULT_IND           = 37,
    /* Add regular/non-test mode messages here */
    NAN_MSG_ID_TESTMODE_REQ                 = 1025,
} nan_msg_id_t;

/* 8-byte control message header used by NAN */
typedef PACKED_PRE struct PACKED_POST {
    A_UINT16 msg_version:4;
    A_UINT16 msg_id:12;
    A_UINT16 msg_len;
    A_UINT16 handle;
    A_UINT16 transaction_id;
} nan_msg_header_t, *nan_msg_header_tp;

#endif /* WLAN_NAN_MSG_COMMON_V2_H */
