/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: dp_extap.c
 * This file defines the important functions pertinent to
 * extender AP functionality to initialize and de-initialize the component.
 */

#include <qdf_nbuf.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#include "dp_extap.h"
#include "dp_extap_mitbl.h"

char *arps[] = { NULL, "req", "rsp", "rreq", "rrsp" };

#ifdef EXTAP_DEBUG
static void print_arp_pkt(qdf_ether_header_t *eh)
{
	qdf_net_arphdr_t *arp = (qdf_net_arphdr_t *)(eh + 1);

	if (arp->ar_op == qdf_htons(QDF_ARP_REQ) ||
		arp->ar_op == qdf_htons(QDF_ARP_RREQ)) {
		extap_debug("arp request for " eaistr " from "
				   eaistr "-" eamstr "\n",
				   eaip(arp->ar_tip), eaip(arp->ar_sip),
				   eamac(arp->ar_sha));
	}

	if (arp->ar_op == qdf_htons(QDF_ARP_RSP) ||
			arp->ar_op == qdf_htons(QDF_ARP_RRSP)) {
		extap_debug("arp reply for " eaistr "-" eamstr " from "
				eaistr "-" eamstr "\n",
				eaip(arp->ar_tip), eamac(arp->ar_tha),
				eaip(arp->ar_sip), eamac(arp->ar_sha));
	}
}

static void print_ipv6_pkt(char *s, qdf_ether_header_t *eh)
{
	qdf_net_ipv6hdr_t *iphdr = (qdf_net_ipv6hdr_t *)(eh + 1);
	qdf_net_nd_msg_t *nd = (qdf_net_nd_msg_t *)(iphdr + 1);
	qdf_net_icmp6_11addr_t *ha;
	char *type, *nds[] = { "rsol", "r-ad", "nsol", "n-ad", "rdr" };

	if (iphdr->ipv6_nexthdr != NEXTHDR_ICMP)
		return;

	if (nd->nd_icmph.icmp6_type >= NDISC_ROUTER_SOLICITATION &&
		nd->nd_icmph.icmp6_type <= NDISC_REDIRECT) {
		ha = (qdf_net_icmp6_11addr_t *)nd->nd_opt;
		type = nds[nd->nd_icmph.icmp6_type - NDISC_ROUTER_SOLICITATION];
		extap_debug("%s (%s):\ts-ip: " eastr6 "\n"
				   "\t\td-ip: " eastr6 "\n"
				   "s: " eamstr "\td: " eamstr "\n"
				   "\tha: " eamstr " cksum = 0x%x len = %u\n", s, type,
				   eaip6(iphdr->ipv6_saddr.s6_addr),
				   eaip6(iphdr->ipv6_daddr.s6_addr),
				   eamac(eh->ether_shost), eamac(eh->ether_dhost),
				   eamac(ha->addr), nd->nd_icmph.icmp6_cksum,
				   qdf_ntohs(iphdr->ipv6_payload_len));
		return;
	} else if (nd->nd_icmph.icmp6_type == ICMPV6_ECHO_REQUEST) {
		extap_debug("%s (ping req):\ts-ip: " eastr6 "\n"
				   "\t\td-ip: " eastr6 "\n"
				   "s: " eamstr "\td: " eamstr "\n", s,
				   eaip6(iphdr->ipv6_saddr.s6_addr),
				   eaip6(iphdr->ipv6_daddr.s6_addr),
				   eamac(eh->ether_shost), eamac(eh->ether_dhost));
	} else if (nd->nd_icmph.icmp6_type == ICMPV6_ECHO_REPLY) {
		extap_debug("%s (ping rpl):\ts-ip: " eastr6 "\n"
				   "\t\td-ip: " eastr6 "\n"
				   "s: " eamstr "\td: " eamstr "\n", s,
				   eaip6(iphdr->ipv6_saddr.s6_addr),
				   eaip6(iphdr->ipv6_daddr.s6_addr),
				   eamac(eh->ether_shost), eamac(eh->ether_dhost));
	} else {
		extap_debug("%s unknown icmp packet 0x%x\n", s,
				   nd->nd_icmph.icmp6_type);
		return;
	}
}
#endif

static inline int is_ipv6_addr_multicast(const qdf_net_ipv6_addr_t *addr)
{
	return (addr->s6_addr32[0] & qdf_htonl(0xff000000U)) ==
		qdf_htonl(0xff000000U);
}

	static void
dp_extap_in_ipv6(dp_pdev_extap_t *extap, qdf_ether_header_t *eh)
{
	qdf_net_ipv6hdr_t *iphdr = (qdf_net_ipv6hdr_t *)(eh + 1);
	u_int8_t *mac;
	qdf_net_icmp6_11addr_t *ha;
	rwlock_state_t lock_state;

	print_ipv6("inp6", eh);

	if (iphdr->ipv6_nexthdr == QDF_NEXTHDR_ICMP) {
		qdf_net_nd_msg_t *nd = (qdf_net_nd_msg_t *)(iphdr + 1);

		switch (nd->nd_icmph.icmp6_type) {
		case QDF_ND_NSOL:	/* ARP Request */
				ha = (qdf_net_icmp6_11addr_t *)nd->nd_opt;
				OS_RWLOCK_WRITE_LOCK(&extap->mi_lock, &lock_state);
				/* save source ip */
				mi_add(&extap->miroot, iphdr->ipv6_saddr.s6_addr,
					  ha->addr, ATH_MITBL_IPV6);
				OS_RWLOCK_WRITE_UNLOCK(&extap->mi_lock, &lock_state);
				return;
		case QDF_ND_NADVT:	/* ARP Response */
				if ((is_ipv6_addr_multicast(&nd->nd_target)) ||
					(nd->nd_icmph.icmp6_dataun.u_nd_advt.override)) {
					ha = (qdf_net_icmp6_11addr_t *)nd->nd_opt;
					/* save target ip */
					OS_RWLOCK_WRITE_LOCK(&extap->mi_lock, &lock_state);
					mi_add(&extap->miroot, nd->nd_target.s6_addr,
						  ha->addr, ATH_MITBL_IPV6);
					OS_RWLOCK_WRITE_UNLOCK(&extap->mi_lock, &lock_state);
				}
				/*
				 * Unlike ipv4, source IP and MAC is not present.
				 * Nothing to restore in the packet
				 */
				break;
		case QDF_ND_RSOL:
		case QDF_ND_RADVT:
		default:
				/* Don't know what to do */
				;
				extap_debug("icmp type 0x%x\n",
						   nd->nd_icmph.icmp6_type);
				break;
		}
	} else {
		extap_debug("inp6 (0x%x):\ts-ip: " eastr6 "\n"
				   "\t\td-ip: " eastr6 "\n"
				   "s: " eamstr "\td: " eamstr "\n", iphdr->ipv6_nexthdr,
				   eaip6(iphdr->ipv6_saddr.s6_addr),
				   eaip6(iphdr->ipv6_daddr.s6_addr),
				   eamac(eh->ether_shost), eamac(eh->ether_dhost));
	}
	OS_RWLOCK_READ_LOCK(&extap->mi_lock, &lock_state);
	mac = mi_lkup(extap->miroot, iphdr->ipv6_daddr.s6_addr,
				 ATH_MITBL_IPV6);
	OS_RWLOCK_READ_UNLOCK(&extap->mi_lock, &lock_state);
	if (mac) {
		extap_debug_mac(eh->ether_dhost, mac);
		WLAN_ADDR_COPY(eh->ether_dhost, mac);
	}
	print_ipv6("INP6", eh);
}

static inline bool is_multicast_group(u_int32_t addr)
{
	return ((addr & 0xf0000000) == 0xe0000000);
}

int dp_extap_input(dp_pdev_extap_t *extap, uint8_t *vdev_macaddr,
				  qdf_ether_header_t *eh)
{
	u_int8_t *mac;
	u_int8_t *sip, *dip;
	qdf_net_iphdr_t *iphdr = NULL;
	qdf_net_arphdr_t *arp = NULL;
	rwlock_state_t lock_state;

	if (qdf_unlikely(!extap))
		return 1;

	switch (qdf_ntohs(eh->ether_type)) {
	case ETHERTYPE_IP:
			iphdr = (qdf_net_iphdr_t *)(eh + 1);
			sip = (u_int8_t *)&iphdr->ip_saddr;
			dip = (u_int8_t *)&iphdr->ip_daddr;
			break;

	case ETHERTYPE_ARP:
			arp = (qdf_net_arphdr_t *)(eh + 1);
			extap_debug("inp %s\t" eaistr "\t" eamstr "\t"
					   eaistr "\t" eamstr "\n",
					   arps[qdf_ntohs(arp->ar_op)],
					   eaip(arp->ar_sip), eamac(arp->ar_sha),
					   eaip(arp->ar_tip), eamac(arp->ar_tha));
			if ((arp->ar_op == qdf_htons(QDF_ARP_REQ) ||
				arp->ar_op == qdf_htons(QDF_ARP_RREQ)) &&
				(QDF_NET_IS_MAC_MULTICAST(eh->ether_dhost))) {
				return 0;
			}

			print_arp(eh);
			sip = arp->ar_sip;
			dip = arp->ar_tip;
			break;

	case ETHERTYPE_PAE:
			extap_debug("Not fwd-ing EAPOL packet from " eamstr "\n",
					   eamac(eh->ether_shost));
			WLAN_ADDR_COPY(eh->ether_dhost, vdev_macaddr);
			return 0;

	case ETHERTYPE_IPV6:
			dp_extap_in_ipv6(extap, eh);
			return 0;

	default:
			extap_debug("Uknown packet type - 0x%x\n",
					   eh->ether_type);
			return 0;
	}

	OS_RWLOCK_READ_LOCK(&extap->mi_lock, &lock_state);
	mac = mi_lkup(extap->miroot, dip, ATH_MITBL_IPV4);
	OS_RWLOCK_READ_UNLOCK(&extap->mi_lock, &lock_state);
	if (mac) {
		extap_debug_mac(eh->ether_dhost, mac);
		WLAN_ADDR_COPY(eh->ether_dhost, mac);

		if (qdf_ntohs(eh->ether_type) == ETHERTYPE_ARP) {
			extap_debug_mac(arp->ar_tha, mac);
			WLAN_ADDR_COPY(arp->ar_tha, mac);
		}
	} else {
		if (iphdr) {
			u_int32_t groupaddr = 0;
			u_int8_t groupaddr_l2[QDF_NET_ETH_LEN];
			u_int8_t ipv4_mc_addr[] = {0x01, 0x00, 0x5e};
			groupaddr = qdf_ntohl(iphdr->ip_daddr);

			if (is_multicast_group(groupaddr)) {
				groupaddr_l2[0] = ipv4_mc_addr[0];
				groupaddr_l2[1] = ipv4_mc_addr[1];
				groupaddr_l2[2] = ipv4_mc_addr[2];
				groupaddr_l2[3] = (groupaddr >> 16) & 0x7f;
				groupaddr_l2[4] = (groupaddr >>  8) & 0xff;
				groupaddr_l2[5] = (groupaddr >>  0) & 0xff;

				WLAN_ADDR_COPY(eh->ether_dhost, groupaddr_l2);
			}
		}
	}
	if (arp) {
		extap_debug("INP %s\t" eaistr "\t" eamstr "\t" eaistr "\t" eamstr "\n",
				   arps[qdf_ntohs(arp->ar_op)],
				   eaip(arp->ar_sip), eamac(arp->ar_sha),
				   eaip(arp->ar_tip), eamac(arp->ar_tha));
	}
	return 0;
}

static int
dp_extap_out_arp(dp_pdev_extap_t *extap, uint8_t *vdev_macaddr,
				qdf_ether_header_t *eh, struct dp_extap_nssol *extap_nssol)
{
	qdf_net_arphdr_t *arp = (qdf_net_arphdr_t *)(eh + 1);
	rwlock_state_t lock_state;

	print_arp(eh);

	if (extap_nssol) {
		extap_nssol->ip_version = ATH_MITBL_IPV4;
		WLAN_ADDR_COPY(extap_nssol->mac, arp->ar_sha);
		qdf_mem_copy((uint8_t *)&extap_nssol->ip,
					arp->ar_sip, QDF_IPV4_ADDR_SIZE);
	}

	OS_RWLOCK_WRITE_LOCK(&extap->mi_lock, &lock_state);
	/* For ARP requests/responses, note down the sender's details */
	mi_add(&extap->miroot, arp->ar_sip, arp->ar_sha,
		  ATH_MITBL_IPV4);
	OS_RWLOCK_WRITE_UNLOCK(&extap->mi_lock, &lock_state);

	extap_debug("out %s\t" eaistr "\t" eamstr "\t" eaistr "\t" eamstr "\n",
			   arps[qdf_ntohs(arp->ar_op)],
			   eaip(arp->ar_sip), eamac(arp->ar_sha),
			   eaip(arp->ar_tip), eamac(arp->ar_tha));
	extap_debug("s: " eamstr "\td: " eamstr "\n",
			   eamac(eh->ether_shost), eamac(eh->ether_dhost));

	/* Modify eth frame as if we sent */
	extap_debug_mac(eh->ether_shost, vdev_macaddr);
	WLAN_ADDR_COPY(eh->ether_shost, vdev_macaddr);

	/* Modify ARP content as if we initiated the req */
	extap_debug_mac(arp->ar_sha, vdev_macaddr);
	WLAN_ADDR_COPY(arp->ar_sha, vdev_macaddr);

	extap_debug("OUT %s\t" eaistr "\t" eamstr "\t" eaistr "\t" eamstr "\n",
			   arps[qdf_ntohs(arp->ar_op)],
			   eaip(arp->ar_sip), eamac(arp->ar_sha),
			   eaip(arp->ar_tip), eamac(arp->ar_tha));
	extap_debug("S: " eamstr "\tD: " eamstr "\n",
			   eamac(eh->ether_shost), eamac(eh->ether_dhost));

	return 0;
}

void compute_udp_checksum(qdf_net_iphdr_t *p_iph, unsigned short  *ip_payload)
{
	register unsigned long sum = 0;
	qdf_net_udphdr_t *udphdrp = (qdf_net_udphdr_t *)ip_payload;
	unsigned short udpLen = qdf_htons(udphdrp->udp_len);

	sum += (qdf_htons(p_iph->ip_saddr >> 16)) & 0xFFFF;
	sum += (qdf_htons(p_iph->ip_saddr)) & 0xFFFF;
	sum += (qdf_htons(p_iph->ip_daddr >> 16)) & 0xFFFF;
	sum += qdf_htons(p_iph->ip_daddr) & 0xFFFF;
	sum += (IPPROTO_UDP); /* protocol and reserved: 17 */
	sum += qdf_htons(udphdrp->udp_len);

	udphdrp->udp_cksum = 0;
	while (udpLen > 1) {
		sum += qdf_htons(*ip_payload);
		ip_payload++;
		udpLen -= 2;
	}
	/* if any bytes left, pad the bytes and add */
	if (udpLen > 0)
		sum += (qdf_htons(*ip_payload) & (0xFF00));

	/* Fold sum to 16 bits: add carrier to result */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	sum = ~sum;
	udphdrp->udp_cksum = ((unsigned short)sum == 0x0000) ? 0xFFFF :
		(unsigned short)ntohs(sum);
	extap_debug("UDP check sum calculated is %x\n", udphdrp->udp_cksum);
}

static void dp_extap_out_ipv4(dp_pdev_extap_t *extap, uint8_t *vdev_macaddr,
							 qdf_ether_header_t *eh)
{
	qdf_net_iphdr_t	*iphdr = (qdf_net_iphdr_t *)(eh + 1);

	if (iphdr->ip_proto == QDF_NBUF_TRAC_UDP_TYPE) { /* protocol is udp*/
		qdf_net_udphdr_t *udphdr = (qdf_net_udphdr_t *)(iphdr + 1);
		/* protocol is DHCP*/
		if (udphdr->dst_port == qdf_htons(QDF_NBUF_TRAC_DHCP_SRV_PORT)) {
			qdf_net_dhcphdr_t *dhcphdr = (qdf_net_dhcphdr_t *)(udphdr + 1);

			if (dhcphdr->dhcp_cookie[0] == 0x63 &&
			   dhcphdr->dhcp_cookie[1] == 0x82 &&
			   dhcphdr->dhcp_cookie[2] == 0x53 &&
			   dhcphdr->dhcp_cookie[3] == 0x63) {
				extap_debug("Found DHCP cookie ..\n");

				if (dhcphdr->dhcp_msg_type == 1) { /* dhcp REQ or DISCOVER*/
					extap_debug("This is DHCP DISCOVER or REQUEST...\n");

					if ((dhcphdr->dhcp_flags & DHCP_FLAGS) == 0) {
						dhcphdr->dhcp_flags |= DHCP_FLAGS;
						extap_debug("Make the dhcp flag to broadcast ...\n");
						compute_udp_checksum(iphdr,	(unsigned short *)udphdr);
						extap_debug("ReComputed udp cksum is %d ..\n",
								   udphdr->udp_cksum);
					}
				}
			} /* Magic cookie for DHCP */
		} /* for dhcp pkt */
	}

	extap_debug_mac(eh->ether_shost, vdev_macaddr);
	WLAN_ADDR_COPY(eh->ether_shost, vdev_macaddr);
}

static int
dp_extap_out_ipv6(dp_pdev_extap_t *extap, uint8_t *vdev_macaddr,
				 qdf_ether_header_t *eh, struct dp_extap_nssol *extap_nssol)
{
	qdf_net_ipv6hdr_t *iphdr = (qdf_net_ipv6hdr_t *)(eh + 1);
	u_int8_t *mac, *ip;
	qdf_net_icmp6_11addr_t *ha;
	rwlock_state_t lock_state;

	print_ipv6("out6", eh);

	if (iphdr->ipv6_nexthdr == QDF_NEXTHDR_ICMP) {
		qdf_net_nd_msg_t	*nd = (qdf_net_nd_msg_t *)(iphdr + 1);

		/* Modify eth frame as if we sent */
		extap_debug_mac(eh->ether_shost, vdev_macaddr);
		WLAN_ADDR_COPY(eh->ether_shost, vdev_macaddr);

		switch (nd->nd_icmph.icmp6_type) {
		case QDF_ND_NSOL:	/* ARP Request */
				ha = (qdf_net_icmp6_11addr_t *)nd->nd_opt;
				/* save source ip */
				if (extap_nssol) {
					extap_nssol->ip_version = ATH_MITBL_IPV6;
					WLAN_ADDR_COPY(extap_nssol->mac, ha->addr);
					qdf_mem_copy((uint8_t *)&extap_nssol->ip,
								iphdr->ipv6_saddr.s6_addr, QDF_IPV6_ADDR_SIZE);
				}

				OS_RWLOCK_WRITE_LOCK(&extap->mi_lock, &lock_state);
				mi_add(&extap->miroot, iphdr->ipv6_saddr.s6_addr,
					  ha->addr, ATH_MITBL_IPV6);
				OS_RWLOCK_WRITE_UNLOCK(&extap->mi_lock, &lock_state);

				extap_debug_mac(ha->addr, vdev_macaddr);
				WLAN_ADDR_COPY(ha->addr, vdev_macaddr);
				nd->nd_icmph.icmp6_cksum = 0;
				nd->nd_icmph.icmp6_cksum = qdf_csum_ipv6(
						(const in6_addr_t *)&iphdr->ipv6_saddr,
						(const in6_addr_t *)&iphdr->ipv6_daddr,
						qdf_ntohs(iphdr->ipv6_payload_len), IPPROTO_ICMPV6,
						csum_partial((__u8 *)nd,
									qdf_ntohs(iphdr->ipv6_payload_len), 0));
				break;
		case QDF_ND_NADVT:	/* ARP Response */

				if ((is_ipv6_addr_multicast(&nd->nd_target)) ||
					(nd->nd_icmph.icmp6_dataun.u_nd_advt.override)) {
					ha = (qdf_net_icmp6_11addr_t *)nd->nd_opt;
					/* save target ip */
					if (extap_nssol) {
						extap_nssol->ip_version = ATH_MITBL_IPV6;
						WLAN_ADDR_COPY(extap_nssol->mac, ha->addr);
						qdf_mem_copy((uint8_t *)&extap_nssol->ip,
									nd->nd_target.s6_addr, QDF_IPV6_ADDR_SIZE);
					}

					OS_RWLOCK_WRITE_LOCK(&extap->mi_lock, &lock_state);
					mi_add(&extap->miroot, nd->nd_target.s6_addr,
						  ha->addr, ATH_MITBL_IPV6);
					OS_RWLOCK_WRITE_UNLOCK(&extap->mi_lock, &lock_state);

					extap_debug_mac(ha->addr, vdev_macaddr);
					WLAN_ADDR_COPY(ha->addr, vdev_macaddr);
				}
				nd->nd_icmph.icmp6_cksum = 0;
				nd->nd_icmph.icmp6_cksum = qdf_csum_ipv6(
						(const in6_addr_t *)&iphdr->ipv6_saddr,
						(const in6_addr_t *)&iphdr->ipv6_daddr,
						qdf_ntohs(iphdr->ipv6_payload_len), IPPROTO_ICMPV6,
						csum_partial((__u8 *)nd,
									qdf_ntohs(iphdr->ipv6_payload_len), 0));
				break;
		case QDF_ND_RSOL:
		case QDF_ND_RADVT:
		default:
				extap_debug("icmp type 0x%x\n",
						   nd->nd_icmph.icmp6_type);
		}
		print_ipv6("OUT6", eh);
		return 0;
	} else {
		extap_debug("out6 (0x%x):\ts-ip:" eastr6 "\n"
				"\t\td-ip: " eastr6 "\n"
				"s: " eamstr "\td: " eamstr "\n", iphdr->ipv6_nexthdr,
				eaip6(iphdr->ipv6_saddr.s6_addr),
				eaip6(iphdr->ipv6_daddr.s6_addr),
				eamac(eh->ether_shost), eamac(eh->ether_dhost));
	}

	ip = iphdr->ipv6_daddr.s6_addr;
	OS_RWLOCK_READ_LOCK(&extap->mi_lock, &lock_state);
	mac = mi_lkup(extap->miroot, ip, ATH_MITBL_IPV6);
	OS_RWLOCK_READ_UNLOCK(&extap->mi_lock, &lock_state);

	if (mac) {
		extap_debug_ip(eh->ether_dhost, mac, ip);
		WLAN_ADDR_COPY(eh->ether_dhost, mac);
	}

	extap_debug_mac(eh->ether_shost, vdev_macaddr);
	WLAN_ADDR_COPY(eh->ether_shost, vdev_macaddr);

	return 0;
}

int dp_extap_output(dp_pdev_extap_t *extap, uint8_t *vdev_macaddr,
				   qdf_ether_header_t *eh, struct dp_extap_nssol *extap_nssol)
{
	if (qdf_unlikely(!extap))
		return 1;

	switch (qdf_ntohs(eh->ether_type)) {
	case ETHERTYPE_ARP:
			return dp_extap_out_arp(extap, vdev_macaddr, eh, extap_nssol);

	case ETHERTYPE_IP:
			dp_extap_out_ipv4(extap, vdev_macaddr, eh);
			break;

	case ETHERTYPE_PAE:
			extap_debug("Not fwd-ing EAPOL packet from " eamstr "\n",
					   eamac(eh->ether_shost));
			WLAN_ADDR_COPY(eh->ether_shost, vdev_macaddr);
			break;

	case ETHERTYPE_IPV6:
			return dp_extap_out_ipv6(extap, vdev_macaddr, eh, extap_nssol);

	default:
			extap_debug("Uknown packet type - 0x%x\n",
					   eh->ether_type);
	}
	return 0;
}

void dp_extap_enable(struct wlan_objmgr_vdev *vdev)
{
	wlan_vdev_mlme_feat_cap_set(vdev, WLAN_VDEV_F_AP);
}

void dp_extap_disable(struct wlan_objmgr_vdev *vdev)
{
	wlan_vdev_mlme_feat_cap_clear(vdev, WLAN_VDEV_F_AP);
}

void dp_extap_mitbl_dump(dp_pdev_extap_t *extap)
{
	rwlock_state_t lock_state;
	if (!extap)
		return;

	OS_RWLOCK_READ_LOCK(&extap->mi_lock, &lock_state);
	mi_tbl_dump(extap->miroot);
	OS_RWLOCK_READ_UNLOCK(&extap->mi_lock, &lock_state);
}

void dp_extap_mitbl_purge(dp_pdev_extap_t *extap)
{
	rwlock_state_t lock_state;
	if (!extap)
		return;

	OS_RWLOCK_WRITE_LOCK(&extap->mi_lock, &lock_state);
	mi_tbl_purge(&extap->miroot);
	OS_RWLOCK_WRITE_UNLOCK(&extap->mi_lock, &lock_state);
}

int dp_extap_tx_process(struct wlan_objmgr_vdev *vdev, struct sk_buff **skb,
					   uint8_t mhdr_len, struct dp_extap_nssol *extap_nssol)
{
	qdf_ether_header_t *eh;
	uint8_t vdev_mac[ETH_ALEN];

	if (qdf_unlikely(dp_is_extap_enabled(vdev) &&
					wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)) {
		*skb = qdf_nbuf_unshare(*skb);
		if (!(*skb))
			return 1;

		wlan_vdev_obj_lock(vdev);
		qdf_mem_copy(vdev_mac, wlan_vdev_mlme_get_macaddr(vdev), ETH_ALEN);
		wlan_vdev_obj_unlock(vdev);
		eh = (qdf_ether_header_t *)((*skb)->data + mhdr_len);

		if (dp_extap_output(dp_get_extap_handle(vdev),
						   vdev_mac, eh, extap_nssol))
			return 1;
	}
	return 0;
}

qdf_export_symbol(dp_extap_tx_process);

int dp_extap_rx_process(struct wlan_objmgr_vdev *vdev, struct sk_buff *skb)
{
	qdf_ether_header_t *eh;
	uint8_t vdev_mac[ETH_ALEN];

	if (qdf_unlikely(dp_is_extap_enabled(vdev) &&
					wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)) {

		wlan_vdev_obj_lock(vdev);
		qdf_mem_copy(vdev_mac, wlan_vdev_mlme_get_macaddr(vdev), ETH_ALEN);
		wlan_vdev_obj_unlock(vdev);
		eh = (qdf_ether_header_t *)skb->data;

		if (dp_extap_input(dp_get_extap_handle(vdev), vdev_mac, eh))
			return 1;
	}
	return 0;
}

qdf_export_symbol(dp_extap_rx_process);

/**
 * @brief extap attach
 *
 * @param pdev objmgr Pointer.
 *
 * @return 0 on success
 * @return -ve on failure
 */
int dp_extap_attach(struct wlan_objmgr_pdev *pdev)
{
	int ret = 0;
	dp_pdev_extap_t *extap_pdev = dp_pdev_get_extap_handle(pdev);

	if (!extap_pdev) {
		extap_debug(" extap_pdev is NULL");
		return -EINVAL;
	}

	OS_RWLOCK_INIT(&extap_pdev->mi_lock);
	return ret;
}
qdf_export_symbol(dp_extap_attach);

/**
 * @brief extap detach
 *
 * @param pdev objmgr Pointer.
 *
 * @return 0 on success
 * @return -ve on failure
 */
int dp_extap_detach(struct wlan_objmgr_pdev *pdev)
{
	int ret = 0;
	dp_pdev_extap_t *extap_pdev = dp_pdev_get_extap_handle(pdev);

	if (!extap_pdev) {
		extap_debug(" extap_pdev is NULL");
		return -EINVAL;
	}
	OS_RWLOCK_DESTROY(&extap_pdev->mi_lock);
	return ret;
}
qdf_export_symbol(dp_extap_detach);
