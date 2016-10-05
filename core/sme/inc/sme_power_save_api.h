/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined(__SME_POWER_SAVE_API_H)
#define __SME_POWER_SAVE_API_H

#include "sme_power_save.h"
#include "ani_global.h"
#include "sme_inside.h"

QDF_STATUS sme_ps_enable_disable(tHalHandle hal_ctx, uint32_t session_id,
		enum sme_ps_cmd command);

QDF_STATUS sme_ps_uapsd_enable(tHalHandle hal_ctx, uint32_t session_id);

QDF_STATUS sme_ps_uapsd_disable(tHalHandle hal_ctx, uint32_t session_id);

/* Condition check if driver is ready to enter in PS */
QDF_STATUS sme_enable_sta_ps_check(tpAniSirGlobal mac_ctx, uint32_t session_id);

QDF_STATUS sme_ps_process_command(tpAniSirGlobal mac_ctx,
		uint32_t session_id,
		enum sme_ps_cmd command);

void sme_set_tspec_uapsd_mask_per_session(tpAniSirGlobal mac_ctx,
		tSirMacTSInfo *ts_info,
		uint8_t session_id);
/* Full Power Req Callback */
typedef void (*uapsd_start_indication_cb)(void *callback_context,
		uint32_t session_id, QDF_STATUS status);

QDF_STATUS sme_ps_start_uapsd(tHalHandle hal_ctx, uint32_t session_id,
		uapsd_start_indication_cb uapsd_start_ind_cb,
		void *callback_context);
QDF_STATUS sme_set_ps_host_offload(tHalHandle hal_ctx,
		tpSirHostOffloadReq request,
		uint8_t session_id);

#ifdef WLAN_NS_OFFLOAD
QDF_STATUS sme_set_ps_ns_offload(tHalHandle hal_ctx,
		tpSirHostOffloadReq request,
		uint8_t session_id);

#endif /* WLAN_NS_OFFLOAD */
/* / Post a message to PE module */
tSirRetStatus sme_post_pe_message(tpAniSirGlobal mac_ctx, tpSirMsgQ pMsg);

QDF_STATUS sme_ps_enable_auto_ps_timer(tHalHandle hal_ctx,
		uint32_t sessionId,
		uint32_t timeout);
QDF_STATUS sme_ps_disable_auto_ps_timer(tHalHandle hal_ctx,
		uint32_t sessionId);

QDF_STATUS sme_ps_open(tHalHandle hal_ctx);

QDF_STATUS sme_ps_open_per_session(tHalHandle hal_ctx, uint32_t session_id);

void sme_auto_ps_entry_timer_expired(void *ps_param);
QDF_STATUS sme_ps_close(tHalHandle hal_ctx);
QDF_STATUS sme_ps_close_per_session(tHalHandle hal_ctx, uint32_t sessionId);
#ifdef FEATURE_WLAN_SCAN_PNO
void sme_set_pno_channel_prediction(tpSirPNOScanReq request_buf,
		tpAniSirGlobal mac_ctx);
QDF_STATUS sme_set_ps_preferred_network_list(tHalHandle hal_ctx,
		tpSirPNOScanReq request,
		uint8_t session_id,
		preferred_network_found_ind_cb callback_routine,
		void *callback_context);
#else
static inline void sme_set_pno_channel_prediction(void *request_buf,
		tpAniSirGlobal mac_ctx)
{}
#endif

bool sme_is_auto_ps_timer_running(tHalHandle hal_ctx,
		uint32_t session_id);

#endif /* #if !defined(__SME_POWER_SAVE_API_H) */

