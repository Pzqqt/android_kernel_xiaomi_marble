/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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

#ifndef _WAL_RX_DESC__H_
#define _WAL_RX_DESC__H_


#if defined(ATH_TARGET)
#include <athdefs.h> /* A_UINT8 */
#else
#include <a_types.h> /* A_UINT8 */
#endif

/*
 * As this header is used by host also,
 * and host will access target registers by target reg tbl,
 * so disable direct-reference here for host.
 *
 */
#if !defined(ATH_PERF_PWR_OFFLOAD)
#if defined(CONFIG_AR900B_SUPPORT) || defined(AR900B) //FIXME_WIFI2 beeliner enbled by default (will be removed once we have target aware HTT)
#include <hw/interface/rx_location_info.h>
#include <hw/interface/rx_pkt_end.h>
#include <hw/interface/rx_phy_ppdu_end.h>
#include <hw/interface/rx_timing_offset.h>
#include <hw/interface/rx_location_info.h>
#include <hw/tlv/rx_attention.h>
#include <hw/tlv/rx_frag_info.h>
#include <hw/tlv/rx_msdu_start.h>
#include <hw/tlv/rx_msdu_end.h>
#include <hw/tlv/rx_mpdu_start.h>
#include <hw/tlv/rx_mpdu_end.h>
#include <hw/tlv/rx_ppdu_start.h>
#include <hw/tlv/rx_ppdu_end.h>
#else
/* HW rx descriptor definitions */
#include <mac_descriptors/rx_attention.h>
#include <mac_descriptors/rx_frag_info.h>
#include <mac_descriptors/rx_msdu_start.h>
#include <mac_descriptors/rx_msdu_end.h>
#include <mac_descriptors/rx_mpdu_start.h>
#include <mac_descriptors/rx_mpdu_end.h>
#include <mac_descriptors/rx_ppdu_start.h>
#include <mac_descriptors/rx_ppdu_end.h>
#endif
/*
 * This struct defines the basic descriptor information, which is
 * written by the 11ac HW MAC into the WAL's rx status descriptor
 * ring.
 */
struct hw_rx_desc_base {
    struct rx_attention  attention;
    struct rx_frag_info  frag_info;
    struct rx_mpdu_start mpdu_start;
    struct rx_msdu_start msdu_start;
    struct rx_msdu_end   msdu_end;
    struct rx_mpdu_end   mpdu_end;
    struct rx_ppdu_start ppdu_start;
    struct rx_ppdu_end   ppdu_end;
};
#endif

/*
 * This struct defines the basic MSDU rx descriptor created by FW.
 */
struct fw_rx_desc_base {
    union {
        struct {
            A_UINT8 discard  : 1,
                    forward  : 1,
                    any_err  : 1,
                    dup_err  : 1,
                    ipa_ind  : 1,
                    inspect  : 1,
                    extension: 2;
        }bits;
        A_UINT8     val;
    }u;
};

#define FW_MSDU_INFO_FIRST_WAKEUP_M 0x40
#define FW_MSDU_INFO_FIRST_WAKEUP_S 6

#define FW_RX_DESC_DISCARD_M 0x1
#define FW_RX_DESC_DISCARD_S 0
#define FW_RX_DESC_FORWARD_M 0x2
#define FW_RX_DESC_FORWARD_S 1
#define FW_RX_DESC_ANY_ERR_M 0x4
#define FW_RX_DESC_ANY_ERR_S 2
#define FW_RX_DESC_DUP_ERR_M 0x8
#define FW_RX_DESC_DUP_ERR_S 3
#define FW_RX_DESC_INSPECT_M 0x20
#define FW_RX_DESC_INSPECT_S 5
#define FW_RX_DESC_EXT_M     0xc0
#define FW_RX_DESC_EXT_S     6

#define FW_RX_DESC_CNT_2_BYTES(_fw_desc_cnt)    (_fw_desc_cnt)

enum {
    FW_RX_DESC_EXT_NONE          = 0,
    FW_RX_DESC_EXT_LRO_ONLY,
    FW_RX_DESC_EXT_LRO_AND_OTHER,
    FW_RX_DESC_EXT_OTHER
};

#define FW_RX_DESC_DISCARD_GET(_var) \
    (((_var) & FW_RX_DESC_DISCARD_M) >> FW_RX_DESC_DISCARD_S)
#define FW_RX_DESC_DISCARD_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_DESC_DISCARD_S))

#define FW_RX_DESC_FORWARD_GET(_var) \
    (((_var) & FW_RX_DESC_FORWARD_M) >> FW_RX_DESC_FORWARD_S)
#define FW_RX_DESC_FORWARD_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_DESC_FORWARD_S))

#define FW_RX_DESC_INSPECT_GET(_var) \
    (((_var) & FW_RX_DESC_INSPECT_M) >> FW_RX_DESC_INSPECT_S)
#define FW_RX_DESC_INSPECT_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_DESC_INSPECT_S))

#define FW_RX_DESC_EXT_GET(_var) \
    (((_var) & FW_RX_DESC_EXT_M) >> FW_RX_DESC_EXT_S)
#define FW_RX_DESC_EXT_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_DESC_EXT_S))


/*
 * This struct defines TCP_CHKSUM_OFFLOAD bit fields which are needed by host.
 */
struct fw_rx_msdu_info {
    union {
        /*
         * The "bits" struct defines the flags in fw_rx_msdu_info used
         * during regular operation.
         */
        struct {
            A_UINT8 tcp_udp_chksum_fail : 1, /* for tcp checksum offload use */
                    ip_chksum_fail      : 1,
                    ipv6_proto          : 1,
                    tcp_proto           : 1,
                    udp_proto           : 1,
                    ip_frag             : 1,
                    first_wakeup        : 1,
                    reserved            : 1;
        } bits;
        /*
         * The "mon" struct defines the flags in fw_rx_msdu_info used
         * during monitor mode.
         */
        struct {
            A_UINT8 last_frag           : 1,
                    reserved            : 7;
        } mon;
        A_UINT8     val;
    } u;
};

/* regular operation flags */

#define FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_M 0x1
#define FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_S 0
#define FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_M      0x2
#define FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_S      1
#define FW_RX_MSDU_INFO_IPV6_PROTO_M          0x4
#define FW_RX_MSDU_INFO_IPV6_PROTO_S          2
#define FW_RX_MSDU_INFO_TCP_PROTO_M           0x8
#define FW_RX_MSDU_INFO_TCP_PROTO_S           3
#define FW_RX_MSDU_INFO_UDP_PROTO_M           0x10
#define FW_RX_MSDU_INFO_UDP_PROTO_S           4
#define FW_RX_MSDU_INFO_IP_FRAG_M             0x20
#define FW_RX_MSDU_INFO_IP_FRAG_S             5
#define FW_RX_MSDU_INFO_FIRST_WAKEUP_M        0x40
#define FW_RX_MSDU_INFO_FIRST_WAKEUP_S        6

#define FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_M) >> FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_S)
#define FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_TCP_UDP_CHKSUM_FAIL_S))

#define FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_M) >> FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_S)
#define FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_IP_CHKSUM_FAIL_S))

#define FW_RX_MSDU_INFO_IPV6_PROTO_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_IPV6_PROTO_M) >> FW_RX_MSDU_INFO_IPV6_PROTO_S)
#define FW_RX_MSDU_INFO_IPV6_PROTO_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_IPV6_PROTO_S))

#define FW_RX_MSDU_INFO_TCP_PROTO_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_TCP_PROTO_M) >> FW_RX_MSDU_INFO_TCP_PROTO_S)
#define FW_RX_MSDU_INFO_TCP_PROTO_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_TCP_PROTO_S))

#define FW_RX_MSDU_INFO_UDP_PROTO_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_UDP_PROTO_M) >> FW_RX_MSDU_INFO_UDP_PROTO_S)
#define FW_RX_MSDU_INFO_UDP_PROTO_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_UDP_PROTO_S))

#define FW_RX_MSDU_INFO_IP_FRAG_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_IP_FRAG_M) >> FW_RX_MSDU_INFO_IP_FRAG_S)
#define FW_RX_MSDU_INFO_IP_FRAG_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_IP_FRAG_S))

#define FW_RX_MSDU_INFO_FIRST_WAKEUP_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_FIRST_WAKEUP_M) >> FW_RX_MSDU_INFO_FIRST_WAKEUP_S)
#define FW_RX_MSDU_INFO_FIRST_WAKEUP_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_FIRST_WAKEUP_S))


/* monitor mode flags */

#define FW_RX_MSDU_INFO_MON_LAST_FRAG_M       0x1
#define FW_RX_MSDU_INFO_MON_LAST_FRAG_S       0


#define FW_RX_MSDU_INFO_MON_LAST_FRAG_GET(_var) \
    (((_var) & FW_RX_MSDU_INFO_MON_LAST_FRAG_M) >> FW_RX_MSDU_INFO_MON_LAST_FRAG_S)
#define FW_RX_MSDU_INFO_MON_LAST_FRAG_SET(_var, _val) \
    ((_var) |= ((_val) << FW_RX_MSDU_INFO_MON_LAST_FRAG_S))


#endif /* _WAL_RX_DESC__H_ */
