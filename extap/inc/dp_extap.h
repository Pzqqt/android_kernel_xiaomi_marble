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

#ifndef __DP_EXTAP_H_
#define __DP_EXTAP_H_

#include "dp_txrx.h"
#include <cdp_txrx_cmn.h>
#include "dp_extap_mitbl.h"
#include <wlan_osif_priv.h>

#define eamstr		"%02x:%02x:%02x:%02x:%02x:%02x"
#define eamac(a)    (a)[0], (a)[1], (a)[2],\
					(a)[3], (a)[4], (a)[5]
#define eaistr		"%u.%u.%u.%u"
#define eaip(a)     ((a)[0] & 0xff), ((a)[1] & 0xff),\
					((a)[2] & 0xff), ((a)[3] & 0xff)
#define eastr6      "%02x%02x:%02x%02x:%02x%02x:"\
					"%02x%02x:%02x%02x:%02x%02x:"\
					"%02x%02x:%02x%02x"
#define eaip6(a)    (a)[0], (a)[1], (a)[2], (a)[3],\
					(a)[4], (a)[5], (a)[6], (a)[7],\
					(a)[8], (a)[9], (a)[10], (a)[11],\
					(a)[12], (a)[13], (a)[14], (a)[15]

#ifdef EXTAP_DEBUG
#define extap_log(level, args...)                       \
	QDF_TRACE(QDF_MODULE_ID_EXT_AP, level, ##args)
#define extap_logl(level, format, args...)              \
	extap_log(level, FL(format), ##args)
#define extap_debug(format, args ...)                   \
	extap_logl(QDF_TRACE_LEVEL_DEBUG, format, ##args)
#define extap_info(format, args ...)                    \
	extap_logl(QDF_TRACE_LEVEL_INFO, format, ##args)
#define extap_warn(format, args ...)                    \
	extap_logl(QDF_TRACE_LEVEL_WARN, format, ##args)
#define extap_err(format, args ...)                     \
	extap_logl(QDF_TRACE_LEVEL_ERROR, format, ##args)
#define extap_alert(format, args ...)                   \
	extap_logl(QDF_TRACE_LEVEL_FATAL, format, ##args)
#define extap_debug_mac(b, c)                           \
	do {                                                \
		extap_debug("Replacing " #b " "                 \
				eamstr " with " eamstr "\n",            \
				eamac(b), eamac(c));                    \
	} while (0)
#define extap_debug_ip(b, c, i)                         \
	do {                                                \
		extap_debug("Replacing " #b " "                 \
				eamstr " with " eamstr                  \
				" for " eaistr "\n",                    \
				eamac(b), eamac(c), eaip(i));           \
	} while (0)
#define print_arp(a)  print_arp_pkt(a)
#define print_ipv6    print_ipv6_pkt
#else
#define extap_debug(format, args ...)
#define extap_info(format, args ...)
#define extap_warn(format, args ...)
#define extap_err(format, args ...)
#define extap_alert(format, args ...)
#define extap_debug_mac(b, c)
#define extap_debug_ip(b, c, i)
#define print_arp(...)
#define print_ipv6(...)
#endif /* EXTAP_DEBUG */

#ifdef BIG_ENDIAN_HOST
#define DHCP_FLAGS 0x8000
#else
#define DHCP_FLAGS 0x0080
#endif

struct dp_extap_nssol {
	uint16_t ip_version;
	uint8_t mac[ETH_ALEN];
	union {
		uint8_t ipv4[QDF_IPV4_ADDR_SIZE];
		uint8_t ipv6[QDF_IPV6_ADDR_SIZE];
	} ip;
};

extern void transcap_nwifi_to_8023(qdf_nbuf_t msdu);
int dp_extap_input(dp_pdev_extap_t *extap, uint8_t *macaddr,
				  qdf_ether_header_t *eh);
int dp_extap_output(dp_pdev_extap_t *extap, uint8_t *macaddr,
				   qdf_ether_header_t *eh, struct dp_extap_nssol *extap_nssol);

/**
 * dp_extap_mitbl_dump() - dump extap miroot table
 * @extap: dp_pdev_extap_t pointer
 *
 * Return: void
 */
void dp_extap_mitbl_dump(dp_pdev_extap_t *extap);

/**
 * dp_extap_mitbl_purge() - purge extap miroot table
 * @extap: dp_pdev_extap_t pointer
 *
 * Return: void
 */
void dp_extap_mitbl_purge(dp_pdev_extap_t *extap);

/**
 * dp_extap_enable() - Enable extap
 * @vdev: vdev object pointer
 *
 * Return: void
 */
void dp_extap_enable(struct wlan_objmgr_vdev *vdev);

/**
 * dp_extap_disable() - Disable extap
 * @vdev: vdev object pointer
 *
 * Return: void
 */
void dp_extap_disable(struct wlan_objmgr_vdev *vdev);

/**
 * dp_is_extap_enabled() - check if extap is enabled
 * @vdev: vdev object pointer
 *
 * Return: 0 if not enabled else 1
 */
static inline uint8_t dp_is_extap_enabled(struct wlan_objmgr_vdev *vdev)
{
	return wlan_vdev_mlme_feat_cap_get(vdev, WLAN_VDEV_F_AP);
}

/**
 * dp_get_extap_handle() - get extap handle from vdev
 * @vdev: vdev object pointer
 *
 * Return: extap handle
 */
static inline
dp_pdev_extap_t *dp_get_extap_handle(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *wlan_pdev;
	dp_txrx_pdev_handle_t *dp_hdl;
	ol_txrx_soc_handle soc;

	wlan_pdev = wlan_vdev_get_pdev(vdev);

	soc = wlan_psoc_get_dp_handle(wlan_pdev_get_psoc(wlan_pdev));
	if (!soc)
		return NULL;
	dp_hdl = cdp_pdev_get_dp_txrx_handle(soc,
						wlan_objmgr_pdev_get_pdev_id(wlan_pdev));
	if (!dp_hdl)
		return NULL;

	return &dp_hdl->extap_hdl;
}

/**
 * dp_pdev_get_extap_handle() - get extap handle from pdev
 * @pdev: pdev object pointer
 *
 * Return: extap handle
 */
static
dp_pdev_extap_t *dp_pdev_get_extap_handle(struct wlan_objmgr_pdev *pdev)
{
	dp_txrx_pdev_handle_t *dp_hdl;
	ol_txrx_soc_handle soc;

	soc = wlan_psoc_get_dp_handle(wlan_pdev_get_psoc(pdev));
	if (!soc)
		return NULL;
	dp_hdl = cdp_pdev_get_dp_txrx_handle(soc, wlan_objmgr_pdev_get_pdev_id(pdev));
	if (!dp_hdl)
		return NULL;

	return &dp_hdl->extap_hdl;
}

int dp_extap_attach(struct wlan_objmgr_pdev *pdev);
int dp_extap_detach(struct wlan_objmgr_pdev *pdev);
#endif /* __DP_EXTAP_H_ */
