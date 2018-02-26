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
 * DOC: wlan_cp_stats_ic_ucfg_defs.h
 *
 * This header file provide definitions and declarations required for northbound
 * specific to WIN
 */

#ifndef __WLAN_CP_STATS_IC_UCFG_DEFS_H__
#define __WLAN_CP_STATS_IC_UCFG_DEFS_H__

#ifdef QCA_SUPPORT_CP_STATS

/**
 * enum wlan_ucfg_cp_stats_vendorcmd_handler_idx - cp stats commands
 * @CP_STATS_GET_ATF_HANDLER_IDX: get ATF stats id
 * @CP_STATS_GET_VOW_STATS_HANDLER_IDX: get vow ext stats id
 * @CP_STATS_VENDOR_CMD_HANDLER_MAX: max command id
 */
enum wlan_ucfg_cp_stats_vendorcmd_handler_idx {
	CP_STATS_GET_ATF_HANDLER_IDX,
	CP_STATS_GET_VOW_STATS_HANDLER_IDX,
	CP_STATS_VENDOR_CMD_HANDLER_MAX,
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_UCFG_DEFS_H__ */
