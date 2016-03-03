/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_net_types
 * This file provides OS dependent net types API's.
 */

#ifndef _I_QDF_NET_TYPES_H
#define _I_QDF_NET_TYPES_H

#include <qdf_types.h>          /* uint8_t, etc. */
#include <asm/checksum.h>
#include <net/ip6_checksum.h>

typedef struct in6_addr __in6_addr_t;
typedef __wsum __wsum_t;

static inline  int32_t __qdf_csum_ipv6(const struct in6_addr *saddr,
				       const struct in6_addr *daddr,
				       __u32 len, unsigned short proto,
				       __wsum sum)
{
	return csum_ipv6_magic((struct in6_addr *)saddr,
			       (struct in6_addr *)daddr, len, proto, sum);
}

#endif /* _I_QDF_NET_TYPES_H */
