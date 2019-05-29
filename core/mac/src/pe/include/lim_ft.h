/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

/**=========================================================================

   Macros and Function prototypes FT and 802.11R purposes

   ========================================================================*/

#ifndef __LIMFT_H__
#define __LIMFT_H__

#include <cds_api.h>
#include <lim_global.h>
#include <ani_global.h>
#include <lim_ser_des_utils.h>

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/
void lim_ft_open(struct mac_context *mac, struct pe_session *pe_session);
void lim_ft_cleanup(struct mac_context *mac, struct pe_session *pe_session);
#ifdef WLAN_FEATURE_HOST_ROAM
void lim_ft_cleanup_pre_auth_info(struct mac_context *mac,
		struct pe_session *pe_session);
int lim_process_ft_pre_auth_req(struct mac_context *mac,
				struct scheduler_msg *pMsg);
void lim_process_ft_preauth_rsp_timeout(struct mac_context *mac);

/**
 * lim_process_mlm_ft_reassoc_req() - Handle the Reassoc request
 * @mac: Global MAC context
 * @reassoc_req: reassoc req
 *
 *  This function handles the Reassoc Req from SME
 *
 *  Return: None
 */
void lim_process_mlm_ft_reassoc_req(struct mac_context *mac,
				    tLimMlmReassocReq *reassoc_req);
void lim_perform_ft_pre_auth(struct mac_context *mac, QDF_STATUS status,
		uint32_t *data, struct pe_session *pe_session);
void lim_post_ft_pre_auth_rsp(struct mac_context *mac, QDF_STATUS status,
		uint8_t *auth_rsp, uint16_t auth_rsp_length,
		struct pe_session *pe_session);
void lim_handle_ft_pre_auth_rsp(struct mac_context *mac, QDF_STATUS status,
		uint8_t *auth_rsp, uint16_t auth_rsp_len,
		struct pe_session *pe_session);
QDF_STATUS lim_ft_setup_auth_session(struct mac_context *mac,
		struct pe_session *pe_session);
void lim_process_mlm_reassoc_cnf(struct mac_context *mac_ctx, uint32_t *msg);
void lim_process_sta_mlm_add_bss_rsp_ft(struct mac_context *mac,
		struct scheduler_msg *limMsgQ, struct pe_session *pe_session);
void lim_process_mlm_reassoc_req(struct mac_context *mac_ctx,
				 tLimMlmReassocReq *reassoc_req);
void lim_preauth_scan_event_handler(struct mac_context *mac_ctx,
				enum sir_scan_event_type event,
				uint8_t session_id,
				uint32_t scan_id);
QDF_STATUS lim_send_preauth_scan_offload(struct mac_context *mac_ctx,
		struct pe_session *session_entry, tSirFTPreAuthReq *ft_preauth_req);
#else
static inline void lim_ft_cleanup_pre_auth_info(struct mac_context *mac,
		struct pe_session *pe_session)
{}
static inline void lim_process_ft_preauth_rsp_timeout(struct mac_context *mac)
{}
static inline
void lim_process_mlm_ft_reassoc_req(struct mac_context *mac,
				    tLimMlmReassocReq *reassoc_req)
{}
static inline void lim_handle_ft_pre_auth_rsp(struct mac_context *mac,
		QDF_STATUS status, uint8_t *auth_rsp,
		uint16_t auth_rsp_len, struct pe_session *pe_session)
{}
static inline void lim_process_mlm_reassoc_cnf(struct mac_context *mac_ctx,
		uint32_t *msg)
{}
static inline void lim_process_sta_mlm_add_bss_rsp_ft(struct mac_context *mac,
		struct scheduler_msg *limMsgQ, struct pe_session *pe_session)
{}
static inline void lim_process_mlm_reassoc_req(struct mac_context *mac_ctx,
					       tLimMlmReassocReq *reassoc_req)
{}
static inline void lim_preauth_scan_event_handler(struct mac_context *mac_ctx,
		enum sir_scan_event_type event,
		uint8_t session_id, uint32_t scan_id)
{}
static inline int lim_process_ft_pre_auth_req(struct mac_context *mac,
		struct scheduler_msg *pMsg)
{
	return 0;
}
#endif

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
void lim_fill_ft_session(struct mac_context *mac,
		struct bss_description *pbssDescription,
		struct pe_session *ft_session,
		struct pe_session *pe_session);

void lim_ft_prepare_add_bss_req(struct mac_context *mac, uint8_t updateEntry,
		struct pe_session *ft_session,
		struct bss_description *bssDescription);

QDF_STATUS lim_send_preauth_scan_offload(struct mac_context *mac_ctx,
		struct pe_session *session_entry, tSirFTPreAuthReq *ft_preauth_req);
#else
static inline void lim_fill_ft_session(struct mac_context *mac,
		struct bss_description *pbssDescription,
		struct pe_session *ft_session,
		struct pe_session *pe_session)
{}
static inline void lim_ft_prepare_add_bss_req(struct mac_context *mac,
		uint8_t updateEntry, struct pe_session *ft_session,
		struct bss_description *bssDescription)
{}
#endif

bool lim_process_ft_update_key(struct mac_context *mac, uint32_t *msg_buf);
QDF_STATUS lim_process_ft_aggr_qos_req(struct mac_context *mac,
		uint32_t *msg_buf);
void lim_process_ft_aggr_qos_rsp(struct mac_context *mac,
				 struct scheduler_msg *limMsg);
void lim_ft_cleanup_all_ft_sessions(struct mac_context *mac);
#endif /* __LIMFT_H__ */
