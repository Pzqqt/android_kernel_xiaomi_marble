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

#ifndef _DP_WRAP_STRUCT_H_
#define _DP_WRAP_STRUCT_H_

#if ATH_SUPPORT_WRAP
#if !WLAN_QWRAP_LEGACY

#include <wbuf.h>

#define	WRAP_DEV_HASHSIZE	32	/*dev hash table size*/

/*device table simple hash function*/
#define	WRAP_DEV_HASH(addr)   \
	(((const u_int8_t *)(addr))[QDF_MAC_ADDR_SIZE - 1] % WRAP_DEV_HASHSIZE)

#define qwrap_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_WRAP, ## params)
#define qwrap_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_WRAP, ## params)
#define qwrap_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_WRAP, ## params)
#define qwrap_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_WRAP, ## params)
#define qwrap_trace(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_WRAP, ## params)

#define WRAP_ISOLATION_DEFVAL 0

typedef rwlock_t wrap_devt_lock_t;

/*wrap device table*/
typedef struct dp_wrap_devt {
	struct dp_wrap_pdev		*wrap_pdev;
					/*back ptr to wrap pdev*/
	wrap_devt_lock_t		wdt_lock;
					/*lock for dev table*/
	TAILQ_HEAD(, dp_wrap_vdev)	wdt_dev;	/*head for device list*/
	ATH_LIST_HEAD(, dp_wrap_vdev)	wdt_hash[WRAP_DEV_HASHSIZE];
					/*head for device hash*/
	TAILQ_HEAD(, dp_wrap_vdev)	wdt_dev_vma;	/*head for device list*/
	ATH_LIST_HEAD(, dp_wrap_vdev)	wdt_hash_vma[WRAP_DEV_HASHSIZE];
					/*head for device hash*/
} dp_wrap_devt_t;

/*wrap pdev struct*/
typedef struct dp_wrap_pdev {
	struct dp_wrap_devt	wp_devt;        /*wrap device table*/
	u_int8_t                wp_isolation;
	int                     wp_use_cnt;     /*wrap pdev use cnt*/
	struct wlan_objmgr_vdev       *mpsta_vdev;
	struct wlan_objmgr_vdev        *wrap_vdev;
	struct net_device       *mpsta_dev;
	u_int8_t                nwrapvaps;     /* Number of active WRAP APs */
	u_int8_t                npstavaps;
} dp_pdev_wrap_t;

/*wrap vdev struct*/
typedef struct dp_wrap_vdev {
	struct dp_wrap_pdev	*wrap_pdev;        /*back ptr to wrap pdev*/
	struct net_device       *dev;
	struct wlan_objmgr_vdev *vdev;
	bool                    is_wrap;
	bool                    is_mpsta;
	bool                    is_psta;
	bool                    is_wired_psta;
	bool                    mat_enabled;
	unsigned char       wrap_dev_oma[ETH_ALEN]; /* dev oma mac address */
	unsigned char       wrap_dev_vma[ETH_ALEN]; /* dev vma mac address */
	TAILQ_ENTRY(dp_wrap_vdev)  wrap_dev_list;	/*wrap oma dev list entry*/
	LIST_ENTRY(dp_wrap_vdev)   wrap_dev_hash;	/*wrap oma hash list entry*/
	TAILQ_ENTRY(dp_wrap_vdev)  wrap_dev_list_vma;  /*wrap vma dev list*/
	LIST_ENTRY(dp_wrap_vdev)   wrap_dev_hash_vma;  /*wrap vma hash list */
	void (*wlan_vdev_xmit_queue)(struct net_device *dev, wbuf_t wbuf);
} dp_vdev_wrap_t;
#endif
#endif
#endif
