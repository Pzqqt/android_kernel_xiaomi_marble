/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_CONVERGED_H
#define __CFG_CONVERGED_H

#include <wlan_scan_cfg.h>
#include "cfg_mlme_score_params.h"
#include "cfg_dp.h"
#include "cfg_hif.h"
#include <wlan_extscan_cfg.h>
#ifdef WLAN_SUPPORT_GREEN_AP
#include "cfg_green_ap_params.h"
#else
#define CFG_GREEN_AP_ALL
#endif
#include <cfg_spectral.h>
#ifdef DCS_INTERFERENCE_DETECTION
#include "wlan_dcs_cfg.h"
#else
#define CFG_DCS_ALL
#endif
#ifdef WLAN_CFR_ENABLE
#include "cfr_cfg.h"
#else
#define CFG_CFR_ALL
#endif

#define CFG_CONVERGED_ALL \
		CFG_SCAN_ALL \
		CFG_DP \
		CFG_EXTSCAN_ALL \
		CFG_GREEN_AP_ALL \
		CFG_SPECTRAL_ALL \
		CFG_HIF \
		CFG_DCS_ALL \
		CFG_CFR_ALL \
		CFG_MLME_SCORE_ALL

#endif /* __CFG_CONVERGED_H */

