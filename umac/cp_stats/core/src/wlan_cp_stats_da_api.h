/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_da_api.h
 *
 * This header file provide declarations for cp stats global context object
 * initialization specific to DA
 */
#ifndef __WLAN_CP_STATS_DA_API_H__
#define __WLAN_CP_STATS_DA_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include "wlan_cp_stats_defs.h"
#include "wlan_cp_stats_da_api.h"

/**
 * wlan_cp_stats_ctx_init_da() - initialize cp stats global context
 * @pdev: Pointer to cp stats global context object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_ctx_init_da(struct cp_stats_context *csc);

/**
 * wlan_cp_stats_ctx_deinit_da() - deinitialize cp stats global context
 * @pdev: Pointer to cp stats global context object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_ctx_deinit_da(struct cp_stats_context *csc);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_DA_API_H__ */
