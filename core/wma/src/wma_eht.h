/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef __WMA_EHT_H
#define __WMA_EHT_H

#include "wma.h"

#ifdef WLAN_FEATURE_11BE
/*
 * wma_eht_update_tgt_services() - update tgt cfg to indicate 11be support
 * @wmi_handle: pointer to WMI handle
 * @cfg: pointer to WMA target services
 *
 * Based on WMI SERVICES information, enable 11be support and set DOT11BE
 * bit in feature caps bitmap.
 *
 * Return: None
 */
void wma_eht_update_tgt_services(struct wmi_unified *wmi_handle,
				 struct wma_tgt_services *cfg);
/**
 * wma_update_target_ext_eht_cap() - Update EHT caps with given extended cap
 * @tgt_hdl: target psoc information
 * @tgt_cfg: Target config
 *
 * This function loop through each hardware mode and for each hardware mode
 * again it loop through each MAC/PHY and pull the caps 2G and 5G specific
 * EHT caps and derives the final cap.
 *
 * Return: None
 */
void wma_update_target_ext_eht_cap(struct target_psoc_info *tgt_hdl,
				   struct wma_tgt_cfg *tgt_cfg);

void wma_update_vdev_eht_ops(uint32_t *eht_ops, tDot11fIEeht_op *eht_op);
#else
static inline void wma_eht_update_tgt_services(struct wmi_unified *wmi_handle,
					       struct wma_tgt_services *cfg)
{
		cfg->en_11be = false;
		return;
}

static inline
void wma_update_target_ext_eht_cap(struct target_psoc_info *tgt_hdl,
				   struct wma_tgt_cfg *tgt_cfg)
{
}

static inline
void wma_update_vdev_eht_ops(uint32_t *eht_ops, tDot11fIEeht_op *eht_op)
{
}
#endif
#endif
