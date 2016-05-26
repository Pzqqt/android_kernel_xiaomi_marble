/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/**
 * DOC: wlan_hdd_nan_datapath.h
 *
 * WLAN Host Device Driver nan datapath API specification
 */

#ifndef __WLAN_HDD_NAN_DATAPATH_H
#define __WLAN_HDD_NAN_DATAPATH_H

struct hdd_context_s;

#ifdef WLAN_FEATURE_NAN_DATAPATH
void hdd_ndp_print_ini_config(struct hdd_context_s *hdd_ctx);
#else
static inline void hdd_ndp_print_ini_config(struct hdd_context_s *hdd_ctx)
{
}
#endif /* WLAN_FEATURE_NAN_DATAPATH */

#endif /* __WLAN_HDD_NAN_DATAPATH_H */

