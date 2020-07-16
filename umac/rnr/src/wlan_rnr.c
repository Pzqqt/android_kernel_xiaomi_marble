/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#include <wlan_rnr.h>
#include <qdf_module.h>
#include <qdf_status.h>
#include <qdf_types.h>

struct rnr_global_info g_rnr_info;

void wlan_rnr_init_cnt(void)
{
	qdf_atomic_init(&(g_rnr_info.vdev_lower_band_cnt));
	qdf_atomic_init(&(g_rnr_info.vdev_6ghz_band_cnt));
}

qdf_export_symbol(wlan_rnr_init_cnt);

void wlan_rnr_lower_band_vdev_inc(void)
{
	qdf_atomic_inc(&(g_rnr_info.vdev_lower_band_cnt));
}

qdf_export_symbol(wlan_rnr_lower_band_vdev_inc);

void wlan_rnr_lower_band_vdev_dec(void)
{
	qdf_atomic_dec(&(g_rnr_info.vdev_lower_band_cnt));
}

qdf_export_symbol(wlan_rnr_lower_band_vdev_dec);

void wlan_rnr_6ghz_vdev_inc(void)
{
	qdf_atomic_inc(&(g_rnr_info.vdev_6ghz_band_cnt));
}

qdf_export_symbol(wlan_rnr_6ghz_vdev_inc);

void wlan_rnr_6ghz_vdev_dec(void)
{
	qdf_atomic_dec(&(g_rnr_info.vdev_6ghz_band_cnt));
}

qdf_export_symbol(wlan_rnr_6ghz_vdev_dec);

void wlan_global_6ghz_pdev_set(struct wlan_objmgr_pdev *pdev)
{
	if (pdev)
		g_rnr_info.pdev_6ghz_ctx = pdev;
}

qdf_export_symbol(wlan_global_6ghz_pdev_set);

void wlan_global_6ghz_pdev_destroy(void)
{
	g_rnr_info.pdev_6ghz_ctx = NULL;
}

qdf_export_symbol(wlan_global_6ghz_pdev_destroy);

int32_t wlan_lower_band_ap_cnt_get(void)
{
	return qdf_atomic_read(&(g_rnr_info.vdev_lower_band_cnt));
}

qdf_export_symbol(wlan_lower_band_ap_cnt_get);

struct wlan_objmgr_pdev *wlan_gbl_6ghz_pdev_get(void)
{
	return g_rnr_info.pdev_6ghz_ctx;
}

qdf_export_symbol(wlan_gbl_6ghz_pdev_get);

void wlan_rnr_set_bss_idx(uint32_t bss_idx)
{
	g_rnr_info.rnr_mbss_idx_map |= (1 << (bss_idx-1));
}

uint32_t wlan_rnr_get_bss_idx(void)
{
	return g_rnr_info.rnr_mbss_idx_map;
}

void  wlan_rnr_clear_bss_idx(void)
{
	g_rnr_info.rnr_mbss_idx_map = 0;
}
