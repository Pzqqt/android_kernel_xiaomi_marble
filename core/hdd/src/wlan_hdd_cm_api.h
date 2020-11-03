/*
 * Copyright (c) 2012-2020, The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_cm_api.h
 *
 * WLAN host device driver connect/disconnect functions declaration
 */

#ifndef __WLAN_HDD_CM_API_H
#define __WLAN_HDD_CM_API_H

#include <net/cfg80211.h>

#ifdef FEATURE_CM_ENABLE
/**
 * wlan_hdd_cm_connect() - cfg80211 connect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @req: Pointer to cfg80211 connect request
 *
 * This function is used to issue connect request to connection manager
 *
 * Context: Any context.
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cm_connect(struct wiphy *wiphy,
			struct net_device *ndev,
			struct cfg80211_connect_params *req);

/**
 * wlan_hdd_cm_disconnect() - cfg80211 disconnect api
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @reason: Disconnect reason code
 *
 * This function is used to issue disconnect request to conection manager
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cm_disconnect(struct wiphy *wiphy,
			   struct net_device *dev, u16 reason);

#else
static inline int
wlan_hdd_cm_connect(struct wiphy *wiphy, struct net_device *ndev,
		    struct cfg80211_connect_params *req)
{
	return 0;
}

static inline int
wlan_hdd_cm_disconnect(struct wiphy *wiphy, struct net_device *dev, u16 reason)
{
	return 0;
}
#endif
#endif
