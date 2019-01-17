/*
 * Copyright (c) 2011-2012, 2014-2019 The Linux Foundation. All rights reserved.
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
 *
 * This file lim_ibss_peer_mgmt.h contains prototypes for
 * the utility functions LIM uses to maintain peers in IBSS.
 * Author:        Chandra Modumudi
 * Date:          03/12/04
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "sir_common.h"
#include "lim_utils.h"

/**
 * ibss_bss_delete()- start the ibss
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
void ibss_bss_add(struct mac_context *mac_ctx, struct pe_session *session);

/**
 * ibss_bss_delete()- delete the current BSS
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
void ibss_bss_delete(struct mac_context *mac_ctx, struct pe_session *session);

/**
 * lim_ibss_delete_all_peers: delete all IBSS peers.
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
void lim_ibss_delete_all_peers(struct mac_context *mac_ctx, struct pe_session *session);

void lim_ibss_init(struct mac_context *);

/**
 * lim_ibss_delete() - Delete ibss while tearing down an IBSS
 *
 * @mac: Pointer to Global MAC structure
 * @session: Pointer to session entry
 *
 * Return: none
 */
void lim_ibss_delete(struct mac_context *, struct pe_session *pe_session);

QDF_STATUS lim_ibss_coalesce(struct mac_context *, tpSirMacMgmtHdr,
				tpSchBeaconStruct, uint8_t *, uint32_t, uint16_t,
				struct pe_session *);
QDF_STATUS lim_ibss_sta_add(struct mac_context *, void *, struct pe_session *);
QDF_STATUS lim_ibss_add_sta_rsp(struct mac_context *, void *, struct pe_session *);

/**
 * lim_process_ibss_del_sta_rsp()- Handle ibss delete
 * peer resp from firmware
 *
 * @mac_ptr: Pointer to Global MAC structure
 * @lim_msg: delete sta response
 * @pe_session: pe session
 *
 * Return: None
 *
 */
void lim_process_ibss_del_sta_rsp(struct mac_context *mac_ctx,
	struct scheduler_msg *lim_msg,
	struct pe_session *pe_session);
tLimIbssPeerNode *lim_ibss_peer_find(struct mac_context *mac, tSirMacAddr macAddr);
void lim_ibss_del_bss_rsp(struct mac_context *, void *, struct pe_session *);
void lim_ibss_del_bss_rsp_when_coalescing(struct mac_context *, void *, struct pe_session *);
void lim_ibss_add_bss_rsp_when_coalescing(struct mac_context *mac, void *msg,
					  struct pe_session *pe_session);
void lim_ibss_decide_protection_on_delete(struct mac_context *mac, tpDphHashNode sta,
					  tpUpdateBeaconParams pBeaconParams,
					  struct pe_session *pe_session);
void lim_ibss_heart_beat_handle(struct mac_context *mac, struct pe_session *pe_session);
void lim_process_ibss_peer_inactivity(struct mac_context *mac, void *buf);
