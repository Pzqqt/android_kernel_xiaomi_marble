/*
 * Copyright (c) 2011-2017 The Linux Foundation. All rights reserved.
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
 * @file wdi_ipa.h
 *
 * @details WIFI <-> IPA direct data-path interface related definitions.
 */

#ifndef _WDI_IPA_H_
#define _WDI_IPA_H_

#if defined(ATH_TARGET)
#include <osapi.h>      /* A_UINT32 */

#else

#if defined(IPA_TARGET)
typedef unsigned int  A_UINT32
typedef unsigned char A_UINT8
#define A_ASSERT(__bool) \
  do {  \
     if (0 == (__bool)) { \
        while(1); \
     } \
  } while(0)
#define A_COMPILE_TIME_ASSERT(assertion_name, predicate) \
    typedef char assertion_name[(predicate) ? 1 : -1];
#define PREPACK
#define POSTPACK

#else

#include <a_types.h>    /* A_UINT32 */
#include <a_osapi.h>    /* PREPACK, POSTPACK */

#endif /* IPA_TARGET */

#endif /* ATH_TARGET */


#ifndef offsetof
#define offsetof(type, field)   ((unsigned int)(&((type *)0)->field))
#endif

/*
 * WDI_IPA version history:
 * 1.0 initial numbered version
 */
#define WDI_IPA_CURRENT_VERSION_MAJOR 1
#define WDI_IPA_CURRENT_VERSION_MINOR 1


#define WDI_IPA_CHECK_SET_VAL(field, val) \
    A_ASSERT(!((val) & ~((field ## _M) >> (field ## _S))))




/**
 * @brief WDI_IPA TX_FRAG_DESC
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                           frag buf ptr                            |
 *     |-------------------------------------------------------------------|
 *     |            Reserved             |             frag len            |
 *     |-------------------------------------------------------------------|
 *     |                                 0                                 |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 * Header fields:
 *   - FRAG_BUF_PTR
 *     Bits 31:0
 *     Purpose: Specify pointer to the starting of Ethernet Packet
 *   - FRAG_LEN
 *     Bits 15:0
 *     Purpose: Specify total length of the Ethernet Packet.
 *   - 0
 *     Bits 31:0
 *     Purpose: fixed value 0, it is used for indicating WIFI HW
 *         that Ethernet Packet is contained in ONE fragment.
 *
 */

#define WDI_IPA_TX_FRAG_DESC_SZ                          12 /* bytes */

#define WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_M              0xffffffff
#define WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_S              0

#define WDI_IPA_TX_FRAG_DESC_FRAG_LEN_M                  0x0000ffff
#define WDI_IPA_TX_FRAG_DESC_FRAG_LEN_S                  0

#define WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_GET(_var)        \
        (((_var) & WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_M) >> \
         WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_S)
#define WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR, _val);  \
            ((_var) &= (WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_FRAG_DESC_FRAG_BUF_PTR_S)); \         } while(0)

#define WDI_IPA_TX_FRAG_DESC_FRAG_LEN_GET(_var)        \
        (((_var) & WDI_IPA_TX_FRAG_DESC_FRAG_LEN_M) >> \
         WDI_IPA_TX_FRAG_DESC_FRAG_LEN_S)
#define WDI_IPA_TX_FRAG_DESC_FRAG_LEN_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_FRAG_DESC_FRAG_LEN, _val);  \
            ((_var) &= (WDI_IPA_TX_FRAG_DESC_FRAG_LEN_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_FRAG_DESC_FRAG_LEN_S)); \             } while(0)

PREPACK struct wdi_ipa_tx_frag_desc_t
{
    A_UINT32 frag_buf_ptr; /* word 0 */

    A_UINT32 /* word 1 */
        frag_len:   16,
        reserved:   16;

    A_UINT32 fixed_val_zero; /* word 2 */
} POSTPACK;



/**
 * @brief WDI_IPA TX_HTT_DESC is same as HTT TX Msdu descriptor (defined in
 *        htt.h). TX_HTT_DESC fields are opaque to IPA uC, so we just need
 *        to define size of the descriptor in wdi_ipa.h. Host WLAN driver
 *        and WIFI FW uses htt.h for the bit-field definitions.
 *
 */
#define WDI_IPA_TX_HTT_DESC_SZ                         16 /* bytes */



/**
 * @brief WDI_IPA TX_IPA_DESC
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                       Reserved                   |     vdev id    |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 * Header fields:
 *   - VDEV_ID
 *     Bits 23:16
 *     Purpose: Specifies the SoftAP VDEV interface, pertaining to the packet
 *         filled by IPA HW.
 */

#define WDI_IPA_TX_IPA_DESC_SZ                        4 /* bytes */

#define WDI_IPA_TX_IPA_DESC_VDEV_ID_M                 0x000000ff
#define WDI_IPA_TX_IPA_DESC_VDEV_ID_S                 0

#define WDI_IPA_TX_IPA_DESC_VDEV_ID_GET(_var)        \
        (((_var) & WDI_IPA_TX_IPA_DESC_VDEV_ID_M) >> \
         WDI_IPA_TX_IPA_DESC_VDEV_ID_S)
#define WDI_IPA_TX_IPA_DESC_VDEV_ID_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_IPA_DESC_VDEV_ID, _val);  \
            ((_var) &= (WDI_IPA_TX_IPA_DESC_VDEV_ID_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_IPA_DESC_VDEV_ID_S)); \
        } while(0)

PREPACK struct wdi_ipa_tx_ipa_desc_t
{
    A_UINT32 /* word 0 */
        vdev_id:      8,
        reserved:     24;
} POSTPACK;


#define WDI_IPA_TX_ETHR_PKT_MAX_SZ                  1518 /* bytes */

PREPACK struct wdi_ipa_tx_pkt_buf_t
{
    A_UINT8 htt_desc[WDI_IPA_TX_HTT_DESC_SZ];
    struct wdi_ipa_tx_frag_desc_t frag_desc;
    struct wdi_ipa_tx_ipa_desc_t ipa_desc;
    A_UINT8 ethr_pkt[WDI_IPA_TX_ETHR_PKT_MAX_SZ];
} POSTPACK;

#define WDI_IPA_TX_PKT_BUF_MAX_SZ                   2048 /* bytes */
A_COMPILE_TIME_ASSERT(verify_pkt_buf_sz,
       sizeof(struct wdi_ipa_tx_pkt_buf_t)<=WDI_IPA_TX_PKT_BUF_MAX_SZ);
#define WDI_IPA_TX_PKT_POOL_SZ                      1023 /* packets */



/**
 * @brief WDI_IPA TX_CE_DESC
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                              src ptr                              |
 *     |-------------------------------------------------------------------|
 *     |           meta data         |S|G|             src buf len         |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 * Header fields:
 *   - SRC_PTR
 *     Bits 31:0
 *     Purpose: Specify pointer to the starting of Pacet Buf
 *   - SRC_BUF_LEN
 *     Bits 15:0
 *     Purpose: Specify number of bytes to be copied by Copy Engine
 *   - G
 *     Bits 16
 *     Purpose: Enable gather of multiple CE descriptors to one destination
 *        Always 0 (disable) for WDI_IPA data path
 *   - S
 *     Bits 17
 *     Purpose: Enable byte swap for endian conversation.
 *         Always 0 (disable) for WDI_IPA data path.
 *     META_DATA
 *     Bits 31:18
 *     Purpose: unused
 */

#define WDI_IPA_TX_CE_DESC_SZ                          8 /* bytes */

#define WDI_IPA_TX_CE_DESC_SRC_PTR_M                   0xffffffff
#define WDI_IPA_TX_CE_DESC_SRC_PTR_S                   0

#define WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_M               0x0000ffff
#define WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_S               0

#define WDI_IPA_TX_CE_DESC_G_M                         0x00010000
#define WDI_IPA_TX_CE_DESC_G_S                         16

#define WDI_IPA_TX_CE_DESC_S_M                         0x00020000
#define WDI_IPA_TX_CE_DESC_S_S                         17

#define WDI_IPA_TX_CE_DESC_META_DATA_M                 0xFFFC0000
#define WDI_IPA_TX_CE_DESC_META_DATA_S                 18

#define WDI_IPA_TX_CE_DESC_SRC_PTR_GET(_var)        \
        (((_var) & WDI_IPA_TX_CE_DESC_SRC_PTR_M) >> \
         WDI_IPA_TX_CE_DESC_SRC_PTR_S)
#define WDI_IPA_TX_CE_DESC_SRC_PTR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_CE_DESC_SRC_PTR, _val);  \
            ((_var) &= (WDI_IPA_TX_CE_DESC_SRC_PTR_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_CE_DESC_SRC_PTR_S)); \
        } while(0)

#define WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_GET(_var)        \
        (((_var) & WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_M) >> \
         WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_S)
#define WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_CE_DESC_SRC_BUF_LEN, _val);  \
            ((_var) &= (WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_CE_DESC_SRC_BUF_LEN_S)); \
        } while(0)

#define WDI_IPA_TX_CE_DESC_G_GET(_var)        \
        (((_var) & WDI_IPA_TX_CE_DESC_G_M) >> \
         WDI_IPA_TX_CE_DESC_G_S)
#define WDI_IPA_TX_CE_DESC_G_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_CE_DESC_G, _val);  \
            ((_var) &= (WDI_IPA_TX_CE_DESC_G_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_CE_DESC_G_S)); \
        } while(0)

#define WDI_IPA_TX_CE_DESC_S_GET(_var)        \
        (((_var) & WDI_IPA_TX_CE_DESC_S_M) >> \
         WDI_IPA_TX_CE_DESC_S_S)
#define WDI_IPA_TX_CE_DESC_S_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_CE_DESC_S, _val);  \
            ((_var) &= (WDI_IPA_TX_CE_DESC_S_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_CE_DESC_S_S)); \
        } while(0)

#define WDI_IPA_TX_CE_DESC_META_DATA_GET(_var)        \
        (((_var) & WDI_IPA_TX_CE_DESC_META_DATA_M) >> \
         WDI_IPA_TX_CE_DESC_META_DATA_S)
#define WDI_IPA_TX_CE_DESC_META_DATA_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_CE_DESC_META_DATA, _val);  \
            ((_var) &= (WDI_IPA_TX_CE_DESC_META_DATA_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_CE_DESC_META_DATA_S)); \
        } while(0)

PREPACK struct wdi_ipa_tx_ce_desc_t
{
    A_UINT32 src_ptr; /* word 0 */
    A_UINT32          /* word 1 */
        src_buf_len:   16,
        g:             1,
        s:             1,
        meta_data:     14;
} POSTPACK;


#define WDI_IPA_TX_CE_RING_SZ                       1024 /* no. of desc */
#define WDI_IPA_TX_INITIAL_PKT_HDR_SZ  28
#define WDI_IPA_TX_CE_FIXED_BUF_LEN    (WDI_IPA_TX_FRAG_DESC_SZ + \
                                        WDI_IPA_TX_HTT_DESC_SZ + \
                                        WDI_IPA_TX_IPA_DESC_SZ + \
                                        WDI_IPA_TX_INITIAL_PKT_HDR_SZ)


/* Verify that CE ring size is power of 2 */
A_COMPILE_TIME_ASSERT(verify_ce_ring_sz,
      ((WDI_IPA_TX_CE_RING_SZ) ^ (WDI_IPA_TX_CE_RING_SZ-1)) ==
      ((WDI_IPA_TX_CE_RING_SZ<<1)-1));

A_COMPILE_TIME_ASSERT(verify_pkt_pool_sz,
      WDI_IPA_TX_PKT_POOL_SZ < WDI_IPA_TX_CE_RING_SZ);


/**
 * @brief WDI_IPA TX_COMP_DESC
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                          Packet Buf Ptr                           |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 * Header fields:
 *   - PKT_BUF_PTR
 *     Bits 31:0
 *     Purpose: Specify pointer to Packet Buf, which is being freed
 *         after TX comp
 */


#define WDI_IPA_TX_COMP_DESC_SZ                          4 /* bytes */

#define WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_M               0xffffffff
#define WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_S               0

#define WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_GET(_var)        \
        (((_var) & WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_M) >> \
         WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_S)
#define WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR, _val);\
            ((_var) &= (WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_M)); \
            ((_var) |= ((_val) << WDI_IPA_TX_COMP_DESC_PKT_BUF_PTR_S)); \
        } while(0)

PREPACK struct wdi_ipa_tx_comp_desc_t
{
    A_UINT32 pkt_buf_ptr; /* word 0 */
} POSTPACK;

#define WDI_IPA_TX_COMP_RING_SZ                          1024 /* no. of desc */
/* Verify that TX COMP ring size is power of 2 */
A_COMPILE_TIME_ASSERT(verify_tx_comp_ring_sz,
      ((WDI_IPA_TX_COMP_RING_SZ) ^ (WDI_IPA_TX_COMP_RING_SZ-1)) ==
      ((WDI_IPA_TX_COMP_RING_SZ<<1)-1));

A_COMPILE_TIME_ASSERT(verify_Pkt_pool_sz,
      WDI_IPA_TX_PKT_POOL_SZ < WDI_IPA_TX_COMP_RING_SZ);



/**
 * @brief WDI_IPA RX_IND_DESC
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                             Packet Ptr                            |
 *     |-------------------------------------------------------------------|
 *     |    Reserved    |    vdev id     |            Packet Length        |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 * Header fields:
 *   - PKT_PTR
 *     Bits 31:0
 *     Purpose: Specify pointer to starting of Ethernet Packet
 *   - PKT_LENGTH
 *     Bits 15:0
 *     Purpose: Specify length Ethernet Packet
 *   - VDEV_ID
 *     Bits 23:16
 *     Purpose: Specify the VDEV ID corrsponding the the packet
 */


#define WDI_IPA_RX_IND_DESC_SZ                      8 /* bytes */

#define WDI_IPA_RX_IND_DESC_PKT_PTR_M               0xffffffff
#define WDI_IPA_RX_IND_DESC_PKT_PTR_S               0

#define WDI_IPA_RX_IND_DESC_PKT_LEN_M               0x0000ffff
#define WDI_IPA_RX_IND_DESC_PKT_LEN_S               0

#define WDI_IPA_RX_IND_DESC_VDEV_ID_M               0x00ff0000
#define WDI_IPA_RX_IND_DESC_VDEV_ID_S               16

#define WDI_IPA_RX_IND_DESC_PKT_PTR_GET(_var)        \
        (((_var) & WDI_IPA_RX_IND_DESC_PKT_PTR_M) >> \
         WDI_IPA_RX_IND_DESC_PKT_PTR_S)
#define WDI_IPA_RX_IND_DESC_PKT_PTR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_RX_IND_DESC_PKT_PTR, _val);\
            ((_var) &= (WDI_IPA_RX_IND_DESC_PKT_PTR_M)); \
            ((_var) |= ((_val) << WDI_IPA_RX_IND_DESC_PKT_PTR_S)); \
        } while(0)

#define WDI_IPA_RX_IND_DESC_PKT_LEN_GET(_var)        \
        (((_var) & WDI_IPA_RX_IND_DESC_PKT_LEN_M) >> \
         WDI_IPA_RX_IND_DESC_PKT_LEN_S)
#define WDI_IPA_RX_IND_DESC_PKT_LEN_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_RX_IND_DESC_PKT_LEN, _val);\
            ((_var) &= (WDI_IPA_RX_IND_DESC_PKT_LEN_M)); \
            ((_var) |= ((_val) << WDI_IPA_RX_IND_DESC_PKT_LEN_S)); \
        } while(0)

#define WDI_IPA_RX_IND_DESC_VDEV_ID_GET(_var)        \
        (((_var) & WDI_IPA_RX_IND_DESC_VDEV_ID_M) >> \
         WDI_IPA_RX_IND_DESC_VDEV_ID_S)
#define WDI_IPA_RX_IND_DESC_VDEV_ID_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WDI_IPA_RX_IND_DESC_VDEV_ID, _val);\
            ((_var) &= (WDI_IPA_RX_IND_DESC_VDEV_ID_M)); \
            ((_var) |= ((_val) << WDI_IPA_RX_IND_DESC_VDEV_ID_S)); \
        } while(0)


PREPACK struct wdi_ipa_rx_ind_desc_t
{
    A_UINT32 pkt_ptr; /* word 0 */
    A_UINT32          /* word 1 */
        pkt_len:16,
        vdev_id:8,
        reserved:8;
} POSTPACK;

#define WDI_IPA_RX_IND_RING_SZ                     512 /* no. of desc */
/* Verify that RX IND ring size is power of 2 */
A_COMPILE_TIME_ASSERT(verify_rx_ind_ring_sz,
      ((WDI_IPA_RX_IND_RING_SZ) ^ (WDI_IPA_RX_IND_RING_SZ-1)) ==
      ((WDI_IPA_RX_IND_RING_SZ<<1)-1));

/**
 * @brief WLAN_WDI_IPA_DBG_STATS
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                          tx comp ring base                        |
 *     |-------------------------------------------------------------------|
 *     |                          tx comp ring size                        |
 *     |-------------------------------------------------------------------|
 *     |                       tx comp ring dbell addr                     |
 *     |-------------------------------------------------------------------|
 *     |                    tx comp ring dbell ind val                     |
 *     |-------------------------------------------------------------------|
 *     |                    tx comp ring dbell cached val                  |
 *     |-------------------------------------------------------------------|
 *     |                           tx pkts enqueued                        |
 *     |-------------------------------------------------------------------|
 *     |                        tx pkts completed                          |
 *     |-------------------------------------------------------------------|
 *     |                          tx is suspend                            |
 *     |-------------------------------------------------------------------|
 *     |                            reserved0                              |
 *     |-------------------------------------------------------------------|
 *     |                         rx ind ring base                          |
 *     |-------------------------------------------------------------------|
 *     |                         rx ind ring size                          |
 *     |-------------------------------------------------------------------|
 *     |                     rx ind ring dbell addr                        |
 *     |-------------------------------------------------------------------|
 *     |                    rx ind ring dbell ind val                      |
 *     |-------------------------------------------------------------------|
 *     |                  rx ind ring dbell cached val                     |
 *     |-------------------------------------------------------------------|
 *     |                        rx ind ring rdidx addr                     |
 *     |-------------------------------------------------------------------|
 *     |                      rx ind ring rd idx cached val                |
 *     |-------------------------------------------------------------------|
 *     |                           rx refill idx                           |
 *     |-------------------------------------------------------------------|
 *     |                      rx num pkts indicated                        |
 *     |-------------------------------------------------------------------|
 *     |                         rx buf refilled                           |
 *     |-------------------------------------------------------------------|
 *     |                       rx num ind drop no space                    |
 *     |-------------------------------------------------------------------|
 *     |                       rx num ind drop no buf                      |
 *     |-------------------------------------------------------------------|
 *     |                          rx is suspend                            |
 *     |-------------------------------------------------------------------|
 *     |                            reserved1                              |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 * Header fields:
 *   - TX_COMP_RING_BASE
 *     Bits 31:0
 *     Purpose: Specifies TX_COMP RING base in DDR
 *   - TX_COMP_RING_SIZE
 *     Bits 31:0
 *     Purpose: Specifies TX_COMP RING size in terms of elements
 *   - TX_COMP_RING_DBELL ADDR
 *     Bits 31:0
 *     Purpose: Specifies DBELL ADDR for updating TX COMP RING write index
 *   - TX_COMP_RING_DBELL_IND_VAL
 *     Bits 31:0
 *     Purpose: Specifies latest TX COMP RING write index written to
 *              Dbell address.
 *   - TX_COMP_RING_DBELL_CACHED_VAL
 *     Bits 31:0
 *     Purpose: Specifies WLAN maintained latest TX COMP RING write index
 *   - TX_PKTS_ENQUEUED
 *     Bits 31:0
 *     Purpose: Specifies total number of packets queued for transmission from
 *              WDI_IPA path
 *   - TX_PKTS_COMPLETED
 *     Bits 31:0
 *     Purpose: Specifies total number of packets completed on WDI_IPA path
 *   - TX_IS_SUSPEND
 *     Bits 31:0
 *     Purpose: Specifies whether WDI_IPA TX is suspended or not.
 *   - RX_IND_RING_BASE
 *     Bits 31:0
 *     Purpose: Specifies RX_IND Ring base in DDR.
 *   - RX_IND_RING_SIZE
 *     Bits 31:0
 *     Purpose: Specifies RX_IND Ring size in terms of elements.
 *   - RX_IND_RING_DBELL_ADDR
 *     Bits 31:0
 *     Purpose: Specifies DBELL ADDR for updating RX_IND WR idx.
 *   - RX_IND_RING_DBELL_IND_VAL
 *     Bits 31:0
 *     Purpose: Specifies latest RX IND RING write index written to
 *              Dbell address.
 *   - RX_IND_RING_DBELL_CACHED_VAL
 *     Bits 31:0
 *     Purpose: Specifies WLAN maintained latest RX IND RING write index
 *   - RX_IND_RING_RDIDX_ADDR
 *     Bits 31:0
 *     Purpose: Specifies ADDR where RX_IND Read Index is updated.
 *   - RX_IND_RING_RDIDX_CACHED_VAL
 *     Bits 31:0
 *     Purpose: Specifies WLAN maintained latest RX IND Read index.
 *   - RX_REFILL_IDX
 *     Bits 31:0
 *     Purpose: Specifies IDX upto which buffers are recycled from RX IND ring.
 *   - RX_NUM_PKTS_INDICATED
 *     Bits 31:0
 *     Purpose: Specifies total number of RX pakets indicated on WDI_IPA path.
 *   - RX_NUM_BUF_REFILLED
 *     Bits 31:0
 *     Purpose: Specifies total number of RX buffers recycled on WDI_IPA path.
 *   - RX_NUM_IND_DROPS_NO_SPACE
 *     Bits 31:0
 *     Purpose: Specifies number of times RX packets dropped as
 *              not enough space in RX_IND ring
 *   - RX_NUM_IND_DROPS_NO_BUF
 *     Bits 31:0
 *     Purpose: Specifies number of times RX packets dropped as
 *              not enough buffers available on WLAN
 *   - RX_IS_SUSPEND
 *     Bits 31:0
 *     Purpose: Specifies whether WDI_IPA RX path is in suspend or not
 */

#define WLAN_WDI_IPA_DBG_STATS_SZ                                     92 /* bytes */

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_M                    0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_S                    0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_M                    0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_S                    0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_M              0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_S              0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_M           0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_S           0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_M        0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_S        0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_M                     0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_S                     0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_M                    0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_S                    0x0

#define WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_M                        0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_S                        0x0

#define WLAN_WDI_IPA_DBG_STATS_RESERVED0_M                            0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RESERVED0_S                            0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_M                     0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_S                     0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_M                     0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_S                     0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_M               0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_S               0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_M            0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_S            0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_M         0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_S         0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_M               0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_S               0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_M         0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_S         0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_M                        0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_S                        0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_M                0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_S                0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_M                      0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_S                      0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_M            0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_S            0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_M              0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_S              0x0

#define WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_M                        0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_S                        0x0

#define WLAN_WDI_IPA_DBG_STATS_RESERVED1_M                            0xffffffff
#define WLAN_WDI_IPA_DBG_STATS_RESERVED1_S                            0x0


#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_BASE_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_SIZE_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_ADDR_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_IND_VAL_S                    )); \
        } while(0)


#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_COMP_RING_DBELL_CACHED_VAL_S                    )); \
        } while(0)


#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_PKTS_ENQUEUED_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_PKTS_COMPLETED_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_M) >> \
          WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_S)
#define WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_TX_IS_SUSPEND_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RESERVED0_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RESERVED0_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RESERVED0_S)
#define WLAN_WDI_IPA_DBG_STATS_RESERVED0_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RESERVED0, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RESERVED0_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RESERVED0_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_BASE_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_SIZE_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_ADDR_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_IND_VAL_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_DBELL_CACHED_VAL_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_ADDR_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IND_RING_RDIDX_CACHED_VAL_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_REFILL_IDX_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_NUM_PKTS_INDICATED_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_BUF_REFILLED_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_SPACE_S                    )); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_NUM_IND_DROPS_NO_BUF_S)); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_S)
#define WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RX_IS_SUSPEND_S)); \
        } while(0)

#define WLAN_WDI_IPA_DBG_STATS_RESERVED1_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_DBG_STATS_RESERVED1_M) >> \
          WLAN_WDI_IPA_DBG_STATS_RESERVED1_S)
#define WLAN_WDI_IPA_DBG_STATS_RESERVED1_SET(_var, _val)  \
        do {                                               \
            WDI_IPA_CHECK_SET_VAL(WLAN_WDI_IPA_DBG_STATS_RESERVED1, _val);\
            ((_var) &= (WLAN_WDI_IPA_DBG_STATS_RESERVED1_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_DBG_STATS_RESERVED1_S                    )); \
        } while(0)


PREPACK struct wlan_wdi_ipa_dbg_stats_t
{
    A_UINT32 tx_comp_ring_base;              /* word 0 */
    A_UINT32 tx_comp_ring_size;              /* word 1 */
    A_UINT32 tx_comp_ring_dbell_addr;        /* word 2 */
    A_UINT32 tx_comp_ring_dbell_ind_val;     /* word 3 */
    A_UINT32 tx_comp_ring_dbell_cached_val;  /* word 4 */
    A_UINT32 tx_pkts_enqueued;               /* word 5 */
    A_UINT32 tx_pkts_completed;              /* word 6 */
    A_UINT32 tx_is_suspend;                  /* word 7 */
    A_UINT32 reserved0;                      /* word 8 */

    A_UINT32 rx_ind_ring_base;               /* word 9 */
    A_UINT32 rx_ind_ring_size;               /* word 10 */
    A_UINT32 rx_ind_ring_dbell_addr;         /* word 11 */
    A_UINT32 rx_ind_ring_dbell_ind_val;      /* word 12 */
    A_UINT32 rx_ind_ring_dbell_cached_val;   /* word 13 */
    A_UINT32 rx_ind_ring_rdidx_addr;         /* word 14 */
    A_UINT32 rx_ind_ring_rdidx_cached_val;   /* word 15 */
    A_UINT32 rx_refill_idx;                  /* word 16 */
    A_UINT32 rx_num_pkts_indicated;          /* word 17 */
    A_UINT32 rx_buf_refilled;                /* word 18 */
    A_UINT32 rx_num_ind_drops_no_space;      /* word 19 */
    A_UINT32 rx_num_ind_drops_no_buf;        /* word 20 */
    A_UINT32 rx_is_suspend;                  /* word 21 */
    A_UINT32 reserved1;                      /* word 22 */
} POSTPACK;

/**
 * @brief WLAN_WDI_IPA_GET_SHARING_STATS_REQ
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                reserved                          |  reset_stats   |
 *     |-------------------------------------------------------------------|
 * Header fields:
 *   - RESET_STATS
 *     Bits 7:0
 *     Purpose: when 1, FW clears sharing stats
 *   - RESERVED
 *     Bits 31:8
 *     Purpose: reserved bits
 **/

#define WLAN_WDI_IPA_GET_SHARING_STATS_REQ_SZ       4 /* bytes */

#define WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_M    0x000000ff
#define WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_S    0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_REQ_RESET_STATS_S)); \
        } while(0)

PREPACK struct wlan_wdi_ipa_get_sharing_stats_req_t {
        A_UINT32 reset_stats:8,
             reserved:24;
} POSTPACK;

/*
 * Response-type specific info for HTT_WDI_IPA_OPCODE_GET_SHARING_STATS
 *
 */
/**
 * @brief WLAN_WDI_IPA_GET_SHARING_STATS_RESP
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                        ipv4_rx_packets_lo                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_rx_packets_hi                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_rx_bytes_lo                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_rx_bytes_lo                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_rx_packets_lo                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_rx_packets_hi                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_rx_bytes_lo                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_rx_bytes_hi                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_tx_packets_lo                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_tx_packets_hi                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_tx_bytes_lo                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv4_tx_bytes_lo                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_tx_packets_lo                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_tx_packets_hi                         |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_tx_bytes_lo                           |
 *     |-------------------------------------------------------------------|
 *     |                        ipv6_tx_bytes_hi                           |
 *     |-------------------------------------------------------------------|
 *
 * Header fields:
 *   - ipv4_rx_packets_lo
 *     Bits 31:0
 *     Purpose: number of IPv4 RX packets, low 32-bit
 *   - ipv4_rx_packets_hi
 *     Bits 31:0
 *     Purpose: number of IPv4 RX packets, high 32-bit
 *   - ipv4_rx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv4 RX packets, low 32-bit
 *   - ipv4_rx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv4 RX packets, low 32-bit
 *   - ipv6_rx_packets_lo
 *     Bits 31:0
 *     Purpose: number of IPv6 RX packets, low 32-bit
 *   - ipv6_rx_packets_hi
 *     Bits 31:0
 *     Purpose: number of IPv6 RX packets, high 32-bit
 *   - ipv6_rx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv6 RX packets, low 32-bit
 *   - ipv6_rx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv6 RX packets, low 32-bit
 *   - ipv4_tx_packets_lo
 *     Bits 31:0
 *     Purpose: number of IPv4 TX packets, low 32-bit
 *   - ipv4_tx_packets_hi
 *     Bits 31:0
 *     Purpose: number of IPv4 TX packets, high 32-bit
 *   - ipv4_tx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv4 TX packets, low 32-bit
 *   - ipv4_tx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv4 TX packets, low 32-bit
 *   - ipv6_tx_packets_lo
 *     Bits 31:0
 *     Purpose: number of IPv6 TX packets, low 32-bit
 *   - ipv6_tx_packets_hi
 *     Bits 31:0
 *     Purpose: number of IPv6 TX packets, high 32-bit
 *   - ipv6_tx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv6 TX packets, low 32-bit
 *   - ipv6_tx_bytes_lo
 *     Bits 31:0
 *     Purpose: bytes of IPv6 TX packets, low 32-bit
 *
 **/

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_SZ       64 /* bytes */

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_M   0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_S   0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_M  0xffffffff
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_S  0x0

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_PKTS_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_RX_BYTES_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_PKTS_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_RX_BYTES_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_PKTS_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV4_TX_BYTES_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_PKTS_HI_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_M) >> \
          WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_S)
#define WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_GET_SHARING_STATS_RESP_IPV6_TX_BYTES_HI_S)); \
        } while(0)

PREPACK struct wlan_wdi_ipa_get_sharing_stats_resp_t {
    A_UINT32 ipv4_rx_packets_lo;
    A_UINT32 ipv4_rx_packets_hi;
        A_UINT32 ipv4_rx_bytes_lo;
        A_UINT32 ipv4_rx_bytes_hi;
        A_UINT32 ipv6_rx_packets_lo;
        A_UINT32 ipv6_rx_packets_hi;
        A_UINT32 ipv6_rx_bytes_lo;
        A_UINT32 ipv6_rx_bytes_hi;

        A_UINT32 ipv4_tx_packets_lo;
        A_UINT32 ipv4_tx_packets_hi;
        A_UINT32 ipv4_tx_bytes_lo;
        A_UINT32 ipv4_tx_bytes_hi;
        A_UINT32 ipv6_tx_packets_lo;
        A_UINT32 ipv6_tx_packets_hi;
        A_UINT32 ipv6_tx_bytes_lo;
        A_UINT32 ipv6_tx_bytes_hi;
} POSTPACK;

/**
 * @brief WLAN_WDI_IPA_SET_QUOTA_REQ
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                reserved                          |    set_quota   |
 *     |-------------------------------------------------------------------|
 *     |                            quota_lo                               |
 *     |-------------------------------------------------------------------|
 *     |                            quota_hi                               |
 *     |-------------------------------------------------------------------|
 * Header fields:
 *   - set_quota
 *     Bits 7:0
 *     Purpose: when 1, FW configures quota and starts quota monitoring. when 0, FW stops.
 *   - RESERVED
 *     Bits 31:8
 *     Purpose: reserved bits
 *   - quota_lo
 *     Bits 31:0
 *     Purpose: bytes of quota to be set, low 32-bit.
 *              It is accumulated number of bytes from when quota is configured.
 *   - quota_hi
 *     Bits 31:0
 *     Purpose: bytes of quota to be set, high 32-bit
 **/

#define WLAN_WDI_IPA_SET_QUOTA_REQ_SZ           12 /* bytes */

#define WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_M  0x000000ff
#define WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_S  0x0

#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_M   0xffffffff
#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_S   0x0

#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_M   0xffffffff
#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_S   0x0

#define WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_M) >> \
          WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_S)
#define WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_SET_QUOTA_REQ_SET_QUOTA_S)); \
        } while(0)

#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_M) >> \
          WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_S)
#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_M) >> \
          WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_S)
#define WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_SET_QUOTA_REQ_QUOTA_HI_S)); \
        } while(0)

PREPACK struct wlan_wdi_ipa_set_quota_req_t {
        A_UINT32 set_quota:8,
             reserved:24;
        A_UINT32 quota_lo;
        A_UINT32 quota_hi;
} POSTPACK;

/**
 * @brief WLAN_WDI_IPA_SET_QUOTA_RESP
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                reserved                          |    success     |
 *     |-------------------------------------------------------------------|
 *     |                            total_bytes_lo                         |
 *     |-------------------------------------------------------------------|
 *     |                            total_bytes_hi                         |
 *     |-------------------------------------------------------------------|
 * Header fields:
 *   - success
 *     Bits 7:0
 *     Purpose: when 1, FW successfully sets the quota.
 *              when 0, FW rejects because quota is smaller than current total
 *              bytes.
 *   - total_bytes_lo
 *     Bits 31:0
 *     Purpose: total bytes so far when the quota is configured, low 32-bit.
 *              It is accumulated number of bytes when quota is configured.
 *   - total_bytes_hi
 *     Bits 31:0
 *     Purpose: total bytes, high 32-bit
 **/

#define WLAN_WDI_IPA_SET_QUOTA_RESP_SZ          12 /* bytes */

#define WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_M   0x000000ff
#define WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_S   0x0

#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_M   0xffffffff
#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_S   0x0

#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_M   0xffffffff
#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_S   0x0

#define WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_M) >> \
          WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_S)
#define WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_SET_QUOTA_RESP_SUCCESS_S)); \
        } while(0)

#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_M) >> \
          WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_S)
#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_M) >> \
          WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_S)
#define WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_SET_QUOTA_RESP_TOTAL_BYTES_HI_S)); \
        } while(0)

PREPACK struct wlan_wdi_ipa_set_quota_resp_t {
        A_UINT32 success:8,
             reserved:24;
        A_UINT32 total_bytes_lo;
        A_UINT32 total_bytes_hi;
} POSTPACK;

/**
 * @brief WLAN_WDI_IPA_QUOTA_IND_T
 *
 *     |31            24|23            16|15             8|7              0|
 *     |----------------+----------------+----------------+----------------|
 *     |                            total_bytes_lo                         |
 *     |-------------------------------------------------------------------|
 *     |                            total_bytes_hi                         |
 *     |-------------------------------------------------------------------|
 * Header fields:
 *   - total_bytes_lo
 *     Bits 31:0
 *     Purpose: accumulated bytes of packets through sharing interface
 *              when reaching to quota, low 32-bit
 *   - total_bytes_hi
 *     Bits 31:0
 *     Purpose: total bytes, high 32-bit
 **/

#define WLAN_WDI_IPA_QUOTA_IND_SZ          8 /* bytes */

#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_M   0xffffffff
#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_S   0x0

#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_M   0xffffffff
#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_S   0x0

#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_M) >> \
          WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_S)
#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_LO_S)); \
        } while(0)

#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_GET(_var)        \
        (((_var) & WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_M) >> \
          WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_S)
#define WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_SET(_var, _val)  \
        do {                                               \
            ((_var) &= (WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_M)); \
            ((_var) |= ((_val) << WLAN_WDI_IPA_QUOTA_IND_TOTAL_BYTES_HI_S)); \
        } while(0)

PREPACK struct wlan_wdi_ipa_quota_ind_t {
        A_UINT32 total_bytes_lo;
        A_UINT32 total_bytes_hi;
} POSTPACK;

#endif /* _WDI_IPA_H_ */
