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

#ifdef QCA_IBSS_SUPPORT
/**
 * ibss_bss_add()- add the ibss
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
 * lim_ibss_delete_all_peers() - delete all IBSS peers.
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
void lim_ibss_delete_all_peers(struct mac_context *mac_ctx, struct pe_session *session);

/**
 * lim_ibss_init() - API to init IBSS
 * @mac_ctx: Pointer to Global MAC structure
 *
 * Return: None
 */
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

/**
 * lim_ibss_coalesce() - API to process IBSS Beacon/Probe Response
 * @param  mac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 *
 * This function is called upon receiving Beacon/Probe Response
 * while operating in IBSS mode.
 *
 * @return Status whether to process or ignore received Beacon Frame
 */
QDF_STATUS lim_ibss_coalesce(struct mac_context *, tpSirMacMgmtHdr,
				tpSchBeaconStruct, uint8_t *, uint32_t, uint16_t,
				struct pe_session *);

/**
 * lim_ibss_sta_add() - API to add an STA context in IBSS role
 * @param  mac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 *
 * This function is called to add an STA context in IBSS role
 * whenever a data frame is received from/for a STA that failed
 * hash lookup at DPH.
 *
 * @return retCode Indicates success or failure return code
 */
QDF_STATUS lim_ibss_sta_add(struct mac_context *, void *, struct pe_session *);

/**
 * lim_ibss_add_sta_rsp() - API to process STA add response in IBSS role
 * @param  mac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 *
 * This function is called to add an STA context in IBSS role
 * whenever a data frame is received from/for a STA that failed
 * hash lookup at DPH.
 *
 * @return retCode Indicates success or failure return code
 */
QDF_STATUS lim_ibss_add_sta_rsp(struct mac_context *, void *, struct pe_session *);

/**
 * lim_process_ibss_del_sta_rsp()- Handle ibss delete
 * peer resp from firmware
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

/**
 * lim_ibss_peer_find() - API to find IBSS peer
 * @mac: pointer to mac context
 * @param  macAddr - MAC address of the peer
 *
 * This function is called while adding a context at
 * DPH & Polaris for a peer in IBSS.
 * If peer is found in the list, capabilities from the
 * returned BSS description are used at DPH node & Polaris.
 *
 * @return Pointer to peer node if found, else NULL
 */
tLimIbssPeerNode *lim_ibss_peer_find(struct mac_context *mac, tSirMacAddr macAddr);

/**
 * lim_ibss_del_bss_rsp() - Handle ibss delete
 * response
 * @mac: Pointer to Global MAC structure
 * @vdev_stop_rsp: pointer to del bss response
 * @pe_session: pointer to pe session
 *
 * Return: None
 *
 */
void lim_ibss_del_bss_rsp(struct mac_context *mac,
			  struct del_bss_resp *vdev_stop_rsp,
			  struct pe_session *pe_session);

/**
 * lim_ibss_del_bss_rsp_when_coalescing() - Handle ibss delete
 * response when coalescing
 * @mac: Pointer to Global MAC structure
 * @vdev_stop_rsp: pointer to del bss response
 * @pe_session: pointer to pe session
 *
 * Return: None
 *
 */
void lim_ibss_del_bss_rsp_when_coalescing(struct mac_context *mac,
					  struct del_bss_resp *vdev_stop_rsp,
					  struct pe_session *pe_session);

/**
 * lim_ibss_add_bss_rsp_when_coalescing()- Handle ADD BSS rsp of IBSS coalescing
 * @mac_ptr: Pointer to Global MAC structure
 * @op_chan_freq: operating chan freq
 * @pe_session: PE session pointer
 *
 * Return: None
 */
void lim_ibss_add_bss_rsp_when_coalescing(struct mac_context *mac,
					  uint32_t op_chan_freq,
					  struct pe_session *pe_session);

/**
 * lim_ibss_decide_protection_on_delete() - decides protection related info.
 * @mac_ctx: global mac context
 * @stads: station hash node
 * @bcn_param: beacon parameters
 * @session: PE session entry
 *
 * Decides all the protection related information.
 *
 * Return: None
 */
void lim_ibss_decide_protection_on_delete(struct mac_context *mac, tpDphHashNode sta,
					  tpUpdateBeaconParams pBeaconParams,
					  struct pe_session *pe_session);

/**
 * lim_ibss_heart_beat_handle() - handle IBSS hearbeat failure
 * @mac_ctx: global mac context
 * @session: PE session entry
 *
 * Hanlde IBSS hearbeat failure.
 *
 * Return: None.
 */
void lim_ibss_heart_beat_handle(struct mac_context *mac, struct pe_session *pe_session);

/**
 * lim_process_ibss_peer_inactivity() - Handle inactive IBSS peer
 * @mac: Global MAC context
 * @buf: pointer to buffer
 *
 * Internal function. Deletes FW indicated peer which is inactive
 *
 * Return: None
 */
void lim_process_ibss_peer_inactivity(struct mac_context *mac, void *buf);
#else
/**
 * ibss_bss_add()- api to add ibss
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
static inline
void ibss_bss_add(struct mac_context *mac_ctx, struct pe_session *session)
{
}

/**
 * ibss_bss_delete()- delete the current BSS
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
static inline
void ibss_bss_delete(struct mac_context *mac_ctx, struct pe_session *session)
{
}

/**
 * lim_ibss_delete_all_peers: delete all IBSS peers.
 * @mac_ctx: Pointer to Global MAC structure
 * @session: pe session
 *
 * Return: None
 */
static inline
void lim_ibss_delete_all_peers(struct mac_context *mac_ctx,
			       struct pe_session *session)
{
}

/**
 * lim_ibss_init() - API to init IBSS
 * @mac_ctx: Pointer to Global MAC structure
 *
 * This function is dummy.
 *
 * Return: None
 */
static inline
void lim_ibss_init(struct mac_context *mac)
{
}

/**
 * lim_ibss_delete() - Delete ibss while tearing down an IBSS
 * @mac: Pointer to Global MAC structure
 * @session: Pointer to session entry
 *
 * Return: none
 */
static inline
void lim_ibss_delete(struct mac_context *mac, struct pe_session *session)
{
}

/**
 * lim_ibss_coalesce() - API to process IBSS Beacon/Probe Response
 * @param  mac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 *
 * This function is dummy.
 *
 * @return Status whether to process or ignore received Beacon Frame
 */
static inline
QDF_STATUS lim_ibss_coalesce(struct mac_context *mac,
			     tpSirMacMgmtHdr pHdr,
			     tpSchBeaconStruct pBeacon,
			     uint8_t *pIEs,
			     uint32_t ieLen, uint16_t fTsfLater,
			     struct pe_session *pe_session)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_ibss_sta_add() - API to add an STA context in IBSS role
 * @param  mac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 *
 * This function is dummy.
 *
 * @return retCode Indicates success or failure return code
 */
static inline
QDF_STATUS lim_ibss_sta_add(struct mac_context *mac, void *pBody,
			    struct pe_session *pe_session)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_ibss_add_sta_rsp() - API to process STA add response in IBSS role
 * @param  mac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 *
 * This function is funny.
 *
 * @return retCode Indicates success or failure return code
 */
static inline
QDF_STATUS lim_ibss_add_sta_rsp(struct mac_context *mac,
				void *msg, struct pe_session *pe_session)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_process_ibss_del_sta_rsp()- Handle ibss delete
 * peer resp from firmware
 * @mac_ptr: Pointer to Global MAC structure
 * @lim_msg: delete sta response
 * @pe_session: pe session
 *
 * This function is dymmy.
 *
 * Return: None
 */
static inline
void lim_process_ibss_del_sta_rsp(struct mac_context *mac_ctx,
				  struct scheduler_msg *lim_msg,
				  struct pe_session *pe_session)
{
}

/**
 * lim_ibss_peer_find() - API to find IBSS peer
 * @mac: pointer to mac context
 * @param  macAddr - MAC address of the peer
 *
 * This function is dymmy.
 *
 * @return Pointer to peer node if found, else NULL
 */
static inline
tLimIbssPeerNode *lim_ibss_peer_find(struct mac_context *mac,
				     tSirMacAddr macAddr)
{
	return NULL;
}

/**
 * lim_ibss_del_bss_rsp() - Handle ibss delete
 * response
 * @mac: Pointer to Global MAC structure
 * @vdev_stop_rsp: pointer to del bss response
 * @pe_session: pointer to pe session
 *
 * This function is dymmy.
 *
 * Return: None
 */
static inline
void lim_ibss_del_bss_rsp(struct mac_context *mac,
			  struct del_bss_resp *vdev_stop_rsp,
			  struct pe_session *pe_session)
{
}

/**
 * lim_ibss_del_bss_rsp_when_coalescing() - Handle ibss delete
 * response when coalescing
 * @mac: Pointer to Global MAC structure
 * @vdev_stop_rsp: pointer to del bss response
 * @pe_session: pointer to pe session
 *
 * Return: None
 *
 */
static inline
void lim_ibss_del_bss_rsp_when_coalescing(struct mac_context *mac,
					  struct del_bss_resp *vdev_stop_rsp,
					  struct pe_session *pe_session)
{
}

/**
 * lim_ibss_add_bss_rsp_when_coalescing()- Handle ADD BSS rsp of IBSS coalescing
 * @mac_ptr: Pointer to Global MAC structure
 * @op_chan_freq: operating chan freq
 * @pe_session: PE session pointer
 *
 * Return: None
 */
static inline
void lim_ibss_add_bss_rsp_when_coalescing(struct mac_context *mac,
					  uint32_t op_chan_freq,
					  struct pe_session *pe_session)
{
}

/**
 * lim_ibss_decide_protection_on_delete() - decides protection related info.
 * @mac_ctx: global mac context
 * @stads: station hash node
 * @bcn_param: beacon parameters
 * @session: PE session entry
 *
 * This function is dummy.
 *
 * Return: None
 */
static inline
void lim_ibss_decide_protection_on_delete(struct mac_context *mac,
					  tpDphHashNode sta,
					  tpUpdateBeaconParams pBeaconParams,
					  struct pe_session *pe_session)
{
}

/**
 * lim_ibss_heart_beat_handle() - handle IBSS hearbeat failure
 * @mac_ctx: global mac context
 * @session: PE session entry
 *
 * This function is dummy.
 *
 * Return: None.
 */
static inline
void lim_ibss_heart_beat_handle(struct mac_context *mac,
				struct pe_session *pe_session)
{
}

/**
 * lim_process_ibss_peer_inactivity() - Handle inactive IBSS peer
 * @mac: Global MAC context
 * @buf: pointer to buffer
 *
 * This function is dummy.
 *
 * Return: None
 */
static inline
void lim_process_ibss_peer_inactivity(struct mac_context *mac, void *buf)
{
}
#endif
