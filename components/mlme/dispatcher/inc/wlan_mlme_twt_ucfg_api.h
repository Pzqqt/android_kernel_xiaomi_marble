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

/**
 * DOC: declare UCFG APIs for TWT exposed by the mlme component
 */

#ifndef _WLAN_MLME_TWT_UCFG_API_H_
#define _WLAN_MLME_TWT_UCFG_API_H_

#include <cfg_ucfg_api.h>
#include <wlan_mlme_public_struct.h>
#include <wlan_mlme_twt_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_mlme_api.h>
#include <wlan_mlme_main.h>
#include <wlan_mlme_twt_api.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_FEATURE_11AX)
/**
 * ucfg_mlme_get_twt_requestor() - Get twt requestor
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_twt_requestor(struct wlan_objmgr_psoc *psoc,
			    bool *val);

/**
 * ucfg_mlme_set_twt_requestor() - Set twt requestor
 * @psoc: pointer to psoc object
 * @val:  value that needs to set to this config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_requestor(struct wlan_objmgr_psoc *psoc,
			    bool val);

/**
 * ucfg_mlme_get_twt_responder() - Get twt responder
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_twt_responder(struct wlan_objmgr_psoc *psoc,
			    bool *val);

/**
 * ucfg_mlme_set_twt_responder() - Set twt responder
 * @psoc: pointer to psoc object
 * @val:  value that needs to set to this config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_responder(struct wlan_objmgr_psoc *psoc,
			    bool val);

/**
 * ucfg_mlme_get_bcast_twt() - Get bcast twt
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_bcast_twt(struct wlan_objmgr_psoc *psoc,
			bool *val);

/**
 * ucfg_mlme_set_bcast_twt() - Set bcast twt
 * @psoc: pointer to psoc object
 * @val:  value that needs to set to this config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_bcast_twt(struct wlan_objmgr_psoc *psoc,
			bool val);

/**
 * ucfg_mlme_get_twt_congestion_timeout() - Get twt congestion timeout
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_twt_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				     uint32_t *val);

/**
 * ucfg_mlme_set_twt_congestion_timeout() - Set twt congestion timeout
 * @psoc: pointer to psoc object
 * @val:  value that needs to set to this config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				     uint32_t val);

/**
 * ucfg_mlme_is_twt_enabled() - Get global twt config support
 * @psoc: pointer to psoc object
 *
 * Return: True if TWT is enabled else false.
 */
static inline bool
ucfg_mlme_is_twt_enabled(struct wlan_objmgr_psoc *psoc)
{
	return mlme_is_twt_enabled(psoc);
}

/**
 * ucfg_mlme_set_enable_twt() - Set global twt config support
 * @psoc: pointer to psoc object
 * @val:  value that needs to set to this config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_enable_twt(struct wlan_objmgr_psoc *psoc,
			 bool val);

/**
 * ucfg_mlme_get_twt_bcast_requestor() - Get twt requestor enabled
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_twt_bcast_requestor(struct wlan_objmgr_psoc *psoc,
				  bool *val);

/**
 * ucfg_mlme_set_twt_bcast_requestor() - Set Global twt bcast requestor support
 * @psoc: pointer to psoc object
 * @val:  Value to be set to config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_bcast_requestor(struct wlan_objmgr_psoc *psoc,
				  bool val);

/**
 * ucfg_mlme_get_twt_bcast_responder() - Get twt responder enabled
 * @psoc: pointer to psoc object
 * @val:  Pointer to the value which will be filled for the caller
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_get_twt_bcast_responder(struct wlan_objmgr_psoc *psoc,
				  bool *val);

/**
 * ucfg_mlme_set_twt_bcast_responder() - Set Global twt bcast responder support
 * @psoc: pointer to psoc object
 * @val:  Value to be set to config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_bcast_responder(struct wlan_objmgr_psoc *psoc,
				  bool val);

/**
 * ucfg_mlme_set_twt_bcast_requestor_tgt_cap() - Update the broadast requestor
 * target capability
 * @psoc: pointer to psoc object
 * @val:  Value to be set to config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_bcast_requestor_tgt_cap(struct wlan_objmgr_psoc *psoc,
					  bool val);

/**
 * ucfg_mlme_get_twt_bcast_requestor_tgt_cap() - Get broadcast requestor TGT
 * capability.
 * @psoc: pointer to psoc object
 *
 * Return: True if broadcast requestor target capability is present.
 */
static inline bool
ucfg_mlme_get_twt_bcast_requestor_tgt_cap(struct wlan_objmgr_psoc *psoc)
{
	return mlme_get_twt_bcast_requestor_tgt_cap(psoc);
}

/**
 * ucfg_mlme_set_twt_bcast_responder_tgt_cap() - Update the broadast responder
 * target capability
 * @psoc: pointer to psoc object
 * @val:  Value to be set to config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_mlme_set_twt_bcast_responder_tgt_cap(struct wlan_objmgr_psoc *psoc,
					  bool val);

/**
 * ucfg_mlme_is_twt_setup_in_progress() - Get TWT setup in progress for
 * given dialog id
 * @psoc: Pointer to global PSOC object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog id
 *
 * Return: True if already the TWT setup is in progress
 */
static inline bool
ucfg_mlme_is_twt_setup_in_progress(struct wlan_objmgr_psoc *psoc,
				   struct qdf_mac_addr *peer_mac,
				   uint8_t dialog_id)
{
	return mlme_is_twt_setup_in_progress(psoc, peer_mac, dialog_id);
}

/**
 * ucfg_mlme_set_twt_setup_done() - Set TWT setup done flag
 * @psoc: Pointer to global PSOC object
 * @peer_mac: Global peer mac address
 * @dialog_id: TWT session dialog id
 * @is_set: true if setup done flag needs to be set
 *
 * Return: None
 */
static inline void
ucfg_mlme_set_twt_setup_done(struct wlan_objmgr_psoc *psoc,
			     struct qdf_mac_addr *peer_mac,
			     uint8_t dialog_id, bool is_set)
{
	mlme_set_twt_setup_done(psoc, peer_mac, dialog_id, is_set);
}

/**
 * ucfg_mlme_is_twt_setup_done() - Get if the TWT setup for given dialog id
 * is complete.
 * @psoc: Pointer to global psoc object
 * @peer_mac:  Pointer to peer mac address
 * @dialog_id: TWT session dialog id
 *
 * Return: True if TWT setup is successful for the dialog id
 */
static inline bool
ucfg_mlme_is_twt_setup_done(struct wlan_objmgr_psoc *psoc,
			    struct qdf_mac_addr *peer_mac, uint8_t dialog_id)
{
	return mlme_is_twt_setup_done(psoc, peer_mac, dialog_id);
}

/**
 * ucfg_mlme_is_flexible_twt_enabled() - Get if flexible TWT is enabled
 * @psoc: Pointer to global psoc object
 *
 * Return: True if flexible TWT is supported
 */
static inline
bool ucfg_mlme_is_flexible_twt_enabled(struct wlan_objmgr_psoc *psoc)
{
	return mlme_is_flexible_twt_enabled(psoc);
}

/**
 * ucfg_mlme_get_twt_peer_capabilities() - Get peer capabilities
 * @psoc: Pointer to global psoc object
 * @peer_mac: Pointer to peer mac address
 *
 * Return: Peer capabilities bitmap
 */
static inline
uint8_t ucfg_mlme_get_twt_peer_capabilities(struct wlan_objmgr_psoc *psoc,
					    struct qdf_mac_addr *peer_mac)
{
	return mlme_get_twt_peer_capabilities(psoc, peer_mac);
}

/**
 * ucfg_mlme_init_twt_context() - Initialize TWT context
 * @psoc: Pointer to global psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS ucfg_mlme_init_twt_context(struct wlan_objmgr_psoc *psoc,
				      struct qdf_mac_addr *peer_mac,
				      uint8_t dialog_id)
{
	return mlme_init_twt_context(psoc, peer_mac, dialog_id);
}

/**
 * ucfg_mlme_set_twt_nudge_tgt_cap() - Set TWT nudge target capability.
 * @psoc: Pointer to global psoc object
 * @val: Value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_mlme_set_twt_nudge_tgt_cap(struct wlan_objmgr_psoc *psoc, bool val);

/**
 * ucfg_mlme_get_twt_nudge_tgt_cap() - Get TWT Nudge target capability
 * @psoc: Pointer to global psoc object
 * @val: Value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_mlme_get_twt_nudge_tgt_cap(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_mlme_set_twt_all_twt_tgt_cap() - Set TWT all dialog support target
 * @psoc: Pointer to global psoc object
 * @val: Value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_mlme_set_twt_all_twt_tgt_cap(struct wlan_objmgr_psoc *psoc, bool val);

/**
 * ucfg_mlme_get_twt_all_twt_tgt_cap()  - Get TWT all dialog support target
 * capability
 * @psoc: Pointer to global psoc object
 * @val: Value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_mlme_get_twt_all_twt_tgt_cap(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_mlme_set_twt_statistics_tgt_cap()  - Set TWT statistics target
 * capability
 * @psoc: Pointer to global psoc object
 * @val: Value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_mlme_set_twt_statistics_tgt_cap(struct wlan_objmgr_psoc *psoc, bool val);

/**
 * ucfg_mlme_get_twt_statistics_tgt_cap() - Get TWT statistics target capability
 * @psoc: Pointer to global psoc object
 * @val: Value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_mlme_get_twt_statistics_tgt_cap(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_mlme_set_twt_session_state()  - Set TWT session state
 * @psoc: Pointer to global psoc object
 * @peer_mac:  Pointer to peer mac address
 * @dialog_id: TWT session dialog id
 * @state: TWT state
 *
 * Return: None
 */
static inline
void ucfg_mlme_set_twt_session_state(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_session_state state)
{
	mlme_set_twt_session_state(psoc, peer_mac, dialog_id, state);
}

/**
 * ucfg_mlme_get_twt_session_state()  - Get TWT session state
 * @psoc: Pointer to global psoc object
 * @peer_mac:  Pointer to peer mac address
 * @dialog_id: TWT session dialog id
 *
 * Return: enum wlan_twt_session_state
 */
static inline enum wlan_twt_session_state
ucfg_mlme_get_twt_session_state(struct wlan_objmgr_psoc *psoc,
				struct qdf_mac_addr *peer_mac,
				uint8_t dialog_id)
{
	return mlme_get_twt_session_state(psoc, peer_mac, dialog_id);
}
#else
static inline QDF_STATUS
ucfg_mlme_get_twt_requestor(struct wlan_objmgr_psoc *psoc,
			    bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_requestor(struct wlan_objmgr_psoc *psoc,
			    bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_responder(struct wlan_objmgr_psoc *psoc,
			    bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_responder(struct wlan_objmgr_psoc *psoc,
			    bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_bcast_twt(struct wlan_objmgr_psoc *psoc,
			bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_bcast_twt(struct wlan_objmgr_psoc *psoc,
			bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				     uint32_t *val)
{
	*val = 0;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				     uint32_t val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline bool
ucfg_mlme_is_twt_enabled(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_enable_twt(struct wlan_objmgr_psoc *psoc,
			 bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline bool
ucfg_mlme_is_twt_setup_done(struct wlan_objmgr_psoc *psoc,
			    struct qdf_mac_addr *peer_mac, uint8_t dialog_id)
{
	return 0;
}

static inline void
ucfg_mlme_set_twt_setup_done(struct wlan_objmgr_psoc *psoc,
			     struct qdf_mac_addr *peer_mac,
			     uint8_t dialog_id, bool is_set)
{}

static inline bool
ucfg_mlme_is_twt_setup_in_progress(struct wlan_objmgr_psoc *psoc,
				   struct qdf_mac_addr *peer_mac,
				   uint8_t dialog_id)
{
	return false;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_bcast_requestor(struct wlan_objmgr_psoc *psoc,
				  bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_bcast_requestor(struct wlan_objmgr_psoc *psoc,
				  bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_bcast_responder(struct wlan_objmgr_psoc *psoc,
				  bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_bcast_responder(struct wlan_objmgr_psoc *psoc,
				  bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_bcast_requestor_tgt_cap(struct wlan_objmgr_psoc *psoc,
					  bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_bcast_responder_tgt_cap(struct wlan_objmgr_psoc *psoc,
					  bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
bool ucfg_mlme_is_flexible_twt_enabled(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static inline bool
ucfg_mlme_get_twt_bcast_requestor_tgt_cap(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static inline
uint8_t ucfg_mlme_get_twt_peer_capabilities(struct wlan_objmgr_psoc *psoc,
					    struct qdf_mac_addr *peer_mac)
{
	return 0;
}

static inline
QDF_STATUS ucfg_mlme_init_twt_context(struct wlan_objmgr_psoc *psoc,
				      struct qdf_mac_addr *peer_mac,
				      uint8_t dialog_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_nudge_tgt_cap(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_all_twt_tgt_cap(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_get_twt_statistics_tgt_cap(struct wlan_objmgr_psoc *psoc, bool *val)
{
	*val = false;
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_nudge_tgt_cap(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_all_twt_tgt_cap(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_mlme_set_twt_statistics_tgt_cap(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
void ucfg_mlme_set_twt_session_state(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_session_state state)
{}

static inline enum wlan_twt_session_state
ucfg_mlme_get_twt_session_state(struct wlan_objmgr_psoc *psoc,
				struct qdf_mac_addr *peer_mac,
				uint8_t dialog_id)
{
	return WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED;
}
#endif /* defined(WLAN_SUPPORT_TWT) && defined(WLAN_FEATURE_11AX) */
#endif /* _WLAN_MLME_TWT_UCFG_API_H_ */
