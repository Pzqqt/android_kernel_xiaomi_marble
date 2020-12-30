/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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

#if ATH_SUPPORT_WRAP
#if !WLAN_QWRAP_LEGACY
/*WRAP includes for MAT*/
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ndisc.h>
#include <net/arp.h>
#include "asf_print.h"    /* asf_print_setup */
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "qdf_lock.h"  /* qdf_spinlock_* */
#include "qdf_types.h" /* qdf_vprint */
#include "dp_wrap.h"
#include "ol_if_athvar.h"

extern char *arps[];

extern void
compute_udp_checksum(qdf_net_iphdr_t *p_iph, unsigned short  *ip_payload);

static u16
checksum(u16 protocol, u16 len, u8 src_addr[], u8 dest_addr[],
	 u16 addrleninbytes, u8 *buff)
{
	u16 pad = 0;
	u16 word16;
	u32 sum = 0;
	int i;

	/* Find out if the length of data is even or odd number. If odd,
	 * add a padding byte = 0 at the end of packet
	 */
	if (len & 1) {
		/* Take care of the last byte by itself. */
		len -= 1;
		pad = 1;
	}

	/* make 16 bit words out of every two adjacent 8 bit words and
	 * calculate the sum of all 16 bit words
	 */
	for (i = 0; i < len; i = i + 2) {
		word16 = buff[i];
		word16 = (word16 << 8) + buff[i + 1];
		sum = sum + (u32)word16;
	}

	if (pad) {
		/* Get the last byte */
		word16 = buff[len];
		word16 <<= 8;
		sum = sum + (u32)word16;
	}

	/* add the UDP pseudo header which contains the IP source and
	 * destination addresses.
	 */
	for (i = 0; i < addrleninbytes; i = i + 2) {
		word16 = src_addr[i];
		word16 = (word16 << 8) + src_addr[i + 1];
		sum = sum + (u32)word16;
	}

	for (i = 0; i < addrleninbytes; i = i + 2) {
		word16 = dest_addr[i];
		word16 = (word16 << 8) + dest_addr[i + 1];
		sum = sum + (u32)word16;
	}

	/* the protocol number and the length of the s packet */
	sum = sum + (u32)protocol + (u32)(len + pad);

	/* keep only the last 16 bits of the 32 bit calculated sum and
	 *  add back the carries
	 */
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	/* Take the one's complement of sum */
	sum = ~sum;

	return (u16)sum;
}

struct eth_arphdr {
	unsigned short	ar_hrd,	/* format of hardware address */
			ar_pro;	/* format of protocol address */
	unsigned char	ar_hln,	/* length of hardware address */
			ar_pln;	/* length of protocol address */
	unsigned short	ar_op;	/* ARP opcode (command) */
	unsigned char	ar_sha[ETH_ALEN],	/* sender hardware address */
			ar_sip[4],		/* sender IP address */
			ar_tha[ETH_ALEN],	/* target hardware address */
			ar_tip[4];		/* target IP address */
} __attribute__((__packed__));

struct dhcp_option {
	u_char type;
	u_char len;
	u_char value[0];	/* option value*/
} __attribute__((__packed__));

struct dhcp_packet {
	u_char              op;          /* packet opcode type */
	u_char              htype;       /* hardware addr type */
	u_char              hlen;        /* hardware addr length */
	u_char              hops;        /* gateway hops */
	u32           xid;         /* transaction ID */
	u16           secs;        /* seconds since boot began */
	u16           flags;       /* flags */
	struct in_addr      ciaddr;      /* client IP address */
	struct in_addr      yiaddr;      /* 'your' IP address */
	struct in_addr      siaddr;      /* server IP address */
	struct in_addr      giaddr;      /* gateway IP address */
	u_char              chaddr[16];  /* client hardware address */
	u_char              sname[64];   /* server host name */
	u_char              file[128];   /* boot file name */
	u_char              magic_cookie[4];   /* magic cookie */
	u_char              options[0];  /* variable-length options field */
} __attribute__((__packed__));

struct eth_icmp6_lladdr {
	unsigned char type;
	unsigned char len;
	unsigned char addr[6];	/* hardware address */
} __attribute__((__packed__));

typedef struct eth_icmp6_lladdr eth_icmp6_lladdr_t;
/* Option: (53) DHCP Message Type  */
#define OPTION_DHCP_MSG_TYPE   0x35
 /* DHCP: Discover (1) */
#define DHCP_DISOVER    1
/*Option: (50) Requested IP Address */
#define OPTION_DHCP_REQUESTED_IP_ADDR   0x32

/**
 * @brief WRAP MAT function for transmit.
 *
 * @param wrap_vdev handle.
 * @param buf
 *
 * @return 0 On success.
 * @return -ve On failure.
 */
int dp_wrap_mat_tx(struct dp_wrap_vdev *wvdev, wbuf_t buf)
{
	struct ether_header *eh;
	u16 ether_type;
	int contig_len = sizeof(struct ether_header);
	int pktlen = wbuf_get_pktlen(buf);
	u8 *src_mac, *des_mac, *p, ismcast;
	u8 *arp_smac = NULL;
	u8 *arp_dmac = NULL;
	struct eth_arphdr *parp = NULL;

	if (!wvdev->mat_enabled)
		return 0;

	if (pktlen < contig_len)
		return -EINVAL;

	eh = (struct ether_header *)(wbuf_header(buf));
	p = (u8 *)(eh + 1);

	ether_type = eh->ether_type;
	src_mac = eh->ether_shost;
	des_mac = eh->ether_dhost;
	ismcast = IEEE80211_IS_MULTICAST(des_mac);

	qwrap_trace("src mac:" QDF_MAC_ADDR_FMT " dst mac: "
		    " "QDF_MAC_ADDR_FMT " "
		    "ether type: %d", QDF_MAC_ADDR_REF(src_mac),
		    QDF_MAC_ADDR_REF(des_mac), ether_type);

	if (ether_type == htons(ETH_P_PAE))
		return 0;

	if (ether_type == htons(ETHERTYPE_ARP)) {
		parp = (struct eth_arphdr *)p;
		contig_len += sizeof(struct eth_arphdr);

		if (pktlen < contig_len)
			return -EINVAL;

		if (parp->ar_hln == ETH_ALEN &&
		    parp->ar_pro == htons(ETH_P_IP)) {
			arp_smac = parp->ar_sha;
			arp_dmac = parp->ar_tha;
		} else {
			parp = NULL;
		}
	}

	if (parp) {
		if (parp->ar_op == htons(ARPOP_REQUEST) ||
		    parp->ar_op == htons(ARPOP_REPLY)) {
			IEEE80211_ADDR_COPY(arp_smac, wvdev->wrap_dev_vma);
			qwrap_debug("ARP %s\t" QDF_IPV4_ADDR_STR "\t"
				    " " QDF_MAC_ADDR_FMT "\t"
				    " " QDF_IPV4_ADDR_STR "\t"
				    " " QDF_MAC_ADDR_FMT "\n",
				    arps[qdf_ntohs(parp->ar_op)],
				    QDF_IPV4_ADDR_ARRAY(parp->ar_sip),
				    QDF_MAC_ADDR_REF(parp->ar_sha),
				    QDF_IPV4_ADDR_ARRAY(parp->ar_tip),
				    QDF_MAC_ADDR_REF(parp->ar_tha));
		}
	} else if (ether_type == htons(ETHERTYPE_IP)) {
		struct iphdr *p_ip = (struct iphdr *)(p);
		qdf_net_udphdr_t *p_udp;
		s16 ip_hlen = 0;
		u16 udplen = 0;

		contig_len += sizeof(struct iphdr);
		if (pktlen < contig_len)
			return -EINVAL;

		ip_hlen = p_ip->ihl * 4;
		p_udp = (qdf_net_udphdr_t *)(((u8 *)p_ip) + ip_hlen);

		/* If Proto is UDP */
		if (p_ip->protocol == IPPROTO_UDP) {
			contig_len += sizeof(struct udphdr);
			if (pktlen < contig_len)
				return -EINVAL;
			udplen = p_ip->tot_len - (p_ip->ihl * 4);
		}
		/*
		 * DHCP request UDP Client SP = 68 (bootpc), DP = 67 (bootps).
		 */
		if ((p_ip->protocol == IPPROTO_UDP) &&
		    (p_udp->dst_port == htons(67))) {
			struct dhcp_packet *p_dhcp;
			struct dhcp_option *option;
			u8 *value;
			u8 dhcpDicover = 0;

			p_dhcp = (struct dhcp_packet *)(((u8 *)p_udp) +
					sizeof(struct udphdr));
			option = (struct dhcp_option *)p_dhcp->options;

			contig_len += sizeof(struct dhcp_packet);
			if (pktlen < contig_len)
				return -EINVAL;

			qwrap_debug("src mac:" QDF_MAC_ADDR_FMT " "
				    "dst mac:" QDF_MAC_ADDR_FMT " "
				    "ether type: %d",
				    QDF_MAC_ADDR_REF(src_mac),
				    QDF_MAC_ADDR_REF(des_mac),
				    ether_type);
			qwrap_debug("DHCP: sport %d dport %d len %d"
				    "chaddr:" QDF_MAC_ADDR_FMT " "
				    "opcode: %d",
				    p_udp->src_port, p_udp->dst_port,
				    udplen,
				    QDF_MAC_ADDR_REF(p_dhcp->chaddr),
				    p_dhcp->op);

			if (p_dhcp->magic_cookie[0] == 0x63 &&
			    p_dhcp->magic_cookie[1] == 0x82 &&
			    p_dhcp->magic_cookie[2] == 0x53 &&
			    p_dhcp->magic_cookie[3] == 0x63) {
				if (p_dhcp->op == 1) {
					/* dhcp REQ or DISCOVER*/
#ifdef BIG_ENDIAN_HOST
					if ((p_dhcp->flags & 0x8000) == 0) {
						p_dhcp->flags |= 0x8000;
#else
					if ((p_dhcp->flags & 0x0080) == 0) {
						p_dhcp->flags |= 0x0080;
#endif

						compute_udp_checksum((qdf_net_iphdr_t *)p_ip, (unsigned short *)p_udp);
					}
				}
			} /* Magic cookie for DHCP*/

			while (option->type != 0xFF) {
				/*Not an end option*/

				contig_len += (option->len + 2);
				if (pktlen < contig_len)
					return -EINVAL;

				value = option->value;
				if (option->type == OPTION_DHCP_MSG_TYPE) {
					if (value[0] == DHCP_DISOVER) {
						dhcpDicover = 1;
					}
					option = (struct dhcp_option *)(value +
						  option->len);

				} else if (option->type ==
					   OPTION_DHCP_REQUESTED_IP_ADDR &&
					   dhcpDicover == 1) {
					u32    *addr = (u32 *)value;

					if (*addr != 0) {
						memset(value, 0, option->len);
						compute_udp_checksum((qdf_net_iphdr_t *)p_ip, (unsigned short *)p_udp);
					}
					qwrap_debug("DHCP Disover with"
						    "IP Addr:"
						    " " QDF_IPV4_ADDR_STR " "
						    "from caddr: "
						    " " QDF_MAC_ADDR_FMT " ",
						    QDF_IPV4_ADDR_ARRAY(value),
						    QDF_MAC_ADDR_REF(p_dhcp->chaddr));

					break;
				} else {
					option = (struct dhcp_option *)(value +
						  option->len);
				}
			}
		}
	} else if (ether_type == htons(ETHERTYPE_IPV6)) {
		struct ipv6hdr *p_ip6 = (struct ipv6hdr *)(p);
		eth_icmp6_lladdr_t  *phwaddr;
		int change_packet = 1;

		contig_len += sizeof(struct ipv6hdr);
		if (pktlen < contig_len)
			return -EINVAL;

		if (p_ip6->nexthdr == IPPROTO_ICMPV6) {
			struct icmp6hdr *p_icmp6;
			u16 icmp6len = ntohs(p_ip6->payload_len);
			p_icmp6 = (struct icmp6hdr *)(p_ip6 + 1);

			contig_len += sizeof(struct icmp6hdr);
			if (pktlen < contig_len)
				return -EINVAL;
			/*
			 * It seems that we only have to modify IPv6 packets
			 * sent by a Proxy STA. Both the solicitation and
			 * adevertisement packets have the STA's OMA.
			 * Flip that to the VMA.
			 */
			switch (p_icmp6->icmp6_type) {
			case NDISC_NEIGHBOUR_SOLICITATION:
			case NDISC_NEIGHBOUR_ADVERTISEMENT:
			{
				contig_len += 16 + sizeof(eth_icmp6_lladdr_t);
				if (pktlen < contig_len)
					return -EINVAL;

				phwaddr = (eth_icmp6_lladdr_t *)
						((u8 *)(p_icmp6 + 1) + 16);
				IEEE80211_ADDR_COPY(phwaddr->addr,
						    wvdev->wrap_dev_vma);
				p_icmp6->icmp6_cksum = 0;
				p_icmp6->icmp6_cksum = htons(checksum(
							     (u16)IPPROTO_ICMPV6,
							     icmp6len,
							     p_ip6->saddr.s6_addr,
							     p_ip6->daddr.s6_addr,
							     16 /* IPv6 has 32 byte addresses */,
							     (u8 *)p_icmp6));
				break;
			}
			case NDISC_ROUTER_SOLICITATION:
			{
				contig_len += sizeof(eth_icmp6_lladdr_t);
				if (pktlen < contig_len)
					return -EINVAL;

					/*replace the HW address with the VMA*/
					phwaddr = (eth_icmp6_lladdr_t *)
							((u8 *)(p_icmp6 + 1));
					break;
				}
			default:
				change_packet = 0;
				break;
			}

			if (change_packet) {
				IEEE80211_ADDR_COPY(phwaddr->addr,
						    wvdev->wrap_dev_vma);
				p_icmp6->icmp6_cksum = 0;
				p_icmp6->icmp6_cksum = htons(checksum((u16)IPPROTO_ICMPV6,
							     icmp6len,
							     p_ip6->saddr.s6_addr,
							     p_ip6->daddr.s6_addr,
							     /* IPv6 has 32 byte addresses */
							     16,
							     (u8 *)p_icmp6));
				qwrap_debug("IPV6 type %d\t sip:"
					    QDF_IPV6_ADDR_STR " \t hwaddr:"
					    QDF_MAC_ADDR_FMT "\t dip:"
					    QDF_IPV6_ADDR_STR "\t\n",
					    p_icmp6->icmp6_type,
					    QDF_IPV6_ADDR_ARRAY(p_ip6->saddr.s6_addr),
					    QDF_MAC_ADDR_REF(phwaddr->addr),
					    QDF_IPV6_ADDR_ARRAY(p_ip6->daddr.s6_addr));
			}
		}
	}
	IEEE80211_ADDR_COPY(src_mac, wvdev->wrap_dev_vma);
	return 0;
}

/**
 * @brief WRAP MAT on receive path.
 *
 * @param wrap_vdev handle.
 * @param buf
 *
 * @return 0 On success.
 * @return -ve On failure.
 */
int dp_wrap_mat_rx(struct dp_wrap_vdev *wvdev, wbuf_t buf)
{
	struct ether_header *eh;
	u16 ether_type;
	int contig_len = sizeof(struct ether_header);
	int pktlen = wbuf_get_pktlen(buf);
	u8 *src_mac, *des_mac, *p, ismcast;
	u8 *arp_smac = NULL;
	u8 *arp_dmac = NULL;
	struct eth_arphdr *parp = NULL;

	eh = (struct ether_header *)(wbuf_header(buf));
	p = (u8 *)(eh + 1);

	if (!wvdev->is_wrap && !wvdev->is_psta)
		return 0;

	if (pktlen < contig_len)
		return -EINVAL;

	ether_type = eh->ether_type;
	src_mac = eh->ether_shost;
	des_mac = eh->ether_dhost;
	ismcast = IEEE80211_IS_MULTICAST(des_mac);

	if (ether_type == htons(ETH_P_PAE)) {
		/* mark the pkt to allow local delivery on the device */
		buf->mark |= WRAP_MARK_ROUTE;
		return 0;
	}

	qwrap_trace("src mac:" QDF_MAC_ADDR_FMT " dst mac: "
		    " " QDF_MAC_ADDR_FMT " "
		    "ether type:%d", QDF_MAC_ADDR_REF(src_mac),
		    QDF_MAC_ADDR_REF(des_mac), ether_type);

	if (ether_type == htons(ETHERTYPE_ARP)) {
		parp = (struct eth_arphdr *)p;
		contig_len += sizeof(struct eth_arphdr);

		if (pktlen < contig_len)
			return -EINVAL;

		if (parp->ar_hln == ETH_ALEN &&
		    parp->ar_pro == htons(ETH_P_IP)) {
			arp_smac = parp->ar_sha;
			arp_dmac = parp->ar_tha;
		} else {
			parp = NULL;
		}
	}

	if (ismcast && wvdev->is_mpsta) {
		struct wlan_objmgr_pdev *pdev;
		struct wlan_objmgr_vdev *vdev = NULL;
		struct wlan_objmgr_vdev *vdev_next = NULL;
		struct wlan_objmgr_pdev_objmgr *objmgr;
		qdf_list_t *vdev_list;

		pdev = wvdev->vdev->vdev_objmgr.wlan_pdev;
		wlan_pdev_obj_lock(pdev);

		objmgr = &pdev->pdev_objmgr;
		vdev_list = &objmgr->wlan_vdev_list;
		/* Get first vdev */
		vdev = wlan_pdev_vdev_list_peek_head(vdev_list);

		while (vdev != NULL) {
			if (qdf_is_macaddr_equal((struct qdf_mac_addr *)src_mac,
			    (struct qdf_mac_addr *)vdev->vdev_mlme.macaddr)) {
				buf->mark |= WRAP_MARK_REFLECT;
				if (dp_wrap_vdev_is_mat_set(vdev))
					IEEE80211_ADDR_COPY(src_mac,
							    vdev->vdev_mlme.mataddr);
				wlan_pdev_obj_unlock(pdev);
				return 0;
			}
			/* get next vdev */
			vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list,
								    vdev);
			vdev = vdev_next;
		}

		wlan_pdev_obj_unlock(pdev);
	}

	if (parp) {
		qwrap_debug("ARP %s\t" QDF_IPV4_ADDR_STR "\t"
			    " " QDF_MAC_ADDR_FMT " "
			    "\t" QDF_IPV4_ADDR_STR "\t" QDF_MAC_ADDR_FMT "\n",
			    arps[qdf_ntohs(parp->ar_op)],
			    QDF_IPV4_ADDR_ARRAY(parp->ar_sip),
			    QDF_MAC_ADDR_REF(parp->ar_sha),
			    QDF_IPV4_ADDR_ARRAY(parp->ar_tip),
			    QDF_MAC_ADDR_REF(parp->ar_tha));
		if (!ismcast) {
			if (parp->ar_op == htons(ARPOP_REQUEST) ||
			    parp->ar_op == htons(ARPOP_REPLY)) {
				if (wvdev->mat_enabled) {
					IEEE80211_ADDR_COPY(arp_dmac,
							    wvdev->wrap_dev_oma);
				}
			}
		} else{
			struct wlan_objmgr_pdev *pdev;
			struct wlan_objmgr_vdev *vdev = NULL, *n_vdev = NULL;
			struct wlan_objmgr_vdev *vdev_next = NULL;
			struct wlan_objmgr_pdev_objmgr *objmgr;
			qdf_list_t *vdev_list;

			pdev = wvdev->vdev->vdev_objmgr.wlan_pdev;

			if (!qdf_is_macaddr_equal((struct qdf_mac_addr *)arp_dmac,
			    (struct qdf_mac_addr *)wvdev->wrap_dev_oma)) {
				wlan_pdev_obj_lock(pdev);

				objmgr = &pdev->pdev_objmgr;
				vdev_list = &objmgr->wlan_vdev_list;
				/* Get first vdev */
				vdev = wlan_pdev_vdev_list_peek_head(vdev_list);

				while (vdev != NULL) {
					if (dp_wrap_vdev_is_mat_set(vdev) &&
					    qdf_is_macaddr_equal((struct qdf_mac_addr *)arp_dmac,
					    (struct qdf_mac_addr *)vdev->vdev_mlme.macaddr)) {
						n_vdev = vdev;
						break;
					}
					/* get next vdev */
					vdev_next = wlan_vdev_get_next_vdev_of_pdev(vdev_list, vdev);
					vdev = vdev_next;
				}

				wlan_pdev_obj_unlock(pdev);
				if (n_vdev) {
					wvdev = dp_wrap_get_vdev_handle(n_vdev);
				} else {
					return 0;
				}
			}

			if (wvdev->mat_enabled == 0)
				return 0;

			IEEE80211_ADDR_COPY(arp_dmac, wvdev->wrap_dev_oma);
			return 0;
		}
	}

	else if (ether_type == htons(ETHERTYPE_IPV6)) {
		/* Add support later if needed */
	}

	if (!(ismcast) && !(wvdev->is_mpsta) && (wvdev->mat_enabled))
		IEEE80211_ADDR_COPY(des_mac, wvdev->wrap_dev_oma);

	return 0;
}
#endif
#endif
