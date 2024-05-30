/*
 * Copyright (c) 2012-2014, 2017, 2021 The Linux Foundation. All rights reserved.
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

#ifndef _ENET__H_
#define _ENET__H_

#if defined(ATH_TARGET)
#include <osapi.h>   /* A_UINT8 */
#else
#include <a_types.h> /* A_UINT8 */
#endif


#define ETHERNET_ADDR_LEN 6 /* bytes */
#define ETHERNET_TYPE_LEN 2 /* bytes - length of the Ethernet type field */

struct ethernet_hdr_t {
    A_UINT8 dest_addr[ETHERNET_ADDR_LEN];
    A_UINT8 src_addr[ETHERNET_ADDR_LEN];
    A_UINT8 ethertype[ETHERNET_TYPE_LEN];
};

#define ETHERNET_HDR_LEN (sizeof(struct ethernet_hdr_t))

#define ETHERNET_CRC_LEN  4     /* bytes - length of the Ethernet CRC */
#define ETHERNET_MAX_LEN  1518  /* bytes */

#define ETHERNET_MTU (ETHERNET_MAX_LEN - (ETHERNET_HDR_LEN + ETHERNET_CRC_LEN))


typedef struct llc_snap_hdr_t {
    A_UINT8 dsap;
    A_UINT8 ssap;
    A_UINT8 cntl;
    A_UINT8 org_code[3];
    A_UINT8 ethertype[2];
} LLC_SNAP_HDR;

#define LLC_SNAP_HDR_LEN (sizeof(struct llc_snap_hdr_t))
#define LLC_SNAP_HDR_OFFSET_ETHERTYPE \
    (offsetof(struct llc_snap_hdr_t, ethertype[0]))

#define ETHERTYPE_VLAN_LEN  4

struct ethernet_vlan_hdr_t {
    A_UINT8 dest_addr[ETHERNET_ADDR_LEN];
    A_UINT8 src_addr[ETHERNET_ADDR_LEN];
    A_UINT8 vlan_tpid[2];
    A_UINT8 vlan_tci[2];
    A_UINT8 ethertype[2];
};

typedef PREPACK struct _wai_hdr {
    A_UINT8 version[2];
    A_UINT8 type;
    A_UINT8 stype;
    A_UINT8 reserve[2];
    A_UINT8 length[2];
    A_UINT8 rxseq[2];
    A_UINT8 frag_sc;
    A_UINT8 more_frag;
    /* followed octets of data */
} POSTPACK wai_hdr;

typedef PREPACK struct {
    A_UINT16 vlan_tci;
    A_UINT16 vlan_encap_p;
} POSTPACK vlan_hdr_t;

#define ETHERTYPE_IS_EAPOL_WAPI(typeorlen)           \
			((typeorlen) == ETHERTYPE_PAE ||  \
			(typeorlen) == ETHERTYPE_WAI)

#define IS_ETHERTYPE(_typeOrLen) ((_typeOrLen) >= 0x0600)

#ifndef ETHERTYPE_IPV4
#define ETHERTYPE_IPV4  0x0800 /* Internet Protocol, Version 4 (IPv4) */
#endif

#ifndef ETHERTYPE_AARP
#define	ETHERTYPE_AARP	0x80f3		/* Internal QCA AARP protocol */
#endif

#ifndef ETHERTYPE_IPX
#define ETHERTYPE_IPX    0x8137    /* IPX over DIX protocol */
#endif

#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP   0x0806 /* Address Resolution Protocol (ARP) */
#endif

#ifndef ETHERTYPE_RARP
#define ETHERTYPE_RARP  0x8035 /* Reverse Address Resolution Protocol (RARP) */
#endif

#ifndef ETHERTYPE_VLAN
#define ETHERTYPE_VLAN  0x8100 /* VLAN TAG protocol */
#endif

#ifndef ETHERTYPE_SNMP
#define ETHERTYPE_SNMP  0x814C /* Simple Network Management Protocol (SNMP) */
#endif

#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6  0x86DD /* Internet Protocol, Version 6 (IPv6) */
#endif

#ifndef ETHERTYPE_PAE
#define ETHERTYPE_PAE   0x888E /* EAP over LAN (EAPOL) */
#endif

#ifndef ETHERTYPE_WAI
#define ETHERTYPE_WAI   0x88B4 /* WAPI */
#endif

#ifndef ETHERTYPE_TDLS
#define ETHERTYPE_TDLS  0x890D /* TDLS */
#endif

#define LLC_SNAP_LSAP 0xaa
#define LLC_UI 0x3

#define RFC1042_SNAP_ORGCODE_0 0x00
#define RFC1042_SNAP_ORGCODE_1 0x00
#define RFC1042_SNAP_ORGCODE_2 0x00

#define BTEP_SNAP_ORGCODE_0 0x00
#define BTEP_SNAP_ORGCODE_1 0x00
#define BTEP_SNAP_ORGCODE_2 0xf8


#define WAI_FRAME_TYPE 0X01
#define WAPI_M4_TYPE 0x0c
#define WAPI_M2_TYPE 0x09


#define ICMP_PROTOCOL   1
#define TCP_PROTOCOL    6
#define UDP_PROTOCOL    17
#define IGMP_PROTOCOL   2
#define ICMPV6_PROTOCOL 58
#define BOOTP_SERVER_PORT 67
#define BOOTP_CLIENT_PORT 68
#define MLD_QUERY 130
#define MLD_DONE  132


#define IS_EAPOL(typeorlen) \
    ((typeorlen) == ETHERTYPE_PAE || \
     (typeorlen) == ETHERTYPE_WAI)

#define IS_SNAP(_llc) \
    ((_llc)->dsap == LLC_SNAP_LSAP && \
     (_llc)->ssap == LLC_SNAP_LSAP && \
     (_llc)->cntl == LLC_UI)

#define IS_RFC1042(_llc) \
    ((_llc)->org_code[0] == RFC1042_SNAP_ORGCODE_0 && \
     (_llc)->org_code[1] == RFC1042_SNAP_ORGCODE_1 && \
     (_llc)->org_code[2] == RFC1042_SNAP_ORGCODE_2)

#define IS_BTEP(_llc) \
    ((_llc)->org_code[0] == BTEP_SNAP_ORGCODE_0 && \
     (_llc)->org_code[1] == BTEP_SNAP_ORGCODE_1 && \
     (_llc)->org_code[2] == BTEP_SNAP_ORGCODE_2)

#define IS_MULTICAST(_hdr) (*(A_UINT8 *)(_hdr) & 0x1)
#define IS_BROADCAST(_hdr) \
    ((*((A_UINT8 *)(_hdr) + 0) == 0xff) && \
     (*((A_UINT8 *)(_hdr) + 1) == 0xff) && \
     (*((A_UINT8 *)(_hdr) + 2) == 0xff) && \
     (*((A_UINT8 *)(_hdr) + 3) == 0xff) && \
     (*((A_UINT8 *)(_hdr) + 4) == 0xff) && \
     (*((A_UINT8 *)(_hdr) + 5) == 0xff))


#endif /* _ENET__H_ */
