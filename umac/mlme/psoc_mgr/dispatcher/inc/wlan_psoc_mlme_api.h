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
 * DOC: Define PSOC MLME public APIs
 */

#ifndef _WLAN_PSOC_MLME_API_H_
#define _WLAN_PSOC_MLME_API_H_

/**
 * wlan_psoc_mlme_get_cmpt_obj() - Returns PSOC MLME component object
 * @psoc: PSOC object
 *
 * Retrieves MLME component object from PSOC object
 *
 * Return: comp handle on SUCCESS
 *         NULL, if it fails to retrieve
 */
struct psoc_mlme_obj *wlan_psoc_mlme_get_cmpt_obj(
						struct wlan_objmgr_psoc *psoc);

/**
 * wlan_psoc_mlme_get_ext_hdl() - Returns legacy handle
 * @psoc: PSOC object
 *
 * Retrieves legacy handle from psoc mlme component object
 *
 * Return: legacy handle on SUCCESS
 *         NULL, if it fails to retrieve
 */
mlme_psoc_ext_t *wlan_psoc_mlme_get_ext_hdl(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_psoc_mlme_set_ext_hdl() - Set legacy handle
 * @psoc_mlme: psoc_mlme object
 * psoc_ext_hdl: PSOC level legacy handle
 *
 * Sets legacy handle in psoc mlme component object
 *
 * Return: Void
 */
void wlan_psoc_mlme_set_ext_hdl(struct psoc_mlme_obj *psoc_mlme,
				mlme_psoc_ext_t *psoc_ext_hdl);

#endif
