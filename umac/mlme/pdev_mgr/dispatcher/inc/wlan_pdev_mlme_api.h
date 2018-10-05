/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: Define PDEV MLME public APIs
 */

#ifndef _WLAN_PDEV_MLME_API_H_
#define _WLAN_PDEV_MLME_API_H_

/**
 * wlan_pdev_mlme_get_cmpt_obj - Returns MLME component object
 *
 * Retrieves MLME component object from PDEV object
 *
 * Return: comp handle on SUCCESS
 *         NULL, if it fails to retrieve
 */
struct pdev_mlme_obj *wlan_pdev_mlme_get_cmpt_obj(
						struct wlan_objmgr_pdev *pdev);
/**
 * wlan_pdev_mlme_get_ext_hdl - Returns legacy handle
 *
 * Retrieves legacy handle from pdev mlme component object
 *
 * Return: legacy handle on SUCCESS
 *         NULL, if it fails to retrieve
 */
void *wlan_pdev_mlme_get_ext_hdl(struct wlan_objmgr_pdev *pdev);

#endif
