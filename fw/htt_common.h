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

/**
 * @file htt_common.h
 *
 * @details the public header file of HTT layer shared between host and firmware
 */

#ifndef _HTT_COMMON_H_
#define _HTT_COMMON_H_

enum htt_sec_type {
    htt_sec_type_none,
    htt_sec_type_wep128,
    htt_sec_type_wep104,
    htt_sec_type_wep40,
    htt_sec_type_tkip,
    htt_sec_type_tkip_nomic,
    htt_sec_type_aes_ccmp,
    htt_sec_type_wapi,
    htt_sec_type_aes_ccmp_256,
    htt_sec_type_aes_gcmp,
    htt_sec_type_aes_gcmp_256,

    /* keep this last! */
    htt_num_sec_types
};

enum htt_rx_ind_mpdu_status {
    HTT_RX_IND_MPDU_STATUS_UNKNOWN = 0x0,
    HTT_RX_IND_MPDU_STATUS_OK,
    HTT_RX_IND_MPDU_STATUS_ERR_FCS,
    HTT_RX_IND_MPDU_STATUS_ERR_DUP,
    HTT_RX_IND_MPDU_STATUS_ERR_REPLAY,
    HTT_RX_IND_MPDU_STATUS_ERR_INV_PEER,
    HTT_RX_IND_MPDU_STATUS_UNAUTH_PEER, /* only accept EAPOL frames */
    HTT_RX_IND_MPDU_STATUS_OUT_OF_SYNC,
    HTT_RX_IND_MPDU_STATUS_MGMT_CTRL, /* Non-data in promiscous mode */
    HTT_RX_IND_MPDU_STATUS_TKIP_MIC_ERR,
    HTT_RX_IND_MPDU_STATUS_DECRYPT_ERR,
    HTT_RX_IND_MPDU_STATUS_MPDU_LENGTH_ERR,
    HTT_RX_IND_MPDU_STATUS_ENCRYPT_REQUIRED_ERR,
    HTT_RX_IND_MPDU_STATUS_PRIVACY_ERR,

    /*
     * MISC: discard for unspecified reasons.
     * Leave this enum value last.
     */
    HTT_RX_IND_MPDU_STATUS_ERR_MISC = 0xFF
};

#define HTT_INVALID_PEER    0xffff
#define HTT_INVALID_VDEV    0xff

#define HTT_NON_QOS_TID     16
#define HTT_INVALID_TID     31

#define HTT_TX_EXT_TID_DEFAULT              0
#define HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST HTT_NON_QOS_TID
#define HTT_TX_EXT_TID_MGMT                17
#define HTT_TX_EXT_TID_INVALID             HTT_INVALID_TID
#define HTT_TX_EXT_TID_NONPAUSE            19



#define HTT_TX_L3_CKSUM_OFFLOAD      1
#define HTT_TX_L4_CKSUM_OFFLOAD      2


/**
 * @brief General specification of the tx frame contents
 *
 * @details
 * For efficiency, the HTT packet type values correspond
 * to the bit positions of the WAL packet type values, so the
 * translation is a simple shift operation.
 * The exception is the "mgmt" type, which specifies frame payload
 * type rather than L2 header type.
 */
enum htt_pkt_type {
    htt_pkt_type_raw = 0,
    htt_pkt_type_native_wifi = 1,
    htt_pkt_type_ethernet = 2,
    htt_pkt_type_mgmt = 3,
    htt_pkt_type_eth2 = 4,

    /* keep this last */
    htt_pkt_num_types
};

/*
 * TX MSDU ID partition -
 * FW supports bigger MSDU ID partition which is defined as
 * HTT_TX_IPA_NEW_MSDU_ID_SPACE_BEGIN
 * When both host and FW support new partition, FW uses
 * HTT_TX_IPA_NEW_MSDU_ID_SPACE_BEGIN
 * If host doesn't support, FW falls back to HTT_TX_IPA_MSDU_ID_SPACE_BEGIN
 * Handshaking is done through WMI_READY and WMI_INIT
 */
#define HTT_TX_HOST_MSDU_ID_SPACE_BEGIN 0
#define HTT_TX_IPA_MSDU_ID_SPACE_BEGIN  3000
#define TGT_RX2TX_MSDU_ID_SPACE_BEGIN 6000
#define HTT_TX_IPA_NEW_MSDU_ID_SPACE_BEGIN  8192  /* = 0x2000 = b10,0000,0000,0000 */
#define TGT_RX2TX_NEW_MSDU_ID_SPACE_BEGIN   12288 /* = 0x3000 = b11,0000,0000,0000 */

#endif /* _HTT_COMMON_H_ */
