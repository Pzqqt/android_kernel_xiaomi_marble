/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
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

/**
 * DOC : contains interface prototypes for son api
 */
#ifndef _SON_API_H_
#define _SON_API_H_

#include <qdf_types.h>
#include <wlan_objmgr_pdev_obj.h>
#include <reg_services_public_struct.h>

/**
 * wlan_son_get_chan_flag() - get chan flag
 * @pdev: pointer to pdev
 * @freq: qdf_freq_t
 * @flag_160: If true, 160 channel info will be obtained;
 *            otherwise 80+80, 80 channel info will be obtained
 * @chan_params: chan parameters
 *
 * Return: combination of enum qca_wlan_vendor_channel_prop_flags and
 *         enum qca_wlan_vendor_channel_prop_flags_2
 */
uint32_t wlan_son_get_chan_flag(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq, bool flag_160,
				struct ch_params *chan_params);
#endif
