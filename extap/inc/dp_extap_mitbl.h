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

#ifndef __DP_EXTAP_MITBL_H_
#define __DP_EXTAP_MITBL_H_

#include "qdf_net_types.h"

#define ATH_MITBL_NONE 0x0000
#define ATH_MITBL_IPV4 0x0001
#define ATH_MITBL_IPV6 0x0002

#if MI_TABLE_AS_TREE
typedef struct mi_node { /* MAC - IP Node */
	struct mi_node *parent,
				   *left,
				   *right;
	u_int8_t h_dest[QDF_NET_ETH_LEN],
	len,
	ip_ver,
	ip[16];	/* v4 or v6 ip addr */
} mi_node_t;
#else
#define mi_node_is_free(n)	((n)->ip_ver == ATH_MITBL_NONE)
#define mi_node_free(n)	do { (n)->ip_ver = ATH_MITBL_NONE; } while (0)

typedef struct mi_node { /* MAC - IP Node */
	u_int8_t h_dest[QDF_NET_ETH_LEN],
	len,
	ip_ver,
	ip[16];	/* v4 or v6 ip addr */
} mi_node_t;
#endif /* MI_TABLE_AS_TREE */

#define minode_ip_len(n)	(((n)->ip_ver == ATH_MITBL_IPV4) ? 4 : 16)
#define mi_ip_len(n)		(((n) == ATH_MITBL_IPV4) ? 4 : 16)
#define mi_add		mi_tbl_add
#define mi_lkup		mi_tbl_lkup

mi_node_t *mi_tbl_add(mi_node_t **root, u_int8_t *ip, u_int8_t *mac, int ipver);
u_int8_t *mi_tbl_lkup(mi_node_t *root, u_int8_t *ip, int ipver);
void mi_tbl_del(mi_node_t **root, u_int8_t *ip, int ipver);
void mi_tbl_dump(void *arg);
void mi_tbl_purge(mi_node_t **root);
#endif /* __DP_EXTAP_MITBL_H_ */
